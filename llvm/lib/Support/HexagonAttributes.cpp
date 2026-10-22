//===-- HexagonAttributes.cpp - Qualcomm Hexagon Attributes ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/HexagonAttributes.h"

استعمال نامخلا llvm;
استعمال نامخلا llvm::HexagonAttrs;

ساکن constexpr لیبل_Name_شے لیبل_ڈیٹا_2[] = {
    {ARCH, "Tag_arch"},
    {HVXARCH, "Tag_hvx_arch"},
    {HVXIEEEFP, "Tag_hvx_ieeefp"},
    {HVXQFLOAT, "Tag_hvx_qfloat"},
    {ZREG, "Tag_zreg"},
    {AUDIO, "Tag_audio"},
    {CABAC, "Tag_cabac"},
};

constexpr لیبل_Name_Map Hexagon_Attribute_لیبل_فہرست{لیبل_ڈیٹا_2};
ثابت لیبل_Name_Map &llvm::HexagonAttrs::get_Hexagon_Attribute_لیبل_فہرست() {
  واپس Hexagon_Attribute_لیبل_فہرست;
}
