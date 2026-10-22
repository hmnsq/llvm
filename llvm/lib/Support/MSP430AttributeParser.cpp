//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/MSP430AttributeParser.h"
#અન્તર્ભાવ "llvm/ADT/ArrayRef.h"

using નામાકાશ llvm;
using નામાકાશ llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::TagISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::TagCodeModel, &MSP430AttributeParser::parseCodeModel},
         {MSP430Attrs::TagDataModel, &MSP430AttributeParser::parseDataModel},
         {MSP430Attrs::TagEnumSize, &MSP430AttributeParser::parseEnumSize}}};

Error MSP430AttributeParser::parseISA(AttrType Tag) {
  સ્થિર અચલ અક્ષર *અચલ StringVals[] = {"None", "MSP430", "MSP430X"};
  પાછા parseStringAttribute("ISA", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseCodeModel(AttrType Tag) {
  સ્થિર અચલ અક્ષર *અચલ StringVals[] = {"None", "Small", "Large"};
  પાછા parseStringAttribute("Code Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseDataModel(AttrType Tag) {
  સ્થિર અચલ અક્ષર *અચલ StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  પાછા parseStringAttribute("Data Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseEnumSize(AttrType Tag) {
  સ્થિર અચલ અક્ષર *અચલ StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  પાછા parseStringAttribute("Enum Size", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t Tag, તાર્કિક &Handled) {
  Handled = ખોટું;
  માટે (અચલ DisplayHandler &Disp : DisplayRoutines) {
    જો (uint64_t(Disp.Attribute) != Tag)
      ચાલુ;
    જો (Error E = (આ->*Disp.Routine)(static_cast<AttrType>(Tag)))
      પાછા E;
    Handled = સાચું;
    તોડો;
  }
  પાછા Error::success();
}
