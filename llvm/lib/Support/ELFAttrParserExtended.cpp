//===-ELFAttrParserExtended.cpp-ELF Extended Attribute Information Printer-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM
// Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===------------------------------------------------------------------===//

#شامل "llvm/Support/ELFAttrParserExtended.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Support/ELFAttributes.h"
#شامل "llvm/Support/Errc.h"
#شامل "llvm/Support/Error.h"
#شامل "llvm/Support/ScopedPrinter.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cstdint>

using فضاینام llvm;
using فضاینام ELFAttrs;

std::optional<unsigned>
ELFExtendedAttrParser::get_Attribute_مقدار(unsigned Tag) ثابت {
  أكد(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  بازگشت std::nullopt;
}

std::optional<unsigned>
ELFExtendedAttrParser::get_Attribute_مقدار(StringRef BuildAttrSubsectionName,
                                         unsigned Tag) ثابت {
  برای (ثابت auto &SubSection : SubSectionVec) {
    اگر (BuildAttrSubsectionName == SubSection.Name)
      برای (ثابت auto &BAItem : SubSection.Content) {
        اگر (Tag == BAItem.Tag)
          بازگشت std::optional<unsigned>(BAItem.Int_مقدار);
      }
  }
  بازگشت std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(unsigned Tag) ثابت {
  أكد(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  بازگشت std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(StringRef BuildAttrSubsectionName,
                                          unsigned Tag) ثابت {
  برای (ثابت auto &SubSection : SubSectionVec) {
    اگر (BuildAttrSubsectionName == SubSection.Name)
      برای (ثابت auto &BAItem : SubSection.Content) {
        اگر (Tag == BAItem.Tag)
          بازگشت std::optional<StringRef>(BAItem.String_مقدار);
      }
  }
  بازگشت std::nullopt;
}

StringRef
ELFExtendedAttrParser::getTagName(ثابت StringRef &BuildAttrSubsectionName,
                                  ثابت unsigned Tag) {
  برای (ثابت auto &Entry : TagsNamesMap) {
    اگر (BuildAttrSubsectionName == Entry.SubsectionName)
      اگر (Tag == Entry.Tag)
        بازگشت Entry.TagName;
  }
  بازگشت "";
}

Error ELFExtendedAttrParser::parse(ArrayRef<uint8_t> Section,
                                   llvm::endianness Endian) {

  unsigned Section_عدد = 0;
  De = DataExtractor(Section, Endian == llvm::endianness::little);

  // Early returns have specific errors. Consume the Error in Cursor.
  ساختار ClearCursorError {
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
  اگر (!Cursor)
    بازگشت Cursor.takeError();
  اگر (ELFAttrs::Format_Version != FormatVersion)
    بازگشت createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(FormatVersion));

  وقتی (!De.eof(Cursor)) {
    uint32_t ExtBASubsectionLength = De.getU32(Cursor);
    اگر (!Cursor)
      بازگشت Cursor.takeError();
    // Minimal valid Extended Build Attributes subsection size is at
    // least 8: length(4) name(at least a single char + null) optionality(1) and
    // type(1)
    // Extended Build Attributes subsection has to fit inside the section.
    اگر (ExtBASubsectionLength < 8 ||
        ExtBASubsectionLength > (Section.size() - Cursor.tell() + 4))
      بازگشت createStringError(
          errc::invalid_argument,
          "invalid Extended Build Attributes subsection size at offset: " +
              utohexstr(Cursor.tell() - 4));

    StringRef VendorName = De.getCStrRef(Cursor);
    اگر (!Cursor)
      بازگشت Cursor.takeError();
    uint8_t IsOptional = De.getU8(Cursor);
    اگر (!Cursor)
      بازگشت Cursor.takeError();
    اگر (!(0 == IsOptional || 1 == IsOptional))
      بازگشت createStringError(
          errc::invalid_argument,
          "\ninvalid Optionality at offset " + utohexstr(Cursor.tell() - 4) +
              ": " + utohexstr(IsOptional) + " (Options are 1|0)");
    StringRef IsOptionalStr = IsOptional ? "optional" : "required";
    uint8_t Type = De.getU8(Cursor);
    اگر (!Cursor)
      بازگشت Cursor.takeError();
    اگر (!(0 == Type || 1 == Type))
      بازگشت createStringError(errc::invalid_argument,
                               "\ninvalid Type at offset " +
                                   utohexstr(Cursor.tell() - 4) + ": " +
                                   utohexstr(Type) + " (Options are 1|0)");
    StringRef TypeStr = Type ? "ntbs" : "uleb128";

    BuildAttributeSubSection BASubSection;
    BASubSection.Name = VendorName;
    BASubSection.IsOptional = IsOptional;
    BASubSection.ParameterType = Type;

    اگر (Sw) {
      Sw->start_سطر() << "Section " << ++Section_عدد << " {\n";
      Sw->indent();
      Sw->print_عدد("SectionLength", ExtBASubsectionLength);
      Sw->start_سطر() << "VendorName" << ": " << VendorName
                      << " Optionality: " << IsOptionalStr
                      << " Type: " << TypeStr << "\n";
      Sw->start_سطر() << "Attributes {\n";
      Sw->indent();
    }

    // Offset in Section
    uint64_t OffsetInSection = Cursor.tell();
    // Size: 4 bytes, Vendor Name: VendorName.size() + 1 (null termination),
    // optionality: 1, type: 1
    uint32_t BytesAllButAttributes = 4 + (VendorName.size() + 1) + 1 + 1;
    وقتی (Cursor.tell() <
           (OffsetInSection + ExtBASubsectionLength - BytesAllButAttributes)) {

      uint64_t Tag = De.getULEB128(Cursor);
      اگر (!Cursor)
        بازگشت Cursor.takeError();

      StringRef TagName = getTagName(VendorName, Tag);

      uint64_t مقدار_Int = 0;
      std::string مقدار_Str = "";
      اگر (Type) { // type==1 --> ntbs
        مقدار_Str = De.getCStrRef(Cursor);
        اگر (!Cursor)
          بازگشت Cursor.takeError();
        اگر (Sw)
          Sw->printString("" != TagName ? TagName : utostr(Tag), مقدار_Str);
      } وگرنه { // type==0 --> uleb128
        مقدار_Int = De.getULEB128(Cursor);
        اگر (!Cursor)
          بازگشت Cursor.takeError();
        اگر (Sw)
          Sw->print_عدد("" != TagName ? TagName : utostr(Tag), مقدار_Int);
      }

      // populate data structure
      BuildAttributeItem BAItem(static_cast<BuildAttributeItem::Types>(Type),
                                Tag, مقدار_Int, مقدار_Str);
      BASubSection.Content.افزودن_back(BAItem);
    }
    اگر (Sw) {
      // Close 'Attributes'
      Sw->unindent();
      Sw->start_سطر() << "}\n";
      // Close 'Section'
      Sw->unindent();
      Sw->start_سطر() << "}\n";
    }

    // populate data structure
    SubSectionVec.افزودن_back(BASubSection);
  }

  بازگشت Cursor.takeError();
}
