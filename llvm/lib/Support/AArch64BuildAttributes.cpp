//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/AArch64BuildAttributes.h"
#ضمن "llvm/ADT/StringSwitch.h"

using نطاق llvm;
using نطاق llvm::AArch64BuildAttributes;

StringRef AArch64BuildAttributes::getVendorName(unsigned Vendor) {
  بدل (Vendor) {
  حالة AEABI_FEATURE_AND_BITS:
    إرجاع "aeabi_feature_and_bits";
  حالة AEABI_PAUTHABI:
    إرجاع "aeabi_pauthabi";
  حالة VENDOR_UNKNOWN:
    إرجاع "";
  افتراضي:
    أكد(0 && "Vendor name error");
    إرجاع "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  إرجاع StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(unsigned Optional) {
  بدل (Optional) {
  حالة REQUIRED:
    إرجاع "required";
  حالة OPTIONAL:
    إرجاع "optional";
  حالة OPTIONAL_NOT_FOUND:
  افتراضي:
    إرجاع "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  إرجاع StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  إرجاع "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(unsigned Type) {
  بدل (Type) {
  حالة ULEB128:
    إرجاع "uleb128";
  حالة NTBS:
    إرجاع "ntbs";
  حالة TYPE_NOT_FOUND:
  افتراضي:
    إرجاع "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  إرجاع StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  إرجاع "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(unsigned PauthABITag) {
  بدل (PauthABITag) {
  حالة TAG_PAUTH_PLATFORM:
    إرجاع "Tag_PAuth_Platform";
  حالة TAG_PAUTH_SCHEMA:
    إرجاع "Tag_PAuth_Schema";
  حالة PAUTHABI_TAG_NOT_FOUND:
  افتراضي:
    إرجاع "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  إرجاع StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(unsigned FeatureAndBitsTag) {
  بدل (FeatureAndBitsTag) {
  حالة TAG_FEATURE_BTI:
    إرجاع "Tag_Feature_BTI";
  حالة TAG_FEATURE_PAC:
    إرجاع "Tag_Feature_PAC";
  حالة TAG_FEATURE_GCS:
    إرجاع "Tag_Feature_GCS";
  حالة FEATURE_AND_BITS_TAG_NOT_FOUND:
  افتراضي:
    إرجاع "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  إرجاع StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
