//===- ARMAttributeParser.cpp - ARM Attribute Information Printer ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/ARMAttributeParser.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/Support/ARMBuildAttributes.h"
#అన్తర్భావ "llvm/Support/Errc.h"
#అన్తర్భావ "llvm/Support/ScopedPrinter.h"
#అన్తర్భావ <optional>

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::ARMBuildAttrs;

#నిర్వచించు ATTRIBUTE_HANDLER(attr)                                                \
  { ARMBuildAttrs::attr, &ARMAttributeParser::attr }

స్థిరరాశి ARMAttributeParser::DisplayHandler ARMAttributeParser::displayRoutines[] =
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
        ATTRIBUTE_HANDLER(ABI_PCS_RW_దత్తాంశం),
        ATTRIBUTE_HANDLER(ABI_PCS_RO_దత్తాంశం),
        ATTRIBUTE_HANDLER(ABI_PCS_GOT_use),
        ATTRIBUTE_HANDLER(ABI_PCS_wchar_t),
        ATTRIBUTE_HANDLER(ABI_FP_rounding),
        ATTRIBUTE_HANDLER(ABI_FP_denormal),
        ATTRIBUTE_HANDLER(ABI_FP_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_user_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_సంఖ్య_model),
        ATTRIBUTE_HANDLER(ABI_align_needed),
        ATTRIBUTE_HANDLER(ABI_align_preserved),
        ATTRIBUTE_HANDLER(ABI_enum_కొలత),
        ATTRIBUTE_HANDLER(ABI_HardFP_use),
        ATTRIBUTE_HANDLER(ABI_VFP_వాదనలు),
        ATTRIBUTE_HANDLER(ABI_WMMX_వాదనలు),
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

#నిర్వచనం_రద్దు ATTRIBUTE_HANDLER

Error ARMAttributeParser::stringAttribute(AttrType ట్యాగ్_2) {
  StringRef ట్యాగ్_Name =
      ELFAttrs::attrTypeAsString(ట్యాగ్_2, ట్యాగ్_గమ్యం_String_Map, /*hasTagPrefix=*/అబద్ధం);
  StringRef desc = de.getCStrRef(cursor);

  ఒకవేళ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_సంఖ్య("Tag", ట్యాగ్_2);
    ఒకవేళ (!ట్యాగ్_Name.ఖాళీలు())
      sw->printString("TagName", ట్యాగ్_Name);
    sw->printString("Value", desc);
  }
  తిరిగి Error::success();
}

స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి CPU_arch_అక్షరశ్రేణులు[] = {"Pre-v4",
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
                                               శూన్యసూచిక,
                                               శూన్యసూచిక,
                                               శూన్యసూచిక,
                                               "ARM v8.1-M Mainline",
                                               "ARM v9-A"};

Error ARMAttributeParser::CPU_arch(AttrType ట్యాగ్_2) {
  తిరిగి parseStringAttribute("CPU_arch", ట్యాగ్_2, ArrayRef(CPU_arch_అక్షరశ్రేణులు));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType ట్యాగ్_2) {
  uint64_t విలువ_2 = de.getULEB128(cursor);

  StringRef profile;
  ఎంపిక (విలువ_2) {
  అప్రమేయ: profile = "Unknown"; ఆపు;
  స్థితి 'A': profile = "Application"; ఆపు;
  స్థితి 'R': profile = "Real-time"; ఆపు;
  స్థితి 'M': profile = "Microcontroller"; ఆపు;
  స్థితి 'S': profile = "Classic"; ఆపు;
  స్థితి 0: profile = "None"; ఆపు;
  }

  printAttribute(ట్యాగ్_2, విలువ_2, profile);
  తిరిగి Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "Permitted"};
  తిరిగి parseStringAttribute("ARM_ISA_use", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  తిరిగి parseStringAttribute("THUMB_ISA_use", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::FP_arch(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  తిరిగి parseStringAttribute("FP_arch", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::WMMX_arch(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  తిరిగి parseStringAttribute("WMMX_arch", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  తిరిగి parseStringAttribute("Advanced_SIMD_arch", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::MVE_arch(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  తిరిగి parseStringAttribute("MVE_arch", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::PCS_config(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  తిరిగి parseStringAttribute("PCS_config", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"v6", "Static Base", "TLS", "Unused"};
  తిరిగి parseStringAttribute("ABI_PCS_R9_use", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::ABI_PCS_RW_దత్తాంశం(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  తిరిగి parseStringAttribute("ABI_PCS_RW_data", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::ABI_PCS_RO_దత్తాంశం(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  తిరిగి parseStringAttribute("ABI_PCS_RO_data", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  తిరిగి parseStringAttribute("ABI_PCS_GOT_use", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  తిరిగి parseStringAttribute("ABI_PCS_wchar_t", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"IEEE-754", "Runtime"};
  తిరిగి parseStringAttribute("ABI_FP_rounding", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Unsupported", "IEEE-754", "Sign Only"};
  తిరిగి parseStringAttribute("ABI_FP_denormal", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "IEEE-754"};
  తిరిగి parseStringAttribute("ABI_FP_exceptions", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "IEEE-754"};
  తిరిగి parseStringAttribute("ABI_FP_user_exceptions", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::ABI_FP_సంఖ్య_model(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  తిరిగి parseStringAttribute("ABI_FP_number_model", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::ABI_align_needed(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t విలువ_2 = de.getULEB128(cursor);

  std::string description;
  ఒకవేళ (విలువ_2 < std::కొలత_2(అక్షరశ్రేణులు))
    description = అక్షరశ్రేణులు[విలువ_2];
  లేకపోతే ఒకవేళ (విలువ_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << విలువ_2) +
                  "-byte extended alignment";
  లేకపోతే
    description = "Invalid";

  printAttribute(ట్యాగ్_2, విలువ_2, description);
  తిరిగి Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *అక్షరశ్రేణులు[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t విలువ_2 = de.getULEB128(cursor);

  std::string description;
  ఒకవేళ (విలువ_2 < std::కొలత_2(అక్షరశ్రేణులు))
    description = std::string(అక్షరశ్రేణులు[విలువ_2]);
  లేకపోతే ఒకవేళ (విలువ_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << విలువ_2) + std::string("-byte data alignment");
  లేకపోతే
    description = "Invalid";

  printAttribute(ట్యాగ్_2, విలువ_2, description);
  తిరిగి Error::success();
}

Error ARMAttributeParser::ABI_enum_కొలత(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  తిరిగి parseStringAttribute("ABI_enum_size", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  తిరిగి parseStringAttribute("ABI_HardFP_use", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::ABI_VFP_వాదనలు(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  తిరిగి parseStringAttribute("ABI_VFP_args", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::ABI_WMMX_వాదనలు(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"AAPCS", "iWMMX", "Custom"};
  తిరిగి parseStringAttribute("ABI_WMMX_args", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  తిరిగి parseStringAttribute("ABI_optimization_goals", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  తిరిగి parseStringAttribute("ABI_FP_optimization_goals", ట్యాగ్_2,
                              ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::compatibility(AttrType ట్యాగ్_2) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  ఒకవేళ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_సంఖ్య("Tag", ట్యాగ్_2);
    sw->ప్రారంభం_పంక్తి() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(ట్యాగ్_2, ట్యాగ్_గమ్యం_String_Map,
                                               /*hasTagPrefix=*/అబద్ధం));
    ఎంపిక (integer) {
    స్థితి 0:
      sw->printString("Description", StringRef("No Specific Requirements"));
      ఆపు;
    స్థితి 1:
      sw->printString("Description", StringRef("AEABI Conformant"));
      ఆపు;
    అప్రమేయ:
      sw->printString("Description", StringRef("AEABI Non-Conformant"));
      ఆపు;
    }
  }
  తిరిగి Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "v6-style"};
  తిరిగి parseStringAttribute("CPU_unaligned_access", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::FP_HP_extension(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"If Available", "Permitted"};
  తిరిగి parseStringAttribute("FP_HP_extension", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  తిరిగి parseStringAttribute("ABI_FP_16bit_format", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::MPextension_use(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "Permitted"};
  తిరిగి parseStringAttribute("MPextension_use", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::DIV_use(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  తిరిగి parseStringAttribute("DIV_use", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::DSP_extension(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "Permitted"};
  తిరిగి parseStringAttribute("DSP_extension", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::T2EE_use(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted", "Permitted"};
  తిరిగి parseStringAttribute("T2EE_use", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::Virtualization_use(AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  తిరిగి parseStringAttribute("Virtualization_use", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  తిరిగి parseStringAttribute("PAC_extension", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  తిరిగి parseStringAttribute("BTI_extension", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Used", "Used"};
  తిరిగి parseStringAttribute("PACRET_use", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType ట్యాగ్_2) {
  స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి అక్షరశ్రేణులు[] = {"Not Used", "Used"};
  తిరిగి parseStringAttribute("BTI_use", ట్యాగ్_2, ArrayRef(అక్షరశ్రేణులు));
}

Error ARMAttributeParser::nodefaults(AttrType ట్యాగ్_2) {
  uint64_t విలువ_2 = de.getULEB128(cursor);
  printAttribute(ట్యాగ్_2, విలువ_2, "Unspecified Tags UNDEFINED");
  తిరిగి Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType ట్యాగ్_2) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_విలువ;

  SmallString<8> Description;
  raw_svector_ostream Desc_ప్రవాహం(Description);

  uint64_t Initial_స్థానభ్రంశం = cursor.tell();
  StringRef Raw_String_విలువ = de.getCStrRef(cursor);
  uint64_t Final_స్థానభ్రంశం = cursor.tell();
  cursor.seek(Initial_స్థానభ్రంశం);
  uint64_t Inner_ట్యాగ్ = de.getULEB128(cursor);

  తార్కిక Valid_Inner_ట్యాగ్ =
      any_యొక్క(ట్యాగ్_గమ్యం_String_Map, [Inner_ట్యాగ్](స్థిరరాశి ట్యాగ్_Name_అంశం &అంశం) {
        తిరిగి అంశం.attr == Inner_ట్యాగ్;
      });

  ఒకవేళ (!Valid_Inner_ట్యాగ్) {
    return_విలువ =
        createStringError(errc::వాదం_బహిర్గతం_యొక్క_domain,
                          Twine(Inner_ట్యాగ్) + " is not a valid tag number");
  } లేకపోతే {
    ఎంపిక (Inner_ట్యాగ్) {
    స్థితి ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_విలువ = de.getULEB128(cursor);
      స్వయంచాలక అక్షరశ్రేణులు = ArrayRef(CPU_arch_అక్షరశ్రేణులు);
      ఒకవేళ (Inner_విలువ >= అక్షరశ్రేణులు.కొలత_2()) {
        return_విలువ = createStringError(
            errc::వాదం_బహిర్గతం_యొక్క_domain,
            Twine(Inner_విలువ) + " is not a valid " +
                ELFAttrs::attrTypeAsString(Inner_ట్యాగ్, ట్యాగ్_గమ్యం_String_Map) +
                " value");
      } లేకపోతే {
        Desc_ప్రవాహం << ELFAttrs::attrTypeAsString(Inner_ట్యాగ్, ట్యాగ్_గమ్యం_String_Map)
                   << " = " << Inner_విలువ;
        ఒకవేళ (అక్షరశ్రేణులు[Inner_విలువ])
          Desc_ప్రవాహం << " (" << అక్షరశ్రేణులు[Inner_విలువ] << ')';
      }
      ఆపు;
    }
    స్థితి ARMBuildAttrs::also_compatible_with:
      return_విలువ = createStringError(
          errc::invalid_వాదం,
          ELFAttrs::attrTypeAsString(Inner_ట్యాగ్, ట్యాగ్_గమ్యం_String_Map) +
              " cannot be recursively defined");
      ఆపు;
    స్థితి ARMBuildAttrs::CPU_raw_name:
    స్థితి ARMBuildAttrs::CPU_name:
    స్థితి ARMBuildAttrs::compatibility:
    స్థితి ARMBuildAttrs::conformance: {
      StringRef Inner_విలువ = de.getCStrRef(cursor);
      Desc_ప్రవాహం << ELFAttrs::attrTypeAsString(Inner_ట్యాగ్, ట్యాగ్_గమ్యం_String_Map)
                 << " = " << Inner_విలువ;
      ఆపు;
    }
    అప్రమేయ: {
      uint64_t Inner_విలువ = de.getULEB128(cursor);
      Desc_ప్రవాహం << ELFAttrs::attrTypeAsString(Inner_ట్యాగ్, ట్యాగ్_గమ్యం_String_Map)
                 << " = " << Inner_విలువ;
    }
    }
  }

  సమితి_Attribute_String(ట్యాగ్_2, Raw_String_విలువ);
  ఒకవేళ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_సంఖ్య("Tag", ట్యాగ్_2);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(ట్యాగ్_2, ట్యాగ్_గమ్యం_String_Map, అబద్ధం));
    sw->printStringEscaped("Value", Raw_String_విలువ);
    ఒకవేళ (!Description.ఖాళీలు()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(Final_స్థానభ్రంశం);

  తిరిగి return_విలువ ? std::move(*return_విలువ) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t ట్యాగ్_2, తార్కిక &handled) {
  handled = అబద్ధం;
  కొరకు (స్థిరరాశి స్వయంచాలక &AH : displayRoutines) {
    ఒకవేళ (uint64_t(AH.attribute) == ట్యాగ్_2) {
      ఒకవేళ (Error e = (ఇది->*AH.routine)(static_cast<AttrType>(ట్యాగ్_2)))
        తిరిగి e;
      handled = నిజం;
      ఆపు;
    }
  }

  తిరిగి Error::success();
}
