//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/RISCVAttributeParser.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"

using ନାମାକାଶ llvm;

ଅଚଳ RISCVAttributeParser::DisplayHandler
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
            RISCVAttrs::ଗଦା_ALIGN,
            &RISCVAttributeParser::ଗଦା_Align,
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
  uint64_t ମୂଲ୍ୟ = de.getULEB128(cursor);
  printAttribute(Tag, ମୂଲ୍ୟ, "Atomic ABI is " + utostr(ମୂଲ୍ୟ));
  ଫେରନ୍ତୁ Error::success();
}

Error RISCVAttributeParser::unalignedAccess(unsigned tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"No unaligned access",
                                        "Unaligned access"};
  ଫେରନ୍ତୁ parseStringAttribute("Unaligned_access", tag, ArrayRef(strings));
}

Error RISCVAttributeParser::ଗଦା_Align(unsigned tag) {
  uint64_t ମୂଲ୍ୟ_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(ମୂଲ୍ୟ_2) + std::string("-bytes");
  printAttribute(tag, ମୂଲ୍ୟ_2, description);
  ଫେରନ୍ତୁ Error::success();
}

Error RISCVAttributeParser::handler(uint64_t tag, ତାର୍କିକ &handled) {
  handled = ମିଥ୍ୟା;
  ପାଇଁ (ଅଚଳ auto &AH : displayRoutines) {
    ଯଦି (uint64_t(AH.attribute) == tag) {
      ଯଦି (Error e = (ଏହି->*AH.routine)(tag))
        ଫେରନ୍ତୁ e;
      handled = ସତ୍ୟ;
      ଭାଙ୍ଗ;
    }
  }

  ଫେରନ୍ତୁ Error::success();
}
