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

#অন্তর্ভাৱ "llvm/Support/DJB.h"
#অন্তর্ভাৱ "llvm/ADT/ArrayRef.h"
#অন্তর্ভাৱ "llvm/Support/Compiler.h"
#অন্তর্ভাৱ "llvm/Support/ConvertUTF.h"
#অন্তর্ভাৱ "llvm/Support/Unicode.h"

using নামাকাশ llvm;

স্থির UTF32 chopOneUTF32(StringRef &Buffer) {
  UTF32 C;
  ধ্রুৱ UTF8 *ধ্রুৱ Begin8Const =
      reinterpret_cast<ধ্রুৱ UTF8 *>(Buffer.begin());
  ধ্রুৱ UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  প্রতিজ্ঞা(!Buffer.empty());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<ধ্রুৱ UTF8 *>(Buffer.end()),
                     &Begin32, &C + 1, lenientConversion);
  Buffer = Buffer.drop_front(Begin8 - Begin8Const);
  ঘূৰাই C;
}

স্থির StringRef toUTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  ধ্রুৱ UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  ConversionResult CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.end(), strictConversion);
  প্রতিজ্ঞা(CR == conversionOK && "Case folding produced invalid char?");
  (শূন্য)CR;
  ঘূৰাই StringRef(reinterpret_cast<ৱর্ণ *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

স্থির UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  যদি (C == 0x130 || C == 0x131)
    ঘূৰাই 'i';
  ঘূৰাই sys::unicode::foldCharSimple(C);
}

স্থির std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef Buffer,
                                                      uint32_t H) {
  তার্কিক AllASCII = সঁচা;
  বাবে (unsigned ৱর্ণ C : Buffer) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  যদি (AllASCII)
    ঘূৰাই H;
  ঘূৰাই std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef Buffer, uint32_t H) {
  যদি (std::optional<uint32_t> Result = fastCaseFoldingDjbHash(Buffer, H))
    ঘূৰাই *Result;

  std::array<UTF8, UNI_MAX_UTF8_BYTES_PER_CODE_POINT> Storage;
  যেতিয়ালৈ (!Buffer.empty()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(Buffer));
    StringRef Folded = toUTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  ঘূৰাই H;
}
