//===-- AArch64AttributeParser.cpp - AArch64 Build Attributes PArser------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with
// LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#ضمن "llvm/Support/AArch64AttributeParser.h"
#ضمن "llvm/Support/AArch64BuildAttributes.h"

std::vector<llvm::SubsectionAndTagToTagName> &
llvm::AArch64AttributeParser::returnTagsNamesMap() {
  ساكن std::vector<SubsectionAndTagToTagName> TagsNamesMap = {
      {"aeabi_pauthabi", 1, "Tag_PAuth_Platform"},
      {"aeabi_pauthabi", 2, "Tag_PAuth_Schema"},
      {"aeabi_feature_and_bits", 0, "Tag_Feature_BTI"},
      {"aeabi_feature_and_bits", 1, "Tag_Feature_PAC"},
      {"aeabi_feature_and_bits", 2, "Tag_Feature_GCS"}};
  أرجع TagsNamesMap;
}

llvm::AArch64BuildAttrSubsections llvm::extractBuildAttributesSubsections(
    ثابت llvm::AArch64AttributeParser &Attributes) {

  llvm::AArch64BuildAttrSubsections SubSections;
  auto Get_Pauth_نيلاي = [&Attributes](unsigned Tag) {
    أرجع Attributes.get_Attribute_نيلاي("aeabi_pauthabi", Tag).نيلاي_or(0);
  };
  SubSections.Pauth.TagPlatform =
      Get_Pauth_نيلاي(llvm::AArch64BuildAttributes::TAG_PAUTH_PLATFORM);
  SubSections.Pauth.TagSchema =
      Get_Pauth_نيلاي(llvm::AArch64BuildAttributes::TAG_PAUTH_SCHEMA);

  auto Get_Feature_نيلاي = [&Attributes](unsigned Tag) {
    أرجع Attributes.get_Attribute_نيلاي("aeabi_feature_and_bits", Tag)
        .نيلاي_or(0);
  };
  SubSections.AndFeatures |=
      Get_Feature_نيلاي(llvm::AArch64BuildAttributes::TAG_FEATURE_BTI);
  SubSections.AndFeatures |=
      Get_Feature_نيلاي(llvm::AArch64BuildAttributes::TAG_FEATURE_PAC) << 1;
  SubSections.AndFeatures |=
      Get_Feature_نيلاي(llvm::AArch64BuildAttributes::TAG_FEATURE_GCS) << 2;

  أرجع SubSections;
}
