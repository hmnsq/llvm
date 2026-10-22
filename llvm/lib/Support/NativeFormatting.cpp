//===- NativeFormatting.cpp - Low level formatting helpers -------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/NativeFormatting.h"
#அந்தர்பாவ "llvm/ADT/ArrayRef.h"
#அந்தர்பாவ "llvm/ADT/SmallString.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Support/Format.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"

#அந்தர்பாவ <cmath>

#எனில் defined(_WIN32) && !defined(__MINGW32__)
#அந்தர்பாவ <float.h> // For _fpclass in llvm::write_double.
#endif

using நாமாகாஶ llvm;

template<typename T, std::size_t N>
நிலையான முழு format_to_buffer(T மதிப்பு, எழுத்து (&Buffer)[N]) {
  எழுத்து *EndPtr = std::end(Buffer);
  எழுத்து *CurPtr = EndPtr;

  செய் {
    *--CurPtr = '0' + எழுத்து(மதிப்பு % 10);
    மதிப்பு /= 10;
  } போது (மதிப்பு);
  திரும்பு EndPtr - CurPtr;
}

நிலையான வெறுமை writeWithCommas(raw_ostream &S, ArrayRef<எழுத்து> Buffer) {
  ப்ரதிஜ்ஞா(!Buffer.empty());

  ArrayRef<எழுத்து> ThisGroup;
  முழு InitialDigits = ((Buffer.size() - 1) % 3) + 1;
  ThisGroup = Buffer.take_front(InitialDigits);
  S.லிக(ThisGroup.data(), ThisGroup.size());

  Buffer = Buffer.drop_front(InitialDigits);
  ப்ரதிஜ்ஞா(Buffer.size() % 3 == 0);
  போது (!Buffer.empty()) {
    S << ',';
    ThisGroup = Buffer.take_front(3);
    S.லிக(ThisGroup.data(), 3);
    Buffer = Buffer.drop_front(3);
  }
}

template <typename T>
நிலையான வெறுமை write_unsigned_impl(raw_ostream &S, T N, size_t MinDigits,
                                IntegerStyle Style, தருக்க IsNegative,
                                தருக்க NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  எழுத்து எண்_Buffer[128];
  size_t Len = format_to_buffer(N, எண்_Buffer);

  எனில் (IsNegative)
    S << '-';
  இல்லையெனில் எனில் (NonNegativePlus)
    S << '+';

  எனில் (Len < MinDigits && Style != IntegerStyle::எண்) {
    க்கு (size_t I = Len; I < MinDigits; ++I)
      S << '0';
  }

  எனில் (Style == IntegerStyle::எண்) {
    writeWithCommas(S, ArrayRef<எழுத்து>(std::end(எண்_Buffer) - Len, Len));
  } இல்லையெனில் {
    S.லிக(std::end(எண்_Buffer) - Len, Len);
  }
}

template <typename T>
நிலையான வெறுமை write_unsigned(raw_ostream &S, T N, size_t MinDigits,
                           IntegerStyle Style, தருக்க IsNegative = பொய்,
                           தருக்க NonNegativePlus = பொய்) {
  // Output using 32-bit div/mod if possible.
  எனில் (N == static_cast<uint32_t>(N))
    write_unsigned_impl(S, static_cast<uint32_t>(N), MinDigits, Style,
                        IsNegative, NonNegativePlus);
  இல்லையெனில்
    write_unsigned_impl(S, N, MinDigits, Style, IsNegative, NonNegativePlus);
}

template <typename T>
நிலையான வெறுமை write_signed(raw_ostream &S, T N, size_t MinDigits,
                         IntegerStyle Style, தருக்க NonNegativePlus = பொய்) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  using UnsignedT = std::make_unsigned_t<T>;

  எனில் (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, பொய்,
                   NonNegativePlus);
    திரும்பு;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, உண்மை, NonNegativePlus);
}

வெறுமை llvm::write_integer(raw_ostream &S, unsigned முழு N, size_t MinDigits,
                         IntegerStyle Style, தருக்க NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, பொய், NonNegativePlus);
}

வெறுமை llvm::write_integer(raw_ostream &S, முழு N, size_t MinDigits,
                         IntegerStyle Style, தருக்க NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

வெறுமை llvm::write_integer(raw_ostream &S, unsigned long N, size_t MinDigits,
                         IntegerStyle Style, தருக்க NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, பொய், NonNegativePlus);
}

வெறுமை llvm::write_integer(raw_ostream &S, long N, size_t MinDigits,
                         IntegerStyle Style, தருக்க NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

வெறுமை llvm::write_integer(raw_ostream &S, unsigned long long N, size_t MinDigits,
                         IntegerStyle Style, தருக்க NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, பொய், NonNegativePlus);
}

வெறுமை llvm::write_integer(raw_ostream &S, long long N, size_t MinDigits,
                         IntegerStyle Style, தருக்க NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

வெறுமை llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<size_t> Width) {
  மாறிலி size_t kMaxWidth = 128u;

  size_t W = std::min(kMaxWidth, Width.மதிப்பு_or(0u));

  unsigned Nibbles = (llvm::bit_width(N) + 3) / 4;
  தருக்க Prefix = (Style == HexPrintStyle::PrefixLower ||
                 Style == HexPrintStyle::PrefixUpper);
  தருக்க Upper =
      (Style == HexPrintStyle::Upper || Style == HexPrintStyle::PrefixUpper);
  unsigned PrefixChars = Prefix ? 2 : 0;
  unsigned இலக்கம்_Chars =
      std::max(static_cast<unsigned>(W), std::max(1u, Nibbles) + PrefixChars);

  எழுத்து எண்_Buffer[kMaxWidth];
  ::memset(எண்_Buffer, '0', std::size(எண்_Buffer));
  எனில் (Prefix)
    எண்_Buffer[1] = 'x';
  எழுத்து *EndPtr = எண்_Buffer + இலக்கம்_Chars;
  எழுத்து *CurPtr = EndPtr;
  போது (N) {
    unsigned எழுத்து x = static_cast<unsigned எழுத்து>(N) % 16;
    *--CurPtr = hexdigit(x, !Upper);
    N /= 16;
  }

  S.லிக(எண்_Buffer, இலக்கம்_Chars);
}

வெறுமை llvm::write_double(raw_ostream &S, இரட்டை N, FloatStyle Style,
                        std::optional<size_t> Precision) {
  size_t Prec = Precision.மதிப்பு_or(getDefaultPrecision(Style));

  எனில் (std::isnan(N)) {
    S << "nan";
    திரும்பு;
  } இல்லையெனில் எனில் (std::isinf(N)) {
    S << (std::signbit(N) ? "-INF" : "INF");
    திரும்பு;
  }

  எழுத்து Letter;
  எனில் (Style == FloatStyle::Exponent)
    Letter = 'e';
  இல்லையெனில் எனில் (Style == FloatStyle::ExponentUpper)
    Letter = 'E';
  இல்லையெனில்
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream Out(Spec);
  Out << "%." << Prec << Letter;

  எனில் (Style == FloatStyle::Exponent || Style == FloatStyle::ExponentUpper) {
#ifdef _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#எனில் defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    எனில் (N == 0.0 && std::signbit(N)) {
      எழுத்து NegativeZero[] = "-0.000000e+00";
      எனில் (Style == FloatStyle::ExponentUpper)
        NegativeZero[strlen(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      திரும்பு;
    }
#இல்லையெனில்
    முழு fpcl = _fpclass(N);

    // negative zero
    எனில் (fpcl == _FPCLASS_NZ) {
      எழுத்து NegativeZero[] = "-0.000000e+00";
      எனில் (Style == FloatStyle::ExponentUpper)
        NegativeZero[strlen(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      திரும்பு;
    }
#endif

    எழுத்து buf[32];
    unsigned len;
    len = format(Spec.c_str(), N).snprint(buf, அளவு(buf));
    எனில் (len <= அளவு(buf) - 2) {
      எனில் (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        முழு cs = buf[len - 4];
        எனில் (cs == '+' || cs == '-') {
          முழு c1 = buf[len - 2];
          முழு c0 = buf[len - 1];
          எனில் (isdigit(static_cast<unsigned எழுத்து>(c1)) &&
              isdigit(static_cast<unsigned எழுத்து>(c0))) {
            // Trim leading '0': "...e+012" -> "...e+12\0"
            buf[len - 3] = c1;
            buf[len - 2] = c0;
            buf[--len] = 0;
          }
        }
      }
      S << buf;
      திரும்பு;
    }
#endif
  }

  எனில் (Style == FloatStyle::Percent)
    N *= 100.0;

  எழுத்து Buf[32];
  format(Spec.c_str(), N).snprint(Buf, அளவு(Buf));
  S << Buf;
  எனில் (Style == FloatStyle::Percent)
    S << '%';
}

தருக்க llvm::isPrefixedHexStyle(HexPrintStyle S) {
  திரும்பு (S == HexPrintStyle::PrefixLower || S == HexPrintStyle::PrefixUpper);
}

size_t llvm::getDefaultPrecision(FloatStyle Style) {
  தேர்வு (Style) {
  நிலை FloatStyle::Exponent:
  நிலை FloatStyle::ExponentUpper:
    திரும்பு 6; // Number of decimal places.
  நிலை FloatStyle::Fixed:
  நிலை FloatStyle::Percent:
    திரும்பு 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
