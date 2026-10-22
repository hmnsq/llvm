//===-- CSKYAttributes.cpp - CSKY Attributes ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/CSKYAttributes.h"

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::CSKYAttrs;

स्थिर ध्रुव चिह्नकम्_Name_Item चिह्नकम्_दत्त[] = {
    {CSKY_ARCH_NAME, "Tag_CSKY_ARCH_NAME"},
    {CSKY_CPU_NAME, "Tag_CSKY_CPU_NAME"},
    {CSKY_CPU_NAME, "Tag_CSKY_CPU_NAME"},
    {CSKY_ISA_FLAGS, "Tag_CSKY_ISA_FLAGS"},
    {CSKY_ISA_EXT_FLAGS, "Tag_CSKY_ISA_EXT_FLAGS"},
    {CSKY_DSP_संस्करणम्, "Tag_CSKY_DSP_VERSION"},
    {CSKY_VDSP_संस्करणम्, "Tag_CSKY_VDSP_VERSION"},
    {CSKY_FPU_संस्करणम्, "Tag_CSKY_FPU_VERSION"},
    {CSKY_FPU_ABI, "Tag_CSKY_FPU_ABI"},
    {CSKY_FPU_ROUNDING, "Tag_CSKY_FPU_ROUNDING"},
    {CSKY_FPU_DENORMAL, "Tag_CSKY_FPU_DENORMAL"},
    {CSKY_FPU_EXCEPTION, "Tag_CSKY_FPU_EXCEPTION"},
    {CSKY_FPU_सङ्ख्या_MODULE, "Tag_CSKY_FPU_NUMBER_MODULE"},
    {CSKY_FPU_HARDFP, "Tag_CSKY_FPU_HARDFP"}};

constexpr चिह्नकम्_Name_Map CSKYAttribute_चिह्नकानि{चिह्नकम्_दत्त};
ध्रुव चिह्नकम्_Name_Map &llvm::CSKYAttrs::get_CSKYAttribute_चिह्नकानि() {
  प्रत्यागम CSKYAttribute_चिह्नकानि;
}
