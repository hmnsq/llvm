//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/AArch64BuildAttributes.h"
#ضمن "llvm/ADT/StringSwitch.h"

استفاده نطاق llvm;
استفاده نطاق llvm::AArch64_Build_صفات;

StringRef AArch64_Build_صفات::getVendorName(بیعلامت Vendor) {
  بدل (Vendor) {
  حالة AEABI_FEATURE_AND_بیتها:
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
VendorID AArch64_Build_صفات::getVendorID(StringRef Vendor) {
  أرجع StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_بیتها)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64_Build_صفات::getOptionalStr(بیعلامت Optional) {
  بدل (Optional) {
  حالة REQUIRED:
    أرجع "required";
  حالة OPTIONAL:
    أرجع "optional";
  حالة OPTIONAL_NOT_یافت:
  افتراضي:
    أرجع "";
  }
}
SubsectionOptional AArch64_Build_صفات::getOptionalID(StringRef Optional) {
  أرجع StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_یافت);
}
StringRef AArch64_Build_صفات::getSubsectionOptionalUnknownError() {
  أرجع "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64_Build_صفات::getTypeStr(بیعلامت Type) {
  بدل (Type) {
  حالة ULEB128:
    أرجع "uleb128";
  حالة NTBS:
    أرجع "ntbs";
  حالة TYPE_NOT_یافت:
  افتراضي:
    أرجع "";
  }
}
SubsectionType AArch64_Build_صفات::getTypeID(StringRef Type) {
  أرجع StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_یافت);
}
StringRef AArch64_Build_صفات::getSubsectionTypeUnknownError() {
  أرجع "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64_Build_صفات::getPauthABITagsStr(بیعلامت PauthABITag) {
  بدل (PauthABITag) {
  حالة برچسب_PAUTH_PLATFORM:
    أرجع "Tag_PAuth_Platform";
  حالة برچسب_PAUTH_SCHEMA:
    أرجع "Tag_PAuth_Schema";
  حالة PAUTHABI_برچسب_NOT_یافت:
  افتراضي:
    أرجع "";
  }
}

PauthABITags AArch64_Build_صفات::getPauthABITagsID(StringRef PauthABITag) {
  أرجع StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", برچسب_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", برچسب_PAUTH_SCHEMA)
      .Default(PAUTHABI_برچسب_NOT_یافت);
}

StringRef
AArch64_Build_صفات::get_Feature_And_بیتها_برچسبها_Str(بیعلامت Feature_And_بیتها_برچسب) {
  بدل (Feature_And_بیتها_برچسب) {
  حالة برچسب_FEATURE_BTI:
    أرجع "Tag_Feature_BTI";
  حالة برچسب_FEATURE_PAC:
    أرجع "Tag_Feature_PAC";
  حالة برچسب_FEATURE_GCS:
    أرجع "Tag_Feature_GCS";
  حالة FEATURE_AND_بیتها_برچسب_NOT_یافت:
  افتراضي:
    أرجع "";
  }
}

Feature_And_بیتها_برچسبها
AArch64_Build_صفات::get_Feature_And_بیتها_برچسبها_ID(StringRef Feature_And_بیتها_برچسب) {
  أرجع StringSwitch<Feature_And_بیتها_برچسبها>(Feature_And_بیتها_برچسب)
      .Case("Tag_Feature_BTI", برچسب_FEATURE_BTI)
      .Case("Tag_Feature_PAC", برچسب_FEATURE_PAC)
      .Case("Tag_Feature_GCS", برچسب_FEATURE_GCS)
      .Default(FEATURE_AND_بیتها_برچسب_NOT_یافت);
}
