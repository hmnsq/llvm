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

#અન્તર્ભાવ "llvm/Support/DJB.h"
#અન્તર્ભાવ "llvm/ADT/ArrayRef.h"
#અન્તર્ભાવ "llvm/Support/Compiler.h"
#અન્તર્ભાવ "llvm/Support/ConvertUTF.h"
#અન્તર્ભાવ "llvm/Support/Unicode.h"

using નામાકાશ llvm;

સ્થિર UTF32 chopOneUTF32(StringRef &Buffer) {
  UTF32 C;
  અચલ UTF8 *અચલ Begin8Const =
      reinterpret_cast<અચલ UTF8 *>(Buffer.begin());
  અચલ UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  પ્રતિજ્ઞા(!Buffer.empty());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<અચલ UTF8 *>(Buffer.end()),
                     &Begin32, &C + 1, lenientConversion);
  Buffer = Buffer.drop_front(Begin8 - Begin8Const);
  પાછા C;
}

સ્થિર StringRef toUTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  અચલ UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  ConversionResult CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.end(), strictConversion);
  પ્રતિજ્ઞા(CR == conversionOK && "Case folding produced invalid char?");
  (શૂન્ય)CR;
  પાછા StringRef(reinterpret_cast<અક્ષર *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

સ્થિર UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  જો (C == 0x130 || C == 0x131)
    પાછા 'i';
  પાછા sys::unicode::foldCharSimple(C);
}

સ્થિર std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef Buffer,
                                                      uint32_t H) {
  તાર્કિક AllASCII = સાચું;
  માટે (unsigned અક્ષર C : Buffer) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  જો (AllASCII)
    પાછા H;
  પાછા std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef Buffer, uint32_t H) {
  જો (std::optional<uint32_t> Result = fastCaseFoldingDjbHash(Buffer, H))
    પાછા *Result;

  std::array<UTF8, UNI_MAX_UTF8_BYTES_PER_CODE_POINT> Storage;
  જ્યારે (!Buffer.empty()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(Buffer));
    StringRef Folded = toUTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  પાછા H;
}
