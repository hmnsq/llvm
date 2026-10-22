//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/MSP430AttributeParser.h"
#ضمن "llvm/ADT/ArrayRef.h"

کارول نطاق llvm;
کارول نطاق llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::نښان_ISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::نښان_کوډ_Model, &MSP430AttributeParser::parse_کوډ_Model},
         {MSP430Attrs::نښان_معلومات_Model, &MSP430AttributeParser::parse_معلومات_Model},
         {MSP430Attrs::نښان_Enum_اندازه, &MSP430AttributeParser::parse_Enum_اندازه}}};

Error MSP430AttributeParser::parseISA(AttrType نښان) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "MSP430", "MSP430X"};
  أرجع parseStringAttribute("ISA", نښان, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_کوډ_Model(AttrType نښان) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Large"};
  أرجع parseStringAttribute("Code Model", نښان, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_معلومات_Model(AttrType نښان) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  أرجع parseStringAttribute("Data Model", نښان, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_Enum_اندازه(AttrType نښان) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  أرجع parseStringAttribute("Enum Size", نښان, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t نښان, منطقي &Handled) {
  Handled = خطأ;
  لكل (ثابت DisplayHandler &Disp : DisplayRoutines) {
    إذا (uint64_t(Disp.Attribute) != نښان)
      استمر;
    إذا (Error E = (هذا->*Disp.Routine)(static_cast<AttrType>(نښان)))
      أرجع E;
    Handled = ریښتیا;
    اكسر;
  }
  أرجع Error::success();
}
