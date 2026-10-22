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

#जर HAVE_ICU
#जर HAVE_WINDOWS_ICU
#अन्तर्भाव <icu.h>
#अन्यथा
#अन्तर्भाव <unicode/ucnv.h>
#endif
#किंवाजर HAVE_ICONV
#अन्तर्भाव <iconv.h>
#endif

using नामाकाश llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
स्थिर शून्य normalizeCharSetName(StringRef CSName,
                                 SmallVectorImpl<अक्षर> &Normalized) {
  तार्किक PrevDigit = असत्य;
  साठी (स्वतः Ch : CSName) {
    जर (isAlnum(Ch)) {
      Ch = toLower(Ch);
      जर (Ch != '0' || PrevDigit) {
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
  जर (Normalized.equals("utf8"))
    परत TextEncoding::UTF8;
  जर (Normalized.equals("ibm1047"))
    परत TextEncoding::IBM1047;
  परत std::nullopt;
}

[[maybe_unused]] स्थिर शून्य HandleOverflow(माप_प्रकार &Capacity, अक्षर *&Output,
                                            माप_प्रकार &OutputLength,
                                            SmallVectorImpl<अक्षर> &Result) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  Capacity =
      (Capacity < Result.max_size() / 2) ? 2 * Capacity : Result.max_size();
  Result.resize(0);
  Result.resize_for_overwrite(Capacity);
  Output = static_cast<अक्षर *>(Result.data());
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
  स्थिरांक ConversionType ConvType;

सार्वजनिक:
  TextEncodingConverterTable(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<अक्षर> &Result) override;

  शून्य reset() override {}
};

std::error_code
TextEncodingConverterTable::convertString(StringRef Source,
                                          SmallVectorImpl<अक्षर> &Result) {
  निवड (ConvType) {
  स्थिती IBM1047ToUTF8:
    ConverterEBCDIC::convertToUTF8(Source, Result);
    परत std::error_code();
  स्थिती UTF8ToIBM1047:
    परत ConverterEBCDIC::convertToEBCDIC(Source, Result);
  }
  llvm_unreachable("Invalid ConvType!");
  परत std::error_code();
}

#जर HAVE_ICU
संरचना UConverterDeleter {
  शून्य चालक()(UConverter *Converter) स्थिरांक {
    जर (Converter)
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
                                SmallVectorImpl<अक्षर> &Result) override;

  शून्य reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_code
TextEncodingConverterICU::convertString(StringRef Source,
                                        SmallVectorImpl<अक्षर> &Result) {
  // Setup the input in case it has no backing data.
  माप_प्रकार आदान_Length = Source.size();
  स्थिरांक अक्षर *In = आदान_Length ? const_cast<अक्षर *>(Source.data()) : "";

  // Setup the output. We directly write into the SmallVector.
  माप_प्रकार Capacity = Result.capacity();
  माप_प्रकार OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);
  अक्षर *Output;
  UErrorCode EC = U_ZERO_ERROR;

  ucnv_setToUCallBack(&*FromConvDesc, UCNV_TO_U_CALLBACK_STOP, रिक्त, रिक्त, रिक्त,
                      &EC);
  ucnv_setFromUCallBack(&*ToConvDesc, UCNV_FROM_U_CALLBACK_STOP, रिक्त, रिक्त,
                        रिक्त, &EC);
  प्रतिज्ञा(U_SUCCESS(EC));

  करा {
    EC = U_ZERO_ERROR;
    स्थिरांक अक्षर *आदान = In;

    Output = static_cast<अक्षर *>(Result.data());
    ucnv_convertEx(&*ToConvDesc, &*FromConvDesc, &Output, Result.end(), &आदान,
                   In + आदान_Length, /*pivotStart=*/रिक्त,
                   /*pivotSource=*/रिक्त, /*pivotTarget=*/रिक्त,
                   /*pivotLimit=*/रिक्त, /*reset=*/सत्य,
                   /*flush=*/सत्य, &EC);
    जर (U_FAILURE(EC)) {
      जर (EC == U_BUFFER_OVERFLOW_ERROR) {
        जर (Capacity < Result.max_size()) {
          HandleOverflow(Capacity, Output, OutputLength, Result);
          सुरूठेवा;
        } अन्यथा {
          Result.resize(Output - Result.data());
          परत std::error_code(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      Result.resize(Output - Result.data());
      परत std::error_code(EILSEQ, std::generic_category());
    }
    थांबा;
  } पर्यंत (सत्य);

  Result.resize(Output - Result.data());
  परत std::error_code();
}

शून्य TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ToConvDesc);
}

#किंवाजर HAVE_ICONV
वर्ग TextEncodingConverterIconv final
    : सार्वजनिक details::TextEncodingConverterImplBase {
  वर्ग UniqueIconvT {
    iconv_t ConvDesc;

  सार्वजनिक:
    चालक iconv_t() स्थिरांक { परत ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      जर (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &चालक=(UniqueIconvT &&Other) {
      जर (&Other != हे) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      परत *हे;
    }
  };
  UniqueIconvT ConvDesc;

सार्वजनिक:
  TextEncodingConverterIconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<अक्षर> &Result) override;

  शून्य reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_code
TextEncodingConverterIconv::convertString(StringRef Source,
                                          SmallVectorImpl<अक्षर> &Result) {
  // Setup the output. We directly write into the SmallVector.
  माप_प्रकार Capacity = Result.capacity();
  अक्षर *Output = static_cast<अक्षर *>(Result.data());
  माप_प्रकार OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);

  माप_प्रकार Ret;
  // Handle errors returned from iconv().
  स्वतः HandleError = [&Capacity, &Output, &OutputLength, &Result,
                      हे](माप_प्रकार Ret) {
    जर (Ret == static_cast<माप_प्रकार>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      जर (त्रुटि_सं == E2BIG && Capacity < Result.max_size()) {
        HandleOverflow(Capacity, Output, OutputLength, Result);
        // Reset converter
        reset();
        परत std::error_code();
      } अन्यथा {
        // Some other error occurred.
        Result.resize(Output - Result.data());
        परत std::error_code(त्रुटि_सं, std::generic_category());
      }
    } अन्यथा {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      परत std::make_error_code(std::errc::illegal_byte_sequence);
    }
  };

  करा {
    माप_प्रकार आदान_Length = Source.size();
    अक्षर *आदान = const_cast<अक्षर *>(Source.data());
    Ret = iconv(ConvDesc, &आदान, &आदान_Length, &Output, &OutputLength);
    जर (Ret != 0) {
      जर (स्वतः EC = HandleError(Ret))
        परत EC;
      सुरूठेवा;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, nullptr, nullptr, &Output, &OutputLength);
    जर (Ret != 0) {
      जर (स्वतः EC = HandleError(Ret))
        परत EC;
      सुरूठेवा;
    }
    थांबा;
  } पर्यंत (सत्य);

  // Re-adjust size to actual size.
  Result.resize(Output - Result.data());
  परत std::error_code();
}

ओळीत शून्य TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, nullptr, nullptr, nullptr, nullptr);
}

#endif // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  जर (CPFrom == CPTo)
    परत std::make_error_code(std::errc::invalid_argument);

  ConversionType Conversion;
  जर (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8ToIBM1047;
  अन्यथा जर (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047ToUTF8;
  अन्यथा
    परत std::make_error_code(std::errc::invalid_argument);

  परत TextEncodingConverter(
      std::make_unique<TextEncodingConverterTable>(Conversion));
}

ErrorOr<TextEncodingConverter> TextEncodingConverter::create(StringRef From,
                                                             StringRef To) {
  std::optional<TextEncoding> FromEncoding = getKnownEncoding(From);
  std::optional<TextEncoding> ToEncoding = getKnownEncoding(To);
  जर (FromEncoding && ToEncoding) {
    ErrorOr<TextEncodingConverter> Converter =
        create(*FromEncoding, *ToEncoding);
    जर (Converter)
      परत Converter;
  }
#जर HAVE_ICU
  UErrorCode EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  जर (U_FAILURE(EC))
    परत std::make_error_code(std::errc::invalid_argument);

  UConverterUniquePtr ToConvDesc(ucnv_open(To.str().c_str(), &EC));
  जर (U_FAILURE(EC))
    परत std::make_error_code(std::errc::invalid_argument);

  स्वतः Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(ToConvDesc));
  परत TextEncodingConverter(std::move(Converter));
#किंवाजर HAVE_ICONV
  iconv_t ConvDesc = iconv_open(To.str().c_str(), From.str().c_str());
  जर (ConvDesc == (iconv_t)-1)
    परत std::make_error_code(std::errc::invalid_argument);
  परत TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#अन्यथा
  परत std::make_error_code(std::errc::invalid_argument);
#endif
}
