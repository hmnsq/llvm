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

ಬಾಹ್ಯ ಧ್ರುವ UnicodeCharSet::CharRanges GeneratedCharacterRanges;
ಬಾಹ್ಯ ಧ್ರುವ UnicodeCharSet::CharRanges GeneratedPrintableRanges;
ಬಾಹ್ಯ ಧ್ರುವ UnicodeCharSet::CharRanges GeneratedControlCharacterRanges;
ಬಾಹ್ಯ ಧ್ರುವ UnicodeCharSet::CharRanges GeneratedFormatCharacterRanges;
ಬಾಹ್ಯ ಧ್ರುವ UnicodeCharSet::CharRanges GeneratedCombiningCharacterRanges;
ಬಾಹ್ಯ ಧ್ರುವ UnicodeCharSet::CharRanges GeneratedDoubleWidthCharacterRanges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
ತಾರ್ಕಿಕ isCharacter(ಪೂರ್ಣಾಙ್ಕ UCS) {
  ಸ್ಥಿರ ಧ್ರುವ UnicodeCharSet Character(GeneratedCharacterRanges);
  ಪ್ರತ್ಯಾಗಮ Character.contains(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
ತಾರ್ಕಿಕ isPrintable(ಪೂರ್ಣಾಙ್ಕ UCS) {
  ಸ್ಥಿರ ಧ್ರುವ UnicodeCharSet Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  ಪ್ರತ್ಯಾಗಮ UCS == 0x00AD || Printables.contains(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
ತಾರ್ಕಿಕ isControl(ಪೂರ್ಣಾಙ್ಕ UCS) {
  ಸ್ಥಿರ ಧ್ರುವ UnicodeCharSet Control(GeneratedControlCharacterRanges);
  ಪ್ರತ್ಯಾಗಮ Control.contains(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
ತಾರ್ಕಿಕ isFormatting(ಪೂರ್ಣಾಙ್ಕ UCS) {
  ಸ್ಥಿರ ಧ್ರುವ UnicodeCharSet Format(GeneratedFormatCharacterRanges);
  ಪ್ರತ್ಯಾಗಮ Format.contains(UCS);
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
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ charWidth(ಪೂರ್ಣಾಙ್ಕ UCS) {
  ಯದಿ (!isPrintable(UCS))
    ಪ್ರತ್ಯಾಗಮ ErrorNonPrintableCharacter;

  ಸ್ಥಿರ ಧ್ರುವ UnicodeCharSet CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  ಯದಿ (CombiningCharacters.contains(UCS))
    ಪ್ರತ್ಯಾಗಮ 0;

  ಸ್ಥಿರ ಧ್ರುವ UnicodeCharSet DoubleWidthCharacters(
      GeneratedDoubleWidthCharacterRanges);

  ಯದಿ (DoubleWidthCharacters.contains(UCS))
    ಪ್ರತ್ಯಾಗಮ 2;
  ಪ್ರತ್ಯಾಗಮ 1;
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isprintableascii(ವರ್ಣ c) { ಪ್ರತ್ಯಾಗಮ c > 31 && c < 127; }

ಪೂರ್ಣಾಙ್ಕ columnWidthUTF8(StringRef Text) {
  unsigned ColumnWidth = 0;
  unsigned Length;
  ಪ್ರತಿ (size_t i = 0, e = Text.size(); i < e; i += Length) {
    Length = get_ಅಂಕೆ_Bytes_For_UTF8(Text[i]);

    // fast path for ASCII characters
    ಯದಿ (Length == 1) {
      ಯದಿ (!isprintableascii(Text[i]))
        ಪ್ರತ್ಯಾಗಮ ErrorNonPrintableCharacter;
      ColumnWidth += 1;
      ಅನುವರ್ತ;
    }

    ಯದಿ (Length <= 0 || i + Length > Text.size())
      ಪ್ರತ್ಯಾಗಮ ErrorInvalidUTF8;
    UTF32 buf[1];
    ಧ್ರುವ UTF8 *Start = reinterpret_cast<ಧ್ರುವ UTF8 *>(Text.data() + i);
    UTF32 *Target = &buf[0];
    ಯದಿ (conversionOK != ConvertUTF8toUTF32(&Start, Start + Length, &Target,
                                           Target + 1, strictConversion))
      ಪ್ರತ್ಯಾಗಮ ErrorInvalidUTF8;
    ಪೂರ್ಣಾಙ್ಕ Width = charWidth(buf[0]);
    ಯದಿ (Width < 0)
      ಪ್ರತ್ಯಾಗಮ ErrorNonPrintableCharacter;
    ColumnWidth += Width;
  }
  ಪ್ರತ್ಯಾಗಮ ColumnWidth;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
