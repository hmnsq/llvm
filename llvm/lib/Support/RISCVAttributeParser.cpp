//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/RISCVAttributeParser.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"

using नामाकाश llvm;

ध्रुव RISCVAttributeParser::DisplayHandler
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
            RISCVAttrs::राशिः_ALIGN,
            &RISCVAttributeParser::राशिः_Align,
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
  uint64_t मानम् = de.getULEB128(cursor);
  printAttribute(Tag, मानम्, "Atomic ABI is " + utostr(मानम्));
  पटिनिवत्त Error::success();
}

Error RISCVAttributeParser::unalignedAccess(unsigned tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"No unaligned access",
                                        "Unaligned access"};
  पटिनिवत्त parseStringAttribute("Unaligned_access", tag, ArrayRef(strings));
}

Error RISCVAttributeParser::राशिः_Align(unsigned tag) {
  uint64_t मानम्_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(मानम्_2) + std::string("-bytes");
  printAttribute(tag, मानम्_2, description);
  पटिनिवत्त Error::success();
}

Error RISCVAttributeParser::handler(uint64_t tag, तार्किक &handled) {
  handled = मिच्छा;
  अत्थाय (ध्रुव auto &AH : displayRoutines) {
    सचे (uint64_t(AH.attribute) == tag) {
      सचे (Error e = (एतत्->*AH.routine)(tag))
        पटिनिवत्त e;
      handled = सच्च;
      भङ्ग;
    }
  }

  पटिनिवत्त Error::success();
}
