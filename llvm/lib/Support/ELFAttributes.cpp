//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/ELFAttributes.h"
#ضمن "llvm/ADT/StringRef.h"

استفاده نطاق llvm;

StringRef ELFAttrs::attrTypeAsString(بیعلامت attr, برچسب_Name_Map برچسب_Name_Map_2,
                                     منطقي has_برچسب_پیشوند) {
  خودکار برچسب_Name_It = پیدا_کن_if(
      برچسب_Name_Map_2, [attr](ثابت برچسب_Name_قلم قلم_2) { أرجع قلم_2.attr == attr; });
  إذا (برچسب_Name_It == برچسب_Name_Map_2.انتها())
    أرجع "";
  StringRef برچسب_Name = برچسب_Name_It->برچسب_Name;
  أرجع has_برچسب_پیشوند ? برچسب_Name : برچسب_Name.drop_front(4);
}

std::optional<بیعلامت> ELFAttrs::attrTypeFromString(StringRef برچسب_2,
                                                     برچسب_Name_Map برچسب_Name_Map_2) {
  منطقي has_برچسب_پیشوند = برچسب_2.starts_with("Tag_");
  خودکار برچسب_Name_It =
      پیدا_کن_if(برچسب_Name_Map_2, [برچسب_2, has_برچسب_پیشوند](ثابت برچسب_Name_قلم قلم_2) {
        أرجع قلم_2.برچسب_Name.drop_front(has_برچسب_پیشوند ? 0 : 4) == برچسب_2;
      });
  إذا (برچسب_Name_It == برچسب_Name_Map_2.انتها())
    أرجع std::nullopt;
  أرجع برچسب_Name_It->attr;
}
