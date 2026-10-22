//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/MSP430AttributeParser.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::ಟ್ಯಾಗ್_ISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::ಟ್ಯಾಗ್_ಸಂಕೇತ_Model, &MSP430AttributeParser::parse_ಸಂಕೇತ_Model},
         {MSP430Attrs::ಟ್ಯಾಗ್_ದತ್ತಾಂಶ_Model, &MSP430AttributeParser::parse_ದತ್ತಾಂಶ_Model},
         {MSP430Attrs::ಟ್ಯಾಗ್_Enum_ಗಾತ್ರ, &MSP430AttributeParser::parse_Enum_ಗಾತ್ರ}}};

Error MSP430AttributeParser::parseISA(AttrType ಟ್ಯಾಗ್) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ StringVals[] = {"None", "MSP430", "MSP430X"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ISA", ಟ್ಯಾಗ್, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_ಸಂಕೇತ_Model(AttrType ಟ್ಯಾಗ್) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ StringVals[] = {"None", "Small", "Large"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("Code Model", ಟ್ಯಾಗ್, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_ದತ್ತಾಂಶ_Model(AttrType ಟ್ಯಾಗ್) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("Data Model", ಟ್ಯಾಗ್, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_Enum_ಗಾತ್ರ(AttrType ಟ್ಯಾಗ್) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("Enum Size", ಟ್ಯಾಗ್, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t ಟ್ಯಾಗ್, ತಾರ್ಕಿಕ &Handled) {
  Handled = ಸುಳ್ಳು;
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ DisplayHandler &Disp : DisplayRoutines) {
    ಆದರೆ (uint64_t(Disp.Attribute) != ಟ್ಯಾಗ್)
      ಮುಂದುವರಿ;
    ಆದರೆ (Error E = (ಇದು->*Disp.Routine)(static_cast<AttrType>(ಟ್ಯಾಗ್)))
      ಹಿಂತಿರುಗಿ E;
    Handled = ನಿಜ;
    ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ Error::success();
}
