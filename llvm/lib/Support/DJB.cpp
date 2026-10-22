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

#ಅನ್ತರ್ಭಾವ "llvm/Support/DJB.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compiler.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ConvertUTF.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Unicode.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ UTF32 chopOneUTF32(StringRef &ಬಫರ್) {
  UTF32 C;
  ಸ್ಥಿರಾಂಕ UTF8 *ಸ್ಥಿರಾಂಕ Begin8Const =
      reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF8 *>(ಬಫರ್.begin());
  ಸ್ಥಿರಾಂಕ UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  ಪ್ರತಿಜ್ಞಾ(!ಬಫರ್.ಖಾಲಿಗಳು());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<ಸ್ಥಿರಾಂಕ UTF8 *>(ಬಫರ್.ಕೊನೆ()),
                     &Begin32, &C + 1, lenientConversion);
  ಬಫರ್ = ಬಫರ್.drop_front(Begin8 - Begin8Const);
  ಹಿಂತಿರುಗಿ C;
}

ಸ್ಥಿರ StringRef ಗಮ್ಯ_UTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  ಸ್ಥಿರಾಂಕ UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  Conversion_ಫಲಿತಾಂಶ CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.ಕೊನೆ(), strictConversion);
  ಪ್ರತಿಜ್ಞಾ(CR == conversionOK && "Case folding produced invalid char?");
  (ಶೂನ್ಯ)CR;
  ಹಿಂತಿರುಗಿ StringRef(reinterpret_cast<ಅಕ್ಷರ *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

ಸ್ಥಿರ UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  ಆದರೆ (C == 0x130 || C == 0x131)
    ಹಿಂತಿರುಗಿ 'i';
  ಹಿಂತಿರುಗಿ sys::unicode::foldCharSimple(C);
}

ಸ್ಥಿರ std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef ಬಫರ್,
                                                      uint32_t H) {
  ತಾರ್ಕಿಕ AllASCII = ನಿಜ;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ C : ಬಫರ್) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  ಆದರೆ (AllASCII)
    ಹಿಂತಿರುಗಿ H;
  ಹಿಂತಿರುಗಿ std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef ಬಫರ್, uint32_t H) {
  ಆದರೆ (std::optional<uint32_t> ಫಲಿತಾಂಶ = fastCaseFoldingDjbHash(ಬಫರ್, H))
    ಹಿಂತಿರುಗಿ *ಫಲಿತಾಂಶ;

  std::array<UTF8, UNI_MAX_UTF8_ಬೈಟುಗಳು_ಪ್ರತಿ_ಸಂಕೇತ_ಬಿಂದು> Storage;
  ಆಗ (!ಬಫರ್.ಖಾಲಿಗಳು()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(ಬಫರ್));
    StringRef Folded = ಗಮ್ಯ_UTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  ಹಿಂತಿರುಗಿ H;
}
