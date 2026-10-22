//===- NativeFormatting.cpp - Low level formatting helpers -------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/NativeFormatting.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ArrayRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Format.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <cmath>

#ਜੇ defined(_WIN32) && !defined(__MINGW32__)
#ਅਨ੍ਤਰ੍ਭਾਵ <float.h> // For _fpclass in llvm::write_double.
#ਜੇ_ਅੰਤ

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਸਾਂਚਾ<ਕਿਸਮ_ਨਾਂ T, std::size_t N>
ਸਥਿਰ ਪੂਰਨ format_ਮੰਜ਼ਿਲ_ਬਫਰ(T ਮੁੱਲ, ਅੱਖਰ (&ਬਫਰ)[N]) {
  ਅੱਖਰ *ਸਿਰਾ_Ptr = std::ਸਿਰਾ(ਬਫਰ);
  ਅੱਖਰ *CurPtr = ਸਿਰਾ_Ptr;

  ਕਰੋ {
    *--CurPtr = '0' + ਅੱਖਰ(ਮੁੱਲ % 10);
    ਮੁੱਲ /= 10;
  } ਜਦੋਂ (ਮੁੱਲ);
  ਵਾਪਸ ਸਿਰਾ_Ptr - CurPtr;
}

ਸਥਿਰ ਖਾਲੀ writeWithCommas(raw_ostream &S, ArrayRef<ਅੱਖਰ> ਬਫਰ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(!ਬਫਰ.ਖਾਲੀ_ਥਾਵਾਂ());

  ArrayRef<ਅੱਖਰ> This_ਸਮੂਹ;
  ਪੂਰਨ InitialDigits = ((ਬਫਰ.ਮਾਪ_2() - 1) % 3) + 1;
  This_ਸਮੂਹ = ਬਫਰ.take_front(InitialDigits);
  S.ਲਿਖ(This_ਸਮੂਹ.ਅੰਕੜੇ(), This_ਸਮੂਹ.ਮਾਪ_2());

  ਬਫਰ = ਬਫਰ.drop_front(InitialDigits);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਬਫਰ.ਮਾਪ_2() % 3 == 0);
  ਜਦੋਂ (!ਬਫਰ.ਖਾਲੀ_ਥਾਵਾਂ()) {
    S << ',';
    This_ਸਮੂਹ = ਬਫਰ.take_front(3);
    S.ਲਿਖ(This_ਸਮੂਹ.ਅੰਕੜੇ(), 3);
    ਬਫਰ = ਬਫਰ.drop_front(3);
  }
}

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ T>
ਸਥਿਰ ਖਾਲੀ write_unsigned_impl(raw_ostream &S, T N, size_t MinDigits,
                                IntegerStyle Style, ਤਾਰਕਿਕ IsNegative,
                                ਤਾਰਕਿਕ NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  ਅੱਖਰ ਸੰਖਿਆ_ਬਫਰ[128];
  size_t Len = format_ਮੰਜ਼ਿਲ_ਬਫਰ(N, ਸੰਖਿਆ_ਬਫਰ);

  ਜੇ (IsNegative)
    S << '-';
  ਨਹੀਂਤਾਂ ਜੇ (NonNegativePlus)
    S << '+';

  ਜੇ (Len < MinDigits && Style != IntegerStyle::ਸੰਖਿਆ) {
    ਲਈ (size_t I = Len; I < MinDigits; ++I)
      S << '0';
  }

  ਜੇ (Style == IntegerStyle::ਸੰਖਿਆ) {
    writeWithCommas(S, ArrayRef<ਅੱਖਰ>(std::ਸਿਰਾ(ਸੰਖਿਆ_ਬਫਰ) - Len, Len));
  } ਨਹੀਂਤਾਂ {
    S.ਲਿਖ(std::ਸਿਰਾ(ਸੰਖਿਆ_ਬਫਰ) - Len, Len);
  }
}

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ T>
ਸਥਿਰ ਖਾਲੀ write_unsigned(raw_ostream &S, T N, size_t MinDigits,
                           IntegerStyle Style, ਤਾਰਕਿਕ IsNegative = ਝੂਠ,
                           ਤਾਰਕਿਕ NonNegativePlus = ਝੂਠ) {
  // Output using 32-bit div/mod if possible.
  ਜੇ (N == static_cast<uint32_t>(N))
    write_unsigned_impl(S, static_cast<uint32_t>(N), MinDigits, Style,
                        IsNegative, NonNegativePlus);
  ਨਹੀਂਤਾਂ
    write_unsigned_impl(S, N, MinDigits, Style, IsNegative, NonNegativePlus);
}

ਸਾਂਚਾ <ਕਿਸਮ_ਨਾਂ T>
ਸਥਿਰ ਖਾਲੀ write_signed(raw_ostream &S, T N, size_t MinDigits,
                         IntegerStyle Style, ਤਾਰਕਿਕ NonNegativePlus = ਝੂਠ) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  ਵਰਤੋਂ UnsignedT = std::make_unsigned_t<T>;

  ਜੇ (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, ਝੂਠ,
                   NonNegativePlus);
    ਵਾਪਸ;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, ਸੱਚ, NonNegativePlus);
}

ਖਾਲੀ llvm::write_integer(raw_ostream &S, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ N, size_t MinDigits,
                         IntegerStyle Style, ਤਾਰਕਿਕ NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, ਝੂਠ, NonNegativePlus);
}

ਖਾਲੀ llvm::write_integer(raw_ostream &S, ਪੂਰਨ N, size_t MinDigits,
                         IntegerStyle Style, ਤਾਰਕਿਕ NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

ਖਾਲੀ llvm::write_integer(raw_ostream &S, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ N, size_t MinDigits,
                         IntegerStyle Style, ਤਾਰਕਿਕ NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, ਝੂਠ, NonNegativePlus);
}

ਖਾਲੀ llvm::write_integer(raw_ostream &S, ਲੰਮਾ N, size_t MinDigits,
                         IntegerStyle Style, ਤਾਰਕਿਕ NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

ਖਾਲੀ llvm::write_integer(raw_ostream &S, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ N, size_t MinDigits,
                         IntegerStyle Style, ਤਾਰਕਿਕ NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, ਝੂਠ, NonNegativePlus);
}

ਖਾਲੀ llvm::write_integer(raw_ostream &S, ਲੰਮਾ ਲੰਮਾ N, size_t MinDigits,
                         IntegerStyle Style, ਤਾਰਕਿਕ NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

ਖਾਲੀ llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<size_t> ਚੌੜਾਈ) {
  ਅਚਲ size_t k_Max_ਚੌੜਾਈ = 128u;

  size_t W = std::min(k_Max_ਚੌੜਾਈ, ਚੌੜਾਈ.ਮੁੱਲ_or(0u));

  ਬਿਨਾਨਿਸ਼ਾਨੀ Nibbles = (llvm::ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_3(N) + 3) / 4;
  ਤਾਰਕਿਕ ਅਗੇਤਰ = (Style == HexPrintStyle::ਅਗੇਤਰ_ਹੇਠਲਾ ||
                 Style == HexPrintStyle::ਅਗੇਤਰ_ਉੱਤਲਾ);
  ਤਾਰਕਿਕ ਉੱਤਲਾ_2 =
      (Style == HexPrintStyle::ਉੱਤਲਾ_2 || Style == HexPrintStyle::ਅਗੇਤਰ_ਉੱਤਲਾ);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅਗੇਤਰ_Chars = ਅਗੇਤਰ ? 2 : 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Chars =
      std::max(static_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ>(W), std::max(1u, Nibbles) + ਅਗੇਤਰ_Chars);

  ਅੱਖਰ ਸੰਖਿਆ_ਬਫਰ[k_Max_ਚੌੜਾਈ];
  ::memset(ਸੰਖਿਆ_ਬਫਰ, '0', std::ਮਾਪ_2(ਸੰਖਿਆ_ਬਫਰ));
  ਜੇ (ਅਗੇਤਰ)
    ਸੰਖਿਆ_ਬਫਰ[1] = 'x';
  ਅੱਖਰ *ਸਿਰਾ_Ptr = ਸੰਖਿਆ_ਬਫਰ + ਅੰਕ_Chars;
  ਅੱਖਰ *CurPtr = ਸਿਰਾ_Ptr;
  ਜਦੋਂ (N) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ x = static_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ>(N) % 16;
    *--CurPtr = hexdigit(x, !ਉੱਤਲਾ_2);
    N /= 16;
  }

  S.ਲਿਖ(ਸੰਖਿਆ_ਬਫਰ, ਅੰਕ_Chars);
}

ਖਾਲੀ llvm::write_double(raw_ostream &S, ਦੁੱਗਣਾ N, FloatStyle Style,
                        std::optional<size_t> Precision) {
  size_t Prec = Precision.ਮੁੱਲ_or(getDefaultPrecision(Style));

  ਜੇ (std::isnan(N)) {
    S << "nan";
    ਵਾਪਸ;
  } ਨਹੀਂਤਾਂ ਜੇ (std::isinf(N)) {
    S << (std::signbit(N) ? "-INF" : "INF");
    ਵਾਪਸ;
  }

  ਅੱਖਰ Letter;
  ਜੇ (Style == FloatStyle::Exponent)
    Letter = 'e';
  ਨਹੀਂਤਾਂ ਜੇ (Style == FloatStyle::Exponent_ਉੱਤਲਾ)
    Letter = 'E';
  ਨਹੀਂਤਾਂ
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream ਬਾਹਰਲਾ(Spec);
  ਬਾਹਰਲਾ << "%." << Prec << Letter;

  ਜੇ (Style == FloatStyle::Exponent || Style == FloatStyle::Exponent_ਉੱਤਲਾ) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#ਜੇ defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    ਜੇ (N == 0.0 && std::signbit(N)) {
      ਅੱਖਰ NegativeZero[] = "-0.000000e+00";
      ਜੇ (Style == FloatStyle::Exponent_ਉੱਤਲਾ)
        NegativeZero[ਲੜੀ_ਲੰਬਾਈ(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      ਵਾਪਸ;
    }
#ਨਹੀਂਤਾਂ
    ਪੂਰਨ fpcl = _fpclass(N);

    // negative zero
    ਜੇ (fpcl == _FPCLASS_NZ) {
      ਅੱਖਰ NegativeZero[] = "-0.000000e+00";
      ਜੇ (Style == FloatStyle::Exponent_ਉੱਤਲਾ)
        NegativeZero[ਲੜੀ_ਲੰਬਾਈ(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      ਵਾਪਸ;
    }
#ਜੇ_ਅੰਤ

    ਅੱਖਰ buf[32];
    ਬਿਨਾਨਿਸ਼ਾਨੀ len;
    len = format(Spec.c_str(), N).snprint(buf, ਆਕਾਰ(buf));
    ਜੇ (len <= ਆਕਾਰ(buf) - 2) {
      ਜੇ (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        ਪੂਰਨ cs = buf[len - 4];
        ਜੇ (cs == '+' || cs == '-') {
          ਪੂਰਨ c1 = buf[len - 2];
          ਪੂਰਨ c0 = buf[len - 1];
          ਜੇ (ਅੰਕ_ਹੈ(static_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ>(c1)) &&
              ਅੰਕ_ਹੈ(static_cast<ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ>(c0))) {
            // Trim leading '0': "...e+012" -> "...e+12\0"
            buf[len - 3] = c1;
            buf[len - 2] = c0;
            buf[--len] = 0;
          }
        }
      }
      S << buf;
      ਵਾਪਸ;
    }
#ਜੇ_ਅੰਤ
  }

  ਜੇ (Style == FloatStyle::Percent)
    N *= 100.0;

  ਅੱਖਰ Buf[32];
  format(Spec.c_str(), N).snprint(Buf, ਆਕਾਰ(Buf));
  S << Buf;
  ਜੇ (Style == FloatStyle::Percent)
    S << '%';
}

ਤਾਰਕਿਕ llvm::isPrefixedHexStyle(HexPrintStyle S) {
  ਵਾਪਸ (S == HexPrintStyle::ਅਗੇਤਰ_ਹੇਠਲਾ || S == HexPrintStyle::ਅਗੇਤਰ_ਉੱਤਲਾ);
}

size_t llvm::getDefaultPrecision(FloatStyle Style) {
  ਚੋਣ (Style) {
  ਹਾਲਤ FloatStyle::Exponent:
  ਹਾਲਤ FloatStyle::Exponent_ਉੱਤਲਾ:
    ਵਾਪਸ 6; // Number of decimal places.
  ਹਾਲਤ FloatStyle::Fixed:
  ਹਾਲਤ FloatStyle::Percent:
    ਵਾਪਸ 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
