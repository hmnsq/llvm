//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/ELFAttributes.h"
#include "llvm/ADT/StringRef.h"

using namespace llvm;

StringRef ELFAttrs::attrTypeAsString(unsigned attr, TagNameMap tagNameMap,
                                     bool hasTagPrefix) {
  auto tagNameIt = find_if(
      tagNameMap, [attr](const TagNameItem item) { ပွၵ်ႈ item.attr == attr; });
  ပေႃး (tagNameIt == tagNameMap.end())
    ပွၵ်ႈ "";
  StringRef tagName = tagNameIt->tagName;
  ပွၵ်ႈ hasTagPrefix ? tagName : tagName.drop_front(4);
}

std::optional<unsigned> ELFAttrs::attrTypeFromString(StringRef tag,
                                                     TagNameMap tagNameMap) {
  bool hasTagPrefix = tag.starts_with("Tag_");
  auto tagNameIt =
      find_if(tagNameMap, [tag, hasTagPrefix](const TagNameItem item) {
        ပွၵ်ႈ item.tagName.drop_front(hasTagPrefix ? 0 : 4) == tag;
      });
  ပေႃး (tagNameIt == tagNameMap.end())
    ပွၵ်ႈ std::nullopt;
  ပွၵ်ႈ tagNameIt->attr;
}
