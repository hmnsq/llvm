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

#అన్తర్భావ "llvm/Support/DJB.h"
#అన్తర్భావ "llvm/ADT/ArrayRef.h"
#అన్తర్భావ "llvm/Support/Compiler.h"
#అన్తర్భావ "llvm/Support/ConvertUTF.h"
#అన్తర్భావ "llvm/Support/Unicode.h"

ఉపయోగించి నామాకాశ llvm;

స్థిర UTF32 chopOneUTF32(StringRef &బఫరు) {
  UTF32 C;
  స్థిరరాశి UTF8 *స్థిరరాశి Begin8Const =
      reinterpret_cast<స్థిరరాశి UTF8 *>(బఫరు.begin());
  స్థిరరాశి UTF8 *Begin8 = Begin8Const;
  UTF32 *Begin32 = &C;

  // In lenient mode we will always end up with a "reasonable" value in C for
  // non-empty input.
  ప్రతిజ్ఞా(!బఫరు.ఖాళీలు());
  ConvertUTF8toUTF32(&Begin8, reinterpret_cast<స్థిరరాశి UTF8 *>(బఫరు.చివర()),
                     &Begin32, &C + 1, lenientConversion);
  బఫరు = బఫరు.drop_front(Begin8 - Begin8Const);
  తిరిగి C;
}

స్థిర StringRef గమ్యం_UTF8(UTF32 C, MutableArrayRef<UTF8> Storage) {
  స్థిరరాశి UTF32 *Begin32 = &C;
  UTF8 *Begin8 = Storage.begin();

  // The case-folded output should always be a valid unicode character, so use
  // strict mode here.
  Conversion_ఫలితం CR = ConvertUTF32toUTF8(&Begin32, &C + 1, &Begin8,
                                           Storage.చివర(), strictConversion);
  ప్రతిజ్ఞా(CR == conversionOK && "Case folding produced invalid char?");
  (శూన్య)CR;
  తిరిగి StringRef(reinterpret_cast<అక్షరం *>(Storage.begin()),
                   Begin8 - Storage.begin());
}

స్థిర UTF32 foldCharDwarf(UTF32 C) {
  // DWARF v5 addition to the unicode folding rules.
  // Fold "Latin Small Letter Dotless I" and "Latin Capital Letter I With Dot
  // Above" into "i".
  ఒకవేళ (C == 0x130 || C == 0x131)
    తిరిగి 'i';
  తిరిగి sys::unicode::foldCharSimple(C);
}

స్థిర std::optional<uint32_t> fastCaseFoldingDjbHash(StringRef బఫరు,
                                                      uint32_t H) {
  తార్కిక AllASCII = నిజం;
  కొరకు (గుర్తులేని అక్షరం C : బఫరు) {
    H = H * 33 + ('A' <= C && C <= 'Z' ? C - 'A' + 'a' : C);
    AllASCII &= C <= 0x7f;
  }
  ఒకవేళ (AllASCII)
    తిరిగి H;
  తిరిగి std::nullopt;
}

uint32_t llvm::caseFoldingDjbHash(StringRef బఫరు, uint32_t H) {
  ఒకవేళ (std::optional<uint32_t> ఫలితం = fastCaseFoldingDjbHash(బఫరు, H))
    తిరిగి *ఫలితం;

  std::array<UTF8, UNI_MAX_UTF8_బైట్లు_చొప్పున_సంకేతం_బిందువు> Storage;
  అయితే (!బఫరు.ఖాళీలు()) {
    UTF32 C = foldCharDwarf(chopOneUTF32(బఫరు));
    StringRef Folded = గమ్యం_UTF8(C, Storage);
    H = djbHash(Folded, H);
  }
  తిరిగి H;
}
