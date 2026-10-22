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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Unicode.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ConvertUTF.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/UnicodeCharRanges.h"

ਨਾਮਾਕਾਸ਼ llvm {
ਨਾਮਾਕਾਸ਼ sys {
ਨਾਮਾਕਾਸ਼ unicode {

ਬਾਹਰੀ ਅਚਲ Unicode_Char_ਸੈੱਟ::CharRanges GeneratedCharacterRanges;
ਬਾਹਰੀ ਅਚਲ Unicode_Char_ਸੈੱਟ::CharRanges GeneratedPrintableRanges;
ਬਾਹਰੀ ਅਚਲ Unicode_Char_ਸੈੱਟ::CharRanges GeneratedControlCharacterRanges;
ਬਾਹਰੀ ਅਚਲ Unicode_Char_ਸੈੱਟ::CharRanges GeneratedFormatCharacterRanges;
ਬਾਹਰੀ ਅਚਲ Unicode_Char_ਸੈੱਟ::CharRanges GeneratedCombiningCharacterRanges;
ਬਾਹਰੀ ਅਚਲ Unicode_Char_ਸੈੱਟ::CharRanges Generated_Double_ਚੌੜਾਈ_Character_Ranges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
ਤਾਰਕਿਕ isCharacter(ਪੂਰਨ UCS) {
  ਸਥਿਰ ਅਚਲ Unicode_Char_ਸੈੱਟ Character(GeneratedCharacterRanges);
  ਵਾਪਸ Character.ਸ਼ਾਮਲ_ਹੈ(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
ਤਾਰਕਿਕ isPrintable(ਪੂਰਨ UCS) {
  ਸਥਿਰ ਅਚਲ Unicode_Char_ਸੈੱਟ Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  ਵਾਪਸ UCS == 0x00AD || Printables.ਸ਼ਾਮਲ_ਹੈ(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
ਤਾਰਕਿਕ isControl(ਪੂਰਨ UCS) {
  ਸਥਿਰ ਅਚਲ Unicode_Char_ਸੈੱਟ Control(GeneratedControlCharacterRanges);
  ਵਾਪਸ Control.ਸ਼ਾਮਲ_ਹੈ(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
ਤਾਰਕਿਕ isFormatting(ਪੂਰਨ UCS) {
  ਸਥਿਰ ਅਚਲ Unicode_Char_ਸੈੱਟ Format(GeneratedFormatCharacterRanges);
  ਵਾਪਸ Format.ਸ਼ਾਮਲ_ਹੈ(UCS);
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
ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ char_ਚੌੜਾਈ(ਪੂਰਨ UCS) {
  ਜੇ (!isPrintable(UCS))
    ਵਾਪਸ ErrorNonPrintableCharacter;

  ਸਥਿਰ ਅਚਲ Unicode_Char_ਸੈੱਟ CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  ਜੇ (CombiningCharacters.ਸ਼ਾਮਲ_ਹੈ(UCS))
    ਵਾਪਸ 0;

  ਸਥਿਰ ਅਚਲ Unicode_Char_ਸੈੱਟ Double_ਚੌੜਾਈ_Characters(
      Generated_Double_ਚੌੜਾਈ_Character_Ranges);

  ਜੇ (Double_ਚੌੜਾਈ_Characters.ਸ਼ਾਮਲ_ਹੈ(UCS))
    ਵਾਪਸ 2;
  ਵਾਪਸ 1;
}

ਸਥਿਰ ਤਾਰਕਿਕ isprintableascii(ਅੱਖਰ c) { ਵਾਪਸ c > 31 && c < 127; }

ਪੂਰਨ ਕਾਲਮ_ਚੌੜਾਈ_UTF8(StringRef ਪਾਠ_2) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਕਾਲਮ_ਚੌੜਾਈ_2 = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾਈ;
  ਲਈ (size_t i = 0, e = ਪਾਠ_2.ਮਾਪ_2(); i < e; i += ਲੰਮਾਈ) {
    ਲੰਮਾਈ = get_ਅੰਕ_ਬਾਈਟਾਂ_For_UTF8(ਪਾਠ_2[i]);

    // fast path for ASCII characters
    ਜੇ (ਲੰਮਾਈ == 1) {
      ਜੇ (!isprintableascii(ਪਾਠ_2[i]))
        ਵਾਪਸ ErrorNonPrintableCharacter;
      ਕਾਲਮ_ਚੌੜਾਈ_2 += 1;
      ਜਾਰੀ;
    }

    ਜੇ (ਲੰਮਾਈ <= 0 || i + ਲੰਮਾਈ > ਪਾਠ_2.ਮਾਪ_2())
      ਵਾਪਸ ErrorInvalidUTF8;
    UTF32 buf[1];
    ਅਚਲ UTF8 *ਸ਼ੁਰੂ = reinterpret_cast<ਅਚਲ UTF8 *>(ਪਾਠ_2.ਅੰਕੜੇ() + i);
    UTF32 *ਨਿਸ਼ਾਨਾ_2 = &buf[0];
    ਜੇ (conversionOK != ConvertUTF8toUTF32(&ਸ਼ੁਰੂ, ਸ਼ੁਰੂ + ਲੰਮਾਈ, &ਨਿਸ਼ਾਨਾ_2,
                                           ਨਿਸ਼ਾਨਾ_2 + 1, strictConversion))
      ਵਾਪਸ ErrorInvalidUTF8;
    ਪੂਰਨ ਚੌੜਾਈ = char_ਚੌੜਾਈ(buf[0]);
    ਜੇ (ਚੌੜਾਈ < 0)
      ਵਾਪਸ ErrorNonPrintableCharacter;
    ਕਾਲਮ_ਚੌੜਾਈ_2 += ਚੌੜਾਈ;
  }
  ਵਾਪਸ ਕਾਲਮ_ਚੌੜਾਈ_2;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
