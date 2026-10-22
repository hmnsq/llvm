//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/ELFAttributes.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

StringRef ELFAttrs::attrTypeAsString(ಚಿಹ್ನೆರಹಿತ attr, ಟ್ಯಾಗ್_Name_Map ಟ್ಯಾಗ್_Name_Map_2,
                                     ತಾರ್ಕಿಕ has_ಟ್ಯಾಗ್_ಪೂರ್ವಪ್ರತ್ಯಯ) {
  ಸ್ವಯಂಚಾಲಿತ ಟ್ಯಾಗ್_Name_It = ಕಂಡುಹಿಡಿ_if(
      ಟ್ಯಾಗ್_Name_Map_2, [attr](ಸ್ಥಿರಾಂಕ ಟ್ಯಾಗ್_Name_ಅಂಶ ಅಂಶ_2) { ಹಿಂತಿರುಗಿ ಅಂಶ_2.attr == attr; });
  ಆದರೆ (ಟ್ಯಾಗ್_Name_It == ಟ್ಯಾಗ್_Name_Map_2.ಕೊನೆ())
    ಹಿಂತಿರುಗಿ "";
  StringRef ಟ್ಯಾಗ್_Name = ಟ್ಯಾಗ್_Name_It->ಟ್ಯಾಗ್_Name;
  ಹಿಂತಿರುಗಿ has_ಟ್ಯಾಗ್_ಪೂರ್ವಪ್ರತ್ಯಯ ? ಟ್ಯಾಗ್_Name : ಟ್ಯಾಗ್_Name.drop_front(4);
}

std::optional<ಚಿಹ್ನೆರಹಿತ> ELFAttrs::attrTypeFromString(StringRef ಟ್ಯಾಗ್_2,
                                                     ಟ್ಯಾಗ್_Name_Map ಟ್ಯಾಗ್_Name_Map_2) {
  ತಾರ್ಕಿಕ has_ಟ್ಯಾಗ್_ಪೂರ್ವಪ್ರತ್ಯಯ = ಟ್ಯಾಗ್_2.starts_with("Tag_");
  ಸ್ವಯಂಚಾಲಿತ ಟ್ಯಾಗ್_Name_It =
      ಕಂಡುಹಿಡಿ_if(ಟ್ಯಾಗ್_Name_Map_2, [ಟ್ಯಾಗ್_2, has_ಟ್ಯಾಗ್_ಪೂರ್ವಪ್ರತ್ಯಯ](ಸ್ಥಿರಾಂಕ ಟ್ಯಾಗ್_Name_ಅಂಶ ಅಂಶ_2) {
        ಹಿಂತಿರುಗಿ ಅಂಶ_2.ಟ್ಯಾಗ್_Name.drop_front(has_ಟ್ಯಾಗ್_ಪೂರ್ವಪ್ರತ್ಯಯ ? 0 : 4) == ಟ್ಯಾಗ್_2;
      });
  ಆದರೆ (ಟ್ಯಾಗ್_Name_It == ಟ್ಯಾಗ್_Name_Map_2.ಕೊನೆ())
    ಹಿಂತಿರುಗಿ std::nullopt;
  ಹಿಂತಿರುಗಿ ಟ್ಯಾಗ್_Name_It->attr;
}
