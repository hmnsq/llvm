//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/RISCVAttributeParser.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"

using நாமாகாஶ llvm;

மாறிலி RISCVAttributeParser::DisplayHandler
    RISCVAttributeParser::displayRoutines[] = {
        {
            RISCVAttrs::ARCH,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            RISCVAttrs::PRIV_SPEC,
            &ELFCompactAttrParser::integerAttribute,
        },
        {
            RISCVAttrs::PRIV_SPEC_MINOR,
            &ELFCompactAttrParser::integerAttribute,
        },
        {
            RISCVAttrs::PRIV_SPEC_REVISION,
            &ELFCompactAttrParser::integerAttribute,
        },
        {
            RISCVAttrs::அடுக்கு_ALIGN,
            &RISCVAttributeParser::அடுக்கு_Align,
        },
        {
            RISCVAttrs::UNALIGNED_ACCESS,
            &RISCVAttributeParser::unalignedAccess,
        },
        {
            RISCVAttrs::ATOMIC_ABI,
            &RISCVAttributeParser::atomicAbi,
        },
};

Error RISCVAttributeParser::atomicAbi(unsigned Tag) {
  uint64_t மதிப்பு = de.getULEB128(cursor);
  printAttribute(Tag, மதிப்பு, "Atomic ABI is " + utostr(மதிப்பு));
  திரும்பு Error::success();
}

Error RISCVAttributeParser::unalignedAccess(unsigned tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"No unaligned access",
                                        "Unaligned access"};
  திரும்பு parseStringAttribute("Unaligned_access", tag, ArrayRef(strings));
}

Error RISCVAttributeParser::அடுக்கு_Align(unsigned tag) {
  uint64_t மதிப்பு_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(மதிப்பு_2) + std::string("-bytes");
  printAttribute(tag, மதிப்பு_2, description);
  திரும்பு Error::success();
}

Error RISCVAttributeParser::handler(uint64_t tag, தருக்க &handled) {
  handled = பொய்;
  க்கு (மாறிலி auto &AH : displayRoutines) {
    எனில் (uint64_t(AH.attribute) == tag) {
      எனில் (Error e = (இது->*AH.routine)(tag))
        திரும்பு e;
      handled = உண்மை;
      நிறுத்து;
    }
  }

  திரும்பு Error::success();
}
