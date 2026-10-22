//===-- HexagonAttributes.cpp - Qualcomm Hexagon Attributes ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/HexagonAttributes.h"

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::HexagonAttrs;

स्थिर constexpr चिह्नकम्_Name_Item चिह्नकम्_दत्त_2[] = {
    {ARCH, "Tag_arch"},
    {HVXARCH, "Tag_hvx_arch"},
    {HVXIEEEFP, "Tag_hvx_ieeefp"},
    {HVXQFLOAT, "Tag_hvx_qfloat"},
    {ZREG, "Tag_zreg"},
    {AUDIO, "Tag_audio"},
    {CABAC, "Tag_cabac"},
};

constexpr चिह्नकम्_Name_Map Hexagon_Attribute_चिह्नकानि{चिह्नकम्_दत्त_2};
ध्रुव चिह्नकम्_Name_Map &llvm::HexagonAttrs::get_Hexagon_Attribute_चिह्नकानि() {
  प्रत्यागम Hexagon_Attribute_चिह्नकानि;
}
