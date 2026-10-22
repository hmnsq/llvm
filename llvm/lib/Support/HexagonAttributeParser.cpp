//===-- HexagonAttributeParser.cpp - Hexagon Attribute Parser -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/HexagonAttributeParser.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਅਚਲ HexagonAttributeParser::DisplayHandler
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

Error HexagonAttributeParser::handler(uint64_t ਟੈਗ, ਤਾਰਕਿਕ &Handled) {
  Handled = ਝੂਠ;
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &R : DisplayRoutines) {
    ਜੇ (uint64_t(R.Attribute) == ਟੈਗ) {
      ਜੇ (Error E = (ਇਹ->*R.Routine)(ਟੈਗ))
        ਵਾਪਸ E;
      Handled = ਸੱਚ;
      ਰੋਕੋ;
    }
  }
  ਵਾਪਸ Error::success();
}
