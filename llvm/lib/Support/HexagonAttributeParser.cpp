//===-- HexagonAttributeParser.cpp - Hexagon Attribute Parser -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/HexagonAttributeParser.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರಾಂಕ HexagonAttributeParser::DisplayHandler
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

Error HexagonAttributeParser::handler(uint64_t ಟ್ಯಾಗ್, ತಾರ್ಕಿಕ &Handled) {
  Handled = ಸುಳ್ಳು;
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &R : DisplayRoutines) {
    ಆದರೆ (uint64_t(R.Attribute) == ಟ್ಯಾಗ್) {
      ಆದರೆ (Error E = (ಇದು->*R.Routine)(ಟ್ಯಾಗ್))
        ಹಿಂತಿರುಗಿ E;
      Handled = ನಿಜ;
      ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ Error::success();
}
