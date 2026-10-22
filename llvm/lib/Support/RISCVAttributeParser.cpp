//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/RISCVAttributeParser.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"

using നാമാകാശ llvm;

സ്ഥിരം RISCVAttributeParser::DisplayHandler
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
            RISCVAttrs::അട്ടി_ALIGN,
            &RISCVAttributeParser::അട്ടി_Align,
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
  uint64_t മൂല്യം = de.getULEB128(cursor);
  printAttribute(Tag, മൂല്യം, "Atomic ABI is " + utostr(മൂല്യം));
  തിരികെ Error::success();
}

Error RISCVAttributeParser::unalignedAccess(unsigned tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"No unaligned access",
                                        "Unaligned access"};
  തിരികെ parseStringAttribute("Unaligned_access", tag, ArrayRef(strings));
}

Error RISCVAttributeParser::അട്ടി_Align(unsigned tag) {
  uint64_t മൂല്യം_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(മൂല്യം_2) + std::string("-bytes");
  printAttribute(tag, മൂല്യം_2, description);
  തിരികെ Error::success();
}

Error RISCVAttributeParser::handler(uint64_t tag, തർക്കം &handled) {
  handled = അസത്യം;
  വേണ്ടി (സ്ഥിരം auto &AH : displayRoutines) {
    എങ്കിൽ (uint64_t(AH.attribute) == tag) {
      എങ്കിൽ (Error e = (ഇത്->*AH.routine)(tag))
        തിരികെ e;
      handled = സത്യം;
      നിർത്തുക;
    }
  }

  തിരികെ Error::success();
}
