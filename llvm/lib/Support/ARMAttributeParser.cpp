//===- ARMAttributeParser.cpp - ARM Attribute Information Printer ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/ARMAttributeParser.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/Support/ARMBuildAttributes.h"
#અન્તર્ભાવ "llvm/Support/Errc.h"
#અન્તર્ભાવ "llvm/Support/ScopedPrinter.h"
#અન્તર્ભાવ <optional>

using નામાકાશ llvm;
using નામાકાશ llvm::ARMBuildAttrs;

#define ATTRIBUTE_HANDLER(attr)                                                \
  { ARMBuildAttrs::attr, &ARMAttributeParser::attr }

અચલ ARMAttributeParser::DisplayHandler ARMAttributeParser::displayRoutines[] =
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
        ATTRIBUTE_HANDLER(ABI_FP_સંખ્યા_model),
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
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/ખોટું);
  StringRef desc = de.getCStrRef(cursor);

  જો (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_સંખ્યા("Tag", tag);
    જો (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  પાછા Error::success();
}

સ્થિર અચલ અક્ષર *અચલ CPU_arch_strings[] = {"Pre-v4",
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
  પાછા parseStringAttribute("CPU_arch", tag, ArrayRef(CPU_arch_strings));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType tag) {
  uint64_t કિંમત_2 = de.getULEB128(cursor);

  StringRef profile;
  પસંદગી (કિંમત_2) {
  મૂળભૂત: profile = "Unknown"; તોડો;
  સ્થિતિ 'A': profile = "Application"; તોડો;
  સ્થિતિ 'R': profile = "Real-time"; તોડો;
  સ્થિતિ 'M': profile = "Microcontroller"; તોડો;
  સ્થિતિ 'S': profile = "Classic"; તોડો;
  સ્થિતિ 0: profile = "None"; તોડો;
  }

  printAttribute(tag, કિંમત_2, profile);
  પાછા Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "Permitted"};
  પાછા parseStringAttribute("ARM_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  પાછા parseStringAttribute("THUMB_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_arch(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  પાછા parseStringAttribute("FP_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::WMMX_arch(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  પાછા parseStringAttribute("WMMX_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  પાછા parseStringAttribute("Advanced_SIMD_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MVE_arch(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  પાછા parseStringAttribute("MVE_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PCS_config(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  પાછા parseStringAttribute("PCS_config", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"v6", "Static Base", "TLS", "Unused"};
  પાછા parseStringAttribute("ABI_PCS_R9_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RW_data(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  પાછા parseStringAttribute("ABI_PCS_RW_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RO_data(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  પાછા parseStringAttribute("ABI_PCS_RO_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  પાછા parseStringAttribute("ABI_PCS_GOT_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  પાછા parseStringAttribute("ABI_PCS_wchar_t", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"IEEE-754", "Runtime"};
  પાછા parseStringAttribute("ABI_FP_rounding", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Unsupported", "IEEE-754", "Sign Only"};
  પાછા parseStringAttribute("ABI_FP_denormal", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "IEEE-754"};
  પાછા parseStringAttribute("ABI_FP_exceptions", tag, ArrayRef(strings));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "IEEE-754"};
  પાછા parseStringAttribute("ABI_FP_user_exceptions", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_સંખ્યા_model(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  પાછા parseStringAttribute("ABI_FP_number_model", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_align_needed(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t કિંમત_2 = de.getULEB128(cursor);

  std::string description;
  જો (કિંમત_2 < std::size(strings))
    description = strings[કિંમત_2];
  નહીંતર જો (કિંમત_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << કિંમત_2) +
                  "-byte extended alignment";
  નહીંતર
    description = "Invalid";

  printAttribute(tag, કિંમત_2, description);
  પાછા Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType tag) {
  સ્થિર અચલ અક્ષર *strings[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t કિંમત_2 = de.getULEB128(cursor);

  std::string description;
  જો (કિંમત_2 < std::size(strings))
    description = std::string(strings[કિંમત_2]);
  નહીંતર જો (કિંમત_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << કિંમત_2) + std::string("-byte data alignment");
  નહીંતર
    description = "Invalid";

  printAttribute(tag, કિંમત_2, description);
  પાછા Error::success();
}

Error ARMAttributeParser::ABI_enum_size(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  પાછા parseStringAttribute("ABI_enum_size", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  પાછા parseStringAttribute("ABI_HardFP_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_VFP_args(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  પાછા parseStringAttribute("ABI_VFP_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_WMMX_args(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"AAPCS", "iWMMX", "Custom"};
  પાછા parseStringAttribute("ABI_WMMX_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  પાછા parseStringAttribute("ABI_optimization_goals", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  પાછા parseStringAttribute("ABI_FP_optimization_goals", tag,
                              ArrayRef(strings));
}

Error ARMAttributeParser::compatibility(AttrType tag) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  જો (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_સંખ્યા("Tag", tag);
    sw->start_લીટી() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                               /*hasTagPrefix=*/ખોટું));
    પસંદગી (integer) {
    સ્થિતિ 0:
      sw->printString("Description", StringRef("No Specific Requirements"));
      તોડો;
    સ્થિતિ 1:
      sw->printString("Description", StringRef("AEABI Conformant"));
      તોડો;
    મૂળભૂત:
      sw->printString("Description", StringRef("AEABI Non-Conformant"));
      તોડો;
    }
  }
  પાછા Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "v6-style"};
  પાછા parseStringAttribute("CPU_unaligned_access", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_HP_extension(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"If Available", "Permitted"};
  પાછા parseStringAttribute("FP_HP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  પાછા parseStringAttribute("ABI_FP_16bit_format", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MPextension_use(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "Permitted"};
  પાછા parseStringAttribute("MPextension_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DIV_use(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  પાછા parseStringAttribute("DIV_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DSP_extension(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "Permitted"};
  પાછા parseStringAttribute("DSP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::T2EE_use(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted", "Permitted"};
  પાછા parseStringAttribute("T2EE_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Virtualization_use(AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  પાછા parseStringAttribute("Virtualization_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  પાછા parseStringAttribute("PAC_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  પાછા parseStringAttribute("BTI_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Used", "Used"};
  પાછા parseStringAttribute("PACRET_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType tag) {
  સ્થિર અચલ અક્ષર *અચલ strings[] = {"Not Used", "Used"};
  પાછા parseStringAttribute("BTI_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::nodefaults(AttrType tag) {
  uint64_t કિંમત_2 = de.getULEB128(cursor);
  printAttribute(tag, કિંમત_2, "Unspecified Tags UNDEFINED");
  પાછા Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType tag) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_કિંમત;

  SmallString<8> Description;
  raw_svector_ostream DescStream(Description);

  uint64_t InitialOffset = cursor.tell();
  StringRef Raw_String_કિંમત = de.getCStrRef(cursor);
  uint64_t FinalOffset = cursor.tell();
  cursor.seek(InitialOffset);
  uint64_t InnerTag = de.getULEB128(cursor);

  તાર્કિક ValidInnerTag =
      any_of(tagToStringMap, [InnerTag](અચલ TagNameItem &Item) {
        પાછા Item.attr == InnerTag;
      });

  જો (!ValidInnerTag) {
    return_કિંમત =
        createStringError(errc::argument_out_of_domain,
                          Twine(InnerTag) + " is not a valid tag number");
  } નહીંતર {
    પસંદગી (InnerTag) {
    સ્થિતિ ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_કિંમત = de.getULEB128(cursor);
      auto strings = ArrayRef(CPU_arch_strings);
      જો (Inner_કિંમત >= strings.size()) {
        return_કિંમત = createStringError(
            errc::argument_out_of_domain,
            Twine(Inner_કિંમત) + " is not a valid " +
                ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
                " value");
      } નહીંતર {
        DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                   << " = " << Inner_કિંમત;
        જો (strings[Inner_કિંમત])
          DescStream << " (" << strings[Inner_કિંમત] << ')';
      }
      તોડો;
    }
    સ્થિતિ ARMBuildAttrs::also_compatible_with:
      return_કિંમત = createStringError(
          errc::invalid_argument,
          ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
              " cannot be recursively defined");
      તોડો;
    સ્થિતિ ARMBuildAttrs::CPU_raw_name:
    સ્થિતિ ARMBuildAttrs::CPU_name:
    સ્થિતિ ARMBuildAttrs::compatibility:
    સ્થિતિ ARMBuildAttrs::conformance: {
      StringRef Inner_કિંમત = de.getCStrRef(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_કિંમત;
      તોડો;
    }
    મૂળભૂત: {
      uint64_t Inner_કિંમત = de.getULEB128(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_કિંમત;
    }
    }
  }

  setAttributeString(tag, Raw_String_કિંમત);
  જો (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_સંખ્યા("Tag", tag);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap, ખોટું));
    sw->printStringEscaped("Value", Raw_String_કિંમત);
    જો (!Description.empty()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(FinalOffset);

  પાછા return_કિંમત ? std::move(*return_કિંમત) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t tag, તાર્કિક &handled) {
  handled = ખોટું;
  માટે (અચલ auto &AH : displayRoutines) {
    જો (uint64_t(AH.attribute) == tag) {
      જો (Error e = (આ->*AH.routine)(static_cast<AttrType>(tag)))
        પાછા e;
      handled = સાચું;
      તોડો;
    }
  }

  પાછા Error::success();
}
