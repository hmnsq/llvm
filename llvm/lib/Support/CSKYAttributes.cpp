//===-- CSKYAttributes.cpp - CSKY Attributes ------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/CSKYAttributes.h"

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::CSKYAttrs;

స్థిర స్థిరరాశి ట్యాగ్_Name_అంశం ట్యాగ్_దత్తాంశం[] = {
    {CSKY_ARCH_NAME, "Tag_CSKY_ARCH_NAME"},
    {CSKY_CPU_NAME, "Tag_CSKY_CPU_NAME"},
    {CSKY_CPU_NAME, "Tag_CSKY_CPU_NAME"},
    {CSKY_ISA_FLAGS, "Tag_CSKY_ISA_FLAGS"},
    {CSKY_ISA_EXT_FLAGS, "Tag_CSKY_ISA_EXT_FLAGS"},
    {CSKY_DSP_సంస్కరణ, "Tag_CSKY_DSP_VERSION"},
    {CSKY_VDSP_సంస్కరణ, "Tag_CSKY_VDSP_VERSION"},
    {CSKY_FPU_సంస్కరణ, "Tag_CSKY_FPU_VERSION"},
    {CSKY_FPU_ABI, "Tag_CSKY_FPU_ABI"},
    {CSKY_FPU_ROUNDING, "Tag_CSKY_FPU_ROUNDING"},
    {CSKY_FPU_DENORMAL, "Tag_CSKY_FPU_DENORMAL"},
    {CSKY_FPU_EXCEPTION, "Tag_CSKY_FPU_EXCEPTION"},
    {CSKY_FPU_సంఖ్య_MODULE, "Tag_CSKY_FPU_NUMBER_MODULE"},
    {CSKY_FPU_HARDFP, "Tag_CSKY_FPU_HARDFP"}};

constexpr ట్యాగ్_Name_Map CSKYAttribute_ట్యాగులు{ట్యాగ్_దత్తాంశం};
స్థిరరాశి ట్యాగ్_Name_Map &llvm::CSKYAttrs::get_CSKYAttribute_ట్యాగులు() {
  తిరిగి CSKYAttribute_ట్యాగులు;
}
