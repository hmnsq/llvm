//===-- HexagonAttributes.cpp - Qualcomm Hexagon Attributes ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/HexagonAttributes.h"

استفاده نطاق llvm;
استفاده نطاق llvm::HexagonAttrs;

ساكن constexpr برچسب_Name_قلم برچسب_داده_2[] = {
    {ARCH, "Tag_arch"},
    {HVXARCH, "Tag_hvx_arch"},
    {HVXIEEEFP, "Tag_hvx_ieeefp"},
    {HVXQFLOAT, "Tag_hvx_qfloat"},
    {ZREG, "Tag_zreg"},
    {AUDIO, "Tag_audio"},
    {CABAC, "Tag_cabac"},
};

constexpr برچسب_Name_Map Hexagon_Attribute_برچسبها{برچسب_داده_2};
ثابت برچسب_Name_Map &llvm::HexagonAttrs::get_Hexagon_Attribute_برچسبها() {
  أرجع Hexagon_Attribute_برچسبها;
}
