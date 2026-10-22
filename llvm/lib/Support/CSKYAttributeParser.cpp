//===-- CSKYAttributeParser.cpp - CSKY Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/CSKYAttributeParser.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errc.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਅਚਲ CSKYAttributeParser::DisplayHandler
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
            CSKYAttrs::CSKY_DSP_ਸੰਸਕਰਣ,
            &CSKYAttributeParser::dsp_ਸੰਸਕਰਣ,
        },
        {
            CSKYAttrs::CSKY_VDSP_ਸੰਸਕਰਣ,
            &CSKYAttributeParser::vdsp_ਸੰਸਕਰਣ,
        },
        {
            CSKYAttrs::CSKY_FPU_ਸੰਸਕਰਣ,
            &CSKYAttributeParser::fpu_ਸੰਸਕਰਣ,
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
            CSKYAttrs::CSKY_FPU_ਸੰਖਿਆ_MODULE,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_FPU_HARDFP,
            &CSKYAttributeParser::fpuHardFP,
        }};

Error CSKYAttributeParser::handler(uint64_t ਟੈਗ_2, ਤਾਰਕਿਕ &handled) {
  handled = ਝੂਠ;
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &AH : displayRoutines) {
    ਜੇ (uint64_t(AH.attribute) == ਟੈਗ_2) {
      ਜੇ (Error e = (ਇਹ->*AH.routine)(ਟੈਗ_2))
        ਵਾਪਸ e;
      handled = ਸੱਚ;
      ਰੋਕੋ;
    }
  }

  ਵਾਪਸ Error::success();
}

Error CSKYAttributeParser::dsp_ਸੰਸਕਰਣ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Error", "DSP Extension", "DSP 2.0"};
  ਵਾਪਸ parseStringAttribute("Tag_CSKY_DSP_VERSION", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error CSKYAttributeParser::vdsp_ਸੰਸਕਰਣ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  ਵਾਪਸ parseStringAttribute("Tag_CSKY_VDSP_VERSION", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error CSKYAttributeParser::fpu_ਸੰਸਕਰਣ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  ਵਾਪਸ parseStringAttribute("Tag_CSKY_FPU_VERSION", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error CSKYAttributeParser::fpuABI(ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Error", "Soft", "SoftFP", "Hard"};
  ਵਾਪਸ parseStringAttribute("Tag_CSKY_FPU_ABI", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error CSKYAttributeParser::fpuRounding(ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"None", "Needed"};
  ਵਾਪਸ parseStringAttribute("Tag_CSKY_FPU_ROUNDING", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error CSKYAttributeParser::fpuDenormal(ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"None", "Needed"};
  ਵਾਪਸ parseStringAttribute("Tag_CSKY_FPU_DENORMAL", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error CSKYAttributeParser::fpuException(ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"None", "Needed"};
  ਵਾਪਸ parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error CSKYAttributeParser::fpuHardFP(ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ_2) {
  uint64_t ਮੁੱਲ_2 = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  ਜੇ (ਮੁੱਲ_2 & 0x1) {
    description += LS;
    description += "Half";
  }
  ਜੇ ((ਮੁੱਲ_2 >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  ਜੇ ((ਮੁੱਲ_2 >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  ਜੇ (description.ਖਾਲੀ_ਥਾਵਾਂ()) {
    printAttribute(ਟੈਗ_2, ਮੁੱਲ_2, "");
    ਵਾਪਸ createStringError(errc::invalid_ਦਲੀਲ,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(ਮੁੱਲ_2));
  }

  printAttribute(ਟੈਗ_2, ਮੁੱਲ_2, description);
  ਵਾਪਸ Error::success();
}
