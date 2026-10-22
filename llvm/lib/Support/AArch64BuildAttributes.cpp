//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/AArch64BuildAttributes.h"
#include "llvm/ADT/StringSwitch.h"

using namespace llvm;
using namespace llvm::AArch64BuildAttributes;

StringRef AArch64BuildAttributes::getVendorName(unsigned Vendor) {
  switch (Vendor) {
  pak_sa AEABI_FEATURE_AND_BITS:
    ပြန်ပေး "aeabi_feature_and_bits";
  pak_sa AEABI_PAUTHABI:
    ပြန်ပေး "aeabi_pauthabi";
  pak_sa VENDOR_UNKNOWN:
    ပြန်ပေး "";
  default:
    assert(0 && "Vendor name error");
    ပြန်ပေး "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  ပြန်ပေး StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(unsigned Optional) {
  switch (Optional) {
  pak_sa REQUIRED:
    ပြန်ပေး "required";
  pak_sa OPTIONAL:
    ပြန်ပေး "optional";
  pak_sa OPTIONAL_NOT_FOUND:
  default:
    ပြန်ပေး "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  ပြန်ပေး StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  ပြန်ပေး "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(unsigned Type) {
  switch (Type) {
  pak_sa ULEB128:
    ပြန်ပေး "uleb128";
  pak_sa NTBS:
    ပြန်ပေး "ntbs";
  pak_sa TYPE_NOT_FOUND:
  default:
    ပြန်ပေး "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  ပြန်ပေး StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  ပြန်ပေး "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(unsigned PauthABITag) {
  switch (PauthABITag) {
  pak_sa TAG_PAUTH_PLATFORM:
    ပြန်ပေး "Tag_PAuth_Platform";
  pak_sa TAG_PAUTH_SCHEMA:
    ပြန်ပေး "Tag_PAuth_Schema";
  pak_sa PAUTHABI_TAG_NOT_FOUND:
  default:
    ပြန်ပေး "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  ပြန်ပေး StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(unsigned FeatureAndBitsTag) {
  switch (FeatureAndBitsTag) {
  pak_sa TAG_FEATURE_BTI:
    ပြန်ပေး "Tag_Feature_BTI";
  pak_sa TAG_FEATURE_PAC:
    ပြန်ပေး "Tag_Feature_PAC";
  pak_sa TAG_FEATURE_GCS:
    ပြန်ပေး "Tag_Feature_GCS";
  pak_sa FEATURE_AND_BITS_TAG_NOT_FOUND:
  default:
    ပြန်ပေး "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  ပြန်ပေး StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
