//===-ELFAttrParserExtended.cpp-ELF Extended Attribute Information Printer-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM
// Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ELFAttrParserExtended.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ELFAttributes.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Errc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ScopedPrinter.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdint>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ ELFAttrs;

std::optional<ਬਿਨਾਨਿਸ਼ਾਨੀ>
ELFExtendedAttrParser::get_Attribute_ਮੁੱਲ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  ਵਾਪਸ std::nullopt;
}

std::optional<ਬਿਨਾਨਿਸ਼ਾਨੀ>
ELFExtendedAttrParser::get_Attribute_ਮੁੱਲ(StringRef BuildAttrSubsectionName,
                                         ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ) ਅਚਲ {
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &SubSection : SubSectionVec) {
    ਜੇ (BuildAttrSubsectionName == SubSection.Name)
      ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &BAItem : SubSection.Content) {
        ਜੇ (ਟੈਗ == BAItem.ਟੈਗ)
          ਵਾਪਸ std::optional<ਬਿਨਾਨਿਸ਼ਾਨੀ>(BAItem.Int_ਮੁੱਲ);
      }
  }
  ਵਾਪਸ std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ) ਅਚਲ {
  ਪ੍ਰਤਿਜ੍ਞਾ(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  ਵਾਪਸ std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(StringRef BuildAttrSubsectionName,
                                          ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ) ਅਚਲ {
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &SubSection : SubSectionVec) {
    ਜੇ (BuildAttrSubsectionName == SubSection.Name)
      ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &BAItem : SubSection.Content) {
        ਜੇ (ਟੈਗ == BAItem.ਟੈਗ)
          ਵਾਪਸ std::optional<StringRef>(BAItem.String_ਮੁੱਲ);
      }
  }
  ਵਾਪਸ std::nullopt;
}

StringRef
ELFExtendedAttrParser::get_ਟੈਗ_Name(ਅਚਲ StringRef &BuildAttrSubsectionName,
                                  ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਟੈਗ) {
  ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &ਇੰਦਰਾਜ : ਟੈਗ_ਸੂਚੀ_ਨਾਂ_Map) {
    ਜੇ (BuildAttrSubsectionName == ਇੰਦਰਾਜ.SubsectionName)
      ਜੇ (ਟੈਗ == ਇੰਦਰਾਜ.ਟੈਗ)
        ਵਾਪਸ ਇੰਦਰਾਜ.ਟੈਗ_Name_2;
  }
  ਵਾਪਸ "";
}

Error ELFExtendedAttrParser::parse(ArrayRef<uint8_t> Section,
                                   llvm::endianness Endian) {

  ਬਿਨਾਨਿਸ਼ਾਨੀ Section_ਸੰਖਿਆ = 0;
  De = ਅੰਕੜੇ_Extractor(Section, Endian == llvm::endianness::little);

  // Early returns have specific errors. Consume the Error in Cursor.
  ਬਣਤਰ ClearCursorError {
    ਅੰਕੜੇ_Extractor::Cursor &Cursor;
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
  uint8_t Format_ਸੰਸਕਰਣ_2 = De.getU8(Cursor);
  ਜੇ (!Cursor)
    ਵਾਪਸ Cursor.takeError();
  ਜੇ (ELFAttrs::Format_ਸੰਸਕਰਣ != Format_ਸੰਸਕਰਣ_2)
    ਵਾਪਸ createStringError(errc::invalid_ਦਲੀਲ,
                             "unrecognized format-version: 0x" +
                                 utohexstr(Format_ਸੰਸਕਰਣ_2));

  ਜਦੋਂ (!De.eof(Cursor)) {
    uint32_t Ext_BASubsection_ਲੰਮਾਈ = De.getU32(Cursor);
    ਜੇ (!Cursor)
      ਵਾਪਸ Cursor.takeError();
    // Minimal valid Extended Build Attributes subsection size is at
    // least 8: length(4) name(at least a single char + null) optionality(1) and
    // type(1)
    // Extended Build Attributes subsection has to fit inside the section.
    ਜੇ (Ext_BASubsection_ਲੰਮਾਈ < 8 ||
        Ext_BASubsection_ਲੰਮਾਈ > (Section.ਮਾਪ_2() - Cursor.tell() + 4))
      ਵਾਪਸ createStringError(
          errc::invalid_ਦਲੀਲ,
          "invalid Extended Build Attributes subsection size at offset: " +
              utohexstr(Cursor.tell() - 4));

    StringRef VendorName = De.getCStrRef(Cursor);
    ਜੇ (!Cursor)
      ਵਾਪਸ Cursor.takeError();
    uint8_t IsOptional = De.getU8(Cursor);
    ਜੇ (!Cursor)
      ਵਾਪਸ Cursor.takeError();
    ਜੇ (!(0 == IsOptional || 1 == IsOptional))
      ਵਾਪਸ createStringError(
          errc::invalid_ਦਲੀਲ,
          "\ninvalid Optionality at offset " + utohexstr(Cursor.tell() - 4) +
              ": " + utohexstr(IsOptional) + " (Options are 1|0)");
    StringRef IsOptionalStr = IsOptional ? "optional" : "required";
    uint8_t Type = De.getU8(Cursor);
    ਜੇ (!Cursor)
      ਵਾਪਸ Cursor.takeError();
    ਜੇ (!(0 == Type || 1 == Type))
      ਵਾਪਸ createStringError(errc::invalid_ਦਲੀਲ,
                               "\ninvalid Type at offset " +
                                   utohexstr(Cursor.tell() - 4) + ": " +
                                   utohexstr(Type) + " (Options are 1|0)");
    StringRef TypeStr = Type ? "ntbs" : "uleb128";

    BuildAttributeSubSection BASubSection;
    BASubSection.Name = VendorName;
    BASubSection.IsOptional = IsOptional;
    BASubSection.ParameterType = Type;

    ਜੇ (Sw) {
      Sw->ਸ਼ੁਰੂ_ਸਤਰ() << "Section " << ++Section_ਸੰਖਿਆ << " {\n";
      Sw->indent();
      Sw->print_ਸੰਖਿਆ("SectionLength", Ext_BASubsection_ਲੰਮਾਈ);
      Sw->ਸ਼ੁਰੂ_ਸਤਰ() << "VendorName" << ": " << VendorName
                      << " Optionality: " << IsOptionalStr
                      << " Type: " << TypeStr << "\n";
      Sw->ਸ਼ੁਰੂ_ਸਤਰ() << "Attributes {\n";
      Sw->indent();
    }

    // Offset in Section
    uint64_t ਵਿੱਥ_ਵਿੱਚ_Section = Cursor.tell();
    // Size: 4 bytes, Vendor Name: VendorName.size() + 1 (null termination),
    // optionality: 1, type: 1
    uint32_t ਬਾਈਟਾਂ_All_But_ਗੁਣ = 4 + (VendorName.ਮਾਪ_2() + 1) + 1 + 1;
    ਜਦੋਂ (Cursor.tell() <
           (ਵਿੱਥ_ਵਿੱਚ_Section + Ext_BASubsection_ਲੰਮਾਈ - ਬਾਈਟਾਂ_All_But_ਗੁਣ)) {

      uint64_t ਟੈਗ = De.getULEB128(Cursor);
      ਜੇ (!Cursor)
        ਵਾਪਸ Cursor.takeError();

      StringRef ਟੈਗ_Name_2 = get_ਟੈਗ_Name(VendorName, ਟੈਗ);

      uint64_t ਮੁੱਲ_Int = 0;
      std::string ਮੁੱਲ_Str = "";
      ਜੇ (Type) { // type==1 --> ntbs
        ਮੁੱਲ_Str = De.getCStrRef(Cursor);
        ਜੇ (!Cursor)
          ਵਾਪਸ Cursor.takeError();
        ਜੇ (Sw)
          Sw->printString("" != ਟੈਗ_Name_2 ? ਟੈਗ_Name_2 : utostr(ਟੈਗ), ਮੁੱਲ_Str);
      } ਨਹੀਂਤਾਂ { // type==0 --> uleb128
        ਮੁੱਲ_Int = De.getULEB128(Cursor);
        ਜੇ (!Cursor)
          ਵਾਪਸ Cursor.takeError();
        ਜੇ (Sw)
          Sw->print_ਸੰਖਿਆ("" != ਟੈਗ_Name_2 ? ਟੈਗ_Name_2 : utostr(ਟੈਗ), ਮੁੱਲ_Int);
      }

      // populate data structure
      Build_Attribute_ਵਸਤੂ BAItem(static_cast<Build_Attribute_ਵਸਤੂ::Types>(Type),
                                ਟੈਗ, ਮੁੱਲ_Int, ਮੁੱਲ_Str);
      BASubSection.Content.ਧੱਕੋ_ਪਿੱਠ(BAItem);
    }
    ਜੇ (Sw) {
      // Close 'Attributes'
      Sw->unindent();
      Sw->ਸ਼ੁਰੂ_ਸਤਰ() << "}\n";
      // Close 'Section'
      Sw->unindent();
      Sw->ਸ਼ੁਰੂ_ਸਤਰ() << "}\n";
    }

    // populate data structure
    SubSectionVec.ਧੱਕੋ_ਪਿੱਠ(BASubSection);
  }

  ਵਾਪਸ Cursor.takeError();
}
