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

#समावेश "llvm/Support/Unicode.h"
#समावेश "llvm/Support/ConvertUTF.h"
#समावेश "llvm/Support/UnicodeCharRanges.h"

namespace llvm {
namespace sys {
namespace unicode {

बाह्य स्थिर UnicodeCharSet::CharRanges GeneratedCharacterRanges;
बाह्य स्थिर UnicodeCharSet::CharRanges GeneratedPrintableRanges;
बाह्य स्थिर UnicodeCharSet::CharRanges GeneratedControlCharacterRanges;
बाह्य स्थिर UnicodeCharSet::CharRanges GeneratedFormatCharacterRanges;
बाह्य स्थिर UnicodeCharSet::CharRanges GeneratedCombiningCharacterRanges;
बाह्य स्थिर UnicodeCharSet::CharRanges GeneratedDoubleWidthCharacterRanges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
bool isCharacter(पूर्णांक UCS) {
  अटल स्थिर UnicodeCharSet Character(GeneratedCharacterRanges);
  वापस Character.contains(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
bool isPrintable(पूर्णांक UCS) {
  अटल स्थिर UnicodeCharSet Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  वापस UCS == 0x00AD || Printables.contains(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
bool isControl(पूर्णांक UCS) {
  अटल स्थिर UnicodeCharSet Control(GeneratedControlCharacterRanges);
  वापस Control.contains(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
bool isFormatting(पूर्णांक UCS) {
  अटल स्थिर UnicodeCharSet Format(GeneratedFormatCharacterRanges);
  वापस Format.contains(UCS);
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
अटल अंतरभूत पूर्णांक charWidth(पूर्णांक UCS) {
  अगर (!isPrintable(UCS))
    वापस ErrorNonPrintableCharacter;

  अटल स्थिर UnicodeCharSet CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  अगर (CombiningCharacters.contains(UCS))
    वापस 0;

  अटल स्थिर UnicodeCharSet DoubleWidthCharacters(
      GeneratedDoubleWidthCharacterRanges);

  अगर (DoubleWidthCharacters.contains(UCS))
    वापस 2;
  वापस 1;
}

अटल bool isprintableascii(अक्षर c) { वापस c > 31 && c < 127; }

पूर्णांक columnWidthUTF8(StringRef Text) {
  अचिन्हित ColumnWidth = 0;
  अचिन्हित लंबाई;
  क्रम (माप_प्रकार i = 0, e = Text.आकार_2(); i < e; i += लंबाई) {
    लंबाई = get_अंक_Bytes_For_UTF8(Text[i]);

    // fast path for ASCII characters
    अगर (लंबाई == 1) {
      अगर (!isprintableascii(Text[i]))
        वापस ErrorNonPrintableCharacter;
      ColumnWidth += 1;
      जारी;
    }

    अगर (लंबाई <= 0 || i + लंबाई > Text.आकार_2())
      वापस ErrorInvalidUTF8;
    UTF32 buf[1];
    स्थिर UTF8 *Start = reinterpret_cast<स्थिर UTF8 *>(Text.तथ्य() + i);
    UTF32 *Target = &buf[0];
    अगर (conversionOK != ConvertUTF8toUTF32(&Start, Start + लंबाई, &Target,
                                           Target + 1, strictConversion))
      वापस ErrorInvalidUTF8;
    पूर्णांक Width = charWidth(buf[0]);
    अगर (Width < 0)
      वापस ErrorNonPrintableCharacter;
    ColumnWidth += Width;
  }
  वापस ColumnWidth;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
