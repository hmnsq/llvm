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

استعمال نامخلا llvm;
استعمال نامخلا llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    ثابت حرف *name, غیرعلامتی لیبل_2, ArrayRef<ثابت حرف *> لڑیاں) {
  uint64_t قدر_2 = de.getULEB128(cursor);
  اگر (قدر_2 >= لڑیاں.سائز_2()) {
    printAttribute(لیبل_2, قدر_2, "");
    واپس createStringError(errc::invalid_دلیل,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(قدر_2));
  }
  printAttribute(لیبل_2, قدر_2, لڑیاں[قدر_2]);
  واپس Error::success();
}

Error ELFCompactAttrParser::integerAttribute(غیرعلامتی لیبل_2) {
  StringRef لیبل_Name =
      ELFAttrs::attrTypeAsString(لیبل_2, لیبل_مقصد_String_Map, /*hasTagPrefix=*/جھوٹ);
  uint64_t قدر_2 = de.getULEB128(cursor);
  اوصاف_2.داخل_کرو(std::make_pair(لیبل_2, قدر_2));

  اگر (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_نمبر("Tag", لیبل_2);
    اگر (!لیبل_Name.خالی_خانے())
      sw->printString("TagName", لیبل_Name);
    sw->print_نمبر("Value", قدر_2);
  }
  واپس Error::success();
}

Error ELFCompactAttrParser::stringAttribute(غیرعلامتی لیبل_2) {
  StringRef لیبل_Name =
      ELFAttrs::attrTypeAsString(لیبل_2, لیبل_مقصد_String_Map, /*hasTagPrefix=*/جھوٹ);
  StringRef desc = de.getCStrRef(cursor);
  سیٹ_Attribute_String(لیبل_2, desc);

  اگر (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_نمبر("Tag", لیبل_2);
    اگر (!لیبل_Name.خالی_خانے())
      sw->printString("TagName", لیبل_Name);
    sw->printString("Value", desc);
  }
  واپس Error::success();
}

خالی ELFCompactAttrParser::printAttribute(غیرعلامتی لیبل_2, غیرعلامتی قدر_2,
                                          StringRef قدر_Desc) {
  اوصاف_2.داخل_کرو(std::make_pair(لیبل_2, قدر_2));

  اگر (sw) {
    StringRef لیبل_Name = ELFAttrs::attrTypeAsString(لیبل_2, لیبل_مقصد_String_Map,
                                                   /*hasTagPrefix=*/جھوٹ);
    DictScope as(*sw, "Attribute");
    sw->print_نمبر("Tag", لیبل_2);
    sw->print_نمبر("Value", قدر_2);
    اگر (!لیبل_Name.خالی_خانے())
      sw->printString("TagName", لیبل_Name);
    اگر (!قدر_Desc.خالی_خانے())
      sw->printString("Description", قدر_Desc);
  }
}

خالی ELFCompactAttrParser::parse_اشاریہ_List(Small_سمتیہ_Impl<uint8_t> &اشاریہ_List) {
  برائے (;;) {
    uint64_t قدر_2 = de.getULEB128(cursor);
    اگر (!cursor || !قدر_2)
      توڑو;
    اشاریہ_List.ڈالو_عقب(قدر_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t لمبائی_2) {
  uint64_t pos;
  uint64_t انتہا = cursor.tell() + لمبائی_2;
  جب ((pos = cursor.tell()) < انتہا) {
    uint64_t لیبل_2 = de.getULEB128(cursor);
    منطقی handled;
    اگر (Error e = handler(لیبل_2, handled))
      واپس e;

    اگر (!handled) {
      اگر (لیبل_2 < 32) {
        واپس createStringError(errc::invalid_دلیل,
                                 "invalid tag 0x" + Twine::utohexstr(لیبل_2) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      اگر (لیبل_2 % 2 == 0) {
        اگر (Error e = integerAttribute(لیبل_2))
          واپس e;
      } ورنہ {
        اگر (Error e = stringAttribute(لیبل_2))
          واپس e;
      }
    }
  }
  واپس Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t لمبائی_2) {
  uint64_t انتہا = cursor.tell() - حجم(لمبائی_2) + لمبائی_2;
  StringRef vendorName = de.getCStrRef(cursor);
  اگر (sw) {
    sw->print_نمبر("SectionLength", لمبائی_2);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  اگر (vendorName.زیریں() != vendor) {
    cursor.seek(انتہا);
    واپس Error::success();
  }

  جب (cursor.tell() < انتہا) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t لیبل_2 = de.getU8(cursor);
    uint32_t سائز_2 = de.getU32(cursor);
    اگر (!cursor)
      واپس cursor.takeError();

    اگر (sw) {
      constexpr EnumStringDef<غیرعلامتی> لیبل_Name_Defs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::علامت},
      };
      ساکن constexpr خودکار لیبل_نام = BUILD_ENUM_لڑیاں(لیبل_Name_Defs);

      sw->printEnum("Tag", لیبل_2, Enum_لڑیاں(لیبل_نام));
      sw->print_نمبر("Size", سائز_2);
    }
    اگر (سائز_2 < 5)
      واپس createStringError(errc::invalid_دلیل,
                               "invalid attribute size " + Twine(سائز_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, اشاریہ_Name;
    Small_سمتیہ<uint8_t, 8> indices;
    انتخاب (لیبل_2) {
    حالت ELFAttrs::File:
      scopeName = "FileAttributes";
      توڑو;
    حالت ELFAttrs::Section:
      scopeName = "SectionAttributes";
      اشاریہ_Name = "Sections";
      parse_اشاریہ_List(indices);
      توڑو;
    حالت ELFAttrs::علامت:
      scopeName = "SymbolAttributes";
      اشاریہ_Name = "Symbols";
      parse_اشاریہ_List(indices);
      توڑو;
    طےشدہ:
      واپس createStringError(errc::invalid_دلیل,
                               "unrecognized tag 0x" + Twine::utohexstr(لیبل_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    اگر (sw) {
      DictScope scope(*sw, scopeName);
      اگر (!indices.خالی_خانے())
        sw->printList(اشاریہ_Name, indices);
      اگر (Error e = parseAttributeList(سائز_2 - 5))
        واپس e;
    } ورنہ اگر (Error e = parseAttributeList(سائز_2 - 5))
      واپس e;
  }
  واپس Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  غیرعلامتی section_نمبر = 0;
  de = ڈیٹا_Extractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  ساخت ClearCursorError {
    ڈیٹا_Extractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t format_نسخہ = de.getU8(cursor);
  اگر (format_نسخہ != ELFAttrs::Format_نسخہ)
    واپس createStringError(errc::invalid_دلیل,
                             "unrecognized format-version: 0x" +
                                 utohexstr(format_نسخہ));

  جب (!de.eof(cursor)) {
    uint32_t section_لمبائی = de.getU32(cursor);
    اگر (!cursor)
      واپس cursor.takeError();

    اگر (sw) {
      sw->آغاز_سطر() << "Section " << ++section_نمبر << " {\n";
      sw->indent();
    }

    اگر (section_لمبائی < 4 || cursor.tell() - 4 + section_لمبائی > section.سائز_2())
      واپس createStringError(errc::invalid_دلیل,
                               "invalid section length " +
                                   Twine(section_لمبائی) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    اگر (Error e = parseSubsection(section_لمبائی))
      واپس e;
    اگر (sw) {
      sw->unindent();
      sw->آغاز_سطر() << "}\n";
    }
  }

  واپس cursor.takeError();
}
