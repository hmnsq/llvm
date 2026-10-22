//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/MSP430AttributeParser.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::चिह्नकम्_ISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::चिह्नकम्_संहिता_Model, &MSP430AttributeParser::parse_संहिता_Model},
         {MSP430Attrs::चिह्नकम्_दत्त_Model, &MSP430AttributeParser::parse_दत्त_Model},
         {MSP430Attrs::चिह्नकम्_Enum_आकार, &MSP430AttributeParser::parse_Enum_आकार}}};

Error MSP430AttributeParser::parseISA(AttrType चिह्नकम्) {
  स्थिर ध्रुव वर्ण *ध्रुव StringVals[] = {"None", "MSP430", "MSP430X"};
  प्रत्यागम parseStringAttribute("ISA", चिह्नकम्, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_संहिता_Model(AttrType चिह्नकम्) {
  स्थिर ध्रुव वर्ण *ध्रुव StringVals[] = {"None", "Small", "Large"};
  प्रत्यागम parseStringAttribute("Code Model", चिह्नकम्, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_दत्त_Model(AttrType चिह्नकम्) {
  स्थिर ध्रुव वर्ण *ध्रुव StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  प्रत्यागम parseStringAttribute("Data Model", चिह्नकम्, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_Enum_आकार(AttrType चिह्नकम्) {
  स्थिर ध्रुव वर्ण *ध्रुव StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  प्रत्यागम parseStringAttribute("Enum Size", चिह्नकम्, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t चिह्नकम्, तार्किक &Handled) {
  Handled = असत्य;
  प्रति (ध्रुव DisplayHandler &Disp : DisplayRoutines) {
    यदि (uint64_t(Disp.Attribute) != चिह्नकम्)
      अनुवर्त;
    यदि (Error E = (एतत्->*Disp.Routine)(static_cast<AttrType>(चिह्नकम्)))
      प्रत्यागम E;
    Handled = सत्य;
    भङ्ग;
  }
  प्रत्यागम Error::success();
}
