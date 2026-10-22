//===-- RISCVAttributes.cpp - RISCV Attributes ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/RISCVAttributes.h"

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::RISCVAttrs;

స్థిర constexpr ట్యాగ్_Name_అంశం ట్యాగ్_దత్తాంశం[] = {
    {దొంతర_ALIGN, "Tag_stack_align"},
    {ARCH, "Tag_arch"},
    {UNALIGNED_ACCESS, "Tag_unaligned_access"},
    {PRIV_SPEC, "Tag_priv_spec"},
    {PRIV_SPEC_MINOR, "Tag_priv_spec_minor"},
    {PRIV_SPEC_REVISION, "Tag_priv_spec_revision"},
    {ATOMIC_ABI, "Tag_atomic_abi"},
};

constexpr ట్యాగ్_Name_Map RISCVAttribute_ట్యాగులు{ట్యాగ్_దత్తాంశం};
స్థిరరాశి ట్యాగ్_Name_Map &llvm::RISCVAttrs::get_RISCVAttribute_ట్యాగులు() {
  తిరిగి RISCVAttribute_ట్యాగులు;
}
