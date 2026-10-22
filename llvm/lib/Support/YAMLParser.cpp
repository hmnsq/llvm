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

استفاده نطاق llvm;
استفاده نطاق yaml;

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
استفاده EncodingInfo = std::pair<UnicodeEncodingForm, بیعلامت>;

/// getUnicodeEncoding - Reads up to the first 4 bytes to determine the Unicode
///                      encoding form of \a Input.
///
/// @param Input A string of length 0 or more.
/// @returns An EncodingInfo indicating the Unicode encoding form of the input
///          and how long the byte order mark is if one exists.
ساكن EncodingInfo getUnicodeEncoding(StringRef درون_داد) {
  إذا (درون_داد.خالی())
    أرجع {UEF_Unknown, 0};

  بدل (uint8_t(درون_داد[0])) {
  حالة 0x00:
    إذا (درون_داد.اندازه_2() >= 4) {
      إذا (  درون_داد[1] == 0
         && uint8_t(درون_داد[2]) == 0xFE
         && uint8_t(درون_داد[3]) == 0xFF)
        أرجع {UEF_UTF32_BE, 4};
      إذا (درون_داد[1] == 0 && درون_داد[2] == 0 && درون_داد[3] != 0)
        أرجع {UEF_UTF32_BE, 0};
    }

    إذا (درون_داد.اندازه_2() >= 2 && درون_داد[1] != 0)
      أرجع {UEF_UTF16_BE, 0};
    أرجع {UEF_Unknown, 0};
  حالة 0xFF:
    إذا (  درون_داد.اندازه_2() >= 4
       && uint8_t(درون_داد[1]) == 0xFE
       && درون_داد[2] == 0
       && درون_داد[3] == 0)
      أرجع {UEF_UTF32_LE, 4};

    إذا (درون_داد.اندازه_2() >= 2 && uint8_t(درون_داد[1]) == 0xFE)
      أرجع {UEF_UTF16_LE, 2};
    أرجع {UEF_Unknown, 0};
  حالة 0xFE:
    إذا (درون_داد.اندازه_2() >= 2 && uint8_t(درون_داد[1]) == 0xFF)
      أرجع {UEF_UTF16_BE, 2};
    أرجع {UEF_Unknown, 0};
  حالة 0xEF:
    إذا (  درون_داد.اندازه_2() >= 3
       && uint8_t(درون_داد[1]) == 0xBB
       && uint8_t(درون_داد[2]) == 0xBF)
      أرجع {UEF_UTF8, 3};
    أرجع {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  إذا (درون_داد.اندازه_2() >= 4 && درون_داد[1] == 0 && درون_داد[2] == 0 && درون_داد[3] == 0)
    أرجع {UEF_UTF32_LE, 0};

  إذا (درون_داد.اندازه_2() >= 2 && درون_داد[1] == 0)
    أرجع {UEF_UTF16_LE, 0};

  أرجع {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
فراغ گره::anchor() {}
فراغ Null_گره::anchor() {}
فراغ Scalar_گره::anchor() {}
فراغ بلوک_Scalar_گره::anchor() {}
فراغ کلید_مقدار_گره::anchor() {}
فراغ Mapping_گره::anchor() {}
فراغ Sequence_گره::anchor() {}
فراغ Alias_گره::anchor() {}

نطاق llvm {
نطاق yaml {

/// Token - A single YAML token.
بنية Token {
  تعداد Token_نوع {
    TK_Error, // Uninitialized token.
    TK_جریان_شروع,
    TK_جریان_انتها,
    TK_نسخه_Directive,
    TK_برچسب_Directive,
    TK_Document_شروع,
    TK_Document_انتها,
    TK_بلوک_مدخل,
    TK_بلوک_انتها,
    TK_بلوک_Sequence_شروع,
    TK_بلوک_Mapping_شروع,
    TK_Flow_مدخل,
    TK_Flow_Sequence_شروع,
    TK_Flow_Sequence_انتها,
    TK_Flow_Mapping_شروع,
    TK_Flow_Mapping_انتها,
    TK_کلید,
    TK_مقدار,
    TK_Scalar,
    TK_بلوک_Scalar,
    TK_Alias,
    TK_Anchor,
    TK_برچسب
  } نوع_3 = TK_Error;

  /// A string of length 0 or more whose begin() points to the logical location
  /// of the token in the input.
  StringRef Range;

  /// The value of a block scalar node.
  std::string مقدار;

  Token() = افتراضي;
};

} // end namespace yaml
} // end namespace llvm

استفاده Token_صف_T = BumpPtrList<Token>;

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
  Token_صف_T::iterator Tok;
  بیعلامت ستونی = 0;
  بیعلامت سطر = 0;
  بیعلامت FlowLevel = 0;
  منطقي IsRequired = خطأ;

  منطقي عملگر ==(ثابت Simple_کلید &Other) {
    أرجع Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
استفاده UTF8Decoded = std::pair<uint32_t, بیعلامت>;

ساكن UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator انتها_2 = Range.انتها();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  إذا (Position < انتها_2 && (*Position & 0x80) == 0) {
    أرجع {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  إذا (Position + 1 < انتها_2 && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    إذا (codepoint >= 0x80)
      أرجع {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  إذا (Position + 2 < انتها_2 && ((*Position & 0xF0) == 0xE0) &&
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
  إذا (Position + 3 < انتها_2 && ((*Position & 0xF8) == 0xF0) &&
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
  Scanner(StringRef درون_داد, منبع_Mgr &SM, منطقي ShowColors = درست,
          std::error_کد *EC = اشاره_تهی);
  Scanner(حافظه_میانگیر_Ref میانگیر, منبع_Mgr &SM_, منطقي ShowColors = درست,
          std::error_کد *EC = اشاره_تهی);

  /// Parse the next token and return it without popping it.
  Token &peek_بعدی();

  /// Parse the next token and pop it from the queue.
  Token get_بعدی();

  فراغ printError(SMLoc Loc, منبع_Mgr::Diag_نوع نوع_3, ثابت Twine &پیام_3,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.Print_پیام(Loc, نوع_3, پیام_3, Ranges, /* FixIts= */ {}, ShowColors);
  }

  فراغ مجموعه_Error(ثابت Twine &پیام_3, StringRef::iterator Position) {
    إذا (Position >= انتها_2)
      Position = انتها_2 - 1;

    // propagate the error if possible
    إذا (EC)
      *EC = make_error_کد(std::errc::invalid_آرگومان);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    إذا (!Failed)
      printError(SMLoc::get_From_اشاره_گر(Position), منبع_Mgr::DK_Error, پیام_3);
    Failed = درست;
  }

  /// Returns true if an error occurred while parsing.
  منطقي failed() {
    أرجع Failed;
  }

خاص:
  فراغ init(حافظه_میانگیر_Ref میانگیر);

  StringRef current_درون_داد() {
    أرجع StringRef(Current, انتها_2 - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    أرجع ::decodeUTF8(StringRef(Position, انتها_2 - Position));
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

  استفاده SkipWhileFunc = StringRef::iterator (Scanner::*)(StringRef::iterator);

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
  فراغ پیمایش_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  منطقي consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  فراغ skip(uint32_t فاصله_2);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  منطقي isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  منطقي is_Plain_امن_Non_Blank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  منطقي is_سطر_خالی(StringRef سطر);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  منطقي consume_سطر_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  فراغ save_Simple_کلید_Candidate( Token_صف_T::iterator Tok
                             , بیعلامت At_ستونی
                             , منطقي IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  فراغ remove_Stale_Simple_کلید_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  فراغ remove_Simple_کلید_Candidates_On_Flow_Level(بیعلامت Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  منطقي unrollIndent(صحيح مقصد_ستونی);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  منطقي rollIndent( صحيح مقصد_ستونی
                 , Token::Token_نوع نوع_3
                 , Token_صف_T::iterator درج_نقطه);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  فراغ skipComment();

  /// Skip whitespace and comments until the start of the next token.
  فراغ پیمایش_مقصد_بعدی_Token();

  /// Must be the first token generated.
  منطقي پیمایش_جریان_شروع();

  /// Generate tokens needed to close out the stream.
  منطقي پیمایش_جریان_انتها();

  /// Scan a %BLAH directive.
  منطقي پیمایش_Directive();

  /// Scan a ... or ---.
  منطقي پیمایش_Document_Indicator(منطقي Is_شروع);

  /// Scan a [ or { and generate the proper flow collection start token.
  منطقي پیمایش_Flow_Collection_شروع(منطقي IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  منطقي پیمایش_Flow_Collection_انتها(منطقي IsSequence);

  /// Scan the , that separates entries in a flow collection.
  منطقي پیمایش_Flow_مدخل();

  /// Scan the - that starts block sequence entries.
  منطقي پیمایش_بلوک_مدخل();

  /// Scan an explicit ? indicating a key.
  منطقي پیمایش_کلید();

  /// Scan an explicit : indicating a value.
  منطقي پیمایش_مقدار();

  /// Scan a quoted scalar.
  منطقي پیمایش_Flow_Scalar(منطقي IsDoubleQuoted);

  /// Scan an unquoted scalar.
  منطقي پیمایش_Plain_Scalar();

  /// Scan an Alias or Anchor starting with * or &.
  منطقي پیمایش_Alias_Or_Anchor(منطقي IsAlias);

  /// Scan a block scalar starting with | or >.
  منطقي پیمایش_بلوک_Scalar(منطقي IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  منطقي پیمایش_بلوک_Scalar_Indicators(حرف &StyleIndicator, حرف &ChompingIndicator,
                                 بیعلامت &IndentIndicator, منطقي &Is_انجامشده);

  /// Scan a style indicator in a block scalar header.
  حرف پیمایش_بلوک_Style_Indicator();

  /// Scan a chomping indicator in a block scalar header.
  حرف پیمایش_بلوک_Chomping_Indicator();

  /// Scan an indentation indicator in a block scalar header.
  بیعلامت پیمایش_بلوک_Indentation_Indicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  منطقي پیمایش_بلوک_Scalar_سرآیند(حرف &ChompingIndicator, بیعلامت &IndentIndicator,
                             منطقي &Is_انجامشده);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  منطقي پیدا_کن_بلوک_Scalar_Indent(بیعلامت &بلوک_Indent, بیعلامت بلوک_Exit_Indent,
                             بیعلامت &سطر_Breaks, منطقي &Is_انجامشده);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  منطقي پیمایش_بلوک_Scalar_Indent(بیعلامت بلوک_Indent, بیعلامت بلوک_Exit_Indent,
                             منطقي &Is_انجامشده);

  /// Scan a tag of the form !stuff.
  منطقي پیمایش_برچسب();

  /// Dispatch to the next scanning function based on \a *Cur.
  منطقي fetch_بیشتر_نشانهها();

  /// The SourceMgr used for diagnostics and buffer management.
  منبع_Mgr &SM;

  /// The original input.
  حافظه_میانگیر_Ref درون_داد_میانگیر;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator انتها_2;

  /// Current YAML indentation level in spaces.
  صحيح Indent;

  /// Current column number in Unicode code points.
  بیعلامت ستونی;

  /// Current line number.
  بیعلامت سطر;

  /// How deep we are in flow style containers. 0 Means at block level.
  بیعلامت FlowLevel;

  /// Are we at the start of the stream?
  منطقي Is_شروع_از_جریان;

  /// Can the next token be the start of a simple key?
  منطقي Is_Simple_کلید_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  منطقي Is_Adjacent_مقدار_Allowed_درون_Flow;

  /// True if an error has occurred.
  منطقي Failed;

  /// Should colors be used when printing out the diagnostic messages?
  منطقي ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  Token_صف_T Token_صف;

  /// Indentation levels.
  Small_وکتور<صحيح, 4> Indents;

  /// Potential simple keys.
  Small_وکتور<Simple_کلید, 4> Simple_کلیدها;

  std::error_کد *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
ساكن فراغ encodeUTF8( uint32_t Unicode_Scalar_مقدار
                      , Small_وکتور_Impl<حرف> &نتیجه) {
  إذا (Unicode_Scalar_مقدار <= 0x7F) {
    نتیجه.افزودن_پشت(Unicode_Scalar_مقدار & 0x7F);
  } وإلا إذا (Unicode_Scalar_مقدار <= 0x7FF) {
    uint8_t اولی_Byte = 0xC0 | ((Unicode_Scalar_مقدار & 0x7C0) >> 6);
    uint8_t دومی_Byte = 0x80 | (Unicode_Scalar_مقدار & 0x3F);
    نتیجه.افزودن_پشت(اولی_Byte);
    نتیجه.افزودن_پشت(دومی_Byte);
  } وإلا إذا (Unicode_Scalar_مقدار <= 0xFFFF) {
    uint8_t اولی_Byte = 0xE0 | ((Unicode_Scalar_مقدار & 0xF000) >> 12);
    uint8_t دومی_Byte = 0x80 | ((Unicode_Scalar_مقدار & 0xFC0) >> 6);
    uint8_t سوم_Byte = 0x80 | (Unicode_Scalar_مقدار & 0x3F);
    نتیجه.افزودن_پشت(اولی_Byte);
    نتیجه.افزودن_پشت(دومی_Byte);
    نتیجه.افزودن_پشت(سوم_Byte);
  } وإلا إذا (Unicode_Scalar_مقدار <= 0x10FFFF) {
    uint8_t اولی_Byte = 0xF0 | ((Unicode_Scalar_مقدار & 0x1F0000) >> 18);
    uint8_t دومی_Byte = 0x80 | ((Unicode_Scalar_مقدار & 0x3F000) >> 12);
    uint8_t سوم_Byte = 0x80 | ((Unicode_Scalar_مقدار & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_مقدار & 0x3F);
    نتیجه.افزودن_پشت(اولی_Byte);
    نتیجه.افزودن_پشت(دومی_Byte);
    نتیجه.افزودن_پشت(سوم_Byte);
    نتیجه.افزودن_پشت(FourthByte);
  }
}

منطقي yaml::dump_نشانهها(StringRef درون_داد, raw_ostream &OS) {
  منبع_Mgr SM;
  Scanner scanner(درون_داد, SM);
  طالما (درست) {
    Token T = scanner.get_بعدی();
    بدل (T.نوع_3) {
    حالة Token::TK_جریان_شروع:
      OS << "Stream-Start: ";
      اكسر;
    حالة Token::TK_جریان_انتها:
      OS << "Stream-End: ";
      اكسر;
    حالة Token::TK_نسخه_Directive:
      OS << "Version-Directive: ";
      اكسر;
    حالة Token::TK_برچسب_Directive:
      OS << "Tag-Directive: ";
      اكسر;
    حالة Token::TK_Document_شروع:
      OS << "Document-Start: ";
      اكسر;
    حالة Token::TK_Document_انتها:
      OS << "Document-End: ";
      اكسر;
    حالة Token::TK_بلوک_مدخل:
      OS << "Block-Entry: ";
      اكسر;
    حالة Token::TK_بلوک_انتها:
      OS << "Block-End: ";
      اكسر;
    حالة Token::TK_بلوک_Sequence_شروع:
      OS << "Block-Sequence-Start: ";
      اكسر;
    حالة Token::TK_بلوک_Mapping_شروع:
      OS << "Block-Mapping-Start: ";
      اكسر;
    حالة Token::TK_Flow_مدخل:
      OS << "Flow-Entry: ";
      اكسر;
    حالة Token::TK_Flow_Sequence_شروع:
      OS << "Flow-Sequence-Start: ";
      اكسر;
    حالة Token::TK_Flow_Sequence_انتها:
      OS << "Flow-Sequence-End: ";
      اكسر;
    حالة Token::TK_Flow_Mapping_شروع:
      OS << "Flow-Mapping-Start: ";
      اكسر;
    حالة Token::TK_Flow_Mapping_انتها:
      OS << "Flow-Mapping-End: ";
      اكسر;
    حالة Token::TK_کلید:
      OS << "Key: ";
      اكسر;
    حالة Token::TK_مقدار:
      OS << "Value: ";
      اكسر;
    حالة Token::TK_Scalar:
      OS << "Scalar: ";
      اكسر;
    حالة Token::TK_بلوک_Scalar:
      OS << "Block Scalar: ";
      اكسر;
    حالة Token::TK_Alias:
      OS << "Alias: ";
      اكسر;
    حالة Token::TK_Anchor:
      OS << "Anchor: ";
      اكسر;
    حالة Token::TK_برچسب:
      OS << "Tag: ";
      اكسر;
    حالة Token::TK_Error:
      اكسر;
    }
    OS << T.Range << "\n";
    إذا (T.نوع_3 == Token::TK_جریان_انتها)
      اكسر;
    وإلا إذا (T.نوع_3 == Token::TK_Error)
      أرجع خطأ;
  }
  أرجع درست;
}

منطقي yaml::پیمایش_نشانهها(StringRef درون_داد) {
  منبع_Mgr SM;
  Scanner scanner(درون_داد, SM);
  طالما (درست) {
    Token T = scanner.get_بعدی();
    إذا (T.نوع_3 == Token::TK_جریان_انتها)
      اكسر;
    وإلا إذا (T.نوع_3 == Token::TK_Error)
      أرجع خطأ;
  }
  أرجع درست;
}

std::string yaml::escape(StringRef درون_داد, منطقي EscapePrintable) {
  std::string Escaped_درون_داد;
  لكل (StringRef::iterator i = درون_داد.begin(), e = درون_داد.انتها(); i != e; ++i) {
    إذا (*i == '\\')
      Escaped_درون_داد += "\\\\";
    وإلا إذا (*i == '"')
      Escaped_درون_داد += "\\\"";
    وإلا إذا (*i == 0)
      Escaped_درون_داد += "\\0";
    وإلا إذا (*i == 0x07)
      Escaped_درون_داد += "\\a";
    وإلا إذا (*i == 0x08)
      Escaped_درون_داد += "\\b";
    وإلا إذا (*i == 0x09)
      Escaped_درون_داد += "\\t";
    وإلا إذا (*i == 0x0A)
      Escaped_درون_داد += "\\n";
    وإلا إذا (*i == 0x0B)
      Escaped_درون_داد += "\\v";
    وإلا إذا (*i == 0x0C)
      Escaped_درون_داد += "\\f";
    وإلا إذا (*i == 0x0D)
      Escaped_درون_داد += "\\r";
    وإلا إذا (*i == 0x1B)
      Escaped_درون_داد += "\\e";
    وإلا إذا ((بیعلامت حرف)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_درون_داد += "\\x" + std::string(2 - HexStr.اندازه_2(), '0') + HexStr;
    } وإلا إذا (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_مقدار
        = decodeUTF8(StringRef(i, درون_داد.انتها() - i));
      إذا (Unicode_Scalar_مقدار.دومی_2 == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_درون_داد, Val);
        // FIXME: Error reporting.
        أرجع Escaped_درون_داد;
      }
      إذا (Unicode_Scalar_مقدار.اولی_2 == 0x85)
        Escaped_درون_داد += "\\N";
      وإلا إذا (Unicode_Scalar_مقدار.اولی_2 == 0xA0)
        Escaped_درون_داد += "\\_";
      وإلا إذا (Unicode_Scalar_مقدار.اولی_2 == 0x2028)
        Escaped_درون_داد += "\\L";
      وإلا إذا (Unicode_Scalar_مقدار.اولی_2 == 0x2029)
        Escaped_درون_داد += "\\P";
      وإلا إذا (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_مقدار.اولی_2))
        Escaped_درون_داد += StringRef(i, Unicode_Scalar_مقدار.دومی_2);
      وإلا {
        std::string HexStr = utohexstr(Unicode_Scalar_مقدار.اولی_2);
        إذا (HexStr.اندازه_2() <= 2)
          Escaped_درون_داد += "\\x" + std::string(2 - HexStr.اندازه_2(), '0') + HexStr;
        وإلا إذا (HexStr.اندازه_2() <= 4)
          Escaped_درون_داد += "\\u" + std::string(4 - HexStr.اندازه_2(), '0') + HexStr;
        وإلا إذا (HexStr.اندازه_2() <= 8)
          Escaped_درون_داد += "\\U" + std::string(8 - HexStr.اندازه_2(), '0') + HexStr;
      }
      i += Unicode_Scalar_مقدار.دومی_2 - 1;
    } وإلا {
      Escaped_درون_داد.افزودن_پشت(*i);
    }
  }
  أرجع Escaped_درون_داد;
}

std::optional<منطقي> yaml::parseBool(StringRef S) {
  بدل (S.اندازه_2()) {
  حالة 1:
    بدل (S.front()) {
    حالة 'y':
    حالة 'Y':
      أرجع درست;
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
        أرجع درست;
      [[fallthrough]];
    حالة 'o':
      إذا (S[1] == 'n') //[Oo]n
        أرجع درست;
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
        أرجع درست;
      [[fallthrough]];
    حالة 'y':
      إذا (S.drop_front() == "es") //[Yy]es
        أرجع درست;
      أرجع std::nullopt;
    افتراضي:
      أرجع std::nullopt;
    }
  حالة 4:
    بدل (S.front()) {
    حالة 'T':
      إذا (S.drop_front() == "RUE") // TRUE
        أرجع درست;
      [[fallthrough]];
    حالة 't':
      إذا (S.drop_front() == "rue") //[Tt]rue
        أرجع درست;
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

Scanner::Scanner(StringRef درون_داد, منبع_Mgr &sm, منطقي ShowColors,
                 std::error_کد *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(حافظه_میانگیر_Ref(درون_داد, "YAML"));
}

Scanner::Scanner(حافظه_میانگیر_Ref میانگیر, منبع_Mgr &SM_, منطقي ShowColors,
                 std::error_کد *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(میانگیر);
}

فراغ Scanner::init(حافظه_میانگیر_Ref میانگیر) {
  درون_داد_میانگیر = میانگیر;
  Current = درون_داد_میانگیر.get_میانگیر_شروع();
  انتها_2 = درون_داد_میانگیر.get_میانگیر_انتها();
  Indent = -1;
  ستونی = 0;
  سطر = 0;
  FlowLevel = 0;
  Is_شروع_از_جریان = درست;
  Is_Simple_کلید_Allowed = درست;
  Is_Adjacent_مقدار_Allowed_درون_Flow = خطأ;
  Failed = خطأ;
  std::unique_ptr<حافظه_میانگیر> درون_داد_میانگیر_Owner =
      حافظه_میانگیر::get_Mem_میانگیر(میانگیر, /*RequiresNullTerminator=*/خطأ);
  SM.جمع_کن_New_منبع_میانگیر(std::move(درون_داد_میانگیر_Owner), SMLoc());
}

Token &Scanner::peek_بعدی() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  منطقي Need_بیشتر = خطأ;
  طالما (درست) {
    إذا (Token_صف.خالی() || Need_بیشتر) {
      إذا (!fetch_بیشتر_نشانهها()) {
        Token_صف.clear();
        Simple_کلیدها.clear();
        Token_صف.افزودن_پشت(Token());
        أرجع Token_صف.front();
      }
    }
    أكد(!Token_صف.خالی() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_کلید_Candidates();
    Simple_کلید SK;
    SK.Tok = Token_صف.begin();
    إذا (!is_contained(Simple_کلیدها, SK))
      اكسر;
    وإلا
      Need_بیشتر = درست;
  }
  أرجع Token_صف.front();
}

Token Scanner::get_بعدی() {
  Token Ret = peek_بعدی();
  // TokenQueue can be empty if there was an error getting the next token.
  إذا (!Token_صف.خالی())
    Token_صف.برداشت_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  إذا (Token_صف.خالی())
    Token_صف.resetAlloc();

  أرجع Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  إذا (Position == انتها_2)
    أرجع Position;
  // Check 7 bit c-printable - b-char.
  إذا (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    أرجع Position + 1;

  // Check for valid UTF-8.
  إذا (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    إذا (   u8d.دومی_2 != 0
        && u8d.اولی_2 != 0xFEFF
        && ( u8d.اولی_2 == 0x85
          || ( u8d.اولی_2 >= 0xA0
            && u8d.اولی_2 <= 0xD7FF)
          || ( u8d.اولی_2 >= 0xE000
            && u8d.اولی_2 <= 0xFFFD)
          || ( u8d.اولی_2 >= 0x10000
            && u8d.اولی_2 <= 0x10FFFF)))
      أرجع Position + u8d.دومی_2;
  }
  أرجع Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  إذا (Position == انتها_2)
    أرجع Position;
  إذا (*Position == 0x0D) {
    إذا (Position + 1 != انتها_2 && *(Position + 1) == 0x0A)
      أرجع Position + 2;
    أرجع Position + 1;
  }

  إذا (*Position == 0x0A)
    أرجع Position + 1;
  أرجع Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  إذا (Position == انتها_2)
    أرجع Position;
  إذا (*Position == ' ')
    أرجع Position + 1;
  أرجع Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  إذا (Position == انتها_2)
    أرجع Position;
  إذا (*Position == ' ' || *Position == '\t')
    أرجع Position + 1;
  أرجع Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  إذا (Position == انتها_2)
    أرجع Position;
  إذا (*Position == ' ' || *Position == '\t')
    أرجع Position;
  أرجع skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  طالما (درست) {
    StringRef::iterator i = (هذا->*Func)(Position);
    إذا (i == Position)
      اكسر;
    Position = i;
  }
  أرجع Position;
}

فراغ Scanner::advanceWhile(SkipWhileFunc Func) {
  خودکار Final = skip_while(Func, Current);
  ستونی += Final - Current;
  Current = Final;
}

ساكن منطقي is_ns_hex_digit(ثابت حرف C) { أرجع isAlnum(C); }

ساكن منطقي is_ns_کلمه_char(ثابت حرف C) { أرجع C == '-' || isAlpha(C); }

فراغ Scanner::پیمایش_ns_uri_char() {
  طالما (درست) {
    إذا (Current == انتها_2)
      اكسر;
    إذا ((*Current == '%' && Current + 2 < انتها_2 &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_کلمه_char(*Current) ||
        StringRef(Current, 1).پیدا_کن_اولی_از("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++ستونی;
    } وإلا {
      اكسر;
    }
  }
}

منطقي Scanner::consume(uint32_t Expected) {
  إذا (Expected >= 0x80) {
    مجموعه_Error("Cannot consume non-ascii characters", Current);
    أرجع خطأ;
  }
  إذا (Current == انتها_2)
    أرجع خطأ;
  إذا (uint8_t(*Current) >= 0x80) {
    مجموعه_Error("Cannot consume non-ascii characters", Current);
    أرجع خطأ;
  }
  إذا (uint8_t(*Current) == Expected) {
    ++Current;
    ++ستونی;
    أرجع درست;
  }
  أرجع خطأ;
}

فراغ Scanner::skip(uint32_t فاصله_2) {
  Current += فاصله_2;
  ستونی += فاصله_2;
  أكد(Current <= انتها_2 && "Skipped past the end");
}

منطقي Scanner::isBlankOrBreak(StringRef::iterator Position) {
  إذا (Position == انتها_2)
    أرجع خطأ;
  أرجع *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

منطقي Scanner::is_Plain_امن_Non_Blank(StringRef::iterator Position) {
  إذا (Position == انتها_2 || isBlankOrBreak(Position))
    أرجع خطأ;
  إذا (FlowLevel &&
      StringRef(Position, 1).پیدا_کن_اولی_از(",[]{}") != StringRef::npos)
    أرجع خطأ;
  أرجع درست;
}

منطقي Scanner::is_سطر_خالی(StringRef سطر) {
  لكل (ثابت خودکار *Position = سطر.begin(); Position != سطر.انتها(); ++Position)
    إذا (!isBlankOrBreak(Position))
      أرجع خطأ;
  أرجع درست;
}

منطقي Scanner::consume_سطر_Break_If_Present() {
  خودکار بعدی_2 = skip_b_break(Current);
  إذا (بعدی_2 == Current)
    أرجع خطأ;
  ستونی = 0;
  ++سطر;
  Current = بعدی_2;
  أرجع درست;
}

فراغ Scanner::save_Simple_کلید_Candidate( Token_صف_T::iterator Tok
                                    , بیعلامت At_ستونی
                                    , منطقي IsRequired) {
  إذا (Is_Simple_کلید_Allowed) {
    Simple_کلید SK;
    SK.Tok = Tok;
    SK.سطر = سطر;
    SK.ستونی = At_ستونی;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_کلیدها.افزودن_پشت(SK);
  }
}

فراغ Scanner::remove_Stale_Simple_کلید_Candidates() {
  لكل (Small_وکتور_Impl<Simple_کلید>::iterator i = Simple_کلیدها.begin();
                                            i != Simple_کلیدها.انتها();) {
    إذا (i->سطر != سطر || i->ستونی + 1024 < ستونی) {
      إذا (i->IsRequired)
        مجموعه_Error( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_کلیدها.erase(i);
    } وإلا {
      ++i;
    }
  }
}

فراغ Scanner::remove_Simple_کلید_Candidates_On_Flow_Level(بیعلامت Level) {
  إذا (!Simple_کلیدها.خالی() && (Simple_کلیدها.انتها() - 1)->FlowLevel == Level)
    Simple_کلیدها.برداشت_پشت();
}

منطقي Scanner::unrollIndent(صحيح مقصد_ستونی) {
  Token T;
  // Indentation is ignored in flow.
  إذا (FlowLevel != 0)
    أرجع درست;

  طالما (Indent > مقصد_ستونی) {
    T.نوع_3 = Token::TK_بلوک_انتها;
    T.Range = StringRef(Current, 1);
    Token_صف.افزودن_پشت(T);
    Indent = Indents.برداشت_پشت_val();
  }

  أرجع درست;
}

منطقي Scanner::rollIndent( صحيح مقصد_ستونی
                        , Token::Token_نوع نوع_3
                        , Token_صف_T::iterator درج_نقطه) {
  إذا (FlowLevel)
    أرجع درست;
  إذا (Indent < مقصد_ستونی) {
    Indents.افزودن_پشت(Indent);
    Indent = مقصد_ستونی;

    Token T;
    T.نوع_3 = نوع_3;
    T.Range = StringRef(Current, 0);
    Token_صف.درج(درج_نقطه, T);
  }
  أرجع درست;
}

فراغ Scanner::skipComment() {
  إذا (Current == انتها_2 || *Current != '#')
    أرجع;
  طالما (درست) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    إذا (I == Current)
      اكسر;
    Current = I;
    ++ستونی;
  }
}

فراغ Scanner::پیمایش_مقصد_بعدی_Token() {
  طالما (درست) {
    طالما (Current != انتها_2 && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    إذا (i == Current)
      اكسر;
    Current = i;
    ++سطر;
    ستونی = 0;
    // New lines may start a simple key.
    إذا (!FlowLevel)
      Is_Simple_کلید_Allowed = درست;
  }
}

منطقي Scanner::پیمایش_جریان_شروع() {
  Is_شروع_از_جریان = خطأ;

  EncodingInfo EI = getUnicodeEncoding(current_درون_داد());

  Token T;
  T.نوع_3 = Token::TK_جریان_شروع;
  T.Range = StringRef(Current, EI.دومی_2);
  Token_صف.افزودن_پشت(T);
  Current += EI.دومی_2;
  أرجع درست;
}

منطقي Scanner::پیمایش_جریان_انتها() {
  // Force an ending new line if one isn't present.
  إذا (ستونی != 0) {
    ستونی = 0;
    ++سطر;
  }

  unrollIndent(-1);
  Simple_کلیدها.clear();
  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_مقدار_Allowed_درون_Flow = خطأ;

  Token T;
  T.نوع_3 = Token::TK_جریان_انتها;
  T.Range = StringRef(Current, 0);
  Token_صف.افزودن_پشت(T);
  أرجع درست;
}

منطقي Scanner::پیمایش_Directive() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_کلیدها.clear();
  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_مقدار_Allowed_درون_Flow = خطأ;

  StringRef::iterator شروع = Current;
  consume('%');
  StringRef::iterator Name_شروع = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(Name_شروع, Current - Name_شروع);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  إذا (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.نوع_3 = Token::TK_نسخه_Directive;
    T.Range = StringRef(شروع, Current - شروع);
    Token_صف.افزودن_پشت(T);
    أرجع درست;
  } وإلا إذا(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.نوع_3 = Token::TK_برچسب_Directive;
    T.Range = StringRef(شروع, Current - شروع);
    Token_صف.افزودن_پشت(T);
    أرجع درست;
  }
  أرجع خطأ;
}

منطقي Scanner::پیمایش_Document_Indicator(منطقي Is_شروع) {
  unrollIndent(-1);
  Simple_کلیدها.clear();
  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_مقدار_Allowed_درون_Flow = خطأ;

  Token T;
  T.نوع_3 = Is_شروع ? Token::TK_Document_شروع : Token::TK_Document_انتها;
  T.Range = StringRef(Current, 3);
  skip(3);
  Token_صف.افزودن_پشت(T);
  أرجع درست;
}

منطقي Scanner::پیمایش_Flow_Collection_شروع(منطقي IsSequence) {
  Token T;
  T.نوع_3 = IsSequence ? Token::TK_Flow_Sequence_شروع
                      : Token::TK_Flow_Mapping_شروع;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_صف.افزودن_پشت(T);

  // [ and { may begin a simple key.
  save_Simple_کلید_Candidate(--Token_صف.انتها(), ستونی - 1, خطأ);

  // And may also be followed by a simple key.
  Is_Simple_کلید_Allowed = درست;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_مقدار_Allowed_درون_Flow = خطأ;
  ++FlowLevel;
  أرجع درست;
}

منطقي Scanner::پیمایش_Flow_Collection_انتها(منطقي IsSequence) {
  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_مقدار_Allowed_درون_Flow = درست;
  Token T;
  T.نوع_3 = IsSequence ? Token::TK_Flow_Sequence_انتها
                      : Token::TK_Flow_Mapping_انتها;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_صف.افزودن_پشت(T);
  إذا (FlowLevel)
    --FlowLevel;
  أرجع درست;
}

منطقي Scanner::پیمایش_Flow_مدخل() {
  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = درست;
  Is_Adjacent_مقدار_Allowed_درون_Flow = خطأ;
  Token T;
  T.نوع_3 = Token::TK_Flow_مدخل;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_صف.افزودن_پشت(T);
  أرجع درست;
}

منطقي Scanner::پیمایش_بلوک_مدخل() {
  rollIndent(ستونی, Token::TK_بلوک_Sequence_شروع, Token_صف.انتها());
  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = درست;
  Is_Adjacent_مقدار_Allowed_درون_Flow = خطأ;
  Token T;
  T.نوع_3 = Token::TK_بلوک_مدخل;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_صف.افزودن_پشت(T);
  أرجع درست;
}

منطقي Scanner::پیمایش_کلید() {
  إذا (!FlowLevel)
    rollIndent(ستونی, Token::TK_بلوک_Mapping_شروع, Token_صف.انتها());

  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = !FlowLevel;
  Is_Adjacent_مقدار_Allowed_درون_Flow = خطأ;

  Token T;
  T.نوع_3 = Token::TK_کلید;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_صف.افزودن_پشت(T);
  أرجع درست;
}

منطقي Scanner::پیمایش_مقدار() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  إذا (!Simple_کلیدها.خالی()) {
    Simple_کلید SK = Simple_کلیدها.برداشت_پشت_val();
    Token T;
    T.نوع_3 = Token::TK_کلید;
    T.Range = SK.Tok->Range;
    Token_صف_T::iterator i, e;
    لكل (i = Token_صف.begin(), e = Token_صف.انتها(); i != e; ++i) {
      إذا (i == SK.Tok)
        اكسر;
    }
    إذا (i == e) {
      Failed = درست;
      أرجع خطأ;
    }
    i = Token_صف.درج(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.ستونی, Token::TK_بلوک_Mapping_شروع, i);

    Is_Simple_کلید_Allowed = خطأ;
  } وإلا {
    إذا (!FlowLevel)
      rollIndent(ستونی, Token::TK_بلوک_Mapping_شروع, Token_صف.انتها());
    Is_Simple_کلید_Allowed = !FlowLevel;
  }
  Is_Adjacent_مقدار_Allowed_درون_Flow = خطأ;

  Token T;
  T.نوع_3 = Token::TK_مقدار;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_صف.افزودن_پشت(T);
  أرجع درست;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE ساكن منطقي
wasEscaped(StringRef::iterator اولی, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
ساكن منطقي wasEscaped(StringRef::iterator اولی,
                       StringRef::iterator Position) {
  أكد(Position - 1 >= اولی);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  طالما (I >= اولی && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  أرجع (Position - 1 - I) % 2 == 1;
}

منطقي Scanner::پیمایش_Flow_Scalar(منطقي IsDoubleQuoted) {
  StringRef::iterator شروع = Current;
  بیعلامت ستون_شروع = ستونی;
  إذا (IsDoubleQuoted) {
    افعل {
      ++Current;
      طالما (Current != انتها_2 && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } طالما (   Current != انتها_2
             && *(Current - 1) == '\\'
             && wasEscaped(شروع + 1, Current));
  } وإلا {
    skip(1);
    طالما (Current != انتها_2) {
      // Skip a ' followed by another '.
      إذا (Current + 1 < انتها_2 && *Current == '\'' && *(Current + 1) == '\'') {
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
        ستونی = 0;
        ++سطر;
      } وإلا {
        إذا (i == انتها_2)
          اكسر;
        Current = i;
        ++ستونی;
      }
    }
  }

  إذا (Current == انتها_2) {
    مجموعه_Error("Expected quote at end of scalar", Current);
    أرجع خطأ;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.نوع_3 = Token::TK_Scalar;
  T.Range = StringRef(شروع, Current - شروع);
  Token_صف.افزودن_پشت(T);

  save_Simple_کلید_Candidate(--Token_صف.انتها(), ستون_شروع, خطأ);

  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_مقدار_Allowed_درون_Flow = درست;

  أرجع درست;
}

منطقي Scanner::پیمایش_Plain_Scalar() {
  StringRef::iterator شروع = Current;
  بیعلامت ستون_شروع = ستونی;
  بیعلامت LeadingBlanks = 0;
  أكد(Indent >= -1 && "Indent must be >= -1 !");
  بیعلامت indent = static_cast<بیعلامت>(Indent + 1);
  طالما (Current != انتها_2) {
    إذا (*Current == '#')
      اكسر;

    طالما (Current != انتها_2 &&
           ((*Current != ':' && is_Plain_امن_Non_Blank(Current)) ||
            (*Current == ':' && is_Plain_امن_Non_Blank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      إذا (i == Current)
        اكسر;
      Current = i;
      ++ستونی;
    }

    // Are we at the end?
    إذا (!isBlankOrBreak(Current))
      اكسر;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    طالما (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      إذا (i != Tmp) {
        إذا (LeadingBlanks && (ستونی < indent) && *Tmp == '\t') {
          مجموعه_Error("Found invalid tab character in indentation", Tmp);
          أرجع خطأ;
        }
        Tmp = i;
        ++ستونی;
      } وإلا {
        i = skip_b_break(Tmp);
        إذا (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        ستونی = 0;
        ++سطر;
      }
    }

    إذا (!FlowLevel && ستونی < indent)
      اكسر;

    Current = Tmp;
  }
  إذا (شروع == Current) {
    مجموعه_Error("Got empty plain scalar", شروع);
    أرجع خطأ;
  }
  Token T;
  T.نوع_3 = Token::TK_Scalar;
  T.Range = StringRef(شروع, Current - شروع);
  Token_صف.افزودن_پشت(T);

  // Plain scalars can be simple keys.
  save_Simple_کلید_Candidate(--Token_صف.انتها(), ستون_شروع, خطأ);

  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_مقدار_Allowed_درون_Flow = خطأ;

  أرجع درست;
}

منطقي Scanner::پیمایش_Alias_Or_Anchor(منطقي IsAlias) {
  StringRef::iterator شروع = Current;
  بیعلامت ستون_شروع = ستونی;
  skip(1);
  طالما (Current != انتها_2) {
    إذا (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      اكسر;
    StringRef::iterator i = skip_ns_char(Current);
    إذا (i == Current)
      اكسر;
    Current = i;
    ++ستونی;
  }

  إذا (شروع + 1 == Current) {
    مجموعه_Error("Got empty alias or anchor", شروع);
    أرجع خطأ;
  }

  Token T;
  T.نوع_3 = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(شروع, Current - شروع);
  Token_صف.افزودن_پشت(T);

  // Alias and anchors can be simple keys.
  save_Simple_کلید_Candidate(--Token_صف.انتها(), ستون_شروع, خطأ);

  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_مقدار_Allowed_درون_Flow = خطأ;

  أرجع درست;
}

منطقي Scanner::پیمایش_بلوک_Scalar_Indicators(حرف &StyleIndicator,
                                        حرف &ChompingIndicator,
                                        بیعلامت &IndentIndicator,
                                        منطقي &Is_انجامشده) {
  StyleIndicator = پیمایش_بلوک_Style_Indicator();
  إذا (!پیمایش_بلوک_Scalar_سرآیند(ChompingIndicator, IndentIndicator, Is_انجامشده))
    أرجع خطأ;
  أرجع درست;
}

حرف Scanner::پیمایش_بلوک_Style_Indicator() {
  حرف Indicator = ' ';
  إذا (Current != انتها_2 && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  أرجع Indicator;
}

حرف Scanner::پیمایش_بلوک_Chomping_Indicator() {
  حرف Indicator = ' ';
  إذا (Current != انتها_2 && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  أرجع Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
ساكن بیعلامت get_Chomped_سطر_Breaks(حرف ChompingIndicator,
                                     بیعلامت سطر_Breaks, StringRef Str) {
  إذا (ChompingIndicator == '-') // Strip all line breaks.
    أرجع 0;
  إذا (ChompingIndicator == '+') // Keep all line breaks.
    أرجع سطر_Breaks;
  // Clip trailing lines.
  أرجع Str.خالی() ? 0 : 1;
}

بیعلامت Scanner::پیمایش_بلوک_Indentation_Indicator() {
  بیعلامت Indent = 0;
  إذا (Current != انتها_2 && (*Current >= '1' && *Current <= '9')) {
    Indent = بیعلامت(*Current - '0');
    skip(1);
  }
  أرجع Indent;
}

منطقي Scanner::پیمایش_بلوک_Scalar_سرآیند(حرف &ChompingIndicator,
                                    بیعلامت &IndentIndicator, منطقي &Is_انجامشده) {
  خودکار شروع = Current;

  ChompingIndicator = پیمایش_بلوک_Chomping_Indicator();
  IndentIndicator = پیمایش_بلوک_Indentation_Indicator();
  // Check for the chomping indicator once again.
  إذا (ChompingIndicator == ' ')
    ChompingIndicator = پیمایش_بلوک_Chomping_Indicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  إذا (Current == انتها_2) { // EOF, we have an empty scalar.
    Token T;
    T.نوع_3 = Token::TK_بلوک_Scalar;
    T.Range = StringRef(شروع, Current - شروع);
    Token_صف.افزودن_پشت(T);
    Is_انجامشده = درست;
    أرجع درست;
  }

  إذا (!consume_سطر_Break_If_Present()) {
    مجموعه_Error("Expected a line break after block scalar header", Current);
    أرجع خطأ;
  }
  أرجع درست;
}

منطقي Scanner::پیدا_کن_بلوک_Scalar_Indent(بیعلامت &بلوک_Indent,
                                    بیعلامت بلوک_Exit_Indent,
                                    بیعلامت &سطر_Breaks, منطقي &Is_انجامشده) {
  بیعلامت Max_All_Space_سطر_Characters = 0;
  StringRef::iterator Longest_All_Space_سطر;

  طالما (درست) {
    advanceWhile(&Scanner::skip_s_space);
    إذا (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      إذا (ستونی <= بلوک_Exit_Indent) { // End of the block literal.
        Is_انجامشده = درست;
        أرجع درست;
      }
      // We found the block's indentation.
      بلوک_Indent = ستونی;
      إذا (Max_All_Space_سطر_Characters > بلوک_Indent) {
        مجموعه_Error(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_سطر);
        أرجع خطأ;
      }
      أرجع درست;
    }
    إذا (skip_b_break(Current) != Current &&
        ستونی > Max_All_Space_سطر_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_سطر_Characters = ستونی;
      Longest_All_Space_سطر = Current;
    }

    // Check for EOF.
    إذا (Current == انتها_2) {
      Is_انجامشده = درست;
      أرجع درست;
    }

    إذا (!consume_سطر_Break_If_Present()) {
      Is_انجامشده = درست;
      أرجع درست;
    }
    ++سطر_Breaks;
  }
  أرجع درست;
}

منطقي Scanner::پیمایش_بلوک_Scalar_Indent(بیعلامت بلوک_Indent,
                                    بیعلامت بلوک_Exit_Indent, منطقي &Is_انجامشده) {
  // Skip the indentation.
  طالما (ستونی < بلوک_Indent) {
    خودکار I = skip_s_space(Current);
    إذا (I == Current)
      اكسر;
    Current = I;
    ++ستونی;
  }

  إذا (skip_nb_char(Current) == Current)
    أرجع درست;

  إذا (ستونی <= بلوک_Exit_Indent) { // End of the block literal.
    Is_انجامشده = درست;
    أرجع درست;
  }

  إذا (ستونی < بلوک_Indent) {
    إذا (Current != انتها_2 && *Current == '#') { // Trailing comment.
      Is_انجامشده = درست;
      أرجع درست;
    }
    مجموعه_Error("A text line is less indented than the block scalar", Current);
    أرجع خطأ;
  }
  أرجع درست; // A normal text line.
}

منطقي Scanner::پیمایش_بلوک_Scalar(منطقي IsLiteral) {
  أكد(*Current == '|' || *Current == '>');
  حرف StyleIndicator;
  حرف ChompingIndicator;
  بیعلامت بلوک_Indent;
  منطقي Is_انجامشده = خطأ;
  إذا (!پیمایش_بلوک_Scalar_Indicators(StyleIndicator, ChompingIndicator, بلوک_Indent,
                                 Is_انجامشده))
    أرجع خطأ;
  إذا (Is_انجامشده)
    أرجع درست;
  منطقي IsFolded = StyleIndicator == '>';

  ثابت خودکار *شروع = Current;
  بیعلامت بلوک_Exit_Indent = Indent < 0 ? 0 : (بیعلامت)Indent;
  بیعلامت سطر_Breaks = 0;
  إذا (بلوک_Indent == 0) {
    إذا (!پیدا_کن_بلوک_Scalar_Indent(بلوک_Indent, بلوک_Exit_Indent, سطر_Breaks,
                               Is_انجامشده))
      أرجع خطأ;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  طالما (!Is_انجامشده) {
    إذا (!پیمایش_بلوک_Scalar_Indent(بلوک_Indent, بلوک_Exit_Indent, Is_انجامشده))
      أرجع خطأ;
    إذا (Is_انجامشده)
      اكسر;

    // Parse the current line.
    خودکار سطر_شروع = Current;
    advanceWhile(&Scanner::skip_nb_char);
    إذا (سطر_شروع != Current) {
      إذا (سطر_Breaks && IsFolded && !Scanner::is_سطر_خالی(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        إذا (سطر_Breaks == 1) {
          Str.append(سطر_Breaks,
                     is_سطر_خالی(StringRef(سطر_شروع, Current - سطر_شروع))
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
    إذا (Current == انتها_2)
      اكسر;

    إذا (!consume_سطر_Break_If_Present())
      اكسر;
    ++سطر_Breaks;
  }

  إذا (Current == انتها_2 && !سطر_Breaks)
    // Ensure that there is at least one line break before the end of file.
    سطر_Breaks = 1;
  Str.append(get_Chomped_سطر_Breaks(ChompingIndicator, سطر_Breaks, Str), '\n');

  // New lines may start a simple key.
  إذا (!FlowLevel)
    Is_Simple_کلید_Allowed = درست;
  Is_Adjacent_مقدار_Allowed_درون_Flow = خطأ;

  Token T;
  T.نوع_3 = Token::TK_بلوک_Scalar;
  T.Range = StringRef(شروع, Current - شروع);
  T.مقدار = std::string(Str);
  Token_صف.افزودن_پشت(T);
  أرجع درست;
}

منطقي Scanner::پیمایش_برچسب() {
  StringRef::iterator شروع = Current;
  بیعلامت ستون_شروع = ستونی;
  skip(1); // Eat !.
  إذا (Current == انتها_2 || isBlankOrBreak(Current)); // An empty tag.
  وإلا إذا (*Current == '<') {
    skip(1);
    پیمایش_ns_uri_char();
    إذا (!consume('>'))
      أرجع خطأ;
  } وإلا {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.نوع_3 = Token::TK_برچسب;
  T.Range = StringRef(شروع, Current - شروع);
  Token_صف.افزودن_پشت(T);

  // Tags can be simple keys.
  save_Simple_کلید_Candidate(--Token_صف.انتها(), ستون_شروع, خطأ);

  Is_Simple_کلید_Allowed = خطأ;
  Is_Adjacent_مقدار_Allowed_درون_Flow = خطأ;

  أرجع درست;
}

منطقي Scanner::fetch_بیشتر_نشانهها() {
  إذا (Is_شروع_از_جریان)
    أرجع پیمایش_جریان_شروع();

  پیمایش_مقصد_بعدی_Token();

  إذا (Current == انتها_2)
    أرجع پیمایش_جریان_انتها();

  remove_Stale_Simple_کلید_Candidates();

  unrollIndent(ستونی);

  إذا (ستونی == 0 && *Current == '%')
    أرجع پیمایش_Directive();

  إذا (ستونی == 0 && Current + 3 <= انتها_2
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == انتها_2 || isBlankOrBreak(Current + 3)))
    أرجع پیمایش_Document_Indicator(درست);

  إذا (ستونی == 0 && Current + 3 <= انتها_2
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == انتها_2 || isBlankOrBreak(Current + 3)))
    أرجع پیمایش_Document_Indicator(خطأ);

  إذا (*Current == '[')
    أرجع پیمایش_Flow_Collection_شروع(درست);

  إذا (*Current == '{')
    أرجع پیمایش_Flow_Collection_شروع(خطأ);

  إذا (*Current == ']')
    أرجع پیمایش_Flow_Collection_انتها(درست);

  إذا (*Current == '}')
    أرجع پیمایش_Flow_Collection_انتها(خطأ);

  إذا (*Current == ',')
    أرجع پیمایش_Flow_مدخل();

  إذا (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == انتها_2))
    أرجع پیمایش_بلوک_مدخل();

  إذا (*Current == '?' && (Current + 1 == انتها_2 || isBlankOrBreak(Current + 1)))
    أرجع پیمایش_کلید();

  إذا (*Current == ':' &&
      (!is_Plain_امن_Non_Blank(Current + 1) || Is_Adjacent_مقدار_Allowed_درون_Flow))
    أرجع پیمایش_مقدار();

  إذا (*Current == '*')
    أرجع پیمایش_Alias_Or_Anchor(درست);

  إذا (*Current == '&')
    أرجع پیمایش_Alias_Or_Anchor(خطأ);

  إذا (*Current == '!')
    أرجع پیمایش_برچسب();

  إذا (*Current == '|' && !FlowLevel)
    أرجع پیمایش_بلوک_Scalar(درست);

  إذا (*Current == '>' && !FlowLevel)
    أرجع پیمایش_بلوک_Scalar(خطأ);

  إذا (*Current == '\'')
    أرجع پیمایش_Flow_Scalar(خطأ);

  إذا (*Current == '"')
    أرجع پیمایش_Flow_Scalar(درست);

  // Get a plain scalar.
  StringRef اولی_Char(Current, 1);
  إذا ((!isBlankOrBreak(Current) &&
       اولی_Char.پیدا_کن_اولی_از("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (اولی_Char.پیدا_کن_اولی_از("?:-") != StringRef::npos &&
       is_Plain_امن_Non_Blank(Current + 1)))
    أرجع پیمایش_Plain_Scalar();

  مجموعه_Error("Unrecognized character while tokenizing.", Current);
  أرجع خطأ;
}

جریان::جریان(StringRef درون_داد, منبع_Mgr &SM, منطقي ShowColors,
               std::error_کد *EC)
    : scanner(جديد Scanner(درون_داد, SM, ShowColors, EC)) {}

جریان::جریان(حافظه_میانگیر_Ref درون_داد_میانگیر, منبع_Mgr &SM, منطقي ShowColors,
               std::error_کد *EC)
    : scanner(جديد Scanner(درون_داد_میانگیر, SM, ShowColors, EC)) {}

جریان::~جریان() = افتراضي;

منطقي جریان::failed() { أرجع scanner->failed(); }

فراغ جریان::printError(گره *N, ثابت Twine &پیام_4, منبع_Mgr::Diag_نوع نوع_3) {
  printError(N ? N->get_منبع_Range() : SMRange(), پیام_4, نوع_3);
}

فراغ جریان::printError(ثابت SMRange &Range, ثابت Twine &پیام_4,
                        منبع_Mgr::Diag_نوع نوع_3) {
  scanner->printError(Range.شروع, نوع_3, پیام_4, Range);
}

document_iterator جریان::begin() {
  إذا (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->get_بعدی();

  CurrentDoc.reset(جديد Document(*هذا));
  أرجع document_iterator(CurrentDoc);
}

document_iterator جریان::انتها() {
  أرجع document_iterator();
}

فراغ جریان::skip() {
  لكل (Document &Doc : *هذا)
    Doc.skip();
}

گره::گره(بیعلامت صحيح Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), برچسب(T) {
  SMLoc شروع = SMLoc::get_From_اشاره_گر(peek_بعدی().Range.begin());
  منبع_Range = SMRange(شروع, شروع);
}

std::string گره::get_Verbatim_برچسب() ثابت {
  StringRef Raw = get_Raw_برچسب();
  إذا (!Raw.خالی() && Raw != "!") {
    std::string Ret;
    إذا (Raw.پیدا_کن_آخرین_از('!') == 0) {
      Ret = std::string(Doc->get_برچسب_Map().پیدا_کن("!")->دومی_2);
      Ret += Raw.substr(1);
      أرجع Ret;
    } وإلا إذا (Raw.starts_with("!!")) {
      Ret = std::string(Doc->get_برچسب_Map().پیدا_کن("!!")->دومی_2);
      Ret += Raw.substr(2);
      أرجع Ret;
    } وإلا {
      StringRef برچسب_Handle = Raw.substr(0, Raw.پیدا_کن_آخرین_از('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->get_برچسب_Map().پیدا_کن(برچسب_Handle);
      إذا (It != Doc->get_برچسب_Map().انتها())
        Ret = std::string(It->دومی_2);
      وإلا {
        Token T;
        T.نوع_3 = Token::TK_برچسب;
        T.Range = برچسب_Handle;
        مجموعه_Error(Twine("Unknown tag handle ") + برچسب_Handle, T);
      }
      Ret += Raw.substr(Raw.پیدا_کن_آخرین_از('!') + 1);
      أرجع Ret;
    }
  }

  بدل (getType()) {
  حالة NK_Null:
    أرجع "tag:yaml.org,2002:null";
  حالة NK_Scalar:
  حالة NK_بلوک_Scalar:
    // TODO: Tag resolution.
    أرجع "tag:yaml.org,2002:str";
  حالة NK_Mapping:
    أرجع "tag:yaml.org,2002:map";
  حالة NK_Sequence:
    أرجع "tag:yaml.org,2002:seq";
  }

  أرجع "";
}

Token &گره::peek_بعدی() {
  أرجع Doc->peek_بعدی();
}

Token گره::get_بعدی() {
  أرجع Doc->get_بعدی();
}

گره *گره::parse_بلوک_گره() {
  أرجع Doc->parse_بلوک_گره();
}

BumpPtrAllocator &گره::getAllocator() {
  أرجع Doc->گره_Allocator;
}

فراغ گره::مجموعه_Error(ثابت Twine &پیام_4, Token &Tok) ثابت {
  Doc->مجموعه_Error(پیام_4, Tok);
}

منطقي گره::failed() ثابت {
  أرجع Doc->failed();
}

StringRef Scalar_گره::get_مقدار(Small_وکتور_Impl<حرف> &Storage) ثابت {
  إذا (مقدار[0] == '"')
    أرجع get_Double_Quoted_مقدار(مقدار, Storage);
  إذا (مقدار[0] == '\'')
    أرجع get_Single_Quoted_مقدار(مقدار, Storage);
  أرجع get_Plain_مقدار(مقدار, Storage);
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
parse_Scalar_مقدار(StringRef Unquoted_مقدار, Small_وکتور_Impl<حرف> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, Small_وکتور_Impl<حرف> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_مقدار.پیدا_کن_اولی_از(LookupChars);
  إذا (I == StringRef::npos)
    أرجع Unquoted_مقدار;

  Storage.clear();
  Storage.reserve(Unquoted_مقدار.اندازه_2());
  حرف آخرین_New_سطر_Added_As = '\0';
  لكل (; I != StringRef::npos; I = Unquoted_مقدار.پیدا_کن_اولی_از(LookupChars)) {
    إذا (Unquoted_مقدار[I] != '\r' && Unquoted_مقدار[I] != '\n') {
      llvm::append_range(Storage, Unquoted_مقدار.take_front(I));
      Unquoted_مقدار = UnescapeCallback(Unquoted_مقدار.drop_front(I), Storage);
      آخرین_New_سطر_Added_As = '\0';
      استمر;
    }
    إذا (size_t آخرین_Non_SWhite = Unquoted_مقدار.پیدا_کن_آخرین_not_از(" \t", I);
        آخرین_Non_SWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_مقدار.take_front(آخرین_Non_SWhite + 1));
      Storage.افزودن_پشت(' ');
      آخرین_New_سطر_Added_As = ' ';
    } وإلا {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      بدل (آخرین_New_سطر_Added_As) {
      حالة ' ':
        أكد(!Storage.خالی() && Storage.پشت() == ' ');
        Storage.پشت() = '\n';
        آخرین_New_سطر_Added_As = '\n';
        اكسر;
      حالة '\n':
        أكد(!Storage.خالی() && Storage.پشت() == '\n');
        Storage.افزودن_پشت('\n');
        اكسر;
      افتراضي:
        Storage.افزودن_پشت(' ');
        آخرین_New_سطر_Added_As = ' ';
        اكسر;
      }
    }
    // Handle Windows-style EOL
    إذا (Unquoted_مقدار.substr(I, 2) == "\r\n")
      I++;
    Unquoted_مقدار = Unquoted_مقدار.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_مقدار);
  أرجع StringRef(Storage.begin(), Storage.اندازه_2());
}

StringRef
Scalar_گره::get_Double_Quoted_مقدار(StringRef Raw_مقدار,
                                 Small_وکتور_Impl<حرف> &Storage) ثابت {
  أكد(Raw_مقدار.اندازه_2() >= 2 && Raw_مقدار.front() == '"' &&
         Raw_مقدار.پشت() == '"');
  StringRef Unquoted_مقدار = Raw_مقدار.substr(1, Raw_مقدار.اندازه_2() - 2);

  خودکار UnescapeFunc = [هذا](StringRef Unquoted_مقدار,
                             Small_وکتور_Impl<حرف> &Storage) {
    أكد(Unquoted_مقدار.take_front(1) == "\\");
    إذا (Unquoted_مقدار.اندازه_2() == 1) {
      Token T;
      T.Range = Unquoted_مقدار;
      مجموعه_Error("Unrecognized escape code", T);
      Storage.clear();
      أرجع StringRef();
    }
    Unquoted_مقدار = Unquoted_مقدار.drop_front(1);
    بدل (Unquoted_مقدار[0]) {
    افتراضي: {
      Token T;
      T.Range = Unquoted_مقدار.take_front(1);
      مجموعه_Error("Unrecognized escape code", T);
      Storage.clear();
      أرجع StringRef();
    }
    حالة '\r':
      // Shrink the Windows-style EOL.
      إذا (Unquoted_مقدار.اندازه_2() >= 2 && Unquoted_مقدار[1] == '\n')
        Unquoted_مقدار = Unquoted_مقدار.drop_front(1);
      [[fallthrough]];
    حالة '\n':
      أرجع Unquoted_مقدار.drop_front(1).ltrim(" \t");
    حالة '0':
      Storage.افزودن_پشت(0x00);
      اكسر;
    حالة 'a':
      Storage.افزودن_پشت(0x07);
      اكسر;
    حالة 'b':
      Storage.افزودن_پشت(0x08);
      اكسر;
    حالة 't':
    حالة 0x09:
      Storage.افزودن_پشت(0x09);
      اكسر;
    حالة 'n':
      Storage.افزودن_پشت(0x0A);
      اكسر;
    حالة 'v':
      Storage.افزودن_پشت(0x0B);
      اكسر;
    حالة 'f':
      Storage.افزودن_پشت(0x0C);
      اكسر;
    حالة 'r':
      Storage.افزودن_پشت(0x0D);
      اكسر;
    حالة 'e':
      Storage.افزودن_پشت(0x1B);
      اكسر;
    حالة ' ':
      Storage.افزودن_پشت(0x20);
      اكسر;
    حالة '"':
      Storage.افزودن_پشت(0x22);
      اكسر;
    حالة '/':
      Storage.افزودن_پشت(0x2F);
      اكسر;
    حالة '\\':
      Storage.افزودن_پشت(0x5C);
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
      إذا (Unquoted_مقدار.اندازه_2() < 3)
        // TODO: Report error.
        اكسر;
      بیعلامت صحيح Unicode_Scalar_مقدار;
      إذا (Unquoted_مقدار.substr(1, 2).getAsInteger(16, Unicode_Scalar_مقدار))
        // TODO: Report error.
        Unicode_Scalar_مقدار = 0xFFFD;
      encodeUTF8(Unicode_Scalar_مقدار, Storage);
      أرجع Unquoted_مقدار.drop_front(3);
    }
    حالة 'u': {
      إذا (Unquoted_مقدار.اندازه_2() < 5)
        // TODO: Report error.
        اكسر;
      بیعلامت صحيح Unicode_Scalar_مقدار;
      إذا (Unquoted_مقدار.substr(1, 4).getAsInteger(16, Unicode_Scalar_مقدار))
        // TODO: Report error.
        Unicode_Scalar_مقدار = 0xFFFD;
      encodeUTF8(Unicode_Scalar_مقدار, Storage);
      أرجع Unquoted_مقدار.drop_front(5);
    }
    حالة 'U': {
      إذا (Unquoted_مقدار.اندازه_2() < 9)
        // TODO: Report error.
        اكسر;
      بیعلامت صحيح Unicode_Scalar_مقدار;
      إذا (Unquoted_مقدار.substr(1, 8).getAsInteger(16, Unicode_Scalar_مقدار))
        // TODO: Report error.
        Unicode_Scalar_مقدار = 0xFFFD;
      encodeUTF8(Unicode_Scalar_مقدار, Storage);
      أرجع Unquoted_مقدار.drop_front(9);
    }
    }
    أرجع Unquoted_مقدار.drop_front(1);
  };

  أرجع parse_Scalar_مقدار(Unquoted_مقدار, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_گره::get_Single_Quoted_مقدار(StringRef Raw_مقدار,
                                           Small_وکتور_Impl<حرف> &Storage) {
  أكد(Raw_مقدار.اندازه_2() >= 2 && Raw_مقدار.front() == '\'' &&
         Raw_مقدار.پشت() == '\'');
  StringRef Unquoted_مقدار = Raw_مقدار.substr(1, Raw_مقدار.اندازه_2() - 2);

  خودکار UnescapeFunc = [](StringRef Unquoted_مقدار,
                         Small_وکتور_Impl<حرف> &Storage) {
    أكد(Unquoted_مقدار.take_front(2) == "''");
    Storage.افزودن_پشت('\'');
    أرجع Unquoted_مقدار.drop_front(2);
  };

  أرجع parse_Scalar_مقدار(Unquoted_مقدار, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_گره::get_Plain_مقدار(StringRef Raw_مقدار,
                                    Small_وکتور_Impl<حرف> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_مقدار = Raw_مقدار.rtrim("\r\n \t");
  أرجع parse_Scalar_مقدار(Raw_مقدار, Storage, "\r\n", اشاره_تهی);
}

گره *کلید_مقدار_گره::get_کلید() {
  إذا (کلید)
    أرجع کلید;
  // Handle implicit null keys.
  {
    Token &t = peek_بعدی();
    إذا (   t.نوع_3 == Token::TK_بلوک_انتها
        || t.نوع_3 == Token::TK_مقدار
        || t.نوع_3 == Token::TK_Error) {
      أرجع کلید = جديد (getAllocator()) Null_گره(Doc);
    }
    إذا (t.نوع_3 == Token::TK_کلید)
      get_بعدی(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peek_بعدی();
  إذا (t.نوع_3 == Token::TK_بلوک_انتها || t.نوع_3 == Token::TK_مقدار) {
    أرجع کلید = جديد (getAllocator()) Null_گره(Doc);
  }

  // We've got a normal key.
  أرجع کلید = parse_بلوک_گره();
}

گره *کلید_مقدار_گره::get_مقدار() {
  إذا (مقدار)
    أرجع مقدار;

  إذا (گره* کلید = get_کلید())
    کلید->skip();
  وإلا {
    مجموعه_Error("Null key in Key Value.", peek_بعدی());
    أرجع مقدار = جديد (getAllocator()) Null_گره(Doc);
  }

  إذا (failed())
    أرجع مقدار = جديد (getAllocator()) Null_گره(Doc);

  // Handle implicit null values.
  {
    Token &t = peek_بعدی();
    إذا (   t.نوع_3 == Token::TK_بلوک_انتها
        || t.نوع_3 == Token::TK_Flow_Mapping_انتها
        || t.نوع_3 == Token::TK_کلید
        || t.نوع_3 == Token::TK_Flow_مدخل
        || t.نوع_3 == Token::TK_Error) {
      أرجع مقدار = جديد (getAllocator()) Null_گره(Doc);
    }

    إذا (t.نوع_3 != Token::TK_مقدار) {
      مجموعه_Error("Unexpected token in Key Value.", t);
      أرجع مقدار = جديد (getAllocator()) Null_گره(Doc);
    }
    get_بعدی(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peek_بعدی();
  إذا (t.نوع_3 == Token::TK_بلوک_انتها || t.نوع_3 == Token::TK_کلید) {
    أرجع مقدار = جديد (getAllocator()) Null_گره(Doc);
  }

  // We got a normal value.
  أرجع مقدار = parse_بلوک_گره();
}

فراغ Mapping_گره::increment() {
  إذا (failed()) {
    Is_At_انتها = درست;
    Current_مدخل = اشاره_تهی;
    أرجع;
  }
  إذا (Current_مدخل) {
    Current_مدخل->skip();
    إذا (Type == MT_Inline) {
      Is_At_انتها = درست;
      Current_مدخل = اشاره_تهی;
      أرجع;
    }
  }
  Token T = peek_بعدی();
  إذا (T.نوع_3 == Token::TK_کلید || T.نوع_3 == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    Current_مدخل = جديد (getAllocator()) کلید_مقدار_گره(Doc);
  } وإلا إذا (Type == MT_بلوک) {
    بدل (T.نوع_3) {
    حالة Token::TK_بلوک_انتها:
      get_بعدی();
      Is_At_انتها = درست;
      Current_مدخل = اشاره_تهی;
      اكسر;
    افتراضي:
      مجموعه_Error("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    حالة Token::TK_Error:
      Is_At_انتها = درست;
      Current_مدخل = اشاره_تهی;
    }
  } وإلا {
    بدل (T.نوع_3) {
    حالة Token::TK_Flow_مدخل:
      // Eat the flow entry and recurse.
      get_بعدی();
      أرجع increment();
    حالة Token::TK_Flow_Mapping_انتها:
      get_بعدی();
      [[fallthrough]];
    حالة Token::TK_Error:
      // Set this to end iterator.
      Is_At_انتها = درست;
      Current_مدخل = اشاره_تهی;
      اكسر;
    افتراضي:
      مجموعه_Error( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      Is_At_انتها = درست;
      Current_مدخل = اشاره_تهی;
    }
  }
}

فراغ Sequence_گره::increment() {
  إذا (failed()) {
    Is_At_انتها = درست;
    Current_مدخل = اشاره_تهی;
    أرجع;
  }
  إذا (Current_مدخل)
    Current_مدخل->skip();
  Token T = peek_بعدی();
  إذا (SeqType == ST_بلوک) {
    بدل (T.نوع_3) {
    حالة Token::TK_بلوک_مدخل:
      get_بعدی();
      Current_مدخل = parse_بلوک_گره();
      إذا (!Current_مدخل) { // An error occurred.
        Is_At_انتها = درست;
        Current_مدخل = اشاره_تهی;
      }
      اكسر;
    حالة Token::TK_بلوک_انتها:
      get_بعدی();
      Is_At_انتها = درست;
      Current_مدخل = اشاره_تهی;
      اكسر;
    افتراضي:
      مجموعه_Error( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    حالة Token::TK_Error:
      Is_At_انتها = درست;
      Current_مدخل = اشاره_تهی;
    }
  } وإلا إذا (SeqType == ST_Indentless) {
    بدل (T.نوع_3) {
    حالة Token::TK_بلوک_مدخل:
      get_بعدی();
      Current_مدخل = parse_بلوک_گره();
      إذا (!Current_مدخل) { // An error occurred.
        Is_At_انتها = درست;
        Current_مدخل = اشاره_تهی;
      }
      اكسر;
    افتراضي:
    حالة Token::TK_Error:
      Is_At_انتها = درست;
      Current_مدخل = اشاره_تهی;
    }
  } وإلا إذا (SeqType == ST_Flow) {
    بدل (T.نوع_3) {
    حالة Token::TK_Flow_مدخل:
      // Eat the flow entry and recurse.
      get_بعدی();
      Was_قبلی_Token_Flow_مدخل = درست;
      أرجع increment();
    حالة Token::TK_Flow_Sequence_انتها:
      get_بعدی();
      [[fallthrough]];
    حالة Token::TK_Error:
      // Set this to end iterator.
      Is_At_انتها = درست;
      Current_مدخل = اشاره_تهی;
      اكسر;
    حالة Token::TK_جریان_انتها:
    حالة Token::TK_Document_انتها:
    حالة Token::TK_Document_شروع:
      مجموعه_Error("Could not find closing ]!", T);
      // Set this to end iterator.
      Is_At_انتها = درست;
      Current_مدخل = اشاره_تهی;
      اكسر;
    افتراضي:
      إذا (!Was_قبلی_Token_Flow_مدخل) {
        مجموعه_Error("Expected , between entries!", T);
        Is_At_انتها = درست;
        Current_مدخل = اشاره_تهی;
        اكسر;
      }
      // Otherwise it must be a flow entry.
      Current_مدخل = parse_بلوک_گره();
      إذا (!Current_مدخل) {
        Is_At_انتها = درست;
      }
      Was_قبلی_Token_Flow_مدخل = خطأ;
      اكسر;
    }
  }
}

Document::Document(جریان &S) : جریان_2(S), ریشه_2(اشاره_تهی) {
  // Tag maps starts with two default mappings.
  برچسب_Map["!"] = "!";
  برچسب_Map["!!"] = "tag:yaml.org,2002:";

  إذا (parseDirectives())
    expectToken(Token::TK_Document_شروع);
  Token &T = peek_بعدی();
  إذا (T.نوع_3 == Token::TK_Document_شروع)
    get_بعدی();
}

منطقي Document::skip()  {
  إذا (جریان_2.scanner->failed())
    أرجع خطأ;
  إذا (!ریشه_2 && !get_ریشه())
    أرجع خطأ;
  ریشه_2->skip();
  Token &T = peek_بعدی();
  إذا (T.نوع_3 == Token::TK_جریان_انتها)
    أرجع خطأ;
  إذا (T.نوع_3 == Token::TK_Document_انتها) {
    get_بعدی();
    أرجع skip();
  }
  أرجع درست;
}

Token &Document::peek_بعدی() {
  أرجع جریان_2.scanner->peek_بعدی();
}

Token Document::get_بعدی() {
  أرجع جریان_2.scanner->get_بعدی();
}

فراغ Document::مجموعه_Error(ثابت Twine &پیام_3, Token &Location) ثابت {
  جریان_2.scanner->مجموعه_Error(پیام_3, Location.Range.begin());
}

منطقي Document::failed() ثابت {
  أرجع جریان_2.scanner->failed();
}

گره *Document::parse_بلوک_گره() {
  Token T = peek_بعدی();
  // Handle properties.
  Token AnchorInfo;
  Token برچسب_Info;
parse_property:
  بدل (T.نوع_3) {
  حالة Token::TK_Alias:
    get_بعدی();
    أرجع جديد (گره_Allocator) Alias_گره(جریان_2.CurrentDoc, T.Range.substr(1));
  حالة Token::TK_Anchor:
    إذا (AnchorInfo.نوع_3 == Token::TK_Anchor) {
      مجموعه_Error("Already encountered an anchor for this node!", T);
      أرجع اشاره_تهی;
    }
    AnchorInfo = get_بعدی(); // Consume TK_Anchor.
    T = peek_بعدی();
    اذهب parse_property;
  حالة Token::TK_برچسب:
    إذا (برچسب_Info.نوع_3 == Token::TK_برچسب) {
      مجموعه_Error("Already encountered a tag for this node!", T);
      أرجع اشاره_تهی;
    }
    برچسب_Info = get_بعدی(); // Consume TK_Tag.
    T = peek_بعدی();
    اذهب parse_property;
  افتراضي:
    اكسر;
  }

  بدل (T.نوع_3) {
  حالة Token::TK_بلوک_مدخل:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    أرجع جديد (گره_Allocator) Sequence_گره( جریان_2.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , برچسب_Info.Range
                                           , Sequence_گره::ST_Indentless);
  حالة Token::TK_بلوک_Sequence_شروع:
    get_بعدی();
    أرجع جديد (گره_Allocator)
      Sequence_گره( جریان_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , برچسب_Info.Range
                  , Sequence_گره::ST_بلوک);
  حالة Token::TK_بلوک_Mapping_شروع:
    get_بعدی();
    أرجع جديد (گره_Allocator)
      Mapping_گره( جریان_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , برچسب_Info.Range
                 , Mapping_گره::MT_بلوک);
  حالة Token::TK_Flow_Sequence_شروع:
    get_بعدی();
    أرجع جديد (گره_Allocator)
      Sequence_گره( جریان_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , برچسب_Info.Range
                  , Sequence_گره::ST_Flow);
  حالة Token::TK_Flow_Mapping_شروع:
    get_بعدی();
    أرجع جديد (گره_Allocator)
      Mapping_گره( جریان_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , برچسب_Info.Range
                 , Mapping_گره::MT_Flow);
  حالة Token::TK_Scalar:
    get_بعدی();
    أرجع جديد (گره_Allocator)
      Scalar_گره( جریان_2.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , برچسب_Info.Range
                , T.Range);
  حالة Token::TK_بلوک_Scalar: {
    get_بعدی();
    StringRef NullTerminatedStr(T.مقدار.c_str(), T.مقدار.درازی_2() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(گره_Allocator).drop_پشت();
    أرجع جديد (گره_Allocator)
        بلوک_Scalar_گره(جریان_2.CurrentDoc, AnchorInfo.Range.substr(1),
                        برچسب_Info.Range, StrCopy, T.Range);
  }
  حالة Token::TK_کلید:
    // Don't eat the TK_Key, KeyValueNode expects it.
    أرجع جديد (گره_Allocator)
      Mapping_گره( جریان_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , برچسب_Info.Range
                 , Mapping_گره::MT_Inline);
  حالة Token::TK_Document_شروع:
  حالة Token::TK_Document_انتها:
  حالة Token::TK_جریان_انتها:
  افتراضي:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    أرجع جديد (گره_Allocator) Null_گره(جریان_2.CurrentDoc);
  حالة Token::TK_Flow_Mapping_انتها:
  حالة Token::TK_Flow_Sequence_انتها:
  حالة Token::TK_Flow_مدخل: {
    إذا (ریشه_2 && (isa<Mapping_گره>(ریشه_2) || isa<Sequence_گره>(ریشه_2)))
      أرجع جديد (گره_Allocator) Null_گره(جریان_2.CurrentDoc);

    مجموعه_Error("Unexpected token", T);
    أرجع اشاره_تهی;
  }
  حالة Token::TK_Error:
    أرجع اشاره_تهی;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  أرجع اشاره_تهی;
}

منطقي Document::parseDirectives() {
  منطقي isDirective = خطأ;
  طالما (درست) {
    Token T = peek_بعدی();
    إذا (T.نوع_3 == Token::TK_برچسب_Directive) {
      parseTAGDirective();
      isDirective = درست;
    } وإلا إذا (T.نوع_3 == Token::TK_نسخه_Directive) {
      parseYAMLDirective();
      isDirective = درست;
    } وإلا {
      اكسر;
    }
  }
  أرجع isDirective;
}

فراغ Document::parseYAMLDirective() {
  get_بعدی(); // Eat %YAML <version>
}

فراغ Document::parseTAGDirective() {
  Token برچسب = get_بعدی(); // %TAG <handle> <prefix>
  StringRef T = برچسب.Range;
  // Strip %TAG
  T = T.substr(T.پیدا_کن_اولی_از(" \t")).ltrim(" \t");
  std::size_t Handle_انتها = T.پیدا_کن_اولی_از(" \t");
  StringRef برچسب_Handle = T.substr(0, Handle_انتها);
  StringRef برچسب_پیشوند = T.substr(Handle_انتها).ltrim(" \t");
  برچسب_Map[برچسب_Handle] = برچسب_پیشوند;
}

منطقي Document::expectToken(صحيح TK) {
  Token T = get_بعدی();
  إذا (T.نوع_3 != TK) {
    مجموعه_Error("Unexpected token", T);
    أرجع خطأ;
  }
  أرجع درست;
}
