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
            CSKYAttrs::CSKY_FPU_संख्या_MODULE,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_FPU_HARDFP,
            &CSKYAttributeParser::fpuHardFP,
        }};

Error CSKYAttributeParser::handler(uint64_t tag, तार्किक &handled) {
  handled = असत्य;
  प्रति (ध्रुव स्वतः &AH : displayRoutines) {
    यदि (uint64_t(AH.attribute) == tag) {
      यदि (Error e = (एतत्->*AH.routine)(tag))
        प्रत्यागम e;
      handled = सत्य;
      भङ्ग;
    }
  }

  प्रत्यागम Error::success();
}

Error CSKYAttributeParser::dspVersion(अचिन्हित tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Error", "DSP Extension", "DSP 2.0"};
  प्रत्यागम parseStringAttribute("Tag_CSKY_DSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::vdspVersion(अचिन्हित tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  प्रत्यागम parseStringAttribute("Tag_CSKY_VDSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuVersion(अचिन्हित tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  प्रत्यागम parseStringAttribute("Tag_CSKY_FPU_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuABI(अचिन्हित tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Error", "Soft", "SoftFP", "Hard"};
  प्रत्यागम parseStringAttribute("Tag_CSKY_FPU_ABI", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuRounding(अचिन्हित tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"None", "Needed"};
  प्रत्यागम parseStringAttribute("Tag_CSKY_FPU_ROUNDING", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuDenormal(अचिन्हित tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"None", "Needed"};
  प्रत्यागम parseStringAttribute("Tag_CSKY_FPU_DENORMAL", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuException(अचिन्हित tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"None", "Needed"};
  प्रत्यागम parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuHardFP(अचिन्हित tag) {
  uint64_t मूल्य_2 = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  यदि (मूल्य_2 & 0x1) {
    description += LS;
    description += "Half";
  }
  यदि ((मूल्य_2 >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  यदि ((मूल्य_2 >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  यदि (description.empty()) {
    printAttribute(tag, मूल्य_2, "");
    प्रत्यागम createStringError(errc::invalid_argument,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(मूल्य_2));
  }

  printAttribute(tag, मूल्य_2, description);
  प्रत्यागम Error::success();
}
