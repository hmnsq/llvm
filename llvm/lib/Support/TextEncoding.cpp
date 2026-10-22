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

#ضمن "llvm/Support/TextEncoding.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/ConvertEBCDIC.h"
#ضمن <system_error>

#ܐܢ HAVE_ICU
#ܐܢ HAVE_WINDOWS_ICU
#ضمن <icu.h>
#ܐܚܪܢܐ
#ضمن <unicode/ucnv.h>
#endif
#وإلاإذا HAVE_ICONV
#ضمن <iconv.h>
#endif

using نطاق llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
ساكن فراغ normalizeCharSetName(StringRef CSName,
                                 SmallVectorImpl<حرف> &Normalized) {
  منطقي PrevDigit = ܕܓܠܐ;
  لكل (auto Ch : CSName) {
    ܐܢ (isAlnum(Ch)) {
      Ch = toLower(Ch);
      ܐܢ (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
        Normalized.push_back(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
ساكن std::optional<TextEncoding> getKnownEncoding(StringRef Name) {
  SmallString<16> Normalized;
  normalizeCharSetName(Name, Normalized);
  ܐܢ (Normalized.equals("utf8"))
    ܦܢܐ TextEncoding::UTF8;
  ܐܢ (Normalized.equals("ibm1047"))
    ܦܢܐ TextEncoding::IBM1047;
  ܦܢܐ std::nullopt;
}

[[maybe_unused]] ساكن فراغ HandleOverflow(size_t &Capacity, حرف *&Output,
                                            size_t &OutputLength,
                                            SmallVectorImpl<حرف> &Result) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  Capacity =
      (Capacity < Result.max_size() / 2) ? 2 * Capacity : Result.max_size();
  Result.resize(0);
  Result.resize_for_overwrite(Capacity);
  Output = static_cast<حرف *>(Result.data());
  OutputLength = Capacity;
}

نطاق {
تعداد ConversionType {
  UTF8ToIBM1047,
  IBM1047ToUTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
صنف TextEncodingConverterTable final
    : عام details::TextEncodingConverterImplBase {
  ثابت ConversionType ConvType;

عام:
  TextEncodingConverterTable(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<حرف> &Result) override;

  فراغ reset() override {}
};

std::error_code
TextEncodingConverterTable::convertString(StringRef Source,
                                          SmallVectorImpl<حرف> &Result) {
  بدل (ConvType) {
  حالة IBM1047ToUTF8:
    ConverterEBCDIC::convertToUTF8(Source, Result);
    ܦܢܐ std::error_code();
  حالة UTF8ToIBM1047:
    ܦܢܐ ConverterEBCDIC::convertToEBCDIC(Source, Result);
  }
  llvm_unreachable("Invalid ConvType!");
  ܦܢܐ std::error_code();
}

#ܐܢ HAVE_ICU
بنية UConverterDeleter {
  فراغ operator()(UConverter *Converter) ثابت {
    ܐܢ (Converter)
      ucnv_close(Converter);
  }
};
using UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

صنف TextEncodingConverterICU final
    : عام details::TextEncodingConverterImplBase {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr ToConvDesc;

عام:
  TextEncodingConverterICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr ToConverter)
      : FromConvDesc(std::move(FromConverter)),
        ToConvDesc(std::move(ToConverter)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<حرف> &Result) override;

  فراغ reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_code
TextEncodingConverterICU::convertString(StringRef Source,
                                        SmallVectorImpl<حرف> &Result) {
  // Setup the input in case it has no backing data.
  size_t InputLength = Source.size();
  ثابت حرف *In = InputLength ? const_cast<حرف *>(Source.data()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);
  حرف *Output;
  UErrorCode EC = U_ZERO_ERROR;

  ucnv_setToUCallBack(&*FromConvDesc, UCNV_TO_U_CALLBACK_STOP, عدم, عدم, عدم,
                      &EC);
  ucnv_setFromUCallBack(&*ToConvDesc, UCNV_FROM_U_CALLBACK_STOP, عدم, عدم,
                        عدم, &EC);
  أكد(U_SUCCESS(EC));

  ܥܒܕ {
    EC = U_ZERO_ERROR;
    ثابت حرف *Input = In;

    Output = static_cast<حرف *>(Result.data());
    ucnv_convertEx(&*ToConvDesc, &*FromConvDesc, &Output, Result.end(), &Input,
                   In + InputLength, /*pivotStart=*/عدم,
                   /*pivotSource=*/عدم, /*pivotTarget=*/عدم,
                   /*pivotLimit=*/عدم, /*reset=*/ܫܪܪܐ,
                   /*flush=*/ܫܪܪܐ, &EC);
    ܐܢ (U_FAILURE(EC)) {
      ܐܢ (EC == U_BUFFER_OVERFLOW_ERROR) {
        ܐܢ (Capacity < Result.max_size()) {
          HandleOverflow(Capacity, Output, OutputLength, Result);
          استمر;
        } ܐܚܪܢܐ {
          Result.resize(Output - Result.data());
          ܦܢܐ std::error_code(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      Result.resize(Output - Result.data());
      ܦܢܐ std::error_code(EILSEQ, std::generic_category());
    }
    اكسر;
  } ܟܡܐ (ܫܪܪܐ);

  Result.resize(Output - Result.data());
  ܦܢܐ std::error_code();
}

فراغ TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ToConvDesc);
}

#وإلاإذا HAVE_ICONV
صنف TextEncodingConverterIconv final
    : عام details::TextEncodingConverterImplBase {
  صنف UniqueIconvT {
    iconv_t ConvDesc;

  عام:
    operator iconv_t() ثابت { ܦܢܐ ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      ܐܢ (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &operator=(UniqueIconvT &&Other) {
      ܐܢ (&Other != هذا) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      ܦܢܐ *هذا;
    }
  };
  UniqueIconvT ConvDesc;

عام:
  TextEncodingConverterIconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<حرف> &Result) override;

  فراغ reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_code
TextEncodingConverterIconv::convertString(StringRef Source,
                                          SmallVectorImpl<حرف> &Result) {
  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  حرف *Output = static_cast<حرف *>(Result.data());
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);

  size_t Ret;
  // Handle errors returned from iconv().
  auto HandleError = [&Capacity, &Output, &OutputLength, &Result,
                      هذا](size_t Ret) {
    ܐܢ (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      ܐܢ (errno == E2BIG && Capacity < Result.max_size()) {
        HandleOverflow(Capacity, Output, OutputLength, Result);
        // Reset converter
        reset();
        ܦܢܐ std::error_code();
      } ܐܚܪܢܐ {
        // Some other error occurred.
        Result.resize(Output - Result.data());
        ܦܢܐ std::error_code(errno, std::generic_category());
      }
    } ܐܚܪܢܐ {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      ܦܢܐ std::make_error_code(std::errc::illegal_byte_sequence);
    }
  };

  ܥܒܕ {
    size_t InputLength = Source.size();
    حرف *Input = const_cast<حرف *>(Source.data());
    Ret = iconv(ConvDesc, &Input, &InputLength, &Output, &OutputLength);
    ܐܢ (Ret != 0) {
      ܐܢ (auto EC = HandleError(Ret))
        ܦܢܐ EC;
      استمر;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, nullptr, nullptr, &Output, &OutputLength);
    ܐܢ (Ret != 0) {
      ܐܢ (auto EC = HandleError(Ret))
        ܦܢܐ EC;
      استمر;
    }
    اكسر;
  } ܟܡܐ (ܫܪܪܐ);

  // Re-adjust size to actual size.
  Result.resize(Output - Result.data());
  ܦܢܐ std::error_code();
}

مضمن فراغ TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, nullptr, nullptr, nullptr, nullptr);
}

#endif // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  ܐܢ (CPFrom == CPTo)
    ܦܢܐ std::make_error_code(std::errc::invalid_argument);

  ConversionType Conversion;
  ܐܢ (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8ToIBM1047;
  ܐܚܪܢܐ ܐܢ (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047ToUTF8;
  ܐܚܪܢܐ
    ܦܢܐ std::make_error_code(std::errc::invalid_argument);

  ܦܢܐ TextEncodingConverter(
      std::make_unique<TextEncodingConverterTable>(Conversion));
}

ErrorOr<TextEncodingConverter> TextEncodingConverter::create(StringRef From,
                                                             StringRef To) {
  std::optional<TextEncoding> FromEncoding = getKnownEncoding(From);
  std::optional<TextEncoding> ToEncoding = getKnownEncoding(To);
  ܐܢ (FromEncoding && ToEncoding) {
    ErrorOr<TextEncodingConverter> Converter =
        create(*FromEncoding, *ToEncoding);
    ܐܢ (Converter)
      ܦܢܐ Converter;
  }
#ܐܢ HAVE_ICU
  UErrorCode EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  ܐܢ (U_FAILURE(EC))
    ܦܢܐ std::make_error_code(std::errc::invalid_argument);

  UConverterUniquePtr ToConvDesc(ucnv_open(To.str().c_str(), &EC));
  ܐܢ (U_FAILURE(EC))
    ܦܢܐ std::make_error_code(std::errc::invalid_argument);

  auto Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(ToConvDesc));
  ܦܢܐ TextEncodingConverter(std::move(Converter));
#وإلاإذا HAVE_ICONV
  iconv_t ConvDesc = iconv_open(To.str().c_str(), From.str().c_str());
  ܐܢ (ConvDesc == (iconv_t)-1)
    ܦܢܐ std::make_error_code(std::errc::invalid_argument);
  ܦܢܐ TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#ܐܚܪܢܐ
  ܦܢܐ std::make_error_code(std::errc::invalid_argument);
#endif
}
