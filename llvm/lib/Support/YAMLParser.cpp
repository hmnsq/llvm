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

#شامل "llvm/Support/YAMLParser.h"
#شامل "llvm/ADT/AllocatorList.h"
#شامل "llvm/ADT/ArrayRef.h"
#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/ADT/Twine.h"
#شامل "llvm/Support/Compiler.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/SMLoc.h"
#شامل "llvm/Support/SourceMgr.h"
#شامل "llvm/Support/Unicode.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cassert>
#شامل <cstddef>
#شامل <cstdint>
#شامل <map>
#شامل <memory>
#شامل <string>
#شامل <system_error>
#شامل <utility>

using فضاینام llvm;
using فضاینام yaml;

شمارش UnicodeEncodingForm {
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
ایستا EncodingInfo getUnicodeEncoding(StringRef درون_داد) {
  اگر (درون_داد.empty())
    بازگشت {UEF_Unknown, 0};

  انتخاب (uint8_t(درون_داد[0])) {
  حالت 0x00:
    اگر (درون_داد.size() >= 4) {
      اگر (  درون_داد[1] == 0
         && uint8_t(درون_داد[2]) == 0xFE
         && uint8_t(درون_داد[3]) == 0xFF)
        بازگشت {UEF_UTF32_BE, 4};
      اگر (درون_داد[1] == 0 && درون_داد[2] == 0 && درون_داد[3] != 0)
        بازگشت {UEF_UTF32_BE, 0};
    }

    اگر (درون_داد.size() >= 2 && درون_داد[1] != 0)
      بازگشت {UEF_UTF16_BE, 0};
    بازگشت {UEF_Unknown, 0};
  حالت 0xFF:
    اگر (  درون_داد.size() >= 4
       && uint8_t(درون_داد[1]) == 0xFE
       && درون_داد[2] == 0
       && درون_داد[3] == 0)
      بازگشت {UEF_UTF32_LE, 4};

    اگر (درون_داد.size() >= 2 && uint8_t(درون_داد[1]) == 0xFE)
      بازگشت {UEF_UTF16_LE, 2};
    بازگشت {UEF_Unknown, 0};
  حالت 0xFE:
    اگر (درون_داد.size() >= 2 && uint8_t(درون_داد[1]) == 0xFF)
      بازگشت {UEF_UTF16_BE, 2};
    بازگشت {UEF_Unknown, 0};
  حالت 0xEF:
    اگر (  درون_داد.size() >= 3
       && uint8_t(درون_داد[1]) == 0xBB
       && uint8_t(درون_داد[2]) == 0xBF)
      بازگشت {UEF_UTF8, 3};
    بازگشت {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  اگر (درون_داد.size() >= 4 && درون_داد[1] == 0 && درون_داد[2] == 0 && درون_داد[3] == 0)
    بازگشت {UEF_UTF32_LE, 0};

  اگر (درون_داد.size() >= 2 && درون_داد[1] == 0)
    بازگشت {UEF_UTF16_LE, 0};

  بازگشت {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
تهی گره::anchor() {}
تهی Null_گره::anchor() {}
تهی Scalar_گره::anchor() {}
تهی Block_Scalar_گره::anchor() {}
تهی کلید_مقدار_گره::anchor() {}
تهی Mapping_گره::anchor() {}
تهی Sequence_گره::anchor() {}
تهی Alias_گره::anchor() {}

فضاینام llvm {
فضاینام yaml {

/// Token - A single YAML token.
ساختار Token {
  شمارش TokenKind {
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
    TK_مقدار,
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
  std::string مقدار;

  Token() = پیشفرض;
};

} // end namespace yaml
} // end namespace llvm

using TokenQueueT = BumpPtrList<Token>;

فضاینام {

/// This struct is used to track simple keys.
///
/// Simple keys are handled by creating an entry in SimpleKeys for each Token
/// which could legally be the start of a simple key. When peekNext is called,
/// if the Token To be returned is referenced by a SimpleKey, we continue
/// tokenizing until that potential simple key has either been found to not be
/// a simple key (we moved on to the next line or went further than 1024 chars).
/// Or when we run into a Value, and then insert a Key token (and possibly
/// others) before the SimpleKey's Tok.
ساختار Simple_کلید {
  TokenQueueT::iterator Tok;
  unsigned Column = 0;
  unsigned سطر = 0;
  unsigned FlowLevel = 0;
  منطقی IsRequired = نادرست;

  منطقی operator ==(ثابت Simple_کلید &Other) {
    بازگشت Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
using UTF8Decoded = std::pair<uint32_t, unsigned>;

ایستا UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator End = Range.end();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  اگر (Position < End && (*Position & 0x80) == 0) {
    بازگشت {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  اگر (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    اگر (codepoint >= 0x80)
      بازگشت {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  اگر (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    اگر (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      بازگشت {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  اگر (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    اگر (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      بازگشت {codepoint, 4};
  }
  بازگشت {0, 0};
}

فضاینام llvm {
فضاینام yaml {

/// Scans YAML tokens from a MemoryBuffer.
رده Scanner {
عمومی:
  Scanner(StringRef درون_داد, SourceMgr &SM, منطقی ShowColors = درست,
          std::error_code *EC = nullptr);
  Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, منطقی ShowColors = درست,
          std::error_code *EC = nullptr);

  /// Parse the next token and return it without popping it.
  Token &peekNext();

  /// Parse the next token and pop it from the queue.
  Token getNext();

  تهی printError(SMLoc Loc, SourceMgr::DiagKind Kind, ثابت Twine &Message,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.PrintMessage(Loc, Kind, Message, Ranges, /* FixIts= */ {}, ShowColors);
  }

  تهی setError(ثابت Twine &Message, StringRef::iterator Position) {
    اگر (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    اگر (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    اگر (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = درست;
  }

  /// Returns true if an error occurred while parsing.
  منطقی failed() {
    بازگشت Failed;
  }

خصوصی:
  تهی init(MemoryBufferRef Buffer);

  StringRef current_درون_داد() {
    بازگشت StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    بازگشت ::decodeUTF8(StringRef(Position, End - Position));
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
  تهی advanceWhile(SkipWhileFunc Func);

  /// Scan ns-uri-char[39]s starting at Cur.
  ///
  /// This updates Cur and Column while scanning.
  تهی scan_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  منطقی consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  تهی skip(uint32_t Distance);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  منطقی isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  منطقی isPlainSafeNonBlank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  منطقی is_سطر_Empty(StringRef سطر);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  منطقی consume_سطر_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  تهی save_Simple_کلید_Candidate( TokenQueueT::iterator Tok
                             , unsigned AtColumn
                             , منطقی IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  تهی remove_Stale_Simple_کلید_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  تهی remove_Simple_کلید_Candidates_On_Flow_Level(unsigned Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  منطقی unrollIndent(صحیح ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  منطقی rollIndent( صحیح ToColumn
                 , Token::TokenKind Kind
                 , TokenQueueT::iterator درج_Point);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  تهی skipComment();

  /// Skip whitespace and comments until the start of the next token.
  تهی scanToNextToken();

  /// Must be the first token generated.
  منطقی scanStreamStart();

  /// Generate tokens needed to close out the stream.
  منطقی scanStreamEnd();

  /// Scan a %BLAH directive.
  منطقی scanDirective();

  /// Scan a ... or ---.
  منطقی scanDocumentIndicator(منطقی IsStart);

  /// Scan a [ or { and generate the proper flow collection start token.
  منطقی scanFlowCollectionStart(منطقی IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  منطقی scanFlowCollectionEnd(منطقی IsSequence);

  /// Scan the , that separates entries in a flow collection.
  منطقی scanFlowEntry();

  /// Scan the - that starts block sequence entries.
  منطقی scanBlockEntry();

  /// Scan an explicit ? indicating a key.
  منطقی scan_کلید();

  /// Scan an explicit : indicating a value.
  منطقی scan_مقدار();

  /// Scan a quoted scalar.
  منطقی scanFlowScalar(منطقی IsDoubleQuoted);

  /// Scan an unquoted scalar.
  منطقی scanPlainScalar();

  /// Scan an Alias or Anchor starting with * or &.
  منطقی scanAliasOrAnchor(منطقی IsAlias);

  /// Scan a block scalar starting with | or >.
  منطقی scanBlockScalar(منطقی IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  منطقی scanBlockScalarIndicators(نویسه &StyleIndicator, نویسه &ChompingIndicator,
                                 unsigned &IndentIndicator, منطقی &IsDone);

  /// Scan a style indicator in a block scalar header.
  نویسه scanBlockStyleIndicator();

  /// Scan a chomping indicator in a block scalar header.
  نویسه scanBlockChompingIndicator();

  /// Scan an indentation indicator in a block scalar header.
  unsigned scanBlockIndentationIndicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  منطقی scanBlockScalarHeader(نویسه &ChompingIndicator, unsigned &IndentIndicator,
                             منطقی &IsDone);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  منطقی findBlockScalarIndent(unsigned &BlockIndent, unsigned BlockExitIndent,
                             unsigned &سطر_Breaks, منطقی &IsDone);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  منطقی scanBlockScalarIndent(unsigned BlockIndent, unsigned BlockExitIndent,
                             منطقی &IsDone);

  /// Scan a tag of the form !stuff.
  منطقی scanTag();

  /// Dispatch to the next scanning function based on \a *Cur.
  منطقی fetchMoreTokens();

  /// The SourceMgr used for diagnostics and buffer management.
  SourceMgr &SM;

  /// The original input.
  MemoryBufferRef درون_داد_Buffer;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator End;

  /// Current YAML indentation level in spaces.
  صحیح Indent;

  /// Current column number in Unicode code points.
  unsigned Column;

  /// Current line number.
  unsigned سطر;

  /// How deep we are in flow style containers. 0 Means at block level.
  unsigned FlowLevel;

  /// Are we at the start of the stream?
  منطقی IsStartOfStream;

  /// Can the next token be the start of a simple key?
  منطقی Is_Simple_کلید_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  منطقی Is_Adjacent_مقدار_Allowed_In_Flow;

  /// True if an error has occurred.
  منطقی Failed;

  /// Should colors be used when printing out the diagnostic messages?
  منطقی ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  TokenQueueT TokenQueue;

  /// Indentation levels.
  SmallVector<صحیح, 4> Indents;

  /// Potential simple keys.
  SmallVector<Simple_کلید, 4> Simple_کلیدها;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
ایستا تهی encodeUTF8( uint32_t Unicode_Scalar_مقدار
                      , SmallVectorImpl<نویسه> &Result) {
  اگر (Unicode_Scalar_مقدار <= 0x7F) {
    Result.افزودن_back(Unicode_Scalar_مقدار & 0x7F);
  } وگرنه اگر (Unicode_Scalar_مقدار <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((Unicode_Scalar_مقدار & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Unicode_Scalar_مقدار & 0x3F);
    Result.افزودن_back(FirstByte);
    Result.افزودن_back(SecondByte);
  } وگرنه اگر (Unicode_Scalar_مقدار <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((Unicode_Scalar_مقدار & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_مقدار & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Unicode_Scalar_مقدار & 0x3F);
    Result.افزودن_back(FirstByte);
    Result.افزودن_back(SecondByte);
    Result.افزودن_back(ThirdByte);
  } وگرنه اگر (Unicode_Scalar_مقدار <= 0x10FFFF) {
    uint8_t FirstByte = 0xF0 | ((Unicode_Scalar_مقدار & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_مقدار & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Unicode_Scalar_مقدار & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_مقدار & 0x3F);
    Result.افزودن_back(FirstByte);
    Result.افزودن_back(SecondByte);
    Result.افزودن_back(ThirdByte);
    Result.افزودن_back(FourthByte);
  }
}

منطقی yaml::dumpTokens(StringRef درون_داد, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(درون_داد, SM);
  وقتی (درست) {
    Token T = scanner.getNext();
    انتخاب (T.Kind) {
    حالت Token::TK_StreamStart:
      OS << "Stream-Start: ";
      بشکن;
    حالت Token::TK_StreamEnd:
      OS << "Stream-End: ";
      بشکن;
    حالت Token::TK_VersionDirective:
      OS << "Version-Directive: ";
      بشکن;
    حالت Token::TK_TagDirective:
      OS << "Tag-Directive: ";
      بشکن;
    حالت Token::TK_DocumentStart:
      OS << "Document-Start: ";
      بشکن;
    حالت Token::TK_DocumentEnd:
      OS << "Document-End: ";
      بشکن;
    حالت Token::TK_BlockEntry:
      OS << "Block-Entry: ";
      بشکن;
    حالت Token::TK_BlockEnd:
      OS << "Block-End: ";
      بشکن;
    حالت Token::TK_BlockSequenceStart:
      OS << "Block-Sequence-Start: ";
      بشکن;
    حالت Token::TK_BlockMappingStart:
      OS << "Block-Mapping-Start: ";
      بشکن;
    حالت Token::TK_FlowEntry:
      OS << "Flow-Entry: ";
      بشکن;
    حالت Token::TK_FlowSequenceStart:
      OS << "Flow-Sequence-Start: ";
      بشکن;
    حالت Token::TK_FlowSequenceEnd:
      OS << "Flow-Sequence-End: ";
      بشکن;
    حالت Token::TK_FlowMappingStart:
      OS << "Flow-Mapping-Start: ";
      بشکن;
    حالت Token::TK_FlowMappingEnd:
      OS << "Flow-Mapping-End: ";
      بشکن;
    حالت Token::TK_کلید:
      OS << "Key: ";
      بشکن;
    حالت Token::TK_مقدار:
      OS << "Value: ";
      بشکن;
    حالت Token::TK_Scalar:
      OS << "Scalar: ";
      بشکن;
    حالت Token::TK_BlockScalar:
      OS << "Block Scalar: ";
      بشکن;
    حالت Token::TK_Alias:
      OS << "Alias: ";
      بشکن;
    حالت Token::TK_Anchor:
      OS << "Anchor: ";
      بشکن;
    حالت Token::TK_Tag:
      OS << "Tag: ";
      بشکن;
    حالت Token::TK_Error:
      بشکن;
    }
    OS << T.Range << "\n";
    اگر (T.Kind == Token::TK_StreamEnd)
      بشکن;
    وگرنه اگر (T.Kind == Token::TK_Error)
      بازگشت نادرست;
  }
  بازگشت درست;
}

منطقی yaml::scanTokens(StringRef درون_داد) {
  SourceMgr SM;
  Scanner scanner(درون_داد, SM);
  وقتی (درست) {
    Token T = scanner.getNext();
    اگر (T.Kind == Token::TK_StreamEnd)
      بشکن;
    وگرنه اگر (T.Kind == Token::TK_Error)
      بازگشت نادرست;
  }
  بازگشت درست;
}

std::string yaml::escape(StringRef درون_داد, منطقی EscapePrintable) {
  std::string Escaped_درون_داد;
  برای (StringRef::iterator i = درون_داد.begin(), e = درون_داد.end(); i != e; ++i) {
    اگر (*i == '\\')
      Escaped_درون_داد += "\\\\";
    وگرنه اگر (*i == '"')
      Escaped_درون_داد += "\\\"";
    وگرنه اگر (*i == 0)
      Escaped_درون_داد += "\\0";
    وگرنه اگر (*i == 0x07)
      Escaped_درون_داد += "\\a";
    وگرنه اگر (*i == 0x08)
      Escaped_درون_داد += "\\b";
    وگرنه اگر (*i == 0x09)
      Escaped_درون_داد += "\\t";
    وگرنه اگر (*i == 0x0A)
      Escaped_درون_داد += "\\n";
    وگرنه اگر (*i == 0x0B)
      Escaped_درون_داد += "\\v";
    وگرنه اگر (*i == 0x0C)
      Escaped_درون_داد += "\\f";
    وگرنه اگر (*i == 0x0D)
      Escaped_درون_داد += "\\r";
    وگرنه اگر (*i == 0x1B)
      Escaped_درون_داد += "\\e";
    وگرنه اگر ((unsigned نویسه)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_درون_داد += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } وگرنه اگر (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_مقدار
        = decodeUTF8(StringRef(i, درون_داد.end() - i));
      اگر (Unicode_Scalar_مقدار.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_درون_داد, Val);
        // FIXME: Error reporting.
        بازگشت Escaped_درون_داد;
      }
      اگر (Unicode_Scalar_مقدار.first == 0x85)
        Escaped_درون_داد += "\\N";
      وگرنه اگر (Unicode_Scalar_مقدار.first == 0xA0)
        Escaped_درون_داد += "\\_";
      وگرنه اگر (Unicode_Scalar_مقدار.first == 0x2028)
        Escaped_درون_داد += "\\L";
      وگرنه اگر (Unicode_Scalar_مقدار.first == 0x2029)
        Escaped_درون_داد += "\\P";
      وگرنه اگر (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_مقدار.first))
        Escaped_درون_داد += StringRef(i, Unicode_Scalar_مقدار.second);
      وگرنه {
        std::string HexStr = utohexstr(Unicode_Scalar_مقدار.first);
        اگر (HexStr.size() <= 2)
          Escaped_درون_داد += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        وگرنه اگر (HexStr.size() <= 4)
          Escaped_درون_داد += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        وگرنه اگر (HexStr.size() <= 8)
          Escaped_درون_داد += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += Unicode_Scalar_مقدار.second - 1;
    } وگرنه {
      Escaped_درون_داد.افزودن_back(*i);
    }
  }
  بازگشت Escaped_درون_داد;
}

std::optional<منطقی> yaml::parseBool(StringRef S) {
  انتخاب (S.size()) {
  حالت 1:
    انتخاب (S.front()) {
    حالت 'y':
    حالت 'Y':
      بازگشت درست;
    حالت 'n':
    حالت 'N':
      بازگشت نادرست;
    پیشفرض:
      بازگشت std::nullopt;
    }
  حالت 2:
    انتخاب (S.front()) {
    حالت 'O':
      اگر (S[1] == 'N') // ON
        بازگشت درست;
      [[fallthrough]];
    حالت 'o':
      اگر (S[1] == 'n') //[Oo]n
        بازگشت درست;
      بازگشت std::nullopt;
    حالت 'N':
      اگر (S[1] == 'O') // NO
        بازگشت نادرست;
      [[fallthrough]];
    حالت 'n':
      اگر (S[1] == 'o') //[Nn]o
        بازگشت نادرست;
      بازگشت std::nullopt;
    پیشفرض:
      بازگشت std::nullopt;
    }
  حالت 3:
    انتخاب (S.front()) {
    حالت 'O':
      اگر (S.drop_front() == "FF") // OFF
        بازگشت نادرست;
      [[fallthrough]];
    حالت 'o':
      اگر (S.drop_front() == "ff") //[Oo]ff
        بازگشت نادرست;
      بازگشت std::nullopt;
    حالت 'Y':
      اگر (S.drop_front() == "ES") // YES
        بازگشت درست;
      [[fallthrough]];
    حالت 'y':
      اگر (S.drop_front() == "es") //[Yy]es
        بازگشت درست;
      بازگشت std::nullopt;
    پیشفرض:
      بازگشت std::nullopt;
    }
  حالت 4:
    انتخاب (S.front()) {
    حالت 'T':
      اگر (S.drop_front() == "RUE") // TRUE
        بازگشت درست;
      [[fallthrough]];
    حالت 't':
      اگر (S.drop_front() == "rue") //[Tt]rue
        بازگشت درست;
      بازگشت std::nullopt;
    پیشفرض:
      بازگشت std::nullopt;
    }
  حالت 5:
    انتخاب (S.front()) {
    حالت 'F':
      اگر (S.drop_front() == "ALSE") // FALSE
        بازگشت نادرست;
      [[fallthrough]];
    حالت 'f':
      اگر (S.drop_front() == "alse") //[Ff]alse
        بازگشت نادرست;
      بازگشت std::nullopt;
    پیشفرض:
      بازگشت std::nullopt;
    }
  پیشفرض:
    بازگشت std::nullopt;
  }
}

Scanner::Scanner(StringRef درون_داد, SourceMgr &sm, منطقی ShowColors,
                 std::error_code *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(MemoryBufferRef(درون_داد, "YAML"));
}

Scanner::Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, منطقی ShowColors,
                 std::error_code *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(Buffer);
}

تهی Scanner::init(MemoryBufferRef Buffer) {
  درون_داد_Buffer = Buffer;
  Current = درون_داد_Buffer.getBufferStart();
  End = درون_داد_Buffer.getBufferEnd();
  Indent = -1;
  Column = 0;
  سطر = 0;
  FlowLevel = 0;
  IsStartOfStream = درست;
  Is_Simple_کلید_Allowed = درست;
  Is_Adjacent_مقدار_Allowed_In_Flow = نادرست;
  Failed = نادرست;
  std::unique_ptr<MemoryBuffer> درون_داد_Buffer_Owner =
      MemoryBuffer::getMemBuffer(Buffer, /*RequiresNullTerminator=*/نادرست);
  SM.AddNewSourceBuffer(std::move(درون_داد_Buffer_Owner), SMLoc());
}

Token &Scanner::peekNext() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  منطقی NeedMore = نادرست;
  وقتی (درست) {
    اگر (TokenQueue.empty() || NeedMore) {
      اگر (!fetchMoreTokens()) {
        TokenQueue.clear();
        Simple_کلیدها.clear();
        TokenQueue.افزودن_back(Token());
        بازگشت TokenQueue.front();
      }
    }
    أكد(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_کلید_Candidates();
    Simple_کلید SK;
    SK.Tok = TokenQueue.begin();
    اگر (!is_contained(Simple_کلیدها, SK))
      بشکن;
    وگرنه
      NeedMore = درست;
  }
  بازگشت TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  اگر (!TokenQueue.empty())
    TokenQueue.برداشت_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  اگر (TokenQueue.empty())
    TokenQueue.resetAlloc();

  بازگشت Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  اگر (Position == End)
    بازگشت Position;
  // Check 7 bit c-printable - b-char.
  اگر (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    بازگشت Position + 1;

  // Check for valid UTF-8.
  اگر (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    اگر (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      بازگشت Position + u8d.second;
  }
  بازگشت Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  اگر (Position == End)
    بازگشت Position;
  اگر (*Position == 0x0D) {
    اگر (Position + 1 != End && *(Position + 1) == 0x0A)
      بازگشت Position + 2;
    بازگشت Position + 1;
  }

  اگر (*Position == 0x0A)
    بازگشت Position + 1;
  بازگشت Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  اگر (Position == End)
    بازگشت Position;
  اگر (*Position == ' ')
    بازگشت Position + 1;
  بازگشت Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  اگر (Position == End)
    بازگشت Position;
  اگر (*Position == ' ' || *Position == '\t')
    بازگشت Position + 1;
  بازگشت Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  اگر (Position == End)
    بازگشت Position;
  اگر (*Position == ' ' || *Position == '\t')
    بازگشت Position;
  بازگشت skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  وقتی (درست) {
    StringRef::iterator i = (این->*Func)(Position);
    اگر (i == Position)
      بشکن;
    Position = i;
  }
  بازگشت Position;
}

تهی Scanner::advanceWhile(SkipWhileFunc Func) {
  auto Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

ایستا منطقی is_ns_hex_digit(ثابت نویسه C) { بازگشت isAlnum(C); }

ایستا منطقی is_ns_word_char(ثابت نویسه C) { بازگشت C == '-' || isAlpha(C); }

تهی Scanner::scan_ns_uri_char() {
  وقتی (درست) {
    اگر (Current == End)
      بشکن;
    اگر ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } وگرنه {
      بشکن;
    }
  }
}

منطقی Scanner::consume(uint32_t Expected) {
  اگر (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    بازگشت نادرست;
  }
  اگر (Current == End)
    بازگشت نادرست;
  اگر (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    بازگشت نادرست;
  }
  اگر (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    بازگشت درست;
  }
  بازگشت نادرست;
}

تهی Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  أكد(Current <= End && "Skipped past the end");
}

منطقی Scanner::isBlankOrBreak(StringRef::iterator Position) {
  اگر (Position == End)
    بازگشت نادرست;
  بازگشت *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

منطقی Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  اگر (Position == End || isBlankOrBreak(Position))
    بازگشت نادرست;
  اگر (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    بازگشت نادرست;
  بازگشت درست;
}

منطقی Scanner::is_سطر_Empty(StringRef سطر) {
  برای (ثابت auto *Position = سطر.begin(); Position != سطر.end(); ++Position)
    اگر (!isBlankOrBreak(Position))
      بازگشت نادرست;
  بازگشت درست;
}

منطقی Scanner::consume_سطر_Break_If_Present() {
  auto Next = skip_b_break(Current);
  اگر (Next == Current)
    بازگشت نادرست;
  Column = 0;
  ++سطر;
  Current = Next;
  بازگشت درست;
}

تهی Scanner::save_Simple_کلید_Candidate( TokenQueueT::iterator Tok
                                    , unsigned AtColumn
                                    , منطقی IsRequired) {
  اگر (Is_Simple_کلید_Allowed) {
    Simple_کلید SK;
    SK.Tok = Tok;
    SK.سطر = سطر;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_کلیدها.افزودن_back(SK);
  }
}

تهی Scanner::remove_Stale_Simple_کلید_Candidates() {
  برای (SmallVectorImpl<Simple_کلید>::iterator i = Simple_کلیدها.begin();
                                            i != Simple_کلیدها.end();) {
    اگر (i->سطر != سطر || i->Column + 1024 < Column) {
      اگر (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_کلیدها.erase(i);
    } وگرنه {
      ++i;
    }
  }
}

تهی Scanner::remove_Simple_کلید_Candidates_On_Flow_Level(unsigned Level) {
  اگر (!Simple_کلیدها.empty() && (Simple_کلیدها.end() - 1)->FlowLevel == Level)
    Simple_کلیدها.برداشت_back();
}

منطقی Scanner::unrollIndent(صحیح ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  اگر (FlowLevel != 0)
    بازگشت درست;

  وقتی (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.افزودن_back(T);
    Indent = Indents.برداشت_back_val();
  }

  بازگشت درست;
}

منطقی Scanner::rollIndent( صحیح ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator درج_Point) {
  اگر (FlowLevel)
    بازگشت درست;
  اگر (Indent < ToColumn) {
    Indents.افزودن_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.درج(درج_Point, T);
  }
  بازگشت درست;
}

تهی Scanner::skipComment() {
  اگر (Current == End || *Current != '#')
    بازگشت;
  وقتی (درست) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    اگر (I == Current)
      بشکن;
    Current = I;
    ++Column;
  }
}

تهی Scanner::scanToNextToken() {
  وقتی (درست) {
    وقتی (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    اگر (i == Current)
      بشکن;
    Current = i;
    ++سطر;
    Column = 0;
    // New lines may start a simple key.
    اگر (!FlowLevel)
      Is_Simple_کلید_Allowed = درست;
  }
}

منطقی Scanner::scanStreamStart() {
  IsStartOfStream = نادرست;

  EncodingInfo EI = getUnicodeEncoding(current_درون_داد());

  Token T;
  T.Kind = Token::TK_StreamStart;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.افزودن_back(T);
  Current += EI.second;
  بازگشت درست;
}

منطقی Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  اگر (Column != 0) {
    Column = 0;
    ++سطر;
  }

  unrollIndent(-1);
  Simple_کلیدها.clear();
  Is_Simple_کلید_Allowed = نادرست;
  Is_Adjacent_مقدار_Allowed_In_Flow = نادرست;

  Token T;
  T.Kind = Token::TK_StreamEnd;
  T.Range = StringRef(Current, 0);
  TokenQueue.افزودن_back(T);
  بازگشت درست;
}

منطقی Scanner::scanDirective() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_کلیدها.clear();
  Is_Simple_کلید_Allowed = نادرست;
  Is_Adjacent_مقدار_Allowed_In_Flow = نادرست;

  StringRef::iterator Start = Current;
  consume('%');
  StringRef::iterator NameStart = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(NameStart, Current - NameStart);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  اگر (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.افزودن_back(T);
    بازگشت درست;
  } وگرنه اگر(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.افزودن_back(T);
    بازگشت درست;
  }
  بازگشت نادرست;
}

منطقی Scanner::scanDocumentIndicator(منطقی IsStart) {
  unrollIndent(-1);
  Simple_کلیدها.clear();
  Is_Simple_کلید_Allowed = نادرست;
  Is_Adjacent_مقدار_Allowed_In_Flow = نادرست;

  Token T;
  T.Kind = IsStart ? Token::TK_DocumentStart : Token::TK_DocumentEnd;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.افزودن_back(T);
  بازگشت درست;
}

منطقی Scanner::scanFlowCollectionStart(منطقی IsSequence) {
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.افزودن_back(T);

  // [ and { may begin a simple key.
  save_Simple_کلید_Candidate(--TokenQueue.end(), Column - 1, نادرست);

  // And may also be followed by a simple key.
  Is_Simple_کلید_Allowed = درست;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_مقدار_Allowed_In_Flow = نادرست;
  ++FlowLevel;
  بازگشت درست;
}

منطقی Scanner::scanFlowCollectionEnd(منطقی IsSequence) {
  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = نادرست;
  Is_Adjacent_مقدار_Allowed_In_Flow = درست;
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceEnd
                      : Token::TK_FlowMappingEnd;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.افزودن_back(T);
  اگر (FlowLevel)
    --FlowLevel;
  بازگشت درست;
}

منطقی Scanner::scanFlowEntry() {
  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = درست;
  Is_Adjacent_مقدار_Allowed_In_Flow = نادرست;
  Token T;
  T.Kind = Token::TK_FlowEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.افزودن_back(T);
  بازگشت درست;
}

منطقی Scanner::scanBlockEntry() {
  rollIndent(Column, Token::TK_BlockSequenceStart, TokenQueue.end());
  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = درست;
  Is_Adjacent_مقدار_Allowed_In_Flow = نادرست;
  Token T;
  T.Kind = Token::TK_BlockEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.افزودن_back(T);
  بازگشت درست;
}

منطقی Scanner::scan_کلید() {
  اگر (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = !FlowLevel;
  Is_Adjacent_مقدار_Allowed_In_Flow = نادرست;

  Token T;
  T.Kind = Token::TK_کلید;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.افزودن_back(T);
  بازگشت درست;
}

منطقی Scanner::scan_مقدار() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  اگر (!Simple_کلیدها.empty()) {
    Simple_کلید SK = Simple_کلیدها.برداشت_back_val();
    Token T;
    T.Kind = Token::TK_کلید;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    برای (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      اگر (i == SK.Tok)
        بشکن;
    }
    اگر (i == e) {
      Failed = درست;
      بازگشت نادرست;
    }
    i = TokenQueue.درج(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    Is_Simple_کلید_Allowed = نادرست;
  } وگرنه {
    اگر (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    Is_Simple_کلید_Allowed = !FlowLevel;
  }
  Is_Adjacent_مقدار_Allowed_In_Flow = نادرست;

  Token T;
  T.Kind = Token::TK_مقدار;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.افزودن_back(T);
  بازگشت درست;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE ایستا منطقی
wasEscaped(StringRef::iterator First, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
ایستا منطقی wasEscaped(StringRef::iterator First,
                       StringRef::iterator Position) {
  أكد(Position - 1 >= First);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  وقتی (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  بازگشت (Position - 1 - I) % 2 == 1;
}

منطقی Scanner::scanFlowScalar(منطقی IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  اگر (IsDoubleQuoted) {
    انجامبده {
      ++Current;
      وقتی (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } وقتی (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } وگرنه {
    skip(1);
    وقتی (Current != End) {
      // Skip a ' followed by another '.
      اگر (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        ادامه;
      } وگرنه اگر (*Current == '\'')
        بشکن;
      StringRef::iterator i = skip_nb_char(Current);
      اگر (i == Current) {
        i = skip_b_break(Current);
        اگر (i == Current)
          بشکن;
        Current = i;
        Column = 0;
        ++سطر;
      } وگرنه {
        اگر (i == End)
          بشکن;
        Current = i;
        ++Column;
      }
    }
  }

  اگر (Current == End) {
    setError("Expected quote at end of scalar", Current);
    بازگشت نادرست;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.افزودن_back(T);

  save_Simple_کلید_Candidate(--TokenQueue.end(), ColStart, نادرست);

  Is_Simple_کلید_Allowed = نادرست;
  Is_Adjacent_مقدار_Allowed_In_Flow = درست;

  بازگشت درست;
}

منطقی Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  unsigned LeadingBlanks = 0;
  أكد(Indent >= -1 && "Indent must be >= -1 !");
  unsigned indent = static_cast<unsigned>(Indent + 1);
  وقتی (Current != End) {
    اگر (*Current == '#')
      بشکن;

    وقتی (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      اگر (i == Current)
        بشکن;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    اگر (!isBlankOrBreak(Current))
      بشکن;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    وقتی (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      اگر (i != Tmp) {
        اگر (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          بازگشت نادرست;
        }
        Tmp = i;
        ++Column;
      } وگرنه {
        i = skip_b_break(Tmp);
        اگر (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++سطر;
      }
    }

    اگر (!FlowLevel && Column < indent)
      بشکن;

    Current = Tmp;
  }
  اگر (Start == Current) {
    setError("Got empty plain scalar", Start);
    بازگشت نادرست;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.افزودن_back(T);

  // Plain scalars can be simple keys.
  save_Simple_کلید_Candidate(--TokenQueue.end(), ColStart, نادرست);

  Is_Simple_کلید_Allowed = نادرست;
  Is_Adjacent_مقدار_Allowed_In_Flow = نادرست;

  بازگشت درست;
}

منطقی Scanner::scanAliasOrAnchor(منطقی IsAlias) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1);
  وقتی (Current != End) {
    اگر (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      بشکن;
    StringRef::iterator i = skip_ns_char(Current);
    اگر (i == Current)
      بشکن;
    Current = i;
    ++Column;
  }

  اگر (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    بازگشت نادرست;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.افزودن_back(T);

  // Alias and anchors can be simple keys.
  save_Simple_کلید_Candidate(--TokenQueue.end(), ColStart, نادرست);

  Is_Simple_کلید_Allowed = نادرست;
  Is_Adjacent_مقدار_Allowed_In_Flow = نادرست;

  بازگشت درست;
}

منطقی Scanner::scanBlockScalarIndicators(نویسه &StyleIndicator,
                                        نویسه &ChompingIndicator,
                                        unsigned &IndentIndicator,
                                        منطقی &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  اگر (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    بازگشت نادرست;
  بازگشت درست;
}

نویسه Scanner::scanBlockStyleIndicator() {
  نویسه Indicator = ' ';
  اگر (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  بازگشت Indicator;
}

نویسه Scanner::scanBlockChompingIndicator() {
  نویسه Indicator = ' ';
  اگر (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  بازگشت Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
ایستا unsigned get_Chomped_سطر_Breaks(نویسه ChompingIndicator,
                                     unsigned سطر_Breaks, StringRef Str) {
  اگر (ChompingIndicator == '-') // Strip all line breaks.
    بازگشت 0;
  اگر (ChompingIndicator == '+') // Keep all line breaks.
    بازگشت سطر_Breaks;
  // Clip trailing lines.
  بازگشت Str.empty() ? 0 : 1;
}

unsigned Scanner::scanBlockIndentationIndicator() {
  unsigned Indent = 0;
  اگر (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = unsigned(*Current - '0');
    skip(1);
  }
  بازگشت Indent;
}

منطقی Scanner::scanBlockScalarHeader(نویسه &ChompingIndicator,
                                    unsigned &IndentIndicator, منطقی &IsDone) {
  auto Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  اگر (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  اگر (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.افزودن_back(T);
    IsDone = درست;
    بازگشت درست;
  }

  اگر (!consume_سطر_Break_If_Present()) {
    setError("Expected a line break after block scalar header", Current);
    بازگشت نادرست;
  }
  بازگشت درست;
}

منطقی Scanner::findBlockScalarIndent(unsigned &BlockIndent,
                                    unsigned BlockExitIndent,
                                    unsigned &سطر_Breaks, منطقی &IsDone) {
  unsigned Max_All_Space_سطر_Characters = 0;
  StringRef::iterator Longest_All_Space_سطر;

  وقتی (درست) {
    advanceWhile(&Scanner::skip_s_space);
    اگر (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      اگر (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = درست;
        بازگشت درست;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      اگر (Max_All_Space_سطر_Characters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_سطر);
        بازگشت نادرست;
      }
      بازگشت درست;
    }
    اگر (skip_b_break(Current) != Current &&
        Column > Max_All_Space_سطر_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_سطر_Characters = Column;
      Longest_All_Space_سطر = Current;
    }

    // Check for EOF.
    اگر (Current == End) {
      IsDone = درست;
      بازگشت درست;
    }

    اگر (!consume_سطر_Break_If_Present()) {
      IsDone = درست;
      بازگشت درست;
    }
    ++سطر_Breaks;
  }
  بازگشت درست;
}

منطقی Scanner::scanBlockScalarIndent(unsigned BlockIndent,
                                    unsigned BlockExitIndent, منطقی &IsDone) {
  // Skip the indentation.
  وقتی (Column < BlockIndent) {
    auto I = skip_s_space(Current);
    اگر (I == Current)
      بشکن;
    Current = I;
    ++Column;
  }

  اگر (skip_nb_char(Current) == Current)
    بازگشت درست;

  اگر (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = درست;
    بازگشت درست;
  }

  اگر (Column < BlockIndent) {
    اگر (Current != End && *Current == '#') { // Trailing comment.
      IsDone = درست;
      بازگشت درست;
    }
    setError("A text line is less indented than the block scalar", Current);
    بازگشت نادرست;
  }
  بازگشت درست; // A normal text line.
}

منطقی Scanner::scanBlockScalar(منطقی IsLiteral) {
  أكد(*Current == '|' || *Current == '>');
  نویسه StyleIndicator;
  نویسه ChompingIndicator;
  unsigned BlockIndent;
  منطقی IsDone = نادرست;
  اگر (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    بازگشت نادرست;
  اگر (IsDone)
    بازگشت درست;
  منطقی IsFolded = StyleIndicator == '>';

  ثابت auto *Start = Current;
  unsigned BlockExitIndent = Indent < 0 ? 0 : (unsigned)Indent;
  unsigned سطر_Breaks = 0;
  اگر (BlockIndent == 0) {
    اگر (!findBlockScalarIndent(BlockIndent, BlockExitIndent, سطر_Breaks,
                               IsDone))
      بازگشت نادرست;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  وقتی (!IsDone) {
    اگر (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      بازگشت نادرست;
    اگر (IsDone)
      بشکن;

    // Parse the current line.
    auto سطر_Start = Current;
    advanceWhile(&Scanner::skip_nb_char);
    اگر (سطر_Start != Current) {
      اگر (سطر_Breaks && IsFolded && !Scanner::is_سطر_Empty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        اگر (سطر_Breaks == 1) {
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
    اگر (Current == End)
      بشکن;

    اگر (!consume_سطر_Break_If_Present())
      بشکن;
    ++سطر_Breaks;
  }

  اگر (Current == End && !سطر_Breaks)
    // Ensure that there is at least one line break before the end of file.
    سطر_Breaks = 1;
  Str.append(get_Chomped_سطر_Breaks(ChompingIndicator, سطر_Breaks, Str), '\n');

  // New lines may start a simple key.
  اگر (!FlowLevel)
    Is_Simple_کلید_Allowed = درست;
  Is_Adjacent_مقدار_Allowed_In_Flow = نادرست;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.مقدار = std::string(Str);
  TokenQueue.افزودن_back(T);
  بازگشت درست;
}

منطقی Scanner::scanTag() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1); // Eat !.
  اگر (Current == End || isBlankOrBreak(Current)); // An empty tag.
  وگرنه اگر (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    اگر (!consume('>'))
      بازگشت نادرست;
  } وگرنه {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.Kind = Token::TK_Tag;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.افزودن_back(T);

  // Tags can be simple keys.
  save_Simple_کلید_Candidate(--TokenQueue.end(), ColStart, نادرست);

  Is_Simple_کلید_Allowed = نادرست;
  Is_Adjacent_مقدار_Allowed_In_Flow = نادرست;

  بازگشت درست;
}

منطقی Scanner::fetchMoreTokens() {
  اگر (IsStartOfStream)
    بازگشت scanStreamStart();

  scanToNextToken();

  اگر (Current == End)
    بازگشت scanStreamEnd();

  remove_Stale_Simple_کلید_Candidates();

  unrollIndent(Column);

  اگر (Column == 0 && *Current == '%')
    بازگشت scanDirective();

  اگر (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    بازگشت scanDocumentIndicator(درست);

  اگر (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    بازگشت scanDocumentIndicator(نادرست);

  اگر (*Current == '[')
    بازگشت scanFlowCollectionStart(درست);

  اگر (*Current == '{')
    بازگشت scanFlowCollectionStart(نادرست);

  اگر (*Current == ']')
    بازگشت scanFlowCollectionEnd(درست);

  اگر (*Current == '}')
    بازگشت scanFlowCollectionEnd(نادرست);

  اگر (*Current == ',')
    بازگشت scanFlowEntry();

  اگر (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    بازگشت scanBlockEntry();

  اگر (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    بازگشت scan_کلید();

  اگر (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || Is_Adjacent_مقدار_Allowed_In_Flow))
    بازگشت scan_مقدار();

  اگر (*Current == '*')
    بازگشت scanAliasOrAnchor(درست);

  اگر (*Current == '&')
    بازگشت scanAliasOrAnchor(نادرست);

  اگر (*Current == '!')
    بازگشت scanTag();

  اگر (*Current == '|' && !FlowLevel)
    بازگشت scanBlockScalar(درست);

  اگر (*Current == '>' && !FlowLevel)
    بازگشت scanBlockScalar(نادرست);

  اگر (*Current == '\'')
    بازگشت scanFlowScalar(نادرست);

  اگر (*Current == '"')
    بازگشت scanFlowScalar(درست);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  اگر ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    بازگشت scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  بازگشت نادرست;
}

Stream::Stream(StringRef درون_داد, SourceMgr &SM, منطقی ShowColors,
               std::error_code *EC)
    : scanner(جدید Scanner(درون_داد, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef درون_داد_Buffer, SourceMgr &SM, منطقی ShowColors,
               std::error_code *EC)
    : scanner(جدید Scanner(درون_داد_Buffer, SM, ShowColors, EC)) {}

Stream::~Stream() = پیشفرض;

منطقی Stream::failed() { بازگشت scanner->failed(); }

تهی Stream::printError(گره *N, ثابت Twine &پیام_2, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), پیام_2, Kind);
}

تهی Stream::printError(ثابت SMRange &Range, ثابت Twine &پیام_2,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, پیام_2, Range);
}

document_iterator Stream::begin() {
  اگر (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(جدید Document(*این));
  بازگشت document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  بازگشت document_iterator();
}

تهی Stream::skip() {
  برای (Document &Doc : *این)
    Doc.skip();
}

گره::گره(unsigned صحیح Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string گره::getVerbatimTag() ثابت {
  StringRef Raw = getRawTag();
  اگر (!Raw.empty() && Raw != "!") {
    std::string Ret;
    اگر (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      بازگشت Ret;
    } وگرنه اگر (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      بازگشت Ret;
    } وگرنه {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      اگر (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      وگرنه {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      بازگشت Ret;
    }
  }

  انتخاب (getType()) {
  حالت NK_Null:
    بازگشت "tag:yaml.org,2002:null";
  حالت NK_Scalar:
  حالت NK_BlockScalar:
    // TODO: Tag resolution.
    بازگشت "tag:yaml.org,2002:str";
  حالت NK_Mapping:
    بازگشت "tag:yaml.org,2002:map";
  حالت NK_Sequence:
    بازگشت "tag:yaml.org,2002:seq";
  }

  بازگشت "";
}

Token &گره::peekNext() {
  بازگشت Doc->peekNext();
}

Token گره::getNext() {
  بازگشت Doc->getNext();
}

گره *گره::parse_Block_گره() {
  بازگشت Doc->parse_Block_گره();
}

BumpPtrAllocator &گره::getAllocator() {
  بازگشت Doc->گره_Allocator;
}

تهی گره::setError(ثابت Twine &پیام_2, Token &Tok) ثابت {
  Doc->setError(پیام_2, Tok);
}

منطقی گره::failed() ثابت {
  بازگشت Doc->failed();
}

StringRef Scalar_گره::get_مقدار(SmallVectorImpl<نویسه> &Storage) ثابت {
  اگر (مقدار[0] == '"')
    بازگشت get_Double_Quoted_مقدار(مقدار, Storage);
  اگر (مقدار[0] == '\'')
    بازگشت get_Single_Quoted_مقدار(مقدار, Storage);
  بازگشت get_Plain_مقدار(مقدار, Storage);
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
ایستا StringRef
parse_Scalar_مقدار(StringRef Unquoted_مقدار, SmallVectorImpl<نویسه> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<نویسه> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_مقدار.find_first_of(LookupChars);
  اگر (I == StringRef::npos)
    بازگشت Unquoted_مقدار;

  Storage.clear();
  Storage.reserve(Unquoted_مقدار.size());
  نویسه Last_New_سطر_Added_As = '\0';
  برای (; I != StringRef::npos; I = Unquoted_مقدار.find_first_of(LookupChars)) {
    اگر (Unquoted_مقدار[I] != '\r' && Unquoted_مقدار[I] != '\n') {
      llvm::append_range(Storage, Unquoted_مقدار.take_front(I));
      Unquoted_مقدار = UnescapeCallback(Unquoted_مقدار.drop_front(I), Storage);
      Last_New_سطر_Added_As = '\0';
      ادامه;
    }
    اگر (size_t LastNonSWhite = Unquoted_مقدار.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_مقدار.take_front(LastNonSWhite + 1));
      Storage.افزودن_back(' ');
      Last_New_سطر_Added_As = ' ';
    } وگرنه {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      انتخاب (Last_New_سطر_Added_As) {
      حالت ' ':
        أكد(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        Last_New_سطر_Added_As = '\n';
        بشکن;
      حالت '\n':
        أكد(!Storage.empty() && Storage.back() == '\n');
        Storage.افزودن_back('\n');
        بشکن;
      پیشفرض:
        Storage.افزودن_back(' ');
        Last_New_سطر_Added_As = ' ';
        بشکن;
      }
    }
    // Handle Windows-style EOL
    اگر (Unquoted_مقدار.substr(I, 2) == "\r\n")
      I++;
    Unquoted_مقدار = Unquoted_مقدار.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_مقدار);
  بازگشت StringRef(Storage.begin(), Storage.size());
}

StringRef
Scalar_گره::get_Double_Quoted_مقدار(StringRef Raw_مقدار,
                                 SmallVectorImpl<نویسه> &Storage) ثابت {
  أكد(Raw_مقدار.size() >= 2 && Raw_مقدار.front() == '"' &&
         Raw_مقدار.back() == '"');
  StringRef Unquoted_مقدار = Raw_مقدار.substr(1, Raw_مقدار.size() - 2);

  auto UnescapeFunc = [این](StringRef Unquoted_مقدار,
                             SmallVectorImpl<نویسه> &Storage) {
    أكد(Unquoted_مقدار.take_front(1) == "\\");
    اگر (Unquoted_مقدار.size() == 1) {
      Token T;
      T.Range = Unquoted_مقدار;
      setError("Unrecognized escape code", T);
      Storage.clear();
      بازگشت StringRef();
    }
    Unquoted_مقدار = Unquoted_مقدار.drop_front(1);
    انتخاب (Unquoted_مقدار[0]) {
    پیشفرض: {
      Token T;
      T.Range = Unquoted_مقدار.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      بازگشت StringRef();
    }
    حالت '\r':
      // Shrink the Windows-style EOL.
      اگر (Unquoted_مقدار.size() >= 2 && Unquoted_مقدار[1] == '\n')
        Unquoted_مقدار = Unquoted_مقدار.drop_front(1);
      [[fallthrough]];
    حالت '\n':
      بازگشت Unquoted_مقدار.drop_front(1).ltrim(" \t");
    حالت '0':
      Storage.افزودن_back(0x00);
      بشکن;
    حالت 'a':
      Storage.افزودن_back(0x07);
      بشکن;
    حالت 'b':
      Storage.افزودن_back(0x08);
      بشکن;
    حالت 't':
    حالت 0x09:
      Storage.افزودن_back(0x09);
      بشکن;
    حالت 'n':
      Storage.افزودن_back(0x0A);
      بشکن;
    حالت 'v':
      Storage.افزودن_back(0x0B);
      بشکن;
    حالت 'f':
      Storage.افزودن_back(0x0C);
      بشکن;
    حالت 'r':
      Storage.افزودن_back(0x0D);
      بشکن;
    حالت 'e':
      Storage.افزودن_back(0x1B);
      بشکن;
    حالت ' ':
      Storage.افزودن_back(0x20);
      بشکن;
    حالت '"':
      Storage.افزودن_back(0x22);
      بشکن;
    حالت '/':
      Storage.افزودن_back(0x2F);
      بشکن;
    حالت '\\':
      Storage.افزودن_back(0x5C);
      بشکن;
    حالت 'N':
      encodeUTF8(0x85, Storage);
      بشکن;
    حالت '_':
      encodeUTF8(0xA0, Storage);
      بشکن;
    حالت 'L':
      encodeUTF8(0x2028, Storage);
      بشکن;
    حالت 'P':
      encodeUTF8(0x2029, Storage);
      بشکن;
    حالت 'x': {
      اگر (Unquoted_مقدار.size() < 3)
        // TODO: Report error.
        بشکن;
      unsigned صحیح Unicode_Scalar_مقدار;
      اگر (Unquoted_مقدار.substr(1, 2).getAsInteger(16, Unicode_Scalar_مقدار))
        // TODO: Report error.
        Unicode_Scalar_مقدار = 0xFFFD;
      encodeUTF8(Unicode_Scalar_مقدار, Storage);
      بازگشت Unquoted_مقدار.drop_front(3);
    }
    حالت 'u': {
      اگر (Unquoted_مقدار.size() < 5)
        // TODO: Report error.
        بشکن;
      unsigned صحیح Unicode_Scalar_مقدار;
      اگر (Unquoted_مقدار.substr(1, 4).getAsInteger(16, Unicode_Scalar_مقدار))
        // TODO: Report error.
        Unicode_Scalar_مقدار = 0xFFFD;
      encodeUTF8(Unicode_Scalar_مقدار, Storage);
      بازگشت Unquoted_مقدار.drop_front(5);
    }
    حالت 'U': {
      اگر (Unquoted_مقدار.size() < 9)
        // TODO: Report error.
        بشکن;
      unsigned صحیح Unicode_Scalar_مقدار;
      اگر (Unquoted_مقدار.substr(1, 8).getAsInteger(16, Unicode_Scalar_مقدار))
        // TODO: Report error.
        Unicode_Scalar_مقدار = 0xFFFD;
      encodeUTF8(Unicode_Scalar_مقدار, Storage);
      بازگشت Unquoted_مقدار.drop_front(9);
    }
    }
    بازگشت Unquoted_مقدار.drop_front(1);
  };

  بازگشت parse_Scalar_مقدار(Unquoted_مقدار, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_گره::get_Single_Quoted_مقدار(StringRef Raw_مقدار,
                                           SmallVectorImpl<نویسه> &Storage) {
  أكد(Raw_مقدار.size() >= 2 && Raw_مقدار.front() == '\'' &&
         Raw_مقدار.back() == '\'');
  StringRef Unquoted_مقدار = Raw_مقدار.substr(1, Raw_مقدار.size() - 2);

  auto UnescapeFunc = [](StringRef Unquoted_مقدار,
                         SmallVectorImpl<نویسه> &Storage) {
    أكد(Unquoted_مقدار.take_front(2) == "''");
    Storage.افزودن_back('\'');
    بازگشت Unquoted_مقدار.drop_front(2);
  };

  بازگشت parse_Scalar_مقدار(Unquoted_مقدار, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_گره::get_Plain_مقدار(StringRef Raw_مقدار,
                                    SmallVectorImpl<نویسه> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_مقدار = Raw_مقدار.rtrim("\r\n \t");
  بازگشت parse_Scalar_مقدار(Raw_مقدار, Storage, "\r\n", nullptr);
}

گره *کلید_مقدار_گره::get_کلید() {
  اگر (کلید)
    بازگشت کلید;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    اگر (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_مقدار
        || t.Kind == Token::TK_Error) {
      بازگشت کلید = جدید (getAllocator()) Null_گره(Doc);
    }
    اگر (t.Kind == Token::TK_کلید)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  اگر (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_مقدار) {
    بازگشت کلید = جدید (getAllocator()) Null_گره(Doc);
  }

  // We've got a normal key.
  بازگشت کلید = parse_Block_گره();
}

گره *کلید_مقدار_گره::get_مقدار() {
  اگر (مقدار)
    بازگشت مقدار;

  اگر (گره* کلید = get_کلید())
    کلید->skip();
  وگرنه {
    setError("Null key in Key Value.", peekNext());
    بازگشت مقدار = جدید (getAllocator()) Null_گره(Doc);
  }

  اگر (failed())
    بازگشت مقدار = جدید (getAllocator()) Null_گره(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    اگر (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_کلید
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      بازگشت مقدار = جدید (getAllocator()) Null_گره(Doc);
    }

    اگر (t.Kind != Token::TK_مقدار) {
      setError("Unexpected token in Key Value.", t);
      بازگشت مقدار = جدید (getAllocator()) Null_گره(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  اگر (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_کلید) {
    بازگشت مقدار = جدید (getAllocator()) Null_گره(Doc);
  }

  // We got a normal value.
  بازگشت مقدار = parse_Block_گره();
}

تهی Mapping_گره::increment() {
  اگر (failed()) {
    IsAtEnd = درست;
    CurrentEntry = nullptr;
    بازگشت;
  }
  اگر (CurrentEntry) {
    CurrentEntry->skip();
    اگر (Type == MT_Inline) {
      IsAtEnd = درست;
      CurrentEntry = nullptr;
      بازگشت;
    }
  }
  Token T = peekNext();
  اگر (T.Kind == Token::TK_کلید || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = جدید (getAllocator()) کلید_مقدار_گره(Doc);
  } وگرنه اگر (Type == MT_Block) {
    انتخاب (T.Kind) {
    حالت Token::TK_BlockEnd:
      getNext();
      IsAtEnd = درست;
      CurrentEntry = nullptr;
      بشکن;
    پیشفرض:
      setError("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    حالت Token::TK_Error:
      IsAtEnd = درست;
      CurrentEntry = nullptr;
    }
  } وگرنه {
    انتخاب (T.Kind) {
    حالت Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      بازگشت increment();
    حالت Token::TK_FlowMappingEnd:
      getNext();
      [[fallthrough]];
    حالت Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = درست;
      CurrentEntry = nullptr;
      بشکن;
    پیشفرض:
      setError( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      IsAtEnd = درست;
      CurrentEntry = nullptr;
    }
  }
}

تهی Sequence_گره::increment() {
  اگر (failed()) {
    IsAtEnd = درست;
    CurrentEntry = nullptr;
    بازگشت;
  }
  اگر (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  اگر (SeqType == ST_Block) {
    انتخاب (T.Kind) {
    حالت Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_گره();
      اگر (!CurrentEntry) { // An error occurred.
        IsAtEnd = درست;
        CurrentEntry = nullptr;
      }
      بشکن;
    حالت Token::TK_BlockEnd:
      getNext();
      IsAtEnd = درست;
      CurrentEntry = nullptr;
      بشکن;
    پیشفرض:
      setError( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    حالت Token::TK_Error:
      IsAtEnd = درست;
      CurrentEntry = nullptr;
    }
  } وگرنه اگر (SeqType == ST_Indentless) {
    انتخاب (T.Kind) {
    حالت Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_گره();
      اگر (!CurrentEntry) { // An error occurred.
        IsAtEnd = درست;
        CurrentEntry = nullptr;
      }
      بشکن;
    پیشفرض:
    حالت Token::TK_Error:
      IsAtEnd = درست;
      CurrentEntry = nullptr;
    }
  } وگرنه اگر (SeqType == ST_Flow) {
    انتخاب (T.Kind) {
    حالت Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      WasPreviousTokenFlowEntry = درست;
      بازگشت increment();
    حالت Token::TK_FlowSequenceEnd:
      getNext();
      [[fallthrough]];
    حالت Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = درست;
      CurrentEntry = nullptr;
      بشکن;
    حالت Token::TK_StreamEnd:
    حالت Token::TK_DocumentEnd:
    حالت Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = درست;
      CurrentEntry = nullptr;
      بشکن;
    پیشفرض:
      اگر (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = درست;
        CurrentEntry = nullptr;
        بشکن;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parse_Block_گره();
      اگر (!CurrentEntry) {
        IsAtEnd = درست;
      }
      WasPreviousTokenFlowEntry = نادرست;
      بشکن;
    }
  }
}

Document::Document(Stream &S) : stream(S), ریشه_2(nullptr) {
  // Tag maps starts with two default mappings.
  TagMap["!"] = "!";
  TagMap["!!"] = "tag:yaml.org,2002:";

  اگر (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peekNext();
  اگر (T.Kind == Token::TK_DocumentStart)
    getNext();
}

منطقی Document::skip()  {
  اگر (stream.scanner->failed())
    بازگشت نادرست;
  اگر (!ریشه_2 && !get_ریشه())
    بازگشت نادرست;
  ریشه_2->skip();
  Token &T = peekNext();
  اگر (T.Kind == Token::TK_StreamEnd)
    بازگشت نادرست;
  اگر (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    بازگشت skip();
  }
  بازگشت درست;
}

Token &Document::peekNext() {
  بازگشت stream.scanner->peekNext();
}

Token Document::getNext() {
  بازگشت stream.scanner->getNext();
}

تهی Document::setError(ثابت Twine &Message, Token &Location) ثابت {
  stream.scanner->setError(Message, Location.Range.begin());
}

منطقی Document::failed() ثابت {
  بازگشت stream.scanner->failed();
}

گره *Document::parse_Block_گره() {
  Token T = peekNext();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  انتخاب (T.Kind) {
  حالت Token::TK_Alias:
    getNext();
    بازگشت جدید (گره_Allocator) Alias_گره(stream.CurrentDoc, T.Range.substr(1));
  حالت Token::TK_Anchor:
    اگر (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      بازگشت nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    برو parse_property;
  حالت Token::TK_Tag:
    اگر (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      بازگشت nullptr;
    }
    TagInfo = getNext(); // Consume TK_Tag.
    T = peekNext();
    برو parse_property;
  پیشفرض:
    بشکن;
  }

  انتخاب (T.Kind) {
  حالت Token::TK_BlockEntry:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    بازگشت جدید (گره_Allocator) Sequence_گره( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , Sequence_گره::ST_Indentless);
  حالت Token::TK_BlockSequenceStart:
    getNext();
    بازگشت جدید (گره_Allocator)
      Sequence_گره( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_گره::ST_Block);
  حالت Token::TK_BlockMappingStart:
    getNext();
    بازگشت جدید (گره_Allocator)
      Mapping_گره( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_گره::MT_Block);
  حالت Token::TK_FlowSequenceStart:
    getNext();
    بازگشت جدید (گره_Allocator)
      Sequence_گره( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_گره::ST_Flow);
  حالت Token::TK_FlowMappingStart:
    getNext();
    بازگشت جدید (گره_Allocator)
      Mapping_گره( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_گره::MT_Flow);
  حالت Token::TK_Scalar:
    getNext();
    بازگشت جدید (گره_Allocator)
      Scalar_گره( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  حالت Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.مقدار.c_str(), T.مقدار.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(گره_Allocator).drop_back();
    بازگشت جدید (گره_Allocator)
        Block_Scalar_گره(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  حالت Token::TK_کلید:
    // Don't eat the TK_Key, KeyValueNode expects it.
    بازگشت جدید (گره_Allocator)
      Mapping_گره( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_گره::MT_Inline);
  حالت Token::TK_DocumentStart:
  حالت Token::TK_DocumentEnd:
  حالت Token::TK_StreamEnd:
  پیشفرض:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    بازگشت جدید (گره_Allocator) Null_گره(stream.CurrentDoc);
  حالت Token::TK_FlowMappingEnd:
  حالت Token::TK_FlowSequenceEnd:
  حالت Token::TK_FlowEntry: {
    اگر (ریشه_2 && (isa<Mapping_گره>(ریشه_2) || isa<Sequence_گره>(ریشه_2)))
      بازگشت جدید (گره_Allocator) Null_گره(stream.CurrentDoc);

    setError("Unexpected token", T);
    بازگشت nullptr;
  }
  حالت Token::TK_Error:
    بازگشت nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  بازگشت nullptr;
}

منطقی Document::parseDirectives() {
  منطقی isDirective = نادرست;
  وقتی (درست) {
    Token T = peekNext();
    اگر (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = درست;
    } وگرنه اگر (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = درست;
    } وگرنه {
      بشکن;
    }
  }
  بازگشت isDirective;
}

تهی Document::parseYAMLDirective() {
  getNext(); // Eat %YAML <version>
}

تهی Document::parseTAGDirective() {
  Token Tag = getNext(); // %TAG <handle> <prefix>
  StringRef T = Tag.Range;
  // Strip %TAG
  T = T.substr(T.find_first_of(" \t")).ltrim(" \t");
  std::size_t HandleEnd = T.find_first_of(" \t");
  StringRef TagHandle = T.substr(0, HandleEnd);
  StringRef TagPrefix = T.substr(HandleEnd).ltrim(" \t");
  TagMap[TagHandle] = TagPrefix;
}

منطقی Document::expectToken(صحیح TK) {
  Token T = getNext();
  اگر (T.Kind != TK) {
    setError("Unexpected token", T);
    بازگشت نادرست;
  }
  بازگشت درست;
}
