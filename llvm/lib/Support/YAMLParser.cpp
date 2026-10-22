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

#ಅನ್ತರ್ಭಾವ "llvm/Support/YAMLParser.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/AllocatorList.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Twine.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compiler.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SMLoc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SourceMgr.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Unicode.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <cassert>
#ಅನ್ತರ್ಭಾವ <cstddef>
#ಅನ್ತರ್ಭಾವ <cstdint>
#ಅನ್ತರ್ಭಾವ <map>
#ಅನ್ತರ್ಭಾವ <memory>
#ಅನ್ತರ್ಭಾವ <string>
#ಅನ್ತರ್ಭಾವ <system_error>
#ಅನ್ತರ್ಭಾವ <utility>

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ yaml;

ಗಣನಾ UnicodeEncodingForm {
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
ಸ್ಥಿರ EncodingInfo getUnicodeEncoding(StringRef ಒಳಹರಿವು) {
  ಯದಿ (ಒಳಹರಿವು.empty())
    ಪ್ರತ್ಯಾಗಮ {UEF_Unknown, 0};

  ವಿಕಲ್ಪ (uint8_t(ಒಳಹರಿವು[0])) {
  ಪಕ್ಷ 0x00:
    ಯದಿ (ಒಳಹರಿವು.size() >= 4) {
      ಯದಿ (  ಒಳಹರಿವು[1] == 0
         && uint8_t(ಒಳಹರಿವು[2]) == 0xFE
         && uint8_t(ಒಳಹರಿವು[3]) == 0xFF)
        ಪ್ರತ್ಯಾಗಮ {UEF_UTF32_BE, 4};
      ಯದಿ (ಒಳಹರಿವು[1] == 0 && ಒಳಹರಿವು[2] == 0 && ಒಳಹರಿವು[3] != 0)
        ಪ್ರತ್ಯಾಗಮ {UEF_UTF32_BE, 0};
    }

    ಯದಿ (ಒಳಹರಿವು.size() >= 2 && ಒಳಹರಿವು[1] != 0)
      ಪ್ರತ್ಯಾಗಮ {UEF_UTF16_BE, 0};
    ಪ್ರತ್ಯಾಗಮ {UEF_Unknown, 0};
  ಪಕ್ಷ 0xFF:
    ಯದಿ (  ಒಳಹರಿವು.size() >= 4
       && uint8_t(ಒಳಹರಿವು[1]) == 0xFE
       && ಒಳಹರಿವು[2] == 0
       && ಒಳಹರಿವು[3] == 0)
      ಪ್ರತ್ಯಾಗಮ {UEF_UTF32_LE, 4};

    ಯದಿ (ಒಳಹರಿವು.size() >= 2 && uint8_t(ಒಳಹರಿವು[1]) == 0xFE)
      ಪ್ರತ್ಯಾಗಮ {UEF_UTF16_LE, 2};
    ಪ್ರತ್ಯಾಗಮ {UEF_Unknown, 0};
  ಪಕ್ಷ 0xFE:
    ಯದಿ (ಒಳಹರಿವು.size() >= 2 && uint8_t(ಒಳಹರಿವು[1]) == 0xFF)
      ಪ್ರತ್ಯಾಗಮ {UEF_UTF16_BE, 2};
    ಪ್ರತ್ಯಾಗಮ {UEF_Unknown, 0};
  ಪಕ್ಷ 0xEF:
    ಯದಿ (  ಒಳಹರಿವು.size() >= 3
       && uint8_t(ಒಳಹರಿವು[1]) == 0xBB
       && uint8_t(ಒಳಹರಿವು[2]) == 0xBF)
      ಪ್ರತ್ಯಾಗಮ {UEF_UTF8, 3};
    ಪ್ರತ್ಯಾಗಮ {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  ಯದಿ (ಒಳಹರಿವು.size() >= 4 && ಒಳಹರಿವು[1] == 0 && ಒಳಹರಿವು[2] == 0 && ಒಳಹರಿವು[3] == 0)
    ಪ್ರತ್ಯಾಗಮ {UEF_UTF32_LE, 0};

  ಯದಿ (ಒಳಹರಿವು.size() >= 2 && ಒಳಹರಿವು[1] == 0)
    ಪ್ರತ್ಯಾಗಮ {UEF_UTF16_LE, 0};

  ಪ್ರತ್ಯಾಗಮ {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
ಶೂನ್ಯ ಗಂಟು::anchor() {}
ಶೂನ್ಯ Null_ಗಂಟು::anchor() {}
ಶೂನ್ಯ Scalar_ಗಂಟು::anchor() {}
ಶೂನ್ಯ Block_Scalar_ಗಂಟು::anchor() {}
ಶೂನ್ಯ ಕೀಲಿ_ಮೌಲ್ಯ_ಗಂಟು::anchor() {}
ಶೂನ್ಯ Mapping_ಗಂಟು::anchor() {}
ಶೂನ್ಯ Sequence_ಗಂಟು::anchor() {}
ಶೂನ್ಯ Alias_ಗಂಟು::anchor() {}

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ yaml {

/// Token - A single YAML token.
ಸಂರಚನಾ Token {
  ಗಣನಾ TokenKind {
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
    TK_ಕೀಲಿ,
    TK_ಮೌಲ್ಯ,
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
  std::string ಮೌಲ್ಯ;

  Token() = ಸ್ವಭಾವ;
};

} // end namespace yaml
} // end namespace llvm

using TokenQueueT = BumpPtrList<Token>;

ನಾಮಾಕಾಶ {

/// This struct is used to track simple keys.
///
/// Simple keys are handled by creating an entry in SimpleKeys for each Token
/// which could legally be the start of a simple key. When peekNext is called,
/// if the Token To be returned is referenced by a SimpleKey, we continue
/// tokenizing until that potential simple key has either been found to not be
/// a simple key (we moved on to the next line or went further than 1024 chars).
/// Or when we run into a Value, and then insert a Key token (and possibly
/// others) before the SimpleKey's Tok.
ಸಂರಚನಾ Simple_ಕೀಲಿ {
  TokenQueueT::iterator Tok;
  unsigned Column = 0;
  unsigned ಸಾಲು = 0;
  unsigned FlowLevel = 0;
  ತಾರ್ಕಿಕ IsRequired = ಅಸತ್ಯ;

  ತಾರ್ಕಿಕ operator ==(ಧ್ರುವ Simple_ಕೀಲಿ &Other) {
    ಪ್ರತ್ಯಾಗಮ Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
using UTF8Decoded = std::pair<uint32_t, unsigned>;

ಸ್ಥಿರ UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator End = Range.end();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  ಯದಿ (Position < End && (*Position & 0x80) == 0) {
    ಪ್ರತ್ಯಾಗಮ {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  ಯದಿ (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    ಯದಿ (codepoint >= 0x80)
      ಪ್ರತ್ಯಾಗಮ {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  ಯದಿ (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    ಯದಿ (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      ಪ್ರತ್ಯಾಗಮ {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  ಯದಿ (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    ಯದಿ (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      ಪ್ರತ್ಯಾಗಮ {codepoint, 4};
  }
  ಪ್ರತ್ಯಾಗಮ {0, 0};
}

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ yaml {

/// Scans YAML tokens from a MemoryBuffer.
ವರ್ಗ Scanner {
ಸಾರ್ವಜನಿಕ:
  Scanner(StringRef ಒಳಹರಿವು, SourceMgr &SM, ತಾರ್ಕಿಕ ShowColors = ಸತ್ಯ,
          std::error_code *EC = nullptr);
  Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, ತಾರ್ಕಿಕ ShowColors = ಸತ್ಯ,
          std::error_code *EC = nullptr);

  /// Parse the next token and return it without popping it.
  Token &peekNext();

  /// Parse the next token and pop it from the queue.
  Token getNext();

  ಶೂನ್ಯ printError(SMLoc Loc, SourceMgr::DiagKind Kind, ಧ್ರುವ Twine &Message,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.PrintMessage(Loc, Kind, Message, Ranges, /* FixIts= */ {}, ShowColors);
  }

  ಶೂನ್ಯ setError(ಧ್ರುವ Twine &Message, StringRef::iterator Position) {
    ಯದಿ (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    ಯದಿ (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    ಯದಿ (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = ಸತ್ಯ;
  }

  /// Returns true if an error occurred while parsing.
  ತಾರ್ಕಿಕ failed() {
    ಪ್ರತ್ಯಾಗಮ Failed;
  }

ಗುಪ್ತ:
  ಶೂನ್ಯ init(MemoryBufferRef Buffer);

  StringRef current_ಒಳಹರಿವು() {
    ಪ್ರತ್ಯಾಗಮ StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    ಪ್ರತ್ಯಾಗಮ ::decodeUTF8(StringRef(Position, End - Position));
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
  ಶೂನ್ಯ advanceWhile(SkipWhileFunc Func);

  /// Scan ns-uri-char[39]s starting at Cur.
  ///
  /// This updates Cur and Column while scanning.
  ಶೂನ್ಯ scan_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  ತಾರ್ಕಿಕ consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  ಶೂನ್ಯ skip(uint32_t Distance);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  ತಾರ್ಕಿಕ isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  ತಾರ್ಕಿಕ isPlainSafeNonBlank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  ತಾರ್ಕಿಕ is_ಸಾಲು_Empty(StringRef ಸಾಲು);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  ತಾರ್ಕಿಕ consume_ಸಾಲು_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  ಶೂನ್ಯ save_Simple_ಕೀಲಿ_Candidate( TokenQueueT::iterator Tok
                             , unsigned AtColumn
                             , ತಾರ್ಕಿಕ IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  ಶೂನ್ಯ remove_Stale_Simple_ಕೀಲಿ_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  ಶೂನ್ಯ remove_Simple_ಕೀಲಿ_Candidates_On_Flow_Level(unsigned Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  ತಾರ್ಕಿಕ unrollIndent(ಪೂರ್ಣಾಙ್ಕ ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  ತಾರ್ಕಿಕ rollIndent( ಪೂರ್ಣಾಙ್ಕ ToColumn
                 , Token::TokenKind Kind
                 , TokenQueueT::iterator ಸೇರಿಸು_Point);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  ಶೂನ್ಯ skipComment();

  /// Skip whitespace and comments until the start of the next token.
  ಶೂನ್ಯ scanToNextToken();

  /// Must be the first token generated.
  ತಾರ್ಕಿಕ scanStreamStart();

  /// Generate tokens needed to close out the stream.
  ತಾರ್ಕಿಕ scanStreamEnd();

  /// Scan a %BLAH directive.
  ತಾರ್ಕಿಕ scanDirective();

  /// Scan a ... or ---.
  ತಾರ್ಕಿಕ scanDocumentIndicator(ತಾರ್ಕಿಕ IsStart);

  /// Scan a [ or { and generate the proper flow collection start token.
  ತಾರ್ಕಿಕ scanFlowCollectionStart(ತಾರ್ಕಿಕ IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  ತಾರ್ಕಿಕ scanFlowCollectionEnd(ತಾರ್ಕಿಕ IsSequence);

  /// Scan the , that separates entries in a flow collection.
  ತಾರ್ಕಿಕ scanFlowEntry();

  /// Scan the - that starts block sequence entries.
  ತಾರ್ಕಿಕ scanBlockEntry();

  /// Scan an explicit ? indicating a key.
  ತಾರ್ಕಿಕ scan_ಕೀಲಿ();

  /// Scan an explicit : indicating a value.
  ತಾರ್ಕಿಕ scan_ಮೌಲ್ಯ();

  /// Scan a quoted scalar.
  ತಾರ್ಕಿಕ scanFlowScalar(ತಾರ್ಕಿಕ IsDoubleQuoted);

  /// Scan an unquoted scalar.
  ತಾರ್ಕಿಕ scanPlainScalar();

  /// Scan an Alias or Anchor starting with * or &.
  ತಾರ್ಕಿಕ scanAliasOrAnchor(ತಾರ್ಕಿಕ IsAlias);

  /// Scan a block scalar starting with | or >.
  ತಾರ್ಕಿಕ scanBlockScalar(ತಾರ್ಕಿಕ IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  ತಾರ್ಕಿಕ scanBlockScalarIndicators(ವರ್ಣ &StyleIndicator, ವರ್ಣ &ChompingIndicator,
                                 unsigned &IndentIndicator, ತಾರ್ಕಿಕ &IsDone);

  /// Scan a style indicator in a block scalar header.
  ವರ್ಣ scanBlockStyleIndicator();

  /// Scan a chomping indicator in a block scalar header.
  ವರ್ಣ scanBlockChompingIndicator();

  /// Scan an indentation indicator in a block scalar header.
  unsigned scanBlockIndentationIndicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  ತಾರ್ಕಿಕ scanBlockScalarHeader(ವರ್ಣ &ChompingIndicator, unsigned &IndentIndicator,
                             ತಾರ್ಕಿಕ &IsDone);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  ತಾರ್ಕಿಕ findBlockScalarIndent(unsigned &BlockIndent, unsigned BlockExitIndent,
                             unsigned &ಸಾಲು_Breaks, ತಾರ್ಕಿಕ &IsDone);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  ತಾರ್ಕಿಕ scanBlockScalarIndent(unsigned BlockIndent, unsigned BlockExitIndent,
                             ತಾರ್ಕಿಕ &IsDone);

  /// Scan a tag of the form !stuff.
  ತಾರ್ಕಿಕ scanTag();

  /// Dispatch to the next scanning function based on \a *Cur.
  ತಾರ್ಕಿಕ fetchMoreTokens();

  /// The SourceMgr used for diagnostics and buffer management.
  SourceMgr &SM;

  /// The original input.
  MemoryBufferRef ಒಳಹರಿವು_Buffer;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator End;

  /// Current YAML indentation level in spaces.
  ಪೂರ್ಣಾಙ್ಕ Indent;

  /// Current column number in Unicode code points.
  unsigned Column;

  /// Current line number.
  unsigned ಸಾಲು;

  /// How deep we are in flow style containers. 0 Means at block level.
  unsigned FlowLevel;

  /// Are we at the start of the stream?
  ತಾರ್ಕಿಕ IsStartOfStream;

  /// Can the next token be the start of a simple key?
  ತಾರ್ಕಿಕ Is_Simple_ಕೀಲಿ_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  ತಾರ್ಕಿಕ Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow;

  /// True if an error has occurred.
  ತಾರ್ಕಿಕ Failed;

  /// Should colors be used when printing out the diagnostic messages?
  ತಾರ್ಕಿಕ ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  TokenQueueT TokenQueue;

  /// Indentation levels.
  SmallVector<ಪೂರ್ಣಾಙ್ಕ, 4> Indents;

  /// Potential simple keys.
  SmallVector<Simple_ಕೀಲಿ, 4> Simple_ಕೀಲಿಗಳು;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
ಸ್ಥಿರ ಶೂನ್ಯ encodeUTF8( uint32_t Unicode_Scalar_ಮೌಲ್ಯ
                      , SmallVectorImpl<ವರ್ಣ> &Result) {
  ಯದಿ (Unicode_Scalar_ಮೌಲ್ಯ <= 0x7F) {
    Result.ತಳ್ಳು_back(Unicode_Scalar_ಮೌಲ್ಯ & 0x7F);
  } ಅನ್ಯಥಾ ಯದಿ (Unicode_Scalar_ಮೌಲ್ಯ <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((Unicode_Scalar_ಮೌಲ್ಯ & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Unicode_Scalar_ಮೌಲ್ಯ & 0x3F);
    Result.ತಳ್ಳು_back(FirstByte);
    Result.ತಳ್ಳು_back(SecondByte);
  } ಅನ್ಯಥಾ ಯದಿ (Unicode_Scalar_ಮೌಲ್ಯ <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((Unicode_Scalar_ಮೌಲ್ಯ & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_ಮೌಲ್ಯ & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Unicode_Scalar_ಮೌಲ್ಯ & 0x3F);
    Result.ತಳ್ಳು_back(FirstByte);
    Result.ತಳ್ಳು_back(SecondByte);
    Result.ತಳ್ಳು_back(ThirdByte);
  } ಅನ್ಯಥಾ ಯದಿ (Unicode_Scalar_ಮೌಲ್ಯ <= 0x10FFFF) {
    uint8_t FirstByte = 0xF0 | ((Unicode_Scalar_ಮೌಲ್ಯ & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_ಮೌಲ್ಯ & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Unicode_Scalar_ಮೌಲ್ಯ & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_ಮೌಲ್ಯ & 0x3F);
    Result.ತಳ್ಳು_back(FirstByte);
    Result.ತಳ್ಳು_back(SecondByte);
    Result.ತಳ್ಳು_back(ThirdByte);
    Result.ತಳ್ಳು_back(FourthByte);
  }
}

ತಾರ್ಕಿಕ yaml::dumpTokens(StringRef ಒಳಹರಿವು, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(ಒಳಹರಿವು, SM);
  ಯಾವತ್ (ಸತ್ಯ) {
    Token T = scanner.getNext();
    ವಿಕಲ್ಪ (T.Kind) {
    ಪಕ್ಷ Token::TK_StreamStart:
      OS << "Stream-Start: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_StreamEnd:
      OS << "Stream-End: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_VersionDirective:
      OS << "Version-Directive: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_TagDirective:
      OS << "Tag-Directive: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_DocumentStart:
      OS << "Document-Start: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_DocumentEnd:
      OS << "Document-End: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_BlockEntry:
      OS << "Block-Entry: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_BlockEnd:
      OS << "Block-End: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_BlockSequenceStart:
      OS << "Block-Sequence-Start: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_BlockMappingStart:
      OS << "Block-Mapping-Start: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_FlowEntry:
      OS << "Flow-Entry: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_FlowSequenceStart:
      OS << "Flow-Sequence-Start: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_FlowSequenceEnd:
      OS << "Flow-Sequence-End: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_FlowMappingStart:
      OS << "Flow-Mapping-Start: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_FlowMappingEnd:
      OS << "Flow-Mapping-End: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_ಕೀಲಿ:
      OS << "Key: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_ಮೌಲ್ಯ:
      OS << "Value: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_Scalar:
      OS << "Scalar: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_BlockScalar:
      OS << "Block Scalar: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_Alias:
      OS << "Alias: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_Anchor:
      OS << "Anchor: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_Tag:
      OS << "Tag: ";
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_Error:
      ಭಙ್ಗ;
    }
    OS << T.Range << "\n";
    ಯದಿ (T.Kind == Token::TK_StreamEnd)
      ಭಙ್ಗ;
    ಅನ್ಯಥಾ ಯದಿ (T.Kind == Token::TK_Error)
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ yaml::scanTokens(StringRef ಒಳಹರಿವು) {
  SourceMgr SM;
  Scanner scanner(ಒಳಹರಿವು, SM);
  ಯಾವತ್ (ಸತ್ಯ) {
    Token T = scanner.getNext();
    ಯದಿ (T.Kind == Token::TK_StreamEnd)
      ಭಙ್ಗ;
    ಅನ್ಯಥಾ ಯದಿ (T.Kind == Token::TK_Error)
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

std::string yaml::escape(StringRef ಒಳಹರಿವು, ತಾರ್ಕಿಕ EscapePrintable) {
  std::string Escaped_ಒಳಹರಿವು;
  ಪ್ರತಿ (StringRef::iterator i = ಒಳಹರಿವು.begin(), e = ಒಳಹರಿವು.end(); i != e; ++i) {
    ಯದಿ (*i == '\\')
      Escaped_ಒಳಹರಿವು += "\\\\";
    ಅನ್ಯಥಾ ಯದಿ (*i == '"')
      Escaped_ಒಳಹರಿವು += "\\\"";
    ಅನ್ಯಥಾ ಯದಿ (*i == 0)
      Escaped_ಒಳಹರಿವು += "\\0";
    ಅನ್ಯಥಾ ಯದಿ (*i == 0x07)
      Escaped_ಒಳಹರಿವು += "\\a";
    ಅನ್ಯಥಾ ಯದಿ (*i == 0x08)
      Escaped_ಒಳಹರಿವು += "\\b";
    ಅನ್ಯಥಾ ಯದಿ (*i == 0x09)
      Escaped_ಒಳಹರಿವು += "\\t";
    ಅನ್ಯಥಾ ಯದಿ (*i == 0x0A)
      Escaped_ಒಳಹರಿವು += "\\n";
    ಅನ್ಯಥಾ ಯದಿ (*i == 0x0B)
      Escaped_ಒಳಹರಿವು += "\\v";
    ಅನ್ಯಥಾ ಯದಿ (*i == 0x0C)
      Escaped_ಒಳಹರಿವು += "\\f";
    ಅನ್ಯಥಾ ಯದಿ (*i == 0x0D)
      Escaped_ಒಳಹರಿವು += "\\r";
    ಅನ್ಯಥಾ ಯದಿ (*i == 0x1B)
      Escaped_ಒಳಹರಿವು += "\\e";
    ಅನ್ಯಥಾ ಯದಿ ((unsigned ವರ್ಣ)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_ಒಳಹರಿವು += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } ಅನ್ಯಥಾ ಯದಿ (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_ಮೌಲ್ಯ
        = decodeUTF8(StringRef(i, ಒಳಹರಿವು.end() - i));
      ಯದಿ (Unicode_Scalar_ಮೌಲ್ಯ.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_ಒಳಹರಿವು, Val);
        // FIXME: Error reporting.
        ಪ್ರತ್ಯಾಗಮ Escaped_ಒಳಹರಿವು;
      }
      ಯದಿ (Unicode_Scalar_ಮೌಲ್ಯ.first == 0x85)
        Escaped_ಒಳಹರಿವು += "\\N";
      ಅನ್ಯಥಾ ಯದಿ (Unicode_Scalar_ಮೌಲ್ಯ.first == 0xA0)
        Escaped_ಒಳಹರಿವು += "\\_";
      ಅನ್ಯಥಾ ಯದಿ (Unicode_Scalar_ಮೌಲ್ಯ.first == 0x2028)
        Escaped_ಒಳಹರಿವು += "\\L";
      ಅನ್ಯಥಾ ಯದಿ (Unicode_Scalar_ಮೌಲ್ಯ.first == 0x2029)
        Escaped_ಒಳಹರಿವು += "\\P";
      ಅನ್ಯಥಾ ಯದಿ (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_ಮೌಲ್ಯ.first))
        Escaped_ಒಳಹರಿವು += StringRef(i, Unicode_Scalar_ಮೌಲ್ಯ.second);
      ಅನ್ಯಥಾ {
        std::string HexStr = utohexstr(Unicode_Scalar_ಮೌಲ್ಯ.first);
        ಯದಿ (HexStr.size() <= 2)
          Escaped_ಒಳಹರಿವು += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        ಅನ್ಯಥಾ ಯದಿ (HexStr.size() <= 4)
          Escaped_ಒಳಹರಿವು += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        ಅನ್ಯಥಾ ಯದಿ (HexStr.size() <= 8)
          Escaped_ಒಳಹರಿವು += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += Unicode_Scalar_ಮೌಲ್ಯ.second - 1;
    } ಅನ್ಯಥಾ {
      Escaped_ಒಳಹರಿವು.ತಳ್ಳು_back(*i);
    }
  }
  ಪ್ರತ್ಯಾಗಮ Escaped_ಒಳಹರಿವು;
}

std::optional<ತಾರ್ಕಿಕ> yaml::parseBool(StringRef S) {
  ವಿಕಲ್ಪ (S.size()) {
  ಪಕ್ಷ 1:
    ವಿಕಲ್ಪ (S.front()) {
    ಪಕ್ಷ 'y':
    ಪಕ್ಷ 'Y':
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    ಪಕ್ಷ 'n':
    ಪಕ್ಷ 'N':
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ std::nullopt;
    }
  ಪಕ್ಷ 2:
    ವಿಕಲ್ಪ (S.front()) {
    ಪಕ್ಷ 'O':
      ಯದಿ (S[1] == 'N') // ON
        ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
      [[fallthrough]];
    ಪಕ್ಷ 'o':
      ಯದಿ (S[1] == 'n') //[Oo]n
        ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
      ಪ್ರತ್ಯಾಗಮ std::nullopt;
    ಪಕ್ಷ 'N':
      ಯದಿ (S[1] == 'O') // NO
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      [[fallthrough]];
    ಪಕ್ಷ 'n':
      ಯದಿ (S[1] == 'o') //[Nn]o
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      ಪ್ರತ್ಯಾಗಮ std::nullopt;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ std::nullopt;
    }
  ಪಕ್ಷ 3:
    ವಿಕಲ್ಪ (S.front()) {
    ಪಕ್ಷ 'O':
      ಯದಿ (S.drop_front() == "FF") // OFF
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      [[fallthrough]];
    ಪಕ್ಷ 'o':
      ಯದಿ (S.drop_front() == "ff") //[Oo]ff
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      ಪ್ರತ್ಯಾಗಮ std::nullopt;
    ಪಕ್ಷ 'Y':
      ಯದಿ (S.drop_front() == "ES") // YES
        ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
      [[fallthrough]];
    ಪಕ್ಷ 'y':
      ಯದಿ (S.drop_front() == "es") //[Yy]es
        ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
      ಪ್ರತ್ಯಾಗಮ std::nullopt;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ std::nullopt;
    }
  ಪಕ್ಷ 4:
    ವಿಕಲ್ಪ (S.front()) {
    ಪಕ್ಷ 'T':
      ಯದಿ (S.drop_front() == "RUE") // TRUE
        ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
      [[fallthrough]];
    ಪಕ್ಷ 't':
      ಯದಿ (S.drop_front() == "rue") //[Tt]rue
        ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
      ಪ್ರತ್ಯಾಗಮ std::nullopt;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ std::nullopt;
    }
  ಪಕ್ಷ 5:
    ವಿಕಲ್ಪ (S.front()) {
    ಪಕ್ಷ 'F':
      ಯದಿ (S.drop_front() == "ALSE") // FALSE
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      [[fallthrough]];
    ಪಕ್ಷ 'f':
      ಯದಿ (S.drop_front() == "alse") //[Ff]alse
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      ಪ್ರತ್ಯಾಗಮ std::nullopt;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ std::nullopt;
    }
  ಸ್ವಭಾವ:
    ಪ್ರತ್ಯಾಗಮ std::nullopt;
  }
}

Scanner::Scanner(StringRef ಒಳಹರಿವು, SourceMgr &sm, ತಾರ್ಕಿಕ ShowColors,
                 std::error_code *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(MemoryBufferRef(ಒಳಹರಿವು, "YAML"));
}

Scanner::Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, ತಾರ್ಕಿಕ ShowColors,
                 std::error_code *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(Buffer);
}

ಶೂನ್ಯ Scanner::init(MemoryBufferRef Buffer) {
  ಒಳಹರಿವು_Buffer = Buffer;
  Current = ಒಳಹರಿವು_Buffer.getBufferStart();
  End = ಒಳಹರಿವು_Buffer.getBufferEnd();
  Indent = -1;
  Column = 0;
  ಸಾಲು = 0;
  FlowLevel = 0;
  IsStartOfStream = ಸತ್ಯ;
  Is_Simple_ಕೀಲಿ_Allowed = ಸತ್ಯ;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow = ಅಸತ್ಯ;
  Failed = ಅಸತ್ಯ;
  std::unique_ptr<MemoryBuffer> ಒಳಹರಿವು_Buffer_Owner =
      MemoryBuffer::getMemBuffer(Buffer, /*RequiresNullTerminator=*/ಅಸತ್ಯ);
  SM.AddNewSourceBuffer(std::move(ಒಳಹರಿವು_Buffer_Owner), SMLoc());
}

Token &Scanner::peekNext() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  ತಾರ್ಕಿಕ NeedMore = ಅಸತ್ಯ;
  ಯಾವತ್ (ಸತ್ಯ) {
    ಯದಿ (TokenQueue.empty() || NeedMore) {
      ಯದಿ (!fetchMoreTokens()) {
        TokenQueue.clear();
        Simple_ಕೀಲಿಗಳು.clear();
        TokenQueue.ತಳ್ಳು_back(Token());
        ಪ್ರತ್ಯಾಗಮ TokenQueue.front();
      }
    }
    ಪ್ರತಿಜ್ಞಾ(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_ಕೀಲಿ_Candidates();
    Simple_ಕೀಲಿ SK;
    SK.Tok = TokenQueue.begin();
    ಯದಿ (!is_contained(Simple_ಕೀಲಿಗಳು, SK))
      ಭಙ್ಗ;
    ಅನ್ಯಥಾ
      NeedMore = ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  ಯದಿ (!TokenQueue.empty())
    TokenQueue.ತೆಗೆ_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  ಯದಿ (TokenQueue.empty())
    TokenQueue.resetAlloc();

  ಪ್ರತ್ಯಾಗಮ Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  ಯದಿ (Position == End)
    ಪ್ರತ್ಯಾಗಮ Position;
  // Check 7 bit c-printable - b-char.
  ಯದಿ (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    ಪ್ರತ್ಯಾಗಮ Position + 1;

  // Check for valid UTF-8.
  ಯದಿ (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    ಯದಿ (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      ಪ್ರತ್ಯಾಗಮ Position + u8d.second;
  }
  ಪ್ರತ್ಯಾಗಮ Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  ಯದಿ (Position == End)
    ಪ್ರತ್ಯಾಗಮ Position;
  ಯದಿ (*Position == 0x0D) {
    ಯದಿ (Position + 1 != End && *(Position + 1) == 0x0A)
      ಪ್ರತ್ಯಾಗಮ Position + 2;
    ಪ್ರತ್ಯಾಗಮ Position + 1;
  }

  ಯದಿ (*Position == 0x0A)
    ಪ್ರತ್ಯಾಗಮ Position + 1;
  ಪ್ರತ್ಯಾಗಮ Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  ಯದಿ (Position == End)
    ಪ್ರತ್ಯಾಗಮ Position;
  ಯದಿ (*Position == ' ')
    ಪ್ರತ್ಯಾಗಮ Position + 1;
  ಪ್ರತ್ಯಾಗಮ Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  ಯದಿ (Position == End)
    ಪ್ರತ್ಯಾಗಮ Position;
  ಯದಿ (*Position == ' ' || *Position == '\t')
    ಪ್ರತ್ಯಾಗಮ Position + 1;
  ಪ್ರತ್ಯಾಗಮ Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  ಯದಿ (Position == End)
    ಪ್ರತ್ಯಾಗಮ Position;
  ಯದಿ (*Position == ' ' || *Position == '\t')
    ಪ್ರತ್ಯಾಗಮ Position;
  ಪ್ರತ್ಯಾಗಮ skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  ಯಾವತ್ (ಸತ್ಯ) {
    StringRef::iterator i = (ಏತತ್->*Func)(Position);
    ಯದಿ (i == Position)
      ಭಙ್ಗ;
    Position = i;
  }
  ಪ್ರತ್ಯಾಗಮ Position;
}

ಶೂನ್ಯ Scanner::advanceWhile(SkipWhileFunc Func) {
  auto Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ is_ns_hex_digit(ಧ್ರುವ ವರ್ಣ C) { ಪ್ರತ್ಯಾಗಮ isAlnum(C); }

ಸ್ಥಿರ ತಾರ್ಕಿಕ is_ns_word_char(ಧ್ರುವ ವರ್ಣ C) { ಪ್ರತ್ಯಾಗಮ C == '-' || isAlpha(C); }

ಶೂನ್ಯ Scanner::scan_ns_uri_char() {
  ಯಾವತ್ (ಸತ್ಯ) {
    ಯದಿ (Current == End)
      ಭಙ್ಗ;
    ಯದಿ ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } ಅನ್ಯಥಾ {
      ಭಙ್ಗ;
    }
  }
}

ತಾರ್ಕಿಕ Scanner::consume(uint32_t Expected) {
  ಯದಿ (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  ಯದಿ (Current == End)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  ಯದಿ (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ಶೂನ್ಯ Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  ಪ್ರತಿಜ್ಞಾ(Current <= End && "Skipped past the end");
}

ತಾರ್ಕಿಕ Scanner::isBlankOrBreak(StringRef::iterator Position) {
  ಯದಿ (Position == End)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

ತಾರ್ಕಿಕ Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  ಯದಿ (Position == End || isBlankOrBreak(Position))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::is_ಸಾಲು_Empty(StringRef ಸಾಲು) {
  ಪ್ರತಿ (ಧ್ರುವ auto *Position = ಸಾಲು.begin(); Position != ಸಾಲು.end(); ++Position)
    ಯದಿ (!isBlankOrBreak(Position))
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::consume_ಸಾಲು_Break_If_Present() {
  auto Next = skip_b_break(Current);
  ಯದಿ (Next == Current)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  Column = 0;
  ++ಸಾಲು;
  Current = Next;
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ಶೂನ್ಯ Scanner::save_Simple_ಕೀಲಿ_Candidate( TokenQueueT::iterator Tok
                                    , unsigned AtColumn
                                    , ತಾರ್ಕಿಕ IsRequired) {
  ಯದಿ (Is_Simple_ಕೀಲಿ_Allowed) {
    Simple_ಕೀಲಿ SK;
    SK.Tok = Tok;
    SK.ಸಾಲು = ಸಾಲು;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_ಕೀಲಿಗಳು.ತಳ್ಳು_back(SK);
  }
}

ಶೂನ್ಯ Scanner::remove_Stale_Simple_ಕೀಲಿ_Candidates() {
  ಪ್ರತಿ (SmallVectorImpl<Simple_ಕೀಲಿ>::iterator i = Simple_ಕೀಲಿಗಳು.begin();
                                            i != Simple_ಕೀಲಿಗಳು.end();) {
    ಯದಿ (i->ಸಾಲು != ಸಾಲು || i->Column + 1024 < Column) {
      ಯದಿ (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_ಕೀಲಿಗಳು.erase(i);
    } ಅನ್ಯಥಾ {
      ++i;
    }
  }
}

ಶೂನ್ಯ Scanner::remove_Simple_ಕೀಲಿ_Candidates_On_Flow_Level(unsigned Level) {
  ಯದಿ (!Simple_ಕೀಲಿಗಳು.empty() && (Simple_ಕೀಲಿಗಳು.end() - 1)->FlowLevel == Level)
    Simple_ಕೀಲಿಗಳು.ತೆಗೆ_back();
}

ತಾರ್ಕಿಕ Scanner::unrollIndent(ಪೂರ್ಣಾಙ್ಕ ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  ಯದಿ (FlowLevel != 0)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ಯಾವತ್ (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.ತಳ್ಳು_back(T);
    Indent = Indents.ತೆಗೆ_back_val();
  }

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::rollIndent( ಪೂರ್ಣಾಙ್ಕ ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator ಸೇರಿಸು_Point) {
  ಯದಿ (FlowLevel)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಯದಿ (Indent < ToColumn) {
    Indents.ತಳ್ಳು_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.ಸೇರಿಸು(ಸೇರಿಸು_Point, T);
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ಶೂನ್ಯ Scanner::skipComment() {
  ಯದಿ (Current == End || *Current != '#')
    ಪ್ರತ್ಯಾಗಮ;
  ಯಾವತ್ (ಸತ್ಯ) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    ಯದಿ (I == Current)
      ಭಙ್ಗ;
    Current = I;
    ++Column;
  }
}

ಶೂನ್ಯ Scanner::scanToNextToken() {
  ಯಾವತ್ (ಸತ್ಯ) {
    ಯಾವತ್ (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    ಯದಿ (i == Current)
      ಭಙ್ಗ;
    Current = i;
    ++ಸಾಲು;
    Column = 0;
    // New lines may start a simple key.
    ಯದಿ (!FlowLevel)
      Is_Simple_ಕೀಲಿ_Allowed = ಸತ್ಯ;
  }
}

ತಾರ್ಕಿಕ Scanner::scanStreamStart() {
  IsStartOfStream = ಅಸತ್ಯ;

  EncodingInfo EI = getUnicodeEncoding(current_ಒಳಹರಿವು());

  Token T;
  T.Kind = Token::TK_StreamStart;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.ತಳ್ಳು_back(T);
  Current += EI.second;
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  ಯದಿ (Column != 0) {
    Column = 0;
    ++ಸಾಲು;
  }

  unrollIndent(-1);
  Simple_ಕೀಲಿಗಳು.clear();
  Is_Simple_ಕೀಲಿ_Allowed = ಅಸತ್ಯ;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow = ಅಸತ್ಯ;

  Token T;
  T.Kind = Token::TK_StreamEnd;
  T.Range = StringRef(Current, 0);
  TokenQueue.ತಳ್ಳು_back(T);
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::scanDirective() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_ಕೀಲಿಗಳು.clear();
  Is_Simple_ಕೀಲಿ_Allowed = ಅಸತ್ಯ;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow = ಅಸತ್ಯ;

  StringRef::iterator Start = Current;
  consume('%');
  StringRef::iterator NameStart = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(NameStart, Current - NameStart);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  ಯದಿ (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ತಳ್ಳು_back(T);
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  } ಅನ್ಯಥಾ ಯದಿ(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ತಳ್ಳು_back(T);
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::scanDocumentIndicator(ತಾರ್ಕಿಕ IsStart) {
  unrollIndent(-1);
  Simple_ಕೀಲಿಗಳು.clear();
  Is_Simple_ಕೀಲಿ_Allowed = ಅಸತ್ಯ;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow = ಅಸತ್ಯ;

  Token T;
  T.Kind = IsStart ? Token::TK_DocumentStart : Token::TK_DocumentEnd;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.ತಳ್ಳು_back(T);
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::scanFlowCollectionStart(ತಾರ್ಕಿಕ IsSequence) {
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ತಳ್ಳು_back(T);

  // [ and { may begin a simple key.
  save_Simple_ಕೀಲಿ_Candidate(--TokenQueue.end(), Column - 1, ಅಸತ್ಯ);

  // And may also be followed by a simple key.
  Is_Simple_ಕೀಲಿ_Allowed = ಸತ್ಯ;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow = ಅಸತ್ಯ;
  ++FlowLevel;
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::scanFlowCollectionEnd(ತಾರ್ಕಿಕ IsSequence) {
  remove_Simple_ಕೀಲಿ_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ಕೀಲಿ_Allowed = ಅಸತ್ಯ;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow = ಸತ್ಯ;
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceEnd
                      : Token::TK_FlowMappingEnd;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ತಳ್ಳು_back(T);
  ಯದಿ (FlowLevel)
    --FlowLevel;
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::scanFlowEntry() {
  remove_Simple_ಕೀಲಿ_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ಕೀಲಿ_Allowed = ಸತ್ಯ;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow = ಅಸತ್ಯ;
  Token T;
  T.Kind = Token::TK_FlowEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ತಳ್ಳು_back(T);
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::scanBlockEntry() {
  rollIndent(Column, Token::TK_BlockSequenceStart, TokenQueue.end());
  remove_Simple_ಕೀಲಿ_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ಕೀಲಿ_Allowed = ಸತ್ಯ;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow = ಅಸತ್ಯ;
  Token T;
  T.Kind = Token::TK_BlockEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ತಳ್ಳು_back(T);
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::scan_ಕೀಲಿ() {
  ಯದಿ (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  remove_Simple_ಕೀಲಿ_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ಕೀಲಿ_Allowed = !FlowLevel;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow = ಅಸತ್ಯ;

  Token T;
  T.Kind = Token::TK_ಕೀಲಿ;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ತಳ್ಳು_back(T);
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::scan_ಮೌಲ್ಯ() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  ಯದಿ (!Simple_ಕೀಲಿಗಳು.empty()) {
    Simple_ಕೀಲಿ SK = Simple_ಕೀಲಿಗಳು.ತೆಗೆ_back_val();
    Token T;
    T.Kind = Token::TK_ಕೀಲಿ;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    ಪ್ರತಿ (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      ಯದಿ (i == SK.Tok)
        ಭಙ್ಗ;
    }
    ಯದಿ (i == e) {
      Failed = ಸತ್ಯ;
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }
    i = TokenQueue.ಸೇರಿಸು(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    Is_Simple_ಕೀಲಿ_Allowed = ಅಸತ್ಯ;
  } ಅನ್ಯಥಾ {
    ಯದಿ (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    Is_Simple_ಕೀಲಿ_Allowed = !FlowLevel;
  }
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow = ಅಸತ್ಯ;

  Token T;
  T.Kind = Token::TK_ಮೌಲ್ಯ;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ತಳ್ಳು_back(T);
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE ಸ್ಥಿರ ತಾರ್ಕಿಕ
wasEscaped(StringRef::iterator First, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
ಸ್ಥಿರ ತಾರ್ಕಿಕ wasEscaped(StringRef::iterator First,
                       StringRef::iterator Position) {
  ಪ್ರತಿಜ್ಞಾ(Position - 1 >= First);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  ಯಾವತ್ (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  ಪ್ರತ್ಯಾಗಮ (Position - 1 - I) % 2 == 1;
}

ತಾರ್ಕಿಕ Scanner::scanFlowScalar(ತಾರ್ಕಿಕ IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  ಯದಿ (IsDoubleQuoted) {
    ಕುರು {
      ++Current;
      ಯಾವತ್ (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } ಯಾವತ್ (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } ಅನ್ಯಥಾ {
    skip(1);
    ಯಾವತ್ (Current != End) {
      // Skip a ' followed by another '.
      ಯದಿ (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        ಅನುವರ್ತ;
      } ಅನ್ಯಥಾ ಯದಿ (*Current == '\'')
        ಭಙ್ಗ;
      StringRef::iterator i = skip_nb_char(Current);
      ಯದಿ (i == Current) {
        i = skip_b_break(Current);
        ಯದಿ (i == Current)
          ಭಙ್ಗ;
        Current = i;
        Column = 0;
        ++ಸಾಲು;
      } ಅನ್ಯಥಾ {
        ಯದಿ (i == End)
          ಭಙ್ಗ;
        Current = i;
        ++Column;
      }
    }
  }

  ಯದಿ (Current == End) {
    setError("Expected quote at end of scalar", Current);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ತಳ್ಳು_back(T);

  save_Simple_ಕೀಲಿ_Candidate(--TokenQueue.end(), ColStart, ಅಸತ್ಯ);

  Is_Simple_ಕೀಲಿ_Allowed = ಅಸತ್ಯ;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow = ಸತ್ಯ;

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  unsigned LeadingBlanks = 0;
  ಪ್ರತಿಜ್ಞಾ(Indent >= -1 && "Indent must be >= -1 !");
  unsigned indent = static_cast<unsigned>(Indent + 1);
  ಯಾವತ್ (Current != End) {
    ಯದಿ (*Current == '#')
      ಭಙ್ಗ;

    ಯಾವತ್ (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      ಯದಿ (i == Current)
        ಭಙ್ಗ;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    ಯದಿ (!isBlankOrBreak(Current))
      ಭಙ್ಗ;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    ಯಾವತ್ (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      ಯದಿ (i != Tmp) {
        ಯದಿ (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
        }
        Tmp = i;
        ++Column;
      } ಅನ್ಯಥಾ {
        i = skip_b_break(Tmp);
        ಯದಿ (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++ಸಾಲು;
      }
    }

    ಯದಿ (!FlowLevel && Column < indent)
      ಭಙ್ಗ;

    Current = Tmp;
  }
  ಯದಿ (Start == Current) {
    setError("Got empty plain scalar", Start);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ತಳ್ಳು_back(T);

  // Plain scalars can be simple keys.
  save_Simple_ಕೀಲಿ_Candidate(--TokenQueue.end(), ColStart, ಅಸತ್ಯ);

  Is_Simple_ಕೀಲಿ_Allowed = ಅಸತ್ಯ;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow = ಅಸತ್ಯ;

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::scanAliasOrAnchor(ತಾರ್ಕಿಕ IsAlias) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1);
  ಯಾವತ್ (Current != End) {
    ಯದಿ (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      ಭಙ್ಗ;
    StringRef::iterator i = skip_ns_char(Current);
    ಯದಿ (i == Current)
      ಭಙ್ಗ;
    Current = i;
    ++Column;
  }

  ಯದಿ (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ತಳ್ಳು_back(T);

  // Alias and anchors can be simple keys.
  save_Simple_ಕೀಲಿ_Candidate(--TokenQueue.end(), ColStart, ಅಸತ್ಯ);

  Is_Simple_ಕೀಲಿ_Allowed = ಅಸತ್ಯ;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow = ಅಸತ್ಯ;

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::scanBlockScalarIndicators(ವರ್ಣ &StyleIndicator,
                                        ವರ್ಣ &ChompingIndicator,
                                        unsigned &IndentIndicator,
                                        ತಾರ್ಕಿಕ &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  ಯದಿ (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ವರ್ಣ Scanner::scanBlockStyleIndicator() {
  ವರ್ಣ Indicator = ' ';
  ಯದಿ (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  ಪ್ರತ್ಯಾಗಮ Indicator;
}

ವರ್ಣ Scanner::scanBlockChompingIndicator() {
  ವರ್ಣ Indicator = ' ';
  ಯದಿ (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  ಪ್ರತ್ಯಾಗಮ Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
ಸ್ಥಿರ unsigned get_Chomped_ಸಾಲು_Breaks(ವರ್ಣ ChompingIndicator,
                                     unsigned ಸಾಲು_Breaks, StringRef Str) {
  ಯದಿ (ChompingIndicator == '-') // Strip all line breaks.
    ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ (ChompingIndicator == '+') // Keep all line breaks.
    ಪ್ರತ್ಯಾಗಮ ಸಾಲು_Breaks;
  // Clip trailing lines.
  ಪ್ರತ್ಯಾಗಮ Str.empty() ? 0 : 1;
}

unsigned Scanner::scanBlockIndentationIndicator() {
  unsigned Indent = 0;
  ಯದಿ (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = unsigned(*Current - '0');
    skip(1);
  }
  ಪ್ರತ್ಯಾಗಮ Indent;
}

ತಾರ್ಕಿಕ Scanner::scanBlockScalarHeader(ವರ್ಣ &ChompingIndicator,
                                    unsigned &IndentIndicator, ತಾರ್ಕಿಕ &IsDone) {
  auto Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  ಯದಿ (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  ಯದಿ (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ತಳ್ಳು_back(T);
    IsDone = ಸತ್ಯ;
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

  ಯದಿ (!consume_ಸಾಲು_Break_If_Present()) {
    setError("Expected a line break after block scalar header", Current);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::findBlockScalarIndent(unsigned &BlockIndent,
                                    unsigned BlockExitIndent,
                                    unsigned &ಸಾಲು_Breaks, ತಾರ್ಕಿಕ &IsDone) {
  unsigned Max_All_Space_ಸಾಲು_Characters = 0;
  StringRef::iterator Longest_All_Space_ಸಾಲು;

  ಯಾವತ್ (ಸತ್ಯ) {
    advanceWhile(&Scanner::skip_s_space);
    ಯದಿ (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      ಯದಿ (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = ಸತ್ಯ;
        ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      ಯದಿ (Max_All_Space_ಸಾಲು_Characters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_ಸಾಲು);
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      }
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }
    ಯದಿ (skip_b_break(Current) != Current &&
        Column > Max_All_Space_ಸಾಲು_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_ಸಾಲು_Characters = Column;
      Longest_All_Space_ಸಾಲು = Current;
    }

    // Check for EOF.
    ಯದಿ (Current == End) {
      IsDone = ಸತ್ಯ;
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }

    ಯದಿ (!consume_ಸಾಲು_Break_If_Present()) {
      IsDone = ಸತ್ಯ;
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }
    ++ಸಾಲು_Breaks;
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::scanBlockScalarIndent(unsigned BlockIndent,
                                    unsigned BlockExitIndent, ತಾರ್ಕಿಕ &IsDone) {
  // Skip the indentation.
  ಯಾವತ್ (Column < BlockIndent) {
    auto I = skip_s_space(Current);
    ಯದಿ (I == Current)
      ಭಙ್ಗ;
    Current = I;
    ++Column;
  }

  ಯದಿ (skip_nb_char(Current) == Current)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ಯದಿ (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = ಸತ್ಯ;
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

  ಯದಿ (Column < BlockIndent) {
    ಯದಿ (Current != End && *Current == '#') { // Trailing comment.
      IsDone = ಸತ್ಯ;
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }
    setError("A text line is less indented than the block scalar", Current);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ; // A normal text line.
}

ತಾರ್ಕಿಕ Scanner::scanBlockScalar(ತಾರ್ಕಿಕ IsLiteral) {
  ಪ್ರತಿಜ್ಞಾ(*Current == '|' || *Current == '>');
  ವರ್ಣ StyleIndicator;
  ವರ್ಣ ChompingIndicator;
  unsigned BlockIndent;
  ತಾರ್ಕಿಕ IsDone = ಅಸತ್ಯ;
  ಯದಿ (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (IsDone)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ತಾರ್ಕಿಕ IsFolded = StyleIndicator == '>';

  ಧ್ರುವ auto *Start = Current;
  unsigned BlockExitIndent = Indent < 0 ? 0 : (unsigned)Indent;
  unsigned ಸಾಲು_Breaks = 0;
  ಯದಿ (BlockIndent == 0) {
    ಯದಿ (!findBlockScalarIndent(BlockIndent, BlockExitIndent, ಸಾಲು_Breaks,
                               IsDone))
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  ಯಾವತ್ (!IsDone) {
    ಯದಿ (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    ಯದಿ (IsDone)
      ಭಙ್ಗ;

    // Parse the current line.
    auto ಸಾಲು_Start = Current;
    advanceWhile(&Scanner::skip_nb_char);
    ಯದಿ (ಸಾಲು_Start != Current) {
      ಯದಿ (ಸಾಲು_Breaks && IsFolded && !Scanner::is_ಸಾಲು_Empty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        ಯದಿ (ಸಾಲು_Breaks == 1) {
          Str.append(ಸಾಲು_Breaks,
                     is_ಸಾಲು_Empty(StringRef(ಸಾಲು_Start, Current - ಸಾಲು_Start))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        ಸಾಲು_Breaks--;
      }
      Str.append(ಸಾಲು_Breaks, '\n');
      Str.append(StringRef(ಸಾಲು_Start, Current - ಸಾಲು_Start));
      ಸಾಲು_Breaks = 0;
    }

    // Check for EOF.
    ಯದಿ (Current == End)
      ಭಙ್ಗ;

    ಯದಿ (!consume_ಸಾಲು_Break_If_Present())
      ಭಙ್ಗ;
    ++ಸಾಲು_Breaks;
  }

  ಯದಿ (Current == End && !ಸಾಲು_Breaks)
    // Ensure that there is at least one line break before the end of file.
    ಸಾಲು_Breaks = 1;
  Str.append(get_Chomped_ಸಾಲು_Breaks(ChompingIndicator, ಸಾಲು_Breaks, Str), '\n');

  // New lines may start a simple key.
  ಯದಿ (!FlowLevel)
    Is_Simple_ಕೀಲಿ_Allowed = ಸತ್ಯ;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow = ಅಸತ್ಯ;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.ಮೌಲ್ಯ = std::string(Str);
  TokenQueue.ತಳ್ಳು_back(T);
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::scanTag() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1); // Eat !.
  ಯದಿ (Current == End || isBlankOrBreak(Current)); // An empty tag.
  ಅನ್ಯಥಾ ಯದಿ (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    ಯದಿ (!consume('>'))
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  } ಅನ್ಯಥಾ {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.Kind = Token::TK_Tag;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ತಳ್ಳು_back(T);

  // Tags can be simple keys.
  save_Simple_ಕೀಲಿ_Candidate(--TokenQueue.end(), ColStart, ಅಸತ್ಯ);

  Is_Simple_ಕೀಲಿ_Allowed = ಅಸತ್ಯ;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow = ಅಸತ್ಯ;

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ Scanner::fetchMoreTokens() {
  ಯದಿ (IsStartOfStream)
    ಪ್ರತ್ಯಾಗಮ scanStreamStart();

  scanToNextToken();

  ಯದಿ (Current == End)
    ಪ್ರತ್ಯಾಗಮ scanStreamEnd();

  remove_Stale_Simple_ಕೀಲಿ_Candidates();

  unrollIndent(Column);

  ಯದಿ (Column == 0 && *Current == '%')
    ಪ್ರತ್ಯಾಗಮ scanDirective();

  ಯದಿ (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    ಪ್ರತ್ಯಾಗಮ scanDocumentIndicator(ಸತ್ಯ);

  ಯದಿ (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    ಪ್ರತ್ಯಾಗಮ scanDocumentIndicator(ಅಸತ್ಯ);

  ಯದಿ (*Current == '[')
    ಪ್ರತ್ಯಾಗಮ scanFlowCollectionStart(ಸತ್ಯ);

  ಯದಿ (*Current == '{')
    ಪ್ರತ್ಯಾಗಮ scanFlowCollectionStart(ಅಸತ್ಯ);

  ಯದಿ (*Current == ']')
    ಪ್ರತ್ಯಾಗಮ scanFlowCollectionEnd(ಸತ್ಯ);

  ಯದಿ (*Current == '}')
    ಪ್ರತ್ಯಾಗಮ scanFlowCollectionEnd(ಅಸತ್ಯ);

  ಯದಿ (*Current == ',')
    ಪ್ರತ್ಯಾಗಮ scanFlowEntry();

  ಯದಿ (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    ಪ್ರತ್ಯಾಗಮ scanBlockEntry();

  ಯದಿ (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    ಪ್ರತ್ಯಾಗಮ scan_ಕೀಲಿ();

  ಯದಿ (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || Is_Adjacent_ಮೌಲ್ಯ_Allowed_In_Flow))
    ಪ್ರತ್ಯಾಗಮ scan_ಮೌಲ್ಯ();

  ಯದಿ (*Current == '*')
    ಪ್ರತ್ಯಾಗಮ scanAliasOrAnchor(ಸತ್ಯ);

  ಯದಿ (*Current == '&')
    ಪ್ರತ್ಯಾಗಮ scanAliasOrAnchor(ಅಸತ್ಯ);

  ಯದಿ (*Current == '!')
    ಪ್ರತ್ಯಾಗಮ scanTag();

  ಯದಿ (*Current == '|' && !FlowLevel)
    ಪ್ರತ್ಯಾಗಮ scanBlockScalar(ಸತ್ಯ);

  ಯದಿ (*Current == '>' && !FlowLevel)
    ಪ್ರತ್ಯಾಗಮ scanBlockScalar(ಅಸತ್ಯ);

  ಯದಿ (*Current == '\'')
    ಪ್ರತ್ಯಾಗಮ scanFlowScalar(ಅಸತ್ಯ);

  ಯದಿ (*Current == '"')
    ಪ್ರತ್ಯಾಗಮ scanFlowScalar(ಸತ್ಯ);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  ಯದಿ ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    ಪ್ರತ್ಯಾಗಮ scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

Stream::Stream(StringRef ಒಳಹರಿವು, SourceMgr &SM, ತಾರ್ಕಿಕ ShowColors,
               std::error_code *EC)
    : scanner(ನೂತನ Scanner(ಒಳಹರಿವು, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef ಒಳಹರಿವು_Buffer, SourceMgr &SM, ತಾರ್ಕಿಕ ShowColors,
               std::error_code *EC)
    : scanner(ನೂತನ Scanner(ಒಳಹರಿವು_Buffer, SM, ShowColors, EC)) {}

Stream::~Stream() = ಸ್ವಭಾವ;

ತಾರ್ಕಿಕ Stream::failed() { ಪ್ರತ್ಯಾಗಮ scanner->failed(); }

ಶೂನ್ಯ Stream::printError(ಗಂಟು *N, ಧ್ರುವ Twine &ಸಂದೇಶ_2, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), ಸಂದೇಶ_2, Kind);
}

ಶೂನ್ಯ Stream::printError(ಧ್ರುವ SMRange &Range, ಧ್ರುವ Twine &ಸಂದೇಶ_2,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, ಸಂದೇಶ_2, Range);
}

document_iterator Stream::begin() {
  ಯದಿ (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(ನೂತನ Document(*ಏತತ್));
  ಪ್ರತ್ಯಾಗಮ document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  ಪ್ರತ್ಯಾಗಮ document_iterator();
}

ಶೂನ್ಯ Stream::skip() {
  ಪ್ರತಿ (Document &Doc : *ಏತತ್)
    Doc.skip();
}

ಗಂಟು::ಗಂಟು(unsigned ಪೂರ್ಣಾಙ್ಕ Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string ಗಂಟು::getVerbatimTag() ಧ್ರುವ {
  StringRef Raw = getRawTag();
  ಯದಿ (!Raw.empty() && Raw != "!") {
    std::string Ret;
    ಯದಿ (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      ಪ್ರತ್ಯಾಗಮ Ret;
    } ಅನ್ಯಥಾ ಯದಿ (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      ಪ್ರತ್ಯಾಗಮ Ret;
    } ಅನ್ಯಥಾ {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      ಯದಿ (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      ಅನ್ಯಥಾ {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      ಪ್ರತ್ಯಾಗಮ Ret;
    }
  }

  ವಿಕಲ್ಪ (getType()) {
  ಪಕ್ಷ NK_Null:
    ಪ್ರತ್ಯಾಗಮ "tag:yaml.org,2002:null";
  ಪಕ್ಷ NK_Scalar:
  ಪಕ್ಷ NK_BlockScalar:
    // TODO: Tag resolution.
    ಪ್ರತ್ಯಾಗಮ "tag:yaml.org,2002:str";
  ಪಕ್ಷ NK_Mapping:
    ಪ್ರತ್ಯಾಗಮ "tag:yaml.org,2002:map";
  ಪಕ್ಷ NK_Sequence:
    ಪ್ರತ್ಯಾಗಮ "tag:yaml.org,2002:seq";
  }

  ಪ್ರತ್ಯಾಗಮ "";
}

Token &ಗಂಟು::peekNext() {
  ಪ್ರತ್ಯಾಗಮ Doc->peekNext();
}

Token ಗಂಟು::getNext() {
  ಪ್ರತ್ಯಾಗಮ Doc->getNext();
}

ಗಂಟು *ಗಂಟು::parse_Block_ಗಂಟು() {
  ಪ್ರತ್ಯಾಗಮ Doc->parse_Block_ಗಂಟು();
}

BumpPtrAllocator &ಗಂಟು::getAllocator() {
  ಪ್ರತ್ಯಾಗಮ Doc->ಗಂಟು_Allocator;
}

ಶೂನ್ಯ ಗಂಟು::setError(ಧ್ರುವ Twine &ಸಂದೇಶ_2, Token &Tok) ಧ್ರುವ {
  Doc->setError(ಸಂದೇಶ_2, Tok);
}

ತಾರ್ಕಿಕ ಗಂಟು::failed() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ Doc->failed();
}

StringRef Scalar_ಗಂಟು::get_ಮೌಲ್ಯ(SmallVectorImpl<ವರ್ಣ> &Storage) ಧ್ರುವ {
  ಯದಿ (ಮೌಲ್ಯ[0] == '"')
    ಪ್ರತ್ಯಾಗಮ get_Double_Quoted_ಮೌಲ್ಯ(ಮೌಲ್ಯ, Storage);
  ಯದಿ (ಮೌಲ್ಯ[0] == '\'')
    ಪ್ರತ್ಯಾಗಮ get_Single_Quoted_ಮೌಲ್ಯ(ಮೌಲ್ಯ, Storage);
  ಪ್ರತ್ಯಾಗಮ get_Plain_ಮೌಲ್ಯ(ಮೌಲ್ಯ, Storage);
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
ಸ್ಥಿರ StringRef
parse_Scalar_ಮೌಲ್ಯ(StringRef Unquoted_ಮೌಲ್ಯ, SmallVectorImpl<ವರ್ಣ> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<ವರ್ಣ> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_ಮೌಲ್ಯ.find_first_of(LookupChars);
  ಯದಿ (I == StringRef::npos)
    ಪ್ರತ್ಯಾಗಮ Unquoted_ಮೌಲ್ಯ;

  Storage.clear();
  Storage.reserve(Unquoted_ಮೌಲ್ಯ.size());
  ವರ್ಣ Last_New_ಸಾಲು_Added_As = '\0';
  ಪ್ರತಿ (; I != StringRef::npos; I = Unquoted_ಮೌಲ್ಯ.find_first_of(LookupChars)) {
    ಯದಿ (Unquoted_ಮೌಲ್ಯ[I] != '\r' && Unquoted_ಮೌಲ್ಯ[I] != '\n') {
      llvm::append_range(Storage, Unquoted_ಮೌಲ್ಯ.take_front(I));
      Unquoted_ಮೌಲ್ಯ = UnescapeCallback(Unquoted_ಮೌಲ್ಯ.drop_front(I), Storage);
      Last_New_ಸಾಲು_Added_As = '\0';
      ಅನುವರ್ತ;
    }
    ಯದಿ (size_t LastNonSWhite = Unquoted_ಮೌಲ್ಯ.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_ಮೌಲ್ಯ.take_front(LastNonSWhite + 1));
      Storage.ತಳ್ಳು_back(' ');
      Last_New_ಸಾಲು_Added_As = ' ';
    } ಅನ್ಯಥಾ {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      ವಿಕಲ್ಪ (Last_New_ಸಾಲು_Added_As) {
      ಪಕ್ಷ ' ':
        ಪ್ರತಿಜ್ಞಾ(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        Last_New_ಸಾಲು_Added_As = '\n';
        ಭಙ್ಗ;
      ಪಕ್ಷ '\n':
        ಪ್ರತಿಜ್ಞಾ(!Storage.empty() && Storage.back() == '\n');
        Storage.ತಳ್ಳು_back('\n');
        ಭಙ್ಗ;
      ಸ್ವಭಾವ:
        Storage.ತಳ್ಳು_back(' ');
        Last_New_ಸಾಲು_Added_As = ' ';
        ಭಙ್ಗ;
      }
    }
    // Handle Windows-style EOL
    ಯದಿ (Unquoted_ಮೌಲ್ಯ.substr(I, 2) == "\r\n")
      I++;
    Unquoted_ಮೌಲ್ಯ = Unquoted_ಮೌಲ್ಯ.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_ಮೌಲ್ಯ);
  ಪ್ರತ್ಯಾಗಮ StringRef(Storage.begin(), Storage.size());
}

StringRef
Scalar_ಗಂಟು::get_Double_Quoted_ಮೌಲ್ಯ(StringRef Raw_ಮೌಲ್ಯ,
                                 SmallVectorImpl<ವರ್ಣ> &Storage) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(Raw_ಮೌಲ್ಯ.size() >= 2 && Raw_ಮೌಲ್ಯ.front() == '"' &&
         Raw_ಮೌಲ್ಯ.back() == '"');
  StringRef Unquoted_ಮೌಲ್ಯ = Raw_ಮೌಲ್ಯ.substr(1, Raw_ಮೌಲ್ಯ.size() - 2);

  auto UnescapeFunc = [ಏತತ್](StringRef Unquoted_ಮೌಲ್ಯ,
                             SmallVectorImpl<ವರ್ಣ> &Storage) {
    ಪ್ರತಿಜ್ಞಾ(Unquoted_ಮೌಲ್ಯ.take_front(1) == "\\");
    ಯದಿ (Unquoted_ಮೌಲ್ಯ.size() == 1) {
      Token T;
      T.Range = Unquoted_ಮೌಲ್ಯ;
      setError("Unrecognized escape code", T);
      Storage.clear();
      ಪ್ರತ್ಯಾಗಮ StringRef();
    }
    Unquoted_ಮೌಲ್ಯ = Unquoted_ಮೌಲ್ಯ.drop_front(1);
    ವಿಕಲ್ಪ (Unquoted_ಮೌಲ್ಯ[0]) {
    ಸ್ವಭಾವ: {
      Token T;
      T.Range = Unquoted_ಮೌಲ್ಯ.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      ಪ್ರತ್ಯಾಗಮ StringRef();
    }
    ಪಕ್ಷ '\r':
      // Shrink the Windows-style EOL.
      ಯದಿ (Unquoted_ಮೌಲ್ಯ.size() >= 2 && Unquoted_ಮೌಲ್ಯ[1] == '\n')
        Unquoted_ಮೌಲ್ಯ = Unquoted_ಮೌಲ್ಯ.drop_front(1);
      [[fallthrough]];
    ಪಕ್ಷ '\n':
      ಪ್ರತ್ಯಾಗಮ Unquoted_ಮೌಲ್ಯ.drop_front(1).ltrim(" \t");
    ಪಕ್ಷ '0':
      Storage.ತಳ್ಳು_back(0x00);
      ಭಙ್ಗ;
    ಪಕ್ಷ 'a':
      Storage.ತಳ್ಳು_back(0x07);
      ಭಙ್ಗ;
    ಪಕ್ಷ 'b':
      Storage.ತಳ್ಳು_back(0x08);
      ಭಙ್ಗ;
    ಪಕ್ಷ 't':
    ಪಕ್ಷ 0x09:
      Storage.ತಳ್ಳು_back(0x09);
      ಭಙ್ಗ;
    ಪಕ್ಷ 'n':
      Storage.ತಳ್ಳು_back(0x0A);
      ಭಙ್ಗ;
    ಪಕ್ಷ 'v':
      Storage.ತಳ್ಳು_back(0x0B);
      ಭಙ್ಗ;
    ಪಕ್ಷ 'f':
      Storage.ತಳ್ಳು_back(0x0C);
      ಭಙ್ಗ;
    ಪಕ್ಷ 'r':
      Storage.ತಳ್ಳು_back(0x0D);
      ಭಙ್ಗ;
    ಪಕ್ಷ 'e':
      Storage.ತಳ್ಳು_back(0x1B);
      ಭಙ್ಗ;
    ಪಕ್ಷ ' ':
      Storage.ತಳ್ಳು_back(0x20);
      ಭಙ್ಗ;
    ಪಕ್ಷ '"':
      Storage.ತಳ್ಳು_back(0x22);
      ಭಙ್ಗ;
    ಪಕ್ಷ '/':
      Storage.ತಳ್ಳು_back(0x2F);
      ಭಙ್ಗ;
    ಪಕ್ಷ '\\':
      Storage.ತಳ್ಳು_back(0x5C);
      ಭಙ್ಗ;
    ಪಕ್ಷ 'N':
      encodeUTF8(0x85, Storage);
      ಭಙ್ಗ;
    ಪಕ್ಷ '_':
      encodeUTF8(0xA0, Storage);
      ಭಙ್ಗ;
    ಪಕ್ಷ 'L':
      encodeUTF8(0x2028, Storage);
      ಭಙ್ಗ;
    ಪಕ್ಷ 'P':
      encodeUTF8(0x2029, Storage);
      ಭಙ್ಗ;
    ಪಕ್ಷ 'x': {
      ಯದಿ (Unquoted_ಮೌಲ್ಯ.size() < 3)
        // TODO: Report error.
        ಭಙ್ಗ;
      unsigned ಪೂರ್ಣಾಙ್ಕ Unicode_Scalar_ಮೌಲ್ಯ;
      ಯದಿ (Unquoted_ಮೌಲ್ಯ.substr(1, 2).getAsInteger(16, Unicode_Scalar_ಮೌಲ್ಯ))
        // TODO: Report error.
        Unicode_Scalar_ಮೌಲ್ಯ = 0xFFFD;
      encodeUTF8(Unicode_Scalar_ಮೌಲ್ಯ, Storage);
      ಪ್ರತ್ಯಾಗಮ Unquoted_ಮೌಲ್ಯ.drop_front(3);
    }
    ಪಕ್ಷ 'u': {
      ಯದಿ (Unquoted_ಮೌಲ್ಯ.size() < 5)
        // TODO: Report error.
        ಭಙ್ಗ;
      unsigned ಪೂರ್ಣಾಙ್ಕ Unicode_Scalar_ಮೌಲ್ಯ;
      ಯದಿ (Unquoted_ಮೌಲ್ಯ.substr(1, 4).getAsInteger(16, Unicode_Scalar_ಮೌಲ್ಯ))
        // TODO: Report error.
        Unicode_Scalar_ಮೌಲ್ಯ = 0xFFFD;
      encodeUTF8(Unicode_Scalar_ಮೌಲ್ಯ, Storage);
      ಪ್ರತ್ಯಾಗಮ Unquoted_ಮೌಲ್ಯ.drop_front(5);
    }
    ಪಕ್ಷ 'U': {
      ಯದಿ (Unquoted_ಮೌಲ್ಯ.size() < 9)
        // TODO: Report error.
        ಭಙ್ಗ;
      unsigned ಪೂರ್ಣಾಙ್ಕ Unicode_Scalar_ಮೌಲ್ಯ;
      ಯದಿ (Unquoted_ಮೌಲ್ಯ.substr(1, 8).getAsInteger(16, Unicode_Scalar_ಮೌಲ್ಯ))
        // TODO: Report error.
        Unicode_Scalar_ಮೌಲ್ಯ = 0xFFFD;
      encodeUTF8(Unicode_Scalar_ಮೌಲ್ಯ, Storage);
      ಪ್ರತ್ಯಾಗಮ Unquoted_ಮೌಲ್ಯ.drop_front(9);
    }
    }
    ಪ್ರತ್ಯಾಗಮ Unquoted_ಮೌಲ್ಯ.drop_front(1);
  };

  ಪ್ರತ್ಯಾಗಮ parse_Scalar_ಮೌಲ್ಯ(Unquoted_ಮೌಲ್ಯ, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_ಗಂಟು::get_Single_Quoted_ಮೌಲ್ಯ(StringRef Raw_ಮೌಲ್ಯ,
                                           SmallVectorImpl<ವರ್ಣ> &Storage) {
  ಪ್ರತಿಜ್ಞಾ(Raw_ಮೌಲ್ಯ.size() >= 2 && Raw_ಮೌಲ್ಯ.front() == '\'' &&
         Raw_ಮೌಲ್ಯ.back() == '\'');
  StringRef Unquoted_ಮೌಲ್ಯ = Raw_ಮೌಲ್ಯ.substr(1, Raw_ಮೌಲ್ಯ.size() - 2);

  auto UnescapeFunc = [](StringRef Unquoted_ಮೌಲ್ಯ,
                         SmallVectorImpl<ವರ್ಣ> &Storage) {
    ಪ್ರತಿಜ್ಞಾ(Unquoted_ಮೌಲ್ಯ.take_front(2) == "''");
    Storage.ತಳ್ಳು_back('\'');
    ಪ್ರತ್ಯಾಗಮ Unquoted_ಮೌಲ್ಯ.drop_front(2);
  };

  ಪ್ರತ್ಯಾಗಮ parse_Scalar_ಮೌಲ್ಯ(Unquoted_ಮೌಲ್ಯ, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_ಗಂಟು::get_Plain_ಮೌಲ್ಯ(StringRef Raw_ಮೌಲ್ಯ,
                                    SmallVectorImpl<ವರ್ಣ> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_ಮೌಲ್ಯ = Raw_ಮೌಲ್ಯ.rtrim("\r\n \t");
  ಪ್ರತ್ಯಾಗಮ parse_Scalar_ಮೌಲ್ಯ(Raw_ಮೌಲ್ಯ, Storage, "\r\n", nullptr);
}

ಗಂಟು *ಕೀಲಿ_ಮೌಲ್ಯ_ಗಂಟು::get_ಕೀಲಿ() {
  ಯದಿ (ಕೀಲಿ)
    ಪ್ರತ್ಯಾಗಮ ಕೀಲಿ;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    ಯದಿ (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_ಮೌಲ್ಯ
        || t.Kind == Token::TK_Error) {
      ಪ್ರತ್ಯಾಗಮ ಕೀಲಿ = ನೂತನ (getAllocator()) Null_ಗಂಟು(Doc);
    }
    ಯದಿ (t.Kind == Token::TK_ಕೀಲಿ)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  ಯದಿ (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_ಮೌಲ್ಯ) {
    ಪ್ರತ್ಯಾಗಮ ಕೀಲಿ = ನೂತನ (getAllocator()) Null_ಗಂಟು(Doc);
  }

  // We've got a normal key.
  ಪ್ರತ್ಯಾಗಮ ಕೀಲಿ = parse_Block_ಗಂಟು();
}

ಗಂಟು *ಕೀಲಿ_ಮೌಲ್ಯ_ಗಂಟು::get_ಮೌಲ್ಯ() {
  ಯದಿ (ಮೌಲ್ಯ)
    ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ;

  ಯದಿ (ಗಂಟು* ಕೀಲಿ = get_ಕೀಲಿ())
    ಕೀಲಿ->skip();
  ಅನ್ಯಥಾ {
    setError("Null key in Key Value.", peekNext());
    ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ = ನೂತನ (getAllocator()) Null_ಗಂಟು(Doc);
  }

  ಯದಿ (failed())
    ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ = ನೂತನ (getAllocator()) Null_ಗಂಟು(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    ಯದಿ (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_ಕೀಲಿ
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ = ನೂತನ (getAllocator()) Null_ಗಂಟು(Doc);
    }

    ಯದಿ (t.Kind != Token::TK_ಮೌಲ್ಯ) {
      setError("Unexpected token in Key Value.", t);
      ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ = ನೂತನ (getAllocator()) Null_ಗಂಟು(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  ಯದಿ (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_ಕೀಲಿ) {
    ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ = ನೂತನ (getAllocator()) Null_ಗಂಟು(Doc);
  }

  // We got a normal value.
  ಪ್ರತ್ಯಾಗಮ ಮೌಲ್ಯ = parse_Block_ಗಂಟು();
}

ಶೂನ್ಯ Mapping_ಗಂಟು::increment() {
  ಯದಿ (failed()) {
    IsAtEnd = ಸತ್ಯ;
    CurrentEntry = nullptr;
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ (CurrentEntry) {
    CurrentEntry->skip();
    ಯದಿ (Type == MT_Inline) {
      IsAtEnd = ಸತ್ಯ;
      CurrentEntry = nullptr;
      ಪ್ರತ್ಯಾಗಮ;
    }
  }
  Token T = peekNext();
  ಯದಿ (T.Kind == Token::TK_ಕೀಲಿ || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = ನೂತನ (getAllocator()) ಕೀಲಿ_ಮೌಲ್ಯ_ಗಂಟು(Doc);
  } ಅನ್ಯಥಾ ಯದಿ (Type == MT_Block) {
    ವಿಕಲ್ಪ (T.Kind) {
    ಪಕ್ಷ Token::TK_BlockEnd:
      getNext();
      IsAtEnd = ಸತ್ಯ;
      CurrentEntry = nullptr;
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      setError("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    ಪಕ್ಷ Token::TK_Error:
      IsAtEnd = ಸತ್ಯ;
      CurrentEntry = nullptr;
    }
  } ಅನ್ಯಥಾ {
    ವಿಕಲ್ಪ (T.Kind) {
    ಪಕ್ಷ Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      ಪ್ರತ್ಯಾಗಮ increment();
    ಪಕ್ಷ Token::TK_FlowMappingEnd:
      getNext();
      [[fallthrough]];
    ಪಕ್ಷ Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = ಸತ್ಯ;
      CurrentEntry = nullptr;
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      setError( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      IsAtEnd = ಸತ್ಯ;
      CurrentEntry = nullptr;
    }
  }
}

ಶೂನ್ಯ Sequence_ಗಂಟು::increment() {
  ಯದಿ (failed()) {
    IsAtEnd = ಸತ್ಯ;
    CurrentEntry = nullptr;
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  ಯದಿ (SeqType == ST_Block) {
    ವಿಕಲ್ಪ (T.Kind) {
    ಪಕ್ಷ Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_ಗಂಟು();
      ಯದಿ (!CurrentEntry) { // An error occurred.
        IsAtEnd = ಸತ್ಯ;
        CurrentEntry = nullptr;
      }
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_BlockEnd:
      getNext();
      IsAtEnd = ಸತ್ಯ;
      CurrentEntry = nullptr;
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      setError( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    ಪಕ್ಷ Token::TK_Error:
      IsAtEnd = ಸತ್ಯ;
      CurrentEntry = nullptr;
    }
  } ಅನ್ಯಥಾ ಯದಿ (SeqType == ST_Indentless) {
    ವಿಕಲ್ಪ (T.Kind) {
    ಪಕ್ಷ Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_ಗಂಟು();
      ಯದಿ (!CurrentEntry) { // An error occurred.
        IsAtEnd = ಸತ್ಯ;
        CurrentEntry = nullptr;
      }
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
    ಪಕ್ಷ Token::TK_Error:
      IsAtEnd = ಸತ್ಯ;
      CurrentEntry = nullptr;
    }
  } ಅನ್ಯಥಾ ಯದಿ (SeqType == ST_Flow) {
    ವಿಕಲ್ಪ (T.Kind) {
    ಪಕ್ಷ Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      WasPreviousTokenFlowEntry = ಸತ್ಯ;
      ಪ್ರತ್ಯಾಗಮ increment();
    ಪಕ್ಷ Token::TK_FlowSequenceEnd:
      getNext();
      [[fallthrough]];
    ಪಕ್ಷ Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = ಸತ್ಯ;
      CurrentEntry = nullptr;
      ಭಙ್ಗ;
    ಪಕ್ಷ Token::TK_StreamEnd:
    ಪಕ್ಷ Token::TK_DocumentEnd:
    ಪಕ್ಷ Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = ಸತ್ಯ;
      CurrentEntry = nullptr;
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      ಯದಿ (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = ಸತ್ಯ;
        CurrentEntry = nullptr;
        ಭಙ್ಗ;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parse_Block_ಗಂಟು();
      ಯದಿ (!CurrentEntry) {
        IsAtEnd = ಸತ್ಯ;
      }
      WasPreviousTokenFlowEntry = ಅಸತ್ಯ;
      ಭಙ್ಗ;
    }
  }
}

Document::Document(Stream &S) : stream(S), ಬೇರು_2(nullptr) {
  // Tag maps starts with two default mappings.
  TagMap["!"] = "!";
  TagMap["!!"] = "tag:yaml.org,2002:";

  ಯದಿ (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peekNext();
  ಯದಿ (T.Kind == Token::TK_DocumentStart)
    getNext();
}

ತಾರ್ಕಿಕ Document::skip()  {
  ಯದಿ (stream.scanner->failed())
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (!ಬೇರು_2 && !get_ಬೇರು())
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಬೇರು_2->skip();
  Token &T = peekNext();
  ಯದಿ (T.Kind == Token::TK_StreamEnd)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    ಪ್ರತ್ಯಾಗಮ skip();
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

Token &Document::peekNext() {
  ಪ್ರತ್ಯಾಗಮ stream.scanner->peekNext();
}

Token Document::getNext() {
  ಪ್ರತ್ಯಾಗಮ stream.scanner->getNext();
}

ಶೂನ್ಯ Document::setError(ಧ್ರುವ Twine &Message, Token &Location) ಧ್ರುವ {
  stream.scanner->setError(Message, Location.Range.begin());
}

ತಾರ್ಕಿಕ Document::failed() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ stream.scanner->failed();
}

ಗಂಟು *Document::parse_Block_ಗಂಟು() {
  Token T = peekNext();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  ವಿಕಲ್ಪ (T.Kind) {
  ಪಕ್ಷ Token::TK_Alias:
    getNext();
    ಪ್ರತ್ಯಾಗಮ ನೂತನ (ಗಂಟು_Allocator) Alias_ಗಂಟು(stream.CurrentDoc, T.Range.substr(1));
  ಪಕ್ಷ Token::TK_Anchor:
    ಯದಿ (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      ಪ್ರತ್ಯಾಗಮ nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    ಗಚ್ಛ parse_property;
  ಪಕ್ಷ Token::TK_Tag:
    ಯದಿ (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      ಪ್ರತ್ಯಾಗಮ nullptr;
    }
    TagInfo = getNext(); // Consume TK_Tag.
    T = peekNext();
    ಗಚ್ಛ parse_property;
  ಸ್ವಭಾವ:
    ಭಙ್ಗ;
  }

  ವಿಕಲ್ಪ (T.Kind) {
  ಪಕ್ಷ Token::TK_BlockEntry:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    ಪ್ರತ್ಯಾಗಮ ನೂತನ (ಗಂಟು_Allocator) Sequence_ಗಂಟು( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , Sequence_ಗಂಟು::ST_Indentless);
  ಪಕ್ಷ Token::TK_BlockSequenceStart:
    getNext();
    ಪ್ರತ್ಯಾಗಮ ನೂತನ (ಗಂಟು_Allocator)
      Sequence_ಗಂಟು( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_ಗಂಟು::ST_Block);
  ಪಕ್ಷ Token::TK_BlockMappingStart:
    getNext();
    ಪ್ರತ್ಯಾಗಮ ನೂತನ (ಗಂಟು_Allocator)
      Mapping_ಗಂಟು( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_ಗಂಟು::MT_Block);
  ಪಕ್ಷ Token::TK_FlowSequenceStart:
    getNext();
    ಪ್ರತ್ಯಾಗಮ ನೂತನ (ಗಂಟು_Allocator)
      Sequence_ಗಂಟು( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_ಗಂಟು::ST_Flow);
  ಪಕ್ಷ Token::TK_FlowMappingStart:
    getNext();
    ಪ್ರತ್ಯಾಗಮ ನೂತನ (ಗಂಟು_Allocator)
      Mapping_ಗಂಟು( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_ಗಂಟು::MT_Flow);
  ಪಕ್ಷ Token::TK_Scalar:
    getNext();
    ಪ್ರತ್ಯಾಗಮ ನೂತನ (ಗಂಟು_Allocator)
      Scalar_ಗಂಟು( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  ಪಕ್ಷ Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.ಮೌಲ್ಯ.c_str(), T.ಮೌಲ್ಯ.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(ಗಂಟು_Allocator).drop_back();
    ಪ್ರತ್ಯಾಗಮ ನೂತನ (ಗಂಟು_Allocator)
        Block_Scalar_ಗಂಟು(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  ಪಕ್ಷ Token::TK_ಕೀಲಿ:
    // Don't eat the TK_Key, KeyValueNode expects it.
    ಪ್ರತ್ಯಾಗಮ ನೂತನ (ಗಂಟು_Allocator)
      Mapping_ಗಂಟು( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_ಗಂಟು::MT_Inline);
  ಪಕ್ಷ Token::TK_DocumentStart:
  ಪಕ್ಷ Token::TK_DocumentEnd:
  ಪಕ್ಷ Token::TK_StreamEnd:
  ಸ್ವಭಾವ:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    ಪ್ರತ್ಯಾಗಮ ನೂತನ (ಗಂಟು_Allocator) Null_ಗಂಟು(stream.CurrentDoc);
  ಪಕ್ಷ Token::TK_FlowMappingEnd:
  ಪಕ್ಷ Token::TK_FlowSequenceEnd:
  ಪಕ್ಷ Token::TK_FlowEntry: {
    ಯದಿ (ಬೇರು_2 && (isa<Mapping_ಗಂಟು>(ಬೇರು_2) || isa<Sequence_ಗಂಟು>(ಬೇರು_2)))
      ಪ್ರತ್ಯಾಗಮ ನೂತನ (ಗಂಟು_Allocator) Null_ಗಂಟು(stream.CurrentDoc);

    setError("Unexpected token", T);
    ಪ್ರತ್ಯಾಗಮ nullptr;
  }
  ಪಕ್ಷ Token::TK_Error:
    ಪ್ರತ್ಯಾಗಮ nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  ಪ್ರತ್ಯಾಗಮ nullptr;
}

ತಾರ್ಕಿಕ Document::parseDirectives() {
  ತಾರ್ಕಿಕ isDirective = ಅಸತ್ಯ;
  ಯಾವತ್ (ಸತ್ಯ) {
    Token T = peekNext();
    ಯದಿ (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = ಸತ್ಯ;
    } ಅನ್ಯಥಾ ಯದಿ (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = ಸತ್ಯ;
    } ಅನ್ಯಥಾ {
      ಭಙ್ಗ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ isDirective;
}

ಶೂನ್ಯ Document::parseYAMLDirective() {
  getNext(); // Eat %YAML <version>
}

ಶೂನ್ಯ Document::parseTAGDirective() {
  Token Tag = getNext(); // %TAG <handle> <prefix>
  StringRef T = Tag.Range;
  // Strip %TAG
  T = T.substr(T.find_first_of(" \t")).ltrim(" \t");
  std::size_t HandleEnd = T.find_first_of(" \t");
  StringRef TagHandle = T.substr(0, HandleEnd);
  StringRef TagPrefix = T.substr(HandleEnd).ltrim(" \t");
  TagMap[TagHandle] = TagPrefix;
}

ತಾರ್ಕಿಕ Document::expectToken(ಪೂರ್ಣಾಙ್ಕ TK) {
  Token T = getNext();
  ಯದಿ (T.Kind != TK) {
    setError("Unexpected token", T);
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}
