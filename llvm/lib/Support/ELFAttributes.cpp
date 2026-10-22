//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/ELFAttributes.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"

using नामाकाश llvm;

StringRef ELFAttrs::attrTypeAsString(unsigned attr, TagNameMap tagNameMap,
                                     तार्किक hasTagPrefix) {
  auto tagNameIt = find_if(
      tagNameMap, [attr](ध्रुव TagNameItem item) { पटिनिवत्त item.attr == attr; });
  सचे (tagNameIt == tagNameMap.end())
    पटिनिवत्त "";
  StringRef tagName = tagNameIt->tagName;
  पटिनिवत्त hasTagPrefix ? tagName : tagName.drop_front(4);
}

std::optional<unsigned> ELFAttrs::attrTypeFromString(StringRef tag,
                                                     TagNameMap tagNameMap) {
  तार्किक hasTagPrefix = tag.starts_with("Tag_");
  auto tagNameIt =
      find_if(tagNameMap, [tag, hasTagPrefix](ध्रुव TagNameItem item) {
        पटिनिवत्त item.tagName.drop_front(hasTagPrefix ? 0 : 4) == tag;
      });
  सचे (tagNameIt == tagNameMap.end())
    पटिनिवत्त std::nullopt;
  पटिनिवत्त tagNameIt->attr;
}
