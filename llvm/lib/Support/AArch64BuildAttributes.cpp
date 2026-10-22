//===-- AArch64BuildAttributes.cpp - AArch64 Build Attributes -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/AArch64BuildAttributes.h"
#అన్తర్భావ "llvm/ADT/StringSwitch.h"

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::AArch64_Build_లక్షణాలు;

StringRef AArch64_Build_లక్షణాలు::getVendorName(గుర్తులేని Vendor) {
  ఎంపిక (Vendor) {
  స్థితి AEABI_FEATURE_AND_బిట్లు:
    తిరిగి "aeabi_feature_and_bits";
  స్థితి AEABI_PAUTHABI:
    తిరిగి "aeabi_pauthabi";
  స్థితి VENDOR_UNKNOWN:
    తిరిగి "";
  అప్రమేయ:
    ప్రతిజ్ఞా(0 && "Vendor name error");
    తిరిగి "";
  }
}
VendorID AArch64_Build_లక్షణాలు::getVendorID(StringRef Vendor) {
  తిరిగి StringSwitch<VendorID>(Vendor)
      .Case("aeabi_feature_and_bits", AEABI_FEATURE_AND_బిట్లు)
      .Case("aeabi_pauthabi", AEABI_PAUTHABI)
      .Default(VENDOR_UNKNOWN);
}

StringRef AArch64_Build_లక్షణాలు::getOptionalStr(గుర్తులేని Optional) {
  ఎంపిక (Optional) {
  స్థితి REQUIRED:
    తిరిగి "required";
  స్థితి OPTIONAL:
    తిరిగి "optional";
  స్థితి OPTIONAL_NOT_దొరికింది:
  అప్రమేయ:
    తిరిగి "";
  }
}
SubsectionOptional AArch64_Build_లక్షణాలు::getOptionalID(StringRef Optional) {
  తిరిగి StringSwitch<SubsectionOptional>(Optional)
      .Case("required", REQUIRED)
      .Case("optional", OPTIONAL)
      .Default(OPTIONAL_NOT_దొరికింది);
}
StringRef AArch64_Build_లక్షణాలు::getSubsectionOptionalUnknownError() {
  తిరిగి "unknown AArch64 build attributes optionality, expected "
         "required|optional";
}

StringRef AArch64_Build_లక్షణాలు::getTypeStr(గుర్తులేని Type) {
  ఎంపిక (Type) {
  స్థితి ULEB128:
    తిరిగి "uleb128";
  స్థితి NTBS:
    తిరిగి "ntbs";
  స్థితి TYPE_NOT_దొరికింది:
  అప్రమేయ:
    తిరిగి "";
  }
}
SubsectionType AArch64_Build_లక్షణాలు::getTypeID(StringRef Type) {
  తిరిగి StringSwitch<SubsectionType>(Type)
      .Cases({"uleb128", "ULEB128"}, ULEB128)
      .Cases({"ntbs", "NTBS"}, NTBS)
      .Default(TYPE_NOT_దొరికింది);
}
StringRef AArch64_Build_లక్షణాలు::getSubsectionTypeUnknownError() {
  తిరిగి "unknown AArch64 build attributes type, expected uleb128|ntbs";
}

StringRef AArch64_Build_లక్షణాలు::getPauthABITagsStr(గుర్తులేని PauthABITag) {
  ఎంపిక (PauthABITag) {
  స్థితి ట్యాగ్_PAUTH_PLATFORM:
    తిరిగి "Tag_PAuth_Platform";
  స్థితి ట్యాగ్_PAUTH_SCHEMA:
    తిరిగి "Tag_PAuth_Schema";
  స్థితి PAUTHABI_ట్యాగ్_NOT_దొరికింది:
  అప్రమేయ:
    తిరిగి "";
  }
}

PauthABITags AArch64_Build_లక్షణాలు::getPauthABITagsID(StringRef PauthABITag) {
  తిరిగి StringSwitch<PauthABITags>(PauthABITag)
      .Case("Tag_PAuth_Platform", ట్యాగ్_PAUTH_PLATFORM)
      .Case("Tag_PAuth_Schema", ట్యాగ్_PAUTH_SCHEMA)
      .Default(PAUTHABI_ట్యాగ్_NOT_దొరికింది);
}

StringRef
AArch64_Build_లక్షణాలు::get_Feature_And_బిట్లు_ట్యాగులు_Str(గుర్తులేని Feature_And_బిట్లు_ట్యాగ్) {
  ఎంపిక (Feature_And_బిట్లు_ట్యాగ్) {
  స్థితి ట్యాగ్_FEATURE_BTI:
    తిరిగి "Tag_Feature_BTI";
  స్థితి ట్యాగ్_FEATURE_PAC:
    తిరిగి "Tag_Feature_PAC";
  స్థితి ట్యాగ్_FEATURE_GCS:
    తిరిగి "Tag_Feature_GCS";
  స్థితి FEATURE_AND_బిట్లు_ట్యాగ్_NOT_దొరికింది:
  అప్రమేయ:
    తిరిగి "";
  }
}

Feature_And_బిట్లు_ట్యాగులు
AArch64_Build_లక్షణాలు::get_Feature_And_బిట్లు_ట్యాగులు_ID(StringRef Feature_And_బిట్లు_ట్యాగ్) {
  తిరిగి StringSwitch<Feature_And_బిట్లు_ట్యాగులు>(Feature_And_బిట్లు_ట్యాగ్)
      .Case("Tag_Feature_BTI", ట్యాగ్_FEATURE_BTI)
      .Case("Tag_Feature_PAC", ట్యాగ్_FEATURE_PAC)
      .Case("Tag_Feature_GCS", ట్యాగ్_FEATURE_GCS)
      .Default(FEATURE_AND_బిట్లు_ట్యాగ్_NOT_దొరికింది);
}
