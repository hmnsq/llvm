//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/AArch64BuildAttributes.h"
#شامل "llvm/ADT/StringSwitch.h"

using فضاینام llvm;
using فضاینام llvm::AArch64BuildAttributes;

StringRef AArch64BuildAttributes::getVendorName(unsigned Vendor) {
  انتخاب (Vendor) {
  حالت AEABI_FEATURE_AND_BITS:
    بازگشت "aeabi_feature_and_bits";
  حالت AEABI_PAUTHABI:
    بازگشت "aeabi_pauthabi";
  حالت VENDOR_UNKNOWN:
    بازگشت "";
  پیشفرض:
    أكد(0 && "Vendor name error");
    بازگشت "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  بازگشت StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(unsigned Optional) {
  انتخاب (Optional) {
  حالت REQUIRED:
    بازگشت "required";
  حالت OPTIONAL:
    بازگشت "optional";
  حالت OPTIONAL_NOT_FOUND:
  پیشفرض:
    بازگشت "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  بازگشت StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  بازگشت "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(unsigned Type) {
  انتخاب (Type) {
  حالت ULEB128:
    بازگشت "uleb128";
  حالت NTBS:
    بازگشت "ntbs";
  حالت TYPE_NOT_FOUND:
  پیشفرض:
    بازگشت "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  بازگشت StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  بازگشت "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(unsigned PauthABITag) {
  انتخاب (PauthABITag) {
  حالت TAG_PAUTH_PLATFORM:
    بازگشت "Tag_PAuth_Platform";
  حالت TAG_PAUTH_SCHEMA:
    بازگشت "Tag_PAuth_Schema";
  حالت PAUTHABI_TAG_NOT_FOUND:
  پیشفرض:
    بازگشت "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  بازگشت StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(unsigned FeatureAndBitsTag) {
  انتخاب (FeatureAndBitsTag) {
  حالت TAG_FEATURE_BTI:
    بازگشت "Tag_Feature_BTI";
  حالت TAG_FEATURE_PAC:
    بازگشت "Tag_Feature_PAC";
  حالت TAG_FEATURE_GCS:
    بازگشت "Tag_Feature_GCS";
  حالت FEATURE_AND_BITS_TAG_NOT_FOUND:
  پیشفرض:
    بازگشت "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  بازگشت StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
