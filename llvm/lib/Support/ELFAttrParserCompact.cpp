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

کارول نطاق llvm;
کارول نطاق llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    ثابت حرف *name, بینښه نښان_2, ArrayRef<ثابت حرف *> مزي_ګانې) {
  uint64_t ارزښت_2 = de.getULEB128(cursor);
  إذا (ارزښت_2 >= مزي_ګانې.اندازه_2()) {
    printAttribute(نښان_2, ارزښت_2, "");
    أرجع createStringError(errc::invalid_دلیل,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(ارزښت_2));
  }
  printAttribute(نښان_2, ارزښت_2, مزي_ګانې[ارزښت_2]);
  أرجع Error::success();
}

Error ELFCompactAttrParser::integerAttribute(بینښه نښان_2) {
  StringRef نښان_Name =
      ELFAttrs::attrTypeAsString(نښان_2, نښان_منزل_String_Map, /*hasTagPrefix=*/خطأ);
  uint64_t ارزښت_2 = de.getULEB128(cursor);
  ځانګړتیاوې_2.ورننباسه(std::make_pair(نښان_2, ارزښت_2));

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_شمېره("Tag", نښان_2);
    إذا (!نښان_Name.تشه())
      sw->printString("TagName", نښان_Name);
    sw->print_شمېره("Value", ارزښت_2);
  }
  أرجع Error::success();
}

Error ELFCompactAttrParser::stringAttribute(بینښه نښان_2) {
  StringRef نښان_Name =
      ELFAttrs::attrTypeAsString(نښان_2, نښان_منزل_String_Map, /*hasTagPrefix=*/خطأ);
  StringRef desc = de.getCStrRef(cursor);
  سېټ_Attribute_String(نښان_2, desc);

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_شمېره("Tag", نښان_2);
    إذا (!نښان_Name.تشه())
      sw->printString("TagName", نښان_Name);
    sw->printString("Value", desc);
  }
  أرجع Error::success();
}

فراغ ELFCompactAttrParser::printAttribute(بینښه نښان_2, بینښه ارزښت_2,
                                          StringRef ارزښت_Desc) {
  ځانګړتیاوې_2.ورننباسه(std::make_pair(نښان_2, ارزښت_2));

  إذا (sw) {
    StringRef نښان_Name = ELFAttrs::attrTypeAsString(نښان_2, نښان_منزل_String_Map,
                                                   /*hasTagPrefix=*/خطأ);
    DictScope as(*sw, "Attribute");
    sw->print_شمېره("Tag", نښان_2);
    sw->print_شمېره("Value", ارزښت_2);
    إذا (!نښان_Name.تشه())
      sw->printString("TagName", نښان_Name);
    إذا (!ارزښت_Desc.تشه())
      sw->printString("Description", ارزښت_Desc);
  }
}

فراغ ELFCompactAttrParser::parse_شاخص_List(Small_ویکټور_Impl<uint8_t> &شاخص_List) {
  لكل (;;) {
    uint64_t ارزښت_2 = de.getULEB128(cursor);
    إذا (!cursor || !ارزښت_2)
      اكسر;
    شاخص_List.ټېل_شا(ارزښت_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t اوږدتیا_2) {
  uint64_t pos;
  uint64_t پای_ټکی = cursor.tell() + اوږدتیا_2;
  طالما ((pos = cursor.tell()) < پای_ټکی) {
    uint64_t نښان_2 = de.getULEB128(cursor);
    منطقي handled;
    إذا (Error e = handler(نښان_2, handled))
      أرجع e;

    إذا (!handled) {
      إذا (نښان_2 < 32) {
        أرجع createStringError(errc::invalid_دلیل,
                                 "invalid tag 0x" + Twine::utohexstr(نښان_2) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      إذا (نښان_2 % 2 == 0) {
        إذا (Error e = integerAttribute(نښان_2))
          أرجع e;
      } وإلا {
        إذا (Error e = stringAttribute(نښان_2))
          أرجع e;
      }
    }
  }
  أرجع Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t اوږدتیا_2) {
  uint64_t پای_ټکی = cursor.tell() - حجم(اوږدتیا_2) + اوږدتیا_2;
  StringRef vendorName = de.getCStrRef(cursor);
  إذا (sw) {
    sw->print_شمېره("SectionLength", اوږدتیا_2);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  إذا (vendorName.ښکتنی() != vendor) {
    cursor.seek(پای_ټکی);
    أرجع Error::success();
  }

  طالما (cursor.tell() < پای_ټکی) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t نښان_2 = de.getU8(cursor);
    uint32_t اندازه_2 = de.getU32(cursor);
    إذا (!cursor)
      أرجع cursor.takeError();

    إذا (sw) {
      constexpr EnumStringDef<بینښه> نښان_Name_Defs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::نښه},
      };
      ساكن constexpr خپلکاری نښان_نومونه = BUILD_ENUM_مزي_ګانې(نښان_Name_Defs);

      sw->printEnum("Tag", نښان_2, Enum_مزي_ګانې(نښان_نومونه));
      sw->print_شمېره("Size", اندازه_2);
    }
    إذا (اندازه_2 < 5)
      أرجع createStringError(errc::invalid_دلیل,
                               "invalid attribute size " + Twine(اندازه_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, شاخص_Name;
    Small_ویکټور<uint8_t, 8> indices;
    بدل (نښان_2) {
    حالة ELFAttrs::File:
      scopeName = "FileAttributes";
      اكسر;
    حالة ELFAttrs::Section:
      scopeName = "SectionAttributes";
      شاخص_Name = "Sections";
      parse_شاخص_List(indices);
      اكسر;
    حالة ELFAttrs::نښه:
      scopeName = "SymbolAttributes";
      شاخص_Name = "Symbols";
      parse_شاخص_List(indices);
      اكسر;
    افتراضي:
      أرجع createStringError(errc::invalid_دلیل,
                               "unrecognized tag 0x" + Twine::utohexstr(نښان_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    إذا (sw) {
      DictScope scope(*sw, scopeName);
      إذا (!indices.تشه())
        sw->printList(شاخص_Name, indices);
      إذا (Error e = parseAttributeList(اندازه_2 - 5))
        أرجع e;
    } وإلا إذا (Error e = parseAttributeList(اندازه_2 - 5))
      أرجع e;
  }
  أرجع Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  بینښه section_شمېره = 0;
  de = معلومات_Extractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  بنية ClearCursorError {
    معلومات_Extractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t format_نسخه = de.getU8(cursor);
  إذا (format_نسخه != ELFAttrs::Format_نسخه)
    أرجع createStringError(errc::invalid_دلیل,
                             "unrecognized format-version: 0x" +
                                 utohexstr(format_نسخه));

  طالما (!de.eof(cursor)) {
    uint32_t section_اوږدتیا = de.getU32(cursor);
    إذا (!cursor)
      أرجع cursor.takeError();

    إذا (sw) {
      sw->پیل_کرښه() << "Section " << ++section_شمېره << " {\n";
      sw->indent();
    }

    إذا (section_اوږدتیا < 4 || cursor.tell() - 4 + section_اوږدتیا > section.اندازه_2())
      أرجع createStringError(errc::invalid_دلیل,
                               "invalid section length " +
                                   Twine(section_اوږدتیا) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    إذا (Error e = parseSubsection(section_اوږدتیا))
      أرجع e;
    إذا (sw) {
      sw->unindent();
      sw->پیل_کرښه() << "}\n";
    }
  }

  أرجع cursor.takeError();
}
