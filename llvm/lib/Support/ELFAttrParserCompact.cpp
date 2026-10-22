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
  uint64_t value = de.getULEB128(cursor);
  만약 (value >= strings.size()) {
    printAttribute(tag, value, "");
    반환 createStringError(errc::invalid_argument,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(value));
  }
  printAttribute(tag, value, strings[value]);
  반환 Error::success();
}

Error ELFCompactAttrParser::integerAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/false);
  uint64_t value = de.getULEB128(cursor);
  attributes.insert(std::make_pair(tag, value));

  만약 (sw) {
    DictScope scope(*sw, "Attribute");
    sw->printNumber("Tag", tag);
    만약 (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printNumber("Value", value);
  }
  반환 Error::success();
}

Error ELFCompactAttrParser::stringAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/false);
  StringRef desc = de.getCStrRef(cursor);
  setAttributeString(tag, desc);

  만약 (sw) {
    DictScope scope(*sw, "Attribute");
    sw->printNumber("Tag", tag);
    만약 (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  반환 Error::success();
}

void ELFCompactAttrParser::printAttribute(unsigned tag, unsigned value,
                                          StringRef valueDesc) {
  attributes.insert(std::make_pair(tag, value));

  만약 (sw) {
    StringRef tagName = ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                                   /*hasTagPrefix=*/false);
    DictScope as(*sw, "Attribute");
    sw->printNumber("Tag", tag);
    sw->printNumber("Value", value);
    만약 (!tagName.empty())
      sw->printString("TagName", tagName);
    만약 (!valueDesc.empty())
      sw->printString("Description", valueDesc);
  }
}

void ELFCompactAttrParser::parseIndexList(SmallVectorImpl<uint8_t> &indexList) {
  위해 (;;) {
    uint64_t value = de.getULEB128(cursor);
    만약 (!cursor || !value)
      break;
    indexList.push_back(value);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t length) {
  uint64_t pos;
  uint64_t end = cursor.tell() + length;
  동안 ((pos = cursor.tell()) < end) {
    uint64_t tag = de.getULEB128(cursor);
    bool handled;
    만약 (Error e = handler(tag, handled))
      반환 e;

    만약 (!handled) {
      만약 (tag < 32) {
        반환 createStringError(errc::invalid_argument,
                                 "invalid tag 0x" + Twine::utohexstr(tag) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      만약 (tag % 2 == 0) {
        만약 (Error e = integerAttribute(tag))
          반환 e;
      } 아니면 {
        만약 (Error e = stringAttribute(tag))
          반환 e;
      }
    }
  }
  반환 Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t length) {
  uint64_t end = cursor.tell() - sizeof(length) + length;
  StringRef vendorName = de.getCStrRef(cursor);
  만약 (sw) {
    sw->printNumber("SectionLength", length);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  만약 (vendorName.lower() != vendor) {
    cursor.seek(end);
    반환 Error::success();
  }

  동안 (cursor.tell() < end) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t tag = de.getU8(cursor);
    uint32_t size = de.getU32(cursor);
    만약 (!cursor)
      반환 cursor.takeError();

    만약 (sw) {
      constexpr EnumStringDef<unsigned> TagNameDefs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::Symbol},
      };
      static constexpr auto TagNames = BUILD_ENUM_STRINGS(TagNameDefs);

      sw->printEnum("Tag", tag, EnumStrings(TagNames));
      sw->printNumber("Size", size);
    }
    만약 (size < 5)
      반환 createStringError(errc::invalid_argument,
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
    case ELFAttrs::Symbol:
      scopeName = "SymbolAttributes";
      indexName = "Symbols";
      parseIndexList(indices);
      break;
    default:
      반환 createStringError(errc::invalid_argument,
                               "unrecognized tag 0x" + Twine::utohexstr(tag) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    만약 (sw) {
      DictScope scope(*sw, scopeName);
      만약 (!indices.empty())
        sw->printList(indexName, indices);
      만약 (Error e = parseAttributeList(size - 5))
        반환 e;
    } 아니면 만약 (Error e = parseAttributeList(size - 5))
      반환 e;
  }
  반환 Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  unsigned sectionNumber = 0;
  de = DataExtractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  struct ClearCursorError {
    DataExtractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t formatVersion = de.getU8(cursor);
  만약 (formatVersion != ELFAttrs::Format_Version)
    반환 createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(formatVersion));

  동안 (!de.eof(cursor)) {
    uint32_t sectionLength = de.getU32(cursor);
    만약 (!cursor)
      반환 cursor.takeError();

    만약 (sw) {
      sw->startLine() << "Section " << ++sectionNumber << " {\n";
      sw->indent();
    }

    만약 (sectionLength < 4 || cursor.tell() - 4 + sectionLength > section.size())
      반환 createStringError(errc::invalid_argument,
                               "invalid section length " +
                                   Twine(sectionLength) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    만약 (Error e = parseSubsection(sectionLength))
      반환 e;
    만약 (sw) {
      sw->unindent();
      sw->startLine() << "}\n";
    }
  }

  반환 cursor.takeError();
}
