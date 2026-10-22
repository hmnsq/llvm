//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/RISCVAttributeParser.h"
#شامل "llvm/ADT/StringExtras.h"

using فضاینام llvm;

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

Error RISCVAttributeParser::atomicAbi(unsigned Tag) {
  uint64_t مقدار = de.getULEB128(cursor);
  printAttribute(Tag, مقدار, "Atomic ABI is " + utostr(مقدار));
  بازگشت Error::success();
}

Error RISCVAttributeParser::unalignedAccess(unsigned tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"No unaligned access",
                                        "Unaligned access"};
  بازگشت parseStringAttribute("Unaligned_access", tag, ArrayRef(strings));
}

Error RISCVAttributeParser::پشته_Align(unsigned tag) {
  uint64_t مقدار_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(مقدار_2) + std::string("-bytes");
  printAttribute(tag, مقدار_2, description);
  بازگشت Error::success();
}

Error RISCVAttributeParser::handler(uint64_t tag, منطقی &handled) {
  handled = نادرست;
  برای (ثابت auto &AH : displayRoutines) {
    اگر (uint64_t(AH.attribute) == tag) {
      اگر (Error e = (این->*AH.routine)(tag))
        بازگشت e;
      handled = درست;
      بشکن;
    }
  }

  بازگشت Error::success();
}
