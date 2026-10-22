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

#अन्तर्भाव "llvm/Support/YAMLParser.h"
#अन्तर्भाव "llvm/ADT/AllocatorList.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/ADT/Twine.h"
#अन्तर्भाव "llvm/Support/Compiler.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/SMLoc.h"
#अन्तर्भाव "llvm/Support/SourceMgr.h"
#अन्तर्भाव "llvm/Support/Unicode.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cassert>
#अन्तर्भाव <cstddef>
#अन्तर्भाव <cstdint>
#अन्तर्भाव <map>
#अन्तर्भाव <memory>
#अन्तर्भाव <string>
#अन्तर्भाव <system_error>
#अन्तर्भाव <utility>

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश yaml;

गणना UnicodeEncodingForm {
  UEF_UTF32_LE, ///< UTF-32 Little Endian
  UEF_UTF32_BE, ///< UTF-32 Big Endian
  UEF_UTF16_LE, ///< UTF-16 Little Endian
  UEF_UTF16_BE, ///< UTF-16 Big Endian
  UEF_UTF8,     ///< UTF-8 or ascii.
  UEF_Unknown   ///< Not a valid Unicode encoding.
};

/// EncodingInfo - Holds the encoding type and length of the byte order mark if
///                it exists. Length is in {0, 2, 3, 4}.
उपयुज्य EncodingInfo = std::pair<UnicodeEncodingForm, अचिह्नित>;

/// getUnicodeEncoding - Reads up to the first 4 bytes to determine the Unicode
///                      encoding form of \a Input.
///
/// @param Input A string of length 0 or more.
/// @returns An EncodingInfo indicating the Unicode encoding form of the input
///          and how long the byte order mark is if one exists.
स्थिर EncodingInfo getUnicodeEncoding(StringRef निवेशः) {
  यदि (निवेशः.रिक्तानि())
    प्रत्यागम {UEF_Unknown, 0};

  विकल्प (uint8_t(निवेशः[0])) {
  पक्ष 0x00:
    यदि (निवेशः.आकार_2() >= 4) {
      यदि (  निवेशः[1] == 0
         && uint8_t(निवेशः[2]) == 0xFE
         && uint8_t(निवेशः[3]) == 0xFF)
        प्रत्यागम {UEF_UTF32_BE, 4};
      यदि (निवेशः[1] == 0 && निवेशः[2] == 0 && निवेशः[3] != 0)
        प्रत्यागम {UEF_UTF32_BE, 0};
    }

    यदि (निवेशः.आकार_2() >= 2 && निवेशः[1] != 0)
      प्रत्यागम {UEF_UTF16_BE, 0};
    प्रत्यागम {UEF_Unknown, 0};
  पक्ष 0xFF:
    यदि (  निवेशः.आकार_2() >= 4
       && uint8_t(निवेशः[1]) == 0xFE
       && निवेशः[2] == 0
       && निवेशः[3] == 0)
      प्रत्यागम {UEF_UTF32_LE, 4};

    यदि (निवेशः.आकार_2() >= 2 && uint8_t(निवेशः[1]) == 0xFE)
      प्रत्यागम {UEF_UTF16_LE, 2};
    प्रत्यागम {UEF_Unknown, 0};
  पक्ष 0xFE:
    यदि (निवेशः.आकार_2() >= 2 && uint8_t(निवेशः[1]) == 0xFF)
      प्रत्यागम {UEF_UTF16_BE, 2};
    प्रत्यागम {UEF_Unknown, 0};
  पक्ष 0xEF:
    यदि (  निवेशः.आकार_2() >= 3
       && uint8_t(निवेशः[1]) == 0xBB
       && uint8_t(निवेशः[2]) == 0xBF)
      प्रत्यागम {UEF_UTF8, 3};
    प्रत्यागम {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  यदि (निवेशः.आकार_2() >= 4 && निवेशः[1] == 0 && निवेशः[2] == 0 && निवेशः[3] == 0)
    प्रत्यागम {UEF_UTF32_LE, 0};

  यदि (निवेशः.आकार_2() >= 2 && निवेशः[1] == 0)
    प्रत्यागम {UEF_UTF16_LE, 0};

  प्रत्यागम {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
शून्य ग्रन्थिः::anchor() {}
शून्य Null_ग्रन्थिः::anchor() {}
शून्य Scalar_ग्रन्थिः::anchor() {}
शून्य खण्डम्_Scalar_ग्रन्थिः::anchor() {}
शून्य कुञ्चिका_मानम्_ग्रन्थिः::anchor() {}
शून्य Mapping_ग्रन्थिः::anchor() {}
शून्य Sequence_ग्रन्थिः::anchor() {}
शून्य Alias_ग्रन्थिः::anchor() {}

नामाकाश llvm {
नामाकाश yaml {

/// Token - A single YAML token.
संरचना Token {
  गणना Token_विधा {
    TK_Error, // Uninitialized token.
    TK_प्रवाहः_Start,
    TK_प्रवाहः_अन्तः,
    TK_संस्करणम्_Directive,
    TK_चिह्नकम्_Directive,
    TK_DocumentStart,
    TK_Document_अन्तः,
    TK_खण्डम्_प्रविष्टिः,
    TK_खण्डम्_अन्तः,
    TK_खण्डम्_Sequence_Start,
    TK_खण्डम्_Mapping_Start,
    TK_Flow_प्रविष्टिः,
    TK_FlowSequenceStart,
    TK_Flow_Sequence_अन्तः,
    TK_FlowMappingStart,
    TK_Flow_Mapping_अन्तः,
    TK_कुञ्चिका,
    TK_मानम्,
    TK_Scalar,
    TK_खण्डम्_Scalar,
    TK_Alias,
    TK_Anchor,
    TK_चिह्नकम्
  } विधा_2 = TK_Error;

  /// A string of length 0 or more whose begin() points to the logical location
  /// of the token in the input.
  StringRef Range;

  /// The value of a block scalar node.
  std::string मानम्;

  Token() = स्वभाव;
};

} // end namespace yaml
} // end namespace llvm

उपयुज्य TokenQueueT = BumpPtrList<Token>;

नामाकाश {

/// This struct is used to track simple keys.
///
/// Simple keys are handled by creating an entry in SimpleKeys for each Token
/// which could legally be the start of a simple key. When peekNext is called,
/// if the Token To be returned is referenced by a SimpleKey, we continue
/// tokenizing until that potential simple key has either been found to not be
/// a simple key (we moved on to the next line or went further than 1024 chars).
/// Or when we run into a Value, and then insert a Key token (and possibly
/// others) before the SimpleKey's Tok.
संरचना Simple_कुञ्चिका {
  TokenQueueT::iterator Tok;
  अचिह्नित Column = 0;
  अचिह्नित पङ्क्तिः = 0;
  अचिह्नित FlowLevel = 0;
  तार्किक IsRequired = असत्य;

  तार्किक संक्रियकः ==(ध्रुव Simple_कुञ्चिका &Other) {
    प्रत्यागम Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
उपयुज्य UTF8Decoded = std::pair<uint32_t, अचिह्नित>;

स्थिर UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator अन्तः_2 = Range.अन्तः();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  यदि (Position < अन्तः_2 && (*Position & 0x80) == 0) {
    प्रत्यागम {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  यदि (Position + 1 < अन्तः_2 && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    यदि (codepoint >= 0x80)
      प्रत्यागम {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  यदि (Position + 2 < अन्तः_2 && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    यदि (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      प्रत्यागम {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  यदि (Position + 3 < अन्तः_2 && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    यदि (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      प्रत्यागम {codepoint, 4};
  }
  प्रत्यागम {0, 0};
}

नामाकाश llvm {
नामाकाश yaml {

/// Scans YAML tokens from a MemoryBuffer.
वर्ग Scanner {
सार्वजनिक:
  Scanner(StringRef निवेशः, SourceMgr &SM, तार्किक ShowColors = सत्य,
          std::error_संहिता *EC = शून्यसूचकः);
  Scanner(स्मृतिः_बफर_Ref बफर, SourceMgr &SM_, तार्किक ShowColors = सत्य,
          std::error_संहिता *EC = शून्यसूचकः);

  /// Parse the next token and return it without popping it.
  Token &peek_अग्र();

  /// Parse the next token and pop it from the queue.
  Token get_अग्र();

  शून्य printError(SMLoc Loc, SourceMgr::Diag_विधा विधा_2, ध्रुव Twine &सन्देशः_3,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.Print_सन्देशः(Loc, विधा_2, सन्देशः_3, Ranges, /* FixIts= */ {}, ShowColors);
  }

  शून्य समुच्चयः_Error(ध्रुव Twine &सन्देशः_3, StringRef::iterator Position) {
    यदि (Position >= अन्तः_2)
      Position = अन्तः_2 - 1;

    // propagate the error if possible
    यदि (EC)
      *EC = make_error_संहिता(std::errc::invalid_तर्कः);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    यदि (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, सन्देशः_3);
    Failed = सत्य;
  }

  /// Returns true if an error occurred while parsing.
  तार्किक failed() {
    प्रत्यागम Failed;
  }

गुप्त:
  शून्य init(स्मृतिः_बफर_Ref बफर);

  StringRef current_निवेशः() {
    प्रत्यागम StringRef(Current, अन्तः_2 - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    प्रत्यागम ::decodeUTF8(StringRef(Position, अन्तः_2 - Position));
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

  उपयुज्य SkipWhileFunc = StringRef::iterator (Scanner::*)(StringRef::iterator);

  /// Skip minimal well-formed code unit subsequences until Func
  ///        returns its input.
  ///
  /// @returns The code unit after the last minimal well-formed code unit
  ///          subsequence that Func accepted.
  StringRef::iterator skip_while( SkipWhileFunc Func
                                , StringRef::iterator Position);

  /// Skip minimal well-formed code unit subsequences until Func returns its
  /// input.
  शून्य advanceWhile(SkipWhileFunc Func);

  /// Scan ns-uri-char[39]s starting at Cur.
  ///
  /// This updates Cur and Column while scanning.
  शून्य अवलोकनम्_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  तार्किक consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  शून्य skip(uint32_t Distance);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  तार्किक isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  तार्किक isPlainSafeNonBlank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  तार्किक is_पङ्क्तिः_रिक्तानि(StringRef पङ्क्तिः);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  तार्किक consume_पङ्क्तिः_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  शून्य save_Simple_कुञ्चिका_Candidate( TokenQueueT::iterator Tok
                             , अचिह्नित AtColumn
                             , तार्किक IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  शून्य remove_Stale_Simple_कुञ्चिका_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  शून्य remove_Simple_कुञ्चिका_Candidates_On_Flow_Level(अचिह्नित Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  तार्किक unrollIndent(पूर्णाङ्क गन्तव्यम्_Column);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  तार्किक rollIndent( पूर्णाङ्क गन्तव्यम्_Column
                 , Token::Token_विधा विधा_2
                 , TokenQueueT::iterator निवेशय_Point);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  शून्य skipComment();

  /// Skip whitespace and comments until the start of the next token.
  शून्य अवलोकनम्_गन्तव्यम्_अग्र_Token();

  /// Must be the first token generated.
  तार्किक अवलोकनम्_प्रवाहः_Start();

  /// Generate tokens needed to close out the stream.
  तार्किक अवलोकनम्_प्रवाहः_अन्तः();

  /// Scan a %BLAH directive.
  तार्किक अवलोकनम्_Directive();

  /// Scan a ... or ---.
  तार्किक अवलोकनम्_Document_Indicator(तार्किक IsStart);

  /// Scan a [ or { and generate the proper flow collection start token.
  तार्किक अवलोकनम्_Flow_Collection_Start(तार्किक IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  तार्किक अवलोकनम्_Flow_Collection_अन्तः(तार्किक IsSequence);

  /// Scan the , that separates entries in a flow collection.
  तार्किक अवलोकनम्_Flow_प्रविष्टिः();

  /// Scan the - that starts block sequence entries.
  तार्किक अवलोकनम्_खण्डम्_प्रविष्टिः();

  /// Scan an explicit ? indicating a key.
  तार्किक अवलोकनम्_कुञ्चिका();

  /// Scan an explicit : indicating a value.
  तार्किक अवलोकनम्_मानम्();

  /// Scan a quoted scalar.
  तार्किक अवलोकनम्_Flow_Scalar(तार्किक IsDoubleQuoted);

  /// Scan an unquoted scalar.
  तार्किक अवलोकनम्_Plain_Scalar();

  /// Scan an Alias or Anchor starting with * or &.
  तार्किक अवलोकनम्_Alias_Or_Anchor(तार्किक IsAlias);

  /// Scan a block scalar starting with | or >.
  तार्किक अवलोकनम्_खण्डम्_Scalar(तार्किक IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  तार्किक अवलोकनम्_खण्डम्_Scalar_Indicators(वर्ण &StyleIndicator, वर्ण &ChompingIndicator,
                                 अचिह्नित &IndentIndicator, तार्किक &Is_समाप्तम्);

  /// Scan a style indicator in a block scalar header.
  वर्ण अवलोकनम्_खण्डम्_Style_Indicator();

  /// Scan a chomping indicator in a block scalar header.
  वर्ण अवलोकनम्_खण्डम्_Chomping_Indicator();

  /// Scan an indentation indicator in a block scalar header.
  अचिह्नित अवलोकनम्_खण्डम्_Indentation_Indicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  तार्किक अवलोकनम्_खण्डम्_Scalar_शीर्षकम्(वर्ण &ChompingIndicator, अचिह्नित &IndentIndicator,
                             तार्किक &Is_समाप्तम्);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  तार्किक find_खण्डम्_Scalar_Indent(अचिह्नित &खण्डम्_Indent, अचिह्नित खण्डम्_Exit_Indent,
                             अचिह्नित &पङ्क्तिः_Breaks, तार्किक &Is_समाप्तम्);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  तार्किक अवलोकनम्_खण्डम्_Scalar_Indent(अचिह्नित खण्डम्_Indent, अचिह्नित खण्डम्_Exit_Indent,
                             तार्किक &Is_समाप्तम्);

  /// Scan a tag of the form !stuff.
  तार्किक अवलोकनम्_चिह्नकम्();

  /// Dispatch to the next scanning function based on \a *Cur.
  तार्किक fetch_अधिकम्_Tokens();

  /// The SourceMgr used for diagnostics and buffer management.
  SourceMgr &SM;

  /// The original input.
  स्मृतिः_बफर_Ref निवेशः_बफर;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator अन्तः_2;

  /// Current YAML indentation level in spaces.
  पूर्णाङ्क Indent;

  /// Current column number in Unicode code points.
  अचिह्नित Column;

  /// Current line number.
  अचिह्नित पङ्क्तिः;

  /// How deep we are in flow style containers. 0 Means at block level.
  अचिह्नित FlowLevel;

  /// Are we at the start of the stream?
  तार्किक Is_Start_सम्बन्धि_प्रवाहः;

  /// Can the next token be the start of a simple key?
  तार्किक Is_Simple_कुञ्चिका_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  तार्किक Is_Adjacent_मानम्_Allowed_अन्तर्_Flow;

  /// True if an error has occurred.
  तार्किक Failed;

  /// Should colors be used when printing out the diagnostic messages?
  तार्किक ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  TokenQueueT TokenQueue;

  /// Indentation levels.
  Small_सदिशम्<पूर्णाङ्क, 4> Indents;

  /// Potential simple keys.
  Small_सदिशम्<Simple_कुञ्चिका, 4> Simple_कुञ्चिकाः;

  std::error_संहिता *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
स्थिर शून्य encodeUTF8( uint32_t Unicode_Scalar_मानम्
                      , Small_सदिशम्_Impl<वर्ण> &परिणाम) {
  यदि (Unicode_Scalar_मानम् <= 0x7F) {
    परिणाम.प्रक्षिप_पृष्ठभागः(Unicode_Scalar_मानम् & 0x7F);
  } अन्यथा यदि (Unicode_Scalar_मानम् <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((Unicode_Scalar_मानम् & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Unicode_Scalar_मानम् & 0x3F);
    परिणाम.प्रक्षिप_पृष्ठभागः(FirstByte);
    परिणाम.प्रक्षिप_पृष्ठभागः(SecondByte);
  } अन्यथा यदि (Unicode_Scalar_मानम् <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((Unicode_Scalar_मानम् & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_मानम् & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Unicode_Scalar_मानम् & 0x3F);
    परिणाम.प्रक्षिप_पृष्ठभागः(FirstByte);
    परिणाम.प्रक्षिप_पृष्ठभागः(SecondByte);
    परिणाम.प्रक्षिप_पृष्ठभागः(ThirdByte);
  } अन्यथा यदि (Unicode_Scalar_मानम् <= 0x10FFFF) {
    uint8_t FirstByte = 0xF0 | ((Unicode_Scalar_मानम् & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_मानम् & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Unicode_Scalar_मानम् & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_मानम् & 0x3F);
    परिणाम.प्रक्षिप_पृष्ठभागः(FirstByte);
    परिणाम.प्रक्षिप_पृष्ठभागः(SecondByte);
    परिणाम.प्रक्षिप_पृष्ठभागः(ThirdByte);
    परिणाम.प्रक्षिप_पृष्ठभागः(FourthByte);
  }
}

तार्किक yaml::dumpTokens(StringRef निवेशः, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(निवेशः, SM);
  यावत् (सत्य) {
    Token T = scanner.get_अग्र();
    विकल्प (T.विधा_2) {
    पक्ष Token::TK_प्रवाहः_Start:
      OS << "Stream-Start: ";
      भङ्ग;
    पक्ष Token::TK_प्रवाहः_अन्तः:
      OS << "Stream-End: ";
      भङ्ग;
    पक्ष Token::TK_संस्करणम्_Directive:
      OS << "Version-Directive: ";
      भङ्ग;
    पक्ष Token::TK_चिह्नकम्_Directive:
      OS << "Tag-Directive: ";
      भङ्ग;
    पक्ष Token::TK_DocumentStart:
      OS << "Document-Start: ";
      भङ्ग;
    पक्ष Token::TK_Document_अन्तः:
      OS << "Document-End: ";
      भङ्ग;
    पक्ष Token::TK_खण्डम्_प्रविष्टिः:
      OS << "Block-Entry: ";
      भङ्ग;
    पक्ष Token::TK_खण्डम्_अन्तः:
      OS << "Block-End: ";
      भङ्ग;
    पक्ष Token::TK_खण्डम्_Sequence_Start:
      OS << "Block-Sequence-Start: ";
      भङ्ग;
    पक्ष Token::TK_खण्डम्_Mapping_Start:
      OS << "Block-Mapping-Start: ";
      भङ्ग;
    पक्ष Token::TK_Flow_प्रविष्टिः:
      OS << "Flow-Entry: ";
      भङ्ग;
    पक्ष Token::TK_FlowSequenceStart:
      OS << "Flow-Sequence-Start: ";
      भङ्ग;
    पक्ष Token::TK_Flow_Sequence_अन्तः:
      OS << "Flow-Sequence-End: ";
      भङ्ग;
    पक्ष Token::TK_FlowMappingStart:
      OS << "Flow-Mapping-Start: ";
      भङ्ग;
    पक्ष Token::TK_Flow_Mapping_अन्तः:
      OS << "Flow-Mapping-End: ";
      भङ्ग;
    पक्ष Token::TK_कुञ्चिका:
      OS << "Key: ";
      भङ्ग;
    पक्ष Token::TK_मानम्:
      OS << "Value: ";
      भङ्ग;
    पक्ष Token::TK_Scalar:
      OS << "Scalar: ";
      भङ्ग;
    पक्ष Token::TK_खण्डम्_Scalar:
      OS << "Block Scalar: ";
      भङ्ग;
    पक्ष Token::TK_Alias:
      OS << "Alias: ";
      भङ्ग;
    पक्ष Token::TK_Anchor:
      OS << "Anchor: ";
      भङ्ग;
    पक्ष Token::TK_चिह्नकम्:
      OS << "Tag: ";
      भङ्ग;
    पक्ष Token::TK_Error:
      भङ्ग;
    }
    OS << T.Range << "\n";
    यदि (T.विधा_2 == Token::TK_प्रवाहः_अन्तः)
      भङ्ग;
    अन्यथा यदि (T.विधा_2 == Token::TK_Error)
      प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य;
}

तार्किक yaml::अवलोकनम्_Tokens(StringRef निवेशः) {
  SourceMgr SM;
  Scanner scanner(निवेशः, SM);
  यावत् (सत्य) {
    Token T = scanner.get_अग्र();
    यदि (T.विधा_2 == Token::TK_प्रवाहः_अन्तः)
      भङ्ग;
    अन्यथा यदि (T.विधा_2 == Token::TK_Error)
      प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य;
}

std::string yaml::escape(StringRef निवेशः, तार्किक EscapePrintable) {
  std::string Escaped_निवेशः;
  प्रति (StringRef::iterator i = निवेशः.begin(), e = निवेशः.अन्तः(); i != e; ++i) {
    यदि (*i == '\\')
      Escaped_निवेशः += "\\\\";
    अन्यथा यदि (*i == '"')
      Escaped_निवेशः += "\\\"";
    अन्यथा यदि (*i == 0)
      Escaped_निवेशः += "\\0";
    अन्यथा यदि (*i == 0x07)
      Escaped_निवेशः += "\\a";
    अन्यथा यदि (*i == 0x08)
      Escaped_निवेशः += "\\b";
    अन्यथा यदि (*i == 0x09)
      Escaped_निवेशः += "\\t";
    अन्यथा यदि (*i == 0x0A)
      Escaped_निवेशः += "\\n";
    अन्यथा यदि (*i == 0x0B)
      Escaped_निवेशः += "\\v";
    अन्यथा यदि (*i == 0x0C)
      Escaped_निवेशः += "\\f";
    अन्यथा यदि (*i == 0x0D)
      Escaped_निवेशः += "\\r";
    अन्यथा यदि (*i == 0x1B)
      Escaped_निवेशः += "\\e";
    अन्यथा यदि ((अचिह्नित वर्ण)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_निवेशः += "\\x" + std::string(2 - HexStr.आकार_2(), '0') + HexStr;
    } अन्यथा यदि (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_मानम्
        = decodeUTF8(StringRef(i, निवेशः.अन्तः() - i));
      यदि (Unicode_Scalar_मानम्.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_निवेशः, Val);
        // FIXME: Error reporting.
        प्रत्यागम Escaped_निवेशः;
      }
      यदि (Unicode_Scalar_मानम्.first == 0x85)
        Escaped_निवेशः += "\\N";
      अन्यथा यदि (Unicode_Scalar_मानम्.first == 0xA0)
        Escaped_निवेशः += "\\_";
      अन्यथा यदि (Unicode_Scalar_मानम्.first == 0x2028)
        Escaped_निवेशः += "\\L";
      अन्यथा यदि (Unicode_Scalar_मानम्.first == 0x2029)
        Escaped_निवेशः += "\\P";
      अन्यथा यदि (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_मानम्.first))
        Escaped_निवेशः += StringRef(i, Unicode_Scalar_मानम्.second);
      अन्यथा {
        std::string HexStr = utohexstr(Unicode_Scalar_मानम्.first);
        यदि (HexStr.आकार_2() <= 2)
          Escaped_निवेशः += "\\x" + std::string(2 - HexStr.आकार_2(), '0') + HexStr;
        अन्यथा यदि (HexStr.आकार_2() <= 4)
          Escaped_निवेशः += "\\u" + std::string(4 - HexStr.आकार_2(), '0') + HexStr;
        अन्यथा यदि (HexStr.आकार_2() <= 8)
          Escaped_निवेशः += "\\U" + std::string(8 - HexStr.आकार_2(), '0') + HexStr;
      }
      i += Unicode_Scalar_मानम्.second - 1;
    } अन्यथा {
      Escaped_निवेशः.प्रक्षिप_पृष्ठभागः(*i);
    }
  }
  प्रत्यागम Escaped_निवेशः;
}

std::optional<तार्किक> yaml::parseBool(StringRef S) {
  विकल्प (S.आकार_2()) {
  पक्ष 1:
    विकल्प (S.front()) {
    पक्ष 'y':
    पक्ष 'Y':
      प्रत्यागम सत्य;
    पक्ष 'n':
    पक्ष 'N':
      प्रत्यागम असत्य;
    स्वभाव:
      प्रत्यागम std::nullopt;
    }
  पक्ष 2:
    विकल्प (S.front()) {
    पक्ष 'O':
      यदि (S[1] == 'N') // ON
        प्रत्यागम सत्य;
      [[fallthrough]];
    पक्ष 'o':
      यदि (S[1] == 'n') //[Oo]n
        प्रत्यागम सत्य;
      प्रत्यागम std::nullopt;
    पक्ष 'N':
      यदि (S[1] == 'O') // NO
        प्रत्यागम असत्य;
      [[fallthrough]];
    पक्ष 'n':
      यदि (S[1] == 'o') //[Nn]o
        प्रत्यागम असत्य;
      प्रत्यागम std::nullopt;
    स्वभाव:
      प्रत्यागम std::nullopt;
    }
  पक्ष 3:
    विकल्प (S.front()) {
    पक्ष 'O':
      यदि (S.drop_front() == "FF") // OFF
        प्रत्यागम असत्य;
      [[fallthrough]];
    पक्ष 'o':
      यदि (S.drop_front() == "ff") //[Oo]ff
        प्रत्यागम असत्य;
      प्रत्यागम std::nullopt;
    पक्ष 'Y':
      यदि (S.drop_front() == "ES") // YES
        प्रत्यागम सत्य;
      [[fallthrough]];
    पक्ष 'y':
      यदि (S.drop_front() == "es") //[Yy]es
        प्रत्यागम सत्य;
      प्रत्यागम std::nullopt;
    स्वभाव:
      प्रत्यागम std::nullopt;
    }
  पक्ष 4:
    विकल्प (S.front()) {
    पक्ष 'T':
      यदि (S.drop_front() == "RUE") // TRUE
        प्रत्यागम सत्य;
      [[fallthrough]];
    पक्ष 't':
      यदि (S.drop_front() == "rue") //[Tt]rue
        प्रत्यागम सत्य;
      प्रत्यागम std::nullopt;
    स्वभाव:
      प्रत्यागम std::nullopt;
    }
  पक्ष 5:
    विकल्प (S.front()) {
    पक्ष 'F':
      यदि (S.drop_front() == "ALSE") // FALSE
        प्रत्यागम असत्य;
      [[fallthrough]];
    पक्ष 'f':
      यदि (S.drop_front() == "alse") //[Ff]alse
        प्रत्यागम असत्य;
      प्रत्यागम std::nullopt;
    स्वभाव:
      प्रत्यागम std::nullopt;
    }
  स्वभाव:
    प्रत्यागम std::nullopt;
  }
}

Scanner::Scanner(StringRef निवेशः, SourceMgr &sm, तार्किक ShowColors,
                 std::error_संहिता *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(स्मृतिः_बफर_Ref(निवेशः, "YAML"));
}

Scanner::Scanner(स्मृतिः_बफर_Ref बफर, SourceMgr &SM_, तार्किक ShowColors,
                 std::error_संहिता *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(बफर);
}

शून्य Scanner::init(स्मृतिः_बफर_Ref बफर) {
  निवेशः_बफर = बफर;
  Current = निवेशः_बफर.get_बफर_Start();
  अन्तः_2 = निवेशः_बफर.get_बफर_अन्तः();
  Indent = -1;
  Column = 0;
  पङ्क्तिः = 0;
  FlowLevel = 0;
  Is_Start_सम्बन्धि_प्रवाहः = सत्य;
  Is_Simple_कुञ्चिका_Allowed = सत्य;
  Is_Adjacent_मानम्_Allowed_अन्तर्_Flow = असत्य;
  Failed = असत्य;
  std::unique_ptr<स्मृतिः_बफर> निवेशः_बफर_Owner =
      स्मृतिः_बफर::get_Mem_बफर(बफर, /*RequiresNullTerminator=*/असत्य);
  SM.सङ्कलय_New_Source_बफर(std::move(निवेशः_बफर_Owner), SMLoc());
}

Token &Scanner::peek_अग्र() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  तार्किक Need_अधिकम् = असत्य;
  यावत् (सत्य) {
    यदि (TokenQueue.रिक्तानि() || Need_अधिकम्) {
      यदि (!fetch_अधिकम्_Tokens()) {
        TokenQueue.clear();
        Simple_कुञ्चिकाः.clear();
        TokenQueue.प्रक्षिप_पृष्ठभागः(Token());
        प्रत्यागम TokenQueue.front();
      }
    }
    प्रतिज्ञा(!TokenQueue.रिक्तानि() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_कुञ्चिका_Candidates();
    Simple_कुञ्चिका SK;
    SK.Tok = TokenQueue.begin();
    यदि (!is_contained(Simple_कुञ्चिकाः, SK))
      भङ्ग;
    अन्यथा
      Need_अधिकम् = सत्य;
  }
  प्रत्यागम TokenQueue.front();
}

Token Scanner::get_अग्र() {
  Token Ret = peek_अग्र();
  // TokenQueue can be empty if there was an error getting the next token.
  यदि (!TokenQueue.रिक्तानि())
    TokenQueue.निष्कासय_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  यदि (TokenQueue.रिक्तानि())
    TokenQueue.resetAlloc();

  प्रत्यागम Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  यदि (Position == अन्तः_2)
    प्रत्यागम Position;
  // Check 7 bit c-printable - b-char.
  यदि (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    प्रत्यागम Position + 1;

  // Check for valid UTF-8.
  यदि (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    यदि (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      प्रत्यागम Position + u8d.second;
  }
  प्रत्यागम Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  यदि (Position == अन्तः_2)
    प्रत्यागम Position;
  यदि (*Position == 0x0D) {
    यदि (Position + 1 != अन्तः_2 && *(Position + 1) == 0x0A)
      प्रत्यागम Position + 2;
    प्रत्यागम Position + 1;
  }

  यदि (*Position == 0x0A)
    प्रत्यागम Position + 1;
  प्रत्यागम Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  यदि (Position == अन्तः_2)
    प्रत्यागम Position;
  यदि (*Position == ' ')
    प्रत्यागम Position + 1;
  प्रत्यागम Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  यदि (Position == अन्तः_2)
    प्रत्यागम Position;
  यदि (*Position == ' ' || *Position == '\t')
    प्रत्यागम Position + 1;
  प्रत्यागम Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  यदि (Position == अन्तः_2)
    प्रत्यागम Position;
  यदि (*Position == ' ' || *Position == '\t')
    प्रत्यागम Position;
  प्रत्यागम skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  यावत् (सत्य) {
    StringRef::iterator i = (एतत्->*Func)(Position);
    यदि (i == Position)
      भङ्ग;
    Position = i;
  }
  प्रत्यागम Position;
}

शून्य Scanner::advanceWhile(SkipWhileFunc Func) {
  स्वचालित Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

स्थिर तार्किक is_ns_hex_digit(ध्रुव वर्ण C) { प्रत्यागम isAlnum(C); }

स्थिर तार्किक is_ns_word_char(ध्रुव वर्ण C) { प्रत्यागम C == '-' || isAlpha(C); }

शून्य Scanner::अवलोकनम्_ns_uri_char() {
  यावत् (सत्य) {
    यदि (Current == अन्तः_2)
      भङ्ग;
    यदि ((*Current == '%' && Current + 2 < अन्तः_2 &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_सम्बन्धि("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } अन्यथा {
      भङ्ग;
    }
  }
}

तार्किक Scanner::consume(uint32_t Expected) {
  यदि (Expected >= 0x80) {
    समुच्चयः_Error("Cannot consume non-ascii characters", Current);
    प्रत्यागम असत्य;
  }
  यदि (Current == अन्तः_2)
    प्रत्यागम असत्य;
  यदि (uint8_t(*Current) >= 0x80) {
    समुच्चयः_Error("Cannot consume non-ascii characters", Current);
    प्रत्यागम असत्य;
  }
  यदि (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

शून्य Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  प्रतिज्ञा(Current <= अन्तः_2 && "Skipped past the end");
}

तार्किक Scanner::isBlankOrBreak(StringRef::iterator Position) {
  यदि (Position == अन्तः_2)
    प्रत्यागम असत्य;
  प्रत्यागम *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

तार्किक Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  यदि (Position == अन्तः_2 || isBlankOrBreak(Position))
    प्रत्यागम असत्य;
  यदि (FlowLevel &&
      StringRef(Position, 1).find_first_सम्बन्धि(",[]{}") != StringRef::npos)
    प्रत्यागम असत्य;
  प्रत्यागम सत्य;
}

तार्किक Scanner::is_पङ्क्तिः_रिक्तानि(StringRef पङ्क्तिः) {
  प्रति (ध्रुव स्वचालित *Position = पङ्क्तिः.begin(); Position != पङ्क्तिः.अन्तः(); ++Position)
    यदि (!isBlankOrBreak(Position))
      प्रत्यागम असत्य;
  प्रत्यागम सत्य;
}

तार्किक Scanner::consume_पङ्क्तिः_Break_If_Present() {
  स्वचालित अग्र_2 = skip_b_break(Current);
  यदि (अग्र_2 == Current)
    प्रत्यागम असत्य;
  Column = 0;
  ++पङ्क्तिः;
  Current = अग्र_2;
  प्रत्यागम सत्य;
}

शून्य Scanner::save_Simple_कुञ्चिका_Candidate( TokenQueueT::iterator Tok
                                    , अचिह्नित AtColumn
                                    , तार्किक IsRequired) {
  यदि (Is_Simple_कुञ्चिका_Allowed) {
    Simple_कुञ्चिका SK;
    SK.Tok = Tok;
    SK.पङ्क्तिः = पङ्क्तिः;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_कुञ्चिकाः.प्रक्षिप_पृष्ठभागः(SK);
  }
}

शून्य Scanner::remove_Stale_Simple_कुञ्चिका_Candidates() {
  प्रति (Small_सदिशम्_Impl<Simple_कुञ्चिका>::iterator i = Simple_कुञ्चिकाः.begin();
                                            i != Simple_कुञ्चिकाः.अन्तः();) {
    यदि (i->पङ्क्तिः != पङ्क्तिः || i->Column + 1024 < Column) {
      यदि (i->IsRequired)
        समुच्चयः_Error( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_कुञ्चिकाः.erase(i);
    } अन्यथा {
      ++i;
    }
  }
}

शून्य Scanner::remove_Simple_कुञ्चिका_Candidates_On_Flow_Level(अचिह्नित Level) {
  यदि (!Simple_कुञ्चिकाः.रिक्तानि() && (Simple_कुञ्चिकाः.अन्तः() - 1)->FlowLevel == Level)
    Simple_कुञ्चिकाः.निष्कासय_पृष्ठभागः();
}

तार्किक Scanner::unrollIndent(पूर्णाङ्क गन्तव्यम्_Column) {
  Token T;
  // Indentation is ignored in flow.
  यदि (FlowLevel != 0)
    प्रत्यागम सत्य;

  यावत् (Indent > गन्तव्यम्_Column) {
    T.विधा_2 = Token::TK_खण्डम्_अन्तः;
    T.Range = StringRef(Current, 1);
    TokenQueue.प्रक्षिप_पृष्ठभागः(T);
    Indent = Indents.निष्कासय_पृष्ठभागः_val();
  }

  प्रत्यागम सत्य;
}

तार्किक Scanner::rollIndent( पूर्णाङ्क गन्तव्यम्_Column
                        , Token::Token_विधा विधा_2
                        , TokenQueueT::iterator निवेशय_Point) {
  यदि (FlowLevel)
    प्रत्यागम सत्य;
  यदि (Indent < गन्तव्यम्_Column) {
    Indents.प्रक्षिप_पृष्ठभागः(Indent);
    Indent = गन्तव्यम्_Column;

    Token T;
    T.विधा_2 = विधा_2;
    T.Range = StringRef(Current, 0);
    TokenQueue.निवेशय(निवेशय_Point, T);
  }
  प्रत्यागम सत्य;
}

शून्य Scanner::skipComment() {
  यदि (Current == अन्तः_2 || *Current != '#')
    प्रत्यागम;
  यावत् (सत्य) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    यदि (I == Current)
      भङ्ग;
    Current = I;
    ++Column;
  }
}

शून्य Scanner::अवलोकनम्_गन्तव्यम्_अग्र_Token() {
  यावत् (सत्य) {
    यावत् (Current != अन्तः_2 && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    यदि (i == Current)
      भङ्ग;
    Current = i;
    ++पङ्क्तिः;
    Column = 0;
    // New lines may start a simple key.
    यदि (!FlowLevel)
      Is_Simple_कुञ्चिका_Allowed = सत्य;
  }
}

तार्किक Scanner::अवलोकनम्_प्रवाहः_Start() {
  Is_Start_सम्बन्धि_प्रवाहः = असत्य;

  EncodingInfo EI = getUnicodeEncoding(current_निवेशः());

  Token T;
  T.विधा_2 = Token::TK_प्रवाहः_Start;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.प्रक्षिप_पृष्ठभागः(T);
  Current += EI.second;
  प्रत्यागम सत्य;
}

तार्किक Scanner::अवलोकनम्_प्रवाहः_अन्तः() {
  // Force an ending new line if one isn't present.
  यदि (Column != 0) {
    Column = 0;
    ++पङ्क्तिः;
  }

  unrollIndent(-1);
  Simple_कुञ्चिकाः.clear();
  Is_Simple_कुञ्चिका_Allowed = असत्य;
  Is_Adjacent_मानम्_Allowed_अन्तर्_Flow = असत्य;

  Token T;
  T.विधा_2 = Token::TK_प्रवाहः_अन्तः;
  T.Range = StringRef(Current, 0);
  TokenQueue.प्रक्षिप_पृष्ठभागः(T);
  प्रत्यागम सत्य;
}

तार्किक Scanner::अवलोकनम्_Directive() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_कुञ्चिकाः.clear();
  Is_Simple_कुञ्चिका_Allowed = असत्य;
  Is_Adjacent_मानम्_Allowed_अन्तर्_Flow = असत्य;

  StringRef::iterator Start = Current;
  consume('%');
  StringRef::iterator NameStart = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(NameStart, Current - NameStart);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  यदि (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.विधा_2 = Token::TK_संस्करणम्_Directive;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.प्रक्षिप_पृष्ठभागः(T);
    प्रत्यागम सत्य;
  } अन्यथा यदि(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.विधा_2 = Token::TK_चिह्नकम्_Directive;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.प्रक्षिप_पृष्ठभागः(T);
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

तार्किक Scanner::अवलोकनम्_Document_Indicator(तार्किक IsStart) {
  unrollIndent(-1);
  Simple_कुञ्चिकाः.clear();
  Is_Simple_कुञ्चिका_Allowed = असत्य;
  Is_Adjacent_मानम्_Allowed_अन्तर्_Flow = असत्य;

  Token T;
  T.विधा_2 = IsStart ? Token::TK_DocumentStart : Token::TK_Document_अन्तः;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.प्रक्षिप_पृष्ठभागः(T);
  प्रत्यागम सत्य;
}

तार्किक Scanner::अवलोकनम्_Flow_Collection_Start(तार्किक IsSequence) {
  Token T;
  T.विधा_2 = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.प्रक्षिप_पृष्ठभागः(T);

  // [ and { may begin a simple key.
  save_Simple_कुञ्चिका_Candidate(--TokenQueue.अन्तः(), Column - 1, असत्य);

  // And may also be followed by a simple key.
  Is_Simple_कुञ्चिका_Allowed = सत्य;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_मानम्_Allowed_अन्तर्_Flow = असत्य;
  ++FlowLevel;
  प्रत्यागम सत्य;
}

तार्किक Scanner::अवलोकनम्_Flow_Collection_अन्तः(तार्किक IsSequence) {
  remove_Simple_कुञ्चिका_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_कुञ्चिका_Allowed = असत्य;
  Is_Adjacent_मानम्_Allowed_अन्तर्_Flow = सत्य;
  Token T;
  T.विधा_2 = IsSequence ? Token::TK_Flow_Sequence_अन्तः
                      : Token::TK_Flow_Mapping_अन्तः;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.प्रक्षिप_पृष्ठभागः(T);
  यदि (FlowLevel)
    --FlowLevel;
  प्रत्यागम सत्य;
}

तार्किक Scanner::अवलोकनम्_Flow_प्रविष्टिः() {
  remove_Simple_कुञ्चिका_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_कुञ्चिका_Allowed = सत्य;
  Is_Adjacent_मानम्_Allowed_अन्तर्_Flow = असत्य;
  Token T;
  T.विधा_2 = Token::TK_Flow_प्रविष्टिः;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.प्रक्षिप_पृष्ठभागः(T);
  प्रत्यागम सत्य;
}

तार्किक Scanner::अवलोकनम्_खण्डम्_प्रविष्टिः() {
  rollIndent(Column, Token::TK_खण्डम्_Sequence_Start, TokenQueue.अन्तः());
  remove_Simple_कुञ्चिका_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_कुञ्चिका_Allowed = सत्य;
  Is_Adjacent_मानम्_Allowed_अन्तर्_Flow = असत्य;
  Token T;
  T.विधा_2 = Token::TK_खण्डम्_प्रविष्टिः;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.प्रक्षिप_पृष्ठभागः(T);
  प्रत्यागम सत्य;
}

तार्किक Scanner::अवलोकनम्_कुञ्चिका() {
  यदि (!FlowLevel)
    rollIndent(Column, Token::TK_खण्डम्_Mapping_Start, TokenQueue.अन्तः());

  remove_Simple_कुञ्चिका_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_कुञ्चिका_Allowed = !FlowLevel;
  Is_Adjacent_मानम्_Allowed_अन्तर्_Flow = असत्य;

  Token T;
  T.विधा_2 = Token::TK_कुञ्चिका;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.प्रक्षिप_पृष्ठभागः(T);
  प्रत्यागम सत्य;
}

तार्किक Scanner::अवलोकनम्_मानम्() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  यदि (!Simple_कुञ्चिकाः.रिक्तानि()) {
    Simple_कुञ्चिका SK = Simple_कुञ्चिकाः.निष्कासय_पृष्ठभागः_val();
    Token T;
    T.विधा_2 = Token::TK_कुञ्चिका;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    प्रति (i = TokenQueue.begin(), e = TokenQueue.अन्तः(); i != e; ++i) {
      यदि (i == SK.Tok)
        भङ्ग;
    }
    यदि (i == e) {
      Failed = सत्य;
      प्रत्यागम असत्य;
    }
    i = TokenQueue.निवेशय(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_खण्डम्_Mapping_Start, i);

    Is_Simple_कुञ्चिका_Allowed = असत्य;
  } अन्यथा {
    यदि (!FlowLevel)
      rollIndent(Column, Token::TK_खण्डम्_Mapping_Start, TokenQueue.अन्तः());
    Is_Simple_कुञ्चिका_Allowed = !FlowLevel;
  }
  Is_Adjacent_मानम्_Allowed_अन्तर्_Flow = असत्य;

  Token T;
  T.विधा_2 = Token::TK_मानम्;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.प्रक्षिप_पृष्ठभागः(T);
  प्रत्यागम सत्य;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE स्थिर तार्किक
wasEscaped(StringRef::iterator First, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
स्थिर तार्किक wasEscaped(StringRef::iterator First,
                       StringRef::iterator Position) {
  प्रतिज्ञा(Position - 1 >= First);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  यावत् (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  प्रत्यागम (Position - 1 - I) % 2 == 1;
}

तार्किक Scanner::अवलोकनम्_Flow_Scalar(तार्किक IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  अचिह्नित ColStart = Column;
  यदि (IsDoubleQuoted) {
    कुरु {
      ++Current;
      यावत् (Current != अन्तः_2 && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } यावत् (   Current != अन्तः_2
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } अन्यथा {
    skip(1);
    यावत् (Current != अन्तः_2) {
      // Skip a ' followed by another '.
      यदि (Current + 1 < अन्तः_2 && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        अनुवर्त;
      } अन्यथा यदि (*Current == '\'')
        भङ्ग;
      StringRef::iterator i = skip_nb_char(Current);
      यदि (i == Current) {
        i = skip_b_break(Current);
        यदि (i == Current)
          भङ्ग;
        Current = i;
        Column = 0;
        ++पङ्क्तिः;
      } अन्यथा {
        यदि (i == अन्तः_2)
          भङ्ग;
        Current = i;
        ++Column;
      }
    }
  }

  यदि (Current == अन्तः_2) {
    समुच्चयः_Error("Expected quote at end of scalar", Current);
    प्रत्यागम असत्य;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.विधा_2 = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.प्रक्षिप_पृष्ठभागः(T);

  save_Simple_कुञ्चिका_Candidate(--TokenQueue.अन्तः(), ColStart, असत्य);

  Is_Simple_कुञ्चिका_Allowed = असत्य;
  Is_Adjacent_मानम्_Allowed_अन्तर्_Flow = सत्य;

  प्रत्यागम सत्य;
}

तार्किक Scanner::अवलोकनम्_Plain_Scalar() {
  StringRef::iterator Start = Current;
  अचिह्नित ColStart = Column;
  अचिह्नित LeadingBlanks = 0;
  प्रतिज्ञा(Indent >= -1 && "Indent must be >= -1 !");
  अचिह्नित indent = static_cast<अचिह्नित>(Indent + 1);
  यावत् (Current != अन्तः_2) {
    यदि (*Current == '#')
      भङ्ग;

    यावत् (Current != अन्तः_2 &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      यदि (i == Current)
        भङ्ग;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    यदि (!isBlankOrBreak(Current))
      भङ्ग;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    यावत् (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      यदि (i != Tmp) {
        यदि (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          समुच्चयः_Error("Found invalid tab character in indentation", Tmp);
          प्रत्यागम असत्य;
        }
        Tmp = i;
        ++Column;
      } अन्यथा {
        i = skip_b_break(Tmp);
        यदि (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++पङ्क्तिः;
      }
    }

    यदि (!FlowLevel && Column < indent)
      भङ्ग;

    Current = Tmp;
  }
  यदि (Start == Current) {
    समुच्चयः_Error("Got empty plain scalar", Start);
    प्रत्यागम असत्य;
  }
  Token T;
  T.विधा_2 = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.प्रक्षिप_पृष्ठभागः(T);

  // Plain scalars can be simple keys.
  save_Simple_कुञ्चिका_Candidate(--TokenQueue.अन्तः(), ColStart, असत्य);

  Is_Simple_कुञ्चिका_Allowed = असत्य;
  Is_Adjacent_मानम्_Allowed_अन्तर्_Flow = असत्य;

  प्रत्यागम सत्य;
}

तार्किक Scanner::अवलोकनम्_Alias_Or_Anchor(तार्किक IsAlias) {
  StringRef::iterator Start = Current;
  अचिह्नित ColStart = Column;
  skip(1);
  यावत् (Current != अन्तः_2) {
    यदि (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      भङ्ग;
    StringRef::iterator i = skip_ns_char(Current);
    यदि (i == Current)
      भङ्ग;
    Current = i;
    ++Column;
  }

  यदि (Start + 1 == Current) {
    समुच्चयः_Error("Got empty alias or anchor", Start);
    प्रत्यागम असत्य;
  }

  Token T;
  T.विधा_2 = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.प्रक्षिप_पृष्ठभागः(T);

  // Alias and anchors can be simple keys.
  save_Simple_कुञ्चिका_Candidate(--TokenQueue.अन्तः(), ColStart, असत्य);

  Is_Simple_कुञ्चिका_Allowed = असत्य;
  Is_Adjacent_मानम्_Allowed_अन्तर्_Flow = असत्य;

  प्रत्यागम सत्य;
}

तार्किक Scanner::अवलोकनम्_खण्डम्_Scalar_Indicators(वर्ण &StyleIndicator,
                                        वर्ण &ChompingIndicator,
                                        अचिह्नित &IndentIndicator,
                                        तार्किक &Is_समाप्तम्) {
  StyleIndicator = अवलोकनम्_खण्डम्_Style_Indicator();
  यदि (!अवलोकनम्_खण्डम्_Scalar_शीर्षकम्(ChompingIndicator, IndentIndicator, Is_समाप्तम्))
    प्रत्यागम असत्य;
  प्रत्यागम सत्य;
}

वर्ण Scanner::अवलोकनम्_खण्डम्_Style_Indicator() {
  वर्ण Indicator = ' ';
  यदि (Current != अन्तः_2 && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  प्रत्यागम Indicator;
}

वर्ण Scanner::अवलोकनम्_खण्डम्_Chomping_Indicator() {
  वर्ण Indicator = ' ';
  यदि (Current != अन्तः_2 && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  प्रत्यागम Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
स्थिर अचिह्नित get_Chomped_पङ्क्तिः_Breaks(वर्ण ChompingIndicator,
                                     अचिह्नित पङ्क्तिः_Breaks, StringRef Str) {
  यदि (ChompingIndicator == '-') // Strip all line breaks.
    प्रत्यागम 0;
  यदि (ChompingIndicator == '+') // Keep all line breaks.
    प्रत्यागम पङ्क्तिः_Breaks;
  // Clip trailing lines.
  प्रत्यागम Str.रिक्तानि() ? 0 : 1;
}

अचिह्नित Scanner::अवलोकनम्_खण्डम्_Indentation_Indicator() {
  अचिह्नित Indent = 0;
  यदि (Current != अन्तः_2 && (*Current >= '1' && *Current <= '9')) {
    Indent = अचिह्नित(*Current - '0');
    skip(1);
  }
  प्रत्यागम Indent;
}

तार्किक Scanner::अवलोकनम्_खण्डम्_Scalar_शीर्षकम्(वर्ण &ChompingIndicator,
                                    अचिह्नित &IndentIndicator, तार्किक &Is_समाप्तम्) {
  स्वचालित Start = Current;

  ChompingIndicator = अवलोकनम्_खण्डम्_Chomping_Indicator();
  IndentIndicator = अवलोकनम्_खण्डम्_Indentation_Indicator();
  // Check for the chomping indicator once again.
  यदि (ChompingIndicator == ' ')
    ChompingIndicator = अवलोकनम्_खण्डम्_Chomping_Indicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  यदि (Current == अन्तः_2) { // EOF, we have an empty scalar.
    Token T;
    T.विधा_2 = Token::TK_खण्डम्_Scalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.प्रक्षिप_पृष्ठभागः(T);
    Is_समाप्तम् = सत्य;
    प्रत्यागम सत्य;
  }

  यदि (!consume_पङ्क्तिः_Break_If_Present()) {
    समुच्चयः_Error("Expected a line break after block scalar header", Current);
    प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य;
}

तार्किक Scanner::find_खण्डम्_Scalar_Indent(अचिह्नित &खण्डम्_Indent,
                                    अचिह्नित खण्डम्_Exit_Indent,
                                    अचिह्नित &पङ्क्तिः_Breaks, तार्किक &Is_समाप्तम्) {
  अचिह्नित Max_All_Space_पङ्क्तिः_Characters = 0;
  StringRef::iterator Longest_All_Space_पङ्क्तिः;

  यावत् (सत्य) {
    advanceWhile(&Scanner::skip_s_space);
    यदि (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      यदि (Column <= खण्डम्_Exit_Indent) { // End of the block literal.
        Is_समाप्तम् = सत्य;
        प्रत्यागम सत्य;
      }
      // We found the block's indentation.
      खण्डम्_Indent = Column;
      यदि (Max_All_Space_पङ्क्तिः_Characters > खण्डम्_Indent) {
        समुच्चयः_Error(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_पङ्क्तिः);
        प्रत्यागम असत्य;
      }
      प्रत्यागम सत्य;
    }
    यदि (skip_b_break(Current) != Current &&
        Column > Max_All_Space_पङ्क्तिः_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_पङ्क्तिः_Characters = Column;
      Longest_All_Space_पङ्क्तिः = Current;
    }

    // Check for EOF.
    यदि (Current == अन्तः_2) {
      Is_समाप्तम् = सत्य;
      प्रत्यागम सत्य;
    }

    यदि (!consume_पङ्क्तिः_Break_If_Present()) {
      Is_समाप्तम् = सत्य;
      प्रत्यागम सत्य;
    }
    ++पङ्क्तिः_Breaks;
  }
  प्रत्यागम सत्य;
}

तार्किक Scanner::अवलोकनम्_खण्डम्_Scalar_Indent(अचिह्नित खण्डम्_Indent,
                                    अचिह्नित खण्डम्_Exit_Indent, तार्किक &Is_समाप्तम्) {
  // Skip the indentation.
  यावत् (Column < खण्डम्_Indent) {
    स्वचालित I = skip_s_space(Current);
    यदि (I == Current)
      भङ्ग;
    Current = I;
    ++Column;
  }

  यदि (skip_nb_char(Current) == Current)
    प्रत्यागम सत्य;

  यदि (Column <= खण्डम्_Exit_Indent) { // End of the block literal.
    Is_समाप्तम् = सत्य;
    प्रत्यागम सत्य;
  }

  यदि (Column < खण्डम्_Indent) {
    यदि (Current != अन्तः_2 && *Current == '#') { // Trailing comment.
      Is_समाप्तम् = सत्य;
      प्रत्यागम सत्य;
    }
    समुच्चयः_Error("A text line is less indented than the block scalar", Current);
    प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य; // A normal text line.
}

तार्किक Scanner::अवलोकनम्_खण्डम्_Scalar(तार्किक IsLiteral) {
  प्रतिज्ञा(*Current == '|' || *Current == '>');
  वर्ण StyleIndicator;
  वर्ण ChompingIndicator;
  अचिह्नित खण्डम्_Indent;
  तार्किक Is_समाप्तम् = असत्य;
  यदि (!अवलोकनम्_खण्डम्_Scalar_Indicators(StyleIndicator, ChompingIndicator, खण्डम्_Indent,
                                 Is_समाप्तम्))
    प्रत्यागम असत्य;
  यदि (Is_समाप्तम्)
    प्रत्यागम सत्य;
  तार्किक IsFolded = StyleIndicator == '>';

  ध्रुव स्वचालित *Start = Current;
  अचिह्नित खण्डम्_Exit_Indent = Indent < 0 ? 0 : (अचिह्नित)Indent;
  अचिह्नित पङ्क्तिः_Breaks = 0;
  यदि (खण्डम्_Indent == 0) {
    यदि (!find_खण्डम्_Scalar_Indent(खण्डम्_Indent, खण्डम्_Exit_Indent, पङ्क्तिः_Breaks,
                               Is_समाप्तम्))
      प्रत्यागम असत्य;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  यावत् (!Is_समाप्तम्) {
    यदि (!अवलोकनम्_खण्डम्_Scalar_Indent(खण्डम्_Indent, खण्डम्_Exit_Indent, Is_समाप्तम्))
      प्रत्यागम असत्य;
    यदि (Is_समाप्तम्)
      भङ्ग;

    // Parse the current line.
    स्वचालित पङ्क्तिः_Start = Current;
    advanceWhile(&Scanner::skip_nb_char);
    यदि (पङ्क्तिः_Start != Current) {
      यदि (पङ्क्तिः_Breaks && IsFolded && !Scanner::is_पङ्क्तिः_रिक्तानि(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        यदि (पङ्क्तिः_Breaks == 1) {
          Str.append(पङ्क्तिः_Breaks,
                     is_पङ्क्तिः_रिक्तानि(StringRef(पङ्क्तिः_Start, Current - पङ्क्तिः_Start))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        पङ्क्तिः_Breaks--;
      }
      Str.append(पङ्क्तिः_Breaks, '\n');
      Str.append(StringRef(पङ्क्तिः_Start, Current - पङ्क्तिः_Start));
      पङ्क्तिः_Breaks = 0;
    }

    // Check for EOF.
    यदि (Current == अन्तः_2)
      भङ्ग;

    यदि (!consume_पङ्क्तिः_Break_If_Present())
      भङ्ग;
    ++पङ्क्तिः_Breaks;
  }

  यदि (Current == अन्तः_2 && !पङ्क्तिः_Breaks)
    // Ensure that there is at least one line break before the end of file.
    पङ्क्तिः_Breaks = 1;
  Str.append(get_Chomped_पङ्क्तिः_Breaks(ChompingIndicator, पङ्क्तिः_Breaks, Str), '\n');

  // New lines may start a simple key.
  यदि (!FlowLevel)
    Is_Simple_कुञ्चिका_Allowed = सत्य;
  Is_Adjacent_मानम्_Allowed_अन्तर्_Flow = असत्य;

  Token T;
  T.विधा_2 = Token::TK_खण्डम्_Scalar;
  T.Range = StringRef(Start, Current - Start);
  T.मानम् = std::string(Str);
  TokenQueue.प्रक्षिप_पृष्ठभागः(T);
  प्रत्यागम सत्य;
}

तार्किक Scanner::अवलोकनम्_चिह्नकम्() {
  StringRef::iterator Start = Current;
  अचिह्नित ColStart = Column;
  skip(1); // Eat !.
  यदि (Current == अन्तः_2 || isBlankOrBreak(Current)); // An empty tag.
  अन्यथा यदि (*Current == '<') {
    skip(1);
    अवलोकनम्_ns_uri_char();
    यदि (!consume('>'))
      प्रत्यागम असत्य;
  } अन्यथा {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.विधा_2 = Token::TK_चिह्नकम्;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.प्रक्षिप_पृष्ठभागः(T);

  // Tags can be simple keys.
  save_Simple_कुञ्चिका_Candidate(--TokenQueue.अन्तः(), ColStart, असत्य);

  Is_Simple_कुञ्चिका_Allowed = असत्य;
  Is_Adjacent_मानम्_Allowed_अन्तर्_Flow = असत्य;

  प्रत्यागम सत्य;
}

तार्किक Scanner::fetch_अधिकम्_Tokens() {
  यदि (Is_Start_सम्बन्धि_प्रवाहः)
    प्रत्यागम अवलोकनम्_प्रवाहः_Start();

  अवलोकनम्_गन्तव्यम्_अग्र_Token();

  यदि (Current == अन्तः_2)
    प्रत्यागम अवलोकनम्_प्रवाहः_अन्तः();

  remove_Stale_Simple_कुञ्चिका_Candidates();

  unrollIndent(Column);

  यदि (Column == 0 && *Current == '%')
    प्रत्यागम अवलोकनम्_Directive();

  यदि (Column == 0 && Current + 3 <= अन्तः_2
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == अन्तः_2 || isBlankOrBreak(Current + 3)))
    प्रत्यागम अवलोकनम्_Document_Indicator(सत्य);

  यदि (Column == 0 && Current + 3 <= अन्तः_2
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == अन्तः_2 || isBlankOrBreak(Current + 3)))
    प्रत्यागम अवलोकनम्_Document_Indicator(असत्य);

  यदि (*Current == '[')
    प्रत्यागम अवलोकनम्_Flow_Collection_Start(सत्य);

  यदि (*Current == '{')
    प्रत्यागम अवलोकनम्_Flow_Collection_Start(असत्य);

  यदि (*Current == ']')
    प्रत्यागम अवलोकनम्_Flow_Collection_अन्तः(सत्य);

  यदि (*Current == '}')
    प्रत्यागम अवलोकनम्_Flow_Collection_अन्तः(असत्य);

  यदि (*Current == ',')
    प्रत्यागम अवलोकनम्_Flow_प्रविष्टिः();

  यदि (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == अन्तः_2))
    प्रत्यागम अवलोकनम्_खण्डम्_प्रविष्टिः();

  यदि (*Current == '?' && (Current + 1 == अन्तः_2 || isBlankOrBreak(Current + 1)))
    प्रत्यागम अवलोकनम्_कुञ्चिका();

  यदि (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || Is_Adjacent_मानम्_Allowed_अन्तर्_Flow))
    प्रत्यागम अवलोकनम्_मानम्();

  यदि (*Current == '*')
    प्रत्यागम अवलोकनम्_Alias_Or_Anchor(सत्य);

  यदि (*Current == '&')
    प्रत्यागम अवलोकनम्_Alias_Or_Anchor(असत्य);

  यदि (*Current == '!')
    प्रत्यागम अवलोकनम्_चिह्नकम्();

  यदि (*Current == '|' && !FlowLevel)
    प्रत्यागम अवलोकनम्_खण्डम्_Scalar(सत्य);

  यदि (*Current == '>' && !FlowLevel)
    प्रत्यागम अवलोकनम्_खण्डम्_Scalar(असत्य);

  यदि (*Current == '\'')
    प्रत्यागम अवलोकनम्_Flow_Scalar(असत्य);

  यदि (*Current == '"')
    प्रत्यागम अवलोकनम्_Flow_Scalar(सत्य);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  यदि ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_सम्बन्धि("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_सम्बन्धि("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    प्रत्यागम अवलोकनम्_Plain_Scalar();

  समुच्चयः_Error("Unrecognized character while tokenizing.", Current);
  प्रत्यागम असत्य;
}

प्रवाहः::प्रवाहः(StringRef निवेशः, SourceMgr &SM, तार्किक ShowColors,
               std::error_संहिता *EC)
    : scanner(नूतन Scanner(निवेशः, SM, ShowColors, EC)) {}

प्रवाहः::प्रवाहः(स्मृतिः_बफर_Ref निवेशः_बफर, SourceMgr &SM, तार्किक ShowColors,
               std::error_संहिता *EC)
    : scanner(नूतन Scanner(निवेशः_बफर, SM, ShowColors, EC)) {}

प्रवाहः::~प्रवाहः() = स्वभाव;

तार्किक प्रवाहः::failed() { प्रत्यागम scanner->failed(); }

शून्य प्रवाहः::printError(ग्रन्थिः *N, ध्रुव Twine &सन्देशः_4, SourceMgr::Diag_विधा विधा_2) {
  printError(N ? N->getSourceRange() : SMRange(), सन्देशः_4, विधा_2);
}

शून्य प्रवाहः::printError(ध्रुव SMRange &Range, ध्रुव Twine &सन्देशः_4,
                        SourceMgr::Diag_विधा विधा_2) {
  scanner->printError(Range.Start, विधा_2, सन्देशः_4, Range);
}

document_iterator प्रवाहः::begin() {
  यदि (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->get_अग्र();

  CurrentDoc.reset(नूतन Document(*एतत्));
  प्रत्यागम document_iterator(CurrentDoc);
}

document_iterator प्रवाहः::अन्तः() {
  प्रत्यागम document_iterator();
}

शून्य प्रवाहः::skip() {
  प्रति (Document &Doc : *एतत्)
    Doc.skip();
}

ग्रन्थिः::ग्रन्थिः(अचिह्नित पूर्णाङ्क Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), चिह्नकम्(T) {
  SMLoc Start = SMLoc::getFromPointer(peek_अग्र().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string ग्रन्थिः::get_Verbatim_चिह्नकम्() ध्रुव {
  StringRef Raw = get_Raw_चिह्नकम्();
  यदि (!Raw.रिक्तानि() && Raw != "!") {
    std::string Ret;
    यदि (Raw.find_चरमः_सम्बन्धि('!') == 0) {
      Ret = std::string(Doc->get_चिह्नकम्_Map().find("!")->second);
      Ret += Raw.substr(1);
      प्रत्यागम Ret;
    } अन्यथा यदि (Raw.starts_with("!!")) {
      Ret = std::string(Doc->get_चिह्नकम्_Map().find("!!")->second);
      Ret += Raw.substr(2);
      प्रत्यागम Ret;
    } अन्यथा {
      StringRef चिह्नकम्_Handle = Raw.substr(0, Raw.find_चरमः_सम्बन्धि('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->get_चिह्नकम्_Map().find(चिह्नकम्_Handle);
      यदि (It != Doc->get_चिह्नकम्_Map().अन्तः())
        Ret = std::string(It->second);
      अन्यथा {
        Token T;
        T.विधा_2 = Token::TK_चिह्नकम्;
        T.Range = चिह्नकम्_Handle;
        समुच्चयः_Error(Twine("Unknown tag handle ") + चिह्नकम्_Handle, T);
      }
      Ret += Raw.substr(Raw.find_चरमः_सम्बन्धि('!') + 1);
      प्रत्यागम Ret;
    }
  }

  विकल्प (getType()) {
  पक्ष NK_Null:
    प्रत्यागम "tag:yaml.org,2002:null";
  पक्ष NK_Scalar:
  पक्ष NK_खण्डम्_Scalar:
    // TODO: Tag resolution.
    प्रत्यागम "tag:yaml.org,2002:str";
  पक्ष NK_Mapping:
    प्रत्यागम "tag:yaml.org,2002:map";
  पक्ष NK_Sequence:
    प्रत्यागम "tag:yaml.org,2002:seq";
  }

  प्रत्यागम "";
}

Token &ग्रन्थिः::peek_अग्र() {
  प्रत्यागम Doc->peek_अग्र();
}

Token ग्रन्थिः::get_अग्र() {
  प्रत्यागम Doc->get_अग्र();
}

ग्रन्थिः *ग्रन्थिः::parse_खण्डम्_ग्रन्थिः() {
  प्रत्यागम Doc->parse_खण्डम्_ग्रन्थिः();
}

BumpPtrAllocator &ग्रन्थिः::getAllocator() {
  प्रत्यागम Doc->ग्रन्थिः_Allocator;
}

शून्य ग्रन्थिः::समुच्चयः_Error(ध्रुव Twine &सन्देशः_4, Token &Tok) ध्रुव {
  Doc->समुच्चयः_Error(सन्देशः_4, Tok);
}

तार्किक ग्रन्थिः::failed() ध्रुव {
  प्रत्यागम Doc->failed();
}

StringRef Scalar_ग्रन्थिः::get_मानम्(Small_सदिशम्_Impl<वर्ण> &Storage) ध्रुव {
  यदि (मानम्[0] == '"')
    प्रत्यागम get_Double_Quoted_मानम्(मानम्, Storage);
  यदि (मानम्[0] == '\'')
    प्रत्यागम get_Single_Quoted_मानम्(मानम्, Storage);
  प्रत्यागम get_Plain_मानम्(मानम्, Storage);
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
स्थिर StringRef
parse_Scalar_मानम्(StringRef Unquoted_मानम्, Small_सदिशम्_Impl<वर्ण> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, Small_सदिशम्_Impl<वर्ण> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_मानम्.find_first_सम्बन्धि(LookupChars);
  यदि (I == StringRef::npos)
    प्रत्यागम Unquoted_मानम्;

  Storage.clear();
  Storage.reserve(Unquoted_मानम्.आकार_2());
  वर्ण चरमः_New_पङ्क्तिः_Added_As = '\0';
  प्रति (; I != StringRef::npos; I = Unquoted_मानम्.find_first_सम्बन्धि(LookupChars)) {
    यदि (Unquoted_मानम्[I] != '\r' && Unquoted_मानम्[I] != '\n') {
      llvm::append_range(Storage, Unquoted_मानम्.take_front(I));
      Unquoted_मानम् = UnescapeCallback(Unquoted_मानम्.drop_front(I), Storage);
      चरमः_New_पङ्क्तिः_Added_As = '\0';
      अनुवर्त;
    }
    यदि (size_t चरमः_Non_SWhite = Unquoted_मानम्.find_चरमः_not_सम्बन्धि(" \t", I);
        चरमः_Non_SWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_मानम्.take_front(चरमः_Non_SWhite + 1));
      Storage.प्रक्षिप_पृष्ठभागः(' ');
      चरमः_New_पङ्क्तिः_Added_As = ' ';
    } अन्यथा {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      विकल्प (चरमः_New_पङ्क्तिः_Added_As) {
      पक्ष ' ':
        प्रतिज्ञा(!Storage.रिक्तानि() && Storage.पृष्ठभागः() == ' ');
        Storage.पृष्ठभागः() = '\n';
        चरमः_New_पङ्क्तिः_Added_As = '\n';
        भङ्ग;
      पक्ष '\n':
        प्रतिज्ञा(!Storage.रिक्तानि() && Storage.पृष्ठभागः() == '\n');
        Storage.प्रक्षिप_पृष्ठभागः('\n');
        भङ्ग;
      स्वभाव:
        Storage.प्रक्षिप_पृष्ठभागः(' ');
        चरमः_New_पङ्क्तिः_Added_As = ' ';
        भङ्ग;
      }
    }
    // Handle Windows-style EOL
    यदि (Unquoted_मानम्.substr(I, 2) == "\r\n")
      I++;
    Unquoted_मानम् = Unquoted_मानम्.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_मानम्);
  प्रत्यागम StringRef(Storage.begin(), Storage.आकार_2());
}

StringRef
Scalar_ग्रन्थिः::get_Double_Quoted_मानम्(StringRef Raw_मानम्,
                                 Small_सदिशम्_Impl<वर्ण> &Storage) ध्रुव {
  प्रतिज्ञा(Raw_मानम्.आकार_2() >= 2 && Raw_मानम्.front() == '"' &&
         Raw_मानम्.पृष्ठभागः() == '"');
  StringRef Unquoted_मानम् = Raw_मानम्.substr(1, Raw_मानम्.आकार_2() - 2);

  स्वचालित UnescapeFunc = [एतत्](StringRef Unquoted_मानम्,
                             Small_सदिशम्_Impl<वर्ण> &Storage) {
    प्रतिज्ञा(Unquoted_मानम्.take_front(1) == "\\");
    यदि (Unquoted_मानम्.आकार_2() == 1) {
      Token T;
      T.Range = Unquoted_मानम्;
      समुच्चयः_Error("Unrecognized escape code", T);
      Storage.clear();
      प्रत्यागम StringRef();
    }
    Unquoted_मानम् = Unquoted_मानम्.drop_front(1);
    विकल्प (Unquoted_मानम्[0]) {
    स्वभाव: {
      Token T;
      T.Range = Unquoted_मानम्.take_front(1);
      समुच्चयः_Error("Unrecognized escape code", T);
      Storage.clear();
      प्रत्यागम StringRef();
    }
    पक्ष '\r':
      // Shrink the Windows-style EOL.
      यदि (Unquoted_मानम्.आकार_2() >= 2 && Unquoted_मानम्[1] == '\n')
        Unquoted_मानम् = Unquoted_मानम्.drop_front(1);
      [[fallthrough]];
    पक्ष '\n':
      प्रत्यागम Unquoted_मानम्.drop_front(1).ltrim(" \t");
    पक्ष '0':
      Storage.प्रक्षिप_पृष्ठभागः(0x00);
      भङ्ग;
    पक्ष 'a':
      Storage.प्रक्षिप_पृष्ठभागः(0x07);
      भङ्ग;
    पक्ष 'b':
      Storage.प्रक्षिप_पृष्ठभागः(0x08);
      भङ्ग;
    पक्ष 't':
    पक्ष 0x09:
      Storage.प्रक्षिप_पृष्ठभागः(0x09);
      भङ्ग;
    पक्ष 'n':
      Storage.प्रक्षिप_पृष्ठभागः(0x0A);
      भङ्ग;
    पक्ष 'v':
      Storage.प्रक्षिप_पृष्ठभागः(0x0B);
      भङ्ग;
    पक्ष 'f':
      Storage.प्रक्षिप_पृष्ठभागः(0x0C);
      भङ्ग;
    पक्ष 'r':
      Storage.प्रक्षिप_पृष्ठभागः(0x0D);
      भङ्ग;
    पक्ष 'e':
      Storage.प्रक्षिप_पृष्ठभागः(0x1B);
      भङ्ग;
    पक्ष ' ':
      Storage.प्रक्षिप_पृष्ठभागः(0x20);
      भङ्ग;
    पक्ष '"':
      Storage.प्रक्षिप_पृष्ठभागः(0x22);
      भङ्ग;
    पक्ष '/':
      Storage.प्रक्षिप_पृष्ठभागः(0x2F);
      भङ्ग;
    पक्ष '\\':
      Storage.प्रक्षिप_पृष्ठभागः(0x5C);
      भङ्ग;
    पक्ष 'N':
      encodeUTF8(0x85, Storage);
      भङ्ग;
    पक्ष '_':
      encodeUTF8(0xA0, Storage);
      भङ्ग;
    पक्ष 'L':
      encodeUTF8(0x2028, Storage);
      भङ्ग;
    पक्ष 'P':
      encodeUTF8(0x2029, Storage);
      भङ्ग;
    पक्ष 'x': {
      यदि (Unquoted_मानम्.आकार_2() < 3)
        // TODO: Report error.
        भङ्ग;
      अचिह्नित पूर्णाङ्क Unicode_Scalar_मानम्;
      यदि (Unquoted_मानम्.substr(1, 2).getAsInteger(16, Unicode_Scalar_मानम्))
        // TODO: Report error.
        Unicode_Scalar_मानम् = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मानम्, Storage);
      प्रत्यागम Unquoted_मानम्.drop_front(3);
    }
    पक्ष 'u': {
      यदि (Unquoted_मानम्.आकार_2() < 5)
        // TODO: Report error.
        भङ्ग;
      अचिह्नित पूर्णाङ्क Unicode_Scalar_मानम्;
      यदि (Unquoted_मानम्.substr(1, 4).getAsInteger(16, Unicode_Scalar_मानम्))
        // TODO: Report error.
        Unicode_Scalar_मानम् = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मानम्, Storage);
      प्रत्यागम Unquoted_मानम्.drop_front(5);
    }
    पक्ष 'U': {
      यदि (Unquoted_मानम्.आकार_2() < 9)
        // TODO: Report error.
        भङ्ग;
      अचिह्नित पूर्णाङ्क Unicode_Scalar_मानम्;
      यदि (Unquoted_मानम्.substr(1, 8).getAsInteger(16, Unicode_Scalar_मानम्))
        // TODO: Report error.
        Unicode_Scalar_मानम् = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मानम्, Storage);
      प्रत्यागम Unquoted_मानम्.drop_front(9);
    }
    }
    प्रत्यागम Unquoted_मानम्.drop_front(1);
  };

  प्रत्यागम parse_Scalar_मानम्(Unquoted_मानम्, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_ग्रन्थिः::get_Single_Quoted_मानम्(StringRef Raw_मानम्,
                                           Small_सदिशम्_Impl<वर्ण> &Storage) {
  प्रतिज्ञा(Raw_मानम्.आकार_2() >= 2 && Raw_मानम्.front() == '\'' &&
         Raw_मानम्.पृष्ठभागः() == '\'');
  StringRef Unquoted_मानम् = Raw_मानम्.substr(1, Raw_मानम्.आकार_2() - 2);

  स्वचालित UnescapeFunc = [](StringRef Unquoted_मानम्,
                         Small_सदिशम्_Impl<वर्ण> &Storage) {
    प्रतिज्ञा(Unquoted_मानम्.take_front(2) == "''");
    Storage.प्रक्षिप_पृष्ठभागः('\'');
    प्रत्यागम Unquoted_मानम्.drop_front(2);
  };

  प्रत्यागम parse_Scalar_मानम्(Unquoted_मानम्, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_ग्रन्थिः::get_Plain_मानम्(StringRef Raw_मानम्,
                                    Small_सदिशम्_Impl<वर्ण> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_मानम् = Raw_मानम्.rtrim("\r\n \t");
  प्रत्यागम parse_Scalar_मानम्(Raw_मानम्, Storage, "\r\n", शून्यसूचकः);
}

ग्रन्थिः *कुञ्चिका_मानम्_ग्रन्थिः::get_कुञ्चिका() {
  यदि (कुञ्चिका)
    प्रत्यागम कुञ्चिका;
  // Handle implicit null keys.
  {
    Token &t = peek_अग्र();
    यदि (   t.विधा_2 == Token::TK_खण्डम्_अन्तः
        || t.विधा_2 == Token::TK_मानम्
        || t.विधा_2 == Token::TK_Error) {
      प्रत्यागम कुञ्चिका = नूतन (getAllocator()) Null_ग्रन्थिः(Doc);
    }
    यदि (t.विधा_2 == Token::TK_कुञ्चिका)
      get_अग्र(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peek_अग्र();
  यदि (t.विधा_2 == Token::TK_खण्डम्_अन्तः || t.विधा_2 == Token::TK_मानम्) {
    प्रत्यागम कुञ्चिका = नूतन (getAllocator()) Null_ग्रन्थिः(Doc);
  }

  // We've got a normal key.
  प्रत्यागम कुञ्चिका = parse_खण्डम्_ग्रन्थिः();
}

ग्रन्थिः *कुञ्चिका_मानम्_ग्रन्थिः::get_मानम्() {
  यदि (मानम्)
    प्रत्यागम मानम्;

  यदि (ग्रन्थिः* कुञ्चिका = get_कुञ्चिका())
    कुञ्चिका->skip();
  अन्यथा {
    समुच्चयः_Error("Null key in Key Value.", peek_अग्र());
    प्रत्यागम मानम् = नूतन (getAllocator()) Null_ग्रन्थिः(Doc);
  }

  यदि (failed())
    प्रत्यागम मानम् = नूतन (getAllocator()) Null_ग्रन्थिः(Doc);

  // Handle implicit null values.
  {
    Token &t = peek_अग्र();
    यदि (   t.विधा_2 == Token::TK_खण्डम्_अन्तः
        || t.विधा_2 == Token::TK_Flow_Mapping_अन्तः
        || t.विधा_2 == Token::TK_कुञ्चिका
        || t.विधा_2 == Token::TK_Flow_प्रविष्टिः
        || t.विधा_2 == Token::TK_Error) {
      प्रत्यागम मानम् = नूतन (getAllocator()) Null_ग्रन्थिः(Doc);
    }

    यदि (t.विधा_2 != Token::TK_मानम्) {
      समुच्चयः_Error("Unexpected token in Key Value.", t);
      प्रत्यागम मानम् = नूतन (getAllocator()) Null_ग्रन्थिः(Doc);
    }
    get_अग्र(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peek_अग्र();
  यदि (t.विधा_2 == Token::TK_खण्डम्_अन्तः || t.विधा_2 == Token::TK_कुञ्चिका) {
    प्रत्यागम मानम् = नूतन (getAllocator()) Null_ग्रन्थिः(Doc);
  }

  // We got a normal value.
  प्रत्यागम मानम् = parse_खण्डम्_ग्रन्थिः();
}

शून्य Mapping_ग्रन्थिः::increment() {
  यदि (failed()) {
    Is_At_अन्तः = सत्य;
    Current_प्रविष्टिः = शून्यसूचकः;
    प्रत्यागम;
  }
  यदि (Current_प्रविष्टिः) {
    Current_प्रविष्टिः->skip();
    यदि (Type == MT_Inline) {
      Is_At_अन्तः = सत्य;
      Current_प्रविष्टिः = शून्यसूचकः;
      प्रत्यागम;
    }
  }
  Token T = peek_अग्र();
  यदि (T.विधा_2 == Token::TK_कुञ्चिका || T.विधा_2 == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    Current_प्रविष्टिः = नूतन (getAllocator()) कुञ्चिका_मानम्_ग्रन्थिः(Doc);
  } अन्यथा यदि (Type == MT_खण्डम्) {
    विकल्प (T.विधा_2) {
    पक्ष Token::TK_खण्डम्_अन्तः:
      get_अग्र();
      Is_At_अन्तः = सत्य;
      Current_प्रविष्टिः = शून्यसूचकः;
      भङ्ग;
    स्वभाव:
      समुच्चयः_Error("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    पक्ष Token::TK_Error:
      Is_At_अन्तः = सत्य;
      Current_प्रविष्टिः = शून्यसूचकः;
    }
  } अन्यथा {
    विकल्प (T.विधा_2) {
    पक्ष Token::TK_Flow_प्रविष्टिः:
      // Eat the flow entry and recurse.
      get_अग्र();
      प्रत्यागम increment();
    पक्ष Token::TK_Flow_Mapping_अन्तः:
      get_अग्र();
      [[fallthrough]];
    पक्ष Token::TK_Error:
      // Set this to end iterator.
      Is_At_अन्तः = सत्य;
      Current_प्रविष्टिः = शून्यसूचकः;
      भङ्ग;
    स्वभाव:
      समुच्चयः_Error( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      Is_At_अन्तः = सत्य;
      Current_प्रविष्टिः = शून्यसूचकः;
    }
  }
}

शून्य Sequence_ग्रन्थिः::increment() {
  यदि (failed()) {
    Is_At_अन्तः = सत्य;
    Current_प्रविष्टिः = शून्यसूचकः;
    प्रत्यागम;
  }
  यदि (Current_प्रविष्टिः)
    Current_प्रविष्टिः->skip();
  Token T = peek_अग्र();
  यदि (SeqType == ST_खण्डम्) {
    विकल्प (T.विधा_2) {
    पक्ष Token::TK_खण्डम्_प्रविष्टिः:
      get_अग्र();
      Current_प्रविष्टिः = parse_खण्डम्_ग्रन्थिः();
      यदि (!Current_प्रविष्टिः) { // An error occurred.
        Is_At_अन्तः = सत्य;
        Current_प्रविष्टिः = शून्यसूचकः;
      }
      भङ्ग;
    पक्ष Token::TK_खण्डम्_अन्तः:
      get_अग्र();
      Is_At_अन्तः = सत्य;
      Current_प्रविष्टिः = शून्यसूचकः;
      भङ्ग;
    स्वभाव:
      समुच्चयः_Error( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    पक्ष Token::TK_Error:
      Is_At_अन्तः = सत्य;
      Current_प्रविष्टिः = शून्यसूचकः;
    }
  } अन्यथा यदि (SeqType == ST_Indentless) {
    विकल्प (T.विधा_2) {
    पक्ष Token::TK_खण्डम्_प्रविष्टिः:
      get_अग्र();
      Current_प्रविष्टिः = parse_खण्डम्_ग्रन्थिः();
      यदि (!Current_प्रविष्टिः) { // An error occurred.
        Is_At_अन्तः = सत्य;
        Current_प्रविष्टिः = शून्यसूचकः;
      }
      भङ्ग;
    स्वभाव:
    पक्ष Token::TK_Error:
      Is_At_अन्तः = सत्य;
      Current_प्रविष्टिः = शून्यसूचकः;
    }
  } अन्यथा यदि (SeqType == ST_Flow) {
    विकल्प (T.विधा_2) {
    पक्ष Token::TK_Flow_प्रविष्टिः:
      // Eat the flow entry and recurse.
      get_अग्र();
      Was_Previous_Token_Flow_प्रविष्टिः = सत्य;
      प्रत्यागम increment();
    पक्ष Token::TK_Flow_Sequence_अन्तः:
      get_अग्र();
      [[fallthrough]];
    पक्ष Token::TK_Error:
      // Set this to end iterator.
      Is_At_अन्तः = सत्य;
      Current_प्रविष्टिः = शून्यसूचकः;
      भङ्ग;
    पक्ष Token::TK_प्रवाहः_अन्तः:
    पक्ष Token::TK_Document_अन्तः:
    पक्ष Token::TK_DocumentStart:
      समुच्चयः_Error("Could not find closing ]!", T);
      // Set this to end iterator.
      Is_At_अन्तः = सत्य;
      Current_प्रविष्टिः = शून्यसूचकः;
      भङ्ग;
    स्वभाव:
      यदि (!Was_Previous_Token_Flow_प्रविष्टिः) {
        समुच्चयः_Error("Expected , between entries!", T);
        Is_At_अन्तः = सत्य;
        Current_प्रविष्टिः = शून्यसूचकः;
        भङ्ग;
      }
      // Otherwise it must be a flow entry.
      Current_प्रविष्टिः = parse_खण्डम्_ग्रन्थिः();
      यदि (!Current_प्रविष्टिः) {
        Is_At_अन्तः = सत्य;
      }
      Was_Previous_Token_Flow_प्रविष्टिः = असत्य;
      भङ्ग;
    }
  }
}

Document::Document(प्रवाहः &S) : प्रवाहः_2(S), मूलम्_2(शून्यसूचकः) {
  // Tag maps starts with two default mappings.
  चिह्नकम्_Map["!"] = "!";
  चिह्नकम्_Map["!!"] = "tag:yaml.org,2002:";

  यदि (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peek_अग्र();
  यदि (T.विधा_2 == Token::TK_DocumentStart)
    get_अग्र();
}

तार्किक Document::skip()  {
  यदि (प्रवाहः_2.scanner->failed())
    प्रत्यागम असत्य;
  यदि (!मूलम्_2 && !get_मूलम्())
    प्रत्यागम असत्य;
  मूलम्_2->skip();
  Token &T = peek_अग्र();
  यदि (T.विधा_2 == Token::TK_प्रवाहः_अन्तः)
    प्रत्यागम असत्य;
  यदि (T.विधा_2 == Token::TK_Document_अन्तः) {
    get_अग्र();
    प्रत्यागम skip();
  }
  प्रत्यागम सत्य;
}

Token &Document::peek_अग्र() {
  प्रत्यागम प्रवाहः_2.scanner->peek_अग्र();
}

Token Document::get_अग्र() {
  प्रत्यागम प्रवाहः_2.scanner->get_अग्र();
}

शून्य Document::समुच्चयः_Error(ध्रुव Twine &सन्देशः_3, Token &Location) ध्रुव {
  प्रवाहः_2.scanner->समुच्चयः_Error(सन्देशः_3, Location.Range.begin());
}

तार्किक Document::failed() ध्रुव {
  प्रत्यागम प्रवाहः_2.scanner->failed();
}

ग्रन्थिः *Document::parse_खण्डम्_ग्रन्थिः() {
  Token T = peek_अग्र();
  // Handle properties.
  Token AnchorInfo;
  Token चिह्नकम्_Info;
parse_property:
  विकल्प (T.विधा_2) {
  पक्ष Token::TK_Alias:
    get_अग्र();
    प्रत्यागम नूतन (ग्रन्थिः_Allocator) Alias_ग्रन्थिः(प्रवाहः_2.CurrentDoc, T.Range.substr(1));
  पक्ष Token::TK_Anchor:
    यदि (AnchorInfo.विधा_2 == Token::TK_Anchor) {
      समुच्चयः_Error("Already encountered an anchor for this node!", T);
      प्रत्यागम शून्यसूचकः;
    }
    AnchorInfo = get_अग्र(); // Consume TK_Anchor.
    T = peek_अग्र();
    गच्छ parse_property;
  पक्ष Token::TK_चिह्नकम्:
    यदि (चिह्नकम्_Info.विधा_2 == Token::TK_चिह्नकम्) {
      समुच्चयः_Error("Already encountered a tag for this node!", T);
      प्रत्यागम शून्यसूचकः;
    }
    चिह्नकम्_Info = get_अग्र(); // Consume TK_Tag.
    T = peek_अग्र();
    गच्छ parse_property;
  स्वभाव:
    भङ्ग;
  }

  विकल्प (T.विधा_2) {
  पक्ष Token::TK_खण्डम्_प्रविष्टिः:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    प्रत्यागम नूतन (ग्रन्थिः_Allocator) Sequence_ग्रन्थिः( प्रवाहः_2.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , चिह्नकम्_Info.Range
                                           , Sequence_ग्रन्थिः::ST_Indentless);
  पक्ष Token::TK_खण्डम्_Sequence_Start:
    get_अग्र();
    प्रत्यागम नूतन (ग्रन्थिः_Allocator)
      Sequence_ग्रन्थिः( प्रवाहः_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , चिह्नकम्_Info.Range
                  , Sequence_ग्रन्थिः::ST_खण्डम्);
  पक्ष Token::TK_खण्डम्_Mapping_Start:
    get_अग्र();
    प्रत्यागम नूतन (ग्रन्थिः_Allocator)
      Mapping_ग्रन्थिः( प्रवाहः_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , चिह्नकम्_Info.Range
                 , Mapping_ग्रन्थिः::MT_खण्डम्);
  पक्ष Token::TK_FlowSequenceStart:
    get_अग्र();
    प्रत्यागम नूतन (ग्रन्थिः_Allocator)
      Sequence_ग्रन्थिः( प्रवाहः_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , चिह्नकम्_Info.Range
                  , Sequence_ग्रन्थिः::ST_Flow);
  पक्ष Token::TK_FlowMappingStart:
    get_अग्र();
    प्रत्यागम नूतन (ग्रन्थिः_Allocator)
      Mapping_ग्रन्थिः( प्रवाहः_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , चिह्नकम्_Info.Range
                 , Mapping_ग्रन्थिः::MT_Flow);
  पक्ष Token::TK_Scalar:
    get_अग्र();
    प्रत्यागम नूतन (ग्रन्थिः_Allocator)
      Scalar_ग्रन्थिः( प्रवाहः_2.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , चिह्नकम्_Info.Range
                , T.Range);
  पक्ष Token::TK_खण्डम्_Scalar: {
    get_अग्र();
    StringRef NullTerminatedStr(T.मानम्.c_str(), T.मानम्.दीर्घता_2() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(ग्रन्थिः_Allocator).drop_पृष्ठभागः();
    प्रत्यागम नूतन (ग्रन्थिः_Allocator)
        खण्डम्_Scalar_ग्रन्थिः(प्रवाहः_2.CurrentDoc, AnchorInfo.Range.substr(1),
                        चिह्नकम्_Info.Range, StrCopy, T.Range);
  }
  पक्ष Token::TK_कुञ्चिका:
    // Don't eat the TK_Key, KeyValueNode expects it.
    प्रत्यागम नूतन (ग्रन्थिः_Allocator)
      Mapping_ग्रन्थिः( प्रवाहः_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , चिह्नकम्_Info.Range
                 , Mapping_ग्रन्थिः::MT_Inline);
  पक्ष Token::TK_DocumentStart:
  पक्ष Token::TK_Document_अन्तः:
  पक्ष Token::TK_प्रवाहः_अन्तः:
  स्वभाव:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    प्रत्यागम नूतन (ग्रन्थिः_Allocator) Null_ग्रन्थिः(प्रवाहः_2.CurrentDoc);
  पक्ष Token::TK_Flow_Mapping_अन्तः:
  पक्ष Token::TK_Flow_Sequence_अन्तः:
  पक्ष Token::TK_Flow_प्रविष्टिः: {
    यदि (मूलम्_2 && (isa<Mapping_ग्रन्थिः>(मूलम्_2) || isa<Sequence_ग्रन्थिः>(मूलम्_2)))
      प्रत्यागम नूतन (ग्रन्थिः_Allocator) Null_ग्रन्थिः(प्रवाहः_2.CurrentDoc);

    समुच्चयः_Error("Unexpected token", T);
    प्रत्यागम शून्यसूचकः;
  }
  पक्ष Token::TK_Error:
    प्रत्यागम शून्यसूचकः;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  प्रत्यागम शून्यसूचकः;
}

तार्किक Document::parseDirectives() {
  तार्किक isDirective = असत्य;
  यावत् (सत्य) {
    Token T = peek_अग्र();
    यदि (T.विधा_2 == Token::TK_चिह्नकम्_Directive) {
      parseTAGDirective();
      isDirective = सत्य;
    } अन्यथा यदि (T.विधा_2 == Token::TK_संस्करणम्_Directive) {
      parseYAMLDirective();
      isDirective = सत्य;
    } अन्यथा {
      भङ्ग;
    }
  }
  प्रत्यागम isDirective;
}

शून्य Document::parseYAMLDirective() {
  get_अग्र(); // Eat %YAML <version>
}

शून्य Document::parseTAGDirective() {
  Token चिह्नकम् = get_अग्र(); // %TAG <handle> <prefix>
  StringRef T = चिह्नकम्.Range;
  // Strip %TAG
  T = T.substr(T.find_first_सम्बन्धि(" \t")).ltrim(" \t");
  std::size_t Handle_अन्तः = T.find_first_सम्बन्धि(" \t");
  StringRef चिह्नकम्_Handle = T.substr(0, Handle_अन्तः);
  StringRef चिह्नकम्_उपसर्गः = T.substr(Handle_अन्तः).ltrim(" \t");
  चिह्नकम्_Map[चिह्नकम्_Handle] = चिह्नकम्_उपसर्गः;
}

तार्किक Document::expectToken(पूर्णाङ्क TK) {
  Token T = get_अग्र();
  यदि (T.विधा_2 != TK) {
    समुच्चयः_Error("Unexpected token", T);
    प्रत्यागम असत्य;
  }
  प्रत्यागम सत्य;
}
