//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/AArch64BuildAttributes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringSwitch.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::AArch64_Build_ਗੁਣ;

StringRef AArch64_Build_ਗੁਣ::getVendorName(ਬਿਨਾਨਿਸ਼ਾਨੀ Vendor) {
  ਚੋਣ (Vendor) {
  ਹਾਲਤ AEABI_FEATURE_AND_ਬਿੱਟ:
    ਵਾਪਸ "aeabi_feature_and_bits";
  ਹਾਲਤ AEABI_PAUTHABI:
    ਵਾਪਸ "aeabi_pauthabi";
  ਹਾਲਤ VENDOR_UNKNOWN:
    ਵਾਪਸ "";
  ਮੂਲ:
    ਪ੍ਰਤਿਜ੍ਞਾ(0 && "Vendor name error");
    ਵਾਪਸ "";
  }
}
VendorID AArch64_Build_ਗੁਣ::getVendorID(StringRef Vendor) {
  ਵਾਪਸ StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_ਬਿੱਟ)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64_Build_ਗੁਣ::getOptionalStr(ਬਿਨਾਨਿਸ਼ਾਨੀ Optional) {
  ਚੋਣ (Optional) {
  ਹਾਲਤ REQUIRED:
    ਵਾਪਸ "required";
  ਹਾਲਤ OPTIONAL:
    ਵਾਪਸ "optional";
  ਹਾਲਤ OPTIONAL_NOT_ਮਿਲਿਆ:
  ਮੂਲ:
    ਵਾਪਸ "";
  }
}
SubsectionOptional AArch64_Build_ਗੁਣ::getOptionalID(StringRef Optional) {
  ਵਾਪਸ StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_ਮਿਲਿਆ);
}
StringRef AArch64_Build_ਗੁਣ::getSubsectionOptionalUnknownError() {
  ਵਾਪਸ "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64_Build_ਗੁਣ::getTypeStr(ਬਿਨਾਨਿਸ਼ਾਨੀ Type) {
  ਚੋਣ (Type) {
  ਹਾਲਤ ULEB128:
    ਵਾਪਸ "uleb128";
  ਹਾਲਤ NTBS:
    ਵਾਪਸ "ntbs";
  ਹਾਲਤ TYPE_NOT_ਮਿਲਿਆ:
  ਮੂਲ:
    ਵਾਪਸ "";
  }
}
SubsectionType AArch64_Build_ਗੁਣ::getTypeID(StringRef Type) {
  ਵਾਪਸ StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_ਮਿਲਿਆ);
}
StringRef AArch64_Build_ਗੁਣ::getSubsectionTypeUnknownError() {
  ਵਾਪਸ "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64_Build_ਗੁਣ::getPauthABITagsStr(ਬਿਨਾਨਿਸ਼ਾਨੀ PauthABITag) {
  ਚੋਣ (PauthABITag) {
  ਹਾਲਤ ਟੈਗ_PAUTH_PLATFORM:
    ਵਾਪਸ "Tag_PAuth_Platform";
  ਹਾਲਤ ਟੈਗ_PAUTH_SCHEMA:
    ਵਾਪਸ "Tag_PAuth_Schema";
  ਹਾਲਤ PAUTHABI_ਟੈਗ_NOT_ਮਿਲਿਆ:
  ਮੂਲ:
    ਵਾਪਸ "";
  }
}

PauthABITags AArch64_Build_ਗੁਣ::getPauthABITagsID(StringRef PauthABITag) {
  ਵਾਪਸ StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", ਟੈਗ_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", ਟੈਗ_PAUTH_SCHEMA)
      .Default(PAUTHABI_ਟੈਗ_NOT_ਮਿਲਿਆ);
}

StringRef
AArch64_Build_ਗੁਣ::get_Feature_And_ਬਿੱਟ_ਟੈਗ_ਸੂਚੀ_Str(ਬਿਨਾਨਿਸ਼ਾਨੀ Feature_And_ਬਿੱਟ_ਟੈਗ) {
  ਚੋਣ (Feature_And_ਬਿੱਟ_ਟੈਗ) {
  ਹਾਲਤ ਟੈਗ_FEATURE_BTI:
    ਵਾਪਸ "Tag_Feature_BTI";
  ਹਾਲਤ ਟੈਗ_FEATURE_PAC:
    ਵਾਪਸ "Tag_Feature_PAC";
  ਹਾਲਤ ਟੈਗ_FEATURE_GCS:
    ਵਾਪਸ "Tag_Feature_GCS";
  ਹਾਲਤ FEATURE_AND_ਬਿੱਟ_ਟੈਗ_NOT_ਮਿਲਿਆ:
  ਮੂਲ:
    ਵਾਪਸ "";
  }
}

Feature_And_ਬਿੱਟ_ਟੈਗ_ਸੂਚੀ
AArch64_Build_ਗੁਣ::get_Feature_And_ਬਿੱਟ_ਟੈਗ_ਸੂਚੀ_ID(StringRef Feature_And_ਬਿੱਟ_ਟੈਗ) {
  ਵਾਪਸ StringSwitch<Feature_And_ਬਿੱਟ_ਟੈਗ_ਸੂਚੀ>(Feature_And_ਬਿੱਟ_ਟੈਗ)
      .Case("Tag_Feature_BTI", ਟੈਗ_FEATURE_BTI)
      .Case("Tag_Feature_PAC", ਟੈਗ_FEATURE_PAC)
      .Case("Tag_Feature_GCS", ਟੈਗ_FEATURE_GCS)
      .Default(FEATURE_AND_ਬਿੱਟ_ਟੈਗ_NOT_ਮਿਲਿਆ);
}
