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

#include "llvm/Support/Unicode.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/UnicodeCharRanges.h"

namespace llvm {
namespace sys {
namespace unicode {

extern const UnicodeCharSet::CharRanges GeneratedCharacterRanges;
extern const UnicodeCharSet::CharRanges GeneratedPrintableRanges;
extern const UnicodeCharSet::CharRanges GeneratedControlCharacterRanges;
extern const UnicodeCharSet::CharRanges GeneratedFormatCharacterRanges;
extern const UnicodeCharSet::CharRanges GeneratedCombiningCharacterRanges;
extern const UnicodeCharSet::CharRanges GeneratedDoubleWidthCharacterRanges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
bool isCharacter(цео UCS) {
  static const UnicodeCharSet Character(GeneratedCharacterRanges);
  врати Character.contains(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
bool isPrintable(цео UCS) {
  static const UnicodeCharSet Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  врати UCS == 0x00AD || Printables.contains(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
bool isControl(цео UCS) {
  static const UnicodeCharSet Control(GeneratedControlCharacterRanges);
  врати Control.contains(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
bool isFormatting(цео UCS) {
  static const UnicodeCharSet Format(GeneratedFormatCharacterRanges);
  врати Format.contains(UCS);
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
static inline цео charWidth(цео UCS) {
  ако (!isPrintable(UCS))
    врати ErrorNonPrintableCharacter;

  static const UnicodeCharSet CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  ако (CombiningCharacters.contains(UCS))
    врати 0;

  static const UnicodeCharSet DoubleWidthCharacters(
      GeneratedDoubleWidthCharacterRanges);

  ако (DoubleWidthCharacters.contains(UCS))
    врати 2;
  врати 1;
}

static bool isprintableascii(char c) { врати c > 31 && c < 127; }

цео columnWidthUTF8(StringRef Text) {
  unsigned ColumnWidth = 0;
  unsigned Length;
  за (size_t i = 0, e = Text.size(); i < e; i += Length) {
    Length = getNumBytesForUTF8(Text[i]);

    // fast path for ASCII characters
    ако (Length == 1) {
      ако (!isprintableascii(Text[i]))
        врати ErrorNonPrintableCharacter;
      ColumnWidth += 1;
      continue;
    }

    ако (Length <= 0 || i + Length > Text.size())
      врати ErrorInvalidUTF8;
    UTF32 buf[1];
    const UTF8 *Start = reinterpret_cast<const UTF8 *>(Text.data() + i);
    UTF32 *Target = &buf[0];
    ако (conversionOK != ConvertUTF8toUTF32(&Start, Start + Length, &Target,
                                           Target + 1, strictConversion))
      врати ErrorInvalidUTF8;
    цео Width = charWidth(buf[0]);
    ако (Width < 0)
      врати ErrorNonPrintableCharacter;
    ColumnWidth += Width;
  }
  врати ColumnWidth;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
