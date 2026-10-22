//===-- RISCVAttributes.cpp - RISCV Attributes ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/RISCVAttributes.h"

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::RISCVAttrs;

ସ୍ଥିର constexpr TagNameItem tagData[] = {
    {ଗଦା_ALIGN, "Tag_stack_align"},
    {ARCH, "Tag_arch"},
    {UNALIGNED_ACCESS, "Tag_unaligned_access"},
    {PRIV_SPEC, "Tag_priv_spec"},
    {PRIV_SPEC_MINOR, "Tag_priv_spec_minor"},
    {PRIV_SPEC_REVISION, "Tag_priv_spec_revision"},
    {ATOMIC_ABI, "Tag_atomic_abi"},
};

constexpr TagNameMap RISCVAttributeTags{tagData};
ଅଚଳ TagNameMap &llvm::RISCVAttrs::getRISCVAttributeTags() {
  ଫେରନ୍ତୁ RISCVAttributeTags;
}
