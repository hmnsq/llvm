//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/AArch64BuildAttributes.h"
#ضمن "llvm/ADT/StringSwitch.h"

استعمال نطاق llvm;
استعمال نطاق llvm::AArch64_Build_خاصيتون;

StringRef AArch64_Build_خاصيتون::getVendorName(بنانشان Vendor) {
  بدل (Vendor) {
  حالة AEABI_FEATURE_AND_بٽ:
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
VendorID AArch64_Build_خاصيتون::getVendorID(StringRef Vendor) {
  أرجع StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_بٽ)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64_Build_خاصيتون::getOptionalStr(بنانشان Optional) {
  بدل (Optional) {
  حالة REQUIRED:
    أرجع "required";
  حالة OPTIONAL:
    أرجع "optional";
  حالة OPTIONAL_NOT_مليو:
  افتراضي:
    أرجع "";
  }
}
SubsectionOptional AArch64_Build_خاصيتون::getOptionalID(StringRef Optional) {
  أرجع StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_مليو);
}
StringRef AArch64_Build_خاصيتون::getSubsectionOptionalUnknownError() {
  أرجع "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64_Build_خاصيتون::getTypeStr(بنانشان Type) {
  بدل (Type) {
  حالة ULEB128:
    أرجع "uleb128";
  حالة NTBS:
    أرجع "ntbs";
  حالة TYPE_NOT_مليو:
  افتراضي:
    أرجع "";
  }
}
SubsectionType AArch64_Build_خاصيتون::getTypeID(StringRef Type) {
  أرجع StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_مليو);
}
StringRef AArch64_Build_خاصيتون::getSubsectionTypeUnknownError() {
  أرجع "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64_Build_خاصيتون::getPauthABITagsStr(بنانشان PauthABITag) {
  بدل (PauthABITag) {
  حالة ٽيگ_PAUTH_PLATFORM:
    أرجع "Tag_PAuth_Platform";
  حالة ٽيگ_PAUTH_SCHEMA:
    أرجع "Tag_PAuth_Schema";
  حالة PAUTHABI_ٽيگ_NOT_مليو:
  افتراضي:
    أرجع "";
  }
}

PauthABITags AArch64_Build_خاصيتون::getPauthABITagsID(StringRef PauthABITag) {
  أرجع StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", ٽيگ_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", ٽيگ_PAUTH_SCHEMA)
      .Default(PAUTHABI_ٽيگ_NOT_مليو);
}

StringRef
AArch64_Build_خاصيتون::get_Feature_And_بٽ_ٽيگ_فهرست_Str(بنانشان Feature_And_بٽ_ٽيگ) {
  بدل (Feature_And_بٽ_ٽيگ) {
  حالة ٽيگ_FEATURE_BTI:
    أرجع "Tag_Feature_BTI";
  حالة ٽيگ_FEATURE_PAC:
    أرجع "Tag_Feature_PAC";
  حالة ٽيگ_FEATURE_GCS:
    أرجع "Tag_Feature_GCS";
  حالة FEATURE_AND_بٽ_ٽيگ_NOT_مليو:
  افتراضي:
    أرجع "";
  }
}

Feature_And_بٽ_ٽيگ_فهرست
AArch64_Build_خاصيتون::get_Feature_And_بٽ_ٽيگ_فهرست_ID(StringRef Feature_And_بٽ_ٽيگ) {
  أرجع StringSwitch<Feature_And_بٽ_ٽيگ_فهرست>(Feature_And_بٽ_ٽيگ)
      .Case("Tag_Feature_BTI", ٽيگ_FEATURE_BTI)
      .Case("Tag_Feature_PAC", ٽيگ_FEATURE_PAC)
      .Case("Tag_Feature_GCS", ٽيگ_FEATURE_GCS)
      .Default(FEATURE_AND_بٽ_ٽيگ_NOT_مليو);
}
