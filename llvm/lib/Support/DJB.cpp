//===-- Support/DJB.cpp ---DJB Hash -----------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains support for the DJ Bernstein hash function.
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/DJB.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/Support/Compiler.h"
#अन्तर्भाव "llvm/Support/ConvertUTF.h"
#अन्तर्भाव "llvm/Support/Unicode.h"

उपयुज्य नामाकाश llvm;

स्थिर UTF32 chopOneUTF32(StringRef &बफर) {
  UTF32 C;
  ध्रुव UTF8 *ध्रुव Begin8Const =
      reinterpret_cast<ध्रुव UTF8 *>(बफर.begin());
  ध्रुव UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  प्रतिज्ञा(!बफर.रिक्तानि());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<ध्रुव UTF8 *>(बफर.अन्तः()),
                     &Begin32, &C + 1, lenientConversion);
  बफर = बफर.drop_front(Begin8 - Begin8Const);
  प्रत्यागम C;
}

स्थिर StringRef गन्तव्यम्_UTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  ध्रुव UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  Conversion_परिणाम CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.अन्तः(), strictConversion);
  प्रतिज्ञा(CR == conversionOK && "Case folding produced invalid char?");
  (शून्य)CR;
  प्रत्यागम StringRef(reinterpret_cast<वर्ण *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

स्थिर UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  यदि (C == 0x130 || C == 0x131)
    प्रत्यागम 'i';
  प्रत्यागम sys::unicode::foldCharSimple(C);
}

स्थिर std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef बफर,
                                                      uint32_t H) {
  तार्किक AllASCII = सत्य;
  प्रति (अचिह्नित वर्ण C : बफर) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  यदि (AllASCII)
    प्रत्यागम H;
  प्रत्यागम std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef बफर, uint32_t H) {
  यदि (std::optional<uint32_t> परिणाम = fastCaseFoldingDjbHash(बफर, H))
    प्रत्यागम *परिणाम;

  std::array<UTF8, UNI_MAX_UTF8_अष्टकानि_प्रत्येकम्_संहिता_POINT> Storage;
  यावत् (!बफर.रिक्तानि()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(बफर));
    StringRef Folded = गन्तव्यम्_UTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  प्रत्यागम H;
}
