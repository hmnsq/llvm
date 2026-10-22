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
स्थिर EncodingInfo getUnicodeEncoding(StringRef आदान) {
  जर (आदान.empty())
    परत {UEF_Unknown, 0};

  निवड (uint8_t(आदान[0])) {
  स्थिती 0x00:
    जर (आदान.size() >= 4) {
      जर (  आदान[1] == 0
         && uint8_t(आदान[2]) == 0xFE
         && uint8_t(आदान[3]) == 0xFF)
        परत {UEF_UTF32_BE, 4};
      जर (आदान[1] == 0 && आदान[2] == 0 && आदान[3] != 0)
        परत {UEF_UTF32_BE, 0};
    }

    जर (आदान.size() >= 2 && आदान[1] != 0)
      परत {UEF_UTF16_BE, 0};
    परत {UEF_Unknown, 0};
  स्थिती 0xFF:
    जर (  आदान.size() >= 4
       && uint8_t(आदान[1]) == 0xFE
       && आदान[2] == 0
       && आदान[3] == 0)
      परत {UEF_UTF32_LE, 4};

    जर (आदान.size() >= 2 && uint8_t(आदान[1]) == 0xFE)
      परत {UEF_UTF16_LE, 2};
    परत {UEF_Unknown, 0};
  स्थिती 0xFE:
    जर (आदान.size() >= 2 && uint8_t(आदान[1]) == 0xFF)
      परत {UEF_UTF16_BE, 2};
    परत {UEF_Unknown, 0};
  स्थिती 0xEF:
    जर (  आदान.size() >= 3
       && uint8_t(आदान[1]) == 0xBB
       && uint8_t(आदान[2]) == 0xBF)
      परत {UEF_UTF8, 3};
    परत {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  जर (आदान.size() >= 4 && आदान[1] == 0 && आदान[2] == 0 && आदान[3] == 0)
    परत {UEF_UTF32_LE, 0};

  जर (आदान.size() >= 2 && आदान[1] == 0)
    परत {UEF_UTF16_LE, 0};

  परत {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
शून्य गाठ::anchor() {}
शून्य Null_गाठ::anchor() {}
शून्य Scalar_गाठ::anchor() {}
शून्य Block_Scalar_गाठ::anchor() {}
शून्य किल्ली_मूल्य_गाठ::anchor() {}
शून्य Mapping_गाठ::anchor() {}
शून्य Sequence_गाठ::anchor() {}
शून्य Alias_गाठ::anchor() {}

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
    TK_किल्ली,
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

  Token() = पूर्वनिर्धारित;
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
संरचना Simple_किल्ली {
  TokenQueueT::iterator Tok;
  अचिन्हित Column = 0;
  अचिन्हित ओळ = 0;
  अचिन्हित FlowLevel = 0;
  तार्किक IsRequired = असत्य;

  तार्किक चालक ==(स्थिरांक Simple_किल्ली &Other) {
    परत Tok == Other.Tok;
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
  जर (Position < End && (*Position & 0x80) == 0) {
    परत {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  जर (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    जर (codepoint >= 0x80)
      परत {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  जर (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    जर (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      परत {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  जर (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    जर (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      परत {codepoint, 4};
  }
  परत {0, 0};
}

नामाकाश llvm {
नामाकाश yaml {

/// Scans YAML tokens from a MemoryBuffer.
वर्ग Scanner {
सार्वजनिक:
  Scanner(StringRef आदान, SourceMgr &SM, तार्किक ShowColors = सत्य,
          std::error_code *EC = nullptr);
  Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, तार्किक ShowColors = सत्य,
          std::error_code *EC = nullptr);

  /// Parse the next token and return it without popping it.
  Token &peekNext();

  /// Parse the next token and pop it from the queue.
  Token getNext();

  शून्य printError(SMLoc Loc, SourceMgr::DiagKind Kind, स्थिरांक Twine &Message,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.PrintMessage(Loc, Kind, Message, Ranges, /* FixIts= */ {}, ShowColors);
  }

  शून्य setError(स्थिरांक Twine &Message, StringRef::iterator Position) {
    जर (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    जर (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    जर (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = सत्य;
  }

  /// Returns true if an error occurred while parsing.
  तार्किक failed() {
    परत Failed;
  }

खाजगी:
  शून्य init(MemoryBufferRef Buffer);

  StringRef current_आदान() {
    परत StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    परत ::decodeUTF8(StringRef(Position, End - Position));
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
  तार्किक is_ओळ_Empty(StringRef ओळ);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  तार्किक consume_ओळ_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  शून्य save_Simple_किल्ली_Candidate( TokenQueueT::iterator Tok
                             , अचिन्हित AtColumn
                             , तार्किक IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  शून्य remove_Stale_Simple_किल्ली_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  शून्य remove_Simple_किल्ली_Candidates_On_Flow_Level(अचिन्हित Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  तार्किक unrollIndent(पूर्णांक ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  तार्किक rollIndent( पूर्णांक ToColumn
                 , Token::TokenKind Kind
                 , TokenQueueT::iterator घाला_Point);

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
  तार्किक scan_किल्ली();

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
  तार्किक scanBlockScalarIndicators(अक्षर &StyleIndicator, अक्षर &ChompingIndicator,
                                 अचिन्हित &IndentIndicator, तार्किक &IsDone);

  /// Scan a style indicator in a block scalar header.
  अक्षर scanBlockStyleIndicator();

  /// Scan a chomping indicator in a block scalar header.
  अक्षर scanBlockChompingIndicator();

  /// Scan an indentation indicator in a block scalar header.
  अचिन्हित scanBlockIndentationIndicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  तार्किक scanBlockScalarHeader(अक्षर &ChompingIndicator, अचिन्हित &IndentIndicator,
                             तार्किक &IsDone);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  तार्किक findBlockScalarIndent(अचिन्हित &BlockIndent, अचिन्हित BlockExitIndent,
                             अचिन्हित &ओळ_Breaks, तार्किक &IsDone);

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
  MemoryBufferRef आदान_Buffer;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator End;

  /// Current YAML indentation level in spaces.
  पूर्णांक Indent;

  /// Current column number in Unicode code points.
  अचिन्हित Column;

  /// Current line number.
  अचिन्हित ओळ;

  /// How deep we are in flow style containers. 0 Means at block level.
  अचिन्हित FlowLevel;

  /// Are we at the start of the stream?
  तार्किक IsStartOfStream;

  /// Can the next token be the start of a simple key?
  तार्किक Is_Simple_किल्ली_Allowed;

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
  SmallVector<पूर्णांक, 4> Indents;

  /// Potential simple keys.
  SmallVector<Simple_किल्ली, 4> Simple_किल्ल्या;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
स्थिर शून्य encodeUTF8( uint32_t Unicode_Scalar_मूल्य
                      , SmallVectorImpl<अक्षर> &Result) {
  जर (Unicode_Scalar_मूल्य <= 0x7F) {
    Result.ढकला_back(Unicode_Scalar_मूल्य & 0x7F);
  } अन्यथा जर (Unicode_Scalar_मूल्य <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((Unicode_Scalar_मूल्य & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Unicode_Scalar_मूल्य & 0x3F);
    Result.ढकला_back(FirstByte);
    Result.ढकला_back(SecondByte);
  } अन्यथा जर (Unicode_Scalar_मूल्य <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((Unicode_Scalar_मूल्य & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_मूल्य & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Unicode_Scalar_मूल्य & 0x3F);
    Result.ढकला_back(FirstByte);
    Result.ढकला_back(SecondByte);
    Result.ढकला_back(ThirdByte);
  } अन्यथा जर (Unicode_Scalar_मूल्य <= 0x10FFFF) {
    uint8_t FirstByte = 0xF0 | ((Unicode_Scalar_मूल्य & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_मूल्य & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Unicode_Scalar_मूल्य & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_मूल्य & 0x3F);
    Result.ढकला_back(FirstByte);
    Result.ढकला_back(SecondByte);
    Result.ढकला_back(ThirdByte);
    Result.ढकला_back(FourthByte);
  }
}

तार्किक yaml::dumpTokens(StringRef आदान, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(आदान, SM);
  पर्यंत (सत्य) {
    Token T = scanner.getNext();
    निवड (T.Kind) {
    स्थिती Token::TK_StreamStart:
      OS << "Stream-Start: ";
      थांबा;
    स्थिती Token::TK_StreamEnd:
      OS << "Stream-End: ";
      थांबा;
    स्थिती Token::TK_VersionDirective:
      OS << "Version-Directive: ";
      थांबा;
    स्थिती Token::TK_TagDirective:
      OS << "Tag-Directive: ";
      थांबा;
    स्थिती Token::TK_DocumentStart:
      OS << "Document-Start: ";
      थांबा;
    स्थिती Token::TK_DocumentEnd:
      OS << "Document-End: ";
      थांबा;
    स्थिती Token::TK_BlockEntry:
      OS << "Block-Entry: ";
      थांबा;
    स्थिती Token::TK_BlockEnd:
      OS << "Block-End: ";
      थांबा;
    स्थिती Token::TK_BlockSequenceStart:
      OS << "Block-Sequence-Start: ";
      थांबा;
    स्थिती Token::TK_BlockMappingStart:
      OS << "Block-Mapping-Start: ";
      थांबा;
    स्थिती Token::TK_FlowEntry:
      OS << "Flow-Entry: ";
      थांबा;
    स्थिती Token::TK_FlowSequenceStart:
      OS << "Flow-Sequence-Start: ";
      थांबा;
    स्थिती Token::TK_FlowSequenceEnd:
      OS << "Flow-Sequence-End: ";
      थांबा;
    स्थिती Token::TK_FlowMappingStart:
      OS << "Flow-Mapping-Start: ";
      थांबा;
    स्थिती Token::TK_FlowMappingEnd:
      OS << "Flow-Mapping-End: ";
      थांबा;
    स्थिती Token::TK_किल्ली:
      OS << "Key: ";
      थांबा;
    स्थिती Token::TK_मूल्य:
      OS << "Value: ";
      थांबा;
    स्थिती Token::TK_Scalar:
      OS << "Scalar: ";
      थांबा;
    स्थिती Token::TK_BlockScalar:
      OS << "Block Scalar: ";
      थांबा;
    स्थिती Token::TK_Alias:
      OS << "Alias: ";
      थांबा;
    स्थिती Token::TK_Anchor:
      OS << "Anchor: ";
      थांबा;
    स्थिती Token::TK_Tag:
      OS << "Tag: ";
      थांबा;
    स्थिती Token::TK_Error:
      थांबा;
    }
    OS << T.Range << "\n";
    जर (T.Kind == Token::TK_StreamEnd)
      थांबा;
    अन्यथा जर (T.Kind == Token::TK_Error)
      परत असत्य;
  }
  परत सत्य;
}

तार्किक yaml::scanTokens(StringRef आदान) {
  SourceMgr SM;
  Scanner scanner(आदान, SM);
  पर्यंत (सत्य) {
    Token T = scanner.getNext();
    जर (T.Kind == Token::TK_StreamEnd)
      थांबा;
    अन्यथा जर (T.Kind == Token::TK_Error)
      परत असत्य;
  }
  परत सत्य;
}

std::string yaml::escape(StringRef आदान, तार्किक EscapePrintable) {
  std::string Escaped_आदान;
  साठी (StringRef::iterator i = आदान.begin(), e = आदान.end(); i != e; ++i) {
    जर (*i == '\\')
      Escaped_आदान += "\\\\";
    अन्यथा जर (*i == '"')
      Escaped_आदान += "\\\"";
    अन्यथा जर (*i == 0)
      Escaped_आदान += "\\0";
    अन्यथा जर (*i == 0x07)
      Escaped_आदान += "\\a";
    अन्यथा जर (*i == 0x08)
      Escaped_आदान += "\\b";
    अन्यथा जर (*i == 0x09)
      Escaped_आदान += "\\t";
    अन्यथा जर (*i == 0x0A)
      Escaped_आदान += "\\n";
    अन्यथा जर (*i == 0x0B)
      Escaped_आदान += "\\v";
    अन्यथा जर (*i == 0x0C)
      Escaped_आदान += "\\f";
    अन्यथा जर (*i == 0x0D)
      Escaped_आदान += "\\r";
    अन्यथा जर (*i == 0x1B)
      Escaped_आदान += "\\e";
    अन्यथा जर ((अचिन्हित अक्षर)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_आदान += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } अन्यथा जर (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_मूल्य
        = decodeUTF8(StringRef(i, आदान.end() - i));
      जर (Unicode_Scalar_मूल्य.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_आदान, Val);
        // FIXME: Error reporting.
        परत Escaped_आदान;
      }
      जर (Unicode_Scalar_मूल्य.first == 0x85)
        Escaped_आदान += "\\N";
      अन्यथा जर (Unicode_Scalar_मूल्य.first == 0xA0)
        Escaped_आदान += "\\_";
      अन्यथा जर (Unicode_Scalar_मूल्य.first == 0x2028)
        Escaped_आदान += "\\L";
      अन्यथा जर (Unicode_Scalar_मूल्य.first == 0x2029)
        Escaped_आदान += "\\P";
      अन्यथा जर (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_मूल्य.first))
        Escaped_आदान += StringRef(i, Unicode_Scalar_मूल्य.second);
      अन्यथा {
        std::string HexStr = utohexstr(Unicode_Scalar_मूल्य.first);
        जर (HexStr.size() <= 2)
          Escaped_आदान += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        अन्यथा जर (HexStr.size() <= 4)
          Escaped_आदान += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        अन्यथा जर (HexStr.size() <= 8)
          Escaped_आदान += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += Unicode_Scalar_मूल्य.second - 1;
    } अन्यथा {
      Escaped_आदान.ढकला_back(*i);
    }
  }
  परत Escaped_आदान;
}

std::optional<तार्किक> yaml::parseBool(StringRef S) {
  निवड (S.size()) {
  स्थिती 1:
    निवड (S.front()) {
    स्थिती 'y':
    स्थिती 'Y':
      परत सत्य;
    स्थिती 'n':
    स्थिती 'N':
      परत असत्य;
    पूर्वनिर्धारित:
      परत std::nullopt;
    }
  स्थिती 2:
    निवड (S.front()) {
    स्थिती 'O':
      जर (S[1] == 'N') // ON
        परत सत्य;
      [[fallthrough]];
    स्थिती 'o':
      जर (S[1] == 'n') //[Oo]n
        परत सत्य;
      परत std::nullopt;
    स्थिती 'N':
      जर (S[1] == 'O') // NO
        परत असत्य;
      [[fallthrough]];
    स्थिती 'n':
      जर (S[1] == 'o') //[Nn]o
        परत असत्य;
      परत std::nullopt;
    पूर्वनिर्धारित:
      परत std::nullopt;
    }
  स्थिती 3:
    निवड (S.front()) {
    स्थिती 'O':
      जर (S.drop_front() == "FF") // OFF
        परत असत्य;
      [[fallthrough]];
    स्थिती 'o':
      जर (S.drop_front() == "ff") //[Oo]ff
        परत असत्य;
      परत std::nullopt;
    स्थिती 'Y':
      जर (S.drop_front() == "ES") // YES
        परत सत्य;
      [[fallthrough]];
    स्थिती 'y':
      जर (S.drop_front() == "es") //[Yy]es
        परत सत्य;
      परत std::nullopt;
    पूर्वनिर्धारित:
      परत std::nullopt;
    }
  स्थिती 4:
    निवड (S.front()) {
    स्थिती 'T':
      जर (S.drop_front() == "RUE") // TRUE
        परत सत्य;
      [[fallthrough]];
    स्थिती 't':
      जर (S.drop_front() == "rue") //[Tt]rue
        परत सत्य;
      परत std::nullopt;
    पूर्वनिर्धारित:
      परत std::nullopt;
    }
  स्थिती 5:
    निवड (S.front()) {
    स्थिती 'F':
      जर (S.drop_front() == "ALSE") // FALSE
        परत असत्य;
      [[fallthrough]];
    स्थिती 'f':
      जर (S.drop_front() == "alse") //[Ff]alse
        परत असत्य;
      परत std::nullopt;
    पूर्वनिर्धारित:
      परत std::nullopt;
    }
  पूर्वनिर्धारित:
    परत std::nullopt;
  }
}

Scanner::Scanner(StringRef आदान, SourceMgr &sm, तार्किक ShowColors,
                 std::error_code *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(MemoryBufferRef(आदान, "YAML"));
}

Scanner::Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, तार्किक ShowColors,
                 std::error_code *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(Buffer);
}

शून्य Scanner::init(MemoryBufferRef Buffer) {
  आदान_Buffer = Buffer;
  Current = आदान_Buffer.getBufferStart();
  End = आदान_Buffer.getBufferEnd();
  Indent = -1;
  Column = 0;
  ओळ = 0;
  FlowLevel = 0;
  IsStartOfStream = सत्य;
  Is_Simple_किल्ली_Allowed = सत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;
  Failed = असत्य;
  std::unique_ptr<MemoryBuffer> आदान_Buffer_Owner =
      MemoryBuffer::getMemBuffer(Buffer, /*RequiresNullTerminator=*/असत्य);
  SM.AddNewSourceBuffer(std::move(आदान_Buffer_Owner), SMLoc());
}

Token &Scanner::peekNext() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  तार्किक NeedMore = असत्य;
  पर्यंत (सत्य) {
    जर (TokenQueue.empty() || NeedMore) {
      जर (!fetchMoreTokens()) {
        TokenQueue.clear();
        Simple_किल्ल्या.clear();
        TokenQueue.ढकला_back(Token());
        परत TokenQueue.front();
      }
    }
    प्रतिज्ञा(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_किल्ली_Candidates();
    Simple_किल्ली SK;
    SK.Tok = TokenQueue.begin();
    जर (!is_contained(Simple_किल्ल्या, SK))
      थांबा;
    अन्यथा
      NeedMore = सत्य;
  }
  परत TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  जर (!TokenQueue.empty())
    TokenQueue.काढा_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  जर (TokenQueue.empty())
    TokenQueue.resetAlloc();

  परत Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  जर (Position == End)
    परत Position;
  // Check 7 bit c-printable - b-char.
  जर (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    परत Position + 1;

  // Check for valid UTF-8.
  जर (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    जर (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      परत Position + u8d.second;
  }
  परत Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  जर (Position == End)
    परत Position;
  जर (*Position == 0x0D) {
    जर (Position + 1 != End && *(Position + 1) == 0x0A)
      परत Position + 2;
    परत Position + 1;
  }

  जर (*Position == 0x0A)
    परत Position + 1;
  परत Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  जर (Position == End)
    परत Position;
  जर (*Position == ' ')
    परत Position + 1;
  परत Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  जर (Position == End)
    परत Position;
  जर (*Position == ' ' || *Position == '\t')
    परत Position + 1;
  परत Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  जर (Position == End)
    परत Position;
  जर (*Position == ' ' || *Position == '\t')
    परत Position;
  परत skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  पर्यंत (सत्य) {
    StringRef::iterator i = (हे->*Func)(Position);
    जर (i == Position)
      थांबा;
    Position = i;
  }
  परत Position;
}

शून्य Scanner::advanceWhile(SkipWhileFunc Func) {
  स्वतः Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

स्थिर तार्किक is_ns_hex_digit(स्थिरांक अक्षर C) { परत isAlnum(C); }

स्थिर तार्किक is_ns_word_char(स्थिरांक अक्षर C) { परत C == '-' || isAlpha(C); }

शून्य Scanner::scan_ns_uri_char() {
  पर्यंत (सत्य) {
    जर (Current == End)
      थांबा;
    जर ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } अन्यथा {
      थांबा;
    }
  }
}

तार्किक Scanner::consume(uint32_t Expected) {
  जर (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    परत असत्य;
  }
  जर (Current == End)
    परत असत्य;
  जर (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    परत असत्य;
  }
  जर (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    परत सत्य;
  }
  परत असत्य;
}

शून्य Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  प्रतिज्ञा(Current <= End && "Skipped past the end");
}

तार्किक Scanner::isBlankOrBreak(StringRef::iterator Position) {
  जर (Position == End)
    परत असत्य;
  परत *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

तार्किक Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  जर (Position == End || isBlankOrBreak(Position))
    परत असत्य;
  जर (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    परत असत्य;
  परत सत्य;
}

तार्किक Scanner::is_ओळ_Empty(StringRef ओळ) {
  साठी (स्थिरांक स्वतः *Position = ओळ.begin(); Position != ओळ.end(); ++Position)
    जर (!isBlankOrBreak(Position))
      परत असत्य;
  परत सत्य;
}

तार्किक Scanner::consume_ओळ_Break_If_Present() {
  स्वतः Next = skip_b_break(Current);
  जर (Next == Current)
    परत असत्य;
  Column = 0;
  ++ओळ;
  Current = Next;
  परत सत्य;
}

शून्य Scanner::save_Simple_किल्ली_Candidate( TokenQueueT::iterator Tok
                                    , अचिन्हित AtColumn
                                    , तार्किक IsRequired) {
  जर (Is_Simple_किल्ली_Allowed) {
    Simple_किल्ली SK;
    SK.Tok = Tok;
    SK.ओळ = ओळ;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_किल्ल्या.ढकला_back(SK);
  }
}

शून्य Scanner::remove_Stale_Simple_किल्ली_Candidates() {
  साठी (SmallVectorImpl<Simple_किल्ली>::iterator i = Simple_किल्ल्या.begin();
                                            i != Simple_किल्ल्या.end();) {
    जर (i->ओळ != ओळ || i->Column + 1024 < Column) {
      जर (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_किल्ल्या.erase(i);
    } अन्यथा {
      ++i;
    }
  }
}

शून्य Scanner::remove_Simple_किल्ली_Candidates_On_Flow_Level(अचिन्हित Level) {
  जर (!Simple_किल्ल्या.empty() && (Simple_किल्ल्या.end() - 1)->FlowLevel == Level)
    Simple_किल्ल्या.काढा_back();
}

तार्किक Scanner::unrollIndent(पूर्णांक ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  जर (FlowLevel != 0)
    परत सत्य;

  पर्यंत (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.ढकला_back(T);
    Indent = Indents.काढा_back_val();
  }

  परत सत्य;
}

तार्किक Scanner::rollIndent( पूर्णांक ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator घाला_Point) {
  जर (FlowLevel)
    परत सत्य;
  जर (Indent < ToColumn) {
    Indents.ढकला_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.घाला(घाला_Point, T);
  }
  परत सत्य;
}

शून्य Scanner::skipComment() {
  जर (Current == End || *Current != '#')
    परत;
  पर्यंत (सत्य) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    जर (I == Current)
      थांबा;
    Current = I;
    ++Column;
  }
}

शून्य Scanner::scanToNextToken() {
  पर्यंत (सत्य) {
    पर्यंत (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    जर (i == Current)
      थांबा;
    Current = i;
    ++ओळ;
    Column = 0;
    // New lines may start a simple key.
    जर (!FlowLevel)
      Is_Simple_किल्ली_Allowed = सत्य;
  }
}

तार्किक Scanner::scanStreamStart() {
  IsStartOfStream = असत्य;

  EncodingInfo EI = getUnicodeEncoding(current_आदान());

  Token T;
  T.Kind = Token::TK_StreamStart;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.ढकला_back(T);
  Current += EI.second;
  परत सत्य;
}

तार्किक Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  जर (Column != 0) {
    Column = 0;
    ++ओळ;
  }

  unrollIndent(-1);
  Simple_किल्ल्या.clear();
  Is_Simple_किल्ली_Allowed = असत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  Token T;
  T.Kind = Token::TK_StreamEnd;
  T.Range = StringRef(Current, 0);
  TokenQueue.ढकला_back(T);
  परत सत्य;
}

तार्किक Scanner::scanDirective() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_किल्ल्या.clear();
  Is_Simple_किल्ली_Allowed = असत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  StringRef::iterator Start = Current;
  consume('%');
  StringRef::iterator NameStart = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(NameStart, Current - NameStart);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  जर (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ढकला_back(T);
    परत सत्य;
  } अन्यथा जर(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ढकला_back(T);
    परत सत्य;
  }
  परत असत्य;
}

तार्किक Scanner::scanDocumentIndicator(तार्किक IsStart) {
  unrollIndent(-1);
  Simple_किल्ल्या.clear();
  Is_Simple_किल्ली_Allowed = असत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  Token T;
  T.Kind = IsStart ? Token::TK_DocumentStart : Token::TK_DocumentEnd;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.ढकला_back(T);
  परत सत्य;
}

तार्किक Scanner::scanFlowCollectionStart(तार्किक IsSequence) {
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ढकला_back(T);

  // [ and { may begin a simple key.
  save_Simple_किल्ली_Candidate(--TokenQueue.end(), Column - 1, असत्य);

  // And may also be followed by a simple key.
  Is_Simple_किल्ली_Allowed = सत्य;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;
  ++FlowLevel;
  परत सत्य;
}

तार्किक Scanner::scanFlowCollectionEnd(तार्किक IsSequence) {
  remove_Simple_किल्ली_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_किल्ली_Allowed = असत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = सत्य;
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceEnd
                      : Token::TK_FlowMappingEnd;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ढकला_back(T);
  जर (FlowLevel)
    --FlowLevel;
  परत सत्य;
}

तार्किक Scanner::scanFlowEntry() {
  remove_Simple_किल्ली_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_किल्ली_Allowed = सत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;
  Token T;
  T.Kind = Token::TK_FlowEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ढकला_back(T);
  परत सत्य;
}

तार्किक Scanner::scanBlockEntry() {
  rollIndent(Column, Token::TK_BlockSequenceStart, TokenQueue.end());
  remove_Simple_किल्ली_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_किल्ली_Allowed = सत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;
  Token T;
  T.Kind = Token::TK_BlockEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ढकला_back(T);
  परत सत्य;
}

तार्किक Scanner::scan_किल्ली() {
  जर (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  remove_Simple_किल्ली_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_किल्ली_Allowed = !FlowLevel;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  Token T;
  T.Kind = Token::TK_किल्ली;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ढकला_back(T);
  परत सत्य;
}

तार्किक Scanner::scan_मूल्य() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  जर (!Simple_किल्ल्या.empty()) {
    Simple_किल्ली SK = Simple_किल्ल्या.काढा_back_val();
    Token T;
    T.Kind = Token::TK_किल्ली;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    साठी (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      जर (i == SK.Tok)
        थांबा;
    }
    जर (i == e) {
      Failed = सत्य;
      परत असत्य;
    }
    i = TokenQueue.घाला(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    Is_Simple_किल्ली_Allowed = असत्य;
  } अन्यथा {
    जर (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    Is_Simple_किल्ली_Allowed = !FlowLevel;
  }
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  Token T;
  T.Kind = Token::TK_मूल्य;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ढकला_back(T);
  परत सत्य;
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
  पर्यंत (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  परत (Position - 1 - I) % 2 == 1;
}

तार्किक Scanner::scanFlowScalar(तार्किक IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  अचिन्हित ColStart = Column;
  जर (IsDoubleQuoted) {
    करा {
      ++Current;
      पर्यंत (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } पर्यंत (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } अन्यथा {
    skip(1);
    पर्यंत (Current != End) {
      // Skip a ' followed by another '.
      जर (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        सुरूठेवा;
      } अन्यथा जर (*Current == '\'')
        थांबा;
      StringRef::iterator i = skip_nb_char(Current);
      जर (i == Current) {
        i = skip_b_break(Current);
        जर (i == Current)
          थांबा;
        Current = i;
        Column = 0;
        ++ओळ;
      } अन्यथा {
        जर (i == End)
          थांबा;
        Current = i;
        ++Column;
      }
    }
  }

  जर (Current == End) {
    setError("Expected quote at end of scalar", Current);
    परत असत्य;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ढकला_back(T);

  save_Simple_किल्ली_Candidate(--TokenQueue.end(), ColStart, असत्य);

  Is_Simple_किल्ली_Allowed = असत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = सत्य;

  परत सत्य;
}

तार्किक Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  अचिन्हित ColStart = Column;
  अचिन्हित LeadingBlanks = 0;
  प्रतिज्ञा(Indent >= -1 && "Indent must be >= -1 !");
  अचिन्हित indent = static_cast<अचिन्हित>(Indent + 1);
  पर्यंत (Current != End) {
    जर (*Current == '#')
      थांबा;

    पर्यंत (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      जर (i == Current)
        थांबा;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    जर (!isBlankOrBreak(Current))
      थांबा;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    पर्यंत (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      जर (i != Tmp) {
        जर (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          परत असत्य;
        }
        Tmp = i;
        ++Column;
      } अन्यथा {
        i = skip_b_break(Tmp);
        जर (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++ओळ;
      }
    }

    जर (!FlowLevel && Column < indent)
      थांबा;

    Current = Tmp;
  }
  जर (Start == Current) {
    setError("Got empty plain scalar", Start);
    परत असत्य;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ढकला_back(T);

  // Plain scalars can be simple keys.
  save_Simple_किल्ली_Candidate(--TokenQueue.end(), ColStart, असत्य);

  Is_Simple_किल्ली_Allowed = असत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  परत सत्य;
}

तार्किक Scanner::scanAliasOrAnchor(तार्किक IsAlias) {
  StringRef::iterator Start = Current;
  अचिन्हित ColStart = Column;
  skip(1);
  पर्यंत (Current != End) {
    जर (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      थांबा;
    StringRef::iterator i = skip_ns_char(Current);
    जर (i == Current)
      थांबा;
    Current = i;
    ++Column;
  }

  जर (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    परत असत्य;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ढकला_back(T);

  // Alias and anchors can be simple keys.
  save_Simple_किल्ली_Candidate(--TokenQueue.end(), ColStart, असत्य);

  Is_Simple_किल्ली_Allowed = असत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  परत सत्य;
}

तार्किक Scanner::scanBlockScalarIndicators(अक्षर &StyleIndicator,
                                        अक्षर &ChompingIndicator,
                                        अचिन्हित &IndentIndicator,
                                        तार्किक &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  जर (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    परत असत्य;
  परत सत्य;
}

अक्षर Scanner::scanBlockStyleIndicator() {
  अक्षर Indicator = ' ';
  जर (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  परत Indicator;
}

अक्षर Scanner::scanBlockChompingIndicator() {
  अक्षर Indicator = ' ';
  जर (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  परत Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
स्थिर अचिन्हित get_Chomped_ओळ_Breaks(अक्षर ChompingIndicator,
                                     अचिन्हित ओळ_Breaks, StringRef Str) {
  जर (ChompingIndicator == '-') // Strip all line breaks.
    परत 0;
  जर (ChompingIndicator == '+') // Keep all line breaks.
    परत ओळ_Breaks;
  // Clip trailing lines.
  परत Str.empty() ? 0 : 1;
}

अचिन्हित Scanner::scanBlockIndentationIndicator() {
  अचिन्हित Indent = 0;
  जर (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = अचिन्हित(*Current - '0');
    skip(1);
  }
  परत Indent;
}

तार्किक Scanner::scanBlockScalarHeader(अक्षर &ChompingIndicator,
                                    अचिन्हित &IndentIndicator, तार्किक &IsDone) {
  स्वतः Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  जर (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  जर (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ढकला_back(T);
    IsDone = सत्य;
    परत सत्य;
  }

  जर (!consume_ओळ_Break_If_Present()) {
    setError("Expected a line break after block scalar header", Current);
    परत असत्य;
  }
  परत सत्य;
}

तार्किक Scanner::findBlockScalarIndent(अचिन्हित &BlockIndent,
                                    अचिन्हित BlockExitIndent,
                                    अचिन्हित &ओळ_Breaks, तार्किक &IsDone) {
  अचिन्हित Max_All_Space_ओळ_Characters = 0;
  StringRef::iterator Longest_All_Space_ओळ;

  पर्यंत (सत्य) {
    advanceWhile(&Scanner::skip_s_space);
    जर (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      जर (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = सत्य;
        परत सत्य;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      जर (Max_All_Space_ओळ_Characters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_ओळ);
        परत असत्य;
      }
      परत सत्य;
    }
    जर (skip_b_break(Current) != Current &&
        Column > Max_All_Space_ओळ_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_ओळ_Characters = Column;
      Longest_All_Space_ओळ = Current;
    }

    // Check for EOF.
    जर (Current == End) {
      IsDone = सत्य;
      परत सत्य;
    }

    जर (!consume_ओळ_Break_If_Present()) {
      IsDone = सत्य;
      परत सत्य;
    }
    ++ओळ_Breaks;
  }
  परत सत्य;
}

तार्किक Scanner::scanBlockScalarIndent(अचिन्हित BlockIndent,
                                    अचिन्हित BlockExitIndent, तार्किक &IsDone) {
  // Skip the indentation.
  पर्यंत (Column < BlockIndent) {
    स्वतः I = skip_s_space(Current);
    जर (I == Current)
      थांबा;
    Current = I;
    ++Column;
  }

  जर (skip_nb_char(Current) == Current)
    परत सत्य;

  जर (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = सत्य;
    परत सत्य;
  }

  जर (Column < BlockIndent) {
    जर (Current != End && *Current == '#') { // Trailing comment.
      IsDone = सत्य;
      परत सत्य;
    }
    setError("A text line is less indented than the block scalar", Current);
    परत असत्य;
  }
  परत सत्य; // A normal text line.
}

तार्किक Scanner::scanBlockScalar(तार्किक IsLiteral) {
  प्रतिज्ञा(*Current == '|' || *Current == '>');
  अक्षर StyleIndicator;
  अक्षर ChompingIndicator;
  अचिन्हित BlockIndent;
  तार्किक IsDone = असत्य;
  जर (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    परत असत्य;
  जर (IsDone)
    परत सत्य;
  तार्किक IsFolded = StyleIndicator == '>';

  स्थिरांक स्वतः *Start = Current;
  अचिन्हित BlockExitIndent = Indent < 0 ? 0 : (अचिन्हित)Indent;
  अचिन्हित ओळ_Breaks = 0;
  जर (BlockIndent == 0) {
    जर (!findBlockScalarIndent(BlockIndent, BlockExitIndent, ओळ_Breaks,
                               IsDone))
      परत असत्य;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  पर्यंत (!IsDone) {
    जर (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      परत असत्य;
    जर (IsDone)
      थांबा;

    // Parse the current line.
    स्वतः ओळ_Start = Current;
    advanceWhile(&Scanner::skip_nb_char);
    जर (ओळ_Start != Current) {
      जर (ओळ_Breaks && IsFolded && !Scanner::is_ओळ_Empty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        जर (ओळ_Breaks == 1) {
          Str.append(ओळ_Breaks,
                     is_ओळ_Empty(StringRef(ओळ_Start, Current - ओळ_Start))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        ओळ_Breaks--;
      }
      Str.append(ओळ_Breaks, '\n');
      Str.append(StringRef(ओळ_Start, Current - ओळ_Start));
      ओळ_Breaks = 0;
    }

    // Check for EOF.
    जर (Current == End)
      थांबा;

    जर (!consume_ओळ_Break_If_Present())
      थांबा;
    ++ओळ_Breaks;
  }

  जर (Current == End && !ओळ_Breaks)
    // Ensure that there is at least one line break before the end of file.
    ओळ_Breaks = 1;
  Str.append(get_Chomped_ओळ_Breaks(ChompingIndicator, ओळ_Breaks, Str), '\n');

  // New lines may start a simple key.
  जर (!FlowLevel)
    Is_Simple_किल्ली_Allowed = सत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.मूल्य = std::string(Str);
  TokenQueue.ढकला_back(T);
  परत सत्य;
}

तार्किक Scanner::scanTag() {
  StringRef::iterator Start = Current;
  अचिन्हित ColStart = Column;
  skip(1); // Eat !.
  जर (Current == End || isBlankOrBreak(Current)); // An empty tag.
  अन्यथा जर (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    जर (!consume('>'))
      परत असत्य;
  } अन्यथा {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.Kind = Token::TK_Tag;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ढकला_back(T);

  // Tags can be simple keys.
  save_Simple_किल्ली_Candidate(--TokenQueue.end(), ColStart, असत्य);

  Is_Simple_किल्ली_Allowed = असत्य;
  Is_Adjacent_मूल्य_Allowed_In_Flow = असत्य;

  परत सत्य;
}

तार्किक Scanner::fetchMoreTokens() {
  जर (IsStartOfStream)
    परत scanStreamStart();

  scanToNextToken();

  जर (Current == End)
    परत scanStreamEnd();

  remove_Stale_Simple_किल्ली_Candidates();

  unrollIndent(Column);

  जर (Column == 0 && *Current == '%')
    परत scanDirective();

  जर (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    परत scanDocumentIndicator(सत्य);

  जर (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    परत scanDocumentIndicator(असत्य);

  जर (*Current == '[')
    परत scanFlowCollectionStart(सत्य);

  जर (*Current == '{')
    परत scanFlowCollectionStart(असत्य);

  जर (*Current == ']')
    परत scanFlowCollectionEnd(सत्य);

  जर (*Current == '}')
    परत scanFlowCollectionEnd(असत्य);

  जर (*Current == ',')
    परत scanFlowEntry();

  जर (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    परत scanBlockEntry();

  जर (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    परत scan_किल्ली();

  जर (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || Is_Adjacent_मूल्य_Allowed_In_Flow))
    परत scan_मूल्य();

  जर (*Current == '*')
    परत scanAliasOrAnchor(सत्य);

  जर (*Current == '&')
    परत scanAliasOrAnchor(असत्य);

  जर (*Current == '!')
    परत scanTag();

  जर (*Current == '|' && !FlowLevel)
    परत scanBlockScalar(सत्य);

  जर (*Current == '>' && !FlowLevel)
    परत scanBlockScalar(असत्य);

  जर (*Current == '\'')
    परत scanFlowScalar(असत्य);

  जर (*Current == '"')
    परत scanFlowScalar(सत्य);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  जर ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    परत scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  परत असत्य;
}

Stream::Stream(StringRef आदान, SourceMgr &SM, तार्किक ShowColors,
               std::error_code *EC)
    : scanner(नवीन Scanner(आदान, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef आदान_Buffer, SourceMgr &SM, तार्किक ShowColors,
               std::error_code *EC)
    : scanner(नवीन Scanner(आदान_Buffer, SM, ShowColors, EC)) {}

Stream::~Stream() = पूर्वनिर्धारित;

तार्किक Stream::failed() { परत scanner->failed(); }

शून्य Stream::printError(गाठ *N, स्थिरांक Twine &संदेश_2, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), संदेश_2, Kind);
}

शून्य Stream::printError(स्थिरांक SMRange &Range, स्थिरांक Twine &संदेश_2,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, संदेश_2, Range);
}

document_iterator Stream::begin() {
  जर (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(नवीन Document(*हे));
  परत document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  परत document_iterator();
}

शून्य Stream::skip() {
  साठी (Document &Doc : *हे)
    Doc.skip();
}

गाठ::गाठ(अचिन्हित पूर्णांक Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string गाठ::getVerbatimTag() स्थिरांक {
  StringRef Raw = getRawTag();
  जर (!Raw.empty() && Raw != "!") {
    std::string Ret;
    जर (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      परत Ret;
    } अन्यथा जर (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      परत Ret;
    } अन्यथा {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      जर (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      अन्यथा {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      परत Ret;
    }
  }

  निवड (getType()) {
  स्थिती NK_Null:
    परत "tag:yaml.org,2002:null";
  स्थिती NK_Scalar:
  स्थिती NK_BlockScalar:
    // TODO: Tag resolution.
    परत "tag:yaml.org,2002:str";
  स्थिती NK_Mapping:
    परत "tag:yaml.org,2002:map";
  स्थिती NK_Sequence:
    परत "tag:yaml.org,2002:seq";
  }

  परत "";
}

Token &गाठ::peekNext() {
  परत Doc->peekNext();
}

Token गाठ::getNext() {
  परत Doc->getNext();
}

गाठ *गाठ::parse_Block_गाठ() {
  परत Doc->parse_Block_गाठ();
}

BumpPtrAllocator &गाठ::getAllocator() {
  परत Doc->गाठ_Allocator;
}

शून्य गाठ::setError(स्थिरांक Twine &संदेश_2, Token &Tok) स्थिरांक {
  Doc->setError(संदेश_2, Tok);
}

तार्किक गाठ::failed() स्थिरांक {
  परत Doc->failed();
}

StringRef Scalar_गाठ::get_मूल्य(SmallVectorImpl<अक्षर> &Storage) स्थिरांक {
  जर (मूल्य[0] == '"')
    परत get_Double_Quoted_मूल्य(मूल्य, Storage);
  जर (मूल्य[0] == '\'')
    परत get_Single_Quoted_मूल्य(मूल्य, Storage);
  परत get_Plain_मूल्य(मूल्य, Storage);
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
parse_Scalar_मूल्य(StringRef Unquoted_मूल्य, SmallVectorImpl<अक्षर> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<अक्षर> &)>
                     UnescapeCallback) {
  माप_प्रकार I = Unquoted_मूल्य.find_first_of(LookupChars);
  जर (I == StringRef::npos)
    परत Unquoted_मूल्य;

  Storage.clear();
  Storage.reserve(Unquoted_मूल्य.size());
  अक्षर Last_New_ओळ_Added_As = '\0';
  साठी (; I != StringRef::npos; I = Unquoted_मूल्य.find_first_of(LookupChars)) {
    जर (Unquoted_मूल्य[I] != '\r' && Unquoted_मूल्य[I] != '\n') {
      llvm::append_range(Storage, Unquoted_मूल्य.take_front(I));
      Unquoted_मूल्य = UnescapeCallback(Unquoted_मूल्य.drop_front(I), Storage);
      Last_New_ओळ_Added_As = '\0';
      सुरूठेवा;
    }
    जर (माप_प्रकार LastNonSWhite = Unquoted_मूल्य.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_मूल्य.take_front(LastNonSWhite + 1));
      Storage.ढकला_back(' ');
      Last_New_ओळ_Added_As = ' ';
    } अन्यथा {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      निवड (Last_New_ओळ_Added_As) {
      स्थिती ' ':
        प्रतिज्ञा(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        Last_New_ओळ_Added_As = '\n';
        थांबा;
      स्थिती '\n':
        प्रतिज्ञा(!Storage.empty() && Storage.back() == '\n');
        Storage.ढकला_back('\n');
        थांबा;
      पूर्वनिर्धारित:
        Storage.ढकला_back(' ');
        Last_New_ओळ_Added_As = ' ';
        थांबा;
      }
    }
    // Handle Windows-style EOL
    जर (Unquoted_मूल्य.substr(I, 2) == "\r\n")
      I++;
    Unquoted_मूल्य = Unquoted_मूल्य.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_मूल्य);
  परत StringRef(Storage.begin(), Storage.size());
}

StringRef
Scalar_गाठ::get_Double_Quoted_मूल्य(StringRef Raw_मूल्य,
                                 SmallVectorImpl<अक्षर> &Storage) स्थिरांक {
  प्रतिज्ञा(Raw_मूल्य.size() >= 2 && Raw_मूल्य.front() == '"' &&
         Raw_मूल्य.back() == '"');
  StringRef Unquoted_मूल्य = Raw_मूल्य.substr(1, Raw_मूल्य.size() - 2);

  स्वतः UnescapeFunc = [हे](StringRef Unquoted_मूल्य,
                             SmallVectorImpl<अक्षर> &Storage) {
    प्रतिज्ञा(Unquoted_मूल्य.take_front(1) == "\\");
    जर (Unquoted_मूल्य.size() == 1) {
      Token T;
      T.Range = Unquoted_मूल्य;
      setError("Unrecognized escape code", T);
      Storage.clear();
      परत StringRef();
    }
    Unquoted_मूल्य = Unquoted_मूल्य.drop_front(1);
    निवड (Unquoted_मूल्य[0]) {
    पूर्वनिर्धारित: {
      Token T;
      T.Range = Unquoted_मूल्य.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      परत StringRef();
    }
    स्थिती '\r':
      // Shrink the Windows-style EOL.
      जर (Unquoted_मूल्य.size() >= 2 && Unquoted_मूल्य[1] == '\n')
        Unquoted_मूल्य = Unquoted_मूल्य.drop_front(1);
      [[fallthrough]];
    स्थिती '\n':
      परत Unquoted_मूल्य.drop_front(1).ltrim(" \t");
    स्थिती '0':
      Storage.ढकला_back(0x00);
      थांबा;
    स्थिती 'a':
      Storage.ढकला_back(0x07);
      थांबा;
    स्थिती 'b':
      Storage.ढकला_back(0x08);
      थांबा;
    स्थिती 't':
    स्थिती 0x09:
      Storage.ढकला_back(0x09);
      थांबा;
    स्थिती 'n':
      Storage.ढकला_back(0x0A);
      थांबा;
    स्थिती 'v':
      Storage.ढकला_back(0x0B);
      थांबा;
    स्थिती 'f':
      Storage.ढकला_back(0x0C);
      थांबा;
    स्थिती 'r':
      Storage.ढकला_back(0x0D);
      थांबा;
    स्थिती 'e':
      Storage.ढकला_back(0x1B);
      थांबा;
    स्थिती ' ':
      Storage.ढकला_back(0x20);
      थांबा;
    स्थिती '"':
      Storage.ढकला_back(0x22);
      थांबा;
    स्थिती '/':
      Storage.ढकला_back(0x2F);
      थांबा;
    स्थिती '\\':
      Storage.ढकला_back(0x5C);
      थांबा;
    स्थिती 'N':
      encodeUTF8(0x85, Storage);
      थांबा;
    स्थिती '_':
      encodeUTF8(0xA0, Storage);
      थांबा;
    स्थिती 'L':
      encodeUTF8(0x2028, Storage);
      थांबा;
    स्थिती 'P':
      encodeUTF8(0x2029, Storage);
      थांबा;
    स्थिती 'x': {
      जर (Unquoted_मूल्य.size() < 3)
        // TODO: Report error.
        थांबा;
      अचिन्हित पूर्णांक Unicode_Scalar_मूल्य;
      जर (Unquoted_मूल्य.substr(1, 2).getAsInteger(16, Unicode_Scalar_मूल्य))
        // TODO: Report error.
        Unicode_Scalar_मूल्य = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मूल्य, Storage);
      परत Unquoted_मूल्य.drop_front(3);
    }
    स्थिती 'u': {
      जर (Unquoted_मूल्य.size() < 5)
        // TODO: Report error.
        थांबा;
      अचिन्हित पूर्णांक Unicode_Scalar_मूल्य;
      जर (Unquoted_मूल्य.substr(1, 4).getAsInteger(16, Unicode_Scalar_मूल्य))
        // TODO: Report error.
        Unicode_Scalar_मूल्य = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मूल्य, Storage);
      परत Unquoted_मूल्य.drop_front(5);
    }
    स्थिती 'U': {
      जर (Unquoted_मूल्य.size() < 9)
        // TODO: Report error.
        थांबा;
      अचिन्हित पूर्णांक Unicode_Scalar_मूल्य;
      जर (Unquoted_मूल्य.substr(1, 8).getAsInteger(16, Unicode_Scalar_मूल्य))
        // TODO: Report error.
        Unicode_Scalar_मूल्य = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मूल्य, Storage);
      परत Unquoted_मूल्य.drop_front(9);
    }
    }
    परत Unquoted_मूल्य.drop_front(1);
  };

  परत parse_Scalar_मूल्य(Unquoted_मूल्य, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_गाठ::get_Single_Quoted_मूल्य(StringRef Raw_मूल्य,
                                           SmallVectorImpl<अक्षर> &Storage) {
  प्रतिज्ञा(Raw_मूल्य.size() >= 2 && Raw_मूल्य.front() == '\'' &&
         Raw_मूल्य.back() == '\'');
  StringRef Unquoted_मूल्य = Raw_मूल्य.substr(1, Raw_मूल्य.size() - 2);

  स्वतः UnescapeFunc = [](StringRef Unquoted_मूल्य,
                         SmallVectorImpl<अक्षर> &Storage) {
    प्रतिज्ञा(Unquoted_मूल्य.take_front(2) == "''");
    Storage.ढकला_back('\'');
    परत Unquoted_मूल्य.drop_front(2);
  };

  परत parse_Scalar_मूल्य(Unquoted_मूल्य, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_गाठ::get_Plain_मूल्य(StringRef Raw_मूल्य,
                                    SmallVectorImpl<अक्षर> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_मूल्य = Raw_मूल्य.rtrim("\r\n \t");
  परत parse_Scalar_मूल्य(Raw_मूल्य, Storage, "\r\n", nullptr);
}

गाठ *किल्ली_मूल्य_गाठ::get_किल्ली() {
  जर (किल्ली)
    परत किल्ली;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    जर (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_मूल्य
        || t.Kind == Token::TK_Error) {
      परत किल्ली = नवीन (getAllocator()) Null_गाठ(Doc);
    }
    जर (t.Kind == Token::TK_किल्ली)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  जर (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_मूल्य) {
    परत किल्ली = नवीन (getAllocator()) Null_गाठ(Doc);
  }

  // We've got a normal key.
  परत किल्ली = parse_Block_गाठ();
}

गाठ *किल्ली_मूल्य_गाठ::get_मूल्य() {
  जर (मूल्य)
    परत मूल्य;

  जर (गाठ* किल्ली = get_किल्ली())
    किल्ली->skip();
  अन्यथा {
    setError("Null key in Key Value.", peekNext());
    परत मूल्य = नवीन (getAllocator()) Null_गाठ(Doc);
  }

  जर (failed())
    परत मूल्य = नवीन (getAllocator()) Null_गाठ(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    जर (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_किल्ली
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      परत मूल्य = नवीन (getAllocator()) Null_गाठ(Doc);
    }

    जर (t.Kind != Token::TK_मूल्य) {
      setError("Unexpected token in Key Value.", t);
      परत मूल्य = नवीन (getAllocator()) Null_गाठ(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  जर (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_किल्ली) {
    परत मूल्य = नवीन (getAllocator()) Null_गाठ(Doc);
  }

  // We got a normal value.
  परत मूल्य = parse_Block_गाठ();
}

शून्य Mapping_गाठ::increment() {
  जर (failed()) {
    IsAtEnd = सत्य;
    CurrentEntry = nullptr;
    परत;
  }
  जर (CurrentEntry) {
    CurrentEntry->skip();
    जर (Type == MT_Inline) {
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      परत;
    }
  }
  Token T = peekNext();
  जर (T.Kind == Token::TK_किल्ली || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = नवीन (getAllocator()) किल्ली_मूल्य_गाठ(Doc);
  } अन्यथा जर (Type == MT_Block) {
    निवड (T.Kind) {
    स्थिती Token::TK_BlockEnd:
      getNext();
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      थांबा;
    पूर्वनिर्धारित:
      setError("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    स्थिती Token::TK_Error:
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
    }
  } अन्यथा {
    निवड (T.Kind) {
    स्थिती Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      परत increment();
    स्थिती Token::TK_FlowMappingEnd:
      getNext();
      [[fallthrough]];
    स्थिती Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      थांबा;
    पूर्वनिर्धारित:
      setError( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
    }
  }
}

शून्य Sequence_गाठ::increment() {
  जर (failed()) {
    IsAtEnd = सत्य;
    CurrentEntry = nullptr;
    परत;
  }
  जर (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  जर (SeqType == ST_Block) {
    निवड (T.Kind) {
    स्थिती Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_गाठ();
      जर (!CurrentEntry) { // An error occurred.
        IsAtEnd = सत्य;
        CurrentEntry = nullptr;
      }
      थांबा;
    स्थिती Token::TK_BlockEnd:
      getNext();
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      थांबा;
    पूर्वनिर्धारित:
      setError( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    स्थिती Token::TK_Error:
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
    }
  } अन्यथा जर (SeqType == ST_Indentless) {
    निवड (T.Kind) {
    स्थिती Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_गाठ();
      जर (!CurrentEntry) { // An error occurred.
        IsAtEnd = सत्य;
        CurrentEntry = nullptr;
      }
      थांबा;
    पूर्वनिर्धारित:
    स्थिती Token::TK_Error:
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
    }
  } अन्यथा जर (SeqType == ST_Flow) {
    निवड (T.Kind) {
    स्थिती Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      WasPreviousTokenFlowEntry = सत्य;
      परत increment();
    स्थिती Token::TK_FlowSequenceEnd:
      getNext();
      [[fallthrough]];
    स्थिती Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      थांबा;
    स्थिती Token::TK_StreamEnd:
    स्थिती Token::TK_DocumentEnd:
    स्थिती Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = सत्य;
      CurrentEntry = nullptr;
      थांबा;
    पूर्वनिर्धारित:
      जर (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = सत्य;
        CurrentEntry = nullptr;
        थांबा;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parse_Block_गाठ();
      जर (!CurrentEntry) {
        IsAtEnd = सत्य;
      }
      WasPreviousTokenFlowEntry = असत्य;
      थांबा;
    }
  }
}

Document::Document(Stream &S) : stream(S), मूळ_2(nullptr) {
  // Tag maps starts with two default mappings.
  TagMap["!"] = "!";
  TagMap["!!"] = "tag:yaml.org,2002:";

  जर (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peekNext();
  जर (T.Kind == Token::TK_DocumentStart)
    getNext();
}

तार्किक Document::skip()  {
  जर (stream.scanner->failed())
    परत असत्य;
  जर (!मूळ_2 && !get_मूळ())
    परत असत्य;
  मूळ_2->skip();
  Token &T = peekNext();
  जर (T.Kind == Token::TK_StreamEnd)
    परत असत्य;
  जर (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    परत skip();
  }
  परत सत्य;
}

Token &Document::peekNext() {
  परत stream.scanner->peekNext();
}

Token Document::getNext() {
  परत stream.scanner->getNext();
}

शून्य Document::setError(स्थिरांक Twine &Message, Token &Location) स्थिरांक {
  stream.scanner->setError(Message, Location.Range.begin());
}

तार्किक Document::failed() स्थिरांक {
  परत stream.scanner->failed();
}

गाठ *Document::parse_Block_गाठ() {
  Token T = peekNext();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  निवड (T.Kind) {
  स्थिती Token::TK_Alias:
    getNext();
    परत नवीन (गाठ_Allocator) Alias_गाठ(stream.CurrentDoc, T.Range.substr(1));
  स्थिती Token::TK_Anchor:
    जर (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      परत nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    जा parse_property;
  स्थिती Token::TK_Tag:
    जर (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      परत nullptr;
    }
    TagInfo = getNext(); // Consume TK_Tag.
    T = peekNext();
    जा parse_property;
  पूर्वनिर्धारित:
    थांबा;
  }

  निवड (T.Kind) {
  स्थिती Token::TK_BlockEntry:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    परत नवीन (गाठ_Allocator) Sequence_गाठ( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , Sequence_गाठ::ST_Indentless);
  स्थिती Token::TK_BlockSequenceStart:
    getNext();
    परत नवीन (गाठ_Allocator)
      Sequence_गाठ( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_गाठ::ST_Block);
  स्थिती Token::TK_BlockMappingStart:
    getNext();
    परत नवीन (गाठ_Allocator)
      Mapping_गाठ( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_गाठ::MT_Block);
  स्थिती Token::TK_FlowSequenceStart:
    getNext();
    परत नवीन (गाठ_Allocator)
      Sequence_गाठ( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_गाठ::ST_Flow);
  स्थिती Token::TK_FlowMappingStart:
    getNext();
    परत नवीन (गाठ_Allocator)
      Mapping_गाठ( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_गाठ::MT_Flow);
  स्थिती Token::TK_Scalar:
    getNext();
    परत नवीन (गाठ_Allocator)
      Scalar_गाठ( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  स्थिती Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.मूल्य.c_str(), T.मूल्य.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(गाठ_Allocator).drop_back();
    परत नवीन (गाठ_Allocator)
        Block_Scalar_गाठ(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  स्थिती Token::TK_किल्ली:
    // Don't eat the TK_Key, KeyValueNode expects it.
    परत नवीन (गाठ_Allocator)
      Mapping_गाठ( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_गाठ::MT_Inline);
  स्थिती Token::TK_DocumentStart:
  स्थिती Token::TK_DocumentEnd:
  स्थिती Token::TK_StreamEnd:
  पूर्वनिर्धारित:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    परत नवीन (गाठ_Allocator) Null_गाठ(stream.CurrentDoc);
  स्थिती Token::TK_FlowMappingEnd:
  स्थिती Token::TK_FlowSequenceEnd:
  स्थिती Token::TK_FlowEntry: {
    जर (मूळ_2 && (isa<Mapping_गाठ>(मूळ_2) || isa<Sequence_गाठ>(मूळ_2)))
      परत नवीन (गाठ_Allocator) Null_गाठ(stream.CurrentDoc);

    setError("Unexpected token", T);
    परत nullptr;
  }
  स्थिती Token::TK_Error:
    परत nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  परत nullptr;
}

तार्किक Document::parseDirectives() {
  तार्किक isDirective = असत्य;
  पर्यंत (सत्य) {
    Token T = peekNext();
    जर (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = सत्य;
    } अन्यथा जर (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = सत्य;
    } अन्यथा {
      थांबा;
    }
  }
  परत isDirective;
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

तार्किक Document::expectToken(पूर्णांक TK) {
  Token T = getNext();
  जर (T.Kind != TK) {
    setError("Unexpected token", T);
    परत असत्य;
  }
  परत सत्य;
}
