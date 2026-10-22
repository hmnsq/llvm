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

#அந்தர்பாவ "llvm/Support/DJB.h"
#அந்தர்பாவ "llvm/ADT/ArrayRef.h"
#அந்தர்பாவ "llvm/Support/Compiler.h"
#அந்தர்பாவ "llvm/Support/ConvertUTF.h"
#அந்தர்பாவ "llvm/Support/Unicode.h"

using நாமாகாஶ llvm;

நிலையான UTF32 chopOneUTF32(StringRef &Buffer) {
  UTF32 C;
  மாறிலி UTF8 *மாறிலி Begin8Const =
      reinterpret_cast<மாறிலி UTF8 *>(Buffer.begin());
  மாறிலி UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  ப்ரதிஜ்ஞா(!Buffer.empty());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<மாறிலி UTF8 *>(Buffer.end()),
                     &Begin32, &C + 1, lenientConversion);
  Buffer = Buffer.drop_front(Begin8 - Begin8Const);
  திரும்பு C;
}

நிலையான StringRef toUTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  மாறிலி UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  ConversionResult CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.end(), strictConversion);
  ப்ரதிஜ்ஞா(CR == conversionOK && "Case folding produced invalid char?");
  (வெறுமை)CR;
  திரும்பு StringRef(reinterpret_cast<எழுத்து *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

நிலையான UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  எனில் (C == 0x130 || C == 0x131)
    திரும்பு 'i';
  திரும்பு sys::unicode::foldCharSimple(C);
}

நிலையான std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef Buffer,
                                                      uint32_t H) {
  தருக்க AllASCII = உண்மை;
  க்கு (unsigned எழுத்து C : Buffer) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  எனில் (AllASCII)
    திரும்பு H;
  திரும்பு std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef Buffer, uint32_t H) {
  எனில் (std::optional<uint32_t> Result = fastCaseFoldingDjbHash(Buffer, H))
    திரும்பு *Result;

  std::array<UTF8, UNI_MAX_UTF8_BYTES_PER_CODE_POINT> Storage;
  போது (!Buffer.empty()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(Buffer));
    StringRef Folded = toUTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  திரும்பு H;
}
