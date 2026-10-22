//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/ELFAttributes.h"
#شامل "llvm/ADT/StringRef.h"

استعمال نامخلا llvm;

StringRef ELFAttrs::attrTypeAsString(غیرعلامتی attr, لیبل_Name_Map لیبل_Name_Map_2,
                                     منطقی has_لیبل_سابقہ) {
  خودکار لیبل_Name_It = ڈھونڈو_if(
      لیبل_Name_Map_2, [attr](ثابت لیبل_Name_شے شے_2) { واپس شے_2.attr == attr; });
  اگر (لیبل_Name_It == لیبل_Name_Map_2.انتہا())
    واپس "";
  StringRef لیبل_Name = لیبل_Name_It->لیبل_Name;
  واپس has_لیبل_سابقہ ? لیبل_Name : لیبل_Name.drop_front(4);
}

std::optional<غیرعلامتی> ELFAttrs::attrTypeFromString(StringRef لیبل_2,
                                                     لیبل_Name_Map لیبل_Name_Map_2) {
  منطقی has_لیبل_سابقہ = لیبل_2.starts_with("Tag_");
  خودکار لیبل_Name_It =
      ڈھونڈو_if(لیبل_Name_Map_2, [لیبل_2, has_لیبل_سابقہ](ثابت لیبل_Name_شے شے_2) {
        واپس شے_2.لیبل_Name.drop_front(has_لیبل_سابقہ ? 0 : 4) == لیبل_2;
      });
  اگر (لیبل_Name_It == لیبل_Name_Map_2.انتہا())
    واپس std::nullopt;
  واپس لیبل_Name_It->attr;
}
