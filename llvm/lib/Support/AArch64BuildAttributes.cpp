//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/AArch64BuildAttributes.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringSwitch.h"

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::AArch64BuildAttributes;

StringRef AArch64BuildAttributes::getVendorName(unsigned Vendor) {
  ବାଛ (Vendor) {
  ସ୍ଥିତି AEABI_FEATURE_AND_BITS:
    ଫେରନ୍ତୁ "aeabi_feature_and_bits";
  ସ୍ଥିତି AEABI_PAUTHABI:
    ଫେରନ୍ତୁ "aeabi_pauthabi";
  ସ୍ଥିତି VENDOR_UNKNOWN:
    ଫେରନ୍ତୁ "";
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ପ୍ରତିଜ୍ଞା(0 && "Vendor name error");
    ଫେରନ୍ତୁ "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  ଫେରନ୍ତୁ StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(unsigned Optional) {
  ବାଛ (Optional) {
  ସ୍ଥିତି REQUIRED:
    ଫେରନ୍ତୁ "required";
  ସ୍ଥିତି OPTIONAL:
    ଫେରନ୍ତୁ "optional";
  ସ୍ଥିତି OPTIONAL_NOT_FOUND:
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଫେରନ୍ତୁ "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  ଫେରନ୍ତୁ StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  ଫେରନ୍ତୁ "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(unsigned Type) {
  ବାଛ (Type) {
  ସ୍ଥିତି ULEB128:
    ଫେରନ୍ତୁ "uleb128";
  ସ୍ଥିତି NTBS:
    ଫେରନ୍ତୁ "ntbs";
  ସ୍ଥିତି TYPE_NOT_FOUND:
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଫେରନ୍ତୁ "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  ଫେରନ୍ତୁ StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  ଫେରନ୍ତୁ "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(unsigned PauthABITag) {
  ବାଛ (PauthABITag) {
  ସ୍ଥିତି TAG_PAUTH_PLATFORM:
    ଫେରନ୍ତୁ "Tag_PAuth_Platform";
  ସ୍ଥିତି TAG_PAUTH_SCHEMA:
    ଫେରନ୍ତୁ "Tag_PAuth_Schema";
  ସ୍ଥିତି PAUTHABI_TAG_NOT_FOUND:
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଫେରନ୍ତୁ "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  ଫେରନ୍ତୁ StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(unsigned FeatureAndBitsTag) {
  ବାଛ (FeatureAndBitsTag) {
  ସ୍ଥିତି TAG_FEATURE_BTI:
    ଫେରନ୍ତୁ "Tag_Feature_BTI";
  ସ୍ଥିତି TAG_FEATURE_PAC:
    ଫେରନ୍ତୁ "Tag_Feature_PAC";
  ସ୍ଥିତି TAG_FEATURE_GCS:
    ଫେରନ୍ତୁ "Tag_Feature_GCS";
  ସ୍ଥିତି FEATURE_AND_BITS_TAG_NOT_FOUND:
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଫେରନ୍ତୁ "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  ଫେରନ୍ତୁ StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
