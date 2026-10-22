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

#شامل "llvm/Support/Unicode.h"
#شامل "llvm/Support/ConvertUTF.h"
#شامل "llvm/Support/UnicodeCharRanges.h"

فضاینام llvm {
فضاینام sys {
فضاینام unicode {

بیرونی ثابت UnicodeCharSet::CharRanges GeneratedCharacterRanges;
بیرونی ثابت UnicodeCharSet::CharRanges GeneratedPrintableRanges;
بیرونی ثابت UnicodeCharSet::CharRanges GeneratedControlCharacterRanges;
بیرونی ثابت UnicodeCharSet::CharRanges GeneratedFormatCharacterRanges;
بیرونی ثابت UnicodeCharSet::CharRanges GeneratedCombiningCharacterRanges;
بیرونی ثابت UnicodeCharSet::CharRanges GeneratedDoubleWidthCharacterRanges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
منطقی isCharacter(صحیح UCS) {
  ایستا ثابت UnicodeCharSet Character(GeneratedCharacterRanges);
  بازگشت Character.contains(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
منطقی isPrintable(صحیح UCS) {
  ایستا ثابت UnicodeCharSet Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  بازگشت UCS == 0x00AD || Printables.contains(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
منطقی isControl(صحیح UCS) {
  ایستا ثابت UnicodeCharSet Control(GeneratedControlCharacterRanges);
  بازگشت Control.contains(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
منطقی isFormatting(صحیح UCS) {
  ایستا ثابت UnicodeCharSet Format(GeneratedFormatCharacterRanges);
  بازگشت Format.contains(UCS);
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
ایستا درونخطی صحیح charWidth(صحیح UCS) {
  اگر (!isPrintable(UCS))
    بازگشت ErrorNonPrintableCharacter;

  ایستا ثابت UnicodeCharSet CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  اگر (CombiningCharacters.contains(UCS))
    بازگشت 0;

  ایستا ثابت UnicodeCharSet DoubleWidthCharacters(
      GeneratedDoubleWidthCharacterRanges);

  اگر (DoubleWidthCharacters.contains(UCS))
    بازگشت 2;
  بازگشت 1;
}

ایستا منطقی isprintableascii(نویسه c) { بازگشت c > 31 && c < 127; }

صحیح columnWidthUTF8(StringRef Text) {
  unsigned ColumnWidth = 0;
  unsigned Length;
  برای (size_t i = 0, e = Text.size(); i < e; i += Length) {
    Length = get_رقم_Bytes_For_UTF8(Text[i]);

    // fast path for ASCII characters
    اگر (Length == 1) {
      اگر (!isprintableascii(Text[i]))
        بازگشت ErrorNonPrintableCharacter;
      ColumnWidth += 1;
      ادامه;
    }

    اگر (Length <= 0 || i + Length > Text.size())
      بازگشت ErrorInvalidUTF8;
    UTF32 buf[1];
    ثابت UTF8 *Start = reinterpret_cast<ثابت UTF8 *>(Text.data() + i);
    UTF32 *Target = &buf[0];
    اگر (conversionOK != ConvertUTF8toUTF32(&Start, Start + Length, &Target,
                                           Target + 1, strictConversion))
      بازگشت ErrorInvalidUTF8;
    صحیح Width = charWidth(buf[0]);
    اگر (Width < 0)
      بازگشت ErrorNonPrintableCharacter;
    ColumnWidth += Width;
  }
  بازگشت ColumnWidth;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
