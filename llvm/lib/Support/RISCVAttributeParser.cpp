//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/RISCVAttributeParser.h"
#include "llvm/ADT/StringExtras.h"

using namespace llvm;

const RISCVAttributeParser::DisplayHandler
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
            RISCVAttrs::सालानसान_ALIGN,
            &RISCVAttributeParser::सालानसान_Align,
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
  uint64_t हालागा = de.getULEB128(cursor);
  इलिम्बाग_Attribute(Tag, हालागा, "Atomic ABI is " + utostr(हालागा));
  return Error::success();
}

Error RISCVAttributeParser::unalignedAccess(unsigned tag) {
  static const char *const strings[] = {"No unaligned access",
                                        "Unaligned access"};
  return parseStringAttribute("Unaligned_access", tag, ArrayRef(strings));
}

Error RISCVAttributeParser::सालानसान_Align(unsigned tag) {
  uint64_t हालागा_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(हालागा_2) + std::string("-bytes");
  इलिम्बाग_Attribute(tag, हालागा_2, description);
  return Error::success();
}

Error RISCVAttributeParser::handler(uint64_t tag, bool &handled) {
  handled = false;
  for (const auto &AH : displayRoutines) {
    if (uint64_t(AH.attribute) == tag) {
      if (Error e = (this->*AH.routine)(tag))
        return e;
      handled = true;
      break;
    }
  }

  return Error::success();
}
