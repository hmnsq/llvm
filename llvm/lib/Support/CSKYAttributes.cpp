//===-- CSKYAttributes.cpp - CSKY Attributes ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/CSKYAttributes.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::CSKYAttrs;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಟ್ಯಾಗ್_Name_ಅಂಶ ಟ್ಯಾಗ್_ದತ್ತಾಂಶ[] = {
    {CSKY_ARCH_NAME, "Tag_CSKY_ARCH_NAME"},
    {CSKY_CPU_NAME, "Tag_CSKY_CPU_NAME"},
    {CSKY_CPU_NAME, "Tag_CSKY_CPU_NAME"},
    {CSKY_ISA_FLAGS, "Tag_CSKY_ISA_FLAGS"},
    {CSKY_ISA_EXT_FLAGS, "Tag_CSKY_ISA_EXT_FLAGS"},
    {CSKY_DSP_ಆವೃತ್ತಿ, "Tag_CSKY_DSP_VERSION"},
    {CSKY_VDSP_ಆವೃತ್ತಿ, "Tag_CSKY_VDSP_VERSION"},
    {CSKY_FPU_ಆವೃತ್ತಿ, "Tag_CSKY_FPU_VERSION"},
    {CSKY_FPU_ABI, "Tag_CSKY_FPU_ABI"},
    {CSKY_FPU_ROUNDING, "Tag_CSKY_FPU_ROUNDING"},
    {CSKY_FPU_DENORMAL, "Tag_CSKY_FPU_DENORMAL"},
    {CSKY_FPU_EXCEPTION, "Tag_CSKY_FPU_EXCEPTION"},
    {CSKY_FPU_ಸಂಖ್ಯೆ_MODULE, "Tag_CSKY_FPU_NUMBER_MODULE"},
    {CSKY_FPU_HARDFP, "Tag_CSKY_FPU_HARDFP"}};

constexpr ಟ್ಯಾಗ್_Name_Map CSKYAttribute_ಟ್ಯಾಗುಗಳು{ಟ್ಯಾಗ್_ದತ್ತಾಂಶ};
ಸ್ಥಿರಾಂಕ ಟ್ಯಾಗ್_Name_Map &llvm::CSKYAttrs::get_CSKYAttribute_ಟ್ಯಾಗುಗಳು() {
  ಹಿಂತಿರುಗಿ CSKYAttribute_ಟ್ಯಾಗುಗಳು;
}
