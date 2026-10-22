//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/AArch64BuildAttributes.h"
#अन्तर्भाव "llvm/ADT/StringSwitch.h"

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::AArch64_Build_गुणाः;

StringRef AArch64_Build_गुणाः::getVendorName(अचिह्नित Vendor) {
  विकल्प (Vendor) {
  पक्ष AEABI_FEATURE_AND_द्वयङ्काः:
    प्रत्यागम "aeabi_feature_and_bits";
  पक्ष AEABI_PAUTHABI:
    प्रत्यागम "aeabi_pauthabi";
  पक्ष VENDOR_UNKNOWN:
    प्रत्यागम "";
  स्वभाव:
    प्रतिज्ञा(0 && "Vendor name error");
    प्रत्यागम "";
  }
}
VendorID AArch64_Build_गुणाः::getVendorID(StringRef Vendor) {
  प्रत्यागम StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_द्वयङ्काः)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64_Build_गुणाः::getOptionalStr(अचिह्नित Optional) {
  विकल्प (Optional) {
  पक्ष REQUIRED:
    प्रत्यागम "required";
  पक्ष OPTIONAL:
    प्रत्यागम "optional";
  पक्ष OPTIONAL_NOT_प्राप्तम्:
  स्वभाव:
    प्रत्यागम "";
  }
}
SubsectionOptional AArch64_Build_गुणाः::getOptionalID(StringRef Optional) {
  प्रत्यागम StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_प्राप्तम्);
}
StringRef AArch64_Build_गुणाः::getSubsectionOptionalUnknownError() {
  प्रत्यागम "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64_Build_गुणाः::getTypeStr(अचिह्नित Type) {
  विकल्प (Type) {
  पक्ष ULEB128:
    प्रत्यागम "uleb128";
  पक्ष NTBS:
    प्रत्यागम "ntbs";
  पक्ष TYPE_NOT_प्राप्तम्:
  स्वभाव:
    प्रत्यागम "";
  }
}
SubsectionType AArch64_Build_गुणाः::getTypeID(StringRef Type) {
  प्रत्यागम StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_प्राप्तम्);
}
StringRef AArch64_Build_गुणाः::getSubsectionTypeUnknownError() {
  प्रत्यागम "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64_Build_गुणाः::getPauthABITagsStr(अचिह्नित PauthABITag) {
  विकल्प (PauthABITag) {
  पक्ष चिह्नकम्_PAUTH_PLATFORM:
    प्रत्यागम "Tag_PAuth_Platform";
  पक्ष चिह्नकम्_PAUTH_SCHEMA:
    प्रत्यागम "Tag_PAuth_Schema";
  पक्ष PAUTHABI_चिह्नकम्_NOT_प्राप्तम्:
  स्वभाव:
    प्रत्यागम "";
  }
}

PauthABITags AArch64_Build_गुणाः::getPauthABITagsID(StringRef PauthABITag) {
  प्रत्यागम StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", चिह्नकम्_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", चिह्नकम्_PAUTH_SCHEMA)
      .Default(PAUTHABI_चिह्नकम्_NOT_प्राप्तम्);
}

StringRef
AArch64_Build_गुणाः::get_Feature_And_द्वयङ्काः_चिह्नकानि_Str(अचिह्नित Feature_And_द्वयङ्काः_चिह्नकम्) {
  विकल्प (Feature_And_द्वयङ्काः_चिह्नकम्) {
  पक्ष चिह्नकम्_FEATURE_BTI:
    प्रत्यागम "Tag_Feature_BTI";
  पक्ष चिह्नकम्_FEATURE_PAC:
    प्रत्यागम "Tag_Feature_PAC";
  पक्ष चिह्नकम्_FEATURE_GCS:
    प्रत्यागम "Tag_Feature_GCS";
  पक्ष FEATURE_AND_द्वयङ्काः_चिह्नकम्_NOT_प्राप्तम्:
  स्वभाव:
    प्रत्यागम "";
  }
}

Feature_And_द्वयङ्काः_चिह्नकानि
AArch64_Build_गुणाः::get_Feature_And_द्वयङ्काः_चिह्नकानि_ID(StringRef Feature_And_द्वयङ्काः_चिह्नकम्) {
  प्रत्यागम StringSwitch<Feature_And_द्वयङ्काः_चिह्नकानि>(Feature_And_द्वयङ्काः_चिह्नकम्)
      .Case("Tag_Feature_BTI", चिह्नकम्_FEATURE_BTI)
      .Case("Tag_Feature_PAC", चिह्नकम्_FEATURE_PAC)
      .Case("Tag_Feature_GCS", चिह्नकम्_FEATURE_GCS)
      .Default(FEATURE_AND_द्वयङ्काः_चिह्नकम्_NOT_प्राप्तम्);
}
