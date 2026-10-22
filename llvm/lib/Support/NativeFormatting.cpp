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
#پایان_اگر

استفاده نطاق llvm;

قالب<نام_نوع T, std::size_t N>
ساكن صحيح format_مقصد_میانگیر(T مقدار, حرف (&میانگیر)[N]) {
  حرف *انتها_Ptr = std::انتها(میانگیر);
  حرف *CurPtr = انتها_Ptr;

  افعل {
    *--CurPtr = '0' + حرف(مقدار % 10);
    مقدار /= 10;
  } طالما (مقدار);
  أرجع انتها_Ptr - CurPtr;
}

ساكن فراغ writeWithCommas(raw_ostream &S, ArrayRef<حرف> میانگیر) {
  أكد(!میانگیر.خالی());

  ArrayRef<حرف> This_گروه;
  صحيح InitialDigits = ((میانگیر.اندازه_2() - 1) % 3) + 1;
  This_گروه = میانگیر.take_front(InitialDigits);
  S.اكتب(This_گروه.داده(), This_گروه.اندازه_2());

  میانگیر = میانگیر.drop_front(InitialDigits);
  أكد(میانگیر.اندازه_2() % 3 == 0);
  طالما (!میانگیر.خالی()) {
    S << ',';
    This_گروه = میانگیر.take_front(3);
    S.اكتب(This_گروه.داده(), 3);
    میانگیر = میانگیر.drop_front(3);
  }
}

قالب <نام_نوع T>
ساكن فراغ write_unsigned_impl(raw_ostream &S, T N, size_t MinDigits,
                                IntegerStyle Style, منطقي IsNegative,
                                منطقي NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  حرف عدد_میانگیر[128];
  size_t Len = format_مقصد_میانگیر(N, عدد_میانگیر);

  إذا (IsNegative)
    S << '-';
  وإلا إذا (NonNegativePlus)
    S << '+';

  إذا (Len < MinDigits && Style != IntegerStyle::عدد) {
    لكل (size_t I = Len; I < MinDigits; ++I)
      S << '0';
  }

  إذا (Style == IntegerStyle::عدد) {
    writeWithCommas(S, ArrayRef<حرف>(std::انتها(عدد_میانگیر) - Len, Len));
  } وإلا {
    S.اكتب(std::انتها(عدد_میانگیر) - Len, Len);
  }
}

قالب <نام_نوع T>
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

قالب <نام_نوع T>
ساكن فراغ write_signed(raw_ostream &S, T N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus = خطأ) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  استفاده UnsignedT = std::make_unsigned_t<T>;

  إذا (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, خطأ,
                   NonNegativePlus);
    أرجع;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, درست, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, بیعلامت صحيح N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, خطأ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, صحيح N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, بیعلامت دراز N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, خطأ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, دراز N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, بیعلامت دراز دراز N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, خطأ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, دراز دراز N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

فراغ llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<size_t> عرض) {
  ثابت size_t k_Max_عرض = 128u;

  size_t W = std::min(k_Max_عرض, عرض.مقدار_or(0u));

  بیعلامت Nibbles = (llvm::بیت_عرض_3(N) + 3) / 4;
  منطقي پیشوند = (Style == HexPrintStyle::پیشوند_پایینی ||
                 Style == HexPrintStyle::پیشوند_فوقانی);
  منطقي فوقانی_2 =
      (Style == HexPrintStyle::فوقانی_2 || Style == HexPrintStyle::پیشوند_فوقانی);
  بیعلامت پیشوند_Chars = پیشوند ? 2 : 0;
  بیعلامت رقم_Chars =
      std::max(static_cast<بیعلامت>(W), std::max(1u, Nibbles) + پیشوند_Chars);

  حرف عدد_میانگیر[k_Max_عرض];
  ::memset(عدد_میانگیر, '0', std::اندازه_2(عدد_میانگیر));
  إذا (پیشوند)
    عدد_میانگیر[1] = 'x';
  حرف *انتها_Ptr = عدد_میانگیر + رقم_Chars;
  حرف *CurPtr = انتها_Ptr;
  طالما (N) {
    بیعلامت حرف x = static_cast<بیعلامت حرف>(N) % 16;
    *--CurPtr = hexdigit(x, !فوقانی_2);
    N /= 16;
  }

  S.اكتب(عدد_میانگیر, رقم_Chars);
}

فراغ llvm::write_double(raw_ostream &S, مزدوج N, FloatStyle Style,
                        std::optional<size_t> Precision) {
  size_t Prec = Precision.مقدار_or(getDefaultPrecision(Style));

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
  وإلا إذا (Style == FloatStyle::Exponent_فوقانی)
    Letter = 'E';
  وإلا
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream بیرون(Spec);
  بیرون << "%." << Prec << Letter;

  إذا (Style == FloatStyle::Exponent || Style == FloatStyle::Exponent_فوقانی) {
#اگر_تعریف_شده _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#إذا defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    إذا (N == 0.0 && std::signbit(N)) {
      حرف NegativeZero[] = "-0.000000e+00";
      إذا (Style == FloatStyle::Exponent_فوقانی)
        NegativeZero[طول_رشته(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      أرجع;
    }
#وإلا
    صحيح fpcl = _fpclass(N);

    // negative zero
    إذا (fpcl == _FPCLASS_NZ) {
      حرف NegativeZero[] = "-0.000000e+00";
      إذا (Style == FloatStyle::Exponent_فوقانی)
        NegativeZero[طول_رشته(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      أرجع;
    }
#پایان_اگر

    حرف buf[32];
    بیعلامت len;
    len = format(Spec.c_str(), N).snprint(buf, حجم(buf));
    إذا (len <= حجم(buf) - 2) {
      إذا (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        صحيح cs = buf[len - 4];
        إذا (cs == '+' || cs == '-') {
          صحيح c1 = buf[len - 2];
          صحيح c0 = buf[len - 1];
          إذا (آیا_رقم(static_cast<بیعلامت حرف>(c1)) &&
              آیا_رقم(static_cast<بیعلامت حرف>(c0))) {
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
#پایان_اگر
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
  أرجع (S == HexPrintStyle::پیشوند_پایینی || S == HexPrintStyle::پیشوند_فوقانی);
}

size_t llvm::getDefaultPrecision(FloatStyle Style) {
  بدل (Style) {
  حالة FloatStyle::Exponent:
  حالة FloatStyle::Exponent_فوقانی:
    أرجع 6; // Number of decimal places.
  حالة FloatStyle::Fixed:
  حالة FloatStyle::Percent:
    أرجع 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
