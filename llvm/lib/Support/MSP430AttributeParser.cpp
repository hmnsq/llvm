//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/MSP430AttributeParser.h"
#समावेश "llvm/ADT/ArrayRef.h"

using namespace llvm;
using namespace llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::TagISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::TagCodeModel, &MSP430AttributeParser::parseCodeModel},
         {MSP430Attrs::Tag_तथ्य_Model, &MSP430AttributeParser::parse_तथ्य_Model},
         {MSP430Attrs::Tag_Enum_आकार, &MSP430AttributeParser::parse_Enum_आकार}}};

Error MSP430AttributeParser::parseISA(AttrType Tag) {
  अटल स्थिर अक्षर *स्थिर StringVals[] = {"None", "MSP430", "MSP430X"};
  वापस parseStringAttribute("ISA", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseCodeModel(AttrType Tag) {
  अटल स्थिर अक्षर *स्थिर StringVals[] = {"None", "Small", "Large"};
  वापस parseStringAttribute("Code Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_तथ्य_Model(AttrType Tag) {
  अटल स्थिर अक्षर *स्थिर StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  वापस parseStringAttribute("Data Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_Enum_आकार(AttrType Tag) {
  अटल स्थिर अक्षर *स्थिर StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  वापस parseStringAttribute("Enum Size", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t Tag, bool &Handled) {
  Handled = false;
  क्रम (स्थिर DisplayHandler &Disp : DisplayRoutines) {
    अगर (uint64_t(Disp.Attribute) != Tag)
      जारी;
    अगर (Error E = (यह->*Disp.Routine)(static_cast<AttrType>(Tag)))
      वापस E;
    Handled = true;
    अवरोध;
  }
  वापस Error::success();
}
