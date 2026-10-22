//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/MSP430AttributeParser.h"
#അന്തര്ഭാവ "llvm/ADT/ArrayRef.h"

using നാമാകാശ llvm;
using നാമാകാശ llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::TagISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::TagCodeModel, &MSP430AttributeParser::parseCodeModel},
         {MSP430Attrs::TagDataModel, &MSP430AttributeParser::parseDataModel},
         {MSP430Attrs::TagEnumSize, &MSP430AttributeParser::parseEnumSize}}};

Error MSP430AttributeParser::parseISA(AttrType Tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം StringVals[] = {"None", "MSP430", "MSP430X"};
  തിരികെ parseStringAttribute("ISA", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseCodeModel(AttrType Tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം StringVals[] = {"None", "Small", "Large"};
  തിരികെ parseStringAttribute("Code Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseDataModel(AttrType Tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  തിരികെ parseStringAttribute("Data Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseEnumSize(AttrType Tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  തിരികെ parseStringAttribute("Enum Size", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t Tag, തർക്കം &Handled) {
  Handled = അസത്യം;
  വേണ്ടി (സ്ഥിരം DisplayHandler &Disp : DisplayRoutines) {
    എങ്കിൽ (uint64_t(Disp.Attribute) != Tag)
      തുടരുക;
    എങ്കിൽ (Error E = (ഇത്->*Disp.Routine)(static_cast<AttrType>(Tag)))
      തിരികെ E;
    Handled = സത്യം;
    നിർത്തുക;
  }
  തിരികെ Error::success();
}
