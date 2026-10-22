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
      tagNameMap, [attr](const TagNameItem item) { 반환 item.attr == attr; });
  만약 (tagNameIt == tagNameMap.end())
    반환 "";
  StringRef tagName = tagNameIt->tagName;
  반환 hasTagPrefix ? tagName : tagName.drop_front(4);
}

std::optional<unsigned> ELFAttrs::attrTypeFromString(StringRef tag,
                                                     TagNameMap tagNameMap) {
  bool hasTagPrefix = tag.starts_with("Tag_");
  auto tagNameIt =
      find_if(tagNameMap, [tag, hasTagPrefix](const TagNameItem item) {
        반환 item.tagName.drop_front(hasTagPrefix ? 0 : 4) == tag;
      });
  만약 (tagNameIt == tagNameMap.end())
    반환 std::nullopt;
  반환 tagNameIt->attr;
}
