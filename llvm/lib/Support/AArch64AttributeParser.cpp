//===-- AArch64AttributeParser.cpp - AArch64 Build Attributes PArser------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with
// LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/AArch64AttributeParser.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/AArch64BuildAttributes.h"

std::ਸਦਿਸ਼<llvm::Subsection_And_ਟੈਗ_ਮੰਜ਼ਿਲ_ਟੈਗ_Name> &
llvm::AArch64AttributeParser::return_ਟੈਗ_ਸੂਚੀ_ਨਾਂ_Map() {
  ਸਥਿਰ std::ਸਦਿਸ਼<Subsection_And_ਟੈਗ_ਮੰਜ਼ਿਲ_ਟੈਗ_Name> ਟੈਗ_ਸੂਚੀ_ਨਾਂ_Map = {
      {"aeabi_pauthabi", 1, "Tag_PAuth_Platform"},
      {"aeabi_pauthabi", 2, "Tag_PAuth_Schema"},
      {"aeabi_feature_and_bits", 0, "Tag_Feature_BTI"},
      {"aeabi_feature_and_bits", 1, "Tag_Feature_PAC"},
      {"aeabi_feature_and_bits", 2, "Tag_Feature_GCS"}};
  ਵਾਪਸ ਟੈਗ_ਸੂਚੀ_ਨਾਂ_Map;
}

llvm::AArch64BuildAttrSubsections llvm::ਕੱਢੋ_Build_ਗੁਣ_Subsections(
    ਅਚਲ llvm::AArch64AttributeParser &ਗੁਣ) {

  llvm::AArch64BuildAttrSubsections SubSections;
  ਸਵੈਚਾਲਿਤ Get_Pauth_ਮੁੱਲ = [&ਗੁਣ](ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ) {
    ਵਾਪਸ ਗੁਣ.get_Attribute_ਮੁੱਲ("aeabi_pauthabi", ਟੈਗ).ਮੁੱਲ_or(0);
  };
  SubSections.Pauth.ਟੈਗ_Platform =
      Get_Pauth_ਮੁੱਲ(llvm::AArch64_Build_ਗੁਣ::ਟੈਗ_PAUTH_PLATFORM);
  SubSections.Pauth.ਟੈਗ_Schema =
      Get_Pauth_ਮੁੱਲ(llvm::AArch64_Build_ਗੁਣ::ਟੈਗ_PAUTH_SCHEMA);

  ਸਵੈਚਾਲਿਤ Get_Feature_ਮੁੱਲ = [&ਗੁਣ](ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ) {
    ਵਾਪਸ ਗੁਣ.get_Attribute_ਮੁੱਲ("aeabi_feature_and_bits", ਟੈਗ)
        .ਮੁੱਲ_or(0);
  };
  SubSections.AndFeatures |=
      Get_Feature_ਮੁੱਲ(llvm::AArch64_Build_ਗੁਣ::ਟੈਗ_FEATURE_BTI);
  SubSections.AndFeatures |=
      Get_Feature_ਮੁੱਲ(llvm::AArch64_Build_ਗੁਣ::ਟੈਗ_FEATURE_PAC) << 1;
  SubSections.AndFeatures |=
      Get_Feature_ਮੁੱਲ(llvm::AArch64_Build_ਗੁਣ::ਟੈਗ_FEATURE_GCS) << 2;

  ਵਾਪਸ SubSections;
}
