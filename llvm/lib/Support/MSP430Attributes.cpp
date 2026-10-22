//===-- MSP430Attributes.cpp - MSP430 Attributes --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/MSP430Attributes.h"

استعمال نامخلا llvm;
استعمال نامخلا llvm::MSP430Attrs;

ساکن constexpr لیبل_Name_شے لیبل_ڈیٹا_2[] = {{لیبل_ISA, "Tag_ISA"},
                                          {لیبل_رمز_Model, "Tag_Code_Model"},
                                          {لیبل_ڈیٹا_Model, "Tag_Data_Model"},
                                          {لیبل_Enum_سائز, "Tag_Enum_Size"}};

constexpr لیبل_Name_Map MSP430_Attribute_لیبل_فہرست{لیبل_ڈیٹا_2};
ثابت لیبل_Name_Map &llvm::MSP430Attrs::get_MSP430_Attribute_لیبل_فہرست() {
  واپس MSP430_Attribute_لیبل_فہرست;
}
