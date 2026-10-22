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

#अन्तर्भाव "llvm/Support/TextEncoding.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Support/ConvertEBCDIC.h"
#अन्तर्भाव <system_error>

#सचे HAVE_ICU
#सचे HAVE_WINDOWS_ICU
#अन्तर्भाव <icu.h>
#अञ्ञथा
#अन्तर्भाव <unicode/ucnv.h>
#endif
#नोचेद्यदि HAVE_ICONV
#अन्तर्भाव <iconv.h>
#endif

using नामाकाश llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
स्थिर शून्य normalizeCharSetName(StringRef CSName,
                                 SmallVectorImpl<वर्ण> &Normalized) {
  तार्किक PrevDigit = मिच्छा;
  अत्थाय (auto Ch : CSName) {
    सचे (isAlnum(Ch)) {
      Ch = toLower(Ch);
      सचे (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
        Normalized.प्रक्षिप_back(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
स्थिर std::optional<TextEncoding> getKnownEncoding(StringRef Name) {
  SmallString<16> Normalized;
  normalizeCharSetName(Name, Normalized);
  सचे (Normalized.equals("utf8"))
    पटिनिवत्त TextEncoding::UTF8;
  सचे (Normalized.equals("ibm1047"))
    पटिनिवत्त TextEncoding::IBM1047;
  पटिनिवत्त std::nullopt;
}

[[maybe_unused]] स्थिर शून्य HandleOverflow(size_t &Capacity, वर्ण *&Output,
                                            size_t &OutputLength,
                                            SmallVectorImpl<वर्ण> &Result) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  Capacity =
      (Capacity < Result.max_size() / 2) ? 2 * Capacity : Result.max_size();
  Result.resize(0);
  Result.resize_for_overwrite(Capacity);
  Output = static_cast<वर्ण *>(Result.data());
  OutputLength = Capacity;
}

नामाकाश {
गणना ConversionType {
  UTF8ToIBM1047,
  IBM1047ToUTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
वग्ग TextEncodingConverterTable final
    : सार्वजनिक details::TextEncodingConverterImplBase {
  ध्रुव ConversionType ConvType;

सार्वजनिक:
  TextEncodingConverterTable(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<वर्ण> &Result) override;

  शून्य reset() override {}
};

std::error_code
TextEncodingConverterTable::convertString(StringRef Source,
                                          SmallVectorImpl<वर्ण> &Result) {
  विकल्प (ConvType) {
  पक्ष IBM1047ToUTF8:
    ConverterEBCDIC::convertToUTF8(Source, Result);
    पटिनिवत्त std::error_code();
  पक्ष UTF8ToIBM1047:
    पटिनिवत्त ConverterEBCDIC::convertToEBCDIC(Source, Result);
  }
  llvm_unreachable("Invalid ConvType!");
  पटिनिवत्त std::error_code();
}

#सचे HAVE_ICU
संरचना UConverterDeleter {
  शून्य operator()(UConverter *Converter) ध्रुव {
    सचे (Converter)
      ucnv_close(Converter);
  }
};
using UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

वग्ग TextEncodingConverterICU final
    : सार्वजनिक details::TextEncodingConverterImplBase {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr ToConvDesc;

सार्वजनिक:
  TextEncodingConverterICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr ToConverter)
      : FromConvDesc(std::move(FromConverter)),
        ToConvDesc(std::move(ToConverter)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<वर्ण> &Result) override;

  शून्य reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_code
TextEncodingConverterICU::convertString(StringRef Source,
                                        SmallVectorImpl<वर्ण> &Result) {
  // Setup the input in case it has no backing data.
  size_t निवेशः_Length = Source.size();
  ध्रुव वर्ण *In = निवेशः_Length ? const_cast<वर्ण *>(Source.data()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);
  वर्ण *Output;
  UErrorCode EC = U_ZERO_ERROR;

  ucnv_setToUCallBack(&*FromConvDesc, UCNV_TO_U_CALLBACK_STOP, रिक्त, रिक्त, रिक्त,
                      &EC);
  ucnv_setFromUCallBack(&*ToConvDesc, UCNV_FROM_U_CALLBACK_STOP, रिक्त, रिक्त,
                        रिक्त, &EC);
  प्रतिज्ञा(U_SUCCESS(EC));

  करोहि {
    EC = U_ZERO_ERROR;
    ध्रुव वर्ण *निवेशः = In;

    Output = static_cast<वर्ण *>(Result.data());
    ucnv_convertEx(&*ToConvDesc, &*FromConvDesc, &Output, Result.end(), &निवेशः,
                   In + निवेशः_Length, /*pivotStart=*/रिक्त,
                   /*pivotSource=*/रिक्त, /*pivotTarget=*/रिक्त,
                   /*pivotLimit=*/रिक्त, /*reset=*/सच्च,
                   /*flush=*/सच्च, &EC);
    सचे (U_FAILURE(EC)) {
      सचे (EC == U_BUFFER_OVERFLOW_ERROR) {
        सचे (Capacity < Result.max_size()) {
          HandleOverflow(Capacity, Output, OutputLength, Result);
          अनुवर्त;
        } अञ्ञथा {
          Result.resize(Output - Result.data());
          पटिनिवत्त std::error_code(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      Result.resize(Output - Result.data());
      पटिनिवत्त std::error_code(EILSEQ, std::generic_category());
    }
    भङ्ग;
  } याव (सच्च);

  Result.resize(Output - Result.data());
  पटिनिवत्त std::error_code();
}

शून्य TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ToConvDesc);
}

#नोचेद्यदि HAVE_ICONV
वग्ग TextEncodingConverterIconv final
    : सार्वजनिक details::TextEncodingConverterImplBase {
  वग्ग UniqueIconvT {
    iconv_t ConvDesc;

  सार्वजनिक:
    operator iconv_t() ध्रुव { पटिनिवत्त ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      सचे (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &operator=(UniqueIconvT &&Other) {
      सचे (&Other != एतत्) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      पटिनिवत्त *एतत्;
    }
  };
  UniqueIconvT ConvDesc;

सार्वजनिक:
  TextEncodingConverterIconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<वर्ण> &Result) override;

  शून्य reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_code
TextEncodingConverterIconv::convertString(StringRef Source,
                                          SmallVectorImpl<वर्ण> &Result) {
  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  वर्ण *Output = static_cast<वर्ण *>(Result.data());
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);

  size_t Ret;
  // Handle errors returned from iconv().
  auto HandleError = [&Capacity, &Output, &OutputLength, &Result,
                      एतत्](size_t Ret) {
    सचे (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      सचे (errno == E2BIG && Capacity < Result.max_size()) {
        HandleOverflow(Capacity, Output, OutputLength, Result);
        // Reset converter
        reset();
        पटिनिवत्त std::error_code();
      } अञ्ञथा {
        // Some other error occurred.
        Result.resize(Output - Result.data());
        पटिनिवत्त std::error_code(errno, std::generic_category());
      }
    } अञ्ञथा {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      पटिनिवत्त std::make_error_code(std::errc::illegal_byte_sequence);
    }
  };

  करोहि {
    size_t निवेशः_Length = Source.size();
    वर्ण *निवेशः = const_cast<वर्ण *>(Source.data());
    Ret = iconv(ConvDesc, &निवेशः, &निवेशः_Length, &Output, &OutputLength);
    सचे (Ret != 0) {
      सचे (auto EC = HandleError(Ret))
        पटिनिवत्त EC;
      अनुवर्त;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, nullptr, nullptr, &Output, &OutputLength);
    सचे (Ret != 0) {
      सचे (auto EC = HandleError(Ret))
        पटिनिवत्त EC;
      अनुवर्त;
    }
    भङ्ग;
  } याव (सच्च);

  // Re-adjust size to actual size.
  Result.resize(Output - Result.data());
  पटिनिवत्त std::error_code();
}

अन्तःस्थ शून्य TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, nullptr, nullptr, nullptr, nullptr);
}

#endif // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  सचे (CPFrom == CPTo)
    पटिनिवत्त std::make_error_code(std::errc::invalid_argument);

  ConversionType Conversion;
  सचे (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8ToIBM1047;
  अञ्ञथा सचे (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047ToUTF8;
  अञ्ञथा
    पटिनिवत्त std::make_error_code(std::errc::invalid_argument);

  पटिनिवत्त TextEncodingConverter(
      std::make_unique<TextEncodingConverterTable>(Conversion));
}

ErrorOr<TextEncodingConverter> TextEncodingConverter::create(StringRef From,
                                                             StringRef To) {
  std::optional<TextEncoding> FromEncoding = getKnownEncoding(From);
  std::optional<TextEncoding> ToEncoding = getKnownEncoding(To);
  सचे (FromEncoding && ToEncoding) {
    ErrorOr<TextEncodingConverter> Converter =
        create(*FromEncoding, *ToEncoding);
    सचे (Converter)
      पटिनिवत्त Converter;
  }
#सचे HAVE_ICU
  UErrorCode EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  सचे (U_FAILURE(EC))
    पटिनिवत्त std::make_error_code(std::errc::invalid_argument);

  UConverterUniquePtr ToConvDesc(ucnv_open(To.str().c_str(), &EC));
  सचे (U_FAILURE(EC))
    पटिनिवत्त std::make_error_code(std::errc::invalid_argument);

  auto Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(ToConvDesc));
  पटिनिवत्त TextEncodingConverter(std::move(Converter));
#नोचेद्यदि HAVE_ICONV
  iconv_t ConvDesc = iconv_open(To.str().c_str(), From.str().c_str());
  सचे (ConvDesc == (iconv_t)-1)
    पटिनिवत्त std::make_error_code(std::errc::invalid_argument);
  पटिनिवत्त TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#अञ्ञथा
  पटिनिवत्त std::make_error_code(std::errc::invalid_argument);
#endif
}
