//===-- HexagonAttributes.cpp - Qualcomm Hexagon Attributes ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/HexagonAttributes.h"

using नामाकाश llvm;
using नामाकाश llvm::HexagonAttrs;

स्थिर constexpr TagNameItem TagData[] = {
    {ARCH, "Tag_arch"},
    {HVXARCH, "Tag_hvx_arch"},
    {HVXIEEEFP, "Tag_hvx_ieeefp"},
    {HVXQFLOAT, "Tag_hvx_qfloat"},
    {ZREG, "Tag_zreg"},
    {AUDIO, "Tag_audio"},
    {CABAC, "Tag_cabac"},
};

constexpr TagNameMap HexagonAttributeTags{TagData};
ध्रुव TagNameMap &llvm::HexagonAttrs::getHexagonAttributeTags() {
  पटिनिवत्त HexagonAttributeTags;
}
