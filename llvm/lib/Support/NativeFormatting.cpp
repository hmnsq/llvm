//===- NativeFormatting.cpp - Low level formatting helpers -------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/Support/NativeFormatting.h"
#അന്തര്ഭാവ "llvm/ADT/ArrayRef.h"
#അന്തര്ഭാവ "llvm/ADT/SmallString.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/Support/Format.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"

#അന്തര്ഭാവ <cmath>

#എങ്കിൽ defined(_WIN32) && !defined(__MINGW32__)
#അന്തര്ഭാവ <float.h> // For _fpclass in llvm::write_double.
#endif

using നാമാകാശ llvm;

template<typename T, std::size_t N>
സ്ഥിതം പൂർണ്ണസംഖ്യ format_to_buffer(T മൂല്യം, അക്ഷരം (&Buffer)[N]) {
  അക്ഷരം *EndPtr = std::end(Buffer);
  അക്ഷരം *CurPtr = EndPtr;

  ചെയ്യുക {
    *--CurPtr = '0' + അക്ഷരം(മൂല്യം % 10);
    മൂല്യം /= 10;
  } ആകുമ്പോൾ (മൂല്യം);
  തിരികെ EndPtr - CurPtr;
}

സ്ഥിതം ശൂന്യം writeWithCommas(raw_ostream &S, ArrayRef<അക്ഷരം> Buffer) {
  പ്രതിജ്ഞാ(!Buffer.empty());

  ArrayRef<അക്ഷരം> ThisGroup;
  പൂർണ്ണസംഖ്യ InitialDigits = ((Buffer.size() - 1) % 3) + 1;
  ThisGroup = Buffer.take_front(InitialDigits);
  S.ലിഖ(ThisGroup.data(), ThisGroup.size());

  Buffer = Buffer.drop_front(InitialDigits);
  പ്രതിജ്ഞാ(Buffer.size() % 3 == 0);
  ആകുമ്പോൾ (!Buffer.empty()) {
    S << ',';
    ThisGroup = Buffer.take_front(3);
    S.ലിഖ(ThisGroup.data(), 3);
    Buffer = Buffer.drop_front(3);
  }
}

template <typename T>
സ്ഥിതം ശൂന്യം write_unsigned_impl(raw_ostream &S, T N, size_t MinDigits,
                                IntegerStyle Style, തർക്കം IsNegative,
                                തർക്കം NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  അക്ഷരം സംഖ്യ_Buffer[128];
  size_t Len = format_to_buffer(N, സംഖ്യ_Buffer);

  എങ്കിൽ (IsNegative)
    S << '-';
  അല്ലെങ്കിൽ എങ്കിൽ (NonNegativePlus)
    S << '+';

  എങ്കിൽ (Len < MinDigits && Style != IntegerStyle::സംഖ്യ) {
    വേണ്ടി (size_t I = Len; I < MinDigits; ++I)
      S << '0';
  }

  എങ്കിൽ (Style == IntegerStyle::സംഖ്യ) {
    writeWithCommas(S, ArrayRef<അക്ഷരം>(std::end(സംഖ്യ_Buffer) - Len, Len));
  } അല്ലെങ്കിൽ {
    S.ലിഖ(std::end(സംഖ്യ_Buffer) - Len, Len);
  }
}

template <typename T>
സ്ഥിതം ശൂന്യം write_unsigned(raw_ostream &S, T N, size_t MinDigits,
                           IntegerStyle Style, തർക്കം IsNegative = അസത്യം,
                           തർക്കം NonNegativePlus = അസത്യം) {
  // Output using 32-bit div/mod if possible.
  എങ്കിൽ (N == static_cast<uint32_t>(N))
    write_unsigned_impl(S, static_cast<uint32_t>(N), MinDigits, Style,
                        IsNegative, NonNegativePlus);
  അല്ലെങ്കിൽ
    write_unsigned_impl(S, N, MinDigits, Style, IsNegative, NonNegativePlus);
}

template <typename T>
സ്ഥിതം ശൂന്യം write_signed(raw_ostream &S, T N, size_t MinDigits,
                         IntegerStyle Style, തർക്കം NonNegativePlus = അസത്യം) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  using UnsignedT = std::make_unsigned_t<T>;

  എങ്കിൽ (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, അസത്യം,
                   NonNegativePlus);
    തിരികെ;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, സത്യം, NonNegativePlus);
}

ശൂന്യം llvm::write_integer(raw_ostream &S, unsigned പൂർണ്ണസംഖ്യ N, size_t MinDigits,
                         IntegerStyle Style, തർക്കം NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, അസത്യം, NonNegativePlus);
}

ശൂന്യം llvm::write_integer(raw_ostream &S, പൂർണ്ണസംഖ്യ N, size_t MinDigits,
                         IntegerStyle Style, തർക്കം NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

ശൂന്യം llvm::write_integer(raw_ostream &S, unsigned long N, size_t MinDigits,
                         IntegerStyle Style, തർക്കം NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, അസത്യം, NonNegativePlus);
}

ശൂന്യം llvm::write_integer(raw_ostream &S, long N, size_t MinDigits,
                         IntegerStyle Style, തർക്കം NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

ശൂന്യം llvm::write_integer(raw_ostream &S, unsigned long long N, size_t MinDigits,
                         IntegerStyle Style, തർക്കം NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, അസത്യം, NonNegativePlus);
}

ശൂന്യം llvm::write_integer(raw_ostream &S, long long N, size_t MinDigits,
                         IntegerStyle Style, തർക്കം NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

ശൂന്യം llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<size_t> Width) {
  സ്ഥിരം size_t kMaxWidth = 128u;

  size_t W = std::min(kMaxWidth, Width.മൂല്യം_or(0u));

  unsigned Nibbles = (llvm::bit_width(N) + 3) / 4;
  തർക്കം Prefix = (Style == HexPrintStyle::PrefixLower ||
                 Style == HexPrintStyle::PrefixUpper);
  തർക്കം Upper =
      (Style == HexPrintStyle::Upper || Style == HexPrintStyle::PrefixUpper);
  unsigned PrefixChars = Prefix ? 2 : 0;
  unsigned അക്കം_Chars =
      std::max(static_cast<unsigned>(W), std::max(1u, Nibbles) + PrefixChars);

  അക്ഷരം സംഖ്യ_Buffer[kMaxWidth];
  ::memset(സംഖ്യ_Buffer, '0', std::size(സംഖ്യ_Buffer));
  എങ്കിൽ (Prefix)
    സംഖ്യ_Buffer[1] = 'x';
  അക്ഷരം *EndPtr = സംഖ്യ_Buffer + അക്കം_Chars;
  അക്ഷരം *CurPtr = EndPtr;
  ആകുമ്പോൾ (N) {
    unsigned അക്ഷരം x = static_cast<unsigned അക്ഷരം>(N) % 16;
    *--CurPtr = hexdigit(x, !Upper);
    N /= 16;
  }

  S.ലിഖ(സംഖ്യ_Buffer, അക്കം_Chars);
}

ശൂന്യം llvm::write_double(raw_ostream &S, ഇരട്ട N, FloatStyle Style,
                        std::optional<size_t> Precision) {
  size_t Prec = Precision.മൂല്യം_or(getDefaultPrecision(Style));

  എങ്കിൽ (std::isnan(N)) {
    S << "nan";
    തിരികെ;
  } അല്ലെങ്കിൽ എങ്കിൽ (std::isinf(N)) {
    S << (std::signbit(N) ? "-INF" : "INF");
    തിരികെ;
  }

  അക്ഷരം Letter;
  എങ്കിൽ (Style == FloatStyle::Exponent)
    Letter = 'e';
  അല്ലെങ്കിൽ എങ്കിൽ (Style == FloatStyle::ExponentUpper)
    Letter = 'E';
  അല്ലെങ്കിൽ
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream Out(Spec);
  Out << "%." << Prec << Letter;

  എങ്കിൽ (Style == FloatStyle::Exponent || Style == FloatStyle::ExponentUpper) {
#ifdef _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#എങ്കിൽ defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    എങ്കിൽ (N == 0.0 && std::signbit(N)) {
      അക്ഷരം NegativeZero[] = "-0.000000e+00";
      എങ്കിൽ (Style == FloatStyle::ExponentUpper)
        NegativeZero[strlen(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      തിരികെ;
    }
#അല്ലെങ്കിൽ
    പൂർണ്ണസംഖ്യ fpcl = _fpclass(N);

    // negative zero
    എങ്കിൽ (fpcl == _FPCLASS_NZ) {
      അക്ഷരം NegativeZero[] = "-0.000000e+00";
      എങ്കിൽ (Style == FloatStyle::ExponentUpper)
        NegativeZero[strlen(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      തിരികെ;
    }
#endif

    അക്ഷരം buf[32];
    unsigned len;
    len = format(Spec.c_str(), N).snprint(buf, വലുപ്പം(buf));
    എങ്കിൽ (len <= വലുപ്പം(buf) - 2) {
      എങ്കിൽ (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        പൂർണ്ണസംഖ്യ cs = buf[len - 4];
        എങ്കിൽ (cs == '+' || cs == '-') {
          പൂർണ്ണസംഖ്യ c1 = buf[len - 2];
          പൂർണ്ണസംഖ്യ c0 = buf[len - 1];
          എങ്കിൽ (isdigit(static_cast<unsigned അക്ഷരം>(c1)) &&
              isdigit(static_cast<unsigned അക്ഷരം>(c0))) {
            // Trim leading '0': "...e+012" -> "...e+12\0"
            buf[len - 3] = c1;
            buf[len - 2] = c0;
            buf[--len] = 0;
          }
        }
      }
      S << buf;
      തിരികെ;
    }
#endif
  }

  എങ്കിൽ (Style == FloatStyle::Percent)
    N *= 100.0;

  അക്ഷരം Buf[32];
  format(Spec.c_str(), N).snprint(Buf, വലുപ്പം(Buf));
  S << Buf;
  എങ്കിൽ (Style == FloatStyle::Percent)
    S << '%';
}

തർക്കം llvm::isPrefixedHexStyle(HexPrintStyle S) {
  തിരികെ (S == HexPrintStyle::PrefixLower || S == HexPrintStyle::PrefixUpper);
}

size_t llvm::getDefaultPrecision(FloatStyle Style) {
  തിരഞ്ഞെടുപ്പ് (Style) {
  അവസ്ഥ FloatStyle::Exponent:
  അവസ്ഥ FloatStyle::ExponentUpper:
    തിരികെ 6; // Number of decimal places.
  അവസ്ഥ FloatStyle::Fixed:
  അവസ്ഥ FloatStyle::Percent:
    തിരികെ 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
