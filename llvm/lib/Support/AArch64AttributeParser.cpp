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

std::سمتیہ<llvm::Subsection_And_ٹیگ_منزل_ٹیگ_Name> &
llvm::AArch64AttributeParser::return_ٹیگ_سوچی_ناں_Map() {
  ساكن std::سمتیہ<Subsection_And_ٹیگ_منزل_ٹیگ_Name> ٹیگ_سوچی_ناں_Map = {
      {"aeabi_pauthabi", 1, "Tag_PAuth_Platform"},
      {"aeabi_pauthabi", 2, "Tag_PAuth_Schema"},
      {"aeabi_feature_and_bits", 0, "Tag_Feature_BTI"},
      {"aeabi_feature_and_bits", 1, "Tag_Feature_PAC"},
      {"aeabi_feature_and_bits", 2, "Tag_Feature_GCS"}};
  أرجع ٹیگ_سوچی_ناں_Map;
}

llvm::AArch64BuildAttrSubsections llvm::کڈھو_Build_گن_Subsections(
    ثابت llvm::AArch64AttributeParser &گن) {

  llvm::AArch64BuildAttrSubsections SubSections;
  خودکار Get_Pauth_قدر = [&گن](بنانشانی ٹیگ) {
    أرجع گن.get_Attribute_قدر("aeabi_pauthabi", ٹیگ).قدر_or(0);
  };
  SubSections.Pauth.ٹیگ_Platform =
      Get_Pauth_قدر(llvm::AArch64_Build_گن::ٹیگ_PAUTH_PLATFORM);
  SubSections.Pauth.ٹیگ_Schema =
      Get_Pauth_قدر(llvm::AArch64_Build_گن::ٹیگ_PAUTH_SCHEMA);

  خودکار Get_Feature_قدر = [&گن](بنانشانی ٹیگ) {
    أرجع گن.get_Attribute_قدر("aeabi_feature_and_bits", ٹیگ)
        .قدر_or(0);
  };
  SubSections.AndFeatures |=
      Get_Feature_قدر(llvm::AArch64_Build_گن::ٹیگ_FEATURE_BTI);
  SubSections.AndFeatures |=
      Get_Feature_قدر(llvm::AArch64_Build_گن::ٹیگ_FEATURE_PAC) << 1;
  SubSections.AndFeatures |=
      Get_Feature_قدر(llvm::AArch64_Build_گن::ٹیگ_FEATURE_GCS) << 2;

  أرجع SubSections;
}
