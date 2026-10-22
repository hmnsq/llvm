//===-- CSKYAttributeParser.cpp - CSKY Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/CSKYAttributeParser.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Support/Errc.h"

using நாமாகாஶ llvm;

மாறிலி CSKYAttributeParser::DisplayHandler
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
            CSKYAttrs::CSKY_FPU_எண்_MODULE,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_FPU_HARDFP,
            &CSKYAttributeParser::fpuHardFP,
        }};

Error CSKYAttributeParser::handler(uint64_t tag, தருக்க &handled) {
  handled = பொய்;
  க்கு (மாறிலி auto &AH : displayRoutines) {
    எனில் (uint64_t(AH.attribute) == tag) {
      எனில் (Error e = (இது->*AH.routine)(tag))
        திரும்பு e;
      handled = உண்மை;
      நிறுத்து;
    }
  }

  திரும்பு Error::success();
}

Error CSKYAttributeParser::dspVersion(unsigned tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Error", "DSP Extension", "DSP 2.0"};
  திரும்பு parseStringAttribute("Tag_CSKY_DSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::vdspVersion(unsigned tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  திரும்பு parseStringAttribute("Tag_CSKY_VDSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuVersion(unsigned tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  திரும்பு parseStringAttribute("Tag_CSKY_FPU_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuABI(unsigned tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Error", "Soft", "SoftFP", "Hard"};
  திரும்பு parseStringAttribute("Tag_CSKY_FPU_ABI", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuRounding(unsigned tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"None", "Needed"};
  திரும்பு parseStringAttribute("Tag_CSKY_FPU_ROUNDING", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuDenormal(unsigned tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"None", "Needed"};
  திரும்பு parseStringAttribute("Tag_CSKY_FPU_DENORMAL", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuException(unsigned tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"None", "Needed"};
  திரும்பு parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuHardFP(unsigned tag) {
  uint64_t மதிப்பு_2 = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  எனில் (மதிப்பு_2 & 0x1) {
    description += LS;
    description += "Half";
  }
  எனில் ((மதிப்பு_2 >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  எனில் ((மதிப்பு_2 >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  எனில் (description.empty()) {
    printAttribute(tag, மதிப்பு_2, "");
    திரும்பு createStringError(errc::invalid_argument,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(மதிப்பு_2));
  }

  printAttribute(tag, மதிப்பு_2, description);
  திரும்பு Error::success();
}
