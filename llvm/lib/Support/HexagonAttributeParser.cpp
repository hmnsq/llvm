//===-- HexagonAttributeParser.cpp - Hexagon Attribute Parser -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/HexagonAttributeParser.h"

ఉపయోగించి నామాకాశ llvm;

స్థిరరాశి HexagonAttributeParser::DisplayHandler
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

Error HexagonAttributeParser::handler(uint64_t ట్యాగ్, తార్కిక &Handled) {
  Handled = అబద్ధం;
  కొరకు (స్థిరరాశి స్వయంచాలక &R : DisplayRoutines) {
    ఒకవేళ (uint64_t(R.Attribute) == ట్యాగ్) {
      ఒకవేళ (Error E = (ఇది->*R.Routine)(ట్యాగ్))
        తిరిగి E;
      Handled = నిజం;
      ఆపు;
    }
  }
  తిరిగి Error::success();
}
