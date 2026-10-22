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

نامخلا llvm {
نامخلا sys {
نامخلا unicode {

خارجی ثابت Unicode_Char_سیٹ::CharRanges GeneratedCharacterRanges;
خارجی ثابت Unicode_Char_سیٹ::CharRanges GeneratedPrintableRanges;
خارجی ثابت Unicode_Char_سیٹ::CharRanges GeneratedControlCharacterRanges;
خارجی ثابت Unicode_Char_سیٹ::CharRanges GeneratedFormatCharacterRanges;
خارجی ثابت Unicode_Char_سیٹ::CharRanges GeneratedCombiningCharacterRanges;
خارجی ثابت Unicode_Char_سیٹ::CharRanges Generated_Double_چوڑائی_Character_Ranges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
منطقی isCharacter(صحیح UCS) {
  ساکن ثابت Unicode_Char_سیٹ Character(GeneratedCharacterRanges);
  واپس Character.شامل_ہے(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
منطقی isPrintable(صحیح UCS) {
  ساکن ثابت Unicode_Char_سیٹ Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  واپس UCS == 0x00AD || Printables.شامل_ہے(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
منطقی isControl(صحیح UCS) {
  ساکن ثابت Unicode_Char_سیٹ Control(GeneratedControlCharacterRanges);
  واپس Control.شامل_ہے(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
منطقی isFormatting(صحیح UCS) {
  ساکن ثابت Unicode_Char_سیٹ Format(GeneratedFormatCharacterRanges);
  واپس Format.شامل_ہے(UCS);
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
ساکن اندرونی صحیح char_چوڑائی(صحیح UCS) {
  اگر (!isPrintable(UCS))
    واپس ErrorNonPrintableCharacter;

  ساکن ثابت Unicode_Char_سیٹ CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  اگر (CombiningCharacters.شامل_ہے(UCS))
    واپس 0;

  ساکن ثابت Unicode_Char_سیٹ Double_چوڑائی_Characters(
      Generated_Double_چوڑائی_Character_Ranges);

  اگر (Double_چوڑائی_Characters.شامل_ہے(UCS))
    واپس 2;
  واپس 1;
}

ساکن منطقی isprintableascii(حرف c) { واپس c > 31 && c < 127; }

صحیح عمود_چوڑائی_UTF8(StringRef متن_2) {
  غیرعلامتی عمود_چوڑائی_2 = 0;
  غیرعلامتی لمبائی;
  برائے (size_t i = 0, e = متن_2.سائز_2(); i < e; i += لمبائی) {
    لمبائی = get_عدد_بائٹیں_For_UTF8(متن_2[i]);

    // fast path for ASCII characters
    اگر (لمبائی == 1) {
      اگر (!isprintableascii(متن_2[i]))
        واپس ErrorNonPrintableCharacter;
      عمود_چوڑائی_2 += 1;
      جاری;
    }

    اگر (لمبائی <= 0 || i + لمبائی > متن_2.سائز_2())
      واپس ErrorInvalidUTF8;
    UTF32 buf[1];
    ثابت UTF8 *آغاز = reinterpret_cast<ثابت UTF8 *>(متن_2.ڈیٹا() + i);
    UTF32 *ہدف_2 = &buf[0];
    اگر (conversionOK != ConvertUTF8toUTF32(&آغاز, آغاز + لمبائی, &ہدف_2,
                                           ہدف_2 + 1, strictConversion))
      واپس ErrorInvalidUTF8;
    صحیح چوڑائی = char_چوڑائی(buf[0]);
    اگر (چوڑائی < 0)
      واپس ErrorNonPrintableCharacter;
    عمود_چوڑائی_2 += چوڑائی;
  }
  واپس عمود_چوڑائی_2;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
