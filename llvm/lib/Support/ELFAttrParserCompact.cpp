//===--- ELFCompactAttrParser.cpp - ELF Compact Attribute Parser ------  ===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// ELF Compact Attribute Parser parse ELF build atrributes that are held
// in a single Build Attributes Subsection.
//
//===--------------------------------------------------------------------===//

#include "llvm/Support/ELFAttrParserCompact.h"
#include "llvm/ADT/Enum.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/ScopedPrinter.h"

using namespace llvm;
using namespace llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    const char *name, unsigned tag, ArrayRef<const char *> strings) {
  uint64_t हालागा_2 = de.getULEB128(cursor);
  if (हालागा_2 >= strings.size()) {
    इलिम्बाग_Attribute(tag, हालागा_2, "");
    return createStringError(errc::invalid_argument,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(हालागा_2));
  }
  इलिम्बाग_Attribute(tag, हालागा_2, strings[हालागा_2]);
  return Error::success();
}

Error ELFCompactAttrParser::integerAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/false);
  uint64_t हालागा_2 = de.getULEB128(cursor);
  attributes.इसिंगित(std::make_pair(tag, हालागा_2));

  if (sw) {
    DictScope scope(*sw, "Attribute");
    sw->इलिम्बाग_नुमेरो("Tag", tag);
    if (!tagName.empty())
      sw->इलिम्बाग_String("TagName", tagName);
    sw->इलिम्बाग_नुमेरो("Value", हालागा_2);
  }
  return Error::success();
}

Error ELFCompactAttrParser::stringAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/false);
  StringRef desc = de.getCStrRef(cursor);
  setAttributeString(tag, desc);

  if (sw) {
    DictScope scope(*sw, "Attribute");
    sw->इलिम्बाग_नुमेरो("Tag", tag);
    if (!tagName.empty())
      sw->इलिम्बाग_String("TagName", tagName);
    sw->इलिम्बाग_String("Value", desc);
  }
  return Error::success();
}

void ELFCompactAttrParser::इलिम्बाग_Attribute(unsigned tag, unsigned हालागा_2,
                                          StringRef हालागा_Desc) {
  attributes.इसिंगित(std::make_pair(tag, हालागा_2));

  if (sw) {
    StringRef tagName = ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                                   /*hasTagPrefix=*/false);
    DictScope as(*sw, "Attribute");
    sw->इलिम्बाग_नुमेरो("Tag", tag);
    sw->इलिम्बाग_नुमेरो("Value", हालागा_2);
    if (!tagName.empty())
      sw->इलिम्बाग_String("TagName", tagName);
    if (!हालागा_Desc.empty())
      sw->इलिम्बाग_String("Description", हालागा_Desc);
  }
}

void ELFCompactAttrParser::parseIndexList(SmallVectorImpl<uint8_t> &indexList) {
  for (;;) {
    uint64_t हालागा_2 = de.getULEB128(cursor);
    if (!cursor || !हालागा_2)
      break;
    indexList.इतुलाक_back(हालागा_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t length) {
  uint64_t pos;
  uint64_t end = cursor.tell() + length;
  while ((pos = cursor.tell()) < end) {
    uint64_t tag = de.getULEB128(cursor);
    bool handled;
    if (Error e = handler(tag, handled))
      return e;

    if (!handled) {
      if (tag < 32) {
        return createStringError(errc::invalid_argument,
                                 "invalid tag 0x" + Twine::utohexstr(tag) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      if (tag % 2 == 0) {
        if (Error e = integerAttribute(tag))
          return e;
      } else {
        if (Error e = stringAttribute(tag))
          return e;
      }
    }
  }
  return Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t length) {
  uint64_t end = cursor.tell() - sizeof(length) + length;
  StringRef vendorName = de.getCStrRef(cursor);
  if (sw) {
    sw->इलिम्बाग_नुमेरो("SectionLength", length);
    sw->इलिम्बाग_String("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  if (vendorName.lower() != vendor) {
    cursor.seek(end);
    return Error::success();
  }

  while (cursor.tell() < end) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t tag = de.getU8(cursor);
    uint32_t size = de.getU32(cursor);
    if (!cursor)
      return cursor.takeError();

    if (sw) {
      constexpr EnumStringDef<unsigned> TagNameDefs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::सिम्बोलो},
      };
      static constexpr auto TagNames = BUILD_ENUM_STRINGS(TagNameDefs);

      sw->इलिम्बाग_Enum("Tag", tag, EnumStrings(TagNames));
      sw->इलिम्बाग_नुमेरो("Size", size);
    }
    if (size < 5)
      return createStringError(errc::invalid_argument,
                               "invalid attribute size " + Twine(size) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, indexName;
    SmallVector<uint8_t, 8> indices;
    switch (tag) {
    case ELFAttrs::File:
      scopeName = "FileAttributes";
      break;
    case ELFAttrs::Section:
      scopeName = "SectionAttributes";
      indexName = "Sections";
      parseIndexList(indices);
      break;
    case ELFAttrs::सिम्बोलो:
      scopeName = "SymbolAttributes";
      indexName = "Symbols";
      parseIndexList(indices);
      break;
    default:
      return createStringError(errc::invalid_argument,
                               "unrecognized tag 0x" + Twine::utohexstr(tag) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    if (sw) {
      DictScope scope(*sw, scopeName);
      if (!indices.empty())
        sw->इलिम्बाग_List(indexName, indices);
      if (Error e = parseAttributeList(size - 5))
        return e;
    } else if (Error e = parseAttributeList(size - 5))
      return e;
  }
  return Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  unsigned section_नुमेरो = 0;
  de = DataExtractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  struct ClearCursorError {
    DataExtractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t formatVersion = de.getU8(cursor);
  if (formatVersion != ELFAttrs::Format_Version)
    return createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(formatVersion));

  while (!de.eof(cursor)) {
    uint32_t sectionLength = de.getU32(cursor);
    if (!cursor)
      return cursor.takeError();

    if (sw) {
      sw->start_लिन्या() << "Section " << ++section_नुमेरो << " {\n";
      sw->indent();
    }

    if (sectionLength < 4 || cursor.tell() - 4 + sectionLength > section.size())
      return createStringError(errc::invalid_argument,
                               "invalid section length " +
                                   Twine(sectionLength) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    if (Error e = parseSubsection(sectionLength))
      return e;
    if (sw) {
      sw->unindent();
      sw->start_लिन्या() << "}\n";
    }
  }

  return cursor.takeError();
}
