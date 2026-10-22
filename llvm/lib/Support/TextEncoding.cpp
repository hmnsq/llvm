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

#అన్తర్భావ "llvm/Support/TextEncoding.h"
#అన్తర్భావ "llvm/ADT/SmallString.h"
#అన్తర్భావ "llvm/ADT/SmallVector.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Support/ConvertEBCDIC.h"
#అన్తర్భావ <system_error>

#ఒకవేళ HAVE_ICU
#ఒకవేళ HAVE_WINDOWS_ICU
#అన్తర్భావ <icu.h>
#లేకపోతే
#అన్తర్భావ <unicode/ucnv.h>
#అయితే_ముగింపు
#లేదావేళ HAVE_ICONV
#అన్తర్భావ <iconv.h>
#అయితే_ముగింపు

ఉపయోగించి నామాకాశ llvm;

// Normalize the charset name with the charset alias matching algorithm proposed
// in https://www.unicode.org/reports/tr22/tr22-8.html#Charset_Alias_Matching.
స్థిర శూన్య normalize_Char_సమితి_Name(StringRef CSName,
                                 Small_సదిశ_Impl<అక్షరం> &Normalized) {
  తార్కిక PrevDigit = అబద్ధం;
  కొరకు (స్వయంచాలక Ch : CSName) {
    ఒకవేళ (isAlnum(Ch)) {
      Ch = గమ్యం_క్రింది(Ch);
      ఒకవేళ (Ch != '0' || PrevDigit) {
        PrevDigit = isDigit(Ch);
        Normalized.నెట్టు_వెనుక(Ch);
      }
    }
  }
}

// Maps the encoding name to enum constant if possible.
స్థిర std::optional<పాఠ్యం_Encoding> get_తెలిసిన_Encoding(StringRef Name) {
  SmallString<16> Normalized;
  normalize_Char_సమితి_Name(Name, Normalized);
  ఒకవేళ (Normalized.equals("utf8"))
    తిరిగి పాఠ్యం_Encoding::UTF8;
  ఒకవేళ (Normalized.equals("ibm1047"))
    తిరిగి పాఠ్యం_Encoding::IBM1047;
  తిరిగి std::nullopt;
}

[[maybe_వాడని]] స్థిర శూన్య Handle_అధికప్రవాహం(size_t &సామర్థ్యం, అక్షరం *&Output,
                                            size_t &Output_నిడివి,
                                            Small_సదిశ_Impl<అక్షరం> &ఫలితం) {
  // No space left in output buffer. Double the size of the underlying
  // memory in the SmallVectorImpl, adjust pointer and length and continue
  // the conversion.
  సామర్థ్యం =
      (సామర్థ్యం < ఫలితం.max_కొలత() / 2) ? 2 * సామర్థ్యం : ఫలితం.max_కొలత();
  ఫలితం.resize(0);
  ఫలితం.resize_for_overwrite(సామర్థ్యం);
  Output = static_cast<అక్షరం *>(ఫలితం.దత్తాంశం());
  Output_నిడివి = సామర్థ్యం;
}

నామాకాశ {
గణన ConversionType {
  UTF8_గమ్యం_IBM1047,
  IBM1047_గమ్యం_UTF8,
};

// Support conversion between EBCDIC 1047 and UTF-8. This class uses
// built-in translation tables that allow for translation between the
// aforementioned encodings. The use of tables for conversion is only
// possible because EBCDIC 1047 is a single-byte, stateless encoding; other
// encodings are not supported.
తరగతి పాఠ్యం_Encoding_Converter_పట్టిక final
    : బహిరంగ details::పాఠ్యం_Encoding_Converter_Impl_పునాది {
  స్థిరరాశి ConversionType ConvType;

బహిరంగ:
  పాఠ్యం_Encoding_Converter_పట్టిక(ConversionType ConvType) : ConvType(ConvType) {}

  std::error_సంకేతం convertString(StringRef ఆధారం,
                                Small_సదిశ_Impl<అక్షరం> &ఫలితం) override;

  శూన్య reset() override {}
};

std::error_సంకేతం
పాఠ్యం_Encoding_Converter_పట్టిక::convertString(StringRef ఆధారం,
                                          Small_సదిశ_Impl<అక్షరం> &ఫలితం) {
  ఎంపిక (ConvType) {
  స్థితి IBM1047_గమ్యం_UTF8:
    ConverterEBCDIC::convert_గమ్యం_UTF8(ఆధారం, ఫలితం);
    తిరిగి std::error_సంకేతం();
  స్థితి UTF8_గమ్యం_IBM1047:
    తిరిగి ConverterEBCDIC::convert_గమ్యం_EBCDIC(ఆధారం, ఫలితం);
  }
  llvm_unreachable("Invalid ConvType!");
  తిరిగి std::error_సంకేతం();
}

#ఒకవేళ HAVE_ICU
నిర్మాణం UConverterDeleter {
  శూన్య క్రియాకారకం()(UConverter *Converter) స్థిరరాశి {
    ఒకవేళ (Converter)
      ucnv_close(Converter);
  }
};
ఉపయోగించి UConverterUniquePtr = std::unique_ptr<UConverter, UConverterDeleter>;

తరగతి పాఠ్యం_Encoding_Converter_ICU final
    : బహిరంగ details::పాఠ్యం_Encoding_Converter_Impl_పునాది {
  UConverterUniquePtr FromConvDesc;
  UConverterUniquePtr గమ్యం_Conv_Desc;

బహిరంగ:
  పాఠ్యం_Encoding_Converter_ICU(UConverterUniquePtr FromConverter,
                           UConverterUniquePtr గమ్యం_Converter)
      : FromConvDesc(std::move(FromConverter)),
        గమ్యం_Conv_Desc(std::move(గమ్యం_Converter)) {}

  std::error_సంకేతం convertString(StringRef ఆధారం,
                                Small_సదిశ_Impl<అక్షరం> &ఫలితం) override;

  శూన్య reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
// TODO: Improve translation of ICU errors to error_code
std::error_సంకేతం
పాఠ్యం_Encoding_Converter_ICU::convertString(StringRef ఆధారం,
                                        Small_సదిశ_Impl<అక్షరం> &ఫలితం) {
  // Setup the input in case it has no backing data.
  size_t ఆదానం_నిడివి = ఆధారం.కొలత_2();
  స్థిరరాశి అక్షరం *లోపల_2 = ఆదానం_నిడివి ? const_cast<అక్షరం *>(ఆధారం.దత్తాంశం()) : "";

  // Setup the output. We directly write into the SmallVector.
  size_t సామర్థ్యం = ఫలితం.సామర్థ్యం_2();
  size_t Output_నిడివి = సామర్థ్యం;
  ఫలితం.resize_for_overwrite(సామర్థ్యం);
  అక్షరం *Output;
  UError_సంకేతం EC = U_ZERO_ERROR;

  ucnv_సమితి_గమ్యం_UCall_వెనుక(&*FromConvDesc, UCNV_గమ్యం_U_CALLBACK_STOP, ఖాళీ, ఖాళీ, ఖాళీ,
                      &EC);
  ucnv_సమితి_From_UCall_వెనుక(&*గమ్యం_Conv_Desc, UCNV_FROM_U_CALLBACK_STOP, ఖాళీ, ఖాళీ,
                        ఖాళీ, &EC);
  ప్రతిజ్ఞా(U_SUCCESS(EC));

  చేయి {
    EC = U_ZERO_ERROR;
    స్థిరరాశి అక్షరం *ఆదానం = లోపల_2;

    Output = static_cast<అక్షరం *>(ఫలితం.దత్తాంశం());
    ucnv_convertEx(&*గమ్యం_Conv_Desc, &*FromConvDesc, &Output, ఫలితం.చివర(), &ఆదానం,
                   లోపల_2 + ఆదానం_నిడివి, /*pivotStart=*/ఖాళీ,
                   /*pivotSource=*/ఖాళీ, /*pivotTarget=*/ఖాళీ,
                   /*pivotLimit=*/ఖాళీ, /*reset=*/నిజం,
                   /*flush=*/నిజం, &EC);
    ఒకవేళ (U_FAILURE(EC)) {
      ఒకవేళ (EC == U_బఫరు_అధికప్రవాహం_ERROR) {
        ఒకవేళ (సామర్థ్యం < ఫలితం.max_కొలత()) {
          Handle_అధికప్రవాహం(సామర్థ్యం, Output, Output_నిడివి, ఫలితం);
          కొనసాగు;
        } లేకపోతే {
          ఫలితం.resize(Output - ఫలితం.దత్తాంశం());
          తిరిగి std::error_సంకేతం(E2BIG, std::generic_category());
        }
      }
      // Some other error occurred.
      ఫలితం.resize(Output - ఫలితం.దత్తాంశం());
      తిరిగి std::error_సంకేతం(EILSEQ, std::generic_category());
    }
    ఆపు;
  } అయితే (నిజం);

  ఫలితం.resize(Output - ఫలితం.దత్తాంశం());
  తిరిగి std::error_సంకేతం();
}

శూన్య పాఠ్యం_Encoding_Converter_ICU::reset() {
  ucnv_reset(&*FromConvDesc);
  ucnv_reset(&*గమ్యం_Conv_Desc);
}

#లేదావేళ HAVE_ICONV
తరగతి పాఠ్యం_Encoding_Converter_Iconv final
    : బహిరంగ details::పాఠ్యం_Encoding_Converter_Impl_పునాది {
  తరగతి UniqueIconvT {
    iconv_t ConvDesc;

  బహిరంగ:
    క్రియాకారకం iconv_t() స్థిరరాశి { తిరిగి ConvDesc; }
    UniqueIconvT(iconv_t CD) : ConvDesc(CD) {}
    ~UniqueIconvT() {
      ఒకవేళ (ConvDesc != (iconv_t)-1) {
        iconv_close(ConvDesc);
        ConvDesc = (iconv_t)-1;
      }
    }
    UniqueIconvT(UniqueIconvT &&Other) : ConvDesc(Other.ConvDesc) {
      Other.ConvDesc = (iconv_t)-1;
    }
    UniqueIconvT &క్రియాకారకం=(UniqueIconvT &&Other) {
      ఒకవేళ (&Other != ఇది) {
        ConvDesc = Other.ConvDesc;
        Other.ConvDesc = (iconv_t)-1;
      }
      తిరిగి *ఇది;
    }
  };
  UniqueIconvT ConvDesc;

బహిరంగ:
  పాఠ్యం_Encoding_Converter_Iconv(UniqueIconvT ConvDesc)
      : ConvDesc(std::move(ConvDesc)) {}

  std::error_సంకేతం convertString(StringRef ఆధారం,
                                Small_సదిశ_Impl<అక్షరం> &ఫలితం) override;

  శూన్య reset() override;
};

// TODO: The current implementation discards the partial result and restarts the
// conversion from the beginning if there is a conversion error due to
// insufficient buffer size. In the future, it would better to save the partial
// result and resume the conversion for the remaining string.
std::error_సంకేతం
పాఠ్యం_Encoding_Converter_Iconv::convertString(StringRef ఆధారం,
                                          Small_సదిశ_Impl<అక్షరం> &ఫలితం) {
  // Setup the output. We directly write into the SmallVector.
  size_t సామర్థ్యం = ఫలితం.సామర్థ్యం_2();
  అక్షరం *Output = static_cast<అక్షరం *>(ఫలితం.దత్తాంశం());
  size_t Output_నిడివి = సామర్థ్యం;
  ఫలితం.resize_for_overwrite(సామర్థ్యం);

  size_t Ret;
  // Handle errors returned from iconv().
  స్వయంచాలక HandleError = [&సామర్థ్యం, &Output, &Output_నిడివి, &ఫలితం,
                      ఇది](size_t Ret) {
    ఒకవేళ (Ret == static_cast<size_t>(-1)) {
      // An error occurred. Check if we can gracefully handle it.
      ఒకవేళ (errno == E2BIG && సామర్థ్యం < ఫలితం.max_కొలత()) {
        Handle_అధికప్రవాహం(సామర్థ్యం, Output, Output_నిడివి, ఫలితం);
        // Reset converter
        reset();
        తిరిగి std::error_సంకేతం();
      } లేకపోతే {
        // Some other error occurred.
        ఫలితం.resize(Output - ఫలితం.దత్తాంశం());
        తిరిగి std::error_సంకేతం(errno, std::generic_category());
      }
    } లేకపోతే {
      // A positive return value indicates that some characters were converted
      // in a nonreversible way, that is, replaced with a SUB symbol. Returning
      // an error in this case makes sure that both conversion routines behave
      // in the same way.
      తిరిగి std::make_error_సంకేతం(std::errc::illegal_byte_sequence);
    }
  };

  చేయి {
    size_t ఆదానం_నిడివి = ఆధారం.కొలత_2();
    అక్షరం *ఆదానం = const_cast<అక్షరం *>(ఆధారం.దత్తాంశం());
    Ret = iconv(ConvDesc, &ఆదానం, &ఆదానం_నిడివి, &Output, &Output_నిడివి);
    ఒకవేళ (Ret != 0) {
      ఒకవేళ (స్వయంచాలక EC = HandleError(Ret))
        తిరిగి EC;
      కొనసాగు;
    }
    // Flush the converter
    Ret = iconv(ConvDesc, శూన్యసూచిక, శూన్యసూచిక, &Output, &Output_నిడివి);
    ఒకవేళ (Ret != 0) {
      ఒకవేళ (స్వయంచాలక EC = HandleError(Ret))
        తిరిగి EC;
      కొనసాగు;
    }
    ఆపు;
  } అయితే (నిజం);

  // Re-adjust size to actual size.
  ఫలితం.resize(Output - ఫలితం.దత్తాంశం());
  తిరిగి std::error_సంకేతం();
}

అంతఃవరుస శూన్య పాఠ్యం_Encoding_Converter_Iconv::reset() {
  iconv(ConvDesc, శూన్యసూచిక, శూన్యసూచిక, శూన్యసూచిక, శూన్యసూచిక);
}

#అయితే_ముగింపు // HAVE_ICONV
} // namespace

ErrorOr<పాఠ్యం_Encoding_Converter>
పాఠ్యం_Encoding_Converter::create(పాఠ్యం_Encoding CPFrom, పాఠ్యం_Encoding CPTo) {

  // Text encodings should be distinct.
  ఒకవేళ (CPFrom == CPTo)
    తిరిగి std::make_error_సంకేతం(std::errc::invalid_వాదం);

  ConversionType Conversion;
  ఒకవేళ (CPFrom == పాఠ్యం_Encoding::UTF8 && CPTo == పాఠ్యం_Encoding::IBM1047)
    Conversion = UTF8_గమ్యం_IBM1047;
  లేకపోతే ఒకవేళ (CPFrom == పాఠ్యం_Encoding::IBM1047 && CPTo == పాఠ్యం_Encoding::UTF8)
    Conversion = IBM1047_గమ్యం_UTF8;
  లేకపోతే
    తిరిగి std::make_error_సంకేతం(std::errc::invalid_వాదం);

  తిరిగి పాఠ్యం_Encoding_Converter(
      std::make_unique<పాఠ్యం_Encoding_Converter_పట్టిక>(Conversion));
}

ErrorOr<పాఠ్యం_Encoding_Converter> పాఠ్యం_Encoding_Converter::create(StringRef From,
                                                             StringRef గమ్యం) {
  std::optional<పాఠ్యం_Encoding> FromEncoding = get_తెలిసిన_Encoding(From);
  std::optional<పాఠ్యం_Encoding> గమ్యం_Encoding = get_తెలిసిన_Encoding(గమ్యం);
  ఒకవేళ (FromEncoding && గమ్యం_Encoding) {
    ErrorOr<పాఠ్యం_Encoding_Converter> Converter =
        create(*FromEncoding, *గమ్యం_Encoding);
    ఒకవేళ (Converter)
      తిరిగి Converter;
  }
#ఒకవేళ HAVE_ICU
  UError_సంకేతం EC = U_ZERO_ERROR;
  UConverterUniquePtr FromConvDesc(ucnv_open(From.str().c_str(), &EC));
  ఒకవేళ (U_FAILURE(EC))
    తిరిగి std::make_error_సంకేతం(std::errc::invalid_వాదం);

  UConverterUniquePtr గమ్యం_Conv_Desc(ucnv_open(గమ్యం.str().c_str(), &EC));
  ఒకవేళ (U_FAILURE(EC))
    తిరిగి std::make_error_సంకేతం(std::errc::invalid_వాదం);

  స్వయంచాలక Converter = std::make_unique<పాఠ్యం_Encoding_Converter_ICU>(
      std::move(FromConvDesc), std::move(గమ్యం_Conv_Desc));
  తిరిగి పాఠ్యం_Encoding_Converter(std::move(Converter));
#లేదావేళ HAVE_ICONV
  iconv_t ConvDesc = iconv_open(గమ్యం.str().c_str(), From.str().c_str());
  ఒకవేళ (ConvDesc == (iconv_t)-1)
    తిరిగి std::make_error_సంకేతం(std::errc::invalid_వాదం);
  తిరిగి పాఠ్యం_Encoding_Converter(
      std::make_unique<పాఠ్యం_Encoding_Converter_Iconv>(ConvDesc));
#లేకపోతే
  తిరిగి std::make_error_సంకేతం(std::errc::invalid_వాదం);
#అయితే_ముగింపు
}
