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

استعمال نطاق llvm;
استعمال نطاق llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    ثابت حرف *name, بنانشان ٽيگ_2, ArrayRef<ثابت حرف *> لڙيون) {
  uint64_t قدر_2 = de.getULEB128(cursor);
  إذا (قدر_2 >= لڙيون.سائيز_2()) {
    printAttribute(ٽيگ_2, قدر_2, "");
    أرجع createStringError(errc::invalid_دليل,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(قدر_2));
  }
  printAttribute(ٽيگ_2, قدر_2, لڙيون[قدر_2]);
  أرجع Error::success();
}

Error ELFCompactAttrParser::integerAttribute(بنانشان ٽيگ_2) {
  StringRef ٽيگ_Name =
      ELFAttrs::attrTypeAsString(ٽيگ_2, ٽيگ_منزل_String_Map, /*hasTagPrefix=*/خطأ);
  uint64_t قدر_2 = de.getULEB128(cursor);
  خاصيتون_2.وجهو(std::make_pair(ٽيگ_2, قدر_2));

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_عدد("Tag", ٽيگ_2);
    إذا (!ٽيگ_Name.خالي_جايون())
      sw->printString("TagName", ٽيگ_Name);
    sw->print_عدد("Value", قدر_2);
  }
  أرجع Error::success();
}

Error ELFCompactAttrParser::stringAttribute(بنانشان ٽيگ_2) {
  StringRef ٽيگ_Name =
      ELFAttrs::attrTypeAsString(ٽيگ_2, ٽيگ_منزل_String_Map, /*hasTagPrefix=*/خطأ);
  StringRef desc = de.getCStrRef(cursor);
  سيٽ_Attribute_String(ٽيگ_2, desc);

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_عدد("Tag", ٽيگ_2);
    إذا (!ٽيگ_Name.خالي_جايون())
      sw->printString("TagName", ٽيگ_Name);
    sw->printString("Value", desc);
  }
  أرجع Error::success();
}

فراغ ELFCompactAttrParser::printAttribute(بنانشان ٽيگ_2, بنانشان قدر_2,
                                          StringRef قدر_Desc) {
  خاصيتون_2.وجهو(std::make_pair(ٽيگ_2, قدر_2));

  إذا (sw) {
    StringRef ٽيگ_Name = ELFAttrs::attrTypeAsString(ٽيگ_2, ٽيگ_منزل_String_Map,
                                                   /*hasTagPrefix=*/خطأ);
    DictScope as(*sw, "Attribute");
    sw->print_عدد("Tag", ٽيگ_2);
    sw->print_عدد("Value", قدر_2);
    إذا (!ٽيگ_Name.خالي_جايون())
      sw->printString("TagName", ٽيگ_Name);
    إذا (!قدر_Desc.خالي_جايون())
      sw->printString("Description", قدر_Desc);
  }
}

فراغ ELFCompactAttrParser::parse_اشاريو_List(Small_ويڪٽر_Impl<uint8_t> &اشاريو_List) {
  لكل (;;) {
    uint64_t قدر_2 = de.getULEB128(cursor);
    إذا (!cursor || !قدر_2)
      اكسر;
    اشاريو_List.ڌڪو_پٺ(قدر_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t ڊيگهائي_2) {
  uint64_t pos;
  uint64_t پڇاڙي = cursor.tell() + ڊيگهائي_2;
  طالما ((pos = cursor.tell()) < پڇاڙي) {
    uint64_t ٽيگ_2 = de.getULEB128(cursor);
    منطقي handled;
    إذا (Error e = handler(ٽيگ_2, handled))
      أرجع e;

    إذا (!handled) {
      إذا (ٽيگ_2 < 32) {
        أرجع createStringError(errc::invalid_دليل,
                                 "invalid tag 0x" + Twine::utohexstr(ٽيگ_2) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      إذا (ٽيگ_2 % 2 == 0) {
        إذا (Error e = integerAttribute(ٽيگ_2))
          أرجع e;
      } وإلا {
        إذا (Error e = stringAttribute(ٽيگ_2))
          أرجع e;
      }
    }
  }
  أرجع Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t ڊيگهائي_2) {
  uint64_t پڇاڙي = cursor.tell() - حجم(ڊيگهائي_2) + ڊيگهائي_2;
  StringRef vendorName = de.getCStrRef(cursor);
  إذا (sw) {
    sw->print_عدد("SectionLength", ڊيگهائي_2);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  إذا (vendorName.هيٺاهون() != vendor) {
    cursor.seek(پڇاڙي);
    أرجع Error::success();
  }

  طالما (cursor.tell() < پڇاڙي) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t ٽيگ_2 = de.getU8(cursor);
    uint32_t سائيز_2 = de.getU32(cursor);
    إذا (!cursor)
      أرجع cursor.takeError();

    إذا (sw) {
      constexpr EnumStringDef<بنانشان> ٽيگ_Name_Defs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::نشان},
      };
      ساكن constexpr خودڪار ٽيگ_نالا = BUILD_ENUM_لڙيون(ٽيگ_Name_Defs);

      sw->printEnum("Tag", ٽيگ_2, Enum_لڙيون(ٽيگ_نالا));
      sw->print_عدد("Size", سائيز_2);
    }
    إذا (سائيز_2 < 5)
      أرجع createStringError(errc::invalid_دليل,
                               "invalid attribute size " + Twine(سائيز_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, اشاريو_Name;
    Small_ويڪٽر<uint8_t, 8> indices;
    بدل (ٽيگ_2) {
    حالة ELFAttrs::File:
      scopeName = "FileAttributes";
      اكسر;
    حالة ELFAttrs::Section:
      scopeName = "SectionAttributes";
      اشاريو_Name = "Sections";
      parse_اشاريو_List(indices);
      اكسر;
    حالة ELFAttrs::نشان:
      scopeName = "SymbolAttributes";
      اشاريو_Name = "Symbols";
      parse_اشاريو_List(indices);
      اكسر;
    افتراضي:
      أرجع createStringError(errc::invalid_دليل,
                               "unrecognized tag 0x" + Twine::utohexstr(ٽيگ_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    إذا (sw) {
      DictScope scope(*sw, scopeName);
      إذا (!indices.خالي_جايون())
        sw->printList(اشاريو_Name, indices);
      إذا (Error e = parseAttributeList(سائيز_2 - 5))
        أرجع e;
    } وإلا إذا (Error e = parseAttributeList(سائيز_2 - 5))
      أرجع e;
  }
  أرجع Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  بنانشان section_عدد = 0;
  de = معلومات_Extractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  بنية ClearCursorError {
    معلومات_Extractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t format_نسخو = de.getU8(cursor);
  إذا (format_نسخو != ELFAttrs::Format_نسخو)
    أرجع createStringError(errc::invalid_دليل,
                             "unrecognized format-version: 0x" +
                                 utohexstr(format_نسخو));

  طالما (!de.eof(cursor)) {
    uint32_t section_ڊيگهائي = de.getU32(cursor);
    إذا (!cursor)
      أرجع cursor.takeError();

    إذا (sw) {
      sw->شروعات_سٽ() << "Section " << ++section_عدد << " {\n";
      sw->indent();
    }

    إذا (section_ڊيگهائي < 4 || cursor.tell() - 4 + section_ڊيگهائي > section.سائيز_2())
      أرجع createStringError(errc::invalid_دليل,
                               "invalid section length " +
                                   Twine(section_ڊيگهائي) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    إذا (Error e = parseSubsection(section_ڊيگهائي))
      أرجع e;
    إذا (sw) {
      sw->unindent();
      sw->شروعات_سٽ() << "}\n";
    }
  }

  أرجع cursor.takeError();
}
