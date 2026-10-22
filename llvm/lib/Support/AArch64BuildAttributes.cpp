//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/AArch64BuildAttributes.h"
#અન્તર્ભાવ "llvm/ADT/StringSwitch.h"

using નામાકાશ llvm;
using નામાકાશ llvm::AArch64BuildAttributes;

StringRef AArch64BuildAttributes::getVendorName(unsigned Vendor) {
  પસંદગી (Vendor) {
  સ્થિતિ AEABI_FEATURE_AND_BITS:
    પાછા "aeabi_feature_and_bits";
  સ્થિતિ AEABI_PAUTHABI:
    પાછા "aeabi_pauthabi";
  સ્થિતિ VENDOR_UNKNOWN:
    પાછા "";
  મૂળભૂત:
    પ્રતિજ્ઞા(0 && "Vendor name error");
    પાછા "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  પાછા StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(unsigned Optional) {
  પસંદગી (Optional) {
  સ્થિતિ REQUIRED:
    પાછા "required";
  સ્થિતિ OPTIONAL:
    પાછા "optional";
  સ્થિતિ OPTIONAL_NOT_FOUND:
  મૂળભૂત:
    પાછા "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  પાછા StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  પાછા "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(unsigned Type) {
  પસંદગી (Type) {
  સ્થિતિ ULEB128:
    પાછા "uleb128";
  સ્થિતિ NTBS:
    પાછા "ntbs";
  સ્થિતિ TYPE_NOT_FOUND:
  મૂળભૂત:
    પાછા "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  પાછા StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  પાછા "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(unsigned PauthABITag) {
  પસંદગી (PauthABITag) {
  સ્થિતિ TAG_PAUTH_PLATFORM:
    પાછા "Tag_PAuth_Platform";
  સ્થિતિ TAG_PAUTH_SCHEMA:
    પાછા "Tag_PAuth_Schema";
  સ્થિતિ PAUTHABI_TAG_NOT_FOUND:
  મૂળભૂત:
    પાછા "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  પાછા StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(unsigned FeatureAndBitsTag) {
  પસંદગી (FeatureAndBitsTag) {
  સ્થિતિ TAG_FEATURE_BTI:
    પાછા "Tag_Feature_BTI";
  સ્થિતિ TAG_FEATURE_PAC:
    પાછા "Tag_Feature_PAC";
  સ્થિતિ TAG_FEATURE_GCS:
    પાછા "Tag_Feature_GCS";
  સ્થિતિ FEATURE_AND_BITS_TAG_NOT_FOUND:
  મૂળભૂત:
    પાછા "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  પાછા StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
