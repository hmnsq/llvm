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

স্থির UTF32 chopOneUTF32(StringRef &Buffer) {
  UTF32 C;
  ধ্রুব UTF8 *ধ্রুব Begin8Const =
      reinterpret_cast<ধ্রুব UTF8 *>(Buffer.begin());
  ধ্রুব UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  প্রতিজ্ঞা(!Buffer.empty());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<ধ্রুব UTF8 *>(Buffer.end()),
                     &Begin32, &C + 1, lenientConversion);
  Buffer = Buffer.drop_front(Begin8 - Begin8Const);
  প্রত্যাগম C;
}

স্থির StringRef toUTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  ধ্রুব UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  ConversionResult CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.end(), strictConversion);
  প্রতিজ্ঞা(CR == conversionOK && "Case folding produced invalid char?");
  (শূন্য)CR;
  প্রত্যাগম StringRef(reinterpret_cast<বর্ণ *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

স্থির UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  যদি (C == 0x130 || C == 0x131)
    প্রত্যাগম 'i';
  প্রত্যাগম sys::unicode::foldCharSimple(C);
}

স্থির std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef Buffer,
                                                      uint32_t H) {
  তার্কিক AllASCII = সত্য;
  প্রতি (unsigned বর্ণ C : Buffer) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  যদি (AllASCII)
    প্রত্যাগম H;
  প্রত্যাগম std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef Buffer, uint32_t H) {
  যদি (std::optional<uint32_t> Result = fastCaseFoldingDjbHash(Buffer, H))
    প্রত্যাগম *Result;

  std::array<UTF8, UNI_MAX_UTF8_BYTES_PER_CODE_POINT> Storage;
  যাবত্ (!Buffer.empty()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(Buffer));
    StringRef Folded = toUTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  প্রত্যাগম H;
}
