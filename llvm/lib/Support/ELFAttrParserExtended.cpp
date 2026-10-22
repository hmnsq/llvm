//===-ELFAttrParserExtended.cpp-ELF Extended Attribute Information Printer-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM
// Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/ELFAttrParserExtended.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/Support/ELFAttributes.h"
#અન્તર્ભાવ "llvm/Support/Errc.h"
#અન્તર્ભાવ "llvm/Support/Error.h"
#અન્તર્ભાવ "llvm/Support/ScopedPrinter.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <cstdint>

using નામાકાશ llvm;
using નામાકાશ ELFAttrs;

std::optional<unsigned>
ELFExtendedAttrParser::get_Attribute_કિંમત(unsigned Tag) અચલ {
  પ્રતિજ્ઞા(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  પાછા std::nullopt;
}

std::optional<unsigned>
ELFExtendedAttrParser::get_Attribute_કિંમત(StringRef BuildAttrSubsectionName,
                                         unsigned Tag) અચલ {
  માટે (અચલ auto &SubSection : SubSectionVec) {
    જો (BuildAttrSubsectionName == SubSection.Name)
      માટે (અચલ auto &BAItem : SubSection.Content) {
        જો (Tag == BAItem.Tag)
          પાછા std::optional<unsigned>(BAItem.Int_કિંમત);
      }
  }
  પાછા std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(unsigned Tag) અચલ {
  પ્રતિજ્ઞા(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  પાછા std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(StringRef BuildAttrSubsectionName,
                                          unsigned Tag) અચલ {
  માટે (અચલ auto &SubSection : SubSectionVec) {
    જો (BuildAttrSubsectionName == SubSection.Name)
      માટે (અચલ auto &BAItem : SubSection.Content) {
        જો (Tag == BAItem.Tag)
          પાછા std::optional<StringRef>(BAItem.String_કિંમત);
      }
  }
  પાછા std::nullopt;
}

StringRef
ELFExtendedAttrParser::getTagName(અચલ StringRef &BuildAttrSubsectionName,
                                  અચલ unsigned Tag) {
  માટે (અચલ auto &Entry : TagsNamesMap) {
    જો (BuildAttrSubsectionName == Entry.SubsectionName)
      જો (Tag == Entry.Tag)
        પાછા Entry.TagName;
  }
  પાછા "";
}

Error ELFExtendedAttrParser::parse(ArrayRef<uint8_t> Section,
                                   llvm::endianness Endian) {

  unsigned Section_સંખ્યા = 0;
  De = DataExtractor(Section, Endian == llvm::endianness::little);

  // Early returns have specific errors. Consume the Error in Cursor.
  સંરચના ClearCursorError {
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
  જો (!Cursor)
    પાછા Cursor.takeError();
  જો (ELFAttrs::Format_Version != FormatVersion)
    પાછા createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(FormatVersion));

  જ્યારે (!De.eof(Cursor)) {
    uint32_t ExtBASubsectionLength = De.getU32(Cursor);
    જો (!Cursor)
      પાછા Cursor.takeError();
    // Minimal valid Extended Build Attributes subsection size is at
    // least 8: length(4) name(at least a single char + null) optionality(1) and
    // type(1)
    // Extended Build Attributes subsection has to fit inside the section.
    જો (ExtBASubsectionLength < 8 ||
        ExtBASubsectionLength > (Section.size() - Cursor.tell() + 4))
      પાછા createStringError(
          errc::invalid_argument,
          "invalid Extended Build Attributes subsection size at offset: " +
              utohexstr(Cursor.tell() - 4));

    StringRef VendorName = De.getCStrRef(Cursor);
    જો (!Cursor)
      પાછા Cursor.takeError();
    uint8_t IsOptional = De.getU8(Cursor);
    જો (!Cursor)
      પાછા Cursor.takeError();
    જો (!(0 == IsOptional || 1 == IsOptional))
      પાછા createStringError(
          errc::invalid_argument,
          "\ninvalid Optionality at offset " + utohexstr(Cursor.tell() - 4) +
              ": " + utohexstr(IsOptional) + " (Options are 1|0)");
    StringRef IsOptionalStr = IsOptional ? "optional" : "required";
    uint8_t Type = De.getU8(Cursor);
    જો (!Cursor)
      પાછા Cursor.takeError();
    જો (!(0 == Type || 1 == Type))
      પાછા createStringError(errc::invalid_argument,
                               "\ninvalid Type at offset " +
                                   utohexstr(Cursor.tell() - 4) + ": " +
                                   utohexstr(Type) + " (Options are 1|0)");
    StringRef TypeStr = Type ? "ntbs" : "uleb128";

    BuildAttributeSubSection BASubSection;
    BASubSection.Name = VendorName;
    BASubSection.IsOptional = IsOptional;
    BASubSection.ParameterType = Type;

    જો (Sw) {
      Sw->start_લીટી() << "Section " << ++Section_સંખ્યા << " {\n";
      Sw->indent();
      Sw->print_સંખ્યા("SectionLength", ExtBASubsectionLength);
      Sw->start_લીટી() << "VendorName" << ": " << VendorName
                      << " Optionality: " << IsOptionalStr
                      << " Type: " << TypeStr << "\n";
      Sw->start_લીટી() << "Attributes {\n";
      Sw->indent();
    }

    // Offset in Section
    uint64_t OffsetInSection = Cursor.tell();
    // Size: 4 bytes, Vendor Name: VendorName.size() + 1 (null termination),
    // optionality: 1, type: 1
    uint32_t BytesAllButAttributes = 4 + (VendorName.size() + 1) + 1 + 1;
    જ્યારે (Cursor.tell() <
           (OffsetInSection + ExtBASubsectionLength - BytesAllButAttributes)) {

      uint64_t Tag = De.getULEB128(Cursor);
      જો (!Cursor)
        પાછા Cursor.takeError();

      StringRef TagName = getTagName(VendorName, Tag);

      uint64_t કિંમત_Int = 0;
      std::string કિંમત_Str = "";
      જો (Type) { // type==1 --> ntbs
        કિંમત_Str = De.getCStrRef(Cursor);
        જો (!Cursor)
          પાછા Cursor.takeError();
        જો (Sw)
          Sw->printString("" != TagName ? TagName : utostr(Tag), કિંમત_Str);
      } નહીંતર { // type==0 --> uleb128
        કિંમત_Int = De.getULEB128(Cursor);
        જો (!Cursor)
          પાછા Cursor.takeError();
        જો (Sw)
          Sw->print_સંખ્યા("" != TagName ? TagName : utostr(Tag), કિંમત_Int);
      }

      // populate data structure
      BuildAttributeItem BAItem(static_cast<BuildAttributeItem::Types>(Type),
                                Tag, કિંમત_Int, કિંમત_Str);
      BASubSection.Content.ધકેલો_back(BAItem);
    }
    જો (Sw) {
      // Close 'Attributes'
      Sw->unindent();
      Sw->start_લીટી() << "}\n";
      // Close 'Section'
      Sw->unindent();
      Sw->start_લીટી() << "}\n";
    }

    // populate data structure
    SubSectionVec.ધકેલો_back(BASubSection);
  }

  પાછા Cursor.takeError();
}
