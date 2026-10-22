//===- ARMAttributeParser.cpp - ARM Attribute Information Printer ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/ARMAttributeParser.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ARMBuildAttributes.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ScopedPrinter.h"
#ಅನ್ತರ್ಭಾವ <optional>

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::ARMBuildAttrs;

#define ATTRIBUTE_HANDLER(attr)                                                \
  { ARMBuildAttrs::attr, &ARMAttributeParser::attr }

ಧ್ರುವ ARMAttributeParser::DisplayHandler ARMAttributeParser::displayRoutines[] =
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
        ATTRIBUTE_HANDLER(ABI_FP_ಸಂಖ್ಯೆ_model),
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
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/ಅಸತ್ಯ);
  StringRef desc = de.getCStrRef(cursor);

  ಯದಿ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_ಸಂಖ್ಯೆ("Tag", tag);
    ಯದಿ (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ CPU_arch_strings[] = {"Pre-v4",
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
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("CPU_arch", tag, ArrayRef(CPU_arch_strings));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType tag) {
  uint64_t ಮೌಲ್ಯ_2 = de.getULEB128(cursor);

  StringRef profile;
  ವಿಕಲ್ಪ (ಮೌಲ್ಯ_2) {
  ಸ್ವಭಾವ: profile = "Unknown"; ಭಙ್ಗ;
  ಪಕ್ಷ 'A': profile = "Application"; ಭಙ್ಗ;
  ಪಕ್ಷ 'R': profile = "Real-time"; ಭಙ್ಗ;
  ಪಕ್ಷ 'M': profile = "Microcontroller"; ಭಙ್ಗ;
  ಪಕ್ಷ 'S': profile = "Classic"; ಭಙ್ಗ;
  ಪಕ್ಷ 0: profile = "None"; ಭಙ್ಗ;
  }

  printAttribute(tag, ಮೌಲ್ಯ_2, profile);
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "Permitted"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ARM_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("THUMB_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_arch(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("FP_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::WMMX_arch(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("WMMX_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("Advanced_SIMD_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MVE_arch(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("MVE_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PCS_config(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("PCS_config", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"v6", "Static Base", "TLS", "Unused"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_PCS_R9_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RW_data(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_PCS_RW_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RO_data(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_PCS_RO_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_PCS_GOT_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_PCS_wchar_t", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"IEEE-754", "Runtime"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_FP_rounding", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Unsupported", "IEEE-754", "Sign Only"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_FP_denormal", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "IEEE-754"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_FP_exceptions", tag, ArrayRef(strings));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "IEEE-754"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_FP_user_exceptions", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_ಸಂಖ್ಯೆ_model(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_FP_number_model", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_align_needed(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t ಮೌಲ್ಯ_2 = de.getULEB128(cursor);

  std::string description;
  ಯದಿ (ಮೌಲ್ಯ_2 < std::size(strings))
    description = strings[ಮೌಲ್ಯ_2];
  ಅನ್ಯಥಾ ಯದಿ (ಮೌಲ್ಯ_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << ಮೌಲ್ಯ_2) +
                  "-byte extended alignment";
  ಅನ್ಯಥಾ
    description = "Invalid";

  printAttribute(tag, ಮೌಲ್ಯ_2, description);
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *strings[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t ಮೌಲ್ಯ_2 = de.getULEB128(cursor);

  std::string description;
  ಯದಿ (ಮೌಲ್ಯ_2 < std::size(strings))
    description = std::string(strings[ಮೌಲ್ಯ_2]);
  ಅನ್ಯಥಾ ಯದಿ (ಮೌಲ್ಯ_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << ಮೌಲ್ಯ_2) + std::string("-byte data alignment");
  ಅನ್ಯಥಾ
    description = "Invalid";

  printAttribute(tag, ಮೌಲ್ಯ_2, description);
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error ARMAttributeParser::ABI_enum_size(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_enum_size", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_HardFP_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_VFP_args(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_VFP_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_WMMX_args(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"AAPCS", "iWMMX", "Custom"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_WMMX_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_optimization_goals", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_FP_optimization_goals", tag,
                              ArrayRef(strings));
}

Error ARMAttributeParser::compatibility(AttrType tag) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  ಯದಿ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_ಸಂಖ್ಯೆ("Tag", tag);
    sw->start_ಸಾಲು() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                               /*hasTagPrefix=*/ಅಸತ್ಯ));
    ವಿಕಲ್ಪ (integer) {
    ಪಕ್ಷ 0:
      sw->printString("Description", StringRef("No Specific Requirements"));
      ಭಙ್ಗ;
    ಪಕ್ಷ 1:
      sw->printString("Description", StringRef("AEABI Conformant"));
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      sw->printString("Description", StringRef("AEABI Non-Conformant"));
      ಭಙ್ಗ;
    }
  }
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "v6-style"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("CPU_unaligned_access", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_HP_extension(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"If Available", "Permitted"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("FP_HP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("ABI_FP_16bit_format", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MPextension_use(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "Permitted"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("MPextension_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DIV_use(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("DIV_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DSP_extension(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "Permitted"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("DSP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::T2EE_use(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted", "Permitted"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("T2EE_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Virtualization_use(AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("Virtualization_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("PAC_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("BTI_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Used", "Used"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("PACRET_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType tag) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ strings[] = {"Not Used", "Used"};
  ಪ್ರತ್ಯಾಗಮ parseStringAttribute("BTI_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::nodefaults(AttrType tag) {
  uint64_t ಮೌಲ್ಯ_2 = de.getULEB128(cursor);
  printAttribute(tag, ಮೌಲ್ಯ_2, "Unspecified Tags UNDEFINED");
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType tag) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_ಮೌಲ್ಯ;

  SmallString<8> Description;
  raw_svector_ostream DescStream(Description);

  uint64_t InitialOffset = cursor.tell();
  StringRef Raw_String_ಮೌಲ್ಯ = de.getCStrRef(cursor);
  uint64_t FinalOffset = cursor.tell();
  cursor.seek(InitialOffset);
  uint64_t InnerTag = de.getULEB128(cursor);

  ತಾರ್ಕಿಕ ValidInnerTag =
      any_of(tagToStringMap, [InnerTag](ಧ್ರುವ TagNameItem &Item) {
        ಪ್ರತ್ಯಾಗಮ Item.attr == InnerTag;
      });

  ಯದಿ (!ValidInnerTag) {
    return_ಮೌಲ್ಯ =
        createStringError(errc::argument_out_of_domain,
                          Twine(InnerTag) + " is not a valid tag number");
  } ಅನ್ಯಥಾ {
    ವಿಕಲ್ಪ (InnerTag) {
    ಪಕ್ಷ ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_ಮೌಲ್ಯ = de.getULEB128(cursor);
      auto strings = ArrayRef(CPU_arch_strings);
      ಯದಿ (Inner_ಮೌಲ್ಯ >= strings.size()) {
        return_ಮೌಲ್ಯ = createStringError(
            errc::argument_out_of_domain,
            Twine(Inner_ಮೌಲ್ಯ) + " is not a valid " +
                ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
                " value");
      } ಅನ್ಯಥಾ {
        DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                   << " = " << Inner_ಮೌಲ್ಯ;
        ಯದಿ (strings[Inner_ಮೌಲ್ಯ])
          DescStream << " (" << strings[Inner_ಮೌಲ್ಯ] << ')';
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ ARMBuildAttrs::also_compatible_with:
      return_ಮೌಲ್ಯ = createStringError(
          errc::invalid_argument,
          ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
              " cannot be recursively defined");
      ಭಙ್ಗ;
    ಪಕ್ಷ ARMBuildAttrs::CPU_raw_name:
    ಪಕ್ಷ ARMBuildAttrs::CPU_name:
    ಪಕ್ಷ ARMBuildAttrs::compatibility:
    ಪಕ್ಷ ARMBuildAttrs::conformance: {
      StringRef Inner_ಮೌಲ್ಯ = de.getCStrRef(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_ಮೌಲ್ಯ;
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      uint64_t Inner_ಮೌಲ್ಯ = de.getULEB128(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_ಮೌಲ್ಯ;
    }
    }
  }

  setAttributeString(tag, Raw_String_ಮೌಲ್ಯ);
  ಯದಿ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_ಸಂಖ್ಯೆ("Tag", tag);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap, ಅಸತ್ಯ));
    sw->printStringEscaped("Value", Raw_String_ಮೌಲ್ಯ);
    ಯದಿ (!Description.empty()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(FinalOffset);

  ಪ್ರತ್ಯಾಗಮ return_ಮೌಲ್ಯ ? std::move(*return_ಮೌಲ್ಯ) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t tag, ತಾರ್ಕಿಕ &handled) {
  handled = ಅಸತ್ಯ;
  ಪ್ರತಿ (ಧ್ರುವ auto &AH : displayRoutines) {
    ಯದಿ (uint64_t(AH.attribute) == tag) {
      ಯದಿ (Error e = (ಏತತ್->*AH.routine)(static_cast<AttrType>(tag)))
        ಪ್ರತ್ಯಾಗಮ e;
      handled = ಸತ್ಯ;
      ಭಙ್ಗ;
    }
  }

  ಪ್ರತ್ಯಾಗಮ Error::success();
}
