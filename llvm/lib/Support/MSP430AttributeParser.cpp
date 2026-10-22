//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/MSP430AttributeParser.h"
#ضمن "llvm/ADT/ArrayRef.h"

using نطاق llvm;
using نطاق llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::TagISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::TagCodeModel, &MSP430AttributeParser::parseCodeModel},
         {MSP430Attrs::TagDataModel, &MSP430AttributeParser::parseDataModel},
         {MSP430Attrs::TagEnumSize, &MSP430AttributeParser::parseEnumSize}}};

Error MSP430AttributeParser::parseISA(AttrType Tag) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "MSP430", "MSP430X"};
  أرجع parseStringAttribute("ISA", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseCodeModel(AttrType Tag) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Large"};
  أرجع parseStringAttribute("Code Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseDataModel(AttrType Tag) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  أرجع parseStringAttribute("Data Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseEnumSize(AttrType Tag) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  أرجع parseStringAttribute("Enum Size", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t Tag, منطقي &Handled) {
  Handled = خطأ;
  لكل (ثابت DisplayHandler &Disp : DisplayRoutines) {
    𐡄𐡍 (uint64_t(Disp.Attribute) != Tag)
      استمر;
    𐡄𐡍 (Error E = (هذا->*Disp.Routine)(static_cast<AttrType>(Tag)))
      أرجع E;
    Handled = 𐡒𐡔𐡈;
    اكسر;
  }
  أرجع Error::success();
}
