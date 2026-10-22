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

#அந்தர்பாவ "llvm/Support/TextEncoding.h"
#அந்தர்பாவ "llvm/ADT/SmallString.h"
#அந்தர்பாவ "llvm/ADT/SmallVector.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Support/ConvertEBCDIC.h"
#அந்தர்பாவ <system_error>

#எனில் HAVE_ICU
#எனில் HAVE_WINDOWS_ICU
#அந்தர்பாவ <icu.h>
#இல்லையெனில்
#அந்தர்பாவ <unicode/ucnv.h>
#endif
#அல்லதெனில் HAVE_ICONV
#அந்தர்பாவ <iconv.h>
#endif

using நாமாகாஶ llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
நிலையான வெறுமை normalizeCharSetName(StringRef CSName,
                                 SmallVectorImpl<எழுத்து> &Normalized) {
  தருக்க PrevDigit = பொய்;
  க்கு (auto Ch : CSName) {
    எனில் (isAlnum(Ch)) {
      Ch = toLower(Ch);
      எனில் (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
        Normalized.தள்ளு_back(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
நிலையான std::optional<TextEncoding> getKnownEncoding(StringRef Name) {
  SmallString<16> Normalized;
  normalizeCharSetName(Name, Normalized);
  எனில் (Normalized.equals("utf8"))
    திரும்பு TextEncoding::UTF8;
  எனில் (Normalized.equals("ibm1047"))
    திரும்பு TextEncoding::IBM1047;
  திரும்பு std::nullopt;
}

[[maybe_unused]] நிலையான வெறுமை HandleOverflow(size_t &Capacity, எழுத்து *&Output,
                                            size_t &OutputLength,
                                            SmallVectorImpl<எழுத்து> &Result) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  Capacity =
      (Capacity < Result.max_size() / 2) ? 2 * Capacity : Result.max_size();
  Result.resize(0);
  Result.resize_for_overwrite(Capacity);
  Output = static_cast<எழுத்து *>(Result.data());
  OutputLength = Capacity;
}

நாமாகாஶ {
எண்ணிக்கை ConversionType {
  UTF8ToIBM1047,
  IBM1047ToUTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
வகுப்பு TextEncodingConverterTable final
    : பொது details::TextEncodingConverterImplBase {
  மாறிலி ConversionType ConvType;

பொது:
  TextEncodingConverterTable(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<எழுத்து> &Result) override;

  வெறுமை reset() override {}
};

std::error_code
TextEncodingConverterTable::convertString(StringRef Source,
                                          SmallVectorImpl<எழுத்து> &Result) {
  தேர்வு (ConvType) {
  நிலை IBM1047ToUTF8:
    ConverterEBCDIC::convertToUTF8(Source, Result);
    திரும்பு std::error_code();
  நிலை UTF8ToIBM1047:
    திரும்பு ConverterEBCDIC::convertToEBCDIC(Source, Result);
  }
  llvm_unreachable("Invalid ConvType!");
  திரும்பு std::error_code();
}

#எனில் HAVE_ICU
கட்டமைப்பு UConverterDeleter {
  வெறுமை operator()(UConverter *Converter) மாறிலி {
    எனில் (Converter)
      ucnv_close(Converter);
  }
};
using UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

வகுப்பு TextEncodingConverterICU final
    : பொது details::TextEncodingConverterImplBase {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr ToConvDesc;

பொது:
  TextEncodingConverterICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr ToConverter)
      : FromConvDesc(std::move(FromConverter)),
        ToConvDesc(std::move(ToConverter)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<எழுத்து> &Result) override;

  வெறுமை reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_code
TextEncodingConverterICU::convertString(StringRef Source,
                                        SmallVectorImpl<எழுத்து> &Result) {
  // Setup the input in case it has no backing data.
  size_t உள்ளீடு_Length = Source.size();
  மாறிலி எழுத்து *In = உள்ளீடு_Length ? const_cast<எழுத்து *>(Source.data()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);
  எழுத்து *Output;
  UErrorCode EC = U_ZERO_ERROR;

  ucnv_setToUCallBack(&*FromConvDesc, UCNV_TO_U_CALLBACK_STOP, வெற்று, வெற்று, வெற்று,
                      &EC);
  ucnv_setFromUCallBack(&*ToConvDesc, UCNV_FROM_U_CALLBACK_STOP, வெற்று, வெற்று,
                        வெற்று, &EC);
  ப்ரதிஜ்ஞா(U_SUCCESS(EC));

  செய் {
    EC = U_ZERO_ERROR;
    மாறிலி எழுத்து *உள்ளீடு = In;

    Output = static_cast<எழுத்து *>(Result.data());
    ucnv_convertEx(&*ToConvDesc, &*FromConvDesc, &Output, Result.end(), &உள்ளீடு,
                   In + உள்ளீடு_Length, /*pivotStart=*/வெற்று,
                   /*pivotSource=*/வெற்று, /*pivotTarget=*/வெற்று,
                   /*pivotLimit=*/வெற்று, /*reset=*/உண்மை,
                   /*flush=*/உண்மை, &EC);
    எனில் (U_FAILURE(EC)) {
      எனில் (EC == U_BUFFER_OVERFLOW_ERROR) {
        எனில் (Capacity < Result.max_size()) {
          HandleOverflow(Capacity, Output, OutputLength, Result);
          தொடர்;
        } இல்லையெனில் {
          Result.resize(Output - Result.data());
          திரும்பு std::error_code(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      Result.resize(Output - Result.data());
      திரும்பு std::error_code(EILSEQ, std::generic_category());
    }
    நிறுத்து;
  } போது (உண்மை);

  Result.resize(Output - Result.data());
  திரும்பு std::error_code();
}

வெறுமை TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ToConvDesc);
}

#அல்லதெனில் HAVE_ICONV
வகுப்பு TextEncodingConverterIconv final
    : பொது details::TextEncodingConverterImplBase {
  வகுப்பு UniqueIconvT {
    iconv_t ConvDesc;

  பொது:
    operator iconv_t() மாறிலி { திரும்பு ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      எனில் (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &operator=(UniqueIconvT &&Other) {
      எனில் (&Other != இது) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      திரும்பு *இது;
    }
  };
  UniqueIconvT ConvDesc;

பொது:
  TextEncodingConverterIconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<எழுத்து> &Result) override;

  வெறுமை reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_code
TextEncodingConverterIconv::convertString(StringRef Source,
                                          SmallVectorImpl<எழுத்து> &Result) {
  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  எழுத்து *Output = static_cast<எழுத்து *>(Result.data());
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);

  size_t Ret;
  // Handle errors returned from iconv().
  auto HandleError = [&Capacity, &Output, &OutputLength, &Result,
                      இது](size_t Ret) {
    எனில் (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      எனில் (errno == E2BIG && Capacity < Result.max_size()) {
        HandleOverflow(Capacity, Output, OutputLength, Result);
        // Reset converter
        reset();
        திரும்பு std::error_code();
      } இல்லையெனில் {
        // Some other error occurred.
        Result.resize(Output - Result.data());
        திரும்பு std::error_code(errno, std::generic_category());
      }
    } இல்லையெனில் {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      திரும்பு std::make_error_code(std::errc::illegal_byte_sequence);
    }
  };

  செய் {
    size_t உள்ளீடு_Length = Source.size();
    எழுத்து *உள்ளீடு = const_cast<எழுத்து *>(Source.data());
    Ret = iconv(ConvDesc, &உள்ளீடு, &உள்ளீடு_Length, &Output, &OutputLength);
    எனில் (Ret != 0) {
      எனில் (auto EC = HandleError(Ret))
        திரும்பு EC;
      தொடர்;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, nullptr, nullptr, &Output, &OutputLength);
    எனில் (Ret != 0) {
      எனில் (auto EC = HandleError(Ret))
        திரும்பு EC;
      தொடர்;
    }
    நிறுத்து;
  } போது (உண்மை);

  // Re-adjust size to actual size.
  Result.resize(Output - Result.data());
  திரும்பு std::error_code();
}

உள்வரி வெறுமை TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, nullptr, nullptr, nullptr, nullptr);
}

#endif // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  எனில் (CPFrom == CPTo)
    திரும்பு std::make_error_code(std::errc::invalid_argument);

  ConversionType Conversion;
  எனில் (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8ToIBM1047;
  இல்லையெனில் எனில் (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047ToUTF8;
  இல்லையெனில்
    திரும்பு std::make_error_code(std::errc::invalid_argument);

  திரும்பு TextEncodingConverter(
      std::make_unique<TextEncodingConverterTable>(Conversion));
}

ErrorOr<TextEncodingConverter> TextEncodingConverter::create(StringRef From,
                                                             StringRef To) {
  std::optional<TextEncoding> FromEncoding = getKnownEncoding(From);
  std::optional<TextEncoding> ToEncoding = getKnownEncoding(To);
  எனில் (FromEncoding && ToEncoding) {
    ErrorOr<TextEncodingConverter> Converter =
        create(*FromEncoding, *ToEncoding);
    எனில் (Converter)
      திரும்பு Converter;
  }
#எனில் HAVE_ICU
  UErrorCode EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  எனில் (U_FAILURE(EC))
    திரும்பு std::make_error_code(std::errc::invalid_argument);

  UConverterUniquePtr ToConvDesc(ucnv_open(To.str().c_str(), &EC));
  எனில் (U_FAILURE(EC))
    திரும்பு std::make_error_code(std::errc::invalid_argument);

  auto Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(ToConvDesc));
  திரும்பு TextEncodingConverter(std::move(Converter));
#அல்லதெனில் HAVE_ICONV
  iconv_t ConvDesc = iconv_open(To.str().c_str(), From.str().c_str());
  எனில் (ConvDesc == (iconv_t)-1)
    திரும்பு std::make_error_code(std::errc::invalid_argument);
  திரும்பு TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#இல்லையெனில்
  திரும்பு std::make_error_code(std::errc::invalid_argument);
#endif
}
