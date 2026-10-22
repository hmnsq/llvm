//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/ELFAttributes.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"

ఉపయోగించి నామాకాశ llvm;

StringRef ELFAttrs::attrTypeAsString(గుర్తులేని attr, ట్యాగ్_Name_Map ట్యాగ్_Name_Map_2,
                                     తార్కిక has_ట్యాగ్_ఉపసర్గ) {
  స్వయంచాలక ట్యాగ్_Name_It = కనుగొను_if(
      ట్యాగ్_Name_Map_2, [attr](స్థిరరాశి ట్యాగ్_Name_అంశం అంశం_2) { తిరిగి అంశం_2.attr == attr; });
  ఒకవేళ (ట్యాగ్_Name_It == ట్యాగ్_Name_Map_2.చివర())
    తిరిగి "";
  StringRef ట్యాగ్_Name = ట్యాగ్_Name_It->ట్యాగ్_Name;
  తిరిగి has_ట్యాగ్_ఉపసర్గ ? ట్యాగ్_Name : ట్యాగ్_Name.drop_front(4);
}

std::optional<గుర్తులేని> ELFAttrs::attrTypeFromString(StringRef ట్యాగ్_2,
                                                     ట్యాగ్_Name_Map ట్యాగ్_Name_Map_2) {
  తార్కిక has_ట్యాగ్_ఉపసర్గ = ట్యాగ్_2.starts_with("Tag_");
  స్వయంచాలక ట్యాగ్_Name_It =
      కనుగొను_if(ట్యాగ్_Name_Map_2, [ట్యాగ్_2, has_ట్యాగ్_ఉపసర్గ](స్థిరరాశి ట్యాగ్_Name_అంశం అంశం_2) {
        తిరిగి అంశం_2.ట్యాగ్_Name.drop_front(has_ట్యాగ్_ఉపసర్గ ? 0 : 4) == ట్యాగ్_2;
      });
  ఒకవేళ (ట్యాగ్_Name_It == ట్యాగ్_Name_Map_2.చివర())
    తిరిగి std::nullopt;
  తిరిగి ట్యాగ్_Name_It->attr;
}
