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

#अन्तर्भाव "llvm/Support/YAMLParser.h"
#अन्तर्भाव "llvm/ADT/AllocatorList.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/ADT/Twine.h"
#अन्तर्भाव "llvm/Support/Compiler.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/SMLoc.h"
#अन्तर्भाव "llvm/Support/SourceMgr.h"
#अन्तर्भाव "llvm/Support/Unicode.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cassert>
#अन्तर्भाव <cstddef>
#अन्तर्भाव <cstdint>
#अन्तर्भाव <map>
#अन्तर्भाव <memory>
#अन्तर्भाव <string>
#अन्तर्भाव <system_error>
#अन्तर्भाव <utility>

using नामाकाश llvm;
using नामाकाश yaml;

गणना UnicodeEncodingForm {
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
स्थिर EncodingInfo getUnicodeEncoding(StringRef निवेशः) {
  सचे (निवेशः.empty())
    पटिनिवत्त {UEF_Unknown, 0};

  विकल्प (uint8_t(निवेशः[0])) {
  पक्ष 0x00:
    सचे (निवेशः.size() >= 4) {
      सचे (  निवेशः[1] == 0
         && uint8_t(निवेशः[2]) == 0xFE
         && uint8_t(निवेशः[3]) == 0xFF)
        पटिनिवत्त {UEF_UTF32_BE, 4};
      सचे (निवेशः[1] == 0 && निवेशः[2] == 0 && निवेशः[3] != 0)
        पटिनिवत्त {UEF_UTF32_BE, 0};
    }

    सचे (निवेशः.size() >= 2 && निवेशः[1] != 0)
      पटिनिवत्त {UEF_UTF16_BE, 0};
    पटिनिवत्त {UEF_Unknown, 0};
  पक्ष 0xFF:
    सचे (  निवेशः.size() >= 4
       && uint8_t(निवेशः[1]) == 0xFE
       && निवेशः[2] == 0
       && निवेशः[3] == 0)
      पटिनिवत्त {UEF_UTF32_LE, 4};

    सचे (निवेशः.size() >= 2 && uint8_t(निवेशः[1]) == 0xFE)
      पटिनिवत्त {UEF_UTF16_LE, 2};
    पटिनिवत्त {UEF_Unknown, 0};
  पक्ष 0xFE:
    सचे (निवेशः.size() >= 2 && uint8_t(निवेशः[1]) == 0xFF)
      पटिनिवत्त {UEF_UTF16_BE, 2};
    पटिनिवत्त {UEF_Unknown, 0};
  पक्ष 0xEF:
    सचे (  निवेशः.size() >= 3
       && uint8_t(निवेशः[1]) == 0xBB
       && uint8_t(निवेशः[2]) == 0xBF)
      पटिनिवत्त {UEF_UTF8, 3};
    पटिनिवत्त {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  सचे (निवेशः.size() >= 4 && निवेशः[1] == 0 && निवेशः[2] == 0 && निवेशः[3] == 0)
    पटिनिवत्त {UEF_UTF32_LE, 0};

  सचे (निवेशः.size() >= 2 && निवेशः[1] == 0)
    पटिनिवत्त {UEF_UTF16_LE, 0};

  पटिनिवत्त {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
शून्य ग्रन्थिः::anchor() {}
शून्य Null_ग्रन्थिः::anchor() {}
शून्य Scalar_ग्रन्थिः::anchor() {}
शून्य Block_Scalar_ग्रन्थिः::anchor() {}
शून्य कुञ्चिका_मानम्_ग्रन्थिः::anchor() {}
शून्य Mapping_ग्रन्थिः::anchor() {}
शून्य Sequence_ग्रन्थिः::anchor() {}
शून्य Alias_ग्रन्थिः::anchor() {}

नामाकाश llvm {
नामाकाश yaml {

/// Token - A single YAML token.
संरचना Token {
  गणना TokenKind {
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
    TK_कुञ्चिका,
    TK_मानम्,
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
  std::string मानम्;

  Token() = स्वभाव;
};

} // end namespace yaml
} // end namespace llvm

using TokenQueueT = BumpPtrList<Token>;

नामाकाश {

/// This struct is used to track simple keys.
///
/// Simple keys are handled by creating an entry in SimpleKeys for each Token
/// which could legally be the start of a simple key. When peekNext is called,
/// if the Token To be returned is referenced by a SimpleKey, we continue
/// tokenizing until that potential simple key has either been found to not be
/// a simple key (we moved on to the next line or went further than 1024 chars).
/// Or when we run into a Value, and then insert a Key token (and possibly
/// others) before the SimpleKey's Tok.
संरचना Simple_कुञ्चिका {
  TokenQueueT::iterator Tok;
  unsigned Column = 0;
  unsigned पङ्क्तिः = 0;
  unsigned FlowLevel = 0;
  तार्किक IsRequired = मिच्छा;

  तार्किक operator ==(ध्रुव Simple_कुञ्चिका &Other) {
    पटिनिवत्त Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
using UTF8Decoded = std::pair<uint32_t, unsigned>;

स्थिर UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator End = Range.end();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  सचे (Position < End && (*Position & 0x80) == 0) {
    पटिनिवत्त {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  सचे (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    सचे (codepoint >= 0x80)
      पटिनिवत्त {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  सचे (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    सचे (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      पटिनिवत्त {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  सचे (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    सचे (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      पटिनिवत्त {codepoint, 4};
  }
  पटिनिवत्त {0, 0};
}

नामाकाश llvm {
नामाकाश yaml {

/// Scans YAML tokens from a MemoryBuffer.
वग्ग Scanner {
सार्वजनिक:
  Scanner(StringRef निवेशः, SourceMgr &SM, तार्किक ShowColors = सच्च,
          std::error_code *EC = nullptr);
  Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, तार्किक ShowColors = सच्च,
          std::error_code *EC = nullptr);

  /// Parse the next token and return it without popping it.
  Token &peekNext();

  /// Parse the next token and pop it from the queue.
  Token getNext();

  शून्य printError(SMLoc Loc, SourceMgr::DiagKind Kind, ध्रुव Twine &Message,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.PrintMessage(Loc, Kind, Message, Ranges, /* FixIts= */ {}, ShowColors);
  }

  शून्य setError(ध्रुव Twine &Message, StringRef::iterator Position) {
    सचे (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    सचे (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    सचे (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = सच्च;
  }

  /// Returns true if an error occurred while parsing.
  तार्किक failed() {
    पटिनिवत्त Failed;
  }

गुप्त:
  शून्य init(MemoryBufferRef Buffer);

  StringRef current_निवेशः() {
    पटिनिवत्त StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    पटिनिवत्त ::decodeUTF8(StringRef(Position, End - Position));
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
  शून्य advanceWhile(SkipWhileFunc Func);

  /// Scan ns-uri-char[39]s starting at Cur.
  ///
  /// This updates Cur and Column while scanning.
  शून्य scan_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  तार्किक consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  शून्य skip(uint32_t Distance);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  तार्किक isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  तार्किक isPlainSafeNonBlank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  तार्किक is_पङ्क्तिः_Empty(StringRef पङ्क्तिः);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  तार्किक consume_पङ्क्तिः_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  शून्य save_Simple_कुञ्चिका_Candidate( TokenQueueT::iterator Tok
                             , unsigned AtColumn
                             , तार्किक IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  शून्य remove_Stale_Simple_कुञ्चिका_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  शून्य remove_Simple_कुञ्चिका_Candidates_On_Flow_Level(unsigned Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  तार्किक unrollIndent(पूर्णाङ्क ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  तार्किक rollIndent( पूर्णाङ्क ToColumn
                 , Token::TokenKind Kind
                 , TokenQueueT::iterator निवेशय_Point);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  शून्य skipComment();

  /// Skip whitespace and comments until the start of the next token.
  शून्य scanToNextToken();

  /// Must be the first token generated.
  तार्किक scanStreamStart();

  /// Generate tokens needed to close out the stream.
  तार्किक scanStreamEnd();

  /// Scan a %BLAH directive.
  तार्किक scanDirective();

  /// Scan a ... or ---.
  तार्किक scanDocumentIndicator(तार्किक IsStart);

  /// Scan a [ or { and generate the proper flow collection start token.
  तार्किक scanFlowCollectionStart(तार्किक IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  तार्किक scanFlowCollectionEnd(तार्किक IsSequence);

  /// Scan the , that separates entries in a flow collection.
  तार्किक scanFlowEntry();

  /// Scan the - that starts block sequence entries.
  तार्किक scanBlockEntry();

  /// Scan an explicit ? indicating a key.
  तार्किक scan_कुञ्चिका();

  /// Scan an explicit : indicating a value.
  तार्किक scan_मानम्();

  /// Scan a quoted scalar.
  तार्किक scanFlowScalar(तार्किक IsDoubleQuoted);

  /// Scan an unquoted scalar.
  तार्किक scanPlainScalar();

  /// Scan an Alias or Anchor starting with * or &.
  तार्किक scanAliasOrAnchor(तार्किक IsAlias);

  /// Scan a block scalar starting with | or >.
  तार्किक scanBlockScalar(तार्किक IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  तार्किक scanBlockScalarIndicators(वर्ण &StyleIndicator, वर्ण &ChompingIndicator,
                                 unsigned &IndentIndicator, तार्किक &IsDone);

  /// Scan a style indicator in a block scalar header.
  वर्ण scanBlockStyleIndicator();

  /// Scan a chomping indicator in a block scalar header.
  वर्ण scanBlockChompingIndicator();

  /// Scan an indentation indicator in a block scalar header.
  unsigned scanBlockIndentationIndicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  तार्किक scanBlockScalarHeader(वर्ण &ChompingIndicator, unsigned &IndentIndicator,
                             तार्किक &IsDone);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  तार्किक findBlockScalarIndent(unsigned &BlockIndent, unsigned BlockExitIndent,
                             unsigned &पङ्क्तिः_Breaks, तार्किक &IsDone);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  तार्किक scanBlockScalarIndent(unsigned BlockIndent, unsigned BlockExitIndent,
                             तार्किक &IsDone);

  /// Scan a tag of the form !stuff.
  तार्किक scanTag();

  /// Dispatch to the next scanning function based on \a *Cur.
  तार्किक fetchMoreTokens();

  /// The SourceMgr used for diagnostics and buffer management.
  SourceMgr &SM;

  /// The original input.
  MemoryBufferRef निवेशः_Buffer;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator End;

  /// Current YAML indentation level in spaces.
  पूर्णाङ्क Indent;

  /// Current column number in Unicode code points.
  unsigned Column;

  /// Current line number.
  unsigned पङ्क्तिः;

  /// How deep we are in flow style containers. 0 Means at block level.
  unsigned FlowLevel;

  /// Are we at the start of the stream?
  तार्किक IsStartOfStream;

  /// Can the next token be the start of a simple key?
  तार्किक Is_Simple_कुञ्चिका_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  तार्किक Is_Adjacent_मानम्_Allowed_In_Flow;

  /// True if an error has occurred.
  तार्किक Failed;

  /// Should colors be used when printing out the diagnostic messages?
  तार्किक ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  TokenQueueT TokenQueue;

  /// Indentation levels.
  SmallVector<पूर्णाङ्क, 4> Indents;

  /// Potential simple keys.
  SmallVector<Simple_कुञ्चिका, 4> Simple_कुञ्चिकाः;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
स्थिर शून्य encodeUTF8( uint32_t Unicode_Scalar_मानम्
                      , SmallVectorImpl<वर्ण> &Result) {
  सचे (Unicode_Scalar_मानम् <= 0x7F) {
    Result.प्रक्षिप_back(Unicode_Scalar_मानम् & 0x7F);
  } अञ्ञथा सचे (Unicode_Scalar_मानम् <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((Unicode_Scalar_मानम् & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Unicode_Scalar_मानम् & 0x3F);
    Result.प्रक्षिप_back(FirstByte);
    Result.प्रक्षिप_back(SecondByte);
  } अञ्ञथा सचे (Unicode_Scalar_मानम् <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((Unicode_Scalar_मानम् & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_मानम् & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Unicode_Scalar_मानम् & 0x3F);
    Result.प्रक्षिप_back(FirstByte);
    Result.प्रक्षिप_back(SecondByte);
    Result.प्रक्षिप_back(ThirdByte);
  } अञ्ञथा सचे (Unicode_Scalar_मानम् <= 0x10FFFF) {
    uint8_t FirstByte = 0xF0 | ((Unicode_Scalar_मानम् & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_मानम् & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Unicode_Scalar_मानम् & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_मानम् & 0x3F);
    Result.प्रक्षिप_back(FirstByte);
    Result.प्रक्षिप_back(SecondByte);
    Result.प्रक्षिप_back(ThirdByte);
    Result.प्रक्षिप_back(FourthByte);
  }
}

तार्किक yaml::dumpTokens(StringRef निवेशः, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(निवेशः, SM);
  याव (सच्च) {
    Token T = scanner.getNext();
    विकल्प (T.Kind) {
    पक्ष Token::TK_StreamStart:
      OS << "Stream-Start: ";
      भङ्ग;
    पक्ष Token::TK_StreamEnd:
      OS << "Stream-End: ";
      भङ्ग;
    पक्ष Token::TK_VersionDirective:
      OS << "Version-Directive: ";
      भङ्ग;
    पक्ष Token::TK_TagDirective:
      OS << "Tag-Directive: ";
      भङ्ग;
    पक्ष Token::TK_DocumentStart:
      OS << "Document-Start: ";
      भङ्ग;
    पक्ष Token::TK_DocumentEnd:
      OS << "Document-End: ";
      भङ्ग;
    पक्ष Token::TK_BlockEntry:
      OS << "Block-Entry: ";
      भङ्ग;
    पक्ष Token::TK_BlockEnd:
      OS << "Block-End: ";
      भङ्ग;
    पक्ष Token::TK_BlockSequenceStart:
      OS << "Block-Sequence-Start: ";
      भङ्ग;
    पक्ष Token::TK_BlockMappingStart:
      OS << "Block-Mapping-Start: ";
      भङ्ग;
    पक्ष Token::TK_FlowEntry:
      OS << "Flow-Entry: ";
      भङ्ग;
    पक्ष Token::TK_FlowSequenceStart:
      OS << "Flow-Sequence-Start: ";
      भङ्ग;
    पक्ष Token::TK_FlowSequenceEnd:
      OS << "Flow-Sequence-End: ";
      भङ्ग;
    पक्ष Token::TK_FlowMappingStart:
      OS << "Flow-Mapping-Start: ";
      भङ्ग;
    पक्ष Token::TK_FlowMappingEnd:
      OS << "Flow-Mapping-End: ";
      भङ्ग;
    पक्ष Token::TK_कुञ्चिका:
      OS << "Key: ";
      भङ्ग;
    पक्ष Token::TK_मानम्:
      OS << "Value: ";
      भङ्ग;
    पक्ष Token::TK_Scalar:
      OS << "Scalar: ";
      भङ्ग;
    पक्ष Token::TK_BlockScalar:
      OS << "Block Scalar: ";
      भङ्ग;
    पक्ष Token::TK_Alias:
      OS << "Alias: ";
      भङ्ग;
    पक्ष Token::TK_Anchor:
      OS << "Anchor: ";
      भङ्ग;
    पक्ष Token::TK_Tag:
      OS << "Tag: ";
      भङ्ग;
    पक्ष Token::TK_Error:
      भङ्ग;
    }
    OS << T.Range << "\n";
    सचे (T.Kind == Token::TK_StreamEnd)
      भङ्ग;
    अञ्ञथा सचे (T.Kind == Token::TK_Error)
      पटिनिवत्त मिच्छा;
  }
  पटिनिवत्त सच्च;
}

तार्किक yaml::scanTokens(StringRef निवेशः) {
  SourceMgr SM;
  Scanner scanner(निवेशः, SM);
  याव (सच्च) {
    Token T = scanner.getNext();
    सचे (T.Kind == Token::TK_StreamEnd)
      भङ्ग;
    अञ्ञथा सचे (T.Kind == Token::TK_Error)
      पटिनिवत्त मिच्छा;
  }
  पटिनिवत्त सच्च;
}

std::string yaml::escape(StringRef निवेशः, तार्किक EscapePrintable) {
  std::string Escaped_निवेशः;
  अत्थाय (StringRef::iterator i = निवेशः.begin(), e = निवेशः.end(); i != e; ++i) {
    सचे (*i == '\\')
      Escaped_निवेशः += "\\\\";
    अञ्ञथा सचे (*i == '"')
      Escaped_निवेशः += "\\\"";
    अञ्ञथा सचे (*i == 0)
      Escaped_निवेशः += "\\0";
    अञ्ञथा सचे (*i == 0x07)
      Escaped_निवेशः += "\\a";
    अञ्ञथा सचे (*i == 0x08)
      Escaped_निवेशः += "\\b";
    अञ्ञथा सचे (*i == 0x09)
      Escaped_निवेशः += "\\t";
    अञ्ञथा सचे (*i == 0x0A)
      Escaped_निवेशः += "\\n";
    अञ्ञथा सचे (*i == 0x0B)
      Escaped_निवेशः += "\\v";
    अञ्ञथा सचे (*i == 0x0C)
      Escaped_निवेशः += "\\f";
    अञ्ञथा सचे (*i == 0x0D)
      Escaped_निवेशः += "\\r";
    अञ्ञथा सचे (*i == 0x1B)
      Escaped_निवेशः += "\\e";
    अञ्ञथा सचे ((unsigned वर्ण)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_निवेशः += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } अञ्ञथा सचे (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_मानम्
        = decodeUTF8(StringRef(i, निवेशः.end() - i));
      सचे (Unicode_Scalar_मानम्.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_निवेशः, Val);
        // FIXME: Error reporting.
        पटिनिवत्त Escaped_निवेशः;
      }
      सचे (Unicode_Scalar_मानम्.first == 0x85)
        Escaped_निवेशः += "\\N";
      अञ्ञथा सचे (Unicode_Scalar_मानम्.first == 0xA0)
        Escaped_निवेशः += "\\_";
      अञ्ञथा सचे (Unicode_Scalar_मानम्.first == 0x2028)
        Escaped_निवेशः += "\\L";
      अञ्ञथा सचे (Unicode_Scalar_मानम्.first == 0x2029)
        Escaped_निवेशः += "\\P";
      अञ्ञथा सचे (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_मानम्.first))
        Escaped_निवेशः += StringRef(i, Unicode_Scalar_मानम्.second);
      अञ्ञथा {
        std::string HexStr = utohexstr(Unicode_Scalar_मानम्.first);
        सचे (HexStr.size() <= 2)
          Escaped_निवेशः += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        अञ्ञथा सचे (HexStr.size() <= 4)
          Escaped_निवेशः += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        अञ्ञथा सचे (HexStr.size() <= 8)
          Escaped_निवेशः += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += Unicode_Scalar_मानम्.second - 1;
    } अञ्ञथा {
      Escaped_निवेशः.प्रक्षिप_back(*i);
    }
  }
  पटिनिवत्त Escaped_निवेशः;
}

std::optional<तार्किक> yaml::parseBool(StringRef S) {
  विकल्प (S.size()) {
  पक्ष 1:
    विकल्प (S.front()) {
    पक्ष 'y':
    पक्ष 'Y':
      पटिनिवत्त सच्च;
    पक्ष 'n':
    पक्ष 'N':
      पटिनिवत्त मिच्छा;
    स्वभाव:
      पटिनिवत्त std::nullopt;
    }
  पक्ष 2:
    विकल्प (S.front()) {
    पक्ष 'O':
      सचे (S[1] == 'N') // ON
        पटिनिवत्त सच्च;
      [[fallthrough]];
    पक्ष 'o':
      सचे (S[1] == 'n') //[Oo]n
        पटिनिवत्त सच्च;
      पटिनिवत्त std::nullopt;
    पक्ष 'N':
      सचे (S[1] == 'O') // NO
        पटिनिवत्त मिच्छा;
      [[fallthrough]];
    पक्ष 'n':
      सचे (S[1] == 'o') //[Nn]o
        पटिनिवत्त मिच्छा;
      पटिनिवत्त std::nullopt;
    स्वभाव:
      पटिनिवत्त std::nullopt;
    }
  पक्ष 3:
    विकल्प (S.front()) {
    पक्ष 'O':
      सचे (S.drop_front() == "FF") // OFF
        पटिनिवत्त मिच्छा;
      [[fallthrough]];
    पक्ष 'o':
      सचे (S.drop_front() == "ff") //[Oo]ff
        पटिनिवत्त मिच्छा;
      पटिनिवत्त std::nullopt;
    पक्ष 'Y':
      सचे (S.drop_front() == "ES") // YES
        पटिनिवत्त सच्च;
      [[fallthrough]];
    पक्ष 'y':
      सचे (S.drop_front() == "es") //[Yy]es
        पटिनिवत्त सच्च;
      पटिनिवत्त std::nullopt;
    स्वभाव:
      पटिनिवत्त std::nullopt;
    }
  पक्ष 4:
    विकल्प (S.front()) {
    पक्ष 'T':
      सचे (S.drop_front() == "RUE") // TRUE
        पटिनिवत्त सच्च;
      [[fallthrough]];
    पक्ष 't':
      सचे (S.drop_front() == "rue") //[Tt]rue
        पटिनिवत्त सच्च;
      पटिनिवत्त std::nullopt;
    स्वभाव:
      पटिनिवत्त std::nullopt;
    }
  पक्ष 5:
    विकल्प (S.front()) {
    पक्ष 'F':
      सचे (S.drop_front() == "ALSE") // FALSE
        पटिनिवत्त मिच्छा;
      [[fallthrough]];
    पक्ष 'f':
      सचे (S.drop_front() == "alse") //[Ff]alse
        पटिनिवत्त मिच्छा;
      पटिनिवत्त std::nullopt;
    स्वभाव:
      पटिनिवत्त std::nullopt;
    }
  स्वभाव:
    पटिनिवत्त std::nullopt;
  }
}

Scanner::Scanner(StringRef निवेशः, SourceMgr &sm, तार्किक ShowColors,
                 std::error_code *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(MemoryBufferRef(निवेशः, "YAML"));
}

Scanner::Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, तार्किक ShowColors,
                 std::error_code *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(Buffer);
}

शून्य Scanner::init(MemoryBufferRef Buffer) {
  निवेशः_Buffer = Buffer;
  Current = निवेशः_Buffer.getBufferStart();
  End = निवेशः_Buffer.getBufferEnd();
  Indent = -1;
  Column = 0;
  पङ्क्तिः = 0;
  FlowLevel = 0;
  IsStartOfStream = सच्च;
  Is_Simple_कुञ्चिका_Allowed = सच्च;
  Is_Adjacent_मानम्_Allowed_In_Flow = मिच्छा;
  Failed = मिच्छा;
  std::unique_ptr<MemoryBuffer> निवेशः_Buffer_Owner =
      MemoryBuffer::getMemBuffer(Buffer, /*RequiresNullTerminator=*/मिच्छा);
  SM.AddNewSourceBuffer(std::move(निवेशः_Buffer_Owner), SMLoc());
}

Token &Scanner::peekNext() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  तार्किक NeedMore = मिच्छा;
  याव (सच्च) {
    सचे (TokenQueue.empty() || NeedMore) {
      सचे (!fetchMoreTokens()) {
        TokenQueue.clear();
        Simple_कुञ्चिकाः.clear();
        TokenQueue.प्रक्षिप_back(Token());
        पटिनिवत्त TokenQueue.front();
      }
    }
    प्रतिज्ञा(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_कुञ्चिका_Candidates();
    Simple_कुञ्चिका SK;
    SK.Tok = TokenQueue.begin();
    सचे (!is_contained(Simple_कुञ्चिकाः, SK))
      भङ्ग;
    अञ्ञथा
      NeedMore = सच्च;
  }
  पटिनिवत्त TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  सचे (!TokenQueue.empty())
    TokenQueue.निष्कासय_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  सचे (TokenQueue.empty())
    TokenQueue.resetAlloc();

  पटिनिवत्त Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  सचे (Position == End)
    पटिनिवत्त Position;
  // Check 7 bit c-printable - b-char.
  सचे (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    पटिनिवत्त Position + 1;

  // Check for valid UTF-8.
  सचे (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    सचे (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      पटिनिवत्त Position + u8d.second;
  }
  पटिनिवत्त Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  सचे (Position == End)
    पटिनिवत्त Position;
  सचे (*Position == 0x0D) {
    सचे (Position + 1 != End && *(Position + 1) == 0x0A)
      पटिनिवत्त Position + 2;
    पटिनिवत्त Position + 1;
  }

  सचे (*Position == 0x0A)
    पटिनिवत्त Position + 1;
  पटिनिवत्त Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  सचे (Position == End)
    पटिनिवत्त Position;
  सचे (*Position == ' ')
    पटिनिवत्त Position + 1;
  पटिनिवत्त Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  सचे (Position == End)
    पटिनिवत्त Position;
  सचे (*Position == ' ' || *Position == '\t')
    पटिनिवत्त Position + 1;
  पटिनिवत्त Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  सचे (Position == End)
    पटिनिवत्त Position;
  सचे (*Position == ' ' || *Position == '\t')
    पटिनिवत्त Position;
  पटिनिवत्त skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  याव (सच्च) {
    StringRef::iterator i = (एतत्->*Func)(Position);
    सचे (i == Position)
      भङ्ग;
    Position = i;
  }
  पटिनिवत्त Position;
}

शून्य Scanner::advanceWhile(SkipWhileFunc Func) {
  auto Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

स्थिर तार्किक is_ns_hex_digit(ध्रुव वर्ण C) { पटिनिवत्त isAlnum(C); }

स्थिर तार्किक is_ns_word_char(ध्रुव वर्ण C) { पटिनिवत्त C == '-' || isAlpha(C); }

शून्य Scanner::scan_ns_uri_char() {
  याव (सच्च) {
    सचे (Current == End)
      भङ्ग;
    सचे ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } अञ्ञथा {
      भङ्ग;
    }
  }
}

तार्किक Scanner::consume(uint32_t Expected) {
  सचे (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    पटिनिवत्त मिच्छा;
  }
  सचे (Current == End)
    पटिनिवत्त मिच्छा;
  सचे (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    पटिनिवत्त मिच्छा;
  }
  सचे (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    पटिनिवत्त सच्च;
  }
  पटिनिवत्त मिच्छा;
}

शून्य Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  प्रतिज्ञा(Current <= End && "Skipped past the end");
}

तार्किक Scanner::isBlankOrBreak(StringRef::iterator Position) {
  सचे (Position == End)
    पटिनिवत्त मिच्छा;
  पटिनिवत्त *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

तार्किक Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  सचे (Position == End || isBlankOrBreak(Position))
    पटिनिवत्त मिच्छा;
  सचे (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    पटिनिवत्त मिच्छा;
  पटिनिवत्त सच्च;
}

तार्किक Scanner::is_पङ्क्तिः_Empty(StringRef पङ्क्तिः) {
  अत्थाय (ध्रुव auto *Position = पङ्क्तिः.begin(); Position != पङ्क्तिः.end(); ++Position)
    सचे (!isBlankOrBreak(Position))
      पटिनिवत्त मिच्छा;
  पटिनिवत्त सच्च;
}

तार्किक Scanner::consume_पङ्क्तिः_Break_If_Present() {
  auto Next = skip_b_break(Current);
  सचे (Next == Current)
    पटिनिवत्त मिच्छा;
  Column = 0;
  ++पङ्क्तिः;
  Current = Next;
  पटिनिवत्त सच्च;
}

शून्य Scanner::save_Simple_कुञ्चिका_Candidate( TokenQueueT::iterator Tok
                                    , unsigned AtColumn
                                    , तार्किक IsRequired) {
  सचे (Is_Simple_कुञ्चिका_Allowed) {
    Simple_कुञ्चिका SK;
    SK.Tok = Tok;
    SK.पङ्क्तिः = पङ्क्तिः;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_कुञ्चिकाः.प्रक्षिप_back(SK);
  }
}

शून्य Scanner::remove_Stale_Simple_कुञ्चिका_Candidates() {
  अत्थाय (SmallVectorImpl<Simple_कुञ्चिका>::iterator i = Simple_कुञ्चिकाः.begin();
                                            i != Simple_कुञ्चिकाः.end();) {
    सचे (i->पङ्क्तिः != पङ्क्तिः || i->Column + 1024 < Column) {
      सचे (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_कुञ्चिकाः.erase(i);
    } अञ्ञथा {
      ++i;
    }
  }
}

शून्य Scanner::remove_Simple_कुञ्चिका_Candidates_On_Flow_Level(unsigned Level) {
  सचे (!Simple_कुञ्चिकाः.empty() && (Simple_कुञ्चिकाः.end() - 1)->FlowLevel == Level)
    Simple_कुञ्चिकाः.निष्कासय_back();
}

तार्किक Scanner::unrollIndent(पूर्णाङ्क ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  सचे (FlowLevel != 0)
    पटिनिवत्त सच्च;

  याव (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.प्रक्षिप_back(T);
    Indent = Indents.निष्कासय_back_val();
  }

  पटिनिवत्त सच्च;
}

तार्किक Scanner::rollIndent( पूर्णाङ्क ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator निवेशय_Point) {
  सचे (FlowLevel)
    पटिनिवत्त सच्च;
  सचे (Indent < ToColumn) {
    Indents.प्रक्षिप_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.निवेशय(निवेशय_Point, T);
  }
  पटिनिवत्त सच्च;
}

शून्य Scanner::skipComment() {
  सचे (Current == End || *Current != '#')
    पटिनिवत्त;
  याव (सच्च) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    सचे (I == Current)
      भङ्ग;
    Current = I;
    ++Column;
  }
}

शून्य Scanner::scanToNextToken() {
  याव (सच्च) {
    याव (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    सचे (i == Current)
      भङ्ग;
    Current = i;
    ++पङ्क्तिः;
    Column = 0;
    // New lines may start a simple key.
    सचे (!FlowLevel)
      Is_Simple_कुञ्चिका_Allowed = सच्च;
  }
}

तार्किक Scanner::scanStreamStart() {
  IsStartOfStream = मिच्छा;

  EncodingInfo EI = getUnicodeEncoding(current_निवेशः());

  Token T;
  T.Kind = Token::TK_StreamStart;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.प्रक्षिप_back(T);
  Current += EI.second;
  पटिनिवत्त सच्च;
}

तार्किक Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  सचे (Column != 0) {
    Column = 0;
    ++पङ्क्तिः;
  }

  unrollIndent(-1);
  Simple_कुञ्चिकाः.clear();
  Is_Simple_कुञ्चिका_Allowed = मिच्छा;
  Is_Adjacent_मानम्_Allowed_In_Flow = मिच्छा;

  Token T;
  T.Kind = Token::TK_StreamEnd;
  T.Range = StringRef(Current, 0);
  TokenQueue.प्रक्षिप_back(T);
  पटिनिवत्त सच्च;
}

तार्किक Scanner::scanDirective() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_कुञ्चिकाः.clear();
  Is_Simple_कुञ्चिका_Allowed = मिच्छा;
  Is_Adjacent_मानम्_Allowed_In_Flow = मिच्छा;

  StringRef::iterator Start = Current;
  consume('%');
  StringRef::iterator NameStart = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(NameStart, Current - NameStart);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  सचे (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.प्रक्षिप_back(T);
    पटिनिवत्त सच्च;
  } अञ्ञथा सचे(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.प्रक्षिप_back(T);
    पटिनिवत्त सच्च;
  }
  पटिनिवत्त मिच्छा;
}

तार्किक Scanner::scanDocumentIndicator(तार्किक IsStart) {
  unrollIndent(-1);
  Simple_कुञ्चिकाः.clear();
  Is_Simple_कुञ्चिका_Allowed = मिच्छा;
  Is_Adjacent_मानम्_Allowed_In_Flow = मिच्छा;

  Token T;
  T.Kind = IsStart ? Token::TK_DocumentStart : Token::TK_DocumentEnd;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.प्रक्षिप_back(T);
  पटिनिवत्त सच्च;
}

तार्किक Scanner::scanFlowCollectionStart(तार्किक IsSequence) {
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.प्रक्षिप_back(T);

  // [ and { may begin a simple key.
  save_Simple_कुञ्चिका_Candidate(--TokenQueue.end(), Column - 1, मिच्छा);

  // And may also be followed by a simple key.
  Is_Simple_कुञ्चिका_Allowed = सच्च;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_मानम्_Allowed_In_Flow = मिच्छा;
  ++FlowLevel;
  पटिनिवत्त सच्च;
}

तार्किक Scanner::scanFlowCollectionEnd(तार्किक IsSequence) {
  remove_Simple_कुञ्चिका_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_कुञ्चिका_Allowed = मिच्छा;
  Is_Adjacent_मानम्_Allowed_In_Flow = सच्च;
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceEnd
                      : Token::TK_FlowMappingEnd;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.प्रक्षिप_back(T);
  सचे (FlowLevel)
    --FlowLevel;
  पटिनिवत्त सच्च;
}

तार्किक Scanner::scanFlowEntry() {
  remove_Simple_कुञ्चिका_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_कुञ्चिका_Allowed = सच्च;
  Is_Adjacent_मानम्_Allowed_In_Flow = मिच्छा;
  Token T;
  T.Kind = Token::TK_FlowEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.प्रक्षिप_back(T);
  पटिनिवत्त सच्च;
}

तार्किक Scanner::scanBlockEntry() {
  rollIndent(Column, Token::TK_BlockSequenceStart, TokenQueue.end());
  remove_Simple_कुञ्चिका_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_कुञ्चिका_Allowed = सच्च;
  Is_Adjacent_मानम्_Allowed_In_Flow = मिच्छा;
  Token T;
  T.Kind = Token::TK_BlockEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.प्रक्षिप_back(T);
  पटिनिवत्त सच्च;
}

तार्किक Scanner::scan_कुञ्चिका() {
  सचे (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  remove_Simple_कुञ्चिका_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_कुञ्चिका_Allowed = !FlowLevel;
  Is_Adjacent_मानम्_Allowed_In_Flow = मिच्छा;

  Token T;
  T.Kind = Token::TK_कुञ्चिका;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.प्रक्षिप_back(T);
  पटिनिवत्त सच्च;
}

तार्किक Scanner::scan_मानम्() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  सचे (!Simple_कुञ्चिकाः.empty()) {
    Simple_कुञ्चिका SK = Simple_कुञ्चिकाः.निष्कासय_back_val();
    Token T;
    T.Kind = Token::TK_कुञ्चिका;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    अत्थाय (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      सचे (i == SK.Tok)
        भङ्ग;
    }
    सचे (i == e) {
      Failed = सच्च;
      पटिनिवत्त मिच्छा;
    }
    i = TokenQueue.निवेशय(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    Is_Simple_कुञ्चिका_Allowed = मिच्छा;
  } अञ्ञथा {
    सचे (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    Is_Simple_कुञ्चिका_Allowed = !FlowLevel;
  }
  Is_Adjacent_मानम्_Allowed_In_Flow = मिच्छा;

  Token T;
  T.Kind = Token::TK_मानम्;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.प्रक्षिप_back(T);
  पटिनिवत्त सच्च;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE स्थिर तार्किक
wasEscaped(StringRef::iterator First, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
स्थिर तार्किक wasEscaped(StringRef::iterator First,
                       StringRef::iterator Position) {
  प्रतिज्ञा(Position - 1 >= First);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  याव (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  पटिनिवत्त (Position - 1 - I) % 2 == 1;
}

तार्किक Scanner::scanFlowScalar(तार्किक IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  सचे (IsDoubleQuoted) {
    करोहि {
      ++Current;
      याव (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } याव (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } अञ्ञथा {
    skip(1);
    याव (Current != End) {
      // Skip a ' followed by another '.
      सचे (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        अनुवर्त;
      } अञ्ञथा सचे (*Current == '\'')
        भङ्ग;
      StringRef::iterator i = skip_nb_char(Current);
      सचे (i == Current) {
        i = skip_b_break(Current);
        सचे (i == Current)
          भङ्ग;
        Current = i;
        Column = 0;
        ++पङ्क्तिः;
      } अञ्ञथा {
        सचे (i == End)
          भङ्ग;
        Current = i;
        ++Column;
      }
    }
  }

  सचे (Current == End) {
    setError("Expected quote at end of scalar", Current);
    पटिनिवत्त मिच्छा;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.प्रक्षिप_back(T);

  save_Simple_कुञ्चिका_Candidate(--TokenQueue.end(), ColStart, मिच्छा);

  Is_Simple_कुञ्चिका_Allowed = मिच्छा;
  Is_Adjacent_मानम्_Allowed_In_Flow = सच्च;

  पटिनिवत्त सच्च;
}

तार्किक Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  unsigned LeadingBlanks = 0;
  प्रतिज्ञा(Indent >= -1 && "Indent must be >= -1 !");
  unsigned indent = static_cast<unsigned>(Indent + 1);
  याव (Current != End) {
    सचे (*Current == '#')
      भङ्ग;

    याव (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      सचे (i == Current)
        भङ्ग;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    सचे (!isBlankOrBreak(Current))
      भङ्ग;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    याव (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      सचे (i != Tmp) {
        सचे (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          पटिनिवत्त मिच्छा;
        }
        Tmp = i;
        ++Column;
      } अञ्ञथा {
        i = skip_b_break(Tmp);
        सचे (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++पङ्क्तिः;
      }
    }

    सचे (!FlowLevel && Column < indent)
      भङ्ग;

    Current = Tmp;
  }
  सचे (Start == Current) {
    setError("Got empty plain scalar", Start);
    पटिनिवत्त मिच्छा;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.प्रक्षिप_back(T);

  // Plain scalars can be simple keys.
  save_Simple_कुञ्चिका_Candidate(--TokenQueue.end(), ColStart, मिच्छा);

  Is_Simple_कुञ्चिका_Allowed = मिच्छा;
  Is_Adjacent_मानम्_Allowed_In_Flow = मिच्छा;

  पटिनिवत्त सच्च;
}

तार्किक Scanner::scanAliasOrAnchor(तार्किक IsAlias) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1);
  याव (Current != End) {
    सचे (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      भङ्ग;
    StringRef::iterator i = skip_ns_char(Current);
    सचे (i == Current)
      भङ्ग;
    Current = i;
    ++Column;
  }

  सचे (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    पटिनिवत्त मिच्छा;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.प्रक्षिप_back(T);

  // Alias and anchors can be simple keys.
  save_Simple_कुञ्चिका_Candidate(--TokenQueue.end(), ColStart, मिच्छा);

  Is_Simple_कुञ्चिका_Allowed = मिच्छा;
  Is_Adjacent_मानम्_Allowed_In_Flow = मिच्छा;

  पटिनिवत्त सच्च;
}

तार्किक Scanner::scanBlockScalarIndicators(वर्ण &StyleIndicator,
                                        वर्ण &ChompingIndicator,
                                        unsigned &IndentIndicator,
                                        तार्किक &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  सचे (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    पटिनिवत्त मिच्छा;
  पटिनिवत्त सच्च;
}

वर्ण Scanner::scanBlockStyleIndicator() {
  वर्ण Indicator = ' ';
  सचे (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  पटिनिवत्त Indicator;
}

वर्ण Scanner::scanBlockChompingIndicator() {
  वर्ण Indicator = ' ';
  सचे (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  पटिनिवत्त Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
स्थिर unsigned get_Chomped_पङ्क्तिः_Breaks(वर्ण ChompingIndicator,
                                     unsigned पङ्क्तिः_Breaks, StringRef Str) {
  सचे (ChompingIndicator == '-') // Strip all line breaks.
    पटिनिवत्त 0;
  सचे (ChompingIndicator == '+') // Keep all line breaks.
    पटिनिवत्त पङ्क्तिः_Breaks;
  // Clip trailing lines.
  पटिनिवत्त Str.empty() ? 0 : 1;
}

unsigned Scanner::scanBlockIndentationIndicator() {
  unsigned Indent = 0;
  सचे (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = unsigned(*Current - '0');
    skip(1);
  }
  पटिनिवत्त Indent;
}

तार्किक Scanner::scanBlockScalarHeader(वर्ण &ChompingIndicator,
                                    unsigned &IndentIndicator, तार्किक &IsDone) {
  auto Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  सचे (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  सचे (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.प्रक्षिप_back(T);
    IsDone = सच्च;
    पटिनिवत्त सच्च;
  }

  सचे (!consume_पङ्क्तिः_Break_If_Present()) {
    setError("Expected a line break after block scalar header", Current);
    पटिनिवत्त मिच्छा;
  }
  पटिनिवत्त सच्च;
}

तार्किक Scanner::findBlockScalarIndent(unsigned &BlockIndent,
                                    unsigned BlockExitIndent,
                                    unsigned &पङ्क्तिः_Breaks, तार्किक &IsDone) {
  unsigned Max_All_Space_पङ्क्तिः_Characters = 0;
  StringRef::iterator Longest_All_Space_पङ्क्तिः;

  याव (सच्च) {
    advanceWhile(&Scanner::skip_s_space);
    सचे (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      सचे (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = सच्च;
        पटिनिवत्त सच्च;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      सचे (Max_All_Space_पङ्क्तिः_Characters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_पङ्क्तिः);
        पटिनिवत्त मिच्छा;
      }
      पटिनिवत्त सच्च;
    }
    सचे (skip_b_break(Current) != Current &&
        Column > Max_All_Space_पङ्क्तिः_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_पङ्क्तिः_Characters = Column;
      Longest_All_Space_पङ्क्तिः = Current;
    }

    // Check for EOF.
    सचे (Current == End) {
      IsDone = सच्च;
      पटिनिवत्त सच्च;
    }

    सचे (!consume_पङ्क्तिः_Break_If_Present()) {
      IsDone = सच्च;
      पटिनिवत्त सच्च;
    }
    ++पङ्क्तिः_Breaks;
  }
  पटिनिवत्त सच्च;
}

तार्किक Scanner::scanBlockScalarIndent(unsigned BlockIndent,
                                    unsigned BlockExitIndent, तार्किक &IsDone) {
  // Skip the indentation.
  याव (Column < BlockIndent) {
    auto I = skip_s_space(Current);
    सचे (I == Current)
      भङ्ग;
    Current = I;
    ++Column;
  }

  सचे (skip_nb_char(Current) == Current)
    पटिनिवत्त सच्च;

  सचे (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = सच्च;
    पटिनिवत्त सच्च;
  }

  सचे (Column < BlockIndent) {
    सचे (Current != End && *Current == '#') { // Trailing comment.
      IsDone = सच्च;
      पटिनिवत्त सच्च;
    }
    setError("A text line is less indented than the block scalar", Current);
    पटिनिवत्त मिच्छा;
  }
  पटिनिवत्त सच्च; // A normal text line.
}

तार्किक Scanner::scanBlockScalar(तार्किक IsLiteral) {
  प्रतिज्ञा(*Current == '|' || *Current == '>');
  वर्ण StyleIndicator;
  वर्ण ChompingIndicator;
  unsigned BlockIndent;
  तार्किक IsDone = मिच्छा;
  सचे (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    पटिनिवत्त मिच्छा;
  सचे (IsDone)
    पटिनिवत्त सच्च;
  तार्किक IsFolded = StyleIndicator == '>';

  ध्रुव auto *Start = Current;
  unsigned BlockExitIndent = Indent < 0 ? 0 : (unsigned)Indent;
  unsigned पङ्क्तिः_Breaks = 0;
  सचे (BlockIndent == 0) {
    सचे (!findBlockScalarIndent(BlockIndent, BlockExitIndent, पङ्क्तिः_Breaks,
                               IsDone))
      पटिनिवत्त मिच्छा;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  याव (!IsDone) {
    सचे (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      पटिनिवत्त मिच्छा;
    सचे (IsDone)
      भङ्ग;

    // Parse the current line.
    auto पङ्क्तिः_Start = Current;
    advanceWhile(&Scanner::skip_nb_char);
    सचे (पङ्क्तिः_Start != Current) {
      सचे (पङ्क्तिः_Breaks && IsFolded && !Scanner::is_पङ्क्तिः_Empty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        सचे (पङ्क्तिः_Breaks == 1) {
          Str.append(पङ्क्तिः_Breaks,
                     is_पङ्क्तिः_Empty(StringRef(पङ्क्तिः_Start, Current - पङ्क्तिः_Start))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        पङ्क्तिः_Breaks--;
      }
      Str.append(पङ्क्तिः_Breaks, '\n');
      Str.append(StringRef(पङ्क्तिः_Start, Current - पङ्क्तिः_Start));
      पङ्क्तिः_Breaks = 0;
    }

    // Check for EOF.
    सचे (Current == End)
      भङ्ग;

    सचे (!consume_पङ्क्तिः_Break_If_Present())
      भङ्ग;
    ++पङ्क्तिः_Breaks;
  }

  सचे (Current == End && !पङ्क्तिः_Breaks)
    // Ensure that there is at least one line break before the end of file.
    पङ्क्तिः_Breaks = 1;
  Str.append(get_Chomped_पङ्क्तिः_Breaks(ChompingIndicator, पङ्क्तिः_Breaks, Str), '\n');

  // New lines may start a simple key.
  सचे (!FlowLevel)
    Is_Simple_कुञ्चिका_Allowed = सच्च;
  Is_Adjacent_मानम्_Allowed_In_Flow = मिच्छा;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.मानम् = std::string(Str);
  TokenQueue.प्रक्षिप_back(T);
  पटिनिवत्त सच्च;
}

तार्किक Scanner::scanTag() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1); // Eat !.
  सचे (Current == End || isBlankOrBreak(Current)); // An empty tag.
  अञ्ञथा सचे (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    सचे (!consume('>'))
      पटिनिवत्त मिच्छा;
  } अञ्ञथा {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.Kind = Token::TK_Tag;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.प्रक्षिप_back(T);

  // Tags can be simple keys.
  save_Simple_कुञ्चिका_Candidate(--TokenQueue.end(), ColStart, मिच्छा);

  Is_Simple_कुञ्चिका_Allowed = मिच्छा;
  Is_Adjacent_मानम्_Allowed_In_Flow = मिच्छा;

  पटिनिवत्त सच्च;
}

तार्किक Scanner::fetchMoreTokens() {
  सचे (IsStartOfStream)
    पटिनिवत्त scanStreamStart();

  scanToNextToken();

  सचे (Current == End)
    पटिनिवत्त scanStreamEnd();

  remove_Stale_Simple_कुञ्चिका_Candidates();

  unrollIndent(Column);

  सचे (Column == 0 && *Current == '%')
    पटिनिवत्त scanDirective();

  सचे (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    पटिनिवत्त scanDocumentIndicator(सच्च);

  सचे (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    पटिनिवत्त scanDocumentIndicator(मिच्छा);

  सचे (*Current == '[')
    पटिनिवत्त scanFlowCollectionStart(सच्च);

  सचे (*Current == '{')
    पटिनिवत्त scanFlowCollectionStart(मिच्छा);

  सचे (*Current == ']')
    पटिनिवत्त scanFlowCollectionEnd(सच्च);

  सचे (*Current == '}')
    पटिनिवत्त scanFlowCollectionEnd(मिच्छा);

  सचे (*Current == ',')
    पटिनिवत्त scanFlowEntry();

  सचे (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    पटिनिवत्त scanBlockEntry();

  सचे (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    पटिनिवत्त scan_कुञ्चिका();

  सचे (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || Is_Adjacent_मानम्_Allowed_In_Flow))
    पटिनिवत्त scan_मानम्();

  सचे (*Current == '*')
    पटिनिवत्त scanAliasOrAnchor(सच्च);

  सचे (*Current == '&')
    पटिनिवत्त scanAliasOrAnchor(मिच्छा);

  सचे (*Current == '!')
    पटिनिवत्त scanTag();

  सचे (*Current == '|' && !FlowLevel)
    पटिनिवत्त scanBlockScalar(सच्च);

  सचे (*Current == '>' && !FlowLevel)
    पटिनिवत्त scanBlockScalar(मिच्छा);

  सचे (*Current == '\'')
    पटिनिवत्त scanFlowScalar(मिच्छा);

  सचे (*Current == '"')
    पटिनिवत्त scanFlowScalar(सच्च);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  सचे ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    पटिनिवत्त scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  पटिनिवत्त मिच्छा;
}

Stream::Stream(StringRef निवेशः, SourceMgr &SM, तार्किक ShowColors,
               std::error_code *EC)
    : scanner(नूतन Scanner(निवेशः, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef निवेशः_Buffer, SourceMgr &SM, तार्किक ShowColors,
               std::error_code *EC)
    : scanner(नूतन Scanner(निवेशः_Buffer, SM, ShowColors, EC)) {}

Stream::~Stream() = स्वभाव;

तार्किक Stream::failed() { पटिनिवत्त scanner->failed(); }

शून्य Stream::printError(ग्रन्थिः *N, ध्रुव Twine &सन्देशः_2, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), सन्देशः_2, Kind);
}

शून्य Stream::printError(ध्रुव SMRange &Range, ध्रुव Twine &सन्देशः_2,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, सन्देशः_2, Range);
}

document_iterator Stream::begin() {
  सचे (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(नूतन Document(*एतत्));
  पटिनिवत्त document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  पटिनिवत्त document_iterator();
}

शून्य Stream::skip() {
  अत्थाय (Document &Doc : *एतत्)
    Doc.skip();
}

ग्रन्थिः::ग्रन्थिः(unsigned पूर्णाङ्क Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string ग्रन्थिः::getVerbatimTag() ध्रुव {
  StringRef Raw = getRawTag();
  सचे (!Raw.empty() && Raw != "!") {
    std::string Ret;
    सचे (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      पटिनिवत्त Ret;
    } अञ्ञथा सचे (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      पटिनिवत्त Ret;
    } अञ्ञथा {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      सचे (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      अञ्ञथा {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      पटिनिवत्त Ret;
    }
  }

  विकल्प (getType()) {
  पक्ष NK_Null:
    पटिनिवत्त "tag:yaml.org,2002:null";
  पक्ष NK_Scalar:
  पक्ष NK_BlockScalar:
    // TODO: Tag resolution.
    पटिनिवत्त "tag:yaml.org,2002:str";
  पक्ष NK_Mapping:
    पटिनिवत्त "tag:yaml.org,2002:map";
  पक्ष NK_Sequence:
    पटिनिवत्त "tag:yaml.org,2002:seq";
  }

  पटिनिवत्त "";
}

Token &ग्रन्थिः::peekNext() {
  पटिनिवत्त Doc->peekNext();
}

Token ग्रन्थिः::getNext() {
  पटिनिवत्त Doc->getNext();
}

ग्रन्थिः *ग्रन्थिः::parse_Block_ग्रन्थिः() {
  पटिनिवत्त Doc->parse_Block_ग्रन्थिः();
}

BumpPtrAllocator &ग्रन्थिः::getAllocator() {
  पटिनिवत्त Doc->ग्रन्थिः_Allocator;
}

शून्य ग्रन्थिः::setError(ध्रुव Twine &सन्देशः_2, Token &Tok) ध्रुव {
  Doc->setError(सन्देशः_2, Tok);
}

तार्किक ग्रन्थिः::failed() ध्रुव {
  पटिनिवत्त Doc->failed();
}

StringRef Scalar_ग्रन्थिः::get_मानम्(SmallVectorImpl<वर्ण> &Storage) ध्रुव {
  सचे (मानम्[0] == '"')
    पटिनिवत्त get_Double_Quoted_मानम्(मानम्, Storage);
  सचे (मानम्[0] == '\'')
    पटिनिवत्त get_Single_Quoted_मानम्(मानम्, Storage);
  पटिनिवत्त get_Plain_मानम्(मानम्, Storage);
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
स्थिर StringRef
parse_Scalar_मानम्(StringRef Unquoted_मानम्, SmallVectorImpl<वर्ण> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<वर्ण> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_मानम्.find_first_of(LookupChars);
  सचे (I == StringRef::npos)
    पटिनिवत्त Unquoted_मानम्;

  Storage.clear();
  Storage.reserve(Unquoted_मानम्.size());
  वर्ण Last_New_पङ्क्तिः_Added_As = '\0';
  अत्थाय (; I != StringRef::npos; I = Unquoted_मानम्.find_first_of(LookupChars)) {
    सचे (Unquoted_मानम्[I] != '\r' && Unquoted_मानम्[I] != '\n') {
      llvm::append_range(Storage, Unquoted_मानम्.take_front(I));
      Unquoted_मानम् = UnescapeCallback(Unquoted_मानम्.drop_front(I), Storage);
      Last_New_पङ्क्तिः_Added_As = '\0';
      अनुवर्त;
    }
    सचे (size_t LastNonSWhite = Unquoted_मानम्.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_मानम्.take_front(LastNonSWhite + 1));
      Storage.प्रक्षिप_back(' ');
      Last_New_पङ्क्तिः_Added_As = ' ';
    } अञ्ञथा {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      विकल्प (Last_New_पङ्क्तिः_Added_As) {
      पक्ष ' ':
        प्रतिज्ञा(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        Last_New_पङ्क्तिः_Added_As = '\n';
        भङ्ग;
      पक्ष '\n':
        प्रतिज्ञा(!Storage.empty() && Storage.back() == '\n');
        Storage.प्रक्षिप_back('\n');
        भङ्ग;
      स्वभाव:
        Storage.प्रक्षिप_back(' ');
        Last_New_पङ्क्तिः_Added_As = ' ';
        भङ्ग;
      }
    }
    // Handle Windows-style EOL
    सचे (Unquoted_मानम्.substr(I, 2) == "\r\n")
      I++;
    Unquoted_मानम् = Unquoted_मानम्.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_मानम्);
  पटिनिवत्त StringRef(Storage.begin(), Storage.size());
}

StringRef
Scalar_ग्रन्थिः::get_Double_Quoted_मानम्(StringRef Raw_मानम्,
                                 SmallVectorImpl<वर्ण> &Storage) ध्रुव {
  प्रतिज्ञा(Raw_मानम्.size() >= 2 && Raw_मानम्.front() == '"' &&
         Raw_मानम्.back() == '"');
  StringRef Unquoted_मानम् = Raw_मानम्.substr(1, Raw_मानम्.size() - 2);

  auto UnescapeFunc = [एतत्](StringRef Unquoted_मानम्,
                             SmallVectorImpl<वर्ण> &Storage) {
    प्रतिज्ञा(Unquoted_मानम्.take_front(1) == "\\");
    सचे (Unquoted_मानम्.size() == 1) {
      Token T;
      T.Range = Unquoted_मानम्;
      setError("Unrecognized escape code", T);
      Storage.clear();
      पटिनिवत्त StringRef();
    }
    Unquoted_मानम् = Unquoted_मानम्.drop_front(1);
    विकल्प (Unquoted_मानम्[0]) {
    स्वभाव: {
      Token T;
      T.Range = Unquoted_मानम्.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      पटिनिवत्त StringRef();
    }
    पक्ष '\r':
      // Shrink the Windows-style EOL.
      सचे (Unquoted_मानम्.size() >= 2 && Unquoted_मानम्[1] == '\n')
        Unquoted_मानम् = Unquoted_मानम्.drop_front(1);
      [[fallthrough]];
    पक्ष '\n':
      पटिनिवत्त Unquoted_मानम्.drop_front(1).ltrim(" \t");
    पक्ष '0':
      Storage.प्रक्षिप_back(0x00);
      भङ्ग;
    पक्ष 'a':
      Storage.प्रक्षिप_back(0x07);
      भङ्ग;
    पक्ष 'b':
      Storage.प्रक्षिप_back(0x08);
      भङ्ग;
    पक्ष 't':
    पक्ष 0x09:
      Storage.प्रक्षिप_back(0x09);
      भङ्ग;
    पक्ष 'n':
      Storage.प्रक्षिप_back(0x0A);
      भङ्ग;
    पक्ष 'v':
      Storage.प्रक्षिप_back(0x0B);
      भङ्ग;
    पक्ष 'f':
      Storage.प्रक्षिप_back(0x0C);
      भङ्ग;
    पक्ष 'r':
      Storage.प्रक्षिप_back(0x0D);
      भङ्ग;
    पक्ष 'e':
      Storage.प्रक्षिप_back(0x1B);
      भङ्ग;
    पक्ष ' ':
      Storage.प्रक्षिप_back(0x20);
      भङ्ग;
    पक्ष '"':
      Storage.प्रक्षिप_back(0x22);
      भङ्ग;
    पक्ष '/':
      Storage.प्रक्षिप_back(0x2F);
      भङ्ग;
    पक्ष '\\':
      Storage.प्रक्षिप_back(0x5C);
      भङ्ग;
    पक्ष 'N':
      encodeUTF8(0x85, Storage);
      भङ्ग;
    पक्ष '_':
      encodeUTF8(0xA0, Storage);
      भङ्ग;
    पक्ष 'L':
      encodeUTF8(0x2028, Storage);
      भङ्ग;
    पक्ष 'P':
      encodeUTF8(0x2029, Storage);
      भङ्ग;
    पक्ष 'x': {
      सचे (Unquoted_मानम्.size() < 3)
        // TODO: Report error.
        भङ्ग;
      unsigned पूर्णाङ्क Unicode_Scalar_मानम्;
      सचे (Unquoted_मानम्.substr(1, 2).getAsInteger(16, Unicode_Scalar_मानम्))
        // TODO: Report error.
        Unicode_Scalar_मानम् = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मानम्, Storage);
      पटिनिवत्त Unquoted_मानम्.drop_front(3);
    }
    पक्ष 'u': {
      सचे (Unquoted_मानम्.size() < 5)
        // TODO: Report error.
        भङ्ग;
      unsigned पूर्णाङ्क Unicode_Scalar_मानम्;
      सचे (Unquoted_मानम्.substr(1, 4).getAsInteger(16, Unicode_Scalar_मानम्))
        // TODO: Report error.
        Unicode_Scalar_मानम् = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मानम्, Storage);
      पटिनिवत्त Unquoted_मानम्.drop_front(5);
    }
    पक्ष 'U': {
      सचे (Unquoted_मानम्.size() < 9)
        // TODO: Report error.
        भङ्ग;
      unsigned पूर्णाङ्क Unicode_Scalar_मानम्;
      सचे (Unquoted_मानम्.substr(1, 8).getAsInteger(16, Unicode_Scalar_मानम्))
        // TODO: Report error.
        Unicode_Scalar_मानम् = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मानम्, Storage);
      पटिनिवत्त Unquoted_मानम्.drop_front(9);
    }
    }
    पटिनिवत्त Unquoted_मानम्.drop_front(1);
  };

  पटिनिवत्त parse_Scalar_मानम्(Unquoted_मानम्, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_ग्रन्थिः::get_Single_Quoted_मानम्(StringRef Raw_मानम्,
                                           SmallVectorImpl<वर्ण> &Storage) {
  प्रतिज्ञा(Raw_मानम्.size() >= 2 && Raw_मानम्.front() == '\'' &&
         Raw_मानम्.back() == '\'');
  StringRef Unquoted_मानम् = Raw_मानम्.substr(1, Raw_मानम्.size() - 2);

  auto UnescapeFunc = [](StringRef Unquoted_मानम्,
                         SmallVectorImpl<वर्ण> &Storage) {
    प्रतिज्ञा(Unquoted_मानम्.take_front(2) == "''");
    Storage.प्रक्षिप_back('\'');
    पटिनिवत्त Unquoted_मानम्.drop_front(2);
  };

  पटिनिवत्त parse_Scalar_मानम्(Unquoted_मानम्, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_ग्रन्थिः::get_Plain_मानम्(StringRef Raw_मानम्,
                                    SmallVectorImpl<वर्ण> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_मानम् = Raw_मानम्.rtrim("\r\n \t");
  पटिनिवत्त parse_Scalar_मानम्(Raw_मानम्, Storage, "\r\n", nullptr);
}

ग्रन्थिः *कुञ्चिका_मानम्_ग्रन्थिः::get_कुञ्चिका() {
  सचे (कुञ्चिका)
    पटिनिवत्त कुञ्चिका;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    सचे (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_मानम्
        || t.Kind == Token::TK_Error) {
      पटिनिवत्त कुञ्चिका = नूतन (getAllocator()) Null_ग्रन्थिः(Doc);
    }
    सचे (t.Kind == Token::TK_कुञ्चिका)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  सचे (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_मानम्) {
    पटिनिवत्त कुञ्चिका = नूतन (getAllocator()) Null_ग्रन्थिः(Doc);
  }

  // We've got a normal key.
  पटिनिवत्त कुञ्चिका = parse_Block_ग्रन्थिः();
}

ग्रन्थिः *कुञ्चिका_मानम्_ग्रन्थिः::get_मानम्() {
  सचे (मानम्)
    पटिनिवत्त मानम्;

  सचे (ग्रन्थिः* कुञ्चिका = get_कुञ्चिका())
    कुञ्चिका->skip();
  अञ्ञथा {
    setError("Null key in Key Value.", peekNext());
    पटिनिवत्त मानम् = नूतन (getAllocator()) Null_ग्रन्थिः(Doc);
  }

  सचे (failed())
    पटिनिवत्त मानम् = नूतन (getAllocator()) Null_ग्रन्थिः(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    सचे (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_कुञ्चिका
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      पटिनिवत्त मानम् = नूतन (getAllocator()) Null_ग्रन्थिः(Doc);
    }

    सचे (t.Kind != Token::TK_मानम्) {
      setError("Unexpected token in Key Value.", t);
      पटिनिवत्त मानम् = नूतन (getAllocator()) Null_ग्रन्थिः(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  सचे (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_कुञ्चिका) {
    पटिनिवत्त मानम् = नूतन (getAllocator()) Null_ग्रन्थिः(Doc);
  }

  // We got a normal value.
  पटिनिवत्त मानम् = parse_Block_ग्रन्थिः();
}

शून्य Mapping_ग्रन्थिः::increment() {
  सचे (failed()) {
    IsAtEnd = सच्च;
    CurrentEntry = nullptr;
    पटिनिवत्त;
  }
  सचे (CurrentEntry) {
    CurrentEntry->skip();
    सचे (Type == MT_Inline) {
      IsAtEnd = सच्च;
      CurrentEntry = nullptr;
      पटिनिवत्त;
    }
  }
  Token T = peekNext();
  सचे (T.Kind == Token::TK_कुञ्चिका || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = नूतन (getAllocator()) कुञ्चिका_मानम्_ग्रन्थिः(Doc);
  } अञ्ञथा सचे (Type == MT_Block) {
    विकल्प (T.Kind) {
    पक्ष Token::TK_BlockEnd:
      getNext();
      IsAtEnd = सच्च;
      CurrentEntry = nullptr;
      भङ्ग;
    स्वभाव:
      setError("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    पक्ष Token::TK_Error:
      IsAtEnd = सच्च;
      CurrentEntry = nullptr;
    }
  } अञ्ञथा {
    विकल्प (T.Kind) {
    पक्ष Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      पटिनिवत्त increment();
    पक्ष Token::TK_FlowMappingEnd:
      getNext();
      [[fallthrough]];
    पक्ष Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = सच्च;
      CurrentEntry = nullptr;
      भङ्ग;
    स्वभाव:
      setError( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      IsAtEnd = सच्च;
      CurrentEntry = nullptr;
    }
  }
}

शून्य Sequence_ग्रन्थिः::increment() {
  सचे (failed()) {
    IsAtEnd = सच्च;
    CurrentEntry = nullptr;
    पटिनिवत्त;
  }
  सचे (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  सचे (SeqType == ST_Block) {
    विकल्प (T.Kind) {
    पक्ष Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_ग्रन्थिः();
      सचे (!CurrentEntry) { // An error occurred.
        IsAtEnd = सच्च;
        CurrentEntry = nullptr;
      }
      भङ्ग;
    पक्ष Token::TK_BlockEnd:
      getNext();
      IsAtEnd = सच्च;
      CurrentEntry = nullptr;
      भङ्ग;
    स्वभाव:
      setError( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    पक्ष Token::TK_Error:
      IsAtEnd = सच्च;
      CurrentEntry = nullptr;
    }
  } अञ्ञथा सचे (SeqType == ST_Indentless) {
    विकल्प (T.Kind) {
    पक्ष Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_ग्रन्थिः();
      सचे (!CurrentEntry) { // An error occurred.
        IsAtEnd = सच्च;
        CurrentEntry = nullptr;
      }
      भङ्ग;
    स्वभाव:
    पक्ष Token::TK_Error:
      IsAtEnd = सच्च;
      CurrentEntry = nullptr;
    }
  } अञ्ञथा सचे (SeqType == ST_Flow) {
    विकल्प (T.Kind) {
    पक्ष Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      WasPreviousTokenFlowEntry = सच्च;
      पटिनिवत्त increment();
    पक्ष Token::TK_FlowSequenceEnd:
      getNext();
      [[fallthrough]];
    पक्ष Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = सच्च;
      CurrentEntry = nullptr;
      भङ्ग;
    पक्ष Token::TK_StreamEnd:
    पक्ष Token::TK_DocumentEnd:
    पक्ष Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = सच्च;
      CurrentEntry = nullptr;
      भङ्ग;
    स्वभाव:
      सचे (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = सच्च;
        CurrentEntry = nullptr;
        भङ्ग;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parse_Block_ग्रन्थिः();
      सचे (!CurrentEntry) {
        IsAtEnd = सच्च;
      }
      WasPreviousTokenFlowEntry = मिच्छा;
      भङ्ग;
    }
  }
}

Document::Document(Stream &S) : stream(S), मूलम्_2(nullptr) {
  // Tag maps starts with two default mappings.
  TagMap["!"] = "!";
  TagMap["!!"] = "tag:yaml.org,2002:";

  सचे (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peekNext();
  सचे (T.Kind == Token::TK_DocumentStart)
    getNext();
}

तार्किक Document::skip()  {
  सचे (stream.scanner->failed())
    पटिनिवत्त मिच्छा;
  सचे (!मूलम्_2 && !get_मूलम्())
    पटिनिवत्त मिच्छा;
  मूलम्_2->skip();
  Token &T = peekNext();
  सचे (T.Kind == Token::TK_StreamEnd)
    पटिनिवत्त मिच्छा;
  सचे (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    पटिनिवत्त skip();
  }
  पटिनिवत्त सच्च;
}

Token &Document::peekNext() {
  पटिनिवत्त stream.scanner->peekNext();
}

Token Document::getNext() {
  पटिनिवत्त stream.scanner->getNext();
}

शून्य Document::setError(ध्रुव Twine &Message, Token &Location) ध्रुव {
  stream.scanner->setError(Message, Location.Range.begin());
}

तार्किक Document::failed() ध्रुव {
  पटिनिवत्त stream.scanner->failed();
}

ग्रन्थिः *Document::parse_Block_ग्रन्थिः() {
  Token T = peekNext();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  विकल्प (T.Kind) {
  पक्ष Token::TK_Alias:
    getNext();
    पटिनिवत्त नूतन (ग्रन्थिः_Allocator) Alias_ग्रन्थिः(stream.CurrentDoc, T.Range.substr(1));
  पक्ष Token::TK_Anchor:
    सचे (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      पटिनिवत्त nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    गच्छ parse_property;
  पक्ष Token::TK_Tag:
    सचे (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      पटिनिवत्त nullptr;
    }
    TagInfo = getNext(); // Consume TK_Tag.
    T = peekNext();
    गच्छ parse_property;
  स्वभाव:
    भङ्ग;
  }

  विकल्प (T.Kind) {
  पक्ष Token::TK_BlockEntry:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    पटिनिवत्त नूतन (ग्रन्थिः_Allocator) Sequence_ग्रन्थिः( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , Sequence_ग्रन्थिः::ST_Indentless);
  पक्ष Token::TK_BlockSequenceStart:
    getNext();
    पटिनिवत्त नूतन (ग्रन्थिः_Allocator)
      Sequence_ग्रन्थिः( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_ग्रन्थिः::ST_Block);
  पक्ष Token::TK_BlockMappingStart:
    getNext();
    पटिनिवत्त नूतन (ग्रन्थिः_Allocator)
      Mapping_ग्रन्थिः( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_ग्रन्थिः::MT_Block);
  पक्ष Token::TK_FlowSequenceStart:
    getNext();
    पटिनिवत्त नूतन (ग्रन्थिः_Allocator)
      Sequence_ग्रन्थिः( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_ग्रन्थिः::ST_Flow);
  पक्ष Token::TK_FlowMappingStart:
    getNext();
    पटिनिवत्त नूतन (ग्रन्थिः_Allocator)
      Mapping_ग्रन्थिः( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_ग्रन्थिः::MT_Flow);
  पक्ष Token::TK_Scalar:
    getNext();
    पटिनिवत्त नूतन (ग्रन्थिः_Allocator)
      Scalar_ग्रन्थिः( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  पक्ष Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.मानम्.c_str(), T.मानम्.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(ग्रन्थिः_Allocator).drop_back();
    पटिनिवत्त नूतन (ग्रन्थिः_Allocator)
        Block_Scalar_ग्रन्थिः(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  पक्ष Token::TK_कुञ्चिका:
    // Don't eat the TK_Key, KeyValueNode expects it.
    पटिनिवत्त नूतन (ग्रन्थिः_Allocator)
      Mapping_ग्रन्थिः( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_ग्रन्थिः::MT_Inline);
  पक्ष Token::TK_DocumentStart:
  पक्ष Token::TK_DocumentEnd:
  पक्ष Token::TK_StreamEnd:
  स्वभाव:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    पटिनिवत्त नूतन (ग्रन्थिः_Allocator) Null_ग्रन्थिः(stream.CurrentDoc);
  पक्ष Token::TK_FlowMappingEnd:
  पक्ष Token::TK_FlowSequenceEnd:
  पक्ष Token::TK_FlowEntry: {
    सचे (मूलम्_2 && (isa<Mapping_ग्रन्थिः>(मूलम्_2) || isa<Sequence_ग्रन्थिः>(मूलम्_2)))
      पटिनिवत्त नूतन (ग्रन्थिः_Allocator) Null_ग्रन्थिः(stream.CurrentDoc);

    setError("Unexpected token", T);
    पटिनिवत्त nullptr;
  }
  पक्ष Token::TK_Error:
    पटिनिवत्त nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  पटिनिवत्त nullptr;
}

तार्किक Document::parseDirectives() {
  तार्किक isDirective = मिच्छा;
  याव (सच्च) {
    Token T = peekNext();
    सचे (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = सच्च;
    } अञ्ञथा सचे (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = सच्च;
    } अञ्ञथा {
      भङ्ग;
    }
  }
  पटिनिवत्त isDirective;
}

शून्य Document::parseYAMLDirective() {
  getNext(); // Eat %YAML <version>
}

शून्य Document::parseTAGDirective() {
  Token Tag = getNext(); // %TAG <handle> <prefix>
  StringRef T = Tag.Range;
  // Strip %TAG
  T = T.substr(T.find_first_of(" \t")).ltrim(" \t");
  std::size_t HandleEnd = T.find_first_of(" \t");
  StringRef TagHandle = T.substr(0, HandleEnd);
  StringRef TagPrefix = T.substr(HandleEnd).ltrim(" \t");
  TagMap[TagHandle] = TagPrefix;
}

तार्किक Document::expectToken(पूर्णाङ्क TK) {
  Token T = getNext();
  सचे (T.Kind != TK) {
    setError("Unexpected token", T);
    पटिनिवत्त मिच्छा;
  }
  पटिनिवत्त सच्च;
}
