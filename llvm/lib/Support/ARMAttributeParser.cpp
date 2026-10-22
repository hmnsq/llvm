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

स्थिरांक ARMAttributeParser::DisplayHandler ARMAttributeParser::displayRoutines[] =
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

  जर (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    जर (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  परत Error::success();
}

स्थिर स्थिरांक अक्षर *स्थिरांक CPU_arch_strings[] = {"Pre-v4",
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
  परत parseStringAttribute("CPU_arch", tag, ArrayRef(CPU_arch_strings));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType tag) {
  uint64_t मूल्य_2 = de.getULEB128(cursor);

  StringRef profile;
  निवड (मूल्य_2) {
  पूर्वनिर्धारित: profile = "Unknown"; थांबा;
  स्थिती 'A': profile = "Application"; थांबा;
  स्थिती 'R': profile = "Real-time"; थांबा;
  स्थिती 'M': profile = "Microcontroller"; थांबा;
  स्थिती 'S': profile = "Classic"; थांबा;
  स्थिती 0: profile = "None"; थांबा;
  }

  printAttribute(tag, मूल्य_2, profile);
  परत Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "Permitted"};
  परत parseStringAttribute("ARM_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  परत parseStringAttribute("THUMB_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_arch(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  परत parseStringAttribute("FP_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::WMMX_arch(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  परत parseStringAttribute("WMMX_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  परत parseStringAttribute("Advanced_SIMD_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MVE_arch(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  परत parseStringAttribute("MVE_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PCS_config(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  परत parseStringAttribute("PCS_config", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"v6", "Static Base", "TLS", "Unused"};
  परत parseStringAttribute("ABI_PCS_R9_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RW_data(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  परत parseStringAttribute("ABI_PCS_RW_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RO_data(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  परत parseStringAttribute("ABI_PCS_RO_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  परत parseStringAttribute("ABI_PCS_GOT_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  परत parseStringAttribute("ABI_PCS_wchar_t", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"IEEE-754", "Runtime"};
  परत parseStringAttribute("ABI_FP_rounding", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Unsupported", "IEEE-754", "Sign Only"};
  परत parseStringAttribute("ABI_FP_denormal", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "IEEE-754"};
  परत parseStringAttribute("ABI_FP_exceptions", tag, ArrayRef(strings));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "IEEE-754"};
  परत parseStringAttribute("ABI_FP_user_exceptions", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_संख्या_model(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  परत parseStringAttribute("ABI_FP_number_model", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_align_needed(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t मूल्य_2 = de.getULEB128(cursor);

  std::string description;
  जर (मूल्य_2 < std::size(strings))
    description = strings[मूल्य_2];
  अन्यथा जर (मूल्य_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << मूल्य_2) +
                  "-byte extended alignment";
  अन्यथा
    description = "Invalid";

  printAttribute(tag, मूल्य_2, description);
  परत Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType tag) {
  स्थिर स्थिरांक अक्षर *strings[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t मूल्य_2 = de.getULEB128(cursor);

  std::string description;
  जर (मूल्य_2 < std::size(strings))
    description = std::string(strings[मूल्य_2]);
  अन्यथा जर (मूल्य_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << मूल्य_2) + std::string("-byte data alignment");
  अन्यथा
    description = "Invalid";

  printAttribute(tag, मूल्य_2, description);
  परत Error::success();
}

Error ARMAttributeParser::ABI_enum_size(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  परत parseStringAttribute("ABI_enum_size", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  परत parseStringAttribute("ABI_HardFP_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_VFP_args(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  परत parseStringAttribute("ABI_VFP_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_WMMX_args(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"AAPCS", "iWMMX", "Custom"};
  परत parseStringAttribute("ABI_WMMX_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  परत parseStringAttribute("ABI_optimization_goals", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  परत parseStringAttribute("ABI_FP_optimization_goals", tag,
                              ArrayRef(strings));
}

Error ARMAttributeParser::compatibility(AttrType tag) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  जर (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    sw->start_ओळ() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                               /*hasTagPrefix=*/असत्य));
    निवड (integer) {
    स्थिती 0:
      sw->printString("Description", StringRef("No Specific Requirements"));
      थांबा;
    स्थिती 1:
      sw->printString("Description", StringRef("AEABI Conformant"));
      थांबा;
    पूर्वनिर्धारित:
      sw->printString("Description", StringRef("AEABI Non-Conformant"));
      थांबा;
    }
  }
  परत Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "v6-style"};
  परत parseStringAttribute("CPU_unaligned_access", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_HP_extension(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"If Available", "Permitted"};
  परत parseStringAttribute("FP_HP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  परत parseStringAttribute("ABI_FP_16bit_format", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MPextension_use(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "Permitted"};
  परत parseStringAttribute("MPextension_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DIV_use(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  परत parseStringAttribute("DIV_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DSP_extension(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "Permitted"};
  परत parseStringAttribute("DSP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::T2EE_use(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted", "Permitted"};
  परत parseStringAttribute("T2EE_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Virtualization_use(AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  परत parseStringAttribute("Virtualization_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  परत parseStringAttribute("PAC_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  परत parseStringAttribute("BTI_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Used", "Used"};
  परत parseStringAttribute("PACRET_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType tag) {
  स्थिर स्थिरांक अक्षर *स्थिरांक strings[] = {"Not Used", "Used"};
  परत parseStringAttribute("BTI_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::nodefaults(AttrType tag) {
  uint64_t मूल्य_2 = de.getULEB128(cursor);
  printAttribute(tag, मूल्य_2, "Unspecified Tags UNDEFINED");
  परत Error::success();
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
      any_of(tagToStringMap, [InnerTag](स्थिरांक TagNameItem &Item) {
        परत Item.attr == InnerTag;
      });

  जर (!ValidInnerTag) {
    return_मूल्य =
        createStringError(errc::argument_out_of_domain,
                          Twine(InnerTag) + " is not a valid tag number");
  } अन्यथा {
    निवड (InnerTag) {
    स्थिती ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_मूल्य = de.getULEB128(cursor);
      स्वतः strings = ArrayRef(CPU_arch_strings);
      जर (Inner_मूल्य >= strings.size()) {
        return_मूल्य = createStringError(
            errc::argument_out_of_domain,
            Twine(Inner_मूल्य) + " is not a valid " +
                ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
                " value");
      } अन्यथा {
        DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                   << " = " << Inner_मूल्य;
        जर (strings[Inner_मूल्य])
          DescStream << " (" << strings[Inner_मूल्य] << ')';
      }
      थांबा;
    }
    स्थिती ARMBuildAttrs::also_compatible_with:
      return_मूल्य = createStringError(
          errc::invalid_argument,
          ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
              " cannot be recursively defined");
      थांबा;
    स्थिती ARMBuildAttrs::CPU_raw_name:
    स्थिती ARMBuildAttrs::CPU_name:
    स्थिती ARMBuildAttrs::compatibility:
    स्थिती ARMBuildAttrs::conformance: {
      StringRef Inner_मूल्य = de.getCStrRef(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_मूल्य;
      थांबा;
    }
    पूर्वनिर्धारित: {
      uint64_t Inner_मूल्य = de.getULEB128(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_मूल्य;
    }
    }
  }

  setAttributeString(tag, Raw_String_मूल्य);
  जर (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap, असत्य));
    sw->printStringEscaped("Value", Raw_String_मूल्य);
    जर (!Description.empty()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(FinalOffset);

  परत return_मूल्य ? std::move(*return_मूल्य) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t tag, तार्किक &handled) {
  handled = असत्य;
  साठी (स्थिरांक स्वतः &AH : displayRoutines) {
    जर (uint64_t(AH.attribute) == tag) {
      जर (Error e = (हे->*AH.routine)(static_cast<AttrType>(tag)))
        परत e;
      handled = सत्य;
      थांबा;
    }
  }

  परत Error::success();
}
