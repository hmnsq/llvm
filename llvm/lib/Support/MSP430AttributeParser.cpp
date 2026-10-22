//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MSP430AttributeParser.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ArrayRef.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::ਟੈਗ_ISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::ਟੈਗ_ਕੋਡ_Model, &MSP430AttributeParser::parse_ਕੋਡ_Model},
         {MSP430Attrs::ਟੈਗ_ਅੰਕੜੇ_Model, &MSP430AttributeParser::parse_ਅੰਕੜੇ_Model},
         {MSP430Attrs::ਟੈਗ_Enum_ਮਾਪ, &MSP430AttributeParser::parse_Enum_ਮਾਪ}}};

Error MSP430AttributeParser::parseISA(AttrType ਟੈਗ) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ StringVals[] = {"None", "MSP430", "MSP430X"};
  ਵਾਪਸ parseStringAttribute("ISA", ਟੈਗ, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_ਕੋਡ_Model(AttrType ਟੈਗ) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ StringVals[] = {"None", "Small", "Large"};
  ਵਾਪਸ parseStringAttribute("Code Model", ਟੈਗ, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_ਅੰਕੜੇ_Model(AttrType ਟੈਗ) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  ਵਾਪਸ parseStringAttribute("Data Model", ਟੈਗ, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_Enum_ਮਾਪ(AttrType ਟੈਗ) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  ਵਾਪਸ parseStringAttribute("Enum Size", ਟੈਗ, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t ਟੈਗ, ਤਾਰਕਿਕ &Handled) {
  Handled = ਝੂਠ;
  ਲਈ (ਅਚਲ DisplayHandler &Disp : DisplayRoutines) {
    ਜੇ (uint64_t(Disp.Attribute) != ਟੈਗ)
      ਜਾਰੀ;
    ਜੇ (Error E = (ਇਹ->*Disp.Routine)(static_cast<AttrType>(ਟੈਗ)))
      ਵਾਪਸ E;
    Handled = ਸੱਚ;
    ਰੋਕੋ;
  }
  ਵਾਪਸ Error::success();
}
