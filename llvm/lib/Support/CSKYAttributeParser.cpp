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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಸ್ಥಿರಾಂಕ CSKYAttributeParser::DisplayHandler
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
            CSKYAttrs::CSKY_DSP_ಆವೃತ್ತಿ,
            &CSKYAttributeParser::dsp_ಆವೃತ್ತಿ,
        },
        {
            CSKYAttrs::CSKY_VDSP_ಆವೃತ್ತಿ,
            &CSKYAttributeParser::vdsp_ಆವೃತ್ತಿ,
        },
        {
            CSKYAttrs::CSKY_FPU_ಆವೃತ್ತಿ,
            &CSKYAttributeParser::fpu_ಆವೃತ್ತಿ,
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

Error CSKYAttributeParser::handler(uint64_t ಟ್ಯಾಗ್_2, ತಾರ್ಕಿಕ &handled) {
  handled = ಸುಳ್ಳು;
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &AH : displayRoutines) {
    ಆದರೆ (uint64_t(AH.attribute) == ಟ್ಯಾಗ್_2) {
      ಆದರೆ (Error e = (ಇದು->*AH.routine)(ಟ್ಯಾಗ್_2))
        ಹಿಂತಿರುಗಿ e;
      handled = ನಿಜ;
      ನಿಲ್ಲಿಸು;
    }
  }

  ಹಿಂತಿರುಗಿ Error::success();
}

Error CSKYAttributeParser::dsp_ಆವೃತ್ತಿ(ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Error", "DSP Extension", "DSP 2.0"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("Tag_CSKY_DSP_VERSION", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error CSKYAttributeParser::vdsp_ಆವೃತ್ತಿ(ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("Tag_CSKY_VDSP_VERSION", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error CSKYAttributeParser::fpu_ಆವೃತ್ತಿ(ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("Tag_CSKY_FPU_VERSION", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error CSKYAttributeParser::fpuABI(ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Error", "Soft", "SoftFP", "Hard"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("Tag_CSKY_FPU_ABI", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error CSKYAttributeParser::fpuRounding(ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"None", "Needed"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("Tag_CSKY_FPU_ROUNDING", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error CSKYAttributeParser::fpuDenormal(ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"None", "Needed"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("Tag_CSKY_FPU_DENORMAL", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error CSKYAttributeParser::fpuException(ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"None", "Needed"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error CSKYAttributeParser::fpuHardFP(ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್_2) {
  uint64_t ಮೌಲ್ಯ_2 = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  ಆದರೆ (ಮೌಲ್ಯ_2 & 0x1) {
    description += LS;
    description += "Half";
  }
  ಆದರೆ ((ಮೌಲ್ಯ_2 >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  ಆದರೆ ((ಮೌಲ್ಯ_2 >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  ಆದರೆ (description.ಖಾಲಿಗಳು()) {
    printAttribute(ಟ್ಯಾಗ್_2, ಮೌಲ್ಯ_2, "");
    ಹಿಂತಿರುಗಿ createStringError(errc::invalid_ವಾದ,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(ಮೌಲ್ಯ_2));
  }

  printAttribute(ಟ್ಯಾಗ್_2, ಮೌಲ್ಯ_2, description);
  ಹಿಂತಿರುಗಿ Error::success();
}
