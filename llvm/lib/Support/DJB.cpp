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

#অন্তর্ভাব "llvm/Support/DJB.h"
#অন্তর্ভাব "llvm/ADT/ArrayRef.h"
#অন্তর্ভাব "llvm/Support/Compiler.h"
#অন্তর্ভাব "llvm/Support/ConvertUTF.h"
#অন্তর্ভাব "llvm/Support/Unicode.h"

using নামাকাশ llvm;

স্থির UTF32 chopOneUTF32(StringRef &বাফার) {
  UTF32 C;
  ধ্রুবক UTF8 *ধ্রুবক Begin8Const =
      reinterpret_cast<ধ্রুবক UTF8 *>(বাফার.begin());
  ধ্রুবক UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  প্রতিজ্ঞা(!বাফার.empty());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<ধ্রুবক UTF8 *>(বাফার.end()),
                     &Begin32, &C + 1, lenientConversion);
  বাফার = বাফার.drop_front(Begin8 - Begin8Const);
  ফেরত C;
}

স্থির StringRef toUTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  ধ্রুবক UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  Conversion_ফলাফল CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.end(), strictConversion);
  প্রতিজ্ঞা(CR == conversionOK && "Case folding produced invalid char?");
  (শূন্য)CR;
  ফেরত StringRef(reinterpret_cast<অক্ষর *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

স্থির UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  যদি (C == 0x130 || C == 0x131)
    ফেরত 'i';
  ফেরত sys::unicode::foldCharSimple(C);
}

স্থির std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef বাফার,
                                                      uint32_t H) {
  যৌক্তিক AllASCII = সত্য;
  জন্য (unsigned অক্ষর C : বাফার) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  যদি (AllASCII)
    ফেরত H;
  ফেরত std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef বাফার, uint32_t H) {
  যদি (std::optional<uint32_t> ফলাফল = fastCaseFoldingDjbHash(বাফার, H))
    ফেরত *ফলাফল;

  std::array<UTF8, UNI_MAX_UTF8_BYTES_PER_CODE_POINT> Storage;
  যখন (!বাফার.empty()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(বাফার));
    StringRef Folded = toUTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  ফেরত H;
}
