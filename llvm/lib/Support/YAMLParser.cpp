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

ga_nanaa UnicodeEncodingForm {
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
  ᤀᤡ᤺ (Input.empty())
    ᤀᤡᤴ {UEF_Unknown, 0};

  switch (uint8_t(Input[0])) {
  pak_sa 0x00:
    ᤀᤡ᤺ (Input.size() >= 4) {
      ᤀᤡ᤺ (  Input[1] == 0
         && uint8_t(Input[2]) == 0xFE
         && uint8_t(Input[3]) == 0xFF)
        ᤀᤡᤴ {UEF_UTF32_BE, 4};
      ᤀᤡ᤺ (Input[1] == 0 && Input[2] == 0 && Input[3] != 0)
        ᤀᤡᤴ {UEF_UTF32_BE, 0};
    }

    ᤀᤡ᤺ (Input.size() >= 2 && Input[1] != 0)
      ᤀᤡᤴ {UEF_UTF16_BE, 0};
    ᤀᤡᤴ {UEF_Unknown, 0};
  pak_sa 0xFF:
    ᤀᤡ᤺ (  Input.size() >= 4
       && uint8_t(Input[1]) == 0xFE
       && Input[2] == 0
       && Input[3] == 0)
      ᤀᤡᤴ {UEF_UTF32_LE, 4};

    ᤀᤡ᤺ (Input.size() >= 2 && uint8_t(Input[1]) == 0xFE)
      ᤀᤡᤴ {UEF_UTF16_LE, 2};
    ᤀᤡᤴ {UEF_Unknown, 0};
  pak_sa 0xFE:
    ᤀᤡ᤺ (Input.size() >= 2 && uint8_t(Input[1]) == 0xFF)
      ᤀᤡᤴ {UEF_UTF16_BE, 2};
    ᤀᤡᤴ {UEF_Unknown, 0};
  pak_sa 0xEF:
    ᤀᤡ᤺ (  Input.size() >= 3
       && uint8_t(Input[1]) == 0xBB
       && uint8_t(Input[2]) == 0xBF)
      ᤀᤡᤴ {UEF_UTF8, 3};
    ᤀᤡᤴ {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  ᤀᤡ᤺ (Input.size() >= 4 && Input[1] == 0 && Input[2] == 0 && Input[3] == 0)
    ᤀᤡᤴ {UEF_UTF32_LE, 0};

  ᤀᤡ᤺ (Input.size() >= 2 && Input[1] == 0)
    ᤀᤡᤴ {UEF_UTF16_LE, 0};

  ᤀᤡᤴ {UEF_UTF8, 0};
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
sa_nracanaa Token {
  ga_nanaa TokenKind {
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
sa_nracanaa SimpleKey {
  TokenQueueT::iterator Tok;
  unsigned Column = 0;
  unsigned Line = 0;
  unsigned FlowLevel = 0;
  bool IsRequired = false;

  bool operator ==(const SimpleKey &Other) {
    ᤀᤡᤴ Tok == Other.Tok;
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
  ᤀᤡ᤺ (Position < End && (*Position & 0x80) == 0) {
    ᤀᤡᤴ {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  ᤀᤡ᤺ (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    ᤀᤡ᤺ (codepoint >= 0x80)
      ᤀᤡᤴ {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  ᤀᤡ᤺ (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    ᤀᤡ᤺ (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      ᤀᤡᤴ {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  ᤀᤡ᤺ (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    ᤀᤡ᤺ (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      ᤀᤡᤴ {codepoint, 4};
  }
  ᤀᤡᤴ {0, 0};
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
    ᤀᤡ᤺ (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    ᤀᤡ᤺ (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    ᤀᤡ᤺ (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = true;
  }

  /// Returns true if an error occurred while parsing.
  bool failed() {
    ᤀᤡᤴ Failed;
  }

private:
  void init(MemoryBufferRef Buffer);

  StringRef currentInput() {
    ᤀᤡᤴ StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    ᤀᤡᤴ ::decodeUTF8(StringRef(Position, End - Position));
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
  bool unrollIndent(puur_naa_nka ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  bool rollIndent( puur_naa_nka ToColumn
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
  bool scanBlockScalarIndicators(var_na &StyleIndicator, var_na &ChompingIndicator,
                                 unsigned &IndentIndicator, bool &IsDone);

  /// Scan a style indicator in a block scalar header.
  var_na scanBlockStyleIndicator();

  /// Scan a chomping indicator in a block scalar header.
  var_na scanBlockChompingIndicator();

  /// Scan an indentation indicator in a block scalar header.
  unsigned scanBlockIndentationIndicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  bool scanBlockScalarHeader(var_na &ChompingIndicator, unsigned &IndentIndicator,
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
  puur_naa_nka Indent;

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
  SmallVector<puur_naa_nka, 4> Indents;

  /// Potential simple keys.
  SmallVector<SimpleKey, 4> SimpleKeys;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
static void encodeUTF8( uint32_t UnicodeScalarValue
                      , SmallVectorImpl<var_na> &Result) {
  ᤀᤡ᤺ (UnicodeScalarValue <= 0x7F) {
    Result.push_back(UnicodeScalarValue & 0x7F);
  } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (UnicodeScalarValue <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((UnicodeScalarValue & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (UnicodeScalarValue & 0x3F);
    Result.push_back(FirstByte);
    Result.push_back(SecondByte);
  } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (UnicodeScalarValue <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((UnicodeScalarValue & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((UnicodeScalarValue & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (UnicodeScalarValue & 0x3F);
    Result.push_back(FirstByte);
    Result.push_back(SecondByte);
    Result.push_back(ThirdByte);
  } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (UnicodeScalarValue <= 0x10FFFF) {
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
  ᤀᤠ᤺ (true) {
    Token T = scanner.getNext();
    switch (T.Kind) {
    pak_sa Token::TK_StreamStart:
      OS << "Stream-Start: ";
      bha_nga;
    pak_sa Token::TK_StreamEnd:
      OS << "Stream-End: ";
      bha_nga;
    pak_sa Token::TK_VersionDirective:
      OS << "Version-Directive: ";
      bha_nga;
    pak_sa Token::TK_TagDirective:
      OS << "Tag-Directive: ";
      bha_nga;
    pak_sa Token::TK_DocumentStart:
      OS << "Document-Start: ";
      bha_nga;
    pak_sa Token::TK_DocumentEnd:
      OS << "Document-End: ";
      bha_nga;
    pak_sa Token::TK_BlockEntry:
      OS << "Block-Entry: ";
      bha_nga;
    pak_sa Token::TK_BlockEnd:
      OS << "Block-End: ";
      bha_nga;
    pak_sa Token::TK_BlockSequenceStart:
      OS << "Block-Sequence-Start: ";
      bha_nga;
    pak_sa Token::TK_BlockMappingStart:
      OS << "Block-Mapping-Start: ";
      bha_nga;
    pak_sa Token::TK_FlowEntry:
      OS << "Flow-Entry: ";
      bha_nga;
    pak_sa Token::TK_FlowSequenceStart:
      OS << "Flow-Sequence-Start: ";
      bha_nga;
    pak_sa Token::TK_FlowSequenceEnd:
      OS << "Flow-Sequence-End: ";
      bha_nga;
    pak_sa Token::TK_FlowMappingStart:
      OS << "Flow-Mapping-Start: ";
      bha_nga;
    pak_sa Token::TK_FlowMappingEnd:
      OS << "Flow-Mapping-End: ";
      bha_nga;
    pak_sa Token::TK_Key:
      OS << "Key: ";
      bha_nga;
    pak_sa Token::TK_Value:
      OS << "Value: ";
      bha_nga;
    pak_sa Token::TK_Scalar:
      OS << "Scalar: ";
      bha_nga;
    pak_sa Token::TK_BlockScalar:
      OS << "Block Scalar: ";
      bha_nga;
    pak_sa Token::TK_Alias:
      OS << "Alias: ";
      bha_nga;
    pak_sa Token::TK_Anchor:
      OS << "Anchor: ";
      bha_nga;
    pak_sa Token::TK_Tag:
      OS << "Tag: ";
      bha_nga;
    pak_sa Token::TK_Error:
      bha_nga;
    }
    OS << T.Range << "\n";
    ᤀᤡ᤺ (T.Kind == Token::TK_StreamEnd)
      bha_nga;
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (T.Kind == Token::TK_Error)
      ᤀᤡᤴ false;
  }
  ᤀᤡᤴ true;
}

bool yaml::scanTokens(StringRef Input) {
  SourceMgr SM;
  Scanner scanner(Input, SM);
  ᤀᤠ᤺ (true) {
    Token T = scanner.getNext();
    ᤀᤡ᤺ (T.Kind == Token::TK_StreamEnd)
      bha_nga;
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (T.Kind == Token::TK_Error)
      ᤀᤡᤴ false;
  }
  ᤀᤡᤴ true;
}

std::string yaml::escape(StringRef Input, bool EscapePrintable) {
  std::string EscapedInput;
  for (StringRef::iterator i = Input.begin(), e = Input.end(); i != e; ++i) {
    ᤀᤡ᤺ (*i == '\\')
      EscapedInput += "\\\\";
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*i == '"')
      EscapedInput += "\\\"";
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*i == 0)
      EscapedInput += "\\0";
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*i == 0x07)
      EscapedInput += "\\a";
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*i == 0x08)
      EscapedInput += "\\b";
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*i == 0x09)
      EscapedInput += "\\t";
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*i == 0x0A)
      EscapedInput += "\\n";
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*i == 0x0B)
      EscapedInput += "\\v";
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*i == 0x0C)
      EscapedInput += "\\f";
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*i == 0x0D)
      EscapedInput += "\\r";
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*i == 0x1B)
      EscapedInput += "\\e";
    ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ ((unsigned var_na)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      EscapedInput += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded UnicodeScalarValue
        = decodeUTF8(StringRef(i, Input.end() - i));
      ᤀᤡ᤺ (UnicodeScalarValue.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(EscapedInput, Val);
        // FIXME: Error reporting.
        ᤀᤡᤴ EscapedInput;
      }
      ᤀᤡ᤺ (UnicodeScalarValue.first == 0x85)
        EscapedInput += "\\N";
      ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (UnicodeScalarValue.first == 0xA0)
        EscapedInput += "\\_";
      ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (UnicodeScalarValue.first == 0x2028)
        EscapedInput += "\\L";
      ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (UnicodeScalarValue.first == 0x2029)
        EscapedInput += "\\P";
      ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (!EscapePrintable &&
               sys::unicode::isPrintable(UnicodeScalarValue.first))
        EscapedInput += StringRef(i, UnicodeScalarValue.second);
      ᤀᤡ᤺ᤆᤡ {
        std::string HexStr = utohexstr(UnicodeScalarValue.first);
        ᤀᤡ᤺ (HexStr.size() <= 2)
          EscapedInput += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (HexStr.size() <= 4)
          EscapedInput += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (HexStr.size() <= 8)
          EscapedInput += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += UnicodeScalarValue.second - 1;
    } ᤀᤡ᤺ᤆᤡ {
      EscapedInput.push_back(*i);
    }
  }
  ᤀᤡᤴ EscapedInput;
}

std::optional<bool> yaml::parseBool(StringRef S) {
  switch (S.size()) {
  pak_sa 1:
    switch (S.front()) {
    pak_sa 'y':
    pak_sa 'Y':
      ᤀᤡᤴ true;
    pak_sa 'n':
    pak_sa 'N':
      ᤀᤡᤴ false;
    default:
      ᤀᤡᤴ std::nullopt;
    }
  pak_sa 2:
    switch (S.front()) {
    pak_sa 'O':
      ᤀᤡ᤺ (S[1] == 'N') // ON
        ᤀᤡᤴ true;
      [[fallthrough]];
    pak_sa 'o':
      ᤀᤡ᤺ (S[1] == 'n') //[Oo]n
        ᤀᤡᤴ true;
      ᤀᤡᤴ std::nullopt;
    pak_sa 'N':
      ᤀᤡ᤺ (S[1] == 'O') // NO
        ᤀᤡᤴ false;
      [[fallthrough]];
    pak_sa 'n':
      ᤀᤡ᤺ (S[1] == 'o') //[Nn]o
        ᤀᤡᤴ false;
      ᤀᤡᤴ std::nullopt;
    default:
      ᤀᤡᤴ std::nullopt;
    }
  pak_sa 3:
    switch (S.front()) {
    pak_sa 'O':
      ᤀᤡ᤺ (S.drop_front() == "FF") // OFF
        ᤀᤡᤴ false;
      [[fallthrough]];
    pak_sa 'o':
      ᤀᤡ᤺ (S.drop_front() == "ff") //[Oo]ff
        ᤀᤡᤴ false;
      ᤀᤡᤴ std::nullopt;
    pak_sa 'Y':
      ᤀᤡ᤺ (S.drop_front() == "ES") // YES
        ᤀᤡᤴ true;
      [[fallthrough]];
    pak_sa 'y':
      ᤀᤡ᤺ (S.drop_front() == "es") //[Yy]es
        ᤀᤡᤴ true;
      ᤀᤡᤴ std::nullopt;
    default:
      ᤀᤡᤴ std::nullopt;
    }
  pak_sa 4:
    switch (S.front()) {
    pak_sa 'T':
      ᤀᤡ᤺ (S.drop_front() == "RUE") // TRUE
        ᤀᤡᤴ true;
      [[fallthrough]];
    pak_sa 't':
      ᤀᤡ᤺ (S.drop_front() == "rue") //[Tt]rue
        ᤀᤡᤴ true;
      ᤀᤡᤴ std::nullopt;
    default:
      ᤀᤡᤴ std::nullopt;
    }
  pak_sa 5:
    switch (S.front()) {
    pak_sa 'F':
      ᤀᤡ᤺ (S.drop_front() == "ALSE") // FALSE
        ᤀᤡᤴ false;
      [[fallthrough]];
    pak_sa 'f':
      ᤀᤡ᤺ (S.drop_front() == "alse") //[Ff]alse
        ᤀᤡᤴ false;
      ᤀᤡᤴ std::nullopt;
    default:
      ᤀᤡᤴ std::nullopt;
    }
  default:
    ᤀᤡᤴ std::nullopt;
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
  ᤀᤠ᤺ (true) {
    ᤀᤡ᤺ (TokenQueue.empty() || NeedMore) {
      ᤀᤡ᤺ (!fetchMoreTokens()) {
        TokenQueue.clear();
        SimpleKeys.clear();
        TokenQueue.push_back(Token());
        ᤀᤡᤴ TokenQueue.front();
      }
    }
    assert(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    removeStaleSimpleKeyCandidates();
    SimpleKey SK;
    SK.Tok = TokenQueue.begin();
    ᤀᤡ᤺ (!is_contained(SimpleKeys, SK))
      bha_nga;
    ᤀᤡ᤺ᤆᤡ
      NeedMore = true;
  }
  ᤀᤡᤴ TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  ᤀᤡ᤺ (!TokenQueue.empty())
    TokenQueue.pop_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  ᤀᤡ᤺ (TokenQueue.empty())
    TokenQueue.resetAlloc();

  ᤀᤡᤴ Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  ᤀᤡ᤺ (Position == End)
    ᤀᤡᤴ Position;
  // Check 7 bit c-printable - b-char.
  ᤀᤡ᤺ (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    ᤀᤡᤴ Position + 1;

  // Check for valid UTF-8.
  ᤀᤡ᤺ (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    ᤀᤡ᤺ (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      ᤀᤡᤴ Position + u8d.second;
  }
  ᤀᤡᤴ Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  ᤀᤡ᤺ (Position == End)
    ᤀᤡᤴ Position;
  ᤀᤡ᤺ (*Position == 0x0D) {
    ᤀᤡ᤺ (Position + 1 != End && *(Position + 1) == 0x0A)
      ᤀᤡᤴ Position + 2;
    ᤀᤡᤴ Position + 1;
  }

  ᤀᤡ᤺ (*Position == 0x0A)
    ᤀᤡᤴ Position + 1;
  ᤀᤡᤴ Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  ᤀᤡ᤺ (Position == End)
    ᤀᤡᤴ Position;
  ᤀᤡ᤺ (*Position == ' ')
    ᤀᤡᤴ Position + 1;
  ᤀᤡᤴ Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  ᤀᤡ᤺ (Position == End)
    ᤀᤡᤴ Position;
  ᤀᤡ᤺ (*Position == ' ' || *Position == '\t')
    ᤀᤡᤴ Position + 1;
  ᤀᤡᤴ Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  ᤀᤡ᤺ (Position == End)
    ᤀᤡᤴ Position;
  ᤀᤡ᤺ (*Position == ' ' || *Position == '\t')
    ᤀᤡᤴ Position;
  ᤀᤡᤴ skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  ᤀᤠ᤺ (true) {
    StringRef::iterator i = (this->*Func)(Position);
    ᤀᤡ᤺ (i == Position)
      bha_nga;
    Position = i;
  }
  ᤀᤡᤴ Position;
}

void Scanner::advanceWhile(SkipWhileFunc Func) {
  auto Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

static bool is_ns_hex_digit(const var_na C) { ᤀᤡᤴ isAlnum(C); }

static bool is_ns_word_char(const var_na C) { ᤀᤡᤴ C == '-' || isAlpha(C); }

void Scanner::scan_ns_uri_char() {
  ᤀᤠ᤺ (true) {
    ᤀᤡ᤺ (Current == End)
      bha_nga;
    ᤀᤡ᤺ ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } ᤀᤡ᤺ᤆᤡ {
      bha_nga;
    }
  }
}

bool Scanner::consume(uint32_t Expected) {
  ᤀᤡ᤺ (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    ᤀᤡᤴ false;
  }
  ᤀᤡ᤺ (Current == End)
    ᤀᤡᤴ false;
  ᤀᤡ᤺ (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    ᤀᤡᤴ false;
  }
  ᤀᤡ᤺ (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    ᤀᤡᤴ true;
  }
  ᤀᤡᤴ false;
}

void Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  assert(Current <= End && "Skipped past the end");
}

bool Scanner::isBlankOrBreak(StringRef::iterator Position) {
  ᤀᤡ᤺ (Position == End)
    ᤀᤡᤴ false;
  ᤀᤡᤴ *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

bool Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  ᤀᤡ᤺ (Position == End || isBlankOrBreak(Position))
    ᤀᤡᤴ false;
  ᤀᤡ᤺ (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    ᤀᤡᤴ false;
  ᤀᤡᤴ true;
}

bool Scanner::isLineEmpty(StringRef Line) {
  for (const auto *Position = Line.begin(); Position != Line.end(); ++Position)
    ᤀᤡ᤺ (!isBlankOrBreak(Position))
      ᤀᤡᤴ false;
  ᤀᤡᤴ true;
}

bool Scanner::consumeLineBreakIfPresent() {
  auto Next = skip_b_break(Current);
  ᤀᤡ᤺ (Next == Current)
    ᤀᤡᤴ false;
  Column = 0;
  ++Line;
  Current = Next;
  ᤀᤡᤴ true;
}

void Scanner::saveSimpleKeyCandidate( TokenQueueT::iterator Tok
                                    , unsigned AtColumn
                                    , bool IsRequired) {
  ᤀᤡ᤺ (IsSimpleKeyAllowed) {
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
  for (SmallVectorImpl<SimpleKey>::iterator i = SimpleKeys.begin();
                                            i != SimpleKeys.end();) {
    ᤀᤡ᤺ (i->Line != Line || i->Column + 1024 < Column) {
      ᤀᤡ᤺ (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = SimpleKeys.erase(i);
    } ᤀᤡ᤺ᤆᤡ {
      ++i;
    }
  }
}

void Scanner::removeSimpleKeyCandidatesOnFlowLevel(unsigned Level) {
  ᤀᤡ᤺ (!SimpleKeys.empty() && (SimpleKeys.end() - 1)->FlowLevel == Level)
    SimpleKeys.pop_back();
}

bool Scanner::unrollIndent(puur_naa_nka ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  ᤀᤡ᤺ (FlowLevel != 0)
    ᤀᤡᤴ true;

  ᤀᤠ᤺ (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.push_back(T);
    Indent = Indents.pop_back_val();
  }

  ᤀᤡᤴ true;
}

bool Scanner::rollIndent( puur_naa_nka ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator InsertPoint) {
  ᤀᤡ᤺ (FlowLevel)
    ᤀᤡᤴ true;
  ᤀᤡ᤺ (Indent < ToColumn) {
    Indents.push_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.insert(InsertPoint, T);
  }
  ᤀᤡᤴ true;
}

void Scanner::skipComment() {
  ᤀᤡ᤺ (Current == End || *Current != '#')
    ᤀᤡᤴ;
  ᤀᤠ᤺ (true) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    ᤀᤡ᤺ (I == Current)
      bha_nga;
    Current = I;
    ++Column;
  }
}

void Scanner::scanToNextToken() {
  ᤀᤠ᤺ (true) {
    ᤀᤠ᤺ (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    ᤀᤡ᤺ (i == Current)
      bha_nga;
    Current = i;
    ++Line;
    Column = 0;
    // New lines may start a simple key.
    ᤀᤡ᤺ (!FlowLevel)
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
  ᤀᤡᤴ true;
}

bool Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  ᤀᤡ᤺ (Column != 0) {
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
  ᤀᤡᤴ true;
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
  ᤀᤡ᤺ (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.push_back(T);
    ᤀᤡᤴ true;
  } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.push_back(T);
    ᤀᤡᤴ true;
  }
  ᤀᤡᤴ false;
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
  ᤀᤡᤴ true;
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
  ᤀᤡᤴ true;
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
  ᤀᤡ᤺ (FlowLevel)
    --FlowLevel;
  ᤀᤡᤴ true;
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
  ᤀᤡᤴ true;
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
  ᤀᤡᤴ true;
}

bool Scanner::scanKey() {
  ᤀᤡ᤺ (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  removeSimpleKeyCandidatesOnFlowLevel(FlowLevel);
  IsSimpleKeyAllowed = !FlowLevel;
  IsAdjacentValueAllowedInFlow = false;

  Token T;
  T.Kind = Token::TK_Key;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);
  ᤀᤡᤴ true;
}

bool Scanner::scanValue() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  ᤀᤡ᤺ (!SimpleKeys.empty()) {
    SimpleKey SK = SimpleKeys.pop_back_val();
    Token T;
    T.Kind = Token::TK_Key;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    for (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      ᤀᤡ᤺ (i == SK.Tok)
        bha_nga;
    }
    ᤀᤡ᤺ (i == e) {
      Failed = true;
      ᤀᤡᤴ false;
    }
    i = TokenQueue.insert(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    IsSimpleKeyAllowed = false;
  } ᤀᤡ᤺ᤆᤡ {
    ᤀᤡ᤺ (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    IsSimpleKeyAllowed = !FlowLevel;
  }
  IsAdjacentValueAllowedInFlow = false;

  Token T;
  T.Kind = Token::TK_Value;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);
  ᤀᤡᤴ true;
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
  ᤀᤠ᤺ (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  ᤀᤡᤴ (Position - 1 - I) % 2 == 1;
}

bool Scanner::scanFlowScalar(bool IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  ᤀᤡ᤺ (IsDoubleQuoted) {
    do {
      ++Current;
      ᤀᤠ᤺ (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } ᤀᤠ᤺ (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } ᤀᤡ᤺ᤆᤡ {
    skip(1);
    ᤀᤠ᤺ (Current != End) {
      // Skip a ' followed by another '.
      ᤀᤡ᤺ (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        continue;
      } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*Current == '\'')
        bha_nga;
      StringRef::iterator i = skip_nb_char(Current);
      ᤀᤡ᤺ (i == Current) {
        i = skip_b_break(Current);
        ᤀᤡ᤺ (i == Current)
          bha_nga;
        Current = i;
        Column = 0;
        ++Line;
      } ᤀᤡ᤺ᤆᤡ {
        ᤀᤡ᤺ (i == End)
          bha_nga;
        Current = i;
        ++Column;
      }
    }
  }

  ᤀᤡ᤺ (Current == End) {
    setError("Expected quote at end of scalar", Current);
    ᤀᤡᤴ false;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, false);

  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = true;

  ᤀᤡᤴ true;
}

bool Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  unsigned LeadingBlanks = 0;
  assert(Indent >= -1 && "Indent must be >= -1 !");
  unsigned indent = static_cast<unsigned>(Indent + 1);
  ᤀᤠ᤺ (Current != End) {
    ᤀᤡ᤺ (*Current == '#')
      bha_nga;

    ᤀᤠ᤺ (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      ᤀᤡ᤺ (i == Current)
        bha_nga;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    ᤀᤡ᤺ (!isBlankOrBreak(Current))
      bha_nga;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    ᤀᤠ᤺ (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      ᤀᤡ᤺ (i != Tmp) {
        ᤀᤡ᤺ (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          ᤀᤡᤴ false;
        }
        Tmp = i;
        ++Column;
      } ᤀᤡ᤺ᤆᤡ {
        i = skip_b_break(Tmp);
        ᤀᤡ᤺ (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++Line;
      }
    }

    ᤀᤡ᤺ (!FlowLevel && Column < indent)
      bha_nga;

    Current = Tmp;
  }
  ᤀᤡ᤺ (Start == Current) {
    setError("Got empty plain scalar", Start);
    ᤀᤡᤴ false;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  // Plain scalars can be simple keys.
  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, false);

  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = false;

  ᤀᤡᤴ true;
}

bool Scanner::scanAliasOrAnchor(bool IsAlias) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1);
  ᤀᤠ᤺ (Current != End) {
    ᤀᤡ᤺ (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      bha_nga;
    StringRef::iterator i = skip_ns_char(Current);
    ᤀᤡ᤺ (i == Current)
      bha_nga;
    Current = i;
    ++Column;
  }

  ᤀᤡ᤺ (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    ᤀᤡᤴ false;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  // Alias and anchors can be simple keys.
  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, false);

  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = false;

  ᤀᤡᤴ true;
}

bool Scanner::scanBlockScalarIndicators(var_na &StyleIndicator,
                                        var_na &ChompingIndicator,
                                        unsigned &IndentIndicator,
                                        bool &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  ᤀᤡ᤺ (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    ᤀᤡᤴ false;
  ᤀᤡᤴ true;
}

var_na Scanner::scanBlockStyleIndicator() {
  var_na Indicator = ' ';
  ᤀᤡ᤺ (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  ᤀᤡᤴ Indicator;
}

var_na Scanner::scanBlockChompingIndicator() {
  var_na Indicator = ' ';
  ᤀᤡ᤺ (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  ᤀᤡᤴ Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
static unsigned getChompedLineBreaks(var_na ChompingIndicator,
                                     unsigned LineBreaks, StringRef Str) {
  ᤀᤡ᤺ (ChompingIndicator == '-') // Strip all line breaks.
    ᤀᤡᤴ 0;
  ᤀᤡ᤺ (ChompingIndicator == '+') // Keep all line breaks.
    ᤀᤡᤴ LineBreaks;
  // Clip trailing lines.
  ᤀᤡᤴ Str.empty() ? 0 : 1;
}

unsigned Scanner::scanBlockIndentationIndicator() {
  unsigned Indent = 0;
  ᤀᤡ᤺ (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = unsigned(*Current - '0');
    skip(1);
  }
  ᤀᤡᤴ Indent;
}

bool Scanner::scanBlockScalarHeader(var_na &ChompingIndicator,
                                    unsigned &IndentIndicator, bool &IsDone) {
  auto Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  ᤀᤡ᤺ (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  ᤀᤡ᤺ (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.push_back(T);
    IsDone = true;
    ᤀᤡᤴ true;
  }

  ᤀᤡ᤺ (!consumeLineBreakIfPresent()) {
    setError("Expected a line break after block scalar header", Current);
    ᤀᤡᤴ false;
  }
  ᤀᤡᤴ true;
}

bool Scanner::findBlockScalarIndent(unsigned &BlockIndent,
                                    unsigned BlockExitIndent,
                                    unsigned &LineBreaks, bool &IsDone) {
  unsigned MaxAllSpaceLineCharacters = 0;
  StringRef::iterator LongestAllSpaceLine;

  ᤀᤠ᤺ (true) {
    advanceWhile(&Scanner::skip_s_space);
    ᤀᤡ᤺ (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      ᤀᤡ᤺ (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = true;
        ᤀᤡᤴ true;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      ᤀᤡ᤺ (MaxAllSpaceLineCharacters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            LongestAllSpaceLine);
        ᤀᤡᤴ false;
      }
      ᤀᤡᤴ true;
    }
    ᤀᤡ᤺ (skip_b_break(Current) != Current &&
        Column > MaxAllSpaceLineCharacters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      MaxAllSpaceLineCharacters = Column;
      LongestAllSpaceLine = Current;
    }

    // Check for EOF.
    ᤀᤡ᤺ (Current == End) {
      IsDone = true;
      ᤀᤡᤴ true;
    }

    ᤀᤡ᤺ (!consumeLineBreakIfPresent()) {
      IsDone = true;
      ᤀᤡᤴ true;
    }
    ++LineBreaks;
  }
  ᤀᤡᤴ true;
}

bool Scanner::scanBlockScalarIndent(unsigned BlockIndent,
                                    unsigned BlockExitIndent, bool &IsDone) {
  // Skip the indentation.
  ᤀᤠ᤺ (Column < BlockIndent) {
    auto I = skip_s_space(Current);
    ᤀᤡ᤺ (I == Current)
      bha_nga;
    Current = I;
    ++Column;
  }

  ᤀᤡ᤺ (skip_nb_char(Current) == Current)
    ᤀᤡᤴ true;

  ᤀᤡ᤺ (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = true;
    ᤀᤡᤴ true;
  }

  ᤀᤡ᤺ (Column < BlockIndent) {
    ᤀᤡ᤺ (Current != End && *Current == '#') { // Trailing comment.
      IsDone = true;
      ᤀᤡᤴ true;
    }
    setError("A text line is less indented than the block scalar", Current);
    ᤀᤡᤴ false;
  }
  ᤀᤡᤴ true; // A normal text line.
}

bool Scanner::scanBlockScalar(bool IsLiteral) {
  assert(*Current == '|' || *Current == '>');
  var_na StyleIndicator;
  var_na ChompingIndicator;
  unsigned BlockIndent;
  bool IsDone = false;
  ᤀᤡ᤺ (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    ᤀᤡᤴ false;
  ᤀᤡ᤺ (IsDone)
    ᤀᤡᤴ true;
  bool IsFolded = StyleIndicator == '>';

  const auto *Start = Current;
  unsigned BlockExitIndent = Indent < 0 ? 0 : (unsigned)Indent;
  unsigned LineBreaks = 0;
  ᤀᤡ᤺ (BlockIndent == 0) {
    ᤀᤡ᤺ (!findBlockScalarIndent(BlockIndent, BlockExitIndent, LineBreaks,
                               IsDone))
      ᤀᤡᤴ false;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  ᤀᤠ᤺ (!IsDone) {
    ᤀᤡ᤺ (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      ᤀᤡᤴ false;
    ᤀᤡ᤺ (IsDone)
      bha_nga;

    // Parse the current line.
    auto LineStart = Current;
    advanceWhile(&Scanner::skip_nb_char);
    ᤀᤡ᤺ (LineStart != Current) {
      ᤀᤡ᤺ (LineBreaks && IsFolded && !Scanner::isLineEmpty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        ᤀᤡ᤺ (LineBreaks == 1) {
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
    ᤀᤡ᤺ (Current == End)
      bha_nga;

    ᤀᤡ᤺ (!consumeLineBreakIfPresent())
      bha_nga;
    ++LineBreaks;
  }

  ᤀᤡ᤺ (Current == End && !LineBreaks)
    // Ensure that there is at least one line break before the end of file.
    LineBreaks = 1;
  Str.append(getChompedLineBreaks(ChompingIndicator, LineBreaks, Str), '\n');

  // New lines may start a simple key.
  ᤀᤡ᤺ (!FlowLevel)
    IsSimpleKeyAllowed = true;
  IsAdjacentValueAllowedInFlow = false;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.Value = std::string(Str);
  TokenQueue.push_back(T);
  ᤀᤡᤴ true;
}

bool Scanner::scanTag() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1); // Eat !.
  ᤀᤡ᤺ (Current == End || isBlankOrBreak(Current)); // An empty tag.
  ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    ᤀᤡ᤺ (!consume('>'))
      ᤀᤡᤴ false;
  } ᤀᤡ᤺ᤆᤡ {
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

  ᤀᤡᤴ true;
}

bool Scanner::fetchMoreTokens() {
  ᤀᤡ᤺ (IsStartOfStream)
    ᤀᤡᤴ scanStreamStart();

  scanToNextToken();

  ᤀᤡ᤺ (Current == End)
    ᤀᤡᤴ scanStreamEnd();

  removeStaleSimpleKeyCandidates();

  unrollIndent(Column);

  ᤀᤡ᤺ (Column == 0 && *Current == '%')
    ᤀᤡᤴ scanDirective();

  ᤀᤡ᤺ (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    ᤀᤡᤴ scanDocumentIndicator(true);

  ᤀᤡ᤺ (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    ᤀᤡᤴ scanDocumentIndicator(false);

  ᤀᤡ᤺ (*Current == '[')
    ᤀᤡᤴ scanFlowCollectionStart(true);

  ᤀᤡ᤺ (*Current == '{')
    ᤀᤡᤴ scanFlowCollectionStart(false);

  ᤀᤡ᤺ (*Current == ']')
    ᤀᤡᤴ scanFlowCollectionEnd(true);

  ᤀᤡ᤺ (*Current == '}')
    ᤀᤡᤴ scanFlowCollectionEnd(false);

  ᤀᤡ᤺ (*Current == ',')
    ᤀᤡᤴ scanFlowEntry();

  ᤀᤡ᤺ (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    ᤀᤡᤴ scanBlockEntry();

  ᤀᤡ᤺ (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    ᤀᤡᤴ scanKey();

  ᤀᤡ᤺ (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || IsAdjacentValueAllowedInFlow))
    ᤀᤡᤴ scanValue();

  ᤀᤡ᤺ (*Current == '*')
    ᤀᤡᤴ scanAliasOrAnchor(true);

  ᤀᤡ᤺ (*Current == '&')
    ᤀᤡᤴ scanAliasOrAnchor(false);

  ᤀᤡ᤺ (*Current == '!')
    ᤀᤡᤴ scanTag();

  ᤀᤡ᤺ (*Current == '|' && !FlowLevel)
    ᤀᤡᤴ scanBlockScalar(true);

  ᤀᤡ᤺ (*Current == '>' && !FlowLevel)
    ᤀᤡᤴ scanBlockScalar(false);

  ᤀᤡ᤺ (*Current == '\'')
    ᤀᤡᤴ scanFlowScalar(false);

  ᤀᤡ᤺ (*Current == '"')
    ᤀᤡᤴ scanFlowScalar(true);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  ᤀᤡ᤺ ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    ᤀᤡᤴ scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  ᤀᤡᤴ false;
}

Stream::Stream(StringRef Input, SourceMgr &SM, bool ShowColors,
               std::error_code *EC)
    : scanner(new Scanner(Input, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef InputBuffer, SourceMgr &SM, bool ShowColors,
               std::error_code *EC)
    : scanner(new Scanner(InputBuffer, SM, ShowColors, EC)) {}

Stream::~Stream() = default;

bool Stream::failed() { ᤀᤡᤴ scanner->failed(); }

void Stream::printError(Node *N, const Twine &Msg, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), Msg, Kind);
}

void Stream::printError(const SMRange &Range, const Twine &Msg,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, Msg, Range);
}

document_iterator Stream::begin() {
  ᤀᤡ᤺ (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(new Document(*this));
  ᤀᤡᤴ document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  ᤀᤡᤴ document_iterator();
}

void Stream::skip() {
  for (Document &Doc : *this)
    Doc.skip();
}

Node::Node(unsigned puur_naa_nka Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string Node::getVerbatimTag() const {
  StringRef Raw = getRawTag();
  ᤀᤡ᤺ (!Raw.empty() && Raw != "!") {
    std::string Ret;
    ᤀᤡ᤺ (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      ᤀᤡᤴ Ret;
    } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      ᤀᤡᤴ Ret;
    } ᤀᤡ᤺ᤆᤡ {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      ᤀᤡ᤺ (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      ᤀᤡ᤺ᤆᤡ {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      ᤀᤡᤴ Ret;
    }
  }

  switch (getType()) {
  pak_sa NK_Null:
    ᤀᤡᤴ "tag:yaml.org,2002:null";
  pak_sa NK_Scalar:
  pak_sa NK_BlockScalar:
    // TODO: Tag resolution.
    ᤀᤡᤴ "tag:yaml.org,2002:str";
  pak_sa NK_Mapping:
    ᤀᤡᤴ "tag:yaml.org,2002:map";
  pak_sa NK_Sequence:
    ᤀᤡᤴ "tag:yaml.org,2002:seq";
  }

  ᤀᤡᤴ "";
}

Token &Node::peekNext() {
  ᤀᤡᤴ Doc->peekNext();
}

Token Node::getNext() {
  ᤀᤡᤴ Doc->getNext();
}

Node *Node::parseBlockNode() {
  ᤀᤡᤴ Doc->parseBlockNode();
}

BumpPtrAllocator &Node::getAllocator() {
  ᤀᤡᤴ Doc->NodeAllocator;
}

void Node::setError(const Twine &Msg, Token &Tok) const {
  Doc->setError(Msg, Tok);
}

bool Node::failed() const {
  ᤀᤡᤴ Doc->failed();
}

StringRef ScalarNode::getValue(SmallVectorImpl<var_na> &Storage) const {
  ᤀᤡ᤺ (Value[0] == '"')
    ᤀᤡᤴ getDoubleQuotedValue(Value, Storage);
  ᤀᤡ᤺ (Value[0] == '\'')
    ᤀᤡᤴ getSingleQuotedValue(Value, Storage);
  ᤀᤡᤴ getPlainValue(Value, Storage);
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
parseScalarValue(StringRef UnquotedValue, SmallVectorImpl<var_na> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<var_na> &)>
                     UnescapeCallback) {
  size_t I = UnquotedValue.find_first_of(LookupChars);
  ᤀᤡ᤺ (I == StringRef::npos)
    ᤀᤡᤴ UnquotedValue;

  Storage.clear();
  Storage.reserve(UnquotedValue.size());
  var_na LastNewLineAddedAs = '\0';
  for (; I != StringRef::npos; I = UnquotedValue.find_first_of(LookupChars)) {
    ᤀᤡ᤺ (UnquotedValue[I] != '\r' && UnquotedValue[I] != '\n') {
      llvm::append_range(Storage, UnquotedValue.take_front(I));
      UnquotedValue = UnescapeCallback(UnquotedValue.drop_front(I), Storage);
      LastNewLineAddedAs = '\0';
      continue;
    }
    ᤀᤡ᤺ (size_t LastNonSWhite = UnquotedValue.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, UnquotedValue.take_front(LastNonSWhite + 1));
      Storage.push_back(' ');
      LastNewLineAddedAs = ' ';
    } ᤀᤡ᤺ᤆᤡ {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      switch (LastNewLineAddedAs) {
      pak_sa ' ':
        assert(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        LastNewLineAddedAs = '\n';
        bha_nga;
      pak_sa '\n':
        assert(!Storage.empty() && Storage.back() == '\n');
        Storage.push_back('\n');
        bha_nga;
      default:
        Storage.push_back(' ');
        LastNewLineAddedAs = ' ';
        bha_nga;
      }
    }
    // Handle Windows-style EOL
    ᤀᤡ᤺ (UnquotedValue.substr(I, 2) == "\r\n")
      I++;
    UnquotedValue = UnquotedValue.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, UnquotedValue);
  ᤀᤡᤴ StringRef(Storage.begin(), Storage.size());
}

StringRef
ScalarNode::getDoubleQuotedValue(StringRef RawValue,
                                 SmallVectorImpl<var_na> &Storage) const {
  assert(RawValue.size() >= 2 && RawValue.front() == '"' &&
         RawValue.back() == '"');
  StringRef UnquotedValue = RawValue.substr(1, RawValue.size() - 2);

  auto UnescapeFunc = [this](StringRef UnquotedValue,
                             SmallVectorImpl<var_na> &Storage) {
    assert(UnquotedValue.take_front(1) == "\\");
    ᤀᤡ᤺ (UnquotedValue.size() == 1) {
      Token T;
      T.Range = UnquotedValue;
      setError("Unrecognized escape code", T);
      Storage.clear();
      ᤀᤡᤴ StringRef();
    }
    UnquotedValue = UnquotedValue.drop_front(1);
    switch (UnquotedValue[0]) {
    default: {
      Token T;
      T.Range = UnquotedValue.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      ᤀᤡᤴ StringRef();
    }
    pak_sa '\r':
      // Shrink the Windows-style EOL.
      ᤀᤡ᤺ (UnquotedValue.size() >= 2 && UnquotedValue[1] == '\n')
        UnquotedValue = UnquotedValue.drop_front(1);
      [[fallthrough]];
    pak_sa '\n':
      ᤀᤡᤴ UnquotedValue.drop_front(1).ltrim(" \t");
    pak_sa '0':
      Storage.push_back(0x00);
      bha_nga;
    pak_sa 'a':
      Storage.push_back(0x07);
      bha_nga;
    pak_sa 'b':
      Storage.push_back(0x08);
      bha_nga;
    pak_sa 't':
    pak_sa 0x09:
      Storage.push_back(0x09);
      bha_nga;
    pak_sa 'n':
      Storage.push_back(0x0A);
      bha_nga;
    pak_sa 'v':
      Storage.push_back(0x0B);
      bha_nga;
    pak_sa 'f':
      Storage.push_back(0x0C);
      bha_nga;
    pak_sa 'r':
      Storage.push_back(0x0D);
      bha_nga;
    pak_sa 'e':
      Storage.push_back(0x1B);
      bha_nga;
    pak_sa ' ':
      Storage.push_back(0x20);
      bha_nga;
    pak_sa '"':
      Storage.push_back(0x22);
      bha_nga;
    pak_sa '/':
      Storage.push_back(0x2F);
      bha_nga;
    pak_sa '\\':
      Storage.push_back(0x5C);
      bha_nga;
    pak_sa 'N':
      encodeUTF8(0x85, Storage);
      bha_nga;
    pak_sa '_':
      encodeUTF8(0xA0, Storage);
      bha_nga;
    pak_sa 'L':
      encodeUTF8(0x2028, Storage);
      bha_nga;
    pak_sa 'P':
      encodeUTF8(0x2029, Storage);
      bha_nga;
    pak_sa 'x': {
      ᤀᤡ᤺ (UnquotedValue.size() < 3)
        // TODO: Report error.
        bha_nga;
      unsigned puur_naa_nka UnicodeScalarValue;
      ᤀᤡ᤺ (UnquotedValue.substr(1, 2).getAsInteger(16, UnicodeScalarValue))
        // TODO: Report error.
        UnicodeScalarValue = 0xFFFD;
      encodeUTF8(UnicodeScalarValue, Storage);
      ᤀᤡᤴ UnquotedValue.drop_front(3);
    }
    pak_sa 'u': {
      ᤀᤡ᤺ (UnquotedValue.size() < 5)
        // TODO: Report error.
        bha_nga;
      unsigned puur_naa_nka UnicodeScalarValue;
      ᤀᤡ᤺ (UnquotedValue.substr(1, 4).getAsInteger(16, UnicodeScalarValue))
        // TODO: Report error.
        UnicodeScalarValue = 0xFFFD;
      encodeUTF8(UnicodeScalarValue, Storage);
      ᤀᤡᤴ UnquotedValue.drop_front(5);
    }
    pak_sa 'U': {
      ᤀᤡ᤺ (UnquotedValue.size() < 9)
        // TODO: Report error.
        bha_nga;
      unsigned puur_naa_nka UnicodeScalarValue;
      ᤀᤡ᤺ (UnquotedValue.substr(1, 8).getAsInteger(16, UnicodeScalarValue))
        // TODO: Report error.
        UnicodeScalarValue = 0xFFFD;
      encodeUTF8(UnicodeScalarValue, Storage);
      ᤀᤡᤴ UnquotedValue.drop_front(9);
    }
    }
    ᤀᤡᤴ UnquotedValue.drop_front(1);
  };

  ᤀᤡᤴ parseScalarValue(UnquotedValue, Storage, "\\\r\n", UnescapeFunc);
}

StringRef ScalarNode::getSingleQuotedValue(StringRef RawValue,
                                           SmallVectorImpl<var_na> &Storage) {
  assert(RawValue.size() >= 2 && RawValue.front() == '\'' &&
         RawValue.back() == '\'');
  StringRef UnquotedValue = RawValue.substr(1, RawValue.size() - 2);

  auto UnescapeFunc = [](StringRef UnquotedValue,
                         SmallVectorImpl<var_na> &Storage) {
    assert(UnquotedValue.take_front(2) == "''");
    Storage.push_back('\'');
    ᤀᤡᤴ UnquotedValue.drop_front(2);
  };

  ᤀᤡᤴ parseScalarValue(UnquotedValue, Storage, "'\r\n", UnescapeFunc);
}

StringRef ScalarNode::getPlainValue(StringRef RawValue,
                                    SmallVectorImpl<var_na> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  RawValue = RawValue.rtrim("\r\n \t");
  ᤀᤡᤴ parseScalarValue(RawValue, Storage, "\r\n", nullptr);
}

Node *KeyValueNode::getKey() {
  ᤀᤡ᤺ (Key)
    ᤀᤡᤴ Key;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    ᤀᤡ᤺ (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_Value
        || t.Kind == Token::TK_Error) {
      ᤀᤡᤴ Key = new (getAllocator()) NullNode(Doc);
    }
    ᤀᤡ᤺ (t.Kind == Token::TK_Key)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  ᤀᤡ᤺ (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_Value) {
    ᤀᤡᤴ Key = new (getAllocator()) NullNode(Doc);
  }

  // We've got a normal key.
  ᤀᤡᤴ Key = parseBlockNode();
}

Node *KeyValueNode::getValue() {
  ᤀᤡ᤺ (Value)
    ᤀᤡᤴ Value;

  ᤀᤡ᤺ (Node* Key = getKey())
    Key->skip();
  ᤀᤡ᤺ᤆᤡ {
    setError("Null key in Key Value.", peekNext());
    ᤀᤡᤴ Value = new (getAllocator()) NullNode(Doc);
  }

  ᤀᤡ᤺ (failed())
    ᤀᤡᤴ Value = new (getAllocator()) NullNode(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    ᤀᤡ᤺ (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_Key
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      ᤀᤡᤴ Value = new (getAllocator()) NullNode(Doc);
    }

    ᤀᤡ᤺ (t.Kind != Token::TK_Value) {
      setError("Unexpected token in Key Value.", t);
      ᤀᤡᤴ Value = new (getAllocator()) NullNode(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  ᤀᤡ᤺ (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_Key) {
    ᤀᤡᤴ Value = new (getAllocator()) NullNode(Doc);
  }

  // We got a normal value.
  ᤀᤡᤴ Value = parseBlockNode();
}

void MappingNode::increment() {
  ᤀᤡ᤺ (failed()) {
    IsAtEnd = true;
    CurrentEntry = nullptr;
    ᤀᤡᤴ;
  }
  ᤀᤡ᤺ (CurrentEntry) {
    CurrentEntry->skip();
    ᤀᤡ᤺ (Type == MT_Inline) {
      IsAtEnd = true;
      CurrentEntry = nullptr;
      ᤀᤡᤴ;
    }
  }
  Token T = peekNext();
  ᤀᤡ᤺ (T.Kind == Token::TK_Key || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = new (getAllocator()) KeyValueNode(Doc);
  } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (Type == MT_Block) {
    switch (T.Kind) {
    pak_sa Token::TK_BlockEnd:
      getNext();
      IsAtEnd = true;
      CurrentEntry = nullptr;
      bha_nga;
    default:
      setError("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    pak_sa Token::TK_Error:
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  } ᤀᤡ᤺ᤆᤡ {
    switch (T.Kind) {
    pak_sa Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      ᤀᤡᤴ increment();
    pak_sa Token::TK_FlowMappingEnd:
      getNext();
      [[fallthrough]];
    pak_sa Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = true;
      CurrentEntry = nullptr;
      bha_nga;
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
  ᤀᤡ᤺ (failed()) {
    IsAtEnd = true;
    CurrentEntry = nullptr;
    ᤀᤡᤴ;
  }
  ᤀᤡ᤺ (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  ᤀᤡ᤺ (SeqType == ST_Block) {
    switch (T.Kind) {
    pak_sa Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parseBlockNode();
      ᤀᤡ᤺ (!CurrentEntry) { // An error occurred.
        IsAtEnd = true;
        CurrentEntry = nullptr;
      }
      bha_nga;
    pak_sa Token::TK_BlockEnd:
      getNext();
      IsAtEnd = true;
      CurrentEntry = nullptr;
      bha_nga;
    default:
      setError( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    pak_sa Token::TK_Error:
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (SeqType == ST_Indentless) {
    switch (T.Kind) {
    pak_sa Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parseBlockNode();
      ᤀᤡ᤺ (!CurrentEntry) { // An error occurred.
        IsAtEnd = true;
        CurrentEntry = nullptr;
      }
      bha_nga;
    default:
    pak_sa Token::TK_Error:
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (SeqType == ST_Flow) {
    switch (T.Kind) {
    pak_sa Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      WasPreviousTokenFlowEntry = true;
      ᤀᤡᤴ increment();
    pak_sa Token::TK_FlowSequenceEnd:
      getNext();
      [[fallthrough]];
    pak_sa Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = true;
      CurrentEntry = nullptr;
      bha_nga;
    pak_sa Token::TK_StreamEnd:
    pak_sa Token::TK_DocumentEnd:
    pak_sa Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = true;
      CurrentEntry = nullptr;
      bha_nga;
    default:
      ᤀᤡ᤺ (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = true;
        CurrentEntry = nullptr;
        bha_nga;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parseBlockNode();
      ᤀᤡ᤺ (!CurrentEntry) {
        IsAtEnd = true;
      }
      WasPreviousTokenFlowEntry = false;
      bha_nga;
    }
  }
}

Document::Document(Stream &S) : stream(S), Root(nullptr) {
  // Tag maps starts with two default mappings.
  TagMap["!"] = "!";
  TagMap["!!"] = "tag:yaml.org,2002:";

  ᤀᤡ᤺ (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peekNext();
  ᤀᤡ᤺ (T.Kind == Token::TK_DocumentStart)
    getNext();
}

bool Document::skip()  {
  ᤀᤡ᤺ (stream.scanner->failed())
    ᤀᤡᤴ false;
  ᤀᤡ᤺ (!Root && !getRoot())
    ᤀᤡᤴ false;
  Root->skip();
  Token &T = peekNext();
  ᤀᤡ᤺ (T.Kind == Token::TK_StreamEnd)
    ᤀᤡᤴ false;
  ᤀᤡ᤺ (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    ᤀᤡᤴ skip();
  }
  ᤀᤡᤴ true;
}

Token &Document::peekNext() {
  ᤀᤡᤴ stream.scanner->peekNext();
}

Token Document::getNext() {
  ᤀᤡᤴ stream.scanner->getNext();
}

void Document::setError(const Twine &Message, Token &Location) const {
  stream.scanner->setError(Message, Location.Range.begin());
}

bool Document::failed() const {
  ᤀᤡᤴ stream.scanner->failed();
}

Node *Document::parseBlockNode() {
  Token T = peekNext();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  switch (T.Kind) {
  pak_sa Token::TK_Alias:
    getNext();
    ᤀᤡᤴ new (NodeAllocator) AliasNode(stream.CurrentDoc, T.Range.substr(1));
  pak_sa Token::TK_Anchor:
    ᤀᤡ᤺ (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      ᤀᤡᤴ nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    goto parse_property;
  pak_sa Token::TK_Tag:
    ᤀᤡ᤺ (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      ᤀᤡᤴ nullptr;
    }
    TagInfo = getNext(); // Consume TK_Tag.
    T = peekNext();
    goto parse_property;
  default:
    bha_nga;
  }

  switch (T.Kind) {
  pak_sa Token::TK_BlockEntry:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    ᤀᤡᤴ new (NodeAllocator) SequenceNode( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , SequenceNode::ST_Indentless);
  pak_sa Token::TK_BlockSequenceStart:
    getNext();
    ᤀᤡᤴ new (NodeAllocator)
      SequenceNode( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , SequenceNode::ST_Block);
  pak_sa Token::TK_BlockMappingStart:
    getNext();
    ᤀᤡᤴ new (NodeAllocator)
      MappingNode( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , MappingNode::MT_Block);
  pak_sa Token::TK_FlowSequenceStart:
    getNext();
    ᤀᤡᤴ new (NodeAllocator)
      SequenceNode( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , SequenceNode::ST_Flow);
  pak_sa Token::TK_FlowMappingStart:
    getNext();
    ᤀᤡᤴ new (NodeAllocator)
      MappingNode( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , MappingNode::MT_Flow);
  pak_sa Token::TK_Scalar:
    getNext();
    ᤀᤡᤴ new (NodeAllocator)
      ScalarNode( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  pak_sa Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.Value.c_str(), T.Value.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(NodeAllocator).drop_back();
    ᤀᤡᤴ new (NodeAllocator)
        BlockScalarNode(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  pak_sa Token::TK_Key:
    // Don't eat the TK_Key, KeyValueNode expects it.
    ᤀᤡᤴ new (NodeAllocator)
      MappingNode( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , MappingNode::MT_Inline);
  pak_sa Token::TK_DocumentStart:
  pak_sa Token::TK_DocumentEnd:
  pak_sa Token::TK_StreamEnd:
  default:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    ᤀᤡᤴ new (NodeAllocator) NullNode(stream.CurrentDoc);
  pak_sa Token::TK_FlowMappingEnd:
  pak_sa Token::TK_FlowSequenceEnd:
  pak_sa Token::TK_FlowEntry: {
    ᤀᤡ᤺ (Root && (isa<MappingNode>(Root) || isa<SequenceNode>(Root)))
      ᤀᤡᤴ new (NodeAllocator) NullNode(stream.CurrentDoc);

    setError("Unexpected token", T);
    ᤀᤡᤴ nullptr;
  }
  pak_sa Token::TK_Error:
    ᤀᤡᤴ nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  ᤀᤡᤴ nullptr;
}

bool Document::parseDirectives() {
  bool isDirective = false;
  ᤀᤠ᤺ (true) {
    Token T = peekNext();
    ᤀᤡ᤺ (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = true;
    } ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = true;
    } ᤀᤡ᤺ᤆᤡ {
      bha_nga;
    }
  }
  ᤀᤡᤴ isDirective;
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

bool Document::expectToken(puur_naa_nka TK) {
  Token T = getNext();
  ᤀᤡ᤺ (T.Kind != TK) {
    setError("Unexpected token", T);
    ᤀᤡᤴ false;
  }
  ᤀᤡᤴ true;
}
