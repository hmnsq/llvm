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

ورتوں نطاق llvm;

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

Error CSKYAttributeParser::handler(uint64_t ٹیگ_2, منطقي &handled) {
  handled = خطأ;
  لكل (ثابت خودکار &AH : displayRoutines) {
    إذا (uint64_t(AH.attribute) == ٹیگ_2) {
      إذا (Error e = (هذا->*AH.routine)(ٹیگ_2))
        أرجع e;
      handled = سچ;
      اكسر;
    }
  }

  أرجع Error::success();
}

Error CSKYAttributeParser::dsp_نسخہ(بنانشانی ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Error", "DSP Extension", "DSP 2.0"};
  أرجع parseStringAttribute("Tag_CSKY_DSP_VERSION", ٹیگ_2, ArrayRef(لڑیاں));
}

Error CSKYAttributeParser::vdsp_نسخہ(بنانشانی ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  أرجع parseStringAttribute("Tag_CSKY_VDSP_VERSION", ٹیگ_2, ArrayRef(لڑیاں));
}

Error CSKYAttributeParser::fpu_نسخہ(بنانشانی ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_VERSION", ٹیگ_2, ArrayRef(لڑیاں));
}

Error CSKYAttributeParser::fpuABI(بنانشانی ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Error", "Soft", "SoftFP", "Hard"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_ABI", ٹیگ_2, ArrayRef(لڑیاں));
}

Error CSKYAttributeParser::fpuRounding(بنانشانی ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"None", "Needed"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_ROUNDING", ٹیگ_2, ArrayRef(لڑیاں));
}

Error CSKYAttributeParser::fpuDenormal(بنانشانی ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"None", "Needed"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_DENORMAL", ٹیگ_2, ArrayRef(لڑیاں));
}

Error CSKYAttributeParser::fpuException(بنانشانی ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"None", "Needed"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", ٹیگ_2, ArrayRef(لڑیاں));
}

Error CSKYAttributeParser::fpuHardFP(بنانشانی ٹیگ_2) {
  uint64_t قدر_2 = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  إذا (قدر_2 & 0x1) {
    description += LS;
    description += "Half";
  }
  إذا ((قدر_2 >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  إذا ((قدر_2 >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  إذا (description.خالی_تھاواں()) {
    printAttribute(ٹیگ_2, قدر_2, "");
    أرجع createStringError(errc::invalid_دلیل,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(قدر_2));
  }

  printAttribute(ٹیگ_2, قدر_2, description);
  أرجع Error::success();
}
