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

using नामाकाश llvm;

स्थिर UTF32 chopOneUTF32(StringRef &Buffer) {
  UTF32 C;
  ध्रुव UTF8 *ध्रुव Begin8Const =
      reinterpret_cast<ध्रुव UTF8 *>(Buffer.begin());
  ध्रुव UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  प्रतिज्ञा(!Buffer.empty());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<ध्रुव UTF8 *>(Buffer.end()),
                     &Begin32, &C + 1, lenientConversion);
  Buffer = Buffer.drop_front(Begin8 - Begin8Const);
  प्रत्यागम C;
}

स्थिर StringRef toUTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  ध्रुव UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  ConversionResult CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.end(), strictConversion);
  प्रतिज्ञा(CR == conversionOK && "Case folding produced invalid char?");
  (शून्य)CR;
  प्रत्यागम StringRef(reinterpret_cast<वर्ण *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

स्थिर UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  जइ (C == 0x130 || C == 0x131)
    प्रत्यागम 'i';
  प्रत्यागम sys::unicode::foldCharSimple(C);
}

स्थिर std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef Buffer,
                                                      uint32_t H) {
  तार्किक AllASCII = सच्च;
  प्रति (unsigned वर्ण C : Buffer) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  जइ (AllASCII)
    प्रत्यागम H;
  प्रत्यागम std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef Buffer, uint32_t H) {
  जइ (std::optional<uint32_t> Result = fastCaseFoldingDjbHash(Buffer, H))
    प्रत्यागम *Result;

  std::array<UTF8, UNI_MAX_UTF8_BYTES_PER_CODE_POINT> Storage;
  जाव (!Buffer.empty()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(Buffer));
    StringRef Folded = toUTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  प्रत्यागम H;
}
