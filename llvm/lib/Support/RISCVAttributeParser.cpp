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

स्थिरांक RISCVAttributeParser::DisplayHandler
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

Error RISCVAttributeParser::atomicAbi(अचिन्हित Tag) {
  uint64_t मूल्य = de.getULEB128(cursor);
  printAttribute(Tag, मूल्य, "Atomic ABI is " + utostr(मूल्य));
  परत Error::success();
}

Error RISCVAttributeParser::unalignedAccess(अचिन्हित tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"No unaligned access",
                                        "Unaligned access"};
  परत parseStringAttribute("Unaligned_access", tag, ArrayRef(strings));
}

Error RISCVAttributeParser::चळत_Align(अचिन्हित tag) {
  uint64_t मूल्य_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(मूल्य_2) + std::string("-bytes");
  printAttribute(tag, मूल्य_2, description);
  परत Error::success();
}

Error RISCVAttributeParser::handler(uint64_t tag, तार्किक &handled) {
  handled = असत्य;
  साठी (स्थिरांक स्वतः &AH : displayRoutines) {
    जर (uint64_t(AH.attribute) == tag) {
      जर (Error e = (हे->*AH.routine)(tag))
        परत e;
      handled = सत्य;
      थांबा;
    }
  }

  परत Error::success();
}
