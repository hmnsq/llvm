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
using EncodingInfo = std::pair<UnicodeEncodingForm, अचिन्हित>;

/// getUnicodeEncoding - Reads up to the first 4 bytes to determine the Unicode
///                      encoding form of \a Input.
///
/// @param Input A string of length 0 or more.
/// @returns An EncodingInfo indicating the Unicode encoding form of the input
///          and how long the byte order mark is if one exists.
स्थिर EncodingInfo getUnicodeEncoding(StringRef निवेश) {
  यदि (निवेश.empty())
    प्रत्यागम {UEF_Unknown, 0};

  विकल्प (uint8_t(निवेश[0])) {
  पक्ष 0x00:
    यदि (निवेश.size() >= 4) {
      यदि (  निवेश[1] == 0
         && uint8_t(निवेश[2]) == 0xFE
         && uint8_t(निवेश[3]) == 0xFF)
        प्रत्यागम {UEF_UTF32_BE, 4};
      यदि (निवेश[1] == 0 && निवेश[2] == 0 && निवेश[3] != 0)
        प्रत्यागम {UEF_UTF32_BE, 0};
    }

    यदि (निवेश.size() >= 2 && निवेश[1] != 0)
      प्रत्यागम {UEF_UTF16_BE, 0};
    प्रत्यागम {UEF_Unknown, 0};
  पक्ष 0xFF:
    यदि (  निवेश.size() >= 4
       && uint8_t(निवेश[1]) == 0xFE
       && निवेश[2] == 0
       && निवेश[3] == 0)
      प्रत्यागम {UEF_UTF32_LE, 4};

    यदि (निवेश.size() >= 2 && uint8_t(निवेश[1]) == 0xFE)
      प्रत्यागम {UEF_UTF16_LE, 2};
    प्रत्यागम {UEF_Unknown, 0};
  पक्ष 0xFE:
    यदि (निवेश.size() >= 2 && uint8_t(निवेश[1]) == 0xFF)
      प्रत्यागम {UEF_UTF16_BE, 2};
    प्रत्यागम {UEF_Unknown, 0};
  पक्ष 0xEF:
    यदि (  निवेश.size() >= 3
       && uint8_t(निवेश[1]) == 0xBB
       && uint8_t(निवेश[2]) == 0xBF)
      प्रत्यागम {UEF_UTF8, 3};
    प्रत्यागम {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  यदि (निवेश.size() >= 4 && निवेश[1] == 0 && निवेश[2] == 0 && निवेश[3] == 0)
    प्रत्यागम {UEF_UTF32_LE, 0};

  यदि (निवेश.size() >= 2 && निवेश[1] == 0)
    प्रत्यागम {UEF_UTF16_LE, 0};

  प्रत्यागम {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
शून्य गाँठ::anchor() {}
शून्य Null_गाँठ::anchor() {}
शून्य Scalar_गाँठ::anchor() {}
शून्य Block_Scalar_गाँठ::anchor() {}
शून्य कुंजी_मूल्य_गाँठ::anchor() {}
शून्य Mapping_गाँठ::anchor() {}
शून्य Sequence_गाँठ::anchor() {}
शून्य Alias_गाँठ::anchor() {}

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
    TK_कुंजी,
    TK_मूल्य,
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
  std::string मूल्य;

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
संरचना Simple_कुंजी {
  TokenQueueT::iterator Tok;
  अचिन्हित Column = 0;
  अचिन्हित पंक्ति = 0;
  अचिन्हित FlowLevel = 0;
  तार्किक IsRequired = असत्य;

  तार्किक चालक ==(ध्रुव Simple_कुंजी &Other) {
    प्रत्यागम Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
using UTF8Decoded = std::pair<uint32_t, अचिन्हित>;

स्थिर UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator End = Range.end();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  यदि (Position < End && (*Position & 0x80) == 0) {
    प्रत्यागम {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  यदि (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    यदि (codepoint >= 0x80)
      प्रत्यागम {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  यदि (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    यदि (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      प्रत्यागम {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  यदि (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    यदि (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      प्रत्यागम {codepoint, 4};
  }
  प्रत्यागम {0, 0};
}

नामाकाश llvm {
नामाकाश yaml {

/// Scans YAML tokens from a MemoryBuffer.
वर्ग Scanner {
सार्वजनिक:
  Scanner(StringRef निवेश, SourceMgr &SM, तार्किक ShowColors = सत्य,
          std::error_code *EC = nullptr);
  Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, तार्किक ShowColors = सत्य,
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
    यदि (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    यदि (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    यदि (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = सत्य;
  }

  /// Returns true if an error occurred while parsing.
  तार्किक failed() {
    प्रत्यागम Failed;
  }

गुप्त:
  शून्य init(MemoryBufferRef Buffer);

  StringRef current_निवेश() {
    प्रत्यागम StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    प्रत्यागम ::decodeUTF8(StringRef(Position, End - Position));
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
  तार्किक is_पंक्ति_Empty(StringRef पंक्ति);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  तार्किक consume_पंक्ति_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  शून्य save_Simple_कुंजी_Candidate( TokenQueueT::iterator Tok
                             , अचिन्हित AtColumn
                             , तार्किक IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  शून्य remove_Stale_Simple_कुंजी_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  शून्य remove_Simple_कुंजी_Candidates_On_Flow_Level(अचिन्हित Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  तार्किक unrollIndent(पूर्णाङ्क ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  तार्किक rollIndent( पूर्णाङ्क ToColumn
                 , Token::TokenKind Kind
                 , TokenQueueT::iterator जोड़ो_Point);

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
  तार्किक scan_कुंजी();

  /// Scan an explicit : indicating a value.
  तार्किक scan_मूल्य();

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
                                 अचिन्हित &IndentIndicator, तार्किक &IsDone);

  /// Scan a style indicator in a block scalar header.
  वर्ण scanBlockStyleIndicator();

  /// Scan a chomping indicator in a block scalar header.
  वर्ण scanBlockChompingIndicator();

  /// Scan an indentation indicator in a block scalar header.
  अचिन्हित scanBlockIndentationIndicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  तार्किक scanBlockScalarHeader(वर्ण &ChompingIndicator, अचिन्हित &IndentIndicator,
                             तार्किक &IsDone);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  तार्किक findBlockScalarIndent(अचिन्हित &BlockIndent, अचिन्हित BlockExitIndent,
                             अचिन्हित &पंक्ति_Breaks, तार्किक &IsDone);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  तार्किक scanBlockScalarIndent(अचिन्हित BlockIndent, अचिन्हित BlockExitIndent,
                             तार्किक &IsDone);

  /// Scan a tag of the form !stuff.
  तार्किक scanTag();

  /// Dispatch to the next scanning function based on \a *Cur.
  तार्किक fetchMoreTokens();

  /// The SourceMgr used for diagnostics and buffer management.
  SourceMgr &SM;

  /// The original input.
  MemoryBufferRef निवेश_Buffer;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator End;

  /// Current YAML indentation level in spaces.
  पूर्णाङ्क Indent;

  /// Current column number in Unicode code points.
  अचिन्हित Column;

  /// Current line number.
  अचिन्हित पंक्ति;

  /// How deep we are in flow style containers. 0 Means at block level.
  अचिन्हित FlowLevel;

  /// Are we at the start of the stream?
  तार्किक IsStartOfStream;

  /// Can the next token be the start of a simple key?
  तार्किक Is_Simple_कुंजी_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  तार्किक Is_Adjacent_मूल्य_Allowed_In_Flow;

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
  SmallVector<Simple_कुंजी, 4> Simple_कुंजियाँ;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
स्थिर शून्य encodeUTF8( uint32_t Unicode_Scalar_मूल्य
                      , SmallVectorImpl<वर्ण> &Result) {
  यदि (Unicode_Scalar_मूल्य <= 0x7F) {
    Result.धकेलो_back(Unicode_Scalar_मूल्य & 0x7F);
  } अन्यथा यदि (Unicode_Scalar_मूल्य <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((Unicode_Scalar_मूल्य & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Unicode_Scalar_मूल्य & 0x3F);
    Result.धकेलो_back(FirstByte);
    Result.धकेलो_back(SecondByte);
  } अन्यथा यदि (Unicode_Scalar_मूल्य <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((Unicode_Scalar_मूल्य & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_मूल्य & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Unicode_Scalar_मूल्य & 0x3F);
    Result.धकेलो_back(FirstByte);
    Result.धकेलो_back(SecondByte);
    Result.धकेलो_back(ThirdByte);
  } अन्यथा यदि (Unicode_Scalar_मूल्य <= 0x10FFFF) {
    uint8_t FirstByte = 0xF0 | ((Unicode_Scalar_मूल्य & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_मूल्य & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Unicode_Scalar_मूल्य & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_मूल्य & 0x3F);
    Result.धकेलो_back(FirstByte);
    Result.धकेलो_back(SecondByte);
    Result.धकेलो_back(ThirdByte);
    Result.धकेलो_back(FourthByte);
  }
}

तार्किक yaml::dumpTokens(StringRef निवेश, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(निवेश, SM);
  यावत् (सत्य) {
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
    पक्ष Token::TK_कुंजी:
      OS << "Key: ";
      भङ्ग;
    पक्ष Token::TK_मूल्य:
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
    यदि (T.Kind == Token::TK_StreamEnd)
      भङ्ग;
    अन्यथा यदि (T.Kind == Token::TK_Error)
      प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य;
}

तार्किक yaml::scanTokens(StringRef निवेश) {
  SourceMgr SM;
  Scanner scanner(निवेश, SM);
  यावत् (सत्य) {
    Token T = scanner.getNext();
    यदि (T.Kind == Token::TK_StreamEnd)
      भङ्ग;
    अन्यथा यदि (T.Kind == Token::TK_Error)
      प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य;
}

std::string yaml::escape(StringRef निवेश, तार्किक EscapePrintable) {
  std::string Escaped_निवेश;
  प्रति (StringRef::iterator i = निवेश.begin(), e = निवेश.end(); i != e; ++i) {
    यदि (*i == '\\')
      Escaped_निवेश += "\\\\";
    अन्यथा यदि (*i == '"')
      Escaped_निवेश += "\\\"";
    अन्यथा यदि (*i == 0)
      Escaped_निवेश += "\\0";
    अन्यथा यदि (*i == 0x07)
      Escaped_निवेश += "\\a";
    अन्यथा यदि (*i == 0x08)
      Escaped_निवेश += "\\b";
    अन्यथा यदि (*i == 0x09)
      Escaped_निवेश += "\\t";
    अन्यथा यदि (*i == 0x0A)
      Escaped_निवेश += "\\n";
    अन्यथा यदि (*i == 0x0B)
      Escaped_निवेश += "\\v";
    अन्यथा यदि (*i == 0x0C)
      Escaped_निवेश += "\\f";
    अन्यथा यदि (*i == 0x0D)
      Escaped_निवेश += "\\r";
    अन्यथा यदि (*i == 0x1B)
      Escaped_निवेश += "\\e";
    अन्यथा यदि ((अचिन्हित वर्ण)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_निवेश += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } अन्यथा यदि (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_मूल्य
        = decodeUTF8(StringRef(i, निवेश.end() - i));
      यदि (Unicode_Scalar_मूल्य.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_निवेश, Val);
        // FIXME: Error reporting.
        प्रत्यागम Escaped_निवेश;
      }
      यदि (Unicode_Scalar_मूल्य.first == 0x85)
        Escaped_निवेश += "\\N";
      अन्यथा यदि (Unicode_Scalar_मूल्य.first == 0xA0)
        Escaped_निवेश += "\\_";
      अन्यथा यदि (Unicode_Scalar_मूल्य.first == 0x2028)
        Escaped_निवेश += "\\L";
      अन्यथा यदि (Unicode_Scalar_मूल्य.first == 0x2029)
        Escaped_निवेश += "\\P";
      अन्यथा यदि (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_मूल्य.first))
        Escaped_निवेश += StringRef(i, Unicode_Scalar_मूल्य.second);
      अन्यथा {
        std::string HexStr = utohexstr(Unicode_Scalar_मूल्य.first);
        यदि (HexStr.size() <= 2)
          Escaped_निवेश += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        अन्यथा यदि (HexStr.size() <= 4)
          Escaped_निवेश += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        अन्यथा यदि (HexStr.size() <= 8)
          Escaped_निवेश += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += Unicode_Scalar_मूल्य.second - 1;
    } अन्यथा {
      Escaped_निवेश.धकेलो_back(*i);
    }
  }
  प्रत्यागम Escaped_निवेश;
}

std::optional<तार्किक> yaml::parseBool(StringRef S) {
  विकल्प (S.size()) {
  पक्ष 1:
    विकल्प (S.front()) {
    पक्ष 'y':
    पक्ष 'Y':
      प्रत्यागम सत्य;
    पक्ष 'n':
    पक्ष 'N':
      प्रत्यागम असत्य;
    स्वभाव:
      प्रत्यागम std::nullopt;
    }
  पक्ष 2:
    विकल्प (S.front()) {
    पक्ष 'O':
      यदि (S[1] == 'N') // ON
        प्रत्यागम सत्य;
      [[fallthrough]];
    पक्ष 'o':
      यदि (S[1] == 'n') //[Oo]n
        प्रत्यागम सत्य;
      प्रत्यागम std::nullopt;
    पक्ष 'N':
      यदि (S[1] == 'O') // NO
        प्रत्यागम असत्य;
      [[fallthrough]];
    पक्ष 'n':
      यदि (S[1] == 'o') //[Nn]o
        प्रत्यागम असत्य;
      प्रत्यागम std::nullopt;
    स्वभाव:
      प्रत्यागम std::nullopt;
    }
  पक्ष 3:
    विकल्प (S.front()) {
    पक्ष 'O':
      यदि (S.drop_front() == "FF") // OFF
        प्रत्यागम असत्य;
      [[fallthrough]];
    पक्ष 'o':
      यदि (S.drop_front() == "ff") //[Oo]ff
        प्रत्यागम असत्य;
      प्रत्यागम std::nullopt;
    पक्ष 'Y':
      यदि (S.drop_front() == "ES") // YES
        प्रत्यागम सत्य;
      [[fallthrough]];
    पक्ष 'y':
      यदि (S.drop_front() == "es") //[Yy]es
        प्रत्यागम सत्य;
      प्रत्यागम std::nullopt;
    स्वभाव:
      प्रत्यागम std::nullopt;
    }
  पक्ष 4:
    विकल्प (S.front()) {
    पक्ष 'T':
      यदि (S.drop_front() == "RUE") // TRUE
        प्रत्यागम सत्य;
      [[fallthrough]];
    पक्ष 't':
      यदि (S.drop_front() == "rue") //[Tt]rue
        प्रत्यागम सत्य;
      प्रत्यागम std::nullopt;
    स्वभाव:
      प्रत्यागम std::nullopt;
    }
  पक्ष 5:
    विकल्प (S.front()) {
    पक्ष 'F':
      यदि (S.drop_front() == "ALSE") // FALSE
        प्रत्यागम असत्य;
      [[fallthrough]];
    पक्ष 'f':
      यदि (S.drop_front() == "alse") //[Ff]alse
        प्रत्यागम असत्य;
      प्रत्यागम std::nullopt;
    स्वभाव:
      प्रत्यागम std::nullopt;
    }
  स्वभाव:
    प्रत्यागम std::nullopt;
  }
}

Scanner::Scanner(StringRef निवेश, SourceMgr &sm, तार्किक ShowColors,
                 std::error_code *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(MemoryBufferRef(निवेश, "YAML"));
}

Scanner::Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, तार्किक ShowColors,
                 std::error_code *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(Buffer);
}

शून्य Scanner::init(MemoryBufferRef Buffer) {
  निवेश_Buffer = Buffer;
  Current = निवेश_Buffer.getBufferStart();
  End = निवेश_Buffer.getBufferEnd();
  Indent = -1;
  Column = 0;
  पंक्ति = 0;
  FlowLevel = 0;
  IsStartOfStream = सत्य;
  Is_Simple_कुंजी_Allowed = सत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;
  Failed = असत्य;
  std::unique_ptr<MemoryBuffer> निवेश_Buffer_Owner =
      MemoryBuffer::getMemBuffer(Buffer, /*RequiresNullTerminator=*/असत्य);
  SM.AddNewSourceBuffer(std::move(निवेश_Buffer_Owner), SMLoc());
}

Token &Scanner::peekNext() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  तार्किक NeedMore = असत्य;
  यावत् (सत्य) {
    यदि (TokenQueue.empty() || NeedMore) {
      यदि (!fetchMoreTokens()) {
        TokenQueue.clear();
        Simple_कुंजियाँ.clear();
        TokenQueue.धकेलो_back(Token());
        प्रत्यागम TokenQueue.front();
      }
    }
    प्रतिज्ञा(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_कुंजी_Candidates();
    Simple_कुंजी SK;
    SK.Tok = TokenQueue.begin();
    यदि (!is_contained(Simple_कुंजियाँ, SK))
      भङ्ग;
    अन्यथा
      NeedMore = सत्य;
  }
  प्रत्यागम TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  यदि (!TokenQueue.empty())
    TokenQueue.निकालो_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  यदि (TokenQueue.empty())
    TokenQueue.resetAlloc();

  प्रत्यागम Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  यदि (Position == End)
    प्रत्यागम Position;
  // Check 7 bit c-printable - b-char.
  यदि (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    प्रत्यागम Position + 1;

  // Check for valid UTF-8.
  यदि (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    यदि (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      प्रत्यागम Position + u8d.second;
  }
  प्रत्यागम Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  यदि (Position == End)
    प्रत्यागम Position;
  यदि (*Position == 0x0D) {
    यदि (Position + 1 != End && *(Position + 1) == 0x0A)
      प्रत्यागम Position + 2;
    प्रत्यागम Position + 1;
  }

  यदि (*Position == 0x0A)
    प्रत्यागम Position + 1;
  प्रत्यागम Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  यदि (Position == End)
    प्रत्यागम Position;
  यदि (*Position == ' ')
    प्रत्यागम Position + 1;
  प्रत्यागम Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  यदि (Position == End)
    प्रत्यागम Position;
  यदि (*Position == ' ' || *Position == '\t')
    प्रत्यागम Position + 1;
  प्रत्यागम Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  यदि (Position == End)
    प्रत्यागम Position;
  यदि (*Position == ' ' || *Position == '\t')
    प्रत्यागम Position;
  प्रत्यागम skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  यावत् (सत्य) {
    StringRef::iterator i = (एतत्->*Func)(Position);
    यदि (i == Position)
      भङ्ग;
    Position = i;
  }
  प्रत्यागम Position;
}

शून्य Scanner::advanceWhile(SkipWhileFunc Func) {
  स्वतः Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

स्थिर तार्किक is_ns_hex_digit(ध्रुव वर्ण C) { प्रत्यागम isAlnum(C); }

स्थिर तार्किक is_ns_word_char(ध्रुव वर्ण C) { प्रत्यागम C == '-' || isAlpha(C); }

शून्य Scanner::scan_ns_uri_char() {
  यावत् (सत्य) {
    यदि (Current == End)
      भङ्ग;
    यदि ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } अन्यथा {
      भङ्ग;
    }
  }
}

तार्किक Scanner::consume(uint32_t Expected) {
  यदि (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    प्रत्यागम असत्य;
  }
  यदि (Current == End)
    प्रत्यागम असत्य;
  यदि (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    प्रत्यागम असत्य;
  }
  यदि (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

शून्य Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  प्रतिज्ञा(Current <= End && "Skipped past the end");
}

तार्किक Scanner::isBlankOrBreak(StringRef::iterator Position) {
  यदि (Position == End)
    प्रत्यागम असत्य;
  प्रत्यागम *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

तार्किक Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  यदि (Position == End || isBlankOrBreak(Position))
    प्रत्यागम असत्य;
  यदि (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    प्रत्यागम असत्य;
  प्रत्यागम सत्य;
}

तार्किक Scanner::is_पंक्ति_Empty(StringRef पंक्ति) {
  प्रति (ध्रुव स्वतः *Position = पंक्ति.begin(); Position != पंक्ति.end(); ++Position)
    यदि (!isBlankOrBreak(Position))
      प्रत्यागम असत्य;
  प्रत्यागम सत्य;
}

तार्किक Scanner::consume_पंक्ति_Break_If_Present() {
  स्वतः Next = skip_b_break(Current);
  यदि (Next == Current)
    प्रत्यागम असत्य;
  Column = 0;
  ++पंक्ति;
  Current = Next;
  प्रत्यागम सत्य;
}

शून्य Scanner::save_Simple_कुंजी_Candidate( TokenQueueT::iterator Tok
                                    , अचिन्हित AtColumn
                                    , तार्किक IsRequired) {
  यदि (Is_Simple_कुंजी_Allowed) {
    Simple_कुंजी SK;
    SK.Tok = Tok;
    SK.पंक्ति = पंक्ति;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_कुंजियाँ.धकेलो_back(SK);
  }
}

शून्य Scanner::remove_Stale_Simple_कुंजी_Candidates() {
  प्रति (SmallVectorImpl<Simple_कुंजी>::iterator i = Simple_कुंजियाँ.begin();
                                            i != Simple_कुंजियाँ.end();) {
    यदि (i->पंक्ति != पंक्ति || i->Column + 1024 < Column) {
      यदि (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_कुंजियाँ.erase(i);
    } अन्यथा {
      ++i;
    }
  }
}

शून्य Scanner::remove_Simple_कुंजी_Candidates_On_Flow_Level(अचिन्हित Level) {
  यदि (!Simple_कुंजियाँ.empty() && (Simple_कुंजियाँ.end() - 1)->FlowLevel == Level)
    Simple_कुंजियाँ.निकालो_back();
}

तार्किक Scanner::unrollIndent(पूर्णाङ्क ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  यदि (FlowLevel != 0)
    प्रत्यागम सत्य;

  यावत् (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.धकेलो_back(T);
    Indent = Indents.निकालो_back_val();
  }

  प्रत्यागम सत्य;
}

तार्किक Scanner::rollIndent( पूर्णाङ्क ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator जोड़ो_Point) {
  यदि (FlowLevel)
    प्रत्यागम सत्य;
  यदि (Indent < ToColumn) {
    Indents.धकेलो_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.जोड़ो(जोड़ो_Point, T);
  }
  प्रत्यागम सत्य;
}

शून्य Scanner::skipComment() {
  यदि (Current == End || *Current != '#')
    प्रत्यागम;
  यावत् (सत्य) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    यदि (I == Current)
      भङ्ग;
    Current = I;
    ++Column;
  }
}

शून्य Scanner::scanToNextToken() {
  यावत् (सत्य) {
    यावत् (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    यदि (i == Current)
      भङ्ग;
    Current = i;
    ++पंक्ति;
    Column = 0;
    // New lines may start a simple key.
    यदि (!FlowLevel)
      Is_Simple_कुंजी_Allowed = सत्य;
  }
}

तार्किक Scanner::scanStreamStart() {
  IsStartOfStream = असत्य;

  EncodingInfo EI = getUnicodeEncoding(current_निवेश());

  Token T;
  T.Kind = Token::TK_StreamStart;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.धकेलो_back(T);
  Current += EI.second;
  प्रत्यागम सत्य;
}

तार्किक Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  यदि (Column != 0) {
    Column = 0;
    ++पंक्ति;
  }

  unrollIndent(-1);
  Simple_कुंजियाँ.clear();
  Is_Simple_कुंजी_Allowed = असत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  Token T;
  T.Kind = Token::TK_StreamEnd;
  T.Range = StringRef(Current, 0);
  TokenQueue.धकेलो_back(T);
  प्रत्यागम सत्य;
}

तार्किक Scanner::scanDirective() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_कुंजियाँ.clear();
  Is_Simple_कुंजी_Allowed = असत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  StringRef::iterator Start = Current;
  consume('%');
  StringRef::iterator NameStart = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(NameStart, Current - NameStart);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  यदि (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.धकेलो_back(T);
    प्रत्यागम सत्य;
  } अन्यथा यदि(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.धकेलो_back(T);
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

तार्किक Scanner::scanDocumentIndicator(तार्किक IsStart) {
  unrollIndent(-1);
  Simple_कुंजियाँ.clear();
  Is_Simple_कुंजी_Allowed = असत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  Token T;
  T.Kind = IsStart ? Token::TK_DocumentStart : Token::TK_DocumentEnd;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.धकेलो_back(T);
  प्रत्यागम सत्य;
}

तार्किक Scanner::scanFlowCollectionStart(तार्किक IsSequence) {
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.धकेलो_back(T);

  // [ and { may begin a simple key.
  save_Simple_कुंजी_Candidate(--TokenQueue.end(), Column - 1, असत्य);

  // And may also be followed by a simple key.
  Is_Simple_कुंजी_Allowed = सत्य;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;
  ++FlowLevel;
  प्रत्यागम सत्य;
}

तार्किक Scanner::scanFlowCollectionEnd(तार्किक IsSequence) {
  remove_Simple_कुंजी_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_कुंजी_Allowed = असत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = सत्य;
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceEnd
                      : Token::TK_FlowMappingEnd;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.धकेलो_back(T);
  यदि (FlowLevel)
    --FlowLevel;
  प्रत्यागम सत्य;
}

तार्किक Scanner::scanFlowEntry() {
  remove_Simple_कुंजी_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_कुंजी_Allowed = सत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;
  Token T;
  T.Kind = Token::TK_FlowEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.धकेलो_back(T);
  प्रत्यागम सत्य;
}

तार्किक Scanner::scanBlockEntry() {
  rollIndent(Column, Token::TK_BlockSequenceStart, TokenQueue.end());
  remove_Simple_कुंजी_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_कुंजी_Allowed = सत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;
  Token T;
  T.Kind = Token::TK_BlockEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.धकेलो_back(T);
  प्रत्यागम सत्य;
}

तार्किक Scanner::scan_कुंजी() {
  यदि (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  remove_Simple_कुंजी_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_कुंजी_Allowed = !FlowLevel;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  Token T;
  T.Kind = Token::TK_कुंजी;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.धकेलो_back(T);
  प्रत्यागम सत्य;
}

तार्किक Scanner::scan_मूल्य() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  यदि (!Simple_कुंजियाँ.empty()) {
    Simple_कुंजी SK = Simple_कुंजियाँ.निकालो_back_val();
    Token T;
    T.Kind = Token::TK_कुंजी;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    प्रति (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      यदि (i == SK.Tok)
        भङ्ग;
    }
    यदि (i == e) {
      Failed = सत्य;
      प्रत्यागम असत्य;
    }
    i = TokenQueue.जोड़ो(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    Is_Simple_कुंजी_Allowed = असत्य;
  } अन्यथा {
    यदि (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    Is_Simple_कुंजी_Allowed = !FlowLevel;
  }
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  Token T;
  T.Kind = Token::TK_मूल्य;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.धकेलो_back(T);
  प्रत्यागम सत्य;
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
  यावत् (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  प्रत्यागम (Position - 1 - I) % 2 == 1;
}

तार्किक Scanner::scanFlowScalar(तार्किक IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  अचिन्हित ColStart = Column;
  यदि (IsDoubleQuoted) {
    कुरु {
      ++Current;
      यावत् (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } यावत् (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } अन्यथा {
    skip(1);
    यावत् (Current != End) {
      // Skip a ' followed by another '.
      यदि (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        अनुवर्त;
      } अन्यथा यदि (*Current == '\'')
        भङ्ग;
      StringRef::iterator i = skip_nb_char(Current);
      यदि (i == Current) {
        i = skip_b_break(Current);
        यदि (i == Current)
          भङ्ग;
        Current = i;
        Column = 0;
        ++पंक्ति;
      } अन्यथा {
        यदि (i == End)
          भङ्ग;
        Current = i;
        ++Column;
      }
    }
  }

  यदि (Current == End) {
    setError("Expected quote at end of scalar", Current);
    प्रत्यागम असत्य;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.धकेलो_back(T);

  save_Simple_कुंजी_Candidate(--TokenQueue.end(), ColStart, असत्य);

  Is_Simple_कुंजी_Allowed = असत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = सत्य;

  प्रत्यागम सत्य;
}

तार्किक Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  अचिन्हित ColStart = Column;
  अचिन्हित LeadingBlanks = 0;
  प्रतिज्ञा(Indent >= -1 && "Indent must be >= -1 !");
  अचिन्हित indent = static_cast<अचिन्हित>(Indent + 1);
  यावत् (Current != End) {
    यदि (*Current == '#')
      भङ्ग;

    यावत् (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      यदि (i == Current)
        भङ्ग;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    यदि (!isBlankOrBreak(Current))
      भङ्ग;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    यावत् (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      यदि (i != Tmp) {
        यदि (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          प्रत्यागम असत्य;
        }
        Tmp = i;
        ++Column;
      } अन्यथा {
        i = skip_b_break(Tmp);
        यदि (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++पंक्ति;
      }
    }

    यदि (!FlowLevel && Column < indent)
      भङ्ग;

    Current = Tmp;
  }
  यदि (Start == Current) {
    setError("Got empty plain scalar", Start);
    प्रत्यागम असत्य;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.धकेलो_back(T);

  // Plain scalars can be simple keys.
  save_Simple_कुंजी_Candidate(--TokenQueue.end(), ColStart, असत्य);

  Is_Simple_कुंजी_Allowed = असत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  प्रत्यागम सत्य;
}

तार्किक Scanner::scanAliasOrAnchor(तार्किक IsAlias) {
  StringRef::iterator Start = Current;
  अचिन्हित ColStart = Column;
  skip(1);
  यावत् (Current != End) {
    यदि (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      भङ्ग;
    StringRef::iterator i = skip_ns_char(Current);
    यदि (i == Current)
      भङ्ग;
    Current = i;
    ++Column;
  }

  यदि (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    प्रत्यागम असत्य;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.धकेलो_back(T);

  // Alias and anchors can be simple keys.
  save_Simple_कुंजी_Candidate(--TokenQueue.end(), ColStart, असत्य);

  Is_Simple_कुंजी_Allowed = असत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  प्रत्यागम सत्य;
}

तार्किक Scanner::scanBlockScalarIndicators(वर्ण &StyleIndicator,
                                        वर्ण &ChompingIndicator,
                                        अचिन्हित &IndentIndicator,
                                        तार्किक &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  यदि (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    प्रत्यागम असत्य;
  प्रत्यागम सत्य;
}

वर्ण Scanner::scanBlockStyleIndicator() {
  वर्ण Indicator = ' ';
  यदि (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  प्रत्यागम Indicator;
}

वर्ण Scanner::scanBlockChompingIndicator() {
  वर्ण Indicator = ' ';
  यदि (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  प्रत्यागम Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
स्थिर अचिन्हित get_Chomped_पंक्ति_Breaks(वर्ण ChompingIndicator,
                                     अचिन्हित पंक्ति_Breaks, StringRef Str) {
  यदि (ChompingIndicator == '-') // Strip all line breaks.
    प्रत्यागम 0;
  यदि (ChompingIndicator == '+') // Keep all line breaks.
    प्रत्यागम पंक्ति_Breaks;
  // Clip trailing lines.
  प्रत्यागम Str.empty() ? 0 : 1;
}

अचिन्हित Scanner::scanBlockIndentationIndicator() {
  अचिन्हित Indent = 0;
  यदि (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = अचिन्हित(*Current - '0');
    skip(1);
  }
  प्रत्यागम Indent;
}

तार्किक Scanner::scanBlockScalarHeader(वर्ण &ChompingIndicator,
                                    अचिन्हित &IndentIndicator, तार्किक &IsDone) {
  स्वतः Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  यदि (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  यदि (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.धकेलो_back(T);
    IsDone = सत्य;
    प्रत्यागम सत्य;
  }

  यदि (!consume_पंक्ति_Break_If_Present()) {
    setError("Expected a line break after block scalar header", Current);
    प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य;
}

तार्किक Scanner::findBlockScalarIndent(अचिन्हित &BlockIndent,
                                    अचिन्हित BlockExitIndent,
                                    अचिन्हित &पंक्ति_Breaks, तार्किक &IsDone) {
  अचिन्हित Max_All_Space_पंक्ति_Characters = 0;
  StringRef::iterator Longest_All_Space_पंक्ति;

  यावत् (सत्य) {
    advanceWhile(&Scanner::skip_s_space);
    यदि (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      यदि (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = सत्य;
        प्रत्यागम सत्य;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      यदि (Max_All_Space_पंक्ति_Characters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_पंक्ति);
        प्रत्यागम असत्य;
      }
      प्रत्यागम सत्य;
    }
    यदि (skip_b_break(Current) != Current &&
        Column > Max_All_Space_पंक्ति_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_पंक्ति_Characters = Column;
      Longest_All_Space_पंक्ति = Current;
    }

    // Check for EOF.
    यदि (Current == End) {
      IsDone = सत्य;
      प्रत्यागम सत्य;
    }

    यदि (!consume_पंक्ति_Break_If_Present()) {
      IsDone = सत्य;
      प्रत्यागम सत्य;
    }
    ++पंक्ति_Breaks;
  }
  प्रत्यागम सत्य;
}

तार्किक Scanner::scanBlockScalarIndent(अचिन्हित BlockIndent,
                                    अचिन्हित BlockExitIndent, तार्किक &IsDone) {
  // Skip the indentation.
  यावत् (Column < BlockIndent) {
    स्वतः I = skip_s_space(Current);
    यदि (I == Current)
      भङ्ग;
    Current = I;
    ++Column;
  }

  यदि (skip_nb_char(Current) == Current)
    प्रत्यागम सत्य;

  यदि (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = सत्य;
    प्रत्यागम सत्य;
  }

  यदि (Column < BlockIndent) {
    यदि (Current != End && *Current == '#') { // Trailing comment.
      IsDone = सत्य;
      प्रत्यागम सत्य;
    }
    setError("A text line is less indented than the block scalar", Current);
    प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य; // A normal text line.
}

तार्किक Scanner::scanBlockScalar(तार्किक IsLiteral) {
  प्रतिज्ञा(*Current == '|' || *Current == '>');
  वर्ण StyleIndicator;
  वर्ण ChompingIndicator;
  अचिन्हित BlockIndent;
  तार्किक IsDone = असत्य;
  यदि (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    प्रत्यागम असत्य;
  यदि (IsDone)
    प्रत्यागम सत्य;
  तार्किक IsFolded = StyleIndicator == '>';

  ध्रुव स्वतः *Start = Current;
  अचिन्हित BlockExitIndent = Indent < 0 ? 0 : (अचिन्हित)Indent;
  अचिन्हित पंक्ति_Breaks = 0;
  यदि (BlockIndent == 0) {
    यदि (!findBlockScalarIndent(BlockIndent, BlockExitIndent, पंक्ति_Breaks,
                               IsDone))
      प्रत्यागम असत्य;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  यावत् (!IsDone) {
    यदि (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      प्रत्यागम असत्य;
    यदि (IsDone)
      भङ्ग;

    // Parse the current line.
    स्वतः पंक्ति_Start = Current;
    advanceWhile(&Scanner::skip_nb_char);
    यदि (पंक्ति_Start != Current) {
      यदि (पंक्ति_Breaks && IsFolded && !Scanner::is_पंक्ति_Empty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        यदि (पंक्ति_Breaks == 1) {
          Str.append(पंक्ति_Breaks,
                     is_पंक्ति_Empty(StringRef(पंक्ति_Start, Current - पंक्ति_Start))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        पंक्ति_Breaks--;
      }
      Str.append(पंक्ति_Breaks, '\n');
      Str.append(StringRef(पंक्ति_Start, Current - पंक्ति_Start));
      पंक्ति_Breaks = 0;
    }

    // Check for EOF.
    यदि (Current == End)
      भङ्ग;

    यदि (!consume_पंक्ति_Break_If_Present())
      भङ्ग;
    ++पंक्ति_Breaks;
  }

  यदि (Current == End && !पंक्ति_Breaks)
    // Ensure that there is at least one line break before the end of file.
    पंक्ति_Breaks = 1;
  Str.append(get_Chomped_पंक्ति_Breaks(ChompingIndicator, पंक्ति_Breaks, Str), '\n');

  // New lines may start a simple key.
  यदि (!FlowLevel)
    Is_Simple_कुंजी_Allowed = सत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.मूल्य = std::string(Str);
  TokenQueue.धकेलो_back(T);
  प्रत्यागम सत्य;
}

तार्किक Scanner::scanTag() {
  StringRef::iterator Start = Current;
  अचिन्हित ColStart = Column;
  skip(1); // Eat !.
  यदि (Current == End || isBlankOrBreak(Current)); // An empty tag.
  अन्यथा यदि (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    यदि (!consume('>'))
      प्रत्यागम असत्य;
  } अन्यथा {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.Kind = Token::TK_Tag;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.धकेलो_back(T);

  // Tags can be simple keys.
  save_Simple_कुंजी_Candidate(--TokenQueue.end(), ColStart, असत्य);

  Is_Simple_कुंजी_Allowed = असत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  प्रत्यागम सत्य;
}

तार्किक Scanner::fetchMoreTokens() {
  यदि (IsStartOfStream)
    प्रत्यागम scanStreamStart();

  scanToNextToken();

  यदि (Current == End)
    प्रत्यागम scanStreamEnd();

  remove_Stale_Simple_कुंजी_Candidates();

  unrollIndent(Column);

  यदि (Column == 0 && *Current == '%')
    प्रत्यागम scanDirective();

  यदि (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    प्रत्यागम scanDocumentIndicator(सत्य);

  यदि (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    प्रत्यागम scanDocumentIndicator(असत्य);

  यदि (*Current == '[')
    प्रत्यागम scanFlowCollectionStart(सत्य);

  यदि (*Current == '{')
    प्रत्यागम scanFlowCollectionStart(असत्य);

  यदि (*Current == ']')
    प्रत्यागम scanFlowCollectionEnd(सत्य);

  यदि (*Current == '}')
    प्रत्यागम scanFlowCollectionEnd(असत्य);

  यदि (*Current == ',')
    प्रत्यागम scanFlowEntry();

  यदि (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    प्रत्यागम scanBlockEntry();

  यदि (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    प्रत्यागम scan_कुंजी();

  यदि (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || Is_Adjacent_मूल्य_Allowed_In_Flow))
    प्रत्यागम scan_मूल्य();

  यदि (*Current == '*')
    प्रत्यागम scanAliasOrAnchor(सत्य);

  यदि (*Current == '&')
    प्रत्यागम scanAliasOrAnchor(असत्य);

  यदि (*Current == '!')
    प्रत्यागम scanTag();

  यदि (*Current == '|' && !FlowLevel)
    प्रत्यागम scanBlockScalar(सत्य);

  यदि (*Current == '>' && !FlowLevel)
    प्रत्यागम scanBlockScalar(असत्य);

  यदि (*Current == '\'')
    प्रत्यागम scanFlowScalar(असत्य);

  यदि (*Current == '"')
    प्रत्यागम scanFlowScalar(सत्य);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  यदि ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    प्रत्यागम scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  प्रत्यागम असत्य;
}

Stream::Stream(StringRef निवेश, SourceMgr &SM, तार्किक ShowColors,
               std::error_code *EC)
    : scanner(नूतन Scanner(निवेश, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef निवेश_Buffer, SourceMgr &SM, तार्किक ShowColors,
               std::error_code *EC)
    : scanner(नूतन Scanner(निवेश_Buffer, SM, ShowColors, EC)) {}

Stream::~Stream() = स्वभाव;

तार्किक Stream::failed() { प्रत्यागम scanner->failed(); }

शून्य Stream::printError(गाँठ *N, ध्रुव Twine &संदेश_2, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), संदेश_2, Kind);
}

शून्य Stream::printError(ध्रुव SMRange &Range, ध्रुव Twine &संदेश_2,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, संदेश_2, Range);
}

document_iterator Stream::begin() {
  यदि (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(नूतन Document(*एतत्));
  प्रत्यागम document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  प्रत्यागम document_iterator();
}

शून्य Stream::skip() {
  प्रति (Document &Doc : *एतत्)
    Doc.skip();
}

गाँठ::गाँठ(अचिन्हित पूर्णाङ्क Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string गाँठ::getVerbatimTag() ध्रुव {
  StringRef Raw = getRawTag();
  यदि (!Raw.empty() && Raw != "!") {
    std::string Ret;
    यदि (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      प्रत्यागम Ret;
    } अन्यथा यदि (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      प्रत्यागम Ret;
    } अन्यथा {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      यदि (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      अन्यथा {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      प्रत्यागम Ret;
    }
  }

  विकल्प (getType()) {
  पक्ष NK_Null:
    प्रत्यागम "tag:yaml.org,2002:null";
  पक्ष NK_Scalar:
  पक्ष NK_BlockScalar:
    // TODO: Tag resolution.
    प्रत्यागम "tag:yaml.org,2002:str";
  पक्ष NK_Mapping:
    प्रत्यागम "tag:yaml.org,2002:map";
  पक्ष NK_Sequence:
    प्रत्यागम "tag:yaml.org,2002:seq";
  }

  प्रत्यागम "";
}

Token &गाँठ::peekNext() {
  प्रत्यागम Doc->peekNext();
}

Token गाँठ::getNext() {
  प्रत्यागम Doc->getNext();
}

गाँठ *गाँठ::parse_Block_गाँठ() {
  प्रत्यागम Doc->parse_Block_गाँठ();
}

BumpPtrAllocator &गाँठ::getAllocator() {
  प्रत्यागम Doc->गाँठ_Allocator;
}

शून्य गाँठ::setError(ध्रुव Twine &संदेश_2, Token &Tok) ध्रुव {
  Doc->setError(संदेश_2, Tok);
}

तार्किक गाँठ::failed() ध्रुव {
  प्रत्यागम Doc->failed();
}

StringRef Scalar_गाँठ::get_मूल्य(SmallVectorImpl<वर्ण> &Storage) ध्रुव {
  यदि (मूल्य[0] == '"')
    प्रत्यागम get_Double_Quoted_मूल्य(मूल्य, Storage);
  यदि (मूल्य[0] == '\'')
    प्रत्यागम get_Single_Quoted_मूल्य(मूल्य, Storage);
  प्रत्यागम get_Plain_मूल्य(मूल्य, Storage);
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
parse_Scalar_मूल्य(StringRef Unquoted_मूल्य, SmallVectorImpl<वर्ण> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<वर्ण> &)>
                     UnescapeCallback) {
  माप_प्रकार I = Unquoted_मूल्य.find_first_of(LookupChars);
  यदि (I == StringRef::npos)
    प्रत्यागम Unquoted_मूल्य;

  Storage.clear();
  Storage.reserve(Unquoted_मूल्य.size());
  वर्ण Last_New_पंक्ति_Added_As = '\0';
  प्रति (; I != StringRef::npos; I = Unquoted_मूल्य.find_first_of(LookupChars)) {
    यदि (Unquoted_मूल्य[I] != '\r' && Unquoted_मूल्य[I] != '\n') {
      llvm::append_range(Storage, Unquoted_मूल्य.take_front(I));
      Unquoted_मूल्य = UnescapeCallback(Unquoted_मूल्य.drop_front(I), Storage);
      Last_New_पंक्ति_Added_As = '\0';
      अनुवर्त;
    }
    यदि (माप_प्रकार LastNonSWhite = Unquoted_मूल्य.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_मूल्य.take_front(LastNonSWhite + 1));
      Storage.धकेलो_back(' ');
      Last_New_पंक्ति_Added_As = ' ';
    } अन्यथा {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      विकल्प (Last_New_पंक्ति_Added_As) {
      पक्ष ' ':
        प्रतिज्ञा(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        Last_New_पंक्ति_Added_As = '\n';
        भङ्ग;
      पक्ष '\n':
        प्रतिज्ञा(!Storage.empty() && Storage.back() == '\n');
        Storage.धकेलो_back('\n');
        भङ्ग;
      स्वभाव:
        Storage.धकेलो_back(' ');
        Last_New_पंक्ति_Added_As = ' ';
        भङ्ग;
      }
    }
    // Handle Windows-style EOL
    यदि (Unquoted_मूल्य.substr(I, 2) == "\r\n")
      I++;
    Unquoted_मूल्य = Unquoted_मूल्य.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_मूल्य);
  प्रत्यागम StringRef(Storage.begin(), Storage.size());
}

StringRef
Scalar_गाँठ::get_Double_Quoted_मूल्य(StringRef Raw_मूल्य,
                                 SmallVectorImpl<वर्ण> &Storage) ध्रुव {
  प्रतिज्ञा(Raw_मूल्य.size() >= 2 && Raw_मूल्य.front() == '"' &&
         Raw_मूल्य.back() == '"');
  StringRef Unquoted_मूल्य = Raw_मूल्य.substr(1, Raw_मूल्य.size() - 2);

  स्वतः UnescapeFunc = [एतत्](StringRef Unquoted_मूल्य,
                             SmallVectorImpl<वर्ण> &Storage) {
    प्रतिज्ञा(Unquoted_मूल्य.take_front(1) == "\\");
    यदि (Unquoted_मूल्य.size() == 1) {
      Token T;
      T.Range = Unquoted_मूल्य;
      setError("Unrecognized escape code", T);
      Storage.clear();
      प्रत्यागम StringRef();
    }
    Unquoted_मूल्य = Unquoted_मूल्य.drop_front(1);
    विकल्प (Unquoted_मूल्य[0]) {
    स्वभाव: {
      Token T;
      T.Range = Unquoted_मूल्य.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      प्रत्यागम StringRef();
    }
    पक्ष '\r':
      // Shrink the Windows-style EOL.
      यदि (Unquoted_मूल्य.size() >= 2 && Unquoted_मूल्य[1] == '\n')
        Unquoted_मूल्य = Unquoted_मूल्य.drop_front(1);
      [[fallthrough]];
    पक्ष '\n':
      प्रत्यागम Unquoted_मूल्य.drop_front(1).ltrim(" \t");
    पक्ष '0':
      Storage.धकेलो_back(0x00);
      भङ्ग;
    पक्ष 'a':
      Storage.धकेलो_back(0x07);
      भङ्ग;
    पक्ष 'b':
      Storage.धकेलो_back(0x08);
      भङ्ग;
    पक्ष 't':
    पक्ष 0x09:
      Storage.धकेलो_back(0x09);
      भङ्ग;
    पक्ष 'n':
      Storage.धकेलो_back(0x0A);
      भङ्ग;
    पक्ष 'v':
      Storage.धकेलो_back(0x0B);
      भङ्ग;
    पक्ष 'f':
      Storage.धकेलो_back(0x0C);
      भङ्ग;
    पक्ष 'r':
      Storage.धकेलो_back(0x0D);
      भङ्ग;
    पक्ष 'e':
      Storage.धकेलो_back(0x1B);
      भङ्ग;
    पक्ष ' ':
      Storage.धकेलो_back(0x20);
      भङ्ग;
    पक्ष '"':
      Storage.धकेलो_back(0x22);
      भङ्ग;
    पक्ष '/':
      Storage.धकेलो_back(0x2F);
      भङ्ग;
    पक्ष '\\':
      Storage.धकेलो_back(0x5C);
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
      यदि (Unquoted_मूल्य.size() < 3)
        // TODO: Report error.
        भङ्ग;
      अचिन्हित पूर्णाङ्क Unicode_Scalar_मूल्य;
      यदि (Unquoted_मूल्य.substr(1, 2).getAsInteger(16, Unicode_Scalar_मूल्य))
        // TODO: Report error.
        Unicode_Scalar_मूल्य = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मूल्य, Storage);
      प्रत्यागम Unquoted_मूल्य.drop_front(3);
    }
    पक्ष 'u': {
      यदि (Unquoted_मूल्य.size() < 5)
        // TODO: Report error.
        भङ्ग;
      अचिन्हित पूर्णाङ्क Unicode_Scalar_मूल्य;
      यदि (Unquoted_मूल्य.substr(1, 4).getAsInteger(16, Unicode_Scalar_मूल्य))
        // TODO: Report error.
        Unicode_Scalar_मूल्य = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मूल्य, Storage);
      प्रत्यागम Unquoted_मूल्य.drop_front(5);
    }
    पक्ष 'U': {
      यदि (Unquoted_मूल्य.size() < 9)
        // TODO: Report error.
        भङ्ग;
      अचिन्हित पूर्णाङ्क Unicode_Scalar_मूल्य;
      यदि (Unquoted_मूल्य.substr(1, 8).getAsInteger(16, Unicode_Scalar_मूल्य))
        // TODO: Report error.
        Unicode_Scalar_मूल्य = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मूल्य, Storage);
      प्रत्यागम Unquoted_मूल्य.drop_front(9);
    }
    }
    प्रत्यागम Unquoted_मूल्य.drop_front(1);
  };

  प्रत्यागम parse_Scalar_मूल्य(Unquoted_मूल्य, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_गाँठ::get_Single_Quoted_मूल्य(StringRef Raw_मूल्य,
                                           SmallVectorImpl<वर्ण> &Storage) {
  प्रतिज्ञा(Raw_मूल्य.size() >= 2 && Raw_मूल्य.front() == '\'' &&
         Raw_मूल्य.back() == '\'');
  StringRef Unquoted_मूल्य = Raw_मूल्य.substr(1, Raw_मूल्य.size() - 2);

  स्वतः UnescapeFunc = [](StringRef Unquoted_मूल्य,
                         SmallVectorImpl<वर्ण> &Storage) {
    प्रतिज्ञा(Unquoted_मूल्य.take_front(2) == "''");
    Storage.धकेलो_back('\'');
    प्रत्यागम Unquoted_मूल्य.drop_front(2);
  };

  प्रत्यागम parse_Scalar_मूल्य(Unquoted_मूल्य, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_गाँठ::get_Plain_मूल्य(StringRef Raw_मूल्य,
                                    SmallVectorImpl<वर्ण> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_मूल्य = Raw_मूल्य.rtrim("\r\n \t");
  प्रत्यागम parse_Scalar_मूल्य(Raw_मूल्य, Storage, "\r\n", nullptr);
}

गाँठ *कुंजी_मूल्य_गाँठ::get_कुंजी() {
  यदि (कुंजी)
    प्रत्यागम कुंजी;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    यदि (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_मूल्य
        || t.Kind == Token::TK_Error) {
      प्रत्यागम कुंजी = नूतन (getAllocator()) Null_गाँठ(Doc);
    }
    यदि (t.Kind == Token::TK_कुंजी)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  यदि (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_मूल्य) {
    प्रत्यागम कुंजी = नूतन (getAllocator()) Null_गाँठ(Doc);
  }

  // We've got a normal key.
  प्रत्यागम कुंजी = parse_Block_गाँठ();
}

गाँठ *कुंजी_मूल्य_गाँठ::get_मूल्य() {
  यदि (मूल्य)
    प्रत्यागम मूल्य;

  यदि (गाँठ* कुंजी = get_कुंजी())
    कुंजी->skip();
  अन्यथा {
    setError("Null key in Key Value.", peekNext());
    प्रत्यागम मूल्य = नूतन (getAllocator()) Null_गाँठ(Doc);
  }

  यदि (failed())
    प्रत्यागम मूल्य = नूतन (getAllocator()) Null_गाँठ(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    यदि (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_कुंजी
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      प्रत्यागम मूल्य = नूतन (getAllocator()) Null_गाँठ(Doc);
    }

    यदि (t.Kind != Token::TK_मूल्य) {
      setError("Unexpected token in Key Value.", t);
      प्रत्यागम मूल्य = नूतन (getAllocator()) Null_गाँठ(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  यदि (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_कुंजी) {
    प्रत्यागम मूल्य = नूतन (getAllocator()) Null_गाँठ(Doc);
  }

  // We got a normal value.
  प्रत्यागम मूल्य = parse_Block_गाँठ();
}

शून्य Mapping_गाँठ::increment() {
  यदि (failed()) {
    IsAtEnd = सत्य;
    CurrentEntry = nullptr;
    प्रत्यागम;
  }
  यदि (CurrentEntry) {
    CurrentEntry->skip();
    यदि (Type == MT_Inline) {
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      प्रत्यागम;
    }
  }
  Token T = peekNext();
  यदि (T.Kind == Token::TK_कुंजी || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = नूतन (getAllocator()) कुंजी_मूल्य_गाँठ(Doc);
  } अन्यथा यदि (Type == MT_Block) {
    विकल्प (T.Kind) {
    पक्ष Token::TK_BlockEnd:
      getNext();
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      भङ्ग;
    स्वभाव:
      setError("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    पक्ष Token::TK_Error:
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
    }
  } अन्यथा {
    विकल्प (T.Kind) {
    पक्ष Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      प्रत्यागम increment();
    पक्ष Token::TK_FlowMappingEnd:
      getNext();
      [[fallthrough]];
    पक्ष Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      भङ्ग;
    स्वभाव:
      setError( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
    }
  }
}

शून्य Sequence_गाँठ::increment() {
  यदि (failed()) {
    IsAtEnd = सत्य;
    CurrentEntry = nullptr;
    प्रत्यागम;
  }
  यदि (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  यदि (SeqType == ST_Block) {
    विकल्प (T.Kind) {
    पक्ष Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_गाँठ();
      यदि (!CurrentEntry) { // An error occurred.
        IsAtEnd = सत्य;
        CurrentEntry = nullptr;
      }
      भङ्ग;
    पक्ष Token::TK_BlockEnd:
      getNext();
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      भङ्ग;
    स्वभाव:
      setError( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    पक्ष Token::TK_Error:
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
    }
  } अन्यथा यदि (SeqType == ST_Indentless) {
    विकल्प (T.Kind) {
    पक्ष Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_गाँठ();
      यदि (!CurrentEntry) { // An error occurred.
        IsAtEnd = सत्य;
        CurrentEntry = nullptr;
      }
      भङ्ग;
    स्वभाव:
    पक्ष Token::TK_Error:
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
    }
  } अन्यथा यदि (SeqType == ST_Flow) {
    विकल्प (T.Kind) {
    पक्ष Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      WasPreviousTokenFlowEntry = सत्य;
      प्रत्यागम increment();
    पक्ष Token::TK_FlowSequenceEnd:
      getNext();
      [[fallthrough]];
    पक्ष Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      भङ्ग;
    पक्ष Token::TK_StreamEnd:
    पक्ष Token::TK_DocumentEnd:
    पक्ष Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      भङ्ग;
    स्वभाव:
      यदि (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = सत्य;
        CurrentEntry = nullptr;
        भङ्ग;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parse_Block_गाँठ();
      यदि (!CurrentEntry) {
        IsAtEnd = सत्य;
      }
      WasPreviousTokenFlowEntry = असत्य;
      भङ्ग;
    }
  }
}

Document::Document(Stream &S) : stream(S), मूल_2(nullptr) {
  // Tag maps starts with two default mappings.
  TagMap["!"] = "!";
  TagMap["!!"] = "tag:yaml.org,2002:";

  यदि (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peekNext();
  यदि (T.Kind == Token::TK_DocumentStart)
    getNext();
}

तार्किक Document::skip()  {
  यदि (stream.scanner->failed())
    प्रत्यागम असत्य;
  यदि (!मूल_2 && !get_मूल())
    प्रत्यागम असत्य;
  मूल_2->skip();
  Token &T = peekNext();
  यदि (T.Kind == Token::TK_StreamEnd)
    प्रत्यागम असत्य;
  यदि (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    प्रत्यागम skip();
  }
  प्रत्यागम सत्य;
}

Token &Document::peekNext() {
  प्रत्यागम stream.scanner->peekNext();
}

Token Document::getNext() {
  प्रत्यागम stream.scanner->getNext();
}

शून्य Document::setError(ध्रुव Twine &Message, Token &Location) ध्रुव {
  stream.scanner->setError(Message, Location.Range.begin());
}

तार्किक Document::failed() ध्रुव {
  प्रत्यागम stream.scanner->failed();
}

गाँठ *Document::parse_Block_गाँठ() {
  Token T = peekNext();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  विकल्प (T.Kind) {
  पक्ष Token::TK_Alias:
    getNext();
    प्रत्यागम नूतन (गाँठ_Allocator) Alias_गाँठ(stream.CurrentDoc, T.Range.substr(1));
  पक्ष Token::TK_Anchor:
    यदि (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      प्रत्यागम nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    गच्छ parse_property;
  पक्ष Token::TK_Tag:
    यदि (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      प्रत्यागम nullptr;
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
    प्रत्यागम नूतन (गाँठ_Allocator) Sequence_गाँठ( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , Sequence_गाँठ::ST_Indentless);
  पक्ष Token::TK_BlockSequenceStart:
    getNext();
    प्रत्यागम नूतन (गाँठ_Allocator)
      Sequence_गाँठ( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_गाँठ::ST_Block);
  पक्ष Token::TK_BlockMappingStart:
    getNext();
    प्रत्यागम नूतन (गाँठ_Allocator)
      Mapping_गाँठ( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_गाँठ::MT_Block);
  पक्ष Token::TK_FlowSequenceStart:
    getNext();
    प्रत्यागम नूतन (गाँठ_Allocator)
      Sequence_गाँठ( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_गाँठ::ST_Flow);
  पक्ष Token::TK_FlowMappingStart:
    getNext();
    प्रत्यागम नूतन (गाँठ_Allocator)
      Mapping_गाँठ( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_गाँठ::MT_Flow);
  पक्ष Token::TK_Scalar:
    getNext();
    प्रत्यागम नूतन (गाँठ_Allocator)
      Scalar_गाँठ( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  पक्ष Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.मूल्य.c_str(), T.मूल्य.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(गाँठ_Allocator).drop_back();
    प्रत्यागम नूतन (गाँठ_Allocator)
        Block_Scalar_गाँठ(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  पक्ष Token::TK_कुंजी:
    // Don't eat the TK_Key, KeyValueNode expects it.
    प्रत्यागम नूतन (गाँठ_Allocator)
      Mapping_गाँठ( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_गाँठ::MT_Inline);
  पक्ष Token::TK_DocumentStart:
  पक्ष Token::TK_DocumentEnd:
  पक्ष Token::TK_StreamEnd:
  स्वभाव:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    प्रत्यागम नूतन (गाँठ_Allocator) Null_गाँठ(stream.CurrentDoc);
  पक्ष Token::TK_FlowMappingEnd:
  पक्ष Token::TK_FlowSequenceEnd:
  पक्ष Token::TK_FlowEntry: {
    यदि (मूल_2 && (isa<Mapping_गाँठ>(मूल_2) || isa<Sequence_गाँठ>(मूल_2)))
      प्रत्यागम नूतन (गाँठ_Allocator) Null_गाँठ(stream.CurrentDoc);

    setError("Unexpected token", T);
    प्रत्यागम nullptr;
  }
  पक्ष Token::TK_Error:
    प्रत्यागम nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  प्रत्यागम nullptr;
}

तार्किक Document::parseDirectives() {
  तार्किक isDirective = असत्य;
  यावत् (सत्य) {
    Token T = peekNext();
    यदि (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = सत्य;
    } अन्यथा यदि (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = सत्य;
    } अन्यथा {
      भङ्ग;
    }
  }
  प्रत्यागम isDirective;
}

शून्य Document::parseYAMLDirective() {
  getNext(); // Eat %YAML <version>
}

शून्य Document::parseTAGDirective() {
  Token Tag = getNext(); // %TAG <handle> <prefix>
  StringRef T = Tag.Range;
  // Strip %TAG
  T = T.substr(T.find_first_of(" \t")).ltrim(" \t");
  std::माप_प्रकार HandleEnd = T.find_first_of(" \t");
  StringRef TagHandle = T.substr(0, HandleEnd);
  StringRef TagPrefix = T.substr(HandleEnd).ltrim(" \t");
  TagMap[TagHandle] = TagPrefix;
}

तार्किक Document::expectToken(पूर्णाङ्क TK) {
  Token T = getNext();
  यदि (T.Kind != TK) {
    setError("Unexpected token", T);
    प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य;
}
