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

#অন্তর্ভাৱ "llvm/Support/YAMLParser.h"
#অন্তর্ভাৱ "llvm/ADT/AllocatorList.h"
#অন্তর্ভাৱ "llvm/ADT/ArrayRef.h"
#অন্তর্ভাৱ "llvm/ADT/STLExtras.h"
#অন্তর্ভাৱ "llvm/ADT/SmallString.h"
#অন্তর্ভাৱ "llvm/ADT/SmallVector.h"
#অন্তর্ভাৱ "llvm/ADT/StringExtras.h"
#অন্তর্ভাৱ "llvm/ADT/StringRef.h"
#অন্তর্ভাৱ "llvm/ADT/Twine.h"
#অন্তর্ভাৱ "llvm/Support/Compiler.h"
#অন্তর্ভাৱ "llvm/Support/ErrorHandling.h"
#অন্তর্ভাৱ "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাৱ "llvm/Support/SMLoc.h"
#অন্তর্ভাৱ "llvm/Support/SourceMgr.h"
#অন্তর্ভাৱ "llvm/Support/Unicode.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"
#অন্তর্ভাৱ <cassert>
#অন্তর্ভাৱ <cstddef>
#অন্তর্ভাৱ <cstdint>
#অন্তর্ভাৱ <map>
#অন্তর্ভাৱ <memory>
#অন্তর্ভাৱ <string>
#অন্তর্ভাৱ <system_error>
#অন্তর্ভাৱ <utility>

using নামাকাশ llvm;
using নামাকাশ yaml;

গণনা UnicodeEncodingForm {
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
স্থির EncodingInfo getUnicodeEncoding(StringRef প্ৰবেশ) {
  যদি (প্ৰবেশ.empty())
    ঘূৰাই {UEF_Unknown, 0};

  ৱিকল্প (uint8_t(প্ৰবেশ[0])) {
  পক্ষ 0x00:
    যদি (প্ৰবেশ.size() >= 4) {
      যদি (  প্ৰবেশ[1] == 0
         && uint8_t(প্ৰবেশ[2]) == 0xFE
         && uint8_t(প্ৰবেশ[3]) == 0xFF)
        ঘূৰাই {UEF_UTF32_BE, 4};
      যদি (প্ৰবেশ[1] == 0 && প্ৰবেশ[2] == 0 && প্ৰবেশ[3] != 0)
        ঘূৰাই {UEF_UTF32_BE, 0};
    }

    যদি (প্ৰবেশ.size() >= 2 && প্ৰবেশ[1] != 0)
      ঘূৰাই {UEF_UTF16_BE, 0};
    ঘূৰাই {UEF_Unknown, 0};
  পক্ষ 0xFF:
    যদি (  প্ৰবেশ.size() >= 4
       && uint8_t(প্ৰবেশ[1]) == 0xFE
       && প্ৰবেশ[2] == 0
       && প্ৰবেশ[3] == 0)
      ঘূৰাই {UEF_UTF32_LE, 4};

    যদি (প্ৰবেশ.size() >= 2 && uint8_t(প্ৰবেশ[1]) == 0xFE)
      ঘূৰাই {UEF_UTF16_LE, 2};
    ঘূৰাই {UEF_Unknown, 0};
  পক্ষ 0xFE:
    যদি (প্ৰবেশ.size() >= 2 && uint8_t(প্ৰবেশ[1]) == 0xFF)
      ঘূৰাই {UEF_UTF16_BE, 2};
    ঘূৰাই {UEF_Unknown, 0};
  পক্ষ 0xEF:
    যদি (  প্ৰবেশ.size() >= 3
       && uint8_t(প্ৰবেশ[1]) == 0xBB
       && uint8_t(প্ৰবেশ[2]) == 0xBF)
      ঘূৰাই {UEF_UTF8, 3};
    ঘূৰাই {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  যদি (প্ৰবেশ.size() >= 4 && প্ৰবেশ[1] == 0 && প্ৰবেশ[2] == 0 && প্ৰবেশ[3] == 0)
    ঘূৰাই {UEF_UTF32_LE, 0};

  যদি (প্ৰবেশ.size() >= 2 && প্ৰবেশ[1] == 0)
    ঘূৰাই {UEF_UTF16_LE, 0};

  ঘূৰাই {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
শূন্য গ্ৰন্থি::anchor() {}
শূন্য Null_গ্ৰন্থি::anchor() {}
শূন্য Scalar_গ্ৰন্থি::anchor() {}
শূন্য Block_Scalar_গ্ৰন্থি::anchor() {}
শূন্য চাবি_মান_গ্ৰন্থি::anchor() {}
শূন্য Mapping_গ্ৰন্থি::anchor() {}
শূন্য Sequence_গ্ৰন্থি::anchor() {}
শূন্য Alias_গ্ৰন্থি::anchor() {}

নামাকাশ llvm {
নামাকাশ yaml {

/// Token - A single YAML token.
সংরচনা Token {
  গণনা TokenKind {
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
    TK_চাবি,
    TK_মান,
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
  std::string মান;

  Token() = স্ৱভাৱ;
};

} // end namespace yaml
} // end namespace llvm

using TokenQueueT = BumpPtrList<Token>;

নামাকাশ {

/// This struct is used to track simple keys.
///
/// Simple keys are handled by creating an entry in SimpleKeys for each Token
/// which could legally be the start of a simple key. When peekNext is called,
/// if the Token To be returned is referenced by a SimpleKey, we continue
/// tokenizing until that potential simple key has either been found to not be
/// a simple key (we moved on to the next line or went further than 1024 chars).
/// Or when we run into a Value, and then insert a Key token (and possibly
/// others) before the SimpleKey's Tok.
সংরচনা Simple_চাবি {
  TokenQueueT::iterator Tok;
  unsigned Column = 0;
  unsigned সাৰি = 0;
  unsigned FlowLevel = 0;
  তার্কিক IsRequired = মিছা;

  তার্কিক operator ==(ধ্রুৱ Simple_চাবি &Other) {
    ঘূৰাই Tok == Other.Tok;
  }
};

} // end anonymous namespace

/// The Unicode scalar value of a UTF-8 minimal well-formed code unit
///        subsequence and the subsequence's length in code units (uint8_t).
///        A length of 0 represents an error.
using UTF8Decoded = std::pair<uint32_t, unsigned>;

স্থির UTF8Decoded decodeUTF8(StringRef Range) {
  StringRef::iterator Position= Range.begin();
  StringRef::iterator End = Range.end();
  // 1 byte: [0x00, 0x7f]
  // Bit pattern: 0xxxxxxx
  যদি (Position < End && (*Position & 0x80) == 0) {
    ঘূৰাই {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  যদি (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    যদি (codepoint >= 0x80)
      ঘূৰাই {codepoint, 2};
  }
  // 3 bytes: [0x8000, 0xffff]
  // Bit pattern: 1110xxxx 10xxxxxx 10xxxxxx
  যদি (Position + 2 < End && ((*Position & 0xF0) == 0xE0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x0F) << 12) |
                         ((*(Position + 1) & 0x3F) << 6) |
                          (*(Position + 2) & 0x3F);
    // Codepoints between 0xD800 and 0xDFFF are invalid, as
    // they are high / low surrogate halves used by UTF-16.
    যদি (codepoint >= 0x800 &&
        (codepoint < 0xD800 || codepoint > 0xDFFF))
      ঘূৰাই {codepoint, 3};
  }
  // 4 bytes: [0x10000, 0x10FFFF]
  // Bit pattern: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
  যদি (Position + 3 < End && ((*Position & 0xF8) == 0xF0) &&
      ((*(Position + 1) & 0xC0) == 0x80) &&
      ((*(Position + 2) & 0xC0) == 0x80) &&
      ((*(Position + 3) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x07) << 18) |
                         ((*(Position + 1) & 0x3F) << 12) |
                         ((*(Position + 2) & 0x3F) << 6) |
                          (*(Position + 3) & 0x3F);
    যদি (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
      ঘূৰাই {codepoint, 4};
  }
  ঘূৰাই {0, 0};
}

নামাকাশ llvm {
নামাকাশ yaml {

/// Scans YAML tokens from a MemoryBuffer.
শ্ৰেণী Scanner {
সার্ৱজনিক:
  Scanner(StringRef প্ৰবেশ, SourceMgr &SM, তার্কিক ShowColors = সঁচা,
          std::error_code *EC = nullptr);
  Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, তার্কিক ShowColors = সঁচা,
          std::error_code *EC = nullptr);

  /// Parse the next token and return it without popping it.
  Token &peekNext();

  /// Parse the next token and pop it from the queue.
  Token getNext();

  শূন্য printError(SMLoc Loc, SourceMgr::DiagKind Kind, ধ্রুৱ Twine &Message,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.PrintMessage(Loc, Kind, Message, Ranges, /* FixIts= */ {}, ShowColors);
  }

  শূন্য setError(ধ্রুৱ Twine &Message, StringRef::iterator Position) {
    যদি (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    যদি (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    যদি (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = সঁচা;
  }

  /// Returns true if an error occurred while parsing.
  তার্কিক failed() {
    ঘূৰাই Failed;
  }

গুপ্ত:
  শূন্য init(MemoryBufferRef Buffer);

  StringRef current_প্ৰবেশ() {
    ঘূৰাই StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    ঘূৰাই ::decodeUTF8(StringRef(Position, End - Position));
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
  শূন্য advanceWhile(SkipWhileFunc Func);

  /// Scan ns-uri-char[39]s starting at Cur.
  ///
  /// This updates Cur and Column while scanning.
  শূন্য scan_ns_uri_char();

  /// Consume a minimal well-formed code unit subsequence starting at
  ///        \a Cur. Return false if it is not the same Unicode scalar value as
  ///        \a Expected. This updates \a Column.
  তার্কিক consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  শূন্য skip(uint32_t Distance);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  তার্কিক isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  তার্কিক isPlainSafeNonBlank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  তার্কিক is_সাৰি_Empty(StringRef সাৰি);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  তার্কিক consume_সাৰি_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  শূন্য save_Simple_চাবি_Candidate( TokenQueueT::iterator Tok
                             , unsigned AtColumn
                             , তার্কিক IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  শূন্য remove_Stale_Simple_চাবি_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  শূন্য remove_Simple_চাবি_Candidates_On_Flow_Level(unsigned Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  তার্কিক unrollIndent(পূর্ণাঙ্ক ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  তার্কিক rollIndent( পূর্ণাঙ্ক ToColumn
                 , Token::TokenKind Kind
                 , TokenQueueT::iterator ঢোকাও_Point);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  শূন্য skipComment();

  /// Skip whitespace and comments until the start of the next token.
  শূন্য scanToNextToken();

  /// Must be the first token generated.
  তার্কিক scanStreamStart();

  /// Generate tokens needed to close out the stream.
  তার্কিক scanStreamEnd();

  /// Scan a %BLAH directive.
  তার্কিক scanDirective();

  /// Scan a ... or ---.
  তার্কিক scanDocumentIndicator(তার্কিক IsStart);

  /// Scan a [ or { and generate the proper flow collection start token.
  তার্কিক scanFlowCollectionStart(তার্কিক IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  তার্কিক scanFlowCollectionEnd(তার্কিক IsSequence);

  /// Scan the , that separates entries in a flow collection.
  তার্কিক scanFlowEntry();

  /// Scan the - that starts block sequence entries.
  তার্কিক scanBlockEntry();

  /// Scan an explicit ? indicating a key.
  তার্কিক scan_চাবি();

  /// Scan an explicit : indicating a value.
  তার্কিক scan_মান();

  /// Scan a quoted scalar.
  তার্কিক scanFlowScalar(তার্কিক IsDoubleQuoted);

  /// Scan an unquoted scalar.
  তার্কিক scanPlainScalar();

  /// Scan an Alias or Anchor starting with * or &.
  তার্কিক scanAliasOrAnchor(তার্কিক IsAlias);

  /// Scan a block scalar starting with | or >.
  তার্কিক scanBlockScalar(তার্কিক IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  তার্কিক scanBlockScalarIndicators(ৱর্ণ &StyleIndicator, ৱর্ণ &ChompingIndicator,
                                 unsigned &IndentIndicator, তার্কিক &IsDone);

  /// Scan a style indicator in a block scalar header.
  ৱর্ণ scanBlockStyleIndicator();

  /// Scan a chomping indicator in a block scalar header.
  ৱর্ণ scanBlockChompingIndicator();

  /// Scan an indentation indicator in a block scalar header.
  unsigned scanBlockIndentationIndicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  তার্কিক scanBlockScalarHeader(ৱর্ণ &ChompingIndicator, unsigned &IndentIndicator,
                             তার্কিক &IsDone);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  তার্কিক findBlockScalarIndent(unsigned &BlockIndent, unsigned BlockExitIndent,
                             unsigned &সাৰি_Breaks, তার্কিক &IsDone);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  তার্কিক scanBlockScalarIndent(unsigned BlockIndent, unsigned BlockExitIndent,
                             তার্কিক &IsDone);

  /// Scan a tag of the form !stuff.
  তার্কিক scanTag();

  /// Dispatch to the next scanning function based on \a *Cur.
  তার্কিক fetchMoreTokens();

  /// The SourceMgr used for diagnostics and buffer management.
  SourceMgr &SM;

  /// The original input.
  MemoryBufferRef প্ৰবেশ_Buffer;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator End;

  /// Current YAML indentation level in spaces.
  পূর্ণাঙ্ক Indent;

  /// Current column number in Unicode code points.
  unsigned Column;

  /// Current line number.
  unsigned সাৰি;

  /// How deep we are in flow style containers. 0 Means at block level.
  unsigned FlowLevel;

  /// Are we at the start of the stream?
  তার্কিক IsStartOfStream;

  /// Can the next token be the start of a simple key?
  তার্কিক Is_Simple_চাবি_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  তার্কিক Is_Adjacent_মান_Allowed_In_Flow;

  /// True if an error has occurred.
  তার্কিক Failed;

  /// Should colors be used when printing out the diagnostic messages?
  তার্কিক ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  TokenQueueT TokenQueue;

  /// Indentation levels.
  SmallVector<পূর্ণাঙ্ক, 4> Indents;

  /// Potential simple keys.
  SmallVector<Simple_চাবি, 4> Simple_চাবিগুলি;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
স্থির শূন্য encodeUTF8( uint32_t Unicode_Scalar_মান
                      , SmallVectorImpl<ৱর্ণ> &Result) {
  যদি (Unicode_Scalar_মান <= 0x7F) {
    Result.ঠেলো_back(Unicode_Scalar_মান & 0x7F);
  } অন্যথা যদি (Unicode_Scalar_মান <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((Unicode_Scalar_মান & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Unicode_Scalar_মান & 0x3F);
    Result.ঠেলো_back(FirstByte);
    Result.ঠেলো_back(SecondByte);
  } অন্যথা যদি (Unicode_Scalar_মান <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((Unicode_Scalar_মান & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_মান & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Unicode_Scalar_মান & 0x3F);
    Result.ঠেলো_back(FirstByte);
    Result.ঠেলো_back(SecondByte);
    Result.ঠেলো_back(ThirdByte);
  } অন্যথা যদি (Unicode_Scalar_মান <= 0x10FFFF) {
    uint8_t FirstByte = 0xF0 | ((Unicode_Scalar_মান & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_মান & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Unicode_Scalar_মান & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_মান & 0x3F);
    Result.ঠেলো_back(FirstByte);
    Result.ঠেলো_back(SecondByte);
    Result.ঠেলো_back(ThirdByte);
    Result.ঠেলো_back(FourthByte);
  }
}

তার্কিক yaml::dumpTokens(StringRef প্ৰবেশ, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(প্ৰবেশ, SM);
  যেতিয়ালৈ (সঁচা) {
    Token T = scanner.getNext();
    ৱিকল্প (T.Kind) {
    পক্ষ Token::TK_StreamStart:
      OS << "Stream-Start: ";
      ভঙ্গ;
    পক্ষ Token::TK_StreamEnd:
      OS << "Stream-End: ";
      ভঙ্গ;
    পক্ষ Token::TK_VersionDirective:
      OS << "Version-Directive: ";
      ভঙ্গ;
    পক্ষ Token::TK_TagDirective:
      OS << "Tag-Directive: ";
      ভঙ্গ;
    পক্ষ Token::TK_DocumentStart:
      OS << "Document-Start: ";
      ভঙ্গ;
    পক্ষ Token::TK_DocumentEnd:
      OS << "Document-End: ";
      ভঙ্গ;
    পক্ষ Token::TK_BlockEntry:
      OS << "Block-Entry: ";
      ভঙ্গ;
    পক্ষ Token::TK_BlockEnd:
      OS << "Block-End: ";
      ভঙ্গ;
    পক্ষ Token::TK_BlockSequenceStart:
      OS << "Block-Sequence-Start: ";
      ভঙ্গ;
    পক্ষ Token::TK_BlockMappingStart:
      OS << "Block-Mapping-Start: ";
      ভঙ্গ;
    পক্ষ Token::TK_FlowEntry:
      OS << "Flow-Entry: ";
      ভঙ্গ;
    পক্ষ Token::TK_FlowSequenceStart:
      OS << "Flow-Sequence-Start: ";
      ভঙ্গ;
    পক্ষ Token::TK_FlowSequenceEnd:
      OS << "Flow-Sequence-End: ";
      ভঙ্গ;
    পক্ষ Token::TK_FlowMappingStart:
      OS << "Flow-Mapping-Start: ";
      ভঙ্গ;
    পক্ষ Token::TK_FlowMappingEnd:
      OS << "Flow-Mapping-End: ";
      ভঙ্গ;
    পক্ষ Token::TK_চাবি:
      OS << "Key: ";
      ভঙ্গ;
    পক্ষ Token::TK_মান:
      OS << "Value: ";
      ভঙ্গ;
    পক্ষ Token::TK_Scalar:
      OS << "Scalar: ";
      ভঙ্গ;
    পক্ষ Token::TK_BlockScalar:
      OS << "Block Scalar: ";
      ভঙ্গ;
    পক্ষ Token::TK_Alias:
      OS << "Alias: ";
      ভঙ্গ;
    পক্ষ Token::TK_Anchor:
      OS << "Anchor: ";
      ভঙ্গ;
    পক্ষ Token::TK_Tag:
      OS << "Tag: ";
      ভঙ্গ;
    পক্ষ Token::TK_Error:
      ভঙ্গ;
    }
    OS << T.Range << "\n";
    যদি (T.Kind == Token::TK_StreamEnd)
      ভঙ্গ;
    অন্যথা যদি (T.Kind == Token::TK_Error)
      ঘূৰাই মিছা;
  }
  ঘূৰাই সঁচা;
}

তার্কিক yaml::scanTokens(StringRef প্ৰবেশ) {
  SourceMgr SM;
  Scanner scanner(প্ৰবেশ, SM);
  যেতিয়ালৈ (সঁচা) {
    Token T = scanner.getNext();
    যদি (T.Kind == Token::TK_StreamEnd)
      ভঙ্গ;
    অন্যথা যদি (T.Kind == Token::TK_Error)
      ঘূৰাই মিছা;
  }
  ঘূৰাই সঁচা;
}

std::string yaml::escape(StringRef প্ৰবেশ, তার্কিক EscapePrintable) {
  std::string Escaped_প্ৰবেশ;
  বাবে (StringRef::iterator i = প্ৰবেশ.begin(), e = প্ৰবেশ.end(); i != e; ++i) {
    যদি (*i == '\\')
      Escaped_প্ৰবেশ += "\\\\";
    অন্যথা যদি (*i == '"')
      Escaped_প্ৰবেশ += "\\\"";
    অন্যথা যদি (*i == 0)
      Escaped_প্ৰবেশ += "\\0";
    অন্যথা যদি (*i == 0x07)
      Escaped_প্ৰবেশ += "\\a";
    অন্যথা যদি (*i == 0x08)
      Escaped_প্ৰবেশ += "\\b";
    অন্যথা যদি (*i == 0x09)
      Escaped_প্ৰবেশ += "\\t";
    অন্যথা যদি (*i == 0x0A)
      Escaped_প্ৰবেশ += "\\n";
    অন্যথা যদি (*i == 0x0B)
      Escaped_প্ৰবেশ += "\\v";
    অন্যথা যদি (*i == 0x0C)
      Escaped_প্ৰবেশ += "\\f";
    অন্যথা যদি (*i == 0x0D)
      Escaped_প্ৰবেশ += "\\r";
    অন্যথা যদি (*i == 0x1B)
      Escaped_প্ৰবেশ += "\\e";
    অন্যথা যদি ((unsigned ৱর্ণ)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_প্ৰবেশ += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
    } অন্যথা যদি (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_মান
        = decodeUTF8(StringRef(i, প্ৰবেশ.end() - i));
      যদি (Unicode_Scalar_মান.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_প্ৰবেশ, Val);
        // FIXME: Error reporting.
        ঘূৰাই Escaped_প্ৰবেশ;
      }
      যদি (Unicode_Scalar_মান.first == 0x85)
        Escaped_প্ৰবেশ += "\\N";
      অন্যথা যদি (Unicode_Scalar_মান.first == 0xA0)
        Escaped_প্ৰবেশ += "\\_";
      অন্যথা যদি (Unicode_Scalar_মান.first == 0x2028)
        Escaped_প্ৰবেশ += "\\L";
      অন্যথা যদি (Unicode_Scalar_মান.first == 0x2029)
        Escaped_প্ৰবেশ += "\\P";
      অন্যথা যদি (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_মান.first))
        Escaped_প্ৰবেশ += StringRef(i, Unicode_Scalar_মান.second);
      অন্যথা {
        std::string HexStr = utohexstr(Unicode_Scalar_মান.first);
        যদি (HexStr.size() <= 2)
          Escaped_প্ৰবেশ += "\\x" + std::string(2 - HexStr.size(), '0') + HexStr;
        অন্যথা যদি (HexStr.size() <= 4)
          Escaped_প্ৰবেশ += "\\u" + std::string(4 - HexStr.size(), '0') + HexStr;
        অন্যথা যদি (HexStr.size() <= 8)
          Escaped_প্ৰবেশ += "\\U" + std::string(8 - HexStr.size(), '0') + HexStr;
      }
      i += Unicode_Scalar_মান.second - 1;
    } অন্যথা {
      Escaped_প্ৰবেশ.ঠেলো_back(*i);
    }
  }
  ঘূৰাই Escaped_প্ৰবেশ;
}

std::optional<তার্কিক> yaml::parseBool(StringRef S) {
  ৱিকল্প (S.size()) {
  পক্ষ 1:
    ৱিকল্প (S.front()) {
    পক্ষ 'y':
    পক্ষ 'Y':
      ঘূৰাই সঁচা;
    পক্ষ 'n':
    পক্ষ 'N':
      ঘূৰাই মিছা;
    স্ৱভাৱ:
      ঘূৰাই std::nullopt;
    }
  পক্ষ 2:
    ৱিকল্প (S.front()) {
    পক্ষ 'O':
      যদি (S[1] == 'N') // ON
        ঘূৰাই সঁচা;
      [[fallthrough]];
    পক্ষ 'o':
      যদি (S[1] == 'n') //[Oo]n
        ঘূৰাই সঁচা;
      ঘূৰাই std::nullopt;
    পক্ষ 'N':
      যদি (S[1] == 'O') // NO
        ঘূৰাই মিছা;
      [[fallthrough]];
    পক্ষ 'n':
      যদি (S[1] == 'o') //[Nn]o
        ঘূৰাই মিছা;
      ঘূৰাই std::nullopt;
    স্ৱভাৱ:
      ঘূৰাই std::nullopt;
    }
  পক্ষ 3:
    ৱিকল্প (S.front()) {
    পক্ষ 'O':
      যদি (S.drop_front() == "FF") // OFF
        ঘূৰাই মিছা;
      [[fallthrough]];
    পক্ষ 'o':
      যদি (S.drop_front() == "ff") //[Oo]ff
        ঘূৰাই মিছা;
      ঘূৰাই std::nullopt;
    পক্ষ 'Y':
      যদি (S.drop_front() == "ES") // YES
        ঘূৰাই সঁচা;
      [[fallthrough]];
    পক্ষ 'y':
      যদি (S.drop_front() == "es") //[Yy]es
        ঘূৰাই সঁচা;
      ঘূৰাই std::nullopt;
    স্ৱভাৱ:
      ঘূৰাই std::nullopt;
    }
  পক্ষ 4:
    ৱিকল্প (S.front()) {
    পক্ষ 'T':
      যদি (S.drop_front() == "RUE") // TRUE
        ঘূৰাই সঁচা;
      [[fallthrough]];
    পক্ষ 't':
      যদি (S.drop_front() == "rue") //[Tt]rue
        ঘূৰাই সঁচা;
      ঘূৰাই std::nullopt;
    স্ৱভাৱ:
      ঘূৰাই std::nullopt;
    }
  পক্ষ 5:
    ৱিকল্প (S.front()) {
    পক্ষ 'F':
      যদি (S.drop_front() == "ALSE") // FALSE
        ঘূৰাই মিছা;
      [[fallthrough]];
    পক্ষ 'f':
      যদি (S.drop_front() == "alse") //[Ff]alse
        ঘূৰাই মিছা;
      ঘূৰাই std::nullopt;
    স্ৱভাৱ:
      ঘূৰাই std::nullopt;
    }
  স্ৱভাৱ:
    ঘূৰাই std::nullopt;
  }
}

Scanner::Scanner(StringRef প্ৰবেশ, SourceMgr &sm, তার্কিক ShowColors,
                 std::error_code *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(MemoryBufferRef(প্ৰবেশ, "YAML"));
}

Scanner::Scanner(MemoryBufferRef Buffer, SourceMgr &SM_, তার্কিক ShowColors,
                 std::error_code *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(Buffer);
}

শূন্য Scanner::init(MemoryBufferRef Buffer) {
  প্ৰবেশ_Buffer = Buffer;
  Current = প্ৰবেশ_Buffer.getBufferStart();
  End = প্ৰবেশ_Buffer.getBufferEnd();
  Indent = -1;
  Column = 0;
  সাৰি = 0;
  FlowLevel = 0;
  IsStartOfStream = সঁচা;
  Is_Simple_চাবি_Allowed = সঁচা;
  Is_Adjacent_মান_Allowed_In_Flow = মিছা;
  Failed = মিছা;
  std::unique_ptr<MemoryBuffer> প্ৰবেশ_Buffer_Owner =
      MemoryBuffer::getMemBuffer(Buffer, /*RequiresNullTerminator=*/মিছা);
  SM.AddNewSourceBuffer(std::move(প্ৰবেশ_Buffer_Owner), SMLoc());
}

Token &Scanner::peekNext() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  তার্কিক NeedMore = মিছা;
  যেতিয়ালৈ (সঁচা) {
    যদি (TokenQueue.empty() || NeedMore) {
      যদি (!fetchMoreTokens()) {
        TokenQueue.clear();
        Simple_চাবিগুলি.clear();
        TokenQueue.ঠেলো_back(Token());
        ঘূৰাই TokenQueue.front();
      }
    }
    প্রতিজ্ঞা(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_চাবি_Candidates();
    Simple_চাবি SK;
    SK.Tok = TokenQueue.begin();
    যদি (!is_contained(Simple_চাবিগুলি, SK))
      ভঙ্গ;
    অন্যথা
      NeedMore = সঁচা;
  }
  ঘূৰাই TokenQueue.front();
}

Token Scanner::getNext() {
  Token Ret = peekNext();
  // TokenQueue can be empty if there was an error getting the next token.
  যদি (!TokenQueue.empty())
    TokenQueue.বেৰ_কৰো_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  যদি (TokenQueue.empty())
    TokenQueue.resetAlloc();

  ঘূৰাই Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  যদি (Position == End)
    ঘূৰাই Position;
  // Check 7 bit c-printable - b-char.
  যদি (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    ঘূৰাই Position + 1;

  // Check for valid UTF-8.
  যদি (uint8_t(*Position) & 0x80) {
    UTF8Decoded u8d = decodeUTF8(Position);
    যদি (   u8d.second != 0
        && u8d.first != 0xFEFF
        && ( u8d.first == 0x85
          || ( u8d.first >= 0xA0
            && u8d.first <= 0xD7FF)
          || ( u8d.first >= 0xE000
            && u8d.first <= 0xFFFD)
          || ( u8d.first >= 0x10000
            && u8d.first <= 0x10FFFF)))
      ঘূৰাই Position + u8d.second;
  }
  ঘূৰাই Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  যদি (Position == End)
    ঘূৰাই Position;
  যদি (*Position == 0x0D) {
    যদি (Position + 1 != End && *(Position + 1) == 0x0A)
      ঘূৰাই Position + 2;
    ঘূৰাই Position + 1;
  }

  যদি (*Position == 0x0A)
    ঘূৰাই Position + 1;
  ঘূৰাই Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  যদি (Position == End)
    ঘূৰাই Position;
  যদি (*Position == ' ')
    ঘূৰাই Position + 1;
  ঘূৰাই Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  যদি (Position == End)
    ঘূৰাই Position;
  যদি (*Position == ' ' || *Position == '\t')
    ঘূৰাই Position + 1;
  ঘূৰাই Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  যদি (Position == End)
    ঘূৰাই Position;
  যদি (*Position == ' ' || *Position == '\t')
    ঘূৰাই Position;
  ঘূৰাই skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  যেতিয়ালৈ (সঁচা) {
    StringRef::iterator i = (এতত্->*Func)(Position);
    যদি (i == Position)
      ভঙ্গ;
    Position = i;
  }
  ঘূৰাই Position;
}

শূন্য Scanner::advanceWhile(SkipWhileFunc Func) {
  auto Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

স্থির তার্কিক is_ns_hex_digit(ধ্রুৱ ৱর্ণ C) { ঘূৰাই isAlnum(C); }

স্থির তার্কিক is_ns_word_char(ধ্রুৱ ৱর্ণ C) { ঘূৰাই C == '-' || isAlpha(C); }

শূন্য Scanner::scan_ns_uri_char() {
  যেতিয়ালৈ (সঁচা) {
    যদি (Current == End)
      ভঙ্গ;
    যদি ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } অন্যথা {
      ভঙ্গ;
    }
  }
}

তার্কিক Scanner::consume(uint32_t Expected) {
  যদি (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    ঘূৰাই মিছা;
  }
  যদি (Current == End)
    ঘূৰাই মিছা;
  যদি (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    ঘূৰাই মিছা;
  }
  যদি (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    ঘূৰাই সঁচা;
  }
  ঘূৰাই মিছা;
}

শূন্য Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  প্রতিজ্ঞা(Current <= End && "Skipped past the end");
}

তার্কিক Scanner::isBlankOrBreak(StringRef::iterator Position) {
  যদি (Position == End)
    ঘূৰাই মিছা;
  ঘূৰাই *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

তার্কিক Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  যদি (Position == End || isBlankOrBreak(Position))
    ঘূৰাই মিছা;
  যদি (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    ঘূৰাই মিছা;
  ঘূৰাই সঁচা;
}

তার্কিক Scanner::is_সাৰি_Empty(StringRef সাৰি) {
  বাবে (ধ্রুৱ auto *Position = সাৰি.begin(); Position != সাৰি.end(); ++Position)
    যদি (!isBlankOrBreak(Position))
      ঘূৰাই মিছা;
  ঘূৰাই সঁচা;
}

তার্কিক Scanner::consume_সাৰি_Break_If_Present() {
  auto Next = skip_b_break(Current);
  যদি (Next == Current)
    ঘূৰাই মিছা;
  Column = 0;
  ++সাৰি;
  Current = Next;
  ঘূৰাই সঁচা;
}

শূন্য Scanner::save_Simple_চাবি_Candidate( TokenQueueT::iterator Tok
                                    , unsigned AtColumn
                                    , তার্কিক IsRequired) {
  যদি (Is_Simple_চাবি_Allowed) {
    Simple_চাবি SK;
    SK.Tok = Tok;
    SK.সাৰি = সাৰি;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_চাবিগুলি.ঠেলো_back(SK);
  }
}

শূন্য Scanner::remove_Stale_Simple_চাবি_Candidates() {
  বাবে (SmallVectorImpl<Simple_চাবি>::iterator i = Simple_চাবিগুলি.begin();
                                            i != Simple_চাবিগুলি.end();) {
    যদি (i->সাৰি != সাৰি || i->Column + 1024 < Column) {
      যদি (i->IsRequired)
        setError( "Could not find expected : for simple key"
                , i->Tok->Range.begin());
      i = Simple_চাবিগুলি.erase(i);
    } অন্যথা {
      ++i;
    }
  }
}

শূন্য Scanner::remove_Simple_চাবি_Candidates_On_Flow_Level(unsigned Level) {
  যদি (!Simple_চাবিগুলি.empty() && (Simple_চাবিগুলি.end() - 1)->FlowLevel == Level)
    Simple_চাবিগুলি.বেৰ_কৰো_back();
}

তার্কিক Scanner::unrollIndent(পূর্ণাঙ্ক ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  যদি (FlowLevel != 0)
    ঘূৰাই সঁচা;

  যেতিয়ালৈ (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.ঠেলো_back(T);
    Indent = Indents.বেৰ_কৰো_back_val();
  }

  ঘূৰাই সঁচা;
}

তার্কিক Scanner::rollIndent( পূর্ণাঙ্ক ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator ঢোকাও_Point) {
  যদি (FlowLevel)
    ঘূৰাই সঁচা;
  যদি (Indent < ToColumn) {
    Indents.ঠেলো_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.ঢোকাও(ঢোকাও_Point, T);
  }
  ঘূৰাই সঁচা;
}

শূন্য Scanner::skipComment() {
  যদি (Current == End || *Current != '#')
    ঘূৰাই;
  যেতিয়ালৈ (সঁচা) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    যদি (I == Current)
      ভঙ্গ;
    Current = I;
    ++Column;
  }
}

শূন্য Scanner::scanToNextToken() {
  যেতিয়ালৈ (সঁচা) {
    যেতিয়ালৈ (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    যদি (i == Current)
      ভঙ্গ;
    Current = i;
    ++সাৰি;
    Column = 0;
    // New lines may start a simple key.
    যদি (!FlowLevel)
      Is_Simple_চাবি_Allowed = সঁচা;
  }
}

তার্কিক Scanner::scanStreamStart() {
  IsStartOfStream = মিছা;

  EncodingInfo EI = getUnicodeEncoding(current_প্ৰবেশ());

  Token T;
  T.Kind = Token::TK_StreamStart;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.ঠেলো_back(T);
  Current += EI.second;
  ঘূৰাই সঁচা;
}

তার্কিক Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  যদি (Column != 0) {
    Column = 0;
    ++সাৰি;
  }

  unrollIndent(-1);
  Simple_চাবিগুলি.clear();
  Is_Simple_চাবি_Allowed = মিছা;
  Is_Adjacent_মান_Allowed_In_Flow = মিছা;

  Token T;
  T.Kind = Token::TK_StreamEnd;
  T.Range = StringRef(Current, 0);
  TokenQueue.ঠেলো_back(T);
  ঘূৰাই সঁচা;
}

তার্কিক Scanner::scanDirective() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_চাবিগুলি.clear();
  Is_Simple_চাবি_Allowed = মিছা;
  Is_Adjacent_মান_Allowed_In_Flow = মিছা;

  StringRef::iterator Start = Current;
  consume('%');
  StringRef::iterator NameStart = Current;
  Current = skip_while(&Scanner::skip_ns_char, Current);
  StringRef Name(NameStart, Current - NameStart);
  Current = skip_while(&Scanner::skip_s_white, Current);

  Token T;
  যদি (Name == "YAML") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_VersionDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ঠেলো_back(T);
    ঘূৰাই সঁচা;
  } অন্যথা যদি(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ঠেলো_back(T);
    ঘূৰাই সঁচা;
  }
  ঘূৰাই মিছা;
}

তার্কিক Scanner::scanDocumentIndicator(তার্কিক IsStart) {
  unrollIndent(-1);
  Simple_চাবিগুলি.clear();
  Is_Simple_চাবি_Allowed = মিছা;
  Is_Adjacent_মান_Allowed_In_Flow = মিছা;

  Token T;
  T.Kind = IsStart ? Token::TK_DocumentStart : Token::TK_DocumentEnd;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.ঠেলো_back(T);
  ঘূৰাই সঁচা;
}

তার্কিক Scanner::scanFlowCollectionStart(তার্কিক IsSequence) {
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ঠেলো_back(T);

  // [ and { may begin a simple key.
  save_Simple_চাবি_Candidate(--TokenQueue.end(), Column - 1, মিছা);

  // And may also be followed by a simple key.
  Is_Simple_চাবি_Allowed = সঁচা;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_মান_Allowed_In_Flow = মিছা;
  ++FlowLevel;
  ঘূৰাই সঁচা;
}

তার্কিক Scanner::scanFlowCollectionEnd(তার্কিক IsSequence) {
  remove_Simple_চাবি_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_চাবি_Allowed = মিছা;
  Is_Adjacent_মান_Allowed_In_Flow = সঁচা;
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceEnd
                      : Token::TK_FlowMappingEnd;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ঠেলো_back(T);
  যদি (FlowLevel)
    --FlowLevel;
  ঘূৰাই সঁচা;
}

তার্কিক Scanner::scanFlowEntry() {
  remove_Simple_চাবি_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_চাবি_Allowed = সঁচা;
  Is_Adjacent_মান_Allowed_In_Flow = মিছা;
  Token T;
  T.Kind = Token::TK_FlowEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ঠেলো_back(T);
  ঘূৰাই সঁচা;
}

তার্কিক Scanner::scanBlockEntry() {
  rollIndent(Column, Token::TK_BlockSequenceStart, TokenQueue.end());
  remove_Simple_চাবি_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_চাবি_Allowed = সঁচা;
  Is_Adjacent_মান_Allowed_In_Flow = মিছা;
  Token T;
  T.Kind = Token::TK_BlockEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ঠেলো_back(T);
  ঘূৰাই সঁচা;
}

তার্কিক Scanner::scan_চাবি() {
  যদি (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  remove_Simple_চাবি_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_চাবি_Allowed = !FlowLevel;
  Is_Adjacent_মান_Allowed_In_Flow = মিছা;

  Token T;
  T.Kind = Token::TK_চাবি;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ঠেলো_back(T);
  ঘূৰাই সঁচা;
}

তার্কিক Scanner::scan_মান() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  যদি (!Simple_চাবিগুলি.empty()) {
    Simple_চাবি SK = Simple_চাবিগুলি.বেৰ_কৰো_back_val();
    Token T;
    T.Kind = Token::TK_চাবি;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    বাবে (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      যদি (i == SK.Tok)
        ভঙ্গ;
    }
    যদি (i == e) {
      Failed = সঁচা;
      ঘূৰাই মিছা;
    }
    i = TokenQueue.ঢোকাও(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    Is_Simple_চাবি_Allowed = মিছা;
  } অন্যথা {
    যদি (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    Is_Simple_চাবি_Allowed = !FlowLevel;
  }
  Is_Adjacent_মান_Allowed_In_Flow = মিছা;

  Token T;
  T.Kind = Token::TK_মান;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ঠেলো_back(T);
  ঘূৰাই সঁচা;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE স্থির তার্কিক
wasEscaped(StringRef::iterator First, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
স্থির তার্কিক wasEscaped(StringRef::iterator First,
                       StringRef::iterator Position) {
  প্রতিজ্ঞা(Position - 1 >= First);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  যেতিয়ালৈ (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  ঘূৰাই (Position - 1 - I) % 2 == 1;
}

তার্কিক Scanner::scanFlowScalar(তার্কিক IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  যদি (IsDoubleQuoted) {
    কৰা {
      ++Current;
      যেতিয়ালৈ (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } যেতিয়ালৈ (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } অন্যথা {
    skip(1);
    যেতিয়ালৈ (Current != End) {
      // Skip a ' followed by another '.
      যদি (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        অনুৱর্ত;
      } অন্যথা যদি (*Current == '\'')
        ভঙ্গ;
      StringRef::iterator i = skip_nb_char(Current);
      যদি (i == Current) {
        i = skip_b_break(Current);
        যদি (i == Current)
          ভঙ্গ;
        Current = i;
        Column = 0;
        ++সাৰি;
      } অন্যথা {
        যদি (i == End)
          ভঙ্গ;
        Current = i;
        ++Column;
      }
    }
  }

  যদি (Current == End) {
    setError("Expected quote at end of scalar", Current);
    ঘূৰাই মিছা;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ঠেলো_back(T);

  save_Simple_চাবি_Candidate(--TokenQueue.end(), ColStart, মিছা);

  Is_Simple_চাবি_Allowed = মিছা;
  Is_Adjacent_মান_Allowed_In_Flow = সঁচা;

  ঘূৰাই সঁচা;
}

তার্কিক Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  unsigned LeadingBlanks = 0;
  প্রতিজ্ঞা(Indent >= -1 && "Indent must be >= -1 !");
  unsigned indent = static_cast<unsigned>(Indent + 1);
  যেতিয়ালৈ (Current != End) {
    যদি (*Current == '#')
      ভঙ্গ;

    যেতিয়ালৈ (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      যদি (i == Current)
        ভঙ্গ;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    যদি (!isBlankOrBreak(Current))
      ভঙ্গ;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    যেতিয়ালৈ (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      যদি (i != Tmp) {
        যদি (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          ঘূৰাই মিছা;
        }
        Tmp = i;
        ++Column;
      } অন্যথা {
        i = skip_b_break(Tmp);
        যদি (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++সাৰি;
      }
    }

    যদি (!FlowLevel && Column < indent)
      ভঙ্গ;

    Current = Tmp;
  }
  যদি (Start == Current) {
    setError("Got empty plain scalar", Start);
    ঘূৰাই মিছা;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ঠেলো_back(T);

  // Plain scalars can be simple keys.
  save_Simple_চাবি_Candidate(--TokenQueue.end(), ColStart, মিছা);

  Is_Simple_চাবি_Allowed = মিছা;
  Is_Adjacent_মান_Allowed_In_Flow = মিছা;

  ঘূৰাই সঁচা;
}

তার্কিক Scanner::scanAliasOrAnchor(তার্কিক IsAlias) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1);
  যেতিয়ালৈ (Current != End) {
    যদি (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      ভঙ্গ;
    StringRef::iterator i = skip_ns_char(Current);
    যদি (i == Current)
      ভঙ্গ;
    Current = i;
    ++Column;
  }

  যদি (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    ঘূৰাই মিছা;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ঠেলো_back(T);

  // Alias and anchors can be simple keys.
  save_Simple_চাবি_Candidate(--TokenQueue.end(), ColStart, মিছা);

  Is_Simple_চাবি_Allowed = মিছা;
  Is_Adjacent_মান_Allowed_In_Flow = মিছা;

  ঘূৰাই সঁচা;
}

তার্কিক Scanner::scanBlockScalarIndicators(ৱর্ণ &StyleIndicator,
                                        ৱর্ণ &ChompingIndicator,
                                        unsigned &IndentIndicator,
                                        তার্কিক &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  যদি (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    ঘূৰাই মিছা;
  ঘূৰাই সঁচা;
}

ৱর্ণ Scanner::scanBlockStyleIndicator() {
  ৱর্ণ Indicator = ' ';
  যদি (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  ঘূৰাই Indicator;
}

ৱর্ণ Scanner::scanBlockChompingIndicator() {
  ৱর্ণ Indicator = ' ';
  যদি (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  ঘূৰাই Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
স্থির unsigned get_Chomped_সাৰি_Breaks(ৱর্ণ ChompingIndicator,
                                     unsigned সাৰি_Breaks, StringRef Str) {
  যদি (ChompingIndicator == '-') // Strip all line breaks.
    ঘূৰাই 0;
  যদি (ChompingIndicator == '+') // Keep all line breaks.
    ঘূৰাই সাৰি_Breaks;
  // Clip trailing lines.
  ঘূৰাই Str.empty() ? 0 : 1;
}

unsigned Scanner::scanBlockIndentationIndicator() {
  unsigned Indent = 0;
  যদি (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = unsigned(*Current - '0');
    skip(1);
  }
  ঘূৰাই Indent;
}

তার্কিক Scanner::scanBlockScalarHeader(ৱর্ণ &ChompingIndicator,
                                    unsigned &IndentIndicator, তার্কিক &IsDone) {
  auto Start = Current;

  ChompingIndicator = scanBlockChompingIndicator();
  IndentIndicator = scanBlockIndentationIndicator();
  // Check for the chomping indicator once again.
  যদি (ChompingIndicator == ' ')
    ChompingIndicator = scanBlockChompingIndicator();
  Current = skip_while(&Scanner::skip_s_white, Current);
  skipComment();

  যদি (Current == End) { // EOF, we have an empty scalar.
    Token T;
    T.Kind = Token::TK_BlockScalar;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ঠেলো_back(T);
    IsDone = সঁচা;
    ঘূৰাই সঁচা;
  }

  যদি (!consume_সাৰি_Break_If_Present()) {
    setError("Expected a line break after block scalar header", Current);
    ঘূৰাই মিছা;
  }
  ঘূৰাই সঁচা;
}

তার্কিক Scanner::findBlockScalarIndent(unsigned &BlockIndent,
                                    unsigned BlockExitIndent,
                                    unsigned &সাৰি_Breaks, তার্কিক &IsDone) {
  unsigned Max_All_Space_সাৰি_Characters = 0;
  StringRef::iterator Longest_All_Space_সাৰি;

  যেতিয়ালৈ (সঁচা) {
    advanceWhile(&Scanner::skip_s_space);
    যদি (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      যদি (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = সঁচা;
        ঘূৰাই সঁচা;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      যদি (Max_All_Space_সাৰি_Characters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_সাৰি);
        ঘূৰাই মিছা;
      }
      ঘূৰাই সঁচা;
    }
    যদি (skip_b_break(Current) != Current &&
        Column > Max_All_Space_সাৰি_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_সাৰি_Characters = Column;
      Longest_All_Space_সাৰি = Current;
    }

    // Check for EOF.
    যদি (Current == End) {
      IsDone = সঁচা;
      ঘূৰাই সঁচা;
    }

    যদি (!consume_সাৰি_Break_If_Present()) {
      IsDone = সঁচা;
      ঘূৰাই সঁচা;
    }
    ++সাৰি_Breaks;
  }
  ঘূৰাই সঁচা;
}

তার্কিক Scanner::scanBlockScalarIndent(unsigned BlockIndent,
                                    unsigned BlockExitIndent, তার্কিক &IsDone) {
  // Skip the indentation.
  যেতিয়ালৈ (Column < BlockIndent) {
    auto I = skip_s_space(Current);
    যদি (I == Current)
      ভঙ্গ;
    Current = I;
    ++Column;
  }

  যদি (skip_nb_char(Current) == Current)
    ঘূৰাই সঁচা;

  যদি (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = সঁচা;
    ঘূৰাই সঁচা;
  }

  যদি (Column < BlockIndent) {
    যদি (Current != End && *Current == '#') { // Trailing comment.
      IsDone = সঁচা;
      ঘূৰাই সঁচা;
    }
    setError("A text line is less indented than the block scalar", Current);
    ঘূৰাই মিছা;
  }
  ঘূৰাই সঁচা; // A normal text line.
}

তার্কিক Scanner::scanBlockScalar(তার্কিক IsLiteral) {
  প্রতিজ্ঞা(*Current == '|' || *Current == '>');
  ৱর্ণ StyleIndicator;
  ৱর্ণ ChompingIndicator;
  unsigned BlockIndent;
  তার্কিক IsDone = মিছা;
  যদি (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    ঘূৰাই মিছা;
  যদি (IsDone)
    ঘূৰাই সঁচা;
  তার্কিক IsFolded = StyleIndicator == '>';

  ধ্রুৱ auto *Start = Current;
  unsigned BlockExitIndent = Indent < 0 ? 0 : (unsigned)Indent;
  unsigned সাৰি_Breaks = 0;
  যদি (BlockIndent == 0) {
    যদি (!findBlockScalarIndent(BlockIndent, BlockExitIndent, সাৰি_Breaks,
                               IsDone))
      ঘূৰাই মিছা;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  যেতিয়ালৈ (!IsDone) {
    যদি (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      ঘূৰাই মিছা;
    যদি (IsDone)
      ভঙ্গ;

    // Parse the current line.
    auto সাৰি_Start = Current;
    advanceWhile(&Scanner::skip_nb_char);
    যদি (সাৰি_Start != Current) {
      যদি (সাৰি_Breaks && IsFolded && !Scanner::is_সাৰি_Empty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        যদি (সাৰি_Breaks == 1) {
          Str.append(সাৰি_Breaks,
                     is_সাৰি_Empty(StringRef(সাৰি_Start, Current - সাৰি_Start))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        সাৰি_Breaks--;
      }
      Str.append(সাৰি_Breaks, '\n');
      Str.append(StringRef(সাৰি_Start, Current - সাৰি_Start));
      সাৰি_Breaks = 0;
    }

    // Check for EOF.
    যদি (Current == End)
      ভঙ্গ;

    যদি (!consume_সাৰি_Break_If_Present())
      ভঙ্গ;
    ++সাৰি_Breaks;
  }

  যদি (Current == End && !সাৰি_Breaks)
    // Ensure that there is at least one line break before the end of file.
    সাৰি_Breaks = 1;
  Str.append(get_Chomped_সাৰি_Breaks(ChompingIndicator, সাৰি_Breaks, Str), '\n');

  // New lines may start a simple key.
  যদি (!FlowLevel)
    Is_Simple_চাবি_Allowed = সঁচা;
  Is_Adjacent_মান_Allowed_In_Flow = মিছা;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.মান = std::string(Str);
  TokenQueue.ঠেলো_back(T);
  ঘূৰাই সঁচা;
}

তার্কিক Scanner::scanTag() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1); // Eat !.
  যদি (Current == End || isBlankOrBreak(Current)); // An empty tag.
  অন্যথা যদি (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    যদি (!consume('>'))
      ঘূৰাই মিছা;
  } অন্যথা {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.Kind = Token::TK_Tag;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ঠেলো_back(T);

  // Tags can be simple keys.
  save_Simple_চাবি_Candidate(--TokenQueue.end(), ColStart, মিছা);

  Is_Simple_চাবি_Allowed = মিছা;
  Is_Adjacent_মান_Allowed_In_Flow = মিছা;

  ঘূৰাই সঁচা;
}

তার্কিক Scanner::fetchMoreTokens() {
  যদি (IsStartOfStream)
    ঘূৰাই scanStreamStart();

  scanToNextToken();

  যদি (Current == End)
    ঘূৰাই scanStreamEnd();

  remove_Stale_Simple_চাবি_Candidates();

  unrollIndent(Column);

  যদি (Column == 0 && *Current == '%')
    ঘূৰাই scanDirective();

  যদি (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    ঘূৰাই scanDocumentIndicator(সঁচা);

  যদি (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    ঘূৰাই scanDocumentIndicator(মিছা);

  যদি (*Current == '[')
    ঘূৰাই scanFlowCollectionStart(সঁচা);

  যদি (*Current == '{')
    ঘূৰাই scanFlowCollectionStart(মিছা);

  যদি (*Current == ']')
    ঘূৰাই scanFlowCollectionEnd(সঁচা);

  যদি (*Current == '}')
    ঘূৰাই scanFlowCollectionEnd(মিছা);

  যদি (*Current == ',')
    ঘূৰাই scanFlowEntry();

  যদি (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    ঘূৰাই scanBlockEntry();

  যদি (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    ঘূৰাই scan_চাবি();

  যদি (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || Is_Adjacent_মান_Allowed_In_Flow))
    ঘূৰাই scan_মান();

  যদি (*Current == '*')
    ঘূৰাই scanAliasOrAnchor(সঁচা);

  যদি (*Current == '&')
    ঘূৰাই scanAliasOrAnchor(মিছা);

  যদি (*Current == '!')
    ঘূৰাই scanTag();

  যদি (*Current == '|' && !FlowLevel)
    ঘূৰাই scanBlockScalar(সঁচা);

  যদি (*Current == '>' && !FlowLevel)
    ঘূৰাই scanBlockScalar(মিছা);

  যদি (*Current == '\'')
    ঘূৰাই scanFlowScalar(মিছা);

  যদি (*Current == '"')
    ঘূৰাই scanFlowScalar(সঁচা);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  যদি ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    ঘূৰাই scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  ঘূৰাই মিছা;
}

Stream::Stream(StringRef প্ৰবেশ, SourceMgr &SM, তার্কিক ShowColors,
               std::error_code *EC)
    : scanner(নূতন Scanner(প্ৰবেশ, SM, ShowColors, EC)) {}

Stream::Stream(MemoryBufferRef প্ৰবেশ_Buffer, SourceMgr &SM, তার্কিক ShowColors,
               std::error_code *EC)
    : scanner(নূতন Scanner(প্ৰবেশ_Buffer, SM, ShowColors, EC)) {}

Stream::~Stream() = স্ৱভাৱ;

তার্কিক Stream::failed() { ঘূৰাই scanner->failed(); }

শূন্য Stream::printError(গ্ৰন্থি *N, ধ্রুৱ Twine &বাৰ্তা_2, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), বাৰ্তা_2, Kind);
}

শূন্য Stream::printError(ধ্রুৱ SMRange &Range, ধ্রুৱ Twine &বাৰ্তা_2,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, বাৰ্তা_2, Range);
}

document_iterator Stream::begin() {
  যদি (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->getNext();

  CurrentDoc.reset(নূতন Document(*এতত্));
  ঘূৰাই document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  ঘূৰাই document_iterator();
}

শূন্য Stream::skip() {
  বাবে (Document &Doc : *এতত্)
    Doc.skip();
}

গ্ৰন্থি::গ্ৰন্থি(unsigned পূর্ণাঙ্ক Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peekNext().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string গ্ৰন্থি::getVerbatimTag() ধ্রুৱ {
  StringRef Raw = getRawTag();
  যদি (!Raw.empty() && Raw != "!") {
    std::string Ret;
    যদি (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      ঘূৰাই Ret;
    } অন্যথা যদি (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      ঘূৰাই Ret;
    } অন্যথা {
      StringRef TagHandle = Raw.substr(0, Raw.find_last_of('!') + 1);
      std::map<StringRef, StringRef>::const_iterator It =
          Doc->getTagMap().find(TagHandle);
      যদি (It != Doc->getTagMap().end())
        Ret = std::string(It->second);
      অন্যথা {
        Token T;
        T.Kind = Token::TK_Tag;
        T.Range = TagHandle;
        setError(Twine("Unknown tag handle ") + TagHandle, T);
      }
      Ret += Raw.substr(Raw.find_last_of('!') + 1);
      ঘূৰাই Ret;
    }
  }

  ৱিকল্প (getType()) {
  পক্ষ NK_Null:
    ঘূৰাই "tag:yaml.org,2002:null";
  পক্ষ NK_Scalar:
  পক্ষ NK_BlockScalar:
    // TODO: Tag resolution.
    ঘূৰাই "tag:yaml.org,2002:str";
  পক্ষ NK_Mapping:
    ঘূৰাই "tag:yaml.org,2002:map";
  পক্ষ NK_Sequence:
    ঘূৰাই "tag:yaml.org,2002:seq";
  }

  ঘূৰাই "";
}

Token &গ্ৰন্থি::peekNext() {
  ঘূৰাই Doc->peekNext();
}

Token গ্ৰন্থি::getNext() {
  ঘূৰাই Doc->getNext();
}

গ্ৰন্থি *গ্ৰন্থি::parse_Block_গ্ৰন্থি() {
  ঘূৰাই Doc->parse_Block_গ্ৰন্থি();
}

BumpPtrAllocator &গ্ৰন্থি::getAllocator() {
  ঘূৰাই Doc->গ্ৰন্থি_Allocator;
}

শূন্য গ্ৰন্থি::setError(ধ্রুৱ Twine &বাৰ্তা_2, Token &Tok) ধ্রুৱ {
  Doc->setError(বাৰ্তা_2, Tok);
}

তার্কিক গ্ৰন্থি::failed() ধ্রুৱ {
  ঘূৰাই Doc->failed();
}

StringRef Scalar_গ্ৰন্থি::get_মান(SmallVectorImpl<ৱর্ণ> &Storage) ধ্রুৱ {
  যদি (মান[0] == '"')
    ঘূৰাই get_Double_Quoted_মান(মান, Storage);
  যদি (মান[0] == '\'')
    ঘূৰাই get_Single_Quoted_মান(মান, Storage);
  ঘূৰাই get_Plain_মান(মান, Storage);
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
স্থির StringRef
parse_Scalar_মান(StringRef Unquoted_মান, SmallVectorImpl<ৱর্ণ> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<ৱর্ণ> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_মান.find_first_of(LookupChars);
  যদি (I == StringRef::npos)
    ঘূৰাই Unquoted_মান;

  Storage.clear();
  Storage.reserve(Unquoted_মান.size());
  ৱর্ণ Last_New_সাৰি_Added_As = '\0';
  বাবে (; I != StringRef::npos; I = Unquoted_মান.find_first_of(LookupChars)) {
    যদি (Unquoted_মান[I] != '\r' && Unquoted_মান[I] != '\n') {
      llvm::append_range(Storage, Unquoted_মান.take_front(I));
      Unquoted_মান = UnescapeCallback(Unquoted_মান.drop_front(I), Storage);
      Last_New_সাৰি_Added_As = '\0';
      অনুৱর্ত;
    }
    যদি (size_t LastNonSWhite = Unquoted_মান.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_মান.take_front(LastNonSWhite + 1));
      Storage.ঠেলো_back(' ');
      Last_New_সাৰি_Added_As = ' ';
    } অন্যথা {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      ৱিকল্প (Last_New_সাৰি_Added_As) {
      পক্ষ ' ':
        প্রতিজ্ঞা(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        Last_New_সাৰি_Added_As = '\n';
        ভঙ্গ;
      পক্ষ '\n':
        প্রতিজ্ঞা(!Storage.empty() && Storage.back() == '\n');
        Storage.ঠেলো_back('\n');
        ভঙ্গ;
      স্ৱভাৱ:
        Storage.ঠেলো_back(' ');
        Last_New_সাৰি_Added_As = ' ';
        ভঙ্গ;
      }
    }
    // Handle Windows-style EOL
    যদি (Unquoted_মান.substr(I, 2) == "\r\n")
      I++;
    Unquoted_মান = Unquoted_মান.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_মান);
  ঘূৰাই StringRef(Storage.begin(), Storage.size());
}

StringRef
Scalar_গ্ৰন্থি::get_Double_Quoted_মান(StringRef Raw_মান,
                                 SmallVectorImpl<ৱর্ণ> &Storage) ধ্রুৱ {
  প্রতিজ্ঞা(Raw_মান.size() >= 2 && Raw_মান.front() == '"' &&
         Raw_মান.back() == '"');
  StringRef Unquoted_মান = Raw_মান.substr(1, Raw_মান.size() - 2);

  auto UnescapeFunc = [এতত্](StringRef Unquoted_মান,
                             SmallVectorImpl<ৱর্ণ> &Storage) {
    প্রতিজ্ঞা(Unquoted_মান.take_front(1) == "\\");
    যদি (Unquoted_মান.size() == 1) {
      Token T;
      T.Range = Unquoted_মান;
      setError("Unrecognized escape code", T);
      Storage.clear();
      ঘূৰাই StringRef();
    }
    Unquoted_মান = Unquoted_মান.drop_front(1);
    ৱিকল্প (Unquoted_মান[0]) {
    স্ৱভাৱ: {
      Token T;
      T.Range = Unquoted_মান.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      ঘূৰাই StringRef();
    }
    পক্ষ '\r':
      // Shrink the Windows-style EOL.
      যদি (Unquoted_মান.size() >= 2 && Unquoted_মান[1] == '\n')
        Unquoted_মান = Unquoted_মান.drop_front(1);
      [[fallthrough]];
    পক্ষ '\n':
      ঘূৰাই Unquoted_মান.drop_front(1).ltrim(" \t");
    পক্ষ '0':
      Storage.ঠেলো_back(0x00);
      ভঙ্গ;
    পক্ষ 'a':
      Storage.ঠেলো_back(0x07);
      ভঙ্গ;
    পক্ষ 'b':
      Storage.ঠেলো_back(0x08);
      ভঙ্গ;
    পক্ষ 't':
    পক্ষ 0x09:
      Storage.ঠেলো_back(0x09);
      ভঙ্গ;
    পক্ষ 'n':
      Storage.ঠেলো_back(0x0A);
      ভঙ্গ;
    পক্ষ 'v':
      Storage.ঠেলো_back(0x0B);
      ভঙ্গ;
    পক্ষ 'f':
      Storage.ঠেলো_back(0x0C);
      ভঙ্গ;
    পক্ষ 'r':
      Storage.ঠেলো_back(0x0D);
      ভঙ্গ;
    পক্ষ 'e':
      Storage.ঠেলো_back(0x1B);
      ভঙ্গ;
    পক্ষ ' ':
      Storage.ঠেলো_back(0x20);
      ভঙ্গ;
    পক্ষ '"':
      Storage.ঠেলো_back(0x22);
      ভঙ্গ;
    পক্ষ '/':
      Storage.ঠেলো_back(0x2F);
      ভঙ্গ;
    পক্ষ '\\':
      Storage.ঠেলো_back(0x5C);
      ভঙ্গ;
    পক্ষ 'N':
      encodeUTF8(0x85, Storage);
      ভঙ্গ;
    পক্ষ '_':
      encodeUTF8(0xA0, Storage);
      ভঙ্গ;
    পক্ষ 'L':
      encodeUTF8(0x2028, Storage);
      ভঙ্গ;
    পক্ষ 'P':
      encodeUTF8(0x2029, Storage);
      ভঙ্গ;
    পক্ষ 'x': {
      যদি (Unquoted_মান.size() < 3)
        // TODO: Report error.
        ভঙ্গ;
      unsigned পূর্ণাঙ্ক Unicode_Scalar_মান;
      যদি (Unquoted_মান.substr(1, 2).getAsInteger(16, Unicode_Scalar_মান))
        // TODO: Report error.
        Unicode_Scalar_মান = 0xFFFD;
      encodeUTF8(Unicode_Scalar_মান, Storage);
      ঘূৰাই Unquoted_মান.drop_front(3);
    }
    পক্ষ 'u': {
      যদি (Unquoted_মান.size() < 5)
        // TODO: Report error.
        ভঙ্গ;
      unsigned পূর্ণাঙ্ক Unicode_Scalar_মান;
      যদি (Unquoted_মান.substr(1, 4).getAsInteger(16, Unicode_Scalar_মান))
        // TODO: Report error.
        Unicode_Scalar_মান = 0xFFFD;
      encodeUTF8(Unicode_Scalar_মান, Storage);
      ঘূৰাই Unquoted_মান.drop_front(5);
    }
    পক্ষ 'U': {
      যদি (Unquoted_মান.size() < 9)
        // TODO: Report error.
        ভঙ্গ;
      unsigned পূর্ণাঙ্ক Unicode_Scalar_মান;
      যদি (Unquoted_মান.substr(1, 8).getAsInteger(16, Unicode_Scalar_মান))
        // TODO: Report error.
        Unicode_Scalar_মান = 0xFFFD;
      encodeUTF8(Unicode_Scalar_মান, Storage);
      ঘূৰাই Unquoted_মান.drop_front(9);
    }
    }
    ঘূৰাই Unquoted_মান.drop_front(1);
  };

  ঘূৰাই parse_Scalar_মান(Unquoted_মান, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_গ্ৰন্থি::get_Single_Quoted_মান(StringRef Raw_মান,
                                           SmallVectorImpl<ৱর্ণ> &Storage) {
  প্রতিজ্ঞা(Raw_মান.size() >= 2 && Raw_মান.front() == '\'' &&
         Raw_মান.back() == '\'');
  StringRef Unquoted_মান = Raw_মান.substr(1, Raw_মান.size() - 2);

  auto UnescapeFunc = [](StringRef Unquoted_মান,
                         SmallVectorImpl<ৱর্ণ> &Storage) {
    প্রতিজ্ঞা(Unquoted_মান.take_front(2) == "''");
    Storage.ঠেলো_back('\'');
    ঘূৰাই Unquoted_মান.drop_front(2);
  };

  ঘূৰাই parse_Scalar_মান(Unquoted_মান, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_গ্ৰন্থি::get_Plain_মান(StringRef Raw_মান,
                                    SmallVectorImpl<ৱর্ণ> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_মান = Raw_মান.rtrim("\r\n \t");
  ঘূৰাই parse_Scalar_মান(Raw_মান, Storage, "\r\n", nullptr);
}

গ্ৰন্থি *চাবি_মান_গ্ৰন্থি::get_চাবি() {
  যদি (চাবি)
    ঘূৰাই চাবি;
  // Handle implicit null keys.
  {
    Token &t = peekNext();
    যদি (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_মান
        || t.Kind == Token::TK_Error) {
      ঘূৰাই চাবি = নূতন (getAllocator()) Null_গ্ৰন্থি(Doc);
    }
    যদি (t.Kind == Token::TK_চাবি)
      getNext(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peekNext();
  যদি (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_মান) {
    ঘূৰাই চাবি = নূতন (getAllocator()) Null_গ্ৰন্থি(Doc);
  }

  // We've got a normal key.
  ঘূৰাই চাবি = parse_Block_গ্ৰন্থি();
}

গ্ৰন্থি *চাবি_মান_গ্ৰন্থি::get_মান() {
  যদি (মান)
    ঘূৰাই মান;

  যদি (গ্ৰন্থি* চাবি = get_চাবি())
    চাবি->skip();
  অন্যথা {
    setError("Null key in Key Value.", peekNext());
    ঘূৰাই মান = নূতন (getAllocator()) Null_গ্ৰন্থি(Doc);
  }

  যদি (failed())
    ঘূৰাই মান = নূতন (getAllocator()) Null_গ্ৰন্থি(Doc);

  // Handle implicit null values.
  {
    Token &t = peekNext();
    যদি (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_চাবি
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      ঘূৰাই মান = নূতন (getAllocator()) Null_গ্ৰন্থি(Doc);
    }

    যদি (t.Kind != Token::TK_মান) {
      setError("Unexpected token in Key Value.", t);
      ঘূৰাই মান = নূতন (getAllocator()) Null_গ্ৰন্থি(Doc);
    }
    getNext(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peekNext();
  যদি (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_চাবি) {
    ঘূৰাই মান = নূতন (getAllocator()) Null_গ্ৰন্থি(Doc);
  }

  // We got a normal value.
  ঘূৰাই মান = parse_Block_গ্ৰন্থি();
}

শূন্য Mapping_গ্ৰন্থি::increment() {
  যদি (failed()) {
    IsAtEnd = সঁচা;
    CurrentEntry = nullptr;
    ঘূৰাই;
  }
  যদি (CurrentEntry) {
    CurrentEntry->skip();
    যদি (Type == MT_Inline) {
      IsAtEnd = সঁচা;
      CurrentEntry = nullptr;
      ঘূৰাই;
    }
  }
  Token T = peekNext();
  যদি (T.Kind == Token::TK_চাবি || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = নূতন (getAllocator()) চাবি_মান_গ্ৰন্থি(Doc);
  } অন্যথা যদি (Type == MT_Block) {
    ৱিকল্প (T.Kind) {
    পক্ষ Token::TK_BlockEnd:
      getNext();
      IsAtEnd = সঁচা;
      CurrentEntry = nullptr;
      ভঙ্গ;
    স্ৱভাৱ:
      setError("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    পক্ষ Token::TK_Error:
      IsAtEnd = সঁচা;
      CurrentEntry = nullptr;
    }
  } অন্যথা {
    ৱিকল্প (T.Kind) {
    পক্ষ Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      ঘূৰাই increment();
    পক্ষ Token::TK_FlowMappingEnd:
      getNext();
      [[fallthrough]];
    পক্ষ Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = সঁচা;
      CurrentEntry = nullptr;
      ভঙ্গ;
    স্ৱভাৱ:
      setError( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      IsAtEnd = সঁচা;
      CurrentEntry = nullptr;
    }
  }
}

শূন্য Sequence_গ্ৰন্থি::increment() {
  যদি (failed()) {
    IsAtEnd = সঁচা;
    CurrentEntry = nullptr;
    ঘূৰাই;
  }
  যদি (CurrentEntry)
    CurrentEntry->skip();
  Token T = peekNext();
  যদি (SeqType == ST_Block) {
    ৱিকল্প (T.Kind) {
    পক্ষ Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_গ্ৰন্থি();
      যদি (!CurrentEntry) { // An error occurred.
        IsAtEnd = সঁচা;
        CurrentEntry = nullptr;
      }
      ভঙ্গ;
    পক্ষ Token::TK_BlockEnd:
      getNext();
      IsAtEnd = সঁচা;
      CurrentEntry = nullptr;
      ভঙ্গ;
    স্ৱভাৱ:
      setError( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    পক্ষ Token::TK_Error:
      IsAtEnd = সঁচা;
      CurrentEntry = nullptr;
    }
  } অন্যথা যদি (SeqType == ST_Indentless) {
    ৱিকল্প (T.Kind) {
    পক্ষ Token::TK_BlockEntry:
      getNext();
      CurrentEntry = parse_Block_গ্ৰন্থি();
      যদি (!CurrentEntry) { // An error occurred.
        IsAtEnd = সঁচা;
        CurrentEntry = nullptr;
      }
      ভঙ্গ;
    স্ৱভাৱ:
    পক্ষ Token::TK_Error:
      IsAtEnd = সঁচা;
      CurrentEntry = nullptr;
    }
  } অন্যথা যদি (SeqType == ST_Flow) {
    ৱিকল্প (T.Kind) {
    পক্ষ Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      getNext();
      WasPreviousTokenFlowEntry = সঁচা;
      ঘূৰাই increment();
    পক্ষ Token::TK_FlowSequenceEnd:
      getNext();
      [[fallthrough]];
    পক্ষ Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = সঁচা;
      CurrentEntry = nullptr;
      ভঙ্গ;
    পক্ষ Token::TK_StreamEnd:
    পক্ষ Token::TK_DocumentEnd:
    পক্ষ Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = সঁচা;
      CurrentEntry = nullptr;
      ভঙ্গ;
    স্ৱভাৱ:
      যদি (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = সঁচা;
        CurrentEntry = nullptr;
        ভঙ্গ;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parse_Block_গ্ৰন্থি();
      যদি (!CurrentEntry) {
        IsAtEnd = সঁচা;
      }
      WasPreviousTokenFlowEntry = মিছা;
      ভঙ্গ;
    }
  }
}

Document::Document(Stream &S) : stream(S), মূল_2(nullptr) {
  // Tag maps starts with two default mappings.
  TagMap["!"] = "!";
  TagMap["!!"] = "tag:yaml.org,2002:";

  যদি (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peekNext();
  যদি (T.Kind == Token::TK_DocumentStart)
    getNext();
}

তার্কিক Document::skip()  {
  যদি (stream.scanner->failed())
    ঘূৰাই মিছা;
  যদি (!মূল_2 && !get_মূল())
    ঘূৰাই মিছা;
  মূল_2->skip();
  Token &T = peekNext();
  যদি (T.Kind == Token::TK_StreamEnd)
    ঘূৰাই মিছা;
  যদি (T.Kind == Token::TK_DocumentEnd) {
    getNext();
    ঘূৰাই skip();
  }
  ঘূৰাই সঁচা;
}

Token &Document::peekNext() {
  ঘূৰাই stream.scanner->peekNext();
}

Token Document::getNext() {
  ঘূৰাই stream.scanner->getNext();
}

শূন্য Document::setError(ধ্রুৱ Twine &Message, Token &Location) ধ্রুৱ {
  stream.scanner->setError(Message, Location.Range.begin());
}

তার্কিক Document::failed() ধ্রুৱ {
  ঘূৰাই stream.scanner->failed();
}

গ্ৰন্থি *Document::parse_Block_গ্ৰন্থি() {
  Token T = peekNext();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  ৱিকল্প (T.Kind) {
  পক্ষ Token::TK_Alias:
    getNext();
    ঘূৰাই নূতন (গ্ৰন্থি_Allocator) Alias_গ্ৰন্থি(stream.CurrentDoc, T.Range.substr(1));
  পক্ষ Token::TK_Anchor:
    যদি (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      ঘূৰাই nullptr;
    }
    AnchorInfo = getNext(); // Consume TK_Anchor.
    T = peekNext();
    গচ্ছ parse_property;
  পক্ষ Token::TK_Tag:
    যদি (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      ঘূৰাই nullptr;
    }
    TagInfo = getNext(); // Consume TK_Tag.
    T = peekNext();
    গচ্ছ parse_property;
  স্ৱভাৱ:
    ভঙ্গ;
  }

  ৱিকল্প (T.Kind) {
  পক্ষ Token::TK_BlockEntry:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    ঘূৰাই নূতন (গ্ৰন্থি_Allocator) Sequence_গ্ৰন্থি( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , Sequence_গ্ৰন্থি::ST_Indentless);
  পক্ষ Token::TK_BlockSequenceStart:
    getNext();
    ঘূৰাই নূতন (গ্ৰন্থি_Allocator)
      Sequence_গ্ৰন্থি( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_গ্ৰন্থি::ST_Block);
  পক্ষ Token::TK_BlockMappingStart:
    getNext();
    ঘূৰাই নূতন (গ্ৰন্থি_Allocator)
      Mapping_গ্ৰন্থি( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_গ্ৰন্থি::MT_Block);
  পক্ষ Token::TK_FlowSequenceStart:
    getNext();
    ঘূৰাই নূতন (গ্ৰন্থি_Allocator)
      Sequence_গ্ৰন্থি( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_গ্ৰন্থি::ST_Flow);
  পক্ষ Token::TK_FlowMappingStart:
    getNext();
    ঘূৰাই নূতন (গ্ৰন্থি_Allocator)
      Mapping_গ্ৰন্থি( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_গ্ৰন্থি::MT_Flow);
  পক্ষ Token::TK_Scalar:
    getNext();
    ঘূৰাই নূতন (গ্ৰন্থি_Allocator)
      Scalar_গ্ৰন্থি( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  পক্ষ Token::TK_BlockScalar: {
    getNext();
    StringRef NullTerminatedStr(T.মান.c_str(), T.মান.length() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(গ্ৰন্থি_Allocator).drop_back();
    ঘূৰাই নূতন (গ্ৰন্থি_Allocator)
        Block_Scalar_গ্ৰন্থি(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  পক্ষ Token::TK_চাবি:
    // Don't eat the TK_Key, KeyValueNode expects it.
    ঘূৰাই নূতন (গ্ৰন্থি_Allocator)
      Mapping_গ্ৰন্থি( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_গ্ৰন্থি::MT_Inline);
  পক্ষ Token::TK_DocumentStart:
  পক্ষ Token::TK_DocumentEnd:
  পক্ষ Token::TK_StreamEnd:
  স্ৱভাৱ:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    ঘূৰাই নূতন (গ্ৰন্থি_Allocator) Null_গ্ৰন্থি(stream.CurrentDoc);
  পক্ষ Token::TK_FlowMappingEnd:
  পক্ষ Token::TK_FlowSequenceEnd:
  পক্ষ Token::TK_FlowEntry: {
    যদি (মূল_2 && (isa<Mapping_গ্ৰন্থি>(মূল_2) || isa<Sequence_গ্ৰন্থি>(মূল_2)))
      ঘূৰাই নূতন (গ্ৰন্থি_Allocator) Null_গ্ৰন্থি(stream.CurrentDoc);

    setError("Unexpected token", T);
    ঘূৰাই nullptr;
  }
  পক্ষ Token::TK_Error:
    ঘূৰাই nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  ঘূৰাই nullptr;
}

তার্কিক Document::parseDirectives() {
  তার্কিক isDirective = মিছা;
  যেতিয়ালৈ (সঁচা) {
    Token T = peekNext();
    যদি (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = সঁচা;
    } অন্যথা যদি (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = সঁচা;
    } অন্যথা {
      ভঙ্গ;
    }
  }
  ঘূৰাই isDirective;
}

শূন্য Document::parseYAMLDirective() {
  getNext(); // Eat %YAML <version>
}

শূন্য Document::parseTAGDirective() {
  Token Tag = getNext(); // %TAG <handle> <prefix>
  StringRef T = Tag.Range;
  // Strip %TAG
  T = T.substr(T.find_first_of(" \t")).ltrim(" \t");
  std::size_t HandleEnd = T.find_first_of(" \t");
  StringRef TagHandle = T.substr(0, HandleEnd);
  StringRef TagPrefix = T.substr(HandleEnd).ltrim(" \t");
  TagMap[TagHandle] = TagPrefix;
}

তার্কিক Document::expectToken(পূর্ণাঙ্ক TK) {
  Token T = getNext();
  যদি (T.Kind != TK) {
    setError("Unexpected token", T);
    ঘূৰাই মিছা;
  }
  ঘূৰাই সঁচা;
}
