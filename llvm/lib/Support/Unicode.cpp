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

#అన్తర్భావ "llvm/Support/Unicode.h"
#అన్తర్భావ "llvm/Support/ConvertUTF.h"
#అన్తర్భావ "llvm/Support/UnicodeCharRanges.h"

నామాకాశ llvm {
నామాకాశ sys {
నామాకాశ unicode {

బాహ్య స్థిరరాశి Unicode_Char_సమితి::CharRanges GeneratedCharacterRanges;
బాహ్య స్థిరరాశి Unicode_Char_సమితి::CharRanges GeneratedPrintableRanges;
బాహ్య స్థిరరాశి Unicode_Char_సమితి::CharRanges GeneratedControlCharacterRanges;
బాహ్య స్థిరరాశి Unicode_Char_సమితి::CharRanges GeneratedFormatCharacterRanges;
బాహ్య స్థిరరాశి Unicode_Char_సమితి::CharRanges GeneratedCombiningCharacterRanges;
బాహ్య స్థిరరాశి Unicode_Char_సమితి::CharRanges Generated_Double_వెడల్పు_Character_Ranges;

/// Unicode code points that are considered characters. This function
/// returns true for any code point defined by Unicode as a character, false for
/// any code point defined as a non-character, as reserved, as a surrogate code
/// point, or not defined at all.
తార్కిక isCharacter(పూర్ణాంకం UCS) {
  స్థిర స్థిరరాశి Unicode_Char_సమితి Character(GeneratedCharacterRanges);
  తిరిగి Character.కలిగియుంది(UCS);
}

/// Unicode code points of the categories L, M, N, P, S and Zs are considered
/// printable.
/// In addition, U+00AD SOFT HYPHEN is also considered printable, as
/// it's actually displayed on most terminals. \return true if the character is
/// considered printable.
తార్కిక isPrintable(పూర్ణాంకం UCS) {
  స్థిర స్థిరరాశి Unicode_Char_సమితి Printables(GeneratedPrintableRanges);
  // Clang special cases 0x00AD (SOFT HYPHEN) which is rendered as an actual
  // hyphen in most terminals.
  తిరిగి UCS == 0x00AD || Printables.కలిగియుంది(UCS);
}

/// Unicode code points of the Cc category are considered
/// control characters.
తార్కిక isControl(పూర్ణాంకం UCS) {
  స్థిర స్థిరరాశి Unicode_Char_సమితి Control(GeneratedControlCharacterRanges);
  తిరిగి Control.కలిగియుంది(UCS);
}

/// Unicode code points of the Cf category are considered
/// formatting characters.
తార్కిక isFormatting(పూర్ణాంకం UCS) {
  స్థిర స్థిరరాశి Unicode_Char_సమితి Format(GeneratedFormatCharacterRanges);
  తిరిగి Format.కలిగియుంది(UCS);
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
స్థిర అంతఃవరుస పూర్ణాంకం char_వెడల్పు(పూర్ణాంకం UCS) {
  ఒకవేళ (!isPrintable(UCS))
    తిరిగి ErrorNonPrintableCharacter;

  స్థిర స్థిరరాశి Unicode_Char_సమితి CombiningCharacters(
      GeneratedCombiningCharacterRanges);

  ఒకవేళ (CombiningCharacters.కలిగియుంది(UCS))
    తిరిగి 0;

  స్థిర స్థిరరాశి Unicode_Char_సమితి Double_వెడల్పు_Characters(
      Generated_Double_వెడల్పు_Character_Ranges);

  ఒకవేళ (Double_వెడల్పు_Characters.కలిగియుంది(UCS))
    తిరిగి 2;
  తిరిగి 1;
}

స్థిర తార్కిక isprintableascii(అక్షరం c) { తిరిగి c > 31 && c < 127; }

పూర్ణాంకం స్తంభం_వెడల్పు_UTF8(StringRef పాఠ్యం_2) {
  గుర్తులేని స్తంభం_వెడల్పు_2 = 0;
  గుర్తులేని నిడివి;
  కొరకు (size_t i = 0, e = పాఠ్యం_2.కొలత_2(); i < e; i += నిడివి) {
    నిడివి = get_అంకె_బైట్లు_For_UTF8(పాఠ్యం_2[i]);

    // fast path for ASCII characters
    ఒకవేళ (నిడివి == 1) {
      ఒకవేళ (!isprintableascii(పాఠ్యం_2[i]))
        తిరిగి ErrorNonPrintableCharacter;
      స్తంభం_వెడల్పు_2 += 1;
      కొనసాగు;
    }

    ఒకవేళ (నిడివి <= 0 || i + నిడివి > పాఠ్యం_2.కొలత_2())
      తిరిగి ErrorInvalidUTF8;
    UTF32 buf[1];
    స్థిరరాశి UTF8 *ప్రారంభం = reinterpret_cast<స్థిరరాశి UTF8 *>(పాఠ్యం_2.దత్తాంశం() + i);
    UTF32 *లక్ష్యం_2 = &buf[0];
    ఒకవేళ (conversionOK != ConvertUTF8toUTF32(&ప్రారంభం, ప్రారంభం + నిడివి, &లక్ష్యం_2,
                                           లక్ష్యం_2 + 1, strictConversion))
      తిరిగి ErrorInvalidUTF8;
    పూర్ణాంకం వెడల్పు = char_వెడల్పు(buf[0]);
    ఒకవేళ (వెడల్పు < 0)
      తిరిగి ErrorNonPrintableCharacter;
    స్తంభం_వెడల్పు_2 += వెడల్పు;
  }
  తిరిగి స్తంభం_వెడల్పు_2;
}

} // namespace unicode
} // namespace sys
} // namespace llvm
