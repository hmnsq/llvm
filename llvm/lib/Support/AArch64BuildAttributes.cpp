//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/AArch64BuildAttributes.h"
#அந்தர்பாவ "llvm/ADT/StringSwitch.h"

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::AArch64BuildAttributes;

StringRef AArch64BuildAttributes::getVendorName(unsigned Vendor) {
  தேர்வு (Vendor) {
  நிலை AEABI_FEATURE_AND_BITS:
    திரும்பு "aeabi_feature_and_bits";
  நிலை AEABI_PAUTHABI:
    திரும்பு "aeabi_pauthabi";
  நிலை VENDOR_UNKNOWN:
    திரும்பு "";
  இயல்பு:
    ப்ரதிஜ்ஞா(0 && "Vendor name error");
    திரும்பு "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  திரும்பு StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(unsigned Optional) {
  தேர்வு (Optional) {
  நிலை REQUIRED:
    திரும்பு "required";
  நிலை OPTIONAL:
    திரும்பு "optional";
  நிலை OPTIONAL_NOT_FOUND:
  இயல்பு:
    திரும்பு "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  திரும்பு StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  திரும்பு "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(unsigned Type) {
  தேர்வு (Type) {
  நிலை ULEB128:
    திரும்பு "uleb128";
  நிலை NTBS:
    திரும்பு "ntbs";
  நிலை TYPE_NOT_FOUND:
  இயல்பு:
    திரும்பு "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  திரும்பு StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  திரும்பு "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(unsigned PauthABITag) {
  தேர்வு (PauthABITag) {
  நிலை TAG_PAUTH_PLATFORM:
    திரும்பு "Tag_PAuth_Platform";
  நிலை TAG_PAUTH_SCHEMA:
    திரும்பு "Tag_PAuth_Schema";
  நிலை PAUTHABI_TAG_NOT_FOUND:
  இயல்பு:
    திரும்பு "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  திரும்பு StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(unsigned FeatureAndBitsTag) {
  தேர்வு (FeatureAndBitsTag) {
  நிலை TAG_FEATURE_BTI:
    திரும்பு "Tag_Feature_BTI";
  நிலை TAG_FEATURE_PAC:
    திரும்பு "Tag_Feature_PAC";
  நிலை TAG_FEATURE_GCS:
    திரும்பு "Tag_Feature_GCS";
  நிலை FEATURE_AND_BITS_TAG_NOT_FOUND:
  இயல்பு:
    திரும்பு "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  திரும்பு StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
