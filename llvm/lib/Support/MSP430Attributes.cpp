//===-- MSP430Attributes.cpp - MSP430 Attributes --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/MSP430Attributes.h"

کارول نطاق llvm;
کارول نطاق llvm::MSP430Attrs;

ساكن constexpr نښان_Name_توکی نښان_معلومات_2[] = {{نښان_ISA, "Tag_ISA"},
                                          {نښان_کوډ_Model, "Tag_Code_Model"},
                                          {نښان_معلومات_Model, "Tag_Data_Model"},
                                          {نښان_Enum_اندازه, "Tag_Enum_Size"}};

constexpr نښان_Name_Map MSP430_Attribute_نښانونه{نښان_معلومات_2};
ثابت نښان_Name_Map &llvm::MSP430Attrs::get_MSP430_Attribute_نښانونه() {
  أرجع MSP430_Attribute_نښانونه;
}
