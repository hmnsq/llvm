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

#అన్తర్భావ "llvm/Support/ELFAttrParserCompact.h"
#అన్తర్భావ "llvm/ADT/Enum.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/Support/Errc.h"
#అన్తర్భావ "llvm/Support/ScopedPrinter.h"

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::ELFAttrs;

Error ELFCompactAttrParser::parseStringAttribute(
    స్థిరరాశి అక్షరం *name, గుర్తులేని ట్యాగ్_2, ArrayRef<స్థిరరాశి అక్షరం *> అక్షరశ్రేణులు) {
  uint64_t విలువ_2 = de.getULEB128(cursor);
  ఒకవేళ (విలువ_2 >= అక్షరశ్రేణులు.కొలత_2()) {
    printAttribute(ట్యాగ్_2, విలువ_2, "");
    తిరిగి createStringError(errc::invalid_వాదం,
                             "unknown " + Twine(name) +
                                 " value: " + Twine(విలువ_2));
  }
  printAttribute(ట్యాగ్_2, విలువ_2, అక్షరశ్రేణులు[విలువ_2]);
  తిరిగి Error::success();
}

Error ELFCompactAttrParser::integerAttribute(గుర్తులేని ట్యాగ్_2) {
  StringRef ట్యాగ్_Name =
      ELFAttrs::attrTypeAsString(ట్యాగ్_2, ట్యాగ్_గమ్యం_String_Map, /*hasTagPrefix=*/అబద్ధం);
  uint64_t విలువ_2 = de.getULEB128(cursor);
  లక్షణాలు_2.చొప్పించు(std::make_pair(ట్యాగ్_2, విలువ_2));

  ఒకవేళ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_సంఖ్య("Tag", ట్యాగ్_2);
    ఒకవేళ (!ట్యాగ్_Name.ఖాళీలు())
      sw->printString("TagName", ట్యాగ్_Name);
    sw->print_సంఖ్య("Value", విలువ_2);
  }
  తిరిగి Error::success();
}

Error ELFCompactAttrParser::stringAttribute(గుర్తులేని ట్యాగ్_2) {
  StringRef ట్యాగ్_Name =
      ELFAttrs::attrTypeAsString(ట్యాగ్_2, ట్యాగ్_గమ్యం_String_Map, /*hasTagPrefix=*/అబద్ధం);
  StringRef desc = de.getCStrRef(cursor);
  సమితి_Attribute_String(ట్యాగ్_2, desc);

  ఒకవేళ (sw) {
    DictScope scope(*sw, "Attribute");
    sw->print_సంఖ్య("Tag", ట్యాగ్_2);
    ఒకవేళ (!ట్యాగ్_Name.ఖాళీలు())
      sw->printString("TagName", ట్యాగ్_Name);
    sw->printString("Value", desc);
  }
  తిరిగి Error::success();
}

శూన్య ELFCompactAttrParser::printAttribute(గుర్తులేని ట్యాగ్_2, గుర్తులేని విలువ_2,
                                          StringRef విలువ_Desc) {
  లక్షణాలు_2.చొప్పించు(std::make_pair(ట్యాగ్_2, విలువ_2));

  ఒకవేళ (sw) {
    StringRef ట్యాగ్_Name = ELFAttrs::attrTypeAsString(ట్యాగ్_2, ట్యాగ్_గమ్యం_String_Map,
                                                   /*hasTagPrefix=*/అబద్ధం);
    DictScope as(*sw, "Attribute");
    sw->print_సంఖ్య("Tag", ట్యాగ్_2);
    sw->print_సంఖ్య("Value", విలువ_2);
    ఒకవేళ (!ట్యాగ్_Name.ఖాళీలు())
      sw->printString("TagName", ట్యాగ్_Name);
    ఒకవేళ (!విలువ_Desc.ఖాళీలు())
      sw->printString("Description", విలువ_Desc);
  }
}

శూన్య ELFCompactAttrParser::parse_సూచీ_List(Small_సదిశ_Impl<uint8_t> &సూచీ_List) {
  కొరకు (;;) {
    uint64_t విలువ_2 = de.getULEB128(cursor);
    ఒకవేళ (!cursor || !విలువ_2)
      ఆపు;
    సూచీ_List.నెట్టు_వెనుక(విలువ_2);
  }
}

Error ELFCompactAttrParser::parseAttributeList(uint32_t నిడివి_2) {
  uint64_t pos;
  uint64_t చివర = cursor.tell() + నిడివి_2;
  అయితే ((pos = cursor.tell()) < చివర) {
    uint64_t ట్యాగ్_2 = de.getULEB128(cursor);
    తార్కిక handled;
    ఒకవేళ (Error e = handler(ట్యాగ్_2, handled))
      తిరిగి e;

    ఒకవేళ (!handled) {
      ఒకవేళ (ట్యాగ్_2 < 32) {
        తిరిగి createStringError(errc::invalid_వాదం,
                                 "invalid tag 0x" + Twine::utohexstr(ట్యాగ్_2) +
                                     " at offset 0x" + Twine::utohexstr(pos));
      }

      ఒకవేళ (ట్యాగ్_2 % 2 == 0) {
        ఒకవేళ (Error e = integerAttribute(ట్యాగ్_2))
          తిరిగి e;
      } లేకపోతే {
        ఒకవేళ (Error e = stringAttribute(ట్యాగ్_2))
          తిరిగి e;
      }
    }
  }
  తిరిగి Error::success();
}

Error ELFCompactAttrParser::parseSubsection(uint32_t నిడివి_2) {
  uint64_t చివర = cursor.tell() - పరిమాణం(నిడివి_2) + నిడివి_2;
  StringRef vendorName = de.getCStrRef(cursor);
  ఒకవేళ (sw) {
    sw->print_సంఖ్య("SectionLength", నిడివి_2);
    sw->printString("Vendor", vendorName);
  }

  // Handle a subsection with an unrecognized vendor-name by skipping
  // over it to the next subsection. ADDENDA32 in the Arm ABI defines
  // that vendor attribute sections must not affect compatibility, so
  // this should always be safe.
  ఒకవేళ (vendorName.క్రింది() != vendor) {
    cursor.seek(చివర);
    తిరిగి Error::success();
  }

  అయితే (cursor.tell() < చివర) {
    /// Tag_File | Tag_Section | Tag_Symbol   uleb128:byte-size
    uint8_t ట్యాగ్_2 = de.getU8(cursor);
    uint32_t కొలత_2 = de.getU32(cursor);
    ఒకవేళ (!cursor)
      తిరిగి cursor.takeError();

    ఒకవేళ (sw) {
      constexpr EnumStringDef<గుర్తులేని> ట్యాగ్_Name_Defs[] = {
          {{"Tag_File"}, ELFAttrs::File},
          {{"Tag_Section"}, ELFAttrs::Section},
          {{"Tag_Symbol"}, ELFAttrs::చిహ్నం},
      };
      స్థిర constexpr స్వయంచాలక ట్యాగ్_పేర్లు = BUILD_ENUM_అక్షరశ్రేణులు(ట్యాగ్_Name_Defs);

      sw->printEnum("Tag", ట్యాగ్_2, Enum_అక్షరశ్రేణులు(ట్యాగ్_పేర్లు));
      sw->print_సంఖ్య("Size", కొలత_2);
    }
    ఒకవేళ (కొలత_2 < 5)
      తిరిగి createStringError(errc::invalid_వాదం,
                               "invalid attribute size " + Twine(కొలత_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));

    StringRef scopeName, సూచీ_Name;
    Small_సదిశ<uint8_t, 8> indices;
    ఎంపిక (ట్యాగ్_2) {
    స్థితి ELFAttrs::File:
      scopeName = "FileAttributes";
      ఆపు;
    స్థితి ELFAttrs::Section:
      scopeName = "SectionAttributes";
      సూచీ_Name = "Sections";
      parse_సూచీ_List(indices);
      ఆపు;
    స్థితి ELFAttrs::చిహ్నం:
      scopeName = "SymbolAttributes";
      సూచీ_Name = "Symbols";
      parse_సూచీ_List(indices);
      ఆపు;
    అప్రమేయ:
      తిరిగి createStringError(errc::invalid_వాదం,
                               "unrecognized tag 0x" + Twine::utohexstr(ట్యాగ్_2) +
                                   " at offset 0x" +
                                   Twine::utohexstr(cursor.tell() - 5));
    }

    ఒకవేళ (sw) {
      DictScope scope(*sw, scopeName);
      ఒకవేళ (!indices.ఖాళీలు())
        sw->printList(సూచీ_Name, indices);
      ఒకవేళ (Error e = parseAttributeList(కొలత_2 - 5))
        తిరిగి e;
    } లేకపోతే ఒకవేళ (Error e = parseAttributeList(కొలత_2 - 5))
      తిరిగి e;
  }
  తిరిగి Error::success();
}

Error ELFCompactAttrParser::parse(ArrayRef<uint8_t> section,
                                  llvm::endianness endian) {
  గుర్తులేని section_సంఖ్య = 0;
  de = దత్తాంశం_Extractor(section, endian == llvm::endianness::little);

  // For early returns, we have more specific errors, consume the Error in
  // cursor.
  నిర్మాణం ClearCursorError {
    దత్తాంశం_Extractor::Cursor &cursor;
    ~ClearCursorError() { consumeError(cursor.takeError()); }
  } clear{cursor};

  // Unrecognized format-version.
  uint8_t format_సంస్కరణ = de.getU8(cursor);
  ఒకవేళ (format_సంస్కరణ != ELFAttrs::Format_సంస్కరణ)
    తిరిగి createStringError(errc::invalid_వాదం,
                             "unrecognized format-version: 0x" +
                                 utohexstr(format_సంస్కరణ));

  అయితే (!de.eof(cursor)) {
    uint32_t section_నిడివి = de.getU32(cursor);
    ఒకవేళ (!cursor)
      తిరిగి cursor.takeError();

    ఒకవేళ (sw) {
      sw->ప్రారంభం_పంక్తి() << "Section " << ++section_సంఖ్య << " {\n";
      sw->indent();
    }

    ఒకవేళ (section_నిడివి < 4 || cursor.tell() - 4 + section_నిడివి > section.కొలత_2())
      తిరిగి createStringError(errc::invalid_వాదం,
                               "invalid section length " +
                                   Twine(section_నిడివి) + " at offset 0x" +
                                   utohexstr(cursor.tell() - 4));

    ఒకవేళ (Error e = parseSubsection(section_నిడివి))
      తిరిగి e;
    ఒకవేళ (sw) {
      sw->unindent();
      sw->ప్రారంభం_పంక్తి() << "}\n";
    }
  }

  తిరిగి cursor.takeError();
}
