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

کارول نطاق llvm;
کارول نطاق llvm::ARMBuildAttrs;

#تعریف_کړه ATTRIBUTE_HANDLER(attr)                                                \
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
        ATTRIBUTE_HANDLER(ABI_FP_شمېره_model),
        ATTRIBUTE_HANDLER(ABI_align_needed),
        ATTRIBUTE_HANDLER(ABI_align_preserved),
        ATTRIBUTE_HANDLER(ABI_enum_اندازه),
        ATTRIBUTE_HANDLER(ABI_HardFP_use),
        ATTRIBUTE_HANDLER(ABI_VFP_دلیلونه),
        ATTRIBUTE_HANDLER(ABI_WMMX_دلیلونه),
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

#تعریف_لغوه ATTRIBUTE_HANDLER

Error ARMAttributeParser::stringAttribute(AttrType نښان_2) {
  StringRef نښان_Name =
      ELFAttrs::attrTypeAsString(نښان_2, نښان_منزل_String_Map, /*hasTagPrefix=*/خطأ);
  StringRef desc = de.getCStrRef(cursor);

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_شمېره("Tag", نښان_2);
    إذا (!نښان_Name.تشه())
      sw->printString("TagName", نښان_Name);
    sw->printString("Value", desc);
  }
  أرجع Error::success();
}

ساكن ثابت حرف *ثابت CPU_arch_مزي_ګانې[] = {"Pre-v4",
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
                                               تش_اشاره,
                                               تش_اشاره,
                                               تش_اشاره,
                                               "ARM v8.1-M Mainline",
                                               "ARM v9-A"};

Error ARMAttributeParser::CPU_arch(AttrType نښان_2) {
  أرجع parseStringAttribute("CPU_arch", نښان_2, ArrayRef(CPU_arch_مزي_ګانې));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType نښان_2) {
  uint64_t ارزښت_2 = de.getULEB128(cursor);

  StringRef profile;
  بدل (ارزښت_2) {
  افتراضي: profile = "Unknown"; اكسر;
  حالة 'A': profile = "Application"; اكسر;
  حالة 'R': profile = "Real-time"; اكسر;
  حالة 'M': profile = "Microcontroller"; اكسر;
  حالة 'S': profile = "Classic"; اكسر;
  حالة 0: profile = "None"; اكسر;
  }

  printAttribute(نښان_2, ارزښت_2, profile);
  أرجع Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "Permitted"};
  أرجع parseStringAttribute("ARM_ISA_use", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  أرجع parseStringAttribute("THUMB_ISA_use", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::FP_arch(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  أرجع parseStringAttribute("FP_arch", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::WMMX_arch(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  أرجع parseStringAttribute("WMMX_arch", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  أرجع parseStringAttribute("Advanced_SIMD_arch", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::MVE_arch(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  أرجع parseStringAttribute("MVE_arch", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::PCS_config(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  أرجع parseStringAttribute("PCS_config", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"v6", "Static Base", "TLS", "Unused"};
  أرجع parseStringAttribute("ABI_PCS_R9_use", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::ABI_PCS_RW_معلومات(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  أرجع parseStringAttribute("ABI_PCS_RW_data", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::ABI_PCS_RO_معلومات(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  أرجع parseStringAttribute("ABI_PCS_RO_data", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  أرجع parseStringAttribute("ABI_PCS_GOT_use", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  أرجع parseStringAttribute("ABI_PCS_wchar_t", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"IEEE-754", "Runtime"};
  أرجع parseStringAttribute("ABI_FP_rounding", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Unsupported", "IEEE-754", "Sign Only"};
  أرجع parseStringAttribute("ABI_FP_denormal", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "IEEE-754"};
  أرجع parseStringAttribute("ABI_FP_exceptions", نښان_2, ArrayRef(مزي_ګانې));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "IEEE-754"};
  أرجع parseStringAttribute("ABI_FP_user_exceptions", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::ABI_FP_شمېره_model(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  أرجع parseStringAttribute("ABI_FP_number_model", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::ABI_align_needed(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t ارزښت_2 = de.getULEB128(cursor);

  std::string description;
  إذا (ارزښت_2 < std::اندازه_2(مزي_ګانې))
    description = مزي_ګانې[ارزښت_2];
  وإلا إذا (ارزښت_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << ارزښت_2) +
                  "-byte extended alignment";
  وإلا
    description = "Invalid";

  printAttribute(نښان_2, ارزښت_2, description);
  أرجع Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType نښان_2) {
  ساكن ثابت حرف *مزي_ګانې[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t ارزښت_2 = de.getULEB128(cursor);

  std::string description;
  إذا (ارزښت_2 < std::اندازه_2(مزي_ګانې))
    description = std::string(مزي_ګانې[ارزښت_2]);
  وإلا إذا (ارزښت_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << ارزښت_2) + std::string("-byte data alignment");
  وإلا
    description = "Invalid";

  printAttribute(نښان_2, ارزښت_2, description);
  أرجع Error::success();
}

Error ARMAttributeParser::ABI_enum_اندازه(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  أرجع parseStringAttribute("ABI_enum_size", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  أرجع parseStringAttribute("ABI_HardFP_use", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::ABI_VFP_دلیلونه(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  أرجع parseStringAttribute("ABI_VFP_args", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::ABI_WMMX_دلیلونه(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"AAPCS", "iWMMX", "Custom"};
  أرجع parseStringAttribute("ABI_WMMX_args", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  أرجع parseStringAttribute("ABI_optimization_goals", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  أرجع parseStringAttribute("ABI_FP_optimization_goals", نښان_2,
                              ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::compatibility(AttrType نښان_2) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_شمېره("Tag", نښان_2);
    sw->پیل_کرښه() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(نښان_2, نښان_منزل_String_Map,
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

Error ARMAttributeParser::CPU_unaligned_access(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "v6-style"};
  أرجع parseStringAttribute("CPU_unaligned_access", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::FP_HP_extension(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"If Available", "Permitted"};
  أرجع parseStringAttribute("FP_HP_extension", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  أرجع parseStringAttribute("ABI_FP_16bit_format", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::MPextension_use(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "Permitted"};
  أرجع parseStringAttribute("MPextension_use", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::DIV_use(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  أرجع parseStringAttribute("DIV_use", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::DSP_extension(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "Permitted"};
  أرجع parseStringAttribute("DSP_extension", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::T2EE_use(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted", "Permitted"};
  أرجع parseStringAttribute("T2EE_use", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::Virtualization_use(AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  أرجع parseStringAttribute("Virtualization_use", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  أرجع parseStringAttribute("PAC_extension", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  أرجع parseStringAttribute("BTI_extension", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Used", "Used"};
  أرجع parseStringAttribute("PACRET_use", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType نښان_2) {
  ساكن ثابت حرف *ثابت مزي_ګانې[] = {"Not Used", "Used"};
  أرجع parseStringAttribute("BTI_use", نښان_2, ArrayRef(مزي_ګانې));
}

Error ARMAttributeParser::nodefaults(AttrType نښان_2) {
  uint64_t ارزښت_2 = de.getULEB128(cursor);
  printAttribute(نښان_2, ارزښت_2, "Unspecified Tags UNDEFINED");
  أرجع Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType نښان_2) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_ارزښت;

  SmallString<8> Description;
  raw_svector_ostream Desc_بهیر(Description);

  uint64_t Initial_دننه_واټن = cursor.tell();
  StringRef Raw_String_ارزښت = de.getCStrRef(cursor);
  uint64_t Final_دننه_واټن = cursor.tell();
  cursor.seek(Initial_دننه_واټن);
  uint64_t Inner_نښان = de.getULEB128(cursor);

  منطقي Valid_Inner_نښان =
      any_د(نښان_منزل_String_Map, [Inner_نښان](ثابت نښان_Name_توکی &توکی) {
        أرجع توکی.attr == Inner_نښان;
      });

  إذا (!Valid_Inner_نښان) {
    return_ارزښت =
        createStringError(errc::دلیل_بهر_د_domain,
                          Twine(Inner_نښان) + " is not a valid tag number");
  } وإلا {
    بدل (Inner_نښان) {
    حالة ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_ارزښت = de.getULEB128(cursor);
      خپلکاری مزي_ګانې = ArrayRef(CPU_arch_مزي_ګانې);
      إذا (Inner_ارزښت >= مزي_ګانې.اندازه_2()) {
        return_ارزښت = createStringError(
            errc::دلیل_بهر_د_domain,
            Twine(Inner_ارزښت) + " is not a valid " +
                ELFAttrs::attrTypeAsString(Inner_نښان, نښان_منزل_String_Map) +
                " value");
      } وإلا {
        Desc_بهیر << ELFAttrs::attrTypeAsString(Inner_نښان, نښان_منزل_String_Map)
                   << " = " << Inner_ارزښت;
        إذا (مزي_ګانې[Inner_ارزښت])
          Desc_بهیر << " (" << مزي_ګانې[Inner_ارزښت] << ')';
      }
      اكسر;
    }
    حالة ARMBuildAttrs::also_compatible_with:
      return_ارزښت = createStringError(
          errc::invalid_دلیل,
          ELFAttrs::attrTypeAsString(Inner_نښان, نښان_منزل_String_Map) +
              " cannot be recursively defined");
      اكسر;
    حالة ARMBuildAttrs::CPU_raw_name:
    حالة ARMBuildAttrs::CPU_name:
    حالة ARMBuildAttrs::compatibility:
    حالة ARMBuildAttrs::conformance: {
      StringRef Inner_ارزښت = de.getCStrRef(cursor);
      Desc_بهیر << ELFAttrs::attrTypeAsString(Inner_نښان, نښان_منزل_String_Map)
                 << " = " << Inner_ارزښت;
      اكسر;
    }
    افتراضي: {
      uint64_t Inner_ارزښت = de.getULEB128(cursor);
      Desc_بهیر << ELFAttrs::attrTypeAsString(Inner_نښان, نښان_منزل_String_Map)
                 << " = " << Inner_ارزښت;
    }
    }
  }

  سېټ_Attribute_String(نښان_2, Raw_String_ارزښت);
  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_شمېره("Tag", نښان_2);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(نښان_2, نښان_منزل_String_Map, خطأ));
    sw->printStringEscaped("Value", Raw_String_ارزښت);
    إذا (!Description.تشه()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(Final_دننه_واټن);

  أرجع return_ارزښت ? std::move(*return_ارزښت) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t نښان_2, منطقي &handled) {
  handled = خطأ;
  لكل (ثابت خپلکاری &AH : displayRoutines) {
    إذا (uint64_t(AH.attribute) == نښان_2) {
      إذا (Error e = (هذا->*AH.routine)(static_cast<AttrType>(نښان_2)))
        أرجع e;
      handled = ریښتیا;
      اكسر;
    }
  }

  أرجع Error::success();
}
