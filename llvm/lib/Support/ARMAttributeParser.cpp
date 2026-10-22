//===- ARMAttributeParser.cpp - ARM Attribute Information Printer ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/ARMAttributeParser.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Support/ARMBuildAttributes.h"
#अन्तर्भाव "llvm/Support/Errc.h"
#अन्तर्भाव "llvm/Support/ScopedPrinter.h"
#अन्तर्भाव <optional>

using नामाकाश llvm;
using नामाकाश llvm::ARMBuildAttrs;

#define ATTRIBUTE_HANDLER(attr)                                                \
  { ARMBuildAttrs::attr, &ARMAttributeParser::attr }

ध्रुव ARMAttributeParser::DisplayHandler ARMAttributeParser::displayRoutines[] =
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
        ATTRIBUTE_HANDLER(ABI_PCS_RW_data),
        ATTRIBUTE_HANDLER(ABI_PCS_RO_data),
        ATTRIBUTE_HANDLER(ABI_PCS_GOT_use),
        ATTRIBUTE_HANDLER(ABI_PCS_wchar_t),
        ATTRIBUTE_HANDLER(ABI_FP_rounding),
        ATTRIBUTE_HANDLER(ABI_FP_denormal),
        ATTRIBUTE_HANDLER(ABI_FP_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_user_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_संख्या_model),
        ATTRIBUTE_HANDLER(ABI_align_needed),
        ATTRIBUTE_HANDLER(ABI_align_preserved),
        ATTRIBUTE_HANDLER(ABI_enum_size),
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

#undef ATTRIBUTE_HANDLER

Error ARMAttributeParser::stringAttribute(AttrType tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/असत्य);
  StringRef desc = de.getCStrRef(cursor);

  यदि (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    यदि (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  प्रत्यागम Error::success();
}

स्थिर ध्रुव वर्ण *ध्रुव CPU_arch_strings[] = {"Pre-v4",
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
  प्रत्यागम parseStringAttribute("CPU_arch", tag, ArrayRef(CPU_arch_strings));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType tag) {
  uint64_t मूल्य_2 = de.getULEB128(cursor);

  StringRef profile;
  विकल्प (मूल्य_2) {
  स्वभाव: profile = "Unknown"; भङ्ग;
  पक्ष 'A': profile = "Application"; भङ्ग;
  पक्ष 'R': profile = "Real-time"; भङ्ग;
  पक्ष 'M': profile = "Microcontroller"; भङ्ग;
  पक्ष 'S': profile = "Classic"; भङ्ग;
  पक्ष 0: profile = "None"; भङ्ग;
  }

  printAttribute(tag, मूल्य_2, profile);
  प्रत्यागम Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "Permitted"};
  प्रत्यागम parseStringAttribute("ARM_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  प्रत्यागम parseStringAttribute("THUMB_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_arch(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  प्रत्यागम parseStringAttribute("FP_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::WMMX_arch(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  प्रत्यागम parseStringAttribute("WMMX_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  प्रत्यागम parseStringAttribute("Advanced_SIMD_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MVE_arch(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  प्रत्यागम parseStringAttribute("MVE_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PCS_config(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  प्रत्यागम parseStringAttribute("PCS_config", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"v6", "Static Base", "TLS", "Unused"};
  प्रत्यागम parseStringAttribute("ABI_PCS_R9_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RW_data(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  प्रत्यागम parseStringAttribute("ABI_PCS_RW_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RO_data(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  प्रत्यागम parseStringAttribute("ABI_PCS_RO_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  प्रत्यागम parseStringAttribute("ABI_PCS_GOT_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  प्रत्यागम parseStringAttribute("ABI_PCS_wchar_t", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"IEEE-754", "Runtime"};
  प्रत्यागम parseStringAttribute("ABI_FP_rounding", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Unsupported", "IEEE-754", "Sign Only"};
  प्रत्यागम parseStringAttribute("ABI_FP_denormal", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "IEEE-754"};
  प्रत्यागम parseStringAttribute("ABI_FP_exceptions", tag, ArrayRef(strings));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "IEEE-754"};
  प्रत्यागम parseStringAttribute("ABI_FP_user_exceptions", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_संख्या_model(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  प्रत्यागम parseStringAttribute("ABI_FP_number_model", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_align_needed(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t मूल्य_2 = de.getULEB128(cursor);

  std::string description;
  यदि (मूल्य_2 < std::size(strings))
    description = strings[मूल्य_2];
  अन्यथा यदि (मूल्य_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << मूल्य_2) +
                  "-byte extended alignment";
  अन्यथा
    description = "Invalid";

  printAttribute(tag, मूल्य_2, description);
  प्रत्यागम Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType tag) {
  स्थिर ध्रुव वर्ण *strings[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t मूल्य_2 = de.getULEB128(cursor);

  std::string description;
  यदि (मूल्य_2 < std::size(strings))
    description = std::string(strings[मूल्य_2]);
  अन्यथा यदि (मूल्य_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << मूल्य_2) + std::string("-byte data alignment");
  अन्यथा
    description = "Invalid";

  printAttribute(tag, मूल्य_2, description);
  प्रत्यागम Error::success();
}

Error ARMAttributeParser::ABI_enum_size(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  प्रत्यागम parseStringAttribute("ABI_enum_size", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  प्रत्यागम parseStringAttribute("ABI_HardFP_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_VFP_args(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  प्रत्यागम parseStringAttribute("ABI_VFP_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_WMMX_args(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"AAPCS", "iWMMX", "Custom"};
  प्रत्यागम parseStringAttribute("ABI_WMMX_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  प्रत्यागम parseStringAttribute("ABI_optimization_goals", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  प्रत्यागम parseStringAttribute("ABI_FP_optimization_goals", tag,
                              ArrayRef(strings));
}

Error ARMAttributeParser::compatibility(AttrType tag) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  यदि (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    sw->start_पंक्ति() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                               /*hasTagPrefix=*/असत्य));
    विकल्प (integer) {
    पक्ष 0:
      sw->printString("Description", StringRef("No Specific Requirements"));
      भङ्ग;
    पक्ष 1:
      sw->printString("Description", StringRef("AEABI Conformant"));
      भङ्ग;
    स्वभाव:
      sw->printString("Description", StringRef("AEABI Non-Conformant"));
      भङ्ग;
    }
  }
  प्रत्यागम Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "v6-style"};
  प्रत्यागम parseStringAttribute("CPU_unaligned_access", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_HP_extension(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"If Available", "Permitted"};
  प्रत्यागम parseStringAttribute("FP_HP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  प्रत्यागम parseStringAttribute("ABI_FP_16bit_format", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MPextension_use(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "Permitted"};
  प्रत्यागम parseStringAttribute("MPextension_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DIV_use(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  प्रत्यागम parseStringAttribute("DIV_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DSP_extension(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "Permitted"};
  प्रत्यागम parseStringAttribute("DSP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::T2EE_use(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted", "Permitted"};
  प्रत्यागम parseStringAttribute("T2EE_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Virtualization_use(AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  प्रत्यागम parseStringAttribute("Virtualization_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  प्रत्यागम parseStringAttribute("PAC_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  प्रत्यागम parseStringAttribute("BTI_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Used", "Used"};
  प्रत्यागम parseStringAttribute("PACRET_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType tag) {
  स्थिर ध्रुव वर्ण *ध्रुव strings[] = {"Not Used", "Used"};
  प्रत्यागम parseStringAttribute("BTI_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::nodefaults(AttrType tag) {
  uint64_t मूल्य_2 = de.getULEB128(cursor);
  printAttribute(tag, मूल्य_2, "Unspecified Tags UNDEFINED");
  प्रत्यागम Error::success();
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

  तार्किक ValidInnerTag =
      any_of(tagToStringMap, [InnerTag](ध्रुव TagNameItem &Item) {
        प्रत्यागम Item.attr == InnerTag;
      });

  यदि (!ValidInnerTag) {
    return_मूल्य =
        createStringError(errc::argument_out_of_domain,
                          Twine(InnerTag) + " is not a valid tag number");
  } अन्यथा {
    विकल्प (InnerTag) {
    पक्ष ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_मूल्य = de.getULEB128(cursor);
      auto strings = ArrayRef(CPU_arch_strings);
      यदि (Inner_मूल्य >= strings.size()) {
        return_मूल्य = createStringError(
            errc::argument_out_of_domain,
            Twine(Inner_मूल्य) + " is not a valid " +
                ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
                " value");
      } अन्यथा {
        DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                   << " = " << Inner_मूल्य;
        यदि (strings[Inner_मूल्य])
          DescStream << " (" << strings[Inner_मूल्य] << ')';
      }
      भङ्ग;
    }
    पक्ष ARMBuildAttrs::also_compatible_with:
      return_मूल्य = createStringError(
          errc::invalid_argument,
          ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
              " cannot be recursively defined");
      भङ्ग;
    पक्ष ARMBuildAttrs::CPU_raw_name:
    पक्ष ARMBuildAttrs::CPU_name:
    पक्ष ARMBuildAttrs::compatibility:
    पक्ष ARMBuildAttrs::conformance: {
      StringRef Inner_मूल्य = de.getCStrRef(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_मूल्य;
      भङ्ग;
    }
    स्वभाव: {
      uint64_t Inner_मूल्य = de.getULEB128(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_मूल्य;
    }
    }
  }

  setAttributeString(tag, Raw_String_मूल्य);
  यदि (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap, असत्य));
    sw->printStringEscaped("Value", Raw_String_मूल्य);
    यदि (!Description.empty()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(FinalOffset);

  प्रत्यागम return_मूल्य ? std::move(*return_मूल्य) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t tag, तार्किक &handled) {
  handled = असत्य;
  प्रति (ध्रुव auto &AH : displayRoutines) {
    यदि (uint64_t(AH.attribute) == tag) {
      यदि (Error e = (एतत्->*AH.routine)(static_cast<AttrType>(tag)))
        प्रत्यागम e;
      handled = सत्य;
      भङ्ग;
    }
  }

  प्रत्यागम Error::success();
}
