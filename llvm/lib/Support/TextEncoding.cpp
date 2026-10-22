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

#അന്തര്ഭാവ "llvm/Support/TextEncoding.h"
#അന്തര്ഭാവ "llvm/ADT/SmallString.h"
#അന്തര്ഭാവ "llvm/ADT/SmallVector.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Support/ConvertEBCDIC.h"
#അന്തര്ഭാവ <system_error>

#എങ്കിൽ HAVE_ICU
#എങ്കിൽ HAVE_WINDOWS_ICU
#അന്തര്ഭാവ <icu.h>
#അല്ലെങ്കിൽ
#അന്തര്ഭാവ <unicode/ucnv.h>
#endif
#അല്ലെങ്കിലെങ്കിൽ HAVE_ICONV
#അന്തര്ഭാവ <iconv.h>
#endif

using നാമാകാശ llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
സ്ഥിതം ശൂന്യം normalizeCharSetName(StringRef CSName,
                                 SmallVectorImpl<അക്ഷരം> &Normalized) {
  തർക്കം PrevDigit = അസത്യം;
  വേണ്ടി (auto Ch : CSName) {
    എങ്കിൽ (isAlnum(Ch)) {
      Ch = toLower(Ch);
      എങ്കിൽ (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
        Normalized.തള്ളുക_back(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
സ്ഥിതം std::optional<TextEncoding> getKnownEncoding(StringRef Name) {
  SmallString<16> Normalized;
  normalizeCharSetName(Name, Normalized);
  എങ്കിൽ (Normalized.equals("utf8"))
    തിരികെ TextEncoding::UTF8;
  എങ്കിൽ (Normalized.equals("ibm1047"))
    തിരികെ TextEncoding::IBM1047;
  തിരികെ std::nullopt;
}

[[maybe_unused]] സ്ഥിതം ശൂന്യം HandleOverflow(size_t &Capacity, അക്ഷരം *&Output,
                                            size_t &OutputLength,
                                            SmallVectorImpl<അക്ഷരം> &Result) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  Capacity =
      (Capacity < Result.max_size() / 2) ? 2 * Capacity : Result.max_size();
  Result.resize(0);
  Result.resize_for_overwrite(Capacity);
  Output = static_cast<അക്ഷരം *>(Result.data());
  OutputLength = Capacity;
}

നാമാകാശ {
എണ്ണൽ ConversionType {
  UTF8ToIBM1047,
  IBM1047ToUTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
വർഗ്ഗം TextEncodingConverterTable final
    : പൊതു details::TextEncodingConverterImplBase {
  സ്ഥിരം ConversionType ConvType;

പൊതു:
  TextEncodingConverterTable(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<അക്ഷരം> &Result) override;

  ശൂന്യം reset() override {}
};

std::error_code
TextEncodingConverterTable::convertString(StringRef Source,
                                          SmallVectorImpl<അക്ഷരം> &Result) {
  തിരഞ്ഞെടുപ്പ് (ConvType) {
  അവസ്ഥ IBM1047ToUTF8:
    ConverterEBCDIC::convertToUTF8(Source, Result);
    തിരികെ std::error_code();
  അവസ്ഥ UTF8ToIBM1047:
    തിരികെ ConverterEBCDIC::convertToEBCDIC(Source, Result);
  }
  llvm_unreachable("Invalid ConvType!");
  തിരികെ std::error_code();
}

#എങ്കിൽ HAVE_ICU
ഘടന UConverterDeleter {
  ശൂന്യം operator()(UConverter *Converter) സ്ഥിരം {
    എങ്കിൽ (Converter)
      ucnv_close(Converter);
  }
};
using UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

വർഗ്ഗം TextEncodingConverterICU final
    : പൊതു details::TextEncodingConverterImplBase {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr ToConvDesc;

പൊതു:
  TextEncodingConverterICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr ToConverter)
      : FromConvDesc(std::move(FromConverter)),
        ToConvDesc(std::move(ToConverter)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<അക്ഷരം> &Result) override;

  ശൂന്യം reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_code
TextEncodingConverterICU::convertString(StringRef Source,
                                        SmallVectorImpl<അക്ഷരം> &Result) {
  // Setup the input in case it has no backing data.
  size_t നിവേശം_Length = Source.size();
  സ്ഥിരം അക്ഷരം *In = നിവേശം_Length ? const_cast<അക്ഷരം *>(Source.data()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);
  അക്ഷരം *Output;
  UErrorCode EC = U_ZERO_ERROR;

  ucnv_setToUCallBack(&*FromConvDesc, UCNV_TO_U_CALLBACK_STOP, ശൂന്യ, ശൂന്യ, ശൂന്യ,
                      &EC);
  ucnv_setFromUCallBack(&*ToConvDesc, UCNV_FROM_U_CALLBACK_STOP, ശൂന്യ, ശൂന്യ,
                        ശൂന്യ, &EC);
  പ്രതിജ്ഞാ(U_SUCCESS(EC));

  ചെയ്യുക {
    EC = U_ZERO_ERROR;
    സ്ഥിരം അക്ഷരം *നിവേശം = In;

    Output = static_cast<അക്ഷരം *>(Result.data());
    ucnv_convertEx(&*ToConvDesc, &*FromConvDesc, &Output, Result.end(), &നിവേശം,
                   In + നിവേശം_Length, /*pivotStart=*/ശൂന്യ,
                   /*pivotSource=*/ശൂന്യ, /*pivotTarget=*/ശൂന്യ,
                   /*pivotLimit=*/ശൂന്യ, /*reset=*/സത്യം,
                   /*flush=*/സത്യം, &EC);
    എങ്കിൽ (U_FAILURE(EC)) {
      എങ്കിൽ (EC == U_BUFFER_OVERFLOW_ERROR) {
        എങ്കിൽ (Capacity < Result.max_size()) {
          HandleOverflow(Capacity, Output, OutputLength, Result);
          തുടരുക;
        } അല്ലെങ്കിൽ {
          Result.resize(Output - Result.data());
          തിരികെ std::error_code(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      Result.resize(Output - Result.data());
      തിരികെ std::error_code(EILSEQ, std::generic_category());
    }
    നിർത്തുക;
  } ആകുമ്പോൾ (സത്യം);

  Result.resize(Output - Result.data());
  തിരികെ std::error_code();
}

ശൂന്യം TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ToConvDesc);
}

#അല്ലെങ്കിലെങ്കിൽ HAVE_ICONV
വർഗ്ഗം TextEncodingConverterIconv final
    : പൊതു details::TextEncodingConverterImplBase {
  വർഗ്ഗം UniqueIconvT {
    iconv_t ConvDesc;

  പൊതു:
    operator iconv_t() സ്ഥിരം { തിരികെ ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      എങ്കിൽ (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &operator=(UniqueIconvT &&Other) {
      എങ്കിൽ (&Other != ഇത്) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      തിരികെ *ഇത്;
    }
  };
  UniqueIconvT ConvDesc;

പൊതു:
  TextEncodingConverterIconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<അക്ഷരം> &Result) override;

  ശൂന്യം reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_code
TextEncodingConverterIconv::convertString(StringRef Source,
                                          SmallVectorImpl<അക്ഷരം> &Result) {
  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  അക്ഷരം *Output = static_cast<അക്ഷരം *>(Result.data());
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);

  size_t Ret;
  // Handle errors returned from iconv().
  auto HandleError = [&Capacity, &Output, &OutputLength, &Result,
                      ഇത്](size_t Ret) {
    എങ്കിൽ (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      എങ്കിൽ (errno == E2BIG && Capacity < Result.max_size()) {
        HandleOverflow(Capacity, Output, OutputLength, Result);
        // Reset converter
        reset();
        തിരികെ std::error_code();
      } അല്ലെങ്കിൽ {
        // Some other error occurred.
        Result.resize(Output - Result.data());
        തിരികെ std::error_code(errno, std::generic_category());
      }
    } അല്ലെങ്കിൽ {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      തിരികെ std::make_error_code(std::errc::illegal_byte_sequence);
    }
  };

  ചെയ്യുക {
    size_t നിവേശം_Length = Source.size();
    അക്ഷരം *നിവേശം = const_cast<അക്ഷരം *>(Source.data());
    Ret = iconv(ConvDesc, &നിവേശം, &നിവേശം_Length, &Output, &OutputLength);
    എങ്കിൽ (Ret != 0) {
      എങ്കിൽ (auto EC = HandleError(Ret))
        തിരികെ EC;
      തുടരുക;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, nullptr, nullptr, &Output, &OutputLength);
    എങ്കിൽ (Ret != 0) {
      എങ്കിൽ (auto EC = HandleError(Ret))
        തിരികെ EC;
      തുടരുക;
    }
    നിർത്തുക;
  } ആകുമ്പോൾ (സത്യം);

  // Re-adjust size to actual size.
  Result.resize(Output - Result.data());
  തിരികെ std::error_code();
}

വരിയിൽ ശൂന്യം TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, nullptr, nullptr, nullptr, nullptr);
}

#endif // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  എങ്കിൽ (CPFrom == CPTo)
    തിരികെ std::make_error_code(std::errc::invalid_argument);

  ConversionType Conversion;
  എങ്കിൽ (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8ToIBM1047;
  അല്ലെങ്കിൽ എങ്കിൽ (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047ToUTF8;
  അല്ലെങ്കിൽ
    തിരികെ std::make_error_code(std::errc::invalid_argument);

  തിരികെ TextEncodingConverter(
      std::make_unique<TextEncodingConverterTable>(Conversion));
}

ErrorOr<TextEncodingConverter> TextEncodingConverter::create(StringRef From,
                                                             StringRef To) {
  std::optional<TextEncoding> FromEncoding = getKnownEncoding(From);
  std::optional<TextEncoding> ToEncoding = getKnownEncoding(To);
  എങ്കിൽ (FromEncoding && ToEncoding) {
    ErrorOr<TextEncodingConverter> Converter =
        create(*FromEncoding, *ToEncoding);
    എങ്കിൽ (Converter)
      തിരികെ Converter;
  }
#എങ്കിൽ HAVE_ICU
  UErrorCode EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  എങ്കിൽ (U_FAILURE(EC))
    തിരികെ std::make_error_code(std::errc::invalid_argument);

  UConverterUniquePtr ToConvDesc(ucnv_open(To.str().c_str(), &EC));
  എങ്കിൽ (U_FAILURE(EC))
    തിരികെ std::make_error_code(std::errc::invalid_argument);

  auto Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(ToConvDesc));
  തിരികെ TextEncodingConverter(std::move(Converter));
#അല്ലെങ്കിലെങ്കിൽ HAVE_ICONV
  iconv_t ConvDesc = iconv_open(To.str().c_str(), From.str().c_str());
  എങ്കിൽ (ConvDesc == (iconv_t)-1)
    തിരികെ std::make_error_code(std::errc::invalid_argument);
  തിരികെ TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#അല്ലെങ്കിൽ
  തിരികെ std::make_error_code(std::errc::invalid_argument);
#endif
}
