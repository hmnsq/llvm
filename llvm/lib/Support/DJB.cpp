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

using فضاینام llvm;

ایستا UTF32 chopOneUTF32(StringRef &Buffer) {
  UTF32 C;
  ثابت UTF8 *ثابت Begin8Const =
      reinterpret_cast<ثابت UTF8 *>(Buffer.begin());
  ثابت UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  أكد(!Buffer.empty());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<ثابت UTF8 *>(Buffer.end()),
                     &Begin32, &C + 1, lenientConversion);
  Buffer = Buffer.drop_front(Begin8 - Begin8Const);
  بازگشت C;
}

ایستا StringRef toUTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  ثابت UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  ConversionResult CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.end(), strictConversion);
  أكد(CR == conversionOK && "Case folding produced invalid char?");
  (تهی)CR;
  بازگشت StringRef(reinterpret_cast<نویسه *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

ایستا UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  اگر (C == 0x130 || C == 0x131)
    بازگشت 'i';
  بازگشت sys::unicode::foldCharSimple(C);
}

ایستا std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef Buffer,
                                                      uint32_t H) {
  منطقی AllASCII = درست;
  برای (unsigned نویسه C : Buffer) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  اگر (AllASCII)
    بازگشت H;
  بازگشت std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef Buffer, uint32_t H) {
  اگر (std::optional<uint32_t> Result = fastCaseFoldingDjbHash(Buffer, H))
    بازگشت *Result;

  std::array<UTF8, UNI_MAX_UTF8_BYTES_PER_CODE_POINT> Storage;
  وقتی (!Buffer.empty()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(Buffer));
    StringRef Folded = toUTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  بازگشت H;
}
