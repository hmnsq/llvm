//===- ARMAttributeParser.cpp - ARM Attribute Information Printer ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ARMAttributeParser.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ARMBuildAttributes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ScopedPrinter.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <optional>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::ARMBuildAttrs;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ATTRIBUTE_HANDLER(attr)                                                \
  { ARMBuildAttrs::attr, &ARMAttributeParser::attr }

ਅਚਲ ARMAttributeParser::DisplayHandler ARMAttributeParser::displayRoutines[] =
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
        ATTRIBUTE_HANDLER(ABI_PCS_RW_ਅੰਕੜੇ),
        ATTRIBUTE_HANDLER(ABI_PCS_RO_ਅੰਕੜੇ),
        ATTRIBUTE_HANDLER(ABI_PCS_GOT_use),
        ATTRIBUTE_HANDLER(ABI_PCS_wchar_t),
        ATTRIBUTE_HANDLER(ABI_FP_rounding),
        ATTRIBUTE_HANDLER(ABI_FP_denormal),
        ATTRIBUTE_HANDLER(ABI_FP_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_user_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_ਸੰਖਿਆ_model),
        ATTRIBUTE_HANDLER(ABI_align_needed),
        ATTRIBUTE_HANDLER(ABI_align_preserved),
        ATTRIBUTE_HANDLER(ABI_enum_ਮਾਪ),
        ATTRIBUTE_HANDLER(ABI_HardFP_use),
        ATTRIBUTE_HANDLER(ABI_VFP_ਦਲੀਲਾਂ),
        ATTRIBUTE_HANDLER(ABI_WMMX_ਦਲੀਲਾਂ),
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

#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ ATTRIBUTE_HANDLER

Error ARMAttributeParser::stringAttribute(AttrType ਟੈਗ_2) {
  StringRef ਟੈਗ_Name =
      ELFAttrs::attrTypeAsString(ਟੈਗ_2, ਟੈਗ_ਮੰਜ਼ਿਲ_String_Map, /*hasTagPrefix=*/ਝੂਠ);
  StringRef desc = de.getCStrRef(cursor);

  ਜੇ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_ਸੰਖਿਆ("Tag", ਟੈਗ_2);
    ਜੇ (!ਟੈਗ_Name.ਖਾਲੀ_ਥਾਵਾਂ())
      sw->printString("TagName", ਟੈਗ_Name);
    sw->printString("Value", desc);
  }
  ਵਾਪਸ Error::success();
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ CPU_arch_ਲੜੀਆਂ[] = {"Pre-v4",
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
                                               ਖਾਲੀ_ਸੂਚਕ,
                                               ਖਾਲੀ_ਸੂਚਕ,
                                               ਖਾਲੀ_ਸੂਚਕ,
                                               "ARM v8.1-M Mainline",
                                               "ARM v9-A"};

Error ARMAttributeParser::CPU_arch(AttrType ਟੈਗ_2) {
  ਵਾਪਸ parseStringAttribute("CPU_arch", ਟੈਗ_2, ArrayRef(CPU_arch_ਲੜੀਆਂ));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType ਟੈਗ_2) {
  uint64_t ਮੁੱਲ_2 = de.getULEB128(cursor);

  StringRef profile;
  ਚੋਣ (ਮੁੱਲ_2) {
  ਮੂਲ: profile = "Unknown"; ਰੋਕੋ;
  ਹਾਲਤ 'A': profile = "Application"; ਰੋਕੋ;
  ਹਾਲਤ 'R': profile = "Real-time"; ਰੋਕੋ;
  ਹਾਲਤ 'M': profile = "Microcontroller"; ਰੋਕੋ;
  ਹਾਲਤ 'S': profile = "Classic"; ਰੋਕੋ;
  ਹਾਲਤ 0: profile = "None"; ਰੋਕੋ;
  }

  printAttribute(ਟੈਗ_2, ਮੁੱਲ_2, profile);
  ਵਾਪਸ Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "Permitted"};
  ਵਾਪਸ parseStringAttribute("ARM_ISA_use", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  ਵਾਪਸ parseStringAttribute("THUMB_ISA_use", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::FP_arch(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  ਵਾਪਸ parseStringAttribute("FP_arch", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::WMMX_arch(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  ਵਾਪਸ parseStringAttribute("WMMX_arch", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  ਵਾਪਸ parseStringAttribute("Advanced_SIMD_arch", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::MVE_arch(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  ਵਾਪਸ parseStringAttribute("MVE_arch", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::PCS_config(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  ਵਾਪਸ parseStringAttribute("PCS_config", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"v6", "Static Base", "TLS", "Unused"};
  ਵਾਪਸ parseStringAttribute("ABI_PCS_R9_use", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::ABI_PCS_RW_ਅੰਕੜੇ(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  ਵਾਪਸ parseStringAttribute("ABI_PCS_RW_data", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::ABI_PCS_RO_ਅੰਕੜੇ(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  ਵਾਪਸ parseStringAttribute("ABI_PCS_RO_data", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  ਵਾਪਸ parseStringAttribute("ABI_PCS_GOT_use", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  ਵਾਪਸ parseStringAttribute("ABI_PCS_wchar_t", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"IEEE-754", "Runtime"};
  ਵਾਪਸ parseStringAttribute("ABI_FP_rounding", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Unsupported", "IEEE-754", "Sign Only"};
  ਵਾਪਸ parseStringAttribute("ABI_FP_denormal", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "IEEE-754"};
  ਵਾਪਸ parseStringAttribute("ABI_FP_exceptions", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "IEEE-754"};
  ਵਾਪਸ parseStringAttribute("ABI_FP_user_exceptions", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::ABI_FP_ਸੰਖਿਆ_model(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  ਵਾਪਸ parseStringAttribute("ABI_FP_number_model", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::ABI_align_needed(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t ਮੁੱਲ_2 = de.getULEB128(cursor);

  std::string description;
  ਜੇ (ਮੁੱਲ_2 < std::ਮਾਪ_2(ਲੜੀਆਂ))
    description = ਲੜੀਆਂ[ਮੁੱਲ_2];
  ਨਹੀਂਤਾਂ ਜੇ (ਮੁੱਲ_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << ਮੁੱਲ_2) +
                  "-byte extended alignment";
  ਨਹੀਂਤਾਂ
    description = "Invalid";

  printAttribute(ਟੈਗ_2, ਮੁੱਲ_2, description);
  ਵਾਪਸ Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਲੜੀਆਂ[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t ਮੁੱਲ_2 = de.getULEB128(cursor);

  std::string description;
  ਜੇ (ਮੁੱਲ_2 < std::ਮਾਪ_2(ਲੜੀਆਂ))
    description = std::string(ਲੜੀਆਂ[ਮੁੱਲ_2]);
  ਨਹੀਂਤਾਂ ਜੇ (ਮੁੱਲ_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << ਮੁੱਲ_2) + std::string("-byte data alignment");
  ਨਹੀਂਤਾਂ
    description = "Invalid";

  printAttribute(ਟੈਗ_2, ਮੁੱਲ_2, description);
  ਵਾਪਸ Error::success();
}

Error ARMAttributeParser::ABI_enum_ਮਾਪ(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  ਵਾਪਸ parseStringAttribute("ABI_enum_size", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  ਵਾਪਸ parseStringAttribute("ABI_HardFP_use", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::ABI_VFP_ਦਲੀਲਾਂ(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  ਵਾਪਸ parseStringAttribute("ABI_VFP_args", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::ABI_WMMX_ਦਲੀਲਾਂ(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"AAPCS", "iWMMX", "Custom"};
  ਵਾਪਸ parseStringAttribute("ABI_WMMX_args", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  ਵਾਪਸ parseStringAttribute("ABI_optimization_goals", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  ਵਾਪਸ parseStringAttribute("ABI_FP_optimization_goals", ਟੈਗ_2,
                              ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::compatibility(AttrType ਟੈਗ_2) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  ਜੇ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_ਸੰਖਿਆ("Tag", ਟੈਗ_2);
    sw->ਸ਼ੁਰੂ_ਸਤਰ() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(ਟੈਗ_2, ਟੈਗ_ਮੰਜ਼ਿਲ_String_Map,
                                               /*hasTagPrefix=*/ਝੂਠ));
    ਚੋਣ (integer) {
    ਹਾਲਤ 0:
      sw->printString("Description", StringRef("No Specific Requirements"));
      ਰੋਕੋ;
    ਹਾਲਤ 1:
      sw->printString("Description", StringRef("AEABI Conformant"));
      ਰੋਕੋ;
    ਮੂਲ:
      sw->printString("Description", StringRef("AEABI Non-Conformant"));
      ਰੋਕੋ;
    }
  }
  ਵਾਪਸ Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "v6-style"};
  ਵਾਪਸ parseStringAttribute("CPU_unaligned_access", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::FP_HP_extension(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"If Available", "Permitted"};
  ਵਾਪਸ parseStringAttribute("FP_HP_extension", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  ਵਾਪਸ parseStringAttribute("ABI_FP_16bit_format", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::MPextension_use(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "Permitted"};
  ਵਾਪਸ parseStringAttribute("MPextension_use", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::DIV_use(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  ਵਾਪਸ parseStringAttribute("DIV_use", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::DSP_extension(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "Permitted"};
  ਵਾਪਸ parseStringAttribute("DSP_extension", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::T2EE_use(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted", "Permitted"};
  ਵਾਪਸ parseStringAttribute("T2EE_use", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::Virtualization_use(AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  ਵਾਪਸ parseStringAttribute("Virtualization_use", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  ਵਾਪਸ parseStringAttribute("PAC_extension", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  ਵਾਪਸ parseStringAttribute("BTI_extension", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Used", "Used"};
  ਵਾਪਸ parseStringAttribute("PACRET_use", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType ਟੈਗ_2) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ ਲੜੀਆਂ[] = {"Not Used", "Used"};
  ਵਾਪਸ parseStringAttribute("BTI_use", ਟੈਗ_2, ArrayRef(ਲੜੀਆਂ));
}

Error ARMAttributeParser::nodefaults(AttrType ਟੈਗ_2) {
  uint64_t ਮੁੱਲ_2 = de.getULEB128(cursor);
  printAttribute(ਟੈਗ_2, ਮੁੱਲ_2, "Unspecified Tags UNDEFINED");
  ਵਾਪਸ Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType ਟੈਗ_2) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_ਮੁੱਲ;

  SmallString<8> Description;
  raw_svector_ostream Desc_ਧਾਰਾ(Description);

  uint64_t Initial_ਵਿੱਥ = cursor.tell();
  StringRef Raw_String_ਮੁੱਲ = de.getCStrRef(cursor);
  uint64_t Final_ਵਿੱਥ = cursor.tell();
  cursor.seek(Initial_ਵਿੱਥ);
  uint64_t Inner_ਟੈਗ = de.getULEB128(cursor);

  ਤਾਰਕਿਕ Valid_Inner_ਟੈਗ =
      any_ਦਾ(ਟੈਗ_ਮੰਜ਼ਿਲ_String_Map, [Inner_ਟੈਗ](ਅਚਲ ਟੈਗ_Name_ਵਸਤੂ &ਵਸਤੂ) {
        ਵਾਪਸ ਵਸਤੂ.attr == Inner_ਟੈਗ;
      });

  ਜੇ (!Valid_Inner_ਟੈਗ) {
    return_ਮੁੱਲ =
        createStringError(errc::ਦਲੀਲ_ਬਾਹਰਲਾ_ਦਾ_domain,
                          Twine(Inner_ਟੈਗ) + " is not a valid tag number");
  } ਨਹੀਂਤਾਂ {
    ਚੋਣ (Inner_ਟੈਗ) {
    ਹਾਲਤ ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_ਮੁੱਲ = de.getULEB128(cursor);
      ਸਵੈਚਾਲਿਤ ਲੜੀਆਂ = ArrayRef(CPU_arch_ਲੜੀਆਂ);
      ਜੇ (Inner_ਮੁੱਲ >= ਲੜੀਆਂ.ਮਾਪ_2()) {
        return_ਮੁੱਲ = createStringError(
            errc::ਦਲੀਲ_ਬਾਹਰਲਾ_ਦਾ_domain,
            Twine(Inner_ਮੁੱਲ) + " is not a valid " +
                ELFAttrs::attrTypeAsString(Inner_ਟੈਗ, ਟੈਗ_ਮੰਜ਼ਿਲ_String_Map) +
                " value");
      } ਨਹੀਂਤਾਂ {
        Desc_ਧਾਰਾ << ELFAttrs::attrTypeAsString(Inner_ਟੈਗ, ਟੈਗ_ਮੰਜ਼ਿਲ_String_Map)
                   << " = " << Inner_ਮੁੱਲ;
        ਜੇ (ਲੜੀਆਂ[Inner_ਮੁੱਲ])
          Desc_ਧਾਰਾ << " (" << ਲੜੀਆਂ[Inner_ਮੁੱਲ] << ')';
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ ARMBuildAttrs::also_compatible_with:
      return_ਮੁੱਲ = createStringError(
          errc::invalid_ਦਲੀਲ,
          ELFAttrs::attrTypeAsString(Inner_ਟੈਗ, ਟੈਗ_ਮੰਜ਼ਿਲ_String_Map) +
              " cannot be recursively defined");
      ਰੋਕੋ;
    ਹਾਲਤ ARMBuildAttrs::CPU_raw_name:
    ਹਾਲਤ ARMBuildAttrs::CPU_name:
    ਹਾਲਤ ARMBuildAttrs::compatibility:
    ਹਾਲਤ ARMBuildAttrs::conformance: {
      StringRef Inner_ਮੁੱਲ = de.getCStrRef(cursor);
      Desc_ਧਾਰਾ << ELFAttrs::attrTypeAsString(Inner_ਟੈਗ, ਟੈਗ_ਮੰਜ਼ਿਲ_String_Map)
                 << " = " << Inner_ਮੁੱਲ;
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      uint64_t Inner_ਮੁੱਲ = de.getULEB128(cursor);
      Desc_ਧਾਰਾ << ELFAttrs::attrTypeAsString(Inner_ਟੈਗ, ਟੈਗ_ਮੰਜ਼ਿਲ_String_Map)
                 << " = " << Inner_ਮੁੱਲ;
    }
    }
  }

  ਸੈੱਟ_Attribute_String(ਟੈਗ_2, Raw_String_ਮੁੱਲ);
  ਜੇ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_ਸੰਖਿਆ("Tag", ਟੈਗ_2);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(ਟੈਗ_2, ਟੈਗ_ਮੰਜ਼ਿਲ_String_Map, ਝੂਠ));
    sw->printStringEscaped("Value", Raw_String_ਮੁੱਲ);
    ਜੇ (!Description.ਖਾਲੀ_ਥਾਵਾਂ()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(Final_ਵਿੱਥ);

  ਵਾਪਸ return_ਮੁੱਲ ? std::move(*return_ਮੁੱਲ) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t ਟੈਗ_2, ਤਾਰਕਿਕ &handled) {
  handled = ਝੂਠ;
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &AH : displayRoutines) {
    ਜੇ (uint64_t(AH.attribute) == ਟੈਗ_2) {
      ਜੇ (Error e = (ਇਹ->*AH.routine)(static_cast<AttrType>(ਟੈਗ_2)))
        ਵਾਪਸ e;
      handled = ਸੱਚ;
      ਰੋਕੋ;
    }
  }

  ਵਾਪਸ Error::success();
}
