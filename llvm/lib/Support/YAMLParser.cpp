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

#అన్తర్భావ "llvm/Support/YAMLParser.h"
#అన్తర్భావ "llvm/ADT/AllocatorList.h"
#అన్తర్భావ "llvm/ADT/ArrayRef.h"
#అన్తర్భావ "llvm/ADT/STLExtras.h"
#అన్తర్భావ "llvm/ADT/SmallString.h"
#అన్తర్భావ "llvm/ADT/SmallVector.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/ADT/Twine.h"
#అన్తర్భావ "llvm/Support/Compiler.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ "llvm/Support/MemoryBuffer.h"
#అన్తర్భావ "llvm/Support/SMLoc.h"
#అన్తర్భావ "llvm/Support/SourceMgr.h"
#అన్తర్భావ "llvm/Support/Unicode.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <cassert>
#అన్తర్భావ <cstddef>
#అన్తర్భావ <cstdint>
#అన్తర్భావ <map>
#అన్తర్భావ <memory>
#అన్తర్భావ <string>
#అన్తర్భావ <system_error>
#అన్తర్భావ <utility>

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ yaml;

గణన UnicodeEncodingForm {
  UEF_UTF32_LE, ///< UTF-32 Little Endian
  UEF_UTF32_BE, ///< UTF-32 Big Endian
  UEF_UTF16_LE, ///< UTF-16 Little Endian
  UEF_UTF16_BE, ///< UTF-16 Big Endian
  UEF_UTF8,     ///< UTF-8 or ascii.
  UEF_Unknown   ///< Not a valid Unicode encoding.
};

/// EncodingInfo - Holds the encoding type and length of the byte order mark if
///                it exists. Length is in {0, 2, 3, 4}.
ఉపయోగించి EncodingInfo = std::pair<UnicodeEncodingForm, గుర్తులేని>;

/// getUnicodeEncoding - Reads up to the first 4 bytes to determine the Unicode
///                      encoding form of \a Input.
///
/// @param Input A string of length 0 or more.
/// @returns An EncodingInfo indicating the Unicode encoding form of the input
///          and how long the byte order mark is if one exists.
స్థిర EncodingInfo getUnicodeEncoding(StringRef ఆదానం) {
  ఒకవేళ (ఆదానం.ఖాళీలు())
    తిరిగి {UEF_Unknown, 0};

  ఎంపిక (uint8_t(ఆదానం[0])) {
  స్థితి 0x00:
    ఒకవేళ (ఆదానం.కొలత_2() >= 4) {
      ఒకవేళ (  ఆదానం[1] == 0
         && uint8_t(ఆదానం[2]) == 0xFE
         && uint8_t(ఆదానం[3]) == 0xFF)
        తిరిగి {UEF_UTF32_BE, 4};
      ఒకవేళ (ఆదానం[1] == 0 && ఆదానం[2] == 0 && ఆదానం[3] != 0)
        తిరిగి {UEF_UTF32_BE, 0};
    }

    ఒకవేళ (ఆదానం.కొలత_2() >= 2 && ఆదానం[1] != 0)
      తిరిగి {UEF_UTF16_BE, 0};
    తిరిగి {UEF_Unknown, 0};
  స్థితి 0xFF:
    ఒకవేళ (  ఆదానం.కొలత_2() >= 4
       && uint8_t(ఆదానం[1]) == 0xFE
       && ఆదానం[2] == 0
       && ఆదానం[3] == 0)
      తిరిగి {UEF_UTF32_LE, 4};

    ఒకవేళ (ఆదానం.కొలత_2() >= 2 && uint8_t(ఆదానం[1]) == 0xFE)
      తిరిగి {UEF_UTF16_LE, 2};
    తిరిగి {UEF_Unknown, 0};
  స్థితి 0xFE:
    ఒకవేళ (ఆదానం.కొలత_2() >= 2 && uint8_t(ఆదానం[1]) == 0xFF)
      తిరిగి {UEF_UTF16_BE, 2};
    తిరిగి {UEF_Unknown, 0};
  స్థితి 0xEF:
    ఒకవేళ (  ఆదానం.కొలత_2() >= 3
       && uint8_t(ఆదానం[1]) == 0xBB
       && uint8_t(ఆదానం[2]) == 0xBF)
      తిరిగి {UEF_UTF8, 3};
    తిరిగి {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  ఒకవేళ (ఆదానం.కొలత_2() >= 4 && ఆదానం[1] == 0 && ఆదానం[2] == 0 && ఆదానం[3] == 0)
    తిరిగి {UEF_UTF32_LE, 0};

  ఒకవేళ (ఆదానం.కొలత_2() >= 2 && ఆదానం[1] == 0)
    తిరిగి {UEF_UTF16_LE, 0};

  తిరిగి {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
శూన్య కణుపు::anchor() {}
శూన్య Null_కణుపు::anchor() {}
శూన్య Scalar_కణుపు::anchor() {}
శూన్య ఖండం_Scalar_కణుపు::anchor() {}
శూన్య కీలకం_విలువ_కణుపు::anchor() {}
శూన్య Mapping_కణుపు::anchor() {}
శూన్య Sequence_కణుపు::anchor() {}
శూన్య Alias_కణుపు::anchor() {}

నామాకాశ llvm {
నామాకాశ yaml {

/// Token - A single YAML token.
నిర్మాణం Token {
  గణన Token_రకం {
    TK_Error, // Uninitialized token.
    TK_ప్రవాహం_ప్రారంభం,
    TK_ప్రవాహం_చివర,
    TK_సంస్కరణ_Directive,
    TK_ట్యాగ్_Directive,
    TK_Document_ప్రారంభం,
    TK_Document_చివర,
    TK_ఖండం_నమోదు,
    TK_ఖండం_చివర,
    TK_ఖండం_Sequence_ప్రారంభం,
    TK_ఖండం_Mapping_ప్రారంభం,
    TK_Flow_నమోదు,
    TK_Flow_Sequence_ప్రారంభం,
    TK_Flow_Sequence_చివర,
    TK_Flow_Mapping_ప్రారంభం,
    TK_Flow_Mapping_చివర,
    TK_కీలకం,
    TK_విలువ,
    TK_Scalar,
    TK_ఖండం_Scalar,
    TK_Alias,
    TK_Anchor,
    TK_ట్యాగ్
  } రకం_3 = TK_Error;

  /// A string of length 0 or more whose begin() points to the logical location
  /// of the token in the input.
  StringRef Range;

  /// The value of a block scalar node.
  std::string విలువ;

  Token() = అప్రమేయ;
};

} // end namespace yaml
} // end namespace llvm

ఉపయోగించి Token_వరుస_T = BumpPtrList<Token>;

నామాకాశ {

/// This struct is used to track simple keys.
///
/// Simple keys are handled by creating an entry in SimpleKeys for each Token
/// which could legally be the start of a simple key. When peekNext is called,
/// if the Token To be returned is referenced by a SimpleKey, we continue
/// tokenizing until that potential simple key has either been found to not be
/// a simple key (we moved on to the next line or went further than 1024 chars).
/// Or when we run into a Value, and then insert a Key token (and possibly
/// others) before the SimpleKey's Tok.
నిర్మాణం Simple_కీలకం {
  Token_వరుస_T::iterator Tok;
  గుర్తులేని స్తంభం = 0;
  గుర్తులేని పంక్తి = 0;
  గుర్తులేని FlowLevel = 0;
  తార్కిక IsRequired = అబద్ధం;

  తార్కిక క్రియాకారకం ==(స్థిరరాశి Simple_కీలకం &Other) {
    తిరిగి Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
ఉపయోగించి UTF8Decoded = std::pair<uint32_t, గుర్తులేని>;

స్థిర UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator చివర_2 = Range.చివర();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  ఒకవేళ (Position < చివర_2 && (*Position & 0x80) == 0) {
    తిరిగి {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  ఒకవేళ (Position + 1 < చివర_2 && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    ఒకవేళ (codepoint >= 0x80)
      తిరిగి {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  ఒకవేళ (Position + 2 < చివర_2 && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    ఒకవేళ (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      తిరిగి {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  ఒకవేళ (Position + 3 < చివర_2 && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    ఒకవేళ (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      తిరిగి {codepoint, 4};
  }
  తిరిగి {0, 0};
}

నామాకాశ llvm {
నామాకాశ yaml {

/// Scans YAML tokens from a MemoryBuffer.
తరగతి Scanner {
బహిరంగ:
  Scanner(StringRef ఆదానం, ఆధారం_Mgr &SM, తార్కిక ShowColors = నిజం,
          std::error_సంకేతం *EC = శూన్యసూచిక);
  Scanner(జ్ఞాపకం_బఫరు_Ref బఫరు, ఆధారం_Mgr &SM_, తార్కిక ShowColors = నిజం,
          std::error_సంకేతం *EC = శూన్యసూచిక);

  /// Parse the next token and return it without popping it.
  Token &peek_తదుపరి();

  /// Parse the next token and pop it from the queue.
  Token get_తదుపరి();

  శూన్య printError(SMLoc Loc, ఆధారం_Mgr::Diag_రకం రకం_3, స్థిరరాశి Twine &సందేశం_3,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.Print_సందేశం(Loc, రకం_3, సందేశం_3, Ranges, /* FixIts= */ {}, ShowColors);
  }

  శూన్య సమితి_Error(స్థిరరాశి Twine &సందేశం_3, StringRef::iterator Position) {
    ఒకవేళ (Position >= చివర_2)
      Position = చివర_2 - 1;

    // propagate the error if possible
    ఒకవేళ (EC)
      *EC = make_error_సంకేతం(std::errc::invalid_వాదం);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    ఒకవేళ (!Failed)
      printError(SMLoc::get_From_సూచిక(Position), ఆధారం_Mgr::DK_Error, సందేశం_3);
    Failed = నిజం;
  }

  /// Returns true if an error occurred while parsing.
  తార్కిక failed() {
    తిరిగి Failed;
  }

అంతరంగ:
  శూన్య init(జ్ఞాపకం_బఫరు_Ref బఫరు);

  StringRef current_ఆదానం() {
    తిరిగి StringRef(Current, చివర_2 - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    తిరిగి ::decodeUTF8(StringRef(Position, చివర_2 - Position));
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

  ఉపయోగించి SkipWhileFunc = StringRef::iterator (Scanner::*)(StringRef::iterator);

  /// Skip minimal well-formed code unit subsequences until Func
  ///        returns its input.
  ///
  /// @returns The code unit after the last minimal well-formed code unit
  ///          subsequence that Func accepted.
  StringRef::iterator skip_while( SkipWhileFunc Func
                                , StringRef::iterator Position);

  /// Skip minimal well-formed code unit subsequences until Func returns its
  /// input.
  శూన్య advanceWhile(SkipWhileFunc Func);

  /// Scan ns-uri-char[39]s starting at Cur.
  ///
  /// This updates Cur and Column while scanning.
  శూన్య పరిశీలన_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  తార్కిక consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  శూన్య skip(uint32_t దూరం_2);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  తార్కిక isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  తార్కిక is_Plain_సురక్షిత_Non_Blank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  తార్కిక is_పంక్తి_ఖాళీలు(StringRef పంక్తి);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  తార్కిక consume_పంక్తి_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  శూన్య save_Simple_కీలకం_Candidate( Token_వరుస_T::iterator Tok
                             , గుర్తులేని At_స్తంభం
                             , తార్కిక IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  శూన్య remove_Stale_Simple_కీలకం_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  శూన్య remove_Simple_కీలకం_Candidates_On_Flow_Level(గుర్తులేని Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  తార్కిక unrollIndent(పూర్ణాంకం గమ్యం_స్తంభం);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  తార్కిక rollIndent( పూర్ణాంకం గమ్యం_స్తంభం
                 , Token::Token_రకం రకం_3
                 , Token_వరుస_T::iterator చొప్పించు_బిందువు);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  శూన్య skipComment();

  /// Skip whitespace and comments until the start of the next token.
  శూన్య పరిశీలన_గమ్యం_తదుపరి_Token();

  /// Must be the first token generated.
  తార్కిక పరిశీలన_ప్రవాహం_ప్రారంభం();

  /// Generate tokens needed to close out the stream.
  తార్కిక పరిశీలన_ప్రవాహం_చివర();

  /// Scan a %BLAH directive.
  తార్కిక పరిశీలన_Directive();

  /// Scan a ... or ---.
  తార్కిక పరిశీలన_Document_Indicator(తార్కిక Is_ప్రారంభం);

  /// Scan a [ or { and generate the proper flow collection start token.
  తార్కిక పరిశీలన_Flow_Collection_ప్రారంభం(తార్కిక IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  తార్కిక పరిశీలన_Flow_Collection_చివర(తార్కిక IsSequence);

  /// Scan the , that separates entries in a flow collection.
  తార్కిక పరిశీలన_Flow_నమోదు();

  /// Scan the - that starts block sequence entries.
  తార్కిక పరిశీలన_ఖండం_నమోదు();

  /// Scan an explicit ? indicating a key.
  తార్కిక పరిశీలన_కీలకం();

  /// Scan an explicit : indicating a value.
  తార్కిక పరిశీలన_విలువ();

  /// Scan a quoted scalar.
  తార్కిక పరిశీలన_Flow_Scalar(తార్కిక IsDoubleQuoted);

  /// Scan an unquoted scalar.
  తార్కిక పరిశీలన_Plain_Scalar();

  /// Scan an Alias or Anchor starting with * or &.
  తార్కిక పరిశీలన_Alias_Or_Anchor(తార్కిక IsAlias);

  /// Scan a block scalar starting with | or >.
  తార్కిక పరిశీలన_ఖండం_Scalar(తార్కిక IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  తార్కిక పరిశీలన_ఖండం_Scalar_Indicators(అక్షరం &StyleIndicator, అక్షరం &ChompingIndicator,
                                 గుర్తులేని &IndentIndicator, తార్కిక &Is_పూర్తి);

  /// Scan a style indicator in a block scalar header.
  అక్షరం పరిశీలన_ఖండం_Style_Indicator();

  /// Scan a chomping indicator in a block scalar header.
  అక్షరం పరిశీలన_ఖండం_Chomping_Indicator();

  /// Scan an indentation indicator in a block scalar header.
  గుర్తులేని పరిశీలన_ఖండం_Indentation_Indicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  తార్కిక పరిశీలన_ఖండం_Scalar_శీర్షిక(అక్షరం &ChompingIndicator, గుర్తులేని &IndentIndicator,
                             తార్కిక &Is_పూర్తి);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  తార్కిక కనుగొను_ఖండం_Scalar_Indent(గుర్తులేని &ఖండం_Indent, గుర్తులేని ఖండం_Exit_Indent,
                             గుర్తులేని &పంక్తి_Breaks, తార్కిక &Is_పూర్తి);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  తార్కిక పరిశీలన_ఖండం_Scalar_Indent(గుర్తులేని ఖండం_Indent, గుర్తులేని ఖండం_Exit_Indent,
                             తార్కిక &Is_పూర్తి);

  /// Scan a tag of the form !stuff.
  తార్కిక పరిశీలన_ట్యాగ్();

  /// Dispatch to the next scanning function based on \a *Cur.
  తార్కిక fetch_మరిన్ని_టోకెన్లు();

  /// The SourceMgr used for diagnostics and buffer management.
  ఆధారం_Mgr &SM;

  /// The original input.
  జ్ఞాపకం_బఫరు_Ref ఆదానం_బఫరు;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator చివర_2;

  /// Current YAML indentation level in spaces.
  పూర్ణాంకం Indent;

  /// Current column number in Unicode code points.
  గుర్తులేని స్తంభం;

  /// Current line number.
  గుర్తులేని పంక్తి;

  /// How deep we are in flow style containers. 0 Means at block level.
  గుర్తులేని FlowLevel;

  /// Are we at the start of the stream?
  తార్కిక Is_ప్రారంభం_యొక్క_ప్రవాహం;

  /// Can the next token be the start of a simple key?
  తార్కిక Is_Simple_కీలకం_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  తార్కిక Is_Adjacent_విలువ_Allowed_లోపల_Flow;

  /// True if an error has occurred.
  తార్కిక Failed;

  /// Should colors be used when printing out the diagnostic messages?
  తార్కిక ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  Token_వరుస_T Token_వరుస;

  /// Indentation levels.
  Small_సదిశ<పూర్ణాంకం, 4> Indents;

  /// Potential simple keys.
  Small_సదిశ<Simple_కీలకం, 4> Simple_కీలకాలు;

  std::error_సంకేతం *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
స్థిర శూన్య encodeUTF8( uint32_t Unicode_Scalar_విలువ
                      , Small_సదిశ_Impl<అక్షరం> &ఫలితం) {
  ఒకవేళ (Unicode_Scalar_విలువ <= 0x7F) {
    ఫలితం.నెట్టు_వెనుక(Unicode_Scalar_విలువ & 0x7F);
  } లేకపోతే ఒకవేళ (Unicode_Scalar_విలువ <= 0x7FF) {
    uint8_t మొదటి_Byte = 0xC0 | ((Unicode_Scalar_విలువ & 0x7C0) >> 6);
    uint8_t రెండవ_Byte = 0x80 | (Unicode_Scalar_విలువ & 0x3F);
    ఫలితం.నెట్టు_వెనుక(మొదటి_Byte);
    ఫలితం.నెట్టు_వెనుక(రెండవ_Byte);
  } లేకపోతే ఒకవేళ (Unicode_Scalar_విలువ <= 0xFFFF) {
    uint8_t మొదటి_Byte = 0xE0 | ((Unicode_Scalar_విలువ & 0xF000) >> 12);
    uint8_t రెండవ_Byte = 0x80 | ((Unicode_Scalar_విలువ & 0xFC0) >> 6);
    uint8_t మూడవ_Byte = 0x80 | (Unicode_Scalar_విలువ & 0x3F);
    ఫలితం.నెట్టు_వెనుక(మొదటి_Byte);
    ఫలితం.నెట్టు_వెనుక(రెండవ_Byte);
    ఫలితం.నెట్టు_వెనుక(మూడవ_Byte);
  } లేకపోతే ఒకవేళ (Unicode_Scalar_విలువ <= 0x10FFFF) {
    uint8_t మొదటి_Byte = 0xF0 | ((Unicode_Scalar_విలువ & 0x1F0000) >> 18);
    uint8_t రెండవ_Byte = 0x80 | ((Unicode_Scalar_విలువ & 0x3F000) >> 12);
    uint8_t మూడవ_Byte = 0x80 | ((Unicode_Scalar_విలువ & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_విలువ & 0x3F);
    ఫలితం.నెట్టు_వెనుక(మొదటి_Byte);
    ఫలితం.నెట్టు_వెనుక(రెండవ_Byte);
    ఫలితం.నెట్టు_వెనుక(మూడవ_Byte);
    ఫలితం.నెట్టు_వెనుక(FourthByte);
  }
}

తార్కిక yaml::dump_టోకెన్లు(StringRef ఆదానం, raw_ostream &OS) {
  ఆధారం_Mgr SM;
  Scanner scanner(ఆదానం, SM);
  అయితే (నిజం) {
    Token T = scanner.get_తదుపరి();
    ఎంపిక (T.రకం_3) {
    స్థితి Token::TK_ప్రవాహం_ప్రారంభం:
      OS << "Stream-Start: ";
      ఆపు;
    స్థితి Token::TK_ప్రవాహం_చివర:
      OS << "Stream-End: ";
      ఆపు;
    స్థితి Token::TK_సంస్కరణ_Directive:
      OS << "Version-Directive: ";
      ఆపు;
    స్థితి Token::TK_ట్యాగ్_Directive:
      OS << "Tag-Directive: ";
      ఆపు;
    స్థితి Token::TK_Document_ప్రారంభం:
      OS << "Document-Start: ";
      ఆపు;
    స్థితి Token::TK_Document_చివర:
      OS << "Document-End: ";
      ఆపు;
    స్థితి Token::TK_ఖండం_నమోదు:
      OS << "Block-Entry: ";
      ఆపు;
    స్థితి Token::TK_ఖండం_చివర:
      OS << "Block-End: ";
      ఆపు;
    స్థితి Token::TK_ఖండం_Sequence_ప్రారంభం:
      OS << "Block-Sequence-Start: ";
      ఆపు;
    స్థితి Token::TK_ఖండం_Mapping_ప్రారంభం:
      OS << "Block-Mapping-Start: ";
      ఆపు;
    స్థితి Token::TK_Flow_నమోదు:
      OS << "Flow-Entry: ";
      ఆపు;
    స్థితి Token::TK_Flow_Sequence_ప్రారంభం:
      OS << "Flow-Sequence-Start: ";
      ఆపు;
    స్థితి Token::TK_Flow_Sequence_చివర:
      OS << "Flow-Sequence-End: ";
      ఆపు;
    స్థితి Token::TK_Flow_Mapping_ప్రారంభం:
      OS << "Flow-Mapping-Start: ";
      ఆపు;
    స్థితి Token::TK_Flow_Mapping_చివర:
      OS << "Flow-Mapping-End: ";
      ఆపు;
    స్థితి Token::TK_కీలకం:
      OS << "Key: ";
      ఆపు;
    స్థితి Token::TK_విలువ:
      OS << "Value: ";
      ఆపు;
    స్థితి Token::TK_Scalar:
      OS << "Scalar: ";
      ఆపు;
    స్థితి Token::TK_ఖండం_Scalar:
      OS << "Block Scalar: ";
      ఆపు;
    స్థితి Token::TK_Alias:
      OS << "Alias: ";
      ఆపు;
    స్థితి Token::TK_Anchor:
      OS << "Anchor: ";
      ఆపు;
    స్థితి Token::TK_ట్యాగ్:
      OS << "Tag: ";
      ఆపు;
    స్థితి Token::TK_Error:
      ఆపు;
    }
    OS << T.Range << "\n";
    ఒకవేళ (T.రకం_3 == Token::TK_ప్రవాహం_చివర)
      ఆపు;
    లేకపోతే ఒకవేళ (T.రకం_3 == Token::TK_Error)
      తిరిగి అబద్ధం;
  }
  తిరిగి నిజం;
}

తార్కిక yaml::పరిశీలన_టోకెన్లు(StringRef ఆదానం) {
  ఆధారం_Mgr SM;
  Scanner scanner(ఆదానం, SM);
  అయితే (నిజం) {
    Token T = scanner.get_తదుపరి();
    ఒకవేళ (T.రకం_3 == Token::TK_ప్రవాహం_చివర)
      ఆపు;
    లేకపోతే ఒకవేళ (T.రకం_3 == Token::TK_Error)
      తిరిగి అబద్ధం;
  }
  తిరిగి నిజం;
}

std::string yaml::escape(StringRef ఆదానం, తార్కిక EscapePrintable) {
  std::string Escaped_ఆదానం;
  కొరకు (StringRef::iterator i = ఆదానం.begin(), e = ఆదానం.చివర(); i != e; ++i) {
    ఒకవేళ (*i == '\\')
      Escaped_ఆదానం += "\\\\";
    లేకపోతే ఒకవేళ (*i == '"')
      Escaped_ఆదానం += "\\\"";
    లేకపోతే ఒకవేళ (*i == 0)
      Escaped_ఆదానం += "\\0";
    లేకపోతే ఒకవేళ (*i == 0x07)
      Escaped_ఆదానం += "\\a";
    లేకపోతే ఒకవేళ (*i == 0x08)
      Escaped_ఆదానం += "\\b";
    లేకపోతే ఒకవేళ (*i == 0x09)
      Escaped_ఆదానం += "\\t";
    లేకపోతే ఒకవేళ (*i == 0x0A)
      Escaped_ఆదానం += "\\n";
    లేకపోతే ఒకవేళ (*i == 0x0B)
      Escaped_ఆదానం += "\\v";
    లేకపోతే ఒకవేళ (*i == 0x0C)
      Escaped_ఆదానం += "\\f";
    లేకపోతే ఒకవేళ (*i == 0x0D)
      Escaped_ఆదానం += "\\r";
    లేకపోతే ఒకవేళ (*i == 0x1B)
      Escaped_ఆదానం += "\\e";
    లేకపోతే ఒకవేళ ((గుర్తులేని అక్షరం)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_ఆదానం += "\\x" + std::string(2 - HexStr.కొలత_2(), '0') + HexStr;
    } లేకపోతే ఒకవేళ (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_విలువ
        = decodeUTF8(StringRef(i, ఆదానం.చివర() - i));
      ఒకవేళ (Unicode_Scalar_విలువ.రెండవ_2 == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_ఆదానం, Val);
        // FIXME: Error reporting.
        తిరిగి Escaped_ఆదానం;
      }
      ఒకవేళ (Unicode_Scalar_విలువ.మొదటి_2 == 0x85)
        Escaped_ఆదానం += "\\N";
      లేకపోతే ఒకవేళ (Unicode_Scalar_విలువ.మొదటి_2 == 0xA0)
        Escaped_ఆదానం += "\\_";
      లేకపోతే ఒకవేళ (Unicode_Scalar_విలువ.మొదటి_2 == 0x2028)
        Escaped_ఆదానం += "\\L";
      లేకపోతే ఒకవేళ (Unicode_Scalar_విలువ.మొదటి_2 == 0x2029)
        Escaped_ఆదానం += "\\P";
      లేకపోతే ఒకవేళ (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_విలువ.మొదటి_2))
        Escaped_ఆదానం += StringRef(i, Unicode_Scalar_విలువ.రెండవ_2);
      లేకపోతే {
        std::string HexStr = utohexstr(Unicode_Scalar_విలువ.మొదటి_2);
        ఒకవేళ (HexStr.కొలత_2() <= 2)
          Escaped_ఆదానం += "\\x" + std::string(2 - HexStr.కొలత_2(), '0') + HexStr;
        లేకపోతే ఒకవేళ (HexStr.కొలత_2() <= 4)
          Escaped_ఆదానం += "\\u" + std::string(4 - HexStr.కొలత_2(), '0') + HexStr;
        లేకపోతే ఒకవేళ (HexStr.కొలత_2() <= 8)
          Escaped_ఆదానం += "\\U" + std::string(8 - HexStr.కొలత_2(), '0') + HexStr;
      }
      i += Unicode_Scalar_విలువ.రెండవ_2 - 1;
    } లేకపోతే {
      Escaped_ఆదానం.నెట్టు_వెనుక(*i);
    }
  }
  తిరిగి Escaped_ఆదానం;
}

std::optional<తార్కిక> yaml::parseBool(StringRef S) {
  ఎంపిక (S.కొలత_2()) {
  స్థితి 1:
    ఎంపిక (S.front()) {
    స్థితి 'y':
    స్థితి 'Y':
      తిరిగి నిజం;
    స్థితి 'n':
    స్థితి 'N':
      తిరిగి అబద్ధం;
    అప్రమేయ:
      తిరిగి std::nullopt;
    }
  స్థితి 2:
    ఎంపిక (S.front()) {
    స్థితి 'O':
      ఒకవేళ (S[1] == 'N') // ON
        తిరిగి నిజం;
      [[fallthrough]];
    స్థితి 'o':
      ఒకవేళ (S[1] == 'n') //[Oo]n
        తిరిగి నిజం;
      తిరిగి std::nullopt;
    స్థితి 'N':
      ఒకవేళ (S[1] == 'O') // NO
        తిరిగి అబద్ధం;
      [[fallthrough]];
    స్థితి 'n':
      ఒకవేళ (S[1] == 'o') //[Nn]o
        తిరిగి అబద్ధం;
      తిరిగి std::nullopt;
    అప్రమేయ:
      తిరిగి std::nullopt;
    }
  స్థితి 3:
    ఎంపిక (S.front()) {
    స్థితి 'O':
      ఒకవేళ (S.drop_front() == "FF") // OFF
        తిరిగి అబద్ధం;
      [[fallthrough]];
    స్థితి 'o':
      ఒకవేళ (S.drop_front() == "ff") //[Oo]ff
        తిరిగి అబద్ధం;
      తిరిగి std::nullopt;
    స్థితి 'Y':
      ఒకవేళ (S.drop_front() == "ES") // YES
        తిరిగి నిజం;
      [[fallthrough]];
    స్థితి 'y':
      ఒకవేళ (S.drop_front() == "es") //[Yy]es
        తిరిగి నిజం;
      తిరిగి std::nullopt;
    అప్రమేయ:
      తిరిగి std::nullopt;
    }
  స్థితి 4:
    ఎంపిక (S.front()) {
    స్థితి 'T':
      ఒకవేళ (S.drop_front() == "RUE") // TRUE
        తిరిగి నిజం;
      [[fallthrough]];
    స్థితి 't':
      ఒకవేళ (S.drop_front() == "rue") //[Tt]rue
        తిరిగి నిజం;
      తిరిగి std::nullopt;
    అప్రమేయ:
      తిరిగి std::nullopt;
    }
  స్థితి 5:
    ఎంపిక (S.front()) {
    స్థితి 'F':
      ఒకవేళ (S.drop_front() == "ALSE") // FALSE
        తిరిగి అబద్ధం;
      [[fallthrough]];
    స్థితి 'f':
      ఒకవేళ (S.drop_front() == "alse") //[Ff]alse
        తిరిగి అబద్ధం;
      తిరిగి std::nullopt;
    అప్రమేయ:
      తిరిగి std::nullopt;
    }
  అప్రమేయ:
    తిరిగి std::nullopt;
  }
}

Scanner::Scanner(StringRef ఆదానం, ఆధారం_Mgr &sm, తార్కిక ShowColors,
                 std::error_సంకేతం *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(జ్ఞాపకం_బఫరు_Ref(ఆదానం, "YAML"));
}

Scanner::Scanner(జ్ఞాపకం_బఫరు_Ref బఫరు, ఆధారం_Mgr &SM_, తార్కిక ShowColors,
                 std::error_సంకేతం *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(బఫరు);
}

శూన్య Scanner::init(జ్ఞాపకం_బఫరు_Ref బఫరు) {
  ఆదానం_బఫరు = బఫరు;
  Current = ఆదానం_బఫరు.get_బఫరు_ప్రారంభం();
  చివర_2 = ఆదానం_బఫరు.get_బఫరు_చివర();
  Indent = -1;
  స్తంభం = 0;
  పంక్తి = 0;
  FlowLevel = 0;
  Is_ప్రారంభం_యొక్క_ప్రవాహం = నిజం;
  Is_Simple_కీలకం_Allowed = నిజం;
  Is_Adjacent_విలువ_Allowed_లోపల_Flow = అబద్ధం;
  Failed = అబద్ధం;
  std::unique_ptr<జ్ఞాపకం_బఫరు> ఆదానం_బఫరు_Owner =
      జ్ఞాపకం_బఫరు::get_Mem_బఫరు(బఫరు, /*RequiresNullTerminator=*/అబద్ధం);
  SM.కూడు_New_ఆధారం_బఫరు(std::move(ఆదానం_బఫరు_Owner), SMLoc());
}

Token &Scanner::peek_తదుపరి() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  తార్కిక Need_మరిన్ని = అబద్ధం;
  అయితే (నిజం) {
    ఒకవేళ (Token_వరుస.ఖాళీలు() || Need_మరిన్ని) {
      ఒకవేళ (!fetch_మరిన్ని_టోకెన్లు()) {
        Token_వరుస.clear();
        Simple_కీలకాలు.clear();
        Token_వరుస.నెట్టు_వెనుక(Token());
        తిరిగి Token_వరుస.front();
      }
    }
    ప్రతిజ్ఞా(!Token_వరుస.ఖాళీలు() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_కీలకం_Candidates();
    Simple_కీలకం SK;
    SK.Tok = Token_వరుస.begin();
    ఒకవేళ (!is_contained(Simple_కీలకాలు, SK))
      ఆపు;
    లేకపోతే
      Need_మరిన్ని = నిజం;
  }
  తిరిగి Token_వరుస.front();
}

Token Scanner::get_తదుపరి() {
  Token Ret = peek_తదుపరి();
  // TokenQueue can be empty if there was an error getting the next token.
  ఒకవేళ (!Token_వరుస.ఖాళీలు())
    Token_వరుస.తీయి_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  ఒకవేళ (Token_వరుస.ఖాళీలు())
    Token_వరుస.resetAlloc();

  తిరిగి Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  ఒకవేళ (Position == చివర_2)
    తిరిగి Position;
  // Check 7 bit c-printable - b-char.
  ఒకవేళ (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    తిరిగి Position + 1;

  // Check for valid UTF-8.
  ఒకవేళ (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    ఒకవేళ (   u8d.రెండవ_2 != 0
        && u8d.మొదటి_2 != 0xFEFF
        && ( u8d.మొదటి_2 == 0x85
          || ( u8d.మొదటి_2 >= 0xA0
            && u8d.మొదటి_2 <= 0xD7FF)
          || ( u8d.మొదటి_2 >= 0xE000
            && u8d.మొదటి_2 <= 0xFFFD)
          || ( u8d.మొదటి_2 >= 0x10000
            && u8d.మొదటి_2 <= 0x10FFFF)))
      తిరిగి Position + u8d.రెండవ_2;
  }
  తిరిగి Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  ఒకవేళ (Position == చివర_2)
    తిరిగి Position;
  ఒకవేళ (*Position == 0x0D) {
    ఒకవేళ (Position + 1 != చివర_2 && *(Position + 1) == 0x0A)
      తిరిగి Position + 2;
    తిరిగి Position + 1;
  }

  ఒకవేళ (*Position == 0x0A)
    తిరిగి Position + 1;
  తిరిగి Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  ఒకవేళ (Position == చివర_2)
    తిరిగి Position;
  ఒకవేళ (*Position == ' ')
    తిరిగి Position + 1;
  తిరిగి Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  ఒకవేళ (Position == చివర_2)
    తిరిగి Position;
  ఒకవేళ (*Position == ' ' || *Position == '\t')
    తిరిగి Position + 1;
  తిరిగి Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  ఒకవేళ (Position == చివర_2)
    తిరిగి Position;
  ఒకవేళ (*Position == ' ' || *Position == '\t')
    తిరిగి Position;
  తిరిగి skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  అయితే (నిజం) {
    StringRef::iterator i = (ఇది->*Func)(Position);
    ఒకవేళ (i == Position)
      ఆపు;
    Position = i;
  }
  తిరిగి Position;
}

శూన్య Scanner::advanceWhile(SkipWhileFunc Func) {
  స్వయంచాలక Final = skip_while(Func, Current);
  స్తంభం += Final - Current;
  Current = Final;
}

స్థిర తార్కిక is_ns_hex_digit(స్థిరరాశి అక్షరం C) { తిరిగి isAlnum(C); }

స్థిర తార్కిక is_ns_మాట_char(స్థిరరాశి అక్షరం C) { తిరిగి C == '-' || isAlpha(C); }

శూన్య Scanner::పరిశీలన_ns_uri_char() {
  అయితే (నిజం) {
    ఒకవేళ (Current == చివర_2)
      ఆపు;
    ఒకవేళ ((*Current == '%' && Current + 2 < చివర_2 &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_మాట_char(*Current) ||
        StringRef(Current, 1).కనుగొను_మొదటి_యొక్క("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++స్తంభం;
    } లేకపోతే {
      ఆపు;
    }
  }
}

తార్కిక Scanner::consume(uint32_t Expected) {
  ఒకవేళ (Expected >= 0x80) {
    సమితి_Error("Cannot consume non-ascii characters", Current);
    తిరిగి అబద్ధం;
  }
  ఒకవేళ (Current == చివర_2)
    తిరిగి అబద్ధం;
  ఒకవేళ (uint8_t(*Current) >= 0x80) {
    సమితి_Error("Cannot consume non-ascii characters", Current);
    తిరిగి అబద్ధం;
  }
  ఒకవేళ (uint8_t(*Current) == Expected) {
    ++Current;
    ++స్తంభం;
    తిరిగి నిజం;
  }
  తిరిగి అబద్ధం;
}

శూన్య Scanner::skip(uint32_t దూరం_2) {
  Current += దూరం_2;
  స్తంభం += దూరం_2;
  ప్రతిజ్ఞా(Current <= చివర_2 && "Skipped past the end");
}

తార్కిక Scanner::isBlankOrBreak(StringRef::iterator Position) {
  ఒకవేళ (Position == చివర_2)
    తిరిగి అబద్ధం;
  తిరిగి *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

తార్కిక Scanner::is_Plain_సురక్షిత_Non_Blank(StringRef::iterator Position) {
  ఒకవేళ (Position == చివర_2 || isBlankOrBreak(Position))
    తిరిగి అబద్ధం;
  ఒకవేళ (FlowLevel &&
      StringRef(Position, 1).కనుగొను_మొదటి_యొక్క(",[]{}") != StringRef::npos)
    తిరిగి అబద్ధం;
  తిరిగి నిజం;
}

తార్కిక Scanner::is_పంక్తి_ఖాళీలు(StringRef పంక్తి) {
  కొరకు (స్థిరరాశి స్వయంచాలక *Position = పంక్తి.begin(); Position != పంక్తి.చివర(); ++Position)
    ఒకవేళ (!isBlankOrBreak(Position))
      తిరిగి అబద్ధం;
  తిరిగి నిజం;
}

తార్కిక Scanner::consume_పంక్తి_Break_If_Present() {
  స్వయంచాలక తదుపరి_2 = skip_b_break(Current);
  ఒకవేళ (తదుపరి_2 == Current)
    తిరిగి అబద్ధం;
  స్తంభం = 0;
  ++పంక్తి;
  Current = తదుపరి_2;
  తిరిగి నిజం;
}

శూన్య Scanner::save_Simple_కీలకం_Candidate( Token_వరుస_T::iterator Tok
                                    , గుర్తులేని At_స్తంభం
                                    , తార్కిక IsRequired) {
  ఒకవేళ (Is_Simple_కీలకం_Allowed) {
    Simple_కీలకం SK;
    SK.Tok = Tok;
    SK.పంక్తి = పంక్తి;
    SK.స్తంభం = At_స్తంభం;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_కీలకాలు.నెట్టు_వెనుక(SK);
  }
}

శూన్య Scanner::remove_Stale_Simple_కీలకం_Candidates() {
  కొరకు (Small_సదిశ_Impl<Simple_కీలకం>::iterator i = Simple_కీలకాలు.begin();
                                            i != Simple_కీలకాలు.చివర();) {
    ఒకవేళ (i->పంక్తి != పంక్తి || i->స్తంభం + 1024 < స్తంభం) {
      ఒకవేళ (i->IsRequired)
        సమితి_Error( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_కీలకాలు.erase(i);
    } లేకపోతే {
      ++i;
    }
  }
}

శూన్య Scanner::remove_Simple_కీలకం_Candidates_On_Flow_Level(గుర్తులేని Level) {
  ఒకవేళ (!Simple_కీలకాలు.ఖాళీలు() && (Simple_కీలకాలు.చివర() - 1)->FlowLevel == Level)
    Simple_కీలకాలు.తీయి_వెనుక();
}

తార్కిక Scanner::unrollIndent(పూర్ణాంకం గమ్యం_స్తంభం) {
  Token T;
  // Indentation is ignored in flow.
  ఒకవేళ (FlowLevel != 0)
    తిరిగి నిజం;

  అయితే (Indent > గమ్యం_స్తంభం) {
    T.రకం_3 = Token::TK_ఖండం_చివర;
    T.Range = StringRef(Current, 1);
    Token_వరుస.నెట్టు_వెనుక(T);
    Indent = Indents.తీయి_వెనుక_val();
  }

  తిరిగి నిజం;
}

తార్కిక Scanner::rollIndent( పూర్ణాంకం గమ్యం_స్తంభం
                        , Token::Token_రకం రకం_3
                        , Token_వరుస_T::iterator చొప్పించు_బిందువు) {
  ఒకవేళ (FlowLevel)
    తిరిగి నిజం;
  ఒకవేళ (Indent < గమ్యం_స్తంభం) {
    Indents.నెట్టు_వెనుక(Indent);
    Indent = గమ్యం_స్తంభం;

    Token T;
    T.రకం_3 = రకం_3;
    T.Range = StringRef(Current, 0);
    Token_వరుస.చొప్పించు(చొప్పించు_బిందువు, T);
  }
  తిరిగి నిజం;
}

శూన్య Scanner::skipComment() {
  ఒకవేళ (Current == చివర_2 || *Current != '#')
    తిరిగి;
  అయితే (నిజం) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    ఒకవేళ (I == Current)
      ఆపు;
    Current = I;
    ++స్తంభం;
  }
}

శూన్య Scanner::పరిశీలన_గమ్యం_తదుపరి_Token() {
  అయితే (నిజం) {
    అయితే (Current != చివర_2 && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    ఒకవేళ (i == Current)
      ఆపు;
    Current = i;
    ++పంక్తి;
    స్తంభం = 0;
    // New lines may start a simple key.
    ఒకవేళ (!FlowLevel)
      Is_Simple_కీలకం_Allowed = నిజం;
  }
}

తార్కిక Scanner::పరిశీలన_ప్రవాహం_ప్రారంభం() {
  Is_ప్రారంభం_యొక్క_ప్రవాహం = అబద్ధం;

  EncodingInfo EI = getUnicodeEncoding(current_ఆదానం());

  Token T;
  T.రకం_3 = Token::TK_ప్రవాహం_ప్రారంభం;
  T.Range = StringRef(Current, EI.రెండవ_2);
  Token_వరుస.నెట్టు_వెనుక(T);
  Current += EI.రెండవ_2;
  తిరిగి నిజం;
}

తార్కిక Scanner::పరిశీలన_ప్రవాహం_చివర() {
  // Force an ending new line if one isn't present.
  ఒకవేళ (స్తంభం != 0) {
    స్తంభం = 0;
    ++పంక్తి;
  }

  unrollIndent(-1);
  Simple_కీలకాలు.clear();
  Is_Simple_కీలకం_Allowed = అబద్ధం;
  Is_Adjacent_విలువ_Allowed_లోపల_Flow = అబద్ధం;

  Token T;
  T.రకం_3 = Token::TK_ప్రవాహం_చివర;
  T.Range = StringRef(Current, 0);
  Token_వరుస.నెట్టు_వెనుక(T);
  తిరిగి నిజం;
}

తార్కిక Scanner::పరిశీలన_Directive() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_కీలకాలు.clear();
  Is_Simple_కీలకం_Allowed = అబద్ధం;
  Is_Adjacent_విలువ_Allowed_లోపల_Flow = అబద్ధం;

  StringRef::iterator ప్రారంభం = Current;
  consume('%');
  StringRef::iterator Name_ప్రారంభం = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(Name_ప్రారంభం, Current - Name_ప్రారంభం);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  ఒకవేళ (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.రకం_3 = Token::TK_సంస్కరణ_Directive;
    T.Range = StringRef(ప్రారంభం, Current - ప్రారంభం);
    Token_వరుస.నెట్టు_వెనుక(T);
    తిరిగి నిజం;
  } లేకపోతే ఒకవేళ(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.రకం_3 = Token::TK_ట్యాగ్_Directive;
    T.Range = StringRef(ప్రారంభం, Current - ప్రారంభం);
    Token_వరుస.నెట్టు_వెనుక(T);
    తిరిగి నిజం;
  }
  తిరిగి అబద్ధం;
}

తార్కిక Scanner::పరిశీలన_Document_Indicator(తార్కిక Is_ప్రారంభం) {
  unrollIndent(-1);
  Simple_కీలకాలు.clear();
  Is_Simple_కీలకం_Allowed = అబద్ధం;
  Is_Adjacent_విలువ_Allowed_లోపల_Flow = అబద్ధం;

  Token T;
  T.రకం_3 = Is_ప్రారంభం ? Token::TK_Document_ప్రారంభం : Token::TK_Document_చివర;
  T.Range = StringRef(Current, 3);
  skip(3);
  Token_వరుస.నెట్టు_వెనుక(T);
  తిరిగి నిజం;
}

తార్కిక Scanner::పరిశీలన_Flow_Collection_ప్రారంభం(తార్కిక IsSequence) {
  Token T;
  T.రకం_3 = IsSequence ? Token::TK_Flow_Sequence_ప్రారంభం
                      : Token::TK_Flow_Mapping_ప్రారంభం;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_వరుస.నెట్టు_వెనుక(T);

  // [ and { may begin a simple key.
  save_Simple_కీలకం_Candidate(--Token_వరుస.చివర(), స్తంభం - 1, అబద్ధం);

  // And may also be followed by a simple key.
  Is_Simple_కీలకం_Allowed = నిజం;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_విలువ_Allowed_లోపల_Flow = అబద్ధం;
  ++FlowLevel;
  తిరిగి నిజం;
}

తార్కిక Scanner::పరిశీలన_Flow_Collection_చివర(తార్కిక IsSequence) {
  remove_Simple_కీలకం_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_కీలకం_Allowed = అబద్ధం;
  Is_Adjacent_విలువ_Allowed_లోపల_Flow = నిజం;
  Token T;
  T.రకం_3 = IsSequence ? Token::TK_Flow_Sequence_చివర
                      : Token::TK_Flow_Mapping_చివర;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_వరుస.నెట్టు_వెనుక(T);
  ఒకవేళ (FlowLevel)
    --FlowLevel;
  తిరిగి నిజం;
}

తార్కిక Scanner::పరిశీలన_Flow_నమోదు() {
  remove_Simple_కీలకం_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_కీలకం_Allowed = నిజం;
  Is_Adjacent_విలువ_Allowed_లోపల_Flow = అబద్ధం;
  Token T;
  T.రకం_3 = Token::TK_Flow_నమోదు;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_వరుస.నెట్టు_వెనుక(T);
  తిరిగి నిజం;
}

తార్కిక Scanner::పరిశీలన_ఖండం_నమోదు() {
  rollIndent(స్తంభం, Token::TK_ఖండం_Sequence_ప్రారంభం, Token_వరుస.చివర());
  remove_Simple_కీలకం_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_కీలకం_Allowed = నిజం;
  Is_Adjacent_విలువ_Allowed_లోపల_Flow = అబద్ధం;
  Token T;
  T.రకం_3 = Token::TK_ఖండం_నమోదు;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_వరుస.నెట్టు_వెనుక(T);
  తిరిగి నిజం;
}

తార్కిక Scanner::పరిశీలన_కీలకం() {
  ఒకవేళ (!FlowLevel)
    rollIndent(స్తంభం, Token::TK_ఖండం_Mapping_ప్రారంభం, Token_వరుస.చివర());

  remove_Simple_కీలకం_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_కీలకం_Allowed = !FlowLevel;
  Is_Adjacent_విలువ_Allowed_లోపల_Flow = అబద్ధం;

  Token T;
  T.రకం_3 = Token::TK_కీలకం;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_వరుస.నెట్టు_వెనుక(T);
  తిరిగి నిజం;
}

తార్కిక Scanner::పరిశీలన_విలువ() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  ఒకవేళ (!Simple_కీలకాలు.ఖాళీలు()) {
    Simple_కీలకం SK = Simple_కీలకాలు.తీయి_వెనుక_val();
    Token T;
    T.రకం_3 = Token::TK_కీలకం;
    T.Range = SK.Tok->Range;
    Token_వరుస_T::iterator i, e;
    కొరకు (i = Token_వరుస.begin(), e = Token_వరుస.చివర(); i != e; ++i) {
      ఒకవేళ (i == SK.Tok)
        ఆపు;
    }
    ఒకవేళ (i == e) {
      Failed = నిజం;
      తిరిగి అబద్ధం;
    }
    i = Token_వరుస.చొప్పించు(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.స్తంభం, Token::TK_ఖండం_Mapping_ప్రారంభం, i);

    Is_Simple_కీలకం_Allowed = అబద్ధం;
  } లేకపోతే {
    ఒకవేళ (!FlowLevel)
      rollIndent(స్తంభం, Token::TK_ఖండం_Mapping_ప్రారంభం, Token_వరుస.చివర());
    Is_Simple_కీలకం_Allowed = !FlowLevel;
  }
  Is_Adjacent_విలువ_Allowed_లోపల_Flow = అబద్ధం;

  Token T;
  T.రకం_3 = Token::TK_విలువ;
  T.Range = StringRef(Current, 1);
  skip(1);
  Token_వరుస.నెట్టు_వెనుక(T);
  తిరిగి నిజం;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE స్థిర తార్కిక
wasEscaped(StringRef::iterator మొదటి, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
స్థిర తార్కిక wasEscaped(StringRef::iterator మొదటి,
                       StringRef::iterator Position) {
  ప్రతిజ్ఞా(Position - 1 >= మొదటి);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  అయితే (I >= మొదటి && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  తిరిగి (Position - 1 - I) % 2 == 1;
}

తార్కిక Scanner::పరిశీలన_Flow_Scalar(తార్కిక IsDoubleQuoted) {
  StringRef::iterator ప్రారంభం = Current;
  గుర్తులేని నిలువు_ప్రారంభం = స్తంభం;
  ఒకవేళ (IsDoubleQuoted) {
    చేయి {
      ++Current;
      అయితే (Current != చివర_2 && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } అయితే (   Current != చివర_2
             && *(Current - 1) == '\\'
             && wasEscaped(ప్రారంభం + 1, Current));
  } లేకపోతే {
    skip(1);
    అయితే (Current != చివర_2) {
      // Skip a ' followed by another '.
      ఒకవేళ (Current + 1 < చివర_2 && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        కొనసాగు;
      } లేకపోతే ఒకవేళ (*Current == '\'')
        ఆపు;
      StringRef::iterator i = skip_nb_char(Current);
      ఒకవేళ (i == Current) {
        i = skip_b_break(Current);
        ఒకవేళ (i == Current)
          ఆపు;
        Current = i;
        స్తంభం = 0;
        ++పంక్తి;
      } లేకపోతే {
        ఒకవేళ (i == చివర_2)
          ఆపు;
        Current = i;
        ++స్తంభం;
      }
    }
  }

  ఒకవేళ (Current == చివర_2) {
    సమితి_Error("Expected quote at end of scalar", Current);
    తిరిగి అబద్ధం;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.రకం_3 = Token::TK_Scalar;
  T.Range = StringRef(ప్రారంభం, Current - ప్రారంభం);
  Token_వరుస.నెట్టు_వెనుక(T);

  save_Simple_కీలకం_Candidate(--Token_వరుస.చివర(), నిలువు_ప్రారంభం, అబద్ధం);

  Is_Simple_కీలకం_Allowed = అబద్ధం;
  Is_Adjacent_విలువ_Allowed_లోపల_Flow = నిజం;

  తిరిగి నిజం;
}

తార్కిక Scanner::పరిశీలన_Plain_Scalar() {
  StringRef::iterator ప్రారంభం = Current;
  గుర్తులేని నిలువు_ప్రారంభం = స్తంభం;
  గుర్తులేని LeadingBlanks = 0;
  ప్రతిజ్ఞా(Indent >= -1 && "Indent must be >= -1 !");
  గుర్తులేని indent = static_cast<గుర్తులేని>(Indent + 1);
  అయితే (Current != చివర_2) {
    ఒకవేళ (*Current == '#')
      ఆపు;

    అయితే (Current != చివర_2 &&
           ((*Current != ':' && is_Plain_సురక్షిత_Non_Blank(Current)) ||
            (*Current == ':' && is_Plain_సురక్షిత_Non_Blank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      ఒకవేళ (i == Current)
        ఆపు;
      Current = i;
      ++స్తంభం;
    }

    // Are we at the end?
    ఒకవేళ (!isBlankOrBreak(Current))
      ఆపు;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    అయితే (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      ఒకవేళ (i != Tmp) {
        ఒకవేళ (LeadingBlanks && (స్తంభం < indent) && *Tmp == '\t') {
          సమితి_Error("Found invalid tab character in indentation", Tmp);
          తిరిగి అబద్ధం;
        }
        Tmp = i;
        ++స్తంభం;
      } లేకపోతే {
        i = skip_b_break(Tmp);
        ఒకవేళ (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        స్తంభం = 0;
        ++పంక్తి;
      }
    }

    ఒకవేళ (!FlowLevel && స్తంభం < indent)
      ఆపు;

    Current = Tmp;
  }
  ఒకవేళ (ప్రారంభం == Current) {
    సమితి_Error("Got empty plain scalar", ప్రారంభం);
    తిరిగి అబద్ధం;
  }
  Token T;
  T.రకం_3 = Token::TK_Scalar;
  T.Range = StringRef(ప్రారంభం, Current - ప్రారంభం);
  Token_వరుస.నెట్టు_వెనుక(T);

  // Plain scalars can be simple keys.
  save_Simple_కీలకం_Candidate(--Token_వరుస.చివర(), నిలువు_ప్రారంభం, అబద్ధం);

  Is_Simple_కీలకం_Allowed = అబద్ధం;
  Is_Adjacent_విలువ_Allowed_లోపల_Flow = అబద్ధం;

  తిరిగి నిజం;
}

తార్కిక Scanner::పరిశీలన_Alias_Or_Anchor(తార్కిక IsAlias) {
  StringRef::iterator ప్రారంభం = Current;
  గుర్తులేని నిలువు_ప్రారంభం = స్తంభం;
  skip(1);
  అయితే (Current != చివర_2) {
    ఒకవేళ (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      ఆపు;
    StringRef::iterator i = skip_ns_char(Current);
    ఒకవేళ (i == Current)
      ఆపు;
    Current = i;
    ++స్తంభం;
  }

  ఒకవేళ (ప్రారంభం + 1 == Current) {
    సమితి_Error("Got empty alias or anchor", ప్రారంభం);
    తిరిగి అబద్ధం;
  }

  Token T;
  T.రకం_3 = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(ప్రారంభం, Current - ప్రారంభం);
  Token_వరుస.నెట్టు_వెనుక(T);

  // Alias and anchors can be simple keys.
  save_Simple_కీలకం_Candidate(--Token_వరుస.చివర(), నిలువు_ప్రారంభం, అబద్ధం);

  Is_Simple_కీలకం_Allowed = అబద్ధం;
  Is_Adjacent_విలువ_Allowed_లోపల_Flow = అబద్ధం;

  తిరిగి నిజం;
}

తార్కిక Scanner::పరిశీలన_ఖండం_Scalar_Indicators(అక్షరం &StyleIndicator,
                                        అక్షరం &ChompingIndicator,
                                        గుర్తులేని &IndentIndicator,
                                        తార్కిక &Is_పూర్తి) {
  StyleIndicator = పరిశీలన_ఖండం_Style_Indicator();
  ఒకవేళ (!పరిశీలన_ఖండం_Scalar_శీర్షిక(ChompingIndicator, IndentIndicator, Is_పూర్తి))
    తిరిగి అబద్ధం;
  తిరిగి నిజం;
}

అక్షరం Scanner::పరిశీలన_ఖండం_Style_Indicator() {
  అక్షరం Indicator = ' ';
  ఒకవేళ (Current != చివర_2 && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  తిరిగి Indicator;
}

అక్షరం Scanner::పరిశీలన_ఖండం_Chomping_Indicator() {
  అక్షరం Indicator = ' ';
  ఒకవేళ (Current != చివర_2 && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  తిరిగి Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
స్థిర గుర్తులేని get_Chomped_పంక్తి_Breaks(అక్షరం ChompingIndicator,
                                     గుర్తులేని పంక్తి_Breaks, StringRef Str) {
  ఒకవేళ (ChompingIndicator == '-') // Strip all line breaks.
    తిరిగి 0;
  ఒకవేళ (ChompingIndicator == '+') // Keep all line breaks.
    తిరిగి పంక్తి_Breaks;
  // Clip trailing lines.
  తిరిగి Str.ఖాళీలు() ? 0 : 1;
}

గుర్తులేని Scanner::పరిశీలన_ఖండం_Indentation_Indicator() {
  గుర్తులేని Indent = 0;
  ఒకవేళ (Current != చివర_2 && (*Current >= '1' && *Current <= '9')) {
    Indent = గుర్తులేని(*Current - '0');
    skip(1);
  }
  తిరిగి Indent;
}

తార్కిక Scanner::పరిశీలన_ఖండం_Scalar_శీర్షిక(అక్షరం &ChompingIndicator,
                                    గుర్తులేని &IndentIndicator, తార్కిక &Is_పూర్తి) {
  స్వయంచాలక ప్రారంభం = Current;

  ChompingIndicator = పరిశీలన_ఖండం_Chomping_Indicator();
  IndentIndicator = పరిశీలన_ఖండం_Indentation_Indicator();
  // Check for the chomping indicator once again.
  ఒకవేళ (ChompingIndicator == ' ')
    ChompingIndicator = పరిశీలన_ఖండం_Chomping_Indicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  ఒకవేళ (Current == చివర_2) { // EOF, we have an empty scalar.
    Token T;
    T.రకం_3 = Token::TK_ఖండం_Scalar;
    T.Range = StringRef(ప్రారంభం, Current - ప్రారంభం);
    Token_వరుస.నెట్టు_వెనుక(T);
    Is_పూర్తి = నిజం;
    తిరిగి నిజం;
  }

  ఒకవేళ (!consume_పంక్తి_Break_If_Present()) {
    సమితి_Error("Expected a line break after block scalar header", Current);
    తిరిగి అబద్ధం;
  }
  తిరిగి నిజం;
}

తార్కిక Scanner::కనుగొను_ఖండం_Scalar_Indent(గుర్తులేని &ఖండం_Indent,
                                    గుర్తులేని ఖండం_Exit_Indent,
                                    గుర్తులేని &పంక్తి_Breaks, తార్కిక &Is_పూర్తి) {
  గుర్తులేని Max_All_Space_పంక్తి_Characters = 0;
  StringRef::iterator Longest_All_Space_పంక్తి;

  అయితే (నిజం) {
    advanceWhile(&Scanner::skip_s_space);
    ఒకవేళ (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      ఒకవేళ (స్తంభం <= ఖండం_Exit_Indent) { // End of the block literal.
        Is_పూర్తి = నిజం;
        తిరిగి నిజం;
      }
      // We found the block's indentation.
      ఖండం_Indent = స్తంభం;
      ఒకవేళ (Max_All_Space_పంక్తి_Characters > ఖండం_Indent) {
        సమితి_Error(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_పంక్తి);
        తిరిగి అబద్ధం;
      }
      తిరిగి నిజం;
    }
    ఒకవేళ (skip_b_break(Current) != Current &&
        స్తంభం > Max_All_Space_పంక్తి_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_పంక్తి_Characters = స్తంభం;
      Longest_All_Space_పంక్తి = Current;
    }

    // Check for EOF.
    ఒకవేళ (Current == చివర_2) {
      Is_పూర్తి = నిజం;
      తిరిగి నిజం;
    }

    ఒకవేళ (!consume_పంక్తి_Break_If_Present()) {
      Is_పూర్తి = నిజం;
      తిరిగి నిజం;
    }
    ++పంక్తి_Breaks;
  }
  తిరిగి నిజం;
}

తార్కిక Scanner::పరిశీలన_ఖండం_Scalar_Indent(గుర్తులేని ఖండం_Indent,
                                    గుర్తులేని ఖండం_Exit_Indent, తార్కిక &Is_పూర్తి) {
  // Skip the indentation.
  అయితే (స్తంభం < ఖండం_Indent) {
    స్వయంచాలక I = skip_s_space(Current);
    ఒకవేళ (I == Current)
      ఆపు;
    Current = I;
    ++స్తంభం;
  }

  ఒకవేళ (skip_nb_char(Current) == Current)
    తిరిగి నిజం;

  ఒకవేళ (స్తంభం <= ఖండం_Exit_Indent) { // End of the block literal.
    Is_పూర్తి = నిజం;
    తిరిగి నిజం;
  }

  ఒకవేళ (స్తంభం < ఖండం_Indent) {
    ఒకవేళ (Current != చివర_2 && *Current == '#') { // Trailing comment.
      Is_పూర్తి = నిజం;
      తిరిగి నిజం;
    }
    సమితి_Error("A text line is less indented than the block scalar", Current);
    తిరిగి అబద్ధం;
  }
  తిరిగి నిజం; // A normal text line.
}

తార్కిక Scanner::పరిశీలన_ఖండం_Scalar(తార్కిక IsLiteral) {
  ప్రతిజ్ఞా(*Current == '|' || *Current == '>');
  అక్షరం StyleIndicator;
  అక్షరం ChompingIndicator;
  గుర్తులేని ఖండం_Indent;
  తార్కిక Is_పూర్తి = అబద్ధం;
  ఒకవేళ (!పరిశీలన_ఖండం_Scalar_Indicators(StyleIndicator, ChompingIndicator, ఖండం_Indent,
                                 Is_పూర్తి))
    తిరిగి అబద్ధం;
  ఒకవేళ (Is_పూర్తి)
    తిరిగి నిజం;
  తార్కిక IsFolded = StyleIndicator == '>';

  స్థిరరాశి స్వయంచాలక *ప్రారంభం = Current;
  గుర్తులేని ఖండం_Exit_Indent = Indent < 0 ? 0 : (గుర్తులేని)Indent;
  గుర్తులేని పంక్తి_Breaks = 0;
  ఒకవేళ (ఖండం_Indent == 0) {
    ఒకవేళ (!కనుగొను_ఖండం_Scalar_Indent(ఖండం_Indent, ఖండం_Exit_Indent, పంక్తి_Breaks,
                               Is_పూర్తి))
      తిరిగి అబద్ధం;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  అయితే (!Is_పూర్తి) {
    ఒకవేళ (!పరిశీలన_ఖండం_Scalar_Indent(ఖండం_Indent, ఖండం_Exit_Indent, Is_పూర్తి))
      తిరిగి అబద్ధం;
    ఒకవేళ (Is_పూర్తి)
      ఆపు;

    // Parse the current line.
    స్వయంచాలక పంక్తి_ప్రారంభం = Current;
    advanceWhile(&Scanner::skip_nb_char);
    ఒకవేళ (పంక్తి_ప్రారంభం != Current) {
      ఒకవేళ (పంక్తి_Breaks && IsFolded && !Scanner::is_పంక్తి_ఖాళీలు(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        ఒకవేళ (పంక్తి_Breaks == 1) {
          Str.append(పంక్తి_Breaks,
                     is_పంక్తి_ఖాళీలు(StringRef(పంక్తి_ప్రారంభం, Current - పంక్తి_ప్రారంభం))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        పంక్తి_Breaks--;
      }
      Str.append(పంక్తి_Breaks, '\n');
      Str.append(StringRef(పంక్తి_ప్రారంభం, Current - పంక్తి_ప్రారంభం));
      పంక్తి_Breaks = 0;
    }

    // Check for EOF.
    ఒకవేళ (Current == చివర_2)
      ఆపు;

    ఒకవేళ (!consume_పంక్తి_Break_If_Present())
      ఆపు;
    ++పంక్తి_Breaks;
  }

  ఒకవేళ (Current == చివర_2 && !పంక్తి_Breaks)
    // Ensure that there is at least one line break before the end of file.
    పంక్తి_Breaks = 1;
  Str.append(get_Chomped_పంక్తి_Breaks(ChompingIndicator, పంక్తి_Breaks, Str), '\n');

  // New lines may start a simple key.
  ఒకవేళ (!FlowLevel)
    Is_Simple_కీలకం_Allowed = నిజం;
  Is_Adjacent_విలువ_Allowed_లోపల_Flow = అబద్ధం;

  Token T;
  T.రకం_3 = Token::TK_ఖండం_Scalar;
  T.Range = StringRef(ప్రారంభం, Current - ప్రారంభం);
  T.విలువ = std::string(Str);
  Token_వరుస.నెట్టు_వెనుక(T);
  తిరిగి నిజం;
}

తార్కిక Scanner::పరిశీలన_ట్యాగ్() {
  StringRef::iterator ప్రారంభం = Current;
  గుర్తులేని నిలువు_ప్రారంభం = స్తంభం;
  skip(1); // Eat !.
  ఒకవేళ (Current == చివర_2 || isBlankOrBreak(Current)); // An empty tag.
  లేకపోతే ఒకవేళ (*Current == '<') {
    skip(1);
    పరిశీలన_ns_uri_char();
    ఒకవేళ (!consume('>'))
      తిరిగి అబద్ధం;
  } లేకపోతే {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.రకం_3 = Token::TK_ట్యాగ్;
  T.Range = StringRef(ప్రారంభం, Current - ప్రారంభం);
  Token_వరుస.నెట్టు_వెనుక(T);

  // Tags can be simple keys.
  save_Simple_కీలకం_Candidate(--Token_వరుస.చివర(), నిలువు_ప్రారంభం, అబద్ధం);

  Is_Simple_కీలకం_Allowed = అబద్ధం;
  Is_Adjacent_విలువ_Allowed_లోపల_Flow = అబద్ధం;

  తిరిగి నిజం;
}

తార్కిక Scanner::fetch_మరిన్ని_టోకెన్లు() {
  ఒకవేళ (Is_ప్రారంభం_యొక్క_ప్రవాహం)
    తిరిగి పరిశీలన_ప్రవాహం_ప్రారంభం();

  పరిశీలన_గమ్యం_తదుపరి_Token();

  ఒకవేళ (Current == చివర_2)
    తిరిగి పరిశీలన_ప్రవాహం_చివర();

  remove_Stale_Simple_కీలకం_Candidates();

  unrollIndent(స్తంభం);

  ఒకవేళ (స్తంభం == 0 && *Current == '%')
    తిరిగి పరిశీలన_Directive();

  ఒకవేళ (స్తంభం == 0 && Current + 3 <= చివర_2
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == చివర_2 || isBlankOrBreak(Current + 3)))
    తిరిగి పరిశీలన_Document_Indicator(నిజం);

  ఒకవేళ (స్తంభం == 0 && Current + 3 <= చివర_2
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == చివర_2 || isBlankOrBreak(Current + 3)))
    తిరిగి పరిశీలన_Document_Indicator(అబద్ధం);

  ఒకవేళ (*Current == '[')
    తిరిగి పరిశీలన_Flow_Collection_ప్రారంభం(నిజం);

  ఒకవేళ (*Current == '{')
    తిరిగి పరిశీలన_Flow_Collection_ప్రారంభం(అబద్ధం);

  ఒకవేళ (*Current == ']')
    తిరిగి పరిశీలన_Flow_Collection_చివర(నిజం);

  ఒకవేళ (*Current == '}')
    తిరిగి పరిశీలన_Flow_Collection_చివర(అబద్ధం);

  ఒకవేళ (*Current == ',')
    తిరిగి పరిశీలన_Flow_నమోదు();

  ఒకవేళ (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == చివర_2))
    తిరిగి పరిశీలన_ఖండం_నమోదు();

  ఒకవేళ (*Current == '?' && (Current + 1 == చివర_2 || isBlankOrBreak(Current + 1)))
    తిరిగి పరిశీలన_కీలకం();

  ఒకవేళ (*Current == ':' &&
      (!is_Plain_సురక్షిత_Non_Blank(Current + 1) || Is_Adjacent_విలువ_Allowed_లోపల_Flow))
    తిరిగి పరిశీలన_విలువ();

  ఒకవేళ (*Current == '*')
    తిరిగి పరిశీలన_Alias_Or_Anchor(నిజం);

  ఒకవేళ (*Current == '&')
    తిరిగి పరిశీలన_Alias_Or_Anchor(అబద్ధం);

  ఒకవేళ (*Current == '!')
    తిరిగి పరిశీలన_ట్యాగ్();

  ఒకవేళ (*Current == '|' && !FlowLevel)
    తిరిగి పరిశీలన_ఖండం_Scalar(నిజం);

  ఒకవేళ (*Current == '>' && !FlowLevel)
    తిరిగి పరిశీలన_ఖండం_Scalar(అబద్ధం);

  ఒకవేళ (*Current == '\'')
    తిరిగి పరిశీలన_Flow_Scalar(అబద్ధం);

  ఒకవేళ (*Current == '"')
    తిరిగి పరిశీలన_Flow_Scalar(నిజం);

  // Get a plain scalar.
  StringRef మొదటి_Char(Current, 1);
  ఒకవేళ ((!isBlankOrBreak(Current) &&
       మొదటి_Char.కనుగొను_మొదటి_యొక్క("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (మొదటి_Char.కనుగొను_మొదటి_యొక్క("?:-") != StringRef::npos &&
       is_Plain_సురక్షిత_Non_Blank(Current + 1)))
    తిరిగి పరిశీలన_Plain_Scalar();

  సమితి_Error("Unrecognized character while tokenizing.", Current);
  తిరిగి అబద్ధం;
}

ప్రవాహం::ప్రవాహం(StringRef ఆదానం, ఆధారం_Mgr &SM, తార్కిక ShowColors,
               std::error_సంకేతం *EC)
    : scanner(కొత్త Scanner(ఆదానం, SM, ShowColors, EC)) {}

ప్రవాహం::ప్రవాహం(జ్ఞాపకం_బఫరు_Ref ఆదానం_బఫరు, ఆధారం_Mgr &SM, తార్కిక ShowColors,
               std::error_సంకేతం *EC)
    : scanner(కొత్త Scanner(ఆదానం_బఫరు, SM, ShowColors, EC)) {}

ప్రవాహం::~ప్రవాహం() = అప్రమేయ;

తార్కిక ప్రవాహం::failed() { తిరిగి scanner->failed(); }

శూన్య ప్రవాహం::printError(కణుపు *N, స్థిరరాశి Twine &సందేశం_4, ఆధారం_Mgr::Diag_రకం రకం_3) {
  printError(N ? N->get_ఆధారం_Range() : SMRange(), సందేశం_4, రకం_3);
}

శూన్య ప్రవాహం::printError(స్థిరరాశి SMRange &Range, స్థిరరాశి Twine &సందేశం_4,
                        ఆధారం_Mgr::Diag_రకం రకం_3) {
  scanner->printError(Range.ప్రారంభం, రకం_3, సందేశం_4, Range);
}

document_iterator ప్రవాహం::begin() {
  ఒకవేళ (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->get_తదుపరి();

  CurrentDoc.reset(కొత్త Document(*ఇది));
  తిరిగి document_iterator(CurrentDoc);
}

document_iterator ప్రవాహం::చివర() {
  తిరిగి document_iterator();
}

శూన్య ప్రవాహం::skip() {
  కొరకు (Document &Doc : *ఇది)
    Doc.skip();
}

కణుపు::కణుపు(గుర్తులేని పూర్ణాంకం Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), ట్యాగ్(T) {
  SMLoc ప్రారంభం = SMLoc::get_From_సూచిక(peek_తదుపరి().Range.begin());
  ఆధారం_Range = SMRange(ప్రారంభం, ప్రారంభం);
}

std::string కణుపు::get_Verbatim_ట్యాగ్() స్థిరరాశి {
  StringRef Raw = get_Raw_ట్యాగ్();
  ఒకవేళ (!Raw.ఖాళీలు() && Raw != "!") {
    std::string Ret;
    ఒకవేళ (Raw.కనుగొను_చివరి_యొక్క('!') == 0) {
      Ret = std::string(Doc->get_ట్యాగ్_Map().కనుగొను("!")->రెండవ_2);
      Ret += Raw.substr(1);
      తిరిగి Ret;
    } లేకపోతే ఒకవేళ (Raw.starts_with("!!")) {
      Ret = std::string(Doc->get_ట్యాగ్_Map().కనుగొను("!!")->రెండవ_2);
      Ret += Raw.substr(2);
      తిరిగి Ret;
    } లేకపోతే {
      StringRef ట్యాగ్_Handle = Raw.substr(0, Raw.కనుగొను_చివరి_యొక్క('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->get_ట్యాగ్_Map().కనుగొను(ట్యాగ్_Handle);
      ఒకవేళ (It != Doc->get_ట్యాగ్_Map().చివర())
        Ret = std::string(It->రెండవ_2);
      లేకపోతే {
        Token T;
        T.రకం_3 = Token::TK_ట్యాగ్;
        T.Range = ట్యాగ్_Handle;
        సమితి_Error(Twine("Unknown tag handle ") + ట్యాగ్_Handle, T);
      }
      Ret += Raw.substr(Raw.కనుగొను_చివరి_యొక్క('!') + 1);
      తిరిగి Ret;
    }
  }

  ఎంపిక (getType()) {
  స్థితి NK_Null:
    తిరిగి "tag:yaml.org,2002:null";
  స్థితి NK_Scalar:
  స్థితి NK_ఖండం_Scalar:
    // TODO: Tag resolution.
    తిరిగి "tag:yaml.org,2002:str";
  స్థితి NK_Mapping:
    తిరిగి "tag:yaml.org,2002:map";
  స్థితి NK_Sequence:
    తిరిగి "tag:yaml.org,2002:seq";
  }

  తిరిగి "";
}

Token &కణుపు::peek_తదుపరి() {
  తిరిగి Doc->peek_తదుపరి();
}

Token కణుపు::get_తదుపరి() {
  తిరిగి Doc->get_తదుపరి();
}

కణుపు *కణుపు::parse_ఖండం_కణుపు() {
  తిరిగి Doc->parse_ఖండం_కణుపు();
}

BumpPtrAllocator &కణుపు::getAllocator() {
  తిరిగి Doc->కణుపు_Allocator;
}

శూన్య కణుపు::సమితి_Error(స్థిరరాశి Twine &సందేశం_4, Token &Tok) స్థిరరాశి {
  Doc->సమితి_Error(సందేశం_4, Tok);
}

తార్కిక కణుపు::failed() స్థిరరాశి {
  తిరిగి Doc->failed();
}

StringRef Scalar_కణుపు::get_విలువ(Small_సదిశ_Impl<అక్షరం> &Storage) స్థిరరాశి {
  ఒకవేళ (విలువ[0] == '"')
    తిరిగి get_Double_Quoted_విలువ(విలువ, Storage);
  ఒకవేళ (విలువ[0] == '\'')
    తిరిగి get_Single_Quoted_విలువ(విలువ, Storage);
  తిరిగి get_Plain_విలువ(విలువ, Storage);
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
స్థిర StringRef
parse_Scalar_విలువ(StringRef Unquoted_విలువ, Small_సదిశ_Impl<అక్షరం> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, Small_సదిశ_Impl<అక్షరం> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_విలువ.కనుగొను_మొదటి_యొక్క(LookupChars);
  ఒకవేళ (I == StringRef::npos)
    తిరిగి Unquoted_విలువ;

  Storage.clear();
  Storage.reserve(Unquoted_విలువ.కొలత_2());
  అక్షరం చివరి_New_పంక్తి_Added_As = '\0';
  కొరకు (; I != StringRef::npos; I = Unquoted_విలువ.కనుగొను_మొదటి_యొక్క(LookupChars)) {
    ఒకవేళ (Unquoted_విలువ[I] != '\r' && Unquoted_విలువ[I] != '\n') {
      llvm::append_range(Storage, Unquoted_విలువ.take_front(I));
      Unquoted_విలువ = UnescapeCallback(Unquoted_విలువ.drop_front(I), Storage);
      చివరి_New_పంక్తి_Added_As = '\0';
      కొనసాగు;
    }
    ఒకవేళ (size_t చివరి_Non_SWhite = Unquoted_విలువ.కనుగొను_చివరి_not_యొక్క(" \t", I);
        చివరి_Non_SWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_విలువ.take_front(చివరి_Non_SWhite + 1));
      Storage.నెట్టు_వెనుక(' ');
      చివరి_New_పంక్తి_Added_As = ' ';
    } లేకపోతే {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      ఎంపిక (చివరి_New_పంక్తి_Added_As) {
      స్థితి ' ':
        ప్రతిజ్ఞా(!Storage.ఖాళీలు() && Storage.వెనుక() == ' ');
        Storage.వెనుక() = '\n';
        చివరి_New_పంక్తి_Added_As = '\n';
        ఆపు;
      స్థితి '\n':
        ప్రతిజ్ఞా(!Storage.ఖాళీలు() && Storage.వెనుక() == '\n');
        Storage.నెట్టు_వెనుక('\n');
        ఆపు;
      అప్రమేయ:
        Storage.నెట్టు_వెనుక(' ');
        చివరి_New_పంక్తి_Added_As = ' ';
        ఆపు;
      }
    }
    // Handle Windows-style EOL
    ఒకవేళ (Unquoted_విలువ.substr(I, 2) == "\r\n")
      I++;
    Unquoted_విలువ = Unquoted_విలువ.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_విలువ);
  తిరిగి StringRef(Storage.begin(), Storage.కొలత_2());
}

StringRef
Scalar_కణుపు::get_Double_Quoted_విలువ(StringRef Raw_విలువ,
                                 Small_సదిశ_Impl<అక్షరం> &Storage) స్థిరరాశి {
  ప్రతిజ్ఞా(Raw_విలువ.కొలత_2() >= 2 && Raw_విలువ.front() == '"' &&
         Raw_విలువ.వెనుక() == '"');
  StringRef Unquoted_విలువ = Raw_విలువ.substr(1, Raw_విలువ.కొలత_2() - 2);

  స్వయంచాలక UnescapeFunc = [ఇది](StringRef Unquoted_విలువ,
                             Small_సదిశ_Impl<అక్షరం> &Storage) {
    ప్రతిజ్ఞా(Unquoted_విలువ.take_front(1) == "\\");
    ఒకవేళ (Unquoted_విలువ.కొలత_2() == 1) {
      Token T;
      T.Range = Unquoted_విలువ;
      సమితి_Error("Unrecognized escape code", T);
      Storage.clear();
      తిరిగి StringRef();
    }
    Unquoted_విలువ = Unquoted_విలువ.drop_front(1);
    ఎంపిక (Unquoted_విలువ[0]) {
    అప్రమేయ: {
      Token T;
      T.Range = Unquoted_విలువ.take_front(1);
      సమితి_Error("Unrecognized escape code", T);
      Storage.clear();
      తిరిగి StringRef();
    }
    స్థితి '\r':
      // Shrink the Windows-style EOL.
      ఒకవేళ (Unquoted_విలువ.కొలత_2() >= 2 && Unquoted_విలువ[1] == '\n')
        Unquoted_విలువ = Unquoted_విలువ.drop_front(1);
      [[fallthrough]];
    స్థితి '\n':
      తిరిగి Unquoted_విలువ.drop_front(1).ltrim(" \t");
    స్థితి '0':
      Storage.నెట్టు_వెనుక(0x00);
      ఆపు;
    స్థితి 'a':
      Storage.నెట్టు_వెనుక(0x07);
      ఆపు;
    స్థితి 'b':
      Storage.నెట్టు_వెనుక(0x08);
      ఆపు;
    స్థితి 't':
    స్థితి 0x09:
      Storage.నెట్టు_వెనుక(0x09);
      ఆపు;
    స్థితి 'n':
      Storage.నెట్టు_వెనుక(0x0A);
      ఆపు;
    స్థితి 'v':
      Storage.నెట్టు_వెనుక(0x0B);
      ఆపు;
    స్థితి 'f':
      Storage.నెట్టు_వెనుక(0x0C);
      ఆపు;
    స్థితి 'r':
      Storage.నెట్టు_వెనుక(0x0D);
      ఆపు;
    స్థితి 'e':
      Storage.నెట్టు_వెనుక(0x1B);
      ఆపు;
    స్థితి ' ':
      Storage.నెట్టు_వెనుక(0x20);
      ఆపు;
    స్థితి '"':
      Storage.నెట్టు_వెనుక(0x22);
      ఆపు;
    స్థితి '/':
      Storage.నెట్టు_వెనుక(0x2F);
      ఆపు;
    స్థితి '\\':
      Storage.నెట్టు_వెనుక(0x5C);
      ఆపు;
    స్థితి 'N':
      encodeUTF8(0x85, Storage);
      ఆపు;
    స్థితి '_':
      encodeUTF8(0xA0, Storage);
      ఆపు;
    స్థితి 'L':
      encodeUTF8(0x2028, Storage);
      ఆపు;
    స్థితి 'P':
      encodeUTF8(0x2029, Storage);
      ఆపు;
    స్థితి 'x': {
      ఒకవేళ (Unquoted_విలువ.కొలత_2() < 3)
        // TODO: Report error.
        ఆపు;
      గుర్తులేని పూర్ణాంకం Unicode_Scalar_విలువ;
      ఒకవేళ (Unquoted_విలువ.substr(1, 2).getAsInteger(16, Unicode_Scalar_విలువ))
        // TODO: Report error.
        Unicode_Scalar_విలువ = 0xFFFD;
      encodeUTF8(Unicode_Scalar_విలువ, Storage);
      తిరిగి Unquoted_విలువ.drop_front(3);
    }
    స్థితి 'u': {
      ఒకవేళ (Unquoted_విలువ.కొలత_2() < 5)
        // TODO: Report error.
        ఆపు;
      గుర్తులేని పూర్ణాంకం Unicode_Scalar_విలువ;
      ఒకవేళ (Unquoted_విలువ.substr(1, 4).getAsInteger(16, Unicode_Scalar_విలువ))
        // TODO: Report error.
        Unicode_Scalar_విలువ = 0xFFFD;
      encodeUTF8(Unicode_Scalar_విలువ, Storage);
      తిరిగి Unquoted_విలువ.drop_front(5);
    }
    స్థితి 'U': {
      ఒకవేళ (Unquoted_విలువ.కొలత_2() < 9)
        // TODO: Report error.
        ఆపు;
      గుర్తులేని పూర్ణాంకం Unicode_Scalar_విలువ;
      ఒకవేళ (Unquoted_విలువ.substr(1, 8).getAsInteger(16, Unicode_Scalar_విలువ))
        // TODO: Report error.
        Unicode_Scalar_విలువ = 0xFFFD;
      encodeUTF8(Unicode_Scalar_విలువ, Storage);
      తిరిగి Unquoted_విలువ.drop_front(9);
    }
    }
    తిరిగి Unquoted_విలువ.drop_front(1);
  };

  తిరిగి parse_Scalar_విలువ(Unquoted_విలువ, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_కణుపు::get_Single_Quoted_విలువ(StringRef Raw_విలువ,
                                           Small_సదిశ_Impl<అక్షరం> &Storage) {
  ప్రతిజ్ఞా(Raw_విలువ.కొలత_2() >= 2 && Raw_విలువ.front() == '\'' &&
         Raw_విలువ.వెనుక() == '\'');
  StringRef Unquoted_విలువ = Raw_విలువ.substr(1, Raw_విలువ.కొలత_2() - 2);

  స్వయంచాలక UnescapeFunc = [](StringRef Unquoted_విలువ,
                         Small_సదిశ_Impl<అక్షరం> &Storage) {
    ప్రతిజ్ఞా(Unquoted_విలువ.take_front(2) == "''");
    Storage.నెట్టు_వెనుక('\'');
    తిరిగి Unquoted_విలువ.drop_front(2);
  };

  తిరిగి parse_Scalar_విలువ(Unquoted_విలువ, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_కణుపు::get_Plain_విలువ(StringRef Raw_విలువ,
                                    Small_సదిశ_Impl<అక్షరం> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_విలువ = Raw_విలువ.rtrim("\r\n \t");
  తిరిగి parse_Scalar_విలువ(Raw_విలువ, Storage, "\r\n", శూన్యసూచిక);
}

కణుపు *కీలకం_విలువ_కణుపు::get_కీలకం() {
  ఒకవేళ (కీలకం)
    తిరిగి కీలకం;
  // Handle implicit null keys.
  {
    Token &t = peek_తదుపరి();
    ఒకవేళ (   t.రకం_3 == Token::TK_ఖండం_చివర
        || t.రకం_3 == Token::TK_విలువ
        || t.రకం_3 == Token::TK_Error) {
      తిరిగి కీలకం = కొత్త (getAllocator()) Null_కణుపు(Doc);
    }
    ఒకవేళ (t.రకం_3 == Token::TK_కీలకం)
      get_తదుపరి(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peek_తదుపరి();
  ఒకవేళ (t.రకం_3 == Token::TK_ఖండం_చివర || t.రకం_3 == Token::TK_విలువ) {
    తిరిగి కీలకం = కొత్త (getAllocator()) Null_కణుపు(Doc);
  }

  // We've got a normal key.
  తిరిగి కీలకం = parse_ఖండం_కణుపు();
}

కణుపు *కీలకం_విలువ_కణుపు::get_విలువ() {
  ఒకవేళ (విలువ)
    తిరిగి విలువ;

  ఒకవేళ (కణుపు* కీలకం = get_కీలకం())
    కీలకం->skip();
  లేకపోతే {
    సమితి_Error("Null key in Key Value.", peek_తదుపరి());
    తిరిగి విలువ = కొత్త (getAllocator()) Null_కణుపు(Doc);
  }

  ఒకవేళ (failed())
    తిరిగి విలువ = కొత్త (getAllocator()) Null_కణుపు(Doc);

  // Handle implicit null values.
  {
    Token &t = peek_తదుపరి();
    ఒకవేళ (   t.రకం_3 == Token::TK_ఖండం_చివర
        || t.రకం_3 == Token::TK_Flow_Mapping_చివర
        || t.రకం_3 == Token::TK_కీలకం
        || t.రకం_3 == Token::TK_Flow_నమోదు
        || t.రకం_3 == Token::TK_Error) {
      తిరిగి విలువ = కొత్త (getAllocator()) Null_కణుపు(Doc);
    }

    ఒకవేళ (t.రకం_3 != Token::TK_విలువ) {
      సమితి_Error("Unexpected token in Key Value.", t);
      తిరిగి విలువ = కొత్త (getAllocator()) Null_కణుపు(Doc);
    }
    get_తదుపరి(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peek_తదుపరి();
  ఒకవేళ (t.రకం_3 == Token::TK_ఖండం_చివర || t.రకం_3 == Token::TK_కీలకం) {
    తిరిగి విలువ = కొత్త (getAllocator()) Null_కణుపు(Doc);
  }

  // We got a normal value.
  తిరిగి విలువ = parse_ఖండం_కణుపు();
}

శూన్య Mapping_కణుపు::increment() {
  ఒకవేళ (failed()) {
    Is_At_చివర = నిజం;
    Current_నమోదు = శూన్యసూచిక;
    తిరిగి;
  }
  ఒకవేళ (Current_నమోదు) {
    Current_నమోదు->skip();
    ఒకవేళ (Type == MT_Inline) {
      Is_At_చివర = నిజం;
      Current_నమోదు = శూన్యసూచిక;
      తిరిగి;
    }
  }
  Token T = peek_తదుపరి();
  ఒకవేళ (T.రకం_3 == Token::TK_కీలకం || T.రకం_3 == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    Current_నమోదు = కొత్త (getAllocator()) కీలకం_విలువ_కణుపు(Doc);
  } లేకపోతే ఒకవేళ (Type == MT_ఖండం) {
    ఎంపిక (T.రకం_3) {
    స్థితి Token::TK_ఖండం_చివర:
      get_తదుపరి();
      Is_At_చివర = నిజం;
      Current_నమోదు = శూన్యసూచిక;
      ఆపు;
    అప్రమేయ:
      సమితి_Error("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    స్థితి Token::TK_Error:
      Is_At_చివర = నిజం;
      Current_నమోదు = శూన్యసూచిక;
    }
  } లేకపోతే {
    ఎంపిక (T.రకం_3) {
    స్థితి Token::TK_Flow_నమోదు:
      // Eat the flow entry and recurse.
      get_తదుపరి();
      తిరిగి increment();
    స్థితి Token::TK_Flow_Mapping_చివర:
      get_తదుపరి();
      [[fallthrough]];
    స్థితి Token::TK_Error:
      // Set this to end iterator.
      Is_At_చివర = నిజం;
      Current_నమోదు = శూన్యసూచిక;
      ఆపు;
    అప్రమేయ:
      సమితి_Error( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      Is_At_చివర = నిజం;
      Current_నమోదు = శూన్యసూచిక;
    }
  }
}

శూన్య Sequence_కణుపు::increment() {
  ఒకవేళ (failed()) {
    Is_At_చివర = నిజం;
    Current_నమోదు = శూన్యసూచిక;
    తిరిగి;
  }
  ఒకవేళ (Current_నమోదు)
    Current_నమోదు->skip();
  Token T = peek_తదుపరి();
  ఒకవేళ (SeqType == ST_ఖండం) {
    ఎంపిక (T.రకం_3) {
    స్థితి Token::TK_ఖండం_నమోదు:
      get_తదుపరి();
      Current_నమోదు = parse_ఖండం_కణుపు();
      ఒకవేళ (!Current_నమోదు) { // An error occurred.
        Is_At_చివర = నిజం;
        Current_నమోదు = శూన్యసూచిక;
      }
      ఆపు;
    స్థితి Token::TK_ఖండం_చివర:
      get_తదుపరి();
      Is_At_చివర = నిజం;
      Current_నమోదు = శూన్యసూచిక;
      ఆపు;
    అప్రమేయ:
      సమితి_Error( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    స్థితి Token::TK_Error:
      Is_At_చివర = నిజం;
      Current_నమోదు = శూన్యసూచిక;
    }
  } లేకపోతే ఒకవేళ (SeqType == ST_Indentless) {
    ఎంపిక (T.రకం_3) {
    స్థితి Token::TK_ఖండం_నమోదు:
      get_తదుపరి();
      Current_నమోదు = parse_ఖండం_కణుపు();
      ఒకవేళ (!Current_నమోదు) { // An error occurred.
        Is_At_చివర = నిజం;
        Current_నమోదు = శూన్యసూచిక;
      }
      ఆపు;
    అప్రమేయ:
    స్థితి Token::TK_Error:
      Is_At_చివర = నిజం;
      Current_నమోదు = శూన్యసూచిక;
    }
  } లేకపోతే ఒకవేళ (SeqType == ST_Flow) {
    ఎంపిక (T.రకం_3) {
    స్థితి Token::TK_Flow_నమోదు:
      // Eat the flow entry and recurse.
      get_తదుపరి();
      Was_మునుపటి_Token_Flow_నమోదు = నిజం;
      తిరిగి increment();
    స్థితి Token::TK_Flow_Sequence_చివర:
      get_తదుపరి();
      [[fallthrough]];
    స్థితి Token::TK_Error:
      // Set this to end iterator.
      Is_At_చివర = నిజం;
      Current_నమోదు = శూన్యసూచిక;
      ఆపు;
    స్థితి Token::TK_ప్రవాహం_చివర:
    స్థితి Token::TK_Document_చివర:
    స్థితి Token::TK_Document_ప్రారంభం:
      సమితి_Error("Could not find closing ]!", T);
      // Set this to end iterator.
      Is_At_చివర = నిజం;
      Current_నమోదు = శూన్యసూచిక;
      ఆపు;
    అప్రమేయ:
      ఒకవేళ (!Was_మునుపటి_Token_Flow_నమోదు) {
        సమితి_Error("Expected , between entries!", T);
        Is_At_చివర = నిజం;
        Current_నమోదు = శూన్యసూచిక;
        ఆపు;
      }
      // Otherwise it must be a flow entry.
      Current_నమోదు = parse_ఖండం_కణుపు();
      ఒకవేళ (!Current_నమోదు) {
        Is_At_చివర = నిజం;
      }
      Was_మునుపటి_Token_Flow_నమోదు = అబద్ధం;
      ఆపు;
    }
  }
}

Document::Document(ప్రవాహం &S) : ప్రవాహం_2(S), మూలం_2(శూన్యసూచిక) {
  // Tag maps starts with two default mappings.
  ట్యాగ్_Map["!"] = "!";
  ట్యాగ్_Map["!!"] = "tag:yaml.org,2002:";

  ఒకవేళ (parseDirectives())
    expectToken(Token::TK_Document_ప్రారంభం);
  Token &T = peek_తదుపరి();
  ఒకవేళ (T.రకం_3 == Token::TK_Document_ప్రారంభం)
    get_తదుపరి();
}

తార్కిక Document::skip()  {
  ఒకవేళ (ప్రవాహం_2.scanner->failed())
    తిరిగి అబద్ధం;
  ఒకవేళ (!మూలం_2 && !get_మూలం())
    తిరిగి అబద్ధం;
  మూలం_2->skip();
  Token &T = peek_తదుపరి();
  ఒకవేళ (T.రకం_3 == Token::TK_ప్రవాహం_చివర)
    తిరిగి అబద్ధం;
  ఒకవేళ (T.రకం_3 == Token::TK_Document_చివర) {
    get_తదుపరి();
    తిరిగి skip();
  }
  తిరిగి నిజం;
}

Token &Document::peek_తదుపరి() {
  తిరిగి ప్రవాహం_2.scanner->peek_తదుపరి();
}

Token Document::get_తదుపరి() {
  తిరిగి ప్రవాహం_2.scanner->get_తదుపరి();
}

శూన్య Document::సమితి_Error(స్థిరరాశి Twine &సందేశం_3, Token &Location) స్థిరరాశి {
  ప్రవాహం_2.scanner->సమితి_Error(సందేశం_3, Location.Range.begin());
}

తార్కిక Document::failed() స్థిరరాశి {
  తిరిగి ప్రవాహం_2.scanner->failed();
}

కణుపు *Document::parse_ఖండం_కణుపు() {
  Token T = peek_తదుపరి();
  // Handle properties.
  Token AnchorInfo;
  Token ట్యాగ్_Info;
parse_property:
  ఎంపిక (T.రకం_3) {
  స్థితి Token::TK_Alias:
    get_తదుపరి();
    తిరిగి కొత్త (కణుపు_Allocator) Alias_కణుపు(ప్రవాహం_2.CurrentDoc, T.Range.substr(1));
  స్థితి Token::TK_Anchor:
    ఒకవేళ (AnchorInfo.రకం_3 == Token::TK_Anchor) {
      సమితి_Error("Already encountered an anchor for this node!", T);
      తిరిగి శూన్యసూచిక;
    }
    AnchorInfo = get_తదుపరి(); // Consume TK_Anchor.
    T = peek_తదుపరి();
    వెళ్ళు parse_property;
  స్థితి Token::TK_ట్యాగ్:
    ఒకవేళ (ట్యాగ్_Info.రకం_3 == Token::TK_ట్యాగ్) {
      సమితి_Error("Already encountered a tag for this node!", T);
      తిరిగి శూన్యసూచిక;
    }
    ట్యాగ్_Info = get_తదుపరి(); // Consume TK_Tag.
    T = peek_తదుపరి();
    వెళ్ళు parse_property;
  అప్రమేయ:
    ఆపు;
  }

  ఎంపిక (T.రకం_3) {
  స్థితి Token::TK_ఖండం_నమోదు:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    తిరిగి కొత్త (కణుపు_Allocator) Sequence_కణుపు( ప్రవాహం_2.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , ట్యాగ్_Info.Range
                                           , Sequence_కణుపు::ST_Indentless);
  స్థితి Token::TK_ఖండం_Sequence_ప్రారంభం:
    get_తదుపరి();
    తిరిగి కొత్త (కణుపు_Allocator)
      Sequence_కణుపు( ప్రవాహం_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , ట్యాగ్_Info.Range
                  , Sequence_కణుపు::ST_ఖండం);
  స్థితి Token::TK_ఖండం_Mapping_ప్రారంభం:
    get_తదుపరి();
    తిరిగి కొత్త (కణుపు_Allocator)
      Mapping_కణుపు( ప్రవాహం_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , ట్యాగ్_Info.Range
                 , Mapping_కణుపు::MT_ఖండం);
  స్థితి Token::TK_Flow_Sequence_ప్రారంభం:
    get_తదుపరి();
    తిరిగి కొత్త (కణుపు_Allocator)
      Sequence_కణుపు( ప్రవాహం_2.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , ట్యాగ్_Info.Range
                  , Sequence_కణుపు::ST_Flow);
  స్థితి Token::TK_Flow_Mapping_ప్రారంభం:
    get_తదుపరి();
    తిరిగి కొత్త (కణుపు_Allocator)
      Mapping_కణుపు( ప్రవాహం_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , ట్యాగ్_Info.Range
                 , Mapping_కణుపు::MT_Flow);
  స్థితి Token::TK_Scalar:
    get_తదుపరి();
    తిరిగి కొత్త (కణుపు_Allocator)
      Scalar_కణుపు( ప్రవాహం_2.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , ట్యాగ్_Info.Range
                , T.Range);
  స్థితి Token::TK_ఖండం_Scalar: {
    get_తదుపరి();
    StringRef NullTerminatedStr(T.విలువ.c_str(), T.విలువ.నిడివి_2() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(కణుపు_Allocator).drop_వెనుక();
    తిరిగి కొత్త (కణుపు_Allocator)
        ఖండం_Scalar_కణుపు(ప్రవాహం_2.CurrentDoc, AnchorInfo.Range.substr(1),
                        ట్యాగ్_Info.Range, StrCopy, T.Range);
  }
  స్థితి Token::TK_కీలకం:
    // Don't eat the TK_Key, KeyValueNode expects it.
    తిరిగి కొత్త (కణుపు_Allocator)
      Mapping_కణుపు( ప్రవాహం_2.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , ట్యాగ్_Info.Range
                 , Mapping_కణుపు::MT_Inline);
  స్థితి Token::TK_Document_ప్రారంభం:
  స్థితి Token::TK_Document_చివర:
  స్థితి Token::TK_ప్రవాహం_చివర:
  అప్రమేయ:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    తిరిగి కొత్త (కణుపు_Allocator) Null_కణుపు(ప్రవాహం_2.CurrentDoc);
  స్థితి Token::TK_Flow_Mapping_చివర:
  స్థితి Token::TK_Flow_Sequence_చివర:
  స్థితి Token::TK_Flow_నమోదు: {
    ఒకవేళ (మూలం_2 && (isa<Mapping_కణుపు>(మూలం_2) || isa<Sequence_కణుపు>(మూలం_2)))
      తిరిగి కొత్త (కణుపు_Allocator) Null_కణుపు(ప్రవాహం_2.CurrentDoc);

    సమితి_Error("Unexpected token", T);
    తిరిగి శూన్యసూచిక;
  }
  స్థితి Token::TK_Error:
    తిరిగి శూన్యసూచిక;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  తిరిగి శూన్యసూచిక;
}

తార్కిక Document::parseDirectives() {
  తార్కిక isDirective = అబద్ధం;
  అయితే (నిజం) {
    Token T = peek_తదుపరి();
    ఒకవేళ (T.రకం_3 == Token::TK_ట్యాగ్_Directive) {
      parseTAGDirective();
      isDirective = నిజం;
    } లేకపోతే ఒకవేళ (T.రకం_3 == Token::TK_సంస్కరణ_Directive) {
      parseYAMLDirective();
      isDirective = నిజం;
    } లేకపోతే {
      ఆపు;
    }
  }
  తిరిగి isDirective;
}

శూన్య Document::parseYAMLDirective() {
  get_తదుపరి(); // Eat %YAML <version>
}

శూన్య Document::parseTAGDirective() {
  Token ట్యాగ్ = get_తదుపరి(); // %TAG <handle> <prefix>
  StringRef T = ట్యాగ్.Range;
  // Strip %TAG
  T = T.substr(T.కనుగొను_మొదటి_యొక్క(" \t")).ltrim(" \t");
  std::size_t Handle_చివర = T.కనుగొను_మొదటి_యొక్క(" \t");
  StringRef ట్యాగ్_Handle = T.substr(0, Handle_చివర);
  StringRef ట్యాగ్_ఉపసర్గ = T.substr(Handle_చివర).ltrim(" \t");
  ట్యాగ్_Map[ట్యాగ్_Handle] = ట్యాగ్_ఉపసర్గ;
}

తార్కిక Document::expectToken(పూర్ణాంకం TK) {
  Token T = get_తదుపరి();
  ఒకవేళ (T.రకం_3 != TK) {
    సమితి_Error("Unexpected token", T);
    తిరిగి అబద్ధం;
  }
  తిరిగి నిజం;
}
