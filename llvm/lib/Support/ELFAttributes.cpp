//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/ELFAttributes.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"

using നാമാകാശ llvm;

StringRef ELFAttrs::attrTypeAsString(unsigned attr, TagNameMap tagNameMap,
                                     തർക്കം hasTagPrefix) {
  auto tagNameIt = find_if(
      tagNameMap, [attr](സ്ഥിരം TagNameItem item) { തിരികെ item.attr == attr; });
  എങ്കിൽ (tagNameIt == tagNameMap.end())
    തിരികെ "";
  StringRef tagName = tagNameIt->tagName;
  തിരികെ hasTagPrefix ? tagName : tagName.drop_front(4);
}

std::optional<unsigned> ELFAttrs::attrTypeFromString(StringRef tag,
                                                     TagNameMap tagNameMap) {
  തർക്കം hasTagPrefix = tag.starts_with("Tag_");
  auto tagNameIt =
      find_if(tagNameMap, [tag, hasTagPrefix](സ്ഥിരം TagNameItem item) {
        തിരികെ item.tagName.drop_front(hasTagPrefix ? 0 : 4) == tag;
      });
  എങ്കിൽ (tagNameIt == tagNameMap.end())
    തിരികെ std::nullopt;
  തിരികെ tagNameIt->attr;
}
