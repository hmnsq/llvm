//===- ARMAttributeParser.cpp - ARM Attribute Information Printer ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/ARMAttributeParser.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/Support/ARMBuildAttributes.h"
#অন্তর্ভাব "llvm/Support/Errc.h"
#অন্তর্ভাব "llvm/Support/ScopedPrinter.h"
#অন্তর্ভাব <optional>

using নামাকাশ llvm;
using নামাকাশ llvm::ARMBuildAttrs;

#define ATTRIBUTE_HANDLER(attr)                                                \
  { ARMBuildAttrs::attr, &ARMAttributeParser::attr }

ধ্রুবক ARMAttributeParser::DisplayHandler ARMAttributeParser::displayRoutines[] =
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
        ATTRIBUTE_HANDLER(ABI_PCS_RW_তথ্য),
        ATTRIBUTE_HANDLER(ABI_PCS_RO_তথ্য),
        ATTRIBUTE_HANDLER(ABI_PCS_GOT_use),
        ATTRIBUTE_HANDLER(ABI_PCS_wchar_t),
        ATTRIBUTE_HANDLER(ABI_FP_rounding),
        ATTRIBUTE_HANDLER(ABI_FP_denormal),
        ATTRIBUTE_HANDLER(ABI_FP_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_user_exceptions),
        ATTRIBUTE_HANDLER(ABI_FP_সংখ্যা_model),
        ATTRIBUTE_HANDLER(ABI_align_needed),
        ATTRIBUTE_HANDLER(ABI_align_preserved),
        ATTRIBUTE_HANDLER(ABI_enum_আকার),
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
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/মিথ্যা);
  StringRef desc = de.getCStrRef(cursor);

  যদি (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_সংখ্যা("Tag", tag);
    যদি (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  ফেরত Error::success();
}

স্থির ধ্রুবক অক্ষর *ধ্রুবক CPU_arch_strings[] = {"Pre-v4",
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
  ফেরত parseStringAttribute("CPU_arch", tag, ArrayRef(CPU_arch_strings));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType tag) {
  uint64_t মান_2 = de.getULEB128(cursor);

  StringRef profile;
  নির্বাচন (মান_2) {
  পূর্বনির্ধারিত: profile = "Unknown"; বিরতি;
  ক্ষেত্র 'A': profile = "Application"; বিরতি;
  ক্ষেত্র 'R': profile = "Real-time"; বিরতি;
  ক্ষেত্র 'M': profile = "Microcontroller"; বিরতি;
  ক্ষেত্র 'S': profile = "Classic"; বিরতি;
  ক্ষেত্র 0: profile = "None"; বিরতি;
  }

  printAttribute(tag, মান_2, profile);
  ফেরত Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "Permitted"};
  ফেরত parseStringAttribute("ARM_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  ফেরত parseStringAttribute("THUMB_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_arch(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  ফেরত parseStringAttribute("FP_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::WMMX_arch(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  ফেরত parseStringAttribute("WMMX_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  ফেরত parseStringAttribute("Advanced_SIMD_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MVE_arch(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  ফেরত parseStringAttribute("MVE_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PCS_config(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  ফেরত parseStringAttribute("PCS_config", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"v6", "Static Base", "TLS", "Unused"};
  ফেরত parseStringAttribute("ABI_PCS_R9_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RW_তথ্য(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  ফেরত parseStringAttribute("ABI_PCS_RW_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RO_তথ্য(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  ফেরত parseStringAttribute("ABI_PCS_RO_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  ফেরত parseStringAttribute("ABI_PCS_GOT_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  ফেরত parseStringAttribute("ABI_PCS_wchar_t", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"IEEE-754", "Runtime"};
  ফেরত parseStringAttribute("ABI_FP_rounding", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Unsupported", "IEEE-754", "Sign Only"};
  ফেরত parseStringAttribute("ABI_FP_denormal", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "IEEE-754"};
  ফেরত parseStringAttribute("ABI_FP_exceptions", tag, ArrayRef(strings));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "IEEE-754"};
  ফেরত parseStringAttribute("ABI_FP_user_exceptions", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_সংখ্যা_model(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  ফেরত parseStringAttribute("ABI_FP_number_model", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_align_needed(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t মান_2 = de.getULEB128(cursor);

  std::string description;
  যদি (মান_2 < std::আকার_3(strings))
    description = strings[মান_2];
  অন্যথা যদি (মান_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << মান_2) +
                  "-byte extended alignment";
  অন্যথা
    description = "Invalid";

  printAttribute(tag, মান_2, description);
  ফেরত Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *strings[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t মান_2 = de.getULEB128(cursor);

  std::string description;
  যদি (মান_2 < std::আকার_3(strings))
    description = std::string(strings[মান_2]);
  অন্যথা যদি (মান_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << মান_2) + std::string("-byte data alignment");
  অন্যথা
    description = "Invalid";

  printAttribute(tag, মান_2, description);
  ফেরত Error::success();
}

Error ARMAttributeParser::ABI_enum_আকার(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  ফেরত parseStringAttribute("ABI_enum_size", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  ফেরত parseStringAttribute("ABI_HardFP_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_VFP_args(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  ফেরত parseStringAttribute("ABI_VFP_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_WMMX_args(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"AAPCS", "iWMMX", "Custom"};
  ফেরত parseStringAttribute("ABI_WMMX_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  ফেরত parseStringAttribute("ABI_optimization_goals", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  ফেরত parseStringAttribute("ABI_FP_optimization_goals", tag,
                              ArrayRef(strings));
}

Error ARMAttributeParser::compatibility(AttrType tag) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  যদি (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_সংখ্যা("Tag", tag);
    sw->start_সারি() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                               /*hasTagPrefix=*/মিথ্যা));
    নির্বাচন (integer) {
    ক্ষেত্র 0:
      sw->printString("Description", StringRef("No Specific Requirements"));
      বিরতি;
    ক্ষেত্র 1:
      sw->printString("Description", StringRef("AEABI Conformant"));
      বিরতি;
    পূর্বনির্ধারিত:
      sw->printString("Description", StringRef("AEABI Non-Conformant"));
      বিরতি;
    }
  }
  ফেরত Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "v6-style"};
  ফেরত parseStringAttribute("CPU_unaligned_access", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_HP_extension(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"If Available", "Permitted"};
  ফেরত parseStringAttribute("FP_HP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  ফেরত parseStringAttribute("ABI_FP_16bit_format", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MPextension_use(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "Permitted"};
  ফেরত parseStringAttribute("MPextension_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DIV_use(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  ফেরত parseStringAttribute("DIV_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DSP_extension(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "Permitted"};
  ফেরত parseStringAttribute("DSP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::T2EE_use(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted", "Permitted"};
  ফেরত parseStringAttribute("T2EE_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Virtualization_use(AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  ফেরত parseStringAttribute("Virtualization_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  ফেরত parseStringAttribute("PAC_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  ফেরত parseStringAttribute("BTI_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Used", "Used"};
  ফেরত parseStringAttribute("PACRET_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType tag) {
  স্থির ধ্রুবক অক্ষর *ধ্রুবক strings[] = {"Not Used", "Used"};
  ফেরত parseStringAttribute("BTI_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::nodefaults(AttrType tag) {
  uint64_t মান_2 = de.getULEB128(cursor);
  printAttribute(tag, মান_2, "Unspecified Tags UNDEFINED");
  ফেরত Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType tag) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_মান;

  SmallString<8> Description;
  raw_svector_ostream DescStream(Description);

  uint64_t InitialOffset = cursor.tell();
  StringRef Raw_String_মান = de.getCStrRef(cursor);
  uint64_t FinalOffset = cursor.tell();
  cursor.seek(InitialOffset);
  uint64_t InnerTag = de.getULEB128(cursor);

  যৌক্তিক ValidInnerTag =
      any_of(tagToStringMap, [InnerTag](ধ্রুবক TagNameItem &Item) {
        ফেরত Item.attr == InnerTag;
      });

  যদি (!ValidInnerTag) {
    return_মান =
        createStringError(errc::argument_out_of_domain,
                          Twine(InnerTag) + " is not a valid tag number");
  } অন্যথা {
    নির্বাচন (InnerTag) {
    ক্ষেত্র ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_মান = de.getULEB128(cursor);
      auto strings = ArrayRef(CPU_arch_strings);
      যদি (Inner_মান >= strings.আকার_3()) {
        return_মান = createStringError(
            errc::argument_out_of_domain,
            Twine(Inner_মান) + " is not a valid " +
                ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
                " value");
      } অন্যথা {
        DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                   << " = " << Inner_মান;
        যদি (strings[Inner_মান])
          DescStream << " (" << strings[Inner_মান] << ')';
      }
      বিরতি;
    }
    ক্ষেত্র ARMBuildAttrs::also_compatible_with:
      return_মান = createStringError(
          errc::invalid_argument,
          ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
              " cannot be recursively defined");
      বিরতি;
    ক্ষেত্র ARMBuildAttrs::CPU_raw_name:
    ক্ষেত্র ARMBuildAttrs::CPU_name:
    ক্ষেত্র ARMBuildAttrs::compatibility:
    ক্ষেত্র ARMBuildAttrs::conformance: {
      StringRef Inner_মান = de.getCStrRef(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_মান;
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      uint64_t Inner_মান = de.getULEB128(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_মান;
    }
    }
  }

  setAttributeString(tag, Raw_String_মান);
  যদি (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_সংখ্যা("Tag", tag);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap, মিথ্যা));
    sw->printStringEscaped("Value", Raw_String_মান);
    যদি (!Description.empty()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(FinalOffset);

  ফেরত return_মান ? std::move(*return_মান) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t tag, যৌক্তিক &handled) {
  handled = মিথ্যা;
  জন্য (ধ্রুবক auto &AH : displayRoutines) {
    যদি (uint64_t(AH.attribute) == tag) {
      যদি (Error e = (এই->*AH.routine)(static_cast<AttrType>(tag)))
        ফেরত e;
      handled = সত্য;
      বিরতি;
    }
  }

  ফেরত Error::success();
}
