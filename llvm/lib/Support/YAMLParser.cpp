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
स्थिर EncodingInfo getUnicodeEncoding(StringRef आगत) {
  यदि (आगत.empty())
    फर्काऊ {UEF_Unknown, 0};

  विकल्प (uint8_t(आगत[0])) {
  पक्ष 0x00:
    यदि (आगत.size() >= 4) {
      यदि (  आगत[1] == 0
         && uint8_t(आगत[2]) == 0xFE
         && uint8_t(आगत[3]) == 0xFF)
        फर्काऊ {UEF_UTF32_BE, 4};
      यदि (आगत[1] == 0 && आगत[2] == 0 && आगत[3] != 0)
        फर्काऊ {UEF_UTF32_BE, 0};
    }

    यदि (आगत.size() >= 2 && आगत[1] != 0)
      फर्काऊ {UEF_UTF16_BE, 0};
    फर्काऊ {UEF_Unknown, 0};
  पक्ष 0xFF:
    यदि (  आगत.size() >= 4
       && uint8_t(आगत[1]) == 0xFE
       && आगत[2] == 0
       && आगत[3] == 0)
      फर्काऊ {UEF_UTF32_LE, 4};

    यदि (आगत.size() >= 2 && uint8_t(आगत[1]) == 0xFE)
      फर्काऊ {UEF_UTF16_LE, 2};
    फर्काऊ {UEF_Unknown, 0};
  पक्ष 0xFE:
    यदि (आगत.size() >= 2 && uint8_t(आगत[1]) == 0xFF)
      फर्काऊ {UEF_UTF16_BE, 2};
    फर्काऊ {UEF_Unknown, 0};
  पक्ष 0xEF:
    यदि (  आगत.size() >= 3
       && uint8_t(आगत[1]) == 0xBB
       && uint8_t(आगत[2]) == 0xBF)
      फर्काऊ {UEF_UTF8, 3};
    फर्काऊ {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  यदि (आगत.size() >= 4 && आगत[1] == 0 && आगत[2] == 0 && आगत[3] == 0)
    फर्काऊ {UEF_UTF32_LE, 0};

  यदि (आगत.size() >= 2 && आगत[1] == 0)
    फर्काऊ {UEF_UTF16_LE, 0};

  फर्काऊ {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
शून्य गाँठो::anchor() {}
शून्य Null_गाँठो::anchor() {}
शून्य Scalar_गाँठो::anchor() {}
शून्य Block_Scalar_गाँठो::anchor() {}
शून्य साँचो_मान_गाँठो::anchor() {}
शून्य Mapping_गाँठो::anchor() {}
शून्य Sequence_गाँठो::anchor() {}
शून्य Alias_गाँठो::anchor() {}

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
    TK_साँचो,
    TK_मान,
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
  std::string मान;

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
संरचना Simple_साँचो {
  TokenQueueT::iterator Tok;
  अचिन्हित Column = 0;
  अचिन्हित पङ्क्ति = 0;
  अचिन्हित FlowLevel = 0;
  तार्किक IsRequired = असत्य;

  तार्किक चालक ==(अचर Simple_साँचो &Other) {
    फर्काऊ Tok == Other.Tok;
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
    फर्काऊ {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  यदि (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    यदि (codepoint >= 0x80)
      फर्काऊ {codepoint, 2};
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
      फर्काऊ {codepoint, 3};
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
      फर्काऊ {codepoint, 4};
  }
  फर्काऊ {0, 0};
}

नामाकाश llvm {
नामाकाश yaml {

/// Scans YAML tokens from a MemoryBuffer.
वर्ग Scanner {
सार्वजनिक:
  Scanner(StringRef आगत, SourceMgr &SM, तार्किक ShowColors = सत्य,
          std::error_code *EC = nullptr);
  Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, तार्किक ShowColors = सत्य,
          std::error_code *EC = nullptr);

  /// Parse the next token and return it without popping it.
  Token &peekNext();

  /// Parse the next token and pop it from the queue.
  Token getNext();

  शून्य printError(SMLoc Loc, SourceMgr::DiagKind Kind, अचर Twine &Message,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.PrintMessage(Loc, Kind, Message, Ranges, /* FixIts= */ {}, ShowColors);
  }

  शून्य setError(अचर Twine &Message, StringRef::iterator Position) {
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
    फर्काऊ Failed;
  }

गुप्त:
  शून्य init(MemoryBufferRef Buffer);

  StringRef current_आगत() {
    फर्काऊ StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    फर्काऊ ::decodeUTF8(StringRef(Position, End - Position));
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
  तार्किक is_पङ्क्ति_Empty(StringRef पङ्क्ति);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  तार्किक consume_पङ्क्ति_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  शून्य save_Simple_साँचो_Candidate( TokenQueueT::iterator Tok
                             , अचिन्हित AtColumn
                             , तार्किक IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  शून्य remove_Stale_Simple_साँचो_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  शून्य remove_Simple_साँचो_Candidates_On_Flow_Level(अचिन्हित Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  तार्किक unrollIndent(पूर्णाङ्क ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  तार्किक rollIndent( पूर्णाङ्क ToColumn
                 , Token::TokenKind Kind
                 , TokenQueueT::iterator घुसाउ_Point);

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
  तार्किक scan_साँचो();

  /// Scan an explicit : indicating a value.
  तार्किक scan_मान();

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
                             अचिन्हित &पङ्क्ति_Breaks, तार्किक &IsDone);

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
  MemoryBufferRef आगत_Buffer;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator End;

  /// Current YAML indentation level in spaces.
  पूर्णाङ्क Indent;

  /// Current column number in Unicode code points.
  अचिन्हित Column;

  /// Current line number.
  अचिन्हित पङ्क्ति;

  /// How deep we are in flow style containers. 0 Means at block level.
  अचिन्हित FlowLevel;

  /// Are we at the start of the stream?
  तार्किक IsStartOfStream;

  /// Can the next token be the start of a simple key?
  तार्किक Is_Simple_साँचो_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  तार्किक Is_Adjacent_मान_Allowed_In_Flow;

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
  SmallVector<Simple_साँचो, 4> Simple_साँचोहरू;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
स्थिर शून्य encodeUTF8( uint32_t Unicode_Scalar_मान
                      , SmallVectorImpl<वर्ण> &Result) {
  यदि (Unicode_Scalar_मान <= 0x7F) {
    Result.थप_back(Unicode_Scalar_मान & 0x7F);
  } अन्यथा यदि (Unicode_Scalar_मान <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((Unicode_Scalar_मान & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Unicode_Scalar_मान & 0x3F);
    Result.थप_back(FirstByte);
    Result.थप_back(SecondByte);
  } अन्यथा यदि (Unicode_Scalar_मान <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((Unicode_Scalar_मान & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_मान & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Unicode_Scalar_मान & 0x3F);
    Result.थप_back(FirstByte);
    Result.थप_back(SecondByte);
    Result.थप_back(ThirdByte);
  } अन्यथा यदि (Unicode_Scalar_मान <= 0x10FFFF) {
    uint8_t FirstByte = 0xF0 | ((Unicode_Scalar_मान & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_मान & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Unicode_Scalar_मान & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_मान & 0x3F);
    Result.थप_back(FirstByte);
    Result.थप_back(SecondByte);
    Result.थप_back(ThirdByte);
    Result.थप_back(FourthByte);
  }
}

तार्किक yaml::dumpTokens(StringRef आगत, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(आगत, SM);
  जबसम्म (सत्य) {
    Token T = scanner.getNext();
    विकल्प (T.Kind) {
    पक्ष Token::TK_StreamStart:
      OS << "Stream-Start: ";
      रोक;
    पक्ष Token::TK_StreamEnd:
      OS << "Stream-End: ";
      रोक;
    पक्ष Token::TK_VersionDirective:
      OS << "Version-Directive: ";
      रोक;
    पक्ष Token::TK_TagDirective:
      OS << "Tag-Directive: ";
      रोक;
    पक्ष Token::TK_DocumentStart:
      OS << "Document-Start: ";
      रोक;
    पक्ष Token::TK_DocumentEnd:
      OS << "Document-End: ";
      रोक;
    पक्ष Token::TK_BlockEntry:
      OS << "Block-Entry: ";
      रोक;
    पक्ष Token::TK_BlockEnd:
      OS << "Block-End: ";
      रोक;
    पक्ष Token::TK_BlockSequenceStart:
      OS << "Block-Sequence-Start: ";
      रोक;
    पक्ष Token::TK_BlockMappingStart:
      OS << "Block-Mapping-Start: ";
      रोक;
    पक्ष Token::TK_FlowEntry:
      OS << "Flow-Entry: ";
      रोक;
    पक्ष Token::TK_FlowSequenceStart:
      OS << "Flow-Sequence-Start: ";
      रोक;
    पक्ष Token::TK_FlowSequenceEnd:
      OS << "Flow-Sequence-End: ";
      रोक;
    पक्ष Token::TK_FlowMappingStart:
      OS << "Flow-Mapping-Start: ";
      रोक;
    पक्ष Token::TK_FlowMappingEnd:
      OS << "Flow-Mapping-End: ";
      रोक;
    पक्ष Token::TK_साँचो:
      OS << "Key: ";
      रोक;
    पक्ष Token::TK_मान:
      OS << "Value: ";
      रोक;
    पक्ष Token::TK_Scalar:
      OS << "Scalar: ";
      रोक;
    पक्ष Token::TK_BlockScalar:
      OS << "Block Scalar: ";
      रोक;
    पक्ष Token::TK_Alias:
      OS << "Alias: ";
      रोक;
    पक्ष Token::TK_Anchor:
      OS << "Anchor: ";
      रोक;
    पक्ष Token::TK_Tag:
      OS << "Tag: ";
      रोक;
    पक्ष Token::TK_Error:
      रोक;
    }
    OS << T.Range << "\n";
    यदि (T.Kind == Token::TK_StreamEnd)
      रोक;
    अन्यथा यदि (T.Kind == Token::TK_Error)
      फर्काऊ असत्य;
  }
  फर्काऊ सत्य;
}

तार्किक yaml::scanTokens(StringRef आगत) {
  SourceMgr SM;
  Scanner scanner(आगत, SM);
  जबसम्म (सत्य) {
    Token T = scanner.getNext();
    यदि (T.Kind == Token::TK_StreamEnd)
      रोक;
    अन्यथा यदि (T.Kind == Token::TK_Error)
      फर्काऊ असत्य;
  }
  फर्काऊ सत्य;
}

std::string yaml::escape(StringRef आगत, तार्किक EscapePrintable) {
  std::string Escaped_आगत;
  लागि (StringRef::iterator i = आगत.begin(), e = आगत.end(); i != e; ++i) {
    यदि (*i == '\\')
      Escaped_आगत += "\\\\";
    अन्यथा यदि (*i == '"')
      Escaped_आगत += "\\\"";
    अन्यथा यदि (*i == 0)
      Escaped_आगत += "\\0";
    अन्यथा यदि (*i == 0x07)
      Escaped_आगत += "\\a";
    अन्यथा यदि (*i == 0x08)
      Escaped_आगत += "\\b";
    अन्यथा यदि (*i == 0x09)
      Escaped_आगत += "\\t";
    अन्यथा यदि (*i == 0x0A)
      Escaped_आगत += "\\n";
    अन्यथा यदि (*i == 0x0B)
      Escaped_आगत += "\\v";
    अन्यथा यदि (*i == 0x0C)
      Escaped_आगत += "\\f";
    अन्यथा यदि (*i == 0x0D)
      Escaped_आगत += "\\r";
    अन्यथा यदि (*i == 0x1B)
      Escaped_आगत += "\\e";
    अन्यथा यदि ((अचिन्हित वर्ण)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_आगत += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } अन्यथा यदि (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_मान
        = decodeUTF8(StringRef(i, आगत.end() - i));
      यदि (Unicode_Scalar_मान.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_आगत, Val);
        // FIXME: Error reporting.
        फर्काऊ Escaped_आगत;
      }
      यदि (Unicode_Scalar_मान.first == 0x85)
        Escaped_आगत += "\\N";
      अन्यथा यदि (Unicode_Scalar_मान.first == 0xA0)
        Escaped_आगत += "\\_";
      अन्यथा यदि (Unicode_Scalar_मान.first == 0x2028)
        Escaped_आगत += "\\L";
      अन्यथा यदि (Unicode_Scalar_मान.first == 0x2029)
        Escaped_आगत += "\\P";
      अन्यथा यदि (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_मान.first))
        Escaped_आगत += StringRef(i, Unicode_Scalar_मान.second);
      अन्यथा {
        std::string HexStr = utohexstr(Unicode_Scalar_मान.first);
        यदि (HexStr.size() <= 2)
          Escaped_आगत += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        अन्यथा यदि (HexStr.size() <= 4)
          Escaped_आगत += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        अन्यथा यदि (HexStr.size() <= 8)
          Escaped_आगत += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += Unicode_Scalar_मान.second - 1;
    } अन्यथा {
      Escaped_आगत.थप_back(*i);
    }
  }
  फर्काऊ Escaped_आगत;
}

std::optional<तार्किक> yaml::parseBool(StringRef S) {
  विकल्प (S.size()) {
  पक्ष 1:
    विकल्प (S.front()) {
    पक्ष 'y':
    पक्ष 'Y':
      फर्काऊ सत्य;
    पक्ष 'n':
    पक्ष 'N':
      फर्काऊ असत्य;
    स्वभाव:
      फर्काऊ std::nullopt;
    }
  पक्ष 2:
    विकल्प (S.front()) {
    पक्ष 'O':
      यदि (S[1] == 'N') // ON
        फर्काऊ सत्य;
      [[fallthrough]];
    पक्ष 'o':
      यदि (S[1] == 'n') //[Oo]n
        फर्काऊ सत्य;
      फर्काऊ std::nullopt;
    पक्ष 'N':
      यदि (S[1] == 'O') // NO
        फर्काऊ असत्य;
      [[fallthrough]];
    पक्ष 'n':
      यदि (S[1] == 'o') //[Nn]o
        फर्काऊ असत्य;
      फर्काऊ std::nullopt;
    स्वभाव:
      फर्काऊ std::nullopt;
    }
  पक्ष 3:
    विकल्प (S.front()) {
    पक्ष 'O':
      यदि (S.drop_front() == "FF") // OFF
        फर्काऊ असत्य;
      [[fallthrough]];
    पक्ष 'o':
      यदि (S.drop_front() == "ff") //[Oo]ff
        फर्काऊ असत्य;
      फर्काऊ std::nullopt;
    पक्ष 'Y':
      यदि (S.drop_front() == "ES") // YES
        फर्काऊ सत्य;
      [[fallthrough]];
    पक्ष 'y':
      यदि (S.drop_front() == "es") //[Yy]es
        फर्काऊ सत्य;
      फर्काऊ std::nullopt;
    स्वभाव:
      फर्काऊ std::nullopt;
    }
  पक्ष 4:
    विकल्प (S.front()) {
    पक्ष 'T':
      यदि (S.drop_front() == "RUE") // TRUE
        फर्काऊ सत्य;
      [[fallthrough]];
    पक्ष 't':
      यदि (S.drop_front() == "rue") //[Tt]rue
        फर्काऊ सत्य;
      फर्काऊ std::nullopt;
    स्वभाव:
      फर्काऊ std::nullopt;
    }
  पक्ष 5:
    विकल्प (S.front()) {
    पक्ष 'F':
      यदि (S.drop_front() == "ALSE") // FALSE
        फर्काऊ असत्य;
      [[fallthrough]];
    पक्ष 'f':
      यदि (S.drop_front() == "alse") //[Ff]alse
        फर्काऊ असत्य;
      फर्काऊ std::nullopt;
    स्वभाव:
      फर्काऊ std::nullopt;
    }
  स्वभाव:
    फर्काऊ std::nullopt;
  }
}

Scanner::Scanner(StringRef आगत, SourceMgr &sm, तार्किक ShowColors,
                 std::error_code *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(MemoryBufferRef(आगत, "YAML"));
}

Scanner::Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, तार्किक ShowColors,
                 std::error_code *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(Buffer);
}

शून्य Scanner::init(MemoryBufferRef Buffer) {
  आगत_Buffer = Buffer;
  Current = आगत_Buffer.getBufferStart();
  End = आगत_Buffer.getBufferEnd();
  Indent = -1;
  Column = 0;
  पङ्क्ति = 0;
  FlowLevel = 0;
  IsStartOfStream = सत्य;
  Is_Simple_साँचो_Allowed = सत्य;
  Is_Adjacent_मान_Allowed_In_Flow = असत्य;
  Failed = असत्य;
  std::unique_ptr<MemoryBuffer> आगत_Buffer_Owner =
      MemoryBuffer::getMemBuffer(Buffer, /*RequiresNullTerminator=*/असत्य);
  SM.AddNewSourceBuffer(std::move(आगत_Buffer_Owner), SMLoc());
}

Token &Scanner::peekNext() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  तार्किक NeedMore = असत्य;
  जबसम्म (सत्य) {
    यदि (TokenQueue.empty() || NeedMore) {
      यदि (!fetchMoreTokens()) {
        TokenQueue.clear();
        Simple_साँचोहरू.clear();
        TokenQueue.थप_back(Token());
        फर्काऊ TokenQueue.front();
      }
    }
    प्रतिज्ञा(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_साँचो_Candidates();
    Simple_साँचो SK;
    SK.Tok = TokenQueue.begin();
    यदि (!is_contained(Simple_साँचोहरू, SK))
      रोक;
    अन्यथा
      NeedMore = सत्य;
  }
  फर्काऊ TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  यदि (!TokenQueue.empty())
    TokenQueue.झिक_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  यदि (TokenQueue.empty())
    TokenQueue.resetAlloc();

  फर्काऊ Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  यदि (Position == End)
    फर्काऊ Position;
  // Check 7 bit c-printable - b-char.
  यदि (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    फर्काऊ Position + 1;

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
      फर्काऊ Position + u8d.second;
  }
  फर्काऊ Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  यदि (Position == End)
    फर्काऊ Position;
  यदि (*Position == 0x0D) {
    यदि (Position + 1 != End && *(Position + 1) == 0x0A)
      फर्काऊ Position + 2;
    फर्काऊ Position + 1;
  }

  यदि (*Position == 0x0A)
    फर्काऊ Position + 1;
  फर्काऊ Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  यदि (Position == End)
    फर्काऊ Position;
  यदि (*Position == ' ')
    फर्काऊ Position + 1;
  फर्काऊ Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  यदि (Position == End)
    फर्काऊ Position;
  यदि (*Position == ' ' || *Position == '\t')
    फर्काऊ Position + 1;
  फर्काऊ Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  यदि (Position == End)
    फर्काऊ Position;
  यदि (*Position == ' ' || *Position == '\t')
    फर्काऊ Position;
  फर्काऊ skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  जबसम्म (सत्य) {
    StringRef::iterator i = (एतत्->*Func)(Position);
    यदि (i == Position)
      रोक;
    Position = i;
  }
  फर्काऊ Position;
}

शून्य Scanner::advanceWhile(SkipWhileFunc Func) {
  स्वतः Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

स्थिर तार्किक is_ns_hex_digit(अचर वर्ण C) { फर्काऊ isAlnum(C); }

स्थिर तार्किक is_ns_word_char(अचर वर्ण C) { फर्काऊ C == '-' || isAlpha(C); }

शून्य Scanner::scan_ns_uri_char() {
  जबसम्म (सत्य) {
    यदि (Current == End)
      रोक;
    यदि ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } अन्यथा {
      रोक;
    }
  }
}

तार्किक Scanner::consume(uint32_t Expected) {
  यदि (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    फर्काऊ असत्य;
  }
  यदि (Current == End)
    फर्काऊ असत्य;
  यदि (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    फर्काऊ असत्य;
  }
  यदि (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    फर्काऊ सत्य;
  }
  फर्काऊ असत्य;
}

शून्य Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  प्रतिज्ञा(Current <= End && "Skipped past the end");
}

तार्किक Scanner::isBlankOrBreak(StringRef::iterator Position) {
  यदि (Position == End)
    फर्काऊ असत्य;
  फर्काऊ *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

तार्किक Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  यदि (Position == End || isBlankOrBreak(Position))
    फर्काऊ असत्य;
  यदि (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    फर्काऊ असत्य;
  फर्काऊ सत्य;
}

तार्किक Scanner::is_पङ्क्ति_Empty(StringRef पङ्क्ति) {
  लागि (अचर स्वतः *Position = पङ्क्ति.begin(); Position != पङ्क्ति.end(); ++Position)
    यदि (!isBlankOrBreak(Position))
      फर्काऊ असत्य;
  फर्काऊ सत्य;
}

तार्किक Scanner::consume_पङ्क्ति_Break_If_Present() {
  स्वतः Next = skip_b_break(Current);
  यदि (Next == Current)
    फर्काऊ असत्य;
  Column = 0;
  ++पङ्क्ति;
  Current = Next;
  फर्काऊ सत्य;
}

शून्य Scanner::save_Simple_साँचो_Candidate( TokenQueueT::iterator Tok
                                    , अचिन्हित AtColumn
                                    , तार्किक IsRequired) {
  यदि (Is_Simple_साँचो_Allowed) {
    Simple_साँचो SK;
    SK.Tok = Tok;
    SK.पङ्क्ति = पङ्क्ति;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_साँचोहरू.थप_back(SK);
  }
}

शून्य Scanner::remove_Stale_Simple_साँचो_Candidates() {
  लागि (SmallVectorImpl<Simple_साँचो>::iterator i = Simple_साँचोहरू.begin();
                                            i != Simple_साँचोहरू.end();) {
    यदि (i->पङ्क्ति != पङ्क्ति || i->Column + 1024 < Column) {
      यदि (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_साँचोहरू.erase(i);
    } अन्यथा {
      ++i;
    }
  }
}

शून्य Scanner::remove_Simple_साँचो_Candidates_On_Flow_Level(अचिन्हित Level) {
  यदि (!Simple_साँचोहरू.empty() && (Simple_साँचोहरू.end() - 1)->FlowLevel == Level)
    Simple_साँचोहरू.झिक_back();
}

तार्किक Scanner::unrollIndent(पूर्णाङ्क ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  यदि (FlowLevel != 0)
    फर्काऊ सत्य;

  जबसम्म (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.थप_back(T);
    Indent = Indents.झिक_back_val();
  }

  फर्काऊ सत्य;
}

तार्किक Scanner::rollIndent( पूर्णाङ्क ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator घुसाउ_Point) {
  यदि (FlowLevel)
    फर्काऊ सत्य;
  यदि (Indent < ToColumn) {
    Indents.थप_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.घुसाउ(घुसाउ_Point, T);
  }
  फर्काऊ सत्य;
}

शून्य Scanner::skipComment() {
  यदि (Current == End || *Current != '#')
    फर्काऊ;
  जबसम्म (सत्य) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    यदि (I == Current)
      रोक;
    Current = I;
    ++Column;
  }
}

शून्य Scanner::scanToNextToken() {
  जबसम्म (सत्य) {
    जबसम्म (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    यदि (i == Current)
      रोक;
    Current = i;
    ++पङ्क्ति;
    Column = 0;
    // New lines may start a simple key.
    यदि (!FlowLevel)
      Is_Simple_साँचो_Allowed = सत्य;
  }
}

तार्किक Scanner::scanStreamStart() {
  IsStartOfStream = असत्य;

  EncodingInfo EI = getUnicodeEncoding(current_आगत());

  Token T;
  T.Kind = Token::TK_StreamStart;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.थप_back(T);
  Current += EI.second;
  फर्काऊ सत्य;
}

तार्किक Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  यदि (Column != 0) {
    Column = 0;
    ++पङ्क्ति;
  }

  unrollIndent(-1);
  Simple_साँचोहरू.clear();
  Is_Simple_साँचो_Allowed = असत्य;
  Is_Adjacent_मान_Allowed_In_Flow = असत्य;

  Token T;
  T.Kind = Token::TK_StreamEnd;
  T.Range = StringRef(Current, 0);
  TokenQueue.थप_back(T);
  फर्काऊ सत्य;
}

तार्किक Scanner::scanDirective() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_साँचोहरू.clear();
  Is_Simple_साँचो_Allowed = असत्य;
  Is_Adjacent_मान_Allowed_In_Flow = असत्य;

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
    TokenQueue.थप_back(T);
    फर्काऊ सत्य;
  } अन्यथा यदि(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.थप_back(T);
    फर्काऊ सत्य;
  }
  फर्काऊ असत्य;
}

तार्किक Scanner::scanDocumentIndicator(तार्किक IsStart) {
  unrollIndent(-1);
  Simple_साँचोहरू.clear();
  Is_Simple_साँचो_Allowed = असत्य;
  Is_Adjacent_मान_Allowed_In_Flow = असत्य;

  Token T;
  T.Kind = IsStart ? Token::TK_DocumentStart : Token::TK_DocumentEnd;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.थप_back(T);
  फर्काऊ सत्य;
}

तार्किक Scanner::scanFlowCollectionStart(तार्किक IsSequence) {
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.थप_back(T);

  // [ and { may begin a simple key.
  save_Simple_साँचो_Candidate(--TokenQueue.end(), Column - 1, असत्य);

  // And may also be followed by a simple key.
  Is_Simple_साँचो_Allowed = सत्य;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_मान_Allowed_In_Flow = असत्य;
  ++FlowLevel;
  फर्काऊ सत्य;
}

तार्किक Scanner::scanFlowCollectionEnd(तार्किक IsSequence) {
  remove_Simple_साँचो_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_साँचो_Allowed = असत्य;
  Is_Adjacent_मान_Allowed_In_Flow = सत्य;
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceEnd
                      : Token::TK_FlowMappingEnd;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.थप_back(T);
  यदि (FlowLevel)
    --FlowLevel;
  फर्काऊ सत्य;
}

तार्किक Scanner::scanFlowEntry() {
  remove_Simple_साँचो_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_साँचो_Allowed = सत्य;
  Is_Adjacent_मान_Allowed_In_Flow = असत्य;
  Token T;
  T.Kind = Token::TK_FlowEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.थप_back(T);
  फर्काऊ सत्य;
}

तार्किक Scanner::scanBlockEntry() {
  rollIndent(Column, Token::TK_BlockSequenceStart, TokenQueue.end());
  remove_Simple_साँचो_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_साँचो_Allowed = सत्य;
  Is_Adjacent_मान_Allowed_In_Flow = असत्य;
  Token T;
  T.Kind = Token::TK_BlockEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.थप_back(T);
  फर्काऊ सत्य;
}

तार्किक Scanner::scan_साँचो() {
  यदि (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  remove_Simple_साँचो_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_साँचो_Allowed = !FlowLevel;
  Is_Adjacent_मान_Allowed_In_Flow = असत्य;

  Token T;
  T.Kind = Token::TK_साँचो;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.थप_back(T);
  फर्काऊ सत्य;
}

तार्किक Scanner::scan_मान() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  यदि (!Simple_साँचोहरू.empty()) {
    Simple_साँचो SK = Simple_साँचोहरू.झिक_back_val();
    Token T;
    T.Kind = Token::TK_साँचो;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    लागि (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      यदि (i == SK.Tok)
        रोक;
    }
    यदि (i == e) {
      Failed = सत्य;
      फर्काऊ असत्य;
    }
    i = TokenQueue.घुसाउ(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    Is_Simple_साँचो_Allowed = असत्य;
  } अन्यथा {
    यदि (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    Is_Simple_साँचो_Allowed = !FlowLevel;
  }
  Is_Adjacent_मान_Allowed_In_Flow = असत्य;

  Token T;
  T.Kind = Token::TK_मान;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.थप_back(T);
  फर्काऊ सत्य;
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
  जबसम्म (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  फर्काऊ (Position - 1 - I) % 2 == 1;
}

तार्किक Scanner::scanFlowScalar(तार्किक IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  अचिन्हित ColStart = Column;
  यदि (IsDoubleQuoted) {
    गर {
      ++Current;
      जबसम्म (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } जबसम्म (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } अन्यथा {
    skip(1);
    जबसम्म (Current != End) {
      // Skip a ' followed by another '.
      यदि (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        जारी;
      } अन्यथा यदि (*Current == '\'')
        रोक;
      StringRef::iterator i = skip_nb_char(Current);
      यदि (i == Current) {
        i = skip_b_break(Current);
        यदि (i == Current)
          रोक;
        Current = i;
        Column = 0;
        ++पङ्क्ति;
      } अन्यथा {
        यदि (i == End)
          रोक;
        Current = i;
        ++Column;
      }
    }
  }

  यदि (Current == End) {
    setError("Expected quote at end of scalar", Current);
    फर्काऊ असत्य;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.थप_back(T);

  save_Simple_साँचो_Candidate(--TokenQueue.end(), ColStart, असत्य);

  Is_Simple_साँचो_Allowed = असत्य;
  Is_Adjacent_मान_Allowed_In_Flow = सत्य;

  फर्काऊ सत्य;
}

तार्किक Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  अचिन्हित ColStart = Column;
  अचिन्हित LeadingBlanks = 0;
  प्रतिज्ञा(Indent >= -1 && "Indent must be >= -1 !");
  अचिन्हित indent = static_cast<अचिन्हित>(Indent + 1);
  जबसम्म (Current != End) {
    यदि (*Current == '#')
      रोक;

    जबसम्म (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      यदि (i == Current)
        रोक;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    यदि (!isBlankOrBreak(Current))
      रोक;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    जबसम्म (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      यदि (i != Tmp) {
        यदि (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          फर्काऊ असत्य;
        }
        Tmp = i;
        ++Column;
      } अन्यथा {
        i = skip_b_break(Tmp);
        यदि (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++पङ्क्ति;
      }
    }

    यदि (!FlowLevel && Column < indent)
      रोक;

    Current = Tmp;
  }
  यदि (Start == Current) {
    setError("Got empty plain scalar", Start);
    फर्काऊ असत्य;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.थप_back(T);

  // Plain scalars can be simple keys.
  save_Simple_साँचो_Candidate(--TokenQueue.end(), ColStart, असत्य);

  Is_Simple_साँचो_Allowed = असत्य;
  Is_Adjacent_मान_Allowed_In_Flow = असत्य;

  फर्काऊ सत्य;
}

तार्किक Scanner::scanAliasOrAnchor(तार्किक IsAlias) {
  StringRef::iterator Start = Current;
  अचिन्हित ColStart = Column;
  skip(1);
  जबसम्म (Current != End) {
    यदि (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      रोक;
    StringRef::iterator i = skip_ns_char(Current);
    यदि (i == Current)
      रोक;
    Current = i;
    ++Column;
  }

  यदि (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    फर्काऊ असत्य;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.थप_back(T);

  // Alias and anchors can be simple keys.
  save_Simple_साँचो_Candidate(--TokenQueue.end(), ColStart, असत्य);

  Is_Simple_साँचो_Allowed = असत्य;
  Is_Adjacent_मान_Allowed_In_Flow = असत्य;

  फर्काऊ सत्य;
}

तार्किक Scanner::scanBlockScalarIndicators(वर्ण &StyleIndicator,
                                        वर्ण &ChompingIndicator,
                                        अचिन्हित &IndentIndicator,
                                        तार्किक &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  यदि (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    फर्काऊ असत्य;
  फर्काऊ सत्य;
}

वर्ण Scanner::scanBlockStyleIndicator() {
  वर्ण Indicator = ' ';
  यदि (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  फर्काऊ Indicator;
}

वर्ण Scanner::scanBlockChompingIndicator() {
  वर्ण Indicator = ' ';
  यदि (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  फर्काऊ Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
स्थिर अचिन्हित get_Chomped_पङ्क्ति_Breaks(वर्ण ChompingIndicator,
                                     अचिन्हित पङ्क्ति_Breaks, StringRef Str) {
  यदि (ChompingIndicator == '-') // Strip all line breaks.
    फर्काऊ 0;
  यदि (ChompingIndicator == '+') // Keep all line breaks.
    फर्काऊ पङ्क्ति_Breaks;
  // Clip trailing lines.
  फर्काऊ Str.empty() ? 0 : 1;
}

अचिन्हित Scanner::scanBlockIndentationIndicator() {
  अचिन्हित Indent = 0;
  यदि (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = अचिन्हित(*Current - '0');
    skip(1);
  }
  फर्काऊ Indent;
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
    TokenQueue.थप_back(T);
    IsDone = सत्य;
    फर्काऊ सत्य;
  }

  यदि (!consume_पङ्क्ति_Break_If_Present()) {
    setError("Expected a line break after block scalar header", Current);
    फर्काऊ असत्य;
  }
  फर्काऊ सत्य;
}

तार्किक Scanner::findBlockScalarIndent(अचिन्हित &BlockIndent,
                                    अचिन्हित BlockExitIndent,
                                    अचिन्हित &पङ्क्ति_Breaks, तार्किक &IsDone) {
  अचिन्हित Max_All_Space_पङ्क्ति_Characters = 0;
  StringRef::iterator Longest_All_Space_पङ्क्ति;

  जबसम्म (सत्य) {
    advanceWhile(&Scanner::skip_s_space);
    यदि (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      यदि (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = सत्य;
        फर्काऊ सत्य;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      यदि (Max_All_Space_पङ्क्ति_Characters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_पङ्क्ति);
        फर्काऊ असत्य;
      }
      फर्काऊ सत्य;
    }
    यदि (skip_b_break(Current) != Current &&
        Column > Max_All_Space_पङ्क्ति_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_पङ्क्ति_Characters = Column;
      Longest_All_Space_पङ्क्ति = Current;
    }

    // Check for EOF.
    यदि (Current == End) {
      IsDone = सत्य;
      फर्काऊ सत्य;
    }

    यदि (!consume_पङ्क्ति_Break_If_Present()) {
      IsDone = सत्य;
      फर्काऊ सत्य;
    }
    ++पङ्क्ति_Breaks;
  }
  फर्काऊ सत्य;
}

तार्किक Scanner::scanBlockScalarIndent(अचिन्हित BlockIndent,
                                    अचिन्हित BlockExitIndent, तार्किक &IsDone) {
  // Skip the indentation.
  जबसम्म (Column < BlockIndent) {
    स्वतः I = skip_s_space(Current);
    यदि (I == Current)
      रोक;
    Current = I;
    ++Column;
  }

  यदि (skip_nb_char(Current) == Current)
    फर्काऊ सत्य;

  यदि (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = सत्य;
    फर्काऊ सत्य;
  }

  यदि (Column < BlockIndent) {
    यदि (Current != End && *Current == '#') { // Trailing comment.
      IsDone = सत्य;
      फर्काऊ सत्य;
    }
    setError("A text line is less indented than the block scalar", Current);
    फर्काऊ असत्य;
  }
  फर्काऊ सत्य; // A normal text line.
}

तार्किक Scanner::scanBlockScalar(तार्किक IsLiteral) {
  प्रतिज्ञा(*Current == '|' || *Current == '>');
  वर्ण StyleIndicator;
  वर्ण ChompingIndicator;
  अचिन्हित BlockIndent;
  तार्किक IsDone = असत्य;
  यदि (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    फर्काऊ असत्य;
  यदि (IsDone)
    फर्काऊ सत्य;
  तार्किक IsFolded = StyleIndicator == '>';

  अचर स्वतः *Start = Current;
  अचिन्हित BlockExitIndent = Indent < 0 ? 0 : (अचिन्हित)Indent;
  अचिन्हित पङ्क्ति_Breaks = 0;
  यदि (BlockIndent == 0) {
    यदि (!findBlockScalarIndent(BlockIndent, BlockExitIndent, पङ्क्ति_Breaks,
                               IsDone))
      फर्काऊ असत्य;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  जबसम्म (!IsDone) {
    यदि (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      फर्काऊ असत्य;
    यदि (IsDone)
      रोक;

    // Parse the current line.
    स्वतः पङ्क्ति_Start = Current;
    advanceWhile(&Scanner::skip_nb_char);
    यदि (पङ्क्ति_Start != Current) {
      यदि (पङ्क्ति_Breaks && IsFolded && !Scanner::is_पङ्क्ति_Empty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        यदि (पङ्क्ति_Breaks == 1) {
          Str.append(पङ्क्ति_Breaks,
                     is_पङ्क्ति_Empty(StringRef(पङ्क्ति_Start, Current - पङ्क्ति_Start))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        पङ्क्ति_Breaks--;
      }
      Str.append(पङ्क्ति_Breaks, '\n');
      Str.append(StringRef(पङ्क्ति_Start, Current - पङ्क्ति_Start));
      पङ्क्ति_Breaks = 0;
    }

    // Check for EOF.
    यदि (Current == End)
      रोक;

    यदि (!consume_पङ्क्ति_Break_If_Present())
      रोक;
    ++पङ्क्ति_Breaks;
  }

  यदि (Current == End && !पङ्क्ति_Breaks)
    // Ensure that there is at least one line break before the end of file.
    पङ्क्ति_Breaks = 1;
  Str.append(get_Chomped_पङ्क्ति_Breaks(ChompingIndicator, पङ्क्ति_Breaks, Str), '\n');

  // New lines may start a simple key.
  यदि (!FlowLevel)
    Is_Simple_साँचो_Allowed = सत्य;
  Is_Adjacent_मान_Allowed_In_Flow = असत्य;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.मान = std::string(Str);
  TokenQueue.थप_back(T);
  फर्काऊ सत्य;
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
      फर्काऊ असत्य;
  } अन्यथा {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.Kind = Token::TK_Tag;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.थप_back(T);

  // Tags can be simple keys.
  save_Simple_साँचो_Candidate(--TokenQueue.end(), ColStart, असत्य);

  Is_Simple_साँचो_Allowed = असत्य;
  Is_Adjacent_मान_Allowed_In_Flow = असत्य;

  फर्काऊ सत्य;
}

तार्किक Scanner::fetchMoreTokens() {
  यदि (IsStartOfStream)
    फर्काऊ scanStreamStart();

  scanToNextToken();

  यदि (Current == End)
    फर्काऊ scanStreamEnd();

  remove_Stale_Simple_साँचो_Candidates();

  unrollIndent(Column);

  यदि (Column == 0 && *Current == '%')
    फर्काऊ scanDirective();

  यदि (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    फर्काऊ scanDocumentIndicator(सत्य);

  यदि (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    फर्काऊ scanDocumentIndicator(असत्य);

  यदि (*Current == '[')
    फर्काऊ scanFlowCollectionStart(सत्य);

  यदि (*Current == '{')
    फर्काऊ scanFlowCollectionStart(असत्य);

  यदि (*Current == ']')
    फर्काऊ scanFlowCollectionEnd(सत्य);

  यदि (*Current == '}')
    फर्काऊ scanFlowCollectionEnd(असत्य);

  यदि (*Current == ',')
    फर्काऊ scanFlowEntry();

  यदि (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    फर्काऊ scanBlockEntry();

  यदि (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    फर्काऊ scan_साँचो();

  यदि (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || Is_Adjacent_मान_Allowed_In_Flow))
    फर्काऊ scan_मान();

  यदि (*Current == '*')
    फर्काऊ scanAliasOrAnchor(सत्य);

  यदि (*Current == '&')
    फर्काऊ scanAliasOrAnchor(असत्य);

  यदि (*Current == '!')
    फर्काऊ scanTag();

  यदि (*Current == '|' && !FlowLevel)
    फर्काऊ scanBlockScalar(सत्य);

  यदि (*Current == '>' && !FlowLevel)
    फर्काऊ scanBlockScalar(असत्य);

  यदि (*Current == '\'')
    फर्काऊ scanFlowScalar(असत्य);

  यदि (*Current == '"')
    फर्काऊ scanFlowScalar(सत्य);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  यदि ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    फर्काऊ scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  फर्काऊ असत्य;
}

Stream::Stream(StringRef आगत, SourceMgr &SM, तार्किक ShowColors,
               std::error_code *EC)
    : scanner(नूतन Scanner(आगत, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef आगत_Buffer, SourceMgr &SM, तार्किक ShowColors,
               std::error_code *EC)
    : scanner(नूतन Scanner(आगत_Buffer, SM, ShowColors, EC)) {}

Stream::~Stream() = स्वभाव;

तार्किक Stream::failed() { फर्काऊ scanner->failed(); }

शून्य Stream::printError(गाँठो *N, अचर Twine &सन्देश_2, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), सन्देश_2, Kind);
}

शून्य Stream::printError(अचर SMRange &Range, अचर Twine &सन्देश_2,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, सन्देश_2, Range);
}

document_iterator Stream::begin() {
  यदि (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(नूतन Document(*एतत्));
  फर्काऊ document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  फर्काऊ document_iterator();
}

शून्य Stream::skip() {
  लागि (Document &Doc : *एतत्)
    Doc.skip();
}

गाँठो::गाँठो(अचिन्हित पूर्णाङ्क Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string गाँठो::getVerbatimTag() अचर {
  StringRef Raw = getRawTag();
  यदि (!Raw.empty() && Raw != "!") {
    std::string Ret;
    यदि (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      फर्काऊ Ret;
    } अन्यथा यदि (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      फर्काऊ Ret;
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
      फर्काऊ Ret;
    }
  }

  विकल्प (getType()) {
  पक्ष NK_Null:
    फर्काऊ "tag:yaml.org,2002:null";
  पक्ष NK_Scalar:
  पक्ष NK_BlockScalar:
    // TODO: Tag resolution.
    फर्काऊ "tag:yaml.org,2002:str";
  पक्ष NK_Mapping:
    फर्काऊ "tag:yaml.org,2002:map";
  पक्ष NK_Sequence:
    फर्काऊ "tag:yaml.org,2002:seq";
  }

  फर्काऊ "";
}

Token &गाँठो::peekNext() {
  फर्काऊ Doc->peekNext();
}

Token गाँठो::getNext() {
  फर्काऊ Doc->getNext();
}

गाँठो *गाँठो::parse_Block_गाँठो() {
  फर्काऊ Doc->parse_Block_गाँठो();
}

BumpPtrAllocator &गाँठो::getAllocator() {
  फर्काऊ Doc->गाँठो_Allocator;
}

शून्य गाँठो::setError(अचर Twine &सन्देश_2, Token &Tok) अचर {
  Doc->setError(सन्देश_2, Tok);
}

तार्किक गाँठो::failed() अचर {
  फर्काऊ Doc->failed();
}

StringRef Scalar_गाँठो::get_मान(SmallVectorImpl<वर्ण> &Storage) अचर {
  यदि (मान[0] == '"')
    फर्काऊ get_Double_Quoted_मान(मान, Storage);
  यदि (मान[0] == '\'')
    फर्काऊ get_Single_Quoted_मान(मान, Storage);
  फर्काऊ get_Plain_मान(मान, Storage);
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
parse_Scalar_मान(StringRef Unquoted_मान, SmallVectorImpl<वर्ण> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<वर्ण> &)>
                     UnescapeCallback) {
  माप_प्रकार I = Unquoted_मान.find_first_of(LookupChars);
  यदि (I == StringRef::npos)
    फर्काऊ Unquoted_मान;

  Storage.clear();
  Storage.reserve(Unquoted_मान.size());
  वर्ण Last_New_पङ्क्ति_Added_As = '\0';
  लागि (; I != StringRef::npos; I = Unquoted_मान.find_first_of(LookupChars)) {
    यदि (Unquoted_मान[I] != '\r' && Unquoted_मान[I] != '\n') {
      llvm::append_range(Storage, Unquoted_मान.take_front(I));
      Unquoted_मान = UnescapeCallback(Unquoted_मान.drop_front(I), Storage);
      Last_New_पङ्क्ति_Added_As = '\0';
      जारी;
    }
    यदि (माप_प्रकार LastNonSWhite = Unquoted_मान.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_मान.take_front(LastNonSWhite + 1));
      Storage.थप_back(' ');
      Last_New_पङ्क्ति_Added_As = ' ';
    } अन्यथा {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      विकल्प (Last_New_पङ्क्ति_Added_As) {
      पक्ष ' ':
        प्रतिज्ञा(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        Last_New_पङ्क्ति_Added_As = '\n';
        रोक;
      पक्ष '\n':
        प्रतिज्ञा(!Storage.empty() && Storage.back() == '\n');
        Storage.थप_back('\n');
        रोक;
      स्वभाव:
        Storage.थप_back(' ');
        Last_New_पङ्क्ति_Added_As = ' ';
        रोक;
      }
    }
    // Handle Windows-style EOL
    यदि (Unquoted_मान.substr(I, 2) == "\r\n")
      I++;
    Unquoted_मान = Unquoted_मान.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_मान);
  फर्काऊ StringRef(Storage.begin(), Storage.size());
}

StringRef
Scalar_गाँठो::get_Double_Quoted_मान(StringRef Raw_मान,
                                 SmallVectorImpl<वर्ण> &Storage) अचर {
  प्रतिज्ञा(Raw_मान.size() >= 2 && Raw_मान.front() == '"' &&
         Raw_मान.back() == '"');
  StringRef Unquoted_मान = Raw_मान.substr(1, Raw_मान.size() - 2);

  स्वतः UnescapeFunc = [एतत्](StringRef Unquoted_मान,
                             SmallVectorImpl<वर्ण> &Storage) {
    प्रतिज्ञा(Unquoted_मान.take_front(1) == "\\");
    यदि (Unquoted_मान.size() == 1) {
      Token T;
      T.Range = Unquoted_मान;
      setError("Unrecognized escape code", T);
      Storage.clear();
      फर्काऊ StringRef();
    }
    Unquoted_मान = Unquoted_मान.drop_front(1);
    विकल्प (Unquoted_मान[0]) {
    स्वभाव: {
      Token T;
      T.Range = Unquoted_मान.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      फर्काऊ StringRef();
    }
    पक्ष '\r':
      // Shrink the Windows-style EOL.
      यदि (Unquoted_मान.size() >= 2 && Unquoted_मान[1] == '\n')
        Unquoted_मान = Unquoted_मान.drop_front(1);
      [[fallthrough]];
    पक्ष '\n':
      फर्काऊ Unquoted_मान.drop_front(1).ltrim(" \t");
    पक्ष '0':
      Storage.थप_back(0x00);
      रोक;
    पक्ष 'a':
      Storage.थप_back(0x07);
      रोक;
    पक्ष 'b':
      Storage.थप_back(0x08);
      रोक;
    पक्ष 't':
    पक्ष 0x09:
      Storage.थप_back(0x09);
      रोक;
    पक्ष 'n':
      Storage.थप_back(0x0A);
      रोक;
    पक्ष 'v':
      Storage.थप_back(0x0B);
      रोक;
    पक्ष 'f':
      Storage.थप_back(0x0C);
      रोक;
    पक्ष 'r':
      Storage.थप_back(0x0D);
      रोक;
    पक्ष 'e':
      Storage.थप_back(0x1B);
      रोक;
    पक्ष ' ':
      Storage.थप_back(0x20);
      रोक;
    पक्ष '"':
      Storage.थप_back(0x22);
      रोक;
    पक्ष '/':
      Storage.थप_back(0x2F);
      रोक;
    पक्ष '\\':
      Storage.थप_back(0x5C);
      रोक;
    पक्ष 'N':
      encodeUTF8(0x85, Storage);
      रोक;
    पक्ष '_':
      encodeUTF8(0xA0, Storage);
      रोक;
    पक्ष 'L':
      encodeUTF8(0x2028, Storage);
      रोक;
    पक्ष 'P':
      encodeUTF8(0x2029, Storage);
      रोक;
    पक्ष 'x': {
      यदि (Unquoted_मान.size() < 3)
        // TODO: Report error.
        रोक;
      अचिन्हित पूर्णाङ्क Unicode_Scalar_मान;
      यदि (Unquoted_मान.substr(1, 2).getAsInteger(16, Unicode_Scalar_मान))
        // TODO: Report error.
        Unicode_Scalar_मान = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मान, Storage);
      फर्काऊ Unquoted_मान.drop_front(3);
    }
    पक्ष 'u': {
      यदि (Unquoted_मान.size() < 5)
        // TODO: Report error.
        रोक;
      अचिन्हित पूर्णाङ्क Unicode_Scalar_मान;
      यदि (Unquoted_मान.substr(1, 4).getAsInteger(16, Unicode_Scalar_मान))
        // TODO: Report error.
        Unicode_Scalar_मान = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मान, Storage);
      फर्काऊ Unquoted_मान.drop_front(5);
    }
    पक्ष 'U': {
      यदि (Unquoted_मान.size() < 9)
        // TODO: Report error.
        रोक;
      अचिन्हित पूर्णाङ्क Unicode_Scalar_मान;
      यदि (Unquoted_मान.substr(1, 8).getAsInteger(16, Unicode_Scalar_मान))
        // TODO: Report error.
        Unicode_Scalar_मान = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मान, Storage);
      फर्काऊ Unquoted_मान.drop_front(9);
    }
    }
    फर्काऊ Unquoted_मान.drop_front(1);
  };

  फर्काऊ parse_Scalar_मान(Unquoted_मान, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_गाँठो::get_Single_Quoted_मान(StringRef Raw_मान,
                                           SmallVectorImpl<वर्ण> &Storage) {
  प्रतिज्ञा(Raw_मान.size() >= 2 && Raw_मान.front() == '\'' &&
         Raw_मान.back() == '\'');
  StringRef Unquoted_मान = Raw_मान.substr(1, Raw_मान.size() - 2);

  स्वतः UnescapeFunc = [](StringRef Unquoted_मान,
                         SmallVectorImpl<वर्ण> &Storage) {
    प्रतिज्ञा(Unquoted_मान.take_front(2) == "''");
    Storage.थप_back('\'');
    फर्काऊ Unquoted_मान.drop_front(2);
  };

  फर्काऊ parse_Scalar_मान(Unquoted_मान, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_गाँठो::get_Plain_मान(StringRef Raw_मान,
                                    SmallVectorImpl<वर्ण> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_मान = Raw_मान.rtrim("\r\n \t");
  फर्काऊ parse_Scalar_मान(Raw_मान, Storage, "\r\n", nullptr);
}

गाँठो *साँचो_मान_गाँठो::get_साँचो() {
  यदि (साँचो)
    फर्काऊ साँचो;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    यदि (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_मान
        || t.Kind == Token::TK_Error) {
      फर्काऊ साँचो = नूतन (getAllocator()) Null_गाँठो(Doc);
    }
    यदि (t.Kind == Token::TK_साँचो)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  यदि (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_मान) {
    फर्काऊ साँचो = नूतन (getAllocator()) Null_गाँठो(Doc);
  }

  // We've got a normal key.
  फर्काऊ साँचो = parse_Block_गाँठो();
}

गाँठो *साँचो_मान_गाँठो::get_मान() {
  यदि (मान)
    फर्काऊ मान;

  यदि (गाँठो* साँचो = get_साँचो())
    साँचो->skip();
  अन्यथा {
    setError("Null key in Key Value.", peekNext());
    फर्काऊ मान = नूतन (getAllocator()) Null_गाँठो(Doc);
  }

  यदि (failed())
    फर्काऊ मान = नूतन (getAllocator()) Null_गाँठो(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    यदि (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_साँचो
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      फर्काऊ मान = नूतन (getAllocator()) Null_गाँठो(Doc);
    }

    यदि (t.Kind != Token::TK_मान) {
      setError("Unexpected token in Key Value.", t);
      फर्काऊ मान = नूतन (getAllocator()) Null_गाँठो(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  यदि (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_साँचो) {
    फर्काऊ मान = नूतन (getAllocator()) Null_गाँठो(Doc);
  }

  // We got a normal value.
  फर्काऊ मान = parse_Block_गाँठो();
}

शून्य Mapping_गाँठो::increment() {
  यदि (failed()) {
    IsAtEnd = सत्य;
    CurrentEntry = nullptr;
    फर्काऊ;
  }
  यदि (CurrentEntry) {
    CurrentEntry->skip();
    यदि (Type == MT_Inline) {
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      फर्काऊ;
    }
  }
  Token T = peekNext();
  यदि (T.Kind == Token::TK_साँचो || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = नूतन (getAllocator()) साँचो_मान_गाँठो(Doc);
  } अन्यथा यदि (Type == MT_Block) {
    विकल्प (T.Kind) {
    पक्ष Token::TK_BlockEnd:
      getNext();
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      रोक;
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
      फर्काऊ increment();
    पक्ष Token::TK_FlowMappingEnd:
      getNext();
      [[fallthrough]];
    पक्ष Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      रोक;
    स्वभाव:
      setError( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
    }
  }
}

शून्य Sequence_गाँठो::increment() {
  यदि (failed()) {
    IsAtEnd = सत्य;
    CurrentEntry = nullptr;
    फर्काऊ;
  }
  यदि (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  यदि (SeqType == ST_Block) {
    विकल्प (T.Kind) {
    पक्ष Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_गाँठो();
      यदि (!CurrentEntry) { // An error occurred.
        IsAtEnd = सत्य;
        CurrentEntry = nullptr;
      }
      रोक;
    पक्ष Token::TK_BlockEnd:
      getNext();
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      रोक;
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
      CurrentEntry = parse_Block_गाँठो();
      यदि (!CurrentEntry) { // An error occurred.
        IsAtEnd = सत्य;
        CurrentEntry = nullptr;
      }
      रोक;
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
      फर्काऊ increment();
    पक्ष Token::TK_FlowSequenceEnd:
      getNext();
      [[fallthrough]];
    पक्ष Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      रोक;
    पक्ष Token::TK_StreamEnd:
    पक्ष Token::TK_DocumentEnd:
    पक्ष Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      रोक;
    स्वभाव:
      यदि (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = सत्य;
        CurrentEntry = nullptr;
        रोक;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parse_Block_गाँठो();
      यदि (!CurrentEntry) {
        IsAtEnd = सत्य;
      }
      WasPreviousTokenFlowEntry = असत्य;
      रोक;
    }
  }
}

Document::Document(Stream &S) : stream(S), जरा_2(nullptr) {
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
    फर्काऊ असत्य;
  यदि (!जरा_2 && !get_जरा())
    फर्काऊ असत्य;
  जरा_2->skip();
  Token &T = peekNext();
  यदि (T.Kind == Token::TK_StreamEnd)
    फर्काऊ असत्य;
  यदि (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    फर्काऊ skip();
  }
  फर्काऊ सत्य;
}

Token &Document::peekNext() {
  फर्काऊ stream.scanner->peekNext();
}

Token Document::getNext() {
  फर्काऊ stream.scanner->getNext();
}

शून्य Document::setError(अचर Twine &Message, Token &Location) अचर {
  stream.scanner->setError(Message, Location.Range.begin());
}

तार्किक Document::failed() अचर {
  फर्काऊ stream.scanner->failed();
}

गाँठो *Document::parse_Block_गाँठो() {
  Token T = peekNext();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  विकल्प (T.Kind) {
  पक्ष Token::TK_Alias:
    getNext();
    फर्काऊ नूतन (गाँठो_Allocator) Alias_गाँठो(stream.CurrentDoc, T.Range.substr(1));
  पक्ष Token::TK_Anchor:
    यदि (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      फर्काऊ nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    गच्छ parse_property;
  पक्ष Token::TK_Tag:
    यदि (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      फर्काऊ nullptr;
    }
    TagInfo = getNext(); // Consume TK_Tag.
    T = peekNext();
    गच्छ parse_property;
  स्वभाव:
    रोक;
  }

  विकल्प (T.Kind) {
  पक्ष Token::TK_BlockEntry:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    फर्काऊ नूतन (गाँठो_Allocator) Sequence_गाँठो( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , Sequence_गाँठो::ST_Indentless);
  पक्ष Token::TK_BlockSequenceStart:
    getNext();
    फर्काऊ नूतन (गाँठो_Allocator)
      Sequence_गाँठो( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_गाँठो::ST_Block);
  पक्ष Token::TK_BlockMappingStart:
    getNext();
    फर्काऊ नूतन (गाँठो_Allocator)
      Mapping_गाँठो( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_गाँठो::MT_Block);
  पक्ष Token::TK_FlowSequenceStart:
    getNext();
    फर्काऊ नूतन (गाँठो_Allocator)
      Sequence_गाँठो( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_गाँठो::ST_Flow);
  पक्ष Token::TK_FlowMappingStart:
    getNext();
    फर्काऊ नूतन (गाँठो_Allocator)
      Mapping_गाँठो( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_गाँठो::MT_Flow);
  पक्ष Token::TK_Scalar:
    getNext();
    फर्काऊ नूतन (गाँठो_Allocator)
      Scalar_गाँठो( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  पक्ष Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.मान.c_str(), T.मान.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(गाँठो_Allocator).drop_back();
    फर्काऊ नूतन (गाँठो_Allocator)
        Block_Scalar_गाँठो(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  पक्ष Token::TK_साँचो:
    // Don't eat the TK_Key, KeyValueNode expects it.
    फर्काऊ नूतन (गाँठो_Allocator)
      Mapping_गाँठो( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_गाँठो::MT_Inline);
  पक्ष Token::TK_DocumentStart:
  पक्ष Token::TK_DocumentEnd:
  पक्ष Token::TK_StreamEnd:
  स्वभाव:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    फर्काऊ नूतन (गाँठो_Allocator) Null_गाँठो(stream.CurrentDoc);
  पक्ष Token::TK_FlowMappingEnd:
  पक्ष Token::TK_FlowSequenceEnd:
  पक्ष Token::TK_FlowEntry: {
    यदि (जरा_2 && (isa<Mapping_गाँठो>(जरा_2) || isa<Sequence_गाँठो>(जरा_2)))
      फर्काऊ नूतन (गाँठो_Allocator) Null_गाँठो(stream.CurrentDoc);

    setError("Unexpected token", T);
    फर्काऊ nullptr;
  }
  पक्ष Token::TK_Error:
    फर्काऊ nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  फर्काऊ nullptr;
}

तार्किक Document::parseDirectives() {
  तार्किक isDirective = असत्य;
  जबसम्म (सत्य) {
    Token T = peekNext();
    यदि (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = सत्य;
    } अन्यथा यदि (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = सत्य;
    } अन्यथा {
      रोक;
    }
  }
  फर्काऊ isDirective;
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
    फर्काऊ असत्य;
  }
  फर्काऊ सत्य;
}
