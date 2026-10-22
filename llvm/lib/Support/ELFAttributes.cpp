//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/ELFAttributes.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"

उपयुज्य नामाकाश llvm;

StringRef ELFAttrs::attrTypeAsString(अचिह्नित attr, चिह्नकम्_Name_Map चिह्नकम्_Name_Map_2,
                                     तार्किक has_चिह्नकम्_उपसर्गः) {
  स्वचालित चिह्नकम्_Name_It = find_if(
      चिह्नकम्_Name_Map_2, [attr](ध्रुव चिह्नकम्_Name_Item item) { प्रत्यागम item.attr == attr; });
  यदि (चिह्नकम्_Name_It == चिह्नकम्_Name_Map_2.अन्तः())
    प्रत्यागम "";
  StringRef चिह्नकम्_Name = चिह्नकम्_Name_It->चिह्नकम्_Name;
  प्रत्यागम has_चिह्नकम्_उपसर्गः ? चिह्नकम्_Name : चिह्नकम्_Name.drop_front(4);
}

std::optional<अचिह्नित> ELFAttrs::attrTypeFromString(StringRef चिह्नकम्_2,
                                                     चिह्नकम्_Name_Map चिह्नकम्_Name_Map_2) {
  तार्किक has_चिह्नकम्_उपसर्गः = चिह्नकम्_2.starts_with("Tag_");
  स्वचालित चिह्नकम्_Name_It =
      find_if(चिह्नकम्_Name_Map_2, [चिह्नकम्_2, has_चिह्नकम्_उपसर्गः](ध्रुव चिह्नकम्_Name_Item item) {
        प्रत्यागम item.चिह्नकम्_Name.drop_front(has_चिह्नकम्_उपसर्गः ? 0 : 4) == चिह्नकम्_2;
      });
  यदि (चिह्नकम्_Name_It == चिह्नकम्_Name_Map_2.अन्तः())
    प्रत्यागम std::nullopt;
  प्रत्यागम चिह्नकम्_Name_It->attr;
}
