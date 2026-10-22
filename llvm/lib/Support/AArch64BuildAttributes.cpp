//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/AArch64BuildAttributes.h"
#شامل "llvm/ADT/StringSwitch.h"

استعمال نامخلا llvm;
استعمال نامخلا llvm::AArch64_Build_اوصاف;

StringRef AArch64_Build_اوصاف::getVendorName(غیرعلامتی Vendor) {
  انتخاب (Vendor) {
  حالت AEABI_FEATURE_AND_بٹ:
    واپس "aeabi_feature_and_bits";
  حالت AEABI_PAUTHABI:
    واپس "aeabi_pauthabi";
  حالت VENDOR_UNKNOWN:
    واپس "";
  طےشدہ:
    أكد(0 && "Vendor name error");
    واپس "";
  }
}
VendorID AArch64_Build_اوصاف::getVendorID(StringRef Vendor) {
  واپس StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_بٹ)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64_Build_اوصاف::getOptionalStr(غیرعلامتی Optional) {
  انتخاب (Optional) {
  حالت REQUIRED:
    واپس "required";
  حالت OPTIONAL:
    واپس "optional";
  حالت OPTIONAL_NOT_ملا:
  طےشدہ:
    واپس "";
  }
}
SubsectionOptional AArch64_Build_اوصاف::getOptionalID(StringRef Optional) {
  واپس StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_ملا);
}
StringRef AArch64_Build_اوصاف::getSubsectionOptionalUnknownError() {
  واپس "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64_Build_اوصاف::getTypeStr(غیرعلامتی Type) {
  انتخاب (Type) {
  حالت ULEB128:
    واپس "uleb128";
  حالت NTBS:
    واپس "ntbs";
  حالت TYPE_NOT_ملا:
  طےشدہ:
    واپس "";
  }
}
SubsectionType AArch64_Build_اوصاف::getTypeID(StringRef Type) {
  واپس StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_ملا);
}
StringRef AArch64_Build_اوصاف::getSubsectionTypeUnknownError() {
  واپس "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64_Build_اوصاف::getPauthABITagsStr(غیرعلامتی PauthABITag) {
  انتخاب (PauthABITag) {
  حالت لیبل_PAUTH_PLATFORM:
    واپس "Tag_PAuth_Platform";
  حالت لیبل_PAUTH_SCHEMA:
    واپس "Tag_PAuth_Schema";
  حالت PAUTHABI_لیبل_NOT_ملا:
  طےشدہ:
    واپس "";
  }
}

PauthABITags AArch64_Build_اوصاف::getPauthABITagsID(StringRef PauthABITag) {
  واپس StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", لیبل_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", لیبل_PAUTH_SCHEMA)
      .Default(PAUTHABI_لیبل_NOT_ملا);
}

StringRef
AArch64_Build_اوصاف::get_Feature_And_بٹ_لیبل_فہرست_Str(غیرعلامتی Feature_And_بٹ_لیبل) {
  انتخاب (Feature_And_بٹ_لیبل) {
  حالت لیبل_FEATURE_BTI:
    واپس "Tag_Feature_BTI";
  حالت لیبل_FEATURE_PAC:
    واپس "Tag_Feature_PAC";
  حالت لیبل_FEATURE_GCS:
    واپس "Tag_Feature_GCS";
  حالت FEATURE_AND_بٹ_لیبل_NOT_ملا:
  طےشدہ:
    واپس "";
  }
}

Feature_And_بٹ_لیبل_فہرست
AArch64_Build_اوصاف::get_Feature_And_بٹ_لیبل_فہرست_ID(StringRef Feature_And_بٹ_لیبل) {
  واپس StringSwitch<Feature_And_بٹ_لیبل_فہرست>(Feature_And_بٹ_لیبل)
      .Case("Tag_Feature_BTI", لیبل_FEATURE_BTI)
      .Case("Tag_Feature_PAC", لیبل_FEATURE_PAC)
      .Case("Tag_Feature_GCS", لیبل_FEATURE_GCS)
      .Default(FEATURE_AND_بٹ_لیبل_NOT_ملا);
}
