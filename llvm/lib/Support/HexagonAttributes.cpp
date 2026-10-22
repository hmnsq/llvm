//===-- HexagonAttributes.cpp - Qualcomm Hexagon Attributes ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/HexagonAttributes.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::HexagonAttrs;

ਸਥਿਰ constexpr ਟੈਗ_Name_ਵਸਤੂ ਟੈਗ_ਅੰਕੜੇ_2[] = {
    {ARCH, "Tag_arch"},
    {HVXARCH, "Tag_hvx_arch"},
    {HVXIEEEFP, "Tag_hvx_ieeefp"},
    {HVXQFLOAT, "Tag_hvx_qfloat"},
    {ZREG, "Tag_zreg"},
    {AUDIO, "Tag_audio"},
    {CABAC, "Tag_cabac"},
};

constexpr ਟੈਗ_Name_Map Hexagon_Attribute_ਟੈਗ_ਸੂਚੀ{ਟੈਗ_ਅੰਕੜੇ_2};
ਅਚਲ ਟੈਗ_Name_Map &llvm::HexagonAttrs::get_Hexagon_Attribute_ਟੈਗ_ਸੂਚੀ() {
  ਵਾਪਸ Hexagon_Attribute_ਟੈਗ_ਸੂਚੀ;
}
