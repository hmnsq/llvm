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
  निवड (Vendor) {
  स्थिती AEABI_FEATURE_AND_BITS:
    परत "aeabi_feature_and_bits";
  स्थिती AEABI_PAUTHABI:
    परत "aeabi_pauthabi";
  स्थिती VENDOR_UNKNOWN:
    परत "";
  पूर्वनिर्धारित:
    प्रतिज्ञा(0 && "Vendor name error");
    परत "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  परत StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(अचिन्हित Optional) {
  निवड (Optional) {
  स्थिती REQUIRED:
    परत "required";
  स्थिती OPTIONAL:
    परत "optional";
  स्थिती OPTIONAL_NOT_FOUND:
  पूर्वनिर्धारित:
    परत "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  परत StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  परत "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(अचिन्हित Type) {
  निवड (Type) {
  स्थिती ULEB128:
    परत "uleb128";
  स्थिती NTBS:
    परत "ntbs";
  स्थिती TYPE_NOT_FOUND:
  पूर्वनिर्धारित:
    परत "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  परत StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  परत "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(अचिन्हित PauthABITag) {
  निवड (PauthABITag) {
  स्थिती TAG_PAUTH_PLATFORM:
    परत "Tag_PAuth_Platform";
  स्थिती TAG_PAUTH_SCHEMA:
    परत "Tag_PAuth_Schema";
  स्थिती PAUTHABI_TAG_NOT_FOUND:
  पूर्वनिर्धारित:
    परत "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  परत StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(अचिन्हित FeatureAndBitsTag) {
  निवड (FeatureAndBitsTag) {
  स्थिती TAG_FEATURE_BTI:
    परत "Tag_Feature_BTI";
  स्थिती TAG_FEATURE_PAC:
    परत "Tag_Feature_PAC";
  स्थिती TAG_FEATURE_GCS:
    परत "Tag_Feature_GCS";
  स्थिती FEATURE_AND_BITS_TAG_NOT_FOUND:
  पूर्वनिर्धारित:
    परत "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  परत StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
