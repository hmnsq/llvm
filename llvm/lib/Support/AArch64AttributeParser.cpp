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

std::ويڪٽر<llvm::Subsection_And_ٽيگ_منزل_ٽيگ_Name> &
llvm::AArch64AttributeParser::return_ٽيگ_فهرست_نالا_Map() {
  ساكن std::ويڪٽر<Subsection_And_ٽيگ_منزل_ٽيگ_Name> ٽيگ_فهرست_نالا_Map = {
      {"aeabi_pauthabi", 1, "Tag_PAuth_Platform"},
      {"aeabi_pauthabi", 2, "Tag_PAuth_Schema"},
      {"aeabi_feature_and_bits", 0, "Tag_Feature_BTI"},
      {"aeabi_feature_and_bits", 1, "Tag_Feature_PAC"},
      {"aeabi_feature_and_bits", 2, "Tag_Feature_GCS"}};
  أرجع ٽيگ_فهرست_نالا_Map;
}

llvm::AArch64BuildAttrSubsections llvm::ڪڍو_Build_خاصيتون_Subsections(
    ثابت llvm::AArch64AttributeParser &خاصيتون) {

  llvm::AArch64BuildAttrSubsections SubSections;
  خودڪار Get_Pauth_قدر = [&خاصيتون](بنانشان ٽيگ) {
    أرجع خاصيتون.get_Attribute_قدر("aeabi_pauthabi", ٽيگ).قدر_or(0);
  };
  SubSections.Pauth.ٽيگ_Platform =
      Get_Pauth_قدر(llvm::AArch64_Build_خاصيتون::ٽيگ_PAUTH_PLATFORM);
  SubSections.Pauth.ٽيگ_Schema =
      Get_Pauth_قدر(llvm::AArch64_Build_خاصيتون::ٽيگ_PAUTH_SCHEMA);

  خودڪار Get_Feature_قدر = [&خاصيتون](بنانشان ٽيگ) {
    أرجع خاصيتون.get_Attribute_قدر("aeabi_feature_and_bits", ٽيگ)
        .قدر_or(0);
  };
  SubSections.AndFeatures |=
      Get_Feature_قدر(llvm::AArch64_Build_خاصيتون::ٽيگ_FEATURE_BTI);
  SubSections.AndFeatures |=
      Get_Feature_قدر(llvm::AArch64_Build_خاصيتون::ٽيگ_FEATURE_PAC) << 1;
  SubSections.AndFeatures |=
      Get_Feature_قدر(llvm::AArch64_Build_خاصيتون::ٽيگ_FEATURE_GCS) << 2;

  أرجع SubSections;
}
