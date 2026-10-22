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

#شامل "llvm/Support/YAMLParser.h"
#شامل "llvm/ADT/AllocatorList.h"
#شامل "llvm/ADT/ArrayRef.h"
#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/ADT/Twine.h"
#شامل "llvm/Support/Compiler.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/SMLoc.h"
#شامل "llvm/Support/SourceMgr.h"
#شامل "llvm/Support/Unicode.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cassert>
#شامل <cstddef>
#شامل <cstdint>
#شامل <map>
#شامل <memory>
#شامل <string>
#شامل <system_error>
#شامل <utility>

استعمال نامخلا llvm;
استعمال نامخلا yaml;

شمار UnicodeEncodingForm {
  UEF_UTF32_LE, ///< UTF-32 Little Endian
  UEF_UTF32_BE, ///< UTF-32 Big Endian
  UEF_UTF16_LE, ///< UTF-16 Little Endian
  UEF_UTF16_BE, ///< UTF-16 Big Endian
  UEF_UTF8,     ///< UTF-8 or ascii.
  UEF_Unknown   ///< Not a valid Unicode encoding.
};

/// EncodingInfo - Holds the encoding type and length of the byte order mark if
///                it exists. Length is in {0, 2, 3, 4}.
استعمال EncodingInfo = std::pair<UnicodeEncodingForm, غیرعلامتی>;

/// getUnicodeEncoding - Reads up to the first 4 bytes to determine the Unicode
///                      encoding form of \a Input.
///
/// @param Input A string of length 0 or more.
/// @returns An EncodingInfo indicating the Unicode encoding form of the input
///          and how long the byte order mark is if one exists.
ساکن EncodingInfo getUnicodeEncoding(StringRef داخلہ) {
  اگر (داخلہ.خالی_خانے())
    واپس {UEF_Unknown, 0};

  انتخاب (uint8_t(داخلہ[0])) {
  حالت 0x00:
    اگر (داخلہ.سائز_2() >= 4) {
      اگر (  داخلہ[1] == 0
         && uint8_t(داخلہ[2]) == 0xFE
         && uint8_t(داخلہ[3]) == 0xFF)
        واپس {UEF_UTF32_BE, 4};
      اگر (داخلہ[1] == 0 && داخلہ[2] == 0 && داخلہ[3] != 0)
        واپس {UEF_UTF32_BE, 0};
    }

    اگر (داخلہ.سائز_2() >= 2 && داخلہ[1] != 0)
      واپس {UEF_UTF16_BE, 0};
    واپس {UEF_Unknown, 0};
  حالت 0xFF:
    اگر (  داخلہ.سائز_2() >= 4
       && uint8_t(داخلہ[1]) == 0xFE
       && داخلہ[2] == 0
       && داخلہ[3] == 0)
      واپس {UEF_UTF32_LE, 4};

    اگر (داخلہ.سائز_2() >= 2 && uint8_t(داخلہ[1]) == 0xFE)
      واپس {UEF_UTF16_LE, 2};
    واپس {UEF_Unknown, 0};
  حالت 0xFE:
    اگر (داخلہ.سائز_2() >= 2 && uint8_t(داخلہ[1]) == 0xFF)
      واپس {UEF_UTF16_BE, 2};
    واپس {UEF_Unknown, 0};
  حالت 0xEF:
    اگر (  داخلہ.سائز_2() >= 3
       && uint8_t(داخلہ[1]) == 0xBB
       && uint8_t(داخلہ[2]) == 0xBF)
      واپس {UEF_UTF8, 3};
    واپس {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  اگر (داخلہ.سائز_2() >= 4 && داخلہ[1] == 0 && داخلہ[2] == 0 && داخلہ[3] == 0)
    واپس {UEF_UTF32_LE, 0};

  اگر (داخلہ.سائز_2() >= 2 && داخلہ[1] == 0)
    واپس {UEF_UTF16_LE, 0};

  واپس {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
خالی گرہ::anchor() {}
خالی Null_گرہ::anchor() {}
خالی Scalar_گرہ::anchor() {}
خالی بلاک_Scalar_گرہ::anchor() {}
خالی کلید_قدر_گرہ::anchor() {}
خالی Mapping_گرہ::anchor() {}
خالی Sequence_گرہ::anchor() {}
خالی Alias_گرہ::anchor() {}

نامخلا llvm {
نامخلا yaml {

/// Token - A single YAML token.
ساخت Token {
  شمار Token_قسم {
    TK_Error, // Uninitialized token.
    TK_دھارا_آغاز,
    TK_دھارا_انتہا,
    TK_نسخہ_Directive,
    TK_لیبل_Directive,
    TK_Document_آغاز,
    TK_Document_انتہا,
    TK_بلاک_اندراج,
    TK_بلاک_انتہا,
    TK_بلاک_Sequence_آغاز,
    TK_بلاک_Mapping_آغاز,
    TK_Flow_اندراج,
    TK_Flow_Sequence_آغاز,
    TK_Flow_Sequence_انتہا,
    TK_Flow_Mapping_آغاز,
    TK_Flow_Mapping_انتہا,
    TK_کلید,
    TK_قدر,
    TK_Scalar,
    TK_بلاک_Scalar,
    TK_Alias,
    TK_Anchor,
    TK_لیبل
  } قسم_3 = TK_Error;

  /// A string of length 0 or more whose begin() points to the logical location
  /// of the token in the input.
  StringRef Range;

  /// The value of a block scalar node.
  std::string قدر;

  Token() = طےشدہ;
};

} // end namespace yaml
} // end namespace llvm

استعمال Token_قطار_T = BumpPtrList<Token>;

نامخلا {

/// This struct is used to track simple keys.
///
/// Simple keys are handled by creating an entry in SimpleKeys for each Token
/// which could legally be the start of a simple key. When peekNext is called,
/// if the Token To be returned is referenced by a SimpleKey, we continue
/// tokenizing until that potential simple key has either been found to not be
/// a simple key (we moved on to the next line or went further than 1024 chars).
/// Or when we run into a Value, and then insert a Key token (and possibly
/// others) before the SimpleKey's Tok.
ساخت Simple_کلید {
  Token_قطار_T::iterator Tok;
  غیرعلامتی عمود = 0;
  غیرعلامتی سطر = 0;
  غیرعلامتی FlowLevel = 0;
  منطقی IsRequired = جھوٹ;

  منطقی عامل ==(ثابت Simple_کلید &Other) {
    واپس Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
استعمال UTF8Decoded = std::pair<uint32_t, غیرعلامتی>;

ساکن UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator انتہا_2 = Range.انتہا();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  اگر (Position < انتہا_2 && (*Position & 0x80) == 0) {
    واپس {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  اگر (Position + 1 < انتہا_2 && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    اگر (codepoint >= 0x80)
      واپس {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  اگر (Position + 2 < انتہا_2 && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    اگر (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      واپس {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  اگر (Position + 3 < انتہا_2 && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    اگر (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      واپس {codepoint, 4};
  }
  واپس {0, 0};
}

نامخلا llvm {
نامخلا yaml {

/// Scans YAML tokens from a MemoryBuffer.
جماعت Scanner {
عوامی:
  Scanner(StringRef داخلہ, ماخذ_Mgr &SM, منطقی ShowColors = سچ,
          std::error_رمز *EC = خالی_اشاریہ);
  Scanner(حافظہ_بفر_Ref بفر, ماخذ_Mgr &SM_, منطقی ShowColors = سچ,
          std::error_رمز *EC = خالی_اشاریہ);

  /// Parse the next token and return it without popping it.
  Token &peek_اگلا();

  /// Parse the next token and pop it from the queue.
  Token get_اگلا();

  خالی printError(SMLoc Loc, ماخذ_Mgr::Diag_قسم قسم_3, ثابت Twine &پیغام_3,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.Print_پیغام(Loc, قسم_3, پیغام_3, Ranges, /* FixIts= */ {}, ShowColors);
  }

  خالی سیٹ_Error(ثابت Twine &پیغام_3, StringRef::iterator Position) {
    اگر (Position >= انتہا_2)
      Position = انتہا_2 - 1;

    // propagate the error if possible
    اگر (EC)
      *EC = make_error_رمز(std::errc::invalid_دلیل);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    اگر (!Failed)
      printError(SMLoc::get_From_اشارہ_گر(Position), ماخذ_Mgr::DK_Error, پیغام_3);
    Failed = سچ;
  }

  /// Returns true if an error occurred while parsing.
  منطقی failed() {
    واپس Failed;
  }

نجی:
  خالی init(حافظہ_بفر_Ref بفر);

  StringRef current_داخلہ() {
    واپس StringRef(Current, انتہا_2 - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    واپس ::decodeUTF8(StringRef(Position, انتہا_2 - Position));
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
  خالی advanceWhile(SkipWhileFunc Func);

  /// Scan ns-uri-char[39]s starting at Cur.
  ///
  /// This updates Cur and Column while scanning.
  خالی جائزہ_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  منطقی consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  خالی skip(uint32_t فاصلہ_2);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  منطقی isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  منطقی is_Plain_محفوظ_Non_Blank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  منطقی is_سطر_خالی_خانے(StringRef سطر);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  منطقی consume_سطر_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  خالی save_Simple_کلید_Candidate( Token_قطار_T::iterator Tok
                             , غیرعلامتی At_عمود
                             , منطقی IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  خالی remove_Stale_Simple_کلید_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  خالی remove_Simple_کلید_Candidates_On_Flow_Level(غیرعلامتی Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  منطقی unrollIndent(صحیح مقصد_عمود);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  منطقی rollIndent( صحیح مقصد_عمود
                 , Token::Token_قسم قسم_3
                 , Token_قطار_T::iterator داخل_کرو_نقطہ);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  خالی skipComment();

  /// Skip whitespace and comments until the start of the next token.
  خالی جائزہ_مقصد_اگلا_Token();

  /// Must be the first token generated.
  منطقی جائزہ_دھارا_آغاز();

  /// Generate tokens needed to close out the stream.
  منطقی جائزہ_دھارا_انتہا();

  /// Scan a %BLAH directive.
  منطقی جائزہ_Directive();

  /// Scan a ... or ---.
  منطقی جائزہ_Document_Indicator(منطقی Is_آغاز);

  /// Scan a [ or { and generate the proper flow collection start token.
  منطقی جائزہ_Flow_Collection_آغاز(منطقی IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  منطقی جائزہ_Flow_Collection_انتہا(منطقی IsSequence);

  /// Scan the , that separates entries in a flow collection.
  منطقی جائزہ_Flow_اندراج();

  /// Scan the - that starts block sequence entries.
  منطقی جائزہ_بلاک_اندراج();

  /// Scan an explicit ? indicating a key.
  منطقی جائزہ_کلید();

  /// Scan an explicit : indicating a value.
  منطقی جائزہ_قدر();

  /// Scan a quoted scalar.
  منطقی جائزہ_Flow_Scalar(منطقی IsDoubleQuoted);

  /// Scan an unquoted scalar.
  منطقی جائزہ_Plain_Scalar();

  /// Scan an Alias or Anchor starting with * or &.
  منطقی جائزہ_Alias_Or_Anchor(منطقی IsAlias);

  /// Scan a block scalar starting with | or >.
  منطقی جائزہ_بلاک_Scalar(منطقی IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  منطقی جائزہ_بلاک_Scalar_Indicators(حرف &StyleIndicator, حرف &ChompingIndicator,
                                 غیرعلامتی &IndentIndicator, منطقی &Is_مکمل);

  /// Scan a style indicator in a block scalar header.
  حرف جائزہ_بلاک_Style_Indicator();

  /// Scan a chomping indicator in a block scalar header.
  حرف جائزہ_بلاک_Chomping_Indicator();

  /// Scan an indentation indicator in a block scalar header.
  غیرعلامتی جائزہ_بلاک_Indentation_Indicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  منطقی جائزہ_بلاک_Scalar_سرخی(حرف &ChompingIndicator, غیرعلامتی &IndentIndicator,
                             منطقی &Is_مکمل);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  منطقی ڈھونڈو_بلاک_Scalar_Indent(غیرعلامتی &بلاک_Indent, غیرعلامتی بلاک_Exit_Indent,
                             غیرعلامتی &سطر_Breaks, منطقی &Is_مکمل);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  منطقی جائزہ_بلاک_Scalar_Indent(غیرعلامتی بلاک_Indent, غیرعلامتی بلاک_Exit_Indent,
                             منطقی &Is_مکمل);

  /// Scan a tag of the form !stuff.
  منطقی جائزہ_لیبل();

  /// Dispatch to the next scanning function based on \a *Cur.
  منطقی fetch_مزید_علامات();

  /// The SourceMgr used for diagnostics and buffer management.
  ماخذ_Mgr &SM;

  /// The original input.
  حافظہ_بفر_Ref داخلہ_بفر;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator انتہا_2;

  /// Current YAML indentation level in spaces.
  صحیح Indent;

  /// Current column number in Unicode code points.
  غیرعلامتی عمود;

  /// Current line number.
  غیرعلامتی سطر;

  /// How deep we are in flow style containers. 0 Means at block level.
  غیرعلامتی FlowLevel;

  /// Are we at the start of the stream?
  منطقی Is_آغاز_کا_دھارا;

  /// Can the next token be the start of a simple key?
  منطقی Is_Simple_کلید_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  منطقی Is_Adjacent_قدر_Allowed_اندر_Flow;

  /// True if an error has occurred.
  منطقی Failed;

  /// Should colors be used when printing out the diagnostic messages?
  منطقی ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  Token_قطار_T Token_قطار;

  /// Indentation levels.
  Small_سمتیہ<صحیح, 4> Indents;

  /// Potential simple keys.
  Small_سمتیہ<Simple_کلید, 4> Simple_کلیدیں;

  std::error_رمز *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
ساکن خالی encodeUTF8( uint32_t Unicode_Scalar_قدر
                      , Small_سمتیہ_Impl<حرف> &نتیجہ) {
  اگر (Unicode_Scalar_قدر <= 0x7F) {
    نتیجہ.ڈالو_عقب(Unicode_Scalar_قدر & 0x7F);
  } ورنہ اگر (Unicode_Scalar_قدر <= 0x7FF) {
    uint8_t پہلا_Byte = 0xC0 | ((Unicode_Scalar_قدر & 0x7C0) >> 6);
    uint8_t دوسرا_Byte = 0x80 | (Unicode_Scalar_قدر & 0x3F);
    نتیجہ.ڈالو_عقب(پہلا_Byte);
    نتیجہ.ڈالو_عقب(دوسرا_Byte);
  } ورنہ اگر (Unicode_Scalar_قدر <= 0xFFFF) {
    uint8_t پہلا_Byte = 0xE0 | ((Unicode_Scalar_قدر & 0xF000) >> 12);
    uint8_t دوسرا_Byte = 0x80 | ((Unicode_Scalar_قدر & 0xFC0) >> 6);
    uint8_t تیسرا_Byte = 0x80 | (Unicode_Scalar_قدر & 0x3F);
    نتیجہ.ڈالو_عقب(پہلا_Byte);
    نتیجہ.ڈالو_عقب(دوسرا_Byte);
    نتیجہ.ڈالو_عقب(تیسرا_Byte);
  } ورنہ اگر (Unicode_Scalar_قدر <= 0x10FFFF) {
    uint8_t پہلا_Byte = 0xF0 | ((Unicode_Scalar_قدر & 0x1F0000) >> 18);
    uint8_t دوسرا_Byte = 0x80 | ((Unicode_Scalar_قدر & 0x3F000) >> 12);
    uint8_t تیسرا_Byte = 0x80 | ((Unicode_Scalar_قدر & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_قدر & 0x3F);
    نتیجہ.ڈالو_عقب(پہلا_Byte);
    نتیجہ.ڈالو_عقب(دوسرا_Byte);
    نتیجہ.ڈالو_عقب(تیسرا_Byte);
    نتیجہ.ڈالو_عقب(FourthByte);
  }
}

منطقی yaml::dump_علامات(StringRef داخلہ, raw_ostream &OS) {
  ماخذ_Mgr SM;
  Scanner scanner(داخلہ, SM);
  جب (سچ) {
    Token T = scanner.get_اگلا();
    انتخاب (T.قسم_3) {
    حالت Token::TK_دھارا_آغاز:
      OS << "Stream-Start: ";
      توڑو;
    حالت Token::TK_دھارا_انتہا:
      OS << "Stream-End: ";
      توڑو;
    حالت Token::TK_نسخہ_Directive:
      OS << "Version-Directive: ";
      توڑو;
    حالت Token::TK_لیبل_Directive:
      OS << "Tag-Directive: ";
      توڑو;
    حالت Token::TK_Document_آغاز:
      OS << "Document-Start: ";
      توڑو;
    حالت Token::TK_Document_انتہا:
      OS << "Document-End: ";
      توڑو;
    حالت Token::TK_بلاک_اندراج:
      OS << "Block-Entry: ";
      توڑو;
    حالت Token::TK_بلاک_انتہا:
      OS << "Block-End: ";
      توڑو;
    حالت Token::TK_بلاک_Sequence_آغاز:
      OS << "Block-Sequence-Start: ";
      توڑو;
    حالت Token::TK_بلاک_Mapping_آغاز:
      OS << "Block-Mapping-Start: ";
      توڑو;
    حالت Token::TK_Flow_اندراج:
      OS << "Flow-Entry: ";
      توڑو;
    حالت Token::TK_Flow_Sequence_آغاز:
      OS << "Flow-Sequence-Start: ";
      توڑو;
    حالت Token::TK_Flow_Sequence_انتہا:
      OS << "Flow-Sequence-End: ";
      توڑو;
    حالت Token::TK_Flow_Mapping_آغاز:
      OS << "Flow-Mapping-Start: ";
      توڑو;
    حالت Token::TK_Flow_Mapping_انتہا:
      OS << "Flow-Mapping-End: ";
      توڑو;
    حالت Token::TK_کلید:
      OS << "Key: ";
      توڑو;
    حالت Token::TK_قدر:
      OS << "Value: ";
      توڑو;
    حالت Token::TK_Scalar:
      OS << "Scalar: ";
      توڑو;
    حالت Token::TK_بلاک_Scalar:
      OS << "Block Scalar: ";
      توڑو;
    حالت Token::TK_Alias:
      OS << "Alias: ";
      توڑو;
    حالت Token::TK_Anchor:
      OS << "Anchor: ";
      توڑو;
    حالت Token::TK_لیبل:
      OS << "Tag: ";
      توڑو;
    حالت Token::TK_Error:
      توڑو;
    }
    OS << T.Range << "\n";
    اگر (T.قسم_3 == Token::TK_دھارا_انتہا)
      توڑو;
    ورنہ اگر (T.قسم_3 == Token::TK_Error)
      واپس جھوٹ;
  }
  واپس سچ;
}

منطقی yaml::جائزہ_علامات(StringRef داخلہ) {
  ماخذ_Mgr SM;
  Scanner scanner(داخلہ, SM);
  جب (سچ) {
    Token T = scanner.get_اگلا();
    اگر (T.قسم_3 == Token::TK_دھارا_انتہا)
      توڑو;
    ورنہ اگر (T.قسم_3 == Token::TK_Error)
      واپس جھوٹ;
  }
  واپس سچ;
}

std::string yaml::escape(StringRef داخلہ, منطقی EscapePrintable) {
  std::string Escaped_داخلہ;
  برائے (StringRef::iterator i = داخلہ.begin(), e = داخلہ.انتہا(); i != e; ++i) {
    اگر (*i == '\\')
      Escaped_داخلہ += "\\\\";
    ورنہ اگر (*i == '"')
      Escaped_داخلہ += "\\\"";
    ورنہ اگر (*i == 0)
      Escaped_داخلہ += "\\0";
    ورنہ اگر (*i == 0x07)
      Escaped_داخلہ += "\\a";
    ورنہ اگر (*i == 0x08)
      Escaped_داخلہ += "\\b";
    ورنہ اگر (*i == 0x09)
      Escaped_داخلہ += "\\t";
    ورنہ اگر (*i == 0x0A)
      Escaped_داخلہ += "\\n";
    ورنہ اگر (*i == 0x0B)
      Escaped_داخلہ += "\\v";
    ورنہ اگر (*i == 0x0C)
      Escaped_داخلہ += "\\f";
    ورنہ اگر (*i == 0x0D)
      Escaped_داخلہ += "\\r";
    ورنہ اگر (*i == 0x1B)
      Escaped_داخلہ += "\\e";
    ورنہ اگر ((غیرعلامتی حرف)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_داخلہ += "\\x" + std::string(2 - HexStr.سائز_2(), '0') + HexStr;
    } ورنہ اگر (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_قدر
        = decodeUTF8(StringRef(i, داخلہ.انتہا() - i));
      اگر (Unicode_Scalar_قدر.دوسرا_2 == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_داخلہ, Val);
        // FIXME: Error reporting.
        واپس Escaped_داخلہ;
      }
      اگر (Unicode_Scalar_قدر.پہلا_2 == 0x85)
        Escaped_داخلہ += "\\N";
      ورنہ اگر (Unicode_Scalar_قدر.پہلا_2 == 0xA0)
        Escaped_داخلہ += "\\_";
      ورنہ اگر (Unicode_Scalar_قدر.پہلا_2 == 0x2028)
        Escaped_داخلہ += "\\L";
      ورنہ اگر (Unicode_Scalar_قدر.پہلا_2 == 0x2029)
        Escaped_داخلہ += "\\P";
      ورنہ اگر (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_قدر.پہلا_2))
        Escaped_داخلہ += StringRef(i, Unicode_Scalar_قدر.دوسرا_2);
      ورنہ {
        std::string HexStr = utohexstr(Unicode_Scalar_قدر.پہلا_2);
        اگر (HexStr.سائز_2() <= 2)
          Escaped_داخلہ += "\\x" + std::string(2 - HexStr.سائز_2(), '0') + HexStr;
        ورنہ اگر (HexStr.سائز_2() <= 4)
          Escaped_داخلہ += "\\u" + std::string(4 - HexStr.سائز_2(), '0') + HexStr;
        ورنہ اگر (HexStr.سائز_2() <= 8)
          Escaped_داخلہ += "\\U" + std::string(8 - HexStr.سائز_2(), '0') + HexStr;
      }
      i += Unicode_Scalar_قدر.دوسرا_2 - 1;
    } ورنہ {
      Escaped_داخلہ.ڈالو_عقب(*i);
    }
  }
  واپس Escaped_داخلہ;
}

std::optional<منطقی> yaml::parseBool(StringRef S) {
  انتخاب (S.سائز_2()) {
  حالت 1:
    انتخاب (S.front()) {
    حالت 'y':
    حالت 'Y':
      واپس سچ;
    حالت 'n':
    حالت 'N':
      واپس جھوٹ;
    طےشدہ:
      واپس std::nullopt;
    }
  حالت 2:
    انتخاب (S.front()) {
    حالت 'O':
      اگر (S[1] == 'N') // ON
        واپس سچ;
      [[fallthrough]];
    حالت 'o':
      اگر (S[1] == 'n') //[Oo]n
        واپس سچ;
      واپس std::nullopt;
    حالت 'N':
      اگر (S[1] == 'O') // NO
        واپس جھوٹ;
      [[fallthrough]];
    حالت 'n':
      اگر (S[1] == 'o') //[Nn]o
        واپس جھوٹ;
      واپس std::nullopt;
    طےشدہ:
      واپس std::nullopt;
    }
  حالت 3:
    انتخاب (S.front()) {
    حالت 'O':
      اگر (S.drop_front() == "FF") // OFF
        واپس جھوٹ;
      [[fallthrough]];
    حالت 'o':
      اگر (S.drop_front() == "ff") //[Oo]ff
        واپس جھوٹ;
      واپس std::nullopt;
    حالت 'Y':
      اگر (S.drop_front() == "ES") // YES
        واپس سچ;
      [[fallthrough]];
    حالت 'y':
      اگر (S.drop_front() == "es") //[Yy]es
        واپس سچ;
      واپس std::nullopt;
    طےشدہ:
      واپس std::nullopt;
    }
  حالت 4:
    انتخاب (S.front()) {
    حالت 'T':
      اگر (S.drop_front() == "RUE") // TRUE
        واپس سچ;
      [[fallthrough]];
    حالت 't':
      اگر (S.drop_front() == "rue") //[Tt]rue
        واپس سچ;
      واپس std::nullopt;
    طےشدہ:
      واپس std::nullopt;
    }
  حالت 5:
    انتخاب (S.front()) {
    حالت 'F':
      اگر (S.drop_front() == "ALSE") // FALSE
        واپس جھوٹ;
      [[fallthrough]];
    حالت 'f':
      اگر (S.drop_front() == "alse") //[Ff]alse
        واپس جھوٹ;
      واپس std::nullopt;
    طےشدہ:
      واپس std::nullopt;
    }
  طےشدہ:
    واپس std::nullopt;
  }
}

Scanner::Scanner(StringRef داخلہ, ماخذ_Mgr &sm, منطقی ShowColors,
                 std::error_رمز *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(حافظہ_بفر_Ref(داخلہ, "YAML"));
}

Scanner::Scanner(حافظہ_بفر_Ref بفر, ماخذ_Mgr &SM_, منطقی ShowColors,
                 std::error_رمز *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(بفر);
}

خالی Scanner::init(حافظہ_بفر_Ref بفر) {
  داخلہ_بفر = بفر;
  Current = داخلہ_بفر.get_بفر_آغاز();
  انتہا_2 = داخلہ_بفر.get_بفر_انتہا();
  Indent = -1;
  عمود = 0;
  سطر = 0;
  FlowLevel = 0;
  Is_آغاز_کا_دھارا = سچ;
  Is_Simple_کلید_Allowed = سچ;
  Is_Adjacent_قدر_Allowed_اندر_Flow = جھوٹ;
  Failed = جھوٹ;
  std::unique_ptr<حافظہ_بفر> داخلہ_بفر_Owner =
      حافظہ_بفر::get_Mem_بفر(بفر, /*RequiresNullTerminator=*/جھوٹ);
  SM.جوڑو_New_ماخذ_بفر(std::move(داخلہ_بفر_Owner), SMLoc());
}

Token &Scanner::peek_اگلا() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  منطقی Need_مزید = جھوٹ;
  جب (سچ) {
    اگر (Token_قطار.خالی_خانے() || Need_مزید) {
      اگر (!fetch_مزید_علامات()) {
        Token_قطار.clear();
        Simple_کلیدیں.clear();
        Token_قطار.ڈالو_عقب(Token());
        واپس Token_قطار.front();
      }
    }
    أكد(!Token_قطار.خالی_خانے() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_کلید_Candidates();
    Simple_کلید SK;
    SK.Tok = Token_قطار.begin();
    اگر (!is_contained(Simple_کلیدیں, SK))
      توڑو;
    ورنہ
      Need_مزید = سچ;
  }
  واپس Token_قطار.front();
}

Token Scanner::get_اگلا() {
  Token Ret = peek_اگلا();
  // TokenQueue can be empty if there was an error getting the next token.
  اگر (!Token_قطار.خالی_خانے())
    Token_قطار.نکالو_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  اگر (Token_قطار.خالی_خانے())
    Token_قطار.resetAlloc();

  واپس Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  اگر (Position == انتہا_2)
    واپس Position;
  // Check 7 bit c-printable - b-char.
  اگر (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    واپس Position + 1;

  // Check for valid UTF-8.
  اگر (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    اگر (   u8d.دوسرا_2 != 0
        && u8d.پہلا_2 != 0xFEFF
        && ( u8d.پہلا_2 == 0x85
          || ( u8d.پہلا_2 >= 0xA0
            && u8d.پہلا_2 <= 0xD7FF)
          || ( u8d.پہلا_2 >= 0xE000
            && u8d.پہلا_2 <= 0xFFFD)
          || ( u8d.پہلا_2 >= 0x10000
            && u8d.پہلا_2 <= 0x10FFFF)))
      واپس Position + u8d.دوسرا_2;
  }
  واپس Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  اگر (Position == انتہا_2)
    واپس Position;
  اگر (*Position == 0x0D) {
    اگر (Position + 1 != انتہا_2 && *(Position + 1) == 0x0A)
      واپس Position + 2;
    واپس Position + 1;
  }

  اگر (*Position == 0x0A)
    واپس Position + 1;
  واپس Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  اگر (Position == انتہا_2)
    واپس Position;
  اگر (*Position == ' ')
    واپس Position + 1;
  واپس Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  اگر (Position == انتہا_2)
    واپس Position;
  اگر (*Position == ' ' || *Position == '\t')
    واپس Position + 1;
  واپس Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  اگر (Position == انتہا_2)
    واپس Position;
  اگر (*Position == ' ' || *Position == '\t')
    واپس Position;
  واپس skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  جب (سچ) {
    StringRef::iterator i = (یہ->*Func)(Position);
    اگر (i == Position)
      توڑو;
    Position = i;
  }
  واپس Position;
}

خالی Scanner::advanceWhile(SkipWhileFunc Func) {
  خودکار Final = skip_while(Func, Current);
  عمود += Final - Current;
  Current = Final;
}

ساکن منطقی is_ns_hex_digit(ثابت حرف C) { واپس isAlnum(C); }

ساکن منطقی is_ns_لفظ_char(ثابت حرف C) { واپس C == '-' || isAlpha(C); }

خالی Scanner::جائزہ_ns_uri_char() {
  جب (سچ) {
    اگر (Current == انتہا_2)
      توڑو;
    اگر ((*Current == '%' && Current + 2 < انتہا_2 &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_لفظ_char(*Current) ||
        StringRef(Current, 1).ڈھونڈو_پہلا_کا("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++عمود;
    } ورنہ {
      توڑو;
    }
  }
}

منطقی Scanner::consume(uint32_t Expected) {
  اگر (Expected >= 0x80) {
    سیٹ_Error("Cannot consume non-ascii characters", Current);
    واپس جھوٹ;
  }
  اگر (Current == انتہا_2)
    واپس جھوٹ;
  اگر (uint8_t(*Current) >= 0x80) {
    سیٹ_Error("Cannot consume non-ascii characters", Current);
    واپس جھوٹ;
  }
  اگر (uint8_t(*Current) == Expected) {
    ++Current;
    ++عمود;
    واپس سچ;
  }
  واپس جھوٹ;
}

خالی Scanner::skip(uint32_t فاصلہ_2) {
  Current += فاصلہ_2;
  عمود += فاصلہ_2;
  أكد(Current <= انتہا_2 && "Skipped past the end");
}

منطقی Scanner::isBlankOrBreak(StringRef::iterator Position) {
  اگر (Position == انتہا_2)
    واپس جھوٹ;
  واپس *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

منطقی Scanner::is_Plain_محفوظ_Non_Blank(StringRef::iterator Position) {
  اگر (Position == انتہا_2 || isBlankOrBreak(Position))
    واپس جھوٹ;
  اگر (FlowLevel &&
      StringRef(Position, 1).ڈھونڈو_پہلا_کا(",[]{}") != StringRef::npos)
    واپس جھوٹ;
  واپس سچ;
}

منطقی Scanner::is_سطر_خالی_خانے(StringRef سطر) {
  برائے (ثابت خودکار *Position = سطر.begin(); Position != سطر.انتہا(); ++Position)
    اگر (!isBlankOrBreak(Position))
      واپس جھوٹ;
  واپس سچ;
}

منطقی Scanner::consume_سطر_Break_If_Present() {
  خودکار اگلا_2 = skip_b_break(Current);
  اگر (اگلا_2 == Current)
    واپس جھوٹ;
  عمود = 0;
  ++سطر;
  Current = اگلا_2;
  واپس سچ;
}

خالی Scanner::save_Simple_کلید_Candidate( Token_قطار_T::iterator Tok
                                    , غیرعلامتی At_عمود
                                    , منطقی IsRequired) {
  اگر (Is_Simple_کلید_Allowed) {
    Simple_کلید SK;
    SK.Tok = Tok;
    SK.سطر = سطر;
    SK.عمود = At_عمود;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_کلیدیں.ڈالو_عقب(SK);
  }
}

خالی Scanner::remove_Stale_Simple_کلید_Candidates() {
  برائے (Small_سمتیہ_Impl<Simple_کلید>::iterator i = Simple_کلیدیں.begin();
                                            i != Simple_کلیدیں.انتہا();) {
    اگر (i->سطر != سطر || i->عمود + 1024 < عمود) {
      اگر (i->IsRequired)
        سیٹ_Error( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_کلیدیں.erase(i);
    } ورنہ {
      ++i;
    }
  }
}

خالی Scanner::remove_Simple_کلید_Candidates_On_Flow_Level(غیرعلامتی Level) {
  اگر (!Simple_کلیدیں.خالی_خانے() && (Simple_کلیدیں.انتہا() - 1)->FlowLevel == Level)
    Simple_کلیدیں.نکالو_عقب();
}

منطقی Scanner::unrollIndent(صحیح مقصد_عمود) {
  Token T;
  // Indentation is ignored in flow.
  اگر (FlowLevel != 0)
    واپس سچ;

  جب (Indent > مقصد_عمود) {
    T.قسم_3 = Token::TK_بلاک_انتہا;
    T.Range = StringRef(Current, 1);
    Token_قطار.ڈالو_عقب(T);
    Indent = Indents.نکالو_عقب_val();
  }

  واپس سچ;
}

منطقی Scanner::rollIndent( صحیح مقصد_عمود
                        , Token::Token_قسم قسم_3
                        , Token_قطار_T::iterator داخل_کرو_نقطہ) {
  اگر (FlowLevel)
    واپس سچ;
  اگر (Indent < مقصد_عمود) {
    Indents.ڈالو_عقب(Indent);
    Indent = مقصد_عمود;

    Token T;
    T.قسم_3 = قسم_3;
    T.Range = StringRef(Current, 0);
    Token_قطار.داخل_کرو(داخل_کرو_نقطہ, T);
  }
  واپس سچ;
}

خالی Scanner::skipComment() {
  اگر (Current == انتہا_2 || *Current != '#')
    واپس;
  جب (سچ) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    اگر (I == Current)
      توڑو;
    Current = I;
    ++عمود;
  }
}

خالی Scanner::جائزہ_مقصد_اگلا_Token() {
  جب (سچ) {
    جب (Current != انتہا_2 && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    اگر (i == Current)
      توڑو;
    Current = i;
    ++سطر;
    عمود = 0;
    // New lines may start a simple key.
    اگر (!FlowLevel)
      Is_Simple_کلید_Allowed = سچ;
  }
}

منطقی Scanner::جائزہ_دھارا_آغاز() {
  Is_آغاز_کا_دھارا = جھوٹ;

  EncodingInfo EI = getUnicodeEncoding(current_داخلہ());

  Token T;
  T.قسم_3 = Token::TK_دھارا_آغاز;
  T.Range = StringRef(Current, EI.دوسرا_2);
  Token_قطار.ڈالو_عقب(T);
  Current += EI.دوسرا_2;
  واپس سچ;
}

منطقی Scanner::جائزہ_دھارا_انتہا() {
  // Force an ending new line if one isn't present.
  اگر (عمود != 0) {
    عمود = 0;
    ++سطر;
  }

  unrollIndent(-1);
  Simple_کلیدیں.clear();
  Is_Simple_کلید_Allowed = جھوٹ;
  Is_Adjacent_قدر_Allowed_اندر_Flow = جھوٹ;

  Token T;
  T.قسم_3 = Token::TK_دھارا_انتہا;
  T.Range = StringRef(Current, 0);
  Token_قطار.ڈالو_عقب(T);
  واپس سچ;
}

منطقی Scanner::جائزہ_Directive() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_کلیدیں.clear();
  Is_Simple_کلید_Allowed = جھوٹ;
  Is_Adjacent_قدر_Allowed_اندر_Flow = جھوٹ;

  StringRef::iterator آغاز = Current;
  consume('%');
  StringRef::iterator Name_آغاز = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(Name_آغاز, Current - Name_آغاز);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  اگر (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.قسم_3 = Token::TK_نسخہ_Directive;
    T.Range = StringRef(آغاز, Current - آغاز);
    Token_قطار.ڈالو_عقب(T);
    واپس سچ;
  } ورنہ اگر(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.قسم_3 = Token::TK_لیبل_Directive;
    T.Range = StringRef(آغاز, Current - آغاز);
    Token_قطار.ڈالو_عقب(T);
    واپس سچ;
  }
  واپس جھوٹ;
}

منطقی Scanner::جائزہ_Document_Indicator(منطقی Is_آغاز) {
  unrollIndent(-1);
  Simple_کلیدیں.clear();
  Is_Simple_کلید_Allowed = جھوٹ;
  Is_Adjacent_قدر_Allowed_اندر_Flow = جھوٹ;

  Token T;
  T.قسم_3 = Is_آغاز ? Token::TK_Document_آغاز : Token::TK_Document_انتہا;
  T.Range = StringRef(Current, 3);
  skip(3);
  Token_قطار.ڈالو_عقب(T);
  واپس سچ;
}

منطقی Scanner::جائزہ_Flow_Collection_آغاز(منطقی IsSequence) {
  Token T;
  T.قسم_3 = IsSequence ? Token::TK_Flow_Sequence_آغاز
                      : Token::TK_Flow_Mapping_آغاز;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڈالو_عقب(T);

  // [ and { may begin a simple key.
  save_Simple_کلید_Candidate(--Token_قطار.انتہا(), عمود - 1, جھوٹ);

  // And may also be followed by a simple key.
  Is_Simple_کلید_Allowed = سچ;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_قدر_Allowed_اندر_Flow = جھوٹ;
  ++FlowLevel;
  واپس سچ;
}

منطقی Scanner::جائزہ_Flow_Collection_انتہا(منطقی IsSequence) {
  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = جھوٹ;
  Is_Adjacent_قدر_Allowed_اندر_Flow = سچ;
  Token T;
  T.قسم_3 = IsSequence ? Token::TK_Flow_Sequence_انتہا
                      : Token::TK_Flow_Mapping_انتہا;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڈالو_عقب(T);
  اگر (FlowLevel)
    --FlowLevel;
  واپس سچ;
}

منطقی Scanner::جائزہ_Flow_اندراج() {
  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = سچ;
  Is_Adjacent_قدر_Allowed_اندر_Flow = جھوٹ;
  Token T;
  T.قسم_3 = Token::TK_Flow_اندراج;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڈالو_عقب(T);
  واپس سچ;
}

منطقی Scanner::جائزہ_بلاک_اندراج() {
  rollIndent(عمود, Token::TK_بلاک_Sequence_آغاز, Token_قطار.انتہا());
  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = سچ;
  Is_Adjacent_قدر_Allowed_اندر_Flow = جھوٹ;
  Token T;
  T.قسم_3 = Token::TK_بلاک_اندراج;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڈالو_عقب(T);
  واپس سچ;
}

منطقی Scanner::جائزہ_کلید() {
  اگر (!FlowLevel)
    rollIndent(عمود, Token::TK_بلاک_Mapping_آغاز, Token_قطار.انتہا());

  remove_Simple_کلید_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_کلید_Allowed = !FlowLevel;
  Is_Adjacent_قدر_Allowed_اندر_Flow = جھوٹ;

  Token T;
  T.قسم_3 = Token::TK_کلید;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڈالو_عقب(T);
  واپس سچ;
}

منطقی Scanner::جائزہ_قدر() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  اگر (!Simple_کلیدیں.خالی_خانے()) {
    Simple_کلید SK = Simple_کلیدیں.نکالو_عقب_val();
    Token T;
    T.قسم_3 = Token::TK_کلید;
    T.Range = SK.Tok->Range;
    Token_قطار_T::iterator i, e;
    برائے (i = Token_قطار.begin(), e = Token_قطار.انتہا(); i != e; ++i) {
      اگر (i == SK.Tok)
        توڑو;
    }
    اگر (i == e) {
      Failed = سچ;
      واپس جھوٹ;
    }
    i = Token_قطار.داخل_کرو(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.عمود, Token::TK_بلاک_Mapping_آغاز, i);

    Is_Simple_کلید_Allowed = جھوٹ;
  } ورنہ {
    اگر (!FlowLevel)
      rollIndent(عمود, Token::TK_بلاک_Mapping_آغاز, Token_قطار.انتہا());
    Is_Simple_کلید_Allowed = !FlowLevel;
  }
  Is_Adjacent_قدر_Allowed_اندر_Flow = جھوٹ;

  Token T;
  T.قسم_3 = Token::TK_قدر;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_قطار.ڈالو_عقب(T);
  واپس سچ;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE ساکن منطقی
wasEscaped(StringRef::iterator پہلا, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
ساکن منطقی wasEscaped(StringRef::iterator پہلا,
                       StringRef::iterator Position) {
  أكد(Position - 1 >= پہلا);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  جب (I >= پہلا && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  واپس (Position - 1 - I) % 2 == 1;
}

منطقی Scanner::جائزہ_Flow_Scalar(منطقی IsDoubleQuoted) {
  StringRef::iterator آغاز = Current;
  غیرعلامتی ستون_آغاز = عمود;
  اگر (IsDoubleQuoted) {
    کرو {
      ++Current;
      جب (Current != انتہا_2 && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } جب (   Current != انتہا_2
             && *(Current - 1) == '\\'
             && wasEscaped(آغاز + 1, Current));
  } ورنہ {
    skip(1);
    جب (Current != انتہا_2) {
      // Skip a ' followed by another '.
      اگر (Current + 1 < انتہا_2 && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        جاری;
      } ورنہ اگر (*Current == '\'')
        توڑو;
      StringRef::iterator i = skip_nb_char(Current);
      اگر (i == Current) {
        i = skip_b_break(Current);
        اگر (i == Current)
          توڑو;
        Current = i;
        عمود = 0;
        ++سطر;
      } ورنہ {
        اگر (i == انتہا_2)
          توڑو;
        Current = i;
        ++عمود;
      }
    }
  }

  اگر (Current == انتہا_2) {
    سیٹ_Error("Expected quote at end of scalar", Current);
    واپس جھوٹ;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.قسم_3 = Token::TK_Scalar;
  T.Range = StringRef(آغاز, Current - آغاز);
  Token_قطار.ڈالو_عقب(T);

  save_Simple_کلید_Candidate(--Token_قطار.انتہا(), ستون_آغاز, جھوٹ);

  Is_Simple_کلید_Allowed = جھوٹ;
  Is_Adjacent_قدر_Allowed_اندر_Flow = سچ;

  واپس سچ;
}

منطقی Scanner::جائزہ_Plain_Scalar() {
  StringRef::iterator آغاز = Current;
  غیرعلامتی ستون_آغاز = عمود;
  غیرعلامتی LeadingBlanks = 0;
  أكد(Indent >= -1 && "Indent must be >= -1 !");
  غیرعلامتی indent = static_cast<غیرعلامتی>(Indent + 1);
  جب (Current != انتہا_2) {
    اگر (*Current == '#')
      توڑو;

    جب (Current != انتہا_2 &&
           ((*Current != ':' && is_Plain_محفوظ_Non_Blank(Current)) ||
            (*Current == ':' && is_Plain_محفوظ_Non_Blank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      اگر (i == Current)
        توڑو;
      Current = i;
      ++عمود;
    }

    // Are we at the end?
    اگر (!isBlankOrBreak(Current))
      توڑو;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    جب (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      اگر (i != Tmp) {
        اگر (LeadingBlanks && (عمود < indent) && *Tmp == '\t') {
          سیٹ_Error("Found invalid tab character in indentation", Tmp);
          واپس جھوٹ;
        }
        Tmp = i;
        ++عمود;
      } ورنہ {
        i = skip_b_break(Tmp);
        اگر (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        عمود = 0;
        ++سطر;
      }
    }

    اگر (!FlowLevel && عمود < indent)
      توڑو;

    Current = Tmp;
  }
  اگر (آغاز == Current) {
    سیٹ_Error("Got empty plain scalar", آغاز);
    واپس جھوٹ;
  }
  Token T;
  T.قسم_3 = Token::TK_Scalar;
  T.Range = StringRef(آغاز, Current - آغاز);
  Token_قطار.ڈالو_عقب(T);

  // Plain scalars can be simple keys.
  save_Simple_کلید_Candidate(--Token_قطار.انتہا(), ستون_آغاز, جھوٹ);

  Is_Simple_کلید_Allowed = جھوٹ;
  Is_Adjacent_قدر_Allowed_اندر_Flow = جھوٹ;

  واپس سچ;
}

منطقی Scanner::جائزہ_Alias_Or_Anchor(منطقی IsAlias) {
  StringRef::iterator آغاز = Current;
  غیرعلامتی ستون_آغاز = عمود;
  skip(1);
  جب (Current != انتہا_2) {
    اگر (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      توڑو;
    StringRef::iterator i = skip_ns_char(Current);
    اگر (i == Current)
      توڑو;
    Current = i;
    ++عمود;
  }

  اگر (آغاز + 1 == Current) {
    سیٹ_Error("Got empty alias or anchor", آغاز);
    واپس جھوٹ;
  }

  Token T;
  T.قسم_3 = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(آغاز, Current - آغاز);
  Token_قطار.ڈالو_عقب(T);

  // Alias and anchors can be simple keys.
  save_Simple_کلید_Candidate(--Token_قطار.انتہا(), ستون_آغاز, جھوٹ);

  Is_Simple_کلید_Allowed = جھوٹ;
  Is_Adjacent_قدر_Allowed_اندر_Flow = جھوٹ;

  واپس سچ;
}

منطقی Scanner::جائزہ_بلاک_Scalar_Indicators(حرف &StyleIndicator,
                                        حرف &ChompingIndicator,
                                        غیرعلامتی &IndentIndicator,
                                        منطقی &Is_مکمل) {
  StyleIndicator = جائزہ_بلاک_Style_Indicator();
  اگر (!جائزہ_بلاک_Scalar_سرخی(ChompingIndicator, IndentIndicator, Is_مکمل))
    واپس جھوٹ;
  واپس سچ;
}

حرف Scanner::جائزہ_بلاک_Style_Indicator() {
  حرف Indicator = ' ';
  اگر (Current != انتہا_2 && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  واپس Indicator;
}

حرف Scanner::جائزہ_بلاک_Chomping_Indicator() {
  حرف Indicator = ' ';
  اگر (Current != انتہا_2 && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  واپس Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
ساکن غیرعلامتی get_Chomped_سطر_Breaks(حرف ChompingIndicator,
                                     غیرعلامتی سطر_Breaks, StringRef Str) {
  اگر (ChompingIndicator == '-') // Strip all line breaks.
    واپس 0;
  اگر (ChompingIndicator == '+') // Keep all line breaks.
    واپس سطر_Breaks;
  // Clip trailing lines.
  واپس Str.خالی_خانے() ? 0 : 1;
}

غیرعلامتی Scanner::جائزہ_بلاک_Indentation_Indicator() {
  غیرعلامتی Indent = 0;
  اگر (Current != انتہا_2 && (*Current >= '1' && *Current <= '9')) {
    Indent = غیرعلامتی(*Current - '0');
    skip(1);
  }
  واپس Indent;
}

منطقی Scanner::جائزہ_بلاک_Scalar_سرخی(حرف &ChompingIndicator,
                                    غیرعلامتی &IndentIndicator, منطقی &Is_مکمل) {
  خودکار آغاز = Current;

  ChompingIndicator = جائزہ_بلاک_Chomping_Indicator();
  IndentIndicator = جائزہ_بلاک_Indentation_Indicator();
  // Check for the chomping indicator once again.
  اگر (ChompingIndicator == ' ')
    ChompingIndicator = جائزہ_بلاک_Chomping_Indicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  اگر (Current == انتہا_2) { // EOF, we have an empty scalar.
    Token T;
    T.قسم_3 = Token::TK_بلاک_Scalar;
    T.Range = StringRef(آغاز, Current - آغاز);
    Token_قطار.ڈالو_عقب(T);
    Is_مکمل = سچ;
    واپس سچ;
  }

  اگر (!consume_سطر_Break_If_Present()) {
    سیٹ_Error("Expected a line break after block scalar header", Current);
    واپس جھوٹ;
  }
  واپس سچ;
}

منطقی Scanner::ڈھونڈو_بلاک_Scalar_Indent(غیرعلامتی &بلاک_Indent,
                                    غیرعلامتی بلاک_Exit_Indent,
                                    غیرعلامتی &سطر_Breaks, منطقی &Is_مکمل) {
  غیرعلامتی Max_All_Space_سطر_Characters = 0;
  StringRef::iterator Longest_All_Space_سطر;

  جب (سچ) {
    advanceWhile(&Scanner::skip_s_space);
    اگر (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      اگر (عمود <= بلاک_Exit_Indent) { // End of the block literal.
        Is_مکمل = سچ;
        واپس سچ;
      }
      // We found the block's indentation.
      بلاک_Indent = عمود;
      اگر (Max_All_Space_سطر_Characters > بلاک_Indent) {
        سیٹ_Error(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_سطر);
        واپس جھوٹ;
      }
      واپس سچ;
    }
    اگر (skip_b_break(Current) != Current &&
        عمود > Max_All_Space_سطر_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_سطر_Characters = عمود;
      Longest_All_Space_سطر = Current;
    }

    // Check for EOF.
    اگر (Current == انتہا_2) {
      Is_مکمل = سچ;
      واپس سچ;
    }

    اگر (!consume_سطر_Break_If_Present()) {
      Is_مکمل = سچ;
      واپس سچ;
    }
    ++سطر_Breaks;
  }
  واپس سچ;
}

منطقی Scanner::جائزہ_بلاک_Scalar_Indent(غیرعلامتی بلاک_Indent,
                                    غیرعلامتی بلاک_Exit_Indent, منطقی &Is_مکمل) {
  // Skip the indentation.
  جب (عمود < بلاک_Indent) {
    خودکار I = skip_s_space(Current);
    اگر (I == Current)
      توڑو;
    Current = I;
    ++عمود;
  }

  اگر (skip_nb_char(Current) == Current)
    واپس سچ;

  اگر (عمود <= بلاک_Exit_Indent) { // End of the block literal.
    Is_مکمل = سچ;
    واپس سچ;
  }

  اگر (عمود < بلاک_Indent) {
    اگر (Current != انتہا_2 && *Current == '#') { // Trailing comment.
      Is_مکمل = سچ;
      واپس سچ;
    }
    سیٹ_Error("A text line is less indented than the block scalar", Current);
    واپس جھوٹ;
  }
  واپس سچ; // A normal text line.
}

منطقی Scanner::جائزہ_بلاک_Scalar(منطقی IsLiteral) {
  أكد(*Current == '|' || *Current == '>');
  حرف StyleIndicator;
  حرف ChompingIndicator;
  غیرعلامتی بلاک_Indent;
  منطقی Is_مکمل = جھوٹ;
  اگر (!جائزہ_بلاک_Scalar_Indicators(StyleIndicator, ChompingIndicator, بلاک_Indent,
                                 Is_مکمل))
    واپس جھوٹ;
  اگر (Is_مکمل)
    واپس سچ;
  منطقی IsFolded = StyleIndicator == '>';

  ثابت خودکار *آغاز = Current;
  غیرعلامتی بلاک_Exit_Indent = Indent < 0 ? 0 : (غیرعلامتی)Indent;
  غیرعلامتی سطر_Breaks = 0;
  اگر (بلاک_Indent == 0) {
    اگر (!ڈھونڈو_بلاک_Scalar_Indent(بلاک_Indent, بلاک_Exit_Indent, سطر_Breaks,
                               Is_مکمل))
      واپس جھوٹ;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  جب (!Is_مکمل) {
    اگر (!جائزہ_بلاک_Scalar_Indent(بلاک_Indent, بلاک_Exit_Indent, Is_مکمل))
      واپس جھوٹ;
    اگر (Is_مکمل)
      توڑو;

    // Parse the current line.
    خودکار سطر_آغاز = Current;
    advanceWhile(&Scanner::skip_nb_char);
    اگر (سطر_آغاز != Current) {
      اگر (سطر_Breaks && IsFolded && !Scanner::is_سطر_خالی_خانے(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        اگر (سطر_Breaks == 1) {
          Str.append(سطر_Breaks,
                     is_سطر_خالی_خانے(StringRef(سطر_آغاز, Current - سطر_آغاز))
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
      Str.append(StringRef(سطر_آغاز, Current - سطر_آغاز));
      سطر_Breaks = 0;
    }

    // Check for EOF.
    اگر (Current == انتہا_2)
      توڑو;

    اگر (!consume_سطر_Break_If_Present())
      توڑو;
    ++سطر_Breaks;
  }

  اگر (Current == انتہا_2 && !سطر_Breaks)
    // Ensure that there is at least one line break before the end of file.
    سطر_Breaks = 1;
  Str.append(get_Chomped_سطر_Breaks(ChompingIndicator, سطر_Breaks, Str), '\n');

  // New lines may start a simple key.
  اگر (!FlowLevel)
    Is_Simple_کلید_Allowed = سچ;
  Is_Adjacent_قدر_Allowed_اندر_Flow = جھوٹ;

  Token T;
  T.قسم_3 = Token::TK_بلاک_Scalar;
  T.Range = StringRef(آغاز, Current - آغاز);
  T.قدر = std::string(Str);
  Token_قطار.ڈالو_عقب(T);
  واپس سچ;
}

منطقی Scanner::جائزہ_لیبل() {
  StringRef::iterator آغاز = Current;
  غیرعلامتی ستون_آغاز = عمود;
  skip(1); // Eat !.
  اگر (Current == انتہا_2 || isBlankOrBreak(Current)); // An empty tag.
  ورنہ اگر (*Current == '<') {
    skip(1);
    جائزہ_ns_uri_char();
    اگر (!consume('>'))
      واپس جھوٹ;
  } ورنہ {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.قسم_3 = Token::TK_لیبل;
  T.Range = StringRef(آغاز, Current - آغاز);
  Token_قطار.ڈالو_عقب(T);

  // Tags can be simple keys.
  save_Simple_کلید_Candidate(--Token_قطار.انتہا(), ستون_آغاز, جھوٹ);

  Is_Simple_کلید_Allowed = جھوٹ;
  Is_Adjacent_قدر_Allowed_اندر_Flow = جھوٹ;

  واپس سچ;
}

منطقی Scanner::fetch_مزید_علامات() {
  اگر (Is_آغاز_کا_دھارا)
    واپس جائزہ_دھارا_آغاز();

  جائزہ_مقصد_اگلا_Token();

  اگر (Current == انتہا_2)
    واپس جائزہ_دھارا_انتہا();

  remove_Stale_Simple_کلید_Candidates();

  unrollIndent(عمود);

  اگر (عمود == 0 && *Current == '%')
    واپس جائزہ_Directive();

  اگر (عمود == 0 && Current + 3 <= انتہا_2
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == انتہا_2 || isBlankOrBreak(Current + 3)))
    واپس جائزہ_Document_Indicator(سچ);

  اگر (عمود == 0 && Current + 3 <= انتہا_2
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == انتہا_2 || isBlankOrBreak(Current + 3)))
    واپس جائزہ_Document_Indicator(جھوٹ);

  اگر (*Current == '[')
    واپس جائزہ_Flow_Collection_آغاز(سچ);

  اگر (*Current == '{')
    واپس جائزہ_Flow_Collection_آغاز(جھوٹ);

  اگر (*Current == ']')
    واپس جائزہ_Flow_Collection_انتہا(سچ);

  اگر (*Current == '}')
    واپس جائزہ_Flow_Collection_انتہا(جھوٹ);

  اگر (*Current == ',')
    واپس جائزہ_Flow_اندراج();

  اگر (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == انتہا_2))
    واپس جائزہ_بلاک_اندراج();

  اگر (*Current == '?' && (Current + 1 == انتہا_2 || isBlankOrBreak(Current + 1)))
    واپس جائزہ_کلید();

  اگر (*Current == ':' &&
      (!is_Plain_محفوظ_Non_Blank(Current + 1) || Is_Adjacent_قدر_Allowed_اندر_Flow))
    واپس جائزہ_قدر();

  اگر (*Current == '*')
    واپس جائزہ_Alias_Or_Anchor(سچ);

  اگر (*Current == '&')
    واپس جائزہ_Alias_Or_Anchor(جھوٹ);

  اگر (*Current == '!')
    واپس جائزہ_لیبل();

  اگر (*Current == '|' && !FlowLevel)
    واپس جائزہ_بلاک_Scalar(سچ);

  اگر (*Current == '>' && !FlowLevel)
    واپس جائزہ_بلاک_Scalar(جھوٹ);

  اگر (*Current == '\'')
    واپس جائزہ_Flow_Scalar(جھوٹ);

  اگر (*Current == '"')
    واپس جائزہ_Flow_Scalar(سچ);

  // Get a plain scalar.
  StringRef پہلا_Char(Current, 1);
  اگر ((!isBlankOrBreak(Current) &&
       پہلا_Char.ڈھونڈو_پہلا_کا("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (پہلا_Char.ڈھونڈو_پہلا_کا("?:-") != StringRef::npos &&
       is_Plain_محفوظ_Non_Blank(Current + 1)))
    واپس جائزہ_Plain_Scalar();

  سیٹ_Error("Unrecognized character while tokenizing.", Current);
  واپس جھوٹ;
}

دھارا::دھارا(StringRef داخلہ, ماخذ_Mgr &SM, منطقی ShowColors,
               std::error_رمز *EC)
    : scanner(نیا Scanner(داخلہ, SM, ShowColors, EC)) {}

دھارا::دھارا(حافظہ_بفر_Ref داخلہ_بفر, ماخذ_Mgr &SM, منطقی ShowColors,
               std::error_رمز *EC)
    : scanner(نیا Scanner(داخلہ_بفر, SM, ShowColors, EC)) {}

دھارا::~دھارا() = طےشدہ;

منطقی دھارا::failed() { واپس scanner->failed(); }

خالی دھارا::printError(گرہ *N, ثابت Twine &پیغام_4, ماخذ_Mgr::Diag_قسم قسم_3) {
  printError(N ? N->get_ماخذ_Range() : SMRange(), پیغام_4, قسم_3);
}

خالی دھارا::printError(ثابت SMRange &Range, ثابت Twine &پیغام_4,
                        ماخذ_Mgr::Diag_قسم قسم_3) {
  scanner->printError(Range.آغاز, قسم_3, پیغام_4, Range);
}

document_iterator دھارا::begin() {
  اگر (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->get_اگلا();

  CurrentDoc.reset(نیا Document(*یہ));
  واپس document_iterator(CurrentDoc);
}

document_iterator دھارا::انتہا() {
  واپس document_iterator();
}

خالی دھارا::skip() {
  برائے (Document &Doc : *یہ)
    Doc.skip();
}

گرہ::گرہ(غیرعلامتی صحیح Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), لیبل(T) {
  SMLoc آغاز = SMLoc::get_From_اشارہ_گر(peek_اگلا().Range.begin());
  ماخذ_Range = SMRange(آغاز, آغاز);
}

std::string گرہ::get_Verbatim_لیبل() ثابت {
  StringRef Raw = get_Raw_لیبل();
  اگر (!Raw.خالی_خانے() && Raw != "!") {
    std::string Ret;
    اگر (Raw.ڈھونڈو_آخری_کا('!') == 0) {
      Ret = std::string(Doc->get_لیبل_Map().ڈھونڈو("!")->دوسرا_2);
      Ret += Raw.substr(1);
      واپس Ret;
    } ورنہ اگر (Raw.starts_with("!!")) {
      Ret = std::string(Doc->get_لیبل_Map().ڈھونڈو("!!")->دوسرا_2);
      Ret += Raw.substr(2);
      واپس Ret;
    } ورنہ {
      StringRef لیبل_Handle = Raw.substr(0, Raw.ڈھونڈو_آخری_کا('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->get_لیبل_Map().ڈھونڈو(لیبل_Handle);
      اگر (It != Doc->get_لیبل_Map().انتہا())
        Ret = std::string(It->دوسرا_2);
      ورنہ {
        Token T;
        T.قسم_3 = Token::TK_لیبل;
        T.Range = لیبل_Handle;
        سیٹ_Error(Twine("Unknown tag handle ") + لیبل_Handle, T);
      }
      Ret += Raw.substr(Raw.ڈھونڈو_آخری_کا('!') + 1);
      واپس Ret;
    }
  }

  انتخاب (getType()) {
  حالت NK_Null:
    واپس "tag:yaml.org,2002:null";
  حالت NK_Scalar:
  حالت NK_بلاک_Scalar:
    // TODO: Tag resolution.
    واپس "tag:yaml.org,2002:str";
  حالت NK_Mapping:
    واپس "tag:yaml.org,2002:map";
  حالت NK_Sequence:
    واپس "tag:yaml.org,2002:seq";
  }

  واپس "";
}

Token &گرہ::peek_اگلا() {
  واپس Doc->peek_اگلا();
}

Token گرہ::get_اگلا() {
  واپس Doc->get_اگلا();
}

گرہ *گرہ::parse_بلاک_گرہ() {
  واپس Doc->parse_بلاک_گرہ();
}

BumpPtrAllocator &گرہ::getAllocator() {
  واپس Doc->گرہ_Allocator;
}

خالی گرہ::سیٹ_Error(ثابت Twine &پیغام_4, Token &Tok) ثابت {
  Doc->سیٹ_Error(پیغام_4, Tok);
}

منطقی گرہ::failed() ثابت {
  واپس Doc->failed();
}

StringRef Scalar_گرہ::get_قدر(Small_سمتیہ_Impl<حرف> &Storage) ثابت {
  اگر (قدر[0] == '"')
    واپس get_Double_Quoted_قدر(قدر, Storage);
  اگر (قدر[0] == '\'')
    واپس get_Single_Quoted_قدر(قدر, Storage);
  واپس get_Plain_قدر(قدر, Storage);
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
ساکن StringRef
parse_Scalar_قدر(StringRef Unquoted_قدر, Small_سمتیہ_Impl<حرف> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, Small_سمتیہ_Impl<حرف> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_قدر.ڈھونڈو_پہلا_کا(LookupChars);
  اگر (I == StringRef::npos)
    واپس Unquoted_قدر;

  Storage.clear();
  Storage.reserve(Unquoted_قدر.سائز_2());
  حرف آخری_New_سطر_Added_As = '\0';
  برائے (; I != StringRef::npos; I = Unquoted_قدر.ڈھونڈو_پہلا_کا(LookupChars)) {
    اگر (Unquoted_قدر[I] != '\r' && Unquoted_قدر[I] != '\n') {
      llvm::append_range(Storage, Unquoted_قدر.take_front(I));
      Unquoted_قدر = UnescapeCallback(Unquoted_قدر.drop_front(I), Storage);
      آخری_New_سطر_Added_As = '\0';
      جاری;
    }
    اگر (size_t آخری_Non_SWhite = Unquoted_قدر.ڈھونڈو_آخری_not_کا(" \t", I);
        آخری_Non_SWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_قدر.take_front(آخری_Non_SWhite + 1));
      Storage.ڈالو_عقب(' ');
      آخری_New_سطر_Added_As = ' ';
    } ورنہ {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      انتخاب (آخری_New_سطر_Added_As) {
      حالت ' ':
        أكد(!Storage.خالی_خانے() && Storage.عقب() == ' ');
        Storage.عقب() = '\n';
        آخری_New_سطر_Added_As = '\n';
        توڑو;
      حالت '\n':
        أكد(!Storage.خالی_خانے() && Storage.عقب() == '\n');
        Storage.ڈالو_عقب('\n');
        توڑو;
      طےشدہ:
        Storage.ڈالو_عقب(' ');
        آخری_New_سطر_Added_As = ' ';
        توڑو;
      }
    }
    // Handle Windows-style EOL
    اگر (Unquoted_قدر.substr(I, 2) == "\r\n")
      I++;
    Unquoted_قدر = Unquoted_قدر.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_قدر);
  واپس StringRef(Storage.begin(), Storage.سائز_2());
}

StringRef
Scalar_گرہ::get_Double_Quoted_قدر(StringRef Raw_قدر,
                                 Small_سمتیہ_Impl<حرف> &Storage) ثابت {
  أكد(Raw_قدر.سائز_2() >= 2 && Raw_قدر.front() == '"' &&
         Raw_قدر.عقب() == '"');
  StringRef Unquoted_قدر = Raw_قدر.substr(1, Raw_قدر.سائز_2() - 2);

  خودکار UnescapeFunc = [یہ](StringRef Unquoted_قدر,
                             Small_سمتیہ_Impl<حرف> &Storage) {
    أكد(Unquoted_قدر.take_front(1) == "\\");
    اگر (Unquoted_قدر.سائز_2() == 1) {
      Token T;
      T.Range = Unquoted_قدر;
      سیٹ_Error("Unrecognized escape code", T);
      Storage.clear();
      واپس StringRef();
    }
    Unquoted_قدر = Unquoted_قدر.drop_front(1);
    انتخاب (Unquoted_قدر[0]) {
    طےشدہ: {
      Token T;
      T.Range = Unquoted_قدر.take_front(1);
      سیٹ_Error("Unrecognized escape code", T);
      Storage.clear();
      واپس StringRef();
    }
    حالت '\r':
      // Shrink the Windows-style EOL.
      اگر (Unquoted_قدر.سائز_2() >= 2 && Unquoted_قدر[1] == '\n')
        Unquoted_قدر = Unquoted_قدر.drop_front(1);
      [[fallthrough]];
    حالت '\n':
      واپس Unquoted_قدر.drop_front(1).ltrim(" \t");
    حالت '0':
      Storage.ڈالو_عقب(0x00);
      توڑو;
    حالت 'a':
      Storage.ڈالو_عقب(0x07);
      توڑو;
    حالت 'b':
      Storage.ڈالو_عقب(0x08);
      توڑو;
    حالت 't':
    حالت 0x09:
      Storage.ڈالو_عقب(0x09);
      توڑو;
    حالت 'n':
      Storage.ڈالو_عقب(0x0A);
      توڑو;
    حالت 'v':
      Storage.ڈالو_عقب(0x0B);
      توڑو;
    حالت 'f':
      Storage.ڈالو_عقب(0x0C);
      توڑو;
    حالت 'r':
      Storage.ڈالو_عقب(0x0D);
      توڑو;
    حالت 'e':
      Storage.ڈالو_عقب(0x1B);
      توڑو;
    حالت ' ':
      Storage.ڈالو_عقب(0x20);
      توڑو;
    حالت '"':
      Storage.ڈالو_عقب(0x22);
      توڑو;
    حالت '/':
      Storage.ڈالو_عقب(0x2F);
      توڑو;
    حالت '\\':
      Storage.ڈالو_عقب(0x5C);
      توڑو;
    حالت 'N':
      encodeUTF8(0x85, Storage);
      توڑو;
    حالت '_':
      encodeUTF8(0xA0, Storage);
      توڑو;
    حالت 'L':
      encodeUTF8(0x2028, Storage);
      توڑو;
    حالت 'P':
      encodeUTF8(0x2029, Storage);
      توڑو;
    حالت 'x': {
      اگر (Unquoted_قدر.سائز_2() < 3)
        // TODO: Report error.
        توڑو;
      غیرعلامتی صحیح Unicode_Scalar_قدر;
      اگر (Unquoted_قدر.substr(1, 2).getAsInteger(16, Unicode_Scalar_قدر))
        // TODO: Report error.
        Unicode_Scalar_قدر = 0xFFFD;
      encodeUTF8(Unicode_Scalar_قدر, Storage);
      واپس Unquoted_قدر.drop_front(3);
    }
    حالت 'u': {
      اگر (Unquoted_قدر.سائز_2() < 5)
        // TODO: Report error.
        توڑو;
      غیرعلامتی صحیح Unicode_Scalar_قدر;
      اگر (Unquoted_قدر.substr(1, 4).getAsInteger(16, Unicode_Scalar_قدر))
        // TODO: Report error.
        Unicode_Scalar_قدر = 0xFFFD;
      encodeUTF8(Unicode_Scalar_قدر, Storage);
      واپس Unquoted_قدر.drop_front(5);
    }
    حالت 'U': {
      اگر (Unquoted_قدر.سائز_2() < 9)
        // TODO: Report error.
        توڑو;
      غیرعلامتی صحیح Unicode_Scalar_قدر;
      اگر (Unquoted_قدر.substr(1, 8).getAsInteger(16, Unicode_Scalar_قدر))
        // TODO: Report error.
        Unicode_Scalar_قدر = 0xFFFD;
      encodeUTF8(Unicode_Scalar_قدر, Storage);
      واپس Unquoted_قدر.drop_front(9);
    }
    }
    واپس Unquoted_قدر.drop_front(1);
  };

  واپس parse_Scalar_قدر(Unquoted_قدر, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_گرہ::get_Single_Quoted_قدر(StringRef Raw_قدر,
                                           Small_سمتیہ_Impl<حرف> &Storage) {
  أكد(Raw_قدر.سائز_2() >= 2 && Raw_قدر.front() == '\'' &&
         Raw_قدر.عقب() == '\'');
  StringRef Unquoted_قدر = Raw_قدر.substr(1, Raw_قدر.سائز_2() - 2);

  خودکار UnescapeFunc = [](StringRef Unquoted_قدر,
                         Small_سمتیہ_Impl<حرف> &Storage) {
    أكد(Unquoted_قدر.take_front(2) == "''");
    Storage.ڈالو_عقب('\'');
    واپس Unquoted_قدر.drop_front(2);
  };

  واپس parse_Scalar_قدر(Unquoted_قدر, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_گرہ::get_Plain_قدر(StringRef Raw_قدر,
                                    Small_سمتیہ_Impl<حرف> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_قدر = Raw_قدر.rtrim("\r\n \t");
  واپس parse_Scalar_قدر(Raw_قدر, Storage, "\r\n", خالی_اشاریہ);
}

گرہ *کلید_قدر_گرہ::get_کلید() {
  اگر (کلید)
    واپس کلید;
  // Handle implicit null keys.
  {
    Token &t = peek_اگلا();
    اگر (   t.قسم_3 == Token::TK_بلاک_انتہا
        || t.قسم_3 == Token::TK_قدر
        || t.قسم_3 == Token::TK_Error) {
      واپس کلید = نیا (getAllocator()) Null_گرہ(Doc);
    }
    اگر (t.قسم_3 == Token::TK_کلید)
      get_اگلا(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peek_اگلا();
  اگر (t.قسم_3 == Token::TK_بلاک_انتہا || t.قسم_3 == Token::TK_قدر) {
    واپس کلید = نیا (getAllocator()) Null_گرہ(Doc);
  }

  // We've got a normal key.
  واپس کلید = parse_بلاک_گرہ();
}

گرہ *کلید_قدر_گرہ::get_قدر() {
  اگر (قدر)
    واپس قدر;

  اگر (گرہ* کلید = get_کلید())
    کلید->skip();
  ورنہ {
    سیٹ_Error("Null key in Key Value.", peek_اگلا());
    واپس قدر = نیا (getAllocator()) Null_گرہ(Doc);
  }

  اگر (failed())
    واپس قدر = نیا (getAllocator()) Null_گرہ(Doc);

  // Handle implicit null values.
  {
    Token &t = peek_اگلا();
    اگر (   t.قسم_3 == Token::TK_بلاک_انتہا
        || t.قسم_3 == Token::TK_Flow_Mapping_انتہا
        || t.قسم_3 == Token::TK_کلید
        || t.قسم_3 == Token::TK_Flow_اندراج
        || t.قسم_3 == Token::TK_Error) {
      واپس قدر = نیا (getAllocator()) Null_گرہ(Doc);
    }

    اگر (t.قسم_3 != Token::TK_قدر) {
      سیٹ_Error("Unexpected token in Key Value.", t);
      واپس قدر = نیا (getAllocator()) Null_گرہ(Doc);
    }
    get_اگلا(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peek_اگلا();
  اگر (t.قسم_3 == Token::TK_بلاک_انتہا || t.قسم_3 == Token::TK_کلید) {
    واپس قدر = نیا (getAllocator()) Null_گرہ(Doc);
  }

  // We got a normal value.
  واپس قدر = parse_بلاک_گرہ();
}

خالی Mapping_گرہ::increment() {
  اگر (failed()) {
    Is_At_انتہا = سچ;
    Current_اندراج = خالی_اشاریہ;
    واپس;
  }
  اگر (Current_اندراج) {
    Current_اندراج->skip();
    اگر (Type == MT_Inline) {
      Is_At_انتہا = سچ;
      Current_اندراج = خالی_اشاریہ;
      واپس;
    }
  }
  Token T = peek_اگلا();
  اگر (T.قسم_3 == Token::TK_کلید || T.قسم_3 == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    Current_اندراج = نیا (getAllocator()) کلید_قدر_گرہ(Doc);
  } ورنہ اگر (Type == MT_بلاک) {
    انتخاب (T.قسم_3) {
    حالت Token::TK_بلاک_انتہا:
      get_اگلا();
      Is_At_انتہا = سچ;
      Current_اندراج = خالی_اشاریہ;
      توڑو;
    طےشدہ:
      سیٹ_Error("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    حالت Token::TK_Error:
      Is_At_انتہا = سچ;
      Current_اندراج = خالی_اشاریہ;
    }
  } ورنہ {
    انتخاب (T.قسم_3) {
    حالت Token::TK_Flow_اندراج:
      // Eat the flow entry and recurse.
      get_اگلا();
      واپس increment();
    حالت Token::TK_Flow_Mapping_انتہا:
      get_اگلا();
      [[fallthrough]];
    حالت Token::TK_Error:
      // Set this to end iterator.
      Is_At_انتہا = سچ;
      Current_اندراج = خالی_اشاریہ;
      توڑو;
    طےشدہ:
      سیٹ_Error( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      Is_At_انتہا = سچ;
      Current_اندراج = خالی_اشاریہ;
    }
  }
}

خالی Sequence_گرہ::increment() {
  اگر (failed()) {
    Is_At_انتہا = سچ;
    Current_اندراج = خالی_اشاریہ;
    واپس;
  }
  اگر (Current_اندراج)
    Current_اندراج->skip();
  Token T = peek_اگلا();
  اگر (SeqType == ST_بلاک) {
    انتخاب (T.قسم_3) {
    حالت Token::TK_بلاک_اندراج:
      get_اگلا();
      Current_اندراج = parse_بلاک_گرہ();
      اگر (!Current_اندراج) { // An error occurred.
        Is_At_انتہا = سچ;
        Current_اندراج = خالی_اشاریہ;
      }
      توڑو;
    حالت Token::TK_بلاک_انتہا:
      get_اگلا();
      Is_At_انتہا = سچ;
      Current_اندراج = خالی_اشاریہ;
      توڑو;
    طےشدہ:
      سیٹ_Error( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    حالت Token::TK_Error:
      Is_At_انتہا = سچ;
      Current_اندراج = خالی_اشاریہ;
    }
  } ورنہ اگر (SeqType == ST_Indentless) {
    انتخاب (T.قسم_3) {
    حالت Token::TK_بلاک_اندراج:
      get_اگلا();
      Current_اندراج = parse_بلاک_گرہ();
      اگر (!Current_اندراج) { // An error occurred.
        Is_At_انتہا = سچ;
        Current_اندراج = خالی_اشاریہ;
      }
      توڑو;
    طےشدہ:
    حالت Token::TK_Error:
      Is_At_انتہا = سچ;
      Current_اندراج = خالی_اشاریہ;
    }
  } ورنہ اگر (SeqType == ST_Flow) {
    انتخاب (T.قسم_3) {
    حالت Token::TK_Flow_اندراج:
      // Eat the flow entry and recurse.
      get_اگلا();
      Was_پچھلا_Token_Flow_اندراج = سچ;
      واپس increment();
    حالت Token::TK_Flow_Sequence_انتہا:
      get_اگلا();
      [[fallthrough]];
    حالت Token::TK_Error:
      // Set this to end iterator.
      Is_At_انتہا = سچ;
      Current_اندراج = خالی_اشاریہ;
      توڑو;
    حالت Token::TK_دھارا_انتہا:
    حالت Token::TK_Document_انتہا:
    حالت Token::TK_Document_آغاز:
      سیٹ_Error("Could not find closing ]!", T);
      // Set this to end iterator.
      Is_At_انتہا = سچ;
      Current_اندراج = خالی_اشاریہ;
      توڑو;
    طےشدہ:
      اگر (!Was_پچھلا_Token_Flow_اندراج) {
        سیٹ_Error("Expected , between entries!", T);
        Is_At_انتہا = سچ;
        Current_اندراج = خالی_اشاریہ;
        توڑو;
      }
      // Otherwise it must be a flow entry.
      Current_اندراج = parse_بلاک_گرہ();
      اگر (!Current_اندراج) {
        Is_At_انتہا = سچ;
      }
      Was_پچھلا_Token_Flow_اندراج = جھوٹ;
      توڑو;
    }
  }
}

Document::Document(دھارا &S) : دھارا_2(S), جڑ_2(خالی_اشاریہ) {
  // Tag maps starts with two default mappings.
  لیبل_Map["!"] = "!";
  لیبل_Map["!!"] = "tag:yaml.org,2002:";

  اگر (parseDirectives())
    expectToken(Token::TK_Document_آغاز);
  Token &T = peek_اگلا();
  اگر (T.قسم_3 == Token::TK_Document_آغاز)
    get_اگلا();
}

منطقی Document::skip()  {
  اگر (دھارا_2.scanner->failed())
    واپس جھوٹ;
  اگر (!جڑ_2 && !get_جڑ())
    واپس جھوٹ;
  جڑ_2->skip();
  Token &T = peek_اگلا();
  اگر (T.قسم_3 == Token::TK_دھارا_انتہا)
    واپس جھوٹ;
  اگر (T.قسم_3 == Token::TK_Document_انتہا) {
    get_اگلا();
    واپس skip();
  }
  واپس سچ;
}

Token &Document::peek_اگلا() {
  واپس دھارا_2.scanner->peek_اگلا();
}

Token Document::get_اگلا() {
  واپس دھارا_2.scanner->get_اگلا();
}

خالی Document::سیٹ_Error(ثابت Twine &پیغام_3, Token &Location) ثابت {
  دھارا_2.scanner->سیٹ_Error(پیغام_3, Location.Range.begin());
}

منطقی Document::failed() ثابت {
  واپس دھارا_2.scanner->failed();
}

گرہ *Document::parse_بلاک_گرہ() {
  Token T = peek_اگلا();
  // Handle properties.
  Token AnchorInfo;
  Token لیبل_Info;
parse_property:
  انتخاب (T.قسم_3) {
  حالت Token::TK_Alias:
    get_اگلا();
    واپس نیا (گرہ_Allocator) Alias_گرہ(دھارا_2.CurrentDoc, T.Range.substr(1));
  حالت Token::TK_Anchor:
    اگر (AnchorInfo.قسم_3 == Token::TK_Anchor) {
      سیٹ_Error("Already encountered an anchor for this node!", T);
      واپس خالی_اشاریہ;
    }
    AnchorInfo = get_اگلا(); // Consume TK_Anchor.
    T = peek_اگلا();
    جاؤ parse_property;
  حالت Token::TK_لیبل:
    اگر (لیبل_Info.قسم_3 == Token::TK_لیبل) {
      سیٹ_Error("Already encountered a tag for this node!", T);
      واپس خالی_اشاریہ;
    }
    لیبل_Info = get_اگلا(); // Consume TK_Tag.
    T = peek_اگلا();
    جاؤ parse_property;
  طےشدہ:
    توڑو;
  }

  انتخاب (T.قسم_3) {
  حالت Token::TK_بلاک_اندراج:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    واپس نیا (گرہ_Allocator) Sequence_گرہ( دھارا_2.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , لیبل_Info.Range
                                           , Sequence_گرہ::ST_Indentless);
  حالت Token::TK_بلاک_Sequence_آغاز:
    get_اگلا();
    واپس نیا (گرہ_Allocator)
      Sequence_گرہ( دھارا_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , لیبل_Info.Range
                  , Sequence_گرہ::ST_بلاک);
  حالت Token::TK_بلاک_Mapping_آغاز:
    get_اگلا();
    واپس نیا (گرہ_Allocator)
      Mapping_گرہ( دھارا_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , لیبل_Info.Range
                 , Mapping_گرہ::MT_بلاک);
  حالت Token::TK_Flow_Sequence_آغاز:
    get_اگلا();
    واپس نیا (گرہ_Allocator)
      Sequence_گرہ( دھارا_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , لیبل_Info.Range
                  , Sequence_گرہ::ST_Flow);
  حالت Token::TK_Flow_Mapping_آغاز:
    get_اگلا();
    واپس نیا (گرہ_Allocator)
      Mapping_گرہ( دھارا_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , لیبل_Info.Range
                 , Mapping_گرہ::MT_Flow);
  حالت Token::TK_Scalar:
    get_اگلا();
    واپس نیا (گرہ_Allocator)
      Scalar_گرہ( دھارا_2.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , لیبل_Info.Range
                , T.Range);
  حالت Token::TK_بلاک_Scalar: {
    get_اگلا();
    StringRef NullTerminatedStr(T.قدر.c_str(), T.قدر.لمبائی_2() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(گرہ_Allocator).drop_عقب();
    واپس نیا (گرہ_Allocator)
        بلاک_Scalar_گرہ(دھارا_2.CurrentDoc, AnchorInfo.Range.substr(1),
                        لیبل_Info.Range, StrCopy, T.Range);
  }
  حالت Token::TK_کلید:
    // Don't eat the TK_Key, KeyValueNode expects it.
    واپس نیا (گرہ_Allocator)
      Mapping_گرہ( دھارا_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , لیبل_Info.Range
                 , Mapping_گرہ::MT_Inline);
  حالت Token::TK_Document_آغاز:
  حالت Token::TK_Document_انتہا:
  حالت Token::TK_دھارا_انتہا:
  طےشدہ:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    واپس نیا (گرہ_Allocator) Null_گرہ(دھارا_2.CurrentDoc);
  حالت Token::TK_Flow_Mapping_انتہا:
  حالت Token::TK_Flow_Sequence_انتہا:
  حالت Token::TK_Flow_اندراج: {
    اگر (جڑ_2 && (isa<Mapping_گرہ>(جڑ_2) || isa<Sequence_گرہ>(جڑ_2)))
      واپس نیا (گرہ_Allocator) Null_گرہ(دھارا_2.CurrentDoc);

    سیٹ_Error("Unexpected token", T);
    واپس خالی_اشاریہ;
  }
  حالت Token::TK_Error:
    واپس خالی_اشاریہ;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  واپس خالی_اشاریہ;
}

منطقی Document::parseDirectives() {
  منطقی isDirective = جھوٹ;
  جب (سچ) {
    Token T = peek_اگلا();
    اگر (T.قسم_3 == Token::TK_لیبل_Directive) {
      parseTAGDirective();
      isDirective = سچ;
    } ورنہ اگر (T.قسم_3 == Token::TK_نسخہ_Directive) {
      parseYAMLDirective();
      isDirective = سچ;
    } ورنہ {
      توڑو;
    }
  }
  واپس isDirective;
}

خالی Document::parseYAMLDirective() {
  get_اگلا(); // Eat %YAML <version>
}

خالی Document::parseTAGDirective() {
  Token لیبل = get_اگلا(); // %TAG <handle> <prefix>
  StringRef T = لیبل.Range;
  // Strip %TAG
  T = T.substr(T.ڈھونڈو_پہلا_کا(" \t")).ltrim(" \t");
  std::size_t Handle_انتہا = T.ڈھونڈو_پہلا_کا(" \t");
  StringRef لیبل_Handle = T.substr(0, Handle_انتہا);
  StringRef لیبل_سابقہ = T.substr(Handle_انتہا).ltrim(" \t");
  لیبل_Map[لیبل_Handle] = لیبل_سابقہ;
}

منطقی Document::expectToken(صحیح TK) {
  Token T = get_اگلا();
  اگر (T.قسم_3 != TK) {
    سیٹ_Error("Unexpected token", T);
    واپس جھوٹ;
  }
  واپس سچ;
}
