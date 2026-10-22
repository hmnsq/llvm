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

ورتوں نطاق llvm;
ورتوں نطاق ELFAttrs;

std::optional<بنانشانی>
ELFExtendedAttrParser::get_Attribute_قدر(بنانشانی ٹیگ) ثابت {
  أكد(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  أرجع std::nullopt;
}

std::optional<بنانشانی>
ELFExtendedAttrParser::get_Attribute_قدر(StringRef BuildAttrSubsectionName,
                                         بنانشانی ٹیگ) ثابت {
  لكل (ثابت خودکار &SubSection : SubSectionVec) {
    إذا (BuildAttrSubsectionName == SubSection.Name)
      لكل (ثابت خودکار &BAItem : SubSection.Content) {
        إذا (ٹیگ == BAItem.ٹیگ)
          أرجع std::optional<بنانشانی>(BAItem.Int_قدر);
      }
  }
  أرجع std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(بنانشانی ٹیگ) ثابت {
  أكد(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  أرجع std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(StringRef BuildAttrSubsectionName,
                                          بنانشانی ٹیگ) ثابت {
  لكل (ثابت خودکار &SubSection : SubSectionVec) {
    إذا (BuildAttrSubsectionName == SubSection.Name)
      لكل (ثابت خودکار &BAItem : SubSection.Content) {
        إذا (ٹیگ == BAItem.ٹیگ)
          أرجع std::optional<StringRef>(BAItem.String_قدر);
      }
  }
  أرجع std::nullopt;
}

StringRef
ELFExtendedAttrParser::get_ٹیگ_Name(ثابت StringRef &BuildAttrSubsectionName,
                                  ثابت بنانشانی ٹیگ) {
  لكل (ثابت خودکار &اندراج : ٹیگ_سوچی_ناں_Map) {
    إذا (BuildAttrSubsectionName == اندراج.SubsectionName)
      إذا (ٹیگ == اندراج.ٹیگ)
        أرجع اندراج.ٹیگ_Name_2;
  }
  أرجع "";
}

Error ELFExtendedAttrParser::parse(ArrayRef<uint8_t> Section,
                                   llvm::endianness Endian) {

  بنانشانی Section_نمبر = 0;
  De = انکڑے_Extractor(Section, Endian == llvm::endianness::little);

  // Early returns have specific errors. Consume the Error in Cursor.
  بنية ClearCursorError {
    انکڑے_Extractor::Cursor &Cursor;
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
  إذا (!Cursor)
    أرجع Cursor.takeError();
  إذا (ELFAttrs::Format_نسخہ != Format_نسخہ_2)
    أرجع createStringError(errc::invalid_دلیل,
                             "unrecognized format-version: 0x" +
                                 utohexstr(Format_نسخہ_2));

  طالما (!De.eof(Cursor)) {
    uint32_t Ext_BASubsection_لمائی = De.getU32(Cursor);
    إذا (!Cursor)
      أرجع Cursor.takeError();
    // Minimal valid Extended Build Attributes subsection size is at
    // least 8: length(4) name(at least a single char + null) optionality(1) and
    // type(1)
    // Extended Build Attributes subsection has to fit inside the section.
    إذا (Ext_BASubsection_لمائی < 8 ||
        Ext_BASubsection_لمائی > (Section.ناپ_2() - Cursor.tell() + 4))
      أرجع createStringError(
          errc::invalid_دلیل,
          "invalid Extended Build Attributes subsection size at offset: " +
              utohexstr(Cursor.tell() - 4));

    StringRef VendorName = De.getCStrRef(Cursor);
    إذا (!Cursor)
      أرجع Cursor.takeError();
    uint8_t IsOptional = De.getU8(Cursor);
    إذا (!Cursor)
      أرجع Cursor.takeError();
    إذا (!(0 == IsOptional || 1 == IsOptional))
      أرجع createStringError(
          errc::invalid_دلیل,
          "\ninvalid Optionality at offset " + utohexstr(Cursor.tell() - 4) +
              ": " + utohexstr(IsOptional) + " (Options are 1|0)");
    StringRef IsOptionalStr = IsOptional ? "optional" : "required";
    uint8_t Type = De.getU8(Cursor);
    إذا (!Cursor)
      أرجع Cursor.takeError();
    إذا (!(0 == Type || 1 == Type))
      أرجع createStringError(errc::invalid_دلیل,
                               "\ninvalid Type at offset " +
                                   utohexstr(Cursor.tell() - 4) + ": " +
                                   utohexstr(Type) + " (Options are 1|0)");
    StringRef TypeStr = Type ? "ntbs" : "uleb128";

    BuildAttributeSubSection BASubSection;
    BASubSection.Name = VendorName;
    BASubSection.IsOptional = IsOptional;
    BASubSection.ParameterType = Type;

    إذا (Sw) {
      Sw->شروع_سطر() << "Section " << ++Section_نمبر << " {\n";
      Sw->indent();
      Sw->print_نمبر("SectionLength", Ext_BASubsection_لمائی);
      Sw->شروع_سطر() << "VendorName" << ": " << VendorName
                      << " Optionality: " << IsOptionalStr
                      << " Type: " << TypeStr << "\n";
      Sw->شروع_سطر() << "Attributes {\n";
      Sw->indent();
    }

    // Offset in Section
    uint64_t وتھ_وچ_Section = Cursor.tell();
    // Size: 4 bytes, Vendor Name: VendorName.size() + 1 (null termination),
    // optionality: 1, type: 1
    uint32_t بائٹاں_All_But_گن = 4 + (VendorName.ناپ_2() + 1) + 1 + 1;
    طالما (Cursor.tell() <
           (وتھ_وچ_Section + Ext_BASubsection_لمائی - بائٹاں_All_But_گن)) {

      uint64_t ٹیگ = De.getULEB128(Cursor);
      إذا (!Cursor)
        أرجع Cursor.takeError();

      StringRef ٹیگ_Name_2 = get_ٹیگ_Name(VendorName, ٹیگ);

      uint64_t قدر_Int = 0;
      std::string قدر_Str = "";
      إذا (Type) { // type==1 --> ntbs
        قدر_Str = De.getCStrRef(Cursor);
        إذا (!Cursor)
          أرجع Cursor.takeError();
        إذا (Sw)
          Sw->printString("" != ٹیگ_Name_2 ? ٹیگ_Name_2 : utostr(ٹیگ), قدر_Str);
      } وإلا { // type==0 --> uleb128
        قدر_Int = De.getULEB128(Cursor);
        إذا (!Cursor)
          أرجع Cursor.takeError();
        إذا (Sw)
          Sw->print_نمبر("" != ٹیگ_Name_2 ? ٹیگ_Name_2 : utostr(ٹیگ), قدر_Int);
      }

      // populate data structure
      Build_Attribute_شے BAItem(static_cast<Build_Attribute_شے::Types>(Type),
                                ٹیگ, قدر_Int, قدر_Str);
      BASubSection.Content.ڈالو_پٹھ(BAItem);
    }
    إذا (Sw) {
      // Close 'Attributes'
      Sw->unindent();
      Sw->شروع_سطر() << "}\n";
      // Close 'Section'
      Sw->unindent();
      Sw->شروع_سطر() << "}\n";
    }

    // populate data structure
    SubSectionVec.ڈالو_پٹھ(BASubSection);
  }

  أرجع Cursor.takeError();
}
