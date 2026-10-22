//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/AArch64BuildAttributes.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringSwitch.h"

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::AArch64BuildAttributes;

StringRef AArch64BuildAttributes::getVendorName(unsigned Vendor) {
  ವಿಕಲ್ಪ (Vendor) {
  ಪಕ್ಷ AEABI_FEATURE_AND_BITS:
    ಪ್ರತ್ಯಾಗಮ "aeabi_feature_and_bits";
  ಪಕ್ಷ AEABI_PAUTHABI:
    ಪ್ರತ್ಯಾಗಮ "aeabi_pauthabi";
  ಪಕ್ಷ VENDOR_UNKNOWN:
    ಪ್ರತ್ಯಾಗಮ "";
  ಸ್ವಭಾವ:
    ಪ್ರತಿಜ್ಞಾ(0 && "Vendor name error");
    ಪ್ರತ್ಯಾಗಮ "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  ಪ್ರತ್ಯಾಗಮ StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(unsigned Optional) {
  ವಿಕಲ್ಪ (Optional) {
  ಪಕ್ಷ REQUIRED:
    ಪ್ರತ್ಯಾಗಮ "required";
  ಪಕ್ಷ OPTIONAL:
    ಪ್ರತ್ಯಾಗಮ "optional";
  ಪಕ್ಷ OPTIONAL_NOT_FOUND:
  ಸ್ವಭಾವ:
    ಪ್ರತ್ಯಾಗಮ "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  ಪ್ರತ್ಯಾಗಮ StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  ಪ್ರತ್ಯಾಗಮ "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(unsigned Type) {
  ವಿಕಲ್ಪ (Type) {
  ಪಕ್ಷ ULEB128:
    ಪ್ರತ್ಯಾಗಮ "uleb128";
  ಪಕ್ಷ NTBS:
    ಪ್ರತ್ಯಾಗಮ "ntbs";
  ಪಕ್ಷ TYPE_NOT_FOUND:
  ಸ್ವಭಾವ:
    ಪ್ರತ್ಯಾಗಮ "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  ಪ್ರತ್ಯಾಗಮ StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  ಪ್ರತ್ಯಾಗಮ "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(unsigned PauthABITag) {
  ವಿಕಲ್ಪ (PauthABITag) {
  ಪಕ್ಷ TAG_PAUTH_PLATFORM:
    ಪ್ರತ್ಯಾಗಮ "Tag_PAuth_Platform";
  ಪಕ್ಷ TAG_PAUTH_SCHEMA:
    ಪ್ರತ್ಯಾಗಮ "Tag_PAuth_Schema";
  ಪಕ್ಷ PAUTHABI_TAG_NOT_FOUND:
  ಸ್ವಭಾವ:
    ಪ್ರತ್ಯಾಗಮ "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  ಪ್ರತ್ಯಾಗಮ StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(unsigned FeatureAndBitsTag) {
  ವಿಕಲ್ಪ (FeatureAndBitsTag) {
  ಪಕ್ಷ TAG_FEATURE_BTI:
    ಪ್ರತ್ಯಾಗಮ "Tag_Feature_BTI";
  ಪಕ್ಷ TAG_FEATURE_PAC:
    ಪ್ರತ್ಯಾಗಮ "Tag_Feature_PAC";
  ಪಕ್ಷ TAG_FEATURE_GCS:
    ಪ್ರತ್ಯಾಗಮ "Tag_Feature_GCS";
  ಪಕ್ಷ FEATURE_AND_BITS_TAG_NOT_FOUND:
  ಸ್ವಭಾವ:
    ಪ್ರತ್ಯಾಗಮ "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  ಪ್ರತ್ಯಾಗಮ StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
