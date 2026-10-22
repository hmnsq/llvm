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

using فضاینام llvm;
using فضاینام llvm::ARMBuildAttrs;

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
        ATTRIBUTE_HANDLER(ABI_FP_عدد_model),
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
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/نادرست);
  StringRef desc = de.getCStrRef(cursor);

  اگر (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_عدد("Tag", tag);
    اگر (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  بازگشت Error::success();
}

ایستا ثابت نویسه *ثابت CPU_arch_strings[] = {"Pre-v4",
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
  بازگشت parseStringAttribute("CPU_arch", tag, ArrayRef(CPU_arch_strings));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType tag) {
  uint64_t مقدار_2 = de.getULEB128(cursor);

  StringRef profile;
  انتخاب (مقدار_2) {
  پیشفرض: profile = "Unknown"; بشکن;
  حالت 'A': profile = "Application"; بشکن;
  حالت 'R': profile = "Real-time"; بشکن;
  حالت 'M': profile = "Microcontroller"; بشکن;
  حالت 'S': profile = "Classic"; بشکن;
  حالت 0: profile = "None"; بشکن;
  }

  printAttribute(tag, مقدار_2, profile);
  بازگشت Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "Permitted"};
  بازگشت parseStringAttribute("ARM_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  بازگشت parseStringAttribute("THUMB_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_arch(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  بازگشت parseStringAttribute("FP_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::WMMX_arch(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  بازگشت parseStringAttribute("WMMX_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  بازگشت parseStringAttribute("Advanced_SIMD_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MVE_arch(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  بازگشت parseStringAttribute("MVE_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PCS_config(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  بازگشت parseStringAttribute("PCS_config", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"v6", "Static Base", "TLS", "Unused"};
  بازگشت parseStringAttribute("ABI_PCS_R9_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RW_data(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  بازگشت parseStringAttribute("ABI_PCS_RW_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RO_data(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  بازگشت parseStringAttribute("ABI_PCS_RO_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  بازگشت parseStringAttribute("ABI_PCS_GOT_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  بازگشت parseStringAttribute("ABI_PCS_wchar_t", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"IEEE-754", "Runtime"};
  بازگشت parseStringAttribute("ABI_FP_rounding", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Unsupported", "IEEE-754", "Sign Only"};
  بازگشت parseStringAttribute("ABI_FP_denormal", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "IEEE-754"};
  بازگشت parseStringAttribute("ABI_FP_exceptions", tag, ArrayRef(strings));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "IEEE-754"};
  بازگشت parseStringAttribute("ABI_FP_user_exceptions", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_عدد_model(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  بازگشت parseStringAttribute("ABI_FP_number_model", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_align_needed(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t مقدار_2 = de.getULEB128(cursor);

  std::string description;
  اگر (مقدار_2 < std::size(strings))
    description = strings[مقدار_2];
  وگرنه اگر (مقدار_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << مقدار_2) +
                  "-byte extended alignment";
  وگرنه
    description = "Invalid";

  printAttribute(tag, مقدار_2, description);
  بازگشت Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType tag) {
  ایستا ثابت نویسه *strings[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t مقدار_2 = de.getULEB128(cursor);

  std::string description;
  اگر (مقدار_2 < std::size(strings))
    description = std::string(strings[مقدار_2]);
  وگرنه اگر (مقدار_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << مقدار_2) + std::string("-byte data alignment");
  وگرنه
    description = "Invalid";

  printAttribute(tag, مقدار_2, description);
  بازگشت Error::success();
}

Error ARMAttributeParser::ABI_enum_size(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  بازگشت parseStringAttribute("ABI_enum_size", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  بازگشت parseStringAttribute("ABI_HardFP_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_VFP_args(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  بازگشت parseStringAttribute("ABI_VFP_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_WMMX_args(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"AAPCS", "iWMMX", "Custom"};
  بازگشت parseStringAttribute("ABI_WMMX_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  بازگشت parseStringAttribute("ABI_optimization_goals", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  بازگشت parseStringAttribute("ABI_FP_optimization_goals", tag,
                              ArrayRef(strings));
}

Error ARMAttributeParser::compatibility(AttrType tag) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  اگر (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_عدد("Tag", tag);
    sw->start_سطر() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                               /*hasTagPrefix=*/نادرست));
    انتخاب (integer) {
    حالت 0:
      sw->printString("Description", StringRef("No Specific Requirements"));
      بشکن;
    حالت 1:
      sw->printString("Description", StringRef("AEABI Conformant"));
      بشکن;
    پیشفرض:
      sw->printString("Description", StringRef("AEABI Non-Conformant"));
      بشکن;
    }
  }
  بازگشت Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "v6-style"};
  بازگشت parseStringAttribute("CPU_unaligned_access", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_HP_extension(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"If Available", "Permitted"};
  بازگشت parseStringAttribute("FP_HP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  بازگشت parseStringAttribute("ABI_FP_16bit_format", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MPextension_use(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "Permitted"};
  بازگشت parseStringAttribute("MPextension_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DIV_use(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  بازگشت parseStringAttribute("DIV_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DSP_extension(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "Permitted"};
  بازگشت parseStringAttribute("DSP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::T2EE_use(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted", "Permitted"};
  بازگشت parseStringAttribute("T2EE_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Virtualization_use(AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  بازگشت parseStringAttribute("Virtualization_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  بازگشت parseStringAttribute("PAC_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  بازگشت parseStringAttribute("BTI_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Used", "Used"};
  بازگشت parseStringAttribute("PACRET_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType tag) {
  ایستا ثابت نویسه *ثابت strings[] = {"Not Used", "Used"};
  بازگشت parseStringAttribute("BTI_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::nodefaults(AttrType tag) {
  uint64_t مقدار_2 = de.getULEB128(cursor);
  printAttribute(tag, مقدار_2, "Unspecified Tags UNDEFINED");
  بازگشت Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType tag) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_مقدار;

  SmallString<8> Description;
  raw_svector_ostream DescStream(Description);

  uint64_t InitialOffset = cursor.tell();
  StringRef Raw_String_مقدار = de.getCStrRef(cursor);
  uint64_t FinalOffset = cursor.tell();
  cursor.seek(InitialOffset);
  uint64_t InnerTag = de.getULEB128(cursor);

  منطقی ValidInnerTag =
      any_of(tagToStringMap, [InnerTag](ثابت TagNameItem &Item) {
        بازگشت Item.attr == InnerTag;
      });

  اگر (!ValidInnerTag) {
    return_مقدار =
        createStringError(errc::argument_out_of_domain,
                          Twine(InnerTag) + " is not a valid tag number");
  } وگرنه {
    انتخاب (InnerTag) {
    حالت ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_مقدار = de.getULEB128(cursor);
      auto strings = ArrayRef(CPU_arch_strings);
      اگر (Inner_مقدار >= strings.size()) {
        return_مقدار = createStringError(
            errc::argument_out_of_domain,
            Twine(Inner_مقدار) + " is not a valid " +
                ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
                " value");
      } وگرنه {
        DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                   << " = " << Inner_مقدار;
        اگر (strings[Inner_مقدار])
          DescStream << " (" << strings[Inner_مقدار] << ')';
      }
      بشکن;
    }
    حالت ARMBuildAttrs::also_compatible_with:
      return_مقدار = createStringError(
          errc::invalid_argument,
          ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
              " cannot be recursively defined");
      بشکن;
    حالت ARMBuildAttrs::CPU_raw_name:
    حالت ARMBuildAttrs::CPU_name:
    حالت ARMBuildAttrs::compatibility:
    حالت ARMBuildAttrs::conformance: {
      StringRef Inner_مقدار = de.getCStrRef(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_مقدار;
      بشکن;
    }
    پیشفرض: {
      uint64_t Inner_مقدار = de.getULEB128(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_مقدار;
    }
    }
  }

  setAttributeString(tag, Raw_String_مقدار);
  اگر (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_عدد("Tag", tag);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap, نادرست));
    sw->printStringEscaped("Value", Raw_String_مقدار);
    اگر (!Description.empty()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(FinalOffset);

  بازگشت return_مقدار ? std::move(*return_مقدار) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t tag, منطقی &handled) {
  handled = نادرست;
  برای (ثابت auto &AH : displayRoutines) {
    اگر (uint64_t(AH.attribute) == tag) {
      اگر (Error e = (این->*AH.routine)(static_cast<AttrType>(tag)))
        بازگشت e;
      handled = درست;
      بشکن;
    }
  }

  بازگشت Error::success();
}
