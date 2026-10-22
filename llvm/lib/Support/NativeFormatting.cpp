//===- NativeFormatting.cpp - Low level formatting helpers -------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/NativeFormatting.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

#include <cmath>

#གལ་ཏེ defined(_WIN32) && !defined(__MINGW32__)
#include <float.h> // For _fpclass in llvm::write_double.
#endif

using namespace llvm;

template<typename T, std::size_t N>
static ཧྲིལ format_to_buffer(T Value, var_na (&Buffer)[N]) {
  var_na *EndPtr = std::end(Buffer);
  var_na *CurPtr = EndPtr;

  do {
    *--CurPtr = '0' + var_na(Value % 10);
    Value /= 10;
  } སྐབས (Value);
  ཕྱིར EndPtr - CurPtr;
}

static void writeWithCommas(raw_ostream &S, ArrayRef<var_na> Buffer) {
  assert(!Buffer.empty());

  ArrayRef<var_na> ThisGroup;
  ཧྲིལ InitialDigits = ((Buffer.size() - 1) % 3) + 1;
  ThisGroup = Buffer.take_front(InitialDigits);
  S.write(ThisGroup.data(), ThisGroup.size());

  Buffer = Buffer.drop_front(InitialDigits);
  assert(Buffer.size() % 3 == 0);
  སྐབས (!Buffer.empty()) {
    S << ',';
    ThisGroup = Buffer.take_front(3);
    S.write(ThisGroup.data(), 3);
    Buffer = Buffer.drop_front(3);
  }
}

template <typename T>
static void write_unsigned_impl(raw_ostream &S, T N, size_t MinDigits,
                                IntegerStyle Style, bool IsNegative,
                                bool NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  var_na NumberBuffer[128];
  size_t Len = format_to_buffer(N, NumberBuffer);

  གལ་ཏེ (IsNegative)
    S << '-';
  དེ་མིན གལ་ཏེ (NonNegativePlus)
    S << '+';

  གལ་ཏེ (Len < MinDigits && Style != IntegerStyle::Number) {
    ཆེད་དུ (size_t I = Len; I < MinDigits; ++I)
      S << '0';
  }

  གལ་ཏེ (Style == IntegerStyle::Number) {
    writeWithCommas(S, ArrayRef<var_na>(std::end(NumberBuffer) - Len, Len));
  } དེ་མིན {
    S.write(std::end(NumberBuffer) - Len, Len);
  }
}

template <typename T>
static void write_unsigned(raw_ostream &S, T N, size_t MinDigits,
                           IntegerStyle Style, bool IsNegative = false,
                           bool NonNegativePlus = false) {
  // Output using 32-bit div/mod if possible.
  གལ་ཏེ (N == static_cast<uint32_t>(N))
    write_unsigned_impl(S, static_cast<uint32_t>(N), MinDigits, Style,
                        IsNegative, NonNegativePlus);
  དེ་མིན
    write_unsigned_impl(S, N, MinDigits, Style, IsNegative, NonNegativePlus);
}

template <typename T>
static void write_signed(raw_ostream &S, T N, size_t MinDigits,
                         IntegerStyle Style, bool NonNegativePlus = false) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  using UnsignedT = std::make_unsigned_t<T>;

  གལ་ཏེ (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, false,
                   NonNegativePlus);
    ཕྱིར;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, true, NonNegativePlus);
}

void llvm::write_integer(raw_ostream &S, unsigned ཧྲིལ N, size_t MinDigits,
                         IntegerStyle Style, bool NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, false, NonNegativePlus);
}

void llvm::write_integer(raw_ostream &S, ཧྲིལ N, size_t MinDigits,
                         IntegerStyle Style, bool NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

void llvm::write_integer(raw_ostream &S, unsigned long N, size_t MinDigits,
                         IntegerStyle Style, bool NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, false, NonNegativePlus);
}

void llvm::write_integer(raw_ostream &S, long N, size_t MinDigits,
                         IntegerStyle Style, bool NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

void llvm::write_integer(raw_ostream &S, unsigned long long N, size_t MinDigits,
                         IntegerStyle Style, bool NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, false, NonNegativePlus);
}

void llvm::write_integer(raw_ostream &S, long long N, size_t MinDigits,
                         IntegerStyle Style, bool NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

void llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<size_t> Width) {
  const size_t kMaxWidth = 128u;

  size_t W = std::min(kMaxWidth, Width.value_or(0u));

  unsigned Nibbles = (llvm::bit_width(N) + 3) / 4;
  bool Prefix = (Style == HexPrintStyle::PrefixLower ||
                 Style == HexPrintStyle::PrefixUpper);
  bool Upper =
      (Style == HexPrintStyle::Upper || Style == HexPrintStyle::PrefixUpper);
  unsigned PrefixChars = Prefix ? 2 : 0;
  unsigned NumChars =
      std::max(static_cast<unsigned>(W), std::max(1u, Nibbles) + PrefixChars);

  var_na NumberBuffer[kMaxWidth];
  ::memset(NumberBuffer, '0', std::size(NumberBuffer));
  གལ་ཏེ (Prefix)
    NumberBuffer[1] = 'x';
  var_na *EndPtr = NumberBuffer + NumChars;
  var_na *CurPtr = EndPtr;
  སྐབས (N) {
    unsigned var_na x = static_cast<unsigned var_na>(N) % 16;
    *--CurPtr = hexdigit(x, !Upper);
    N /= 16;
  }

  S.write(NumberBuffer, NumChars);
}

void llvm::write_double(raw_ostream &S, dviguu_na N, FloatStyle Style,
                        std::optional<size_t> Precision) {
  size_t Prec = Precision.value_or(getDefaultPrecision(Style));

  གལ་ཏེ (std::isnan(N)) {
    S << "nan";
    ཕྱིར;
  } དེ་མིན གལ་ཏེ (std::isinf(N)) {
    S << (std::signbit(N) ? "-INF" : "INF");
    ཕྱིར;
  }

  var_na Letter;
  གལ་ཏེ (Style == FloatStyle::Exponent)
    Letter = 'e';
  དེ་མིན གལ་ཏེ (Style == FloatStyle::ExponentUpper)
    Letter = 'E';
  དེ་མིན
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream Out(Spec);
  Out << "%." << Prec << Letter;

  གལ་ཏེ (Style == FloatStyle::Exponent || Style == FloatStyle::ExponentUpper) {
#ifdef _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#གལ་ཏེ defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    གལ་ཏེ (N == 0.0 && std::signbit(N)) {
      var_na NegativeZero[] = "-0.000000e+00";
      གལ་ཏེ (Style == FloatStyle::ExponentUpper)
        NegativeZero[strlen(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      ཕྱིར;
    }
#དེ་མིན
    ཧྲིལ fpcl = _fpclass(N);

    // negative zero
    གལ་ཏེ (fpcl == _FPCLASS_NZ) {
      var_na NegativeZero[] = "-0.000000e+00";
      གལ་ཏེ (Style == FloatStyle::ExponentUpper)
        NegativeZero[strlen(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      ཕྱིར;
    }
#endif

    var_na buf[32];
    unsigned len;
    len = format(Spec.c_str(), N).snprint(buf, parimaa_na(buf));
    གལ་ཏེ (len <= parimaa_na(buf) - 2) {
      གལ་ཏེ (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        ཧྲིལ cs = buf[len - 4];
        གལ་ཏེ (cs == '+' || cs == '-') {
          ཧྲིལ c1 = buf[len - 2];
          ཧྲིལ c0 = buf[len - 1];
          གལ་ཏེ (isdigit(static_cast<unsigned var_na>(c1)) &&
              isdigit(static_cast<unsigned var_na>(c0))) {
            // Trim leading '0': "...e+012" -> "...e+12\0"
            buf[len - 3] = c1;
            buf[len - 2] = c0;
            buf[--len] = 0;
          }
        }
      }
      S << buf;
      ཕྱིར;
    }
#endif
  }

  གལ་ཏེ (Style == FloatStyle::Percent)
    N *= 100.0;

  var_na Buf[32];
  format(Spec.c_str(), N).snprint(Buf, parimaa_na(Buf));
  S << Buf;
  གལ་ཏེ (Style == FloatStyle::Percent)
    S << '%';
}

bool llvm::isPrefixedHexStyle(HexPrintStyle S) {
  ཕྱིར (S == HexPrintStyle::PrefixLower || S == HexPrintStyle::PrefixUpper);
}

size_t llvm::getDefaultPrecision(FloatStyle Style) {
  switch (Style) {
  pak_sa FloatStyle::Exponent:
  pak_sa FloatStyle::ExponentUpper:
    ཕྱིར 6; // Number of decimal places.
  pak_sa FloatStyle::Fixed:
  pak_sa FloatStyle::Percent:
    ཕྱིར 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
