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

std::وکتور<llvm::Subsection_And_برچسب_مقصد_برچسب_Name> &
llvm::AArch64AttributeParser::return_برچسبها_نامها_Map() {
  ساكن std::وکتور<Subsection_And_برچسب_مقصد_برچسب_Name> برچسبها_نامها_Map = {
      {"aeabi_pauthabi", 1, "Tag_PAuth_Platform"},
      {"aeabi_pauthabi", 2, "Tag_PAuth_Schema"},
      {"aeabi_feature_and_bits", 0, "Tag_Feature_BTI"},
      {"aeabi_feature_and_bits", 1, "Tag_Feature_PAC"},
      {"aeabi_feature_and_bits", 2, "Tag_Feature_GCS"}};
  أرجع برچسبها_نامها_Map;
}

llvm::AArch64BuildAttrSubsections llvm::استخراج_Build_صفات_Subsections(
    ثابت llvm::AArch64AttributeParser &صفات) {

  llvm::AArch64BuildAttrSubsections SubSections;
  خودکار Get_Pauth_مقدار = [&صفات](بیعلامت برچسب) {
    أرجع صفات.get_Attribute_مقدار("aeabi_pauthabi", برچسب).مقدار_or(0);
  };
  SubSections.Pauth.برچسب_Platform =
      Get_Pauth_مقدار(llvm::AArch64_Build_صفات::برچسب_PAUTH_PLATFORM);
  SubSections.Pauth.برچسب_Schema =
      Get_Pauth_مقدار(llvm::AArch64_Build_صفات::برچسب_PAUTH_SCHEMA);

  خودکار Get_Feature_مقدار = [&صفات](بیعلامت برچسب) {
    أرجع صفات.get_Attribute_مقدار("aeabi_feature_and_bits", برچسب)
        .مقدار_or(0);
  };
  SubSections.AndFeatures |=
      Get_Feature_مقدار(llvm::AArch64_Build_صفات::برچسب_FEATURE_BTI);
  SubSections.AndFeatures |=
      Get_Feature_مقدار(llvm::AArch64_Build_صفات::برچسب_FEATURE_PAC) << 1;
  SubSections.AndFeatures |=
      Get_Feature_مقدار(llvm::AArch64_Build_صفات::برچسب_FEATURE_GCS) << 2;

  أرجع SubSections;
}
