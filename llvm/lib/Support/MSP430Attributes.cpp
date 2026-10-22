//===-- MSP430Attributes.cpp - MSP430 Attributes --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/MSP430Attributes.h"

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::MSP430Attrs;

స్థిర constexpr ట్యాగ్_Name_అంశం ట్యాగ్_దత్తాంశం_2[] = {{ట్యాగ్_ISA, "Tag_ISA"},
                                          {ట్యాగ్_సంకేతం_Model, "Tag_Code_Model"},
                                          {ట్యాగ్_దత్తాంశం_Model, "Tag_Data_Model"},
                                          {ట్యాగ్_Enum_కొలత, "Tag_Enum_Size"}};

constexpr ట్యాగ్_Name_Map MSP430_Attribute_ట్యాగులు{ట్యాగ్_దత్తాంశం_2};
స్థిరరాశి ట్యాగ్_Name_Map &llvm::MSP430Attrs::get_MSP430_Attribute_ట్యాగులు() {
  తిరిగి MSP430_Attribute_ట్యాగులు;
}
