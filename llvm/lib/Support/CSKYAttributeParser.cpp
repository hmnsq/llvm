//===-- CSKYAttributeParser.cpp - CSKY Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/CSKYAttributeParser.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/Errc.h"

using فضاینام llvm;

ثابت CSKYAttributeParser::DisplayHandler
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
            CSKYAttrs::CSKY_FPU_عدد_MODULE,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_FPU_HARDFP,
            &CSKYAttributeParser::fpuHardFP,
        }};

Error CSKYAttributeParser::handler(uint64_t tag, منطقی &handled) {
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

Error CSKYAttributeParser::dspVersion(unsigned tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Error", "DSP Extension", "DSP 2.0"};
  بازگشت parseStringAttribute("Tag_CSKY_DSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::vdspVersion(unsigned tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  بازگشت parseStringAttribute("Tag_CSKY_VDSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuVersion(unsigned tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  بازگشت parseStringAttribute("Tag_CSKY_FPU_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuABI(unsigned tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Error", "Soft", "SoftFP", "Hard"};
  بازگشت parseStringAttribute("Tag_CSKY_FPU_ABI", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuRounding(unsigned tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"None", "Needed"};
  بازگشت parseStringAttribute("Tag_CSKY_FPU_ROUNDING", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuDenormal(unsigned tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"None", "Needed"};
  بازگشت parseStringAttribute("Tag_CSKY_FPU_DENORMAL", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuException(unsigned tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"None", "Needed"};
  بازگشت parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuHardFP(unsigned tag) {
  uint64_t مقدار_2 = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  اگر (مقدار_2 & 0x1) {
    description += LS;
    description += "Half";
  }
  اگر ((مقدار_2 >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  اگر ((مقدار_2 >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  اگر (description.empty()) {
    printAttribute(tag, مقدار_2, "");
    بازگشت createStringError(errc::invalid_argument,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(مقدار_2));
  }

  printAttribute(tag, مقدار_2, description);
  بازگشت Error::success();
}
