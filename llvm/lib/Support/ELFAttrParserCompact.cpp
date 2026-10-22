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

#अन्तर्भाव "llvm/Support/ELFAttrParserCompact.h"
#अन्तर्भाव "llvm/ADT/Enum.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Support/Errc.h"
#अन्तर्भाव "llvm/Support/ScopedPrinter.h"

using नामाकाश llvm;
using नामाकाश llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    ध्रुव वर्ण *name, unsigned tag, ArrayRef<ध्रुव वर्ण *> strings) {
  uint64_t मूल्य_2 = de.getULEB128(cursor);
  यदि (मूल्य_2 >= strings.size()) {
    printAttribute(tag, मूल्य_2, "");
    प्रत्यागम createStringError(errc::invalid_argument,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(मूल्य_2));
  }
  printAttribute(tag, मूल्य_2, strings[मूल्य_2]);
  प्रत्यागम Error::success();
}

Error ELFCompactAttrParser::integerAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/असत्य);
  uint64_t मूल्य_2 = de.getULEB128(cursor);
  attributes.जोड़ो(std::make_pair(tag, मूल्य_2));

  यदि (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    यदि (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->print_संख्या("Value", मूल्य_2);
  }
  प्रत्यागम Error::success();
}

Error ELFCompactAttrParser::stringAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/असत्य);
  StringRef desc = de.getCStrRef(cursor);
  setAttributeString(tag, desc);

  यदि (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    यदि (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  प्रत्यागम Error::success();
}

शून्य ELFCompactAttrParser::printAttribute(unsigned tag, unsigned मूल्य_2,
                                          StringRef मूल्य_Desc) {
  attributes.जोड़ो(std::make_pair(tag, मूल्य_2));

  यदि (sw) {
    StringRef tagName = ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                                   /*hasTagPrefix=*/असत्य);
    DictScope as(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    sw->print_संख्या("Value", मूल्य_2);
    यदि (!tagName.empty())
      sw->printString("TagName", tagName);
    यदि (!मूल्य_Desc.empty())
      sw->printString("Description", मूल्य_Desc);
  }
}

शून्य ELFCompactAttrParser::parseIndexList(SmallVectorImpl<uint8_t> &indexList) {
  प्रति (;;) {
    uint64_t मूल्य_2 = de.getULEB128(cursor);
    यदि (!cursor || !मूल्य_2)
      भङ्ग;
    indexList.धकेलो_back(मूल्य_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t length) {
  uint64_t pos;
  uint64_t end = cursor.tell() + length;
  यावत् ((pos = cursor.tell()) < end) {
    uint64_t tag = de.getULEB128(cursor);
    तार्किक handled;
    यदि (Error e = handler(tag, handled))
      प्रत्यागम e;

    यदि (!handled) {
      यदि (tag < 32) {
        प्रत्यागम createStringError(errc::invalid_argument,
                                 "invalid tag 0x" + Twine::utohexstr(tag) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      यदि (tag % 2 == 0) {
        यदि (Error e = integerAttribute(tag))
          प्रत्यागम e;
      } अन्यथा {
        यदि (Error e = stringAttribute(tag))
          प्रत्यागम e;
      }
    }
  }
  प्रत्यागम Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t length) {
  uint64_t end = cursor.tell() - परिमाण(length) + length;
  StringRef vendorName = de.getCStrRef(cursor);
  यदि (sw) {
    sw->print_संख्या("SectionLength", length);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  यदि (vendorName.lower() != vendor) {
    cursor.seek(end);
    प्रत्यागम Error::success();
  }

  यावत् (cursor.tell() < end) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t tag = de.getU8(cursor);
    uint32_t size = de.getU32(cursor);
    यदि (!cursor)
      प्रत्यागम cursor.takeError();

    यदि (sw) {
      constexpr EnumStringDef<unsigned> TagNameDefs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::प्रतीक},
      };
      स्थिर constexpr auto TagNames = BUILD_ENUM_STRINGS(TagNameDefs);

      sw->printEnum("Tag", tag, EnumStrings(TagNames));
      sw->print_संख्या("Size", size);
    }
    यदि (size < 5)
      प्रत्यागम createStringError(errc::invalid_argument,
                               "invalid attribute size " + Twine(size) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, indexName;
    SmallVector<uint8_t, 8> indices;
    विकल्प (tag) {
    पक्ष ELFAttrs::File:
      scopeName = "FileAttributes";
      भङ्ग;
    पक्ष ELFAttrs::Section:
      scopeName = "SectionAttributes";
      indexName = "Sections";
      parseIndexList(indices);
      भङ्ग;
    पक्ष ELFAttrs::प्रतीक:
      scopeName = "SymbolAttributes";
      indexName = "Symbols";
      parseIndexList(indices);
      भङ्ग;
    स्वभाव:
      प्रत्यागम createStringError(errc::invalid_argument,
                               "unrecognized tag 0x" + Twine::utohexstr(tag) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    यदि (sw) {
      DictScope scope(*sw, scopeName);
      यदि (!indices.empty())
        sw->printList(indexName, indices);
      यदि (Error e = parseAttributeList(size - 5))
        प्रत्यागम e;
    } अन्यथा यदि (Error e = parseAttributeList(size - 5))
      प्रत्यागम e;
  }
  प्रत्यागम Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  unsigned section_संख्या = 0;
  de = DataExtractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  संरचना ClearCursorError {
    DataExtractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t formatVersion = de.getU8(cursor);
  यदि (formatVersion != ELFAttrs::Format_Version)
    प्रत्यागम createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(formatVersion));

  यावत् (!de.eof(cursor)) {
    uint32_t sectionLength = de.getU32(cursor);
    यदि (!cursor)
      प्रत्यागम cursor.takeError();

    यदि (sw) {
      sw->start_पंक्ति() << "Section " << ++section_संख्या << " {\n";
      sw->indent();
    }

    यदि (sectionLength < 4 || cursor.tell() - 4 + sectionLength > section.size())
      प्रत्यागम createStringError(errc::invalid_argument,
                               "invalid section length " +
                                   Twine(sectionLength) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    यदि (Error e = parseSubsection(sectionLength))
      प्रत्यागम e;
    यदि (sw) {
      sw->unindent();
      sw->start_पंक्ति() << "}\n";
    }
  }

  प्रत्यागम cursor.takeError();
}
