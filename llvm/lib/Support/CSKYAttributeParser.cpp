//===-- CSKYAttributeParser.cpp - CSKY Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/CSKYAttributeParser.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errc.h"

using ನಾಮಾಕಾಶ llvm;

ಧ್ರುವ CSKYAttributeParser::DisplayHandler
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
            CSKYAttrs::CSKY_FPU_ಸಂಖ್ಯೆ_MODULE,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_FPU_HARDFP,
            &CSKYAttributeParser::fpuHardFP,
        }};

Error CSKYAttributeParser::handler(uint64_t tag, ತಾರ್ಕಿಕ &handled) {
  handled = ಅಸತ್ಯ;
  ಪ್ರತಿ (ಧ್ರುವ auto &AH : displayRoutines) {
    ಯದಿ (uint64_t(AH.attribute) == tag) {
      ಯದಿ (Error e = (ಏತತ್->*AH.routine)(tag))
        ಪ್ರತ್ಯಾಗಮ e;
      handled = ಸತ್ಯ;
      ಭಙ್ಗ;
    }
  }

  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error CSKYAttributeParser::dspVersion(unsigned tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Error", "DSP Extension", "DSP 2.0"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("Tag_CSKY_DSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::vdspVersion(unsigned tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("Tag_CSKY_VDSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuVersion(unsigned tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("Tag_CSKY_FPU_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuABI(unsigned tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Error", "Soft", "SoftFP", "Hard"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("Tag_CSKY_FPU_ABI", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuRounding(unsigned tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"None", "Needed"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("Tag_CSKY_FPU_ROUNDING", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuDenormal(unsigned tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"None", "Needed"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("Tag_CSKY_FPU_DENORMAL", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuException(unsigned tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"None", "Needed"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuHardFP(unsigned tag) {
  uint64_t ಮೌಲ್ಯ_2 = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  ಯದಿ (ಮೌಲ್ಯ_2 & 0x1) {
    description += LS;
    description += "Half";
  }
  ಯದಿ ((ಮೌಲ್ಯ_2 >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  ಯದಿ ((ಮೌಲ್ಯ_2 >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  ಯದಿ (description.empty()) {
    printAttribute(tag, ಮೌಲ್ಯ_2, "");
    ಪ್ರತ್ಯಾಗಮ createStringError(errc::invalid_argument,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(ಮೌಲ್ಯ_2));
  }

  printAttribute(tag, ಮೌಲ್ಯ_2, description);
  ಪ್ರತ್ಯಾಗಮ Error::success();
}
