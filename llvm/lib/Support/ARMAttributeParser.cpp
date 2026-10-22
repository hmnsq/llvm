//===- ARMAttributeParser.cpp - ARM Attribute Information Printer ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/ARMAttributeParser.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/Support/ARMBuildAttributes.h"
#അന്തര്ഭാവ "llvm/Support/Errc.h"
#അന്തര്ഭാവ "llvm/Support/ScopedPrinter.h"
#അന്തര്ഭാവ <optional>

using നാമാകാശ llvm;
using നാമാകാശ llvm::ARMBuildAttrs;

#define ATTRIBUTE_HANDLER(attr)                                                \
  { ARMBuildAttrs::attr, &ARMAttributeParser::attr }

സ്ഥിരം ARMAttributeParser::DisplayHandler ARMAttributeParser::displayRoutines[] =
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
        ATTRIBUTE_HANDLER(ABI_FP_സംഖ്യ_model),
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
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/അസത്യം);
  StringRef desc = de.getCStrRef(cursor);

  എങ്കിൽ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_സംഖ്യ("Tag", tag);
    എങ്കിൽ (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  തിരികെ Error::success();
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം CPU_arch_strings[] = {"Pre-v4",
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
  തിരികെ parseStringAttribute("CPU_arch", tag, ArrayRef(CPU_arch_strings));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType tag) {
  uint64_t മൂല്യം_2 = de.getULEB128(cursor);

  StringRef profile;
  തിരഞ്ഞെടുപ്പ് (മൂല്യം_2) {
  സ്ഥിരസ്ഥിതി: profile = "Unknown"; നിർത്തുക;
  അവസ്ഥ 'A': profile = "Application"; നിർത്തുക;
  അവസ്ഥ 'R': profile = "Real-time"; നിർത്തുക;
  അവസ്ഥ 'M': profile = "Microcontroller"; നിർത്തുക;
  അവസ്ഥ 'S': profile = "Classic"; നിർത്തുക;
  അവസ്ഥ 0: profile = "None"; നിർത്തുക;
  }

  printAttribute(tag, മൂല്യം_2, profile);
  തിരികെ Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "Permitted"};
  തിരികെ parseStringAttribute("ARM_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  തിരികെ parseStringAttribute("THUMB_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_arch(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  തിരികെ parseStringAttribute("FP_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::WMMX_arch(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  തിരികെ parseStringAttribute("WMMX_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  തിരികെ parseStringAttribute("Advanced_SIMD_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MVE_arch(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  തിരികെ parseStringAttribute("MVE_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PCS_config(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  തിരികെ parseStringAttribute("PCS_config", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"v6", "Static Base", "TLS", "Unused"};
  തിരികെ parseStringAttribute("ABI_PCS_R9_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RW_data(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  തിരികെ parseStringAttribute("ABI_PCS_RW_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RO_data(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  തിരികെ parseStringAttribute("ABI_PCS_RO_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  തിരികെ parseStringAttribute("ABI_PCS_GOT_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  തിരികെ parseStringAttribute("ABI_PCS_wchar_t", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"IEEE-754", "Runtime"};
  തിരികെ parseStringAttribute("ABI_FP_rounding", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Unsupported", "IEEE-754", "Sign Only"};
  തിരികെ parseStringAttribute("ABI_FP_denormal", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "IEEE-754"};
  തിരികെ parseStringAttribute("ABI_FP_exceptions", tag, ArrayRef(strings));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "IEEE-754"};
  തിരികെ parseStringAttribute("ABI_FP_user_exceptions", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_സംഖ്യ_model(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  തിരികെ parseStringAttribute("ABI_FP_number_model", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_align_needed(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t മൂല്യം_2 = de.getULEB128(cursor);

  std::string description;
  എങ്കിൽ (മൂല്യം_2 < std::size(strings))
    description = strings[മൂല്യം_2];
  അല്ലെങ്കിൽ എങ്കിൽ (മൂല്യം_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << മൂല്യം_2) +
                  "-byte extended alignment";
  അല്ലെങ്കിൽ
    description = "Invalid";

  printAttribute(tag, മൂല്യം_2, description);
  തിരികെ Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *strings[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t മൂല്യം_2 = de.getULEB128(cursor);

  std::string description;
  എങ്കിൽ (മൂല്യം_2 < std::size(strings))
    description = std::string(strings[മൂല്യം_2]);
  അല്ലെങ്കിൽ എങ്കിൽ (മൂല്യം_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << മൂല്യം_2) + std::string("-byte data alignment");
  അല്ലെങ്കിൽ
    description = "Invalid";

  printAttribute(tag, മൂല്യം_2, description);
  തിരികെ Error::success();
}

Error ARMAttributeParser::ABI_enum_size(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  തിരികെ parseStringAttribute("ABI_enum_size", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  തിരികെ parseStringAttribute("ABI_HardFP_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_VFP_args(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  തിരികെ parseStringAttribute("ABI_VFP_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_WMMX_args(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"AAPCS", "iWMMX", "Custom"};
  തിരികെ parseStringAttribute("ABI_WMMX_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  തിരികെ parseStringAttribute("ABI_optimization_goals", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  തിരികെ parseStringAttribute("ABI_FP_optimization_goals", tag,
                              ArrayRef(strings));
}

Error ARMAttributeParser::compatibility(AttrType tag) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  എങ്കിൽ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_സംഖ്യ("Tag", tag);
    sw->start_വരി() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                               /*hasTagPrefix=*/അസത്യം));
    തിരഞ്ഞെടുപ്പ് (integer) {
    അവസ്ഥ 0:
      sw->printString("Description", StringRef("No Specific Requirements"));
      നിർത്തുക;
    അവസ്ഥ 1:
      sw->printString("Description", StringRef("AEABI Conformant"));
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      sw->printString("Description", StringRef("AEABI Non-Conformant"));
      നിർത്തുക;
    }
  }
  തിരികെ Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "v6-style"};
  തിരികെ parseStringAttribute("CPU_unaligned_access", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_HP_extension(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"If Available", "Permitted"};
  തിരികെ parseStringAttribute("FP_HP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  തിരികെ parseStringAttribute("ABI_FP_16bit_format", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MPextension_use(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "Permitted"};
  തിരികെ parseStringAttribute("MPextension_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DIV_use(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  തിരികെ parseStringAttribute("DIV_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DSP_extension(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "Permitted"};
  തിരികെ parseStringAttribute("DSP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::T2EE_use(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted", "Permitted"};
  തിരികെ parseStringAttribute("T2EE_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Virtualization_use(AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  തിരികെ parseStringAttribute("Virtualization_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  തിരികെ parseStringAttribute("PAC_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  തിരികെ parseStringAttribute("BTI_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Used", "Used"};
  തിരികെ parseStringAttribute("PACRET_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType tag) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം strings[] = {"Not Used", "Used"};
  തിരികെ parseStringAttribute("BTI_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::nodefaults(AttrType tag) {
  uint64_t മൂല്യം_2 = de.getULEB128(cursor);
  printAttribute(tag, മൂല്യം_2, "Unspecified Tags UNDEFINED");
  തിരികെ Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType tag) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_മൂല്യം;

  SmallString<8> Description;
  raw_svector_ostream DescStream(Description);

  uint64_t InitialOffset = cursor.tell();
  StringRef Raw_String_മൂല്യം = de.getCStrRef(cursor);
  uint64_t FinalOffset = cursor.tell();
  cursor.seek(InitialOffset);
  uint64_t InnerTag = de.getULEB128(cursor);

  തർക്കം ValidInnerTag =
      any_of(tagToStringMap, [InnerTag](സ്ഥിരം TagNameItem &Item) {
        തിരികെ Item.attr == InnerTag;
      });

  എങ്കിൽ (!ValidInnerTag) {
    return_മൂല്യം =
        createStringError(errc::argument_out_of_domain,
                          Twine(InnerTag) + " is not a valid tag number");
  } അല്ലെങ്കിൽ {
    തിരഞ്ഞെടുപ്പ് (InnerTag) {
    അവസ്ഥ ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_മൂല്യം = de.getULEB128(cursor);
      auto strings = ArrayRef(CPU_arch_strings);
      എങ്കിൽ (Inner_മൂല്യം >= strings.size()) {
        return_മൂല്യം = createStringError(
            errc::argument_out_of_domain,
            Twine(Inner_മൂല്യം) + " is not a valid " +
                ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
                " value");
      } അല്ലെങ്കിൽ {
        DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                   << " = " << Inner_മൂല്യം;
        എങ്കിൽ (strings[Inner_മൂല്യം])
          DescStream << " (" << strings[Inner_മൂല്യം] << ')';
      }
      നിർത്തുക;
    }
    അവസ്ഥ ARMBuildAttrs::also_compatible_with:
      return_മൂല്യം = createStringError(
          errc::invalid_argument,
          ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
              " cannot be recursively defined");
      നിർത്തുക;
    അവസ്ഥ ARMBuildAttrs::CPU_raw_name:
    അവസ്ഥ ARMBuildAttrs::CPU_name:
    അവസ്ഥ ARMBuildAttrs::compatibility:
    അവസ്ഥ ARMBuildAttrs::conformance: {
      StringRef Inner_മൂല്യം = de.getCStrRef(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_മൂല്യം;
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      uint64_t Inner_മൂല്യം = de.getULEB128(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_മൂല്യം;
    }
    }
  }

  setAttributeString(tag, Raw_String_മൂല്യം);
  എങ്കിൽ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_സംഖ്യ("Tag", tag);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap, അസത്യം));
    sw->printStringEscaped("Value", Raw_String_മൂല്യം);
    എങ്കിൽ (!Description.empty()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(FinalOffset);

  തിരികെ return_മൂല്യം ? std::move(*return_മൂല്യം) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t tag, തർക്കം &handled) {
  handled = അസത്യം;
  വേണ്ടി (സ്ഥിരം auto &AH : displayRoutines) {
    എങ്കിൽ (uint64_t(AH.attribute) == tag) {
      എങ്കിൽ (Error e = (ഇത്->*AH.routine)(static_cast<AttrType>(tag)))
        തിരികെ e;
      handled = സത്യം;
      നിർത്തുക;
    }
  }

  തിരികെ Error::success();
}
