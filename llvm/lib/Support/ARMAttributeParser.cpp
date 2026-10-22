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

अचर ARMAttributeParser::DisplayHandler ARMAttributeParser::displayRoutines[] =
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
        ATTRIBUTE_HANDLER(ABI_FP_सङ्ख्या_model),
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
    sw->print_सङ्ख्या("Tag", tag);
    यदि (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  फर्काऊ Error::success();
}

स्थिर अचर वर्ण *अचर CPU_arch_strings[] = {"Pre-v4",
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
  फर्काऊ parseStringAttribute("CPU_arch", tag, ArrayRef(CPU_arch_strings));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType tag) {
  uint64_t मान_2 = de.getULEB128(cursor);

  StringRef profile;
  विकल्प (मान_2) {
  स्वभाव: profile = "Unknown"; रोक;
  पक्ष 'A': profile = "Application"; रोक;
  पक्ष 'R': profile = "Real-time"; रोक;
  पक्ष 'M': profile = "Microcontroller"; रोक;
  पक्ष 'S': profile = "Classic"; रोक;
  पक्ष 0: profile = "None"; रोक;
  }

  printAttribute(tag, मान_2, profile);
  फर्काऊ Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "Permitted"};
  फर्काऊ parseStringAttribute("ARM_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  फर्काऊ parseStringAttribute("THUMB_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_arch(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  फर्काऊ parseStringAttribute("FP_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::WMMX_arch(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  फर्काऊ parseStringAttribute("WMMX_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  फर्काऊ parseStringAttribute("Advanced_SIMD_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MVE_arch(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  फर्काऊ parseStringAttribute("MVE_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PCS_config(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  फर्काऊ parseStringAttribute("PCS_config", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"v6", "Static Base", "TLS", "Unused"};
  फर्काऊ parseStringAttribute("ABI_PCS_R9_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RW_data(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  फर्काऊ parseStringAttribute("ABI_PCS_RW_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RO_data(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  फर्काऊ parseStringAttribute("ABI_PCS_RO_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  फर्काऊ parseStringAttribute("ABI_PCS_GOT_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  फर्काऊ parseStringAttribute("ABI_PCS_wchar_t", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"IEEE-754", "Runtime"};
  फर्काऊ parseStringAttribute("ABI_FP_rounding", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Unsupported", "IEEE-754", "Sign Only"};
  फर्काऊ parseStringAttribute("ABI_FP_denormal", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "IEEE-754"};
  फर्काऊ parseStringAttribute("ABI_FP_exceptions", tag, ArrayRef(strings));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "IEEE-754"};
  फर्काऊ parseStringAttribute("ABI_FP_user_exceptions", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_सङ्ख्या_model(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  फर्काऊ parseStringAttribute("ABI_FP_number_model", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_align_needed(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t मान_2 = de.getULEB128(cursor);

  std::string description;
  यदि (मान_2 < std::size(strings))
    description = strings[मान_2];
  अन्यथा यदि (मान_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << मान_2) +
                  "-byte extended alignment";
  अन्यथा
    description = "Invalid";

  printAttribute(tag, मान_2, description);
  फर्काऊ Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType tag) {
  स्थिर अचर वर्ण *strings[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t मान_2 = de.getULEB128(cursor);

  std::string description;
  यदि (मान_2 < std::size(strings))
    description = std::string(strings[मान_2]);
  अन्यथा यदि (मान_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << मान_2) + std::string("-byte data alignment");
  अन्यथा
    description = "Invalid";

  printAttribute(tag, मान_2, description);
  फर्काऊ Error::success();
}

Error ARMAttributeParser::ABI_enum_size(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  फर्काऊ parseStringAttribute("ABI_enum_size", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  फर्काऊ parseStringAttribute("ABI_HardFP_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_VFP_args(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  फर्काऊ parseStringAttribute("ABI_VFP_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_WMMX_args(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"AAPCS", "iWMMX", "Custom"};
  फर्काऊ parseStringAttribute("ABI_WMMX_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  फर्काऊ parseStringAttribute("ABI_optimization_goals", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  फर्काऊ parseStringAttribute("ABI_FP_optimization_goals", tag,
                              ArrayRef(strings));
}

Error ARMAttributeParser::compatibility(AttrType tag) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  यदि (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_सङ्ख्या("Tag", tag);
    sw->start_पङ्क्ति() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                               /*hasTagPrefix=*/असत्य));
    विकल्प (integer) {
    पक्ष 0:
      sw->printString("Description", StringRef("No Specific Requirements"));
      रोक;
    पक्ष 1:
      sw->printString("Description", StringRef("AEABI Conformant"));
      रोक;
    स्वभाव:
      sw->printString("Description", StringRef("AEABI Non-Conformant"));
      रोक;
    }
  }
  फर्काऊ Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "v6-style"};
  फर्काऊ parseStringAttribute("CPU_unaligned_access", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_HP_extension(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"If Available", "Permitted"};
  फर्काऊ parseStringAttribute("FP_HP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  फर्काऊ parseStringAttribute("ABI_FP_16bit_format", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MPextension_use(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "Permitted"};
  फर्काऊ parseStringAttribute("MPextension_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DIV_use(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  फर्काऊ parseStringAttribute("DIV_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DSP_extension(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "Permitted"};
  फर्काऊ parseStringAttribute("DSP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::T2EE_use(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted", "Permitted"};
  फर्काऊ parseStringAttribute("T2EE_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Virtualization_use(AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  फर्काऊ parseStringAttribute("Virtualization_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  फर्काऊ parseStringAttribute("PAC_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  फर्काऊ parseStringAttribute("BTI_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Used", "Used"};
  फर्काऊ parseStringAttribute("PACRET_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType tag) {
  स्थिर अचर वर्ण *अचर strings[] = {"Not Used", "Used"};
  फर्काऊ parseStringAttribute("BTI_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::nodefaults(AttrType tag) {
  uint64_t मान_2 = de.getULEB128(cursor);
  printAttribute(tag, मान_2, "Unspecified Tags UNDEFINED");
  फर्काऊ Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType tag) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_मान;

  SmallString<8> Description;
  raw_svector_ostream DescStream(Description);

  uint64_t InitialOffset = cursor.tell();
  StringRef Raw_String_मान = de.getCStrRef(cursor);
  uint64_t FinalOffset = cursor.tell();
  cursor.seek(InitialOffset);
  uint64_t InnerTag = de.getULEB128(cursor);

  तार्किक ValidInnerTag =
      any_of(tagToStringMap, [InnerTag](अचर TagNameItem &Item) {
        फर्काऊ Item.attr == InnerTag;
      });

  यदि (!ValidInnerTag) {
    return_मान =
        createStringError(errc::argument_out_of_domain,
                          Twine(InnerTag) + " is not a valid tag number");
  } अन्यथा {
    विकल्प (InnerTag) {
    पक्ष ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_मान = de.getULEB128(cursor);
      स्वतः strings = ArrayRef(CPU_arch_strings);
      यदि (Inner_मान >= strings.size()) {
        return_मान = createStringError(
            errc::argument_out_of_domain,
            Twine(Inner_मान) + " is not a valid " +
                ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
                " value");
      } अन्यथा {
        DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                   << " = " << Inner_मान;
        यदि (strings[Inner_मान])
          DescStream << " (" << strings[Inner_मान] << ')';
      }
      रोक;
    }
    पक्ष ARMBuildAttrs::also_compatible_with:
      return_मान = createStringError(
          errc::invalid_argument,
          ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
              " cannot be recursively defined");
      रोक;
    पक्ष ARMBuildAttrs::CPU_raw_name:
    पक्ष ARMBuildAttrs::CPU_name:
    पक्ष ARMBuildAttrs::compatibility:
    पक्ष ARMBuildAttrs::conformance: {
      StringRef Inner_मान = de.getCStrRef(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_मान;
      रोक;
    }
    स्वभाव: {
      uint64_t Inner_मान = de.getULEB128(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_मान;
    }
    }
  }

  setAttributeString(tag, Raw_String_मान);
  यदि (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_सङ्ख्या("Tag", tag);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap, असत्य));
    sw->printStringEscaped("Value", Raw_String_मान);
    यदि (!Description.empty()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(FinalOffset);

  फर्काऊ return_मान ? std::move(*return_मान) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t tag, तार्किक &handled) {
  handled = असत्य;
  लागि (अचर स्वतः &AH : displayRoutines) {
    यदि (uint64_t(AH.attribute) == tag) {
      यदि (Error e = (एतत्->*AH.routine)(static_cast<AttrType>(tag)))
        फर्काऊ e;
      handled = सत्य;
      रोक;
    }
  }

  फर्काऊ Error::success();
}
