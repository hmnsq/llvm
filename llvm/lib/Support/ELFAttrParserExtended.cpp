//===-ELFAttrParserExtended.cpp-ELF Extended Attribute Information Printer-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM
// Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/ELFAttrParserExtended.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ELFAttributes.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Errc.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ScopedPrinter.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <cstdint>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ ELFAttrs;

std::optional<ಚಿಹ್ನೆರಹಿತ>
ELFExtendedAttrParser::get_Attribute_ಮೌಲ್ಯ(ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  ಹಿಂತಿರುಗಿ std::nullopt;
}

std::optional<ಚಿಹ್ನೆರಹಿತ>
ELFExtendedAttrParser::get_Attribute_ಮೌಲ್ಯ(StringRef BuildAttrSubsectionName,
                                         ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್) ಸ್ಥಿರಾಂಕ {
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &SubSection : SubSectionVec) {
    ಆದರೆ (BuildAttrSubsectionName == SubSection.Name)
      ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &BAItem : SubSection.Content) {
        ಆದರೆ (ಟ್ಯಾಗ್ == BAItem.ಟ್ಯಾಗ್)
          ಹಿಂತಿರುಗಿ std::optional<ಚಿಹ್ನೆರಹಿತ>(BAItem.Int_ಮೌಲ್ಯ);
      }
  }
  ಹಿಂತಿರುಗಿ std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್) ಸ್ಥಿರಾಂಕ {
  ಪ್ರತಿಜ್ಞಾ(
      0 &&
      "use getAttributeValue overloaded version accepting Stringref, unsigned");
  ಹಿಂತಿರುಗಿ std::nullopt;
}

std::optional<StringRef>
ELFExtendedAttrParser::getAttributeString(StringRef BuildAttrSubsectionName,
                                          ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್) ಸ್ಥಿರಾಂಕ {
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &SubSection : SubSectionVec) {
    ಆದರೆ (BuildAttrSubsectionName == SubSection.Name)
      ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &BAItem : SubSection.Content) {
        ಆದರೆ (ಟ್ಯಾಗ್ == BAItem.ಟ್ಯಾಗ್)
          ಹಿಂತಿರುಗಿ std::optional<StringRef>(BAItem.String_ಮೌಲ್ಯ);
      }
  }
  ಹಿಂತಿರುಗಿ std::nullopt;
}

StringRef
ELFExtendedAttrParser::get_ಟ್ಯಾಗ್_Name(ಸ್ಥಿರಾಂಕ StringRef &BuildAttrSubsectionName,
                                  ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಟ್ಯಾಗ್) {
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &ನಮೂದು : ಟ್ಯಾಗುಗಳು_ಹೆಸರುಗಳು_Map) {
    ಆದರೆ (BuildAttrSubsectionName == ನಮೂದು.SubsectionName)
      ಆದರೆ (ಟ್ಯಾಗ್ == ನಮೂದು.ಟ್ಯಾಗ್)
        ಹಿಂತಿರುಗಿ ನಮೂದು.ಟ್ಯಾಗ್_Name_2;
  }
  ಹಿಂತಿರುಗಿ "";
}

Error ELFExtendedAttrParser::parse(ArrayRef<uint8_t> Section,
                                   llvm::endianness Endian) {

  ಚಿಹ್ನೆರಹಿತ Section_ಸಂಖ್ಯೆ = 0;
  De = ದತ್ತಾಂಶ_Extractor(Section, Endian == llvm::endianness::little);

  // Early returns have specific errors. Consume the Error in Cursor.
  ರಚನೆ ClearCursorError {
    ದತ್ತಾಂಶ_Extractor::Cursor &Cursor;
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
  uint8_t Format_ಆವೃತ್ತಿ_2 = De.getU8(Cursor);
  ಆದರೆ (!Cursor)
    ಹಿಂತಿರುಗಿ Cursor.takeError();
  ಆದರೆ (ELFAttrs::Format_ಆವೃತ್ತಿ != Format_ಆವೃತ್ತಿ_2)
    ಹಿಂತಿರುಗಿ createStringError(errc::invalid_ವಾದ,
                             "unrecognized format-version: 0x" +
                                 utohexstr(Format_ಆವೃತ್ತಿ_2));

  ಆಗ (!De.eof(Cursor)) {
    uint32_t Ext_BASubsection_ಉದ್ದಳತೆ = De.getU32(Cursor);
    ಆದರೆ (!Cursor)
      ಹಿಂತಿರುಗಿ Cursor.takeError();
    // Minimal valid Extended Build Attributes subsection size is at
    // least 8: length(4) name(at least a single char + null) optionality(1) and
    // type(1)
    // Extended Build Attributes subsection has to fit inside the section.
    ಆದರೆ (Ext_BASubsection_ಉದ್ದಳತೆ < 8 ||
        Ext_BASubsection_ಉದ್ದಳತೆ > (Section.ಗಾತ್ರ_3() - Cursor.tell() + 4))
      ಹಿಂತಿರುಗಿ createStringError(
          errc::invalid_ವಾದ,
          "invalid Extended Build Attributes subsection size at offset: " +
              utohexstr(Cursor.tell() - 4));

    StringRef VendorName = De.getCStrRef(Cursor);
    ಆದರೆ (!Cursor)
      ಹಿಂತಿರುಗಿ Cursor.takeError();
    uint8_t IsOptional = De.getU8(Cursor);
    ಆದರೆ (!Cursor)
      ಹಿಂತಿರುಗಿ Cursor.takeError();
    ಆದರೆ (!(0 == IsOptional || 1 == IsOptional))
      ಹಿಂತಿರುಗಿ createStringError(
          errc::invalid_ವಾದ,
          "\ninvalid Optionality at offset " + utohexstr(Cursor.tell() - 4) +
              ": " + utohexstr(IsOptional) + " (Options are 1|0)");
    StringRef IsOptionalStr = IsOptional ? "optional" : "required";
    uint8_t Type = De.getU8(Cursor);
    ಆದರೆ (!Cursor)
      ಹಿಂತಿರುಗಿ Cursor.takeError();
    ಆದರೆ (!(0 == Type || 1 == Type))
      ಹಿಂತಿರುಗಿ createStringError(errc::invalid_ವಾದ,
                               "\ninvalid Type at offset " +
                                   utohexstr(Cursor.tell() - 4) + ": " +
                                   utohexstr(Type) + " (Options are 1|0)");
    StringRef TypeStr = Type ? "ntbs" : "uleb128";

    BuildAttributeSubSection BASubSection;
    BASubSection.Name = VendorName;
    BASubSection.IsOptional = IsOptional;
    BASubSection.ParameterType = Type;

    ಆದರೆ (Sw) {
      Sw->ಆರಂಭ_ಸಾಲು() << "Section " << ++Section_ಸಂಖ್ಯೆ << " {\n";
      Sw->indent();
      Sw->print_ಸಂಖ್ಯೆ("SectionLength", Ext_BASubsection_ಉದ್ದಳತೆ);
      Sw->ಆರಂಭ_ಸಾಲು() << "VendorName" << ": " << VendorName
                      << " Optionality: " << IsOptionalStr
                      << " Type: " << TypeStr << "\n";
      Sw->ಆರಂಭ_ಸಾಲು() << "Attributes {\n";
      Sw->indent();
    }

    // Offset in Section
    uint64_t ಸ್ಥಾನಾಂತರ_ಒಳ_Section = Cursor.tell();
    // Size: 4 bytes, Vendor Name: VendorName.size() + 1 (null termination),
    // optionality: 1, type: 1
    uint32_t ಬೈಟುಗಳು_All_But_ಗುಣಲಕ್ಷಣಗಳು = 4 + (VendorName.ಗಾತ್ರ_3() + 1) + 1 + 1;
    ಆಗ (Cursor.tell() <
           (ಸ್ಥಾನಾಂತರ_ಒಳ_Section + Ext_BASubsection_ಉದ್ದಳತೆ - ಬೈಟುಗಳು_All_But_ಗುಣಲಕ್ಷಣಗಳು)) {

      uint64_t ಟ್ಯಾಗ್ = De.getULEB128(Cursor);
      ಆದರೆ (!Cursor)
        ಹಿಂತಿರುಗಿ Cursor.takeError();

      StringRef ಟ್ಯಾಗ್_Name_2 = get_ಟ್ಯಾಗ್_Name(VendorName, ಟ್ಯಾಗ್);

      uint64_t ಮೌಲ್ಯ_Int = 0;
      std::string ಮೌಲ್ಯ_Str = "";
      ಆದರೆ (Type) { // type==1 --> ntbs
        ಮೌಲ್ಯ_Str = De.getCStrRef(Cursor);
        ಆದರೆ (!Cursor)
          ಹಿಂತಿರುಗಿ Cursor.takeError();
        ಆದರೆ (Sw)
          Sw->printString("" != ಟ್ಯಾಗ್_Name_2 ? ಟ್ಯಾಗ್_Name_2 : utostr(ಟ್ಯಾಗ್), ಮೌಲ್ಯ_Str);
      } ಇಲ್ಲದಿದ್ದರೆ { // type==0 --> uleb128
        ಮೌಲ್ಯ_Int = De.getULEB128(Cursor);
        ಆದರೆ (!Cursor)
          ಹಿಂತಿರುಗಿ Cursor.takeError();
        ಆದರೆ (Sw)
          Sw->print_ಸಂಖ್ಯೆ("" != ಟ್ಯಾಗ್_Name_2 ? ಟ್ಯಾಗ್_Name_2 : utostr(ಟ್ಯಾಗ್), ಮೌಲ್ಯ_Int);
      }

      // populate data structure
      Build_Attribute_ಅಂಶ BAItem(static_cast<Build_Attribute_ಅಂಶ::Types>(Type),
                                ಟ್ಯಾಗ್, ಮೌಲ್ಯ_Int, ಮೌಲ್ಯ_Str);
      BASubSection.Content.ತಳ್ಳು_ಹಿಂಭಾಗ(BAItem);
    }
    ಆದರೆ (Sw) {
      // Close 'Attributes'
      Sw->unindent();
      Sw->ಆರಂಭ_ಸಾಲು() << "}\n";
      // Close 'Section'
      Sw->unindent();
      Sw->ಆರಂಭ_ಸಾಲು() << "}\n";
    }

    // populate data structure
    SubSectionVec.ತಳ್ಳು_ಹಿಂಭಾಗ(BASubSection);
  }

  ಹಿಂತಿರುಗಿ Cursor.takeError();
}
