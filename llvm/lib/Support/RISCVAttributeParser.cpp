//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/RISCVAttributeParser.h"
#ضمن "llvm/ADT/StringExtras.h"

استفاده نطاق llvm;

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
            RISCVAttrs::پشته_ALIGN,
            &RISCVAttributeParser::پشته_Align,
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

Error RISCVAttributeParser::atomicAbi(بیعلامت برچسب) {
  uint64_t مقدار = de.getULEB128(cursor);
  printAttribute(برچسب, مقدار, "Atomic ABI is " + utostr(مقدار));
  أرجع Error::success();
}

Error RISCVAttributeParser::unalignedAccess(بیعلامت برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"No unaligned access",
                                        "Unaligned access"};
  أرجع parseStringAttribute("Unaligned_access", برچسب_2, ArrayRef(رشتهها));
}

Error RISCVAttributeParser::پشته_Align(بیعلامت برچسب_2) {
  uint64_t مقدار_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(مقدار_2) + std::string("-bytes");
  printAttribute(برچسب_2, مقدار_2, description);
  أرجع Error::success();
}

Error RISCVAttributeParser::handler(uint64_t برچسب_2, منطقي &handled) {
  handled = خطأ;
  لكل (ثابت خودکار &AH : displayRoutines) {
    إذا (uint64_t(AH.attribute) == برچسب_2) {
      إذا (Error e = (هذا->*AH.routine)(برچسب_2))
        أرجع e;
      handled = درست;
      اكسر;
    }
  }

  أرجع Error::success();
}
