//===-- AArch64AttributeParser.cpp - AArch64 Build Attributes PArser------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with
// LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/AArch64AttributeParser.h"
#അന്തര്ഭാവ "llvm/Support/AArch64BuildAttributes.h"

std::vector<llvm::SubsectionAndTagToTagName> &
llvm::AArch64AttributeParser::returnTagsNamesMap() {
  സ്ഥിതം std::vector<SubsectionAndTagToTagName> TagsNamesMap = {
      {"aeabi_pauthabi", 1, "Tag_PAuth_Platform"},
      {"aeabi_pauthabi", 2, "Tag_PAuth_Schema"},
      {"aeabi_feature_and_bits", 0, "Tag_Feature_BTI"},
      {"aeabi_feature_and_bits", 1, "Tag_Feature_PAC"},
      {"aeabi_feature_and_bits", 2, "Tag_Feature_GCS"}};
  തിരികെ TagsNamesMap;
}

llvm::AArch64BuildAttrSubsections llvm::extractBuildAttributesSubsections(
    സ്ഥിരം llvm::AArch64AttributeParser &Attributes) {

  llvm::AArch64BuildAttrSubsections SubSections;
  auto Get_Pauth_മൂല്യം = [&Attributes](unsigned Tag) {
    തിരികെ Attributes.get_Attribute_മൂല്യം("aeabi_pauthabi", Tag).മൂല്യം_or(0);
  };
  SubSections.Pauth.TagPlatform =
      Get_Pauth_മൂല്യം(llvm::AArch64BuildAttributes::TAG_PAUTH_PLATFORM);
  SubSections.Pauth.TagSchema =
      Get_Pauth_മൂല്യം(llvm::AArch64BuildAttributes::TAG_PAUTH_SCHEMA);

  auto Get_Feature_മൂല്യം = [&Attributes](unsigned Tag) {
    തിരികെ Attributes.get_Attribute_മൂല്യം("aeabi_feature_and_bits", Tag)
        .മൂല്യം_or(0);
  };
  SubSections.AndFeatures |=
      Get_Feature_മൂല്യം(llvm::AArch64BuildAttributes::TAG_FEATURE_BTI);
  SubSections.AndFeatures |=
      Get_Feature_മൂല്യം(llvm::AArch64BuildAttributes::TAG_FEATURE_PAC) << 1;
  SubSections.AndFeatures |=
      Get_Feature_മൂല്യം(llvm::AArch64BuildAttributes::TAG_FEATURE_GCS) << 2;

  തിരികെ SubSections;
}
