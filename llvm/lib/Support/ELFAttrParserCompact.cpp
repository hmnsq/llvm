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

استفاده نطاق llvm;
استفاده نطاق llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    ثابت حرف *name, بیعلامت برچسب_2, ArrayRef<ثابت حرف *> رشتهها) {
  uint64_t مقدار_2 = de.getULEB128(cursor);
  إذا (مقدار_2 >= رشتهها.اندازه_2()) {
    printAttribute(برچسب_2, مقدار_2, "");
    أرجع createStringError(errc::invalid_آرگومان,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(مقدار_2));
  }
  printAttribute(برچسب_2, مقدار_2, رشتهها[مقدار_2]);
  أرجع Error::success();
}

Error ELFCompactAttrParser::integerAttribute(بیعلامت برچسب_2) {
  StringRef برچسب_Name =
      ELFAttrs::attrTypeAsString(برچسب_2, برچسب_مقصد_String_Map, /*hasTagPrefix=*/خطأ);
  uint64_t مقدار_2 = de.getULEB128(cursor);
  صفات_2.درج(std::make_pair(برچسب_2, مقدار_2));

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_عدد("Tag", برچسب_2);
    إذا (!برچسب_Name.خالی())
      sw->printString("TagName", برچسب_Name);
    sw->print_عدد("Value", مقدار_2);
  }
  أرجع Error::success();
}

Error ELFCompactAttrParser::stringAttribute(بیعلامت برچسب_2) {
  StringRef برچسب_Name =
      ELFAttrs::attrTypeAsString(برچسب_2, برچسب_مقصد_String_Map, /*hasTagPrefix=*/خطأ);
  StringRef desc = de.getCStrRef(cursor);
  مجموعه_Attribute_String(برچسب_2, desc);

  إذا (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_عدد("Tag", برچسب_2);
    إذا (!برچسب_Name.خالی())
      sw->printString("TagName", برچسب_Name);
    sw->printString("Value", desc);
  }
  أرجع Error::success();
}

فراغ ELFCompactAttrParser::printAttribute(بیعلامت برچسب_2, بیعلامت مقدار_2,
                                          StringRef مقدار_Desc) {
  صفات_2.درج(std::make_pair(برچسب_2, مقدار_2));

  إذا (sw) {
    StringRef برچسب_Name = ELFAttrs::attrTypeAsString(برچسب_2, برچسب_مقصد_String_Map,
                                                   /*hasTagPrefix=*/خطأ);
    DictScope as(*sw, "Attribute");
    sw->print_عدد("Tag", برچسب_2);
    sw->print_عدد("Value", مقدار_2);
    إذا (!برچسب_Name.خالی())
      sw->printString("TagName", برچسب_Name);
    إذا (!مقدار_Desc.خالی())
      sw->printString("Description", مقدار_Desc);
  }
}

فراغ ELFCompactAttrParser::parse_شاخص_List(Small_وکتور_Impl<uint8_t> &شاخص_List) {
  لكل (;;) {
    uint64_t مقدار_2 = de.getULEB128(cursor);
    إذا (!cursor || !مقدار_2)
      اكسر;
    شاخص_List.افزودن_پشت(مقدار_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t درازی_2) {
  uint64_t pos;
  uint64_t انتها = cursor.tell() + درازی_2;
  طالما ((pos = cursor.tell()) < انتها) {
    uint64_t برچسب_2 = de.getULEB128(cursor);
    منطقي handled;
    إذا (Error e = handler(برچسب_2, handled))
      أرجع e;

    إذا (!handled) {
      إذا (برچسب_2 < 32) {
        أرجع createStringError(errc::invalid_آرگومان,
                                 "invalid tag 0x" + Twine::utohexstr(برچسب_2) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      إذا (برچسب_2 % 2 == 0) {
        إذا (Error e = integerAttribute(برچسب_2))
          أرجع e;
      } وإلا {
        إذا (Error e = stringAttribute(برچسب_2))
          أرجع e;
      }
    }
  }
  أرجع Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t درازی_2) {
  uint64_t انتها = cursor.tell() - حجم(درازی_2) + درازی_2;
  StringRef vendorName = de.getCStrRef(cursor);
  إذا (sw) {
    sw->print_عدد("SectionLength", درازی_2);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  إذا (vendorName.پایینی() != vendor) {
    cursor.seek(انتها);
    أرجع Error::success();
  }

  طالما (cursor.tell() < انتها) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t برچسب_2 = de.getU8(cursor);
    uint32_t اندازه_2 = de.getU32(cursor);
    إذا (!cursor)
      أرجع cursor.takeError();

    إذا (sw) {
      constexpr EnumStringDef<بیعلامت> برچسب_Name_Defs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::نماد},
      };
      ساكن constexpr خودکار برچسب_نامها = BUILD_ENUM_رشتهها(برچسب_Name_Defs);

      sw->printEnum("Tag", برچسب_2, Enum_رشتهها(برچسب_نامها));
      sw->print_عدد("Size", اندازه_2);
    }
    إذا (اندازه_2 < 5)
      أرجع createStringError(errc::invalid_آرگومان,
                               "invalid attribute size " + Twine(اندازه_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, شاخص_Name;
    Small_وکتور<uint8_t, 8> indices;
    بدل (برچسب_2) {
    حالة ELFAttrs::File:
      scopeName = "FileAttributes";
      اكسر;
    حالة ELFAttrs::Section:
      scopeName = "SectionAttributes";
      شاخص_Name = "Sections";
      parse_شاخص_List(indices);
      اكسر;
    حالة ELFAttrs::نماد:
      scopeName = "SymbolAttributes";
      شاخص_Name = "Symbols";
      parse_شاخص_List(indices);
      اكسر;
    افتراضي:
      أرجع createStringError(errc::invalid_آرگومان,
                               "unrecognized tag 0x" + Twine::utohexstr(برچسب_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    إذا (sw) {
      DictScope scope(*sw, scopeName);
      إذا (!indices.خالی())
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
  بیعلامت section_عدد = 0;
  de = داده_Extractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  بنية ClearCursorError {
    داده_Extractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t format_نسخه = de.getU8(cursor);
  إذا (format_نسخه != ELFAttrs::Format_نسخه)
    أرجع createStringError(errc::invalid_آرگومان,
                             "unrecognized format-version: 0x" +
                                 utohexstr(format_نسخه));

  طالما (!de.eof(cursor)) {
    uint32_t section_درازی = de.getU32(cursor);
    إذا (!cursor)
      أرجع cursor.takeError();

    إذا (sw) {
      sw->شروع_سطر() << "Section " << ++section_عدد << " {\n";
      sw->indent();
    }

    إذا (section_درازی < 4 || cursor.tell() - 4 + section_درازی > section.اندازه_2())
      أرجع createStringError(errc::invalid_آرگومان,
                               "invalid section length " +
                                   Twine(section_درازی) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    إذا (Error e = parseSubsection(section_درازی))
      أرجع e;
    إذا (sw) {
      sw->unindent();
      sw->شروع_سطر() << "}\n";
    }
  }

  أرجع cursor.takeError();
}
