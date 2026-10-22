//===-- CSKYAttributeParser.cpp - CSKY Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/CSKYAttributeParser.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/Support/Errc.h"

using નામાકાશ llvm;

અચલ CSKYAttributeParser::DisplayHandler
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
            CSKYAttrs::CSKY_FPU_સંખ્યા_MODULE,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_FPU_HARDFP,
            &CSKYAttributeParser::fpuHardFP,
        }};

Error CSKYAttributeParser::handler(uint64_t tag, તાર્કિક &handled) {
  handled = ખોટું;
  માટે (અચલ auto &AH : displayRoutines) {
    જો (uint64_t(AH.attribute) == tag) {
      જો (Error e = (આ->*AH.routine)(tag))
        પાછા e;
      handled = સાચું;
      તોડો;
    }
  }

  પાછા Error::success();
}

Error CSKYAttributeParser::dspVersion(unsigned tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Error", "DSP Extension", "DSP 2.0"};
  પાછા parseStringAttribute("Tag_CSKY_DSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::vdspVersion(unsigned tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  પાછા parseStringAttribute("Tag_CSKY_VDSP_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuVersion(unsigned tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  પાછા parseStringAttribute("Tag_CSKY_FPU_VERSION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuABI(unsigned tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Error", "Soft", "SoftFP", "Hard"};
  પાછા parseStringAttribute("Tag_CSKY_FPU_ABI", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuRounding(unsigned tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"None", "Needed"};
  પાછા parseStringAttribute("Tag_CSKY_FPU_ROUNDING", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuDenormal(unsigned tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"None", "Needed"};
  પાછા parseStringAttribute("Tag_CSKY_FPU_DENORMAL", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuException(unsigned tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"None", "Needed"};
  પાછા parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", tag, ArrayRef(strings));
}

Error CSKYAttributeParser::fpuHardFP(unsigned tag) {
  uint64_t કિંમત_2 = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  જો (કિંમત_2 & 0x1) {
    description += LS;
    description += "Half";
  }
  જો ((કિંમત_2 >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  જો ((કિંમત_2 >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  જો (description.empty()) {
    printAttribute(tag, કિંમત_2, "");
    પાછા createStringError(errc::invalid_argument,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(કિંમત_2));
  }

  printAttribute(tag, કિંમત_2, description);
  પાછા Error::success();
}
