//===-- CSKYAttributes.cpp - CSKY Attributes ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/CSKYAttributes.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::CSKYAttrs;

ਸਥਿਰ ਅਚਲ ਟੈਗ_Name_ਵਸਤੂ ਟੈਗ_ਅੰਕੜੇ[] = {
    {CSKY_ARCH_NAME, "Tag_CSKY_ARCH_NAME"},
    {CSKY_CPU_NAME, "Tag_CSKY_CPU_NAME"},
    {CSKY_CPU_NAME, "Tag_CSKY_CPU_NAME"},
    {CSKY_ISA_FLAGS, "Tag_CSKY_ISA_FLAGS"},
    {CSKY_ISA_EXT_FLAGS, "Tag_CSKY_ISA_EXT_FLAGS"},
    {CSKY_DSP_ਸੰਸਕਰਣ, "Tag_CSKY_DSP_VERSION"},
    {CSKY_VDSP_ਸੰਸਕਰਣ, "Tag_CSKY_VDSP_VERSION"},
    {CSKY_FPU_ਸੰਸਕਰਣ, "Tag_CSKY_FPU_VERSION"},
    {CSKY_FPU_ABI, "Tag_CSKY_FPU_ABI"},
    {CSKY_FPU_ROUNDING, "Tag_CSKY_FPU_ROUNDING"},
    {CSKY_FPU_DENORMAL, "Tag_CSKY_FPU_DENORMAL"},
    {CSKY_FPU_EXCEPTION, "Tag_CSKY_FPU_EXCEPTION"},
    {CSKY_FPU_ਸੰਖਿਆ_MODULE, "Tag_CSKY_FPU_NUMBER_MODULE"},
    {CSKY_FPU_HARDFP, "Tag_CSKY_FPU_HARDFP"}};

constexpr ਟੈਗ_Name_Map CSKYAttribute_ਟੈਗ_ਸੂਚੀ{ਟੈਗ_ਅੰਕੜੇ};
ਅਚਲ ਟੈਗ_Name_Map &llvm::CSKYAttrs::get_CSKYAttribute_ਟੈਗ_ਸੂਚੀ() {
  ਵਾਪਸ CSKYAttribute_ਟੈਗ_ਸੂਚੀ;
}
