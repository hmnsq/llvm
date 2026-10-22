//===- ARMAttributeParser.cpp - ARM Attribute Information Printer ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/ARMAttributeParser.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ARMBuildAttributes.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Errc.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ScopedPrinter.h"
#ଅନ୍ତର୍ଭାଵ <optional>

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::ARMBuildAttrs;

#define ATTRIBUTE_HANDLER(attr)                                                \
  { ARMBuildAttrs::attr, &ARMAttributeParser::attr }

ଅଚଳ ARMAttributeParser::DisplayHandler ARMAttributeParser::displayRoutines[] =
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
        ATTRIBUTE_HANDLER(ABI_FP_ସଂଖ୍ୟା_model),
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
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/ମିଥ୍ୟା);
  StringRef desc = de.getCStrRef(cursor);

  ଯଦି (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_ସଂଖ୍ୟା("Tag", tag);
    ଯଦି (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  ଫେରନ୍ତୁ Error::success();
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ CPU_arch_strings[] = {"Pre-v4",
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
  ଫେରନ୍ତୁ parseStringAttribute("CPU_arch", tag, ArrayRef(CPU_arch_strings));
}

Error ARMAttributeParser::CPU_arch_profile(AttrType tag) {
  uint64_t ମୂଲ୍ୟ_2 = de.getULEB128(cursor);

  StringRef profile;
  ବାଛ (ମୂଲ୍ୟ_2) {
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ: profile = "Unknown"; ଭାଙ୍ଗ;
  ସ୍ଥିତି 'A': profile = "Application"; ଭାଙ୍ଗ;
  ସ୍ଥିତି 'R': profile = "Real-time"; ଭାଙ୍ଗ;
  ସ୍ଥିତି 'M': profile = "Microcontroller"; ଭାଙ୍ଗ;
  ସ୍ଥିତି 'S': profile = "Classic"; ଭାଙ୍ଗ;
  ସ୍ଥିତି 0: profile = "None"; ଭାଙ୍ଗ;
  }

  printAttribute(tag, ମୂଲ୍ୟ_2, profile);
  ଫେରନ୍ତୁ Error::success();
}

Error ARMAttributeParser::ARM_ISA_use(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "Permitted"};
  ଫେରନ୍ତୁ parseStringAttribute("ARM_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::THUMB_ISA_use(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "Thumb-1", "Thumb-2",
                                        "Permitted"};
  ଫେରନ୍ତୁ parseStringAttribute("THUMB_ISA_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_arch(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {
      "Not Permitted", "VFPv1",     "VFPv2",      "VFPv3",         "VFPv3-D16",
      "VFPv4",         "VFPv4-D16", "ARMv8-a FP", "ARMv8-a FP-D16"};
  ଫେରନ୍ତୁ parseStringAttribute("FP_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::WMMX_arch(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "WMMXv1", "WMMXv2"};
  ଫେରନ୍ତୁ parseStringAttribute("WMMX_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Advanced_SIMD_arch(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "NEONv1", "NEONv2+FMA",
                                        "ARMv8-a NEON", "ARMv8.1-a NEON"};
  ଫେରନ୍ତୁ parseStringAttribute("Advanced_SIMD_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MVE_arch(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "MVE integer",
                                        "MVE integer and float"};
  ଫେରନ୍ତୁ parseStringAttribute("MVE_arch", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PCS_config(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"None",
                                        "Bare Platform",
                                        "Linux Application",
                                        "Linux DSO",
                                        "Palm OS 2004",
                                        "Reserved (Palm OS)",
                                        "Symbian OS 2004",
                                        "Reserved (Symbian OS)"};
  ଫେରନ୍ତୁ parseStringAttribute("PCS_config", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_R9_use(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"v6", "Static Base", "TLS", "Unused"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_PCS_R9_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RW_data(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Absolute", "PC-relative",
                                        "SB-relative", "Not Permitted"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_PCS_RW_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_RO_data(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Absolute", "PC-relative",
                                        "Not Permitted"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_PCS_RO_data", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_GOT_use(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "Direct",
                                        "GOT-Indirect"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_PCS_GOT_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_PCS_wchar_t(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "Unknown", "2-byte",
                                        "Unknown", "4-byte"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_PCS_wchar_t", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_rounding(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"IEEE-754", "Runtime"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_FP_rounding", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_denormal(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Unsupported", "IEEE-754", "Sign Only"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_FP_denormal", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_exceptions(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "IEEE-754"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_FP_exceptions", tag, ArrayRef(strings));
}
Error ARMAttributeParser::ABI_FP_user_exceptions(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "IEEE-754"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_FP_user_exceptions", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_ସଂଖ୍ୟା_model(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "Finite Only", "RTABI",
                                        "IEEE-754"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_FP_number_model", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_align_needed(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "8-byte alignment",
                                        "4-byte alignment", "Reserved"};

  uint64_t ମୂଲ୍ୟ_2 = de.getULEB128(cursor);

  std::string description;
  ଯଦି (ମୂଲ୍ୟ_2 < std::size(strings))
    description = strings[ମୂଲ୍ୟ_2];
  ଅନ୍ୟଥା ଯଦି (ମୂଲ୍ୟ_2 <= 12)
    description = "8-byte alignment, " + utostr(1ULL << ମୂଲ୍ୟ_2) +
                  "-byte extended alignment";
  ଅନ୍ୟଥା
    description = "Invalid";

  printAttribute(tag, ମୂଲ୍ୟ_2, description);
  ଫେରନ୍ତୁ Error::success();
}

Error ARMAttributeParser::ABI_align_preserved(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *strings[] = {"Not Required", "8-byte data alignment",
                                  "8-byte data and code alignment", "Reserved"};

  uint64_t ମୂଲ୍ୟ_2 = de.getULEB128(cursor);

  std::string description;
  ଯଦି (ମୂଲ୍ୟ_2 < std::size(strings))
    description = std::string(strings[ମୂଲ୍ୟ_2]);
  ଅନ୍ୟଥା ଯଦି (ମୂଲ୍ୟ_2 <= 12)
    description = std::string("8-byte stack alignment, ") +
                  utostr(1ULL << ମୂଲ୍ୟ_2) + std::string("-byte data alignment");
  ଅନ୍ୟଥା
    description = "Invalid";

  printAttribute(tag, ମୂଲ୍ୟ_2, description);
  ଫେରନ୍ତୁ Error::success();
}

Error ARMAttributeParser::ABI_enum_size(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "Packed", "Int32",
                                        "External Int32"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_enum_size", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_HardFP_use(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Tag_FP_arch", "Single-Precision",
                                        "Reserved", "Tag_FP_arch (deprecated)"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_HardFP_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_VFP_args(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"AAPCS", "AAPCS VFP", "Custom",
                                        "Not Permitted"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_VFP_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_WMMX_args(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"AAPCS", "iWMMX", "Custom"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_WMMX_args", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_optimization_goals(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {
      "None",          "Speed",           "Aggressive Speed",
      "Size",          "Aggressive Size", "Debugging",
      "Best Debugging"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_optimization_goals", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_optimization_goals(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {
      "None",     "Speed",        "Aggressive Speed", "Size", "Aggressive Size",
      "Accuracy", "Best Accuracy"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_FP_optimization_goals", tag,
                              ArrayRef(strings));
}

Error ARMAttributeParser::compatibility(AttrType tag) {
  uint64_t integer = de.getULEB128(cursor);
  StringRef string = de.getCStrRef(cursor);

  ଯଦି (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_ସଂଖ୍ୟା("Tag", tag);
    sw->start_ଧାଡ଼ି() << "Value: " << integer << ", " << string << '\n';
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                               /*hasTagPrefix=*/ମିଥ୍ୟା));
    ବାଛ (integer) {
    ସ୍ଥିତି 0:
      sw->printString("Description", StringRef("No Specific Requirements"));
      ଭାଙ୍ଗ;
    ସ୍ଥିତି 1:
      sw->printString("Description", StringRef("AEABI Conformant"));
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      sw->printString("Description", StringRef("AEABI Non-Conformant"));
      ଭାଙ୍ଗ;
    }
  }
  ଫେରନ୍ତୁ Error::success();
}

Error ARMAttributeParser::CPU_unaligned_access(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "v6-style"};
  ଫେରନ୍ତୁ parseStringAttribute("CPU_unaligned_access", tag, ArrayRef(strings));
}

Error ARMAttributeParser::FP_HP_extension(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"If Available", "Permitted"};
  ଫେରନ୍ତୁ parseStringAttribute("FP_HP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::ABI_FP_16bit_format(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "IEEE-754", "VFPv3"};
  ଫେରନ୍ତୁ parseStringAttribute("ABI_FP_16bit_format", tag, ArrayRef(strings));
}

Error ARMAttributeParser::MPextension_use(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "Permitted"};
  ଫେରନ୍ତୁ parseStringAttribute("MPextension_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DIV_use(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"If Available", "Not Permitted",
                                        "Permitted"};
  ଫେରନ୍ତୁ parseStringAttribute("DIV_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::DSP_extension(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "Permitted"};
  ଫେରନ୍ତୁ parseStringAttribute("DSP_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::T2EE_use(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted", "Permitted"};
  ଫେରନ୍ତୁ parseStringAttribute("T2EE_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::Virtualization_use(AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {
      "Not Permitted", "TrustZone", "Virtualization Extensions",
      "TrustZone + Virtualization Extensions"};
  ଫେରନ୍ତୁ parseStringAttribute("Virtualization_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PAC_extension(ARMBuildAttrs::AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  ଫେରନ୍ତୁ parseStringAttribute("PAC_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_extension(ARMBuildAttrs::AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Permitted",
                                        "Permitted in NOP space", "Permitted"};
  ଫେରନ୍ତୁ parseStringAttribute("BTI_extension", tag, ArrayRef(strings));
}

Error ARMAttributeParser::PACRET_use(ARMBuildAttrs::AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Used", "Used"};
  ଫେରନ୍ତୁ parseStringAttribute("PACRET_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::BTI_use(ARMBuildAttrs::AttrType tag) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ strings[] = {"Not Used", "Used"};
  ଫେରନ୍ତୁ parseStringAttribute("BTI_use", tag, ArrayRef(strings));
}

Error ARMAttributeParser::nodefaults(AttrType tag) {
  uint64_t ମୂଲ୍ୟ_2 = de.getULEB128(cursor);
  printAttribute(tag, ମୂଲ୍ୟ_2, "Unspecified Tags UNDEFINED");
  ଫେରନ୍ତୁ Error::success();
}

Error ARMAttributeParser::also_compatible_with(AttrType tag) {
  // Parse value as a C string first in order to print it in escaped form later.
  // Then, parse it again to catch errors or to pretty print if Tag_CPU_arch.
  std::optional<Error> return_ମୂଲ୍ୟ;

  SmallString<8> Description;
  raw_svector_ostream DescStream(Description);

  uint64_t InitialOffset = cursor.tell();
  StringRef Raw_String_ମୂଲ୍ୟ = de.getCStrRef(cursor);
  uint64_t FinalOffset = cursor.tell();
  cursor.seek(InitialOffset);
  uint64_t InnerTag = de.getULEB128(cursor);

  ତାର୍କିକ ValidInnerTag =
      any_of(tagToStringMap, [InnerTag](ଅଚଳ TagNameItem &Item) {
        ଫେରନ୍ତୁ Item.attr == InnerTag;
      });

  ଯଦି (!ValidInnerTag) {
    return_ମୂଲ୍ୟ =
        createStringError(errc::argument_out_of_domain,
                          Twine(InnerTag) + " is not a valid tag number");
  } ଅନ୍ୟଥା {
    ବାଛ (InnerTag) {
    ସ୍ଥିତି ARMBuildAttrs::CPU_arch: {
      uint64_t Inner_ମୂଲ୍ୟ = de.getULEB128(cursor);
      auto strings = ArrayRef(CPU_arch_strings);
      ଯଦି (Inner_ମୂଲ୍ୟ >= strings.size()) {
        return_ମୂଲ୍ୟ = createStringError(
            errc::argument_out_of_domain,
            Twine(Inner_ମୂଲ୍ୟ) + " is not a valid " +
                ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
                " value");
      } ଅନ୍ୟଥା {
        DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                   << " = " << Inner_ମୂଲ୍ୟ;
        ଯଦି (strings[Inner_ମୂଲ୍ୟ])
          DescStream << " (" << strings[Inner_ମୂଲ୍ୟ] << ')';
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି ARMBuildAttrs::also_compatible_with:
      return_ମୂଲ୍ୟ = createStringError(
          errc::invalid_argument,
          ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap) +
              " cannot be recursively defined");
      ଭାଙ୍ଗ;
    ସ୍ଥିତି ARMBuildAttrs::CPU_raw_name:
    ସ୍ଥିତି ARMBuildAttrs::CPU_name:
    ସ୍ଥିତି ARMBuildAttrs::compatibility:
    ସ୍ଥିତି ARMBuildAttrs::conformance: {
      StringRef Inner_ମୂଲ୍ୟ = de.getCStrRef(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_ମୂଲ୍ୟ;
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      uint64_t Inner_ମୂଲ୍ୟ = de.getULEB128(cursor);
      DescStream << ELFAttrs::attrTypeAsString(InnerTag, tagToStringMap)
                 << " = " << Inner_ମୂଲ୍ୟ;
    }
    }
  }

  setAttributeString(tag, Raw_String_ମୂଲ୍ୟ);
  ଯଦି (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_ସଂଖ୍ୟା("Tag", tag);
    sw->printString("TagName",
                    ELFAttrs::attrTypeAsString(tag, tagToStringMap, ମିଥ୍ୟା));
    sw->printStringEscaped("Value", Raw_String_ମୂଲ୍ୟ);
    ଯଦି (!Description.empty()) {
      sw->printString("Description", Description);
    }
  }

  cursor.seek(FinalOffset);

  ଫେରନ୍ତୁ return_ମୂଲ୍ୟ ? std::move(*return_ମୂଲ୍ୟ) : Error::success();
}

Error ARMAttributeParser::handler(uint64_t tag, ତାର୍କିକ &handled) {
  handled = ମିଥ୍ୟା;
  ପାଇଁ (ଅଚଳ auto &AH : displayRoutines) {
    ଯଦି (uint64_t(AH.attribute) == tag) {
      ଯଦି (Error e = (ଏହି->*AH.routine)(static_cast<AttrType>(tag)))
        ଫେରନ୍ତୁ e;
      handled = ସତ୍ୟ;
      ଭାଙ୍ଗ;
    }
  }

  ଫେରନ୍ତୁ Error::success();
}
