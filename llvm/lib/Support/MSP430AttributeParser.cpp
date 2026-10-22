//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/MSP430AttributeParser.h"
#شامل "llvm/ADT/ArrayRef.h"

استعمال نامخلا llvm;
استعمال نامخلا llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::لیبل_ISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::لیبل_رمز_Model, &MSP430AttributeParser::parse_رمز_Model},
         {MSP430Attrs::لیبل_ڈیٹا_Model, &MSP430AttributeParser::parse_ڈیٹا_Model},
         {MSP430Attrs::لیبل_Enum_سائز, &MSP430AttributeParser::parse_Enum_سائز}}};

Error MSP430AttributeParser::parseISA(AttrType لیبل) {
  ساکن ثابت حرف *ثابت StringVals[] = {"None", "MSP430", "MSP430X"};
  واپس parseStringAttribute("ISA", لیبل, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_رمز_Model(AttrType لیبل) {
  ساکن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Large"};
  واپس parseStringAttribute("Code Model", لیبل, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_ڈیٹا_Model(AttrType لیبل) {
  ساکن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  واپس parseStringAttribute("Data Model", لیبل, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_Enum_سائز(AttrType لیبل) {
  ساکن ثابت حرف *ثابت StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  واپس parseStringAttribute("Enum Size", لیبل, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t لیبل, منطقی &Handled) {
  Handled = جھوٹ;
  برائے (ثابت DisplayHandler &Disp : DisplayRoutines) {
    اگر (uint64_t(Disp.Attribute) != لیبل)
      جاری;
    اگر (Error E = (یہ->*Disp.Routine)(static_cast<AttrType>(لیبل)))
      واپس E;
    Handled = سچ;
    توڑو;
  }
  واپس Error::success();
}
