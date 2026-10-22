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

बाह्य अचर UnicodeCharSet::CharRanges GeneratedCharacterRanges;
बाह्य अचर UnicodeCharSet::CharRanges GeneratedPrintableRanges;
बाह्य अचर UnicodeCharSet::CharRanges GeneratedControlCharacterRanges;
बाह्य अचर UnicodeCharSet::CharRanges GeneratedFormatCharacterRanges;
बाह्य अचर UnicodeCharSet::CharRanges GeneratedCombiningCharacterRanges;
बाह्य अचर UnicodeCharSet::CharRanges GeneratedDoubleWidthCharacterRanges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
तार्किक isCharacter(पूर्णाङ्क UCS) {
  स्थिर अचर UnicodeCharSet Character(GeneratedCharacterRanges);
  फर्काऊ Character.contains(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
तार्किक isPrintable(पूर्णाङ्क UCS) {
  स्थिर अचर UnicodeCharSet Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  फर्काऊ UCS == 0x00AD || Printables.contains(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
तार्किक isControl(पूर्णाङ्क UCS) {
  स्थिर अचर UnicodeCharSet Control(GeneratedControlCharacterRanges);
  फर्काऊ Control.contains(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
तार्किक isFormatting(पूर्णाङ्क UCS) {
  स्थिर अचर UnicodeCharSet Format(GeneratedFormatCharacterRanges);
  फर्काऊ Format.contains(UCS);
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
  यदि (!isPrintable(UCS))
    फर्काऊ ErrorNonPrintableCharacter;

  स्थिर अचर UnicodeCharSet CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  यदि (CombiningCharacters.contains(UCS))
    फर्काऊ 0;

  स्थिर अचर UnicodeCharSet DoubleWidthCharacters(
      GeneratedDoubleWidthCharacterRanges);

  यदि (DoubleWidthCharacters.contains(UCS))
    फर्काऊ 2;
  फर्काऊ 1;
}

स्थिर तार्किक isprintableascii(वर्ण c) { फर्काऊ c > 31 && c < 127; }

पूर्णाङ्क columnWidthUTF8(StringRef Text) {
  अचिन्हित ColumnWidth = 0;
  अचिन्हित Length;
  लागि (माप_प्रकार i = 0, e = Text.size(); i < e; i += Length) {
    Length = get_अङ्क_Bytes_For_UTF8(Text[i]);

    // fast path for ASCII characters
    यदि (Length == 1) {
      यदि (!isprintableascii(Text[i]))
        फर्काऊ ErrorNonPrintableCharacter;
      ColumnWidth += 1;
      जारी;
    }

    यदि (Length <= 0 || i + Length > Text.size())
      फर्काऊ ErrorInvalidUTF8;
    UTF32 buf[1];
    अचर UTF8 *Start = reinterpret_cast<अचर UTF8 *>(Text.data() + i);
    UTF32 *Target = &buf[0];
    यदि (conversionOK != ConvertUTF8toUTF32(&Start, Start + Length, &Target,
                                           Target + 1, strictConversion))
      फर्काऊ ErrorInvalidUTF8;
    पूर्णाङ्क Width = charWidth(buf[0]);
    यदि (Width < 0)
      फर्काऊ ErrorNonPrintableCharacter;
    ColumnWidth += Width;
  }
  फर्काऊ ColumnWidth;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
