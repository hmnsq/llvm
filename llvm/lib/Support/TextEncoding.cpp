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

#شامل "llvm/Support/TextEncoding.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Support/ConvertEBCDIC.h"
#شامل <system_error>

#اگر HAVE_ICU
#اگر HAVE_WINDOWS_ICU
#شامل <icu.h>
#ورنہ
#شامل <unicode/ucnv.h>
#اگر_ختم
#وگرنہ_اگر HAVE_ICONV
#شامل <iconv.h>
#اگر_ختم

استعمال نامخلا llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
ساکن خالی normalize_Char_سیٹ_Name(StringRef CSName,
                                 Small_سمتیہ_Impl<حرف> &Normalized) {
  منطقی پچھلا_Digit = جھوٹ;
  برائے (خودکار Ch : CSName) {
    اگر (isAlnum(Ch)) {
      Ch = مقصد_زیریں(Ch);
      اگر (Ch != '0' || پچھلا_Digit) {
        پچھلا_Digit = isDigit(Ch);
        Normalized.ڈالو_عقب(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
ساکن std::optional<متن_Encoding> get_معلوم_Encoding(StringRef Name) {
  SmallString<16> Normalized;
  normalize_Char_سیٹ_Name(Name, Normalized);
  اگر (Normalized.equals("utf8"))
    واپس متن_Encoding::UTF8;
  اگر (Normalized.equals("ibm1047"))
    واپس متن_Encoding::IBM1047;
  واپس std::nullopt;
}

[[maybe_غیر_مستعمل]] ساکن خالی Handle_حد_سے_تجاوز(size_t &استعداد, حرف *&Output,
                                            size_t &Output_لمبائی,
                                            Small_سمتیہ_Impl<حرف> &نتیجہ) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  استعداد =
      (استعداد < نتیجہ.max_سائز() / 2) ? 2 * استعداد : نتیجہ.max_سائز();
  نتیجہ.resize(0);
  نتیجہ.resize_for_overwrite(استعداد);
  Output = static_cast<حرف *>(نتیجہ.ڈیٹا());
  Output_لمبائی = استعداد;
}

نامخلا {
شمار ConversionType {
  UTF8_مقصد_IBM1047,
  IBM1047_مقصد_UTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
جماعت متن_Encoding_Converter_جدول final
    : عوامی details::متن_Encoding_Converter_Impl_بنیاد {
  ثابت ConversionType ConvType;

عوامی:
  متن_Encoding_Converter_جدول(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_رمز convertString(StringRef ماخذ,
                                Small_سمتیہ_Impl<حرف> &نتیجہ) override;

  خالی reset() override {}
};

std::error_رمز
متن_Encoding_Converter_جدول::convertString(StringRef ماخذ,
                                          Small_سمتیہ_Impl<حرف> &نتیجہ) {
  انتخاب (ConvType) {
  حالت IBM1047_مقصد_UTF8:
    ConverterEBCDIC::convert_مقصد_UTF8(ماخذ, نتیجہ);
    واپس std::error_رمز();
  حالت UTF8_مقصد_IBM1047:
    واپس ConverterEBCDIC::convert_مقصد_EBCDIC(ماخذ, نتیجہ);
  }
  llvm_unreachable("Invalid ConvType!");
  واپس std::error_رمز();
}

#اگر HAVE_ICU
ساخت UConverterDeleter {
  خالی عامل()(UConverter *Converter) ثابت {
    اگر (Converter)
      ucnv_close(Converter);
  }
};
استعمال UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

جماعت متن_Encoding_Converter_ICU final
    : عوامی details::متن_Encoding_Converter_Impl_بنیاد {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr مقصد_Conv_Desc;

عوامی:
  متن_Encoding_Converter_ICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr مقصد_Converter)
      : FromConvDesc(std::move(FromConverter)),
        مقصد_Conv_Desc(std::move(مقصد_Converter)) {}

  std::error_رمز convertString(StringRef ماخذ,
                                Small_سمتیہ_Impl<حرف> &نتیجہ) override;

  خالی reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_رمز
متن_Encoding_Converter_ICU::convertString(StringRef ماخذ,
                                        Small_سمتیہ_Impl<حرف> &نتیجہ) {
  // Setup the input in case it has no backing data.
  size_t داخلہ_لمبائی = ماخذ.سائز_2();
  ثابت حرف *اندر_2 = داخلہ_لمبائی ? const_cast<حرف *>(ماخذ.ڈیٹا()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t استعداد = نتیجہ.استعداد_2();
  size_t Output_لمبائی = استعداد;
  نتیجہ.resize_for_overwrite(استعداد);
  حرف *Output;
  UError_رمز EC = U_ZERO_ERROR;

  ucnv_سیٹ_مقصد_UCall_عقب(&*FromConvDesc, UCNV_مقصد_U_CALLBACK_STOP, تہی, تہی, تہی,
                      &EC);
  ucnv_سیٹ_From_UCall_عقب(&*مقصد_Conv_Desc, UCNV_FROM_U_CALLBACK_STOP, تہی, تہی,
                        تہی, &EC);
  أكد(U_SUCCESS(EC));

  کرو {
    EC = U_ZERO_ERROR;
    ثابت حرف *داخلہ = اندر_2;

    Output = static_cast<حرف *>(نتیجہ.ڈیٹا());
    ucnv_convertEx(&*مقصد_Conv_Desc, &*FromConvDesc, &Output, نتیجہ.انتہا(), &داخلہ,
                   اندر_2 + داخلہ_لمبائی, /*pivotStart=*/تہی,
                   /*pivotSource=*/تہی, /*pivotTarget=*/تہی,
                   /*pivotLimit=*/تہی, /*reset=*/سچ,
                   /*flush=*/سچ, &EC);
    اگر (U_FAILURE(EC)) {
      اگر (EC == U_بفر_حد_سے_تجاوز_ERROR) {
        اگر (استعداد < نتیجہ.max_سائز()) {
          Handle_حد_سے_تجاوز(استعداد, Output, Output_لمبائی, نتیجہ);
          جاری;
        } ورنہ {
          نتیجہ.resize(Output - نتیجہ.ڈیٹا());
          واپس std::error_رمز(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      نتیجہ.resize(Output - نتیجہ.ڈیٹا());
      واپس std::error_رمز(EILSEQ, std::generic_category());
    }
    توڑو;
  } جب (سچ);

  نتیجہ.resize(Output - نتیجہ.ڈیٹا());
  واپس std::error_رمز();
}

خالی متن_Encoding_Converter_ICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*مقصد_Conv_Desc);
}

#وگرنہ_اگر HAVE_ICONV
جماعت متن_Encoding_Converter_Iconv final
    : عوامی details::متن_Encoding_Converter_Impl_بنیاد {
  جماعت UniqueIconvT {
    iconv_t ConvDesc;

  عوامی:
    عامل iconv_t() ثابت { واپس ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      اگر (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &عامل=(UniqueIconvT &&Other) {
      اگر (&Other != یہ) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      واپس *یہ;
    }
  };
  UniqueIconvT ConvDesc;

عوامی:
  متن_Encoding_Converter_Iconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_رمز convertString(StringRef ماخذ,
                                Small_سمتیہ_Impl<حرف> &نتیجہ) override;

  خالی reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_رمز
متن_Encoding_Converter_Iconv::convertString(StringRef ماخذ,
                                          Small_سمتیہ_Impl<حرف> &نتیجہ) {
  // Setup the output. We directly write into the SmallVector.
  size_t استعداد = نتیجہ.استعداد_2();
  حرف *Output = static_cast<حرف *>(نتیجہ.ڈیٹا());
  size_t Output_لمبائی = استعداد;
  نتیجہ.resize_for_overwrite(استعداد);

  size_t Ret;
  // Handle errors returned from iconv().
  خودکار HandleError = [&استعداد, &Output, &Output_لمبائی, &نتیجہ,
                      یہ](size_t Ret) {
    اگر (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      اگر (errno == E2BIG && استعداد < نتیجہ.max_سائز()) {
        Handle_حد_سے_تجاوز(استعداد, Output, Output_لمبائی, نتیجہ);
        // Reset converter
        reset();
        واپس std::error_رمز();
      } ورنہ {
        // Some other error occurred.
        نتیجہ.resize(Output - نتیجہ.ڈیٹا());
        واپس std::error_رمز(errno, std::generic_category());
      }
    } ورنہ {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      واپس std::make_error_رمز(std::errc::illegal_byte_sequence);
    }
  };

  کرو {
    size_t داخلہ_لمبائی = ماخذ.سائز_2();
    حرف *داخلہ = const_cast<حرف *>(ماخذ.ڈیٹا());
    Ret = iconv(ConvDesc, &داخلہ, &داخلہ_لمبائی, &Output, &Output_لمبائی);
    اگر (Ret != 0) {
      اگر (خودکار EC = HandleError(Ret))
        واپس EC;
      جاری;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, خالی_اشاریہ, خالی_اشاریہ, &Output, &Output_لمبائی);
    اگر (Ret != 0) {
      اگر (خودکار EC = HandleError(Ret))
        واپس EC;
      جاری;
    }
    توڑو;
  } جب (سچ);

  // Re-adjust size to actual size.
  نتیجہ.resize(Output - نتیجہ.ڈیٹا());
  واپس std::error_رمز();
}

اندرونی خالی متن_Encoding_Converter_Iconv::reset() {
  iconv(ConvDesc, خالی_اشاریہ, خالی_اشاریہ, خالی_اشاریہ, خالی_اشاریہ);
}

#اگر_ختم // HAVE_ICONV
} // namespace

ErrorOr<متن_Encoding_Converter>
متن_Encoding_Converter::create(متن_Encoding CPFrom, متن_Encoding CPTo) {

  // Text encodings should be distinct.
  اگر (CPFrom == CPTo)
    واپس std::make_error_رمز(std::errc::invalid_دلیل);

  ConversionType Conversion;
  اگر (CPFrom == متن_Encoding::UTF8 && CPTo == متن_Encoding::IBM1047)
    Conversion = UTF8_مقصد_IBM1047;
  ورنہ اگر (CPFrom == متن_Encoding::IBM1047 && CPTo == متن_Encoding::UTF8)
    Conversion = IBM1047_مقصد_UTF8;
  ورنہ
    واپس std::make_error_رمز(std::errc::invalid_دلیل);

  واپس متن_Encoding_Converter(
      std::make_unique<متن_Encoding_Converter_جدول>(Conversion));
}

ErrorOr<متن_Encoding_Converter> متن_Encoding_Converter::create(StringRef From,
                                                             StringRef مقصد) {
  std::optional<متن_Encoding> FromEncoding = get_معلوم_Encoding(From);
  std::optional<متن_Encoding> مقصد_Encoding = get_معلوم_Encoding(مقصد);
  اگر (FromEncoding && مقصد_Encoding) {
    ErrorOr<متن_Encoding_Converter> Converter =
        create(*FromEncoding, *مقصد_Encoding);
    اگر (Converter)
      واپس Converter;
  }
#اگر HAVE_ICU
  UError_رمز EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  اگر (U_FAILURE(EC))
    واپس std::make_error_رمز(std::errc::invalid_دلیل);

  UConverterUniquePtr مقصد_Conv_Desc(ucnv_open(مقصد.str().c_str(), &EC));
  اگر (U_FAILURE(EC))
    واپس std::make_error_رمز(std::errc::invalid_دلیل);

  خودکار Converter = std::make_unique<متن_Encoding_Converter_ICU>(
      std::move(FromConvDesc), std::move(مقصد_Conv_Desc));
  واپس متن_Encoding_Converter(std::move(Converter));
#وگرنہ_اگر HAVE_ICONV
  iconv_t ConvDesc = iconv_open(مقصد.str().c_str(), From.str().c_str());
  اگر (ConvDesc == (iconv_t)-1)
    واپس std::make_error_رمز(std::errc::invalid_دلیل);
  واپس متن_Encoding_Converter(
      std::make_unique<متن_Encoding_Converter_Iconv>(ConvDesc));
#ورنہ
  واپس std::make_error_رمز(std::errc::invalid_دلیل);
#اگر_ختم
}
