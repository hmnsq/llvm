//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/AArch64BuildAttributes.h"
#ضمن "llvm/ADT/StringSwitch.h"

ورتوں نطاق llvm;
ورتوں نطاق llvm::AArch64_Build_گن;

StringRef AArch64_Build_گن::getVendorName(بنانشانی Vendor) {
  بدل (Vendor) {
  حالة AEABI_FEATURE_AND_بٹ:
    أرجع "aeabi_feature_and_bits";
  حالة AEABI_PAUTHABI:
    أرجع "aeabi_pauthabi";
  حالة VENDOR_UNKNOWN:
    أرجع "";
  افتراضي:
    أكد(0 && "Vendor name error");
    أرجع "";
  }
}
VendorID AArch64_Build_گن::getVendorID(StringRef Vendor) {
  أرجع StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_بٹ)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64_Build_گن::getOptionalStr(بنانشانی Optional) {
  بدل (Optional) {
  حالة REQUIRED:
    أرجع "required";
  حالة OPTIONAL:
    أرجع "optional";
  حالة OPTIONAL_NOT_لبھیا:
  افتراضي:
    أرجع "";
  }
}
SubsectionOptional AArch64_Build_گن::getOptionalID(StringRef Optional) {
  أرجع StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_لبھیا);
}
StringRef AArch64_Build_گن::getSubsectionOptionalUnknownError() {
  أرجع "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64_Build_گن::getTypeStr(بنانشانی Type) {
  بدل (Type) {
  حالة ULEB128:
    أرجع "uleb128";
  حالة NTBS:
    أرجع "ntbs";
  حالة TYPE_NOT_لبھیا:
  افتراضي:
    أرجع "";
  }
}
SubsectionType AArch64_Build_گن::getTypeID(StringRef Type) {
  أرجع StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_لبھیا);
}
StringRef AArch64_Build_گن::getSubsectionTypeUnknownError() {
  أرجع "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64_Build_گن::getPauthABITagsStr(بنانشانی PauthABITag) {
  بدل (PauthABITag) {
  حالة ٹیگ_PAUTH_PLATFORM:
    أرجع "Tag_PAuth_Platform";
  حالة ٹیگ_PAUTH_SCHEMA:
    أرجع "Tag_PAuth_Schema";
  حالة PAUTHABI_ٹیگ_NOT_لبھیا:
  افتراضي:
    أرجع "";
  }
}

PauthABITags AArch64_Build_گن::getPauthABITagsID(StringRef PauthABITag) {
  أرجع StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", ٹیگ_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", ٹیگ_PAUTH_SCHEMA)
      .Default(PAUTHABI_ٹیگ_NOT_لبھیا);
}

StringRef
AArch64_Build_گن::get_Feature_And_بٹ_ٹیگ_سوچی_Str(بنانشانی Feature_And_بٹ_ٹیگ) {
  بدل (Feature_And_بٹ_ٹیگ) {
  حالة ٹیگ_FEATURE_BTI:
    أرجع "Tag_Feature_BTI";
  حالة ٹیگ_FEATURE_PAC:
    أرجع "Tag_Feature_PAC";
  حالة ٹیگ_FEATURE_GCS:
    أرجع "Tag_Feature_GCS";
  حالة FEATURE_AND_بٹ_ٹیگ_NOT_لبھیا:
  افتراضي:
    أرجع "";
  }
}

Feature_And_بٹ_ٹیگ_سوچی
AArch64_Build_گن::get_Feature_And_بٹ_ٹیگ_سوچی_ID(StringRef Feature_And_بٹ_ٹیگ) {
  أرجع StringSwitch<Feature_And_بٹ_ٹیگ_سوچی>(Feature_And_بٹ_ٹیگ)
      .Case("Tag_Feature_BTI", ٹیگ_FEATURE_BTI)
      .Case("Tag_Feature_PAC", ٹیگ_FEATURE_PAC)
      .Case("Tag_Feature_GCS", ٹیگ_FEATURE_GCS)
      .Default(FEATURE_AND_بٹ_ٹیگ_NOT_لبھیا);
}
