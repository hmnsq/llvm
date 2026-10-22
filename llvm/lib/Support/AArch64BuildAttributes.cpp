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
    врати "aeabi_feature_and_bits";
  case AEABI_PAUTHABI:
    врати "aeabi_pauthabi";
  case VENDOR_UNKNOWN:
    врати "";
  default:
    assert(0 && "Vendor name error");
    врати "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  врати StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(unsigned Optional) {
  switch (Optional) {
  case REQUIRED:
    врати "required";
  case OPTIONAL:
    врати "optional";
  case OPTIONAL_NOT_FOUND:
  default:
    врати "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  врати StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  врати "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(unsigned Type) {
  switch (Type) {
  case ULEB128:
    врати "uleb128";
  case NTBS:
    врати "ntbs";
  case TYPE_NOT_FOUND:
  default:
    врати "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  врати StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  врати "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(unsigned PauthABITag) {
  switch (PauthABITag) {
  case TAG_PAUTH_PLATFORM:
    врати "Tag_PAuth_Platform";
  case TAG_PAUTH_SCHEMA:
    врати "Tag_PAuth_Schema";
  case PAUTHABI_TAG_NOT_FOUND:
  default:
    врати "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  врати StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(unsigned FeatureAndBitsTag) {
  switch (FeatureAndBitsTag) {
  case TAG_FEATURE_BTI:
    врати "Tag_Feature_BTI";
  case TAG_FEATURE_PAC:
    врати "Tag_Feature_PAC";
  case TAG_FEATURE_GCS:
    врати "Tag_Feature_GCS";
  case FEATURE_AND_BITS_TAG_NOT_FOUND:
  default:
    врати "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  врати StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
