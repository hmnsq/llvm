//===-- CSKYAttributeParser.cpp - CSKY Attribute Parser -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/CSKYAttributeParser.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Support/Errc.h"

उपयुज्य नामाकाश llvm;

ध्रुव CSKYAttributeParser::DisplayHandler
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
            CSKYAttrs::CSKY_DSP_संस्करणम्,
            &CSKYAttributeParser::dsp_संस्करणम्,
        },
        {
            CSKYAttrs::CSKY_VDSP_संस्करणम्,
            &CSKYAttributeParser::vdsp_संस्करणम्,
        },
        {
            CSKYAttrs::CSKY_FPU_संस्करणम्,
            &CSKYAttributeParser::fpu_संस्करणम्,
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
            CSKYAttrs::CSKY_FPU_सङ्ख्या_MODULE,
            &ELFCompactAttrParser::stringAttribute,
        },
        {
            CSKYAttrs::CSKY_FPU_HARDFP,
            &CSKYAttributeParser::fpuHardFP,
        }};

Error CSKYAttributeParser::handler(uint64_t चिह्नकम्_2, तार्किक &handled) {
  handled = असत्य;
  प्रति (ध्रुव स्वचालित &AH : displayRoutines) {
    यदि (uint64_t(AH.attribute) == चिह्नकम्_2) {
      यदि (Error e = (एतत्->*AH.routine)(चिह्नकम्_2))
        प्रत्यागम e;
      handled = सत्य;
      भङ्ग;
    }
  }

  प्रत्यागम Error::success();
}

Error CSKYAttributeParser::dsp_संस्करणम्(अचिह्नित चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Error", "DSP Extension", "DSP 2.0"};
  प्रत्यागम parseStringAttribute("Tag_CSKY_DSP_VERSION", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error CSKYAttributeParser::vdsp_संस्करणम्(अचिह्नित चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Error", "VDSP Version 1",
                                        "VDSP Version 2"};
  प्रत्यागम parseStringAttribute("Tag_CSKY_VDSP_VERSION", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error CSKYAttributeParser::fpu_संस्करणम्(अचिह्नित चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Error", "FPU Version 1",
                                        "FPU Version 2", "FPU Version 3"};
  प्रत्यागम parseStringAttribute("Tag_CSKY_FPU_VERSION", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error CSKYAttributeParser::fpuABI(अचिह्नित चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Error", "Soft", "SoftFP", "Hard"};
  प्रत्यागम parseStringAttribute("Tag_CSKY_FPU_ABI", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error CSKYAttributeParser::fpuRounding(अचिह्नित चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"None", "Needed"};
  प्रत्यागम parseStringAttribute("Tag_CSKY_FPU_ROUNDING", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error CSKYAttributeParser::fpuDenormal(अचिह्नित चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"None", "Needed"};
  प्रत्यागम parseStringAttribute("Tag_CSKY_FPU_DENORMAL", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error CSKYAttributeParser::fpuException(अचिह्नित चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"None", "Needed"};
  प्रत्यागम parseStringAttribute("Tag_CSKY_FPU_EXCEPTION", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error CSKYAttributeParser::fpuHardFP(अचिह्नित चिह्नकम्_2) {
  uint64_t मानम्_2 = de.getULEB128(cursor);
  ListSeparator LS(" ");

  std::string description;

  यदि (मानम्_2 & 0x1) {
    description += LS;
    description += "Half";
  }
  यदि ((मानम्_2 >> 1) & 0x1) {
    description += LS;
    description += "Single";
  }
  यदि ((मानम्_2 >> 2) & 0x1) {
    description += LS;
    description += "Double";
  }

  यदि (description.रिक्तानि()) {
    printAttribute(चिह्नकम्_2, मानम्_2, "");
    प्रत्यागम createStringError(errc::invalid_तर्कः,
                             "unknown Tag_CSKY_FPU_HARDFP value: " +
                                 Twine(मानम्_2));
  }

  printAttribute(चिह्नकम्_2, मानम्_2, description);
  प्रत्यागम Error::success();
}
