//===-- MSP430Attributes.cpp - MSP430 Attributes --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/MSP430Attributes.h"

using नामाकाश llvm;
using नामाकाश llvm::MSP430Attrs;

स्थिर constexpr TagNameItem TagData[] = {{TagISA, "Tag_ISA"},
                                          {TagCodeModel, "Tag_Code_Model"},
                                          {TagDataModel, "Tag_Data_Model"},
                                          {TagEnumSize, "Tag_Enum_Size"}};

constexpr TagNameMap MSP430AttributeTags{TagData};
अचर TagNameMap &llvm::MSP430Attrs::getMSP430AttributeTags() {
  फर्काऊ MSP430AttributeTags;
}
