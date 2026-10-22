//===- NativeFormatting.cpp - Low level formatting helpers -------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/NativeFormatting.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Support/Format.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

#अन्तर्भाव <cmath>

#यदि defined(_WIN32) && !defined(__MINGW32__)
#अन्तर्भाव <float.h> // For _fpclass in llvm::write_double.
#यदि_अन्तः

उपयुज्य नामाकाश llvm;

प्रतिरूपम्<प्रकारनाम T, std::size_t N>
स्थिर पूर्णाङ्क format_गन्तव्यम्_बफर(T मानम्, वर्ण (&बफर)[N]) {
  वर्ण *अन्तः_Ptr = std::अन्तः(बफर);
  वर्ण *CurPtr = अन्तः_Ptr;

  कुरु {
    *--CurPtr = '0' + वर्ण(मानम् % 10);
    मानम् /= 10;
  } यावत् (मानम्);
  प्रत्यागम अन्तः_Ptr - CurPtr;
}

स्थिर शून्य writeWithCommas(raw_ostream &S, ArrayRef<वर्ण> बफर) {
  प्रतिज्ञा(!बफर.रिक्तानि());

  ArrayRef<वर्ण> This_गणः;
  पूर्णाङ्क InitialDigits = ((बफर.आकार_2() - 1) % 3) + 1;
  This_गणः = बफर.take_front(InitialDigits);
  S.लिख(This_गणः.दत्त(), This_गणः.आकार_2());

  बफर = बफर.drop_front(InitialDigits);
  प्रतिज्ञा(बफर.आकार_2() % 3 == 0);
  यावत् (!बफर.रिक्तानि()) {
    S << ',';
    This_गणः = बफर.take_front(3);
    S.लिख(This_गणः.दत्त(), 3);
    बफर = बफर.drop_front(3);
  }
}

प्रतिरूपम् <प्रकारनाम T>
स्थिर शून्य write_unsigned_impl(raw_ostream &S, T N, size_t MinDigits,
                                IntegerStyle Style, तार्किक IsNegative,
                                तार्किक NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  वर्ण सङ्ख्या_बफर[128];
  size_t Len = format_गन्तव्यम्_बफर(N, सङ्ख्या_बफर);

  यदि (IsNegative)
    S << '-';
  अन्यथा यदि (NonNegativePlus)
    S << '+';

  यदि (Len < MinDigits && Style != IntegerStyle::सङ्ख्या) {
    प्रति (size_t I = Len; I < MinDigits; ++I)
      S << '0';
  }

  यदि (Style == IntegerStyle::सङ्ख्या) {
    writeWithCommas(S, ArrayRef<वर्ण>(std::अन्तः(सङ्ख्या_बफर) - Len, Len));
  } अन्यथा {
    S.लिख(std::अन्तः(सङ्ख्या_बफर) - Len, Len);
  }
}

प्रतिरूपम् <प्रकारनाम T>
स्थिर शून्य write_unsigned(raw_ostream &S, T N, size_t MinDigits,
                           IntegerStyle Style, तार्किक IsNegative = असत्य,
                           तार्किक NonNegativePlus = असत्य) {
  // Output using 32-bit div/mod if possible.
  यदि (N == static_cast<uint32_t>(N))
    write_unsigned_impl(S, static_cast<uint32_t>(N), MinDigits, Style,
                        IsNegative, NonNegativePlus);
  अन्यथा
    write_unsigned_impl(S, N, MinDigits, Style, IsNegative, NonNegativePlus);
}

प्रतिरूपम् <प्रकारनाम T>
स्थिर शून्य write_signed(raw_ostream &S, T N, size_t MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus = असत्य) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  उपयुज्य UnsignedT = std::make_unsigned_t<T>;

  यदि (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, असत्य,
                   NonNegativePlus);
    प्रत्यागम;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, सत्य, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, अचिह्नित पूर्णाङ्क N, size_t MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, असत्य, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, पूर्णाङ्क N, size_t MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, अचिह्नित दीर्घ N, size_t MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, असत्य, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, दीर्घ N, size_t MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, अचिह्नित दीर्घ दीर्घ N, size_t MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, असत्य, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, दीर्घ दीर्घ N, size_t MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

शून्य llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<size_t> विस्तारः) {
  ध्रुव size_t k_Max_विस्तारः = 128u;

  size_t W = std::min(k_Max_विस्तारः, विस्तारः.मानम्_or(0u));

  अचिह्नित Nibbles = (llvm::bit_विस्तारः_2(N) + 3) / 4;
  तार्किक उपसर्गः = (Style == HexPrintStyle::उपसर्गः_अधस्तनम् ||
                 Style == HexPrintStyle::उपसर्गः_ऊर्ध्वस्थम्);
  तार्किक ऊर्ध्वस्थम्_2 =
      (Style == HexPrintStyle::ऊर्ध्वस्थम्_2 || Style == HexPrintStyle::उपसर्गः_ऊर्ध्वस्थम्);
  अचिह्नित उपसर्गः_Chars = उपसर्गः ? 2 : 0;
  अचिह्नित अङ्कः_Chars =
      std::max(static_cast<अचिह्नित>(W), std::max(1u, Nibbles) + उपसर्गः_Chars);

  वर्ण सङ्ख्या_बफर[k_Max_विस्तारः];
  ::स्मृतिं_पूरय(सङ्ख्या_बफर, '0', std::आकार_2(सङ्ख्या_बफर));
  यदि (उपसर्गः)
    सङ्ख्या_बफर[1] = 'x';
  वर्ण *अन्तः_Ptr = सङ्ख्या_बफर + अङ्कः_Chars;
  वर्ण *CurPtr = अन्तः_Ptr;
  यावत् (N) {
    अचिह्नित वर्ण x = static_cast<अचिह्नित वर्ण>(N) % 16;
    *--CurPtr = hexdigit(x, !ऊर्ध्वस्थम्_2);
    N /= 16;
  }

  S.लिख(सङ्ख्या_बफर, अङ्कः_Chars);
}

शून्य llvm::write_double(raw_ostream &S, द्विगुण N, FloatStyle Style,
                        std::optional<size_t> Precision) {
  size_t Prec = Precision.मानम्_or(getDefaultPrecision(Style));

  यदि (std::isnan(N)) {
    S << "nan";
    प्रत्यागम;
  } अन्यथा यदि (std::isinf(N)) {
    S << (std::signbit(N) ? "-INF" : "INF");
    प्रत्यागम;
  }

  वर्ण Letter;
  यदि (Style == FloatStyle::Exponent)
    Letter = 'e';
  अन्यथा यदि (Style == FloatStyle::Exponent_ऊर्ध्वस्थम्)
    Letter = 'E';
  अन्यथा
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream बहिर्गमनम्(Spec);
  बहिर्गमनम् << "%." << Prec << Letter;

  यदि (Style == FloatStyle::Exponent || Style == FloatStyle::Exponent_ऊर्ध्वस्थम्) {
#यदि_परिभाषितम् _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#यदि defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    यदि (N == 0.0 && std::signbit(N)) {
      वर्ण NegativeZero[] = "-0.000000e+00";
      यदि (Style == FloatStyle::Exponent_ऊर्ध्वस्थम्)
        NegativeZero[वाक्यदैर्घ्यम्(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      प्रत्यागम;
    }
#अन्यथा
    पूर्णाङ्क fpcl = _fpclass(N);

    // negative zero
    यदि (fpcl == _FPCLASS_NZ) {
      वर्ण NegativeZero[] = "-0.000000e+00";
      यदि (Style == FloatStyle::Exponent_ऊर्ध्वस्थम्)
        NegativeZero[वाक्यदैर्घ्यम्(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      प्रत्यागम;
    }
#यदि_अन्तः

    वर्ण buf[32];
    अचिह्नित len;
    len = format(Spec.c_str(), N).snprint(buf, परिमाण(buf));
    यदि (len <= परिमाण(buf) - 2) {
      यदि (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        पूर्णाङ्क cs = buf[len - 4];
        यदि (cs == '+' || cs == '-') {
          पूर्णाङ्क c1 = buf[len - 2];
          पूर्णाङ्क c0 = buf[len - 1];
          यदि (अङ्कः_वा(static_cast<अचिह्नित वर्ण>(c1)) &&
              अङ्कः_वा(static_cast<अचिह्नित वर्ण>(c0))) {
            // Trim leading '0': "...e+012" -> "...e+12\0"
            buf[len - 3] = c1;
            buf[len - 2] = c0;
            buf[--len] = 0;
          }
        }
      }
      S << buf;
      प्रत्यागम;
    }
#यदि_अन्तः
  }

  यदि (Style == FloatStyle::Percent)
    N *= 100.0;

  वर्ण Buf[32];
  format(Spec.c_str(), N).snprint(Buf, परिमाण(Buf));
  S << Buf;
  यदि (Style == FloatStyle::Percent)
    S << '%';
}

तार्किक llvm::isPrefixedHexStyle(HexPrintStyle S) {
  प्रत्यागम (S == HexPrintStyle::उपसर्गः_अधस्तनम् || S == HexPrintStyle::उपसर्गः_ऊर्ध्वस्थम्);
}

size_t llvm::getDefaultPrecision(FloatStyle Style) {
  विकल्प (Style) {
  पक्ष FloatStyle::Exponent:
  पक्ष FloatStyle::Exponent_ऊर्ध्वस्थम्:
    प्रत्यागम 6; // Number of decimal places.
  पक्ष FloatStyle::Fixed:
  पक्ष FloatStyle::Percent:
    प्रत्यागम 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
