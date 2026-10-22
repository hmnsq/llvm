//===-- RISCVAttributes.cpp - RISCV Attributes ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/RISCVAttributes.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::RISCVAttrs;

ಸ್ಥಿರ constexpr ಟ್ಯಾಗ್_Name_ಅಂಶ ಟ್ಯಾಗ್_ದತ್ತಾಂಶ[] = {
    {ರಾಶಿ_ALIGN, "Tag_stack_align"},
    {ARCH, "Tag_arch"},
    {UNALIGNED_ACCESS, "Tag_unaligned_access"},
    {PRIV_SPEC, "Tag_priv_spec"},
    {PRIV_SPEC_MINOR, "Tag_priv_spec_minor"},
    {PRIV_SPEC_REVISION, "Tag_priv_spec_revision"},
    {ATOMIC_ABI, "Tag_atomic_abi"},
};

constexpr ಟ್ಯಾಗ್_Name_Map RISCVAttribute_ಟ್ಯಾಗುಗಳು{ಟ್ಯಾಗ್_ದತ್ತಾಂಶ};
ಸ್ಥಿರಾಂಕ ಟ್ಯಾಗ್_Name_Map &llvm::RISCVAttrs::get_RISCVAttribute_ಟ್ಯಾಗುಗಳು() {
  ಹಿಂತಿರುಗಿ RISCVAttribute_ಟ್ಯಾಗುಗಳು;
}
