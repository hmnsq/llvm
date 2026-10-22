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

#അന്തര്ഭാവ "llvm/Support/Unicode.h"
#അന്തര്ഭാവ "llvm/Support/ConvertUTF.h"
#അന്തര്ഭാവ "llvm/Support/UnicodeCharRanges.h"

നാമാകാശ llvm {
നാമാകാശ sys {
നാമാകാശ unicode {

ബാഹ്യ സ്ഥിരം UnicodeCharSet::CharRanges GeneratedCharacterRanges;
ബാഹ്യ സ്ഥിരം UnicodeCharSet::CharRanges GeneratedPrintableRanges;
ബാഹ്യ സ്ഥിരം UnicodeCharSet::CharRanges GeneratedControlCharacterRanges;
ബാഹ്യ സ്ഥിരം UnicodeCharSet::CharRanges GeneratedFormatCharacterRanges;
ബാഹ്യ സ്ഥിരം UnicodeCharSet::CharRanges GeneratedCombiningCharacterRanges;
ബാഹ്യ സ്ഥിരം UnicodeCharSet::CharRanges GeneratedDoubleWidthCharacterRanges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
തർക്കം isCharacter(പൂർണ്ണസംഖ്യ UCS) {
  സ്ഥിതം സ്ഥിരം UnicodeCharSet Character(GeneratedCharacterRanges);
  തിരികെ Character.contains(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
തർക്കം isPrintable(പൂർണ്ണസംഖ്യ UCS) {
  സ്ഥിതം സ്ഥിരം UnicodeCharSet Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  തിരികെ UCS == 0x00AD || Printables.contains(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
തർക്കം isControl(പൂർണ്ണസംഖ്യ UCS) {
  സ്ഥിതം സ്ഥിരം UnicodeCharSet Control(GeneratedControlCharacterRanges);
  തിരികെ Control.contains(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
തർക്കം isFormatting(പൂർണ്ണസംഖ്യ UCS) {
  സ്ഥിതം സ്ഥിരം UnicodeCharSet Format(GeneratedFormatCharacterRanges);
  തിരികെ Format.contains(UCS);
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
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ charWidth(പൂർണ്ണസംഖ്യ UCS) {
  എങ്കിൽ (!isPrintable(UCS))
    തിരികെ ErrorNonPrintableCharacter;

  സ്ഥിതം സ്ഥിരം UnicodeCharSet CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  എങ്കിൽ (CombiningCharacters.contains(UCS))
    തിരികെ 0;

  സ്ഥിതം സ്ഥിരം UnicodeCharSet DoubleWidthCharacters(
      GeneratedDoubleWidthCharacterRanges);

  എങ്കിൽ (DoubleWidthCharacters.contains(UCS))
    തിരികെ 2;
  തിരികെ 1;
}

സ്ഥിതം തർക്കം isprintableascii(അക്ഷരം c) { തിരികെ c > 31 && c < 127; }

പൂർണ്ണസംഖ്യ columnWidthUTF8(StringRef Text) {
  unsigned ColumnWidth = 0;
  unsigned Length;
  വേണ്ടി (size_t i = 0, e = Text.size(); i < e; i += Length) {
    Length = get_അക്കം_Bytes_For_UTF8(Text[i]);

    // fast path for ASCII characters
    എങ്കിൽ (Length == 1) {
      എങ്കിൽ (!isprintableascii(Text[i]))
        തിരികെ ErrorNonPrintableCharacter;
      ColumnWidth += 1;
      തുടരുക;
    }

    എങ്കിൽ (Length <= 0 || i + Length > Text.size())
      തിരികെ ErrorInvalidUTF8;
    UTF32 buf[1];
    സ്ഥിരം UTF8 *Start = reinterpret_cast<സ്ഥിരം UTF8 *>(Text.data() + i);
    UTF32 *Target = &buf[0];
    എങ്കിൽ (conversionOK != ConvertUTF8toUTF32(&Start, Start + Length, &Target,
                                           Target + 1, strictConversion))
      തിരികെ ErrorInvalidUTF8;
    പൂർണ്ണസംഖ്യ Width = charWidth(buf[0]);
    എങ്കിൽ (Width < 0)
      തിരികെ ErrorNonPrintableCharacter;
    ColumnWidth += Width;
  }
  തിരികെ ColumnWidth;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
