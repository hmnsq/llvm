//===-ELFAttrParserExtended.cpp-ELF Extended Attribute Information Printer-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM
// Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===------------------------------------------------------------------===//

#समावेश "llvm/Support/ELFAttrParserExtended.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/Support/ELFAttributes.h"
#समावेश "llvm/Support/Errc.h"
#समावेश "llvm/Support/Error.h"
#समावेश "llvm/Support/ScopedPrinter.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <cstdint>

using namespace llvm;
using namespace ELFAttrs;

std::optional<अचिन्हित>
ELFExtendedAttrParser::get_Attribute_मूल्य(अचिन्हित Tag) स्थिर {
  निश्चित(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  वापस std::nullopt;
}

std::optional<अचिन्हित>
ELFExtendedAttrParser::get_Attribute_मूल्य(StringRef BuildAttrSubsectionName,
                                         अचिन्हित Tag) स्थिर {
  क्रम (स्थिर स्वतः &SubSection : SubSectionVec) {
    अगर (BuildAttrSubsectionName == SubSection.Name)
      क्रम (स्थिर स्वतः &BAItem : SubSection.Content) {
        अगर (Tag == BAItem.Tag)
          वापस std::optional<अचिन्हित>(BAItem.Int_मूल्य);
      }
  }
  वापस std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(अचिन्हित Tag) स्थिर {
  निश्चित(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  वापस std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(StringRef BuildAttrSubsectionName,
                                          अचिन्हित Tag) स्थिर {
  क्रम (स्थिर स्वतः &SubSection : SubSectionVec) {
    अगर (BuildAttrSubsectionName == SubSection.Name)
      क्रम (स्थिर स्वतः &BAItem : SubSection.Content) {
        अगर (Tag == BAItem.Tag)
          वापस std::optional<StringRef>(BAItem.String_मूल्य);
      }
  }
  वापस std::nullopt;
}

StringRef
ELFExtendedAttrParser::getTagName(स्थिर StringRef &BuildAttrSubsectionName,
                                  स्थिर अचिन्हित Tag) {
  क्रम (स्थिर स्वतः &Entry : TagsNamesMap) {
    अगर (BuildAttrSubsectionName == Entry.SubsectionName)
      अगर (Tag == Entry.Tag)
        वापस Entry.TagName;
  }
  वापस "";
}

Error ELFExtendedAttrParser::parse(ArrayRef<uint8_t> Section,
                                   llvm::endianness Endian) {

  अचिन्हित Section_संख्या = 0;
  De = तथ्य_Extractor(Section, Endian == llvm::endianness::little);

  // Early returns have specific errors. Consume the Error in Cursor.
  काष्ठा ClearCursorError {
    तथ्य_Extractor::Cursor &Cursor;
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
  अगर (!Cursor)
    वापस Cursor.takeError();
  अगर (ELFAttrs::Format_Version != FormatVersion)
    वापस createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(FormatVersion));

  जबतक (!De.खप(Cursor)) {
    uint32_t Ext_BASubsection_लंबाई = De.getU32(Cursor);
    अगर (!Cursor)
      वापस Cursor.takeError();
    // Minimal valid Extended Build Attributes subsection size is at
    // least 8: length(4) name(at least a single char + null) optionality(1) and
    // type(1)
    // Extended Build Attributes subsection has to fit inside the section.
    अगर (Ext_BASubsection_लंबाई < 8 ||
        Ext_BASubsection_लंबाई > (Section.आकार_2() - Cursor.tell() + 4))
      वापस createStringError(
          errc::invalid_argument,
          "invalid Extended Build Attributes subsection size at offset: " +
              utohexstr(Cursor.tell() - 4));

    StringRef VendorName = De.getCStrRef(Cursor);
    अगर (!Cursor)
      वापस Cursor.takeError();
    uint8_t IsOptional = De.getU8(Cursor);
    अगर (!Cursor)
      वापस Cursor.takeError();
    अगर (!(0 == IsOptional || 1 == IsOptional))
      वापस createStringError(
          errc::invalid_argument,
          "\ninvalid Optionality at offset " + utohexstr(Cursor.tell() - 4) +
              ": " + utohexstr(IsOptional) + " (Options are 1|0)");
    StringRef IsOptionalStr = IsOptional ? "optional" : "required";
    uint8_t Type = De.getU8(Cursor);
    अगर (!Cursor)
      वापस Cursor.takeError();
    अगर (!(0 == Type || 1 == Type))
      वापस createStringError(errc::invalid_argument,
                               "\ninvalid Type at offset " +
                                   utohexstr(Cursor.tell() - 4) + ": " +
                                   utohexstr(Type) + " (Options are 1|0)");
    StringRef TypeStr = Type ? "ntbs" : "uleb128";

    BuildAttributeSubSection BASubSection;
    BASubSection.Name = VendorName;
    BASubSection.IsOptional = IsOptional;
    BASubSection.ParameterType = Type;

    अगर (Sw) {
      Sw->start_पंक्ति() << "Section " << ++Section_संख्या << " {\n";
      Sw->indent();
      Sw->print_संख्या("SectionLength", Ext_BASubsection_लंबाई);
      Sw->start_पंक्ति() << "VendorName" << ": " << VendorName
                      << " Optionality: " << IsOptionalStr
                      << " Type: " << TypeStr << "\n";
      Sw->start_पंक्ति() << "Attributes {\n";
      Sw->indent();
    }

    // Offset in Section
    uint64_t OffsetInSection = Cursor.tell();
    // Size: 4 bytes, Vendor Name: VendorName.size() + 1 (null termination),
    // optionality: 1, type: 1
    uint32_t BytesAllButAttributes = 4 + (VendorName.आकार_2() + 1) + 1 + 1;
    जबतक (Cursor.tell() <
           (OffsetInSection + Ext_BASubsection_लंबाई - BytesAllButAttributes)) {

      uint64_t Tag = De.getULEB128(Cursor);
      अगर (!Cursor)
        वापस Cursor.takeError();

      StringRef TagName = getTagName(VendorName, Tag);

      uint64_t मूल्य_Int = 0;
      std::string मूल्य_Str = "";
      अगर (Type) { // type==1 --> ntbs
        मूल्य_Str = De.getCStrRef(Cursor);
        अगर (!Cursor)
          वापस Cursor.takeError();
        अगर (Sw)
          Sw->printString("" != TagName ? TagName : utostr(Tag), मूल्य_Str);
      } अन्यथा { // type==0 --> uleb128
        मूल्य_Int = De.getULEB128(Cursor);
        अगर (!Cursor)
          वापस Cursor.takeError();
        अगर (Sw)
          Sw->print_संख्या("" != TagName ? TagName : utostr(Tag), मूल्य_Int);
      }

      // populate data structure
      BuildAttributeItem BAItem(static_cast<BuildAttributeItem::Types>(Type),
                                Tag, मूल्य_Int, मूल्य_Str);
      BASubSection.Content.धकेलो_back(BAItem);
    }
    अगर (Sw) {
      // Close 'Attributes'
      Sw->unindent();
      Sw->start_पंक्ति() << "}\n";
      // Close 'Section'
      Sw->unindent();
      Sw->start_पंक्ति() << "}\n";
    }

    // populate data structure
    SubSectionVec.धकेलो_back(BASubSection);
  }

  वापस Cursor.takeError();
}
