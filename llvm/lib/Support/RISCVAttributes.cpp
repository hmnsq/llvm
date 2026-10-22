//===-- RISCVAttributes.cpp - RISCV Attributes ----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/RISCVAttributes.h"

using નામાકાશ llvm;
using નામાકાશ llvm::RISCVAttrs;

સ્થિર constexpr TagNameItem tagData[] = {
    {ઢગલો_ALIGN, "Tag_stack_align"},
    {ARCH, "Tag_arch"},
    {UNALIGNED_ACCESS, "Tag_unaligned_access"},
    {PRIV_SPEC, "Tag_priv_spec"},
    {PRIV_SPEC_MINOR, "Tag_priv_spec_minor"},
    {PRIV_SPEC_REVISION, "Tag_priv_spec_revision"},
    {ATOMIC_ABI, "Tag_atomic_abi"},
};

constexpr TagNameMap RISCVAttributeTags{tagData};
અચલ TagNameMap &llvm::RISCVAttrs::getRISCVAttributeTags() {
  પાછા RISCVAttributeTags;
}
