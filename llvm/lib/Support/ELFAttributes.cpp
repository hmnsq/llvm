//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/ELFAttributes.h"
#ضمن "llvm/ADT/StringRef.h"

ورتوں نطاق llvm;

StringRef ELFAttrs::attrTypeAsString(بنانشانی attr, ٹیگ_Name_Map ٹیگ_Name_Map_2,
                                     منطقي has_ٹیگ_اگیتر) {
  خودکار ٹیگ_Name_It = لبھو_if(
      ٹیگ_Name_Map_2, [attr](ثابت ٹیگ_Name_شے شے_2) { أرجع شے_2.attr == attr; });
  إذا (ٹیگ_Name_It == ٹیگ_Name_Map_2.سرا())
    أرجع "";
  StringRef ٹیگ_Name = ٹیگ_Name_It->ٹیگ_Name;
  أرجع has_ٹیگ_اگیتر ? ٹیگ_Name : ٹیگ_Name.drop_front(4);
}

std::optional<بنانشانی> ELFAttrs::attrTypeFromString(StringRef ٹیگ_2,
                                                     ٹیگ_Name_Map ٹیگ_Name_Map_2) {
  منطقي has_ٹیگ_اگیتر = ٹیگ_2.starts_with("Tag_");
  خودکار ٹیگ_Name_It =
      لبھو_if(ٹیگ_Name_Map_2, [ٹیگ_2, has_ٹیگ_اگیتر](ثابت ٹیگ_Name_شے شے_2) {
        أرجع شے_2.ٹیگ_Name.drop_front(has_ٹیگ_اگیتر ? 0 : 4) == ٹیگ_2;
      });
  إذا (ٹیگ_Name_It == ٹیگ_Name_Map_2.سرا())
    أرجع std::nullopt;
  أرجع ٹیگ_Name_It->attr;
}
