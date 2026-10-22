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

अचर RISCVAttributeParser::DisplayHandler
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
            RISCVAttrs::थाक_ALIGN,
            &RISCVAttributeParser::थाक_Align,
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
  uint64_t मान = de.getULEB128(cursor);
  printAttribute(Tag, मान, "Atomic ABI is " + utostr(मान));
  फर्काऊ Error::success();
}

Error RISCVAttributeParser::unalignedAccess(अचिन्हित tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"No unaligned access",
                                        "Unaligned access"};
  फर्काऊ parseStringAttribute("Unaligned_access", tag, ArrayRef(strings));
}

Error RISCVAttributeParser::थाक_Align(अचिन्हित tag) {
  uint64_t मान_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(मान_2) + std::string("-bytes");
  printAttribute(tag, मान_2, description);
  फर्काऊ Error::success();
}

Error RISCVAttributeParser::handler(uint64_t tag, तार्किक &handled) {
  handled = असत्य;
  लागि (अचर स्वतः &AH : displayRoutines) {
    यदि (uint64_t(AH.attribute) == tag) {
      यदि (Error e = (एतत्->*AH.routine)(tag))
        फर्काऊ e;
      handled = सत्य;
      रोक;
    }
  }

  फर्काऊ Error::success();
}
