//===-- HexagonAttributeParser.cpp - Hexagon Attribute Parser -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/HexagonAttributeParser.h"

using namespace llvm;

const HexagonAttributeParser::DisplayHandler
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

Error HexagonAttributeParser::handler(uint64_t Tag, bool &Handled) {
  Handled = false;
  위해 (const auto &R : DisplayRoutines) {
    만약 (uint64_t(R.Attribute) == Tag) {
      만약 (Error E = (this->*R.Routine)(Tag))
        반환 E;
      Handled = true;
      break;
    }
  }
  반환 Error::success();
}
