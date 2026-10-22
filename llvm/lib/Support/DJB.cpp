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

#समावेश "llvm/Support/DJB.h"
#समावेश "llvm/ADT/ArrayRef.h"
#समावेश "llvm/Support/Compiler.h"
#समावेश "llvm/Support/ConvertUTF.h"
#समावेश "llvm/Support/Unicode.h"

using namespace llvm;

अटल UTF32 chopOneUTF32(StringRef &बफर) {
  UTF32 C;
  स्थिर UTF8 *स्थिर Begin8Const =
      reinterpret_cast<स्थिर UTF8 *>(बफर.begin());
  स्थिर UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  निश्चित(!बफर.empty());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<स्थिर UTF8 *>(बफर.end()),
                     &Begin32, &C + 1, lenientConversion);
  बफर = बफर.drop_front(Begin8 - Begin8Const);
  वापस C;
}

अटल StringRef toUTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  स्थिर UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  Conversion_परिणाम CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.end(), strictConversion);
  निश्चित(CR == conversionOK && "Case folding produced invalid char?");
  (व्योम)CR;
  वापस StringRef(reinterpret_cast<अक्षर *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

अटल UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  अगर (C == 0x130 || C == 0x131)
    वापस 'i';
  वापस sys::unicode::foldCharSimple(C);
}

अटल std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef बफर,
                                                      uint32_t H) {
  bool AllASCII = true;
  क्रम (अचिन्हित अक्षर C : बफर) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  अगर (AllASCII)
    वापस H;
  वापस std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef बफर, uint32_t H) {
  अगर (std::optional<uint32_t> परिणाम = fastCaseFoldingDjbHash(बफर, H))
    वापस *परिणाम;

  std::array<UTF8, UNI_MAX_UTF8_BYTES_PER_CODE_POINT> Storage;
  जबतक (!बफर.empty()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(बफर));
    StringRef Folded = toUTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  वापस H;
}
