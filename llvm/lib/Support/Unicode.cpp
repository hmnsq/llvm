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

बाह्य ध्रुव Unicode_Char_समुच्चयः::CharRanges GeneratedCharacterRanges;
बाह्य ध्रुव Unicode_Char_समुच्चयः::CharRanges GeneratedPrintableRanges;
बाह्य ध्रुव Unicode_Char_समुच्चयः::CharRanges GeneratedControlCharacterRanges;
बाह्य ध्रुव Unicode_Char_समुच्चयः::CharRanges GeneratedFormatCharacterRanges;
बाह्य ध्रुव Unicode_Char_समुच्चयः::CharRanges GeneratedCombiningCharacterRanges;
बाह्य ध्रुव Unicode_Char_समुच्चयः::CharRanges Generated_Double_विस्तारः_Character_Ranges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
तार्किक isCharacter(पूर्णाङ्क UCS) {
  स्थिर ध्रुव Unicode_Char_समुच्चयः Character(GeneratedCharacterRanges);
  प्रत्यागम Character.अन्तर्भवति(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
तार्किक isPrintable(पूर्णाङ्क UCS) {
  स्थिर ध्रुव Unicode_Char_समुच्चयः Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  प्रत्यागम UCS == 0x00AD || Printables.अन्तर्भवति(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
तार्किक isControl(पूर्णाङ्क UCS) {
  स्थिर ध्रुव Unicode_Char_समुच्चयः Control(GeneratedControlCharacterRanges);
  प्रत्यागम Control.अन्तर्भवति(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
तार्किक isFormatting(पूर्णाङ्क UCS) {
  स्थिर ध्रुव Unicode_Char_समुच्चयः Format(GeneratedFormatCharacterRanges);
  प्रत्यागम Format.अन्तर्भवति(UCS);
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
स्थिर अन्तःस्थ पूर्णाङ्क char_विस्तारः(पूर्णाङ्क UCS) {
  यदि (!isPrintable(UCS))
    प्रत्यागम ErrorNonPrintableCharacter;

  स्थिर ध्रुव Unicode_Char_समुच्चयः CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  यदि (CombiningCharacters.अन्तर्भवति(UCS))
    प्रत्यागम 0;

  स्थिर ध्रुव Unicode_Char_समुच्चयः Double_विस्तारः_Characters(
      Generated_Double_विस्तारः_Character_Ranges);

  यदि (Double_विस्तारः_Characters.अन्तर्भवति(UCS))
    प्रत्यागम 2;
  प्रत्यागम 1;
}

स्थिर तार्किक isprintableascii(वर्ण c) { प्रत्यागम c > 31 && c < 127; }

पूर्णाङ्क column_विस्तारः_UTF8(StringRef Text) {
  अचिह्नित Column_विस्तारः = 0;
  अचिह्नित दीर्घता;
  प्रति (size_t i = 0, e = Text.आकार_2(); i < e; i += दीर्घता) {
    दीर्घता = get_अङ्कः_अष्टकानि_For_UTF8(Text[i]);

    // fast path for ASCII characters
    यदि (दीर्घता == 1) {
      यदि (!isprintableascii(Text[i]))
        प्रत्यागम ErrorNonPrintableCharacter;
      Column_विस्तारः += 1;
      अनुवर्त;
    }

    यदि (दीर्घता <= 0 || i + दीर्घता > Text.आकार_2())
      प्रत्यागम ErrorInvalidUTF8;
    UTF32 buf[1];
    ध्रुव UTF8 *Start = reinterpret_cast<ध्रुव UTF8 *>(Text.दत्त() + i);
    UTF32 *Target = &buf[0];
    यदि (conversionOK != ConvertUTF8toUTF32(&Start, Start + दीर्घता, &Target,
                                           Target + 1, strictConversion))
      प्रत्यागम ErrorInvalidUTF8;
    पूर्णाङ्क विस्तारः = char_विस्तारः(buf[0]);
    यदि (विस्तारः < 0)
      प्रत्यागम ErrorNonPrintableCharacter;
    Column_विस्तारः += विस्तारः;
  }
  प्रत्यागम Column_विस्तारः;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
