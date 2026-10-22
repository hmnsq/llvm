//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/MSP430AttributeParser.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ArrayRef.h"

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::TagISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::TagCodeModel, &MSP430AttributeParser::parseCodeModel},
         {MSP430Attrs::TagDataModel, &MSP430AttributeParser::parseDataModel},
         {MSP430Attrs::TagEnumSize, &MSP430AttributeParser::parseEnumSize}}};

Error MSP430AttributeParser::parseISA(AttrType Tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ StringVals[] = {"None", "MSP430", "MSP430X"};
  ଫେରନ୍ତୁ parseStringAttribute("ISA", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseCodeModel(AttrType Tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ StringVals[] = {"None", "Small", "Large"};
  ଫେରନ୍ତୁ parseStringAttribute("Code Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseDataModel(AttrType Tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  ଫେରନ୍ତୁ parseStringAttribute("Data Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseEnumSize(AttrType Tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  ଫେରନ୍ତୁ parseStringAttribute("Enum Size", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t Tag, ତାର୍କିକ &Handled) {
  Handled = ମିଥ୍ୟା;
  ପାଇଁ (ଅଚଳ DisplayHandler &Disp : DisplayRoutines) {
    ଯଦି (uint64_t(Disp.Attribute) != Tag)
      ଜାରି;
    ଯଦି (Error E = (ଏହି->*Disp.Routine)(static_cast<AttrType>(Tag)))
      ଫେରନ୍ତୁ E;
    Handled = ସତ୍ୟ;
    ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ Error::success();
}
