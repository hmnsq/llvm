//===-- AArch64AttributeParser.cpp - AArch64 Build Attributes PArser------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with
// LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/AArch64AttributeParser.h"
#अन्तर्भाव "llvm/Support/AArch64BuildAttributes.h"

std::सदिशम्<llvm::Subsection_And_चिह्नकम्_गन्तव्यम्_चिह्नकम्_Name> &
llvm::AArch64AttributeParser::return_चिह्नकानि_नामानि_Map() {
  स्थिर std::सदिशम्<Subsection_And_चिह्नकम्_गन्तव्यम्_चिह्नकम्_Name> चिह्नकानि_नामानि_Map = {
      {"aeabi_pauthabi", 1, "Tag_PAuth_Platform"},
      {"aeabi_pauthabi", 2, "Tag_PAuth_Schema"},
      {"aeabi_feature_and_bits", 0, "Tag_Feature_BTI"},
      {"aeabi_feature_and_bits", 1, "Tag_Feature_PAC"},
      {"aeabi_feature_and_bits", 2, "Tag_Feature_GCS"}};
  प्रत्यागम चिह्नकानि_नामानि_Map;
}

llvm::AArch64BuildAttrSubsections llvm::extract_Build_गुणाः_Subsections(
    ध्रुव llvm::AArch64AttributeParser &गुणाः) {

  llvm::AArch64BuildAttrSubsections SubSections;
  स्वचालित Get_Pauth_मानम् = [&गुणाः](अचिह्नित चिह्नकम्) {
    प्रत्यागम गुणाः.get_Attribute_मानम्("aeabi_pauthabi", चिह्नकम्).मानम्_or(0);
  };
  SubSections.Pauth.चिह्नकम्_Platform =
      Get_Pauth_मानम्(llvm::AArch64_Build_गुणाः::चिह्नकम्_PAUTH_PLATFORM);
  SubSections.Pauth.चिह्नकम्_Schema =
      Get_Pauth_मानम्(llvm::AArch64_Build_गुणाः::चिह्नकम्_PAUTH_SCHEMA);

  स्वचालित Get_Feature_मानम् = [&गुणाः](अचिह्नित चिह्नकम्) {
    प्रत्यागम गुणाः.get_Attribute_मानम्("aeabi_feature_and_bits", चिह्नकम्)
        .मानम्_or(0);
  };
  SubSections.AndFeatures |=
      Get_Feature_मानम्(llvm::AArch64_Build_गुणाः::चिह्नकम्_FEATURE_BTI);
  SubSections.AndFeatures |=
      Get_Feature_मानम्(llvm::AArch64_Build_गुणाः::चिह्नकम्_FEATURE_PAC) << 1;
  SubSections.AndFeatures |=
      Get_Feature_मानम्(llvm::AArch64_Build_गुणाः::चिह्नकम्_FEATURE_GCS) << 2;

  प्रत्यागम SubSections;
}
