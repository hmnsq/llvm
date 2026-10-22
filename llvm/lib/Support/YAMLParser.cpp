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

#அந்தர்பாவ "llvm/Support/YAMLParser.h"
#அந்தர்பாவ "llvm/ADT/AllocatorList.h"
#அந்தர்பாவ "llvm/ADT/ArrayRef.h"
#அந்தர்பாவ "llvm/ADT/STLExtras.h"
#அந்தர்பாவ "llvm/ADT/SmallString.h"
#அந்தர்பாவ "llvm/ADT/SmallVector.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/ADT/Twine.h"
#அந்தர்பாவ "llvm/Support/Compiler.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ "llvm/Support/MemoryBuffer.h"
#அந்தர்பாவ "llvm/Support/SMLoc.h"
#அந்தர்பாவ "llvm/Support/SourceMgr.h"
#அந்தர்பாவ "llvm/Support/Unicode.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <cassert>
#அந்தர்பாவ <cstddef>
#அந்தர்பாவ <cstdint>
#அந்தர்பாவ <map>
#அந்தர்பாவ <memory>
#அந்தர்பாவ <string>
#அந்தர்பாவ <system_error>
#அந்தர்பாவ <utility>

using நாமாகாஶ llvm;
using நாமாகாஶ yaml;

எண்ணிக்கை UnicodeEncodingForm {
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
நிலையான EncodingInfo getUnicodeEncoding(StringRef உள்ளீடு) {
  எனில் (உள்ளீடு.empty())
    திரும்பு {UEF_Unknown, 0};

  தேர்வு (uint8_t(உள்ளீடு[0])) {
  நிலை 0x00:
    எனில் (உள்ளீடு.size() >= 4) {
      எனில் (  உள்ளீடு[1] == 0
         && uint8_t(உள்ளீடு[2]) == 0xFE
         && uint8_t(உள்ளீடு[3]) == 0xFF)
        திரும்பு {UEF_UTF32_BE, 4};
      எனில் (உள்ளீடு[1] == 0 && உள்ளீடு[2] == 0 && உள்ளீடு[3] != 0)
        திரும்பு {UEF_UTF32_BE, 0};
    }

    எனில் (உள்ளீடு.size() >= 2 && உள்ளீடு[1] != 0)
      திரும்பு {UEF_UTF16_BE, 0};
    திரும்பு {UEF_Unknown, 0};
  நிலை 0xFF:
    எனில் (  உள்ளீடு.size() >= 4
       && uint8_t(உள்ளீடு[1]) == 0xFE
       && உள்ளீடு[2] == 0
       && உள்ளீடு[3] == 0)
      திரும்பு {UEF_UTF32_LE, 4};

    எனில் (உள்ளீடு.size() >= 2 && uint8_t(உள்ளீடு[1]) == 0xFE)
      திரும்பு {UEF_UTF16_LE, 2};
    திரும்பு {UEF_Unknown, 0};
  நிலை 0xFE:
    எனில் (உள்ளீடு.size() >= 2 && uint8_t(உள்ளீடு[1]) == 0xFF)
      திரும்பு {UEF_UTF16_BE, 2};
    திரும்பு {UEF_Unknown, 0};
  நிலை 0xEF:
    எனில் (  உள்ளீடு.size() >= 3
       && uint8_t(உள்ளீடு[1]) == 0xBB
       && uint8_t(உள்ளீடு[2]) == 0xBF)
      திரும்பு {UEF_UTF8, 3};
    திரும்பு {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  எனில் (உள்ளீடு.size() >= 4 && உள்ளீடு[1] == 0 && உள்ளீடு[2] == 0 && உள்ளீடு[3] == 0)
    திரும்பு {UEF_UTF32_LE, 0};

  எனில் (உள்ளீடு.size() >= 2 && உள்ளீடு[1] == 0)
    திரும்பு {UEF_UTF16_LE, 0};

  திரும்பு {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
வெறுமை முனை::anchor() {}
வெறுமை Null_முனை::anchor() {}
வெறுமை Scalar_முனை::anchor() {}
வெறுமை Block_Scalar_முனை::anchor() {}
வெறுமை சாவி_மதிப்பு_முனை::anchor() {}
வெறுமை Mapping_முனை::anchor() {}
வெறுமை Sequence_முனை::anchor() {}
வெறுமை Alias_முனை::anchor() {}

நாமாகாஶ llvm {
நாமாகாஶ yaml {

/// Token - A single YAML token.
கட்டமைப்பு Token {
  எண்ணிக்கை TokenKind {
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
    TK_சாவி,
    TK_மதிப்பு,
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
  std::string மதிப்பு;

  Token() = இயல்பு;
};

} // end namespace yaml
} // end namespace llvm

using TokenQueueT = BumpPtrList<Token>;

நாமாகாஶ {

/// This struct is used to track simple keys.
///
/// Simple keys are handled by creating an entry in SimpleKeys for each Token
/// which could legally be the start of a simple key. When peekNext is called,
/// if the Token To be returned is referenced by a SimpleKey, we continue
/// tokenizing until that potential simple key has either been found to not be
/// a simple key (we moved on to the next line or went further than 1024 chars).
/// Or when we run into a Value, and then insert a Key token (and possibly
/// others) before the SimpleKey's Tok.
கட்டமைப்பு Simple_சாவி {
  TokenQueueT::iterator Tok;
  unsigned Column = 0;
  unsigned வரி = 0;
  unsigned FlowLevel = 0;
  தருக்க IsRequired = பொய்;

  தருக்க operator ==(மாறிலி Simple_சாவி &Other) {
    திரும்பு Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
using UTF8Decoded = std::pair<uint32_t, unsigned>;

நிலையான UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator End = Range.end();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  எனில் (Position < End && (*Position & 0x80) == 0) {
    திரும்பு {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  எனில் (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    எனில் (codepoint >= 0x80)
      திரும்பு {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  எனில் (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    எனில் (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      திரும்பு {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  எனில் (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    எனில் (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      திரும்பு {codepoint, 4};
  }
  திரும்பு {0, 0};
}

நாமாகாஶ llvm {
நாமாகாஶ yaml {

/// Scans YAML tokens from a MemoryBuffer.
வகுப்பு Scanner {
பொது:
  Scanner(StringRef உள்ளீடு, SourceMgr &SM, தருக்க ShowColors = உண்மை,
          std::error_code *EC = nullptr);
  Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, தருக்க ShowColors = உண்மை,
          std::error_code *EC = nullptr);

  /// Parse the next token and return it without popping it.
  Token &peekNext();

  /// Parse the next token and pop it from the queue.
  Token getNext();

  வெறுமை printError(SMLoc Loc, SourceMgr::DiagKind Kind, மாறிலி Twine &Message,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.PrintMessage(Loc, Kind, Message, Ranges, /* FixIts= */ {}, ShowColors);
  }

  வெறுமை setError(மாறிலி Twine &Message, StringRef::iterator Position) {
    எனில் (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    எனில் (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    எனில் (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = உண்மை;
  }

  /// Returns true if an error occurred while parsing.
  தருக்க failed() {
    திரும்பு Failed;
  }

தனிப்பட்ட:
  வெறுமை init(MemoryBufferRef Buffer);

  StringRef current_உள்ளீடு() {
    திரும்பு StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    திரும்பு ::decodeUTF8(StringRef(Position, End - Position));
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
  வெறுமை advanceWhile(SkipWhileFunc Func);

  /// Scan ns-uri-char[39]s starting at Cur.
  ///
  /// This updates Cur and Column while scanning.
  வெறுமை scan_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  தருக்க consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  வெறுமை skip(uint32_t Distance);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  தருக்க isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  தருக்க isPlainSafeNonBlank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  தருக்க is_வரி_Empty(StringRef வரி);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  தருக்க consume_வரி_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  வெறுமை save_Simple_சாவி_Candidate( TokenQueueT::iterator Tok
                             , unsigned AtColumn
                             , தருக்க IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  வெறுமை remove_Stale_Simple_சாவி_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  வெறுமை remove_Simple_சாவி_Candidates_On_Flow_Level(unsigned Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  தருக்க unrollIndent(முழு ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  தருக்க rollIndent( முழு ToColumn
                 , Token::TokenKind Kind
                 , TokenQueueT::iterator செருகு_Point);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  வெறுமை skipComment();

  /// Skip whitespace and comments until the start of the next token.
  வெறுமை scanToNextToken();

  /// Must be the first token generated.
  தருக்க scanStreamStart();

  /// Generate tokens needed to close out the stream.
  தருக்க scanStreamEnd();

  /// Scan a %BLAH directive.
  தருக்க scanDirective();

  /// Scan a ... or ---.
  தருக்க scanDocumentIndicator(தருக்க IsStart);

  /// Scan a [ or { and generate the proper flow collection start token.
  தருக்க scanFlowCollectionStart(தருக்க IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  தருக்க scanFlowCollectionEnd(தருக்க IsSequence);

  /// Scan the , that separates entries in a flow collection.
  தருக்க scanFlowEntry();

  /// Scan the - that starts block sequence entries.
  தருக்க scanBlockEntry();

  /// Scan an explicit ? indicating a key.
  தருக்க scan_சாவி();

  /// Scan an explicit : indicating a value.
  தருக்க scan_மதிப்பு();

  /// Scan a quoted scalar.
  தருக்க scanFlowScalar(தருக்க IsDoubleQuoted);

  /// Scan an unquoted scalar.
  தருக்க scanPlainScalar();

  /// Scan an Alias or Anchor starting with * or &.
  தருக்க scanAliasOrAnchor(தருக்க IsAlias);

  /// Scan a block scalar starting with | or >.
  தருக்க scanBlockScalar(தருக்க IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  தருக்க scanBlockScalarIndicators(எழுத்து &StyleIndicator, எழுத்து &ChompingIndicator,
                                 unsigned &IndentIndicator, தருக்க &IsDone);

  /// Scan a style indicator in a block scalar header.
  எழுத்து scanBlockStyleIndicator();

  /// Scan a chomping indicator in a block scalar header.
  எழுத்து scanBlockChompingIndicator();

  /// Scan an indentation indicator in a block scalar header.
  unsigned scanBlockIndentationIndicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  தருக்க scanBlockScalarHeader(எழுத்து &ChompingIndicator, unsigned &IndentIndicator,
                             தருக்க &IsDone);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  தருக்க findBlockScalarIndent(unsigned &BlockIndent, unsigned BlockExitIndent,
                             unsigned &வரி_Breaks, தருக்க &IsDone);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  தருக்க scanBlockScalarIndent(unsigned BlockIndent, unsigned BlockExitIndent,
                             தருக்க &IsDone);

  /// Scan a tag of the form !stuff.
  தருக்க scanTag();

  /// Dispatch to the next scanning function based on \a *Cur.
  தருக்க fetchMoreTokens();

  /// The SourceMgr used for diagnostics and buffer management.
  SourceMgr &SM;

  /// The original input.
  MemoryBufferRef உள்ளீடு_Buffer;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator End;

  /// Current YAML indentation level in spaces.
  முழு Indent;

  /// Current column number in Unicode code points.
  unsigned Column;

  /// Current line number.
  unsigned வரி;

  /// How deep we are in flow style containers. 0 Means at block level.
  unsigned FlowLevel;

  /// Are we at the start of the stream?
  தருக்க IsStartOfStream;

  /// Can the next token be the start of a simple key?
  தருக்க Is_Simple_சாவி_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  தருக்க Is_Adjacent_மதிப்பு_Allowed_In_Flow;

  /// True if an error has occurred.
  தருக்க Failed;

  /// Should colors be used when printing out the diagnostic messages?
  தருக்க ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  TokenQueueT TokenQueue;

  /// Indentation levels.
  SmallVector<முழு, 4> Indents;

  /// Potential simple keys.
  SmallVector<Simple_சாவி, 4> Simple_சாவிகள்;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
நிலையான வெறுமை encodeUTF8( uint32_t Unicode_Scalar_மதிப்பு
                      , SmallVectorImpl<எழுத்து> &Result) {
  எனில் (Unicode_Scalar_மதிப்பு <= 0x7F) {
    Result.தள்ளு_back(Unicode_Scalar_மதிப்பு & 0x7F);
  } இல்லையெனில் எனில் (Unicode_Scalar_மதிப்பு <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((Unicode_Scalar_மதிப்பு & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Unicode_Scalar_மதிப்பு & 0x3F);
    Result.தள்ளு_back(FirstByte);
    Result.தள்ளு_back(SecondByte);
  } இல்லையெனில் எனில் (Unicode_Scalar_மதிப்பு <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((Unicode_Scalar_மதிப்பு & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_மதிப்பு & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Unicode_Scalar_மதிப்பு & 0x3F);
    Result.தள்ளு_back(FirstByte);
    Result.தள்ளு_back(SecondByte);
    Result.தள்ளு_back(ThirdByte);
  } இல்லையெனில் எனில் (Unicode_Scalar_மதிப்பு <= 0x10FFFF) {
    uint8_t FirstByte = 0xF0 | ((Unicode_Scalar_மதிப்பு & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_மதிப்பு & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Unicode_Scalar_மதிப்பு & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_மதிப்பு & 0x3F);
    Result.தள்ளு_back(FirstByte);
    Result.தள்ளு_back(SecondByte);
    Result.தள்ளு_back(ThirdByte);
    Result.தள்ளு_back(FourthByte);
  }
}

தருக்க yaml::dumpTokens(StringRef உள்ளீடு, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(உள்ளீடு, SM);
  போது (உண்மை) {
    Token T = scanner.getNext();
    தேர்வு (T.Kind) {
    நிலை Token::TK_StreamStart:
      OS << "Stream-Start: ";
      நிறுத்து;
    நிலை Token::TK_StreamEnd:
      OS << "Stream-End: ";
      நிறுத்து;
    நிலை Token::TK_VersionDirective:
      OS << "Version-Directive: ";
      நிறுத்து;
    நிலை Token::TK_TagDirective:
      OS << "Tag-Directive: ";
      நிறுத்து;
    நிலை Token::TK_DocumentStart:
      OS << "Document-Start: ";
      நிறுத்து;
    நிலை Token::TK_DocumentEnd:
      OS << "Document-End: ";
      நிறுத்து;
    நிலை Token::TK_BlockEntry:
      OS << "Block-Entry: ";
      நிறுத்து;
    நிலை Token::TK_BlockEnd:
      OS << "Block-End: ";
      நிறுத்து;
    நிலை Token::TK_BlockSequenceStart:
      OS << "Block-Sequence-Start: ";
      நிறுத்து;
    நிலை Token::TK_BlockMappingStart:
      OS << "Block-Mapping-Start: ";
      நிறுத்து;
    நிலை Token::TK_FlowEntry:
      OS << "Flow-Entry: ";
      நிறுத்து;
    நிலை Token::TK_FlowSequenceStart:
      OS << "Flow-Sequence-Start: ";
      நிறுத்து;
    நிலை Token::TK_FlowSequenceEnd:
      OS << "Flow-Sequence-End: ";
      நிறுத்து;
    நிலை Token::TK_FlowMappingStart:
      OS << "Flow-Mapping-Start: ";
      நிறுத்து;
    நிலை Token::TK_FlowMappingEnd:
      OS << "Flow-Mapping-End: ";
      நிறுத்து;
    நிலை Token::TK_சாவி:
      OS << "Key: ";
      நிறுத்து;
    நிலை Token::TK_மதிப்பு:
      OS << "Value: ";
      நிறுத்து;
    நிலை Token::TK_Scalar:
      OS << "Scalar: ";
      நிறுத்து;
    நிலை Token::TK_BlockScalar:
      OS << "Block Scalar: ";
      நிறுத்து;
    நிலை Token::TK_Alias:
      OS << "Alias: ";
      நிறுத்து;
    நிலை Token::TK_Anchor:
      OS << "Anchor: ";
      நிறுத்து;
    நிலை Token::TK_Tag:
      OS << "Tag: ";
      நிறுத்து;
    நிலை Token::TK_Error:
      நிறுத்து;
    }
    OS << T.Range << "\n";
    எனில் (T.Kind == Token::TK_StreamEnd)
      நிறுத்து;
    இல்லையெனில் எனில் (T.Kind == Token::TK_Error)
      திரும்பு பொய்;
  }
  திரும்பு உண்மை;
}

தருக்க yaml::scanTokens(StringRef உள்ளீடு) {
  SourceMgr SM;
  Scanner scanner(உள்ளீடு, SM);
  போது (உண்மை) {
    Token T = scanner.getNext();
    எனில் (T.Kind == Token::TK_StreamEnd)
      நிறுத்து;
    இல்லையெனில் எனில் (T.Kind == Token::TK_Error)
      திரும்பு பொய்;
  }
  திரும்பு உண்மை;
}

std::string yaml::escape(StringRef உள்ளீடு, தருக்க EscapePrintable) {
  std::string Escaped_உள்ளீடு;
  க்கு (StringRef::iterator i = உள்ளீடு.begin(), e = உள்ளீடு.end(); i != e; ++i) {
    எனில் (*i == '\\')
      Escaped_உள்ளீடு += "\\\\";
    இல்லையெனில் எனில் (*i == '"')
      Escaped_உள்ளீடு += "\\\"";
    இல்லையெனில் எனில் (*i == 0)
      Escaped_உள்ளீடு += "\\0";
    இல்லையெனில் எனில் (*i == 0x07)
      Escaped_உள்ளீடு += "\\a";
    இல்லையெனில் எனில் (*i == 0x08)
      Escaped_உள்ளீடு += "\\b";
    இல்லையெனில் எனில் (*i == 0x09)
      Escaped_உள்ளீடு += "\\t";
    இல்லையெனில் எனில் (*i == 0x0A)
      Escaped_உள்ளீடு += "\\n";
    இல்லையெனில் எனில் (*i == 0x0B)
      Escaped_உள்ளீடு += "\\v";
    இல்லையெனில் எனில் (*i == 0x0C)
      Escaped_உள்ளீடு += "\\f";
    இல்லையெனில் எனில் (*i == 0x0D)
      Escaped_உள்ளீடு += "\\r";
    இல்லையெனில் எனில் (*i == 0x1B)
      Escaped_உள்ளீடு += "\\e";
    இல்லையெனில் எனில் ((unsigned எழுத்து)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_உள்ளீடு += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } இல்லையெனில் எனில் (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_மதிப்பு
        = decodeUTF8(StringRef(i, உள்ளீடு.end() - i));
      எனில் (Unicode_Scalar_மதிப்பு.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_உள்ளீடு, Val);
        // FIXME: Error reporting.
        திரும்பு Escaped_உள்ளீடு;
      }
      எனில் (Unicode_Scalar_மதிப்பு.first == 0x85)
        Escaped_உள்ளீடு += "\\N";
      இல்லையெனில் எனில் (Unicode_Scalar_மதிப்பு.first == 0xA0)
        Escaped_உள்ளீடு += "\\_";
      இல்லையெனில் எனில் (Unicode_Scalar_மதிப்பு.first == 0x2028)
        Escaped_உள்ளீடு += "\\L";
      இல்லையெனில் எனில் (Unicode_Scalar_மதிப்பு.first == 0x2029)
        Escaped_உள்ளீடு += "\\P";
      இல்லையெனில் எனில் (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_மதிப்பு.first))
        Escaped_உள்ளீடு += StringRef(i, Unicode_Scalar_மதிப்பு.second);
      இல்லையெனில் {
        std::string HexStr = utohexstr(Unicode_Scalar_மதிப்பு.first);
        எனில் (HexStr.size() <= 2)
          Escaped_உள்ளீடு += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        இல்லையெனில் எனில் (HexStr.size() <= 4)
          Escaped_உள்ளீடு += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        இல்லையெனில் எனில் (HexStr.size() <= 8)
          Escaped_உள்ளீடு += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += Unicode_Scalar_மதிப்பு.second - 1;
    } இல்லையெனில் {
      Escaped_உள்ளீடு.தள்ளு_back(*i);
    }
  }
  திரும்பு Escaped_உள்ளீடு;
}

std::optional<தருக்க> yaml::parseBool(StringRef S) {
  தேர்வு (S.size()) {
  நிலை 1:
    தேர்வு (S.front()) {
    நிலை 'y':
    நிலை 'Y':
      திரும்பு உண்மை;
    நிலை 'n':
    நிலை 'N':
      திரும்பு பொய்;
    இயல்பு:
      திரும்பு std::nullopt;
    }
  நிலை 2:
    தேர்வு (S.front()) {
    நிலை 'O':
      எனில் (S[1] == 'N') // ON
        திரும்பு உண்மை;
      [[fallthrough]];
    நிலை 'o':
      எனில் (S[1] == 'n') //[Oo]n
        திரும்பு உண்மை;
      திரும்பு std::nullopt;
    நிலை 'N':
      எனில் (S[1] == 'O') // NO
        திரும்பு பொய்;
      [[fallthrough]];
    நிலை 'n':
      எனில் (S[1] == 'o') //[Nn]o
        திரும்பு பொய்;
      திரும்பு std::nullopt;
    இயல்பு:
      திரும்பு std::nullopt;
    }
  நிலை 3:
    தேர்வு (S.front()) {
    நிலை 'O':
      எனில் (S.drop_front() == "FF") // OFF
        திரும்பு பொய்;
      [[fallthrough]];
    நிலை 'o':
      எனில் (S.drop_front() == "ff") //[Oo]ff
        திரும்பு பொய்;
      திரும்பு std::nullopt;
    நிலை 'Y':
      எனில் (S.drop_front() == "ES") // YES
        திரும்பு உண்மை;
      [[fallthrough]];
    நிலை 'y':
      எனில் (S.drop_front() == "es") //[Yy]es
        திரும்பு உண்மை;
      திரும்பு std::nullopt;
    இயல்பு:
      திரும்பு std::nullopt;
    }
  நிலை 4:
    தேர்வு (S.front()) {
    நிலை 'T':
      எனில் (S.drop_front() == "RUE") // TRUE
        திரும்பு உண்மை;
      [[fallthrough]];
    நிலை 't':
      எனில் (S.drop_front() == "rue") //[Tt]rue
        திரும்பு உண்மை;
      திரும்பு std::nullopt;
    இயல்பு:
      திரும்பு std::nullopt;
    }
  நிலை 5:
    தேர்வு (S.front()) {
    நிலை 'F':
      எனில் (S.drop_front() == "ALSE") // FALSE
        திரும்பு பொய்;
      [[fallthrough]];
    நிலை 'f':
      எனில் (S.drop_front() == "alse") //[Ff]alse
        திரும்பு பொய்;
      திரும்பு std::nullopt;
    இயல்பு:
      திரும்பு std::nullopt;
    }
  இயல்பு:
    திரும்பு std::nullopt;
  }
}

Scanner::Scanner(StringRef உள்ளீடு, SourceMgr &sm, தருக்க ShowColors,
                 std::error_code *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(MemoryBufferRef(உள்ளீடு, "YAML"));
}

Scanner::Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, தருக்க ShowColors,
                 std::error_code *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(Buffer);
}

வெறுமை Scanner::init(MemoryBufferRef Buffer) {
  உள்ளீடு_Buffer = Buffer;
  Current = உள்ளீடு_Buffer.getBufferStart();
  End = உள்ளீடு_Buffer.getBufferEnd();
  Indent = -1;
  Column = 0;
  வரி = 0;
  FlowLevel = 0;
  IsStartOfStream = உண்மை;
  Is_Simple_சாவி_Allowed = உண்மை;
  Is_Adjacent_மதிப்பு_Allowed_In_Flow = பொய்;
  Failed = பொய்;
  std::unique_ptr<MemoryBuffer> உள்ளீடு_Buffer_Owner =
      MemoryBuffer::getMemBuffer(Buffer, /*RequiresNullTerminator=*/பொய்);
  SM.AddNewSourceBuffer(std::move(உள்ளீடு_Buffer_Owner), SMLoc());
}

Token &Scanner::peekNext() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  தருக்க NeedMore = பொய்;
  போது (உண்மை) {
    எனில் (TokenQueue.empty() || NeedMore) {
      எனில் (!fetchMoreTokens()) {
        TokenQueue.clear();
        Simple_சாவிகள்.clear();
        TokenQueue.தள்ளு_back(Token());
        திரும்பு TokenQueue.front();
      }
    }
    ப்ரதிஜ்ஞா(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_சாவி_Candidates();
    Simple_சாவி SK;
    SK.Tok = TokenQueue.begin();
    எனில் (!is_contained(Simple_சாவிகள், SK))
      நிறுத்து;
    இல்லையெனில்
      NeedMore = உண்மை;
  }
  திரும்பு TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  எனில் (!TokenQueue.empty())
    TokenQueue.எடு_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  எனில் (TokenQueue.empty())
    TokenQueue.resetAlloc();

  திரும்பு Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  எனில் (Position == End)
    திரும்பு Position;
  // Check 7 bit c-printable - b-char.
  எனில் (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    திரும்பு Position + 1;

  // Check for valid UTF-8.
  எனில் (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    எனில் (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      திரும்பு Position + u8d.second;
  }
  திரும்பு Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  எனில் (Position == End)
    திரும்பு Position;
  எனில் (*Position == 0x0D) {
    எனில் (Position + 1 != End && *(Position + 1) == 0x0A)
      திரும்பு Position + 2;
    திரும்பு Position + 1;
  }

  எனில் (*Position == 0x0A)
    திரும்பு Position + 1;
  திரும்பு Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  எனில் (Position == End)
    திரும்பு Position;
  எனில் (*Position == ' ')
    திரும்பு Position + 1;
  திரும்பு Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  எனில் (Position == End)
    திரும்பு Position;
  எனில் (*Position == ' ' || *Position == '\t')
    திரும்பு Position + 1;
  திரும்பு Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  எனில் (Position == End)
    திரும்பு Position;
  எனில் (*Position == ' ' || *Position == '\t')
    திரும்பு Position;
  திரும்பு skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  போது (உண்மை) {
    StringRef::iterator i = (இது->*Func)(Position);
    எனில் (i == Position)
      நிறுத்து;
    Position = i;
  }
  திரும்பு Position;
}

வெறுமை Scanner::advanceWhile(SkipWhileFunc Func) {
  auto Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

நிலையான தருக்க is_ns_hex_digit(மாறிலி எழுத்து C) { திரும்பு isAlnum(C); }

நிலையான தருக்க is_ns_word_char(மாறிலி எழுத்து C) { திரும்பு C == '-' || isAlpha(C); }

வெறுமை Scanner::scan_ns_uri_char() {
  போது (உண்மை) {
    எனில் (Current == End)
      நிறுத்து;
    எனில் ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } இல்லையெனில் {
      நிறுத்து;
    }
  }
}

தருக்க Scanner::consume(uint32_t Expected) {
  எனில் (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    திரும்பு பொய்;
  }
  எனில் (Current == End)
    திரும்பு பொய்;
  எனில் (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    திரும்பு பொய்;
  }
  எனில் (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    திரும்பு உண்மை;
  }
  திரும்பு பொய்;
}

வெறுமை Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  ப்ரதிஜ்ஞா(Current <= End && "Skipped past the end");
}

தருக்க Scanner::isBlankOrBreak(StringRef::iterator Position) {
  எனில் (Position == End)
    திரும்பு பொய்;
  திரும்பு *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

தருக்க Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  எனில் (Position == End || isBlankOrBreak(Position))
    திரும்பு பொய்;
  எனில் (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    திரும்பு பொய்;
  திரும்பு உண்மை;
}

தருக்க Scanner::is_வரி_Empty(StringRef வரி) {
  க்கு (மாறிலி auto *Position = வரி.begin(); Position != வரி.end(); ++Position)
    எனில் (!isBlankOrBreak(Position))
      திரும்பு பொய்;
  திரும்பு உண்மை;
}

தருக்க Scanner::consume_வரி_Break_If_Present() {
  auto Next = skip_b_break(Current);
  எனில் (Next == Current)
    திரும்பு பொய்;
  Column = 0;
  ++வரி;
  Current = Next;
  திரும்பு உண்மை;
}

வெறுமை Scanner::save_Simple_சாவி_Candidate( TokenQueueT::iterator Tok
                                    , unsigned AtColumn
                                    , தருக்க IsRequired) {
  எனில் (Is_Simple_சாவி_Allowed) {
    Simple_சாவி SK;
    SK.Tok = Tok;
    SK.வரி = வரி;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_சாவிகள்.தள்ளு_back(SK);
  }
}

வெறுமை Scanner::remove_Stale_Simple_சாவி_Candidates() {
  க்கு (SmallVectorImpl<Simple_சாவி>::iterator i = Simple_சாவிகள்.begin();
                                            i != Simple_சாவிகள்.end();) {
    எனில் (i->வரி != வரி || i->Column + 1024 < Column) {
      எனில் (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_சாவிகள்.erase(i);
    } இல்லையெனில் {
      ++i;
    }
  }
}

வெறுமை Scanner::remove_Simple_சாவி_Candidates_On_Flow_Level(unsigned Level) {
  எனில் (!Simple_சாவிகள்.empty() && (Simple_சாவிகள்.end() - 1)->FlowLevel == Level)
    Simple_சாவிகள்.எடு_back();
}

தருக்க Scanner::unrollIndent(முழு ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  எனில் (FlowLevel != 0)
    திரும்பு உண்மை;

  போது (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.தள்ளு_back(T);
    Indent = Indents.எடு_back_val();
  }

  திரும்பு உண்மை;
}

தருக்க Scanner::rollIndent( முழு ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator செருகு_Point) {
  எனில் (FlowLevel)
    திரும்பு உண்மை;
  எனில் (Indent < ToColumn) {
    Indents.தள்ளு_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.செருகு(செருகு_Point, T);
  }
  திரும்பு உண்மை;
}

வெறுமை Scanner::skipComment() {
  எனில் (Current == End || *Current != '#')
    திரும்பு;
  போது (உண்மை) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    எனில் (I == Current)
      நிறுத்து;
    Current = I;
    ++Column;
  }
}

வெறுமை Scanner::scanToNextToken() {
  போது (உண்மை) {
    போது (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    எனில் (i == Current)
      நிறுத்து;
    Current = i;
    ++வரி;
    Column = 0;
    // New lines may start a simple key.
    எனில் (!FlowLevel)
      Is_Simple_சாவி_Allowed = உண்மை;
  }
}

தருக்க Scanner::scanStreamStart() {
  IsStartOfStream = பொய்;

  EncodingInfo EI = getUnicodeEncoding(current_உள்ளீடு());

  Token T;
  T.Kind = Token::TK_StreamStart;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.தள்ளு_back(T);
  Current += EI.second;
  திரும்பு உண்மை;
}

தருக்க Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  எனில் (Column != 0) {
    Column = 0;
    ++வரி;
  }

  unrollIndent(-1);
  Simple_சாவிகள்.clear();
  Is_Simple_சாவி_Allowed = பொய்;
  Is_Adjacent_மதிப்பு_Allowed_In_Flow = பொய்;

  Token T;
  T.Kind = Token::TK_StreamEnd;
  T.Range = StringRef(Current, 0);
  TokenQueue.தள்ளு_back(T);
  திரும்பு உண்மை;
}

தருக்க Scanner::scanDirective() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_சாவிகள்.clear();
  Is_Simple_சாவி_Allowed = பொய்;
  Is_Adjacent_மதிப்பு_Allowed_In_Flow = பொய்;

  StringRef::iterator Start = Current;
  consume('%');
  StringRef::iterator NameStart = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(NameStart, Current - NameStart);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  எனில் (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.தள்ளு_back(T);
    திரும்பு உண்மை;
  } இல்லையெனில் எனில்(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.தள்ளு_back(T);
    திரும்பு உண்மை;
  }
  திரும்பு பொய்;
}

தருக்க Scanner::scanDocumentIndicator(தருக்க IsStart) {
  unrollIndent(-1);
  Simple_சாவிகள்.clear();
  Is_Simple_சாவி_Allowed = பொய்;
  Is_Adjacent_மதிப்பு_Allowed_In_Flow = பொய்;

  Token T;
  T.Kind = IsStart ? Token::TK_DocumentStart : Token::TK_DocumentEnd;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.தள்ளு_back(T);
  திரும்பு உண்மை;
}

தருக்க Scanner::scanFlowCollectionStart(தருக்க IsSequence) {
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.தள்ளு_back(T);

  // [ and { may begin a simple key.
  save_Simple_சாவி_Candidate(--TokenQueue.end(), Column - 1, பொய்);

  // And may also be followed by a simple key.
  Is_Simple_சாவி_Allowed = உண்மை;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_மதிப்பு_Allowed_In_Flow = பொய்;
  ++FlowLevel;
  திரும்பு உண்மை;
}

தருக்க Scanner::scanFlowCollectionEnd(தருக்க IsSequence) {
  remove_Simple_சாவி_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_சாவி_Allowed = பொய்;
  Is_Adjacent_மதிப்பு_Allowed_In_Flow = உண்மை;
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceEnd
                      : Token::TK_FlowMappingEnd;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.தள்ளு_back(T);
  எனில் (FlowLevel)
    --FlowLevel;
  திரும்பு உண்மை;
}

தருக்க Scanner::scanFlowEntry() {
  remove_Simple_சாவி_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_சாவி_Allowed = உண்மை;
  Is_Adjacent_மதிப்பு_Allowed_In_Flow = பொய்;
  Token T;
  T.Kind = Token::TK_FlowEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.தள்ளு_back(T);
  திரும்பு உண்மை;
}

தருக்க Scanner::scanBlockEntry() {
  rollIndent(Column, Token::TK_BlockSequenceStart, TokenQueue.end());
  remove_Simple_சாவி_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_சாவி_Allowed = உண்மை;
  Is_Adjacent_மதிப்பு_Allowed_In_Flow = பொய்;
  Token T;
  T.Kind = Token::TK_BlockEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.தள்ளு_back(T);
  திரும்பு உண்மை;
}

தருக்க Scanner::scan_சாவி() {
  எனில் (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  remove_Simple_சாவி_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_சாவி_Allowed = !FlowLevel;
  Is_Adjacent_மதிப்பு_Allowed_In_Flow = பொய்;

  Token T;
  T.Kind = Token::TK_சாவி;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.தள்ளு_back(T);
  திரும்பு உண்மை;
}

தருக்க Scanner::scan_மதிப்பு() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  எனில் (!Simple_சாவிகள்.empty()) {
    Simple_சாவி SK = Simple_சாவிகள்.எடு_back_val();
    Token T;
    T.Kind = Token::TK_சாவி;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    க்கு (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      எனில் (i == SK.Tok)
        நிறுத்து;
    }
    எனில் (i == e) {
      Failed = உண்மை;
      திரும்பு பொய்;
    }
    i = TokenQueue.செருகு(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    Is_Simple_சாவி_Allowed = பொய்;
  } இல்லையெனில் {
    எனில் (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    Is_Simple_சாவி_Allowed = !FlowLevel;
  }
  Is_Adjacent_மதிப்பு_Allowed_In_Flow = பொய்;

  Token T;
  T.Kind = Token::TK_மதிப்பு;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.தள்ளு_back(T);
  திரும்பு உண்மை;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE நிலையான தருக்க
wasEscaped(StringRef::iterator First, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
நிலையான தருக்க wasEscaped(StringRef::iterator First,
                       StringRef::iterator Position) {
  ப்ரதிஜ்ஞா(Position - 1 >= First);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  போது (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  திரும்பு (Position - 1 - I) % 2 == 1;
}

தருக்க Scanner::scanFlowScalar(தருக்க IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  எனில் (IsDoubleQuoted) {
    செய் {
      ++Current;
      போது (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } போது (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } இல்லையெனில் {
    skip(1);
    போது (Current != End) {
      // Skip a ' followed by another '.
      எனில் (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        தொடர்;
      } இல்லையெனில் எனில் (*Current == '\'')
        நிறுத்து;
      StringRef::iterator i = skip_nb_char(Current);
      எனில் (i == Current) {
        i = skip_b_break(Current);
        எனில் (i == Current)
          நிறுத்து;
        Current = i;
        Column = 0;
        ++வரி;
      } இல்லையெனில் {
        எனில் (i == End)
          நிறுத்து;
        Current = i;
        ++Column;
      }
    }
  }

  எனில் (Current == End) {
    setError("Expected quote at end of scalar", Current);
    திரும்பு பொய்;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.தள்ளு_back(T);

  save_Simple_சாவி_Candidate(--TokenQueue.end(), ColStart, பொய்);

  Is_Simple_சாவி_Allowed = பொய்;
  Is_Adjacent_மதிப்பு_Allowed_In_Flow = உண்மை;

  திரும்பு உண்மை;
}

தருக்க Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  unsigned LeadingBlanks = 0;
  ப்ரதிஜ்ஞா(Indent >= -1 && "Indent must be >= -1 !");
  unsigned indent = static_cast<unsigned>(Indent + 1);
  போது (Current != End) {
    எனில் (*Current == '#')
      நிறுத்து;

    போது (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      எனில் (i == Current)
        நிறுத்து;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    எனில் (!isBlankOrBreak(Current))
      நிறுத்து;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    போது (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      எனில் (i != Tmp) {
        எனில் (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          திரும்பு பொய்;
        }
        Tmp = i;
        ++Column;
      } இல்லையெனில் {
        i = skip_b_break(Tmp);
        எனில் (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++வரி;
      }
    }

    எனில் (!FlowLevel && Column < indent)
      நிறுத்து;

    Current = Tmp;
  }
  எனில் (Start == Current) {
    setError("Got empty plain scalar", Start);
    திரும்பு பொய்;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.தள்ளு_back(T);

  // Plain scalars can be simple keys.
  save_Simple_சாவி_Candidate(--TokenQueue.end(), ColStart, பொய்);

  Is_Simple_சாவி_Allowed = பொய்;
  Is_Adjacent_மதிப்பு_Allowed_In_Flow = பொய்;

  திரும்பு உண்மை;
}

தருக்க Scanner::scanAliasOrAnchor(தருக்க IsAlias) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1);
  போது (Current != End) {
    எனில் (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      நிறுத்து;
    StringRef::iterator i = skip_ns_char(Current);
    எனில் (i == Current)
      நிறுத்து;
    Current = i;
    ++Column;
  }

  எனில் (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    திரும்பு பொய்;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.தள்ளு_back(T);

  // Alias and anchors can be simple keys.
  save_Simple_சாவி_Candidate(--TokenQueue.end(), ColStart, பொய்);

  Is_Simple_சாவி_Allowed = பொய்;
  Is_Adjacent_மதிப்பு_Allowed_In_Flow = பொய்;

  திரும்பு உண்மை;
}

தருக்க Scanner::scanBlockScalarIndicators(எழுத்து &StyleIndicator,
                                        எழுத்து &ChompingIndicator,
                                        unsigned &IndentIndicator,
                                        தருக்க &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  எனில் (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    திரும்பு பொய்;
  திரும்பு உண்மை;
}

எழுத்து Scanner::scanBlockStyleIndicator() {
  எழுத்து Indicator = ' ';
  எனில் (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  திரும்பு Indicator;
}

எழுத்து Scanner::scanBlockChompingIndicator() {
  எழுத்து Indicator = ' ';
  எனில் (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  திரும்பு Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
நிலையான unsigned get_Chomped_வரி_Breaks(எழுத்து ChompingIndicator,
                                     unsigned வரி_Breaks, StringRef Str) {
  எனில் (ChompingIndicator == '-') // Strip all line breaks.
    திரும்பு 0;
  எனில் (ChompingIndicator == '+') // Keep all line breaks.
    திரும்பு வரி_Breaks;
  // Clip trailing lines.
  திரும்பு Str.empty() ? 0 : 1;
}

unsigned Scanner::scanBlockIndentationIndicator() {
  unsigned Indent = 0;
  எனில் (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = unsigned(*Current - '0');
    skip(1);
  }
  திரும்பு Indent;
}

தருக்க Scanner::scanBlockScalarHeader(எழுத்து &ChompingIndicator,
                                    unsigned &IndentIndicator, தருக்க &IsDone) {
  auto Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  எனில் (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  எனில் (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.தள்ளு_back(T);
    IsDone = உண்மை;
    திரும்பு உண்மை;
  }

  எனில் (!consume_வரி_Break_If_Present()) {
    setError("Expected a line break after block scalar header", Current);
    திரும்பு பொய்;
  }
  திரும்பு உண்மை;
}

தருக்க Scanner::findBlockScalarIndent(unsigned &BlockIndent,
                                    unsigned BlockExitIndent,
                                    unsigned &வரி_Breaks, தருக்க &IsDone) {
  unsigned Max_All_Space_வரி_Characters = 0;
  StringRef::iterator Longest_All_Space_வரி;

  போது (உண்மை) {
    advanceWhile(&Scanner::skip_s_space);
    எனில் (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      எனில் (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = உண்மை;
        திரும்பு உண்மை;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      எனில் (Max_All_Space_வரி_Characters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_வரி);
        திரும்பு பொய்;
      }
      திரும்பு உண்மை;
    }
    எனில் (skip_b_break(Current) != Current &&
        Column > Max_All_Space_வரி_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_வரி_Characters = Column;
      Longest_All_Space_வரி = Current;
    }

    // Check for EOF.
    எனில் (Current == End) {
      IsDone = உண்மை;
      திரும்பு உண்மை;
    }

    எனில் (!consume_வரி_Break_If_Present()) {
      IsDone = உண்மை;
      திரும்பு உண்மை;
    }
    ++வரி_Breaks;
  }
  திரும்பு உண்மை;
}

தருக்க Scanner::scanBlockScalarIndent(unsigned BlockIndent,
                                    unsigned BlockExitIndent, தருக்க &IsDone) {
  // Skip the indentation.
  போது (Column < BlockIndent) {
    auto I = skip_s_space(Current);
    எனில் (I == Current)
      நிறுத்து;
    Current = I;
    ++Column;
  }

  எனில் (skip_nb_char(Current) == Current)
    திரும்பு உண்மை;

  எனில் (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = உண்மை;
    திரும்பு உண்மை;
  }

  எனில் (Column < BlockIndent) {
    எனில் (Current != End && *Current == '#') { // Trailing comment.
      IsDone = உண்மை;
      திரும்பு உண்மை;
    }
    setError("A text line is less indented than the block scalar", Current);
    திரும்பு பொய்;
  }
  திரும்பு உண்மை; // A normal text line.
}

தருக்க Scanner::scanBlockScalar(தருக்க IsLiteral) {
  ப்ரதிஜ்ஞா(*Current == '|' || *Current == '>');
  எழுத்து StyleIndicator;
  எழுத்து ChompingIndicator;
  unsigned BlockIndent;
  தருக்க IsDone = பொய்;
  எனில் (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    திரும்பு பொய்;
  எனில் (IsDone)
    திரும்பு உண்மை;
  தருக்க IsFolded = StyleIndicator == '>';

  மாறிலி auto *Start = Current;
  unsigned BlockExitIndent = Indent < 0 ? 0 : (unsigned)Indent;
  unsigned வரி_Breaks = 0;
  எனில் (BlockIndent == 0) {
    எனில் (!findBlockScalarIndent(BlockIndent, BlockExitIndent, வரி_Breaks,
                               IsDone))
      திரும்பு பொய்;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  போது (!IsDone) {
    எனில் (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      திரும்பு பொய்;
    எனில் (IsDone)
      நிறுத்து;

    // Parse the current line.
    auto வரி_Start = Current;
    advanceWhile(&Scanner::skip_nb_char);
    எனில் (வரி_Start != Current) {
      எனில் (வரி_Breaks && IsFolded && !Scanner::is_வரி_Empty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        எனில் (வரி_Breaks == 1) {
          Str.append(வரி_Breaks,
                     is_வரி_Empty(StringRef(வரி_Start, Current - வரி_Start))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        வரி_Breaks--;
      }
      Str.append(வரி_Breaks, '\n');
      Str.append(StringRef(வரி_Start, Current - வரி_Start));
      வரி_Breaks = 0;
    }

    // Check for EOF.
    எனில் (Current == End)
      நிறுத்து;

    எனில் (!consume_வரி_Break_If_Present())
      நிறுத்து;
    ++வரி_Breaks;
  }

  எனில் (Current == End && !வரி_Breaks)
    // Ensure that there is at least one line break before the end of file.
    வரி_Breaks = 1;
  Str.append(get_Chomped_வரி_Breaks(ChompingIndicator, வரி_Breaks, Str), '\n');

  // New lines may start a simple key.
  எனில் (!FlowLevel)
    Is_Simple_சாவி_Allowed = உண்மை;
  Is_Adjacent_மதிப்பு_Allowed_In_Flow = பொய்;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.மதிப்பு = std::string(Str);
  TokenQueue.தள்ளு_back(T);
  திரும்பு உண்மை;
}

தருக்க Scanner::scanTag() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1); // Eat !.
  எனில் (Current == End || isBlankOrBreak(Current)); // An empty tag.
  இல்லையெனில் எனில் (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    எனில் (!consume('>'))
      திரும்பு பொய்;
  } இல்லையெனில் {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.Kind = Token::TK_Tag;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.தள்ளு_back(T);

  // Tags can be simple keys.
  save_Simple_சாவி_Candidate(--TokenQueue.end(), ColStart, பொய்);

  Is_Simple_சாவி_Allowed = பொய்;
  Is_Adjacent_மதிப்பு_Allowed_In_Flow = பொய்;

  திரும்பு உண்மை;
}

தருக்க Scanner::fetchMoreTokens() {
  எனில் (IsStartOfStream)
    திரும்பு scanStreamStart();

  scanToNextToken();

  எனில் (Current == End)
    திரும்பு scanStreamEnd();

  remove_Stale_Simple_சாவி_Candidates();

  unrollIndent(Column);

  எனில் (Column == 0 && *Current == '%')
    திரும்பு scanDirective();

  எனில் (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    திரும்பு scanDocumentIndicator(உண்மை);

  எனில் (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    திரும்பு scanDocumentIndicator(பொய்);

  எனில் (*Current == '[')
    திரும்பு scanFlowCollectionStart(உண்மை);

  எனில் (*Current == '{')
    திரும்பு scanFlowCollectionStart(பொய்);

  எனில் (*Current == ']')
    திரும்பு scanFlowCollectionEnd(உண்மை);

  எனில் (*Current == '}')
    திரும்பு scanFlowCollectionEnd(பொய்);

  எனில் (*Current == ',')
    திரும்பு scanFlowEntry();

  எனில் (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    திரும்பு scanBlockEntry();

  எனில் (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    திரும்பு scan_சாவி();

  எனில் (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || Is_Adjacent_மதிப்பு_Allowed_In_Flow))
    திரும்பு scan_மதிப்பு();

  எனில் (*Current == '*')
    திரும்பு scanAliasOrAnchor(உண்மை);

  எனில் (*Current == '&')
    திரும்பு scanAliasOrAnchor(பொய்);

  எனில் (*Current == '!')
    திரும்பு scanTag();

  எனில் (*Current == '|' && !FlowLevel)
    திரும்பு scanBlockScalar(உண்மை);

  எனில் (*Current == '>' && !FlowLevel)
    திரும்பு scanBlockScalar(பொய்);

  எனில் (*Current == '\'')
    திரும்பு scanFlowScalar(பொய்);

  எனில் (*Current == '"')
    திரும்பு scanFlowScalar(உண்மை);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  எனில் ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    திரும்பு scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  திரும்பு பொய்;
}

Stream::Stream(StringRef உள்ளீடு, SourceMgr &SM, தருக்க ShowColors,
               std::error_code *EC)
    : scanner(புதிய Scanner(உள்ளீடு, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef உள்ளீடு_Buffer, SourceMgr &SM, தருக்க ShowColors,
               std::error_code *EC)
    : scanner(புதிய Scanner(உள்ளீடு_Buffer, SM, ShowColors, EC)) {}

Stream::~Stream() = இயல்பு;

தருக்க Stream::failed() { திரும்பு scanner->failed(); }

வெறுமை Stream::printError(முனை *N, மாறிலி Twine &செய்தி_2, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), செய்தி_2, Kind);
}

வெறுமை Stream::printError(மாறிலி SMRange &Range, மாறிலி Twine &செய்தி_2,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, செய்தி_2, Range);
}

document_iterator Stream::begin() {
  எனில் (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(புதிய Document(*இது));
  திரும்பு document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  திரும்பு document_iterator();
}

வெறுமை Stream::skip() {
  க்கு (Document &Doc : *இது)
    Doc.skip();
}

முனை::முனை(unsigned முழு Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string முனை::getVerbatimTag() மாறிலி {
  StringRef Raw = getRawTag();
  எனில் (!Raw.empty() && Raw != "!") {
    std::string Ret;
    எனில் (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      திரும்பு Ret;
    } இல்லையெனில் எனில் (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      திரும்பு Ret;
    } இல்லையெனில் {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      எனில் (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      இல்லையெனில் {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      திரும்பு Ret;
    }
  }

  தேர்வு (getType()) {
  நிலை NK_Null:
    திரும்பு "tag:yaml.org,2002:null";
  நிலை NK_Scalar:
  நிலை NK_BlockScalar:
    // TODO: Tag resolution.
    திரும்பு "tag:yaml.org,2002:str";
  நிலை NK_Mapping:
    திரும்பு "tag:yaml.org,2002:map";
  நிலை NK_Sequence:
    திரும்பு "tag:yaml.org,2002:seq";
  }

  திரும்பு "";
}

Token &முனை::peekNext() {
  திரும்பு Doc->peekNext();
}

Token முனை::getNext() {
  திரும்பு Doc->getNext();
}

முனை *முனை::parse_Block_முனை() {
  திரும்பு Doc->parse_Block_முனை();
}

BumpPtrAllocator &முனை::getAllocator() {
  திரும்பு Doc->முனை_Allocator;
}

வெறுமை முனை::setError(மாறிலி Twine &செய்தி_2, Token &Tok) மாறிலி {
  Doc->setError(செய்தி_2, Tok);
}

தருக்க முனை::failed() மாறிலி {
  திரும்பு Doc->failed();
}

StringRef Scalar_முனை::get_மதிப்பு(SmallVectorImpl<எழுத்து> &Storage) மாறிலி {
  எனில் (மதிப்பு[0] == '"')
    திரும்பு get_Double_Quoted_மதிப்பு(மதிப்பு, Storage);
  எனில் (மதிப்பு[0] == '\'')
    திரும்பு get_Single_Quoted_மதிப்பு(மதிப்பு, Storage);
  திரும்பு get_Plain_மதிப்பு(மதிப்பு, Storage);
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
நிலையான StringRef
parse_Scalar_மதிப்பு(StringRef Unquoted_மதிப்பு, SmallVectorImpl<எழுத்து> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<எழுத்து> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_மதிப்பு.find_first_of(LookupChars);
  எனில் (I == StringRef::npos)
    திரும்பு Unquoted_மதிப்பு;

  Storage.clear();
  Storage.reserve(Unquoted_மதிப்பு.size());
  எழுத்து Last_New_வரி_Added_As = '\0';
  க்கு (; I != StringRef::npos; I = Unquoted_மதிப்பு.find_first_of(LookupChars)) {
    எனில் (Unquoted_மதிப்பு[I] != '\r' && Unquoted_மதிப்பு[I] != '\n') {
      llvm::append_range(Storage, Unquoted_மதிப்பு.take_front(I));
      Unquoted_மதிப்பு = UnescapeCallback(Unquoted_மதிப்பு.drop_front(I), Storage);
      Last_New_வரி_Added_As = '\0';
      தொடர்;
    }
    எனில் (size_t LastNonSWhite = Unquoted_மதிப்பு.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_மதிப்பு.take_front(LastNonSWhite + 1));
      Storage.தள்ளு_back(' ');
      Last_New_வரி_Added_As = ' ';
    } இல்லையெனில் {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      தேர்வு (Last_New_வரி_Added_As) {
      நிலை ' ':
        ப்ரதிஜ்ஞா(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        Last_New_வரி_Added_As = '\n';
        நிறுத்து;
      நிலை '\n':
        ப்ரதிஜ்ஞா(!Storage.empty() && Storage.back() == '\n');
        Storage.தள்ளு_back('\n');
        நிறுத்து;
      இயல்பு:
        Storage.தள்ளு_back(' ');
        Last_New_வரி_Added_As = ' ';
        நிறுத்து;
      }
    }
    // Handle Windows-style EOL
    எனில் (Unquoted_மதிப்பு.substr(I, 2) == "\r\n")
      I++;
    Unquoted_மதிப்பு = Unquoted_மதிப்பு.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_மதிப்பு);
  திரும்பு StringRef(Storage.begin(), Storage.size());
}

StringRef
Scalar_முனை::get_Double_Quoted_மதிப்பு(StringRef Raw_மதிப்பு,
                                 SmallVectorImpl<எழுத்து> &Storage) மாறிலி {
  ப்ரதிஜ்ஞா(Raw_மதிப்பு.size() >= 2 && Raw_மதிப்பு.front() == '"' &&
         Raw_மதிப்பு.back() == '"');
  StringRef Unquoted_மதிப்பு = Raw_மதிப்பு.substr(1, Raw_மதிப்பு.size() - 2);

  auto UnescapeFunc = [இது](StringRef Unquoted_மதிப்பு,
                             SmallVectorImpl<எழுத்து> &Storage) {
    ப்ரதிஜ்ஞா(Unquoted_மதிப்பு.take_front(1) == "\\");
    எனில் (Unquoted_மதிப்பு.size() == 1) {
      Token T;
      T.Range = Unquoted_மதிப்பு;
      setError("Unrecognized escape code", T);
      Storage.clear();
      திரும்பு StringRef();
    }
    Unquoted_மதிப்பு = Unquoted_மதிப்பு.drop_front(1);
    தேர்வு (Unquoted_மதிப்பு[0]) {
    இயல்பு: {
      Token T;
      T.Range = Unquoted_மதிப்பு.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      திரும்பு StringRef();
    }
    நிலை '\r':
      // Shrink the Windows-style EOL.
      எனில் (Unquoted_மதிப்பு.size() >= 2 && Unquoted_மதிப்பு[1] == '\n')
        Unquoted_மதிப்பு = Unquoted_மதிப்பு.drop_front(1);
      [[fallthrough]];
    நிலை '\n':
      திரும்பு Unquoted_மதிப்பு.drop_front(1).ltrim(" \t");
    நிலை '0':
      Storage.தள்ளு_back(0x00);
      நிறுத்து;
    நிலை 'a':
      Storage.தள்ளு_back(0x07);
      நிறுத்து;
    நிலை 'b':
      Storage.தள்ளு_back(0x08);
      நிறுத்து;
    நிலை 't':
    நிலை 0x09:
      Storage.தள்ளு_back(0x09);
      நிறுத்து;
    நிலை 'n':
      Storage.தள்ளு_back(0x0A);
      நிறுத்து;
    நிலை 'v':
      Storage.தள்ளு_back(0x0B);
      நிறுத்து;
    நிலை 'f':
      Storage.தள்ளு_back(0x0C);
      நிறுத்து;
    நிலை 'r':
      Storage.தள்ளு_back(0x0D);
      நிறுத்து;
    நிலை 'e':
      Storage.தள்ளு_back(0x1B);
      நிறுத்து;
    நிலை ' ':
      Storage.தள்ளு_back(0x20);
      நிறுத்து;
    நிலை '"':
      Storage.தள்ளு_back(0x22);
      நிறுத்து;
    நிலை '/':
      Storage.தள்ளு_back(0x2F);
      நிறுத்து;
    நிலை '\\':
      Storage.தள்ளு_back(0x5C);
      நிறுத்து;
    நிலை 'N':
      encodeUTF8(0x85, Storage);
      நிறுத்து;
    நிலை '_':
      encodeUTF8(0xA0, Storage);
      நிறுத்து;
    நிலை 'L':
      encodeUTF8(0x2028, Storage);
      நிறுத்து;
    நிலை 'P':
      encodeUTF8(0x2029, Storage);
      நிறுத்து;
    நிலை 'x': {
      எனில் (Unquoted_மதிப்பு.size() < 3)
        // TODO: Report error.
        நிறுத்து;
      unsigned முழு Unicode_Scalar_மதிப்பு;
      எனில் (Unquoted_மதிப்பு.substr(1, 2).getAsInteger(16, Unicode_Scalar_மதிப்பு))
        // TODO: Report error.
        Unicode_Scalar_மதிப்பு = 0xFFFD;
      encodeUTF8(Unicode_Scalar_மதிப்பு, Storage);
      திரும்பு Unquoted_மதிப்பு.drop_front(3);
    }
    நிலை 'u': {
      எனில் (Unquoted_மதிப்பு.size() < 5)
        // TODO: Report error.
        நிறுத்து;
      unsigned முழு Unicode_Scalar_மதிப்பு;
      எனில் (Unquoted_மதிப்பு.substr(1, 4).getAsInteger(16, Unicode_Scalar_மதிப்பு))
        // TODO: Report error.
        Unicode_Scalar_மதிப்பு = 0xFFFD;
      encodeUTF8(Unicode_Scalar_மதிப்பு, Storage);
      திரும்பு Unquoted_மதிப்பு.drop_front(5);
    }
    நிலை 'U': {
      எனில் (Unquoted_மதிப்பு.size() < 9)
        // TODO: Report error.
        நிறுத்து;
      unsigned முழு Unicode_Scalar_மதிப்பு;
      எனில் (Unquoted_மதிப்பு.substr(1, 8).getAsInteger(16, Unicode_Scalar_மதிப்பு))
        // TODO: Report error.
        Unicode_Scalar_மதிப்பு = 0xFFFD;
      encodeUTF8(Unicode_Scalar_மதிப்பு, Storage);
      திரும்பு Unquoted_மதிப்பு.drop_front(9);
    }
    }
    திரும்பு Unquoted_மதிப்பு.drop_front(1);
  };

  திரும்பு parse_Scalar_மதிப்பு(Unquoted_மதிப்பு, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_முனை::get_Single_Quoted_மதிப்பு(StringRef Raw_மதிப்பு,
                                           SmallVectorImpl<எழுத்து> &Storage) {
  ப்ரதிஜ்ஞா(Raw_மதிப்பு.size() >= 2 && Raw_மதிப்பு.front() == '\'' &&
         Raw_மதிப்பு.back() == '\'');
  StringRef Unquoted_மதிப்பு = Raw_மதிப்பு.substr(1, Raw_மதிப்பு.size() - 2);

  auto UnescapeFunc = [](StringRef Unquoted_மதிப்பு,
                         SmallVectorImpl<எழுத்து> &Storage) {
    ப்ரதிஜ்ஞா(Unquoted_மதிப்பு.take_front(2) == "''");
    Storage.தள்ளு_back('\'');
    திரும்பு Unquoted_மதிப்பு.drop_front(2);
  };

  திரும்பு parse_Scalar_மதிப்பு(Unquoted_மதிப்பு, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_முனை::get_Plain_மதிப்பு(StringRef Raw_மதிப்பு,
                                    SmallVectorImpl<எழுத்து> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_மதிப்பு = Raw_மதிப்பு.rtrim("\r\n \t");
  திரும்பு parse_Scalar_மதிப்பு(Raw_மதிப்பு, Storage, "\r\n", nullptr);
}

முனை *சாவி_மதிப்பு_முனை::get_சாவி() {
  எனில் (சாவி)
    திரும்பு சாவி;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    எனில் (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_மதிப்பு
        || t.Kind == Token::TK_Error) {
      திரும்பு சாவி = புதிய (getAllocator()) Null_முனை(Doc);
    }
    எனில் (t.Kind == Token::TK_சாவி)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  எனில் (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_மதிப்பு) {
    திரும்பு சாவி = புதிய (getAllocator()) Null_முனை(Doc);
  }

  // We've got a normal key.
  திரும்பு சாவி = parse_Block_முனை();
}

முனை *சாவி_மதிப்பு_முனை::get_மதிப்பு() {
  எனில் (மதிப்பு)
    திரும்பு மதிப்பு;

  எனில் (முனை* சாவி = get_சாவி())
    சாவி->skip();
  இல்லையெனில் {
    setError("Null key in Key Value.", peekNext());
    திரும்பு மதிப்பு = புதிய (getAllocator()) Null_முனை(Doc);
  }

  எனில் (failed())
    திரும்பு மதிப்பு = புதிய (getAllocator()) Null_முனை(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    எனில் (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_சாவி
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      திரும்பு மதிப்பு = புதிய (getAllocator()) Null_முனை(Doc);
    }

    எனில் (t.Kind != Token::TK_மதிப்பு) {
      setError("Unexpected token in Key Value.", t);
      திரும்பு மதிப்பு = புதிய (getAllocator()) Null_முனை(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  எனில் (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_சாவி) {
    திரும்பு மதிப்பு = புதிய (getAllocator()) Null_முனை(Doc);
  }

  // We got a normal value.
  திரும்பு மதிப்பு = parse_Block_முனை();
}

வெறுமை Mapping_முனை::increment() {
  எனில் (failed()) {
    IsAtEnd = உண்மை;
    CurrentEntry = nullptr;
    திரும்பு;
  }
  எனில் (CurrentEntry) {
    CurrentEntry->skip();
    எனில் (Type == MT_Inline) {
      IsAtEnd = உண்மை;
      CurrentEntry = nullptr;
      திரும்பு;
    }
  }
  Token T = peekNext();
  எனில் (T.Kind == Token::TK_சாவி || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = புதிய (getAllocator()) சாவி_மதிப்பு_முனை(Doc);
  } இல்லையெனில் எனில் (Type == MT_Block) {
    தேர்வு (T.Kind) {
    நிலை Token::TK_BlockEnd:
      getNext();
      IsAtEnd = உண்மை;
      CurrentEntry = nullptr;
      நிறுத்து;
    இயல்பு:
      setError("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    நிலை Token::TK_Error:
      IsAtEnd = உண்மை;
      CurrentEntry = nullptr;
    }
  } இல்லையெனில் {
    தேர்வு (T.Kind) {
    நிலை Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      திரும்பு increment();
    நிலை Token::TK_FlowMappingEnd:
      getNext();
      [[fallthrough]];
    நிலை Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = உண்மை;
      CurrentEntry = nullptr;
      நிறுத்து;
    இயல்பு:
      setError( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      IsAtEnd = உண்மை;
      CurrentEntry = nullptr;
    }
  }
}

வெறுமை Sequence_முனை::increment() {
  எனில் (failed()) {
    IsAtEnd = உண்மை;
    CurrentEntry = nullptr;
    திரும்பு;
  }
  எனில் (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  எனில் (SeqType == ST_Block) {
    தேர்வு (T.Kind) {
    நிலை Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_முனை();
      எனில் (!CurrentEntry) { // An error occurred.
        IsAtEnd = உண்மை;
        CurrentEntry = nullptr;
      }
      நிறுத்து;
    நிலை Token::TK_BlockEnd:
      getNext();
      IsAtEnd = உண்மை;
      CurrentEntry = nullptr;
      நிறுத்து;
    இயல்பு:
      setError( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    நிலை Token::TK_Error:
      IsAtEnd = உண்மை;
      CurrentEntry = nullptr;
    }
  } இல்லையெனில் எனில் (SeqType == ST_Indentless) {
    தேர்வு (T.Kind) {
    நிலை Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_முனை();
      எனில் (!CurrentEntry) { // An error occurred.
        IsAtEnd = உண்மை;
        CurrentEntry = nullptr;
      }
      நிறுத்து;
    இயல்பு:
    நிலை Token::TK_Error:
      IsAtEnd = உண்மை;
      CurrentEntry = nullptr;
    }
  } இல்லையெனில் எனில் (SeqType == ST_Flow) {
    தேர்வு (T.Kind) {
    நிலை Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      WasPreviousTokenFlowEntry = உண்மை;
      திரும்பு increment();
    நிலை Token::TK_FlowSequenceEnd:
      getNext();
      [[fallthrough]];
    நிலை Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = உண்மை;
      CurrentEntry = nullptr;
      நிறுத்து;
    நிலை Token::TK_StreamEnd:
    நிலை Token::TK_DocumentEnd:
    நிலை Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = உண்மை;
      CurrentEntry = nullptr;
      நிறுத்து;
    இயல்பு:
      எனில் (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = உண்மை;
        CurrentEntry = nullptr;
        நிறுத்து;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parse_Block_முனை();
      எனில் (!CurrentEntry) {
        IsAtEnd = உண்மை;
      }
      WasPreviousTokenFlowEntry = பொய்;
      நிறுத்து;
    }
  }
}

Document::Document(Stream &S) : stream(S), வேர்_2(nullptr) {
  // Tag maps starts with two default mappings.
  TagMap["!"] = "!";
  TagMap["!!"] = "tag:yaml.org,2002:";

  எனில் (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peekNext();
  எனில் (T.Kind == Token::TK_DocumentStart)
    getNext();
}

தருக்க Document::skip()  {
  எனில் (stream.scanner->failed())
    திரும்பு பொய்;
  எனில் (!வேர்_2 && !get_வேர்())
    திரும்பு பொய்;
  வேர்_2->skip();
  Token &T = peekNext();
  எனில் (T.Kind == Token::TK_StreamEnd)
    திரும்பு பொய்;
  எனில் (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    திரும்பு skip();
  }
  திரும்பு உண்மை;
}

Token &Document::peekNext() {
  திரும்பு stream.scanner->peekNext();
}

Token Document::getNext() {
  திரும்பு stream.scanner->getNext();
}

வெறுமை Document::setError(மாறிலி Twine &Message, Token &Location) மாறிலி {
  stream.scanner->setError(Message, Location.Range.begin());
}

தருக்க Document::failed() மாறிலி {
  திரும்பு stream.scanner->failed();
}

முனை *Document::parse_Block_முனை() {
  Token T = peekNext();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  தேர்வு (T.Kind) {
  நிலை Token::TK_Alias:
    getNext();
    திரும்பு புதிய (முனை_Allocator) Alias_முனை(stream.CurrentDoc, T.Range.substr(1));
  நிலை Token::TK_Anchor:
    எனில் (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      திரும்பு nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    செல் parse_property;
  நிலை Token::TK_Tag:
    எனில் (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      திரும்பு nullptr;
    }
    TagInfo = getNext(); // Consume TK_Tag.
    T = peekNext();
    செல் parse_property;
  இயல்பு:
    நிறுத்து;
  }

  தேர்வு (T.Kind) {
  நிலை Token::TK_BlockEntry:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    திரும்பு புதிய (முனை_Allocator) Sequence_முனை( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , Sequence_முனை::ST_Indentless);
  நிலை Token::TK_BlockSequenceStart:
    getNext();
    திரும்பு புதிய (முனை_Allocator)
      Sequence_முனை( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_முனை::ST_Block);
  நிலை Token::TK_BlockMappingStart:
    getNext();
    திரும்பு புதிய (முனை_Allocator)
      Mapping_முனை( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_முனை::MT_Block);
  நிலை Token::TK_FlowSequenceStart:
    getNext();
    திரும்பு புதிய (முனை_Allocator)
      Sequence_முனை( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_முனை::ST_Flow);
  நிலை Token::TK_FlowMappingStart:
    getNext();
    திரும்பு புதிய (முனை_Allocator)
      Mapping_முனை( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_முனை::MT_Flow);
  நிலை Token::TK_Scalar:
    getNext();
    திரும்பு புதிய (முனை_Allocator)
      Scalar_முனை( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  நிலை Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.மதிப்பு.c_str(), T.மதிப்பு.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(முனை_Allocator).drop_back();
    திரும்பு புதிய (முனை_Allocator)
        Block_Scalar_முனை(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  நிலை Token::TK_சாவி:
    // Don't eat the TK_Key, KeyValueNode expects it.
    திரும்பு புதிய (முனை_Allocator)
      Mapping_முனை( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_முனை::MT_Inline);
  நிலை Token::TK_DocumentStart:
  நிலை Token::TK_DocumentEnd:
  நிலை Token::TK_StreamEnd:
  இயல்பு:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    திரும்பு புதிய (முனை_Allocator) Null_முனை(stream.CurrentDoc);
  நிலை Token::TK_FlowMappingEnd:
  நிலை Token::TK_FlowSequenceEnd:
  நிலை Token::TK_FlowEntry: {
    எனில் (வேர்_2 && (isa<Mapping_முனை>(வேர்_2) || isa<Sequence_முனை>(வேர்_2)))
      திரும்பு புதிய (முனை_Allocator) Null_முனை(stream.CurrentDoc);

    setError("Unexpected token", T);
    திரும்பு nullptr;
  }
  நிலை Token::TK_Error:
    திரும்பு nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  திரும்பு nullptr;
}

தருக்க Document::parseDirectives() {
  தருக்க isDirective = பொய்;
  போது (உண்மை) {
    Token T = peekNext();
    எனில் (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = உண்மை;
    } இல்லையெனில் எனில் (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = உண்மை;
    } இல்லையெனில் {
      நிறுத்து;
    }
  }
  திரும்பு isDirective;
}

வெறுமை Document::parseYAMLDirective() {
  getNext(); // Eat %YAML <version>
}

வெறுமை Document::parseTAGDirective() {
  Token Tag = getNext(); // %TAG <handle> <prefix>
  StringRef T = Tag.Range;
  // Strip %TAG
  T = T.substr(T.find_first_of(" \t")).ltrim(" \t");
  std::size_t HandleEnd = T.find_first_of(" \t");
  StringRef TagHandle = T.substr(0, HandleEnd);
  StringRef TagPrefix = T.substr(HandleEnd).ltrim(" \t");
  TagMap[TagHandle] = TagPrefix;
}

தருக்க Document::expectToken(முழு TK) {
  Token T = getNext();
  எனில் (T.Kind != TK) {
    setError("Unexpected token", T);
    திரும்பு பொய்;
  }
  திரும்பு உண்மை;
}
