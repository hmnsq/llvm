//===- ARMAttributeParser.cpp - ARM Attribute Information Printer ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/ARMAttributeParser.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/Support/ARMBuildAttributes.h"
#समावेश "llvm/Support/Errc.h"
#समावेश "llvm/Support/ScopedPrinter.h"
#समावेश <optional>

using namespace llvm;
using namespace llvm::ARMBuildAttrs;

#घोषणा ATTRIBUTE_HANDLER(attr)                                                \
  { ARMBuildAttrs::attr, &ARMAttributeParser::attr }

स्थिर ARMAttributeParser::DisplayHandler ARMAttributeParser::displayRoutines[] =
    {
        {ARMBuildAttrs::CPU_raw_name, &ARMAttributeParser::stringAttribute},
        {ARMBuildAttrs::CPU_name, &ARMAttributeParser::stringAttribute},
        ATTRIBUTE_HANDLER(CPU_arch),
        ATTRIBUTE_HANDLER(CPU_arch_profile),
        ATTRIBUTE_HANDLER(ARM_ISA_use),
        ATTRIBUTE_HANDLER(THUMB_ISA_use),
        ATTRIBUTE_HANDLER(FP_arch),
        ATTRIBUTE_HANDLER(WMMX_arch),
        ATTRIBUTE_HANDLER(Advanced_SIMD_arch),
        ATTRIBUTE_HANDLER(MVE_arch),
        ATTRIBUTE_HANDLER(PCS_config),
        ATTRIBUTE_HANDLER(ABI_PCS_R9_use),
        ATTRIBUTE_HANDLER(ABI_PCS_RW_तथ्य),
        ATTRIBUTE_HANDLER(ABI_PCS_RO_तथ्य),
        ATTRIBUTE_HANDLER(ABI_PCS_GOT_use),
        ATTRIBUTE_HANDLER(ABI_PCS_wchar_t),
        ATTRIBUTE_HANDLER(ABI_FP_rounding),
        ATTRIBUTE_HANDLER(ABI_FP_denormal),
        ATTRIBUTE_HANDLER(ABI_FP_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_user_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_संख्या_model),
        ATTRIBUTE_HANDLER(ABI_align_needed),
        ATTRIBUTE_HANDLER(ABI_align_preserved),
        ATTRIBUTE_HANDLER(ABI_enum_आकार),
        ATTRIBUTE_HANDLER(ABI_HardFP_use),
        ATTRIBUTE_HANDLER(ABI_VFP_args),
        ATTRIBUTE_HANDLER(ABI_WMMX_args),
        ATTRIBUTE_HANDLER(ABI_optimization_goals),
        ATTRIBUTE_HANDLER(ABI_FP_optimization_goals),
        ATTRIBUTE_HANDLER(compatibility),
        ATTRIBUTE_HANDLER(CPU_unaligned_access),
        ATTRIBUTE_HANDLER(FP_HP_extension),
        ATTRIBUTE_HANDLER(ABI_FP_16bit_format),
        ATTRIBUTE_HANDLER(MPextension_use),
        ATTRIBUTE_HANDLER(DIV_use),
        ATTRIBUTE_HANDLER(DSP_extension),
        ATTRIBUTE_HANDLER(T2EE_use),
        ATTRIBUTE_HANDLER(Virtualization_use),
        ATTRIBUTE_HANDLER(PAC_extension),
        ATTRIBUTE_HANDLER(BTI_extension),
        ATTRIBUTE_HANDLER(PACRET_use),
        ATTRIBUTE_HANDLER(BTI_use),
        ATTRIBUTE_HANDLER(nodefaults),
        ATTRIBUTE_HANDLER(also_compatible_with),
};

#अघोषित ATTRIBUTE_HANDLER

Error ARMAttributeParser::stringAttribute(AttrType tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/false);
  StringRef desc = de.getCStrRef(cursor);

  अगर (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    अगर (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  वापस Error::success();
}

अटल स्थिर अक्षर *स्थिर CPU_arch_strings[] = {"Pre-v4",
                                               "ARM v4",
                                               "ARM v4T",
                                               "ARM v5T",
                                               "ARM v5TE",
                                               "ARM v5TEJ",
                                               "ARM v6",
                                               "ARM v6KZ",
                                               "ARM v6T2",
                                               "ARM v6K",
                                               "ARM v7",
                                               "ARM v6-M",
                                               "ARM v6S-M",
                                               "ARM v7E-M",
                                               "ARM v8-A",
                                               "ARM v8-R",
                                               "ARM v8-M Baseline",
                                               "ARM v8-M Mainline",
                                               nullptr,
                                               nullptr,
                                               nullptr,
                                               "ARM v8.1-M Mainline",
                                               "ARM v9-A"};

Error ARMAttributeParser::CPU_arch(AttrType tag) {
  वापस parseStringAttribute("CPU_arch", tag, ArrayRef(CPU_arch_strings));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType tag) {
  uint64_t मूल्य_2 = de.getULEB128(cursor);

  StringRef profile;
  चयन (मूल्य_2) {
  शेष: profile = "Unknown"; अवरोध;
  हाल 'A': profile = "Application"; अवरोध;
  हाल 'R': profile = "Real-time"; अवरोध;
  हाल 'M': profile = "Microcontroller"; अवरोध;
  हाल 'S': profile = "Classic"; अवरोध;
  हाल 0: profile = "None"; अवरोध;
  }

  printAttribute(tag, मूल्य_2, profile);
  वापस Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "Permitted"};
  वापस parseStringAttribute("ARM_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  वापस parseStringAttribute("THUMB_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_arch(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  वापस parseStringAttribute("FP_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::WMMX_arch(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  वापस parseStringAttribute("WMMX_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  वापस parseStringAttribute("Advanced_SIMD_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MVE_arch(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  वापस parseStringAttribute("MVE_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PCS_config(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  वापस parseStringAttribute("PCS_config", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"v6", "Static Base", "TLS", "Unused"};
  वापस parseStringAttribute("ABI_PCS_R9_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RW_तथ्य(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  वापस parseStringAttribute("ABI_PCS_RW_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RO_तथ्य(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  वापस parseStringAttribute("ABI_PCS_RO_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  वापस parseStringAttribute("ABI_PCS_GOT_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  वापस parseStringAttribute("ABI_PCS_wchar_t", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"IEEE-754", "Runtime"};
  वापस parseStringAttribute("ABI_FP_rounding", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Unsupported", "IEEE-754", "Sign Only"};
  वापस parseStringAttribute("ABI_FP_denormal", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "IEEE-754"};
  वापस parseStringAttribute("ABI_FP_exceptions", tag, ArrayRef(strings));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "IEEE-754"};
  वापस parseStringAttribute("ABI_FP_user_exceptions", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_संख्या_model(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  वापस parseStringAttribute("ABI_FP_number_model", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_align_needed(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t मूल्य_2 = de.getULEB128(cursor);

  std::string description;
  अगर (मूल्य_2 < std::आकार_2(strings))
    description = strings[मूल्य_2];
  अन्यथा अगर (मूल्य_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << मूल्य_2) +
                  "-byte extended alignment";
  अन्यथा
    description = "Invalid";

  printAttribute(tag, मूल्य_2, description);
  वापस Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType tag) {
  अटल स्थिर अक्षर *strings[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t मूल्य_2 = de.getULEB128(cursor);

  std::string description;
  अगर (मूल्य_2 < std::आकार_2(strings))
    description = std::string(strings[मूल्य_2]);
  अन्यथा अगर (मूल्य_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << मूल्य_2) + std::string("-byte data alignment");
  अन्यथा
    description = "Invalid";

  printAttribute(tag, मूल्य_2, description);
  वापस Error::success();
}

Error ARMAttributeParser::ABI_enum_आकार(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  वापस parseStringAttribute("ABI_enum_size", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  वापस parseStringAttribute("ABI_HardFP_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_VFP_args(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  वापस parseStringAttribute("ABI_VFP_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_WMMX_args(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"AAPCS", "iWMMX", "Custom"};
  वापस parseStringAttribute("ABI_WMMX_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  वापस parseStringAttribute("ABI_optimization_goals", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  वापस parseStringAttribute("ABI_FP_optimization_goals", tag,
                              ArrayRef(strings));
}

Error ARMAttributeParser::compatibility(AttrType tag) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  अगर (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    sw->start_पंक्ति() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                               /*hasTagPrefix=*/false));
    चयन (integer) {
    हाल 0:
      sw->printString("Description", StringRef("No Specific Requirements"));
      अवरोध;
    हाल 1:
      sw->printString("Description", StringRef("AEABI Conformant"));
      अवरोध;
    शेष:
      sw->printString("Description", StringRef("AEABI Non-Conformant"));
      अवरोध;
    }
  }
  वापस Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "v6-style"};
  वापस parseStringAttribute("CPU_unaligned_access", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_HP_extension(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"If Available", "Permitted"};
  वापस parseStringAttribute("FP_HP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  वापस parseStringAttribute("ABI_FP_16bit_format", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MPextension_use(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "Permitted"};
  वापस parseStringAttribute("MPextension_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DIV_use(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  वापस parseStringAttribute("DIV_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DSP_extension(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "Permitted"};
  वापस parseStringAttribute("DSP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::T2EE_use(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted", "Permitted"};
  वापस parseStringAttribute("T2EE_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Virtualization_use(AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  वापस parseStringAttribute("Virtualization_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  वापस parseStringAttribute("PAC_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  वापस parseStringAttribute("BTI_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Used", "Used"};
  वापस parseStringAttribute("PACRET_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType tag) {
  अटल स्थिर अक्षर *स्थिर strings[] = {"Not Used", "Used"};
  वापस parseStringAttribute("BTI_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::nodefaults(AttrType tag) {
  uint64_t मूल्य_2 = de.getULEB128(cursor);
  printAttribute(tag, मूल्य_2, "Unspecified Tags UNDEFINED");
  वापस Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType tag) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_मूल्य;

  SmallString<8> Description;
  raw_svector_ostream DescStream(Description);

  uint64_t InitialOffset = cursor.tell();
  StringRef Raw_String_मूल्य = de.getCStrRef(cursor);
  uint64_t FinalOffset = cursor.tell();
  cursor.seek(InitialOffset);
  uint64_t InnerTag = de.getULEB128(cursor);

  bool ValidInnerTag =
      any_of(tagToStringMap, [InnerTag](स्थिर TagNameItem &Item) {
        वापस Item.attr == InnerTag;
      });

  अगर (!ValidInnerTag) {
    return_मूल्य =
        createStringError(errc::argument_out_of_domain,
                          Twine(InnerTag) + " is not a valid tag number");
  } अन्यथा {
    चयन (InnerTag) {
    हाल ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_मूल्य = de.getULEB128(cursor);
      स्वतः strings = ArrayRef(CPU_arch_strings);
      अगर (Inner_मूल्य >= strings.आकार_2()) {
        return_मूल्य = createStringError(
            errc::argument_out_of_domain,
            Twine(Inner_मूल्य) + " is not a valid " +
                ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
                " value");
      } अन्यथा {
        DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                   << " = " << Inner_मूल्य;
        अगर (strings[Inner_मूल्य])
          DescStream << " (" << strings[Inner_मूल्य] << ')';
      }
      अवरोध;
    }
    हाल ARMBuildAttrs::also_compatible_with:
      return_मूल्य = createStringError(
          errc::invalid_argument,
          ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
              " cannot be recursively defined");
      अवरोध;
    हाल ARMBuildAttrs::CPU_raw_name:
    हाल ARMBuildAttrs::CPU_name:
    हाल ARMBuildAttrs::compatibility:
    हाल ARMBuildAttrs::conformance: {
      StringRef Inner_मूल्य = de.getCStrRef(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_मूल्य;
      अवरोध;
    }
    शेष: {
      uint64_t Inner_मूल्य = de.getULEB128(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_मूल्य;
    }
    }
  }

  setAttributeString(tag, Raw_String_मूल्य);
  अगर (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap, false));
    sw->printStringEscaped("Value", Raw_String_मूल्य);
    अगर (!Description.empty()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(FinalOffset);

  वापस return_मूल्य ? std::move(*return_मूल्य) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t tag, bool &handled) {
  handled = false;
  क्रम (स्थिर स्वतः &AH : displayRoutines) {
    अगर (uint64_t(AH.attribute) == tag) {
      अगर (Error e = (यह->*AH.routine)(static_cast<AttrType>(tag)))
        वापस e;
      handled = true;
      अवरोध;
    }
  }

  वापस Error::success();
}
