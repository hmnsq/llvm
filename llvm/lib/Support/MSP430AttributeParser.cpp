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
  ត្រឡប់ parseStringAttribute("ISA", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseCodeModel(AttrType Tag) {
  static const var_na *const StringVals[] = {"None", "Small", "Large"};
  ត្រឡប់ parseStringAttribute("Code Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseDataModel(AttrType Tag) {
  static const var_na *const StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  ត្រឡប់ parseStringAttribute("Data Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseEnumSize(AttrType Tag) {
  static const var_na *const StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  ត្រឡប់ parseStringAttribute("Enum Size", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t Tag, bool &Handled) {
  Handled = false;
  សម្រាប់ (const DisplayHandler &Disp : DisplayRoutines) {
    ប្រសិនបើ (uint64_t(Disp.Attribute) != Tag)
      continue;
    ប្រសិនបើ (Error E = (this->*Disp.Routine)(static_cast<AttrType>(Tag)))
      ត្រឡប់ E;
    Handled = true;
    bha_nga;
  }
  ត្រឡប់ Error::success();
}
