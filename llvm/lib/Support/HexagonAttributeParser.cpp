//===-- HexagonAttributeParser.cpp - Hexagon Attribute Parser -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/HexagonAttributeParser.h"

using ନାମାକାଶ llvm;

ଅଚଳ HexagonAttributeParser::DisplayHandler
    HexagonAttributeParser::DisplayRoutines[] = {
        {
            HexagonAttrs::ARCH,
            &ELFCompactAttrParser::integerAttribute,
        },
        {
            HexagonAttrs::HVXARCH,
            &ELFCompactAttrParser::integerAttribute,
        },
        {
            HexagonAttrs::HVXIEEEFP,
            &ELFCompactAttrParser::integerAttribute,
        },
        {
            HexagonAttrs::HVXQFLOAT,
            &ELFCompactAttrParser::integerAttribute,
        },
        {
            HexagonAttrs::ZREG,
            &ELFCompactAttrParser::integerAttribute,
        },
        {
            HexagonAttrs::AUDIO,
            &ELFCompactAttrParser::integerAttribute,
        },
        {
            HexagonAttrs::CABAC,
            &ELFCompactAttrParser::integerAttribute,
        }};

Error HexagonAttributeParser::handler(uint64_t Tag, ତାର୍କିକ &Handled) {
  Handled = ମିଥ୍ୟା;
  ପାଇଁ (ଅଚଳ auto &R : DisplayRoutines) {
    ଯଦି (uint64_t(R.Attribute) == Tag) {
      ଯଦି (Error E = (ଏହି->*R.Routine)(Tag))
        ଫେରନ୍ତୁ E;
      Handled = ସତ୍ୟ;
      ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ Error::success();
}
