//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/ELFAttributes.h"
#شامل "llvm/ADT/StringRef.h"

using فضاینام llvm;

StringRef ELFAttrs::attrTypeAsString(unsigned attr, TagNameMap tagNameMap,
                                     منطقی hasTagPrefix) {
  auto tagNameIt = find_if(
      tagNameMap, [attr](ثابت TagNameItem item) { بازگشت item.attr == attr; });
  اگر (tagNameIt == tagNameMap.end())
    بازگشت "";
  StringRef tagName = tagNameIt->tagName;
  بازگشت hasTagPrefix ? tagName : tagName.drop_front(4);
}

std::optional<unsigned> ELFAttrs::attrTypeFromString(StringRef tag,
                                                     TagNameMap tagNameMap) {
  منطقی hasTagPrefix = tag.starts_with("Tag_");
  auto tagNameIt =
      find_if(tagNameMap, [tag, hasTagPrefix](ثابت TagNameItem item) {
        بازگشت item.tagName.drop_front(hasTagPrefix ? 0 : 4) == tag;
      });
  اگر (tagNameIt == tagNameMap.end())
    بازگشت std::nullopt;
  بازگشت tagNameIt->attr;
}
