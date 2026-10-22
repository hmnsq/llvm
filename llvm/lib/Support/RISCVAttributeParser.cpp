//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/RISCVAttributeParser.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"

using ನಾಮಾಕಾಶ llvm;

ಧ್ರುವ RISCVAttributeParser::DisplayHandler
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
            RISCVAttrs::ರಾಶಿ_ALIGN,
            &RISCVAttributeParser::ರಾಶಿ_Align,
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
  uint64_t ಮೌಲ್ಯ = de.getULEB128(cursor);
  printAttribute(Tag, ಮೌಲ್ಯ, "Atomic ABI is " + utostr(ಮೌಲ್ಯ));
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error RISCVAttributeParser::unalignedAccess(unsigned tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"No unaligned access",
                                        "Unaligned access"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("Unaligned_access", tag, ArrayRef(strings));
}

Error RISCVAttributeParser::ರಾಶಿ_Align(unsigned tag) {
  uint64_t ಮೌಲ್ಯ_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(ಮೌಲ್ಯ_2) + std::string("-bytes");
  printAttribute(tag, ಮೌಲ್ಯ_2, description);
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error RISCVAttributeParser::handler(uint64_t tag, ತಾರ್ಕಿಕ &handled) {
  handled = ಅಸತ್ಯ;
  ಪ್ರತಿ (ಧ್ರುವ auto &AH : displayRoutines) {
    ಯದಿ (uint64_t(AH.attribute) == tag) {
      ಯದಿ (Error e = (ಏತತ್->*AH.routine)(tag))
        ಪ್ರತ್ಯಾಗಮ e;
      handled = ಸತ್ಯ;
      ಭಙ್ಗ;
    }
  }

  ಪ್ರತ್ಯಾಗಮ Error::success();
}
