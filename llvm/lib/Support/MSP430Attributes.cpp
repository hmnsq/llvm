//===-- MSP430Attributes.cpp - MSP430 Attributes --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/MSP430Attributes.h"

using namespace llvm;
using namespace llvm::MSP430Attrs;

अटल constexpr TagNameItem Tag_तथ्य[] = {{TagISA, "Tag_ISA"},
                                          {TagCodeModel, "Tag_Code_Model"},
                                          {Tag_तथ्य_Model, "Tag_Data_Model"},
                                          {Tag_Enum_आकार, "Tag_Enum_Size"}};

constexpr TagNameMap MSP430AttributeTags{Tag_तथ्य};
स्थिर TagNameMap &llvm::MSP430Attrs::getMSP430AttributeTags() {
  वापस MSP430AttributeTags;
}
