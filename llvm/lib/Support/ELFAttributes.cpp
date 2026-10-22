//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/ELFAttributes.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"

using નામાકાશ llvm;

StringRef ELFAttrs::attrTypeAsString(unsigned attr, TagNameMap tagNameMap,
                                     તાર્કિક hasTagPrefix) {
  auto tagNameIt = find_if(
      tagNameMap, [attr](અચલ TagNameItem item) { પાછા item.attr == attr; });
  જો (tagNameIt == tagNameMap.end())
    પાછા "";
  StringRef tagName = tagNameIt->tagName;
  પાછા hasTagPrefix ? tagName : tagName.drop_front(4);
}

std::optional<unsigned> ELFAttrs::attrTypeFromString(StringRef tag,
                                                     TagNameMap tagNameMap) {
  તાર્કિક hasTagPrefix = tag.starts_with("Tag_");
  auto tagNameIt =
      find_if(tagNameMap, [tag, hasTagPrefix](અચલ TagNameItem item) {
        પાછા item.tagName.drop_front(hasTagPrefix ? 0 : 4) == tag;
      });
  જો (tagNameIt == tagNameMap.end())
    પાછા std::nullopt;
  પાછા tagNameIt->attr;
}
