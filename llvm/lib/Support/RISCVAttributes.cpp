//===-- RISCVAttributes.cpp - RISCV Attributes ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/RISCVAttributes.h"

استعمال نامخلا llvm;
استعمال نامخلا llvm::RISCVAttrs;

ساکن constexpr لیبل_Name_شے لیبل_ڈیٹا[] = {
    {ڈھیر_ALIGN, "Tag_stack_align"},
    {ARCH, "Tag_arch"},
    {UNALIGNED_ACCESS, "Tag_unaligned_access"},
    {PRIV_SPEC, "Tag_priv_spec"},
    {PRIV_SPEC_MINOR, "Tag_priv_spec_minor"},
    {PRIV_SPEC_REVISION, "Tag_priv_spec_revision"},
    {ATOMIC_ABI, "Tag_atomic_abi"},
};

constexpr لیبل_Name_Map RISCVAttribute_لیبل_فہرست{لیبل_ڈیٹا};
ثابت لیبل_Name_Map &llvm::RISCVAttrs::get_RISCVAttribute_لیبل_فہرست() {
  واپس RISCVAttribute_لیبل_فہرست;
}
