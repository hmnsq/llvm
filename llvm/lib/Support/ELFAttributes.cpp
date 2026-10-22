//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/ELFAttributes.h"
#ضمن "llvm/ADT/StringRef.h"

using نطاق llvm;

StringRef ELFAttrs::attrTypeAsString(unsigned attr, TagNameMap tagNameMap,
                                     منطقي hasTagPrefix) {
  auto tagNameIt = find_if(
      tagNameMap, [attr](ثابت TagNameItem item) { החזר item.attr == attr; });
  אם (tagNameIt == tagNameMap.end())
    החזר "";
  StringRef tagName = tagNameIt->tagName;
  החזר hasTagPrefix ? tagName : tagName.drop_front(4);
}

std::optional<unsigned> ELFAttrs::attrTypeFromString(StringRef tag,
                                                     TagNameMap tagNameMap) {
  منطقي hasTagPrefix = tag.starts_with("Tag_");
  auto tagNameIt =
      find_if(tagNameMap, [tag, hasTagPrefix](ثابت TagNameItem item) {
        החזר item.tagName.drop_front(hasTagPrefix ? 0 : 4) == tag;
      });
  אם (tagNameIt == tagNameMap.end())
    החזר std::nullopt;
  החזר tagNameIt->attr;
}
