//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/RISCVAttributeParser.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਅਚਲ RISCVAttributeParser::DisplayHandler
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
            RISCVAttrs::ਢੇਰ_ALIGN,
            &RISCVAttributeParser::ਢੇਰ_Align,
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

Error RISCVAttributeParser::atomicAbi(ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ) {
  uint64_t ਮੁੱਲ = de.getULEB128(cursor);
  printAttribute(ਟੈਗ, ਮੁੱਲ, "Atomic ABI is " + utostr(ਮੁੱਲ));
  ਵਾਪਸ Error::success();
}

Error RISCVAttributeParser::unalignedAccess(ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"No unaligned access",
                                        "Unaligned access"};
  ਵਾਪਸ parseStringAttribute("Unaligned_access", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error RISCVAttributeParser::ਢੇਰ_Align(ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ_2) {
  uint64_t ਮੁੱਲ_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(ਮੁੱਲ_2) + std::string("-bytes");
  printAttribute(ਟੈਗ_2, ਮੁੱਲ_2, description);
  ਵਾਪਸ Error::success();
}

Error RISCVAttributeParser::handler(uint64_t ਟੈਗ_2, ਤਾਰਕਿਕ &handled) {
  handled = ਝੂਠ;
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &AH : displayRoutines) {
    ਜੇ (uint64_t(AH.attribute) == ਟੈਗ_2) {
      ਜੇ (Error e = (ਇਹ->*AH.routine)(ਟੈਗ_2))
        ਵਾਪਸ e;
      handled = ਸੱਚ;
      ਰੋਕੋ;
    }
  }

  ਵਾਪਸ Error::success();
}
