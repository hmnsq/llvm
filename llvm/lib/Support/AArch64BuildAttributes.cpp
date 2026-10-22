//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/AArch64BuildAttributes.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringSwitch.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು;

StringRef AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::getVendorName(ಚಿಹ್ನೆರಹಿತ Vendor) {
  ಆಯ್ಕೆ (Vendor) {
  ಸ್ಥಿತಿ AEABI_FEATURE_AND_ಬಿಟ್ಟುಗಳು:
    ಹಿಂತಿರುಗಿ "aeabi_feature_and_bits";
  ಸ್ಥಿತಿ AEABI_PAUTHABI:
    ಹಿಂತಿರುಗಿ "aeabi_pauthabi";
  ಸ್ಥಿತಿ VENDOR_UNKNOWN:
    ಹಿಂತಿರುಗಿ "";
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಪ್ರತಿಜ್ಞಾ(0 && "Vendor name error");
    ಹಿಂತಿರುಗಿ "";
  }
}
VendorID AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::getVendorID(StringRef Vendor) {
  ಹಿಂತಿರುಗಿ StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_ಬಿಟ್ಟುಗಳು)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::getOptionalStr(ಚಿಹ್ನೆರಹಿತ Optional) {
  ಆಯ್ಕೆ (Optional) {
  ಸ್ಥಿತಿ REQUIRED:
    ಹಿಂತಿರುಗಿ "required";
  ಸ್ಥಿತಿ OPTIONAL:
    ಹಿಂತಿರುಗಿ "optional";
  ಸ್ಥಿತಿ OPTIONAL_NOT_ಸಿಕ್ಕಿತು:
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಹಿಂತಿರುಗಿ "";
  }
}
SubsectionOptional AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::getOptionalID(StringRef Optional) {
  ಹಿಂತಿರುಗಿ StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_ಸಿಕ್ಕಿತು);
}
StringRef AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::getSubsectionOptionalUnknownError() {
  ಹಿಂತಿರುಗಿ "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::getTypeStr(ಚಿಹ್ನೆರಹಿತ Type) {
  ಆಯ್ಕೆ (Type) {
  ಸ್ಥಿತಿ ULEB128:
    ಹಿಂತಿರುಗಿ "uleb128";
  ಸ್ಥಿತಿ NTBS:
    ಹಿಂತಿರುಗಿ "ntbs";
  ಸ್ಥಿತಿ TYPE_NOT_ಸಿಕ್ಕಿತು:
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಹಿಂತಿರುಗಿ "";
  }
}
SubsectionType AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::getTypeID(StringRef Type) {
  ಹಿಂತಿರುಗಿ StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_ಸಿಕ್ಕಿತು);
}
StringRef AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::getSubsectionTypeUnknownError() {
  ಹಿಂತಿರುಗಿ "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::getPauthABITagsStr(ಚಿಹ್ನೆರಹಿತ PauthABITag) {
  ಆಯ್ಕೆ (PauthABITag) {
  ಸ್ಥಿತಿ ಟ್ಯಾಗ್_PAUTH_PLATFORM:
    ಹಿಂತಿರುಗಿ "Tag_PAuth_Platform";
  ಸ್ಥಿತಿ ಟ್ಯಾಗ್_PAUTH_SCHEMA:
    ಹಿಂತಿರುಗಿ "Tag_PAuth_Schema";
  ಸ್ಥಿತಿ PAUTHABI_ಟ್ಯಾಗ್_NOT_ಸಿಕ್ಕಿತು:
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಹಿಂತಿರುಗಿ "";
  }
}

PauthABITags AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::getPauthABITagsID(StringRef PauthABITag) {
  ಹಿಂತಿರುಗಿ StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", ಟ್ಯಾಗ್_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", ಟ್ಯಾಗ್_PAUTH_SCHEMA)
      .Default(PAUTHABI_ಟ್ಯಾಗ್_NOT_ಸಿಕ್ಕಿತು);
}

StringRef
AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::get_Feature_And_ಬಿಟ್ಟುಗಳು_ಟ್ಯಾಗುಗಳು_Str(ಚಿಹ್ನೆರಹಿತ Feature_And_ಬಿಟ್ಟುಗಳು_ಟ್ಯಾಗ್) {
  ಆಯ್ಕೆ (Feature_And_ಬಿಟ್ಟುಗಳು_ಟ್ಯಾಗ್) {
  ಸ್ಥಿತಿ ಟ್ಯಾಗ್_FEATURE_BTI:
    ಹಿಂತಿರುಗಿ "Tag_Feature_BTI";
  ಸ್ಥಿತಿ ಟ್ಯಾಗ್_FEATURE_PAC:
    ಹಿಂತಿರುಗಿ "Tag_Feature_PAC";
  ಸ್ಥಿತಿ ಟ್ಯಾಗ್_FEATURE_GCS:
    ಹಿಂತಿರುಗಿ "Tag_Feature_GCS";
  ಸ್ಥಿತಿ FEATURE_AND_ಬಿಟ್ಟುಗಳು_ಟ್ಯಾಗ್_NOT_ಸಿಕ್ಕಿತು:
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಹಿಂತಿರುಗಿ "";
  }
}

Feature_And_ಬಿಟ್ಟುಗಳು_ಟ್ಯಾಗುಗಳು
AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::get_Feature_And_ಬಿಟ್ಟುಗಳು_ಟ್ಯಾಗುಗಳು_ID(StringRef Feature_And_ಬಿಟ್ಟುಗಳು_ಟ್ಯಾಗ್) {
  ಹಿಂತಿರುಗಿ StringSwitch<Feature_And_ಬಿಟ್ಟುಗಳು_ಟ್ಯಾಗುಗಳು>(Feature_And_ಬಿಟ್ಟುಗಳು_ಟ್ಯಾಗ್)
      .Case("Tag_Feature_BTI", ಟ್ಯಾಗ್_FEATURE_BTI)
      .Case("Tag_Feature_PAC", ಟ್ಯಾಗ್_FEATURE_PAC)
      .Case("Tag_Feature_GCS", ಟ್ಯಾಗ್_FEATURE_GCS)
      .Default(FEATURE_AND_ಬಿಟ್ಟುಗಳು_ಟ್ಯಾಗ್_NOT_ಸಿಕ್ಕಿತು);
}
