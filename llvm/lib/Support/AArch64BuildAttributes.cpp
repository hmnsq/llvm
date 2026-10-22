//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/AArch64BuildAttributes.h"
#अन्तर्भाव "llvm/ADT/StringSwitch.h"

using नामाकाश llvm;
using नामाकाश llvm::AArch64BuildAttributes;

StringRef AArch64BuildAttributes::getVendorName(अचिन्हित Vendor) {
  विकल्प (Vendor) {
  पक्ष AEABI_FEATURE_AND_BITS:
    फर्काऊ "aeabi_feature_and_bits";
  पक्ष AEABI_PAUTHABI:
    फर्काऊ "aeabi_pauthabi";
  पक्ष VENDOR_UNKNOWN:
    फर्काऊ "";
  स्वभाव:
    प्रतिज्ञा(0 && "Vendor name error");
    फर्काऊ "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  फर्काऊ StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(अचिन्हित Optional) {
  विकल्प (Optional) {
  पक्ष REQUIRED:
    फर्काऊ "required";
  पक्ष OPTIONAL:
    फर्काऊ "optional";
  पक्ष OPTIONAL_NOT_FOUND:
  स्वभाव:
    फर्काऊ "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  फर्काऊ StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  फर्काऊ "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(अचिन्हित Type) {
  विकल्प (Type) {
  पक्ष ULEB128:
    फर्काऊ "uleb128";
  पक्ष NTBS:
    फर्काऊ "ntbs";
  पक्ष TYPE_NOT_FOUND:
  स्वभाव:
    फर्काऊ "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  फर्काऊ StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  फर्काऊ "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(अचिन्हित PauthABITag) {
  विकल्प (PauthABITag) {
  पक्ष TAG_PAUTH_PLATFORM:
    फर्काऊ "Tag_PAuth_Platform";
  पक्ष TAG_PAUTH_SCHEMA:
    फर्काऊ "Tag_PAuth_Schema";
  पक्ष PAUTHABI_TAG_NOT_FOUND:
  स्वभाव:
    फर्काऊ "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  फर्काऊ StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(अचिन्हित FeatureAndBitsTag) {
  विकल्प (FeatureAndBitsTag) {
  पक्ष TAG_FEATURE_BTI:
    फर्काऊ "Tag_Feature_BTI";
  पक्ष TAG_FEATURE_PAC:
    फर्काऊ "Tag_Feature_PAC";
  पक्ष TAG_FEATURE_GCS:
    फर्काऊ "Tag_Feature_GCS";
  पक्ष FEATURE_AND_BITS_TAG_NOT_FOUND:
  स्वभाव:
    फर्काऊ "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  फर्काऊ StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
