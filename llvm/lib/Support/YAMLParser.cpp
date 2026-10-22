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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/YAMLParser.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/AllocatorList.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ArrayRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/STLExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallString.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallVector.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Twine.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Compiler.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MemoryBuffer.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/SMLoc.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/SourceMgr.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Unicode.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <cassert>
#ଅନ୍ତର୍ଭାଵ <cstddef>
#ଅନ୍ତର୍ଭାଵ <cstdint>
#ଅନ୍ତର୍ଭାଵ <map>
#ଅନ୍ତର୍ଭାଵ <memory>
#ଅନ୍ତର୍ଭାଵ <string>
#ଅନ୍ତର୍ଭାଵ <system_error>
#ଅନ୍ତର୍ଭାଵ <utility>

using ନାମାକାଶ llvm;
using ନାମାକାଶ yaml;

ଗଣନା UnicodeEncodingForm {
  UEF_UTF32_LE, ///< UTF-32 Little Endian
  UEF_UTF32_BE, ///< UTF-32 Big Endian
  UEF_UTF16_LE, ///< UTF-16 Little Endian
  UEF_UTF16_BE, ///< UTF-16 Big Endian
  UEF_UTF8,     ///< UTF-8 or ascii.
  UEF_Unknown   ///< Not a valid Unicode encoding.
};

/// EncodingInfo - Holds the encoding type and length of the byte order mark if
///                it exists. Length is in {0, 2, 3, 4}.
using EncodingInfo = std::pair<UnicodeEncodingForm, unsigned>;

/// getUnicodeEncoding - Reads up to the first 4 bytes to determine the Unicode
///                      encoding form of \a Input.
///
/// @param Input A string of length 0 or more.
/// @returns An EncodingInfo indicating the Unicode encoding form of the input
///          and how long the byte order mark is if one exists.
ସ୍ଥିର EncodingInfo getUnicodeEncoding(StringRef ନିବେଶ) {
  ଯଦି (ନିବେଶ.empty())
    ଫେରନ୍ତୁ {UEF_Unknown, 0};

  ବାଛ (uint8_t(ନିବେଶ[0])) {
  ସ୍ଥିତି 0x00:
    ଯଦି (ନିବେଶ.size() >= 4) {
      ଯଦି (  ନିବେଶ[1] == 0
         && uint8_t(ନିବେଶ[2]) == 0xFE
         && uint8_t(ନିବେଶ[3]) == 0xFF)
        ଫେରନ୍ତୁ {UEF_UTF32_BE, 4};
      ଯଦି (ନିବେଶ[1] == 0 && ନିବେଶ[2] == 0 && ନିବେଶ[3] != 0)
        ଫେରନ୍ତୁ {UEF_UTF32_BE, 0};
    }

    ଯଦି (ନିବେଶ.size() >= 2 && ନିବେଶ[1] != 0)
      ଫେରନ୍ତୁ {UEF_UTF16_BE, 0};
    ଫେରନ୍ତୁ {UEF_Unknown, 0};
  ସ୍ଥିତି 0xFF:
    ଯଦି (  ନିବେଶ.size() >= 4
       && uint8_t(ନିବେଶ[1]) == 0xFE
       && ନିବେଶ[2] == 0
       && ନିବେଶ[3] == 0)
      ଫେରନ୍ତୁ {UEF_UTF32_LE, 4};

    ଯଦି (ନିବେଶ.size() >= 2 && uint8_t(ନିବେଶ[1]) == 0xFE)
      ଫେରନ୍ତୁ {UEF_UTF16_LE, 2};
    ଫେରନ୍ତୁ {UEF_Unknown, 0};
  ସ୍ଥିତି 0xFE:
    ଯଦି (ନିବେଶ.size() >= 2 && uint8_t(ନିବେଶ[1]) == 0xFF)
      ଫେରନ୍ତୁ {UEF_UTF16_BE, 2};
    ଫେରନ୍ତୁ {UEF_Unknown, 0};
  ସ୍ଥିତି 0xEF:
    ଯଦି (  ନିବେଶ.size() >= 3
       && uint8_t(ନିବେଶ[1]) == 0xBB
       && uint8_t(ନିବେଶ[2]) == 0xBF)
      ଫେରନ୍ତୁ {UEF_UTF8, 3};
    ଫେରନ୍ତୁ {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  ଯଦି (ନିବେଶ.size() >= 4 && ନିବେଶ[1] == 0 && ନିବେଶ[2] == 0 && ନିବେଶ[3] == 0)
    ଫେରନ୍ତୁ {UEF_UTF32_LE, 0};

  ଯଦି (ନିବେଶ.size() >= 2 && ନିବେଶ[1] == 0)
    ଫେରନ୍ତୁ {UEF_UTF16_LE, 0};

  ଫେରନ୍ତୁ {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
ଶୂନ୍ୟ ଗଣ୍ଠି::anchor() {}
ଶୂନ୍ୟ Null_ଗଣ୍ଠି::anchor() {}
ଶୂନ୍ୟ Scalar_ଗଣ୍ଠି::anchor() {}
ଶୂନ୍ୟ Block_Scalar_ଗଣ୍ଠି::anchor() {}
ଶୂନ୍ୟ ଚାବି_ମୂଲ୍ୟ_ଗଣ୍ଠି::anchor() {}
ଶୂନ୍ୟ Mapping_ଗଣ୍ଠି::anchor() {}
ଶୂନ୍ୟ Sequence_ଗଣ୍ଠି::anchor() {}
ଶୂନ୍ୟ Alias_ଗଣ୍ଠି::anchor() {}

ନାମାକାଶ llvm {
ନାମାକାଶ yaml {

/// Token - A single YAML token.
ସଂରଚନା Token {
  ଗଣନା TokenKind {
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
    TK_ଚାବି,
    TK_ମୂଲ୍ୟ,
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
  std::string ମୂଲ୍ୟ;

  Token() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;
};

} // end namespace yaml
} // end namespace llvm

using TokenQueueT = BumpPtrList<Token>;

ନାମାକାଶ {

/// This struct is used to track simple keys.
///
/// Simple keys are handled by creating an entry in SimpleKeys for each Token
/// which could legally be the start of a simple key. When peekNext is called,
/// if the Token To be returned is referenced by a SimpleKey, we continue
/// tokenizing until that potential simple key has either been found to not be
/// a simple key (we moved on to the next line or went further than 1024 chars).
/// Or when we run into a Value, and then insert a Key token (and possibly
/// others) before the SimpleKey's Tok.
ସଂରଚନା Simple_ଚାବି {
  TokenQueueT::iterator Tok;
  unsigned Column = 0;
  unsigned ଧାଡ଼ି = 0;
  unsigned FlowLevel = 0;
  ତାର୍କିକ IsRequired = ମିଥ୍ୟା;

  ତାର୍କିକ operator ==(ଅଚଳ Simple_ଚାବି &Other) {
    ଫେରନ୍ତୁ Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
using UTF8Decoded = std::pair<uint32_t, unsigned>;

ସ୍ଥିର UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator End = Range.end();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  ଯଦି (Position < End && (*Position & 0x80) == 0) {
    ଫେରନ୍ତୁ {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  ଯଦି (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    ଯଦି (codepoint >= 0x80)
      ଫେରନ୍ତୁ {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  ଯଦି (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    ଯଦି (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      ଫେରନ୍ତୁ {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  ଯଦି (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    ଯଦି (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      ଫେରନ୍ତୁ {codepoint, 4};
  }
  ଫେରନ୍ତୁ {0, 0};
}

ନାମାକାଶ llvm {
ନାମାକାଶ yaml {

/// Scans YAML tokens from a MemoryBuffer.
ଶ୍ରେଣୀ Scanner {
ସର୍ବସାଧାରଣ:
  Scanner(StringRef ନିବେଶ, SourceMgr &SM, ତାର୍କିକ ShowColors = ସତ୍ୟ,
          std::error_code *EC = nullptr);
  Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, ତାର୍କିକ ShowColors = ସତ୍ୟ,
          std::error_code *EC = nullptr);

  /// Parse the next token and return it without popping it.
  Token &peekNext();

  /// Parse the next token and pop it from the queue.
  Token getNext();

  ଶୂନ୍ୟ printError(SMLoc Loc, SourceMgr::DiagKind Kind, ଅଚଳ Twine &Message,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.PrintMessage(Loc, Kind, Message, Ranges, /* FixIts= */ {}, ShowColors);
  }

  ଶୂନ୍ୟ setError(ଅଚଳ Twine &Message, StringRef::iterator Position) {
    ଯଦି (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    ଯଦି (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    ଯଦି (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = ସତ୍ୟ;
  }

  /// Returns true if an error occurred while parsing.
  ତାର୍କିକ failed() {
    ଫେରନ୍ତୁ Failed;
  }

ବ୍ୟକ୍ତିଗତ:
  ଶୂନ୍ୟ init(MemoryBufferRef Buffer);

  StringRef current_ନିବେଶ() {
    ଫେରନ୍ତୁ StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    ଫେରନ୍ତୁ ::decodeUTF8(StringRef(Position, End - Position));
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
  ଶୂନ୍ୟ advanceWhile(SkipWhileFunc Func);

  /// Scan ns-uri-char[39]s starting at Cur.
  ///
  /// This updates Cur and Column while scanning.
  ଶୂନ୍ୟ scan_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  ତାର୍କିକ consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  ଶୂନ୍ୟ skip(uint32_t Distance);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  ତାର୍କିକ isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  ତାର୍କିକ isPlainSafeNonBlank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  ତାର୍କିକ is_ଧାଡ଼ି_Empty(StringRef ଧାଡ଼ି);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  ତାର୍କିକ consume_ଧାଡ଼ି_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  ଶୂନ୍ୟ save_Simple_ଚାବି_Candidate( TokenQueueT::iterator Tok
                             , unsigned AtColumn
                             , ତାର୍କିକ IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  ଶୂନ୍ୟ remove_Stale_Simple_ଚାବି_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  ଶୂନ୍ୟ remove_Simple_ଚାବି_Candidates_On_Flow_Level(unsigned Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  ତାର୍କିକ unrollIndent(ପୂର୍ଣ୍ଣସଂଖ୍ୟା ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  ତାର୍କିକ rollIndent( ପୂର୍ଣ୍ଣସଂଖ୍ୟା ToColumn
                 , Token::TokenKind Kind
                 , TokenQueueT::iterator ଭର୍ତ୍ତି_Point);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  ଶୂନ୍ୟ skipComment();

  /// Skip whitespace and comments until the start of the next token.
  ଶୂନ୍ୟ scanToNextToken();

  /// Must be the first token generated.
  ତାର୍କିକ scanStreamStart();

  /// Generate tokens needed to close out the stream.
  ତାର୍କିକ scanStreamEnd();

  /// Scan a %BLAH directive.
  ତାର୍କିକ scanDirective();

  /// Scan a ... or ---.
  ତାର୍କିକ scanDocumentIndicator(ତାର୍କିକ IsStart);

  /// Scan a [ or { and generate the proper flow collection start token.
  ତାର୍କିକ scanFlowCollectionStart(ତାର୍କିକ IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  ତାର୍କିକ scanFlowCollectionEnd(ତାର୍କିକ IsSequence);

  /// Scan the , that separates entries in a flow collection.
  ତାର୍କିକ scanFlowEntry();

  /// Scan the - that starts block sequence entries.
  ତାର୍କିକ scanBlockEntry();

  /// Scan an explicit ? indicating a key.
  ତାର୍କିକ scan_ଚାବି();

  /// Scan an explicit : indicating a value.
  ତାର୍କିକ scan_ମୂଲ୍ୟ();

  /// Scan a quoted scalar.
  ତାର୍କିକ scanFlowScalar(ତାର୍କିକ IsDoubleQuoted);

  /// Scan an unquoted scalar.
  ତାର୍କିକ scanPlainScalar();

  /// Scan an Alias or Anchor starting with * or &.
  ତାର୍କିକ scanAliasOrAnchor(ତାର୍କିକ IsAlias);

  /// Scan a block scalar starting with | or >.
  ତାର୍କିକ scanBlockScalar(ତାର୍କିକ IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  ତାର୍କିକ scanBlockScalarIndicators(ଅକ୍ଷର &StyleIndicator, ଅକ୍ଷର &ChompingIndicator,
                                 unsigned &IndentIndicator, ତାର୍କିକ &IsDone);

  /// Scan a style indicator in a block scalar header.
  ଅକ୍ଷର scanBlockStyleIndicator();

  /// Scan a chomping indicator in a block scalar header.
  ଅକ୍ଷର scanBlockChompingIndicator();

  /// Scan an indentation indicator in a block scalar header.
  unsigned scanBlockIndentationIndicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  ତାର୍କିକ scanBlockScalarHeader(ଅକ୍ଷର &ChompingIndicator, unsigned &IndentIndicator,
                             ତାର୍କିକ &IsDone);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  ତାର୍କିକ findBlockScalarIndent(unsigned &BlockIndent, unsigned BlockExitIndent,
                             unsigned &ଧାଡ଼ି_Breaks, ତାର୍କିକ &IsDone);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  ତାର୍କିକ scanBlockScalarIndent(unsigned BlockIndent, unsigned BlockExitIndent,
                             ତାର୍କିକ &IsDone);

  /// Scan a tag of the form !stuff.
  ତାର୍କିକ scanTag();

  /// Dispatch to the next scanning function based on \a *Cur.
  ତାର୍କିକ fetchMoreTokens();

  /// The SourceMgr used for diagnostics and buffer management.
  SourceMgr &SM;

  /// The original input.
  MemoryBufferRef ନିବେଶ_Buffer;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator End;

  /// Current YAML indentation level in spaces.
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Indent;

  /// Current column number in Unicode code points.
  unsigned Column;

  /// Current line number.
  unsigned ଧାଡ଼ି;

  /// How deep we are in flow style containers. 0 Means at block level.
  unsigned FlowLevel;

  /// Are we at the start of the stream?
  ତାର୍କିକ IsStartOfStream;

  /// Can the next token be the start of a simple key?
  ତାର୍କିକ Is_Simple_ଚାବି_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  ତାର୍କିକ Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow;

  /// True if an error has occurred.
  ତାର୍କିକ Failed;

  /// Should colors be used when printing out the diagnostic messages?
  ତାର୍କିକ ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  TokenQueueT TokenQueue;

  /// Indentation levels.
  SmallVector<ପୂର୍ଣ୍ଣସଂଖ୍ୟା, 4> Indents;

  /// Potential simple keys.
  SmallVector<Simple_ଚାବି, 4> Simple_ଚାବିଗୁଡ଼ିକ;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
ସ୍ଥିର ଶୂନ୍ୟ encodeUTF8( uint32_t Unicode_Scalar_ମୂଲ୍ୟ
                      , SmallVectorImpl<ଅକ୍ଷର> &Result) {
  ଯଦି (Unicode_Scalar_ମୂଲ୍ୟ <= 0x7F) {
    Result.ଠେଲ_back(Unicode_Scalar_ମୂଲ୍ୟ & 0x7F);
  } ଅନ୍ୟଥା ଯଦି (Unicode_Scalar_ମୂଲ୍ୟ <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((Unicode_Scalar_ମୂଲ୍ୟ & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Unicode_Scalar_ମୂଲ୍ୟ & 0x3F);
    Result.ଠେଲ_back(FirstByte);
    Result.ଠେଲ_back(SecondByte);
  } ଅନ୍ୟଥା ଯଦି (Unicode_Scalar_ମୂଲ୍ୟ <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((Unicode_Scalar_ମୂଲ୍ୟ & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_ମୂଲ୍ୟ & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Unicode_Scalar_ମୂଲ୍ୟ & 0x3F);
    Result.ଠେଲ_back(FirstByte);
    Result.ଠେଲ_back(SecondByte);
    Result.ଠେଲ_back(ThirdByte);
  } ଅନ୍ୟଥା ଯଦି (Unicode_Scalar_ମୂଲ୍ୟ <= 0x10FFFF) {
    uint8_t FirstByte = 0xF0 | ((Unicode_Scalar_ମୂଲ୍ୟ & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_ମୂଲ୍ୟ & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Unicode_Scalar_ମୂଲ୍ୟ & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_ମୂଲ୍ୟ & 0x3F);
    Result.ଠେଲ_back(FirstByte);
    Result.ଠେଲ_back(SecondByte);
    Result.ଠେଲ_back(ThirdByte);
    Result.ଠେଲ_back(FourthByte);
  }
}

ତାର୍କିକ yaml::dumpTokens(StringRef ନିବେଶ, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(ନିବେଶ, SM);
  ଯେତେବେଳେ (ସତ୍ୟ) {
    Token T = scanner.getNext();
    ବାଛ (T.Kind) {
    ସ୍ଥିତି Token::TK_StreamStart:
      OS << "Stream-Start: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_StreamEnd:
      OS << "Stream-End: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_VersionDirective:
      OS << "Version-Directive: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_TagDirective:
      OS << "Tag-Directive: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_DocumentStart:
      OS << "Document-Start: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_DocumentEnd:
      OS << "Document-End: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_BlockEntry:
      OS << "Block-Entry: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_BlockEnd:
      OS << "Block-End: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_BlockSequenceStart:
      OS << "Block-Sequence-Start: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_BlockMappingStart:
      OS << "Block-Mapping-Start: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_FlowEntry:
      OS << "Flow-Entry: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_FlowSequenceStart:
      OS << "Flow-Sequence-Start: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_FlowSequenceEnd:
      OS << "Flow-Sequence-End: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_FlowMappingStart:
      OS << "Flow-Mapping-Start: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_FlowMappingEnd:
      OS << "Flow-Mapping-End: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_ଚାବି:
      OS << "Key: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_ମୂଲ୍ୟ:
      OS << "Value: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_Scalar:
      OS << "Scalar: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_BlockScalar:
      OS << "Block Scalar: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_Alias:
      OS << "Alias: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_Anchor:
      OS << "Anchor: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_Tag:
      OS << "Tag: ";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_Error:
      ଭାଙ୍ଗ;
    }
    OS << T.Range << "\n";
    ଯଦି (T.Kind == Token::TK_StreamEnd)
      ଭାଙ୍ଗ;
    ଅନ୍ୟଥା ଯଦି (T.Kind == Token::TK_Error)
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ yaml::scanTokens(StringRef ନିବେଶ) {
  SourceMgr SM;
  Scanner scanner(ନିବେଶ, SM);
  ଯେତେବେଳେ (ସତ୍ୟ) {
    Token T = scanner.getNext();
    ଯଦି (T.Kind == Token::TK_StreamEnd)
      ଭାଙ୍ଗ;
    ଅନ୍ୟଥା ଯଦି (T.Kind == Token::TK_Error)
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

std::string yaml::escape(StringRef ନିବେଶ, ତାର୍କିକ EscapePrintable) {
  std::string Escaped_ନିବେଶ;
  ପାଇଁ (StringRef::iterator i = ନିବେଶ.begin(), e = ନିବେଶ.end(); i != e; ++i) {
    ଯଦି (*i == '\\')
      Escaped_ନିବେଶ += "\\\\";
    ଅନ୍ୟଥା ଯଦି (*i == '"')
      Escaped_ନିବେଶ += "\\\"";
    ଅନ୍ୟଥା ଯଦି (*i == 0)
      Escaped_ନିବେଶ += "\\0";
    ଅନ୍ୟଥା ଯଦି (*i == 0x07)
      Escaped_ନିବେଶ += "\\a";
    ଅନ୍ୟଥା ଯଦି (*i == 0x08)
      Escaped_ନିବେଶ += "\\b";
    ଅନ୍ୟଥା ଯଦି (*i == 0x09)
      Escaped_ନିବେଶ += "\\t";
    ଅନ୍ୟଥା ଯଦି (*i == 0x0A)
      Escaped_ନିବେଶ += "\\n";
    ଅନ୍ୟଥା ଯଦି (*i == 0x0B)
      Escaped_ନିବେଶ += "\\v";
    ଅନ୍ୟଥା ଯଦି (*i == 0x0C)
      Escaped_ନିବେଶ += "\\f";
    ଅନ୍ୟଥା ଯଦି (*i == 0x0D)
      Escaped_ନିବେଶ += "\\r";
    ଅନ୍ୟଥା ଯଦି (*i == 0x1B)
      Escaped_ନିବେଶ += "\\e";
    ଅନ୍ୟଥା ଯଦି ((unsigned ଅକ୍ଷର)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_ନିବେଶ += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } ଅନ୍ୟଥା ଯଦି (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_ମୂଲ୍ୟ
        = decodeUTF8(StringRef(i, ନିବେଶ.end() - i));
      ଯଦି (Unicode_Scalar_ମୂଲ୍ୟ.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_ନିବେଶ, Val);
        // FIXME: Error reporting.
        ଫେରନ୍ତୁ Escaped_ନିବେଶ;
      }
      ଯଦି (Unicode_Scalar_ମୂଲ୍ୟ.first == 0x85)
        Escaped_ନିବେଶ += "\\N";
      ଅନ୍ୟଥା ଯଦି (Unicode_Scalar_ମୂଲ୍ୟ.first == 0xA0)
        Escaped_ନିବେଶ += "\\_";
      ଅନ୍ୟଥା ଯଦି (Unicode_Scalar_ମୂଲ୍ୟ.first == 0x2028)
        Escaped_ନିବେଶ += "\\L";
      ଅନ୍ୟଥା ଯଦି (Unicode_Scalar_ମୂଲ୍ୟ.first == 0x2029)
        Escaped_ନିବେଶ += "\\P";
      ଅନ୍ୟଥା ଯଦି (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_ମୂଲ୍ୟ.first))
        Escaped_ନିବେଶ += StringRef(i, Unicode_Scalar_ମୂଲ୍ୟ.second);
      ଅନ୍ୟଥା {
        std::string HexStr = utohexstr(Unicode_Scalar_ମୂଲ୍ୟ.first);
        ଯଦି (HexStr.size() <= 2)
          Escaped_ନିବେଶ += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        ଅନ୍ୟଥା ଯଦି (HexStr.size() <= 4)
          Escaped_ନିବେଶ += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        ଅନ୍ୟଥା ଯଦି (HexStr.size() <= 8)
          Escaped_ନିବେଶ += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += Unicode_Scalar_ମୂଲ୍ୟ.second - 1;
    } ଅନ୍ୟଥା {
      Escaped_ନିବେଶ.ଠେଲ_back(*i);
    }
  }
  ଫେରନ୍ତୁ Escaped_ନିବେଶ;
}

std::optional<ତାର୍କିକ> yaml::parseBool(StringRef S) {
  ବାଛ (S.size()) {
  ସ୍ଥିତି 1:
    ବାଛ (S.front()) {
    ସ୍ଥିତି 'y':
    ସ୍ଥିତି 'Y':
      ଫେରନ୍ତୁ ସତ୍ୟ;
    ସ୍ଥିତି 'n':
    ସ୍ଥିତି 'N':
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ std::nullopt;
    }
  ସ୍ଥିତି 2:
    ବାଛ (S.front()) {
    ସ୍ଥିତି 'O':
      ଯଦି (S[1] == 'N') // ON
        ଫେରନ୍ତୁ ସତ୍ୟ;
      [[fallthrough]];
    ସ୍ଥିତି 'o':
      ଯଦି (S[1] == 'n') //[Oo]n
        ଫେରନ୍ତୁ ସତ୍ୟ;
      ଫେରନ୍ତୁ std::nullopt;
    ସ୍ଥିତି 'N':
      ଯଦି (S[1] == 'O') // NO
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      [[fallthrough]];
    ସ୍ଥିତି 'n':
      ଯଦି (S[1] == 'o') //[Nn]o
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      ଫେରନ୍ତୁ std::nullopt;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ std::nullopt;
    }
  ସ୍ଥିତି 3:
    ବାଛ (S.front()) {
    ସ୍ଥିତି 'O':
      ଯଦି (S.drop_front() == "FF") // OFF
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      [[fallthrough]];
    ସ୍ଥିତି 'o':
      ଯଦି (S.drop_front() == "ff") //[Oo]ff
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      ଫେରନ୍ତୁ std::nullopt;
    ସ୍ଥିତି 'Y':
      ଯଦି (S.drop_front() == "ES") // YES
        ଫେରନ୍ତୁ ସତ୍ୟ;
      [[fallthrough]];
    ସ୍ଥିତି 'y':
      ଯଦି (S.drop_front() == "es") //[Yy]es
        ଫେରନ୍ତୁ ସତ୍ୟ;
      ଫେରନ୍ତୁ std::nullopt;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ std::nullopt;
    }
  ସ୍ଥିତି 4:
    ବାଛ (S.front()) {
    ସ୍ଥିତି 'T':
      ଯଦି (S.drop_front() == "RUE") // TRUE
        ଫେରନ୍ତୁ ସତ୍ୟ;
      [[fallthrough]];
    ସ୍ଥିତି 't':
      ଯଦି (S.drop_front() == "rue") //[Tt]rue
        ଫେରନ୍ତୁ ସତ୍ୟ;
      ଫେରନ୍ତୁ std::nullopt;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ std::nullopt;
    }
  ସ୍ଥିତି 5:
    ବାଛ (S.front()) {
    ସ୍ଥିତି 'F':
      ଯଦି (S.drop_front() == "ALSE") // FALSE
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      [[fallthrough]];
    ସ୍ଥିତି 'f':
      ଯଦି (S.drop_front() == "alse") //[Ff]alse
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      ଫେରନ୍ତୁ std::nullopt;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ std::nullopt;
    }
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଫେରନ୍ତୁ std::nullopt;
  }
}

Scanner::Scanner(StringRef ନିବେଶ, SourceMgr &sm, ତାର୍କିକ ShowColors,
                 std::error_code *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(MemoryBufferRef(ନିବେଶ, "YAML"));
}

Scanner::Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, ତାର୍କିକ ShowColors,
                 std::error_code *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(Buffer);
}

ଶୂନ୍ୟ Scanner::init(MemoryBufferRef Buffer) {
  ନିବେଶ_Buffer = Buffer;
  Current = ନିବେଶ_Buffer.getBufferStart();
  End = ନିବେଶ_Buffer.getBufferEnd();
  Indent = -1;
  Column = 0;
  ଧାଡ଼ି = 0;
  FlowLevel = 0;
  IsStartOfStream = ସତ୍ୟ;
  Is_Simple_ଚାବି_Allowed = ସତ୍ୟ;
  Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow = ମିଥ୍ୟା;
  Failed = ମିଥ୍ୟା;
  std::unique_ptr<MemoryBuffer> ନିବେଶ_Buffer_Owner =
      MemoryBuffer::getMemBuffer(Buffer, /*RequiresNullTerminator=*/ମିଥ୍ୟା);
  SM.AddNewSourceBuffer(std::move(ନିବେଶ_Buffer_Owner), SMLoc());
}

Token &Scanner::peekNext() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  ତାର୍କିକ NeedMore = ମିଥ୍ୟା;
  ଯେତେବେଳେ (ସତ୍ୟ) {
    ଯଦି (TokenQueue.empty() || NeedMore) {
      ଯଦି (!fetchMoreTokens()) {
        TokenQueue.clear();
        Simple_ଚାବିଗୁଡ଼ିକ.clear();
        TokenQueue.ଠେଲ_back(Token());
        ଫେରନ୍ତୁ TokenQueue.front();
      }
    }
    ପ୍ରତିଜ୍ଞା(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_ଚାବି_Candidates();
    Simple_ଚାବି SK;
    SK.Tok = TokenQueue.begin();
    ଯଦି (!is_contained(Simple_ଚାବିଗୁଡ଼ିକ, SK))
      ଭାଙ୍ଗ;
    ଅନ୍ୟଥା
      NeedMore = ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  ଯଦି (!TokenQueue.empty())
    TokenQueue.କାଢ଼_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  ଯଦି (TokenQueue.empty())
    TokenQueue.resetAlloc();

  ଫେରନ୍ତୁ Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  ଯଦି (Position == End)
    ଫେରନ୍ତୁ Position;
  // Check 7 bit c-printable - b-char.
  ଯଦି (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    ଫେରନ୍ତୁ Position + 1;

  // Check for valid UTF-8.
  ଯଦି (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    ଯଦି (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      ଫେରନ୍ତୁ Position + u8d.second;
  }
  ଫେରନ୍ତୁ Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  ଯଦି (Position == End)
    ଫେରନ୍ତୁ Position;
  ଯଦି (*Position == 0x0D) {
    ଯଦି (Position + 1 != End && *(Position + 1) == 0x0A)
      ଫେରନ୍ତୁ Position + 2;
    ଫେରନ୍ତୁ Position + 1;
  }

  ଯଦି (*Position == 0x0A)
    ଫେରନ୍ତୁ Position + 1;
  ଫେରନ୍ତୁ Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  ଯଦି (Position == End)
    ଫେରନ୍ତୁ Position;
  ଯଦି (*Position == ' ')
    ଫେରନ୍ତୁ Position + 1;
  ଫେରନ୍ତୁ Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  ଯଦି (Position == End)
    ଫେରନ୍ତୁ Position;
  ଯଦି (*Position == ' ' || *Position == '\t')
    ଫେରନ୍ତୁ Position + 1;
  ଫେରନ୍ତୁ Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  ଯଦି (Position == End)
    ଫେରନ୍ତୁ Position;
  ଯଦି (*Position == ' ' || *Position == '\t')
    ଫେରନ୍ତୁ Position;
  ଫେରନ୍ତୁ skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  ଯେତେବେଳେ (ସତ୍ୟ) {
    StringRef::iterator i = (ଏହି->*Func)(Position);
    ଯଦି (i == Position)
      ଭାଙ୍ଗ;
    Position = i;
  }
  ଫେରନ୍ତୁ Position;
}

ଶୂନ୍ୟ Scanner::advanceWhile(SkipWhileFunc Func) {
  auto Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

ସ୍ଥିର ତାର୍କିକ is_ns_hex_digit(ଅଚଳ ଅକ୍ଷର C) { ଫେରନ୍ତୁ isAlnum(C); }

ସ୍ଥିର ତାର୍କିକ is_ns_word_char(ଅଚଳ ଅକ୍ଷର C) { ଫେରନ୍ତୁ C == '-' || isAlpha(C); }

ଶୂନ୍ୟ Scanner::scan_ns_uri_char() {
  ଯେତେବେଳେ (ସତ୍ୟ) {
    ଯଦି (Current == End)
      ଭାଙ୍ଗ;
    ଯଦି ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } ଅନ୍ୟଥା {
      ଭାଙ୍ଗ;
    }
  }
}

ତାର୍କିକ Scanner::consume(uint32_t Expected) {
  ଯଦି (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  ଯଦି (Current == End)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  ଯଦି (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ଶୂନ୍ୟ Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  ପ୍ରତିଜ୍ଞା(Current <= End && "Skipped past the end");
}

ତାର୍କିକ Scanner::isBlankOrBreak(StringRef::iterator Position) {
  ଯଦି (Position == End)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଫେରନ୍ତୁ *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

ତାର୍କିକ Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  ଯଦି (Position == End || isBlankOrBreak(Position))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::is_ଧାଡ଼ି_Empty(StringRef ଧାଡ଼ି) {
  ପାଇଁ (ଅଚଳ auto *Position = ଧାଡ଼ି.begin(); Position != ଧାଡ଼ି.end(); ++Position)
    ଯଦି (!isBlankOrBreak(Position))
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::consume_ଧାଡ଼ି_Break_If_Present() {
  auto Next = skip_b_break(Current);
  ଯଦି (Next == Current)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  Column = 0;
  ++ଧାଡ଼ି;
  Current = Next;
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ଶୂନ୍ୟ Scanner::save_Simple_ଚାବି_Candidate( TokenQueueT::iterator Tok
                                    , unsigned AtColumn
                                    , ତାର୍କିକ IsRequired) {
  ଯଦି (Is_Simple_ଚାବି_Allowed) {
    Simple_ଚାବି SK;
    SK.Tok = Tok;
    SK.ଧାଡ଼ି = ଧାଡ଼ି;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_ଚାବିଗୁଡ଼ିକ.ଠେଲ_back(SK);
  }
}

ଶୂନ୍ୟ Scanner::remove_Stale_Simple_ଚାବି_Candidates() {
  ପାଇଁ (SmallVectorImpl<Simple_ଚାବି>::iterator i = Simple_ଚାବିଗୁଡ଼ିକ.begin();
                                            i != Simple_ଚାବିଗୁଡ଼ିକ.end();) {
    ଯଦି (i->ଧାଡ଼ି != ଧାଡ଼ି || i->Column + 1024 < Column) {
      ଯଦି (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_ଚାବିଗୁଡ଼ିକ.erase(i);
    } ଅନ୍ୟଥା {
      ++i;
    }
  }
}

ଶୂନ୍ୟ Scanner::remove_Simple_ଚାବି_Candidates_On_Flow_Level(unsigned Level) {
  ଯଦି (!Simple_ଚାବିଗୁଡ଼ିକ.empty() && (Simple_ଚାବିଗୁଡ଼ିକ.end() - 1)->FlowLevel == Level)
    Simple_ଚାବିଗୁଡ଼ିକ.କାଢ଼_back();
}

ତାର୍କିକ Scanner::unrollIndent(ପୂର୍ଣ୍ଣସଂଖ୍ୟା ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  ଯଦି (FlowLevel != 0)
    ଫେରନ୍ତୁ ସତ୍ୟ;

  ଯେତେବେଳେ (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.ଠେଲ_back(T);
    Indent = Indents.କାଢ଼_back_val();
  }

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::rollIndent( ପୂର୍ଣ୍ଣସଂଖ୍ୟା ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator ଭର୍ତ୍ତି_Point) {
  ଯଦି (FlowLevel)
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଯଦି (Indent < ToColumn) {
    Indents.ଠେଲ_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.ଭର୍ତ୍ତି(ଭର୍ତ୍ତି_Point, T);
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ଶୂନ୍ୟ Scanner::skipComment() {
  ଯଦି (Current == End || *Current != '#')
    ଫେରନ୍ତୁ;
  ଯେତେବେଳେ (ସତ୍ୟ) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    ଯଦି (I == Current)
      ଭାଙ୍ଗ;
    Current = I;
    ++Column;
  }
}

ଶୂନ୍ୟ Scanner::scanToNextToken() {
  ଯେତେବେଳେ (ସତ୍ୟ) {
    ଯେତେବେଳେ (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    ଯଦି (i == Current)
      ଭାଙ୍ଗ;
    Current = i;
    ++ଧାଡ଼ି;
    Column = 0;
    // New lines may start a simple key.
    ଯଦି (!FlowLevel)
      Is_Simple_ଚାବି_Allowed = ସତ୍ୟ;
  }
}

ତାର୍କିକ Scanner::scanStreamStart() {
  IsStartOfStream = ମିଥ୍ୟା;

  EncodingInfo EI = getUnicodeEncoding(current_ନିବେଶ());

  Token T;
  T.Kind = Token::TK_StreamStart;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.ଠେଲ_back(T);
  Current += EI.second;
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  ଯଦି (Column != 0) {
    Column = 0;
    ++ଧାଡ଼ି;
  }

  unrollIndent(-1);
  Simple_ଚାବିଗୁଡ଼ିକ.clear();
  Is_Simple_ଚାବି_Allowed = ମିଥ୍ୟା;
  Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow = ମିଥ୍ୟା;

  Token T;
  T.Kind = Token::TK_StreamEnd;
  T.Range = StringRef(Current, 0);
  TokenQueue.ଠେଲ_back(T);
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::scanDirective() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_ଚାବିଗୁଡ଼ିକ.clear();
  Is_Simple_ଚାବି_Allowed = ମିଥ୍ୟା;
  Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow = ମିଥ୍ୟା;

  StringRef::iterator Start = Current;
  consume('%');
  StringRef::iterator NameStart = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(NameStart, Current - NameStart);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  ଯଦି (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ଠେଲ_back(T);
    ଫେରନ୍ତୁ ସତ୍ୟ;
  } ଅନ୍ୟଥା ଯଦି(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ଠେଲ_back(T);
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

ତାର୍କିକ Scanner::scanDocumentIndicator(ତାର୍କିକ IsStart) {
  unrollIndent(-1);
  Simple_ଚାବିଗୁଡ଼ିକ.clear();
  Is_Simple_ଚାବି_Allowed = ମିଥ୍ୟା;
  Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow = ମିଥ୍ୟା;

  Token T;
  T.Kind = IsStart ? Token::TK_DocumentStart : Token::TK_DocumentEnd;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.ଠେଲ_back(T);
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::scanFlowCollectionStart(ତାର୍କିକ IsSequence) {
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ଠେଲ_back(T);

  // [ and { may begin a simple key.
  save_Simple_ଚାବି_Candidate(--TokenQueue.end(), Column - 1, ମିଥ୍ୟା);

  // And may also be followed by a simple key.
  Is_Simple_ଚାବି_Allowed = ସତ୍ୟ;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow = ମିଥ୍ୟା;
  ++FlowLevel;
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::scanFlowCollectionEnd(ତାର୍କିକ IsSequence) {
  remove_Simple_ଚାବି_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ଚାବି_Allowed = ମିଥ୍ୟା;
  Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow = ସତ୍ୟ;
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceEnd
                      : Token::TK_FlowMappingEnd;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ଠେଲ_back(T);
  ଯଦି (FlowLevel)
    --FlowLevel;
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::scanFlowEntry() {
  remove_Simple_ଚାବି_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ଚାବି_Allowed = ସତ୍ୟ;
  Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow = ମିଥ୍ୟା;
  Token T;
  T.Kind = Token::TK_FlowEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ଠେଲ_back(T);
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::scanBlockEntry() {
  rollIndent(Column, Token::TK_BlockSequenceStart, TokenQueue.end());
  remove_Simple_ଚାବି_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ଚାବି_Allowed = ସତ୍ୟ;
  Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow = ମିଥ୍ୟା;
  Token T;
  T.Kind = Token::TK_BlockEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ଠେଲ_back(T);
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::scan_ଚାବି() {
  ଯଦି (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  remove_Simple_ଚାବି_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_ଚାବି_Allowed = !FlowLevel;
  Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow = ମିଥ୍ୟା;

  Token T;
  T.Kind = Token::TK_ଚାବି;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ଠେଲ_back(T);
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::scan_ମୂଲ୍ୟ() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  ଯଦି (!Simple_ଚାବିଗୁଡ଼ିକ.empty()) {
    Simple_ଚାବି SK = Simple_ଚାବିଗୁଡ଼ିକ.କାଢ଼_back_val();
    Token T;
    T.Kind = Token::TK_ଚାବି;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    ପାଇଁ (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      ଯଦି (i == SK.Tok)
        ଭାଙ୍ଗ;
    }
    ଯଦି (i == e) {
      Failed = ସତ୍ୟ;
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }
    i = TokenQueue.ଭର୍ତ୍ତି(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    Is_Simple_ଚାବି_Allowed = ମିଥ୍ୟା;
  } ଅନ୍ୟଥା {
    ଯଦି (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    Is_Simple_ଚାବି_Allowed = !FlowLevel;
  }
  Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow = ମିଥ୍ୟା;

  Token T;
  T.Kind = Token::TK_ମୂଲ୍ୟ;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ଠେଲ_back(T);
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE ସ୍ଥିର ତାର୍କିକ
wasEscaped(StringRef::iterator First, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
ସ୍ଥିର ତାର୍କିକ wasEscaped(StringRef::iterator First,
                       StringRef::iterator Position) {
  ପ୍ରତିଜ୍ଞା(Position - 1 >= First);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  ଯେତେବେଳେ (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  ଫେରନ୍ତୁ (Position - 1 - I) % 2 == 1;
}

ତାର୍କିକ Scanner::scanFlowScalar(ତାର୍କିକ IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  ଯଦି (IsDoubleQuoted) {
    କର {
      ++Current;
      ଯେତେବେଳେ (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } ଯେତେବେଳେ (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } ଅନ୍ୟଥା {
    skip(1);
    ଯେତେବେଳେ (Current != End) {
      // Skip a ' followed by another '.
      ଯଦି (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        ଜାରି;
      } ଅନ୍ୟଥା ଯଦି (*Current == '\'')
        ଭାଙ୍ଗ;
      StringRef::iterator i = skip_nb_char(Current);
      ଯଦି (i == Current) {
        i = skip_b_break(Current);
        ଯଦି (i == Current)
          ଭାଙ୍ଗ;
        Current = i;
        Column = 0;
        ++ଧାଡ଼ି;
      } ଅନ୍ୟଥା {
        ଯଦି (i == End)
          ଭାଙ୍ଗ;
        Current = i;
        ++Column;
      }
    }
  }

  ଯଦି (Current == End) {
    setError("Expected quote at end of scalar", Current);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ଠେଲ_back(T);

  save_Simple_ଚାବି_Candidate(--TokenQueue.end(), ColStart, ମିଥ୍ୟା);

  Is_Simple_ଚାବି_Allowed = ମିଥ୍ୟା;
  Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow = ସତ୍ୟ;

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  unsigned LeadingBlanks = 0;
  ପ୍ରତିଜ୍ଞା(Indent >= -1 && "Indent must be >= -1 !");
  unsigned indent = static_cast<unsigned>(Indent + 1);
  ଯେତେବେଳେ (Current != End) {
    ଯଦି (*Current == '#')
      ଭାଙ୍ଗ;

    ଯେତେବେଳେ (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      ଯଦି (i == Current)
        ଭାଙ୍ଗ;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    ଯଦି (!isBlankOrBreak(Current))
      ଭାଙ୍ଗ;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    ଯେତେବେଳେ (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      ଯଦି (i != Tmp) {
        ଯଦି (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          ଫେରନ୍ତୁ ମିଥ୍ୟା;
        }
        Tmp = i;
        ++Column;
      } ଅନ୍ୟଥା {
        i = skip_b_break(Tmp);
        ଯଦି (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++ଧାଡ଼ି;
      }
    }

    ଯଦି (!FlowLevel && Column < indent)
      ଭାଙ୍ଗ;

    Current = Tmp;
  }
  ଯଦି (Start == Current) {
    setError("Got empty plain scalar", Start);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ଠେଲ_back(T);

  // Plain scalars can be simple keys.
  save_Simple_ଚାବି_Candidate(--TokenQueue.end(), ColStart, ମିଥ୍ୟା);

  Is_Simple_ଚାବି_Allowed = ମିଥ୍ୟା;
  Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow = ମିଥ୍ୟା;

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::scanAliasOrAnchor(ତାର୍କିକ IsAlias) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1);
  ଯେତେବେଳେ (Current != End) {
    ଯଦି (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      ଭାଙ୍ଗ;
    StringRef::iterator i = skip_ns_char(Current);
    ଯଦି (i == Current)
      ଭାଙ୍ଗ;
    Current = i;
    ++Column;
  }

  ଯଦି (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ଠେଲ_back(T);

  // Alias and anchors can be simple keys.
  save_Simple_ଚାବି_Candidate(--TokenQueue.end(), ColStart, ମିଥ୍ୟା);

  Is_Simple_ଚାବି_Allowed = ମିଥ୍ୟା;
  Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow = ମିଥ୍ୟା;

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::scanBlockScalarIndicators(ଅକ୍ଷର &StyleIndicator,
                                        ଅକ୍ଷର &ChompingIndicator,
                                        unsigned &IndentIndicator,
                                        ତାର୍କିକ &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  ଯଦି (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ଅକ୍ଷର Scanner::scanBlockStyleIndicator() {
  ଅକ୍ଷର Indicator = ' ';
  ଯଦି (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  ଫେରନ୍ତୁ Indicator;
}

ଅକ୍ଷର Scanner::scanBlockChompingIndicator() {
  ଅକ୍ଷର Indicator = ' ';
  ଯଦି (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  ଫେରନ୍ତୁ Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
ସ୍ଥିର unsigned get_Chomped_ଧାଡ଼ି_Breaks(ଅକ୍ଷର ChompingIndicator,
                                     unsigned ଧାଡ଼ି_Breaks, StringRef Str) {
  ଯଦି (ChompingIndicator == '-') // Strip all line breaks.
    ଫେରନ୍ତୁ 0;
  ଯଦି (ChompingIndicator == '+') // Keep all line breaks.
    ଫେରନ୍ତୁ ଧାଡ଼ି_Breaks;
  // Clip trailing lines.
  ଫେରନ୍ତୁ Str.empty() ? 0 : 1;
}

unsigned Scanner::scanBlockIndentationIndicator() {
  unsigned Indent = 0;
  ଯଦି (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = unsigned(*Current - '0');
    skip(1);
  }
  ଫେରନ୍ତୁ Indent;
}

ତାର୍କିକ Scanner::scanBlockScalarHeader(ଅକ୍ଷର &ChompingIndicator,
                                    unsigned &IndentIndicator, ତାର୍କିକ &IsDone) {
  auto Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  ଯଦି (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  ଯଦି (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ଠେଲ_back(T);
    IsDone = ସତ୍ୟ;
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  ଯଦି (!consume_ଧାଡ଼ି_Break_If_Present()) {
    setError("Expected a line break after block scalar header", Current);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::findBlockScalarIndent(unsigned &BlockIndent,
                                    unsigned BlockExitIndent,
                                    unsigned &ଧାଡ଼ି_Breaks, ତାର୍କିକ &IsDone) {
  unsigned Max_All_Space_ଧାଡ଼ି_Characters = 0;
  StringRef::iterator Longest_All_Space_ଧାଡ଼ି;

  ଯେତେବେଳେ (ସତ୍ୟ) {
    advanceWhile(&Scanner::skip_s_space);
    ଯଦି (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      ଯଦି (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = ସତ୍ୟ;
        ଫେରନ୍ତୁ ସତ୍ୟ;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      ଯଦି (Max_All_Space_ଧାଡ଼ି_Characters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_ଧାଡ଼ି);
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      }
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }
    ଯଦି (skip_b_break(Current) != Current &&
        Column > Max_All_Space_ଧାଡ଼ି_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_ଧାଡ଼ି_Characters = Column;
      Longest_All_Space_ଧାଡ଼ି = Current;
    }

    // Check for EOF.
    ଯଦି (Current == End) {
      IsDone = ସତ୍ୟ;
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }

    ଯଦି (!consume_ଧାଡ଼ି_Break_If_Present()) {
      IsDone = ସତ୍ୟ;
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }
    ++ଧାଡ଼ି_Breaks;
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::scanBlockScalarIndent(unsigned BlockIndent,
                                    unsigned BlockExitIndent, ତାର୍କିକ &IsDone) {
  // Skip the indentation.
  ଯେତେବେଳେ (Column < BlockIndent) {
    auto I = skip_s_space(Current);
    ଯଦି (I == Current)
      ଭାଙ୍ଗ;
    Current = I;
    ++Column;
  }

  ଯଦି (skip_nb_char(Current) == Current)
    ଫେରନ୍ତୁ ସତ୍ୟ;

  ଯଦି (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = ସତ୍ୟ;
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  ଯଦି (Column < BlockIndent) {
    ଯଦି (Current != End && *Current == '#') { // Trailing comment.
      IsDone = ସତ୍ୟ;
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }
    setError("A text line is less indented than the block scalar", Current);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  ଫେରନ୍ତୁ ସତ୍ୟ; // A normal text line.
}

ତାର୍କିକ Scanner::scanBlockScalar(ତାର୍କିକ IsLiteral) {
  ପ୍ରତିଜ୍ଞା(*Current == '|' || *Current == '>');
  ଅକ୍ଷର StyleIndicator;
  ଅକ୍ଷର ChompingIndicator;
  unsigned BlockIndent;
  ତାର୍କିକ IsDone = ମିଥ୍ୟା;
  ଯଦି (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (IsDone)
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ତାର୍କିକ IsFolded = StyleIndicator == '>';

  ଅଚଳ auto *Start = Current;
  unsigned BlockExitIndent = Indent < 0 ? 0 : (unsigned)Indent;
  unsigned ଧାଡ଼ି_Breaks = 0;
  ଯଦି (BlockIndent == 0) {
    ଯଦି (!findBlockScalarIndent(BlockIndent, BlockExitIndent, ଧାଡ଼ି_Breaks,
                               IsDone))
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  ଯେତେବେଳେ (!IsDone) {
    ଯଦି (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    ଯଦି (IsDone)
      ଭାଙ୍ଗ;

    // Parse the current line.
    auto ଧାଡ଼ି_Start = Current;
    advanceWhile(&Scanner::skip_nb_char);
    ଯଦି (ଧାଡ଼ି_Start != Current) {
      ଯଦି (ଧାଡ଼ି_Breaks && IsFolded && !Scanner::is_ଧାଡ଼ି_Empty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        ଯଦି (ଧାଡ଼ି_Breaks == 1) {
          Str.append(ଧାଡ଼ି_Breaks,
                     is_ଧାଡ଼ି_Empty(StringRef(ଧାଡ଼ି_Start, Current - ଧାଡ଼ି_Start))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        ଧାଡ଼ି_Breaks--;
      }
      Str.append(ଧାଡ଼ି_Breaks, '\n');
      Str.append(StringRef(ଧାଡ଼ି_Start, Current - ଧାଡ଼ି_Start));
      ଧାଡ଼ି_Breaks = 0;
    }

    // Check for EOF.
    ଯଦି (Current == End)
      ଭାଙ୍ଗ;

    ଯଦି (!consume_ଧାଡ଼ି_Break_If_Present())
      ଭାଙ୍ଗ;
    ++ଧାଡ଼ି_Breaks;
  }

  ଯଦି (Current == End && !ଧାଡ଼ି_Breaks)
    // Ensure that there is at least one line break before the end of file.
    ଧାଡ଼ି_Breaks = 1;
  Str.append(get_Chomped_ଧାଡ଼ି_Breaks(ChompingIndicator, ଧାଡ଼ି_Breaks, Str), '\n');

  // New lines may start a simple key.
  ଯଦି (!FlowLevel)
    Is_Simple_ଚାବି_Allowed = ସତ୍ୟ;
  Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow = ମିଥ୍ୟା;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.ମୂଲ୍ୟ = std::string(Str);
  TokenQueue.ଠେଲ_back(T);
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::scanTag() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1); // Eat !.
  ଯଦି (Current == End || isBlankOrBreak(Current)); // An empty tag.
  ଅନ୍ୟଥା ଯଦି (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    ଯଦି (!consume('>'))
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
  } ଅନ୍ୟଥା {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.Kind = Token::TK_Tag;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ଠେଲ_back(T);

  // Tags can be simple keys.
  save_Simple_ଚାବି_Candidate(--TokenQueue.end(), ColStart, ମିଥ୍ୟା);

  Is_Simple_ଚାବି_Allowed = ମିଥ୍ୟା;
  Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow = ମିଥ୍ୟା;

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

ତାର୍କିକ Scanner::fetchMoreTokens() {
  ଯଦି (IsStartOfStream)
    ଫେରନ୍ତୁ scanStreamStart();

  scanToNextToken();

  ଯଦି (Current == End)
    ଫେରନ୍ତୁ scanStreamEnd();

  remove_Stale_Simple_ଚାବି_Candidates();

  unrollIndent(Column);

  ଯଦି (Column == 0 && *Current == '%')
    ଫେରନ୍ତୁ scanDirective();

  ଯଦି (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    ଫେରନ୍ତୁ scanDocumentIndicator(ସତ୍ୟ);

  ଯଦି (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    ଫେରନ୍ତୁ scanDocumentIndicator(ମିଥ୍ୟା);

  ଯଦି (*Current == '[')
    ଫେରନ୍ତୁ scanFlowCollectionStart(ସତ୍ୟ);

  ଯଦି (*Current == '{')
    ଫେରନ୍ତୁ scanFlowCollectionStart(ମିଥ୍ୟା);

  ଯଦି (*Current == ']')
    ଫେରନ୍ତୁ scanFlowCollectionEnd(ସତ୍ୟ);

  ଯଦି (*Current == '}')
    ଫେରନ୍ତୁ scanFlowCollectionEnd(ମିଥ୍ୟା);

  ଯଦି (*Current == ',')
    ଫେରନ୍ତୁ scanFlowEntry();

  ଯଦି (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    ଫେରନ୍ତୁ scanBlockEntry();

  ଯଦି (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    ଫେରନ୍ତୁ scan_ଚାବି();

  ଯଦି (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || Is_Adjacent_ମୂଲ୍ୟ_Allowed_In_Flow))
    ଫେରନ୍ତୁ scan_ମୂଲ୍ୟ();

  ଯଦି (*Current == '*')
    ଫେରନ୍ତୁ scanAliasOrAnchor(ସତ୍ୟ);

  ଯଦି (*Current == '&')
    ଫେରନ୍ତୁ scanAliasOrAnchor(ମିଥ୍ୟା);

  ଯଦି (*Current == '!')
    ଫେରନ୍ତୁ scanTag();

  ଯଦି (*Current == '|' && !FlowLevel)
    ଫେରନ୍ତୁ scanBlockScalar(ସତ୍ୟ);

  ଯଦି (*Current == '>' && !FlowLevel)
    ଫେରନ୍ତୁ scanBlockScalar(ମିଥ୍ୟା);

  ଯଦି (*Current == '\'')
    ଫେରନ୍ତୁ scanFlowScalar(ମିଥ୍ୟା);

  ଯଦି (*Current == '"')
    ଫେରନ୍ତୁ scanFlowScalar(ସତ୍ୟ);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  ଯଦି ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    ଫେରନ୍ତୁ scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

Stream::Stream(StringRef ନିବେଶ, SourceMgr &SM, ତାର୍କିକ ShowColors,
               std::error_code *EC)
    : scanner(ନୂତନ Scanner(ନିବେଶ, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef ନିବେଶ_Buffer, SourceMgr &SM, ତାର୍କିକ ShowColors,
               std::error_code *EC)
    : scanner(ନୂତନ Scanner(ନିବେଶ_Buffer, SM, ShowColors, EC)) {}

Stream::~Stream() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

ତାର୍କିକ Stream::failed() { ଫେରନ୍ତୁ scanner->failed(); }

ଶୂନ୍ୟ Stream::printError(ଗଣ୍ଠି *N, ଅଚଳ Twine &ସନ୍ଦେଶ_2, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), ସନ୍ଦେଶ_2, Kind);
}

ଶୂନ୍ୟ Stream::printError(ଅଚଳ SMRange &Range, ଅଚଳ Twine &ସନ୍ଦେଶ_2,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, ସନ୍ଦେଶ_2, Range);
}

document_iterator Stream::begin() {
  ଯଦି (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(ନୂତନ Document(*ଏହି));
  ଫେରନ୍ତୁ document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  ଫେରନ୍ତୁ document_iterator();
}

ଶୂନ୍ୟ Stream::skip() {
  ପାଇଁ (Document &Doc : *ଏହି)
    Doc.skip();
}

ଗଣ୍ଠି::ଗଣ୍ଠି(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string ଗଣ୍ଠି::getVerbatimTag() ଅଚଳ {
  StringRef Raw = getRawTag();
  ଯଦି (!Raw.empty() && Raw != "!") {
    std::string Ret;
    ଯଦି (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      ଫେରନ୍ତୁ Ret;
    } ଅନ୍ୟଥା ଯଦି (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      ଫେରନ୍ତୁ Ret;
    } ଅନ୍ୟଥା {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      ଯଦି (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      ଅନ୍ୟଥା {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      ଫେରନ୍ତୁ Ret;
    }
  }

  ବାଛ (getType()) {
  ସ୍ଥିତି NK_Null:
    ଫେରନ୍ତୁ "tag:yaml.org,2002:null";
  ସ୍ଥିତି NK_Scalar:
  ସ୍ଥିତି NK_BlockScalar:
    // TODO: Tag resolution.
    ଫେରନ୍ତୁ "tag:yaml.org,2002:str";
  ସ୍ଥିତି NK_Mapping:
    ଫେରନ୍ତୁ "tag:yaml.org,2002:map";
  ସ୍ଥିତି NK_Sequence:
    ଫେରନ୍ତୁ "tag:yaml.org,2002:seq";
  }

  ଫେରନ୍ତୁ "";
}

Token &ଗଣ୍ଠି::peekNext() {
  ଫେରନ୍ତୁ Doc->peekNext();
}

Token ଗଣ୍ଠି::getNext() {
  ଫେରନ୍ତୁ Doc->getNext();
}

ଗଣ୍ଠି *ଗଣ୍ଠି::parse_Block_ଗଣ୍ଠି() {
  ଫେରନ୍ତୁ Doc->parse_Block_ଗଣ୍ଠି();
}

BumpPtrAllocator &ଗଣ୍ଠି::getAllocator() {
  ଫେରନ୍ତୁ Doc->ଗଣ୍ଠି_Allocator;
}

ଶୂନ୍ୟ ଗଣ୍ଠି::setError(ଅଚଳ Twine &ସନ୍ଦେଶ_2, Token &Tok) ଅଚଳ {
  Doc->setError(ସନ୍ଦେଶ_2, Tok);
}

ତାର୍କିକ ଗଣ୍ଠି::failed() ଅଚଳ {
  ଫେରନ୍ତୁ Doc->failed();
}

StringRef Scalar_ଗଣ୍ଠି::get_ମୂଲ୍ୟ(SmallVectorImpl<ଅକ୍ଷର> &Storage) ଅଚଳ {
  ଯଦି (ମୂଲ୍ୟ[0] == '"')
    ଫେରନ୍ତୁ get_Double_Quoted_ମୂଲ୍ୟ(ମୂଲ୍ୟ, Storage);
  ଯଦି (ମୂଲ୍ୟ[0] == '\'')
    ଫେରନ୍ତୁ get_Single_Quoted_ମୂଲ୍ୟ(ମୂଲ୍ୟ, Storage);
  ଫେରନ୍ତୁ get_Plain_ମୂଲ୍ୟ(ମୂଲ୍ୟ, Storage);
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
ସ୍ଥିର StringRef
parse_Scalar_ମୂଲ୍ୟ(StringRef Unquoted_ମୂଲ୍ୟ, SmallVectorImpl<ଅକ୍ଷର> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<ଅକ୍ଷର> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_ମୂଲ୍ୟ.find_first_of(LookupChars);
  ଯଦି (I == StringRef::npos)
    ଫେରନ୍ତୁ Unquoted_ମୂଲ୍ୟ;

  Storage.clear();
  Storage.reserve(Unquoted_ମୂଲ୍ୟ.size());
  ଅକ୍ଷର Last_New_ଧାଡ଼ି_Added_As = '\0';
  ପାଇଁ (; I != StringRef::npos; I = Unquoted_ମୂଲ୍ୟ.find_first_of(LookupChars)) {
    ଯଦି (Unquoted_ମୂଲ୍ୟ[I] != '\r' && Unquoted_ମୂଲ୍ୟ[I] != '\n') {
      llvm::append_range(Storage, Unquoted_ମୂଲ୍ୟ.take_front(I));
      Unquoted_ମୂଲ୍ୟ = UnescapeCallback(Unquoted_ମୂଲ୍ୟ.drop_front(I), Storage);
      Last_New_ଧାଡ଼ି_Added_As = '\0';
      ଜାରି;
    }
    ଯଦି (size_t LastNonSWhite = Unquoted_ମୂଲ୍ୟ.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_ମୂଲ୍ୟ.take_front(LastNonSWhite + 1));
      Storage.ଠେଲ_back(' ');
      Last_New_ଧାଡ଼ି_Added_As = ' ';
    } ଅନ୍ୟଥା {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      ବାଛ (Last_New_ଧାଡ଼ି_Added_As) {
      ସ୍ଥିତି ' ':
        ପ୍ରତିଜ୍ଞା(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        Last_New_ଧାଡ଼ି_Added_As = '\n';
        ଭାଙ୍ଗ;
      ସ୍ଥିତି '\n':
        ପ୍ରତିଜ୍ଞା(!Storage.empty() && Storage.back() == '\n');
        Storage.ଠେଲ_back('\n');
        ଭାଙ୍ଗ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        Storage.ଠେଲ_back(' ');
        Last_New_ଧାଡ଼ି_Added_As = ' ';
        ଭାଙ୍ଗ;
      }
    }
    // Handle Windows-style EOL
    ଯଦି (Unquoted_ମୂଲ୍ୟ.substr(I, 2) == "\r\n")
      I++;
    Unquoted_ମୂଲ୍ୟ = Unquoted_ମୂଲ୍ୟ.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_ମୂଲ୍ୟ);
  ଫେରନ୍ତୁ StringRef(Storage.begin(), Storage.size());
}

StringRef
Scalar_ଗଣ୍ଠି::get_Double_Quoted_ମୂଲ୍ୟ(StringRef Raw_ମୂଲ୍ୟ,
                                 SmallVectorImpl<ଅକ୍ଷର> &Storage) ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(Raw_ମୂଲ୍ୟ.size() >= 2 && Raw_ମୂଲ୍ୟ.front() == '"' &&
         Raw_ମୂଲ୍ୟ.back() == '"');
  StringRef Unquoted_ମୂଲ୍ୟ = Raw_ମୂଲ୍ୟ.substr(1, Raw_ମୂଲ୍ୟ.size() - 2);

  auto UnescapeFunc = [ଏହି](StringRef Unquoted_ମୂଲ୍ୟ,
                             SmallVectorImpl<ଅକ୍ଷର> &Storage) {
    ପ୍ରତିଜ୍ଞା(Unquoted_ମୂଲ୍ୟ.take_front(1) == "\\");
    ଯଦି (Unquoted_ମୂଲ୍ୟ.size() == 1) {
      Token T;
      T.Range = Unquoted_ମୂଲ୍ୟ;
      setError("Unrecognized escape code", T);
      Storage.clear();
      ଫେରନ୍ତୁ StringRef();
    }
    Unquoted_ମୂଲ୍ୟ = Unquoted_ମୂଲ୍ୟ.drop_front(1);
    ବାଛ (Unquoted_ମୂଲ୍ୟ[0]) {
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      Token T;
      T.Range = Unquoted_ମୂଲ୍ୟ.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      ଫେରନ୍ତୁ StringRef();
    }
    ସ୍ଥିତି '\r':
      // Shrink the Windows-style EOL.
      ଯଦି (Unquoted_ମୂଲ୍ୟ.size() >= 2 && Unquoted_ମୂଲ୍ୟ[1] == '\n')
        Unquoted_ମୂଲ୍ୟ = Unquoted_ମୂଲ୍ୟ.drop_front(1);
      [[fallthrough]];
    ସ୍ଥିତି '\n':
      ଫେରନ୍ତୁ Unquoted_ମୂଲ୍ୟ.drop_front(1).ltrim(" \t");
    ସ୍ଥିତି '0':
      Storage.ଠେଲ_back(0x00);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'a':
      Storage.ଠେଲ_back(0x07);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'b':
      Storage.ଠେଲ_back(0x08);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 't':
    ସ୍ଥିତି 0x09:
      Storage.ଠେଲ_back(0x09);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'n':
      Storage.ଠେଲ_back(0x0A);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'v':
      Storage.ଠେଲ_back(0x0B);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'f':
      Storage.ଠେଲ_back(0x0C);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'r':
      Storage.ଠେଲ_back(0x0D);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'e':
      Storage.ଠେଲ_back(0x1B);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି ' ':
      Storage.ଠେଲ_back(0x20);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି '"':
      Storage.ଠେଲ_back(0x22);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି '/':
      Storage.ଠେଲ_back(0x2F);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି '\\':
      Storage.ଠେଲ_back(0x5C);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'N':
      encodeUTF8(0x85, Storage);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି '_':
      encodeUTF8(0xA0, Storage);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'L':
      encodeUTF8(0x2028, Storage);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'P':
      encodeUTF8(0x2029, Storage);
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 'x': {
      ଯଦି (Unquoted_ମୂଲ୍ୟ.size() < 3)
        // TODO: Report error.
        ଭାଙ୍ଗ;
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା Unicode_Scalar_ମୂଲ୍ୟ;
      ଯଦି (Unquoted_ମୂଲ୍ୟ.substr(1, 2).getAsInteger(16, Unicode_Scalar_ମୂଲ୍ୟ))
        // TODO: Report error.
        Unicode_Scalar_ମୂଲ୍ୟ = 0xFFFD;
      encodeUTF8(Unicode_Scalar_ମୂଲ୍ୟ, Storage);
      ଫେରନ୍ତୁ Unquoted_ମୂଲ୍ୟ.drop_front(3);
    }
    ସ୍ଥିତି 'u': {
      ଯଦି (Unquoted_ମୂଲ୍ୟ.size() < 5)
        // TODO: Report error.
        ଭାଙ୍ଗ;
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା Unicode_Scalar_ମୂଲ୍ୟ;
      ଯଦି (Unquoted_ମୂଲ୍ୟ.substr(1, 4).getAsInteger(16, Unicode_Scalar_ମୂଲ୍ୟ))
        // TODO: Report error.
        Unicode_Scalar_ମୂଲ୍ୟ = 0xFFFD;
      encodeUTF8(Unicode_Scalar_ମୂଲ୍ୟ, Storage);
      ଫେରନ୍ତୁ Unquoted_ମୂଲ୍ୟ.drop_front(5);
    }
    ସ୍ଥିତି 'U': {
      ଯଦି (Unquoted_ମୂଲ୍ୟ.size() < 9)
        // TODO: Report error.
        ଭାଙ୍ଗ;
      unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା Unicode_Scalar_ମୂଲ୍ୟ;
      ଯଦି (Unquoted_ମୂଲ୍ୟ.substr(1, 8).getAsInteger(16, Unicode_Scalar_ମୂଲ୍ୟ))
        // TODO: Report error.
        Unicode_Scalar_ମୂଲ୍ୟ = 0xFFFD;
      encodeUTF8(Unicode_Scalar_ମୂଲ୍ୟ, Storage);
      ଫେରନ୍ତୁ Unquoted_ମୂଲ୍ୟ.drop_front(9);
    }
    }
    ଫେରନ୍ତୁ Unquoted_ମୂଲ୍ୟ.drop_front(1);
  };

  ଫେରନ୍ତୁ parse_Scalar_ମୂଲ୍ୟ(Unquoted_ମୂଲ୍ୟ, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_ଗଣ୍ଠି::get_Single_Quoted_ମୂଲ୍ୟ(StringRef Raw_ମୂଲ୍ୟ,
                                           SmallVectorImpl<ଅକ୍ଷର> &Storage) {
  ପ୍ରତିଜ୍ଞା(Raw_ମୂଲ୍ୟ.size() >= 2 && Raw_ମୂଲ୍ୟ.front() == '\'' &&
         Raw_ମୂଲ୍ୟ.back() == '\'');
  StringRef Unquoted_ମୂଲ୍ୟ = Raw_ମୂଲ୍ୟ.substr(1, Raw_ମୂଲ୍ୟ.size() - 2);

  auto UnescapeFunc = [](StringRef Unquoted_ମୂଲ୍ୟ,
                         SmallVectorImpl<ଅକ୍ଷର> &Storage) {
    ପ୍ରତିଜ୍ଞା(Unquoted_ମୂଲ୍ୟ.take_front(2) == "''");
    Storage.ଠେଲ_back('\'');
    ଫେରନ୍ତୁ Unquoted_ମୂଲ୍ୟ.drop_front(2);
  };

  ଫେରନ୍ତୁ parse_Scalar_ମୂଲ୍ୟ(Unquoted_ମୂଲ୍ୟ, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_ଗଣ୍ଠି::get_Plain_ମୂଲ୍ୟ(StringRef Raw_ମୂଲ୍ୟ,
                                    SmallVectorImpl<ଅକ୍ଷର> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_ମୂଲ୍ୟ = Raw_ମୂଲ୍ୟ.rtrim("\r\n \t");
  ଫେରନ୍ତୁ parse_Scalar_ମୂଲ୍ୟ(Raw_ମୂଲ୍ୟ, Storage, "\r\n", nullptr);
}

ଗଣ୍ଠି *ଚାବି_ମୂଲ୍ୟ_ଗଣ୍ଠି::get_ଚାବି() {
  ଯଦି (ଚାବି)
    ଫେରନ୍ତୁ ଚାବି;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    ଯଦି (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_ମୂଲ୍ୟ
        || t.Kind == Token::TK_Error) {
      ଫେରନ୍ତୁ ଚାବି = ନୂତନ (getAllocator()) Null_ଗଣ୍ଠି(Doc);
    }
    ଯଦି (t.Kind == Token::TK_ଚାବି)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  ଯଦି (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_ମୂଲ୍ୟ) {
    ଫେରନ୍ତୁ ଚାବି = ନୂତନ (getAllocator()) Null_ଗଣ୍ଠି(Doc);
  }

  // We've got a normal key.
  ଫେରନ୍ତୁ ଚାବି = parse_Block_ଗଣ୍ଠି();
}

ଗଣ୍ଠି *ଚାବି_ମୂଲ୍ୟ_ଗଣ୍ଠି::get_ମୂଲ୍ୟ() {
  ଯଦି (ମୂଲ୍ୟ)
    ଫେରନ୍ତୁ ମୂଲ୍ୟ;

  ଯଦି (ଗଣ୍ଠି* ଚାବି = get_ଚାବି())
    ଚାବି->skip();
  ଅନ୍ୟଥା {
    setError("Null key in Key Value.", peekNext());
    ଫେରନ୍ତୁ ମୂଲ୍ୟ = ନୂତନ (getAllocator()) Null_ଗଣ୍ଠି(Doc);
  }

  ଯଦି (failed())
    ଫେରନ୍ତୁ ମୂଲ୍ୟ = ନୂତନ (getAllocator()) Null_ଗଣ୍ଠି(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    ଯଦି (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_ଚାବି
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      ଫେରନ୍ତୁ ମୂଲ୍ୟ = ନୂତନ (getAllocator()) Null_ଗଣ୍ଠି(Doc);
    }

    ଯଦି (t.Kind != Token::TK_ମୂଲ୍ୟ) {
      setError("Unexpected token in Key Value.", t);
      ଫେରନ୍ତୁ ମୂଲ୍ୟ = ନୂତନ (getAllocator()) Null_ଗଣ୍ଠି(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  ଯଦି (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_ଚାବି) {
    ଫେରନ୍ତୁ ମୂଲ୍ୟ = ନୂତନ (getAllocator()) Null_ଗଣ୍ଠି(Doc);
  }

  // We got a normal value.
  ଫେରନ୍ତୁ ମୂଲ୍ୟ = parse_Block_ଗଣ୍ଠି();
}

ଶୂନ୍ୟ Mapping_ଗଣ୍ଠି::increment() {
  ଯଦି (failed()) {
    IsAtEnd = ସତ୍ୟ;
    CurrentEntry = nullptr;
    ଫେରନ୍ତୁ;
  }
  ଯଦି (CurrentEntry) {
    CurrentEntry->skip();
    ଯଦି (Type == MT_Inline) {
      IsAtEnd = ସତ୍ୟ;
      CurrentEntry = nullptr;
      ଫେରନ୍ତୁ;
    }
  }
  Token T = peekNext();
  ଯଦି (T.Kind == Token::TK_ଚାବି || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = ନୂତନ (getAllocator()) ଚାବି_ମୂଲ୍ୟ_ଗଣ୍ଠି(Doc);
  } ଅନ୍ୟଥା ଯଦି (Type == MT_Block) {
    ବାଛ (T.Kind) {
    ସ୍ଥିତି Token::TK_BlockEnd:
      getNext();
      IsAtEnd = ସତ୍ୟ;
      CurrentEntry = nullptr;
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      setError("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    ସ୍ଥିତି Token::TK_Error:
      IsAtEnd = ସତ୍ୟ;
      CurrentEntry = nullptr;
    }
  } ଅନ୍ୟଥା {
    ବାଛ (T.Kind) {
    ସ୍ଥିତି Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      ଫେରନ୍ତୁ increment();
    ସ୍ଥିତି Token::TK_FlowMappingEnd:
      getNext();
      [[fallthrough]];
    ସ୍ଥିତି Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = ସତ୍ୟ;
      CurrentEntry = nullptr;
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      setError( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      IsAtEnd = ସତ୍ୟ;
      CurrentEntry = nullptr;
    }
  }
}

ଶୂନ୍ୟ Sequence_ଗଣ୍ଠି::increment() {
  ଯଦି (failed()) {
    IsAtEnd = ସତ୍ୟ;
    CurrentEntry = nullptr;
    ଫେରନ୍ତୁ;
  }
  ଯଦି (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  ଯଦି (SeqType == ST_Block) {
    ବାଛ (T.Kind) {
    ସ୍ଥିତି Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_ଗଣ୍ଠି();
      ଯଦି (!CurrentEntry) { // An error occurred.
        IsAtEnd = ସତ୍ୟ;
        CurrentEntry = nullptr;
      }
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_BlockEnd:
      getNext();
      IsAtEnd = ସତ୍ୟ;
      CurrentEntry = nullptr;
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      setError( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    ସ୍ଥିତି Token::TK_Error:
      IsAtEnd = ସତ୍ୟ;
      CurrentEntry = nullptr;
    }
  } ଅନ୍ୟଥା ଯଦି (SeqType == ST_Indentless) {
    ବାଛ (T.Kind) {
    ସ୍ଥିତି Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_ଗଣ୍ଠି();
      ଯଦି (!CurrentEntry) { // An error occurred.
        IsAtEnd = ସତ୍ୟ;
        CurrentEntry = nullptr;
      }
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ସ୍ଥିତି Token::TK_Error:
      IsAtEnd = ସତ୍ୟ;
      CurrentEntry = nullptr;
    }
  } ଅନ୍ୟଥା ଯଦି (SeqType == ST_Flow) {
    ବାଛ (T.Kind) {
    ସ୍ଥିତି Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      WasPreviousTokenFlowEntry = ସତ୍ୟ;
      ଫେରନ୍ତୁ increment();
    ସ୍ଥିତି Token::TK_FlowSequenceEnd:
      getNext();
      [[fallthrough]];
    ସ୍ଥିତି Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = ସତ୍ୟ;
      CurrentEntry = nullptr;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି Token::TK_StreamEnd:
    ସ୍ଥିତି Token::TK_DocumentEnd:
    ସ୍ଥିତି Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = ସତ୍ୟ;
      CurrentEntry = nullptr;
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଯଦି (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = ସତ୍ୟ;
        CurrentEntry = nullptr;
        ଭାଙ୍ଗ;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parse_Block_ଗଣ୍ଠି();
      ଯଦି (!CurrentEntry) {
        IsAtEnd = ସତ୍ୟ;
      }
      WasPreviousTokenFlowEntry = ମିଥ୍ୟା;
      ଭାଙ୍ଗ;
    }
  }
}

Document::Document(Stream &S) : stream(S), ମୂଳ_2(nullptr) {
  // Tag maps starts with two default mappings.
  TagMap["!"] = "!";
  TagMap["!!"] = "tag:yaml.org,2002:";

  ଯଦି (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peekNext();
  ଯଦି (T.Kind == Token::TK_DocumentStart)
    getNext();
}

ତାର୍କିକ Document::skip()  {
  ଯଦି (stream.scanner->failed())
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (!ମୂଳ_2 && !get_ମୂଳ())
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ମୂଳ_2->skip();
  Token &T = peekNext();
  ଯଦି (T.Kind == Token::TK_StreamEnd)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଯଦି (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    ଫେରନ୍ତୁ skip();
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

Token &Document::peekNext() {
  ଫେରନ୍ତୁ stream.scanner->peekNext();
}

Token Document::getNext() {
  ଫେରନ୍ତୁ stream.scanner->getNext();
}

ଶୂନ୍ୟ Document::setError(ଅଚଳ Twine &Message, Token &Location) ଅଚଳ {
  stream.scanner->setError(Message, Location.Range.begin());
}

ତାର୍କିକ Document::failed() ଅଚଳ {
  ଫେରନ୍ତୁ stream.scanner->failed();
}

ଗଣ୍ଠି *Document::parse_Block_ଗଣ୍ଠି() {
  Token T = peekNext();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  ବାଛ (T.Kind) {
  ସ୍ଥିତି Token::TK_Alias:
    getNext();
    ଫେରନ୍ତୁ ନୂତନ (ଗଣ୍ଠି_Allocator) Alias_ଗଣ୍ଠି(stream.CurrentDoc, T.Range.substr(1));
  ସ୍ଥିତି Token::TK_Anchor:
    ଯଦି (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      ଫେରନ୍ତୁ nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    ଯାଅ parse_property;
  ସ୍ଥିତି Token::TK_Tag:
    ଯଦି (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      ଫେରନ୍ତୁ nullptr;
    }
    TagInfo = getNext(); // Consume TK_Tag.
    T = peekNext();
    ଯାଅ parse_property;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଭାଙ୍ଗ;
  }

  ବାଛ (T.Kind) {
  ସ୍ଥିତି Token::TK_BlockEntry:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    ଫେରନ୍ତୁ ନୂତନ (ଗଣ୍ଠି_Allocator) Sequence_ଗଣ୍ଠି( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , Sequence_ଗଣ୍ଠି::ST_Indentless);
  ସ୍ଥିତି Token::TK_BlockSequenceStart:
    getNext();
    ଫେରନ୍ତୁ ନୂତନ (ଗଣ୍ଠି_Allocator)
      Sequence_ଗଣ୍ଠି( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_ଗଣ୍ଠି::ST_Block);
  ସ୍ଥିତି Token::TK_BlockMappingStart:
    getNext();
    ଫେରନ୍ତୁ ନୂତନ (ଗଣ୍ଠି_Allocator)
      Mapping_ଗଣ୍ଠି( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_ଗଣ୍ଠି::MT_Block);
  ସ୍ଥିତି Token::TK_FlowSequenceStart:
    getNext();
    ଫେରନ୍ତୁ ନୂତନ (ଗଣ୍ଠି_Allocator)
      Sequence_ଗଣ୍ଠି( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_ଗଣ୍ଠି::ST_Flow);
  ସ୍ଥିତି Token::TK_FlowMappingStart:
    getNext();
    ଫେରନ୍ତୁ ନୂତନ (ଗଣ୍ଠି_Allocator)
      Mapping_ଗଣ୍ଠି( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_ଗଣ୍ଠି::MT_Flow);
  ସ୍ଥିତି Token::TK_Scalar:
    getNext();
    ଫେରନ୍ତୁ ନୂତନ (ଗଣ୍ଠି_Allocator)
      Scalar_ଗଣ୍ଠି( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  ସ୍ଥିତି Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.ମୂଲ୍ୟ.c_str(), T.ମୂଲ୍ୟ.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(ଗଣ୍ଠି_Allocator).drop_back();
    ଫେରନ୍ତୁ ନୂତନ (ଗଣ୍ଠି_Allocator)
        Block_Scalar_ଗଣ୍ଠି(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  ସ୍ଥିତି Token::TK_ଚାବି:
    // Don't eat the TK_Key, KeyValueNode expects it.
    ଫେରନ୍ତୁ ନୂତନ (ଗଣ୍ଠି_Allocator)
      Mapping_ଗଣ୍ଠି( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_ଗଣ୍ଠି::MT_Inline);
  ସ୍ଥିତି Token::TK_DocumentStart:
  ସ୍ଥିତି Token::TK_DocumentEnd:
  ସ୍ଥିତି Token::TK_StreamEnd:
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    ଫେରନ୍ତୁ ନୂତନ (ଗଣ୍ଠି_Allocator) Null_ଗଣ୍ଠି(stream.CurrentDoc);
  ସ୍ଥିତି Token::TK_FlowMappingEnd:
  ସ୍ଥିତି Token::TK_FlowSequenceEnd:
  ସ୍ଥିତି Token::TK_FlowEntry: {
    ଯଦି (ମୂଳ_2 && (isa<Mapping_ଗଣ୍ଠି>(ମୂଳ_2) || isa<Sequence_ଗଣ୍ଠି>(ମୂଳ_2)))
      ଫେରନ୍ତୁ ନୂତନ (ଗଣ୍ଠି_Allocator) Null_ଗଣ୍ଠି(stream.CurrentDoc);

    setError("Unexpected token", T);
    ଫେରନ୍ତୁ nullptr;
  }
  ସ୍ଥିତି Token::TK_Error:
    ଫେରନ୍ତୁ nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  ଫେରନ୍ତୁ nullptr;
}

ତାର୍କିକ Document::parseDirectives() {
  ତାର୍କିକ isDirective = ମିଥ୍ୟା;
  ଯେତେବେଳେ (ସତ୍ୟ) {
    Token T = peekNext();
    ଯଦି (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = ସତ୍ୟ;
    } ଅନ୍ୟଥା ଯଦି (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = ସତ୍ୟ;
    } ଅନ୍ୟଥା {
      ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ isDirective;
}

ଶୂନ୍ୟ Document::parseYAMLDirective() {
  getNext(); // Eat %YAML <version>
}

ଶୂନ୍ୟ Document::parseTAGDirective() {
  Token Tag = getNext(); // %TAG <handle> <prefix>
  StringRef T = Tag.Range;
  // Strip %TAG
  T = T.substr(T.find_first_of(" \t")).ltrim(" \t");
  std::size_t HandleEnd = T.find_first_of(" \t");
  StringRef TagHandle = T.substr(0, HandleEnd);
  StringRef TagPrefix = T.substr(HandleEnd).ltrim(" \t");
  TagMap[TagHandle] = TagPrefix;
}

ତାର୍କିକ Document::expectToken(ପୂର୍ଣ୍ଣସଂଖ୍ୟା TK) {
  Token T = getNext();
  ଯଦି (T.Kind != TK) {
    setError("Unexpected token", T);
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}
