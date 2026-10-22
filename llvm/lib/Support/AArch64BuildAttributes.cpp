//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/Support/AArch64BuildAttributes.h"
#অন্তর্ভাৱ "llvm/ADT/StringSwitch.h"

using নামাকাশ llvm;
using নামাকাশ llvm::AArch64BuildAttributes;

StringRef AArch64BuildAttributes::getVendorName(unsigned Vendor) {
  ৱিকল্প (Vendor) {
  পক্ষ AEABI_FEATURE_AND_BITS:
    ঘূৰাই "aeabi_feature_and_bits";
  পক্ষ AEABI_PAUTHABI:
    ঘূৰাই "aeabi_pauthabi";
  পক্ষ VENDOR_UNKNOWN:
    ঘূৰাই "";
  স্ৱভাৱ:
    প্রতিজ্ঞা(0 && "Vendor name error");
    ঘূৰাই "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  ঘূৰাই StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(unsigned Optional) {
  ৱিকল্প (Optional) {
  পক্ষ REQUIRED:
    ঘূৰাই "required";
  পক্ষ OPTIONAL:
    ঘূৰাই "optional";
  পক্ষ OPTIONAL_NOT_FOUND:
  স্ৱভাৱ:
    ঘূৰাই "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  ঘূৰাই StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  ঘূৰাই "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(unsigned Type) {
  ৱিকল্প (Type) {
  পক্ষ ULEB128:
    ঘূৰাই "uleb128";
  পক্ষ NTBS:
    ঘূৰাই "ntbs";
  পক্ষ TYPE_NOT_FOUND:
  স্ৱভাৱ:
    ঘূৰাই "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  ঘূৰাই StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  ঘূৰাই "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(unsigned PauthABITag) {
  ৱিকল্প (PauthABITag) {
  পক্ষ TAG_PAUTH_PLATFORM:
    ঘূৰাই "Tag_PAuth_Platform";
  পক্ষ TAG_PAUTH_SCHEMA:
    ঘূৰাই "Tag_PAuth_Schema";
  পক্ষ PAUTHABI_TAG_NOT_FOUND:
  স্ৱভাৱ:
    ঘূৰাই "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  ঘূৰাই StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(unsigned FeatureAndBitsTag) {
  ৱিকল্প (FeatureAndBitsTag) {
  পক্ষ TAG_FEATURE_BTI:
    ঘূৰাই "Tag_Feature_BTI";
  পক্ষ TAG_FEATURE_PAC:
    ঘূৰাই "Tag_Feature_PAC";
  পক্ষ TAG_FEATURE_GCS:
    ঘূৰাই "Tag_Feature_GCS";
  পক্ষ FEATURE_AND_BITS_TAG_NOT_FOUND:
  স্ৱভাৱ:
    ঘূৰাই "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  ঘূৰাই StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
