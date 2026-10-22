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

#ضمن "llvm/Support/YAMLParser.h"
#ضمن "llvm/ADT/AllocatorList.h"
#ضمن "llvm/ADT/ArrayRef.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/ADT/Twine.h"
#ضمن "llvm/Support/Compiler.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/SMLoc.h"
#ضمن "llvm/Support/SourceMgr.h"
#ضمن "llvm/Support/Unicode.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cassert>
#ضمن <cstddef>
#ضمن <cstdint>
#ضمن <map>
#ضمن <memory>
#ضمن <string>
#ضمن <system_error>
#ضمن <utility>

using نطاق llvm;
using نطاق yaml;

تعداد UnicodeEncodingForm {
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
ساكن EncodingInfo getUnicodeEncoding(StringRef Input) {
  אם (Input.empty())
    החזר {UEF_Unknown, 0};

  بدل (uint8_t(Input[0])) {
  حالة 0x00:
    אם (Input.size() >= 4) {
      אם (  Input[1] == 0
         && uint8_t(Input[2]) == 0xFE
         && uint8_t(Input[3]) == 0xFF)
        החזר {UEF_UTF32_BE, 4};
      אם (Input[1] == 0 && Input[2] == 0 && Input[3] != 0)
        החזר {UEF_UTF32_BE, 0};
    }

    אם (Input.size() >= 2 && Input[1] != 0)
      החזר {UEF_UTF16_BE, 0};
    החזר {UEF_Unknown, 0};
  حالة 0xFF:
    אם (  Input.size() >= 4
       && uint8_t(Input[1]) == 0xFE
       && Input[2] == 0
       && Input[3] == 0)
      החזר {UEF_UTF32_LE, 4};

    אם (Input.size() >= 2 && uint8_t(Input[1]) == 0xFE)
      החזר {UEF_UTF16_LE, 2};
    החזר {UEF_Unknown, 0};
  حالة 0xFE:
    אם (Input.size() >= 2 && uint8_t(Input[1]) == 0xFF)
      החזר {UEF_UTF16_BE, 2};
    החזר {UEF_Unknown, 0};
  حالة 0xEF:
    אם (  Input.size() >= 3
       && uint8_t(Input[1]) == 0xBB
       && uint8_t(Input[2]) == 0xBF)
      החזר {UEF_UTF8, 3};
    החזר {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  אם (Input.size() >= 4 && Input[1] == 0 && Input[2] == 0 && Input[3] == 0)
    החזר {UEF_UTF32_LE, 0};

  אם (Input.size() >= 2 && Input[1] == 0)
    החזר {UEF_UTF16_LE, 0};

  החזר {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
فراغ Node::anchor() {}
فراغ NullNode::anchor() {}
فراغ ScalarNode::anchor() {}
فراغ BlockScalarNode::anchor() {}
فراغ KeyValueNode::anchor() {}
فراغ MappingNode::anchor() {}
فراغ SequenceNode::anchor() {}
فراغ AliasNode::anchor() {}

نطاق llvm {
نطاق yaml {

/// Token - A single YAML token.
بنية Token {
  تعداد TokenKind {
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

  Token() = افتراضي;
};

} // end namespace yaml
} // end namespace llvm

using TokenQueueT = BumpPtrList<Token>;

نطاق {

/// This struct is used to track simple keys.
///
/// Simple keys are handled by creating an entry in SimpleKeys for each Token
/// which could legally be the start of a simple key. When peekNext is called,
/// if the Token To be returned is referenced by a SimpleKey, we continue
/// tokenizing until that potential simple key has either been found to not be
/// a simple key (we moved on to the next line or went further than 1024 chars).
/// Or when we run into a Value, and then insert a Key token (and possibly
/// others) before the SimpleKey's Tok.
بنية SimpleKey {
  TokenQueueT::iterator Tok;
  unsigned Column = 0;
  unsigned Line = 0;
  unsigned FlowLevel = 0;
  منطقي IsRequired = שקר;

  منطقي operator ==(ثابت SimpleKey &Other) {
    החזר Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
using UTF8Decoded = std::pair<uint32_t, unsigned>;

ساكن UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator End = Range.end();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  אם (Position < End && (*Position & 0x80) == 0) {
    החזר {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  אם (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    אם (codepoint >= 0x80)
      החזר {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  אם (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    אם (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      החזר {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  אם (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    אם (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      החזר {codepoint, 4};
  }
  החזר {0, 0};
}

نطاق llvm {
نطاق yaml {

/// Scans YAML tokens from a MemoryBuffer.
מחלקה Scanner {
عام:
  Scanner(StringRef Input, SourceMgr &SM, منطقي ShowColors = אמת,
          std::error_code *EC = nullptr);
  Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, منطقي ShowColors = אמת,
          std::error_code *EC = nullptr);

  /// Parse the next token and return it without popping it.
  Token &peekNext();

  /// Parse the next token and pop it from the queue.
  Token getNext();

  فراغ printError(SMLoc Loc, SourceMgr::DiagKind Kind, ثابت Twine &Message,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.PrintMessage(Loc, Kind, Message, Ranges, /* FixIts= */ {}, ShowColors);
  }

  فراغ setError(ثابت Twine &Message, StringRef::iterator Position) {
    אם (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    אם (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    אם (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = אמת;
  }

  /// Returns true if an error occurred while parsing.
  منطقي failed() {
    החזר Failed;
  }

خاص:
  فراغ init(MemoryBufferRef Buffer);

  StringRef currentInput() {
    החזר StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    החזר ::decodeUTF8(StringRef(Position, End - Position));
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
  فراغ advanceWhile(SkipWhileFunc Func);

  /// Scan ns-uri-char[39]s starting at Cur.
  ///
  /// This updates Cur and Column while scanning.
  فراغ scan_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  منطقي consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  فراغ skip(uint32_t Distance);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  منطقي isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  منطقي isPlainSafeNonBlank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  منطقي isLineEmpty(StringRef Line);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  منطقي consumeLineBreakIfPresent();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  فراغ saveSimpleKeyCandidate( TokenQueueT::iterator Tok
                             , unsigned AtColumn
                             , منطقي IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  فراغ removeStaleSimpleKeyCandidates();

  /// Remove all simple keys on FlowLevel \a Level.
  فراغ removeSimpleKeyCandidatesOnFlowLevel(unsigned Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  منطقي unrollIndent(صحيح ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  منطقي rollIndent( صحيح ToColumn
                 , Token::TokenKind Kind
                 , TokenQueueT::iterator InsertPoint);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  فراغ skipComment();

  /// Skip whitespace and comments until the start of the next token.
  فراغ scanToNextToken();

  /// Must be the first token generated.
  منطقي scanStreamStart();

  /// Generate tokens needed to close out the stream.
  منطقي scanStreamEnd();

  /// Scan a %BLAH directive.
  منطقي scanDirective();

  /// Scan a ... or ---.
  منطقي scanDocumentIndicator(منطقي IsStart);

  /// Scan a [ or { and generate the proper flow collection start token.
  منطقي scanFlowCollectionStart(منطقي IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  منطقي scanFlowCollectionEnd(منطقي IsSequence);

  /// Scan the , that separates entries in a flow collection.
  منطقي scanFlowEntry();

  /// Scan the - that starts block sequence entries.
  منطقي scanBlockEntry();

  /// Scan an explicit ? indicating a key.
  منطقي scanKey();

  /// Scan an explicit : indicating a value.
  منطقي scanValue();

  /// Scan a quoted scalar.
  منطقي scanFlowScalar(منطقي IsDoubleQuoted);

  /// Scan an unquoted scalar.
  منطقي scanPlainScalar();

  /// Scan an Alias or Anchor starting with * or &.
  منطقي scanAliasOrAnchor(منطقي IsAlias);

  /// Scan a block scalar starting with | or >.
  منطقي scanBlockScalar(منطقي IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  منطقي scanBlockScalarIndicators(حرف &StyleIndicator, حرف &ChompingIndicator,
                                 unsigned &IndentIndicator, منطقي &IsDone);

  /// Scan a style indicator in a block scalar header.
  حرف scanBlockStyleIndicator();

  /// Scan a chomping indicator in a block scalar header.
  حرف scanBlockChompingIndicator();

  /// Scan an indentation indicator in a block scalar header.
  unsigned scanBlockIndentationIndicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  منطقي scanBlockScalarHeader(حرف &ChompingIndicator, unsigned &IndentIndicator,
                             منطقي &IsDone);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  منطقي findBlockScalarIndent(unsigned &BlockIndent, unsigned BlockExitIndent,
                             unsigned &LineBreaks, منطقي &IsDone);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  منطقي scanBlockScalarIndent(unsigned BlockIndent, unsigned BlockExitIndent,
                             منطقي &IsDone);

  /// Scan a tag of the form !stuff.
  منطقي scanTag();

  /// Dispatch to the next scanning function based on \a *Cur.
  منطقي fetchMoreTokens();

  /// The SourceMgr used for diagnostics and buffer management.
  SourceMgr &SM;

  /// The original input.
  MemoryBufferRef InputBuffer;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator End;

  /// Current YAML indentation level in spaces.
  صحيح Indent;

  /// Current column number in Unicode code points.
  unsigned Column;

  /// Current line number.
  unsigned Line;

  /// How deep we are in flow style containers. 0 Means at block level.
  unsigned FlowLevel;

  /// Are we at the start of the stream?
  منطقي IsStartOfStream;

  /// Can the next token be the start of a simple key?
  منطقي IsSimpleKeyAllowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  منطقي IsAdjacentValueAllowedInFlow;

  /// True if an error has occurred.
  منطقي Failed;

  /// Should colors be used when printing out the diagnostic messages?
  منطقي ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  TokenQueueT TokenQueue;

  /// Indentation levels.
  SmallVector<صحيح, 4> Indents;

  /// Potential simple keys.
  SmallVector<SimpleKey, 4> SimpleKeys;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
ساكن فراغ encodeUTF8( uint32_t UnicodeScalarValue
                      , SmallVectorImpl<حرف> &Result) {
  אם (UnicodeScalarValue <= 0x7F) {
    Result.push_back(UnicodeScalarValue & 0x7F);
  } אחרת אם (UnicodeScalarValue <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((UnicodeScalarValue & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (UnicodeScalarValue & 0x3F);
    Result.push_back(FirstByte);
    Result.push_back(SecondByte);
  } אחרת אם (UnicodeScalarValue <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((UnicodeScalarValue & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((UnicodeScalarValue & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (UnicodeScalarValue & 0x3F);
    Result.push_back(FirstByte);
    Result.push_back(SecondByte);
    Result.push_back(ThirdByte);
  } אחרת אם (UnicodeScalarValue <= 0x10FFFF) {
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

منطقي yaml::dumpTokens(StringRef Input, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(Input, SM);
  כלעוד (אמת) {
    Token T = scanner.getNext();
    بدل (T.Kind) {
    حالة Token::TK_StreamStart:
      OS << "Stream-Start: ";
      اكسر;
    حالة Token::TK_StreamEnd:
      OS << "Stream-End: ";
      اكسر;
    حالة Token::TK_VersionDirective:
      OS << "Version-Directive: ";
      اكسر;
    حالة Token::TK_TagDirective:
      OS << "Tag-Directive: ";
      اكسر;
    حالة Token::TK_DocumentStart:
      OS << "Document-Start: ";
      اكسر;
    حالة Token::TK_DocumentEnd:
      OS << "Document-End: ";
      اكسر;
    حالة Token::TK_BlockEntry:
      OS << "Block-Entry: ";
      اكسر;
    حالة Token::TK_BlockEnd:
      OS << "Block-End: ";
      اكسر;
    حالة Token::TK_BlockSequenceStart:
      OS << "Block-Sequence-Start: ";
      اكسر;
    حالة Token::TK_BlockMappingStart:
      OS << "Block-Mapping-Start: ";
      اكسر;
    حالة Token::TK_FlowEntry:
      OS << "Flow-Entry: ";
      اكسر;
    حالة Token::TK_FlowSequenceStart:
      OS << "Flow-Sequence-Start: ";
      اكسر;
    حالة Token::TK_FlowSequenceEnd:
      OS << "Flow-Sequence-End: ";
      اكسر;
    حالة Token::TK_FlowMappingStart:
      OS << "Flow-Mapping-Start: ";
      اكسر;
    حالة Token::TK_FlowMappingEnd:
      OS << "Flow-Mapping-End: ";
      اكسر;
    حالة Token::TK_Key:
      OS << "Key: ";
      اكسر;
    حالة Token::TK_Value:
      OS << "Value: ";
      اكسر;
    حالة Token::TK_Scalar:
      OS << "Scalar: ";
      اكسر;
    حالة Token::TK_BlockScalar:
      OS << "Block Scalar: ";
      اكسر;
    حالة Token::TK_Alias:
      OS << "Alias: ";
      اكسر;
    حالة Token::TK_Anchor:
      OS << "Anchor: ";
      اكسر;
    حالة Token::TK_Tag:
      OS << "Tag: ";
      اكسر;
    حالة Token::TK_Error:
      اكسر;
    }
    OS << T.Range << "\n";
    אם (T.Kind == Token::TK_StreamEnd)
      اكسر;
    אחרת אם (T.Kind == Token::TK_Error)
      החזר שקר;
  }
  החזר אמת;
}

منطقي yaml::scanTokens(StringRef Input) {
  SourceMgr SM;
  Scanner scanner(Input, SM);
  כלעוד (אמת) {
    Token T = scanner.getNext();
    אם (T.Kind == Token::TK_StreamEnd)
      اكسر;
    אחרת אם (T.Kind == Token::TK_Error)
      החזר שקר;
  }
  החזר אמת;
}

std::string yaml::escape(StringRef Input, منطقي EscapePrintable) {
  std::string EscapedInput;
  עבור (StringRef::iterator i = Input.begin(), e = Input.end(); i != e; ++i) {
    אם (*i == '\\')
      EscapedInput += "\\\\";
    אחרת אם (*i == '"')
      EscapedInput += "\\\"";
    אחרת אם (*i == 0)
      EscapedInput += "\\0";
    אחרת אם (*i == 0x07)
      EscapedInput += "\\a";
    אחרת אם (*i == 0x08)
      EscapedInput += "\\b";
    אחרת אם (*i == 0x09)
      EscapedInput += "\\t";
    אחרת אם (*i == 0x0A)
      EscapedInput += "\\n";
    אחרת אם (*i == 0x0B)
      EscapedInput += "\\v";
    אחרת אם (*i == 0x0C)
      EscapedInput += "\\f";
    אחרת אם (*i == 0x0D)
      EscapedInput += "\\r";
    אחרת אם (*i == 0x1B)
      EscapedInput += "\\e";
    אחרת אם ((unsigned حرف)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      EscapedInput += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } אחרת אם (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded UnicodeScalarValue
        = decodeUTF8(StringRef(i, Input.end() - i));
      אם (UnicodeScalarValue.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(EscapedInput, Val);
        // FIXME: Error reporting.
        החזר EscapedInput;
      }
      אם (UnicodeScalarValue.first == 0x85)
        EscapedInput += "\\N";
      אחרת אם (UnicodeScalarValue.first == 0xA0)
        EscapedInput += "\\_";
      אחרת אם (UnicodeScalarValue.first == 0x2028)
        EscapedInput += "\\L";
      אחרת אם (UnicodeScalarValue.first == 0x2029)
        EscapedInput += "\\P";
      אחרת אם (!EscapePrintable &&
               sys::unicode::isPrintable(UnicodeScalarValue.first))
        EscapedInput += StringRef(i, UnicodeScalarValue.second);
      אחרת {
        std::string HexStr = utohexstr(UnicodeScalarValue.first);
        אם (HexStr.size() <= 2)
          EscapedInput += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        אחרת אם (HexStr.size() <= 4)
          EscapedInput += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        אחרת אם (HexStr.size() <= 8)
          EscapedInput += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += UnicodeScalarValue.second - 1;
    } אחרת {
      EscapedInput.push_back(*i);
    }
  }
  החזר EscapedInput;
}

std::optional<منطقي> yaml::parseBool(StringRef S) {
  بدل (S.size()) {
  حالة 1:
    بدل (S.front()) {
    حالة 'y':
    حالة 'Y':
      החזר אמת;
    حالة 'n':
    حالة 'N':
      החזר שקר;
    افتراضي:
      החזר std::nullopt;
    }
  حالة 2:
    بدل (S.front()) {
    حالة 'O':
      אם (S[1] == 'N') // ON
        החזר אמת;
      [[fallthrough]];
    حالة 'o':
      אם (S[1] == 'n') //[Oo]n
        החזר אמת;
      החזר std::nullopt;
    حالة 'N':
      אם (S[1] == 'O') // NO
        החזר שקר;
      [[fallthrough]];
    حالة 'n':
      אם (S[1] == 'o') //[Nn]o
        החזר שקר;
      החזר std::nullopt;
    افتراضي:
      החזר std::nullopt;
    }
  حالة 3:
    بدل (S.front()) {
    حالة 'O':
      אם (S.drop_front() == "FF") // OFF
        החזר שקר;
      [[fallthrough]];
    حالة 'o':
      אם (S.drop_front() == "ff") //[Oo]ff
        החזר שקר;
      החזר std::nullopt;
    حالة 'Y':
      אם (S.drop_front() == "ES") // YES
        החזר אמת;
      [[fallthrough]];
    حالة 'y':
      אם (S.drop_front() == "es") //[Yy]es
        החזר אמת;
      החזר std::nullopt;
    افتراضي:
      החזר std::nullopt;
    }
  حالة 4:
    بدل (S.front()) {
    حالة 'T':
      אם (S.drop_front() == "RUE") // TRUE
        החזר אמת;
      [[fallthrough]];
    حالة 't':
      אם (S.drop_front() == "rue") //[Tt]rue
        החזר אמת;
      החזר std::nullopt;
    افتراضي:
      החזר std::nullopt;
    }
  حالة 5:
    بدل (S.front()) {
    حالة 'F':
      אם (S.drop_front() == "ALSE") // FALSE
        החזר שקר;
      [[fallthrough]];
    حالة 'f':
      אם (S.drop_front() == "alse") //[Ff]alse
        החזר שקר;
      החזר std::nullopt;
    افتراضي:
      החזר std::nullopt;
    }
  افتراضي:
    החזר std::nullopt;
  }
}

Scanner::Scanner(StringRef Input, SourceMgr &sm, منطقي ShowColors,
                 std::error_code *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(MemoryBufferRef(Input, "YAML"));
}

Scanner::Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, منطقي ShowColors,
                 std::error_code *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(Buffer);
}

فراغ Scanner::init(MemoryBufferRef Buffer) {
  InputBuffer = Buffer;
  Current = InputBuffer.getBufferStart();
  End = InputBuffer.getBufferEnd();
  Indent = -1;
  Column = 0;
  Line = 0;
  FlowLevel = 0;
  IsStartOfStream = אמת;
  IsSimpleKeyAllowed = אמת;
  IsAdjacentValueAllowedInFlow = שקר;
  Failed = שקר;
  std::unique_ptr<MemoryBuffer> InputBufferOwner =
      MemoryBuffer::getMemBuffer(Buffer, /*RequiresNullTerminator=*/שקר);
  SM.AddNewSourceBuffer(std::move(InputBufferOwner), SMLoc());
}

Token &Scanner::peekNext() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  منطقي NeedMore = שקר;
  כלעוד (אמת) {
    אם (TokenQueue.empty() || NeedMore) {
      אם (!fetchMoreTokens()) {
        TokenQueue.clear();
        SimpleKeys.clear();
        TokenQueue.push_back(Token());
        החזר TokenQueue.front();
      }
    }
    أكد(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    removeStaleSimpleKeyCandidates();
    SimpleKey SK;
    SK.Tok = TokenQueue.begin();
    אם (!is_contained(SimpleKeys, SK))
      اكسر;
    אחרת
      NeedMore = אמת;
  }
  החזר TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  אם (!TokenQueue.empty())
    TokenQueue.pop_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  אם (TokenQueue.empty())
    TokenQueue.resetAlloc();

  החזר Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  אם (Position == End)
    החזר Position;
  // Check 7 bit c-printable - b-char.
  אם (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    החזר Position + 1;

  // Check for valid UTF-8.
  אם (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    אם (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      החזר Position + u8d.second;
  }
  החזר Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  אם (Position == End)
    החזר Position;
  אם (*Position == 0x0D) {
    אם (Position + 1 != End && *(Position + 1) == 0x0A)
      החזר Position + 2;
    החזר Position + 1;
  }

  אם (*Position == 0x0A)
    החזר Position + 1;
  החזר Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  אם (Position == End)
    החזר Position;
  אם (*Position == ' ')
    החזר Position + 1;
  החזר Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  אם (Position == End)
    החזר Position;
  אם (*Position == ' ' || *Position == '\t')
    החזר Position + 1;
  החזר Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  אם (Position == End)
    החזר Position;
  אם (*Position == ' ' || *Position == '\t')
    החזר Position;
  החזר skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  כלעוד (אמת) {
    StringRef::iterator i = (هذا->*Func)(Position);
    אם (i == Position)
      اكسر;
    Position = i;
  }
  החזר Position;
}

فراغ Scanner::advanceWhile(SkipWhileFunc Func) {
  auto Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

ساكن منطقي is_ns_hex_digit(ثابت حرف C) { החזר isAlnum(C); }

ساكن منطقي is_ns_word_char(ثابت حرف C) { החזר C == '-' || isAlpha(C); }

فراغ Scanner::scan_ns_uri_char() {
  כלעוד (אמת) {
    אם (Current == End)
      اكسر;
    אם ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } אחרת {
      اكسر;
    }
  }
}

منطقي Scanner::consume(uint32_t Expected) {
  אם (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    החזר שקר;
  }
  אם (Current == End)
    החזר שקר;
  אם (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    החזר שקר;
  }
  אם (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    החזר אמת;
  }
  החזר שקר;
}

فراغ Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  أكد(Current <= End && "Skipped past the end");
}

منطقي Scanner::isBlankOrBreak(StringRef::iterator Position) {
  אם (Position == End)
    החזר שקר;
  החזר *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

منطقي Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  אם (Position == End || isBlankOrBreak(Position))
    החזר שקר;
  אם (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    החזר שקר;
  החזר אמת;
}

منطقي Scanner::isLineEmpty(StringRef Line) {
  עבור (ثابت auto *Position = Line.begin(); Position != Line.end(); ++Position)
    אם (!isBlankOrBreak(Position))
      החזר שקר;
  החזר אמת;
}

منطقي Scanner::consumeLineBreakIfPresent() {
  auto Next = skip_b_break(Current);
  אם (Next == Current)
    החזר שקר;
  Column = 0;
  ++Line;
  Current = Next;
  החזר אמת;
}

فراغ Scanner::saveSimpleKeyCandidate( TokenQueueT::iterator Tok
                                    , unsigned AtColumn
                                    , منطقي IsRequired) {
  אם (IsSimpleKeyAllowed) {
    SimpleKey SK;
    SK.Tok = Tok;
    SK.Line = Line;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    SimpleKeys.push_back(SK);
  }
}

فراغ Scanner::removeStaleSimpleKeyCandidates() {
  עבור (SmallVectorImpl<SimpleKey>::iterator i = SimpleKeys.begin();
                                            i != SimpleKeys.end();) {
    אם (i->Line != Line || i->Column + 1024 < Column) {
      אם (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = SimpleKeys.erase(i);
    } אחרת {
      ++i;
    }
  }
}

فراغ Scanner::removeSimpleKeyCandidatesOnFlowLevel(unsigned Level) {
  אם (!SimpleKeys.empty() && (SimpleKeys.end() - 1)->FlowLevel == Level)
    SimpleKeys.pop_back();
}

منطقي Scanner::unrollIndent(صحيح ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  אם (FlowLevel != 0)
    החזר אמת;

  כלעוד (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.push_back(T);
    Indent = Indents.pop_back_val();
  }

  החזר אמת;
}

منطقي Scanner::rollIndent( صحيح ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator InsertPoint) {
  אם (FlowLevel)
    החזר אמת;
  אם (Indent < ToColumn) {
    Indents.push_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.insert(InsertPoint, T);
  }
  החזר אמת;
}

فراغ Scanner::skipComment() {
  אם (Current == End || *Current != '#')
    החזר;
  כלעוד (אמת) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    אם (I == Current)
      اكسر;
    Current = I;
    ++Column;
  }
}

فراغ Scanner::scanToNextToken() {
  כלעוד (אמת) {
    כלעוד (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    אם (i == Current)
      اكسر;
    Current = i;
    ++Line;
    Column = 0;
    // New lines may start a simple key.
    אם (!FlowLevel)
      IsSimpleKeyAllowed = אמת;
  }
}

منطقي Scanner::scanStreamStart() {
  IsStartOfStream = שקר;

  EncodingInfo EI = getUnicodeEncoding(currentInput());

  Token T;
  T.Kind = Token::TK_StreamStart;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.push_back(T);
  Current += EI.second;
  החזר אמת;
}

منطقي Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  אם (Column != 0) {
    Column = 0;
    ++Line;
  }

  unrollIndent(-1);
  SimpleKeys.clear();
  IsSimpleKeyAllowed = שקר;
  IsAdjacentValueAllowedInFlow = שקר;

  Token T;
  T.Kind = Token::TK_StreamEnd;
  T.Range = StringRef(Current, 0);
  TokenQueue.push_back(T);
  החזר אמת;
}

منطقي Scanner::scanDirective() {
  // Reset the indentation level.
  unrollIndent(-1);
  SimpleKeys.clear();
  IsSimpleKeyAllowed = שקר;
  IsAdjacentValueAllowedInFlow = שקר;

  StringRef::iterator Start = Current;
  consume('%');
  StringRef::iterator NameStart = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(NameStart, Current - NameStart);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  אם (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.push_back(T);
    החזר אמת;
  } אחרת אם(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.push_back(T);
    החזר אמת;
  }
  החזר שקר;
}

منطقي Scanner::scanDocumentIndicator(منطقي IsStart) {
  unrollIndent(-1);
  SimpleKeys.clear();
  IsSimpleKeyAllowed = שקר;
  IsAdjacentValueAllowedInFlow = שקר;

  Token T;
  T.Kind = IsStart ? Token::TK_DocumentStart : Token::TK_DocumentEnd;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.push_back(T);
  החזר אמת;
}

منطقي Scanner::scanFlowCollectionStart(منطقي IsSequence) {
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);

  // [ and { may begin a simple key.
  saveSimpleKeyCandidate(--TokenQueue.end(), Column - 1, שקר);

  // And may also be followed by a simple key.
  IsSimpleKeyAllowed = אמת;
  // Adjacent values are allowed in flows only after JSON-style keys.
  IsAdjacentValueAllowedInFlow = שקר;
  ++FlowLevel;
  החזר אמת;
}

منطقي Scanner::scanFlowCollectionEnd(منطقي IsSequence) {
  removeSimpleKeyCandidatesOnFlowLevel(FlowLevel);
  IsSimpleKeyAllowed = שקר;
  IsAdjacentValueAllowedInFlow = אמת;
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceEnd
                      : Token::TK_FlowMappingEnd;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);
  אם (FlowLevel)
    --FlowLevel;
  החזר אמת;
}

منطقي Scanner::scanFlowEntry() {
  removeSimpleKeyCandidatesOnFlowLevel(FlowLevel);
  IsSimpleKeyAllowed = אמת;
  IsAdjacentValueAllowedInFlow = שקר;
  Token T;
  T.Kind = Token::TK_FlowEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);
  החזר אמת;
}

منطقي Scanner::scanBlockEntry() {
  rollIndent(Column, Token::TK_BlockSequenceStart, TokenQueue.end());
  removeSimpleKeyCandidatesOnFlowLevel(FlowLevel);
  IsSimpleKeyAllowed = אמת;
  IsAdjacentValueAllowedInFlow = שקר;
  Token T;
  T.Kind = Token::TK_BlockEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);
  החזר אמת;
}

منطقي Scanner::scanKey() {
  אם (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  removeSimpleKeyCandidatesOnFlowLevel(FlowLevel);
  IsSimpleKeyAllowed = !FlowLevel;
  IsAdjacentValueAllowedInFlow = שקר;

  Token T;
  T.Kind = Token::TK_Key;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);
  החזר אמת;
}

منطقي Scanner::scanValue() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  אם (!SimpleKeys.empty()) {
    SimpleKey SK = SimpleKeys.pop_back_val();
    Token T;
    T.Kind = Token::TK_Key;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    עבור (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      אם (i == SK.Tok)
        اكسر;
    }
    אם (i == e) {
      Failed = אמת;
      החזר שקר;
    }
    i = TokenQueue.insert(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    IsSimpleKeyAllowed = שקר;
  } אחרת {
    אם (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    IsSimpleKeyAllowed = !FlowLevel;
  }
  IsAdjacentValueAllowedInFlow = שקר;

  Token T;
  T.Kind = Token::TK_Value;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.push_back(T);
  החזר אמת;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE ساكن منطقي
wasEscaped(StringRef::iterator First, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
ساكن منطقي wasEscaped(StringRef::iterator First,
                       StringRef::iterator Position) {
  أكد(Position - 1 >= First);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  כלעוד (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  החזר (Position - 1 - I) % 2 == 1;
}

منطقي Scanner::scanFlowScalar(منطقي IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  אם (IsDoubleQuoted) {
    עשה {
      ++Current;
      כלעוד (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } כלעוד (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } אחרת {
    skip(1);
    כלעוד (Current != End) {
      // Skip a ' followed by another '.
      אם (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        استمر;
      } אחרת אם (*Current == '\'')
        اكسر;
      StringRef::iterator i = skip_nb_char(Current);
      אם (i == Current) {
        i = skip_b_break(Current);
        אם (i == Current)
          اكسر;
        Current = i;
        Column = 0;
        ++Line;
      } אחרת {
        אם (i == End)
          اكسر;
        Current = i;
        ++Column;
      }
    }
  }

  אם (Current == End) {
    setError("Expected quote at end of scalar", Current);
    החזר שקר;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, שקר);

  IsSimpleKeyAllowed = שקר;
  IsAdjacentValueAllowedInFlow = אמת;

  החזר אמת;
}

منطقي Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  unsigned LeadingBlanks = 0;
  أكد(Indent >= -1 && "Indent must be >= -1 !");
  unsigned indent = static_cast<unsigned>(Indent + 1);
  כלעוד (Current != End) {
    אם (*Current == '#')
      اكسر;

    כלעוד (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      אם (i == Current)
        اكسر;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    אם (!isBlankOrBreak(Current))
      اكسر;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    כלעוד (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      אם (i != Tmp) {
        אם (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          החזר שקר;
        }
        Tmp = i;
        ++Column;
      } אחרת {
        i = skip_b_break(Tmp);
        אם (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++Line;
      }
    }

    אם (!FlowLevel && Column < indent)
      اكسر;

    Current = Tmp;
  }
  אם (Start == Current) {
    setError("Got empty plain scalar", Start);
    החזר שקר;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  // Plain scalars can be simple keys.
  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, שקר);

  IsSimpleKeyAllowed = שקר;
  IsAdjacentValueAllowedInFlow = שקר;

  החזר אמת;
}

منطقي Scanner::scanAliasOrAnchor(منطقي IsAlias) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1);
  כלעוד (Current != End) {
    אם (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      اكسر;
    StringRef::iterator i = skip_ns_char(Current);
    אם (i == Current)
      اكسر;
    Current = i;
    ++Column;
  }

  אם (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    החזר שקר;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  // Alias and anchors can be simple keys.
  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, שקר);

  IsSimpleKeyAllowed = שקר;
  IsAdjacentValueAllowedInFlow = שקר;

  החזר אמת;
}

منطقي Scanner::scanBlockScalarIndicators(حرف &StyleIndicator,
                                        حرف &ChompingIndicator,
                                        unsigned &IndentIndicator,
                                        منطقي &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  אם (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    החזר שקר;
  החזר אמת;
}

حرف Scanner::scanBlockStyleIndicator() {
  حرف Indicator = ' ';
  אם (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  החזר Indicator;
}

حرف Scanner::scanBlockChompingIndicator() {
  حرف Indicator = ' ';
  אם (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  החזר Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
ساكن unsigned getChompedLineBreaks(حرف ChompingIndicator,
                                     unsigned LineBreaks, StringRef Str) {
  אם (ChompingIndicator == '-') // Strip all line breaks.
    החזר 0;
  אם (ChompingIndicator == '+') // Keep all line breaks.
    החזר LineBreaks;
  // Clip trailing lines.
  החזר Str.empty() ? 0 : 1;
}

unsigned Scanner::scanBlockIndentationIndicator() {
  unsigned Indent = 0;
  אם (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = unsigned(*Current - '0');
    skip(1);
  }
  החזר Indent;
}

منطقي Scanner::scanBlockScalarHeader(حرف &ChompingIndicator,
                                    unsigned &IndentIndicator, منطقي &IsDone) {
  auto Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  אם (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  אם (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.push_back(T);
    IsDone = אמת;
    החזר אמת;
  }

  אם (!consumeLineBreakIfPresent()) {
    setError("Expected a line break after block scalar header", Current);
    החזר שקר;
  }
  החזר אמת;
}

منطقي Scanner::findBlockScalarIndent(unsigned &BlockIndent,
                                    unsigned BlockExitIndent,
                                    unsigned &LineBreaks, منطقي &IsDone) {
  unsigned MaxAllSpaceLineCharacters = 0;
  StringRef::iterator LongestAllSpaceLine;

  כלעוד (אמת) {
    advanceWhile(&Scanner::skip_s_space);
    אם (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      אם (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = אמת;
        החזר אמת;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      אם (MaxAllSpaceLineCharacters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            LongestAllSpaceLine);
        החזר שקר;
      }
      החזר אמת;
    }
    אם (skip_b_break(Current) != Current &&
        Column > MaxAllSpaceLineCharacters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      MaxAllSpaceLineCharacters = Column;
      LongestAllSpaceLine = Current;
    }

    // Check for EOF.
    אם (Current == End) {
      IsDone = אמת;
      החזר אמת;
    }

    אם (!consumeLineBreakIfPresent()) {
      IsDone = אמת;
      החזר אמת;
    }
    ++LineBreaks;
  }
  החזר אמת;
}

منطقي Scanner::scanBlockScalarIndent(unsigned BlockIndent,
                                    unsigned BlockExitIndent, منطقي &IsDone) {
  // Skip the indentation.
  כלעוד (Column < BlockIndent) {
    auto I = skip_s_space(Current);
    אם (I == Current)
      اكسر;
    Current = I;
    ++Column;
  }

  אם (skip_nb_char(Current) == Current)
    החזר אמת;

  אם (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = אמת;
    החזר אמת;
  }

  אם (Column < BlockIndent) {
    אם (Current != End && *Current == '#') { // Trailing comment.
      IsDone = אמת;
      החזר אמת;
    }
    setError("A text line is less indented than the block scalar", Current);
    החזר שקר;
  }
  החזר אמת; // A normal text line.
}

منطقي Scanner::scanBlockScalar(منطقي IsLiteral) {
  أكد(*Current == '|' || *Current == '>');
  حرف StyleIndicator;
  حرف ChompingIndicator;
  unsigned BlockIndent;
  منطقي IsDone = שקר;
  אם (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    החזר שקר;
  אם (IsDone)
    החזר אמת;
  منطقي IsFolded = StyleIndicator == '>';

  ثابت auto *Start = Current;
  unsigned BlockExitIndent = Indent < 0 ? 0 : (unsigned)Indent;
  unsigned LineBreaks = 0;
  אם (BlockIndent == 0) {
    אם (!findBlockScalarIndent(BlockIndent, BlockExitIndent, LineBreaks,
                               IsDone))
      החזר שקר;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  כלעוד (!IsDone) {
    אם (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      החזר שקר;
    אם (IsDone)
      اكسر;

    // Parse the current line.
    auto LineStart = Current;
    advanceWhile(&Scanner::skip_nb_char);
    אם (LineStart != Current) {
      אם (LineBreaks && IsFolded && !Scanner::isLineEmpty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        אם (LineBreaks == 1) {
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
    אם (Current == End)
      اكسر;

    אם (!consumeLineBreakIfPresent())
      اكسر;
    ++LineBreaks;
  }

  אם (Current == End && !LineBreaks)
    // Ensure that there is at least one line break before the end of file.
    LineBreaks = 1;
  Str.append(getChompedLineBreaks(ChompingIndicator, LineBreaks, Str), '\n');

  // New lines may start a simple key.
  אם (!FlowLevel)
    IsSimpleKeyAllowed = אמת;
  IsAdjacentValueAllowedInFlow = שקר;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.Value = std::string(Str);
  TokenQueue.push_back(T);
  החזר אמת;
}

منطقي Scanner::scanTag() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1); // Eat !.
  אם (Current == End || isBlankOrBreak(Current)); // An empty tag.
  אחרת אם (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    אם (!consume('>'))
      החזר שקר;
  } אחרת {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.Kind = Token::TK_Tag;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.push_back(T);

  // Tags can be simple keys.
  saveSimpleKeyCandidate(--TokenQueue.end(), ColStart, שקר);

  IsSimpleKeyAllowed = שקר;
  IsAdjacentValueAllowedInFlow = שקר;

  החזר אמת;
}

منطقي Scanner::fetchMoreTokens() {
  אם (IsStartOfStream)
    החזר scanStreamStart();

  scanToNextToken();

  אם (Current == End)
    החזר scanStreamEnd();

  removeStaleSimpleKeyCandidates();

  unrollIndent(Column);

  אם (Column == 0 && *Current == '%')
    החזר scanDirective();

  אם (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    החזר scanDocumentIndicator(אמת);

  אם (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    החזר scanDocumentIndicator(שקר);

  אם (*Current == '[')
    החזר scanFlowCollectionStart(אמת);

  אם (*Current == '{')
    החזר scanFlowCollectionStart(שקר);

  אם (*Current == ']')
    החזר scanFlowCollectionEnd(אמת);

  אם (*Current == '}')
    החזר scanFlowCollectionEnd(שקר);

  אם (*Current == ',')
    החזר scanFlowEntry();

  אם (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    החזר scanBlockEntry();

  אם (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    החזר scanKey();

  אם (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || IsAdjacentValueAllowedInFlow))
    החזר scanValue();

  אם (*Current == '*')
    החזר scanAliasOrAnchor(אמת);

  אם (*Current == '&')
    החזר scanAliasOrAnchor(שקר);

  אם (*Current == '!')
    החזר scanTag();

  אם (*Current == '|' && !FlowLevel)
    החזר scanBlockScalar(אמת);

  אם (*Current == '>' && !FlowLevel)
    החזר scanBlockScalar(שקר);

  אם (*Current == '\'')
    החזר scanFlowScalar(שקר);

  אם (*Current == '"')
    החזר scanFlowScalar(אמת);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  אם ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    החזר scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  החזר שקר;
}

Stream::Stream(StringRef Input, SourceMgr &SM, منطقي ShowColors,
               std::error_code *EC)
    : scanner(جديد Scanner(Input, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef InputBuffer, SourceMgr &SM, منطقي ShowColors,
               std::error_code *EC)
    : scanner(جديد Scanner(InputBuffer, SM, ShowColors, EC)) {}

Stream::~Stream() = افتراضي;

منطقي Stream::failed() { החזר scanner->failed(); }

فراغ Stream::printError(Node *N, ثابت Twine &Msg, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), Msg, Kind);
}

فراغ Stream::printError(ثابت SMRange &Range, ثابت Twine &Msg,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, Msg, Range);
}

document_iterator Stream::begin() {
  אם (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(جديد Document(*هذا));
  החזר document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  החזר document_iterator();
}

فراغ Stream::skip() {
  עבור (Document &Doc : *هذا)
    Doc.skip();
}

Node::Node(unsigned صحيح Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string Node::getVerbatimTag() ثابت {
  StringRef Raw = getRawTag();
  אם (!Raw.empty() && Raw != "!") {
    std::string Ret;
    אם (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      החזר Ret;
    } אחרת אם (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      החזר Ret;
    } אחרת {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      אם (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      אחרת {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      החזר Ret;
    }
  }

  بدل (getType()) {
  حالة NK_Null:
    החזר "tag:yaml.org,2002:null";
  حالة NK_Scalar:
  حالة NK_BlockScalar:
    // TODO: Tag resolution.
    החזר "tag:yaml.org,2002:str";
  حالة NK_Mapping:
    החזר "tag:yaml.org,2002:map";
  حالة NK_Sequence:
    החזר "tag:yaml.org,2002:seq";
  }

  החזר "";
}

Token &Node::peekNext() {
  החזר Doc->peekNext();
}

Token Node::getNext() {
  החזר Doc->getNext();
}

Node *Node::parseBlockNode() {
  החזר Doc->parseBlockNode();
}

BumpPtrAllocator &Node::getAllocator() {
  החזר Doc->NodeAllocator;
}

فراغ Node::setError(ثابت Twine &Msg, Token &Tok) ثابت {
  Doc->setError(Msg, Tok);
}

منطقي Node::failed() ثابت {
  החזר Doc->failed();
}

StringRef ScalarNode::getValue(SmallVectorImpl<حرف> &Storage) ثابت {
  אם (Value[0] == '"')
    החזר getDoubleQuotedValue(Value, Storage);
  אם (Value[0] == '\'')
    החזר getSingleQuotedValue(Value, Storage);
  החזר getPlainValue(Value, Storage);
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
ساكن StringRef
parseScalarValue(StringRef UnquotedValue, SmallVectorImpl<حرف> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<حرف> &)>
                     UnescapeCallback) {
  size_t I = UnquotedValue.find_first_of(LookupChars);
  אם (I == StringRef::npos)
    החזר UnquotedValue;

  Storage.clear();
  Storage.reserve(UnquotedValue.size());
  حرف LastNewLineAddedAs = '\0';
  עבור (; I != StringRef::npos; I = UnquotedValue.find_first_of(LookupChars)) {
    אם (UnquotedValue[I] != '\r' && UnquotedValue[I] != '\n') {
      llvm::append_range(Storage, UnquotedValue.take_front(I));
      UnquotedValue = UnescapeCallback(UnquotedValue.drop_front(I), Storage);
      LastNewLineAddedAs = '\0';
      استمر;
    }
    אם (size_t LastNonSWhite = UnquotedValue.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, UnquotedValue.take_front(LastNonSWhite + 1));
      Storage.push_back(' ');
      LastNewLineAddedAs = ' ';
    } אחרת {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      بدل (LastNewLineAddedAs) {
      حالة ' ':
        أكد(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        LastNewLineAddedAs = '\n';
        اكسر;
      حالة '\n':
        أكد(!Storage.empty() && Storage.back() == '\n');
        Storage.push_back('\n');
        اكسر;
      افتراضي:
        Storage.push_back(' ');
        LastNewLineAddedAs = ' ';
        اكسر;
      }
    }
    // Handle Windows-style EOL
    אם (UnquotedValue.substr(I, 2) == "\r\n")
      I++;
    UnquotedValue = UnquotedValue.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, UnquotedValue);
  החזר StringRef(Storage.begin(), Storage.size());
}

StringRef
ScalarNode::getDoubleQuotedValue(StringRef RawValue,
                                 SmallVectorImpl<حرف> &Storage) ثابت {
  أكد(RawValue.size() >= 2 && RawValue.front() == '"' &&
         RawValue.back() == '"');
  StringRef UnquotedValue = RawValue.substr(1, RawValue.size() - 2);

  auto UnescapeFunc = [هذا](StringRef UnquotedValue,
                             SmallVectorImpl<حرف> &Storage) {
    أكد(UnquotedValue.take_front(1) == "\\");
    אם (UnquotedValue.size() == 1) {
      Token T;
      T.Range = UnquotedValue;
      setError("Unrecognized escape code", T);
      Storage.clear();
      החזר StringRef();
    }
    UnquotedValue = UnquotedValue.drop_front(1);
    بدل (UnquotedValue[0]) {
    افتراضي: {
      Token T;
      T.Range = UnquotedValue.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      החזר StringRef();
    }
    حالة '\r':
      // Shrink the Windows-style EOL.
      אם (UnquotedValue.size() >= 2 && UnquotedValue[1] == '\n')
        UnquotedValue = UnquotedValue.drop_front(1);
      [[fallthrough]];
    حالة '\n':
      החזר UnquotedValue.drop_front(1).ltrim(" \t");
    حالة '0':
      Storage.push_back(0x00);
      اكسر;
    حالة 'a':
      Storage.push_back(0x07);
      اكسر;
    حالة 'b':
      Storage.push_back(0x08);
      اكسر;
    حالة 't':
    حالة 0x09:
      Storage.push_back(0x09);
      اكسر;
    حالة 'n':
      Storage.push_back(0x0A);
      اكسر;
    حالة 'v':
      Storage.push_back(0x0B);
      اكسر;
    حالة 'f':
      Storage.push_back(0x0C);
      اكسر;
    حالة 'r':
      Storage.push_back(0x0D);
      اكسر;
    حالة 'e':
      Storage.push_back(0x1B);
      اكسر;
    حالة ' ':
      Storage.push_back(0x20);
      اكسر;
    حالة '"':
      Storage.push_back(0x22);
      اكسر;
    حالة '/':
      Storage.push_back(0x2F);
      اكسر;
    حالة '\\':
      Storage.push_back(0x5C);
      اكسر;
    حالة 'N':
      encodeUTF8(0x85, Storage);
      اكسر;
    حالة '_':
      encodeUTF8(0xA0, Storage);
      اكسر;
    حالة 'L':
      encodeUTF8(0x2028, Storage);
      اكسر;
    حالة 'P':
      encodeUTF8(0x2029, Storage);
      اكسر;
    حالة 'x': {
      אם (UnquotedValue.size() < 3)
        // TODO: Report error.
        اكسر;
      unsigned صحيح UnicodeScalarValue;
      אם (UnquotedValue.substr(1, 2).getAsInteger(16, UnicodeScalarValue))
        // TODO: Report error.
        UnicodeScalarValue = 0xFFFD;
      encodeUTF8(UnicodeScalarValue, Storage);
      החזר UnquotedValue.drop_front(3);
    }
    حالة 'u': {
      אם (UnquotedValue.size() < 5)
        // TODO: Report error.
        اكسر;
      unsigned صحيح UnicodeScalarValue;
      אם (UnquotedValue.substr(1, 4).getAsInteger(16, UnicodeScalarValue))
        // TODO: Report error.
        UnicodeScalarValue = 0xFFFD;
      encodeUTF8(UnicodeScalarValue, Storage);
      החזר UnquotedValue.drop_front(5);
    }
    حالة 'U': {
      אם (UnquotedValue.size() < 9)
        // TODO: Report error.
        اكسر;
      unsigned صحيح UnicodeScalarValue;
      אם (UnquotedValue.substr(1, 8).getAsInteger(16, UnicodeScalarValue))
        // TODO: Report error.
        UnicodeScalarValue = 0xFFFD;
      encodeUTF8(UnicodeScalarValue, Storage);
      החזר UnquotedValue.drop_front(9);
    }
    }
    החזר UnquotedValue.drop_front(1);
  };

  החזר parseScalarValue(UnquotedValue, Storage, "\\\r\n", UnescapeFunc);
}

StringRef ScalarNode::getSingleQuotedValue(StringRef RawValue,
                                           SmallVectorImpl<حرف> &Storage) {
  أكد(RawValue.size() >= 2 && RawValue.front() == '\'' &&
         RawValue.back() == '\'');
  StringRef UnquotedValue = RawValue.substr(1, RawValue.size() - 2);

  auto UnescapeFunc = [](StringRef UnquotedValue,
                         SmallVectorImpl<حرف> &Storage) {
    أكد(UnquotedValue.take_front(2) == "''");
    Storage.push_back('\'');
    החזר UnquotedValue.drop_front(2);
  };

  החזר parseScalarValue(UnquotedValue, Storage, "'\r\n", UnescapeFunc);
}

StringRef ScalarNode::getPlainValue(StringRef RawValue,
                                    SmallVectorImpl<حرف> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  RawValue = RawValue.rtrim("\r\n \t");
  החזר parseScalarValue(RawValue, Storage, "\r\n", nullptr);
}

Node *KeyValueNode::getKey() {
  אם (Key)
    החזר Key;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    אם (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_Value
        || t.Kind == Token::TK_Error) {
      החזר Key = جديد (getAllocator()) NullNode(Doc);
    }
    אם (t.Kind == Token::TK_Key)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  אם (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_Value) {
    החזר Key = جديد (getAllocator()) NullNode(Doc);
  }

  // We've got a normal key.
  החזר Key = parseBlockNode();
}

Node *KeyValueNode::getValue() {
  אם (Value)
    החזר Value;

  אם (Node* Key = getKey())
    Key->skip();
  אחרת {
    setError("Null key in Key Value.", peekNext());
    החזר Value = جديد (getAllocator()) NullNode(Doc);
  }

  אם (failed())
    החזר Value = جديد (getAllocator()) NullNode(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    אם (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_Key
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      החזר Value = جديد (getAllocator()) NullNode(Doc);
    }

    אם (t.Kind != Token::TK_Value) {
      setError("Unexpected token in Key Value.", t);
      החזר Value = جديد (getAllocator()) NullNode(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  אם (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_Key) {
    החזר Value = جديد (getAllocator()) NullNode(Doc);
  }

  // We got a normal value.
  החזר Value = parseBlockNode();
}

فراغ MappingNode::increment() {
  אם (failed()) {
    IsAtEnd = אמת;
    CurrentEntry = nullptr;
    החזר;
  }
  אם (CurrentEntry) {
    CurrentEntry->skip();
    אם (Type == MT_Inline) {
      IsAtEnd = אמת;
      CurrentEntry = nullptr;
      החזר;
    }
  }
  Token T = peekNext();
  אם (T.Kind == Token::TK_Key || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = جديد (getAllocator()) KeyValueNode(Doc);
  } אחרת אם (Type == MT_Block) {
    بدل (T.Kind) {
    حالة Token::TK_BlockEnd:
      getNext();
      IsAtEnd = אמת;
      CurrentEntry = nullptr;
      اكسر;
    افتراضي:
      setError("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    حالة Token::TK_Error:
      IsAtEnd = אמת;
      CurrentEntry = nullptr;
    }
  } אחרת {
    بدل (T.Kind) {
    حالة Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      החזר increment();
    حالة Token::TK_FlowMappingEnd:
      getNext();
      [[fallthrough]];
    حالة Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = אמת;
      CurrentEntry = nullptr;
      اكسر;
    افتراضي:
      setError( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      IsAtEnd = אמת;
      CurrentEntry = nullptr;
    }
  }
}

فراغ SequenceNode::increment() {
  אם (failed()) {
    IsAtEnd = אמת;
    CurrentEntry = nullptr;
    החזר;
  }
  אם (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  אם (SeqType == ST_Block) {
    بدل (T.Kind) {
    حالة Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parseBlockNode();
      אם (!CurrentEntry) { // An error occurred.
        IsAtEnd = אמת;
        CurrentEntry = nullptr;
      }
      اكسر;
    حالة Token::TK_BlockEnd:
      getNext();
      IsAtEnd = אמת;
      CurrentEntry = nullptr;
      اكسر;
    افتراضي:
      setError( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    حالة Token::TK_Error:
      IsAtEnd = אמת;
      CurrentEntry = nullptr;
    }
  } אחרת אם (SeqType == ST_Indentless) {
    بدل (T.Kind) {
    حالة Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parseBlockNode();
      אם (!CurrentEntry) { // An error occurred.
        IsAtEnd = אמת;
        CurrentEntry = nullptr;
      }
      اكسر;
    افتراضي:
    حالة Token::TK_Error:
      IsAtEnd = אמת;
      CurrentEntry = nullptr;
    }
  } אחרת אם (SeqType == ST_Flow) {
    بدل (T.Kind) {
    حالة Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      WasPreviousTokenFlowEntry = אמת;
      החזר increment();
    حالة Token::TK_FlowSequenceEnd:
      getNext();
      [[fallthrough]];
    حالة Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = אמת;
      CurrentEntry = nullptr;
      اكسر;
    حالة Token::TK_StreamEnd:
    حالة Token::TK_DocumentEnd:
    حالة Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = אמת;
      CurrentEntry = nullptr;
      اكسر;
    افتراضي:
      אם (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = אמת;
        CurrentEntry = nullptr;
        اكسر;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parseBlockNode();
      אם (!CurrentEntry) {
        IsAtEnd = אמת;
      }
      WasPreviousTokenFlowEntry = שקר;
      اكسر;
    }
  }
}

Document::Document(Stream &S) : stream(S), Root(nullptr) {
  // Tag maps starts with two default mappings.
  TagMap["!"] = "!";
  TagMap["!!"] = "tag:yaml.org,2002:";

  אם (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peekNext();
  אם (T.Kind == Token::TK_DocumentStart)
    getNext();
}

منطقي Document::skip()  {
  אם (stream.scanner->failed())
    החזר שקר;
  אם (!Root && !getRoot())
    החזר שקר;
  Root->skip();
  Token &T = peekNext();
  אם (T.Kind == Token::TK_StreamEnd)
    החזר שקר;
  אם (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    החזר skip();
  }
  החזר אמת;
}

Token &Document::peekNext() {
  החזר stream.scanner->peekNext();
}

Token Document::getNext() {
  החזר stream.scanner->getNext();
}

فراغ Document::setError(ثابت Twine &Message, Token &Location) ثابت {
  stream.scanner->setError(Message, Location.Range.begin());
}

منطقي Document::failed() ثابت {
  החזר stream.scanner->failed();
}

Node *Document::parseBlockNode() {
  Token T = peekNext();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  بدل (T.Kind) {
  حالة Token::TK_Alias:
    getNext();
    החזר جديد (NodeAllocator) AliasNode(stream.CurrentDoc, T.Range.substr(1));
  حالة Token::TK_Anchor:
    אם (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      החזר nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    اذهب parse_property;
  حالة Token::TK_Tag:
    אם (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      החזר nullptr;
    }
    TagInfo = getNext(); // Consume TK_Tag.
    T = peekNext();
    اذهب parse_property;
  افتراضي:
    اكسر;
  }

  بدل (T.Kind) {
  حالة Token::TK_BlockEntry:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    החזר جديد (NodeAllocator) SequenceNode( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , SequenceNode::ST_Indentless);
  حالة Token::TK_BlockSequenceStart:
    getNext();
    החזר جديد (NodeAllocator)
      SequenceNode( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , SequenceNode::ST_Block);
  حالة Token::TK_BlockMappingStart:
    getNext();
    החזר جديد (NodeAllocator)
      MappingNode( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , MappingNode::MT_Block);
  حالة Token::TK_FlowSequenceStart:
    getNext();
    החזר جديد (NodeAllocator)
      SequenceNode( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , SequenceNode::ST_Flow);
  حالة Token::TK_FlowMappingStart:
    getNext();
    החזר جديد (NodeAllocator)
      MappingNode( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , MappingNode::MT_Flow);
  حالة Token::TK_Scalar:
    getNext();
    החזר جديد (NodeAllocator)
      ScalarNode( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  حالة Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.Value.c_str(), T.Value.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(NodeAllocator).drop_back();
    החזר جديد (NodeAllocator)
        BlockScalarNode(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  حالة Token::TK_Key:
    // Don't eat the TK_Key, KeyValueNode expects it.
    החזר جديد (NodeAllocator)
      MappingNode( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , MappingNode::MT_Inline);
  حالة Token::TK_DocumentStart:
  حالة Token::TK_DocumentEnd:
  حالة Token::TK_StreamEnd:
  افتراضي:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    החזר جديد (NodeAllocator) NullNode(stream.CurrentDoc);
  حالة Token::TK_FlowMappingEnd:
  حالة Token::TK_FlowSequenceEnd:
  حالة Token::TK_FlowEntry: {
    אם (Root && (isa<MappingNode>(Root) || isa<SequenceNode>(Root)))
      החזר جديد (NodeAllocator) NullNode(stream.CurrentDoc);

    setError("Unexpected token", T);
    החזר nullptr;
  }
  حالة Token::TK_Error:
    החזר nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  החזר nullptr;
}

منطقي Document::parseDirectives() {
  منطقي isDirective = שקר;
  כלעוד (אמת) {
    Token T = peekNext();
    אם (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = אמת;
    } אחרת אם (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = אמת;
    } אחרת {
      اكسر;
    }
  }
  החזר isDirective;
}

فراغ Document::parseYAMLDirective() {
  getNext(); // Eat %YAML <version>
}

فراغ Document::parseTAGDirective() {
  Token Tag = getNext(); // %TAG <handle> <prefix>
  StringRef T = Tag.Range;
  // Strip %TAG
  T = T.substr(T.find_first_of(" \t")).ltrim(" \t");
  std::size_t HandleEnd = T.find_first_of(" \t");
  StringRef TagHandle = T.substr(0, HandleEnd);
  StringRef TagPrefix = T.substr(HandleEnd).ltrim(" \t");
  TagMap[TagHandle] = TagPrefix;
}

منطقي Document::expectToken(صحيح TK) {
  Token T = getNext();
  אם (T.Kind != TK) {
    setError("Unexpected token", T);
    החזר שקר;
  }
  החזר אמת;
}
