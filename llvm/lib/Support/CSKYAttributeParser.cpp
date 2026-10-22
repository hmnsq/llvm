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

استعمال نطاق llvm;

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
            CSKYAttrs::CSKY_DSP_نسخو,
            &CSKYAttributeParser::dsp_نسخو,
        },
        {
            CSKYAttrs::CSKY_VDSP_نسخو,
            &CSKYAttributeParser::vdsp_نسخو,
        },
        {
            CSKYAttrs::CSKY_FPU_نسخو,
            &CSKYAttributeParser::fpu_نسخو,
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

Error CSKYAttributeParser::handler(uint64_t ٽيگ_2, منطقي &handled) {
  handled = خطأ;
  لكل (ثابت خودڪار &AH : displayRoutines) {
    إذا (uint64_t(AH.attribute) == ٽيگ_2) {
      إذا (Error e = (هذا->*AH.routine)(ٽيگ_2))
        أرجع e;
      handled = سچ;
      اكسر;
    }
  }

  أرجع Error::success();
}

Error CSKYAttributeParser::dsp_نسخو(بنانشان ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Error", "DSP Extension", "DSP 2.0"};
  أرجع parseStringAttribute("Tag_CSKY_DSP_VERSION", ٽيگ_2, ArrayRef(لڙيون));
}

Error CSKYAttributeParser::vdsp_نسخو(بنانشان ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  أرجع parseStringAttribute("Tag_CSKY_VDSP_VERSION", ٽيگ_2, ArrayRef(لڙيون));
}

Error CSKYAttributeParser::fpu_نسخو(بنانشان ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_VERSION", ٽيگ_2, ArrayRef(لڙيون));
}

Error CSKYAttributeParser::fpuABI(بنانشان ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Error", "Soft", "SoftFP", "Hard"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_ABI", ٽيگ_2, ArrayRef(لڙيون));
}

Error CSKYAttributeParser::fpuRounding(بنانشان ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"None", "Needed"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_ROUNDING", ٽيگ_2, ArrayRef(لڙيون));
}

Error CSKYAttributeParser::fpuDenormal(بنانشان ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"None", "Needed"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_DENORMAL", ٽيگ_2, ArrayRef(لڙيون));
}

Error CSKYAttributeParser::fpuException(بنانشان ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"None", "Needed"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", ٽيگ_2, ArrayRef(لڙيون));
}

Error CSKYAttributeParser::fpuHardFP(بنانشان ٽيگ_2) {
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

  إذا (description.خالي_جايون()) {
    printAttribute(ٽيگ_2, قدر_2, "");
    أرجع createStringError(errc::invalid_دليل,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(قدر_2));
  }

  printAttribute(ٽيگ_2, قدر_2, description);
  أرجع Error::success();
}
