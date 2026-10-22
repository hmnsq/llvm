//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/RISCVAttributeParser.h"
#شامل "llvm/ADT/StringExtras.h"

استعمال نامخلا llvm;

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

Error RISCVAttributeParser::atomicAbi(غیرعلامتی لیبل) {
  uint64_t قدر = de.getULEB128(cursor);
  printAttribute(لیبل, قدر, "Atomic ABI is " + utostr(قدر));
  واپس Error::success();
}

Error RISCVAttributeParser::unalignedAccess(غیرعلامتی لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"No unaligned access",
                                        "Unaligned access"};
  واپس parseStringAttribute("Unaligned_access", لیبل_2, ArrayRef(لڑیاں));
}

Error RISCVAttributeParser::ڈھیر_Align(غیرعلامتی لیبل_2) {
  uint64_t قدر_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(قدر_2) + std::string("-bytes");
  printAttribute(لیبل_2, قدر_2, description);
  واپس Error::success();
}

Error RISCVAttributeParser::handler(uint64_t لیبل_2, منطقی &handled) {
  handled = جھوٹ;
  برائے (ثابت خودکار &AH : displayRoutines) {
    اگر (uint64_t(AH.attribute) == لیبل_2) {
      اگر (Error e = (یہ->*AH.routine)(لیبل_2))
        واپس e;
      handled = سچ;
      توڑو;
    }
  }

  واپس Error::success();
}
