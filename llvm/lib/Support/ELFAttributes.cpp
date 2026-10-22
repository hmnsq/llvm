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
      tagNameMap, [attr](const TagNameItem item) { ᰀᰧᰌ item.attr == attr; });
  ᰀᰧ (tagNameIt == tagNameMap.end())
    ᰀᰧᰌ "";
  StringRef tagName = tagNameIt->tagName;
  ᰀᰧᰌ hasTagPrefix ? tagName : tagName.drop_front(4);
}

std::optional<unsigned> ELFAttrs::attrTypeFromString(StringRef tag,
                                                     TagNameMap tagNameMap) {
  bool hasTagPrefix = tag.starts_with("Tag_");
  auto tagNameIt =
      find_if(tagNameMap, [tag, hasTagPrefix](const TagNameItem item) {
        ᰀᰧᰌ item.tagName.drop_front(hasTagPrefix ? 0 : 4) == tag;
      });
  ᰀᰧ (tagNameIt == tagNameMap.end())
    ᰀᰧᰌ std::nullopt;
  ᰀᰧᰌ tagNameIt->attr;
}
