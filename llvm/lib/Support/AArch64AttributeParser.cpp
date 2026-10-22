//===-- AArch64AttributeParser.cpp - AArch64 Build Attributes PArser------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with
// LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/AArch64AttributeParser.h"
#અન્તર્ભાવ "llvm/Support/AArch64BuildAttributes.h"

std::vector<llvm::SubsectionAndTagToTagName> &
llvm::AArch64AttributeParser::returnTagsNamesMap() {
  સ્થિર std::vector<SubsectionAndTagToTagName> TagsNamesMap = {
      {"aeabi_pauthabi", 1, "Tag_PAuth_Platform"},
      {"aeabi_pauthabi", 2, "Tag_PAuth_Schema"},
      {"aeabi_feature_and_bits", 0, "Tag_Feature_BTI"},
      {"aeabi_feature_and_bits", 1, "Tag_Feature_PAC"},
      {"aeabi_feature_and_bits", 2, "Tag_Feature_GCS"}};
  પાછા TagsNamesMap;
}

llvm::AArch64BuildAttrSubsections llvm::extractBuildAttributesSubsections(
    અચલ llvm::AArch64AttributeParser &Attributes) {

  llvm::AArch64BuildAttrSubsections SubSections;
  auto Get_Pauth_કિંમત = [&Attributes](unsigned Tag) {
    પાછા Attributes.get_Attribute_કિંમત("aeabi_pauthabi", Tag).કિંમત_or(0);
  };
  SubSections.Pauth.TagPlatform =
      Get_Pauth_કિંમત(llvm::AArch64BuildAttributes::TAG_PAUTH_PLATFORM);
  SubSections.Pauth.TagSchema =
      Get_Pauth_કિંમત(llvm::AArch64BuildAttributes::TAG_PAUTH_SCHEMA);

  auto Get_Feature_કિંમત = [&Attributes](unsigned Tag) {
    પાછા Attributes.get_Attribute_કિંમત("aeabi_feature_and_bits", Tag)
        .કિંમત_or(0);
  };
  SubSections.AndFeatures |=
      Get_Feature_કિંમત(llvm::AArch64BuildAttributes::TAG_FEATURE_BTI);
  SubSections.AndFeatures |=
      Get_Feature_કિંમત(llvm::AArch64BuildAttributes::TAG_FEATURE_PAC) << 1;
  SubSections.AndFeatures |=
      Get_Feature_કિંમત(llvm::AArch64BuildAttributes::TAG_FEATURE_GCS) << 2;

  પાછા SubSections;
}
