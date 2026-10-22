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
#د_که_پای

کارول نطاق llvm;

کينډۍ<ډول_نوم T, std::size_t N>
ساكن صحيح format_منزل_بفر(T ارزښت, حرف (&بفر)[N]) {
  حرف *پای_ټکی_Ptr = std::پای_ټکی(بفر);
  حرف *CurPtr = پای_ټکی_Ptr;

  افعل {
    *--CurPtr = '0' + حرف(ارزښت % 10);
    ارزښت /= 10;
  } طالما (ارزښت);
  أرجع پای_ټکی_Ptr - CurPtr;
}

ساكن فراغ writeWithCommas(raw_ostream &S, ArrayRef<حرف> بفر) {
  أكد(!بفر.تشه());

  ArrayRef<حرف> This_ډله;
  صحيح InitialDigits = ((بفر.اندازه_2() - 1) % 3) + 1;
  This_ډله = بفر.take_front(InitialDigits);
  S.اكتب(This_ډله.معلومات(), This_ډله.اندازه_2());

  بفر = بفر.drop_front(InitialDigits);
  أكد(بفر.اندازه_2() % 3 == 0);
  طالما (!بفر.تشه()) {
    S << ',';
    This_ډله = بفر.take_front(3);
    S.اكتب(This_ډله.معلومات(), 3);
    بفر = بفر.drop_front(3);
  }
}

کينډۍ <ډول_نوم T>
ساكن فراغ write_unsigned_impl(raw_ostream &S, T N, size_t MinDigits,
                                IntegerStyle Style, منطقي IsNegative,
                                منطقي NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  حرف شمېره_بفر[128];
  size_t Len = format_منزل_بفر(N, شمېره_بفر);

  إذا (IsNegative)
    S << '-';
  وإلا إذا (NonNegativePlus)
    S << '+';

  إذا (Len < MinDigits && Style != IntegerStyle::شمېره) {
    لكل (size_t I = Len; I < MinDigits; ++I)
      S << '0';
  }

  إذا (Style == IntegerStyle::شمېره) {
    writeWithCommas(S, ArrayRef<حرف>(std::پای_ټکی(شمېره_بفر) - Len, Len));
  } وإلا {
    S.اكتب(std::پای_ټکی(شمېره_بفر) - Len, Len);
  }
}

کينډۍ <ډول_نوم T>
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

کينډۍ <ډول_نوم T>
ساكن فراغ write_signed(raw_ostream &S, T N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus = خطأ) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  کارول UnsignedT = std::make_unsigned_t<T>;

  إذا (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, خطأ,
                   NonNegativePlus);
    أرجع;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, ریښتیا, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, بینښه صحيح N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, خطأ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, صحيح N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, بینښه اوږد N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, خطأ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, اوږد N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, بینښه اوږد اوږد N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, خطأ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, اوږد اوږد N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

فراغ llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<size_t> پلنوالی) {
  ثابت size_t k_Max_پلنوالی = 128u;

  size_t W = std::min(k_Max_پلنوالی, پلنوالی.ارزښت_or(0u));

  بینښه Nibbles = (llvm::بټ_پلنوالی_3(N) + 3) / 4;
  منطقي مختاړی = (Style == HexPrintStyle::مختاړی_ښکتنی ||
                 Style == HexPrintStyle::مختاړی_پاسنی);
  منطقي پاسنی_2 =
      (Style == HexPrintStyle::پاسنی_2 || Style == HexPrintStyle::مختاړی_پاسنی);
  بینښه مختاړی_Chars = مختاړی ? 2 : 0;
  بینښه عدد_Chars =
      std::max(static_cast<بینښه>(W), std::max(1u, Nibbles) + مختاړی_Chars);

  حرف شمېره_بفر[k_Max_پلنوالی];
  ::memset(شمېره_بفر, '0', std::اندازه_2(شمېره_بفر));
  إذا (مختاړی)
    شمېره_بفر[1] = 'x';
  حرف *پای_ټکی_Ptr = شمېره_بفر + عدد_Chars;
  حرف *CurPtr = پای_ټکی_Ptr;
  طالما (N) {
    بینښه حرف x = static_cast<بینښه حرف>(N) % 16;
    *--CurPtr = hexdigit(x, !پاسنی_2);
    N /= 16;
  }

  S.اكتب(شمېره_بفر, عدد_Chars);
}

فراغ llvm::write_double(raw_ostream &S, مزدوج N, FloatStyle Style,
                        std::optional<size_t> Precision) {
  size_t Prec = Precision.ارزښت_or(getDefaultPrecision(Style));

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
  وإلا إذا (Style == FloatStyle::Exponent_پاسنی)
    Letter = 'E';
  وإلا
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream بهر(Spec);
  بهر << "%." << Prec << Letter;

  إذا (Style == FloatStyle::Exponent || Style == FloatStyle::Exponent_پاسنی) {
#که_تعریف_شوی _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#إذا defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    إذا (N == 0.0 && std::signbit(N)) {
      حرف NegativeZero[] = "-0.000000e+00";
      إذا (Style == FloatStyle::Exponent_پاسنی)
        NegativeZero[مزي_اوږدوالی(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      أرجع;
    }
#وإلا
    صحيح fpcl = _fpclass(N);

    // negative zero
    إذا (fpcl == _FPCLASS_NZ) {
      حرف NegativeZero[] = "-0.000000e+00";
      إذا (Style == FloatStyle::Exponent_پاسنی)
        NegativeZero[مزي_اوږدوالی(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      أرجع;
    }
#د_که_پای

    حرف buf[32];
    بینښه len;
    len = format(Spec.c_str(), N).snprint(buf, حجم(buf));
    إذا (len <= حجم(buf) - 2) {
      إذا (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        صحيح cs = buf[len - 4];
        إذا (cs == '+' || cs == '-') {
          صحيح c1 = buf[len - 2];
          صحيح c0 = buf[len - 1];
          إذا (که_عدد(static_cast<بینښه حرف>(c1)) &&
              که_عدد(static_cast<بینښه حرف>(c0))) {
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
#د_که_پای
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
  أرجع (S == HexPrintStyle::مختاړی_ښکتنی || S == HexPrintStyle::مختاړی_پاسنی);
}

size_t llvm::getDefaultPrecision(FloatStyle Style) {
  بدل (Style) {
  حالة FloatStyle::Exponent:
  حالة FloatStyle::Exponent_پاسنی:
    أرجع 6; // Number of decimal places.
  حالة FloatStyle::Fixed:
  حالة FloatStyle::Percent:
    أرجع 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
