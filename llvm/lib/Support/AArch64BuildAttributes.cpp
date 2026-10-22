//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/AArch64BuildAttributes.h"
#समावेश "llvm/ADT/StringSwitch.h"

using namespace llvm;
using namespace llvm::AArch64BuildAttributes;

StringRef AArch64BuildAttributes::getVendorName(अचिन्हित Vendor) {
  चयन (Vendor) {
  हाल AEABI_FEATURE_AND_BITS:
    वापस "aeabi_feature_and_bits";
  हाल AEABI_PAUTHABI:
    वापस "aeabi_pauthabi";
  हाल VENDOR_UNKNOWN:
    वापस "";
  शेष:
    निश्चित(0 && "Vendor name error");
    वापस "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  वापस StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(अचिन्हित Optional) {
  चयन (Optional) {
  हाल REQUIRED:
    वापस "required";
  हाल OPTIONAL:
    वापस "optional";
  हाल OPTIONAL_NOT_FOUND:
  शेष:
    वापस "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  वापस StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  वापस "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(अचिन्हित Type) {
  चयन (Type) {
  हाल ULEB128:
    वापस "uleb128";
  हाल NTBS:
    वापस "ntbs";
  हाल TYPE_NOT_FOUND:
  शेष:
    वापस "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  वापस StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  वापस "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(अचिन्हित PauthABITag) {
  चयन (PauthABITag) {
  हाल TAG_PAUTH_PLATFORM:
    वापस "Tag_PAuth_Platform";
  हाल TAG_PAUTH_SCHEMA:
    वापस "Tag_PAuth_Schema";
  हाल PAUTHABI_TAG_NOT_FOUND:
  शेष:
    वापस "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  वापस StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(अचिन्हित FeatureAndBitsTag) {
  चयन (FeatureAndBitsTag) {
  हाल TAG_FEATURE_BTI:
    वापस "Tag_Feature_BTI";
  हाल TAG_FEATURE_PAC:
    वापस "Tag_Feature_PAC";
  हाल TAG_FEATURE_GCS:
    वापस "Tag_Feature_GCS";
  हाल FEATURE_AND_BITS_TAG_NOT_FOUND:
  शेष:
    वापस "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  वापस StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
