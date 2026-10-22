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
  case AEABI_FEATURE_AND_BITS:
    戻る "aeabi_feature_and_bits";
  case AEABI_PAUTHABI:
    戻る "aeabi_pauthabi";
  case VENDOR_UNKNOWN:
    戻る "";
  default:
    assert(0 && "Vendor name error");
    戻る "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  戻る StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(unsigned Optional) {
  switch (Optional) {
  case REQUIRED:
    戻る "required";
  case OPTIONAL:
    戻る "optional";
  case OPTIONAL_NOT_FOUND:
  default:
    戻る "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  戻る StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  戻る "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(unsigned Type) {
  switch (Type) {
  case ULEB128:
    戻る "uleb128";
  case NTBS:
    戻る "ntbs";
  case TYPE_NOT_FOUND:
  default:
    戻る "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  戻る StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  戻る "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(unsigned PauthABITag) {
  switch (PauthABITag) {
  case TAG_PAUTH_PLATFORM:
    戻る "Tag_PAuth_Platform";
  case TAG_PAUTH_SCHEMA:
    戻る "Tag_PAuth_Schema";
  case PAUTHABI_TAG_NOT_FOUND:
  default:
    戻る "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  戻る StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(unsigned FeatureAndBitsTag) {
  switch (FeatureAndBitsTag) {
  case TAG_FEATURE_BTI:
    戻る "Tag_Feature_BTI";
  case TAG_FEATURE_PAC:
    戻る "Tag_Feature_PAC";
  case TAG_FEATURE_GCS:
    戻る "Tag_Feature_GCS";
  case FEATURE_AND_BITS_TAG_NOT_FOUND:
  default:
    戻る "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  戻る StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
