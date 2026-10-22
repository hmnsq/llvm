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
#endif

using نطاق llvm;

template<typename T, std::size_t N>
ساكن صحيح format_to_buffer(T قىممەت, حرف (&Buffer)[N]) {
  حرف *EndPtr = std::end(Buffer);
  حرف *CurPtr = EndPtr;

  افعل {
    *--CurPtr = '0' + حرف(قىممەت % 10);
    قىممەت /= 10;
  } طالما (قىممەت);
  أرجع EndPtr - CurPtr;
}

ساكن فراغ writeWithCommas(raw_ostream &S, ArrayRef<حرف> Buffer) {
  أكد(!Buffer.empty());

  ArrayRef<حرف> ThisGroup;
  صحيح InitialDigits = ((Buffer.size() - 1) % 3) + 1;
  ThisGroup = Buffer.take_front(InitialDigits);
  S.اكتب(ThisGroup.data(), ThisGroup.size());

  Buffer = Buffer.drop_front(InitialDigits);
  أكد(Buffer.size() % 3 == 0);
  طالما (!Buffer.empty()) {
    S << ',';
    ThisGroup = Buffer.take_front(3);
    S.اكتب(ThisGroup.data(), 3);
    Buffer = Buffer.drop_front(3);
  }
}

template <typename T>
ساكن فراغ write_unsigned_impl(raw_ostream &S, T N, size_t MinDigits,
                                IntegerStyle Style, منطقي IsNegative,
                                منطقي NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  حرف سان_Buffer[128];
  size_t Len = format_to_buffer(N, سان_Buffer);

  إذا (IsNegative)
    S << '-';
  وإلا إذا (NonNegativePlus)
    S << '+';

  إذا (Len < MinDigits && Style != IntegerStyle::سان) {
    لكل (size_t I = Len; I < MinDigits; ++I)
      S << '0';
  }

  إذا (Style == IntegerStyle::سان) {
    writeWithCommas(S, ArrayRef<حرف>(std::end(سان_Buffer) - Len, Len));
  } وإلا {
    S.اكتب(std::end(سان_Buffer) - Len, Len);
  }
}

template <typename T>
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

template <typename T>
ساكن فراغ write_signed(raw_ostream &S, T N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus = خطأ) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  using UnsignedT = std::make_unsigned_t<T>;

  إذا (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, خطأ,
                   NonNegativePlus);
    أرجع;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, true, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, unsigned صحيح N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, خطأ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, صحيح N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, unsigned long N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, خطأ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, long N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, unsigned long long N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, خطأ, NonNegativePlus);
}

فراغ llvm::write_integer(raw_ostream &S, long long N, size_t MinDigits,
                         IntegerStyle Style, منطقي NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

فراغ llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<size_t> Width) {
  ثابت size_t kMaxWidth = 128u;

  size_t W = std::min(kMaxWidth, Width.قىممەت_or(0u));

  unsigned Nibbles = (llvm::bit_width(N) + 3) / 4;
  منطقي Prefix = (Style == HexPrintStyle::PrefixLower ||
                 Style == HexPrintStyle::PrefixUpper);
  منطقي Upper =
      (Style == HexPrintStyle::Upper || Style == HexPrintStyle::PrefixUpper);
  unsigned PrefixChars = Prefix ? 2 : 0;
  unsigned رەقەم_Chars =
      std::max(static_cast<unsigned>(W), std::max(1u, Nibbles) + PrefixChars);

  حرف سان_Buffer[kMaxWidth];
  ::memset(سان_Buffer, '0', std::size(سان_Buffer));
  إذا (Prefix)
    سان_Buffer[1] = 'x';
  حرف *EndPtr = سان_Buffer + رەقەم_Chars;
  حرف *CurPtr = EndPtr;
  طالما (N) {
    unsigned حرف x = static_cast<unsigned حرف>(N) % 16;
    *--CurPtr = hexdigit(x, !Upper);
    N /= 16;
  }

  S.اكتب(سان_Buffer, رەقەم_Chars);
}

فراغ llvm::write_double(raw_ostream &S, مزدوج N, FloatStyle Style,
                        std::optional<size_t> Precision) {
  size_t Prec = Precision.قىممەت_or(getDefaultPrecision(Style));

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
  وإلا إذا (Style == FloatStyle::ExponentUpper)
    Letter = 'E';
  وإلا
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream Out(Spec);
  Out << "%." << Prec << Letter;

  إذا (Style == FloatStyle::Exponent || Style == FloatStyle::ExponentUpper) {
#ifdef _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#إذا defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    إذا (N == 0.0 && std::signbit(N)) {
      حرف NegativeZero[] = "-0.000000e+00";
      إذا (Style == FloatStyle::ExponentUpper)
        NegativeZero[strlen(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      أرجع;
    }
#وإلا
    صحيح fpcl = _fpclass(N);

    // negative zero
    إذا (fpcl == _FPCLASS_NZ) {
      حرف NegativeZero[] = "-0.000000e+00";
      إذا (Style == FloatStyle::ExponentUpper)
        NegativeZero[strlen(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      أرجع;
    }
#endif

    حرف buf[32];
    unsigned len;
    len = format(Spec.c_str(), N).snprint(buf, حجم(buf));
    إذا (len <= حجم(buf) - 2) {
      إذا (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        صحيح cs = buf[len - 4];
        إذا (cs == '+' || cs == '-') {
          صحيح c1 = buf[len - 2];
          صحيح c0 = buf[len - 1];
          إذا (isdigit(static_cast<unsigned حرف>(c1)) &&
              isdigit(static_cast<unsigned حرف>(c0))) {
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
#endif
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
  أرجع (S == HexPrintStyle::PrefixLower || S == HexPrintStyle::PrefixUpper);
}

size_t llvm::getDefaultPrecision(FloatStyle Style) {
  بدل (Style) {
  حالة FloatStyle::Exponent:
  حالة FloatStyle::ExponentUpper:
    أرجع 6; // Number of decimal places.
  حالة FloatStyle::Fixed:
  حالة FloatStyle::Percent:
    أرجع 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
