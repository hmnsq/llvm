//===-ELFAttrParserExtended.cpp-ELF Extended Attribute Information Printer-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM
// Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===------------------------------------------------------------------===//

#ضمن "llvm/Support/ELFAttrParserExtended.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/ELFAttributes.h"
#ضمن "llvm/Support/Errc.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/ScopedPrinter.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cstdint>

using نطاق llvm;
using نطاق ELFAttrs;

std::optional<unsigned>
ELFExtendedAttrParser::getAttributeValue(unsigned Tag) ثابت {
  أكد(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  החזר std::nullopt;
}

std::optional<unsigned>
ELFExtendedAttrParser::getAttributeValue(StringRef BuildAttrSubsectionName,
                                         unsigned Tag) ثابت {
  עבור (ثابت auto &SubSection : SubSectionVec) {
    אם (BuildAttrSubsectionName == SubSection.Name)
      עבור (ثابت auto &BAItem : SubSection.Content) {
        אם (Tag == BAItem.Tag)
          החזר std::optional<unsigned>(BAItem.IntValue);
      }
  }
  החזר std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(unsigned Tag) ثابت {
  أكد(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  החזר std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(StringRef BuildAttrSubsectionName,
                                          unsigned Tag) ثابت {
  עבור (ثابت auto &SubSection : SubSectionVec) {
    אם (BuildAttrSubsectionName == SubSection.Name)
      עבור (ثابت auto &BAItem : SubSection.Content) {
        אם (Tag == BAItem.Tag)
          החזר std::optional<StringRef>(BAItem.StringValue);
      }
  }
  החזר std::nullopt;
}

StringRef
ELFExtendedAttrParser::getTagName(ثابت StringRef &BuildAttrSubsectionName,
                                  ثابت unsigned Tag) {
  עבור (ثابت auto &Entry : TagsNamesMap) {
    אם (BuildAttrSubsectionName == Entry.SubsectionName)
      אם (Tag == Entry.Tag)
        החזר Entry.TagName;
  }
  החזר "";
}

Error ELFExtendedAttrParser::parse(ArrayRef<uint8_t> Section,
                                   llvm::endianness Endian) {

  unsigned SectionNumber = 0;
  De = DataExtractor(Section, Endian == llvm::endianness::little);

  // Early returns have specific errors. Consume the Error in Cursor.
  بنية ClearCursorError {
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
  אם (!Cursor)
    החזר Cursor.takeError();
  אם (ELFAttrs::Format_Version != FormatVersion)
    החזר createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(FormatVersion));

  כלעוד (!De.eof(Cursor)) {
    uint32_t ExtBASubsectionLength = De.getU32(Cursor);
    אם (!Cursor)
      החזר Cursor.takeError();
    // Minimal valid Extended Build Attributes subsection size is at
    // least 8: length(4) name(at least a single char + null) optionality(1) and
    // type(1)
    // Extended Build Attributes subsection has to fit inside the section.
    אם (ExtBASubsectionLength < 8 ||
        ExtBASubsectionLength > (Section.size() - Cursor.tell() + 4))
      החזר createStringError(
          errc::invalid_argument,
          "invalid Extended Build Attributes subsection size at offset: " +
              utohexstr(Cursor.tell() - 4));

    StringRef VendorName = De.getCStrRef(Cursor);
    אם (!Cursor)
      החזר Cursor.takeError();
    uint8_t IsOptional = De.getU8(Cursor);
    אם (!Cursor)
      החזר Cursor.takeError();
    אם (!(0 == IsOptional || 1 == IsOptional))
      החזר createStringError(
          errc::invalid_argument,
          "\ninvalid Optionality at offset " + utohexstr(Cursor.tell() - 4) +
              ": " + utohexstr(IsOptional) + " (Options are 1|0)");
    StringRef IsOptionalStr = IsOptional ? "optional" : "required";
    uint8_t Type = De.getU8(Cursor);
    אם (!Cursor)
      החזר Cursor.takeError();
    אם (!(0 == Type || 1 == Type))
      החזר createStringError(errc::invalid_argument,
                               "\ninvalid Type at offset " +
                                   utohexstr(Cursor.tell() - 4) + ": " +
                                   utohexstr(Type) + " (Options are 1|0)");
    StringRef TypeStr = Type ? "ntbs" : "uleb128";

    BuildAttributeSubSection BASubSection;
    BASubSection.Name = VendorName;
    BASubSection.IsOptional = IsOptional;
    BASubSection.ParameterType = Type;

    אם (Sw) {
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
    כלעוד (Cursor.tell() <
           (OffsetInSection + ExtBASubsectionLength - BytesAllButAttributes)) {

      uint64_t Tag = De.getULEB128(Cursor);
      אם (!Cursor)
        החזר Cursor.takeError();

      StringRef TagName = getTagName(VendorName, Tag);

      uint64_t ValueInt = 0;
      std::string ValueStr = "";
      אם (Type) { // type==1 --> ntbs
        ValueStr = De.getCStrRef(Cursor);
        אם (!Cursor)
          החזר Cursor.takeError();
        אם (Sw)
          Sw->printString("" != TagName ? TagName : utostr(Tag), ValueStr);
      } אחרת { // type==0 --> uleb128
        ValueInt = De.getULEB128(Cursor);
        אם (!Cursor)
          החזר Cursor.takeError();
        אם (Sw)
          Sw->printNumber("" != TagName ? TagName : utostr(Tag), ValueInt);
      }

      // populate data structure
      BuildAttributeItem BAItem(static_cast<BuildAttributeItem::Types>(Type),
                                Tag, ValueInt, ValueStr);
      BASubSection.Content.push_back(BAItem);
    }
    אם (Sw) {
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

  החזר Cursor.takeError();
}
