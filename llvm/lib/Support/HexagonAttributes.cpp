//===-- HexagonAttributes.cpp - Qualcomm Hexagon Attributes ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/HexagonAttributes.h"

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::HexagonAttrs;

స్థిర constexpr ట్యాగ్_Name_అంశం ట్యాగ్_దత్తాంశం_2[] = {
    {ARCH, "Tag_arch"},
    {HVXARCH, "Tag_hvx_arch"},
    {HVXIEEEFP, "Tag_hvx_ieeefp"},
    {HVXQFLOAT, "Tag_hvx_qfloat"},
    {ZREG, "Tag_zreg"},
    {AUDIO, "Tag_audio"},
    {CABAC, "Tag_cabac"},
};

constexpr ట్యాగ్_Name_Map Hexagon_Attribute_ట్యాగులు{ట్యాగ్_దత్తాంశం_2};
స్థిరరాశి ట్యాగ్_Name_Map &llvm::HexagonAttrs::get_Hexagon_Attribute_ట్యాగులు() {
  తిరిగి Hexagon_Attribute_ట్యాగులు;
}
