//===-- CSKYAttributeParser.cpp - CSKY Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/CSKYAttributeParser.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/Support/Errc.h"

ఉపయోగించి నామాకాశ llvm;

స్థిరరాశి CSKYAttributeParser::DisplayHandler
    CSKYAttributeParser::displayRoutines[] = {
        {
            CSKYAttrs::CSKY_ARCH_NAME,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_CPU_NAME,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_ISA_FLAGS,
            &ELFCompactAttrParser::integerAttribute,
        },
        {
            CSKYAttrs::CSKY_ISA_EXT_FLAGS,
            &ELFCompactAttrParser::integerAttribute,
        },
        {
            CSKYAttrs::CSKY_DSP_సంస్కరణ,
            &CSKYAttributeParser::dsp_సంస్కరణ,
        },
        {
            CSKYAttrs::CSKY_VDSP_సంస్కరణ,
            &CSKYAttributeParser::vdsp_సంస్కరణ,
        },
        {
            CSKYAttrs::CSKY_FPU_సంస్కరణ,
            &CSKYAttributeParser::fpu_సంస్కరణ,
        },
        {
            CSKYAttrs::CSKY_FPU_ABI,
            &CSKYAttributeParser::fpuABI,
        },
        {
            CSKYAttrs::CSKY_FPU_ROUNDING,
            &CSKYAttributeParser::fpuRounding,
        },
        {
            CSKYAttrs::CSKY_FPU_DENORMAL,
            &CSKYAttributeParser::fpuDenormal,
        },
        {
            CSKYAttrs::CSKY_FPU_EXCEPTION,
            &CSKYAttributeParser::fpuException,
        },
        {
            CSKYAttrs::CSKY_FPU_సంఖ్య_MODULE,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_FPU_HARDFP,
            &CSKYAttributeParser::fpuHardFP,
        }};

Error CSKYAttributeParser::handler(uint64_t ట్యాగ్_2, తార్కిక &handled) {
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

Error CSKYAttributeParser::dsp_సంస్కరణ(గుర్తులేని ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Error", "DSP Extension", "DSP 2.0"};
  తిరిగి parseStringAttribute("Tag_CSKY_DSP_VERSION", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error CSKYAttributeParser::vdsp_సంస్కరణ(గుర్తులేని ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  తిరిగి parseStringAttribute("Tag_CSKY_VDSP_VERSION", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error CSKYAttributeParser::fpu_సంస్కరణ(గుర్తులేని ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  తిరిగి parseStringAttribute("Tag_CSKY_FPU_VERSION", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error CSKYAttributeParser::fpuABI(గుర్తులేని ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Error", "Soft", "SoftFP", "Hard"};
  తిరిగి parseStringAttribute("Tag_CSKY_FPU_ABI", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error CSKYAttributeParser::fpuRounding(గుర్తులేని ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"None", "Needed"};
  తిరిగి parseStringAttribute("Tag_CSKY_FPU_ROUNDING", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error CSKYAttributeParser::fpuDenormal(గుర్తులేని ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"None", "Needed"};
  తిరిగి parseStringAttribute("Tag_CSKY_FPU_DENORMAL", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error CSKYAttributeParser::fpuException(గుర్తులేని ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"None", "Needed"};
  తిరిగి parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error CSKYAttributeParser::fpuHardFP(గుర్తులేని ట్యాగ్_2) {
  uint64_t విలువ_2 = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  ఒకవేళ (విలువ_2 & 0x1) {
    description += LS;
    description += "Half";
  }
  ఒకవేళ ((విలువ_2 >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  ఒకవేళ ((విలువ_2 >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  ఒకవేళ (description.ఖాళీలు()) {
    printAttribute(ట్యాగ్_2, విలువ_2, "");
    తిరిగి createStringError(errc::invalid_వాదం,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(విలువ_2));
  }

  printAttribute(ట్యాగ్_2, విలువ_2, description);
  తిరిగి Error::success();
}
