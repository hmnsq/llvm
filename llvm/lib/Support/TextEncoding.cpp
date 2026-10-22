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

#यदि HAVE_ICU
#यदि HAVE_WINDOWS_ICU
#अन्तर्भाव <icu.h>
#अन्यथा
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
  तार्किक PrevDigit = असत्य;
  प्रति (auto Ch : CSName) {
    यदि (isAlnum(Ch)) {
      Ch = toLower(Ch);
      यदि (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
        Normalized.ढकला_back(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
स्थिर std::optional<TextEncoding> getKnownEncoding(StringRef Name) {
  SmallString<16> Normalized;
  normalizeCharSetName(Name, Normalized);
  यदि (Normalized.equals("utf8"))
    प्रत्यागम TextEncoding::UTF8;
  यदि (Normalized.equals("ibm1047"))
    प्रत्यागम TextEncoding::IBM1047;
  प्रत्यागम std::nullopt;
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
वर्ग TextEncodingConverterTable final
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
    प्रत्यागम std::error_code();
  पक्ष UTF8ToIBM1047:
    प्रत्यागम ConverterEBCDIC::convertToEBCDIC(Source, Result);
  }
  llvm_unreachable("Invalid ConvType!");
  प्रत्यागम std::error_code();
}

#यदि HAVE_ICU
संरचना UConverterDeleter {
  शून्य operator()(UConverter *Converter) ध्रुव {
    यदि (Converter)
      ucnv_close(Converter);
  }
};
using UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

वर्ग TextEncodingConverterICU final
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
  size_t आदान_Length = Source.size();
  ध्रुव वर्ण *In = आदान_Length ? const_cast<वर्ण *>(Source.data()) : "";

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

  कुरु {
    EC = U_ZERO_ERROR;
    ध्रुव वर्ण *आदान = In;

    Output = static_cast<वर्ण *>(Result.data());
    ucnv_convertEx(&*ToConvDesc, &*FromConvDesc, &Output, Result.end(), &आदान,
                   In + आदान_Length, /*pivotStart=*/रिक्त,
                   /*pivotSource=*/रिक्त, /*pivotTarget=*/रिक्त,
                   /*pivotLimit=*/रिक्त, /*reset=*/सत्य,
                   /*flush=*/सत्य, &EC);
    यदि (U_FAILURE(EC)) {
      यदि (EC == U_BUFFER_OVERFLOW_ERROR) {
        यदि (Capacity < Result.max_size()) {
          HandleOverflow(Capacity, Output, OutputLength, Result);
          अनुवर्त;
        } अन्यथा {
          Result.resize(Output - Result.data());
          प्रत्यागम std::error_code(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      Result.resize(Output - Result.data());
      प्रत्यागम std::error_code(EILSEQ, std::generic_category());
    }
    भङ्ग;
  } यावत् (सत्य);

  Result.resize(Output - Result.data());
  प्रत्यागम std::error_code();
}

शून्य TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ToConvDesc);
}

#नोचेद्यदि HAVE_ICONV
वर्ग TextEncodingConverterIconv final
    : सार्वजनिक details::TextEncodingConverterImplBase {
  वर्ग UniqueIconvT {
    iconv_t ConvDesc;

  सार्वजनिक:
    operator iconv_t() ध्रुव { प्रत्यागम ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      यदि (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &operator=(UniqueIconvT &&Other) {
      यदि (&Other != एतत्) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      प्रत्यागम *एतत्;
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
    यदि (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      यदि (errno == E2BIG && Capacity < Result.max_size()) {
        HandleOverflow(Capacity, Output, OutputLength, Result);
        // Reset converter
        reset();
        प्रत्यागम std::error_code();
      } अन्यथा {
        // Some other error occurred.
        Result.resize(Output - Result.data());
        प्रत्यागम std::error_code(errno, std::generic_category());
      }
    } अन्यथा {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      प्रत्यागम std::make_error_code(std::errc::illegal_byte_sequence);
    }
  };

  कुरु {
    size_t आदान_Length = Source.size();
    वर्ण *आदान = const_cast<वर्ण *>(Source.data());
    Ret = iconv(ConvDesc, &आदान, &आदान_Length, &Output, &OutputLength);
    यदि (Ret != 0) {
      यदि (auto EC = HandleError(Ret))
        प्रत्यागम EC;
      अनुवर्त;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, nullptr, nullptr, &Output, &OutputLength);
    यदि (Ret != 0) {
      यदि (auto EC = HandleError(Ret))
        प्रत्यागम EC;
      अनुवर्त;
    }
    भङ्ग;
  } यावत् (सत्य);

  // Re-adjust size to actual size.
  Result.resize(Output - Result.data());
  प्रत्यागम std::error_code();
}

अन्तःस्थ शून्य TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, nullptr, nullptr, nullptr, nullptr);
}

#endif // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  यदि (CPFrom == CPTo)
    प्रत्यागम std::make_error_code(std::errc::invalid_argument);

  ConversionType Conversion;
  यदि (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8ToIBM1047;
  अन्यथा यदि (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047ToUTF8;
  अन्यथा
    प्रत्यागम std::make_error_code(std::errc::invalid_argument);

  प्रत्यागम TextEncodingConverter(
      std::make_unique<TextEncodingConverterTable>(Conversion));
}

ErrorOr<TextEncodingConverter> TextEncodingConverter::create(StringRef From,
                                                             StringRef To) {
  std::optional<TextEncoding> FromEncoding = getKnownEncoding(From);
  std::optional<TextEncoding> ToEncoding = getKnownEncoding(To);
  यदि (FromEncoding && ToEncoding) {
    ErrorOr<TextEncodingConverter> Converter =
        create(*FromEncoding, *ToEncoding);
    यदि (Converter)
      प्रत्यागम Converter;
  }
#यदि HAVE_ICU
  UErrorCode EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  यदि (U_FAILURE(EC))
    प्रत्यागम std::make_error_code(std::errc::invalid_argument);

  UConverterUniquePtr ToConvDesc(ucnv_open(To.str().c_str(), &EC));
  यदि (U_FAILURE(EC))
    प्रत्यागम std::make_error_code(std::errc::invalid_argument);

  auto Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(ToConvDesc));
  प्रत्यागम TextEncodingConverter(std::move(Converter));
#नोचेद्यदि HAVE_ICONV
  iconv_t ConvDesc = iconv_open(To.str().c_str(), From.str().c_str());
  यदि (ConvDesc == (iconv_t)-1)
    प्रत्यागम std::make_error_code(std::errc::invalid_argument);
  प्रत्यागम TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#अन्यथा
  प्रत्यागम std::make_error_code(std::errc::invalid_argument);
#endif
}
