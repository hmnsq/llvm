//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/RISCVAttributeParser.h"
#ضمن "llvm/ADT/StringExtras.h"

کارول نطاق llvm;

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
            RISCVAttrs::ډېری_ALIGN,
            &RISCVAttributeParser::ډېری_Align,
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

Error RISCVAttributeParser::atomicAbi(بینښه نښان) {
  uint64_t ارزښت = de.getULEB128(cursor);
  printAttribute(نښان, ارزښت, "Atomic ABI is " + utostr(ارزښت));
  أرجع Error::success();
}

Error RISCVAttributeParser::unalignedAccess(بینښه نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"No unaligned access",
                                        "Unaligned access"};
  أرجع parseStringAttribute("Unaligned_access", نښان_2, ArrayRef(مزي_ګانې));
}

Error RISCVAttributeParser::ډېری_Align(بینښه نښان_2) {
  uint64_t ارزښت_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(ارزښت_2) + std::string("-bytes");
  printAttribute(نښان_2, ارزښت_2, description);
  أرجع Error::success();
}

Error RISCVAttributeParser::handler(uint64_t نښان_2, منطقي &handled) {
  handled = خطأ;
  لكل (ثابت خپلکاری &AH : displayRoutines) {
    إذا (uint64_t(AH.attribute) == نښان_2) {
      إذا (Error e = (هذا->*AH.routine)(نښان_2))
        أرجع e;
      handled = ریښتیا;
      اكسر;
    }
  }

  أرجع Error::success();
}
