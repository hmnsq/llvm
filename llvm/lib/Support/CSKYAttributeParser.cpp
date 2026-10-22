//===-- CSKYAttributeParser.cpp - CSKY Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/CSKYAttributeParser.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/Support/Errc.h"

using নামাকাশ llvm;

ধ্রুব CSKYAttributeParser::DisplayHandler
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
            CSKYAttrs::CSKY_FPU_সংখ্যা_MODULE,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_FPU_HARDFP,
            &CSKYAttributeParser::fpuHardFP,
        }};

Error CSKYAttributeParser::handler(uint64_t tag, তার্কিক &handled) {
  handled = অসত্য;
  প্রতি (ধ্রুব auto &AH : displayRoutines) {
    যদি (uint64_t(AH.attribute) == tag) {
      যদি (Error e = (এতত্->*AH.routine)(tag))
        প্রত্যাগম e;
      handled = সত্য;
      ভঙ্গ;
    }
  }

  প্রত্যাগম Error::success();
}

Error CSKYAttributeParser::dspVersion(unsigned tag) {
  স্থির ধ্রুব বর্ণ *ধ্রুব strings[] = {"Error", "DSP Extension", "DSP 2.0"};
  প্রত্যাগম parseStringAttribute("Tag_CSKY_DSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::vdspVersion(unsigned tag) {
  স্থির ধ্রুব বর্ণ *ধ্রুব strings[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  প্রত্যাগম parseStringAttribute("Tag_CSKY_VDSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuVersion(unsigned tag) {
  স্থির ধ্রুব বর্ণ *ধ্রুব strings[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  প্রত্যাগম parseStringAttribute("Tag_CSKY_FPU_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuABI(unsigned tag) {
  স্থির ধ্রুব বর্ণ *ধ্রুব strings[] = {"Error", "Soft", "SoftFP", "Hard"};
  প্রত্যাগম parseStringAttribute("Tag_CSKY_FPU_ABI", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuRounding(unsigned tag) {
  স্থির ধ্রুব বর্ণ *ধ্রুব strings[] = {"None", "Needed"};
  প্রত্যাগম parseStringAttribute("Tag_CSKY_FPU_ROUNDING", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuDenormal(unsigned tag) {
  স্থির ধ্রুব বর্ণ *ধ্রুব strings[] = {"None", "Needed"};
  প্রত্যাগম parseStringAttribute("Tag_CSKY_FPU_DENORMAL", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuException(unsigned tag) {
  স্থির ধ্রুব বর্ণ *ধ্রুব strings[] = {"None", "Needed"};
  প্রত্যাগম parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuHardFP(unsigned tag) {
  uint64_t মান_2 = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  যদি (মান_2 & 0x1) {
    description += LS;
    description += "Half";
  }
  যদি ((মান_2 >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  যদি ((মান_2 >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  যদি (description.empty()) {
    printAttribute(tag, মান_2, "");
    প্রত্যাগম createStringError(errc::invalid_argument,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(মান_2));
  }

  printAttribute(tag, মান_2, description);
  প্রত্যাগম Error::success();
}
