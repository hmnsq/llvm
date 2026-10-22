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

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    ध्रुव वर्ण *name, अचिह्नित चिह्नकम्_2, ArrayRef<ध्रुव वर्ण *> वाक्यानि) {
  uint64_t मानम्_2 = de.getULEB128(cursor);
  यदि (मानम्_2 >= वाक्यानि.आकार_2()) {
    printAttribute(चिह्नकम्_2, मानम्_2, "");
    प्रत्यागम createStringError(errc::invalid_तर्कः,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(मानम्_2));
  }
  printAttribute(चिह्नकम्_2, मानम्_2, वाक्यानि[मानम्_2]);
  प्रत्यागम Error::success();
}

Error ELFCompactAttrParser::integerAttribute(अचिह्नित चिह्नकम्_2) {
  StringRef चिह्नकम्_Name =
      ELFAttrs::attrTypeAsString(चिह्नकम्_2, चिह्नकम्_गन्तव्यम्_String_Map, /*hasTagPrefix=*/असत्य);
  uint64_t मानम्_2 = de.getULEB128(cursor);
  गुणाः_2.निवेशय(std::make_pair(चिह्नकम्_2, मानम्_2));

  यदि (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_सङ्ख्या("Tag", चिह्नकम्_2);
    यदि (!चिह्नकम्_Name.रिक्तानि())
      sw->printString("TagName", चिह्नकम्_Name);
    sw->print_सङ्ख्या("Value", मानम्_2);
  }
  प्रत्यागम Error::success();
}

Error ELFCompactAttrParser::stringAttribute(अचिह्नित चिह्नकम्_2) {
  StringRef चिह्नकम्_Name =
      ELFAttrs::attrTypeAsString(चिह्नकम्_2, चिह्नकम्_गन्तव्यम्_String_Map, /*hasTagPrefix=*/असत्य);
  StringRef desc = de.getCStrRef(cursor);
  समुच्चयः_Attribute_String(चिह्नकम्_2, desc);

  यदि (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_सङ्ख्या("Tag", चिह्नकम्_2);
    यदि (!चिह्नकम्_Name.रिक्तानि())
      sw->printString("TagName", चिह्नकम्_Name);
    sw->printString("Value", desc);
  }
  प्रत्यागम Error::success();
}

शून्य ELFCompactAttrParser::printAttribute(अचिह्नित चिह्नकम्_2, अचिह्नित मानम्_2,
                                          StringRef मानम्_Desc) {
  गुणाः_2.निवेशय(std::make_pair(चिह्नकम्_2, मानम्_2));

  यदि (sw) {
    StringRef चिह्नकम्_Name = ELFAttrs::attrTypeAsString(चिह्नकम्_2, चिह्नकम्_गन्तव्यम्_String_Map,
                                                   /*hasTagPrefix=*/असत्य);
    DictScope as(*sw, "Attribute");
    sw->print_सङ्ख्या("Tag", चिह्नकम्_2);
    sw->print_सङ्ख्या("Value", मानम्_2);
    यदि (!चिह्नकम्_Name.रिक्तानि())
      sw->printString("TagName", चिह्नकम्_Name);
    यदि (!मानम्_Desc.रिक्तानि())
      sw->printString("Description", मानम्_Desc);
  }
}

शून्य ELFCompactAttrParser::parse_सूचकांक_List(Small_सदिशम्_Impl<uint8_t> &सूचकांक_List) {
  प्रति (;;) {
    uint64_t मानम्_2 = de.getULEB128(cursor);
    यदि (!cursor || !मानम्_2)
      भङ्ग;
    सूचकांक_List.प्रक्षिप_पृष्ठभागः(मानम्_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t दीर्घता_2) {
  uint64_t pos;
  uint64_t अन्तः = cursor.tell() + दीर्घता_2;
  यावत् ((pos = cursor.tell()) < अन्तः) {
    uint64_t चिह्नकम्_2 = de.getULEB128(cursor);
    तार्किक handled;
    यदि (Error e = handler(चिह्नकम्_2, handled))
      प्रत्यागम e;

    यदि (!handled) {
      यदि (चिह्नकम्_2 < 32) {
        प्रत्यागम createStringError(errc::invalid_तर्कः,
                                 "invalid tag 0x" + Twine::utohexstr(चिह्नकम्_2) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      यदि (चिह्नकम्_2 % 2 == 0) {
        यदि (Error e = integerAttribute(चिह्नकम्_2))
          प्रत्यागम e;
      } अन्यथा {
        यदि (Error e = stringAttribute(चिह्नकम्_2))
          प्रत्यागम e;
      }
    }
  }
  प्रत्यागम Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t दीर्घता_2) {
  uint64_t अन्तः = cursor.tell() - परिमाण(दीर्घता_2) + दीर्घता_2;
  StringRef vendorName = de.getCStrRef(cursor);
  यदि (sw) {
    sw->print_सङ्ख्या("SectionLength", दीर्घता_2);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  यदि (vendorName.अधस्तनम्() != vendor) {
    cursor.seek(अन्तः);
    प्रत्यागम Error::success();
  }

  यावत् (cursor.tell() < अन्तः) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t चिह्नकम्_2 = de.getU8(cursor);
    uint32_t आकार_2 = de.getU32(cursor);
    यदि (!cursor)
      प्रत्यागम cursor.takeError();

    यदि (sw) {
      constexpr EnumStringDef<अचिह्नित> चिह्नकम्_Name_Defs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::चिह्नम्},
      };
      स्थिर constexpr स्वचालित चिह्नकम्_नामानि = BUILD_ENUM_वाक्यानि(चिह्नकम्_Name_Defs);

      sw->printEnum("Tag", चिह्नकम्_2, Enum_वाक्यानि(चिह्नकम्_नामानि));
      sw->print_सङ्ख्या("Size", आकार_2);
    }
    यदि (आकार_2 < 5)
      प्रत्यागम createStringError(errc::invalid_तर्कः,
                               "invalid attribute size " + Twine(आकार_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, सूचकांक_Name;
    Small_सदिशम्<uint8_t, 8> indices;
    विकल्प (चिह्नकम्_2) {
    पक्ष ELFAttrs::File:
      scopeName = "FileAttributes";
      भङ्ग;
    पक्ष ELFAttrs::Section:
      scopeName = "SectionAttributes";
      सूचकांक_Name = "Sections";
      parse_सूचकांक_List(indices);
      भङ्ग;
    पक्ष ELFAttrs::चिह्नम्:
      scopeName = "SymbolAttributes";
      सूचकांक_Name = "Symbols";
      parse_सूचकांक_List(indices);
      भङ्ग;
    स्वभाव:
      प्रत्यागम createStringError(errc::invalid_तर्कः,
                               "unrecognized tag 0x" + Twine::utohexstr(चिह्नकम्_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    यदि (sw) {
      DictScope scope(*sw, scopeName);
      यदि (!indices.रिक्तानि())
        sw->printList(सूचकांक_Name, indices);
      यदि (Error e = parseAttributeList(आकार_2 - 5))
        प्रत्यागम e;
    } अन्यथा यदि (Error e = parseAttributeList(आकार_2 - 5))
      प्रत्यागम e;
  }
  प्रत्यागम Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  अचिह्नित section_सङ्ख्या = 0;
  de = दत्त_Extractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  संरचना ClearCursorError {
    दत्त_Extractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t format_संस्करणम् = de.getU8(cursor);
  यदि (format_संस्करणम् != ELFAttrs::Format_संस्करणम्)
    प्रत्यागम createStringError(errc::invalid_तर्कः,
                             "unrecognized format-version: 0x" +
                                 utohexstr(format_संस्करणम्));

  यावत् (!de.eof(cursor)) {
    uint32_t section_दीर्घता = de.getU32(cursor);
    यदि (!cursor)
      प्रत्यागम cursor.takeError();

    यदि (sw) {
      sw->start_पङ्क्तिः() << "Section " << ++section_सङ्ख्या << " {\n";
      sw->indent();
    }

    यदि (section_दीर्घता < 4 || cursor.tell() - 4 + section_दीर्घता > section.आकार_2())
      प्रत्यागम createStringError(errc::invalid_तर्कः,
                               "invalid section length " +
                                   Twine(section_दीर्घता) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    यदि (Error e = parseSubsection(section_दीर्घता))
      प्रत्यागम e;
    यदि (sw) {
      sw->unindent();
      sw->start_पङ्क्तिः() << "}\n";
    }
  }

  प्रत्यागम cursor.takeError();
}
