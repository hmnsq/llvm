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

using ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರ UTF32 chopOneUTF32(StringRef &Buffer) {
  UTF32 C;
  ಧ್ರುವ UTF8 *ಧ್ರುವ Begin8Const =
      reinterpret_cast<ಧ್ರುವ UTF8 *>(Buffer.begin());
  ಧ್ರುವ UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  ಪ್ರತಿಜ್ಞಾ(!Buffer.empty());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<ಧ್ರುವ UTF8 *>(Buffer.end()),
                     &Begin32, &C + 1, lenientConversion);
  Buffer = Buffer.drop_front(Begin8 - Begin8Const);
  ಪ್ರತ್ಯಾಗಮ C;
}

ಸ್ಥಿರ StringRef toUTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  ಧ್ರುವ UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  ConversionResult CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.end(), strictConversion);
  ಪ್ರತಿಜ್ಞಾ(CR == conversionOK && "Case folding produced invalid char?");
  (ಶೂನ್ಯ)CR;
  ಪ್ರತ್ಯಾಗಮ StringRef(reinterpret_cast<ವರ್ಣ *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

ಸ್ಥಿರ UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  ಯದಿ (C == 0x130 || C == 0x131)
    ಪ್ರತ್ಯಾಗಮ 'i';
  ಪ್ರತ್ಯಾಗಮ sys::unicode::foldCharSimple(C);
}

ಸ್ಥಿರ std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef Buffer,
                                                      uint32_t H) {
  ತಾರ್ಕಿಕ AllASCII = ಸತ್ಯ;
  ಪ್ರತಿ (unsigned ವರ್ಣ C : Buffer) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  ಯದಿ (AllASCII)
    ಪ್ರತ್ಯಾಗಮ H;
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef Buffer, uint32_t H) {
  ಯದಿ (std::optional<uint32_t> Result = fastCaseFoldingDjbHash(Buffer, H))
    ಪ್ರತ್ಯಾಗಮ *Result;

  std::array<UTF8, UNI_MAX_UTF8_BYTES_PER_CODE_POINT> Storage;
  ಯಾವತ್ (!Buffer.empty()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(Buffer));
    StringRef Folded = toUTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  ಪ್ರತ್ಯಾಗಮ H;
}
