//===-- MSP430Attributes.cpp - MSP430 Attributes --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/MSP430Attributes.h"

using নামাকাশ llvm;
using নামাকাশ llvm::MSP430Attrs;

স্থির constexpr TagNameItem Tag_তথ্য[] = {{TagISA, "Tag_ISA"},
                                          {TagCodeModel, "Tag_Code_Model"},
                                          {Tag_তথ্য_Model, "Tag_Data_Model"},
                                          {Tag_Enum_আকার, "Tag_Enum_Size"}};

constexpr TagNameMap MSP430AttributeTags{Tag_তথ্য};
ধ্রুবক TagNameMap &llvm::MSP430Attrs::getMSP430AttributeTags() {
  ফেরত MSP430AttributeTags;
}
