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

#समावेश "llvm/Support/ELFAttrParserCompact.h"
#समावेश "llvm/ADT/Enum.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/Support/Errc.h"
#समावेश "llvm/Support/ScopedPrinter.h"

using namespace llvm;
using namespace llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    स्थिर अक्षर *name, अचिन्हित tag, ArrayRef<स्थिर अक्षर *> strings) {
  uint64_t मूल्य_2 = de.getULEB128(cursor);
  अगर (मूल्य_2 >= strings.आकार_2()) {
    printAttribute(tag, मूल्य_2, "");
    वापस createStringError(errc::invalid_argument,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(मूल्य_2));
  }
  printAttribute(tag, मूल्य_2, strings[मूल्य_2]);
  वापस Error::success();
}

Error ELFCompactAttrParser::integerAttribute(अचिन्हित tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/false);
  uint64_t मूल्य_2 = de.getULEB128(cursor);
  attributes.जोड़ो(std::make_pair(tag, मूल्य_2));

  अगर (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    अगर (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->print_संख्या("Value", मूल्य_2);
  }
  वापस Error::success();
}

Error ELFCompactAttrParser::stringAttribute(अचिन्हित tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/false);
  StringRef desc = de.getCStrRef(cursor);
  setAttributeString(tag, desc);

  अगर (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    अगर (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  वापस Error::success();
}

व्योम ELFCompactAttrParser::printAttribute(अचिन्हित tag, अचिन्हित मूल्य_2,
                                          StringRef मूल्य_Desc) {
  attributes.जोड़ो(std::make_pair(tag, मूल्य_2));

  अगर (sw) {
    StringRef tagName = ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                                   /*hasTagPrefix=*/false);
    DictScope as(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    sw->print_संख्या("Value", मूल्य_2);
    अगर (!tagName.empty())
      sw->printString("TagName", tagName);
    अगर (!मूल्य_Desc.empty())
      sw->printString("Description", मूल्य_Desc);
  }
}

व्योम ELFCompactAttrParser::parse_सूचकांक_List(SmallVectorImpl<uint8_t> &सूचकांक_List) {
  क्रम (;;) {
    uint64_t मूल्य_2 = de.getULEB128(cursor);
    अगर (!cursor || !मूल्य_2)
      अवरोध;
    सूचकांक_List.धकेलो_back(मूल्य_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t लंबाई_2) {
  uint64_t pos;
  uint64_t end = cursor.tell() + लंबाई_2;
  जबतक ((pos = cursor.tell()) < end) {
    uint64_t tag = de.getULEB128(cursor);
    bool handled;
    अगर (Error e = handler(tag, handled))
      वापस e;

    अगर (!handled) {
      अगर (tag < 32) {
        वापस createStringError(errc::invalid_argument,
                                 "invalid tag 0x" + Twine::utohexstr(tag) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      अगर (tag % 2 == 0) {
        अगर (Error e = integerAttribute(tag))
          वापस e;
      } अन्यथा {
        अगर (Error e = stringAttribute(tag))
          वापस e;
      }
    }
  }
  वापस Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t लंबाई_2) {
  uint64_t end = cursor.tell() - माप(लंबाई_2) + लंबाई_2;
  StringRef vendorName = de.getCStrRef(cursor);
  अगर (sw) {
    sw->print_संख्या("SectionLength", लंबाई_2);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  अगर (vendorName.lower() != vendor) {
    cursor.seek(end);
    वापस Error::success();
  }

  जबतक (cursor.tell() < end) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t tag = de.getU8(cursor);
    uint32_t आकार_2 = de.getU32(cursor);
    अगर (!cursor)
      वापस cursor.takeError();

    अगर (sw) {
      constexpr EnumStringDef<अचिन्हित> TagNameDefs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::प्रतीक},
      };
      अटल constexpr स्वतः TagNames = BUILD_ENUM_STRINGS(TagNameDefs);

      sw->printEnum("Tag", tag, EnumStrings(TagNames));
      sw->print_संख्या("Size", आकार_2);
    }
    अगर (आकार_2 < 5)
      वापस createStringError(errc::invalid_argument,
                               "invalid attribute size " + Twine(आकार_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, सूचकांक_Name;
    SmallVector<uint8_t, 8> indices;
    चयन (tag) {
    हाल ELFAttrs::File:
      scopeName = "FileAttributes";
      अवरोध;
    हाल ELFAttrs::Section:
      scopeName = "SectionAttributes";
      सूचकांक_Name = "Sections";
      parse_सूचकांक_List(indices);
      अवरोध;
    हाल ELFAttrs::प्रतीक:
      scopeName = "SymbolAttributes";
      सूचकांक_Name = "Symbols";
      parse_सूचकांक_List(indices);
      अवरोध;
    शेष:
      वापस createStringError(errc::invalid_argument,
                               "unrecognized tag 0x" + Twine::utohexstr(tag) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    अगर (sw) {
      DictScope scope(*sw, scopeName);
      अगर (!indices.empty())
        sw->printList(सूचकांक_Name, indices);
      अगर (Error e = parseAttributeList(आकार_2 - 5))
        वापस e;
    } अन्यथा अगर (Error e = parseAttributeList(आकार_2 - 5))
      वापस e;
  }
  वापस Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  अचिन्हित section_संख्या = 0;
  de = तथ्य_Extractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  काष्ठा ClearCursorError {
    तथ्य_Extractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } सफा{cursor};

  // Unrecognized format-version.
  uint8_t formatVersion = de.getU8(cursor);
  अगर (formatVersion != ELFAttrs::Format_Version)
    वापस createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(formatVersion));

  जबतक (!de.खप(cursor)) {
    uint32_t section_लंबाई = de.getU32(cursor);
    अगर (!cursor)
      वापस cursor.takeError();

    अगर (sw) {
      sw->start_पंक्ति() << "Section " << ++section_संख्या << " {\n";
      sw->indent();
    }

    अगर (section_लंबाई < 4 || cursor.tell() - 4 + section_लंबाई > section.आकार_2())
      वापस createStringError(errc::invalid_argument,
                               "invalid section length " +
                                   Twine(section_लंबाई) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    अगर (Error e = parseSubsection(section_लंबाई))
      वापस e;
    अगर (sw) {
      sw->unindent();
      sw->start_पंक्ति() << "}\n";
    }
  }

  वापस cursor.takeError();
}
