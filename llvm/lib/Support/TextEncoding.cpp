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
#यदि_अन्तः
#नोचेद्यदि HAVE_ICONV
#अन्तर्भाव <iconv.h>
#यदि_अन्तः

उपयुज्य नामाकाश llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
स्थिर शून्य normalize_Char_समुच्चयः_Name(StringRef CSName,
                                 Small_सदिशम्_Impl<वर्ण> &Normalized) {
  तार्किक पूर्व_Digit = असत्य;
  प्रति (स्वचालित Ch : CSName) {
    यदि (isAlnum(Ch)) {
      Ch = गन्तव्यम्_अधस्तनम्(Ch);
      यदि (Ch != '0' || पूर्व_Digit) {
        पूर्व_Digit = isDigit(Ch);
        Normalized.प्रक्षिप_पृष्ठभागः(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
स्थिर std::optional<TextEncoding> get_ज्ञातम्_Encoding(StringRef Name) {
  SmallString<16> Normalized;
  normalize_Char_समुच्चयः_Name(Name, Normalized);
  यदि (Normalized.equals("utf8"))
    प्रत्यागम TextEncoding::UTF8;
  यदि (Normalized.equals("ibm1047"))
    प्रत्यागम TextEncoding::IBM1047;
  प्रत्यागम std::nullopt;
}

[[maybe_अप्रयुक्तम्]] स्थिर शून्य Handle_अतिप्रवाहः(size_t &क्षमता, वर्ण *&Output,
                                            size_t &Output_दीर्घता,
                                            Small_सदिशम्_Impl<वर्ण> &परिणाम) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  क्षमता =
      (क्षमता < परिणाम.max_आकार() / 2) ? 2 * क्षमता : परिणाम.max_आकार();
  परिणाम.resize(0);
  परिणाम.resize_for_overwrite(क्षमता);
  Output = static_cast<वर्ण *>(परिणाम.दत्त());
  Output_दीर्घता = क्षमता;
}

नामाकाश {
गणना ConversionType {
  UTF8_गन्तव्यम्_IBM1047,
  IBM1047_गन्तव्यम्_UTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
वर्ग Text_Encoding_Converter_सारणी final
    : सार्वजनिक details::Text_Encoding_Converter_Impl_आधारः {
  ध्रुव ConversionType ConvType;

सार्वजनिक:
  Text_Encoding_Converter_सारणी(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_संहिता convertString(StringRef Source,
                                Small_सदिशम्_Impl<वर्ण> &परिणाम) override;

  शून्य reset() override {}
};

std::error_संहिता
Text_Encoding_Converter_सारणी::convertString(StringRef Source,
                                          Small_सदिशम्_Impl<वर्ण> &परिणाम) {
  विकल्प (ConvType) {
  पक्ष IBM1047_गन्तव्यम्_UTF8:
    ConverterEBCDIC::convert_गन्तव्यम्_UTF8(Source, परिणाम);
    प्रत्यागम std::error_संहिता();
  पक्ष UTF8_गन्तव्यम्_IBM1047:
    प्रत्यागम ConverterEBCDIC::convert_गन्तव्यम्_EBCDIC(Source, परिणाम);
  }
  llvm_unreachable("Invalid ConvType!");
  प्रत्यागम std::error_संहिता();
}

#यदि HAVE_ICU
संरचना UConverterDeleter {
  शून्य संक्रियकः()(UConverter *Converter) ध्रुव {
    यदि (Converter)
      ucnv_close(Converter);
  }
};
उपयुज्य UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

वर्ग TextEncodingConverterICU final
    : सार्वजनिक details::Text_Encoding_Converter_Impl_आधारः {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr गन्तव्यम्_Conv_Desc;

सार्वजनिक:
  TextEncodingConverterICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr गन्तव्यम्_Converter)
      : FromConvDesc(std::move(FromConverter)),
        गन्तव्यम्_Conv_Desc(std::move(गन्तव्यम्_Converter)) {}

  std::error_संहिता convertString(StringRef Source,
                                Small_सदिशम्_Impl<वर्ण> &परिणाम) override;

  शून्य reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_संहिता
TextEncodingConverterICU::convertString(StringRef Source,
                                        Small_सदिशम्_Impl<वर्ण> &परिणाम) {
  // Setup the input in case it has no backing data.
  size_t निवेशः_दीर्घता = Source.आकार_2();
  ध्रुव वर्ण *अन्तर्_2 = निवेशः_दीर्घता ? const_cast<वर्ण *>(Source.दत्त()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t क्षमता = परिणाम.क्षमता_2();
  size_t Output_दीर्घता = क्षमता;
  परिणाम.resize_for_overwrite(क्षमता);
  वर्ण *Output;
  UError_संहिता EC = U_ZERO_ERROR;

  ucnv_समुच्चयः_गन्तव्यम्_UCall_पृष्ठभागः(&*FromConvDesc, UCNV_गन्तव्यम्_U_CALLBACK_STOP, रिक्त, रिक्त, रिक्त,
                      &EC);
  ucnv_समुच्चयः_From_UCall_पृष्ठभागः(&*गन्तव्यम्_Conv_Desc, UCNV_FROM_U_CALLBACK_STOP, रिक्त, रिक्त,
                        रिक्त, &EC);
  प्रतिज्ञा(U_SUCCESS(EC));

  कुरु {
    EC = U_ZERO_ERROR;
    ध्रुव वर्ण *निवेशः = अन्तर्_2;

    Output = static_cast<वर्ण *>(परिणाम.दत्त());
    ucnv_convertEx(&*गन्तव्यम्_Conv_Desc, &*FromConvDesc, &Output, परिणाम.अन्तः(), &निवेशः,
                   अन्तर्_2 + निवेशः_दीर्घता, /*pivotStart=*/रिक्त,
                   /*pivotSource=*/रिक्त, /*pivotTarget=*/रिक्त,
                   /*pivotLimit=*/रिक्त, /*reset=*/सत्य,
                   /*flush=*/सत्य, &EC);
    यदि (U_FAILURE(EC)) {
      यदि (EC == U_बफर_अतिप्रवाहः_ERROR) {
        यदि (क्षमता < परिणाम.max_आकार()) {
          Handle_अतिप्रवाहः(क्षमता, Output, Output_दीर्घता, परिणाम);
          अनुवर्त;
        } अन्यथा {
          परिणाम.resize(Output - परिणाम.दत्त());
          प्रत्यागम std::error_संहिता(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      परिणाम.resize(Output - परिणाम.दत्त());
      प्रत्यागम std::error_संहिता(EILSEQ, std::generic_category());
    }
    भङ्ग;
  } यावत् (सत्य);

  परिणाम.resize(Output - परिणाम.दत्त());
  प्रत्यागम std::error_संहिता();
}

शून्य TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*गन्तव्यम्_Conv_Desc);
}

#नोचेद्यदि HAVE_ICONV
वर्ग TextEncodingConverterIconv final
    : सार्वजनिक details::Text_Encoding_Converter_Impl_आधारः {
  वर्ग UniqueIconvT {
    iconv_t ConvDesc;

  सार्वजनिक:
    संक्रियकः iconv_t() ध्रुव { प्रत्यागम ConvDesc; }
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
    UniqueIconvT &संक्रियकः=(UniqueIconvT &&Other) {
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

  std::error_संहिता convertString(StringRef Source,
                                Small_सदिशम्_Impl<वर्ण> &परिणाम) override;

  शून्य reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_संहिता
TextEncodingConverterIconv::convertString(StringRef Source,
                                          Small_सदिशम्_Impl<वर्ण> &परिणाम) {
  // Setup the output. We directly write into the SmallVector.
  size_t क्षमता = परिणाम.क्षमता_2();
  वर्ण *Output = static_cast<वर्ण *>(परिणाम.दत्त());
  size_t Output_दीर्घता = क्षमता;
  परिणाम.resize_for_overwrite(क्षमता);

  size_t Ret;
  // Handle errors returned from iconv().
  स्वचालित HandleError = [&क्षमता, &Output, &Output_दीर्घता, &परिणाम,
                      एतत्](size_t Ret) {
    यदि (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      यदि (errno == E2BIG && क्षमता < परिणाम.max_आकार()) {
        Handle_अतिप्रवाहः(क्षमता, Output, Output_दीर्घता, परिणाम);
        // Reset converter
        reset();
        प्रत्यागम std::error_संहिता();
      } अन्यथा {
        // Some other error occurred.
        परिणाम.resize(Output - परिणाम.दत्त());
        प्रत्यागम std::error_संहिता(errno, std::generic_category());
      }
    } अन्यथा {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      प्रत्यागम std::make_error_संहिता(std::errc::illegal_byte_sequence);
    }
  };

  कुरु {
    size_t निवेशः_दीर्घता = Source.आकार_2();
    वर्ण *निवेशः = const_cast<वर्ण *>(Source.दत्त());
    Ret = iconv(ConvDesc, &निवेशः, &निवेशः_दीर्घता, &Output, &Output_दीर्घता);
    यदि (Ret != 0) {
      यदि (स्वचालित EC = HandleError(Ret))
        प्रत्यागम EC;
      अनुवर्त;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, शून्यसूचकः, शून्यसूचकः, &Output, &Output_दीर्घता);
    यदि (Ret != 0) {
      यदि (स्वचालित EC = HandleError(Ret))
        प्रत्यागम EC;
      अनुवर्त;
    }
    भङ्ग;
  } यावत् (सत्य);

  // Re-adjust size to actual size.
  परिणाम.resize(Output - परिणाम.दत्त());
  प्रत्यागम std::error_संहिता();
}

अन्तःस्थ शून्य TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, शून्यसूचकः, शून्यसूचकः, शून्यसूचकः, शून्यसूचकः);
}

#यदि_अन्तः // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  यदि (CPFrom == CPTo)
    प्रत्यागम std::make_error_संहिता(std::errc::invalid_तर्कः);

  ConversionType Conversion;
  यदि (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8_गन्तव्यम्_IBM1047;
  अन्यथा यदि (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047_गन्तव्यम्_UTF8;
  अन्यथा
    प्रत्यागम std::make_error_संहिता(std::errc::invalid_तर्कः);

  प्रत्यागम TextEncodingConverter(
      std::make_unique<Text_Encoding_Converter_सारणी>(Conversion));
}

ErrorOr<TextEncodingConverter> TextEncodingConverter::create(StringRef From,
                                                             StringRef गन्तव्यम्) {
  std::optional<TextEncoding> FromEncoding = get_ज्ञातम्_Encoding(From);
  std::optional<TextEncoding> गन्तव्यम्_Encoding = get_ज्ञातम्_Encoding(गन्तव्यम्);
  यदि (FromEncoding && गन्तव्यम्_Encoding) {
    ErrorOr<TextEncodingConverter> Converter =
        create(*FromEncoding, *गन्तव्यम्_Encoding);
    यदि (Converter)
      प्रत्यागम Converter;
  }
#यदि HAVE_ICU
  UError_संहिता EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  यदि (U_FAILURE(EC))
    प्रत्यागम std::make_error_संहिता(std::errc::invalid_तर्कः);

  UConverterUniquePtr गन्तव्यम्_Conv_Desc(ucnv_open(गन्तव्यम्.str().c_str(), &EC));
  यदि (U_FAILURE(EC))
    प्रत्यागम std::make_error_संहिता(std::errc::invalid_तर्कः);

  स्वचालित Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(गन्तव्यम्_Conv_Desc));
  प्रत्यागम TextEncodingConverter(std::move(Converter));
#नोचेद्यदि HAVE_ICONV
  iconv_t ConvDesc = iconv_open(गन्तव्यम्.str().c_str(), From.str().c_str());
  यदि (ConvDesc == (iconv_t)-1)
    प्रत्यागम std::make_error_संहिता(std::errc::invalid_तर्कः);
  प्रत्यागम TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#अन्यथा
  प्रत्यागम std::make_error_संहिता(std::errc::invalid_तर्कः);
#यदि_अन्तः
}
