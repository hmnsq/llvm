//===-- HexagonAttributes.cpp - Qualcomm Hexagon Attributes ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/HexagonAttributes.h"

کارول نطاق llvm;
کارول نطاق llvm::HexagonAttrs;

ساكن constexpr نښان_Name_توکی نښان_معلومات_2[] = {
    {ARCH, "Tag_arch"},
    {HVXARCH, "Tag_hvx_arch"},
    {HVXIEEEFP, "Tag_hvx_ieeefp"},
    {HVXQFLOAT, "Tag_hvx_qfloat"},
    {ZREG, "Tag_zreg"},
    {AUDIO, "Tag_audio"},
    {CABAC, "Tag_cabac"},
};

constexpr نښان_Name_Map Hexagon_Attribute_نښانونه{نښان_معلومات_2};
ثابت نښان_Name_Map &llvm::HexagonAttrs::get_Hexagon_Attribute_نښانونه() {
  أرجع Hexagon_Attribute_نښانونه;
}
