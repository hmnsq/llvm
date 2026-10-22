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

استفاده نطاق llvm;

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
            CSKYAttrs::CSKY_DSP_نسخه,
            &CSKYAttributeParser::dsp_نسخه,
        },
        {
            CSKYAttrs::CSKY_VDSP_نسخه,
            &CSKYAttributeParser::vdsp_نسخه,
        },
        {
            CSKYAttrs::CSKY_FPU_نسخه,
            &CSKYAttributeParser::fpu_نسخه,
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

Error CSKYAttributeParser::handler(uint64_t برچسب_2, منطقي &handled) {
  handled = خطأ;
  لكل (ثابت خودکار &AH : displayRoutines) {
    إذا (uint64_t(AH.attribute) == برچسب_2) {
      إذا (Error e = (هذا->*AH.routine)(برچسب_2))
        أرجع e;
      handled = درست;
      اكسر;
    }
  }

  أرجع Error::success();
}

Error CSKYAttributeParser::dsp_نسخه(بیعلامت برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Error", "DSP Extension", "DSP 2.0"};
  أرجع parseStringAttribute("Tag_CSKY_DSP_VERSION", برچسب_2, ArrayRef(رشتهها));
}

Error CSKYAttributeParser::vdsp_نسخه(بیعلامت برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  أرجع parseStringAttribute("Tag_CSKY_VDSP_VERSION", برچسب_2, ArrayRef(رشتهها));
}

Error CSKYAttributeParser::fpu_نسخه(بیعلامت برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_VERSION", برچسب_2, ArrayRef(رشتهها));
}

Error CSKYAttributeParser::fpuABI(بیعلامت برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Error", "Soft", "SoftFP", "Hard"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_ABI", برچسب_2, ArrayRef(رشتهها));
}

Error CSKYAttributeParser::fpuRounding(بیعلامت برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"None", "Needed"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_ROUNDING", برچسب_2, ArrayRef(رشتهها));
}

Error CSKYAttributeParser::fpuDenormal(بیعلامت برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"None", "Needed"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_DENORMAL", برچسب_2, ArrayRef(رشتهها));
}

Error CSKYAttributeParser::fpuException(بیعلامت برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"None", "Needed"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", برچسب_2, ArrayRef(رشتهها));
}

Error CSKYAttributeParser::fpuHardFP(بیعلامت برچسب_2) {
  uint64_t مقدار_2 = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  إذا (مقدار_2 & 0x1) {
    description += LS;
    description += "Half";
  }
  إذا ((مقدار_2 >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  إذا ((مقدار_2 >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  إذا (description.خالی()) {
    printAttribute(برچسب_2, مقدار_2, "");
    أرجع createStringError(errc::invalid_آرگومان,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(مقدار_2));
  }

  printAttribute(برچسب_2, مقدار_2, description);
  أرجع Error::success();
}
