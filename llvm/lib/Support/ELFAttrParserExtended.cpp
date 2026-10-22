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

استعمال نامخلا llvm;
استعمال نامخلا ELFAttrs;

std::optional<غیرعلامتی>
ELFExtendedAttrParser::get_Attribute_قدر(غیرعلامتی لیبل) ثابت {
  أكد(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  واپس std::nullopt;
}

std::optional<غیرعلامتی>
ELFExtendedAttrParser::get_Attribute_قدر(StringRef BuildAttrSubsectionName,
                                         غیرعلامتی لیبل) ثابت {
  برائے (ثابت خودکار &SubSection : SubSectionVec) {
    اگر (BuildAttrSubsectionName == SubSection.Name)
      برائے (ثابت خودکار &BAItem : SubSection.Content) {
        اگر (لیبل == BAItem.لیبل)
          واپس std::optional<غیرعلامتی>(BAItem.Int_قدر);
      }
  }
  واپس std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(غیرعلامتی لیبل) ثابت {
  أكد(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  واپس std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(StringRef BuildAttrSubsectionName,
                                          غیرعلامتی لیبل) ثابت {
  برائے (ثابت خودکار &SubSection : SubSectionVec) {
    اگر (BuildAttrSubsectionName == SubSection.Name)
      برائے (ثابت خودکار &BAItem : SubSection.Content) {
        اگر (لیبل == BAItem.لیبل)
          واپس std::optional<StringRef>(BAItem.String_قدر);
      }
  }
  واپس std::nullopt;
}

StringRef
ELFExtendedAttrParser::get_لیبل_Name(ثابت StringRef &BuildAttrSubsectionName,
                                  ثابت غیرعلامتی لیبل) {
  برائے (ثابت خودکار &اندراج_2 : لیبل_فہرست_نام_Map) {
    اگر (BuildAttrSubsectionName == اندراج_2.SubsectionName)
      اگر (لیبل == اندراج_2.لیبل)
        واپس اندراج_2.لیبل_Name_2;
  }
  واپس "";
}

Error ELFExtendedAttrParser::parse(ArrayRef<uint8_t> Section,
                                   llvm::endianness Endian) {

  غیرعلامتی Section_نمبر = 0;
  De = ڈیٹا_Extractor(Section, Endian == llvm::endianness::little);

  // Early returns have specific errors. Consume the Error in Cursor.
  ساخت ClearCursorError {
    ڈیٹا_Extractor::Cursor &Cursor;
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
  uint8_t Format_نسخہ_2 = De.getU8(Cursor);
  اگر (!Cursor)
    واپس Cursor.takeError();
  اگر (ELFAttrs::Format_نسخہ != Format_نسخہ_2)
    واپس createStringError(errc::invalid_دلیل,
                             "unrecognized format-version: 0x" +
                                 utohexstr(Format_نسخہ_2));

  جب (!De.eof(Cursor)) {
    uint32_t Ext_BASubsection_لمبائی = De.getU32(Cursor);
    اگر (!Cursor)
      واپس Cursor.takeError();
    // Minimal valid Extended Build Attributes subsection size is at
    // least 8: length(4) name(at least a single char + null) optionality(1) and
    // type(1)
    // Extended Build Attributes subsection has to fit inside the section.
    اگر (Ext_BASubsection_لمبائی < 8 ||
        Ext_BASubsection_لمبائی > (Section.سائز_2() - Cursor.tell() + 4))
      واپس createStringError(
          errc::invalid_دلیل,
          "invalid Extended Build Attributes subsection size at offset: " +
              utohexstr(Cursor.tell() - 4));

    StringRef VendorName = De.getCStrRef(Cursor);
    اگر (!Cursor)
      واپس Cursor.takeError();
    uint8_t IsOptional = De.getU8(Cursor);
    اگر (!Cursor)
      واپس Cursor.takeError();
    اگر (!(0 == IsOptional || 1 == IsOptional))
      واپس createStringError(
          errc::invalid_دلیل,
          "\ninvalid Optionality at offset " + utohexstr(Cursor.tell() - 4) +
              ": " + utohexstr(IsOptional) + " (Options are 1|0)");
    StringRef IsOptionalStr = IsOptional ? "optional" : "required";
    uint8_t Type = De.getU8(Cursor);
    اگر (!Cursor)
      واپس Cursor.takeError();
    اگر (!(0 == Type || 1 == Type))
      واپس createStringError(errc::invalid_دلیل,
                               "\ninvalid Type at offset " +
                                   utohexstr(Cursor.tell() - 4) + ": " +
                                   utohexstr(Type) + " (Options are 1|0)");
    StringRef TypeStr = Type ? "ntbs" : "uleb128";

    BuildAttributeSubSection BASubSection;
    BASubSection.Name = VendorName;
    BASubSection.IsOptional = IsOptional;
    BASubSection.ParameterType = Type;

    اگر (Sw) {
      Sw->آغاز_سطر() << "Section " << ++Section_نمبر << " {\n";
      Sw->indent();
      Sw->print_نمبر("SectionLength", Ext_BASubsection_لمبائی);
      Sw->آغاز_سطر() << "VendorName" << ": " << VendorName
                      << " Optionality: " << IsOptionalStr
                      << " Type: " << TypeStr << "\n";
      Sw->آغاز_سطر() << "Attributes {\n";
      Sw->indent();
    }

    // Offset in Section
    uint64_t انحراف_اندر_Section = Cursor.tell();
    // Size: 4 bytes, Vendor Name: VendorName.size() + 1 (null termination),
    // optionality: 1, type: 1
    uint32_t بائٹیں_All_But_اوصاف = 4 + (VendorName.سائز_2() + 1) + 1 + 1;
    جب (Cursor.tell() <
           (انحراف_اندر_Section + Ext_BASubsection_لمبائی - بائٹیں_All_But_اوصاف)) {

      uint64_t لیبل = De.getULEB128(Cursor);
      اگر (!Cursor)
        واپس Cursor.takeError();

      StringRef لیبل_Name_2 = get_لیبل_Name(VendorName, لیبل);

      uint64_t قدر_Int = 0;
      std::string قدر_Str = "";
      اگر (Type) { // type==1 --> ntbs
        قدر_Str = De.getCStrRef(Cursor);
        اگر (!Cursor)
          واپس Cursor.takeError();
        اگر (Sw)
          Sw->printString("" != لیبل_Name_2 ? لیبل_Name_2 : utostr(لیبل), قدر_Str);
      } ورنہ { // type==0 --> uleb128
        قدر_Int = De.getULEB128(Cursor);
        اگر (!Cursor)
          واپس Cursor.takeError();
        اگر (Sw)
          Sw->print_نمبر("" != لیبل_Name_2 ? لیبل_Name_2 : utostr(لیبل), قدر_Int);
      }

      // populate data structure
      Build_Attribute_شے BAItem(static_cast<Build_Attribute_شے::Types>(Type),
                                لیبل, قدر_Int, قدر_Str);
      BASubSection.Content.ڈالو_عقب(BAItem);
    }
    اگر (Sw) {
      // Close 'Attributes'
      Sw->unindent();
      Sw->آغاز_سطر() << "}\n";
      // Close 'Section'
      Sw->unindent();
      Sw->آغاز_سطر() << "}\n";
    }

    // populate data structure
    SubSectionVec.ڈالو_عقب(BASubSection);
  }

  واپس Cursor.takeError();
}
