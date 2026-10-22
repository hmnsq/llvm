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

#ಅನ್ತರ್ಭಾವ "llvm/Support/ELFAttrParserCompact.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Enum.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ScopedPrinter.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್_2, ArrayRef<ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *> ಅಕ್ಷರಸರಣಿಗಳು) {
  uint64_t ಮೌಲ್ಯ_2 = de.getULEB128(cursor);
  ಆದರೆ (ಮೌಲ್ಯ_2 >= ಅಕ್ಷರಸರಣಿಗಳು.ಗಾತ್ರ_3()) {
    printAttribute(ಟ್ಯಾಗ್_2, ಮೌಲ್ಯ_2, "");
    ಹಿಂತಿರುಗಿ createStringError(errc::invalid_ವಾದ,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(ಮೌಲ್ಯ_2));
  }
  printAttribute(ಟ್ಯಾಗ್_2, ಮೌಲ್ಯ_2, ಅಕ್ಷರಸರಣಿಗಳು[ಮೌಲ್ಯ_2]);
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ELFCompactAttrParser::integerAttribute(ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್_2) {
  StringRef ಟ್ಯಾಗ್_Name =
      ELFAttrs::attrTypeAsString(ಟ್ಯಾಗ್_2, ಟ್ಯಾಗ್_ಗಮ್ಯ_String_Map, /*hasTagPrefix=*/ಸುಳ್ಳು);
  uint64_t ಮೌಲ್ಯ_2 = de.getULEB128(cursor);
  ಗುಣಲಕ್ಷಣಗಳು_2.ಸೇರಿಸು(std::make_pair(ಟ್ಯಾಗ್_2, ಮೌಲ್ಯ_2));

  ಆದರೆ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_ಸಂಖ್ಯೆ("Tag", ಟ್ಯಾಗ್_2);
    ಆದರೆ (!ಟ್ಯಾಗ್_Name.ಖಾಲಿಗಳು())
      sw->printString("TagName", ಟ್ಯಾಗ್_Name);
    sw->print_ಸಂಖ್ಯೆ("Value", ಮೌಲ್ಯ_2);
  }
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ELFCompactAttrParser::stringAttribute(ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್_2) {
  StringRef ಟ್ಯಾಗ್_Name =
      ELFAttrs::attrTypeAsString(ಟ್ಯಾಗ್_2, ಟ್ಯಾಗ್_ಗಮ್ಯ_String_Map, /*hasTagPrefix=*/ಸುಳ್ಳು);
  StringRef desc = de.getCStrRef(cursor);
  ಗಣ_Attribute_String(ಟ್ಯಾಗ್_2, desc);

  ಆದರೆ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_ಸಂಖ್ಯೆ("Tag", ಟ್ಯಾಗ್_2);
    ಆದರೆ (!ಟ್ಯಾಗ್_Name.ಖಾಲಿಗಳು())
      sw->printString("TagName", ಟ್ಯಾಗ್_Name);
    sw->printString("Value", desc);
  }
  ಹಿಂತಿರುಗಿ Error::success();
}

ಶೂನ್ಯ ELFCompactAttrParser::printAttribute(ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್_2, ಚಿಹ್ನೆರಹಿತ ಮೌಲ್ಯ_2,
                                          StringRef ಮೌಲ್ಯ_Desc) {
  ಗುಣಲಕ್ಷಣಗಳು_2.ಸೇರಿಸು(std::make_pair(ಟ್ಯಾಗ್_2, ಮೌಲ್ಯ_2));

  ಆದರೆ (sw) {
    StringRef ಟ್ಯಾಗ್_Name = ELFAttrs::attrTypeAsString(ಟ್ಯಾಗ್_2, ಟ್ಯಾಗ್_ಗಮ್ಯ_String_Map,
                                                   /*hasTagPrefix=*/ಸುಳ್ಳು);
    DictScope as(*sw, "Attribute");
    sw->print_ಸಂಖ್ಯೆ("Tag", ಟ್ಯಾಗ್_2);
    sw->print_ಸಂಖ್ಯೆ("Value", ಮೌಲ್ಯ_2);
    ಆದರೆ (!ಟ್ಯಾಗ್_Name.ಖಾಲಿಗಳು())
      sw->printString("TagName", ಟ್ಯಾಗ್_Name);
    ಆದರೆ (!ಮೌಲ್ಯ_Desc.ಖಾಲಿಗಳು())
      sw->printString("Description", ಮೌಲ್ಯ_Desc);
  }
}

ಶೂನ್ಯ ELFCompactAttrParser::parse_ಸೂಚ್ಯಂಕ_List(Small_ಸದಿಶ_Impl<uint8_t> &ಸೂಚ್ಯಂಕ_List) {
  ಗಾಗಿ (;;) {
    uint64_t ಮೌಲ್ಯ_2 = de.getULEB128(cursor);
    ಆದರೆ (!cursor || !ಮೌಲ್ಯ_2)
      ನಿಲ್ಲಿಸು;
    ಸೂಚ್ಯಂಕ_List.ತಳ್ಳು_ಹಿಂಭಾಗ(ಮೌಲ್ಯ_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t ಉದ್ದಳತೆ_2) {
  uint64_t pos;
  uint64_t ಕೊನೆ = cursor.tell() + ಉದ್ದಳತೆ_2;
  ಆಗ ((pos = cursor.tell()) < ಕೊನೆ) {
    uint64_t ಟ್ಯಾಗ್_2 = de.getULEB128(cursor);
    ತಾರ್ಕಿಕ handled;
    ಆದರೆ (Error e = handler(ಟ್ಯಾಗ್_2, handled))
      ಹಿಂತಿರುಗಿ e;

    ಆದರೆ (!handled) {
      ಆದರೆ (ಟ್ಯಾಗ್_2 < 32) {
        ಹಿಂತಿರುಗಿ createStringError(errc::invalid_ವಾದ,
                                 "invalid tag 0x" + Twine::utohexstr(ಟ್ಯಾಗ್_2) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      ಆದರೆ (ಟ್ಯಾಗ್_2 % 2 == 0) {
        ಆದರೆ (Error e = integerAttribute(ಟ್ಯಾಗ್_2))
          ಹಿಂತಿರುಗಿ e;
      } ಇಲ್ಲದಿದ್ದರೆ {
        ಆದರೆ (Error e = stringAttribute(ಟ್ಯಾಗ್_2))
          ಹಿಂತಿರುಗಿ e;
      }
    }
  }
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t ಉದ್ದಳತೆ_2) {
  uint64_t ಕೊನೆ = cursor.tell() - ಗಾತ್ರ(ಉದ್ದಳತೆ_2) + ಉದ್ದಳತೆ_2;
  StringRef vendorName = de.getCStrRef(cursor);
  ಆದರೆ (sw) {
    sw->print_ಸಂಖ್ಯೆ("SectionLength", ಉದ್ದಳತೆ_2);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  ಆದರೆ (vendorName.ಅಧೋ() != vendor) {
    cursor.seek(ಕೊನೆ);
    ಹಿಂತಿರುಗಿ Error::success();
  }

  ಆಗ (cursor.tell() < ಕೊನೆ) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t ಟ್ಯಾಗ್_2 = de.getU8(cursor);
    uint32_t ಗಾತ್ರ_3 = de.getU32(cursor);
    ಆದರೆ (!cursor)
      ಹಿಂತಿರುಗಿ cursor.takeError();

    ಆದರೆ (sw) {
      constexpr EnumStringDef<ಚಿಹ್ನೆರಹಿತ> ಟ್ಯಾಗ್_Name_Defs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::ಚಿಹ್ನೆ},
      };
      ಸ್ಥಿರ constexpr ಸ್ವಯಂಚಾಲಿತ ಟ್ಯಾಗ್_ಹೆಸರುಗಳು = BUILD_ENUM_ಅಕ್ಷರಸರಣಿಗಳು(ಟ್ಯಾಗ್_Name_Defs);

      sw->printEnum("Tag", ಟ್ಯಾಗ್_2, Enum_ಅಕ್ಷರಸರಣಿಗಳು(ಟ್ಯಾಗ್_ಹೆಸರುಗಳು));
      sw->print_ಸಂಖ್ಯೆ("Size", ಗಾತ್ರ_3);
    }
    ಆದರೆ (ಗಾತ್ರ_3 < 5)
      ಹಿಂತಿರುಗಿ createStringError(errc::invalid_ವಾದ,
                               "invalid attribute size " + Twine(ಗಾತ್ರ_3) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, ಸೂಚ್ಯಂಕ_Name;
    Small_ಸದಿಶ<uint8_t, 8> indices;
    ಆಯ್ಕೆ (ಟ್ಯಾಗ್_2) {
    ಸ್ಥಿತಿ ELFAttrs::File:
      scopeName = "FileAttributes";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ ELFAttrs::Section:
      scopeName = "SectionAttributes";
      ಸೂಚ್ಯಂಕ_Name = "Sections";
      parse_ಸೂಚ್ಯಂಕ_List(indices);
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ ELFAttrs::ಚಿಹ್ನೆ:
      scopeName = "SymbolAttributes";
      ಸೂಚ್ಯಂಕ_Name = "Symbols";
      parse_ಸೂಚ್ಯಂಕ_List(indices);
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ createStringError(errc::invalid_ವಾದ,
                               "unrecognized tag 0x" + Twine::utohexstr(ಟ್ಯಾಗ್_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    ಆದರೆ (sw) {
      DictScope scope(*sw, scopeName);
      ಆದರೆ (!indices.ಖಾಲಿಗಳು())
        sw->printList(ಸೂಚ್ಯಂಕ_Name, indices);
      ಆದರೆ (Error e = parseAttributeList(ಗಾತ್ರ_3 - 5))
        ಹಿಂತಿರುಗಿ e;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Error e = parseAttributeList(ಗಾತ್ರ_3 - 5))
      ಹಿಂತಿರುಗಿ e;
  }
  ಹಿಂತಿರುಗಿ Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  ಚಿಹ್ನೆರಹಿತ section_ಸಂಖ್ಯೆ = 0;
  de = ದತ್ತಾಂಶ_Extractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  ರಚನೆ ClearCursorError {
    ದತ್ತಾಂಶ_Extractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t format_ಆವೃತ್ತಿ = de.getU8(cursor);
  ಆದರೆ (format_ಆವೃತ್ತಿ != ELFAttrs::Format_ಆವೃತ್ತಿ)
    ಹಿಂತಿರುಗಿ createStringError(errc::invalid_ವಾದ,
                             "unrecognized format-version: 0x" +
                                 utohexstr(format_ಆವೃತ್ತಿ));

  ಆಗ (!de.eof(cursor)) {
    uint32_t section_ಉದ್ದಳತೆ = de.getU32(cursor);
    ಆದರೆ (!cursor)
      ಹಿಂತಿರುಗಿ cursor.takeError();

    ಆದರೆ (sw) {
      sw->ಆರಂಭ_ಸಾಲು() << "Section " << ++section_ಸಂಖ್ಯೆ << " {\n";
      sw->indent();
    }

    ಆದರೆ (section_ಉದ್ದಳತೆ < 4 || cursor.tell() - 4 + section_ಉದ್ದಳತೆ > section.ಗಾತ್ರ_3())
      ಹಿಂತಿರುಗಿ createStringError(errc::invalid_ವಾದ,
                               "invalid section length " +
                                   Twine(section_ಉದ್ದಳತೆ) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    ಆದರೆ (Error e = parseSubsection(section_ಉದ್ದಳತೆ))
      ಹಿಂತಿರುಗಿ e;
    ಆದರೆ (sw) {
      sw->unindent();
      sw->ಆರಂಭ_ಸಾಲು() << "}\n";
    }
  }

  ಹಿಂತಿರುಗಿ cursor.takeError();
}
