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
#নোচেদ্যদি HAVE_ICONV
#অন্তর্ভাব <iconv.h>
#endif

using নামাকাশ llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
স্থির শূন্য normalizeCharSetName(StringRef CSName,
                                 SmallVectorImpl<বর্ণ> &Normalized) {
  তার্কিক PrevDigit = অসত্য;
  প্রতি (auto Ch : CSName) {
    যদি (isAlnum(Ch)) {
      Ch = toLower(Ch);
      যদি (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
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
    প্রত্যাগম TextEncoding::UTF8;
  যদি (Normalized.equals("ibm1047"))
    প্রত্যাগম TextEncoding::IBM1047;
  প্রত্যাগম std::nullopt;
}

[[maybe_unused]] স্থির শূন্য HandleOverflow(size_t &Capacity, বর্ণ *&Output,
                                            size_t &OutputLength,
                                            SmallVectorImpl<বর্ণ> &Result) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  Capacity =
      (Capacity < Result.max_size() / 2) ? 2 * Capacity : Result.max_size();
  Result.resize(0);
  Result.resize_for_overwrite(Capacity);
  Output = static_cast<বর্ণ *>(Result.data());
  OutputLength = Capacity;
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
বর্গ TextEncodingConverterTable final
    : সার্বজনিক details::TextEncodingConverterImplBase {
  ধ্রুব ConversionType ConvType;

সার্বজনিক:
  TextEncodingConverterTable(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<বর্ণ> &Result) override;

  শূন্য reset() override {}
};

std::error_code
TextEncodingConverterTable::convertString(StringRef Source,
                                          SmallVectorImpl<বর্ণ> &Result) {
  বিকল্প (ConvType) {
  পক্ষ IBM1047ToUTF8:
    ConverterEBCDIC::convertToUTF8(Source, Result);
    প্রত্যাগম std::error_code();
  পক্ষ UTF8ToIBM1047:
    প্রত্যাগম ConverterEBCDIC::convertToEBCDIC(Source, Result);
  }
  llvm_unreachable("Invalid ConvType!");
  প্রত্যাগম std::error_code();
}

#যদি HAVE_ICU
সংরচনা UConverterDeleter {
  শূন্য operator()(UConverter *Converter) ধ্রুব {
    যদি (Converter)
      ucnv_close(Converter);
  }
};
using UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

বর্গ TextEncodingConverterICU final
    : সার্বজনিক details::TextEncodingConverterImplBase {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr ToConvDesc;

সার্বজনিক:
  TextEncodingConverterICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr ToConverter)
      : FromConvDesc(std::move(FromConverter)),
        ToConvDesc(std::move(ToConverter)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<বর্ণ> &Result) override;

  শূন্য reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_code
TextEncodingConverterICU::convertString(StringRef Source,
                                        SmallVectorImpl<বর্ণ> &Result) {
  // Setup the input in case it has no backing data.
  size_t প্রবেশ_Length = Source.size();
  ধ্রুব বর্ণ *In = প্রবেশ_Length ? const_cast<বর্ণ *>(Source.data()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);
  বর্ণ *Output;
  UErrorCode EC = U_ZERO_ERROR;

  ucnv_setToUCallBack(&*FromConvDesc, UCNV_TO_U_CALLBACK_STOP, রিক্ত, রিক্ত, রিক্ত,
                      &EC);
  ucnv_setFromUCallBack(&*ToConvDesc, UCNV_FROM_U_CALLBACK_STOP, রিক্ত, রিক্ত,
                        রিক্ত, &EC);
  প্রতিজ্ঞা(U_SUCCESS(EC));

  কুরু {
    EC = U_ZERO_ERROR;
    ধ্রুব বর্ণ *প্রবেশ = In;

    Output = static_cast<বর্ণ *>(Result.data());
    ucnv_convertEx(&*ToConvDesc, &*FromConvDesc, &Output, Result.end(), &প্রবেশ,
                   In + প্রবেশ_Length, /*pivotStart=*/রিক্ত,
                   /*pivotSource=*/রিক্ত, /*pivotTarget=*/রিক্ত,
                   /*pivotLimit=*/রিক্ত, /*reset=*/সত্য,
                   /*flush=*/সত্য, &EC);
    যদি (U_FAILURE(EC)) {
      যদি (EC == U_BUFFER_OVERFLOW_ERROR) {
        যদি (Capacity < Result.max_size()) {
          HandleOverflow(Capacity, Output, OutputLength, Result);
          অনুবর্ত;
        } অন্যথা {
          Result.resize(Output - Result.data());
          প্রত্যাগম std::error_code(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      Result.resize(Output - Result.data());
      প্রত্যাগম std::error_code(EILSEQ, std::generic_category());
    }
    ভঙ্গ;
  } যাবত্ (সত্য);

  Result.resize(Output - Result.data());
  প্রত্যাগম std::error_code();
}

শূন্য TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ToConvDesc);
}

#নোচেদ্যদি HAVE_ICONV
বর্গ TextEncodingConverterIconv final
    : সার্বজনিক details::TextEncodingConverterImplBase {
  বর্গ UniqueIconvT {
    iconv_t ConvDesc;

  সার্বজনিক:
    operator iconv_t() ধ্রুব { প্রত্যাগম ConvDesc; }
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
      যদি (&Other != এতত্) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      প্রত্যাগম *এতত্;
    }
  };
  UniqueIconvT ConvDesc;

সার্বজনিক:
  TextEncodingConverterIconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<বর্ণ> &Result) override;

  শূন্য reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_code
TextEncodingConverterIconv::convertString(StringRef Source,
                                          SmallVectorImpl<বর্ণ> &Result) {
  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  বর্ণ *Output = static_cast<বর্ণ *>(Result.data());
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);

  size_t Ret;
  // Handle errors returned from iconv().
  auto HandleError = [&Capacity, &Output, &OutputLength, &Result,
                      এতত্](size_t Ret) {
    যদি (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      যদি (errno == E2BIG && Capacity < Result.max_size()) {
        HandleOverflow(Capacity, Output, OutputLength, Result);
        // Reset converter
        reset();
        প্রত্যাগম std::error_code();
      } অন্যথা {
        // Some other error occurred.
        Result.resize(Output - Result.data());
        প্রত্যাগম std::error_code(errno, std::generic_category());
      }
    } অন্যথা {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      প্রত্যাগম std::make_error_code(std::errc::illegal_byte_sequence);
    }
  };

  কুরু {
    size_t প্রবেশ_Length = Source.size();
    বর্ণ *প্রবেশ = const_cast<বর্ণ *>(Source.data());
    Ret = iconv(ConvDesc, &প্রবেশ, &প্রবেশ_Length, &Output, &OutputLength);
    যদি (Ret != 0) {
      যদি (auto EC = HandleError(Ret))
        প্রত্যাগম EC;
      অনুবর্ত;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, nullptr, nullptr, &Output, &OutputLength);
    যদি (Ret != 0) {
      যদি (auto EC = HandleError(Ret))
        প্রত্যাগম EC;
      অনুবর্ত;
    }
    ভঙ্গ;
  } যাবত্ (সত্য);

  // Re-adjust size to actual size.
  Result.resize(Output - Result.data());
  প্রত্যাগম std::error_code();
}

অন্তঃস্থ শূন্য TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, nullptr, nullptr, nullptr, nullptr);
}

#endif // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  যদি (CPFrom == CPTo)
    প্রত্যাগম std::make_error_code(std::errc::invalid_argument);

  ConversionType Conversion;
  যদি (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8ToIBM1047;
  অন্যথা যদি (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047ToUTF8;
  অন্যথা
    প্রত্যাগম std::make_error_code(std::errc::invalid_argument);

  প্রত্যাগম TextEncodingConverter(
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
      প্রত্যাগম Converter;
  }
#যদি HAVE_ICU
  UErrorCode EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  যদি (U_FAILURE(EC))
    প্রত্যাগম std::make_error_code(std::errc::invalid_argument);

  UConverterUniquePtr ToConvDesc(ucnv_open(To.str().c_str(), &EC));
  যদি (U_FAILURE(EC))
    প্রত্যাগম std::make_error_code(std::errc::invalid_argument);

  auto Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(ToConvDesc));
  প্রত্যাগম TextEncodingConverter(std::move(Converter));
#নোচেদ্যদি HAVE_ICONV
  iconv_t ConvDesc = iconv_open(To.str().c_str(), From.str().c_str());
  যদি (ConvDesc == (iconv_t)-1)
    প্রত্যাগম std::make_error_code(std::errc::invalid_argument);
  প্রত্যাগম TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#অন্যথা
  প্রত্যাগম std::make_error_code(std::errc::invalid_argument);
#endif
}
