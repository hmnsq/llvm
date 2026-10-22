//===-- HexagonAttributes.cpp - Qualcomm Hexagon Attributes ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/HexagonAttributes.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::HexagonAttrs;

ಸ್ಥಿರ constexpr ಟ್ಯಾಗ್_Name_ಅಂಶ ಟ್ಯಾಗ್_ದತ್ತಾಂಶ_2[] = {
    {ARCH, "Tag_arch"},
    {HVXARCH, "Tag_hvx_arch"},
    {HVXIEEEFP, "Tag_hvx_ieeefp"},
    {HVXQFLOAT, "Tag_hvx_qfloat"},
    {ZREG, "Tag_zreg"},
    {AUDIO, "Tag_audio"},
    {CABAC, "Tag_cabac"},
};

constexpr ಟ್ಯಾಗ್_Name_Map Hexagon_Attribute_ಟ್ಯಾಗುಗಳು{ಟ್ಯಾಗ್_ದತ್ತಾಂಶ_2};
ಸ್ಥಿರಾಂಕ ಟ್ಯಾಗ್_Name_Map &llvm::HexagonAttrs::get_Hexagon_Attribute_ಟ್ಯಾಗುಗಳು() {
  ಹಿಂತಿರುಗಿ Hexagon_Attribute_ಟ್ಯಾಗುಗಳು;
}
