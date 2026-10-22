//===-- HexagonAttributes.cpp - Qualcomm Hexagon Attributes ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/HexagonAttributes.h"

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::HexagonAttrs;

ସ୍ଥିର constexpr TagNameItem TagData[] = {
    {ARCH, "Tag_arch"},
    {HVXARCH, "Tag_hvx_arch"},
    {HVXIEEEFP, "Tag_hvx_ieeefp"},
    {HVXQFLOAT, "Tag_hvx_qfloat"},
    {ZREG, "Tag_zreg"},
    {AUDIO, "Tag_audio"},
    {CABAC, "Tag_cabac"},
};

constexpr TagNameMap HexagonAttributeTags{TagData};
ଅଚଳ TagNameMap &llvm::HexagonAttrs::getHexagonAttributeTags() {
  ଫେରନ୍ତୁ HexagonAttributeTags;
}
