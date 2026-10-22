//===-- CSKYAttributes.cpp - CSKY Attributes ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/CSKYAttributes.h"

کارول نطاق llvm;
کارول نطاق llvm::CSKYAttrs;

ساكن ثابت نښان_Name_توکی نښان_معلومات[] = {
    {CSKY_ARCH_NAME, "Tag_CSKY_ARCH_NAME"},
    {CSKY_CPU_NAME, "Tag_CSKY_CPU_NAME"},
    {CSKY_CPU_NAME, "Tag_CSKY_CPU_NAME"},
    {CSKY_ISA_FLAGS, "Tag_CSKY_ISA_FLAGS"},
    {CSKY_ISA_EXT_FLAGS, "Tag_CSKY_ISA_EXT_FLAGS"},
    {CSKY_DSP_نسخه, "Tag_CSKY_DSP_VERSION"},
    {CSKY_VDSP_نسخه, "Tag_CSKY_VDSP_VERSION"},
    {CSKY_FPU_نسخه, "Tag_CSKY_FPU_VERSION"},
    {CSKY_FPU_ABI, "Tag_CSKY_FPU_ABI"},
    {CSKY_FPU_ROUNDING, "Tag_CSKY_FPU_ROUNDING"},
    {CSKY_FPU_DENORMAL, "Tag_CSKY_FPU_DENORMAL"},
    {CSKY_FPU_EXCEPTION, "Tag_CSKY_FPU_EXCEPTION"},
    {CSKY_FPU_شمېره_MODULE, "Tag_CSKY_FPU_NUMBER_MODULE"},
    {CSKY_FPU_HARDFP, "Tag_CSKY_FPU_HARDFP"}};

constexpr نښان_Name_Map CSKYAttribute_نښانونه{نښان_معلومات};
ثابت نښان_Name_Map &llvm::CSKYAttrs::get_CSKYAttribute_نښانونه() {
  أرجع CSKYAttribute_نښانونه;
}
