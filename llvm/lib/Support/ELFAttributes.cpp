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

StringRef ELFAttrs::attrTypeAsString(अचिन्हित attr, TagNameMap tagNameMap,
                                     तार्किक hasTagPrefix) {
  स्वतः tagNameIt = find_if(
      tagNameMap, [attr](स्थिरांक TagNameItem item) { परत item.attr == attr; });
  जर (tagNameIt == tagNameMap.end())
    परत "";
  StringRef tagName = tagNameIt->tagName;
  परत hasTagPrefix ? tagName : tagName.drop_front(4);
}

std::optional<अचिन्हित> ELFAttrs::attrTypeFromString(StringRef tag,
                                                     TagNameMap tagNameMap) {
  तार्किक hasTagPrefix = tag.starts_with("Tag_");
  स्वतः tagNameIt =
      find_if(tagNameMap, [tag, hasTagPrefix](स्थिरांक TagNameItem item) {
        परत item.tagName.drop_front(hasTagPrefix ? 0 : 4) == tag;
      });
  जर (tagNameIt == tagNameMap.end())
    परत std::nullopt;
  परत tagNameIt->attr;
}
