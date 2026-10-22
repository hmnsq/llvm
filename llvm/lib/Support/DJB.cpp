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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/DJB.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ArrayRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Compiler.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ConvertUTF.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Unicode.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਸਥਿਰ UTF32 chopOneUTF32(StringRef &ਬਫਰ) {
  UTF32 C;
  ਅਚਲ UTF8 *ਅਚਲ Begin8Const =
      reinterpret_cast<ਅਚਲ UTF8 *>(ਬਫਰ.begin());
  ਅਚਲ UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  ਪ੍ਰਤਿਜ੍ਞਾ(!ਬਫਰ.ਖਾਲੀ_ਥਾਵਾਂ());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<ਅਚਲ UTF8 *>(ਬਫਰ.ਸਿਰਾ()),
                     &Begin32, &C + 1, lenientConversion);
  ਬਫਰ = ਬਫਰ.drop_front(Begin8 - Begin8Const);
  ਵਾਪਸ C;
}

ਸਥਿਰ StringRef ਮੰਜ਼ਿਲ_UTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  ਅਚਲ UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  Conversion_ਨਤੀਜਾ CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.ਸਿਰਾ(), strictConversion);
  ਪ੍ਰਤਿਜ੍ਞਾ(CR == conversionOK && "Case folding produced invalid char?");
  (ਖਾਲੀ)CR;
  ਵਾਪਸ StringRef(reinterpret_cast<ਅੱਖਰ *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

ਸਥਿਰ UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  ਜੇ (C == 0x130 || C == 0x131)
    ਵਾਪਸ 'i';
  ਵਾਪਸ sys::unicode::foldCharSimple(C);
}

ਸਥਿਰ std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef ਬਫਰ,
                                                      uint32_t H) {
  ਤਾਰਕਿਕ AllASCII = ਸੱਚ;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ C : ਬਫਰ) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  ਜੇ (AllASCII)
    ਵਾਪਸ H;
  ਵਾਪਸ std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef ਬਫਰ, uint32_t H) {
  ਜੇ (std::optional<uint32_t> ਨਤੀਜਾ = fastCaseFoldingDjbHash(ਬਫਰ, H))
    ਵਾਪਸ *ਨਤੀਜਾ;

  std::array<UTF8, UNI_MAX_UTF8_ਬਾਈਟਾਂ_ਪ੍ਰਤੀ_ਕੋਡ_ਬਿੰਦੂ> Storage;
  ਜਦੋਂ (!ਬਫਰ.ਖਾਲੀ_ਥਾਵਾਂ()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(ਬਫਰ));
    StringRef Folded = ਮੰਜ਼ਿਲ_UTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  ਵਾਪਸ H;
}
