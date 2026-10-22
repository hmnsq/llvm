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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/TextEncoding.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallString.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallVector.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ConvertEBCDIC.h"
#ଅନ୍ତର୍ଭାଵ <system_error>

#ଯଦି HAVE_ICU
#ଯଦି HAVE_WINDOWS_ICU
#ଅନ୍ତର୍ଭାଵ <icu.h>
#ଅନ୍ୟଥା
#ଅନ୍ତର୍ଭାଵ <unicode/ucnv.h>
#endif
#ନଚେତଯଦି HAVE_ICONV
#ଅନ୍ତର୍ଭାଵ <iconv.h>
#endif

using ନାମାକାଶ llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
ସ୍ଥିର ଶୂନ୍ୟ normalizeCharSetName(StringRef CSName,
                                 SmallVectorImpl<ଅକ୍ଷର> &Normalized) {
  ତାର୍କିକ PrevDigit = ମିଥ୍ୟା;
  ପାଇଁ (auto Ch : CSName) {
    ଯଦି (isAlnum(Ch)) {
      Ch = toLower(Ch);
      ଯଦି (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
        Normalized.ଠେଲ_back(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
ସ୍ଥିର std::optional<TextEncoding> getKnownEncoding(StringRef Name) {
  SmallString<16> Normalized;
  normalizeCharSetName(Name, Normalized);
  ଯଦି (Normalized.equals("utf8"))
    ଫେରନ୍ତୁ TextEncoding::UTF8;
  ଯଦି (Normalized.equals("ibm1047"))
    ଫେରନ୍ତୁ TextEncoding::IBM1047;
  ଫେରନ୍ତୁ std::nullopt;
}

[[maybe_unused]] ସ୍ଥିର ଶୂନ୍ୟ HandleOverflow(size_t &Capacity, ଅକ୍ଷର *&Output,
                                            size_t &OutputLength,
                                            SmallVectorImpl<ଅକ୍ଷର> &Result) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  Capacity =
      (Capacity < Result.max_size() / 2) ? 2 * Capacity : Result.max_size();
  Result.resize(0);
  Result.resize_for_overwrite(Capacity);
  Output = static_cast<ଅକ୍ଷର *>(Result.data());
  OutputLength = Capacity;
}

ନାମାକାଶ {
ଗଣନା ConversionType {
  UTF8ToIBM1047,
  IBM1047ToUTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
ଶ୍ରେଣୀ TextEncodingConverterTable final
    : ସର୍ବସାଧାରଣ details::TextEncodingConverterImplBase {
  ଅଚଳ ConversionType ConvType;

ସର୍ବସାଧାରଣ:
  TextEncodingConverterTable(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<ଅକ୍ଷର> &Result) override;

  ଶୂନ୍ୟ reset() override {}
};

std::error_code
TextEncodingConverterTable::convertString(StringRef Source,
                                          SmallVectorImpl<ଅକ୍ଷର> &Result) {
  ବାଛ (ConvType) {
  ସ୍ଥିତି IBM1047ToUTF8:
    ConverterEBCDIC::convertToUTF8(Source, Result);
    ଫେରନ୍ତୁ std::error_code();
  ସ୍ଥିତି UTF8ToIBM1047:
    ଫେରନ୍ତୁ ConverterEBCDIC::convertToEBCDIC(Source, Result);
  }
  llvm_unreachable("Invalid ConvType!");
  ଫେରନ୍ତୁ std::error_code();
}

#ଯଦି HAVE_ICU
ସଂରଚନା UConverterDeleter {
  ଶୂନ୍ୟ operator()(UConverter *Converter) ଅଚଳ {
    ଯଦି (Converter)
      ucnv_close(Converter);
  }
};
using UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

ଶ୍ରେଣୀ TextEncodingConverterICU final
    : ସର୍ବସାଧାରଣ details::TextEncodingConverterImplBase {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr ToConvDesc;

ସର୍ବସାଧାରଣ:
  TextEncodingConverterICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr ToConverter)
      : FromConvDesc(std::move(FromConverter)),
        ToConvDesc(std::move(ToConverter)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<ଅକ୍ଷର> &Result) override;

  ଶୂନ୍ୟ reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_code
TextEncodingConverterICU::convertString(StringRef Source,
                                        SmallVectorImpl<ଅକ୍ଷର> &Result) {
  // Setup the input in case it has no backing data.
  size_t ନିବେଶ_Length = Source.size();
  ଅଚଳ ଅକ୍ଷର *In = ନିବେଶ_Length ? const_cast<ଅକ୍ଷର *>(Source.data()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);
  ଅକ୍ଷର *Output;
  UErrorCode EC = U_ZERO_ERROR;

  ucnv_setToUCallBack(&*FromConvDesc, UCNV_TO_U_CALLBACK_STOP, ଖାଲି, ଖାଲି, ଖାଲି,
                      &EC);
  ucnv_setFromUCallBack(&*ToConvDesc, UCNV_FROM_U_CALLBACK_STOP, ଖାଲି, ଖାଲି,
                        ଖାଲି, &EC);
  ପ୍ରତିଜ୍ଞା(U_SUCCESS(EC));

  କର {
    EC = U_ZERO_ERROR;
    ଅଚଳ ଅକ୍ଷର *ନିବେଶ = In;

    Output = static_cast<ଅକ୍ଷର *>(Result.data());
    ucnv_convertEx(&*ToConvDesc, &*FromConvDesc, &Output, Result.end(), &ନିବେଶ,
                   In + ନିବେଶ_Length, /*pivotStart=*/ଖାଲି,
                   /*pivotSource=*/ଖାଲି, /*pivotTarget=*/ଖାଲି,
                   /*pivotLimit=*/ଖାଲି, /*reset=*/ସତ୍ୟ,
                   /*flush=*/ସତ୍ୟ, &EC);
    ଯଦି (U_FAILURE(EC)) {
      ଯଦି (EC == U_BUFFER_OVERFLOW_ERROR) {
        ଯଦି (Capacity < Result.max_size()) {
          HandleOverflow(Capacity, Output, OutputLength, Result);
          ଜାରି;
        } ଅନ୍ୟଥା {
          Result.resize(Output - Result.data());
          ଫେରନ୍ତୁ std::error_code(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      Result.resize(Output - Result.data());
      ଫେରନ୍ତୁ std::error_code(EILSEQ, std::generic_category());
    }
    ଭାଙ୍ଗ;
  } ଯେତେବେଳେ (ସତ୍ୟ);

  Result.resize(Output - Result.data());
  ଫେରନ୍ତୁ std::error_code();
}

ଶୂନ୍ୟ TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ToConvDesc);
}

#ନଚେତଯଦି HAVE_ICONV
ଶ୍ରେଣୀ TextEncodingConverterIconv final
    : ସର୍ବସାଧାରଣ details::TextEncodingConverterImplBase {
  ଶ୍ରେଣୀ UniqueIconvT {
    iconv_t ConvDesc;

  ସର୍ବସାଧାରଣ:
    operator iconv_t() ଅଚଳ { ଫେରନ୍ତୁ ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      ଯଦି (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &operator=(UniqueIconvT &&Other) {
      ଯଦି (&Other != ଏହି) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      ଫେରନ୍ତୁ *ଏହି;
    }
  };
  UniqueIconvT ConvDesc;

ସର୍ବସାଧାରଣ:
  TextEncodingConverterIconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<ଅକ୍ଷର> &Result) override;

  ଶୂନ୍ୟ reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_code
TextEncodingConverterIconv::convertString(StringRef Source,
                                          SmallVectorImpl<ଅକ୍ଷର> &Result) {
  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  ଅକ୍ଷର *Output = static_cast<ଅକ୍ଷର *>(Result.data());
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);

  size_t Ret;
  // Handle errors returned from iconv().
  auto HandleError = [&Capacity, &Output, &OutputLength, &Result,
                      ଏହି](size_t Ret) {
    ଯଦି (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      ଯଦି (errno == E2BIG && Capacity < Result.max_size()) {
        HandleOverflow(Capacity, Output, OutputLength, Result);
        // Reset converter
        reset();
        ଫେରନ୍ତୁ std::error_code();
      } ଅନ୍ୟଥା {
        // Some other error occurred.
        Result.resize(Output - Result.data());
        ଫେରନ୍ତୁ std::error_code(errno, std::generic_category());
      }
    } ଅନ୍ୟଥା {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      ଫେରନ୍ତୁ std::make_error_code(std::errc::illegal_byte_sequence);
    }
  };

  କର {
    size_t ନିବେଶ_Length = Source.size();
    ଅକ୍ଷର *ନିବେଶ = const_cast<ଅକ୍ଷର *>(Source.data());
    Ret = iconv(ConvDesc, &ନିବେଶ, &ନିବେଶ_Length, &Output, &OutputLength);
    ଯଦି (Ret != 0) {
      ଯଦି (auto EC = HandleError(Ret))
        ଫେରନ୍ତୁ EC;
      ଜାରି;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, nullptr, nullptr, &Output, &OutputLength);
    ଯଦି (Ret != 0) {
      ଯଦି (auto EC = HandleError(Ret))
        ଫେରନ୍ତୁ EC;
      ଜାରି;
    }
    ଭାଙ୍ଗ;
  } ଯେତେବେଳେ (ସତ୍ୟ);

  // Re-adjust size to actual size.
  Result.resize(Output - Result.data());
  ଫେରନ୍ତୁ std::error_code();
}

ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, nullptr, nullptr, nullptr, nullptr);
}

#endif // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  ଯଦି (CPFrom == CPTo)
    ଫେରନ୍ତୁ std::make_error_code(std::errc::invalid_argument);

  ConversionType Conversion;
  ଯଦି (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8ToIBM1047;
  ଅନ୍ୟଥା ଯଦି (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047ToUTF8;
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ std::make_error_code(std::errc::invalid_argument);

  ଫେରନ୍ତୁ TextEncodingConverter(
      std::make_unique<TextEncodingConverterTable>(Conversion));
}

ErrorOr<TextEncodingConverter> TextEncodingConverter::create(StringRef From,
                                                             StringRef To) {
  std::optional<TextEncoding> FromEncoding = getKnownEncoding(From);
  std::optional<TextEncoding> ToEncoding = getKnownEncoding(To);
  ଯଦି (FromEncoding && ToEncoding) {
    ErrorOr<TextEncodingConverter> Converter =
        create(*FromEncoding, *ToEncoding);
    ଯଦି (Converter)
      ଫେରନ୍ତୁ Converter;
  }
#ଯଦି HAVE_ICU
  UErrorCode EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  ଯଦି (U_FAILURE(EC))
    ଫେରନ୍ତୁ std::make_error_code(std::errc::invalid_argument);

  UConverterUniquePtr ToConvDesc(ucnv_open(To.str().c_str(), &EC));
  ଯଦି (U_FAILURE(EC))
    ଫେରନ୍ତୁ std::make_error_code(std::errc::invalid_argument);

  auto Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(ToConvDesc));
  ଫେରନ୍ତୁ TextEncodingConverter(std::move(Converter));
#ନଚେତଯଦି HAVE_ICONV
  iconv_t ConvDesc = iconv_open(To.str().c_str(), From.str().c_str());
  ଯଦି (ConvDesc == (iconv_t)-1)
    ଫେରନ୍ତୁ std::make_error_code(std::errc::invalid_argument);
  ଫେରନ୍ତୁ TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#ଅନ୍ୟଥା
  ଫେରନ୍ତୁ std::make_error_code(std::errc::invalid_argument);
#endif
}
