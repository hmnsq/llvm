//===-- AArch64AttributeParser.cpp - AArch64 Build Attributes PArser------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with
// LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#شامل "llvm/Support/AArch64AttributeParser.h"
#شامل "llvm/Support/AArch64BuildAttributes.h"

std::سمتیہ<llvm::Subsection_And_لیبل_مقصد_لیبل_Name> &
llvm::AArch64AttributeParser::return_لیبل_فہرست_نام_Map() {
  ساکن std::سمتیہ<Subsection_And_لیبل_مقصد_لیبل_Name> لیبل_فہرست_نام_Map = {
      {"aeabi_pauthabi", 1, "Tag_PAuth_Platform"},
      {"aeabi_pauthabi", 2, "Tag_PAuth_Schema"},
      {"aeabi_feature_and_bits", 0, "Tag_Feature_BTI"},
      {"aeabi_feature_and_bits", 1, "Tag_Feature_PAC"},
      {"aeabi_feature_and_bits", 2, "Tag_Feature_GCS"}};
  واپس لیبل_فہرست_نام_Map;
}

llvm::AArch64BuildAttrSubsections llvm::نکالو_Build_اوصاف_Subsections(
    ثابت llvm::AArch64AttributeParser &اوصاف) {

  llvm::AArch64BuildAttrSubsections SubSections;
  خودکار Get_Pauth_قدر = [&اوصاف](غیرعلامتی لیبل) {
    واپس اوصاف.get_Attribute_قدر("aeabi_pauthabi", لیبل).قدر_or(0);
  };
  SubSections.Pauth.لیبل_Platform =
      Get_Pauth_قدر(llvm::AArch64_Build_اوصاف::لیبل_PAUTH_PLATFORM);
  SubSections.Pauth.لیبل_Schema =
      Get_Pauth_قدر(llvm::AArch64_Build_اوصاف::لیبل_PAUTH_SCHEMA);

  خودکار Get_Feature_قدر = [&اوصاف](غیرعلامتی لیبل) {
    واپس اوصاف.get_Attribute_قدر("aeabi_feature_and_bits", لیبل)
        .قدر_or(0);
  };
  SubSections.AndFeatures |=
      Get_Feature_قدر(llvm::AArch64_Build_اوصاف::لیبل_FEATURE_BTI);
  SubSections.AndFeatures |=
      Get_Feature_قدر(llvm::AArch64_Build_اوصاف::لیبل_FEATURE_PAC) << 1;
  SubSections.AndFeatures |=
      Get_Feature_قدر(llvm::AArch64_Build_اوصاف::لیبل_FEATURE_GCS) << 2;

  واپس SubSections;
}
