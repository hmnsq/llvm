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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/TextEncoding.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallVector.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ConvertEBCDIC.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <system_error>

#ਜੇ HAVE_ICU
#ਜੇ HAVE_WINDOWS_ICU
#ਅਨ੍ਤਰ੍ਭਾਵ <icu.h>
#ਨਹੀਂਤਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ <unicode/ucnv.h>
#ਜੇ_ਅੰਤ
#ਜਾਂਜੇ HAVE_ICONV
#ਅਨ੍ਤਰ੍ਭਾਵ <iconv.h>
#ਜੇ_ਅੰਤ

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
ਸਥਿਰ ਖਾਲੀ normalize_Char_ਸੈੱਟ_Name(StringRef CSName,
                                 Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &Normalized) {
  ਤਾਰਕਿਕ PrevDigit = ਝੂਠ;
  ਲਈ (ਸਵੈਚਾਲਿਤ Ch : CSName) {
    ਜੇ (isAlnum(Ch)) {
      Ch = ਮੰਜ਼ਿਲ_ਹੇਠਲਾ(Ch);
      ਜੇ (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
        Normalized.ਧੱਕੋ_ਪਿੱਠ(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
ਸਥਿਰ std::optional<ਪਾਠ_Encoding> get_ਗਿਆਤ_Encoding(StringRef Name) {
  SmallString<16> Normalized;
  normalize_Char_ਸੈੱਟ_Name(Name, Normalized);
  ਜੇ (Normalized.equals("utf8"))
    ਵਾਪਸ ਪਾਠ_Encoding::UTF8;
  ਜੇ (Normalized.equals("ibm1047"))
    ਵਾਪਸ ਪਾਠ_Encoding::IBM1047;
  ਵਾਪਸ std::nullopt;
}

[[maybe_ਅਣਵਰਤਿਆ]] ਸਥਿਰ ਖਾਲੀ Handle_ਉਛਾਲ(size_t &ਸਮਰੱਥਾ, ਅੱਖਰ *&Output,
                                            size_t &Output_ਲੰਮਾਈ,
                                            Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  ਸਮਰੱਥਾ =
      (ਸਮਰੱਥਾ < ਨਤੀਜਾ.max_ਮਾਪ() / 2) ? 2 * ਸਮਰੱਥਾ : ਨਤੀਜਾ.max_ਮਾਪ();
  ਨਤੀਜਾ.resize(0);
  ਨਤੀਜਾ.resize_for_overwrite(ਸਮਰੱਥਾ);
  Output = static_cast<ਅੱਖਰ *>(ਨਤੀਜਾ.ਅੰਕੜੇ());
  Output_ਲੰਮਾਈ = ਸਮਰੱਥਾ;
}

ਨਾਮਾਕਾਸ਼ {
ਗਿਣਤੀ ConversionType {
  UTF8_ਮੰਜ਼ਿਲ_IBM1047,
  IBM1047_ਮੰਜ਼ਿਲ_UTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
ਵਰਗ ਪਾਠ_Encoding_Converter_ਸਾਰਣੀ final
    : ਜਨਤਕ details::ਪਾਠ_Encoding_Converter_Impl_ਅਧਾਰ {
  ਅਚਲ ConversionType ConvType;

ਜਨਤਕ:
  ਪਾਠ_Encoding_Converter_ਸਾਰਣੀ(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_ਕੋਡ convertString(StringRef ਸਰੋਤ,
                                Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ) override;

  ਖਾਲੀ reset() override {}
};

std::error_ਕੋਡ
ਪਾਠ_Encoding_Converter_ਸਾਰਣੀ::convertString(StringRef ਸਰੋਤ,
                                          Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ) {
  ਚੋਣ (ConvType) {
  ਹਾਲਤ IBM1047_ਮੰਜ਼ਿਲ_UTF8:
    ConverterEBCDIC::convert_ਮੰਜ਼ਿਲ_UTF8(ਸਰੋਤ, ਨਤੀਜਾ);
    ਵਾਪਸ std::error_ਕੋਡ();
  ਹਾਲਤ UTF8_ਮੰਜ਼ਿਲ_IBM1047:
    ਵਾਪਸ ConverterEBCDIC::convert_ਮੰਜ਼ਿਲ_EBCDIC(ਸਰੋਤ, ਨਤੀਜਾ);
  }
  llvm_unreachable("Invalid ConvType!");
  ਵਾਪਸ std::error_ਕੋਡ();
}

#ਜੇ HAVE_ICU
ਬਣਤਰ UConverterDeleter {
  ਖਾਲੀ ਸੰਚਾਲਕ()(UConverter *Converter) ਅਚਲ {
    ਜੇ (Converter)
      ucnv_close(Converter);
  }
};
ਵਰਤੋਂ UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

ਵਰਗ ਪਾਠ_Encoding_Converter_ICU final
    : ਜਨਤਕ details::ਪਾਠ_Encoding_Converter_Impl_ਅਧਾਰ {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr ਮੰਜ਼ਿਲ_Conv_Desc;

ਜਨਤਕ:
  ਪਾਠ_Encoding_Converter_ICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr ਮੰਜ਼ਿਲ_Converter)
      : FromConvDesc(std::move(FromConverter)),
        ਮੰਜ਼ਿਲ_Conv_Desc(std::move(ਮੰਜ਼ਿਲ_Converter)) {}

  std::error_ਕੋਡ convertString(StringRef ਸਰੋਤ,
                                Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ) override;

  ਖਾਲੀ reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_ਕੋਡ
ਪਾਠ_Encoding_Converter_ICU::convertString(StringRef ਸਰੋਤ,
                                        Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ) {
  // Setup the input in case it has no backing data.
  size_t ਨਿਵੇਸ਼_ਲੰਮਾਈ = ਸਰੋਤ.ਮਾਪ_2();
  ਅਚਲ ਅੱਖਰ *ਵਿੱਚ_3 = ਨਿਵੇਸ਼_ਲੰਮਾਈ ? const_cast<ਅੱਖਰ *>(ਸਰੋਤ.ਅੰਕੜੇ()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t ਸਮਰੱਥਾ = ਨਤੀਜਾ.ਸਮਰੱਥਾ_2();
  size_t Output_ਲੰਮਾਈ = ਸਮਰੱਥਾ;
  ਨਤੀਜਾ.resize_for_overwrite(ਸਮਰੱਥਾ);
  ਅੱਖਰ *Output;
  UError_ਕੋਡ EC = U_ZERO_ERROR;

  ucnv_ਸੈੱਟ_ਮੰਜ਼ਿਲ_UCall_ਪਿੱਠ(&*FromConvDesc, UCNV_ਮੰਜ਼ਿਲ_U_CALLBACK_STOP, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ,
                      &EC);
  ucnv_ਸੈੱਟ_From_UCall_ਪਿੱਠ(&*ਮੰਜ਼ਿਲ_Conv_Desc, UCNV_FROM_U_CALLBACK_STOP, ਨਿਰਾ, ਨਿਰਾ,
                        ਨਿਰਾ, &EC);
  ਪ੍ਰਤਿਜ੍ਞਾ(U_SUCCESS(EC));

  ਕਰੋ {
    EC = U_ZERO_ERROR;
    ਅਚਲ ਅੱਖਰ *ਨਿਵੇਸ਼ = ਵਿੱਚ_3;

    Output = static_cast<ਅੱਖਰ *>(ਨਤੀਜਾ.ਅੰਕੜੇ());
    ucnv_convertEx(&*ਮੰਜ਼ਿਲ_Conv_Desc, &*FromConvDesc, &Output, ਨਤੀਜਾ.ਸਿਰਾ(), &ਨਿਵੇਸ਼,
                   ਵਿੱਚ_3 + ਨਿਵੇਸ਼_ਲੰਮਾਈ, /*pivotStart=*/ਨਿਰਾ,
                   /*pivotSource=*/ਨਿਰਾ, /*pivotTarget=*/ਨਿਰਾ,
                   /*pivotLimit=*/ਨਿਰਾ, /*reset=*/ਸੱਚ,
                   /*flush=*/ਸੱਚ, &EC);
    ਜੇ (U_FAILURE(EC)) {
      ਜੇ (EC == U_ਬਫਰ_ਉਛਾਲ_ERROR) {
        ਜੇ (ਸਮਰੱਥਾ < ਨਤੀਜਾ.max_ਮਾਪ()) {
          Handle_ਉਛਾਲ(ਸਮਰੱਥਾ, Output, Output_ਲੰਮਾਈ, ਨਤੀਜਾ);
          ਜਾਰੀ;
        } ਨਹੀਂਤਾਂ {
          ਨਤੀਜਾ.resize(Output - ਨਤੀਜਾ.ਅੰਕੜੇ());
          ਵਾਪਸ std::error_ਕੋਡ(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      ਨਤੀਜਾ.resize(Output - ਨਤੀਜਾ.ਅੰਕੜੇ());
      ਵਾਪਸ std::error_ਕੋਡ(EILSEQ, std::generic_category());
    }
    ਰੋਕੋ;
  } ਜਦੋਂ (ਸੱਚ);

  ਨਤੀਜਾ.resize(Output - ਨਤੀਜਾ.ਅੰਕੜੇ());
  ਵਾਪਸ std::error_ਕੋਡ();
}

ਖਾਲੀ ਪਾਠ_Encoding_Converter_ICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ਮੰਜ਼ਿਲ_Conv_Desc);
}

#ਜਾਂਜੇ HAVE_ICONV
ਵਰਗ ਪਾਠ_Encoding_Converter_Iconv final
    : ਜਨਤਕ details::ਪਾਠ_Encoding_Converter_Impl_ਅਧਾਰ {
  ਵਰਗ UniqueIconvT {
    iconv_t ConvDesc;

  ਜਨਤਕ:
    ਸੰਚਾਲਕ iconv_t() ਅਚਲ { ਵਾਪਸ ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      ਜੇ (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &ਸੰਚਾਲਕ=(UniqueIconvT &&Other) {
      ਜੇ (&Other != ਇਹ) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      ਵਾਪਸ *ਇਹ;
    }
  };
  UniqueIconvT ConvDesc;

ਜਨਤਕ:
  ਪਾਠ_Encoding_Converter_Iconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_ਕੋਡ convertString(StringRef ਸਰੋਤ,
                                Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ) override;

  ਖਾਲੀ reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_ਕੋਡ
ਪਾਠ_Encoding_Converter_Iconv::convertString(StringRef ਸਰੋਤ,
                                          Small_ਸਦਿਸ਼_Impl<ਅੱਖਰ> &ਨਤੀਜਾ) {
  // Setup the output. We directly write into the SmallVector.
  size_t ਸਮਰੱਥਾ = ਨਤੀਜਾ.ਸਮਰੱਥਾ_2();
  ਅੱਖਰ *Output = static_cast<ਅੱਖਰ *>(ਨਤੀਜਾ.ਅੰਕੜੇ());
  size_t Output_ਲੰਮਾਈ = ਸਮਰੱਥਾ;
  ਨਤੀਜਾ.resize_for_overwrite(ਸਮਰੱਥਾ);

  size_t Ret;
  // Handle errors returned from iconv().
  ਸਵੈਚਾਲਿਤ HandleError = [&ਸਮਰੱਥਾ, &Output, &Output_ਲੰਮਾਈ, &ਨਤੀਜਾ,
                      ਇਹ](size_t Ret) {
    ਜੇ (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      ਜੇ (errno == E2BIG && ਸਮਰੱਥਾ < ਨਤੀਜਾ.max_ਮਾਪ()) {
        Handle_ਉਛਾਲ(ਸਮਰੱਥਾ, Output, Output_ਲੰਮਾਈ, ਨਤੀਜਾ);
        // Reset converter
        reset();
        ਵਾਪਸ std::error_ਕੋਡ();
      } ਨਹੀਂਤਾਂ {
        // Some other error occurred.
        ਨਤੀਜਾ.resize(Output - ਨਤੀਜਾ.ਅੰਕੜੇ());
        ਵਾਪਸ std::error_ਕੋਡ(errno, std::generic_category());
      }
    } ਨਹੀਂਤਾਂ {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      ਵਾਪਸ std::make_error_ਕੋਡ(std::errc::illegal_byte_sequence);
    }
  };

  ਕਰੋ {
    size_t ਨਿਵੇਸ਼_ਲੰਮਾਈ = ਸਰੋਤ.ਮਾਪ_2();
    ਅੱਖਰ *ਨਿਵੇਸ਼ = const_cast<ਅੱਖਰ *>(ਸਰੋਤ.ਅੰਕੜੇ());
    Ret = iconv(ConvDesc, &ਨਿਵੇਸ਼, &ਨਿਵੇਸ਼_ਲੰਮਾਈ, &Output, &Output_ਲੰਮਾਈ);
    ਜੇ (Ret != 0) {
      ਜੇ (ਸਵੈਚਾਲਿਤ EC = HandleError(Ret))
        ਵਾਪਸ EC;
      ਜਾਰੀ;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, ਖਾਲੀ_ਸੂਚਕ, ਖਾਲੀ_ਸੂਚਕ, &Output, &Output_ਲੰਮਾਈ);
    ਜੇ (Ret != 0) {
      ਜੇ (ਸਵੈਚਾਲਿਤ EC = HandleError(Ret))
        ਵਾਪਸ EC;
      ਜਾਰੀ;
    }
    ਰੋਕੋ;
  } ਜਦੋਂ (ਸੱਚ);

  // Re-adjust size to actual size.
  ਨਤੀਜਾ.resize(Output - ਨਤੀਜਾ.ਅੰਕੜੇ());
  ਵਾਪਸ std::error_ਕੋਡ();
}

ਸਤਰਅੰਦਰ ਖਾਲੀ ਪਾਠ_Encoding_Converter_Iconv::reset() {
  iconv(ConvDesc, ਖਾਲੀ_ਸੂਚਕ, ਖਾਲੀ_ਸੂਚਕ, ਖਾਲੀ_ਸੂਚਕ, ਖਾਲੀ_ਸੂਚਕ);
}

#ਜੇ_ਅੰਤ // HAVE_ICONV
} // namespace

ErrorOr<ਪਾਠ_Encoding_Converter>
ਪਾਠ_Encoding_Converter::create(ਪਾਠ_Encoding CPFrom, ਪਾਠ_Encoding CPTo) {

  // Text encodings should be distinct.
  ਜੇ (CPFrom == CPTo)
    ਵਾਪਸ std::make_error_ਕੋਡ(std::errc::invalid_ਦਲੀਲ);

  ConversionType Conversion;
  ਜੇ (CPFrom == ਪਾਠ_Encoding::UTF8 && CPTo == ਪਾਠ_Encoding::IBM1047)
    Conversion = UTF8_ਮੰਜ਼ਿਲ_IBM1047;
  ਨਹੀਂਤਾਂ ਜੇ (CPFrom == ਪਾਠ_Encoding::IBM1047 && CPTo == ਪਾਠ_Encoding::UTF8)
    Conversion = IBM1047_ਮੰਜ਼ਿਲ_UTF8;
  ਨਹੀਂਤਾਂ
    ਵਾਪਸ std::make_error_ਕੋਡ(std::errc::invalid_ਦਲੀਲ);

  ਵਾਪਸ ਪਾਠ_Encoding_Converter(
      std::make_unique<ਪਾਠ_Encoding_Converter_ਸਾਰਣੀ>(Conversion));
}

ErrorOr<ਪਾਠ_Encoding_Converter> ਪਾਠ_Encoding_Converter::create(StringRef From,
                                                             StringRef ਮੰਜ਼ਿਲ) {
  std::optional<ਪਾਠ_Encoding> FromEncoding = get_ਗਿਆਤ_Encoding(From);
  std::optional<ਪਾਠ_Encoding> ਮੰਜ਼ਿਲ_Encoding = get_ਗਿਆਤ_Encoding(ਮੰਜ਼ਿਲ);
  ਜੇ (FromEncoding && ਮੰਜ਼ਿਲ_Encoding) {
    ErrorOr<ਪਾਠ_Encoding_Converter> Converter =
        create(*FromEncoding, *ਮੰਜ਼ਿਲ_Encoding);
    ਜੇ (Converter)
      ਵਾਪਸ Converter;
  }
#ਜੇ HAVE_ICU
  UError_ਕੋਡ EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  ਜੇ (U_FAILURE(EC))
    ਵਾਪਸ std::make_error_ਕੋਡ(std::errc::invalid_ਦਲੀਲ);

  UConverterUniquePtr ਮੰਜ਼ਿਲ_Conv_Desc(ucnv_open(ਮੰਜ਼ਿਲ.str().c_str(), &EC));
  ਜੇ (U_FAILURE(EC))
    ਵਾਪਸ std::make_error_ਕੋਡ(std::errc::invalid_ਦਲੀਲ);

  ਸਵੈਚਾਲਿਤ Converter = std::make_unique<ਪਾਠ_Encoding_Converter_ICU>(
      std::move(FromConvDesc), std::move(ਮੰਜ਼ਿਲ_Conv_Desc));
  ਵਾਪਸ ਪਾਠ_Encoding_Converter(std::move(Converter));
#ਜਾਂਜੇ HAVE_ICONV
  iconv_t ConvDesc = iconv_open(ਮੰਜ਼ਿਲ.str().c_str(), From.str().c_str());
  ਜੇ (ConvDesc == (iconv_t)-1)
    ਵਾਪਸ std::make_error_ਕੋਡ(std::errc::invalid_ਦਲੀਲ);
  ਵਾਪਸ ਪਾਠ_Encoding_Converter(
      std::make_unique<ਪਾਠ_Encoding_Converter_Iconv>(ConvDesc));
#ਨਹੀਂਤਾਂ
  ਵਾਪਸ std::make_error_ਕੋਡ(std::errc::invalid_ਦਲੀਲ);
#ਜੇ_ਅੰਤ
}
