//===-- MSP430Attributes.cpp - MSP430 Attributes --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/MSP430Attributes.h"

استفاده نطاق llvm;
استفاده نطاق llvm::MSP430Attrs;

ساكن constexpr برچسب_Name_قلم برچسب_داده_2[] = {{برچسب_ISA, "Tag_ISA"},
                                          {برچسب_کد_Model, "Tag_Code_Model"},
                                          {برچسب_داده_Model, "Tag_Data_Model"},
                                          {برچسب_Enum_اندازه, "Tag_Enum_Size"}};

constexpr برچسب_Name_Map MSP430_Attribute_برچسبها{برچسب_داده_2};
ثابت برچسب_Name_Map &llvm::MSP430Attrs::get_MSP430_Attribute_برچسبها() {
  أرجع MSP430_Attribute_برچسبها;
}
