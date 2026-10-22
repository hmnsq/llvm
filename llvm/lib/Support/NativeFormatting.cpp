//===- NativeFormatting.cpp - Low level formatting helpers -------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/NativeFormatting.h"
#شامل "llvm/ADT/ArrayRef.h"
#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/Format.h"
#شامل "llvm/Support/raw_ostream.h"

#شامل <cmath>

#اگر defined(_WIN32) && !defined(__MINGW32__)
#شامل <float.h> // For _fpclass in llvm::write_double.
#اگر_ختم

استعمال نامخلا llvm;

سانچہ<قسم_نام T, std::size_t N>
ساکن صحیح format_مقصد_بفر(T قدر, حرف (&بفر)[N]) {
  حرف *انتہا_Ptr = std::انتہا(بفر);
  حرف *CurPtr = انتہا_Ptr;

  کرو {
    *--CurPtr = '0' + حرف(قدر % 10);
    قدر /= 10;
  } جب (قدر);
  واپس انتہا_Ptr - CurPtr;
}

ساکن خالی writeWithCommas(raw_ostream &S, ArrayRef<حرف> بفر) {
  أكد(!بفر.خالی_خانے());

  ArrayRef<حرف> This_گروہ;
  صحیح InitialDigits = ((بفر.سائز_2() - 1) % 3) + 1;
  This_گروہ = بفر.take_front(InitialDigits);
  S.اكتب(This_گروہ.ڈیٹا(), This_گروہ.سائز_2());

  بفر = بفر.drop_front(InitialDigits);
  أكد(بفر.سائز_2() % 3 == 0);
  جب (!بفر.خالی_خانے()) {
    S << ',';
    This_گروہ = بفر.take_front(3);
    S.اكتب(This_گروہ.ڈیٹا(), 3);
    بفر = بفر.drop_front(3);
  }
}

سانچہ <قسم_نام T>
ساکن خالی write_unsigned_impl(raw_ostream &S, T N, size_t MinDigits,
                                IntegerStyle Style, منطقی IsNegative,
                                منطقی NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  حرف نمبر_بفر[128];
  size_t Len = format_مقصد_بفر(N, نمبر_بفر);

  اگر (IsNegative)
    S << '-';
  ورنہ اگر (NonNegativePlus)
    S << '+';

  اگر (Len < MinDigits && Style != IntegerStyle::نمبر) {
    برائے (size_t I = Len; I < MinDigits; ++I)
      S << '0';
  }

  اگر (Style == IntegerStyle::نمبر) {
    writeWithCommas(S, ArrayRef<حرف>(std::انتہا(نمبر_بفر) - Len, Len));
  } ورنہ {
    S.اكتب(std::انتہا(نمبر_بفر) - Len, Len);
  }
}

سانچہ <قسم_نام T>
ساکن خالی write_unsigned(raw_ostream &S, T N, size_t MinDigits,
                           IntegerStyle Style, منطقی IsNegative = جھوٹ,
                           منطقی NonNegativePlus = جھوٹ) {
  // Output using 32-bit div/mod if possible.
  اگر (N == static_cast<uint32_t>(N))
    write_unsigned_impl(S, static_cast<uint32_t>(N), MinDigits, Style,
                        IsNegative, NonNegativePlus);
  ورنہ
    write_unsigned_impl(S, N, MinDigits, Style, IsNegative, NonNegativePlus);
}

سانچہ <قسم_نام T>
ساکن خالی write_signed(raw_ostream &S, T N, size_t MinDigits,
                         IntegerStyle Style, منطقی NonNegativePlus = جھوٹ) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  استعمال UnsignedT = std::make_unsigned_t<T>;

  اگر (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, جھوٹ,
                   NonNegativePlus);
    واپس;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, سچ, NonNegativePlus);
}

خالی llvm::write_integer(raw_ostream &S, غیرعلامتی صحیح N, size_t MinDigits,
                         IntegerStyle Style, منطقی NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, جھوٹ, NonNegativePlus);
}

خالی llvm::write_integer(raw_ostream &S, صحیح N, size_t MinDigits,
                         IntegerStyle Style, منطقی NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

خالی llvm::write_integer(raw_ostream &S, غیرعلامتی طویل N, size_t MinDigits,
                         IntegerStyle Style, منطقی NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, جھوٹ, NonNegativePlus);
}

خالی llvm::write_integer(raw_ostream &S, طویل N, size_t MinDigits,
                         IntegerStyle Style, منطقی NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

خالی llvm::write_integer(raw_ostream &S, غیرعلامتی طویل طویل N, size_t MinDigits,
                         IntegerStyle Style, منطقی NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, جھوٹ, NonNegativePlus);
}

خالی llvm::write_integer(raw_ostream &S, طویل طویل N, size_t MinDigits,
                         IntegerStyle Style, منطقی NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

خالی llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<size_t> چوڑائی) {
  ثابت size_t k_Max_چوڑائی = 128u;

  size_t W = std::min(k_Max_چوڑائی, چوڑائی.قدر_or(0u));

  غیرعلامتی Nibbles = (llvm::ثنائی_ہندسہ_چوڑائی_3(N) + 3) / 4;
  منطقی سابقہ = (Style == HexPrintStyle::سابقہ_زیریں ||
                 Style == HexPrintStyle::سابقہ_بالائی);
  منطقی بالائی_2 =
      (Style == HexPrintStyle::بالائی_2 || Style == HexPrintStyle::سابقہ_بالائی);
  غیرعلامتی سابقہ_Chars = سابقہ ? 2 : 0;
  غیرعلامتی عدد_Chars =
      std::max(static_cast<غیرعلامتی>(W), std::max(1u, Nibbles) + سابقہ_Chars);

  حرف نمبر_بفر[k_Max_چوڑائی];
  ::memset(نمبر_بفر, '0', std::سائز_2(نمبر_بفر));
  اگر (سابقہ)
    نمبر_بفر[1] = 'x';
  حرف *انتہا_Ptr = نمبر_بفر + عدد_Chars;
  حرف *CurPtr = انتہا_Ptr;
  جب (N) {
    غیرعلامتی حرف x = static_cast<غیرعلامتی حرف>(N) % 16;
    *--CurPtr = hexdigit(x, !بالائی_2);
    N /= 16;
  }

  S.اكتب(نمبر_بفر, عدد_Chars);
}

خالی llvm::write_double(raw_ostream &S, دوہرا N, FloatStyle Style,
                        std::optional<size_t> Precision) {
  size_t Prec = Precision.قدر_or(getDefaultPrecision(Style));

  اگر (std::isnan(N)) {
    S << "nan";
    واپس;
  } ورنہ اگر (std::isinf(N)) {
    S << (std::signbit(N) ? "-INF" : "INF");
    واپس;
  }

  حرف Letter;
  اگر (Style == FloatStyle::Exponent)
    Letter = 'e';
  ورنہ اگر (Style == FloatStyle::Exponent_بالائی)
    Letter = 'E';
  ورنہ
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream برآمد(Spec);
  برآمد << "%." << Prec << Letter;

  اگر (Style == FloatStyle::Exponent || Style == FloatStyle::Exponent_بالائی) {
#اگر_تعریف_شدہ _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#اگر defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    اگر (N == 0.0 && std::signbit(N)) {
      حرف NegativeZero[] = "-0.000000e+00";
      اگر (Style == FloatStyle::Exponent_بالائی)
        NegativeZero[لڑی_لمبائی(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      واپس;
    }
#ورنہ
    صحیح fpcl = _fpclass(N);

    // negative zero
    اگر (fpcl == _FPCLASS_NZ) {
      حرف NegativeZero[] = "-0.000000e+00";
      اگر (Style == FloatStyle::Exponent_بالائی)
        NegativeZero[لڑی_لمبائی(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      واپس;
    }
#اگر_ختم

    حرف buf[32];
    غیرعلامتی len;
    len = format(Spec.c_str(), N).snprint(buf, حجم(buf));
    اگر (len <= حجم(buf) - 2) {
      اگر (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        صحیح cs = buf[len - 4];
        اگر (cs == '+' || cs == '-') {
          صحیح c1 = buf[len - 2];
          صحیح c0 = buf[len - 1];
          اگر (ہندسہ_ہے(static_cast<غیرعلامتی حرف>(c1)) &&
              ہندسہ_ہے(static_cast<غیرعلامتی حرف>(c0))) {
            // Trim leading '0': "...e+012" -> "...e+12\0"
            buf[len - 3] = c1;
            buf[len - 2] = c0;
            buf[--len] = 0;
          }
        }
      }
      S << buf;
      واپس;
    }
#اگر_ختم
  }

  اگر (Style == FloatStyle::Percent)
    N *= 100.0;

  حرف Buf[32];
  format(Spec.c_str(), N).snprint(Buf, حجم(Buf));
  S << Buf;
  اگر (Style == FloatStyle::Percent)
    S << '%';
}

منطقی llvm::isPrefixedHexStyle(HexPrintStyle S) {
  واپس (S == HexPrintStyle::سابقہ_زیریں || S == HexPrintStyle::سابقہ_بالائی);
}

size_t llvm::getDefaultPrecision(FloatStyle Style) {
  انتخاب (Style) {
  حالت FloatStyle::Exponent:
  حالت FloatStyle::Exponent_بالائی:
    واپس 6; // Number of decimal places.
  حالت FloatStyle::Fixed:
  حالت FloatStyle::Percent:
    واپس 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
