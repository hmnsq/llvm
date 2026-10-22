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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ELFAttrParserCompact.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Enum.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ScopedPrinter.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    ਅਚਲ ਅੱਖਰ *name, ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ_2, ArrayRef<ਅਚਲ ਅੱਖਰ *> ਲੜੀਆਂ) {
  uint64_t ਮੁੱਲ_2 = de.getULEB128(cursor);
  ਜੇ (ਮੁੱਲ_2 >= ਲੜੀਆਂ.ਮਾਪ_2()) {
    printAttribute(ਟੈਗ_2, ਮੁੱਲ_2, "");
    ਵਾਪਸ createStringError(errc::invalid_ਦਲੀਲ,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(ਮੁੱਲ_2));
  }
  printAttribute(ਟੈਗ_2, ਮੁੱਲ_2, ਲੜੀਆਂ[ਮੁੱਲ_2]);
  ਵਾਪਸ Error::success();
}

Error ELFCompactAttrParser::integerAttribute(ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ_2) {
  StringRef ਟੈਗ_Name =
      ELFAttrs::attrTypeAsString(ਟੈਗ_2, ਟੈਗ_ਮੰਜ਼ਿਲ_String_Map, /*hasTagPrefix=*/ਝੂਠ);
  uint64_t ਮੁੱਲ_2 = de.getULEB128(cursor);
  ਗੁਣ_2.ਪਾਓ(std::make_pair(ਟੈਗ_2, ਮੁੱਲ_2));

  ਜੇ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_ਸੰਖਿਆ("Tag", ਟੈਗ_2);
    ਜੇ (!ਟੈਗ_Name.ਖਾਲੀ_ਥਾਵਾਂ())
      sw->printString("TagName", ਟੈਗ_Name);
    sw->print_ਸੰਖਿਆ("Value", ਮੁੱਲ_2);
  }
  ਵਾਪਸ Error::success();
}

Error ELFCompactAttrParser::stringAttribute(ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ_2) {
  StringRef ਟੈਗ_Name =
      ELFAttrs::attrTypeAsString(ਟੈਗ_2, ਟੈਗ_ਮੰਜ਼ਿਲ_String_Map, /*hasTagPrefix=*/ਝੂਠ);
  StringRef desc = de.getCStrRef(cursor);
  ਸੈੱਟ_Attribute_String(ਟੈਗ_2, desc);

  ਜੇ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_ਸੰਖਿਆ("Tag", ਟੈਗ_2);
    ਜੇ (!ਟੈਗ_Name.ਖਾਲੀ_ਥਾਵਾਂ())
      sw->printString("TagName", ਟੈਗ_Name);
    sw->printString("Value", desc);
  }
  ਵਾਪਸ Error::success();
}

ਖਾਲੀ ELFCompactAttrParser::printAttribute(ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ_2, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮੁੱਲ_2,
                                          StringRef ਮੁੱਲ_Desc) {
  ਗੁਣ_2.ਪਾਓ(std::make_pair(ਟੈਗ_2, ਮੁੱਲ_2));

  ਜੇ (sw) {
    StringRef ਟੈਗ_Name = ELFAttrs::attrTypeAsString(ਟੈਗ_2, ਟੈਗ_ਮੰਜ਼ਿਲ_String_Map,
                                                   /*hasTagPrefix=*/ਝੂਠ);
    DictScope as(*sw, "Attribute");
    sw->print_ਸੰਖਿਆ("Tag", ਟੈਗ_2);
    sw->print_ਸੰਖਿਆ("Value", ਮੁੱਲ_2);
    ਜੇ (!ਟੈਗ_Name.ਖਾਲੀ_ਥਾਵਾਂ())
      sw->printString("TagName", ਟੈਗ_Name);
    ਜੇ (!ਮੁੱਲ_Desc.ਖਾਲੀ_ਥਾਵਾਂ())
      sw->printString("Description", ਮੁੱਲ_Desc);
  }
}

ਖਾਲੀ ELFCompactAttrParser::parse_ਸੂਚਕਾਂਕ_List(Small_ਸਦਿਸ਼_Impl<uint8_t> &ਸੂਚਕਾਂਕ_List) {
  ਲਈ (;;) {
    uint64_t ਮੁੱਲ_2 = de.getULEB128(cursor);
    ਜੇ (!cursor || !ਮੁੱਲ_2)
      ਰੋਕੋ;
    ਸੂਚਕਾਂਕ_List.ਧੱਕੋ_ਪਿੱਠ(ਮੁੱਲ_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t ਲੰਮਾਈ_2) {
  uint64_t pos;
  uint64_t ਸਿਰਾ = cursor.tell() + ਲੰਮਾਈ_2;
  ਜਦੋਂ ((pos = cursor.tell()) < ਸਿਰਾ) {
    uint64_t ਟੈਗ_2 = de.getULEB128(cursor);
    ਤਾਰਕਿਕ handled;
    ਜੇ (Error e = handler(ਟੈਗ_2, handled))
      ਵਾਪਸ e;

    ਜੇ (!handled) {
      ਜੇ (ਟੈਗ_2 < 32) {
        ਵਾਪਸ createStringError(errc::invalid_ਦਲੀਲ,
                                 "invalid tag 0x" + Twine::utohexstr(ਟੈਗ_2) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      ਜੇ (ਟੈਗ_2 % 2 == 0) {
        ਜੇ (Error e = integerAttribute(ਟੈਗ_2))
          ਵਾਪਸ e;
      } ਨਹੀਂਤਾਂ {
        ਜੇ (Error e = stringAttribute(ਟੈਗ_2))
          ਵਾਪਸ e;
      }
    }
  }
  ਵਾਪਸ Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t ਲੰਮਾਈ_2) {
  uint64_t ਸਿਰਾ = cursor.tell() - ਆਕਾਰ(ਲੰਮਾਈ_2) + ਲੰਮਾਈ_2;
  StringRef vendorName = de.getCStrRef(cursor);
  ਜੇ (sw) {
    sw->print_ਸੰਖਿਆ("SectionLength", ਲੰਮਾਈ_2);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  ਜੇ (vendorName.ਹੇਠਲਾ() != vendor) {
    cursor.seek(ਸਿਰਾ);
    ਵਾਪਸ Error::success();
  }

  ਜਦੋਂ (cursor.tell() < ਸਿਰਾ) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t ਟੈਗ_2 = de.getU8(cursor);
    uint32_t ਮਾਪ_2 = de.getU32(cursor);
    ਜੇ (!cursor)
      ਵਾਪਸ cursor.takeError();

    ਜੇ (sw) {
      constexpr EnumStringDef<ਬਿਨਾਨਿਸ਼ਾਨੀ> ਟੈਗ_Name_Defs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::ਚਿੰਨ੍ਹ},
      };
      ਸਥਿਰ constexpr ਸਵੈਚਾਲਿਤ ਟੈਗ_ਨਾਂ = BUILD_ENUM_ਲੜੀਆਂ(ਟੈਗ_Name_Defs);

      sw->printEnum("Tag", ਟੈਗ_2, Enum_ਲੜੀਆਂ(ਟੈਗ_ਨਾਂ));
      sw->print_ਸੰਖਿਆ("Size", ਮਾਪ_2);
    }
    ਜੇ (ਮਾਪ_2 < 5)
      ਵਾਪਸ createStringError(errc::invalid_ਦਲੀਲ,
                               "invalid attribute size " + Twine(ਮਾਪ_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, ਸੂਚਕਾਂਕ_Name;
    Small_ਸਦਿਸ਼<uint8_t, 8> indices;
    ਚੋਣ (ਟੈਗ_2) {
    ਹਾਲਤ ELFAttrs::File:
      scopeName = "FileAttributes";
      ਰੋਕੋ;
    ਹਾਲਤ ELFAttrs::Section:
      scopeName = "SectionAttributes";
      ਸੂਚਕਾਂਕ_Name = "Sections";
      parse_ਸੂਚਕਾਂਕ_List(indices);
      ਰੋਕੋ;
    ਹਾਲਤ ELFAttrs::ਚਿੰਨ੍ਹ:
      scopeName = "SymbolAttributes";
      ਸੂਚਕਾਂਕ_Name = "Symbols";
      parse_ਸੂਚਕਾਂਕ_List(indices);
      ਰੋਕੋ;
    ਮੂਲ:
      ਵਾਪਸ createStringError(errc::invalid_ਦਲੀਲ,
                               "unrecognized tag 0x" + Twine::utohexstr(ਟੈਗ_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    ਜੇ (sw) {
      DictScope scope(*sw, scopeName);
      ਜੇ (!indices.ਖਾਲੀ_ਥਾਵਾਂ())
        sw->printList(ਸੂਚਕਾਂਕ_Name, indices);
      ਜੇ (Error e = parseAttributeList(ਮਾਪ_2 - 5))
        ਵਾਪਸ e;
    } ਨਹੀਂਤਾਂ ਜੇ (Error e = parseAttributeList(ਮਾਪ_2 - 5))
      ਵਾਪਸ e;
  }
  ਵਾਪਸ Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ section_ਸੰਖਿਆ = 0;
  de = ਅੰਕੜੇ_Extractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  ਬਣਤਰ ClearCursorError {
    ਅੰਕੜੇ_Extractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t format_ਸੰਸਕਰਣ = de.getU8(cursor);
  ਜੇ (format_ਸੰਸਕਰਣ != ELFAttrs::Format_ਸੰਸਕਰਣ)
    ਵਾਪਸ createStringError(errc::invalid_ਦਲੀਲ,
                             "unrecognized format-version: 0x" +
                                 utohexstr(format_ਸੰਸਕਰਣ));

  ਜਦੋਂ (!de.eof(cursor)) {
    uint32_t section_ਲੰਮਾਈ = de.getU32(cursor);
    ਜੇ (!cursor)
      ਵਾਪਸ cursor.takeError();

    ਜੇ (sw) {
      sw->ਸ਼ੁਰੂ_ਸਤਰ() << "Section " << ++section_ਸੰਖਿਆ << " {\n";
      sw->indent();
    }

    ਜੇ (section_ਲੰਮਾਈ < 4 || cursor.tell() - 4 + section_ਲੰਮਾਈ > section.ਮਾਪ_2())
      ਵਾਪਸ createStringError(errc::invalid_ਦਲੀਲ,
                               "invalid section length " +
                                   Twine(section_ਲੰਮਾਈ) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    ਜੇ (Error e = parseSubsection(section_ਲੰਮਾਈ))
      ਵਾਪਸ e;
    ਜੇ (sw) {
      sw->unindent();
      sw->ਸ਼ੁਰੂ_ਸਤਰ() << "}\n";
    }
  }

  ਵਾਪਸ cursor.takeError();
}
