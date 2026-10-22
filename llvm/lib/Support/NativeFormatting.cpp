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
#جيڪڏهن_پورو

استعمال نطاق llvm;

سانچو<قسم_نالو T, std::size_t N>
ساكن صحيح format_منزل_بفر(T قدر, حرف (&بفر)[N]) {
  حرف *پڇاڙي_Ptr = std::پڇاڙي(بفر);
  حرف *CurPtr = پڇاڙي_Ptr;

  افعل {
    *--CurPtr = '0' + حرف(قدر % 10);
    قدر /= 10;
  } طالما (قدر);
  أرجع پڇاڙي_Ptr - CurPtr;
}

ساكن فراغ writeWithCommas(raw_ostream &S, ArrayRef<حرف> بفر) {
  أكد(!بفر.خالي_جايون());

  ArrayRef<حرف> This_گروهه;
  صحيح InitialDigits = ((بفر.سائيز_2() - 1) % 3) + 1;
  This_گروهه = بفر.take_front(InitialDigits);
  S.اكتب(This_گروهه.معلومات(), This_گروهه.سائيز_2());

  بفر = بفر.drop_front(InitialDigits);
  أكد(بفر.سائيز_2() % 3 == 0);
  طالما (!بفر.خالي_جايون()) {
    S << ',';
    This_گروهه = بفر.take_front(3);
    S.اكتب(This_گروهه.معلومات(), 3);
    بفر = بفر.drop_front(3);
  }
}

سانچو <قسم_نالو T>
ساكن فراغ write_unsigned_impl(raw_ostream &S, T N, size_t MinDigits,
                                IntegerStyle Style, منطقي IsNegative,
                                منطقي NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  حرف عدد_بفر[128];
  size_t Len = format_منزل_بفر(N, عدد_بفر);

  إذا (IsNegative)
    S << '-';
  وإلا إذا (NonNegativePlus)
    S << '+';

  إذا (Len < MinDigits && Style != IntegerStyle::عدد) {
    لكل (size_t I = Len; I < MinDigits; ++I)
      S << '0';
  }

  إذا (Style == IntegerStyle::عدد) {
    writeWithCommas(S, ArrayRef<حرف>(std::پڇاڙي(عدد_بفر) - Len, Len));
  } وإلا {
    S.اكتب(std::پڇاڙي(عدد_بفر) - Len, Len);
  }
}

سانچو <قسم_نالو T>
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

سانچو <قسم_نالو T>
ساكن فراغ write_signed(raw_ostream &S, T N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus = خطأ) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  استعمال UnsignedT = std::make_unsigned_t<T>;

  إذا (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, خطأ,
                   NonNegativePlus);
    أرجع;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, سچ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, بنانشان صحيح N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, خطأ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, صحيح N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, بنانشان ڊگهو N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, خطأ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, ڊگهو N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, بنانشان ڊگهو ڊگهو N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, خطأ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, ڊگهو ڊگهو N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

فراغ llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<size_t> ويڪر) {
  ثابت size_t k_Max_ويڪر = 128u;

  size_t W = std::min(k_Max_ويڪر, ويڪر.قدر_or(0u));

  بنانشان Nibbles = (llvm::ٻٽو_انگ_ويڪر_3(N) + 3) / 4;
  منطقي اڳياڙي = (Style == HexPrintStyle::اڳياڙي_هيٺاهون ||
                 Style == HexPrintStyle::اڳياڙي_مٿانهون);
  منطقي مٿانهون_2 =
      (Style == HexPrintStyle::مٿانهون_2 || Style == HexPrintStyle::اڳياڙي_مٿانهون);
  بنانشان اڳياڙي_Chars = اڳياڙي ? 2 : 0;
  بنانشان انگ_Chars =
      std::max(static_cast<بنانشان>(W), std::max(1u, Nibbles) + اڳياڙي_Chars);

  حرف عدد_بفر[k_Max_ويڪر];
  ::memset(عدد_بفر, '0', std::سائيز_2(عدد_بفر));
  إذا (اڳياڙي)
    عدد_بفر[1] = 'x';
  حرف *پڇاڙي_Ptr = عدد_بفر + انگ_Chars;
  حرف *CurPtr = پڇاڙي_Ptr;
  طالما (N) {
    بنانشان حرف x = static_cast<بنانشان حرف>(N) % 16;
    *--CurPtr = hexdigit(x, !مٿانهون_2);
    N /= 16;
  }

  S.اكتب(عدد_بفر, انگ_Chars);
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
  وإلا إذا (Style == FloatStyle::Exponent_مٿانهون)
    Letter = 'E';
  وإلا
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream ٻاهريون(Spec);
  ٻاهريون << "%." << Prec << Letter;

  إذا (Style == FloatStyle::Exponent || Style == FloatStyle::Exponent_مٿانهون) {
#جيڪڏهن_وصف_ٿيل _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#إذا defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    إذا (N == 0.0 && std::signbit(N)) {
      حرف NegativeZero[] = "-0.000000e+00";
      إذا (Style == FloatStyle::Exponent_مٿانهون)
        NegativeZero[لڙي_ڊيگهه(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      أرجع;
    }
#وإلا
    صحيح fpcl = _fpclass(N);

    // negative zero
    إذا (fpcl == _FPCLASS_NZ) {
      حرف NegativeZero[] = "-0.000000e+00";
      إذا (Style == FloatStyle::Exponent_مٿانهون)
        NegativeZero[لڙي_ڊيگهه(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      أرجع;
    }
#جيڪڏهن_پورو

    حرف buf[32];
    بنانشان len;
    len = format(Spec.c_str(), N).snprint(buf, حجم(buf));
    إذا (len <= حجم(buf) - 2) {
      إذا (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        صحيح cs = buf[len - 4];
        إذا (cs == '+' || cs == '-') {
          صحيح c1 = buf[len - 2];
          صحيح c0 = buf[len - 1];
          إذا (انگ_آهي(static_cast<بنانشان حرف>(c1)) &&
              انگ_آهي(static_cast<بنانشان حرف>(c0))) {
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
#جيڪڏهن_پورو
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
  أرجع (S == HexPrintStyle::اڳياڙي_هيٺاهون || S == HexPrintStyle::اڳياڙي_مٿانهون);
}

size_t llvm::getDefaultPrecision(FloatStyle Style) {
  بدل (Style) {
  حالة FloatStyle::Exponent:
  حالة FloatStyle::Exponent_مٿانهون:
    أرجع 6; // Number of decimal places.
  حالة FloatStyle::Fixed:
  حالة FloatStyle::Percent:
    أرجع 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
