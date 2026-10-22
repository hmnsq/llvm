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
#endif

using فضاینام llvm;

template<typename T, std::size_t N>
ایستا صحیح format_to_buffer(T مقدار, نویسه (&Buffer)[N]) {
  نویسه *EndPtr = std::end(Buffer);
  نویسه *CurPtr = EndPtr;

  انجامبده {
    *--CurPtr = '0' + نویسه(مقدار % 10);
    مقدار /= 10;
  } وقتی (مقدار);
  بازگشت EndPtr - CurPtr;
}

ایستا تهی writeWithCommas(raw_ostream &S, ArrayRef<نویسه> Buffer) {
  أكد(!Buffer.empty());

  ArrayRef<نویسه> ThisGroup;
  صحیح InitialDigits = ((Buffer.size() - 1) % 3) + 1;
  ThisGroup = Buffer.take_front(InitialDigits);
  S.اكتب(ThisGroup.data(), ThisGroup.size());

  Buffer = Buffer.drop_front(InitialDigits);
  أكد(Buffer.size() % 3 == 0);
  وقتی (!Buffer.empty()) {
    S << ',';
    ThisGroup = Buffer.take_front(3);
    S.اكتب(ThisGroup.data(), 3);
    Buffer = Buffer.drop_front(3);
  }
}

template <typename T>
ایستا تهی write_unsigned_impl(raw_ostream &S, T N, size_t MinDigits,
                                IntegerStyle Style, منطقی IsNegative,
                                منطقی NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  نویسه عدد_Buffer[128];
  size_t Len = format_to_buffer(N, عدد_Buffer);

  اگر (IsNegative)
    S << '-';
  وگرنه اگر (NonNegativePlus)
    S << '+';

  اگر (Len < MinDigits && Style != IntegerStyle::عدد) {
    برای (size_t I = Len; I < MinDigits; ++I)
      S << '0';
  }

  اگر (Style == IntegerStyle::عدد) {
    writeWithCommas(S, ArrayRef<نویسه>(std::end(عدد_Buffer) - Len, Len));
  } وگرنه {
    S.اكتب(std::end(عدد_Buffer) - Len, Len);
  }
}

template <typename T>
ایستا تهی write_unsigned(raw_ostream &S, T N, size_t MinDigits,
                           IntegerStyle Style, منطقی IsNegative = نادرست,
                           منطقی NonNegativePlus = نادرست) {
  // Output using 32-bit div/mod if possible.
  اگر (N == static_cast<uint32_t>(N))
    write_unsigned_impl(S, static_cast<uint32_t>(N), MinDigits, Style,
                        IsNegative, NonNegativePlus);
  وگرنه
    write_unsigned_impl(S, N, MinDigits, Style, IsNegative, NonNegativePlus);
}

template <typename T>
ایستا تهی write_signed(raw_ostream &S, T N, size_t MinDigits,
                         IntegerStyle Style, منطقی NonNegativePlus = نادرست) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  using UnsignedT = std::make_unsigned_t<T>;

  اگر (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, نادرست,
                   NonNegativePlus);
    بازگشت;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, درست, NonNegativePlus);
}

تهی llvm::write_integer(raw_ostream &S, unsigned صحیح N, size_t MinDigits,
                         IntegerStyle Style, منطقی NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, نادرست, NonNegativePlus);
}

تهی llvm::write_integer(raw_ostream &S, صحیح N, size_t MinDigits,
                         IntegerStyle Style, منطقی NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

تهی llvm::write_integer(raw_ostream &S, unsigned long N, size_t MinDigits,
                         IntegerStyle Style, منطقی NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, نادرست, NonNegativePlus);
}

تهی llvm::write_integer(raw_ostream &S, long N, size_t MinDigits,
                         IntegerStyle Style, منطقی NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

تهی llvm::write_integer(raw_ostream &S, unsigned long long N, size_t MinDigits,
                         IntegerStyle Style, منطقی NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, نادرست, NonNegativePlus);
}

تهی llvm::write_integer(raw_ostream &S, long long N, size_t MinDigits,
                         IntegerStyle Style, منطقی NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

تهی llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<size_t> Width) {
  ثابت size_t kMaxWidth = 128u;

  size_t W = std::min(kMaxWidth, Width.مقدار_or(0u));

  unsigned Nibbles = (llvm::bit_width(N) + 3) / 4;
  منطقی Prefix = (Style == HexPrintStyle::PrefixLower ||
                 Style == HexPrintStyle::PrefixUpper);
  منطقی Upper =
      (Style == HexPrintStyle::Upper || Style == HexPrintStyle::PrefixUpper);
  unsigned PrefixChars = Prefix ? 2 : 0;
  unsigned رقم_Chars =
      std::max(static_cast<unsigned>(W), std::max(1u, Nibbles) + PrefixChars);

  نویسه عدد_Buffer[kMaxWidth];
  ::memset(عدد_Buffer, '0', std::size(عدد_Buffer));
  اگر (Prefix)
    عدد_Buffer[1] = 'x';
  نویسه *EndPtr = عدد_Buffer + رقم_Chars;
  نویسه *CurPtr = EndPtr;
  وقتی (N) {
    unsigned نویسه x = static_cast<unsigned نویسه>(N) % 16;
    *--CurPtr = hexdigit(x, !Upper);
    N /= 16;
  }

  S.اكتب(عدد_Buffer, رقم_Chars);
}

تهی llvm::write_double(raw_ostream &S, دوگانه N, FloatStyle Style,
                        std::optional<size_t> Precision) {
  size_t Prec = Precision.مقدار_or(getDefaultPrecision(Style));

  اگر (std::isnan(N)) {
    S << "nan";
    بازگشت;
  } وگرنه اگر (std::isinf(N)) {
    S << (std::signbit(N) ? "-INF" : "INF");
    بازگشت;
  }

  نویسه Letter;
  اگر (Style == FloatStyle::Exponent)
    Letter = 'e';
  وگرنه اگر (Style == FloatStyle::ExponentUpper)
    Letter = 'E';
  وگرنه
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream Out(Spec);
  Out << "%." << Prec << Letter;

  اگر (Style == FloatStyle::Exponent || Style == FloatStyle::ExponentUpper) {
#ifdef _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#اگر defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    اگر (N == 0.0 && std::signbit(N)) {
      نویسه NegativeZero[] = "-0.000000e+00";
      اگر (Style == FloatStyle::ExponentUpper)
        NegativeZero[strlen(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      بازگشت;
    }
#وگرنه
    صحیح fpcl = _fpclass(N);

    // negative zero
    اگر (fpcl == _FPCLASS_NZ) {
      نویسه NegativeZero[] = "-0.000000e+00";
      اگر (Style == FloatStyle::ExponentUpper)
        NegativeZero[strlen(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      بازگشت;
    }
#endif

    نویسه buf[32];
    unsigned len;
    len = format(Spec.c_str(), N).snprint(buf, اندازه(buf));
    اگر (len <= اندازه(buf) - 2) {
      اگر (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        صحیح cs = buf[len - 4];
        اگر (cs == '+' || cs == '-') {
          صحیح c1 = buf[len - 2];
          صحیح c0 = buf[len - 1];
          اگر (isdigit(static_cast<unsigned نویسه>(c1)) &&
              isdigit(static_cast<unsigned نویسه>(c0))) {
            // Trim leading '0': "...e+012" -> "...e+12\0"
            buf[len - 3] = c1;
            buf[len - 2] = c0;
            buf[--len] = 0;
          }
        }
      }
      S << buf;
      بازگشت;
    }
#endif
  }

  اگر (Style == FloatStyle::Percent)
    N *= 100.0;

  نویسه Buf[32];
  format(Spec.c_str(), N).snprint(Buf, اندازه(Buf));
  S << Buf;
  اگر (Style == FloatStyle::Percent)
    S << '%';
}

منطقی llvm::isPrefixedHexStyle(HexPrintStyle S) {
  بازگشت (S == HexPrintStyle::PrefixLower || S == HexPrintStyle::PrefixUpper);
}

size_t llvm::getDefaultPrecision(FloatStyle Style) {
  انتخاب (Style) {
  حالت FloatStyle::Exponent:
  حالت FloatStyle::ExponentUpper:
    بازگشت 6; // Number of decimal places.
  حالت FloatStyle::Fixed:
  حالت FloatStyle::Percent:
    بازگشت 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
