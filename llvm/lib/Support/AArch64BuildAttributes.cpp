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
  বিকল্প (Vendor) {
  পক্ষ AEABI_FEATURE_AND_BITS:
    প্রত্যাগম "aeabi_feature_and_bits";
  পক্ষ AEABI_PAUTHABI:
    প্রত্যাগম "aeabi_pauthabi";
  পক্ষ VENDOR_UNKNOWN:
    প্রত্যাগম "";
  স্বভাব:
    প্রতিজ্ঞা(0 && "Vendor name error");
    প্রত্যাগম "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  প্রত্যাগম StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(unsigned Optional) {
  বিকল্প (Optional) {
  পক্ষ REQUIRED:
    প্রত্যাগম "required";
  পক্ষ OPTIONAL:
    প্রত্যাগম "optional";
  পক্ষ OPTIONAL_NOT_FOUND:
  স্বভাব:
    প্রত্যাগম "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  প্রত্যাগম StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  প্রত্যাগম "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(unsigned Type) {
  বিকল্প (Type) {
  পক্ষ ULEB128:
    প্রত্যাগম "uleb128";
  পক্ষ NTBS:
    প্রত্যাগম "ntbs";
  পক্ষ TYPE_NOT_FOUND:
  স্বভাব:
    প্রত্যাগম "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  প্রত্যাগম StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  প্রত্যাগম "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(unsigned PauthABITag) {
  বিকল্প (PauthABITag) {
  পক্ষ TAG_PAUTH_PLATFORM:
    প্রত্যাগম "Tag_PAuth_Platform";
  পক্ষ TAG_PAUTH_SCHEMA:
    প্রত্যাগম "Tag_PAuth_Schema";
  পক্ষ PAUTHABI_TAG_NOT_FOUND:
  স্বভাব:
    প্রত্যাগম "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  প্রত্যাগম StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(unsigned FeatureAndBitsTag) {
  বিকল্প (FeatureAndBitsTag) {
  পক্ষ TAG_FEATURE_BTI:
    প্রত্যাগম "Tag_Feature_BTI";
  পক্ষ TAG_FEATURE_PAC:
    প্রত্যাগম "Tag_Feature_PAC";
  পক্ষ TAG_FEATURE_GCS:
    প্রত্যাগম "Tag_Feature_GCS";
  পক্ষ FEATURE_AND_BITS_TAG_NOT_FOUND:
  স্বভাব:
    প্রত্যাগম "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  প্রত্যাগম StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
