//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/AArch64BuildAttributes.h"
#അന്തര്ഭാവ "llvm/ADT/StringSwitch.h"

using നാമാകാശ llvm;
using നാമാകാശ llvm::AArch64BuildAttributes;

StringRef AArch64BuildAttributes::getVendorName(unsigned Vendor) {
  തിരഞ്ഞെടുപ്പ് (Vendor) {
  അവസ്ഥ AEABI_FEATURE_AND_BITS:
    തിരികെ "aeabi_feature_and_bits";
  അവസ്ഥ AEABI_PAUTHABI:
    തിരികെ "aeabi_pauthabi";
  അവസ്ഥ VENDOR_UNKNOWN:
    തിരികെ "";
  സ്ഥിരസ്ഥിതി:
    പ്രതിജ്ഞാ(0 && "Vendor name error");
    തിരികെ "";
  }
}
VendorID AArch64BuildAttributes::getVendorID(StringRef Vendor) {
  തിരികെ StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_BITS)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64BuildAttributes::getOptionalStr(unsigned Optional) {
  തിരഞ്ഞെടുപ്പ് (Optional) {
  അവസ്ഥ REQUIRED:
    തിരികെ "required";
  അവസ്ഥ OPTIONAL:
    തിരികെ "optional";
  അവസ്ഥ OPTIONAL_NOT_FOUND:
  സ്ഥിരസ്ഥിതി:
    തിരികെ "";
  }
}
SubsectionOptional AArch64BuildAttributes::getOptionalID(StringRef Optional) {
  തിരികെ StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionOptionalUnknownError() {
  തിരികെ "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64BuildAttributes::getTypeStr(unsigned Type) {
  തിരഞ്ഞെടുപ്പ് (Type) {
  അവസ്ഥ ULEB128:
    തിരികെ "uleb128";
  അവസ്ഥ NTBS:
    തിരികെ "ntbs";
  അവസ്ഥ TYPE_NOT_FOUND:
  സ്ഥിരസ്ഥിതി:
    തിരികെ "";
  }
}
SubsectionType AArch64BuildAttributes::getTypeID(StringRef Type) {
  തിരികെ StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_FOUND);
}
StringRef AArch64BuildAttributes::getSubsectionTypeUnknownError() {
  തിരികെ "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64BuildAttributes::getPauthABITagsStr(unsigned PauthABITag) {
  തിരഞ്ഞെടുപ്പ് (PauthABITag) {
  അവസ്ഥ TAG_PAUTH_PLATFORM:
    തിരികെ "Tag_PAuth_Platform";
  അവസ്ഥ TAG_PAUTH_SCHEMA:
    തിരികെ "Tag_PAuth_Schema";
  അവസ്ഥ PAUTHABI_TAG_NOT_FOUND:
  സ്ഥിരസ്ഥിതി:
    തിരികെ "";
  }
}

PauthABITags AArch64BuildAttributes::getPauthABITagsID(StringRef PauthABITag) {
  തിരികെ StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", TAG_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", TAG_PAUTH_SCHEMA)
      .Default(PAUTHABI_TAG_NOT_FOUND);
}

StringRef
AArch64BuildAttributes::getFeatureAndBitsTagsStr(unsigned FeatureAndBitsTag) {
  തിരഞ്ഞെടുപ്പ് (FeatureAndBitsTag) {
  അവസ്ഥ TAG_FEATURE_BTI:
    തിരികെ "Tag_Feature_BTI";
  അവസ്ഥ TAG_FEATURE_PAC:
    തിരികെ "Tag_Feature_PAC";
  അവസ്ഥ TAG_FEATURE_GCS:
    തിരികെ "Tag_Feature_GCS";
  അവസ്ഥ FEATURE_AND_BITS_TAG_NOT_FOUND:
  സ്ഥിരസ്ഥിതി:
    തിരികെ "";
  }
}

FeatureAndBitsTags
AArch64BuildAttributes::getFeatureAndBitsTagsID(StringRef FeatureAndBitsTag) {
  തിരികെ StringSwitch<FeatureAndBitsTags>(FeatureAndBitsTag)
      .Case("Tag_Feature_BTI", TAG_FEATURE_BTI)
      .Case("Tag_Feature_PAC", TAG_FEATURE_PAC)
      .Case("Tag_Feature_GCS", TAG_FEATURE_GCS)
      .Default(FEATURE_AND_BITS_TAG_NOT_FOUND);
}
