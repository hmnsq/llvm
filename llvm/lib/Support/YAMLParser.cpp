//===- YAMLParser.cpp - Simple YAML parser --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements a YAML parser.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/YAMLParser.h"
#include "llvm/ADT/AllocatorList.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SMLoc.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/Unicode.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <utility>

using namespace llvm;
using namespace yaml;

enum UnicodeEncodingForm {
  UEF_UTF32_LE, ///< UTF-32 Little Endian
  UEF_UTF32_BE, ///< UTF-32 Big Endian
  UEF_UTF16_LE, ///< UTF-16 Little Endian
  UEF_UTF16_BE, ///< UTF-16 Big Endian
  UEF_UTF8,     ///< UTF-8 or ascii.
  UEF_Unknown   ///< Not a valid Unicode encoding.
};

/// EncodingInfo - Holds the encoding type and length of the byte order mark if
///                it exists. Length is in {0, 2, 3, 4}.
using EncodingInfo = std::pair<UnicodeEncodingForm, unsigned>;

/// getUnicodeEncoding - Reads up to the first 4 bytes to determine the Unicode
///                      encoding form of \a Input.
///
/// @param Input A string of length 0 or more.
/// @returns An EncodingInfo indicating the Unicode encoding form of the input
///          and how long the byte order mark is if one exists.
static EncodingInfo getUnicodeEncoding(StringRef Input) {
  nếu (Input.empty())
    trả_về {UEF_Unknown, 0};

  switch (uint8_t(Input[0])) {
  case 0x00:
    nếu (Input.size() >= 4) {
      nếu (  Input[1] == 0
         && uint8_t(Input[2]) == 0xFE
         && uint8_t(Input[3]) == 0xFF)
        trả_về {UEF_UTF32_BE, 4};
      nếu (Input[1] == 0 && Input[2] == 0 && Input[3] != 0)
        trả_về {UEF_UTF32_BE, 0};
    }

    nếu (Input.size() >= 2 && Input[1] != 0)
      trả_về {UEF_UTF16_BE, 0};
    trả_về {UEF_Unknown, 0};
  case 0xFF:
    nếu (  Input.size() >= 4
       && uint8_t(Input[1]) == 0xFE
       && Input[2] == 0
       && Input[3] == 0)
      trả_về {UEF_UTF32_LE, 4};

    nếu (Input.size() >= 2 && uint8_t(Input[1]) == 0xFE)
      trả_về {UEF_UTF16_LE, 2};
    trả_về {UEF_Unknown, 0};
  case 0xFE:
    nếu (Input.size() >= 2 && uint8_t(Input[1]) == 0xFF)
      trả_về {UEF_UTF16_BE, 2};
    trả_về {UEF_Unknown, 0};
  case 0xEF:
    nếu (  Input.size() >= 3
       && uint8_t(Input[1]) == 0xBB
       && uint8_t(Input[2]) == 0xBF)
      trả_về {UEF_UTF8, 3};
    trả_về {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  nếu (Input.size() >= 4 && Input[1] == 0 && Input[2] == 0 && Input[3] == 0)
    trả_về {UEF_UTF32_LE, 0};

  nếu (Input.size() >= 2 && Input[1] == 0)
    trả_về {UEF_UTF16_LE, 0};

  trả_về {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
void Node::anchor() {}
void NullNode::anchor() {}
void ScalarNode::anchor() {}
void BlockScalarNode::anchor() {}
void KeyValueNode::anchor() {}
void MappingNode::anchor() {}
void SequenceNode::anchor() {}
void AliasNode::anchor() {}

namespace llvm {
namespace yaml {

/// Token - A single YAML token.
struct Token {
  enum TokenKind {
    TK_Error, // Uninitialized token.
    TK_StreamStart,
    TK_StreamEnd,
    TK_VersionDirective,
    TK_TagDirective,
    TK_DocumentStart,
    TK_DocumentEnd,
    TK_BlockEntry,
    TK_BlockEnd,
    TK_BlockSequenceStart,
    TK_BlockMappingStart,
    TK_FlowEntry,
    TK_FlowSequenceStart,
    TK_FlowSequenceEnd,
    TK_FlowMappingStart,
    TK_FlowMappingEnd,
    TK_Key,
    TK_Value,
    TK_Scalar,
    TK_BlockScalar,
    TK_Alias,
    TK_Anchor,
    TK_Tag
  } Kind = TK_Error;

  /// A string of length 0 or more whose begin() points to the logical location
  /// of the token in the input.
  StringRef Range;

  /// The value of a block scalar node.
  std::string Value;

  Token() = default;
};

} // end namespace yaml
} // end namespace llvm

using TokenQueueT = BumpPtrList<Token>;

namespace {

/// This struct is used to track simple keys.
///
/// Simple keys are handled by creating an entry in SimpleKeys for each Token
/// which could legally be the start of a simple key. When peekNext is called,
/// if the Token To be returned is referenced by a SimpleKey, we continue
/// tokenizing until that potential simple key has either been found to not be
/// a simple key (we moved on to the next line or went further than 1024 chars).
/// Or when we run into a Value, and then insert a Key token (and possibly
/// others) before the SimpleKey's Tok.
struct SimpleKey {
  TokenQueueT::iterator Tok;
  unsigned Column = 0;
  unsigned Line = 0;
  unsigned FlowLevel = 0;
  bool IsRequired = false;

  bool operator ==(const SimpleKey &Other) {
    trả_về Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
using UTF8Decoded = std::pair<uint32_t, unsigned>;

static UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator End = Range.end();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  nếu (Position < End && (*Position & 0x80) == 0) {
    trả_về {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  nếu (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    nếu (codepoint >= 0x80)
      trả_về {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  nếu (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    nếu (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      trả_về {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  nếu (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    nếu (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      trả_về {codepoint, 4};
  }
  trả_về {0, 0};
}

namespace llvm {
namespace yaml {

/// Scans YAML tokens from a MemoryBuffer.
class Scanner {
public:
  Scanner(StringRef Input, SourceMgr &SM, bool ShowColors = true,
          std::error_code *EC = nullptr);
  Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, bool ShowColors = true,
          std::error_code *EC = nullptr);

  /// Parse the next token and return it without popping it.
  Token &peekNext();

  /// Parse the next token and pop it from the queue.
  Token getNext();

  void printError(SMLoc Loc, SourceMgr::DiagKind Kind, const Twine &Message,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.PrintMessage(Loc, Kind, Message, Ranges, /* FixIts= */ {}, ShowColors);
  }

  void setError(const Twine &Message, StringRef::iterator Position) {
    nếu (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    nếu (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    nếu (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = true;
  }

  /// Returns true if an error occurred while parsing.
  bool failed() {
    trả_về Failed;
  }

private:
  void init(MemoryBufferRef Buffer);

  StringRef currentInput() {
    trả_về StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    trả_về ::decodeUTF8(StringRef(Position, End - Position));
  }

  // The following functions are based on the gramar rules in the YAML spec. The
  // style of the function names it meant to closely match how they are written
  // in the spec. The number within the [] is the number of the grammar rule in
  // the spec.
  //
  // See 4.2 [Production Naming Conventions] for the meaning of the prefixes.
  //
  // c-
  //   A production starting and ending with a special character.
  // b-
  //   A production matching a single line break.
  // nb-
  //   A production starting and ending with a non-break character.
  // s-
  //   A production starting and ending with a white space character.
  // ns-
  //   A production starting and ending with a non-space character.
  // l-
  //   A production matching complete line(s).

  /// Skip a single nb-char[27] starting at Position.
  ///
  /// A nb-char is 0x9 | [0x20-0x7E] | 0x85 | [0xA0-0xD7FF] | [0xE000-0xFEFE]
  ///                  | [0xFF00-0xFFFD] | [0x10000-0x10FFFF]
  ///
  /// @returns The code unit after the nb-char, or Position if it's not an
  ///          nb-char.
  StringRef::iterator skip_nb_char(StringRef::iterator Position);

  /// Skip a single b-break[28] starting at Position.
  ///
  /// A b-break is 0xD 0xA | 0xD | 0xA
  ///
  /// @returns The code unit after the b-break, or Position if it's not a
  ///          b-break.
  StringRef::iterator skip_b_break(StringRef::iterator Position);

  /// Skip a single s-space[31] starting at Position.
  ///
  /// An s-space is 0x20
  ///
  /// @returns The code unit after the s-space, or Position if it's not a
  ///          s-space.
  StringRef::iterator skip_s_space(StringRef::iterator Position);

  /// Skip a single s-white[33] starting at Position.
  ///
  /// A s-white is 0x20 | 0x9
  ///
  /// @returns The code unit after the s-white, or Position if it's not a
  ///          s-white.
  StringRef::iterator skip_s_white(StringRef::iterator Position);

  /// Skip a single ns-char[34] starting at Position.
  ///
  /// A ns-char is nb-char - s-white
  ///
  /// @returns The code unit after the ns-char, or Position if it's not a
  ///          ns-char.
  StringRef::iterator skip_ns_char(StringRef::iterator Position);

  using SkipWhileFunc = StringRef::iterator (Scanner::*)(StringRef::iterator);

  /// Skip minimal well-formed code unit subsequences until Func
  ///        returns its input.
  ///
  /// @returns The code unit after the last minimal well-formed code unit
  ///          subsequence that Func accepted.
  StringRef::iterator skip_while( SkipWhileFunc Func
                                , StringRef::iterator Position);

  /// Skip minimal well-formed code unit subsequences until Func returns its
  /// input.
  void advanceWhile(SkipWhileFunc Func);

  /// Scan ns-uri-char[39]s starting at Cur.
  ///
  /// This updates Cur and Column while scanning.
  void scan_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  bool consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  void skip(uint32_t Distance);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  bool isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  bool isPlainSafeNonBlank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  bool isLineEmpty(StringRef Line);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  bool consumeLineBreakIfPresent();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  void saveSimpleKeyCandidate( TokenQueueT::iterator Tok
                             , unsigned AtColumn
                             , bool IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  void removeStaleSimpleKeyCandidates();

  /// Remove all simple keys on FlowLevel \a Level.
  void removeSimpleKeyCandidatesOnFlowLevel(unsigned Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  bool unrollIndent(số_nguyên ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  bool rollIndent( số_nguyên ToColumn
                 , Token::TokenKind Kind
                 , TokenQueueT::iterator InsertPoint);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  void skipComment();

  /// Skip whitespace and comments until the start of the next token.
  void scanToNextToken();

  /// Must be the first token generated.
  bool scanStreamStart();

  /// Generate tokens needed to close out the stream.
  bool scanStreamEnd();

  /// Scan a %BLAH directive.
  bool scanDirective();

  /// Scan a ... or ---.
  bool scanDocumentIndicator(bool IsStart);

  /// Scan a [ or { and generate the proper flow collection start token.
  bool scanFlowCollectionStart(bool IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  bool scanFlowCollectionEnd(bool IsSequence);

  /// Scan the , that separates entries in a flow collection.
  bool scanFlowEntry();

  /// Scan the - that starts block sequence entries.
  bool scanBlockEntry();

  /// Scan an explicit ? indicating a key.
  bool scanKey();

  /// Scan an explicit : indicating a value.
  bool scanValue();

  /// Scan a quoted scalar.
  bool scanFlowScalar(bool IsDoubleQuoted);

  /// Scan an unquoted scalar.
  bool scanPlainScalar();

  /// Scan an Alias or Anchor starting with * or &.
  bool scanAliasOrAnchor(bool IsAlias);

  /// Scan a block scalar starting with | or >.
  bool scanBlockScalar(bool IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  bool scanBlockScalarIndicators(char &StyleIndicator, char &ChompingIndicator,
                                 unsigned &IndentIndicator, bool &IsDone);

  /// Scan a style indicator in a block scalar header.
  char scanBlockStyleIndicator();

  /// Scan a chomping indicator in a block scalar header.
  char scanBlockChompingIndicator();

  /// Scan an indentation indicator in a block scalar header.
  unsigned scanBlockIndentationIndicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  bool scanBlockScalarHeader(char &ChompingIndicator, unsigned &IndentIndicator,
                             bool &IsDone);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  bool findBlockScalarIndent(unsigned &BlockIndent, unsigned BlockExitIndent,
                             unsigned &LineBreaks, bool &IsDone);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  bool scanBlockScalarIndent(unsigned BlockIndent, unsigned BlockExitIndent,
                             bool &IsDone);

  /// Scan a tag of the form !stuff.
  bool scanTag();

  /// Dispatch to the next scanning function based on \a *Cur.
  bool fetchMoreTokens();

  /// The SourceMgr used for diagnostics and buffer management.
  SourceMgr &SM;

  /// The original input.
  MemoryBufferRef InputBuffer;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator End;

  /// Current YAML indentation level in spaces.
  số_nguyên Indent;

  /// Current column number in Unicode code points.
  unsigned Column;

  /// Current line number.
  unsigned Line;

  /// How deep we are in flow style containers. 0 Means at block level.
  unsigned FlowLevel;

  /// Are we at the start of the stream?
  bool IsStartOfStream;

  /// Can the next token be the start of a simple key?
  bool IsSimpleKeyAllowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  bool IsAdjacentValueAllowedInFlow;

  /// True if an error has occurred.
  bool Failed;

  /// Should colors be used when printing out the diagnostic messages?
  bool ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  TokenQueueT TokenQueue;

  /// Indentation levels.
  SmallVector<số_nguyên, 4> Indents;

  /// Potential simple keys.
  SmallVector<SimpleKey, 4> SimpleKeys;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
static void encodeUTF8( uint32_t UnicodeScalarValue
                      , SmallVectorImpl<char> &Result) {
  nếu (UnicodeScalarValue <= 0x7F) {
    Result.push_back(UnicodeScalarValue & 0x7F);
  } không_thì nếu (UnicodeScalarValue <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((UnicodeScalarValue & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (UnicodeScalarValue & 0x3F);
    Result.push_back(FirstByte);
    Result.push_back(SecondByte);
  } không_thì nếu (UnicodeScalarValue <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((UnicodeScalarValue & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((UnicodeScalarValue & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (UnicodeScalarValue & 0x3F);
    Result.push_back(FirstByte);
    Result.push_back(SecondByte);
    Result.push_back(ThirdByte);
  } không_thì nếu (UnicodeScalarValue <= 0x10FFFF) {
    uint8_t FirstByte = 0xF0 | ((UnicodeScalarValue & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((UnicodeScalarValue & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((UnicodeScalarValue & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (UnicodeScalarValue & 0x3F);
    Result.push_back(FirstByte);
    Result.push_back(SecondByte);
    Result.push_back(ThirdByte);
    Result.push_back(FourthByte);
  }
}

bool yaml::dumpTokens(StringRef Input, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(Input, SM);
  trong_khi (true) {
    Token T = scanner.getNext();
    switch (T.Kind) {
    case Token::TK_StreamStart:
      OS << "Stream-Start: ";
      break;
    case Token::TK_StreamEnd:
      OS << "Stream-End: ";
      break;
    case Token::TK_VersionDirective:
      OS << "Version-Directive: ";
      break;
    case Token::TK_TagDirective:
      OS << "Tag-Directive: ";
      break;
    case Token::TK_DocumentStart:
      OS << "Document-Start: ";
      break;
    case Token::TK_DocumentEnd:
      OS << "Document-End: ";
      break;
    case Token::TK_BlockEntry:
      OS << "Block-Entry: ";
      break;
    case Token::TK_BlockEnd:
      OS << "Block-End: ";
      break;
    case Token::TK_BlockSequenceStart:
      OS << "Block-Sequence-Start: ";
      break;
    case Token::TK_BlockMappingStart:
      OS << "Block-Mapping-Start: ";
      break;
    case Token::TK_FlowEntry:
      OS << "Flow-Entry: ";
      break;
    case Token::TK_FlowSequenceStart:
      OS << "Flow-Sequence-Start: ";
      break;
    case Token::TK_FlowSequenceEnd:
      OS << "Flow-Sequence-End: ";
      break;
    case Token::TK_FlowMappingStart:
      OS << "Flow-Mapping-Start: ";
      break;
    case Token::TK_FlowMappingEnd:
      OS << "Flow-Mapping-End: ";
      break;
    case Token::TK_Key:
      OS << "Key: ";
      break;
    case Token::TK_Value:
      OS << "Value: ";
      break;
    case Token::TK_Scalar:
      OS << "Scalar: ";
      break;
    case Token::TK_BlockScalar:
      OS << "Block Scalar: ";
      break;
    case Token::TK_Alias:
      OS << "Alias: ";
      break;
    case Token::TK_Anchor:
      OS << "Anchor: ";
      break;
    case Token::TK_Tag:
      OS << "Tag: ";
      break;
    case Token::TK_Error:
      break;
    }
    OS << T.Range << "\n";
    nếu (T.Kind == Token::TK_StreamEnd)
      break;
    không_thì nếu (T.Kind == Token::TK_Error)
      trả_về false;
  }
  trả_về true;
}

bool yaml::scanTokens(StringRef Input) {
  SourceMgr SM;
  Scanner scanner(Input, SM);
  trong_khi (true) {
    Token T = scanner.getNext();
    nếu (T.Kind == Token::TK_StreamEnd)
      break;
    không_thì nếu (T.Kind == Token::TK_Error)
      trả_về false;
  }
  trả_về true;
}

std::string yaml::escape(StringRef Input, bool EscapePrintable) {
  std::string EscapedInput;
  cho (StringRef::iterator i = Input.begin(), e = Input.end(); i != e; ++i) {
    nếu (*i == '\\')
      EscapedInput += "\\\\";
    không_thì nếu (*i == '"')
      EscapedInput += "\\\"";
    không_thì nếu (*i == 0)
      EscapedInput += "\\0";
    không_thì nếu (*i == 0x07)
      EscapedInput += "\\a";
    không_thì nếu (*i == 0x08)
      EscapedInput += "\\b";
    không_thì nếu (*i == 0x09)
      EscapedInput += "\\t";
    không_thì nếu (*i == 0x0A)
      EscapedInput += "\\n";
    không_thì nếu (*i == 0x0B)
      EscapedInput += "\\v";
    không_thì nếu (*i == 0x0C)
      EscapedInput += "\\f";
    không_thì nếu (*i == 0x0D)
      EscapedInput += "\\r";
    không_thì nếu (*i == 0x1B)
      EscapedInput += "\\e";
    không_thì nếu ((unsigned char)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      EscapedInput += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } không_thì nếu (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded UnicodeScalarValue
        = decodeUTF8(StringRef(i, Input.end() - i));
      nếu (UnicodeScalarValue.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(EscapedInput, Val);
        // FIXME: Error reporting.
        trả_về EscapedInput;
      }
      nếu (UnicodeScalarValue.first == 0x85)
        EscapedInput += "\\N";
      không_thì nếu (UnicodeScalarValue.first == 0xA0)
        EscapedInput += "\\_";
      không_thì nếu (UnicodeScalarValue.first == 0x2028)
        EscapedInput += "\\L";
      không_thì nếu (UnicodeScalarValue.first == 0x2029)
        EscapedInput += "\\P";
      không_thì nếu (!EscapePrintable &&
               sys::unicode::isPrintable(UnicodeScalarValue.first))
        EscapedInput += StringRef(i, UnicodeScalarValue.second);
      không_thì {
        std::string HexStr = utohexstr(UnicodeScalarValue.first);
        nếu (HexStr.size() <= 2)
          EscapedInput += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        không_thì nếu (HexStr.size() <= 4)
          EscapedInput += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        không_thì nếu (HexStr.size() <= 8)
          EscapedInput += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += UnicodeScalarValue.second - 1;
    } không_thì {
      EscapedInput.push_back(*i);
    }
  }
  trả_về EscapedInput;
}

std::optional<bool> yaml::parseBool(StringRef S) {
  switch (S.size()) {
  case 1:
    switch (S.front()) {
    case 'y':
    case 'Y':
      trả_về true;
    case 'n':
    case 'N':
      trả_về false;
    default:
      trả_về std::nullopt;
    }
  case 2:
    switch (S.front()) {
    case 'O':
      nếu (S[1] == 'N') // ON
        trả_về true;
      [[fallthrough]];
    case 'o':
      nếu (S[1] == 'n') //[Oo]n
        trả_về true;
      trả_về std::nullopt;
    case 'N':
      nếu (S[1] == 'O') // NO
        trả_về false;
      [[fallthrough]];
    case 'n':
      nếu (S[1] == 'o') //[Nn]o
        trả_về false;
      trả_về std::nullopt;
    default:
      trả_về std::nullopt;
    }
  case 3:
    switch (S.front()) {
    case 'O':
      nếu (S.drop_front() == "FF") // OFF
        trả_về false;
      [[fallthrough]];
    case 'o':
      nếu (S.drop_front() == "ff") //[Oo]ff
        trả_về false;
      trả_về std::nullopt;
    case 'Y':
      nếu (S.drop_front() == "ES") // YES
        trả_về true;
      [[fallthrough]];
    case 'y':
      nếu (S.drop_front() == "es") //[Yy]es
        trả_về true;
      trả_về std::nullopt;
    default:
      trả_về std::nullopt;
    }
  case 4:
    switch (S.front()) {
    case 'T':
      nếu (S.drop_front() == "RUE") // TRUE
        trả_về true;
      [[fallthrough]];
    case 't':
      nếu (S.drop_front() == "rue") //[Tt]rue
        trả_về true;
      trả_về std::nullopt;
    default:
      trả_về std::nullopt;
    }
  case 5:
    switch (S.front()) {
    case 'F':
      nếu (S.drop_front() == "ALSE") // FALSE
        trả_về false;
      [[fallthrough]];
    case 'f':
      nếu (S.drop_front() == "alse") //[Ff]alse
        trả_về false;
      trả_về std::nullopt;
    default:
      trả_về std::nullopt;
    }
  default:
    trả_về std::nullopt;
  }
}

Scanner::Scanner(StringRef Input, SourceMgr &sm, bool ShowColors,
                 std::error_code *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(MemoryBufferRef(Input, "YAML"));
}

Scanner::Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, bool ShowColors,
                 std::error_code *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(Buffer);
}

void Scanner::init(MemoryBufferRef Buffer) {
  InputBuffer = Buffer;
  Current = InputBuffer.getBufferStart();
  End = InputBuffer.getBufferEnd();
  Indent = -1;
  Column = 0;
  Line = 0;
  FlowLevel = 0;
  IsStartOfStream = true;
  IsSimpleKeyAllowed = true;
  IsAdjacentValueAllowedInFlow = false;
  Failed = false;
  std::unique_ptr<MemoryBuffer> InputBufferOwner =
      MemoryBuffer::getMemBuffer(Buffer, /*RequiresNullTerminator=*/false);
  SM.AddNewSourceBuffer(std::move(InputBufferOwner), SMLoc());
}

Token &Scanner::peekNext() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  bool NeedMore = false;
  trong_khi (true) {
    nếu (TokenQueue.empty() || NeedMore) {
      nếu (!fetchMoreTokens()) {
        TokenQueue.clear();
        SimpleKeys.clear();
        TokenQueue.push_back(Token());
        trả_về TokenQueue.front();
      }
    }
    assert(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    removeStaleSimpleKeyCandidates();
    SimpleKey SK;
    SK.Tok = TokenQueue.begin();
    nếu (!is_contained(SimpleKeys, SK))
      break;
    không_thì
      NeedMore = true;
  }
  trả_về TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  nếu (!TokenQueue.empty())
    TokenQueue.pop_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  nếu (TokenQueue.empty())
    TokenQueue.resetAlloc();

  trả_về Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  nếu (Position == End)
    trả_về Position;
  // Check 7 bit c-printable - b-char.
  nếu (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    trả_về Position + 1;

  // Check for valid UTF-8.
  nếu (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    nếu (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      trả_về Position + u8d.second;
  }
  trả_về Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  nếu (Position == End)
    trả_về Position;
  nếu (*Position == 0x0D) {
    nếu (Position + 1 != End && *(Position + 1) == 0x0A)
      trả_về Position + 2;
    trả_về Position + 1;
  }

  nếu (*Position == 0x0A)
    trả_về Position + 1;
  trả_về Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  nếu (Position == End)
    trả_về Position;
  nếu (*Position == ' ')
    trả_về Position + 1;
  trả_về Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  nếu (Position == End)
    trả_về Position;
  nếu (*Position == ' ' || *Position == '\t')
    trả_về Position + 1;
  trả_về Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  nếu (Position == End)
    trả_về Position;
  nếu (*Position == ' ' || *Position == '\t')
    trả_về Position;
  trả_về skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  trong_khi (true) {
    StringRef::iterator i = (this->*Func)(Position);
    nếu (i == Position)
      break;
    Position = i;
  }
  trả_về Position;
}

void Scanner::advanceWhile(SkipWhileFunc Func) {
  auto Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

static bool is_ns_hex_digit(const char C) { trả_về isAlnum(C); }

static bool is_ns_word_char(const char C) { trả_về C == '-' || isAlpha(C); }

void Scanner::scan_ns_uri_char() {
  trong_khi (true) {
    nếu (Current == End)
      break;
    nếu ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } không_thì {
      break;
    }
  }
}

bool Scanner::consume(uint32_t Expected) {
  nếu (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    trả_về false;
  }
  nếu (Current == End)
    trả_về false;
  nếu (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    trả_về false;
  }
  nếu (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    trả_về true;
  }
  trả_về false;
}

void Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  assert(Current <= End && "Skipped past the end");
}

bool Scanner::isBlankOrBreak(StringRef::iterator Position) {
  nếu (Position == End)
    trả_về false;
  trả_về *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

bool Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  nếu (Position == End || isBlankOrBreak(Position))
    trả_về false;
  nếu (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    trả_về false;
  trả_về true;
}

bool Scanner::isLineEmpty(StringRef Line) {
  cho (const auto *Position = Line.begin(); Position != Line.end(); ++Position)
    nếu (!isBlankOrBreak(Position))
      trả_về false;
  trả_về true;
}

bool Scanner::consumeLineBreakIfPresent() {
  auto Next = skip_b_break(Current);
  nếu (Next == Current)
    trả_về false;
  Column = 0;
  ++Line;
  Current = Next;
  trả_về true;
}

void Scanner::saveSimpleKeyCandidate( TokenQueueT::iterator Tok
                                    , unsigned AtColumn
                                    , bool IsRequired) {
  nếu (IsSimpleKeyAllowed) {
    SimpleKey SK;
    SK.Tok = Tok;
    SK.Line = Line;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    SimpleKeys.push_back(SK);
  }
}

void Scanner::removeStaleSimpleKeyCandidates() {
  cho (SmallVectorImpl<SimpleKey>::iterator i = SimpleKeys.begin();
                                            i != SimpleKeys.end();) {
    nếu (i->Line != Line || i->Column + 1024 < Column) {
      nếu (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = SimpleKeys.erase(i);
    } không_thì {
      ++i;
    }
  }
}

void Scanner::removeSimpleKeyCandidatesOnFlowLevel(unsigned Level) {
  nếu (!SimpleKeys.empty() && (SimpleKeys.end() - 1)->FlowLevel == Level)
    SimpleKeys.pop_back();
}

bool Scanner::unrollIndent(số_nguyên ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  nếu (FlowLevel != 0)
    trả_về true;

  trong_khi (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.push_back(T);
    Indent = Indents.pop_back_val();
  }

  trả_về true;
}

bool Scanner::rollIndent( số_nguyên ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator InsertPoint) {
  nếu (FlowLevel)
    trả_về true;
  nếu (Indent < ToColumn) {
    Indents.push_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.insert(InsertPoint, T);
  }
  trả_về true;
}

void Scanner::skipComment() {
  nếu (Current == End || *Current != '#')
    trả_về;
  trong_khi (true) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    nếu (I == Current)
      break;
    Current = I;
    ++Column;
  }
}

void Scanner::scanToNextToken() {
  trong_khi (true) {
    trong_khi (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    nếu (i == Current)
      break;
    Current = i;
    ++Line;
    Column = 0;
    // New lines may start a simple key.
    nếu (!FlowLevel)
      IsSimpleKeyAllowed = true;
  }
}

bool Scanner::scanStreamStart() {
  IsStartOfStream = false;

  EncodingInfo EI = getUnicodeEncoding(currentInput());

  Token T;
  T.Kind = Token::TK_StreamStart;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.push_back(T);
  Current += EI.second;
  trả_về true;
}

bool Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  nếu (Column != 0) {
    Column = 0;
    ++Line;
  }

  unrollIndent(-1);
  SimpleKeys.clear();
  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = false;

  Token T;
  T.Kind = Token::TK_StreamEnd;
  T.Range = StringRef(Current, 0);
  TokenQueue.push_back(T);
  trả_về true;
}

bool Scanner::scanDirective() {
  // Reset the indentation level.
  unrollIndent(-1);
  SimpleKeys.clear();
  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = false;

  StringRef::iterator Start = Current;
  consume('%');
  StringRef::iterator NameStart = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(NameStart, Current - NameStart);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  nếu (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.push_back(T);
    trả_về true;
  } không_thì nếu(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.push_back(T);
    trả_về true;
  }
  trả_về false;
}

bool Scanner::scanDocumentIndicator(bool IsStart) {
  unrollIndent(-1);
  SimpleKeys.clear();
  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = false;

  Token T;
  T.Kind = IsStart ? Token::TK_DocumentStart : Token::TK_DocumentEnd;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.push_back(T);
  trả_về true;
}

bool Scanner::scanFlowCollectionStart(bool IsSequence) {
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);

  // [ and { may begin a simple key.
  saveSimpleKeyCandidate(--TokenQueue.end(), Column - 1, false);

  // And may also be followed by a simple key.
  IsSimpleKeyAllowed = true;
  // Adjacent values are allowed in flows only after JSON-style keys.
  IsAdjacentValueAllowedInFlow = false;
  ++FlowLevel;
  trả_về true;
}

bool Scanner::scanFlowCollectionEnd(bool IsSequence) {
  removeSimpleKeyCandidatesOnFlowLevel(FlowLevel);
  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = true;
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceEnd
                      : Token::TK_FlowMappingEnd;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);
  nếu (FlowLevel)
    --FlowLevel;
  trả_về true;
}

bool Scanner::scanFlowEntry() {
  removeSimpleKeyCandidatesOnFlowLevel(FlowLevel);
  IsSimpleKeyAllowed = true;
  IsAdjacentValueAllowedInFlow = false;
  Token T;
  T.Kind = Token::TK_FlowEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);
  trả_về true;
}

bool Scanner::scanBlockEntry() {
  rollIndent(Column, Token::TK_BlockSequenceStart, TokenQueue.end());
  removeSimpleKeyCandidatesOnFlowLevel(FlowLevel);
  IsSimpleKeyAllowed = true;
  IsAdjacentValueAllowedInFlow = false;
  Token T;
  T.Kind = Token::TK_BlockEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);
  trả_về true;
}

bool Scanner::scanKey() {
  nếu (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  removeSimpleKeyCandidatesOnFlowLevel(FlowLevel);
  IsSimpleKeyAllowed = !FlowLevel;
  IsAdjacentValueAllowedInFlow = false;

  Token T;
  T.Kind = Token::TK_Key;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);
  trả_về true;
}

bool Scanner::scanValue() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  nếu (!SimpleKeys.empty()) {
    SimpleKey SK = SimpleKeys.pop_back_val();
    Token T;
    T.Kind = Token::TK_Key;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    cho (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      nếu (i == SK.Tok)
        break;
    }
    nếu (i == e) {
      Failed = true;
      trả_về false;
    }
    i = TokenQueue.insert(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    IsSimpleKeyAllowed = false;
  } không_thì {
    nếu (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    IsSimpleKeyAllowed = !FlowLevel;
  }
  IsAdjacentValueAllowedInFlow = false;

  Token T;
  T.Kind = Token::TK_Value;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);
  trả_về true;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE static bool
wasEscaped(StringRef::iterator First, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
static bool wasEscaped(StringRef::iterator First,
                       StringRef::iterator Position) {
  assert(Position - 1 >= First);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  trong_khi (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  trả_về (Position - 1 - I) % 2 == 1;
}

bool Scanner::scanFlowScalar(bool IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  nếu (IsDoubleQuoted) {
    do {
      ++Current;
      trong_khi (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } trong_khi (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } không_thì {
    skip(1);
    trong_khi (Current != End) {
      // Skip a ' followed by another '.
      nếu (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        continue;
      } không_thì nếu (*Current == '\'')
        break;
      StringRef::iterator i = skip_nb_char(Current);
      nếu (i == Current) {
        i = skip_b_break(Current);
        nếu (i == Current)
          break;
        Current = i;
        Column = 0;
        ++Line;
      } không_thì {
        nếu (i == End)
          break;
        Current = i;
        ++Column;
      }
    }
  }

  nếu (Current == End) {
    setError("Expected quote at end of scalar", Current);
    trả_về false;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, false);

  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = true;

  trả_về true;
}

bool Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  unsigned LeadingBlanks = 0;
  assert(Indent >= -1 && "Indent must be >= -1 !");
  unsigned indent = static_cast<unsigned>(Indent + 1);
  trong_khi (Current != End) {
    nếu (*Current == '#')
      break;

    trong_khi (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      nếu (i == Current)
        break;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    nếu (!isBlankOrBreak(Current))
      break;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    trong_khi (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      nếu (i != Tmp) {
        nếu (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          trả_về false;
        }
        Tmp = i;
        ++Column;
      } không_thì {
        i = skip_b_break(Tmp);
        nếu (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++Line;
      }
    }

    nếu (!FlowLevel && Column < indent)
      break;

    Current = Tmp;
  }
  nếu (Start == Current) {
    setError("Got empty plain scalar", Start);
    trả_về false;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  // Plain scalars can be simple keys.
  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, false);

  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = false;

  trả_về true;
}

bool Scanner::scanAliasOrAnchor(bool IsAlias) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1);
  trong_khi (Current != End) {
    nếu (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      break;
    StringRef::iterator i = skip_ns_char(Current);
    nếu (i == Current)
      break;
    Current = i;
    ++Column;
  }

  nếu (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    trả_về false;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  // Alias and anchors can be simple keys.
  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, false);

  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = false;

  trả_về true;
}

bool Scanner::scanBlockScalarIndicators(char &StyleIndicator,
                                        char &ChompingIndicator,
                                        unsigned &IndentIndicator,
                                        bool &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  nếu (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    trả_về false;
  trả_về true;
}

char Scanner::scanBlockStyleIndicator() {
  char Indicator = ' ';
  nếu (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  trả_về Indicator;
}

char Scanner::scanBlockChompingIndicator() {
  char Indicator = ' ';
  nếu (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  trả_về Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
static unsigned getChompedLineBreaks(char ChompingIndicator,
                                     unsigned LineBreaks, StringRef Str) {
  nếu (ChompingIndicator == '-') // Strip all line breaks.
    trả_về 0;
  nếu (ChompingIndicator == '+') // Keep all line breaks.
    trả_về LineBreaks;
  // Clip trailing lines.
  trả_về Str.empty() ? 0 : 1;
}

unsigned Scanner::scanBlockIndentationIndicator() {
  unsigned Indent = 0;
  nếu (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = unsigned(*Current - '0');
    skip(1);
  }
  trả_về Indent;
}

bool Scanner::scanBlockScalarHeader(char &ChompingIndicator,
                                    unsigned &IndentIndicator, bool &IsDone) {
  auto Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  nếu (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  nếu (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.push_back(T);
    IsDone = true;
    trả_về true;
  }

  nếu (!consumeLineBreakIfPresent()) {
    setError("Expected a line break after block scalar header", Current);
    trả_về false;
  }
  trả_về true;
}

bool Scanner::findBlockScalarIndent(unsigned &BlockIndent,
                                    unsigned BlockExitIndent,
                                    unsigned &LineBreaks, bool &IsDone) {
  unsigned MaxAllSpaceLineCharacters = 0;
  StringRef::iterator LongestAllSpaceLine;

  trong_khi (true) {
    advanceWhile(&Scanner::skip_s_space);
    nếu (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      nếu (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = true;
        trả_về true;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      nếu (MaxAllSpaceLineCharacters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            LongestAllSpaceLine);
        trả_về false;
      }
      trả_về true;
    }
    nếu (skip_b_break(Current) != Current &&
        Column > MaxAllSpaceLineCharacters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      MaxAllSpaceLineCharacters = Column;
      LongestAllSpaceLine = Current;
    }

    // Check for EOF.
    nếu (Current == End) {
      IsDone = true;
      trả_về true;
    }

    nếu (!consumeLineBreakIfPresent()) {
      IsDone = true;
      trả_về true;
    }
    ++LineBreaks;
  }
  trả_về true;
}

bool Scanner::scanBlockScalarIndent(unsigned BlockIndent,
                                    unsigned BlockExitIndent, bool &IsDone) {
  // Skip the indentation.
  trong_khi (Column < BlockIndent) {
    auto I = skip_s_space(Current);
    nếu (I == Current)
      break;
    Current = I;
    ++Column;
  }

  nếu (skip_nb_char(Current) == Current)
    trả_về true;

  nếu (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = true;
    trả_về true;
  }

  nếu (Column < BlockIndent) {
    nếu (Current != End && *Current == '#') { // Trailing comment.
      IsDone = true;
      trả_về true;
    }
    setError("A text line is less indented than the block scalar", Current);
    trả_về false;
  }
  trả_về true; // A normal text line.
}

bool Scanner::scanBlockScalar(bool IsLiteral) {
  assert(*Current == '|' || *Current == '>');
  char StyleIndicator;
  char ChompingIndicator;
  unsigned BlockIndent;
  bool IsDone = false;
  nếu (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    trả_về false;
  nếu (IsDone)
    trả_về true;
  bool IsFolded = StyleIndicator == '>';

  const auto *Start = Current;
  unsigned BlockExitIndent = Indent < 0 ? 0 : (unsigned)Indent;
  unsigned LineBreaks = 0;
  nếu (BlockIndent == 0) {
    nếu (!findBlockScalarIndent(BlockIndent, BlockExitIndent, LineBreaks,
                               IsDone))
      trả_về false;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  trong_khi (!IsDone) {
    nếu (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      trả_về false;
    nếu (IsDone)
      break;

    // Parse the current line.
    auto LineStart = Current;
    advanceWhile(&Scanner::skip_nb_char);
    nếu (LineStart != Current) {
      nếu (LineBreaks && IsFolded && !Scanner::isLineEmpty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        nếu (LineBreaks == 1) {
          Str.append(LineBreaks,
                     isLineEmpty(StringRef(LineStart, Current - LineStart))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        LineBreaks--;
      }
      Str.append(LineBreaks, '\n');
      Str.append(StringRef(LineStart, Current - LineStart));
      LineBreaks = 0;
    }

    // Check for EOF.
    nếu (Current == End)
      break;

    nếu (!consumeLineBreakIfPresent())
      break;
    ++LineBreaks;
  }

  nếu (Current == End && !LineBreaks)
    // Ensure that there is at least one line break before the end of file.
    LineBreaks = 1;
  Str.append(getChompedLineBreaks(ChompingIndicator, LineBreaks, Str), '\n');

  // New lines may start a simple key.
  nếu (!FlowLevel)
    IsSimpleKeyAllowed = true;
  IsAdjacentValueAllowedInFlow = false;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.Value = std::string(Str);
  TokenQueue.push_back(T);
  trả_về true;
}

bool Scanner::scanTag() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1); // Eat !.
  nếu (Current == End || isBlankOrBreak(Current)); // An empty tag.
  không_thì nếu (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    nếu (!consume('>'))
      trả_về false;
  } không_thì {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.Kind = Token::TK_Tag;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  // Tags can be simple keys.
  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, false);

  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = false;

  trả_về true;
}

bool Scanner::fetchMoreTokens() {
  nếu (IsStartOfStream)
    trả_về scanStreamStart();

  scanToNextToken();

  nếu (Current == End)
    trả_về scanStreamEnd();

  removeStaleSimpleKeyCandidates();

  unrollIndent(Column);

  nếu (Column == 0 && *Current == '%')
    trả_về scanDirective();

  nếu (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    trả_về scanDocumentIndicator(true);

  nếu (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    trả_về scanDocumentIndicator(false);

  nếu (*Current == '[')
    trả_về scanFlowCollectionStart(true);

  nếu (*Current == '{')
    trả_về scanFlowCollectionStart(false);

  nếu (*Current == ']')
    trả_về scanFlowCollectionEnd(true);

  nếu (*Current == '}')
    trả_về scanFlowCollectionEnd(false);

  nếu (*Current == ',')
    trả_về scanFlowEntry();

  nếu (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    trả_về scanBlockEntry();

  nếu (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    trả_về scanKey();

  nếu (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || IsAdjacentValueAllowedInFlow))
    trả_về scanValue();

  nếu (*Current == '*')
    trả_về scanAliasOrAnchor(true);

  nếu (*Current == '&')
    trả_về scanAliasOrAnchor(false);

  nếu (*Current == '!')
    trả_về scanTag();

  nếu (*Current == '|' && !FlowLevel)
    trả_về scanBlockScalar(true);

  nếu (*Current == '>' && !FlowLevel)
    trả_về scanBlockScalar(false);

  nếu (*Current == '\'')
    trả_về scanFlowScalar(false);

  nếu (*Current == '"')
    trả_về scanFlowScalar(true);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  nếu ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    trả_về scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  trả_về false;
}

Stream::Stream(StringRef Input, SourceMgr &SM, bool ShowColors,
               std::error_code *EC)
    : scanner(new Scanner(Input, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef InputBuffer, SourceMgr &SM, bool ShowColors,
               std::error_code *EC)
    : scanner(new Scanner(InputBuffer, SM, ShowColors, EC)) {}

Stream::~Stream() = default;

bool Stream::failed() { trả_về scanner->failed(); }

void Stream::printError(Node *N, const Twine &Msg, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), Msg, Kind);
}

void Stream::printError(const SMRange &Range, const Twine &Msg,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, Msg, Range);
}

document_iterator Stream::begin() {
  nếu (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(new Document(*this));
  trả_về document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  trả_về document_iterator();
}

void Stream::skip() {
  cho (Document &Doc : *this)
    Doc.skip();
}

Node::Node(unsigned số_nguyên Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string Node::getVerbatimTag() const {
  StringRef Raw = getRawTag();
  nếu (!Raw.empty() && Raw != "!") {
    std::string Ret;
    nếu (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      trả_về Ret;
    } không_thì nếu (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      trả_về Ret;
    } không_thì {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      nếu (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      không_thì {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      trả_về Ret;
    }
  }

  switch (getType()) {
  case NK_Null:
    trả_về "tag:yaml.org,2002:null";
  case NK_Scalar:
  case NK_BlockScalar:
    // TODO: Tag resolution.
    trả_về "tag:yaml.org,2002:str";
  case NK_Mapping:
    trả_về "tag:yaml.org,2002:map";
  case NK_Sequence:
    trả_về "tag:yaml.org,2002:seq";
  }

  trả_về "";
}

Token &Node::peekNext() {
  trả_về Doc->peekNext();
}

Token Node::getNext() {
  trả_về Doc->getNext();
}

Node *Node::parseBlockNode() {
  trả_về Doc->parseBlockNode();
}

BumpPtrAllocator &Node::getAllocator() {
  trả_về Doc->NodeAllocator;
}

void Node::setError(const Twine &Msg, Token &Tok) const {
  Doc->setError(Msg, Tok);
}

bool Node::failed() const {
  trả_về Doc->failed();
}

StringRef ScalarNode::getValue(SmallVectorImpl<char> &Storage) const {
  nếu (Value[0] == '"')
    trả_về getDoubleQuotedValue(Value, Storage);
  nếu (Value[0] == '\'')
    trả_về getSingleQuotedValue(Value, Storage);
  trả_về getPlainValue(Value, Storage);
}

/// parseScalarValue - A common parsing routine for all flow scalar styles.
/// It handles line break characters by itself, adds regular content characters
/// to the result, and forwards escaped sequences to the provided routine for
/// the style-specific processing.
///
/// \param UnquotedValue - An input value without quotation marks.
/// \param Storage - A storage for the result if the input value is multiline or
/// contains escaped characters.
/// \param LookupChars - A set of special characters to search in the input
/// string. Should include line break characters and the escape character
/// specific for the processing scalar style, if any.
/// \param UnescapeCallback - This is called when the escape character is found
/// in the input.
/// \returns - The unfolded and unescaped value.
static StringRef
parseScalarValue(StringRef UnquotedValue, SmallVectorImpl<char> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<char> &)>
                     UnescapeCallback) {
  size_t I = UnquotedValue.find_first_of(LookupChars);
  nếu (I == StringRef::npos)
    trả_về UnquotedValue;

  Storage.clear();
  Storage.reserve(UnquotedValue.size());
  char LastNewLineAddedAs = '\0';
  cho (; I != StringRef::npos; I = UnquotedValue.find_first_of(LookupChars)) {
    nếu (UnquotedValue[I] != '\r' && UnquotedValue[I] != '\n') {
      llvm::append_range(Storage, UnquotedValue.take_front(I));
      UnquotedValue = UnescapeCallback(UnquotedValue.drop_front(I), Storage);
      LastNewLineAddedAs = '\0';
      continue;
    }
    nếu (size_t LastNonSWhite = UnquotedValue.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, UnquotedValue.take_front(LastNonSWhite + 1));
      Storage.push_back(' ');
      LastNewLineAddedAs = ' ';
    } không_thì {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      switch (LastNewLineAddedAs) {
      case ' ':
        assert(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        LastNewLineAddedAs = '\n';
        break;
      case '\n':
        assert(!Storage.empty() && Storage.back() == '\n');
        Storage.push_back('\n');
        break;
      default:
        Storage.push_back(' ');
        LastNewLineAddedAs = ' ';
        break;
      }
    }
    // Handle Windows-style EOL
    nếu (UnquotedValue.substr(I, 2) == "\r\n")
      I++;
    UnquotedValue = UnquotedValue.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, UnquotedValue);
  trả_về StringRef(Storage.begin(), Storage.size());
}

StringRef
ScalarNode::getDoubleQuotedValue(StringRef RawValue,
                                 SmallVectorImpl<char> &Storage) const {
  assert(RawValue.size() >= 2 && RawValue.front() == '"' &&
         RawValue.back() == '"');
  StringRef UnquotedValue = RawValue.substr(1, RawValue.size() - 2);

  auto UnescapeFunc = [this](StringRef UnquotedValue,
                             SmallVectorImpl<char> &Storage) {
    assert(UnquotedValue.take_front(1) == "\\");
    nếu (UnquotedValue.size() == 1) {
      Token T;
      T.Range = UnquotedValue;
      setError("Unrecognized escape code", T);
      Storage.clear();
      trả_về StringRef();
    }
    UnquotedValue = UnquotedValue.drop_front(1);
    switch (UnquotedValue[0]) {
    default: {
      Token T;
      T.Range = UnquotedValue.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      trả_về StringRef();
    }
    case '\r':
      // Shrink the Windows-style EOL.
      nếu (UnquotedValue.size() >= 2 && UnquotedValue[1] == '\n')
        UnquotedValue = UnquotedValue.drop_front(1);
      [[fallthrough]];
    case '\n':
      trả_về UnquotedValue.drop_front(1).ltrim(" \t");
    case '0':
      Storage.push_back(0x00);
      break;
    case 'a':
      Storage.push_back(0x07);
      break;
    case 'b':
      Storage.push_back(0x08);
      break;
    case 't':
    case 0x09:
      Storage.push_back(0x09);
      break;
    case 'n':
      Storage.push_back(0x0A);
      break;
    case 'v':
      Storage.push_back(0x0B);
      break;
    case 'f':
      Storage.push_back(0x0C);
      break;
    case 'r':
      Storage.push_back(0x0D);
      break;
    case 'e':
      Storage.push_back(0x1B);
      break;
    case ' ':
      Storage.push_back(0x20);
      break;
    case '"':
      Storage.push_back(0x22);
      break;
    case '/':
      Storage.push_back(0x2F);
      break;
    case '\\':
      Storage.push_back(0x5C);
      break;
    case 'N':
      encodeUTF8(0x85, Storage);
      break;
    case '_':
      encodeUTF8(0xA0, Storage);
      break;
    case 'L':
      encodeUTF8(0x2028, Storage);
      break;
    case 'P':
      encodeUTF8(0x2029, Storage);
      break;
    case 'x': {
      nếu (UnquotedValue.size() < 3)
        // TODO: Report error.
        break;
      unsigned số_nguyên UnicodeScalarValue;
      nếu (UnquotedValue.substr(1, 2).getAsInteger(16, UnicodeScalarValue))
        // TODO: Report error.
        UnicodeScalarValue = 0xFFFD;
      encodeUTF8(UnicodeScalarValue, Storage);
      trả_về UnquotedValue.drop_front(3);
    }
    case 'u': {
      nếu (UnquotedValue.size() < 5)
        // TODO: Report error.
        break;
      unsigned số_nguyên UnicodeScalarValue;
      nếu (UnquotedValue.substr(1, 4).getAsInteger(16, UnicodeScalarValue))
        // TODO: Report error.
        UnicodeScalarValue = 0xFFFD;
      encodeUTF8(UnicodeScalarValue, Storage);
      trả_về UnquotedValue.drop_front(5);
    }
    case 'U': {
      nếu (UnquotedValue.size() < 9)
        // TODO: Report error.
        break;
      unsigned số_nguyên UnicodeScalarValue;
      nếu (UnquotedValue.substr(1, 8).getAsInteger(16, UnicodeScalarValue))
        // TODO: Report error.
        UnicodeScalarValue = 0xFFFD;
      encodeUTF8(UnicodeScalarValue, Storage);
      trả_về UnquotedValue.drop_front(9);
    }
    }
    trả_về UnquotedValue.drop_front(1);
  };

  trả_về parseScalarValue(UnquotedValue, Storage, "\\\r\n", UnescapeFunc);
}

StringRef ScalarNode::getSingleQuotedValue(StringRef RawValue,
                                           SmallVectorImpl<char> &Storage) {
  assert(RawValue.size() >= 2 && RawValue.front() == '\'' &&
         RawValue.back() == '\'');
  StringRef UnquotedValue = RawValue.substr(1, RawValue.size() - 2);

  auto UnescapeFunc = [](StringRef UnquotedValue,
                         SmallVectorImpl<char> &Storage) {
    assert(UnquotedValue.take_front(2) == "''");
    Storage.push_back('\'');
    trả_về UnquotedValue.drop_front(2);
  };

  trả_về parseScalarValue(UnquotedValue, Storage, "'\r\n", UnescapeFunc);
}

StringRef ScalarNode::getPlainValue(StringRef RawValue,
                                    SmallVectorImpl<char> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  RawValue = RawValue.rtrim("\r\n \t");
  trả_về parseScalarValue(RawValue, Storage, "\r\n", nullptr);
}

Node *KeyValueNode::getKey() {
  nếu (Key)
    trả_về Key;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    nếu (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_Value
        || t.Kind == Token::TK_Error) {
      trả_về Key = new (getAllocator()) NullNode(Doc);
    }
    nếu (t.Kind == Token::TK_Key)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  nếu (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_Value) {
    trả_về Key = new (getAllocator()) NullNode(Doc);
  }

  // We've got a normal key.
  trả_về Key = parseBlockNode();
}

Node *KeyValueNode::getValue() {
  nếu (Value)
    trả_về Value;

  nếu (Node* Key = getKey())
    Key->skip();
  không_thì {
    setError("Null key in Key Value.", peekNext());
    trả_về Value = new (getAllocator()) NullNode(Doc);
  }

  nếu (failed())
    trả_về Value = new (getAllocator()) NullNode(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    nếu (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_Key
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      trả_về Value = new (getAllocator()) NullNode(Doc);
    }

    nếu (t.Kind != Token::TK_Value) {
      setError("Unexpected token in Key Value.", t);
      trả_về Value = new (getAllocator()) NullNode(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  nếu (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_Key) {
    trả_về Value = new (getAllocator()) NullNode(Doc);
  }

  // We got a normal value.
  trả_về Value = parseBlockNode();
}

void MappingNode::increment() {
  nếu (failed()) {
    IsAtEnd = true;
    CurrentEntry = nullptr;
    trả_về;
  }
  nếu (CurrentEntry) {
    CurrentEntry->skip();
    nếu (Type == MT_Inline) {
      IsAtEnd = true;
      CurrentEntry = nullptr;
      trả_về;
    }
  }
  Token T = peekNext();
  nếu (T.Kind == Token::TK_Key || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = new (getAllocator()) KeyValueNode(Doc);
  } không_thì nếu (Type == MT_Block) {
    switch (T.Kind) {
    case Token::TK_BlockEnd:
      getNext();
      IsAtEnd = true;
      CurrentEntry = nullptr;
      break;
    default:
      setError("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    case Token::TK_Error:
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  } không_thì {
    switch (T.Kind) {
    case Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      trả_về increment();
    case Token::TK_FlowMappingEnd:
      getNext();
      [[fallthrough]];
    case Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = true;
      CurrentEntry = nullptr;
      break;
    default:
      setError( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  }
}

void SequenceNode::increment() {
  nếu (failed()) {
    IsAtEnd = true;
    CurrentEntry = nullptr;
    trả_về;
  }
  nếu (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  nếu (SeqType == ST_Block) {
    switch (T.Kind) {
    case Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parseBlockNode();
      nếu (!CurrentEntry) { // An error occurred.
        IsAtEnd = true;
        CurrentEntry = nullptr;
      }
      break;
    case Token::TK_BlockEnd:
      getNext();
      IsAtEnd = true;
      CurrentEntry = nullptr;
      break;
    default:
      setError( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    case Token::TK_Error:
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  } không_thì nếu (SeqType == ST_Indentless) {
    switch (T.Kind) {
    case Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parseBlockNode();
      nếu (!CurrentEntry) { // An error occurred.
        IsAtEnd = true;
        CurrentEntry = nullptr;
      }
      break;
    default:
    case Token::TK_Error:
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  } không_thì nếu (SeqType == ST_Flow) {
    switch (T.Kind) {
    case Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      WasPreviousTokenFlowEntry = true;
      trả_về increment();
    case Token::TK_FlowSequenceEnd:
      getNext();
      [[fallthrough]];
    case Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = true;
      CurrentEntry = nullptr;
      break;
    case Token::TK_StreamEnd:
    case Token::TK_DocumentEnd:
    case Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = true;
      CurrentEntry = nullptr;
      break;
    default:
      nếu (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = true;
        CurrentEntry = nullptr;
        break;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parseBlockNode();
      nếu (!CurrentEntry) {
        IsAtEnd = true;
      }
      WasPreviousTokenFlowEntry = false;
      break;
    }
  }
}

Document::Document(Stream &S) : stream(S), Root(nullptr) {
  // Tag maps starts with two default mappings.
  TagMap["!"] = "!";
  TagMap["!!"] = "tag:yaml.org,2002:";

  nếu (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peekNext();
  nếu (T.Kind == Token::TK_DocumentStart)
    getNext();
}

bool Document::skip()  {
  nếu (stream.scanner->failed())
    trả_về false;
  nếu (!Root && !getRoot())
    trả_về false;
  Root->skip();
  Token &T = peekNext();
  nếu (T.Kind == Token::TK_StreamEnd)
    trả_về false;
  nếu (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    trả_về skip();
  }
  trả_về true;
}

Token &Document::peekNext() {
  trả_về stream.scanner->peekNext();
}

Token Document::getNext() {
  trả_về stream.scanner->getNext();
}

void Document::setError(const Twine &Message, Token &Location) const {
  stream.scanner->setError(Message, Location.Range.begin());
}

bool Document::failed() const {
  trả_về stream.scanner->failed();
}

Node *Document::parseBlockNode() {
  Token T = peekNext();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  switch (T.Kind) {
  case Token::TK_Alias:
    getNext();
    trả_về new (NodeAllocator) AliasNode(stream.CurrentDoc, T.Range.substr(1));
  case Token::TK_Anchor:
    nếu (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      trả_về nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    goto parse_property;
  case Token::TK_Tag:
    nếu (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      trả_về nullptr;
    }
    TagInfo = getNext(); // Consume TK_Tag.
    T = peekNext();
    goto parse_property;
  default:
    break;
  }

  switch (T.Kind) {
  case Token::TK_BlockEntry:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    trả_về new (NodeAllocator) SequenceNode( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , SequenceNode::ST_Indentless);
  case Token::TK_BlockSequenceStart:
    getNext();
    trả_về new (NodeAllocator)
      SequenceNode( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , SequenceNode::ST_Block);
  case Token::TK_BlockMappingStart:
    getNext();
    trả_về new (NodeAllocator)
      MappingNode( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , MappingNode::MT_Block);
  case Token::TK_FlowSequenceStart:
    getNext();
    trả_về new (NodeAllocator)
      SequenceNode( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , SequenceNode::ST_Flow);
  case Token::TK_FlowMappingStart:
    getNext();
    trả_về new (NodeAllocator)
      MappingNode( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , MappingNode::MT_Flow);
  case Token::TK_Scalar:
    getNext();
    trả_về new (NodeAllocator)
      ScalarNode( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  case Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.Value.c_str(), T.Value.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(NodeAllocator).drop_back();
    trả_về new (NodeAllocator)
        BlockScalarNode(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  case Token::TK_Key:
    // Don't eat the TK_Key, KeyValueNode expects it.
    trả_về new (NodeAllocator)
      MappingNode( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , MappingNode::MT_Inline);
  case Token::TK_DocumentStart:
  case Token::TK_DocumentEnd:
  case Token::TK_StreamEnd:
  default:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    trả_về new (NodeAllocator) NullNode(stream.CurrentDoc);
  case Token::TK_FlowMappingEnd:
  case Token::TK_FlowSequenceEnd:
  case Token::TK_FlowEntry: {
    nếu (Root && (isa<MappingNode>(Root) || isa<SequenceNode>(Root)))
      trả_về new (NodeAllocator) NullNode(stream.CurrentDoc);

    setError("Unexpected token", T);
    trả_về nullptr;
  }
  case Token::TK_Error:
    trả_về nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  trả_về nullptr;
}

bool Document::parseDirectives() {
  bool isDirective = false;
  trong_khi (true) {
    Token T = peekNext();
    nếu (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = true;
    } không_thì nếu (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = true;
    } không_thì {
      break;
    }
  }
  trả_về isDirective;
}

void Document::parseYAMLDirective() {
  getNext(); // Eat %YAML <version>
}

void Document::parseTAGDirective() {
  Token Tag = getNext(); // %TAG <handle> <prefix>
  StringRef T = Tag.Range;
  // Strip %TAG
  T = T.substr(T.find_first_of(" \t")).ltrim(" \t");
  std::size_t HandleEnd = T.find_first_of(" \t");
  StringRef TagHandle = T.substr(0, HandleEnd);
  StringRef TagPrefix = T.substr(HandleEnd).ltrim(" \t");
  TagMap[TagHandle] = TagPrefix;
}

bool Document::expectToken(số_nguyên TK) {
  Token T = getNext();
  nếu (T.Kind != TK) {
    setError("Unexpected token", T);
    trả_về false;
  }
  trả_về true;
}
