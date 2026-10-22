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

#അന്തര്ഭാവ "llvm/Support/ELFAttrParserCompact.h"
#അന്തര്ഭാവ "llvm/ADT/Enum.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/Support/Errc.h"
#അന്തര്ഭാവ "llvm/Support/ScopedPrinter.h"

using നാമാകാശ llvm;
using നാമാകാശ llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    സ്ഥിരം അക്ഷരം *name, unsigned tag, ArrayRef<സ്ഥിരം അക്ഷരം *> strings) {
  uint64_t മൂല്യം_2 = de.getULEB128(cursor);
  എങ്കിൽ (മൂല്യം_2 >= strings.size()) {
    printAttribute(tag, മൂല്യം_2, "");
    തിരികെ createStringError(errc::invalid_argument,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(മൂല്യം_2));
  }
  printAttribute(tag, മൂല്യം_2, strings[മൂല്യം_2]);
  തിരികെ Error::success();
}

Error ELFCompactAttrParser::integerAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/അസത്യം);
  uint64_t മൂല്യം_2 = de.getULEB128(cursor);
  attributes.തിരുകുക(std::make_pair(tag, മൂല്യം_2));

  എങ്കിൽ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_സംഖ്യ("Tag", tag);
    എങ്കിൽ (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->print_സംഖ്യ("Value", മൂല്യം_2);
  }
  തിരികെ Error::success();
}

Error ELFCompactAttrParser::stringAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/അസത്യം);
  StringRef desc = de.getCStrRef(cursor);
  setAttributeString(tag, desc);

  എങ്കിൽ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_സംഖ്യ("Tag", tag);
    എങ്കിൽ (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  തിരികെ Error::success();
}

ശൂന്യം ELFCompactAttrParser::printAttribute(unsigned tag, unsigned മൂല്യം_2,
                                          StringRef മൂല്യം_Desc) {
  attributes.തിരുകുക(std::make_pair(tag, മൂല്യം_2));

  എങ്കിൽ (sw) {
    StringRef tagName = ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                                   /*hasTagPrefix=*/അസത്യം);
    DictScope as(*sw, "Attribute");
    sw->print_സംഖ്യ("Tag", tag);
    sw->print_സംഖ്യ("Value", മൂല്യം_2);
    എങ്കിൽ (!tagName.empty())
      sw->printString("TagName", tagName);
    എങ്കിൽ (!മൂല്യം_Desc.empty())
      sw->printString("Description", മൂല്യം_Desc);
  }
}

ശൂന്യം ELFCompactAttrParser::parseIndexList(SmallVectorImpl<uint8_t> &indexList) {
  വേണ്ടി (;;) {
    uint64_t മൂല്യം_2 = de.getULEB128(cursor);
    എങ്കിൽ (!cursor || !മൂല്യം_2)
      നിർത്തുക;
    indexList.തള്ളുക_back(മൂല്യം_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t length) {
  uint64_t pos;
  uint64_t end = cursor.tell() + length;
  ആകുമ്പോൾ ((pos = cursor.tell()) < end) {
    uint64_t tag = de.getULEB128(cursor);
    തർക്കം handled;
    എങ്കിൽ (Error e = handler(tag, handled))
      തിരികെ e;

    എങ്കിൽ (!handled) {
      എങ്കിൽ (tag < 32) {
        തിരികെ createStringError(errc::invalid_argument,
                                 "invalid tag 0x" + Twine::utohexstr(tag) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      എങ്കിൽ (tag % 2 == 0) {
        എങ്കിൽ (Error e = integerAttribute(tag))
          തിരികെ e;
      } അല്ലെങ്കിൽ {
        എങ്കിൽ (Error e = stringAttribute(tag))
          തിരികെ e;
      }
    }
  }
  തിരികെ Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t length) {
  uint64_t end = cursor.tell() - വലുപ്പം(length) + length;
  StringRef vendorName = de.getCStrRef(cursor);
  എങ്കിൽ (sw) {
    sw->print_സംഖ്യ("SectionLength", length);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  എങ്കിൽ (vendorName.lower() != vendor) {
    cursor.seek(end);
    തിരികെ Error::success();
  }

  ആകുമ്പോൾ (cursor.tell() < end) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t tag = de.getU8(cursor);
    uint32_t size = de.getU32(cursor);
    എങ്കിൽ (!cursor)
      തിരികെ cursor.takeError();

    എങ്കിൽ (sw) {
      constexpr EnumStringDef<unsigned> TagNameDefs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::ചിഹ്നം},
      };
      സ്ഥിതം constexpr auto TagNames = BUILD_ENUM_STRINGS(TagNameDefs);

      sw->printEnum("Tag", tag, EnumStrings(TagNames));
      sw->print_സംഖ്യ("Size", size);
    }
    എങ്കിൽ (size < 5)
      തിരികെ createStringError(errc::invalid_argument,
                               "invalid attribute size " + Twine(size) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, indexName;
    SmallVector<uint8_t, 8> indices;
    തിരഞ്ഞെടുപ്പ് (tag) {
    അവസ്ഥ ELFAttrs::File:
      scopeName = "FileAttributes";
      നിർത്തുക;
    അവസ്ഥ ELFAttrs::Section:
      scopeName = "SectionAttributes";
      indexName = "Sections";
      parseIndexList(indices);
      നിർത്തുക;
    അവസ്ഥ ELFAttrs::ചിഹ്നം:
      scopeName = "SymbolAttributes";
      indexName = "Symbols";
      parseIndexList(indices);
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      തിരികെ createStringError(errc::invalid_argument,
                               "unrecognized tag 0x" + Twine::utohexstr(tag) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    എങ്കിൽ (sw) {
      DictScope scope(*sw, scopeName);
      എങ്കിൽ (!indices.empty())
        sw->printList(indexName, indices);
      എങ്കിൽ (Error e = parseAttributeList(size - 5))
        തിരികെ e;
    } അല്ലെങ്കിൽ എങ്കിൽ (Error e = parseAttributeList(size - 5))
      തിരികെ e;
  }
  തിരികെ Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  unsigned section_സംഖ്യ = 0;
  de = DataExtractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  ഘടന ClearCursorError {
    DataExtractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t formatVersion = de.getU8(cursor);
  എങ്കിൽ (formatVersion != ELFAttrs::Format_Version)
    തിരികെ createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(formatVersion));

  ആകുമ്പോൾ (!de.eof(cursor)) {
    uint32_t sectionLength = de.getU32(cursor);
    എങ്കിൽ (!cursor)
      തിരികെ cursor.takeError();

    എങ്കിൽ (sw) {
      sw->start_വരി() << "Section " << ++section_സംഖ്യ << " {\n";
      sw->indent();
    }

    എങ്കിൽ (sectionLength < 4 || cursor.tell() - 4 + sectionLength > section.size())
      തിരികെ createStringError(errc::invalid_argument,
                               "invalid section length " +
                                   Twine(sectionLength) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    എങ്കിൽ (Error e = parseSubsection(sectionLength))
      തിരികെ e;
    എങ്കിൽ (sw) {
      sw->unindent();
      sw->start_വരി() << "}\n";
    }
  }

  തിരികെ cursor.takeError();
}
