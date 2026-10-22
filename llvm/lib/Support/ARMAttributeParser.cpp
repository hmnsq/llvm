//===- ARMAttributeParser.cpp - ARM Attribute Information Printer ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/ARMAttributeParser.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Support/ARMBuildAttributes.h"
#அந்தர்பாவ "llvm/Support/Errc.h"
#அந்தர்பாவ "llvm/Support/ScopedPrinter.h"
#அந்தர்பாவ <optional>

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::ARMBuildAttrs;

#define ATTRIBUTE_HANDLER(attr)                                                \
  { ARMBuildAttrs::attr, &ARMAttributeParser::attr }

மாறிலி ARMAttributeParser::DisplayHandler ARMAttributeParser::displayRoutines[] =
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
        ATTRIBUTE_HANDLER(ABI_FP_எண்_model),
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
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/பொய்);
  StringRef desc = de.getCStrRef(cursor);

  எனில் (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_எண்("Tag", tag);
    எனில் (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  திரும்பு Error::success();
}

நிலையான மாறிலி எழுத்து *மாறிலி CPU_arch_strings[] = {"Pre-v4",
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
  திரும்பு parseStringAttribute("CPU_arch", tag, ArrayRef(CPU_arch_strings));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType tag) {
  uint64_t மதிப்பு_2 = de.getULEB128(cursor);

  StringRef profile;
  தேர்வு (மதிப்பு_2) {
  இயல்பு: profile = "Unknown"; நிறுத்து;
  நிலை 'A': profile = "Application"; நிறுத்து;
  நிலை 'R': profile = "Real-time"; நிறுத்து;
  நிலை 'M': profile = "Microcontroller"; நிறுத்து;
  நிலை 'S': profile = "Classic"; நிறுத்து;
  நிலை 0: profile = "None"; நிறுத்து;
  }

  printAttribute(tag, மதிப்பு_2, profile);
  திரும்பு Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "Permitted"};
  திரும்பு parseStringAttribute("ARM_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  திரும்பு parseStringAttribute("THUMB_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_arch(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  திரும்பு parseStringAttribute("FP_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::WMMX_arch(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  திரும்பு parseStringAttribute("WMMX_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  திரும்பு parseStringAttribute("Advanced_SIMD_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MVE_arch(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  திரும்பு parseStringAttribute("MVE_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PCS_config(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  திரும்பு parseStringAttribute("PCS_config", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"v6", "Static Base", "TLS", "Unused"};
  திரும்பு parseStringAttribute("ABI_PCS_R9_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RW_data(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  திரும்பு parseStringAttribute("ABI_PCS_RW_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RO_data(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  திரும்பு parseStringAttribute("ABI_PCS_RO_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  திரும்பு parseStringAttribute("ABI_PCS_GOT_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  திரும்பு parseStringAttribute("ABI_PCS_wchar_t", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"IEEE-754", "Runtime"};
  திரும்பு parseStringAttribute("ABI_FP_rounding", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Unsupported", "IEEE-754", "Sign Only"};
  திரும்பு parseStringAttribute("ABI_FP_denormal", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "IEEE-754"};
  திரும்பு parseStringAttribute("ABI_FP_exceptions", tag, ArrayRef(strings));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "IEEE-754"};
  திரும்பு parseStringAttribute("ABI_FP_user_exceptions", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_எண்_model(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  திரும்பு parseStringAttribute("ABI_FP_number_model", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_align_needed(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t மதிப்பு_2 = de.getULEB128(cursor);

  std::string description;
  எனில் (மதிப்பு_2 < std::size(strings))
    description = strings[மதிப்பு_2];
  இல்லையெனில் எனில் (மதிப்பு_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << மதிப்பு_2) +
                  "-byte extended alignment";
  இல்லையெனில்
    description = "Invalid";

  printAttribute(tag, மதிப்பு_2, description);
  திரும்பு Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType tag) {
  நிலையான மாறிலி எழுத்து *strings[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t மதிப்பு_2 = de.getULEB128(cursor);

  std::string description;
  எனில் (மதிப்பு_2 < std::size(strings))
    description = std::string(strings[மதிப்பு_2]);
  இல்லையெனில் எனில் (மதிப்பு_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << மதிப்பு_2) + std::string("-byte data alignment");
  இல்லையெனில்
    description = "Invalid";

  printAttribute(tag, மதிப்பு_2, description);
  திரும்பு Error::success();
}

Error ARMAttributeParser::ABI_enum_size(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  திரும்பு parseStringAttribute("ABI_enum_size", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  திரும்பு parseStringAttribute("ABI_HardFP_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_VFP_args(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  திரும்பு parseStringAttribute("ABI_VFP_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_WMMX_args(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"AAPCS", "iWMMX", "Custom"};
  திரும்பு parseStringAttribute("ABI_WMMX_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  திரும்பு parseStringAttribute("ABI_optimization_goals", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  திரும்பு parseStringAttribute("ABI_FP_optimization_goals", tag,
                              ArrayRef(strings));
}

Error ARMAttributeParser::compatibility(AttrType tag) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  எனில் (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_எண்("Tag", tag);
    sw->start_வரி() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                               /*hasTagPrefix=*/பொய்));
    தேர்வு (integer) {
    நிலை 0:
      sw->printString("Description", StringRef("No Specific Requirements"));
      நிறுத்து;
    நிலை 1:
      sw->printString("Description", StringRef("AEABI Conformant"));
      நிறுத்து;
    இயல்பு:
      sw->printString("Description", StringRef("AEABI Non-Conformant"));
      நிறுத்து;
    }
  }
  திரும்பு Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "v6-style"};
  திரும்பு parseStringAttribute("CPU_unaligned_access", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_HP_extension(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"If Available", "Permitted"};
  திரும்பு parseStringAttribute("FP_HP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  திரும்பு parseStringAttribute("ABI_FP_16bit_format", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MPextension_use(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "Permitted"};
  திரும்பு parseStringAttribute("MPextension_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DIV_use(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  திரும்பு parseStringAttribute("DIV_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DSP_extension(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "Permitted"};
  திரும்பு parseStringAttribute("DSP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::T2EE_use(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted", "Permitted"};
  திரும்பு parseStringAttribute("T2EE_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Virtualization_use(AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  திரும்பு parseStringAttribute("Virtualization_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  திரும்பு parseStringAttribute("PAC_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  திரும்பு parseStringAttribute("BTI_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Used", "Used"};
  திரும்பு parseStringAttribute("PACRET_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType tag) {
  நிலையான மாறிலி எழுத்து *மாறிலி strings[] = {"Not Used", "Used"};
  திரும்பு parseStringAttribute("BTI_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::nodefaults(AttrType tag) {
  uint64_t மதிப்பு_2 = de.getULEB128(cursor);
  printAttribute(tag, மதிப்பு_2, "Unspecified Tags UNDEFINED");
  திரும்பு Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType tag) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_மதிப்பு;

  SmallString<8> Description;
  raw_svector_ostream DescStream(Description);

  uint64_t InitialOffset = cursor.tell();
  StringRef Raw_String_மதிப்பு = de.getCStrRef(cursor);
  uint64_t FinalOffset = cursor.tell();
  cursor.seek(InitialOffset);
  uint64_t InnerTag = de.getULEB128(cursor);

  தருக்க ValidInnerTag =
      any_of(tagToStringMap, [InnerTag](மாறிலி TagNameItem &Item) {
        திரும்பு Item.attr == InnerTag;
      });

  எனில் (!ValidInnerTag) {
    return_மதிப்பு =
        createStringError(errc::argument_out_of_domain,
                          Twine(InnerTag) + " is not a valid tag number");
  } இல்லையெனில் {
    தேர்வு (InnerTag) {
    நிலை ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_மதிப்பு = de.getULEB128(cursor);
      auto strings = ArrayRef(CPU_arch_strings);
      எனில் (Inner_மதிப்பு >= strings.size()) {
        return_மதிப்பு = createStringError(
            errc::argument_out_of_domain,
            Twine(Inner_மதிப்பு) + " is not a valid " +
                ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
                " value");
      } இல்லையெனில் {
        DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                   << " = " << Inner_மதிப்பு;
        எனில் (strings[Inner_மதிப்பு])
          DescStream << " (" << strings[Inner_மதிப்பு] << ')';
      }
      நிறுத்து;
    }
    நிலை ARMBuildAttrs::also_compatible_with:
      return_மதிப்பு = createStringError(
          errc::invalid_argument,
          ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
              " cannot be recursively defined");
      நிறுத்து;
    நிலை ARMBuildAttrs::CPU_raw_name:
    நிலை ARMBuildAttrs::CPU_name:
    நிலை ARMBuildAttrs::compatibility:
    நிலை ARMBuildAttrs::conformance: {
      StringRef Inner_மதிப்பு = de.getCStrRef(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_மதிப்பு;
      நிறுத்து;
    }
    இயல்பு: {
      uint64_t Inner_மதிப்பு = de.getULEB128(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_மதிப்பு;
    }
    }
  }

  setAttributeString(tag, Raw_String_மதிப்பு);
  எனில் (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_எண்("Tag", tag);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap, பொய்));
    sw->printStringEscaped("Value", Raw_String_மதிப்பு);
    எனில் (!Description.empty()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(FinalOffset);

  திரும்பு return_மதிப்பு ? std::move(*return_மதிப்பு) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t tag, தருக்க &handled) {
  handled = பொய்;
  க்கு (மாறிலி auto &AH : displayRoutines) {
    எனில் (uint64_t(AH.attribute) == tag) {
      எனில் (Error e = (இது->*AH.routine)(static_cast<AttrType>(tag)))
        திரும்பு e;
      handled = உண்மை;
      நிறுத்து;
    }
  }

  திரும்பு Error::success();
}
