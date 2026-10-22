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

#અન્તર્ભાવ "llvm/Support/YAMLParser.h"
#અન્તર્ભાવ "llvm/ADT/AllocatorList.h"
#અન્તર્ભાવ "llvm/ADT/ArrayRef.h"
#અન્તર્ભાવ "llvm/ADT/STLExtras.h"
#અન્તર્ભાવ "llvm/ADT/SmallString.h"
#અન્તર્ભાવ "llvm/ADT/SmallVector.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/ADT/Twine.h"
#અન્તર્ભાવ "llvm/Support/Compiler.h"
#અન્તર્ભાવ "llvm/Support/ErrorHandling.h"
#અન્તર્ભાવ "llvm/Support/MemoryBuffer.h"
#અન્તર્ભાવ "llvm/Support/SMLoc.h"
#અન્તર્ભાવ "llvm/Support/SourceMgr.h"
#અન્તર્ભાવ "llvm/Support/Unicode.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <cassert>
#અન્તર્ભાવ <cstddef>
#અન્તર્ભાવ <cstdint>
#અન્તર્ભાવ <map>
#અન્તર્ભાવ <memory>
#અન્તર્ભાવ <string>
#અન્તર્ભાવ <system_error>
#અન્તર્ભાવ <utility>

using નામાકાશ llvm;
using નામાકાશ yaml;

ગણના UnicodeEncodingForm {
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
સ્થિર EncodingInfo getUnicodeEncoding(StringRef આગત) {
  જો (આગત.empty())
    પાછા {UEF_Unknown, 0};

  પસંદગી (uint8_t(આગત[0])) {
  સ્થિતિ 0x00:
    જો (આગત.size() >= 4) {
      જો (  આગત[1] == 0
         && uint8_t(આગત[2]) == 0xFE
         && uint8_t(આગત[3]) == 0xFF)
        પાછા {UEF_UTF32_BE, 4};
      જો (આગત[1] == 0 && આગત[2] == 0 && આગત[3] != 0)
        પાછા {UEF_UTF32_BE, 0};
    }

    જો (આગત.size() >= 2 && આગત[1] != 0)
      પાછા {UEF_UTF16_BE, 0};
    પાછા {UEF_Unknown, 0};
  સ્થિતિ 0xFF:
    જો (  આગત.size() >= 4
       && uint8_t(આગત[1]) == 0xFE
       && આગત[2] == 0
       && આગત[3] == 0)
      પાછા {UEF_UTF32_LE, 4};

    જો (આગત.size() >= 2 && uint8_t(આગત[1]) == 0xFE)
      પાછા {UEF_UTF16_LE, 2};
    પાછા {UEF_Unknown, 0};
  સ્થિતિ 0xFE:
    જો (આગત.size() >= 2 && uint8_t(આગત[1]) == 0xFF)
      પાછા {UEF_UTF16_BE, 2};
    પાછા {UEF_Unknown, 0};
  સ્થિતિ 0xEF:
    જો (  આગત.size() >= 3
       && uint8_t(આગત[1]) == 0xBB
       && uint8_t(આગત[2]) == 0xBF)
      પાછા {UEF_UTF8, 3};
    પાછા {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  જો (આગત.size() >= 4 && આગત[1] == 0 && આગત[2] == 0 && આગત[3] == 0)
    પાછા {UEF_UTF32_LE, 0};

  જો (આગત.size() >= 2 && આગત[1] == 0)
    પાછા {UEF_UTF16_LE, 0};

  પાછા {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
શૂન્ય ગાંઠ::anchor() {}
શૂન્ય Null_ગાંઠ::anchor() {}
શૂન્ય Scalar_ગાંઠ::anchor() {}
શૂન્ય Block_Scalar_ગાંઠ::anchor() {}
શૂન્ય ચાવી_કિંમત_ગાંઠ::anchor() {}
શૂન્ય Mapping_ગાંઠ::anchor() {}
શૂન્ય Sequence_ગાંઠ::anchor() {}
શૂન્ય Alias_ગાંઠ::anchor() {}

નામાકાશ llvm {
નામાકાશ yaml {

/// Token - A single YAML token.
સંરચના Token {
  ગણના TokenKind {
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
    TK_ચાવી,
    TK_કિંમત,
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
  std::string કિંમત;

  Token() = મૂળભૂત;
};

} // end namespace yaml
} // end namespace llvm

using TokenQueueT = BumpPtrList<Token>;

નામાકાશ {

/// This struct is used to track simple keys.
///
/// Simple keys are handled by creating an entry in SimpleKeys for each Token
/// which could legally be the start of a simple key. When peekNext is called,
/// if the Token To be returned is referenced by a SimpleKey, we continue
/// tokenizing until that potential simple key has either been found to not be
/// a simple key (we moved on to the next line or went further than 1024 chars).
/// Or when we run into a Value, and then insert a Key token (and possibly
/// others) before the SimpleKey's Tok.
સંરચના Simple_ચાવી {
  TokenQueueT::iterator Tok;
  unsigned Column = 0;
  unsigned લીટી = 0;
  unsigned FlowLevel = 0;
  તાર્કિક IsRequired = ખોટું;

  તાર્કિક operator ==(અચલ Simple_ચાવી &Other) {
    પાછા Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
using UTF8Decoded = std::pair<uint32_t, unsigned>;

સ્થિર UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator End = Range.end();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  જો (Position < End && (*Position & 0x80) == 0) {
    પાછા {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  જો (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    જો (codepoint >= 0x80)
      પાછા {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  જો (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    જો (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      પાછા {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  જો (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    જો (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      પાછા {codepoint, 4};
  }
  પાછા {0, 0};
}

નામાકાશ llvm {
નામાકાશ yaml {

/// Scans YAML tokens from a MemoryBuffer.
વર્ગ Scanner {
જાહેર:
  Scanner(StringRef આગત, SourceMgr &SM, તાર્કિક ShowColors = સાચું,
          std::error_code *EC = nullptr);
  Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, તાર્કિક ShowColors = સાચું,
          std::error_code *EC = nullptr);

  /// Parse the next token and return it without popping it.
  Token &peekNext();

  /// Parse the next token and pop it from the queue.
  Token getNext();

  શૂન્ય printError(SMLoc Loc, SourceMgr::DiagKind Kind, અચલ Twine &Message,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.PrintMessage(Loc, Kind, Message, Ranges, /* FixIts= */ {}, ShowColors);
  }

  શૂન્ય setError(અચલ Twine &Message, StringRef::iterator Position) {
    જો (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    જો (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    જો (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = સાચું;
  }

  /// Returns true if an error occurred while parsing.
  તાર્કિક failed() {
    પાછા Failed;
  }

ખાનગી:
  શૂન્ય init(MemoryBufferRef Buffer);

  StringRef current_આગત() {
    પાછા StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    પાછા ::decodeUTF8(StringRef(Position, End - Position));
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
  શૂન્ય advanceWhile(SkipWhileFunc Func);

  /// Scan ns-uri-char[39]s starting at Cur.
  ///
  /// This updates Cur and Column while scanning.
  શૂન્ય scan_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  તાર્કિક consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  શૂન્ય skip(uint32_t Distance);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  તાર્કિક isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  તાર્કિક isPlainSafeNonBlank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  તાર્કિક is_લીટી_Empty(StringRef લીટી);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  તાર્કિક consume_લીટી_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  શૂન્ય save_Simple_ચાવી_Candidate( TokenQueueT::iterator Tok
                             , unsigned AtColumn
                             , તાર્કિક IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  શૂન્ય remove_Stale_Simple_ચાવી_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  શૂન્ય remove_Simple_ચાવી_Candidates_On_Flow_Level(unsigned Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  તાર્કિક unrollIndent(પૂર્ણાંક ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  તાર્કિક rollIndent( પૂર્ણાંક ToColumn
                 , Token::TokenKind Kind
                 , TokenQueueT::iterator ઉમેરો_Point);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  શૂન્ય skipComment();

  /// Skip whitespace and comments until the start of the next token.
  શૂન્ય scanToNextToken();

  /// Must be the first token generated.
  તાર્કિક scanStreamStart();

  /// Generate tokens needed to close out the stream.
  તાર્કિક scanStreamEnd();

  /// Scan a %BLAH directive.
  તાર્કિક scanDirective();

  /// Scan a ... or ---.
  તાર્કિક scanDocumentIndicator(તાર્કિક IsStart);

  /// Scan a [ or { and generate the proper flow collection start token.
  તાર્કિક scanFlowCollectionStart(તાર્કિક IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  તાર્કિક scanFlowCollectionEnd(તાર્કિક IsSequence);

  /// Scan the , that separates entries in a flow collection.
  તાર્કિક scanFlowEntry();

  /// Scan the - that starts block sequence entries.
  તાર્કિક scanBlockEntry();

  /// Scan an explicit ? indicating a key.
  તાર્કિક scan_ચાવી();

  /// Scan an explicit : indicating a value.
  તાર્કિક scan_કિંમત();

  /// Scan a quoted scalar.
  તાર્કિક scanFlowScalar(તાર્કિક IsDoubleQuoted);

  /// Scan an unquoted scalar.
  તાર્કિક scanPlainScalar();

  /// Scan an Alias or Anchor starting with * or &.
  તાર્કિક scanAliasOrAnchor(તાર્કિક IsAlias);

  /// Scan a block scalar starting with | or >.
  તાર્કિક scanBlockScalar(તાર્કિક IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  તાર્કિક scanBlockScalarIndicators(અક્ષર &StyleIndicator, અક્ષર &ChompingIndicator,
                                 unsigned &IndentIndicator, તાર્કિક &IsDone);

  /// Scan a style indicator in a block scalar header.
  અક્ષર scanBlockStyleIndicator();

  /// Scan a chomping indicator in a block scalar header.
  અક્ષર scanBlockChompingIndicator();

  /// Scan an indentation indicator in a block scalar header.
  unsigned scanBlockIndentationIndicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  તાર્કિક scanBlockScalarHeader(અક્ષર &ChompingIndicator, unsigned &IndentIndicator,
                             તાર્કિક &IsDone);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  તાર્કિક findBlockScalarIndent(unsigned &BlockIndent, unsigned BlockExitIndent,
                             unsigned &લીટી_Breaks, તાર્કિક &IsDone);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  તાર્કિક scanBlockScalarIndent(unsigned BlockIndent, unsigned BlockExitIndent,
                             તાર્કિક &IsDone);

  /// Scan a tag of the form !stuff.
  તાર્કિક scanTag();

  /// Dispatch to the next scanning function based on \a *Cur.
  તાર્કિક fetchMoreTokens();

  /// The SourceMgr used for diagnostics and buffer management.
  SourceMgr &SM;

  /// The original input.
  MemoryBufferRef આગત_Buffer;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator End;

  /// Current YAML indentation level in spaces.
  પૂર્ણાંક Indent;

  /// Current column number in Unicode code points.
  unsigned Column;

  /// Current line number.
  unsigned લીટી;

  /// How deep we are in flow style containers. 0 Means at block level.
  unsigned FlowLevel;

  /// Are we at the start of the stream?
  તાર્કિક IsStartOfStream;

  /// Can the next token be the start of a simple key?
  તાર્કિક Is_Simple_ચાવી_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  તાર્કિક Is_Adjacent_કિંમત_Allowed_In_Flow;

  /// True if an error has occurred.
  તાર્કિક Failed;

  /// Should colors be used when printing out the diagnostic messages?
  તાર્કિક ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  TokenQueueT TokenQueue;

  /// Indentation levels.
  SmallVector<પૂર્ણાંક, 4> Indents;

  /// Potential simple keys.
  SmallVector<Simple_ચાવી, 4> Simple_ચાવીઓ;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
સ્થિર શૂન્ય encodeUTF8( uint32_t Unicode_Scalar_કિંમત
                      , SmallVectorImpl<અક્ષર> &Result) {
  જો (Unicode_Scalar_કિંમત <= 0x7F) {
    Result.ધકેલો_back(Unicode_Scalar_કિંમત & 0x7F);
  } નહીંતર જો (Unicode_Scalar_કિંમત <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((Unicode_Scalar_કિંમત & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Unicode_Scalar_કિંમત & 0x3F);
    Result.ધકેલો_back(FirstByte);
    Result.ધકેલો_back(SecondByte);
  } નહીંતર જો (Unicode_Scalar_કિંમત <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((Unicode_Scalar_કિંમત & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_કિંમત & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Unicode_Scalar_કિંમત & 0x3F);
    Result.ધકેલો_back(FirstByte);
    Result.ધકેલો_back(SecondByte);
    Result.ધકેલો_back(ThirdByte);
  } નહીંતર જો (Unicode_Scalar_કિંમત <= 0x10FFFF) {
    uint8_t FirstByte = 0xF0 | ((Unicode_Scalar_કિંમત & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_કિંમત & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Unicode_Scalar_કિંમત & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_કિંમત & 0x3F);
    Result.ધકેલો_back(FirstByte);
    Result.ધકેલો_back(SecondByte);
    Result.ધકેલો_back(ThirdByte);
    Result.ધકેલો_back(FourthByte);
  }
}

તાર્કિક yaml::dumpTokens(StringRef આગત, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(આગત, SM);
  જ્યારે (સાચું) {
    Token T = scanner.getNext();
    પસંદગી (T.Kind) {
    સ્થિતિ Token::TK_StreamStart:
      OS << "Stream-Start: ";
      તોડો;
    સ્થિતિ Token::TK_StreamEnd:
      OS << "Stream-End: ";
      તોડો;
    સ્થિતિ Token::TK_VersionDirective:
      OS << "Version-Directive: ";
      તોડો;
    સ્થિતિ Token::TK_TagDirective:
      OS << "Tag-Directive: ";
      તોડો;
    સ્થિતિ Token::TK_DocumentStart:
      OS << "Document-Start: ";
      તોડો;
    સ્થિતિ Token::TK_DocumentEnd:
      OS << "Document-End: ";
      તોડો;
    સ્થિતિ Token::TK_BlockEntry:
      OS << "Block-Entry: ";
      તોડો;
    સ્થિતિ Token::TK_BlockEnd:
      OS << "Block-End: ";
      તોડો;
    સ્થિતિ Token::TK_BlockSequenceStart:
      OS << "Block-Sequence-Start: ";
      તોડો;
    સ્થિતિ Token::TK_BlockMappingStart:
      OS << "Block-Mapping-Start: ";
      તોડો;
    સ્થિતિ Token::TK_FlowEntry:
      OS << "Flow-Entry: ";
      તોડો;
    સ્થિતિ Token::TK_FlowSequenceStart:
      OS << "Flow-Sequence-Start: ";
      તોડો;
    સ્થિતિ Token::TK_FlowSequenceEnd:
      OS << "Flow-Sequence-End: ";
      તોડો;
    સ્થિતિ Token::TK_FlowMappingStart:
      OS << "Flow-Mapping-Start: ";
      તોડો;
    સ્થિતિ Token::TK_FlowMappingEnd:
      OS << "Flow-Mapping-End: ";
      તોડો;
    સ્થિતિ Token::TK_ચાવી:
      OS << "Key: ";
      તોડો;
    સ્થિતિ Token::TK_કિંમત:
      OS << "Value: ";
      તોડો;
    સ્થિતિ Token::TK_Scalar:
      OS << "Scalar: ";
      તોડો;
    સ્થિતિ Token::TK_BlockScalar:
      OS << "Block Scalar: ";
      તોડો;
    સ્થિતિ Token::TK_Alias:
      OS << "Alias: ";
      તોડો;
    સ્થિતિ Token::TK_Anchor:
      OS << "Anchor: ";
      તોડો;
    સ્થિતિ Token::TK_Tag:
      OS << "Tag: ";
      તોડો;
    સ્થિતિ Token::TK_Error:
      તોડો;
    }
    OS << T.Range << "\n";
    જો (T.Kind == Token::TK_StreamEnd)
      તોડો;
    નહીંતર જો (T.Kind == Token::TK_Error)
      પાછા ખોટું;
  }
  પાછા સાચું;
}

તાર્કિક yaml::scanTokens(StringRef આગત) {
  SourceMgr SM;
  Scanner scanner(આગત, SM);
  જ્યારે (સાચું) {
    Token T = scanner.getNext();
    જો (T.Kind == Token::TK_StreamEnd)
      તોડો;
    નહીંતર જો (T.Kind == Token::TK_Error)
      પાછા ખોટું;
  }
  પાછા સાચું;
}

std::string yaml::escape(StringRef આગત, તાર્કિક EscapePrintable) {
  std::string Escaped_આગત;
  માટે (StringRef::iterator i = આગત.begin(), e = આગત.end(); i != e; ++i) {
    જો (*i == '\\')
      Escaped_આગત += "\\\\";
    નહીંતર જો (*i == '"')
      Escaped_આગત += "\\\"";
    નહીંતર જો (*i == 0)
      Escaped_આગત += "\\0";
    નહીંતર જો (*i == 0x07)
      Escaped_આગત += "\\a";
    નહીંતર જો (*i == 0x08)
      Escaped_આગત += "\\b";
    નહીંતર જો (*i == 0x09)
      Escaped_આગત += "\\t";
    નહીંતર જો (*i == 0x0A)
      Escaped_આગત += "\\n";
    નહીંતર જો (*i == 0x0B)
      Escaped_આગત += "\\v";
    નહીંતર જો (*i == 0x0C)
      Escaped_આગત += "\\f";
    નહીંતર જો (*i == 0x0D)
      Escaped_આગત += "\\r";
    નહીંતર જો (*i == 0x1B)
      Escaped_આગત += "\\e";
    નહીંતર જો ((unsigned અક્ષર)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_આગત += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } નહીંતર જો (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_કિંમત
        = decodeUTF8(StringRef(i, આગત.end() - i));
      જો (Unicode_Scalar_કિંમત.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_આગત, Val);
        // FIXME: Error reporting.
        પાછા Escaped_આગત;
      }
      જો (Unicode_Scalar_કિંમત.first == 0x85)
        Escaped_આગત += "\\N";
      નહીંતર જો (Unicode_Scalar_કિંમત.first == 0xA0)
        Escaped_આગત += "\\_";
      નહીંતર જો (Unicode_Scalar_કિંમત.first == 0x2028)
        Escaped_આગત += "\\L";
      નહીંતર જો (Unicode_Scalar_કિંમત.first == 0x2029)
        Escaped_આગત += "\\P";
      નહીંતર જો (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_કિંમત.first))
        Escaped_આગત += StringRef(i, Unicode_Scalar_કિંમત.second);
      નહીંતર {
        std::string HexStr = utohexstr(Unicode_Scalar_કિંમત.first);
        જો (HexStr.size() <= 2)
          Escaped_આગત += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        નહીંતર જો (HexStr.size() <= 4)
          Escaped_આગત += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        નહીંતર જો (HexStr.size() <= 8)
          Escaped_આગત += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += Unicode_Scalar_કિંમત.second - 1;
    } નહીંતર {
      Escaped_આગત.ધકેલો_back(*i);
    }
  }
  પાછા Escaped_આગત;
}

std::optional<તાર્કિક> yaml::parseBool(StringRef S) {
  પસંદગી (S.size()) {
  સ્થિતિ 1:
    પસંદગી (S.front()) {
    સ્થિતિ 'y':
    સ્થિતિ 'Y':
      પાછા સાચું;
    સ્થિતિ 'n':
    સ્થિતિ 'N':
      પાછા ખોટું;
    મૂળભૂત:
      પાછા std::nullopt;
    }
  સ્થિતિ 2:
    પસંદગી (S.front()) {
    સ્થિતિ 'O':
      જો (S[1] == 'N') // ON
        પાછા સાચું;
      [[fallthrough]];
    સ્થિતિ 'o':
      જો (S[1] == 'n') //[Oo]n
        પાછા સાચું;
      પાછા std::nullopt;
    સ્થિતિ 'N':
      જો (S[1] == 'O') // NO
        પાછા ખોટું;
      [[fallthrough]];
    સ્થિતિ 'n':
      જો (S[1] == 'o') //[Nn]o
        પાછા ખોટું;
      પાછા std::nullopt;
    મૂળભૂત:
      પાછા std::nullopt;
    }
  સ્થિતિ 3:
    પસંદગી (S.front()) {
    સ્થિતિ 'O':
      જો (S.drop_front() == "FF") // OFF
        પાછા ખોટું;
      [[fallthrough]];
    સ્થિતિ 'o':
      જો (S.drop_front() == "ff") //[Oo]ff
        પાછા ખોટું;
      પાછા std::nullopt;
    સ્થિતિ 'Y':
      જો (S.drop_front() == "ES") // YES
        પાછા સાચું;
      [[fallthrough]];
    સ્થિતિ 'y':
      જો (S.drop_front() == "es") //[Yy]es
        પાછા સાચું;
      પાછા std::nullopt;
    મૂળભૂત:
      પાછા std::nullopt;
    }
  સ્થિતિ 4:
    પસંદગી (S.front()) {
    સ્થિતિ 'T':
      જો (S.drop_front() == "RUE") // TRUE
        પાછા સાચું;
      [[fallthrough]];
    સ્થિતિ 't':
      જો (S.drop_front() == "rue") //[Tt]rue
        પાછા સાચું;
      પાછા std::nullopt;
    મૂળભૂત:
      પાછા std::nullopt;
    }
  સ્થિતિ 5:
    પસંદગી (S.front()) {
    સ્થિતિ 'F':
      જો (S.drop_front() == "ALSE") // FALSE
        પાછા ખોટું;
      [[fallthrough]];
    સ્થિતિ 'f':
      જો (S.drop_front() == "alse") //[Ff]alse
        પાછા ખોટું;
      પાછા std::nullopt;
    મૂળભૂત:
      પાછા std::nullopt;
    }
  મૂળભૂત:
    પાછા std::nullopt;
  }
}

Scanner::Scanner(StringRef આગત, SourceMgr &sm, તાર્કિક ShowColors,
                 std::error_code *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(MemoryBufferRef(આગત, "YAML"));
}

Scanner::Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, તાર્કિક ShowColors,
                 std::error_code *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(Buffer);
}

શૂન્ય Scanner::init(MemoryBufferRef Buffer) {
  આગત_Buffer = Buffer;
  Current = આગત_Buffer.getBufferStart();
  End = આગત_Buffer.getBufferEnd();
  Indent = -1;
  Column = 0;
  લીટી = 0;
  FlowLevel = 0;
  IsStartOfStream = સાચું;
  Is_Simple_ચાવી_Allowed = સાચું;
  Is_Adjacent_કિંમત_Allowed_In_Flow = ખોટું;
  Failed = ખોટું;
  std::unique_ptr<MemoryBuffer> આગત_Buffer_Owner =
      MemoryBuffer::getMemBuffer(Buffer, /*RequiresNullTerminator=*/ખોટું);
  SM.AddNewSourceBuffer(std::move(આગત_Buffer_Owner), SMLoc());
}

Token &Scanner::peekNext() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  તાર્કિક NeedMore = ખોટું;
  જ્યારે (સાચું) {
    જો (TokenQueue.empty() || NeedMore) {
      જો (!fetchMoreTokens()) {
        TokenQueue.clear();
        Simple_ચાવીઓ.clear();
        TokenQueue.ધકેલો_back(Token());
        પાછા TokenQueue.front();
      }
    }
    પ્રતિજ્ઞા(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_ચાવી_Candidates();
    Simple_ચાવી SK;
    SK.Tok = TokenQueue.begin();
    જો (!is_contained(Simple_ચાવીઓ, SK))
      તોડો;
    નહીંતર
      NeedMore = સાચું;
  }
  પાછા TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  જો (!TokenQueue.empty())
    TokenQueue.કાઢો_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  જો (TokenQueue.empty())
    TokenQueue.resetAlloc();

  પાછા Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  જો (Position == End)
    પાછા Position;
  // Check 7 bit c-printable - b-char.
  જો (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    પાછા Position + 1;

  // Check for valid UTF-8.
  જો (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    જો (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      પાછા Position + u8d.second;
  }
  પાછા Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  જો (Position == End)
    પાછા Position;
  જો (*Position == 0x0D) {
    જો (Position + 1 != End && *(Position + 1) == 0x0A)
      પાછા Position + 2;
    પાછા Position + 1;
  }

  જો (*Position == 0x0A)
    પાછા Position + 1;
  પાછા Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  જો (Position == End)
    પાછા Position;
  જો (*Position == ' ')
    પાછા Position + 1;
  પાછા Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  જો (Position == End)
    પાછા Position;
  જો (*Position == ' ' || *Position == '\t')
    પાછા Position + 1;
  પાછા Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  જો (Position == End)
    પાછા Position;
  જો (*Position == ' ' || *Position == '\t')
    પાછા Position;
  પાછા skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  જ્યારે (સાચું) {
    StringRef::iterator i = (આ->*Func)(Position);
    જો (i == Position)
      તોડો;
    Position = i;
  }
  પાછા Position;
}

શૂન્ય Scanner::advanceWhile(SkipWhileFunc Func) {
  auto Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

સ્થિર તાર્કિક is_ns_hex_digit(અચલ અક્ષર C) { પાછા isAlnum(C); }

સ્થિર તાર્કિક is_ns_word_char(અચલ અક્ષર C) { પાછા C == '-' || isAlpha(C); }

શૂન્ય Scanner::scan_ns_uri_char() {
  જ્યારે (સાચું) {
    જો (Current == End)
      તોડો;
    જો ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } નહીંતર {
      તોડો;
    }
  }
}

તાર્કિક Scanner::consume(uint32_t Expected) {
  જો (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    પાછા ખોટું;
  }
  જો (Current == End)
    પાછા ખોટું;
  જો (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    પાછા ખોટું;
  }
  જો (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    પાછા સાચું;
  }
  પાછા ખોટું;
}

શૂન્ય Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  પ્રતિજ્ઞા(Current <= End && "Skipped past the end");
}

તાર્કિક Scanner::isBlankOrBreak(StringRef::iterator Position) {
  જો (Position == End)
    પાછા ખોટું;
  પાછા *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

તાર્કિક Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  જો (Position == End || isBlankOrBreak(Position))
    પાછા ખોટું;
  જો (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    પાછા ખોટું;
  પાછા સાચું;
}

તાર્કિક Scanner::is_લીટી_Empty(StringRef લીટી) {
  માટે (અચલ auto *Position = લીટી.begin(); Position != લીટી.end(); ++Position)
    જો (!isBlankOrBreak(Position))
      પાછા ખોટું;
  પાછા સાચું;
}

તાર્કિક Scanner::consume_લીટી_Break_If_Present() {
  auto Next = skip_b_break(Current);
  જો (Next == Current)
    પાછા ખોટું;
  Column = 0;
  ++લીટી;
  Current = Next;
  પાછા સાચું;
}

શૂન્ય Scanner::save_Simple_ચાવી_Candidate( TokenQueueT::iterator Tok
                                    , unsigned AtColumn
                                    , તાર્કિક IsRequired) {
  જો (Is_Simple_ચાવી_Allowed) {
    Simple_ચાવી SK;
    SK.Tok = Tok;
    SK.લીટી = લીટી;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_ચાવીઓ.ધકેલો_back(SK);
  }
}

શૂન્ય Scanner::remove_Stale_Simple_ચાવી_Candidates() {
  માટે (SmallVectorImpl<Simple_ચાવી>::iterator i = Simple_ચાવીઓ.begin();
                                            i != Simple_ચાવીઓ.end();) {
    જો (i->લીટી != લીટી || i->Column + 1024 < Column) {
      જો (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_ચાવીઓ.erase(i);
    } નહીંતર {
      ++i;
    }
  }
}

શૂન્ય Scanner::remove_Simple_ચાવી_Candidates_On_Flow_Level(unsigned Level) {
  જો (!Simple_ચાવીઓ.empty() && (Simple_ચાવીઓ.end() - 1)->FlowLevel == Level)
    Simple_ચાવીઓ.કાઢો_back();
}

તાર્કિક Scanner::unrollIndent(પૂર્ણાંક ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  જો (FlowLevel != 0)
    પાછા સાચું;

  જ્યારે (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.ધકેલો_back(T);
    Indent = Indents.કાઢો_back_val();
  }

  પાછા સાચું;
}

તાર્કિક Scanner::rollIndent( પૂર્ણાંક ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator ઉમેરો_Point) {
  જો (FlowLevel)
    પાછા સાચું;
  જો (Indent < ToColumn) {
    Indents.ધકેલો_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.ઉમેરો(ઉમેરો_Point, T);
  }
  પાછા સાચું;
}

શૂન્ય Scanner::skipComment() {
  જો (Current == End || *Current != '#')
    પાછા;
  જ્યારે (સાચું) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    જો (I == Current)
      તોડો;
    Current = I;
    ++Column;
  }
}

શૂન્ય Scanner::scanToNextToken() {
  જ્યારે (સાચું) {
    જ્યારે (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    જો (i == Current)
      તોડો;
    Current = i;
    ++લીટી;
    Column = 0;
    // New lines may start a simple key.
    જો (!FlowLevel)
      Is_Simple_ચાવી_Allowed = સાચું;
  }
}

તાર્કિક Scanner::scanStreamStart() {
  IsStartOfStream = ખોટું;

  EncodingInfo EI = getUnicodeEncoding(current_આગત());

  Token T;
  T.Kind = Token::TK_StreamStart;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.ધકેલો_back(T);
  Current += EI.second;
  પાછા સાચું;
}

તાર્કિક Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  જો (Column != 0) {
    Column = 0;
    ++લીટી;
  }

  unrollIndent(-1);
  Simple_ચાવીઓ.clear();
  Is_Simple_ચાવી_Allowed = ખોટું;
  Is_Adjacent_કિંમત_Allowed_In_Flow = ખોટું;

  Token T;
  T.Kind = Token::TK_StreamEnd;
  T.Range = StringRef(Current, 0);
  TokenQueue.ધકેલો_back(T);
  પાછા સાચું;
}

તાર્કિક Scanner::scanDirective() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_ચાવીઓ.clear();
  Is_Simple_ચાવી_Allowed = ખોટું;
  Is_Adjacent_કિંમત_Allowed_In_Flow = ખોટું;

  StringRef::iterator Start = Current;
  consume('%');
  StringRef::iterator NameStart = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(NameStart, Current - NameStart);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  જો (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ધકેલો_back(T);
    પાછા સાચું;
  } નહીંતર જો(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ધકેલો_back(T);
    પાછા સાચું;
  }
  પાછા ખોટું;
}

તાર્કિક Scanner::scanDocumentIndicator(તાર્કિક IsStart) {
  unrollIndent(-1);
  Simple_ચાવીઓ.clear();
  Is_Simple_ચાવી_Allowed = ખોટું;
  Is_Adjacent_કિંમત_Allowed_In_Flow = ખોટું;

  Token T;
  T.Kind = IsStart ? Token::TK_DocumentStart : Token::TK_DocumentEnd;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.ધકેલો_back(T);
  પાછા સાચું;
}

તાર્કિક Scanner::scanFlowCollectionStart(તાર્કિક IsSequence) {
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ધકેલો_back(T);

  // [ and { may begin a simple key.
  save_Simple_ચાવી_Candidate(--TokenQueue.end(), Column - 1, ખોટું);

  // And may also be followed by a simple key.
  Is_Simple_ચાવી_Allowed = સાચું;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_કિંમત_Allowed_In_Flow = ખોટું;
  ++FlowLevel;
  પાછા સાચું;
}

તાર્કિક Scanner::scanFlowCollectionEnd(તાર્કિક IsSequence) {
  remove_Simple_ચાવી_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ચાવી_Allowed = ખોટું;
  Is_Adjacent_કિંમત_Allowed_In_Flow = સાચું;
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceEnd
                      : Token::TK_FlowMappingEnd;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ધકેલો_back(T);
  જો (FlowLevel)
    --FlowLevel;
  પાછા સાચું;
}

તાર્કિક Scanner::scanFlowEntry() {
  remove_Simple_ચાવી_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ચાવી_Allowed = સાચું;
  Is_Adjacent_કિંમત_Allowed_In_Flow = ખોટું;
  Token T;
  T.Kind = Token::TK_FlowEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ધકેલો_back(T);
  પાછા સાચું;
}

તાર્કિક Scanner::scanBlockEntry() {
  rollIndent(Column, Token::TK_BlockSequenceStart, TokenQueue.end());
  remove_Simple_ચાવી_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ચાવી_Allowed = સાચું;
  Is_Adjacent_કિંમત_Allowed_In_Flow = ખોટું;
  Token T;
  T.Kind = Token::TK_BlockEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ધકેલો_back(T);
  પાછા સાચું;
}

તાર્કિક Scanner::scan_ચાવી() {
  જો (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  remove_Simple_ચાવી_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ચાવી_Allowed = !FlowLevel;
  Is_Adjacent_કિંમત_Allowed_In_Flow = ખોટું;

  Token T;
  T.Kind = Token::TK_ચાવી;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ધકેલો_back(T);
  પાછા સાચું;
}

તાર્કિક Scanner::scan_કિંમત() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  જો (!Simple_ચાવીઓ.empty()) {
    Simple_ચાવી SK = Simple_ચાવીઓ.કાઢો_back_val();
    Token T;
    T.Kind = Token::TK_ચાવી;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    માટે (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      જો (i == SK.Tok)
        તોડો;
    }
    જો (i == e) {
      Failed = સાચું;
      પાછા ખોટું;
    }
    i = TokenQueue.ઉમેરો(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    Is_Simple_ચાવી_Allowed = ખોટું;
  } નહીંતર {
    જો (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    Is_Simple_ચાવી_Allowed = !FlowLevel;
  }
  Is_Adjacent_કિંમત_Allowed_In_Flow = ખોટું;

  Token T;
  T.Kind = Token::TK_કિંમત;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ધકેલો_back(T);
  પાછા સાચું;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE સ્થિર તાર્કિક
wasEscaped(StringRef::iterator First, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
સ્થિર તાર્કિક wasEscaped(StringRef::iterator First,
                       StringRef::iterator Position) {
  પ્રતિજ્ઞા(Position - 1 >= First);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  જ્યારે (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  પાછા (Position - 1 - I) % 2 == 1;
}

તાર્કિક Scanner::scanFlowScalar(તાર્કિક IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  જો (IsDoubleQuoted) {
    કરો {
      ++Current;
      જ્યારે (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } જ્યારે (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } નહીંતર {
    skip(1);
    જ્યારે (Current != End) {
      // Skip a ' followed by another '.
      જો (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        ચાલુ;
      } નહીંતર જો (*Current == '\'')
        તોડો;
      StringRef::iterator i = skip_nb_char(Current);
      જો (i == Current) {
        i = skip_b_break(Current);
        જો (i == Current)
          તોડો;
        Current = i;
        Column = 0;
        ++લીટી;
      } નહીંતર {
        જો (i == End)
          તોડો;
        Current = i;
        ++Column;
      }
    }
  }

  જો (Current == End) {
    setError("Expected quote at end of scalar", Current);
    પાછા ખોટું;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ધકેલો_back(T);

  save_Simple_ચાવી_Candidate(--TokenQueue.end(), ColStart, ખોટું);

  Is_Simple_ચાવી_Allowed = ખોટું;
  Is_Adjacent_કિંમત_Allowed_In_Flow = સાચું;

  પાછા સાચું;
}

તાર્કિક Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  unsigned LeadingBlanks = 0;
  પ્રતિજ્ઞા(Indent >= -1 && "Indent must be >= -1 !");
  unsigned indent = static_cast<unsigned>(Indent + 1);
  જ્યારે (Current != End) {
    જો (*Current == '#')
      તોડો;

    જ્યારે (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      જો (i == Current)
        તોડો;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    જો (!isBlankOrBreak(Current))
      તોડો;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    જ્યારે (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      જો (i != Tmp) {
        જો (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          પાછા ખોટું;
        }
        Tmp = i;
        ++Column;
      } નહીંતર {
        i = skip_b_break(Tmp);
        જો (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++લીટી;
      }
    }

    જો (!FlowLevel && Column < indent)
      તોડો;

    Current = Tmp;
  }
  જો (Start == Current) {
    setError("Got empty plain scalar", Start);
    પાછા ખોટું;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ધકેલો_back(T);

  // Plain scalars can be simple keys.
  save_Simple_ચાવી_Candidate(--TokenQueue.end(), ColStart, ખોટું);

  Is_Simple_ચાવી_Allowed = ખોટું;
  Is_Adjacent_કિંમત_Allowed_In_Flow = ખોટું;

  પાછા સાચું;
}

તાર્કિક Scanner::scanAliasOrAnchor(તાર્કિક IsAlias) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1);
  જ્યારે (Current != End) {
    જો (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      તોડો;
    StringRef::iterator i = skip_ns_char(Current);
    જો (i == Current)
      તોડો;
    Current = i;
    ++Column;
  }

  જો (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    પાછા ખોટું;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ધકેલો_back(T);

  // Alias and anchors can be simple keys.
  save_Simple_ચાવી_Candidate(--TokenQueue.end(), ColStart, ખોટું);

  Is_Simple_ચાવી_Allowed = ખોટું;
  Is_Adjacent_કિંમત_Allowed_In_Flow = ખોટું;

  પાછા સાચું;
}

તાર્કિક Scanner::scanBlockScalarIndicators(અક્ષર &StyleIndicator,
                                        અક્ષર &ChompingIndicator,
                                        unsigned &IndentIndicator,
                                        તાર્કિક &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  જો (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    પાછા ખોટું;
  પાછા સાચું;
}

અક્ષર Scanner::scanBlockStyleIndicator() {
  અક્ષર Indicator = ' ';
  જો (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  પાછા Indicator;
}

અક્ષર Scanner::scanBlockChompingIndicator() {
  અક્ષર Indicator = ' ';
  જો (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  પાછા Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
સ્થિર unsigned get_Chomped_લીટી_Breaks(અક્ષર ChompingIndicator,
                                     unsigned લીટી_Breaks, StringRef Str) {
  જો (ChompingIndicator == '-') // Strip all line breaks.
    પાછા 0;
  જો (ChompingIndicator == '+') // Keep all line breaks.
    પાછા લીટી_Breaks;
  // Clip trailing lines.
  પાછા Str.empty() ? 0 : 1;
}

unsigned Scanner::scanBlockIndentationIndicator() {
  unsigned Indent = 0;
  જો (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = unsigned(*Current - '0');
    skip(1);
  }
  પાછા Indent;
}

તાર્કિક Scanner::scanBlockScalarHeader(અક્ષર &ChompingIndicator,
                                    unsigned &IndentIndicator, તાર્કિક &IsDone) {
  auto Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  જો (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  જો (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ધકેલો_back(T);
    IsDone = સાચું;
    પાછા સાચું;
  }

  જો (!consume_લીટી_Break_If_Present()) {
    setError("Expected a line break after block scalar header", Current);
    પાછા ખોટું;
  }
  પાછા સાચું;
}

તાર્કિક Scanner::findBlockScalarIndent(unsigned &BlockIndent,
                                    unsigned BlockExitIndent,
                                    unsigned &લીટી_Breaks, તાર્કિક &IsDone) {
  unsigned Max_All_Space_લીટી_Characters = 0;
  StringRef::iterator Longest_All_Space_લીટી;

  જ્યારે (સાચું) {
    advanceWhile(&Scanner::skip_s_space);
    જો (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      જો (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = સાચું;
        પાછા સાચું;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      જો (Max_All_Space_લીટી_Characters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_લીટી);
        પાછા ખોટું;
      }
      પાછા સાચું;
    }
    જો (skip_b_break(Current) != Current &&
        Column > Max_All_Space_લીટી_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_લીટી_Characters = Column;
      Longest_All_Space_લીટી = Current;
    }

    // Check for EOF.
    જો (Current == End) {
      IsDone = સાચું;
      પાછા સાચું;
    }

    જો (!consume_લીટી_Break_If_Present()) {
      IsDone = સાચું;
      પાછા સાચું;
    }
    ++લીટી_Breaks;
  }
  પાછા સાચું;
}

તાર્કિક Scanner::scanBlockScalarIndent(unsigned BlockIndent,
                                    unsigned BlockExitIndent, તાર્કિક &IsDone) {
  // Skip the indentation.
  જ્યારે (Column < BlockIndent) {
    auto I = skip_s_space(Current);
    જો (I == Current)
      તોડો;
    Current = I;
    ++Column;
  }

  જો (skip_nb_char(Current) == Current)
    પાછા સાચું;

  જો (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = સાચું;
    પાછા સાચું;
  }

  જો (Column < BlockIndent) {
    જો (Current != End && *Current == '#') { // Trailing comment.
      IsDone = સાચું;
      પાછા સાચું;
    }
    setError("A text line is less indented than the block scalar", Current);
    પાછા ખોટું;
  }
  પાછા સાચું; // A normal text line.
}

તાર્કિક Scanner::scanBlockScalar(તાર્કિક IsLiteral) {
  પ્રતિજ્ઞા(*Current == '|' || *Current == '>');
  અક્ષર StyleIndicator;
  અક્ષર ChompingIndicator;
  unsigned BlockIndent;
  તાર્કિક IsDone = ખોટું;
  જો (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    પાછા ખોટું;
  જો (IsDone)
    પાછા સાચું;
  તાર્કિક IsFolded = StyleIndicator == '>';

  અચલ auto *Start = Current;
  unsigned BlockExitIndent = Indent < 0 ? 0 : (unsigned)Indent;
  unsigned લીટી_Breaks = 0;
  જો (BlockIndent == 0) {
    જો (!findBlockScalarIndent(BlockIndent, BlockExitIndent, લીટી_Breaks,
                               IsDone))
      પાછા ખોટું;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  જ્યારે (!IsDone) {
    જો (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      પાછા ખોટું;
    જો (IsDone)
      તોડો;

    // Parse the current line.
    auto લીટી_Start = Current;
    advanceWhile(&Scanner::skip_nb_char);
    જો (લીટી_Start != Current) {
      જો (લીટી_Breaks && IsFolded && !Scanner::is_લીટી_Empty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        જો (લીટી_Breaks == 1) {
          Str.append(લીટી_Breaks,
                     is_લીટી_Empty(StringRef(લીટી_Start, Current - લીટી_Start))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        લીટી_Breaks--;
      }
      Str.append(લીટી_Breaks, '\n');
      Str.append(StringRef(લીટી_Start, Current - લીટી_Start));
      લીટી_Breaks = 0;
    }

    // Check for EOF.
    જો (Current == End)
      તોડો;

    જો (!consume_લીટી_Break_If_Present())
      તોડો;
    ++લીટી_Breaks;
  }

  જો (Current == End && !લીટી_Breaks)
    // Ensure that there is at least one line break before the end of file.
    લીટી_Breaks = 1;
  Str.append(get_Chomped_લીટી_Breaks(ChompingIndicator, લીટી_Breaks, Str), '\n');

  // New lines may start a simple key.
  જો (!FlowLevel)
    Is_Simple_ચાવી_Allowed = સાચું;
  Is_Adjacent_કિંમત_Allowed_In_Flow = ખોટું;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.કિંમત = std::string(Str);
  TokenQueue.ધકેલો_back(T);
  પાછા સાચું;
}

તાર્કિક Scanner::scanTag() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1); // Eat !.
  જો (Current == End || isBlankOrBreak(Current)); // An empty tag.
  નહીંતર જો (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    જો (!consume('>'))
      પાછા ખોટું;
  } નહીંતર {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.Kind = Token::TK_Tag;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ધકેલો_back(T);

  // Tags can be simple keys.
  save_Simple_ચાવી_Candidate(--TokenQueue.end(), ColStart, ખોટું);

  Is_Simple_ચાવી_Allowed = ખોટું;
  Is_Adjacent_કિંમત_Allowed_In_Flow = ખોટું;

  પાછા સાચું;
}

તાર્કિક Scanner::fetchMoreTokens() {
  જો (IsStartOfStream)
    પાછા scanStreamStart();

  scanToNextToken();

  જો (Current == End)
    પાછા scanStreamEnd();

  remove_Stale_Simple_ચાવી_Candidates();

  unrollIndent(Column);

  જો (Column == 0 && *Current == '%')
    પાછા scanDirective();

  જો (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    પાછા scanDocumentIndicator(સાચું);

  જો (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    પાછા scanDocumentIndicator(ખોટું);

  જો (*Current == '[')
    પાછા scanFlowCollectionStart(સાચું);

  જો (*Current == '{')
    પાછા scanFlowCollectionStart(ખોટું);

  જો (*Current == ']')
    પાછા scanFlowCollectionEnd(સાચું);

  જો (*Current == '}')
    પાછા scanFlowCollectionEnd(ખોટું);

  જો (*Current == ',')
    પાછા scanFlowEntry();

  જો (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    પાછા scanBlockEntry();

  જો (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    પાછા scan_ચાવી();

  જો (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || Is_Adjacent_કિંમત_Allowed_In_Flow))
    પાછા scan_કિંમત();

  જો (*Current == '*')
    પાછા scanAliasOrAnchor(સાચું);

  જો (*Current == '&')
    પાછા scanAliasOrAnchor(ખોટું);

  જો (*Current == '!')
    પાછા scanTag();

  જો (*Current == '|' && !FlowLevel)
    પાછા scanBlockScalar(સાચું);

  જો (*Current == '>' && !FlowLevel)
    પાછા scanBlockScalar(ખોટું);

  જો (*Current == '\'')
    પાછા scanFlowScalar(ખોટું);

  જો (*Current == '"')
    પાછા scanFlowScalar(સાચું);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  જો ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    પાછા scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  પાછા ખોટું;
}

Stream::Stream(StringRef આગત, SourceMgr &SM, તાર્કિક ShowColors,
               std::error_code *EC)
    : scanner(નવું Scanner(આગત, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef આગત_Buffer, SourceMgr &SM, તાર્કિક ShowColors,
               std::error_code *EC)
    : scanner(નવું Scanner(આગત_Buffer, SM, ShowColors, EC)) {}

Stream::~Stream() = મૂળભૂત;

તાર્કિક Stream::failed() { પાછા scanner->failed(); }

શૂન્ય Stream::printError(ગાંઠ *N, અચલ Twine &સંદેશ_2, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), સંદેશ_2, Kind);
}

શૂન્ય Stream::printError(અચલ SMRange &Range, અચલ Twine &સંદેશ_2,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, સંદેશ_2, Range);
}

document_iterator Stream::begin() {
  જો (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(નવું Document(*આ));
  પાછા document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  પાછા document_iterator();
}

શૂન્ય Stream::skip() {
  માટે (Document &Doc : *આ)
    Doc.skip();
}

ગાંઠ::ગાંઠ(unsigned પૂર્ણાંક Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string ગાંઠ::getVerbatimTag() અચલ {
  StringRef Raw = getRawTag();
  જો (!Raw.empty() && Raw != "!") {
    std::string Ret;
    જો (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      પાછા Ret;
    } નહીંતર જો (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      પાછા Ret;
    } નહીંતર {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      જો (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      નહીંતર {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      પાછા Ret;
    }
  }

  પસંદગી (getType()) {
  સ્થિતિ NK_Null:
    પાછા "tag:yaml.org,2002:null";
  સ્થિતિ NK_Scalar:
  સ્થિતિ NK_BlockScalar:
    // TODO: Tag resolution.
    પાછા "tag:yaml.org,2002:str";
  સ્થિતિ NK_Mapping:
    પાછા "tag:yaml.org,2002:map";
  સ્થિતિ NK_Sequence:
    પાછા "tag:yaml.org,2002:seq";
  }

  પાછા "";
}

Token &ગાંઠ::peekNext() {
  પાછા Doc->peekNext();
}

Token ગાંઠ::getNext() {
  પાછા Doc->getNext();
}

ગાંઠ *ગાંઠ::parse_Block_ગાંઠ() {
  પાછા Doc->parse_Block_ગાંઠ();
}

BumpPtrAllocator &ગાંઠ::getAllocator() {
  પાછા Doc->ગાંઠ_Allocator;
}

શૂન્ય ગાંઠ::setError(અચલ Twine &સંદેશ_2, Token &Tok) અચલ {
  Doc->setError(સંદેશ_2, Tok);
}

તાર્કિક ગાંઠ::failed() અચલ {
  પાછા Doc->failed();
}

StringRef Scalar_ગાંઠ::get_કિંમત(SmallVectorImpl<અક્ષર> &Storage) અચલ {
  જો (કિંમત[0] == '"')
    પાછા get_Double_Quoted_કિંમત(કિંમત, Storage);
  જો (કિંમત[0] == '\'')
    પાછા get_Single_Quoted_કિંમત(કિંમત, Storage);
  પાછા get_Plain_કિંમત(કિંમત, Storage);
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
સ્થિર StringRef
parse_Scalar_કિંમત(StringRef Unquoted_કિંમત, SmallVectorImpl<અક્ષર> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<અક્ષર> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_કિંમત.find_first_of(LookupChars);
  જો (I == StringRef::npos)
    પાછા Unquoted_કિંમત;

  Storage.clear();
  Storage.reserve(Unquoted_કિંમત.size());
  અક્ષર Last_New_લીટી_Added_As = '\0';
  માટે (; I != StringRef::npos; I = Unquoted_કિંમત.find_first_of(LookupChars)) {
    જો (Unquoted_કિંમત[I] != '\r' && Unquoted_કિંમત[I] != '\n') {
      llvm::append_range(Storage, Unquoted_કિંમત.take_front(I));
      Unquoted_કિંમત = UnescapeCallback(Unquoted_કિંમત.drop_front(I), Storage);
      Last_New_લીટી_Added_As = '\0';
      ચાલુ;
    }
    જો (size_t LastNonSWhite = Unquoted_કિંમત.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_કિંમત.take_front(LastNonSWhite + 1));
      Storage.ધકેલો_back(' ');
      Last_New_લીટી_Added_As = ' ';
    } નહીંતર {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      પસંદગી (Last_New_લીટી_Added_As) {
      સ્થિતિ ' ':
        પ્રતિજ્ઞા(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        Last_New_લીટી_Added_As = '\n';
        તોડો;
      સ્થિતિ '\n':
        પ્રતિજ્ઞા(!Storage.empty() && Storage.back() == '\n');
        Storage.ધકેલો_back('\n');
        તોડો;
      મૂળભૂત:
        Storage.ધકેલો_back(' ');
        Last_New_લીટી_Added_As = ' ';
        તોડો;
      }
    }
    // Handle Windows-style EOL
    જો (Unquoted_કિંમત.substr(I, 2) == "\r\n")
      I++;
    Unquoted_કિંમત = Unquoted_કિંમત.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_કિંમત);
  પાછા StringRef(Storage.begin(), Storage.size());
}

StringRef
Scalar_ગાંઠ::get_Double_Quoted_કિંમત(StringRef Raw_કિંમત,
                                 SmallVectorImpl<અક્ષર> &Storage) અચલ {
  પ્રતિજ્ઞા(Raw_કિંમત.size() >= 2 && Raw_કિંમત.front() == '"' &&
         Raw_કિંમત.back() == '"');
  StringRef Unquoted_કિંમત = Raw_કિંમત.substr(1, Raw_કિંમત.size() - 2);

  auto UnescapeFunc = [આ](StringRef Unquoted_કિંમત,
                             SmallVectorImpl<અક્ષર> &Storage) {
    પ્રતિજ્ઞા(Unquoted_કિંમત.take_front(1) == "\\");
    જો (Unquoted_કિંમત.size() == 1) {
      Token T;
      T.Range = Unquoted_કિંમત;
      setError("Unrecognized escape code", T);
      Storage.clear();
      પાછા StringRef();
    }
    Unquoted_કિંમત = Unquoted_કિંમત.drop_front(1);
    પસંદગી (Unquoted_કિંમત[0]) {
    મૂળભૂત: {
      Token T;
      T.Range = Unquoted_કિંમત.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      પાછા StringRef();
    }
    સ્થિતિ '\r':
      // Shrink the Windows-style EOL.
      જો (Unquoted_કિંમત.size() >= 2 && Unquoted_કિંમત[1] == '\n')
        Unquoted_કિંમત = Unquoted_કિંમત.drop_front(1);
      [[fallthrough]];
    સ્થિતિ '\n':
      પાછા Unquoted_કિંમત.drop_front(1).ltrim(" \t");
    સ્થિતિ '0':
      Storage.ધકેલો_back(0x00);
      તોડો;
    સ્થિતિ 'a':
      Storage.ધકેલો_back(0x07);
      તોડો;
    સ્થિતિ 'b':
      Storage.ધકેલો_back(0x08);
      તોડો;
    સ્થિતિ 't':
    સ્થિતિ 0x09:
      Storage.ધકેલો_back(0x09);
      તોડો;
    સ્થિતિ 'n':
      Storage.ધકેલો_back(0x0A);
      તોડો;
    સ્થિતિ 'v':
      Storage.ધકેલો_back(0x0B);
      તોડો;
    સ્થિતિ 'f':
      Storage.ધકેલો_back(0x0C);
      તોડો;
    સ્થિતિ 'r':
      Storage.ધકેલો_back(0x0D);
      તોડો;
    સ્થિતિ 'e':
      Storage.ધકેલો_back(0x1B);
      તોડો;
    સ્થિતિ ' ':
      Storage.ધકેલો_back(0x20);
      તોડો;
    સ્થિતિ '"':
      Storage.ધકેલો_back(0x22);
      તોડો;
    સ્થિતિ '/':
      Storage.ધકેલો_back(0x2F);
      તોડો;
    સ્થિતિ '\\':
      Storage.ધકેલો_back(0x5C);
      તોડો;
    સ્થિતિ 'N':
      encodeUTF8(0x85, Storage);
      તોડો;
    સ્થિતિ '_':
      encodeUTF8(0xA0, Storage);
      તોડો;
    સ્થિતિ 'L':
      encodeUTF8(0x2028, Storage);
      તોડો;
    સ્થિતિ 'P':
      encodeUTF8(0x2029, Storage);
      તોડો;
    સ્થિતિ 'x': {
      જો (Unquoted_કિંમત.size() < 3)
        // TODO: Report error.
        તોડો;
      unsigned પૂર્ણાંક Unicode_Scalar_કિંમત;
      જો (Unquoted_કિંમત.substr(1, 2).getAsInteger(16, Unicode_Scalar_કિંમત))
        // TODO: Report error.
        Unicode_Scalar_કિંમત = 0xFFFD;
      encodeUTF8(Unicode_Scalar_કિંમત, Storage);
      પાછા Unquoted_કિંમત.drop_front(3);
    }
    સ્થિતિ 'u': {
      જો (Unquoted_કિંમત.size() < 5)
        // TODO: Report error.
        તોડો;
      unsigned પૂર્ણાંક Unicode_Scalar_કિંમત;
      જો (Unquoted_કિંમત.substr(1, 4).getAsInteger(16, Unicode_Scalar_કિંમત))
        // TODO: Report error.
        Unicode_Scalar_કિંમત = 0xFFFD;
      encodeUTF8(Unicode_Scalar_કિંમત, Storage);
      પાછા Unquoted_કિંમત.drop_front(5);
    }
    સ્થિતિ 'U': {
      જો (Unquoted_કિંમત.size() < 9)
        // TODO: Report error.
        તોડો;
      unsigned પૂર્ણાંક Unicode_Scalar_કિંમત;
      જો (Unquoted_કિંમત.substr(1, 8).getAsInteger(16, Unicode_Scalar_કિંમત))
        // TODO: Report error.
        Unicode_Scalar_કિંમત = 0xFFFD;
      encodeUTF8(Unicode_Scalar_કિંમત, Storage);
      પાછા Unquoted_કિંમત.drop_front(9);
    }
    }
    પાછા Unquoted_કિંમત.drop_front(1);
  };

  પાછા parse_Scalar_કિંમત(Unquoted_કિંમત, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_ગાંઠ::get_Single_Quoted_કિંમત(StringRef Raw_કિંમત,
                                           SmallVectorImpl<અક્ષર> &Storage) {
  પ્રતિજ્ઞા(Raw_કિંમત.size() >= 2 && Raw_કિંમત.front() == '\'' &&
         Raw_કિંમત.back() == '\'');
  StringRef Unquoted_કિંમત = Raw_કિંમત.substr(1, Raw_કિંમત.size() - 2);

  auto UnescapeFunc = [](StringRef Unquoted_કિંમત,
                         SmallVectorImpl<અક્ષર> &Storage) {
    પ્રતિજ્ઞા(Unquoted_કિંમત.take_front(2) == "''");
    Storage.ધકેલો_back('\'');
    પાછા Unquoted_કિંમત.drop_front(2);
  };

  પાછા parse_Scalar_કિંમત(Unquoted_કિંમત, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_ગાંઠ::get_Plain_કિંમત(StringRef Raw_કિંમત,
                                    SmallVectorImpl<અક્ષર> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_કિંમત = Raw_કિંમત.rtrim("\r\n \t");
  પાછા parse_Scalar_કિંમત(Raw_કિંમત, Storage, "\r\n", nullptr);
}

ગાંઠ *ચાવી_કિંમત_ગાંઠ::get_ચાવી() {
  જો (ચાવી)
    પાછા ચાવી;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    જો (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_કિંમત
        || t.Kind == Token::TK_Error) {
      પાછા ચાવી = નવું (getAllocator()) Null_ગાંઠ(Doc);
    }
    જો (t.Kind == Token::TK_ચાવી)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  જો (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_કિંમત) {
    પાછા ચાવી = નવું (getAllocator()) Null_ગાંઠ(Doc);
  }

  // We've got a normal key.
  પાછા ચાવી = parse_Block_ગાંઠ();
}

ગાંઠ *ચાવી_કિંમત_ગાંઠ::get_કિંમત() {
  જો (કિંમત)
    પાછા કિંમત;

  જો (ગાંઠ* ચાવી = get_ચાવી())
    ચાવી->skip();
  નહીંતર {
    setError("Null key in Key Value.", peekNext());
    પાછા કિંમત = નવું (getAllocator()) Null_ગાંઠ(Doc);
  }

  જો (failed())
    પાછા કિંમત = નવું (getAllocator()) Null_ગાંઠ(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    જો (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_ચાવી
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      પાછા કિંમત = નવું (getAllocator()) Null_ગાંઠ(Doc);
    }

    જો (t.Kind != Token::TK_કિંમત) {
      setError("Unexpected token in Key Value.", t);
      પાછા કિંમત = નવું (getAllocator()) Null_ગાંઠ(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  જો (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_ચાવી) {
    પાછા કિંમત = નવું (getAllocator()) Null_ગાંઠ(Doc);
  }

  // We got a normal value.
  પાછા કિંમત = parse_Block_ગાંઠ();
}

શૂન્ય Mapping_ગાંઠ::increment() {
  જો (failed()) {
    IsAtEnd = સાચું;
    CurrentEntry = nullptr;
    પાછા;
  }
  જો (CurrentEntry) {
    CurrentEntry->skip();
    જો (Type == MT_Inline) {
      IsAtEnd = સાચું;
      CurrentEntry = nullptr;
      પાછા;
    }
  }
  Token T = peekNext();
  જો (T.Kind == Token::TK_ચાવી || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = નવું (getAllocator()) ચાવી_કિંમત_ગાંઠ(Doc);
  } નહીંતર જો (Type == MT_Block) {
    પસંદગી (T.Kind) {
    સ્થિતિ Token::TK_BlockEnd:
      getNext();
      IsAtEnd = સાચું;
      CurrentEntry = nullptr;
      તોડો;
    મૂળભૂત:
      setError("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    સ્થિતિ Token::TK_Error:
      IsAtEnd = સાચું;
      CurrentEntry = nullptr;
    }
  } નહીંતર {
    પસંદગી (T.Kind) {
    સ્થિતિ Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      પાછા increment();
    સ્થિતિ Token::TK_FlowMappingEnd:
      getNext();
      [[fallthrough]];
    સ્થિતિ Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = સાચું;
      CurrentEntry = nullptr;
      તોડો;
    મૂળભૂત:
      setError( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      IsAtEnd = સાચું;
      CurrentEntry = nullptr;
    }
  }
}

શૂન્ય Sequence_ગાંઠ::increment() {
  જો (failed()) {
    IsAtEnd = સાચું;
    CurrentEntry = nullptr;
    પાછા;
  }
  જો (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  જો (SeqType == ST_Block) {
    પસંદગી (T.Kind) {
    સ્થિતિ Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_ગાંઠ();
      જો (!CurrentEntry) { // An error occurred.
        IsAtEnd = સાચું;
        CurrentEntry = nullptr;
      }
      તોડો;
    સ્થિતિ Token::TK_BlockEnd:
      getNext();
      IsAtEnd = સાચું;
      CurrentEntry = nullptr;
      તોડો;
    મૂળભૂત:
      setError( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    સ્થિતિ Token::TK_Error:
      IsAtEnd = સાચું;
      CurrentEntry = nullptr;
    }
  } નહીંતર જો (SeqType == ST_Indentless) {
    પસંદગી (T.Kind) {
    સ્થિતિ Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_ગાંઠ();
      જો (!CurrentEntry) { // An error occurred.
        IsAtEnd = સાચું;
        CurrentEntry = nullptr;
      }
      તોડો;
    મૂળભૂત:
    સ્થિતિ Token::TK_Error:
      IsAtEnd = સાચું;
      CurrentEntry = nullptr;
    }
  } નહીંતર જો (SeqType == ST_Flow) {
    પસંદગી (T.Kind) {
    સ્થિતિ Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      WasPreviousTokenFlowEntry = સાચું;
      પાછા increment();
    સ્થિતિ Token::TK_FlowSequenceEnd:
      getNext();
      [[fallthrough]];
    સ્થિતિ Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = સાચું;
      CurrentEntry = nullptr;
      તોડો;
    સ્થિતિ Token::TK_StreamEnd:
    સ્થિતિ Token::TK_DocumentEnd:
    સ્થિતિ Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = સાચું;
      CurrentEntry = nullptr;
      તોડો;
    મૂળભૂત:
      જો (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = સાચું;
        CurrentEntry = nullptr;
        તોડો;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parse_Block_ગાંઠ();
      જો (!CurrentEntry) {
        IsAtEnd = સાચું;
      }
      WasPreviousTokenFlowEntry = ખોટું;
      તોડો;
    }
  }
}

Document::Document(Stream &S) : stream(S), મૂળ_2(nullptr) {
  // Tag maps starts with two default mappings.
  TagMap["!"] = "!";
  TagMap["!!"] = "tag:yaml.org,2002:";

  જો (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peekNext();
  જો (T.Kind == Token::TK_DocumentStart)
    getNext();
}

તાર્કિક Document::skip()  {
  જો (stream.scanner->failed())
    પાછા ખોટું;
  જો (!મૂળ_2 && !get_મૂળ())
    પાછા ખોટું;
  મૂળ_2->skip();
  Token &T = peekNext();
  જો (T.Kind == Token::TK_StreamEnd)
    પાછા ખોટું;
  જો (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    પાછા skip();
  }
  પાછા સાચું;
}

Token &Document::peekNext() {
  પાછા stream.scanner->peekNext();
}

Token Document::getNext() {
  પાછા stream.scanner->getNext();
}

શૂન્ય Document::setError(અચલ Twine &Message, Token &Location) અચલ {
  stream.scanner->setError(Message, Location.Range.begin());
}

તાર્કિક Document::failed() અચલ {
  પાછા stream.scanner->failed();
}

ગાંઠ *Document::parse_Block_ગાંઠ() {
  Token T = peekNext();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  પસંદગી (T.Kind) {
  સ્થિતિ Token::TK_Alias:
    getNext();
    પાછા નવું (ગાંઠ_Allocator) Alias_ગાંઠ(stream.CurrentDoc, T.Range.substr(1));
  સ્થિતિ Token::TK_Anchor:
    જો (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      પાછા nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    જાઓ parse_property;
  સ્થિતિ Token::TK_Tag:
    જો (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      પાછા nullptr;
    }
    TagInfo = getNext(); // Consume TK_Tag.
    T = peekNext();
    જાઓ parse_property;
  મૂળભૂત:
    તોડો;
  }

  પસંદગી (T.Kind) {
  સ્થિતિ Token::TK_BlockEntry:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    પાછા નવું (ગાંઠ_Allocator) Sequence_ગાંઠ( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , Sequence_ગાંઠ::ST_Indentless);
  સ્થિતિ Token::TK_BlockSequenceStart:
    getNext();
    પાછા નવું (ગાંઠ_Allocator)
      Sequence_ગાંઠ( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_ગાંઠ::ST_Block);
  સ્થિતિ Token::TK_BlockMappingStart:
    getNext();
    પાછા નવું (ગાંઠ_Allocator)
      Mapping_ગાંઠ( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_ગાંઠ::MT_Block);
  સ્થિતિ Token::TK_FlowSequenceStart:
    getNext();
    પાછા નવું (ગાંઠ_Allocator)
      Sequence_ગાંઠ( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_ગાંઠ::ST_Flow);
  સ્થિતિ Token::TK_FlowMappingStart:
    getNext();
    પાછા નવું (ગાંઠ_Allocator)
      Mapping_ગાંઠ( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_ગાંઠ::MT_Flow);
  સ્થિતિ Token::TK_Scalar:
    getNext();
    પાછા નવું (ગાંઠ_Allocator)
      Scalar_ગાંઠ( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  સ્થિતિ Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.કિંમત.c_str(), T.કિંમત.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(ગાંઠ_Allocator).drop_back();
    પાછા નવું (ગાંઠ_Allocator)
        Block_Scalar_ગાંઠ(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  સ્થિતિ Token::TK_ચાવી:
    // Don't eat the TK_Key, KeyValueNode expects it.
    પાછા નવું (ગાંઠ_Allocator)
      Mapping_ગાંઠ( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_ગાંઠ::MT_Inline);
  સ્થિતિ Token::TK_DocumentStart:
  સ્થિતિ Token::TK_DocumentEnd:
  સ્થિતિ Token::TK_StreamEnd:
  મૂળભૂત:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    પાછા નવું (ગાંઠ_Allocator) Null_ગાંઠ(stream.CurrentDoc);
  સ્થિતિ Token::TK_FlowMappingEnd:
  સ્થિતિ Token::TK_FlowSequenceEnd:
  સ્થિતિ Token::TK_FlowEntry: {
    જો (મૂળ_2 && (isa<Mapping_ગાંઠ>(મૂળ_2) || isa<Sequence_ગાંઠ>(મૂળ_2)))
      પાછા નવું (ગાંઠ_Allocator) Null_ગાંઠ(stream.CurrentDoc);

    setError("Unexpected token", T);
    પાછા nullptr;
  }
  સ્થિતિ Token::TK_Error:
    પાછા nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  પાછા nullptr;
}

તાર્કિક Document::parseDirectives() {
  તાર્કિક isDirective = ખોટું;
  જ્યારે (સાચું) {
    Token T = peekNext();
    જો (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = સાચું;
    } નહીંતર જો (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = સાચું;
    } નહીંતર {
      તોડો;
    }
  }
  પાછા isDirective;
}

શૂન્ય Document::parseYAMLDirective() {
  getNext(); // Eat %YAML <version>
}

શૂન્ય Document::parseTAGDirective() {
  Token Tag = getNext(); // %TAG <handle> <prefix>
  StringRef T = Tag.Range;
  // Strip %TAG
  T = T.substr(T.find_first_of(" \t")).ltrim(" \t");
  std::size_t HandleEnd = T.find_first_of(" \t");
  StringRef TagHandle = T.substr(0, HandleEnd);
  StringRef TagPrefix = T.substr(HandleEnd).ltrim(" \t");
  TagMap[TagHandle] = TagPrefix;
}

તાર્કિક Document::expectToken(પૂર્ણાંક TK) {
  Token T = getNext();
  જો (T.Kind != TK) {
    setError("Unexpected token", T);
    પાછા ખોટું;
  }
  પાછા સાચું;
}
