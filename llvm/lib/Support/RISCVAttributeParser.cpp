//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/RISCVAttributeParser.h"
#ضمن "llvm/ADT/StringExtras.h"

using نطاق llvm;

ثابت RISCVAttributeParser::DisplayHandler
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
            RISCVAttrs::مكدس_ALIGN,
            &RISCVAttributeParser::مكدس_Align,
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
  uint64_t قيمة = de.getULEB128(cursor);
  اطبع_Attribute(Tag, قيمة, "Atomic ABI is " + utostr(قيمة));
  أرجع Error::success();
}

Error RISCVAttributeParser::unalignedAccess(unsigned tag) {
  ساكن ثابت حرف *ثابت strings[] = {"No unaligned access",
                                        "Unaligned access"};
  أرجع parseStringAttribute("Unaligned_access", tag, ArrayRef(strings));
}

Error RISCVAttributeParser::مكدس_Align(unsigned tag) {
  uint64_t قيمة_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(قيمة_2) + std::string("-bytes");
  اطبع_Attribute(tag, قيمة_2, description);
  أرجع Error::success();
}

Error RISCVAttributeParser::handler(uint64_t tag, منطقي &handled) {
  handled = خطأ;
  لكل (ثابت auto &AH : displayRoutines) {
    إذا (uint64_t(AH.attribute) == tag) {
      إذا (Error e = (هذا->*AH.routine)(tag))
        أرجع e;
      handled = true;
      اكسر;
    }
  }

  أرجع Error::success();
}
