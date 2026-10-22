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

خارجي ثابت Unicode_Char_سیٹ::CharRanges GeneratedCharacterRanges;
خارجي ثابت Unicode_Char_سیٹ::CharRanges GeneratedPrintableRanges;
خارجي ثابت Unicode_Char_سیٹ::CharRanges GeneratedControlCharacterRanges;
خارجي ثابت Unicode_Char_سیٹ::CharRanges GeneratedFormatCharacterRanges;
خارجي ثابت Unicode_Char_سیٹ::CharRanges GeneratedCombiningCharacterRanges;
خارجي ثابت Unicode_Char_سیٹ::CharRanges Generated_Double_چوڑائی_Character_Ranges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
منطقي isCharacter(صحيح UCS) {
  ساكن ثابت Unicode_Char_سیٹ Character(GeneratedCharacterRanges);
  أرجع Character.شامل_اے(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
منطقي isPrintable(صحيح UCS) {
  ساكن ثابت Unicode_Char_سیٹ Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  أرجع UCS == 0x00AD || Printables.شامل_اے(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
منطقي isControl(صحيح UCS) {
  ساكن ثابت Unicode_Char_سیٹ Control(GeneratedControlCharacterRanges);
  أرجع Control.شامل_اے(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
منطقي isFormatting(صحيح UCS) {
  ساكن ثابت Unicode_Char_سیٹ Format(GeneratedFormatCharacterRanges);
  أرجع Format.شامل_اے(UCS);
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
ساكن مضمن صحيح char_چوڑائی(صحيح UCS) {
  إذا (!isPrintable(UCS))
    أرجع ErrorNonPrintableCharacter;

  ساكن ثابت Unicode_Char_سیٹ CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  إذا (CombiningCharacters.شامل_اے(UCS))
    أرجع 0;

  ساكن ثابت Unicode_Char_سیٹ Double_چوڑائی_Characters(
      Generated_Double_چوڑائی_Character_Ranges);

  إذا (Double_چوڑائی_Characters.شامل_اے(UCS))
    أرجع 2;
  أرجع 1;
}

ساكن منطقي isprintableascii(حرف c) { أرجع c > 31 && c < 127; }

صحيح کالم_چوڑائی_UTF8(StringRef متن_2) {
  بنانشانی کالم_چوڑائی_2 = 0;
  بنانشانی لمائی;
  لكل (size_t i = 0, e = متن_2.ناپ_2(); i < e; i += لمائی) {
    لمائی = get_عدد_بائٹاں_For_UTF8(متن_2[i]);

    // fast path for ASCII characters
    إذا (لمائی == 1) {
      إذا (!isprintableascii(متن_2[i]))
        أرجع ErrorNonPrintableCharacter;
      کالم_چوڑائی_2 += 1;
      استمر;
    }

    إذا (لمائی <= 0 || i + لمائی > متن_2.ناپ_2())
      أرجع ErrorInvalidUTF8;
    UTF32 buf[1];
    ثابت UTF8 *شروع = reinterpret_cast<ثابت UTF8 *>(متن_2.انکڑے() + i);
    UTF32 *نشانہ_2 = &buf[0];
    إذا (conversionOK != ConvertUTF8toUTF32(&شروع, شروع + لمائی, &نشانہ_2,
                                           نشانہ_2 + 1, strictConversion))
      أرجع ErrorInvalidUTF8;
    صحيح چوڑائی = char_چوڑائی(buf[0]);
    إذا (چوڑائی < 0)
      أرجع ErrorNonPrintableCharacter;
    کالم_چوڑائی_2 += چوڑائی;
  }
  أرجع کالم_چوڑائی_2;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
