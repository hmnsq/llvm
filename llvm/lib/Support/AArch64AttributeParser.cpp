//===-- AArch64AttributeParser.cpp - AArch64 Build Attributes PArser------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with
// LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#समावेश "llvm/Support/AArch64AttributeParser.h"
#समावेश "llvm/Support/AArch64BuildAttributes.h"

std::vector<llvm::SubsectionAndTagToTagName> &
llvm::AArch64AttributeParser::returnTagsNamesMap() {
  अटल std::vector<SubsectionAndTagToTagName> TagsNamesMap = {
      {"aeabi_pauthabi", 1, "Tag_PAuth_Platform"},
      {"aeabi_pauthabi", 2, "Tag_PAuth_Schema"},
      {"aeabi_feature_and_bits", 0, "Tag_Feature_BTI"},
      {"aeabi_feature_and_bits", 1, "Tag_Feature_PAC"},
      {"aeabi_feature_and_bits", 2, "Tag_Feature_GCS"}};
  वापस TagsNamesMap;
}

llvm::AArch64BuildAttrSubsections llvm::extractBuildAttributesSubsections(
    स्थिर llvm::AArch64AttributeParser &Attributes) {

  llvm::AArch64BuildAttrSubsections SubSections;
  स्वतः Get_Pauth_मूल्य = [&Attributes](अचिन्हित Tag) {
    वापस Attributes.get_Attribute_मूल्य("aeabi_pauthabi", Tag).मूल्य_or(0);
  };
  SubSections.Pauth.TagPlatform =
      Get_Pauth_मूल्य(llvm::AArch64BuildAttributes::TAG_PAUTH_PLATFORM);
  SubSections.Pauth.TagSchema =
      Get_Pauth_मूल्य(llvm::AArch64BuildAttributes::TAG_PAUTH_SCHEMA);

  स्वतः Get_Feature_मूल्य = [&Attributes](अचिन्हित Tag) {
    वापस Attributes.get_Attribute_मूल्य("aeabi_feature_and_bits", Tag)
        .मूल्य_or(0);
  };
  SubSections.AndFeatures |=
      Get_Feature_मूल्य(llvm::AArch64BuildAttributes::TAG_FEATURE_BTI);
  SubSections.AndFeatures |=
      Get_Feature_मूल्य(llvm::AArch64BuildAttributes::TAG_FEATURE_PAC) << 1;
  SubSections.AndFeatures |=
      Get_Feature_मूल्य(llvm::AArch64BuildAttributes::TAG_FEATURE_GCS) << 2;

  वापस SubSections;
}
