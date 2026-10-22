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

استعمال نامخلا llvm;

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
            CSKYAttrs::CSKY_DSP_نسخہ,
            &CSKYAttributeParser::dsp_نسخہ,
        },
        {
            CSKYAttrs::CSKY_VDSP_نسخہ,
            &CSKYAttributeParser::vdsp_نسخہ,
        },
        {
            CSKYAttrs::CSKY_FPU_نسخہ,
            &CSKYAttributeParser::fpu_نسخہ,
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
            CSKYAttrs::CSKY_FPU_نمبر_MODULE,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_FPU_HARDFP,
            &CSKYAttributeParser::fpuHardFP,
        }};

Error CSKYAttributeParser::handler(uint64_t لیبل_2, منطقی &handled) {
  handled = جھوٹ;
  برائے (ثابت خودکار &AH : displayRoutines) {
    اگر (uint64_t(AH.attribute) == لیبل_2) {
      اگر (Error e = (یہ->*AH.routine)(لیبل_2))
        واپس e;
      handled = سچ;
      توڑو;
    }
  }

  واپس Error::success();
}

Error CSKYAttributeParser::dsp_نسخہ(غیرعلامتی لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Error", "DSP Extension", "DSP 2.0"};
  واپس parseStringAttribute("Tag_CSKY_DSP_VERSION", لیبل_2, ArrayRef(لڑیاں));
}

Error CSKYAttributeParser::vdsp_نسخہ(غیرعلامتی لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  واپس parseStringAttribute("Tag_CSKY_VDSP_VERSION", لیبل_2, ArrayRef(لڑیاں));
}

Error CSKYAttributeParser::fpu_نسخہ(غیرعلامتی لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  واپس parseStringAttribute("Tag_CSKY_FPU_VERSION", لیبل_2, ArrayRef(لڑیاں));
}

Error CSKYAttributeParser::fpuABI(غیرعلامتی لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Error", "Soft", "SoftFP", "Hard"};
  واپس parseStringAttribute("Tag_CSKY_FPU_ABI", لیبل_2, ArrayRef(لڑیاں));
}

Error CSKYAttributeParser::fpuRounding(غیرعلامتی لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"None", "Needed"};
  واپس parseStringAttribute("Tag_CSKY_FPU_ROUNDING", لیبل_2, ArrayRef(لڑیاں));
}

Error CSKYAttributeParser::fpuDenormal(غیرعلامتی لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"None", "Needed"};
  واپس parseStringAttribute("Tag_CSKY_FPU_DENORMAL", لیبل_2, ArrayRef(لڑیاں));
}

Error CSKYAttributeParser::fpuException(غیرعلامتی لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"None", "Needed"};
  واپس parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", لیبل_2, ArrayRef(لڑیاں));
}

Error CSKYAttributeParser::fpuHardFP(غیرعلامتی لیبل_2) {
  uint64_t قدر_2 = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  اگر (قدر_2 & 0x1) {
    description += LS;
    description += "Half";
  }
  اگر ((قدر_2 >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  اگر ((قدر_2 >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  اگر (description.خالی_خانے()) {
    printAttribute(لیبل_2, قدر_2, "");
    واپس createStringError(errc::invalid_دلیل,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(قدر_2));
  }

  printAttribute(لیبل_2, قدر_2, description);
  واپس Error::success();
}
