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

#അന്തര്ഭാവ "llvm/Support/YAMLParser.h"
#അന്തര്ഭാവ "llvm/ADT/AllocatorList.h"
#അന്തര്ഭാവ "llvm/ADT/ArrayRef.h"
#അന്തര്ഭാവ "llvm/ADT/STLExtras.h"
#അന്തര്ഭാവ "llvm/ADT/SmallString.h"
#അന്തര്ഭാവ "llvm/ADT/SmallVector.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/ADT/Twine.h"
#അന്തര്ഭാവ "llvm/Support/Compiler.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
#അന്തര്ഭാവ "llvm/Support/MemoryBuffer.h"
#അന്തര്ഭാവ "llvm/Support/SMLoc.h"
#അന്തര്ഭാവ "llvm/Support/SourceMgr.h"
#അന്തര്ഭാവ "llvm/Support/Unicode.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <cassert>
#അന്തര്ഭാവ <cstddef>
#അന്തര്ഭാവ <cstdint>
#അന്തര്ഭാവ <map>
#അന്തര്ഭാവ <memory>
#അന്തര്ഭാവ <string>
#അന്തര്ഭാവ <system_error>
#അന്തര്ഭാവ <utility>

using നാമാകാശ llvm;
using നാമാകാശ yaml;

എണ്ണൽ UnicodeEncodingForm {
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
സ്ഥിതം EncodingInfo getUnicodeEncoding(StringRef നിവേശം) {
  എങ്കിൽ (നിവേശം.empty())
    തിരികെ {UEF_Unknown, 0};

  തിരഞ്ഞെടുപ്പ് (uint8_t(നിവേശം[0])) {
  അവസ്ഥ 0x00:
    എങ്കിൽ (നിവേശം.size() >= 4) {
      എങ്കിൽ (  നിവേശം[1] == 0
         && uint8_t(നിവേശം[2]) == 0xFE
         && uint8_t(നിവേശം[3]) == 0xFF)
        തിരികെ {UEF_UTF32_BE, 4};
      എങ്കിൽ (നിവേശം[1] == 0 && നിവേശം[2] == 0 && നിവേശം[3] != 0)
        തിരികെ {UEF_UTF32_BE, 0};
    }

    എങ്കിൽ (നിവേശം.size() >= 2 && നിവേശം[1] != 0)
      തിരികെ {UEF_UTF16_BE, 0};
    തിരികെ {UEF_Unknown, 0};
  അവസ്ഥ 0xFF:
    എങ്കിൽ (  നിവേശം.size() >= 4
       && uint8_t(നിവേശം[1]) == 0xFE
       && നിവേശം[2] == 0
       && നിവേശം[3] == 0)
      തിരികെ {UEF_UTF32_LE, 4};

    എങ്കിൽ (നിവേശം.size() >= 2 && uint8_t(നിവേശം[1]) == 0xFE)
      തിരികെ {UEF_UTF16_LE, 2};
    തിരികെ {UEF_Unknown, 0};
  അവസ്ഥ 0xFE:
    എങ്കിൽ (നിവേശം.size() >= 2 && uint8_t(നിവേശം[1]) == 0xFF)
      തിരികെ {UEF_UTF16_BE, 2};
    തിരികെ {UEF_Unknown, 0};
  അവസ്ഥ 0xEF:
    എങ്കിൽ (  നിവേശം.size() >= 3
       && uint8_t(നിവേശം[1]) == 0xBB
       && uint8_t(നിവേശം[2]) == 0xBF)
      തിരികെ {UEF_UTF8, 3};
    തിരികെ {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  എങ്കിൽ (നിവേശം.size() >= 4 && നിവേശം[1] == 0 && നിവേശം[2] == 0 && നിവേശം[3] == 0)
    തിരികെ {UEF_UTF32_LE, 0};

  എങ്കിൽ (നിവേശം.size() >= 2 && നിവേശം[1] == 0)
    തിരികെ {UEF_UTF16_LE, 0};

  തിരികെ {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
ശൂന്യം കണ്ണി::anchor() {}
ശൂന്യം Null_കണ്ണി::anchor() {}
ശൂന്യം Scalar_കണ്ണി::anchor() {}
ശൂന്യം Block_Scalar_കണ്ണി::anchor() {}
ശൂന്യം താക്കോൽ_മൂല്യം_കണ്ണി::anchor() {}
ശൂന്യം Mapping_കണ്ണി::anchor() {}
ശൂന്യം Sequence_കണ്ണി::anchor() {}
ശൂന്യം Alias_കണ്ണി::anchor() {}

നാമാകാശ llvm {
നാമാകാശ yaml {

/// Token - A single YAML token.
ഘടന Token {
  എണ്ണൽ TokenKind {
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
    TK_താക്കോൽ,
    TK_മൂല്യം,
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
  std::string മൂല്യം;

  Token() = സ്ഥിരസ്ഥിതി;
};

} // end namespace yaml
} // end namespace llvm

using TokenQueueT = BumpPtrList<Token>;

നാമാകാശ {

/// This struct is used to track simple keys.
///
/// Simple keys are handled by creating an entry in SimpleKeys for each Token
/// which could legally be the start of a simple key. When peekNext is called,
/// if the Token To be returned is referenced by a SimpleKey, we continue
/// tokenizing until that potential simple key has either been found to not be
/// a simple key (we moved on to the next line or went further than 1024 chars).
/// Or when we run into a Value, and then insert a Key token (and possibly
/// others) before the SimpleKey's Tok.
ഘടന Simple_താക്കോൽ {
  TokenQueueT::iterator Tok;
  unsigned Column = 0;
  unsigned വരി = 0;
  unsigned FlowLevel = 0;
  തർക്കം IsRequired = അസത്യം;

  തർക്കം operator ==(സ്ഥിരം Simple_താക്കോൽ &Other) {
    തിരികെ Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
using UTF8Decoded = std::pair<uint32_t, unsigned>;

സ്ഥിതം UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator End = Range.end();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  എങ്കിൽ (Position < End && (*Position & 0x80) == 0) {
    തിരികെ {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  എങ്കിൽ (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    എങ്കിൽ (codepoint >= 0x80)
      തിരികെ {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  എങ്കിൽ (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    എങ്കിൽ (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      തിരികെ {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  എങ്കിൽ (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    എങ്കിൽ (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      തിരികെ {codepoint, 4};
  }
  തിരികെ {0, 0};
}

നാമാകാശ llvm {
നാമാകാശ yaml {

/// Scans YAML tokens from a MemoryBuffer.
വർഗ്ഗം Scanner {
പൊതു:
  Scanner(StringRef നിവേശം, SourceMgr &SM, തർക്കം ShowColors = സത്യം,
          std::error_code *EC = nullptr);
  Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, തർക്കം ShowColors = സത്യം,
          std::error_code *EC = nullptr);

  /// Parse the next token and return it without popping it.
  Token &peekNext();

  /// Parse the next token and pop it from the queue.
  Token getNext();

  ശൂന്യം printError(SMLoc Loc, SourceMgr::DiagKind Kind, സ്ഥിരം Twine &Message,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.PrintMessage(Loc, Kind, Message, Ranges, /* FixIts= */ {}, ShowColors);
  }

  ശൂന്യം setError(സ്ഥിരം Twine &Message, StringRef::iterator Position) {
    എങ്കിൽ (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    എങ്കിൽ (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    എങ്കിൽ (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = സത്യം;
  }

  /// Returns true if an error occurred while parsing.
  തർക്കം failed() {
    തിരികെ Failed;
  }

സ്വകാര്യ:
  ശൂന്യം init(MemoryBufferRef Buffer);

  StringRef current_നിവേശം() {
    തിരികെ StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    തിരികെ ::decodeUTF8(StringRef(Position, End - Position));
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
  ശൂന്യം advanceWhile(SkipWhileFunc Func);

  /// Scan ns-uri-char[39]s starting at Cur.
  ///
  /// This updates Cur and Column while scanning.
  ശൂന്യം scan_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  തർക്കം consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  ശൂന്യം skip(uint32_t Distance);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  തർക്കം isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  തർക്കം isPlainSafeNonBlank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  തർക്കം is_വരി_Empty(StringRef വരി);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  തർക്കം consume_വരി_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  ശൂന്യം save_Simple_താക്കോൽ_Candidate( TokenQueueT::iterator Tok
                             , unsigned AtColumn
                             , തർക്കം IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  ശൂന്യം remove_Stale_Simple_താക്കോൽ_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  ശൂന്യം remove_Simple_താക്കോൽ_Candidates_On_Flow_Level(unsigned Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  തർക്കം unrollIndent(പൂർണ്ണസംഖ്യ ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  തർക്കം rollIndent( പൂർണ്ണസംഖ്യ ToColumn
                 , Token::TokenKind Kind
                 , TokenQueueT::iterator തിരുകുക_Point);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  ശൂന്യം skipComment();

  /// Skip whitespace and comments until the start of the next token.
  ശൂന്യം scanToNextToken();

  /// Must be the first token generated.
  തർക്കം scanStreamStart();

  /// Generate tokens needed to close out the stream.
  തർക്കം scanStreamEnd();

  /// Scan a %BLAH directive.
  തർക്കം scanDirective();

  /// Scan a ... or ---.
  തർക്കം scanDocumentIndicator(തർക്കം IsStart);

  /// Scan a [ or { and generate the proper flow collection start token.
  തർക്കം scanFlowCollectionStart(തർക്കം IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  തർക്കം scanFlowCollectionEnd(തർക്കം IsSequence);

  /// Scan the , that separates entries in a flow collection.
  തർക്കം scanFlowEntry();

  /// Scan the - that starts block sequence entries.
  തർക്കം scanBlockEntry();

  /// Scan an explicit ? indicating a key.
  തർക്കം scan_താക്കോൽ();

  /// Scan an explicit : indicating a value.
  തർക്കം scan_മൂല്യം();

  /// Scan a quoted scalar.
  തർക്കം scanFlowScalar(തർക്കം IsDoubleQuoted);

  /// Scan an unquoted scalar.
  തർക്കം scanPlainScalar();

  /// Scan an Alias or Anchor starting with * or &.
  തർക്കം scanAliasOrAnchor(തർക്കം IsAlias);

  /// Scan a block scalar starting with | or >.
  തർക്കം scanBlockScalar(തർക്കം IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  തർക്കം scanBlockScalarIndicators(അക്ഷരം &StyleIndicator, അക്ഷരം &ChompingIndicator,
                                 unsigned &IndentIndicator, തർക്കം &IsDone);

  /// Scan a style indicator in a block scalar header.
  അക്ഷരം scanBlockStyleIndicator();

  /// Scan a chomping indicator in a block scalar header.
  അക്ഷരം scanBlockChompingIndicator();

  /// Scan an indentation indicator in a block scalar header.
  unsigned scanBlockIndentationIndicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  തർക്കം scanBlockScalarHeader(അക്ഷരം &ChompingIndicator, unsigned &IndentIndicator,
                             തർക്കം &IsDone);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  തർക്കം findBlockScalarIndent(unsigned &BlockIndent, unsigned BlockExitIndent,
                             unsigned &വരി_Breaks, തർക്കം &IsDone);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  തർക്കം scanBlockScalarIndent(unsigned BlockIndent, unsigned BlockExitIndent,
                             തർക്കം &IsDone);

  /// Scan a tag of the form !stuff.
  തർക്കം scanTag();

  /// Dispatch to the next scanning function based on \a *Cur.
  തർക്കം fetchMoreTokens();

  /// The SourceMgr used for diagnostics and buffer management.
  SourceMgr &SM;

  /// The original input.
  MemoryBufferRef നിവേശം_Buffer;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator End;

  /// Current YAML indentation level in spaces.
  പൂർണ്ണസംഖ്യ Indent;

  /// Current column number in Unicode code points.
  unsigned Column;

  /// Current line number.
  unsigned വരി;

  /// How deep we are in flow style containers. 0 Means at block level.
  unsigned FlowLevel;

  /// Are we at the start of the stream?
  തർക്കം IsStartOfStream;

  /// Can the next token be the start of a simple key?
  തർക്കം Is_Simple_താക്കോൽ_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  തർക്കം Is_Adjacent_മൂല്യം_Allowed_In_Flow;

  /// True if an error has occurred.
  തർക്കം Failed;

  /// Should colors be used when printing out the diagnostic messages?
  തർക്കം ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  TokenQueueT TokenQueue;

  /// Indentation levels.
  SmallVector<പൂർണ്ണസംഖ്യ, 4> Indents;

  /// Potential simple keys.
  SmallVector<Simple_താക്കോൽ, 4> Simple_താക്കോലുകൾ;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
സ്ഥിതം ശൂന്യം encodeUTF8( uint32_t Unicode_Scalar_മൂല്യം
                      , SmallVectorImpl<അക്ഷരം> &Result) {
  എങ്കിൽ (Unicode_Scalar_മൂല്യം <= 0x7F) {
    Result.തള്ളുക_back(Unicode_Scalar_മൂല്യം & 0x7F);
  } അല്ലെങ്കിൽ എങ്കിൽ (Unicode_Scalar_മൂല്യം <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((Unicode_Scalar_മൂല്യം & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Unicode_Scalar_മൂല്യം & 0x3F);
    Result.തള്ളുക_back(FirstByte);
    Result.തള്ളുക_back(SecondByte);
  } അല്ലെങ്കിൽ എങ്കിൽ (Unicode_Scalar_മൂല്യം <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((Unicode_Scalar_മൂല്യം & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_മൂല്യം & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Unicode_Scalar_മൂല്യം & 0x3F);
    Result.തള്ളുക_back(FirstByte);
    Result.തള്ളുക_back(SecondByte);
    Result.തള്ളുക_back(ThirdByte);
  } അല്ലെങ്കിൽ എങ്കിൽ (Unicode_Scalar_മൂല്യം <= 0x10FFFF) {
    uint8_t FirstByte = 0xF0 | ((Unicode_Scalar_മൂല്യം & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_മൂല്യം & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Unicode_Scalar_മൂല്യം & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_മൂല്യം & 0x3F);
    Result.തള്ളുക_back(FirstByte);
    Result.തള്ളുക_back(SecondByte);
    Result.തള്ളുക_back(ThirdByte);
    Result.തള്ളുക_back(FourthByte);
  }
}

തർക്കം yaml::dumpTokens(StringRef നിവേശം, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(നിവേശം, SM);
  ആകുമ്പോൾ (സത്യം) {
    Token T = scanner.getNext();
    തിരഞ്ഞെടുപ്പ് (T.Kind) {
    അവസ്ഥ Token::TK_StreamStart:
      OS << "Stream-Start: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_StreamEnd:
      OS << "Stream-End: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_VersionDirective:
      OS << "Version-Directive: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_TagDirective:
      OS << "Tag-Directive: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_DocumentStart:
      OS << "Document-Start: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_DocumentEnd:
      OS << "Document-End: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_BlockEntry:
      OS << "Block-Entry: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_BlockEnd:
      OS << "Block-End: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_BlockSequenceStart:
      OS << "Block-Sequence-Start: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_BlockMappingStart:
      OS << "Block-Mapping-Start: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_FlowEntry:
      OS << "Flow-Entry: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_FlowSequenceStart:
      OS << "Flow-Sequence-Start: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_FlowSequenceEnd:
      OS << "Flow-Sequence-End: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_FlowMappingStart:
      OS << "Flow-Mapping-Start: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_FlowMappingEnd:
      OS << "Flow-Mapping-End: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_താക്കോൽ:
      OS << "Key: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_മൂല്യം:
      OS << "Value: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_Scalar:
      OS << "Scalar: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_BlockScalar:
      OS << "Block Scalar: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_Alias:
      OS << "Alias: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_Anchor:
      OS << "Anchor: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_Tag:
      OS << "Tag: ";
      നിർത്തുക;
    അവസ്ഥ Token::TK_Error:
      നിർത്തുക;
    }
    OS << T.Range << "\n";
    എങ്കിൽ (T.Kind == Token::TK_StreamEnd)
      നിർത്തുക;
    അല്ലെങ്കിൽ എങ്കിൽ (T.Kind == Token::TK_Error)
      തിരികെ അസത്യം;
  }
  തിരികെ സത്യം;
}

തർക്കം yaml::scanTokens(StringRef നിവേശം) {
  SourceMgr SM;
  Scanner scanner(നിവേശം, SM);
  ആകുമ്പോൾ (സത്യം) {
    Token T = scanner.getNext();
    എങ്കിൽ (T.Kind == Token::TK_StreamEnd)
      നിർത്തുക;
    അല്ലെങ്കിൽ എങ്കിൽ (T.Kind == Token::TK_Error)
      തിരികെ അസത്യം;
  }
  തിരികെ സത്യം;
}

std::string yaml::escape(StringRef നിവേശം, തർക്കം EscapePrintable) {
  std::string Escaped_നിവേശം;
  വേണ്ടി (StringRef::iterator i = നിവേശം.begin(), e = നിവേശം.end(); i != e; ++i) {
    എങ്കിൽ (*i == '\\')
      Escaped_നിവേശം += "\\\\";
    അല്ലെങ്കിൽ എങ്കിൽ (*i == '"')
      Escaped_നിവേശം += "\\\"";
    അല്ലെങ്കിൽ എങ്കിൽ (*i == 0)
      Escaped_നിവേശം += "\\0";
    അല്ലെങ്കിൽ എങ്കിൽ (*i == 0x07)
      Escaped_നിവേശം += "\\a";
    അല്ലെങ്കിൽ എങ്കിൽ (*i == 0x08)
      Escaped_നിവേശം += "\\b";
    അല്ലെങ്കിൽ എങ്കിൽ (*i == 0x09)
      Escaped_നിവേശം += "\\t";
    അല്ലെങ്കിൽ എങ്കിൽ (*i == 0x0A)
      Escaped_നിവേശം += "\\n";
    അല്ലെങ്കിൽ എങ്കിൽ (*i == 0x0B)
      Escaped_നിവേശം += "\\v";
    അല്ലെങ്കിൽ എങ്കിൽ (*i == 0x0C)
      Escaped_നിവേശം += "\\f";
    അല്ലെങ്കിൽ എങ്കിൽ (*i == 0x0D)
      Escaped_നിവേശം += "\\r";
    അല്ലെങ്കിൽ എങ്കിൽ (*i == 0x1B)
      Escaped_നിവേശം += "\\e";
    അല്ലെങ്കിൽ എങ്കിൽ ((unsigned അക്ഷരം)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_നിവേശം += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } അല്ലെങ്കിൽ എങ്കിൽ (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_മൂല്യം
        = decodeUTF8(StringRef(i, നിവേശം.end() - i));
      എങ്കിൽ (Unicode_Scalar_മൂല്യം.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_നിവേശം, Val);
        // FIXME: Error reporting.
        തിരികെ Escaped_നിവേശം;
      }
      എങ്കിൽ (Unicode_Scalar_മൂല്യം.first == 0x85)
        Escaped_നിവേശം += "\\N";
      അല്ലെങ്കിൽ എങ്കിൽ (Unicode_Scalar_മൂല്യം.first == 0xA0)
        Escaped_നിവേശം += "\\_";
      അല്ലെങ്കിൽ എങ്കിൽ (Unicode_Scalar_മൂല്യം.first == 0x2028)
        Escaped_നിവേശം += "\\L";
      അല്ലെങ്കിൽ എങ്കിൽ (Unicode_Scalar_മൂല്യം.first == 0x2029)
        Escaped_നിവേശം += "\\P";
      അല്ലെങ്കിൽ എങ്കിൽ (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_മൂല്യം.first))
        Escaped_നിവേശം += StringRef(i, Unicode_Scalar_മൂല്യം.second);
      അല്ലെങ്കിൽ {
        std::string HexStr = utohexstr(Unicode_Scalar_മൂല്യം.first);
        എങ്കിൽ (HexStr.size() <= 2)
          Escaped_നിവേശം += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        അല്ലെങ്കിൽ എങ്കിൽ (HexStr.size() <= 4)
          Escaped_നിവേശം += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        അല്ലെങ്കിൽ എങ്കിൽ (HexStr.size() <= 8)
          Escaped_നിവേശം += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += Unicode_Scalar_മൂല്യം.second - 1;
    } അല്ലെങ്കിൽ {
      Escaped_നിവേശം.തള്ളുക_back(*i);
    }
  }
  തിരികെ Escaped_നിവേശം;
}

std::optional<തർക്കം> yaml::parseBool(StringRef S) {
  തിരഞ്ഞെടുപ്പ് (S.size()) {
  അവസ്ഥ 1:
    തിരഞ്ഞെടുപ്പ് (S.front()) {
    അവസ്ഥ 'y':
    അവസ്ഥ 'Y':
      തിരികെ സത്യം;
    അവസ്ഥ 'n':
    അവസ്ഥ 'N':
      തിരികെ അസത്യം;
    സ്ഥിരസ്ഥിതി:
      തിരികെ std::nullopt;
    }
  അവസ്ഥ 2:
    തിരഞ്ഞെടുപ്പ് (S.front()) {
    അവസ്ഥ 'O':
      എങ്കിൽ (S[1] == 'N') // ON
        തിരികെ സത്യം;
      [[fallthrough]];
    അവസ്ഥ 'o':
      എങ്കിൽ (S[1] == 'n') //[Oo]n
        തിരികെ സത്യം;
      തിരികെ std::nullopt;
    അവസ്ഥ 'N':
      എങ്കിൽ (S[1] == 'O') // NO
        തിരികെ അസത്യം;
      [[fallthrough]];
    അവസ്ഥ 'n':
      എങ്കിൽ (S[1] == 'o') //[Nn]o
        തിരികെ അസത്യം;
      തിരികെ std::nullopt;
    സ്ഥിരസ്ഥിതി:
      തിരികെ std::nullopt;
    }
  അവസ്ഥ 3:
    തിരഞ്ഞെടുപ്പ് (S.front()) {
    അവസ്ഥ 'O':
      എങ്കിൽ (S.drop_front() == "FF") // OFF
        തിരികെ അസത്യം;
      [[fallthrough]];
    അവസ്ഥ 'o':
      എങ്കിൽ (S.drop_front() == "ff") //[Oo]ff
        തിരികെ അസത്യം;
      തിരികെ std::nullopt;
    അവസ്ഥ 'Y':
      എങ്കിൽ (S.drop_front() == "ES") // YES
        തിരികെ സത്യം;
      [[fallthrough]];
    അവസ്ഥ 'y':
      എങ്കിൽ (S.drop_front() == "es") //[Yy]es
        തിരികെ സത്യം;
      തിരികെ std::nullopt;
    സ്ഥിരസ്ഥിതി:
      തിരികെ std::nullopt;
    }
  അവസ്ഥ 4:
    തിരഞ്ഞെടുപ്പ് (S.front()) {
    അവസ്ഥ 'T':
      എങ്കിൽ (S.drop_front() == "RUE") // TRUE
        തിരികെ സത്യം;
      [[fallthrough]];
    അവസ്ഥ 't':
      എങ്കിൽ (S.drop_front() == "rue") //[Tt]rue
        തിരികെ സത്യം;
      തിരികെ std::nullopt;
    സ്ഥിരസ്ഥിതി:
      തിരികെ std::nullopt;
    }
  അവസ്ഥ 5:
    തിരഞ്ഞെടുപ്പ് (S.front()) {
    അവസ്ഥ 'F':
      എങ്കിൽ (S.drop_front() == "ALSE") // FALSE
        തിരികെ അസത്യം;
      [[fallthrough]];
    അവസ്ഥ 'f':
      എങ്കിൽ (S.drop_front() == "alse") //[Ff]alse
        തിരികെ അസത്യം;
      തിരികെ std::nullopt;
    സ്ഥിരസ്ഥിതി:
      തിരികെ std::nullopt;
    }
  സ്ഥിരസ്ഥിതി:
    തിരികെ std::nullopt;
  }
}

Scanner::Scanner(StringRef നിവേശം, SourceMgr &sm, തർക്കം ShowColors,
                 std::error_code *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(MemoryBufferRef(നിവേശം, "YAML"));
}

Scanner::Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, തർക്കം ShowColors,
                 std::error_code *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(Buffer);
}

ശൂന്യം Scanner::init(MemoryBufferRef Buffer) {
  നിവേശം_Buffer = Buffer;
  Current = നിവേശം_Buffer.getBufferStart();
  End = നിവേശം_Buffer.getBufferEnd();
  Indent = -1;
  Column = 0;
  വരി = 0;
  FlowLevel = 0;
  IsStartOfStream = സത്യം;
  Is_Simple_താക്കോൽ_Allowed = സത്യം;
  Is_Adjacent_മൂല്യം_Allowed_In_Flow = അസത്യം;
  Failed = അസത്യം;
  std::unique_ptr<MemoryBuffer> നിവേശം_Buffer_Owner =
      MemoryBuffer::getMemBuffer(Buffer, /*RequiresNullTerminator=*/അസത്യം);
  SM.AddNewSourceBuffer(std::move(നിവേശം_Buffer_Owner), SMLoc());
}

Token &Scanner::peekNext() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  തർക്കം NeedMore = അസത്യം;
  ആകുമ്പോൾ (സത്യം) {
    എങ്കിൽ (TokenQueue.empty() || NeedMore) {
      എങ്കിൽ (!fetchMoreTokens()) {
        TokenQueue.clear();
        Simple_താക്കോലുകൾ.clear();
        TokenQueue.തള്ളുക_back(Token());
        തിരികെ TokenQueue.front();
      }
    }
    പ്രതിജ്ഞാ(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_താക്കോൽ_Candidates();
    Simple_താക്കോൽ SK;
    SK.Tok = TokenQueue.begin();
    എങ്കിൽ (!is_contained(Simple_താക്കോലുകൾ, SK))
      നിർത്തുക;
    അല്ലെങ്കിൽ
      NeedMore = സത്യം;
  }
  തിരികെ TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  എങ്കിൽ (!TokenQueue.empty())
    TokenQueue.എടുക്കുക_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  എങ്കിൽ (TokenQueue.empty())
    TokenQueue.resetAlloc();

  തിരികെ Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  എങ്കിൽ (Position == End)
    തിരികെ Position;
  // Check 7 bit c-printable - b-char.
  എങ്കിൽ (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    തിരികെ Position + 1;

  // Check for valid UTF-8.
  എങ്കിൽ (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    എങ്കിൽ (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      തിരികെ Position + u8d.second;
  }
  തിരികെ Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  എങ്കിൽ (Position == End)
    തിരികെ Position;
  എങ്കിൽ (*Position == 0x0D) {
    എങ്കിൽ (Position + 1 != End && *(Position + 1) == 0x0A)
      തിരികെ Position + 2;
    തിരികെ Position + 1;
  }

  എങ്കിൽ (*Position == 0x0A)
    തിരികെ Position + 1;
  തിരികെ Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  എങ്കിൽ (Position == End)
    തിരികെ Position;
  എങ്കിൽ (*Position == ' ')
    തിരികെ Position + 1;
  തിരികെ Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  എങ്കിൽ (Position == End)
    തിരികെ Position;
  എങ്കിൽ (*Position == ' ' || *Position == '\t')
    തിരികെ Position + 1;
  തിരികെ Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  എങ്കിൽ (Position == End)
    തിരികെ Position;
  എങ്കിൽ (*Position == ' ' || *Position == '\t')
    തിരികെ Position;
  തിരികെ skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  ആകുമ്പോൾ (സത്യം) {
    StringRef::iterator i = (ഇത്->*Func)(Position);
    എങ്കിൽ (i == Position)
      നിർത്തുക;
    Position = i;
  }
  തിരികെ Position;
}

ശൂന്യം Scanner::advanceWhile(SkipWhileFunc Func) {
  auto Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

സ്ഥിതം തർക്കം is_ns_hex_digit(സ്ഥിരം അക്ഷരം C) { തിരികെ isAlnum(C); }

സ്ഥിതം തർക്കം is_ns_word_char(സ്ഥിരം അക്ഷരം C) { തിരികെ C == '-' || isAlpha(C); }

ശൂന്യം Scanner::scan_ns_uri_char() {
  ആകുമ്പോൾ (സത്യം) {
    എങ്കിൽ (Current == End)
      നിർത്തുക;
    എങ്കിൽ ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } അല്ലെങ്കിൽ {
      നിർത്തുക;
    }
  }
}

തർക്കം Scanner::consume(uint32_t Expected) {
  എങ്കിൽ (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    തിരികെ അസത്യം;
  }
  എങ്കിൽ (Current == End)
    തിരികെ അസത്യം;
  എങ്കിൽ (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    തിരികെ അസത്യം;
  }
  എങ്കിൽ (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    തിരികെ സത്യം;
  }
  തിരികെ അസത്യം;
}

ശൂന്യം Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  പ്രതിജ്ഞാ(Current <= End && "Skipped past the end");
}

തർക്കം Scanner::isBlankOrBreak(StringRef::iterator Position) {
  എങ്കിൽ (Position == End)
    തിരികെ അസത്യം;
  തിരികെ *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

തർക്കം Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  എങ്കിൽ (Position == End || isBlankOrBreak(Position))
    തിരികെ അസത്യം;
  എങ്കിൽ (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    തിരികെ അസത്യം;
  തിരികെ സത്യം;
}

തർക്കം Scanner::is_വരി_Empty(StringRef വരി) {
  വേണ്ടി (സ്ഥിരം auto *Position = വരി.begin(); Position != വരി.end(); ++Position)
    എങ്കിൽ (!isBlankOrBreak(Position))
      തിരികെ അസത്യം;
  തിരികെ സത്യം;
}

തർക്കം Scanner::consume_വരി_Break_If_Present() {
  auto Next = skip_b_break(Current);
  എങ്കിൽ (Next == Current)
    തിരികെ അസത്യം;
  Column = 0;
  ++വരി;
  Current = Next;
  തിരികെ സത്യം;
}

ശൂന്യം Scanner::save_Simple_താക്കോൽ_Candidate( TokenQueueT::iterator Tok
                                    , unsigned AtColumn
                                    , തർക്കം IsRequired) {
  എങ്കിൽ (Is_Simple_താക്കോൽ_Allowed) {
    Simple_താക്കോൽ SK;
    SK.Tok = Tok;
    SK.വരി = വരി;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_താക്കോലുകൾ.തള്ളുക_back(SK);
  }
}

ശൂന്യം Scanner::remove_Stale_Simple_താക്കോൽ_Candidates() {
  വേണ്ടി (SmallVectorImpl<Simple_താക്കോൽ>::iterator i = Simple_താക്കോലുകൾ.begin();
                                            i != Simple_താക്കോലുകൾ.end();) {
    എങ്കിൽ (i->വരി != വരി || i->Column + 1024 < Column) {
      എങ്കിൽ (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_താക്കോലുകൾ.erase(i);
    } അല്ലെങ്കിൽ {
      ++i;
    }
  }
}

ശൂന്യം Scanner::remove_Simple_താക്കോൽ_Candidates_On_Flow_Level(unsigned Level) {
  എങ്കിൽ (!Simple_താക്കോലുകൾ.empty() && (Simple_താക്കോലുകൾ.end() - 1)->FlowLevel == Level)
    Simple_താക്കോലുകൾ.എടുക്കുക_back();
}

തർക്കം Scanner::unrollIndent(പൂർണ്ണസംഖ്യ ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  എങ്കിൽ (FlowLevel != 0)
    തിരികെ സത്യം;

  ആകുമ്പോൾ (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.തള്ളുക_back(T);
    Indent = Indents.എടുക്കുക_back_val();
  }

  തിരികെ സത്യം;
}

തർക്കം Scanner::rollIndent( പൂർണ്ണസംഖ്യ ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator തിരുകുക_Point) {
  എങ്കിൽ (FlowLevel)
    തിരികെ സത്യം;
  എങ്കിൽ (Indent < ToColumn) {
    Indents.തള്ളുക_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.തിരുകുക(തിരുകുക_Point, T);
  }
  തിരികെ സത്യം;
}

ശൂന്യം Scanner::skipComment() {
  എങ്കിൽ (Current == End || *Current != '#')
    തിരികെ;
  ആകുമ്പോൾ (സത്യം) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    എങ്കിൽ (I == Current)
      നിർത്തുക;
    Current = I;
    ++Column;
  }
}

ശൂന്യം Scanner::scanToNextToken() {
  ആകുമ്പോൾ (സത്യം) {
    ആകുമ്പോൾ (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    എങ്കിൽ (i == Current)
      നിർത്തുക;
    Current = i;
    ++വരി;
    Column = 0;
    // New lines may start a simple key.
    എങ്കിൽ (!FlowLevel)
      Is_Simple_താക്കോൽ_Allowed = സത്യം;
  }
}

തർക്കം Scanner::scanStreamStart() {
  IsStartOfStream = അസത്യം;

  EncodingInfo EI = getUnicodeEncoding(current_നിവേശം());

  Token T;
  T.Kind = Token::TK_StreamStart;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.തള്ളുക_back(T);
  Current += EI.second;
  തിരികെ സത്യം;
}

തർക്കം Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  എങ്കിൽ (Column != 0) {
    Column = 0;
    ++വരി;
  }

  unrollIndent(-1);
  Simple_താക്കോലുകൾ.clear();
  Is_Simple_താക്കോൽ_Allowed = അസത്യം;
  Is_Adjacent_മൂല്യം_Allowed_In_Flow = അസത്യം;

  Token T;
  T.Kind = Token::TK_StreamEnd;
  T.Range = StringRef(Current, 0);
  TokenQueue.തള്ളുക_back(T);
  തിരികെ സത്യം;
}

തർക്കം Scanner::scanDirective() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_താക്കോലുകൾ.clear();
  Is_Simple_താക്കോൽ_Allowed = അസത്യം;
  Is_Adjacent_മൂല്യം_Allowed_In_Flow = അസത്യം;

  StringRef::iterator Start = Current;
  consume('%');
  StringRef::iterator NameStart = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(NameStart, Current - NameStart);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  എങ്കിൽ (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.തള്ളുക_back(T);
    തിരികെ സത്യം;
  } അല്ലെങ്കിൽ എങ്കിൽ(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.തള്ളുക_back(T);
    തിരികെ സത്യം;
  }
  തിരികെ അസത്യം;
}

തർക്കം Scanner::scanDocumentIndicator(തർക്കം IsStart) {
  unrollIndent(-1);
  Simple_താക്കോലുകൾ.clear();
  Is_Simple_താക്കോൽ_Allowed = അസത്യം;
  Is_Adjacent_മൂല്യം_Allowed_In_Flow = അസത്യം;

  Token T;
  T.Kind = IsStart ? Token::TK_DocumentStart : Token::TK_DocumentEnd;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.തള്ളുക_back(T);
  തിരികെ സത്യം;
}

തർക്കം Scanner::scanFlowCollectionStart(തർക്കം IsSequence) {
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.തള്ളുക_back(T);

  // [ and { may begin a simple key.
  save_Simple_താക്കോൽ_Candidate(--TokenQueue.end(), Column - 1, അസത്യം);

  // And may also be followed by a simple key.
  Is_Simple_താക്കോൽ_Allowed = സത്യം;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_മൂല്യം_Allowed_In_Flow = അസത്യം;
  ++FlowLevel;
  തിരികെ സത്യം;
}

തർക്കം Scanner::scanFlowCollectionEnd(തർക്കം IsSequence) {
  remove_Simple_താക്കോൽ_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_താക്കോൽ_Allowed = അസത്യം;
  Is_Adjacent_മൂല്യം_Allowed_In_Flow = സത്യം;
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceEnd
                      : Token::TK_FlowMappingEnd;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.തള്ളുക_back(T);
  എങ്കിൽ (FlowLevel)
    --FlowLevel;
  തിരികെ സത്യം;
}

തർക്കം Scanner::scanFlowEntry() {
  remove_Simple_താക്കോൽ_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_താക്കോൽ_Allowed = സത്യം;
  Is_Adjacent_മൂല്യം_Allowed_In_Flow = അസത്യം;
  Token T;
  T.Kind = Token::TK_FlowEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.തള്ളുക_back(T);
  തിരികെ സത്യം;
}

തർക്കം Scanner::scanBlockEntry() {
  rollIndent(Column, Token::TK_BlockSequenceStart, TokenQueue.end());
  remove_Simple_താക്കോൽ_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_താക്കോൽ_Allowed = സത്യം;
  Is_Adjacent_മൂല്യം_Allowed_In_Flow = അസത്യം;
  Token T;
  T.Kind = Token::TK_BlockEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.തള്ളുക_back(T);
  തിരികെ സത്യം;
}

തർക്കം Scanner::scan_താക്കോൽ() {
  എങ്കിൽ (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  remove_Simple_താക്കോൽ_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_താക്കോൽ_Allowed = !FlowLevel;
  Is_Adjacent_മൂല്യം_Allowed_In_Flow = അസത്യം;

  Token T;
  T.Kind = Token::TK_താക്കോൽ;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.തള്ളുക_back(T);
  തിരികെ സത്യം;
}

തർക്കം Scanner::scan_മൂല്യം() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  എങ്കിൽ (!Simple_താക്കോലുകൾ.empty()) {
    Simple_താക്കോൽ SK = Simple_താക്കോലുകൾ.എടുക്കുക_back_val();
    Token T;
    T.Kind = Token::TK_താക്കോൽ;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    വേണ്ടി (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      എങ്കിൽ (i == SK.Tok)
        നിർത്തുക;
    }
    എങ്കിൽ (i == e) {
      Failed = സത്യം;
      തിരികെ അസത്യം;
    }
    i = TokenQueue.തിരുകുക(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    Is_Simple_താക്കോൽ_Allowed = അസത്യം;
  } അല്ലെങ്കിൽ {
    എങ്കിൽ (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    Is_Simple_താക്കോൽ_Allowed = !FlowLevel;
  }
  Is_Adjacent_മൂല്യം_Allowed_In_Flow = അസത്യം;

  Token T;
  T.Kind = Token::TK_മൂല്യം;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.തള്ളുക_back(T);
  തിരികെ സത്യം;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE സ്ഥിതം തർക്കം
wasEscaped(StringRef::iterator First, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
സ്ഥിതം തർക്കം wasEscaped(StringRef::iterator First,
                       StringRef::iterator Position) {
  പ്രതിജ്ഞാ(Position - 1 >= First);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  ആകുമ്പോൾ (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  തിരികെ (Position - 1 - I) % 2 == 1;
}

തർക്കം Scanner::scanFlowScalar(തർക്കം IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  എങ്കിൽ (IsDoubleQuoted) {
    ചെയ്യുക {
      ++Current;
      ആകുമ്പോൾ (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } ആകുമ്പോൾ (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } അല്ലെങ്കിൽ {
    skip(1);
    ആകുമ്പോൾ (Current != End) {
      // Skip a ' followed by another '.
      എങ്കിൽ (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        തുടരുക;
      } അല്ലെങ്കിൽ എങ്കിൽ (*Current == '\'')
        നിർത്തുക;
      StringRef::iterator i = skip_nb_char(Current);
      എങ്കിൽ (i == Current) {
        i = skip_b_break(Current);
        എങ്കിൽ (i == Current)
          നിർത്തുക;
        Current = i;
        Column = 0;
        ++വരി;
      } അല്ലെങ്കിൽ {
        എങ്കിൽ (i == End)
          നിർത്തുക;
        Current = i;
        ++Column;
      }
    }
  }

  എങ്കിൽ (Current == End) {
    setError("Expected quote at end of scalar", Current);
    തിരികെ അസത്യം;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.തള്ളുക_back(T);

  save_Simple_താക്കോൽ_Candidate(--TokenQueue.end(), ColStart, അസത്യം);

  Is_Simple_താക്കോൽ_Allowed = അസത്യം;
  Is_Adjacent_മൂല്യം_Allowed_In_Flow = സത്യം;

  തിരികെ സത്യം;
}

തർക്കം Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  unsigned LeadingBlanks = 0;
  പ്രതിജ്ഞാ(Indent >= -1 && "Indent must be >= -1 !");
  unsigned indent = static_cast<unsigned>(Indent + 1);
  ആകുമ്പോൾ (Current != End) {
    എങ്കിൽ (*Current == '#')
      നിർത്തുക;

    ആകുമ്പോൾ (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      എങ്കിൽ (i == Current)
        നിർത്തുക;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    എങ്കിൽ (!isBlankOrBreak(Current))
      നിർത്തുക;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    ആകുമ്പോൾ (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      എങ്കിൽ (i != Tmp) {
        എങ്കിൽ (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          തിരികെ അസത്യം;
        }
        Tmp = i;
        ++Column;
      } അല്ലെങ്കിൽ {
        i = skip_b_break(Tmp);
        എങ്കിൽ (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++വരി;
      }
    }

    എങ്കിൽ (!FlowLevel && Column < indent)
      നിർത്തുക;

    Current = Tmp;
  }
  എങ്കിൽ (Start == Current) {
    setError("Got empty plain scalar", Start);
    തിരികെ അസത്യം;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.തള്ളുക_back(T);

  // Plain scalars can be simple keys.
  save_Simple_താക്കോൽ_Candidate(--TokenQueue.end(), ColStart, അസത്യം);

  Is_Simple_താക്കോൽ_Allowed = അസത്യം;
  Is_Adjacent_മൂല്യം_Allowed_In_Flow = അസത്യം;

  തിരികെ സത്യം;
}

തർക്കം Scanner::scanAliasOrAnchor(തർക്കം IsAlias) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1);
  ആകുമ്പോൾ (Current != End) {
    എങ്കിൽ (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      നിർത്തുക;
    StringRef::iterator i = skip_ns_char(Current);
    എങ്കിൽ (i == Current)
      നിർത്തുക;
    Current = i;
    ++Column;
  }

  എങ്കിൽ (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    തിരികെ അസത്യം;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.തള്ളുക_back(T);

  // Alias and anchors can be simple keys.
  save_Simple_താക്കോൽ_Candidate(--TokenQueue.end(), ColStart, അസത്യം);

  Is_Simple_താക്കോൽ_Allowed = അസത്യം;
  Is_Adjacent_മൂല്യം_Allowed_In_Flow = അസത്യം;

  തിരികെ സത്യം;
}

തർക്കം Scanner::scanBlockScalarIndicators(അക്ഷരം &StyleIndicator,
                                        അക്ഷരം &ChompingIndicator,
                                        unsigned &IndentIndicator,
                                        തർക്കം &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  എങ്കിൽ (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    തിരികെ അസത്യം;
  തിരികെ സത്യം;
}

അക്ഷരം Scanner::scanBlockStyleIndicator() {
  അക്ഷരം Indicator = ' ';
  എങ്കിൽ (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  തിരികെ Indicator;
}

അക്ഷരം Scanner::scanBlockChompingIndicator() {
  അക്ഷരം Indicator = ' ';
  എങ്കിൽ (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  തിരികെ Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
സ്ഥിതം unsigned get_Chomped_വരി_Breaks(അക്ഷരം ChompingIndicator,
                                     unsigned വരി_Breaks, StringRef Str) {
  എങ്കിൽ (ChompingIndicator == '-') // Strip all line breaks.
    തിരികെ 0;
  എങ്കിൽ (ChompingIndicator == '+') // Keep all line breaks.
    തിരികെ വരി_Breaks;
  // Clip trailing lines.
  തിരികെ Str.empty() ? 0 : 1;
}

unsigned Scanner::scanBlockIndentationIndicator() {
  unsigned Indent = 0;
  എങ്കിൽ (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = unsigned(*Current - '0');
    skip(1);
  }
  തിരികെ Indent;
}

തർക്കം Scanner::scanBlockScalarHeader(അക്ഷരം &ChompingIndicator,
                                    unsigned &IndentIndicator, തർക്കം &IsDone) {
  auto Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  എങ്കിൽ (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  എങ്കിൽ (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.തള്ളുക_back(T);
    IsDone = സത്യം;
    തിരികെ സത്യം;
  }

  എങ്കിൽ (!consume_വരി_Break_If_Present()) {
    setError("Expected a line break after block scalar header", Current);
    തിരികെ അസത്യം;
  }
  തിരികെ സത്യം;
}

തർക്കം Scanner::findBlockScalarIndent(unsigned &BlockIndent,
                                    unsigned BlockExitIndent,
                                    unsigned &വരി_Breaks, തർക്കം &IsDone) {
  unsigned Max_All_Space_വരി_Characters = 0;
  StringRef::iterator Longest_All_Space_വരി;

  ആകുമ്പോൾ (സത്യം) {
    advanceWhile(&Scanner::skip_s_space);
    എങ്കിൽ (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      എങ്കിൽ (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = സത്യം;
        തിരികെ സത്യം;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      എങ്കിൽ (Max_All_Space_വരി_Characters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_വരി);
        തിരികെ അസത്യം;
      }
      തിരികെ സത്യം;
    }
    എങ്കിൽ (skip_b_break(Current) != Current &&
        Column > Max_All_Space_വരി_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_വരി_Characters = Column;
      Longest_All_Space_വരി = Current;
    }

    // Check for EOF.
    എങ്കിൽ (Current == End) {
      IsDone = സത്യം;
      തിരികെ സത്യം;
    }

    എങ്കിൽ (!consume_വരി_Break_If_Present()) {
      IsDone = സത്യം;
      തിരികെ സത്യം;
    }
    ++വരി_Breaks;
  }
  തിരികെ സത്യം;
}

തർക്കം Scanner::scanBlockScalarIndent(unsigned BlockIndent,
                                    unsigned BlockExitIndent, തർക്കം &IsDone) {
  // Skip the indentation.
  ആകുമ്പോൾ (Column < BlockIndent) {
    auto I = skip_s_space(Current);
    എങ്കിൽ (I == Current)
      നിർത്തുക;
    Current = I;
    ++Column;
  }

  എങ്കിൽ (skip_nb_char(Current) == Current)
    തിരികെ സത്യം;

  എങ്കിൽ (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = സത്യം;
    തിരികെ സത്യം;
  }

  എങ്കിൽ (Column < BlockIndent) {
    എങ്കിൽ (Current != End && *Current == '#') { // Trailing comment.
      IsDone = സത്യം;
      തിരികെ സത്യം;
    }
    setError("A text line is less indented than the block scalar", Current);
    തിരികെ അസത്യം;
  }
  തിരികെ സത്യം; // A normal text line.
}

തർക്കം Scanner::scanBlockScalar(തർക്കം IsLiteral) {
  പ്രതിജ്ഞാ(*Current == '|' || *Current == '>');
  അക്ഷരം StyleIndicator;
  അക്ഷരം ChompingIndicator;
  unsigned BlockIndent;
  തർക്കം IsDone = അസത്യം;
  എങ്കിൽ (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    തിരികെ അസത്യം;
  എങ്കിൽ (IsDone)
    തിരികെ സത്യം;
  തർക്കം IsFolded = StyleIndicator == '>';

  സ്ഥിരം auto *Start = Current;
  unsigned BlockExitIndent = Indent < 0 ? 0 : (unsigned)Indent;
  unsigned വരി_Breaks = 0;
  എങ്കിൽ (BlockIndent == 0) {
    എങ്കിൽ (!findBlockScalarIndent(BlockIndent, BlockExitIndent, വരി_Breaks,
                               IsDone))
      തിരികെ അസത്യം;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  ആകുമ്പോൾ (!IsDone) {
    എങ്കിൽ (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      തിരികെ അസത്യം;
    എങ്കിൽ (IsDone)
      നിർത്തുക;

    // Parse the current line.
    auto വരി_Start = Current;
    advanceWhile(&Scanner::skip_nb_char);
    എങ്കിൽ (വരി_Start != Current) {
      എങ്കിൽ (വരി_Breaks && IsFolded && !Scanner::is_വരി_Empty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        എങ്കിൽ (വരി_Breaks == 1) {
          Str.append(വരി_Breaks,
                     is_വരി_Empty(StringRef(വരി_Start, Current - വരി_Start))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        വരി_Breaks--;
      }
      Str.append(വരി_Breaks, '\n');
      Str.append(StringRef(വരി_Start, Current - വരി_Start));
      വരി_Breaks = 0;
    }

    // Check for EOF.
    എങ്കിൽ (Current == End)
      നിർത്തുക;

    എങ്കിൽ (!consume_വരി_Break_If_Present())
      നിർത്തുക;
    ++വരി_Breaks;
  }

  എങ്കിൽ (Current == End && !വരി_Breaks)
    // Ensure that there is at least one line break before the end of file.
    വരി_Breaks = 1;
  Str.append(get_Chomped_വരി_Breaks(ChompingIndicator, വരി_Breaks, Str), '\n');

  // New lines may start a simple key.
  എങ്കിൽ (!FlowLevel)
    Is_Simple_താക്കോൽ_Allowed = സത്യം;
  Is_Adjacent_മൂല്യം_Allowed_In_Flow = അസത്യം;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.മൂല്യം = std::string(Str);
  TokenQueue.തള്ളുക_back(T);
  തിരികെ സത്യം;
}

തർക്കം Scanner::scanTag() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1); // Eat !.
  എങ്കിൽ (Current == End || isBlankOrBreak(Current)); // An empty tag.
  അല്ലെങ്കിൽ എങ്കിൽ (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    എങ്കിൽ (!consume('>'))
      തിരികെ അസത്യം;
  } അല്ലെങ്കിൽ {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.Kind = Token::TK_Tag;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.തള്ളുക_back(T);

  // Tags can be simple keys.
  save_Simple_താക്കോൽ_Candidate(--TokenQueue.end(), ColStart, അസത്യം);

  Is_Simple_താക്കോൽ_Allowed = അസത്യം;
  Is_Adjacent_മൂല്യം_Allowed_In_Flow = അസത്യം;

  തിരികെ സത്യം;
}

തർക്കം Scanner::fetchMoreTokens() {
  എങ്കിൽ (IsStartOfStream)
    തിരികെ scanStreamStart();

  scanToNextToken();

  എങ്കിൽ (Current == End)
    തിരികെ scanStreamEnd();

  remove_Stale_Simple_താക്കോൽ_Candidates();

  unrollIndent(Column);

  എങ്കിൽ (Column == 0 && *Current == '%')
    തിരികെ scanDirective();

  എങ്കിൽ (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    തിരികെ scanDocumentIndicator(സത്യം);

  എങ്കിൽ (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    തിരികെ scanDocumentIndicator(അസത്യം);

  എങ്കിൽ (*Current == '[')
    തിരികെ scanFlowCollectionStart(സത്യം);

  എങ്കിൽ (*Current == '{')
    തിരികെ scanFlowCollectionStart(അസത്യം);

  എങ്കിൽ (*Current == ']')
    തിരികെ scanFlowCollectionEnd(സത്യം);

  എങ്കിൽ (*Current == '}')
    തിരികെ scanFlowCollectionEnd(അസത്യം);

  എങ്കിൽ (*Current == ',')
    തിരികെ scanFlowEntry();

  എങ്കിൽ (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    തിരികെ scanBlockEntry();

  എങ്കിൽ (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    തിരികെ scan_താക്കോൽ();

  എങ്കിൽ (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || Is_Adjacent_മൂല്യം_Allowed_In_Flow))
    തിരികെ scan_മൂല്യം();

  എങ്കിൽ (*Current == '*')
    തിരികെ scanAliasOrAnchor(സത്യം);

  എങ്കിൽ (*Current == '&')
    തിരികെ scanAliasOrAnchor(അസത്യം);

  എങ്കിൽ (*Current == '!')
    തിരികെ scanTag();

  എങ്കിൽ (*Current == '|' && !FlowLevel)
    തിരികെ scanBlockScalar(സത്യം);

  എങ്കിൽ (*Current == '>' && !FlowLevel)
    തിരികെ scanBlockScalar(അസത്യം);

  എങ്കിൽ (*Current == '\'')
    തിരികെ scanFlowScalar(അസത്യം);

  എങ്കിൽ (*Current == '"')
    തിരികെ scanFlowScalar(സത്യം);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  എങ്കിൽ ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    തിരികെ scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  തിരികെ അസത്യം;
}

Stream::Stream(StringRef നിവേശം, SourceMgr &SM, തർക്കം ShowColors,
               std::error_code *EC)
    : scanner(പുതിയ Scanner(നിവേശം, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef നിവേശം_Buffer, SourceMgr &SM, തർക്കം ShowColors,
               std::error_code *EC)
    : scanner(പുതിയ Scanner(നിവേശം_Buffer, SM, ShowColors, EC)) {}

Stream::~Stream() = സ്ഥിരസ്ഥിതി;

തർക്കം Stream::failed() { തിരികെ scanner->failed(); }

ശൂന്യം Stream::printError(കണ്ണി *N, സ്ഥിരം Twine &സന്ദേശം_2, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), സന്ദേശം_2, Kind);
}

ശൂന്യം Stream::printError(സ്ഥിരം SMRange &Range, സ്ഥിരം Twine &സന്ദേശം_2,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, സന്ദേശം_2, Range);
}

document_iterator Stream::begin() {
  എങ്കിൽ (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(പുതിയ Document(*ഇത്));
  തിരികെ document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  തിരികെ document_iterator();
}

ശൂന്യം Stream::skip() {
  വേണ്ടി (Document &Doc : *ഇത്)
    Doc.skip();
}

കണ്ണി::കണ്ണി(unsigned പൂർണ്ണസംഖ്യ Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string കണ്ണി::getVerbatimTag() സ്ഥിരം {
  StringRef Raw = getRawTag();
  എങ്കിൽ (!Raw.empty() && Raw != "!") {
    std::string Ret;
    എങ്കിൽ (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      തിരികെ Ret;
    } അല്ലെങ്കിൽ എങ്കിൽ (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      തിരികെ Ret;
    } അല്ലെങ്കിൽ {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      എങ്കിൽ (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      അല്ലെങ്കിൽ {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      തിരികെ Ret;
    }
  }

  തിരഞ്ഞെടുപ്പ് (getType()) {
  അവസ്ഥ NK_Null:
    തിരികെ "tag:yaml.org,2002:null";
  അവസ്ഥ NK_Scalar:
  അവസ്ഥ NK_BlockScalar:
    // TODO: Tag resolution.
    തിരികെ "tag:yaml.org,2002:str";
  അവസ്ഥ NK_Mapping:
    തിരികെ "tag:yaml.org,2002:map";
  അവസ്ഥ NK_Sequence:
    തിരികെ "tag:yaml.org,2002:seq";
  }

  തിരികെ "";
}

Token &കണ്ണി::peekNext() {
  തിരികെ Doc->peekNext();
}

Token കണ്ണി::getNext() {
  തിരികെ Doc->getNext();
}

കണ്ണി *കണ്ണി::parse_Block_കണ്ണി() {
  തിരികെ Doc->parse_Block_കണ്ണി();
}

BumpPtrAllocator &കണ്ണി::getAllocator() {
  തിരികെ Doc->കണ്ണി_Allocator;
}

ശൂന്യം കണ്ണി::setError(സ്ഥിരം Twine &സന്ദേശം_2, Token &Tok) സ്ഥിരം {
  Doc->setError(സന്ദേശം_2, Tok);
}

തർക്കം കണ്ണി::failed() സ്ഥിരം {
  തിരികെ Doc->failed();
}

StringRef Scalar_കണ്ണി::get_മൂല്യം(SmallVectorImpl<അക്ഷരം> &Storage) സ്ഥിരം {
  എങ്കിൽ (മൂല്യം[0] == '"')
    തിരികെ get_Double_Quoted_മൂല്യം(മൂല്യം, Storage);
  എങ്കിൽ (മൂല്യം[0] == '\'')
    തിരികെ get_Single_Quoted_മൂല്യം(മൂല്യം, Storage);
  തിരികെ get_Plain_മൂല്യം(മൂല്യം, Storage);
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
സ്ഥിതം StringRef
parse_Scalar_മൂല്യം(StringRef Unquoted_മൂല്യം, SmallVectorImpl<അക്ഷരം> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<അക്ഷരം> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_മൂല്യം.find_first_of(LookupChars);
  എങ്കിൽ (I == StringRef::npos)
    തിരികെ Unquoted_മൂല്യം;

  Storage.clear();
  Storage.reserve(Unquoted_മൂല്യം.size());
  അക്ഷരം Last_New_വരി_Added_As = '\0';
  വേണ്ടി (; I != StringRef::npos; I = Unquoted_മൂല്യം.find_first_of(LookupChars)) {
    എങ്കിൽ (Unquoted_മൂല്യം[I] != '\r' && Unquoted_മൂല്യം[I] != '\n') {
      llvm::append_range(Storage, Unquoted_മൂല്യം.take_front(I));
      Unquoted_മൂല്യം = UnescapeCallback(Unquoted_മൂല്യം.drop_front(I), Storage);
      Last_New_വരി_Added_As = '\0';
      തുടരുക;
    }
    എങ്കിൽ (size_t LastNonSWhite = Unquoted_മൂല്യം.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_മൂല്യം.take_front(LastNonSWhite + 1));
      Storage.തള്ളുക_back(' ');
      Last_New_വരി_Added_As = ' ';
    } അല്ലെങ്കിൽ {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      തിരഞ്ഞെടുപ്പ് (Last_New_വരി_Added_As) {
      അവസ്ഥ ' ':
        പ്രതിജ്ഞാ(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        Last_New_വരി_Added_As = '\n';
        നിർത്തുക;
      അവസ്ഥ '\n':
        പ്രതിജ്ഞാ(!Storage.empty() && Storage.back() == '\n');
        Storage.തള്ളുക_back('\n');
        നിർത്തുക;
      സ്ഥിരസ്ഥിതി:
        Storage.തള്ളുക_back(' ');
        Last_New_വരി_Added_As = ' ';
        നിർത്തുക;
      }
    }
    // Handle Windows-style EOL
    എങ്കിൽ (Unquoted_മൂല്യം.substr(I, 2) == "\r\n")
      I++;
    Unquoted_മൂല്യം = Unquoted_മൂല്യം.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_മൂല്യം);
  തിരികെ StringRef(Storage.begin(), Storage.size());
}

StringRef
Scalar_കണ്ണി::get_Double_Quoted_മൂല്യം(StringRef Raw_മൂല്യം,
                                 SmallVectorImpl<അക്ഷരം> &Storage) സ്ഥിരം {
  പ്രതിജ്ഞാ(Raw_മൂല്യം.size() >= 2 && Raw_മൂല്യം.front() == '"' &&
         Raw_മൂല്യം.back() == '"');
  StringRef Unquoted_മൂല്യം = Raw_മൂല്യം.substr(1, Raw_മൂല്യം.size() - 2);

  auto UnescapeFunc = [ഇത്](StringRef Unquoted_മൂല്യം,
                             SmallVectorImpl<അക്ഷരം> &Storage) {
    പ്രതിജ്ഞാ(Unquoted_മൂല്യം.take_front(1) == "\\");
    എങ്കിൽ (Unquoted_മൂല്യം.size() == 1) {
      Token T;
      T.Range = Unquoted_മൂല്യം;
      setError("Unrecognized escape code", T);
      Storage.clear();
      തിരികെ StringRef();
    }
    Unquoted_മൂല്യം = Unquoted_മൂല്യം.drop_front(1);
    തിരഞ്ഞെടുപ്പ് (Unquoted_മൂല്യം[0]) {
    സ്ഥിരസ്ഥിതി: {
      Token T;
      T.Range = Unquoted_മൂല്യം.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      തിരികെ StringRef();
    }
    അവസ്ഥ '\r':
      // Shrink the Windows-style EOL.
      എങ്കിൽ (Unquoted_മൂല്യം.size() >= 2 && Unquoted_മൂല്യം[1] == '\n')
        Unquoted_മൂല്യം = Unquoted_മൂല്യം.drop_front(1);
      [[fallthrough]];
    അവസ്ഥ '\n':
      തിരികെ Unquoted_മൂല്യം.drop_front(1).ltrim(" \t");
    അവസ്ഥ '0':
      Storage.തള്ളുക_back(0x00);
      നിർത്തുക;
    അവസ്ഥ 'a':
      Storage.തള്ളുക_back(0x07);
      നിർത്തുക;
    അവസ്ഥ 'b':
      Storage.തള്ളുക_back(0x08);
      നിർത്തുക;
    അവസ്ഥ 't':
    അവസ്ഥ 0x09:
      Storage.തള്ളുക_back(0x09);
      നിർത്തുക;
    അവസ്ഥ 'n':
      Storage.തള്ളുക_back(0x0A);
      നിർത്തുക;
    അവസ്ഥ 'v':
      Storage.തള്ളുക_back(0x0B);
      നിർത്തുക;
    അവസ്ഥ 'f':
      Storage.തള്ളുക_back(0x0C);
      നിർത്തുക;
    അവസ്ഥ 'r':
      Storage.തള്ളുക_back(0x0D);
      നിർത്തുക;
    അവസ്ഥ 'e':
      Storage.തള്ളുക_back(0x1B);
      നിർത്തുക;
    അവസ്ഥ ' ':
      Storage.തള്ളുക_back(0x20);
      നിർത്തുക;
    അവസ്ഥ '"':
      Storage.തള്ളുക_back(0x22);
      നിർത്തുക;
    അവസ്ഥ '/':
      Storage.തള്ളുക_back(0x2F);
      നിർത്തുക;
    അവസ്ഥ '\\':
      Storage.തള്ളുക_back(0x5C);
      നിർത്തുക;
    അവസ്ഥ 'N':
      encodeUTF8(0x85, Storage);
      നിർത്തുക;
    അവസ്ഥ '_':
      encodeUTF8(0xA0, Storage);
      നിർത്തുക;
    അവസ്ഥ 'L':
      encodeUTF8(0x2028, Storage);
      നിർത്തുക;
    അവസ്ഥ 'P':
      encodeUTF8(0x2029, Storage);
      നിർത്തുക;
    അവസ്ഥ 'x': {
      എങ്കിൽ (Unquoted_മൂല്യം.size() < 3)
        // TODO: Report error.
        നിർത്തുക;
      unsigned പൂർണ്ണസംഖ്യ Unicode_Scalar_മൂല്യം;
      എങ്കിൽ (Unquoted_മൂല്യം.substr(1, 2).getAsInteger(16, Unicode_Scalar_മൂല്യം))
        // TODO: Report error.
        Unicode_Scalar_മൂല്യം = 0xFFFD;
      encodeUTF8(Unicode_Scalar_മൂല്യം, Storage);
      തിരികെ Unquoted_മൂല്യം.drop_front(3);
    }
    അവസ്ഥ 'u': {
      എങ്കിൽ (Unquoted_മൂല്യം.size() < 5)
        // TODO: Report error.
        നിർത്തുക;
      unsigned പൂർണ്ണസംഖ്യ Unicode_Scalar_മൂല്യം;
      എങ്കിൽ (Unquoted_മൂല്യം.substr(1, 4).getAsInteger(16, Unicode_Scalar_മൂല്യം))
        // TODO: Report error.
        Unicode_Scalar_മൂല്യം = 0xFFFD;
      encodeUTF8(Unicode_Scalar_മൂല്യം, Storage);
      തിരികെ Unquoted_മൂല്യം.drop_front(5);
    }
    അവസ്ഥ 'U': {
      എങ്കിൽ (Unquoted_മൂല്യം.size() < 9)
        // TODO: Report error.
        നിർത്തുക;
      unsigned പൂർണ്ണസംഖ്യ Unicode_Scalar_മൂല്യം;
      എങ്കിൽ (Unquoted_മൂല്യം.substr(1, 8).getAsInteger(16, Unicode_Scalar_മൂല്യം))
        // TODO: Report error.
        Unicode_Scalar_മൂല്യം = 0xFFFD;
      encodeUTF8(Unicode_Scalar_മൂല്യം, Storage);
      തിരികെ Unquoted_മൂല്യം.drop_front(9);
    }
    }
    തിരികെ Unquoted_മൂല്യം.drop_front(1);
  };

  തിരികെ parse_Scalar_മൂല്യം(Unquoted_മൂല്യം, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_കണ്ണി::get_Single_Quoted_മൂല്യം(StringRef Raw_മൂല്യം,
                                           SmallVectorImpl<അക്ഷരം> &Storage) {
  പ്രതിജ്ഞാ(Raw_മൂല്യം.size() >= 2 && Raw_മൂല്യം.front() == '\'' &&
         Raw_മൂല്യം.back() == '\'');
  StringRef Unquoted_മൂല്യം = Raw_മൂല്യം.substr(1, Raw_മൂല്യം.size() - 2);

  auto UnescapeFunc = [](StringRef Unquoted_മൂല്യം,
                         SmallVectorImpl<അക്ഷരം> &Storage) {
    പ്രതിജ്ഞാ(Unquoted_മൂല്യം.take_front(2) == "''");
    Storage.തള്ളുക_back('\'');
    തിരികെ Unquoted_മൂല്യം.drop_front(2);
  };

  തിരികെ parse_Scalar_മൂല്യം(Unquoted_മൂല്യം, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_കണ്ണി::get_Plain_മൂല്യം(StringRef Raw_മൂല്യം,
                                    SmallVectorImpl<അക്ഷരം> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_മൂല്യം = Raw_മൂല്യം.rtrim("\r\n \t");
  തിരികെ parse_Scalar_മൂല്യം(Raw_മൂല്യം, Storage, "\r\n", nullptr);
}

കണ്ണി *താക്കോൽ_മൂല്യം_കണ്ണി::get_താക്കോൽ() {
  എങ്കിൽ (താക്കോൽ)
    തിരികെ താക്കോൽ;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    എങ്കിൽ (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_മൂല്യം
        || t.Kind == Token::TK_Error) {
      തിരികെ താക്കോൽ = പുതിയ (getAllocator()) Null_കണ്ണി(Doc);
    }
    എങ്കിൽ (t.Kind == Token::TK_താക്കോൽ)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  എങ്കിൽ (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_മൂല്യം) {
    തിരികെ താക്കോൽ = പുതിയ (getAllocator()) Null_കണ്ണി(Doc);
  }

  // We've got a normal key.
  തിരികെ താക്കോൽ = parse_Block_കണ്ണി();
}

കണ്ണി *താക്കോൽ_മൂല്യം_കണ്ണി::get_മൂല്യം() {
  എങ്കിൽ (മൂല്യം)
    തിരികെ മൂല്യം;

  എങ്കിൽ (കണ്ണി* താക്കോൽ = get_താക്കോൽ())
    താക്കോൽ->skip();
  അല്ലെങ്കിൽ {
    setError("Null key in Key Value.", peekNext());
    തിരികെ മൂല്യം = പുതിയ (getAllocator()) Null_കണ്ണി(Doc);
  }

  എങ്കിൽ (failed())
    തിരികെ മൂല്യം = പുതിയ (getAllocator()) Null_കണ്ണി(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    എങ്കിൽ (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_താക്കോൽ
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      തിരികെ മൂല്യം = പുതിയ (getAllocator()) Null_കണ്ണി(Doc);
    }

    എങ്കിൽ (t.Kind != Token::TK_മൂല്യം) {
      setError("Unexpected token in Key Value.", t);
      തിരികെ മൂല്യം = പുതിയ (getAllocator()) Null_കണ്ണി(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  എങ്കിൽ (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_താക്കോൽ) {
    തിരികെ മൂല്യം = പുതിയ (getAllocator()) Null_കണ്ണി(Doc);
  }

  // We got a normal value.
  തിരികെ മൂല്യം = parse_Block_കണ്ണി();
}

ശൂന്യം Mapping_കണ്ണി::increment() {
  എങ്കിൽ (failed()) {
    IsAtEnd = സത്യം;
    CurrentEntry = nullptr;
    തിരികെ;
  }
  എങ്കിൽ (CurrentEntry) {
    CurrentEntry->skip();
    എങ്കിൽ (Type == MT_Inline) {
      IsAtEnd = സത്യം;
      CurrentEntry = nullptr;
      തിരികെ;
    }
  }
  Token T = peekNext();
  എങ്കിൽ (T.Kind == Token::TK_താക്കോൽ || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = പുതിയ (getAllocator()) താക്കോൽ_മൂല്യം_കണ്ണി(Doc);
  } അല്ലെങ്കിൽ എങ്കിൽ (Type == MT_Block) {
    തിരഞ്ഞെടുപ്പ് (T.Kind) {
    അവസ്ഥ Token::TK_BlockEnd:
      getNext();
      IsAtEnd = സത്യം;
      CurrentEntry = nullptr;
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      setError("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    അവസ്ഥ Token::TK_Error:
      IsAtEnd = സത്യം;
      CurrentEntry = nullptr;
    }
  } അല്ലെങ്കിൽ {
    തിരഞ്ഞെടുപ്പ് (T.Kind) {
    അവസ്ഥ Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      തിരികെ increment();
    അവസ്ഥ Token::TK_FlowMappingEnd:
      getNext();
      [[fallthrough]];
    അവസ്ഥ Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = സത്യം;
      CurrentEntry = nullptr;
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      setError( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      IsAtEnd = സത്യം;
      CurrentEntry = nullptr;
    }
  }
}

ശൂന്യം Sequence_കണ്ണി::increment() {
  എങ്കിൽ (failed()) {
    IsAtEnd = സത്യം;
    CurrentEntry = nullptr;
    തിരികെ;
  }
  എങ്കിൽ (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  എങ്കിൽ (SeqType == ST_Block) {
    തിരഞ്ഞെടുപ്പ് (T.Kind) {
    അവസ്ഥ Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_കണ്ണി();
      എങ്കിൽ (!CurrentEntry) { // An error occurred.
        IsAtEnd = സത്യം;
        CurrentEntry = nullptr;
      }
      നിർത്തുക;
    അവസ്ഥ Token::TK_BlockEnd:
      getNext();
      IsAtEnd = സത്യം;
      CurrentEntry = nullptr;
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      setError( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    അവസ്ഥ Token::TK_Error:
      IsAtEnd = സത്യം;
      CurrentEntry = nullptr;
    }
  } അല്ലെങ്കിൽ എങ്കിൽ (SeqType == ST_Indentless) {
    തിരഞ്ഞെടുപ്പ് (T.Kind) {
    അവസ്ഥ Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_കണ്ണി();
      എങ്കിൽ (!CurrentEntry) { // An error occurred.
        IsAtEnd = സത്യം;
        CurrentEntry = nullptr;
      }
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
    അവസ്ഥ Token::TK_Error:
      IsAtEnd = സത്യം;
      CurrentEntry = nullptr;
    }
  } അല്ലെങ്കിൽ എങ്കിൽ (SeqType == ST_Flow) {
    തിരഞ്ഞെടുപ്പ് (T.Kind) {
    അവസ്ഥ Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      WasPreviousTokenFlowEntry = സത്യം;
      തിരികെ increment();
    അവസ്ഥ Token::TK_FlowSequenceEnd:
      getNext();
      [[fallthrough]];
    അവസ്ഥ Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = സത്യം;
      CurrentEntry = nullptr;
      നിർത്തുക;
    അവസ്ഥ Token::TK_StreamEnd:
    അവസ്ഥ Token::TK_DocumentEnd:
    അവസ്ഥ Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = സത്യം;
      CurrentEntry = nullptr;
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      എങ്കിൽ (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = സത്യം;
        CurrentEntry = nullptr;
        നിർത്തുക;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parse_Block_കണ്ണി();
      എങ്കിൽ (!CurrentEntry) {
        IsAtEnd = സത്യം;
      }
      WasPreviousTokenFlowEntry = അസത്യം;
      നിർത്തുക;
    }
  }
}

Document::Document(Stream &S) : stream(S), വേര്_2(nullptr) {
  // Tag maps starts with two default mappings.
  TagMap["!"] = "!";
  TagMap["!!"] = "tag:yaml.org,2002:";

  എങ്കിൽ (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peekNext();
  എങ്കിൽ (T.Kind == Token::TK_DocumentStart)
    getNext();
}

തർക്കം Document::skip()  {
  എങ്കിൽ (stream.scanner->failed())
    തിരികെ അസത്യം;
  എങ്കിൽ (!വേര്_2 && !get_വേര്())
    തിരികെ അസത്യം;
  വേര്_2->skip();
  Token &T = peekNext();
  എങ്കിൽ (T.Kind == Token::TK_StreamEnd)
    തിരികെ അസത്യം;
  എങ്കിൽ (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    തിരികെ skip();
  }
  തിരികെ സത്യം;
}

Token &Document::peekNext() {
  തിരികെ stream.scanner->peekNext();
}

Token Document::getNext() {
  തിരികെ stream.scanner->getNext();
}

ശൂന്യം Document::setError(സ്ഥിരം Twine &Message, Token &Location) സ്ഥിരം {
  stream.scanner->setError(Message, Location.Range.begin());
}

തർക്കം Document::failed() സ്ഥിരം {
  തിരികെ stream.scanner->failed();
}

കണ്ണി *Document::parse_Block_കണ്ണി() {
  Token T = peekNext();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  തിരഞ്ഞെടുപ്പ് (T.Kind) {
  അവസ്ഥ Token::TK_Alias:
    getNext();
    തിരികെ പുതിയ (കണ്ണി_Allocator) Alias_കണ്ണി(stream.CurrentDoc, T.Range.substr(1));
  അവസ്ഥ Token::TK_Anchor:
    എങ്കിൽ (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      തിരികെ nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    പോകുക parse_property;
  അവസ്ഥ Token::TK_Tag:
    എങ്കിൽ (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      തിരികെ nullptr;
    }
    TagInfo = getNext(); // Consume TK_Tag.
    T = peekNext();
    പോകുക parse_property;
  സ്ഥിരസ്ഥിതി:
    നിർത്തുക;
  }

  തിരഞ്ഞെടുപ്പ് (T.Kind) {
  അവസ്ഥ Token::TK_BlockEntry:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    തിരികെ പുതിയ (കണ്ണി_Allocator) Sequence_കണ്ണി( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , Sequence_കണ്ണി::ST_Indentless);
  അവസ്ഥ Token::TK_BlockSequenceStart:
    getNext();
    തിരികെ പുതിയ (കണ്ണി_Allocator)
      Sequence_കണ്ണി( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_കണ്ണി::ST_Block);
  അവസ്ഥ Token::TK_BlockMappingStart:
    getNext();
    തിരികെ പുതിയ (കണ്ണി_Allocator)
      Mapping_കണ്ണി( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_കണ്ണി::MT_Block);
  അവസ്ഥ Token::TK_FlowSequenceStart:
    getNext();
    തിരികെ പുതിയ (കണ്ണി_Allocator)
      Sequence_കണ്ണി( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_കണ്ണി::ST_Flow);
  അവസ്ഥ Token::TK_FlowMappingStart:
    getNext();
    തിരികെ പുതിയ (കണ്ണി_Allocator)
      Mapping_കണ്ണി( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_കണ്ണി::MT_Flow);
  അവസ്ഥ Token::TK_Scalar:
    getNext();
    തിരികെ പുതിയ (കണ്ണി_Allocator)
      Scalar_കണ്ണി( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  അവസ്ഥ Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.മൂല്യം.c_str(), T.മൂല്യം.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(കണ്ണി_Allocator).drop_back();
    തിരികെ പുതിയ (കണ്ണി_Allocator)
        Block_Scalar_കണ്ണി(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  അവസ്ഥ Token::TK_താക്കോൽ:
    // Don't eat the TK_Key, KeyValueNode expects it.
    തിരികെ പുതിയ (കണ്ണി_Allocator)
      Mapping_കണ്ണി( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_കണ്ണി::MT_Inline);
  അവസ്ഥ Token::TK_DocumentStart:
  അവസ്ഥ Token::TK_DocumentEnd:
  അവസ്ഥ Token::TK_StreamEnd:
  സ്ഥിരസ്ഥിതി:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    തിരികെ പുതിയ (കണ്ണി_Allocator) Null_കണ്ണി(stream.CurrentDoc);
  അവസ്ഥ Token::TK_FlowMappingEnd:
  അവസ്ഥ Token::TK_FlowSequenceEnd:
  അവസ്ഥ Token::TK_FlowEntry: {
    എങ്കിൽ (വേര്_2 && (isa<Mapping_കണ്ണി>(വേര്_2) || isa<Sequence_കണ്ണി>(വേര്_2)))
      തിരികെ പുതിയ (കണ്ണി_Allocator) Null_കണ്ണി(stream.CurrentDoc);

    setError("Unexpected token", T);
    തിരികെ nullptr;
  }
  അവസ്ഥ Token::TK_Error:
    തിരികെ nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  തിരികെ nullptr;
}

തർക്കം Document::parseDirectives() {
  തർക്കം isDirective = അസത്യം;
  ആകുമ്പോൾ (സത്യം) {
    Token T = peekNext();
    എങ്കിൽ (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = സത്യം;
    } അല്ലെങ്കിൽ എങ്കിൽ (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = സത്യം;
    } അല്ലെങ്കിൽ {
      നിർത്തുക;
    }
  }
  തിരികെ isDirective;
}

ശൂന്യം Document::parseYAMLDirective() {
  getNext(); // Eat %YAML <version>
}

ശൂന്യം Document::parseTAGDirective() {
  Token Tag = getNext(); // %TAG <handle> <prefix>
  StringRef T = Tag.Range;
  // Strip %TAG
  T = T.substr(T.find_first_of(" \t")).ltrim(" \t");
  std::size_t HandleEnd = T.find_first_of(" \t");
  StringRef TagHandle = T.substr(0, HandleEnd);
  StringRef TagPrefix = T.substr(HandleEnd).ltrim(" \t");
  TagMap[TagHandle] = TagPrefix;
}

തർക്കം Document::expectToken(പൂർണ്ണസംഖ്യ TK) {
  Token T = getNext();
  എങ്കിൽ (T.Kind != TK) {
    setError("Unexpected token", T);
    തിരികെ അസത്യം;
  }
  തിരികെ സത്യം;
}
