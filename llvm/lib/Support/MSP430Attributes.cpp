//===-- MSP430Attributes.cpp - MSP430 Attributes --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MSP430Attributes.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::MSP430Attrs;

ਸਥਿਰ constexpr ਟੈਗ_Name_ਵਸਤੂ ਟੈਗ_ਅੰਕੜੇ_2[] = {{ਟੈਗ_ISA, "Tag_ISA"},
                                          {ਟੈਗ_ਕੋਡ_Model, "Tag_Code_Model"},
                                          {ਟੈਗ_ਅੰਕੜੇ_Model, "Tag_Data_Model"},
                                          {ਟੈਗ_Enum_ਮਾਪ, "Tag_Enum_Size"}};

constexpr ਟੈਗ_Name_Map MSP430_Attribute_ਟੈਗ_ਸੂਚੀ{ਟੈਗ_ਅੰਕੜੇ_2};
ਅਚਲ ਟੈਗ_Name_Map &llvm::MSP430Attrs::get_MSP430_Attribute_ਟੈਗ_ਸੂਚੀ() {
  ਵਾਪਸ MSP430_Attribute_ਟੈਗ_ਸੂਚੀ;
}
