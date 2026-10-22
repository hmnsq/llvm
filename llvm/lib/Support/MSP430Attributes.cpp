//===-- MSP430Attributes.cpp - MSP430 Attributes --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/MSP430Attributes.h"

ورتوں نطاق llvm;
ورتوں نطاق llvm::MSP430Attrs;

ساكن constexpr ٹیگ_Name_شے ٹیگ_انکڑے_2[] = {{ٹیگ_ISA, "Tag_ISA"},
                                          {ٹیگ_کوڈ_Model, "Tag_Code_Model"},
                                          {ٹیگ_انکڑے_Model, "Tag_Data_Model"},
                                          {ٹیگ_Enum_ناپ, "Tag_Enum_Size"}};

constexpr ٹیگ_Name_Map MSP430_Attribute_ٹیگ_سوچی{ٹیگ_انکڑے_2};
ثابت ٹیگ_Name_Map &llvm::MSP430Attrs::get_MSP430_Attribute_ٹیگ_سوچی() {
  أرجع MSP430_Attribute_ٹیگ_سوچی;
}
