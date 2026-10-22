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

std::ویکټور<llvm::Subsection_And_نښان_منزل_نښان_Name> &
llvm::AArch64AttributeParser::return_نښانونه_نومونه_Map() {
  ساكن std::ویکټور<Subsection_And_نښان_منزل_نښان_Name> نښانونه_نومونه_Map = {
      {"aeabi_pauthabi", 1, "Tag_PAuth_Platform"},
      {"aeabi_pauthabi", 2, "Tag_PAuth_Schema"},
      {"aeabi_feature_and_bits", 0, "Tag_Feature_BTI"},
      {"aeabi_feature_and_bits", 1, "Tag_Feature_PAC"},
      {"aeabi_feature_and_bits", 2, "Tag_Feature_GCS"}};
  أرجع نښانونه_نومونه_Map;
}

llvm::AArch64BuildAttrSubsections llvm::ایستل_Build_ځانګړتیاوې_Subsections(
    ثابت llvm::AArch64AttributeParser &ځانګړتیاوې) {

  llvm::AArch64BuildAttrSubsections SubSections;
  خپلکاری Get_Pauth_ارزښت = [&ځانګړتیاوې](بینښه نښان) {
    أرجع ځانګړتیاوې.get_Attribute_ارزښت("aeabi_pauthabi", نښان).ارزښت_or(0);
  };
  SubSections.Pauth.نښان_Platform =
      Get_Pauth_ارزښت(llvm::AArch64_Build_ځانګړتیاوې::نښان_PAUTH_PLATFORM);
  SubSections.Pauth.نښان_Schema =
      Get_Pauth_ارزښت(llvm::AArch64_Build_ځانګړتیاوې::نښان_PAUTH_SCHEMA);

  خپلکاری Get_Feature_ارزښت = [&ځانګړتیاوې](بینښه نښان) {
    أرجع ځانګړتیاوې.get_Attribute_ارزښت("aeabi_feature_and_bits", نښان)
        .ارزښت_or(0);
  };
  SubSections.AndFeatures |=
      Get_Feature_ارزښت(llvm::AArch64_Build_ځانګړتیاوې::نښان_FEATURE_BTI);
  SubSections.AndFeatures |=
      Get_Feature_ارزښت(llvm::AArch64_Build_ځانګړتیاوې::نښان_FEATURE_PAC) << 1;
  SubSections.AndFeatures |=
      Get_Feature_ارزښت(llvm::AArch64_Build_ځانګړتیاوې::نښان_FEATURE_GCS) << 2;

  أرجع SubSections;
}
