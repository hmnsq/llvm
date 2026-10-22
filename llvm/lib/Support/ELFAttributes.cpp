//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/ELFAttributes.h"
#ضمن "llvm/ADT/StringRef.h"

کارول نطاق llvm;

StringRef ELFAttrs::attrTypeAsString(بینښه attr, نښان_Name_Map نښان_Name_Map_2,
                                     منطقي has_نښان_مختاړی) {
  خپلکاری نښان_Name_It = پیدا_کړه_if(
      نښان_Name_Map_2, [attr](ثابت نښان_Name_توکی توکی_2) { أرجع توکی_2.attr == attr; });
  إذا (نښان_Name_It == نښان_Name_Map_2.پای_ټکی())
    أرجع "";
  StringRef نښان_Name = نښان_Name_It->نښان_Name;
  أرجع has_نښان_مختاړی ? نښان_Name : نښان_Name.drop_front(4);
}

std::optional<بینښه> ELFAttrs::attrTypeFromString(StringRef نښان_2,
                                                     نښان_Name_Map نښان_Name_Map_2) {
  منطقي has_نښان_مختاړی = نښان_2.starts_with("Tag_");
  خپلکاری نښان_Name_It =
      پیدا_کړه_if(نښان_Name_Map_2, [نښان_2, has_نښان_مختاړی](ثابت نښان_Name_توکی توکی_2) {
        أرجع توکی_2.نښان_Name.drop_front(has_نښان_مختاړی ? 0 : 4) == نښان_2;
      });
  إذا (نښان_Name_It == نښان_Name_Map_2.پای_ټکی())
    أرجع std::nullopt;
  أرجع نښان_Name_It->attr;
}
