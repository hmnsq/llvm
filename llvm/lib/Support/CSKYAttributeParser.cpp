//===-- CSKYAttributeParser.cpp - CSKY Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/CSKYAttributeParser.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/Errc.h"

using نطاق llvm;

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
            CSKYAttrs::CSKY_FPU_NUMBER_MODULE,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_FPU_HARDFP,
            &CSKYAttributeParser::fpuHardFP,
        }};

Error CSKYAttributeParser::handler(uint64_t tag, منطقي &handled) {
  handled = خطأ;
  لكل (ثابت auto &AH : displayRoutines) {
    𐡄𐡍 (uint64_t(AH.attribute) == tag) {
      𐡄𐡍 (Error e = (هذا->*AH.routine)(tag))
        أرجع e;
      handled = 𐡒𐡔𐡈;
      اكسر;
    }
  }

  أرجع Error::success();
}

Error CSKYAttributeParser::dspVersion(unsigned tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Error", "DSP Extension", "DSP 2.0"};
  أرجع parseStringAttribute("Tag_CSKY_DSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::vdspVersion(unsigned tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  أرجع parseStringAttribute("Tag_CSKY_VDSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuVersion(unsigned tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuABI(unsigned tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Error", "Soft", "SoftFP", "Hard"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_ABI", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuRounding(unsigned tag) {
  ساكن ثابت حرف *ثابت strings[] = {"None", "Needed"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_ROUNDING", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuDenormal(unsigned tag) {
  ساكن ثابت حرف *ثابت strings[] = {"None", "Needed"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_DENORMAL", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuException(unsigned tag) {
  ساكن ثابت حرف *ثابت strings[] = {"None", "Needed"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuHardFP(unsigned tag) {
  uint64_t value = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  𐡄𐡍 (value & 0x1) {
    description += LS;
    description += "Half";
  }
  𐡄𐡍 ((value >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  𐡄𐡍 ((value >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  𐡄𐡍 (description.empty()) {
    printAttribute(tag, value, "");
    أرجع createStringError(errc::invalid_argument,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(value));
  }

  printAttribute(tag, value, description);
  أرجع Error::success();
}
