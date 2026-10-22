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

#شامل "llvm/Support/DJB.h"
#شامل "llvm/ADT/ArrayRef.h"
#شامل "llvm/Support/Compiler.h"
#شامل "llvm/Support/ConvertUTF.h"
#شامل "llvm/Support/Unicode.h"

استعمال نامخلا llvm;

ساکن UTF32 chopOneUTF32(StringRef &بفر) {
  UTF32 C;
  ثابت UTF8 *ثابت Begin8Const =
      reinterpret_cast<ثابت UTF8 *>(بفر.begin());
  ثابت UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  أكد(!بفر.خالی_خانے());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<ثابت UTF8 *>(بفر.انتہا()),
                     &Begin32, &C + 1, lenientConversion);
  بفر = بفر.drop_front(Begin8 - Begin8Const);
  واپس C;
}

ساکن StringRef مقصد_UTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  ثابت UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  Conversion_نتیجہ CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.انتہا(), strictConversion);
  أكد(CR == conversionOK && "Case folding produced invalid char?");
  (خالی)CR;
  واپس StringRef(reinterpret_cast<حرف *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

ساکن UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  اگر (C == 0x130 || C == 0x131)
    واپس 'i';
  واپس sys::unicode::foldCharSimple(C);
}

ساکن std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef بفر,
                                                      uint32_t H) {
  منطقی AllASCII = سچ;
  برائے (غیرعلامتی حرف C : بفر) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  اگر (AllASCII)
    واپس H;
  واپس std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef بفر, uint32_t H) {
  اگر (std::optional<uint32_t> نتیجہ = fastCaseFoldingDjbHash(بفر, H))
    واپس *نتیجہ;

  std::array<UTF8, UNI_MAX_UTF8_بائٹیں_فی_رمز_نقطہ> Storage;
  جب (!بفر.خالی_خانے()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(بفر));
    StringRef Folded = مقصد_UTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  واپس H;
}
