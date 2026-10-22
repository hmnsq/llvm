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

ورتوں نطاق llvm;
ورتوں نطاق llvm::ARMBuildAttrs;

#تعریف_کرو ATTRIBUTE_HANDLER(attr)                                                \
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
        ATTRIBUTE_HANDLER(ABI_PCS_RW_انکڑے),
        ATTRIBUTE_HANDLER(ABI_PCS_RO_انکڑے),
        ATTRIBUTE_HANDLER(ABI_PCS_GOT_use),
        ATTRIBUTE_HANDLER(ABI_PCS_wchar_t),
        ATTRIBUTE_HANDLER(ABI_FP_rounding),
        ATTRIBUTE_HANDLER(ABI_FP_denormal),
        ATTRIBUTE_HANDLER(ABI_FP_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_user_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_نمبر_model),
        ATTRIBUTE_HANDLER(ABI_align_needed),
        ATTRIBUTE_HANDLER(ABI_align_preserved),
        ATTRIBUTE_HANDLER(ABI_enum_ناپ),
        ATTRIBUTE_HANDLER(ABI_HardFP_use),
        ATTRIBUTE_HANDLER(ABI_VFP_دلیلاں),
        ATTRIBUTE_HANDLER(ABI_WMMX_دلیلاں),
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

#تعریف_رد ATTRIBUTE_HANDLER

Error ARMAttributeParser::stringAttribute(AttrType ٹیگ_2) {
  StringRef ٹیگ_Name =
      ELFAttrs::attrTypeAsString(ٹیگ_2, ٹیگ_منزل_String_Map, /*hasTagPrefix=*/خطأ);
  StringRef desc = de.getCStrRef(cursor);

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_نمبر("Tag", ٹیگ_2);
    إذا (!ٹیگ_Name.خالی_تھاواں())
      sw->printString("TagName", ٹیگ_Name);
    sw->printString("Value", desc);
  }
  أرجع Error::success();
}

ساكن ثابت حرف *ثابت CPU_arch_لڑیاں[] = {"Pre-v4",
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
                                               خالی_سوچک,
                                               خالی_سوچک,
                                               خالی_سوچک,
                                               "ARM v8.1-M Mainline",
                                               "ARM v9-A"};

Error ARMAttributeParser::CPU_arch(AttrType ٹیگ_2) {
  أرجع parseStringAttribute("CPU_arch", ٹیگ_2, ArrayRef(CPU_arch_لڑیاں));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType ٹیگ_2) {
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

  printAttribute(ٹیگ_2, قدر_2, profile);
  أرجع Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Permitted"};
  أرجع parseStringAttribute("ARM_ISA_use", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  أرجع parseStringAttribute("THUMB_ISA_use", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::FP_arch(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  أرجع parseStringAttribute("FP_arch", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::WMMX_arch(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  أرجع parseStringAttribute("WMMX_arch", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  أرجع parseStringAttribute("Advanced_SIMD_arch", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::MVE_arch(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  أرجع parseStringAttribute("MVE_arch", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::PCS_config(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  أرجع parseStringAttribute("PCS_config", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"v6", "Static Base", "TLS", "Unused"};
  أرجع parseStringAttribute("ABI_PCS_R9_use", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_PCS_RW_انکڑے(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  أرجع parseStringAttribute("ABI_PCS_RW_data", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_PCS_RO_انکڑے(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  أرجع parseStringAttribute("ABI_PCS_RO_data", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  أرجع parseStringAttribute("ABI_PCS_GOT_use", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  أرجع parseStringAttribute("ABI_PCS_wchar_t", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"IEEE-754", "Runtime"};
  أرجع parseStringAttribute("ABI_FP_rounding", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Unsupported", "IEEE-754", "Sign Only"};
  أرجع parseStringAttribute("ABI_FP_denormal", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "IEEE-754"};
  أرجع parseStringAttribute("ABI_FP_exceptions", ٹیگ_2, ArrayRef(لڑیاں));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "IEEE-754"};
  أرجع parseStringAttribute("ABI_FP_user_exceptions", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_FP_نمبر_model(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  أرجع parseStringAttribute("ABI_FP_number_model", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_align_needed(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t قدر_2 = de.getULEB128(cursor);

  std::string description;
  إذا (قدر_2 < std::ناپ_2(لڑیاں))
    description = لڑیاں[قدر_2];
  وإلا إذا (قدر_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << قدر_2) +
                  "-byte extended alignment";
  وإلا
    description = "Invalid";

  printAttribute(ٹیگ_2, قدر_2, description);
  أرجع Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType ٹیگ_2) {
  ساكن ثابت حرف *لڑیاں[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t قدر_2 = de.getULEB128(cursor);

  std::string description;
  إذا (قدر_2 < std::ناپ_2(لڑیاں))
    description = std::string(لڑیاں[قدر_2]);
  وإلا إذا (قدر_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << قدر_2) + std::string("-byte data alignment");
  وإلا
    description = "Invalid";

  printAttribute(ٹیگ_2, قدر_2, description);
  أرجع Error::success();
}

Error ARMAttributeParser::ABI_enum_ناپ(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  أرجع parseStringAttribute("ABI_enum_size", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  أرجع parseStringAttribute("ABI_HardFP_use", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_VFP_دلیلاں(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  أرجع parseStringAttribute("ABI_VFP_args", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_WMMX_دلیلاں(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"AAPCS", "iWMMX", "Custom"};
  أرجع parseStringAttribute("ABI_WMMX_args", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  أرجع parseStringAttribute("ABI_optimization_goals", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  أرجع parseStringAttribute("ABI_FP_optimization_goals", ٹیگ_2,
                              ArrayRef(لڑیاں));
}

Error ARMAttributeParser::compatibility(AttrType ٹیگ_2) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_نمبر("Tag", ٹیگ_2);
    sw->شروع_سطر() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(ٹیگ_2, ٹیگ_منزل_String_Map,
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

Error ARMAttributeParser::CPU_unaligned_access(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "v6-style"};
  أرجع parseStringAttribute("CPU_unaligned_access", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::FP_HP_extension(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"If Available", "Permitted"};
  أرجع parseStringAttribute("FP_HP_extension", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  أرجع parseStringAttribute("ABI_FP_16bit_format", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::MPextension_use(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Permitted"};
  أرجع parseStringAttribute("MPextension_use", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::DIV_use(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  أرجع parseStringAttribute("DIV_use", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::DSP_extension(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Permitted"};
  أرجع parseStringAttribute("DSP_extension", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::T2EE_use(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Permitted"};
  أرجع parseStringAttribute("T2EE_use", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::Virtualization_use(AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  أرجع parseStringAttribute("Virtualization_use", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  أرجع parseStringAttribute("PAC_extension", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  أرجع parseStringAttribute("BTI_extension", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Used", "Used"};
  أرجع parseStringAttribute("PACRET_use", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType ٹیگ_2) {
  ساكن ثابت حرف *ثابت لڑیاں[] = {"Not Used", "Used"};
  أرجع parseStringAttribute("BTI_use", ٹیگ_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::nodefaults(AttrType ٹیگ_2) {
  uint64_t قدر_2 = de.getULEB128(cursor);
  printAttribute(ٹیگ_2, قدر_2, "Unspecified Tags UNDEFINED");
  أرجع Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType ٹیگ_2) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_قدر;

  SmallString<8> Description;
  raw_svector_ostream Desc_دھارا(Description);

  uint64_t Initial_وتھ = cursor.tell();
  StringRef Raw_String_قدر = de.getCStrRef(cursor);
  uint64_t Final_وتھ = cursor.tell();
  cursor.seek(Initial_وتھ);
  uint64_t Inner_ٹیگ = de.getULEB128(cursor);

  منطقي Valid_Inner_ٹیگ =
      any_دا(ٹیگ_منزل_String_Map, [Inner_ٹیگ](ثابت ٹیگ_Name_شے &شے) {
        أرجع شے.attr == Inner_ٹیگ;
      });

  إذا (!Valid_Inner_ٹیگ) {
    return_قدر =
        createStringError(errc::دلیل_باہرلا_دا_domain,
                          Twine(Inner_ٹیگ) + " is not a valid tag number");
  } وإلا {
    بدل (Inner_ٹیگ) {
    حالة ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_قدر = de.getULEB128(cursor);
      خودکار لڑیاں = ArrayRef(CPU_arch_لڑیاں);
      إذا (Inner_قدر >= لڑیاں.ناپ_2()) {
        return_قدر = createStringError(
            errc::دلیل_باہرلا_دا_domain,
            Twine(Inner_قدر) + " is not a valid " +
                ELFAttrs::attrTypeAsString(Inner_ٹیگ, ٹیگ_منزل_String_Map) +
                " value");
      } وإلا {
        Desc_دھارا << ELFAttrs::attrTypeAsString(Inner_ٹیگ, ٹیگ_منزل_String_Map)
                   << " = " << Inner_قدر;
        إذا (لڑیاں[Inner_قدر])
          Desc_دھارا << " (" << لڑیاں[Inner_قدر] << ')';
      }
      اكسر;
    }
    حالة ARMBuildAttrs::also_compatible_with:
      return_قدر = createStringError(
          errc::invalid_دلیل,
          ELFAttrs::attrTypeAsString(Inner_ٹیگ, ٹیگ_منزل_String_Map) +
              " cannot be recursively defined");
      اكسر;
    حالة ARMBuildAttrs::CPU_raw_name:
    حالة ARMBuildAttrs::CPU_name:
    حالة ARMBuildAttrs::compatibility:
    حالة ARMBuildAttrs::conformance: {
      StringRef Inner_قدر = de.getCStrRef(cursor);
      Desc_دھارا << ELFAttrs::attrTypeAsString(Inner_ٹیگ, ٹیگ_منزل_String_Map)
                 << " = " << Inner_قدر;
      اكسر;
    }
    افتراضي: {
      uint64_t Inner_قدر = de.getULEB128(cursor);
      Desc_دھارا << ELFAttrs::attrTypeAsString(Inner_ٹیگ, ٹیگ_منزل_String_Map)
                 << " = " << Inner_قدر;
    }
    }
  }

  سیٹ_Attribute_String(ٹیگ_2, Raw_String_قدر);
  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_نمبر("Tag", ٹیگ_2);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(ٹیگ_2, ٹیگ_منزل_String_Map, خطأ));
    sw->printStringEscaped("Value", Raw_String_قدر);
    إذا (!Description.خالی_تھاواں()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(Final_وتھ);

  أرجع return_قدر ? std::move(*return_قدر) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t ٹیگ_2, منطقي &handled) {
  handled = خطأ;
  لكل (ثابت خودکار &AH : displayRoutines) {
    إذا (uint64_t(AH.attribute) == ٹیگ_2) {
      إذا (Error e = (هذا->*AH.routine)(static_cast<AttrType>(ٹیگ_2)))
        أرجع e;
      handled = سچ;
      اكسر;
    }
  }

  أرجع Error::success();
}
