//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/MSP430AttributeParser.h"
#ضمن "llvm/ADT/ArrayRef.h"

استعمال نطاق llvm;
استعمال نطاق llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::ٽيگ_ISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::ٽيگ_ڪوڊ_Model, &MSP430AttributeParser::parse_ڪوڊ_Model},
         {MSP430Attrs::ٽيگ_معلومات_Model, &MSP430AttributeParser::parse_معلومات_Model},
         {MSP430Attrs::ٽيگ_Enum_سائيز, &MSP430AttributeParser::parse_Enum_سائيز}}};

Error MSP430AttributeParser::parseISA(AttrType ٽيگ) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "MSP430", "MSP430X"};
  أرجع parseStringAttribute("ISA", ٽيگ, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_ڪوڊ_Model(AttrType ٽيگ) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Large"};
  أرجع parseStringAttribute("Code Model", ٽيگ, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_معلومات_Model(AttrType ٽيگ) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  أرجع parseStringAttribute("Data Model", ٽيگ, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_Enum_سائيز(AttrType ٽيگ) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  أرجع parseStringAttribute("Enum Size", ٽيگ, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t ٽيگ, منطقي &Handled) {
  Handled = خطأ;
  لكل (ثابت DisplayHandler &Disp : DisplayRoutines) {
    إذا (uint64_t(Disp.Attribute) != ٽيگ)
      استمر;
    إذا (Error E = (هذا->*Disp.Routine)(static_cast<AttrType>(ٽيگ)))
      أرجع E;
    Handled = سچ;
    اكسر;
  }
  أرجع Error::success();
}
