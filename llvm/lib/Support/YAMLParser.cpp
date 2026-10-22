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
ساكن EncodingInfo getUnicodeEncoding(StringRef داخلہ) {
  إذا (داخلہ.empty())
    أرجع {UEF_Unknown, 0};

  بدل (uint8_t(داخلہ[0])) {
  حالة 0x00:
    إذا (داخلہ.size() >= 4) {
      إذا (  داخلہ[1] == 0
         && uint8_t(داخلہ[2]) == 0xFE
         && uint8_t(داخلہ[3]) == 0xFF)
        أرجع {UEF_UTF32_BE, 4};
      إذا (داخلہ[1] == 0 && داخلہ[2] == 0 && داخلہ[3] != 0)
        أرجع {UEF_UTF32_BE, 0};
    }

    إذا (داخلہ.size() >= 2 && داخلہ[1] != 0)
      أرجع {UEF_UTF16_BE, 0};
    أرجع {UEF_Unknown, 0};
  حالة 0xFF:
    إذا (  داخلہ.size() >= 4
       && uint8_t(داخلہ[1]) == 0xFE
       && داخلہ[2] == 0
       && داخلہ[3] == 0)
      أرجع {UEF_UTF32_LE, 4};

    إذا (داخلہ.size() >= 2 && uint8_t(داخلہ[1]) == 0xFE)
      أرجع {UEF_UTF16_LE, 2};
    أرجع {UEF_Unknown, 0};
  حالة 0xFE:
    إذا (داخلہ.size() >= 2 && uint8_t(داخلہ[1]) == 0xFF)
      أرجع {UEF_UTF16_BE, 2};
    أرجع {UEF_Unknown, 0};
  حالة 0xEF:
    إذا (  داخلہ.size() >= 3
       && uint8_t(داخلہ[1]) == 0xBB
       && uint8_t(داخلہ[2]) == 0xBF)
      أرجع {UEF_UTF8, 3};
    أرجع {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  إذا (داخلہ.size() >= 4 && داخلہ[1] == 0 && داخلہ[2] == 0 && داخلہ[3] == 0)
    أرجع {UEF_UTF32_LE, 0};

  إذا (داخلہ.size() >= 2 && داخلہ[1] == 0)
    أرجع {UEF_UTF16_LE, 0};

  أرجع {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
فراغ گرہ::anchor() {}
فراغ Null_گرہ::anchor() {}
فراغ Scalar_گرہ::anchor() {}
فراغ Block_Scalar_گرہ::anchor() {}
فراغ کلید_قدر_گرہ::anchor() {}
فراغ Mapping_گرہ::anchor() {}
فراغ Sequence_گرہ::anchor() {}
فراغ Alias_گرہ::anchor() {}

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
    TK_کلید,
    TK_قدر,
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
  std::string قدر;

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
بنية Simple_کلید {
  TokenQueueT::iterator Tok;
  unsigned Column = 0;
  unsigned سطر = 0;
  unsigned FlowLevel = 0;
  منطقي IsRequired = خطأ;

  منطقي operator ==(ثابت Simple_کلید &Other) {
    أرجع Tok == Other.Tok;
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
  إذا (Position < End && (*Position & 0x80) == 0) {
    أرجع {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  إذا (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    إذا (codepoint >= 0x80)
      أرجع {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  إذا (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    إذا (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      أرجع {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  إذا (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    إذا (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      أرجع {codepoint, 4};
  }
  أرجع {0, 0};
}

نطاق llvm {
نطاق yaml {

/// Scans YAML tokens from a MemoryBuffer.
صنف Scanner {
عام:
  Scanner(StringRef داخلہ, SourceMgr &SM, منطقي ShowColors = true,
          std::error_code *EC = nullptr);
  Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, منطقي ShowColors = true,
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
    إذا (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    إذا (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    إذا (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = true;
  }

  /// Returns true if an error occurred while parsing.
  منطقي failed() {
    أرجع Failed;
  }

خاص:
  فراغ init(MemoryBufferRef Buffer);

  StringRef current_داخلہ() {
    أرجع StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    أرجع ::decodeUTF8(StringRef(Position, End - Position));
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
  منطقي is_سطر_Empty(StringRef سطر);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  منطقي consume_سطر_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  فراغ save_Simple_کلید_Candidate( TokenQueueT::iterator Tok
                             , unsigned AtColumn
                             , منطقي IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  فراغ remove_Stale_Simple_کلید_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  فراغ remove_Simple_کلید_Candidates_On_Flow_Level(unsigned Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  منطقي unrollIndent(صحيح ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  منطقي rollIndent( صحيح ToColumn
                 , Token::TokenKind Kind
                 , TokenQueueT::iterator داخل_کرو_Point);

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
  منطقي scan_کلید();

  /// Scan an explicit : indicating a value.
  منطقي scan_قدر();

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
                             unsigned &سطر_Breaks, منطقي &IsDone);

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
  MemoryBufferRef داخلہ_Buffer;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator End;

  /// Current YAML indentation level in spaces.
  صحيح Indent;

  /// Current column number in Unicode code points.
  unsigned Column;

  /// Current line number.
  unsigned سطر;

  /// How deep we are in flow style containers. 0 Means at block level.
  unsigned FlowLevel;

  /// Are we at the start of the stream?
  منطقي IsStartOfStream;

  /// Can the next token be the start of a simple key?
  منطقي Is_Simple_کلید_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  منطقي Is_Adjacent_قدر_Allowed_In_Flow;

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
  SmallVector<Simple_کلید, 4> Simple_کلیدیں;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
ساكن فراغ encodeUTF8( uint32_t Unicode_Scalar_قدر
                      , SmallVectorImpl<حرف> &Result) {
  إذا (Unicode_Scalar_قدر <= 0x7F) {
    Result.ڈالو_back(Unicode_Scalar_قدر & 0x7F);
  } وإلا إذا (Unicode_Scalar_قدر <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((Unicode_Scalar_قدر & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Unicode_Scalar_قدر & 0x3F);
    Result.ڈالو_back(FirstByte);
    Result.ڈالو_back(SecondByte);
  } وإلا إذا (Unicode_Scalar_قدر <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((Unicode_Scalar_قدر & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_قدر & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Unicode_Scalar_قدر & 0x3F);
    Result.ڈالو_back(FirstByte);
    Result.ڈالو_back(SecondByte);
    Result.ڈالو_back(ThirdByte);
  } وإلا إذا (Unicode_Scalar_قدر <= 0x10FFFF) {
    uint8_t FirstByte = 0xF0 | ((Unicode_Scalar_قدر & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_قدر & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Unicode_Scalar_قدر & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_قدر & 0x3F);
    Result.ڈالو_back(FirstByte);
    Result.ڈالو_back(SecondByte);
    Result.ڈالو_back(ThirdByte);
    Result.ڈالو_back(FourthByte);
  }
}

منطقي yaml::dumpTokens(StringRef داخلہ, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(داخلہ, SM);
  طالما (true) {
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
    حالة Token::TK_کلید:
      OS << "Key: ";
      اكسر;
    حالة Token::TK_قدر:
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
    إذا (T.Kind == Token::TK_StreamEnd)
      اكسر;
    وإلا إذا (T.Kind == Token::TK_Error)
      أرجع خطأ;
  }
  أرجع true;
}

منطقي yaml::scanTokens(StringRef داخلہ) {
  SourceMgr SM;
  Scanner scanner(داخلہ, SM);
  طالما (true) {
    Token T = scanner.getNext();
    إذا (T.Kind == Token::TK_StreamEnd)
      اكسر;
    وإلا إذا (T.Kind == Token::TK_Error)
      أرجع خطأ;
  }
  أرجع true;
}

std::string yaml::escape(StringRef داخلہ, منطقي EscapePrintable) {
  std::string Escaped_داخلہ;
  لكل (StringRef::iterator i = داخلہ.begin(), e = داخلہ.end(); i != e; ++i) {
    إذا (*i == '\\')
      Escaped_داخلہ += "\\\\";
    وإلا إذا (*i == '"')
      Escaped_داخلہ += "\\\"";
    وإلا إذا (*i == 0)
      Escaped_داخلہ += "\\0";
    وإلا إذا (*i == 0x07)
      Escaped_داخلہ += "\\a";
    وإلا إذا (*i == 0x08)
      Escaped_داخلہ += "\\b";
    وإلا إذا (*i == 0x09)
      Escaped_داخلہ += "\\t";
    وإلا إذا (*i == 0x0A)
      Escaped_داخلہ += "\\n";
    وإلا إذا (*i == 0x0B)
      Escaped_داخلہ += "\\v";
    وإلا إذا (*i == 0x0C)
      Escaped_داخلہ += "\\f";
    وإلا إذا (*i == 0x0D)
      Escaped_داخلہ += "\\r";
    وإلا إذا (*i == 0x1B)
      Escaped_داخلہ += "\\e";
    وإلا إذا ((unsigned حرف)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_داخلہ += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } وإلا إذا (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_قدر
        = decodeUTF8(StringRef(i, داخلہ.end() - i));
      إذا (Unicode_Scalar_قدر.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_داخلہ, Val);
        // FIXME: Error reporting.
        أرجع Escaped_داخلہ;
      }
      إذا (Unicode_Scalar_قدر.first == 0x85)
        Escaped_داخلہ += "\\N";
      وإلا إذا (Unicode_Scalar_قدر.first == 0xA0)
        Escaped_داخلہ += "\\_";
      وإلا إذا (Unicode_Scalar_قدر.first == 0x2028)
        Escaped_داخلہ += "\\L";
      وإلا إذا (Unicode_Scalar_قدر.first == 0x2029)
        Escaped_داخلہ += "\\P";
      وإلا إذا (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_قدر.first))
        Escaped_داخلہ += StringRef(i, Unicode_Scalar_قدر.second);
      وإلا {
        std::string HexStr = utohexstr(Unicode_Scalar_قدر.first);
        إذا (HexStr.size() <= 2)
          Escaped_داخلہ += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        وإلا إذا (HexStr.size() <= 4)
          Escaped_داخلہ += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        وإلا إذا (HexStr.size() <= 8)
          Escaped_داخلہ += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += Unicode_Scalar_قدر.second - 1;
    } وإلا {
      Escaped_داخلہ.ڈالو_back(*i);
    }
  }
  أرجع Escaped_داخلہ;
}

std::optional<منطقي> yaml::parseBool(StringRef S) {
  بدل (S.size()) {
  حالة 1:
    بدل (S.front()) {
    حالة 'y':
    حالة 'Y':
      أرجع true;
    حالة 'n':
    حالة 'N':
      أرجع خطأ;
    افتراضي:
      أرجع std::nullopt;
    }
  حالة 2:
    بدل (S.front()) {
    حالة 'O':
      إذا (S[1] == 'N') // ON
        أرجع true;
      [[fallthrough]];
    حالة 'o':
      إذا (S[1] == 'n') //[Oo]n
        أرجع true;
      أرجع std::nullopt;
    حالة 'N':
      إذا (S[1] == 'O') // NO
        أرجع خطأ;
      [[fallthrough]];
    حالة 'n':
      إذا (S[1] == 'o') //[Nn]o
        أرجع خطأ;
      أرجع std::nullopt;
    افتراضي:
      أرجع std::nullopt;
    }
  حالة 3:
    بدل (S.front()) {
    حالة 'O':
      إذا (S.drop_front() == "FF") // OFF
        أرجع خطأ;
      [[fallthrough]];
    حالة 'o':
      إذا (S.drop_front() == "ff") //[Oo]ff
        أرجع خطأ;
      أرجع std::nullopt;
    حالة 'Y':
      إذا (S.drop_front() == "ES") // YES
        أرجع true;
      [[fallthrough]];
    حالة 'y':
      إذا (S.drop_front() == "es") //[Yy]es
        أرجع true;
      أرجع std::nullopt;
    افتراضي:
      أرجع std::nullopt;
    }
  حالة 4:
    بدل (S.front()) {
    حالة 'T':
      إذا (S.drop_front() == "RUE") // TRUE
        أرجع true;
      [[fallthrough]];
    حالة 't':
      إذا (S.drop_front() == "rue") //[Tt]rue
        أرجع true;
      أرجع std::nullopt;
    افتراضي:
      أرجع std::nullopt;
    }
  حالة 5:
    بدل (S.front()) {
    حالة 'F':
      إذا (S.drop_front() == "ALSE") // FALSE
        أرجع خطأ;
      [[fallthrough]];
    حالة 'f':
      إذا (S.drop_front() == "alse") //[Ff]alse
        أرجع خطأ;
      أرجع std::nullopt;
    افتراضي:
      أرجع std::nullopt;
    }
  افتراضي:
    أرجع std::nullopt;
  }
}

Scanner::Scanner(StringRef داخلہ, SourceMgr &sm, منطقي ShowColors,
                 std::error_code *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(MemoryBufferRef(داخلہ, "YAML"));
}

Scanner::Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, منطقي ShowColors,
                 std::error_code *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(Buffer);
}

فراغ Scanner::init(MemoryBufferRef Buffer) {
  داخلہ_Buffer = Buffer;
  Current = داخلہ_Buffer.getBufferStart();
  End = داخلہ_Buffer.getBufferEnd();
  Indent = -1;
  Column = 0;
  سطر = 0;
  FlowLevel = 0;
  IsStartOfStream = true;
  Is_Simple_کلید_Allowed = true;
  Is_Adjacent_قدر_Allowed_In_Flow = خطأ;
  Failed = خطأ;
  std::unique_ptr<MemoryBuffer> داخلہ_Buffer_Owner =
      MemoryBuffer::getMemBuffer(Buffer, /*RequiresNullTerminator=*/خطأ);
  SM.AddNewSourceBuffer(std::move(داخلہ_Buffer_Owner), SMLoc());
}

Token &Scanner::peekNext() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  منطقي NeedMore = خطأ;
  طالما (true) {
    إذا (TokenQueue.empty() || NeedMore) {
      إذا (!fetchMoreTokens()) {
        TokenQueue.clear();
        Simple_کلیدیں.clear();
        TokenQueue.ڈالو_back(Token());
        أرجع TokenQueue.front();
      }
    }
    أكد(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_کلید_Candidates();
    Simple_کلید SK;
    SK.Tok = TokenQueue.begin();
    إذا (!is_contained(Simple_کلیدیں, SK))
      اكسر;
    وإلا
      NeedMore = true;
  }
  أرجع TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  إذا (!TokenQueue.empty())
    TokenQueue.نکالو_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  إذا (TokenQueue.empty())
    TokenQueue.resetAlloc();

  أرجع Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  إذا (Position == End)
    أرجع Position;
  // Check 7 bit c-printable - b-char.
  إذا (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    أرجع Position + 1;

  // Check for valid UTF-8.
  إذا (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    إذا (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      أرجع Position + u8d.second;
  }
  أرجع Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  إذا (Position == End)
    أرجع Position;
  إذا (*Position == 0x0D) {
    إذا (Position + 1 != End && *(Position + 1) == 0x0A)
      أرجع Position + 2;
    أرجع Position + 1;
  }

  إذا (*Position == 0x0A)
    أرجع Position + 1;
  أرجع Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  إذا (Position == End)
    أرجع Position;
  إذا (*Position == ' ')
    أرجع Position + 1;
  أرجع Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  إذا (Position == End)
    أرجع Position;
  إذا (*Position == ' ' || *Position == '\t')
    أرجع Position + 1;
  أرجع Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  إذا (Position == End)
    أرجع Position;
  إذا (*Position == ' ' || *Position == '\t')
    أرجع Position;
  أرجع skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  طالما (true) {
    StringRef::iterator i = (هذا->*Func)(Position);
    إذا (i == Position)
      اكسر;
    Position = i;
  }
  أرجع Position;
}

فراغ Scanner::advanceWhile(SkipWhileFunc Func) {
  auto Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

ساكن منطقي is_ns_hex_digit(ثابت حرف C) { أرجع isAlnum(C); }

ساكن منطقي is_ns_word_char(ثابت حرف C) { أرجع C == '-' || isAlpha(C); }

فراغ Scanner::scan_ns_uri_char() {
  طالما (true) {
    إذا (Current == End)
      اكسر;
    إذا ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } وإلا {
      اكسر;
    }
  }
}

منطقي Scanner::consume(uint32_t Expected) {
  إذا (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    أرجع خطأ;
  }
  إذا (Current == End)
    أرجع خطأ;
  إذا (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    أرجع خطأ;
  }
  إذا (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    أرجع true;
  }
  أرجع خطأ;
}

فراغ Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  أكد(Current <= End && "Skipped past the end");
}

منطقي Scanner::isBlankOrBreak(StringRef::iterator Position) {
  إذا (Position == End)
    أرجع خطأ;
  أرجع *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

منطقي Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  إذا (Position == End || isBlankOrBreak(Position))
    أرجع خطأ;
  إذا (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    أرجع خطأ;
  أرجع true;
}

منطقي Scanner::is_سطر_Empty(StringRef سطر) {
  لكل (ثابت auto *Position = سطر.begin(); Position != سطر.end(); ++Position)
    إذا (!isBlankOrBreak(Position))
      أرجع خطأ;
  أرجع true;
}

منطقي Scanner::consume_سطر_Break_If_Present() {
  auto Next = skip_b_break(Current);
  إذا (Next == Current)
    أرجع خطأ;
  Column = 0;
  ++سطر;
  Current = Next;
  أرجع true;
}

فراغ Scanner::save_Simple_کلید_Candidate( TokenQueueT::iterator Tok
                                    , unsigned AtColumn
                                    , منطقي IsRequired) {
  إذا (Is_Simple_کلید_Allowed) {
    Simple_کلید SK;
    SK.Tok = Tok;
    SK.سطر = سطر;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_کلیدیں.ڈالو_back(SK);
  }
}

فراغ Scanner::remove_Stale_Simple_کلید_Candidates() {
  لكل (SmallVectorImpl<Simple_کلید>::iterator i = Simple_کلیدیں.begin();
                                            i != Simple_کلیدیں.end();) {
    إذا (i->سطر != سطر || i->Column + 1024 < Column) {
      إذا (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_کلیدیں.erase(i);
    } وإلا {
      ++i;
    }
  }
}

فراغ Scanner::remove_Simple_کلید_Candidates_On_Flow_Level(unsigned Level) {
  إذا (!Simple_کلیدیں.empty() && (Simple_کلیدیں.end() - 1)->FlowLevel == Level)
    Simple_کلیدیں.نکالو_back();
}

منطقي Scanner::unrollIndent(صحيح ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  إذا (FlowLevel != 0)
    أرجع true;

  طالما (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.ڈالو_back(T);
    Indent = Indents.نکالو_back_val();
  }

  أرجع true;
}

منطقي Scanner::rollIndent( صحيح ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator داخل_کرو_Point) {
  إذا (FlowLevel)
    أرجع true;
  إذا (Indent < ToColumn) {
    Indents.ڈالو_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.داخل_کرو(داخل_کرو_Point, T);
  }
  أرجع true;
}

فراغ Scanner::skipComment() {
  إذا (Current == End || *Current != '#')
    أرجع;
  طالما (true) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    إذا (I == Current)
      اكسر;
    Current = I;
    ++Column;
  }
}

فراغ Scanner::scanToNextToken() {
  طالما (true) {
    طالما (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    إذا (i == Current)
      اكسر;
    Current = i;
    ++سطر;
    Column = 0;
    // New lines may start a simple key.
    إذا (!FlowLevel)
      Is_Simple_کلید_Allowed = true;
  }
}

منطقي Scanner::scanStreamStart() {
  IsStartOfStream = خطأ;

  EncodingInfo EI = getUnicodeEncoding(current_داخلہ());

  Token T;
  T.Kind = Token::TK_StreamStart;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.ڈالو_back(T);
  Current += EI.second;
  أرجع true;
}

منطقي Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  إذا (Column != 0) {
    Column = 0;
    ++سطر;
  }

  unrollIndent(-1);
  Simple_کلیدیں.clear();
  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_In_Flow = خطأ;

  Token T;
  T.Kind = Token::TK_StreamEnd;
  T.Range = StringRef(Current, 0);
  TokenQueue.ڈالو_back(T);
  أرجع true;
}

منطقي Scanner::scanDirective() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_کلیدیں.clear();
  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_In_Flow = خطأ;

  StringRef::iterator Start = Current;
  consume('%');
  StringRef::iterator NameStart = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(NameStart, Current - NameStart);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  إذا (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ڈالو_back(T);
    أرجع true;
  } وإلا إذا(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ڈالو_back(T);
    أرجع true;
  }
  أرجع خطأ;
}

منطقي Scanner::scanDocumentIndicator(منطقي IsStart) {
  unrollIndent(-1);
  Simple_کلیدیں.clear();
  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_In_Flow = خطأ;

  Token T;
  T.Kind = IsStart ? Token::TK_DocumentStart : Token::TK_DocumentEnd;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.ڈالو_back(T);
  أرجع true;
}

منطقي Scanner::scanFlowCollectionStart(منطقي IsSequence) {
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ڈالو_back(T);

  // [ and { may begin a simple key.
  save_Simple_کلید_Candidate(--TokenQueue.end(), Column - 1, خطأ);

  // And may also be followed by a simple key.
  Is_Simple_کلید_Allowed = true;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_قدر_Allowed_In_Flow = خطأ;
  ++FlowLevel;
  أرجع true;
}

منطقي Scanner::scanFlowCollectionEnd(منطقي IsSequence) {
  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_In_Flow = true;
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceEnd
                      : Token::TK_FlowMappingEnd;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ڈالو_back(T);
  إذا (FlowLevel)
    --FlowLevel;
  أرجع true;
}

منطقي Scanner::scanFlowEntry() {
  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = true;
  Is_Adjacent_قدر_Allowed_In_Flow = خطأ;
  Token T;
  T.Kind = Token::TK_FlowEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ڈالو_back(T);
  أرجع true;
}

منطقي Scanner::scanBlockEntry() {
  rollIndent(Column, Token::TK_BlockSequenceStart, TokenQueue.end());
  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = true;
  Is_Adjacent_قدر_Allowed_In_Flow = خطأ;
  Token T;
  T.Kind = Token::TK_BlockEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ڈالو_back(T);
  أرجع true;
}

منطقي Scanner::scan_کلید() {
  إذا (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = !FlowLevel;
  Is_Adjacent_قدر_Allowed_In_Flow = خطأ;

  Token T;
  T.Kind = Token::TK_کلید;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ڈالو_back(T);
  أرجع true;
}

منطقي Scanner::scan_قدر() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  إذا (!Simple_کلیدیں.empty()) {
    Simple_کلید SK = Simple_کلیدیں.نکالو_back_val();
    Token T;
    T.Kind = Token::TK_کلید;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    لكل (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      إذا (i == SK.Tok)
        اكسر;
    }
    إذا (i == e) {
      Failed = true;
      أرجع خطأ;
    }
    i = TokenQueue.داخل_کرو(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    Is_Simple_کلید_Allowed = خطأ;
  } وإلا {
    إذا (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    Is_Simple_کلید_Allowed = !FlowLevel;
  }
  Is_Adjacent_قدر_Allowed_In_Flow = خطأ;

  Token T;
  T.Kind = Token::TK_قدر;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ڈالو_back(T);
  أرجع true;
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
  طالما (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  أرجع (Position - 1 - I) % 2 == 1;
}

منطقي Scanner::scanFlowScalar(منطقي IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  إذا (IsDoubleQuoted) {
    افعل {
      ++Current;
      طالما (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } طالما (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } وإلا {
    skip(1);
    طالما (Current != End) {
      // Skip a ' followed by another '.
      إذا (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        استمر;
      } وإلا إذا (*Current == '\'')
        اكسر;
      StringRef::iterator i = skip_nb_char(Current);
      إذا (i == Current) {
        i = skip_b_break(Current);
        إذا (i == Current)
          اكسر;
        Current = i;
        Column = 0;
        ++سطر;
      } وإلا {
        إذا (i == End)
          اكسر;
        Current = i;
        ++Column;
      }
    }
  }

  إذا (Current == End) {
    setError("Expected quote at end of scalar", Current);
    أرجع خطأ;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ڈالو_back(T);

  save_Simple_کلید_Candidate(--TokenQueue.end(), ColStart, خطأ);

  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_In_Flow = true;

  أرجع true;
}

منطقي Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  unsigned LeadingBlanks = 0;
  أكد(Indent >= -1 && "Indent must be >= -1 !");
  unsigned indent = static_cast<unsigned>(Indent + 1);
  طالما (Current != End) {
    إذا (*Current == '#')
      اكسر;

    طالما (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      إذا (i == Current)
        اكسر;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    إذا (!isBlankOrBreak(Current))
      اكسر;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    طالما (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      إذا (i != Tmp) {
        إذا (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          أرجع خطأ;
        }
        Tmp = i;
        ++Column;
      } وإلا {
        i = skip_b_break(Tmp);
        إذا (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++سطر;
      }
    }

    إذا (!FlowLevel && Column < indent)
      اكسر;

    Current = Tmp;
  }
  إذا (Start == Current) {
    setError("Got empty plain scalar", Start);
    أرجع خطأ;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ڈالو_back(T);

  // Plain scalars can be simple keys.
  save_Simple_کلید_Candidate(--TokenQueue.end(), ColStart, خطأ);

  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_In_Flow = خطأ;

  أرجع true;
}

منطقي Scanner::scanAliasOrAnchor(منطقي IsAlias) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1);
  طالما (Current != End) {
    إذا (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      اكسر;
    StringRef::iterator i = skip_ns_char(Current);
    إذا (i == Current)
      اكسر;
    Current = i;
    ++Column;
  }

  إذا (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    أرجع خطأ;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ڈالو_back(T);

  // Alias and anchors can be simple keys.
  save_Simple_کلید_Candidate(--TokenQueue.end(), ColStart, خطأ);

  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_In_Flow = خطأ;

  أرجع true;
}

منطقي Scanner::scanBlockScalarIndicators(حرف &StyleIndicator,
                                        حرف &ChompingIndicator,
                                        unsigned &IndentIndicator,
                                        منطقي &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  إذا (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    أرجع خطأ;
  أرجع true;
}

حرف Scanner::scanBlockStyleIndicator() {
  حرف Indicator = ' ';
  إذا (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  أرجع Indicator;
}

حرف Scanner::scanBlockChompingIndicator() {
  حرف Indicator = ' ';
  إذا (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  أرجع Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
ساكن unsigned get_Chomped_سطر_Breaks(حرف ChompingIndicator,
                                     unsigned سطر_Breaks, StringRef Str) {
  إذا (ChompingIndicator == '-') // Strip all line breaks.
    أرجع 0;
  إذا (ChompingIndicator == '+') // Keep all line breaks.
    أرجع سطر_Breaks;
  // Clip trailing lines.
  أرجع Str.empty() ? 0 : 1;
}

unsigned Scanner::scanBlockIndentationIndicator() {
  unsigned Indent = 0;
  إذا (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = unsigned(*Current - '0');
    skip(1);
  }
  أرجع Indent;
}

منطقي Scanner::scanBlockScalarHeader(حرف &ChompingIndicator,
                                    unsigned &IndentIndicator, منطقي &IsDone) {
  auto Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  إذا (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  إذا (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ڈالو_back(T);
    IsDone = true;
    أرجع true;
  }

  إذا (!consume_سطر_Break_If_Present()) {
    setError("Expected a line break after block scalar header", Current);
    أرجع خطأ;
  }
  أرجع true;
}

منطقي Scanner::findBlockScalarIndent(unsigned &BlockIndent,
                                    unsigned BlockExitIndent,
                                    unsigned &سطر_Breaks, منطقي &IsDone) {
  unsigned Max_All_Space_سطر_Characters = 0;
  StringRef::iterator Longest_All_Space_سطر;

  طالما (true) {
    advanceWhile(&Scanner::skip_s_space);
    إذا (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      إذا (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = true;
        أرجع true;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      إذا (Max_All_Space_سطر_Characters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_سطر);
        أرجع خطأ;
      }
      أرجع true;
    }
    إذا (skip_b_break(Current) != Current &&
        Column > Max_All_Space_سطر_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_سطر_Characters = Column;
      Longest_All_Space_سطر = Current;
    }

    // Check for EOF.
    إذا (Current == End) {
      IsDone = true;
      أرجع true;
    }

    إذا (!consume_سطر_Break_If_Present()) {
      IsDone = true;
      أرجع true;
    }
    ++سطر_Breaks;
  }
  أرجع true;
}

منطقي Scanner::scanBlockScalarIndent(unsigned BlockIndent,
                                    unsigned BlockExitIndent, منطقي &IsDone) {
  // Skip the indentation.
  طالما (Column < BlockIndent) {
    auto I = skip_s_space(Current);
    إذا (I == Current)
      اكسر;
    Current = I;
    ++Column;
  }

  إذا (skip_nb_char(Current) == Current)
    أرجع true;

  إذا (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = true;
    أرجع true;
  }

  إذا (Column < BlockIndent) {
    إذا (Current != End && *Current == '#') { // Trailing comment.
      IsDone = true;
      أرجع true;
    }
    setError("A text line is less indented than the block scalar", Current);
    أرجع خطأ;
  }
  أرجع true; // A normal text line.
}

منطقي Scanner::scanBlockScalar(منطقي IsLiteral) {
  أكد(*Current == '|' || *Current == '>');
  حرف StyleIndicator;
  حرف ChompingIndicator;
  unsigned BlockIndent;
  منطقي IsDone = خطأ;
  إذا (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    أرجع خطأ;
  إذا (IsDone)
    أرجع true;
  منطقي IsFolded = StyleIndicator == '>';

  ثابت auto *Start = Current;
  unsigned BlockExitIndent = Indent < 0 ? 0 : (unsigned)Indent;
  unsigned سطر_Breaks = 0;
  إذا (BlockIndent == 0) {
    إذا (!findBlockScalarIndent(BlockIndent, BlockExitIndent, سطر_Breaks,
                               IsDone))
      أرجع خطأ;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  طالما (!IsDone) {
    إذا (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      أرجع خطأ;
    إذا (IsDone)
      اكسر;

    // Parse the current line.
    auto سطر_Start = Current;
    advanceWhile(&Scanner::skip_nb_char);
    إذا (سطر_Start != Current) {
      إذا (سطر_Breaks && IsFolded && !Scanner::is_سطر_Empty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        إذا (سطر_Breaks == 1) {
          Str.append(سطر_Breaks,
                     is_سطر_Empty(StringRef(سطر_Start, Current - سطر_Start))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        سطر_Breaks--;
      }
      Str.append(سطر_Breaks, '\n');
      Str.append(StringRef(سطر_Start, Current - سطر_Start));
      سطر_Breaks = 0;
    }

    // Check for EOF.
    إذا (Current == End)
      اكسر;

    إذا (!consume_سطر_Break_If_Present())
      اكسر;
    ++سطر_Breaks;
  }

  إذا (Current == End && !سطر_Breaks)
    // Ensure that there is at least one line break before the end of file.
    سطر_Breaks = 1;
  Str.append(get_Chomped_سطر_Breaks(ChompingIndicator, سطر_Breaks, Str), '\n');

  // New lines may start a simple key.
  إذا (!FlowLevel)
    Is_Simple_کلید_Allowed = true;
  Is_Adjacent_قدر_Allowed_In_Flow = خطأ;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.قدر = std::string(Str);
  TokenQueue.ڈالو_back(T);
  أرجع true;
}

منطقي Scanner::scanTag() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1); // Eat !.
  إذا (Current == End || isBlankOrBreak(Current)); // An empty tag.
  وإلا إذا (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    إذا (!consume('>'))
      أرجع خطأ;
  } وإلا {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.Kind = Token::TK_Tag;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ڈالو_back(T);

  // Tags can be simple keys.
  save_Simple_کلید_Candidate(--TokenQueue.end(), ColStart, خطأ);

  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_In_Flow = خطأ;

  أرجع true;
}

منطقي Scanner::fetchMoreTokens() {
  إذا (IsStartOfStream)
    أرجع scanStreamStart();

  scanToNextToken();

  إذا (Current == End)
    أرجع scanStreamEnd();

  remove_Stale_Simple_کلید_Candidates();

  unrollIndent(Column);

  إذا (Column == 0 && *Current == '%')
    أرجع scanDirective();

  إذا (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    أرجع scanDocumentIndicator(true);

  إذا (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    أرجع scanDocumentIndicator(خطأ);

  إذا (*Current == '[')
    أرجع scanFlowCollectionStart(true);

  إذا (*Current == '{')
    أرجع scanFlowCollectionStart(خطأ);

  إذا (*Current == ']')
    أرجع scanFlowCollectionEnd(true);

  إذا (*Current == '}')
    أرجع scanFlowCollectionEnd(خطأ);

  إذا (*Current == ',')
    أرجع scanFlowEntry();

  إذا (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    أرجع scanBlockEntry();

  إذا (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    أرجع scan_کلید();

  إذا (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || Is_Adjacent_قدر_Allowed_In_Flow))
    أرجع scan_قدر();

  إذا (*Current == '*')
    أرجع scanAliasOrAnchor(true);

  إذا (*Current == '&')
    أرجع scanAliasOrAnchor(خطأ);

  إذا (*Current == '!')
    أرجع scanTag();

  إذا (*Current == '|' && !FlowLevel)
    أرجع scanBlockScalar(true);

  إذا (*Current == '>' && !FlowLevel)
    أرجع scanBlockScalar(خطأ);

  إذا (*Current == '\'')
    أرجع scanFlowScalar(خطأ);

  إذا (*Current == '"')
    أرجع scanFlowScalar(true);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  إذا ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    أرجع scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  أرجع خطأ;
}

Stream::Stream(StringRef داخلہ, SourceMgr &SM, منطقي ShowColors,
               std::error_code *EC)
    : scanner(جديد Scanner(داخلہ, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef داخلہ_Buffer, SourceMgr &SM, منطقي ShowColors,
               std::error_code *EC)
    : scanner(جديد Scanner(داخلہ_Buffer, SM, ShowColors, EC)) {}

Stream::~Stream() = افتراضي;

منطقي Stream::failed() { أرجع scanner->failed(); }

فراغ Stream::printError(گرہ *N, ثابت Twine &پیغام_2, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), پیغام_2, Kind);
}

فراغ Stream::printError(ثابت SMRange &Range, ثابت Twine &پیغام_2,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, پیغام_2, Range);
}

document_iterator Stream::begin() {
  إذا (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(جديد Document(*هذا));
  أرجع document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  أرجع document_iterator();
}

فراغ Stream::skip() {
  لكل (Document &Doc : *هذا)
    Doc.skip();
}

گرہ::گرہ(unsigned صحيح Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string گرہ::getVerbatimTag() ثابت {
  StringRef Raw = getRawTag();
  إذا (!Raw.empty() && Raw != "!") {
    std::string Ret;
    إذا (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      أرجع Ret;
    } وإلا إذا (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      أرجع Ret;
    } وإلا {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      إذا (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      وإلا {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      أرجع Ret;
    }
  }

  بدل (getType()) {
  حالة NK_Null:
    أرجع "tag:yaml.org,2002:null";
  حالة NK_Scalar:
  حالة NK_BlockScalar:
    // TODO: Tag resolution.
    أرجع "tag:yaml.org,2002:str";
  حالة NK_Mapping:
    أرجع "tag:yaml.org,2002:map";
  حالة NK_Sequence:
    أرجع "tag:yaml.org,2002:seq";
  }

  أرجع "";
}

Token &گرہ::peekNext() {
  أرجع Doc->peekNext();
}

Token گرہ::getNext() {
  أرجع Doc->getNext();
}

گرہ *گرہ::parse_Block_گرہ() {
  أرجع Doc->parse_Block_گرہ();
}

BumpPtrAllocator &گرہ::getAllocator() {
  أرجع Doc->گرہ_Allocator;
}

فراغ گرہ::setError(ثابت Twine &پیغام_2, Token &Tok) ثابت {
  Doc->setError(پیغام_2, Tok);
}

منطقي گرہ::failed() ثابت {
  أرجع Doc->failed();
}

StringRef Scalar_گرہ::get_قدر(SmallVectorImpl<حرف> &Storage) ثابت {
  إذا (قدر[0] == '"')
    أرجع get_Double_Quoted_قدر(قدر, Storage);
  إذا (قدر[0] == '\'')
    أرجع get_Single_Quoted_قدر(قدر, Storage);
  أرجع get_Plain_قدر(قدر, Storage);
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
parse_Scalar_قدر(StringRef Unquoted_قدر, SmallVectorImpl<حرف> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<حرف> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_قدر.find_first_of(LookupChars);
  إذا (I == StringRef::npos)
    أرجع Unquoted_قدر;

  Storage.clear();
  Storage.reserve(Unquoted_قدر.size());
  حرف Last_New_سطر_Added_As = '\0';
  لكل (; I != StringRef::npos; I = Unquoted_قدر.find_first_of(LookupChars)) {
    إذا (Unquoted_قدر[I] != '\r' && Unquoted_قدر[I] != '\n') {
      llvm::append_range(Storage, Unquoted_قدر.take_front(I));
      Unquoted_قدر = UnescapeCallback(Unquoted_قدر.drop_front(I), Storage);
      Last_New_سطر_Added_As = '\0';
      استمر;
    }
    إذا (size_t LastNonSWhite = Unquoted_قدر.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_قدر.take_front(LastNonSWhite + 1));
      Storage.ڈالو_back(' ');
      Last_New_سطر_Added_As = ' ';
    } وإلا {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      بدل (Last_New_سطر_Added_As) {
      حالة ' ':
        أكد(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        Last_New_سطر_Added_As = '\n';
        اكسر;
      حالة '\n':
        أكد(!Storage.empty() && Storage.back() == '\n');
        Storage.ڈالو_back('\n');
        اكسر;
      افتراضي:
        Storage.ڈالو_back(' ');
        Last_New_سطر_Added_As = ' ';
        اكسر;
      }
    }
    // Handle Windows-style EOL
    إذا (Unquoted_قدر.substr(I, 2) == "\r\n")
      I++;
    Unquoted_قدر = Unquoted_قدر.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_قدر);
  أرجع StringRef(Storage.begin(), Storage.size());
}

StringRef
Scalar_گرہ::get_Double_Quoted_قدر(StringRef Raw_قدر,
                                 SmallVectorImpl<حرف> &Storage) ثابت {
  أكد(Raw_قدر.size() >= 2 && Raw_قدر.front() == '"' &&
         Raw_قدر.back() == '"');
  StringRef Unquoted_قدر = Raw_قدر.substr(1, Raw_قدر.size() - 2);

  auto UnescapeFunc = [هذا](StringRef Unquoted_قدر,
                             SmallVectorImpl<حرف> &Storage) {
    أكد(Unquoted_قدر.take_front(1) == "\\");
    إذا (Unquoted_قدر.size() == 1) {
      Token T;
      T.Range = Unquoted_قدر;
      setError("Unrecognized escape code", T);
      Storage.clear();
      أرجع StringRef();
    }
    Unquoted_قدر = Unquoted_قدر.drop_front(1);
    بدل (Unquoted_قدر[0]) {
    افتراضي: {
      Token T;
      T.Range = Unquoted_قدر.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      أرجع StringRef();
    }
    حالة '\r':
      // Shrink the Windows-style EOL.
      إذا (Unquoted_قدر.size() >= 2 && Unquoted_قدر[1] == '\n')
        Unquoted_قدر = Unquoted_قدر.drop_front(1);
      [[fallthrough]];
    حالة '\n':
      أرجع Unquoted_قدر.drop_front(1).ltrim(" \t");
    حالة '0':
      Storage.ڈالو_back(0x00);
      اكسر;
    حالة 'a':
      Storage.ڈالو_back(0x07);
      اكسر;
    حالة 'b':
      Storage.ڈالو_back(0x08);
      اكسر;
    حالة 't':
    حالة 0x09:
      Storage.ڈالو_back(0x09);
      اكسر;
    حالة 'n':
      Storage.ڈالو_back(0x0A);
      اكسر;
    حالة 'v':
      Storage.ڈالو_back(0x0B);
      اكسر;
    حالة 'f':
      Storage.ڈالو_back(0x0C);
      اكسر;
    حالة 'r':
      Storage.ڈالو_back(0x0D);
      اكسر;
    حالة 'e':
      Storage.ڈالو_back(0x1B);
      اكسر;
    حالة ' ':
      Storage.ڈالو_back(0x20);
      اكسر;
    حالة '"':
      Storage.ڈالو_back(0x22);
      اكسر;
    حالة '/':
      Storage.ڈالو_back(0x2F);
      اكسر;
    حالة '\\':
      Storage.ڈالو_back(0x5C);
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
      إذا (Unquoted_قدر.size() < 3)
        // TODO: Report error.
        اكسر;
      unsigned صحيح Unicode_Scalar_قدر;
      إذا (Unquoted_قدر.substr(1, 2).getAsInteger(16, Unicode_Scalar_قدر))
        // TODO: Report error.
        Unicode_Scalar_قدر = 0xFFFD;
      encodeUTF8(Unicode_Scalar_قدر, Storage);
      أرجع Unquoted_قدر.drop_front(3);
    }
    حالة 'u': {
      إذا (Unquoted_قدر.size() < 5)
        // TODO: Report error.
        اكسر;
      unsigned صحيح Unicode_Scalar_قدر;
      إذا (Unquoted_قدر.substr(1, 4).getAsInteger(16, Unicode_Scalar_قدر))
        // TODO: Report error.
        Unicode_Scalar_قدر = 0xFFFD;
      encodeUTF8(Unicode_Scalar_قدر, Storage);
      أرجع Unquoted_قدر.drop_front(5);
    }
    حالة 'U': {
      إذا (Unquoted_قدر.size() < 9)
        // TODO: Report error.
        اكسر;
      unsigned صحيح Unicode_Scalar_قدر;
      إذا (Unquoted_قدر.substr(1, 8).getAsInteger(16, Unicode_Scalar_قدر))
        // TODO: Report error.
        Unicode_Scalar_قدر = 0xFFFD;
      encodeUTF8(Unicode_Scalar_قدر, Storage);
      أرجع Unquoted_قدر.drop_front(9);
    }
    }
    أرجع Unquoted_قدر.drop_front(1);
  };

  أرجع parse_Scalar_قدر(Unquoted_قدر, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_گرہ::get_Single_Quoted_قدر(StringRef Raw_قدر,
                                           SmallVectorImpl<حرف> &Storage) {
  أكد(Raw_قدر.size() >= 2 && Raw_قدر.front() == '\'' &&
         Raw_قدر.back() == '\'');
  StringRef Unquoted_قدر = Raw_قدر.substr(1, Raw_قدر.size() - 2);

  auto UnescapeFunc = [](StringRef Unquoted_قدر,
                         SmallVectorImpl<حرف> &Storage) {
    أكد(Unquoted_قدر.take_front(2) == "''");
    Storage.ڈالو_back('\'');
    أرجع Unquoted_قدر.drop_front(2);
  };

  أرجع parse_Scalar_قدر(Unquoted_قدر, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_گرہ::get_Plain_قدر(StringRef Raw_قدر,
                                    SmallVectorImpl<حرف> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_قدر = Raw_قدر.rtrim("\r\n \t");
  أرجع parse_Scalar_قدر(Raw_قدر, Storage, "\r\n", nullptr);
}

گرہ *کلید_قدر_گرہ::get_کلید() {
  إذا (کلید)
    أرجع کلید;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    إذا (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_قدر
        || t.Kind == Token::TK_Error) {
      أرجع کلید = جديد (getAllocator()) Null_گرہ(Doc);
    }
    إذا (t.Kind == Token::TK_کلید)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  إذا (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_قدر) {
    أرجع کلید = جديد (getAllocator()) Null_گرہ(Doc);
  }

  // We've got a normal key.
  أرجع کلید = parse_Block_گرہ();
}

گرہ *کلید_قدر_گرہ::get_قدر() {
  إذا (قدر)
    أرجع قدر;

  إذا (گرہ* کلید = get_کلید())
    کلید->skip();
  وإلا {
    setError("Null key in Key Value.", peekNext());
    أرجع قدر = جديد (getAllocator()) Null_گرہ(Doc);
  }

  إذا (failed())
    أرجع قدر = جديد (getAllocator()) Null_گرہ(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    إذا (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_کلید
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      أرجع قدر = جديد (getAllocator()) Null_گرہ(Doc);
    }

    إذا (t.Kind != Token::TK_قدر) {
      setError("Unexpected token in Key Value.", t);
      أرجع قدر = جديد (getAllocator()) Null_گرہ(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  إذا (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_کلید) {
    أرجع قدر = جديد (getAllocator()) Null_گرہ(Doc);
  }

  // We got a normal value.
  أرجع قدر = parse_Block_گرہ();
}

فراغ Mapping_گرہ::increment() {
  إذا (failed()) {
    IsAtEnd = true;
    CurrentEntry = nullptr;
    أرجع;
  }
  إذا (CurrentEntry) {
    CurrentEntry->skip();
    إذا (Type == MT_Inline) {
      IsAtEnd = true;
      CurrentEntry = nullptr;
      أرجع;
    }
  }
  Token T = peekNext();
  إذا (T.Kind == Token::TK_کلید || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = جديد (getAllocator()) کلید_قدر_گرہ(Doc);
  } وإلا إذا (Type == MT_Block) {
    بدل (T.Kind) {
    حالة Token::TK_BlockEnd:
      getNext();
      IsAtEnd = true;
      CurrentEntry = nullptr;
      اكسر;
    افتراضي:
      setError("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    حالة Token::TK_Error:
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  } وإلا {
    بدل (T.Kind) {
    حالة Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      أرجع increment();
    حالة Token::TK_FlowMappingEnd:
      getNext();
      [[fallthrough]];
    حالة Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = true;
      CurrentEntry = nullptr;
      اكسر;
    افتراضي:
      setError( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  }
}

فراغ Sequence_گرہ::increment() {
  إذا (failed()) {
    IsAtEnd = true;
    CurrentEntry = nullptr;
    أرجع;
  }
  إذا (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  إذا (SeqType == ST_Block) {
    بدل (T.Kind) {
    حالة Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_گرہ();
      إذا (!CurrentEntry) { // An error occurred.
        IsAtEnd = true;
        CurrentEntry = nullptr;
      }
      اكسر;
    حالة Token::TK_BlockEnd:
      getNext();
      IsAtEnd = true;
      CurrentEntry = nullptr;
      اكسر;
    افتراضي:
      setError( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    حالة Token::TK_Error:
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  } وإلا إذا (SeqType == ST_Indentless) {
    بدل (T.Kind) {
    حالة Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_گرہ();
      إذا (!CurrentEntry) { // An error occurred.
        IsAtEnd = true;
        CurrentEntry = nullptr;
      }
      اكسر;
    افتراضي:
    حالة Token::TK_Error:
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  } وإلا إذا (SeqType == ST_Flow) {
    بدل (T.Kind) {
    حالة Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      WasPreviousTokenFlowEntry = true;
      أرجع increment();
    حالة Token::TK_FlowSequenceEnd:
      getNext();
      [[fallthrough]];
    حالة Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = true;
      CurrentEntry = nullptr;
      اكسر;
    حالة Token::TK_StreamEnd:
    حالة Token::TK_DocumentEnd:
    حالة Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = true;
      CurrentEntry = nullptr;
      اكسر;
    افتراضي:
      إذا (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = true;
        CurrentEntry = nullptr;
        اكسر;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parse_Block_گرہ();
      إذا (!CurrentEntry) {
        IsAtEnd = true;
      }
      WasPreviousTokenFlowEntry = خطأ;
      اكسر;
    }
  }
}

Document::Document(Stream &S) : stream(S), جڑ_2(nullptr) {
  // Tag maps starts with two default mappings.
  TagMap["!"] = "!";
  TagMap["!!"] = "tag:yaml.org,2002:";

  إذا (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peekNext();
  إذا (T.Kind == Token::TK_DocumentStart)
    getNext();
}

منطقي Document::skip()  {
  إذا (stream.scanner->failed())
    أرجع خطأ;
  إذا (!جڑ_2 && !get_جڑ())
    أرجع خطأ;
  جڑ_2->skip();
  Token &T = peekNext();
  إذا (T.Kind == Token::TK_StreamEnd)
    أرجع خطأ;
  إذا (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    أرجع skip();
  }
  أرجع true;
}

Token &Document::peekNext() {
  أرجع stream.scanner->peekNext();
}

Token Document::getNext() {
  أرجع stream.scanner->getNext();
}

فراغ Document::setError(ثابت Twine &Message, Token &Location) ثابت {
  stream.scanner->setError(Message, Location.Range.begin());
}

منطقي Document::failed() ثابت {
  أرجع stream.scanner->failed();
}

گرہ *Document::parse_Block_گرہ() {
  Token T = peekNext();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  بدل (T.Kind) {
  حالة Token::TK_Alias:
    getNext();
    أرجع جديد (گرہ_Allocator) Alias_گرہ(stream.CurrentDoc, T.Range.substr(1));
  حالة Token::TK_Anchor:
    إذا (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      أرجع nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    اذهب parse_property;
  حالة Token::TK_Tag:
    إذا (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      أرجع nullptr;
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
    أرجع جديد (گرہ_Allocator) Sequence_گرہ( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , Sequence_گرہ::ST_Indentless);
  حالة Token::TK_BlockSequenceStart:
    getNext();
    أرجع جديد (گرہ_Allocator)
      Sequence_گرہ( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_گرہ::ST_Block);
  حالة Token::TK_BlockMappingStart:
    getNext();
    أرجع جديد (گرہ_Allocator)
      Mapping_گرہ( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_گرہ::MT_Block);
  حالة Token::TK_FlowSequenceStart:
    getNext();
    أرجع جديد (گرہ_Allocator)
      Sequence_گرہ( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_گرہ::ST_Flow);
  حالة Token::TK_FlowMappingStart:
    getNext();
    أرجع جديد (گرہ_Allocator)
      Mapping_گرہ( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_گرہ::MT_Flow);
  حالة Token::TK_Scalar:
    getNext();
    أرجع جديد (گرہ_Allocator)
      Scalar_گرہ( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  حالة Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.قدر.c_str(), T.قدر.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(گرہ_Allocator).drop_back();
    أرجع جديد (گرہ_Allocator)
        Block_Scalar_گرہ(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  حالة Token::TK_کلید:
    // Don't eat the TK_Key, KeyValueNode expects it.
    أرجع جديد (گرہ_Allocator)
      Mapping_گرہ( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_گرہ::MT_Inline);
  حالة Token::TK_DocumentStart:
  حالة Token::TK_DocumentEnd:
  حالة Token::TK_StreamEnd:
  افتراضي:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    أرجع جديد (گرہ_Allocator) Null_گرہ(stream.CurrentDoc);
  حالة Token::TK_FlowMappingEnd:
  حالة Token::TK_FlowSequenceEnd:
  حالة Token::TK_FlowEntry: {
    إذا (جڑ_2 && (isa<Mapping_گرہ>(جڑ_2) || isa<Sequence_گرہ>(جڑ_2)))
      أرجع جديد (گرہ_Allocator) Null_گرہ(stream.CurrentDoc);

    setError("Unexpected token", T);
    أرجع nullptr;
  }
  حالة Token::TK_Error:
    أرجع nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  أرجع nullptr;
}

منطقي Document::parseDirectives() {
  منطقي isDirective = خطأ;
  طالما (true) {
    Token T = peekNext();
    إذا (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = true;
    } وإلا إذا (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = true;
    } وإلا {
      اكسر;
    }
  }
  أرجع isDirective;
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
  إذا (T.Kind != TK) {
    setError("Unexpected token", T);
    أرجع خطأ;
  }
  أرجع true;
}
