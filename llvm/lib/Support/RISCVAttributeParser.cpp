//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/Support/RISCVAttributeParser.h"
#অন্তর্ভাৱ "llvm/ADT/StringExtras.h"

using নামাকাশ llvm;

ধ্রুৱ RISCVAttributeParser::DisplayHandler
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
            RISCVAttrs::স্তূপ_ALIGN,
            &RISCVAttributeParser::স্তূপ_Align,
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
  uint64_t মান = de.getULEB128(cursor);
  printAttribute(Tag, মান, "Atomic ABI is " + utostr(মান));
  ঘূৰাই Error::success();
}

Error RISCVAttributeParser::unalignedAccess(unsigned tag) {
  স্থির ধ্রুৱ ৱর্ণ *ধ্রুৱ strings[] = {"No unaligned access",
                                        "Unaligned access"};
  ঘূৰাই parseStringAttribute("Unaligned_access", tag, ArrayRef(strings));
}

Error RISCVAttributeParser::স্তূপ_Align(unsigned tag) {
  uint64_t মান_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(মান_2) + std::string("-bytes");
  printAttribute(tag, মান_2, description);
  ঘূৰাই Error::success();
}

Error RISCVAttributeParser::handler(uint64_t tag, তার্কিক &handled) {
  handled = মিছা;
  বাবে (ধ্রুৱ auto &AH : displayRoutines) {
    যদি (uint64_t(AH.attribute) == tag) {
      যদি (Error e = (এতত্->*AH.routine)(tag))
        ঘূৰাই e;
      handled = সঁচা;
      ভঙ্গ;
    }
  }

  ঘূৰাই Error::success();
}
