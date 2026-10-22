//===-- AArch64AttributeParser.cpp - AArch64 Build Attributes PArser------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with
// LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/AArch64AttributeParser.h"
#அந்தர்பாவ "llvm/Support/AArch64BuildAttributes.h"

std::vector<llvm::SubsectionAndTagToTagName> &
llvm::AArch64AttributeParser::returnTagsNamesMap() {
  நிலையான std::vector<SubsectionAndTagToTagName> TagsNamesMap = {
      {"aeabi_pauthabi", 1, "Tag_PAuth_Platform"},
      {"aeabi_pauthabi", 2, "Tag_PAuth_Schema"},
      {"aeabi_feature_and_bits", 0, "Tag_Feature_BTI"},
      {"aeabi_feature_and_bits", 1, "Tag_Feature_PAC"},
      {"aeabi_feature_and_bits", 2, "Tag_Feature_GCS"}};
  திரும்பு TagsNamesMap;
}

llvm::AArch64BuildAttrSubsections llvm::extractBuildAttributesSubsections(
    மாறிலி llvm::AArch64AttributeParser &Attributes) {

  llvm::AArch64BuildAttrSubsections SubSections;
  auto Get_Pauth_மதிப்பு = [&Attributes](unsigned Tag) {
    திரும்பு Attributes.get_Attribute_மதிப்பு("aeabi_pauthabi", Tag).மதிப்பு_or(0);
  };
  SubSections.Pauth.TagPlatform =
      Get_Pauth_மதிப்பு(llvm::AArch64BuildAttributes::TAG_PAUTH_PLATFORM);
  SubSections.Pauth.TagSchema =
      Get_Pauth_மதிப்பு(llvm::AArch64BuildAttributes::TAG_PAUTH_SCHEMA);

  auto Get_Feature_மதிப்பு = [&Attributes](unsigned Tag) {
    திரும்பு Attributes.get_Attribute_மதிப்பு("aeabi_feature_and_bits", Tag)
        .மதிப்பு_or(0);
  };
  SubSections.AndFeatures |=
      Get_Feature_மதிப்பு(llvm::AArch64BuildAttributes::TAG_FEATURE_BTI);
  SubSections.AndFeatures |=
      Get_Feature_மதிப்பு(llvm::AArch64BuildAttributes::TAG_FEATURE_PAC) << 1;
  SubSections.AndFeatures |=
      Get_Feature_மதிப்பு(llvm::AArch64BuildAttributes::TAG_FEATURE_GCS) << 2;

  திரும்பு SubSections;
}
