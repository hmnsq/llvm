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

خارجي ثابت Unicode_Char_مجموعه::CharRanges GeneratedCharacterRanges;
خارجي ثابت Unicode_Char_مجموعه::CharRanges GeneratedPrintableRanges;
خارجي ثابت Unicode_Char_مجموعه::CharRanges GeneratedControlCharacterRanges;
خارجي ثابت Unicode_Char_مجموعه::CharRanges GeneratedFormatCharacterRanges;
خارجي ثابت Unicode_Char_مجموعه::CharRanges GeneratedCombiningCharacterRanges;
خارجي ثابت Unicode_Char_مجموعه::CharRanges Generated_Double_عرض_Character_Ranges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
منطقي isCharacter(صحيح UCS) {
  ساكن ثابت Unicode_Char_مجموعه Character(GeneratedCharacterRanges);
  أرجع Character.شامل_است(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
منطقي isPrintable(صحيح UCS) {
  ساكن ثابت Unicode_Char_مجموعه Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  أرجع UCS == 0x00AD || Printables.شامل_است(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
منطقي isControl(صحيح UCS) {
  ساكن ثابت Unicode_Char_مجموعه Control(GeneratedControlCharacterRanges);
  أرجع Control.شامل_است(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
منطقي isFormatting(صحيح UCS) {
  ساكن ثابت Unicode_Char_مجموعه Format(GeneratedFormatCharacterRanges);
  أرجع Format.شامل_است(UCS);
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
ساكن مضمن صحيح char_عرض(صحيح UCS) {
  إذا (!isPrintable(UCS))
    أرجع ErrorNonPrintableCharacter;

  ساكن ثابت Unicode_Char_مجموعه CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  إذا (CombiningCharacters.شامل_است(UCS))
    أرجع 0;

  ساكن ثابت Unicode_Char_مجموعه Double_عرض_Characters(
      Generated_Double_عرض_Character_Ranges);

  إذا (Double_عرض_Characters.شامل_است(UCS))
    أرجع 2;
  أرجع 1;
}

ساكن منطقي isprintableascii(حرف c) { أرجع c > 31 && c < 127; }

صحيح ستونی_عرض_UTF8(StringRef متن_2) {
  بیعلامت ستونی_عرض_2 = 0;
  بیعلامت درازی;
  لكل (size_t i = 0, e = متن_2.اندازه_2(); i < e; i += درازی) {
    درازی = get_رقم_بایتها_For_UTF8(متن_2[i]);

    // fast path for ASCII characters
    إذا (درازی == 1) {
      إذا (!isprintableascii(متن_2[i]))
        أرجع ErrorNonPrintableCharacter;
      ستونی_عرض_2 += 1;
      استمر;
    }

    إذا (درازی <= 0 || i + درازی > متن_2.اندازه_2())
      أرجع ErrorInvalidUTF8;
    UTF32 buf[1];
    ثابت UTF8 *شروع = reinterpret_cast<ثابت UTF8 *>(متن_2.داده() + i);
    UTF32 *هدف_2 = &buf[0];
    إذا (conversionOK != ConvertUTF8toUTF32(&شروع, شروع + درازی, &هدف_2,
                                           هدف_2 + 1, strictConversion))
      أرجع ErrorInvalidUTF8;
    صحيح عرض = char_عرض(buf[0]);
    إذا (عرض < 0)
      أرجع ErrorNonPrintableCharacter;
    ستونی_عرض_2 += عرض;
  }
  أرجع ستونی_عرض_2;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
