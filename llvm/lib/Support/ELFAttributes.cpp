//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/ELFAttributes.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"

using নামাকাশ llvm;

StringRef ELFAttrs::attrTypeAsString(unsigned attr, TagNameMap tagNameMap,
                                     যৌক্তিক hasTagPrefix) {
  auto tagNameIt = find_if(
      tagNameMap, [attr](ধ্রুবক TagNameItem item) { ফেরত item.attr == attr; });
  যদি (tagNameIt == tagNameMap.end())
    ফেরত "";
  StringRef tagName = tagNameIt->tagName;
  ফেরত hasTagPrefix ? tagName : tagName.drop_front(4);
}

std::optional<unsigned> ELFAttrs::attrTypeFromString(StringRef tag,
                                                     TagNameMap tagNameMap) {
  যৌক্তিক hasTagPrefix = tag.starts_with("Tag_");
  auto tagNameIt =
      find_if(tagNameMap, [tag, hasTagPrefix](ধ্রুবক TagNameItem item) {
        ফেরত item.tagName.drop_front(hasTagPrefix ? 0 : 4) == tag;
      });
  যদি (tagNameIt == tagNameMap.end())
    ফেরত std::nullopt;
  ফেরত tagNameIt->attr;
}
