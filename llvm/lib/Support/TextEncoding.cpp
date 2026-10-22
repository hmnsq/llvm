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

#অন্তর্ভাব "llvm/Support/TextEncoding.h"
#অন্তর্ভাব "llvm/ADT/SmallString.h"
#অন্তর্ভাব "llvm/ADT/SmallVector.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/Config/config.h"
#অন্তর্ভাব "llvm/Support/ConvertEBCDIC.h"
#অন্তর্ভাব <system_error>

#যদি HAVE_ICU
#যদি HAVE_WINDOWS_ICU
#অন্তর্ভাব <icu.h>
#অন্যথা
#অন্তর্ভাব <unicode/ucnv.h>
#endif
#নতুবা_যদি HAVE_ICONV
#অন্তর্ভাব <iconv.h>
#endif

using নামাকাশ llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
স্থির শূন্য normalizeCharSetName(StringRef CSName,
                                 SmallVectorImpl<অক্ষর> &Normalized) {
  যৌক্তিক পূর্ব_Digit = মিথ্যা;
  জন্য (auto Ch : CSName) {
    যদি (isAlnum(Ch)) {
      Ch = toLower(Ch);
      যদি (Ch != '0' || পূর্ব_Digit) {
        পূর্ব_Digit = isDigit(Ch);
        Normalized.ঠেলো_back(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
স্থির std::optional<TextEncoding> getKnownEncoding(StringRef Name) {
  SmallString<16> Normalized;
  normalizeCharSetName(Name, Normalized);
  যদি (Normalized.equals("utf8"))
    ফেরত TextEncoding::UTF8;
  যদি (Normalized.equals("ibm1047"))
    ফেরত TextEncoding::IBM1047;
  ফেরত std::nullopt;
}

[[maybe_unused]] স্থির শূন্য HandleOverflow(size_t &Capacity, অক্ষর *&Output,
                                            size_t &Output_দৈর্ঘ্য,
                                            SmallVectorImpl<অক্ষর> &ফলাফল) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  Capacity =
      (Capacity < ফলাফল.max_আকার() / 2) ? 2 * Capacity : ফলাফল.max_আকার();
  ফলাফল.resize(0);
  ফলাফল.resize_for_overwrite(Capacity);
  Output = static_cast<অক্ষর *>(ফলাফল.তথ্য());
  Output_দৈর্ঘ্য = Capacity;
}

নামাকাশ {
গণনা ConversionType {
  UTF8ToIBM1047,
  IBM1047ToUTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
শ্রেণী TextEncodingConverterTable final
    : সর্বজনীন details::TextEncodingConverterImplBase {
  ধ্রুবক ConversionType ConvType;

সর্বজনীন:
  TextEncodingConverterTable(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<অক্ষর> &ফলাফল) override;

  শূন্য reset() override {}
};

std::error_code
TextEncodingConverterTable::convertString(StringRef Source,
                                          SmallVectorImpl<অক্ষর> &ফলাফল) {
  নির্বাচন (ConvType) {
  ক্ষেত্র IBM1047ToUTF8:
    ConverterEBCDIC::convertToUTF8(Source, ফলাফল);
    ফেরত std::error_code();
  ক্ষেত্র UTF8ToIBM1047:
    ফেরত ConverterEBCDIC::convertToEBCDIC(Source, ফলাফল);
  }
  llvm_unreachable("Invalid ConvType!");
  ফেরত std::error_code();
}

#যদি HAVE_ICU
কাঠামো UConverterDeleter {
  শূন্য operator()(UConverter *Converter) ধ্রুবক {
    যদি (Converter)
      ucnv_close(Converter);
  }
};
using UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

শ্রেণী TextEncodingConverterICU final
    : সর্বজনীন details::TextEncodingConverterImplBase {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr ToConvDesc;

সর্বজনীন:
  TextEncodingConverterICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr ToConverter)
      : FromConvDesc(std::move(FromConverter)),
        ToConvDesc(std::move(ToConverter)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<অক্ষর> &ফলাফল) override;

  শূন্য reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_code
TextEncodingConverterICU::convertString(StringRef Source,
                                        SmallVectorImpl<অক্ষর> &ফলাফল) {
  // Setup the input in case it has no backing data.
  size_t প্রবেশ_দৈর্ঘ্য = Source.আকার_3();
  ধ্রুবক অক্ষর *In = প্রবেশ_দৈর্ঘ্য ? const_cast<অক্ষর *>(Source.তথ্য()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = ফলাফল.capacity();
  size_t Output_দৈর্ঘ্য = Capacity;
  ফলাফল.resize_for_overwrite(Capacity);
  অক্ষর *Output;
  UErrorCode EC = U_ZERO_ERROR;

  ucnv_setToUCallBack(&*FromConvDesc, UCNV_TO_U_CALLBACK_STOP, ফাঁকা, ফাঁকা, ফাঁকা,
                      &EC);
  ucnv_setFromUCallBack(&*ToConvDesc, UCNV_FROM_U_CALLBACK_STOP, ফাঁকা, ফাঁকা,
                        ফাঁকা, &EC);
  প্রতিজ্ঞা(U_SUCCESS(EC));

  করো {
    EC = U_ZERO_ERROR;
    ধ্রুবক অক্ষর *প্রবেশ = In;

    Output = static_cast<অক্ষর *>(ফলাফল.তথ্য());
    ucnv_convertEx(&*ToConvDesc, &*FromConvDesc, &Output, ফলাফল.end(), &প্রবেশ,
                   In + প্রবেশ_দৈর্ঘ্য, /*pivotStart=*/ফাঁকা,
                   /*pivotSource=*/ফাঁকা, /*pivotTarget=*/ফাঁকা,
                   /*pivotLimit=*/ফাঁকা, /*reset=*/সত্য,
                   /*flush=*/সত্য, &EC);
    যদি (U_FAILURE(EC)) {
      যদি (EC == U_বাফার_OVERFLOW_ERROR) {
        যদি (Capacity < ফলাফল.max_আকার()) {
          HandleOverflow(Capacity, Output, Output_দৈর্ঘ্য, ফলাফল);
          চালু;
        } অন্যথা {
          ফলাফল.resize(Output - ফলাফল.তথ্য());
          ফেরত std::error_code(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      ফলাফল.resize(Output - ফলাফল.তথ্য());
      ফেরত std::error_code(EILSEQ, std::generic_category());
    }
    বিরতি;
  } যখন (সত্য);

  ফলাফল.resize(Output - ফলাফল.তথ্য());
  ফেরত std::error_code();
}

শূন্য TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ToConvDesc);
}

#নতুবা_যদি HAVE_ICONV
শ্রেণী TextEncodingConverterIconv final
    : সর্বজনীন details::TextEncodingConverterImplBase {
  শ্রেণী UniqueIconvT {
    iconv_t ConvDesc;

  সর্বজনীন:
    operator iconv_t() ধ্রুবক { ফেরত ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      যদি (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &operator=(UniqueIconvT &&Other) {
      যদি (&Other != এই) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      ফেরত *এই;
    }
  };
  UniqueIconvT ConvDesc;

সর্বজনীন:
  TextEncodingConverterIconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<অক্ষর> &ফলাফল) override;

  শূন্য reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_code
TextEncodingConverterIconv::convertString(StringRef Source,
                                          SmallVectorImpl<অক্ষর> &ফলাফল) {
  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = ফলাফল.capacity();
  অক্ষর *Output = static_cast<অক্ষর *>(ফলাফল.তথ্য());
  size_t Output_দৈর্ঘ্য = Capacity;
  ফলাফল.resize_for_overwrite(Capacity);

  size_t Ret;
  // Handle errors returned from iconv().
  auto HandleError = [&Capacity, &Output, &Output_দৈর্ঘ্য, &ফলাফল,
                      এই](size_t Ret) {
    যদি (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      যদি (errno == E2BIG && Capacity < ফলাফল.max_আকার()) {
        HandleOverflow(Capacity, Output, Output_দৈর্ঘ্য, ফলাফল);
        // Reset converter
        reset();
        ফেরত std::error_code();
      } অন্যথা {
        // Some other error occurred.
        ফলাফল.resize(Output - ফলাফল.তথ্য());
        ফেরত std::error_code(errno, std::generic_category());
      }
    } অন্যথা {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      ফেরত std::make_error_code(std::errc::illegal_byte_sequence);
    }
  };

  করো {
    size_t প্রবেশ_দৈর্ঘ্য = Source.আকার_3();
    অক্ষর *প্রবেশ = const_cast<অক্ষর *>(Source.তথ্য());
    Ret = iconv(ConvDesc, &প্রবেশ, &প্রবেশ_দৈর্ঘ্য, &Output, &Output_দৈর্ঘ্য);
    যদি (Ret != 0) {
      যদি (auto EC = HandleError(Ret))
        ফেরত EC;
      চালু;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, nullptr, nullptr, &Output, &Output_দৈর্ঘ্য);
    যদি (Ret != 0) {
      যদি (auto EC = HandleError(Ret))
        ফেরত EC;
      চালু;
    }
    বিরতি;
  } যখন (সত্য);

  // Re-adjust size to actual size.
  ফলাফল.resize(Output - ফলাফল.তথ্য());
  ফেরত std::error_code();
}

অন্তঃরেখ শূন্য TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, nullptr, nullptr, nullptr, nullptr);
}

#endif // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  যদি (CPFrom == CPTo)
    ফেরত std::make_error_code(std::errc::invalid_argument);

  ConversionType Conversion;
  যদি (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8ToIBM1047;
  অন্যথা যদি (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047ToUTF8;
  অন্যথা
    ফেরত std::make_error_code(std::errc::invalid_argument);

  ফেরত TextEncodingConverter(
      std::make_unique<TextEncodingConverterTable>(Conversion));
}

ErrorOr<TextEncodingConverter> TextEncodingConverter::create(StringRef From,
                                                             StringRef To) {
  std::optional<TextEncoding> FromEncoding = getKnownEncoding(From);
  std::optional<TextEncoding> ToEncoding = getKnownEncoding(To);
  যদি (FromEncoding && ToEncoding) {
    ErrorOr<TextEncodingConverter> Converter =
        create(*FromEncoding, *ToEncoding);
    যদি (Converter)
      ফেরত Converter;
  }
#যদি HAVE_ICU
  UErrorCode EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  যদি (U_FAILURE(EC))
    ফেরত std::make_error_code(std::errc::invalid_argument);

  UConverterUniquePtr ToConvDesc(ucnv_open(To.str().c_str(), &EC));
  যদি (U_FAILURE(EC))
    ফেরত std::make_error_code(std::errc::invalid_argument);

  auto Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(ToConvDesc));
  ফেরত TextEncodingConverter(std::move(Converter));
#নতুবা_যদি HAVE_ICONV
  iconv_t ConvDesc = iconv_open(To.str().c_str(), From.str().c_str());
  যদি (ConvDesc == (iconv_t)-1)
    ফেরত std::make_error_code(std::errc::invalid_argument);
  ফেরত TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#অন্যথা
  ফেরত std::make_error_code(std::errc::invalid_argument);
#endif
}
