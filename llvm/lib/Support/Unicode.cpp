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

#அந்தர்பாவ "llvm/Support/Unicode.h"
#அந்தர்பாவ "llvm/Support/ConvertUTF.h"
#அந்தர்பாவ "llvm/Support/UnicodeCharRanges.h"

நாமாகாஶ llvm {
நாமாகாஶ sys {
நாமாகாஶ unicode {

வெளி மாறிலி UnicodeCharSet::CharRanges GeneratedCharacterRanges;
வெளி மாறிலி UnicodeCharSet::CharRanges GeneratedPrintableRanges;
வெளி மாறிலி UnicodeCharSet::CharRanges GeneratedControlCharacterRanges;
வெளி மாறிலி UnicodeCharSet::CharRanges GeneratedFormatCharacterRanges;
வெளி மாறிலி UnicodeCharSet::CharRanges GeneratedCombiningCharacterRanges;
வெளி மாறிலி UnicodeCharSet::CharRanges GeneratedDoubleWidthCharacterRanges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
தருக்க isCharacter(முழு UCS) {
  நிலையான மாறிலி UnicodeCharSet Character(GeneratedCharacterRanges);
  திரும்பு Character.contains(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
தருக்க isPrintable(முழு UCS) {
  நிலையான மாறிலி UnicodeCharSet Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  திரும்பு UCS == 0x00AD || Printables.contains(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
தருக்க isControl(முழு UCS) {
  நிலையான மாறிலி UnicodeCharSet Control(GeneratedControlCharacterRanges);
  திரும்பு Control.contains(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
தருக்க isFormatting(முழு UCS) {
  நிலையான மாறிலி UnicodeCharSet Format(GeneratedFormatCharacterRanges);
  திரும்பு Format.contains(UCS);
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
நிலையான உள்வரி முழு charWidth(முழு UCS) {
  எனில் (!isPrintable(UCS))
    திரும்பு ErrorNonPrintableCharacter;

  நிலையான மாறிலி UnicodeCharSet CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  எனில் (CombiningCharacters.contains(UCS))
    திரும்பு 0;

  நிலையான மாறிலி UnicodeCharSet DoubleWidthCharacters(
      GeneratedDoubleWidthCharacterRanges);

  எனில் (DoubleWidthCharacters.contains(UCS))
    திரும்பு 2;
  திரும்பு 1;
}

நிலையான தருக்க isprintableascii(எழுத்து c) { திரும்பு c > 31 && c < 127; }

முழு columnWidthUTF8(StringRef Text) {
  unsigned ColumnWidth = 0;
  unsigned Length;
  க்கு (size_t i = 0, e = Text.size(); i < e; i += Length) {
    Length = get_இலக்கம்_Bytes_For_UTF8(Text[i]);

    // fast path for ASCII characters
    எனில் (Length == 1) {
      எனில் (!isprintableascii(Text[i]))
        திரும்பு ErrorNonPrintableCharacter;
      ColumnWidth += 1;
      தொடர்;
    }

    எனில் (Length <= 0 || i + Length > Text.size())
      திரும்பு ErrorInvalidUTF8;
    UTF32 buf[1];
    மாறிலி UTF8 *Start = reinterpret_cast<மாறிலி UTF8 *>(Text.data() + i);
    UTF32 *Target = &buf[0];
    எனில் (conversionOK != ConvertUTF8toUTF32(&Start, Start + Length, &Target,
                                           Target + 1, strictConversion))
      திரும்பு ErrorInvalidUTF8;
    முழு Width = charWidth(buf[0]);
    எனில் (Width < 0)
      திரும்பு ErrorNonPrintableCharacter;
    ColumnWidth += Width;
  }
  திரும்பு ColumnWidth;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
