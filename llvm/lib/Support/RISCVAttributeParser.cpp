//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/RISCVAttributeParser.h"
#समावेश "llvm/ADT/StringExtras.h"

using namespace llvm;

स्थिर RISCVAttributeParser::DisplayHandler
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
            RISCVAttrs::ढेर_ALIGN,
            &RISCVAttributeParser::ढेर_Align,
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

Error RISCVAttributeParser::atomicAbi(अचिन्हित Tag) {
  uint64_t मूल्य = de.getULEB128(cursor);
  printAttribute(Tag, मूल्य, "Atomic ABI is " + utostr(मूल्य));
  वापस Error::success();
}

Error RISCVAttributeParser::unalignedAccess(अचिन्हित tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"No unaligned access",
                                        "Unaligned access"};
  वापस parseStringAttribute("Unaligned_access", tag, ArrayRef(strings));
}

Error RISCVAttributeParser::ढेर_Align(अचिन्हित tag) {
  uint64_t मूल्य_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(मूल्य_2) + std::string("-bytes");
  printAttribute(tag, मूल्य_2, description);
  वापस Error::success();
}

Error RISCVAttributeParser::handler(uint64_t tag, bool &handled) {
  handled = false;
  क्रम (स्थिर स्वतः &AH : displayRoutines) {
    अगर (uint64_t(AH.attribute) == tag) {
      अगर (Error e = (यह->*AH.routine)(tag))
        वापस e;
      handled = true;
      अवरोध;
    }
  }

  वापस Error::success();
}
