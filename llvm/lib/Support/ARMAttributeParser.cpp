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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::ARMBuildAttrs;

#ವ್ಯಾಖ್ಯಾನಿಸು ATTRIBUTE_HANDLER(attr)                                                \
  { ARMBuildAttrs::attr, &ARMAttributeParser::attr }

ಸ್ಥಿರಾಂಕ ARMAttributeParser::DisplayHandler ARMAttributeParser::displayRoutines[] =
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
        ATTRIBUTE_HANDLER(ABI_PCS_RW_ದತ್ತಾಂಶ),
        ATTRIBUTE_HANDLER(ABI_PCS_RO_ದತ್ತಾಂಶ),
        ATTRIBUTE_HANDLER(ABI_PCS_GOT_use),
        ATTRIBUTE_HANDLER(ABI_PCS_wchar_t),
        ATTRIBUTE_HANDLER(ABI_FP_rounding),
        ATTRIBUTE_HANDLER(ABI_FP_denormal),
        ATTRIBUTE_HANDLER(ABI_FP_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_user_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_ಸಂಖ್ಯೆ_model),
        ATTRIBUTE_HANDLER(ABI_align_needed),
        ATTRIBUTE_HANDLER(ABI_align_preserved),
        ATTRIBUTE_HANDLER(ABI_enum_ಗಾತ್ರ),
        ATTRIBUTE_HANDLER(ABI_HardFP_use),
        ATTRIBUTE_HANDLER(ABI_VFP_ವಾದಗಳು),
        ATTRIBUTE_HANDLER(ABI_WMMX_ವಾದಗಳು),
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

#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು ATTRIBUTE_HANDLER

Error ARMAttributeParser::stringAttribute(AttrType ಟ್ಯಾಗ್_2) {
  StringRef ಟ್ಯಾಗ್_Name =
      ELFAttrs::attrTypeAsString(ಟ್ಯಾಗ್_2, ಟ್ಯಾಗ್_ಗಮ್ಯ_String_Map, /*hasTagPrefix=*/ಸುಳ್ಳು);
  StringRef desc = de.getCStrRef(cursor);

  ಆದರೆ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_ಸಂಖ್ಯೆ("Tag", ಟ್ಯಾಗ್_2);
    ಆದರೆ (!ಟ್ಯಾಗ್_Name.ಖಾಲಿಗಳು())
      sw->printString("TagName", ಟ್ಯಾಗ್_Name);
    sw->printString("Value", desc);
  }
  ಹಿಂತಿರುಗಿ Error::success();
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ CPU_arch_ಅಕ್ಷರಸರಣಿಗಳು[] = {"Pre-v4",
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
                                               ಶೂನ್ಯಸೂಚಕ,
                                               ಶೂನ್ಯಸೂಚಕ,
                                               ಶೂನ್ಯಸೂಚಕ,
                                               "ARM v8.1-M Mainline",
                                               "ARM v9-A"};

Error ARMAttributeParser::CPU_arch(AttrType ಟ್ಯಾಗ್_2) {
  ಹಿಂತಿರುಗಿ parseStringAttribute("CPU_arch", ಟ್ಯಾಗ್_2, ArrayRef(CPU_arch_ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType ಟ್ಯಾಗ್_2) {
  uint64_t ಮೌಲ್ಯ_2 = de.getULEB128(cursor);

  StringRef profile;
  ಆಯ್ಕೆ (ಮೌಲ್ಯ_2) {
  ಪೂರ್ವನಿಯೋಜಿತ: profile = "Unknown"; ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ 'A': profile = "Application"; ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ 'R': profile = "Real-time"; ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ 'M': profile = "Microcontroller"; ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ 'S': profile = "Classic"; ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ 0: profile = "None"; ನಿಲ್ಲಿಸು;
  }

  printAttribute(ಟ್ಯಾಗ್_2, ಮೌಲ್ಯ_2, profile);
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "Permitted"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ARM_ISA_use", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("THUMB_ISA_use", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::FP_arch(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("FP_arch", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::WMMX_arch(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("WMMX_arch", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("Advanced_SIMD_arch", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::MVE_arch(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("MVE_arch", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::PCS_config(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("PCS_config", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"v6", "Static Base", "TLS", "Unused"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_PCS_R9_use", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::ABI_PCS_RW_ದತ್ತಾಂಶ(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_PCS_RW_data", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::ABI_PCS_RO_ದತ್ತಾಂಶ(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_PCS_RO_data", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_PCS_GOT_use", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_PCS_wchar_t", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"IEEE-754", "Runtime"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_FP_rounding", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Unsupported", "IEEE-754", "Sign Only"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_FP_denormal", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "IEEE-754"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_FP_exceptions", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "IEEE-754"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_FP_user_exceptions", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::ABI_FP_ಸಂಖ್ಯೆ_model(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_FP_number_model", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::ABI_align_needed(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t ಮೌಲ್ಯ_2 = de.getULEB128(cursor);

  std::string description;
  ಆದರೆ (ಮೌಲ್ಯ_2 < std::ಗಾತ್ರ_3(ಅಕ್ಷರಸರಣಿಗಳು))
    description = ಅಕ್ಷರಸರಣಿಗಳು[ಮೌಲ್ಯ_2];
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮೌಲ್ಯ_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << ಮೌಲ್ಯ_2) +
                  "-byte extended alignment";
  ಇಲ್ಲದಿದ್ದರೆ
    description = "Invalid";

  printAttribute(ಟ್ಯಾಗ್_2, ಮೌಲ್ಯ_2, description);
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t ಮೌಲ್ಯ_2 = de.getULEB128(cursor);

  std::string description;
  ಆದರೆ (ಮೌಲ್ಯ_2 < std::ಗಾತ್ರ_3(ಅಕ್ಷರಸರಣಿಗಳು))
    description = std::string(ಅಕ್ಷರಸರಣಿಗಳು[ಮೌಲ್ಯ_2]);
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮೌಲ್ಯ_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << ಮೌಲ್ಯ_2) + std::string("-byte data alignment");
  ಇಲ್ಲದಿದ್ದರೆ
    description = "Invalid";

  printAttribute(ಟ್ಯಾಗ್_2, ಮೌಲ್ಯ_2, description);
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ARMAttributeParser::ABI_enum_ಗಾತ್ರ(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_enum_size", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_HardFP_use", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::ABI_VFP_ವಾದಗಳು(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_VFP_args", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::ABI_WMMX_ವಾದಗಳು(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"AAPCS", "iWMMX", "Custom"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_WMMX_args", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_optimization_goals", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_FP_optimization_goals", ಟ್ಯಾಗ್_2,
                              ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::compatibility(AttrType ಟ್ಯಾಗ್_2) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  ಆದರೆ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_ಸಂಖ್ಯೆ("Tag", ಟ್ಯಾಗ್_2);
    sw->ಆರಂಭ_ಸಾಲು() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(ಟ್ಯಾಗ್_2, ಟ್ಯಾಗ್_ಗಮ್ಯ_String_Map,
                                               /*hasTagPrefix=*/ಸುಳ್ಳು));
    ಆಯ್ಕೆ (integer) {
    ಸ್ಥಿತಿ 0:
      sw->printString("Description", StringRef("No Specific Requirements"));
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ 1:
      sw->printString("Description", StringRef("AEABI Conformant"));
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      sw->printString("Description", StringRef("AEABI Non-Conformant"));
      ನಿಲ್ಲಿಸು;
    }
  }
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "v6-style"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("CPU_unaligned_access", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::FP_HP_extension(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"If Available", "Permitted"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("FP_HP_extension", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("ABI_FP_16bit_format", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::MPextension_use(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "Permitted"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("MPextension_use", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::DIV_use(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("DIV_use", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::DSP_extension(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "Permitted"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("DSP_extension", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::T2EE_use(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted", "Permitted"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("T2EE_use", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::Virtualization_use(AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("Virtualization_use", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("PAC_extension", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("BTI_extension", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Used", "Used"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("PACRET_use", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType ಟ್ಯಾಗ್_2) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ ಅಕ್ಷರಸರಣಿಗಳು[] = {"Not Used", "Used"};
  ಹಿಂತಿರುಗಿ parseStringAttribute("BTI_use", ಟ್ಯಾಗ್_2, ArrayRef(ಅಕ್ಷರಸರಣಿಗಳು));
}

Error ARMAttributeParser::nodefaults(AttrType ಟ್ಯಾಗ್_2) {
  uint64_t ಮೌಲ್ಯ_2 = de.getULEB128(cursor);
  printAttribute(ಟ್ಯಾಗ್_2, ಮೌಲ್ಯ_2, "Unspecified Tags UNDEFINED");
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType ಟ್ಯಾಗ್_2) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_ಮೌಲ್ಯ;

  SmallString<8> Description;
  raw_svector_ostream Desc_ಹರಿವು(Description);

  uint64_t Initial_ಸ್ಥಾನಾಂತರ = cursor.tell();
  StringRef Raw_String_ಮೌಲ್ಯ = de.getCStrRef(cursor);
  uint64_t Final_ಸ್ಥಾನಾಂತರ = cursor.tell();
  cursor.seek(Initial_ಸ್ಥಾನಾಂತರ);
  uint64_t Inner_ಟ್ಯಾಗ್ = de.getULEB128(cursor);

  ತಾರ್ಕಿಕ Valid_Inner_ಟ್ಯಾಗ್ =
      any_ದ(ಟ್ಯಾಗ್_ಗಮ್ಯ_String_Map, [Inner_ಟ್ಯಾಗ್](ಸ್ಥಿರಾಂಕ ಟ್ಯಾಗ್_Name_ಅಂಶ &ಅಂಶ) {
        ಹಿಂತಿರುಗಿ ಅಂಶ.attr == Inner_ಟ್ಯಾಗ್;
      });

  ಆದರೆ (!Valid_Inner_ಟ್ಯಾಗ್) {
    return_ಮೌಲ್ಯ =
        createStringError(errc::ವಾದ_ಹೊರಹೋಗು_ದ_domain,
                          Twine(Inner_ಟ್ಯಾಗ್) + " is not a valid tag number");
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಆಯ್ಕೆ (Inner_ಟ್ಯಾಗ್) {
    ಸ್ಥಿತಿ ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_ಮೌಲ್ಯ = de.getULEB128(cursor);
      ಸ್ವಯಂಚಾಲಿತ ಅಕ್ಷರಸರಣಿಗಳು = ArrayRef(CPU_arch_ಅಕ್ಷರಸರಣಿಗಳು);
      ಆದರೆ (Inner_ಮೌಲ್ಯ >= ಅಕ್ಷರಸರಣಿಗಳು.ಗಾತ್ರ_3()) {
        return_ಮೌಲ್ಯ = createStringError(
            errc::ವಾದ_ಹೊರಹೋಗು_ದ_domain,
            Twine(Inner_ಮೌಲ್ಯ) + " is not a valid " +
                ELFAttrs::attrTypeAsString(Inner_ಟ್ಯಾಗ್, ಟ್ಯಾಗ್_ಗಮ್ಯ_String_Map) +
                " value");
      } ಇಲ್ಲದಿದ್ದರೆ {
        Desc_ಹರಿವು << ELFAttrs::attrTypeAsString(Inner_ಟ್ಯಾಗ್, ಟ್ಯಾಗ್_ಗಮ್ಯ_String_Map)
                   << " = " << Inner_ಮೌಲ್ಯ;
        ಆದರೆ (ಅಕ್ಷರಸರಣಿಗಳು[Inner_ಮೌಲ್ಯ])
          Desc_ಹರಿವು << " (" << ಅಕ್ಷರಸರಣಿಗಳು[Inner_ಮೌಲ್ಯ] << ')';
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ ARMBuildAttrs::also_compatible_with:
      return_ಮೌಲ್ಯ = createStringError(
          errc::invalid_ವಾದ,
          ELFAttrs::attrTypeAsString(Inner_ಟ್ಯಾಗ್, ಟ್ಯಾಗ್_ಗಮ್ಯ_String_Map) +
              " cannot be recursively defined");
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ ARMBuildAttrs::CPU_raw_name:
    ಸ್ಥಿತಿ ARMBuildAttrs::CPU_name:
    ಸ್ಥಿತಿ ARMBuildAttrs::compatibility:
    ಸ್ಥಿತಿ ARMBuildAttrs::conformance: {
      StringRef Inner_ಮೌಲ್ಯ = de.getCStrRef(cursor);
      Desc_ಹರಿವು << ELFAttrs::attrTypeAsString(Inner_ಟ್ಯಾಗ್, ಟ್ಯಾಗ್_ಗಮ್ಯ_String_Map)
                 << " = " << Inner_ಮೌಲ್ಯ;
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      uint64_t Inner_ಮೌಲ್ಯ = de.getULEB128(cursor);
      Desc_ಹರಿವು << ELFAttrs::attrTypeAsString(Inner_ಟ್ಯಾಗ್, ಟ್ಯಾಗ್_ಗಮ್ಯ_String_Map)
                 << " = " << Inner_ಮೌಲ್ಯ;
    }
    }
  }

  ಗಣ_Attribute_String(ಟ್ಯಾಗ್_2, Raw_String_ಮೌಲ್ಯ);
  ಆದರೆ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_ಸಂಖ್ಯೆ("Tag", ಟ್ಯಾಗ್_2);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(ಟ್ಯಾಗ್_2, ಟ್ಯಾಗ್_ಗಮ್ಯ_String_Map, ಸುಳ್ಳು));
    sw->printStringEscaped("Value", Raw_String_ಮೌಲ್ಯ);
    ಆದರೆ (!Description.ಖಾಲಿಗಳು()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(Final_ಸ್ಥಾನಾಂತರ);

  ಹಿಂತಿರುಗಿ return_ಮೌಲ್ಯ ? std::move(*return_ಮೌಲ್ಯ) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t ಟ್ಯಾಗ್_2, ತಾರ್ಕಿಕ &handled) {
  handled = ಸುಳ್ಳು;
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &AH : displayRoutines) {
    ಆದರೆ (uint64_t(AH.attribute) == ಟ್ಯಾಗ್_2) {
      ಆದರೆ (Error e = (ಇದು->*AH.routine)(static_cast<AttrType>(ಟ್ಯಾಗ್_2)))
        ಹಿಂತಿರುಗಿ e;
      handled = ನಿಜ;
      ನಿಲ್ಲಿಸು;
    }
  }

  ಹಿಂತಿರುಗಿ Error::success();
}
