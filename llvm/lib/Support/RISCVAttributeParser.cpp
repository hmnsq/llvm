//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/RISCVAttributeParser.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"

using નામાકાશ llvm;

અચલ RISCVAttributeParser::DisplayHandler
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
            RISCVAttrs::ઢગલો_ALIGN,
            &RISCVAttributeParser::ઢગલો_Align,
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
  uint64_t કિંમત = de.getULEB128(cursor);
  printAttribute(Tag, કિંમત, "Atomic ABI is " + utostr(કિંમત));
  પાછા Error::success();
}

Error RISCVAttributeParser::unalignedAccess(unsigned tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"No unaligned access",
                                        "Unaligned access"};
  પાછા parseStringAttribute("Unaligned_access", tag, ArrayRef(strings));
}

Error RISCVAttributeParser::ઢગલો_Align(unsigned tag) {
  uint64_t કિંમત_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(કિંમત_2) + std::string("-bytes");
  printAttribute(tag, કિંમત_2, description);
  પાછા Error::success();
}

Error RISCVAttributeParser::handler(uint64_t tag, તાર્કિક &handled) {
  handled = ખોટું;
  માટે (અચલ auto &AH : displayRoutines) {
    જો (uint64_t(AH.attribute) == tag) {
      જો (Error e = (આ->*AH.routine)(tag))
        પાછા e;
      handled = સાચું;
      તોડો;
    }
  }

  પાછા Error::success();
}
