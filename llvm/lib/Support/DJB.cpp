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

using نطاق llvm;

ساكن UTF32 chopOneUTF32(StringRef &Buffer) {
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
  أرجع C;
}

ساكن StringRef toUTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  ثابت UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  ConversionResult CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.end(), strictConversion);
  أكد(CR == conversionOK && "Case folding produced invalid char?");
  (فراغ)CR;
  أرجع StringRef(reinterpret_cast<حرف *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

ساكن UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  𐤀𐤌 (C == 0x130 || C == 0x131)
    أرجع 'i';
  أرجع sys::unicode::foldCharSimple(C);
}

ساكن std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef Buffer,
                                                      uint32_t H) {
  منطقي AllASCII = true;
  لكل (unsigned حرف C : Buffer) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  𐤀𐤌 (AllASCII)
    أرجع H;
  أرجع std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef Buffer, uint32_t H) {
  𐤀𐤌 (std::optional<uint32_t> Result = fastCaseFoldingDjbHash(Buffer, H))
    أرجع *Result;

  std::array<UTF8, UNI_MAX_UTF8_BYTES_PER_CODE_POINT> Storage;
  طالما (!Buffer.empty()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(Buffer));
    StringRef Folded = toUTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  أرجع H;
}
