//===-- AArch64AttributeParser.cpp - AArch64 Build Attributes PArser------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with
// LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/AArch64AttributeParser.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/AArch64BuildAttributes.h"

std::ಸದಿಶ<llvm::Subsection_And_ಟ್ಯಾಗ್_ಗಮ್ಯ_ಟ್ಯಾಗ್_Name> &
llvm::AArch64AttributeParser::return_ಟ್ಯಾಗುಗಳು_ಹೆಸರುಗಳು_Map() {
  ಸ್ಥಿರ std::ಸದಿಶ<Subsection_And_ಟ್ಯಾಗ್_ಗಮ್ಯ_ಟ್ಯಾಗ್_Name> ಟ್ಯಾಗುಗಳು_ಹೆಸರುಗಳು_Map = {
      {"aeabi_pauthabi", 1, "Tag_PAuth_Platform"},
      {"aeabi_pauthabi", 2, "Tag_PAuth_Schema"},
      {"aeabi_feature_and_bits", 0, "Tag_Feature_BTI"},
      {"aeabi_feature_and_bits", 1, "Tag_Feature_PAC"},
      {"aeabi_feature_and_bits", 2, "Tag_Feature_GCS"}};
  ಹಿಂತಿರುಗಿ ಟ್ಯಾಗುಗಳು_ಹೆಸರುಗಳು_Map;
}

llvm::AArch64BuildAttrSubsections llvm::ಹೊರತೆಗೆ_Build_ಗುಣಲಕ್ಷಣಗಳು_Subsections(
    ಸ್ಥಿರಾಂಕ llvm::AArch64AttributeParser &ಗುಣಲಕ್ಷಣಗಳು) {

  llvm::AArch64BuildAttrSubsections SubSections;
  ಸ್ವಯಂಚಾಲಿತ Get_Pauth_ಮೌಲ್ಯ = [&ಗುಣಲಕ್ಷಣಗಳು](ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್) {
    ಹಿಂತಿರುಗಿ ಗುಣಲಕ್ಷಣಗಳು.get_Attribute_ಮೌಲ್ಯ("aeabi_pauthabi", ಟ್ಯಾಗ್).ಮೌಲ್ಯ_or(0);
  };
  SubSections.Pauth.ಟ್ಯಾಗ್_Platform =
      Get_Pauth_ಮೌಲ್ಯ(llvm::AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::ಟ್ಯಾಗ್_PAUTH_PLATFORM);
  SubSections.Pauth.ಟ್ಯಾಗ್_Schema =
      Get_Pauth_ಮೌಲ್ಯ(llvm::AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::ಟ್ಯಾಗ್_PAUTH_SCHEMA);

  ಸ್ವಯಂಚಾಲಿತ Get_Feature_ಮೌಲ್ಯ = [&ಗುಣಲಕ್ಷಣಗಳು](ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್) {
    ಹಿಂತಿರುಗಿ ಗುಣಲಕ್ಷಣಗಳು.get_Attribute_ಮೌಲ್ಯ("aeabi_feature_and_bits", ಟ್ಯಾಗ್)
        .ಮೌಲ್ಯ_or(0);
  };
  SubSections.AndFeatures |=
      Get_Feature_ಮೌಲ್ಯ(llvm::AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::ಟ್ಯಾಗ್_FEATURE_BTI);
  SubSections.AndFeatures |=
      Get_Feature_ಮೌಲ್ಯ(llvm::AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::ಟ್ಯಾಗ್_FEATURE_PAC) << 1;
  SubSections.AndFeatures |=
      Get_Feature_ಮೌಲ್ಯ(llvm::AArch64_Build_ಗುಣಲಕ್ಷಣಗಳು::ಟ್ಯಾಗ್_FEATURE_GCS) << 2;

  ಹಿಂತಿರುಗಿ SubSections;
}
