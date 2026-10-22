//===-- MSP430Attributes.cpp - MSP430 Attributes --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/MSP430Attributes.h"

استعمال نطاق llvm;
استعمال نطاق llvm::MSP430Attrs;

ساكن constexpr ٽيگ_Name_شي ٽيگ_معلومات_2[] = {{ٽيگ_ISA, "Tag_ISA"},
                                          {ٽيگ_ڪوڊ_Model, "Tag_Code_Model"},
                                          {ٽيگ_معلومات_Model, "Tag_Data_Model"},
                                          {ٽيگ_Enum_سائيز, "Tag_Enum_Size"}};

constexpr ٽيگ_Name_Map MSP430_Attribute_ٽيگ_فهرست{ٽيگ_معلومات_2};
ثابت ٽيگ_Name_Map &llvm::MSP430Attrs::get_MSP430_Attribute_ٽيگ_فهرست() {
  أرجع MSP430_Attribute_ٽيگ_فهرست;
}
