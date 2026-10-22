//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/AArch64BuildAttributes.h"
#ضمن "llvm/ADT/StringSwitch.h"

کارول نطاق llvm;
کارول نطاق llvm::AArch64_Build_ځانګړتیاوې;

StringRef AArch64_Build_ځانګړتیاوې::getVendorName(بینښه Vendor) {
  بدل (Vendor) {
  حالة AEABI_FEATURE_AND_بټونه:
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
VendorID AArch64_Build_ځانګړتیاوې::getVendorID(StringRef Vendor) {
  أرجع StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_بټونه)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64_Build_ځانګړتیاوې::getOptionalStr(بینښه Optional) {
  بدل (Optional) {
  حالة REQUIRED:
    أرجع "required";
  حالة OPTIONAL:
    أرجع "optional";
  حالة OPTIONAL_NOT_وموندل:
  افتراضي:
    أرجع "";
  }
}
SubsectionOptional AArch64_Build_ځانګړتیاوې::getOptionalID(StringRef Optional) {
  أرجع StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_وموندل);
}
StringRef AArch64_Build_ځانګړتیاوې::getSubsectionOptionalUnknownError() {
  أرجع "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64_Build_ځانګړتیاوې::getTypeStr(بینښه Type) {
  بدل (Type) {
  حالة ULEB128:
    أرجع "uleb128";
  حالة NTBS:
    أرجع "ntbs";
  حالة TYPE_NOT_وموندل:
  افتراضي:
    أرجع "";
  }
}
SubsectionType AArch64_Build_ځانګړتیاوې::getTypeID(StringRef Type) {
  أرجع StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_وموندل);
}
StringRef AArch64_Build_ځانګړتیاوې::getSubsectionTypeUnknownError() {
  أرجع "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64_Build_ځانګړتیاوې::getPauthABITagsStr(بینښه PauthABITag) {
  بدل (PauthABITag) {
  حالة نښان_PAUTH_PLATFORM:
    أرجع "Tag_PAuth_Platform";
  حالة نښان_PAUTH_SCHEMA:
    أرجع "Tag_PAuth_Schema";
  حالة PAUTHABI_نښان_NOT_وموندل:
  افتراضي:
    أرجع "";
  }
}

PauthABITags AArch64_Build_ځانګړتیاوې::getPauthABITagsID(StringRef PauthABITag) {
  أرجع StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", نښان_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", نښان_PAUTH_SCHEMA)
      .Default(PAUTHABI_نښان_NOT_وموندل);
}

StringRef
AArch64_Build_ځانګړتیاوې::get_Feature_And_بټونه_نښانونه_Str(بینښه Feature_And_بټونه_نښان) {
  بدل (Feature_And_بټونه_نښان) {
  حالة نښان_FEATURE_BTI:
    أرجع "Tag_Feature_BTI";
  حالة نښان_FEATURE_PAC:
    أرجع "Tag_Feature_PAC";
  حالة نښان_FEATURE_GCS:
    أرجع "Tag_Feature_GCS";
  حالة FEATURE_AND_بټونه_نښان_NOT_وموندل:
  افتراضي:
    أرجع "";
  }
}

Feature_And_بټونه_نښانونه
AArch64_Build_ځانګړتیاوې::get_Feature_And_بټونه_نښانونه_ID(StringRef Feature_And_بټونه_نښان) {
  أرجع StringSwitch<Feature_And_بټونه_نښانونه>(Feature_And_بټونه_نښان)
      .Case("Tag_Feature_BTI", نښان_FEATURE_BTI)
      .Case("Tag_Feature_PAC", نښان_FEATURE_PAC)
      .Case("Tag_Feature_GCS", نښان_FEATURE_GCS)
      .Default(FEATURE_AND_بټونه_نښان_NOT_وموندل);
}
