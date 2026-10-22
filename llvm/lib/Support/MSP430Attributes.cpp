//===-- MSP430Attributes.cpp - MSP430 Attributes --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/MSP430Attributes.h"

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::MSP430Attrs;

स्थिर constexpr चिह्नकम्_Name_Item चिह्नकम्_दत्त_2[] = {{चिह्नकम्_ISA, "Tag_ISA"},
                                          {चिह्नकम्_संहिता_Model, "Tag_Code_Model"},
                                          {चिह्नकम्_दत्त_Model, "Tag_Data_Model"},
                                          {चिह्नकम्_Enum_आकार, "Tag_Enum_Size"}};

constexpr चिह्नकम्_Name_Map MSP430_Attribute_चिह्नकानि{चिह्नकम्_दत्त_2};
ध्रुव चिह्नकम्_Name_Map &llvm::MSP430Attrs::get_MSP430_Attribute_चिह्नकानि() {
  प्रत्यागम MSP430_Attribute_चिह्नकानि;
}
