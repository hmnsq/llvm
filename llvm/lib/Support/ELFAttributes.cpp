//===-- ELFAttributes.cpp - ELF Attributes --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/ELFAttributes.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"

using நாமாகாஶ llvm;

StringRef ELFAttrs::attrTypeAsString(unsigned attr, TagNameMap tagNameMap,
                                     தருக்க hasTagPrefix) {
  auto tagNameIt = find_if(
      tagNameMap, [attr](மாறிலி TagNameItem item) { திரும்பு item.attr == attr; });
  எனில் (tagNameIt == tagNameMap.end())
    திரும்பு "";
  StringRef tagName = tagNameIt->tagName;
  திரும்பு hasTagPrefix ? tagName : tagName.drop_front(4);
}

std::optional<unsigned> ELFAttrs::attrTypeFromString(StringRef tag,
                                                     TagNameMap tagNameMap) {
  தருக்க hasTagPrefix = tag.starts_with("Tag_");
  auto tagNameIt =
      find_if(tagNameMap, [tag, hasTagPrefix](மாறிலி TagNameItem item) {
        திரும்பு item.tagName.drop_front(hasTagPrefix ? 0 : 4) == tag;
      });
  எனில் (tagNameIt == tagNameMap.end())
    திரும்பு std::nullopt;
  திரும்பு tagNameIt->attr;
}
