//===-- HexagonAttributeParser.cpp - Hexagon Attribute Parser -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/HexagonAttributeParser.h"

using ನಾಮಾಕಾಶ llvm;

ಧ್ರುವ HexagonAttributeParser::DisplayHandler
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

Error HexagonAttributeParser::handler(uint64_t Tag, ತಾರ್ಕಿಕ &Handled) {
  Handled = ಅಸತ್ಯ;
  ಪ್ರತಿ (ಧ್ರುವ auto &R : DisplayRoutines) {
    ಯದಿ (uint64_t(R.Attribute) == Tag) {
      ಯದಿ (Error E = (ಏತತ್->*R.Routine)(Tag))
        ಪ್ರತ್ಯಾಗಮ E;
      Handled = ಸತ್ಯ;
      ಭಙ್ಗ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ Error::success();
}
