//===-- RISCVAttributes.cpp - RISCV Attributes ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/RISCVAttributes.h"

کارول نطاق llvm;
کارول نطاق llvm::RISCVAttrs;

ساكن constexpr نښان_Name_توکی نښان_معلومات[] = {
    {ډېری_ALIGN, "Tag_stack_align"},
    {ARCH, "Tag_arch"},
    {UNALIGNED_ACCESS, "Tag_unaligned_access"},
    {PRIV_SPEC, "Tag_priv_spec"},
    {PRIV_SPEC_MINOR, "Tag_priv_spec_minor"},
    {PRIV_SPEC_REVISION, "Tag_priv_spec_revision"},
    {ATOMIC_ABI, "Tag_atomic_abi"},
};

constexpr نښان_Name_Map RISCVAttribute_نښانونه{نښان_معلومات};
ثابت نښان_Name_Map &llvm::RISCVAttrs::get_RISCVAttribute_نښانونه() {
  أرجع RISCVAttribute_نښانونه;
}
