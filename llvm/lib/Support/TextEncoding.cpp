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

#અન્તર્ભાવ "llvm/Support/TextEncoding.h"
#અન્તર્ભાવ "llvm/ADT/SmallString.h"
#અન્તર્ભાવ "llvm/ADT/SmallVector.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Support/ConvertEBCDIC.h"
#અન્તર્ભાવ <system_error>

#જો HAVE_ICU
#જો HAVE_WINDOWS_ICU
#અન્તર્ભાવ <icu.h>
#નહીંતર
#અન્તર્ભાવ <unicode/ucnv.h>
#endif
#અથવાજો HAVE_ICONV
#અન્તર્ભાવ <iconv.h>
#endif

using નામાકાશ llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
સ્થિર શૂન્ય normalizeCharSetName(StringRef CSName,
                                 SmallVectorImpl<અક્ષર> &Normalized) {
  તાર્કિક PrevDigit = ખોટું;
  માટે (auto Ch : CSName) {
    જો (isAlnum(Ch)) {
      Ch = toLower(Ch);
      જો (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
        Normalized.ધકેલો_back(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
સ્થિર std::optional<TextEncoding> getKnownEncoding(StringRef Name) {
  SmallString<16> Normalized;
  normalizeCharSetName(Name, Normalized);
  જો (Normalized.equals("utf8"))
    પાછા TextEncoding::UTF8;
  જો (Normalized.equals("ibm1047"))
    પાછા TextEncoding::IBM1047;
  પાછા std::nullopt;
}

[[maybe_unused]] સ્થિર શૂન્ય HandleOverflow(size_t &Capacity, અક્ષર *&Output,
                                            size_t &OutputLength,
                                            SmallVectorImpl<અક્ષર> &Result) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  Capacity =
      (Capacity < Result.max_size() / 2) ? 2 * Capacity : Result.max_size();
  Result.resize(0);
  Result.resize_for_overwrite(Capacity);
  Output = static_cast<અક્ષર *>(Result.data());
  OutputLength = Capacity;
}

નામાકાશ {
ગણના ConversionType {
  UTF8ToIBM1047,
  IBM1047ToUTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
વર્ગ TextEncodingConverterTable final
    : જાહેર details::TextEncodingConverterImplBase {
  અચલ ConversionType ConvType;

જાહેર:
  TextEncodingConverterTable(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<અક્ષર> &Result) override;

  શૂન્ય reset() override {}
};

std::error_code
TextEncodingConverterTable::convertString(StringRef Source,
                                          SmallVectorImpl<અક્ષર> &Result) {
  પસંદગી (ConvType) {
  સ્થિતિ IBM1047ToUTF8:
    ConverterEBCDIC::convertToUTF8(Source, Result);
    પાછા std::error_code();
  સ્થિતિ UTF8ToIBM1047:
    પાછા ConverterEBCDIC::convertToEBCDIC(Source, Result);
  }
  llvm_unreachable("Invalid ConvType!");
  પાછા std::error_code();
}

#જો HAVE_ICU
સંરચના UConverterDeleter {
  શૂન્ય operator()(UConverter *Converter) અચલ {
    જો (Converter)
      ucnv_close(Converter);
  }
};
using UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

વર્ગ TextEncodingConverterICU final
    : જાહેર details::TextEncodingConverterImplBase {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr ToConvDesc;

જાહેર:
  TextEncodingConverterICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr ToConverter)
      : FromConvDesc(std::move(FromConverter)),
        ToConvDesc(std::move(ToConverter)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<અક્ષર> &Result) override;

  શૂન્ય reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_code
TextEncodingConverterICU::convertString(StringRef Source,
                                        SmallVectorImpl<અક્ષર> &Result) {
  // Setup the input in case it has no backing data.
  size_t આગત_Length = Source.size();
  અચલ અક્ષર *In = આગત_Length ? const_cast<અક્ષર *>(Source.data()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);
  અક્ષર *Output;
  UErrorCode EC = U_ZERO_ERROR;

  ucnv_setToUCallBack(&*FromConvDesc, UCNV_TO_U_CALLBACK_STOP, ખાલી, ખાલી, ખાલી,
                      &EC);
  ucnv_setFromUCallBack(&*ToConvDesc, UCNV_FROM_U_CALLBACK_STOP, ખાલી, ખાલી,
                        ખાલી, &EC);
  પ્રતિજ્ઞા(U_SUCCESS(EC));

  કરો {
    EC = U_ZERO_ERROR;
    અચલ અક્ષર *આગત = In;

    Output = static_cast<અક્ષર *>(Result.data());
    ucnv_convertEx(&*ToConvDesc, &*FromConvDesc, &Output, Result.end(), &આગત,
                   In + આગત_Length, /*pivotStart=*/ખાલી,
                   /*pivotSource=*/ખાલી, /*pivotTarget=*/ખાલી,
                   /*pivotLimit=*/ખાલી, /*reset=*/સાચું,
                   /*flush=*/સાચું, &EC);
    જો (U_FAILURE(EC)) {
      જો (EC == U_BUFFER_OVERFLOW_ERROR) {
        જો (Capacity < Result.max_size()) {
          HandleOverflow(Capacity, Output, OutputLength, Result);
          ચાલુ;
        } નહીંતર {
          Result.resize(Output - Result.data());
          પાછા std::error_code(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      Result.resize(Output - Result.data());
      પાછા std::error_code(EILSEQ, std::generic_category());
    }
    તોડો;
  } જ્યારે (સાચું);

  Result.resize(Output - Result.data());
  પાછા std::error_code();
}

શૂન્ય TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ToConvDesc);
}

#અથવાજો HAVE_ICONV
વર્ગ TextEncodingConverterIconv final
    : જાહેર details::TextEncodingConverterImplBase {
  વર્ગ UniqueIconvT {
    iconv_t ConvDesc;

  જાહેર:
    operator iconv_t() અચલ { પાછા ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      જો (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &operator=(UniqueIconvT &&Other) {
      જો (&Other != આ) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      પાછા *આ;
    }
  };
  UniqueIconvT ConvDesc;

જાહેર:
  TextEncodingConverterIconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<અક્ષર> &Result) override;

  શૂન્ય reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_code
TextEncodingConverterIconv::convertString(StringRef Source,
                                          SmallVectorImpl<અક્ષર> &Result) {
  // Setup the output. We directly write into the SmallVector.
  size_t Capacity = Result.capacity();
  અક્ષર *Output = static_cast<અક્ષર *>(Result.data());
  size_t OutputLength = Capacity;
  Result.resize_for_overwrite(Capacity);

  size_t Ret;
  // Handle errors returned from iconv().
  auto HandleError = [&Capacity, &Output, &OutputLength, &Result,
                      આ](size_t Ret) {
    જો (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      જો (errno == E2BIG && Capacity < Result.max_size()) {
        HandleOverflow(Capacity, Output, OutputLength, Result);
        // Reset converter
        reset();
        પાછા std::error_code();
      } નહીંતર {
        // Some other error occurred.
        Result.resize(Output - Result.data());
        પાછા std::error_code(errno, std::generic_category());
      }
    } નહીંતર {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      પાછા std::make_error_code(std::errc::illegal_byte_sequence);
    }
  };

  કરો {
    size_t આગત_Length = Source.size();
    અક્ષર *આગત = const_cast<અક્ષર *>(Source.data());
    Ret = iconv(ConvDesc, &આગત, &આગત_Length, &Output, &OutputLength);
    જો (Ret != 0) {
      જો (auto EC = HandleError(Ret))
        પાછા EC;
      ચાલુ;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, nullptr, nullptr, &Output, &OutputLength);
    જો (Ret != 0) {
      જો (auto EC = HandleError(Ret))
        પાછા EC;
      ચાલુ;
    }
    તોડો;
  } જ્યારે (સાચું);

  // Re-adjust size to actual size.
  Result.resize(Output - Result.data());
  પાછા std::error_code();
}

અંતઃરેખ શૂન્ય TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, nullptr, nullptr, nullptr, nullptr);
}

#endif // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  જો (CPFrom == CPTo)
    પાછા std::make_error_code(std::errc::invalid_argument);

  ConversionType Conversion;
  જો (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8ToIBM1047;
  નહીંતર જો (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047ToUTF8;
  નહીંતર
    પાછા std::make_error_code(std::errc::invalid_argument);

  પાછા TextEncodingConverter(
      std::make_unique<TextEncodingConverterTable>(Conversion));
}

ErrorOr<TextEncodingConverter> TextEncodingConverter::create(StringRef From,
                                                             StringRef To) {
  std::optional<TextEncoding> FromEncoding = getKnownEncoding(From);
  std::optional<TextEncoding> ToEncoding = getKnownEncoding(To);
  જો (FromEncoding && ToEncoding) {
    ErrorOr<TextEncodingConverter> Converter =
        create(*FromEncoding, *ToEncoding);
    જો (Converter)
      પાછા Converter;
  }
#જો HAVE_ICU
  UErrorCode EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  જો (U_FAILURE(EC))
    પાછા std::make_error_code(std::errc::invalid_argument);

  UConverterUniquePtr ToConvDesc(ucnv_open(To.str().c_str(), &EC));
  જો (U_FAILURE(EC))
    પાછા std::make_error_code(std::errc::invalid_argument);

  auto Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(ToConvDesc));
  પાછા TextEncodingConverter(std::move(Converter));
#અથવાજો HAVE_ICONV
  iconv_t ConvDesc = iconv_open(To.str().c_str(), From.str().c_str());
  જો (ConvDesc == (iconv_t)-1)
    પાછા std::make_error_code(std::errc::invalid_argument);
  પાછા TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#નહીંતર
  પાછા std::make_error_code(std::errc::invalid_argument);
#endif
}
