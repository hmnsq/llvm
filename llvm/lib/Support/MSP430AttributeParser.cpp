//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/MSP430AttributeParser.h"
#ضمن "llvm/ADT/ArrayRef.h"

ورتوں نطاق llvm;
ورتوں نطاق llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::ٹیگ_ISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::ٹیگ_کوڈ_Model, &MSP430AttributeParser::parse_کوڈ_Model},
         {MSP430Attrs::ٹیگ_انکڑے_Model, &MSP430AttributeParser::parse_انکڑے_Model},
         {MSP430Attrs::ٹیگ_Enum_ناپ, &MSP430AttributeParser::parse_Enum_ناپ}}};

Error MSP430AttributeParser::parseISA(AttrType ٹیگ) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "MSP430", "MSP430X"};
  أرجع parseStringAttribute("ISA", ٹیگ, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_کوڈ_Model(AttrType ٹیگ) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Large"};
  أرجع parseStringAttribute("Code Model", ٹیگ, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_انکڑے_Model(AttrType ٹیگ) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  أرجع parseStringAttribute("Data Model", ٹیگ, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_Enum_ناپ(AttrType ٹیگ) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  أرجع parseStringAttribute("Enum Size", ٹیگ, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t ٹیگ, منطقي &Handled) {
  Handled = خطأ;
  لكل (ثابت DisplayHandler &Disp : DisplayRoutines) {
    إذا (uint64_t(Disp.Attribute) != ٹیگ)
      استمر;
    إذا (Error E = (هذا->*Disp.Routine)(static_cast<AttrType>(ٹیگ)))
      أرجع E;
    Handled = سچ;
    اكسر;
  }
  أرجع Error::success();
}
