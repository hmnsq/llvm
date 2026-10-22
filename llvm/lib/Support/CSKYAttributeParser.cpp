//===-- CSKYAttributeParser.cpp - CSKY Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/CSKYAttributeParser.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Support/Errc.h"

using नामाकाश llvm;

ध्रुव CSKYAttributeParser::DisplayHandler
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
            CSKYAttrs::CSKY_DSP_VERSION,
            &CSKYAttributeParser::dspVersion,
        },
        {
            CSKYAttrs::CSKY_VDSP_VERSION,
            &CSKYAttributeParser::vdspVersion,
        },
        {
            CSKYAttrs::CSKY_FPU_VERSION,
            &CSKYAttributeParser::fpuVersion,
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
            CSKYAttrs::CSKY_FPU_सङ्ख्या_MODULE,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_FPU_HARDFP,
            &CSKYAttributeParser::fpuHardFP,
        }};

Error CSKYAttributeParser::handler(uint64_t tag, तार्किक &handled) {
  handled = मिच्छा;
  अत्थाय (ध्रुव auto &AH : displayRoutines) {
    सचे (uint64_t(AH.attribute) == tag) {
      सचे (Error e = (एतत्->*AH.routine)(tag))
        पटिनिवत्त e;
      handled = सच्च;
      भङ्ग;
    }
  }

  पटिनिवत्त Error::success();
}

Error CSKYAttributeParser::dspVersion(unsigned tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Error", "DSP Extension", "DSP 2.0"};
  पटिनिवत्त parseStringAttribute("Tag_CSKY_DSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::vdspVersion(unsigned tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  पटिनिवत्त parseStringAttribute("Tag_CSKY_VDSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuVersion(unsigned tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  पटिनिवत्त parseStringAttribute("Tag_CSKY_FPU_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuABI(unsigned tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Error", "Soft", "SoftFP", "Hard"};
  पटिनिवत्त parseStringAttribute("Tag_CSKY_FPU_ABI", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuRounding(unsigned tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"None", "Needed"};
  पटिनिवत्त parseStringAttribute("Tag_CSKY_FPU_ROUNDING", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuDenormal(unsigned tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"None", "Needed"};
  पटिनिवत्त parseStringAttribute("Tag_CSKY_FPU_DENORMAL", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuException(unsigned tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"None", "Needed"};
  पटिनिवत्त parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuHardFP(unsigned tag) {
  uint64_t मानम्_2 = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  सचे (मानम्_2 & 0x1) {
    description += LS;
    description += "Half";
  }
  सचे ((मानम्_2 >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  सचे ((मानम्_2 >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  सचे (description.empty()) {
    printAttribute(tag, मानम्_2, "");
    पटिनिवत्त createStringError(errc::invalid_argument,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(मानम्_2));
  }

  printAttribute(tag, मानम्_2, description);
  पटिनिवत्त Error::success();
}
