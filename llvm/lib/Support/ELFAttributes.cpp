//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/ELFAttributes.h"
#ضمن "llvm/ADT/StringRef.h"

استعمال نطاق llvm;

StringRef ELFAttrs::attrTypeAsString(بنانشان attr, ٽيگ_Name_Map ٽيگ_Name_Map_2,
                                     منطقي has_ٽيگ_اڳياڙي) {
  خودڪار ٽيگ_Name_It = ڳوليو_if(
      ٽيگ_Name_Map_2, [attr](ثابت ٽيگ_Name_شي شي_2) { أرجع شي_2.attr == attr; });
  إذا (ٽيگ_Name_It == ٽيگ_Name_Map_2.پڇاڙي())
    أرجع "";
  StringRef ٽيگ_Name = ٽيگ_Name_It->ٽيگ_Name;
  أرجع has_ٽيگ_اڳياڙي ? ٽيگ_Name : ٽيگ_Name.drop_front(4);
}

std::optional<بنانشان> ELFAttrs::attrTypeFromString(StringRef ٽيگ_2,
                                                     ٽيگ_Name_Map ٽيگ_Name_Map_2) {
  منطقي has_ٽيگ_اڳياڙي = ٽيگ_2.starts_with("Tag_");
  خودڪار ٽيگ_Name_It =
      ڳوليو_if(ٽيگ_Name_Map_2, [ٽيگ_2, has_ٽيگ_اڳياڙي](ثابت ٽيگ_Name_شي شي_2) {
        أرجع شي_2.ٽيگ_Name.drop_front(has_ٽيگ_اڳياڙي ? 0 : 4) == ٽيگ_2;
      });
  إذا (ٽيگ_Name_It == ٽيگ_Name_Map_2.پڇاڙي())
    أرجع std::nullopt;
  أرجع ٽيگ_Name_It->attr;
}
