//===- NativeFormatting.cpp - Low level formatting helpers -------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/NativeFormatting.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Support/Format.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

#अन्तर्भाव <cmath>

#जइ defined(_WIN32) && !defined(__MINGW32__)
#अन्तर्भाव <float.h> // For _fpclass in llvm::write_double.
#endif

using नामाकाश llvm;

template<typename T, std::size_t N>
स्थिर पूर्णाङ्क format_to_buffer(T मानम्, वर्ण (&Buffer)[N]) {
  वर्ण *EndPtr = std::end(Buffer);
  वर्ण *CurPtr = EndPtr;

  करेहि {
    *--CurPtr = '0' + वर्ण(मानम् % 10);
    मानम् /= 10;
  } जाव (मानम्);
  प्रत्यागम EndPtr - CurPtr;
}

स्थिर शून्य writeWithCommas(raw_ostream &S, ArrayRef<वर्ण> Buffer) {
  प्रतिज्ञा(!Buffer.empty());

  ArrayRef<वर्ण> ThisGroup;
  पूर्णाङ्क InitialDigits = ((Buffer.size() - 1) % 3) + 1;
  ThisGroup = Buffer.take_front(InitialDigits);
  S.लिख(ThisGroup.data(), ThisGroup.size());

  Buffer = Buffer.drop_front(InitialDigits);
  प्रतिज्ञा(Buffer.size() % 3 == 0);
  जाव (!Buffer.empty()) {
    S << ',';
    ThisGroup = Buffer.take_front(3);
    S.लिख(ThisGroup.data(), 3);
    Buffer = Buffer.drop_front(3);
  }
}

template <typename T>
स्थिर शून्य write_unsigned_impl(raw_ostream &S, T N, size_t MinDigits,
                                IntegerStyle Style, तार्किक IsNegative,
                                तार्किक NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  वर्ण सङ्ख्या_Buffer[128];
  size_t Len = format_to_buffer(N, सङ्ख्या_Buffer);

  जइ (IsNegative)
    S << '-';
  अण्णहा जइ (NonNegativePlus)
    S << '+';

  जइ (Len < MinDigits && Style != IntegerStyle::सङ्ख्या) {
    प्रति (size_t I = Len; I < MinDigits; ++I)
      S << '0';
  }

  जइ (Style == IntegerStyle::सङ्ख्या) {
    writeWithCommas(S, ArrayRef<वर्ण>(std::end(सङ्ख्या_Buffer) - Len, Len));
  } अण्णहा {
    S.लिख(std::end(सङ्ख्या_Buffer) - Len, Len);
  }
}

template <typename T>
स्थिर शून्य write_unsigned(raw_ostream &S, T N, size_t MinDigits,
                           IntegerStyle Style, तार्किक IsNegative = मिच्छ,
                           तार्किक NonNegativePlus = मिच्छ) {
  // Output using 32-bit div/mod if possible.
  जइ (N == static_cast<uint32_t>(N))
    write_unsigned_impl(S, static_cast<uint32_t>(N), MinDigits, Style,
                        IsNegative, NonNegativePlus);
  अण्णहा
    write_unsigned_impl(S, N, MinDigits, Style, IsNegative, NonNegativePlus);
}

template <typename T>
स्थिर शून्य write_signed(raw_ostream &S, T N, size_t MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus = मिच्छ) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  using UnsignedT = std::make_unsigned_t<T>;

  जइ (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, मिच्छ,
                   NonNegativePlus);
    प्रत्यागम;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, सच्च, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, unsigned पूर्णाङ्क N, size_t MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, मिच्छ, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, पूर्णाङ्क N, size_t MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, unsigned long N, size_t MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, मिच्छ, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, long N, size_t MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, unsigned long long N, size_t MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, मिच्छ, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, long long N, size_t MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

शून्य llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<size_t> Width) {
  ध्रुव size_t kMaxWidth = 128u;

  size_t W = std::min(kMaxWidth, Width.मानम्_or(0u));

  unsigned Nibbles = (llvm::bit_width(N) + 3) / 4;
  तार्किक Prefix = (Style == HexPrintStyle::PrefixLower ||
                 Style == HexPrintStyle::PrefixUpper);
  तार्किक Upper =
      (Style == HexPrintStyle::Upper || Style == HexPrintStyle::PrefixUpper);
  unsigned PrefixChars = Prefix ? 2 : 0;
  unsigned अङ्कः_Chars =
      std::max(static_cast<unsigned>(W), std::max(1u, Nibbles) + PrefixChars);

  वर्ण सङ्ख्या_Buffer[kMaxWidth];
  ::memset(सङ्ख्या_Buffer, '0', std::size(सङ्ख्या_Buffer));
  जइ (Prefix)
    सङ्ख्या_Buffer[1] = 'x';
  वर्ण *EndPtr = सङ्ख्या_Buffer + अङ्कः_Chars;
  वर्ण *CurPtr = EndPtr;
  जाव (N) {
    unsigned वर्ण x = static_cast<unsigned वर्ण>(N) % 16;
    *--CurPtr = hexdigit(x, !Upper);
    N /= 16;
  }

  S.लिख(सङ्ख्या_Buffer, अङ्कः_Chars);
}

शून्य llvm::write_double(raw_ostream &S, द्विगुण N, FloatStyle Style,
                        std::optional<size_t> Precision) {
  size_t Prec = Precision.मानम्_or(getDefaultPrecision(Style));

  जइ (std::isnan(N)) {
    S << "nan";
    प्रत्यागम;
  } अण्णहा जइ (std::isinf(N)) {
    S << (std::signbit(N) ? "-INF" : "INF");
    प्रत्यागम;
  }

  वर्ण Letter;
  जइ (Style == FloatStyle::Exponent)
    Letter = 'e';
  अण्णहा जइ (Style == FloatStyle::ExponentUpper)
    Letter = 'E';
  अण्णहा
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream Out(Spec);
  Out << "%." << Prec << Letter;

  जइ (Style == FloatStyle::Exponent || Style == FloatStyle::ExponentUpper) {
#ifdef _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#जइ defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    जइ (N == 0.0 && std::signbit(N)) {
      वर्ण NegativeZero[] = "-0.000000e+00";
      जइ (Style == FloatStyle::ExponentUpper)
        NegativeZero[strlen(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      प्रत्यागम;
    }
#अण्णहा
    पूर्णाङ्क fpcl = _fpclass(N);

    // negative zero
    जइ (fpcl == _FPCLASS_NZ) {
      वर्ण NegativeZero[] = "-0.000000e+00";
      जइ (Style == FloatStyle::ExponentUpper)
        NegativeZero[strlen(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      प्रत्यागम;
    }
#endif

    वर्ण buf[32];
    unsigned len;
    len = format(Spec.c_str(), N).snprint(buf, परिमाण(buf));
    जइ (len <= परिमाण(buf) - 2) {
      जइ (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        पूर्णाङ्क cs = buf[len - 4];
        जइ (cs == '+' || cs == '-') {
          पूर्णाङ्क c1 = buf[len - 2];
          पूर्णाङ्क c0 = buf[len - 1];
          जइ (isdigit(static_cast<unsigned वर्ण>(c1)) &&
              isdigit(static_cast<unsigned वर्ण>(c0))) {
            // Trim leading '0': "...e+012" -> "...e+12\0"
            buf[len - 3] = c1;
            buf[len - 2] = c0;
            buf[--len] = 0;
          }
        }
      }
      S << buf;
      प्रत्यागम;
    }
#endif
  }

  जइ (Style == FloatStyle::Percent)
    N *= 100.0;

  वर्ण Buf[32];
  format(Spec.c_str(), N).snprint(Buf, परिमाण(Buf));
  S << Buf;
  जइ (Style == FloatStyle::Percent)
    S << '%';
}

तार्किक llvm::isPrefixedHexStyle(HexPrintStyle S) {
  प्रत्यागम (S == HexPrintStyle::PrefixLower || S == HexPrintStyle::PrefixUpper);
}

size_t llvm::getDefaultPrecision(FloatStyle Style) {
  विकल्प (Style) {
  पक्ष FloatStyle::Exponent:
  पक्ष FloatStyle::ExponentUpper:
    प्रत्यागम 6; // Number of decimal places.
  पक्ष FloatStyle::Fixed:
  पक्ष FloatStyle::Percent:
    प्रत्यागम 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
