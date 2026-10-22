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

استعمال نطاق llvm;
استعمال نطاق yaml;

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
استعمال EncodingInfo = std::pair<UnicodeEncodingForm, بنانشان>;

/// getUnicodeEncoding - Reads up to the first 4 bytes to determine the Unicode
///                      encoding form of \a Input.
///
/// @param Input A string of length 0 or more.
/// @returns An EncodingInfo indicating the Unicode encoding form of the input
///          and how long the byte order mark is if one exists.
ساكن EncodingInfo getUnicodeEncoding(StringRef داخلا) {
  إذا (داخلا.خالي_جايون())
    أرجع {UEF_Unknown, 0};

  بدل (uint8_t(داخلا[0])) {
  حالة 0x00:
    إذا (داخلا.سائيز_2() >= 4) {
      إذا (  داخلا[1] == 0
         && uint8_t(داخلا[2]) == 0xFE
         && uint8_t(داخلا[3]) == 0xFF)
        أرجع {UEF_UTF32_BE, 4};
      إذا (داخلا[1] == 0 && داخلا[2] == 0 && داخلا[3] != 0)
        أرجع {UEF_UTF32_BE, 0};
    }

    إذا (داخلا.سائيز_2() >= 2 && داخلا[1] != 0)
      أرجع {UEF_UTF16_BE, 0};
    أرجع {UEF_Unknown, 0};
  حالة 0xFF:
    إذا (  داخلا.سائيز_2() >= 4
       && uint8_t(داخلا[1]) == 0xFE
       && داخلا[2] == 0
       && داخلا[3] == 0)
      أرجع {UEF_UTF32_LE, 4};

    إذا (داخلا.سائيز_2() >= 2 && uint8_t(داخلا[1]) == 0xFE)
      أرجع {UEF_UTF16_LE, 2};
    أرجع {UEF_Unknown, 0};
  حالة 0xFE:
    إذا (داخلا.سائيز_2() >= 2 && uint8_t(داخلا[1]) == 0xFF)
      أرجع {UEF_UTF16_BE, 2};
    أرجع {UEF_Unknown, 0};
  حالة 0xEF:
    إذا (  داخلا.سائيز_2() >= 3
       && uint8_t(داخلا[1]) == 0xBB
       && uint8_t(داخلا[2]) == 0xBF)
      أرجع {UEF_UTF8, 3};
    أرجع {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  إذا (داخلا.سائيز_2() >= 4 && داخلا[1] == 0 && داخلا[2] == 0 && داخلا[3] == 0)
    أرجع {UEF_UTF32_LE, 0};

  إذا (داخلا.سائيز_2() >= 2 && داخلا[1] == 0)
    أرجع {UEF_UTF16_LE, 0};

  أرجع {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
فراغ ڳنڍ::anchor() {}
فراغ Null_ڳنڍ::anchor() {}
فراغ Scalar_ڳنڍ::anchor() {}
فراغ بلاڪ_Scalar_ڳنڍ::anchor() {}
فراغ ڪنجي_قدر_ڳنڍ::anchor() {}
فراغ Mapping_ڳنڍ::anchor() {}
فراغ Sequence_ڳنڍ::anchor() {}
فراغ Alias_ڳنڍ::anchor() {}

نطاق llvm {
نطاق yaml {

/// Token - A single YAML token.
بنية Token {
  تعداد Token_قسم {
    TK_Error, // Uninitialized token.
    TK_وهڪرو_شروعات,
    TK_وهڪرو_پڇاڙي,
    TK_نسخو_Directive,
    TK_ٽيگ_Directive,
    TK_Document_شروعات,
    TK_Document_پڇاڙي,
    TK_بلاڪ_داخلا,
    TK_بلاڪ_پڇاڙي,
    TK_بلاڪ_Sequence_شروعات,
    TK_بلاڪ_Mapping_شروعات,
    TK_Flow_داخلا,
    TK_Flow_Sequence_شروعات,
    TK_Flow_Sequence_پڇاڙي,
    TK_Flow_Mapping_شروعات,
    TK_Flow_Mapping_پڇاڙي,
    TK_ڪنجي,
    TK_قدر,
    TK_Scalar,
    TK_بلاڪ_Scalar,
    TK_Alias,
    TK_Anchor,
    TK_ٽيگ
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

استعمال Token_قطار_T = BumpPtrList<Token>;

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
بنية Simple_ڪنجي {
  Token_قطار_T::iterator Tok;
  بنانشان ڪالم = 0;
  بنانشان سٽ = 0;
  بنانشان FlowLevel = 0;
  منطقي IsRequired = خطأ;

  منطقي عامل ==(ثابت Simple_ڪنجي &Other) {
    أرجع Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
استعمال UTF8Decoded = std::pair<uint32_t, بنانشان>;

ساكن UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator پڇاڙي_2 = Range.پڇاڙي();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  إذا (Position < پڇاڙي_2 && (*Position & 0x80) == 0) {
    أرجع {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  إذا (Position + 1 < پڇاڙي_2 && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    إذا (codepoint >= 0x80)
      أرجع {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  إذا (Position + 2 < پڇاڙي_2 && ((*Position & 0xF0) == 0xE0) &&
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
  إذا (Position + 3 < پڇاڙي_2 && ((*Position & 0xF8) == 0xF0) &&
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
  Scanner(StringRef داخلا, ذريعو_Mgr &SM, منطقي ShowColors = سچ,
          std::error_ڪوڊ *EC = خالي_اشارو);
  Scanner(يادگيري_بفر_Ref بفر, ذريعو_Mgr &SM_, منطقي ShowColors = سچ,
          std::error_ڪوڊ *EC = خالي_اشارو);

  /// Parse the next token and return it without popping it.
  Token &peek_ايندڙ();

  /// Parse the next token and pop it from the queue.
  Token get_ايندڙ();

  فراغ printError(SMLoc Loc, ذريعو_Mgr::Diag_قسم قسم_2, ثابت Twine &پيغام_2,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.Print_پيغام(Loc, قسم_2, پيغام_2, Ranges, /* FixIts= */ {}, ShowColors);
  }

  فراغ سيٽ_Error(ثابت Twine &پيغام_2, StringRef::iterator Position) {
    إذا (Position >= پڇاڙي_2)
      Position = پڇاڙي_2 - 1;

    // propagate the error if possible
    إذا (EC)
      *EC = make_error_ڪوڊ(std::errc::invalid_دليل);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    إذا (!Failed)
      printError(SMLoc::get_From_اشارو(Position), ذريعو_Mgr::DK_Error, پيغام_2);
    Failed = سچ;
  }

  /// Returns true if an error occurred while parsing.
  منطقي failed() {
    أرجع Failed;
  }

خاص:
  فراغ init(يادگيري_بفر_Ref بفر);

  StringRef current_داخلا() {
    أرجع StringRef(Current, پڇاڙي_2 - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    أرجع ::decodeUTF8(StringRef(Position, پڇاڙي_2 - Position));
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

  استعمال SkipWhileFunc = StringRef::iterator (Scanner::*)(StringRef::iterator);

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
  فراغ جاچ_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  منطقي consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  فراغ skip(uint32_t مفاصلو_2);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  منطقي isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  منطقي is_Plain_محفوظ_Non_Blank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  منطقي is_سٽ_خالي_جايون(StringRef سٽ);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  منطقي consume_سٽ_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  فراغ save_Simple_ڪنجي_Candidate( Token_قطار_T::iterator Tok
                             , بنانشان At_ڪالم
                             , منطقي IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  فراغ remove_Stale_Simple_ڪنجي_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  فراغ remove_Simple_ڪنجي_Candidates_On_Flow_Level(بنانشان Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  منطقي unrollIndent(صحيح منزل_ڪالم);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  منطقي rollIndent( صحيح منزل_ڪالم
                 , Token::Token_قسم قسم_2
                 , Token_قطار_T::iterator وجهو_نقطو);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  فراغ skipComment();

  /// Skip whitespace and comments until the start of the next token.
  فراغ جاچ_منزل_ايندڙ_Token();

  /// Must be the first token generated.
  منطقي جاچ_وهڪرو_شروعات();

  /// Generate tokens needed to close out the stream.
  منطقي جاچ_وهڪرو_پڇاڙي();

  /// Scan a %BLAH directive.
  منطقي جاچ_Directive();

  /// Scan a ... or ---.
  منطقي جاچ_Document_Indicator(منطقي Is_شروعات);

  /// Scan a [ or { and generate the proper flow collection start token.
  منطقي جاچ_Flow_Collection_شروعات(منطقي IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  منطقي جاچ_Flow_Collection_پڇاڙي(منطقي IsSequence);

  /// Scan the , that separates entries in a flow collection.
  منطقي جاچ_Flow_داخلا();

  /// Scan the - that starts block sequence entries.
  منطقي جاچ_بلاڪ_داخلا();

  /// Scan an explicit ? indicating a key.
  منطقي جاچ_ڪنجي();

  /// Scan an explicit : indicating a value.
  منطقي جاچ_قدر();

  /// Scan a quoted scalar.
  منطقي جاچ_Flow_Scalar(منطقي IsDoubleQuoted);

  /// Scan an unquoted scalar.
  منطقي جاچ_Plain_Scalar();

  /// Scan an Alias or Anchor starting with * or &.
  منطقي جاچ_Alias_Or_Anchor(منطقي IsAlias);

  /// Scan a block scalar starting with | or >.
  منطقي جاچ_بلاڪ_Scalar(منطقي IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  منطقي جاچ_بلاڪ_Scalar_Indicators(حرف &StyleIndicator, حرف &ChompingIndicator,
                                 بنانشان &IndentIndicator, منطقي &Is_پورو);

  /// Scan a style indicator in a block scalar header.
  حرف جاچ_بلاڪ_Style_Indicator();

  /// Scan a chomping indicator in a block scalar header.
  حرف جاچ_بلاڪ_Chomping_Indicator();

  /// Scan an indentation indicator in a block scalar header.
  بنانشان جاچ_بلاڪ_Indentation_Indicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  منطقي جاچ_بلاڪ_Scalar_سرو(حرف &ChompingIndicator, بنانشان &IndentIndicator,
                             منطقي &Is_پورو);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  منطقي ڳوليو_بلاڪ_Scalar_Indent(بنانشان &بلاڪ_Indent, بنانشان بلاڪ_Exit_Indent,
                             بنانشان &سٽ_Breaks, منطقي &Is_پورو);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  منطقي جاچ_بلاڪ_Scalar_Indent(بنانشان بلاڪ_Indent, بنانشان بلاڪ_Exit_Indent,
                             منطقي &Is_پورو);

  /// Scan a tag of the form !stuff.
  منطقي جاچ_ٽيگ();

  /// Dispatch to the next scanning function based on \a *Cur.
  منطقي fetch_وڌيڪ_ٽوڪن();

  /// The SourceMgr used for diagnostics and buffer management.
  ذريعو_Mgr &SM;

  /// The original input.
  يادگيري_بفر_Ref داخلا_بفر;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator پڇاڙي_2;

  /// Current YAML indentation level in spaces.
  صحيح Indent;

  /// Current column number in Unicode code points.
  بنانشان ڪالم;

  /// Current line number.
  بنانشان سٽ;

  /// How deep we are in flow style containers. 0 Means at block level.
  بنانشان FlowLevel;

  /// Are we at the start of the stream?
  منطقي Is_شروعات_جو_وهڪرو;

  /// Can the next token be the start of a simple key?
  منطقي Is_Simple_ڪنجي_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  منطقي Is_Adjacent_قدر_Allowed_اندريون_Flow;

  /// True if an error has occurred.
  منطقي Failed;

  /// Should colors be used when printing out the diagnostic messages?
  منطقي ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  Token_قطار_T Token_قطار;

  /// Indentation levels.
  Small_ويڪٽر<صحيح, 4> Indents;

  /// Potential simple keys.
  Small_ويڪٽر<Simple_ڪنجي, 4> Simple_ڪنجيون;

  std::error_ڪوڊ *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
ساكن فراغ encodeUTF8( uint32_t Unicode_Scalar_قدر
                      , Small_ويڪٽر_Impl<حرف> &نتيجو) {
  إذا (Unicode_Scalar_قدر <= 0x7F) {
    نتيجو.ڌڪو_پٺ(Unicode_Scalar_قدر & 0x7F);
  } وإلا إذا (Unicode_Scalar_قدر <= 0x7FF) {
    uint8_t پهريون_Byte = 0xC0 | ((Unicode_Scalar_قدر & 0x7C0) >> 6);
    uint8_t ٻيو_Byte = 0x80 | (Unicode_Scalar_قدر & 0x3F);
    نتيجو.ڌڪو_پٺ(پهريون_Byte);
    نتيجو.ڌڪو_پٺ(ٻيو_Byte);
  } وإلا إذا (Unicode_Scalar_قدر <= 0xFFFF) {
    uint8_t پهريون_Byte = 0xE0 | ((Unicode_Scalar_قدر & 0xF000) >> 12);
    uint8_t ٻيو_Byte = 0x80 | ((Unicode_Scalar_قدر & 0xFC0) >> 6);
    uint8_t ٽيون_Byte = 0x80 | (Unicode_Scalar_قدر & 0x3F);
    نتيجو.ڌڪو_پٺ(پهريون_Byte);
    نتيجو.ڌڪو_پٺ(ٻيو_Byte);
    نتيجو.ڌڪو_پٺ(ٽيون_Byte);
  } وإلا إذا (Unicode_Scalar_قدر <= 0x10FFFF) {
    uint8_t پهريون_Byte = 0xF0 | ((Unicode_Scalar_قدر & 0x1F0000) >> 18);
    uint8_t ٻيو_Byte = 0x80 | ((Unicode_Scalar_قدر & 0x3F000) >> 12);
    uint8_t ٽيون_Byte = 0x80 | ((Unicode_Scalar_قدر & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_قدر & 0x3F);
    نتيجو.ڌڪو_پٺ(پهريون_Byte);
    نتيجو.ڌڪو_پٺ(ٻيو_Byte);
    نتيجو.ڌڪو_پٺ(ٽيون_Byte);
    نتيجو.ڌڪو_پٺ(FourthByte);
  }
}

منطقي yaml::dump_ٽوڪن(StringRef داخلا, raw_ostream &OS) {
  ذريعو_Mgr SM;
  Scanner scanner(داخلا, SM);
  طالما (سچ) {
    Token T = scanner.get_ايندڙ();
    بدل (T.قسم_2) {
    حالة Token::TK_وهڪرو_شروعات:
      OS << "Stream-Start: ";
      اكسر;
    حالة Token::TK_وهڪرو_پڇاڙي:
      OS << "Stream-End: ";
      اكسر;
    حالة Token::TK_نسخو_Directive:
      OS << "Version-Directive: ";
      اكسر;
    حالة Token::TK_ٽيگ_Directive:
      OS << "Tag-Directive: ";
      اكسر;
    حالة Token::TK_Document_شروعات:
      OS << "Document-Start: ";
      اكسر;
    حالة Token::TK_Document_پڇاڙي:
      OS << "Document-End: ";
      اكسر;
    حالة Token::TK_بلاڪ_داخلا:
      OS << "Block-Entry: ";
      اكسر;
    حالة Token::TK_بلاڪ_پڇاڙي:
      OS << "Block-End: ";
      اكسر;
    حالة Token::TK_بلاڪ_Sequence_شروعات:
      OS << "Block-Sequence-Start: ";
      اكسر;
    حالة Token::TK_بلاڪ_Mapping_شروعات:
      OS << "Block-Mapping-Start: ";
      اكسر;
    حالة Token::TK_Flow_داخلا:
      OS << "Flow-Entry: ";
      اكسر;
    حالة Token::TK_Flow_Sequence_شروعات:
      OS << "Flow-Sequence-Start: ";
      اكسر;
    حالة Token::TK_Flow_Sequence_پڇاڙي:
      OS << "Flow-Sequence-End: ";
      اكسر;
    حالة Token::TK_Flow_Mapping_شروعات:
      OS << "Flow-Mapping-Start: ";
      اكسر;
    حالة Token::TK_Flow_Mapping_پڇاڙي:
      OS << "Flow-Mapping-End: ";
      اكسر;
    حالة Token::TK_ڪنجي:
      OS << "Key: ";
      اكسر;
    حالة Token::TK_قدر:
      OS << "Value: ";
      اكسر;
    حالة Token::TK_Scalar:
      OS << "Scalar: ";
      اكسر;
    حالة Token::TK_بلاڪ_Scalar:
      OS << "Block Scalar: ";
      اكسر;
    حالة Token::TK_Alias:
      OS << "Alias: ";
      اكسر;
    حالة Token::TK_Anchor:
      OS << "Anchor: ";
      اكسر;
    حالة Token::TK_ٽيگ:
      OS << "Tag: ";
      اكسر;
    حالة Token::TK_Error:
      اكسر;
    }
    OS << T.Range << "\n";
    إذا (T.قسم_2 == Token::TK_وهڪرو_پڇاڙي)
      اكسر;
    وإلا إذا (T.قسم_2 == Token::TK_Error)
      أرجع خطأ;
  }
  أرجع سچ;
}

منطقي yaml::جاچ_ٽوڪن(StringRef داخلا) {
  ذريعو_Mgr SM;
  Scanner scanner(داخلا, SM);
  طالما (سچ) {
    Token T = scanner.get_ايندڙ();
    إذا (T.قسم_2 == Token::TK_وهڪرو_پڇاڙي)
      اكسر;
    وإلا إذا (T.قسم_2 == Token::TK_Error)
      أرجع خطأ;
  }
  أرجع سچ;
}

std::string yaml::escape(StringRef داخلا, منطقي EscapePrintable) {
  std::string Escaped_داخلا;
  لكل (StringRef::iterator i = داخلا.begin(), e = داخلا.پڇاڙي(); i != e; ++i) {
    إذا (*i == '\\')
      Escaped_داخلا += "\\\\";
    وإلا إذا (*i == '"')
      Escaped_داخلا += "\\\"";
    وإلا إذا (*i == 0)
      Escaped_داخلا += "\\0";
    وإلا إذا (*i == 0x07)
      Escaped_داخلا += "\\a";
    وإلا إذا (*i == 0x08)
      Escaped_داخلا += "\\b";
    وإلا إذا (*i == 0x09)
      Escaped_داخلا += "\\t";
    وإلا إذا (*i == 0x0A)
      Escaped_داخلا += "\\n";
    وإلا إذا (*i == 0x0B)
      Escaped_داخلا += "\\v";
    وإلا إذا (*i == 0x0C)
      Escaped_داخلا += "\\f";
    وإلا إذا (*i == 0x0D)
      Escaped_داخلا += "\\r";
    وإلا إذا (*i == 0x1B)
      Escaped_داخلا += "\\e";
    وإلا إذا ((بنانشان حرف)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_داخلا += "\\x" + std::string(2 - HexStr.سائيز_2(), '0') + HexStr;
    } وإلا إذا (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_قدر
        = decodeUTF8(StringRef(i, داخلا.پڇاڙي() - i));
      إذا (Unicode_Scalar_قدر.ٻيو_2 == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_داخلا, Val);
        // FIXME: Error reporting.
        أرجع Escaped_داخلا;
      }
      إذا (Unicode_Scalar_قدر.پهريون_2 == 0x85)
        Escaped_داخلا += "\\N";
      وإلا إذا (Unicode_Scalar_قدر.پهريون_2 == 0xA0)
        Escaped_داخلا += "\\_";
      وإلا إذا (Unicode_Scalar_قدر.پهريون_2 == 0x2028)
        Escaped_داخلا += "\\L";
      وإلا إذا (Unicode_Scalar_قدر.پهريون_2 == 0x2029)
        Escaped_داخلا += "\\P";
      وإلا إذا (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_قدر.پهريون_2))
        Escaped_داخلا += StringRef(i, Unicode_Scalar_قدر.ٻيو_2);
      وإلا {
        std::string HexStr = utohexstr(Unicode_Scalar_قدر.پهريون_2);
        إذا (HexStr.سائيز_2() <= 2)
          Escaped_داخلا += "\\x" + std::string(2 - HexStr.سائيز_2(), '0') + HexStr;
        وإلا إذا (HexStr.سائيز_2() <= 4)
          Escaped_داخلا += "\\u" + std::string(4 - HexStr.سائيز_2(), '0') + HexStr;
        وإلا إذا (HexStr.سائيز_2() <= 8)
          Escaped_داخلا += "\\U" + std::string(8 - HexStr.سائيز_2(), '0') + HexStr;
      }
      i += Unicode_Scalar_قدر.ٻيو_2 - 1;
    } وإلا {
      Escaped_داخلا.ڌڪو_پٺ(*i);
    }
  }
  أرجع Escaped_داخلا;
}

std::optional<منطقي> yaml::parseBool(StringRef S) {
  بدل (S.سائيز_2()) {
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

Scanner::Scanner(StringRef داخلا, ذريعو_Mgr &sm, منطقي ShowColors,
                 std::error_ڪوڊ *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(يادگيري_بفر_Ref(داخلا, "YAML"));
}

Scanner::Scanner(يادگيري_بفر_Ref بفر, ذريعو_Mgr &SM_, منطقي ShowColors,
                 std::error_ڪوڊ *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(بفر);
}

فراغ Scanner::init(يادگيري_بفر_Ref بفر) {
  داخلا_بفر = بفر;
  Current = داخلا_بفر.get_بفر_شروعات();
  پڇاڙي_2 = داخلا_بفر.get_بفر_پڇاڙي();
  Indent = -1;
  ڪالم = 0;
  سٽ = 0;
  FlowLevel = 0;
  Is_شروعات_جو_وهڪرو = سچ;
  Is_Simple_ڪنجي_Allowed = سچ;
  Is_Adjacent_قدر_Allowed_اندريون_Flow = خطأ;
  Failed = خطأ;
  std::unique_ptr<يادگيري_بفر> داخلا_بفر_Owner =
      يادگيري_بفر::get_Mem_بفر(بفر, /*RequiresNullTerminator=*/خطأ);
  SM.جوڙيو_New_ذريعو_بفر(std::move(داخلا_بفر_Owner), SMLoc());
}

Token &Scanner::peek_ايندڙ() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  منطقي Need_وڌيڪ = خطأ;
  طالما (سچ) {
    إذا (Token_قطار.خالي_جايون() || Need_وڌيڪ) {
      إذا (!fetch_وڌيڪ_ٽوڪن()) {
        Token_قطار.clear();
        Simple_ڪنجيون.clear();
        Token_قطار.ڌڪو_پٺ(Token());
        أرجع Token_قطار.front();
      }
    }
    أكد(!Token_قطار.خالي_جايون() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_ڪنجي_Candidates();
    Simple_ڪنجي SK;
    SK.Tok = Token_قطار.begin();
    إذا (!is_contained(Simple_ڪنجيون, SK))
      اكسر;
    وإلا
      Need_وڌيڪ = سچ;
  }
  أرجع Token_قطار.front();
}

Token Scanner::get_ايندڙ() {
  Token Ret = peek_ايندڙ();
  // TokenQueue can be empty if there was an error getting the next token.
  إذا (!Token_قطار.خالي_جايون())
    Token_قطار.ڪڍو_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  إذا (Token_قطار.خالي_جايون())
    Token_قطار.resetAlloc();

  أرجع Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  إذا (Position == پڇاڙي_2)
    أرجع Position;
  // Check 7 bit c-printable - b-char.
  إذا (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    أرجع Position + 1;

  // Check for valid UTF-8.
  إذا (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    إذا (   u8d.ٻيو_2 != 0
        && u8d.پهريون_2 != 0xFEFF
        && ( u8d.پهريون_2 == 0x85
          || ( u8d.پهريون_2 >= 0xA0
            && u8d.پهريون_2 <= 0xD7FF)
          || ( u8d.پهريون_2 >= 0xE000
            && u8d.پهريون_2 <= 0xFFFD)
          || ( u8d.پهريون_2 >= 0x10000
            && u8d.پهريون_2 <= 0x10FFFF)))
      أرجع Position + u8d.ٻيو_2;
  }
  أرجع Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  إذا (Position == پڇاڙي_2)
    أرجع Position;
  إذا (*Position == 0x0D) {
    إذا (Position + 1 != پڇاڙي_2 && *(Position + 1) == 0x0A)
      أرجع Position + 2;
    أرجع Position + 1;
  }

  إذا (*Position == 0x0A)
    أرجع Position + 1;
  أرجع Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  إذا (Position == پڇاڙي_2)
    أرجع Position;
  إذا (*Position == ' ')
    أرجع Position + 1;
  أرجع Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  إذا (Position == پڇاڙي_2)
    أرجع Position;
  إذا (*Position == ' ' || *Position == '\t')
    أرجع Position + 1;
  أرجع Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  إذا (Position == پڇاڙي_2)
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
  خودڪار Final = skip_while(Func, Current);
  ڪالم += Final - Current;
  Current = Final;
}

ساكن منطقي is_ns_hex_digit(ثابت حرف C) { أرجع isAlnum(C); }

ساكن منطقي is_ns_لفظ_char(ثابت حرف C) { أرجع C == '-' || isAlpha(C); }

فراغ Scanner::جاچ_ns_uri_char() {
  طالما (سچ) {
    إذا (Current == پڇاڙي_2)
      اكسر;
    إذا ((*Current == '%' && Current + 2 < پڇاڙي_2 &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_لفظ_char(*Current) ||
        StringRef(Current, 1).ڳوليو_پهريون_جو("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++ڪالم;
    } وإلا {
      اكسر;
    }
  }
}

منطقي Scanner::consume(uint32_t Expected) {
  إذا (Expected >= 0x80) {
    سيٽ_Error("Cannot consume non-ascii characters", Current);
    أرجع خطأ;
  }
  إذا (Current == پڇاڙي_2)
    أرجع خطأ;
  إذا (uint8_t(*Current) >= 0x80) {
    سيٽ_Error("Cannot consume non-ascii characters", Current);
    أرجع خطأ;
  }
  إذا (uint8_t(*Current) == Expected) {
    ++Current;
    ++ڪالم;
    أرجع سچ;
  }
  أرجع خطأ;
}

فراغ Scanner::skip(uint32_t مفاصلو_2) {
  Current += مفاصلو_2;
  ڪالم += مفاصلو_2;
  أكد(Current <= پڇاڙي_2 && "Skipped past the end");
}

منطقي Scanner::isBlankOrBreak(StringRef::iterator Position) {
  إذا (Position == پڇاڙي_2)
    أرجع خطأ;
  أرجع *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

منطقي Scanner::is_Plain_محفوظ_Non_Blank(StringRef::iterator Position) {
  إذا (Position == پڇاڙي_2 || isBlankOrBreak(Position))
    أرجع خطأ;
  إذا (FlowLevel &&
      StringRef(Position, 1).ڳوليو_پهريون_جو(",[]{}") != StringRef::npos)
    أرجع خطأ;
  أرجع سچ;
}

منطقي Scanner::is_سٽ_خالي_جايون(StringRef سٽ) {
  لكل (ثابت خودڪار *Position = سٽ.begin(); Position != سٽ.پڇاڙي(); ++Position)
    إذا (!isBlankOrBreak(Position))
      أرجع خطأ;
  أرجع سچ;
}

منطقي Scanner::consume_سٽ_Break_If_Present() {
  خودڪار ايندڙ_2 = skip_b_break(Current);
  إذا (ايندڙ_2 == Current)
    أرجع خطأ;
  ڪالم = 0;
  ++سٽ;
  Current = ايندڙ_2;
  أرجع سچ;
}

فراغ Scanner::save_Simple_ڪنجي_Candidate( Token_قطار_T::iterator Tok
                                    , بنانشان At_ڪالم
                                    , منطقي IsRequired) {
  إذا (Is_Simple_ڪنجي_Allowed) {
    Simple_ڪنجي SK;
    SK.Tok = Tok;
    SK.سٽ = سٽ;
    SK.ڪالم = At_ڪالم;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_ڪنجيون.ڌڪو_پٺ(SK);
  }
}

فراغ Scanner::remove_Stale_Simple_ڪنجي_Candidates() {
  لكل (Small_ويڪٽر_Impl<Simple_ڪنجي>::iterator i = Simple_ڪنجيون.begin();
                                            i != Simple_ڪنجيون.پڇاڙي();) {
    إذا (i->سٽ != سٽ || i->ڪالم + 1024 < ڪالم) {
      إذا (i->IsRequired)
        سيٽ_Error( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_ڪنجيون.erase(i);
    } وإلا {
      ++i;
    }
  }
}

فراغ Scanner::remove_Simple_ڪنجي_Candidates_On_Flow_Level(بنانشان Level) {
  إذا (!Simple_ڪنجيون.خالي_جايون() && (Simple_ڪنجيون.پڇاڙي() - 1)->FlowLevel == Level)
    Simple_ڪنجيون.ڪڍو_پٺ();
}

منطقي Scanner::unrollIndent(صحيح منزل_ڪالم) {
  Token T;
  // Indentation is ignored in flow.
  إذا (FlowLevel != 0)
    أرجع سچ;

  طالما (Indent > منزل_ڪالم) {
    T.قسم_2 = Token::TK_بلاڪ_پڇاڙي;
    T.Range = StringRef(Current, 1);
    Token_قطار.ڌڪو_پٺ(T);
    Indent = Indents.ڪڍو_پٺ_val();
  }

  أرجع سچ;
}

منطقي Scanner::rollIndent( صحيح منزل_ڪالم
                        , Token::Token_قسم قسم_2
                        , Token_قطار_T::iterator وجهو_نقطو) {
  إذا (FlowLevel)
    أرجع سچ;
  إذا (Indent < منزل_ڪالم) {
    Indents.ڌڪو_پٺ(Indent);
    Indent = منزل_ڪالم;

    Token T;
    T.قسم_2 = قسم_2;
    T.Range = StringRef(Current, 0);
    Token_قطار.وجهو(وجهو_نقطو, T);
  }
  أرجع سچ;
}

فراغ Scanner::skipComment() {
  إذا (Current == پڇاڙي_2 || *Current != '#')
    أرجع;
  طالما (سچ) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    إذا (I == Current)
      اكسر;
    Current = I;
    ++ڪالم;
  }
}

فراغ Scanner::جاچ_منزل_ايندڙ_Token() {
  طالما (سچ) {
    طالما (Current != پڇاڙي_2 && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    إذا (i == Current)
      اكسر;
    Current = i;
    ++سٽ;
    ڪالم = 0;
    // New lines may start a simple key.
    إذا (!FlowLevel)
      Is_Simple_ڪنجي_Allowed = سچ;
  }
}

منطقي Scanner::جاچ_وهڪرو_شروعات() {
  Is_شروعات_جو_وهڪرو = خطأ;

  EncodingInfo EI = getUnicodeEncoding(current_داخلا());

  Token T;
  T.قسم_2 = Token::TK_وهڪرو_شروعات;
  T.Range = StringRef(Current, EI.ٻيو_2);
  Token_قطار.ڌڪو_پٺ(T);
  Current += EI.ٻيو_2;
  أرجع سچ;
}

منطقي Scanner::جاچ_وهڪرو_پڇاڙي() {
  // Force an ending new line if one isn't present.
  إذا (ڪالم != 0) {
    ڪالم = 0;
    ++سٽ;
  }

  unrollIndent(-1);
  Simple_ڪنجيون.clear();
  Is_Simple_ڪنجي_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_اندريون_Flow = خطأ;

  Token T;
  T.قسم_2 = Token::TK_وهڪرو_پڇاڙي;
  T.Range = StringRef(Current, 0);
  Token_قطار.ڌڪو_پٺ(T);
  أرجع سچ;
}

منطقي Scanner::جاچ_Directive() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_ڪنجيون.clear();
  Is_Simple_ڪنجي_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_اندريون_Flow = خطأ;

  StringRef::iterator شروعات = Current;
  consume('%');
  StringRef::iterator Name_شروعات = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(Name_شروعات, Current - Name_شروعات);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  إذا (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.قسم_2 = Token::TK_نسخو_Directive;
    T.Range = StringRef(شروعات, Current - شروعات);
    Token_قطار.ڌڪو_پٺ(T);
    أرجع سچ;
  } وإلا إذا(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.قسم_2 = Token::TK_ٽيگ_Directive;
    T.Range = StringRef(شروعات, Current - شروعات);
    Token_قطار.ڌڪو_پٺ(T);
    أرجع سچ;
  }
  أرجع خطأ;
}

منطقي Scanner::جاچ_Document_Indicator(منطقي Is_شروعات) {
  unrollIndent(-1);
  Simple_ڪنجيون.clear();
  Is_Simple_ڪنجي_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_اندريون_Flow = خطأ;

  Token T;
  T.قسم_2 = Is_شروعات ? Token::TK_Document_شروعات : Token::TK_Document_پڇاڙي;
  T.Range = StringRef(Current, 3);
  skip(3);
  Token_قطار.ڌڪو_پٺ(T);
  أرجع سچ;
}

منطقي Scanner::جاچ_Flow_Collection_شروعات(منطقي IsSequence) {
  Token T;
  T.قسم_2 = IsSequence ? Token::TK_Flow_Sequence_شروعات
                      : Token::TK_Flow_Mapping_شروعات;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڌڪو_پٺ(T);

  // [ and { may begin a simple key.
  save_Simple_ڪنجي_Candidate(--Token_قطار.پڇاڙي(), ڪالم - 1, خطأ);

  // And may also be followed by a simple key.
  Is_Simple_ڪنجي_Allowed = سچ;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_قدر_Allowed_اندريون_Flow = خطأ;
  ++FlowLevel;
  أرجع سچ;
}

منطقي Scanner::جاچ_Flow_Collection_پڇاڙي(منطقي IsSequence) {
  remove_Simple_ڪنجي_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ڪنجي_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_اندريون_Flow = سچ;
  Token T;
  T.قسم_2 = IsSequence ? Token::TK_Flow_Sequence_پڇاڙي
                      : Token::TK_Flow_Mapping_پڇاڙي;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڌڪو_پٺ(T);
  إذا (FlowLevel)
    --FlowLevel;
  أرجع سچ;
}

منطقي Scanner::جاچ_Flow_داخلا() {
  remove_Simple_ڪنجي_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ڪنجي_Allowed = سچ;
  Is_Adjacent_قدر_Allowed_اندريون_Flow = خطأ;
  Token T;
  T.قسم_2 = Token::TK_Flow_داخلا;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڌڪو_پٺ(T);
  أرجع سچ;
}

منطقي Scanner::جاچ_بلاڪ_داخلا() {
  rollIndent(ڪالم, Token::TK_بلاڪ_Sequence_شروعات, Token_قطار.پڇاڙي());
  remove_Simple_ڪنجي_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ڪنجي_Allowed = سچ;
  Is_Adjacent_قدر_Allowed_اندريون_Flow = خطأ;
  Token T;
  T.قسم_2 = Token::TK_بلاڪ_داخلا;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڌڪو_پٺ(T);
  أرجع سچ;
}

منطقي Scanner::جاچ_ڪنجي() {
  إذا (!FlowLevel)
    rollIndent(ڪالم, Token::TK_بلاڪ_Mapping_شروعات, Token_قطار.پڇاڙي());

  remove_Simple_ڪنجي_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ڪنجي_Allowed = !FlowLevel;
  Is_Adjacent_قدر_Allowed_اندريون_Flow = خطأ;

  Token T;
  T.قسم_2 = Token::TK_ڪنجي;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڌڪو_پٺ(T);
  أرجع سچ;
}

منطقي Scanner::جاچ_قدر() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  إذا (!Simple_ڪنجيون.خالي_جايون()) {
    Simple_ڪنجي SK = Simple_ڪنجيون.ڪڍو_پٺ_val();
    Token T;
    T.قسم_2 = Token::TK_ڪنجي;
    T.Range = SK.Tok->Range;
    Token_قطار_T::iterator i, e;
    لكل (i = Token_قطار.begin(), e = Token_قطار.پڇاڙي(); i != e; ++i) {
      إذا (i == SK.Tok)
        اكسر;
    }
    إذا (i == e) {
      Failed = سچ;
      أرجع خطأ;
    }
    i = Token_قطار.وجهو(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.ڪالم, Token::TK_بلاڪ_Mapping_شروعات, i);

    Is_Simple_ڪنجي_Allowed = خطأ;
  } وإلا {
    إذا (!FlowLevel)
      rollIndent(ڪالم, Token::TK_بلاڪ_Mapping_شروعات, Token_قطار.پڇاڙي());
    Is_Simple_ڪنجي_Allowed = !FlowLevel;
  }
  Is_Adjacent_قدر_Allowed_اندريون_Flow = خطأ;

  Token T;
  T.قسم_2 = Token::TK_قدر;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڌڪو_پٺ(T);
  أرجع سچ;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE ساكن منطقي
wasEscaped(StringRef::iterator پهريون, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
ساكن منطقي wasEscaped(StringRef::iterator پهريون,
                       StringRef::iterator Position) {
  أكد(Position - 1 >= پهريون);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  طالما (I >= پهريون && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  أرجع (Position - 1 - I) % 2 == 1;
}

منطقي Scanner::جاچ_Flow_Scalar(منطقي IsDoubleQuoted) {
  StringRef::iterator شروعات = Current;
  بنانشان کانو_شروعات = ڪالم;
  إذا (IsDoubleQuoted) {
    افعل {
      ++Current;
      طالما (Current != پڇاڙي_2 && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } طالما (   Current != پڇاڙي_2
             && *(Current - 1) == '\\'
             && wasEscaped(شروعات + 1, Current));
  } وإلا {
    skip(1);
    طالما (Current != پڇاڙي_2) {
      // Skip a ' followed by another '.
      إذا (Current + 1 < پڇاڙي_2 && *Current == '\'' && *(Current + 1) == '\'') {
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
        ڪالم = 0;
        ++سٽ;
      } وإلا {
        إذا (i == پڇاڙي_2)
          اكسر;
        Current = i;
        ++ڪالم;
      }
    }
  }

  إذا (Current == پڇاڙي_2) {
    سيٽ_Error("Expected quote at end of scalar", Current);
    أرجع خطأ;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.قسم_2 = Token::TK_Scalar;
  T.Range = StringRef(شروعات, Current - شروعات);
  Token_قطار.ڌڪو_پٺ(T);

  save_Simple_ڪنجي_Candidate(--Token_قطار.پڇاڙي(), کانو_شروعات, خطأ);

  Is_Simple_ڪنجي_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_اندريون_Flow = سچ;

  أرجع سچ;
}

منطقي Scanner::جاچ_Plain_Scalar() {
  StringRef::iterator شروعات = Current;
  بنانشان کانو_شروعات = ڪالم;
  بنانشان LeadingBlanks = 0;
  أكد(Indent >= -1 && "Indent must be >= -1 !");
  بنانشان indent = static_cast<بنانشان>(Indent + 1);
  طالما (Current != پڇاڙي_2) {
    إذا (*Current == '#')
      اكسر;

    طالما (Current != پڇاڙي_2 &&
           ((*Current != ':' && is_Plain_محفوظ_Non_Blank(Current)) ||
            (*Current == ':' && is_Plain_محفوظ_Non_Blank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      إذا (i == Current)
        اكسر;
      Current = i;
      ++ڪالم;
    }

    // Are we at the end?
    إذا (!isBlankOrBreak(Current))
      اكسر;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    طالما (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      إذا (i != Tmp) {
        إذا (LeadingBlanks && (ڪالم < indent) && *Tmp == '\t') {
          سيٽ_Error("Found invalid tab character in indentation", Tmp);
          أرجع خطأ;
        }
        Tmp = i;
        ++ڪالم;
      } وإلا {
        i = skip_b_break(Tmp);
        إذا (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        ڪالم = 0;
        ++سٽ;
      }
    }

    إذا (!FlowLevel && ڪالم < indent)
      اكسر;

    Current = Tmp;
  }
  إذا (شروعات == Current) {
    سيٽ_Error("Got empty plain scalar", شروعات);
    أرجع خطأ;
  }
  Token T;
  T.قسم_2 = Token::TK_Scalar;
  T.Range = StringRef(شروعات, Current - شروعات);
  Token_قطار.ڌڪو_پٺ(T);

  // Plain scalars can be simple keys.
  save_Simple_ڪنجي_Candidate(--Token_قطار.پڇاڙي(), کانو_شروعات, خطأ);

  Is_Simple_ڪنجي_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_اندريون_Flow = خطأ;

  أرجع سچ;
}

منطقي Scanner::جاچ_Alias_Or_Anchor(منطقي IsAlias) {
  StringRef::iterator شروعات = Current;
  بنانشان کانو_شروعات = ڪالم;
  skip(1);
  طالما (Current != پڇاڙي_2) {
    إذا (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      اكسر;
    StringRef::iterator i = skip_ns_char(Current);
    إذا (i == Current)
      اكسر;
    Current = i;
    ++ڪالم;
  }

  إذا (شروعات + 1 == Current) {
    سيٽ_Error("Got empty alias or anchor", شروعات);
    أرجع خطأ;
  }

  Token T;
  T.قسم_2 = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(شروعات, Current - شروعات);
  Token_قطار.ڌڪو_پٺ(T);

  // Alias and anchors can be simple keys.
  save_Simple_ڪنجي_Candidate(--Token_قطار.پڇاڙي(), کانو_شروعات, خطأ);

  Is_Simple_ڪنجي_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_اندريون_Flow = خطأ;

  أرجع سچ;
}

منطقي Scanner::جاچ_بلاڪ_Scalar_Indicators(حرف &StyleIndicator,
                                        حرف &ChompingIndicator,
                                        بنانشان &IndentIndicator,
                                        منطقي &Is_پورو) {
  StyleIndicator = جاچ_بلاڪ_Style_Indicator();
  إذا (!جاچ_بلاڪ_Scalar_سرو(ChompingIndicator, IndentIndicator, Is_پورو))
    أرجع خطأ;
  أرجع سچ;
}

حرف Scanner::جاچ_بلاڪ_Style_Indicator() {
  حرف Indicator = ' ';
  إذا (Current != پڇاڙي_2 && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  أرجع Indicator;
}

حرف Scanner::جاچ_بلاڪ_Chomping_Indicator() {
  حرف Indicator = ' ';
  إذا (Current != پڇاڙي_2 && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  أرجع Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
ساكن بنانشان get_Chomped_سٽ_Breaks(حرف ChompingIndicator,
                                     بنانشان سٽ_Breaks, StringRef Str) {
  إذا (ChompingIndicator == '-') // Strip all line breaks.
    أرجع 0;
  إذا (ChompingIndicator == '+') // Keep all line breaks.
    أرجع سٽ_Breaks;
  // Clip trailing lines.
  أرجع Str.خالي_جايون() ? 0 : 1;
}

بنانشان Scanner::جاچ_بلاڪ_Indentation_Indicator() {
  بنانشان Indent = 0;
  إذا (Current != پڇاڙي_2 && (*Current >= '1' && *Current <= '9')) {
    Indent = بنانشان(*Current - '0');
    skip(1);
  }
  أرجع Indent;
}

منطقي Scanner::جاچ_بلاڪ_Scalar_سرو(حرف &ChompingIndicator,
                                    بنانشان &IndentIndicator, منطقي &Is_پورو) {
  خودڪار شروعات = Current;

  ChompingIndicator = جاچ_بلاڪ_Chomping_Indicator();
  IndentIndicator = جاچ_بلاڪ_Indentation_Indicator();
  // Check for the chomping indicator once again.
  إذا (ChompingIndicator == ' ')
    ChompingIndicator = جاچ_بلاڪ_Chomping_Indicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  إذا (Current == پڇاڙي_2) { // EOF, we have an empty scalar.
    Token T;
    T.قسم_2 = Token::TK_بلاڪ_Scalar;
    T.Range = StringRef(شروعات, Current - شروعات);
    Token_قطار.ڌڪو_پٺ(T);
    Is_پورو = سچ;
    أرجع سچ;
  }

  إذا (!consume_سٽ_Break_If_Present()) {
    سيٽ_Error("Expected a line break after block scalar header", Current);
    أرجع خطأ;
  }
  أرجع سچ;
}

منطقي Scanner::ڳوليو_بلاڪ_Scalar_Indent(بنانشان &بلاڪ_Indent,
                                    بنانشان بلاڪ_Exit_Indent,
                                    بنانشان &سٽ_Breaks, منطقي &Is_پورو) {
  بنانشان Max_All_Space_سٽ_Characters = 0;
  StringRef::iterator Longest_All_Space_سٽ;

  طالما (سچ) {
    advanceWhile(&Scanner::skip_s_space);
    إذا (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      إذا (ڪالم <= بلاڪ_Exit_Indent) { // End of the block literal.
        Is_پورو = سچ;
        أرجع سچ;
      }
      // We found the block's indentation.
      بلاڪ_Indent = ڪالم;
      إذا (Max_All_Space_سٽ_Characters > بلاڪ_Indent) {
        سيٽ_Error(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_سٽ);
        أرجع خطأ;
      }
      أرجع سچ;
    }
    إذا (skip_b_break(Current) != Current &&
        ڪالم > Max_All_Space_سٽ_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_سٽ_Characters = ڪالم;
      Longest_All_Space_سٽ = Current;
    }

    // Check for EOF.
    إذا (Current == پڇاڙي_2) {
      Is_پورو = سچ;
      أرجع سچ;
    }

    إذا (!consume_سٽ_Break_If_Present()) {
      Is_پورو = سچ;
      أرجع سچ;
    }
    ++سٽ_Breaks;
  }
  أرجع سچ;
}

منطقي Scanner::جاچ_بلاڪ_Scalar_Indent(بنانشان بلاڪ_Indent,
                                    بنانشان بلاڪ_Exit_Indent, منطقي &Is_پورو) {
  // Skip the indentation.
  طالما (ڪالم < بلاڪ_Indent) {
    خودڪار I = skip_s_space(Current);
    إذا (I == Current)
      اكسر;
    Current = I;
    ++ڪالم;
  }

  إذا (skip_nb_char(Current) == Current)
    أرجع سچ;

  إذا (ڪالم <= بلاڪ_Exit_Indent) { // End of the block literal.
    Is_پورو = سچ;
    أرجع سچ;
  }

  إذا (ڪالم < بلاڪ_Indent) {
    إذا (Current != پڇاڙي_2 && *Current == '#') { // Trailing comment.
      Is_پورو = سچ;
      أرجع سچ;
    }
    سيٽ_Error("A text line is less indented than the block scalar", Current);
    أرجع خطأ;
  }
  أرجع سچ; // A normal text line.
}

منطقي Scanner::جاچ_بلاڪ_Scalar(منطقي IsLiteral) {
  أكد(*Current == '|' || *Current == '>');
  حرف StyleIndicator;
  حرف ChompingIndicator;
  بنانشان بلاڪ_Indent;
  منطقي Is_پورو = خطأ;
  إذا (!جاچ_بلاڪ_Scalar_Indicators(StyleIndicator, ChompingIndicator, بلاڪ_Indent,
                                 Is_پورو))
    أرجع خطأ;
  إذا (Is_پورو)
    أرجع سچ;
  منطقي IsFolded = StyleIndicator == '>';

  ثابت خودڪار *شروعات = Current;
  بنانشان بلاڪ_Exit_Indent = Indent < 0 ? 0 : (بنانشان)Indent;
  بنانشان سٽ_Breaks = 0;
  إذا (بلاڪ_Indent == 0) {
    إذا (!ڳوليو_بلاڪ_Scalar_Indent(بلاڪ_Indent, بلاڪ_Exit_Indent, سٽ_Breaks,
                               Is_پورو))
      أرجع خطأ;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  طالما (!Is_پورو) {
    إذا (!جاچ_بلاڪ_Scalar_Indent(بلاڪ_Indent, بلاڪ_Exit_Indent, Is_پورو))
      أرجع خطأ;
    إذا (Is_پورو)
      اكسر;

    // Parse the current line.
    خودڪار سٽ_شروعات = Current;
    advanceWhile(&Scanner::skip_nb_char);
    إذا (سٽ_شروعات != Current) {
      إذا (سٽ_Breaks && IsFolded && !Scanner::is_سٽ_خالي_جايون(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        إذا (سٽ_Breaks == 1) {
          Str.append(سٽ_Breaks,
                     is_سٽ_خالي_جايون(StringRef(سٽ_شروعات, Current - سٽ_شروعات))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        سٽ_Breaks--;
      }
      Str.append(سٽ_Breaks, '\n');
      Str.append(StringRef(سٽ_شروعات, Current - سٽ_شروعات));
      سٽ_Breaks = 0;
    }

    // Check for EOF.
    إذا (Current == پڇاڙي_2)
      اكسر;

    إذا (!consume_سٽ_Break_If_Present())
      اكسر;
    ++سٽ_Breaks;
  }

  إذا (Current == پڇاڙي_2 && !سٽ_Breaks)
    // Ensure that there is at least one line break before the end of file.
    سٽ_Breaks = 1;
  Str.append(get_Chomped_سٽ_Breaks(ChompingIndicator, سٽ_Breaks, Str), '\n');

  // New lines may start a simple key.
  إذا (!FlowLevel)
    Is_Simple_ڪنجي_Allowed = سچ;
  Is_Adjacent_قدر_Allowed_اندريون_Flow = خطأ;

  Token T;
  T.قسم_2 = Token::TK_بلاڪ_Scalar;
  T.Range = StringRef(شروعات, Current - شروعات);
  T.قدر = std::string(Str);
  Token_قطار.ڌڪو_پٺ(T);
  أرجع سچ;
}

منطقي Scanner::جاچ_ٽيگ() {
  StringRef::iterator شروعات = Current;
  بنانشان کانو_شروعات = ڪالم;
  skip(1); // Eat !.
  إذا (Current == پڇاڙي_2 || isBlankOrBreak(Current)); // An empty tag.
  وإلا إذا (*Current == '<') {
    skip(1);
    جاچ_ns_uri_char();
    إذا (!consume('>'))
      أرجع خطأ;
  } وإلا {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.قسم_2 = Token::TK_ٽيگ;
  T.Range = StringRef(شروعات, Current - شروعات);
  Token_قطار.ڌڪو_پٺ(T);

  // Tags can be simple keys.
  save_Simple_ڪنجي_Candidate(--Token_قطار.پڇاڙي(), کانو_شروعات, خطأ);

  Is_Simple_ڪنجي_Allowed = خطأ;
  Is_Adjacent_قدر_Allowed_اندريون_Flow = خطأ;

  أرجع سچ;
}

منطقي Scanner::fetch_وڌيڪ_ٽوڪن() {
  إذا (Is_شروعات_جو_وهڪرو)
    أرجع جاچ_وهڪرو_شروعات();

  جاچ_منزل_ايندڙ_Token();

  إذا (Current == پڇاڙي_2)
    أرجع جاچ_وهڪرو_پڇاڙي();

  remove_Stale_Simple_ڪنجي_Candidates();

  unrollIndent(ڪالم);

  إذا (ڪالم == 0 && *Current == '%')
    أرجع جاچ_Directive();

  إذا (ڪالم == 0 && Current + 3 <= پڇاڙي_2
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == پڇاڙي_2 || isBlankOrBreak(Current + 3)))
    أرجع جاچ_Document_Indicator(سچ);

  إذا (ڪالم == 0 && Current + 3 <= پڇاڙي_2
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == پڇاڙي_2 || isBlankOrBreak(Current + 3)))
    أرجع جاچ_Document_Indicator(خطأ);

  إذا (*Current == '[')
    أرجع جاچ_Flow_Collection_شروعات(سچ);

  إذا (*Current == '{')
    أرجع جاچ_Flow_Collection_شروعات(خطأ);

  إذا (*Current == ']')
    أرجع جاچ_Flow_Collection_پڇاڙي(سچ);

  إذا (*Current == '}')
    أرجع جاچ_Flow_Collection_پڇاڙي(خطأ);

  إذا (*Current == ',')
    أرجع جاچ_Flow_داخلا();

  إذا (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == پڇاڙي_2))
    أرجع جاچ_بلاڪ_داخلا();

  إذا (*Current == '?' && (Current + 1 == پڇاڙي_2 || isBlankOrBreak(Current + 1)))
    أرجع جاچ_ڪنجي();

  إذا (*Current == ':' &&
      (!is_Plain_محفوظ_Non_Blank(Current + 1) || Is_Adjacent_قدر_Allowed_اندريون_Flow))
    أرجع جاچ_قدر();

  إذا (*Current == '*')
    أرجع جاچ_Alias_Or_Anchor(سچ);

  إذا (*Current == '&')
    أرجع جاچ_Alias_Or_Anchor(خطأ);

  إذا (*Current == '!')
    أرجع جاچ_ٽيگ();

  إذا (*Current == '|' && !FlowLevel)
    أرجع جاچ_بلاڪ_Scalar(سچ);

  إذا (*Current == '>' && !FlowLevel)
    أرجع جاچ_بلاڪ_Scalar(خطأ);

  إذا (*Current == '\'')
    أرجع جاچ_Flow_Scalar(خطأ);

  إذا (*Current == '"')
    أرجع جاچ_Flow_Scalar(سچ);

  // Get a plain scalar.
  StringRef پهريون_Char(Current, 1);
  إذا ((!isBlankOrBreak(Current) &&
       پهريون_Char.ڳوليو_پهريون_جو("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (پهريون_Char.ڳوليو_پهريون_جو("?:-") != StringRef::npos &&
       is_Plain_محفوظ_Non_Blank(Current + 1)))
    أرجع جاچ_Plain_Scalar();

  سيٽ_Error("Unrecognized character while tokenizing.", Current);
  أرجع خطأ;
}

وهڪرو::وهڪرو(StringRef داخلا, ذريعو_Mgr &SM, منطقي ShowColors,
               std::error_ڪوڊ *EC)
    : scanner(جديد Scanner(داخلا, SM, ShowColors, EC)) {}

وهڪرو::وهڪرو(يادگيري_بفر_Ref داخلا_بفر, ذريعو_Mgr &SM, منطقي ShowColors,
               std::error_ڪوڊ *EC)
    : scanner(جديد Scanner(داخلا_بفر, SM, ShowColors, EC)) {}

وهڪرو::~وهڪرو() = افتراضي;

منطقي وهڪرو::failed() { أرجع scanner->failed(); }

فراغ وهڪرو::printError(ڳنڍ *N, ثابت Twine &نياپو_2, ذريعو_Mgr::Diag_قسم قسم_2) {
  printError(N ? N->get_ذريعو_Range() : SMRange(), نياپو_2, قسم_2);
}

فراغ وهڪرو::printError(ثابت SMRange &Range, ثابت Twine &نياپو_2,
                        ذريعو_Mgr::Diag_قسم قسم_2) {
  scanner->printError(Range.شروعات, قسم_2, نياپو_2, Range);
}

document_iterator وهڪرو::begin() {
  إذا (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->get_ايندڙ();

  CurrentDoc.reset(جديد Document(*هذا));
  أرجع document_iterator(CurrentDoc);
}

document_iterator وهڪرو::پڇاڙي() {
  أرجع document_iterator();
}

فراغ وهڪرو::skip() {
  لكل (Document &Doc : *هذا)
    Doc.skip();
}

ڳنڍ::ڳنڍ(بنانشان صحيح Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), ٽيگ(T) {
  SMLoc شروعات = SMLoc::get_From_اشارو(peek_ايندڙ().Range.begin());
  ذريعو_Range = SMRange(شروعات, شروعات);
}

std::string ڳنڍ::get_Verbatim_ٽيگ() ثابت {
  StringRef Raw = get_Raw_ٽيگ();
  إذا (!Raw.خالي_جايون() && Raw != "!") {
    std::string Ret;
    إذا (Raw.ڳوليو_آخري_جو('!') == 0) {
      Ret = std::string(Doc->get_ٽيگ_Map().ڳوليو("!")->ٻيو_2);
      Ret += Raw.substr(1);
      أرجع Ret;
    } وإلا إذا (Raw.starts_with("!!")) {
      Ret = std::string(Doc->get_ٽيگ_Map().ڳوليو("!!")->ٻيو_2);
      Ret += Raw.substr(2);
      أرجع Ret;
    } وإلا {
      StringRef ٽيگ_Handle = Raw.substr(0, Raw.ڳوليو_آخري_جو('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->get_ٽيگ_Map().ڳوليو(ٽيگ_Handle);
      إذا (It != Doc->get_ٽيگ_Map().پڇاڙي())
        Ret = std::string(It->ٻيو_2);
      وإلا {
        Token T;
        T.قسم_2 = Token::TK_ٽيگ;
        T.Range = ٽيگ_Handle;
        سيٽ_Error(Twine("Unknown tag handle ") + ٽيگ_Handle, T);
      }
      Ret += Raw.substr(Raw.ڳوليو_آخري_جو('!') + 1);
      أرجع Ret;
    }
  }

  بدل (getType()) {
  حالة NK_Null:
    أرجع "tag:yaml.org,2002:null";
  حالة NK_Scalar:
  حالة NK_بلاڪ_Scalar:
    // TODO: Tag resolution.
    أرجع "tag:yaml.org,2002:str";
  حالة NK_Mapping:
    أرجع "tag:yaml.org,2002:map";
  حالة NK_Sequence:
    أرجع "tag:yaml.org,2002:seq";
  }

  أرجع "";
}

Token &ڳنڍ::peek_ايندڙ() {
  أرجع Doc->peek_ايندڙ();
}

Token ڳنڍ::get_ايندڙ() {
  أرجع Doc->get_ايندڙ();
}

ڳنڍ *ڳنڍ::parse_بلاڪ_ڳنڍ() {
  أرجع Doc->parse_بلاڪ_ڳنڍ();
}

BumpPtrAllocator &ڳنڍ::getAllocator() {
  أرجع Doc->ڳنڍ_Allocator;
}

فراغ ڳنڍ::سيٽ_Error(ثابت Twine &نياپو_2, Token &Tok) ثابت {
  Doc->سيٽ_Error(نياپو_2, Tok);
}

منطقي ڳنڍ::failed() ثابت {
  أرجع Doc->failed();
}

StringRef Scalar_ڳنڍ::get_قدر(Small_ويڪٽر_Impl<حرف> &Storage) ثابت {
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
parse_Scalar_قدر(StringRef Unquoted_قدر, Small_ويڪٽر_Impl<حرف> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, Small_ويڪٽر_Impl<حرف> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_قدر.ڳوليو_پهريون_جو(LookupChars);
  إذا (I == StringRef::npos)
    أرجع Unquoted_قدر;

  Storage.clear();
  Storage.reserve(Unquoted_قدر.سائيز_2());
  حرف آخري_New_سٽ_Added_As = '\0';
  لكل (; I != StringRef::npos; I = Unquoted_قدر.ڳوليو_پهريون_جو(LookupChars)) {
    إذا (Unquoted_قدر[I] != '\r' && Unquoted_قدر[I] != '\n') {
      llvm::append_range(Storage, Unquoted_قدر.take_front(I));
      Unquoted_قدر = UnescapeCallback(Unquoted_قدر.drop_front(I), Storage);
      آخري_New_سٽ_Added_As = '\0';
      استمر;
    }
    إذا (size_t آخري_Non_SWhite = Unquoted_قدر.ڳوليو_آخري_not_جو(" \t", I);
        آخري_Non_SWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_قدر.take_front(آخري_Non_SWhite + 1));
      Storage.ڌڪو_پٺ(' ');
      آخري_New_سٽ_Added_As = ' ';
    } وإلا {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      بدل (آخري_New_سٽ_Added_As) {
      حالة ' ':
        أكد(!Storage.خالي_جايون() && Storage.پٺ() == ' ');
        Storage.پٺ() = '\n';
        آخري_New_سٽ_Added_As = '\n';
        اكسر;
      حالة '\n':
        أكد(!Storage.خالي_جايون() && Storage.پٺ() == '\n');
        Storage.ڌڪو_پٺ('\n');
        اكسر;
      افتراضي:
        Storage.ڌڪو_پٺ(' ');
        آخري_New_سٽ_Added_As = ' ';
        اكسر;
      }
    }
    // Handle Windows-style EOL
    إذا (Unquoted_قدر.substr(I, 2) == "\r\n")
      I++;
    Unquoted_قدر = Unquoted_قدر.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_قدر);
  أرجع StringRef(Storage.begin(), Storage.سائيز_2());
}

StringRef
Scalar_ڳنڍ::get_Double_Quoted_قدر(StringRef Raw_قدر,
                                 Small_ويڪٽر_Impl<حرف> &Storage) ثابت {
  أكد(Raw_قدر.سائيز_2() >= 2 && Raw_قدر.front() == '"' &&
         Raw_قدر.پٺ() == '"');
  StringRef Unquoted_قدر = Raw_قدر.substr(1, Raw_قدر.سائيز_2() - 2);

  خودڪار UnescapeFunc = [هذا](StringRef Unquoted_قدر,
                             Small_ويڪٽر_Impl<حرف> &Storage) {
    أكد(Unquoted_قدر.take_front(1) == "\\");
    إذا (Unquoted_قدر.سائيز_2() == 1) {
      Token T;
      T.Range = Unquoted_قدر;
      سيٽ_Error("Unrecognized escape code", T);
      Storage.clear();
      أرجع StringRef();
    }
    Unquoted_قدر = Unquoted_قدر.drop_front(1);
    بدل (Unquoted_قدر[0]) {
    افتراضي: {
      Token T;
      T.Range = Unquoted_قدر.take_front(1);
      سيٽ_Error("Unrecognized escape code", T);
      Storage.clear();
      أرجع StringRef();
    }
    حالة '\r':
      // Shrink the Windows-style EOL.
      إذا (Unquoted_قدر.سائيز_2() >= 2 && Unquoted_قدر[1] == '\n')
        Unquoted_قدر = Unquoted_قدر.drop_front(1);
      [[fallthrough]];
    حالة '\n':
      أرجع Unquoted_قدر.drop_front(1).ltrim(" \t");
    حالة '0':
      Storage.ڌڪو_پٺ(0x00);
      اكسر;
    حالة 'a':
      Storage.ڌڪو_پٺ(0x07);
      اكسر;
    حالة 'b':
      Storage.ڌڪو_پٺ(0x08);
      اكسر;
    حالة 't':
    حالة 0x09:
      Storage.ڌڪو_پٺ(0x09);
      اكسر;
    حالة 'n':
      Storage.ڌڪو_پٺ(0x0A);
      اكسر;
    حالة 'v':
      Storage.ڌڪو_پٺ(0x0B);
      اكسر;
    حالة 'f':
      Storage.ڌڪو_پٺ(0x0C);
      اكسر;
    حالة 'r':
      Storage.ڌڪو_پٺ(0x0D);
      اكسر;
    حالة 'e':
      Storage.ڌڪو_پٺ(0x1B);
      اكسر;
    حالة ' ':
      Storage.ڌڪو_پٺ(0x20);
      اكسر;
    حالة '"':
      Storage.ڌڪو_پٺ(0x22);
      اكسر;
    حالة '/':
      Storage.ڌڪو_پٺ(0x2F);
      اكسر;
    حالة '\\':
      Storage.ڌڪو_پٺ(0x5C);
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
      إذا (Unquoted_قدر.سائيز_2() < 3)
        // TODO: Report error.
        اكسر;
      بنانشان صحيح Unicode_Scalar_قدر;
      إذا (Unquoted_قدر.substr(1, 2).getAsInteger(16, Unicode_Scalar_قدر))
        // TODO: Report error.
        Unicode_Scalar_قدر = 0xFFFD;
      encodeUTF8(Unicode_Scalar_قدر, Storage);
      أرجع Unquoted_قدر.drop_front(3);
    }
    حالة 'u': {
      إذا (Unquoted_قدر.سائيز_2() < 5)
        // TODO: Report error.
        اكسر;
      بنانشان صحيح Unicode_Scalar_قدر;
      إذا (Unquoted_قدر.substr(1, 4).getAsInteger(16, Unicode_Scalar_قدر))
        // TODO: Report error.
        Unicode_Scalar_قدر = 0xFFFD;
      encodeUTF8(Unicode_Scalar_قدر, Storage);
      أرجع Unquoted_قدر.drop_front(5);
    }
    حالة 'U': {
      إذا (Unquoted_قدر.سائيز_2() < 9)
        // TODO: Report error.
        اكسر;
      بنانشان صحيح Unicode_Scalar_قدر;
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

StringRef Scalar_ڳنڍ::get_Single_Quoted_قدر(StringRef Raw_قدر,
                                           Small_ويڪٽر_Impl<حرف> &Storage) {
  أكد(Raw_قدر.سائيز_2() >= 2 && Raw_قدر.front() == '\'' &&
         Raw_قدر.پٺ() == '\'');
  StringRef Unquoted_قدر = Raw_قدر.substr(1, Raw_قدر.سائيز_2() - 2);

  خودڪار UnescapeFunc = [](StringRef Unquoted_قدر,
                         Small_ويڪٽر_Impl<حرف> &Storage) {
    أكد(Unquoted_قدر.take_front(2) == "''");
    Storage.ڌڪو_پٺ('\'');
    أرجع Unquoted_قدر.drop_front(2);
  };

  أرجع parse_Scalar_قدر(Unquoted_قدر, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_ڳنڍ::get_Plain_قدر(StringRef Raw_قدر,
                                    Small_ويڪٽر_Impl<حرف> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_قدر = Raw_قدر.rtrim("\r\n \t");
  أرجع parse_Scalar_قدر(Raw_قدر, Storage, "\r\n", خالي_اشارو);
}

ڳنڍ *ڪنجي_قدر_ڳنڍ::get_ڪنجي() {
  إذا (ڪنجي)
    أرجع ڪنجي;
  // Handle implicit null keys.
  {
    Token &t = peek_ايندڙ();
    إذا (   t.قسم_2 == Token::TK_بلاڪ_پڇاڙي
        || t.قسم_2 == Token::TK_قدر
        || t.قسم_2 == Token::TK_Error) {
      أرجع ڪنجي = جديد (getAllocator()) Null_ڳنڍ(Doc);
    }
    إذا (t.قسم_2 == Token::TK_ڪنجي)
      get_ايندڙ(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peek_ايندڙ();
  إذا (t.قسم_2 == Token::TK_بلاڪ_پڇاڙي || t.قسم_2 == Token::TK_قدر) {
    أرجع ڪنجي = جديد (getAllocator()) Null_ڳنڍ(Doc);
  }

  // We've got a normal key.
  أرجع ڪنجي = parse_بلاڪ_ڳنڍ();
}

ڳنڍ *ڪنجي_قدر_ڳنڍ::get_قدر() {
  إذا (قدر)
    أرجع قدر;

  إذا (ڳنڍ* ڪنجي = get_ڪنجي())
    ڪنجي->skip();
  وإلا {
    سيٽ_Error("Null key in Key Value.", peek_ايندڙ());
    أرجع قدر = جديد (getAllocator()) Null_ڳنڍ(Doc);
  }

  إذا (failed())
    أرجع قدر = جديد (getAllocator()) Null_ڳنڍ(Doc);

  // Handle implicit null values.
  {
    Token &t = peek_ايندڙ();
    إذا (   t.قسم_2 == Token::TK_بلاڪ_پڇاڙي
        || t.قسم_2 == Token::TK_Flow_Mapping_پڇاڙي
        || t.قسم_2 == Token::TK_ڪنجي
        || t.قسم_2 == Token::TK_Flow_داخلا
        || t.قسم_2 == Token::TK_Error) {
      أرجع قدر = جديد (getAllocator()) Null_ڳنڍ(Doc);
    }

    إذا (t.قسم_2 != Token::TK_قدر) {
      سيٽ_Error("Unexpected token in Key Value.", t);
      أرجع قدر = جديد (getAllocator()) Null_ڳنڍ(Doc);
    }
    get_ايندڙ(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peek_ايندڙ();
  إذا (t.قسم_2 == Token::TK_بلاڪ_پڇاڙي || t.قسم_2 == Token::TK_ڪنجي) {
    أرجع قدر = جديد (getAllocator()) Null_ڳنڍ(Doc);
  }

  // We got a normal value.
  أرجع قدر = parse_بلاڪ_ڳنڍ();
}

فراغ Mapping_ڳنڍ::increment() {
  إذا (failed()) {
    Is_At_پڇاڙي = سچ;
    Current_داخلا = خالي_اشارو;
    أرجع;
  }
  إذا (Current_داخلا) {
    Current_داخلا->skip();
    إذا (Type == MT_Inline) {
      Is_At_پڇاڙي = سچ;
      Current_داخلا = خالي_اشارو;
      أرجع;
    }
  }
  Token T = peek_ايندڙ();
  إذا (T.قسم_2 == Token::TK_ڪنجي || T.قسم_2 == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    Current_داخلا = جديد (getAllocator()) ڪنجي_قدر_ڳنڍ(Doc);
  } وإلا إذا (Type == MT_بلاڪ) {
    بدل (T.قسم_2) {
    حالة Token::TK_بلاڪ_پڇاڙي:
      get_ايندڙ();
      Is_At_پڇاڙي = سچ;
      Current_داخلا = خالي_اشارو;
      اكسر;
    افتراضي:
      سيٽ_Error("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    حالة Token::TK_Error:
      Is_At_پڇاڙي = سچ;
      Current_داخلا = خالي_اشارو;
    }
  } وإلا {
    بدل (T.قسم_2) {
    حالة Token::TK_Flow_داخلا:
      // Eat the flow entry and recurse.
      get_ايندڙ();
      أرجع increment();
    حالة Token::TK_Flow_Mapping_پڇاڙي:
      get_ايندڙ();
      [[fallthrough]];
    حالة Token::TK_Error:
      // Set this to end iterator.
      Is_At_پڇاڙي = سچ;
      Current_داخلا = خالي_اشارو;
      اكسر;
    افتراضي:
      سيٽ_Error( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      Is_At_پڇاڙي = سچ;
      Current_داخلا = خالي_اشارو;
    }
  }
}

فراغ Sequence_ڳنڍ::increment() {
  إذا (failed()) {
    Is_At_پڇاڙي = سچ;
    Current_داخلا = خالي_اشارو;
    أرجع;
  }
  إذا (Current_داخلا)
    Current_داخلا->skip();
  Token T = peek_ايندڙ();
  إذا (SeqType == ST_بلاڪ) {
    بدل (T.قسم_2) {
    حالة Token::TK_بلاڪ_داخلا:
      get_ايندڙ();
      Current_داخلا = parse_بلاڪ_ڳنڍ();
      إذا (!Current_داخلا) { // An error occurred.
        Is_At_پڇاڙي = سچ;
        Current_داخلا = خالي_اشارو;
      }
      اكسر;
    حالة Token::TK_بلاڪ_پڇاڙي:
      get_ايندڙ();
      Is_At_پڇاڙي = سچ;
      Current_داخلا = خالي_اشارو;
      اكسر;
    افتراضي:
      سيٽ_Error( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    حالة Token::TK_Error:
      Is_At_پڇاڙي = سچ;
      Current_داخلا = خالي_اشارو;
    }
  } وإلا إذا (SeqType == ST_Indentless) {
    بدل (T.قسم_2) {
    حالة Token::TK_بلاڪ_داخلا:
      get_ايندڙ();
      Current_داخلا = parse_بلاڪ_ڳنڍ();
      إذا (!Current_داخلا) { // An error occurred.
        Is_At_پڇاڙي = سچ;
        Current_داخلا = خالي_اشارو;
      }
      اكسر;
    افتراضي:
    حالة Token::TK_Error:
      Is_At_پڇاڙي = سچ;
      Current_داخلا = خالي_اشارو;
    }
  } وإلا إذا (SeqType == ST_Flow) {
    بدل (T.قسم_2) {
    حالة Token::TK_Flow_داخلا:
      // Eat the flow entry and recurse.
      get_ايندڙ();
      Was_اڳيون_Token_Flow_داخلا = سچ;
      أرجع increment();
    حالة Token::TK_Flow_Sequence_پڇاڙي:
      get_ايندڙ();
      [[fallthrough]];
    حالة Token::TK_Error:
      // Set this to end iterator.
      Is_At_پڇاڙي = سچ;
      Current_داخلا = خالي_اشارو;
      اكسر;
    حالة Token::TK_وهڪرو_پڇاڙي:
    حالة Token::TK_Document_پڇاڙي:
    حالة Token::TK_Document_شروعات:
      سيٽ_Error("Could not find closing ]!", T);
      // Set this to end iterator.
      Is_At_پڇاڙي = سچ;
      Current_داخلا = خالي_اشارو;
      اكسر;
    افتراضي:
      إذا (!Was_اڳيون_Token_Flow_داخلا) {
        سيٽ_Error("Expected , between entries!", T);
        Is_At_پڇاڙي = سچ;
        Current_داخلا = خالي_اشارو;
        اكسر;
      }
      // Otherwise it must be a flow entry.
      Current_داخلا = parse_بلاڪ_ڳنڍ();
      إذا (!Current_داخلا) {
        Is_At_پڇاڙي = سچ;
      }
      Was_اڳيون_Token_Flow_داخلا = خطأ;
      اكسر;
    }
  }
}

Document::Document(وهڪرو &S) : وهڪرو_2(S), پاڙ_2(خالي_اشارو) {
  // Tag maps starts with two default mappings.
  ٽيگ_Map["!"] = "!";
  ٽيگ_Map["!!"] = "tag:yaml.org,2002:";

  إذا (parseDirectives())
    expectToken(Token::TK_Document_شروعات);
  Token &T = peek_ايندڙ();
  إذا (T.قسم_2 == Token::TK_Document_شروعات)
    get_ايندڙ();
}

منطقي Document::skip()  {
  إذا (وهڪرو_2.scanner->failed())
    أرجع خطأ;
  إذا (!پاڙ_2 && !get_پاڙ())
    أرجع خطأ;
  پاڙ_2->skip();
  Token &T = peek_ايندڙ();
  إذا (T.قسم_2 == Token::TK_وهڪرو_پڇاڙي)
    أرجع خطأ;
  إذا (T.قسم_2 == Token::TK_Document_پڇاڙي) {
    get_ايندڙ();
    أرجع skip();
  }
  أرجع سچ;
}

Token &Document::peek_ايندڙ() {
  أرجع وهڪرو_2.scanner->peek_ايندڙ();
}

Token Document::get_ايندڙ() {
  أرجع وهڪرو_2.scanner->get_ايندڙ();
}

فراغ Document::سيٽ_Error(ثابت Twine &پيغام_2, Token &Location) ثابت {
  وهڪرو_2.scanner->سيٽ_Error(پيغام_2, Location.Range.begin());
}

منطقي Document::failed() ثابت {
  أرجع وهڪرو_2.scanner->failed();
}

ڳنڍ *Document::parse_بلاڪ_ڳنڍ() {
  Token T = peek_ايندڙ();
  // Handle properties.
  Token AnchorInfo;
  Token ٽيگ_Info;
parse_property:
  بدل (T.قسم_2) {
  حالة Token::TK_Alias:
    get_ايندڙ();
    أرجع جديد (ڳنڍ_Allocator) Alias_ڳنڍ(وهڪرو_2.CurrentDoc, T.Range.substr(1));
  حالة Token::TK_Anchor:
    إذا (AnchorInfo.قسم_2 == Token::TK_Anchor) {
      سيٽ_Error("Already encountered an anchor for this node!", T);
      أرجع خالي_اشارو;
    }
    AnchorInfo = get_ايندڙ(); // Consume TK_Anchor.
    T = peek_ايندڙ();
    اذهب parse_property;
  حالة Token::TK_ٽيگ:
    إذا (ٽيگ_Info.قسم_2 == Token::TK_ٽيگ) {
      سيٽ_Error("Already encountered a tag for this node!", T);
      أرجع خالي_اشارو;
    }
    ٽيگ_Info = get_ايندڙ(); // Consume TK_Tag.
    T = peek_ايندڙ();
    اذهب parse_property;
  افتراضي:
    اكسر;
  }

  بدل (T.قسم_2) {
  حالة Token::TK_بلاڪ_داخلا:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    أرجع جديد (ڳنڍ_Allocator) Sequence_ڳنڍ( وهڪرو_2.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , ٽيگ_Info.Range
                                           , Sequence_ڳنڍ::ST_Indentless);
  حالة Token::TK_بلاڪ_Sequence_شروعات:
    get_ايندڙ();
    أرجع جديد (ڳنڍ_Allocator)
      Sequence_ڳنڍ( وهڪرو_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , ٽيگ_Info.Range
                  , Sequence_ڳنڍ::ST_بلاڪ);
  حالة Token::TK_بلاڪ_Mapping_شروعات:
    get_ايندڙ();
    أرجع جديد (ڳنڍ_Allocator)
      Mapping_ڳنڍ( وهڪرو_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , ٽيگ_Info.Range
                 , Mapping_ڳنڍ::MT_بلاڪ);
  حالة Token::TK_Flow_Sequence_شروعات:
    get_ايندڙ();
    أرجع جديد (ڳنڍ_Allocator)
      Sequence_ڳنڍ( وهڪرو_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , ٽيگ_Info.Range
                  , Sequence_ڳنڍ::ST_Flow);
  حالة Token::TK_Flow_Mapping_شروعات:
    get_ايندڙ();
    أرجع جديد (ڳنڍ_Allocator)
      Mapping_ڳنڍ( وهڪرو_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , ٽيگ_Info.Range
                 , Mapping_ڳنڍ::MT_Flow);
  حالة Token::TK_Scalar:
    get_ايندڙ();
    أرجع جديد (ڳنڍ_Allocator)
      Scalar_ڳنڍ( وهڪرو_2.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , ٽيگ_Info.Range
                , T.Range);
  حالة Token::TK_بلاڪ_Scalar: {
    get_ايندڙ();
    StringRef NullTerminatedStr(T.قدر.c_str(), T.قدر.ڊيگهائي_2() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(ڳنڍ_Allocator).drop_پٺ();
    أرجع جديد (ڳنڍ_Allocator)
        بلاڪ_Scalar_ڳنڍ(وهڪرو_2.CurrentDoc, AnchorInfo.Range.substr(1),
                        ٽيگ_Info.Range, StrCopy, T.Range);
  }
  حالة Token::TK_ڪنجي:
    // Don't eat the TK_Key, KeyValueNode expects it.
    أرجع جديد (ڳنڍ_Allocator)
      Mapping_ڳنڍ( وهڪرو_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , ٽيگ_Info.Range
                 , Mapping_ڳنڍ::MT_Inline);
  حالة Token::TK_Document_شروعات:
  حالة Token::TK_Document_پڇاڙي:
  حالة Token::TK_وهڪرو_پڇاڙي:
  افتراضي:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    أرجع جديد (ڳنڍ_Allocator) Null_ڳنڍ(وهڪرو_2.CurrentDoc);
  حالة Token::TK_Flow_Mapping_پڇاڙي:
  حالة Token::TK_Flow_Sequence_پڇاڙي:
  حالة Token::TK_Flow_داخلا: {
    إذا (پاڙ_2 && (isa<Mapping_ڳنڍ>(پاڙ_2) || isa<Sequence_ڳنڍ>(پاڙ_2)))
      أرجع جديد (ڳنڍ_Allocator) Null_ڳنڍ(وهڪرو_2.CurrentDoc);

    سيٽ_Error("Unexpected token", T);
    أرجع خالي_اشارو;
  }
  حالة Token::TK_Error:
    أرجع خالي_اشارو;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  أرجع خالي_اشارو;
}

منطقي Document::parseDirectives() {
  منطقي isDirective = خطأ;
  طالما (سچ) {
    Token T = peek_ايندڙ();
    إذا (T.قسم_2 == Token::TK_ٽيگ_Directive) {
      parseTAGDirective();
      isDirective = سچ;
    } وإلا إذا (T.قسم_2 == Token::TK_نسخو_Directive) {
      parseYAMLDirective();
      isDirective = سچ;
    } وإلا {
      اكسر;
    }
  }
  أرجع isDirective;
}

فراغ Document::parseYAMLDirective() {
  get_ايندڙ(); // Eat %YAML <version>
}

فراغ Document::parseTAGDirective() {
  Token ٽيگ = get_ايندڙ(); // %TAG <handle> <prefix>
  StringRef T = ٽيگ.Range;
  // Strip %TAG
  T = T.substr(T.ڳوليو_پهريون_جو(" \t")).ltrim(" \t");
  std::size_t Handle_پڇاڙي = T.ڳوليو_پهريون_جو(" \t");
  StringRef ٽيگ_Handle = T.substr(0, Handle_پڇاڙي);
  StringRef ٽيگ_اڳياڙي = T.substr(Handle_پڇاڙي).ltrim(" \t");
  ٽيگ_Map[ٽيگ_Handle] = ٽيگ_اڳياڙي;
}

منطقي Document::expectToken(صحيح TK) {
  Token T = get_ايندڙ();
  إذا (T.قسم_2 != TK) {
    سيٽ_Error("Unexpected token", T);
    أرجع خطأ;
  }
  أرجع سچ;
}
