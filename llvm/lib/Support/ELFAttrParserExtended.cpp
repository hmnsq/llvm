//===-ELFAttrParserExtended.cpp-ELF Extended Attribute Information Printer-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM
// Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/ELFAttrParserExtended.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/Support/ELFAttributes.h"
#அந்தர்பாவ "llvm/Support/Errc.h"
#அந்தர்பாவ "llvm/Support/Error.h"
#அந்தர்பாவ "llvm/Support/ScopedPrinter.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <cstdint>

using நாமாகாஶ llvm;
using நாமாகாஶ ELFAttrs;

std::optional<unsigned>
ELFExtendedAttrParser::get_Attribute_மதிப்பு(unsigned Tag) மாறிலி {
  ப்ரதிஜ்ஞா(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  திரும்பு std::nullopt;
}

std::optional<unsigned>
ELFExtendedAttrParser::get_Attribute_மதிப்பு(StringRef BuildAttrSubsectionName,
                                         unsigned Tag) மாறிலி {
  க்கு (மாறிலி auto &SubSection : SubSectionVec) {
    எனில் (BuildAttrSubsectionName == SubSection.Name)
      க்கு (மாறிலி auto &BAItem : SubSection.Content) {
        எனில் (Tag == BAItem.Tag)
          திரும்பு std::optional<unsigned>(BAItem.Int_மதிப்பு);
      }
  }
  திரும்பு std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(unsigned Tag) மாறிலி {
  ப்ரதிஜ்ஞா(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  திரும்பு std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(StringRef BuildAttrSubsectionName,
                                          unsigned Tag) மாறிலி {
  க்கு (மாறிலி auto &SubSection : SubSectionVec) {
    எனில் (BuildAttrSubsectionName == SubSection.Name)
      க்கு (மாறிலி auto &BAItem : SubSection.Content) {
        எனில் (Tag == BAItem.Tag)
          திரும்பு std::optional<StringRef>(BAItem.String_மதிப்பு);
      }
  }
  திரும்பு std::nullopt;
}

StringRef
ELFExtendedAttrParser::getTagName(மாறிலி StringRef &BuildAttrSubsectionName,
                                  மாறிலி unsigned Tag) {
  க்கு (மாறிலி auto &Entry : TagsNamesMap) {
    எனில் (BuildAttrSubsectionName == Entry.SubsectionName)
      எனில் (Tag == Entry.Tag)
        திரும்பு Entry.TagName;
  }
  திரும்பு "";
}

Error ELFExtendedAttrParser::parse(ArrayRef<uint8_t> Section,
                                   llvm::endianness Endian) {

  unsigned Section_எண் = 0;
  De = DataExtractor(Section, Endian == llvm::endianness::little);

  // Early returns have specific errors. Consume the Error in Cursor.
  கட்டமைப்பு ClearCursorError {
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
  எனில் (!Cursor)
    திரும்பு Cursor.takeError();
  எனில் (ELFAttrs::Format_Version != FormatVersion)
    திரும்பு createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(FormatVersion));

  போது (!De.eof(Cursor)) {
    uint32_t ExtBASubsectionLength = De.getU32(Cursor);
    எனில் (!Cursor)
      திரும்பு Cursor.takeError();
    // Minimal valid Extended Build Attributes subsection size is at
    // least 8: length(4) name(at least a single char + null) optionality(1) and
    // type(1)
    // Extended Build Attributes subsection has to fit inside the section.
    எனில் (ExtBASubsectionLength < 8 ||
        ExtBASubsectionLength > (Section.size() - Cursor.tell() + 4))
      திரும்பு createStringError(
          errc::invalid_argument,
          "invalid Extended Build Attributes subsection size at offset: " +
              utohexstr(Cursor.tell() - 4));

    StringRef VendorName = De.getCStrRef(Cursor);
    எனில் (!Cursor)
      திரும்பு Cursor.takeError();
    uint8_t IsOptional = De.getU8(Cursor);
    எனில் (!Cursor)
      திரும்பு Cursor.takeError();
    எனில் (!(0 == IsOptional || 1 == IsOptional))
      திரும்பு createStringError(
          errc::invalid_argument,
          "\ninvalid Optionality at offset " + utohexstr(Cursor.tell() - 4) +
              ": " + utohexstr(IsOptional) + " (Options are 1|0)");
    StringRef IsOptionalStr = IsOptional ? "optional" : "required";
    uint8_t Type = De.getU8(Cursor);
    எனில் (!Cursor)
      திரும்பு Cursor.takeError();
    எனில் (!(0 == Type || 1 == Type))
      திரும்பு createStringError(errc::invalid_argument,
                               "\ninvalid Type at offset " +
                                   utohexstr(Cursor.tell() - 4) + ": " +
                                   utohexstr(Type) + " (Options are 1|0)");
    StringRef TypeStr = Type ? "ntbs" : "uleb128";

    BuildAttributeSubSection BASubSection;
    BASubSection.Name = VendorName;
    BASubSection.IsOptional = IsOptional;
    BASubSection.ParameterType = Type;

    எனில் (Sw) {
      Sw->start_வரி() << "Section " << ++Section_எண் << " {\n";
      Sw->indent();
      Sw->print_எண்("SectionLength", ExtBASubsectionLength);
      Sw->start_வரி() << "VendorName" << ": " << VendorName
                      << " Optionality: " << IsOptionalStr
                      << " Type: " << TypeStr << "\n";
      Sw->start_வரி() << "Attributes {\n";
      Sw->indent();
    }

    // Offset in Section
    uint64_t OffsetInSection = Cursor.tell();
    // Size: 4 bytes, Vendor Name: VendorName.size() + 1 (null termination),
    // optionality: 1, type: 1
    uint32_t BytesAllButAttributes = 4 + (VendorName.size() + 1) + 1 + 1;
    போது (Cursor.tell() <
           (OffsetInSection + ExtBASubsectionLength - BytesAllButAttributes)) {

      uint64_t Tag = De.getULEB128(Cursor);
      எனில் (!Cursor)
        திரும்பு Cursor.takeError();

      StringRef TagName = getTagName(VendorName, Tag);

      uint64_t மதிப்பு_Int = 0;
      std::string மதிப்பு_Str = "";
      எனில் (Type) { // type==1 --> ntbs
        மதிப்பு_Str = De.getCStrRef(Cursor);
        எனில் (!Cursor)
          திரும்பு Cursor.takeError();
        எனில் (Sw)
          Sw->printString("" != TagName ? TagName : utostr(Tag), மதிப்பு_Str);
      } இல்லையெனில் { // type==0 --> uleb128
        மதிப்பு_Int = De.getULEB128(Cursor);
        எனில் (!Cursor)
          திரும்பு Cursor.takeError();
        எனில் (Sw)
          Sw->print_எண்("" != TagName ? TagName : utostr(Tag), மதிப்பு_Int);
      }

      // populate data structure
      BuildAttributeItem BAItem(static_cast<BuildAttributeItem::Types>(Type),
                                Tag, மதிப்பு_Int, மதிப்பு_Str);
      BASubSection.Content.தள்ளு_back(BAItem);
    }
    எனில் (Sw) {
      // Close 'Attributes'
      Sw->unindent();
      Sw->start_வரி() << "}\n";
      // Close 'Section'
      Sw->unindent();
      Sw->start_வரி() << "}\n";
    }

    // populate data structure
    SubSectionVec.தள்ளு_back(BASubSection);
  }

  திரும்பு Cursor.takeError();
}
