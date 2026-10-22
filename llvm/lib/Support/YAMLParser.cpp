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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/YAMLParser.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/AllocatorList.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ArrayRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/STLExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallVector.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Twine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Compiler.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SMLoc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SourceMgr.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Unicode.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstddef>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdint>
#ਅਨ੍ਤਰ੍ਭਾਵ <map>
#ਅਨ੍ਤਰ੍ਭਾਵ <memory>
#ਅਨ੍ਤਰ੍ਭਾਵ <string>
#ਅਨ੍ਤਰ੍ਭਾਵ <system_error>
#ਅਨ੍ਤਰ੍ਭਾਵ <utility>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ yaml;

ਗਿਣਤੀ UnicodeEncodingForm {
  UEF_UTF32_LE, ///< UTF-32 Little Endian
  UEF_UTF32_BE, ///< UTF-32 Big Endian
  UEF_UTF16_LE, ///< UTF-16 Little Endian
  UEF_UTF16_BE, ///< UTF-16 Big Endian
  UEF_UTF8,     ///< UTF-8 or ascii.
  UEF_Unknown   ///< Not a valid Unicode encoding.
};

/// EncodingInfo - Holds the encoding type and length of the byte order mark if
///                it exists. Length is in {0, 2, 3, 4}.
ਵਰਤੋਂ EncodingInfo = std::pair<UnicodeEncodingForm, ਬਿਨਾਨਿਸ਼ਾਨੀ>;

/// getUnicodeEncoding - Reads up to the first 4 bytes to determine the Unicode
///                      encoding form of \a Input.
///
/// @param Input A string of length 0 or more.
/// @returns An EncodingInfo indicating the Unicode encoding form of the input
///          and how long the byte order mark is if one exists.
ਸਥਿਰ EncodingInfo getUnicodeEncoding(StringRef ਨਿਵੇਸ਼) {
  ਜੇ (ਨਿਵੇਸ਼.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ {UEF_Unknown, 0};

  ਚੋਣ (uint8_t(ਨਿਵੇਸ਼[0])) {
  ਹਾਲਤ 0x00:
    ਜੇ (ਨਿਵੇਸ਼.ਮਾਪ_2() >= 4) {
      ਜੇ (  ਨਿਵੇਸ਼[1] == 0
         && uint8_t(ਨਿਵੇਸ਼[2]) == 0xFE
         && uint8_t(ਨਿਵੇਸ਼[3]) == 0xFF)
        ਵਾਪਸ {UEF_UTF32_BE, 4};
      ਜੇ (ਨਿਵੇਸ਼[1] == 0 && ਨਿਵੇਸ਼[2] == 0 && ਨਿਵੇਸ਼[3] != 0)
        ਵਾਪਸ {UEF_UTF32_BE, 0};
    }

    ਜੇ (ਨਿਵੇਸ਼.ਮਾਪ_2() >= 2 && ਨਿਵੇਸ਼[1] != 0)
      ਵਾਪਸ {UEF_UTF16_BE, 0};
    ਵਾਪਸ {UEF_Unknown, 0};
  ਹਾਲਤ 0xFF:
    ਜੇ (  ਨਿਵੇਸ਼.ਮਾਪ_2() >= 4
       && uint8_t(ਨਿਵੇਸ਼[1]) == 0xFE
       && ਨਿਵੇਸ਼[2] == 0
       && ਨਿਵੇਸ਼[3] == 0)
      ਵਾਪਸ {UEF_UTF32_LE, 4};

    ਜੇ (ਨਿਵੇਸ਼.ਮਾਪ_2() >= 2 && uint8_t(ਨਿਵੇਸ਼[1]) == 0xFE)
      ਵਾਪਸ {UEF_UTF16_LE, 2};
    ਵਾਪਸ {UEF_Unknown, 0};
  ਹਾਲਤ 0xFE:
    ਜੇ (ਨਿਵੇਸ਼.ਮਾਪ_2() >= 2 && uint8_t(ਨਿਵੇਸ਼[1]) == 0xFF)
      ਵਾਪਸ {UEF_UTF16_BE, 2};
    ਵਾਪਸ {UEF_Unknown, 0};
  ਹਾਲਤ 0xEF:
    ਜੇ (  ਨਿਵੇਸ਼.ਮਾਪ_2() >= 3
       && uint8_t(ਨਿਵੇਸ਼[1]) == 0xBB
       && uint8_t(ਨਿਵੇਸ਼[2]) == 0xBF)
      ਵਾਪਸ {UEF_UTF8, 3};
    ਵਾਪਸ {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  ਜੇ (ਨਿਵੇਸ਼.ਮਾਪ_2() >= 4 && ਨਿਵੇਸ਼[1] == 0 && ਨਿਵੇਸ਼[2] == 0 && ਨਿਵੇਸ਼[3] == 0)
    ਵਾਪਸ {UEF_UTF32_LE, 0};

  ਜੇ (ਨਿਵੇਸ਼.ਮਾਪ_2() >= 2 && ਨਿਵੇਸ਼[1] == 0)
    ਵਾਪਸ {UEF_UTF16_LE, 0};

  ਵਾਪਸ {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
ਖਾਲੀ ਗੰਢ::anchor() {}
ਖਾਲੀ Null_ਗੰਢ::anchor() {}
ਖਾਲੀ Scalar_ਗੰਢ::anchor() {}
ਖਾਲੀ ਖੰਡ_Scalar_ਗੰਢ::anchor() {}
ਖਾਲੀ ਕੁੰਜੀ_ਮੁੱਲ_ਗੰਢ::anchor() {}
ਖਾਲੀ Mapping_ਗੰਢ::anchor() {}
ਖਾਲੀ Sequence_ਗੰਢ::anchor() {}
ਖਾਲੀ Alias_ਗੰਢ::anchor() {}

ਨਾਮਾਕਾਸ਼ llvm {
ਨਾਮਾਕਾਸ਼ yaml {

/// Token - A single YAML token.
ਬਣਤਰ Token {
  ਗਿਣਤੀ Token_ਕਿਸਮ {
    TK_Error, // Uninitialized token.
    TK_ਧਾਰਾ_ਸ਼ੁਰੂ,
    TK_ਧਾਰਾ_ਸਿਰਾ,
    TK_ਸੰਸਕਰਣ_Directive,
    TK_ਟੈਗ_Directive,
    TK_Document_ਸ਼ੁਰੂ,
    TK_Document_ਸਿਰਾ,
    TK_ਖੰਡ_ਇੰਦਰਾਜ,
    TK_ਖੰਡ_ਸਿਰਾ,
    TK_ਖੰਡ_Sequence_ਸ਼ੁਰੂ,
    TK_ਖੰਡ_Mapping_ਸ਼ੁਰੂ,
    TK_Flow_ਇੰਦਰਾਜ,
    TK_Flow_Sequence_ਸ਼ੁਰੂ,
    TK_Flow_Sequence_ਸਿਰਾ,
    TK_Flow_Mapping_ਸ਼ੁਰੂ,
    TK_Flow_Mapping_ਸਿਰਾ,
    TK_ਕੁੰਜੀ,
    TK_ਮੁੱਲ,
    TK_Scalar,
    TK_ਖੰਡ_Scalar,
    TK_Alias,
    TK_Anchor,
    TK_ਟੈਗ
  } ਕਿਸਮ_3 = TK_Error;

  /// A string of length 0 or more whose begin() points to the logical location
  /// of the token in the input.
  StringRef Range;

  /// The value of a block scalar node.
  std::string ਮੁੱਲ;

  Token() = ਮੂਲ;
};

} // end namespace yaml
} // end namespace llvm

ਵਰਤੋਂ Token_ਕਤਾਰ_T = BumpPtrList<Token>;

ਨਾਮਾਕਾਸ਼ {

/// This struct is used to track simple keys.
///
/// Simple keys are handled by creating an entry in SimpleKeys for each Token
/// which could legally be the start of a simple key. When peekNext is called,
/// if the Token To be returned is referenced by a SimpleKey, we continue
/// tokenizing until that potential simple key has either been found to not be
/// a simple key (we moved on to the next line or went further than 1024 chars).
/// Or when we run into a Value, and then insert a Key token (and possibly
/// others) before the SimpleKey's Tok.
ਬਣਤਰ Simple_ਕੁੰਜੀ {
  Token_ਕਤਾਰ_T::iterator Tok;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਕਾਲਮ = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ FlowLevel = 0;
  ਤਾਰਕਿਕ IsRequired = ਝੂਠ;

  ਤਾਰਕਿਕ ਸੰਚਾਲਕ ==(ਅਚਲ Simple_ਕੁੰਜੀ &Other) {
    ਵਾਪਸ Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
ਵਰਤੋਂ UTF8Decoded = std::pair<uint32_t, ਬਿਨਾਨਿਸ਼ਾਨੀ>;

ਸਥਿਰ UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator ਸਿਰਾ_2 = Range.ਸਿਰਾ();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  ਜੇ (Position < ਸਿਰਾ_2 && (*Position & 0x80) == 0) {
    ਵਾਪਸ {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  ਜੇ (Position + 1 < ਸਿਰਾ_2 && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    ਜੇ (codepoint >= 0x80)
      ਵਾਪਸ {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  ਜੇ (Position + 2 < ਸਿਰਾ_2 && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    ਜੇ (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      ਵਾਪਸ {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  ਜੇ (Position + 3 < ਸਿਰਾ_2 && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    ਜੇ (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      ਵਾਪਸ {codepoint, 4};
  }
  ਵਾਪਸ {0, 0};
}

ਨਾਮਾਕਾਸ਼ llvm {
ਨਾਮਾਕਾਸ਼ yaml {

/// Scans YAML tokens from a MemoryBuffer.
ਵਰਗ Scanner {
ਜਨਤਕ:
  Scanner(StringRef ਨਿਵੇਸ਼, ਸਰੋਤ_Mgr &SM, ਤਾਰਕਿਕ ShowColors = ਸੱਚ,
          std::error_ਕੋਡ *EC = ਖਾਲੀ_ਸੂਚਕ);
  Scanner(ਸਮ੍ਰਿਤੀ_ਬਫਰ_Ref ਬਫਰ, ਸਰੋਤ_Mgr &SM_, ਤਾਰਕਿਕ ShowColors = ਸੱਚ,
          std::error_ਕੋਡ *EC = ਖਾਲੀ_ਸੂਚਕ);

  /// Parse the next token and return it without popping it.
  Token &peek_ਅਗਲਾ();

  /// Parse the next token and pop it from the queue.
  Token get_ਅਗਲਾ();

  ਖਾਲੀ printError(SMLoc Loc, ਸਰੋਤ_Mgr::Diag_ਕਿਸਮ ਕਿਸਮ_3, ਅਚਲ Twine &ਸੁਨੇਹਾ_3,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.Print_ਸੁਨੇਹਾ(Loc, ਕਿਸਮ_3, ਸੁਨੇਹਾ_3, Ranges, /* FixIts= */ {}, ShowColors);
  }

  ਖਾਲੀ ਸੈੱਟ_Error(ਅਚਲ Twine &ਸੁਨੇਹਾ_3, StringRef::iterator Position) {
    ਜੇ (Position >= ਸਿਰਾ_2)
      Position = ਸਿਰਾ_2 - 1;

    // propagate the error if possible
    ਜੇ (EC)
      *EC = make_error_ਕੋਡ(std::errc::invalid_ਦਲੀਲ);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    ਜੇ (!Failed)
      printError(SMLoc::get_From_ਸੂਚਕ(Position), ਸਰੋਤ_Mgr::DK_Error, ਸੁਨੇਹਾ_3);
    Failed = ਸੱਚ;
  }

  /// Returns true if an error occurred while parsing.
  ਤਾਰਕਿਕ failed() {
    ਵਾਪਸ Failed;
  }

ਨਿਜੀ:
  ਖਾਲੀ init(ਸਮ੍ਰਿਤੀ_ਬਫਰ_Ref ਬਫਰ);

  StringRef current_ਨਿਵੇਸ਼() {
    ਵਾਪਸ StringRef(Current, ਸਿਰਾ_2 - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    ਵਾਪਸ ::decodeUTF8(StringRef(Position, ਸਿਰਾ_2 - Position));
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

  ਵਰਤੋਂ SkipWhileFunc = StringRef::iterator (Scanner::*)(StringRef::iterator);

  /// Skip minimal well-formed code unit subsequences until Func
  ///        returns its input.
  ///
  /// @returns The code unit after the last minimal well-formed code unit
  ///          subsequence that Func accepted.
  StringRef::iterator skip_while( SkipWhileFunc Func
                                , StringRef::iterator Position);

  /// Skip minimal well-formed code unit subsequences until Func returns its
  /// input.
  ਖਾਲੀ advanceWhile(SkipWhileFunc Func);

  /// Scan ns-uri-char[39]s starting at Cur.
  ///
  /// This updates Cur and Column while scanning.
  ਖਾਲੀ ਜਾਂਚ_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  ਤਾਰਕਿਕ consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  ਖਾਲੀ skip(uint32_t ਦੂਰੀ_2);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  ਤਾਰਕਿਕ isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  ਤਾਰਕਿਕ is_Plain_ਸੁਰੱਖਿਅਤ_Non_Blank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  ਤਾਰਕਿਕ is_ਸਤਰ_ਖਾਲੀ_ਥਾਵਾਂ(StringRef ਸਤਰ);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  ਤਾਰਕਿਕ consume_ਸਤਰ_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  ਖਾਲੀ save_Simple_ਕੁੰਜੀ_Candidate( Token_ਕਤਾਰ_T::iterator Tok
                             , ਬਿਨਾਨਿਸ਼ਾਨੀ At_ਕਾਲਮ
                             , ਤਾਰਕਿਕ IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  ਖਾਲੀ remove_Stale_Simple_ਕੁੰਜੀ_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  ਖਾਲੀ remove_Simple_ਕੁੰਜੀ_Candidates_On_Flow_Level(ਬਿਨਾਨਿਸ਼ਾਨੀ Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  ਤਾਰਕਿਕ unrollIndent(ਪੂਰਨ ਮੰਜ਼ਿਲ_ਕਾਲਮ);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  ਤਾਰਕਿਕ rollIndent( ਪੂਰਨ ਮੰਜ਼ਿਲ_ਕਾਲਮ
                 , Token::Token_ਕਿਸਮ ਕਿਸਮ_3
                 , Token_ਕਤਾਰ_T::iterator ਪਾਓ_ਬਿੰਦੂ);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  ਖਾਲੀ skipComment();

  /// Skip whitespace and comments until the start of the next token.
  ਖਾਲੀ ਜਾਂਚ_ਮੰਜ਼ਿਲ_ਅਗਲਾ_Token();

  /// Must be the first token generated.
  ਤਾਰਕਿਕ ਜਾਂਚ_ਧਾਰਾ_ਸ਼ੁਰੂ();

  /// Generate tokens needed to close out the stream.
  ਤਾਰਕਿਕ ਜਾਂਚ_ਧਾਰਾ_ਸਿਰਾ();

  /// Scan a %BLAH directive.
  ਤਾਰਕਿਕ ਜਾਂਚ_Directive();

  /// Scan a ... or ---.
  ਤਾਰਕਿਕ ਜਾਂਚ_Document_Indicator(ਤਾਰਕਿਕ Is_ਸ਼ੁਰੂ);

  /// Scan a [ or { and generate the proper flow collection start token.
  ਤਾਰਕਿਕ ਜਾਂਚ_Flow_Collection_ਸ਼ੁਰੂ(ਤਾਰਕਿਕ IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  ਤਾਰਕਿਕ ਜਾਂਚ_Flow_Collection_ਸਿਰਾ(ਤਾਰਕਿਕ IsSequence);

  /// Scan the , that separates entries in a flow collection.
  ਤਾਰਕਿਕ ਜਾਂਚ_Flow_ਇੰਦਰਾਜ();

  /// Scan the - that starts block sequence entries.
  ਤਾਰਕਿਕ ਜਾਂਚ_ਖੰਡ_ਇੰਦਰਾਜ();

  /// Scan an explicit ? indicating a key.
  ਤਾਰਕਿਕ ਜਾਂਚ_ਕੁੰਜੀ();

  /// Scan an explicit : indicating a value.
  ਤਾਰਕਿਕ ਜਾਂਚ_ਮੁੱਲ();

  /// Scan a quoted scalar.
  ਤਾਰਕਿਕ ਜਾਂਚ_Flow_Scalar(ਤਾਰਕਿਕ IsDoubleQuoted);

  /// Scan an unquoted scalar.
  ਤਾਰਕਿਕ ਜਾਂਚ_Plain_Scalar();

  /// Scan an Alias or Anchor starting with * or &.
  ਤਾਰਕਿਕ ਜਾਂਚ_Alias_Or_Anchor(ਤਾਰਕਿਕ IsAlias);

  /// Scan a block scalar starting with | or >.
  ਤਾਰਕਿਕ ਜਾਂਚ_ਖੰਡ_Scalar(ਤਾਰਕਿਕ IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  ਤਾਰਕਿਕ ਜਾਂਚ_ਖੰਡ_Scalar_Indicators(ਅੱਖਰ &StyleIndicator, ਅੱਖਰ &ChompingIndicator,
                                 ਬਿਨਾਨਿਸ਼ਾਨੀ &IndentIndicator, ਤਾਰਕਿਕ &Is_ਪੂਰਾ);

  /// Scan a style indicator in a block scalar header.
  ਅੱਖਰ ਜਾਂਚ_ਖੰਡ_Style_Indicator();

  /// Scan a chomping indicator in a block scalar header.
  ਅੱਖਰ ਜਾਂਚ_ਖੰਡ_Chomping_Indicator();

  /// Scan an indentation indicator in a block scalar header.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਜਾਂਚ_ਖੰਡ_Indentation_Indicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  ਤਾਰਕਿਕ ਜਾਂਚ_ਖੰਡ_Scalar_ਸਿਰਲੇਖ(ਅੱਖਰ &ChompingIndicator, ਬਿਨਾਨਿਸ਼ਾਨੀ &IndentIndicator,
                             ਤਾਰਕਿਕ &Is_ਪੂਰਾ);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  ਤਾਰਕਿਕ ਲੱਭੋ_ਖੰਡ_Scalar_Indent(ਬਿਨਾਨਿਸ਼ਾਨੀ &ਖੰਡ_Indent, ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੰਡ_Exit_Indent,
                             ਬਿਨਾਨਿਸ਼ਾਨੀ &ਸਤਰ_Breaks, ਤਾਰਕਿਕ &Is_ਪੂਰਾ);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  ਤਾਰਕਿਕ ਜਾਂਚ_ਖੰਡ_Scalar_Indent(ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੰਡ_Indent, ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੰਡ_Exit_Indent,
                             ਤਾਰਕਿਕ &Is_ਪੂਰਾ);

  /// Scan a tag of the form !stuff.
  ਤਾਰਕਿਕ ਜਾਂਚ_ਟੈਗ();

  /// Dispatch to the next scanning function based on \a *Cur.
  ਤਾਰਕਿਕ fetch_ਹੋਰ_ਟੋਕਨ();

  /// The SourceMgr used for diagnostics and buffer management.
  ਸਰੋਤ_Mgr &SM;

  /// The original input.
  ਸਮ੍ਰਿਤੀ_ਬਫਰ_Ref ਨਿਵੇਸ਼_ਬਫਰ;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator ਸਿਰਾ_2;

  /// Current YAML indentation level in spaces.
  ਪੂਰਨ Indent;

  /// Current column number in Unicode code points.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਕਾਲਮ;

  /// Current line number.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ;

  /// How deep we are in flow style containers. 0 Means at block level.
  ਬਿਨਾਨਿਸ਼ਾਨੀ FlowLevel;

  /// Are we at the start of the stream?
  ਤਾਰਕਿਕ Is_ਸ਼ੁਰੂ_ਦਾ_ਧਾਰਾ;

  /// Can the next token be the start of a simple key?
  ਤਾਰਕਿਕ Is_Simple_ਕੁੰਜੀ_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  ਤਾਰਕਿਕ Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow;

  /// True if an error has occurred.
  ਤਾਰਕਿਕ Failed;

  /// Should colors be used when printing out the diagnostic messages?
  ਤਾਰਕਿਕ ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  Token_ਕਤਾਰ_T Token_ਕਤਾਰ;

  /// Indentation levels.
  Small_ਸਦਿਸ਼<ਪੂਰਨ, 4> Indents;

  /// Potential simple keys.
  Small_ਸਦਿਸ਼<Simple_ਕੁੰਜੀ, 4> Simple_ਕੁੰਜੀਆਂ;

  std::error_ਕੋਡ *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
ਸਥਿਰ ਖਾਲੀ encodeUTF8( uint32_t Unicode_Scalar_ਮੁੱਲ
                      , Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ) {
  ਜੇ (Unicode_Scalar_ਮੁੱਲ <= 0x7F) {
    ਨਤੀਜਾ.ਧੱਕੋ_ਪਿੱਠ(Unicode_Scalar_ਮੁੱਲ & 0x7F);
  } ਨਹੀਂਤਾਂ ਜੇ (Unicode_Scalar_ਮੁੱਲ <= 0x7FF) {
    uint8_t ਪਹਿਲਾ_Byte = 0xC0 | ((Unicode_Scalar_ਮੁੱਲ & 0x7C0) >> 6);
    uint8_t ਦੂਜਾ_Byte = 0x80 | (Unicode_Scalar_ਮੁੱਲ & 0x3F);
    ਨਤੀਜਾ.ਧੱਕੋ_ਪਿੱਠ(ਪਹਿਲਾ_Byte);
    ਨਤੀਜਾ.ਧੱਕੋ_ਪਿੱਠ(ਦੂਜਾ_Byte);
  } ਨਹੀਂਤਾਂ ਜੇ (Unicode_Scalar_ਮੁੱਲ <= 0xFFFF) {
    uint8_t ਪਹਿਲਾ_Byte = 0xE0 | ((Unicode_Scalar_ਮੁੱਲ & 0xF000) >> 12);
    uint8_t ਦੂਜਾ_Byte = 0x80 | ((Unicode_Scalar_ਮੁੱਲ & 0xFC0) >> 6);
    uint8_t ਤੀਜਾ_Byte = 0x80 | (Unicode_Scalar_ਮੁੱਲ & 0x3F);
    ਨਤੀਜਾ.ਧੱਕੋ_ਪਿੱਠ(ਪਹਿਲਾ_Byte);
    ਨਤੀਜਾ.ਧੱਕੋ_ਪਿੱਠ(ਦੂਜਾ_Byte);
    ਨਤੀਜਾ.ਧੱਕੋ_ਪਿੱਠ(ਤੀਜਾ_Byte);
  } ਨਹੀਂਤਾਂ ਜੇ (Unicode_Scalar_ਮੁੱਲ <= 0x10FFFF) {
    uint8_t ਪਹਿਲਾ_Byte = 0xF0 | ((Unicode_Scalar_ਮੁੱਲ & 0x1F0000) >> 18);
    uint8_t ਦੂਜਾ_Byte = 0x80 | ((Unicode_Scalar_ਮੁੱਲ & 0x3F000) >> 12);
    uint8_t ਤੀਜਾ_Byte = 0x80 | ((Unicode_Scalar_ਮੁੱਲ & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_ਮੁੱਲ & 0x3F);
    ਨਤੀਜਾ.ਧੱਕੋ_ਪਿੱਠ(ਪਹਿਲਾ_Byte);
    ਨਤੀਜਾ.ਧੱਕੋ_ਪਿੱਠ(ਦੂਜਾ_Byte);
    ਨਤੀਜਾ.ਧੱਕੋ_ਪਿੱਠ(ਤੀਜਾ_Byte);
    ਨਤੀਜਾ.ਧੱਕੋ_ਪਿੱਠ(FourthByte);
  }
}

ਤਾਰਕਿਕ yaml::dump_ਟੋਕਨ(StringRef ਨਿਵੇਸ਼, raw_ostream &OS) {
  ਸਰੋਤ_Mgr SM;
  Scanner scanner(ਨਿਵੇਸ਼, SM);
  ਜਦੋਂ (ਸੱਚ) {
    Token T = scanner.get_ਅਗਲਾ();
    ਚੋਣ (T.ਕਿਸਮ_3) {
    ਹਾਲਤ Token::TK_ਧਾਰਾ_ਸ਼ੁਰੂ:
      OS << "Stream-Start: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_ਧਾਰਾ_ਸਿਰਾ:
      OS << "Stream-End: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_ਸੰਸਕਰਣ_Directive:
      OS << "Version-Directive: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_ਟੈਗ_Directive:
      OS << "Tag-Directive: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_Document_ਸ਼ੁਰੂ:
      OS << "Document-Start: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_Document_ਸਿਰਾ:
      OS << "Document-End: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_ਖੰਡ_ਇੰਦਰਾਜ:
      OS << "Block-Entry: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_ਖੰਡ_ਸਿਰਾ:
      OS << "Block-End: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_ਖੰਡ_Sequence_ਸ਼ੁਰੂ:
      OS << "Block-Sequence-Start: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_ਖੰਡ_Mapping_ਸ਼ੁਰੂ:
      OS << "Block-Mapping-Start: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_Flow_ਇੰਦਰਾਜ:
      OS << "Flow-Entry: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_Flow_Sequence_ਸ਼ੁਰੂ:
      OS << "Flow-Sequence-Start: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_Flow_Sequence_ਸਿਰਾ:
      OS << "Flow-Sequence-End: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_Flow_Mapping_ਸ਼ੁਰੂ:
      OS << "Flow-Mapping-Start: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_Flow_Mapping_ਸਿਰਾ:
      OS << "Flow-Mapping-End: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_ਕੁੰਜੀ:
      OS << "Key: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_ਮੁੱਲ:
      OS << "Value: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_Scalar:
      OS << "Scalar: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_ਖੰਡ_Scalar:
      OS << "Block Scalar: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_Alias:
      OS << "Alias: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_Anchor:
      OS << "Anchor: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_ਟੈਗ:
      OS << "Tag: ";
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_Error:
      ਰੋਕੋ;
    }
    OS << T.Range << "\n";
    ਜੇ (T.ਕਿਸਮ_3 == Token::TK_ਧਾਰਾ_ਸਿਰਾ)
      ਰੋਕੋ;
    ਨਹੀਂਤਾਂ ਜੇ (T.ਕਿਸਮ_3 == Token::TK_Error)
      ਵਾਪਸ ਝੂਠ;
  }
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ yaml::ਜਾਂਚ_ਟੋਕਨ(StringRef ਨਿਵੇਸ਼) {
  ਸਰੋਤ_Mgr SM;
  Scanner scanner(ਨਿਵੇਸ਼, SM);
  ਜਦੋਂ (ਸੱਚ) {
    Token T = scanner.get_ਅਗਲਾ();
    ਜੇ (T.ਕਿਸਮ_3 == Token::TK_ਧਾਰਾ_ਸਿਰਾ)
      ਰੋਕੋ;
    ਨਹੀਂਤਾਂ ਜੇ (T.ਕਿਸਮ_3 == Token::TK_Error)
      ਵਾਪਸ ਝੂਠ;
  }
  ਵਾਪਸ ਸੱਚ;
}

std::string yaml::escape(StringRef ਨਿਵੇਸ਼, ਤਾਰਕਿਕ EscapePrintable) {
  std::string Escaped_ਨਿਵੇਸ਼;
  ਲਈ (StringRef::iterator i = ਨਿਵੇਸ਼.begin(), e = ਨਿਵੇਸ਼.ਸਿਰਾ(); i != e; ++i) {
    ਜੇ (*i == '\\')
      Escaped_ਨਿਵੇਸ਼ += "\\\\";
    ਨਹੀਂਤਾਂ ਜੇ (*i == '"')
      Escaped_ਨਿਵੇਸ਼ += "\\\"";
    ਨਹੀਂਤਾਂ ਜੇ (*i == 0)
      Escaped_ਨਿਵੇਸ਼ += "\\0";
    ਨਹੀਂਤਾਂ ਜੇ (*i == 0x07)
      Escaped_ਨਿਵੇਸ਼ += "\\a";
    ਨਹੀਂਤਾਂ ਜੇ (*i == 0x08)
      Escaped_ਨਿਵੇਸ਼ += "\\b";
    ਨਹੀਂਤਾਂ ਜੇ (*i == 0x09)
      Escaped_ਨਿਵੇਸ਼ += "\\t";
    ਨਹੀਂਤਾਂ ਜੇ (*i == 0x0A)
      Escaped_ਨਿਵੇਸ਼ += "\\n";
    ਨਹੀਂਤਾਂ ਜੇ (*i == 0x0B)
      Escaped_ਨਿਵੇਸ਼ += "\\v";
    ਨਹੀਂਤਾਂ ਜੇ (*i == 0x0C)
      Escaped_ਨਿਵੇਸ਼ += "\\f";
    ਨਹੀਂਤਾਂ ਜੇ (*i == 0x0D)
      Escaped_ਨਿਵੇਸ਼ += "\\r";
    ਨਹੀਂਤਾਂ ਜੇ (*i == 0x1B)
      Escaped_ਨਿਵੇਸ਼ += "\\e";
    ਨਹੀਂਤਾਂ ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_ਨਿਵੇਸ਼ += "\\x" + std::string(2 - HexStr.ਮਾਪ_2(), '0') + HexStr;
    } ਨਹੀਂਤਾਂ ਜੇ (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_ਮੁੱਲ
        = decodeUTF8(StringRef(i, ਨਿਵੇਸ਼.ਸਿਰਾ() - i));
      ਜੇ (Unicode_Scalar_ਮੁੱਲ.ਦੂਜਾ_2 == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_ਨਿਵੇਸ਼, Val);
        // FIXME: Error reporting.
        ਵਾਪਸ Escaped_ਨਿਵੇਸ਼;
      }
      ਜੇ (Unicode_Scalar_ਮੁੱਲ.ਪਹਿਲਾ_2 == 0x85)
        Escaped_ਨਿਵੇਸ਼ += "\\N";
      ਨਹੀਂਤਾਂ ਜੇ (Unicode_Scalar_ਮੁੱਲ.ਪਹਿਲਾ_2 == 0xA0)
        Escaped_ਨਿਵੇਸ਼ += "\\_";
      ਨਹੀਂਤਾਂ ਜੇ (Unicode_Scalar_ਮੁੱਲ.ਪਹਿਲਾ_2 == 0x2028)
        Escaped_ਨਿਵੇਸ਼ += "\\L";
      ਨਹੀਂਤਾਂ ਜੇ (Unicode_Scalar_ਮੁੱਲ.ਪਹਿਲਾ_2 == 0x2029)
        Escaped_ਨਿਵੇਸ਼ += "\\P";
      ਨਹੀਂਤਾਂ ਜੇ (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_ਮੁੱਲ.ਪਹਿਲਾ_2))
        Escaped_ਨਿਵੇਸ਼ += StringRef(i, Unicode_Scalar_ਮੁੱਲ.ਦੂਜਾ_2);
      ਨਹੀਂਤਾਂ {
        std::string HexStr = utohexstr(Unicode_Scalar_ਮੁੱਲ.ਪਹਿਲਾ_2);
        ਜੇ (HexStr.ਮਾਪ_2() <= 2)
          Escaped_ਨਿਵੇਸ਼ += "\\x" + std::string(2 - HexStr.ਮਾਪ_2(), '0') + HexStr;
        ਨਹੀਂਤਾਂ ਜੇ (HexStr.ਮਾਪ_2() <= 4)
          Escaped_ਨਿਵੇਸ਼ += "\\u" + std::string(4 - HexStr.ਮਾਪ_2(), '0') + HexStr;
        ਨਹੀਂਤਾਂ ਜੇ (HexStr.ਮਾਪ_2() <= 8)
          Escaped_ਨਿਵੇਸ਼ += "\\U" + std::string(8 - HexStr.ਮਾਪ_2(), '0') + HexStr;
      }
      i += Unicode_Scalar_ਮੁੱਲ.ਦੂਜਾ_2 - 1;
    } ਨਹੀਂਤਾਂ {
      Escaped_ਨਿਵੇਸ਼.ਧੱਕੋ_ਪਿੱਠ(*i);
    }
  }
  ਵਾਪਸ Escaped_ਨਿਵੇਸ਼;
}

std::optional<ਤਾਰਕਿਕ> yaml::parseBool(StringRef S) {
  ਚੋਣ (S.ਮਾਪ_2()) {
  ਹਾਲਤ 1:
    ਚੋਣ (S.front()) {
    ਹਾਲਤ 'y':
    ਹਾਲਤ 'Y':
      ਵਾਪਸ ਸੱਚ;
    ਹਾਲਤ 'n':
    ਹਾਲਤ 'N':
      ਵਾਪਸ ਝੂਠ;
    ਮੂਲ:
      ਵਾਪਸ std::nullopt;
    }
  ਹਾਲਤ 2:
    ਚੋਣ (S.front()) {
    ਹਾਲਤ 'O':
      ਜੇ (S[1] == 'N') // ON
        ਵਾਪਸ ਸੱਚ;
      [[fallthrough]];
    ਹਾਲਤ 'o':
      ਜੇ (S[1] == 'n') //[Oo]n
        ਵਾਪਸ ਸੱਚ;
      ਵਾਪਸ std::nullopt;
    ਹਾਲਤ 'N':
      ਜੇ (S[1] == 'O') // NO
        ਵਾਪਸ ਝੂਠ;
      [[fallthrough]];
    ਹਾਲਤ 'n':
      ਜੇ (S[1] == 'o') //[Nn]o
        ਵਾਪਸ ਝੂਠ;
      ਵਾਪਸ std::nullopt;
    ਮੂਲ:
      ਵਾਪਸ std::nullopt;
    }
  ਹਾਲਤ 3:
    ਚੋਣ (S.front()) {
    ਹਾਲਤ 'O':
      ਜੇ (S.drop_front() == "FF") // OFF
        ਵਾਪਸ ਝੂਠ;
      [[fallthrough]];
    ਹਾਲਤ 'o':
      ਜੇ (S.drop_front() == "ff") //[Oo]ff
        ਵਾਪਸ ਝੂਠ;
      ਵਾਪਸ std::nullopt;
    ਹਾਲਤ 'Y':
      ਜੇ (S.drop_front() == "ES") // YES
        ਵਾਪਸ ਸੱਚ;
      [[fallthrough]];
    ਹਾਲਤ 'y':
      ਜੇ (S.drop_front() == "es") //[Yy]es
        ਵਾਪਸ ਸੱਚ;
      ਵਾਪਸ std::nullopt;
    ਮੂਲ:
      ਵਾਪਸ std::nullopt;
    }
  ਹਾਲਤ 4:
    ਚੋਣ (S.front()) {
    ਹਾਲਤ 'T':
      ਜੇ (S.drop_front() == "RUE") // TRUE
        ਵਾਪਸ ਸੱਚ;
      [[fallthrough]];
    ਹਾਲਤ 't':
      ਜੇ (S.drop_front() == "rue") //[Tt]rue
        ਵਾਪਸ ਸੱਚ;
      ਵਾਪਸ std::nullopt;
    ਮੂਲ:
      ਵਾਪਸ std::nullopt;
    }
  ਹਾਲਤ 5:
    ਚੋਣ (S.front()) {
    ਹਾਲਤ 'F':
      ਜੇ (S.drop_front() == "ALSE") // FALSE
        ਵਾਪਸ ਝੂਠ;
      [[fallthrough]];
    ਹਾਲਤ 'f':
      ਜੇ (S.drop_front() == "alse") //[Ff]alse
        ਵਾਪਸ ਝੂਠ;
      ਵਾਪਸ std::nullopt;
    ਮੂਲ:
      ਵਾਪਸ std::nullopt;
    }
  ਮੂਲ:
    ਵਾਪਸ std::nullopt;
  }
}

Scanner::Scanner(StringRef ਨਿਵੇਸ਼, ਸਰੋਤ_Mgr &sm, ਤਾਰਕਿਕ ShowColors,
                 std::error_ਕੋਡ *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(ਸਮ੍ਰਿਤੀ_ਬਫਰ_Ref(ਨਿਵੇਸ਼, "YAML"));
}

Scanner::Scanner(ਸਮ੍ਰਿਤੀ_ਬਫਰ_Ref ਬਫਰ, ਸਰੋਤ_Mgr &SM_, ਤਾਰਕਿਕ ShowColors,
                 std::error_ਕੋਡ *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(ਬਫਰ);
}

ਖਾਲੀ Scanner::init(ਸਮ੍ਰਿਤੀ_ਬਫਰ_Ref ਬਫਰ) {
  ਨਿਵੇਸ਼_ਬਫਰ = ਬਫਰ;
  Current = ਨਿਵੇਸ਼_ਬਫਰ.get_ਬਫਰ_ਸ਼ੁਰੂ();
  ਸਿਰਾ_2 = ਨਿਵੇਸ਼_ਬਫਰ.get_ਬਫਰ_ਸਿਰਾ();
  Indent = -1;
  ਕਾਲਮ = 0;
  ਸਤਰ = 0;
  FlowLevel = 0;
  Is_ਸ਼ੁਰੂ_ਦਾ_ਧਾਰਾ = ਸੱਚ;
  Is_Simple_ਕੁੰਜੀ_Allowed = ਸੱਚ;
  Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow = ਝੂਠ;
  Failed = ਝੂਠ;
  std::unique_ptr<ਸਮ੍ਰਿਤੀ_ਬਫਰ> ਨਿਵੇਸ਼_ਬਫਰ_Owner =
      ਸਮ੍ਰਿਤੀ_ਬਫਰ::get_Mem_ਬਫਰ(ਬਫਰ, /*RequiresNullTerminator=*/ਝੂਠ);
  SM.ਜੋੜੋ_New_ਸਰੋਤ_ਬਫਰ(std::move(ਨਿਵੇਸ਼_ਬਫਰ_Owner), SMLoc());
}

Token &Scanner::peek_ਅਗਲਾ() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  ਤਾਰਕਿਕ Need_ਹੋਰ = ਝੂਠ;
  ਜਦੋਂ (ਸੱਚ) {
    ਜੇ (Token_ਕਤਾਰ.ਖਾਲੀ_ਥਾਵਾਂ() || Need_ਹੋਰ) {
      ਜੇ (!fetch_ਹੋਰ_ਟੋਕਨ()) {
        Token_ਕਤਾਰ.clear();
        Simple_ਕੁੰਜੀਆਂ.clear();
        Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(Token());
        ਵਾਪਸ Token_ਕਤਾਰ.front();
      }
    }
    ਪ੍ਰਤਿਜ੍ਞਾ(!Token_ਕਤਾਰ.ਖਾਲੀ_ਥਾਵਾਂ() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_ਕੁੰਜੀ_Candidates();
    Simple_ਕੁੰਜੀ SK;
    SK.Tok = Token_ਕਤਾਰ.begin();
    ਜੇ (!is_contained(Simple_ਕੁੰਜੀਆਂ, SK))
      ਰੋਕੋ;
    ਨਹੀਂਤਾਂ
      Need_ਹੋਰ = ਸੱਚ;
  }
  ਵਾਪਸ Token_ਕਤਾਰ.front();
}

Token Scanner::get_ਅਗਲਾ() {
  Token Ret = peek_ਅਗਲਾ();
  // TokenQueue can be empty if there was an error getting the next token.
  ਜੇ (!Token_ਕਤਾਰ.ਖਾਲੀ_ਥਾਵਾਂ())
    Token_ਕਤਾਰ.ਕੱਢੋ_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  ਜੇ (Token_ਕਤਾਰ.ਖਾਲੀ_ਥਾਵਾਂ())
    Token_ਕਤਾਰ.resetAlloc();

  ਵਾਪਸ Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  ਜੇ (Position == ਸਿਰਾ_2)
    ਵਾਪਸ Position;
  // Check 7 bit c-printable - b-char.
  ਜੇ (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    ਵਾਪਸ Position + 1;

  // Check for valid UTF-8.
  ਜੇ (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    ਜੇ (   u8d.ਦੂਜਾ_2 != 0
        && u8d.ਪਹਿਲਾ_2 != 0xFEFF
        && ( u8d.ਪਹਿਲਾ_2 == 0x85
          || ( u8d.ਪਹਿਲਾ_2 >= 0xA0
            && u8d.ਪਹਿਲਾ_2 <= 0xD7FF)
          || ( u8d.ਪਹਿਲਾ_2 >= 0xE000
            && u8d.ਪਹਿਲਾ_2 <= 0xFFFD)
          || ( u8d.ਪਹਿਲਾ_2 >= 0x10000
            && u8d.ਪਹਿਲਾ_2 <= 0x10FFFF)))
      ਵਾਪਸ Position + u8d.ਦੂਜਾ_2;
  }
  ਵਾਪਸ Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  ਜੇ (Position == ਸਿਰਾ_2)
    ਵਾਪਸ Position;
  ਜੇ (*Position == 0x0D) {
    ਜੇ (Position + 1 != ਸਿਰਾ_2 && *(Position + 1) == 0x0A)
      ਵਾਪਸ Position + 2;
    ਵਾਪਸ Position + 1;
  }

  ਜੇ (*Position == 0x0A)
    ਵਾਪਸ Position + 1;
  ਵਾਪਸ Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  ਜੇ (Position == ਸਿਰਾ_2)
    ਵਾਪਸ Position;
  ਜੇ (*Position == ' ')
    ਵਾਪਸ Position + 1;
  ਵਾਪਸ Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  ਜੇ (Position == ਸਿਰਾ_2)
    ਵਾਪਸ Position;
  ਜੇ (*Position == ' ' || *Position == '\t')
    ਵਾਪਸ Position + 1;
  ਵਾਪਸ Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  ਜੇ (Position == ਸਿਰਾ_2)
    ਵਾਪਸ Position;
  ਜੇ (*Position == ' ' || *Position == '\t')
    ਵਾਪਸ Position;
  ਵਾਪਸ skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  ਜਦੋਂ (ਸੱਚ) {
    StringRef::iterator i = (ਇਹ->*Func)(Position);
    ਜੇ (i == Position)
      ਰੋਕੋ;
    Position = i;
  }
  ਵਾਪਸ Position;
}

ਖਾਲੀ Scanner::advanceWhile(SkipWhileFunc Func) {
  ਸਵੈਚਾਲਿਤ Final = skip_while(Func, Current);
  ਕਾਲਮ += Final - Current;
  Current = Final;
}

ਸਥਿਰ ਤਾਰਕਿਕ is_ns_hex_digit(ਅਚਲ ਅੱਖਰ C) { ਵਾਪਸ isAlnum(C); }

ਸਥਿਰ ਤਾਰਕਿਕ is_ns_ਸ਼ਬਦ_char(ਅਚਲ ਅੱਖਰ C) { ਵਾਪਸ C == '-' || isAlpha(C); }

ਖਾਲੀ Scanner::ਜਾਂਚ_ns_uri_char() {
  ਜਦੋਂ (ਸੱਚ) {
    ਜੇ (Current == ਸਿਰਾ_2)
      ਰੋਕੋ;
    ਜੇ ((*Current == '%' && Current + 2 < ਸਿਰਾ_2 &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_ਸ਼ਬਦ_char(*Current) ||
        StringRef(Current, 1).ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++ਕਾਲਮ;
    } ਨਹੀਂਤਾਂ {
      ਰੋਕੋ;
    }
  }
}

ਤਾਰਕਿਕ Scanner::consume(uint32_t Expected) {
  ਜੇ (Expected >= 0x80) {
    ਸੈੱਟ_Error("Cannot consume non-ascii characters", Current);
    ਵਾਪਸ ਝੂਠ;
  }
  ਜੇ (Current == ਸਿਰਾ_2)
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (uint8_t(*Current) >= 0x80) {
    ਸੈੱਟ_Error("Cannot consume non-ascii characters", Current);
    ਵਾਪਸ ਝੂਠ;
  }
  ਜੇ (uint8_t(*Current) == Expected) {
    ++Current;
    ++ਕਾਲਮ;
    ਵਾਪਸ ਸੱਚ;
  }
  ਵਾਪਸ ਝੂਠ;
}

ਖਾਲੀ Scanner::skip(uint32_t ਦੂਰੀ_2) {
  Current += ਦੂਰੀ_2;
  ਕਾਲਮ += ਦੂਰੀ_2;
  ਪ੍ਰਤਿਜ੍ਞਾ(Current <= ਸਿਰਾ_2 && "Skipped past the end");
}

ਤਾਰਕਿਕ Scanner::isBlankOrBreak(StringRef::iterator Position) {
  ਜੇ (Position == ਸਿਰਾ_2)
    ਵਾਪਸ ਝੂਠ;
  ਵਾਪਸ *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

ਤਾਰਕਿਕ Scanner::is_Plain_ਸੁਰੱਖਿਅਤ_Non_Blank(StringRef::iterator Position) {
  ਜੇ (Position == ਸਿਰਾ_2 || isBlankOrBreak(Position))
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (FlowLevel &&
      StringRef(Position, 1).ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(",[]{}") != StringRef::npos)
    ਵਾਪਸ ਝੂਠ;
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::is_ਸਤਰ_ਖਾਲੀ_ਥਾਵਾਂ(StringRef ਸਤਰ) {
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ *Position = ਸਤਰ.begin(); Position != ਸਤਰ.ਸਿਰਾ(); ++Position)
    ਜੇ (!isBlankOrBreak(Position))
      ਵਾਪਸ ਝੂਠ;
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::consume_ਸਤਰ_Break_If_Present() {
  ਸਵੈਚਾਲਿਤ ਅਗਲਾ_2 = skip_b_break(Current);
  ਜੇ (ਅਗਲਾ_2 == Current)
    ਵਾਪਸ ਝੂਠ;
  ਕਾਲਮ = 0;
  ++ਸਤਰ;
  Current = ਅਗਲਾ_2;
  ਵਾਪਸ ਸੱਚ;
}

ਖਾਲੀ Scanner::save_Simple_ਕੁੰਜੀ_Candidate( Token_ਕਤਾਰ_T::iterator Tok
                                    , ਬਿਨਾਨਿਸ਼ਾਨੀ At_ਕਾਲਮ
                                    , ਤਾਰਕਿਕ IsRequired) {
  ਜੇ (Is_Simple_ਕੁੰਜੀ_Allowed) {
    Simple_ਕੁੰਜੀ SK;
    SK.Tok = Tok;
    SK.ਸਤਰ = ਸਤਰ;
    SK.ਕਾਲਮ = At_ਕਾਲਮ;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_ਕੁੰਜੀਆਂ.ਧੱਕੋ_ਪਿੱਠ(SK);
  }
}

ਖਾਲੀ Scanner::remove_Stale_Simple_ਕੁੰਜੀ_Candidates() {
  ਲਈ (Small_ਸਦਿਸ਼_Impl<Simple_ਕੁੰਜੀ>::iterator i = Simple_ਕੁੰਜੀਆਂ.begin();
                                            i != Simple_ਕੁੰਜੀਆਂ.ਸਿਰਾ();) {
    ਜੇ (i->ਸਤਰ != ਸਤਰ || i->ਕਾਲਮ + 1024 < ਕਾਲਮ) {
      ਜੇ (i->IsRequired)
        ਸੈੱਟ_Error( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_ਕੁੰਜੀਆਂ.erase(i);
    } ਨਹੀਂਤਾਂ {
      ++i;
    }
  }
}

ਖਾਲੀ Scanner::remove_Simple_ਕੁੰਜੀ_Candidates_On_Flow_Level(ਬਿਨਾਨਿਸ਼ਾਨੀ Level) {
  ਜੇ (!Simple_ਕੁੰਜੀਆਂ.ਖਾਲੀ_ਥਾਵਾਂ() && (Simple_ਕੁੰਜੀਆਂ.ਸਿਰਾ() - 1)->FlowLevel == Level)
    Simple_ਕੁੰਜੀਆਂ.ਕੱਢੋ_ਪਿੱਠ();
}

ਤਾਰਕਿਕ Scanner::unrollIndent(ਪੂਰਨ ਮੰਜ਼ਿਲ_ਕਾਲਮ) {
  Token T;
  // Indentation is ignored in flow.
  ਜੇ (FlowLevel != 0)
    ਵਾਪਸ ਸੱਚ;

  ਜਦੋਂ (Indent > ਮੰਜ਼ਿਲ_ਕਾਲਮ) {
    T.ਕਿਸਮ_3 = Token::TK_ਖੰਡ_ਸਿਰਾ;
    T.Range = StringRef(Current, 1);
    Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);
    Indent = Indents.ਕੱਢੋ_ਪਿੱਠ_val();
  }

  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::rollIndent( ਪੂਰਨ ਮੰਜ਼ਿਲ_ਕਾਲਮ
                        , Token::Token_ਕਿਸਮ ਕਿਸਮ_3
                        , Token_ਕਤਾਰ_T::iterator ਪਾਓ_ਬਿੰਦੂ) {
  ਜੇ (FlowLevel)
    ਵਾਪਸ ਸੱਚ;
  ਜੇ (Indent < ਮੰਜ਼ਿਲ_ਕਾਲਮ) {
    Indents.ਧੱਕੋ_ਪਿੱਠ(Indent);
    Indent = ਮੰਜ਼ਿਲ_ਕਾਲਮ;

    Token T;
    T.ਕਿਸਮ_3 = ਕਿਸਮ_3;
    T.Range = StringRef(Current, 0);
    Token_ਕਤਾਰ.ਪਾਓ(ਪਾਓ_ਬਿੰਦੂ, T);
  }
  ਵਾਪਸ ਸੱਚ;
}

ਖਾਲੀ Scanner::skipComment() {
  ਜੇ (Current == ਸਿਰਾ_2 || *Current != '#')
    ਵਾਪਸ;
  ਜਦੋਂ (ਸੱਚ) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    ਜੇ (I == Current)
      ਰੋਕੋ;
    Current = I;
    ++ਕਾਲਮ;
  }
}

ਖਾਲੀ Scanner::ਜਾਂਚ_ਮੰਜ਼ਿਲ_ਅਗਲਾ_Token() {
  ਜਦੋਂ (ਸੱਚ) {
    ਜਦੋਂ (Current != ਸਿਰਾ_2 && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    ਜੇ (i == Current)
      ਰੋਕੋ;
    Current = i;
    ++ਸਤਰ;
    ਕਾਲਮ = 0;
    // New lines may start a simple key.
    ਜੇ (!FlowLevel)
      Is_Simple_ਕੁੰਜੀ_Allowed = ਸੱਚ;
  }
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_ਧਾਰਾ_ਸ਼ੁਰੂ() {
  Is_ਸ਼ੁਰੂ_ਦਾ_ਧਾਰਾ = ਝੂਠ;

  EncodingInfo EI = getUnicodeEncoding(current_ਨਿਵੇਸ਼());

  Token T;
  T.ਕਿਸਮ_3 = Token::TK_ਧਾਰਾ_ਸ਼ੁਰੂ;
  T.Range = StringRef(Current, EI.ਦੂਜਾ_2);
  Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);
  Current += EI.ਦੂਜਾ_2;
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_ਧਾਰਾ_ਸਿਰਾ() {
  // Force an ending new line if one isn't present.
  ਜੇ (ਕਾਲਮ != 0) {
    ਕਾਲਮ = 0;
    ++ਸਤਰ;
  }

  unrollIndent(-1);
  Simple_ਕੁੰਜੀਆਂ.clear();
  Is_Simple_ਕੁੰਜੀ_Allowed = ਝੂਠ;
  Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow = ਝੂਠ;

  Token T;
  T.ਕਿਸਮ_3 = Token::TK_ਧਾਰਾ_ਸਿਰਾ;
  T.Range = StringRef(Current, 0);
  Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_Directive() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_ਕੁੰਜੀਆਂ.clear();
  Is_Simple_ਕੁੰਜੀ_Allowed = ਝੂਠ;
  Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow = ਝੂਠ;

  StringRef::iterator ਸ਼ੁਰੂ = Current;
  consume('%');
  StringRef::iterator Name_ਸ਼ੁਰੂ = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(Name_ਸ਼ੁਰੂ, Current - Name_ਸ਼ੁਰੂ);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  ਜੇ (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.ਕਿਸਮ_3 = Token::TK_ਸੰਸਕਰਣ_Directive;
    T.Range = StringRef(ਸ਼ੁਰੂ, Current - ਸ਼ੁਰੂ);
    Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);
    ਵਾਪਸ ਸੱਚ;
  } ਨਹੀਂਤਾਂ ਜੇ(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.ਕਿਸਮ_3 = Token::TK_ਟੈਗ_Directive;
    T.Range = StringRef(ਸ਼ੁਰੂ, Current - ਸ਼ੁਰੂ);
    Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);
    ਵਾਪਸ ਸੱਚ;
  }
  ਵਾਪਸ ਝੂਠ;
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_Document_Indicator(ਤਾਰਕਿਕ Is_ਸ਼ੁਰੂ) {
  unrollIndent(-1);
  Simple_ਕੁੰਜੀਆਂ.clear();
  Is_Simple_ਕੁੰਜੀ_Allowed = ਝੂਠ;
  Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow = ਝੂਠ;

  Token T;
  T.ਕਿਸਮ_3 = Is_ਸ਼ੁਰੂ ? Token::TK_Document_ਸ਼ੁਰੂ : Token::TK_Document_ਸਿਰਾ;
  T.Range = StringRef(Current, 3);
  skip(3);
  Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_Flow_Collection_ਸ਼ੁਰੂ(ਤਾਰਕਿਕ IsSequence) {
  Token T;
  T.ਕਿਸਮ_3 = IsSequence ? Token::TK_Flow_Sequence_ਸ਼ੁਰੂ
                      : Token::TK_Flow_Mapping_ਸ਼ੁਰੂ;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);

  // [ and { may begin a simple key.
  save_Simple_ਕੁੰਜੀ_Candidate(--Token_ਕਤਾਰ.ਸਿਰਾ(), ਕਾਲਮ - 1, ਝੂਠ);

  // And may also be followed by a simple key.
  Is_Simple_ਕੁੰਜੀ_Allowed = ਸੱਚ;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow = ਝੂਠ;
  ++FlowLevel;
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_Flow_Collection_ਸਿਰਾ(ਤਾਰਕਿਕ IsSequence) {
  remove_Simple_ਕੁੰਜੀ_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ਕੁੰਜੀ_Allowed = ਝੂਠ;
  Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow = ਸੱਚ;
  Token T;
  T.ਕਿਸਮ_3 = IsSequence ? Token::TK_Flow_Sequence_ਸਿਰਾ
                      : Token::TK_Flow_Mapping_ਸਿਰਾ;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);
  ਜੇ (FlowLevel)
    --FlowLevel;
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_Flow_ਇੰਦਰਾਜ() {
  remove_Simple_ਕੁੰਜੀ_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ਕੁੰਜੀ_Allowed = ਸੱਚ;
  Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow = ਝੂਠ;
  Token T;
  T.ਕਿਸਮ_3 = Token::TK_Flow_ਇੰਦਰਾਜ;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_ਖੰਡ_ਇੰਦਰਾਜ() {
  rollIndent(ਕਾਲਮ, Token::TK_ਖੰਡ_Sequence_ਸ਼ੁਰੂ, Token_ਕਤਾਰ.ਸਿਰਾ());
  remove_Simple_ਕੁੰਜੀ_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ਕੁੰਜੀ_Allowed = ਸੱਚ;
  Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow = ਝੂਠ;
  Token T;
  T.ਕਿਸਮ_3 = Token::TK_ਖੰਡ_ਇੰਦਰਾਜ;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_ਕੁੰਜੀ() {
  ਜੇ (!FlowLevel)
    rollIndent(ਕਾਲਮ, Token::TK_ਖੰਡ_Mapping_ਸ਼ੁਰੂ, Token_ਕਤਾਰ.ਸਿਰਾ());

  remove_Simple_ਕੁੰਜੀ_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ਕੁੰਜੀ_Allowed = !FlowLevel;
  Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow = ਝੂਠ;

  Token T;
  T.ਕਿਸਮ_3 = Token::TK_ਕੁੰਜੀ;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_ਮੁੱਲ() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  ਜੇ (!Simple_ਕੁੰਜੀਆਂ.ਖਾਲੀ_ਥਾਵਾਂ()) {
    Simple_ਕੁੰਜੀ SK = Simple_ਕੁੰਜੀਆਂ.ਕੱਢੋ_ਪਿੱਠ_val();
    Token T;
    T.ਕਿਸਮ_3 = Token::TK_ਕੁੰਜੀ;
    T.Range = SK.Tok->Range;
    Token_ਕਤਾਰ_T::iterator i, e;
    ਲਈ (i = Token_ਕਤਾਰ.begin(), e = Token_ਕਤਾਰ.ਸਿਰਾ(); i != e; ++i) {
      ਜੇ (i == SK.Tok)
        ਰੋਕੋ;
    }
    ਜੇ (i == e) {
      Failed = ਸੱਚ;
      ਵਾਪਸ ਝੂਠ;
    }
    i = Token_ਕਤਾਰ.ਪਾਓ(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.ਕਾਲਮ, Token::TK_ਖੰਡ_Mapping_ਸ਼ੁਰੂ, i);

    Is_Simple_ਕੁੰਜੀ_Allowed = ਝੂਠ;
  } ਨਹੀਂਤਾਂ {
    ਜੇ (!FlowLevel)
      rollIndent(ਕਾਲਮ, Token::TK_ਖੰਡ_Mapping_ਸ਼ੁਰੂ, Token_ਕਤਾਰ.ਸਿਰਾ());
    Is_Simple_ਕੁੰਜੀ_Allowed = !FlowLevel;
  }
  Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow = ਝੂਠ;

  Token T;
  T.ਕਿਸਮ_3 = Token::TK_ਮੁੱਲ;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);
  ਵਾਪਸ ਸੱਚ;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE ਸਥਿਰ ਤਾਰਕਿਕ
wasEscaped(StringRef::iterator ਪਹਿਲਾ, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
ਸਥਿਰ ਤਾਰਕਿਕ wasEscaped(StringRef::iterator ਪਹਿਲਾ,
                       StringRef::iterator Position) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Position - 1 >= ਪਹਿਲਾ);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  ਜਦੋਂ (I >= ਪਹਿਲਾ && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  ਵਾਪਸ (Position - 1 - I) % 2 == 1;
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_Flow_Scalar(ਤਾਰਕਿਕ IsDoubleQuoted) {
  StringRef::iterator ਸ਼ੁਰੂ = Current;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਾਨਾ_ਸ਼ੁਰੂ = ਕਾਲਮ;
  ਜੇ (IsDoubleQuoted) {
    ਕਰੋ {
      ++Current;
      ਜਦੋਂ (Current != ਸਿਰਾ_2 && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } ਜਦੋਂ (   Current != ਸਿਰਾ_2
             && *(Current - 1) == '\\'
             && wasEscaped(ਸ਼ੁਰੂ + 1, Current));
  } ਨਹੀਂਤਾਂ {
    skip(1);
    ਜਦੋਂ (Current != ਸਿਰਾ_2) {
      // Skip a ' followed by another '.
      ਜੇ (Current + 1 < ਸਿਰਾ_2 && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        ਜਾਰੀ;
      } ਨਹੀਂਤਾਂ ਜੇ (*Current == '\'')
        ਰੋਕੋ;
      StringRef::iterator i = skip_nb_char(Current);
      ਜੇ (i == Current) {
        i = skip_b_break(Current);
        ਜੇ (i == Current)
          ਰੋਕੋ;
        Current = i;
        ਕਾਲਮ = 0;
        ++ਸਤਰ;
      } ਨਹੀਂਤਾਂ {
        ਜੇ (i == ਸਿਰਾ_2)
          ਰੋਕੋ;
        Current = i;
        ++ਕਾਲਮ;
      }
    }
  }

  ਜੇ (Current == ਸਿਰਾ_2) {
    ਸੈੱਟ_Error("Expected quote at end of scalar", Current);
    ਵਾਪਸ ਝੂਠ;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.ਕਿਸਮ_3 = Token::TK_Scalar;
  T.Range = StringRef(ਸ਼ੁਰੂ, Current - ਸ਼ੁਰੂ);
  Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);

  save_Simple_ਕੁੰਜੀ_Candidate(--Token_ਕਤਾਰ.ਸਿਰਾ(), ਖਾਨਾ_ਸ਼ੁਰੂ, ਝੂਠ);

  Is_Simple_ਕੁੰਜੀ_Allowed = ਝੂਠ;
  Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow = ਸੱਚ;

  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_Plain_Scalar() {
  StringRef::iterator ਸ਼ੁਰੂ = Current;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਾਨਾ_ਸ਼ੁਰੂ = ਕਾਲਮ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ LeadingBlanks = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ(Indent >= -1 && "Indent must be >= -1 !");
  ਬਿਨਾਨਿਸ਼ਾਨੀ indent = static_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ>(Indent + 1);
  ਜਦੋਂ (Current != ਸਿਰਾ_2) {
    ਜੇ (*Current == '#')
      ਰੋਕੋ;

    ਜਦੋਂ (Current != ਸਿਰਾ_2 &&
           ((*Current != ':' && is_Plain_ਸੁਰੱਖਿਅਤ_Non_Blank(Current)) ||
            (*Current == ':' && is_Plain_ਸੁਰੱਖਿਅਤ_Non_Blank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      ਜੇ (i == Current)
        ਰੋਕੋ;
      Current = i;
      ++ਕਾਲਮ;
    }

    // Are we at the end?
    ਜੇ (!isBlankOrBreak(Current))
      ਰੋਕੋ;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    ਜਦੋਂ (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      ਜੇ (i != Tmp) {
        ਜੇ (LeadingBlanks && (ਕਾਲਮ < indent) && *Tmp == '\t') {
          ਸੈੱਟ_Error("Found invalid tab character in indentation", Tmp);
          ਵਾਪਸ ਝੂਠ;
        }
        Tmp = i;
        ++ਕਾਲਮ;
      } ਨਹੀਂਤਾਂ {
        i = skip_b_break(Tmp);
        ਜੇ (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        ਕਾਲਮ = 0;
        ++ਸਤਰ;
      }
    }

    ਜੇ (!FlowLevel && ਕਾਲਮ < indent)
      ਰੋਕੋ;

    Current = Tmp;
  }
  ਜੇ (ਸ਼ੁਰੂ == Current) {
    ਸੈੱਟ_Error("Got empty plain scalar", ਸ਼ੁਰੂ);
    ਵਾਪਸ ਝੂਠ;
  }
  Token T;
  T.ਕਿਸਮ_3 = Token::TK_Scalar;
  T.Range = StringRef(ਸ਼ੁਰੂ, Current - ਸ਼ੁਰੂ);
  Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);

  // Plain scalars can be simple keys.
  save_Simple_ਕੁੰਜੀ_Candidate(--Token_ਕਤਾਰ.ਸਿਰਾ(), ਖਾਨਾ_ਸ਼ੁਰੂ, ਝੂਠ);

  Is_Simple_ਕੁੰਜੀ_Allowed = ਝੂਠ;
  Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow = ਝੂਠ;

  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_Alias_Or_Anchor(ਤਾਰਕਿਕ IsAlias) {
  StringRef::iterator ਸ਼ੁਰੂ = Current;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਾਨਾ_ਸ਼ੁਰੂ = ਕਾਲਮ;
  skip(1);
  ਜਦੋਂ (Current != ਸਿਰਾ_2) {
    ਜੇ (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      ਰੋਕੋ;
    StringRef::iterator i = skip_ns_char(Current);
    ਜੇ (i == Current)
      ਰੋਕੋ;
    Current = i;
    ++ਕਾਲਮ;
  }

  ਜੇ (ਸ਼ੁਰੂ + 1 == Current) {
    ਸੈੱਟ_Error("Got empty alias or anchor", ਸ਼ੁਰੂ);
    ਵਾਪਸ ਝੂਠ;
  }

  Token T;
  T.ਕਿਸਮ_3 = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(ਸ਼ੁਰੂ, Current - ਸ਼ੁਰੂ);
  Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);

  // Alias and anchors can be simple keys.
  save_Simple_ਕੁੰਜੀ_Candidate(--Token_ਕਤਾਰ.ਸਿਰਾ(), ਖਾਨਾ_ਸ਼ੁਰੂ, ਝੂਠ);

  Is_Simple_ਕੁੰਜੀ_Allowed = ਝੂਠ;
  Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow = ਝੂਠ;

  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_ਖੰਡ_Scalar_Indicators(ਅੱਖਰ &StyleIndicator,
                                        ਅੱਖਰ &ChompingIndicator,
                                        ਬਿਨਾਨਿਸ਼ਾਨੀ &IndentIndicator,
                                        ਤਾਰਕਿਕ &Is_ਪੂਰਾ) {
  StyleIndicator = ਜਾਂਚ_ਖੰਡ_Style_Indicator();
  ਜੇ (!ਜਾਂਚ_ਖੰਡ_Scalar_ਸਿਰਲੇਖ(ChompingIndicator, IndentIndicator, Is_ਪੂਰਾ))
    ਵਾਪਸ ਝੂਠ;
  ਵਾਪਸ ਸੱਚ;
}

ਅੱਖਰ Scanner::ਜਾਂਚ_ਖੰਡ_Style_Indicator() {
  ਅੱਖਰ Indicator = ' ';
  ਜੇ (Current != ਸਿਰਾ_2 && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  ਵਾਪਸ Indicator;
}

ਅੱਖਰ Scanner::ਜਾਂਚ_ਖੰਡ_Chomping_Indicator() {
  ਅੱਖਰ Indicator = ' ';
  ਜੇ (Current != ਸਿਰਾ_2 && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  ਵਾਪਸ Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ get_Chomped_ਸਤਰ_Breaks(ਅੱਖਰ ChompingIndicator,
                                     ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_Breaks, StringRef Str) {
  ਜੇ (ChompingIndicator == '-') // Strip all line breaks.
    ਵਾਪਸ 0;
  ਜੇ (ChompingIndicator == '+') // Keep all line breaks.
    ਵਾਪਸ ਸਤਰ_Breaks;
  // Clip trailing lines.
  ਵਾਪਸ Str.ਖਾਲੀ_ਥਾਵਾਂ() ? 0 : 1;
}

ਬਿਨਾਨਿਸ਼ਾਨੀ Scanner::ਜਾਂਚ_ਖੰਡ_Indentation_Indicator() {
  ਬਿਨਾਨਿਸ਼ਾਨੀ Indent = 0;
  ਜੇ (Current != ਸਿਰਾ_2 && (*Current >= '1' && *Current <= '9')) {
    Indent = ਬਿਨਾਨਿਸ਼ਾਨੀ(*Current - '0');
    skip(1);
  }
  ਵਾਪਸ Indent;
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_ਖੰਡ_Scalar_ਸਿਰਲੇਖ(ਅੱਖਰ &ChompingIndicator,
                                    ਬਿਨਾਨਿਸ਼ਾਨੀ &IndentIndicator, ਤਾਰਕਿਕ &Is_ਪੂਰਾ) {
  ਸਵੈਚਾਲਿਤ ਸ਼ੁਰੂ = Current;

  ChompingIndicator = ਜਾਂਚ_ਖੰਡ_Chomping_Indicator();
  IndentIndicator = ਜਾਂਚ_ਖੰਡ_Indentation_Indicator();
  // Check for the chomping indicator once again.
  ਜੇ (ChompingIndicator == ' ')
    ChompingIndicator = ਜਾਂਚ_ਖੰਡ_Chomping_Indicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  ਜੇ (Current == ਸਿਰਾ_2) { // EOF, we have an empty scalar.
    Token T;
    T.ਕਿਸਮ_3 = Token::TK_ਖੰਡ_Scalar;
    T.Range = StringRef(ਸ਼ੁਰੂ, Current - ਸ਼ੁਰੂ);
    Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);
    Is_ਪੂਰਾ = ਸੱਚ;
    ਵਾਪਸ ਸੱਚ;
  }

  ਜੇ (!consume_ਸਤਰ_Break_If_Present()) {
    ਸੈੱਟ_Error("Expected a line break after block scalar header", Current);
    ਵਾਪਸ ਝੂਠ;
  }
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::ਲੱਭੋ_ਖੰਡ_Scalar_Indent(ਬਿਨਾਨਿਸ਼ਾਨੀ &ਖੰਡ_Indent,
                                    ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੰਡ_Exit_Indent,
                                    ਬਿਨਾਨਿਸ਼ਾਨੀ &ਸਤਰ_Breaks, ਤਾਰਕਿਕ &Is_ਪੂਰਾ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ Max_All_Space_ਸਤਰ_Characters = 0;
  StringRef::iterator Longest_All_Space_ਸਤਰ;

  ਜਦੋਂ (ਸੱਚ) {
    advanceWhile(&Scanner::skip_s_space);
    ਜੇ (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      ਜੇ (ਕਾਲਮ <= ਖੰਡ_Exit_Indent) { // End of the block literal.
        Is_ਪੂਰਾ = ਸੱਚ;
        ਵਾਪਸ ਸੱਚ;
      }
      // We found the block's indentation.
      ਖੰਡ_Indent = ਕਾਲਮ;
      ਜੇ (Max_All_Space_ਸਤਰ_Characters > ਖੰਡ_Indent) {
        ਸੈੱਟ_Error(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_ਸਤਰ);
        ਵਾਪਸ ਝੂਠ;
      }
      ਵਾਪਸ ਸੱਚ;
    }
    ਜੇ (skip_b_break(Current) != Current &&
        ਕਾਲਮ > Max_All_Space_ਸਤਰ_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_ਸਤਰ_Characters = ਕਾਲਮ;
      Longest_All_Space_ਸਤਰ = Current;
    }

    // Check for EOF.
    ਜੇ (Current == ਸਿਰਾ_2) {
      Is_ਪੂਰਾ = ਸੱਚ;
      ਵਾਪਸ ਸੱਚ;
    }

    ਜੇ (!consume_ਸਤਰ_Break_If_Present()) {
      Is_ਪੂਰਾ = ਸੱਚ;
      ਵਾਪਸ ਸੱਚ;
    }
    ++ਸਤਰ_Breaks;
  }
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_ਖੰਡ_Scalar_Indent(ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੰਡ_Indent,
                                    ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੰਡ_Exit_Indent, ਤਾਰਕਿਕ &Is_ਪੂਰਾ) {
  // Skip the indentation.
  ਜਦੋਂ (ਕਾਲਮ < ਖੰਡ_Indent) {
    ਸਵੈਚਾਲਿਤ I = skip_s_space(Current);
    ਜੇ (I == Current)
      ਰੋਕੋ;
    Current = I;
    ++ਕਾਲਮ;
  }

  ਜੇ (skip_nb_char(Current) == Current)
    ਵਾਪਸ ਸੱਚ;

  ਜੇ (ਕਾਲਮ <= ਖੰਡ_Exit_Indent) { // End of the block literal.
    Is_ਪੂਰਾ = ਸੱਚ;
    ਵਾਪਸ ਸੱਚ;
  }

  ਜੇ (ਕਾਲਮ < ਖੰਡ_Indent) {
    ਜੇ (Current != ਸਿਰਾ_2 && *Current == '#') { // Trailing comment.
      Is_ਪੂਰਾ = ਸੱਚ;
      ਵਾਪਸ ਸੱਚ;
    }
    ਸੈੱਟ_Error("A text line is less indented than the block scalar", Current);
    ਵਾਪਸ ਝੂਠ;
  }
  ਵਾਪਸ ਸੱਚ; // A normal text line.
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_ਖੰਡ_Scalar(ਤਾਰਕਿਕ IsLiteral) {
  ਪ੍ਰਤਿਜ੍ਞਾ(*Current == '|' || *Current == '>');
  ਅੱਖਰ StyleIndicator;
  ਅੱਖਰ ChompingIndicator;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੰਡ_Indent;
  ਤਾਰਕਿਕ Is_ਪੂਰਾ = ਝੂਠ;
  ਜੇ (!ਜਾਂਚ_ਖੰਡ_Scalar_Indicators(StyleIndicator, ChompingIndicator, ਖੰਡ_Indent,
                                 Is_ਪੂਰਾ))
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (Is_ਪੂਰਾ)
    ਵਾਪਸ ਸੱਚ;
  ਤਾਰਕਿਕ IsFolded = StyleIndicator == '>';

  ਅਚਲ ਸਵੈਚਾਲਿਤ *ਸ਼ੁਰੂ = Current;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੰਡ_Exit_Indent = Indent < 0 ? 0 : (ਬਿਨਾਨਿਸ਼ਾਨੀ)Indent;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਤਰ_Breaks = 0;
  ਜੇ (ਖੰਡ_Indent == 0) {
    ਜੇ (!ਲੱਭੋ_ਖੰਡ_Scalar_Indent(ਖੰਡ_Indent, ਖੰਡ_Exit_Indent, ਸਤਰ_Breaks,
                               Is_ਪੂਰਾ))
      ਵਾਪਸ ਝੂਠ;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  ਜਦੋਂ (!Is_ਪੂਰਾ) {
    ਜੇ (!ਜਾਂਚ_ਖੰਡ_Scalar_Indent(ਖੰਡ_Indent, ਖੰਡ_Exit_Indent, Is_ਪੂਰਾ))
      ਵਾਪਸ ਝੂਠ;
    ਜੇ (Is_ਪੂਰਾ)
      ਰੋਕੋ;

    // Parse the current line.
    ਸਵੈਚਾਲਿਤ ਸਤਰ_ਸ਼ੁਰੂ = Current;
    advanceWhile(&Scanner::skip_nb_char);
    ਜੇ (ਸਤਰ_ਸ਼ੁਰੂ != Current) {
      ਜੇ (ਸਤਰ_Breaks && IsFolded && !Scanner::is_ਸਤਰ_ਖਾਲੀ_ਥਾਵਾਂ(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        ਜੇ (ਸਤਰ_Breaks == 1) {
          Str.append(ਸਤਰ_Breaks,
                     is_ਸਤਰ_ਖਾਲੀ_ਥਾਵਾਂ(StringRef(ਸਤਰ_ਸ਼ੁਰੂ, Current - ਸਤਰ_ਸ਼ੁਰੂ))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        ਸਤਰ_Breaks--;
      }
      Str.append(ਸਤਰ_Breaks, '\n');
      Str.append(StringRef(ਸਤਰ_ਸ਼ੁਰੂ, Current - ਸਤਰ_ਸ਼ੁਰੂ));
      ਸਤਰ_Breaks = 0;
    }

    // Check for EOF.
    ਜੇ (Current == ਸਿਰਾ_2)
      ਰੋਕੋ;

    ਜੇ (!consume_ਸਤਰ_Break_If_Present())
      ਰੋਕੋ;
    ++ਸਤਰ_Breaks;
  }

  ਜੇ (Current == ਸਿਰਾ_2 && !ਸਤਰ_Breaks)
    // Ensure that there is at least one line break before the end of file.
    ਸਤਰ_Breaks = 1;
  Str.append(get_Chomped_ਸਤਰ_Breaks(ChompingIndicator, ਸਤਰ_Breaks, Str), '\n');

  // New lines may start a simple key.
  ਜੇ (!FlowLevel)
    Is_Simple_ਕੁੰਜੀ_Allowed = ਸੱਚ;
  Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow = ਝੂਠ;

  Token T;
  T.ਕਿਸਮ_3 = Token::TK_ਖੰਡ_Scalar;
  T.Range = StringRef(ਸ਼ੁਰੂ, Current - ਸ਼ੁਰੂ);
  T.ਮੁੱਲ = std::string(Str);
  Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);
  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::ਜਾਂਚ_ਟੈਗ() {
  StringRef::iterator ਸ਼ੁਰੂ = Current;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਾਨਾ_ਸ਼ੁਰੂ = ਕਾਲਮ;
  skip(1); // Eat !.
  ਜੇ (Current == ਸਿਰਾ_2 || isBlankOrBreak(Current)); // An empty tag.
  ਨਹੀਂਤਾਂ ਜੇ (*Current == '<') {
    skip(1);
    ਜਾਂਚ_ns_uri_char();
    ਜੇ (!consume('>'))
      ਵਾਪਸ ਝੂਠ;
  } ਨਹੀਂਤਾਂ {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.ਕਿਸਮ_3 = Token::TK_ਟੈਗ;
  T.Range = StringRef(ਸ਼ੁਰੂ, Current - ਸ਼ੁਰੂ);
  Token_ਕਤਾਰ.ਧੱਕੋ_ਪਿੱਠ(T);

  // Tags can be simple keys.
  save_Simple_ਕੁੰਜੀ_Candidate(--Token_ਕਤਾਰ.ਸਿਰਾ(), ਖਾਨਾ_ਸ਼ੁਰੂ, ਝੂਠ);

  Is_Simple_ਕੁੰਜੀ_Allowed = ਝੂਠ;
  Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow = ਝੂਠ;

  ਵਾਪਸ ਸੱਚ;
}

ਤਾਰਕਿਕ Scanner::fetch_ਹੋਰ_ਟੋਕਨ() {
  ਜੇ (Is_ਸ਼ੁਰੂ_ਦਾ_ਧਾਰਾ)
    ਵਾਪਸ ਜਾਂਚ_ਧਾਰਾ_ਸ਼ੁਰੂ();

  ਜਾਂਚ_ਮੰਜ਼ਿਲ_ਅਗਲਾ_Token();

  ਜੇ (Current == ਸਿਰਾ_2)
    ਵਾਪਸ ਜਾਂਚ_ਧਾਰਾ_ਸਿਰਾ();

  remove_Stale_Simple_ਕੁੰਜੀ_Candidates();

  unrollIndent(ਕਾਲਮ);

  ਜੇ (ਕਾਲਮ == 0 && *Current == '%')
    ਵਾਪਸ ਜਾਂਚ_Directive();

  ਜੇ (ਕਾਲਮ == 0 && Current + 3 <= ਸਿਰਾ_2
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == ਸਿਰਾ_2 || isBlankOrBreak(Current + 3)))
    ਵਾਪਸ ਜਾਂਚ_Document_Indicator(ਸੱਚ);

  ਜੇ (ਕਾਲਮ == 0 && Current + 3 <= ਸਿਰਾ_2
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == ਸਿਰਾ_2 || isBlankOrBreak(Current + 3)))
    ਵਾਪਸ ਜਾਂਚ_Document_Indicator(ਝੂਠ);

  ਜੇ (*Current == '[')
    ਵਾਪਸ ਜਾਂਚ_Flow_Collection_ਸ਼ੁਰੂ(ਸੱਚ);

  ਜੇ (*Current == '{')
    ਵਾਪਸ ਜਾਂਚ_Flow_Collection_ਸ਼ੁਰੂ(ਝੂਠ);

  ਜੇ (*Current == ']')
    ਵਾਪਸ ਜਾਂਚ_Flow_Collection_ਸਿਰਾ(ਸੱਚ);

  ਜੇ (*Current == '}')
    ਵਾਪਸ ਜਾਂਚ_Flow_Collection_ਸਿਰਾ(ਝੂਠ);

  ਜੇ (*Current == ',')
    ਵਾਪਸ ਜਾਂਚ_Flow_ਇੰਦਰਾਜ();

  ਜੇ (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == ਸਿਰਾ_2))
    ਵਾਪਸ ਜਾਂਚ_ਖੰਡ_ਇੰਦਰਾਜ();

  ਜੇ (*Current == '?' && (Current + 1 == ਸਿਰਾ_2 || isBlankOrBreak(Current + 1)))
    ਵਾਪਸ ਜਾਂਚ_ਕੁੰਜੀ();

  ਜੇ (*Current == ':' &&
      (!is_Plain_ਸੁਰੱਖਿਅਤ_Non_Blank(Current + 1) || Is_Adjacent_ਮੁੱਲ_Allowed_ਵਿੱਚ_Flow))
    ਵਾਪਸ ਜਾਂਚ_ਮੁੱਲ();

  ਜੇ (*Current == '*')
    ਵਾਪਸ ਜਾਂਚ_Alias_Or_Anchor(ਸੱਚ);

  ਜੇ (*Current == '&')
    ਵਾਪਸ ਜਾਂਚ_Alias_Or_Anchor(ਝੂਠ);

  ਜੇ (*Current == '!')
    ਵਾਪਸ ਜਾਂਚ_ਟੈਗ();

  ਜੇ (*Current == '|' && !FlowLevel)
    ਵਾਪਸ ਜਾਂਚ_ਖੰਡ_Scalar(ਸੱਚ);

  ਜੇ (*Current == '>' && !FlowLevel)
    ਵਾਪਸ ਜਾਂਚ_ਖੰਡ_Scalar(ਝੂਠ);

  ਜੇ (*Current == '\'')
    ਵਾਪਸ ਜਾਂਚ_Flow_Scalar(ਝੂਠ);

  ਜੇ (*Current == '"')
    ਵਾਪਸ ਜਾਂਚ_Flow_Scalar(ਸੱਚ);

  // Get a plain scalar.
  StringRef ਪਹਿਲਾ_Char(Current, 1);
  ਜੇ ((!isBlankOrBreak(Current) &&
       ਪਹਿਲਾ_Char.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (ਪਹਿਲਾ_Char.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ("?:-") != StringRef::npos &&
       is_Plain_ਸੁਰੱਖਿਅਤ_Non_Blank(Current + 1)))
    ਵਾਪਸ ਜਾਂਚ_Plain_Scalar();

  ਸੈੱਟ_Error("Unrecognized character while tokenizing.", Current);
  ਵਾਪਸ ਝੂਠ;
}

ਧਾਰਾ::ਧਾਰਾ(StringRef ਨਿਵੇਸ਼, ਸਰੋਤ_Mgr &SM, ਤਾਰਕਿਕ ShowColors,
               std::error_ਕੋਡ *EC)
    : scanner(ਨਵਾਂ Scanner(ਨਿਵੇਸ਼, SM, ShowColors, EC)) {}

ਧਾਰਾ::ਧਾਰਾ(ਸਮ੍ਰਿਤੀ_ਬਫਰ_Ref ਨਿਵੇਸ਼_ਬਫਰ, ਸਰੋਤ_Mgr &SM, ਤਾਰਕਿਕ ShowColors,
               std::error_ਕੋਡ *EC)
    : scanner(ਨਵਾਂ Scanner(ਨਿਵੇਸ਼_ਬਫਰ, SM, ShowColors, EC)) {}

ਧਾਰਾ::~ਧਾਰਾ() = ਮੂਲ;

ਤਾਰਕਿਕ ਧਾਰਾ::failed() { ਵਾਪਸ scanner->failed(); }

ਖਾਲੀ ਧਾਰਾ::printError(ਗੰਢ *N, ਅਚਲ Twine &ਸੁਨੇਹਾ_4, ਸਰੋਤ_Mgr::Diag_ਕਿਸਮ ਕਿਸਮ_3) {
  printError(N ? N->get_ਸਰੋਤ_Range() : SMRange(), ਸੁਨੇਹਾ_4, ਕਿਸਮ_3);
}

ਖਾਲੀ ਧਾਰਾ::printError(ਅਚਲ SMRange &Range, ਅਚਲ Twine &ਸੁਨੇਹਾ_4,
                        ਸਰੋਤ_Mgr::Diag_ਕਿਸਮ ਕਿਸਮ_3) {
  scanner->printError(Range.ਸ਼ੁਰੂ, ਕਿਸਮ_3, ਸੁਨੇਹਾ_4, Range);
}

document_iterator ਧਾਰਾ::begin() {
  ਜੇ (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->get_ਅਗਲਾ();

  CurrentDoc.reset(ਨਵਾਂ Document(*ਇਹ));
  ਵਾਪਸ document_iterator(CurrentDoc);
}

document_iterator ਧਾਰਾ::ਸਿਰਾ() {
  ਵਾਪਸ document_iterator();
}

ਖਾਲੀ ਧਾਰਾ::skip() {
  ਲਈ (Document &Doc : *ਇਹ)
    Doc.skip();
}

ਗੰਢ::ਗੰਢ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), ਟੈਗ(T) {
  SMLoc ਸ਼ੁਰੂ = SMLoc::get_From_ਸੂਚਕ(peek_ਅਗਲਾ().Range.begin());
  ਸਰੋਤ_Range = SMRange(ਸ਼ੁਰੂ, ਸ਼ੁਰੂ);
}

std::string ਗੰਢ::get_Verbatim_ਟੈਗ() ਅਚਲ {
  StringRef Raw = get_Raw_ਟੈਗ();
  ਜੇ (!Raw.ਖਾਲੀ_ਥਾਵਾਂ() && Raw != "!") {
    std::string Ret;
    ਜੇ (Raw.ਲੱਭੋ_ਆਖਰੀ_ਦਾ('!') == 0) {
      Ret = std::string(Doc->get_ਟੈਗ_Map().ਲੱਭੋ("!")->ਦੂਜਾ_2);
      Ret += Raw.substr(1);
      ਵਾਪਸ Ret;
    } ਨਹੀਂਤਾਂ ਜੇ (Raw.starts_with("!!")) {
      Ret = std::string(Doc->get_ਟੈਗ_Map().ਲੱਭੋ("!!")->ਦੂਜਾ_2);
      Ret += Raw.substr(2);
      ਵਾਪਸ Ret;
    } ਨਹੀਂਤਾਂ {
      StringRef ਟੈਗ_Handle = Raw.substr(0, Raw.ਲੱਭੋ_ਆਖਰੀ_ਦਾ('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->get_ਟੈਗ_Map().ਲੱਭੋ(ਟੈਗ_Handle);
      ਜੇ (It != Doc->get_ਟੈਗ_Map().ਸਿਰਾ())
        Ret = std::string(It->ਦੂਜਾ_2);
      ਨਹੀਂਤਾਂ {
        Token T;
        T.ਕਿਸਮ_3 = Token::TK_ਟੈਗ;
        T.Range = ਟੈਗ_Handle;
        ਸੈੱਟ_Error(Twine("Unknown tag handle ") + ਟੈਗ_Handle, T);
      }
      Ret += Raw.substr(Raw.ਲੱਭੋ_ਆਖਰੀ_ਦਾ('!') + 1);
      ਵਾਪਸ Ret;
    }
  }

  ਚੋਣ (getType()) {
  ਹਾਲਤ NK_Null:
    ਵਾਪਸ "tag:yaml.org,2002:null";
  ਹਾਲਤ NK_Scalar:
  ਹਾਲਤ NK_ਖੰਡ_Scalar:
    // TODO: Tag resolution.
    ਵਾਪਸ "tag:yaml.org,2002:str";
  ਹਾਲਤ NK_Mapping:
    ਵਾਪਸ "tag:yaml.org,2002:map";
  ਹਾਲਤ NK_Sequence:
    ਵਾਪਸ "tag:yaml.org,2002:seq";
  }

  ਵਾਪਸ "";
}

Token &ਗੰਢ::peek_ਅਗਲਾ() {
  ਵਾਪਸ Doc->peek_ਅਗਲਾ();
}

Token ਗੰਢ::get_ਅਗਲਾ() {
  ਵਾਪਸ Doc->get_ਅਗਲਾ();
}

ਗੰਢ *ਗੰਢ::parse_ਖੰਡ_ਗੰਢ() {
  ਵਾਪਸ Doc->parse_ਖੰਡ_ਗੰਢ();
}

BumpPtrAllocator &ਗੰਢ::getAllocator() {
  ਵਾਪਸ Doc->ਗੰਢ_Allocator;
}

ਖਾਲੀ ਗੰਢ::ਸੈੱਟ_Error(ਅਚਲ Twine &ਸੁਨੇਹਾ_4, Token &Tok) ਅਚਲ {
  Doc->ਸੈੱਟ_Error(ਸੁਨੇਹਾ_4, Tok);
}

ਤਾਰਕਿਕ ਗੰਢ::failed() ਅਚਲ {
  ਵਾਪਸ Doc->failed();
}

StringRef Scalar_ਗੰਢ::get_ਮੁੱਲ(Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Storage) ਅਚਲ {
  ਜੇ (ਮੁੱਲ[0] == '"')
    ਵਾਪਸ get_Double_Quoted_ਮੁੱਲ(ਮੁੱਲ, Storage);
  ਜੇ (ਮੁੱਲ[0] == '\'')
    ਵਾਪਸ get_Single_Quoted_ਮੁੱਲ(ਮੁੱਲ, Storage);
  ਵਾਪਸ get_Plain_ਮੁੱਲ(ਮੁੱਲ, Storage);
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
ਸਥਿਰ StringRef
parse_Scalar_ਮੁੱਲ(StringRef Unquoted_ਮੁੱਲ, Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_ਮੁੱਲ.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(LookupChars);
  ਜੇ (I == StringRef::npos)
    ਵਾਪਸ Unquoted_ਮੁੱਲ;

  Storage.clear();
  Storage.reserve(Unquoted_ਮੁੱਲ.ਮਾਪ_2());
  ਅੱਖਰ ਆਖਰੀ_New_ਸਤਰ_Added_As = '\0';
  ਲਈ (; I != StringRef::npos; I = Unquoted_ਮੁੱਲ.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(LookupChars)) {
    ਜੇ (Unquoted_ਮੁੱਲ[I] != '\r' && Unquoted_ਮੁੱਲ[I] != '\n') {
      llvm::append_range(Storage, Unquoted_ਮੁੱਲ.take_front(I));
      Unquoted_ਮੁੱਲ = UnescapeCallback(Unquoted_ਮੁੱਲ.drop_front(I), Storage);
      ਆਖਰੀ_New_ਸਤਰ_Added_As = '\0';
      ਜਾਰੀ;
    }
    ਜੇ (size_t ਆਖਰੀ_Non_SWhite = Unquoted_ਮੁੱਲ.ਲੱਭੋ_ਆਖਰੀ_not_ਦਾ(" \t", I);
        ਆਖਰੀ_Non_SWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_ਮੁੱਲ.take_front(ਆਖਰੀ_Non_SWhite + 1));
      Storage.ਧੱਕੋ_ਪਿੱਠ(' ');
      ਆਖਰੀ_New_ਸਤਰ_Added_As = ' ';
    } ਨਹੀਂਤਾਂ {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      ਚੋਣ (ਆਖਰੀ_New_ਸਤਰ_Added_As) {
      ਹਾਲਤ ' ':
        ਪ੍ਰਤਿਜ੍ਞਾ(!Storage.ਖਾਲੀ_ਥਾਵਾਂ() && Storage.ਪਿੱਠ() == ' ');
        Storage.ਪਿੱਠ() = '\n';
        ਆਖਰੀ_New_ਸਤਰ_Added_As = '\n';
        ਰੋਕੋ;
      ਹਾਲਤ '\n':
        ਪ੍ਰਤਿਜ੍ਞਾ(!Storage.ਖਾਲੀ_ਥਾਵਾਂ() && Storage.ਪਿੱਠ() == '\n');
        Storage.ਧੱਕੋ_ਪਿੱਠ('\n');
        ਰੋਕੋ;
      ਮੂਲ:
        Storage.ਧੱਕੋ_ਪਿੱਠ(' ');
        ਆਖਰੀ_New_ਸਤਰ_Added_As = ' ';
        ਰੋਕੋ;
      }
    }
    // Handle Windows-style EOL
    ਜੇ (Unquoted_ਮੁੱਲ.substr(I, 2) == "\r\n")
      I++;
    Unquoted_ਮੁੱਲ = Unquoted_ਮੁੱਲ.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_ਮੁੱਲ);
  ਵਾਪਸ StringRef(Storage.begin(), Storage.ਮਾਪ_2());
}

StringRef
Scalar_ਗੰਢ::get_Double_Quoted_ਮੁੱਲ(StringRef Raw_ਮੁੱਲ,
                                 Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Storage) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(Raw_ਮੁੱਲ.ਮਾਪ_2() >= 2 && Raw_ਮੁੱਲ.front() == '"' &&
         Raw_ਮੁੱਲ.ਪਿੱਠ() == '"');
  StringRef Unquoted_ਮੁੱਲ = Raw_ਮੁੱਲ.substr(1, Raw_ਮੁੱਲ.ਮਾਪ_2() - 2);

  ਸਵੈਚਾਲਿਤ UnescapeFunc = [ਇਹ](StringRef Unquoted_ਮੁੱਲ,
                             Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Storage) {
    ਪ੍ਰਤਿਜ੍ਞਾ(Unquoted_ਮੁੱਲ.take_front(1) == "\\");
    ਜੇ (Unquoted_ਮੁੱਲ.ਮਾਪ_2() == 1) {
      Token T;
      T.Range = Unquoted_ਮੁੱਲ;
      ਸੈੱਟ_Error("Unrecognized escape code", T);
      Storage.clear();
      ਵਾਪਸ StringRef();
    }
    Unquoted_ਮੁੱਲ = Unquoted_ਮੁੱਲ.drop_front(1);
    ਚੋਣ (Unquoted_ਮੁੱਲ[0]) {
    ਮੂਲ: {
      Token T;
      T.Range = Unquoted_ਮੁੱਲ.take_front(1);
      ਸੈੱਟ_Error("Unrecognized escape code", T);
      Storage.clear();
      ਵਾਪਸ StringRef();
    }
    ਹਾਲਤ '\r':
      // Shrink the Windows-style EOL.
      ਜੇ (Unquoted_ਮੁੱਲ.ਮਾਪ_2() >= 2 && Unquoted_ਮੁੱਲ[1] == '\n')
        Unquoted_ਮੁੱਲ = Unquoted_ਮੁੱਲ.drop_front(1);
      [[fallthrough]];
    ਹਾਲਤ '\n':
      ਵਾਪਸ Unquoted_ਮੁੱਲ.drop_front(1).ltrim(" \t");
    ਹਾਲਤ '0':
      Storage.ਧੱਕੋ_ਪਿੱਠ(0x00);
      ਰੋਕੋ;
    ਹਾਲਤ 'a':
      Storage.ਧੱਕੋ_ਪਿੱਠ(0x07);
      ਰੋਕੋ;
    ਹਾਲਤ 'b':
      Storage.ਧੱਕੋ_ਪਿੱਠ(0x08);
      ਰੋਕੋ;
    ਹਾਲਤ 't':
    ਹਾਲਤ 0x09:
      Storage.ਧੱਕੋ_ਪਿੱਠ(0x09);
      ਰੋਕੋ;
    ਹਾਲਤ 'n':
      Storage.ਧੱਕੋ_ਪਿੱਠ(0x0A);
      ਰੋਕੋ;
    ਹਾਲਤ 'v':
      Storage.ਧੱਕੋ_ਪਿੱਠ(0x0B);
      ਰੋਕੋ;
    ਹਾਲਤ 'f':
      Storage.ਧੱਕੋ_ਪਿੱਠ(0x0C);
      ਰੋਕੋ;
    ਹਾਲਤ 'r':
      Storage.ਧੱਕੋ_ਪਿੱਠ(0x0D);
      ਰੋਕੋ;
    ਹਾਲਤ 'e':
      Storage.ਧੱਕੋ_ਪਿੱਠ(0x1B);
      ਰੋਕੋ;
    ਹਾਲਤ ' ':
      Storage.ਧੱਕੋ_ਪਿੱਠ(0x20);
      ਰੋਕੋ;
    ਹਾਲਤ '"':
      Storage.ਧੱਕੋ_ਪਿੱਠ(0x22);
      ਰੋਕੋ;
    ਹਾਲਤ '/':
      Storage.ਧੱਕੋ_ਪਿੱਠ(0x2F);
      ਰੋਕੋ;
    ਹਾਲਤ '\\':
      Storage.ਧੱਕੋ_ਪਿੱਠ(0x5C);
      ਰੋਕੋ;
    ਹਾਲਤ 'N':
      encodeUTF8(0x85, Storage);
      ਰੋਕੋ;
    ਹਾਲਤ '_':
      encodeUTF8(0xA0, Storage);
      ਰੋਕੋ;
    ਹਾਲਤ 'L':
      encodeUTF8(0x2028, Storage);
      ਰੋਕੋ;
    ਹਾਲਤ 'P':
      encodeUTF8(0x2029, Storage);
      ਰੋਕੋ;
    ਹਾਲਤ 'x': {
      ਜੇ (Unquoted_ਮੁੱਲ.ਮਾਪ_2() < 3)
        // TODO: Report error.
        ਰੋਕੋ;
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ Unicode_Scalar_ਮੁੱਲ;
      ਜੇ (Unquoted_ਮੁੱਲ.substr(1, 2).getAsInteger(16, Unicode_Scalar_ਮੁੱਲ))
        // TODO: Report error.
        Unicode_Scalar_ਮੁੱਲ = 0xFFFD;
      encodeUTF8(Unicode_Scalar_ਮੁੱਲ, Storage);
      ਵਾਪਸ Unquoted_ਮੁੱਲ.drop_front(3);
    }
    ਹਾਲਤ 'u': {
      ਜੇ (Unquoted_ਮੁੱਲ.ਮਾਪ_2() < 5)
        // TODO: Report error.
        ਰੋਕੋ;
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ Unicode_Scalar_ਮੁੱਲ;
      ਜੇ (Unquoted_ਮੁੱਲ.substr(1, 4).getAsInteger(16, Unicode_Scalar_ਮੁੱਲ))
        // TODO: Report error.
        Unicode_Scalar_ਮੁੱਲ = 0xFFFD;
      encodeUTF8(Unicode_Scalar_ਮੁੱਲ, Storage);
      ਵਾਪਸ Unquoted_ਮੁੱਲ.drop_front(5);
    }
    ਹਾਲਤ 'U': {
      ਜੇ (Unquoted_ਮੁੱਲ.ਮਾਪ_2() < 9)
        // TODO: Report error.
        ਰੋਕੋ;
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ Unicode_Scalar_ਮੁੱਲ;
      ਜੇ (Unquoted_ਮੁੱਲ.substr(1, 8).getAsInteger(16, Unicode_Scalar_ਮੁੱਲ))
        // TODO: Report error.
        Unicode_Scalar_ਮੁੱਲ = 0xFFFD;
      encodeUTF8(Unicode_Scalar_ਮੁੱਲ, Storage);
      ਵਾਪਸ Unquoted_ਮੁੱਲ.drop_front(9);
    }
    }
    ਵਾਪਸ Unquoted_ਮੁੱਲ.drop_front(1);
  };

  ਵਾਪਸ parse_Scalar_ਮੁੱਲ(Unquoted_ਮੁੱਲ, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_ਗੰਢ::get_Single_Quoted_ਮੁੱਲ(StringRef Raw_ਮੁੱਲ,
                                           Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Storage) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Raw_ਮੁੱਲ.ਮਾਪ_2() >= 2 && Raw_ਮੁੱਲ.front() == '\'' &&
         Raw_ਮੁੱਲ.ਪਿੱਠ() == '\'');
  StringRef Unquoted_ਮੁੱਲ = Raw_ਮੁੱਲ.substr(1, Raw_ਮੁੱਲ.ਮਾਪ_2() - 2);

  ਸਵੈਚਾਲਿਤ UnescapeFunc = [](StringRef Unquoted_ਮੁੱਲ,
                         Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Storage) {
    ਪ੍ਰਤਿਜ੍ਞਾ(Unquoted_ਮੁੱਲ.take_front(2) == "''");
    Storage.ਧੱਕੋ_ਪਿੱਠ('\'');
    ਵਾਪਸ Unquoted_ਮੁੱਲ.drop_front(2);
  };

  ਵਾਪਸ parse_Scalar_ਮੁੱਲ(Unquoted_ਮੁੱਲ, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_ਗੰਢ::get_Plain_ਮੁੱਲ(StringRef Raw_ਮੁੱਲ,
                                    Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_ਮੁੱਲ = Raw_ਮੁੱਲ.rtrim("\r\n \t");
  ਵਾਪਸ parse_Scalar_ਮੁੱਲ(Raw_ਮੁੱਲ, Storage, "\r\n", ਖਾਲੀ_ਸੂਚਕ);
}

ਗੰਢ *ਕੁੰਜੀ_ਮੁੱਲ_ਗੰਢ::get_ਕੁੰਜੀ() {
  ਜੇ (ਕੁੰਜੀ)
    ਵਾਪਸ ਕੁੰਜੀ;
  // Handle implicit null keys.
  {
    Token &t = peek_ਅਗਲਾ();
    ਜੇ (   t.ਕਿਸਮ_3 == Token::TK_ਖੰਡ_ਸਿਰਾ
        || t.ਕਿਸਮ_3 == Token::TK_ਮੁੱਲ
        || t.ਕਿਸਮ_3 == Token::TK_Error) {
      ਵਾਪਸ ਕੁੰਜੀ = ਨਵਾਂ (getAllocator()) Null_ਗੰਢ(Doc);
    }
    ਜੇ (t.ਕਿਸਮ_3 == Token::TK_ਕੁੰਜੀ)
      get_ਅਗਲਾ(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peek_ਅਗਲਾ();
  ਜੇ (t.ਕਿਸਮ_3 == Token::TK_ਖੰਡ_ਸਿਰਾ || t.ਕਿਸਮ_3 == Token::TK_ਮੁੱਲ) {
    ਵਾਪਸ ਕੁੰਜੀ = ਨਵਾਂ (getAllocator()) Null_ਗੰਢ(Doc);
  }

  // We've got a normal key.
  ਵਾਪਸ ਕੁੰਜੀ = parse_ਖੰਡ_ਗੰਢ();
}

ਗੰਢ *ਕੁੰਜੀ_ਮੁੱਲ_ਗੰਢ::get_ਮੁੱਲ() {
  ਜੇ (ਮੁੱਲ)
    ਵਾਪਸ ਮੁੱਲ;

  ਜੇ (ਗੰਢ* ਕੁੰਜੀ = get_ਕੁੰਜੀ())
    ਕੁੰਜੀ->skip();
  ਨਹੀਂਤਾਂ {
    ਸੈੱਟ_Error("Null key in Key Value.", peek_ਅਗਲਾ());
    ਵਾਪਸ ਮੁੱਲ = ਨਵਾਂ (getAllocator()) Null_ਗੰਢ(Doc);
  }

  ਜੇ (failed())
    ਵਾਪਸ ਮੁੱਲ = ਨਵਾਂ (getAllocator()) Null_ਗੰਢ(Doc);

  // Handle implicit null values.
  {
    Token &t = peek_ਅਗਲਾ();
    ਜੇ (   t.ਕਿਸਮ_3 == Token::TK_ਖੰਡ_ਸਿਰਾ
        || t.ਕਿਸਮ_3 == Token::TK_Flow_Mapping_ਸਿਰਾ
        || t.ਕਿਸਮ_3 == Token::TK_ਕੁੰਜੀ
        || t.ਕਿਸਮ_3 == Token::TK_Flow_ਇੰਦਰਾਜ
        || t.ਕਿਸਮ_3 == Token::TK_Error) {
      ਵਾਪਸ ਮੁੱਲ = ਨਵਾਂ (getAllocator()) Null_ਗੰਢ(Doc);
    }

    ਜੇ (t.ਕਿਸਮ_3 != Token::TK_ਮੁੱਲ) {
      ਸੈੱਟ_Error("Unexpected token in Key Value.", t);
      ਵਾਪਸ ਮੁੱਲ = ਨਵਾਂ (getAllocator()) Null_ਗੰਢ(Doc);
    }
    get_ਅਗਲਾ(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peek_ਅਗਲਾ();
  ਜੇ (t.ਕਿਸਮ_3 == Token::TK_ਖੰਡ_ਸਿਰਾ || t.ਕਿਸਮ_3 == Token::TK_ਕੁੰਜੀ) {
    ਵਾਪਸ ਮੁੱਲ = ਨਵਾਂ (getAllocator()) Null_ਗੰਢ(Doc);
  }

  // We got a normal value.
  ਵਾਪਸ ਮੁੱਲ = parse_ਖੰਡ_ਗੰਢ();
}

ਖਾਲੀ Mapping_ਗੰਢ::increment() {
  ਜੇ (failed()) {
    Is_At_ਸਿਰਾ = ਸੱਚ;
    Current_ਇੰਦਰਾਜ = ਖਾਲੀ_ਸੂਚਕ;
    ਵਾਪਸ;
  }
  ਜੇ (Current_ਇੰਦਰਾਜ) {
    Current_ਇੰਦਰਾਜ->skip();
    ਜੇ (Type == MT_Inline) {
      Is_At_ਸਿਰਾ = ਸੱਚ;
      Current_ਇੰਦਰਾਜ = ਖਾਲੀ_ਸੂਚਕ;
      ਵਾਪਸ;
    }
  }
  Token T = peek_ਅਗਲਾ();
  ਜੇ (T.ਕਿਸਮ_3 == Token::TK_ਕੁੰਜੀ || T.ਕਿਸਮ_3 == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    Current_ਇੰਦਰਾਜ = ਨਵਾਂ (getAllocator()) ਕੁੰਜੀ_ਮੁੱਲ_ਗੰਢ(Doc);
  } ਨਹੀਂਤਾਂ ਜੇ (Type == MT_ਖੰਡ) {
    ਚੋਣ (T.ਕਿਸਮ_3) {
    ਹਾਲਤ Token::TK_ਖੰਡ_ਸਿਰਾ:
      get_ਅਗਲਾ();
      Is_At_ਸਿਰਾ = ਸੱਚ;
      Current_ਇੰਦਰਾਜ = ਖਾਲੀ_ਸੂਚਕ;
      ਰੋਕੋ;
    ਮੂਲ:
      ਸੈੱਟ_Error("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    ਹਾਲਤ Token::TK_Error:
      Is_At_ਸਿਰਾ = ਸੱਚ;
      Current_ਇੰਦਰਾਜ = ਖਾਲੀ_ਸੂਚਕ;
    }
  } ਨਹੀਂਤਾਂ {
    ਚੋਣ (T.ਕਿਸਮ_3) {
    ਹਾਲਤ Token::TK_Flow_ਇੰਦਰਾਜ:
      // Eat the flow entry and recurse.
      get_ਅਗਲਾ();
      ਵਾਪਸ increment();
    ਹਾਲਤ Token::TK_Flow_Mapping_ਸਿਰਾ:
      get_ਅਗਲਾ();
      [[fallthrough]];
    ਹਾਲਤ Token::TK_Error:
      // Set this to end iterator.
      Is_At_ਸਿਰਾ = ਸੱਚ;
      Current_ਇੰਦਰਾਜ = ਖਾਲੀ_ਸੂਚਕ;
      ਰੋਕੋ;
    ਮੂਲ:
      ਸੈੱਟ_Error( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      Is_At_ਸਿਰਾ = ਸੱਚ;
      Current_ਇੰਦਰਾਜ = ਖਾਲੀ_ਸੂਚਕ;
    }
  }
}

ਖਾਲੀ Sequence_ਗੰਢ::increment() {
  ਜੇ (failed()) {
    Is_At_ਸਿਰਾ = ਸੱਚ;
    Current_ਇੰਦਰਾਜ = ਖਾਲੀ_ਸੂਚਕ;
    ਵਾਪਸ;
  }
  ਜੇ (Current_ਇੰਦਰਾਜ)
    Current_ਇੰਦਰਾਜ->skip();
  Token T = peek_ਅਗਲਾ();
  ਜੇ (SeqType == ST_ਖੰਡ) {
    ਚੋਣ (T.ਕਿਸਮ_3) {
    ਹਾਲਤ Token::TK_ਖੰਡ_ਇੰਦਰਾਜ:
      get_ਅਗਲਾ();
      Current_ਇੰਦਰਾਜ = parse_ਖੰਡ_ਗੰਢ();
      ਜੇ (!Current_ਇੰਦਰਾਜ) { // An error occurred.
        Is_At_ਸਿਰਾ = ਸੱਚ;
        Current_ਇੰਦਰਾਜ = ਖਾਲੀ_ਸੂਚਕ;
      }
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_ਖੰਡ_ਸਿਰਾ:
      get_ਅਗਲਾ();
      Is_At_ਸਿਰਾ = ਸੱਚ;
      Current_ਇੰਦਰਾਜ = ਖਾਲੀ_ਸੂਚਕ;
      ਰੋਕੋ;
    ਮੂਲ:
      ਸੈੱਟ_Error( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    ਹਾਲਤ Token::TK_Error:
      Is_At_ਸਿਰਾ = ਸੱਚ;
      Current_ਇੰਦਰਾਜ = ਖਾਲੀ_ਸੂਚਕ;
    }
  } ਨਹੀਂਤਾਂ ਜੇ (SeqType == ST_Indentless) {
    ਚੋਣ (T.ਕਿਸਮ_3) {
    ਹਾਲਤ Token::TK_ਖੰਡ_ਇੰਦਰਾਜ:
      get_ਅਗਲਾ();
      Current_ਇੰਦਰਾਜ = parse_ਖੰਡ_ਗੰਢ();
      ਜੇ (!Current_ਇੰਦਰਾਜ) { // An error occurred.
        Is_At_ਸਿਰਾ = ਸੱਚ;
        Current_ਇੰਦਰਾਜ = ਖਾਲੀ_ਸੂਚਕ;
      }
      ਰੋਕੋ;
    ਮੂਲ:
    ਹਾਲਤ Token::TK_Error:
      Is_At_ਸਿਰਾ = ਸੱਚ;
      Current_ਇੰਦਰਾਜ = ਖਾਲੀ_ਸੂਚਕ;
    }
  } ਨਹੀਂਤਾਂ ਜੇ (SeqType == ST_Flow) {
    ਚੋਣ (T.ਕਿਸਮ_3) {
    ਹਾਲਤ Token::TK_Flow_ਇੰਦਰਾਜ:
      // Eat the flow entry and recurse.
      get_ਅਗਲਾ();
      Was_ਪਿਛਲਾ_Token_Flow_ਇੰਦਰਾਜ = ਸੱਚ;
      ਵਾਪਸ increment();
    ਹਾਲਤ Token::TK_Flow_Sequence_ਸਿਰਾ:
      get_ਅਗਲਾ();
      [[fallthrough]];
    ਹਾਲਤ Token::TK_Error:
      // Set this to end iterator.
      Is_At_ਸਿਰਾ = ਸੱਚ;
      Current_ਇੰਦਰਾਜ = ਖਾਲੀ_ਸੂਚਕ;
      ਰੋਕੋ;
    ਹਾਲਤ Token::TK_ਧਾਰਾ_ਸਿਰਾ:
    ਹਾਲਤ Token::TK_Document_ਸਿਰਾ:
    ਹਾਲਤ Token::TK_Document_ਸ਼ੁਰੂ:
      ਸੈੱਟ_Error("Could not find closing ]!", T);
      // Set this to end iterator.
      Is_At_ਸਿਰਾ = ਸੱਚ;
      Current_ਇੰਦਰਾਜ = ਖਾਲੀ_ਸੂਚਕ;
      ਰੋਕੋ;
    ਮੂਲ:
      ਜੇ (!Was_ਪਿਛਲਾ_Token_Flow_ਇੰਦਰਾਜ) {
        ਸੈੱਟ_Error("Expected , between entries!", T);
        Is_At_ਸਿਰਾ = ਸੱਚ;
        Current_ਇੰਦਰਾਜ = ਖਾਲੀ_ਸੂਚਕ;
        ਰੋਕੋ;
      }
      // Otherwise it must be a flow entry.
      Current_ਇੰਦਰਾਜ = parse_ਖੰਡ_ਗੰਢ();
      ਜੇ (!Current_ਇੰਦਰਾਜ) {
        Is_At_ਸਿਰਾ = ਸੱਚ;
      }
      Was_ਪਿਛਲਾ_Token_Flow_ਇੰਦਰਾਜ = ਝੂਠ;
      ਰੋਕੋ;
    }
  }
}

Document::Document(ਧਾਰਾ &S) : ਧਾਰਾ_2(S), ਜੜ੍ਹ_2(ਖਾਲੀ_ਸੂਚਕ) {
  // Tag maps starts with two default mappings.
  ਟੈਗ_Map["!"] = "!";
  ਟੈਗ_Map["!!"] = "tag:yaml.org,2002:";

  ਜੇ (parseDirectives())
    expectToken(Token::TK_Document_ਸ਼ੁਰੂ);
  Token &T = peek_ਅਗਲਾ();
  ਜੇ (T.ਕਿਸਮ_3 == Token::TK_Document_ਸ਼ੁਰੂ)
    get_ਅਗਲਾ();
}

ਤਾਰਕਿਕ Document::skip()  {
  ਜੇ (ਧਾਰਾ_2.scanner->failed())
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (!ਜੜ੍ਹ_2 && !get_ਜੜ੍ਹ())
    ਵਾਪਸ ਝੂਠ;
  ਜੜ੍ਹ_2->skip();
  Token &T = peek_ਅਗਲਾ();
  ਜੇ (T.ਕਿਸਮ_3 == Token::TK_ਧਾਰਾ_ਸਿਰਾ)
    ਵਾਪਸ ਝੂਠ;
  ਜੇ (T.ਕਿਸਮ_3 == Token::TK_Document_ਸਿਰਾ) {
    get_ਅਗਲਾ();
    ਵਾਪਸ skip();
  }
  ਵਾਪਸ ਸੱਚ;
}

Token &Document::peek_ਅਗਲਾ() {
  ਵਾਪਸ ਧਾਰਾ_2.scanner->peek_ਅਗਲਾ();
}

Token Document::get_ਅਗਲਾ() {
  ਵਾਪਸ ਧਾਰਾ_2.scanner->get_ਅਗਲਾ();
}

ਖਾਲੀ Document::ਸੈੱਟ_Error(ਅਚਲ Twine &ਸੁਨੇਹਾ_3, Token &Location) ਅਚਲ {
  ਧਾਰਾ_2.scanner->ਸੈੱਟ_Error(ਸੁਨੇਹਾ_3, Location.Range.begin());
}

ਤਾਰਕਿਕ Document::failed() ਅਚਲ {
  ਵਾਪਸ ਧਾਰਾ_2.scanner->failed();
}

ਗੰਢ *Document::parse_ਖੰਡ_ਗੰਢ() {
  Token T = peek_ਅਗਲਾ();
  // Handle properties.
  Token AnchorInfo;
  Token ਟੈਗ_Info;
parse_property:
  ਚੋਣ (T.ਕਿਸਮ_3) {
  ਹਾਲਤ Token::TK_Alias:
    get_ਅਗਲਾ();
    ਵਾਪਸ ਨਵਾਂ (ਗੰਢ_Allocator) Alias_ਗੰਢ(ਧਾਰਾ_2.CurrentDoc, T.Range.substr(1));
  ਹਾਲਤ Token::TK_Anchor:
    ਜੇ (AnchorInfo.ਕਿਸਮ_3 == Token::TK_Anchor) {
      ਸੈੱਟ_Error("Already encountered an anchor for this node!", T);
      ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
    }
    AnchorInfo = get_ਅਗਲਾ(); // Consume TK_Anchor.
    T = peek_ਅਗਲਾ();
    ਜਾਓ parse_property;
  ਹਾਲਤ Token::TK_ਟੈਗ:
    ਜੇ (ਟੈਗ_Info.ਕਿਸਮ_3 == Token::TK_ਟੈਗ) {
      ਸੈੱਟ_Error("Already encountered a tag for this node!", T);
      ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
    }
    ਟੈਗ_Info = get_ਅਗਲਾ(); // Consume TK_Tag.
    T = peek_ਅਗਲਾ();
    ਜਾਓ parse_property;
  ਮੂਲ:
    ਰੋਕੋ;
  }

  ਚੋਣ (T.ਕਿਸਮ_3) {
  ਹਾਲਤ Token::TK_ਖੰਡ_ਇੰਦਰਾਜ:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    ਵਾਪਸ ਨਵਾਂ (ਗੰਢ_Allocator) Sequence_ਗੰਢ( ਧਾਰਾ_2.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , ਟੈਗ_Info.Range
                                           , Sequence_ਗੰਢ::ST_Indentless);
  ਹਾਲਤ Token::TK_ਖੰਡ_Sequence_ਸ਼ੁਰੂ:
    get_ਅਗਲਾ();
    ਵਾਪਸ ਨਵਾਂ (ਗੰਢ_Allocator)
      Sequence_ਗੰਢ( ਧਾਰਾ_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , ਟੈਗ_Info.Range
                  , Sequence_ਗੰਢ::ST_ਖੰਡ);
  ਹਾਲਤ Token::TK_ਖੰਡ_Mapping_ਸ਼ੁਰੂ:
    get_ਅਗਲਾ();
    ਵਾਪਸ ਨਵਾਂ (ਗੰਢ_Allocator)
      Mapping_ਗੰਢ( ਧਾਰਾ_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , ਟੈਗ_Info.Range
                 , Mapping_ਗੰਢ::MT_ਖੰਡ);
  ਹਾਲਤ Token::TK_Flow_Sequence_ਸ਼ੁਰੂ:
    get_ਅਗਲਾ();
    ਵਾਪਸ ਨਵਾਂ (ਗੰਢ_Allocator)
      Sequence_ਗੰਢ( ਧਾਰਾ_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , ਟੈਗ_Info.Range
                  , Sequence_ਗੰਢ::ST_Flow);
  ਹਾਲਤ Token::TK_Flow_Mapping_ਸ਼ੁਰੂ:
    get_ਅਗਲਾ();
    ਵਾਪਸ ਨਵਾਂ (ਗੰਢ_Allocator)
      Mapping_ਗੰਢ( ਧਾਰਾ_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , ਟੈਗ_Info.Range
                 , Mapping_ਗੰਢ::MT_Flow);
  ਹਾਲਤ Token::TK_Scalar:
    get_ਅਗਲਾ();
    ਵਾਪਸ ਨਵਾਂ (ਗੰਢ_Allocator)
      Scalar_ਗੰਢ( ਧਾਰਾ_2.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , ਟੈਗ_Info.Range
                , T.Range);
  ਹਾਲਤ Token::TK_ਖੰਡ_Scalar: {
    get_ਅਗਲਾ();
    StringRef NullTerminatedStr(T.ਮੁੱਲ.c_str(), T.ਮੁੱਲ.ਲੰਮਾਈ_2() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(ਗੰਢ_Allocator).drop_ਪਿੱਠ();
    ਵਾਪਸ ਨਵਾਂ (ਗੰਢ_Allocator)
        ਖੰਡ_Scalar_ਗੰਢ(ਧਾਰਾ_2.CurrentDoc, AnchorInfo.Range.substr(1),
                        ਟੈਗ_Info.Range, StrCopy, T.Range);
  }
  ਹਾਲਤ Token::TK_ਕੁੰਜੀ:
    // Don't eat the TK_Key, KeyValueNode expects it.
    ਵਾਪਸ ਨਵਾਂ (ਗੰਢ_Allocator)
      Mapping_ਗੰਢ( ਧਾਰਾ_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , ਟੈਗ_Info.Range
                 , Mapping_ਗੰਢ::MT_Inline);
  ਹਾਲਤ Token::TK_Document_ਸ਼ੁਰੂ:
  ਹਾਲਤ Token::TK_Document_ਸਿਰਾ:
  ਹਾਲਤ Token::TK_ਧਾਰਾ_ਸਿਰਾ:
  ਮੂਲ:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    ਵਾਪਸ ਨਵਾਂ (ਗੰਢ_Allocator) Null_ਗੰਢ(ਧਾਰਾ_2.CurrentDoc);
  ਹਾਲਤ Token::TK_Flow_Mapping_ਸਿਰਾ:
  ਹਾਲਤ Token::TK_Flow_Sequence_ਸਿਰਾ:
  ਹਾਲਤ Token::TK_Flow_ਇੰਦਰਾਜ: {
    ਜੇ (ਜੜ੍ਹ_2 && (isa<Mapping_ਗੰਢ>(ਜੜ੍ਹ_2) || isa<Sequence_ਗੰਢ>(ਜੜ੍ਹ_2)))
      ਵਾਪਸ ਨਵਾਂ (ਗੰਢ_Allocator) Null_ਗੰਢ(ਧਾਰਾ_2.CurrentDoc);

    ਸੈੱਟ_Error("Unexpected token", T);
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  }
  ਹਾਲਤ Token::TK_Error:
    ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
}

ਤਾਰਕਿਕ Document::parseDirectives() {
  ਤਾਰਕਿਕ isDirective = ਝੂਠ;
  ਜਦੋਂ (ਸੱਚ) {
    Token T = peek_ਅਗਲਾ();
    ਜੇ (T.ਕਿਸਮ_3 == Token::TK_ਟੈਗ_Directive) {
      parseTAGDirective();
      isDirective = ਸੱਚ;
    } ਨਹੀਂਤਾਂ ਜੇ (T.ਕਿਸਮ_3 == Token::TK_ਸੰਸਕਰਣ_Directive) {
      parseYAMLDirective();
      isDirective = ਸੱਚ;
    } ਨਹੀਂਤਾਂ {
      ਰੋਕੋ;
    }
  }
  ਵਾਪਸ isDirective;
}

ਖਾਲੀ Document::parseYAMLDirective() {
  get_ਅਗਲਾ(); // Eat %YAML <version>
}

ਖਾਲੀ Document::parseTAGDirective() {
  Token ਟੈਗ = get_ਅਗਲਾ(); // %TAG <handle> <prefix>
  StringRef T = ਟੈਗ.Range;
  // Strip %TAG
  T = T.substr(T.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(" \t")).ltrim(" \t");
  std::size_t Handle_ਸਿਰਾ = T.ਲੱਭੋ_ਪਹਿਲਾ_ਦਾ(" \t");
  StringRef ਟੈਗ_Handle = T.substr(0, Handle_ਸਿਰਾ);
  StringRef ਟੈਗ_ਅਗੇਤਰ = T.substr(Handle_ਸਿਰਾ).ltrim(" \t");
  ਟੈਗ_Map[ਟੈਗ_Handle] = ਟੈਗ_ਅਗੇਤਰ;
}

ਤਾਰਕਿਕ Document::expectToken(ਪੂਰਨ TK) {
  Token T = get_ਅਗਲਾ();
  ਜੇ (T.ਕਿਸਮ_3 != TK) {
    ਸੈੱਟ_Error("Unexpected token", T);
    ਵਾਪਸ ਝੂਠ;
  }
  ਵਾਪਸ ਸੱਚ;
}
