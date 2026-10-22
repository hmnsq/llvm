//===- NativeFormatting.cpp - Low level formatting helpers -------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/NativeFormatting.h"
#અન્તર્ભાવ "llvm/ADT/ArrayRef.h"
#અન્તર્ભાવ "llvm/ADT/SmallString.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/Support/Format.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"

#અન્તર્ભાવ <cmath>

#જો defined(_WIN32) && !defined(__MINGW32__)
#અન્તર્ભાવ <float.h> // For _fpclass in llvm::write_double.
#endif

using નામાકાશ llvm;

template<typename T, std::size_t N>
સ્થિર પૂર્ણાંક format_to_buffer(T કિંમત, અક્ષર (&Buffer)[N]) {
  અક્ષર *EndPtr = std::end(Buffer);
  અક્ષર *CurPtr = EndPtr;

  કરો {
    *--CurPtr = '0' + અક્ષર(કિંમત % 10);
    કિંમત /= 10;
  } જ્યારે (કિંમત);
  પાછા EndPtr - CurPtr;
}

સ્થિર શૂન્ય writeWithCommas(raw_ostream &S, ArrayRef<અક્ષર> Buffer) {
  પ્રતિજ્ઞા(!Buffer.empty());

  ArrayRef<અક્ષર> ThisGroup;
  પૂર્ણાંક InitialDigits = ((Buffer.size() - 1) % 3) + 1;
  ThisGroup = Buffer.take_front(InitialDigits);
  S.લિખ(ThisGroup.data(), ThisGroup.size());

  Buffer = Buffer.drop_front(InitialDigits);
  પ્રતિજ્ઞા(Buffer.size() % 3 == 0);
  જ્યારે (!Buffer.empty()) {
    S << ',';
    ThisGroup = Buffer.take_front(3);
    S.લિખ(ThisGroup.data(), 3);
    Buffer = Buffer.drop_front(3);
  }
}

template <typename T>
સ્થિર શૂન્ય write_unsigned_impl(raw_ostream &S, T N, size_t MinDigits,
                                IntegerStyle Style, તાર્કિક IsNegative,
                                તાર્કિક NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  અક્ષર સંખ્યા_Buffer[128];
  size_t Len = format_to_buffer(N, સંખ્યા_Buffer);

  જો (IsNegative)
    S << '-';
  નહીંતર જો (NonNegativePlus)
    S << '+';

  જો (Len < MinDigits && Style != IntegerStyle::સંખ્યા) {
    માટે (size_t I = Len; I < MinDigits; ++I)
      S << '0';
  }

  જો (Style == IntegerStyle::સંખ્યા) {
    writeWithCommas(S, ArrayRef<અક્ષર>(std::end(સંખ્યા_Buffer) - Len, Len));
  } નહીંતર {
    S.લિખ(std::end(સંખ્યા_Buffer) - Len, Len);
  }
}

template <typename T>
સ્થિર શૂન્ય write_unsigned(raw_ostream &S, T N, size_t MinDigits,
                           IntegerStyle Style, તાર્કિક IsNegative = ખોટું,
                           તાર્કિક NonNegativePlus = ખોટું) {
  // Output using 32-bit div/mod if possible.
  જો (N == static_cast<uint32_t>(N))
    write_unsigned_impl(S, static_cast<uint32_t>(N), MinDigits, Style,
                        IsNegative, NonNegativePlus);
  નહીંતર
    write_unsigned_impl(S, N, MinDigits, Style, IsNegative, NonNegativePlus);
}

template <typename T>
સ્થિર શૂન્ય write_signed(raw_ostream &S, T N, size_t MinDigits,
                         IntegerStyle Style, તાર્કિક NonNegativePlus = ખોટું) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  using UnsignedT = std::make_unsigned_t<T>;

  જો (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, ખોટું,
                   NonNegativePlus);
    પાછા;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, સાચું, NonNegativePlus);
}

શૂન્ય llvm::write_integer(raw_ostream &S, unsigned પૂર્ણાંક N, size_t MinDigits,
                         IntegerStyle Style, તાર્કિક NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, ખોટું, NonNegativePlus);
}

શૂન્ય llvm::write_integer(raw_ostream &S, પૂર્ણાંક N, size_t MinDigits,
                         IntegerStyle Style, તાર્કિક NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

શૂન્ય llvm::write_integer(raw_ostream &S, unsigned long N, size_t MinDigits,
                         IntegerStyle Style, તાર્કિક NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, ખોટું, NonNegativePlus);
}

શૂન્ય llvm::write_integer(raw_ostream &S, long N, size_t MinDigits,
                         IntegerStyle Style, તાર્કિક NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

શૂન્ય llvm::write_integer(raw_ostream &S, unsigned long long N, size_t MinDigits,
                         IntegerStyle Style, તાર્કિક NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, ખોટું, NonNegativePlus);
}

શૂન્ય llvm::write_integer(raw_ostream &S, long long N, size_t MinDigits,
                         IntegerStyle Style, તાર્કિક NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

શૂન્ય llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<size_t> Width) {
  અચલ size_t kMaxWidth = 128u;

  size_t W = std::min(kMaxWidth, Width.કિંમત_or(0u));

  unsigned Nibbles = (llvm::bit_width(N) + 3) / 4;
  તાર્કિક Prefix = (Style == HexPrintStyle::PrefixLower ||
                 Style == HexPrintStyle::PrefixUpper);
  તાર્કિક Upper =
      (Style == HexPrintStyle::Upper || Style == HexPrintStyle::PrefixUpper);
  unsigned PrefixChars = Prefix ? 2 : 0;
  unsigned અંક_Chars =
      std::max(static_cast<unsigned>(W), std::max(1u, Nibbles) + PrefixChars);

  અક્ષર સંખ્યા_Buffer[kMaxWidth];
  ::memset(સંખ્યા_Buffer, '0', std::size(સંખ્યા_Buffer));
  જો (Prefix)
    સંખ્યા_Buffer[1] = 'x';
  અક્ષર *EndPtr = સંખ્યા_Buffer + અંક_Chars;
  અક્ષર *CurPtr = EndPtr;
  જ્યારે (N) {
    unsigned અક્ષર x = static_cast<unsigned અક્ષર>(N) % 16;
    *--CurPtr = hexdigit(x, !Upper);
    N /= 16;
  }

  S.લિખ(સંખ્યા_Buffer, અંક_Chars);
}

શૂન્ય llvm::write_double(raw_ostream &S, દ્વિગુણ N, FloatStyle Style,
                        std::optional<size_t> Precision) {
  size_t Prec = Precision.કિંમત_or(getDefaultPrecision(Style));

  જો (std::isnan(N)) {
    S << "nan";
    પાછા;
  } નહીંતર જો (std::isinf(N)) {
    S << (std::signbit(N) ? "-INF" : "INF");
    પાછા;
  }

  અક્ષર Letter;
  જો (Style == FloatStyle::Exponent)
    Letter = 'e';
  નહીંતર જો (Style == FloatStyle::ExponentUpper)
    Letter = 'E';
  નહીંતર
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream Out(Spec);
  Out << "%." << Prec << Letter;

  જો (Style == FloatStyle::Exponent || Style == FloatStyle::ExponentUpper) {
#ifdef _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#જો defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    જો (N == 0.0 && std::signbit(N)) {
      અક્ષર NegativeZero[] = "-0.000000e+00";
      જો (Style == FloatStyle::ExponentUpper)
        NegativeZero[strlen(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      પાછા;
    }
#નહીંતર
    પૂર્ણાંક fpcl = _fpclass(N);

    // negative zero
    જો (fpcl == _FPCLASS_NZ) {
      અક્ષર NegativeZero[] = "-0.000000e+00";
      જો (Style == FloatStyle::ExponentUpper)
        NegativeZero[strlen(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      પાછા;
    }
#endif

    અક્ષર buf[32];
    unsigned len;
    len = format(Spec.c_str(), N).snprint(buf, કદ(buf));
    જો (len <= કદ(buf) - 2) {
      જો (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        પૂર્ણાંક cs = buf[len - 4];
        જો (cs == '+' || cs == '-') {
          પૂર્ણાંક c1 = buf[len - 2];
          પૂર્ણાંક c0 = buf[len - 1];
          જો (isdigit(static_cast<unsigned અક્ષર>(c1)) &&
              isdigit(static_cast<unsigned અક્ષર>(c0))) {
            // Trim leading '0': "...e+012" -> "...e+12\0"
            buf[len - 3] = c1;
            buf[len - 2] = c0;
            buf[--len] = 0;
          }
        }
      }
      S << buf;
      પાછા;
    }
#endif
  }

  જો (Style == FloatStyle::Percent)
    N *= 100.0;

  અક્ષર Buf[32];
  format(Spec.c_str(), N).snprint(Buf, કદ(Buf));
  S << Buf;
  જો (Style == FloatStyle::Percent)
    S << '%';
}

તાર્કિક llvm::isPrefixedHexStyle(HexPrintStyle S) {
  પાછા (S == HexPrintStyle::PrefixLower || S == HexPrintStyle::PrefixUpper);
}

size_t llvm::getDefaultPrecision(FloatStyle Style) {
  પસંદગી (Style) {
  સ્થિતિ FloatStyle::Exponent:
  સ્થિતિ FloatStyle::ExponentUpper:
    પાછા 6; // Number of decimal places.
  સ્થિતિ FloatStyle::Fixed:
  સ્થિતિ FloatStyle::Percent:
    પાછા 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
