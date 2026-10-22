//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/MSP430AttributeParser.h"
#ضمن "llvm/ADT/ArrayRef.h"

استفاده نطاق llvm;
استفاده نطاق llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::برچسب_ISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::برچسب_کد_Model, &MSP430AttributeParser::parse_کد_Model},
         {MSP430Attrs::برچسب_داده_Model, &MSP430AttributeParser::parse_داده_Model},
         {MSP430Attrs::برچسب_Enum_اندازه, &MSP430AttributeParser::parse_Enum_اندازه}}};

Error MSP430AttributeParser::parseISA(AttrType برچسب) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "MSP430", "MSP430X"};
  أرجع parseStringAttribute("ISA", برچسب, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_کد_Model(AttrType برچسب) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Large"};
  أرجع parseStringAttribute("Code Model", برچسب, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_داده_Model(AttrType برچسب) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  أرجع parseStringAttribute("Data Model", برچسب, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_Enum_اندازه(AttrType برچسب) {
  ساكن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  أرجع parseStringAttribute("Enum Size", برچسب, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t برچسب, منطقي &Handled) {
  Handled = خطأ;
  لكل (ثابت DisplayHandler &Disp : DisplayRoutines) {
    إذا (uint64_t(Disp.Attribute) != برچسب)
      استمر;
    إذا (Error E = (هذا->*Disp.Routine)(static_cast<AttrType>(برچسب)))
      أرجع E;
    Handled = درست;
    اكسر;
  }
  أرجع Error::success();
}
