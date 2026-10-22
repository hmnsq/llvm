//===- ARMAttributeParser.cpp - ARM Attribute Information Printer ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/ARMAttributeParser.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/ARMBuildAttributes.h"
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/ScopedPrinter.h"
#ضمن <optional>

استعمال نطاق llvm;
استعمال نطاق llvm::ARMBuildAttrs;

#وصف_ڪريو ATTRIBUTE_HANDLER(attr)                                                \
  { ARMBuildAttrs::attr, &ARMAttributeParser::attr }

ثابت ARMAttributeParser::DisplayHandler ARMAttributeParser::displayRoutines[] =
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
        ATTRIBUTE_HANDLER(ABI_PCS_RW_معلومات),
        ATTRIBUTE_HANDLER(ABI_PCS_RO_معلومات),
        ATTRIBUTE_HANDLER(ABI_PCS_GOT_use),
        ATTRIBUTE_HANDLER(ABI_PCS_wchar_t),
        ATTRIBUTE_HANDLER(ABI_FP_rounding),
        ATTRIBUTE_HANDLER(ABI_FP_denormal),
        ATTRIBUTE_HANDLER(ABI_FP_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_user_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_عدد_model),
        ATTRIBUTE_HANDLER(ABI_align_needed),
        ATTRIBUTE_HANDLER(ABI_align_preserved),
        ATTRIBUTE_HANDLER(ABI_enum_سائيز),
        ATTRIBUTE_HANDLER(ABI_HardFP_use),
        ATTRIBUTE_HANDLER(ABI_VFP_دليلون),
        ATTRIBUTE_HANDLER(ABI_WMMX_دليلون),
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

#وصف_رد ATTRIBUTE_HANDLER

Error ARMAttributeParser::stringAttribute(AttrType ٽيگ_2) {
  StringRef ٽيگ_Name =
      ELFAttrs::attrTypeAsString(ٽيگ_2, ٽيگ_منزل_String_Map, /*hasTagPrefix=*/خطأ);
  StringRef desc = de.getCStrRef(cursor);

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_عدد("Tag", ٽيگ_2);
    إذا (!ٽيگ_Name.خالي_جايون())
      sw->printString("TagName", ٽيگ_Name);
    sw->printString("Value", desc);
  }
  أرجع Error::success();
}

ساكن ثابت حرف *ثابت CPU_arch_لڙيون[] = {"Pre-v4",
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
                                               خالي_اشارو,
                                               خالي_اشارو,
                                               خالي_اشارو,
                                               "ARM v8.1-M Mainline",
                                               "ARM v9-A"};

Error ARMAttributeParser::CPU_arch(AttrType ٽيگ_2) {
  أرجع parseStringAttribute("CPU_arch", ٽيگ_2, ArrayRef(CPU_arch_لڙيون));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType ٽيگ_2) {
  uint64_t قدر_2 = de.getULEB128(cursor);

  StringRef profile;
  بدل (قدر_2) {
  افتراضي: profile = "Unknown"; اكسر;
  حالة 'A': profile = "Application"; اكسر;
  حالة 'R': profile = "Real-time"; اكسر;
  حالة 'M': profile = "Microcontroller"; اكسر;
  حالة 'S': profile = "Classic"; اكسر;
  حالة 0: profile = "None"; اكسر;
  }

  printAttribute(ٽيگ_2, قدر_2, profile);
  أرجع Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "Permitted"};
  أرجع parseStringAttribute("ARM_ISA_use", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  أرجع parseStringAttribute("THUMB_ISA_use", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::FP_arch(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  أرجع parseStringAttribute("FP_arch", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::WMMX_arch(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  أرجع parseStringAttribute("WMMX_arch", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  أرجع parseStringAttribute("Advanced_SIMD_arch", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::MVE_arch(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  أرجع parseStringAttribute("MVE_arch", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::PCS_config(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  أرجع parseStringAttribute("PCS_config", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"v6", "Static Base", "TLS", "Unused"};
  أرجع parseStringAttribute("ABI_PCS_R9_use", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::ABI_PCS_RW_معلومات(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  أرجع parseStringAttribute("ABI_PCS_RW_data", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::ABI_PCS_RO_معلومات(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  أرجع parseStringAttribute("ABI_PCS_RO_data", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  أرجع parseStringAttribute("ABI_PCS_GOT_use", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  أرجع parseStringAttribute("ABI_PCS_wchar_t", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"IEEE-754", "Runtime"};
  أرجع parseStringAttribute("ABI_FP_rounding", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Unsupported", "IEEE-754", "Sign Only"};
  أرجع parseStringAttribute("ABI_FP_denormal", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "IEEE-754"};
  أرجع parseStringAttribute("ABI_FP_exceptions", ٽيگ_2, ArrayRef(لڙيون));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "IEEE-754"};
  أرجع parseStringAttribute("ABI_FP_user_exceptions", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::ABI_FP_عدد_model(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  أرجع parseStringAttribute("ABI_FP_number_model", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::ABI_align_needed(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t قدر_2 = de.getULEB128(cursor);

  std::string description;
  إذا (قدر_2 < std::سائيز_2(لڙيون))
    description = لڙيون[قدر_2];
  وإلا إذا (قدر_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << قدر_2) +
                  "-byte extended alignment";
  وإلا
    description = "Invalid";

  printAttribute(ٽيگ_2, قدر_2, description);
  أرجع Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType ٽيگ_2) {
  ساكن ثابت حرف *لڙيون[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t قدر_2 = de.getULEB128(cursor);

  std::string description;
  إذا (قدر_2 < std::سائيز_2(لڙيون))
    description = std::string(لڙيون[قدر_2]);
  وإلا إذا (قدر_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << قدر_2) + std::string("-byte data alignment");
  وإلا
    description = "Invalid";

  printAttribute(ٽيگ_2, قدر_2, description);
  أرجع Error::success();
}

Error ARMAttributeParser::ABI_enum_سائيز(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  أرجع parseStringAttribute("ABI_enum_size", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  أرجع parseStringAttribute("ABI_HardFP_use", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::ABI_VFP_دليلون(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  أرجع parseStringAttribute("ABI_VFP_args", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::ABI_WMMX_دليلون(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"AAPCS", "iWMMX", "Custom"};
  أرجع parseStringAttribute("ABI_WMMX_args", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  أرجع parseStringAttribute("ABI_optimization_goals", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  أرجع parseStringAttribute("ABI_FP_optimization_goals", ٽيگ_2,
                              ArrayRef(لڙيون));
}

Error ARMAttributeParser::compatibility(AttrType ٽيگ_2) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_عدد("Tag", ٽيگ_2);
    sw->شروعات_سٽ() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(ٽيگ_2, ٽيگ_منزل_String_Map,
                                               /*hasTagPrefix=*/خطأ));
    بدل (integer) {
    حالة 0:
      sw->printString("Description", StringRef("No Specific Requirements"));
      اكسر;
    حالة 1:
      sw->printString("Description", StringRef("AEABI Conformant"));
      اكسر;
    افتراضي:
      sw->printString("Description", StringRef("AEABI Non-Conformant"));
      اكسر;
    }
  }
  أرجع Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "v6-style"};
  أرجع parseStringAttribute("CPU_unaligned_access", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::FP_HP_extension(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"If Available", "Permitted"};
  أرجع parseStringAttribute("FP_HP_extension", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  أرجع parseStringAttribute("ABI_FP_16bit_format", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::MPextension_use(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "Permitted"};
  أرجع parseStringAttribute("MPextension_use", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::DIV_use(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  أرجع parseStringAttribute("DIV_use", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::DSP_extension(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "Permitted"};
  أرجع parseStringAttribute("DSP_extension", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::T2EE_use(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted", "Permitted"};
  أرجع parseStringAttribute("T2EE_use", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::Virtualization_use(AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  أرجع parseStringAttribute("Virtualization_use", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  أرجع parseStringAttribute("PAC_extension", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  أرجع parseStringAttribute("BTI_extension", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Used", "Used"};
  أرجع parseStringAttribute("PACRET_use", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType ٽيگ_2) {
  ساكن ثابت حرف *ثابت لڙيون[] = {"Not Used", "Used"};
  أرجع parseStringAttribute("BTI_use", ٽيگ_2, ArrayRef(لڙيون));
}

Error ARMAttributeParser::nodefaults(AttrType ٽيگ_2) {
  uint64_t قدر_2 = de.getULEB128(cursor);
  printAttribute(ٽيگ_2, قدر_2, "Unspecified Tags UNDEFINED");
  أرجع Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType ٽيگ_2) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_قدر;

  SmallString<8> Description;
  raw_svector_ostream Desc_وهڪرو(Description);

  uint64_t Initial_وٿي = cursor.tell();
  StringRef Raw_String_قدر = de.getCStrRef(cursor);
  uint64_t Final_وٿي = cursor.tell();
  cursor.seek(Initial_وٿي);
  uint64_t Inner_ٽيگ = de.getULEB128(cursor);

  منطقي Valid_Inner_ٽيگ =
      any_جو(ٽيگ_منزل_String_Map, [Inner_ٽيگ](ثابت ٽيگ_Name_شي &شي) {
        أرجع شي.attr == Inner_ٽيگ;
      });

  إذا (!Valid_Inner_ٽيگ) {
    return_قدر =
        createStringError(errc::دليل_ٻاهريون_جو_domain,
                          Twine(Inner_ٽيگ) + " is not a valid tag number");
  } وإلا {
    بدل (Inner_ٽيگ) {
    حالة ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_قدر = de.getULEB128(cursor);
      خودڪار لڙيون = ArrayRef(CPU_arch_لڙيون);
      إذا (Inner_قدر >= لڙيون.سائيز_2()) {
        return_قدر = createStringError(
            errc::دليل_ٻاهريون_جو_domain,
            Twine(Inner_قدر) + " is not a valid " +
                ELFAttrs::attrTypeAsString(Inner_ٽيگ, ٽيگ_منزل_String_Map) +
                " value");
      } وإلا {
        Desc_وهڪرو << ELFAttrs::attrTypeAsString(Inner_ٽيگ, ٽيگ_منزل_String_Map)
                   << " = " << Inner_قدر;
        إذا (لڙيون[Inner_قدر])
          Desc_وهڪرو << " (" << لڙيون[Inner_قدر] << ')';
      }
      اكسر;
    }
    حالة ARMBuildAttrs::also_compatible_with:
      return_قدر = createStringError(
          errc::invalid_دليل,
          ELFAttrs::attrTypeAsString(Inner_ٽيگ, ٽيگ_منزل_String_Map) +
              " cannot be recursively defined");
      اكسر;
    حالة ARMBuildAttrs::CPU_raw_name:
    حالة ARMBuildAttrs::CPU_name:
    حالة ARMBuildAttrs::compatibility:
    حالة ARMBuildAttrs::conformance: {
      StringRef Inner_قدر = de.getCStrRef(cursor);
      Desc_وهڪرو << ELFAttrs::attrTypeAsString(Inner_ٽيگ, ٽيگ_منزل_String_Map)
                 << " = " << Inner_قدر;
      اكسر;
    }
    افتراضي: {
      uint64_t Inner_قدر = de.getULEB128(cursor);
      Desc_وهڪرو << ELFAttrs::attrTypeAsString(Inner_ٽيگ, ٽيگ_منزل_String_Map)
                 << " = " << Inner_قدر;
    }
    }
  }

  سيٽ_Attribute_String(ٽيگ_2, Raw_String_قدر);
  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_عدد("Tag", ٽيگ_2);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(ٽيگ_2, ٽيگ_منزل_String_Map, خطأ));
    sw->printStringEscaped("Value", Raw_String_قدر);
    إذا (!Description.خالي_جايون()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(Final_وٿي);

  أرجع return_قدر ? std::move(*return_قدر) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t ٽيگ_2, منطقي &handled) {
  handled = خطأ;
  لكل (ثابت خودڪار &AH : displayRoutines) {
    إذا (uint64_t(AH.attribute) == ٽيگ_2) {
      إذا (Error e = (هذا->*AH.routine)(static_cast<AttrType>(ٽيگ_2)))
        أرجع e;
      handled = سچ;
      اكسر;
    }
  }

  أرجع Error::success();
}
