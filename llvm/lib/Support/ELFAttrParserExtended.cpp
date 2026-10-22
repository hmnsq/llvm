//===-ELFAttrParserExtended.cpp-ELF Extended Attribute Information Printer-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM
// Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===------------------------------------------------------------------===//

#include "llvm/Support/ELFAttrParserExtended.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/ELFAttributes.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/ScopedPrinter.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdint>

using namespace llvm;
using namespace ELFAttrs;

std::optional<unsigned>
ELFExtendedAttrParser::getAttributeValue(unsigned Tag) const {
  assert(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  ᮝᮜᮤ std::nullopt;
}

std::optional<unsigned>
ELFExtendedAttrParser::getAttributeValue(StringRef BuildAttrSubsectionName,
                                         unsigned Tag) const {
  for (const auto &SubSection : SubSectionVec) {
    ᮚᮦᮔ᮪ (BuildAttrSubsectionName == SubSection.Name)
      for (const auto &BAItem : SubSection.Content) {
        ᮚᮦᮔ᮪ (Tag == BAItem.Tag)
          ᮝᮜᮤ std::optional<unsigned>(BAItem.IntValue);
      }
  }
  ᮝᮜᮤ std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(unsigned Tag) const {
  assert(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  ᮝᮜᮤ std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(StringRef BuildAttrSubsectionName,
                                          unsigned Tag) const {
  for (const auto &SubSection : SubSectionVec) {
    ᮚᮦᮔ᮪ (BuildAttrSubsectionName == SubSection.Name)
      for (const auto &BAItem : SubSection.Content) {
        ᮚᮦᮔ᮪ (Tag == BAItem.Tag)
          ᮝᮜᮤ std::optional<StringRef>(BAItem.StringValue);
      }
  }
  ᮝᮜᮤ std::nullopt;
}

StringRef
ELFExtendedAttrParser::getTagName(const StringRef &BuildAttrSubsectionName,
                                  const unsigned Tag) {
  for (const auto &Entry : TagsNamesMap) {
    ᮚᮦᮔ᮪ (BuildAttrSubsectionName == Entry.SubsectionName)
      ᮚᮦᮔ᮪ (Tag == Entry.Tag)
        ᮝᮜᮤ Entry.TagName;
  }
  ᮝᮜᮤ "";
}

Error ELFExtendedAttrParser::parse(ArrayRef<uint8_t> Section,
                                   llvm::endianness Endian) {

  unsigned SectionNumber = 0;
  De = DataExtractor(Section, Endian == llvm::endianness::little);

  // Early returns have specific errors. Consume the Error in Cursor.
  sa_nracanaa ClearCursorError {
    DataExtractor::Cursor &Cursor;
    ~ClearCursorError() { consumeError(Cursor.takeError()); }
  } Clear{Cursor};

  /*
      ELF Extended Build Attributes Layout:
      <format-version: ‘A’> --> Currently, there is only one version: 'A' (0x41)
      [ <uint32: subsection-length> <NTBS: vendor-name> <bytes: vendor-data> ]
        --> subsection-length: Offset from the start of this subsection to the
     start of the next one.
        --> vendor-name: Null-terminated byte string.
        --> vendor-data expands to:
          [ <uint8: optional> <uint8: parameter type> <attribute>* ]
            --> optional: 0 = required, 1 = optional.
            --> parameter type: 0 = ULEB128, 1 = NTBS.
            --> attribute: <tag, value>* pair. Tag is ULEB128, value is of
     <parameter type>.
  */

  // Get format-version
  uint8_t FormatVersion = De.getU8(Cursor);
  ᮚᮦᮔ᮪ (!Cursor)
    ᮝᮜᮤ Cursor.takeError();
  ᮚᮦᮔ᮪ (ELFAttrs::Format_Version != FormatVersion)
    ᮝᮜᮤ createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(FormatVersion));

  ᮞᮨᮓᮨᮜ (!De.eof(Cursor)) {
    uint32_t ExtBASubsectionLength = De.getU32(Cursor);
    ᮚᮦᮔ᮪ (!Cursor)
      ᮝᮜᮤ Cursor.takeError();
    // Minimal valid Extended Build Attributes subsection size is at
    // least 8: length(4) name(at least a single char + null) optionality(1) and
    // type(1)
    // Extended Build Attributes subsection has to fit inside the section.
    ᮚᮦᮔ᮪ (ExtBASubsectionLength < 8 ||
        ExtBASubsectionLength > (Section.size() - Cursor.tell() + 4))
      ᮝᮜᮤ createStringError(
          errc::invalid_argument,
          "invalid Extended Build Attributes subsection size at offset: " +
              utohexstr(Cursor.tell() - 4));

    StringRef VendorName = De.getCStrRef(Cursor);
    ᮚᮦᮔ᮪ (!Cursor)
      ᮝᮜᮤ Cursor.takeError();
    uint8_t IsOptional = De.getU8(Cursor);
    ᮚᮦᮔ᮪ (!Cursor)
      ᮝᮜᮤ Cursor.takeError();
    ᮚᮦᮔ᮪ (!(0 == IsOptional || 1 == IsOptional))
      ᮝᮜᮤ createStringError(
          errc::invalid_argument,
          "\ninvalid Optionality at offset " + utohexstr(Cursor.tell() - 4) +
              ": " + utohexstr(IsOptional) + " (Options are 1|0)");
    StringRef IsOptionalStr = IsOptional ? "optional" : "required";
    uint8_t Type = De.getU8(Cursor);
    ᮚᮦᮔ᮪ (!Cursor)
      ᮝᮜᮤ Cursor.takeError();
    ᮚᮦᮔ᮪ (!(0 == Type || 1 == Type))
      ᮝᮜᮤ createStringError(errc::invalid_argument,
                               "\ninvalid Type at offset " +
                                   utohexstr(Cursor.tell() - 4) + ": " +
                                   utohexstr(Type) + " (Options are 1|0)");
    StringRef TypeStr = Type ? "ntbs" : "uleb128";

    BuildAttributeSubSection BASubSection;
    BASubSection.Name = VendorName;
    BASubSection.IsOptional = IsOptional;
    BASubSection.ParameterType = Type;

    ᮚᮦᮔ᮪ (Sw) {
      Sw->startLine() << "Section " << ++SectionNumber << " {\n";
      Sw->indent();
      Sw->printNumber("SectionLength", ExtBASubsectionLength);
      Sw->startLine() << "VendorName" << ": " << VendorName
                      << " Optionality: " << IsOptionalStr
                      << " Type: " << TypeStr << "\n";
      Sw->startLine() << "Attributes {\n";
      Sw->indent();
    }

    // Offset in Section
    uint64_t OffsetInSection = Cursor.tell();
    // Size: 4 bytes, Vendor Name: VendorName.size() + 1 (null termination),
    // optionality: 1, type: 1
    uint32_t BytesAllButAttributes = 4 + (VendorName.size() + 1) + 1 + 1;
    ᮞᮨᮓᮨᮜ (Cursor.tell() <
           (OffsetInSection + ExtBASubsectionLength - BytesAllButAttributes)) {

      uint64_t Tag = De.getULEB128(Cursor);
      ᮚᮦᮔ᮪ (!Cursor)
        ᮝᮜᮤ Cursor.takeError();

      StringRef TagName = getTagName(VendorName, Tag);

      uint64_t ValueInt = 0;
      std::string ValueStr = "";
      ᮚᮦᮔ᮪ (Type) { // type==1 --> ntbs
        ValueStr = De.getCStrRef(Cursor);
        ᮚᮦᮔ᮪ (!Cursor)
          ᮝᮜᮤ Cursor.takeError();
        ᮚᮦᮔ᮪ (Sw)
          Sw->printString("" != TagName ? TagName : utostr(Tag), ValueStr);
      } ᮜᮤᮚ { // type==0 --> uleb128
        ValueInt = De.getULEB128(Cursor);
        ᮚᮦᮔ᮪ (!Cursor)
          ᮝᮜᮤ Cursor.takeError();
        ᮚᮦᮔ᮪ (Sw)
          Sw->printNumber("" != TagName ? TagName : utostr(Tag), ValueInt);
      }

      // populate data structure
      BuildAttributeItem BAItem(static_cast<BuildAttributeItem::Types>(Type),
                                Tag, ValueInt, ValueStr);
      BASubSection.Content.push_back(BAItem);
    }
    ᮚᮦᮔ᮪ (Sw) {
      // Close 'Attributes'
      Sw->unindent();
      Sw->startLine() << "}\n";
      // Close 'Section'
      Sw->unindent();
      Sw->startLine() << "}\n";
    }

    // populate data structure
    SubSectionVec.push_back(BASubSection);
  }

  ᮝᮜᮤ Cursor.takeError();
}
