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

बाह्य स्थिरांक UnicodeCharSet::CharRanges GeneratedCharacterRanges;
बाह्य स्थिरांक UnicodeCharSet::CharRanges GeneratedPrintableRanges;
बाह्य स्थिरांक UnicodeCharSet::CharRanges GeneratedControlCharacterRanges;
बाह्य स्थिरांक UnicodeCharSet::CharRanges GeneratedFormatCharacterRanges;
बाह्य स्थिरांक UnicodeCharSet::CharRanges GeneratedCombiningCharacterRanges;
बाह्य स्थिरांक UnicodeCharSet::CharRanges GeneratedDoubleWidthCharacterRanges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
तार्किक isCharacter(पूर्णांक UCS) {
  स्थिर स्थिरांक UnicodeCharSet Character(GeneratedCharacterRanges);
  परत Character.contains(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
तार्किक isPrintable(पूर्णांक UCS) {
  स्थिर स्थिरांक UnicodeCharSet Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  परत UCS == 0x00AD || Printables.contains(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
तार्किक isControl(पूर्णांक UCS) {
  स्थिर स्थिरांक UnicodeCharSet Control(GeneratedControlCharacterRanges);
  परत Control.contains(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
तार्किक isFormatting(पूर्णांक UCS) {
  स्थिर स्थिरांक UnicodeCharSet Format(GeneratedFormatCharacterRanges);
  परत Format.contains(UCS);
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
स्थिर ओळीत पूर्णांक charWidth(पूर्णांक UCS) {
  जर (!isPrintable(UCS))
    परत ErrorNonPrintableCharacter;

  स्थिर स्थिरांक UnicodeCharSet CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  जर (CombiningCharacters.contains(UCS))
    परत 0;

  स्थिर स्थिरांक UnicodeCharSet DoubleWidthCharacters(
      GeneratedDoubleWidthCharacterRanges);

  जर (DoubleWidthCharacters.contains(UCS))
    परत 2;
  परत 1;
}

स्थिर तार्किक isprintableascii(अक्षर c) { परत c > 31 && c < 127; }

पूर्णांक columnWidthUTF8(StringRef Text) {
  अचिन्हित ColumnWidth = 0;
  अचिन्हित Length;
  साठी (माप_प्रकार i = 0, e = Text.size(); i < e; i += Length) {
    Length = get_अंक_Bytes_For_UTF8(Text[i]);

    // fast path for ASCII characters
    जर (Length == 1) {
      जर (!isprintableascii(Text[i]))
        परत ErrorNonPrintableCharacter;
      ColumnWidth += 1;
      सुरूठेवा;
    }

    जर (Length <= 0 || i + Length > Text.size())
      परत ErrorInvalidUTF8;
    UTF32 buf[1];
    स्थिरांक UTF8 *Start = reinterpret_cast<स्थिरांक UTF8 *>(Text.data() + i);
    UTF32 *Target = &buf[0];
    जर (conversionOK != ConvertUTF8toUTF32(&Start, Start + Length, &Target,
                                           Target + 1, strictConversion))
      परत ErrorInvalidUTF8;
    पूर्णांक Width = charWidth(buf[0]);
    जर (Width < 0)
      परत ErrorNonPrintableCharacter;
    ColumnWidth += Width;
  }
  परत ColumnWidth;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
