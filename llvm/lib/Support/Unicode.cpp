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

#ضمن "llvm/Support/Unicode.h"
#ضمن "llvm/Support/ConvertUTF.h"
#ضمن "llvm/Support/UnicodeCharRanges.h"

نطاق llvm {
نطاق sys {
نطاق unicode {

خارجي ثابت UnicodeCharSet::CharRanges GeneratedCharacterRanges;
خارجي ثابت UnicodeCharSet::CharRanges GeneratedPrintableRanges;
خارجي ثابت UnicodeCharSet::CharRanges GeneratedControlCharacterRanges;
خارجي ثابت UnicodeCharSet::CharRanges GeneratedFormatCharacterRanges;
خارجي ثابت UnicodeCharSet::CharRanges GeneratedCombiningCharacterRanges;
خارجي ثابت UnicodeCharSet::CharRanges GeneratedDoubleWidthCharacterRanges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
منطقي isCharacter(صحيح UCS) {
  ساكن ثابت UnicodeCharSet Character(GeneratedCharacterRanges);
  החזר Character.contains(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
منطقي isPrintable(صحيح UCS) {
  ساكن ثابت UnicodeCharSet Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  החזר UCS == 0x00AD || Printables.contains(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
منطقي isControl(صحيح UCS) {
  ساكن ثابت UnicodeCharSet Control(GeneratedControlCharacterRanges);
  החזר Control.contains(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
منطقي isFormatting(صحيح UCS) {
  ساكن ثابت UnicodeCharSet Format(GeneratedFormatCharacterRanges);
  החזר Format.contains(UCS);
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
ساكن مضمن صحيح charWidth(صحيح UCS) {
  אם (!isPrintable(UCS))
    החזר ErrorNonPrintableCharacter;

  ساكن ثابت UnicodeCharSet CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  אם (CombiningCharacters.contains(UCS))
    החזר 0;

  ساكن ثابت UnicodeCharSet DoubleWidthCharacters(
      GeneratedDoubleWidthCharacterRanges);

  אם (DoubleWidthCharacters.contains(UCS))
    החזר 2;
  החזר 1;
}

ساكن منطقي isprintableascii(حرف c) { החזר c > 31 && c < 127; }

صحيح columnWidthUTF8(StringRef Text) {
  unsigned ColumnWidth = 0;
  unsigned Length;
  עבור (size_t i = 0, e = Text.size(); i < e; i += Length) {
    Length = getNumBytesForUTF8(Text[i]);

    // fast path for ASCII characters
    אם (Length == 1) {
      אם (!isprintableascii(Text[i]))
        החזר ErrorNonPrintableCharacter;
      ColumnWidth += 1;
      استمر;
    }

    אם (Length <= 0 || i + Length > Text.size())
      החזר ErrorInvalidUTF8;
    UTF32 buf[1];
    ثابت UTF8 *Start = reinterpret_cast<ثابت UTF8 *>(Text.data() + i);
    UTF32 *Target = &buf[0];
    אם (conversionOK != ConvertUTF8toUTF32(&Start, Start + Length, &Target,
                                           Target + 1, strictConversion))
      החזר ErrorInvalidUTF8;
    صحيح Width = charWidth(buf[0]);
    אם (Width < 0)
      החזר ErrorNonPrintableCharacter;
    ColumnWidth += Width;
  }
  החזר ColumnWidth;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
