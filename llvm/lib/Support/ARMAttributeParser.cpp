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

استفاده نطاق llvm;
استفاده نطاق llvm::ARMBuildAttrs;

#تعریف_کن ATTRIBUTE_HANDLER(attr)                                                \
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
        ATTRIBUTE_HANDLER(ABI_PCS_RW_داده),
        ATTRIBUTE_HANDLER(ABI_PCS_RO_داده),
        ATTRIBUTE_HANDLER(ABI_PCS_GOT_use),
        ATTRIBUTE_HANDLER(ABI_PCS_wchar_t),
        ATTRIBUTE_HANDLER(ABI_FP_rounding),
        ATTRIBUTE_HANDLER(ABI_FP_denormal),
        ATTRIBUTE_HANDLER(ABI_FP_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_user_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_عدد_model),
        ATTRIBUTE_HANDLER(ABI_align_needed),
        ATTRIBUTE_HANDLER(ABI_align_preserved),
        ATTRIBUTE_HANDLER(ABI_enum_اندازه),
        ATTRIBUTE_HANDLER(ABI_HardFP_use),
        ATTRIBUTE_HANDLER(ABI_VFP_آرگومانها),
        ATTRIBUTE_HANDLER(ABI_WMMX_آرگومانها),
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

#لغو_تعریف ATTRIBUTE_HANDLER

Error ARMAttributeParser::stringAttribute(AttrType برچسب_2) {
  StringRef برچسب_Name =
      ELFAttrs::attrTypeAsString(برچسب_2, برچسب_مقصد_String_Map, /*hasTagPrefix=*/خطأ);
  StringRef desc = de.getCStrRef(cursor);

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_عدد("Tag", برچسب_2);
    إذا (!برچسب_Name.خالی())
      sw->printString("TagName", برچسب_Name);
    sw->printString("Value", desc);
  }
  أرجع Error::success();
}

ساكن ثابت حرف *ثابت CPU_arch_رشتهها[] = {"Pre-v4",
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
                                               اشاره_تهی,
                                               اشاره_تهی,
                                               اشاره_تهی,
                                               "ARM v8.1-M Mainline",
                                               "ARM v9-A"};

Error ARMAttributeParser::CPU_arch(AttrType برچسب_2) {
  أرجع parseStringAttribute("CPU_arch", برچسب_2, ArrayRef(CPU_arch_رشتهها));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType برچسب_2) {
  uint64_t مقدار_2 = de.getULEB128(cursor);

  StringRef profile;
  بدل (مقدار_2) {
  افتراضي: profile = "Unknown"; اكسر;
  حالة 'A': profile = "Application"; اكسر;
  حالة 'R': profile = "Real-time"; اكسر;
  حالة 'M': profile = "Microcontroller"; اكسر;
  حالة 'S': profile = "Classic"; اكسر;
  حالة 0: profile = "None"; اكسر;
  }

  printAttribute(برچسب_2, مقدار_2, profile);
  أرجع Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "Permitted"};
  أرجع parseStringAttribute("ARM_ISA_use", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  أرجع parseStringAttribute("THUMB_ISA_use", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::FP_arch(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  أرجع parseStringAttribute("FP_arch", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::WMMX_arch(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  أرجع parseStringAttribute("WMMX_arch", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  أرجع parseStringAttribute("Advanced_SIMD_arch", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::MVE_arch(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  أرجع parseStringAttribute("MVE_arch", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::PCS_config(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  أرجع parseStringAttribute("PCS_config", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"v6", "Static Base", "TLS", "Unused"};
  أرجع parseStringAttribute("ABI_PCS_R9_use", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::ABI_PCS_RW_داده(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  أرجع parseStringAttribute("ABI_PCS_RW_data", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::ABI_PCS_RO_داده(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  أرجع parseStringAttribute("ABI_PCS_RO_data", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  أرجع parseStringAttribute("ABI_PCS_GOT_use", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  أرجع parseStringAttribute("ABI_PCS_wchar_t", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"IEEE-754", "Runtime"};
  أرجع parseStringAttribute("ABI_FP_rounding", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Unsupported", "IEEE-754", "Sign Only"};
  أرجع parseStringAttribute("ABI_FP_denormal", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "IEEE-754"};
  أرجع parseStringAttribute("ABI_FP_exceptions", برچسب_2, ArrayRef(رشتهها));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "IEEE-754"};
  أرجع parseStringAttribute("ABI_FP_user_exceptions", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::ABI_FP_عدد_model(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  أرجع parseStringAttribute("ABI_FP_number_model", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::ABI_align_needed(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t مقدار_2 = de.getULEB128(cursor);

  std::string description;
  إذا (مقدار_2 < std::اندازه_2(رشتهها))
    description = رشتهها[مقدار_2];
  وإلا إذا (مقدار_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << مقدار_2) +
                  "-byte extended alignment";
  وإلا
    description = "Invalid";

  printAttribute(برچسب_2, مقدار_2, description);
  أرجع Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType برچسب_2) {
  ساكن ثابت حرف *رشتهها[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t مقدار_2 = de.getULEB128(cursor);

  std::string description;
  إذا (مقدار_2 < std::اندازه_2(رشتهها))
    description = std::string(رشتهها[مقدار_2]);
  وإلا إذا (مقدار_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << مقدار_2) + std::string("-byte data alignment");
  وإلا
    description = "Invalid";

  printAttribute(برچسب_2, مقدار_2, description);
  أرجع Error::success();
}

Error ARMAttributeParser::ABI_enum_اندازه(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  أرجع parseStringAttribute("ABI_enum_size", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  أرجع parseStringAttribute("ABI_HardFP_use", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::ABI_VFP_آرگومانها(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  أرجع parseStringAttribute("ABI_VFP_args", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::ABI_WMMX_آرگومانها(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"AAPCS", "iWMMX", "Custom"};
  أرجع parseStringAttribute("ABI_WMMX_args", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  أرجع parseStringAttribute("ABI_optimization_goals", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  أرجع parseStringAttribute("ABI_FP_optimization_goals", برچسب_2,
                              ArrayRef(رشتهها));
}

Error ARMAttributeParser::compatibility(AttrType برچسب_2) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_عدد("Tag", برچسب_2);
    sw->شروع_سطر() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(برچسب_2, برچسب_مقصد_String_Map,
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

Error ARMAttributeParser::CPU_unaligned_access(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "v6-style"};
  أرجع parseStringAttribute("CPU_unaligned_access", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::FP_HP_extension(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"If Available", "Permitted"};
  أرجع parseStringAttribute("FP_HP_extension", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  أرجع parseStringAttribute("ABI_FP_16bit_format", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::MPextension_use(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "Permitted"};
  أرجع parseStringAttribute("MPextension_use", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::DIV_use(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  أرجع parseStringAttribute("DIV_use", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::DSP_extension(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "Permitted"};
  أرجع parseStringAttribute("DSP_extension", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::T2EE_use(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted", "Permitted"};
  أرجع parseStringAttribute("T2EE_use", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::Virtualization_use(AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  أرجع parseStringAttribute("Virtualization_use", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  أرجع parseStringAttribute("PAC_extension", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  أرجع parseStringAttribute("BTI_extension", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Used", "Used"};
  أرجع parseStringAttribute("PACRET_use", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType برچسب_2) {
  ساكن ثابت حرف *ثابت رشتهها[] = {"Not Used", "Used"};
  أرجع parseStringAttribute("BTI_use", برچسب_2, ArrayRef(رشتهها));
}

Error ARMAttributeParser::nodefaults(AttrType برچسب_2) {
  uint64_t مقدار_2 = de.getULEB128(cursor);
  printAttribute(برچسب_2, مقدار_2, "Unspecified Tags UNDEFINED");
  أرجع Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType برچسب_2) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_مقدار;

  SmallString<8> Description;
  raw_svector_ostream Desc_جریان(Description);

  uint64_t Initial_جابه_جایی = cursor.tell();
  StringRef Raw_String_مقدار = de.getCStrRef(cursor);
  uint64_t Final_جابه_جایی = cursor.tell();
  cursor.seek(Initial_جابه_جایی);
  uint64_t Inner_برچسب = de.getULEB128(cursor);

  منطقي Valid_Inner_برچسب =
      any_از(برچسب_مقصد_String_Map, [Inner_برچسب](ثابت برچسب_Name_قلم &قلم) {
        أرجع قلم.attr == Inner_برچسب;
      });

  إذا (!Valid_Inner_برچسب) {
    return_مقدار =
        createStringError(errc::آرگومان_بیرون_از_domain,
                          Twine(Inner_برچسب) + " is not a valid tag number");
  } وإلا {
    بدل (Inner_برچسب) {
    حالة ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_مقدار = de.getULEB128(cursor);
      خودکار رشتهها = ArrayRef(CPU_arch_رشتهها);
      إذا (Inner_مقدار >= رشتهها.اندازه_2()) {
        return_مقدار = createStringError(
            errc::آرگومان_بیرون_از_domain,
            Twine(Inner_مقدار) + " is not a valid " +
                ELFAttrs::attrTypeAsString(Inner_برچسب, برچسب_مقصد_String_Map) +
                " value");
      } وإلا {
        Desc_جریان << ELFAttrs::attrTypeAsString(Inner_برچسب, برچسب_مقصد_String_Map)
                   << " = " << Inner_مقدار;
        إذا (رشتهها[Inner_مقدار])
          Desc_جریان << " (" << رشتهها[Inner_مقدار] << ')';
      }
      اكسر;
    }
    حالة ARMBuildAttrs::also_compatible_with:
      return_مقدار = createStringError(
          errc::invalid_آرگومان,
          ELFAttrs::attrTypeAsString(Inner_برچسب, برچسب_مقصد_String_Map) +
              " cannot be recursively defined");
      اكسر;
    حالة ARMBuildAttrs::CPU_raw_name:
    حالة ARMBuildAttrs::CPU_name:
    حالة ARMBuildAttrs::compatibility:
    حالة ARMBuildAttrs::conformance: {
      StringRef Inner_مقدار = de.getCStrRef(cursor);
      Desc_جریان << ELFAttrs::attrTypeAsString(Inner_برچسب, برچسب_مقصد_String_Map)
                 << " = " << Inner_مقدار;
      اكسر;
    }
    افتراضي: {
      uint64_t Inner_مقدار = de.getULEB128(cursor);
      Desc_جریان << ELFAttrs::attrTypeAsString(Inner_برچسب, برچسب_مقصد_String_Map)
                 << " = " << Inner_مقدار;
    }
    }
  }

  مجموعه_Attribute_String(برچسب_2, Raw_String_مقدار);
  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_عدد("Tag", برچسب_2);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(برچسب_2, برچسب_مقصد_String_Map, خطأ));
    sw->printStringEscaped("Value", Raw_String_مقدار);
    إذا (!Description.خالی()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(Final_جابه_جایی);

  أرجع return_مقدار ? std::move(*return_مقدار) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t برچسب_2, منطقي &handled) {
  handled = خطأ;
  لكل (ثابت خودکار &AH : displayRoutines) {
    إذا (uint64_t(AH.attribute) == برچسب_2) {
      إذا (Error e = (هذا->*AH.routine)(static_cast<AttrType>(برچسب_2)))
        أرجع e;
      handled = درست;
      اكسر;
    }
  }

  أرجع Error::success();
}
