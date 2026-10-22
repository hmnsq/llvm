//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/MSP430AttributeParser.h"
#అన్తర్భావ "llvm/ADT/ArrayRef.h"

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::ట్యాగ్_ISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::ట్యాగ్_సంకేతం_Model, &MSP430AttributeParser::parse_సంకేతం_Model},
         {MSP430Attrs::ట్యాగ్_దత్తాంశం_Model, &MSP430AttributeParser::parse_దత్తాంశం_Model},
         {MSP430Attrs::ట్యాగ్_Enum_కొలత, &MSP430AttributeParser::parse_Enum_కొలత}}};

Error MSP430AttributeParser::parseISA(AttrType ట్యాగ్) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి StringVals[] = {"None", "MSP430", "MSP430X"};
  తిరిగి parseStringAttribute("ISA", ట్యాగ్, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_సంకేతం_Model(AttrType ట్యాగ్) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి StringVals[] = {"None", "Small", "Large"};
  తిరిగి parseStringAttribute("Code Model", ట్యాగ్, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_దత్తాంశం_Model(AttrType ట్యాగ్) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  తిరిగి parseStringAttribute("Data Model", ట్యాగ్, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_Enum_కొలత(AttrType ట్యాగ్) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  తిరిగి parseStringAttribute("Enum Size", ట్యాగ్, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t ట్యాగ్, తార్కిక &Handled) {
  Handled = అబద్ధం;
  కొరకు (స్థిరరాశి DisplayHandler &Disp : DisplayRoutines) {
    ఒకవేళ (uint64_t(Disp.Attribute) != ట్యాగ్)
      కొనసాగు;
    ఒకవేళ (Error E = (ఇది->*Disp.Routine)(static_cast<AttrType>(ట్యాగ్)))
      తిరిగి E;
    Handled = నిజం;
    ఆపు;
  }
  తిరిగి Error::success();
}
