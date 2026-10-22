//===-- AArch64AttributeParser.cpp - AArch64 Build Attributes PArser------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with
// LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/AArch64AttributeParser.h"
#అన్తర్భావ "llvm/Support/AArch64BuildAttributes.h"

std::సదిశ<llvm::Subsection_And_ట్యాగ్_గమ్యం_ట్యాగ్_Name> &
llvm::AArch64AttributeParser::return_ట్యాగులు_పేర్లు_Map() {
  స్థిర std::సదిశ<Subsection_And_ట్యాగ్_గమ్యం_ట్యాగ్_Name> ట్యాగులు_పేర్లు_Map = {
      {"aeabi_pauthabi", 1, "Tag_PAuth_Platform"},
      {"aeabi_pauthabi", 2, "Tag_PAuth_Schema"},
      {"aeabi_feature_and_bits", 0, "Tag_Feature_BTI"},
      {"aeabi_feature_and_bits", 1, "Tag_Feature_PAC"},
      {"aeabi_feature_and_bits", 2, "Tag_Feature_GCS"}};
  తిరిగి ట్యాగులు_పేర్లు_Map;
}

llvm::AArch64BuildAttrSubsections llvm::వెలికితీయి_Build_లక్షణాలు_Subsections(
    స్థిరరాశి llvm::AArch64AttributeParser &లక్షణాలు) {

  llvm::AArch64BuildAttrSubsections SubSections;
  స్వయంచాలక Get_Pauth_విలువ = [&లక్షణాలు](గుర్తులేని ట్యాగ్) {
    తిరిగి లక్షణాలు.get_Attribute_విలువ("aeabi_pauthabi", ట్యాగ్).విలువ_or(0);
  };
  SubSections.Pauth.ట్యాగ్_Platform =
      Get_Pauth_విలువ(llvm::AArch64_Build_లక్షణాలు::ట్యాగ్_PAUTH_PLATFORM);
  SubSections.Pauth.ట్యాగ్_Schema =
      Get_Pauth_విలువ(llvm::AArch64_Build_లక్షణాలు::ట్యాగ్_PAUTH_SCHEMA);

  స్వయంచాలక Get_Feature_విలువ = [&లక్షణాలు](గుర్తులేని ట్యాగ్) {
    తిరిగి లక్షణాలు.get_Attribute_విలువ("aeabi_feature_and_bits", ట్యాగ్)
        .విలువ_or(0);
  };
  SubSections.AndFeatures |=
      Get_Feature_విలువ(llvm::AArch64_Build_లక్షణాలు::ట్యాగ్_FEATURE_BTI);
  SubSections.AndFeatures |=
      Get_Feature_విలువ(llvm::AArch64_Build_లక్షణాలు::ట్యాగ్_FEATURE_PAC) << 1;
  SubSections.AndFeatures |=
      Get_Feature_విలువ(llvm::AArch64_Build_లక్షణాలు::ట్యాగ్_FEATURE_GCS) << 2;

  తిరిగి SubSections;
}
