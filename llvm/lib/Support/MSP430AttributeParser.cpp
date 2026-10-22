//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/MSP430AttributeParser.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"

using नामाकाश llvm;
using नामाकाश llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::TagISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::TagCodeModel, &MSP430AttributeParser::parseCodeModel},
         {MSP430Attrs::TagDataModel, &MSP430AttributeParser::parseDataModel},
         {MSP430Attrs::TagEnumSize, &MSP430AttributeParser::parseEnumSize}}};

Error MSP430AttributeParser::parseISA(AttrType Tag) {
  स्थिर अचर वर्ण *अचर StringVals[] = {"None", "MSP430", "MSP430X"};
  फर्काऊ parseStringAttribute("ISA", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseCodeModel(AttrType Tag) {
  स्थिर अचर वर्ण *अचर StringVals[] = {"None", "Small", "Large"};
  फर्काऊ parseStringAttribute("Code Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseDataModel(AttrType Tag) {
  स्थिर अचर वर्ण *अचर StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  फर्काऊ parseStringAttribute("Data Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseEnumSize(AttrType Tag) {
  स्थिर अचर वर्ण *अचर StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  फर्काऊ parseStringAttribute("Enum Size", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t Tag, तार्किक &Handled) {
  Handled = असत्य;
  लागि (अचर DisplayHandler &Disp : DisplayRoutines) {
    यदि (uint64_t(Disp.Attribute) != Tag)
      जारी;
    यदि (Error E = (एतत्->*Disp.Routine)(static_cast<AttrType>(Tag)))
      फर्काऊ E;
    Handled = सत्य;
    रोक;
  }
  फर्काऊ Error::success();
}
