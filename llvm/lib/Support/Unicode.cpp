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

#अन्तर्भाव "llvm/Support/Unicode.h"
#अन्तर्भाव "llvm/Support/ConvertUTF.h"
#अन्तर्भाव "llvm/Support/UnicodeCharRanges.h"

नामाकाश llvm {
नामाकाश sys {
नामाकाश unicode {

बाह्य ध्रुव UnicodeCharSet::CharRanges GeneratedCharacterRanges;
बाह्य ध्रुव UnicodeCharSet::CharRanges GeneratedPrintableRanges;
बाह्य ध्रुव UnicodeCharSet::CharRanges GeneratedControlCharacterRanges;
बाह्य ध्रुव UnicodeCharSet::CharRanges GeneratedFormatCharacterRanges;
बाह्य ध्रुव UnicodeCharSet::CharRanges GeneratedCombiningCharacterRanges;
बाह्य ध्रुव UnicodeCharSet::CharRanges GeneratedDoubleWidthCharacterRanges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
तार्किक isCharacter(पूर्णाङ्क UCS) {
  स्थिर ध्रुव UnicodeCharSet Character(GeneratedCharacterRanges);
  प्रत्यागम Character.contains(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
तार्किक isPrintable(पूर्णाङ्क UCS) {
  स्थिर ध्रुव UnicodeCharSet Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  प्रत्यागम UCS == 0x00AD || Printables.contains(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
तार्किक isControl(पूर्णाङ्क UCS) {
  स्थिर ध्रुव UnicodeCharSet Control(GeneratedControlCharacterRanges);
  प्रत्यागम Control.contains(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
तार्किक isFormatting(पूर्णाङ्क UCS) {
  स्थिर ध्रुव UnicodeCharSet Format(GeneratedFormatCharacterRanges);
  प्रत्यागम Format.contains(UCS);
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
स्थिर अन्तःस्थ पूर्णाङ्क charWidth(पूर्णाङ्क UCS) {
  जइ (!isPrintable(UCS))
    प्रत्यागम ErrorNonPrintableCharacter;

  स्थिर ध्रुव UnicodeCharSet CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  जइ (CombiningCharacters.contains(UCS))
    प्रत्यागम 0;

  स्थिर ध्रुव UnicodeCharSet DoubleWidthCharacters(
      GeneratedDoubleWidthCharacterRanges);

  जइ (DoubleWidthCharacters.contains(UCS))
    प्रत्यागम 2;
  प्रत्यागम 1;
}

स्थिर तार्किक isprintableascii(वर्ण c) { प्रत्यागम c > 31 && c < 127; }

पूर्णाङ्क columnWidthUTF8(StringRef Text) {
  unsigned ColumnWidth = 0;
  unsigned Length;
  प्रति (size_t i = 0, e = Text.size(); i < e; i += Length) {
    Length = get_अङ्कः_Bytes_For_UTF8(Text[i]);

    // fast path for ASCII characters
    जइ (Length == 1) {
      जइ (!isprintableascii(Text[i]))
        प्रत्यागम ErrorNonPrintableCharacter;
      ColumnWidth += 1;
      अनुवर्त;
    }

    जइ (Length <= 0 || i + Length > Text.size())
      प्रत्यागम ErrorInvalidUTF8;
    UTF32 buf[1];
    ध्रुव UTF8 *Start = reinterpret_cast<ध्रुव UTF8 *>(Text.data() + i);
    UTF32 *Target = &buf[0];
    जइ (conversionOK != ConvertUTF8toUTF32(&Start, Start + Length, &Target,
                                           Target + 1, strictConversion))
      प्रत्यागम ErrorInvalidUTF8;
    पूर्णाङ्क Width = charWidth(buf[0]);
    जइ (Width < 0)
      प्रत्यागम ErrorNonPrintableCharacter;
    ColumnWidth += Width;
  }
  प्रत्यागम ColumnWidth;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
