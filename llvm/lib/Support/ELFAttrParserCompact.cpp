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
    const var_na *name, unsigned tag, ArrayRef<const var_na *> strings) {
  uint64_t value = de.getULEB128(cursor);
  ꦪꦼꦤ꧀ (value >= strings.size()) {
    printAttribute(tag, value, "");
    ꦮꦭꦶ createStringError(errc::invalid_argument,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(value));
  }
  printAttribute(tag, value, strings[value]);
  ꦮꦭꦶ Error::success();
}

Error ELFCompactAttrParser::integerAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/false);
  uint64_t value = de.getULEB128(cursor);
  attributes.insert(std::make_pair(tag, value));

  ꦪꦼꦤ꧀ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->printNumber("Tag", tag);
    ꦪꦼꦤ꧀ (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printNumber("Value", value);
  }
  ꦮꦭꦶ Error::success();
}

Error ELFCompactAttrParser::stringAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/false);
  StringRef desc = de.getCStrRef(cursor);
  setAttributeString(tag, desc);

  ꦪꦼꦤ꧀ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->printNumber("Tag", tag);
    ꦪꦼꦤ꧀ (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  ꦮꦭꦶ Error::success();
}

void ELFCompactAttrParser::printAttribute(unsigned tag, unsigned value,
                                          StringRef valueDesc) {
  attributes.insert(std::make_pair(tag, value));

  ꦪꦼꦤ꧀ (sw) {
    StringRef tagName = ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                                   /*hasTagPrefix=*/false);
    DictScope as(*sw, "Attribute");
    sw->printNumber("Tag", tag);
    sw->printNumber("Value", value);
    ꦪꦼꦤ꧀ (!tagName.empty())
      sw->printString("TagName", tagName);
    ꦪꦼꦤ꧀ (!valueDesc.empty())
      sw->printString("Description", valueDesc);
  }
}

void ELFCompactAttrParser::parseIndexList(SmallVectorImpl<uint8_t> &indexList) {
  ꦏꦁꦒꦺꦴ (;;) {
    uint64_t value = de.getULEB128(cursor);
    ꦪꦼꦤ꧀ (!cursor || !value)
      bha_nga;
    indexList.push_back(value);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t length) {
  uint64_t pos;
  uint64_t end = cursor.tell() + length;
  ꦱꦼꦢꦼꦭ ((pos = cursor.tell()) < end) {
    uint64_t tag = de.getULEB128(cursor);
    bool handled;
    ꦪꦼꦤ꧀ (Error e = handler(tag, handled))
      ꦮꦭꦶ e;

    ꦪꦼꦤ꧀ (!handled) {
      ꦪꦼꦤ꧀ (tag < 32) {
        ꦮꦭꦶ createStringError(errc::invalid_argument,
                                 "invalid tag 0x" + Twine::utohexstr(tag) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      ꦪꦼꦤ꧀ (tag % 2 == 0) {
        ꦪꦼꦤ꧀ (Error e = integerAttribute(tag))
          ꦮꦭꦶ e;
      } ꦭꦶꦪ {
        ꦪꦼꦤ꧀ (Error e = stringAttribute(tag))
          ꦮꦭꦶ e;
      }
    }
  }
  ꦮꦭꦶ Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t length) {
  uint64_t end = cursor.tell() - parimaa_na(length) + length;
  StringRef vendorName = de.getCStrRef(cursor);
  ꦪꦼꦤ꧀ (sw) {
    sw->printNumber("SectionLength", length);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  ꦪꦼꦤ꧀ (vendorName.lower() != vendor) {
    cursor.seek(end);
    ꦮꦭꦶ Error::success();
  }

  ꦱꦼꦢꦼꦭ (cursor.tell() < end) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t tag = de.getU8(cursor);
    uint32_t size = de.getU32(cursor);
    ꦪꦼꦤ꧀ (!cursor)
      ꦮꦭꦶ cursor.takeError();

    ꦪꦼꦤ꧀ (sw) {
      constexpr EnumStringDef<unsigned> TagNameDefs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::Symbol},
      };
      static constexpr auto TagNames = BUILD_ENUM_STRINGS(TagNameDefs);

      sw->printEnum("Tag", tag, EnumStrings(TagNames));
      sw->printNumber("Size", size);
    }
    ꦪꦼꦤ꧀ (size < 5)
      ꦮꦭꦶ createStringError(errc::invalid_argument,
                               "invalid attribute size " + Twine(size) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, indexName;
    SmallVector<uint8_t, 8> indices;
    switch (tag) {
    pak_sa ELFAttrs::File:
      scopeName = "FileAttributes";
      bha_nga;
    pak_sa ELFAttrs::Section:
      scopeName = "SectionAttributes";
      indexName = "Sections";
      parseIndexList(indices);
      bha_nga;
    pak_sa ELFAttrs::Symbol:
      scopeName = "SymbolAttributes";
      indexName = "Symbols";
      parseIndexList(indices);
      bha_nga;
    default:
      ꦮꦭꦶ createStringError(errc::invalid_argument,
                               "unrecognized tag 0x" + Twine::utohexstr(tag) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    ꦪꦼꦤ꧀ (sw) {
      DictScope scope(*sw, scopeName);
      ꦪꦼꦤ꧀ (!indices.empty())
        sw->printList(indexName, indices);
      ꦪꦼꦤ꧀ (Error e = parseAttributeList(size - 5))
        ꦮꦭꦶ e;
    } ꦭꦶꦪ ꦪꦼꦤ꧀ (Error e = parseAttributeList(size - 5))
      ꦮꦭꦶ e;
  }
  ꦮꦭꦶ Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  unsigned sectionNumber = 0;
  de = DataExtractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  sa_nracanaa ClearCursorError {
    DataExtractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t formatVersion = de.getU8(cursor);
  ꦪꦼꦤ꧀ (formatVersion != ELFAttrs::Format_Version)
    ꦮꦭꦶ createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(formatVersion));

  ꦱꦼꦢꦼꦭ (!de.eof(cursor)) {
    uint32_t sectionLength = de.getU32(cursor);
    ꦪꦼꦤ꧀ (!cursor)
      ꦮꦭꦶ cursor.takeError();

    ꦪꦼꦤ꧀ (sw) {
      sw->startLine() << "Section " << ++sectionNumber << " {\n";
      sw->indent();
    }

    ꦪꦼꦤ꧀ (sectionLength < 4 || cursor.tell() - 4 + sectionLength > section.size())
      ꦮꦭꦶ createStringError(errc::invalid_argument,
                               "invalid section length " +
                                   Twine(sectionLength) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    ꦪꦼꦤ꧀ (Error e = parseSubsection(sectionLength))
      ꦮꦭꦶ e;
    ꦪꦼꦤ꧀ (sw) {
      sw->unindent();
      sw->startLine() << "}\n";
    }
  }

  ꦮꦭꦶ cursor.takeError();
}
