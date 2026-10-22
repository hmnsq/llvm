//===-- CSKYAttributeParser.cpp - CSKY Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/CSKYAttributeParser.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/Support/Errc.h"

using നാമാകാശ llvm;

സ്ഥിരം CSKYAttributeParser::DisplayHandler
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
            CSKYAttrs::CSKY_FPU_സംഖ്യ_MODULE,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_FPU_HARDFP,
            &CSKYAttributeParser::fpuHardFP,
        }};

Error CSKYAttributeParser::handler(uint64_t tag, തർക്കം &handled) {
  handled = അസത്യം;
  വേണ്ടി (സ്ഥിരം auto &AH : displayRoutines) {
    എങ്കിൽ (uint64_t(AH.attribute) == tag) {
      എങ്കിൽ (Error e = (ഇത്->*AH.routine)(tag))
        തിരികെ e;
      handled = സത്യം;
      നിർത്തുക;
    }
  }

  തിരികെ Error::success();
}

Error CSKYAttributeParser::dspVersion(unsigned tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Error", "DSP Extension", "DSP 2.0"};
  തിരികെ parseStringAttribute("Tag_CSKY_DSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::vdspVersion(unsigned tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  തിരികെ parseStringAttribute("Tag_CSKY_VDSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuVersion(unsigned tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  തിരികെ parseStringAttribute("Tag_CSKY_FPU_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuABI(unsigned tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Error", "Soft", "SoftFP", "Hard"};
  തിരികെ parseStringAttribute("Tag_CSKY_FPU_ABI", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuRounding(unsigned tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"None", "Needed"};
  തിരികെ parseStringAttribute("Tag_CSKY_FPU_ROUNDING", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuDenormal(unsigned tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"None", "Needed"};
  തിരികെ parseStringAttribute("Tag_CSKY_FPU_DENORMAL", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuException(unsigned tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"None", "Needed"};
  തിരികെ parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuHardFP(unsigned tag) {
  uint64_t മൂല്യം_2 = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  എങ്കിൽ (മൂല്യം_2 & 0x1) {
    description += LS;
    description += "Half";
  }
  എങ്കിൽ ((മൂല്യം_2 >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  എങ്കിൽ ((മൂല്യം_2 >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  എങ്കിൽ (description.empty()) {
    printAttribute(tag, മൂല്യം_2, "");
    തിരികെ createStringError(errc::invalid_argument,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(മൂല്യം_2));
  }

  printAttribute(tag, മൂല്യം_2, description);
  തിരികെ Error::success();
}
