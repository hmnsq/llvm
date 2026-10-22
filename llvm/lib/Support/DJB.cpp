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

#ضمن "llvm/Support/DJB.h"
#ضمن "llvm/ADT/ArrayRef.h"
#ضمن "llvm/Support/Compiler.h"
#ضمن "llvm/Support/ConvertUTF.h"
#ضمن "llvm/Support/Unicode.h"

استفاده نطاق llvm;

ساكن UTF32 chopOneUTF32(StringRef &میانگیر) {
  UTF32 C;
  ثابت UTF8 *ثابت Begin8Const =
      reinterpret_cast<ثابت UTF8 *>(میانگیر.begin());
  ثابت UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  أكد(!میانگیر.خالی());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<ثابت UTF8 *>(میانگیر.انتها()),
                     &Begin32, &C + 1, lenientConversion);
  میانگیر = میانگیر.drop_front(Begin8 - Begin8Const);
  أرجع C;
}

ساكن StringRef مقصد_UTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  ثابت UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  Conversion_نتیجه CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.انتها(), strictConversion);
  أكد(CR == conversionOK && "Case folding produced invalid char?");
  (فراغ)CR;
  أرجع StringRef(reinterpret_cast<حرف *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

ساكن UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  إذا (C == 0x130 || C == 0x131)
    أرجع 'i';
  أرجع sys::unicode::foldCharSimple(C);
}

ساكن std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef میانگیر,
                                                      uint32_t H) {
  منطقي AllASCII = درست;
  لكل (بیعلامت حرف C : میانگیر) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  إذا (AllASCII)
    أرجع H;
  أرجع std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef میانگیر, uint32_t H) {
  إذا (std::optional<uint32_t> نتیجه = fastCaseFoldingDjbHash(میانگیر, H))
    أرجع *نتیجه;

  std::array<UTF8, UNI_MAX_UTF8_بایتها_در_هر_کد_نقطه> Storage;
  طالما (!میانگیر.خالی()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(میانگیر));
    StringRef Folded = مقصد_UTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  أرجع H;
}
