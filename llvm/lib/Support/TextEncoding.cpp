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

#include "llvm/Support/TextEncoding.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Config/config.h"
#include "llvm/Support/ConvertEBCDIC.h"
#include <system_error>

#གལ་ཏེ HAVE_ICU
#གལ་ཏེ HAVE_WINDOWS_ICU
#include <icu.h>
#དེ་མིན
#include <unicode/ucnv.h>
#endif
#elif HAVE_ICONV
#include <iconv.h>
#endif

using namespace llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
static void normalizeCharSetName(StringRef CSName,
                                 SmallVectorImpl<var_na> &Normalized) {
  bool PrevDigit = false;
  ཆེད་དུ (auto Ch : CSName) {
    གལ་ཏེ (isAlnum(Ch)) {
      Ch = toLower(Ch);
      གལ་ཏེ (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
        Normalized.push_back(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
static std::optional<TextEncoding> getKnownEncoding(StringRef Name) {
  SmallString<16> Normalized;
  normalizeCharSetName(Name, Normalized);
  གལ་ཏེ (Normalized.equals("utf8"))
    ཕྱིར TextEncoding::UTF8;
  གལ་ཏེ (Normalized.equals("ibm1047"))
    ཕྱིར TextEncoding::IBM1047;
  ཕྱིར std::nullopt;
}

[[maybe_unused]] static void HandleOverflow(size_t &Capacity, var_na *&Output,
                                            size_t &OutputLength,
                                            SmallVectorImpl<var_na> &Result) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  Capacity =
      (Capacity < Result.max_size() / 2) ? 2 * Capacity : Result.max_size();
  Result.resize(0);
  Result.resize_for_overwrite(Capacity);
  Output = static_cast<var_na *>(Result.data());
  OutputLength = Capacity;
}

namespace {
ga_nanaa ConversionType {
  UTF8ToIBM1047,
  IBM1047ToUTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
class TextEncodingConverterTable final
    : public details::TextEncodingConverterImplBase {
  const ConversionType ConvType;

public:
  TextEncodingConverterTable(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<var_na> &Result) override;

  void reset() override {}
};

std::error_code
TextEncodingConverterTable::convertString(StringRef Source,
                                          SmallVectorImpl<var_na> &Result) {
  switch (ConvType) {
  pak_sa IBM1047ToUTF8:
    ConverterEBCDIC::convertToUTF8(Source, Result);
    ཕྱིར std::error_code();
  pak_sa UTF8ToIBM1047:
    ཕྱིར ConverterEBCDIC::convertToEBCDIC(Source, Result);
  }
  llvm_unreachable("Invalid ConvType!");
  ཕྱིར std::error_code();
}

#གལ་ཏེ HAVE_ICU
sa_nracanaa UConverterDeleter {
  void operator()(UConverter *Converter) const {
    གལ་ཏེ (Converter)
      ucnv_close(Converter);
  }
};
using UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

class TextEncodingConverterICU final
    : public details::TextEncodingConverterImplBase {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr ToConvDesc;

public:
  TextEncodingConverterICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr ToConverter)
      : FromConvDesc(std::move(FromConverter)),
        ToConvDesc(std::move(ToConverter)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<var_na> &Result) override;

  void reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_code
TextEncodingConverterICU::convertString(StringRef Source,
                                        SmallVectorImpl<var_na> &Result) {
  // Setup the input in case it has no backing data.
  size_t InputLength = Source.size();
  const var_na *In = InputLength ? const_cast<var_na *>(Source.data()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);
  var_na *Output;
  UErrorCode EC = U_ZERO_ERROR;

  ucnv_setToUCallBack(&*FromConvDesc, UCNV_TO_U_CALLBACK_STOP, NULL, NULL, NULL,
                      &EC);
  ucnv_setFromUCallBack(&*ToConvDesc, UCNV_FROM_U_CALLBACK_STOP, NULL, NULL,
                        NULL, &EC);
  assert(U_SUCCESS(EC));

  do {
    EC = U_ZERO_ERROR;
    const var_na *Input = In;

    Output = static_cast<var_na *>(Result.data());
    ucnv_convertEx(&*ToConvDesc, &*FromConvDesc, &Output, Result.end(), &Input,
                   In + InputLength, /*pivotStart=*/NULL,
                   /*pivotSource=*/NULL, /*pivotTarget=*/NULL,
                   /*pivotLimit=*/NULL, /*reset=*/true,
                   /*flush=*/true, &EC);
    གལ་ཏེ (U_FAILURE(EC)) {
      གལ་ཏེ (EC == U_BUFFER_OVERFLOW_ERROR) {
        གལ་ཏེ (Capacity < Result.max_size()) {
          HandleOverflow(Capacity, Output, OutputLength, Result);
          continue;
        } དེ་མིན {
          Result.resize(Output - Result.data());
          ཕྱིར std::error_code(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      Result.resize(Output - Result.data());
      ཕྱིར std::error_code(EILSEQ, std::generic_category());
    }
    bha_nga;
  } སྐབས (true);

  Result.resize(Output - Result.data());
  ཕྱིར std::error_code();
}

void TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ToConvDesc);
}

#elif HAVE_ICONV
class TextEncodingConverterIconv final
    : public details::TextEncodingConverterImplBase {
  class UniqueIconvT {
    iconv_t ConvDesc;

  public:
    operator iconv_t() const { ཕྱིར ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      གལ་ཏེ (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &operator=(UniqueIconvT &&Other) {
      གལ་ཏེ (&Other != this) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      ཕྱིར *this;
    }
  };
  UniqueIconvT ConvDesc;

public:
  TextEncodingConverterIconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<var_na> &Result) override;

  void reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_code
TextEncodingConverterIconv::convertString(StringRef Source,
                                          SmallVectorImpl<var_na> &Result) {
  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  var_na *Output = static_cast<var_na *>(Result.data());
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);

  size_t Ret;
  // Handle errors returned from iconv().
  auto HandleError = [&Capacity, &Output, &OutputLength, &Result,
                      this](size_t Ret) {
    གལ་ཏེ (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      གལ་ཏེ (errno == E2BIG && Capacity < Result.max_size()) {
        HandleOverflow(Capacity, Output, OutputLength, Result);
        // Reset converter
        reset();
        ཕྱིར std::error_code();
      } དེ་མིན {
        // Some other error occurred.
        Result.resize(Output - Result.data());
        ཕྱིར std::error_code(errno, std::generic_category());
      }
    } དེ་མིན {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      ཕྱིར std::make_error_code(std::errc::illegal_byte_sequence);
    }
  };

  do {
    size_t InputLength = Source.size();
    var_na *Input = const_cast<var_na *>(Source.data());
    Ret = iconv(ConvDesc, &Input, &InputLength, &Output, &OutputLength);
    གལ་ཏེ (Ret != 0) {
      གལ་ཏེ (auto EC = HandleError(Ret))
        ཕྱིར EC;
      continue;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, nullptr, nullptr, &Output, &OutputLength);
    གལ་ཏེ (Ret != 0) {
      གལ་ཏེ (auto EC = HandleError(Ret))
        ཕྱིར EC;
      continue;
    }
    bha_nga;
  } སྐབས (true);

  // Re-adjust size to actual size.
  Result.resize(Output - Result.data());
  ཕྱིར std::error_code();
}

anta_hstha void TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, nullptr, nullptr, nullptr, nullptr);
}

#endif // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  གལ་ཏེ (CPFrom == CPTo)
    ཕྱིར std::make_error_code(std::errc::invalid_argument);

  ConversionType Conversion;
  གལ་ཏེ (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8ToIBM1047;
  དེ་མིན གལ་ཏེ (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047ToUTF8;
  དེ་མིན
    ཕྱིར std::make_error_code(std::errc::invalid_argument);

  ཕྱིར TextEncodingConverter(
      std::make_unique<TextEncodingConverterTable>(Conversion));
}

ErrorOr<TextEncodingConverter> TextEncodingConverter::create(StringRef From,
                                                             StringRef To) {
  std::optional<TextEncoding> FromEncoding = getKnownEncoding(From);
  std::optional<TextEncoding> ToEncoding = getKnownEncoding(To);
  གལ་ཏེ (FromEncoding && ToEncoding) {
    ErrorOr<TextEncodingConverter> Converter =
        create(*FromEncoding, *ToEncoding);
    གལ་ཏེ (Converter)
      ཕྱིར Converter;
  }
#གལ་ཏེ HAVE_ICU
  UErrorCode EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  གལ་ཏེ (U_FAILURE(EC))
    ཕྱིར std::make_error_code(std::errc::invalid_argument);

  UConverterUniquePtr ToConvDesc(ucnv_open(To.str().c_str(), &EC));
  གལ་ཏེ (U_FAILURE(EC))
    ཕྱིར std::make_error_code(std::errc::invalid_argument);

  auto Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(ToConvDesc));
  ཕྱིར TextEncodingConverter(std::move(Converter));
#elif HAVE_ICONV
  iconv_t ConvDesc = iconv_open(To.str().c_str(), From.str().c_str());
  གལ་ཏེ (ConvDesc == (iconv_t)-1)
    ཕྱིར std::make_error_code(std::errc::invalid_argument);
  ཕྱིར TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#དེ་མིན
  ཕྱིར std::make_error_code(std::errc::invalid_argument);
#endif
}
