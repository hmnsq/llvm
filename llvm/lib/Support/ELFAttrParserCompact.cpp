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
  uint64_t मानम्_2 = de.getULEB128(cursor);
  सचे (मानम्_2 >= strings.size()) {
    printAttribute(tag, मानम्_2, "");
    पटिनिवत्त createStringError(errc::invalid_argument,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(मानम्_2));
  }
  printAttribute(tag, मानम्_2, strings[मानम्_2]);
  पटिनिवत्त Error::success();
}

Error ELFCompactAttrParser::integerAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/मिच्छा);
  uint64_t मानम्_2 = de.getULEB128(cursor);
  attributes.निवेशय(std::make_pair(tag, मानम्_2));

  सचे (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_सङ्ख्या("Tag", tag);
    सचे (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->print_सङ्ख्या("Value", मानम्_2);
  }
  पटिनिवत्त Error::success();
}

Error ELFCompactAttrParser::stringAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/मिच्छा);
  StringRef desc = de.getCStrRef(cursor);
  setAttributeString(tag, desc);

  सचे (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_सङ्ख्या("Tag", tag);
    सचे (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  पटिनिवत्त Error::success();
}

शून्य ELFCompactAttrParser::printAttribute(unsigned tag, unsigned मानम्_2,
                                          StringRef मानम्_Desc) {
  attributes.निवेशय(std::make_pair(tag, मानम्_2));

  सचे (sw) {
    StringRef tagName = ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                                   /*hasTagPrefix=*/मिच्छा);
    DictScope as(*sw, "Attribute");
    sw->print_सङ्ख्या("Tag", tag);
    sw->print_सङ्ख्या("Value", मानम्_2);
    सचे (!tagName.empty())
      sw->printString("TagName", tagName);
    सचे (!मानम्_Desc.empty())
      sw->printString("Description", मानम्_Desc);
  }
}

शून्य ELFCompactAttrParser::parseIndexList(SmallVectorImpl<uint8_t> &indexList) {
  अत्थाय (;;) {
    uint64_t मानम्_2 = de.getULEB128(cursor);
    सचे (!cursor || !मानम्_2)
      भङ्ग;
    indexList.प्रक्षिप_back(मानम्_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t length) {
  uint64_t pos;
  uint64_t end = cursor.tell() + length;
  याव ((pos = cursor.tell()) < end) {
    uint64_t tag = de.getULEB128(cursor);
    तार्किक handled;
    सचे (Error e = handler(tag, handled))
      पटिनिवत्त e;

    सचे (!handled) {
      सचे (tag < 32) {
        पटिनिवत्त createStringError(errc::invalid_argument,
                                 "invalid tag 0x" + Twine::utohexstr(tag) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      सचे (tag % 2 == 0) {
        सचे (Error e = integerAttribute(tag))
          पटिनिवत्त e;
      } अञ्ञथा {
        सचे (Error e = stringAttribute(tag))
          पटिनिवत्त e;
      }
    }
  }
  पटिनिवत्त Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t length) {
  uint64_t end = cursor.tell() - परिमाण(length) + length;
  StringRef vendorName = de.getCStrRef(cursor);
  सचे (sw) {
    sw->print_सङ्ख्या("SectionLength", length);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  सचे (vendorName.lower() != vendor) {
    cursor.seek(end);
    पटिनिवत्त Error::success();
  }

  याव (cursor.tell() < end) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t tag = de.getU8(cursor);
    uint32_t size = de.getU32(cursor);
    सचे (!cursor)
      पटिनिवत्त cursor.takeError();

    सचे (sw) {
      constexpr EnumStringDef<unsigned> TagNameDefs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::चिह्नम्},
      };
      स्थिर constexpr auto TagNames = BUILD_ENUM_STRINGS(TagNameDefs);

      sw->printEnum("Tag", tag, EnumStrings(TagNames));
      sw->print_सङ्ख्या("Size", size);
    }
    सचे (size < 5)
      पटिनिवत्त createStringError(errc::invalid_argument,
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
    पक्ष ELFAttrs::चिह्नम्:
      scopeName = "SymbolAttributes";
      indexName = "Symbols";
      parseIndexList(indices);
      भङ्ग;
    स्वभाव:
      पटिनिवत्त createStringError(errc::invalid_argument,
                               "unrecognized tag 0x" + Twine::utohexstr(tag) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    सचे (sw) {
      DictScope scope(*sw, scopeName);
      सचे (!indices.empty())
        sw->printList(indexName, indices);
      सचे (Error e = parseAttributeList(size - 5))
        पटिनिवत्त e;
    } अञ्ञथा सचे (Error e = parseAttributeList(size - 5))
      पटिनिवत्त e;
  }
  पटिनिवत्त Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  unsigned section_सङ्ख्या = 0;
  de = DataExtractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  संरचना ClearCursorError {
    DataExtractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t formatVersion = de.getU8(cursor);
  सचे (formatVersion != ELFAttrs::Format_Version)
    पटिनिवत्त createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(formatVersion));

  याव (!de.eof(cursor)) {
    uint32_t sectionLength = de.getU32(cursor);
    सचे (!cursor)
      पटिनिवत्त cursor.takeError();

    सचे (sw) {
      sw->start_पङ्क्तिः() << "Section " << ++section_सङ्ख्या << " {\n";
      sw->indent();
    }

    सचे (sectionLength < 4 || cursor.tell() - 4 + sectionLength > section.size())
      पटिनिवत्त createStringError(errc::invalid_argument,
                               "invalid section length " +
                                   Twine(sectionLength) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    सचे (Error e = parseSubsection(sectionLength))
      पटिनिवत्त e;
    सचे (sw) {
      sw->unindent();
      sw->start_पङ्क्तिः() << "}\n";
    }
  }

  पटिनिवत्त cursor.takeError();
}
