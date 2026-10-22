//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/MSP430AttributeParser.h"
#অন্তর্ভাব "llvm/ADT/ArrayRef.h"

using নামাকাশ llvm;
using নামাকাশ llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::TagISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::TagCodeModel, &MSP430AttributeParser::parseCodeModel},
         {MSP430Attrs::TagDataModel, &MSP430AttributeParser::parseDataModel},
         {MSP430Attrs::TagEnumSize, &MSP430AttributeParser::parseEnumSize}}};

Error MSP430AttributeParser::parseISA(AttrType Tag) {
  স্থির ধ্রুব বর্ণ *ধ্রুব StringVals[] = {"None", "MSP430", "MSP430X"};
  প্রত্যাগম parseStringAttribute("ISA", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseCodeModel(AttrType Tag) {
  স্থির ধ্রুব বর্ণ *ধ্রুব StringVals[] = {"None", "Small", "Large"};
  প্রত্যাগম parseStringAttribute("Code Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseDataModel(AttrType Tag) {
  স্থির ধ্রুব বর্ণ *ধ্রুব StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  প্রত্যাগম parseStringAttribute("Data Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseEnumSize(AttrType Tag) {
  স্থির ধ্রুব বর্ণ *ধ্রুব StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  প্রত্যাগম parseStringAttribute("Enum Size", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t Tag, তার্কিক &Handled) {
  Handled = অসত্য;
  প্রতি (ধ্রুব DisplayHandler &Disp : DisplayRoutines) {
    যদি (uint64_t(Disp.Attribute) != Tag)
      অনুবর্ত;
    যদি (Error E = (এতত্->*Disp.Routine)(static_cast<AttrType>(Tag)))
      প্রত্যাগম E;
    Handled = সত্য;
    ভঙ্গ;
  }
  প্রত্যাগম Error::success();
}
