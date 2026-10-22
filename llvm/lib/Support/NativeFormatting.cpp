//===- NativeFormatting.cpp - Low level formatting helpers -------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/NativeFormatting.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ArrayRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallString.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Format.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"

#ଅନ୍ତର୍ଭାଵ <cmath>

#ଯଦି defined(_WIN32) && !defined(__MINGW32__)
#ଅନ୍ତର୍ଭାଵ <float.h> // For _fpclass in llvm::write_double.
#endif

using ନାମାକାଶ llvm;

template<typename T, std::size_t N>
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା format_to_buffer(T ମୂଲ୍ୟ, ଅକ୍ଷର (&Buffer)[N]) {
  ଅକ୍ଷର *EndPtr = std::end(Buffer);
  ଅକ୍ଷର *CurPtr = EndPtr;

  କର {
    *--CurPtr = '0' + ଅକ୍ଷର(ମୂଲ୍ୟ % 10);
    ମୂଲ୍ୟ /= 10;
  } ଯେତେବେଳେ (ମୂଲ୍ୟ);
  ଫେରନ୍ତୁ EndPtr - CurPtr;
}

ସ୍ଥିର ଶୂନ୍ୟ writeWithCommas(raw_ostream &S, ArrayRef<ଅକ୍ଷର> Buffer) {
  ପ୍ରତିଜ୍ଞା(!Buffer.empty());

  ArrayRef<ଅକ୍ଷର> ThisGroup;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା InitialDigits = ((Buffer.size() - 1) % 3) + 1;
  ThisGroup = Buffer.take_front(InitialDigits);
  S.ଲିଖ(ThisGroup.data(), ThisGroup.size());

  Buffer = Buffer.drop_front(InitialDigits);
  ପ୍ରତିଜ୍ଞା(Buffer.size() % 3 == 0);
  ଯେତେବେଳେ (!Buffer.empty()) {
    S << ',';
    ThisGroup = Buffer.take_front(3);
    S.ଲିଖ(ThisGroup.data(), 3);
    Buffer = Buffer.drop_front(3);
  }
}

template <typename T>
ସ୍ଥିର ଶୂନ୍ୟ write_unsigned_impl(raw_ostream &S, T N, size_t MinDigits,
                                IntegerStyle Style, ତାର୍କିକ IsNegative,
                                ତାର୍କିକ NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  ଅକ୍ଷର ସଂଖ୍ୟା_Buffer[128];
  size_t Len = format_to_buffer(N, ସଂଖ୍ୟା_Buffer);

  ଯଦି (IsNegative)
    S << '-';
  ଅନ୍ୟଥା ଯଦି (NonNegativePlus)
    S << '+';

  ଯଦି (Len < MinDigits && Style != IntegerStyle::ସଂଖ୍ୟା) {
    ପାଇଁ (size_t I = Len; I < MinDigits; ++I)
      S << '0';
  }

  ଯଦି (Style == IntegerStyle::ସଂଖ୍ୟା) {
    writeWithCommas(S, ArrayRef<ଅକ୍ଷର>(std::end(ସଂଖ୍ୟା_Buffer) - Len, Len));
  } ଅନ୍ୟଥା {
    S.ଲିଖ(std::end(ସଂଖ୍ୟା_Buffer) - Len, Len);
  }
}

template <typename T>
ସ୍ଥିର ଶୂନ୍ୟ write_unsigned(raw_ostream &S, T N, size_t MinDigits,
                           IntegerStyle Style, ତାର୍କିକ IsNegative = ମିଥ୍ୟା,
                           ତାର୍କିକ NonNegativePlus = ମିଥ୍ୟା) {
  // Output using 32-bit div/mod if possible.
  ଯଦି (N == static_cast<uint32_t>(N))
    write_unsigned_impl(S, static_cast<uint32_t>(N), MinDigits, Style,
                        IsNegative, NonNegativePlus);
  ଅନ୍ୟଥା
    write_unsigned_impl(S, N, MinDigits, Style, IsNegative, NonNegativePlus);
}

template <typename T>
ସ୍ଥିର ଶୂନ୍ୟ write_signed(raw_ostream &S, T N, size_t MinDigits,
                         IntegerStyle Style, ତାର୍କିକ NonNegativePlus = ମିଥ୍ୟା) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  using UnsignedT = std::make_unsigned_t<T>;

  ଯଦି (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, ମିଥ୍ୟା,
                   NonNegativePlus);
    ଫେରନ୍ତୁ;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, ସତ୍ୟ, NonNegativePlus);
}

ଶୂନ୍ୟ llvm::write_integer(raw_ostream &S, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା N, size_t MinDigits,
                         IntegerStyle Style, ତାର୍କିକ NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, ମିଥ୍ୟା, NonNegativePlus);
}

ଶୂନ୍ୟ llvm::write_integer(raw_ostream &S, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N, size_t MinDigits,
                         IntegerStyle Style, ତାର୍କିକ NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

ଶୂନ୍ୟ llvm::write_integer(raw_ostream &S, unsigned long N, size_t MinDigits,
                         IntegerStyle Style, ତାର୍କିକ NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, ମିଥ୍ୟା, NonNegativePlus);
}

ଶୂନ୍ୟ llvm::write_integer(raw_ostream &S, long N, size_t MinDigits,
                         IntegerStyle Style, ତାର୍କିକ NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

ଶୂନ୍ୟ llvm::write_integer(raw_ostream &S, unsigned long long N, size_t MinDigits,
                         IntegerStyle Style, ତାର୍କିକ NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, ମିଥ୍ୟା, NonNegativePlus);
}

ଶୂନ୍ୟ llvm::write_integer(raw_ostream &S, long long N, size_t MinDigits,
                         IntegerStyle Style, ତାର୍କିକ NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

ଶୂନ୍ୟ llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<size_t> Width) {
  ଅଚଳ size_t kMaxWidth = 128u;

  size_t W = std::min(kMaxWidth, Width.ମୂଲ୍ୟ_or(0u));

  unsigned Nibbles = (llvm::bit_width(N) + 3) / 4;
  ତାର୍କିକ Prefix = (Style == HexPrintStyle::PrefixLower ||
                 Style == HexPrintStyle::PrefixUpper);
  ତାର୍କିକ Upper =
      (Style == HexPrintStyle::Upper || Style == HexPrintStyle::PrefixUpper);
  unsigned PrefixChars = Prefix ? 2 : 0;
  unsigned ଅଙ୍କ_Chars =
      std::max(static_cast<unsigned>(W), std::max(1u, Nibbles) + PrefixChars);

  ଅକ୍ଷର ସଂଖ୍ୟା_Buffer[kMaxWidth];
  ::memset(ସଂଖ୍ୟା_Buffer, '0', std::size(ସଂଖ୍ୟା_Buffer));
  ଯଦି (Prefix)
    ସଂଖ୍ୟା_Buffer[1] = 'x';
  ଅକ୍ଷର *EndPtr = ସଂଖ୍ୟା_Buffer + ଅଙ୍କ_Chars;
  ଅକ୍ଷର *CurPtr = EndPtr;
  ଯେତେବେଳେ (N) {
    unsigned ଅକ୍ଷର x = static_cast<unsigned ଅକ୍ଷର>(N) % 16;
    *--CurPtr = hexdigit(x, !Upper);
    N /= 16;
  }

  S.ଲିଖ(ସଂଖ୍ୟା_Buffer, ଅଙ୍କ_Chars);
}

ଶୂନ୍ୟ llvm::write_double(raw_ostream &S, ଦ୍ୱିଗୁଣ N, FloatStyle Style,
                        std::optional<size_t> Precision) {
  size_t Prec = Precision.ମୂଲ୍ୟ_or(getDefaultPrecision(Style));

  ଯଦି (std::isnan(N)) {
    S << "nan";
    ଫେରନ୍ତୁ;
  } ଅନ୍ୟଥା ଯଦି (std::isinf(N)) {
    S << (std::signbit(N) ? "-INF" : "INF");
    ଫେରନ୍ତୁ;
  }

  ଅକ୍ଷର Letter;
  ଯଦି (Style == FloatStyle::Exponent)
    Letter = 'e';
  ଅନ୍ୟଥା ଯଦି (Style == FloatStyle::ExponentUpper)
    Letter = 'E';
  ଅନ୍ୟଥା
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream Out(Spec);
  Out << "%." << Prec << Letter;

  ଯଦି (Style == FloatStyle::Exponent || Style == FloatStyle::ExponentUpper) {
#ifdef _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#ଯଦି defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    ଯଦି (N == 0.0 && std::signbit(N)) {
      ଅକ୍ଷର NegativeZero[] = "-0.000000e+00";
      ଯଦି (Style == FloatStyle::ExponentUpper)
        NegativeZero[strlen(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      ଫେରନ୍ତୁ;
    }
#ଅନ୍ୟଥା
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା fpcl = _fpclass(N);

    // negative zero
    ଯଦି (fpcl == _FPCLASS_NZ) {
      ଅକ୍ଷର NegativeZero[] = "-0.000000e+00";
      ଯଦି (Style == FloatStyle::ExponentUpper)
        NegativeZero[strlen(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      ଫେରନ୍ତୁ;
    }
#endif

    ଅକ୍ଷର buf[32];
    unsigned len;
    len = format(Spec.c_str(), N).snprint(buf, ଆକାର(buf));
    ଯଦି (len <= ଆକାର(buf) - 2) {
      ଯଦି (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା cs = buf[len - 4];
        ଯଦି (cs == '+' || cs == '-') {
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା c1 = buf[len - 2];
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା c0 = buf[len - 1];
          ଯଦି (isdigit(static_cast<unsigned ଅକ୍ଷର>(c1)) &&
              isdigit(static_cast<unsigned ଅକ୍ଷର>(c0))) {
            // Trim leading '0': "...e+012" -> "...e+12\0"
            buf[len - 3] = c1;
            buf[len - 2] = c0;
            buf[--len] = 0;
          }
        }
      }
      S << buf;
      ଫେରନ୍ତୁ;
    }
#endif
  }

  ଯଦି (Style == FloatStyle::Percent)
    N *= 100.0;

  ଅକ୍ଷର Buf[32];
  format(Spec.c_str(), N).snprint(Buf, ଆକାର(Buf));
  S << Buf;
  ଯଦି (Style == FloatStyle::Percent)
    S << '%';
}

ତାର୍କିକ llvm::isPrefixedHexStyle(HexPrintStyle S) {
  ଫେରନ୍ତୁ (S == HexPrintStyle::PrefixLower || S == HexPrintStyle::PrefixUpper);
}

size_t llvm::getDefaultPrecision(FloatStyle Style) {
  ବାଛ (Style) {
  ସ୍ଥିତି FloatStyle::Exponent:
  ସ୍ଥିତି FloatStyle::ExponentUpper:
    ଫେରନ୍ତୁ 6; // Number of decimal places.
  ସ୍ଥିତି FloatStyle::Fixed:
  ସ୍ଥିତି FloatStyle::Percent:
    ଫେରନ୍ତୁ 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
