//===-- RISCVAttributes.cpp - RISCV Attributes ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/RISCVAttributes.h"

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::RISCVAttrs;

स्थिर constexpr चिह्नकम्_Name_Item चिह्नकम्_दत्त[] = {
    {राशिः_ALIGN, "Tag_stack_align"},
    {ARCH, "Tag_arch"},
    {UNALIGNED_ACCESS, "Tag_unaligned_access"},
    {PRIV_SPEC, "Tag_priv_spec"},
    {PRIV_SPEC_MINOR, "Tag_priv_spec_minor"},
    {PRIV_SPEC_REVISION, "Tag_priv_spec_revision"},
    {ATOMIC_ABI, "Tag_atomic_abi"},
};

constexpr चिह्नकम्_Name_Map RISCVAttribute_चिह्नकानि{चिह्नकम्_दत्त};
ध्रुव चिह्नकम्_Name_Map &llvm::RISCVAttrs::get_RISCVAttribute_चिह्नकानि() {
  प्रत्यागम RISCVAttribute_चिह्नकानि;
}
