//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/RISCVAttributeParser.h"
#ضمن "llvm/ADT/StringExtras.h"

ورتوں نطاق llvm;

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
            RISCVAttrs::ڈھیر_ALIGN,
            &RISCVAttributeParser::ڈھیر_Align,
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

Error RISCVAttributeParser::atomicAbi(بنانشانی ٹیگ) {
  uint64_t قدر = de.getULEB128(cursor);
  printAttribute(ٹیگ, قدر, "Atomic ABI is " + utostr(قدر));
  أرجع Error::success();
}

Error RISCVAttributeParser::unalignedAccess(بنانشانی ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"No unaligned access",
                                        "Unaligned access"};
  أرجع parseStringAttribute("Unaligned_access", ٹیگ_2, ArrayRef(لڑیاں));
}

Error RISCVAttributeParser::ڈھیر_Align(بنانشانی ٹیگ_2) {
  uint64_t قدر_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(قدر_2) + std::string("-bytes");
  printAttribute(ٹیگ_2, قدر_2, description);
  أرجع Error::success();
}

Error RISCVAttributeParser::handler(uint64_t ٹیگ_2, منطقي &handled) {
  handled = خطأ;
  لكل (ثابت خودکار &AH : displayRoutines) {
    إذا (uint64_t(AH.attribute) == ٹیگ_2) {
      إذا (Error e = (هذا->*AH.routine)(ٹیگ_2))
        أرجع e;
      handled = سچ;
      اكسر;
    }
  }

  أرجع Error::success();
}
