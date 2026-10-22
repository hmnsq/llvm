//===-- MSP430Attributes.cpp - MSP430 Attributes --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/MSP430Attributes.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::MSP430Attrs;

ಸ್ಥಿರ constexpr ಟ್ಯಾಗ್_Name_ಅಂಶ ಟ್ಯಾಗ್_ದತ್ತಾಂಶ_2[] = {{ಟ್ಯಾಗ್_ISA, "Tag_ISA"},
                                          {ಟ್ಯಾಗ್_ಸಂಕೇತ_Model, "Tag_Code_Model"},
                                          {ಟ್ಯಾಗ್_ದತ್ತಾಂಶ_Model, "Tag_Data_Model"},
                                          {ಟ್ಯಾಗ್_Enum_ಗಾತ್ರ, "Tag_Enum_Size"}};

constexpr ಟ್ಯಾಗ್_Name_Map MSP430_Attribute_ಟ್ಯಾಗುಗಳು{ಟ್ಯಾಗ್_ದತ್ತಾಂಶ_2};
ಸ್ಥಿರಾಂಕ ಟ್ಯಾಗ್_Name_Map &llvm::MSP430Attrs::get_MSP430_Attribute_ಟ್ಯಾಗುಗಳು() {
  ಹಿಂತಿರುಗಿ MSP430_Attribute_ಟ್ಯಾಗುಗಳು;
}
