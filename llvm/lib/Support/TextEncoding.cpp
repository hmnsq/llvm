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

#شامل "llvm/Support/TextEncoding.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Support/ConvertEBCDIC.h"
#شامل <system_error>

#اگر HAVE_ICU
#اگر HAVE_WINDOWS_ICU
#شامل <icu.h>
#وگرنه
#شامل <unicode/ucnv.h>
#endif
#وگرنهاگر HAVE_ICONV
#شامل <iconv.h>
#endif

using فضاینام llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
ایستا تهی normalizeCharSetName(StringRef CSName,
                                 SmallVectorImpl<نویسه> &Normalized) {
  منطقی PrevDigit = نادرست;
  برای (auto Ch : CSName) {
    اگر (isAlnum(Ch)) {
      Ch = toLower(Ch);
      اگر (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
        Normalized.افزودن_back(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
ایستا std::optional<TextEncoding> getKnownEncoding(StringRef Name) {
  SmallString<16> Normalized;
  normalizeCharSetName(Name, Normalized);
  اگر (Normalized.equals("utf8"))
    بازگشت TextEncoding::UTF8;
  اگر (Normalized.equals("ibm1047"))
    بازگشت TextEncoding::IBM1047;
  بازگشت std::nullopt;
}

[[maybe_unused]] ایستا تهی HandleOverflow(size_t &Capacity, نویسه *&Output,
                                            size_t &OutputLength,
                                            SmallVectorImpl<نویسه> &Result) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  Capacity =
      (Capacity < Result.max_size() / 2) ? 2 * Capacity : Result.max_size();
  Result.resize(0);
  Result.resize_for_overwrite(Capacity);
  Output = static_cast<نویسه *>(Result.data());
  OutputLength = Capacity;
}

فضاینام {
شمارش ConversionType {
  UTF8ToIBM1047,
  IBM1047ToUTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
رده TextEncodingConverterTable final
    : عمومی details::TextEncodingConverterImplBase {
  ثابت ConversionType ConvType;

عمومی:
  TextEncodingConverterTable(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<نویسه> &Result) override;

  تهی reset() override {}
};

std::error_code
TextEncodingConverterTable::convertString(StringRef Source,
                                          SmallVectorImpl<نویسه> &Result) {
  انتخاب (ConvType) {
  حالت IBM1047ToUTF8:
    ConverterEBCDIC::convertToUTF8(Source, Result);
    بازگشت std::error_code();
  حالت UTF8ToIBM1047:
    بازگشت ConverterEBCDIC::convertToEBCDIC(Source, Result);
  }
  llvm_unreachable("Invalid ConvType!");
  بازگشت std::error_code();
}

#اگر HAVE_ICU
ساختار UConverterDeleter {
  تهی operator()(UConverter *Converter) ثابت {
    اگر (Converter)
      ucnv_close(Converter);
  }
};
using UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

رده TextEncodingConverterICU final
    : عمومی details::TextEncodingConverterImplBase {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr ToConvDesc;

عمومی:
  TextEncodingConverterICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr ToConverter)
      : FromConvDesc(std::move(FromConverter)),
        ToConvDesc(std::move(ToConverter)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<نویسه> &Result) override;

  تهی reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_code
TextEncodingConverterICU::convertString(StringRef Source,
                                        SmallVectorImpl<نویسه> &Result) {
  // Setup the input in case it has no backing data.
  size_t درون_داد_Length = Source.size();
  ثابت نویسه *In = درون_داد_Length ? const_cast<نویسه *>(Source.data()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);
  نویسه *Output;
  UErrorCode EC = U_ZERO_ERROR;

  ucnv_setToUCallBack(&*FromConvDesc, UCNV_TO_U_CALLBACK_STOP, پوچ, پوچ, پوچ,
                      &EC);
  ucnv_setFromUCallBack(&*ToConvDesc, UCNV_FROM_U_CALLBACK_STOP, پوچ, پوچ,
                        پوچ, &EC);
  أكد(U_SUCCESS(EC));

  انجامبده {
    EC = U_ZERO_ERROR;
    ثابت نویسه *درون_داد = In;

    Output = static_cast<نویسه *>(Result.data());
    ucnv_convertEx(&*ToConvDesc, &*FromConvDesc, &Output, Result.end(), &درون_داد,
                   In + درون_داد_Length, /*pivotStart=*/پوچ,
                   /*pivotSource=*/پوچ, /*pivotTarget=*/پوچ,
                   /*pivotLimit=*/پوچ, /*reset=*/درست,
                   /*flush=*/درست, &EC);
    اگر (U_FAILURE(EC)) {
      اگر (EC == U_BUFFER_OVERFLOW_ERROR) {
        اگر (Capacity < Result.max_size()) {
          HandleOverflow(Capacity, Output, OutputLength, Result);
          ادامه;
        } وگرنه {
          Result.resize(Output - Result.data());
          بازگشت std::error_code(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      Result.resize(Output - Result.data());
      بازگشت std::error_code(EILSEQ, std::generic_category());
    }
    بشکن;
  } وقتی (درست);

  Result.resize(Output - Result.data());
  بازگشت std::error_code();
}

تهی TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ToConvDesc);
}

#وگرنهاگر HAVE_ICONV
رده TextEncodingConverterIconv final
    : عمومی details::TextEncodingConverterImplBase {
  رده UniqueIconvT {
    iconv_t ConvDesc;

  عمومی:
    operator iconv_t() ثابت { بازگشت ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      اگر (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &operator=(UniqueIconvT &&Other) {
      اگر (&Other != این) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      بازگشت *این;
    }
  };
  UniqueIconvT ConvDesc;

عمومی:
  TextEncodingConverterIconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<نویسه> &Result) override;

  تهی reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_code
TextEncodingConverterIconv::convertString(StringRef Source,
                                          SmallVectorImpl<نویسه> &Result) {
  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  نویسه *Output = static_cast<نویسه *>(Result.data());
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);

  size_t Ret;
  // Handle errors returned from iconv().
  auto HandleError = [&Capacity, &Output, &OutputLength, &Result,
                      این](size_t Ret) {
    اگر (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      اگر (errno == E2BIG && Capacity < Result.max_size()) {
        HandleOverflow(Capacity, Output, OutputLength, Result);
        // Reset converter
        reset();
        بازگشت std::error_code();
      } وگرنه {
        // Some other error occurred.
        Result.resize(Output - Result.data());
        بازگشت std::error_code(errno, std::generic_category());
      }
    } وگرنه {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      بازگشت std::make_error_code(std::errc::illegal_byte_sequence);
    }
  };

  انجامبده {
    size_t درون_داد_Length = Source.size();
    نویسه *درون_داد = const_cast<نویسه *>(Source.data());
    Ret = iconv(ConvDesc, &درون_داد, &درون_داد_Length, &Output, &OutputLength);
    اگر (Ret != 0) {
      اگر (auto EC = HandleError(Ret))
        بازگشت EC;
      ادامه;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, nullptr, nullptr, &Output, &OutputLength);
    اگر (Ret != 0) {
      اگر (auto EC = HandleError(Ret))
        بازگشت EC;
      ادامه;
    }
    بشکن;
  } وقتی (درست);

  // Re-adjust size to actual size.
  Result.resize(Output - Result.data());
  بازگشت std::error_code();
}

درونخطی تهی TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, nullptr, nullptr, nullptr, nullptr);
}

#endif // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  اگر (CPFrom == CPTo)
    بازگشت std::make_error_code(std::errc::invalid_argument);

  ConversionType Conversion;
  اگر (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8ToIBM1047;
  وگرنه اگر (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047ToUTF8;
  وگرنه
    بازگشت std::make_error_code(std::errc::invalid_argument);

  بازگشت TextEncodingConverter(
      std::make_unique<TextEncodingConverterTable>(Conversion));
}

ErrorOr<TextEncodingConverter> TextEncodingConverter::create(StringRef From,
                                                             StringRef To) {
  std::optional<TextEncoding> FromEncoding = getKnownEncoding(From);
  std::optional<TextEncoding> ToEncoding = getKnownEncoding(To);
  اگر (FromEncoding && ToEncoding) {
    ErrorOr<TextEncodingConverter> Converter =
        create(*FromEncoding, *ToEncoding);
    اگر (Converter)
      بازگشت Converter;
  }
#اگر HAVE_ICU
  UErrorCode EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  اگر (U_FAILURE(EC))
    بازگشت std::make_error_code(std::errc::invalid_argument);

  UConverterUniquePtr ToConvDesc(ucnv_open(To.str().c_str(), &EC));
  اگر (U_FAILURE(EC))
    بازگشت std::make_error_code(std::errc::invalid_argument);

  auto Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(ToConvDesc));
  بازگشت TextEncodingConverter(std::move(Converter));
#وگرنهاگر HAVE_ICONV
  iconv_t ConvDesc = iconv_open(To.str().c_str(), From.str().c_str());
  اگر (ConvDesc == (iconv_t)-1)
    بازگشت std::make_error_code(std::errc::invalid_argument);
  بازگشت TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#وگرنه
  بازگشت std::make_error_code(std::errc::invalid_argument);
#endif
}
