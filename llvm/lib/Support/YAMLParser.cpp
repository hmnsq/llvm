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
  ถ้า (Input.empty())
    คืน {UEF_Unknown, 0};

  switch (uint8_t(Input[0])) {
  pak_sa 0x00:
    ถ้า (Input.size() >= 4) {
      ถ้า (  Input[1] == 0
         && uint8_t(Input[2]) == 0xFE
         && uint8_t(Input[3]) == 0xFF)
        คืน {UEF_UTF32_BE, 4};
      ถ้า (Input[1] == 0 && Input[2] == 0 && Input[3] != 0)
        คืน {UEF_UTF32_BE, 0};
    }

    ถ้า (Input.size() >= 2 && Input[1] != 0)
      คืน {UEF_UTF16_BE, 0};
    คืน {UEF_Unknown, 0};
  pak_sa 0xFF:
    ถ้า (  Input.size() >= 4
       && uint8_t(Input[1]) == 0xFE
       && Input[2] == 0
       && Input[3] == 0)
      คืน {UEF_UTF32_LE, 4};

    ถ้า (Input.size() >= 2 && uint8_t(Input[1]) == 0xFE)
      คืน {UEF_UTF16_LE, 2};
    คืน {UEF_Unknown, 0};
  pak_sa 0xFE:
    ถ้า (Input.size() >= 2 && uint8_t(Input[1]) == 0xFF)
      คืน {UEF_UTF16_BE, 2};
    คืน {UEF_Unknown, 0};
  pak_sa 0xEF:
    ถ้า (  Input.size() >= 3
       && uint8_t(Input[1]) == 0xBB
       && uint8_t(Input[2]) == 0xBF)
      คืน {UEF_UTF8, 3};
    คืน {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  ถ้า (Input.size() >= 4 && Input[1] == 0 && Input[2] == 0 && Input[3] == 0)
    คืน {UEF_UTF32_LE, 0};

  ถ้า (Input.size() >= 2 && Input[1] == 0)
    คืน {UEF_UTF16_LE, 0};

  คืน {UEF_UTF8, 0};
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
    คืน Tok == Other.Tok;
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
  ถ้า (Position < End && (*Position & 0x80) == 0) {
    คืน {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  ถ้า (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    ถ้า (codepoint >= 0x80)
      คืน {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  ถ้า (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    ถ้า (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      คืน {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  ถ้า (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    ถ้า (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      คืน {codepoint, 4};
  }
  คืน {0, 0};
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
    ถ้า (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    ถ้า (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    ถ้า (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = true;
  }

  /// Returns true if an error occurred while parsing.
  bool failed() {
    คืน Failed;
  }

private:
  void init(MemoryBufferRef Buffer);

  StringRef currentInput() {
    คืน StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    คืน ::decodeUTF8(StringRef(Position, End - Position));
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
  bool unrollIndent(จำนวนเต็ม ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  bool rollIndent( จำนวนเต็ม ToColumn
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
  จำนวนเต็ม Indent;

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
  SmallVector<จำนวนเต็ม, 4> Indents;

  /// Potential simple keys.
  SmallVector<SimpleKey, 4> SimpleKeys;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
static void encodeUTF8( uint32_t UnicodeScalarValue
                      , SmallVectorImpl<var_na> &Result) {
  ถ้า (UnicodeScalarValue <= 0x7F) {
    Result.push_back(UnicodeScalarValue & 0x7F);
  } มิฉะนั้น ถ้า (UnicodeScalarValue <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((UnicodeScalarValue & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (UnicodeScalarValue & 0x3F);
    Result.push_back(FirstByte);
    Result.push_back(SecondByte);
  } มิฉะนั้น ถ้า (UnicodeScalarValue <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((UnicodeScalarValue & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((UnicodeScalarValue & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (UnicodeScalarValue & 0x3F);
    Result.push_back(FirstByte);
    Result.push_back(SecondByte);
    Result.push_back(ThirdByte);
  } มิฉะนั้น ถ้า (UnicodeScalarValue <= 0x10FFFF) {
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
  ขณะ (true) {
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
    ถ้า (T.Kind == Token::TK_StreamEnd)
      bha_nga;
    มิฉะนั้น ถ้า (T.Kind == Token::TK_Error)
      คืน false;
  }
  คืน true;
}

bool yaml::scanTokens(StringRef Input) {
  SourceMgr SM;
  Scanner scanner(Input, SM);
  ขณะ (true) {
    Token T = scanner.getNext();
    ถ้า (T.Kind == Token::TK_StreamEnd)
      bha_nga;
    มิฉะนั้น ถ้า (T.Kind == Token::TK_Error)
      คืน false;
  }
  คืน true;
}

std::string yaml::escape(StringRef Input, bool EscapePrintable) {
  std::string EscapedInput;
  สำหรับ (StringRef::iterator i = Input.begin(), e = Input.end(); i != e; ++i) {
    ถ้า (*i == '\\')
      EscapedInput += "\\\\";
    มิฉะนั้น ถ้า (*i == '"')
      EscapedInput += "\\\"";
    มิฉะนั้น ถ้า (*i == 0)
      EscapedInput += "\\0";
    มิฉะนั้น ถ้า (*i == 0x07)
      EscapedInput += "\\a";
    มิฉะนั้น ถ้า (*i == 0x08)
      EscapedInput += "\\b";
    มิฉะนั้น ถ้า (*i == 0x09)
      EscapedInput += "\\t";
    มิฉะนั้น ถ้า (*i == 0x0A)
      EscapedInput += "\\n";
    มิฉะนั้น ถ้า (*i == 0x0B)
      EscapedInput += "\\v";
    มิฉะนั้น ถ้า (*i == 0x0C)
      EscapedInput += "\\f";
    มิฉะนั้น ถ้า (*i == 0x0D)
      EscapedInput += "\\r";
    มิฉะนั้น ถ้า (*i == 0x1B)
      EscapedInput += "\\e";
    มิฉะนั้น ถ้า ((unsigned var_na)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      EscapedInput += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } มิฉะนั้น ถ้า (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded UnicodeScalarValue
        = decodeUTF8(StringRef(i, Input.end() - i));
      ถ้า (UnicodeScalarValue.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(EscapedInput, Val);
        // FIXME: Error reporting.
        คืน EscapedInput;
      }
      ถ้า (UnicodeScalarValue.first == 0x85)
        EscapedInput += "\\N";
      มิฉะนั้น ถ้า (UnicodeScalarValue.first == 0xA0)
        EscapedInput += "\\_";
      มิฉะนั้น ถ้า (UnicodeScalarValue.first == 0x2028)
        EscapedInput += "\\L";
      มิฉะนั้น ถ้า (UnicodeScalarValue.first == 0x2029)
        EscapedInput += "\\P";
      มิฉะนั้น ถ้า (!EscapePrintable &&
               sys::unicode::isPrintable(UnicodeScalarValue.first))
        EscapedInput += StringRef(i, UnicodeScalarValue.second);
      มิฉะนั้น {
        std::string HexStr = utohexstr(UnicodeScalarValue.first);
        ถ้า (HexStr.size() <= 2)
          EscapedInput += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        มิฉะนั้น ถ้า (HexStr.size() <= 4)
          EscapedInput += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        มิฉะนั้น ถ้า (HexStr.size() <= 8)
          EscapedInput += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += UnicodeScalarValue.second - 1;
    } มิฉะนั้น {
      EscapedInput.push_back(*i);
    }
  }
  คืน EscapedInput;
}

std::optional<bool> yaml::parseBool(StringRef S) {
  switch (S.size()) {
  pak_sa 1:
    switch (S.front()) {
    pak_sa 'y':
    pak_sa 'Y':
      คืน true;
    pak_sa 'n':
    pak_sa 'N':
      คืน false;
    default:
      คืน std::nullopt;
    }
  pak_sa 2:
    switch (S.front()) {
    pak_sa 'O':
      ถ้า (S[1] == 'N') // ON
        คืน true;
      [[fallthrough]];
    pak_sa 'o':
      ถ้า (S[1] == 'n') //[Oo]n
        คืน true;
      คืน std::nullopt;
    pak_sa 'N':
      ถ้า (S[1] == 'O') // NO
        คืน false;
      [[fallthrough]];
    pak_sa 'n':
      ถ้า (S[1] == 'o') //[Nn]o
        คืน false;
      คืน std::nullopt;
    default:
      คืน std::nullopt;
    }
  pak_sa 3:
    switch (S.front()) {
    pak_sa 'O':
      ถ้า (S.drop_front() == "FF") // OFF
        คืน false;
      [[fallthrough]];
    pak_sa 'o':
      ถ้า (S.drop_front() == "ff") //[Oo]ff
        คืน false;
      คืน std::nullopt;
    pak_sa 'Y':
      ถ้า (S.drop_front() == "ES") // YES
        คืน true;
      [[fallthrough]];
    pak_sa 'y':
      ถ้า (S.drop_front() == "es") //[Yy]es
        คืน true;
      คืน std::nullopt;
    default:
      คืน std::nullopt;
    }
  pak_sa 4:
    switch (S.front()) {
    pak_sa 'T':
      ถ้า (S.drop_front() == "RUE") // TRUE
        คืน true;
      [[fallthrough]];
    pak_sa 't':
      ถ้า (S.drop_front() == "rue") //[Tt]rue
        คืน true;
      คืน std::nullopt;
    default:
      คืน std::nullopt;
    }
  pak_sa 5:
    switch (S.front()) {
    pak_sa 'F':
      ถ้า (S.drop_front() == "ALSE") // FALSE
        คืน false;
      [[fallthrough]];
    pak_sa 'f':
      ถ้า (S.drop_front() == "alse") //[Ff]alse
        คืน false;
      คืน std::nullopt;
    default:
      คืน std::nullopt;
    }
  default:
    คืน std::nullopt;
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
  ขณะ (true) {
    ถ้า (TokenQueue.empty() || NeedMore) {
      ถ้า (!fetchMoreTokens()) {
        TokenQueue.clear();
        SimpleKeys.clear();
        TokenQueue.push_back(Token());
        คืน TokenQueue.front();
      }
    }
    assert(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    removeStaleSimpleKeyCandidates();
    SimpleKey SK;
    SK.Tok = TokenQueue.begin();
    ถ้า (!is_contained(SimpleKeys, SK))
      bha_nga;
    มิฉะนั้น
      NeedMore = true;
  }
  คืน TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  ถ้า (!TokenQueue.empty())
    TokenQueue.pop_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  ถ้า (TokenQueue.empty())
    TokenQueue.resetAlloc();

  คืน Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  ถ้า (Position == End)
    คืน Position;
  // Check 7 bit c-printable - b-char.
  ถ้า (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    คืน Position + 1;

  // Check for valid UTF-8.
  ถ้า (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    ถ้า (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      คืน Position + u8d.second;
  }
  คืน Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  ถ้า (Position == End)
    คืน Position;
  ถ้า (*Position == 0x0D) {
    ถ้า (Position + 1 != End && *(Position + 1) == 0x0A)
      คืน Position + 2;
    คืน Position + 1;
  }

  ถ้า (*Position == 0x0A)
    คืน Position + 1;
  คืน Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  ถ้า (Position == End)
    คืน Position;
  ถ้า (*Position == ' ')
    คืน Position + 1;
  คืน Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  ถ้า (Position == End)
    คืน Position;
  ถ้า (*Position == ' ' || *Position == '\t')
    คืน Position + 1;
  คืน Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  ถ้า (Position == End)
    คืน Position;
  ถ้า (*Position == ' ' || *Position == '\t')
    คืน Position;
  คืน skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  ขณะ (true) {
    StringRef::iterator i = (this->*Func)(Position);
    ถ้า (i == Position)
      bha_nga;
    Position = i;
  }
  คืน Position;
}

void Scanner::advanceWhile(SkipWhileFunc Func) {
  auto Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

static bool is_ns_hex_digit(const var_na C) { คืน isAlnum(C); }

static bool is_ns_word_char(const var_na C) { คืน C == '-' || isAlpha(C); }

void Scanner::scan_ns_uri_char() {
  ขณะ (true) {
    ถ้า (Current == End)
      bha_nga;
    ถ้า ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } มิฉะนั้น {
      bha_nga;
    }
  }
}

bool Scanner::consume(uint32_t Expected) {
  ถ้า (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    คืน false;
  }
  ถ้า (Current == End)
    คืน false;
  ถ้า (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    คืน false;
  }
  ถ้า (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    คืน true;
  }
  คืน false;
}

void Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  assert(Current <= End && "Skipped past the end");
}

bool Scanner::isBlankOrBreak(StringRef::iterator Position) {
  ถ้า (Position == End)
    คืน false;
  คืน *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

bool Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  ถ้า (Position == End || isBlankOrBreak(Position))
    คืน false;
  ถ้า (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    คืน false;
  คืน true;
}

bool Scanner::isLineEmpty(StringRef Line) {
  สำหรับ (const auto *Position = Line.begin(); Position != Line.end(); ++Position)
    ถ้า (!isBlankOrBreak(Position))
      คืน false;
  คืน true;
}

bool Scanner::consumeLineBreakIfPresent() {
  auto Next = skip_b_break(Current);
  ถ้า (Next == Current)
    คืน false;
  Column = 0;
  ++Line;
  Current = Next;
  คืน true;
}

void Scanner::saveSimpleKeyCandidate( TokenQueueT::iterator Tok
                                    , unsigned AtColumn
                                    , bool IsRequired) {
  ถ้า (IsSimpleKeyAllowed) {
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
  สำหรับ (SmallVectorImpl<SimpleKey>::iterator i = SimpleKeys.begin();
                                            i != SimpleKeys.end();) {
    ถ้า (i->Line != Line || i->Column + 1024 < Column) {
      ถ้า (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = SimpleKeys.erase(i);
    } มิฉะนั้น {
      ++i;
    }
  }
}

void Scanner::removeSimpleKeyCandidatesOnFlowLevel(unsigned Level) {
  ถ้า (!SimpleKeys.empty() && (SimpleKeys.end() - 1)->FlowLevel == Level)
    SimpleKeys.pop_back();
}

bool Scanner::unrollIndent(จำนวนเต็ม ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  ถ้า (FlowLevel != 0)
    คืน true;

  ขณะ (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.push_back(T);
    Indent = Indents.pop_back_val();
  }

  คืน true;
}

bool Scanner::rollIndent( จำนวนเต็ม ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator InsertPoint) {
  ถ้า (FlowLevel)
    คืน true;
  ถ้า (Indent < ToColumn) {
    Indents.push_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.insert(InsertPoint, T);
  }
  คืน true;
}

void Scanner::skipComment() {
  ถ้า (Current == End || *Current != '#')
    คืน;
  ขณะ (true) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    ถ้า (I == Current)
      bha_nga;
    Current = I;
    ++Column;
  }
}

void Scanner::scanToNextToken() {
  ขณะ (true) {
    ขณะ (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    ถ้า (i == Current)
      bha_nga;
    Current = i;
    ++Line;
    Column = 0;
    // New lines may start a simple key.
    ถ้า (!FlowLevel)
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
  คืน true;
}

bool Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  ถ้า (Column != 0) {
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
  คืน true;
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
  ถ้า (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.push_back(T);
    คืน true;
  } มิฉะนั้น ถ้า(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.push_back(T);
    คืน true;
  }
  คืน false;
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
  คืน true;
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
  คืน true;
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
  ถ้า (FlowLevel)
    --FlowLevel;
  คืน true;
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
  คืน true;
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
  คืน true;
}

bool Scanner::scanKey() {
  ถ้า (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  removeSimpleKeyCandidatesOnFlowLevel(FlowLevel);
  IsSimpleKeyAllowed = !FlowLevel;
  IsAdjacentValueAllowedInFlow = false;

  Token T;
  T.Kind = Token::TK_Key;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);
  คืน true;
}

bool Scanner::scanValue() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  ถ้า (!SimpleKeys.empty()) {
    SimpleKey SK = SimpleKeys.pop_back_val();
    Token T;
    T.Kind = Token::TK_Key;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    สำหรับ (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      ถ้า (i == SK.Tok)
        bha_nga;
    }
    ถ้า (i == e) {
      Failed = true;
      คืน false;
    }
    i = TokenQueue.insert(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    IsSimpleKeyAllowed = false;
  } มิฉะนั้น {
    ถ้า (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    IsSimpleKeyAllowed = !FlowLevel;
  }
  IsAdjacentValueAllowedInFlow = false;

  Token T;
  T.Kind = Token::TK_Value;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);
  คืน true;
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
  ขณะ (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  คืน (Position - 1 - I) % 2 == 1;
}

bool Scanner::scanFlowScalar(bool IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  ถ้า (IsDoubleQuoted) {
    do {
      ++Current;
      ขณะ (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } ขณะ (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } มิฉะนั้น {
    skip(1);
    ขณะ (Current != End) {
      // Skip a ' followed by another '.
      ถ้า (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        continue;
      } มิฉะนั้น ถ้า (*Current == '\'')
        bha_nga;
      StringRef::iterator i = skip_nb_char(Current);
      ถ้า (i == Current) {
        i = skip_b_break(Current);
        ถ้า (i == Current)
          bha_nga;
        Current = i;
        Column = 0;
        ++Line;
      } มิฉะนั้น {
        ถ้า (i == End)
          bha_nga;
        Current = i;
        ++Column;
      }
    }
  }

  ถ้า (Current == End) {
    setError("Expected quote at end of scalar", Current);
    คืน false;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, false);

  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = true;

  คืน true;
}

bool Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  unsigned LeadingBlanks = 0;
  assert(Indent >= -1 && "Indent must be >= -1 !");
  unsigned indent = static_cast<unsigned>(Indent + 1);
  ขณะ (Current != End) {
    ถ้า (*Current == '#')
      bha_nga;

    ขณะ (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      ถ้า (i == Current)
        bha_nga;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    ถ้า (!isBlankOrBreak(Current))
      bha_nga;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    ขณะ (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      ถ้า (i != Tmp) {
        ถ้า (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          คืน false;
        }
        Tmp = i;
        ++Column;
      } มิฉะนั้น {
        i = skip_b_break(Tmp);
        ถ้า (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++Line;
      }
    }

    ถ้า (!FlowLevel && Column < indent)
      bha_nga;

    Current = Tmp;
  }
  ถ้า (Start == Current) {
    setError("Got empty plain scalar", Start);
    คืน false;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  // Plain scalars can be simple keys.
  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, false);

  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = false;

  คืน true;
}

bool Scanner::scanAliasOrAnchor(bool IsAlias) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1);
  ขณะ (Current != End) {
    ถ้า (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      bha_nga;
    StringRef::iterator i = skip_ns_char(Current);
    ถ้า (i == Current)
      bha_nga;
    Current = i;
    ++Column;
  }

  ถ้า (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    คืน false;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  // Alias and anchors can be simple keys.
  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, false);

  IsSimpleKeyAllowed = false;
  IsAdjacentValueAllowedInFlow = false;

  คืน true;
}

bool Scanner::scanBlockScalarIndicators(var_na &StyleIndicator,
                                        var_na &ChompingIndicator,
                                        unsigned &IndentIndicator,
                                        bool &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  ถ้า (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    คืน false;
  คืน true;
}

var_na Scanner::scanBlockStyleIndicator() {
  var_na Indicator = ' ';
  ถ้า (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  คืน Indicator;
}

var_na Scanner::scanBlockChompingIndicator() {
  var_na Indicator = ' ';
  ถ้า (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  คืน Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
static unsigned getChompedLineBreaks(var_na ChompingIndicator,
                                     unsigned LineBreaks, StringRef Str) {
  ถ้า (ChompingIndicator == '-') // Strip all line breaks.
    คืน 0;
  ถ้า (ChompingIndicator == '+') // Keep all line breaks.
    คืน LineBreaks;
  // Clip trailing lines.
  คืน Str.empty() ? 0 : 1;
}

unsigned Scanner::scanBlockIndentationIndicator() {
  unsigned Indent = 0;
  ถ้า (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = unsigned(*Current - '0');
    skip(1);
  }
  คืน Indent;
}

bool Scanner::scanBlockScalarHeader(var_na &ChompingIndicator,
                                    unsigned &IndentIndicator, bool &IsDone) {
  auto Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  ถ้า (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  ถ้า (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.push_back(T);
    IsDone = true;
    คืน true;
  }

  ถ้า (!consumeLineBreakIfPresent()) {
    setError("Expected a line break after block scalar header", Current);
    คืน false;
  }
  คืน true;
}

bool Scanner::findBlockScalarIndent(unsigned &BlockIndent,
                                    unsigned BlockExitIndent,
                                    unsigned &LineBreaks, bool &IsDone) {
  unsigned MaxAllSpaceLineCharacters = 0;
  StringRef::iterator LongestAllSpaceLine;

  ขณะ (true) {
    advanceWhile(&Scanner::skip_s_space);
    ถ้า (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      ถ้า (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = true;
        คืน true;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      ถ้า (MaxAllSpaceLineCharacters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            LongestAllSpaceLine);
        คืน false;
      }
      คืน true;
    }
    ถ้า (skip_b_break(Current) != Current &&
        Column > MaxAllSpaceLineCharacters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      MaxAllSpaceLineCharacters = Column;
      LongestAllSpaceLine = Current;
    }

    // Check for EOF.
    ถ้า (Current == End) {
      IsDone = true;
      คืน true;
    }

    ถ้า (!consumeLineBreakIfPresent()) {
      IsDone = true;
      คืน true;
    }
    ++LineBreaks;
  }
  คืน true;
}

bool Scanner::scanBlockScalarIndent(unsigned BlockIndent,
                                    unsigned BlockExitIndent, bool &IsDone) {
  // Skip the indentation.
  ขณะ (Column < BlockIndent) {
    auto I = skip_s_space(Current);
    ถ้า (I == Current)
      bha_nga;
    Current = I;
    ++Column;
  }

  ถ้า (skip_nb_char(Current) == Current)
    คืน true;

  ถ้า (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = true;
    คืน true;
  }

  ถ้า (Column < BlockIndent) {
    ถ้า (Current != End && *Current == '#') { // Trailing comment.
      IsDone = true;
      คืน true;
    }
    setError("A text line is less indented than the block scalar", Current);
    คืน false;
  }
  คืน true; // A normal text line.
}

bool Scanner::scanBlockScalar(bool IsLiteral) {
  assert(*Current == '|' || *Current == '>');
  var_na StyleIndicator;
  var_na ChompingIndicator;
  unsigned BlockIndent;
  bool IsDone = false;
  ถ้า (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    คืน false;
  ถ้า (IsDone)
    คืน true;
  bool IsFolded = StyleIndicator == '>';

  const auto *Start = Current;
  unsigned BlockExitIndent = Indent < 0 ? 0 : (unsigned)Indent;
  unsigned LineBreaks = 0;
  ถ้า (BlockIndent == 0) {
    ถ้า (!findBlockScalarIndent(BlockIndent, BlockExitIndent, LineBreaks,
                               IsDone))
      คืน false;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  ขณะ (!IsDone) {
    ถ้า (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      คืน false;
    ถ้า (IsDone)
      bha_nga;

    // Parse the current line.
    auto LineStart = Current;
    advanceWhile(&Scanner::skip_nb_char);
    ถ้า (LineStart != Current) {
      ถ้า (LineBreaks && IsFolded && !Scanner::isLineEmpty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        ถ้า (LineBreaks == 1) {
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
    ถ้า (Current == End)
      bha_nga;

    ถ้า (!consumeLineBreakIfPresent())
      bha_nga;
    ++LineBreaks;
  }

  ถ้า (Current == End && !LineBreaks)
    // Ensure that there is at least one line break before the end of file.
    LineBreaks = 1;
  Str.append(getChompedLineBreaks(ChompingIndicator, LineBreaks, Str), '\n');

  // New lines may start a simple key.
  ถ้า (!FlowLevel)
    IsSimpleKeyAllowed = true;
  IsAdjacentValueAllowedInFlow = false;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.Value = std::string(Str);
  TokenQueue.push_back(T);
  คืน true;
}

bool Scanner::scanTag() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1); // Eat !.
  ถ้า (Current == End || isBlankOrBreak(Current)); // An empty tag.
  มิฉะนั้น ถ้า (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    ถ้า (!consume('>'))
      คืน false;
  } มิฉะนั้น {
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

  คืน true;
}

bool Scanner::fetchMoreTokens() {
  ถ้า (IsStartOfStream)
    คืน scanStreamStart();

  scanToNextToken();

  ถ้า (Current == End)
    คืน scanStreamEnd();

  removeStaleSimpleKeyCandidates();

  unrollIndent(Column);

  ถ้า (Column == 0 && *Current == '%')
    คืน scanDirective();

  ถ้า (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    คืน scanDocumentIndicator(true);

  ถ้า (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    คืน scanDocumentIndicator(false);

  ถ้า (*Current == '[')
    คืน scanFlowCollectionStart(true);

  ถ้า (*Current == '{')
    คืน scanFlowCollectionStart(false);

  ถ้า (*Current == ']')
    คืน scanFlowCollectionEnd(true);

  ถ้า (*Current == '}')
    คืน scanFlowCollectionEnd(false);

  ถ้า (*Current == ',')
    คืน scanFlowEntry();

  ถ้า (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    คืน scanBlockEntry();

  ถ้า (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    คืน scanKey();

  ถ้า (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || IsAdjacentValueAllowedInFlow))
    คืน scanValue();

  ถ้า (*Current == '*')
    คืน scanAliasOrAnchor(true);

  ถ้า (*Current == '&')
    คืน scanAliasOrAnchor(false);

  ถ้า (*Current == '!')
    คืน scanTag();

  ถ้า (*Current == '|' && !FlowLevel)
    คืน scanBlockScalar(true);

  ถ้า (*Current == '>' && !FlowLevel)
    คืน scanBlockScalar(false);

  ถ้า (*Current == '\'')
    คืน scanFlowScalar(false);

  ถ้า (*Current == '"')
    คืน scanFlowScalar(true);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  ถ้า ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    คืน scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  คืน false;
}

Stream::Stream(StringRef Input, SourceMgr &SM, bool ShowColors,
               std::error_code *EC)
    : scanner(new Scanner(Input, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef InputBuffer, SourceMgr &SM, bool ShowColors,
               std::error_code *EC)
    : scanner(new Scanner(InputBuffer, SM, ShowColors, EC)) {}

Stream::~Stream() = default;

bool Stream::failed() { คืน scanner->failed(); }

void Stream::printError(Node *N, const Twine &Msg, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), Msg, Kind);
}

void Stream::printError(const SMRange &Range, const Twine &Msg,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, Msg, Range);
}

document_iterator Stream::begin() {
  ถ้า (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(new Document(*this));
  คืน document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  คืน document_iterator();
}

void Stream::skip() {
  สำหรับ (Document &Doc : *this)
    Doc.skip();
}

Node::Node(unsigned จำนวนเต็ม Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string Node::getVerbatimTag() const {
  StringRef Raw = getRawTag();
  ถ้า (!Raw.empty() && Raw != "!") {
    std::string Ret;
    ถ้า (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      คืน Ret;
    } มิฉะนั้น ถ้า (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      คืน Ret;
    } มิฉะนั้น {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      ถ้า (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      มิฉะนั้น {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      คืน Ret;
    }
  }

  switch (getType()) {
  pak_sa NK_Null:
    คืน "tag:yaml.org,2002:null";
  pak_sa NK_Scalar:
  pak_sa NK_BlockScalar:
    // TODO: Tag resolution.
    คืน "tag:yaml.org,2002:str";
  pak_sa NK_Mapping:
    คืน "tag:yaml.org,2002:map";
  pak_sa NK_Sequence:
    คืน "tag:yaml.org,2002:seq";
  }

  คืน "";
}

Token &Node::peekNext() {
  คืน Doc->peekNext();
}

Token Node::getNext() {
  คืน Doc->getNext();
}

Node *Node::parseBlockNode() {
  คืน Doc->parseBlockNode();
}

BumpPtrAllocator &Node::getAllocator() {
  คืน Doc->NodeAllocator;
}

void Node::setError(const Twine &Msg, Token &Tok) const {
  Doc->setError(Msg, Tok);
}

bool Node::failed() const {
  คืน Doc->failed();
}

StringRef ScalarNode::getValue(SmallVectorImpl<var_na> &Storage) const {
  ถ้า (Value[0] == '"')
    คืน getDoubleQuotedValue(Value, Storage);
  ถ้า (Value[0] == '\'')
    คืน getSingleQuotedValue(Value, Storage);
  คืน getPlainValue(Value, Storage);
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
  ถ้า (I == StringRef::npos)
    คืน UnquotedValue;

  Storage.clear();
  Storage.reserve(UnquotedValue.size());
  var_na LastNewLineAddedAs = '\0';
  สำหรับ (; I != StringRef::npos; I = UnquotedValue.find_first_of(LookupChars)) {
    ถ้า (UnquotedValue[I] != '\r' && UnquotedValue[I] != '\n') {
      llvm::append_range(Storage, UnquotedValue.take_front(I));
      UnquotedValue = UnescapeCallback(UnquotedValue.drop_front(I), Storage);
      LastNewLineAddedAs = '\0';
      continue;
    }
    ถ้า (size_t LastNonSWhite = UnquotedValue.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, UnquotedValue.take_front(LastNonSWhite + 1));
      Storage.push_back(' ');
      LastNewLineAddedAs = ' ';
    } มิฉะนั้น {
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
    ถ้า (UnquotedValue.substr(I, 2) == "\r\n")
      I++;
    UnquotedValue = UnquotedValue.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, UnquotedValue);
  คืน StringRef(Storage.begin(), Storage.size());
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
    ถ้า (UnquotedValue.size() == 1) {
      Token T;
      T.Range = UnquotedValue;
      setError("Unrecognized escape code", T);
      Storage.clear();
      คืน StringRef();
    }
    UnquotedValue = UnquotedValue.drop_front(1);
    switch (UnquotedValue[0]) {
    default: {
      Token T;
      T.Range = UnquotedValue.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      คืน StringRef();
    }
    pak_sa '\r':
      // Shrink the Windows-style EOL.
      ถ้า (UnquotedValue.size() >= 2 && UnquotedValue[1] == '\n')
        UnquotedValue = UnquotedValue.drop_front(1);
      [[fallthrough]];
    pak_sa '\n':
      คืน UnquotedValue.drop_front(1).ltrim(" \t");
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
      ถ้า (UnquotedValue.size() < 3)
        // TODO: Report error.
        bha_nga;
      unsigned จำนวนเต็ม UnicodeScalarValue;
      ถ้า (UnquotedValue.substr(1, 2).getAsInteger(16, UnicodeScalarValue))
        // TODO: Report error.
        UnicodeScalarValue = 0xFFFD;
      encodeUTF8(UnicodeScalarValue, Storage);
      คืน UnquotedValue.drop_front(3);
    }
    pak_sa 'u': {
      ถ้า (UnquotedValue.size() < 5)
        // TODO: Report error.
        bha_nga;
      unsigned จำนวนเต็ม UnicodeScalarValue;
      ถ้า (UnquotedValue.substr(1, 4).getAsInteger(16, UnicodeScalarValue))
        // TODO: Report error.
        UnicodeScalarValue = 0xFFFD;
      encodeUTF8(UnicodeScalarValue, Storage);
      คืน UnquotedValue.drop_front(5);
    }
    pak_sa 'U': {
      ถ้า (UnquotedValue.size() < 9)
        // TODO: Report error.
        bha_nga;
      unsigned จำนวนเต็ม UnicodeScalarValue;
      ถ้า (UnquotedValue.substr(1, 8).getAsInteger(16, UnicodeScalarValue))
        // TODO: Report error.
        UnicodeScalarValue = 0xFFFD;
      encodeUTF8(UnicodeScalarValue, Storage);
      คืน UnquotedValue.drop_front(9);
    }
    }
    คืน UnquotedValue.drop_front(1);
  };

  คืน parseScalarValue(UnquotedValue, Storage, "\\\r\n", UnescapeFunc);
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
    คืน UnquotedValue.drop_front(2);
  };

  คืน parseScalarValue(UnquotedValue, Storage, "'\r\n", UnescapeFunc);
}

StringRef ScalarNode::getPlainValue(StringRef RawValue,
                                    SmallVectorImpl<var_na> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  RawValue = RawValue.rtrim("\r\n \t");
  คืน parseScalarValue(RawValue, Storage, "\r\n", nullptr);
}

Node *KeyValueNode::getKey() {
  ถ้า (Key)
    คืน Key;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    ถ้า (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_Value
        || t.Kind == Token::TK_Error) {
      คืน Key = new (getAllocator()) NullNode(Doc);
    }
    ถ้า (t.Kind == Token::TK_Key)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  ถ้า (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_Value) {
    คืน Key = new (getAllocator()) NullNode(Doc);
  }

  // We've got a normal key.
  คืน Key = parseBlockNode();
}

Node *KeyValueNode::getValue() {
  ถ้า (Value)
    คืน Value;

  ถ้า (Node* Key = getKey())
    Key->skip();
  มิฉะนั้น {
    setError("Null key in Key Value.", peekNext());
    คืน Value = new (getAllocator()) NullNode(Doc);
  }

  ถ้า (failed())
    คืน Value = new (getAllocator()) NullNode(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    ถ้า (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_Key
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      คืน Value = new (getAllocator()) NullNode(Doc);
    }

    ถ้า (t.Kind != Token::TK_Value) {
      setError("Unexpected token in Key Value.", t);
      คืน Value = new (getAllocator()) NullNode(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  ถ้า (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_Key) {
    คืน Value = new (getAllocator()) NullNode(Doc);
  }

  // We got a normal value.
  คืน Value = parseBlockNode();
}

void MappingNode::increment() {
  ถ้า (failed()) {
    IsAtEnd = true;
    CurrentEntry = nullptr;
    คืน;
  }
  ถ้า (CurrentEntry) {
    CurrentEntry->skip();
    ถ้า (Type == MT_Inline) {
      IsAtEnd = true;
      CurrentEntry = nullptr;
      คืน;
    }
  }
  Token T = peekNext();
  ถ้า (T.Kind == Token::TK_Key || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = new (getAllocator()) KeyValueNode(Doc);
  } มิฉะนั้น ถ้า (Type == MT_Block) {
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
  } มิฉะนั้น {
    switch (T.Kind) {
    pak_sa Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      คืน increment();
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
  ถ้า (failed()) {
    IsAtEnd = true;
    CurrentEntry = nullptr;
    คืน;
  }
  ถ้า (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  ถ้า (SeqType == ST_Block) {
    switch (T.Kind) {
    pak_sa Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parseBlockNode();
      ถ้า (!CurrentEntry) { // An error occurred.
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
  } มิฉะนั้น ถ้า (SeqType == ST_Indentless) {
    switch (T.Kind) {
    pak_sa Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parseBlockNode();
      ถ้า (!CurrentEntry) { // An error occurred.
        IsAtEnd = true;
        CurrentEntry = nullptr;
      }
      bha_nga;
    default:
    pak_sa Token::TK_Error:
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  } มิฉะนั้น ถ้า (SeqType == ST_Flow) {
    switch (T.Kind) {
    pak_sa Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      WasPreviousTokenFlowEntry = true;
      คืน increment();
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
      ถ้า (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = true;
        CurrentEntry = nullptr;
        bha_nga;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parseBlockNode();
      ถ้า (!CurrentEntry) {
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

  ถ้า (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peekNext();
  ถ้า (T.Kind == Token::TK_DocumentStart)
    getNext();
}

bool Document::skip()  {
  ถ้า (stream.scanner->failed())
    คืน false;
  ถ้า (!Root && !getRoot())
    คืน false;
  Root->skip();
  Token &T = peekNext();
  ถ้า (T.Kind == Token::TK_StreamEnd)
    คืน false;
  ถ้า (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    คืน skip();
  }
  คืน true;
}

Token &Document::peekNext() {
  คืน stream.scanner->peekNext();
}

Token Document::getNext() {
  คืน stream.scanner->getNext();
}

void Document::setError(const Twine &Message, Token &Location) const {
  stream.scanner->setError(Message, Location.Range.begin());
}

bool Document::failed() const {
  คืน stream.scanner->failed();
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
    คืน new (NodeAllocator) AliasNode(stream.CurrentDoc, T.Range.substr(1));
  pak_sa Token::TK_Anchor:
    ถ้า (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      คืน nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    goto parse_property;
  pak_sa Token::TK_Tag:
    ถ้า (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      คืน nullptr;
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
    คืน new (NodeAllocator) SequenceNode( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , SequenceNode::ST_Indentless);
  pak_sa Token::TK_BlockSequenceStart:
    getNext();
    คืน new (NodeAllocator)
      SequenceNode( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , SequenceNode::ST_Block);
  pak_sa Token::TK_BlockMappingStart:
    getNext();
    คืน new (NodeAllocator)
      MappingNode( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , MappingNode::MT_Block);
  pak_sa Token::TK_FlowSequenceStart:
    getNext();
    คืน new (NodeAllocator)
      SequenceNode( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , SequenceNode::ST_Flow);
  pak_sa Token::TK_FlowMappingStart:
    getNext();
    คืน new (NodeAllocator)
      MappingNode( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , MappingNode::MT_Flow);
  pak_sa Token::TK_Scalar:
    getNext();
    คืน new (NodeAllocator)
      ScalarNode( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  pak_sa Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.Value.c_str(), T.Value.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(NodeAllocator).drop_back();
    คืน new (NodeAllocator)
        BlockScalarNode(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  pak_sa Token::TK_Key:
    // Don't eat the TK_Key, KeyValueNode expects it.
    คืน new (NodeAllocator)
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
    คืน new (NodeAllocator) NullNode(stream.CurrentDoc);
  pak_sa Token::TK_FlowMappingEnd:
  pak_sa Token::TK_FlowSequenceEnd:
  pak_sa Token::TK_FlowEntry: {
    ถ้า (Root && (isa<MappingNode>(Root) || isa<SequenceNode>(Root)))
      คืน new (NodeAllocator) NullNode(stream.CurrentDoc);

    setError("Unexpected token", T);
    คืน nullptr;
  }
  pak_sa Token::TK_Error:
    คืน nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  คืน nullptr;
}

bool Document::parseDirectives() {
  bool isDirective = false;
  ขณะ (true) {
    Token T = peekNext();
    ถ้า (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = true;
    } มิฉะนั้น ถ้า (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = true;
    } มิฉะนั้น {
      bha_nga;
    }
  }
  คืน isDirective;
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

bool Document::expectToken(จำนวนเต็ม TK) {
  Token T = getNext();
  ถ้า (T.Kind != TK) {
    setError("Unexpected token", T);
    คืน false;
  }
  คืน true;
}
