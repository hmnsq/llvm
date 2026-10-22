//===-- TextEncoding.cpp - Text encoding conversion class ---------*- C++ -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file provides utility classes to convert between different character
/// encodings.
///
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/TextEncoding.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ConvertEBCDIC.h"
#ಅನ್ತರ್ಭಾವ <system_error>

#ಆದರೆ HAVE_ICU
#ಆದರೆ HAVE_WINDOWS_ICU
#ಅನ್ತರ್ಭಾವ <icu.h>
#ಇಲ್ಲದಿದ್ದರೆ
#ಅನ್ತರ್ಭಾವ <unicode/ucnv.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅಥವಾಆದರೆ HAVE_ICONV
#ಅನ್ತರ್ಭಾವ <iconv.h>
#ಆದರೆ_ಅಂತ್ಯ

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
ಸ್ಥಿರ ಶೂನ್ಯ normalize_Char_ಗಣ_Name(StringRef CSName,
                                 Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &Normalized) {
  ತಾರ್ಕಿಕ PrevDigit = ಸುಳ್ಳು;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ Ch : CSName) {
    ಆದರೆ (isAlnum(Ch)) {
      Ch = ಗಮ್ಯ_ಅಧೋ(Ch);
      ಆದರೆ (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
        Normalized.ತಳ್ಳು_ಹಿಂಭಾಗ(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
ಸ್ಥಿರ std::optional<ಪಠ್ಯ_Encoding> get_ತಿಳಿದ_Encoding(StringRef Name) {
  SmallString<16> Normalized;
  normalize_Char_ಗಣ_Name(Name, Normalized);
  ಆದರೆ (Normalized.equals("utf8"))
    ಹಿಂತಿರುಗಿ ಪಠ್ಯ_Encoding::UTF8;
  ಆದರೆ (Normalized.equals("ibm1047"))
    ಹಿಂತಿರುಗಿ ಪಠ್ಯ_Encoding::IBM1047;
  ಹಿಂತಿರುಗಿ std::nullopt;
}

[[maybe_ಬಳಸದ]] ಸ್ಥಿರ ಶೂನ್ಯ Handle_ಅತಿಪ್ರವಾಹ(size_t &ಸಾಮರ್ಥ್ಯ, ಅಕ್ಷರ *&Output,
                                            size_t &Output_ಉದ್ದಳತೆ,
                                            Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  ಸಾಮರ್ಥ್ಯ =
      (ಸಾಮರ್ಥ್ಯ < ಫಲಿತಾಂಶ.max_ಗಾತ್ರ() / 2) ? 2 * ಸಾಮರ್ಥ್ಯ : ಫಲಿತಾಂಶ.max_ಗಾತ್ರ();
  ಫಲಿತಾಂಶ.resize(0);
  ಫಲಿತಾಂಶ.resize_for_overwrite(ಸಾಮರ್ಥ್ಯ);
  Output = static_cast<ಅಕ್ಷರ *>(ಫಲಿತಾಂಶ.ದತ್ತಾಂಶ());
  Output_ಉದ್ದಳತೆ = ಸಾಮರ್ಥ್ಯ;
}

ನಾಮಾಕಾಶ {
ಎಣಿಕೆ ConversionType {
  UTF8_ಗಮ್ಯ_IBM1047,
  IBM1047_ಗಮ್ಯ_UTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
ವರ್ಗ ಪಠ್ಯ_Encoding_Converter_ಕೋಷ್ಟಕ final
    : ಸಾರ್ವಜನಿಕ details::ಪಠ್ಯ_Encoding_Converter_Impl_ತಳ {
  ಸ್ಥಿರಾಂಕ ConversionType ConvType;

ಸಾರ್ವಜನಿಕ:
  ಪಠ್ಯ_Encoding_Converter_ಕೋಷ್ಟಕ(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_ಸಂಕೇತ convertString(StringRef ಮೂಲ,
                                Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ) override;

  ಶೂನ್ಯ reset() override {}
};

std::error_ಸಂಕೇತ
ಪಠ್ಯ_Encoding_Converter_ಕೋಷ್ಟಕ::convertString(StringRef ಮೂಲ,
                                          Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ) {
  ಆಯ್ಕೆ (ConvType) {
  ಸ್ಥಿತಿ IBM1047_ಗಮ್ಯ_UTF8:
    ConverterEBCDIC::convert_ಗಮ್ಯ_UTF8(ಮೂಲ, ಫಲಿತಾಂಶ);
    ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
  ಸ್ಥಿತಿ UTF8_ಗಮ್ಯ_IBM1047:
    ಹಿಂತಿರುಗಿ ConverterEBCDIC::convert_ಗಮ್ಯ_EBCDIC(ಮೂಲ, ಫಲಿತಾಂಶ);
  }
  llvm_unreachable("Invalid ConvType!");
  ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
}

#ಆದರೆ HAVE_ICU
ರಚನೆ UConverterDeleter {
  ಶೂನ್ಯ ಕ್ರಿಯಾಕಾರಕ()(UConverter *Converter) ಸ್ಥಿರಾಂಕ {
    ಆದರೆ (Converter)
      ucnv_close(Converter);
  }
};
ಬಳಸಿ UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

ವರ್ಗ ಪಠ್ಯ_Encoding_Converter_ICU final
    : ಸಾರ್ವಜನಿಕ details::ಪಠ್ಯ_Encoding_Converter_Impl_ತಳ {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr ಗಮ್ಯ_Conv_Desc;

ಸಾರ್ವಜನಿಕ:
  ಪಠ್ಯ_Encoding_Converter_ICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr ಗಮ್ಯ_Converter)
      : FromConvDesc(std::move(FromConverter)),
        ಗಮ್ಯ_Conv_Desc(std::move(ಗಮ್ಯ_Converter)) {}

  std::error_ಸಂಕೇತ convertString(StringRef ಮೂಲ,
                                Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ) override;

  ಶೂನ್ಯ reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_ಸಂಕೇತ
ಪಠ್ಯ_Encoding_Converter_ICU::convertString(StringRef ಮೂಲ,
                                        Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ) {
  // Setup the input in case it has no backing data.
  size_t ಒಳಹರಿವು_ಉದ್ದಳತೆ = ಮೂಲ.ಗಾತ್ರ_3();
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಒಳ_2 = ಒಳಹರಿವು_ಉದ್ದಳತೆ ? const_cast<ಅಕ್ಷರ *>(ಮೂಲ.ದತ್ತಾಂಶ()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t ಸಾಮರ್ಥ್ಯ = ಫಲಿತಾಂಶ.ಸಾಮರ್ಥ್ಯ_2();
  size_t Output_ಉದ್ದಳತೆ = ಸಾಮರ್ಥ್ಯ;
  ಫಲಿತಾಂಶ.resize_for_overwrite(ಸಾಮರ್ಥ್ಯ);
  ಅಕ್ಷರ *Output;
  UError_ಸಂಕೇತ EC = U_ZERO_ERROR;

  ucnv_ಗಣ_ಗಮ್ಯ_UCall_ಹಿಂಭಾಗ(&*FromConvDesc, UCNV_ಗಮ್ಯ_U_CALLBACK_STOP, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ,
                      &EC);
  ucnv_ಗಣ_From_UCall_ಹಿಂಭಾಗ(&*ಗಮ್ಯ_Conv_Desc, UCNV_FROM_U_CALLBACK_STOP, ಖಾಲಿ, ಖಾಲಿ,
                        ಖಾಲಿ, &EC);
  ಪ್ರತಿಜ್ಞಾ(U_SUCCESS(EC));

  ಮಾಡು {
    EC = U_ZERO_ERROR;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಒಳಹರಿವು = ಒಳ_2;

    Output = static_cast<ಅಕ್ಷರ *>(ಫಲಿತಾಂಶ.ದತ್ತಾಂಶ());
    ucnv_convertEx(&*ಗಮ್ಯ_Conv_Desc, &*FromConvDesc, &Output, ಫಲಿತಾಂಶ.ಕೊನೆ(), &ಒಳಹರಿವು,
                   ಒಳ_2 + ಒಳಹರಿವು_ಉದ್ದಳತೆ, /*pivotStart=*/ಖಾಲಿ,
                   /*pivotSource=*/ಖಾಲಿ, /*pivotTarget=*/ಖಾಲಿ,
                   /*pivotLimit=*/ಖಾಲಿ, /*reset=*/ನಿಜ,
                   /*flush=*/ನಿಜ, &EC);
    ಆದರೆ (U_FAILURE(EC)) {
      ಆದರೆ (EC == U_ಬಫರ್_ಅತಿಪ್ರವಾಹ_ERROR) {
        ಆದರೆ (ಸಾಮರ್ಥ್ಯ < ಫಲಿತಾಂಶ.max_ಗಾತ್ರ()) {
          Handle_ಅತಿಪ್ರವಾಹ(ಸಾಮರ್ಥ್ಯ, Output, Output_ಉದ್ದಳತೆ, ಫಲಿತಾಂಶ);
          ಮುಂದುವರಿ;
        } ಇಲ್ಲದಿದ್ದರೆ {
          ಫಲಿತಾಂಶ.resize(Output - ಫಲಿತಾಂಶ.ದತ್ತಾಂಶ());
          ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      ಫಲಿತಾಂಶ.resize(Output - ಫಲಿತಾಂಶ.ದತ್ತಾಂಶ());
      ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ(EILSEQ, std::generic_category());
    }
    ನಿಲ್ಲಿಸು;
  } ಆಗ (ನಿಜ);

  ಫಲಿತಾಂಶ.resize(Output - ಫಲಿತಾಂಶ.ದತ್ತಾಂಶ());
  ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
}

ಶೂನ್ಯ ಪಠ್ಯ_Encoding_Converter_ICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ಗಮ್ಯ_Conv_Desc);
}

#ಅಥವಾಆದರೆ HAVE_ICONV
ವರ್ಗ ಪಠ್ಯ_Encoding_Converter_Iconv final
    : ಸಾರ್ವಜನಿಕ details::ಪಠ್ಯ_Encoding_Converter_Impl_ತಳ {
  ವರ್ಗ UniqueIconvT {
    iconv_t ConvDesc;

  ಸಾರ್ವಜನಿಕ:
    ಕ್ರಿಯಾಕಾರಕ iconv_t() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      ಆದರೆ (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &ಕ್ರಿಯಾಕಾರಕ=(UniqueIconvT &&Other) {
      ಆದರೆ (&Other != ಇದು) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      ಹಿಂತಿರುಗಿ *ಇದು;
    }
  };
  UniqueIconvT ConvDesc;

ಸಾರ್ವಜನಿಕ:
  ಪಠ್ಯ_Encoding_Converter_Iconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_ಸಂಕೇತ convertString(StringRef ಮೂಲ,
                                Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ) override;

  ಶೂನ್ಯ reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_ಸಂಕೇತ
ಪಠ್ಯ_Encoding_Converter_Iconv::convertString(StringRef ಮೂಲ,
                                          Small_ಸದಿಶ_Impl<ಅಕ್ಷರ> &ಫಲಿತಾಂಶ) {
  // Setup the output. We directly write into the SmallVector.
  size_t ಸಾಮರ್ಥ್ಯ = ಫಲಿತಾಂಶ.ಸಾಮರ್ಥ್ಯ_2();
  ಅಕ್ಷರ *Output = static_cast<ಅಕ್ಷರ *>(ಫಲಿತಾಂಶ.ದತ್ತಾಂಶ());
  size_t Output_ಉದ್ದಳತೆ = ಸಾಮರ್ಥ್ಯ;
  ಫಲಿತಾಂಶ.resize_for_overwrite(ಸಾಮರ್ಥ್ಯ);

  size_t Ret;
  // Handle errors returned from iconv().
  ಸ್ವಯಂಚಾಲಿತ HandleError = [&ಸಾಮರ್ಥ್ಯ, &Output, &Output_ಉದ್ದಳತೆ, &ಫಲಿತಾಂಶ,
                      ಇದು](size_t Ret) {
    ಆದರೆ (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      ಆದರೆ (errno == E2BIG && ಸಾಮರ್ಥ್ಯ < ಫಲಿತಾಂಶ.max_ಗಾತ್ರ()) {
        Handle_ಅತಿಪ್ರವಾಹ(ಸಾಮರ್ಥ್ಯ, Output, Output_ಉದ್ದಳತೆ, ಫಲಿತಾಂಶ);
        // Reset converter
        reset();
        ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
      } ಇಲ್ಲದಿದ್ದರೆ {
        // Some other error occurred.
        ಫಲಿತಾಂಶ.resize(Output - ಫಲಿತಾಂಶ.ದತ್ತಾಂಶ());
        ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ(errno, std::generic_category());
      }
    } ಇಲ್ಲದಿದ್ದರೆ {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      ಹಿಂತಿರುಗಿ std::make_error_ಸಂಕೇತ(std::errc::illegal_byte_sequence);
    }
  };

  ಮಾಡು {
    size_t ಒಳಹರಿವು_ಉದ್ದಳತೆ = ಮೂಲ.ಗಾತ್ರ_3();
    ಅಕ್ಷರ *ಒಳಹರಿವು = const_cast<ಅಕ್ಷರ *>(ಮೂಲ.ದತ್ತಾಂಶ());
    Ret = iconv(ConvDesc, &ಒಳಹರಿವು, &ಒಳಹರಿವು_ಉದ್ದಳತೆ, &Output, &Output_ಉದ್ದಳತೆ);
    ಆದರೆ (Ret != 0) {
      ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = HandleError(Ret))
        ಹಿಂತಿರುಗಿ EC;
      ಮುಂದುವರಿ;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, ಶೂನ್ಯಸೂಚಕ, ಶೂನ್ಯಸೂಚಕ, &Output, &Output_ಉದ್ದಳತೆ);
    ಆದರೆ (Ret != 0) {
      ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = HandleError(Ret))
        ಹಿಂತಿರುಗಿ EC;
      ಮುಂದುವರಿ;
    }
    ನಿಲ್ಲಿಸು;
  } ಆಗ (ನಿಜ);

  // Re-adjust size to actual size.
  ಫಲಿತಾಂಶ.resize(Output - ಫಲಿತಾಂಶ.ದತ್ತಾಂಶ());
  ಹಿಂತಿರುಗಿ std::error_ಸಂಕೇತ();
}

ಸಾಲೊಳಗೆ ಶೂನ್ಯ ಪಠ್ಯ_Encoding_Converter_Iconv::reset() {
  iconv(ConvDesc, ಶೂನ್ಯಸೂಚಕ, ಶೂನ್ಯಸೂಚಕ, ಶೂನ್ಯಸೂಚಕ, ಶೂನ್ಯಸೂಚಕ);
}

#ಆದರೆ_ಅಂತ್ಯ // HAVE_ICONV
} // namespace

ErrorOr<ಪಠ್ಯ_Encoding_Converter>
ಪಠ್ಯ_Encoding_Converter::create(ಪಠ್ಯ_Encoding CPFrom, ಪಠ್ಯ_Encoding CPTo) {

  // Text encodings should be distinct.
  ಆದರೆ (CPFrom == CPTo)
    ಹಿಂತಿರುಗಿ std::make_error_ಸಂಕೇತ(std::errc::invalid_ವಾದ);

  ConversionType Conversion;
  ಆದರೆ (CPFrom == ಪಠ್ಯ_Encoding::UTF8 && CPTo == ಪಠ್ಯ_Encoding::IBM1047)
    Conversion = UTF8_ಗಮ್ಯ_IBM1047;
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (CPFrom == ಪಠ್ಯ_Encoding::IBM1047 && CPTo == ಪಠ್ಯ_Encoding::UTF8)
    Conversion = IBM1047_ಗಮ್ಯ_UTF8;
  ಇಲ್ಲದಿದ್ದರೆ
    ಹಿಂತಿರುಗಿ std::make_error_ಸಂಕೇತ(std::errc::invalid_ವಾದ);

  ಹಿಂತಿರುಗಿ ಪಠ್ಯ_Encoding_Converter(
      std::make_unique<ಪಠ್ಯ_Encoding_Converter_ಕೋಷ್ಟಕ>(Conversion));
}

ErrorOr<ಪಠ್ಯ_Encoding_Converter> ಪಠ್ಯ_Encoding_Converter::create(StringRef From,
                                                             StringRef ಗಮ್ಯ) {
  std::optional<ಪಠ್ಯ_Encoding> FromEncoding = get_ತಿಳಿದ_Encoding(From);
  std::optional<ಪಠ್ಯ_Encoding> ಗಮ್ಯ_Encoding = get_ತಿಳಿದ_Encoding(ಗಮ್ಯ);
  ಆದರೆ (FromEncoding && ಗಮ್ಯ_Encoding) {
    ErrorOr<ಪಠ್ಯ_Encoding_Converter> Converter =
        create(*FromEncoding, *ಗಮ್ಯ_Encoding);
    ಆದರೆ (Converter)
      ಹಿಂತಿರುಗಿ Converter;
  }
#ಆದರೆ HAVE_ICU
  UError_ಸಂಕೇತ EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  ಆದರೆ (U_FAILURE(EC))
    ಹಿಂತಿರುಗಿ std::make_error_ಸಂಕೇತ(std::errc::invalid_ವಾದ);

  UConverterUniquePtr ಗಮ್ಯ_Conv_Desc(ucnv_open(ಗಮ್ಯ.str().c_str(), &EC));
  ಆದರೆ (U_FAILURE(EC))
    ಹಿಂತಿರುಗಿ std::make_error_ಸಂಕೇತ(std::errc::invalid_ವಾದ);

  ಸ್ವಯಂಚಾಲಿತ Converter = std::make_unique<ಪಠ್ಯ_Encoding_Converter_ICU>(
      std::move(FromConvDesc), std::move(ಗಮ್ಯ_Conv_Desc));
  ಹಿಂತಿರುಗಿ ಪಠ್ಯ_Encoding_Converter(std::move(Converter));
#ಅಥವಾಆದರೆ HAVE_ICONV
  iconv_t ConvDesc = iconv_open(ಗಮ್ಯ.str().c_str(), From.str().c_str());
  ಆದರೆ (ConvDesc == (iconv_t)-1)
    ಹಿಂತಿರುಗಿ std::make_error_ಸಂಕೇತ(std::errc::invalid_ವಾದ);
  ಹಿಂತಿರುಗಿ ಪಠ್ಯ_Encoding_Converter(
      std::make_unique<ಪಠ್ಯ_Encoding_Converter_Iconv>(ConvDesc));
#ಇಲ್ಲದಿದ್ದರೆ
  ಹಿಂತಿರುಗಿ std::make_error_ಸಂಕೇತ(std::errc::invalid_ವಾದ);
#ಆದರೆ_ಅಂತ್ಯ
}
