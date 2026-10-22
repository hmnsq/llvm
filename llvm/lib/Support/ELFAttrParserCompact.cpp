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
  uint64_t قيمة_2 = de.getULEB128(cursor);
  إذا (قيمة_2 >= strings.size()) {
    printAttribute(tag, قيمة_2, "");
    إرجاع createStringError(errc::invalid_argument,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(قيمة_2));
  }
  printAttribute(tag, قيمة_2, strings[قيمة_2]);
  إرجاع Error::success();
}

Error ELFCompactAttrParser::integerAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/خطأ);
  uint64_t قيمة_2 = de.getULEB128(cursor);
  attributes.أدرج(std::make_pair(tag, قيمة_2));

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_العدد("Tag", tag);
    إذا (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->print_العدد("Value", قيمة_2);
  }
  إرجاع Error::success();
}

Error ELFCompactAttrParser::stringAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/خطأ);
  StringRef desc = de.getCStrRef(cursor);
  setAttributeString(tag, desc);

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_العدد("Tag", tag);
    إذا (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  إرجاع Error::success();
}

فراغ ELFCompactAttrParser::printAttribute(unsigned tag, unsigned قيمة_2,
                                          StringRef قيمة_Desc) {
  attributes.أدرج(std::make_pair(tag, قيمة_2));

  إذا (sw) {
    StringRef tagName = ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                                   /*hasTagPrefix=*/خطأ);
    DictScope as(*sw, "Attribute");
    sw->print_العدد("Tag", tag);
    sw->print_العدد("Value", قيمة_2);
    إذا (!tagName.empty())
      sw->printString("TagName", tagName);
    إذا (!قيمة_Desc.empty())
      sw->printString("Description", قيمة_Desc);
  }
}

فراغ ELFCompactAttrParser::parseIndexList(SmallVectorImpl<uint8_t> &indexList) {
  لكل (;;) {
    uint64_t قيمة_2 = de.getULEB128(cursor);
    إذا (!cursor || !قيمة_2)
      اكسر;
    indexList.ادفع_back(قيمة_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t length) {
  uint64_t pos;
  uint64_t end = cursor.tell() + length;
  بينما ((pos = cursor.tell()) < end) {
    uint64_t tag = de.getULEB128(cursor);
    منطقي handled;
    إذا (Error e = handler(tag, handled))
      إرجاع e;

    إذا (!handled) {
      إذا (tag < 32) {
        إرجاع createStringError(errc::invalid_argument,
                                 "invalid tag 0x" + Twine::utohexstr(tag) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      إذا (tag % 2 == 0) {
        إذا (Error e = integerAttribute(tag))
          إرجاع e;
      } وإلا {
        إذا (Error e = stringAttribute(tag))
          إرجاع e;
      }
    }
  }
  إرجاع Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t length) {
  uint64_t end = cursor.tell() - حجم(length) + length;
  StringRef vendorName = de.getCStrRef(cursor);
  إذا (sw) {
    sw->print_العدد("SectionLength", length);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  إذا (vendorName.lower() != vendor) {
    cursor.seek(end);
    إرجاع Error::success();
  }

  بينما (cursor.tell() < end) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t tag = de.getU8(cursor);
    uint32_t size = de.getU32(cursor);
    إذا (!cursor)
      إرجاع cursor.takeError();

    إذا (sw) {
      constexpr EnumStringDef<unsigned> TagNameDefs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::رمز},
      };
      ساكن constexpr auto TagNames = BUILD_ENUM_STRINGS(TagNameDefs);

      sw->printEnum("Tag", tag, EnumStrings(TagNames));
      sw->print_العدد("Size", size);
    }
    إذا (size < 5)
      إرجاع createStringError(errc::invalid_argument,
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
    حالة ELFAttrs::رمز:
      scopeName = "SymbolAttributes";
      indexName = "Symbols";
      parseIndexList(indices);
      اكسر;
    افتراضي:
      إرجاع createStringError(errc::invalid_argument,
                               "unrecognized tag 0x" + Twine::utohexstr(tag) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    إذا (sw) {
      DictScope scope(*sw, scopeName);
      إذا (!indices.empty())
        sw->printList(indexName, indices);
      إذا (Error e = parseAttributeList(size - 5))
        إرجاع e;
    } وإلا إذا (Error e = parseAttributeList(size - 5))
      إرجاع e;
  }
  إرجاع Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  unsigned section_العدد = 0;
  de = DataExtractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  بنية ClearCursorError {
    DataExtractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t formatVersion = de.getU8(cursor);
  إذا (formatVersion != ELFAttrs::Format_Version)
    إرجاع createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(formatVersion));

  بينما (!de.eof(cursor)) {
    uint32_t sectionLength = de.getU32(cursor);
    إذا (!cursor)
      إرجاع cursor.takeError();

    إذا (sw) {
      sw->start_سطر() << "Section " << ++section_العدد << " {\n";
      sw->indent();
    }

    إذا (sectionLength < 4 || cursor.tell() - 4 + sectionLength > section.size())
      إرجاع createStringError(errc::invalid_argument,
                               "invalid section length " +
                                   Twine(sectionLength) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    إذا (Error e = parseSubsection(sectionLength))
      إرجاع e;
    إذا (sw) {
      sw->unindent();
      sw->start_سطر() << "}\n";
    }
  }

  إرجاع cursor.takeError();
}
