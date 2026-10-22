//===- llvm/Support/Unicode.cpp - Unicode character properties  -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements functions that allow querying certain properties of
// Unicode characters.
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Unicode.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ConvertUTF.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/UnicodeCharRanges.h"

ନାମାକାଶ llvm {
ନାମାକାଶ sys {
ନାମାକାଶ unicode {

ବାହ୍ୟ ଅଚଳ UnicodeCharSet::CharRanges GeneratedCharacterRanges;
ବାହ୍ୟ ଅଚଳ UnicodeCharSet::CharRanges GeneratedPrintableRanges;
ବାହ୍ୟ ଅଚଳ UnicodeCharSet::CharRanges GeneratedControlCharacterRanges;
ବାହ୍ୟ ଅଚଳ UnicodeCharSet::CharRanges GeneratedFormatCharacterRanges;
ବାହ୍ୟ ଅଚଳ UnicodeCharSet::CharRanges GeneratedCombiningCharacterRanges;
ବାହ୍ୟ ଅଚଳ UnicodeCharSet::CharRanges GeneratedDoubleWidthCharacterRanges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
ତାର୍କିକ isCharacter(ପୂର୍ଣ୍ଣସଂଖ୍ୟା UCS) {
  ସ୍ଥିର ଅଚଳ UnicodeCharSet Character(GeneratedCharacterRanges);
  ଫେରନ୍ତୁ Character.contains(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
ତାର୍କିକ isPrintable(ପୂର୍ଣ୍ଣସଂଖ୍ୟା UCS) {
  ସ୍ଥିର ଅଚଳ UnicodeCharSet Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  ଫେରନ୍ତୁ UCS == 0x00AD || Printables.contains(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
ତାର୍କିକ isControl(ପୂର୍ଣ୍ଣସଂଖ୍ୟା UCS) {
  ସ୍ଥିର ଅଚଳ UnicodeCharSet Control(GeneratedControlCharacterRanges);
  ଫେରନ୍ତୁ Control.contains(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
ତାର୍କିକ isFormatting(ପୂର୍ଣ୍ଣସଂଖ୍ୟା UCS) {
  ସ୍ଥିର ଅଚଳ UnicodeCharSet Format(GeneratedFormatCharacterRanges);
  ଫେରନ୍ତୁ Format.contains(UCS);
}

/// Gets the number of positions a character is likely to occupy when output
/// on a terminal ("character width"). This depends on the implementation of the
/// terminal, and there's no standard definition of character width.
/// The implementation defines it in a way that is expected to be compatible
/// with a generic Unicode-capable terminal.
/// \return Character width:
///   * ErrorNonPrintableCharacter (-1) for non-printable characters (as
///     identified by isPrintable);
///   * 0 for non-spacing and enclosing combining marks;
///   * 2 for CJK characters excluding halfwidth forms;
///   * 1 for all remaining characters.
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା charWidth(ପୂର୍ଣ୍ଣସଂଖ୍ୟା UCS) {
  ଯଦି (!isPrintable(UCS))
    ଫେରନ୍ତୁ ErrorNonPrintableCharacter;

  ସ୍ଥିର ଅଚଳ UnicodeCharSet CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  ଯଦି (CombiningCharacters.contains(UCS))
    ଫେରନ୍ତୁ 0;

  ସ୍ଥିର ଅଚଳ UnicodeCharSet DoubleWidthCharacters(
      GeneratedDoubleWidthCharacterRanges);

  ଯଦି (DoubleWidthCharacters.contains(UCS))
    ଫେରନ୍ତୁ 2;
  ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ତାର୍କିକ isprintableascii(ଅକ୍ଷର c) { ଫେରନ୍ତୁ c > 31 && c < 127; }

ପୂର୍ଣ୍ଣସଂଖ୍ୟା columnWidthUTF8(StringRef Text) {
  unsigned ColumnWidth = 0;
  unsigned Length;
  ପାଇଁ (size_t i = 0, e = Text.size(); i < e; i += Length) {
    Length = get_ଅଙ୍କ_Bytes_For_UTF8(Text[i]);

    // fast path for ASCII characters
    ଯଦି (Length == 1) {
      ଯଦି (!isprintableascii(Text[i]))
        ଫେରନ୍ତୁ ErrorNonPrintableCharacter;
      ColumnWidth += 1;
      ଜାରି;
    }

    ଯଦି (Length <= 0 || i + Length > Text.size())
      ଫେରନ୍ତୁ ErrorInvalidUTF8;
    UTF32 buf[1];
    ଅଚଳ UTF8 *Start = reinterpret_cast<ଅଚଳ UTF8 *>(Text.data() + i);
    UTF32 *Target = &buf[0];
    ଯଦି (conversionOK != ConvertUTF8toUTF32(&Start, Start + Length, &Target,
                                           Target + 1, strictConversion))
      ଫେରନ୍ତୁ ErrorInvalidUTF8;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା Width = charWidth(buf[0]);
    ଯଦି (Width < 0)
      ଫେରନ୍ତୁ ErrorNonPrintableCharacter;
    ColumnWidth += Width;
  }
  ଫେରନ୍ତୁ ColumnWidth;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
