//===-ELFAttrParserExtended.cpp-ELF Extended Attribute Information Printer-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM
// Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/Support/ELFAttrParserExtended.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/Support/ELFAttributes.h"
#অন্তর্ভাব "llvm/Support/Errc.h"
#অন্তর্ভাব "llvm/Support/Error.h"
#অন্তর্ভাব "llvm/Support/ScopedPrinter.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <cstdint>

using নামাকাশ llvm;
using নামাকাশ ELFAttrs;

std::optional<unsigned>
ELFExtendedAttrParser::get_Attribute_মান(unsigned Tag) ধ্রুবক {
  প্রতিজ্ঞা(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  ফেরত std::nullopt;
}

std::optional<unsigned>
ELFExtendedAttrParser::get_Attribute_মান(StringRef BuildAttrSubsectionName,
                                         unsigned Tag) ধ্রুবক {
  জন্য (ধ্রুবক auto &SubSection : SubSectionVec) {
    যদি (BuildAttrSubsectionName == SubSection.Name)
      জন্য (ধ্রুবক auto &BAItem : SubSection.Content) {
        যদি (Tag == BAItem.Tag)
          ফেরত std::optional<unsigned>(BAItem.Int_মান);
      }
  }
  ফেরত std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(unsigned Tag) ধ্রুবক {
  প্রতিজ্ঞা(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  ফেরত std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(StringRef BuildAttrSubsectionName,
                                          unsigned Tag) ধ্রুবক {
  জন্য (ধ্রুবক auto &SubSection : SubSectionVec) {
    যদি (BuildAttrSubsectionName == SubSection.Name)
      জন্য (ধ্রুবক auto &BAItem : SubSection.Content) {
        যদি (Tag == BAItem.Tag)
          ফেরত std::optional<StringRef>(BAItem.String_মান);
      }
  }
  ফেরত std::nullopt;
}

StringRef
ELFExtendedAttrParser::getTagName(ধ্রুবক StringRef &BuildAttrSubsectionName,
                                  ধ্রুবক unsigned Tag) {
  জন্য (ধ্রুবক auto &Entry : TagsNamesMap) {
    যদি (BuildAttrSubsectionName == Entry.SubsectionName)
      যদি (Tag == Entry.Tag)
        ফেরত Entry.TagName;
  }
  ফেরত "";
}

Error ELFExtendedAttrParser::parse(ArrayRef<uint8_t> Section,
                                   llvm::endianness Endian) {

  unsigned Section_সংখ্যা = 0;
  De = তথ্য_Extractor(Section, Endian == llvm::endianness::little);

  // Early returns have specific errors. Consume the Error in Cursor.
  কাঠামো ClearCursorError {
    তথ্য_Extractor::Cursor &Cursor;
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
  যদি (!Cursor)
    ফেরত Cursor.takeError();
  যদি (ELFAttrs::Format_Version != FormatVersion)
    ফেরত createStringError(errc::invalid_argument,
                             "unrecognized format-version: 0x" +
                                 utohexstr(FormatVersion));

  যখন (!De.eof(Cursor)) {
    uint32_t Ext_BASubsection_দৈর্ঘ্য = De.getU32(Cursor);
    যদি (!Cursor)
      ফেরত Cursor.takeError();
    // Minimal valid Extended Build Attributes subsection size is at
    // least 8: length(4) name(at least a single char + null) optionality(1) and
    // type(1)
    // Extended Build Attributes subsection has to fit inside the section.
    যদি (Ext_BASubsection_দৈর্ঘ্য < 8 ||
        Ext_BASubsection_দৈর্ঘ্য > (Section.আকার_3() - Cursor.tell() + 4))
      ফেরত createStringError(
          errc::invalid_argument,
          "invalid Extended Build Attributes subsection size at offset: " +
              utohexstr(Cursor.tell() - 4));

    StringRef VendorName = De.getCStrRef(Cursor);
    যদি (!Cursor)
      ফেরত Cursor.takeError();
    uint8_t IsOptional = De.getU8(Cursor);
    যদি (!Cursor)
      ফেরত Cursor.takeError();
    যদি (!(0 == IsOptional || 1 == IsOptional))
      ফেরত createStringError(
          errc::invalid_argument,
          "\ninvalid Optionality at offset " + utohexstr(Cursor.tell() - 4) +
              ": " + utohexstr(IsOptional) + " (Options are 1|0)");
    StringRef IsOptionalStr = IsOptional ? "optional" : "required";
    uint8_t Type = De.getU8(Cursor);
    যদি (!Cursor)
      ফেরত Cursor.takeError();
    যদি (!(0 == Type || 1 == Type))
      ফেরত createStringError(errc::invalid_argument,
                               "\ninvalid Type at offset " +
                                   utohexstr(Cursor.tell() - 4) + ": " +
                                   utohexstr(Type) + " (Options are 1|0)");
    StringRef TypeStr = Type ? "ntbs" : "uleb128";

    BuildAttributeSubSection BASubSection;
    BASubSection.Name = VendorName;
    BASubSection.IsOptional = IsOptional;
    BASubSection.ParameterType = Type;

    যদি (Sw) {
      Sw->start_সারি() << "Section " << ++Section_সংখ্যা << " {\n";
      Sw->indent();
      Sw->print_সংখ্যা("SectionLength", Ext_BASubsection_দৈর্ঘ্য);
      Sw->start_সারি() << "VendorName" << ": " << VendorName
                      << " Optionality: " << IsOptionalStr
                      << " Type: " << TypeStr << "\n";
      Sw->start_সারি() << "Attributes {\n";
      Sw->indent();
    }

    // Offset in Section
    uint64_t OffsetInSection = Cursor.tell();
    // Size: 4 bytes, Vendor Name: VendorName.size() + 1 (null termination),
    // optionality: 1, type: 1
    uint32_t BytesAllButAttributes = 4 + (VendorName.আকার_3() + 1) + 1 + 1;
    যখন (Cursor.tell() <
           (OffsetInSection + Ext_BASubsection_দৈর্ঘ্য - BytesAllButAttributes)) {

      uint64_t Tag = De.getULEB128(Cursor);
      যদি (!Cursor)
        ফেরত Cursor.takeError();

      StringRef TagName = getTagName(VendorName, Tag);

      uint64_t মান_Int = 0;
      std::string মান_Str = "";
      যদি (Type) { // type==1 --> ntbs
        মান_Str = De.getCStrRef(Cursor);
        যদি (!Cursor)
          ফেরত Cursor.takeError();
        যদি (Sw)
          Sw->printString("" != TagName ? TagName : utostr(Tag), মান_Str);
      } অন্যথা { // type==0 --> uleb128
        মান_Int = De.getULEB128(Cursor);
        যদি (!Cursor)
          ফেরত Cursor.takeError();
        যদি (Sw)
          Sw->print_সংখ্যা("" != TagName ? TagName : utostr(Tag), মান_Int);
      }

      // populate data structure
      BuildAttributeItem BAItem(static_cast<BuildAttributeItem::Types>(Type),
                                Tag, মান_Int, মান_Str);
      BASubSection.Content.ঠেলো_back(BAItem);
    }
    যদি (Sw) {
      // Close 'Attributes'
      Sw->unindent();
      Sw->start_সারি() << "}\n";
      // Close 'Section'
      Sw->unindent();
      Sw->start_সারি() << "}\n";
    }

    // populate data structure
    SubSectionVec.ঠেলো_back(BASubSection);
  }

  ফেরত Cursor.takeError();
}
