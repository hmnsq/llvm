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

#অন্তর্ভাব "llvm/Support/YAMLParser.h"
#অন্তর্ভাব "llvm/ADT/AllocatorList.h"
#অন্তর্ভাব "llvm/ADT/ArrayRef.h"
#অন্তর্ভাব "llvm/ADT/STLExtras.h"
#অন্তর্ভাব "llvm/ADT/SmallString.h"
#অন্তর্ভাব "llvm/ADT/SmallVector.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/ADT/Twine.h"
#অন্তর্ভাব "llvm/Support/Compiler.h"
#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
#অন্তর্ভাব "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাব "llvm/Support/SMLoc.h"
#অন্তর্ভাব "llvm/Support/SourceMgr.h"
#অন্তর্ভাব "llvm/Support/Unicode.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <cassert>
#অন্তর্ভাব <cstddef>
#অন্তর্ভাব <cstdint>
#অন্তর্ভাব <map>
#অন্তর্ভাব <memory>
#অন্তর্ভাব <string>
#অন্তর্ভাব <system_error>
#অন্তর্ভাব <utility>

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
স্থির EncodingInfo getUnicodeEncoding(StringRef প্রবেশ) {
  যদি (প্রবেশ.empty())
    ফেরত {UEF_Unknown, 0};

  নির্বাচন (uint8_t(প্রবেশ[0])) {
  ক্ষেত্র 0x00:
    যদি (প্রবেশ.আকার_3() >= 4) {
      যদি (  প্রবেশ[1] == 0
         && uint8_t(প্রবেশ[2]) == 0xFE
         && uint8_t(প্রবেশ[3]) == 0xFF)
        ফেরত {UEF_UTF32_BE, 4};
      যদি (প্রবেশ[1] == 0 && প্রবেশ[2] == 0 && প্রবেশ[3] != 0)
        ফেরত {UEF_UTF32_BE, 0};
    }

    যদি (প্রবেশ.আকার_3() >= 2 && প্রবেশ[1] != 0)
      ফেরত {UEF_UTF16_BE, 0};
    ফেরত {UEF_Unknown, 0};
  ক্ষেত্র 0xFF:
    যদি (  প্রবেশ.আকার_3() >= 4
       && uint8_t(প্রবেশ[1]) == 0xFE
       && প্রবেশ[2] == 0
       && প্রবেশ[3] == 0)
      ফেরত {UEF_UTF32_LE, 4};

    যদি (প্রবেশ.আকার_3() >= 2 && uint8_t(প্রবেশ[1]) == 0xFE)
      ফেরত {UEF_UTF16_LE, 2};
    ফেরত {UEF_Unknown, 0};
  ক্ষেত্র 0xFE:
    যদি (প্রবেশ.আকার_3() >= 2 && uint8_t(প্রবেশ[1]) == 0xFF)
      ফেরত {UEF_UTF16_BE, 2};
    ফেরত {UEF_Unknown, 0};
  ক্ষেত্র 0xEF:
    যদি (  প্রবেশ.আকার_3() >= 3
       && uint8_t(প্রবেশ[1]) == 0xBB
       && uint8_t(প্রবেশ[2]) == 0xBF)
      ফেরত {UEF_UTF8, 3};
    ফেরত {UEF_Unknown, 0};
  }

  // It could still be utf-32 or utf-16.
  যদি (প্রবেশ.আকার_3() >= 4 && প্রবেশ[1] == 0 && প্রবেশ[2] == 0 && প্রবেশ[3] == 0)
    ফেরত {UEF_UTF32_LE, 0};

  যদি (প্রবেশ.আকার_3() >= 2 && প্রবেশ[1] == 0)
    ফেরত {UEF_UTF16_LE, 0};

  ফেরত {UEF_UTF8, 0};
}

/// Pin the vtables to this file.
শূন্য গ্রন্থি::anchor() {}
শূন্য Null_গ্রন্থি::anchor() {}
শূন্য Scalar_গ্রন্থি::anchor() {}
শূন্য Block_Scalar_গ্রন্থি::anchor() {}
শূন্য চাবি_মান_গ্রন্থি::anchor() {}
শূন্য Mapping_গ্রন্থি::anchor() {}
শূন্য Sequence_গ্রন্থি::anchor() {}
শূন্য Alias_গ্রন্থি::anchor() {}

নামাকাশ llvm {
নামাকাশ yaml {

/// Token - A single YAML token.
কাঠামো Token {
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

  Token() = পূর্বনির্ধারিত;
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
কাঠামো Simple_চাবি {
  TokenQueueT::iterator Tok;
  unsigned Column = 0;
  unsigned সারি = 0;
  unsigned FlowLevel = 0;
  যৌক্তিক IsRequired = মিথ্যা;

  যৌক্তিক operator ==(ধ্রুবক Simple_চাবি &Other) {
    ফেরত Tok == Other.Tok;
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
    ফেরত {*Position, 1};
  }
  // 2 bytes: [0x80, 0x7ff]
  // Bit pattern: 110xxxxx 10xxxxxx
  যদি (Position + 1 < End && ((*Position & 0xE0) == 0xC0) &&
      ((*(Position + 1) & 0xC0) == 0x80)) {
    uint32_t codepoint = ((*Position & 0x1F) << 6) |
                          (*(Position + 1) & 0x3F);
    যদি (codepoint >= 0x80)
      ফেরত {codepoint, 2};
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
      ফেরত {codepoint, 3};
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
      ফেরত {codepoint, 4};
  }
  ফেরত {0, 0};
}

নামাকাশ llvm {
নামাকাশ yaml {

/// Scans YAML tokens from a MemoryBuffer.
শ্রেণী Scanner {
সর্বজনীন:
  Scanner(StringRef প্রবেশ, SourceMgr &SM, যৌক্তিক ShowColors = সত্য,
          std::error_code *EC = nullptr);
  Scanner(Memory_বাফার_Ref বাফার, SourceMgr &SM_, যৌক্তিক ShowColors = সত্য,
          std::error_code *EC = nullptr);

  /// Parse the next token and return it without popping it.
  Token &peek_পরবর্তী();

  /// Parse the next token and pop it from the queue.
  Token get_পরবর্তী();

  শূন্য printError(SMLoc Loc, SourceMgr::DiagKind Kind, ধ্রুবক Twine &Message,
                  ArrayRef<SMRange> Ranges = {}) {
    SM.PrintMessage(Loc, Kind, Message, Ranges, /* FixIts= */ {}, ShowColors);
  }

  শূন্য setError(ধ্রুবক Twine &Message, StringRef::iterator Position) {
    যদি (Position >= End)
      Position = End - 1;

    // propagate the error if possible
    যদি (EC)
      *EC = make_error_code(std::errc::invalid_argument);

    // Don't print out more errors after the first one we encounter. The rest
    // are just the result of the first, and have no meaning.
    যদি (!Failed)
      printError(SMLoc::getFromPointer(Position), SourceMgr::DK_Error, Message);
    Failed = সত্য;
  }

  /// Returns true if an error occurred while parsing.
  যৌক্তিক failed() {
    ফেরত Failed;
  }

ব্যক্তিগত:
  শূন্য init(Memory_বাফার_Ref বাফার);

  StringRef current_প্রবেশ() {
    ফেরত StringRef(Current, End - Current);
  }

  /// Decode a UTF-8 minimal well-formed code unit subsequence starting
  ///        at \a Position.
  ///
  /// If the UTF-8 code units starting at Position do not form a well-formed
  /// code unit subsequence, then the Unicode scalar value is 0, and the length
  /// is 0.
  UTF8Decoded decodeUTF8(StringRef::iterator Position) {
    ফেরত ::decodeUTF8(StringRef(Position, End - Position));
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
  যৌক্তিক consume(uint32_t Expected);

  /// Skip \a Distance UTF-8 code units. Updates \a Cur and \a Column.
  শূন্য skip(uint32_t Distance);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is whitespace or a new line
  যৌক্তিক isBlankOrBreak(StringRef::iterator Position);

  /// Return true if the minimal well-formed code unit subsequence at
  ///        Pos is considered a "safe" character for plain scalars.
  যৌক্তিক isPlainSafeNonBlank(StringRef::iterator Position);

  /// Return true if the line is a line break, false otherwise.
  যৌক্তিক is_সারি_Empty(StringRef সারি);

  /// Consume a single b-break[28] if it's present at the current position.
  ///
  /// Return false if the code unit at the current position isn't a line break.
  যৌক্তিক consume_সারি_Break_If_Present();

  /// If IsSimpleKeyAllowed, create and push_back a new SimpleKey.
  শূন্য save_Simple_চাবি_Candidate( TokenQueueT::iterator Tok
                             , unsigned AtColumn
                             , যৌক্তিক IsRequired);

  /// Remove simple keys that can no longer be valid simple keys.
  ///
  /// Invalid simple keys are not on the current line or are further than 1024
  /// columns back.
  শূন্য remove_Stale_Simple_চাবি_Candidates();

  /// Remove all simple keys on FlowLevel \a Level.
  শূন্য remove_Simple_চাবি_Candidates_On_Flow_Level(unsigned Level);

  /// Unroll indentation in \a Indents back to \a Col. Creates BlockEnd
  ///        tokens if needed.
  যৌক্তিক unrollIndent(পূর্ণসংখ্যা ToColumn);

  /// Increase indent to \a Col. Creates \a Kind token at \a InsertPoint
  ///        if needed.
  যৌক্তিক rollIndent( পূর্ণসংখ্যা ToColumn
                 , Token::TokenKind Kind
                 , TokenQueueT::iterator ঢোকাও_Point);

  /// Skip a single-line comment when the comment starts at the current
  /// position of the scanner.
  শূন্য skipComment();

  /// Skip whitespace and comments until the start of the next token.
  শূন্য scan_To_পরবর্তী_Token();

  /// Must be the first token generated.
  যৌক্তিক scanStreamStart();

  /// Generate tokens needed to close out the stream.
  যৌক্তিক scanStreamEnd();

  /// Scan a %BLAH directive.
  যৌক্তিক scanDirective();

  /// Scan a ... or ---.
  যৌক্তিক scanDocumentIndicator(যৌক্তিক IsStart);

  /// Scan a [ or { and generate the proper flow collection start token.
  যৌক্তিক scanFlowCollectionStart(যৌক্তিক IsSequence);

  /// Scan a ] or } and generate the proper flow collection end token.
  যৌক্তিক scanFlowCollectionEnd(যৌক্তিক IsSequence);

  /// Scan the , that separates entries in a flow collection.
  যৌক্তিক scanFlowEntry();

  /// Scan the - that starts block sequence entries.
  যৌক্তিক scanBlockEntry();

  /// Scan an explicit ? indicating a key.
  যৌক্তিক scan_চাবি();

  /// Scan an explicit : indicating a value.
  যৌক্তিক scan_মান();

  /// Scan a quoted scalar.
  যৌক্তিক scanFlowScalar(যৌক্তিক IsDoubleQuoted);

  /// Scan an unquoted scalar.
  যৌক্তিক scanPlainScalar();

  /// Scan an Alias or Anchor starting with * or &.
  যৌক্তিক scanAliasOrAnchor(যৌক্তিক IsAlias);

  /// Scan a block scalar starting with | or >.
  যৌক্তিক scanBlockScalar(যৌক্তিক IsLiteral);

  /// Scan a block scalar style indicator and header.
  ///
  /// Note: This is distinct from scanBlockScalarHeader to mirror the fact that
  /// YAML does not consider the style indicator to be a part of the header.
  ///
  /// Return false if an error occurred.
  যৌক্তিক scanBlockScalarIndicators(অক্ষর &StyleIndicator, অক্ষর &ChompingIndicator,
                                 unsigned &IndentIndicator, যৌক্তিক &IsDone);

  /// Scan a style indicator in a block scalar header.
  অক্ষর scanBlockStyleIndicator();

  /// Scan a chomping indicator in a block scalar header.
  অক্ষর scanBlockChompingIndicator();

  /// Scan an indentation indicator in a block scalar header.
  unsigned scanBlockIndentationIndicator();

  /// Scan a block scalar header.
  ///
  /// Return false if an error occurred.
  যৌক্তিক scanBlockScalarHeader(অক্ষর &ChompingIndicator, unsigned &IndentIndicator,
                             যৌক্তিক &IsDone);

  /// Look for the indentation level of a block scalar.
  ///
  /// Return false if an error occurred.
  যৌক্তিক findBlockScalarIndent(unsigned &BlockIndent, unsigned BlockExitIndent,
                             unsigned &সারি_Breaks, যৌক্তিক &IsDone);

  /// Scan the indentation of a text line in a block scalar.
  ///
  /// Return false if an error occurred.
  যৌক্তিক scanBlockScalarIndent(unsigned BlockIndent, unsigned BlockExitIndent,
                             যৌক্তিক &IsDone);

  /// Scan a tag of the form !stuff.
  যৌক্তিক scanTag();

  /// Dispatch to the next scanning function based on \a *Cur.
  যৌক্তিক fetchMoreTokens();

  /// The SourceMgr used for diagnostics and buffer management.
  SourceMgr &SM;

  /// The original input.
  Memory_বাফার_Ref প্রবেশ_বাফার;

  /// The current position of the scanner.
  StringRef::iterator Current;

  /// The end of the input (one past the last character).
  StringRef::iterator End;

  /// Current YAML indentation level in spaces.
  পূর্ণসংখ্যা Indent;

  /// Current column number in Unicode code points.
  unsigned Column;

  /// Current line number.
  unsigned সারি;

  /// How deep we are in flow style containers. 0 Means at block level.
  unsigned FlowLevel;

  /// Are we at the start of the stream?
  যৌক্তিক IsStartOfStream;

  /// Can the next token be the start of a simple key?
  যৌক্তিক Is_Simple_চাবি_Allowed;

  /// Can the next token be a value indicator even if it does not have a
  /// trailing space?
  যৌক্তিক Is_Adjacent_মান_Allowed_In_Flow;

  /// True if an error has occurred.
  যৌক্তিক Failed;

  /// Should colors be used when printing out the diagnostic messages?
  যৌক্তিক ShowColors;

  /// Queue of tokens. This is required to queue up tokens while looking
  ///        for the end of a simple key. And for cases where a single character
  ///        can produce multiple tokens (e.g. BlockEnd).
  TokenQueueT TokenQueue;

  /// Indentation levels.
  SmallVector<পূর্ণসংখ্যা, 4> Indents;

  /// Potential simple keys.
  SmallVector<Simple_চাবি, 4> Simple_চাবিগুলি;

  std::error_code *EC;
};

} // end namespace yaml
} // end namespace llvm

/// encodeUTF8 - Encode \a UnicodeScalarValue in UTF-8 and append it to result.
স্থির শূন্য encodeUTF8( uint32_t Unicode_Scalar_মান
                      , SmallVectorImpl<অক্ষর> &ফলাফল) {
  যদি (Unicode_Scalar_মান <= 0x7F) {
    ফলাফল.ঠেলো_back(Unicode_Scalar_মান & 0x7F);
  } অন্যথা যদি (Unicode_Scalar_মান <= 0x7FF) {
    uint8_t FirstByte = 0xC0 | ((Unicode_Scalar_মান & 0x7C0) >> 6);
    uint8_t SecondByte = 0x80 | (Unicode_Scalar_মান & 0x3F);
    ফলাফল.ঠেলো_back(FirstByte);
    ফলাফল.ঠেলো_back(SecondByte);
  } অন্যথা যদি (Unicode_Scalar_মান <= 0xFFFF) {
    uint8_t FirstByte = 0xE0 | ((Unicode_Scalar_মান & 0xF000) >> 12);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_মান & 0xFC0) >> 6);
    uint8_t ThirdByte = 0x80 | (Unicode_Scalar_মান & 0x3F);
    ফলাফল.ঠেলো_back(FirstByte);
    ফলাফল.ঠেলো_back(SecondByte);
    ফলাফল.ঠেলো_back(ThirdByte);
  } অন্যথা যদি (Unicode_Scalar_মান <= 0x10FFFF) {
    uint8_t FirstByte = 0xF0 | ((Unicode_Scalar_মান & 0x1F0000) >> 18);
    uint8_t SecondByte = 0x80 | ((Unicode_Scalar_মান & 0x3F000) >> 12);
    uint8_t ThirdByte = 0x80 | ((Unicode_Scalar_মান & 0xFC0) >> 6);
    uint8_t FourthByte = 0x80 | (Unicode_Scalar_মান & 0x3F);
    ফলাফল.ঠেলো_back(FirstByte);
    ফলাফল.ঠেলো_back(SecondByte);
    ফলাফল.ঠেলো_back(ThirdByte);
    ফলাফল.ঠেলো_back(FourthByte);
  }
}

যৌক্তিক yaml::dumpTokens(StringRef প্রবেশ, raw_ostream &OS) {
  SourceMgr SM;
  Scanner scanner(প্রবেশ, SM);
  যখন (সত্য) {
    Token T = scanner.get_পরবর্তী();
    নির্বাচন (T.Kind) {
    ক্ষেত্র Token::TK_StreamStart:
      OS << "Stream-Start: ";
      বিরতি;
    ক্ষেত্র Token::TK_StreamEnd:
      OS << "Stream-End: ";
      বিরতি;
    ক্ষেত্র Token::TK_VersionDirective:
      OS << "Version-Directive: ";
      বিরতি;
    ক্ষেত্র Token::TK_TagDirective:
      OS << "Tag-Directive: ";
      বিরতি;
    ক্ষেত্র Token::TK_DocumentStart:
      OS << "Document-Start: ";
      বিরতি;
    ক্ষেত্র Token::TK_DocumentEnd:
      OS << "Document-End: ";
      বিরতি;
    ক্ষেত্র Token::TK_BlockEntry:
      OS << "Block-Entry: ";
      বিরতি;
    ক্ষেত্র Token::TK_BlockEnd:
      OS << "Block-End: ";
      বিরতি;
    ক্ষেত্র Token::TK_BlockSequenceStart:
      OS << "Block-Sequence-Start: ";
      বিরতি;
    ক্ষেত্র Token::TK_BlockMappingStart:
      OS << "Block-Mapping-Start: ";
      বিরতি;
    ক্ষেত্র Token::TK_FlowEntry:
      OS << "Flow-Entry: ";
      বিরতি;
    ক্ষেত্র Token::TK_FlowSequenceStart:
      OS << "Flow-Sequence-Start: ";
      বিরতি;
    ক্ষেত্র Token::TK_FlowSequenceEnd:
      OS << "Flow-Sequence-End: ";
      বিরতি;
    ক্ষেত্র Token::TK_FlowMappingStart:
      OS << "Flow-Mapping-Start: ";
      বিরতি;
    ক্ষেত্র Token::TK_FlowMappingEnd:
      OS << "Flow-Mapping-End: ";
      বিরতি;
    ক্ষেত্র Token::TK_চাবি:
      OS << "Key: ";
      বিরতি;
    ক্ষেত্র Token::TK_মান:
      OS << "Value: ";
      বিরতি;
    ক্ষেত্র Token::TK_Scalar:
      OS << "Scalar: ";
      বিরতি;
    ক্ষেত্র Token::TK_BlockScalar:
      OS << "Block Scalar: ";
      বিরতি;
    ক্ষেত্র Token::TK_Alias:
      OS << "Alias: ";
      বিরতি;
    ক্ষেত্র Token::TK_Anchor:
      OS << "Anchor: ";
      বিরতি;
    ক্ষেত্র Token::TK_Tag:
      OS << "Tag: ";
      বিরতি;
    ক্ষেত্র Token::TK_Error:
      বিরতি;
    }
    OS << T.Range << "\n";
    যদি (T.Kind == Token::TK_StreamEnd)
      বিরতি;
    অন্যথা যদি (T.Kind == Token::TK_Error)
      ফেরত মিথ্যা;
  }
  ফেরত সত্য;
}

যৌক্তিক yaml::scanTokens(StringRef প্রবেশ) {
  SourceMgr SM;
  Scanner scanner(প্রবেশ, SM);
  যখন (সত্য) {
    Token T = scanner.get_পরবর্তী();
    যদি (T.Kind == Token::TK_StreamEnd)
      বিরতি;
    অন্যথা যদি (T.Kind == Token::TK_Error)
      ফেরত মিথ্যা;
  }
  ফেরত সত্য;
}

std::string yaml::escape(StringRef প্রবেশ, যৌক্তিক EscapePrintable) {
  std::string Escaped_প্রবেশ;
  জন্য (StringRef::iterator i = প্রবেশ.begin(), e = প্রবেশ.end(); i != e; ++i) {
    যদি (*i == '\\')
      Escaped_প্রবেশ += "\\\\";
    অন্যথা যদি (*i == '"')
      Escaped_প্রবেশ += "\\\"";
    অন্যথা যদি (*i == 0)
      Escaped_প্রবেশ += "\\0";
    অন্যথা যদি (*i == 0x07)
      Escaped_প্রবেশ += "\\a";
    অন্যথা যদি (*i == 0x08)
      Escaped_প্রবেশ += "\\b";
    অন্যথা যদি (*i == 0x09)
      Escaped_প্রবেশ += "\\t";
    অন্যথা যদি (*i == 0x0A)
      Escaped_প্রবেশ += "\\n";
    অন্যথা যদি (*i == 0x0B)
      Escaped_প্রবেশ += "\\v";
    অন্যথা যদি (*i == 0x0C)
      Escaped_প্রবেশ += "\\f";
    অন্যথা যদি (*i == 0x0D)
      Escaped_প্রবেশ += "\\r";
    অন্যথা যদি (*i == 0x1B)
      Escaped_প্রবেশ += "\\e";
    অন্যথা যদি ((unsigned অক্ষর)*i < 0x20) { // Control characters not handled above.
      std::string HexStr = utohexstr(*i);
      Escaped_প্রবেশ += "\\x" + std::string(2 - HexStr.আকার_3(), '0') + HexStr;
    } অন্যথা যদি (*i & 0x80) { // UTF-8 multiple code unit subsequence.
      UTF8Decoded Unicode_Scalar_মান
        = decodeUTF8(StringRef(i, প্রবেশ.end() - i));
      যদি (Unicode_Scalar_মান.second == 0) {
        // Found invalid char.
        SmallString<4> Val;
        encodeUTF8(0xFFFD, Val);
        llvm::append_range(Escaped_প্রবেশ, Val);
        // FIXME: Error reporting.
        ফেরত Escaped_প্রবেশ;
      }
      যদি (Unicode_Scalar_মান.first == 0x85)
        Escaped_প্রবেশ += "\\N";
      অন্যথা যদি (Unicode_Scalar_মান.first == 0xA0)
        Escaped_প্রবেশ += "\\_";
      অন্যথা যদি (Unicode_Scalar_মান.first == 0x2028)
        Escaped_প্রবেশ += "\\L";
      অন্যথা যদি (Unicode_Scalar_মান.first == 0x2029)
        Escaped_প্রবেশ += "\\P";
      অন্যথা যদি (!EscapePrintable &&
               sys::unicode::isPrintable(Unicode_Scalar_মান.first))
        Escaped_প্রবেশ += StringRef(i, Unicode_Scalar_মান.second);
      অন্যথা {
        std::string HexStr = utohexstr(Unicode_Scalar_মান.first);
        যদি (HexStr.আকার_3() <= 2)
          Escaped_প্রবেশ += "\\x" + std::string(2 - HexStr.আকার_3(), '0') + HexStr;
        অন্যথা যদি (HexStr.আকার_3() <= 4)
          Escaped_প্রবেশ += "\\u" + std::string(4 - HexStr.আকার_3(), '0') + HexStr;
        অন্যথা যদি (HexStr.আকার_3() <= 8)
          Escaped_প্রবেশ += "\\U" + std::string(8 - HexStr.আকার_3(), '0') + HexStr;
      }
      i += Unicode_Scalar_মান.second - 1;
    } অন্যথা {
      Escaped_প্রবেশ.ঠেলো_back(*i);
    }
  }
  ফেরত Escaped_প্রবেশ;
}

std::optional<যৌক্তিক> yaml::parseBool(StringRef S) {
  নির্বাচন (S.আকার_3()) {
  ক্ষেত্র 1:
    নির্বাচন (S.front()) {
    ক্ষেত্র 'y':
    ক্ষেত্র 'Y':
      ফেরত সত্য;
    ক্ষেত্র 'n':
    ক্ষেত্র 'N':
      ফেরত মিথ্যা;
    পূর্বনির্ধারিত:
      ফেরত std::nullopt;
    }
  ক্ষেত্র 2:
    নির্বাচন (S.front()) {
    ক্ষেত্র 'O':
      যদি (S[1] == 'N') // ON
        ফেরত সত্য;
      [[fallthrough]];
    ক্ষেত্র 'o':
      যদি (S[1] == 'n') //[Oo]n
        ফেরত সত্য;
      ফেরত std::nullopt;
    ক্ষেত্র 'N':
      যদি (S[1] == 'O') // NO
        ফেরত মিথ্যা;
      [[fallthrough]];
    ক্ষেত্র 'n':
      যদি (S[1] == 'o') //[Nn]o
        ফেরত মিথ্যা;
      ফেরত std::nullopt;
    পূর্বনির্ধারিত:
      ফেরত std::nullopt;
    }
  ক্ষেত্র 3:
    নির্বাচন (S.front()) {
    ক্ষেত্র 'O':
      যদি (S.drop_front() == "FF") // OFF
        ফেরত মিথ্যা;
      [[fallthrough]];
    ক্ষেত্র 'o':
      যদি (S.drop_front() == "ff") //[Oo]ff
        ফেরত মিথ্যা;
      ফেরত std::nullopt;
    ক্ষেত্র 'Y':
      যদি (S.drop_front() == "ES") // YES
        ফেরত সত্য;
      [[fallthrough]];
    ক্ষেত্র 'y':
      যদি (S.drop_front() == "es") //[Yy]es
        ফেরত সত্য;
      ফেরত std::nullopt;
    পূর্বনির্ধারিত:
      ফেরত std::nullopt;
    }
  ক্ষেত্র 4:
    নির্বাচন (S.front()) {
    ক্ষেত্র 'T':
      যদি (S.drop_front() == "RUE") // TRUE
        ফেরত সত্য;
      [[fallthrough]];
    ক্ষেত্র 't':
      যদি (S.drop_front() == "rue") //[Tt]rue
        ফেরত সত্য;
      ফেরত std::nullopt;
    পূর্বনির্ধারিত:
      ফেরত std::nullopt;
    }
  ক্ষেত্র 5:
    নির্বাচন (S.front()) {
    ক্ষেত্র 'F':
      যদি (S.drop_front() == "ALSE") // FALSE
        ফেরত মিথ্যা;
      [[fallthrough]];
    ক্ষেত্র 'f':
      যদি (S.drop_front() == "alse") //[Ff]alse
        ফেরত মিথ্যা;
      ফেরত std::nullopt;
    পূর্বনির্ধারিত:
      ফেরত std::nullopt;
    }
  পূর্বনির্ধারিত:
    ফেরত std::nullopt;
  }
}

Scanner::Scanner(StringRef প্রবেশ, SourceMgr &sm, যৌক্তিক ShowColors,
                 std::error_code *EC)
    : SM(sm), ShowColors(ShowColors), EC(EC) {
  init(Memory_বাফার_Ref(প্রবেশ, "YAML"));
}

Scanner::Scanner(Memory_বাফার_Ref বাফার, SourceMgr &SM_, যৌক্তিক ShowColors,
                 std::error_code *EC)
    : SM(SM_), ShowColors(ShowColors), EC(EC) {
  init(বাফার);
}

শূন্য Scanner::init(Memory_বাফার_Ref বাফার) {
  প্রবেশ_বাফার = বাফার;
  Current = প্রবেশ_বাফার.get_বাফার_Start();
  End = প্রবেশ_বাফার.get_বাফার_End();
  Indent = -1;
  Column = 0;
  সারি = 0;
  FlowLevel = 0;
  IsStartOfStream = সত্য;
  Is_Simple_চাবি_Allowed = সত্য;
  Is_Adjacent_মান_Allowed_In_Flow = মিথ্যা;
  Failed = মিথ্যা;
  std::unique_ptr<Memory_বাফার> প্রবেশ_বাফার_Owner =
      Memory_বাফার::get_Mem_বাফার(বাফার, /*RequiresNullTerminator=*/মিথ্যা);
  SM.Add_New_Source_বাফার(std::move(প্রবেশ_বাফার_Owner), SMLoc());
}

Token &Scanner::peek_পরবর্তী() {
  // If the current token is a possible simple key, keep parsing until we
  // can confirm.
  যৌক্তিক NeedMore = মিথ্যা;
  যখন (সত্য) {
    যদি (TokenQueue.empty() || NeedMore) {
      যদি (!fetchMoreTokens()) {
        TokenQueue.clear();
        Simple_চাবিগুলি.clear();
        TokenQueue.ঠেলো_back(Token());
        ফেরত TokenQueue.front();
      }
    }
    প্রতিজ্ঞা(!TokenQueue.empty() &&
            "fetchMoreTokens lied about getting tokens!");

    remove_Stale_Simple_চাবি_Candidates();
    Simple_চাবি SK;
    SK.Tok = TokenQueue.begin();
    যদি (!is_contained(Simple_চাবিগুলি, SK))
      বিরতি;
    অন্যথা
      NeedMore = সত্য;
  }
  ফেরত TokenQueue.front();
}

Token Scanner::get_পরবর্তী() {
  Token Ret = peek_পরবর্তী();
  // TokenQueue can be empty if there was an error getting the next token.
  যদি (!TokenQueue.empty())
    TokenQueue.বের_করো_front();

  // There cannot be any referenced Token's if the TokenQueue is empty. So do a
  // quick deallocation of them all.
  যদি (TokenQueue.empty())
    TokenQueue.resetAlloc();

  ফেরত Ret;
}

StringRef::iterator Scanner::skip_nb_char(StringRef::iterator Position) {
  যদি (Position == End)
    ফেরত Position;
  // Check 7 bit c-printable - b-char.
  যদি (   *Position == 0x09
      || (*Position >= 0x20 && *Position <= 0x7E))
    ফেরত Position + 1;

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
      ফেরত Position + u8d.second;
  }
  ফেরত Position;
}

StringRef::iterator Scanner::skip_b_break(StringRef::iterator Position) {
  যদি (Position == End)
    ফেরত Position;
  যদি (*Position == 0x0D) {
    যদি (Position + 1 != End && *(Position + 1) == 0x0A)
      ফেরত Position + 2;
    ফেরত Position + 1;
  }

  যদি (*Position == 0x0A)
    ফেরত Position + 1;
  ফেরত Position;
}

StringRef::iterator Scanner::skip_s_space(StringRef::iterator Position) {
  যদি (Position == End)
    ফেরত Position;
  যদি (*Position == ' ')
    ফেরত Position + 1;
  ফেরত Position;
}

StringRef::iterator Scanner::skip_s_white(StringRef::iterator Position) {
  যদি (Position == End)
    ফেরত Position;
  যদি (*Position == ' ' || *Position == '\t')
    ফেরত Position + 1;
  ফেরত Position;
}

StringRef::iterator Scanner::skip_ns_char(StringRef::iterator Position) {
  যদি (Position == End)
    ফেরত Position;
  যদি (*Position == ' ' || *Position == '\t')
    ফেরত Position;
  ফেরত skip_nb_char(Position);
}

StringRef::iterator Scanner::skip_while( SkipWhileFunc Func
                                       , StringRef::iterator Position) {
  যখন (সত্য) {
    StringRef::iterator i = (এই->*Func)(Position);
    যদি (i == Position)
      বিরতি;
    Position = i;
  }
  ফেরত Position;
}

শূন্য Scanner::advanceWhile(SkipWhileFunc Func) {
  auto Final = skip_while(Func, Current);
  Column += Final - Current;
  Current = Final;
}

স্থির যৌক্তিক is_ns_hex_digit(ধ্রুবক অক্ষর C) { ফেরত isAlnum(C); }

স্থির যৌক্তিক is_ns_word_char(ধ্রুবক অক্ষর C) { ফেরত C == '-' || isAlpha(C); }

শূন্য Scanner::scan_ns_uri_char() {
  যখন (সত্য) {
    যদি (Current == End)
      বিরতি;
    যদি ((*Current == '%' && Current + 2 < End &&
         is_ns_hex_digit(*(Current + 1)) && is_ns_hex_digit(*(Current + 2))) ||
        is_ns_word_char(*Current) ||
        StringRef(Current, 1).find_first_of("#;/?:@&=+$,_.!~*'()[]") !=
            StringRef::npos) {
      ++Current;
      ++Column;
    } অন্যথা {
      বিরতি;
    }
  }
}

যৌক্তিক Scanner::consume(uint32_t Expected) {
  যদি (Expected >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    ফেরত মিথ্যা;
  }
  যদি (Current == End)
    ফেরত মিথ্যা;
  যদি (uint8_t(*Current) >= 0x80) {
    setError("Cannot consume non-ascii characters", Current);
    ফেরত মিথ্যা;
  }
  যদি (uint8_t(*Current) == Expected) {
    ++Current;
    ++Column;
    ফেরত সত্য;
  }
  ফেরত মিথ্যা;
}

শূন্য Scanner::skip(uint32_t Distance) {
  Current += Distance;
  Column += Distance;
  প্রতিজ্ঞা(Current <= End && "Skipped past the end");
}

যৌক্তিক Scanner::isBlankOrBreak(StringRef::iterator Position) {
  যদি (Position == End)
    ফেরত মিথ্যা;
  ফেরত *Position == ' ' || *Position == '\t' || *Position == '\r' ||
         *Position == '\n';
}

যৌক্তিক Scanner::isPlainSafeNonBlank(StringRef::iterator Position) {
  যদি (Position == End || isBlankOrBreak(Position))
    ফেরত মিথ্যা;
  যদি (FlowLevel &&
      StringRef(Position, 1).find_first_of(",[]{}") != StringRef::npos)
    ফেরত মিথ্যা;
  ফেরত সত্য;
}

যৌক্তিক Scanner::is_সারি_Empty(StringRef সারি) {
  জন্য (ধ্রুবক auto *Position = সারি.begin(); Position != সারি.end(); ++Position)
    যদি (!isBlankOrBreak(Position))
      ফেরত মিথ্যা;
  ফেরত সত্য;
}

যৌক্তিক Scanner::consume_সারি_Break_If_Present() {
  auto পরবর্তী_2 = skip_b_break(Current);
  যদি (পরবর্তী_2 == Current)
    ফেরত মিথ্যা;
  Column = 0;
  ++সারি;
  Current = পরবর্তী_2;
  ফেরত সত্য;
}

শূন্য Scanner::save_Simple_চাবি_Candidate( TokenQueueT::iterator Tok
                                    , unsigned AtColumn
                                    , যৌক্তিক IsRequired) {
  যদি (Is_Simple_চাবি_Allowed) {
    Simple_চাবি SK;
    SK.Tok = Tok;
    SK.সারি = সারি;
    SK.Column = AtColumn;
    SK.IsRequired = IsRequired;
    SK.FlowLevel = FlowLevel;
    Simple_চাবিগুলি.ঠেলো_back(SK);
  }
}

শূন্য Scanner::remove_Stale_Simple_চাবি_Candidates() {
  জন্য (SmallVectorImpl<Simple_চাবি>::iterator i = Simple_চাবিগুলি.begin();
                                            i != Simple_চাবিগুলি.end();) {
    যদি (i->সারি != সারি || i->Column + 1024 < Column) {
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
    Simple_চাবিগুলি.বের_করো_back();
}

যৌক্তিক Scanner::unrollIndent(পূর্ণসংখ্যা ToColumn) {
  Token T;
  // Indentation is ignored in flow.
  যদি (FlowLevel != 0)
    ফেরত সত্য;

  যখন (Indent > ToColumn) {
    T.Kind = Token::TK_BlockEnd;
    T.Range = StringRef(Current, 1);
    TokenQueue.ঠেলো_back(T);
    Indent = Indents.বের_করো_back_val();
  }

  ফেরত সত্য;
}

যৌক্তিক Scanner::rollIndent( পূর্ণসংখ্যা ToColumn
                        , Token::TokenKind Kind
                        , TokenQueueT::iterator ঢোকাও_Point) {
  যদি (FlowLevel)
    ফেরত সত্য;
  যদি (Indent < ToColumn) {
    Indents.ঠেলো_back(Indent);
    Indent = ToColumn;

    Token T;
    T.Kind = Kind;
    T.Range = StringRef(Current, 0);
    TokenQueue.ঢোকাও(ঢোকাও_Point, T);
  }
  ফেরত সত্য;
}

শূন্য Scanner::skipComment() {
  যদি (Current == End || *Current != '#')
    ফেরত;
  যখন (সত্য) {
    // This may skip more than one byte, thus Column is only incremented
    // for code points.
    StringRef::iterator I = skip_nb_char(Current);
    যদি (I == Current)
      বিরতি;
    Current = I;
    ++Column;
  }
}

শূন্য Scanner::scan_To_পরবর্তী_Token() {
  যখন (সত্য) {
    যখন (Current != End && (*Current == ' ' || *Current == '\t')) {
      skip(1);
    }

    skipComment();

    // Skip EOL.
    StringRef::iterator i = skip_b_break(Current);
    যদি (i == Current)
      বিরতি;
    Current = i;
    ++সারি;
    Column = 0;
    // New lines may start a simple key.
    যদি (!FlowLevel)
      Is_Simple_চাবি_Allowed = সত্য;
  }
}

যৌক্তিক Scanner::scanStreamStart() {
  IsStartOfStream = মিথ্যা;

  EncodingInfo EI = getUnicodeEncoding(current_প্রবেশ());

  Token T;
  T.Kind = Token::TK_StreamStart;
  T.Range = StringRef(Current, EI.second);
  TokenQueue.ঠেলো_back(T);
  Current += EI.second;
  ফেরত সত্য;
}

যৌক্তিক Scanner::scanStreamEnd() {
  // Force an ending new line if one isn't present.
  যদি (Column != 0) {
    Column = 0;
    ++সারি;
  }

  unrollIndent(-1);
  Simple_চাবিগুলি.clear();
  Is_Simple_চাবি_Allowed = মিথ্যা;
  Is_Adjacent_মান_Allowed_In_Flow = মিথ্যা;

  Token T;
  T.Kind = Token::TK_StreamEnd;
  T.Range = StringRef(Current, 0);
  TokenQueue.ঠেলো_back(T);
  ফেরত সত্য;
}

যৌক্তিক Scanner::scanDirective() {
  // Reset the indentation level.
  unrollIndent(-1);
  Simple_চাবিগুলি.clear();
  Is_Simple_চাবি_Allowed = মিথ্যা;
  Is_Adjacent_মান_Allowed_In_Flow = মিথ্যা;

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
    ফেরত সত্য;
  } অন্যথা যদি(Name == "TAG") {
    Current = skip_while(&Scanner::skip_ns_char, Current);
    Current = skip_while(&Scanner::skip_s_white, Current);
    Current = skip_while(&Scanner::skip_ns_char, Current);
    T.Kind = Token::TK_TagDirective;
    T.Range = StringRef(Start, Current - Start);
    TokenQueue.ঠেলো_back(T);
    ফেরত সত্য;
  }
  ফেরত মিথ্যা;
}

যৌক্তিক Scanner::scanDocumentIndicator(যৌক্তিক IsStart) {
  unrollIndent(-1);
  Simple_চাবিগুলি.clear();
  Is_Simple_চাবি_Allowed = মিথ্যা;
  Is_Adjacent_মান_Allowed_In_Flow = মিথ্যা;

  Token T;
  T.Kind = IsStart ? Token::TK_DocumentStart : Token::TK_DocumentEnd;
  T.Range = StringRef(Current, 3);
  skip(3);
  TokenQueue.ঠেলো_back(T);
  ফেরত সত্য;
}

যৌক্তিক Scanner::scanFlowCollectionStart(যৌক্তিক IsSequence) {
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceStart
                      : Token::TK_FlowMappingStart;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ঠেলো_back(T);

  // [ and { may begin a simple key.
  save_Simple_চাবি_Candidate(--TokenQueue.end(), Column - 1, মিথ্যা);

  // And may also be followed by a simple key.
  Is_Simple_চাবি_Allowed = সত্য;
  // Adjacent values are allowed in flows only after JSON-style keys.
  Is_Adjacent_মান_Allowed_In_Flow = মিথ্যা;
  ++FlowLevel;
  ফেরত সত্য;
}

যৌক্তিক Scanner::scanFlowCollectionEnd(যৌক্তিক IsSequence) {
  remove_Simple_চাবি_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_চাবি_Allowed = মিথ্যা;
  Is_Adjacent_মান_Allowed_In_Flow = সত্য;
  Token T;
  T.Kind = IsSequence ? Token::TK_FlowSequenceEnd
                      : Token::TK_FlowMappingEnd;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ঠেলো_back(T);
  যদি (FlowLevel)
    --FlowLevel;
  ফেরত সত্য;
}

যৌক্তিক Scanner::scanFlowEntry() {
  remove_Simple_চাবি_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_চাবি_Allowed = সত্য;
  Is_Adjacent_মান_Allowed_In_Flow = মিথ্যা;
  Token T;
  T.Kind = Token::TK_FlowEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ঠেলো_back(T);
  ফেরত সত্য;
}

যৌক্তিক Scanner::scanBlockEntry() {
  rollIndent(Column, Token::TK_BlockSequenceStart, TokenQueue.end());
  remove_Simple_চাবি_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_চাবি_Allowed = সত্য;
  Is_Adjacent_মান_Allowed_In_Flow = মিথ্যা;
  Token T;
  T.Kind = Token::TK_BlockEntry;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ঠেলো_back(T);
  ফেরত সত্য;
}

যৌক্তিক Scanner::scan_চাবি() {
  যদি (!FlowLevel)
    rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());

  remove_Simple_চাবি_Candidates_On_Flow_Level(FlowLevel);
  Is_Simple_চাবি_Allowed = !FlowLevel;
  Is_Adjacent_মান_Allowed_In_Flow = মিথ্যা;

  Token T;
  T.Kind = Token::TK_চাবি;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ঠেলো_back(T);
  ফেরত সত্য;
}

যৌক্তিক Scanner::scan_মান() {
  // If the previous token could have been a simple key, insert the key token
  // into the token queue.
  যদি (!Simple_চাবিগুলি.empty()) {
    Simple_চাবি SK = Simple_চাবিগুলি.বের_করো_back_val();
    Token T;
    T.Kind = Token::TK_চাবি;
    T.Range = SK.Tok->Range;
    TokenQueueT::iterator i, e;
    জন্য (i = TokenQueue.begin(), e = TokenQueue.end(); i != e; ++i) {
      যদি (i == SK.Tok)
        বিরতি;
    }
    যদি (i == e) {
      Failed = সত্য;
      ফেরত মিথ্যা;
    }
    i = TokenQueue.ঢোকাও(i, T);

    // We may also need to add a Block-Mapping-Start token.
    rollIndent(SK.Column, Token::TK_BlockMappingStart, i);

    Is_Simple_চাবি_Allowed = মিথ্যা;
  } অন্যথা {
    যদি (!FlowLevel)
      rollIndent(Column, Token::TK_BlockMappingStart, TokenQueue.end());
    Is_Simple_চাবি_Allowed = !FlowLevel;
  }
  Is_Adjacent_মান_Allowed_In_Flow = মিথ্যা;

  Token T;
  T.Kind = Token::TK_মান;
  T.Range = StringRef(Current, 1);
  skip(1);
  TokenQueue.ঠেলো_back(T);
  ফেরত সত্য;
}

// Forbidding inlining improves performance by roughly 20%.
// FIXME: Remove once llvm optimizes this to the faster version without hints.
LLVM_ATTRIBUTE_NOINLINE স্থির যৌক্তিক
wasEscaped(StringRef::iterator First, StringRef::iterator Position);

// Returns whether a character at 'Position' was escaped with a leading '\'.
// 'First' specifies the position of the first character in the string.
স্থির যৌক্তিক wasEscaped(StringRef::iterator First,
                       StringRef::iterator Position) {
  প্রতিজ্ঞা(Position - 1 >= First);
  StringRef::iterator I = Position - 1;
  // We calculate the number of consecutive '\'s before the current position
  // by iterating backwards through our string.
  যখন (I >= First && *I == '\\') --I;
  // (Position - 1 - I) now contains the number of '\'s before the current
  // position. If it is odd, the character at 'Position' was escaped.
  ফেরত (Position - 1 - I) % 2 == 1;
}

যৌক্তিক Scanner::scanFlowScalar(যৌক্তিক IsDoubleQuoted) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  যদি (IsDoubleQuoted) {
    করো {
      ++Current;
      যখন (Current != End && *Current != '"')
        ++Current;
      // Repeat until the previous character was not a '\' or was an escaped
      // backslash.
    } যখন (   Current != End
             && *(Current - 1) == '\\'
             && wasEscaped(Start + 1, Current));
  } অন্যথা {
    skip(1);
    যখন (Current != End) {
      // Skip a ' followed by another '.
      যদি (Current + 1 < End && *Current == '\'' && *(Current + 1) == '\'') {
        skip(2);
        চালু;
      } অন্যথা যদি (*Current == '\'')
        বিরতি;
      StringRef::iterator i = skip_nb_char(Current);
      যদি (i == Current) {
        i = skip_b_break(Current);
        যদি (i == Current)
          বিরতি;
        Current = i;
        Column = 0;
        ++সারি;
      } অন্যথা {
        যদি (i == End)
          বিরতি;
        Current = i;
        ++Column;
      }
    }
  }

  যদি (Current == End) {
    setError("Expected quote at end of scalar", Current);
    ফেরত মিথ্যা;
  }

  skip(1); // Skip ending quote.
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ঠেলো_back(T);

  save_Simple_চাবি_Candidate(--TokenQueue.end(), ColStart, মিথ্যা);

  Is_Simple_চাবি_Allowed = মিথ্যা;
  Is_Adjacent_মান_Allowed_In_Flow = সত্য;

  ফেরত সত্য;
}

যৌক্তিক Scanner::scanPlainScalar() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  unsigned LeadingBlanks = 0;
  প্রতিজ্ঞা(Indent >= -1 && "Indent must be >= -1 !");
  unsigned indent = static_cast<unsigned>(Indent + 1);
  যখন (Current != End) {
    যদি (*Current == '#')
      বিরতি;

    যখন (Current != End &&
           ((*Current != ':' && isPlainSafeNonBlank(Current)) ||
            (*Current == ':' && isPlainSafeNonBlank(Current + 1)))) {
      StringRef::iterator i = skip_nb_char(Current);
      যদি (i == Current)
        বিরতি;
      Current = i;
      ++Column;
    }

    // Are we at the end?
    যদি (!isBlankOrBreak(Current))
      বিরতি;

    // Eat blanks.
    StringRef::iterator Tmp = Current;
    যখন (isBlankOrBreak(Tmp)) {
      StringRef::iterator i = skip_s_white(Tmp);
      যদি (i != Tmp) {
        যদি (LeadingBlanks && (Column < indent) && *Tmp == '\t') {
          setError("Found invalid tab character in indentation", Tmp);
          ফেরত মিথ্যা;
        }
        Tmp = i;
        ++Column;
      } অন্যথা {
        i = skip_b_break(Tmp);
        যদি (!LeadingBlanks)
          LeadingBlanks = 1;
        Tmp = i;
        Column = 0;
        ++সারি;
      }
    }

    যদি (!FlowLevel && Column < indent)
      বিরতি;

    Current = Tmp;
  }
  যদি (Start == Current) {
    setError("Got empty plain scalar", Start);
    ফেরত মিথ্যা;
  }
  Token T;
  T.Kind = Token::TK_Scalar;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ঠেলো_back(T);

  // Plain scalars can be simple keys.
  save_Simple_চাবি_Candidate(--TokenQueue.end(), ColStart, মিথ্যা);

  Is_Simple_চাবি_Allowed = মিথ্যা;
  Is_Adjacent_মান_Allowed_In_Flow = মিথ্যা;

  ফেরত সত্য;
}

যৌক্তিক Scanner::scanAliasOrAnchor(যৌক্তিক IsAlias) {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1);
  যখন (Current != End) {
    যদি (   *Current == '[' || *Current == ']'
        || *Current == '{' || *Current == '}'
        || *Current == ','
        || *Current == ':')
      বিরতি;
    StringRef::iterator i = skip_ns_char(Current);
    যদি (i == Current)
      বিরতি;
    Current = i;
    ++Column;
  }

  যদি (Start + 1 == Current) {
    setError("Got empty alias or anchor", Start);
    ফেরত মিথ্যা;
  }

  Token T;
  T.Kind = IsAlias ? Token::TK_Alias : Token::TK_Anchor;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ঠেলো_back(T);

  // Alias and anchors can be simple keys.
  save_Simple_চাবি_Candidate(--TokenQueue.end(), ColStart, মিথ্যা);

  Is_Simple_চাবি_Allowed = মিথ্যা;
  Is_Adjacent_মান_Allowed_In_Flow = মিথ্যা;

  ফেরত সত্য;
}

যৌক্তিক Scanner::scanBlockScalarIndicators(অক্ষর &StyleIndicator,
                                        অক্ষর &ChompingIndicator,
                                        unsigned &IndentIndicator,
                                        যৌক্তিক &IsDone) {
  StyleIndicator = scanBlockStyleIndicator();
  যদি (!scanBlockScalarHeader(ChompingIndicator, IndentIndicator, IsDone))
    ফেরত মিথ্যা;
  ফেরত সত্য;
}

অক্ষর Scanner::scanBlockStyleIndicator() {
  অক্ষর Indicator = ' ';
  যদি (Current != End && (*Current == '>' || *Current == '|')) {
    Indicator = *Current;
    skip(1);
  }
  ফেরত Indicator;
}

অক্ষর Scanner::scanBlockChompingIndicator() {
  অক্ষর Indicator = ' ';
  যদি (Current != End && (*Current == '+' || *Current == '-')) {
    Indicator = *Current;
    skip(1);
  }
  ফেরত Indicator;
}

/// Get the number of line breaks after chomping.
///
/// Return the number of trailing line breaks to emit, depending on
/// \p ChompingIndicator.
স্থির unsigned get_Chomped_সারি_Breaks(অক্ষর ChompingIndicator,
                                     unsigned সারি_Breaks, StringRef Str) {
  যদি (ChompingIndicator == '-') // Strip all line breaks.
    ফেরত 0;
  যদি (ChompingIndicator == '+') // Keep all line breaks.
    ফেরত সারি_Breaks;
  // Clip trailing lines.
  ফেরত Str.empty() ? 0 : 1;
}

unsigned Scanner::scanBlockIndentationIndicator() {
  unsigned Indent = 0;
  যদি (Current != End && (*Current >= '1' && *Current <= '9')) {
    Indent = unsigned(*Current - '0');
    skip(1);
  }
  ফেরত Indent;
}

যৌক্তিক Scanner::scanBlockScalarHeader(অক্ষর &ChompingIndicator,
                                    unsigned &IndentIndicator, যৌক্তিক &IsDone) {
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
    IsDone = সত্য;
    ফেরত সত্য;
  }

  যদি (!consume_সারি_Break_If_Present()) {
    setError("Expected a line break after block scalar header", Current);
    ফেরত মিথ্যা;
  }
  ফেরত সত্য;
}

যৌক্তিক Scanner::findBlockScalarIndent(unsigned &BlockIndent,
                                    unsigned BlockExitIndent,
                                    unsigned &সারি_Breaks, যৌক্তিক &IsDone) {
  unsigned Max_All_Space_সারি_Characters = 0;
  StringRef::iterator Longest_All_Space_সারি;

  যখন (সত্য) {
    advanceWhile(&Scanner::skip_s_space);
    যদি (skip_nb_char(Current) != Current) {
      // This line isn't empty, so try and find the indentation.
      যদি (Column <= BlockExitIndent) { // End of the block literal.
        IsDone = সত্য;
        ফেরত সত্য;
      }
      // We found the block's indentation.
      BlockIndent = Column;
      যদি (Max_All_Space_সারি_Characters > BlockIndent) {
        setError(
            "Leading all-spaces line must be smaller than the block indent",
            Longest_All_Space_সারি);
        ফেরত মিথ্যা;
      }
      ফেরত সত্য;
    }
    যদি (skip_b_break(Current) != Current &&
        Column > Max_All_Space_সারি_Characters) {
      // Record the longest all-space line in case it's longer than the
      // discovered block indent.
      Max_All_Space_সারি_Characters = Column;
      Longest_All_Space_সারি = Current;
    }

    // Check for EOF.
    যদি (Current == End) {
      IsDone = সত্য;
      ফেরত সত্য;
    }

    যদি (!consume_সারি_Break_If_Present()) {
      IsDone = সত্য;
      ফেরত সত্য;
    }
    ++সারি_Breaks;
  }
  ফেরত সত্য;
}

যৌক্তিক Scanner::scanBlockScalarIndent(unsigned BlockIndent,
                                    unsigned BlockExitIndent, যৌক্তিক &IsDone) {
  // Skip the indentation.
  যখন (Column < BlockIndent) {
    auto I = skip_s_space(Current);
    যদি (I == Current)
      বিরতি;
    Current = I;
    ++Column;
  }

  যদি (skip_nb_char(Current) == Current)
    ফেরত সত্য;

  যদি (Column <= BlockExitIndent) { // End of the block literal.
    IsDone = সত্য;
    ফেরত সত্য;
  }

  যদি (Column < BlockIndent) {
    যদি (Current != End && *Current == '#') { // Trailing comment.
      IsDone = সত্য;
      ফেরত সত্য;
    }
    setError("A text line is less indented than the block scalar", Current);
    ফেরত মিথ্যা;
  }
  ফেরত সত্য; // A normal text line.
}

যৌক্তিক Scanner::scanBlockScalar(যৌক্তিক IsLiteral) {
  প্রতিজ্ঞা(*Current == '|' || *Current == '>');
  অক্ষর StyleIndicator;
  অক্ষর ChompingIndicator;
  unsigned BlockIndent;
  যৌক্তিক IsDone = মিথ্যা;
  যদি (!scanBlockScalarIndicators(StyleIndicator, ChompingIndicator, BlockIndent,
                                 IsDone))
    ফেরত মিথ্যা;
  যদি (IsDone)
    ফেরত সত্য;
  যৌক্তিক IsFolded = StyleIndicator == '>';

  ধ্রুবক auto *Start = Current;
  unsigned BlockExitIndent = Indent < 0 ? 0 : (unsigned)Indent;
  unsigned সারি_Breaks = 0;
  যদি (BlockIndent == 0) {
    যদি (!findBlockScalarIndent(BlockIndent, BlockExitIndent, সারি_Breaks,
                               IsDone))
      ফেরত মিথ্যা;
  }

  // Scan the block's scalars body.
  SmallString<256> Str;
  যখন (!IsDone) {
    যদি (!scanBlockScalarIndent(BlockIndent, BlockExitIndent, IsDone))
      ফেরত মিথ্যা;
    যদি (IsDone)
      বিরতি;

    // Parse the current line.
    auto সারি_Start = Current;
    advanceWhile(&Scanner::skip_nb_char);
    যদি (সারি_Start != Current) {
      যদি (সারি_Breaks && IsFolded && !Scanner::is_সারি_Empty(Str)) {
        // The folded style "folds" any single line break between content into a
        // single space, except when that content is "empty" (only contains
        // whitespace) in which case the line break is left as-is.
        যদি (সারি_Breaks == 1) {
          Str.append(সারি_Breaks,
                     is_সারি_Empty(StringRef(সারি_Start, Current - সারি_Start))
                         ? '\n'
                         : ' ');
        }
        // If we saw a single line break, we are completely replacing it and so
        // want `LineBreaks == 0`. Otherwise this decrement accounts for the
        // fact that the first line break is "trimmed", only being used to
        // signal a sequence of line breaks which should not be folded.
        সারি_Breaks--;
      }
      Str.append(সারি_Breaks, '\n');
      Str.append(StringRef(সারি_Start, Current - সারি_Start));
      সারি_Breaks = 0;
    }

    // Check for EOF.
    যদি (Current == End)
      বিরতি;

    যদি (!consume_সারি_Break_If_Present())
      বিরতি;
    ++সারি_Breaks;
  }

  যদি (Current == End && !সারি_Breaks)
    // Ensure that there is at least one line break before the end of file.
    সারি_Breaks = 1;
  Str.append(get_Chomped_সারি_Breaks(ChompingIndicator, সারি_Breaks, Str), '\n');

  // New lines may start a simple key.
  যদি (!FlowLevel)
    Is_Simple_চাবি_Allowed = সত্য;
  Is_Adjacent_মান_Allowed_In_Flow = মিথ্যা;

  Token T;
  T.Kind = Token::TK_BlockScalar;
  T.Range = StringRef(Start, Current - Start);
  T.মান = std::string(Str);
  TokenQueue.ঠেলো_back(T);
  ফেরত সত্য;
}

যৌক্তিক Scanner::scanTag() {
  StringRef::iterator Start = Current;
  unsigned ColStart = Column;
  skip(1); // Eat !.
  যদি (Current == End || isBlankOrBreak(Current)); // An empty tag.
  অন্যথা যদি (*Current == '<') {
    skip(1);
    scan_ns_uri_char();
    যদি (!consume('>'))
      ফেরত মিথ্যা;
  } অন্যথা {
    // FIXME: Actually parse the c-ns-shorthand-tag rule.
    Current = skip_while(&Scanner::skip_ns_char, Current);
  }

  Token T;
  T.Kind = Token::TK_Tag;
  T.Range = StringRef(Start, Current - Start);
  TokenQueue.ঠেলো_back(T);

  // Tags can be simple keys.
  save_Simple_চাবি_Candidate(--TokenQueue.end(), ColStart, মিথ্যা);

  Is_Simple_চাবি_Allowed = মিথ্যা;
  Is_Adjacent_মান_Allowed_In_Flow = মিথ্যা;

  ফেরত সত্য;
}

যৌক্তিক Scanner::fetchMoreTokens() {
  যদি (IsStartOfStream)
    ফেরত scanStreamStart();

  scan_To_পরবর্তী_Token();

  যদি (Current == End)
    ফেরত scanStreamEnd();

  remove_Stale_Simple_চাবি_Candidates();

  unrollIndent(Column);

  যদি (Column == 0 && *Current == '%')
    ফেরত scanDirective();

  যদি (Column == 0 && Current + 3 <= End
      && *Current == '-'
      && *(Current + 1) == '-'
      && *(Current + 2) == '-'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    ফেরত scanDocumentIndicator(সত্য);

  যদি (Column == 0 && Current + 3 <= End
      && *Current == '.'
      && *(Current + 1) == '.'
      && *(Current + 2) == '.'
      && (Current + 3 == End || isBlankOrBreak(Current + 3)))
    ফেরত scanDocumentIndicator(মিথ্যা);

  যদি (*Current == '[')
    ফেরত scanFlowCollectionStart(সত্য);

  যদি (*Current == '{')
    ফেরত scanFlowCollectionStart(মিথ্যা);

  যদি (*Current == ']')
    ফেরত scanFlowCollectionEnd(সত্য);

  যদি (*Current == '}')
    ফেরত scanFlowCollectionEnd(মিথ্যা);

  যদি (*Current == ',')
    ফেরত scanFlowEntry();

  যদি (*Current == '-' && (isBlankOrBreak(Current + 1) || Current + 1 == End))
    ফেরত scanBlockEntry();

  যদি (*Current == '?' && (Current + 1 == End || isBlankOrBreak(Current + 1)))
    ফেরত scan_চাবি();

  যদি (*Current == ':' &&
      (!isPlainSafeNonBlank(Current + 1) || Is_Adjacent_মান_Allowed_In_Flow))
    ফেরত scan_মান();

  যদি (*Current == '*')
    ফেরত scanAliasOrAnchor(সত্য);

  যদি (*Current == '&')
    ফেরত scanAliasOrAnchor(মিথ্যা);

  যদি (*Current == '!')
    ফেরত scanTag();

  যদি (*Current == '|' && !FlowLevel)
    ফেরত scanBlockScalar(সত্য);

  যদি (*Current == '>' && !FlowLevel)
    ফেরত scanBlockScalar(মিথ্যা);

  যদি (*Current == '\'')
    ফেরত scanFlowScalar(মিথ্যা);

  যদি (*Current == '"')
    ফেরত scanFlowScalar(সত্য);

  // Get a plain scalar.
  StringRef FirstChar(Current, 1);
  যদি ((!isBlankOrBreak(Current) &&
       FirstChar.find_first_of("-?:,[]{}#&*!|>'\"%@`") == StringRef::npos) ||
      (FirstChar.find_first_of("?:-") != StringRef::npos &&
       isPlainSafeNonBlank(Current + 1)))
    ফেরত scanPlainScalar();

  setError("Unrecognized character while tokenizing.", Current);
  ফেরত মিথ্যা;
}

Stream::Stream(StringRef প্রবেশ, SourceMgr &SM, যৌক্তিক ShowColors,
               std::error_code *EC)
    : scanner(নতুন Scanner(প্রবেশ, SM, ShowColors, EC)) {}

Stream::Stream(Memory_বাফার_Ref প্রবেশ_বাফার, SourceMgr &SM, যৌক্তিক ShowColors,
               std::error_code *EC)
    : scanner(নতুন Scanner(প্রবেশ_বাফার, SM, ShowColors, EC)) {}

Stream::~Stream() = পূর্বনির্ধারিত;

যৌক্তিক Stream::failed() { ফেরত scanner->failed(); }

শূন্য Stream::printError(গ্রন্থি *N, ধ্রুবক Twine &বার্তা_2, SourceMgr::DiagKind Kind) {
  printError(N ? N->getSourceRange() : SMRange(), বার্তা_2, Kind);
}

শূন্য Stream::printError(ধ্রুবক SMRange &Range, ধ্রুবক Twine &বার্তা_2,
                        SourceMgr::DiagKind Kind) {
  scanner->printError(Range.Start, Kind, বার্তা_2, Range);
}

document_iterator Stream::begin() {
  যদি (CurrentDoc)
    report_fatal_error("Can only iterate over the stream once");

  // Skip Stream-Start.
  scanner->get_পরবর্তী();

  CurrentDoc.reset(নতুন Document(*এই));
  ফেরত document_iterator(CurrentDoc);
}

document_iterator Stream::end() {
  ফেরত document_iterator();
}

শূন্য Stream::skip() {
  জন্য (Document &Doc : *এই)
    Doc.skip();
}

গ্রন্থি::গ্রন্থি(unsigned পূর্ণসংখ্যা Type, std::unique_ptr<Document> &D, StringRef A,
           StringRef T)
    : Doc(D), TypeID(Type), Anchor(A), Tag(T) {
  SMLoc Start = SMLoc::getFromPointer(peek_পরবর্তী().Range.begin());
  SourceRange = SMRange(Start, Start);
}

std::string গ্রন্থি::getVerbatimTag() ধ্রুবক {
  StringRef Raw = getRawTag();
  যদি (!Raw.empty() && Raw != "!") {
    std::string Ret;
    যদি (Raw.find_last_of('!') == 0) {
      Ret = std::string(Doc->getTagMap().find("!")->second);
      Ret += Raw.substr(1);
      ফেরত Ret;
    } অন্যথা যদি (Raw.starts_with("!!")) {
      Ret = std::string(Doc->getTagMap().find("!!")->second);
      Ret += Raw.substr(2);
      ফেরত Ret;
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
      ফেরত Ret;
    }
  }

  নির্বাচন (getType()) {
  ক্ষেত্র NK_Null:
    ফেরত "tag:yaml.org,2002:null";
  ক্ষেত্র NK_Scalar:
  ক্ষেত্র NK_BlockScalar:
    // TODO: Tag resolution.
    ফেরত "tag:yaml.org,2002:str";
  ক্ষেত্র NK_Mapping:
    ফেরত "tag:yaml.org,2002:map";
  ক্ষেত্র NK_Sequence:
    ফেরত "tag:yaml.org,2002:seq";
  }

  ফেরত "";
}

Token &গ্রন্থি::peek_পরবর্তী() {
  ফেরত Doc->peek_পরবর্তী();
}

Token গ্রন্থি::get_পরবর্তী() {
  ফেরত Doc->get_পরবর্তী();
}

গ্রন্থি *গ্রন্থি::parse_Block_গ্রন্থি() {
  ফেরত Doc->parse_Block_গ্রন্থি();
}

BumpPtrAllocator &গ্রন্থি::getAllocator() {
  ফেরত Doc->গ্রন্থি_Allocator;
}

শূন্য গ্রন্থি::setError(ধ্রুবক Twine &বার্তা_2, Token &Tok) ধ্রুবক {
  Doc->setError(বার্তা_2, Tok);
}

যৌক্তিক গ্রন্থি::failed() ধ্রুবক {
  ফেরত Doc->failed();
}

StringRef Scalar_গ্রন্থি::get_মান(SmallVectorImpl<অক্ষর> &Storage) ধ্রুবক {
  যদি (মান[0] == '"')
    ফেরত get_Double_Quoted_মান(মান, Storage);
  যদি (মান[0] == '\'')
    ফেরত get_Single_Quoted_মান(মান, Storage);
  ফেরত get_Plain_মান(মান, Storage);
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
parse_Scalar_মান(StringRef Unquoted_মান, SmallVectorImpl<অক্ষর> &Storage,
                 StringRef LookupChars,
                 std::function<StringRef(StringRef, SmallVectorImpl<অক্ষর> &)>
                     UnescapeCallback) {
  size_t I = Unquoted_মান.find_first_of(LookupChars);
  যদি (I == StringRef::npos)
    ফেরত Unquoted_মান;

  Storage.clear();
  Storage.reserve(Unquoted_মান.আকার_3());
  অক্ষর Last_New_সারি_Added_As = '\0';
  জন্য (; I != StringRef::npos; I = Unquoted_মান.find_first_of(LookupChars)) {
    যদি (Unquoted_মান[I] != '\r' && Unquoted_মান[I] != '\n') {
      llvm::append_range(Storage, Unquoted_মান.take_front(I));
      Unquoted_মান = UnescapeCallback(Unquoted_মান.drop_front(I), Storage);
      Last_New_সারি_Added_As = '\0';
      চালু;
    }
    যদি (size_t LastNonSWhite = Unquoted_মান.find_last_not_of(" \t", I);
        LastNonSWhite != StringRef::npos) {
      llvm::append_range(Storage, Unquoted_মান.take_front(LastNonSWhite + 1));
      Storage.ঠেলো_back(' ');
      Last_New_সারি_Added_As = ' ';
    } অন্যথা {
      // Note: we can't just check if the last character in Storage is ' ',
      // '\n', or something else; that would give a wrong result for double
      // quoted values containing an escaped space character before a new-line
      // character.
      নির্বাচন (Last_New_সারি_Added_As) {
      ক্ষেত্র ' ':
        প্রতিজ্ঞা(!Storage.empty() && Storage.back() == ' ');
        Storage.back() = '\n';
        Last_New_সারি_Added_As = '\n';
        বিরতি;
      ক্ষেত্র '\n':
        প্রতিজ্ঞা(!Storage.empty() && Storage.back() == '\n');
        Storage.ঠেলো_back('\n');
        বিরতি;
      পূর্বনির্ধারিত:
        Storage.ঠেলো_back(' ');
        Last_New_সারি_Added_As = ' ';
        বিরতি;
      }
    }
    // Handle Windows-style EOL
    যদি (Unquoted_মান.substr(I, 2) == "\r\n")
      I++;
    Unquoted_মান = Unquoted_মান.drop_front(I + 1).ltrim(" \t");
  }
  llvm::append_range(Storage, Unquoted_মান);
  ফেরত StringRef(Storage.begin(), Storage.আকার_3());
}

StringRef
Scalar_গ্রন্থি::get_Double_Quoted_মান(StringRef Raw_মান,
                                 SmallVectorImpl<অক্ষর> &Storage) ধ্রুবক {
  প্রতিজ্ঞা(Raw_মান.আকার_3() >= 2 && Raw_মান.front() == '"' &&
         Raw_মান.back() == '"');
  StringRef Unquoted_মান = Raw_মান.substr(1, Raw_মান.আকার_3() - 2);

  auto UnescapeFunc = [এই](StringRef Unquoted_মান,
                             SmallVectorImpl<অক্ষর> &Storage) {
    প্রতিজ্ঞা(Unquoted_মান.take_front(1) == "\\");
    যদি (Unquoted_মান.আকার_3() == 1) {
      Token T;
      T.Range = Unquoted_মান;
      setError("Unrecognized escape code", T);
      Storage.clear();
      ফেরত StringRef();
    }
    Unquoted_মান = Unquoted_মান.drop_front(1);
    নির্বাচন (Unquoted_মান[0]) {
    পূর্বনির্ধারিত: {
      Token T;
      T.Range = Unquoted_মান.take_front(1);
      setError("Unrecognized escape code", T);
      Storage.clear();
      ফেরত StringRef();
    }
    ক্ষেত্র '\r':
      // Shrink the Windows-style EOL.
      যদি (Unquoted_মান.আকার_3() >= 2 && Unquoted_মান[1] == '\n')
        Unquoted_মান = Unquoted_মান.drop_front(1);
      [[fallthrough]];
    ক্ষেত্র '\n':
      ফেরত Unquoted_মান.drop_front(1).ltrim(" \t");
    ক্ষেত্র '0':
      Storage.ঠেলো_back(0x00);
      বিরতি;
    ক্ষেত্র 'a':
      Storage.ঠেলো_back(0x07);
      বিরতি;
    ক্ষেত্র 'b':
      Storage.ঠেলো_back(0x08);
      বিরতি;
    ক্ষেত্র 't':
    ক্ষেত্র 0x09:
      Storage.ঠেলো_back(0x09);
      বিরতি;
    ক্ষেত্র 'n':
      Storage.ঠেলো_back(0x0A);
      বিরতি;
    ক্ষেত্র 'v':
      Storage.ঠেলো_back(0x0B);
      বিরতি;
    ক্ষেত্র 'f':
      Storage.ঠেলো_back(0x0C);
      বিরতি;
    ক্ষেত্র 'r':
      Storage.ঠেলো_back(0x0D);
      বিরতি;
    ক্ষেত্র 'e':
      Storage.ঠেলো_back(0x1B);
      বিরতি;
    ক্ষেত্র ' ':
      Storage.ঠেলো_back(0x20);
      বিরতি;
    ক্ষেত্র '"':
      Storage.ঠেলো_back(0x22);
      বিরতি;
    ক্ষেত্র '/':
      Storage.ঠেলো_back(0x2F);
      বিরতি;
    ক্ষেত্র '\\':
      Storage.ঠেলো_back(0x5C);
      বিরতি;
    ক্ষেত্র 'N':
      encodeUTF8(0x85, Storage);
      বিরতি;
    ক্ষেত্র '_':
      encodeUTF8(0xA0, Storage);
      বিরতি;
    ক্ষেত্র 'L':
      encodeUTF8(0x2028, Storage);
      বিরতি;
    ক্ষেত্র 'P':
      encodeUTF8(0x2029, Storage);
      বিরতি;
    ক্ষেত্র 'x': {
      যদি (Unquoted_মান.আকার_3() < 3)
        // TODO: Report error.
        বিরতি;
      unsigned পূর্ণসংখ্যা Unicode_Scalar_মান;
      যদি (Unquoted_মান.substr(1, 2).getAsInteger(16, Unicode_Scalar_মান))
        // TODO: Report error.
        Unicode_Scalar_মান = 0xFFFD;
      encodeUTF8(Unicode_Scalar_মান, Storage);
      ফেরত Unquoted_মান.drop_front(3);
    }
    ক্ষেত্র 'u': {
      যদি (Unquoted_মান.আকার_3() < 5)
        // TODO: Report error.
        বিরতি;
      unsigned পূর্ণসংখ্যা Unicode_Scalar_মান;
      যদি (Unquoted_মান.substr(1, 4).getAsInteger(16, Unicode_Scalar_মান))
        // TODO: Report error.
        Unicode_Scalar_মান = 0xFFFD;
      encodeUTF8(Unicode_Scalar_মান, Storage);
      ফেরত Unquoted_মান.drop_front(5);
    }
    ক্ষেত্র 'U': {
      যদি (Unquoted_মান.আকার_3() < 9)
        // TODO: Report error.
        বিরতি;
      unsigned পূর্ণসংখ্যা Unicode_Scalar_মান;
      যদি (Unquoted_মান.substr(1, 8).getAsInteger(16, Unicode_Scalar_মান))
        // TODO: Report error.
        Unicode_Scalar_মান = 0xFFFD;
      encodeUTF8(Unicode_Scalar_মান, Storage);
      ফেরত Unquoted_মান.drop_front(9);
    }
    }
    ফেরত Unquoted_মান.drop_front(1);
  };

  ফেরত parse_Scalar_মান(Unquoted_মান, Storage, "\\\r\n", UnescapeFunc);
}

StringRef Scalar_গ্রন্থি::get_Single_Quoted_মান(StringRef Raw_মান,
                                           SmallVectorImpl<অক্ষর> &Storage) {
  প্রতিজ্ঞা(Raw_মান.আকার_3() >= 2 && Raw_মান.front() == '\'' &&
         Raw_মান.back() == '\'');
  StringRef Unquoted_মান = Raw_মান.substr(1, Raw_মান.আকার_3() - 2);

  auto UnescapeFunc = [](StringRef Unquoted_মান,
                         SmallVectorImpl<অক্ষর> &Storage) {
    প্রতিজ্ঞা(Unquoted_মান.take_front(2) == "''");
    Storage.ঠেলো_back('\'');
    ফেরত Unquoted_মান.drop_front(2);
  };

  ফেরত parse_Scalar_মান(Unquoted_মান, Storage, "'\r\n", UnescapeFunc);
}

StringRef Scalar_গ্রন্থি::get_Plain_মান(StringRef Raw_মান,
                                    SmallVectorImpl<অক্ষর> &Storage) {
  // Trim trailing whitespace ('b-char' and 's-white').
  // NOTE: Alternatively we could change the scanner to not include whitespace
  //       here in the first place.
  Raw_মান = Raw_মান.rtrim("\r\n \t");
  ফেরত parse_Scalar_মান(Raw_মান, Storage, "\r\n", nullptr);
}

গ্রন্থি *চাবি_মান_গ্রন্থি::get_চাবি() {
  যদি (চাবি)
    ফেরত চাবি;
  // Handle implicit null keys.
  {
    Token &t = peek_পরবর্তী();
    যদি (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_মান
        || t.Kind == Token::TK_Error) {
      ফেরত চাবি = নতুন (getAllocator()) Null_গ্রন্থি(Doc);
    }
    যদি (t.Kind == Token::TK_চাবি)
      get_পরবর্তী(); // skip TK_Key.
  }

  // Handle explicit null keys.
  Token &t = peek_পরবর্তী();
  যদি (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_মান) {
    ফেরত চাবি = নতুন (getAllocator()) Null_গ্রন্থি(Doc);
  }

  // We've got a normal key.
  ফেরত চাবি = parse_Block_গ্রন্থি();
}

গ্রন্থি *চাবি_মান_গ্রন্থি::get_মান() {
  যদি (মান)
    ফেরত মান;

  যদি (গ্রন্থি* চাবি = get_চাবি())
    চাবি->skip();
  অন্যথা {
    setError("Null key in Key Value.", peek_পরবর্তী());
    ফেরত মান = নতুন (getAllocator()) Null_গ্রন্থি(Doc);
  }

  যদি (failed())
    ফেরত মান = নতুন (getAllocator()) Null_গ্রন্থি(Doc);

  // Handle implicit null values.
  {
    Token &t = peek_পরবর্তী();
    যদি (   t.Kind == Token::TK_BlockEnd
        || t.Kind == Token::TK_FlowMappingEnd
        || t.Kind == Token::TK_চাবি
        || t.Kind == Token::TK_FlowEntry
        || t.Kind == Token::TK_Error) {
      ফেরত মান = নতুন (getAllocator()) Null_গ্রন্থি(Doc);
    }

    যদি (t.Kind != Token::TK_মান) {
      setError("Unexpected token in Key Value.", t);
      ফেরত মান = নতুন (getAllocator()) Null_গ্রন্থি(Doc);
    }
    get_পরবর্তী(); // skip TK_Value.
  }

  // Handle explicit null values.
  Token &t = peek_পরবর্তী();
  যদি (t.Kind == Token::TK_BlockEnd || t.Kind == Token::TK_চাবি) {
    ফেরত মান = নতুন (getAllocator()) Null_গ্রন্থি(Doc);
  }

  // We got a normal value.
  ফেরত মান = parse_Block_গ্রন্থি();
}

শূন্য Mapping_গ্রন্থি::increment() {
  যদি (failed()) {
    IsAtEnd = সত্য;
    CurrentEntry = nullptr;
    ফেরত;
  }
  যদি (CurrentEntry) {
    CurrentEntry->skip();
    যদি (Type == MT_Inline) {
      IsAtEnd = সত্য;
      CurrentEntry = nullptr;
      ফেরত;
    }
  }
  Token T = peek_পরবর্তী();
  যদি (T.Kind == Token::TK_চাবি || T.Kind == Token::TK_Scalar) {
    // KeyValueNode eats the TK_Key. That way it can detect null keys.
    CurrentEntry = নতুন (getAllocator()) চাবি_মান_গ্রন্থি(Doc);
  } অন্যথা যদি (Type == MT_Block) {
    নির্বাচন (T.Kind) {
    ক্ষেত্র Token::TK_BlockEnd:
      get_পরবর্তী();
      IsAtEnd = সত্য;
      CurrentEntry = nullptr;
      বিরতি;
    পূর্বনির্ধারিত:
      setError("Unexpected token. Expected Key or Block End", T);
      [[fallthrough]];
    ক্ষেত্র Token::TK_Error:
      IsAtEnd = সত্য;
      CurrentEntry = nullptr;
    }
  } অন্যথা {
    নির্বাচন (T.Kind) {
    ক্ষেত্র Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      get_পরবর্তী();
      ফেরত increment();
    ক্ষেত্র Token::TK_FlowMappingEnd:
      get_পরবর্তী();
      [[fallthrough]];
    ক্ষেত্র Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = সত্য;
      CurrentEntry = nullptr;
      বিরতি;
    পূর্বনির্ধারিত:
      setError( "Unexpected token. Expected Key, Flow Entry, or Flow "
                "Mapping End."
              , T);
      IsAtEnd = সত্য;
      CurrentEntry = nullptr;
    }
  }
}

শূন্য Sequence_গ্রন্থি::increment() {
  যদি (failed()) {
    IsAtEnd = সত্য;
    CurrentEntry = nullptr;
    ফেরত;
  }
  যদি (CurrentEntry)
    CurrentEntry->skip();
  Token T = peek_পরবর্তী();
  যদি (SeqType == ST_Block) {
    নির্বাচন (T.Kind) {
    ক্ষেত্র Token::TK_BlockEntry:
      get_পরবর্তী();
      CurrentEntry = parse_Block_গ্রন্থি();
      যদি (!CurrentEntry) { // An error occurred.
        IsAtEnd = সত্য;
        CurrentEntry = nullptr;
      }
      বিরতি;
    ক্ষেত্র Token::TK_BlockEnd:
      get_পরবর্তী();
      IsAtEnd = সত্য;
      CurrentEntry = nullptr;
      বিরতি;
    পূর্বনির্ধারিত:
      setError( "Unexpected token. Expected Block Entry or Block End."
              , T);
      [[fallthrough]];
    ক্ষেত্র Token::TK_Error:
      IsAtEnd = সত্য;
      CurrentEntry = nullptr;
    }
  } অন্যথা যদি (SeqType == ST_Indentless) {
    নির্বাচন (T.Kind) {
    ক্ষেত্র Token::TK_BlockEntry:
      get_পরবর্তী();
      CurrentEntry = parse_Block_গ্রন্থি();
      যদি (!CurrentEntry) { // An error occurred.
        IsAtEnd = সত্য;
        CurrentEntry = nullptr;
      }
      বিরতি;
    পূর্বনির্ধারিত:
    ক্ষেত্র Token::TK_Error:
      IsAtEnd = সত্য;
      CurrentEntry = nullptr;
    }
  } অন্যথা যদি (SeqType == ST_Flow) {
    নির্বাচন (T.Kind) {
    ক্ষেত্র Token::TK_FlowEntry:
      // Eat the flow entry and recurse.
      get_পরবর্তী();
      WasPreviousTokenFlowEntry = সত্য;
      ফেরত increment();
    ক্ষেত্র Token::TK_FlowSequenceEnd:
      get_পরবর্তী();
      [[fallthrough]];
    ক্ষেত্র Token::TK_Error:
      // Set this to end iterator.
      IsAtEnd = সত্য;
      CurrentEntry = nullptr;
      বিরতি;
    ক্ষেত্র Token::TK_StreamEnd:
    ক্ষেত্র Token::TK_DocumentEnd:
    ক্ষেত্র Token::TK_DocumentStart:
      setError("Could not find closing ]!", T);
      // Set this to end iterator.
      IsAtEnd = সত্য;
      CurrentEntry = nullptr;
      বিরতি;
    পূর্বনির্ধারিত:
      যদি (!WasPreviousTokenFlowEntry) {
        setError("Expected , between entries!", T);
        IsAtEnd = সত্য;
        CurrentEntry = nullptr;
        বিরতি;
      }
      // Otherwise it must be a flow entry.
      CurrentEntry = parse_Block_গ্রন্থি();
      যদি (!CurrentEntry) {
        IsAtEnd = সত্য;
      }
      WasPreviousTokenFlowEntry = মিথ্যা;
      বিরতি;
    }
  }
}

Document::Document(Stream &S) : stream(S), মূল_2(nullptr) {
  // Tag maps starts with two default mappings.
  TagMap["!"] = "!";
  TagMap["!!"] = "tag:yaml.org,2002:";

  যদি (parseDirectives())
    expectToken(Token::TK_DocumentStart);
  Token &T = peek_পরবর্তী();
  যদি (T.Kind == Token::TK_DocumentStart)
    get_পরবর্তী();
}

যৌক্তিক Document::skip()  {
  যদি (stream.scanner->failed())
    ফেরত মিথ্যা;
  যদি (!মূল_2 && !get_মূল())
    ফেরত মিথ্যা;
  মূল_2->skip();
  Token &T = peek_পরবর্তী();
  যদি (T.Kind == Token::TK_StreamEnd)
    ফেরত মিথ্যা;
  যদি (T.Kind == Token::TK_DocumentEnd) {
    get_পরবর্তী();
    ফেরত skip();
  }
  ফেরত সত্য;
}

Token &Document::peek_পরবর্তী() {
  ফেরত stream.scanner->peek_পরবর্তী();
}

Token Document::get_পরবর্তী() {
  ফেরত stream.scanner->get_পরবর্তী();
}

শূন্য Document::setError(ধ্রুবক Twine &Message, Token &Location) ধ্রুবক {
  stream.scanner->setError(Message, Location.Range.begin());
}

যৌক্তিক Document::failed() ধ্রুবক {
  ফেরত stream.scanner->failed();
}

গ্রন্থি *Document::parse_Block_গ্রন্থি() {
  Token T = peek_পরবর্তী();
  // Handle properties.
  Token AnchorInfo;
  Token TagInfo;
parse_property:
  নির্বাচন (T.Kind) {
  ক্ষেত্র Token::TK_Alias:
    get_পরবর্তী();
    ফেরত নতুন (গ্রন্থি_Allocator) Alias_গ্রন্থি(stream.CurrentDoc, T.Range.substr(1));
  ক্ষেত্র Token::TK_Anchor:
    যদি (AnchorInfo.Kind == Token::TK_Anchor) {
      setError("Already encountered an anchor for this node!", T);
      ফেরত nullptr;
    }
    AnchorInfo = get_পরবর্তী(); // Consume TK_Anchor.
    T = peek_পরবর্তী();
    যাও parse_property;
  ক্ষেত্র Token::TK_Tag:
    যদি (TagInfo.Kind == Token::TK_Tag) {
      setError("Already encountered a tag for this node!", T);
      ফেরত nullptr;
    }
    TagInfo = get_পরবর্তী(); // Consume TK_Tag.
    T = peek_পরবর্তী();
    যাও parse_property;
  পূর্বনির্ধারিত:
    বিরতি;
  }

  নির্বাচন (T.Kind) {
  ক্ষেত্র Token::TK_BlockEntry:
    // We got an unindented BlockEntry sequence. This is not terminated with
    // a BlockEnd.
    // Don't eat the TK_BlockEntry, SequenceNode needs it.
    ফেরত নতুন (গ্রন্থি_Allocator) Sequence_গ্রন্থি( stream.CurrentDoc
                                           , AnchorInfo.Range.substr(1)
                                           , TagInfo.Range
                                           , Sequence_গ্রন্থি::ST_Indentless);
  ক্ষেত্র Token::TK_BlockSequenceStart:
    get_পরবর্তী();
    ফেরত নতুন (গ্রন্থি_Allocator)
      Sequence_গ্রন্থি( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_গ্রন্থি::ST_Block);
  ক্ষেত্র Token::TK_BlockMappingStart:
    get_পরবর্তী();
    ফেরত নতুন (গ্রন্থি_Allocator)
      Mapping_গ্রন্থি( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_গ্রন্থি::MT_Block);
  ক্ষেত্র Token::TK_FlowSequenceStart:
    get_পরবর্তী();
    ফেরত নতুন (গ্রন্থি_Allocator)
      Sequence_গ্রন্থি( stream.CurrentDoc
                  , AnchorInfo.Range.substr(1)
                  , TagInfo.Range
                  , Sequence_গ্রন্থি::ST_Flow);
  ক্ষেত্র Token::TK_FlowMappingStart:
    get_পরবর্তী();
    ফেরত নতুন (গ্রন্থি_Allocator)
      Mapping_গ্রন্থি( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_গ্রন্থি::MT_Flow);
  ক্ষেত্র Token::TK_Scalar:
    get_পরবর্তী();
    ফেরত নতুন (গ্রন্থি_Allocator)
      Scalar_গ্রন্থি( stream.CurrentDoc
                , AnchorInfo.Range.substr(1)
                , TagInfo.Range
                , T.Range);
  ক্ষেত্র Token::TK_BlockScalar: {
    get_পরবর্তী();
    StringRef NullTerminatedStr(T.মান.c_str(), T.মান.দৈর্ঘ্য_2() + 1);
    StringRef StrCopy = NullTerminatedStr.copy(গ্রন্থি_Allocator).drop_back();
    ফেরত নতুন (গ্রন্থি_Allocator)
        Block_Scalar_গ্রন্থি(stream.CurrentDoc, AnchorInfo.Range.substr(1),
                        TagInfo.Range, StrCopy, T.Range);
  }
  ক্ষেত্র Token::TK_চাবি:
    // Don't eat the TK_Key, KeyValueNode expects it.
    ফেরত নতুন (গ্রন্থি_Allocator)
      Mapping_গ্রন্থি( stream.CurrentDoc
                 , AnchorInfo.Range.substr(1)
                 , TagInfo.Range
                 , Mapping_গ্রন্থি::MT_Inline);
  ক্ষেত্র Token::TK_DocumentStart:
  ক্ষেত্র Token::TK_DocumentEnd:
  ক্ষেত্র Token::TK_StreamEnd:
  পূর্বনির্ধারিত:
    // TODO: Properly handle tags. "[!!str ]" should resolve to !!str "", not
    //       !!null null.
    ফেরত নতুন (গ্রন্থি_Allocator) Null_গ্রন্থি(stream.CurrentDoc);
  ক্ষেত্র Token::TK_FlowMappingEnd:
  ক্ষেত্র Token::TK_FlowSequenceEnd:
  ক্ষেত্র Token::TK_FlowEntry: {
    যদি (মূল_2 && (isa<Mapping_গ্রন্থি>(মূল_2) || isa<Sequence_গ্রন্থি>(মূল_2)))
      ফেরত নতুন (গ্রন্থি_Allocator) Null_গ্রন্থি(stream.CurrentDoc);

    setError("Unexpected token", T);
    ফেরত nullptr;
  }
  ক্ষেত্র Token::TK_Error:
    ফেরত nullptr;
  }
  llvm_unreachable("Control flow shouldn't reach here.");
  ফেরত nullptr;
}

যৌক্তিক Document::parseDirectives() {
  যৌক্তিক isDirective = মিথ্যা;
  যখন (সত্য) {
    Token T = peek_পরবর্তী();
    যদি (T.Kind == Token::TK_TagDirective) {
      parseTAGDirective();
      isDirective = সত্য;
    } অন্যথা যদি (T.Kind == Token::TK_VersionDirective) {
      parseYAMLDirective();
      isDirective = সত্য;
    } অন্যথা {
      বিরতি;
    }
  }
  ফেরত isDirective;
}

শূন্য Document::parseYAMLDirective() {
  get_পরবর্তী(); // Eat %YAML <version>
}

শূন্য Document::parseTAGDirective() {
  Token Tag = get_পরবর্তী(); // %TAG <handle> <prefix>
  StringRef T = Tag.Range;
  // Strip %TAG
  T = T.substr(T.find_first_of(" \t")).ltrim(" \t");
  std::size_t HandleEnd = T.find_first_of(" \t");
  StringRef TagHandle = T.substr(0, HandleEnd);
  StringRef TagPrefix = T.substr(HandleEnd).ltrim(" \t");
  TagMap[TagHandle] = TagPrefix;
}

যৌক্তিক Document::expectToken(পূর্ণসংখ্যা TK) {
  Token T = get_পরবর্তী();
  যদি (T.Kind != TK) {
    setError("Unexpected token", T);
    ফেরত মিথ্যা;
  }
  ফেরত সত্য;
}
