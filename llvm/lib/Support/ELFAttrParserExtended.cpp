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
  返回 std::nullopt;
}

std::optional<unsigned>
ELFExtendedAttrParser::getAttributeValue(StringRef BuildAttrSubsectionName,
                                         unsigned Tag) const {
  为 (const auto &SubSection : SubSectionVec) {
    如果 (BuildAttrSubsectionName == SubSection.Name)
      为 (const auto &BAItem : SubSection.Content) {
        如果 (Tag == BAItem.Tag)
          返回 std::optional<unsigned>(BAItem.IntValue);
      }
  }
  返回 std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(unsigned Tag) const {
  assert(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  返回 std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(StringRef BuildAttrSubsectionName,
                                          unsigned Tag) const {
  为 (const auto &SubSection : SubSectionVec) {
    如果 (BuildAttrSubsectionName == SubSection.Name)
      为 (const auto &BAItem : SubSection.Content) {
        如果 (Tag == BAItem.Tag)
          返回 std::optional<StringRef>(BAItem.StringValue);
      }
  }
  返回 std::nullopt;
}

StringRef
ELFExtendedAttrParser::getTagName(const StringRef &BuildAttrSubsectionName,
                                  const unsigned Tag) {
  为 (const auto &Entry : TagsNamesMap) {
    如果 (BuildAttrSubsectionName == Entry.SubsectionName)
      如果 (Tag == Entry.Tag)
        返回 Entry.TagName;
  }
  返回 "";
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
  如果 (!Cursor)
    返回 Cursor.takeError();
  如果 (ELFAttrs::Format_Version != FormatVersion)
    返回 createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(FormatVersion));

  当 (!De.eof(Cursor)) {
    uint32_t ExtBASubsectionLength = De.getU32(Cursor);
    如果 (!Cursor)
      返回 Cursor.takeError();
    // Minimal valid Extended Build Attributes subsection size is at
    // least 8: length(4) name(at least a single char + null) optionality(1) and
    // type(1)
    // Extended Build Attributes subsection has to fit inside the section.
    如果 (ExtBASubsectionLength < 8 ||
        ExtBASubsectionLength > (Section.size() - Cursor.tell() + 4))
      返回 createStringError(
          errc::invalid_argument,
          "invalid Extended Build Attributes subsection size at offset: " +
              utohexstr(Cursor.tell() - 4));

    StringRef VendorName = De.getCStrRef(Cursor);
    如果 (!Cursor)
      返回 Cursor.takeError();
    uint8_t IsOptional = De.getU8(Cursor);
    如果 (!Cursor)
      返回 Cursor.takeError();
    如果 (!(0 == IsOptional || 1 == IsOptional))
      返回 createStringError(
          errc::invalid_argument,
          "\ninvalid Optionality at offset " + utohexstr(Cursor.tell() - 4) +
              ": " + utohexstr(IsOptional) + " (Options are 1|0)");
    StringRef IsOptionalStr = IsOptional ? "optional" : "required";
    uint8_t Type = De.getU8(Cursor);
    如果 (!Cursor)
      返回 Cursor.takeError();
    如果 (!(0 == Type || 1 == Type))
      返回 createStringError(errc::invalid_argument,
                               "\ninvalid Type at offset " +
                                   utohexstr(Cursor.tell() - 4) + ": " +
                                   utohexstr(Type) + " (Options are 1|0)");
    StringRef TypeStr = Type ? "ntbs" : "uleb128";

    BuildAttributeSubSection BASubSection;
    BASubSection.Name = VendorName;
    BASubSection.IsOptional = IsOptional;
    BASubSection.ParameterType = Type;

    如果 (Sw) {
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
    当 (Cursor.tell() <
           (OffsetInSection + ExtBASubsectionLength - BytesAllButAttributes)) {

      uint64_t Tag = De.getULEB128(Cursor);
      如果 (!Cursor)
        返回 Cursor.takeError();

      StringRef TagName = getTagName(VendorName, Tag);

      uint64_t ValueInt = 0;
      std::string ValueStr = "";
      如果 (Type) { // type==1 --> ntbs
        ValueStr = De.getCStrRef(Cursor);
        如果 (!Cursor)
          返回 Cursor.takeError();
        如果 (Sw)
          Sw->printString("" != TagName ? TagName : utostr(Tag), ValueStr);
      } 否则 { // type==0 --> uleb128
        ValueInt = De.getULEB128(Cursor);
        如果 (!Cursor)
          返回 Cursor.takeError();
        如果 (Sw)
          Sw->printNumber("" != TagName ? TagName : utostr(Tag), ValueInt);
      }

      // populate data structure
      BuildAttributeItem BAItem(static_cast<BuildAttributeItem::Types>(Type),
                                Tag, ValueInt, ValueStr);
      BASubSection.Content.push_back(BAItem);
    }
    如果 (Sw) {
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

  返回 Cursor.takeError();
}
