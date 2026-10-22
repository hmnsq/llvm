//===-- HexagonAttributeParser.cpp - Hexagon Attribute Parser -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/HexagonAttributeParser.h"

उपयुज्य नामाकाश llvm;

ध्रुव HexagonAttributeParser::DisplayHandler
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

Error HexagonAttributeParser::handler(uint64_t चिह्नकम्, तार्किक &Handled) {
  Handled = असत्य;
  प्रति (ध्रुव स्वचालित &R : DisplayRoutines) {
    यदि (uint64_t(R.Attribute) == चिह्नकम्) {
      यदि (Error E = (एतत्->*R.Routine)(चिह्नकम्))
        प्रत्यागम E;
      Handled = सत्य;
      भङ्ग;
    }
  }
  प्रत्यागम Error::success();
}
