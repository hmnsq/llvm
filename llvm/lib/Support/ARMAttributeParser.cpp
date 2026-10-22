//===- ARMAttributeParser.cpp - ARM Attribute Information Printer ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/ARMAttributeParser.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/ARMBuildAttributes.h"
#شامل "llvm/Support/Errc.h"
#شامل "llvm/Support/ScopedPrinter.h"
#شامل <optional>

استعمال نامخلا llvm;
استعمال نامخلا llvm::ARMBuildAttrs;

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
        ATTRIBUTE_HANDLER(ABI_PCS_RW_ڈیٹا),
        ATTRIBUTE_HANDLER(ABI_PCS_RO_ڈیٹا),
        ATTRIBUTE_HANDLER(ABI_PCS_GOT_use),
        ATTRIBUTE_HANDLER(ABI_PCS_wchar_t),
        ATTRIBUTE_HANDLER(ABI_FP_rounding),
        ATTRIBUTE_HANDLER(ABI_FP_denormal),
        ATTRIBUTE_HANDLER(ABI_FP_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_user_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_نمبر_model),
        ATTRIBUTE_HANDLER(ABI_align_needed),
        ATTRIBUTE_HANDLER(ABI_align_preserved),
        ATTRIBUTE_HANDLER(ABI_enum_سائز),
        ATTRIBUTE_HANDLER(ABI_HardFP_use),
        ATTRIBUTE_HANDLER(ABI_VFP_دلائل),
        ATTRIBUTE_HANDLER(ABI_WMMX_دلائل),
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

#تعریف_منسوخ ATTRIBUTE_HANDLER

Error ARMAttributeParser::stringAttribute(AttrType لیبل_2) {
  StringRef لیبل_Name =
      ELFAttrs::attrTypeAsString(لیبل_2, لیبل_مقصد_String_Map, /*hasTagPrefix=*/جھوٹ);
  StringRef desc = de.getCStrRef(cursor);

  اگر (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_نمبر("Tag", لیبل_2);
    اگر (!لیبل_Name.خالی_خانے())
      sw->printString("TagName", لیبل_Name);
    sw->printString("Value", desc);
  }
  واپس Error::success();
}

ساکن ثابت حرف *ثابت CPU_arch_لڑیاں[] = {"Pre-v4",
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
                                               خالی_اشاریہ,
                                               خالی_اشاریہ,
                                               خالی_اشاریہ,
                                               "ARM v8.1-M Mainline",
                                               "ARM v9-A"};

Error ARMAttributeParser::CPU_arch(AttrType لیبل_2) {
  واپس parseStringAttribute("CPU_arch", لیبل_2, ArrayRef(CPU_arch_لڑیاں));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType لیبل_2) {
  uint64_t قدر_2 = de.getULEB128(cursor);

  StringRef profile;
  انتخاب (قدر_2) {
  طےشدہ: profile = "Unknown"; توڑو;
  حالت 'A': profile = "Application"; توڑو;
  حالت 'R': profile = "Real-time"; توڑو;
  حالت 'M': profile = "Microcontroller"; توڑو;
  حالت 'S': profile = "Classic"; توڑو;
  حالت 0: profile = "None"; توڑو;
  }

  printAttribute(لیبل_2, قدر_2, profile);
  واپس Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Permitted"};
  واپس parseStringAttribute("ARM_ISA_use", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  واپس parseStringAttribute("THUMB_ISA_use", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::FP_arch(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  واپس parseStringAttribute("FP_arch", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::WMMX_arch(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  واپس parseStringAttribute("WMMX_arch", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  واپس parseStringAttribute("Advanced_SIMD_arch", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::MVE_arch(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  واپس parseStringAttribute("MVE_arch", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::PCS_config(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  واپس parseStringAttribute("PCS_config", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"v6", "Static Base", "TLS", "Unused"};
  واپس parseStringAttribute("ABI_PCS_R9_use", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_PCS_RW_ڈیٹا(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  واپس parseStringAttribute("ABI_PCS_RW_data", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_PCS_RO_ڈیٹا(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  واپس parseStringAttribute("ABI_PCS_RO_data", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  واپس parseStringAttribute("ABI_PCS_GOT_use", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  واپس parseStringAttribute("ABI_PCS_wchar_t", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"IEEE-754", "Runtime"};
  واپس parseStringAttribute("ABI_FP_rounding", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Unsupported", "IEEE-754", "Sign Only"};
  واپس parseStringAttribute("ABI_FP_denormal", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "IEEE-754"};
  واپس parseStringAttribute("ABI_FP_exceptions", لیبل_2, ArrayRef(لڑیاں));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "IEEE-754"};
  واپس parseStringAttribute("ABI_FP_user_exceptions", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_FP_نمبر_model(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  واپس parseStringAttribute("ABI_FP_number_model", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_align_needed(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t قدر_2 = de.getULEB128(cursor);

  std::string description;
  اگر (قدر_2 < std::سائز_2(لڑیاں))
    description = لڑیاں[قدر_2];
  ورنہ اگر (قدر_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << قدر_2) +
                  "-byte extended alignment";
  ورنہ
    description = "Invalid";

  printAttribute(لیبل_2, قدر_2, description);
  واپس Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType لیبل_2) {
  ساکن ثابت حرف *لڑیاں[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t قدر_2 = de.getULEB128(cursor);

  std::string description;
  اگر (قدر_2 < std::سائز_2(لڑیاں))
    description = std::string(لڑیاں[قدر_2]);
  ورنہ اگر (قدر_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << قدر_2) + std::string("-byte data alignment");
  ورنہ
    description = "Invalid";

  printAttribute(لیبل_2, قدر_2, description);
  واپس Error::success();
}

Error ARMAttributeParser::ABI_enum_سائز(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  واپس parseStringAttribute("ABI_enum_size", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  واپس parseStringAttribute("ABI_HardFP_use", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_VFP_دلائل(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  واپس parseStringAttribute("ABI_VFP_args", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_WMMX_دلائل(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"AAPCS", "iWMMX", "Custom"};
  واپس parseStringAttribute("ABI_WMMX_args", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  واپس parseStringAttribute("ABI_optimization_goals", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  واپس parseStringAttribute("ABI_FP_optimization_goals", لیبل_2,
                              ArrayRef(لڑیاں));
}

Error ARMAttributeParser::compatibility(AttrType لیبل_2) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  اگر (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_نمبر("Tag", لیبل_2);
    sw->آغاز_سطر() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(لیبل_2, لیبل_مقصد_String_Map,
                                               /*hasTagPrefix=*/جھوٹ));
    انتخاب (integer) {
    حالت 0:
      sw->printString("Description", StringRef("No Specific Requirements"));
      توڑو;
    حالت 1:
      sw->printString("Description", StringRef("AEABI Conformant"));
      توڑو;
    طےشدہ:
      sw->printString("Description", StringRef("AEABI Non-Conformant"));
      توڑو;
    }
  }
  واپس Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "v6-style"};
  واپس parseStringAttribute("CPU_unaligned_access", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::FP_HP_extension(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"If Available", "Permitted"};
  واپس parseStringAttribute("FP_HP_extension", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  واپس parseStringAttribute("ABI_FP_16bit_format", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::MPextension_use(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Permitted"};
  واپس parseStringAttribute("MPextension_use", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::DIV_use(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  واپس parseStringAttribute("DIV_use", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::DSP_extension(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Permitted"};
  واپس parseStringAttribute("DSP_extension", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::T2EE_use(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted", "Permitted"};
  واپس parseStringAttribute("T2EE_use", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::Virtualization_use(AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  واپس parseStringAttribute("Virtualization_use", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  واپس parseStringAttribute("PAC_extension", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  واپس parseStringAttribute("BTI_extension", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Used", "Used"};
  واپس parseStringAttribute("PACRET_use", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType لیبل_2) {
  ساکن ثابت حرف *ثابت لڑیاں[] = {"Not Used", "Used"};
  واپس parseStringAttribute("BTI_use", لیبل_2, ArrayRef(لڑیاں));
}

Error ARMAttributeParser::nodefaults(AttrType لیبل_2) {
  uint64_t قدر_2 = de.getULEB128(cursor);
  printAttribute(لیبل_2, قدر_2, "Unspecified Tags UNDEFINED");
  واپس Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType لیبل_2) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_قدر;

  SmallString<8> Description;
  raw_svector_ostream Desc_دھارا(Description);

  uint64_t Initial_انحراف = cursor.tell();
  StringRef Raw_String_قدر = de.getCStrRef(cursor);
  uint64_t Final_انحراف = cursor.tell();
  cursor.seek(Initial_انحراف);
  uint64_t Inner_لیبل = de.getULEB128(cursor);

  منطقی Valid_Inner_لیبل =
      any_کا(لیبل_مقصد_String_Map, [Inner_لیبل](ثابت لیبل_Name_شے &شے) {
        واپس شے.attr == Inner_لیبل;
      });

  اگر (!Valid_Inner_لیبل) {
    return_قدر =
        createStringError(errc::دلیل_برآمد_کا_domain,
                          Twine(Inner_لیبل) + " is not a valid tag number");
  } ورنہ {
    انتخاب (Inner_لیبل) {
    حالت ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_قدر = de.getULEB128(cursor);
      خودکار لڑیاں = ArrayRef(CPU_arch_لڑیاں);
      اگر (Inner_قدر >= لڑیاں.سائز_2()) {
        return_قدر = createStringError(
            errc::دلیل_برآمد_کا_domain,
            Twine(Inner_قدر) + " is not a valid " +
                ELFAttrs::attrTypeAsString(Inner_لیبل, لیبل_مقصد_String_Map) +
                " value");
      } ورنہ {
        Desc_دھارا << ELFAttrs::attrTypeAsString(Inner_لیبل, لیبل_مقصد_String_Map)
                   << " = " << Inner_قدر;
        اگر (لڑیاں[Inner_قدر])
          Desc_دھارا << " (" << لڑیاں[Inner_قدر] << ')';
      }
      توڑو;
    }
    حالت ARMBuildAttrs::also_compatible_with:
      return_قدر = createStringError(
          errc::invalid_دلیل,
          ELFAttrs::attrTypeAsString(Inner_لیبل, لیبل_مقصد_String_Map) +
              " cannot be recursively defined");
      توڑو;
    حالت ARMBuildAttrs::CPU_raw_name:
    حالت ARMBuildAttrs::CPU_name:
    حالت ARMBuildAttrs::compatibility:
    حالت ARMBuildAttrs::conformance: {
      StringRef Inner_قدر = de.getCStrRef(cursor);
      Desc_دھارا << ELFAttrs::attrTypeAsString(Inner_لیبل, لیبل_مقصد_String_Map)
                 << " = " << Inner_قدر;
      توڑو;
    }
    طےشدہ: {
      uint64_t Inner_قدر = de.getULEB128(cursor);
      Desc_دھارا << ELFAttrs::attrTypeAsString(Inner_لیبل, لیبل_مقصد_String_Map)
                 << " = " << Inner_قدر;
    }
    }
  }

  سیٹ_Attribute_String(لیبل_2, Raw_String_قدر);
  اگر (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_نمبر("Tag", لیبل_2);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(لیبل_2, لیبل_مقصد_String_Map, جھوٹ));
    sw->printStringEscaped("Value", Raw_String_قدر);
    اگر (!Description.خالی_خانے()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(Final_انحراف);

  واپس return_قدر ? std::move(*return_قدر) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t لیبل_2, منطقی &handled) {
  handled = جھوٹ;
  برائے (ثابت خودکار &AH : displayRoutines) {
    اگر (uint64_t(AH.attribute) == لیبل_2) {
      اگر (Error e = (یہ->*AH.routine)(static_cast<AttrType>(لیبل_2)))
        واپس e;
      handled = سچ;
      توڑو;
    }
  }

  واپس Error::success();
}
