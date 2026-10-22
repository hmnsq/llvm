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

ورتوں نطاق llvm;
ورتوں نطاق llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    ثابت حرف *name, بنانشانی ٹیگ_2, ArrayRef<ثابت حرف *> لڑیاں) {
  uint64_t قدر_2 = de.getULEB128(cursor);
  إذا (قدر_2 >= لڑیاں.ناپ_2()) {
    printAttribute(ٹیگ_2, قدر_2, "");
    أرجع createStringError(errc::invalid_دلیل,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(قدر_2));
  }
  printAttribute(ٹیگ_2, قدر_2, لڑیاں[قدر_2]);
  أرجع Error::success();
}

Error ELFCompactAttrParser::integerAttribute(بنانشانی ٹیگ_2) {
  StringRef ٹیگ_Name =
      ELFAttrs::attrTypeAsString(ٹیگ_2, ٹیگ_منزل_String_Map, /*hasTagPrefix=*/خطأ);
  uint64_t قدر_2 = de.getULEB128(cursor);
  گن_2.داخل_کرو(std::make_pair(ٹیگ_2, قدر_2));

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_نمبر("Tag", ٹیگ_2);
    إذا (!ٹیگ_Name.خالی_تھاواں())
      sw->printString("TagName", ٹیگ_Name);
    sw->print_نمبر("Value", قدر_2);
  }
  أرجع Error::success();
}

Error ELFCompactAttrParser::stringAttribute(بنانشانی ٹیگ_2) {
  StringRef ٹیگ_Name =
      ELFAttrs::attrTypeAsString(ٹیگ_2, ٹیگ_منزل_String_Map, /*hasTagPrefix=*/خطأ);
  StringRef desc = de.getCStrRef(cursor);
  سیٹ_Attribute_String(ٹیگ_2, desc);

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_نمبر("Tag", ٹیگ_2);
    إذا (!ٹیگ_Name.خالی_تھاواں())
      sw->printString("TagName", ٹیگ_Name);
    sw->printString("Value", desc);
  }
  أرجع Error::success();
}

فراغ ELFCompactAttrParser::printAttribute(بنانشانی ٹیگ_2, بنانشانی قدر_2,
                                          StringRef قدر_Desc) {
  گن_2.داخل_کرو(std::make_pair(ٹیگ_2, قدر_2));

  إذا (sw) {
    StringRef ٹیگ_Name = ELFAttrs::attrTypeAsString(ٹیگ_2, ٹیگ_منزل_String_Map,
                                                   /*hasTagPrefix=*/خطأ);
    DictScope as(*sw, "Attribute");
    sw->print_نمبر("Tag", ٹیگ_2);
    sw->print_نمبر("Value", قدر_2);
    إذا (!ٹیگ_Name.خالی_تھاواں())
      sw->printString("TagName", ٹیگ_Name);
    إذا (!قدر_Desc.خالی_تھاواں())
      sw->printString("Description", قدر_Desc);
  }
}

فراغ ELFCompactAttrParser::parse_اشاریہ_List(Small_سمتیہ_Impl<uint8_t> &اشاریہ_List) {
  لكل (;;) {
    uint64_t قدر_2 = de.getULEB128(cursor);
    إذا (!cursor || !قدر_2)
      اكسر;
    اشاریہ_List.ڈالو_پٹھ(قدر_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t لمائی_2) {
  uint64_t pos;
  uint64_t سرا = cursor.tell() + لمائی_2;
  طالما ((pos = cursor.tell()) < سرا) {
    uint64_t ٹیگ_2 = de.getULEB128(cursor);
    منطقي handled;
    إذا (Error e = handler(ٹیگ_2, handled))
      أرجع e;

    إذا (!handled) {
      إذا (ٹیگ_2 < 32) {
        أرجع createStringError(errc::invalid_دلیل,
                                 "invalid tag 0x" + Twine::utohexstr(ٹیگ_2) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      إذا (ٹیگ_2 % 2 == 0) {
        إذا (Error e = integerAttribute(ٹیگ_2))
          أرجع e;
      } وإلا {
        إذا (Error e = stringAttribute(ٹیگ_2))
          أرجع e;
      }
    }
  }
  أرجع Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t لمائی_2) {
  uint64_t سرا = cursor.tell() - حجم(لمائی_2) + لمائی_2;
  StringRef vendorName = de.getCStrRef(cursor);
  إذا (sw) {
    sw->print_نمبر("SectionLength", لمائی_2);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  إذا (vendorName.ہیٹھلا() != vendor) {
    cursor.seek(سرا);
    أرجع Error::success();
  }

  طالما (cursor.tell() < سرا) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t ٹیگ_2 = de.getU8(cursor);
    uint32_t ناپ_2 = de.getU32(cursor);
    إذا (!cursor)
      أرجع cursor.takeError();

    إذا (sw) {
      constexpr EnumStringDef<بنانشانی> ٹیگ_Name_Defs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::علامت},
      };
      ساكن constexpr خودکار ٹیگ_ناں = BUILD_ENUM_لڑیاں(ٹیگ_Name_Defs);

      sw->printEnum("Tag", ٹیگ_2, Enum_لڑیاں(ٹیگ_ناں));
      sw->print_نمبر("Size", ناپ_2);
    }
    إذا (ناپ_2 < 5)
      أرجع createStringError(errc::invalid_دلیل,
                               "invalid attribute size " + Twine(ناپ_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, اشاریہ_Name;
    Small_سمتیہ<uint8_t, 8> indices;
    بدل (ٹیگ_2) {
    حالة ELFAttrs::File:
      scopeName = "FileAttributes";
      اكسر;
    حالة ELFAttrs::Section:
      scopeName = "SectionAttributes";
      اشاریہ_Name = "Sections";
      parse_اشاریہ_List(indices);
      اكسر;
    حالة ELFAttrs::علامت:
      scopeName = "SymbolAttributes";
      اشاریہ_Name = "Symbols";
      parse_اشاریہ_List(indices);
      اكسر;
    افتراضي:
      أرجع createStringError(errc::invalid_دلیل,
                               "unrecognized tag 0x" + Twine::utohexstr(ٹیگ_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    إذا (sw) {
      DictScope scope(*sw, scopeName);
      إذا (!indices.خالی_تھاواں())
        sw->printList(اشاریہ_Name, indices);
      إذا (Error e = parseAttributeList(ناپ_2 - 5))
        أرجع e;
    } وإلا إذا (Error e = parseAttributeList(ناپ_2 - 5))
      أرجع e;
  }
  أرجع Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  بنانشانی section_نمبر = 0;
  de = انکڑے_Extractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  بنية ClearCursorError {
    انکڑے_Extractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t format_نسخہ = de.getU8(cursor);
  إذا (format_نسخہ != ELFAttrs::Format_نسخہ)
    أرجع createStringError(errc::invalid_دلیل,
                             "unrecognized format-version: 0x" +
                                 utohexstr(format_نسخہ));

  طالما (!de.eof(cursor)) {
    uint32_t section_لمائی = de.getU32(cursor);
    إذا (!cursor)
      أرجع cursor.takeError();

    إذا (sw) {
      sw->شروع_سطر() << "Section " << ++section_نمبر << " {\n";
      sw->indent();
    }

    إذا (section_لمائی < 4 || cursor.tell() - 4 + section_لمائی > section.ناپ_2())
      أرجع createStringError(errc::invalid_دلیل,
                               "invalid section length " +
                                   Twine(section_لمائی) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    إذا (Error e = parseSubsection(section_لمائی))
      أرجع e;
    إذا (sw) {
      sw->unindent();
      sw->شروع_سطر() << "}\n";
    }
  }

  أرجع cursor.takeError();
}
