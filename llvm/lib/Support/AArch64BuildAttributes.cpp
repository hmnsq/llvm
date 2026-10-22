//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/AArch64BuildAttributes.h"
#অন্তর্ভাব "llvm/ADT/StringSwitch.h"

using নামাকাশ llvm;
using নামাকাশ llvm::AArch64BuildAttributes;

StringRef AArch64BuildAttributes::getVendorName(unsigned Vendor) {
  নির্বাচন (Vendor) {
  ক্ষেত্র AEABI_FEATURE_AND_BITS:
    ফেরত "aeabi_feature_and_bits";
  ক্ষেত্র AEABI_PAUTHABI:
    ফেরত "aeabi_pauthabi";
  ক্ষেত্র VENDOR_UNKNOWN:
    ফেরত "";
  পূর্বনির্ধারিত:
    প্রতিজ্ঞা(0 && "Vendor name error");
    ফেরত "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  ফেরত StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(unsigned Optional) {
  নির্বাচন (Optional) {
  ক্ষেত্র REQUIRED:
    ফেরত "required";
  ক্ষেত্র OPTIONAL:
    ফেরত "optional";
  ক্ষেত্র OPTIONAL_NOT_FOUND:
  পূর্বনির্ধারিত:
    ফেরত "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  ফেরত StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  ফেরত "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(unsigned Type) {
  নির্বাচন (Type) {
  ক্ষেত্র ULEB128:
    ফেরত "uleb128";
  ক্ষেত্র NTBS:
    ফেরত "ntbs";
  ক্ষেত্র TYPE_NOT_FOUND:
  পূর্বনির্ধারিত:
    ফেরত "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  ফেরত StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  ফেরত "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(unsigned PauthABITag) {
  নির্বাচন (PauthABITag) {
  ক্ষেত্র TAG_PAUTH_PLATFORM:
    ফেরত "Tag_PAuth_Platform";
  ক্ষেত্র TAG_PAUTH_SCHEMA:
    ফেরত "Tag_PAuth_Schema";
  ক্ষেত্র PAUTHABI_TAG_NOT_FOUND:
  পূর্বনির্ধারিত:
    ফেরত "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  ফেরত StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(unsigned FeatureAndBitsTag) {
  নির্বাচন (FeatureAndBitsTag) {
  ক্ষেত্র TAG_FEATURE_BTI:
    ফেরত "Tag_Feature_BTI";
  ক্ষেত্র TAG_FEATURE_PAC:
    ফেরত "Tag_Feature_PAC";
  ক্ষেত্র TAG_FEATURE_GCS:
    ফেরত "Tag_Feature_GCS";
  ক্ষেত্র FEATURE_AND_BITS_TAG_NOT_FOUND:
  পূর্বনির্ধারিত:
    ফেরত "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  ফেরত StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
