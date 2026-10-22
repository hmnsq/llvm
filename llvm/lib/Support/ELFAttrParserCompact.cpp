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

#ضمن "llvm/Support/ELFAttrParserCompact.h"
#ضمن "llvm/ADT/Enum.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/ScopedPrinter.h"

using نطاق llvm;
using نطاق llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    ثابت حرف *name, unsigned tag, ArrayRef<ثابت حرف *> strings) {
  uint64_t value = de.getULEB128(cursor);
  𐤀𐤌 (value >= strings.size()) {
    printAttribute(tag, value, "");
    أرجع createStringError(errc::invalid_argument,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(value));
  }
  printAttribute(tag, value, strings[value]);
  أرجع Error::success();
}

Error ELFCompactAttrParser::integerAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/خطأ);
  uint64_t value = de.getULEB128(cursor);
  attributes.insert(std::make_pair(tag, value));

  𐤀𐤌 (sw) {
    DictScope scope(*sw, "Attribute");
    sw->printNumber("Tag", tag);
    𐤀𐤌 (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printNumber("Value", value);
  }
  أرجع Error::success();
}

Error ELFCompactAttrParser::stringAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/خطأ);
  StringRef desc = de.getCStrRef(cursor);
  setAttributeString(tag, desc);

  𐤀𐤌 (sw) {
    DictScope scope(*sw, "Attribute");
    sw->printNumber("Tag", tag);
    𐤀𐤌 (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  أرجع Error::success();
}

فراغ ELFCompactAttrParser::printAttribute(unsigned tag, unsigned value,
                                          StringRef valueDesc) {
  attributes.insert(std::make_pair(tag, value));

  𐤀𐤌 (sw) {
    StringRef tagName = ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                                   /*hasTagPrefix=*/خطأ);
    DictScope as(*sw, "Attribute");
    sw->printNumber("Tag", tag);
    sw->printNumber("Value", value);
    𐤀𐤌 (!tagName.empty())
      sw->printString("TagName", tagName);
    𐤀𐤌 (!valueDesc.empty())
      sw->printString("Description", valueDesc);
  }
}

فراغ ELFCompactAttrParser::parseIndexList(SmallVectorImpl<uint8_t> &indexList) {
  لكل (;;) {
    uint64_t value = de.getULEB128(cursor);
    𐤀𐤌 (!cursor || !value)
      اكسر;
    indexList.push_back(value);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t length) {
  uint64_t pos;
  uint64_t end = cursor.tell() + length;
  طالما ((pos = cursor.tell()) < end) {
    uint64_t tag = de.getULEB128(cursor);
    منطقي handled;
    𐤀𐤌 (Error e = handler(tag, handled))
      أرجع e;

    𐤀𐤌 (!handled) {
      𐤀𐤌 (tag < 32) {
        أرجع createStringError(errc::invalid_argument,
                                 "invalid tag 0x" + Twine::utohexstr(tag) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      𐤀𐤌 (tag % 2 == 0) {
        𐤀𐤌 (Error e = integerAttribute(tag))
          أرجع e;
      } وإلا {
        𐤀𐤌 (Error e = stringAttribute(tag))
          أرجع e;
      }
    }
  }
  أرجع Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t length) {
  uint64_t end = cursor.tell() - حجم(length) + length;
  StringRef vendorName = de.getCStrRef(cursor);
  𐤀𐤌 (sw) {
    sw->printNumber("SectionLength", length);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  𐤀𐤌 (vendorName.lower() != vendor) {
    cursor.seek(end);
    أرجع Error::success();
  }

  طالما (cursor.tell() < end) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t tag = de.getU8(cursor);
    uint32_t size = de.getU32(cursor);
    𐤀𐤌 (!cursor)
      أرجع cursor.takeError();

    𐤀𐤌 (sw) {
      constexpr EnumStringDef<unsigned> TagNameDefs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::Symbol},
      };
      ساكن constexpr auto TagNames = BUILD_ENUM_STRINGS(TagNameDefs);

      sw->printEnum("Tag", tag, EnumStrings(TagNames));
      sw->printNumber("Size", size);
    }
    𐤀𐤌 (size < 5)
      أرجع createStringError(errc::invalid_argument,
                               "invalid attribute size " + Twine(size) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, indexName;
    SmallVector<uint8_t, 8> indices;
    بدل (tag) {
    حالة ELFAttrs::File:
      scopeName = "FileAttributes";
      اكسر;
    حالة ELFAttrs::Section:
      scopeName = "SectionAttributes";
      indexName = "Sections";
      parseIndexList(indices);
      اكسر;
    حالة ELFAttrs::Symbol:
      scopeName = "SymbolAttributes";
      indexName = "Symbols";
      parseIndexList(indices);
      اكسر;
    افتراضي:
      أرجع createStringError(errc::invalid_argument,
                               "unrecognized tag 0x" + Twine::utohexstr(tag) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    𐤀𐤌 (sw) {
      DictScope scope(*sw, scopeName);
      𐤀𐤌 (!indices.empty())
        sw->printList(indexName, indices);
      𐤀𐤌 (Error e = parseAttributeList(size - 5))
        أرجع e;
    } وإلا 𐤀𐤌 (Error e = parseAttributeList(size - 5))
      أرجع e;
  }
  أرجع Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  unsigned sectionNumber = 0;
  de = DataExtractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  بنية ClearCursorError {
    DataExtractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t formatVersion = de.getU8(cursor);
  𐤀𐤌 (formatVersion != ELFAttrs::Format_Version)
    أرجع createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(formatVersion));

  طالما (!de.eof(cursor)) {
    uint32_t sectionLength = de.getU32(cursor);
    𐤀𐤌 (!cursor)
      أرجع cursor.takeError();

    𐤀𐤌 (sw) {
      sw->startLine() << "Section " << ++sectionNumber << " {\n";
      sw->indent();
    }

    𐤀𐤌 (sectionLength < 4 || cursor.tell() - 4 + sectionLength > section.size())
      أرجع createStringError(errc::invalid_argument,
                               "invalid section length " +
                                   Twine(sectionLength) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    𐤀𐤌 (Error e = parseSubsection(sectionLength))
      أرجع e;
    𐤀𐤌 (sw) {
      sw->unindent();
      sw->startLine() << "}\n";
    }
  }

  أرجع cursor.takeError();
}
