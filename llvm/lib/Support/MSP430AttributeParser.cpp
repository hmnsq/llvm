//===-- MSP430AttributeParser.cpp - MSP430 Attribute Parser ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/MSP430AttributeParser.h"
#அந்தர்பாவ "llvm/ADT/ArrayRef.h"

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::MSP430Attrs;

constexpr std::array<MSP430AttributeParser::DisplayHandler, 4>
    MSP430AttributeParser::DisplayRoutines{
        {{MSP430Attrs::TagISA, &MSP430AttributeParser::parseISA},
         {MSP430Attrs::TagCodeModel, &MSP430AttributeParser::parseCodeModel},
         {MSP430Attrs::TagDataModel, &MSP430AttributeParser::parseDataModel},
         {MSP430Attrs::TagEnumSize, &MSP430AttributeParser::parseEnumSize}}};

Error MSP430AttributeParser::parseISA(AttrType Tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி StringVals[] = {"None", "MSP430", "MSP430X"};
  திரும்பு parseStringAttribute("ISA", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseCodeModel(AttrType Tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி StringVals[] = {"None", "Small", "Large"};
  திரும்பு parseStringAttribute("Code Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseDataModel(AttrType Tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி StringVals[] = {"None", "Small", "Large",
                                           "Restricted"};
  திரும்பு parseStringAttribute("Data Model", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::parseEnumSize(AttrType Tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி StringVals[] = {"None", "Small", "Integer",
                                           "Don't Care"};
  திரும்பு parseStringAttribute("Enum Size", Tag, ArrayRef(StringVals));
}

Error MSP430AttributeParser::handler(uint64_t Tag, தருக்க &Handled) {
  Handled = பொய்;
  க்கு (மாறிலி DisplayHandler &Disp : DisplayRoutines) {
    எனில் (uint64_t(Disp.Attribute) != Tag)
      தொடர்;
    எனில் (Error E = (இது->*Disp.Routine)(static_cast<AttrType>(Tag)))
      திரும்பு E;
    Handled = உண்மை;
    நிறுத்து;
  }
  திரும்பு Error::success();
}
