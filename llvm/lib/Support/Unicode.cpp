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

বাহ্য ধ্রুব UnicodeCharSet::CharRanges GeneratedCharacterRanges;
বাহ্য ধ্রুব UnicodeCharSet::CharRanges GeneratedPrintableRanges;
বাহ্য ধ্রুব UnicodeCharSet::CharRanges GeneratedControlCharacterRanges;
বাহ্য ধ্রুব UnicodeCharSet::CharRanges GeneratedFormatCharacterRanges;
বাহ্য ধ্রুব UnicodeCharSet::CharRanges GeneratedCombiningCharacterRanges;
বাহ্য ধ্রুব UnicodeCharSet::CharRanges GeneratedDoubleWidthCharacterRanges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
তার্কিক isCharacter(পূর্ণাঙ্ক UCS) {
  স্থির ধ্রুব UnicodeCharSet Character(GeneratedCharacterRanges);
  প্রত্যাগম Character.contains(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
তার্কিক isPrintable(পূর্ণাঙ্ক UCS) {
  স্থির ধ্রুব UnicodeCharSet Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  প্রত্যাগম UCS == 0x00AD || Printables.contains(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
তার্কিক isControl(পূর্ণাঙ্ক UCS) {
  স্থির ধ্রুব UnicodeCharSet Control(GeneratedControlCharacterRanges);
  প্রত্যাগম Control.contains(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
তার্কিক isFormatting(পূর্ণাঙ্ক UCS) {
  স্থির ধ্রুব UnicodeCharSet Format(GeneratedFormatCharacterRanges);
  প্রত্যাগম Format.contains(UCS);
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
স্থির অন্তঃস্থ পূর্ণাঙ্ক charWidth(পূর্ণাঙ্ক UCS) {
  যদি (!isPrintable(UCS))
    প্রত্যাগম ErrorNonPrintableCharacter;

  স্থির ধ্রুব UnicodeCharSet CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  যদি (CombiningCharacters.contains(UCS))
    প্রত্যাগম 0;

  স্থির ধ্রুব UnicodeCharSet DoubleWidthCharacters(
      GeneratedDoubleWidthCharacterRanges);

  যদি (DoubleWidthCharacters.contains(UCS))
    প্রত্যাগম 2;
  প্রত্যাগম 1;
}

স্থির তার্কিক isprintableascii(বর্ণ c) { প্রত্যাগম c > 31 && c < 127; }

পূর্ণাঙ্ক columnWidthUTF8(StringRef Text) {
  unsigned ColumnWidth = 0;
  unsigned Length;
  প্রতি (size_t i = 0, e = Text.size(); i < e; i += Length) {
    Length = get_অঙ্ক_Bytes_For_UTF8(Text[i]);

    // fast path for ASCII characters
    যদি (Length == 1) {
      যদি (!isprintableascii(Text[i]))
        প্রত্যাগম ErrorNonPrintableCharacter;
      ColumnWidth += 1;
      অনুবর্ত;
    }

    যদি (Length <= 0 || i + Length > Text.size())
      প্রত্যাগম ErrorInvalidUTF8;
    UTF32 buf[1];
    ধ্রুব UTF8 *Start = reinterpret_cast<ধ্রুব UTF8 *>(Text.data() + i);
    UTF32 *Target = &buf[0];
    যদি (conversionOK != ConvertUTF8toUTF32(&Start, Start + Length, &Target,
                                           Target + 1, strictConversion))
      প্রত্যাগম ErrorInvalidUTF8;
    পূর্ণাঙ্ক Width = charWidth(buf[0]);
    যদি (Width < 0)
      প্রত্যাগম ErrorNonPrintableCharacter;
    ColumnWidth += Width;
  }
  প্রত্যাগম ColumnWidth;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
