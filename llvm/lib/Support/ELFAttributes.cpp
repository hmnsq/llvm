//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/ELFAttributes.h"
#समावेश "llvm/ADT/StringRef.h"

using namespace llvm;

StringRef ELFAttrs::attrTypeAsString(अचिन्हित attr, TagNameMap tagNameMap,
                                     bool hasTagPrefix) {
  स्वतः tagNameIt = find_if(
      tagNameMap, [attr](स्थिर TagNameItem item) { वापस item.attr == attr; });
  अगर (tagNameIt == tagNameMap.end())
    वापस "";
  StringRef tagName = tagNameIt->tagName;
  वापस hasTagPrefix ? tagName : tagName.drop_front(4);
}

std::optional<अचिन्हित> ELFAttrs::attrTypeFromString(StringRef tag,
                                                     TagNameMap tagNameMap) {
  bool hasTagPrefix = tag.starts_with("Tag_");
  स्वतः tagNameIt =
      find_if(tagNameMap, [tag, hasTagPrefix](स्थिर TagNameItem item) {
        वापस item.tagName.drop_front(hasTagPrefix ? 0 : 4) == tag;
      });
  अगर (tagNameIt == tagNameMap.end())
    वापस std::nullopt;
  वापस tagNameIt->attr;
}
