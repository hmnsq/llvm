//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/ELFAttributes.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"

using ನಾಮಾಕಾಶ llvm;

StringRef ELFAttrs::attrTypeAsString(unsigned attr, TagNameMap tagNameMap,
                                     ತಾರ್ಕಿಕ hasTagPrefix) {
  auto tagNameIt = find_if(
      tagNameMap, [attr](ಧ್ರುವ TagNameItem item) { ಪ್ರತ್ಯಾಗಮ item.attr == attr; });
  ಯದಿ (tagNameIt == tagNameMap.end())
    ಪ್ರತ್ಯಾಗಮ "";
  StringRef tagName = tagNameIt->tagName;
  ಪ್ರತ್ಯಾಗಮ hasTagPrefix ? tagName : tagName.drop_front(4);
}

std::optional<unsigned> ELFAttrs::attrTypeFromString(StringRef tag,
                                                     TagNameMap tagNameMap) {
  ತಾರ್ಕಿಕ hasTagPrefix = tag.starts_with("Tag_");
  auto tagNameIt =
      find_if(tagNameMap, [tag, hasTagPrefix](ಧ್ರುವ TagNameItem item) {
        ಪ್ರತ್ಯಾಗಮ item.tagName.drop_front(hasTagPrefix ? 0 : 4) == tag;
      });
  ಯದಿ (tagNameIt == tagNameMap.end())
    ಪ್ರತ್ಯಾಗಮ std::nullopt;
  ಪ್ರತ್ಯಾಗಮ tagNameIt->attr;
}
