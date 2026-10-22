//===-- RISCVAttributeParser.cpp - RISCV Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/RISCVAttributeParser.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"

ఉపయోగించి నామాకాశ llvm;

స్థిరరాశి RISCVAttributeParser::DisplayHandler
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
            RISCVAttrs::దొంతర_ALIGN,
            &RISCVAttributeParser::దొంతర_Align,
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

Error RISCVAttributeParser::atomicAbi(గుర్తులేని ట్యాగ్) {
  uint64_t విలువ = de.getULEB128(cursor);
  printAttribute(ట్యాగ్, విలువ, "Atomic ABI is " + utostr(విలువ));
  తిరిగి Error::success();
}

Error RISCVAttributeParser::unalignedAccess(గుర్తులేని ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"No unaligned access",
                                        "Unaligned access"};
  తిరిగి parseStringAttribute("Unaligned_access", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error RISCVAttributeParser::దొంతర_Align(గుర్తులేని ట్యాగ్_2) {
  uint64_t విలువ_2 = de.getULEB128(cursor);
  std::string description =
      "Stack alignment is " + utostr(విలువ_2) + std::string("-bytes");
  printAttribute(ట్యాగ్_2, విలువ_2, description);
  తిరిగి Error::success();
}

Error RISCVAttributeParser::handler(uint64_t ట్యాగ్_2, తార్కిక &handled) {
  handled = అబద్ధం;
  కొరకు (స్థిరరాశి స్వయంచాలక &AH : displayRoutines) {
    ఒకవేళ (uint64_t(AH.attribute) == ట్యాగ్_2) {
      ఒకవేళ (Error e = (ఇది->*AH.routine)(ట్యాగ్_2))
        తిరిగి e;
      handled = నిజం;
      ఆపు;
    }
  }

  తిరిగి Error::success();
}
