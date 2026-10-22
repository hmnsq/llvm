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

#અન્તર્ભાવ "llvm/Support/ELFAttrParserCompact.h"
#અન્તર્ભાવ "llvm/ADT/Enum.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/Support/Errc.h"
#અન્તર્ભાવ "llvm/Support/ScopedPrinter.h"

using નામાકાશ llvm;
using નામાકાશ llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    અચલ અક્ષર *name, unsigned tag, ArrayRef<અચલ અક્ષર *> strings) {
  uint64_t કિંમત_2 = de.getULEB128(cursor);
  જો (કિંમત_2 >= strings.size()) {
    printAttribute(tag, કિંમત_2, "");
    પાછા createStringError(errc::invalid_argument,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(કિંમત_2));
  }
  printAttribute(tag, કિંમત_2, strings[કિંમત_2]);
  પાછા Error::success();
}

Error ELFCompactAttrParser::integerAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/ખોટું);
  uint64_t કિંમત_2 = de.getULEB128(cursor);
  attributes.ઉમેરો(std::make_pair(tag, કિંમત_2));

  જો (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_સંખ્યા("Tag", tag);
    જો (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->print_સંખ્યા("Value", કિંમત_2);
  }
  પાછા Error::success();
}

Error ELFCompactAttrParser::stringAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/ખોટું);
  StringRef desc = de.getCStrRef(cursor);
  setAttributeString(tag, desc);

  જો (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_સંખ્યા("Tag", tag);
    જો (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  પાછા Error::success();
}

શૂન્ય ELFCompactAttrParser::printAttribute(unsigned tag, unsigned કિંમત_2,
                                          StringRef કિંમત_Desc) {
  attributes.ઉમેરો(std::make_pair(tag, કિંમત_2));

  જો (sw) {
    StringRef tagName = ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                                   /*hasTagPrefix=*/ખોટું);
    DictScope as(*sw, "Attribute");
    sw->print_સંખ્યા("Tag", tag);
    sw->print_સંખ્યા("Value", કિંમત_2);
    જો (!tagName.empty())
      sw->printString("TagName", tagName);
    જો (!કિંમત_Desc.empty())
      sw->printString("Description", કિંમત_Desc);
  }
}

શૂન્ય ELFCompactAttrParser::parseIndexList(SmallVectorImpl<uint8_t> &indexList) {
  માટે (;;) {
    uint64_t કિંમત_2 = de.getULEB128(cursor);
    જો (!cursor || !કિંમત_2)
      તોડો;
    indexList.ધકેલો_back(કિંમત_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t length) {
  uint64_t pos;
  uint64_t end = cursor.tell() + length;
  જ્યારે ((pos = cursor.tell()) < end) {
    uint64_t tag = de.getULEB128(cursor);
    તાર્કિક handled;
    જો (Error e = handler(tag, handled))
      પાછા e;

    જો (!handled) {
      જો (tag < 32) {
        પાછા createStringError(errc::invalid_argument,
                                 "invalid tag 0x" + Twine::utohexstr(tag) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      જો (tag % 2 == 0) {
        જો (Error e = integerAttribute(tag))
          પાછા e;
      } નહીંતર {
        જો (Error e = stringAttribute(tag))
          પાછા e;
      }
    }
  }
  પાછા Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t length) {
  uint64_t end = cursor.tell() - કદ(length) + length;
  StringRef vendorName = de.getCStrRef(cursor);
  જો (sw) {
    sw->print_સંખ્યા("SectionLength", length);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  જો (vendorName.lower() != vendor) {
    cursor.seek(end);
    પાછા Error::success();
  }

  જ્યારે (cursor.tell() < end) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t tag = de.getU8(cursor);
    uint32_t size = de.getU32(cursor);
    જો (!cursor)
      પાછા cursor.takeError();

    જો (sw) {
      constexpr EnumStringDef<unsigned> TagNameDefs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::પ્રતીક},
      };
      સ્થિર constexpr auto TagNames = BUILD_ENUM_STRINGS(TagNameDefs);

      sw->printEnum("Tag", tag, EnumStrings(TagNames));
      sw->print_સંખ્યા("Size", size);
    }
    જો (size < 5)
      પાછા createStringError(errc::invalid_argument,
                               "invalid attribute size " + Twine(size) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, indexName;
    SmallVector<uint8_t, 8> indices;
    પસંદગી (tag) {
    સ્થિતિ ELFAttrs::File:
      scopeName = "FileAttributes";
      તોડો;
    સ્થિતિ ELFAttrs::Section:
      scopeName = "SectionAttributes";
      indexName = "Sections";
      parseIndexList(indices);
      તોડો;
    સ્થિતિ ELFAttrs::પ્રતીક:
      scopeName = "SymbolAttributes";
      indexName = "Symbols";
      parseIndexList(indices);
      તોડો;
    મૂળભૂત:
      પાછા createStringError(errc::invalid_argument,
                               "unrecognized tag 0x" + Twine::utohexstr(tag) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    જો (sw) {
      DictScope scope(*sw, scopeName);
      જો (!indices.empty())
        sw->printList(indexName, indices);
      જો (Error e = parseAttributeList(size - 5))
        પાછા e;
    } નહીંતર જો (Error e = parseAttributeList(size - 5))
      પાછા e;
  }
  પાછા Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  unsigned section_સંખ્યા = 0;
  de = DataExtractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  સંરચના ClearCursorError {
    DataExtractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t formatVersion = de.getU8(cursor);
  જો (formatVersion != ELFAttrs::Format_Version)
    પાછા createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(formatVersion));

  જ્યારે (!de.eof(cursor)) {
    uint32_t sectionLength = de.getU32(cursor);
    જો (!cursor)
      પાછા cursor.takeError();

    જો (sw) {
      sw->start_લીટી() << "Section " << ++section_સંખ્યા << " {\n";
      sw->indent();
    }

    જો (sectionLength < 4 || cursor.tell() - 4 + sectionLength > section.size())
      પાછા createStringError(errc::invalid_argument,
                               "invalid section length " +
                                   Twine(sectionLength) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    જો (Error e = parseSubsection(sectionLength))
      પાછા e;
    જો (sw) {
      sw->unindent();
      sw->start_લીટી() << "}\n";
    }
  }

  પાછા cursor.takeError();
}
