//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/MSP430AttributeParser.h"
#include "llvm/ADT/ArrayRef.h"

using namespace llvm;
using namespace llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::TagISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::TagCodeModel, &MSP430AttributeParser::parseCodeModel},
         {MSP430Attrs::TagDataModel, &MSP430AttributeParser::parseDataModel},
         {MSP430Attrs::TagEnumSize, &MSP430AttributeParser::parseEnumSize}}};

Error MSP430AttributeParser::parseISA(AttrType Tag) {
  static const var_na *const StringVals[] = {"None", "MSP430", "MSP430X"};
  ສົ່ງຄືນ parseStringAttribute("ISA", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseCodeModel(AttrType Tag) {
  static const var_na *const StringVals[] = {"None", "Small", "Large"};
  ສົ່ງຄືນ parseStringAttribute("Code Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseDataModel(AttrType Tag) {
  static const var_na *const StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  ສົ່ງຄືນ parseStringAttribute("Data Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseEnumSize(AttrType Tag) {
  static const var_na *const StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  ສົ່ງຄືນ parseStringAttribute("Enum Size", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t Tag, bool &Handled) {
  Handled = false;
  ສຳລັບ (const DisplayHandler &Disp : DisplayRoutines) {
    ຖ້າ (uint64_t(Disp.Attribute) != Tag)
      continue;
    ຖ້າ (Error E = (this->*Disp.Routine)(static_cast<AttrType>(Tag)))
      ສົ່ງຄືນ E;
    Handled = true;
    bha_nga;
  }
  ສົ່ງຄືນ Error::success();
}
