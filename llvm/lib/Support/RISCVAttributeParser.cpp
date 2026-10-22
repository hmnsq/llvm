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
            RISCVAttrs::चळत_ALIGN,
            &RISCVAttributeParser::चळत_Align,
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
  uint64_t मूल्य = de.getULEB128(cursor);
  printAttribute(Tag, मूल्य, "Atomic ABI is " + utostr(मूल्य));
  प्रत्यागम Error::success();
}

Error RISCVAttributeParser::unalignedAccess(unsigned tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"No unaligned access",
                                        "Unaligned access"};
  प्रत्यागम parseStringAttribute("Unaligned_access", tag, ArrayRef(strings));
}

Error RISCVAttributeParser::चळत_Align(unsigned tag) {
  uint64_t मूल्य_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(मूल्य_2) + std::string("-bytes");
  printAttribute(tag, मूल्य_2, description);
  प्रत्यागम Error::success();
}

Error RISCVAttributeParser::handler(uint64_t tag, तार्किक &handled) {
  handled = असत्य;
  प्रति (ध्रुव auto &AH : displayRoutines) {
    यदि (uint64_t(AH.attribute) == tag) {
      यदि (Error e = (एतत्->*AH.routine)(tag))
        प्रत्यागम e;
      handled = सत्य;
      भङ्ग;
    }
  }

  प्रत्यागम Error::success();
}
