//===- NativeFormatting.cpp - Low level formatting helpers -------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/NativeFormatting.h"
#అన్తర్భావ "llvm/ADT/ArrayRef.h"
#అన్తర్భావ "llvm/ADT/SmallString.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/Support/Format.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"

#అన్తర్భావ <cmath>

#ఒకవేళ defined(_WIN32) && !defined(__MINGW32__)
#అన్తర్భావ <float.h> // For _fpclass in llvm::write_double.
#అయితే_ముగింపు

ఉపయోగించి నామాకాశ llvm;

మూస<రకం_పేరు T, std::size_t N>
స్థిర పూర్ణాంకం format_గమ్యం_బఫరు(T విలువ, అక్షరం (&బఫరు)[N]) {
  అక్షరం *చివర_Ptr = std::చివర(బఫరు);
  అక్షరం *CurPtr = చివర_Ptr;

  చేయి {
    *--CurPtr = '0' + అక్షరం(విలువ % 10);
    విలువ /= 10;
  } అయితే (విలువ);
  తిరిగి చివర_Ptr - CurPtr;
}

స్థిర శూన్య writeWithCommas(raw_ostream &S, ArrayRef<అక్షరం> బఫరు) {
  ప్రతిజ్ఞా(!బఫరు.ఖాళీలు());

  ArrayRef<అక్షరం> This_సమూహం;
  పూర్ణాంకం InitialDigits = ((బఫరు.కొలత_2() - 1) % 3) + 1;
  This_సమూహం = బఫరు.take_front(InitialDigits);
  S.లిఖ(This_సమూహం.దత్తాంశం(), This_సమూహం.కొలత_2());

  బఫరు = బఫరు.drop_front(InitialDigits);
  ప్రతిజ్ఞా(బఫరు.కొలత_2() % 3 == 0);
  అయితే (!బఫరు.ఖాళీలు()) {
    S << ',';
    This_సమూహం = బఫరు.take_front(3);
    S.లిఖ(This_సమూహం.దత్తాంశం(), 3);
    బఫరు = బఫరు.drop_front(3);
  }
}

మూస <రకం_పేరు T>
స్థిర శూన్య write_unsigned_impl(raw_ostream &S, T N, size_t MinDigits,
                                IntegerStyle Style, తార్కిక IsNegative,
                                తార్కిక NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  అక్షరం సంఖ్య_బఫరు[128];
  size_t Len = format_గమ్యం_బఫరు(N, సంఖ్య_బఫరు);

  ఒకవేళ (IsNegative)
    S << '-';
  లేకపోతే ఒకవేళ (NonNegativePlus)
    S << '+';

  ఒకవేళ (Len < MinDigits && Style != IntegerStyle::సంఖ్య) {
    కొరకు (size_t I = Len; I < MinDigits; ++I)
      S << '0';
  }

  ఒకవేళ (Style == IntegerStyle::సంఖ్య) {
    writeWithCommas(S, ArrayRef<అక్షరం>(std::చివర(సంఖ్య_బఫరు) - Len, Len));
  } లేకపోతే {
    S.లిఖ(std::చివర(సంఖ్య_బఫరు) - Len, Len);
  }
}

మూస <రకం_పేరు T>
స్థిర శూన్య write_unsigned(raw_ostream &S, T N, size_t MinDigits,
                           IntegerStyle Style, తార్కిక IsNegative = అబద్ధం,
                           తార్కిక NonNegativePlus = అబద్ధం) {
  // Output using 32-bit div/mod if possible.
  ఒకవేళ (N == static_cast<uint32_t>(N))
    write_unsigned_impl(S, static_cast<uint32_t>(N), MinDigits, Style,
                        IsNegative, NonNegativePlus);
  లేకపోతే
    write_unsigned_impl(S, N, MinDigits, Style, IsNegative, NonNegativePlus);
}

మూస <రకం_పేరు T>
స్థిర శూన్య write_signed(raw_ostream &S, T N, size_t MinDigits,
                         IntegerStyle Style, తార్కిక NonNegativePlus = అబద్ధం) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  ఉపయోగించి UnsignedT = std::make_unsigned_t<T>;

  ఒకవేళ (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, అబద్ధం,
                   NonNegativePlus);
    తిరిగి;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, నిజం, NonNegativePlus);
}

శూన్య llvm::write_integer(raw_ostream &S, గుర్తులేని పూర్ణాంకం N, size_t MinDigits,
                         IntegerStyle Style, తార్కిక NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, అబద్ధం, NonNegativePlus);
}

శూన్య llvm::write_integer(raw_ostream &S, పూర్ణాంకం N, size_t MinDigits,
                         IntegerStyle Style, తార్కిక NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

శూన్య llvm::write_integer(raw_ostream &S, గుర్తులేని దీర్ఘ N, size_t MinDigits,
                         IntegerStyle Style, తార్కిక NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, అబద్ధం, NonNegativePlus);
}

శూన్య llvm::write_integer(raw_ostream &S, దీర్ఘ N, size_t MinDigits,
                         IntegerStyle Style, తార్కిక NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

శూన్య llvm::write_integer(raw_ostream &S, గుర్తులేని దీర్ఘ దీర్ఘ N, size_t MinDigits,
                         IntegerStyle Style, తార్కిక NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, అబద్ధం, NonNegativePlus);
}

శూన్య llvm::write_integer(raw_ostream &S, దీర్ఘ దీర్ఘ N, size_t MinDigits,
                         IntegerStyle Style, తార్కిక NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

శూన్య llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<size_t> వెడల్పు) {
  స్థిరరాశి size_t k_Max_వెడల్పు = 128u;

  size_t W = std::min(k_Max_వెడల్పు, వెడల్పు.విలువ_or(0u));

  గుర్తులేని Nibbles = (llvm::బిట్టు_వెడల్పు_3(N) + 3) / 4;
  తార్కిక ఉపసర్గ = (Style == HexPrintStyle::ఉపసర్గ_క్రింది ||
                 Style == HexPrintStyle::ఉపసర్గ_పై);
  తార్కిక పై_2 =
      (Style == HexPrintStyle::పై_2 || Style == HexPrintStyle::ఉపసర్గ_పై);
  గుర్తులేని ఉపసర్గ_Chars = ఉపసర్గ ? 2 : 0;
  గుర్తులేని అంకె_Chars =
      std::max(static_cast<గుర్తులేని>(W), std::max(1u, Nibbles) + ఉపసర్గ_Chars);

  అక్షరం సంఖ్య_బఫరు[k_Max_వెడల్పు];
  ::memset(సంఖ్య_బఫరు, '0', std::కొలత_2(సంఖ్య_బఫరు));
  ఒకవేళ (ఉపసర్గ)
    సంఖ్య_బఫరు[1] = 'x';
  అక్షరం *చివర_Ptr = సంఖ్య_బఫరు + అంకె_Chars;
  అక్షరం *CurPtr = చివర_Ptr;
  అయితే (N) {
    గుర్తులేని అక్షరం x = static_cast<గుర్తులేని అక్షరం>(N) % 16;
    *--CurPtr = hexdigit(x, !పై_2);
    N /= 16;
  }

  S.లిఖ(సంఖ్య_బఫరు, అంకె_Chars);
}

శూన్య llvm::write_double(raw_ostream &S, ద్విగుణ N, FloatStyle Style,
                        std::optional<size_t> Precision) {
  size_t Prec = Precision.విలువ_or(getDefaultPrecision(Style));

  ఒకవేళ (std::isnan(N)) {
    S << "nan";
    తిరిగి;
  } లేకపోతే ఒకవేళ (std::isinf(N)) {
    S << (std::signbit(N) ? "-INF" : "INF");
    తిరిగి;
  }

  అక్షరం Letter;
  ఒకవేళ (Style == FloatStyle::Exponent)
    Letter = 'e';
  లేకపోతే ఒకవేళ (Style == FloatStyle::Exponent_పై)
    Letter = 'E';
  లేకపోతే
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream బహిర్గతం(Spec);
  బహిర్గతం << "%." << Prec << Letter;

  ఒకవేళ (Style == FloatStyle::Exponent || Style == FloatStyle::Exponent_పై) {
#నిర్వచితమైతే _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#ఒకవేళ defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    ఒకవేళ (N == 0.0 && std::signbit(N)) {
      అక్షరం NegativeZero[] = "-0.000000e+00";
      ఒకవేళ (Style == FloatStyle::Exponent_పై)
        NegativeZero[అక్షరశ్రేణి_పొడవు(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      తిరిగి;
    }
#లేకపోతే
    పూర్ణాంకం fpcl = _fpclass(N);

    // negative zero
    ఒకవేళ (fpcl == _FPCLASS_NZ) {
      అక్షరం NegativeZero[] = "-0.000000e+00";
      ఒకవేళ (Style == FloatStyle::Exponent_పై)
        NegativeZero[అక్షరశ్రేణి_పొడవు(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      తిరిగి;
    }
#అయితే_ముగింపు

    అక్షరం buf[32];
    గుర్తులేని len;
    len = format(Spec.c_str(), N).snprint(buf, పరిమాణం(buf));
    ఒకవేళ (len <= పరిమాణం(buf) - 2) {
      ఒకవేళ (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        పూర్ణాంకం cs = buf[len - 4];
        ఒకవేళ (cs == '+' || cs == '-') {
          పూర్ణాంకం c1 = buf[len - 2];
          పూర్ణాంకం c0 = buf[len - 1];
          ఒకవేళ (అంకెనా(static_cast<గుర్తులేని అక్షరం>(c1)) &&
              అంకెనా(static_cast<గుర్తులేని అక్షరం>(c0))) {
            // Trim leading '0': "...e+012" -> "...e+12\0"
            buf[len - 3] = c1;
            buf[len - 2] = c0;
            buf[--len] = 0;
          }
        }
      }
      S << buf;
      తిరిగి;
    }
#అయితే_ముగింపు
  }

  ఒకవేళ (Style == FloatStyle::Percent)
    N *= 100.0;

  అక్షరం Buf[32];
  format(Spec.c_str(), N).snprint(Buf, పరిమాణం(Buf));
  S << Buf;
  ఒకవేళ (Style == FloatStyle::Percent)
    S << '%';
}

తార్కిక llvm::isPrefixedHexStyle(HexPrintStyle S) {
  తిరిగి (S == HexPrintStyle::ఉపసర్గ_క్రింది || S == HexPrintStyle::ఉపసర్గ_పై);
}

size_t llvm::getDefaultPrecision(FloatStyle Style) {
  ఎంపిక (Style) {
  స్థితి FloatStyle::Exponent:
  స్థితి FloatStyle::Exponent_పై:
    తిరిగి 6; // Number of decimal places.
  స్థితి FloatStyle::Fixed:
  స్థితి FloatStyle::Percent:
    తిరిగి 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
