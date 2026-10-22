//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ELFAttributes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

StringRef ELFAttrs::attrTypeAsString(ਬਿਨਾਨਿਸ਼ਾਨੀ attr, ਟੈਗ_Name_Map ਟੈਗ_Name_Map_2,
                                     ਤਾਰਕਿਕ has_ਟੈਗ_ਅਗੇਤਰ) {
  ਸਵੈਚਾਲਿਤ ਟੈਗ_Name_It = ਲੱਭੋ_if(
      ਟੈਗ_Name_Map_2, [attr](ਅਚਲ ਟੈਗ_Name_ਵਸਤੂ ਵਸਤੂ_2) { ਵਾਪਸ ਵਸਤੂ_2.attr == attr; });
  ਜੇ (ਟੈਗ_Name_It == ਟੈਗ_Name_Map_2.ਸਿਰਾ())
    ਵਾਪਸ "";
  StringRef ਟੈਗ_Name = ਟੈਗ_Name_It->ਟੈਗ_Name;
  ਵਾਪਸ has_ਟੈਗ_ਅਗੇਤਰ ? ਟੈਗ_Name : ਟੈਗ_Name.drop_front(4);
}

std::optional<ਬਿਨਾਨਿਸ਼ਾਨੀ> ELFAttrs::attrTypeFromString(StringRef ਟੈਗ_2,
                                                     ਟੈਗ_Name_Map ਟੈਗ_Name_Map_2) {
  ਤਾਰਕਿਕ has_ਟੈਗ_ਅਗੇਤਰ = ਟੈਗ_2.starts_with("Tag_");
  ਸਵੈਚਾਲਿਤ ਟੈਗ_Name_It =
      ਲੱਭੋ_if(ਟੈਗ_Name_Map_2, [ਟੈਗ_2, has_ਟੈਗ_ਅਗੇਤਰ](ਅਚਲ ਟੈਗ_Name_ਵਸਤੂ ਵਸਤੂ_2) {
        ਵਾਪਸ ਵਸਤੂ_2.ਟੈਗ_Name.drop_front(has_ਟੈਗ_ਅਗੇਤਰ ? 0 : 4) == ਟੈਗ_2;
      });
  ਜੇ (ਟੈਗ_Name_It == ਟੈਗ_Name_Map_2.ਸਿਰਾ())
    ਵਾਪਸ std::nullopt;
  ਵਾਪਸ ਟੈਗ_Name_It->attr;
}
