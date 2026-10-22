//===-ELFAttrParserExtended.cpp-ELF Extended Attribute Information Printer-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM
// Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/ELFAttrParserExtended.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Support/ELFAttributes.h"
#अन्तर्भाव "llvm/Support/Errc.h"
#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव "llvm/Support/ScopedPrinter.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cstdint>

using नामाकाश llvm;
using नामाकाश ELFAttrs;

std::optional<अचिन्हित>
ELFExtendedAttrParser::get_Attribute_मूल्य(अचिन्हित Tag) स्थिरांक {
  प्रतिज्ञा(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  परत std::nullopt;
}

std::optional<अचिन्हित>
ELFExtendedAttrParser::get_Attribute_मूल्य(StringRef BuildAttrSubsectionName,
                                         अचिन्हित Tag) स्थिरांक {
  साठी (स्थिरांक स्वतः &SubSection : SubSectionVec) {
    जर (BuildAttrSubsectionName == SubSection.Name)
      साठी (स्थिरांक स्वतः &BAItem : SubSection.Content) {
        जर (Tag == BAItem.Tag)
          परत std::optional<अचिन्हित>(BAItem.Int_मूल्य);
      }
  }
  परत std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(अचिन्हित Tag) स्थिरांक {
  प्रतिज्ञा(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  परत std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(StringRef BuildAttrSubsectionName,
                                          अचिन्हित Tag) स्थिरांक {
  साठी (स्थिरांक स्वतः &SubSection : SubSectionVec) {
    जर (BuildAttrSubsectionName == SubSection.Name)
      साठी (स्थिरांक स्वतः &BAItem : SubSection.Content) {
        जर (Tag == BAItem.Tag)
          परत std::optional<StringRef>(BAItem.String_मूल्य);
      }
  }
  परत std::nullopt;
}

StringRef
ELFExtendedAttrParser::getTagName(स्थिरांक StringRef &BuildAttrSubsectionName,
                                  स्थिरांक अचिन्हित Tag) {
  साठी (स्थिरांक स्वतः &Entry : TagsNamesMap) {
    जर (BuildAttrSubsectionName == Entry.SubsectionName)
      जर (Tag == Entry.Tag)
        परत Entry.TagName;
  }
  परत "";
}

Error ELFExtendedAttrParser::parse(ArrayRef<uint8_t> Section,
                                   llvm::endianness Endian) {

  अचिन्हित Section_संख्या = 0;
  De = DataExtractor(Section, Endian == llvm::endianness::little);

  // Early returns have specific errors. Consume the Error in Cursor.
  संरचना ClearCursorError {
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
  जर (!Cursor)
    परत Cursor.takeError();
  जर (ELFAttrs::Format_Version != FormatVersion)
    परत createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(FormatVersion));

  पर्यंत (!De.eof(Cursor)) {
    uint32_t ExtBASubsectionLength = De.getU32(Cursor);
    जर (!Cursor)
      परत Cursor.takeError();
    // Minimal valid Extended Build Attributes subsection size is at
    // least 8: length(4) name(at least a single char + null) optionality(1) and
    // type(1)
    // Extended Build Attributes subsection has to fit inside the section.
    जर (ExtBASubsectionLength < 8 ||
        ExtBASubsectionLength > (Section.size() - Cursor.tell() + 4))
      परत createStringError(
          errc::invalid_argument,
          "invalid Extended Build Attributes subsection size at offset: " +
              utohexstr(Cursor.tell() - 4));

    StringRef VendorName = De.getCStrRef(Cursor);
    जर (!Cursor)
      परत Cursor.takeError();
    uint8_t IsOptional = De.getU8(Cursor);
    जर (!Cursor)
      परत Cursor.takeError();
    जर (!(0 == IsOptional || 1 == IsOptional))
      परत createStringError(
          errc::invalid_argument,
          "\ninvalid Optionality at offset " + utohexstr(Cursor.tell() - 4) +
              ": " + utohexstr(IsOptional) + " (Options are 1|0)");
    StringRef IsOptionalStr = IsOptional ? "optional" : "required";
    uint8_t Type = De.getU8(Cursor);
    जर (!Cursor)
      परत Cursor.takeError();
    जर (!(0 == Type || 1 == Type))
      परत createStringError(errc::invalid_argument,
                               "\ninvalid Type at offset " +
                                   utohexstr(Cursor.tell() - 4) + ": " +
                                   utohexstr(Type) + " (Options are 1|0)");
    StringRef TypeStr = Type ? "ntbs" : "uleb128";

    BuildAttributeSubSection BASubSection;
    BASubSection.Name = VendorName;
    BASubSection.IsOptional = IsOptional;
    BASubSection.ParameterType = Type;

    जर (Sw) {
      Sw->start_ओळ() << "Section " << ++Section_संख्या << " {\n";
      Sw->indent();
      Sw->print_संख्या("SectionLength", ExtBASubsectionLength);
      Sw->start_ओळ() << "VendorName" << ": " << VendorName
                      << " Optionality: " << IsOptionalStr
                      << " Type: " << TypeStr << "\n";
      Sw->start_ओळ() << "Attributes {\n";
      Sw->indent();
    }

    // Offset in Section
    uint64_t OffsetInSection = Cursor.tell();
    // Size: 4 bytes, Vendor Name: VendorName.size() + 1 (null termination),
    // optionality: 1, type: 1
    uint32_t BytesAllButAttributes = 4 + (VendorName.size() + 1) + 1 + 1;
    पर्यंत (Cursor.tell() <
           (OffsetInSection + ExtBASubsectionLength - BytesAllButAttributes)) {

      uint64_t Tag = De.getULEB128(Cursor);
      जर (!Cursor)
        परत Cursor.takeError();

      StringRef TagName = getTagName(VendorName, Tag);

      uint64_t मूल्य_Int = 0;
      std::string मूल्य_Str = "";
      जर (Type) { // type==1 --> ntbs
        मूल्य_Str = De.getCStrRef(Cursor);
        जर (!Cursor)
          परत Cursor.takeError();
        जर (Sw)
          Sw->printString("" != TagName ? TagName : utostr(Tag), मूल्य_Str);
      } अन्यथा { // type==0 --> uleb128
        मूल्य_Int = De.getULEB128(Cursor);
        जर (!Cursor)
          परत Cursor.takeError();
        जर (Sw)
          Sw->print_संख्या("" != TagName ? TagName : utostr(Tag), मूल्य_Int);
      }

      // populate data structure
      BuildAttributeItem BAItem(static_cast<BuildAttributeItem::Types>(Type),
                                Tag, मूल्य_Int, मूल्य_Str);
      BASubSection.Content.ढकला_back(BAItem);
    }
    जर (Sw) {
      // Close 'Attributes'
      Sw->unindent();
      Sw->start_ओळ() << "}\n";
      // Close 'Section'
      Sw->unindent();
      Sw->start_ओळ() << "}\n";
    }

    // populate data structure
    SubSectionVec.ढकला_back(BASubSection);
  }

  परत Cursor.takeError();
}
