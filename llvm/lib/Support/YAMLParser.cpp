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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ yaml;

ಎಣಿಕೆ UnicodeEncodingForm {
  UEF_UTF32_LE, ///< UTF-32 Little Endian
  UEF_UTF32_BE, ///< UTF-32 Big Endian
  UEF_UTF16_LE, ///< UTF-16 Little Endian
  UEF_UTF16_BE, ///< UTF-16 Big Endian
  UEF_UTF8,     ///< UTF-8 or ascii.
  UEF_Unknown   ///< Not a valid Unicode encoding.
};

/// EncodingInfo - Holds the encoding type and length of the byte order mark if
///                it exists. Length is in {0, 2, 3, 4}.
ಬಳಸಿ EncodingInfo = std::pair<UnicodeEncodingForm, ಚಿಹ್ನೆರಹಿತ>;

/// getUnicodeEncoding - Reads up to the first 4 bytes to determine the Unicode
///                      encoding form of \a Input.
///
/// @param Input A string of length 0 or more.
/// @returns An EncodingInfo indicating the Unicode encoding form of the input
///          and how long the byte order mark is if one exists.
ಸ್ಥಿರ EncodingInfo getUnicodeEncoding(StringRef ಒಳಹರಿವು) {
  ಆದರೆ (ಒಳಹರಿವು.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ {UEF_Unknown, 0};

  ಆಯ್ಕೆ (uint8_t(ಒಳಹರಿವು[0])) {
  ಸ್ಥಿತಿ 0x00:
    ಆದರೆ (ಒಳಹರಿವು.ಗಾತ್ರ_3() >= 4) {
      ಆದರೆ (  ಒಳಹರಿವು[1] == 0
         && uint8_t(ಒಳಹರಿವು[2]) == 0xFE
         && uint8_t(ಒಳಹರಿವು[3]) == 0xFF)
        ಹಿಂತಿರುಗಿ {UEF_UTF32_BE, 4};
      ಆದರೆ (ಒಳಹರಿವು[1] == 0 && ಒಳಹರಿವು[2] == 0 && ಒಳಹರಿವು[3] != 0)
        ಹಿಂತಿರುಗಿ {UEF_UTF32_BE, 0};
    }

    ಆದರೆ (ಒಳಹರಿವು.ಗಾತ್ರ_3() >= 2 && ಒಳಹರಿವು[1] != 0)
      ಹಿಂತಿರುಗಿ {UEF_UTF16_BE, 0};
    ಹಿಂತಿರುಗಿ {UEF_Unknown, 0};
  ಸ್ಥಿತಿ 0xFF:
    ಆದರೆ (  ಒಳಹರಿವು.ಗಾತ್ರ_3() >= 4
       && uint8_t(ಒಳಹರಿವು[1]) == 0xFE
       && ಒಳಹರಿವು[2] == 0
       && ಒಳಹರಿವು[3] == 0)
      ಹಿಂತಿರುಗಿ {UEF_UTF32_LE, 4};

    ಆದರೆ (ಒಳಹರಿವು.ಗಾತ್ರ_3() >= 2 && uint8_t(ಒಳಹರಿವು[1]) == 0xFE)
      ಹಿಂತಿರುಗಿ {UEF_UTF16_LE, 2};
    ಹಿಂತಿರುಗಿ {UEF_Unknown, 0};
  ಸ್ಥಿತಿ 0xFE:
    ಆದರೆ (ಒಳಹರಿವು.ಗಾತ್ರ_3() >= 2 && uint8_t(ಒಳಹರಿವು[1]) == 0xFF)
      ಹಿಂತಿರುಗಿ {UEF_UTF16_BE, 2};
    ಹಿಂತಿರುಗಿ {UEF_Unknown, 0};
  ಸ್ಥಿತಿ 0xEF:
    ಆದರೆ (  ಒಳಹರಿವು.ಗಾತ್ರ_3() >= 3
       && uint8_t(ಒಳಹರಿವು[1]) == 0xBB
       && uint8_t(ಒಳಹರಿವು[2]) == 0xBF)
      ಹಿಂತಿರುಗಿ {UEF_UTF8, 3};
    ಹಿಂತಿರುಗಿ {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  ಆದರೆ (ಒಳಹರಿವು.ಗಾತ್ರ_3() >= 4 && ಒಳಹರಿವು[1] == 0 && ಒಳಹರಿವು[2] == 0 && ಒಳಹರಿವು[3] == 0)
    ಹಿಂತಿರುಗಿ {UEF_UTF32_LE, 0};

  ಆದರೆ (ಒಳಹರಿವು.ಗಾತ್ರ_3() >= 2 && ಒಳಹರಿವು[1] == 0)
    ಹಿಂತಿರುಗಿ {UEF_UTF16_LE, 0};

  ಹಿಂತಿರುಗಿ {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
ಶೂನ್ಯ ಗಂಟು::anchor() {}
ಶೂನ್ಯ Null_ಗಂಟು::anchor() {}
ಶೂನ್ಯ Scalar_ಗಂಟು::anchor() {}
ಶೂನ್ಯ ಖಂಡ_Scalar_ಗಂಟು::anchor() {}
ಶೂನ್ಯ ಕೀಲಿ_ಮೌಲ್ಯ_ಗಂಟು::anchor() {}
ಶೂನ್ಯ Mapping_ಗಂಟು::anchor() {}
ಶೂನ್ಯ Sequence_ಗಂಟು::anchor() {}
ಶೂನ್ಯ Alias_ಗಂಟು::anchor() {}

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ yaml {

/// Token - A single YAML token.
ರಚನೆ Token {
  ಎಣಿಕೆ Token_ಬಗೆ {
    TK_Error, // Uninitialized token.
    TK_ಹರಿವು_ಆರಂಭ,
    TK_ಹರಿವು_ಕೊನೆ,
    TK_ಆವೃತ್ತಿ_Directive,
    TK_ಟ್ಯಾಗ್_Directive,
    TK_Document_ಆರಂಭ,
    TK_Document_ಕೊನೆ,
    TK_ಖಂಡ_ನಮೂದು,
    TK_ಖಂಡ_ಕೊನೆ,
    TK_ಖಂಡ_Sequence_ಆರಂಭ,
    TK_ಖಂಡ_Mapping_ಆರಂಭ,
    TK_Flow_ನಮೂದು,
    TK_Flow_Sequence_ಆರಂಭ,
    TK_Flow_Sequence_ಕೊನೆ,
    TK_Flow_Mapping_ಆರಂಭ,
    TK_Flow_Mapping_ಕೊನೆ,
    TK_ಕೀಲಿ,
    TK_ಮೌಲ್ಯ,
    TK_Scalar,
    TK_ಖಂಡ_Scalar,
    TK_Alias,
    TK_Anchor,
    TK_ಟ್ಯಾಗ್
  } ಬಗೆ_2 = TK_Error;

  /// A string of length 0 or more whose begin() points to the logical location
  /// of the token in the input.
  StringRef Range;

  /// The value of a block scalar node.
  std::string ಮೌಲ್ಯ;

  Token() = ಪೂರ್ವನಿಯೋಜಿತ;
};

} // end namespace yaml
} // end namespace llvm

ಬಳಸಿ Token_ಸರತಿ_T = BumpPtrList<Token>;

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
ರಚನೆ Simple_ಕೀಲಿ {
  Token_ಸರತಿ_T::iterator Tok;
  ಚಿಹ್ನೆರಹಿತ ಸ್ತಂಭ = 0;
  ಚಿಹ್ನೆರಹಿತ ಸಾಲು = 0;
  ಚಿಹ್ನೆರಹಿತ FlowLevel = 0;
  ತಾರ್ಕಿಕ IsRequired = ಸುಳ್ಳು;

  ತಾರ್ಕಿಕ ಕ್ರಿಯಾಕಾರಕ ==(ಸ್ಥಿರಾಂಕ Simple_ಕೀಲಿ &Other) {
    ಹಿಂತಿರುಗಿ Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
ಬಳಸಿ UTF8Decoded = std::pair<uint32_t, ಚಿಹ್ನೆರಹಿತ>;

ಸ್ಥಿರ UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator ಕೊನೆ_2 = Range.ಕೊನೆ();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  ಆದರೆ (Position < ಕೊನೆ_2 && (*Position & 0x80) == 0) {
    ಹಿಂತಿರುಗಿ {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  ಆದರೆ (Position + 1 < ಕೊನೆ_2 && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    ಆದರೆ (codepoint >= 0x80)
      ಹಿಂತಿರುಗಿ {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  ಆದರೆ (Position + 2 < ಕೊನೆ_2 && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    ಆದರೆ (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      ಹಿಂತಿರುಗಿ {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  ಆದರೆ (Position + 3 < ಕೊನೆ_2 && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    ಆದರೆ (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      ಹಿಂತಿರುಗಿ {codepoint, 4};
  }
  ಹಿಂತಿರುಗಿ {0, 0};
}

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ yaml {

/// Scans YAML tokens from a MemoryBuffer.
ವರ್ಗ Scanner {
ಸಾರ್ವಜನಿಕ:
  Scanner(StringRef ಒಳಹರಿವು, ಮೂಲ_Mgr &SM, ತಾರ್ಕಿಕ ShowColors = ನಿಜ,
          std::error_ಸಂಕೇತ *EC = ಶೂನ್ಯಸೂಚಕ);
  Scanner(ಸ್ಮರಣೆ_ಬಫರ್_Ref ಬಫರ್, ಮೂಲ_Mgr &SM_, ತಾರ್ಕಿಕ ShowColors = ನಿಜ,
          std::error_ಸಂಕೇತ *EC = ಶೂನ್ಯಸೂಚಕ);

  /// Parse the next token and return it without popping it.
  Token &peek_ಮುಂದಿನ();

  /// Parse the next token and pop it from the queue.
  Token get_ಮುಂದಿನ();

  ಶೂನ್ಯ printError(SMLoc Loc, ಮೂಲ_Mgr::Diag_ಬಗೆ ಬಗೆ_2, ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_3,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.Print_ಸಂದೇಶ(Loc, ಬಗೆ_2, ಸಂದೇಶ_3, Ranges, /* FixIts= */ {}, ShowColors);
  }

  ಶೂನ್ಯ ಗಣ_Error(ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_3, StringRef::iterator Position) {
    ಆದರೆ (Position >= ಕೊನೆ_2)
      Position = ಕೊನೆ_2 - 1;

    // propagate the error if possible
    ಆದರೆ (EC)
      *EC = make_error_ಸಂಕೇತ(std::errc::invalid_ವಾದ);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    ಆದರೆ (!Failed)
      printError(SMLoc::get_From_ಸೂಚಕ(Position), ಮೂಲ_Mgr::DK_Error, ಸಂದೇಶ_3);
    Failed = ನಿಜ;
  }

  /// Returns true if an error occurred while parsing.
  ತಾರ್ಕಿಕ failed() {
    ಹಿಂತಿರುಗಿ Failed;
  }

ಖಾಸಗಿ:
  ಶೂನ್ಯ init(ಸ್ಮರಣೆ_ಬಫರ್_Ref ಬಫರ್);

  StringRef current_ಒಳಹರಿವು() {
    ಹಿಂತಿರುಗಿ StringRef(Current, ಕೊನೆ_2 - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    ಹಿಂತಿರುಗಿ ::decodeUTF8(StringRef(Position, ಕೊನೆ_2 - Position));
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

  ಬಳಸಿ SkipWhileFunc = StringRef::iterator (Scanner::*)(StringRef::iterator);

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
  ಶೂನ್ಯ ಪರಿಶೀಲನೆ_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  ತಾರ್ಕಿಕ consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  ಶೂನ್ಯ skip(uint32_t ದೂರ_2);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  ತಾರ್ಕಿಕ isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  ತಾರ್ಕಿಕ is_Plain_ಸುರಕ್ಷಿತ_Non_Blank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  ತಾರ್ಕಿಕ is_ಸಾಲು_ಖಾಲಿಗಳು(StringRef ಸಾಲು);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  ತಾರ್ಕಿಕ consume_ಸಾಲು_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  ಶೂನ್ಯ save_Simple_ಕೀಲಿ_Candidate( Token_ಸರತಿ_T::iterator Tok
                             , ಚಿಹ್ನೆರಹಿತ At_ಸ್ತಂಭ
                             , ತಾರ್ಕಿಕ IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  ಶೂನ್ಯ remove_Stale_Simple_ಕೀಲಿ_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  ಶೂನ್ಯ remove_Simple_ಕೀಲಿ_Candidates_On_Flow_Level(ಚಿಹ್ನೆರಹಿತ Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  ತಾರ್ಕಿಕ unrollIndent(ಪೂರ್ಣಾಂಕ ಗಮ್ಯ_ಸ್ತಂಭ);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  ತಾರ್ಕಿಕ rollIndent( ಪೂರ್ಣಾಂಕ ಗಮ್ಯ_ಸ್ತಂಭ
                 , Token::Token_ಬಗೆ ಬಗೆ_2
                 , Token_ಸರತಿ_T::iterator ಸೇರಿಸು_ಬಿಂದು);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  ಶೂನ್ಯ skipComment();

  /// Skip whitespace and comments until the start of the next token.
  ಶೂನ್ಯ ಪರಿಶೀಲನೆ_ಗಮ್ಯ_ಮುಂದಿನ_Token();

  /// Must be the first token generated.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_ಹರಿವು_ಆರಂಭ();

  /// Generate tokens needed to close out the stream.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_ಹರಿವು_ಕೊನೆ();

  /// Scan a %BLAH directive.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_Directive();

  /// Scan a ... or ---.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_Document_Indicator(ತಾರ್ಕಿಕ Is_ಆರಂಭ);

  /// Scan a [ or { and generate the proper flow collection start token.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_Flow_Collection_ಆರಂಭ(ತಾರ್ಕಿಕ IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_Flow_Collection_ಕೊನೆ(ತಾರ್ಕಿಕ IsSequence);

  /// Scan the , that separates entries in a flow collection.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_Flow_ನಮೂದು();

  /// Scan the - that starts block sequence entries.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_ಖಂಡ_ನಮೂದು();

  /// Scan an explicit ? indicating a key.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_ಕೀಲಿ();

  /// Scan an explicit : indicating a value.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_ಮೌಲ್ಯ();

  /// Scan a quoted scalar.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_Flow_Scalar(ತಾರ್ಕಿಕ IsDoubleQuoted);

  /// Scan an unquoted scalar.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_Plain_Scalar();

  /// Scan an Alias or Anchor starting with * or &.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_Alias_Or_Anchor(ತಾರ್ಕಿಕ IsAlias);

  /// Scan a block scalar starting with | or >.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_ಖಂಡ_Scalar(ತಾರ್ಕಿಕ IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_ಖಂಡ_Scalar_Indicators(ಅಕ್ಷರ &StyleIndicator, ಅಕ್ಷರ &ChompingIndicator,
                                 ಚಿಹ್ನೆರಹಿತ &IndentIndicator, ತಾರ್ಕಿಕ &Is_ಮುಗಿದ);

  /// Scan a style indicator in a block scalar header.
  ಅಕ್ಷರ ಪರಿಶೀಲನೆ_ಖಂಡ_Style_Indicator();

  /// Scan a chomping indicator in a block scalar header.
  ಅಕ್ಷರ ಪರಿಶೀಲನೆ_ಖಂಡ_Chomping_Indicator();

  /// Scan an indentation indicator in a block scalar header.
  ಚಿಹ್ನೆರಹಿತ ಪರಿಶೀಲನೆ_ಖಂಡ_Indentation_Indicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_ಖಂಡ_Scalar_ಶೀರ್ಷಿಕೆ(ಅಕ್ಷರ &ChompingIndicator, ಚಿಹ್ನೆರಹಿತ &IndentIndicator,
                             ತಾರ್ಕಿಕ &Is_ಮುಗಿದ);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  ತಾರ್ಕಿಕ ಕಂಡುಹಿಡಿ_ಖಂಡ_Scalar_Indent(ಚಿಹ್ನೆರಹಿತ &ಖಂಡ_Indent, ಚಿಹ್ನೆರಹಿತ ಖಂಡ_Exit_Indent,
                             ಚಿಹ್ನೆರಹಿತ &ಸಾಲು_Breaks, ತಾರ್ಕಿಕ &Is_ಮುಗಿದ);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_ಖಂಡ_Scalar_Indent(ಚಿಹ್ನೆರಹಿತ ಖಂಡ_Indent, ಚಿಹ್ನೆರಹಿತ ಖಂಡ_Exit_Indent,
                             ತಾರ್ಕಿಕ &Is_ಮುಗಿದ);

  /// Scan a tag of the form !stuff.
  ತಾರ್ಕಿಕ ಪರಿಶೀಲನೆ_ಟ್ಯಾಗ್();

  /// Dispatch to the next scanning function based on \a *Cur.
  ತಾರ್ಕಿಕ fetch_ಇನ್ನಷ್ಟು_ಟೋಕನುಗಳು();

  /// The SourceMgr used for diagnostics and buffer management.
  ಮೂಲ_Mgr &SM;

  /// The original input.
  ಸ್ಮರಣೆ_ಬಫರ್_Ref ಒಳಹರಿವು_ಬಫರ್;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator ಕೊನೆ_2;

  /// Current YAML indentation level in spaces.
  ಪೂರ್ಣಾಂಕ Indent;

  /// Current column number in Unicode code points.
  ಚಿಹ್ನೆರಹಿತ ಸ್ತಂಭ;

  /// Current line number.
  ಚಿಹ್ನೆರಹಿತ ಸಾಲು;

  /// How deep we are in flow style containers. 0 Means at block level.
  ಚಿಹ್ನೆರಹಿತ FlowLevel;

  /// Are we at the start of the stream?
  ತಾರ್ಕಿಕ Is_ಆರಂಭ_ದ_ಹರಿವು;

  /// Can the next token be the start of a simple key?
  ತಾರ್ಕಿಕ Is_Simple_ಕೀಲಿ_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  ತಾರ್ಕಿಕ Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow;

  /// True if an error has occurred.
  ತಾರ್ಕಿಕ Failed;

  /// Should colors be used when printing out the diagnostic messages?
  ತಾರ್ಕಿಕ ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  Token_ಸರತಿ_T Token_ಸರತಿ;

  /// Indentation levels.
  Small_ಸದಿಶ<ಪೂರ್ಣಾಂಕ, 4> Indents;

  /// Potential simple keys.
  Small_ಸದಿಶ<Simple_ಕೀಲಿ, 4> Simple_ಕೀಲಿಗಳು;

  std::error_ಸಂಕೇತ *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
ಸ್ಥಿರ ಶೂನ್ಯ encodeUTF8( uint32_t Unicode_Scalar_ಮೌಲ್ಯ
                      , Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ) {
  ಆದರೆ (Unicode_Scalar_ಮೌಲ್ಯ <= 0x7F) {
    ಫಲಿತಾಂಶ.ತಳ್ಳು_ಹಿಂಭಾಗ(Unicode_Scalar_ಮೌಲ್ಯ & 0x7F);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Unicode_Scalar_ಮೌಲ್ಯ <= 0x7FF) {
    uint8_t ಮೊದಲ_Byte = 0xC0 | ((Unicode_Scalar_ಮೌಲ್ಯ & 0x7C0) >> 6);
    uint8_t ಎರಡನೇ_Byte = 0x80 | (Unicode_Scalar_ಮೌಲ್ಯ & 0x3F);
    ಫಲಿತಾಂಶ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮೊದಲ_Byte);
    ಫಲಿತಾಂಶ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಎರಡನೇ_Byte);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Unicode_Scalar_ಮೌಲ್ಯ <= 0xFFFF) {
    uint8_t ಮೊದಲ_Byte = 0xE0 | ((Unicode_Scalar_ಮೌಲ್ಯ & 0xF000) >> 12);
    uint8_t ಎರಡನೇ_Byte = 0x80 | ((Unicode_Scalar_ಮೌಲ್ಯ & 0xFC0) >> 6);
    uint8_t ಮೂರನೇ_Byte = 0x80 | (Unicode_Scalar_ಮೌಲ್ಯ & 0x3F);
    ಫಲಿತಾಂಶ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮೊದಲ_Byte);
    ಫಲಿತಾಂಶ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಎರಡನೇ_Byte);
    ಫಲಿತಾಂಶ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮೂರನೇ_Byte);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Unicode_Scalar_ಮೌಲ್ಯ <= 0x10FFFF) {
    uint8_t ಮೊದಲ_Byte = 0xF0 | ((Unicode_Scalar_ಮೌಲ್ಯ & 0x1F0000) >> 18);
    uint8_t ಎರಡನೇ_Byte = 0x80 | ((Unicode_Scalar_ಮೌಲ್ಯ & 0x3F000) >> 12);
    uint8_t ಮೂರನೇ_Byte = 0x80 | ((Unicode_Scalar_ಮೌಲ್ಯ & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_ಮೌಲ್ಯ & 0x3F);
    ಫಲಿತಾಂಶ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮೊದಲ_Byte);
    ಫಲಿತಾಂಶ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಎರಡನೇ_Byte);
    ಫಲಿತಾಂಶ.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮೂರನೇ_Byte);
    ಫಲಿತಾಂಶ.ತಳ್ಳು_ಹಿಂಭಾಗ(FourthByte);
  }
}

ತಾರ್ಕಿಕ yaml::dump_ಟೋಕನುಗಳು(StringRef ಒಳಹರಿವು, raw_ostream &OS) {
  ಮೂಲ_Mgr SM;
  Scanner scanner(ಒಳಹರಿವು, SM);
  ಆಗ (ನಿಜ) {
    Token T = scanner.get_ಮುಂದಿನ();
    ಆಯ್ಕೆ (T.ಬಗೆ_2) {
    ಸ್ಥಿತಿ Token::TK_ಹರಿವು_ಆರಂಭ:
      OS << "Stream-Start: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_ಹರಿವು_ಕೊನೆ:
      OS << "Stream-End: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_ಆವೃತ್ತಿ_Directive:
      OS << "Version-Directive: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_ಟ್ಯಾಗ್_Directive:
      OS << "Tag-Directive: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_Document_ಆರಂಭ:
      OS << "Document-Start: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_Document_ಕೊನೆ:
      OS << "Document-End: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_ಖಂಡ_ನಮೂದು:
      OS << "Block-Entry: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_ಖಂಡ_ಕೊನೆ:
      OS << "Block-End: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_ಖಂಡ_Sequence_ಆರಂಭ:
      OS << "Block-Sequence-Start: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_ಖಂಡ_Mapping_ಆರಂಭ:
      OS << "Block-Mapping-Start: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_Flow_ನಮೂದು:
      OS << "Flow-Entry: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_Flow_Sequence_ಆರಂಭ:
      OS << "Flow-Sequence-Start: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_Flow_Sequence_ಕೊನೆ:
      OS << "Flow-Sequence-End: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_Flow_Mapping_ಆರಂಭ:
      OS << "Flow-Mapping-Start: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_Flow_Mapping_ಕೊನೆ:
      OS << "Flow-Mapping-End: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_ಕೀಲಿ:
      OS << "Key: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_ಮೌಲ್ಯ:
      OS << "Value: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_Scalar:
      OS << "Scalar: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_ಖಂಡ_Scalar:
      OS << "Block Scalar: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_Alias:
      OS << "Alias: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_Anchor:
      OS << "Anchor: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_ಟ್ಯಾಗ್:
      OS << "Tag: ";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_Error:
      ನಿಲ್ಲಿಸು;
    }
    OS << T.Range << "\n";
    ಆದರೆ (T.ಬಗೆ_2 == Token::TK_ಹರಿವು_ಕೊನೆ)
      ನಿಲ್ಲಿಸು;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (T.ಬಗೆ_2 == Token::TK_Error)
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ yaml::ಪರಿಶೀಲನೆ_ಟೋಕನುಗಳು(StringRef ಒಳಹರಿವು) {
  ಮೂಲ_Mgr SM;
  Scanner scanner(ಒಳಹರಿವು, SM);
  ಆಗ (ನಿಜ) {
    Token T = scanner.get_ಮುಂದಿನ();
    ಆದರೆ (T.ಬಗೆ_2 == Token::TK_ಹರಿವು_ಕೊನೆ)
      ನಿಲ್ಲಿಸು;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (T.ಬಗೆ_2 == Token::TK_Error)
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

std::string yaml::escape(StringRef ಒಳಹರಿವು, ತಾರ್ಕಿಕ EscapePrintable) {
  std::string Escaped_ಒಳಹರಿವು;
  ಗಾಗಿ (StringRef::iterator i = ಒಳಹರಿವು.begin(), e = ಒಳಹರಿವು.ಕೊನೆ(); i != e; ++i) {
    ಆದರೆ (*i == '\\')
      Escaped_ಒಳಹರಿವು += "\\\\";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*i == '"')
      Escaped_ಒಳಹರಿವು += "\\\"";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*i == 0)
      Escaped_ಒಳಹರಿವು += "\\0";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*i == 0x07)
      Escaped_ಒಳಹರಿವು += "\\a";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*i == 0x08)
      Escaped_ಒಳಹರಿವು += "\\b";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*i == 0x09)
      Escaped_ಒಳಹರಿವು += "\\t";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*i == 0x0A)
      Escaped_ಒಳಹರಿವು += "\\n";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*i == 0x0B)
      Escaped_ಒಳಹರಿವು += "\\v";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*i == 0x0C)
      Escaped_ಒಳಹರಿವು += "\\f";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*i == 0x0D)
      Escaped_ಒಳಹರಿವು += "\\r";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*i == 0x1B)
      Escaped_ಒಳಹರಿವು += "\\e";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_ಒಳಹರಿವು += "\\x" + std::string(2 - HexStr.ಗಾತ್ರ_3(), '0') + HexStr;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_ಮೌಲ್ಯ
        = decodeUTF8(StringRef(i, ಒಳಹರಿವು.ಕೊನೆ() - i));
      ಆದರೆ (Unicode_Scalar_ಮೌಲ್ಯ.ಎರಡನೇ_2 == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_ಒಳಹರಿವು, Val);
        // FIXME: Error reporting.
        ಹಿಂತಿರುಗಿ Escaped_ಒಳಹರಿವು;
      }
      ಆದರೆ (Unicode_Scalar_ಮೌಲ್ಯ.ಮೊದಲ_2 == 0x85)
        Escaped_ಒಳಹರಿವು += "\\N";
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Unicode_Scalar_ಮೌಲ್ಯ.ಮೊದಲ_2 == 0xA0)
        Escaped_ಒಳಹರಿವು += "\\_";
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Unicode_Scalar_ಮೌಲ್ಯ.ಮೊದಲ_2 == 0x2028)
        Escaped_ಒಳಹರಿವು += "\\L";
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Unicode_Scalar_ಮೌಲ್ಯ.ಮೊದಲ_2 == 0x2029)
        Escaped_ಒಳಹರಿವು += "\\P";
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_ಮೌಲ್ಯ.ಮೊದಲ_2))
        Escaped_ಒಳಹರಿವು += StringRef(i, Unicode_Scalar_ಮೌಲ್ಯ.ಎರಡನೇ_2);
      ಇಲ್ಲದಿದ್ದರೆ {
        std::string HexStr = utohexstr(Unicode_Scalar_ಮೌಲ್ಯ.ಮೊದಲ_2);
        ಆದರೆ (HexStr.ಗಾತ್ರ_3() <= 2)
          Escaped_ಒಳಹರಿವು += "\\x" + std::string(2 - HexStr.ಗಾತ್ರ_3(), '0') + HexStr;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (HexStr.ಗಾತ್ರ_3() <= 4)
          Escaped_ಒಳಹರಿವು += "\\u" + std::string(4 - HexStr.ಗಾತ್ರ_3(), '0') + HexStr;
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (HexStr.ಗಾತ್ರ_3() <= 8)
          Escaped_ಒಳಹರಿವು += "\\U" + std::string(8 - HexStr.ಗಾತ್ರ_3(), '0') + HexStr;
      }
      i += Unicode_Scalar_ಮೌಲ್ಯ.ಎರಡನೇ_2 - 1;
    } ಇಲ್ಲದಿದ್ದರೆ {
      Escaped_ಒಳಹರಿವು.ತಳ್ಳು_ಹಿಂಭಾಗ(*i);
    }
  }
  ಹಿಂತಿರುಗಿ Escaped_ಒಳಹರಿವು;
}

std::optional<ತಾರ್ಕಿಕ> yaml::parseBool(StringRef S) {
  ಆಯ್ಕೆ (S.ಗಾತ್ರ_3()) {
  ಸ್ಥಿತಿ 1:
    ಆಯ್ಕೆ (S.front()) {
    ಸ್ಥಿತಿ 'y':
    ಸ್ಥಿತಿ 'Y':
      ಹಿಂತಿರುಗಿ ನಿಜ;
    ಸ್ಥಿತಿ 'n':
    ಸ್ಥಿತಿ 'N':
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ std::nullopt;
    }
  ಸ್ಥಿತಿ 2:
    ಆಯ್ಕೆ (S.front()) {
    ಸ್ಥಿತಿ 'O':
      ಆದರೆ (S[1] == 'N') // ON
        ಹಿಂತಿರುಗಿ ನಿಜ;
      [[fallthrough]];
    ಸ್ಥಿತಿ 'o':
      ಆದರೆ (S[1] == 'n') //[Oo]n
        ಹಿಂತಿರುಗಿ ನಿಜ;
      ಹಿಂತಿರುಗಿ std::nullopt;
    ಸ್ಥಿತಿ 'N':
      ಆದರೆ (S[1] == 'O') // NO
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      [[fallthrough]];
    ಸ್ಥಿತಿ 'n':
      ಆದರೆ (S[1] == 'o') //[Nn]o
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      ಹಿಂತಿರುಗಿ std::nullopt;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ std::nullopt;
    }
  ಸ್ಥಿತಿ 3:
    ಆಯ್ಕೆ (S.front()) {
    ಸ್ಥಿತಿ 'O':
      ಆದರೆ (S.drop_front() == "FF") // OFF
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      [[fallthrough]];
    ಸ್ಥಿತಿ 'o':
      ಆದರೆ (S.drop_front() == "ff") //[Oo]ff
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      ಹಿಂತಿರುಗಿ std::nullopt;
    ಸ್ಥಿತಿ 'Y':
      ಆದರೆ (S.drop_front() == "ES") // YES
        ಹಿಂತಿರುಗಿ ನಿಜ;
      [[fallthrough]];
    ಸ್ಥಿತಿ 'y':
      ಆದರೆ (S.drop_front() == "es") //[Yy]es
        ಹಿಂತಿರುಗಿ ನಿಜ;
      ಹಿಂತಿರುಗಿ std::nullopt;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ std::nullopt;
    }
  ಸ್ಥಿತಿ 4:
    ಆಯ್ಕೆ (S.front()) {
    ಸ್ಥಿತಿ 'T':
      ಆದರೆ (S.drop_front() == "RUE") // TRUE
        ಹಿಂತಿರುಗಿ ನಿಜ;
      [[fallthrough]];
    ಸ್ಥಿತಿ 't':
      ಆದರೆ (S.drop_front() == "rue") //[Tt]rue
        ಹಿಂತಿರುಗಿ ನಿಜ;
      ಹಿಂತಿರುಗಿ std::nullopt;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ std::nullopt;
    }
  ಸ್ಥಿತಿ 5:
    ಆಯ್ಕೆ (S.front()) {
    ಸ್ಥಿತಿ 'F':
      ಆದರೆ (S.drop_front() == "ALSE") // FALSE
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      [[fallthrough]];
    ಸ್ಥಿತಿ 'f':
      ಆದರೆ (S.drop_front() == "alse") //[Ff]alse
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      ಹಿಂತಿರುಗಿ std::nullopt;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ std::nullopt;
    }
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಹಿಂತಿರುಗಿ std::nullopt;
  }
}

Scanner::Scanner(StringRef ಒಳಹರಿವು, ಮೂಲ_Mgr &sm, ತಾರ್ಕಿಕ ShowColors,
                 std::error_ಸಂಕೇತ *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(ಸ್ಮರಣೆ_ಬಫರ್_Ref(ಒಳಹರಿವು, "YAML"));
}

Scanner::Scanner(ಸ್ಮರಣೆ_ಬಫರ್_Ref ಬಫರ್, ಮೂಲ_Mgr &SM_, ತಾರ್ಕಿಕ ShowColors,
                 std::error_ಸಂಕೇತ *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(ಬಫರ್);
}

ಶೂನ್ಯ Scanner::init(ಸ್ಮರಣೆ_ಬಫರ್_Ref ಬಫರ್) {
  ಒಳಹರಿವು_ಬಫರ್ = ಬಫರ್;
  Current = ಒಳಹರಿವು_ಬಫರ್.get_ಬಫರ್_ಆರಂಭ();
  ಕೊನೆ_2 = ಒಳಹರಿವು_ಬಫರ್.get_ಬಫರ್_ಕೊನೆ();
  Indent = -1;
  ಸ್ತಂಭ = 0;
  ಸಾಲು = 0;
  FlowLevel = 0;
  Is_ಆರಂಭ_ದ_ಹರಿವು = ನಿಜ;
  Is_Simple_ಕೀಲಿ_Allowed = ನಿಜ;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow = ಸುಳ್ಳು;
  Failed = ಸುಳ್ಳು;
  std::unique_ptr<ಸ್ಮರಣೆ_ಬಫರ್> ಒಳಹರಿವು_ಬಫರ್_Owner =
      ಸ್ಮರಣೆ_ಬಫರ್::get_Mem_ಬಫರ್(ಬಫರ್, /*RequiresNullTerminator=*/ಸುಳ್ಳು);
  SM.ಕೂಡಿಸು_New_ಮೂಲ_ಬಫರ್(std::move(ಒಳಹರಿವು_ಬಫರ್_Owner), SMLoc());
}

Token &Scanner::peek_ಮುಂದಿನ() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  ತಾರ್ಕಿಕ Need_ಇನ್ನಷ್ಟು = ಸುಳ್ಳು;
  ಆಗ (ನಿಜ) {
    ಆದರೆ (Token_ಸರತಿ.ಖಾಲಿಗಳು() || Need_ಇನ್ನಷ್ಟು) {
      ಆದರೆ (!fetch_ಇನ್ನಷ್ಟು_ಟೋಕನುಗಳು()) {
        Token_ಸರತಿ.clear();
        Simple_ಕೀಲಿಗಳು.clear();
        Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(Token());
        ಹಿಂತಿರುಗಿ Token_ಸರತಿ.front();
      }
    }
    ಪ್ರತಿಜ್ಞಾ(!Token_ಸರತಿ.ಖಾಲಿಗಳು() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_ಕೀಲಿ_Candidates();
    Simple_ಕೀಲಿ SK;
    SK.Tok = Token_ಸರತಿ.begin();
    ಆದರೆ (!is_contained(Simple_ಕೀಲಿಗಳು, SK))
      ನಿಲ್ಲಿಸು;
    ಇಲ್ಲದಿದ್ದರೆ
      Need_ಇನ್ನಷ್ಟು = ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ Token_ಸರತಿ.front();
}

Token Scanner::get_ಮುಂದಿನ() {
  Token Ret = peek_ಮುಂದಿನ();
  // TokenQueue can be empty if there was an error getting the next token.
  ಆದರೆ (!Token_ಸರತಿ.ಖಾಲಿಗಳು())
    Token_ಸರತಿ.ತೆಗೆ_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  ಆದರೆ (Token_ಸರತಿ.ಖಾಲಿಗಳು())
    Token_ಸರತಿ.resetAlloc();

  ಹಿಂತಿರುಗಿ Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  ಆದರೆ (Position == ಕೊನೆ_2)
    ಹಿಂತಿರುಗಿ Position;
  // Check 7 bit c-printable - b-char.
  ಆದರೆ (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    ಹಿಂತಿರುಗಿ Position + 1;

  // Check for valid UTF-8.
  ಆದರೆ (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    ಆದರೆ (   u8d.ಎರಡನೇ_2 != 0
        && u8d.ಮೊದಲ_2 != 0xFEFF
        && ( u8d.ಮೊದಲ_2 == 0x85
          || ( u8d.ಮೊದಲ_2 >= 0xA0
            && u8d.ಮೊದಲ_2 <= 0xD7FF)
          || ( u8d.ಮೊದಲ_2 >= 0xE000
            && u8d.ಮೊದಲ_2 <= 0xFFFD)
          || ( u8d.ಮೊದಲ_2 >= 0x10000
            && u8d.ಮೊದಲ_2 <= 0x10FFFF)))
      ಹಿಂತಿರುಗಿ Position + u8d.ಎರಡನೇ_2;
  }
  ಹಿಂತಿರುಗಿ Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  ಆದರೆ (Position == ಕೊನೆ_2)
    ಹಿಂತಿರುಗಿ Position;
  ಆದರೆ (*Position == 0x0D) {
    ಆದರೆ (Position + 1 != ಕೊನೆ_2 && *(Position + 1) == 0x0A)
      ಹಿಂತಿರುಗಿ Position + 2;
    ಹಿಂತಿರುಗಿ Position + 1;
  }

  ಆದರೆ (*Position == 0x0A)
    ಹಿಂತಿರುಗಿ Position + 1;
  ಹಿಂತಿರುಗಿ Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  ಆದರೆ (Position == ಕೊನೆ_2)
    ಹಿಂತಿರುಗಿ Position;
  ಆದರೆ (*Position == ' ')
    ಹಿಂತಿರುಗಿ Position + 1;
  ಹಿಂತಿರುಗಿ Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  ಆದರೆ (Position == ಕೊನೆ_2)
    ಹಿಂತಿರುಗಿ Position;
  ಆದರೆ (*Position == ' ' || *Position == '\t')
    ಹಿಂತಿರುಗಿ Position + 1;
  ಹಿಂತಿರುಗಿ Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  ಆದರೆ (Position == ಕೊನೆ_2)
    ಹಿಂತಿರುಗಿ Position;
  ಆದರೆ (*Position == ' ' || *Position == '\t')
    ಹಿಂತಿರುಗಿ Position;
  ಹಿಂತಿರುಗಿ skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  ಆಗ (ನಿಜ) {
    StringRef::iterator i = (ಇದು->*Func)(Position);
    ಆದರೆ (i == Position)
      ನಿಲ್ಲಿಸು;
    Position = i;
  }
  ಹಿಂತಿರುಗಿ Position;
}

ಶೂನ್ಯ Scanner::advanceWhile(SkipWhileFunc Func) {
  ಸ್ವಯಂಚಾಲಿತ Final = skip_while(Func, Current);
  ಸ್ತಂಭ += Final - Current;
  Current = Final;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ is_ns_hex_digit(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ C) { ಹಿಂತಿರುಗಿ isAlnum(C); }

ಸ್ಥಿರ ತಾರ್ಕಿಕ is_ns_ಶಬ್ದ_char(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ C) { ಹಿಂತಿರುಗಿ C == '-' || isAlpha(C); }

ಶೂನ್ಯ Scanner::ಪರಿಶೀಲನೆ_ns_uri_char() {
  ಆಗ (ನಿಜ) {
    ಆದರೆ (Current == ಕೊನೆ_2)
      ನಿಲ್ಲಿಸು;
    ಆದರೆ ((*Current == '%' && Current + 2 < ಕೊನೆ_2 &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_ಶಬ್ದ_char(*Current) ||
        StringRef(Current, 1).ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++ಸ್ತಂಭ;
    } ಇಲ್ಲದಿದ್ದರೆ {
      ನಿಲ್ಲಿಸು;
    }
  }
}

ತಾರ್ಕಿಕ Scanner::consume(uint32_t Expected) {
  ಆದರೆ (Expected >= 0x80) {
    ಗಣ_Error("Cannot consume non-ascii characters", Current);
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಆದರೆ (Current == ಕೊನೆ_2)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (uint8_t(*Current) >= 0x80) {
    ಗಣ_Error("Cannot consume non-ascii characters", Current);
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಆದರೆ (uint8_t(*Current) == Expected) {
    ++Current;
    ++ಸ್ತಂಭ;
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ಶೂನ್ಯ Scanner::skip(uint32_t ದೂರ_2) {
  Current += ದೂರ_2;
  ಸ್ತಂಭ += ದೂರ_2;
  ಪ್ರತಿಜ್ಞಾ(Current <= ಕೊನೆ_2 && "Skipped past the end");
}

ತಾರ್ಕಿಕ Scanner::isBlankOrBreak(StringRef::iterator Position) {
  ಆದರೆ (Position == ಕೊನೆ_2)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಹಿಂತಿರುಗಿ *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

ತಾರ್ಕಿಕ Scanner::is_Plain_ಸುರಕ್ಷಿತ_Non_Blank(StringRef::iterator Position) {
  ಆದರೆ (Position == ಕೊನೆ_2 || isBlankOrBreak(Position))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (FlowLevel &&
      StringRef(Position, 1).ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(",[]{}") != StringRef::npos)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::is_ಸಾಲು_ಖಾಲಿಗಳು(StringRef ಸಾಲು) {
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ *Position = ಸಾಲು.begin(); Position != ಸಾಲು.ಕೊನೆ(); ++Position)
    ಆದರೆ (!isBlankOrBreak(Position))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::consume_ಸಾಲು_Break_If_Present() {
  ಸ್ವಯಂಚಾಲಿತ ಮುಂದಿನ_2 = skip_b_break(Current);
  ಆದರೆ (ಮುಂದಿನ_2 == Current)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಸ್ತಂಭ = 0;
  ++ಸಾಲು;
  Current = ಮುಂದಿನ_2;
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ಶೂನ್ಯ Scanner::save_Simple_ಕೀಲಿ_Candidate( Token_ಸರತಿ_T::iterator Tok
                                    , ಚಿಹ್ನೆರಹಿತ At_ಸ್ತಂಭ
                                    , ತಾರ್ಕಿಕ IsRequired) {
  ಆದರೆ (Is_Simple_ಕೀಲಿ_Allowed) {
    Simple_ಕೀಲಿ SK;
    SK.Tok = Tok;
    SK.ಸಾಲು = ಸಾಲು;
    SK.ಸ್ತಂಭ = At_ಸ್ತಂಭ;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_ಕೀಲಿಗಳು.ತಳ್ಳು_ಹಿಂಭಾಗ(SK);
  }
}

ಶೂನ್ಯ Scanner::remove_Stale_Simple_ಕೀಲಿ_Candidates() {
  ಗಾಗಿ (Small_ಸದಿಶ_Impl<Simple_ಕೀಲಿ>::iterator i = Simple_ಕೀಲಿಗಳು.begin();
                                            i != Simple_ಕೀಲಿಗಳು.ಕೊನೆ();) {
    ಆದರೆ (i->ಸಾಲು != ಸಾಲು || i->ಸ್ತಂಭ + 1024 < ಸ್ತಂಭ) {
      ಆದರೆ (i->IsRequired)
        ಗಣ_Error( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_ಕೀಲಿಗಳು.erase(i);
    } ಇಲ್ಲದಿದ್ದರೆ {
      ++i;
    }
  }
}

ಶೂನ್ಯ Scanner::remove_Simple_ಕೀಲಿ_Candidates_On_Flow_Level(ಚಿಹ್ನೆರಹಿತ Level) {
  ಆದರೆ (!Simple_ಕೀಲಿಗಳು.ಖಾಲಿಗಳು() && (Simple_ಕೀಲಿಗಳು.ಕೊನೆ() - 1)->FlowLevel == Level)
    Simple_ಕೀಲಿಗಳು.ತೆಗೆ_ಹಿಂಭಾಗ();
}

ತಾರ್ಕಿಕ Scanner::unrollIndent(ಪೂರ್ಣಾಂಕ ಗಮ್ಯ_ಸ್ತಂಭ) {
  Token T;
  // Indentation is ignored in flow.
  ಆದರೆ (FlowLevel != 0)
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಆಗ (Indent > ಗಮ್ಯ_ಸ್ತಂಭ) {
    T.ಬಗೆ_2 = Token::TK_ಖಂಡ_ಕೊನೆ;
    T.Range = StringRef(Current, 1);
    Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);
    Indent = Indents.ತೆಗೆ_ಹಿಂಭಾಗ_val();
  }

  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::rollIndent( ಪೂರ್ಣಾಂಕ ಗಮ್ಯ_ಸ್ತಂಭ
                        , Token::Token_ಬಗೆ ಬಗೆ_2
                        , Token_ಸರತಿ_T::iterator ಸೇರಿಸು_ಬಿಂದು) {
  ಆದರೆ (FlowLevel)
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಆದರೆ (Indent < ಗಮ್ಯ_ಸ್ತಂಭ) {
    Indents.ತಳ್ಳು_ಹಿಂಭಾಗ(Indent);
    Indent = ಗಮ್ಯ_ಸ್ತಂಭ;

    Token T;
    T.ಬಗೆ_2 = ಬಗೆ_2;
    T.Range = StringRef(Current, 0);
    Token_ಸರತಿ.ಸೇರಿಸು(ಸೇರಿಸು_ಬಿಂದು, T);
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ಶೂನ್ಯ Scanner::skipComment() {
  ಆದರೆ (Current == ಕೊನೆ_2 || *Current != '#')
    ಹಿಂತಿರುಗಿ;
  ಆಗ (ನಿಜ) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    ಆದರೆ (I == Current)
      ನಿಲ್ಲಿಸು;
    Current = I;
    ++ಸ್ತಂಭ;
  }
}

ಶೂನ್ಯ Scanner::ಪರಿಶೀಲನೆ_ಗಮ್ಯ_ಮುಂದಿನ_Token() {
  ಆಗ (ನಿಜ) {
    ಆಗ (Current != ಕೊನೆ_2 && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    ಆದರೆ (i == Current)
      ನಿಲ್ಲಿಸು;
    Current = i;
    ++ಸಾಲು;
    ಸ್ತಂಭ = 0;
    // New lines may start a simple key.
    ಆದರೆ (!FlowLevel)
      Is_Simple_ಕೀಲಿ_Allowed = ನಿಜ;
  }
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_ಹರಿವು_ಆರಂಭ() {
  Is_ಆರಂಭ_ದ_ಹರಿವು = ಸುಳ್ಳು;

  EncodingInfo EI = getUnicodeEncoding(current_ಒಳಹರಿವು());

  Token T;
  T.ಬಗೆ_2 = Token::TK_ಹರಿವು_ಆರಂಭ;
  T.Range = StringRef(Current, EI.ಎರಡನೇ_2);
  Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);
  Current += EI.ಎರಡನೇ_2;
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_ಹರಿವು_ಕೊನೆ() {
  // Force an ending new line if one isn't present.
  ಆದರೆ (ಸ್ತಂಭ != 0) {
    ಸ್ತಂಭ = 0;
    ++ಸಾಲು;
  }

  unrollIndent(-1);
  Simple_ಕೀಲಿಗಳು.clear();
  Is_Simple_ಕೀಲಿ_Allowed = ಸುಳ್ಳು;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow = ಸುಳ್ಳು;

  Token T;
  T.ಬಗೆ_2 = Token::TK_ಹರಿವು_ಕೊನೆ;
  T.Range = StringRef(Current, 0);
  Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_Directive() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_ಕೀಲಿಗಳು.clear();
  Is_Simple_ಕೀಲಿ_Allowed = ಸುಳ್ಳು;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow = ಸುಳ್ಳು;

  StringRef::iterator ಆರಂಭ = Current;
  consume('%');
  StringRef::iterator Name_ಆರಂಭ = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(Name_ಆರಂಭ, Current - Name_ಆರಂಭ);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  ಆದರೆ (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.ಬಗೆ_2 = Token::TK_ಆವೃತ್ತಿ_Directive;
    T.Range = StringRef(ಆರಂಭ, Current - ಆರಂಭ);
    Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);
    ಹಿಂತಿರುಗಿ ನಿಜ;
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.ಬಗೆ_2 = Token::TK_ಟ್ಯಾಗ್_Directive;
    T.Range = StringRef(ಆರಂಭ, Current - ಆರಂಭ);
    Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_Document_Indicator(ತಾರ್ಕಿಕ Is_ಆರಂಭ) {
  unrollIndent(-1);
  Simple_ಕೀಲಿಗಳು.clear();
  Is_Simple_ಕೀಲಿ_Allowed = ಸುಳ್ಳು;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow = ಸುಳ್ಳು;

  Token T;
  T.ಬಗೆ_2 = Is_ಆರಂಭ ? Token::TK_Document_ಆರಂಭ : Token::TK_Document_ಕೊನೆ;
  T.Range = StringRef(Current, 3);
  skip(3);
  Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_Flow_Collection_ಆರಂಭ(ತಾರ್ಕಿಕ IsSequence) {
  Token T;
  T.ಬಗೆ_2 = IsSequence ? Token::TK_Flow_Sequence_ಆರಂಭ
                      : Token::TK_Flow_Mapping_ಆರಂಭ;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);

  // [ and { may begin a simple key.
  save_Simple_ಕೀಲಿ_Candidate(--Token_ಸರತಿ.ಕೊನೆ(), ಸ್ತಂಭ - 1, ಸುಳ್ಳು);

  // And may also be followed by a simple key.
  Is_Simple_ಕೀಲಿ_Allowed = ನಿಜ;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow = ಸುಳ್ಳು;
  ++FlowLevel;
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_Flow_Collection_ಕೊನೆ(ತಾರ್ಕಿಕ IsSequence) {
  remove_Simple_ಕೀಲಿ_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ಕೀಲಿ_Allowed = ಸುಳ್ಳು;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow = ನಿಜ;
  Token T;
  T.ಬಗೆ_2 = IsSequence ? Token::TK_Flow_Sequence_ಕೊನೆ
                      : Token::TK_Flow_Mapping_ಕೊನೆ;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);
  ಆದರೆ (FlowLevel)
    --FlowLevel;
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_Flow_ನಮೂದು() {
  remove_Simple_ಕೀಲಿ_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ಕೀಲಿ_Allowed = ನಿಜ;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow = ಸುಳ್ಳು;
  Token T;
  T.ಬಗೆ_2 = Token::TK_Flow_ನಮೂದು;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_ಖಂಡ_ನಮೂದು() {
  rollIndent(ಸ್ತಂಭ, Token::TK_ಖಂಡ_Sequence_ಆರಂಭ, Token_ಸರತಿ.ಕೊನೆ());
  remove_Simple_ಕೀಲಿ_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ಕೀಲಿ_Allowed = ನಿಜ;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow = ಸುಳ್ಳು;
  Token T;
  T.ಬಗೆ_2 = Token::TK_ಖಂಡ_ನಮೂದು;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_ಕೀಲಿ() {
  ಆದರೆ (!FlowLevel)
    rollIndent(ಸ್ತಂಭ, Token::TK_ಖಂಡ_Mapping_ಆರಂಭ, Token_ಸರತಿ.ಕೊನೆ());

  remove_Simple_ಕೀಲಿ_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ಕೀಲಿ_Allowed = !FlowLevel;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow = ಸುಳ್ಳು;

  Token T;
  T.ಬಗೆ_2 = Token::TK_ಕೀಲಿ;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_ಮೌಲ್ಯ() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  ಆದರೆ (!Simple_ಕೀಲಿಗಳು.ಖಾಲಿಗಳು()) {
    Simple_ಕೀಲಿ SK = Simple_ಕೀಲಿಗಳು.ತೆಗೆ_ಹಿಂಭಾಗ_val();
    Token T;
    T.ಬಗೆ_2 = Token::TK_ಕೀಲಿ;
    T.Range = SK.Tok->Range;
    Token_ಸರತಿ_T::iterator i, e;
    ಗಾಗಿ (i = Token_ಸರತಿ.begin(), e = Token_ಸರತಿ.ಕೊನೆ(); i != e; ++i) {
      ಆದರೆ (i == SK.Tok)
        ನಿಲ್ಲಿಸು;
    }
    ಆದರೆ (i == e) {
      Failed = ನಿಜ;
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }
    i = Token_ಸರತಿ.ಸೇರಿಸು(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.ಸ್ತಂಭ, Token::TK_ಖಂಡ_Mapping_ಆರಂಭ, i);

    Is_Simple_ಕೀಲಿ_Allowed = ಸುಳ್ಳು;
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಆದರೆ (!FlowLevel)
      rollIndent(ಸ್ತಂಭ, Token::TK_ಖಂಡ_Mapping_ಆರಂಭ, Token_ಸರತಿ.ಕೊನೆ());
    Is_Simple_ಕೀಲಿ_Allowed = !FlowLevel;
  }
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow = ಸುಳ್ಳು;

  Token T;
  T.ಬಗೆ_2 = Token::TK_ಮೌಲ್ಯ;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE ಸ್ಥಿರ ತಾರ್ಕಿಕ
wasEscaped(StringRef::iterator ಮೊದಲ, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
ಸ್ಥಿರ ತಾರ್ಕಿಕ wasEscaped(StringRef::iterator ಮೊದಲ,
                       StringRef::iterator Position) {
  ಪ್ರತಿಜ್ಞಾ(Position - 1 >= ಮೊದಲ);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  ಆಗ (I >= ಮೊದಲ && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  ಹಿಂತಿರುಗಿ (Position - 1 - I) % 2 == 1;
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_Flow_Scalar(ತಾರ್ಕಿಕ IsDoubleQuoted) {
  StringRef::iterator ಆರಂಭ = Current;
  ಚಿಹ್ನೆರಹಿತ ಲಂಬಸಾಲು_ಆರಂಭ = ಸ್ತಂಭ;
  ಆದರೆ (IsDoubleQuoted) {
    ಮಾಡು {
      ++Current;
      ಆಗ (Current != ಕೊನೆ_2 && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } ಆಗ (   Current != ಕೊನೆ_2
             && *(Current - 1) == '\\'
             && wasEscaped(ಆರಂಭ + 1, Current));
  } ಇಲ್ಲದಿದ್ದರೆ {
    skip(1);
    ಆಗ (Current != ಕೊನೆ_2) {
      // Skip a ' followed by another '.
      ಆದರೆ (Current + 1 < ಕೊನೆ_2 && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        ಮುಂದುವರಿ;
      } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*Current == '\'')
        ನಿಲ್ಲಿಸು;
      StringRef::iterator i = skip_nb_char(Current);
      ಆದರೆ (i == Current) {
        i = skip_b_break(Current);
        ಆದರೆ (i == Current)
          ನಿಲ್ಲಿಸು;
        Current = i;
        ಸ್ತಂಭ = 0;
        ++ಸಾಲು;
      } ಇಲ್ಲದಿದ್ದರೆ {
        ಆದರೆ (i == ಕೊನೆ_2)
          ನಿಲ್ಲಿಸು;
        Current = i;
        ++ಸ್ತಂಭ;
      }
    }
  }

  ಆದರೆ (Current == ಕೊನೆ_2) {
    ಗಣ_Error("Expected quote at end of scalar", Current);
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.ಬಗೆ_2 = Token::TK_Scalar;
  T.Range = StringRef(ಆರಂಭ, Current - ಆರಂಭ);
  Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);

  save_Simple_ಕೀಲಿ_Candidate(--Token_ಸರತಿ.ಕೊನೆ(), ಲಂಬಸಾಲು_ಆರಂಭ, ಸುಳ್ಳು);

  Is_Simple_ಕೀಲಿ_Allowed = ಸುಳ್ಳು;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow = ನಿಜ;

  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_Plain_Scalar() {
  StringRef::iterator ಆರಂಭ = Current;
  ಚಿಹ್ನೆರಹಿತ ಲಂಬಸಾಲು_ಆರಂಭ = ಸ್ತಂಭ;
  ಚಿಹ್ನೆರಹಿತ LeadingBlanks = 0;
  ಪ್ರತಿಜ್ಞಾ(Indent >= -1 && "Indent must be >= -1 !");
  ಚಿಹ್ನೆರಹಿತ indent = static_cast<ಚಿಹ್ನೆರಹಿತ>(Indent + 1);
  ಆಗ (Current != ಕೊನೆ_2) {
    ಆದರೆ (*Current == '#')
      ನಿಲ್ಲಿಸು;

    ಆಗ (Current != ಕೊನೆ_2 &&
           ((*Current != ':' && is_Plain_ಸುರಕ್ಷಿತ_Non_Blank(Current)) ||
            (*Current == ':' && is_Plain_ಸುರಕ್ಷಿತ_Non_Blank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      ಆದರೆ (i == Current)
        ನಿಲ್ಲಿಸು;
      Current = i;
      ++ಸ್ತಂಭ;
    }

    // Are we at the end?
    ಆದರೆ (!isBlankOrBreak(Current))
      ನಿಲ್ಲಿಸು;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    ಆಗ (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      ಆದರೆ (i != Tmp) {
        ಆದರೆ (LeadingBlanks && (ಸ್ತಂಭ < indent) && *Tmp == '\t') {
          ಗಣ_Error("Found invalid tab character in indentation", Tmp);
          ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
        }
        Tmp = i;
        ++ಸ್ತಂಭ;
      } ಇಲ್ಲದಿದ್ದರೆ {
        i = skip_b_break(Tmp);
        ಆದರೆ (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        ಸ್ತಂಭ = 0;
        ++ಸಾಲು;
      }
    }

    ಆದರೆ (!FlowLevel && ಸ್ತಂಭ < indent)
      ನಿಲ್ಲಿಸು;

    Current = Tmp;
  }
  ಆದರೆ (ಆರಂಭ == Current) {
    ಗಣ_Error("Got empty plain scalar", ಆರಂಭ);
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  Token T;
  T.ಬಗೆ_2 = Token::TK_Scalar;
  T.Range = StringRef(ಆರಂಭ, Current - ಆರಂಭ);
  Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);

  // Plain scalars can be simple keys.
  save_Simple_ಕೀಲಿ_Candidate(--Token_ಸರತಿ.ಕೊನೆ(), ಲಂಬಸಾಲು_ಆರಂಭ, ಸುಳ್ಳು);

  Is_Simple_ಕೀಲಿ_Allowed = ಸುಳ್ಳು;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow = ಸುಳ್ಳು;

  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_Alias_Or_Anchor(ತಾರ್ಕಿಕ IsAlias) {
  StringRef::iterator ಆರಂಭ = Current;
  ಚಿಹ್ನೆರಹಿತ ಲಂಬಸಾಲು_ಆರಂಭ = ಸ್ತಂಭ;
  skip(1);
  ಆಗ (Current != ಕೊನೆ_2) {
    ಆದರೆ (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      ನಿಲ್ಲಿಸು;
    StringRef::iterator i = skip_ns_char(Current);
    ಆದರೆ (i == Current)
      ನಿಲ್ಲಿಸು;
    Current = i;
    ++ಸ್ತಂಭ;
  }

  ಆದರೆ (ಆರಂಭ + 1 == Current) {
    ಗಣ_Error("Got empty alias or anchor", ಆರಂಭ);
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  Token T;
  T.ಬಗೆ_2 = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(ಆರಂಭ, Current - ಆರಂಭ);
  Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);

  // Alias and anchors can be simple keys.
  save_Simple_ಕೀಲಿ_Candidate(--Token_ಸರತಿ.ಕೊನೆ(), ಲಂಬಸಾಲು_ಆರಂಭ, ಸುಳ್ಳು);

  Is_Simple_ಕೀಲಿ_Allowed = ಸುಳ್ಳು;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow = ಸುಳ್ಳು;

  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_ಖಂಡ_Scalar_Indicators(ಅಕ್ಷರ &StyleIndicator,
                                        ಅಕ್ಷರ &ChompingIndicator,
                                        ಚಿಹ್ನೆರಹಿತ &IndentIndicator,
                                        ತಾರ್ಕಿಕ &Is_ಮುಗಿದ) {
  StyleIndicator = ಪರಿಶೀಲನೆ_ಖಂಡ_Style_Indicator();
  ಆದರೆ (!ಪರಿಶೀಲನೆ_ಖಂಡ_Scalar_ಶೀರ್ಷಿಕೆ(ChompingIndicator, IndentIndicator, Is_ಮುಗಿದ))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ಅಕ್ಷರ Scanner::ಪರಿಶೀಲನೆ_ಖಂಡ_Style_Indicator() {
  ಅಕ್ಷರ Indicator = ' ';
  ಆದರೆ (Current != ಕೊನೆ_2 && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  ಹಿಂತಿರುಗಿ Indicator;
}

ಅಕ್ಷರ Scanner::ಪರಿಶೀಲನೆ_ಖಂಡ_Chomping_Indicator() {
  ಅಕ್ಷರ Indicator = ' ';
  ಆದರೆ (Current != ಕೊನೆ_2 && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  ಹಿಂತಿರುಗಿ Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ get_Chomped_ಸಾಲು_Breaks(ಅಕ್ಷರ ChompingIndicator,
                                     ಚಿಹ್ನೆರಹಿತ ಸಾಲು_Breaks, StringRef Str) {
  ಆದರೆ (ChompingIndicator == '-') // Strip all line breaks.
    ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ (ChompingIndicator == '+') // Keep all line breaks.
    ಹಿಂತಿರುಗಿ ಸಾಲು_Breaks;
  // Clip trailing lines.
  ಹಿಂತಿರುಗಿ Str.ಖಾಲಿಗಳು() ? 0 : 1;
}

ಚಿಹ್ನೆರಹಿತ Scanner::ಪರಿಶೀಲನೆ_ಖಂಡ_Indentation_Indicator() {
  ಚಿಹ್ನೆರಹಿತ Indent = 0;
  ಆದರೆ (Current != ಕೊನೆ_2 && (*Current >= '1' && *Current <= '9')) {
    Indent = ಚಿಹ್ನೆರಹಿತ(*Current - '0');
    skip(1);
  }
  ಹಿಂತಿರುಗಿ Indent;
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_ಖಂಡ_Scalar_ಶೀರ್ಷಿಕೆ(ಅಕ್ಷರ &ChompingIndicator,
                                    ಚಿಹ್ನೆರಹಿತ &IndentIndicator, ತಾರ್ಕಿಕ &Is_ಮುಗಿದ) {
  ಸ್ವಯಂಚಾಲಿತ ಆರಂಭ = Current;

  ChompingIndicator = ಪರಿಶೀಲನೆ_ಖಂಡ_Chomping_Indicator();
  IndentIndicator = ಪರಿಶೀಲನೆ_ಖಂಡ_Indentation_Indicator();
  // Check for the chomping indicator once again.
  ಆದರೆ (ChompingIndicator == ' ')
    ChompingIndicator = ಪರಿಶೀಲನೆ_ಖಂಡ_Chomping_Indicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  ಆದರೆ (Current == ಕೊನೆ_2) { // EOF, we have an empty scalar.
    Token T;
    T.ಬಗೆ_2 = Token::TK_ಖಂಡ_Scalar;
    T.Range = StringRef(ಆರಂಭ, Current - ಆರಂಭ);
    Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);
    Is_ಮುಗಿದ = ನಿಜ;
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }

  ಆದರೆ (!consume_ಸಾಲು_Break_If_Present()) {
    ಗಣ_Error("Expected a line break after block scalar header", Current);
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::ಕಂಡುಹಿಡಿ_ಖಂಡ_Scalar_Indent(ಚಿಹ್ನೆರಹಿತ &ಖಂಡ_Indent,
                                    ಚಿಹ್ನೆರಹಿತ ಖಂಡ_Exit_Indent,
                                    ಚಿಹ್ನೆರಹಿತ &ಸಾಲು_Breaks, ತಾರ್ಕಿಕ &Is_ಮುಗಿದ) {
  ಚಿಹ್ನೆರಹಿತ Max_All_Space_ಸಾಲು_Characters = 0;
  StringRef::iterator Longest_All_Space_ಸಾಲು;

  ಆಗ (ನಿಜ) {
    advanceWhile(&Scanner::skip_s_space);
    ಆದರೆ (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      ಆದರೆ (ಸ್ತಂಭ <= ಖಂಡ_Exit_Indent) { // End of the block literal.
        Is_ಮುಗಿದ = ನಿಜ;
        ಹಿಂತಿರುಗಿ ನಿಜ;
      }
      // We found the block's indentation.
      ಖಂಡ_Indent = ಸ್ತಂಭ;
      ಆದರೆ (Max_All_Space_ಸಾಲು_Characters > ಖಂಡ_Indent) {
        ಗಣ_Error(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_ಸಾಲು);
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      }
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }
    ಆದರೆ (skip_b_break(Current) != Current &&
        ಸ್ತಂಭ > Max_All_Space_ಸಾಲು_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_ಸಾಲು_Characters = ಸ್ತಂಭ;
      Longest_All_Space_ಸಾಲು = Current;
    }

    // Check for EOF.
    ಆದರೆ (Current == ಕೊನೆ_2) {
      Is_ಮುಗಿದ = ನಿಜ;
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }

    ಆದರೆ (!consume_ಸಾಲು_Break_If_Present()) {
      Is_ಮುಗಿದ = ನಿಜ;
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }
    ++ಸಾಲು_Breaks;
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_ಖಂಡ_Scalar_Indent(ಚಿಹ್ನೆರಹಿತ ಖಂಡ_Indent,
                                    ಚಿಹ್ನೆರಹಿತ ಖಂಡ_Exit_Indent, ತಾರ್ಕಿಕ &Is_ಮುಗಿದ) {
  // Skip the indentation.
  ಆಗ (ಸ್ತಂಭ < ಖಂಡ_Indent) {
    ಸ್ವಯಂಚಾಲಿತ I = skip_s_space(Current);
    ಆದರೆ (I == Current)
      ನಿಲ್ಲಿಸು;
    Current = I;
    ++ಸ್ತಂಭ;
  }

  ಆದರೆ (skip_nb_char(Current) == Current)
    ಹಿಂತಿರುಗಿ ನಿಜ;

  ಆದರೆ (ಸ್ತಂಭ <= ಖಂಡ_Exit_Indent) { // End of the block literal.
    Is_ಮುಗಿದ = ನಿಜ;
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }

  ಆದರೆ (ಸ್ತಂಭ < ಖಂಡ_Indent) {
    ಆದರೆ (Current != ಕೊನೆ_2 && *Current == '#') { // Trailing comment.
      Is_ಮುಗಿದ = ನಿಜ;
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }
    ಗಣ_Error("A text line is less indented than the block scalar", Current);
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಹಿಂತಿರುಗಿ ನಿಜ; // A normal text line.
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_ಖಂಡ_Scalar(ತಾರ್ಕಿಕ IsLiteral) {
  ಪ್ರತಿಜ್ಞಾ(*Current == '|' || *Current == '>');
  ಅಕ್ಷರ StyleIndicator;
  ಅಕ್ಷರ ChompingIndicator;
  ಚಿಹ್ನೆರಹಿತ ಖಂಡ_Indent;
  ತಾರ್ಕಿಕ Is_ಮುಗಿದ = ಸುಳ್ಳು;
  ಆದರೆ (!ಪರಿಶೀಲನೆ_ಖಂಡ_Scalar_Indicators(StyleIndicator, ChompingIndicator, ಖಂಡ_Indent,
                                 Is_ಮುಗಿದ))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (Is_ಮುಗಿದ)
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ತಾರ್ಕಿಕ IsFolded = StyleIndicator == '>';

  ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ *ಆರಂಭ = Current;
  ಚಿಹ್ನೆರಹಿತ ಖಂಡ_Exit_Indent = Indent < 0 ? 0 : (ಚಿಹ್ನೆರಹಿತ)Indent;
  ಚಿಹ್ನೆರಹಿತ ಸಾಲು_Breaks = 0;
  ಆದರೆ (ಖಂಡ_Indent == 0) {
    ಆದರೆ (!ಕಂಡುಹಿಡಿ_ಖಂಡ_Scalar_Indent(ಖಂಡ_Indent, ಖಂಡ_Exit_Indent, ಸಾಲು_Breaks,
                               Is_ಮುಗಿದ))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  ಆಗ (!Is_ಮುಗಿದ) {
    ಆದರೆ (!ಪರಿಶೀಲನೆ_ಖಂಡ_Scalar_Indent(ಖಂಡ_Indent, ಖಂಡ_Exit_Indent, Is_ಮುಗಿದ))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    ಆದರೆ (Is_ಮುಗಿದ)
      ನಿಲ್ಲಿಸು;

    // Parse the current line.
    ಸ್ವಯಂಚಾಲಿತ ಸಾಲು_ಆರಂಭ = Current;
    advanceWhile(&Scanner::skip_nb_char);
    ಆದರೆ (ಸಾಲು_ಆರಂಭ != Current) {
      ಆದರೆ (ಸಾಲು_Breaks && IsFolded && !Scanner::is_ಸಾಲು_ಖಾಲಿಗಳು(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        ಆದರೆ (ಸಾಲು_Breaks == 1) {
          Str.append(ಸಾಲು_Breaks,
                     is_ಸಾಲು_ಖಾಲಿಗಳು(StringRef(ಸಾಲು_ಆರಂಭ, Current - ಸಾಲು_ಆರಂಭ))
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
      Str.append(StringRef(ಸಾಲು_ಆರಂಭ, Current - ಸಾಲು_ಆರಂಭ));
      ಸಾಲು_Breaks = 0;
    }

    // Check for EOF.
    ಆದರೆ (Current == ಕೊನೆ_2)
      ನಿಲ್ಲಿಸು;

    ಆದರೆ (!consume_ಸಾಲು_Break_If_Present())
      ನಿಲ್ಲಿಸು;
    ++ಸಾಲು_Breaks;
  }

  ಆದರೆ (Current == ಕೊನೆ_2 && !ಸಾಲು_Breaks)
    // Ensure that there is at least one line break before the end of file.
    ಸಾಲು_Breaks = 1;
  Str.append(get_Chomped_ಸಾಲು_Breaks(ChompingIndicator, ಸಾಲು_Breaks, Str), '\n');

  // New lines may start a simple key.
  ಆದರೆ (!FlowLevel)
    Is_Simple_ಕೀಲಿ_Allowed = ನಿಜ;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow = ಸುಳ್ಳು;

  Token T;
  T.ಬಗೆ_2 = Token::TK_ಖಂಡ_Scalar;
  T.Range = StringRef(ಆರಂಭ, Current - ಆರಂಭ);
  T.ಮೌಲ್ಯ = std::string(Str);
  Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::ಪರಿಶೀಲನೆ_ಟ್ಯಾಗ್() {
  StringRef::iterator ಆರಂಭ = Current;
  ಚಿಹ್ನೆರಹಿತ ಲಂಬಸಾಲು_ಆರಂಭ = ಸ್ತಂಭ;
  skip(1); // Eat !.
  ಆದರೆ (Current == ಕೊನೆ_2 || isBlankOrBreak(Current)); // An empty tag.
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*Current == '<') {
    skip(1);
    ಪರಿಶೀಲನೆ_ns_uri_char();
    ಆದರೆ (!consume('>'))
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  } ಇಲ್ಲದಿದ್ದರೆ {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.ಬಗೆ_2 = Token::TK_ಟ್ಯಾಗ್;
  T.Range = StringRef(ಆರಂಭ, Current - ಆರಂಭ);
  Token_ಸರತಿ.ತಳ್ಳು_ಹಿಂಭಾಗ(T);

  // Tags can be simple keys.
  save_Simple_ಕೀಲಿ_Candidate(--Token_ಸರತಿ.ಕೊನೆ(), ಲಂಬಸಾಲು_ಆರಂಭ, ಸುಳ್ಳು);

  Is_Simple_ಕೀಲಿ_Allowed = ಸುಳ್ಳು;
  Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow = ಸುಳ್ಳು;

  ಹಿಂತಿರುಗಿ ನಿಜ;
}

ತಾರ್ಕಿಕ Scanner::fetch_ಇನ್ನಷ್ಟು_ಟೋಕನುಗಳು() {
  ಆದರೆ (Is_ಆರಂಭ_ದ_ಹರಿವು)
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_ಹರಿವು_ಆರಂಭ();

  ಪರಿಶೀಲನೆ_ಗಮ್ಯ_ಮುಂದಿನ_Token();

  ಆದರೆ (Current == ಕೊನೆ_2)
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_ಹರಿವು_ಕೊನೆ();

  remove_Stale_Simple_ಕೀಲಿ_Candidates();

  unrollIndent(ಸ್ತಂಭ);

  ಆದರೆ (ಸ್ತಂಭ == 0 && *Current == '%')
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_Directive();

  ಆದರೆ (ಸ್ತಂಭ == 0 && Current + 3 <= ಕೊನೆ_2
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == ಕೊನೆ_2 || isBlankOrBreak(Current + 3)))
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_Document_Indicator(ನಿಜ);

  ಆದರೆ (ಸ್ತಂಭ == 0 && Current + 3 <= ಕೊನೆ_2
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == ಕೊನೆ_2 || isBlankOrBreak(Current + 3)))
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_Document_Indicator(ಸುಳ್ಳು);

  ಆದರೆ (*Current == '[')
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_Flow_Collection_ಆರಂಭ(ನಿಜ);

  ಆದರೆ (*Current == '{')
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_Flow_Collection_ಆರಂಭ(ಸುಳ್ಳು);

  ಆದರೆ (*Current == ']')
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_Flow_Collection_ಕೊನೆ(ನಿಜ);

  ಆದರೆ (*Current == '}')
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_Flow_Collection_ಕೊನೆ(ಸುಳ್ಳು);

  ಆದರೆ (*Current == ',')
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_Flow_ನಮೂದು();

  ಆದರೆ (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == ಕೊನೆ_2))
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_ಖಂಡ_ನಮೂದು();

  ಆದರೆ (*Current == '?' && (Current + 1 == ಕೊನೆ_2 || isBlankOrBreak(Current + 1)))
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_ಕೀಲಿ();

  ಆದರೆ (*Current == ':' &&
      (!is_Plain_ಸುರಕ್ಷಿತ_Non_Blank(Current + 1) || Is_Adjacent_ಮೌಲ್ಯ_Allowed_ಒಳ_Flow))
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_ಮೌಲ್ಯ();

  ಆದರೆ (*Current == '*')
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_Alias_Or_Anchor(ನಿಜ);

  ಆದರೆ (*Current == '&')
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_Alias_Or_Anchor(ಸುಳ್ಳು);

  ಆದರೆ (*Current == '!')
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_ಟ್ಯಾಗ್();

  ಆದರೆ (*Current == '|' && !FlowLevel)
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_ಖಂಡ_Scalar(ನಿಜ);

  ಆದರೆ (*Current == '>' && !FlowLevel)
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_ಖಂಡ_Scalar(ಸುಳ್ಳು);

  ಆದರೆ (*Current == '\'')
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_Flow_Scalar(ಸುಳ್ಳು);

  ಆದರೆ (*Current == '"')
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_Flow_Scalar(ನಿಜ);

  // Get a plain scalar.
  StringRef ಮೊದಲ_Char(Current, 1);
  ಆದರೆ ((!isBlankOrBreak(Current) &&
       ಮೊದಲ_Char.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (ಮೊದಲ_Char.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ("?:-") != StringRef::npos &&
       is_Plain_ಸುರಕ್ಷಿತ_Non_Blank(Current + 1)))
    ಹಿಂತಿರುಗಿ ಪರಿಶೀಲನೆ_Plain_Scalar();

  ಗಣ_Error("Unrecognized character while tokenizing.", Current);
  ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
}

ಹರಿವು::ಹರಿವು(StringRef ಒಳಹರಿವು, ಮೂಲ_Mgr &SM, ತಾರ್ಕಿಕ ShowColors,
               std::error_ಸಂಕೇತ *EC)
    : scanner(ಹೊಸ Scanner(ಒಳಹರಿವು, SM, ShowColors, EC)) {}

ಹರಿವು::ಹರಿವು(ಸ್ಮರಣೆ_ಬಫರ್_Ref ಒಳಹರಿವು_ಬಫರ್, ಮೂಲ_Mgr &SM, ತಾರ್ಕಿಕ ShowColors,
               std::error_ಸಂಕೇತ *EC)
    : scanner(ಹೊಸ Scanner(ಒಳಹರಿವು_ಬಫರ್, SM, ShowColors, EC)) {}

ಹರಿವು::~ಹರಿವು() = ಪೂರ್ವನಿಯೋಜಿತ;

ತಾರ್ಕಿಕ ಹರಿವು::failed() { ಹಿಂತಿರುಗಿ scanner->failed(); }

ಶೂನ್ಯ ಹರಿವು::printError(ಗಂಟು *N, ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_4, ಮೂಲ_Mgr::Diag_ಬಗೆ ಬಗೆ_2) {
  printError(N ? N->get_ಮೂಲ_Range() : SMRange(), ಸಂದೇಶ_4, ಬಗೆ_2);
}

ಶೂನ್ಯ ಹರಿವು::printError(ಸ್ಥಿರಾಂಕ SMRange &Range, ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_4,
                        ಮೂಲ_Mgr::Diag_ಬಗೆ ಬಗೆ_2) {
  scanner->printError(Range.ಆರಂಭ, ಬಗೆ_2, ಸಂದೇಶ_4, Range);
}

document_iterator ಹರಿವು::begin() {
  ಆದರೆ (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->get_ಮುಂದಿನ();

  CurrentDoc.reset(ಹೊಸ Document(*ಇದು));
  ಹಿಂತಿರುಗಿ document_iterator(CurrentDoc);
}

document_iterator ಹರಿವು::ಕೊನೆ() {
  ಹಿಂತಿರುಗಿ document_iterator();
}

ಶೂನ್ಯ ಹರಿವು::skip() {
  ಗಾಗಿ (Document &Doc : *ಇದು)
    Doc.skip();
}

ಗಂಟು::ಗಂಟು(ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), ಟ್ಯಾಗ್(T) {
  SMLoc ಆರಂಭ = SMLoc::get_From_ಸೂಚಕ(peek_ಮುಂದಿನ().Range.begin());
  ಮೂಲ_Range = SMRange(ಆರಂಭ, ಆರಂಭ);
}

std::string ಗಂಟು::get_Verbatim_ಟ್ಯಾಗ್() ಸ್ಥಿರಾಂಕ {
  StringRef Raw = get_Raw_ಟ್ಯಾಗ್();
  ಆದರೆ (!Raw.ಖಾಲಿಗಳು() && Raw != "!") {
    std::string Ret;
    ಆದರೆ (Raw.ಕಂಡುಹಿಡಿ_ಕೊನೆಯ_ದ('!') == 0) {
      Ret = std::string(Doc->get_ಟ್ಯಾಗ್_Map().ಕಂಡುಹಿಡಿ("!")->ಎರಡನೇ_2);
      Ret += Raw.substr(1);
      ಹಿಂತಿರುಗಿ Ret;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Raw.starts_with("!!")) {
      Ret = std::string(Doc->get_ಟ್ಯಾಗ್_Map().ಕಂಡುಹಿಡಿ("!!")->ಎರಡನೇ_2);
      Ret += Raw.substr(2);
      ಹಿಂತಿರುಗಿ Ret;
    } ಇಲ್ಲದಿದ್ದರೆ {
      StringRef ಟ್ಯಾಗ್_Handle = Raw.substr(0, Raw.ಕಂಡುಹಿಡಿ_ಕೊನೆಯ_ದ('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->get_ಟ್ಯಾಗ್_Map().ಕಂಡುಹಿಡಿ(ಟ್ಯಾಗ್_Handle);
      ಆದರೆ (It != Doc->get_ಟ್ಯಾಗ್_Map().ಕೊನೆ())
        Ret = std::string(It->ಎರಡನೇ_2);
      ಇಲ್ಲದಿದ್ದರೆ {
        Token T;
        T.ಬಗೆ_2 = Token::TK_ಟ್ಯಾಗ್;
        T.Range = ಟ್ಯಾಗ್_Handle;
        ಗಣ_Error(Twine("Unknown tag handle ") + ಟ್ಯಾಗ್_Handle, T);
      }
      Ret += Raw.substr(Raw.ಕಂಡುಹಿಡಿ_ಕೊನೆಯ_ದ('!') + 1);
      ಹಿಂತಿರುಗಿ Ret;
    }
  }

  ಆಯ್ಕೆ (getType()) {
  ಸ್ಥಿತಿ NK_Null:
    ಹಿಂತಿರುಗಿ "tag:yaml.org,2002:null";
  ಸ್ಥಿತಿ NK_Scalar:
  ಸ್ಥಿತಿ NK_ಖಂಡ_Scalar:
    // TODO: Tag resolution.
    ಹಿಂತಿರುಗಿ "tag:yaml.org,2002:str";
  ಸ್ಥಿತಿ NK_Mapping:
    ಹಿಂತಿರುಗಿ "tag:yaml.org,2002:map";
  ಸ್ಥಿತಿ NK_Sequence:
    ಹಿಂತಿರುಗಿ "tag:yaml.org,2002:seq";
  }

  ಹಿಂತಿರುಗಿ "";
}

Token &ಗಂಟು::peek_ಮುಂದಿನ() {
  ಹಿಂತಿರುಗಿ Doc->peek_ಮುಂದಿನ();
}

Token ಗಂಟು::get_ಮುಂದಿನ() {
  ಹಿಂತಿರುಗಿ Doc->get_ಮುಂದಿನ();
}

ಗಂಟು *ಗಂಟು::parse_ಖಂಡ_ಗಂಟು() {
  ಹಿಂತಿರುಗಿ Doc->parse_ಖಂಡ_ಗಂಟು();
}

BumpPtrAllocator &ಗಂಟು::getAllocator() {
  ಹಿಂತಿರುಗಿ Doc->ಗಂಟು_Allocator;
}

ಶೂನ್ಯ ಗಂಟು::ಗಣ_Error(ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_4, Token &Tok) ಸ್ಥಿರಾಂಕ {
  Doc->ಗಣ_Error(ಸಂದೇಶ_4, Tok);
}

ತಾರ್ಕಿಕ ಗಂಟು::failed() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ Doc->failed();
}

StringRef Scalar_ಗಂಟು::get_ಮೌಲ್ಯ(Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Storage) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಮೌಲ್ಯ[0] == '"')
    ಹಿಂತಿರುಗಿ get_Double_Quoted_ಮೌಲ್ಯ(ಮೌಲ್ಯ, Storage);
  ಆದರೆ (ಮೌಲ್ಯ[0] == '\'')
    ಹಿಂತಿರುಗಿ get_Single_Quoted_ಮೌಲ್ಯ(ಮೌಲ್ಯ, Storage);
  ಹಿಂತಿರುಗಿ get_Plain_ಮೌಲ್ಯ(ಮೌಲ್ಯ, Storage);
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
parse_Scalar_ಮೌಲ್ಯ(StringRef Unquoted_ಮೌಲ್ಯ, Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_ಮೌಲ್ಯ.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(LookupChars);
  ಆದರೆ (I == StringRef::npos)
    ಹಿಂತಿರುಗಿ Unquoted_ಮೌಲ್ಯ;

  Storage.clear();
  Storage.reserve(Unquoted_ಮೌಲ್ಯ.ಗಾತ್ರ_3());
  ಅಕ್ಷರ ಕೊನೆಯ_New_ಸಾಲು_Added_As = '\0';
  ಗಾಗಿ (; I != StringRef::npos; I = Unquoted_ಮೌಲ್ಯ.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(LookupChars)) {
    ಆದರೆ (Unquoted_ಮೌಲ್ಯ[I] != '\r' && Unquoted_ಮೌಲ್ಯ[I] != '\n') {
      llvm::append_range(Storage, Unquoted_ಮೌಲ್ಯ.take_front(I));
      Unquoted_ಮೌಲ್ಯ = UnescapeCallback(Unquoted_ಮೌಲ್ಯ.drop_front(I), Storage);
      ಕೊನೆಯ_New_ಸಾಲು_Added_As = '\0';
      ಮುಂದುವರಿ;
    }
    ಆದರೆ (size_t ಕೊನೆಯ_Non_SWhite = Unquoted_ಮೌಲ್ಯ.ಕಂಡುಹಿಡಿ_ಕೊನೆಯ_not_ದ(" \t", I);
        ಕೊನೆಯ_Non_SWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_ಮೌಲ್ಯ.take_front(ಕೊನೆಯ_Non_SWhite + 1));
      Storage.ತಳ್ಳು_ಹಿಂಭಾಗ(' ');
      ಕೊನೆಯ_New_ಸಾಲು_Added_As = ' ';
    } ಇಲ್ಲದಿದ್ದರೆ {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      ಆಯ್ಕೆ (ಕೊನೆಯ_New_ಸಾಲು_Added_As) {
      ಸ್ಥಿತಿ ' ':
        ಪ್ರತಿಜ್ಞಾ(!Storage.ಖಾಲಿಗಳು() && Storage.ಹಿಂಭಾಗ() == ' ');
        Storage.ಹಿಂಭಾಗ() = '\n';
        ಕೊನೆಯ_New_ಸಾಲು_Added_As = '\n';
        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ '\n':
        ಪ್ರತಿಜ್ಞಾ(!Storage.ಖಾಲಿಗಳು() && Storage.ಹಿಂಭಾಗ() == '\n');
        Storage.ತಳ್ಳು_ಹಿಂಭಾಗ('\n');
        ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ:
        Storage.ತಳ್ಳು_ಹಿಂಭಾಗ(' ');
        ಕೊನೆಯ_New_ಸಾಲು_Added_As = ' ';
        ನಿಲ್ಲಿಸು;
      }
    }
    // Handle Windows-style EOL
    ಆದರೆ (Unquoted_ಮೌಲ್ಯ.substr(I, 2) == "\r\n")
      I++;
    Unquoted_ಮೌಲ್ಯ = Unquoted_ಮೌಲ್ಯ.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_ಮೌಲ್ಯ);
  ಹಿಂತಿರುಗಿ StringRef(Storage.begin(), Storage.ಗಾತ್ರ_3());
}

StringRef
Scalar_ಗಂಟು::get_Double_Quoted_ಮೌಲ್ಯ(StringRef Raw_ಮೌಲ್ಯ,
                                 Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Storage) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(Raw_ಮೌಲ್ಯ.ಗಾತ್ರ_3() >= 2 && Raw_ಮೌಲ್ಯ.front() == '"' &&
         Raw_ಮೌಲ್ಯ.ಹಿಂಭಾಗ() == '"');
  StringRef Unquoted_ಮೌಲ್ಯ = Raw_ಮೌಲ್ಯ.substr(1, Raw_ಮೌಲ್ಯ.ಗಾತ್ರ_3() - 2);

  ಸ್ವಯಂಚಾಲಿತ UnescapeFunc = [ಇದು](StringRef Unquoted_ಮೌಲ್ಯ,
                             Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Storage) {
    ಪ್ರತಿಜ್ಞಾ(Unquoted_ಮೌಲ್ಯ.take_front(1) == "\\");
    ಆದರೆ (Unquoted_ಮೌಲ್ಯ.ಗಾತ್ರ_3() == 1) {
      Token T;
      T.Range = Unquoted_ಮೌಲ್ಯ;
      ಗಣ_Error("Unrecognized escape code", T);
      Storage.clear();
      ಹಿಂತಿರುಗಿ StringRef();
    }
    Unquoted_ಮೌಲ್ಯ = Unquoted_ಮೌಲ್ಯ.drop_front(1);
    ಆಯ್ಕೆ (Unquoted_ಮೌಲ್ಯ[0]) {
    ಪೂರ್ವನಿಯೋಜಿತ: {
      Token T;
      T.Range = Unquoted_ಮೌಲ್ಯ.take_front(1);
      ಗಣ_Error("Unrecognized escape code", T);
      Storage.clear();
      ಹಿಂತಿರುಗಿ StringRef();
    }
    ಸ್ಥಿತಿ '\r':
      // Shrink the Windows-style EOL.
      ಆದರೆ (Unquoted_ಮೌಲ್ಯ.ಗಾತ್ರ_3() >= 2 && Unquoted_ಮೌಲ್ಯ[1] == '\n')
        Unquoted_ಮೌಲ್ಯ = Unquoted_ಮೌಲ್ಯ.drop_front(1);
      [[fallthrough]];
    ಸ್ಥಿತಿ '\n':
      ಹಿಂತಿರುಗಿ Unquoted_ಮೌಲ್ಯ.drop_front(1).ltrim(" \t");
    ಸ್ಥಿತಿ '0':
      Storage.ತಳ್ಳು_ಹಿಂಭಾಗ(0x00);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'a':
      Storage.ತಳ್ಳು_ಹಿಂಭಾಗ(0x07);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'b':
      Storage.ತಳ್ಳು_ಹಿಂಭಾಗ(0x08);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 't':
    ಸ್ಥಿತಿ 0x09:
      Storage.ತಳ್ಳು_ಹಿಂಭಾಗ(0x09);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'n':
      Storage.ತಳ್ಳು_ಹಿಂಭಾಗ(0x0A);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'v':
      Storage.ತಳ್ಳು_ಹಿಂಭಾಗ(0x0B);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'f':
      Storage.ತಳ್ಳು_ಹಿಂಭಾಗ(0x0C);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'r':
      Storage.ತಳ್ಳು_ಹಿಂಭಾಗ(0x0D);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'e':
      Storage.ತಳ್ಳು_ಹಿಂಭಾಗ(0x1B);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ ' ':
      Storage.ತಳ್ಳು_ಹಿಂಭಾಗ(0x20);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '"':
      Storage.ತಳ್ಳು_ಹಿಂಭಾಗ(0x22);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '/':
      Storage.ತಳ್ಳು_ಹಿಂಭಾಗ(0x2F);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '\\':
      Storage.ತಳ್ಳು_ಹಿಂಭಾಗ(0x5C);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'N':
      encodeUTF8(0x85, Storage);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '_':
      encodeUTF8(0xA0, Storage);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'L':
      encodeUTF8(0x2028, Storage);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'P':
      encodeUTF8(0x2029, Storage);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 'x': {
      ಆದರೆ (Unquoted_ಮೌಲ್ಯ.ಗಾತ್ರ_3() < 3)
        // TODO: Report error.
        ನಿಲ್ಲಿಸು;
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ Unicode_Scalar_ಮೌಲ್ಯ;
      ಆದರೆ (Unquoted_ಮೌಲ್ಯ.substr(1, 2).getAsInteger(16, Unicode_Scalar_ಮೌಲ್ಯ))
        // TODO: Report error.
        Unicode_Scalar_ಮೌಲ್ಯ = 0xFFFD;
      encodeUTF8(Unicode_Scalar_ಮೌಲ್ಯ, Storage);
      ಹಿಂತಿರುಗಿ Unquoted_ಮೌಲ್ಯ.drop_front(3);
    }
    ಸ್ಥಿತಿ 'u': {
      ಆದರೆ (Unquoted_ಮೌಲ್ಯ.ಗಾತ್ರ_3() < 5)
        // TODO: Report error.
        ನಿಲ್ಲಿಸು;
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ Unicode_Scalar_ಮೌಲ್ಯ;
      ಆದರೆ (Unquoted_ಮೌಲ್ಯ.substr(1, 4).getAsInteger(16, Unicode_Scalar_ಮೌಲ್ಯ))
        // TODO: Report error.
        Unicode_Scalar_ಮೌಲ್ಯ = 0xFFFD;
      encodeUTF8(Unicode_Scalar_ಮೌಲ್ಯ, Storage);
      ಹಿಂತಿರುಗಿ Unquoted_ಮೌಲ್ಯ.drop_front(5);
    }
    ಸ್ಥಿತಿ 'U': {
      ಆದರೆ (Unquoted_ಮೌಲ್ಯ.ಗಾತ್ರ_3() < 9)
        // TODO: Report error.
        ನಿಲ್ಲಿಸು;
      ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ Unicode_Scalar_ಮೌಲ್ಯ;
      ಆದರೆ (Unquoted_ಮೌಲ್ಯ.substr(1, 8).getAsInteger(16, Unicode_Scalar_ಮೌಲ್ಯ))
        // TODO: Report error.
        Unicode_Scalar_ಮೌಲ್ಯ = 0xFFFD;
      encodeUTF8(Unicode_Scalar_ಮೌಲ್ಯ, Storage);
      ಹಿಂತಿರುಗಿ Unquoted_ಮೌಲ್ಯ.drop_front(9);
    }
    }
    ಹಿಂತಿರುಗಿ Unquoted_ಮೌಲ್ಯ.drop_front(1);
  };

  ಹಿಂತಿರುಗಿ parse_Scalar_ಮೌಲ್ಯ(Unquoted_ಮೌಲ್ಯ, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_ಗಂಟು::get_Single_Quoted_ಮೌಲ್ಯ(StringRef Raw_ಮೌಲ್ಯ,
                                           Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Storage) {
  ಪ್ರತಿಜ್ಞಾ(Raw_ಮೌಲ್ಯ.ಗಾತ್ರ_3() >= 2 && Raw_ಮೌಲ್ಯ.front() == '\'' &&
         Raw_ಮೌಲ್ಯ.ಹಿಂಭಾಗ() == '\'');
  StringRef Unquoted_ಮೌಲ್ಯ = Raw_ಮೌಲ್ಯ.substr(1, Raw_ಮೌಲ್ಯ.ಗಾತ್ರ_3() - 2);

  ಸ್ವಯಂಚಾಲಿತ UnescapeFunc = [](StringRef Unquoted_ಮೌಲ್ಯ,
                         Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Storage) {
    ಪ್ರತಿಜ್ಞಾ(Unquoted_ಮೌಲ್ಯ.take_front(2) == "''");
    Storage.ತಳ್ಳು_ಹಿಂಭಾಗ('\'');
    ಹಿಂತಿರುಗಿ Unquoted_ಮೌಲ್ಯ.drop_front(2);
  };

  ಹಿಂತಿರುಗಿ parse_Scalar_ಮೌಲ್ಯ(Unquoted_ಮೌಲ್ಯ, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_ಗಂಟು::get_Plain_ಮೌಲ್ಯ(StringRef Raw_ಮೌಲ್ಯ,
                                    Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_ಮೌಲ್ಯ = Raw_ಮೌಲ್ಯ.rtrim("\r\n \t");
  ಹಿಂತಿರುಗಿ parse_Scalar_ಮೌಲ್ಯ(Raw_ಮೌಲ್ಯ, Storage, "\r\n", ಶೂನ್ಯಸೂಚಕ);
}

ಗಂಟು *ಕೀಲಿ_ಮೌಲ್ಯ_ಗಂಟು::get_ಕೀಲಿ() {
  ಆದರೆ (ಕೀಲಿ)
    ಹಿಂತಿರುಗಿ ಕೀಲಿ;
  // Handle implicit null keys.
  {
    Token &t = peek_ಮುಂದಿನ();
    ಆದರೆ (   t.ಬಗೆ_2 == Token::TK_ಖಂಡ_ಕೊನೆ
        || t.ಬಗೆ_2 == Token::TK_ಮೌಲ್ಯ
        || t.ಬಗೆ_2 == Token::TK_Error) {
      ಹಿಂತಿರುಗಿ ಕೀಲಿ = ಹೊಸ (getAllocator()) Null_ಗಂಟು(Doc);
    }
    ಆದರೆ (t.ಬಗೆ_2 == Token::TK_ಕೀಲಿ)
      get_ಮುಂದಿನ(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peek_ಮುಂದಿನ();
  ಆದರೆ (t.ಬಗೆ_2 == Token::TK_ಖಂಡ_ಕೊನೆ || t.ಬಗೆ_2 == Token::TK_ಮೌಲ್ಯ) {
    ಹಿಂತಿರುಗಿ ಕೀಲಿ = ಹೊಸ (getAllocator()) Null_ಗಂಟು(Doc);
  }

  // We've got a normal key.
  ಹಿಂತಿರುಗಿ ಕೀಲಿ = parse_ಖಂಡ_ಗಂಟು();
}

ಗಂಟು *ಕೀಲಿ_ಮೌಲ್ಯ_ಗಂಟು::get_ಮೌಲ್ಯ() {
  ಆದರೆ (ಮೌಲ್ಯ)
    ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ;

  ಆದರೆ (ಗಂಟು* ಕೀಲಿ = get_ಕೀಲಿ())
    ಕೀಲಿ->skip();
  ಇಲ್ಲದಿದ್ದರೆ {
    ಗಣ_Error("Null key in Key Value.", peek_ಮುಂದಿನ());
    ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ = ಹೊಸ (getAllocator()) Null_ಗಂಟು(Doc);
  }

  ಆದರೆ (failed())
    ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ = ಹೊಸ (getAllocator()) Null_ಗಂಟು(Doc);

  // Handle implicit null values.
  {
    Token &t = peek_ಮುಂದಿನ();
    ಆದರೆ (   t.ಬಗೆ_2 == Token::TK_ಖಂಡ_ಕೊನೆ
        || t.ಬಗೆ_2 == Token::TK_Flow_Mapping_ಕೊನೆ
        || t.ಬಗೆ_2 == Token::TK_ಕೀಲಿ
        || t.ಬಗೆ_2 == Token::TK_Flow_ನಮೂದು
        || t.ಬಗೆ_2 == Token::TK_Error) {
      ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ = ಹೊಸ (getAllocator()) Null_ಗಂಟು(Doc);
    }

    ಆದರೆ (t.ಬಗೆ_2 != Token::TK_ಮೌಲ್ಯ) {
      ಗಣ_Error("Unexpected token in Key Value.", t);
      ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ = ಹೊಸ (getAllocator()) Null_ಗಂಟು(Doc);
    }
    get_ಮುಂದಿನ(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peek_ಮುಂದಿನ();
  ಆದರೆ (t.ಬಗೆ_2 == Token::TK_ಖಂಡ_ಕೊನೆ || t.ಬಗೆ_2 == Token::TK_ಕೀಲಿ) {
    ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ = ಹೊಸ (getAllocator()) Null_ಗಂಟು(Doc);
  }

  // We got a normal value.
  ಹಿಂತಿರುಗಿ ಮೌಲ್ಯ = parse_ಖಂಡ_ಗಂಟು();
}

ಶೂನ್ಯ Mapping_ಗಂಟು::increment() {
  ಆದರೆ (failed()) {
    Is_At_ಕೊನೆ = ನಿಜ;
    Current_ನಮೂದು = ಶೂನ್ಯಸೂಚಕ;
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ (Current_ನಮೂದು) {
    Current_ನಮೂದು->skip();
    ಆದರೆ (Type == MT_Inline) {
      Is_At_ಕೊನೆ = ನಿಜ;
      Current_ನಮೂದು = ಶೂನ್ಯಸೂಚಕ;
      ಹಿಂತಿರುಗಿ;
    }
  }
  Token T = peek_ಮುಂದಿನ();
  ಆದರೆ (T.ಬಗೆ_2 == Token::TK_ಕೀಲಿ || T.ಬಗೆ_2 == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    Current_ನಮೂದು = ಹೊಸ (getAllocator()) ಕೀಲಿ_ಮೌಲ್ಯ_ಗಂಟು(Doc);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Type == MT_ಖಂಡ) {
    ಆಯ್ಕೆ (T.ಬಗೆ_2) {
    ಸ್ಥಿತಿ Token::TK_ಖಂಡ_ಕೊನೆ:
      get_ಮುಂದಿನ();
      Is_At_ಕೊನೆ = ನಿಜ;
      Current_ನಮೂದು = ಶೂನ್ಯಸೂಚಕ;
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಗಣ_Error("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    ಸ್ಥಿತಿ Token::TK_Error:
      Is_At_ಕೊನೆ = ನಿಜ;
      Current_ನಮೂದು = ಶೂನ್ಯಸೂಚಕ;
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಆಯ್ಕೆ (T.ಬಗೆ_2) {
    ಸ್ಥಿತಿ Token::TK_Flow_ನಮೂದು:
      // Eat the flow entry and recurse.
      get_ಮುಂದಿನ();
      ಹಿಂತಿರುಗಿ increment();
    ಸ್ಥಿತಿ Token::TK_Flow_Mapping_ಕೊನೆ:
      get_ಮುಂದಿನ();
      [[fallthrough]];
    ಸ್ಥಿತಿ Token::TK_Error:
      // Set this to end iterator.
      Is_At_ಕೊನೆ = ನಿಜ;
      Current_ನಮೂದು = ಶೂನ್ಯಸೂಚಕ;
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಗಣ_Error( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      Is_At_ಕೊನೆ = ನಿಜ;
      Current_ನಮೂದು = ಶೂನ್ಯಸೂಚಕ;
    }
  }
}

ಶೂನ್ಯ Sequence_ಗಂಟು::increment() {
  ಆದರೆ (failed()) {
    Is_At_ಕೊನೆ = ನಿಜ;
    Current_ನಮೂದು = ಶೂನ್ಯಸೂಚಕ;
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ (Current_ನಮೂದು)
    Current_ನಮೂದು->skip();
  Token T = peek_ಮುಂದಿನ();
  ಆದರೆ (SeqType == ST_ಖಂಡ) {
    ಆಯ್ಕೆ (T.ಬಗೆ_2) {
    ಸ್ಥಿತಿ Token::TK_ಖಂಡ_ನಮೂದು:
      get_ಮುಂದಿನ();
      Current_ನಮೂದು = parse_ಖಂಡ_ಗಂಟು();
      ಆದರೆ (!Current_ನಮೂದು) { // An error occurred.
        Is_At_ಕೊನೆ = ನಿಜ;
        Current_ನಮೂದು = ಶೂನ್ಯಸೂಚಕ;
      }
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_ಖಂಡ_ಕೊನೆ:
      get_ಮುಂದಿನ();
      Is_At_ಕೊನೆ = ನಿಜ;
      Current_ನಮೂದು = ಶೂನ್ಯಸೂಚಕ;
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಗಣ_Error( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    ಸ್ಥಿತಿ Token::TK_Error:
      Is_At_ಕೊನೆ = ನಿಜ;
      Current_ನಮೂದು = ಶೂನ್ಯಸೂಚಕ;
    }
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (SeqType == ST_Indentless) {
    ಆಯ್ಕೆ (T.ಬಗೆ_2) {
    ಸ್ಥಿತಿ Token::TK_ಖಂಡ_ನಮೂದು:
      get_ಮುಂದಿನ();
      Current_ನಮೂದು = parse_ಖಂಡ_ಗಂಟು();
      ಆದರೆ (!Current_ನಮೂದು) { // An error occurred.
        Is_At_ಕೊನೆ = ನಿಜ;
        Current_ನಮೂದು = ಶೂನ್ಯಸೂಚಕ;
      }
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
    ಸ್ಥಿತಿ Token::TK_Error:
      Is_At_ಕೊನೆ = ನಿಜ;
      Current_ನಮೂದು = ಶೂನ್ಯಸೂಚಕ;
    }
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (SeqType == ST_Flow) {
    ಆಯ್ಕೆ (T.ಬಗೆ_2) {
    ಸ್ಥಿತಿ Token::TK_Flow_ನಮೂದು:
      // Eat the flow entry and recurse.
      get_ಮುಂದಿನ();
      Was_ಹಿಂದಿನ_Token_Flow_ನಮೂದು = ನಿಜ;
      ಹಿಂತಿರುಗಿ increment();
    ಸ್ಥಿತಿ Token::TK_Flow_Sequence_ಕೊನೆ:
      get_ಮುಂದಿನ();
      [[fallthrough]];
    ಸ್ಥಿತಿ Token::TK_Error:
      // Set this to end iterator.
      Is_At_ಕೊನೆ = ನಿಜ;
      Current_ನಮೂದು = ಶೂನ್ಯಸೂಚಕ;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ Token::TK_ಹರಿವು_ಕೊನೆ:
    ಸ್ಥಿತಿ Token::TK_Document_ಕೊನೆ:
    ಸ್ಥಿತಿ Token::TK_Document_ಆರಂಭ:
      ಗಣ_Error("Could not find closing ]!", T);
      // Set this to end iterator.
      Is_At_ಕೊನೆ = ನಿಜ;
      Current_ನಮೂದು = ಶೂನ್ಯಸೂಚಕ;
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಆದರೆ (!Was_ಹಿಂದಿನ_Token_Flow_ನಮೂದು) {
        ಗಣ_Error("Expected , between entries!", T);
        Is_At_ಕೊನೆ = ನಿಜ;
        Current_ನಮೂದು = ಶೂನ್ಯಸೂಚಕ;
        ನಿಲ್ಲಿಸು;
      }
      // Otherwise it must be a flow entry.
      Current_ನಮೂದು = parse_ಖಂಡ_ಗಂಟು();
      ಆದರೆ (!Current_ನಮೂದು) {
        Is_At_ಕೊನೆ = ನಿಜ;
      }
      Was_ಹಿಂದಿನ_Token_Flow_ನಮೂದು = ಸುಳ್ಳು;
      ನಿಲ್ಲಿಸು;
    }
  }
}

Document::Document(ಹರಿವು &S) : ಹರಿವು_2(S), ಬೇರು_2(ಶೂನ್ಯಸೂಚಕ) {
  // Tag maps starts with two default mappings.
  ಟ್ಯಾಗ್_Map["!"] = "!";
  ಟ್ಯಾಗ್_Map["!!"] = "tag:yaml.org,2002:";

  ಆದರೆ (parseDirectives())
    expectToken(Token::TK_Document_ಆರಂಭ);
  Token &T = peek_ಮುಂದಿನ();
  ಆದರೆ (T.ಬಗೆ_2 == Token::TK_Document_ಆರಂಭ)
    get_ಮುಂದಿನ();
}

ತಾರ್ಕಿಕ Document::skip()  {
  ಆದರೆ (ಹರಿವು_2.scanner->failed())
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (!ಬೇರು_2 && !get_ಬೇರು())
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಬೇರು_2->skip();
  Token &T = peek_ಮುಂದಿನ();
  ಆದರೆ (T.ಬಗೆ_2 == Token::TK_ಹರಿವು_ಕೊನೆ)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಆದರೆ (T.ಬಗೆ_2 == Token::TK_Document_ಕೊನೆ) {
    get_ಮುಂದಿನ();
    ಹಿಂತಿರುಗಿ skip();
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}

Token &Document::peek_ಮುಂದಿನ() {
  ಹಿಂತಿರುಗಿ ಹರಿವು_2.scanner->peek_ಮುಂದಿನ();
}

Token Document::get_ಮುಂದಿನ() {
  ಹಿಂತಿರುಗಿ ಹರಿವು_2.scanner->get_ಮುಂದಿನ();
}

ಶೂನ್ಯ Document::ಗಣ_Error(ಸ್ಥಿರಾಂಕ Twine &ಸಂದೇಶ_3, Token &Location) ಸ್ಥಿರಾಂಕ {
  ಹರಿವು_2.scanner->ಗಣ_Error(ಸಂದೇಶ_3, Location.Range.begin());
}

ತಾರ್ಕಿಕ Document::failed() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ ಹರಿವು_2.scanner->failed();
}

ಗಂಟು *Document::parse_ಖಂಡ_ಗಂಟು() {
  Token T = peek_ಮುಂದಿನ();
  // Handle properties.
  Token AnchorInfo;
  Token ಟ್ಯಾಗ್_Info;
parse_property:
  ಆಯ್ಕೆ (T.ಬಗೆ_2) {
  ಸ್ಥಿತಿ Token::TK_Alias:
    get_ಮುಂದಿನ();
    ಹಿಂತಿರುಗಿ ಹೊಸ (ಗಂಟು_Allocator) Alias_ಗಂಟು(ಹರಿವು_2.CurrentDoc, T.Range.substr(1));
  ಸ್ಥಿತಿ Token::TK_Anchor:
    ಆದರೆ (AnchorInfo.ಬಗೆ_2 == Token::TK_Anchor) {
      ಗಣ_Error("Already encountered an anchor for this node!", T);
      ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
    }
    AnchorInfo = get_ಮುಂದಿನ(); // Consume TK_Anchor.
    T = peek_ಮುಂದಿನ();
    ಹೋಗು parse_property;
  ಸ್ಥಿತಿ Token::TK_ಟ್ಯಾಗ್:
    ಆದರೆ (ಟ್ಯಾಗ್_Info.ಬಗೆ_2 == Token::TK_ಟ್ಯಾಗ್) {
      ಗಣ_Error("Already encountered a tag for this node!", T);
      ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
    }
    ಟ್ಯಾಗ್_Info = get_ಮುಂದಿನ(); // Consume TK_Tag.
    T = peek_ಮುಂದಿನ();
    ಹೋಗು parse_property;
  ಪೂರ್ವನಿಯೋಜಿತ:
    ನಿಲ್ಲಿಸು;
  }

  ಆಯ್ಕೆ (T.ಬಗೆ_2) {
  ಸ್ಥಿತಿ Token::TK_ಖಂಡ_ನಮೂದು:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    ಹಿಂತಿರುಗಿ ಹೊಸ (ಗಂಟು_Allocator) Sequence_ಗಂಟು( ಹರಿವು_2.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , ಟ್ಯಾಗ್_Info.Range
                                           , Sequence_ಗಂಟು::ST_Indentless);
  ಸ್ಥಿತಿ Token::TK_ಖಂಡ_Sequence_ಆರಂಭ:
    get_ಮುಂದಿನ();
    ಹಿಂತಿರುಗಿ ಹೊಸ (ಗಂಟು_Allocator)
      Sequence_ಗಂಟು( ಹರಿವು_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , ಟ್ಯಾಗ್_Info.Range
                  , Sequence_ಗಂಟು::ST_ಖಂಡ);
  ಸ್ಥಿತಿ Token::TK_ಖಂಡ_Mapping_ಆರಂಭ:
    get_ಮುಂದಿನ();
    ಹಿಂತಿರುಗಿ ಹೊಸ (ಗಂಟು_Allocator)
      Mapping_ಗಂಟು( ಹರಿವು_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , ಟ್ಯಾಗ್_Info.Range
                 , Mapping_ಗಂಟು::MT_ಖಂಡ);
  ಸ್ಥಿತಿ Token::TK_Flow_Sequence_ಆರಂಭ:
    get_ಮುಂದಿನ();
    ಹಿಂತಿರುಗಿ ಹೊಸ (ಗಂಟು_Allocator)
      Sequence_ಗಂಟು( ಹರಿವು_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , ಟ್ಯಾಗ್_Info.Range
                  , Sequence_ಗಂಟು::ST_Flow);
  ಸ್ಥಿತಿ Token::TK_Flow_Mapping_ಆರಂಭ:
    get_ಮುಂದಿನ();
    ಹಿಂತಿರುಗಿ ಹೊಸ (ಗಂಟು_Allocator)
      Mapping_ಗಂಟು( ಹರಿವು_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , ಟ್ಯಾಗ್_Info.Range
                 , Mapping_ಗಂಟು::MT_Flow);
  ಸ್ಥಿತಿ Token::TK_Scalar:
    get_ಮುಂದಿನ();
    ಹಿಂತಿರುಗಿ ಹೊಸ (ಗಂಟು_Allocator)
      Scalar_ಗಂಟು( ಹರಿವು_2.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , ಟ್ಯಾಗ್_Info.Range
                , T.Range);
  ಸ್ಥಿತಿ Token::TK_ಖಂಡ_Scalar: {
    get_ಮುಂದಿನ();
    StringRef NullTerminatedStr(T.ಮೌಲ್ಯ.c_str(), T.ಮೌಲ್ಯ.ಉದ್ದಳತೆ_2() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(ಗಂಟು_Allocator).drop_ಹಿಂಭಾಗ();
    ಹಿಂತಿರುಗಿ ಹೊಸ (ಗಂಟು_Allocator)
        ಖಂಡ_Scalar_ಗಂಟು(ಹರಿವು_2.CurrentDoc, AnchorInfo.Range.substr(1),
                        ಟ್ಯಾಗ್_Info.Range, StrCopy, T.Range);
  }
  ಸ್ಥಿತಿ Token::TK_ಕೀಲಿ:
    // Don't eat the TK_Key, KeyValueNode expects it.
    ಹಿಂತಿರುಗಿ ಹೊಸ (ಗಂಟು_Allocator)
      Mapping_ಗಂಟು( ಹರಿವು_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , ಟ್ಯಾಗ್_Info.Range
                 , Mapping_ಗಂಟು::MT_Inline);
  ಸ್ಥಿತಿ Token::TK_Document_ಆರಂಭ:
  ಸ್ಥಿತಿ Token::TK_Document_ಕೊನೆ:
  ಸ್ಥಿತಿ Token::TK_ಹರಿವು_ಕೊನೆ:
  ಪೂರ್ವನಿಯೋಜಿತ:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    ಹಿಂತಿರುಗಿ ಹೊಸ (ಗಂಟು_Allocator) Null_ಗಂಟು(ಹರಿವು_2.CurrentDoc);
  ಸ್ಥಿತಿ Token::TK_Flow_Mapping_ಕೊನೆ:
  ಸ್ಥಿತಿ Token::TK_Flow_Sequence_ಕೊನೆ:
  ಸ್ಥಿತಿ Token::TK_Flow_ನಮೂದು: {
    ಆದರೆ (ಬೇರು_2 && (isa<Mapping_ಗಂಟು>(ಬೇರು_2) || isa<Sequence_ಗಂಟು>(ಬೇರು_2)))
      ಹಿಂತಿರುಗಿ ಹೊಸ (ಗಂಟು_Allocator) Null_ಗಂಟು(ಹರಿವು_2.CurrentDoc);

    ಗಣ_Error("Unexpected token", T);
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  }
  ಸ್ಥಿತಿ Token::TK_Error:
    ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
}

ತಾರ್ಕಿಕ Document::parseDirectives() {
  ತಾರ್ಕಿಕ isDirective = ಸುಳ್ಳು;
  ಆಗ (ನಿಜ) {
    Token T = peek_ಮುಂದಿನ();
    ಆದರೆ (T.ಬಗೆ_2 == Token::TK_ಟ್ಯಾಗ್_Directive) {
      parseTAGDirective();
      isDirective = ನಿಜ;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (T.ಬಗೆ_2 == Token::TK_ಆವೃತ್ತಿ_Directive) {
      parseYAMLDirective();
      isDirective = ನಿಜ;
    } ಇಲ್ಲದಿದ್ದರೆ {
      ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ isDirective;
}

ಶೂನ್ಯ Document::parseYAMLDirective() {
  get_ಮುಂದಿನ(); // Eat %YAML <version>
}

ಶೂನ್ಯ Document::parseTAGDirective() {
  Token ಟ್ಯಾಗ್ = get_ಮುಂದಿನ(); // %TAG <handle> <prefix>
  StringRef T = ಟ್ಯಾಗ್.Range;
  // Strip %TAG
  T = T.substr(T.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(" \t")).ltrim(" \t");
  std::size_t Handle_ಕೊನೆ = T.ಕಂಡುಹಿಡಿ_ಮೊದಲ_ದ(" \t");
  StringRef ಟ್ಯಾಗ್_Handle = T.substr(0, Handle_ಕೊನೆ);
  StringRef ಟ್ಯಾಗ್_ಪೂರ್ವಪ್ರತ್ಯಯ = T.substr(Handle_ಕೊನೆ).ltrim(" \t");
  ಟ್ಯಾಗ್_Map[ಟ್ಯಾಗ್_Handle] = ಟ್ಯಾಗ್_ಪೂರ್ವಪ್ರತ್ಯಯ;
}

ತಾರ್ಕಿಕ Document::expectToken(ಪೂರ್ಣಾಂಕ TK) {
  Token T = get_ಮುಂದಿನ();
  ಆದರೆ (T.ಬಗೆ_2 != TK) {
    ಗಣ_Error("Unexpected token", T);
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  }
  ಹಿಂತಿರುಗಿ ನಿಜ;
}
