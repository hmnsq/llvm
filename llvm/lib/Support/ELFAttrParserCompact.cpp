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

#அந்தர்பாவ "llvm/Support/ELFAttrParserCompact.h"
#அந்தர்பாவ "llvm/ADT/Enum.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Support/Errc.h"
#அந்தர்பாவ "llvm/Support/ScopedPrinter.h"

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    மாறிலி எழுத்து *name, unsigned tag, ArrayRef<மாறிலி எழுத்து *> strings) {
  uint64_t மதிப்பு_2 = de.getULEB128(cursor);
  எனில் (மதிப்பு_2 >= strings.size()) {
    printAttribute(tag, மதிப்பு_2, "");
    திரும்பு createStringError(errc::invalid_argument,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(மதிப்பு_2));
  }
  printAttribute(tag, மதிப்பு_2, strings[மதிப்பு_2]);
  திரும்பு Error::success();
}

Error ELFCompactAttrParser::integerAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/பொய்);
  uint64_t மதிப்பு_2 = de.getULEB128(cursor);
  attributes.செருகு(std::make_pair(tag, மதிப்பு_2));

  எனில் (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_எண்("Tag", tag);
    எனில் (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->print_எண்("Value", மதிப்பு_2);
  }
  திரும்பு Error::success();
}

Error ELFCompactAttrParser::stringAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/பொய்);
  StringRef desc = de.getCStrRef(cursor);
  setAttributeString(tag, desc);

  எனில் (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_எண்("Tag", tag);
    எனில் (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  திரும்பு Error::success();
}

வெறுமை ELFCompactAttrParser::printAttribute(unsigned tag, unsigned மதிப்பு_2,
                                          StringRef மதிப்பு_Desc) {
  attributes.செருகு(std::make_pair(tag, மதிப்பு_2));

  எனில் (sw) {
    StringRef tagName = ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                                   /*hasTagPrefix=*/பொய்);
    DictScope as(*sw, "Attribute");
    sw->print_எண்("Tag", tag);
    sw->print_எண்("Value", மதிப்பு_2);
    எனில் (!tagName.empty())
      sw->printString("TagName", tagName);
    எனில் (!மதிப்பு_Desc.empty())
      sw->printString("Description", மதிப்பு_Desc);
  }
}

வெறுமை ELFCompactAttrParser::parseIndexList(SmallVectorImpl<uint8_t> &indexList) {
  க்கு (;;) {
    uint64_t மதிப்பு_2 = de.getULEB128(cursor);
    எனில் (!cursor || !மதிப்பு_2)
      நிறுத்து;
    indexList.தள்ளு_back(மதிப்பு_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t length) {
  uint64_t pos;
  uint64_t end = cursor.tell() + length;
  போது ((pos = cursor.tell()) < end) {
    uint64_t tag = de.getULEB128(cursor);
    தருக்க handled;
    எனில் (Error e = handler(tag, handled))
      திரும்பு e;

    எனில் (!handled) {
      எனில் (tag < 32) {
        திரும்பு createStringError(errc::invalid_argument,
                                 "invalid tag 0x" + Twine::utohexstr(tag) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      எனில் (tag % 2 == 0) {
        எனில் (Error e = integerAttribute(tag))
          திரும்பு e;
      } இல்லையெனில் {
        எனில் (Error e = stringAttribute(tag))
          திரும்பு e;
      }
    }
  }
  திரும்பு Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t length) {
  uint64_t end = cursor.tell() - அளவு(length) + length;
  StringRef vendorName = de.getCStrRef(cursor);
  எனில் (sw) {
    sw->print_எண்("SectionLength", length);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  எனில் (vendorName.lower() != vendor) {
    cursor.seek(end);
    திரும்பு Error::success();
  }

  போது (cursor.tell() < end) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t tag = de.getU8(cursor);
    uint32_t size = de.getU32(cursor);
    எனில் (!cursor)
      திரும்பு cursor.takeError();

    எனில் (sw) {
      constexpr EnumStringDef<unsigned> TagNameDefs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::குறியீடு},
      };
      நிலையான constexpr auto TagNames = BUILD_ENUM_STRINGS(TagNameDefs);

      sw->printEnum("Tag", tag, EnumStrings(TagNames));
      sw->print_எண்("Size", size);
    }
    எனில் (size < 5)
      திரும்பு createStringError(errc::invalid_argument,
                               "invalid attribute size " + Twine(size) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, indexName;
    SmallVector<uint8_t, 8> indices;
    தேர்வு (tag) {
    நிலை ELFAttrs::File:
      scopeName = "FileAttributes";
      நிறுத்து;
    நிலை ELFAttrs::Section:
      scopeName = "SectionAttributes";
      indexName = "Sections";
      parseIndexList(indices);
      நிறுத்து;
    நிலை ELFAttrs::குறியீடு:
      scopeName = "SymbolAttributes";
      indexName = "Symbols";
      parseIndexList(indices);
      நிறுத்து;
    இயல்பு:
      திரும்பு createStringError(errc::invalid_argument,
                               "unrecognized tag 0x" + Twine::utohexstr(tag) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    எனில் (sw) {
      DictScope scope(*sw, scopeName);
      எனில் (!indices.empty())
        sw->printList(indexName, indices);
      எனில் (Error e = parseAttributeList(size - 5))
        திரும்பு e;
    } இல்லையெனில் எனில் (Error e = parseAttributeList(size - 5))
      திரும்பு e;
  }
  திரும்பு Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  unsigned section_எண் = 0;
  de = DataExtractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  கட்டமைப்பு ClearCursorError {
    DataExtractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t formatVersion = de.getU8(cursor);
  எனில் (formatVersion != ELFAttrs::Format_Version)
    திரும்பு createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(formatVersion));

  போது (!de.eof(cursor)) {
    uint32_t sectionLength = de.getU32(cursor);
    எனில் (!cursor)
      திரும்பு cursor.takeError();

    எனில் (sw) {
      sw->start_வரி() << "Section " << ++section_எண் << " {\n";
      sw->indent();
    }

    எனில் (sectionLength < 4 || cursor.tell() - 4 + sectionLength > section.size())
      திரும்பு createStringError(errc::invalid_argument,
                               "invalid section length " +
                                   Twine(sectionLength) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    எனில் (Error e = parseSubsection(sectionLength))
      திரும்பு e;
    எனில் (sw) {
      sw->unindent();
      sw->start_வரி() << "}\n";
    }
  }

  திரும்பு cursor.takeError();
}
