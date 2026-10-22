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

کارول نطاق llvm;
کارول نطاق yaml;

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
کارول EncodingInfo = std::pair<UnicodeEncodingForm, بینښه>;

/// getUnicodeEncoding - Reads up to the first 4 bytes to determine the Unicode
///                      encoding form of \a Input.
///
/// @param Input A string of length 0 or more.
/// @returns An EncodingInfo indicating the Unicode encoding form of the input
///          and how long the byte order mark is if one exists.
ساكن EncodingInfo getUnicodeEncoding(StringRef ننوت) {
  إذا (ننوت.تشه())
    أرجع {UEF_Unknown, 0};

  بدل (uint8_t(ننوت[0])) {
  حالة 0x00:
    إذا (ننوت.اندازه_2() >= 4) {
      إذا (  ننوت[1] == 0
         && uint8_t(ننوت[2]) == 0xFE
         && uint8_t(ننوت[3]) == 0xFF)
        أرجع {UEF_UTF32_BE, 4};
      إذا (ننوت[1] == 0 && ننوت[2] == 0 && ننوت[3] != 0)
        أرجع {UEF_UTF32_BE, 0};
    }

    إذا (ننوت.اندازه_2() >= 2 && ننوت[1] != 0)
      أرجع {UEF_UTF16_BE, 0};
    أرجع {UEF_Unknown, 0};
  حالة 0xFF:
    إذا (  ننوت.اندازه_2() >= 4
       && uint8_t(ننوت[1]) == 0xFE
       && ننوت[2] == 0
       && ننوت[3] == 0)
      أرجع {UEF_UTF32_LE, 4};

    إذا (ننوت.اندازه_2() >= 2 && uint8_t(ننوت[1]) == 0xFE)
      أرجع {UEF_UTF16_LE, 2};
    أرجع {UEF_Unknown, 0};
  حالة 0xFE:
    إذا (ننوت.اندازه_2() >= 2 && uint8_t(ننوت[1]) == 0xFF)
      أرجع {UEF_UTF16_BE, 2};
    أرجع {UEF_Unknown, 0};
  حالة 0xEF:
    إذا (  ننوت.اندازه_2() >= 3
       && uint8_t(ننوت[1]) == 0xBB
       && uint8_t(ننوت[2]) == 0xBF)
      أرجع {UEF_UTF8, 3};
    أرجع {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  إذا (ننوت.اندازه_2() >= 4 && ننوت[1] == 0 && ننوت[2] == 0 && ننوت[3] == 0)
    أرجع {UEF_UTF32_LE, 0};

  إذا (ننوت.اندازه_2() >= 2 && ننوت[1] == 0)
    أرجع {UEF_UTF16_LE, 0};

  أرجع {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
فراغ غوټه::anchor() {}
فراغ Null_غوټه::anchor() {}
فراغ Scalar_غوټه::anchor() {}
فراغ بلاک_Scalar_غوټه::anchor() {}
فراغ کلی_ارزښت_غوټه::anchor() {}
فراغ Mapping_غوټه::anchor() {}
فراغ Sequence_غوټه::anchor() {}
فراغ Alias_غوټه::anchor() {}

نطاق llvm {
نطاق yaml {

/// Token - A single YAML token.
بنية Token {
  تعداد Token_ډول {
    TK_Error, // Uninitialized token.
    TK_بهیر_پیل,
    TK_بهیر_پای_ټکی,
    TK_نسخه_Directive,
    TK_نښان_Directive,
    TK_Document_پیل,
    TK_Document_پای_ټکی,
    TK_بلاک_ثبت,
    TK_بلاک_پای_ټکی,
    TK_بلاک_Sequence_پیل,
    TK_بلاک_Mapping_پیل,
    TK_Flow_ثبت,
    TK_Flow_Sequence_پیل,
    TK_Flow_Sequence_پای_ټکی,
    TK_Flow_Mapping_پیل,
    TK_Flow_Mapping_پای_ټکی,
    TK_کلی,
    TK_ارزښت,
    TK_Scalar,
    TK_بلاک_Scalar,
    TK_Alias,
    TK_Anchor,
    TK_نښان
  } ډول_2 = TK_Error;

  /// A string of length 0 or more whose begin() points to the logical location
  /// of the token in the input.
  StringRef Range;

  /// The value of a block scalar node.
  std::string ارزښت;

  Token() = افتراضي;
};

} // end namespace yaml
} // end namespace llvm

کارول Token_کتار_T = BumpPtrList<Token>;

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
بنية Simple_کلی {
  Token_کتار_T::iterator Tok;
  بینښه ستنه = 0;
  بینښه کرښه = 0;
  بینښه FlowLevel = 0;
  منطقي IsRequired = خطأ;

  منطقي چلوونکی ==(ثابت Simple_کلی &Other) {
    أرجع Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
کارول UTF8Decoded = std::pair<uint32_t, بینښه>;

ساكن UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator پای_ټکی_2 = Range.پای_ټکی();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  إذا (Position < پای_ټکی_2 && (*Position & 0x80) == 0) {
    أرجع {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  إذا (Position + 1 < پای_ټکی_2 && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    إذا (codepoint >= 0x80)
      أرجع {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  إذا (Position + 2 < پای_ټکی_2 && ((*Position & 0xF0) == 0xE0) &&
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
  إذا (Position + 3 < پای_ټکی_2 && ((*Position & 0xF8) == 0xF0) &&
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
  Scanner(StringRef ننوت, سرچینه_Mgr &SM, منطقي ShowColors = ریښتیا,
          std::error_کوډ *EC = تش_اشاره);
  Scanner(حافظه_بفر_Ref بفر, سرچینه_Mgr &SM_, منطقي ShowColors = ریښتیا,
          std::error_کوډ *EC = تش_اشاره);

  /// Parse the next token and return it without popping it.
  Token &peek_بل();

  /// Parse the next token and pop it from the queue.
  Token get_بل();

  فراغ printError(SMLoc Loc, سرچینه_Mgr::Diag_ډول ډول_2, ثابت Twine &پیغام_3,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.Print_پیغام(Loc, ډول_2, پیغام_3, Ranges, /* FixIts= */ {}, ShowColors);
  }

  فراغ سېټ_Error(ثابت Twine &پیغام_3, StringRef::iterator Position) {
    إذا (Position >= پای_ټکی_2)
      Position = پای_ټکی_2 - 1;

    // propagate the error if possible
    إذا (EC)
      *EC = make_error_کوډ(std::errc::invalid_دلیل);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    إذا (!Failed)
      printError(SMLoc::get_From_اشاره_ګر(Position), سرچینه_Mgr::DK_Error, پیغام_3);
    Failed = ریښتیا;
  }

  /// Returns true if an error occurred while parsing.
  منطقي failed() {
    أرجع Failed;
  }

خاص:
  فراغ init(حافظه_بفر_Ref بفر);

  StringRef current_ننوت() {
    أرجع StringRef(Current, پای_ټکی_2 - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    أرجع ::decodeUTF8(StringRef(Position, پای_ټکی_2 - Position));
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

  کارول SkipWhileFunc = StringRef::iterator (Scanner::*)(StringRef::iterator);

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
  فراغ لټون_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  منطقي consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  فراغ skip(uint32_t واټن_2);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  منطقي isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  منطقي is_Plain_خوندي_Non_Blank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  منطقي is_کرښه_تشه(StringRef کرښه);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  منطقي consume_کرښه_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  فراغ save_Simple_کلی_Candidate( Token_کتار_T::iterator Tok
                             , بینښه At_ستنه
                             , منطقي IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  فراغ remove_Stale_Simple_کلی_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  فراغ remove_Simple_کلی_Candidates_On_Flow_Level(بینښه Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  منطقي unrollIndent(صحيح منزل_ستنه);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  منطقي rollIndent( صحيح منزل_ستنه
                 , Token::Token_ډول ډول_2
                 , Token_کتار_T::iterator ورننباسه_ټکی);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  فراغ skipComment();

  /// Skip whitespace and comments until the start of the next token.
  فراغ لټون_منزل_بل_Token();

  /// Must be the first token generated.
  منطقي لټون_بهیر_پیل();

  /// Generate tokens needed to close out the stream.
  منطقي لټون_بهیر_پای_ټکی();

  /// Scan a %BLAH directive.
  منطقي لټون_Directive();

  /// Scan a ... or ---.
  منطقي لټون_Document_Indicator(منطقي Is_پیل);

  /// Scan a [ or { and generate the proper flow collection start token.
  منطقي لټون_Flow_Collection_پیل(منطقي IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  منطقي لټون_Flow_Collection_پای_ټکی(منطقي IsSequence);

  /// Scan the , that separates entries in a flow collection.
  منطقي لټون_Flow_ثبت();

  /// Scan the - that starts block sequence entries.
  منطقي لټون_بلاک_ثبت();

  /// Scan an explicit ? indicating a key.
  منطقي لټون_کلی();

  /// Scan an explicit : indicating a value.
  منطقي لټون_ارزښت();

  /// Scan a quoted scalar.
  منطقي لټون_Flow_Scalar(منطقي IsDoubleQuoted);

  /// Scan an unquoted scalar.
  منطقي لټون_Plain_Scalar();

  /// Scan an Alias or Anchor starting with * or &.
  منطقي لټون_Alias_Or_Anchor(منطقي IsAlias);

  /// Scan a block scalar starting with | or >.
  منطقي لټون_بلاک_Scalar(منطقي IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  منطقي لټون_بلاک_Scalar_Indicators(حرف &StyleIndicator, حرف &ChompingIndicator,
                                 بینښه &IndentIndicator, منطقي &Is_بشپړ);

  /// Scan a style indicator in a block scalar header.
  حرف لټون_بلاک_Style_Indicator();

  /// Scan a chomping indicator in a block scalar header.
  حرف لټون_بلاک_Chomping_Indicator();

  /// Scan an indentation indicator in a block scalar header.
  بینښه لټون_بلاک_Indentation_Indicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  منطقي لټون_بلاک_Scalar_سرلیک(حرف &ChompingIndicator, بینښه &IndentIndicator,
                             منطقي &Is_بشپړ);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  منطقي پیدا_کړه_بلاک_Scalar_Indent(بینښه &بلاک_Indent, بینښه بلاک_Exit_Indent,
                             بینښه &کرښه_Breaks, منطقي &Is_بشپړ);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  منطقي لټون_بلاک_Scalar_Indent(بینښه بلاک_Indent, بینښه بلاک_Exit_Indent,
                             منطقي &Is_بشپړ);

  /// Scan a tag of the form !stuff.
  منطقي لټون_نښان();

  /// Dispatch to the next scanning function based on \a *Cur.
  منطقي fetch_نور_نښې();

  /// The SourceMgr used for diagnostics and buffer management.
  سرچینه_Mgr &SM;

  /// The original input.
  حافظه_بفر_Ref ننوت_بفر;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator پای_ټکی_2;

  /// Current YAML indentation level in spaces.
  صحيح Indent;

  /// Current column number in Unicode code points.
  بینښه ستنه;

  /// Current line number.
  بینښه کرښه;

  /// How deep we are in flow style containers. 0 Means at block level.
  بینښه FlowLevel;

  /// Are we at the start of the stream?
  منطقي Is_پیل_د_بهیر;

  /// Can the next token be the start of a simple key?
  منطقي Is_Simple_کلی_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  منطقي Is_Adjacent_ارزښت_Allowed_دننه_Flow;

  /// True if an error has occurred.
  منطقي Failed;

  /// Should colors be used when printing out the diagnostic messages?
  منطقي ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  Token_کتار_T Token_کتار;

  /// Indentation levels.
  Small_ویکټور<صحيح, 4> Indents;

  /// Potential simple keys.
  Small_ویکټور<Simple_کلی, 4> Simple_کلیګانې;

  std::error_کوډ *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
ساكن فراغ encodeUTF8( uint32_t Unicode_Scalar_ارزښت
                      , Small_ویکټور_Impl<حرف> &پایله) {
  إذا (Unicode_Scalar_ارزښت <= 0x7F) {
    پایله.ټېل_شا(Unicode_Scalar_ارزښت & 0x7F);
  } وإلا إذا (Unicode_Scalar_ارزښت <= 0x7FF) {
    uint8_t لومړی_Byte = 0xC0 | ((Unicode_Scalar_ارزښت & 0x7C0) >> 6);
    uint8_t دوهم_Byte = 0x80 | (Unicode_Scalar_ارزښت & 0x3F);
    پایله.ټېل_شا(لومړی_Byte);
    پایله.ټېل_شا(دوهم_Byte);
  } وإلا إذا (Unicode_Scalar_ارزښت <= 0xFFFF) {
    uint8_t لومړی_Byte = 0xE0 | ((Unicode_Scalar_ارزښت & 0xF000) >> 12);
    uint8_t دوهم_Byte = 0x80 | ((Unicode_Scalar_ارزښت & 0xFC0) >> 6);
    uint8_t دریم_Byte = 0x80 | (Unicode_Scalar_ارزښت & 0x3F);
    پایله.ټېل_شا(لومړی_Byte);
    پایله.ټېل_شا(دوهم_Byte);
    پایله.ټېل_شا(دریم_Byte);
  } وإلا إذا (Unicode_Scalar_ارزښت <= 0x10FFFF) {
    uint8_t لومړی_Byte = 0xF0 | ((Unicode_Scalar_ارزښت & 0x1F0000) >> 18);
    uint8_t دوهم_Byte = 0x80 | ((Unicode_Scalar_ارزښت & 0x3F000) >> 12);
    uint8_t دریم_Byte = 0x80 | ((Unicode_Scalar_ارزښت & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_ارزښت & 0x3F);
    پایله.ټېل_شا(لومړی_Byte);
    پایله.ټېل_شا(دوهم_Byte);
    پایله.ټېل_شا(دریم_Byte);
    پایله.ټېل_شا(FourthByte);
  }
}

منطقي yaml::dump_نښې(StringRef ننوت, raw_ostream &OS) {
  سرچینه_Mgr SM;
  Scanner scanner(ننوت, SM);
  طالما (ریښتیا) {
    Token T = scanner.get_بل();
    بدل (T.ډول_2) {
    حالة Token::TK_بهیر_پیل:
      OS << "Stream-Start: ";
      اكسر;
    حالة Token::TK_بهیر_پای_ټکی:
      OS << "Stream-End: ";
      اكسر;
    حالة Token::TK_نسخه_Directive:
      OS << "Version-Directive: ";
      اكسر;
    حالة Token::TK_نښان_Directive:
      OS << "Tag-Directive: ";
      اكسر;
    حالة Token::TK_Document_پیل:
      OS << "Document-Start: ";
      اكسر;
    حالة Token::TK_Document_پای_ټکی:
      OS << "Document-End: ";
      اكسر;
    حالة Token::TK_بلاک_ثبت:
      OS << "Block-Entry: ";
      اكسر;
    حالة Token::TK_بلاک_پای_ټکی:
      OS << "Block-End: ";
      اكسر;
    حالة Token::TK_بلاک_Sequence_پیل:
      OS << "Block-Sequence-Start: ";
      اكسر;
    حالة Token::TK_بلاک_Mapping_پیل:
      OS << "Block-Mapping-Start: ";
      اكسر;
    حالة Token::TK_Flow_ثبت:
      OS << "Flow-Entry: ";
      اكسر;
    حالة Token::TK_Flow_Sequence_پیل:
      OS << "Flow-Sequence-Start: ";
      اكسر;
    حالة Token::TK_Flow_Sequence_پای_ټکی:
      OS << "Flow-Sequence-End: ";
      اكسر;
    حالة Token::TK_Flow_Mapping_پیل:
      OS << "Flow-Mapping-Start: ";
      اكسر;
    حالة Token::TK_Flow_Mapping_پای_ټکی:
      OS << "Flow-Mapping-End: ";
      اكسر;
    حالة Token::TK_کلی:
      OS << "Key: ";
      اكسر;
    حالة Token::TK_ارزښت:
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
    حالة Token::TK_نښان:
      OS << "Tag: ";
      اكسر;
    حالة Token::TK_Error:
      اكسر;
    }
    OS << T.Range << "\n";
    إذا (T.ډول_2 == Token::TK_بهیر_پای_ټکی)
      اكسر;
    وإلا إذا (T.ډول_2 == Token::TK_Error)
      أرجع خطأ;
  }
  أرجع ریښتیا;
}

منطقي yaml::لټون_نښې(StringRef ننوت) {
  سرچینه_Mgr SM;
  Scanner scanner(ننوت, SM);
  طالما (ریښتیا) {
    Token T = scanner.get_بل();
    إذا (T.ډول_2 == Token::TK_بهیر_پای_ټکی)
      اكسر;
    وإلا إذا (T.ډول_2 == Token::TK_Error)
      أرجع خطأ;
  }
  أرجع ریښتیا;
}

std::string yaml::escape(StringRef ننوت, منطقي EscapePrintable) {
  std::string Escaped_ننوت;
  لكل (StringRef::iterator i = ننوت.begin(), e = ننوت.پای_ټکی(); i != e; ++i) {
    إذا (*i == '\\')
      Escaped_ننوت += "\\\\";
    وإلا إذا (*i == '"')
      Escaped_ننوت += "\\\"";
    وإلا إذا (*i == 0)
      Escaped_ننوت += "\\0";
    وإلا إذا (*i == 0x07)
      Escaped_ننوت += "\\a";
    وإلا إذا (*i == 0x08)
      Escaped_ننوت += "\\b";
    وإلا إذا (*i == 0x09)
      Escaped_ننوت += "\\t";
    وإلا إذا (*i == 0x0A)
      Escaped_ننوت += "\\n";
    وإلا إذا (*i == 0x0B)
      Escaped_ننوت += "\\v";
    وإلا إذا (*i == 0x0C)
      Escaped_ننوت += "\\f";
    وإلا إذا (*i == 0x0D)
      Escaped_ننوت += "\\r";
    وإلا إذا (*i == 0x1B)
      Escaped_ننوت += "\\e";
    وإلا إذا ((بینښه حرف)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_ننوت += "\\x" + std::string(2 - HexStr.اندازه_2(), '0') + HexStr;
    } وإلا إذا (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_ارزښت
        = decodeUTF8(StringRef(i, ننوت.پای_ټکی() - i));
      إذا (Unicode_Scalar_ارزښت.دوهم_2 == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_ننوت, Val);
        // FIXME: Error reporting.
        أرجع Escaped_ننوت;
      }
      إذا (Unicode_Scalar_ارزښت.لومړی_2 == 0x85)
        Escaped_ننوت += "\\N";
      وإلا إذا (Unicode_Scalar_ارزښت.لومړی_2 == 0xA0)
        Escaped_ننوت += "\\_";
      وإلا إذا (Unicode_Scalar_ارزښت.لومړی_2 == 0x2028)
        Escaped_ننوت += "\\L";
      وإلا إذا (Unicode_Scalar_ارزښت.لومړی_2 == 0x2029)
        Escaped_ننوت += "\\P";
      وإلا إذا (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_ارزښت.لومړی_2))
        Escaped_ننوت += StringRef(i, Unicode_Scalar_ارزښت.دوهم_2);
      وإلا {
        std::string HexStr = utohexstr(Unicode_Scalar_ارزښت.لومړی_2);
        إذا (HexStr.اندازه_2() <= 2)
          Escaped_ننوت += "\\x" + std::string(2 - HexStr.اندازه_2(), '0') + HexStr;
        وإلا إذا (HexStr.اندازه_2() <= 4)
          Escaped_ننوت += "\\u" + std::string(4 - HexStr.اندازه_2(), '0') + HexStr;
        وإلا إذا (HexStr.اندازه_2() <= 8)
          Escaped_ننوت += "\\U" + std::string(8 - HexStr.اندازه_2(), '0') + HexStr;
      }
      i += Unicode_Scalar_ارزښت.دوهم_2 - 1;
    } وإلا {
      Escaped_ننوت.ټېل_شا(*i);
    }
  }
  أرجع Escaped_ننوت;
}

std::optional<منطقي> yaml::parseBool(StringRef S) {
  بدل (S.اندازه_2()) {
  حالة 1:
    بدل (S.front()) {
    حالة 'y':
    حالة 'Y':
      أرجع ریښتیا;
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
        أرجع ریښتیا;
      [[fallthrough]];
    حالة 'o':
      إذا (S[1] == 'n') //[Oo]n
        أرجع ریښتیا;
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
        أرجع ریښتیا;
      [[fallthrough]];
    حالة 'y':
      إذا (S.drop_front() == "es") //[Yy]es
        أرجع ریښتیا;
      أرجع std::nullopt;
    افتراضي:
      أرجع std::nullopt;
    }
  حالة 4:
    بدل (S.front()) {
    حالة 'T':
      إذا (S.drop_front() == "RUE") // TRUE
        أرجع ریښتیا;
      [[fallthrough]];
    حالة 't':
      إذا (S.drop_front() == "rue") //[Tt]rue
        أرجع ریښتیا;
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

Scanner::Scanner(StringRef ننوت, سرچینه_Mgr &sm, منطقي ShowColors,
                 std::error_کوډ *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(حافظه_بفر_Ref(ننوت, "YAML"));
}

Scanner::Scanner(حافظه_بفر_Ref بفر, سرچینه_Mgr &SM_, منطقي ShowColors,
                 std::error_کوډ *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(بفر);
}

فراغ Scanner::init(حافظه_بفر_Ref بفر) {
  ننوت_بفر = بفر;
  Current = ننوت_بفر.get_بفر_پیل();
  پای_ټکی_2 = ننوت_بفر.get_بفر_پای_ټکی();
  Indent = -1;
  ستنه = 0;
  کرښه = 0;
  FlowLevel = 0;
  Is_پیل_د_بهیر = ریښتیا;
  Is_Simple_کلی_Allowed = ریښتیا;
  Is_Adjacent_ارزښت_Allowed_دننه_Flow = خطأ;
  Failed = خطأ;
  std::unique_ptr<حافظه_بفر> ننوت_بفر_Owner =
      حافظه_بفر::get_Mem_بفر(بفر, /*RequiresNullTerminator=*/خطأ);
  SM.جمع_کړه_New_سرچینه_بفر(std::move(ننوت_بفر_Owner), SMLoc());
}

Token &Scanner::peek_بل() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  منطقي Need_نور = خطأ;
  طالما (ریښتیا) {
    إذا (Token_کتار.تشه() || Need_نور) {
      إذا (!fetch_نور_نښې()) {
        Token_کتار.clear();
        Simple_کلیګانې.clear();
        Token_کتار.ټېل_شا(Token());
        أرجع Token_کتار.front();
      }
    }
    أكد(!Token_کتار.تشه() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_کلی_Candidates();
    Simple_کلی SK;
    SK.Tok = Token_کتار.begin();
    إذا (!is_contained(Simple_کلیګانې, SK))
      اكسر;
    وإلا
      Need_نور = ریښتیا;
  }
  أرجع Token_کتار.front();
}

Token Scanner::get_بل() {
  Token Ret = peek_بل();
  // TokenQueue can be empty if there was an error getting the next token.
  إذا (!Token_کتار.تشه())
    Token_کتار.راوباسه_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  إذا (Token_کتار.تشه())
    Token_کتار.resetAlloc();

  أرجع Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  إذا (Position == پای_ټکی_2)
    أرجع Position;
  // Check 7 bit c-printable - b-char.
  إذا (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    أرجع Position + 1;

  // Check for valid UTF-8.
  إذا (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    إذا (   u8d.دوهم_2 != 0
        && u8d.لومړی_2 != 0xFEFF
        && ( u8d.لومړی_2 == 0x85
          || ( u8d.لومړی_2 >= 0xA0
            && u8d.لومړی_2 <= 0xD7FF)
          || ( u8d.لومړی_2 >= 0xE000
            && u8d.لومړی_2 <= 0xFFFD)
          || ( u8d.لومړی_2 >= 0x10000
            && u8d.لومړی_2 <= 0x10FFFF)))
      أرجع Position + u8d.دوهم_2;
  }
  أرجع Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  إذا (Position == پای_ټکی_2)
    أرجع Position;
  إذا (*Position == 0x0D) {
    إذا (Position + 1 != پای_ټکی_2 && *(Position + 1) == 0x0A)
      أرجع Position + 2;
    أرجع Position + 1;
  }

  إذا (*Position == 0x0A)
    أرجع Position + 1;
  أرجع Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  إذا (Position == پای_ټکی_2)
    أرجع Position;
  إذا (*Position == ' ')
    أرجع Position + 1;
  أرجع Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  إذا (Position == پای_ټکی_2)
    أرجع Position;
  إذا (*Position == ' ' || *Position == '\t')
    أرجع Position + 1;
  أرجع Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  إذا (Position == پای_ټکی_2)
    أرجع Position;
  إذا (*Position == ' ' || *Position == '\t')
    أرجع Position;
  أرجع skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  طالما (ریښتیا) {
    StringRef::iterator i = (هذا->*Func)(Position);
    إذا (i == Position)
      اكسر;
    Position = i;
  }
  أرجع Position;
}

فراغ Scanner::advanceWhile(SkipWhileFunc Func) {
  خپلکاری Final = skip_while(Func, Current);
  ستنه += Final - Current;
  Current = Final;
}

ساكن منطقي is_ns_hex_digit(ثابت حرف C) { أرجع isAlnum(C); }

ساكن منطقي is_ns_کلمه_char(ثابت حرف C) { أرجع C == '-' || isAlpha(C); }

فراغ Scanner::لټون_ns_uri_char() {
  طالما (ریښتیا) {
    إذا (Current == پای_ټکی_2)
      اكسر;
    إذا ((*Current == '%' && Current + 2 < پای_ټکی_2 &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_کلمه_char(*Current) ||
        StringRef(Current, 1).پیدا_کړه_لومړی_د("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++ستنه;
    } وإلا {
      اكسر;
    }
  }
}

منطقي Scanner::consume(uint32_t Expected) {
  إذا (Expected >= 0x80) {
    سېټ_Error("Cannot consume non-ascii characters", Current);
    أرجع خطأ;
  }
  إذا (Current == پای_ټکی_2)
    أرجع خطأ;
  إذا (uint8_t(*Current) >= 0x80) {
    سېټ_Error("Cannot consume non-ascii characters", Current);
    أرجع خطأ;
  }
  إذا (uint8_t(*Current) == Expected) {
    ++Current;
    ++ستنه;
    أرجع ریښتیا;
  }
  أرجع خطأ;
}

فراغ Scanner::skip(uint32_t واټن_2) {
  Current += واټن_2;
  ستنه += واټن_2;
  أكد(Current <= پای_ټکی_2 && "Skipped past the end");
}

منطقي Scanner::isBlankOrBreak(StringRef::iterator Position) {
  إذا (Position == پای_ټکی_2)
    أرجع خطأ;
  أرجع *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

منطقي Scanner::is_Plain_خوندي_Non_Blank(StringRef::iterator Position) {
  إذا (Position == پای_ټکی_2 || isBlankOrBreak(Position))
    أرجع خطأ;
  إذا (FlowLevel &&
      StringRef(Position, 1).پیدا_کړه_لومړی_د(",[]{}") != StringRef::npos)
    أرجع خطأ;
  أرجع ریښتیا;
}

منطقي Scanner::is_کرښه_تشه(StringRef کرښه) {
  لكل (ثابت خپلکاری *Position = کرښه.begin(); Position != کرښه.پای_ټکی(); ++Position)
    إذا (!isBlankOrBreak(Position))
      أرجع خطأ;
  أرجع ریښتیا;
}

منطقي Scanner::consume_کرښه_Break_If_Present() {
  خپلکاری بل_2 = skip_b_break(Current);
  إذا (بل_2 == Current)
    أرجع خطأ;
  ستنه = 0;
  ++کرښه;
  Current = بل_2;
  أرجع ریښتیا;
}

فراغ Scanner::save_Simple_کلی_Candidate( Token_کتار_T::iterator Tok
                                    , بینښه At_ستنه
                                    , منطقي IsRequired) {
  إذا (Is_Simple_کلی_Allowed) {
    Simple_کلی SK;
    SK.Tok = Tok;
    SK.کرښه = کرښه;
    SK.ستنه = At_ستنه;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_کلیګانې.ټېل_شا(SK);
  }
}

فراغ Scanner::remove_Stale_Simple_کلی_Candidates() {
  لكل (Small_ویکټور_Impl<Simple_کلی>::iterator i = Simple_کلیګانې.begin();
                                            i != Simple_کلیګانې.پای_ټکی();) {
    إذا (i->کرښه != کرښه || i->ستنه + 1024 < ستنه) {
      إذا (i->IsRequired)
        سېټ_Error( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_کلیګانې.erase(i);
    } وإلا {
      ++i;
    }
  }
}

فراغ Scanner::remove_Simple_کلی_Candidates_On_Flow_Level(بینښه Level) {
  إذا (!Simple_کلیګانې.تشه() && (Simple_کلیګانې.پای_ټکی() - 1)->FlowLevel == Level)
    Simple_کلیګانې.راوباسه_شا();
}

منطقي Scanner::unrollIndent(صحيح منزل_ستنه) {
  Token T;
  // Indentation is ignored in flow.
  إذا (FlowLevel != 0)
    أرجع ریښتیا;

  طالما (Indent > منزل_ستنه) {
    T.ډول_2 = Token::TK_بلاک_پای_ټکی;
    T.Range = StringRef(Current, 1);
    Token_کتار.ټېل_شا(T);
    Indent = Indents.راوباسه_شا_val();
  }

  أرجع ریښتیا;
}

منطقي Scanner::rollIndent( صحيح منزل_ستنه
                        , Token::Token_ډول ډول_2
                        , Token_کتار_T::iterator ورننباسه_ټکی) {
  إذا (FlowLevel)
    أرجع ریښتیا;
  إذا (Indent < منزل_ستنه) {
    Indents.ټېل_شا(Indent);
    Indent = منزل_ستنه;

    Token T;
    T.ډول_2 = ډول_2;
    T.Range = StringRef(Current, 0);
    Token_کتار.ورننباسه(ورننباسه_ټکی, T);
  }
  أرجع ریښتیا;
}

فراغ Scanner::skipComment() {
  إذا (Current == پای_ټکی_2 || *Current != '#')
    أرجع;
  طالما (ریښتیا) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    إذا (I == Current)
      اكسر;
    Current = I;
    ++ستنه;
  }
}

فراغ Scanner::لټون_منزل_بل_Token() {
  طالما (ریښتیا) {
    طالما (Current != پای_ټکی_2 && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    إذا (i == Current)
      اكسر;
    Current = i;
    ++کرښه;
    ستنه = 0;
    // New lines may start a simple key.
    إذا (!FlowLevel)
      Is_Simple_کلی_Allowed = ریښتیا;
  }
}

منطقي Scanner::لټون_بهیر_پیل() {
  Is_پیل_د_بهیر = خطأ;

  EncodingInfo EI = getUnicodeEncoding(current_ننوت());

  Token T;
  T.ډول_2 = Token::TK_بهیر_پیل;
  T.Range = StringRef(Current, EI.دوهم_2);
  Token_کتار.ټېل_شا(T);
  Current += EI.دوهم_2;
  أرجع ریښتیا;
}

منطقي Scanner::لټون_بهیر_پای_ټکی() {
  // Force an ending new line if one isn't present.
  إذا (ستنه != 0) {
    ستنه = 0;
    ++کرښه;
  }

  unrollIndent(-1);
  Simple_کلیګانې.clear();
  Is_Simple_کلی_Allowed = خطأ;
  Is_Adjacent_ارزښت_Allowed_دننه_Flow = خطأ;

  Token T;
  T.ډول_2 = Token::TK_بهیر_پای_ټکی;
  T.Range = StringRef(Current, 0);
  Token_کتار.ټېل_شا(T);
  أرجع ریښتیا;
}

منطقي Scanner::لټون_Directive() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_کلیګانې.clear();
  Is_Simple_کلی_Allowed = خطأ;
  Is_Adjacent_ارزښت_Allowed_دننه_Flow = خطأ;

  StringRef::iterator پیل = Current;
  consume('%');
  StringRef::iterator Name_پیل = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(Name_پیل, Current - Name_پیل);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  إذا (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.ډول_2 = Token::TK_نسخه_Directive;
    T.Range = StringRef(پیل, Current - پیل);
    Token_کتار.ټېل_شا(T);
    أرجع ریښتیا;
  } وإلا إذا(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.ډول_2 = Token::TK_نښان_Directive;
    T.Range = StringRef(پیل, Current - پیل);
    Token_کتار.ټېل_شا(T);
    أرجع ریښتیا;
  }
  أرجع خطأ;
}

منطقي Scanner::لټون_Document_Indicator(منطقي Is_پیل) {
  unrollIndent(-1);
  Simple_کلیګانې.clear();
  Is_Simple_کلی_Allowed = خطأ;
  Is_Adjacent_ارزښت_Allowed_دننه_Flow = خطأ;

  Token T;
  T.ډول_2 = Is_پیل ? Token::TK_Document_پیل : Token::TK_Document_پای_ټکی;
  T.Range = StringRef(Current, 3);
  skip(3);
  Token_کتار.ټېل_شا(T);
  أرجع ریښتیا;
}

منطقي Scanner::لټون_Flow_Collection_پیل(منطقي IsSequence) {
  Token T;
  T.ډول_2 = IsSequence ? Token::TK_Flow_Sequence_پیل
                      : Token::TK_Flow_Mapping_پیل;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_کتار.ټېل_شا(T);

  // [ and { may begin a simple key.
  save_Simple_کلی_Candidate(--Token_کتار.پای_ټکی(), ستنه - 1, خطأ);

  // And may also be followed by a simple key.
  Is_Simple_کلی_Allowed = ریښتیا;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_ارزښت_Allowed_دننه_Flow = خطأ;
  ++FlowLevel;
  أرجع ریښتیا;
}

منطقي Scanner::لټون_Flow_Collection_پای_ټکی(منطقي IsSequence) {
  remove_Simple_کلی_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلی_Allowed = خطأ;
  Is_Adjacent_ارزښت_Allowed_دننه_Flow = ریښتیا;
  Token T;
  T.ډول_2 = IsSequence ? Token::TK_Flow_Sequence_پای_ټکی
                      : Token::TK_Flow_Mapping_پای_ټکی;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_کتار.ټېل_شا(T);
  إذا (FlowLevel)
    --FlowLevel;
  أرجع ریښتیا;
}

منطقي Scanner::لټون_Flow_ثبت() {
  remove_Simple_کلی_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلی_Allowed = ریښتیا;
  Is_Adjacent_ارزښت_Allowed_دننه_Flow = خطأ;
  Token T;
  T.ډول_2 = Token::TK_Flow_ثبت;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_کتار.ټېل_شا(T);
  أرجع ریښتیا;
}

منطقي Scanner::لټون_بلاک_ثبت() {
  rollIndent(ستنه, Token::TK_بلاک_Sequence_پیل, Token_کتار.پای_ټکی());
  remove_Simple_کلی_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلی_Allowed = ریښتیا;
  Is_Adjacent_ارزښت_Allowed_دننه_Flow = خطأ;
  Token T;
  T.ډول_2 = Token::TK_بلاک_ثبت;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_کتار.ټېل_شا(T);
  أرجع ریښتیا;
}

منطقي Scanner::لټون_کلی() {
  إذا (!FlowLevel)
    rollIndent(ستنه, Token::TK_بلاک_Mapping_پیل, Token_کتار.پای_ټکی());

  remove_Simple_کلی_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلی_Allowed = !FlowLevel;
  Is_Adjacent_ارزښت_Allowed_دننه_Flow = خطأ;

  Token T;
  T.ډول_2 = Token::TK_کلی;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_کتار.ټېل_شا(T);
  أرجع ریښتیا;
}

منطقي Scanner::لټون_ارزښت() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  إذا (!Simple_کلیګانې.تشه()) {
    Simple_کلی SK = Simple_کلیګانې.راوباسه_شا_val();
    Token T;
    T.ډول_2 = Token::TK_کلی;
    T.Range = SK.Tok->Range;
    Token_کتار_T::iterator i, e;
    لكل (i = Token_کتار.begin(), e = Token_کتار.پای_ټکی(); i != e; ++i) {
      إذا (i == SK.Tok)
        اكسر;
    }
    إذا (i == e) {
      Failed = ریښتیا;
      أرجع خطأ;
    }
    i = Token_کتار.ورننباسه(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.ستنه, Token::TK_بلاک_Mapping_پیل, i);

    Is_Simple_کلی_Allowed = خطأ;
  } وإلا {
    إذا (!FlowLevel)
      rollIndent(ستنه, Token::TK_بلاک_Mapping_پیل, Token_کتار.پای_ټکی());
    Is_Simple_کلی_Allowed = !FlowLevel;
  }
  Is_Adjacent_ارزښت_Allowed_دننه_Flow = خطأ;

  Token T;
  T.ډول_2 = Token::TK_ارزښت;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_کتار.ټېل_شا(T);
  أرجع ریښتیا;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE ساكن منطقي
wasEscaped(StringRef::iterator لومړی, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
ساكن منطقي wasEscaped(StringRef::iterator لومړی,
                       StringRef::iterator Position) {
  أكد(Position - 1 >= لومړی);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  طالما (I >= لومړی && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  أرجع (Position - 1 - I) % 2 == 1;
}

منطقي Scanner::لټون_Flow_Scalar(منطقي IsDoubleQuoted) {
  StringRef::iterator پیل = Current;
  بینښه ستون_پیل = ستنه;
  إذا (IsDoubleQuoted) {
    افعل {
      ++Current;
      طالما (Current != پای_ټکی_2 && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } طالما (   Current != پای_ټکی_2
             && *(Current - 1) == '\\'
             && wasEscaped(پیل + 1, Current));
  } وإلا {
    skip(1);
    طالما (Current != پای_ټکی_2) {
      // Skip a ' followed by another '.
      إذا (Current + 1 < پای_ټکی_2 && *Current == '\'' && *(Current + 1) == '\'') {
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
        ستنه = 0;
        ++کرښه;
      } وإلا {
        إذا (i == پای_ټکی_2)
          اكسر;
        Current = i;
        ++ستنه;
      }
    }
  }

  إذا (Current == پای_ټکی_2) {
    سېټ_Error("Expected quote at end of scalar", Current);
    أرجع خطأ;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.ډول_2 = Token::TK_Scalar;
  T.Range = StringRef(پیل, Current - پیل);
  Token_کتار.ټېل_شا(T);

  save_Simple_کلی_Candidate(--Token_کتار.پای_ټکی(), ستون_پیل, خطأ);

  Is_Simple_کلی_Allowed = خطأ;
  Is_Adjacent_ارزښت_Allowed_دننه_Flow = ریښتیا;

  أرجع ریښتیا;
}

منطقي Scanner::لټون_Plain_Scalar() {
  StringRef::iterator پیل = Current;
  بینښه ستون_پیل = ستنه;
  بینښه LeadingBlanks = 0;
  أكد(Indent >= -1 && "Indent must be >= -1 !");
  بینښه indent = static_cast<بینښه>(Indent + 1);
  طالما (Current != پای_ټکی_2) {
    إذا (*Current == '#')
      اكسر;

    طالما (Current != پای_ټکی_2 &&
           ((*Current != ':' && is_Plain_خوندي_Non_Blank(Current)) ||
            (*Current == ':' && is_Plain_خوندي_Non_Blank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      إذا (i == Current)
        اكسر;
      Current = i;
      ++ستنه;
    }

    // Are we at the end?
    إذا (!isBlankOrBreak(Current))
      اكسر;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    طالما (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      إذا (i != Tmp) {
        إذا (LeadingBlanks && (ستنه < indent) && *Tmp == '\t') {
          سېټ_Error("Found invalid tab character in indentation", Tmp);
          أرجع خطأ;
        }
        Tmp = i;
        ++ستنه;
      } وإلا {
        i = skip_b_break(Tmp);
        إذا (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        ستنه = 0;
        ++کرښه;
      }
    }

    إذا (!FlowLevel && ستنه < indent)
      اكسر;

    Current = Tmp;
  }
  إذا (پیل == Current) {
    سېټ_Error("Got empty plain scalar", پیل);
    أرجع خطأ;
  }
  Token T;
  T.ډول_2 = Token::TK_Scalar;
  T.Range = StringRef(پیل, Current - پیل);
  Token_کتار.ټېل_شا(T);

  // Plain scalars can be simple keys.
  save_Simple_کلی_Candidate(--Token_کتار.پای_ټکی(), ستون_پیل, خطأ);

  Is_Simple_کلی_Allowed = خطأ;
  Is_Adjacent_ارزښت_Allowed_دننه_Flow = خطأ;

  أرجع ریښتیا;
}

منطقي Scanner::لټون_Alias_Or_Anchor(منطقي IsAlias) {
  StringRef::iterator پیل = Current;
  بینښه ستون_پیل = ستنه;
  skip(1);
  طالما (Current != پای_ټکی_2) {
    إذا (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      اكسر;
    StringRef::iterator i = skip_ns_char(Current);
    إذا (i == Current)
      اكسر;
    Current = i;
    ++ستنه;
  }

  إذا (پیل + 1 == Current) {
    سېټ_Error("Got empty alias or anchor", پیل);
    أرجع خطأ;
  }

  Token T;
  T.ډول_2 = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(پیل, Current - پیل);
  Token_کتار.ټېل_شا(T);

  // Alias and anchors can be simple keys.
  save_Simple_کلی_Candidate(--Token_کتار.پای_ټکی(), ستون_پیل, خطأ);

  Is_Simple_کلی_Allowed = خطأ;
  Is_Adjacent_ارزښت_Allowed_دننه_Flow = خطأ;

  أرجع ریښتیا;
}

منطقي Scanner::لټون_بلاک_Scalar_Indicators(حرف &StyleIndicator,
                                        حرف &ChompingIndicator,
                                        بینښه &IndentIndicator,
                                        منطقي &Is_بشپړ) {
  StyleIndicator = لټون_بلاک_Style_Indicator();
  إذا (!لټون_بلاک_Scalar_سرلیک(ChompingIndicator, IndentIndicator, Is_بشپړ))
    أرجع خطأ;
  أرجع ریښتیا;
}

حرف Scanner::لټون_بلاک_Style_Indicator() {
  حرف Indicator = ' ';
  إذا (Current != پای_ټکی_2 && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  أرجع Indicator;
}

حرف Scanner::لټون_بلاک_Chomping_Indicator() {
  حرف Indicator = ' ';
  إذا (Current != پای_ټکی_2 && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  أرجع Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
ساكن بینښه get_Chomped_کرښه_Breaks(حرف ChompingIndicator,
                                     بینښه کرښه_Breaks, StringRef Str) {
  إذا (ChompingIndicator == '-') // Strip all line breaks.
    أرجع 0;
  إذا (ChompingIndicator == '+') // Keep all line breaks.
    أرجع کرښه_Breaks;
  // Clip trailing lines.
  أرجع Str.تشه() ? 0 : 1;
}

بینښه Scanner::لټون_بلاک_Indentation_Indicator() {
  بینښه Indent = 0;
  إذا (Current != پای_ټکی_2 && (*Current >= '1' && *Current <= '9')) {
    Indent = بینښه(*Current - '0');
    skip(1);
  }
  أرجع Indent;
}

منطقي Scanner::لټون_بلاک_Scalar_سرلیک(حرف &ChompingIndicator,
                                    بینښه &IndentIndicator, منطقي &Is_بشپړ) {
  خپلکاری پیل = Current;

  ChompingIndicator = لټون_بلاک_Chomping_Indicator();
  IndentIndicator = لټون_بلاک_Indentation_Indicator();
  // Check for the chomping indicator once again.
  إذا (ChompingIndicator == ' ')
    ChompingIndicator = لټون_بلاک_Chomping_Indicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  إذا (Current == پای_ټکی_2) { // EOF, we have an empty scalar.
    Token T;
    T.ډول_2 = Token::TK_بلاک_Scalar;
    T.Range = StringRef(پیل, Current - پیل);
    Token_کتار.ټېل_شا(T);
    Is_بشپړ = ریښتیا;
    أرجع ریښتیا;
  }

  إذا (!consume_کرښه_Break_If_Present()) {
    سېټ_Error("Expected a line break after block scalar header", Current);
    أرجع خطأ;
  }
  أرجع ریښتیا;
}

منطقي Scanner::پیدا_کړه_بلاک_Scalar_Indent(بینښه &بلاک_Indent,
                                    بینښه بلاک_Exit_Indent,
                                    بینښه &کرښه_Breaks, منطقي &Is_بشپړ) {
  بینښه Max_All_Space_کرښه_Characters = 0;
  StringRef::iterator Longest_All_Space_کرښه;

  طالما (ریښتیا) {
    advanceWhile(&Scanner::skip_s_space);
    إذا (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      إذا (ستنه <= بلاک_Exit_Indent) { // End of the block literal.
        Is_بشپړ = ریښتیا;
        أرجع ریښتیا;
      }
      // We found the block's indentation.
      بلاک_Indent = ستنه;
      إذا (Max_All_Space_کرښه_Characters > بلاک_Indent) {
        سېټ_Error(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_کرښه);
        أرجع خطأ;
      }
      أرجع ریښتیا;
    }
    إذا (skip_b_break(Current) != Current &&
        ستنه > Max_All_Space_کرښه_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_کرښه_Characters = ستنه;
      Longest_All_Space_کرښه = Current;
    }

    // Check for EOF.
    إذا (Current == پای_ټکی_2) {
      Is_بشپړ = ریښتیا;
      أرجع ریښتیا;
    }

    إذا (!consume_کرښه_Break_If_Present()) {
      Is_بشپړ = ریښتیا;
      أرجع ریښتیا;
    }
    ++کرښه_Breaks;
  }
  أرجع ریښتیا;
}

منطقي Scanner::لټون_بلاک_Scalar_Indent(بینښه بلاک_Indent,
                                    بینښه بلاک_Exit_Indent, منطقي &Is_بشپړ) {
  // Skip the indentation.
  طالما (ستنه < بلاک_Indent) {
    خپلکاری I = skip_s_space(Current);
    إذا (I == Current)
      اكسر;
    Current = I;
    ++ستنه;
  }

  إذا (skip_nb_char(Current) == Current)
    أرجع ریښتیا;

  إذا (ستنه <= بلاک_Exit_Indent) { // End of the block literal.
    Is_بشپړ = ریښتیا;
    أرجع ریښتیا;
  }

  إذا (ستنه < بلاک_Indent) {
    إذا (Current != پای_ټکی_2 && *Current == '#') { // Trailing comment.
      Is_بشپړ = ریښتیا;
      أرجع ریښتیا;
    }
    سېټ_Error("A text line is less indented than the block scalar", Current);
    أرجع خطأ;
  }
  أرجع ریښتیا; // A normal text line.
}

منطقي Scanner::لټون_بلاک_Scalar(منطقي IsLiteral) {
  أكد(*Current == '|' || *Current == '>');
  حرف StyleIndicator;
  حرف ChompingIndicator;
  بینښه بلاک_Indent;
  منطقي Is_بشپړ = خطأ;
  إذا (!لټون_بلاک_Scalar_Indicators(StyleIndicator, ChompingIndicator, بلاک_Indent,
                                 Is_بشپړ))
    أرجع خطأ;
  إذا (Is_بشپړ)
    أرجع ریښتیا;
  منطقي IsFolded = StyleIndicator == '>';

  ثابت خپلکاری *پیل = Current;
  بینښه بلاک_Exit_Indent = Indent < 0 ? 0 : (بینښه)Indent;
  بینښه کرښه_Breaks = 0;
  إذا (بلاک_Indent == 0) {
    إذا (!پیدا_کړه_بلاک_Scalar_Indent(بلاک_Indent, بلاک_Exit_Indent, کرښه_Breaks,
                               Is_بشپړ))
      أرجع خطأ;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  طالما (!Is_بشپړ) {
    إذا (!لټون_بلاک_Scalar_Indent(بلاک_Indent, بلاک_Exit_Indent, Is_بشپړ))
      أرجع خطأ;
    إذا (Is_بشپړ)
      اكسر;

    // Parse the current line.
    خپلکاری کرښه_پیل = Current;
    advanceWhile(&Scanner::skip_nb_char);
    إذا (کرښه_پیل != Current) {
      إذا (کرښه_Breaks && IsFolded && !Scanner::is_کرښه_تشه(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        إذا (کرښه_Breaks == 1) {
          Str.append(کرښه_Breaks,
                     is_کرښه_تشه(StringRef(کرښه_پیل, Current - کرښه_پیل))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        کرښه_Breaks--;
      }
      Str.append(کرښه_Breaks, '\n');
      Str.append(StringRef(کرښه_پیل, Current - کرښه_پیل));
      کرښه_Breaks = 0;
    }

    // Check for EOF.
    إذا (Current == پای_ټکی_2)
      اكسر;

    إذا (!consume_کرښه_Break_If_Present())
      اكسر;
    ++کرښه_Breaks;
  }

  إذا (Current == پای_ټکی_2 && !کرښه_Breaks)
    // Ensure that there is at least one line break before the end of file.
    کرښه_Breaks = 1;
  Str.append(get_Chomped_کرښه_Breaks(ChompingIndicator, کرښه_Breaks, Str), '\n');

  // New lines may start a simple key.
  إذا (!FlowLevel)
    Is_Simple_کلی_Allowed = ریښتیا;
  Is_Adjacent_ارزښت_Allowed_دننه_Flow = خطأ;

  Token T;
  T.ډول_2 = Token::TK_بلاک_Scalar;
  T.Range = StringRef(پیل, Current - پیل);
  T.ارزښت = std::string(Str);
  Token_کتار.ټېل_شا(T);
  أرجع ریښتیا;
}

منطقي Scanner::لټون_نښان() {
  StringRef::iterator پیل = Current;
  بینښه ستون_پیل = ستنه;
  skip(1); // Eat !.
  إذا (Current == پای_ټکی_2 || isBlankOrBreak(Current)); // An empty tag.
  وإلا إذا (*Current == '<') {
    skip(1);
    لټون_ns_uri_char();
    إذا (!consume('>'))
      أرجع خطأ;
  } وإلا {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.ډول_2 = Token::TK_نښان;
  T.Range = StringRef(پیل, Current - پیل);
  Token_کتار.ټېل_شا(T);

  // Tags can be simple keys.
  save_Simple_کلی_Candidate(--Token_کتار.پای_ټکی(), ستون_پیل, خطأ);

  Is_Simple_کلی_Allowed = خطأ;
  Is_Adjacent_ارزښت_Allowed_دننه_Flow = خطأ;

  أرجع ریښتیا;
}

منطقي Scanner::fetch_نور_نښې() {
  إذا (Is_پیل_د_بهیر)
    أرجع لټون_بهیر_پیل();

  لټون_منزل_بل_Token();

  إذا (Current == پای_ټکی_2)
    أرجع لټون_بهیر_پای_ټکی();

  remove_Stale_Simple_کلی_Candidates();

  unrollIndent(ستنه);

  إذا (ستنه == 0 && *Current == '%')
    أرجع لټون_Directive();

  إذا (ستنه == 0 && Current + 3 <= پای_ټکی_2
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == پای_ټکی_2 || isBlankOrBreak(Current + 3)))
    أرجع لټون_Document_Indicator(ریښتیا);

  إذا (ستنه == 0 && Current + 3 <= پای_ټکی_2
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == پای_ټکی_2 || isBlankOrBreak(Current + 3)))
    أرجع لټون_Document_Indicator(خطأ);

  إذا (*Current == '[')
    أرجع لټون_Flow_Collection_پیل(ریښتیا);

  إذا (*Current == '{')
    أرجع لټون_Flow_Collection_پیل(خطأ);

  إذا (*Current == ']')
    أرجع لټون_Flow_Collection_پای_ټکی(ریښتیا);

  إذا (*Current == '}')
    أرجع لټون_Flow_Collection_پای_ټکی(خطأ);

  إذا (*Current == ',')
    أرجع لټون_Flow_ثبت();

  إذا (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == پای_ټکی_2))
    أرجع لټون_بلاک_ثبت();

  إذا (*Current == '?' && (Current + 1 == پای_ټکی_2 || isBlankOrBreak(Current + 1)))
    أرجع لټون_کلی();

  إذا (*Current == ':' &&
      (!is_Plain_خوندي_Non_Blank(Current + 1) || Is_Adjacent_ارزښت_Allowed_دننه_Flow))
    أرجع لټون_ارزښت();

  إذا (*Current == '*')
    أرجع لټون_Alias_Or_Anchor(ریښتیا);

  إذا (*Current == '&')
    أرجع لټون_Alias_Or_Anchor(خطأ);

  إذا (*Current == '!')
    أرجع لټون_نښان();

  إذا (*Current == '|' && !FlowLevel)
    أرجع لټون_بلاک_Scalar(ریښتیا);

  إذا (*Current == '>' && !FlowLevel)
    أرجع لټون_بلاک_Scalar(خطأ);

  إذا (*Current == '\'')
    أرجع لټون_Flow_Scalar(خطأ);

  إذا (*Current == '"')
    أرجع لټون_Flow_Scalar(ریښتیا);

  // Get a plain scalar.
  StringRef لومړی_Char(Current, 1);
  إذا ((!isBlankOrBreak(Current) &&
       لومړی_Char.پیدا_کړه_لومړی_د("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (لومړی_Char.پیدا_کړه_لومړی_د("?:-") != StringRef::npos &&
       is_Plain_خوندي_Non_Blank(Current + 1)))
    أرجع لټون_Plain_Scalar();

  سېټ_Error("Unrecognized character while tokenizing.", Current);
  أرجع خطأ;
}

بهیر::بهیر(StringRef ننوت, سرچینه_Mgr &SM, منطقي ShowColors,
               std::error_کوډ *EC)
    : scanner(جديد Scanner(ننوت, SM, ShowColors, EC)) {}

بهیر::بهیر(حافظه_بفر_Ref ننوت_بفر, سرچینه_Mgr &SM, منطقي ShowColors,
               std::error_کوډ *EC)
    : scanner(جديد Scanner(ننوت_بفر, SM, ShowColors, EC)) {}

بهیر::~بهیر() = افتراضي;

منطقي بهیر::failed() { أرجع scanner->failed(); }

فراغ بهیر::printError(غوټه *N, ثابت Twine &پیغام_4, سرچینه_Mgr::Diag_ډول ډول_2) {
  printError(N ? N->get_سرچینه_Range() : SMRange(), پیغام_4, ډول_2);
}

فراغ بهیر::printError(ثابت SMRange &Range, ثابت Twine &پیغام_4,
                        سرچینه_Mgr::Diag_ډول ډول_2) {
  scanner->printError(Range.پیل, ډول_2, پیغام_4, Range);
}

document_iterator بهیر::begin() {
  إذا (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->get_بل();

  CurrentDoc.reset(جديد Document(*هذا));
  أرجع document_iterator(CurrentDoc);
}

document_iterator بهیر::پای_ټکی() {
  أرجع document_iterator();
}

فراغ بهیر::skip() {
  لكل (Document &Doc : *هذا)
    Doc.skip();
}

غوټه::غوټه(بینښه صحيح Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), نښان(T) {
  SMLoc پیل = SMLoc::get_From_اشاره_ګر(peek_بل().Range.begin());
  سرچینه_Range = SMRange(پیل, پیل);
}

std::string غوټه::get_Verbatim_نښان() ثابت {
  StringRef Raw = get_Raw_نښان();
  إذا (!Raw.تشه() && Raw != "!") {
    std::string Ret;
    إذا (Raw.پیدا_کړه_وروستی_د('!') == 0) {
      Ret = std::string(Doc->get_نښان_Map().پیدا_کړه("!")->دوهم_2);
      Ret += Raw.substr(1);
      أرجع Ret;
    } وإلا إذا (Raw.starts_with("!!")) {
      Ret = std::string(Doc->get_نښان_Map().پیدا_کړه("!!")->دوهم_2);
      Ret += Raw.substr(2);
      أرجع Ret;
    } وإلا {
      StringRef نښان_Handle = Raw.substr(0, Raw.پیدا_کړه_وروستی_د('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->get_نښان_Map().پیدا_کړه(نښان_Handle);
      إذا (It != Doc->get_نښان_Map().پای_ټکی())
        Ret = std::string(It->دوهم_2);
      وإلا {
        Token T;
        T.ډول_2 = Token::TK_نښان;
        T.Range = نښان_Handle;
        سېټ_Error(Twine("Unknown tag handle ") + نښان_Handle, T);
      }
      Ret += Raw.substr(Raw.پیدا_کړه_وروستی_د('!') + 1);
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

Token &غوټه::peek_بل() {
  أرجع Doc->peek_بل();
}

Token غوټه::get_بل() {
  أرجع Doc->get_بل();
}

غوټه *غوټه::parse_بلاک_غوټه() {
  أرجع Doc->parse_بلاک_غوټه();
}

BumpPtrAllocator &غوټه::getAllocator() {
  أرجع Doc->غوټه_Allocator;
}

فراغ غوټه::سېټ_Error(ثابت Twine &پیغام_4, Token &Tok) ثابت {
  Doc->سېټ_Error(پیغام_4, Tok);
}

منطقي غوټه::failed() ثابت {
  أرجع Doc->failed();
}

StringRef Scalar_غوټه::get_ارزښت(Small_ویکټور_Impl<حرف> &Storage) ثابت {
  إذا (ارزښت[0] == '"')
    أرجع get_Double_Quoted_ارزښت(ارزښت, Storage);
  إذا (ارزښت[0] == '\'')
    أرجع get_Single_Quoted_ارزښت(ارزښت, Storage);
  أرجع get_Plain_ارزښت(ارزښت, Storage);
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
parse_Scalar_ارزښت(StringRef Unquoted_ارزښت, Small_ویکټور_Impl<حرف> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, Small_ویکټور_Impl<حرف> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_ارزښت.پیدا_کړه_لومړی_د(LookupChars);
  إذا (I == StringRef::npos)
    أرجع Unquoted_ارزښت;

  Storage.clear();
  Storage.reserve(Unquoted_ارزښت.اندازه_2());
  حرف وروستی_New_کرښه_Added_As = '\0';
  لكل (; I != StringRef::npos; I = Unquoted_ارزښت.پیدا_کړه_لومړی_د(LookupChars)) {
    إذا (Unquoted_ارزښت[I] != '\r' && Unquoted_ارزښت[I] != '\n') {
      llvm::append_range(Storage, Unquoted_ارزښت.take_front(I));
      Unquoted_ارزښت = UnescapeCallback(Unquoted_ارزښت.drop_front(I), Storage);
      وروستی_New_کرښه_Added_As = '\0';
      استمر;
    }
    إذا (size_t وروستی_Non_SWhite = Unquoted_ارزښت.پیدا_کړه_وروستی_not_د(" \t", I);
        وروستی_Non_SWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_ارزښت.take_front(وروستی_Non_SWhite + 1));
      Storage.ټېل_شا(' ');
      وروستی_New_کرښه_Added_As = ' ';
    } وإلا {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      بدل (وروستی_New_کرښه_Added_As) {
      حالة ' ':
        أكد(!Storage.تشه() && Storage.شا() == ' ');
        Storage.شا() = '\n';
        وروستی_New_کرښه_Added_As = '\n';
        اكسر;
      حالة '\n':
        أكد(!Storage.تشه() && Storage.شا() == '\n');
        Storage.ټېل_شا('\n');
        اكسر;
      افتراضي:
        Storage.ټېل_شا(' ');
        وروستی_New_کرښه_Added_As = ' ';
        اكسر;
      }
    }
    // Handle Windows-style EOL
    إذا (Unquoted_ارزښت.substr(I, 2) == "\r\n")
      I++;
    Unquoted_ارزښت = Unquoted_ارزښت.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_ارزښت);
  أرجع StringRef(Storage.begin(), Storage.اندازه_2());
}

StringRef
Scalar_غوټه::get_Double_Quoted_ارزښت(StringRef Raw_ارزښت,
                                 Small_ویکټور_Impl<حرف> &Storage) ثابت {
  أكد(Raw_ارزښت.اندازه_2() >= 2 && Raw_ارزښت.front() == '"' &&
         Raw_ارزښت.شا() == '"');
  StringRef Unquoted_ارزښت = Raw_ارزښت.substr(1, Raw_ارزښت.اندازه_2() - 2);

  خپلکاری UnescapeFunc = [هذا](StringRef Unquoted_ارزښت,
                             Small_ویکټور_Impl<حرف> &Storage) {
    أكد(Unquoted_ارزښت.take_front(1) == "\\");
    إذا (Unquoted_ارزښت.اندازه_2() == 1) {
      Token T;
      T.Range = Unquoted_ارزښت;
      سېټ_Error("Unrecognized escape code", T);
      Storage.clear();
      أرجع StringRef();
    }
    Unquoted_ارزښت = Unquoted_ارزښت.drop_front(1);
    بدل (Unquoted_ارزښت[0]) {
    افتراضي: {
      Token T;
      T.Range = Unquoted_ارزښت.take_front(1);
      سېټ_Error("Unrecognized escape code", T);
      Storage.clear();
      أرجع StringRef();
    }
    حالة '\r':
      // Shrink the Windows-style EOL.
      إذا (Unquoted_ارزښت.اندازه_2() >= 2 && Unquoted_ارزښت[1] == '\n')
        Unquoted_ارزښت = Unquoted_ارزښت.drop_front(1);
      [[fallthrough]];
    حالة '\n':
      أرجع Unquoted_ارزښت.drop_front(1).ltrim(" \t");
    حالة '0':
      Storage.ټېل_شا(0x00);
      اكسر;
    حالة 'a':
      Storage.ټېل_شا(0x07);
      اكسر;
    حالة 'b':
      Storage.ټېل_شا(0x08);
      اكسر;
    حالة 't':
    حالة 0x09:
      Storage.ټېل_شا(0x09);
      اكسر;
    حالة 'n':
      Storage.ټېل_شا(0x0A);
      اكسر;
    حالة 'v':
      Storage.ټېل_شا(0x0B);
      اكسر;
    حالة 'f':
      Storage.ټېل_شا(0x0C);
      اكسر;
    حالة 'r':
      Storage.ټېل_شا(0x0D);
      اكسر;
    حالة 'e':
      Storage.ټېل_شا(0x1B);
      اكسر;
    حالة ' ':
      Storage.ټېل_شا(0x20);
      اكسر;
    حالة '"':
      Storage.ټېل_شا(0x22);
      اكسر;
    حالة '/':
      Storage.ټېل_شا(0x2F);
      اكسر;
    حالة '\\':
      Storage.ټېل_شا(0x5C);
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
      إذا (Unquoted_ارزښت.اندازه_2() < 3)
        // TODO: Report error.
        اكسر;
      بینښه صحيح Unicode_Scalar_ارزښت;
      إذا (Unquoted_ارزښت.substr(1, 2).getAsInteger(16, Unicode_Scalar_ارزښت))
        // TODO: Report error.
        Unicode_Scalar_ارزښت = 0xFFFD;
      encodeUTF8(Unicode_Scalar_ارزښت, Storage);
      أرجع Unquoted_ارزښت.drop_front(3);
    }
    حالة 'u': {
      إذا (Unquoted_ارزښت.اندازه_2() < 5)
        // TODO: Report error.
        اكسر;
      بینښه صحيح Unicode_Scalar_ارزښت;
      إذا (Unquoted_ارزښت.substr(1, 4).getAsInteger(16, Unicode_Scalar_ارزښت))
        // TODO: Report error.
        Unicode_Scalar_ارزښت = 0xFFFD;
      encodeUTF8(Unicode_Scalar_ارزښت, Storage);
      أرجع Unquoted_ارزښت.drop_front(5);
    }
    حالة 'U': {
      إذا (Unquoted_ارزښت.اندازه_2() < 9)
        // TODO: Report error.
        اكسر;
      بینښه صحيح Unicode_Scalar_ارزښت;
      إذا (Unquoted_ارزښت.substr(1, 8).getAsInteger(16, Unicode_Scalar_ارزښت))
        // TODO: Report error.
        Unicode_Scalar_ارزښت = 0xFFFD;
      encodeUTF8(Unicode_Scalar_ارزښت, Storage);
      أرجع Unquoted_ارزښت.drop_front(9);
    }
    }
    أرجع Unquoted_ارزښت.drop_front(1);
  };

  أرجع parse_Scalar_ارزښت(Unquoted_ارزښت, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_غوټه::get_Single_Quoted_ارزښت(StringRef Raw_ارزښت,
                                           Small_ویکټور_Impl<حرف> &Storage) {
  أكد(Raw_ارزښت.اندازه_2() >= 2 && Raw_ارزښت.front() == '\'' &&
         Raw_ارزښت.شا() == '\'');
  StringRef Unquoted_ارزښت = Raw_ارزښت.substr(1, Raw_ارزښت.اندازه_2() - 2);

  خپلکاری UnescapeFunc = [](StringRef Unquoted_ارزښت,
                         Small_ویکټور_Impl<حرف> &Storage) {
    أكد(Unquoted_ارزښت.take_front(2) == "''");
    Storage.ټېل_شا('\'');
    أرجع Unquoted_ارزښت.drop_front(2);
  };

  أرجع parse_Scalar_ارزښت(Unquoted_ارزښت, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_غوټه::get_Plain_ارزښت(StringRef Raw_ارزښت,
                                    Small_ویکټور_Impl<حرف> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_ارزښت = Raw_ارزښت.rtrim("\r\n \t");
  أرجع parse_Scalar_ارزښت(Raw_ارزښت, Storage, "\r\n", تش_اشاره);
}

غوټه *کلی_ارزښت_غوټه::get_کلی() {
  إذا (کلی)
    أرجع کلی;
  // Handle implicit null keys.
  {
    Token &t = peek_بل();
    إذا (   t.ډول_2 == Token::TK_بلاک_پای_ټکی
        || t.ډول_2 == Token::TK_ارزښت
        || t.ډول_2 == Token::TK_Error) {
      أرجع کلی = جديد (getAllocator()) Null_غوټه(Doc);
    }
    إذا (t.ډول_2 == Token::TK_کلی)
      get_بل(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peek_بل();
  إذا (t.ډول_2 == Token::TK_بلاک_پای_ټکی || t.ډول_2 == Token::TK_ارزښت) {
    أرجع کلی = جديد (getAllocator()) Null_غوټه(Doc);
  }

  // We've got a normal key.
  أرجع کلی = parse_بلاک_غوټه();
}

غوټه *کلی_ارزښت_غوټه::get_ارزښت() {
  إذا (ارزښت)
    أرجع ارزښت;

  إذا (غوټه* کلی = get_کلی())
    کلی->skip();
  وإلا {
    سېټ_Error("Null key in Key Value.", peek_بل());
    أرجع ارزښت = جديد (getAllocator()) Null_غوټه(Doc);
  }

  إذا (failed())
    أرجع ارزښت = جديد (getAllocator()) Null_غوټه(Doc);

  // Handle implicit null values.
  {
    Token &t = peek_بل();
    إذا (   t.ډول_2 == Token::TK_بلاک_پای_ټکی
        || t.ډول_2 == Token::TK_Flow_Mapping_پای_ټکی
        || t.ډول_2 == Token::TK_کلی
        || t.ډول_2 == Token::TK_Flow_ثبت
        || t.ډول_2 == Token::TK_Error) {
      أرجع ارزښت = جديد (getAllocator()) Null_غوټه(Doc);
    }

    إذا (t.ډول_2 != Token::TK_ارزښت) {
      سېټ_Error("Unexpected token in Key Value.", t);
      أرجع ارزښت = جديد (getAllocator()) Null_غوټه(Doc);
    }
    get_بل(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peek_بل();
  إذا (t.ډول_2 == Token::TK_بلاک_پای_ټکی || t.ډول_2 == Token::TK_کلی) {
    أرجع ارزښت = جديد (getAllocator()) Null_غوټه(Doc);
  }

  // We got a normal value.
  أرجع ارزښت = parse_بلاک_غوټه();
}

فراغ Mapping_غوټه::increment() {
  إذا (failed()) {
    Is_At_پای_ټکی = ریښتیا;
    Current_ثبت = تش_اشاره;
    أرجع;
  }
  إذا (Current_ثبت) {
    Current_ثبت->skip();
    إذا (Type == MT_Inline) {
      Is_At_پای_ټکی = ریښتیا;
      Current_ثبت = تش_اشاره;
      أرجع;
    }
  }
  Token T = peek_بل();
  إذا (T.ډول_2 == Token::TK_کلی || T.ډول_2 == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    Current_ثبت = جديد (getAllocator()) کلی_ارزښت_غوټه(Doc);
  } وإلا إذا (Type == MT_بلاک) {
    بدل (T.ډول_2) {
    حالة Token::TK_بلاک_پای_ټکی:
      get_بل();
      Is_At_پای_ټکی = ریښتیا;
      Current_ثبت = تش_اشاره;
      اكسر;
    افتراضي:
      سېټ_Error("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    حالة Token::TK_Error:
      Is_At_پای_ټکی = ریښتیا;
      Current_ثبت = تش_اشاره;
    }
  } وإلا {
    بدل (T.ډول_2) {
    حالة Token::TK_Flow_ثبت:
      // Eat the flow entry and recurse.
      get_بل();
      أرجع increment();
    حالة Token::TK_Flow_Mapping_پای_ټکی:
      get_بل();
      [[fallthrough]];
    حالة Token::TK_Error:
      // Set this to end iterator.
      Is_At_پای_ټکی = ریښتیا;
      Current_ثبت = تش_اشاره;
      اكسر;
    افتراضي:
      سېټ_Error( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      Is_At_پای_ټکی = ریښتیا;
      Current_ثبت = تش_اشاره;
    }
  }
}

فراغ Sequence_غوټه::increment() {
  إذا (failed()) {
    Is_At_پای_ټکی = ریښتیا;
    Current_ثبت = تش_اشاره;
    أرجع;
  }
  إذا (Current_ثبت)
    Current_ثبت->skip();
  Token T = peek_بل();
  إذا (SeqType == ST_بلاک) {
    بدل (T.ډول_2) {
    حالة Token::TK_بلاک_ثبت:
      get_بل();
      Current_ثبت = parse_بلاک_غوټه();
      إذا (!Current_ثبت) { // An error occurred.
        Is_At_پای_ټکی = ریښتیا;
        Current_ثبت = تش_اشاره;
      }
      اكسر;
    حالة Token::TK_بلاک_پای_ټکی:
      get_بل();
      Is_At_پای_ټکی = ریښتیا;
      Current_ثبت = تش_اشاره;
      اكسر;
    افتراضي:
      سېټ_Error( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    حالة Token::TK_Error:
      Is_At_پای_ټکی = ریښتیا;
      Current_ثبت = تش_اشاره;
    }
  } وإلا إذا (SeqType == ST_Indentless) {
    بدل (T.ډول_2) {
    حالة Token::TK_بلاک_ثبت:
      get_بل();
      Current_ثبت = parse_بلاک_غوټه();
      إذا (!Current_ثبت) { // An error occurred.
        Is_At_پای_ټکی = ریښتیا;
        Current_ثبت = تش_اشاره;
      }
      اكسر;
    افتراضي:
    حالة Token::TK_Error:
      Is_At_پای_ټکی = ریښتیا;
      Current_ثبت = تش_اشاره;
    }
  } وإلا إذا (SeqType == ST_Flow) {
    بدل (T.ډول_2) {
    حالة Token::TK_Flow_ثبت:
      // Eat the flow entry and recurse.
      get_بل();
      Was_پخوانی_Token_Flow_ثبت = ریښتیا;
      أرجع increment();
    حالة Token::TK_Flow_Sequence_پای_ټکی:
      get_بل();
      [[fallthrough]];
    حالة Token::TK_Error:
      // Set this to end iterator.
      Is_At_پای_ټکی = ریښتیا;
      Current_ثبت = تش_اشاره;
      اكسر;
    حالة Token::TK_بهیر_پای_ټکی:
    حالة Token::TK_Document_پای_ټکی:
    حالة Token::TK_Document_پیل:
      سېټ_Error("Could not find closing ]!", T);
      // Set this to end iterator.
      Is_At_پای_ټکی = ریښتیا;
      Current_ثبت = تش_اشاره;
      اكسر;
    افتراضي:
      إذا (!Was_پخوانی_Token_Flow_ثبت) {
        سېټ_Error("Expected , between entries!", T);
        Is_At_پای_ټکی = ریښتیا;
        Current_ثبت = تش_اشاره;
        اكسر;
      }
      // Otherwise it must be a flow entry.
      Current_ثبت = parse_بلاک_غوټه();
      إذا (!Current_ثبت) {
        Is_At_پای_ټکی = ریښتیا;
      }
      Was_پخوانی_Token_Flow_ثبت = خطأ;
      اكسر;
    }
  }
}

Document::Document(بهیر &S) : بهیر_2(S), ریښه_2(تش_اشاره) {
  // Tag maps starts with two default mappings.
  نښان_Map["!"] = "!";
  نښان_Map["!!"] = "tag:yaml.org,2002:";

  إذا (parseDirectives())
    expectToken(Token::TK_Document_پیل);
  Token &T = peek_بل();
  إذا (T.ډول_2 == Token::TK_Document_پیل)
    get_بل();
}

منطقي Document::skip()  {
  إذا (بهیر_2.scanner->failed())
    أرجع خطأ;
  إذا (!ریښه_2 && !get_ریښه())
    أرجع خطأ;
  ریښه_2->skip();
  Token &T = peek_بل();
  إذا (T.ډول_2 == Token::TK_بهیر_پای_ټکی)
    أرجع خطأ;
  إذا (T.ډول_2 == Token::TK_Document_پای_ټکی) {
    get_بل();
    أرجع skip();
  }
  أرجع ریښتیا;
}

Token &Document::peek_بل() {
  أرجع بهیر_2.scanner->peek_بل();
}

Token Document::get_بل() {
  أرجع بهیر_2.scanner->get_بل();
}

فراغ Document::سېټ_Error(ثابت Twine &پیغام_3, Token &Location) ثابت {
  بهیر_2.scanner->سېټ_Error(پیغام_3, Location.Range.begin());
}

منطقي Document::failed() ثابت {
  أرجع بهیر_2.scanner->failed();
}

غوټه *Document::parse_بلاک_غوټه() {
  Token T = peek_بل();
  // Handle properties.
  Token AnchorInfo;
  Token نښان_Info;
parse_property:
  بدل (T.ډول_2) {
  حالة Token::TK_Alias:
    get_بل();
    أرجع جديد (غوټه_Allocator) Alias_غوټه(بهیر_2.CurrentDoc, T.Range.substr(1));
  حالة Token::TK_Anchor:
    إذا (AnchorInfo.ډول_2 == Token::TK_Anchor) {
      سېټ_Error("Already encountered an anchor for this node!", T);
      أرجع تش_اشاره;
    }
    AnchorInfo = get_بل(); // Consume TK_Anchor.
    T = peek_بل();
    اذهب parse_property;
  حالة Token::TK_نښان:
    إذا (نښان_Info.ډول_2 == Token::TK_نښان) {
      سېټ_Error("Already encountered a tag for this node!", T);
      أرجع تش_اشاره;
    }
    نښان_Info = get_بل(); // Consume TK_Tag.
    T = peek_بل();
    اذهب parse_property;
  افتراضي:
    اكسر;
  }

  بدل (T.ډول_2) {
  حالة Token::TK_بلاک_ثبت:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    أرجع جديد (غوټه_Allocator) Sequence_غوټه( بهیر_2.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , نښان_Info.Range
                                           , Sequence_غوټه::ST_Indentless);
  حالة Token::TK_بلاک_Sequence_پیل:
    get_بل();
    أرجع جديد (غوټه_Allocator)
      Sequence_غوټه( بهیر_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , نښان_Info.Range
                  , Sequence_غوټه::ST_بلاک);
  حالة Token::TK_بلاک_Mapping_پیل:
    get_بل();
    أرجع جديد (غوټه_Allocator)
      Mapping_غوټه( بهیر_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , نښان_Info.Range
                 , Mapping_غوټه::MT_بلاک);
  حالة Token::TK_Flow_Sequence_پیل:
    get_بل();
    أرجع جديد (غوټه_Allocator)
      Sequence_غوټه( بهیر_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , نښان_Info.Range
                  , Sequence_غوټه::ST_Flow);
  حالة Token::TK_Flow_Mapping_پیل:
    get_بل();
    أرجع جديد (غوټه_Allocator)
      Mapping_غوټه( بهیر_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , نښان_Info.Range
                 , Mapping_غوټه::MT_Flow);
  حالة Token::TK_Scalar:
    get_بل();
    أرجع جديد (غوټه_Allocator)
      Scalar_غوټه( بهیر_2.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , نښان_Info.Range
                , T.Range);
  حالة Token::TK_بلاک_Scalar: {
    get_بل();
    StringRef NullTerminatedStr(T.ارزښت.c_str(), T.ارزښت.اوږدتیا_2() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(غوټه_Allocator).drop_شا();
    أرجع جديد (غوټه_Allocator)
        بلاک_Scalar_غوټه(بهیر_2.CurrentDoc, AnchorInfo.Range.substr(1),
                        نښان_Info.Range, StrCopy, T.Range);
  }
  حالة Token::TK_کلی:
    // Don't eat the TK_Key, KeyValueNode expects it.
    أرجع جديد (غوټه_Allocator)
      Mapping_غوټه( بهیر_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , نښان_Info.Range
                 , Mapping_غوټه::MT_Inline);
  حالة Token::TK_Document_پیل:
  حالة Token::TK_Document_پای_ټکی:
  حالة Token::TK_بهیر_پای_ټکی:
  افتراضي:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    أرجع جديد (غوټه_Allocator) Null_غوټه(بهیر_2.CurrentDoc);
  حالة Token::TK_Flow_Mapping_پای_ټکی:
  حالة Token::TK_Flow_Sequence_پای_ټکی:
  حالة Token::TK_Flow_ثبت: {
    إذا (ریښه_2 && (isa<Mapping_غوټه>(ریښه_2) || isa<Sequence_غوټه>(ریښه_2)))
      أرجع جديد (غوټه_Allocator) Null_غوټه(بهیر_2.CurrentDoc);

    سېټ_Error("Unexpected token", T);
    أرجع تش_اشاره;
  }
  حالة Token::TK_Error:
    أرجع تش_اشاره;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  أرجع تش_اشاره;
}

منطقي Document::parseDirectives() {
  منطقي isDirective = خطأ;
  طالما (ریښتیا) {
    Token T = peek_بل();
    إذا (T.ډول_2 == Token::TK_نښان_Directive) {
      parseTAGDirective();
      isDirective = ریښتیا;
    } وإلا إذا (T.ډول_2 == Token::TK_نسخه_Directive) {
      parseYAMLDirective();
      isDirective = ریښتیا;
    } وإلا {
      اكسر;
    }
  }
  أرجع isDirective;
}

فراغ Document::parseYAMLDirective() {
  get_بل(); // Eat %YAML <version>
}

فراغ Document::parseTAGDirective() {
  Token نښان = get_بل(); // %TAG <handle> <prefix>
  StringRef T = نښان.Range;
  // Strip %TAG
  T = T.substr(T.پیدا_کړه_لومړی_د(" \t")).ltrim(" \t");
  std::size_t Handle_پای_ټکی = T.پیدا_کړه_لومړی_د(" \t");
  StringRef نښان_Handle = T.substr(0, Handle_پای_ټکی);
  StringRef نښان_مختاړی = T.substr(Handle_پای_ټکی).ltrim(" \t");
  نښان_Map[نښان_Handle] = نښان_مختاړی;
}

منطقي Document::expectToken(صحيح TK) {
  Token T = get_بل();
  إذا (T.ډول_2 != TK) {
    سېټ_Error("Unexpected token", T);
    أرجع خطأ;
  }
  أرجع ریښتیا;
}
