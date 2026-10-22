//===-- RISCVAttributes.cpp - RISCV Attributes ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/RISCVAttributes.h"

ورتوں نطاق llvm;
ورتوں نطاق llvm::RISCVAttrs;

ساكن constexpr ٹیگ_Name_شے ٹیگ_انکڑے[] = {
    {ڈھیر_ALIGN, "Tag_stack_align"},
    {ARCH, "Tag_arch"},
    {UNALIGNED_ACCESS, "Tag_unaligned_access"},
    {PRIV_SPEC, "Tag_priv_spec"},
    {PRIV_SPEC_MINOR, "Tag_priv_spec_minor"},
    {PRIV_SPEC_REVISION, "Tag_priv_spec_revision"},
    {ATOMIC_ABI, "Tag_atomic_abi"},
};

constexpr ٹیگ_Name_Map RISCVAttribute_ٹیگ_سوچی{ٹیگ_انکڑے};
ثابت ٹیگ_Name_Map &llvm::RISCVAttrs::get_RISCVAttribute_ٹیگ_سوچی() {
  أرجع RISCVAttribute_ٹیگ_سوچی;
}
