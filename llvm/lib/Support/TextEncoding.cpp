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

#ಯದಿ HAVE_ICU
#ಯದಿ HAVE_WINDOWS_ICU
#ಅನ್ತರ್ಭಾವ <icu.h>
#ಅನ್ಯಥಾ
#ಅನ್ತರ್ಭಾವ <unicode/ucnv.h>
#endif
#ನೋಚೇದ್ಯದಿ HAVE_ICONV
#ಅನ್ತರ್ಭಾವ <iconv.h>
#endif

using ನಾಮಾಕಾಶ llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
ಸ್ಥಿರ ಶೂನ್ಯ normalizeCharSetName(StringRef CSName,
                                 SmallVectorImpl<ವರ್ಣ> &Normalized) {
  ತಾರ್ಕಿಕ PrevDigit = ಅಸತ್ಯ;
  ಪ್ರತಿ (auto Ch : CSName) {
    ಯದಿ (isAlnum(Ch)) {
      Ch = toLower(Ch);
      ಯದಿ (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
        Normalized.ತಳ್ಳು_back(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
ಸ್ಥಿರ std::optional<TextEncoding> getKnownEncoding(StringRef Name) {
  SmallString<16> Normalized;
  normalizeCharSetName(Name, Normalized);
  ಯದಿ (Normalized.equals("utf8"))
    ಪ್ರತ್ಯಾಗಮ TextEncoding::UTF8;
  ಯದಿ (Normalized.equals("ibm1047"))
    ಪ್ರತ್ಯಾಗಮ TextEncoding::IBM1047;
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}

[[maybe_unused]] ಸ್ಥಿರ ಶೂನ್ಯ HandleOverflow(size_t &Capacity, ವರ್ಣ *&Output,
                                            size_t &OutputLength,
                                            SmallVectorImpl<ವರ್ಣ> &Result) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  Capacity =
      (Capacity < Result.max_size() / 2) ? 2 * Capacity : Result.max_size();
  Result.resize(0);
  Result.resize_for_overwrite(Capacity);
  Output = static_cast<ವರ್ಣ *>(Result.data());
  OutputLength = Capacity;
}

ನಾಮಾಕಾಶ {
ಗಣನಾ ConversionType {
  UTF8ToIBM1047,
  IBM1047ToUTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
ವರ್ಗ TextEncodingConverterTable final
    : ಸಾರ್ವಜನಿಕ details::TextEncodingConverterImplBase {
  ಧ್ರುವ ConversionType ConvType;

ಸಾರ್ವಜನಿಕ:
  TextEncodingConverterTable(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<ವರ್ಣ> &Result) override;

  ಶೂನ್ಯ reset() override {}
};

std::error_code
TextEncodingConverterTable::convertString(StringRef Source,
                                          SmallVectorImpl<ವರ್ಣ> &Result) {
  ವಿಕಲ್ಪ (ConvType) {
  ಪಕ್ಷ IBM1047ToUTF8:
    ConverterEBCDIC::convertToUTF8(Source, Result);
    ಪ್ರತ್ಯಾಗಮ std::error_code();
  ಪಕ್ಷ UTF8ToIBM1047:
    ಪ್ರತ್ಯಾಗಮ ConverterEBCDIC::convertToEBCDIC(Source, Result);
  }
  llvm_unreachable("Invalid ConvType!");
  ಪ್ರತ್ಯಾಗಮ std::error_code();
}

#ಯದಿ HAVE_ICU
ಸಂರಚನಾ UConverterDeleter {
  ಶೂನ್ಯ operator()(UConverter *Converter) ಧ್ರುವ {
    ಯದಿ (Converter)
      ucnv_close(Converter);
  }
};
using UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

ವರ್ಗ TextEncodingConverterICU final
    : ಸಾರ್ವಜನಿಕ details::TextEncodingConverterImplBase {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr ToConvDesc;

ಸಾರ್ವಜನಿಕ:
  TextEncodingConverterICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr ToConverter)
      : FromConvDesc(std::move(FromConverter)),
        ToConvDesc(std::move(ToConverter)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<ವರ್ಣ> &Result) override;

  ಶೂನ್ಯ reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_code
TextEncodingConverterICU::convertString(StringRef Source,
                                        SmallVectorImpl<ವರ್ಣ> &Result) {
  // Setup the input in case it has no backing data.
  size_t ಒಳಹರಿವು_Length = Source.size();
  ಧ್ರುವ ವರ್ಣ *In = ಒಳಹರಿವು_Length ? const_cast<ವರ್ಣ *>(Source.data()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);
  ವರ್ಣ *Output;
  UErrorCode EC = U_ZERO_ERROR;

  ucnv_setToUCallBack(&*FromConvDesc, UCNV_TO_U_CALLBACK_STOP, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ,
                      &EC);
  ucnv_setFromUCallBack(&*ToConvDesc, UCNV_FROM_U_CALLBACK_STOP, ರಿಕ್ತ, ರಿಕ್ತ,
                        ರಿಕ್ತ, &EC);
  ಪ್ರತಿಜ್ಞಾ(U_SUCCESS(EC));

  ಕುರು {
    EC = U_ZERO_ERROR;
    ಧ್ರುವ ವರ್ಣ *ಒಳಹರಿವು = In;

    Output = static_cast<ವರ್ಣ *>(Result.data());
    ucnv_convertEx(&*ToConvDesc, &*FromConvDesc, &Output, Result.end(), &ಒಳಹರಿವು,
                   In + ಒಳಹರಿವು_Length, /*pivotStart=*/ರಿಕ್ತ,
                   /*pivotSource=*/ರಿಕ್ತ, /*pivotTarget=*/ರಿಕ್ತ,
                   /*pivotLimit=*/ರಿಕ್ತ, /*reset=*/ಸತ್ಯ,
                   /*flush=*/ಸತ್ಯ, &EC);
    ಯದಿ (U_FAILURE(EC)) {
      ಯದಿ (EC == U_BUFFER_OVERFLOW_ERROR) {
        ಯದಿ (Capacity < Result.max_size()) {
          HandleOverflow(Capacity, Output, OutputLength, Result);
          ಅನುವರ್ತ;
        } ಅನ್ಯಥಾ {
          Result.resize(Output - Result.data());
          ಪ್ರತ್ಯಾಗಮ std::error_code(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      Result.resize(Output - Result.data());
      ಪ್ರತ್ಯಾಗಮ std::error_code(EILSEQ, std::generic_category());
    }
    ಭಙ್ಗ;
  } ಯಾವತ್ (ಸತ್ಯ);

  Result.resize(Output - Result.data());
  ಪ್ರತ್ಯಾಗಮ std::error_code();
}

ಶೂನ್ಯ TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ToConvDesc);
}

#ನೋಚೇದ್ಯದಿ HAVE_ICONV
ವರ್ಗ TextEncodingConverterIconv final
    : ಸಾರ್ವಜನಿಕ details::TextEncodingConverterImplBase {
  ವರ್ಗ UniqueIconvT {
    iconv_t ConvDesc;

  ಸಾರ್ವಜನಿಕ:
    operator iconv_t() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      ಯದಿ (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &operator=(UniqueIconvT &&Other) {
      ಯದಿ (&Other != ಏತತ್) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
    }
  };
  UniqueIconvT ConvDesc;

ಸಾರ್ವಜನಿಕ:
  TextEncodingConverterIconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<ವರ್ಣ> &Result) override;

  ಶೂನ್ಯ reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_code
TextEncodingConverterIconv::convertString(StringRef Source,
                                          SmallVectorImpl<ವರ್ಣ> &Result) {
  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  ವರ್ಣ *Output = static_cast<ವರ್ಣ *>(Result.data());
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);

  size_t Ret;
  // Handle errors returned from iconv().
  auto HandleError = [&Capacity, &Output, &OutputLength, &Result,
                      ಏತತ್](size_t Ret) {
    ಯದಿ (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      ಯದಿ (errno == E2BIG && Capacity < Result.max_size()) {
        HandleOverflow(Capacity, Output, OutputLength, Result);
        // Reset converter
        reset();
        ಪ್ರತ್ಯಾಗಮ std::error_code();
      } ಅನ್ಯಥಾ {
        // Some other error occurred.
        Result.resize(Output - Result.data());
        ಪ್ರತ್ಯಾಗಮ std::error_code(errno, std::generic_category());
      }
    } ಅನ್ಯಥಾ {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      ಪ್ರತ್ಯಾಗಮ std::make_error_code(std::errc::illegal_byte_sequence);
    }
  };

  ಕುರು {
    size_t ಒಳಹರಿವು_Length = Source.size();
    ವರ್ಣ *ಒಳಹರಿವು = const_cast<ವರ್ಣ *>(Source.data());
    Ret = iconv(ConvDesc, &ಒಳಹರಿವು, &ಒಳಹರಿವು_Length, &Output, &OutputLength);
    ಯದಿ (Ret != 0) {
      ಯದಿ (auto EC = HandleError(Ret))
        ಪ್ರತ್ಯಾಗಮ EC;
      ಅನುವರ್ತ;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, nullptr, nullptr, &Output, &OutputLength);
    ಯದಿ (Ret != 0) {
      ಯದಿ (auto EC = HandleError(Ret))
        ಪ್ರತ್ಯಾಗಮ EC;
      ಅನುವರ್ತ;
    }
    ಭಙ್ಗ;
  } ಯಾವತ್ (ಸತ್ಯ);

  // Re-adjust size to actual size.
  Result.resize(Output - Result.data());
  ಪ್ರತ್ಯಾಗಮ std::error_code();
}

ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, nullptr, nullptr, nullptr, nullptr);
}

#endif // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  ಯದಿ (CPFrom == CPTo)
    ಪ್ರತ್ಯಾಗಮ std::make_error_code(std::errc::invalid_argument);

  ConversionType Conversion;
  ಯದಿ (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8ToIBM1047;
  ಅನ್ಯಥಾ ಯದಿ (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047ToUTF8;
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ std::make_error_code(std::errc::invalid_argument);

  ಪ್ರತ್ಯಾಗಮ TextEncodingConverter(
      std::make_unique<TextEncodingConverterTable>(Conversion));
}

ErrorOr<TextEncodingConverter> TextEncodingConverter::create(StringRef From,
                                                             StringRef To) {
  std::optional<TextEncoding> FromEncoding = getKnownEncoding(From);
  std::optional<TextEncoding> ToEncoding = getKnownEncoding(To);
  ಯದಿ (FromEncoding && ToEncoding) {
    ErrorOr<TextEncodingConverter> Converter =
        create(*FromEncoding, *ToEncoding);
    ಯದಿ (Converter)
      ಪ್ರತ್ಯಾಗಮ Converter;
  }
#ಯದಿ HAVE_ICU
  UErrorCode EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  ಯದಿ (U_FAILURE(EC))
    ಪ್ರತ್ಯಾಗಮ std::make_error_code(std::errc::invalid_argument);

  UConverterUniquePtr ToConvDesc(ucnv_open(To.str().c_str(), &EC));
  ಯದಿ (U_FAILURE(EC))
    ಪ್ರತ್ಯಾಗಮ std::make_error_code(std::errc::invalid_argument);

  auto Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(ToConvDesc));
  ಪ್ರತ್ಯಾಗಮ TextEncodingConverter(std::move(Converter));
#ನೋಚೇದ್ಯದಿ HAVE_ICONV
  iconv_t ConvDesc = iconv_open(To.str().c_str(), From.str().c_str());
  ಯದಿ (ConvDesc == (iconv_t)-1)
    ಪ್ರತ್ಯಾಗಮ std::make_error_code(std::errc::invalid_argument);
  ಪ್ರತ್ಯಾಗಮ TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#ಅನ್ಯಥಾ
  ಪ್ರತ್ಯಾಗಮ std::make_error_code(std::errc::invalid_argument);
#endif
}
