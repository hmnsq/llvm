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
  врати std::nullopt;
}

std::optional<unsigned>
ELFExtendedAttrParser::getAttributeValue(StringRef BuildAttrSubsectionName,
                                         unsigned Tag) const {
  за (const auto &SubSection : SubSectionVec) {
    ако (BuildAttrSubsectionName == SubSection.Name)
      за (const auto &BAItem : SubSection.Content) {
        ако (Tag == BAItem.Tag)
          врати std::optional<unsigned>(BAItem.IntValue);
      }
  }
  врати std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(unsigned Tag) const {
  assert(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  врати std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(StringRef BuildAttrSubsectionName,
                                          unsigned Tag) const {
  за (const auto &SubSection : SubSectionVec) {
    ако (BuildAttrSubsectionName == SubSection.Name)
      за (const auto &BAItem : SubSection.Content) {
        ако (Tag == BAItem.Tag)
          врати std::optional<StringRef>(BAItem.StringValue);
      }
  }
  врати std::nullopt;
}

StringRef
ELFExtendedAttrParser::getTagName(const StringRef &BuildAttrSubsectionName,
                                  const unsigned Tag) {
  за (const auto &Entry : TagsNamesMap) {
    ако (BuildAttrSubsectionName == Entry.SubsectionName)
      ако (Tag == Entry.Tag)
        врати Entry.TagName;
  }
  врати "";
}

Error ELFExtendedAttrParser::parse(ArrayRef<uint8_t> Section,
                                   llvm::endianness Endian) {

  unsigned SectionNumber = 0;
  De = DataExtractor(Section, Endian == llvm::endianness::little);

  // Early returns have specific errors. Consume the Error in Cursor.
  struct ClearCursorError {
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
  ако (!Cursor)
    врати Cursor.takeError();
  ако (ELFAttrs::Format_Version != FormatVersion)
    врати createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(FormatVersion));

  док (!De.eof(Cursor)) {
    uint32_t ExtBASubsectionLength = De.getU32(Cursor);
    ако (!Cursor)
      врати Cursor.takeError();
    // Minimal valid Extended Build Attributes subsection size is at
    // least 8: length(4) name(at least a single char + null) optionality(1) and
    // type(1)
    // Extended Build Attributes subsection has to fit inside the section.
    ако (ExtBASubsectionLength < 8 ||
        ExtBASubsectionLength > (Section.size() - Cursor.tell() + 4))
      врати createStringError(
          errc::invalid_argument,
          "invalid Extended Build Attributes subsection size at offset: " +
              utohexstr(Cursor.tell() - 4));

    StringRef VendorName = De.getCStrRef(Cursor);
    ако (!Cursor)
      врати Cursor.takeError();
    uint8_t IsOptional = De.getU8(Cursor);
    ако (!Cursor)
      врати Cursor.takeError();
    ако (!(0 == IsOptional || 1 == IsOptional))
      врати createStringError(
          errc::invalid_argument,
          "\ninvalid Optionality at offset " + utohexstr(Cursor.tell() - 4) +
              ": " + utohexstr(IsOptional) + " (Options are 1|0)");
    StringRef IsOptionalStr = IsOptional ? "optional" : "required";
    uint8_t Type = De.getU8(Cursor);
    ако (!Cursor)
      врати Cursor.takeError();
    ако (!(0 == Type || 1 == Type))
      врати createStringError(errc::invalid_argument,
                               "\ninvalid Type at offset " +
                                   utohexstr(Cursor.tell() - 4) + ": " +
                                   utohexstr(Type) + " (Options are 1|0)");
    StringRef TypeStr = Type ? "ntbs" : "uleb128";

    BuildAttributeSubSection BASubSection;
    BASubSection.Name = VendorName;
    BASubSection.IsOptional = IsOptional;
    BASubSection.ParameterType = Type;

    ако (Sw) {
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
    док (Cursor.tell() <
           (OffsetInSection + ExtBASubsectionLength - BytesAllButAttributes)) {

      uint64_t Tag = De.getULEB128(Cursor);
      ако (!Cursor)
        врати Cursor.takeError();

      StringRef TagName = getTagName(VendorName, Tag);

      uint64_t ValueInt = 0;
      std::string ValueStr = "";
      ако (Type) { // type==1 --> ntbs
        ValueStr = De.getCStrRef(Cursor);
        ако (!Cursor)
          врати Cursor.takeError();
        ако (Sw)
          Sw->printString("" != TagName ? TagName : utostr(Tag), ValueStr);
      } инако { // type==0 --> uleb128
        ValueInt = De.getULEB128(Cursor);
        ако (!Cursor)
          врати Cursor.takeError();
        ако (Sw)
          Sw->printNumber("" != TagName ? TagName : utostr(Tag), ValueInt);
      }

      // populate data structure
      BuildAttributeItem BAItem(static_cast<BuildAttributeItem::Types>(Type),
                                Tag, ValueInt, ValueStr);
      BASubSection.Content.push_back(BAItem);
    }
    ако (Sw) {
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

  врати Cursor.takeError();
}
