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
#پایان_اگر
#وإلاإذا HAVE_ICONV
#ضمن <iconv.h>
#پایان_اگر

استفاده نطاق llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
ساكن فراغ normalize_Char_مجموعه_Name(StringRef CSName,
                                 Small_وکتور_Impl<حرف> &Normalized) {
  منطقي PrevDigit = خطأ;
  لكل (خودکار Ch : CSName) {
    إذا (isAlnum(Ch)) {
      Ch = مقصد_پایینی(Ch);
      إذا (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
        Normalized.افزودن_پشت(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
ساكن std::optional<متن_Encoding> get_معلوم_Encoding(StringRef Name) {
  SmallString<16> Normalized;
  normalize_Char_مجموعه_Name(Name, Normalized);
  إذا (Normalized.equals("utf8"))
    أرجع متن_Encoding::UTF8;
  إذا (Normalized.equals("ibm1047"))
    أرجع متن_Encoding::IBM1047;
  أرجع std::nullopt;
}

[[maybe_استفادهنشده]] ساكن فراغ Handle_سرریز(size_t &ظرفیت, حرف *&Output,
                                            size_t &Output_درازی,
                                            Small_وکتور_Impl<حرف> &نتیجه) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  ظرفیت =
      (ظرفیت < نتیجه.max_اندازه() / 2) ? 2 * ظرفیت : نتیجه.max_اندازه();
  نتیجه.resize(0);
  نتیجه.resize_for_overwrite(ظرفیت);
  Output = static_cast<حرف *>(نتیجه.داده());
  Output_درازی = ظرفیت;
}

نطاق {
تعداد ConversionType {
  UTF8_مقصد_IBM1047,
  IBM1047_مقصد_UTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
صنف متن_Encoding_Converter_جدول final
    : عام details::متن_Encoding_Converter_Impl_پایه {
  ثابت ConversionType ConvType;

عام:
  متن_Encoding_Converter_جدول(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_کد convertString(StringRef منبع,
                                Small_وکتور_Impl<حرف> &نتیجه) override;

  فراغ reset() override {}
};

std::error_کد
متن_Encoding_Converter_جدول::convertString(StringRef منبع,
                                          Small_وکتور_Impl<حرف> &نتیجه) {
  بدل (ConvType) {
  حالة IBM1047_مقصد_UTF8:
    ConverterEBCDIC::convert_مقصد_UTF8(منبع, نتیجه);
    أرجع std::error_کد();
  حالة UTF8_مقصد_IBM1047:
    أرجع ConverterEBCDIC::convert_مقصد_EBCDIC(منبع, نتیجه);
  }
  llvm_unreachable("Invalid ConvType!");
  أرجع std::error_کد();
}

#إذا HAVE_ICU
بنية UConverterDeleter {
  فراغ عملگر()(UConverter *Converter) ثابت {
    إذا (Converter)
      ucnv_close(Converter);
  }
};
استفاده UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

صنف متن_Encoding_Converter_ICU final
    : عام details::متن_Encoding_Converter_Impl_پایه {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr مقصد_Conv_Desc;

عام:
  متن_Encoding_Converter_ICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr مقصد_Converter)
      : FromConvDesc(std::move(FromConverter)),
        مقصد_Conv_Desc(std::move(مقصد_Converter)) {}

  std::error_کد convertString(StringRef منبع,
                                Small_وکتور_Impl<حرف> &نتیجه) override;

  فراغ reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_کد
متن_Encoding_Converter_ICU::convertString(StringRef منبع,
                                        Small_وکتور_Impl<حرف> &نتیجه) {
  // Setup the input in case it has no backing data.
  size_t درون_داد_درازی = منبع.اندازه_2();
  ثابت حرف *درون_2 = درون_داد_درازی ? const_cast<حرف *>(منبع.داده()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t ظرفیت = نتیجه.ظرفیت_2();
  size_t Output_درازی = ظرفیت;
  نتیجه.resize_for_overwrite(ظرفیت);
  حرف *Output;
  UError_کد EC = U_ZERO_ERROR;

  ucnv_مجموعه_مقصد_UCall_پشت(&*FromConvDesc, UCNV_مقصد_U_CALLBACK_STOP, عدم, عدم, عدم,
                      &EC);
  ucnv_مجموعه_From_UCall_پشت(&*مقصد_Conv_Desc, UCNV_FROM_U_CALLBACK_STOP, عدم, عدم,
                        عدم, &EC);
  أكد(U_SUCCESS(EC));

  افعل {
    EC = U_ZERO_ERROR;
    ثابت حرف *درون_داد = درون_2;

    Output = static_cast<حرف *>(نتیجه.داده());
    ucnv_convertEx(&*مقصد_Conv_Desc, &*FromConvDesc, &Output, نتیجه.انتها(), &درون_داد,
                   درون_2 + درون_داد_درازی, /*pivotStart=*/عدم,
                   /*pivotSource=*/عدم, /*pivotTarget=*/عدم,
                   /*pivotLimit=*/عدم, /*reset=*/درست,
                   /*flush=*/درست, &EC);
    إذا (U_FAILURE(EC)) {
      إذا (EC == U_میانگیر_سرریز_ERROR) {
        إذا (ظرفیت < نتیجه.max_اندازه()) {
          Handle_سرریز(ظرفیت, Output, Output_درازی, نتیجه);
          استمر;
        } وإلا {
          نتیجه.resize(Output - نتیجه.داده());
          أرجع std::error_کد(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      نتیجه.resize(Output - نتیجه.داده());
      أرجع std::error_کد(EILSEQ, std::generic_category());
    }
    اكسر;
  } طالما (درست);

  نتیجه.resize(Output - نتیجه.داده());
  أرجع std::error_کد();
}

فراغ متن_Encoding_Converter_ICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*مقصد_Conv_Desc);
}

#وإلاإذا HAVE_ICONV
صنف متن_Encoding_Converter_Iconv final
    : عام details::متن_Encoding_Converter_Impl_پایه {
  صنف UniqueIconvT {
    iconv_t ConvDesc;

  عام:
    عملگر iconv_t() ثابت { أرجع ConvDesc; }
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
    UniqueIconvT &عملگر=(UniqueIconvT &&Other) {
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

  std::error_کد convertString(StringRef منبع,
                                Small_وکتور_Impl<حرف> &نتیجه) override;

  فراغ reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_کد
متن_Encoding_Converter_Iconv::convertString(StringRef منبع,
                                          Small_وکتور_Impl<حرف> &نتیجه) {
  // Setup the output. We directly write into the SmallVector.
  size_t ظرفیت = نتیجه.ظرفیت_2();
  حرف *Output = static_cast<حرف *>(نتیجه.داده());
  size_t Output_درازی = ظرفیت;
  نتیجه.resize_for_overwrite(ظرفیت);

  size_t Ret;
  // Handle errors returned from iconv().
  خودکار HandleError = [&ظرفیت, &Output, &Output_درازی, &نتیجه,
                      هذا](size_t Ret) {
    إذا (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      إذا (errno == E2BIG && ظرفیت < نتیجه.max_اندازه()) {
        Handle_سرریز(ظرفیت, Output, Output_درازی, نتیجه);
        // Reset converter
        reset();
        أرجع std::error_کد();
      } وإلا {
        // Some other error occurred.
        نتیجه.resize(Output - نتیجه.داده());
        أرجع std::error_کد(errno, std::generic_category());
      }
    } وإلا {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      أرجع std::make_error_کد(std::errc::illegal_byte_sequence);
    }
  };

  افعل {
    size_t درون_داد_درازی = منبع.اندازه_2();
    حرف *درون_داد = const_cast<حرف *>(منبع.داده());
    Ret = iconv(ConvDesc, &درون_داد, &درون_داد_درازی, &Output, &Output_درازی);
    إذا (Ret != 0) {
      إذا (خودکار EC = HandleError(Ret))
        أرجع EC;
      استمر;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, اشاره_تهی, اشاره_تهی, &Output, &Output_درازی);
    إذا (Ret != 0) {
      إذا (خودکار EC = HandleError(Ret))
        أرجع EC;
      استمر;
    }
    اكسر;
  } طالما (درست);

  // Re-adjust size to actual size.
  نتیجه.resize(Output - نتیجه.داده());
  أرجع std::error_کد();
}

مضمن فراغ متن_Encoding_Converter_Iconv::reset() {
  iconv(ConvDesc, اشاره_تهی, اشاره_تهی, اشاره_تهی, اشاره_تهی);
}

#پایان_اگر // HAVE_ICONV
} // namespace

ErrorOr<متن_Encoding_Converter>
متن_Encoding_Converter::create(متن_Encoding CPFrom, متن_Encoding CPTo) {

  // Text encodings should be distinct.
  إذا (CPFrom == CPTo)
    أرجع std::make_error_کد(std::errc::invalid_آرگومان);

  ConversionType Conversion;
  إذا (CPFrom == متن_Encoding::UTF8 && CPTo == متن_Encoding::IBM1047)
    Conversion = UTF8_مقصد_IBM1047;
  وإلا إذا (CPFrom == متن_Encoding::IBM1047 && CPTo == متن_Encoding::UTF8)
    Conversion = IBM1047_مقصد_UTF8;
  وإلا
    أرجع std::make_error_کد(std::errc::invalid_آرگومان);

  أرجع متن_Encoding_Converter(
      std::make_unique<متن_Encoding_Converter_جدول>(Conversion));
}

ErrorOr<متن_Encoding_Converter> متن_Encoding_Converter::create(StringRef From,
                                                             StringRef مقصد) {
  std::optional<متن_Encoding> FromEncoding = get_معلوم_Encoding(From);
  std::optional<متن_Encoding> مقصد_Encoding = get_معلوم_Encoding(مقصد);
  إذا (FromEncoding && مقصد_Encoding) {
    ErrorOr<متن_Encoding_Converter> Converter =
        create(*FromEncoding, *مقصد_Encoding);
    إذا (Converter)
      أرجع Converter;
  }
#إذا HAVE_ICU
  UError_کد EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  إذا (U_FAILURE(EC))
    أرجع std::make_error_کد(std::errc::invalid_آرگومان);

  UConverterUniquePtr مقصد_Conv_Desc(ucnv_open(مقصد.str().c_str(), &EC));
  إذا (U_FAILURE(EC))
    أرجع std::make_error_کد(std::errc::invalid_آرگومان);

  خودکار Converter = std::make_unique<متن_Encoding_Converter_ICU>(
      std::move(FromConvDesc), std::move(مقصد_Conv_Desc));
  أرجع متن_Encoding_Converter(std::move(Converter));
#وإلاإذا HAVE_ICONV
  iconv_t ConvDesc = iconv_open(مقصد.str().c_str(), From.str().c_str());
  إذا (ConvDesc == (iconv_t)-1)
    أرجع std::make_error_کد(std::errc::invalid_آرگومان);
  أرجع متن_Encoding_Converter(
      std::make_unique<متن_Encoding_Converter_Iconv>(ConvDesc));
#وإلا
  أرجع std::make_error_کد(std::errc::invalid_آرگومان);
#پایان_اگر
}
