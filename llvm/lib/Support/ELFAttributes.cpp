//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/ELFAttributes.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"

using ନାମାକାଶ llvm;

StringRef ELFAttrs::attrTypeAsString(unsigned attr, TagNameMap tagNameMap,
                                     ତାର୍କିକ hasTagPrefix) {
  auto tagNameIt = find_if(
      tagNameMap, [attr](ଅଚଳ TagNameItem item) { ଫେରନ୍ତୁ item.attr == attr; });
  ଯଦି (tagNameIt == tagNameMap.end())
    ଫେରନ୍ତୁ "";
  StringRef tagName = tagNameIt->tagName;
  ଫେରନ୍ତୁ hasTagPrefix ? tagName : tagName.drop_front(4);
}

std::optional<unsigned> ELFAttrs::attrTypeFromString(StringRef tag,
                                                     TagNameMap tagNameMap) {
  ତାର୍କିକ hasTagPrefix = tag.starts_with("Tag_");
  auto tagNameIt =
      find_if(tagNameMap, [tag, hasTagPrefix](ଅଚଳ TagNameItem item) {
        ଫେରନ୍ତୁ item.tagName.drop_front(hasTagPrefix ? 0 : 4) == tag;
      });
  ଯଦି (tagNameIt == tagNameMap.end())
    ଫେରନ୍ତୁ std::nullopt;
  ଫେରନ୍ତୁ tagNameIt->attr;
}
