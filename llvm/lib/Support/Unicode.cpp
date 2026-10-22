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

#অন্তর্ভাব "llvm/Support/Unicode.h"
#অন্তর্ভাব "llvm/Support/ConvertUTF.h"
#অন্তর্ভাব "llvm/Support/UnicodeCharRanges.h"

নামাকাশ llvm {
নামাকাশ sys {
নামাকাশ unicode {

বাহ্যিক ধ্রুবক UnicodeCharSet::CharRanges GeneratedCharacterRanges;
বাহ্যিক ধ্রুবক UnicodeCharSet::CharRanges GeneratedPrintableRanges;
বাহ্যিক ধ্রুবক UnicodeCharSet::CharRanges GeneratedControlCharacterRanges;
বাহ্যিক ধ্রুবক UnicodeCharSet::CharRanges GeneratedFormatCharacterRanges;
বাহ্যিক ধ্রুবক UnicodeCharSet::CharRanges GeneratedCombiningCharacterRanges;
বাহ্যিক ধ্রুবক UnicodeCharSet::CharRanges GeneratedDoubleWidthCharacterRanges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
যৌক্তিক isCharacter(পূর্ণসংখ্যা UCS) {
  স্থির ধ্রুবক UnicodeCharSet Character(GeneratedCharacterRanges);
  ফেরত Character.contains(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
যৌক্তিক isPrintable(পূর্ণসংখ্যা UCS) {
  স্থির ধ্রুবক UnicodeCharSet Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  ফেরত UCS == 0x00AD || Printables.contains(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
যৌক্তিক isControl(পূর্ণসংখ্যা UCS) {
  স্থির ধ্রুবক UnicodeCharSet Control(GeneratedControlCharacterRanges);
  ফেরত Control.contains(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
যৌক্তিক isFormatting(পূর্ণসংখ্যা UCS) {
  স্থির ধ্রুবক UnicodeCharSet Format(GeneratedFormatCharacterRanges);
  ফেরত Format.contains(UCS);
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
স্থির অন্তঃরেখ পূর্ণসংখ্যা charWidth(পূর্ণসংখ্যা UCS) {
  যদি (!isPrintable(UCS))
    ফেরত ErrorNonPrintableCharacter;

  স্থির ধ্রুবক UnicodeCharSet CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  যদি (CombiningCharacters.contains(UCS))
    ফেরত 0;

  স্থির ধ্রুবক UnicodeCharSet DoubleWidthCharacters(
      GeneratedDoubleWidthCharacterRanges);

  যদি (DoubleWidthCharacters.contains(UCS))
    ফেরত 2;
  ফেরত 1;
}

স্থির যৌক্তিক isprintableascii(অক্ষর c) { ফেরত c > 31 && c < 127; }

পূর্ণসংখ্যা columnWidthUTF8(StringRef Text) {
  unsigned ColumnWidth = 0;
  unsigned দৈর্ঘ্য;
  জন্য (size_t i = 0, e = Text.আকার_3(); i < e; i += দৈর্ঘ্য) {
    দৈর্ঘ্য = get_অঙ্ক_Bytes_For_UTF8(Text[i]);

    // fast path for ASCII characters
    যদি (দৈর্ঘ্য == 1) {
      যদি (!isprintableascii(Text[i]))
        ফেরত ErrorNonPrintableCharacter;
      ColumnWidth += 1;
      চালু;
    }

    যদি (দৈর্ঘ্য <= 0 || i + দৈর্ঘ্য > Text.আকার_3())
      ফেরত ErrorInvalidUTF8;
    UTF32 buf[1];
    ধ্রুবক UTF8 *Start = reinterpret_cast<ধ্রুবক UTF8 *>(Text.তথ্য() + i);
    UTF32 *Target = &buf[0];
    যদি (conversionOK != ConvertUTF8toUTF32(&Start, Start + দৈর্ঘ্য, &Target,
                                           Target + 1, strictConversion))
      ফেরত ErrorInvalidUTF8;
    পূর্ণসংখ্যা Width = charWidth(buf[0]);
    যদি (Width < 0)
      ফেরত ErrorNonPrintableCharacter;
    ColumnWidth += Width;
  }
  ফেরত ColumnWidth;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
