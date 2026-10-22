//===-- RISCVAttributes.cpp - RISCV Attributes ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/RISCVAttributes.h"

استفاده نطاق llvm;
استفاده نطاق llvm::RISCVAttrs;

ساكن constexpr برچسب_Name_قلم برچسب_داده[] = {
    {پشته_ALIGN, "Tag_stack_align"},
    {ARCH, "Tag_arch"},
    {UNALIGNED_ACCESS, "Tag_unaligned_access"},
    {PRIV_SPEC, "Tag_priv_spec"},
    {PRIV_SPEC_MINOR, "Tag_priv_spec_minor"},
    {PRIV_SPEC_REVISION, "Tag_priv_spec_revision"},
    {ATOMIC_ABI, "Tag_atomic_abi"},
};

constexpr برچسب_Name_Map RISCVAttribute_برچسبها{برچسب_داده};
ثابت برچسب_Name_Map &llvm::RISCVAttrs::get_RISCVAttribute_برچسبها() {
  أرجع RISCVAttribute_برچسبها;
}
