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

#ಅನ್ತರ್ಭಾವ "llvm/Support/Unicode.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ConvertUTF.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/UnicodeCharRanges.h"

ನಾಮಾಕಾಶ llvm {
ನಾಮಾಕಾಶ sys {
ನಾಮಾಕಾಶ unicode {

ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ Unicode_Char_ಗಣ::CharRanges GeneratedCharacterRanges;
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ Unicode_Char_ಗಣ::CharRanges GeneratedPrintableRanges;
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ Unicode_Char_ಗಣ::CharRanges GeneratedControlCharacterRanges;
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ Unicode_Char_ಗಣ::CharRanges GeneratedFormatCharacterRanges;
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ Unicode_Char_ಗಣ::CharRanges GeneratedCombiningCharacterRanges;
ಬಾಹ್ಯ ಸ್ಥಿರಾಂಕ Unicode_Char_ಗಣ::CharRanges Generated_Double_ಅಗಲ_Character_Ranges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
ತಾರ್ಕಿಕ isCharacter(ಪೂರ್ಣಾಂಕ UCS) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Unicode_Char_ಗಣ Character(GeneratedCharacterRanges);
  ಹಿಂತಿರುಗಿ Character.ಒಳಗೊಂಡಿದೆ(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
ತಾರ್ಕಿಕ isPrintable(ಪೂರ್ಣಾಂಕ UCS) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Unicode_Char_ಗಣ Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  ಹಿಂತಿರುಗಿ UCS == 0x00AD || Printables.ಒಳಗೊಂಡಿದೆ(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
ತಾರ್ಕಿಕ isControl(ಪೂರ್ಣಾಂಕ UCS) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Unicode_Char_ಗಣ Control(GeneratedControlCharacterRanges);
  ಹಿಂತಿರುಗಿ Control.ಒಳಗೊಂಡಿದೆ(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
ತಾರ್ಕಿಕ isFormatting(ಪೂರ್ಣಾಂಕ UCS) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Unicode_Char_ಗಣ Format(GeneratedFormatCharacterRanges);
  ಹಿಂತಿರುಗಿ Format.ಒಳಗೊಂಡಿದೆ(UCS);
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
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ char_ಅಗಲ(ಪೂರ್ಣಾಂಕ UCS) {
  ಆದರೆ (!isPrintable(UCS))
    ಹಿಂತಿರುಗಿ ErrorNonPrintableCharacter;

  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Unicode_Char_ಗಣ CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  ಆದರೆ (CombiningCharacters.ಒಳಗೊಂಡಿದೆ(UCS))
    ಹಿಂತಿರುಗಿ 0;

  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Unicode_Char_ಗಣ Double_ಅಗಲ_Characters(
      Generated_Double_ಅಗಲ_Character_Ranges);

  ಆದರೆ (Double_ಅಗಲ_Characters.ಒಳಗೊಂಡಿದೆ(UCS))
    ಹಿಂತಿರುಗಿ 2;
  ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isprintableascii(ಅಕ್ಷರ c) { ಹಿಂತಿರುಗಿ c > 31 && c < 127; }

ಪೂರ್ಣಾಂಕ ಸ್ತಂಭ_ಅಗಲ_UTF8(StringRef ಪಠ್ಯ_2) {
  ಚಿಹ್ನೆರಹಿತ ಸ್ತಂಭ_ಅಗಲ_2 = 0;
  ಚಿಹ್ನೆರಹಿತ ಉದ್ದಳತೆ;
  ಗಾಗಿ (size_t i = 0, e = ಪಠ್ಯ_2.ಗಾತ್ರ_3(); i < e; i += ಉದ್ದಳತೆ) {
    ಉದ್ದಳತೆ = get_ಅಂಕೆ_ಬೈಟುಗಳು_For_UTF8(ಪಠ್ಯ_2[i]);

    // fast path for ASCII characters
    ಆದರೆ (ಉದ್ದಳತೆ == 1) {
      ಆದರೆ (!isprintableascii(ಪಠ್ಯ_2[i]))
        ಹಿಂತಿರುಗಿ ErrorNonPrintableCharacter;
      ಸ್ತಂಭ_ಅಗಲ_2 += 1;
      ಮುಂದುವರಿ;
    }

    ಆದರೆ (ಉದ್ದಳತೆ <= 0 || i + ಉದ್ದಳತೆ > ಪಠ್ಯ_2.ಗಾತ್ರ_3())
      ಹಿಂತಿರುಗಿ ErrorInvalidUTF8;
    UTF32 buf[1];
    ಸ್ಥಿರಾಂಕ UTF8 *ಆರಂಭ = reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF8 *>(ಪಠ್ಯ_2.ದತ್ತಾಂಶ() + i);
    UTF32 *ಗುರಿ_2 = &buf[0];
    ಆದರೆ (conversionOK != ConvertUTF8toUTF32(&ಆರಂಭ, ಆರಂಭ + ಉದ್ದಳತೆ, &ಗುರಿ_2,
                                           ಗುರಿ_2 + 1, strictConversion))
      ಹಿಂತಿರುಗಿ ErrorInvalidUTF8;
    ಪೂರ್ಣಾಂಕ ಅಗಲ = char_ಅಗಲ(buf[0]);
    ಆದರೆ (ಅಗಲ < 0)
      ಹಿಂತಿರುಗಿ ErrorNonPrintableCharacter;
    ಸ್ತಂಭ_ಅಗಲ_2 += ಅಗಲ;
  }
  ಹಿಂತಿರುಗಿ ಸ್ತಂಭ_ಅಗಲ_2;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
