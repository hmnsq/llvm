//===-- HexagonAttributes.cpp - Qualcomm Hexagon Attributes ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/HexagonAttributes.h"

using namespace llvm;
using namespace llvm::HexagonAttrs;

अटल constexpr TagNameItem Tag_तथ्य[] = {
    {ARCH, "Tag_arch"},
    {HVXARCH, "Tag_hvx_arch"},
    {HVXIEEEFP, "Tag_hvx_ieeefp"},
    {HVXQFLOAT, "Tag_hvx_qfloat"},
    {ZREG, "Tag_zreg"},
    {AUDIO, "Tag_audio"},
    {CABAC, "Tag_cabac"},
};

constexpr TagNameMap HexagonAttributeTags{Tag_तथ्य};
स्थिर TagNameMap &llvm::HexagonAttrs::getHexagonAttributeTags() {
  वापस HexagonAttributeTags;
}
