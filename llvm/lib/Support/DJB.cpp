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

#അന്തര്ഭാവ "llvm/Support/DJB.h"
#അന്തര്ഭാവ "llvm/ADT/ArrayRef.h"
#അന്തര്ഭാവ "llvm/Support/Compiler.h"
#അന്തര്ഭാവ "llvm/Support/ConvertUTF.h"
#അന്തര്ഭാവ "llvm/Support/Unicode.h"

using നാമാകാശ llvm;

സ്ഥിതം UTF32 chopOneUTF32(StringRef &Buffer) {
  UTF32 C;
  സ്ഥിരം UTF8 *സ്ഥിരം Begin8Const =
      reinterpret_cast<സ്ഥിരം UTF8 *>(Buffer.begin());
  സ്ഥിരം UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  പ്രതിജ്ഞാ(!Buffer.empty());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<സ്ഥിരം UTF8 *>(Buffer.end()),
                     &Begin32, &C + 1, lenientConversion);
  Buffer = Buffer.drop_front(Begin8 - Begin8Const);
  തിരികെ C;
}

സ്ഥിതം StringRef toUTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  സ്ഥിരം UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  ConversionResult CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.end(), strictConversion);
  പ്രതിജ്ഞാ(CR == conversionOK && "Case folding produced invalid char?");
  (ശൂന്യം)CR;
  തിരികെ StringRef(reinterpret_cast<അക്ഷരം *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

സ്ഥിതം UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  എങ്കിൽ (C == 0x130 || C == 0x131)
    തിരികെ 'i';
  തിരികെ sys::unicode::foldCharSimple(C);
}

സ്ഥിതം std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef Buffer,
                                                      uint32_t H) {
  തർക്കം AllASCII = സത്യം;
  വേണ്ടി (unsigned അക്ഷരം C : Buffer) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  എങ്കിൽ (AllASCII)
    തിരികെ H;
  തിരികെ std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef Buffer, uint32_t H) {
  എങ്കിൽ (std::optional<uint32_t> Result = fastCaseFoldingDjbHash(Buffer, H))
    തിരികെ *Result;

  std::array<UTF8, UNI_MAX_UTF8_BYTES_PER_CODE_POINT> Storage;
  ആകുമ്പോൾ (!Buffer.empty()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(Buffer));
    StringRef Folded = toUTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  തിരികെ H;
}
