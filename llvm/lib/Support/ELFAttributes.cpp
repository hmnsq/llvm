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
                                     তার্কিক hasTagPrefix) {
  auto tagNameIt = find_if(
      tagNameMap, [attr](ধ্রুব TagNameItem item) { প্রত্যাগম item.attr == attr; });
  যদি (tagNameIt == tagNameMap.end())
    প্রত্যাগম "";
  StringRef tagName = tagNameIt->tagName;
  প্রত্যাগম hasTagPrefix ? tagName : tagName.drop_front(4);
}

std::optional<unsigned> ELFAttrs::attrTypeFromString(StringRef tag,
                                                     TagNameMap tagNameMap) {
  তার্কিক hasTagPrefix = tag.starts_with("Tag_");
  auto tagNameIt =
      find_if(tagNameMap, [tag, hasTagPrefix](ধ্রুব TagNameItem item) {
        প্রত্যাগম item.tagName.drop_front(hasTagPrefix ? 0 : 4) == tag;
      });
  যদি (tagNameIt == tagNameMap.end())
    প্রত্যাগম std::nullopt;
  প্রত্যাগম tagNameIt->attr;
}
