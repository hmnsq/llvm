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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/DJB.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ArrayRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Compiler.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ConvertUTF.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Unicode.h"

using ନାମାକାଶ llvm;

ସ୍ଥିର UTF32 chopOneUTF32(StringRef &Buffer) {
  UTF32 C;
  ଅଚଳ UTF8 *ଅଚଳ Begin8Const =
      reinterpret_cast<ଅଚଳ UTF8 *>(Buffer.begin());
  ଅଚଳ UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  ପ୍ରତିଜ୍ଞା(!Buffer.empty());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<ଅଚଳ UTF8 *>(Buffer.end()),
                     &Begin32, &C + 1, lenientConversion);
  Buffer = Buffer.drop_front(Begin8 - Begin8Const);
  ଫେରନ୍ତୁ C;
}

ସ୍ଥିର StringRef toUTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  ଅଚଳ UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  ConversionResult CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.end(), strictConversion);
  ପ୍ରତିଜ୍ଞା(CR == conversionOK && "Case folding produced invalid char?");
  (ଶୂନ୍ୟ)CR;
  ଫେରନ୍ତୁ StringRef(reinterpret_cast<ଅକ୍ଷର *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

ସ୍ଥିର UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  ଯଦି (C == 0x130 || C == 0x131)
    ଫେରନ୍ତୁ 'i';
  ଫେରନ୍ତୁ sys::unicode::foldCharSimple(C);
}

ସ୍ଥିର std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef Buffer,
                                                      uint32_t H) {
  ତାର୍କିକ AllASCII = ସତ୍ୟ;
  ପାଇଁ (unsigned ଅକ୍ଷର C : Buffer) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  ଯଦି (AllASCII)
    ଫେରନ୍ତୁ H;
  ଫେରନ୍ତୁ std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef Buffer, uint32_t H) {
  ଯଦି (std::optional<uint32_t> Result = fastCaseFoldingDjbHash(Buffer, H))
    ଫେରନ୍ତୁ *Result;

  std::array<UTF8, UNI_MAX_UTF8_BYTES_PER_CODE_POINT> Storage;
  ଯେତେବେଳେ (!Buffer.empty()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(Buffer));
    StringRef Folded = toUTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  ଫେରନ୍ତୁ H;
}
