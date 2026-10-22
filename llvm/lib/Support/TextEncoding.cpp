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

#אם HAVE_ICU
#אם HAVE_WINDOWS_ICU
#ضمن <icu.h>
#אחרת
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
  منطقي PrevDigit = שקר;
  עבור (auto Ch : CSName) {
    אם (isAlnum(Ch)) {
      Ch = toLower(Ch);
      אם (Ch != '0' || PrevDigit) {
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
  אם (Normalized.equals("utf8"))
    החזר TextEncoding::UTF8;
  אם (Normalized.equals("ibm1047"))
    החזר TextEncoding::IBM1047;
  החזר std::nullopt;
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
מחלקה TextEncodingConverterTable final
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
    החזר std::error_code();
  حالة UTF8ToIBM1047:
    החזר ConverterEBCDIC::convertToEBCDIC(Source, Result);
  }
  llvm_unreachable("Invalid ConvType!");
  החזר std::error_code();
}

#אם HAVE_ICU
بنية UConverterDeleter {
  فراغ operator()(UConverter *Converter) ثابت {
    אם (Converter)
      ucnv_close(Converter);
  }
};
using UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

מחלקה TextEncodingConverterICU final
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

  עשה {
    EC = U_ZERO_ERROR;
    ثابت حرف *Input = In;

    Output = static_cast<حرف *>(Result.data());
    ucnv_convertEx(&*ToConvDesc, &*FromConvDesc, &Output, Result.end(), &Input,
                   In + InputLength, /*pivotStart=*/عدم,
                   /*pivotSource=*/عدم, /*pivotTarget=*/عدم,
                   /*pivotLimit=*/عدم, /*reset=*/אמת,
                   /*flush=*/אמת, &EC);
    אם (U_FAILURE(EC)) {
      אם (EC == U_BUFFER_OVERFLOW_ERROR) {
        אם (Capacity < Result.max_size()) {
          HandleOverflow(Capacity, Output, OutputLength, Result);
          استمر;
        } אחרת {
          Result.resize(Output - Result.data());
          החזר std::error_code(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      Result.resize(Output - Result.data());
      החזר std::error_code(EILSEQ, std::generic_category());
    }
    اكسر;
  } כלעוד (אמת);

  Result.resize(Output - Result.data());
  החזר std::error_code();
}

فراغ TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ToConvDesc);
}

#وإلاإذا HAVE_ICONV
מחלקה TextEncodingConverterIconv final
    : عام details::TextEncodingConverterImplBase {
  מחלקה UniqueIconvT {
    iconv_t ConvDesc;

  عام:
    operator iconv_t() ثابت { החזר ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      אם (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &operator=(UniqueIconvT &&Other) {
      אם (&Other != هذا) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      החזר *هذا;
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
    אם (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      אם (errno == E2BIG && Capacity < Result.max_size()) {
        HandleOverflow(Capacity, Output, OutputLength, Result);
        // Reset converter
        reset();
        החזר std::error_code();
      } אחרת {
        // Some other error occurred.
        Result.resize(Output - Result.data());
        החזר std::error_code(errno, std::generic_category());
      }
    } אחרת {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      החזר std::make_error_code(std::errc::illegal_byte_sequence);
    }
  };

  עשה {
    size_t InputLength = Source.size();
    حرف *Input = const_cast<حرف *>(Source.data());
    Ret = iconv(ConvDesc, &Input, &InputLength, &Output, &OutputLength);
    אם (Ret != 0) {
      אם (auto EC = HandleError(Ret))
        החזר EC;
      استمر;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, nullptr, nullptr, &Output, &OutputLength);
    אם (Ret != 0) {
      אם (auto EC = HandleError(Ret))
        החזר EC;
      استمر;
    }
    اكسر;
  } כלעוד (אמת);

  // Re-adjust size to actual size.
  Result.resize(Output - Result.data());
  החזר std::error_code();
}

مضمن فراغ TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, nullptr, nullptr, nullptr, nullptr);
}

#endif // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  אם (CPFrom == CPTo)
    החזר std::make_error_code(std::errc::invalid_argument);

  ConversionType Conversion;
  אם (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8ToIBM1047;
  אחרת אם (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047ToUTF8;
  אחרת
    החזר std::make_error_code(std::errc::invalid_argument);

  החזר TextEncodingConverter(
      std::make_unique<TextEncodingConverterTable>(Conversion));
}

ErrorOr<TextEncodingConverter> TextEncodingConverter::create(StringRef From,
                                                             StringRef To) {
  std::optional<TextEncoding> FromEncoding = getKnownEncoding(From);
  std::optional<TextEncoding> ToEncoding = getKnownEncoding(To);
  אם (FromEncoding && ToEncoding) {
    ErrorOr<TextEncodingConverter> Converter =
        create(*FromEncoding, *ToEncoding);
    אם (Converter)
      החזר Converter;
  }
#אם HAVE_ICU
  UErrorCode EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  אם (U_FAILURE(EC))
    החזר std::make_error_code(std::errc::invalid_argument);

  UConverterUniquePtr ToConvDesc(ucnv_open(To.str().c_str(), &EC));
  אם (U_FAILURE(EC))
    החזר std::make_error_code(std::errc::invalid_argument);

  auto Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(ToConvDesc));
  החזר TextEncodingConverter(std::move(Converter));
#وإلاإذا HAVE_ICONV
  iconv_t ConvDesc = iconv_open(To.str().c_str(), From.str().c_str());
  אם (ConvDesc == (iconv_t)-1)
    החזר std::make_error_code(std::errc::invalid_argument);
  החזר TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#אחרת
  החזר std::make_error_code(std::errc::invalid_argument);
#endif
}
