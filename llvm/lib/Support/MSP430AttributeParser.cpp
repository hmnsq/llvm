//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/MSP430AttributeParser.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::TagISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::TagCodeModel, &MSP430AttributeParser::parseCodeModel},
         {MSP430Attrs::TagDataModel, &MSP430AttributeParser::parseDataModel},
         {MSP430Attrs::TagEnumSize, &MSP430AttributeParser::parseEnumSize}}};

Error MSP430AttributeParser::parseISA(AttrType Tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ StringVals[] = {"None", "MSP430", "MSP430X"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ISA", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseCodeModel(AttrType Tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ StringVals[] = {"None", "Small", "Large"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("Code Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseDataModel(AttrType Tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("Data Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseEnumSize(AttrType Tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("Enum Size", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t Tag, ತಾರ್ಕಿಕ &Handled) {
  Handled = ಅಸತ್ಯ;
  ಪ್ರತಿ (ಧ್ರುವ DisplayHandler &Disp : DisplayRoutines) {
    ಯದಿ (uint64_t(Disp.Attribute) != Tag)
      ಅನುವರ್ತ;
    ಯದಿ (Error E = (ಏತತ್->*Disp.Routine)(static_cast<AttrType>(Tag)))
      ಪ್ರತ್ಯಾಗಮ E;
    Handled = ಸತ್ಯ;
    ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ Error::success();
}
