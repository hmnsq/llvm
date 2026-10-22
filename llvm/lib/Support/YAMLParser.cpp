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

#समावेश "llvm/Support/YAMLParser.h"
#समावेश "llvm/ADT/AllocatorList.h"
#समावेश "llvm/ADT/ArrayRef.h"
#समावेश "llvm/ADT/STLExtras.h"
#समावेश "llvm/ADT/SmallString.h"
#समावेश "llvm/ADT/SmallVector.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/ADT/Twine.h"
#समावेश "llvm/Support/Compiler.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश "llvm/Support/MemoryBuffer.h"
#समावेश "llvm/Support/SMLoc.h"
#समावेश "llvm/Support/SourceMgr.h"
#समावेश "llvm/Support/Unicode.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <cassert>
#समावेश <cstddef>
#समावेश <cstdint>
#समावेश <map>
#समावेश <memory>
#समावेश <string>
#समावेश <system_error>
#समावेश <utility>

using namespace llvm;
using namespace yaml;

क्रमागत UnicodeEncodingForm {
  UEF_UTF32_LE, ///< UTF-32 Little Endian
  UEF_UTF32_BE, ///< UTF-32 Big Endian
  UEF_UTF16_LE, ///< UTF-16 Little Endian
  UEF_UTF16_BE, ///< UTF-16 Big Endian
  UEF_UTF8,     ///< UTF-8 or ascii.
  UEF_Unknown   ///< Not a valid Unicode encoding.
};

/// EncodingInfo - Holds the encoding type and length of the byte order mark if
///                it exists. Length is in {0, 2, 3, 4}.
using EncodingInfo = std::pair<UnicodeEncodingForm, अचिन्हित>;

/// getUnicodeEncoding - Reads up to the first 4 bytes to determine the Unicode
///                      encoding form of \a Input.
///
/// @param Input A string of length 0 or more.
/// @returns An EncodingInfo indicating the Unicode encoding form of the input
///          and how long the byte order mark is if one exists.
अटल EncodingInfo getUnicodeEncoding(StringRef निवेश) {
  अगर (निवेश.empty())
    वापस {UEF_Unknown, 0};

  चयन (uint8_t(निवेश[0])) {
  हाल 0x00:
    अगर (निवेश.आकार_2() >= 4) {
      अगर (  निवेश[1] == 0
         && uint8_t(निवेश[2]) == 0xFE
         && uint8_t(निवेश[3]) == 0xFF)
        वापस {UEF_UTF32_BE, 4};
      अगर (निवेश[1] == 0 && निवेश[2] == 0 && निवेश[3] != 0)
        वापस {UEF_UTF32_BE, 0};
    }

    अगर (निवेश.आकार_2() >= 2 && निवेश[1] != 0)
      वापस {UEF_UTF16_BE, 0};
    वापस {UEF_Unknown, 0};
  हाल 0xFF:
    अगर (  निवेश.आकार_2() >= 4
       && uint8_t(निवेश[1]) == 0xFE
       && निवेश[2] == 0
       && निवेश[3] == 0)
      वापस {UEF_UTF32_LE, 4};

    अगर (निवेश.आकार_2() >= 2 && uint8_t(निवेश[1]) == 0xFE)
      वापस {UEF_UTF16_LE, 2};
    वापस {UEF_Unknown, 0};
  हाल 0xFE:
    अगर (निवेश.आकार_2() >= 2 && uint8_t(निवेश[1]) == 0xFF)
      वापस {UEF_UTF16_BE, 2};
    वापस {UEF_Unknown, 0};
  हाल 0xEF:
    अगर (  निवेश.आकार_2() >= 3
       && uint8_t(निवेश[1]) == 0xBB
       && uint8_t(निवेश[2]) == 0xBF)
      वापस {UEF_UTF8, 3};
    वापस {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  अगर (निवेश.आकार_2() >= 4 && निवेश[1] == 0 && निवेश[2] == 0 && निवेश[3] == 0)
    वापस {UEF_UTF32_LE, 0};

  अगर (निवेश.आकार_2() >= 2 && निवेश[1] == 0)
    वापस {UEF_UTF16_LE, 0};

  वापस {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
व्योम गाँठ::anchor() {}
व्योम Null_गाँठ::anchor() {}
व्योम Scalar_गाँठ::anchor() {}
व्योम Block_Scalar_गाँठ::anchor() {}
व्योम कुंजी_मूल्य_गाँठ::anchor() {}
व्योम Mapping_गाँठ::anchor() {}
व्योम Sequence_गाँठ::anchor() {}
व्योम Alias_गाँठ::anchor() {}

namespace llvm {
namespace yaml {

/// Token - A single YAML token.
काष्ठा Token {
  क्रमागत TokenKind {
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
    TK_कुंजी,
    TK_मूल्य,
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
  std::string मूल्य;

  Token() = शेष;
};

} // end namespace yaml
} // end namespace llvm

using TokenQueueT = BumpPtrList<Token>;

namespace {

/// This struct is used to track simple keys.
///
/// Simple keys are handled by creating an entry in SimpleKeys for each Token
/// which could legally be the start of a simple key. When peekNext is called,
/// if the Token To be returned is referenced by a SimpleKey, we continue
/// tokenizing until that potential simple key has either been found to not be
/// a simple key (we moved on to the next line or went further than 1024 chars).
/// Or when we run into a Value, and then insert a Key token (and possibly
/// others) before the SimpleKey's Tok.
काष्ठा Simple_कुंजी {
  TokenQueueT::iterator Tok;
  अचिन्हित Column = 0;
  अचिन्हित पंक्ति_2 = 0;
  अचिन्हित FlowLevel = 0;
  bool IsRequired = false;

  bool चालक ==(स्थिर Simple_कुंजी &Other) {
    वापस Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
using UTF8Decoded = std::pair<uint32_t, अचिन्हित>;

अटल UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator End = Range.end();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  अगर (Position < End && (*Position & 0x80) == 0) {
    वापस {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  अगर (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    अगर (codepoint >= 0x80)
      वापस {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  अगर (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    अगर (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      वापस {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  अगर (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    अगर (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      वापस {codepoint, 4};
  }
  वापस {0, 0};
}

namespace llvm {
namespace yaml {

/// Scans YAML tokens from a MemoryBuffer.
श्रेणी Scanner {
खुला:
  Scanner(StringRef निवेश, SourceMgr &SM, bool ShowColors = true,
          std::error_code *EC = nullptr);
  Scanner(Memory_बफर_Ref बफर, SourceMgr &SM_, bool ShowColors = true,
          std::error_code *EC = nullptr);

  /// Parse the next token and return it without popping it.
  Token &peek_अग्र();

  /// Parse the next token and pop it from the queue.
  Token get_अग्र();

  व्योम printError(SMLoc Loc, SourceMgr::DiagKind Kind, स्थिर Twine &Message,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.PrintMessage(Loc, Kind, Message, Ranges, /* FixIts= */ {}, ShowColors);
  }

  व्योम setError(स्थिर Twine &Message, StringRef::iterator Position) {
    अगर (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    अगर (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    अगर (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = true;
  }

  /// Returns true if an error occurred while parsing.
  bool failed() {
    वापस Failed;
  }

निजी:
  व्योम init(Memory_बफर_Ref बफर);

  StringRef current_निवेश() {
    वापस StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    वापस ::decodeUTF8(StringRef(Position, End - Position));
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
  व्योम advanceWhile(SkipWhileFunc Func);

  /// Scan ns-uri-char[39]s starting at Cur.
  ///
  /// This updates Cur and Column while scanning.
  व्योम scan_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  bool consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  व्योम skip(uint32_t Distance);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  bool isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  bool isPlainSafeNonBlank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  bool is_पंक्ति_Empty(StringRef पंक्ति_2);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  bool consume_पंक्ति_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  व्योम save_Simple_कुंजी_Candidate( TokenQueueT::iterator Tok
                             , अचिन्हित AtColumn
                             , bool IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  व्योम remove_Stale_Simple_कुंजी_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  व्योम remove_Simple_कुंजी_Candidates_On_Flow_Level(अचिन्हित Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  bool unrollIndent(पूर्णांक ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  bool rollIndent( पूर्णांक ToColumn
                 , Token::TokenKind Kind
                 , TokenQueueT::iterator जोड़ो_Point);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  व्योम skipComment();

  /// Skip whitespace and comments until the start of the next token.
  व्योम scan_To_अग्र_Token();

  /// Must be the first token generated.
  bool scanStreamStart();

  /// Generate tokens needed to close out the stream.
  bool scanStreamEnd();

  /// Scan a %BLAH directive.
  bool scanDirective();

  /// Scan a ... or ---.
  bool scanDocumentIndicator(bool IsStart);

  /// Scan a [ or { and generate the proper flow collection start token.
  bool scanFlowCollectionStart(bool IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  bool scanFlowCollectionEnd(bool IsSequence);

  /// Scan the , that separates entries in a flow collection.
  bool scanFlowEntry();

  /// Scan the - that starts block sequence entries.
  bool scanBlockEntry();

  /// Scan an explicit ? indicating a key.
  bool scan_कुंजी();

  /// Scan an explicit : indicating a value.
  bool scan_मूल्य();

  /// Scan a quoted scalar.
  bool scanFlowScalar(bool IsDoubleQuoted);

  /// Scan an unquoted scalar.
  bool scanPlainScalar();

  /// Scan an Alias or Anchor starting with * or &.
  bool scanAliasOrAnchor(bool IsAlias);

  /// Scan a block scalar starting with | or >.
  bool scanBlockScalar(bool IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  bool scanBlockScalarIndicators(अक्षर &StyleIndicator, अक्षर &ChompingIndicator,
                                 अचिन्हित &IndentIndicator, bool &IsDone);

  /// Scan a style indicator in a block scalar header.
  अक्षर scanBlockStyleIndicator();

  /// Scan a chomping indicator in a block scalar header.
  अक्षर scanBlockChompingIndicator();

  /// Scan an indentation indicator in a block scalar header.
  अचिन्हित scanBlockIndentationIndicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  bool scanBlockScalarHeader(अक्षर &ChompingIndicator, अचिन्हित &IndentIndicator,
                             bool &IsDone);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  bool findBlockScalarIndent(अचिन्हित &BlockIndent, अचिन्हित BlockExitIndent,
                             अचिन्हित &पंक्ति_Breaks, bool &IsDone);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  bool scanBlockScalarIndent(अचिन्हित BlockIndent, अचिन्हित BlockExitIndent,
                             bool &IsDone);

  /// Scan a tag of the form !stuff.
  bool scanTag();

  /// Dispatch to the next scanning function based on \a *Cur.
  bool fetchMoreTokens();

  /// The SourceMgr used for diagnostics and buffer management.
  SourceMgr &SM;

  /// The original input.
  Memory_बफर_Ref निवेश_बफर;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator End;

  /// Current YAML indentation level in spaces.
  पूर्णांक Indent;

  /// Current column number in Unicode code points.
  अचिन्हित Column;

  /// Current line number.
  अचिन्हित पंक्ति_2;

  /// How deep we are in flow style containers. 0 Means at block level.
  अचिन्हित FlowLevel;

  /// Are we at the start of the stream?
  bool IsStartOfStream;

  /// Can the next token be the start of a simple key?
  bool Is_Simple_कुंजी_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  bool Is_Adjacent_मूल्य_Allowed_In_Flow;

  /// True if an error has occurred.
  bool Failed;

  /// Should colors be used when printing out the diagnostic messages?
  bool ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  TokenQueueT TokenQueue;

  /// Indentation levels.
  SmallVector<पूर्णांक, 4> Indents;

  /// Potential simple keys.
  SmallVector<Simple_कुंजी, 4> Simple_कुंजियाँ;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
अटल व्योम encodeUTF8( uint32_t Unicode_Scalar_मूल्य
                      , SmallVectorImpl<अक्षर> &परिणाम) {
  अगर (Unicode_Scalar_मूल्य <= 0x7F) {
    परिणाम.धकेलो_back(Unicode_Scalar_मूल्य & 0x7F);
  } अन्यथा अगर (Unicode_Scalar_मूल्य <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((Unicode_Scalar_मूल्य & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Unicode_Scalar_मूल्य & 0x3F);
    परिणाम.धकेलो_back(FirstByte);
    परिणाम.धकेलो_back(SecondByte);
  } अन्यथा अगर (Unicode_Scalar_मूल्य <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((Unicode_Scalar_मूल्य & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_मूल्य & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Unicode_Scalar_मूल्य & 0x3F);
    परिणाम.धकेलो_back(FirstByte);
    परिणाम.धकेलो_back(SecondByte);
    परिणाम.धकेलो_back(ThirdByte);
  } अन्यथा अगर (Unicode_Scalar_मूल्य <= 0x10FFFF) {
    uint8_t FirstByte = 0xF0 | ((Unicode_Scalar_मूल्य & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_मूल्य & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Unicode_Scalar_मूल्य & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_मूल्य & 0x3F);
    परिणाम.धकेलो_back(FirstByte);
    परिणाम.धकेलो_back(SecondByte);
    परिणाम.धकेलो_back(ThirdByte);
    परिणाम.धकेलो_back(FourthByte);
  }
}

bool yaml::dumpTokens(StringRef निवेश, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(निवेश, SM);
  जबतक (true) {
    Token T = scanner.get_अग्र();
    चयन (T.Kind) {
    हाल Token::TK_StreamStart:
      OS << "Stream-Start: ";
      अवरोध;
    हाल Token::TK_StreamEnd:
      OS << "Stream-End: ";
      अवरोध;
    हाल Token::TK_VersionDirective:
      OS << "Version-Directive: ";
      अवरोध;
    हाल Token::TK_TagDirective:
      OS << "Tag-Directive: ";
      अवरोध;
    हाल Token::TK_DocumentStart:
      OS << "Document-Start: ";
      अवरोध;
    हाल Token::TK_DocumentEnd:
      OS << "Document-End: ";
      अवरोध;
    हाल Token::TK_BlockEntry:
      OS << "Block-Entry: ";
      अवरोध;
    हाल Token::TK_BlockEnd:
      OS << "Block-End: ";
      अवरोध;
    हाल Token::TK_BlockSequenceStart:
      OS << "Block-Sequence-Start: ";
      अवरोध;
    हाल Token::TK_BlockMappingStart:
      OS << "Block-Mapping-Start: ";
      अवरोध;
    हाल Token::TK_FlowEntry:
      OS << "Flow-Entry: ";
      अवरोध;
    हाल Token::TK_FlowSequenceStart:
      OS << "Flow-Sequence-Start: ";
      अवरोध;
    हाल Token::TK_FlowSequenceEnd:
      OS << "Flow-Sequence-End: ";
      अवरोध;
    हाल Token::TK_FlowMappingStart:
      OS << "Flow-Mapping-Start: ";
      अवरोध;
    हाल Token::TK_FlowMappingEnd:
      OS << "Flow-Mapping-End: ";
      अवरोध;
    हाल Token::TK_कुंजी:
      OS << "Key: ";
      अवरोध;
    हाल Token::TK_मूल्य:
      OS << "Value: ";
      अवरोध;
    हाल Token::TK_Scalar:
      OS << "Scalar: ";
      अवरोध;
    हाल Token::TK_BlockScalar:
      OS << "Block Scalar: ";
      अवरोध;
    हाल Token::TK_Alias:
      OS << "Alias: ";
      अवरोध;
    हाल Token::TK_Anchor:
      OS << "Anchor: ";
      अवरोध;
    हाल Token::TK_Tag:
      OS << "Tag: ";
      अवरोध;
    हाल Token::TK_Error:
      अवरोध;
    }
    OS << T.Range << "\n";
    अगर (T.Kind == Token::TK_StreamEnd)
      अवरोध;
    अन्यथा अगर (T.Kind == Token::TK_Error)
      वापस false;
  }
  वापस true;
}

bool yaml::scanTokens(StringRef निवेश) {
  SourceMgr SM;
  Scanner scanner(निवेश, SM);
  जबतक (true) {
    Token T = scanner.get_अग्र();
    अगर (T.Kind == Token::TK_StreamEnd)
      अवरोध;
    अन्यथा अगर (T.Kind == Token::TK_Error)
      वापस false;
  }
  वापस true;
}

std::string yaml::escape(StringRef निवेश, bool EscapePrintable) {
  std::string Escaped_निवेश;
  क्रम (StringRef::iterator i = निवेश.begin(), e = निवेश.end(); i != e; ++i) {
    अगर (*i == '\\')
      Escaped_निवेश += "\\\\";
    अन्यथा अगर (*i == '"')
      Escaped_निवेश += "\\\"";
    अन्यथा अगर (*i == 0)
      Escaped_निवेश += "\\0";
    अन्यथा अगर (*i == 0x07)
      Escaped_निवेश += "\\a";
    अन्यथा अगर (*i == 0x08)
      Escaped_निवेश += "\\b";
    अन्यथा अगर (*i == 0x09)
      Escaped_निवेश += "\\t";
    अन्यथा अगर (*i == 0x0A)
      Escaped_निवेश += "\\n";
    अन्यथा अगर (*i == 0x0B)
      Escaped_निवेश += "\\v";
    अन्यथा अगर (*i == 0x0C)
      Escaped_निवेश += "\\f";
    अन्यथा अगर (*i == 0x0D)
      Escaped_निवेश += "\\r";
    अन्यथा अगर (*i == 0x1B)
      Escaped_निवेश += "\\e";
    अन्यथा अगर ((अचिन्हित अक्षर)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_निवेश += "\\x" + std::string(2 - HexStr.आकार_2(), '0') + HexStr;
    } अन्यथा अगर (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_मूल्य
        = decodeUTF8(StringRef(i, निवेश.end() - i));
      अगर (Unicode_Scalar_मूल्य.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_निवेश, Val);
        // FIXME: Error reporting.
        वापस Escaped_निवेश;
      }
      अगर (Unicode_Scalar_मूल्य.first == 0x85)
        Escaped_निवेश += "\\N";
      अन्यथा अगर (Unicode_Scalar_मूल्य.first == 0xA0)
        Escaped_निवेश += "\\_";
      अन्यथा अगर (Unicode_Scalar_मूल्य.first == 0x2028)
        Escaped_निवेश += "\\L";
      अन्यथा अगर (Unicode_Scalar_मूल्य.first == 0x2029)
        Escaped_निवेश += "\\P";
      अन्यथा अगर (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_मूल्य.first))
        Escaped_निवेश += StringRef(i, Unicode_Scalar_मूल्य.second);
      अन्यथा {
        std::string HexStr = utohexstr(Unicode_Scalar_मूल्य.first);
        अगर (HexStr.आकार_2() <= 2)
          Escaped_निवेश += "\\x" + std::string(2 - HexStr.आकार_2(), '0') + HexStr;
        अन्यथा अगर (HexStr.आकार_2() <= 4)
          Escaped_निवेश += "\\u" + std::string(4 - HexStr.आकार_2(), '0') + HexStr;
        अन्यथा अगर (HexStr.आकार_2() <= 8)
          Escaped_निवेश += "\\U" + std::string(8 - HexStr.आकार_2(), '0') + HexStr;
      }
      i += Unicode_Scalar_मूल्य.second - 1;
    } अन्यथा {
      Escaped_निवेश.धकेलो_back(*i);
    }
  }
  वापस Escaped_निवेश;
}

std::optional<bool> yaml::parseBool(StringRef S) {
  चयन (S.आकार_2()) {
  हाल 1:
    चयन (S.front()) {
    हाल 'y':
    हाल 'Y':
      वापस true;
    हाल 'n':
    हाल 'N':
      वापस false;
    शेष:
      वापस std::nullopt;
    }
  हाल 2:
    चयन (S.front()) {
    हाल 'O':
      अगर (S[1] == 'N') // ON
        वापस true;
      [[fallthrough]];
    हाल 'o':
      अगर (S[1] == 'n') //[Oo]n
        वापस true;
      वापस std::nullopt;
    हाल 'N':
      अगर (S[1] == 'O') // NO
        वापस false;
      [[fallthrough]];
    हाल 'n':
      अगर (S[1] == 'o') //[Nn]o
        वापस false;
      वापस std::nullopt;
    शेष:
      वापस std::nullopt;
    }
  हाल 3:
    चयन (S.front()) {
    हाल 'O':
      अगर (S.drop_front() == "FF") // OFF
        वापस false;
      [[fallthrough]];
    हाल 'o':
      अगर (S.drop_front() == "ff") //[Oo]ff
        वापस false;
      वापस std::nullopt;
    हाल 'Y':
      अगर (S.drop_front() == "ES") // YES
        वापस true;
      [[fallthrough]];
    हाल 'y':
      अगर (S.drop_front() == "es") //[Yy]es
        वापस true;
      वापस std::nullopt;
    शेष:
      वापस std::nullopt;
    }
  हाल 4:
    चयन (S.front()) {
    हाल 'T':
      अगर (S.drop_front() == "RUE") // TRUE
        वापस true;
      [[fallthrough]];
    हाल 't':
      अगर (S.drop_front() == "rue") //[Tt]rue
        वापस true;
      वापस std::nullopt;
    शेष:
      वापस std::nullopt;
    }
  हाल 5:
    चयन (S.front()) {
    हाल 'F':
      अगर (S.drop_front() == "ALSE") // FALSE
        वापस false;
      [[fallthrough]];
    हाल 'f':
      अगर (S.drop_front() == "alse") //[Ff]alse
        वापस false;
      वापस std::nullopt;
    शेष:
      वापस std::nullopt;
    }
  शेष:
    वापस std::nullopt;
  }
}

Scanner::Scanner(StringRef निवेश, SourceMgr &sm, bool ShowColors,
                 std::error_code *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(Memory_बफर_Ref(निवेश, "YAML"));
}

Scanner::Scanner(Memory_बफर_Ref बफर, SourceMgr &SM_, bool ShowColors,
                 std::error_code *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(बफर);
}

व्योम Scanner::init(Memory_बफर_Ref बफर) {
  निवेश_बफर = बफर;
  Current = निवेश_बफर.get_बफर_Start();
  End = निवेश_बफर.get_बफर_End();
  Indent = -1;
  Column = 0;
  पंक्ति_2 = 0;
  FlowLevel = 0;
  IsStartOfStream = true;
  Is_Simple_कुंजी_Allowed = true;
  Is_Adjacent_मूल्य_Allowed_In_Flow = false;
  Failed = false;
  std::unique_ptr<Memory_बफर> निवेश_बफर_Owner =
      Memory_बफर::get_Mem_बफर(बफर, /*RequiresNullTerminator=*/false);
  SM.Add_New_Source_बफर(std::move(निवेश_बफर_Owner), SMLoc());
}

Token &Scanner::peek_अग्र() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  bool NeedMore = false;
  जबतक (true) {
    अगर (TokenQueue.empty() || NeedMore) {
      अगर (!fetchMoreTokens()) {
        TokenQueue.सफा();
        Simple_कुंजियाँ.सफा();
        TokenQueue.धकेलो_back(Token());
        वापस TokenQueue.front();
      }
    }
    निश्चित(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_कुंजी_Candidates();
    Simple_कुंजी SK;
    SK.Tok = TokenQueue.begin();
    अगर (!is_contained(Simple_कुंजियाँ, SK))
      अवरोध;
    अन्यथा
      NeedMore = true;
  }
  वापस TokenQueue.front();
}

Token Scanner::get_अग्र() {
  Token Ret = peek_अग्र();
  // TokenQueue can be empty if there was an error getting the next token.
  अगर (!TokenQueue.empty())
    TokenQueue.निकालो_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  अगर (TokenQueue.empty())
    TokenQueue.resetAlloc();

  वापस Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  अगर (Position == End)
    वापस Position;
  // Check 7 bit c-printable - b-char.
  अगर (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    वापस Position + 1;

  // Check for valid UTF-8.
  अगर (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    अगर (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      वापस Position + u8d.second;
  }
  वापस Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  अगर (Position == End)
    वापस Position;
  अगर (*Position == 0x0D) {
    अगर (Position + 1 != End && *(Position + 1) == 0x0A)
      वापस Position + 2;
    वापस Position + 1;
  }

  अगर (*Position == 0x0A)
    वापस Position + 1;
  वापस Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  अगर (Position == End)
    वापस Position;
  अगर (*Position == ' ')
    वापस Position + 1;
  वापस Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  अगर (Position == End)
    वापस Position;
  अगर (*Position == ' ' || *Position == '\t')
    वापस Position + 1;
  वापस Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  अगर (Position == End)
    वापस Position;
  अगर (*Position == ' ' || *Position == '\t')
    वापस Position;
  वापस skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  जबतक (true) {
    StringRef::iterator i = (यह->*Func)(Position);
    अगर (i == Position)
      अवरोध;
    Position = i;
  }
  वापस Position;
}

व्योम Scanner::advanceWhile(SkipWhileFunc Func) {
  स्वतः Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

अटल bool is_ns_hex_digit(स्थिर अक्षर C) { वापस isAlnum(C); }

अटल bool is_ns_word_char(स्थिर अक्षर C) { वापस C == '-' || isAlpha(C); }

व्योम Scanner::scan_ns_uri_char() {
  जबतक (true) {
    अगर (Current == End)
      अवरोध;
    अगर ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } अन्यथा {
      अवरोध;
    }
  }
}

bool Scanner::consume(uint32_t Expected) {
  अगर (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    वापस false;
  }
  अगर (Current == End)
    वापस false;
  अगर (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    वापस false;
  }
  अगर (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    वापस true;
  }
  वापस false;
}

व्योम Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  निश्चित(Current <= End && "Skipped past the end");
}

bool Scanner::isBlankOrBreak(StringRef::iterator Position) {
  अगर (Position == End)
    वापस false;
  वापस *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

bool Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  अगर (Position == End || isBlankOrBreak(Position))
    वापस false;
  अगर (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    वापस false;
  वापस true;
}

bool Scanner::is_पंक्ति_Empty(StringRef पंक्ति_2) {
  क्रम (स्थिर स्वतः *Position = पंक्ति_2.begin(); Position != पंक्ति_2.end(); ++Position)
    अगर (!isBlankOrBreak(Position))
      वापस false;
  वापस true;
}

bool Scanner::consume_पंक्ति_Break_If_Present() {
  स्वतः अग्र_2 = skip_b_break(Current);
  अगर (अग्र_2 == Current)
    वापस false;
  Column = 0;
  ++पंक्ति_2;
  Current = अग्र_2;
  वापस true;
}

व्योम Scanner::save_Simple_कुंजी_Candidate( TokenQueueT::iterator Tok
                                    , अचिन्हित AtColumn
                                    , bool IsRequired) {
  अगर (Is_Simple_कुंजी_Allowed) {
    Simple_कुंजी SK;
    SK.Tok = Tok;
    SK.पंक्ति_2 = पंक्ति_2;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_कुंजियाँ.धकेलो_back(SK);
  }
}

व्योम Scanner::remove_Stale_Simple_कुंजी_Candidates() {
  क्रम (SmallVectorImpl<Simple_कुंजी>::iterator i = Simple_कुंजियाँ.begin();
                                            i != Simple_कुंजियाँ.end();) {
    अगर (i->पंक्ति_2 != पंक्ति_2 || i->Column + 1024 < Column) {
      अगर (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_कुंजियाँ.erase(i);
    } अन्यथा {
      ++i;
    }
  }
}

व्योम Scanner::remove_Simple_कुंजी_Candidates_On_Flow_Level(अचिन्हित Level) {
  अगर (!Simple_कुंजियाँ.empty() && (Simple_कुंजियाँ.end() - 1)->FlowLevel == Level)
    Simple_कुंजियाँ.निकालो_back();
}

bool Scanner::unrollIndent(पूर्णांक ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  अगर (FlowLevel != 0)
    वापस true;

  जबतक (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.धकेलो_back(T);
    Indent = Indents.निकालो_back_val();
  }

  वापस true;
}

bool Scanner::rollIndent( पूर्णांक ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator जोड़ो_Point) {
  अगर (FlowLevel)
    वापस true;
  अगर (Indent < ToColumn) {
    Indents.धकेलो_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.जोड़ो(जोड़ो_Point, T);
  }
  वापस true;
}

व्योम Scanner::skipComment() {
  अगर (Current == End || *Current != '#')
    वापस;
  जबतक (true) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    अगर (I == Current)
      अवरोध;
    Current = I;
    ++Column;
  }
}

व्योम Scanner::scan_To_अग्र_Token() {
  जबतक (true) {
    जबतक (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    अगर (i == Current)
      अवरोध;
    Current = i;
    ++पंक्ति_2;
    Column = 0;
    // New lines may start a simple key.
    अगर (!FlowLevel)
      Is_Simple_कुंजी_Allowed = true;
  }
}

bool Scanner::scanStreamStart() {
  IsStartOfStream = false;

  EncodingInfo EI = getUnicodeEncoding(current_निवेश());

  Token T;
  T.Kind = Token::TK_StreamStart;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.धकेलो_back(T);
  Current += EI.second;
  वापस true;
}

bool Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  अगर (Column != 0) {
    Column = 0;
    ++पंक्ति_2;
  }

  unrollIndent(-1);
  Simple_कुंजियाँ.सफा();
  Is_Simple_कुंजी_Allowed = false;
  Is_Adjacent_मूल्य_Allowed_In_Flow = false;

  Token T;
  T.Kind = Token::TK_StreamEnd;
  T.Range = StringRef(Current, 0);
  TokenQueue.धकेलो_back(T);
  वापस true;
}

bool Scanner::scanDirective() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_कुंजियाँ.सफा();
  Is_Simple_कुंजी_Allowed = false;
  Is_Adjacent_मूल्य_Allowed_In_Flow = false;

  StringRef::iterator Start = Current;
  consume('%');
  StringRef::iterator NameStart = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(NameStart, Current - NameStart);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  अगर (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.धकेलो_back(T);
    वापस true;
  } अन्यथा अगर(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.धकेलो_back(T);
    वापस true;
  }
  वापस false;
}

bool Scanner::scanDocumentIndicator(bool IsStart) {
  unrollIndent(-1);
  Simple_कुंजियाँ.सफा();
  Is_Simple_कुंजी_Allowed = false;
  Is_Adjacent_मूल्य_Allowed_In_Flow = false;

  Token T;
  T.Kind = IsStart ? Token::TK_DocumentStart : Token::TK_DocumentEnd;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.धकेलो_back(T);
  वापस true;
}

bool Scanner::scanFlowCollectionStart(bool IsSequence) {
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.धकेलो_back(T);

  // [ and { may begin a simple key.
  save_Simple_कुंजी_Candidate(--TokenQueue.end(), Column - 1, false);

  // And may also be followed by a simple key.
  Is_Simple_कुंजी_Allowed = true;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_मूल्य_Allowed_In_Flow = false;
  ++FlowLevel;
  वापस true;
}

bool Scanner::scanFlowCollectionEnd(bool IsSequence) {
  remove_Simple_कुंजी_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_कुंजी_Allowed = false;
  Is_Adjacent_मूल्य_Allowed_In_Flow = true;
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceEnd
                      : Token::TK_FlowMappingEnd;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.धकेलो_back(T);
  अगर (FlowLevel)
    --FlowLevel;
  वापस true;
}

bool Scanner::scanFlowEntry() {
  remove_Simple_कुंजी_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_कुंजी_Allowed = true;
  Is_Adjacent_मूल्य_Allowed_In_Flow = false;
  Token T;
  T.Kind = Token::TK_FlowEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.धकेलो_back(T);
  वापस true;
}

bool Scanner::scanBlockEntry() {
  rollIndent(Column, Token::TK_BlockSequenceStart, TokenQueue.end());
  remove_Simple_कुंजी_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_कुंजी_Allowed = true;
  Is_Adjacent_मूल्य_Allowed_In_Flow = false;
  Token T;
  T.Kind = Token::TK_BlockEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.धकेलो_back(T);
  वापस true;
}

bool Scanner::scan_कुंजी() {
  अगर (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  remove_Simple_कुंजी_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_कुंजी_Allowed = !FlowLevel;
  Is_Adjacent_मूल्य_Allowed_In_Flow = false;

  Token T;
  T.Kind = Token::TK_कुंजी;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.धकेलो_back(T);
  वापस true;
}

bool Scanner::scan_मूल्य() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  अगर (!Simple_कुंजियाँ.empty()) {
    Simple_कुंजी SK = Simple_कुंजियाँ.निकालो_back_val();
    Token T;
    T.Kind = Token::TK_कुंजी;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    क्रम (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      अगर (i == SK.Tok)
        अवरोध;
    }
    अगर (i == e) {
      Failed = true;
      वापस false;
    }
    i = TokenQueue.जोड़ो(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    Is_Simple_कुंजी_Allowed = false;
  } अन्यथा {
    अगर (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    Is_Simple_कुंजी_Allowed = !FlowLevel;
  }
  Is_Adjacent_मूल्य_Allowed_In_Flow = false;

  Token T;
  T.Kind = Token::TK_मूल्य;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.धकेलो_back(T);
  वापस true;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE अटल bool
wasEscaped(StringRef::iterator First, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
अटल bool wasEscaped(StringRef::iterator First,
                       StringRef::iterator Position) {
  निश्चित(Position - 1 >= First);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  जबतक (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  वापस (Position - 1 - I) % 2 == 1;
}

bool Scanner::scanFlowScalar(bool IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  अचिन्हित ColStart = Column;
  अगर (IsDoubleQuoted) {
    करो {
      ++Current;
      जबतक (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } जबतक (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } अन्यथा {
    skip(1);
    जबतक (Current != End) {
      // Skip a ' followed by another '.
      अगर (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        जारी;
      } अन्यथा अगर (*Current == '\'')
        अवरोध;
      StringRef::iterator i = skip_nb_char(Current);
      अगर (i == Current) {
        i = skip_b_break(Current);
        अगर (i == Current)
          अवरोध;
        Current = i;
        Column = 0;
        ++पंक्ति_2;
      } अन्यथा {
        अगर (i == End)
          अवरोध;
        Current = i;
        ++Column;
      }
    }
  }

  अगर (Current == End) {
    setError("Expected quote at end of scalar", Current);
    वापस false;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.धकेलो_back(T);

  save_Simple_कुंजी_Candidate(--TokenQueue.end(), ColStart, false);

  Is_Simple_कुंजी_Allowed = false;
  Is_Adjacent_मूल्य_Allowed_In_Flow = true;

  वापस true;
}

bool Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  अचिन्हित ColStart = Column;
  अचिन्हित LeadingBlanks = 0;
  निश्चित(Indent >= -1 && "Indent must be >= -1 !");
  अचिन्हित indent = static_cast<अचिन्हित>(Indent + 1);
  जबतक (Current != End) {
    अगर (*Current == '#')
      अवरोध;

    जबतक (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      अगर (i == Current)
        अवरोध;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    अगर (!isBlankOrBreak(Current))
      अवरोध;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    जबतक (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      अगर (i != Tmp) {
        अगर (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          वापस false;
        }
        Tmp = i;
        ++Column;
      } अन्यथा {
        i = skip_b_break(Tmp);
        अगर (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++पंक्ति_2;
      }
    }

    अगर (!FlowLevel && Column < indent)
      अवरोध;

    Current = Tmp;
  }
  अगर (Start == Current) {
    setError("Got empty plain scalar", Start);
    वापस false;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.धकेलो_back(T);

  // Plain scalars can be simple keys.
  save_Simple_कुंजी_Candidate(--TokenQueue.end(), ColStart, false);

  Is_Simple_कुंजी_Allowed = false;
  Is_Adjacent_मूल्य_Allowed_In_Flow = false;

  वापस true;
}

bool Scanner::scanAliasOrAnchor(bool IsAlias) {
  StringRef::iterator Start = Current;
  अचिन्हित ColStart = Column;
  skip(1);
  जबतक (Current != End) {
    अगर (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      अवरोध;
    StringRef::iterator i = skip_ns_char(Current);
    अगर (i == Current)
      अवरोध;
    Current = i;
    ++Column;
  }

  अगर (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    वापस false;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.धकेलो_back(T);

  // Alias and anchors can be simple keys.
  save_Simple_कुंजी_Candidate(--TokenQueue.end(), ColStart, false);

  Is_Simple_कुंजी_Allowed = false;
  Is_Adjacent_मूल्य_Allowed_In_Flow = false;

  वापस true;
}

bool Scanner::scanBlockScalarIndicators(अक्षर &StyleIndicator,
                                        अक्षर &ChompingIndicator,
                                        अचिन्हित &IndentIndicator,
                                        bool &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  अगर (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    वापस false;
  वापस true;
}

अक्षर Scanner::scanBlockStyleIndicator() {
  अक्षर Indicator = ' ';
  अगर (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  वापस Indicator;
}

अक्षर Scanner::scanBlockChompingIndicator() {
  अक्षर Indicator = ' ';
  अगर (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  वापस Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
अटल अचिन्हित get_Chomped_पंक्ति_Breaks(अक्षर ChompingIndicator,
                                     अचिन्हित पंक्ति_Breaks, StringRef Str) {
  अगर (ChompingIndicator == '-') // Strip all line breaks.
    वापस 0;
  अगर (ChompingIndicator == '+') // Keep all line breaks.
    वापस पंक्ति_Breaks;
  // Clip trailing lines.
  वापस Str.empty() ? 0 : 1;
}

अचिन्हित Scanner::scanBlockIndentationIndicator() {
  अचिन्हित Indent = 0;
  अगर (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = अचिन्हित(*Current - '0');
    skip(1);
  }
  वापस Indent;
}

bool Scanner::scanBlockScalarHeader(अक्षर &ChompingIndicator,
                                    अचिन्हित &IndentIndicator, bool &IsDone) {
  स्वतः Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  अगर (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  अगर (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.धकेलो_back(T);
    IsDone = true;
    वापस true;
  }

  अगर (!consume_पंक्ति_Break_If_Present()) {
    setError("Expected a line break after block scalar header", Current);
    वापस false;
  }
  वापस true;
}

bool Scanner::findBlockScalarIndent(अचिन्हित &BlockIndent,
                                    अचिन्हित BlockExitIndent,
                                    अचिन्हित &पंक्ति_Breaks, bool &IsDone) {
  अचिन्हित Max_All_Space_पंक्ति_Characters = 0;
  StringRef::iterator Longest_All_Space_पंक्ति;

  जबतक (true) {
    advanceWhile(&Scanner::skip_s_space);
    अगर (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      अगर (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = true;
        वापस true;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      अगर (Max_All_Space_पंक्ति_Characters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_पंक्ति);
        वापस false;
      }
      वापस true;
    }
    अगर (skip_b_break(Current) != Current &&
        Column > Max_All_Space_पंक्ति_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_पंक्ति_Characters = Column;
      Longest_All_Space_पंक्ति = Current;
    }

    // Check for EOF.
    अगर (Current == End) {
      IsDone = true;
      वापस true;
    }

    अगर (!consume_पंक्ति_Break_If_Present()) {
      IsDone = true;
      वापस true;
    }
    ++पंक्ति_Breaks;
  }
  वापस true;
}

bool Scanner::scanBlockScalarIndent(अचिन्हित BlockIndent,
                                    अचिन्हित BlockExitIndent, bool &IsDone) {
  // Skip the indentation.
  जबतक (Column < BlockIndent) {
    स्वतः I = skip_s_space(Current);
    अगर (I == Current)
      अवरोध;
    Current = I;
    ++Column;
  }

  अगर (skip_nb_char(Current) == Current)
    वापस true;

  अगर (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = true;
    वापस true;
  }

  अगर (Column < BlockIndent) {
    अगर (Current != End && *Current == '#') { // Trailing comment.
      IsDone = true;
      वापस true;
    }
    setError("A text line is less indented than the block scalar", Current);
    वापस false;
  }
  वापस true; // A normal text line.
}

bool Scanner::scanBlockScalar(bool IsLiteral) {
  निश्चित(*Current == '|' || *Current == '>');
  अक्षर StyleIndicator;
  अक्षर ChompingIndicator;
  अचिन्हित BlockIndent;
  bool IsDone = false;
  अगर (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    वापस false;
  अगर (IsDone)
    वापस true;
  bool IsFolded = StyleIndicator == '>';

  स्थिर स्वतः *Start = Current;
  अचिन्हित BlockExitIndent = Indent < 0 ? 0 : (अचिन्हित)Indent;
  अचिन्हित पंक्ति_Breaks = 0;
  अगर (BlockIndent == 0) {
    अगर (!findBlockScalarIndent(BlockIndent, BlockExitIndent, पंक्ति_Breaks,
                               IsDone))
      वापस false;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  जबतक (!IsDone) {
    अगर (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      वापस false;
    अगर (IsDone)
      अवरोध;

    // Parse the current line.
    स्वतः पंक्ति_Start = Current;
    advanceWhile(&Scanner::skip_nb_char);
    अगर (पंक्ति_Start != Current) {
      अगर (पंक्ति_Breaks && IsFolded && !Scanner::is_पंक्ति_Empty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        अगर (पंक्ति_Breaks == 1) {
          Str.append(पंक्ति_Breaks,
                     is_पंक्ति_Empty(StringRef(पंक्ति_Start, Current - पंक्ति_Start))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        पंक्ति_Breaks--;
      }
      Str.append(पंक्ति_Breaks, '\n');
      Str.append(StringRef(पंक्ति_Start, Current - पंक्ति_Start));
      पंक्ति_Breaks = 0;
    }

    // Check for EOF.
    अगर (Current == End)
      अवरोध;

    अगर (!consume_पंक्ति_Break_If_Present())
      अवरोध;
    ++पंक्ति_Breaks;
  }

  अगर (Current == End && !पंक्ति_Breaks)
    // Ensure that there is at least one line break before the end of file.
    पंक्ति_Breaks = 1;
  Str.append(get_Chomped_पंक्ति_Breaks(ChompingIndicator, पंक्ति_Breaks, Str), '\n');

  // New lines may start a simple key.
  अगर (!FlowLevel)
    Is_Simple_कुंजी_Allowed = true;
  Is_Adjacent_मूल्य_Allowed_In_Flow = false;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.मूल्य = std::string(Str);
  TokenQueue.धकेलो_back(T);
  वापस true;
}

bool Scanner::scanTag() {
  StringRef::iterator Start = Current;
  अचिन्हित ColStart = Column;
  skip(1); // Eat !.
  अगर (Current == End || isBlankOrBreak(Current)); // An empty tag.
  अन्यथा अगर (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    अगर (!consume('>'))
      वापस false;
  } अन्यथा {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.Kind = Token::TK_Tag;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.धकेलो_back(T);

  // Tags can be simple keys.
  save_Simple_कुंजी_Candidate(--TokenQueue.end(), ColStart, false);

  Is_Simple_कुंजी_Allowed = false;
  Is_Adjacent_मूल्य_Allowed_In_Flow = false;

  वापस true;
}

bool Scanner::fetchMoreTokens() {
  अगर (IsStartOfStream)
    वापस scanStreamStart();

  scan_To_अग्र_Token();

  अगर (Current == End)
    वापस scanStreamEnd();

  remove_Stale_Simple_कुंजी_Candidates();

  unrollIndent(Column);

  अगर (Column == 0 && *Current == '%')
    वापस scanDirective();

  अगर (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    वापस scanDocumentIndicator(true);

  अगर (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    वापस scanDocumentIndicator(false);

  अगर (*Current == '[')
    वापस scanFlowCollectionStart(true);

  अगर (*Current == '{')
    वापस scanFlowCollectionStart(false);

  अगर (*Current == ']')
    वापस scanFlowCollectionEnd(true);

  अगर (*Current == '}')
    वापस scanFlowCollectionEnd(false);

  अगर (*Current == ',')
    वापस scanFlowEntry();

  अगर (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    वापस scanBlockEntry();

  अगर (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    वापस scan_कुंजी();

  अगर (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || Is_Adjacent_मूल्य_Allowed_In_Flow))
    वापस scan_मूल्य();

  अगर (*Current == '*')
    वापस scanAliasOrAnchor(true);

  अगर (*Current == '&')
    वापस scanAliasOrAnchor(false);

  अगर (*Current == '!')
    वापस scanTag();

  अगर (*Current == '|' && !FlowLevel)
    वापस scanBlockScalar(true);

  अगर (*Current == '>' && !FlowLevel)
    वापस scanBlockScalar(false);

  अगर (*Current == '\'')
    वापस scanFlowScalar(false);

  अगर (*Current == '"')
    वापस scanFlowScalar(true);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  अगर ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    वापस scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  वापस false;
}

Stream::Stream(StringRef निवेश, SourceMgr &SM, bool ShowColors,
               std::error_code *EC)
    : scanner(नया Scanner(निवेश, SM, ShowColors, EC)) {}

Stream::Stream(Memory_बफर_Ref निवेश_बफर, SourceMgr &SM, bool ShowColors,
               std::error_code *EC)
    : scanner(नया Scanner(निवेश_बफर, SM, ShowColors, EC)) {}

Stream::~Stream() = शेष;

bool Stream::failed() { वापस scanner->failed(); }

व्योम Stream::printError(गाँठ *N, स्थिर Twine &संदेश_2, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), संदेश_2, Kind);
}

व्योम Stream::printError(स्थिर SMRange &Range, स्थिर Twine &संदेश_2,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, संदेश_2, Range);
}

document_iterator Stream::begin() {
  अगर (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->get_अग्र();

  CurrentDoc.reset(नया Document(*यह));
  वापस document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  वापस document_iterator();
}

व्योम Stream::skip() {
  क्रम (Document &Doc : *यह)
    Doc.skip();
}

गाँठ::गाँठ(अचिन्हित पूर्णांक Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peek_अग्र().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string गाँठ::getVerbatimTag() स्थिर {
  StringRef Raw = getRawTag();
  अगर (!Raw.empty() && Raw != "!") {
    std::string Ret;
    अगर (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      वापस Ret;
    } अन्यथा अगर (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      वापस Ret;
    } अन्यथा {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      अगर (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      अन्यथा {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      वापस Ret;
    }
  }

  चयन (getType()) {
  हाल NK_Null:
    वापस "tag:yaml.org,2002:null";
  हाल NK_Scalar:
  हाल NK_BlockScalar:
    // TODO: Tag resolution.
    वापस "tag:yaml.org,2002:str";
  हाल NK_Mapping:
    वापस "tag:yaml.org,2002:map";
  हाल NK_Sequence:
    वापस "tag:yaml.org,2002:seq";
  }

  वापस "";
}

Token &गाँठ::peek_अग्र() {
  वापस Doc->peek_अग्र();
}

Token गाँठ::get_अग्र() {
  वापस Doc->get_अग्र();
}

गाँठ *गाँठ::parse_Block_गाँठ() {
  वापस Doc->parse_Block_गाँठ();
}

BumpPtrAllocator &गाँठ::getAllocator() {
  वापस Doc->गाँठ_Allocator;
}

व्योम गाँठ::setError(स्थिर Twine &संदेश_2, Token &Tok) स्थिर {
  Doc->setError(संदेश_2, Tok);
}

bool गाँठ::failed() स्थिर {
  वापस Doc->failed();
}

StringRef Scalar_गाँठ::get_मूल्य(SmallVectorImpl<अक्षर> &Storage) स्थिर {
  अगर (मूल्य[0] == '"')
    वापस get_Double_Quoted_मूल्य(मूल्य, Storage);
  अगर (मूल्य[0] == '\'')
    वापस get_Single_Quoted_मूल्य(मूल्य, Storage);
  वापस get_Plain_मूल्य(मूल्य, Storage);
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
अटल StringRef
parse_Scalar_मूल्य(StringRef Unquoted_मूल्य, SmallVectorImpl<अक्षर> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<अक्षर> &)>
                     UnescapeCallback) {
  माप_प्रकार I = Unquoted_मूल्य.find_first_of(LookupChars);
  अगर (I == StringRef::npos)
    वापस Unquoted_मूल्य;

  Storage.सफा();
  Storage.reserve(Unquoted_मूल्य.आकार_2());
  अक्षर Last_New_पंक्ति_Added_As = '\0';
  क्रम (; I != StringRef::npos; I = Unquoted_मूल्य.find_first_of(LookupChars)) {
    अगर (Unquoted_मूल्य[I] != '\r' && Unquoted_मूल्य[I] != '\n') {
      llvm::append_range(Storage, Unquoted_मूल्य.take_front(I));
      Unquoted_मूल्य = UnescapeCallback(Unquoted_मूल्य.drop_front(I), Storage);
      Last_New_पंक्ति_Added_As = '\0';
      जारी;
    }
    अगर (माप_प्रकार LastNonSWhite = Unquoted_मूल्य.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_मूल्य.take_front(LastNonSWhite + 1));
      Storage.धकेलो_back(' ');
      Last_New_पंक्ति_Added_As = ' ';
    } अन्यथा {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      चयन (Last_New_पंक्ति_Added_As) {
      हाल ' ':
        निश्चित(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        Last_New_पंक्ति_Added_As = '\n';
        अवरोध;
      हाल '\n':
        निश्चित(!Storage.empty() && Storage.back() == '\n');
        Storage.धकेलो_back('\n');
        अवरोध;
      शेष:
        Storage.धकेलो_back(' ');
        Last_New_पंक्ति_Added_As = ' ';
        अवरोध;
      }
    }
    // Handle Windows-style EOL
    अगर (Unquoted_मूल्य.substr(I, 2) == "\r\n")
      I++;
    Unquoted_मूल्य = Unquoted_मूल्य.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_मूल्य);
  वापस StringRef(Storage.begin(), Storage.आकार_2());
}

StringRef
Scalar_गाँठ::get_Double_Quoted_मूल्य(StringRef Raw_मूल्य,
                                 SmallVectorImpl<अक्षर> &Storage) स्थिर {
  निश्चित(Raw_मूल्य.आकार_2() >= 2 && Raw_मूल्य.front() == '"' &&
         Raw_मूल्य.back() == '"');
  StringRef Unquoted_मूल्य = Raw_मूल्य.substr(1, Raw_मूल्य.आकार_2() - 2);

  स्वतः UnescapeFunc = [यह](StringRef Unquoted_मूल्य,
                             SmallVectorImpl<अक्षर> &Storage) {
    निश्चित(Unquoted_मूल्य.take_front(1) == "\\");
    अगर (Unquoted_मूल्य.आकार_2() == 1) {
      Token T;
      T.Range = Unquoted_मूल्य;
      setError("Unrecognized escape code", T);
      Storage.सफा();
      वापस StringRef();
    }
    Unquoted_मूल्य = Unquoted_मूल्य.drop_front(1);
    चयन (Unquoted_मूल्य[0]) {
    शेष: {
      Token T;
      T.Range = Unquoted_मूल्य.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.सफा();
      वापस StringRef();
    }
    हाल '\r':
      // Shrink the Windows-style EOL.
      अगर (Unquoted_मूल्य.आकार_2() >= 2 && Unquoted_मूल्य[1] == '\n')
        Unquoted_मूल्य = Unquoted_मूल्य.drop_front(1);
      [[fallthrough]];
    हाल '\n':
      वापस Unquoted_मूल्य.drop_front(1).ltrim(" \t");
    हाल '0':
      Storage.धकेलो_back(0x00);
      अवरोध;
    हाल 'a':
      Storage.धकेलो_back(0x07);
      अवरोध;
    हाल 'b':
      Storage.धकेलो_back(0x08);
      अवरोध;
    हाल 't':
    हाल 0x09:
      Storage.धकेलो_back(0x09);
      अवरोध;
    हाल 'n':
      Storage.धकेलो_back(0x0A);
      अवरोध;
    हाल 'v':
      Storage.धकेलो_back(0x0B);
      अवरोध;
    हाल 'f':
      Storage.धकेलो_back(0x0C);
      अवरोध;
    हाल 'r':
      Storage.धकेलो_back(0x0D);
      अवरोध;
    हाल 'e':
      Storage.धकेलो_back(0x1B);
      अवरोध;
    हाल ' ':
      Storage.धकेलो_back(0x20);
      अवरोध;
    हाल '"':
      Storage.धकेलो_back(0x22);
      अवरोध;
    हाल '/':
      Storage.धकेलो_back(0x2F);
      अवरोध;
    हाल '\\':
      Storage.धकेलो_back(0x5C);
      अवरोध;
    हाल 'N':
      encodeUTF8(0x85, Storage);
      अवरोध;
    हाल '_':
      encodeUTF8(0xA0, Storage);
      अवरोध;
    हाल 'L':
      encodeUTF8(0x2028, Storage);
      अवरोध;
    हाल 'P':
      encodeUTF8(0x2029, Storage);
      अवरोध;
    हाल 'x': {
      अगर (Unquoted_मूल्य.आकार_2() < 3)
        // TODO: Report error.
        अवरोध;
      अचिन्हित पूर्णांक Unicode_Scalar_मूल्य;
      अगर (Unquoted_मूल्य.substr(1, 2).getAsInteger(16, Unicode_Scalar_मूल्य))
        // TODO: Report error.
        Unicode_Scalar_मूल्य = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मूल्य, Storage);
      वापस Unquoted_मूल्य.drop_front(3);
    }
    हाल 'u': {
      अगर (Unquoted_मूल्य.आकार_2() < 5)
        // TODO: Report error.
        अवरोध;
      अचिन्हित पूर्णांक Unicode_Scalar_मूल्य;
      अगर (Unquoted_मूल्य.substr(1, 4).getAsInteger(16, Unicode_Scalar_मूल्य))
        // TODO: Report error.
        Unicode_Scalar_मूल्य = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मूल्य, Storage);
      वापस Unquoted_मूल्य.drop_front(5);
    }
    हाल 'U': {
      अगर (Unquoted_मूल्य.आकार_2() < 9)
        // TODO: Report error.
        अवरोध;
      अचिन्हित पूर्णांक Unicode_Scalar_मूल्य;
      अगर (Unquoted_मूल्य.substr(1, 8).getAsInteger(16, Unicode_Scalar_मूल्य))
        // TODO: Report error.
        Unicode_Scalar_मूल्य = 0xFFFD;
      encodeUTF8(Unicode_Scalar_मूल्य, Storage);
      वापस Unquoted_मूल्य.drop_front(9);
    }
    }
    वापस Unquoted_मूल्य.drop_front(1);
  };

  वापस parse_Scalar_मूल्य(Unquoted_मूल्य, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_गाँठ::get_Single_Quoted_मूल्य(StringRef Raw_मूल्य,
                                           SmallVectorImpl<अक्षर> &Storage) {
  निश्चित(Raw_मूल्य.आकार_2() >= 2 && Raw_मूल्य.front() == '\'' &&
         Raw_मूल्य.back() == '\'');
  StringRef Unquoted_मूल्य = Raw_मूल्य.substr(1, Raw_मूल्य.आकार_2() - 2);

  स्वतः UnescapeFunc = [](StringRef Unquoted_मूल्य,
                         SmallVectorImpl<अक्षर> &Storage) {
    निश्चित(Unquoted_मूल्य.take_front(2) == "''");
    Storage.धकेलो_back('\'');
    वापस Unquoted_मूल्य.drop_front(2);
  };

  वापस parse_Scalar_मूल्य(Unquoted_मूल्य, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_गाँठ::get_Plain_मूल्य(StringRef Raw_मूल्य,
                                    SmallVectorImpl<अक्षर> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_मूल्य = Raw_मूल्य.rtrim("\r\n \t");
  वापस parse_Scalar_मूल्य(Raw_मूल्य, Storage, "\r\n", nullptr);
}

गाँठ *कुंजी_मूल्य_गाँठ::get_कुंजी() {
  अगर (कुंजी)
    वापस कुंजी;
  // Handle implicit null keys.
  {
    Token &t = peek_अग्र();
    अगर (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_मूल्य
        || t.Kind == Token::TK_Error) {
      वापस कुंजी = नया (getAllocator()) Null_गाँठ(Doc);
    }
    अगर (t.Kind == Token::TK_कुंजी)
      get_अग्र(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peek_अग्र();
  अगर (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_मूल्य) {
    वापस कुंजी = नया (getAllocator()) Null_गाँठ(Doc);
  }

  // We've got a normal key.
  वापस कुंजी = parse_Block_गाँठ();
}

गाँठ *कुंजी_मूल्य_गाँठ::get_मूल्य() {
  अगर (मूल्य)
    वापस मूल्य;

  अगर (गाँठ* कुंजी = get_कुंजी())
    कुंजी->skip();
  अन्यथा {
    setError("Null key in Key Value.", peek_अग्र());
    वापस मूल्य = नया (getAllocator()) Null_गाँठ(Doc);
  }

  अगर (failed())
    वापस मूल्य = नया (getAllocator()) Null_गाँठ(Doc);

  // Handle implicit null values.
  {
    Token &t = peek_अग्र();
    अगर (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_कुंजी
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      वापस मूल्य = नया (getAllocator()) Null_गाँठ(Doc);
    }

    अगर (t.Kind != Token::TK_मूल्य) {
      setError("Unexpected token in Key Value.", t);
      वापस मूल्य = नया (getAllocator()) Null_गाँठ(Doc);
    }
    get_अग्र(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peek_अग्र();
  अगर (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_कुंजी) {
    वापस मूल्य = नया (getAllocator()) Null_गाँठ(Doc);
  }

  // We got a normal value.
  वापस मूल्य = parse_Block_गाँठ();
}

व्योम Mapping_गाँठ::increment() {
  अगर (failed()) {
    IsAtEnd = true;
    CurrentEntry = nullptr;
    वापस;
  }
  अगर (CurrentEntry) {
    CurrentEntry->skip();
    अगर (Type == MT_Inline) {
      IsAtEnd = true;
      CurrentEntry = nullptr;
      वापस;
    }
  }
  Token T = peek_अग्र();
  अगर (T.Kind == Token::TK_कुंजी || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = नया (getAllocator()) कुंजी_मूल्य_गाँठ(Doc);
  } अन्यथा अगर (Type == MT_Block) {
    चयन (T.Kind) {
    हाल Token::TK_BlockEnd:
      get_अग्र();
      IsAtEnd = true;
      CurrentEntry = nullptr;
      अवरोध;
    शेष:
      setError("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    हाल Token::TK_Error:
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  } अन्यथा {
    चयन (T.Kind) {
    हाल Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      get_अग्र();
      वापस increment();
    हाल Token::TK_FlowMappingEnd:
      get_अग्र();
      [[fallthrough]];
    हाल Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = true;
      CurrentEntry = nullptr;
      अवरोध;
    शेष:
      setError( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  }
}

व्योम Sequence_गाँठ::increment() {
  अगर (failed()) {
    IsAtEnd = true;
    CurrentEntry = nullptr;
    वापस;
  }
  अगर (CurrentEntry)
    CurrentEntry->skip();
  Token T = peek_अग्र();
  अगर (SeqType == ST_Block) {
    चयन (T.Kind) {
    हाल Token::TK_BlockEntry:
      get_अग्र();
      CurrentEntry = parse_Block_गाँठ();
      अगर (!CurrentEntry) { // An error occurred.
        IsAtEnd = true;
        CurrentEntry = nullptr;
      }
      अवरोध;
    हाल Token::TK_BlockEnd:
      get_अग्र();
      IsAtEnd = true;
      CurrentEntry = nullptr;
      अवरोध;
    शेष:
      setError( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    हाल Token::TK_Error:
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  } अन्यथा अगर (SeqType == ST_Indentless) {
    चयन (T.Kind) {
    हाल Token::TK_BlockEntry:
      get_अग्र();
      CurrentEntry = parse_Block_गाँठ();
      अगर (!CurrentEntry) { // An error occurred.
        IsAtEnd = true;
        CurrentEntry = nullptr;
      }
      अवरोध;
    शेष:
    हाल Token::TK_Error:
      IsAtEnd = true;
      CurrentEntry = nullptr;
    }
  } अन्यथा अगर (SeqType == ST_Flow) {
    चयन (T.Kind) {
    हाल Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      get_अग्र();
      WasPreviousTokenFlowEntry = true;
      वापस increment();
    हाल Token::TK_FlowSequenceEnd:
      get_अग्र();
      [[fallthrough]];
    हाल Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = true;
      CurrentEntry = nullptr;
      अवरोध;
    हाल Token::TK_StreamEnd:
    हाल Token::TK_DocumentEnd:
    हाल Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = true;
      CurrentEntry = nullptr;
      अवरोध;
    शेष:
      अगर (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = true;
        CurrentEntry = nullptr;
        अवरोध;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parse_Block_गाँठ();
      अगर (!CurrentEntry) {
        IsAtEnd = true;
      }
      WasPreviousTokenFlowEntry = false;
      अवरोध;
    }
  }
}

Document::Document(Stream &S) : stream(S), मूल_2(nullptr) {
  // Tag maps starts with two default mappings.
  TagMap["!"] = "!";
  TagMap["!!"] = "tag:yaml.org,2002:";

  अगर (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peek_अग्र();
  अगर (T.Kind == Token::TK_DocumentStart)
    get_अग्र();
}

bool Document::skip()  {
  अगर (stream.scanner->failed())
    वापस false;
  अगर (!मूल_2 && !get_मूल())
    वापस false;
  मूल_2->skip();
  Token &T = peek_अग्र();
  अगर (T.Kind == Token::TK_StreamEnd)
    वापस false;
  अगर (T.Kind == Token::TK_DocumentEnd) {
    get_अग्र();
    वापस skip();
  }
  वापस true;
}

Token &Document::peek_अग्र() {
  वापस stream.scanner->peek_अग्र();
}

Token Document::get_अग्र() {
  वापस stream.scanner->get_अग्र();
}

व्योम Document::setError(स्थिर Twine &Message, Token &Location) स्थिर {
  stream.scanner->setError(Message, Location.Range.begin());
}

bool Document::failed() स्थिर {
  वापस stream.scanner->failed();
}

गाँठ *Document::parse_Block_गाँठ() {
  Token T = peek_अग्र();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  चयन (T.Kind) {
  हाल Token::TK_Alias:
    get_अग्र();
    वापस नया (गाँठ_Allocator) Alias_गाँठ(stream.CurrentDoc, T.Range.substr(1));
  हाल Token::TK_Anchor:
    अगर (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      वापस nullptr;
    }
    AnchorInfo = get_अग्र(); // Consume TK_Anchor.
    T = peek_अग्र();
    जाओ parse_property;
  हाल Token::TK_Tag:
    अगर (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      वापस nullptr;
    }
    TagInfo = get_अग्र(); // Consume TK_Tag.
    T = peek_अग्र();
    जाओ parse_property;
  शेष:
    अवरोध;
  }

  चयन (T.Kind) {
  हाल Token::TK_BlockEntry:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    वापस नया (गाँठ_Allocator) Sequence_गाँठ( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , Sequence_गाँठ::ST_Indentless);
  हाल Token::TK_BlockSequenceStart:
    get_अग्र();
    वापस नया (गाँठ_Allocator)
      Sequence_गाँठ( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_गाँठ::ST_Block);
  हाल Token::TK_BlockMappingStart:
    get_अग्र();
    वापस नया (गाँठ_Allocator)
      Mapping_गाँठ( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_गाँठ::MT_Block);
  हाल Token::TK_FlowSequenceStart:
    get_अग्र();
    वापस नया (गाँठ_Allocator)
      Sequence_गाँठ( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_गाँठ::ST_Flow);
  हाल Token::TK_FlowMappingStart:
    get_अग्र();
    वापस नया (गाँठ_Allocator)
      Mapping_गाँठ( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_गाँठ::MT_Flow);
  हाल Token::TK_Scalar:
    get_अग्र();
    वापस नया (गाँठ_Allocator)
      Scalar_गाँठ( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  हाल Token::TK_BlockScalar: {
    get_अग्र();
    StringRef NullTerminatedStr(T.मूल्य.c_str(), T.मूल्य.लंबाई_2() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(गाँठ_Allocator).drop_back();
    वापस नया (गाँठ_Allocator)
        Block_Scalar_गाँठ(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  हाल Token::TK_कुंजी:
    // Don't eat the TK_Key, KeyValueNode expects it.
    वापस नया (गाँठ_Allocator)
      Mapping_गाँठ( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_गाँठ::MT_Inline);
  हाल Token::TK_DocumentStart:
  हाल Token::TK_DocumentEnd:
  हाल Token::TK_StreamEnd:
  शेष:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    वापस नया (गाँठ_Allocator) Null_गाँठ(stream.CurrentDoc);
  हाल Token::TK_FlowMappingEnd:
  हाल Token::TK_FlowSequenceEnd:
  हाल Token::TK_FlowEntry: {
    अगर (मूल_2 && (isa<Mapping_गाँठ>(मूल_2) || isa<Sequence_गाँठ>(मूल_2)))
      वापस नया (गाँठ_Allocator) Null_गाँठ(stream.CurrentDoc);

    setError("Unexpected token", T);
    वापस nullptr;
  }
  हाल Token::TK_Error:
    वापस nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  वापस nullptr;
}

bool Document::parseDirectives() {
  bool isDirective = false;
  जबतक (true) {
    Token T = peek_अग्र();
    अगर (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = true;
    } अन्यथा अगर (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = true;
    } अन्यथा {
      अवरोध;
    }
  }
  वापस isDirective;
}

व्योम Document::parseYAMLDirective() {
  get_अग्र(); // Eat %YAML <version>
}

व्योम Document::parseTAGDirective() {
  Token Tag = get_अग्र(); // %TAG <handle> <prefix>
  StringRef T = Tag.Range;
  // Strip %TAG
  T = T.substr(T.find_first_of(" \t")).ltrim(" \t");
  std::माप_प्रकार HandleEnd = T.find_first_of(" \t");
  StringRef TagHandle = T.substr(0, HandleEnd);
  StringRef TagPrefix = T.substr(HandleEnd).ltrim(" \t");
  TagMap[TagHandle] = TagPrefix;
}

bool Document::expectToken(पूर्णांक TK) {
  Token T = get_अग्र();
  अगर (T.Kind != TK) {
    setError("Unexpected token", T);
    वापस false;
  }
  वापस true;
}
