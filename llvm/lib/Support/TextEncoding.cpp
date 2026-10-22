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

#ضمن "llvm/Support/TextEncoding.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/ConvertEBCDIC.h"
#ضمن <system_error>

#إذا HAVE_ICU
#إذا HAVE_WINDOWS_ICU
#ضمن <icu.h>
#وإلا
#ضمن <unicode/ucnv.h>
#جيڪڏهن_پورو
#وإلاإذا HAVE_ICONV
#ضمن <iconv.h>
#جيڪڏهن_پورو

استعمال نطاق llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
ساكن فراغ normalize_Char_سيٽ_Name(StringRef CSName,
                                 Small_ويڪٽر_Impl<حرف> &Normalized) {
  منطقي PrevDigit = خطأ;
  لكل (خودڪار Ch : CSName) {
    إذا (isAlnum(Ch)) {
      Ch = منزل_هيٺاهون(Ch);
      إذا (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
        Normalized.ڌڪو_پٺ(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
ساكن std::optional<متن_Encoding> get_معلوم_Encoding(StringRef Name) {
  SmallString<16> Normalized;
  normalize_Char_سيٽ_Name(Name, Normalized);
  إذا (Normalized.equals("utf8"))
    أرجع متن_Encoding::UTF8;
  إذا (Normalized.equals("ibm1047"))
    أرجع متن_Encoding::IBM1047;
  أرجع std::nullopt;
}

[[maybe_اڻ_استعمال_ٿيل]] ساكن فراغ Handle_حد_کان_مٿي(size_t &صلاحيت, حرف *&Output,
                                            size_t &Output_ڊيگهائي,
                                            Small_ويڪٽر_Impl<حرف> &نتيجو) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  صلاحيت =
      (صلاحيت < نتيجو.max_سائيز() / 2) ? 2 * صلاحيت : نتيجو.max_سائيز();
  نتيجو.resize(0);
  نتيجو.resize_for_overwrite(صلاحيت);
  Output = static_cast<حرف *>(نتيجو.معلومات());
  Output_ڊيگهائي = صلاحيت;
}

نطاق {
تعداد ConversionType {
  UTF8_منزل_IBM1047,
  IBM1047_منزل_UTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
صنف متن_Encoding_Converter_جدول final
    : عام details::متن_Encoding_Converter_Impl_بنياد {
  ثابت ConversionType ConvType;

عام:
  متن_Encoding_Converter_جدول(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_ڪوڊ convertString(StringRef ذريعو,
                                Small_ويڪٽر_Impl<حرف> &نتيجو) override;

  فراغ reset() override {}
};

std::error_ڪوڊ
متن_Encoding_Converter_جدول::convertString(StringRef ذريعو,
                                          Small_ويڪٽر_Impl<حرف> &نتيجو) {
  بدل (ConvType) {
  حالة IBM1047_منزل_UTF8:
    ConverterEBCDIC::convert_منزل_UTF8(ذريعو, نتيجو);
    أرجع std::error_ڪوڊ();
  حالة UTF8_منزل_IBM1047:
    أرجع ConverterEBCDIC::convert_منزل_EBCDIC(ذريعو, نتيجو);
  }
  llvm_unreachable("Invalid ConvType!");
  أرجع std::error_ڪوڊ();
}

#إذا HAVE_ICU
بنية UConverterDeleter {
  فراغ عامل()(UConverter *Converter) ثابت {
    إذا (Converter)
      ucnv_close(Converter);
  }
};
استعمال UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

صنف متن_Encoding_Converter_ICU final
    : عام details::متن_Encoding_Converter_Impl_بنياد {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr منزل_Conv_Desc;

عام:
  متن_Encoding_Converter_ICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr منزل_Converter)
      : FromConvDesc(std::move(FromConverter)),
        منزل_Conv_Desc(std::move(منزل_Converter)) {}

  std::error_ڪوڊ convertString(StringRef ذريعو,
                                Small_ويڪٽر_Impl<حرف> &نتيجو) override;

  فراغ reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_ڪوڊ
متن_Encoding_Converter_ICU::convertString(StringRef ذريعو,
                                        Small_ويڪٽر_Impl<حرف> &نتيجو) {
  // Setup the input in case it has no backing data.
  size_t داخلا_ڊيگهائي = ذريعو.سائيز_2();
  ثابت حرف *اندريون_2 = داخلا_ڊيگهائي ? const_cast<حرف *>(ذريعو.معلومات()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t صلاحيت = نتيجو.صلاحيت_2();
  size_t Output_ڊيگهائي = صلاحيت;
  نتيجو.resize_for_overwrite(صلاحيت);
  حرف *Output;
  UError_ڪوڊ EC = U_ZERO_ERROR;

  ucnv_سيٽ_منزل_UCall_پٺ(&*FromConvDesc, UCNV_منزل_U_CALLBACK_STOP, عدم, عدم, عدم,
                      &EC);
  ucnv_سيٽ_From_UCall_پٺ(&*منزل_Conv_Desc, UCNV_FROM_U_CALLBACK_STOP, عدم, عدم,
                        عدم, &EC);
  أكد(U_SUCCESS(EC));

  افعل {
    EC = U_ZERO_ERROR;
    ثابت حرف *داخلا = اندريون_2;

    Output = static_cast<حرف *>(نتيجو.معلومات());
    ucnv_convertEx(&*منزل_Conv_Desc, &*FromConvDesc, &Output, نتيجو.پڇاڙي(), &داخلا,
                   اندريون_2 + داخلا_ڊيگهائي, /*pivotStart=*/عدم,
                   /*pivotSource=*/عدم, /*pivotTarget=*/عدم,
                   /*pivotLimit=*/عدم, /*reset=*/سچ,
                   /*flush=*/سچ, &EC);
    إذا (U_FAILURE(EC)) {
      إذا (EC == U_بفر_حد_کان_مٿي_ERROR) {
        إذا (صلاحيت < نتيجو.max_سائيز()) {
          Handle_حد_کان_مٿي(صلاحيت, Output, Output_ڊيگهائي, نتيجو);
          استمر;
        } وإلا {
          نتيجو.resize(Output - نتيجو.معلومات());
          أرجع std::error_ڪوڊ(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      نتيجو.resize(Output - نتيجو.معلومات());
      أرجع std::error_ڪوڊ(EILSEQ, std::generic_category());
    }
    اكسر;
  } طالما (سچ);

  نتيجو.resize(Output - نتيجو.معلومات());
  أرجع std::error_ڪوڊ();
}

فراغ متن_Encoding_Converter_ICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*منزل_Conv_Desc);
}

#وإلاإذا HAVE_ICONV
صنف متن_Encoding_Converter_Iconv final
    : عام details::متن_Encoding_Converter_Impl_بنياد {
  صنف UniqueIconvT {
    iconv_t ConvDesc;

  عام:
    عامل iconv_t() ثابت { أرجع ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      إذا (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &عامل=(UniqueIconvT &&Other) {
      إذا (&Other != هذا) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      أرجع *هذا;
    }
  };
  UniqueIconvT ConvDesc;

عام:
  متن_Encoding_Converter_Iconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_ڪوڊ convertString(StringRef ذريعو,
                                Small_ويڪٽر_Impl<حرف> &نتيجو) override;

  فراغ reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_ڪوڊ
متن_Encoding_Converter_Iconv::convertString(StringRef ذريعو,
                                          Small_ويڪٽر_Impl<حرف> &نتيجو) {
  // Setup the output. We directly write into the SmallVector.
  size_t صلاحيت = نتيجو.صلاحيت_2();
  حرف *Output = static_cast<حرف *>(نتيجو.معلومات());
  size_t Output_ڊيگهائي = صلاحيت;
  نتيجو.resize_for_overwrite(صلاحيت);

  size_t Ret;
  // Handle errors returned from iconv().
  خودڪار HandleError = [&صلاحيت, &Output, &Output_ڊيگهائي, &نتيجو,
                      هذا](size_t Ret) {
    إذا (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      إذا (errno == E2BIG && صلاحيت < نتيجو.max_سائيز()) {
        Handle_حد_کان_مٿي(صلاحيت, Output, Output_ڊيگهائي, نتيجو);
        // Reset converter
        reset();
        أرجع std::error_ڪوڊ();
      } وإلا {
        // Some other error occurred.
        نتيجو.resize(Output - نتيجو.معلومات());
        أرجع std::error_ڪوڊ(errno, std::generic_category());
      }
    } وإلا {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      أرجع std::make_error_ڪوڊ(std::errc::illegal_byte_sequence);
    }
  };

  افعل {
    size_t داخلا_ڊيگهائي = ذريعو.سائيز_2();
    حرف *داخلا = const_cast<حرف *>(ذريعو.معلومات());
    Ret = iconv(ConvDesc, &داخلا, &داخلا_ڊيگهائي, &Output, &Output_ڊيگهائي);
    إذا (Ret != 0) {
      إذا (خودڪار EC = HandleError(Ret))
        أرجع EC;
      استمر;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, خالي_اشارو, خالي_اشارو, &Output, &Output_ڊيگهائي);
    إذا (Ret != 0) {
      إذا (خودڪار EC = HandleError(Ret))
        أرجع EC;
      استمر;
    }
    اكسر;
  } طالما (سچ);

  // Re-adjust size to actual size.
  نتيجو.resize(Output - نتيجو.معلومات());
  أرجع std::error_ڪوڊ();
}

مضمن فراغ متن_Encoding_Converter_Iconv::reset() {
  iconv(ConvDesc, خالي_اشارو, خالي_اشارو, خالي_اشارو, خالي_اشارو);
}

#جيڪڏهن_پورو // HAVE_ICONV
} // namespace

ErrorOr<متن_Encoding_Converter>
متن_Encoding_Converter::create(متن_Encoding CPFrom, متن_Encoding CPTo) {

  // Text encodings should be distinct.
  إذا (CPFrom == CPTo)
    أرجع std::make_error_ڪوڊ(std::errc::invalid_دليل);

  ConversionType Conversion;
  إذا (CPFrom == متن_Encoding::UTF8 && CPTo == متن_Encoding::IBM1047)
    Conversion = UTF8_منزل_IBM1047;
  وإلا إذا (CPFrom == متن_Encoding::IBM1047 && CPTo == متن_Encoding::UTF8)
    Conversion = IBM1047_منزل_UTF8;
  وإلا
    أرجع std::make_error_ڪوڊ(std::errc::invalid_دليل);

  أرجع متن_Encoding_Converter(
      std::make_unique<متن_Encoding_Converter_جدول>(Conversion));
}

ErrorOr<متن_Encoding_Converter> متن_Encoding_Converter::create(StringRef From,
                                                             StringRef منزل) {
  std::optional<متن_Encoding> FromEncoding = get_معلوم_Encoding(From);
  std::optional<متن_Encoding> منزل_Encoding = get_معلوم_Encoding(منزل);
  إذا (FromEncoding && منزل_Encoding) {
    ErrorOr<متن_Encoding_Converter> Converter =
        create(*FromEncoding, *منزل_Encoding);
    إذا (Converter)
      أرجع Converter;
  }
#إذا HAVE_ICU
  UError_ڪوڊ EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  إذا (U_FAILURE(EC))
    أرجع std::make_error_ڪوڊ(std::errc::invalid_دليل);

  UConverterUniquePtr منزل_Conv_Desc(ucnv_open(منزل.str().c_str(), &EC));
  إذا (U_FAILURE(EC))
    أرجع std::make_error_ڪوڊ(std::errc::invalid_دليل);

  خودڪار Converter = std::make_unique<متن_Encoding_Converter_ICU>(
      std::move(FromConvDesc), std::move(منزل_Conv_Desc));
  أرجع متن_Encoding_Converter(std::move(Converter));
#وإلاإذا HAVE_ICONV
  iconv_t ConvDesc = iconv_open(منزل.str().c_str(), From.str().c_str());
  إذا (ConvDesc == (iconv_t)-1)
    أرجع std::make_error_ڪوڊ(std::errc::invalid_دليل);
  أرجع متن_Encoding_Converter(
      std::make_unique<متن_Encoding_Converter_Iconv>(ConvDesc));
#وإلا
  أرجع std::make_error_ڪوڊ(std::errc::invalid_دليل);
#جيڪڏهن_پورو
}
