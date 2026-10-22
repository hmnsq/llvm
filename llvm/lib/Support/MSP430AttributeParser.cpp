//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/MSP430AttributeParser.h"
#شامل "llvm/ADT/ArrayRef.h"

using فضاینام llvm;
using فضاینام llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::TagISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::TagCodeModel, &MSP430AttributeParser::parseCodeModel},
         {MSP430Attrs::TagDataModel, &MSP430AttributeParser::parseDataModel},
         {MSP430Attrs::TagEnumSize, &MSP430AttributeParser::parseEnumSize}}};

Error MSP430AttributeParser::parseISA(AttrType Tag) {
  ایستا ثابت نویسه *ثابت StringVals[] = {"None", "MSP430", "MSP430X"};
  بازگشت parseStringAttribute("ISA", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseCodeModel(AttrType Tag) {
  ایستا ثابت نویسه *ثابت StringVals[] = {"None", "Small", "Large"};
  بازگشت parseStringAttribute("Code Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseDataModel(AttrType Tag) {
  ایستا ثابت نویسه *ثابت StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  بازگشت parseStringAttribute("Data Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseEnumSize(AttrType Tag) {
  ایستا ثابت نویسه *ثابت StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  بازگشت parseStringAttribute("Enum Size", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t Tag, منطقی &Handled) {
  Handled = نادرست;
  برای (ثابت DisplayHandler &Disp : DisplayRoutines) {
    اگر (uint64_t(Disp.Attribute) != Tag)
      ادامه;
    اگر (Error E = (این->*Disp.Routine)(static_cast<AttrType>(Tag)))
      بازگشت E;
    Handled = درست;
    بشکن;
  }
  بازگشت Error::success();
}
