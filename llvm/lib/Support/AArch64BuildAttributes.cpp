//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/AArch64BuildAttributes.h"
#अन्तर्भाव "llvm/ADT/StringSwitch.h"

using नामाकाश llvm;
using नामाकाश llvm::AArch64BuildAttributes;

StringRef AArch64BuildAttributes::getVendorName(unsigned Vendor) {
  विकल्प (Vendor) {
  पक्ष AEABI_FEATURE_AND_BITS:
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
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  प्रत्यागम StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(unsigned Optional) {
  विकल्प (Optional) {
  पक्ष REQUIRED:
    प्रत्यागम "required";
  पक्ष OPTIONAL:
    प्रत्यागम "optional";
  पक्ष OPTIONAL_NOT_FOUND:
  स्वभाव:
    प्रत्यागम "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  प्रत्यागम StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  प्रत्यागम "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(unsigned Type) {
  विकल्प (Type) {
  पक्ष ULEB128:
    प्रत्यागम "uleb128";
  पक्ष NTBS:
    प्रत्यागम "ntbs";
  पक्ष TYPE_NOT_FOUND:
  स्वभाव:
    प्रत्यागम "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  प्रत्यागम StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  प्रत्यागम "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(unsigned PauthABITag) {
  विकल्प (PauthABITag) {
  पक्ष TAG_PAUTH_PLATFORM:
    प्रत्यागम "Tag_PAuth_Platform";
  पक्ष TAG_PAUTH_SCHEMA:
    प्रत्यागम "Tag_PAuth_Schema";
  पक्ष PAUTHABI_TAG_NOT_FOUND:
  स्वभाव:
    प्रत्यागम "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  प्रत्यागम StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(unsigned FeatureAndBitsTag) {
  विकल्प (FeatureAndBitsTag) {
  पक्ष TAG_FEATURE_BTI:
    प्रत्यागम "Tag_Feature_BTI";
  पक्ष TAG_FEATURE_PAC:
    प्रत्यागम "Tag_Feature_PAC";
  पक्ष TAG_FEATURE_GCS:
    प्रत्यागम "Tag_Feature_GCS";
  पक्ष FEATURE_AND_BITS_TAG_NOT_FOUND:
  स्वभाव:
    प्रत्यागम "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  प्रत्यागम StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
