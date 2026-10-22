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

#समावेश "llvm/Support/TextEncoding.h"
#समावेश "llvm/ADT/SmallString.h"
#समावेश "llvm/ADT/SmallVector.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/Config/config.h"
#समावेश "llvm/Support/ConvertEBCDIC.h"
#समावेश <system_error>

#अगर HAVE_ICU
#अगर HAVE_WINDOWS_ICU
#समावेश <icu.h>
#अन्यथा
#समावेश <unicode/ucnv.h>
#पूर्ण_अगर
#या_अगर HAVE_ICONV
#समावेश <iconv.h>
#पूर्ण_अगर

using namespace llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
अटल व्योम normalizeCharSetName(StringRef CSName,
                                 SmallVectorImpl<अक्षर> &Normalized) {
  bool पूर्व_Digit = false;
  क्रम (स्वतः Ch : CSName) {
    अगर (isAlnum(Ch)) {
      Ch = toLower(Ch);
      अगर (Ch != '0' || पूर्व_Digit) {
        पूर्व_Digit = isDigit(Ch);
        Normalized.धकेलो_back(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
अटल std::optional<TextEncoding> getKnownEncoding(StringRef Name) {
  SmallString<16> Normalized;
  normalizeCharSetName(Name, Normalized);
  अगर (Normalized.equals("utf8"))
    वापस TextEncoding::UTF8;
  अगर (Normalized.equals("ibm1047"))
    वापस TextEncoding::IBM1047;
  वापस std::nullopt;
}

[[maybe_unused]] अटल व्योम HandleOverflow(माप_प्रकार &Capacity, अक्षर *&Output,
                                            माप_प्रकार &Output_लंबाई,
                                            SmallVectorImpl<अक्षर> &परिणाम) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  Capacity =
      (Capacity < परिणाम.max_आकार() / 2) ? 2 * Capacity : परिणाम.max_आकार();
  परिणाम.resize(0);
  परिणाम.resize_for_overwrite(Capacity);
  Output = static_cast<अक्षर *>(परिणाम.तथ्य());
  Output_लंबाई = Capacity;
}

namespace {
क्रमागत ConversionType {
  UTF8ToIBM1047,
  IBM1047ToUTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
श्रेणी TextEncodingConverterTable final
    : खुला details::TextEncodingConverterImplBase {
  स्थिर ConversionType ConvType;

खुला:
  TextEncodingConverterTable(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<अक्षर> &परिणाम) override;

  व्योम reset() override {}
};

std::error_code
TextEncodingConverterTable::convertString(StringRef Source,
                                          SmallVectorImpl<अक्षर> &परिणाम) {
  चयन (ConvType) {
  हाल IBM1047ToUTF8:
    ConverterEBCDIC::convertToUTF8(Source, परिणाम);
    वापस std::error_code();
  हाल UTF8ToIBM1047:
    वापस ConverterEBCDIC::convertToEBCDIC(Source, परिणाम);
  }
  llvm_unreachable("Invalid ConvType!");
  वापस std::error_code();
}

#अगर HAVE_ICU
काष्ठा UConverterDeleter {
  व्योम चालक()(UConverter *Converter) स्थिर {
    अगर (Converter)
      ucnv_close(Converter);
  }
};
using UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

श्रेणी TextEncodingConverterICU final
    : खुला details::TextEncodingConverterImplBase {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr ToConvDesc;

खुला:
  TextEncodingConverterICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr ToConverter)
      : FromConvDesc(std::move(FromConverter)),
        ToConvDesc(std::move(ToConverter)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<अक्षर> &परिणाम) override;

  व्योम reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_code
TextEncodingConverterICU::convertString(StringRef Source,
                                        SmallVectorImpl<अक्षर> &परिणाम) {
  // Setup the input in case it has no backing data.
  माप_प्रकार निवेश_लंबाई = Source.आकार_2();
  स्थिर अक्षर *In = निवेश_लंबाई ? const_cast<अक्षर *>(Source.तथ्य()) : "";

  // Setup the output. We directly write into the SmallVector.
  माप_प्रकार Capacity = परिणाम.capacity();
  माप_प्रकार Output_लंबाई = Capacity;
  परिणाम.resize_for_overwrite(Capacity);
  अक्षर *Output;
  UErrorCode EC = U_ZERO_ERROR;

  ucnv_setToUCallBack(&*FromConvDesc, UCNV_TO_U_CALLBACK_STOP, शून्य, शून्य, शून्य,
                      &EC);
  ucnv_setFromUCallBack(&*ToConvDesc, UCNV_FROM_U_CALLBACK_STOP, शून्य, शून्य,
                        शून्य, &EC);
  निश्चित(U_SUCCESS(EC));

  करो {
    EC = U_ZERO_ERROR;
    स्थिर अक्षर *निवेश = In;

    Output = static_cast<अक्षर *>(परिणाम.तथ्य());
    ucnv_convertEx(&*ToConvDesc, &*FromConvDesc, &Output, परिणाम.end(), &निवेश,
                   In + निवेश_लंबाई, /*pivotStart=*/शून्य,
                   /*pivotSource=*/शून्य, /*pivotTarget=*/शून्य,
                   /*pivotLimit=*/शून्य, /*reset=*/true,
                   /*flush=*/true, &EC);
    अगर (U_FAILURE(EC)) {
      अगर (EC == U_बफर_OVERFLOW_ERROR) {
        अगर (Capacity < परिणाम.max_आकार()) {
          HandleOverflow(Capacity, Output, Output_लंबाई, परिणाम);
          जारी;
        } अन्यथा {
          परिणाम.resize(Output - परिणाम.तथ्य());
          वापस std::error_code(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      परिणाम.resize(Output - परिणाम.तथ्य());
      वापस std::error_code(EILSEQ, std::generic_category());
    }
    अवरोध;
  } जबतक (true);

  परिणाम.resize(Output - परिणाम.तथ्य());
  वापस std::error_code();
}

व्योम TextEncodingConverterICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*ToConvDesc);
}

#या_अगर HAVE_ICONV
श्रेणी TextEncodingConverterIconv final
    : खुला details::TextEncodingConverterImplBase {
  श्रेणी UniqueIconvT {
    iconv_t ConvDesc;

  खुला:
    चालक iconv_t() स्थिर { वापस ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      अगर (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &चालक=(UniqueIconvT &&Other) {
      अगर (&Other != यह) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      वापस *यह;
    }
  };
  UniqueIconvT ConvDesc;

खुला:
  TextEncodingConverterIconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_code convertString(StringRef Source,
                                SmallVectorImpl<अक्षर> &परिणाम) override;

  व्योम reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_code
TextEncodingConverterIconv::convertString(StringRef Source,
                                          SmallVectorImpl<अक्षर> &परिणाम) {
  // Setup the output. We directly write into the SmallVector.
  माप_प्रकार Capacity = परिणाम.capacity();
  अक्षर *Output = static_cast<अक्षर *>(परिणाम.तथ्य());
  माप_प्रकार Output_लंबाई = Capacity;
  परिणाम.resize_for_overwrite(Capacity);

  माप_प्रकार Ret;
  // Handle errors returned from iconv().
  स्वतः HandleError = [&Capacity, &Output, &Output_लंबाई, &परिणाम,
                      यह](माप_प्रकार Ret) {
    अगर (Ret == static_cast<माप_प्रकार>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      अगर (त्रुटि_सं == E2BIG && Capacity < परिणाम.max_आकार()) {
        HandleOverflow(Capacity, Output, Output_लंबाई, परिणाम);
        // Reset converter
        reset();
        वापस std::error_code();
      } अन्यथा {
        // Some other error occurred.
        परिणाम.resize(Output - परिणाम.तथ्य());
        वापस std::error_code(त्रुटि_सं, std::generic_category());
      }
    } अन्यथा {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      वापस std::make_error_code(std::errc::illegal_byte_sequence);
    }
  };

  करो {
    माप_प्रकार निवेश_लंबाई = Source.आकार_2();
    अक्षर *निवेश = const_cast<अक्षर *>(Source.तथ्य());
    Ret = iconv(ConvDesc, &निवेश, &निवेश_लंबाई, &Output, &Output_लंबाई);
    अगर (Ret != 0) {
      अगर (स्वतः EC = HandleError(Ret))
        वापस EC;
      जारी;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, nullptr, nullptr, &Output, &Output_लंबाई);
    अगर (Ret != 0) {
      अगर (स्वतः EC = HandleError(Ret))
        वापस EC;
      जारी;
    }
    अवरोध;
  } जबतक (true);

  // Re-adjust size to actual size.
  परिणाम.resize(Output - परिणाम.तथ्य());
  वापस std::error_code();
}

अंतरभूत व्योम TextEncodingConverterIconv::reset() {
  iconv(ConvDesc, nullptr, nullptr, nullptr, nullptr);
}

#पूर्ण_अगर // HAVE_ICONV
} // namespace

ErrorOr<TextEncodingConverter>
TextEncodingConverter::create(TextEncoding CPFrom, TextEncoding CPTo) {

  // Text encodings should be distinct.
  अगर (CPFrom == CPTo)
    वापस std::make_error_code(std::errc::invalid_argument);

  ConversionType Conversion;
  अगर (CPFrom == TextEncoding::UTF8 && CPTo == TextEncoding::IBM1047)
    Conversion = UTF8ToIBM1047;
  अन्यथा अगर (CPFrom == TextEncoding::IBM1047 && CPTo == TextEncoding::UTF8)
    Conversion = IBM1047ToUTF8;
  अन्यथा
    वापस std::make_error_code(std::errc::invalid_argument);

  वापस TextEncodingConverter(
      std::make_unique<TextEncodingConverterTable>(Conversion));
}

ErrorOr<TextEncodingConverter> TextEncodingConverter::create(StringRef From,
                                                             StringRef To) {
  std::optional<TextEncoding> FromEncoding = getKnownEncoding(From);
  std::optional<TextEncoding> ToEncoding = getKnownEncoding(To);
  अगर (FromEncoding && ToEncoding) {
    ErrorOr<TextEncodingConverter> Converter =
        create(*FromEncoding, *ToEncoding);
    अगर (Converter)
      वापस Converter;
  }
#अगर HAVE_ICU
  UErrorCode EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  अगर (U_FAILURE(EC))
    वापस std::make_error_code(std::errc::invalid_argument);

  UConverterUniquePtr ToConvDesc(ucnv_open(To.str().c_str(), &EC));
  अगर (U_FAILURE(EC))
    वापस std::make_error_code(std::errc::invalid_argument);

  स्वतः Converter = std::make_unique<TextEncodingConverterICU>(
      std::move(FromConvDesc), std::move(ToConvDesc));
  वापस TextEncodingConverter(std::move(Converter));
#या_अगर HAVE_ICONV
  iconv_t ConvDesc = iconv_open(To.str().c_str(), From.str().c_str());
  अगर (ConvDesc == (iconv_t)-1)
    वापस std::make_error_code(std::errc::invalid_argument);
  वापस TextEncodingConverter(
      std::make_unique<TextEncodingConverterIconv>(ConvDesc));
#अन्यथा
  वापस std::make_error_code(std::errc::invalid_argument);
#पूर्ण_अगर
}
