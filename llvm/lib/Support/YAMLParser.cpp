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

#ضمن "llvm/Support/YAMLParser.h"
#ضمن "llvm/ADT/AllocatorList.h"
#ضمن "llvm/ADT/ArrayRef.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/ADT/Twine.h"
#ضمن "llvm/Support/Compiler.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/SMLoc.h"
#ضمن "llvm/Support/SourceMgr.h"
#ضمن "llvm/Support/Unicode.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cassert>
#ضمن <cstddef>
#ضمن <cstdint>
#ضمن <map>
#ضمن <memory>
#ضمن <string>
#ضمن <system_error>
#ضمن <utility>

ورتوں نطاق llvm;
ورتوں نطاق yaml;

تعداد UnicodeEncodingForm {
  UEF_UTF32_LE, ///< UTF-32 Little Endian
  UEF_UTF32_BE, ///< UTF-32 Big Endian
  UEF_UTF16_LE, ///< UTF-16 Little Endian
  UEF_UTF16_BE, ///< UTF-16 Big Endian
  UEF_UTF8,     ///< UTF-8 or ascii.
  UEF_Unknown   ///< Not a valid Unicode encoding.
};

/// EncodingInfo - Holds the encoding type and length of the byte order mark if
///                it exists. Length is in {0, 2, 3, 4}.
ورتوں EncodingInfo = std::pair<UnicodeEncodingForm, بنانشانی>;

/// getUnicodeEncoding - Reads up to the first 4 bytes to determine the Unicode
///                      encoding form of \a Input.
///
/// @param Input A string of length 0 or more.
/// @returns An EncodingInfo indicating the Unicode encoding form of the input
///          and how long the byte order mark is if one exists.
ساكن EncodingInfo getUnicodeEncoding(StringRef داخلہ) {
  إذا (داخلہ.خالی_تھاواں())
    أرجع {UEF_Unknown, 0};

  بدل (uint8_t(داخلہ[0])) {
  حالة 0x00:
    إذا (داخلہ.ناپ_2() >= 4) {
      إذا (  داخلہ[1] == 0
         && uint8_t(داخلہ[2]) == 0xFE
         && uint8_t(داخلہ[3]) == 0xFF)
        أرجع {UEF_UTF32_BE, 4};
      إذا (داخلہ[1] == 0 && داخلہ[2] == 0 && داخلہ[3] != 0)
        أرجع {UEF_UTF32_BE, 0};
    }

    إذا (داخلہ.ناپ_2() >= 2 && داخلہ[1] != 0)
      أرجع {UEF_UTF16_BE, 0};
    أرجع {UEF_Unknown, 0};
  حالة 0xFF:
    إذا (  داخلہ.ناپ_2() >= 4
       && uint8_t(داخلہ[1]) == 0xFE
       && داخلہ[2] == 0
       && داخلہ[3] == 0)
      أرجع {UEF_UTF32_LE, 4};

    إذا (داخلہ.ناپ_2() >= 2 && uint8_t(داخلہ[1]) == 0xFE)
      أرجع {UEF_UTF16_LE, 2};
    أرجع {UEF_Unknown, 0};
  حالة 0xFE:
    إذا (داخلہ.ناپ_2() >= 2 && uint8_t(داخلہ[1]) == 0xFF)
      أرجع {UEF_UTF16_BE, 2};
    أرجع {UEF_Unknown, 0};
  حالة 0xEF:
    إذا (  داخلہ.ناپ_2() >= 3
       && uint8_t(داخلہ[1]) == 0xBB
       && uint8_t(داخلہ[2]) == 0xBF)
      أرجع {UEF_UTF8, 3};
    أرجع {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  إذا (داخلہ.ناپ_2() >= 4 && داخلہ[1] == 0 && داخلہ[2] == 0 && داخلہ[3] == 0)
    أرجع {UEF_UTF32_LE, 0};

  إذا (داخلہ.ناپ_2() >= 2 && داخلہ[1] == 0)
    أرجع {UEF_UTF16_LE, 0};

  أرجع {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
فراغ گرہ::anchor() {}
فراغ Null_گرہ::anchor() {}
فراغ Scalar_گرہ::anchor() {}
فراغ بلاک_Scalar_گرہ::anchor() {}
فراغ کلید_قدر_گرہ::anchor() {}
فراغ Mapping_گرہ::anchor() {}
فراغ Sequence_گرہ::anchor() {}
فراغ Alias_گرہ::anchor() {}

نطاق llvm {
نطاق yaml {

/// Token - A single YAML token.
بنية Token {
  تعداد Token_قسم {
    TK_Error, // Uninitialized token.
    TK_دھارا_شروع,
    TK_دھارا_سرا,
    TK_نسخہ_Directive,
    TK_ٹیگ_Directive,
    TK_Document_شروع,
    TK_Document_سرا,
    TK_بلاک_اندراج,
    TK_بلاک_سرا,
    TK_بلاک_Sequence_شروع,
    TK_بلاک_Mapping_شروع,
    TK_Flow_اندراج,
    TK_Flow_Sequence_شروع,
    TK_Flow_Sequence_سرا,
    TK_Flow_Mapping_شروع,
    TK_Flow_Mapping_سرا,
    TK_کلید,
    TK_قدر,
    TK_Scalar,
    TK_بلاک_Scalar,
    TK_Alias,
    TK_Anchor,
    TK_ٹیگ
  } قسم_2 = TK_Error;

  /// A string of length 0 or more whose begin() points to the logical location
  /// of the token in the input.
  StringRef Range;

  /// The value of a block scalar node.
  std::string قدر;

  Token() = افتراضي;
};

} // end namespace yaml
} // end namespace llvm

ورتوں Token_قطار_T = BumpPtrList<Token>;

نطاق {

/// This struct is used to track simple keys.
///
/// Simple keys are handled by creating an entry in SimpleKeys for each Token
/// which could legally be the start of a simple key. When peekNext is called,
/// if the Token To be returned is referenced by a SimpleKey, we continue
/// tokenizing until that potential simple key has either been found to not be
/// a simple key (we moved on to the next line or went further than 1024 chars).
/// Or when we run into a Value, and then insert a Key token (and possibly
/// others) before the SimpleKey's Tok.
بنية Simple_کلید {
  Token_قطار_T::iterator Tok;
  بنانشانی کالم = 0;
  بنانشانی سطر = 0;
  بنانشانی FlowLevel = 0;
  منطقي IsRequired = خطأ;

  منطقي عامل ==(ثابت Simple_کلید &Other) {
    أرجع Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
ورتوں UTF8Decoded = std::pair<uint32_t, بنانشانی>;

ساكن UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator سرا_2 = Range.سرا();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  إذا (Position < سرا_2 && (*Position & 0x80) == 0) {
    أرجع {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  إذا (Position + 1 < سرا_2 && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    إذا (codepoint >= 0x80)
      أرجع {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  إذا (Position + 2 < سرا_2 && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    إذا (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      أرجع {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  إذا (Position + 3 < سرا_2 && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    إذا (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      أرجع {codepoint, 4};
  }
  أرجع {0, 0};
}

نطاق llvm {
نطاق yaml {

/// Scans YAML tokens from a MemoryBuffer.
صنف Scanner {
عام:
  Scanner(StringRef داخلہ, سروت_Mgr &SM, منطقي ShowColors = سچ,
          std::error_کوڈ *EC = خالی_سوچک);
  Scanner(حافظہ_بفر_Ref بفر, سروت_Mgr &SM_, منطقي ShowColors = سچ,
          std::error_کوڈ *EC = خالی_سوچک);

  /// Parse the next token and return it without popping it.
  Token &peek_اگلا();

  /// Parse the next token and pop it from the queue.
  Token get_اگلا();

  فراغ printError(SMLoc Loc, سروت_Mgr::Diag_قسم قسم_2, ثابت Twine &سنیہا_2,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.Print_سنیہا(Loc, قسم_2, سنیہا_2, Ranges, /* FixIts= */ {}, ShowColors);
  }

  فراغ سیٹ_Error(ثابت Twine &سنیہا_2, StringRef::iterator Position) {
    إذا (Position >= سرا_2)
      Position = سرا_2 - 1;

    // propagate the error if possible
    إذا (EC)
      *EC = make_error_کوڈ(std::errc::invalid_دلیل);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    إذا (!Failed)
      printError(SMLoc::get_From_اشارہ_گر(Position), سروت_Mgr::DK_Error, سنیہا_2);
    Failed = سچ;
  }

  /// Returns true if an error occurred while parsing.
  منطقي failed() {
    أرجع Failed;
  }

خاص:
  فراغ init(حافظہ_بفر_Ref بفر);

  StringRef current_داخلہ() {
    أرجع StringRef(Current, سرا_2 - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    أرجع ::decodeUTF8(StringRef(Position, سرا_2 - Position));
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

  ورتوں SkipWhileFunc = StringRef::iterator (Scanner::*)(StringRef::iterator);

  /// Skip minimal well-formed code unit subsequences until Func
  ///        returns its input.
  ///
  /// @returns The code unit after the last minimal well-formed code unit
  ///          subsequence that Func accepted.
  StringRef::iterator skip_while( SkipWhileFunc Func
                                , StringRef::iterator Position);

  /// Skip minimal well-formed code unit subsequences until Func returns its
  /// input.
  فراغ advanceWhile(SkipWhileFunc Func);

  /// Scan ns-uri-char[39]s starting at Cur.
  ///
  /// This updates Cur and Column while scanning.
  فراغ جانچ_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  منطقي consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  فراغ skip(uint32_t فاصلہ_2);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  منطقي isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  منطقي is_Plain_محفوظ_Non_Blank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  منطقي is_سطر_خالی_تھاواں(StringRef سطر);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  منطقي consume_سطر_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  فراغ save_Simple_کلید_Candidate( Token_قطار_T::iterator Tok
                             , بنانشانی At_کالم
                             , منطقي IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  فراغ remove_Stale_Simple_کلید_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  فراغ remove_Simple_کلید_Candidates_On_Flow_Level(بنانشانی Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  منطقي unrollIndent(صحيح منزل_کالم);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  منطقي rollIndent( صحيح منزل_کالم
                 , Token::Token_قسم قسم_2
                 , Token_قطار_T::iterator داخل_کرو_نقطہ);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  فراغ skipComment();

  /// Skip whitespace and comments until the start of the next token.
  فراغ جانچ_منزل_اگلا_Token();

  /// Must be the first token generated.
  منطقي جانچ_دھارا_شروع();

  /// Generate tokens needed to close out the stream.
  منطقي جانچ_دھارا_سرا();

  /// Scan a %BLAH directive.
  منطقي جانچ_Directive();

  /// Scan a ... or ---.
  منطقي جانچ_Document_Indicator(منطقي Is_شروع);

  /// Scan a [ or { and generate the proper flow collection start token.
  منطقي جانچ_Flow_Collection_شروع(منطقي IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  منطقي جانچ_Flow_Collection_سرا(منطقي IsSequence);

  /// Scan the , that separates entries in a flow collection.
  منطقي جانچ_Flow_اندراج();

  /// Scan the - that starts block sequence entries.
  منطقي جانچ_بلاک_اندراج();

  /// Scan an explicit ? indicating a key.
  منطقي جانچ_کلید();

  /// Scan an explicit : indicating a value.
  منطقي جانچ_قدر();

  /// Scan a quoted scalar.
  منطقي جانچ_Flow_Scalar(منطقي IsDoubleQuoted);

  /// Scan an unquoted scalar.
  منطقي جانچ_Plain_Scalar();

  /// Scan an Alias or Anchor starting with * or &.
  منطقي جانچ_Alias_Or_Anchor(منطقي IsAlias);

  /// Scan a block scalar starting with | or >.
  منطقي جانچ_بلاک_Scalar(منطقي IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  منطقي جانچ_بلاک_Scalar_Indicators(حرف &StyleIndicator, حرف &ChompingIndicator,
                                 بنانشانی &IndentIndicator, منطقي &Is_پورا);

  /// Scan a style indicator in a block scalar header.
  حرف جانچ_بلاک_Style_Indicator();

  /// Scan a chomping indicator in a block scalar header.
  حرف جانچ_بلاک_Chomping_Indicator();

  /// Scan an indentation indicator in a block scalar header.
  بنانشانی جانچ_بلاک_Indentation_Indicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  منطقي جانچ_بلاک_Scalar_سرخی(حرف &ChompingIndicator, بنانشانی &IndentIndicator,
                             منطقي &Is_پورا);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  منطقي لبھو_بلاک_Scalar_Indent(بنانشانی &بلاک_Indent, بنانشانی بلاک_Exit_Indent,
                             بنانشانی &سطر_Breaks, منطقي &Is_پورا);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  منطقي جانچ_بلاک_Scalar_Indent(بنانشانی بلاک_Indent, بنانشانی بلاک_Exit_Indent,
                             منطقي &Is_پورا);

  /// Scan a tag of the form !stuff.
  منطقي جانچ_ٹیگ();

  /// Dispatch to the next scanning function based on \a *Cur.
  منطقي fetch_ہور_ٹوکن();

  /// The SourceMgr used for diagnostics and buffer management.
  سروت_Mgr &SM;

  /// The original input.
  حافظہ_بفر_Ref داخلہ_بفر;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator سرا_2;

  /// Current YAML indentation level in spaces.
  صحيح Indent;

  /// Current column number in Unicode code points.
  بنانشانی کالم;

  /// Current line number.
  بنانشانی سطر;

  /// How deep we are in flow style containers. 0 Means at block level.
  بنانشانی FlowLevel;

  /// Are we at the start of the stream?
  منطقي Is_شروع_دا_دھارا;

  /// Can the next token be the start of a simple key?
  منطقي Is_Simple_کلید_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  منطقي Is_Adjacent_قدر_Allowed_وچ_Flow;

  /// True if an error has occurred.
  منطقي Failed;

  /// Should colors be used when printing out the diagnostic messages?
  منطقي ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  Token_قطار_T Token_قطار;

  /// Indentation levels.
  Small_سمتیہ<صحيح, 4> Indents;

  /// Potential simple keys.
  Small_سمتیہ<Simple_کلید, 4> Simple_کلیدیں;

  std::error_کوڈ *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
ساكن فراغ encodeUTF8( uint32_t Unicode_Scalar_قدر
                      , Small_سمتیہ_Impl<حرف> &نتیجہ) {
  إذا (Unicode_Scalar_قدر <= 0x7F) {
    نتیجہ.ڈالو_پٹھ(Unicode_Scalar_قدر & 0x7F);
  } وإلا إذا (Unicode_Scalar_قدر <= 0x7FF) {
    uint8_t پہلا_Byte = 0xC0 | ((Unicode_Scalar_قدر & 0x7C0) >> 6);
    uint8_t دوجا_Byte = 0x80 | (Unicode_Scalar_قدر & 0x3F);
    نتیجہ.ڈالو_پٹھ(پہلا_Byte);
    نتیجہ.ڈالو_پٹھ(دوجا_Byte);
  } وإلا إذا (Unicode_Scalar_قدر <= 0xFFFF) {
    uint8_t پہلا_Byte = 0xE0 | ((Unicode_Scalar_قدر & 0xF000) >> 12);
    uint8_t دوجا_Byte = 0x80 | ((Unicode_Scalar_قدر & 0xFC0) >> 6);
    uint8_t تیجا_Byte = 0x80 | (Unicode_Scalar_قدر & 0x3F);
    نتیجہ.ڈالو_پٹھ(پہلا_Byte);
    نتیجہ.ڈالو_پٹھ(دوجا_Byte);
    نتیجہ.ڈالو_پٹھ(تیجا_Byte);
  } وإلا إذا (Unicode_Scalar_قدر <= 0x10FFFF) {
    uint8_t پہلا_Byte = 0xF0 | ((Unicode_Scalar_قدر & 0x1F0000) >> 18);
    uint8_t دوجا_Byte = 0x80 | ((Unicode_Scalar_قدر & 0x3F000) >> 12);
    uint8_t تیجا_Byte = 0x80 | ((Unicode_Scalar_قدر & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_قدر & 0x3F);
    نتیجہ.ڈالو_پٹھ(پہلا_Byte);
    نتیجہ.ڈالو_پٹھ(دوجا_Byte);
    نتیجہ.ڈالو_پٹھ(تیجا_Byte);
    نتیجہ.ڈالو_پٹھ(FourthByte);
  }
}

منطقي yaml::dump_ٹوکن(StringRef داخلہ, raw_ostream &OS) {
  سروت_Mgr SM;
  Scanner scanner(داخلہ, SM);
  طالما (سچ) {
    Token T = scanner.get_اگلا();
    بدل (T.قسم_2) {
    حالة Token::TK_دھارا_شروع:
      OS << "Stream-Start: ";
      اكسر;
    حالة Token::TK_دھارا_سرا:
      OS << "Stream-End: ";
      اكسر;
    حالة Token::TK_نسخہ_Directive:
      OS << "Version-Directive: ";
      اكسر;
    حالة Token::TK_ٹیگ_Directive:
      OS << "Tag-Directive: ";
      اكسر;
    حالة Token::TK_Document_شروع:
      OS << "Document-Start: ";
      اكسر;
    حالة Token::TK_Document_سرا:
      OS << "Document-End: ";
      اكسر;
    حالة Token::TK_بلاک_اندراج:
      OS << "Block-Entry: ";
      اكسر;
    حالة Token::TK_بلاک_سرا:
      OS << "Block-End: ";
      اكسر;
    حالة Token::TK_بلاک_Sequence_شروع:
      OS << "Block-Sequence-Start: ";
      اكسر;
    حالة Token::TK_بلاک_Mapping_شروع:
      OS << "Block-Mapping-Start: ";
      اكسر;
    حالة Token::TK_Flow_اندراج:
      OS << "Flow-Entry: ";
      اكسر;
    حالة Token::TK_Flow_Sequence_شروع:
      OS << "Flow-Sequence-Start: ";
      اكسر;
    حالة Token::TK_Flow_Sequence_سرا:
      OS << "Flow-Sequence-End: ";
      اكسر;
    حالة Token::TK_Flow_Mapping_شروع:
      OS << "Flow-Mapping-Start: ";
      اكسر;
    حالة Token::TK_Flow_Mapping_سرا:
      OS << "Flow-Mapping-End: ";
      اكسر;
    حالة Token::TK_کلید:
      OS << "Key: ";
      اكسر;
    حالة Token::TK_قدر:
      OS << "Value: ";
      اكسر;
    حالة Token::TK_Scalar:
      OS << "Scalar: ";
      اكسر;
    حالة Token::TK_بلاک_Scalar:
      OS << "Block Scalar: ";
      اكسر;
    حالة Token::TK_Alias:
      OS << "Alias: ";
      اكسر;
    حالة Token::TK_Anchor:
      OS << "Anchor: ";
      اكسر;
    حالة Token::TK_ٹیگ:
      OS << "Tag: ";
      اكسر;
    حالة Token::TK_Error:
      اكسر;
    }
    OS << T.Range << "\n";
    إذا (T.قسم_2 == Token::TK_دھارا_سرا)
      اكسر;
    وإلا إذا (T.قسم_2 == Token::TK_Error)
      أرجع خطأ;
  }
  أرجع سچ;
}

منطقي yaml::جانچ_ٹوکن(StringRef داخلہ) {
  سروت_Mgr SM;
  Scanner scanner(داخلہ, SM);
  طالما (سچ) {
    Token T = scanner.get_اگلا();
    إذا (T.قسم_2 == Token::TK_دھارا_سرا)
      اكسر;
    وإلا إذا (T.قسم_2 == Token::TK_Error)
      أرجع خطأ;
  }
  أرجع سچ;
}

std::string yaml::escape(StringRef داخلہ, منطقي EscapePrintable) {
  std::string Escaped_داخلہ;
  لكل (StringRef::iterator i = داخلہ.begin(), e = داخلہ.سرا(); i != e; ++i) {
    إذا (*i == '\\')
      Escaped_داخلہ += "\\\\";
    وإلا إذا (*i == '"')
      Escaped_داخلہ += "\\\"";
    وإلا إذا (*i == 0)
      Escaped_داخلہ += "\\0";
    وإلا إذا (*i == 0x07)
      Escaped_داخلہ += "\\a";
    وإلا إذا (*i == 0x08)
      Escaped_داخلہ += "\\b";
    وإلا إذا (*i == 0x09)
      Escaped_داخلہ += "\\t";
    وإلا إذا (*i == 0x0A)
      Escaped_داخلہ += "\\n";
    وإلا إذا (*i == 0x0B)
      Escaped_داخلہ += "\\v";
    وإلا إذا (*i == 0x0C)
      Escaped_داخلہ += "\\f";
    وإلا إذا (*i == 0x0D)
      Escaped_داخلہ += "\\r";
    وإلا إذا (*i == 0x1B)
      Escaped_داخلہ += "\\e";
    وإلا إذا ((بنانشانی حرف)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_داخلہ += "\\x" + std::string(2 - HexStr.ناپ_2(), '0') + HexStr;
    } وإلا إذا (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_قدر
        = decodeUTF8(StringRef(i, داخلہ.سرا() - i));
      إذا (Unicode_Scalar_قدر.دوجا_2 == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_داخلہ, Val);
        // FIXME: Error reporting.
        أرجع Escaped_داخلہ;
      }
      إذا (Unicode_Scalar_قدر.پہلا_2 == 0x85)
        Escaped_داخلہ += "\\N";
      وإلا إذا (Unicode_Scalar_قدر.پہلا_2 == 0xA0)
        Escaped_داخلہ += "\\_";
      وإلا إذا (Unicode_Scalar_قدر.پہلا_2 == 0x2028)
        Escaped_داخلہ += "\\L";
      وإلا إذا (Unicode_Scalar_قدر.پہلا_2 == 0x2029)
        Escaped_داخلہ += "\\P";
      وإلا إذا (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_قدر.پہلا_2))
        Escaped_داخلہ += StringRef(i, Unicode_Scalar_قدر.دوجا_2);
      وإلا {
        std::string HexStr = utohexstr(Unicode_Scalar_قدر.پہلا_2);
        إذا (HexStr.ناپ_2() <= 2)
          Escaped_داخلہ += "\\x" + std::string(2 - HexStr.ناپ_2(), '0') + HexStr;
        وإلا إذا (HexStr.ناپ_2() <= 4)
          Escaped_داخلہ += "\\u" + std::string(4 - HexStr.ناپ_2(), '0') + HexStr;
        وإلا إذا (HexStr.ناپ_2() <= 8)
          Escaped_داخلہ += "\\U" + std::string(8 - HexStr.ناپ_2(), '0') + HexStr;
      }
      i += Unicode_Scalar_قدر.دوجا_2 - 1;
    } وإلا {
      Escaped_داخلہ.ڈالو_پٹھ(*i);
    }
  }
  أرجع Escaped_داخلہ;
}

std::optional<منطقي> yaml::parseBool(StringRef S) {
  بدل (S.ناپ_2()) {
  حالة 1:
    بدل (S.front()) {
    حالة 'y':
    حالة 'Y':
      أرجع سچ;
    حالة 'n':
    حالة 'N':
      أرجع خطأ;
    افتراضي:
      أرجع std::nullopt;
    }
  حالة 2:
    بدل (S.front()) {
    حالة 'O':
      إذا (S[1] == 'N') // ON
        أرجع سچ;
      [[fallthrough]];
    حالة 'o':
      إذا (S[1] == 'n') //[Oo]n
        أرجع سچ;
      أرجع std::nullopt;
    حالة 'N':
      إذا (S[1] == 'O') // NO
        أرجع خطأ;
      [[fallthrough]];
    حالة 'n':
      إذا (S[1] == 'o') //[Nn]o
        أرجع خطأ;
      أرجع std::nullopt;
    افتراضي:
      أرجع std::nullopt;
    }
  حالة 3:
    بدل (S.front()) {
    حالة 'O':
      إذا (S.drop_front() == "FF") // OFF
        أرجع خطأ;
      [[fallthrough]];
    حالة 'o':
      إذا (S.drop_front() == "ff") //[Oo]ff
        أرجع خطأ;
      أرجع std::nullopt;
    حالة 'Y':
      إذا (S.drop_front() == "ES") // YES
        أرجع سچ;
      [[fallthrough]];
    حالة 'y':
      إذا (S.drop_front() == "es") //[Yy]es
        أرجع سچ;
      أرجع std::nullopt;
    افتراضي:
      أرجع std::nullopt;
    }
  حالة 4:
    بدل (S.front()) {
    حالة 'T':
      إذا (S.drop_front() == "RUE") // TRUE
        أرجع سچ;
      [[fallthrough]];
    حالة 't':
      إذا (S.drop_front() == "rue") //[Tt]rue
        أرجع سچ;
      أرجع std::nullopt;
    افتراضي:
      أرجع std::nullopt;
    }
  حالة 5:
    بدل (S.front()) {
    حالة 'F':
      إذا (S.drop_front() == "ALSE") // FALSE
        أرجع خطأ;
      [[fallthrough]];
    حالة 'f':
      إذا (S.drop_front() == "alse") //[Ff]alse
        أرجع خطأ;
      أرجع std::nullopt;
    افتراضي:
      أرجع std::nullopt;
    }
  افتراضي:
    أرجع std::nullopt;
  }
}

Scanner::Scanner(StringRef داخلہ, سروت_Mgr &sm, منطقي ShowColors,
                 std::error_کوڈ *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(حافظہ_بفر_Ref(داخلہ, "YAML"));
}

Scanner::Scanner(حافظہ_بفر_Ref بفر, سروت_Mgr &SM_, منطقي ShowColors,
                 std::error_کوڈ *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(بفر);
}

فراغ Scanner::init(حافظہ_بفر_Ref بفر) {
  داخلہ_بفر = بفر;
  Current = داخلہ_بفر.get_بفر_شروع();
  سرا_2 = داخلہ_بفر.get_بفر_سرا();
  Indent = -1;
  کالم = 0;
  سطر = 0;
  FlowLevel = 0;
  Is_شروع_دا_دھارا = سچ;
  Is_Simple_کلید_Allowed = سچ;
  Is_Adjacent_قدر_Allowed_وچ_Flow = خطأ;
  Failed = خطأ;
  std::unique_ptr<حافظہ_بفر> داخلہ_بفر_Owner =
      حافظہ_بفر::get_Mem_بفر(بفر, /*RequiresNullTerminator=*/خطأ);
  SM.جوڑو_New_سروت_بفر(std::move(داخلہ_بفر_Owner), SMLoc());
}

Token &Scanner::peek_اگلا() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  منطقي Need_ہور = خطأ;
  طالما (سچ) {
    إذا (Token_قطار.خالی_تھاواں() || Need_ہور) {
      إذا (!fetch_ہور_ٹوکن()) {
        Token_قطار.clear();
        Simple_کلیدیں.clear();
        Token_قطار.ڈالو_پٹھ(Token());
        أرجع Token_قطار.front();
      }
    }
    أكد(!Token_قطار.خالی_تھاواں() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_کلید_Candidates();
    Simple_کلید SK;
    SK.Tok = Token_قطار.begin();
    إذا (!is_contained(Simple_کلیدیں, SK))
      اكسر;
    وإلا
      Need_ہور = سچ;
  }
  أرجع Token_قطار.front();
}

Token Scanner::get_اگلا() {
  Token Ret = peek_اگلا();
  // TokenQueue can be empty if there was an error getting the next token.
  إذا (!Token_قطار.خالی_تھاواں())
    Token_قطار.نکالو_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  إذا (Token_قطار.خالی_تھاواں())
    Token_قطار.resetAlloc();

  أرجع Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  إذا (Position == سرا_2)
    أرجع Position;
  // Check 7 bit c-printable - b-char.
  إذا (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    أرجع Position + 1;

  // Check for valid UTF-8.
  إذا (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    إذا (   u8d.دوجا_2 != 0
        && u8d.پہلا_2 != 0xFEFF
        && ( u8d.پہلا_2 == 0x85
          || ( u8d.پہلا_2 >= 0xA0
            && u8d.پہلا_2 <= 0xD7FF)
          || ( u8d.پہلا_2 >= 0xE000
            && u8d.پہلا_2 <= 0xFFFD)
          || ( u8d.پہلا_2 >= 0x10000
            && u8d.پہلا_2 <= 0x10FFFF)))
      أرجع Position + u8d.دوجا_2;
  }
  أرجع Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  إذا (Position == سرا_2)
    أرجع Position;
  إذا (*Position == 0x0D) {
    إذا (Position + 1 != سرا_2 && *(Position + 1) == 0x0A)
      أرجع Position + 2;
    أرجع Position + 1;
  }

  إذا (*Position == 0x0A)
    أرجع Position + 1;
  أرجع Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  إذا (Position == سرا_2)
    أرجع Position;
  إذا (*Position == ' ')
    أرجع Position + 1;
  أرجع Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  إذا (Position == سرا_2)
    أرجع Position;
  إذا (*Position == ' ' || *Position == '\t')
    أرجع Position + 1;
  أرجع Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  إذا (Position == سرا_2)
    أرجع Position;
  إذا (*Position == ' ' || *Position == '\t')
    أرجع Position;
  أرجع skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  طالما (سچ) {
    StringRef::iterator i = (هذا->*Func)(Position);
    إذا (i == Position)
      اكسر;
    Position = i;
  }
  أرجع Position;
}

فراغ Scanner::advanceWhile(SkipWhileFunc Func) {
  خودکار Final = skip_while(Func, Current);
  کالم += Final - Current;
  Current = Final;
}

ساكن منطقي is_ns_hex_digit(ثابت حرف C) { أرجع isAlnum(C); }

ساكن منطقي is_ns_لفظ_char(ثابت حرف C) { أرجع C == '-' || isAlpha(C); }

فراغ Scanner::جانچ_ns_uri_char() {
  طالما (سچ) {
    إذا (Current == سرا_2)
      اكسر;
    إذا ((*Current == '%' && Current + 2 < سرا_2 &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_لفظ_char(*Current) ||
        StringRef(Current, 1).لبھو_پہلا_دا("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++کالم;
    } وإلا {
      اكسر;
    }
  }
}

منطقي Scanner::consume(uint32_t Expected) {
  إذا (Expected >= 0x80) {
    سیٹ_Error("Cannot consume non-ascii characters", Current);
    أرجع خطأ;
  }
  إذا (Current == سرا_2)
    أرجع خطأ;
  إذا (uint8_t(*Current) >= 0x80) {
    سیٹ_Error("Cannot consume non-ascii characters", Current);
    أرجع خطأ;
  }
  إذا (uint8_t(*Current) == Expected) {
    ++Current;
    ++کالم;
    أرجع سچ;
  }
  أرجع خطأ;
}

فراغ Scanner::skip(uint32_t فاصلہ_2) {
  Current += فاصلہ_2;
  کالم += فاصلہ_2;
  أكد(Current <= سرا_2 && "Skipped past the end");
}

منطقي Scanner::isBlankOrBreak(StringRef::iterator Position) {
  إذا (Position == سرا_2)
    أرجع خطأ;
  أرجع *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

منطقي Scanner::is_Plain_محفوظ_Non_Blank(StringRef::iterator Position) {
  إذا (Position == سرا_2 || isBlankOrBreak(Position))
    أرجع خطأ;
  إذا (FlowLevel &&
      StringRef(Position, 1).لبھو_پہلا_دا(",[]{}") != StringRef::npos)
    أرجع خطأ;
  أرجع سچ;
}

منطقي Scanner::is_سطر_خالی_تھاواں(StringRef سطر) {
  لكل (ثابت خودکار *Position = سطر.begin(); Position != سطر.سرا(); ++Position)
    إذا (!isBlankOrBreak(Position))
      أرجع خطأ;
  أرجع سچ;
}

منطقي Scanner::consume_سطر_Break_If_Present() {
  خودکار اگلا_2 = skip_b_break(Current);
  إذا (اگلا_2 == Current)
    أرجع خطأ;
  کالم = 0;
  ++سطر;
  Current = اگلا_2;
  أرجع سچ;
}

فراغ Scanner::save_Simple_کلید_Candidate( Token_قطار_T::iterator Tok
                                    , بنانشانی At_کالم
                                    , منطقي IsRequired) {
  إذا (Is_Simple_کلید_Allowed) {
    Simple_کلید SK;
    SK.Tok = Tok;
    SK.سطر = سطر;
    SK.کالم = At_کالم;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_کلیدیں.ڈالو_پٹھ(SK);
  }
}

فراغ Scanner::remove_Stale_Simple_کلید_Candidates() {
  لكل (Small_سمتیہ_Impl<Simple_کلید>::iterator i = Simple_کلیدیں.begin();
                                            i != Simple_کلیدیں.سرا();) {
    إذا (i->سطر != سطر || i->کالم + 1024 < کالم) {
      إذا (i->IsRequired)
        سیٹ_Error( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_کلیدیں.erase(i);
    } وإلا {
      ++i;
    }
  }
}

فراغ Scanner::remove_Simple_کلید_Candidates_On_Flow_Level(بنانشانی Level) {
  إذا (!Simple_کلیدیں.خالی_تھاواں() && (Simple_کلیدیں.سرا() - 1)->FlowLevel == Level)
    Simple_کلیدیں.نکالو_پٹھ();
}

منطقي Scanner::unrollIndent(صحيح منزل_کالم) {
  Token T;
  // Indentation is ignored in flow.
  إذا (FlowLevel != 0)
    أرجع سچ;

  طالما (Indent > منزل_کالم) {
    T.قسم_2 = Token::TK_بلاک_سرا;
    T.Range = StringRef(Current, 1);
    Token_قطار.ڈالو_پٹھ(T);
    Indent = Indents.نکالو_پٹھ_val();
  }

  أرجع سچ;
}

منطقي Scanner::rollIndent( صحيح منزل_کالم
                        , Token::Token_قسم قسم_2
                        , Token_قطار_T::iterator داخل_کرو_نقطہ) {
  إذا (FlowLevel)
    أرجع سچ;
  إذا (Indent < منزل_کالم) {
    Indents.ڈالو_پٹھ(Indent);
    Indent = منزل_کالم;

    Token T;
    T.قسم_2 = قسم_2;
    T.Range = StringRef(Current, 0);
    Token_قطار.داخل_کرو(داخل_کرو_نقطہ, T);
  }
  أرجع سچ;
}

فراغ Scanner::skipComment() {
  إذا (Current == سرا_2 || *Current != '#')
    أرجع;
  طالما (سچ) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    إذا (I == Current)
      اكسر;
    Current = I;
    ++کالم;
  }
}

فراغ Scanner::جانچ_منزل_اگلا_Token() {
  طالما (سچ) {
    طالما (Current != سرا_2 && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    إذا (i == Current)
      اكسر;
    Current = i;
    ++سطر;
    کالم = 0;
    // New lines may start a simple key.
    إذا (!FlowLevel)
      Is_Simple_کلید_Allowed = سچ;
  }
}

منطقي Scanner::جانچ_دھارا_شروع() {
  Is_شروع_دا_دھارا = خطأ;

  EncodingInfo EI = getUnicodeEncoding(current_داخلہ());

  Token T;
  T.قسم_2 = Token::TK_دھارا_شروع;
  T.Range = StringRef(Current, EI.دوجا_2);
  Token_قطار.ڈالو_پٹھ(T);
  Current += EI.دوجا_2;
  أرجع سچ;
}

منطقي Scanner::جانچ_دھارا_سرا() {
  // Force an ending new line if one isn't present.
  إذا (کالم != 0) {
    کالم = 0;
    ++سطر;
  }

  unrollIndent(-1);
  Simple_کلیدیں.clear();
  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_وچ_Flow = خطأ;

  Token T;
  T.قسم_2 = Token::TK_دھارا_سرا;
  T.Range = StringRef(Current, 0);
  Token_قطار.ڈالو_پٹھ(T);
  أرجع سچ;
}

منطقي Scanner::جانچ_Directive() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_کلیدیں.clear();
  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_وچ_Flow = خطأ;

  StringRef::iterator شروع = Current;
  consume('%');
  StringRef::iterator Name_شروع = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(Name_شروع, Current - Name_شروع);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  إذا (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.قسم_2 = Token::TK_نسخہ_Directive;
    T.Range = StringRef(شروع, Current - شروع);
    Token_قطار.ڈالو_پٹھ(T);
    أرجع سچ;
  } وإلا إذا(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.قسم_2 = Token::TK_ٹیگ_Directive;
    T.Range = StringRef(شروع, Current - شروع);
    Token_قطار.ڈالو_پٹھ(T);
    أرجع سچ;
  }
  أرجع خطأ;
}

منطقي Scanner::جانچ_Document_Indicator(منطقي Is_شروع) {
  unrollIndent(-1);
  Simple_کلیدیں.clear();
  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_وچ_Flow = خطأ;

  Token T;
  T.قسم_2 = Is_شروع ? Token::TK_Document_شروع : Token::TK_Document_سرا;
  T.Range = StringRef(Current, 3);
  skip(3);
  Token_قطار.ڈالو_پٹھ(T);
  أرجع سچ;
}

منطقي Scanner::جانچ_Flow_Collection_شروع(منطقي IsSequence) {
  Token T;
  T.قسم_2 = IsSequence ? Token::TK_Flow_Sequence_شروع
                      : Token::TK_Flow_Mapping_شروع;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڈالو_پٹھ(T);

  // [ and { may begin a simple key.
  save_Simple_کلید_Candidate(--Token_قطار.سرا(), کالم - 1, خطأ);

  // And may also be followed by a simple key.
  Is_Simple_کلید_Allowed = سچ;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_قدر_Allowed_وچ_Flow = خطأ;
  ++FlowLevel;
  أرجع سچ;
}

منطقي Scanner::جانچ_Flow_Collection_سرا(منطقي IsSequence) {
  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_وچ_Flow = سچ;
  Token T;
  T.قسم_2 = IsSequence ? Token::TK_Flow_Sequence_سرا
                      : Token::TK_Flow_Mapping_سرا;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڈالو_پٹھ(T);
  إذا (FlowLevel)
    --FlowLevel;
  أرجع سچ;
}

منطقي Scanner::جانچ_Flow_اندراج() {
  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = سچ;
  Is_Adjacent_قدر_Allowed_وچ_Flow = خطأ;
  Token T;
  T.قسم_2 = Token::TK_Flow_اندراج;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڈالو_پٹھ(T);
  أرجع سچ;
}

منطقي Scanner::جانچ_بلاک_اندراج() {
  rollIndent(کالم, Token::TK_بلاک_Sequence_شروع, Token_قطار.سرا());
  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = سچ;
  Is_Adjacent_قدر_Allowed_وچ_Flow = خطأ;
  Token T;
  T.قسم_2 = Token::TK_بلاک_اندراج;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڈالو_پٹھ(T);
  أرجع سچ;
}

منطقي Scanner::جانچ_کلید() {
  إذا (!FlowLevel)
    rollIndent(کالم, Token::TK_بلاک_Mapping_شروع, Token_قطار.سرا());

  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = !FlowLevel;
  Is_Adjacent_قدر_Allowed_وچ_Flow = خطأ;

  Token T;
  T.قسم_2 = Token::TK_کلید;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڈالو_پٹھ(T);
  أرجع سچ;
}

منطقي Scanner::جانچ_قدر() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  إذا (!Simple_کلیدیں.خالی_تھاواں()) {
    Simple_کلید SK = Simple_کلیدیں.نکالو_پٹھ_val();
    Token T;
    T.قسم_2 = Token::TK_کلید;
    T.Range = SK.Tok->Range;
    Token_قطار_T::iterator i, e;
    لكل (i = Token_قطار.begin(), e = Token_قطار.سرا(); i != e; ++i) {
      إذا (i == SK.Tok)
        اكسر;
    }
    إذا (i == e) {
      Failed = سچ;
      أرجع خطأ;
    }
    i = Token_قطار.داخل_کرو(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.کالم, Token::TK_بلاک_Mapping_شروع, i);

    Is_Simple_کلید_Allowed = خطأ;
  } وإلا {
    إذا (!FlowLevel)
      rollIndent(کالم, Token::TK_بلاک_Mapping_شروع, Token_قطار.سرا());
    Is_Simple_کلید_Allowed = !FlowLevel;
  }
  Is_Adjacent_قدر_Allowed_وچ_Flow = خطأ;

  Token T;
  T.قسم_2 = Token::TK_قدر;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڈالو_پٹھ(T);
  أرجع سچ;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE ساكن منطقي
wasEscaped(StringRef::iterator پہلا, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
ساكن منطقي wasEscaped(StringRef::iterator پہلا,
                       StringRef::iterator Position) {
  أكد(Position - 1 >= پہلا);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  طالما (I >= پہلا && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  أرجع (Position - 1 - I) % 2 == 1;
}

منطقي Scanner::جانچ_Flow_Scalar(منطقي IsDoubleQuoted) {
  StringRef::iterator شروع = Current;
  بنانشانی خانہ_شروع = کالم;
  إذا (IsDoubleQuoted) {
    افعل {
      ++Current;
      طالما (Current != سرا_2 && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } طالما (   Current != سرا_2
             && *(Current - 1) == '\\'
             && wasEscaped(شروع + 1, Current));
  } وإلا {
    skip(1);
    طالما (Current != سرا_2) {
      // Skip a ' followed by another '.
      إذا (Current + 1 < سرا_2 && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        استمر;
      } وإلا إذا (*Current == '\'')
        اكسر;
      StringRef::iterator i = skip_nb_char(Current);
      إذا (i == Current) {
        i = skip_b_break(Current);
        إذا (i == Current)
          اكسر;
        Current = i;
        کالم = 0;
        ++سطر;
      } وإلا {
        إذا (i == سرا_2)
          اكسر;
        Current = i;
        ++کالم;
      }
    }
  }

  إذا (Current == سرا_2) {
    سیٹ_Error("Expected quote at end of scalar", Current);
    أرجع خطأ;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.قسم_2 = Token::TK_Scalar;
  T.Range = StringRef(شروع, Current - شروع);
  Token_قطار.ڈالو_پٹھ(T);

  save_Simple_کلید_Candidate(--Token_قطار.سرا(), خانہ_شروع, خطأ);

  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_وچ_Flow = سچ;

  أرجع سچ;
}

منطقي Scanner::جانچ_Plain_Scalar() {
  StringRef::iterator شروع = Current;
  بنانشانی خانہ_شروع = کالم;
  بنانشانی LeadingBlanks = 0;
  أكد(Indent >= -1 && "Indent must be >= -1 !");
  بنانشانی indent = static_cast<بنانشانی>(Indent + 1);
  طالما (Current != سرا_2) {
    إذا (*Current == '#')
      اكسر;

    طالما (Current != سرا_2 &&
           ((*Current != ':' && is_Plain_محفوظ_Non_Blank(Current)) ||
            (*Current == ':' && is_Plain_محفوظ_Non_Blank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      إذا (i == Current)
        اكسر;
      Current = i;
      ++کالم;
    }

    // Are we at the end?
    إذا (!isBlankOrBreak(Current))
      اكسر;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    طالما (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      إذا (i != Tmp) {
        إذا (LeadingBlanks && (کالم < indent) && *Tmp == '\t') {
          سیٹ_Error("Found invalid tab character in indentation", Tmp);
          أرجع خطأ;
        }
        Tmp = i;
        ++کالم;
      } وإلا {
        i = skip_b_break(Tmp);
        إذا (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        کالم = 0;
        ++سطر;
      }
    }

    إذا (!FlowLevel && کالم < indent)
      اكسر;

    Current = Tmp;
  }
  إذا (شروع == Current) {
    سیٹ_Error("Got empty plain scalar", شروع);
    أرجع خطأ;
  }
  Token T;
  T.قسم_2 = Token::TK_Scalar;
  T.Range = StringRef(شروع, Current - شروع);
  Token_قطار.ڈالو_پٹھ(T);

  // Plain scalars can be simple keys.
  save_Simple_کلید_Candidate(--Token_قطار.سرا(), خانہ_شروع, خطأ);

  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_وچ_Flow = خطأ;

  أرجع سچ;
}

منطقي Scanner::جانچ_Alias_Or_Anchor(منطقي IsAlias) {
  StringRef::iterator شروع = Current;
  بنانشانی خانہ_شروع = کالم;
  skip(1);
  طالما (Current != سرا_2) {
    إذا (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      اكسر;
    StringRef::iterator i = skip_ns_char(Current);
    إذا (i == Current)
      اكسر;
    Current = i;
    ++کالم;
  }

  إذا (شروع + 1 == Current) {
    سیٹ_Error("Got empty alias or anchor", شروع);
    أرجع خطأ;
  }

  Token T;
  T.قسم_2 = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(شروع, Current - شروع);
  Token_قطار.ڈالو_پٹھ(T);

  // Alias and anchors can be simple keys.
  save_Simple_کلید_Candidate(--Token_قطار.سرا(), خانہ_شروع, خطأ);

  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_وچ_Flow = خطأ;

  أرجع سچ;
}

منطقي Scanner::جانچ_بلاک_Scalar_Indicators(حرف &StyleIndicator,
                                        حرف &ChompingIndicator,
                                        بنانشانی &IndentIndicator,
                                        منطقي &Is_پورا) {
  StyleIndicator = جانچ_بلاک_Style_Indicator();
  إذا (!جانچ_بلاک_Scalar_سرخی(ChompingIndicator, IndentIndicator, Is_پورا))
    أرجع خطأ;
  أرجع سچ;
}

حرف Scanner::جانچ_بلاک_Style_Indicator() {
  حرف Indicator = ' ';
  إذا (Current != سرا_2 && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  أرجع Indicator;
}

حرف Scanner::جانچ_بلاک_Chomping_Indicator() {
  حرف Indicator = ' ';
  إذا (Current != سرا_2 && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  أرجع Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
ساكن بنانشانی get_Chomped_سطر_Breaks(حرف ChompingIndicator,
                                     بنانشانی سطر_Breaks, StringRef Str) {
  إذا (ChompingIndicator == '-') // Strip all line breaks.
    أرجع 0;
  إذا (ChompingIndicator == '+') // Keep all line breaks.
    أرجع سطر_Breaks;
  // Clip trailing lines.
  أرجع Str.خالی_تھاواں() ? 0 : 1;
}

بنانشانی Scanner::جانچ_بلاک_Indentation_Indicator() {
  بنانشانی Indent = 0;
  إذا (Current != سرا_2 && (*Current >= '1' && *Current <= '9')) {
    Indent = بنانشانی(*Current - '0');
    skip(1);
  }
  أرجع Indent;
}

منطقي Scanner::جانچ_بلاک_Scalar_سرخی(حرف &ChompingIndicator,
                                    بنانشانی &IndentIndicator, منطقي &Is_پورا) {
  خودکار شروع = Current;

  ChompingIndicator = جانچ_بلاک_Chomping_Indicator();
  IndentIndicator = جانچ_بلاک_Indentation_Indicator();
  // Check for the chomping indicator once again.
  إذا (ChompingIndicator == ' ')
    ChompingIndicator = جانچ_بلاک_Chomping_Indicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  إذا (Current == سرا_2) { // EOF, we have an empty scalar.
    Token T;
    T.قسم_2 = Token::TK_بلاک_Scalar;
    T.Range = StringRef(شروع, Current - شروع);
    Token_قطار.ڈالو_پٹھ(T);
    Is_پورا = سچ;
    أرجع سچ;
  }

  إذا (!consume_سطر_Break_If_Present()) {
    سیٹ_Error("Expected a line break after block scalar header", Current);
    أرجع خطأ;
  }
  أرجع سچ;
}

منطقي Scanner::لبھو_بلاک_Scalar_Indent(بنانشانی &بلاک_Indent,
                                    بنانشانی بلاک_Exit_Indent,
                                    بنانشانی &سطر_Breaks, منطقي &Is_پورا) {
  بنانشانی Max_All_Space_سطر_Characters = 0;
  StringRef::iterator Longest_All_Space_سطر;

  طالما (سچ) {
    advanceWhile(&Scanner::skip_s_space);
    إذا (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      إذا (کالم <= بلاک_Exit_Indent) { // End of the block literal.
        Is_پورا = سچ;
        أرجع سچ;
      }
      // We found the block's indentation.
      بلاک_Indent = کالم;
      إذا (Max_All_Space_سطر_Characters > بلاک_Indent) {
        سیٹ_Error(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_سطر);
        أرجع خطأ;
      }
      أرجع سچ;
    }
    إذا (skip_b_break(Current) != Current &&
        کالم > Max_All_Space_سطر_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_سطر_Characters = کالم;
      Longest_All_Space_سطر = Current;
    }

    // Check for EOF.
    إذا (Current == سرا_2) {
      Is_پورا = سچ;
      أرجع سچ;
    }

    إذا (!consume_سطر_Break_If_Present()) {
      Is_پورا = سچ;
      أرجع سچ;
    }
    ++سطر_Breaks;
  }
  أرجع سچ;
}

منطقي Scanner::جانچ_بلاک_Scalar_Indent(بنانشانی بلاک_Indent,
                                    بنانشانی بلاک_Exit_Indent, منطقي &Is_پورا) {
  // Skip the indentation.
  طالما (کالم < بلاک_Indent) {
    خودکار I = skip_s_space(Current);
    إذا (I == Current)
      اكسر;
    Current = I;
    ++کالم;
  }

  إذا (skip_nb_char(Current) == Current)
    أرجع سچ;

  إذا (کالم <= بلاک_Exit_Indent) { // End of the block literal.
    Is_پورا = سچ;
    أرجع سچ;
  }

  إذا (کالم < بلاک_Indent) {
    إذا (Current != سرا_2 && *Current == '#') { // Trailing comment.
      Is_پورا = سچ;
      أرجع سچ;
    }
    سیٹ_Error("A text line is less indented than the block scalar", Current);
    أرجع خطأ;
  }
  أرجع سچ; // A normal text line.
}

منطقي Scanner::جانچ_بلاک_Scalar(منطقي IsLiteral) {
  أكد(*Current == '|' || *Current == '>');
  حرف StyleIndicator;
  حرف ChompingIndicator;
  بنانشانی بلاک_Indent;
  منطقي Is_پورا = خطأ;
  إذا (!جانچ_بلاک_Scalar_Indicators(StyleIndicator, ChompingIndicator, بلاک_Indent,
                                 Is_پورا))
    أرجع خطأ;
  إذا (Is_پورا)
    أرجع سچ;
  منطقي IsFolded = StyleIndicator == '>';

  ثابت خودکار *شروع = Current;
  بنانشانی بلاک_Exit_Indent = Indent < 0 ? 0 : (بنانشانی)Indent;
  بنانشانی سطر_Breaks = 0;
  إذا (بلاک_Indent == 0) {
    إذا (!لبھو_بلاک_Scalar_Indent(بلاک_Indent, بلاک_Exit_Indent, سطر_Breaks,
                               Is_پورا))
      أرجع خطأ;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  طالما (!Is_پورا) {
    إذا (!جانچ_بلاک_Scalar_Indent(بلاک_Indent, بلاک_Exit_Indent, Is_پورا))
      أرجع خطأ;
    إذا (Is_پورا)
      اكسر;

    // Parse the current line.
    خودکار سطر_شروع = Current;
    advanceWhile(&Scanner::skip_nb_char);
    إذا (سطر_شروع != Current) {
      إذا (سطر_Breaks && IsFolded && !Scanner::is_سطر_خالی_تھاواں(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        إذا (سطر_Breaks == 1) {
          Str.append(سطر_Breaks,
                     is_سطر_خالی_تھاواں(StringRef(سطر_شروع, Current - سطر_شروع))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        سطر_Breaks--;
      }
      Str.append(سطر_Breaks, '\n');
      Str.append(StringRef(سطر_شروع, Current - سطر_شروع));
      سطر_Breaks = 0;
    }

    // Check for EOF.
    إذا (Current == سرا_2)
      اكسر;

    إذا (!consume_سطر_Break_If_Present())
      اكسر;
    ++سطر_Breaks;
  }

  إذا (Current == سرا_2 && !سطر_Breaks)
    // Ensure that there is at least one line break before the end of file.
    سطر_Breaks = 1;
  Str.append(get_Chomped_سطر_Breaks(ChompingIndicator, سطر_Breaks, Str), '\n');

  // New lines may start a simple key.
  إذا (!FlowLevel)
    Is_Simple_کلید_Allowed = سچ;
  Is_Adjacent_قدر_Allowed_وچ_Flow = خطأ;

  Token T;
  T.قسم_2 = Token::TK_بلاک_Scalar;
  T.Range = StringRef(شروع, Current - شروع);
  T.قدر = std::string(Str);
  Token_قطار.ڈالو_پٹھ(T);
  أرجع سچ;
}

منطقي Scanner::جانچ_ٹیگ() {
  StringRef::iterator شروع = Current;
  بنانشانی خانہ_شروع = کالم;
  skip(1); // Eat !.
  إذا (Current == سرا_2 || isBlankOrBreak(Current)); // An empty tag.
  وإلا إذا (*Current == '<') {
    skip(1);
    جانچ_ns_uri_char();
    إذا (!consume('>'))
      أرجع خطأ;
  } وإلا {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.قسم_2 = Token::TK_ٹیگ;
  T.Range = StringRef(شروع, Current - شروع);
  Token_قطار.ڈالو_پٹھ(T);

  // Tags can be simple keys.
  save_Simple_کلید_Candidate(--Token_قطار.سرا(), خانہ_شروع, خطأ);

  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_وچ_Flow = خطأ;

  أرجع سچ;
}

منطقي Scanner::fetch_ہور_ٹوکن() {
  إذا (Is_شروع_دا_دھارا)
    أرجع جانچ_دھارا_شروع();

  جانچ_منزل_اگلا_Token();

  إذا (Current == سرا_2)
    أرجع جانچ_دھارا_سرا();

  remove_Stale_Simple_کلید_Candidates();

  unrollIndent(کالم);

  إذا (کالم == 0 && *Current == '%')
    أرجع جانچ_Directive();

  إذا (کالم == 0 && Current + 3 <= سرا_2
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == سرا_2 || isBlankOrBreak(Current + 3)))
    أرجع جانچ_Document_Indicator(سچ);

  إذا (کالم == 0 && Current + 3 <= سرا_2
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == سرا_2 || isBlankOrBreak(Current + 3)))
    أرجع جانچ_Document_Indicator(خطأ);

  إذا (*Current == '[')
    أرجع جانچ_Flow_Collection_شروع(سچ);

  إذا (*Current == '{')
    أرجع جانچ_Flow_Collection_شروع(خطأ);

  إذا (*Current == ']')
    أرجع جانچ_Flow_Collection_سرا(سچ);

  إذا (*Current == '}')
    أرجع جانچ_Flow_Collection_سرا(خطأ);

  إذا (*Current == ',')
    أرجع جانچ_Flow_اندراج();

  إذا (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == سرا_2))
    أرجع جانچ_بلاک_اندراج();

  إذا (*Current == '?' && (Current + 1 == سرا_2 || isBlankOrBreak(Current + 1)))
    أرجع جانچ_کلید();

  إذا (*Current == ':' &&
      (!is_Plain_محفوظ_Non_Blank(Current + 1) || Is_Adjacent_قدر_Allowed_وچ_Flow))
    أرجع جانچ_قدر();

  إذا (*Current == '*')
    أرجع جانچ_Alias_Or_Anchor(سچ);

  إذا (*Current == '&')
    أرجع جانچ_Alias_Or_Anchor(خطأ);

  إذا (*Current == '!')
    أرجع جانچ_ٹیگ();

  إذا (*Current == '|' && !FlowLevel)
    أرجع جانچ_بلاک_Scalar(سچ);

  إذا (*Current == '>' && !FlowLevel)
    أرجع جانچ_بلاک_Scalar(خطأ);

  إذا (*Current == '\'')
    أرجع جانچ_Flow_Scalar(خطأ);

  إذا (*Current == '"')
    أرجع جانچ_Flow_Scalar(سچ);

  // Get a plain scalar.
  StringRef پہلا_Char(Current, 1);
  إذا ((!isBlankOrBreak(Current) &&
       پہلا_Char.لبھو_پہلا_دا("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (پہلا_Char.لبھو_پہلا_دا("?:-") != StringRef::npos &&
       is_Plain_محفوظ_Non_Blank(Current + 1)))
    أرجع جانچ_Plain_Scalar();

  سیٹ_Error("Unrecognized character while tokenizing.", Current);
  أرجع خطأ;
}

دھارا::دھارا(StringRef داخلہ, سروت_Mgr &SM, منطقي ShowColors,
               std::error_کوڈ *EC)
    : scanner(جديد Scanner(داخلہ, SM, ShowColors, EC)) {}

دھارا::دھارا(حافظہ_بفر_Ref داخلہ_بفر, سروت_Mgr &SM, منطقي ShowColors,
               std::error_کوڈ *EC)
    : scanner(جديد Scanner(داخلہ_بفر, SM, ShowColors, EC)) {}

دھارا::~دھارا() = افتراضي;

منطقي دھارا::failed() { أرجع scanner->failed(); }

فراغ دھارا::printError(گرہ *N, ثابت Twine &پیغام_2, سروت_Mgr::Diag_قسم قسم_2) {
  printError(N ? N->get_سروت_Range() : SMRange(), پیغام_2, قسم_2);
}

فراغ دھارا::printError(ثابت SMRange &Range, ثابت Twine &پیغام_2,
                        سروت_Mgr::Diag_قسم قسم_2) {
  scanner->printError(Range.شروع, قسم_2, پیغام_2, Range);
}

document_iterator دھارا::begin() {
  إذا (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->get_اگلا();

  CurrentDoc.reset(جديد Document(*هذا));
  أرجع document_iterator(CurrentDoc);
}

document_iterator دھارا::سرا() {
  أرجع document_iterator();
}

فراغ دھارا::skip() {
  لكل (Document &Doc : *هذا)
    Doc.skip();
}

گرہ::گرہ(بنانشانی صحيح Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), ٹیگ(T) {
  SMLoc شروع = SMLoc::get_From_اشارہ_گر(peek_اگلا().Range.begin());
  سروت_Range = SMRange(شروع, شروع);
}

std::string گرہ::get_Verbatim_ٹیگ() ثابت {
  StringRef Raw = get_Raw_ٹیگ();
  إذا (!Raw.خالی_تھاواں() && Raw != "!") {
    std::string Ret;
    إذا (Raw.لبھو_آخری_دا('!') == 0) {
      Ret = std::string(Doc->get_ٹیگ_Map().لبھو("!")->دوجا_2);
      Ret += Raw.substr(1);
      أرجع Ret;
    } وإلا إذا (Raw.starts_with("!!")) {
      Ret = std::string(Doc->get_ٹیگ_Map().لبھو("!!")->دوجا_2);
      Ret += Raw.substr(2);
      أرجع Ret;
    } وإلا {
      StringRef ٹیگ_Handle = Raw.substr(0, Raw.لبھو_آخری_دا('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->get_ٹیگ_Map().لبھو(ٹیگ_Handle);
      إذا (It != Doc->get_ٹیگ_Map().سرا())
        Ret = std::string(It->دوجا_2);
      وإلا {
        Token T;
        T.قسم_2 = Token::TK_ٹیگ;
        T.Range = ٹیگ_Handle;
        سیٹ_Error(Twine("Unknown tag handle ") + ٹیگ_Handle, T);
      }
      Ret += Raw.substr(Raw.لبھو_آخری_دا('!') + 1);
      أرجع Ret;
    }
  }

  بدل (getType()) {
  حالة NK_Null:
    أرجع "tag:yaml.org,2002:null";
  حالة NK_Scalar:
  حالة NK_بلاک_Scalar:
    // TODO: Tag resolution.
    أرجع "tag:yaml.org,2002:str";
  حالة NK_Mapping:
    أرجع "tag:yaml.org,2002:map";
  حالة NK_Sequence:
    أرجع "tag:yaml.org,2002:seq";
  }

  أرجع "";
}

Token &گرہ::peek_اگلا() {
  أرجع Doc->peek_اگلا();
}

Token گرہ::get_اگلا() {
  أرجع Doc->get_اگلا();
}

گرہ *گرہ::parse_بلاک_گرہ() {
  أرجع Doc->parse_بلاک_گرہ();
}

BumpPtrAllocator &گرہ::getAllocator() {
  أرجع Doc->گرہ_Allocator;
}

فراغ گرہ::سیٹ_Error(ثابت Twine &پیغام_2, Token &Tok) ثابت {
  Doc->سیٹ_Error(پیغام_2, Tok);
}

منطقي گرہ::failed() ثابت {
  أرجع Doc->failed();
}

StringRef Scalar_گرہ::get_قدر(Small_سمتیہ_Impl<حرف> &Storage) ثابت {
  إذا (قدر[0] == '"')
    أرجع get_Double_Quoted_قدر(قدر, Storage);
  إذا (قدر[0] == '\'')
    أرجع get_Single_Quoted_قدر(قدر, Storage);
  أرجع get_Plain_قدر(قدر, Storage);
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
ساكن StringRef
parse_Scalar_قدر(StringRef Unquoted_قدر, Small_سمتیہ_Impl<حرف> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, Small_سمتیہ_Impl<حرف> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_قدر.لبھو_پہلا_دا(LookupChars);
  إذا (I == StringRef::npos)
    أرجع Unquoted_قدر;

  Storage.clear();
  Storage.reserve(Unquoted_قدر.ناپ_2());
  حرف آخری_New_سطر_Added_As = '\0';
  لكل (; I != StringRef::npos; I = Unquoted_قدر.لبھو_پہلا_دا(LookupChars)) {
    إذا (Unquoted_قدر[I] != '\r' && Unquoted_قدر[I] != '\n') {
      llvm::append_range(Storage, Unquoted_قدر.take_front(I));
      Unquoted_قدر = UnescapeCallback(Unquoted_قدر.drop_front(I), Storage);
      آخری_New_سطر_Added_As = '\0';
      استمر;
    }
    إذا (size_t آخری_Non_SWhite = Unquoted_قدر.لبھو_آخری_not_دا(" \t", I);
        آخری_Non_SWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_قدر.take_front(آخری_Non_SWhite + 1));
      Storage.ڈالو_پٹھ(' ');
      آخری_New_سطر_Added_As = ' ';
    } وإلا {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      بدل (آخری_New_سطر_Added_As) {
      حالة ' ':
        أكد(!Storage.خالی_تھاواں() && Storage.پٹھ() == ' ');
        Storage.پٹھ() = '\n';
        آخری_New_سطر_Added_As = '\n';
        اكسر;
      حالة '\n':
        أكد(!Storage.خالی_تھاواں() && Storage.پٹھ() == '\n');
        Storage.ڈالو_پٹھ('\n');
        اكسر;
      افتراضي:
        Storage.ڈالو_پٹھ(' ');
        آخری_New_سطر_Added_As = ' ';
        اكسر;
      }
    }
    // Handle Windows-style EOL
    إذا (Unquoted_قدر.substr(I, 2) == "\r\n")
      I++;
    Unquoted_قدر = Unquoted_قدر.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_قدر);
  أرجع StringRef(Storage.begin(), Storage.ناپ_2());
}

StringRef
Scalar_گرہ::get_Double_Quoted_قدر(StringRef Raw_قدر,
                                 Small_سمتیہ_Impl<حرف> &Storage) ثابت {
  أكد(Raw_قدر.ناپ_2() >= 2 && Raw_قدر.front() == '"' &&
         Raw_قدر.پٹھ() == '"');
  StringRef Unquoted_قدر = Raw_قدر.substr(1, Raw_قدر.ناپ_2() - 2);

  خودکار UnescapeFunc = [هذا](StringRef Unquoted_قدر,
                             Small_سمتیہ_Impl<حرف> &Storage) {
    أكد(Unquoted_قدر.take_front(1) == "\\");
    إذا (Unquoted_قدر.ناپ_2() == 1) {
      Token T;
      T.Range = Unquoted_قدر;
      سیٹ_Error("Unrecognized escape code", T);
      Storage.clear();
      أرجع StringRef();
    }
    Unquoted_قدر = Unquoted_قدر.drop_front(1);
    بدل (Unquoted_قدر[0]) {
    افتراضي: {
      Token T;
      T.Range = Unquoted_قدر.take_front(1);
      سیٹ_Error("Unrecognized escape code", T);
      Storage.clear();
      أرجع StringRef();
    }
    حالة '\r':
      // Shrink the Windows-style EOL.
      إذا (Unquoted_قدر.ناپ_2() >= 2 && Unquoted_قدر[1] == '\n')
        Unquoted_قدر = Unquoted_قدر.drop_front(1);
      [[fallthrough]];
    حالة '\n':
      أرجع Unquoted_قدر.drop_front(1).ltrim(" \t");
    حالة '0':
      Storage.ڈالو_پٹھ(0x00);
      اكسر;
    حالة 'a':
      Storage.ڈالو_پٹھ(0x07);
      اكسر;
    حالة 'b':
      Storage.ڈالو_پٹھ(0x08);
      اكسر;
    حالة 't':
    حالة 0x09:
      Storage.ڈالو_پٹھ(0x09);
      اكسر;
    حالة 'n':
      Storage.ڈالو_پٹھ(0x0A);
      اكسر;
    حالة 'v':
      Storage.ڈالو_پٹھ(0x0B);
      اكسر;
    حالة 'f':
      Storage.ڈالو_پٹھ(0x0C);
      اكسر;
    حالة 'r':
      Storage.ڈالو_پٹھ(0x0D);
      اكسر;
    حالة 'e':
      Storage.ڈالو_پٹھ(0x1B);
      اكسر;
    حالة ' ':
      Storage.ڈالو_پٹھ(0x20);
      اكسر;
    حالة '"':
      Storage.ڈالو_پٹھ(0x22);
      اكسر;
    حالة '/':
      Storage.ڈالو_پٹھ(0x2F);
      اكسر;
    حالة '\\':
      Storage.ڈالو_پٹھ(0x5C);
      اكسر;
    حالة 'N':
      encodeUTF8(0x85, Storage);
      اكسر;
    حالة '_':
      encodeUTF8(0xA0, Storage);
      اكسر;
    حالة 'L':
      encodeUTF8(0x2028, Storage);
      اكسر;
    حالة 'P':
      encodeUTF8(0x2029, Storage);
      اكسر;
    حالة 'x': {
      إذا (Unquoted_قدر.ناپ_2() < 3)
        // TODO: Report error.
        اكسر;
      بنانشانی صحيح Unicode_Scalar_قدر;
      إذا (Unquoted_قدر.substr(1, 2).getAsInteger(16, Unicode_Scalar_قدر))
        // TODO: Report error.
        Unicode_Scalar_قدر = 0xFFFD;
      encodeUTF8(Unicode_Scalar_قدر, Storage);
      أرجع Unquoted_قدر.drop_front(3);
    }
    حالة 'u': {
      إذا (Unquoted_قدر.ناپ_2() < 5)
        // TODO: Report error.
        اكسر;
      بنانشانی صحيح Unicode_Scalar_قدر;
      إذا (Unquoted_قدر.substr(1, 4).getAsInteger(16, Unicode_Scalar_قدر))
        // TODO: Report error.
        Unicode_Scalar_قدر = 0xFFFD;
      encodeUTF8(Unicode_Scalar_قدر, Storage);
      أرجع Unquoted_قدر.drop_front(5);
    }
    حالة 'U': {
      إذا (Unquoted_قدر.ناپ_2() < 9)
        // TODO: Report error.
        اكسر;
      بنانشانی صحيح Unicode_Scalar_قدر;
      إذا (Unquoted_قدر.substr(1, 8).getAsInteger(16, Unicode_Scalar_قدر))
        // TODO: Report error.
        Unicode_Scalar_قدر = 0xFFFD;
      encodeUTF8(Unicode_Scalar_قدر, Storage);
      أرجع Unquoted_قدر.drop_front(9);
    }
    }
    أرجع Unquoted_قدر.drop_front(1);
  };

  أرجع parse_Scalar_قدر(Unquoted_قدر, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_گرہ::get_Single_Quoted_قدر(StringRef Raw_قدر,
                                           Small_سمتیہ_Impl<حرف> &Storage) {
  أكد(Raw_قدر.ناپ_2() >= 2 && Raw_قدر.front() == '\'' &&
         Raw_قدر.پٹھ() == '\'');
  StringRef Unquoted_قدر = Raw_قدر.substr(1, Raw_قدر.ناپ_2() - 2);

  خودکار UnescapeFunc = [](StringRef Unquoted_قدر,
                         Small_سمتیہ_Impl<حرف> &Storage) {
    أكد(Unquoted_قدر.take_front(2) == "''");
    Storage.ڈالو_پٹھ('\'');
    أرجع Unquoted_قدر.drop_front(2);
  };

  أرجع parse_Scalar_قدر(Unquoted_قدر, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_گرہ::get_Plain_قدر(StringRef Raw_قدر,
                                    Small_سمتیہ_Impl<حرف> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_قدر = Raw_قدر.rtrim("\r\n \t");
  أرجع parse_Scalar_قدر(Raw_قدر, Storage, "\r\n", خالی_سوچک);
}

گرہ *کلید_قدر_گرہ::get_کلید() {
  إذا (کلید)
    أرجع کلید;
  // Handle implicit null keys.
  {
    Token &t = peek_اگلا();
    إذا (   t.قسم_2 == Token::TK_بلاک_سرا
        || t.قسم_2 == Token::TK_قدر
        || t.قسم_2 == Token::TK_Error) {
      أرجع کلید = جديد (getAllocator()) Null_گرہ(Doc);
    }
    إذا (t.قسم_2 == Token::TK_کلید)
      get_اگلا(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peek_اگلا();
  إذا (t.قسم_2 == Token::TK_بلاک_سرا || t.قسم_2 == Token::TK_قدر) {
    أرجع کلید = جديد (getAllocator()) Null_گرہ(Doc);
  }

  // We've got a normal key.
  أرجع کلید = parse_بلاک_گرہ();
}

گرہ *کلید_قدر_گرہ::get_قدر() {
  إذا (قدر)
    أرجع قدر;

  إذا (گرہ* کلید = get_کلید())
    کلید->skip();
  وإلا {
    سیٹ_Error("Null key in Key Value.", peek_اگلا());
    أرجع قدر = جديد (getAllocator()) Null_گرہ(Doc);
  }

  إذا (failed())
    أرجع قدر = جديد (getAllocator()) Null_گرہ(Doc);

  // Handle implicit null values.
  {
    Token &t = peek_اگلا();
    إذا (   t.قسم_2 == Token::TK_بلاک_سرا
        || t.قسم_2 == Token::TK_Flow_Mapping_سرا
        || t.قسم_2 == Token::TK_کلید
        || t.قسم_2 == Token::TK_Flow_اندراج
        || t.قسم_2 == Token::TK_Error) {
      أرجع قدر = جديد (getAllocator()) Null_گرہ(Doc);
    }

    إذا (t.قسم_2 != Token::TK_قدر) {
      سیٹ_Error("Unexpected token in Key Value.", t);
      أرجع قدر = جديد (getAllocator()) Null_گرہ(Doc);
    }
    get_اگلا(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peek_اگلا();
  إذا (t.قسم_2 == Token::TK_بلاک_سرا || t.قسم_2 == Token::TK_کلید) {
    أرجع قدر = جديد (getAllocator()) Null_گرہ(Doc);
  }

  // We got a normal value.
  أرجع قدر = parse_بلاک_گرہ();
}

فراغ Mapping_گرہ::increment() {
  إذا (failed()) {
    Is_At_سرا = سچ;
    Current_اندراج = خالی_سوچک;
    أرجع;
  }
  إذا (Current_اندراج) {
    Current_اندراج->skip();
    إذا (Type == MT_Inline) {
      Is_At_سرا = سچ;
      Current_اندراج = خالی_سوچک;
      أرجع;
    }
  }
  Token T = peek_اگلا();
  إذا (T.قسم_2 == Token::TK_کلید || T.قسم_2 == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    Current_اندراج = جديد (getAllocator()) کلید_قدر_گرہ(Doc);
  } وإلا إذا (Type == MT_بلاک) {
    بدل (T.قسم_2) {
    حالة Token::TK_بلاک_سرا:
      get_اگلا();
      Is_At_سرا = سچ;
      Current_اندراج = خالی_سوچک;
      اكسر;
    افتراضي:
      سیٹ_Error("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    حالة Token::TK_Error:
      Is_At_سرا = سچ;
      Current_اندراج = خالی_سوچک;
    }
  } وإلا {
    بدل (T.قسم_2) {
    حالة Token::TK_Flow_اندراج:
      // Eat the flow entry and recurse.
      get_اگلا();
      أرجع increment();
    حالة Token::TK_Flow_Mapping_سرا:
      get_اگلا();
      [[fallthrough]];
    حالة Token::TK_Error:
      // Set this to end iterator.
      Is_At_سرا = سچ;
      Current_اندراج = خالی_سوچک;
      اكسر;
    افتراضي:
      سیٹ_Error( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      Is_At_سرا = سچ;
      Current_اندراج = خالی_سوچک;
    }
  }
}

فراغ Sequence_گرہ::increment() {
  إذا (failed()) {
    Is_At_سرا = سچ;
    Current_اندراج = خالی_سوچک;
    أرجع;
  }
  إذا (Current_اندراج)
    Current_اندراج->skip();
  Token T = peek_اگلا();
  إذا (SeqType == ST_بلاک) {
    بدل (T.قسم_2) {
    حالة Token::TK_بلاک_اندراج:
      get_اگلا();
      Current_اندراج = parse_بلاک_گرہ();
      إذا (!Current_اندراج) { // An error occurred.
        Is_At_سرا = سچ;
        Current_اندراج = خالی_سوچک;
      }
      اكسر;
    حالة Token::TK_بلاک_سرا:
      get_اگلا();
      Is_At_سرا = سچ;
      Current_اندراج = خالی_سوچک;
      اكسر;
    افتراضي:
      سیٹ_Error( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    حالة Token::TK_Error:
      Is_At_سرا = سچ;
      Current_اندراج = خالی_سوچک;
    }
  } وإلا إذا (SeqType == ST_Indentless) {
    بدل (T.قسم_2) {
    حالة Token::TK_بلاک_اندراج:
      get_اگلا();
      Current_اندراج = parse_بلاک_گرہ();
      إذا (!Current_اندراج) { // An error occurred.
        Is_At_سرا = سچ;
        Current_اندراج = خالی_سوچک;
      }
      اكسر;
    افتراضي:
    حالة Token::TK_Error:
      Is_At_سرا = سچ;
      Current_اندراج = خالی_سوچک;
    }
  } وإلا إذا (SeqType == ST_Flow) {
    بدل (T.قسم_2) {
    حالة Token::TK_Flow_اندراج:
      // Eat the flow entry and recurse.
      get_اگلا();
      Was_پچھلا_Token_Flow_اندراج = سچ;
      أرجع increment();
    حالة Token::TK_Flow_Sequence_سرا:
      get_اگلا();
      [[fallthrough]];
    حالة Token::TK_Error:
      // Set this to end iterator.
      Is_At_سرا = سچ;
      Current_اندراج = خالی_سوچک;
      اكسر;
    حالة Token::TK_دھارا_سرا:
    حالة Token::TK_Document_سرا:
    حالة Token::TK_Document_شروع:
      سیٹ_Error("Could not find closing ]!", T);
      // Set this to end iterator.
      Is_At_سرا = سچ;
      Current_اندراج = خالی_سوچک;
      اكسر;
    افتراضي:
      إذا (!Was_پچھلا_Token_Flow_اندراج) {
        سیٹ_Error("Expected , between entries!", T);
        Is_At_سرا = سچ;
        Current_اندراج = خالی_سوچک;
        اكسر;
      }
      // Otherwise it must be a flow entry.
      Current_اندراج = parse_بلاک_گرہ();
      إذا (!Current_اندراج) {
        Is_At_سرا = سچ;
      }
      Was_پچھلا_Token_Flow_اندراج = خطأ;
      اكسر;
    }
  }
}

Document::Document(دھارا &S) : دھارا_2(S), جڑ_2(خالی_سوچک) {
  // Tag maps starts with two default mappings.
  ٹیگ_Map["!"] = "!";
  ٹیگ_Map["!!"] = "tag:yaml.org,2002:";

  إذا (parseDirectives())
    expectToken(Token::TK_Document_شروع);
  Token &T = peek_اگلا();
  إذا (T.قسم_2 == Token::TK_Document_شروع)
    get_اگلا();
}

منطقي Document::skip()  {
  إذا (دھارا_2.scanner->failed())
    أرجع خطأ;
  إذا (!جڑ_2 && !get_جڑ())
    أرجع خطأ;
  جڑ_2->skip();
  Token &T = peek_اگلا();
  إذا (T.قسم_2 == Token::TK_دھارا_سرا)
    أرجع خطأ;
  إذا (T.قسم_2 == Token::TK_Document_سرا) {
    get_اگلا();
    أرجع skip();
  }
  أرجع سچ;
}

Token &Document::peek_اگلا() {
  أرجع دھارا_2.scanner->peek_اگلا();
}

Token Document::get_اگلا() {
  أرجع دھارا_2.scanner->get_اگلا();
}

فراغ Document::سیٹ_Error(ثابت Twine &سنیہا_2, Token &Location) ثابت {
  دھارا_2.scanner->سیٹ_Error(سنیہا_2, Location.Range.begin());
}

منطقي Document::failed() ثابت {
  أرجع دھارا_2.scanner->failed();
}

گرہ *Document::parse_بلاک_گرہ() {
  Token T = peek_اگلا();
  // Handle properties.
  Token AnchorInfo;
  Token ٹیگ_Info;
parse_property:
  بدل (T.قسم_2) {
  حالة Token::TK_Alias:
    get_اگلا();
    أرجع جديد (گرہ_Allocator) Alias_گرہ(دھارا_2.CurrentDoc, T.Range.substr(1));
  حالة Token::TK_Anchor:
    إذا (AnchorInfo.قسم_2 == Token::TK_Anchor) {
      سیٹ_Error("Already encountered an anchor for this node!", T);
      أرجع خالی_سوچک;
    }
    AnchorInfo = get_اگلا(); // Consume TK_Anchor.
    T = peek_اگلا();
    اذهب parse_property;
  حالة Token::TK_ٹیگ:
    إذا (ٹیگ_Info.قسم_2 == Token::TK_ٹیگ) {
      سیٹ_Error("Already encountered a tag for this node!", T);
      أرجع خالی_سوچک;
    }
    ٹیگ_Info = get_اگلا(); // Consume TK_Tag.
    T = peek_اگلا();
    اذهب parse_property;
  افتراضي:
    اكسر;
  }

  بدل (T.قسم_2) {
  حالة Token::TK_بلاک_اندراج:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    أرجع جديد (گرہ_Allocator) Sequence_گرہ( دھارا_2.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , ٹیگ_Info.Range
                                           , Sequence_گرہ::ST_Indentless);
  حالة Token::TK_بلاک_Sequence_شروع:
    get_اگلا();
    أرجع جديد (گرہ_Allocator)
      Sequence_گرہ( دھارا_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , ٹیگ_Info.Range
                  , Sequence_گرہ::ST_بلاک);
  حالة Token::TK_بلاک_Mapping_شروع:
    get_اگلا();
    أرجع جديد (گرہ_Allocator)
      Mapping_گرہ( دھارا_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , ٹیگ_Info.Range
                 , Mapping_گرہ::MT_بلاک);
  حالة Token::TK_Flow_Sequence_شروع:
    get_اگلا();
    أرجع جديد (گرہ_Allocator)
      Sequence_گرہ( دھارا_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , ٹیگ_Info.Range
                  , Sequence_گرہ::ST_Flow);
  حالة Token::TK_Flow_Mapping_شروع:
    get_اگلا();
    أرجع جديد (گرہ_Allocator)
      Mapping_گرہ( دھارا_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , ٹیگ_Info.Range
                 , Mapping_گرہ::MT_Flow);
  حالة Token::TK_Scalar:
    get_اگلا();
    أرجع جديد (گرہ_Allocator)
      Scalar_گرہ( دھارا_2.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , ٹیگ_Info.Range
                , T.Range);
  حالة Token::TK_بلاک_Scalar: {
    get_اگلا();
    StringRef NullTerminatedStr(T.قدر.c_str(), T.قدر.لمائی_2() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(گرہ_Allocator).drop_پٹھ();
    أرجع جديد (گرہ_Allocator)
        بلاک_Scalar_گرہ(دھارا_2.CurrentDoc, AnchorInfo.Range.substr(1),
                        ٹیگ_Info.Range, StrCopy, T.Range);
  }
  حالة Token::TK_کلید:
    // Don't eat the TK_Key, KeyValueNode expects it.
    أرجع جديد (گرہ_Allocator)
      Mapping_گرہ( دھارا_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , ٹیگ_Info.Range
                 , Mapping_گرہ::MT_Inline);
  حالة Token::TK_Document_شروع:
  حالة Token::TK_Document_سرا:
  حالة Token::TK_دھارا_سرا:
  افتراضي:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    أرجع جديد (گرہ_Allocator) Null_گرہ(دھارا_2.CurrentDoc);
  حالة Token::TK_Flow_Mapping_سرا:
  حالة Token::TK_Flow_Sequence_سرا:
  حالة Token::TK_Flow_اندراج: {
    إذا (جڑ_2 && (isa<Mapping_گرہ>(جڑ_2) || isa<Sequence_گرہ>(جڑ_2)))
      أرجع جديد (گرہ_Allocator) Null_گرہ(دھارا_2.CurrentDoc);

    سیٹ_Error("Unexpected token", T);
    أرجع خالی_سوچک;
  }
  حالة Token::TK_Error:
    أرجع خالی_سوچک;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  أرجع خالی_سوچک;
}

منطقي Document::parseDirectives() {
  منطقي isDirective = خطأ;
  طالما (سچ) {
    Token T = peek_اگلا();
    إذا (T.قسم_2 == Token::TK_ٹیگ_Directive) {
      parseTAGDirective();
      isDirective = سچ;
    } وإلا إذا (T.قسم_2 == Token::TK_نسخہ_Directive) {
      parseYAMLDirective();
      isDirective = سچ;
    } وإلا {
      اكسر;
    }
  }
  أرجع isDirective;
}

فراغ Document::parseYAMLDirective() {
  get_اگلا(); // Eat %YAML <version>
}

فراغ Document::parseTAGDirective() {
  Token ٹیگ = get_اگلا(); // %TAG <handle> <prefix>
  StringRef T = ٹیگ.Range;
  // Strip %TAG
  T = T.substr(T.لبھو_پہلا_دا(" \t")).ltrim(" \t");
  std::size_t Handle_سرا = T.لبھو_پہلا_دا(" \t");
  StringRef ٹیگ_Handle = T.substr(0, Handle_سرا);
  StringRef ٹیگ_اگیتر = T.substr(Handle_سرا).ltrim(" \t");
  ٹیگ_Map[ٹیگ_Handle] = ٹیگ_اگیتر;
}

منطقي Document::expectToken(صحيح TK) {
  Token T = get_اگلا();
  إذا (T.قسم_2 != TK) {
    سیٹ_Error("Unexpected token", T);
    أرجع خطأ;
  }
  أرجع سچ;
}
