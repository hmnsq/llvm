//===-- RISCVAttributes.cpp - RISCV Attributes ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/RISCVAttributes.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::RISCVAttrs;

ਸਥਿਰ constexpr ਟੈਗ_Name_ਵਸਤੂ ਟੈਗ_ਅੰਕੜੇ[] = {
    {ਢੇਰ_ALIGN, "Tag_stack_align"},
    {ARCH, "Tag_arch"},
    {UNALIGNED_ACCESS, "Tag_unaligned_access"},
    {PRIV_SPEC, "Tag_priv_spec"},
    {PRIV_SPEC_MINOR, "Tag_priv_spec_minor"},
    {PRIV_SPEC_REVISION, "Tag_priv_spec_revision"},
    {ATOMIC_ABI, "Tag_atomic_abi"},
};

constexpr ਟੈਗ_Name_Map RISCVAttribute_ਟੈਗ_ਸੂਚੀ{ਟੈਗ_ਅੰਕੜੇ};
ਅਚਲ ਟੈਗ_Name_Map &llvm::RISCVAttrs::get_RISCVAttribute_ਟੈਗ_ਸੂਚੀ() {
  ਵਾਪਸ RISCVAttribute_ਟੈਗ_ਸੂਚੀ;
}
