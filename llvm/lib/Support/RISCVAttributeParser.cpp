//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/RISCVAttributeParser.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"

उपयुज्य नामाकाश llvm;

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

Error RISCVAttributeParser::atomicAbi(अचिह्नित चिह्नकम्) {
  uint64_t मानम् = de.getULEB128(cursor);
  printAttribute(चिह्नकम्, मानम्, "Atomic ABI is " + utostr(मानम्));
  प्रत्यागम Error::success();
}

Error RISCVAttributeParser::unalignedAccess(अचिह्नित चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"No unaligned access",
                                        "Unaligned access"};
  प्रत्यागम parseStringAttribute("Unaligned_access", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error RISCVAttributeParser::राशिः_Align(अचिह्नित चिह्नकम्_2) {
  uint64_t मानम्_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(मानम्_2) + std::string("-bytes");
  printAttribute(चिह्नकम्_2, मानम्_2, description);
  प्रत्यागम Error::success();
}

Error RISCVAttributeParser::handler(uint64_t चिह्नकम्_2, तार्किक &handled) {
  handled = असत्य;
  प्रति (ध्रुव स्वचालित &AH : displayRoutines) {
    यदि (uint64_t(AH.attribute) == चिह्नकम्_2) {
      यदि (Error e = (एतत्->*AH.routine)(चिह्नकम्_2))
        प्रत्यागम e;
      handled = सत्य;
      भङ्ग;
    }
  }

  प्रत्यागम Error::success();
}
