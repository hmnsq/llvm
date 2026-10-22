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

#অন্তর্ভাব "llvm/Support/ELFAttrParserCompact.h"
#অন্তর্ভাব "llvm/ADT/Enum.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/Support/Errc.h"
#অন্তর্ভাব "llvm/Support/ScopedPrinter.h"

using নামাকাশ llvm;
using নামাকাশ llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    ধ্রুবক অক্ষর *name, unsigned tag, ArrayRef<ধ্রুবক অক্ষর *> strings) {
  uint64_t মান_2 = de.getULEB128(cursor);
  যদি (মান_2 >= strings.আকার_3()) {
    printAttribute(tag, মান_2, "");
    ফেরত createStringError(errc::invalid_argument,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(মান_2));
  }
  printAttribute(tag, মান_2, strings[মান_2]);
  ফেরত Error::success();
}

Error ELFCompactAttrParser::integerAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/মিথ্যা);
  uint64_t মান_2 = de.getULEB128(cursor);
  attributes.ঢোকাও(std::make_pair(tag, মান_2));

  যদি (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_সংখ্যা("Tag", tag);
    যদি (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->print_সংখ্যা("Value", মান_2);
  }
  ফেরত Error::success();
}

Error ELFCompactAttrParser::stringAttribute(unsigned tag) {
  StringRef tagName =
      ELFAttrs::attrTypeAsString(tag, tagToStringMap, /*hasTagPrefix=*/মিথ্যা);
  StringRef desc = de.getCStrRef(cursor);
  setAttributeString(tag, desc);

  যদি (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_সংখ্যা("Tag", tag);
    যদি (!tagName.empty())
      sw->printString("TagName", tagName);
    sw->printString("Value", desc);
  }
  ফেরত Error::success();
}

শূন্য ELFCompactAttrParser::printAttribute(unsigned tag, unsigned মান_2,
                                          StringRef মান_Desc) {
  attributes.ঢোকাও(std::make_pair(tag, মান_2));

  যদি (sw) {
    StringRef tagName = ELFAttrs::attrTypeAsString(tag, tagToStringMap,
                                                   /*hasTagPrefix=*/মিথ্যা);
    DictScope as(*sw, "Attribute");
    sw->print_সংখ্যা("Tag", tag);
    sw->print_সংখ্যা("Value", মান_2);
    যদি (!tagName.empty())
      sw->printString("TagName", tagName);
    যদি (!মান_Desc.empty())
      sw->printString("Description", মান_Desc);
  }
}

শূন্য ELFCompactAttrParser::parse_সূচক_List(SmallVectorImpl<uint8_t> &সূচক_List) {
  জন্য (;;) {
    uint64_t মান_2 = de.getULEB128(cursor);
    যদি (!cursor || !মান_2)
      বিরতি;
    সূচক_List.ঠেলো_back(মান_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t দৈর্ঘ্য_2) {
  uint64_t pos;
  uint64_t end = cursor.tell() + দৈর্ঘ্য_2;
  যখন ((pos = cursor.tell()) < end) {
    uint64_t tag = de.getULEB128(cursor);
    যৌক্তিক handled;
    যদি (Error e = handler(tag, handled))
      ফেরত e;

    যদি (!handled) {
      যদি (tag < 32) {
        ফেরত createStringError(errc::invalid_argument,
                                 "invalid tag 0x" + Twine::utohexstr(tag) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      যদি (tag % 2 == 0) {
        যদি (Error e = integerAttribute(tag))
          ফেরত e;
      } অন্যথা {
        যদি (Error e = stringAttribute(tag))
          ফেরত e;
      }
    }
  }
  ফেরত Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t দৈর্ঘ্য_2) {
  uint64_t end = cursor.tell() - আকার(দৈর্ঘ্য_2) + দৈর্ঘ্য_2;
  StringRef vendorName = de.getCStrRef(cursor);
  যদি (sw) {
    sw->print_সংখ্যা("SectionLength", দৈর্ঘ্য_2);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  যদি (vendorName.lower() != vendor) {
    cursor.seek(end);
    ফেরত Error::success();
  }

  যখন (cursor.tell() < end) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t tag = de.getU8(cursor);
    uint32_t আকার_3 = de.getU32(cursor);
    যদি (!cursor)
      ফেরত cursor.takeError();

    যদি (sw) {
      constexpr EnumStringDef<unsigned> TagNameDefs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::প্রতীক},
      };
      স্থির constexpr auto TagNames = BUILD_ENUM_STRINGS(TagNameDefs);

      sw->printEnum("Tag", tag, EnumStrings(TagNames));
      sw->print_সংখ্যা("Size", আকার_3);
    }
    যদি (আকার_3 < 5)
      ফেরত createStringError(errc::invalid_argument,
                               "invalid attribute size " + Twine(আকার_3) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, সূচক_Name;
    SmallVector<uint8_t, 8> indices;
    নির্বাচন (tag) {
    ক্ষেত্র ELFAttrs::File:
      scopeName = "FileAttributes";
      বিরতি;
    ক্ষেত্র ELFAttrs::Section:
      scopeName = "SectionAttributes";
      সূচক_Name = "Sections";
      parse_সূচক_List(indices);
      বিরতি;
    ক্ষেত্র ELFAttrs::প্রতীক:
      scopeName = "SymbolAttributes";
      সূচক_Name = "Symbols";
      parse_সূচক_List(indices);
      বিরতি;
    পূর্বনির্ধারিত:
      ফেরত createStringError(errc::invalid_argument,
                               "unrecognized tag 0x" + Twine::utohexstr(tag) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    যদি (sw) {
      DictScope scope(*sw, scopeName);
      যদি (!indices.empty())
        sw->printList(সূচক_Name, indices);
      যদি (Error e = parseAttributeList(আকার_3 - 5))
        ফেরত e;
    } অন্যথা যদি (Error e = parseAttributeList(আকার_3 - 5))
      ফেরত e;
  }
  ফেরত Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  unsigned section_সংখ্যা = 0;
  de = তথ্য_Extractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  কাঠামো ClearCursorError {
    তথ্য_Extractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t formatVersion = de.getU8(cursor);
  যদি (formatVersion != ELFAttrs::Format_Version)
    ফেরত createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(formatVersion));

  যখন (!de.eof(cursor)) {
    uint32_t section_দৈর্ঘ্য = de.getU32(cursor);
    যদি (!cursor)
      ফেরত cursor.takeError();

    যদি (sw) {
      sw->start_সারি() << "Section " << ++section_সংখ্যা << " {\n";
      sw->indent();
    }

    যদি (section_দৈর্ঘ্য < 4 || cursor.tell() - 4 + section_দৈর্ঘ্য > section.আকার_3())
      ফেরত createStringError(errc::invalid_argument,
                               "invalid section length " +
                                   Twine(section_দৈর্ঘ্য) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    যদি (Error e = parseSubsection(section_দৈর্ঘ্য))
      ফেরত e;
    যদি (sw) {
      sw->unindent();
      sw->start_সারি() << "}\n";
    }
  }

  ফেরত cursor.takeError();
}
