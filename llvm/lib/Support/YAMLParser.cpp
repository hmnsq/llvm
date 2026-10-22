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
  如果 (Input.empty())
    返回 {UEF_Unknown, 0};

  switch (uint8_t(Input[0])) {
  case 0x00:
    如果 (Input.size() >= 4) {
      如果 (  Input[1] == 0
         && uint8_t(Input[2]) == 0xFE
         && uint8_t(Input[3]) == 0xFF)
        返回 {UEF_UTF32_BE, 4};
      如果 (Input[1] == 0 && Input[2] == 0 && Input[3] != 0)
        返回 {UEF_UTF32_BE, 0};
    }

    如果 (Input.size() >= 2 && Input[1] != 0)
      返回 {UEF_UTF16_BE, 0};
    返回 {UEF_Unknown, 0};
  case 0xFF:
    如果 (  Input.size() >= 4
       && uint8_t(Input[1]) == 0xFE
       && Input[2] == 0
       && Input[3] == 0)
      返回 {UEF_UTF32_LE, 4};

    如果 (Input.size() >= 2 && uint8_t(Input[1]) == 0xFE)
      返回 {UEF_UTF16_LE, 2};
    返回 {UEF_Unknown, 0};
  case 0xFE:
    如果 (Input.size() >= 2 && uint8_t(Input[1]) == 0xFF)
      返回 {UEF_UTF16_BE, 2};
    返回 {UEF_Unknown, 0};
  case 0xEF:
    如果 (  Input.size() >= 3
       && uint8_t(Input[1]) == 0xBB
       && uint8_t(Input[2]) == 0xBF)
      返回 {UEF_UTF8, 3};
    返回 {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  如果 (Input.size() >= 4 && Input[1] == 0 && Input[2] == 0 && Input[3] == 0)
    返回 {UEF_UTF32_LE, 0};

  如果 (Input.size() >= 2 && Input[1] == 0)
    返回 {UEF_UTF16_LE, 0};

  返回 {UEF_UTF8, 0};
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
    返回 Tok == Other.Tok;
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
  如果 (Position < End && (*Position & 0x80) == 0) {
    返回 {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  如果 (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    如果 (codepoint >= 0x80)
      返回 {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  如果 (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    如果 (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      返回 {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  如果 (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    如果 (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      返回 {codepoint, 4};
  }
  返回 {0, 0};
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
    如果 (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    如果 (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    如果 (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = true;
  }

  /// Returns true if an error occurred while parsing.
  bool failed() {
    返回 Failed;
  }

private:
  void init(MemoryBufferRef Buffer);

  StringRef currentInput() {
    返回 StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    返回 ::decodeUTF8(StringRef(Position, End - Position));
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
  bool unrollIndent(整数 ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  bool rollIndent( 整数 ToColumn
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
  整数 Indent;

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
  SmallVector<整数, 4> Indents;

  /// Potential simple keys.
  SmallVector<SimpleKey, 4> SimpleKeys;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
static void encodeUTF8( uint32_t UnicodeScalarValue
                      , SmallVectorImpl<char> &Result) {
  如果 (UnicodeScalarValue <= 0x7F) {
    Result.push_back(UnicodeScalarValue & 0x7F);
  } 否则 如果 (UnicodeScalarValue <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((UnicodeScalarValue & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (UnicodeScalarValue & 0x3F);
    Result.push_back(FirstByte);
    Result.push_back(SecondByte);
  } 否则 如果 (UnicodeScalarValue <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((UnicodeScalarValue & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((UnicodeScalarValue & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (UnicodeScalarValue & 0x3F);
    Result.push_back(FirstByte);
    Result.push_back(SecondByte);
    Result.push_back(ThirdByte);
  } 否则 如果 (UnicodeScalarValue <= 0x10FFFF) {
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
  当 (true) {
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
    如果 (T.Kind == Token::TK_StreamEnd)
      break;
    否则 如果 (T.Kind == Token::TK_Error)
      返回 false;
  }
  返回 true;
}

bool yaml::scanTokens(StringRef Input) {
  SourceMgr SM;
  Scanner scanner(Input, SM);
  当 (true) {
    Token T = scanner.getNext();
    如果 (T.Kind == Token::TK_StreamEnd)
      break;
    否则 如果 (T.Kind == Token::TK_Error)
      返回 false;
  }
  返回 true;
}

std::string yaml::escape(StringRef Input, bool EscapePrintable) {
  std::string EscapedInput;
  为 (StringRef::iterator i = Input.begin(), e = Input.end(); i != e; ++i) {
    如果 (*i == '\\')
      EscapedInput += "\\\\";
    否则 如果 (*i == '"')
      EscapedInput += "\\\"";
    否则 如果 (*i == 0)
      EscapedInput += "\\0";
    否则 如果 (*i == 0x07)
      EscapedInput += "\\a";
    否则 如果 (*i == 0x08)
      EscapedInput += "\\b";
    否则 如果 (*i == 0x09)
      EscapedInput += "\\t";
    否则 如果 (*i == 0x0A)
      EscapedInput += "\\n";
    否则 如果 (*i == 0x0B)
      EscapedInput += "\\v";
    否则 如果 (*i == 0x0C)
      EscapedInput += "\\f";
    否则 如果 (*i == 0x0D)
      EscapedInput += "\\r";
    否则 如果 (*i == 0x1B)
      EscapedInput += "\\e";
    否则 如果 ((unsigned char)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      EscapedInput += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } 否则 如果 (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded UnicodeScalarValue
        = decodeUTF8(StringRef(i, Input.end() - i));
      如果 (UnicodeScalarValue.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(EscapedInput, Val);
        // FIXME: Error reporting.
        返回 EscapedInput;
      }
      如果 (UnicodeScalarValue.first == 0x85)
        EscapedInput += "\\N";
      否则 如果 (UnicodeScalarValue.first == 0xA0)
        EscapedInput += "\\_";
      否则 如果 (UnicodeScalarValue.first == 0x2028)
        EscapedInput += "\\L";
      否则 如果 (UnicodeScalarValue.first == 0x2029)
        EscapedInput += "\\P";
      否则 如果 (!EscapePrintable &&
               sys::unicode::isPrintable(UnicodeScalarValue.first))
        EscapedInput += StringRef(i, UnicodeScalarValue.second);
      否则 {
        std::string HexStr = utohexstr(UnicodeScalarValue.first);
        如果 (HexStr.size() <= 2)
          EscapedInput += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        否则 如果 (HexStr.size() <= 4)
          EscapedInput += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        否则 如果 (HexStr.size() <= 8)
          EscapedInput += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += UnicodeScalarValue.second - 1;
    } 否则 {
      EscapedInput.push_back(*i);
    }
  }
  返回 EscapedInput;
}

std::optional<bool> yaml::parseBool(StringRef S) {
  switch (S.size()) {
  case 1:
    switch (S.front()) {
    case 'y':
    case 'Y':
      返回 true;
    case 'n':
    case 'N':
      返回 false;
    default:
      返回 std::nullopt;
    }
  case 2:
    switch (S.front()) {
    case 'O':
      如果 (S[1] == 'N') // ON
        返回 true;
      [[fallthrough]];
    case 'o':
      如果 (S[1] == 'n') //[Oo]n
        返回 true;
      返回 std::nullopt;
    case 'N':
      如果 (S[1] == 'O') // NO
        返回 false;
      [[fallthrough]];
    case 'n':
      如果 (S[1] == 'o') //[Nn]o
        返回 false;
      返回 std::nullopt;
    default:
      返回 std::nullopt;
    }
  case 3:
    switch (S.front()) {
    case 'O':
      如果 (S.drop_front() == "FF") // OFF
        返回 false;
      [[fallthrough]];
    case 'o':
      如果 (S.drop_front() == "ff") //[Oo]ff
        返回 false;
      返回 std::nullopt;
    case 'Y':
      如果 (S.drop_front() == "ES") // YES
        返回 true;
      [[fallthrough]];
    case 'y':
      如果 (S.drop_front() == "es") //[Yy]es
        返回 true;
      返回 std::nullopt;
    default:
      返回 std::nullopt;
    }
  case 4:
    switch (S.front()) {
    case 'T':
      如果 (S.drop_front() == "RUE") // TRUE
        返回 true;
      [[fallthrough]];
    case 't':
      如果 (S.drop_front() == "rue") //[Tt]rue
        返回 true;
      返回 std::nullopt;
    default:
      返回 std::nullopt;
    }
  case 5:
    switch (S.front()) {
    case 'F':
      如果 (S.drop_front() == "ALSE") // FALSE
        返回 false;
      [[fallthrough]];
    case 'f':
      如果 (S.drop_front() == "alse") //[Ff]alse
        返回 false;
      返回 std::nullopt;
    default:
      返回 std::nullopt;
    }
  default:
    返回 std::nullopt;
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
  当 (true) {
    如果 (TokenQueue.empty() || NeedMore) {
      如果 (!fetchMoreTokens()) {
        TokenQueue.clear();
        SimpleKeys.clear();
        TokenQueue.push_back(Token());
        返回 TokenQueue.front();
      }
    }
    assert(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    removeStaleSimpleKeyCandidates();
    SimpleKey SK;
    SK.Tok = TokenQueue.begin();
    如果 (!is_contained(SimpleKeys, SK))
      break;
    否则
      NeedMore = true;
  }
  返回 TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  如果 (!TokenQueue.empty())
    TokenQueue.pop_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  如果 (TokenQueue.empty())
    TokenQueue.resetAlloc();

  返回 Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  如果 (Position == End)
    返回 Position;
  // Check 7 bit c-printable - b-char.
  如果 (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    返回 Position + 1;

  // Check for valid UTF-8.
  如果 (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    如果 (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      返回 Position + u8d.second;
  }
  返回 Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  如果 (Position == End)
    返回 Position;
  如果 (*Position == 0x0D) {
    如果 (Position + 1 != End && *(Position + 1) == 0x0A)
      返回 Position + 2;
    返回 Position + 1;
  }

  如果 (*Position == 0x0A)
    返回 Position + 1;
  返回 Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  如果 (Position == End)
    返回 Position;
  如果 (*Position == ' ')
    返回 Position + 1;
  返回 Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  如果 (Position == End)
    返回 Position;
  如果 (*Position == ' ' || *Position == '\t')
    返回 Position + 1;
  返回 Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  如果 (Position == End)
    返回 Position;
  如果 (*Position == ' ' || *Position == '\t')
    返回 Position;
  返回 skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  当 (true) {
    StringRef::iterator i = (this->*Func)(Position);
    如果 (i == Position)
      break;
    Position = i;
  }
  返回 Position;
}

void Scanner::advanceWhile(SkipWhileFunc Func) {
  auto Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

static bool is_ns_hex_digit(const char C) { 返回 isAlnum(C); }

static bool is_ns_word_char(const char C) { 返回 C == '-' || isAlpha(C); }

void Scanner::scan_ns_uri_char() {
  当 (true) {
    如果 (Current == End)
      break;
    如果 ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } 否则 {
      break;
    }
  }
}

bool Scanner::consume(uint32_t Expected) {
  如果 (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    返回 false;
  }
  如果 (Current == End)
    返回 false;
  如果 (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    返回 false;
  }
  如果 (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    返回 true;
  }
  返回 false;
}

void Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  assert(Current <= End && "Skipped past the end");
}

bool Scanner::isBlankOrBreak(StringRef::iterator Position) {
  如果 (Position == End)
    返回 false;
  返回 *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

bool Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  如果 (Position == End || isBlankOrBreak(Position))
    返回 false;
  如果 (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    返回 false;
  返回 true;
}

bool Scanner::isLineEmpty(StringRef Line) {
  为 (const auto *Position = Line.begin(); Position != Line.end(); ++Position)
    如果 (!isBlankOrBreak(Position))
      返回 false;
  返回 true;
}

bool Scanner::consumeLineBreakIfPresent() {
  auto Next = skip_b_break(Current);
  如果 (Next == Current)
    返回 false;
  Column = 0;
  ++Line;
  Current = Next;
  返回 true;
}

void Scanner::saveSimpleKeyCandidate( TokenQueueT::iterator Tok
                                    , unsigned AtColumn
                                    , bool IsRequired) {
  如果 (IsSimpleKeyAllowed) {
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
  为 (SmallVectorImpl<SimpleKey>::iterator i = SimpleKeys.begin();
                                            i != SimpleKeys.end();) {
    如果 (i->Line != Line || i->Column + 1024 < Column) {
      如果 (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = SimpleKeys.erase(i);
    } 否则 {
      ++i;
    }
  }
}

void Scanner::removeSimpleKeyCandidatesOnFlowLevel(unsigned Level) {
  如果 (!SimpleKeys.empty() && (SimpleKeys.end() - 1)->FlowLevel == Level)
    SimpleKeys.pop_back();
}

bool Scanner::unrollIndent(整数 ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  如果 (FlowLevel != 0)
    返回 true;

  当 (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.push_back(T);
    Indent = Indents.pop_back_val();
  }

  返回 true;
}

bool Scanner::rollIndent( 整数 ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator InsertPoint) {
  如果 (FlowLevel)
    返回 true;
  如果 (Indent < ToColumn) {
    Indents.push_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.insert(InsertPoint, T);
  }
  返回 true;
}

void Scanner::skipComment() {
  如果 (Current == End || *Current != '#')
    返回;
  当 (true) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    如果 (I == Current)
      break;
    Current = I;
    ++Column;
  }
}

void Scanner::scanToNextToken() {
  当 (true) {
    当 (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    如果 (i == Current)
      break;
    Current = i;
    ++Line;
    Column = 0;
    // New lines may start a simple key.
    如果 (!FlowLevel)
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
  返回 true;
}

bool Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  如果 (Column != 0) {
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
  返回 true;
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
  如果 (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.push_back(T);
    返回 true;
  } 否则 如果(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.push_back(T);
    返回 true;
  }
  返回 false;
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
  返回 true;
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
  返回 true;
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
  如果 (FlowLevel)
    --FlowLevel;
  返回 true;
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
  返回 true;
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
  返回 true;
}

bool Scanner::scanKey() {
  如果 (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  removeSimpleKeyCandidatesOnFlowLevel(FlowLevel);
  IsSimpleKeyAllowed = !FlowLevel;
  IsAdjacentValueAllowedInFlow = false;

  Token T;
  T.Kind = Token::TK_Key;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);
  返回 true;
}

bool Scanner::scanValue() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  如果 (!SimpleKeys.empty()) {
    SimpleKey SK = SimpleKeys.pop_back_val();
    Token T;
    T.Kind = Token::TK_Key;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    为 (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      如果 (i == SK.Tok)
        break;
    }
    如果 (i == e) {
      Failed = true;
      返回 false;
    }
    i = TokenQueue.insert(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    IsSimpleKeyAllowed = false;
  } 否则 {
    如果 (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    IsSimpleKeyAllowed = !FlowLevel;
  }
  IsAdjacentValueAllowedInFlow = false;

  Token T;
  T.Kind = Token::TK_Value;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);
  返回 true;
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
  当 (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  返回 (Position - 1 - I) % 2 == 1;
}

bool Scanner::scanFlowScalar(bool IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  如果 (IsDoubleQuoted) {
    do {
      ++Current;
      当 (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } 当 (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } 否则 {
    skip(1);
    当 (Current != End) {
      // Skip a ' followed by another '.
      如果 (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        continue;
      } 否则 如果 (*Current == '\'')
        break;
      StringRef::iterator i = skip_nb_char(Current);
      如果 (i == Current) {
        i = skip_b_break(Current);
        如果 (i == Current)
          break;
        Current = i;
        Column = 0;
        ++Line;
      } 否则 {
        如果 (i == End)
          break;
        Current = i;
        ++Column;
      }
    }
  }

  如果 (Current == End) {
    setError("Expected quote at end of scalar", Current);
    返回 false;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, false);

  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = true;

  返回 true;
}

bool Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  unsigned LeadingBlanks = 0;
  assert(Indent >= -1 && "Indent must be >= -1 !");
  unsigned indent = static_cast<unsigned>(Indent + 1);
  当 (Current != End) {
    如果 (*Current == '#')
      break;

    当 (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      如果 (i == Current)
        break;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    如果 (!isBlankOrBreak(Current))
      break;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    当 (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      如果 (i != Tmp) {
        如果 (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          返回 false;
        }
        Tmp = i;
        ++Column;
      } 否则 {
        i = skip_b_break(Tmp);
        如果 (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++Line;
      }
    }

    如果 (!FlowLevel && Column < indent)
      break;

    Current = Tmp;
  }
  如果 (Start == Current) {
    setError("Got empty plain scalar", Start);
    返回 false;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  // Plain scalars can be simple keys.
  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, false);

  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = false;

  返回 true;
}

bool Scanner::scanAliasOrAnchor(bool IsAlias) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1);
  当 (Current != End) {
    如果 (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      break;
    StringRef::iterator i = skip_ns_char(Current);
    如果 (i == Current)
      break;
    Current = i;
    ++Column;
  }

  如果 (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    返回 false;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  // Alias and anchors can be simple keys.
  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, false);

  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = false;

  返回 true;
}

bool Scanner::scanBlockScalarIndicators(char &StyleIndicator,
                                        char &ChompingIndicator,
                                        unsigned &IndentIndicator,
                                        bool &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  如果 (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    返回 false;
  返回 true;
}

char Scanner::scanBlockStyleIndicator() {
  char Indicator = ' ';
  如果 (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  返回 Indicator;
}

char Scanner::scanBlockChompingIndicator() {
  char Indicator = ' ';
  如果 (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  返回 Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
static unsigned getChompedLineBreaks(char ChompingIndicator,
                                     unsigned LineBreaks, StringRef Str) {
  如果 (ChompingIndicator == '-') // Strip all line breaks.
    返回 0;
  如果 (ChompingIndicator == '+') // Keep all line breaks.
    返回 LineBreaks;
  // Clip trailing lines.
  返回 Str.empty() ? 0 : 1;
}

unsigned Scanner::scanBlockIndentationIndicator() {
  unsigned Indent = 0;
  如果 (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = unsigned(*Current - '0');
    skip(1);
  }
  返回 Indent;
}

bool Scanner::scanBlockScalarHeader(char &ChompingIndicator,
                                    unsigned &IndentIndicator, bool &IsDone) {
  auto Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  如果 (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  如果 (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.push_back(T);
    IsDone = true;
    返回 true;
  }

  如果 (!consumeLineBreakIfPresent()) {
    setError("Expected a line break after block scalar header", Current);
    返回 false;
  }
  返回 true;
}

bool Scanner::findBlockScalarIndent(unsigned &BlockIndent,
                                    unsigned BlockExitIndent,
                                    unsigned &LineBreaks, bool &IsDone) {
  unsigned MaxAllSpaceLineCharacters = 0;
  StringRef::iterator LongestAllSpaceLine;

  当 (true) {
    advanceWhile(&Scanner::skip_s_space);
    如果 (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      如果 (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = true;
        返回 true;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      如果 (MaxAllSpaceLineCharacters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            LongestAllSpaceLine);
        返回 false;
      }
      返回 true;
    }
    如果 (skip_b_break(Current) != Current &&
        Column > MaxAllSpaceLineCharacters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      MaxAllSpaceLineCharacters = Column;
      LongestAllSpaceLine = Current;
    }

    // Check for EOF.
    如果 (Current == End) {
      IsDone = true;
      返回 true;
    }

    如果 (!consumeLineBreakIfPresent()) {
      IsDone = true;
      返回 true;
    }
    ++LineBreaks;
  }
  返回 true;
}

bool Scanner::scanBlockScalarIndent(unsigned BlockIndent,
                                    unsigned BlockExitIndent, bool &IsDone) {
  // Skip the indentation.
  当 (Column < BlockIndent) {
    auto I = skip_s_space(Current);
    如果 (I == Current)
      break;
    Current = I;
    ++Column;
  }

  如果 (skip_nb_char(Current) == Current)
    返回 true;

  如果 (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = true;
    返回 true;
  }

  如果 (Column < BlockIndent) {
    如果 (Current != End && *Current == '#') { // Trailing comment.
      IsDone = true;
      返回 true;
    }
    setError("A text line is less indented than the block scalar", Current);
    返回 false;
  }
  返回 true; // A normal text line.
}

bool Scanner::scanBlockScalar(bool IsLiteral) {
  assert(*Current == '|' || *Current == '>');
  char StyleIndicator;
  char ChompingIndicator;
  unsigned BlockIndent;
  bool IsDone = false;
  如果 (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    返回 false;
  如果 (IsDone)
    返回 true;
  bool IsFolded = StyleIndicator == '>';

  const auto *Start = Current;
  unsigned BlockExitIndent = Indent < 0 ? 0 : (unsigned)Indent;
  unsigned LineBreaks = 0;
  如果 (BlockIndent == 0) {
    如果 (!findBlockScalarIndent(BlockIndent, BlockExitIndent, LineBreaks,
                               IsDone))
      返回 false;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  当 (!IsDone) {
    如果 (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      返回 false;
    如果 (IsDone)
      break;

    // Parse the current line.
    auto LineStart = Current;
    advanceWhile(&Scanner::skip_nb_char);
    如果 (LineStart != Current) {
      如果 (LineBreaks && IsFolded && !Scanner::isLineEmpty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        如果 (LineBreaks == 1) {
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
    如果 (Current == End)
      break;

    如果 (!consumeLineBreakIfPresent())
      break;
    ++LineBreaks;
  }

  如果 (Current == End && !LineBreaks)
    // Ensure that there is at least one line break before the end of file.
    LineBreaks = 1;
  Str.append(getChompedLineBreaks(ChompingIndicator, LineBreaks, Str), '\n');

  // New lines may start a simple key.
  如果 (!FlowLevel)
    IsSimpleKeyAllowed = true;
  IsAdjacentValueAllowedInFlow = false;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.Value = std::string(Str);
  TokenQueue.push_back(T);
  返回 true;
}

bool Scanner::scanTag() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1); // Eat !.
  如果 (Current == End || isBlankOrBreak(Current)); // An empty tag.
  否则 如果 (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    如果 (!consume('>'))
      返回 false;
  } 否则 {
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

  返回 true;
}

bool Scanner::fetchMoreTokens() {
  如果 (IsStartOfStream)
    返回 scanStreamStart();

  scanToNextToken();

  如果 (Current == End)
    返回 scanStreamEnd();

  removeStaleSimpleKeyCandidates();

  unrollIndent(Column);

  如果 (Column == 0 && *Current == '%')
    返回 scanDirective();

  如果 (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    返回 scanDocumentIndicator(true);

  如果 (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    返回 scanDocumentIndicator(false);

  如果 (*Current == '[')
    返回 scanFlowCollectionStart(true);

  如果 (*Current == '{')
    返回 scanFlowCollectionStart(false);

  如果 (*Current == ']')
    返回 scanFlowCollectionEnd(true);

  如果 (*Current == '}')
    返回 scanFlowCollectionEnd(false);

  如果 (*Current == ',')
    返回 scanFlowEntry();

  如果 (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    返回 scanBlockEntry();

  如果 (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    返回 scanKey();

  如果 (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || IsAdjacentValueAllowedInFlow))
    返回 scanValue();

  如果 (*Current == '*')
    返回 scanAliasOrAnchor(true);

  如果 (*Current == '&')
    返回 scanAliasOrAnchor(false);

  如果 (*Current == '!')
    返回 scanTag();

  如果 (*Current == '|' && !FlowLevel)
    返回 scanBlockScalar(true);

  如果 (*Current == '>' && !FlowLevel)
    返回 scanBlockScalar(false);

  如果 (*Current == '\'')
    返回 scanFlowScalar(false);

  如果 (*Current == '"')
    返回 scanFlowScalar(true);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  如果 ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    返回 scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  返回 false;
}

Stream::Stream(StringRef Input, SourceMgr &SM, bool ShowColors,
               std::error_code *EC)
    : scanner(new Scanner(Input, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef InputBuffer, SourceMgr &SM, bool ShowColors,
               std::error_code *EC)
    : scanner(new Scanner(InputBuffer, SM, ShowColors, EC)) {}

Stream::~Stream() = default;

bool Stream::failed() { 返回 scanner->failed(); }

void Stream::printError(Node *N, const Twine &Msg, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), Msg, Kind);
}

void Stream::printError(const SMRange &Range, const Twine &Msg,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, Msg, Range);
}

document_iterator Stream::begin() {
  如果 (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(new Document(*this));
  返回 document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  返回 document_iterator();
}

void Stream::skip() {
  为 (Document &Doc : *this)
    Doc.skip();
}

Node::Node(unsigned 整数 Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string Node::getVerbatimTag() const {
  StringRef Raw = getRawTag();
  如果 (!Raw.empty() && Raw != "!") {
    std::string Ret;
    如果 (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      返回 Ret;
    } 否则 如果 (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      返回 Ret;
    } 否则 {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      如果 (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      否则 {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      返回 Ret;
    }
  }

  switch (getType()) {
  case NK_Null:
    返回 "tag:yaml.org,2002:null";
  case NK_Scalar:
  case NK_BlockScalar:
    // TODO: Tag resolution.
    返回 "tag:yaml.org,2002:str";
  case NK_Mapping:
    返回 "tag:yaml.org,2002:map";
  case NK_Sequence:
    返回 "tag:yaml.org,2002:seq";
  }

  返回 "";
}

Token &Node::peekNext() {
  返回 Doc->peekNext();
}

Token Node::getNext() {
  返回 Doc->getNext();
}

Node *Node::parseBlockNode() {
  返回 Doc->parseBlockNode();
}

BumpPtrAllocator &Node::getAllocator() {
  返回 Doc->NodeAllocator;
}

void Node::setError(const Twine &Msg, Token &Tok) const {
  Doc->setError(Msg, Tok);
}

bool Node::failed() const {
  返回 Doc->failed();
}

StringRef ScalarNode::getValue(SmallVectorImpl<char> &Storage) const {
  如果 (Value[0] == '"')
    返回 getDoubleQuotedValue(Value, Storage);
  如果 (Value[0] == '\'')
    返回 getSingleQuotedValue(Value, Storage);
  返回 getPlainValue(Value, Storage);
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
  如果 (I == StringRef::npos)
    返回 UnquotedValue;

  Storage.clear();
  Storage.reserve(UnquotedValue.size());
  char LastNewLineAddedAs = '\0';
  为 (; I != StringRef::npos; I = UnquotedValue.find_first_of(LookupChars)) {
    如果 (UnquotedValue[I] != '\r' && UnquotedValue[I] != '\n') {
      llvm::append_range(Storage, UnquotedValue.take_front(I));
      UnquotedValue = UnescapeCallback(UnquotedValue.drop_front(I), Storage);
      LastNewLineAddedAs = '\0';
      continue;
    }
    如果 (size_t LastNonSWhite = UnquotedValue.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, UnquotedValue.take_front(LastNonSWhite + 1));
      Storage.push_back(' ');
      LastNewLineAddedAs = ' ';
    } 否则 {
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
    如果 (UnquotedValue.substr(I, 2) == "\r\n")
      I++;
    UnquotedValue = UnquotedValue.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, UnquotedValue);
  返回 StringRef(Storage.begin(), Storage.size());
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
    如果 (UnquotedValue.size() == 1) {
      Token T;
      T.Range = UnquotedValue;
      setError("Unrecognized escape code", T);
      Storage.clear();
      返回 StringRef();
    }
    UnquotedValue = UnquotedValue.drop_front(1);
    switch (UnquotedValue[0]) {
    default: {
      Token T;
      T.Range = UnquotedValue.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      返回 StringRef();
    }
    case '\r':
      // Shrink the Windows-style EOL.
      如果 (UnquotedValue.size() >= 2 && UnquotedValue[1] == '\n')
        UnquotedValue = UnquotedValue.drop_front(1);
      [[fallthrough]];
    case '\n':
      返回 UnquotedValue.drop_front(1).ltrim(" \t");
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
      如果 (UnquotedValue.size() < 3)
        // TODO: Report error.
        break;
      unsigned 整数 UnicodeScalarValue;
      如果 (UnquotedValue.substr(1, 2).getAsInteger(16, UnicodeScalarValue))
        // TODO: Report error.
        UnicodeScalarValue = 0xFFFD;
      encodeUTF8(UnicodeScalarValue, Storage);
      返回 UnquotedValue.drop_front(3);
    }
    case 'u': {
      如果 (UnquotedValue.size() < 5)
        // TODO: Report error.
        break;
      unsigned 整数 UnicodeScalarValue;
      如果 (UnquotedValue.substr(1, 4).getAsInteger(16, UnicodeScalarValue))
        // TODO: Report error.
        UnicodeScalarValue = 0xFFFD;
      encodeUTF8(UnicodeScalarValue, Storage);
      返回 UnquotedValue.drop_front(5);
    }
    case 'U': {
      如果 (UnquotedValue.size() < 9)
        // TODO: Report error.
        break;
      unsigned 整数 UnicodeScalarValue;
      如果 (UnquotedValue.substr(1, 8).getAsInteger(16, UnicodeScalarValue))
        // TODO: Report error.
        UnicodeScalarValue = 0xFFFD;
      encodeUTF8(UnicodeScalarValue, Storage);
      返回 UnquotedValue.drop_front(9);
    }
    }
    返回 UnquotedValue.drop_front(1);
  };

  返回 parseScalarValue(UnquotedValue, Storage, "\\\r\n", UnescapeFunc);
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
    返回 UnquotedValue.drop_front(2);
  };

  返回 parseScalarValue(UnquotedValue, Storage, "'\r\n", UnescapeFunc);
}

StringRef ScalarNode::getPlainValue(StringRef RawValue,
                                    SmallVectorImpl<char> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  RawValue = RawValue.rtrim("\r\n \t");
  返回 parseScalarValue(RawValue, Storage, "\r\n", nullptr);
}

Node *KeyValueNode::getKey() {
  如果 (Key)
    返回 Key;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    如果 (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_Value
        || t.Kind == Token::TK_Error) {
      返回 Key = new (getAllocator()) NullNode(Doc);
    }
    如果 (t.Kind == Token::TK_Key)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  如果 (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_Value) {
    返回 Key = new (getAllocator()) NullNode(Doc);
  }

  // We've got a normal key.
  返回 Key = parseBlockNode();
}

Node *KeyValueNode::getValue() {
  如果 (Value)
    返回 Value;

  如果 (Node* Key = getKey())
    Key->skip();
  否则 {
    setError("Null key in Key Value.", peekNext());
    返回 Value = new (getAllocator()) NullNode(Doc);
  }

  如果 (failed())
    返回 Value = new (getAllocator()) NullNode(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    如果 (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_Key
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      返回 Value = new (getAllocator()) NullNode(Doc);
    }

    如果 (t.Kind != Token::TK_Value) {
      setError("Unexpected token in Key Value.", t);
      返回 Value = new (getAllocator()) NullNode(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  如果 (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_Key) {
    返回 Value = new (getAllocator()) NullNode(Doc);
  }

  // We got a normal value.
  返回 Value = parseBlockNode();
}

void MappingNode::increment() {
  如果 (failed()) {
    IsAtEnd = true;
    CurrentEntry = nullptr;
    返回;
  }
  如果 (CurrentEntry) {
    CurrentEntry->skip();
    如果 (Type == MT_Inline) {
      IsAtEnd = true;
      CurrentEntry = nullptr;
      返回;
    }
  }
  Token T = peekNext();
  如果 (T.Kind == Token::TK_Key || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = new (getAllocator()) KeyValueNode(Doc);
  } 否则 如果 (Type == MT_Block) {
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
  } 否则 {
    switch (T.Kind) {
    case Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      返回 increment();
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
  如果 (failed()) {
    IsAtEnd = true;
    CurrentEntry = nullptr;
    返回;
  }
  如果 (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  如果 (SeqType == ST_Block) {
    switch (T.Kind) {
    case Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parseBlockNode();
      如果 (!CurrentEntry) { // An error occurred.
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
  } 否则 如果 (SeqType == ST_Indentless) {
    switch (T.Kind) {
    case Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parseBlockNode();
      如果 (!CurrentEntry) { // An error occurred.
        IsAtEnd = true;
        CurrentEntry = nullptr;
      }
      break;
    default:
    case Token::TK_Error:
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  } 否则 如果 (SeqType == ST_Flow) {
    switch (T.Kind) {
    case Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      WasPreviousTokenFlowEntry = true;
      返回 increment();
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
      如果 (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = true;
        CurrentEntry = nullptr;
        break;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parseBlockNode();
      如果 (!CurrentEntry) {
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

  如果 (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peekNext();
  如果 (T.Kind == Token::TK_DocumentStart)
    getNext();
}

bool Document::skip()  {
  如果 (stream.scanner->failed())
    返回 false;
  如果 (!Root && !getRoot())
    返回 false;
  Root->skip();
  Token &T = peekNext();
  如果 (T.Kind == Token::TK_StreamEnd)
    返回 false;
  如果 (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    返回 skip();
  }
  返回 true;
}

Token &Document::peekNext() {
  返回 stream.scanner->peekNext();
}

Token Document::getNext() {
  返回 stream.scanner->getNext();
}

void Document::setError(const Twine &Message, Token &Location) const {
  stream.scanner->setError(Message, Location.Range.begin());
}

bool Document::failed() const {
  返回 stream.scanner->failed();
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
    返回 new (NodeAllocator) AliasNode(stream.CurrentDoc, T.Range.substr(1));
  case Token::TK_Anchor:
    如果 (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      返回 nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    goto parse_property;
  case Token::TK_Tag:
    如果 (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      返回 nullptr;
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
    返回 new (NodeAllocator) SequenceNode( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , SequenceNode::ST_Indentless);
  case Token::TK_BlockSequenceStart:
    getNext();
    返回 new (NodeAllocator)
      SequenceNode( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , SequenceNode::ST_Block);
  case Token::TK_BlockMappingStart:
    getNext();
    返回 new (NodeAllocator)
      MappingNode( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , MappingNode::MT_Block);
  case Token::TK_FlowSequenceStart:
    getNext();
    返回 new (NodeAllocator)
      SequenceNode( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , SequenceNode::ST_Flow);
  case Token::TK_FlowMappingStart:
    getNext();
    返回 new (NodeAllocator)
      MappingNode( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , MappingNode::MT_Flow);
  case Token::TK_Scalar:
    getNext();
    返回 new (NodeAllocator)
      ScalarNode( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  case Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.Value.c_str(), T.Value.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(NodeAllocator).drop_back();
    返回 new (NodeAllocator)
        BlockScalarNode(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  case Token::TK_Key:
    // Don't eat the TK_Key, KeyValueNode expects it.
    返回 new (NodeAllocator)
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
    返回 new (NodeAllocator) NullNode(stream.CurrentDoc);
  case Token::TK_FlowMappingEnd:
  case Token::TK_FlowSequenceEnd:
  case Token::TK_FlowEntry: {
    如果 (Root && (isa<MappingNode>(Root) || isa<SequenceNode>(Root)))
      返回 new (NodeAllocator) NullNode(stream.CurrentDoc);

    setError("Unexpected token", T);
    返回 nullptr;
  }
  case Token::TK_Error:
    返回 nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  返回 nullptr;
}

bool Document::parseDirectives() {
  bool isDirective = false;
  当 (true) {
    Token T = peekNext();
    如果 (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = true;
    } 否则 如果 (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = true;
    } 否则 {
      break;
    }
  }
  返回 isDirective;
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

bool Document::expectToken(整数 TK) {
  Token T = getNext();
  如果 (T.Kind != TK) {
    setError("Unexpected token", T);
    返回 false;
  }
  返回 true;
}
