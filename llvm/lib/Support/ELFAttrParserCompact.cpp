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
    स्थिरांक अक्षर *name, अचिन्हित tag, ArrayRef<स्थिरांक अक्षर *> strings) {
  uint64_t मूल्य_2 = de.getULEB128(cursor);
  जर (मूल्य_2 >= strings.size()) {
    printAttribute(tag, मूल्य_2, "");
    परत createStringError(errc::invalid_argument,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(मूल्य_2));
  }
  printAttribute(tag, मूल्य_2, strings[मूल्य_2]);
  परत Error::success();
}

Error ELFCompactAttrParser::integerAttribute(अचिन्हित tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/असत्य);
  uint64_t मूल्य_2 = de.getULEB128(cursor);
  attributes.घाला(std::make_pair(tag, मूल्य_2));

  जर (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    जर (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->print_संख्या("Value", मूल्य_2);
  }
  परत Error::success();
}

Error ELFCompactAttrParser::stringAttribute(अचिन्हित tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/असत्य);
  StringRef desc = de.getCStrRef(cursor);
  setAttributeString(tag, desc);

  जर (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    जर (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  परत Error::success();
}

शून्य ELFCompactAttrParser::printAttribute(अचिन्हित tag, अचिन्हित मूल्य_2,
                                          StringRef मूल्य_Desc) {
  attributes.घाला(std::make_pair(tag, मूल्य_2));

  जर (sw) {
    StringRef tagName = ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                                   /*hasTagPrefix=*/असत्य);
    DictScope as(*sw, "Attribute");
    sw->print_संख्या("Tag", tag);
    sw->print_संख्या("Value", मूल्य_2);
    जर (!tagName.empty())
      sw->printString("TagName", tagName);
    जर (!मूल्य_Desc.empty())
      sw->printString("Description", मूल्य_Desc);
  }
}

शून्य ELFCompactAttrParser::parseIndexList(SmallVectorImpl<uint8_t> &indexList) {
  साठी (;;) {
    uint64_t मूल्य_2 = de.getULEB128(cursor);
    जर (!cursor || !मूल्य_2)
      थांबा;
    indexList.ढकला_back(मूल्य_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t length) {
  uint64_t pos;
  uint64_t end = cursor.tell() + length;
  पर्यंत ((pos = cursor.tell()) < end) {
    uint64_t tag = de.getULEB128(cursor);
    तार्किक handled;
    जर (Error e = handler(tag, handled))
      परत e;

    जर (!handled) {
      जर (tag < 32) {
        परत createStringError(errc::invalid_argument,
                                 "invalid tag 0x" + Twine::utohexstr(tag) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      जर (tag % 2 == 0) {
        जर (Error e = integerAttribute(tag))
          परत e;
      } अन्यथा {
        जर (Error e = stringAttribute(tag))
          परत e;
      }
    }
  }
  परत Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t length) {
  uint64_t end = cursor.tell() - आकार(length) + length;
  StringRef vendorName = de.getCStrRef(cursor);
  जर (sw) {
    sw->print_संख्या("SectionLength", length);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  जर (vendorName.lower() != vendor) {
    cursor.seek(end);
    परत Error::success();
  }

  पर्यंत (cursor.tell() < end) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t tag = de.getU8(cursor);
    uint32_t size = de.getU32(cursor);
    जर (!cursor)
      परत cursor.takeError();

    जर (sw) {
      constexpr EnumStringDef<अचिन्हित> TagNameDefs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::चिन्ह},
      };
      स्थिर constexpr स्वतः TagNames = BUILD_ENUM_STRINGS(TagNameDefs);

      sw->printEnum("Tag", tag, EnumStrings(TagNames));
      sw->print_संख्या("Size", size);
    }
    जर (size < 5)
      परत createStringError(errc::invalid_argument,
                               "invalid attribute size " + Twine(size) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, indexName;
    SmallVector<uint8_t, 8> indices;
    निवड (tag) {
    स्थिती ELFAttrs::File:
      scopeName = "FileAttributes";
      थांबा;
    स्थिती ELFAttrs::Section:
      scopeName = "SectionAttributes";
      indexName = "Sections";
      parseIndexList(indices);
      थांबा;
    स्थिती ELFAttrs::चिन्ह:
      scopeName = "SymbolAttributes";
      indexName = "Symbols";
      parseIndexList(indices);
      थांबा;
    पूर्वनिर्धारित:
      परत createStringError(errc::invalid_argument,
                               "unrecognized tag 0x" + Twine::utohexstr(tag) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    जर (sw) {
      DictScope scope(*sw, scopeName);
      जर (!indices.empty())
        sw->printList(indexName, indices);
      जर (Error e = parseAttributeList(size - 5))
        परत e;
    } अन्यथा जर (Error e = parseAttributeList(size - 5))
      परत e;
  }
  परत Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  अचिन्हित section_संख्या = 0;
  de = DataExtractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  संरचना ClearCursorError {
    DataExtractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t formatVersion = de.getU8(cursor);
  जर (formatVersion != ELFAttrs::Format_Version)
    परत createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(formatVersion));

  पर्यंत (!de.eof(cursor)) {
    uint32_t sectionLength = de.getU32(cursor);
    जर (!cursor)
      परत cursor.takeError();

    जर (sw) {
      sw->start_ओळ() << "Section " << ++section_संख्या << " {\n";
      sw->indent();
    }

    जर (sectionLength < 4 || cursor.tell() - 4 + sectionLength > section.size())
      परत createStringError(errc::invalid_argument,
                               "invalid section length " +
                                   Twine(sectionLength) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    जर (Error e = parseSubsection(sectionLength))
      परत e;
    जर (sw) {
      sw->unindent();
      sw->start_ओळ() << "}\n";
    }
  }

  परत cursor.takeError();
}
