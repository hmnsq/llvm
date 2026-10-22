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

کارول نطاق llvm;

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
            CSKYAttrs::CSKY_FPU_شمېره_MODULE,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_FPU_HARDFP,
            &CSKYAttributeParser::fpuHardFP,
        }};

Error CSKYAttributeParser::handler(uint64_t نښان_2, منطقي &handled) {
  handled = خطأ;
  لكل (ثابت خپلکاری &AH : displayRoutines) {
    إذا (uint64_t(AH.attribute) == نښان_2) {
      إذا (Error e = (هذا->*AH.routine)(نښان_2))
        أرجع e;
      handled = ریښتیا;
      اكسر;
    }
  }

  أرجع Error::success();
}

Error CSKYAttributeParser::dsp_نسخه(بینښه نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Error", "DSP Extension", "DSP 2.0"};
  أرجع parseStringAttribute("Tag_CSKY_DSP_VERSION", نښان_2, ArrayRef(مزي_ګانې));
}

Error CSKYAttributeParser::vdsp_نسخه(بینښه نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  أرجع parseStringAttribute("Tag_CSKY_VDSP_VERSION", نښان_2, ArrayRef(مزي_ګانې));
}

Error CSKYAttributeParser::fpu_نسخه(بینښه نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_VERSION", نښان_2, ArrayRef(مزي_ګانې));
}

Error CSKYAttributeParser::fpuABI(بینښه نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Error", "Soft", "SoftFP", "Hard"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_ABI", نښان_2, ArrayRef(مزي_ګانې));
}

Error CSKYAttributeParser::fpuRounding(بینښه نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"None", "Needed"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_ROUNDING", نښان_2, ArrayRef(مزي_ګانې));
}

Error CSKYAttributeParser::fpuDenormal(بینښه نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"None", "Needed"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_DENORMAL", نښان_2, ArrayRef(مزي_ګانې));
}

Error CSKYAttributeParser::fpuException(بینښه نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"None", "Needed"};
  أرجع parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", نښان_2, ArrayRef(مزي_ګانې));
}

Error CSKYAttributeParser::fpuHardFP(بینښه نښان_2) {
  uint64_t ارزښت_2 = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  إذا (ارزښت_2 & 0x1) {
    description += LS;
    description += "Half";
  }
  إذا ((ارزښت_2 >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  إذا ((ارزښت_2 >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  إذا (description.تشه()) {
    printAttribute(نښان_2, ارزښت_2, "");
    أرجع createStringError(errc::invalid_دلیل,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(ارزښت_2));
  }

  printAttribute(نښان_2, ارزښت_2, description);
  أرجع Error::success();
}
