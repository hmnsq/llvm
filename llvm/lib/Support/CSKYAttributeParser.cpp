//===-- CSKYAttributeParser.cpp - CSKY Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/CSKYAttributeParser.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Errc.h"

using namespace llvm;

const CSKYAttributeParser::DisplayHandler
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

Error CSKYAttributeParser::handler(uint64_t tag, bool &handled) {
  handled = false;
  ꦏꦁꦒꦺꦴ (const auto &AH : displayRoutines) {
    ꦪꦼꦤ꧀ (uint64_t(AH.attribute) == tag) {
      ꦪꦼꦤ꧀ (Error e = (this->*AH.routine)(tag))
        ꦮꦭꦶ e;
      handled = true;
      bha_nga;
    }
  }

  ꦮꦭꦶ Error::success();
}

Error CSKYAttributeParser::dspVersion(unsigned tag) {
  static const var_na *const strings[] = {"Error", "DSP Extension", "DSP 2.0"};
  ꦮꦭꦶ parseStringAttribute("Tag_CSKY_DSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::vdspVersion(unsigned tag) {
  static const var_na *const strings[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  ꦮꦭꦶ parseStringAttribute("Tag_CSKY_VDSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuVersion(unsigned tag) {
  static const var_na *const strings[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  ꦮꦭꦶ parseStringAttribute("Tag_CSKY_FPU_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuABI(unsigned tag) {
  static const var_na *const strings[] = {"Error", "Soft", "SoftFP", "Hard"};
  ꦮꦭꦶ parseStringAttribute("Tag_CSKY_FPU_ABI", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuRounding(unsigned tag) {
  static const var_na *const strings[] = {"None", "Needed"};
  ꦮꦭꦶ parseStringAttribute("Tag_CSKY_FPU_ROUNDING", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuDenormal(unsigned tag) {
  static const var_na *const strings[] = {"None", "Needed"};
  ꦮꦭꦶ parseStringAttribute("Tag_CSKY_FPU_DENORMAL", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuException(unsigned tag) {
  static const var_na *const strings[] = {"None", "Needed"};
  ꦮꦭꦶ parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuHardFP(unsigned tag) {
  uint64_t value = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  ꦪꦼꦤ꧀ (value & 0x1) {
    description += LS;
    description += "Half";
  }
  ꦪꦼꦤ꧀ ((value >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  ꦪꦼꦤ꧀ ((value >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  ꦪꦼꦤ꧀ (description.empty()) {
    printAttribute(tag, value, "");
    ꦮꦭꦶ createStringError(errc::invalid_argument,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(value));
  }

  printAttribute(tag, value, description);
  ꦮꦭꦶ Error::success();
}
