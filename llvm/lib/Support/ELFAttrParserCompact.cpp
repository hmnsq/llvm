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

#شامل "llvm/Support/ELFAttrParserCompact.h"
#شامل "llvm/ADT/Enum.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/Errc.h"
#شامل "llvm/Support/ScopedPrinter.h"

using فضاینام llvm;
using فضاینام llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    ثابت نویسه *name, unsigned tag, ArrayRef<ثابت نویسه *> strings) {
  uint64_t مقدار_2 = de.getULEB128(cursor);
  اگر (مقدار_2 >= strings.size()) {
    printAttribute(tag, مقدار_2, "");
    بازگشت createStringError(errc::invalid_argument,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(مقدار_2));
  }
  printAttribute(tag, مقدار_2, strings[مقدار_2]);
  بازگشت Error::success();
}

Error ELFCompactAttrParser::integerAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/نادرست);
  uint64_t مقدار_2 = de.getULEB128(cursor);
  attributes.درج(std::make_pair(tag, مقدار_2));

  اگر (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_عدد("Tag", tag);
    اگر (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->print_عدد("Value", مقدار_2);
  }
  بازگشت Error::success();
}

Error ELFCompactAttrParser::stringAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/نادرست);
  StringRef desc = de.getCStrRef(cursor);
  setAttributeString(tag, desc);

  اگر (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_عدد("Tag", tag);
    اگر (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  بازگشت Error::success();
}

تهی ELFCompactAttrParser::printAttribute(unsigned tag, unsigned مقدار_2,
                                          StringRef مقدار_Desc) {
  attributes.درج(std::make_pair(tag, مقدار_2));

  اگر (sw) {
    StringRef tagName = ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                                   /*hasTagPrefix=*/نادرست);
    DictScope as(*sw, "Attribute");
    sw->print_عدد("Tag", tag);
    sw->print_عدد("Value", مقدار_2);
    اگر (!tagName.empty())
      sw->printString("TagName", tagName);
    اگر (!مقدار_Desc.empty())
      sw->printString("Description", مقدار_Desc);
  }
}

تهی ELFCompactAttrParser::parseIndexList(SmallVectorImpl<uint8_t> &indexList) {
  برای (;;) {
    uint64_t مقدار_2 = de.getULEB128(cursor);
    اگر (!cursor || !مقدار_2)
      بشکن;
    indexList.افزودن_back(مقدار_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t length) {
  uint64_t pos;
  uint64_t end = cursor.tell() + length;
  وقتی ((pos = cursor.tell()) < end) {
    uint64_t tag = de.getULEB128(cursor);
    منطقی handled;
    اگر (Error e = handler(tag, handled))
      بازگشت e;

    اگر (!handled) {
      اگر (tag < 32) {
        بازگشت createStringError(errc::invalid_argument,
                                 "invalid tag 0x" + Twine::utohexstr(tag) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      اگر (tag % 2 == 0) {
        اگر (Error e = integerAttribute(tag))
          بازگشت e;
      } وگرنه {
        اگر (Error e = stringAttribute(tag))
          بازگشت e;
      }
    }
  }
  بازگشت Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t length) {
  uint64_t end = cursor.tell() - اندازه(length) + length;
  StringRef vendorName = de.getCStrRef(cursor);
  اگر (sw) {
    sw->print_عدد("SectionLength", length);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  اگر (vendorName.lower() != vendor) {
    cursor.seek(end);
    بازگشت Error::success();
  }

  وقتی (cursor.tell() < end) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t tag = de.getU8(cursor);
    uint32_t size = de.getU32(cursor);
    اگر (!cursor)
      بازگشت cursor.takeError();

    اگر (sw) {
      constexpr EnumStringDef<unsigned> TagNameDefs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::نماد},
      };
      ایستا constexpr auto TagNames = BUILD_ENUM_STRINGS(TagNameDefs);

      sw->printEnum("Tag", tag, EnumStrings(TagNames));
      sw->print_عدد("Size", size);
    }
    اگر (size < 5)
      بازگشت createStringError(errc::invalid_argument,
                               "invalid attribute size " + Twine(size) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, indexName;
    SmallVector<uint8_t, 8> indices;
    انتخاب (tag) {
    حالت ELFAttrs::File:
      scopeName = "FileAttributes";
      بشکن;
    حالت ELFAttrs::Section:
      scopeName = "SectionAttributes";
      indexName = "Sections";
      parseIndexList(indices);
      بشکن;
    حالت ELFAttrs::نماد:
      scopeName = "SymbolAttributes";
      indexName = "Symbols";
      parseIndexList(indices);
      بشکن;
    پیشفرض:
      بازگشت createStringError(errc::invalid_argument,
                               "unrecognized tag 0x" + Twine::utohexstr(tag) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    اگر (sw) {
      DictScope scope(*sw, scopeName);
      اگر (!indices.empty())
        sw->printList(indexName, indices);
      اگر (Error e = parseAttributeList(size - 5))
        بازگشت e;
    } وگرنه اگر (Error e = parseAttributeList(size - 5))
      بازگشت e;
  }
  بازگشت Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  unsigned section_عدد = 0;
  de = DataExtractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  ساختار ClearCursorError {
    DataExtractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t formatVersion = de.getU8(cursor);
  اگر (formatVersion != ELFAttrs::Format_Version)
    بازگشت createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(formatVersion));

  وقتی (!de.eof(cursor)) {
    uint32_t sectionLength = de.getU32(cursor);
    اگر (!cursor)
      بازگشت cursor.takeError();

    اگر (sw) {
      sw->start_سطر() << "Section " << ++section_عدد << " {\n";
      sw->indent();
    }

    اگر (sectionLength < 4 || cursor.tell() - 4 + sectionLength > section.size())
      بازگشت createStringError(errc::invalid_argument,
                               "invalid section length " +
                                   Twine(sectionLength) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    اگر (Error e = parseSubsection(sectionLength))
      بازگشت e;
    اگر (sw) {
      sw->unindent();
      sw->start_سطر() << "}\n";
    }
  }

  بازگشت cursor.takeError();
}
