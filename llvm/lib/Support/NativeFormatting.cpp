//===- NativeFormatting.cpp - Low level formatting helpers -------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/NativeFormatting.h"
#ضمن "llvm/ADT/ArrayRef.h"
#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/Format.h"
#ضمن "llvm/Support/raw_ostream.h"

#ضمن <cmath>

#إذا defined(_WIN32) && !defined(__MINGW32__)
#ضمن <float.h> // For _fpclass in llvm::write_double.
#جے_انت

ورتوں نطاق llvm;

سانچا<قسم_ناں T, std::size_t N>
ساكن صحيح format_منزل_بفر(T قدر, حرف (&بفر)[N]) {
  حرف *سرا_Ptr = std::سرا(بفر);
  حرف *CurPtr = سرا_Ptr;

  افعل {
    *--CurPtr = '0' + حرف(قدر % 10);
    قدر /= 10;
  } طالما (قدر);
  أرجع سرا_Ptr - CurPtr;
}

ساكن فراغ writeWithCommas(raw_ostream &S, ArrayRef<حرف> بفر) {
  أكد(!بفر.خالی_تھاواں());

  ArrayRef<حرف> This_گروہ;
  صحيح InitialDigits = ((بفر.ناپ_2() - 1) % 3) + 1;
  This_گروہ = بفر.take_front(InitialDigits);
  S.اكتب(This_گروہ.انکڑے(), This_گروہ.ناپ_2());

  بفر = بفر.drop_front(InitialDigits);
  أكد(بفر.ناپ_2() % 3 == 0);
  طالما (!بفر.خالی_تھاواں()) {
    S << ',';
    This_گروہ = بفر.take_front(3);
    S.اكتب(This_گروہ.انکڑے(), 3);
    بفر = بفر.drop_front(3);
  }
}

سانچا <قسم_ناں T>
ساكن فراغ write_unsigned_impl(raw_ostream &S, T N, size_t MinDigits,
                                IntegerStyle Style, منطقي IsNegative,
                                منطقي NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  حرف نمبر_بفر[128];
  size_t Len = format_منزل_بفر(N, نمبر_بفر);

  إذا (IsNegative)
    S << '-';
  وإلا إذا (NonNegativePlus)
    S << '+';

  إذا (Len < MinDigits && Style != IntegerStyle::نمبر) {
    لكل (size_t I = Len; I < MinDigits; ++I)
      S << '0';
  }

  إذا (Style == IntegerStyle::نمبر) {
    writeWithCommas(S, ArrayRef<حرف>(std::سرا(نمبر_بفر) - Len, Len));
  } وإلا {
    S.اكتب(std::سرا(نمبر_بفر) - Len, Len);
  }
}

سانچا <قسم_ناں T>
ساكن فراغ write_unsigned(raw_ostream &S, T N, size_t MinDigits,
                           IntegerStyle Style, منطقي IsNegative = خطأ,
                           منطقي NonNegativePlus = خطأ) {
  // Output using 32-bit div/mod if possible.
  إذا (N == static_cast<uint32_t>(N))
    write_unsigned_impl(S, static_cast<uint32_t>(N), MinDigits, Style,
                        IsNegative, NonNegativePlus);
  وإلا
    write_unsigned_impl(S, N, MinDigits, Style, IsNegative, NonNegativePlus);
}

سانچا <قسم_ناں T>
ساكن فراغ write_signed(raw_ostream &S, T N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus = خطأ) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  ورتوں UnsignedT = std::make_unsigned_t<T>;

  إذا (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, خطأ,
                   NonNegativePlus);
    أرجع;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, سچ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, بنانشانی صحيح N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, خطأ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, صحيح N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, بنانشانی لما N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, خطأ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, لما N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, بنانشانی لما لما N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, خطأ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, لما لما N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

فراغ llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<size_t> چوڑائی) {
  ثابت size_t k_Max_چوڑائی = 128u;

  size_t W = std::min(k_Max_چوڑائی, چوڑائی.قدر_or(0u));

  بنانشانی Nibbles = (llvm::ثنائی_ہندسہ_چوڑائی_3(N) + 3) / 4;
  منطقي اگیتر = (Style == HexPrintStyle::اگیتر_ہیٹھلا ||
                 Style == HexPrintStyle::اگیتر_اتلا);
  منطقي اتلا_2 =
      (Style == HexPrintStyle::اتلا_2 || Style == HexPrintStyle::اگیتر_اتلا);
  بنانشانی اگیتر_Chars = اگیتر ? 2 : 0;
  بنانشانی عدد_Chars =
      std::max(static_cast<بنانشانی>(W), std::max(1u, Nibbles) + اگیتر_Chars);

  حرف نمبر_بفر[k_Max_چوڑائی];
  ::memset(نمبر_بفر, '0', std::ناپ_2(نمبر_بفر));
  إذا (اگیتر)
    نمبر_بفر[1] = 'x';
  حرف *سرا_Ptr = نمبر_بفر + عدد_Chars;
  حرف *CurPtr = سرا_Ptr;
  طالما (N) {
    بنانشانی حرف x = static_cast<بنانشانی حرف>(N) % 16;
    *--CurPtr = hexdigit(x, !اتلا_2);
    N /= 16;
  }

  S.اكتب(نمبر_بفر, عدد_Chars);
}

فراغ llvm::write_double(raw_ostream &S, مزدوج N, FloatStyle Style,
                        std::optional<size_t> Precision) {
  size_t Prec = Precision.قدر_or(getDefaultPrecision(Style));

  إذا (std::isnan(N)) {
    S << "nan";
    أرجع;
  } وإلا إذا (std::isinf(N)) {
    S << (std::signbit(N) ? "-INF" : "INF");
    أرجع;
  }

  حرف Letter;
  إذا (Style == FloatStyle::Exponent)
    Letter = 'e';
  وإلا إذا (Style == FloatStyle::Exponent_اتلا)
    Letter = 'E';
  وإلا
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream باہرلا(Spec);
  باہرلا << "%." << Prec << Letter;

  إذا (Style == FloatStyle::Exponent || Style == FloatStyle::Exponent_اتلا) {
#جے_تعریف_شدہ _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#إذا defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    إذا (N == 0.0 && std::signbit(N)) {
      حرف NegativeZero[] = "-0.000000e+00";
      إذا (Style == FloatStyle::Exponent_اتلا)
        NegativeZero[لڑی_لمبائی(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      أرجع;
    }
#وإلا
    صحيح fpcl = _fpclass(N);

    // negative zero
    إذا (fpcl == _FPCLASS_NZ) {
      حرف NegativeZero[] = "-0.000000e+00";
      إذا (Style == FloatStyle::Exponent_اتلا)
        NegativeZero[لڑی_لمبائی(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      أرجع;
    }
#جے_انت

    حرف buf[32];
    بنانشانی len;
    len = format(Spec.c_str(), N).snprint(buf, حجم(buf));
    إذا (len <= حجم(buf) - 2) {
      إذا (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        صحيح cs = buf[len - 4];
        إذا (cs == '+' || cs == '-') {
          صحيح c1 = buf[len - 2];
          صحيح c0 = buf[len - 1];
          إذا (ہندسہ_اے(static_cast<بنانشانی حرف>(c1)) &&
              ہندسہ_اے(static_cast<بنانشانی حرف>(c0))) {
            // Trim leading '0': "...e+012" -> "...e+12\0"
            buf[len - 3] = c1;
            buf[len - 2] = c0;
            buf[--len] = 0;
          }
        }
      }
      S << buf;
      أرجع;
    }
#جے_انت
  }

  إذا (Style == FloatStyle::Percent)
    N *= 100.0;

  حرف Buf[32];
  format(Spec.c_str(), N).snprint(Buf, حجم(Buf));
  S << Buf;
  إذا (Style == FloatStyle::Percent)
    S << '%';
}

منطقي llvm::isPrefixedHexStyle(HexPrintStyle S) {
  أرجع (S == HexPrintStyle::اگیتر_ہیٹھلا || S == HexPrintStyle::اگیتر_اتلا);
}

size_t llvm::getDefaultPrecision(FloatStyle Style) {
  بدل (Style) {
  حالة FloatStyle::Exponent:
  حالة FloatStyle::Exponent_اتلا:
    أرجع 6; // Number of decimal places.
  حالة FloatStyle::Fixed:
  حالة FloatStyle::Percent:
    أرجع 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
