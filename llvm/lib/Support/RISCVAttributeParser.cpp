//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/RISCVAttributeParser.h"
#ضمن "llvm/ADT/StringExtras.h"

استعمال نطاق llvm;

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
            RISCVAttrs::ڍير_ALIGN,
            &RISCVAttributeParser::ڍير_Align,
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

Error RISCVAttributeParser::atomicAbi(بنانشان ٽيگ) {
  uint64_t قدر = de.getULEB128(cursor);
  printAttribute(ٽيگ, قدر, "Atomic ABI is " + utostr(قدر));
  أرجع Error::success();
}

Error RISCVAttributeParser::unalignedAccess(بنانشان ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"No unaligned access",
                                        "Unaligned access"};
  أرجع parseStringAttribute("Unaligned_access", ٽيگ_2, ArrayRef(لڙيون));
}

Error RISCVAttributeParser::ڍير_Align(بنانشان ٽيگ_2) {
  uint64_t قدر_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(قدر_2) + std::string("-bytes");
  printAttribute(ٽيگ_2, قدر_2, description);
  أرجع Error::success();
}

Error RISCVAttributeParser::handler(uint64_t ٽيگ_2, منطقي &handled) {
  handled = خطأ;
  لكل (ثابت خودڪار &AH : displayRoutines) {
    إذا (uint64_t(AH.attribute) == ٽيگ_2) {
      إذا (Error e = (هذا->*AH.routine)(ٽيگ_2))
        أرجع e;
      handled = سچ;
      اكسر;
    }
  }

  أرجع Error::success();
}
