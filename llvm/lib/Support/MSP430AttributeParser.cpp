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
         {MSP430Attrs::Tag_তথ্য_Model, &MSP430AttributeParser::parse_তথ্য_Model},
         {MSP430Attrs::Tag_Enum_আকার, &MSP430AttributeParser::parse_Enum_আকার}}};

Error MSP430AttributeParser::parseISA(AttrType Tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক StringVals[] = {"None", "MSP430", "MSP430X"};
  ফেরত parseStringAttribute("ISA", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseCodeModel(AttrType Tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক StringVals[] = {"None", "Small", "Large"};
  ফেরত parseStringAttribute("Code Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_তথ্য_Model(AttrType Tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  ফেরত parseStringAttribute("Data Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parse_Enum_আকার(AttrType Tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  ফেরত parseStringAttribute("Enum Size", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t Tag, যৌক্তিক &Handled) {
  Handled = মিথ্যা;
  জন্য (ধ্রুবক DisplayHandler &Disp : DisplayRoutines) {
    যদি (uint64_t(Disp.Attribute) != Tag)
      চালু;
    যদি (Error E = (এই->*Disp.Routine)(static_cast<AttrType>(Tag)))
      ফেরত E;
    Handled = সত্য;
    বিরতি;
  }
  ফেরত Error::success();
}
