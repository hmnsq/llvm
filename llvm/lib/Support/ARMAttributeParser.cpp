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

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::ARMBuildAttrs;

#परिभाषय ATTRIBUTE_HANDLER(attr)                                                \
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
        ATTRIBUTE_HANDLER(ABI_PCS_RW_दत्त),
        ATTRIBUTE_HANDLER(ABI_PCS_RO_दत्त),
        ATTRIBUTE_HANDLER(ABI_PCS_GOT_use),
        ATTRIBUTE_HANDLER(ABI_PCS_wchar_t),
        ATTRIBUTE_HANDLER(ABI_FP_rounding),
        ATTRIBUTE_HANDLER(ABI_FP_denormal),
        ATTRIBUTE_HANDLER(ABI_FP_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_user_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_सङ्ख्या_model),
        ATTRIBUTE_HANDLER(ABI_align_needed),
        ATTRIBUTE_HANDLER(ABI_align_preserved),
        ATTRIBUTE_HANDLER(ABI_enum_आकार),
        ATTRIBUTE_HANDLER(ABI_HardFP_use),
        ATTRIBUTE_HANDLER(ABI_VFP_तर्काः),
        ATTRIBUTE_HANDLER(ABI_WMMX_तर्काः),
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

#अपरिभाषय ATTRIBUTE_HANDLER

Error ARMAttributeParser::stringAttribute(AttrType चिह्नकम्_2) {
  StringRef चिह्नकम्_Name =
      ELFAttrs::attrTypeAsString(चिह्नकम्_2, चिह्नकम्_गन्तव्यम्_String_Map, /*hasTagPrefix=*/असत्य);
  StringRef desc = de.getCStrRef(cursor);

  यदि (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_सङ्ख्या("Tag", चिह्नकम्_2);
    यदि (!चिह्नकम्_Name.रिक्तानि())
      sw->printString("TagName", चिह्नकम्_Name);
    sw->printString("Value", desc);
  }
  प्रत्यागम Error::success();
}

स्थिर ध्रुव वर्ण *ध्रुव CPU_arch_वाक्यानि[] = {"Pre-v4",
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
                                               शून्यसूचकः,
                                               शून्यसूचकः,
                                               शून्यसूचकः,
                                               "ARM v8.1-M Mainline",
                                               "ARM v9-A"};

Error ARMAttributeParser::CPU_arch(AttrType चिह्नकम्_2) {
  प्रत्यागम parseStringAttribute("CPU_arch", चिह्नकम्_2, ArrayRef(CPU_arch_वाक्यानि));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType चिह्नकम्_2) {
  uint64_t मानम्_2 = de.getULEB128(cursor);

  StringRef profile;
  विकल्प (मानम्_2) {
  स्वभाव: profile = "Unknown"; भङ्ग;
  पक्ष 'A': profile = "Application"; भङ्ग;
  पक्ष 'R': profile = "Real-time"; भङ्ग;
  पक्ष 'M': profile = "Microcontroller"; भङ्ग;
  पक्ष 'S': profile = "Classic"; भङ्ग;
  पक्ष 0: profile = "None"; भङ्ग;
  }

  printAttribute(चिह्नकम्_2, मानम्_2, profile);
  प्रत्यागम Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "Permitted"};
  प्रत्यागम parseStringAttribute("ARM_ISA_use", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  प्रत्यागम parseStringAttribute("THUMB_ISA_use", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::FP_arch(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  प्रत्यागम parseStringAttribute("FP_arch", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::WMMX_arch(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  प्रत्यागम parseStringAttribute("WMMX_arch", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  प्रत्यागम parseStringAttribute("Advanced_SIMD_arch", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::MVE_arch(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  प्रत्यागम parseStringAttribute("MVE_arch", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::PCS_config(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  प्रत्यागम parseStringAttribute("PCS_config", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"v6", "Static Base", "TLS", "Unused"};
  प्रत्यागम parseStringAttribute("ABI_PCS_R9_use", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::ABI_PCS_RW_दत्त(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  प्रत्यागम parseStringAttribute("ABI_PCS_RW_data", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::ABI_PCS_RO_दत्त(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  प्रत्यागम parseStringAttribute("ABI_PCS_RO_data", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  प्रत्यागम parseStringAttribute("ABI_PCS_GOT_use", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  प्रत्यागम parseStringAttribute("ABI_PCS_wchar_t", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"IEEE-754", "Runtime"};
  प्रत्यागम parseStringAttribute("ABI_FP_rounding", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Unsupported", "IEEE-754", "Sign Only"};
  प्रत्यागम parseStringAttribute("ABI_FP_denormal", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "IEEE-754"};
  प्रत्यागम parseStringAttribute("ABI_FP_exceptions", चिह्नकम्_2, ArrayRef(वाक्यानि));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "IEEE-754"};
  प्रत्यागम parseStringAttribute("ABI_FP_user_exceptions", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::ABI_FP_सङ्ख्या_model(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  प्रत्यागम parseStringAttribute("ABI_FP_number_model", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::ABI_align_needed(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t मानम्_2 = de.getULEB128(cursor);

  std::string description;
  यदि (मानम्_2 < std::आकार_2(वाक्यानि))
    description = वाक्यानि[मानम्_2];
  अन्यथा यदि (मानम्_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << मानम्_2) +
                  "-byte extended alignment";
  अन्यथा
    description = "Invalid";

  printAttribute(चिह्नकम्_2, मानम्_2, description);
  प्रत्यागम Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *वाक्यानि[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t मानम्_2 = de.getULEB128(cursor);

  std::string description;
  यदि (मानम्_2 < std::आकार_2(वाक्यानि))
    description = std::string(वाक्यानि[मानम्_2]);
  अन्यथा यदि (मानम्_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << मानम्_2) + std::string("-byte data alignment");
  अन्यथा
    description = "Invalid";

  printAttribute(चिह्नकम्_2, मानम्_2, description);
  प्रत्यागम Error::success();
}

Error ARMAttributeParser::ABI_enum_आकार(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  प्रत्यागम parseStringAttribute("ABI_enum_size", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  प्रत्यागम parseStringAttribute("ABI_HardFP_use", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::ABI_VFP_तर्काः(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  प्रत्यागम parseStringAttribute("ABI_VFP_args", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::ABI_WMMX_तर्काः(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"AAPCS", "iWMMX", "Custom"};
  प्रत्यागम parseStringAttribute("ABI_WMMX_args", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  प्रत्यागम parseStringAttribute("ABI_optimization_goals", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  प्रत्यागम parseStringAttribute("ABI_FP_optimization_goals", चिह्नकम्_2,
                              ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::compatibility(AttrType चिह्नकम्_2) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  यदि (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_सङ्ख्या("Tag", चिह्नकम्_2);
    sw->start_पङ्क्तिः() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(चिह्नकम्_2, चिह्नकम्_गन्तव्यम्_String_Map,
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

Error ARMAttributeParser::CPU_unaligned_access(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "v6-style"};
  प्रत्यागम parseStringAttribute("CPU_unaligned_access", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::FP_HP_extension(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"If Available", "Permitted"};
  प्रत्यागम parseStringAttribute("FP_HP_extension", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  प्रत्यागम parseStringAttribute("ABI_FP_16bit_format", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::MPextension_use(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "Permitted"};
  प्रत्यागम parseStringAttribute("MPextension_use", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::DIV_use(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  प्रत्यागम parseStringAttribute("DIV_use", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::DSP_extension(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "Permitted"};
  प्रत्यागम parseStringAttribute("DSP_extension", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::T2EE_use(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted", "Permitted"};
  प्रत्यागम parseStringAttribute("T2EE_use", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::Virtualization_use(AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  प्रत्यागम parseStringAttribute("Virtualization_use", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  प्रत्यागम parseStringAttribute("PAC_extension", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  प्रत्यागम parseStringAttribute("BTI_extension", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Used", "Used"};
  प्रत्यागम parseStringAttribute("PACRET_use", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType चिह्नकम्_2) {
  स्थिर ध्रुव वर्ण *ध्रुव वाक्यानि[] = {"Not Used", "Used"};
  प्रत्यागम parseStringAttribute("BTI_use", चिह्नकम्_2, ArrayRef(वाक्यानि));
}

Error ARMAttributeParser::nodefaults(AttrType चिह्नकम्_2) {
  uint64_t मानम्_2 = de.getULEB128(cursor);
  printAttribute(चिह्नकम्_2, मानम्_2, "Unspecified Tags UNDEFINED");
  प्रत्यागम Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType चिह्नकम्_2) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_मानम्;

  SmallString<8> Description;
  raw_svector_ostream Desc_प्रवाहः(Description);

  uint64_t Initial_विस्थापनम् = cursor.tell();
  StringRef Raw_String_मानम् = de.getCStrRef(cursor);
  uint64_t Final_विस्थापनम् = cursor.tell();
  cursor.seek(Initial_विस्थापनम्);
  uint64_t Inner_चिह्नकम् = de.getULEB128(cursor);

  तार्किक Valid_Inner_चिह्नकम् =
      any_सम्बन्धि(चिह्नकम्_गन्तव्यम्_String_Map, [Inner_चिह्नकम्](ध्रुव चिह्नकम्_Name_Item &Item) {
        प्रत्यागम Item.attr == Inner_चिह्नकम्;
      });

  यदि (!Valid_Inner_चिह्नकम्) {
    return_मानम् =
        createStringError(errc::तर्कः_बहिर्गमनम्_सम्बन्धि_domain,
                          Twine(Inner_चिह्नकम्) + " is not a valid tag number");
  } अन्यथा {
    विकल्प (Inner_चिह्नकम्) {
    पक्ष ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_मानम् = de.getULEB128(cursor);
      स्वचालित वाक्यानि = ArrayRef(CPU_arch_वाक्यानि);
      यदि (Inner_मानम् >= वाक्यानि.आकार_2()) {
        return_मानम् = createStringError(
            errc::तर्कः_बहिर्गमनम्_सम्बन्धि_domain,
            Twine(Inner_मानम्) + " is not a valid " +
                ELFAttrs::attrTypeAsString(Inner_चिह्नकम्, चिह्नकम्_गन्तव्यम्_String_Map) +
                " value");
      } अन्यथा {
        Desc_प्रवाहः << ELFAttrs::attrTypeAsString(Inner_चिह्नकम्, चिह्नकम्_गन्तव्यम्_String_Map)
                   << " = " << Inner_मानम्;
        यदि (वाक्यानि[Inner_मानम्])
          Desc_प्रवाहः << " (" << वाक्यानि[Inner_मानम्] << ')';
      }
      भङ्ग;
    }
    पक्ष ARMBuildAttrs::also_compatible_with:
      return_मानम् = createStringError(
          errc::invalid_तर्कः,
          ELFAttrs::attrTypeAsString(Inner_चिह्नकम्, चिह्नकम्_गन्तव्यम्_String_Map) +
              " cannot be recursively defined");
      भङ्ग;
    पक्ष ARMBuildAttrs::CPU_raw_name:
    पक्ष ARMBuildAttrs::CPU_name:
    पक्ष ARMBuildAttrs::compatibility:
    पक्ष ARMBuildAttrs::conformance: {
      StringRef Inner_मानम् = de.getCStrRef(cursor);
      Desc_प्रवाहः << ELFAttrs::attrTypeAsString(Inner_चिह्नकम्, चिह्नकम्_गन्तव्यम्_String_Map)
                 << " = " << Inner_मानम्;
      भङ्ग;
    }
    स्वभाव: {
      uint64_t Inner_मानम् = de.getULEB128(cursor);
      Desc_प्रवाहः << ELFAttrs::attrTypeAsString(Inner_चिह्नकम्, चिह्नकम्_गन्तव्यम्_String_Map)
                 << " = " << Inner_मानम्;
    }
    }
  }

  समुच्चयः_Attribute_String(चिह्नकम्_2, Raw_String_मानम्);
  यदि (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_सङ्ख्या("Tag", चिह्नकम्_2);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(चिह्नकम्_2, चिह्नकम्_गन्तव्यम्_String_Map, असत्य));
    sw->printStringEscaped("Value", Raw_String_मानम्);
    यदि (!Description.रिक्तानि()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(Final_विस्थापनम्);

  प्रत्यागम return_मानम् ? std::move(*return_मानम्) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t चिह्नकम्_2, तार्किक &handled) {
  handled = असत्य;
  प्रति (ध्रुव स्वचालित &AH : displayRoutines) {
    यदि (uint64_t(AH.attribute) == चिह्नकम्_2) {
      यदि (Error e = (एतत्->*AH.routine)(static_cast<AttrType>(चिह्नकम्_2)))
        प्रत्यागम e;
      handled = सत्य;
      भङ्ग;
    }
  }

  प्रत्यागम Error::success();
}
