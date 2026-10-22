//===-- RISCVAttributes.cpp - RISCV Attributes ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/RISCVAttributes.h"

استعمال نطاق llvm;
استعمال نطاق llvm::RISCVAttrs;

ساكن constexpr ٽيگ_Name_شي ٽيگ_معلومات[] = {
    {ڍير_ALIGN, "Tag_stack_align"},
    {ARCH, "Tag_arch"},
    {UNALIGNED_ACCESS, "Tag_unaligned_access"},
    {PRIV_SPEC, "Tag_priv_spec"},
    {PRIV_SPEC_MINOR, "Tag_priv_spec_minor"},
    {PRIV_SPEC_REVISION, "Tag_priv_spec_revision"},
    {ATOMIC_ABI, "Tag_atomic_abi"},
};

constexpr ٽيگ_Name_Map RISCVAttribute_ٽيگ_فهرست{ٽيگ_معلومات};
ثابت ٽيگ_Name_Map &llvm::RISCVAttrs::get_RISCVAttribute_ٽيگ_فهرست() {
  أرجع RISCVAttribute_ٽيگ_فهرست;
}
