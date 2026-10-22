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

using نطاق llvm;
using نطاق llvm::ARMBuildAttrs;

#define ATTRIBUTE_HANDLER(attr)                                                \
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
        ATTRIBUTE_HANDLER(ABI_PCS_RW_data),
        ATTRIBUTE_HANDLER(ABI_PCS_RO_data),
        ATTRIBUTE_HANDLER(ABI_PCS_GOT_use),
        ATTRIBUTE_HANDLER(ABI_PCS_wchar_t),
        ATTRIBUTE_HANDLER(ABI_FP_rounding),
        ATTRIBUTE_HANDLER(ABI_FP_denormal),
        ATTRIBUTE_HANDLER(ABI_FP_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_user_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_number_model),
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
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/ܕܓܠܐ);
  StringRef desc = de.getCStrRef(cursor);

  ܐܢ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->printNumber("Tag", tag);
    ܐܢ (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  ܦܢܐ Error::success();
}

ساكن ثابت حرف *ثابت CPU_arch_strings[] = {"Pre-v4",
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
  ܦܢܐ parseStringAttribute("CPU_arch", tag, ArrayRef(CPU_arch_strings));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType tag) {
  uint64_t value = de.getULEB128(cursor);

  StringRef profile;
  بدل (value) {
  افتراضي: profile = "Unknown"; اكسر;
  حالة 'A': profile = "Application"; اكسر;
  حالة 'R': profile = "Real-time"; اكسر;
  حالة 'M': profile = "Microcontroller"; اكسر;
  حالة 'S': profile = "Classic"; اكسر;
  حالة 0: profile = "None"; اكسر;
  }

  printAttribute(tag, value, profile);
  ܦܢܐ Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "Permitted"};
  ܦܢܐ parseStringAttribute("ARM_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  ܦܢܐ parseStringAttribute("THUMB_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_arch(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  ܦܢܐ parseStringAttribute("FP_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::WMMX_arch(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  ܦܢܐ parseStringAttribute("WMMX_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  ܦܢܐ parseStringAttribute("Advanced_SIMD_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MVE_arch(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  ܦܢܐ parseStringAttribute("MVE_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PCS_config(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  ܦܢܐ parseStringAttribute("PCS_config", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"v6", "Static Base", "TLS", "Unused"};
  ܦܢܐ parseStringAttribute("ABI_PCS_R9_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RW_data(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  ܦܢܐ parseStringAttribute("ABI_PCS_RW_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RO_data(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  ܦܢܐ parseStringAttribute("ABI_PCS_RO_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  ܦܢܐ parseStringAttribute("ABI_PCS_GOT_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  ܦܢܐ parseStringAttribute("ABI_PCS_wchar_t", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"IEEE-754", "Runtime"};
  ܦܢܐ parseStringAttribute("ABI_FP_rounding", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Unsupported", "IEEE-754", "Sign Only"};
  ܦܢܐ parseStringAttribute("ABI_FP_denormal", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "IEEE-754"};
  ܦܢܐ parseStringAttribute("ABI_FP_exceptions", tag, ArrayRef(strings));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "IEEE-754"};
  ܦܢܐ parseStringAttribute("ABI_FP_user_exceptions", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_number_model(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  ܦܢܐ parseStringAttribute("ABI_FP_number_model", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_align_needed(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t value = de.getULEB128(cursor);

  std::string description;
  ܐܢ (value < std::size(strings))
    description = strings[value];
  ܐܚܪܢܐ ܐܢ (value <= 12)
    description = "8-byte alignment, " + utostr(1ULL << value) +
                  "-byte extended alignment";
  ܐܚܪܢܐ
    description = "Invalid";

  printAttribute(tag, value, description);
  ܦܢܐ Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType tag) {
  ساكن ثابت حرف *strings[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t value = de.getULEB128(cursor);

  std::string description;
  ܐܢ (value < std::size(strings))
    description = std::string(strings[value]);
  ܐܚܪܢܐ ܐܢ (value <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << value) + std::string("-byte data alignment");
  ܐܚܪܢܐ
    description = "Invalid";

  printAttribute(tag, value, description);
  ܦܢܐ Error::success();
}

Error ARMAttributeParser::ABI_enum_size(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  ܦܢܐ parseStringAttribute("ABI_enum_size", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  ܦܢܐ parseStringAttribute("ABI_HardFP_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_VFP_args(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  ܦܢܐ parseStringAttribute("ABI_VFP_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_WMMX_args(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"AAPCS", "iWMMX", "Custom"};
  ܦܢܐ parseStringAttribute("ABI_WMMX_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  ܦܢܐ parseStringAttribute("ABI_optimization_goals", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  ܦܢܐ parseStringAttribute("ABI_FP_optimization_goals", tag,
                              ArrayRef(strings));
}

Error ARMAttributeParser::compatibility(AttrType tag) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  ܐܢ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->printNumber("Tag", tag);
    sw->startLine() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                               /*hasTagPrefix=*/ܕܓܠܐ));
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
  ܦܢܐ Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "v6-style"};
  ܦܢܐ parseStringAttribute("CPU_unaligned_access", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_HP_extension(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"If Available", "Permitted"};
  ܦܢܐ parseStringAttribute("FP_HP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  ܦܢܐ parseStringAttribute("ABI_FP_16bit_format", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MPextension_use(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "Permitted"};
  ܦܢܐ parseStringAttribute("MPextension_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DIV_use(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  ܦܢܐ parseStringAttribute("DIV_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DSP_extension(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "Permitted"};
  ܦܢܐ parseStringAttribute("DSP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::T2EE_use(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted", "Permitted"};
  ܦܢܐ parseStringAttribute("T2EE_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Virtualization_use(AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  ܦܢܐ parseStringAttribute("Virtualization_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  ܦܢܐ parseStringAttribute("PAC_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  ܦܢܐ parseStringAttribute("BTI_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Used", "Used"};
  ܦܢܐ parseStringAttribute("PACRET_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType tag) {
  ساكن ثابت حرف *ثابت strings[] = {"Not Used", "Used"};
  ܦܢܐ parseStringAttribute("BTI_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::nodefaults(AttrType tag) {
  uint64_t value = de.getULEB128(cursor);
  printAttribute(tag, value, "Unspecified Tags UNDEFINED");
  ܦܢܐ Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType tag) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> returnValue;

  SmallString<8> Description;
  raw_svector_ostream DescStream(Description);

  uint64_t InitialOffset = cursor.tell();
  StringRef RawStringValue = de.getCStrRef(cursor);
  uint64_t FinalOffset = cursor.tell();
  cursor.seek(InitialOffset);
  uint64_t InnerTag = de.getULEB128(cursor);

  منطقي ValidInnerTag =
      any_of(tagToStringMap, [InnerTag](ثابت TagNameItem &Item) {
        ܦܢܐ Item.attr == InnerTag;
      });

  ܐܢ (!ValidInnerTag) {
    returnValue =
        createStringError(errc::argument_out_of_domain,
                          Twine(InnerTag) + " is not a valid tag number");
  } ܐܚܪܢܐ {
    بدل (InnerTag) {
    حالة ARMBuildAttrs::CPU_arch: {
      uint64_t InnerValue = de.getULEB128(cursor);
      auto strings = ArrayRef(CPU_arch_strings);
      ܐܢ (InnerValue >= strings.size()) {
        returnValue = createStringError(
            errc::argument_out_of_domain,
            Twine(InnerValue) + " is not a valid " +
                ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
                " value");
      } ܐܚܪܢܐ {
        DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                   << " = " << InnerValue;
        ܐܢ (strings[InnerValue])
          DescStream << " (" << strings[InnerValue] << ')';
      }
      اكسر;
    }
    حالة ARMBuildAttrs::also_compatible_with:
      returnValue = createStringError(
          errc::invalid_argument,
          ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
              " cannot be recursively defined");
      اكسر;
    حالة ARMBuildAttrs::CPU_raw_name:
    حالة ARMBuildAttrs::CPU_name:
    حالة ARMBuildAttrs::compatibility:
    حالة ARMBuildAttrs::conformance: {
      StringRef InnerValue = de.getCStrRef(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << InnerValue;
      اكسر;
    }
    افتراضي: {
      uint64_t InnerValue = de.getULEB128(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << InnerValue;
    }
    }
  }

  setAttributeString(tag, RawStringValue);
  ܐܢ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->printNumber("Tag", tag);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap, ܕܓܠܐ));
    sw->printStringEscaped("Value", RawStringValue);
    ܐܢ (!Description.empty()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(FinalOffset);

  ܦܢܐ returnValue ? std::move(*returnValue) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t tag, منطقي &handled) {
  handled = ܕܓܠܐ;
  لكل (ثابت auto &AH : displayRoutines) {
    ܐܢ (uint64_t(AH.attribute) == tag) {
      ܐܢ (Error e = (هذا->*AH.routine)(static_cast<AttrType>(tag)))
        ܦܢܐ e;
      handled = ܫܪܪܐ;
      اكسر;
    }
  }

  ܦܢܐ Error::success();
}
