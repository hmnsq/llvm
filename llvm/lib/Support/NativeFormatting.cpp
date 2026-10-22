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

#यदि defined(_WIN32) && !defined(__MINGW32__)
#अन्तर्भाव <float.h> // For _fpclass in llvm::write_double.
#endif

using नामाकाश llvm;

ढाँचा<typename T, std::माप_प्रकार N>
स्थिर पूर्णाङ्क format_to_buffer(T मान, वर्ण (&Buffer)[N]) {
  वर्ण *EndPtr = std::end(Buffer);
  वर्ण *CurPtr = EndPtr;

  गर {
    *--CurPtr = '0' + वर्ण(मान % 10);
    मान /= 10;
  } जबसम्म (मान);
  फर्काऊ EndPtr - CurPtr;
}

स्थिर शून्य writeWithCommas(raw_ostream &S, ArrayRef<वर्ण> Buffer) {
  प्रतिज्ञा(!Buffer.empty());

  ArrayRef<वर्ण> ThisGroup;
  पूर्णाङ्क InitialDigits = ((Buffer.size() - 1) % 3) + 1;
  ThisGroup = Buffer.take_front(InitialDigits);
  S.लिख(ThisGroup.data(), ThisGroup.size());

  Buffer = Buffer.drop_front(InitialDigits);
  प्रतिज्ञा(Buffer.size() % 3 == 0);
  जबसम्म (!Buffer.empty()) {
    S << ',';
    ThisGroup = Buffer.take_front(3);
    S.लिख(ThisGroup.data(), 3);
    Buffer = Buffer.drop_front(3);
  }
}

ढाँचा <typename T>
स्थिर शून्य write_unsigned_impl(raw_ostream &S, T N, माप_प्रकार MinDigits,
                                IntegerStyle Style, तार्किक IsNegative,
                                तार्किक NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  वर्ण सङ्ख्या_Buffer[128];
  माप_प्रकार Len = format_to_buffer(N, सङ्ख्या_Buffer);

  यदि (IsNegative)
    S << '-';
  अन्यथा यदि (NonNegativePlus)
    S << '+';

  यदि (Len < MinDigits && Style != IntegerStyle::सङ्ख्या) {
    लागि (माप_प्रकार I = Len; I < MinDigits; ++I)
      S << '0';
  }

  यदि (Style == IntegerStyle::सङ्ख्या) {
    writeWithCommas(S, ArrayRef<वर्ण>(std::end(सङ्ख्या_Buffer) - Len, Len));
  } अन्यथा {
    S.लिख(std::end(सङ्ख्या_Buffer) - Len, Len);
  }
}

ढाँचा <typename T>
स्थिर शून्य write_unsigned(raw_ostream &S, T N, माप_प्रकार MinDigits,
                           IntegerStyle Style, तार्किक IsNegative = असत्य,
                           तार्किक NonNegativePlus = असत्य) {
  // Output using 32-bit div/mod if possible.
  यदि (N == static_cast<uint32_t>(N))
    write_unsigned_impl(S, static_cast<uint32_t>(N), MinDigits, Style,
                        IsNegative, NonNegativePlus);
  अन्यथा
    write_unsigned_impl(S, N, MinDigits, Style, IsNegative, NonNegativePlus);
}

ढाँचा <typename T>
स्थिर शून्य write_signed(raw_ostream &S, T N, माप_प्रकार MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus = असत्य) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  using UnsignedT = std::make_unsigned_t<T>;

  यदि (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, असत्य,
                   NonNegativePlus);
    फर्काऊ;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, सत्य, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, अचिन्हित पूर्णाङ्क N, माप_प्रकार MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, असत्य, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, पूर्णाङ्क N, माप_प्रकार MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, अचिन्हित दीर्घ N, माप_प्रकार MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, असत्य, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, दीर्घ N, माप_प्रकार MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, अचिन्हित दीर्घ दीर्घ N, माप_प्रकार MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, असत्य, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, दीर्घ दीर्घ N, माप_प्रकार MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

शून्य llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<माप_प्रकार> Width) {
  अचर माप_प्रकार kMaxWidth = 128u;

  माप_प्रकार W = std::min(kMaxWidth, Width.मान_or(0u));

  अचिन्हित Nibbles = (llvm::bit_width(N) + 3) / 4;
  तार्किक Prefix = (Style == HexPrintStyle::PrefixLower ||
                 Style == HexPrintStyle::PrefixUpper);
  तार्किक Upper =
      (Style == HexPrintStyle::Upper || Style == HexPrintStyle::PrefixUpper);
  अचिन्हित PrefixChars = Prefix ? 2 : 0;
  अचिन्हित अङ्क_Chars =
      std::max(static_cast<अचिन्हित>(W), std::max(1u, Nibbles) + PrefixChars);

  वर्ण सङ्ख्या_Buffer[kMaxWidth];
  ::स_रखो(सङ्ख्या_Buffer, '0', std::size(सङ्ख्या_Buffer));
  यदि (Prefix)
    सङ्ख्या_Buffer[1] = 'x';
  वर्ण *EndPtr = सङ्ख्या_Buffer + अङ्क_Chars;
  वर्ण *CurPtr = EndPtr;
  जबसम्म (N) {
    अचिन्हित वर्ण x = static_cast<अचिन्हित वर्ण>(N) % 16;
    *--CurPtr = hexdigit(x, !Upper);
    N /= 16;
  }

  S.लिख(सङ्ख्या_Buffer, अङ्क_Chars);
}

शून्य llvm::write_double(raw_ostream &S, द्विगुण N, FloatStyle Style,
                        std::optional<माप_प्रकार> Precision) {
  माप_प्रकार Prec = Precision.मान_or(getDefaultPrecision(Style));

  यदि (std::isnan(N)) {
    S << "nan";
    फर्काऊ;
  } अन्यथा यदि (std::isinf(N)) {
    S << (std::signbit(N) ? "-INF" : "INF");
    फर्काऊ;
  }

  वर्ण Letter;
  यदि (Style == FloatStyle::Exponent)
    Letter = 'e';
  अन्यथा यदि (Style == FloatStyle::ExponentUpper)
    Letter = 'E';
  अन्यथा
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream Out(Spec);
  Out << "%." << Prec << Letter;

  यदि (Style == FloatStyle::Exponent || Style == FloatStyle::ExponentUpper) {
#ifdef _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#यदि defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    यदि (N == 0.0 && std::signbit(N)) {
      वर्ण NegativeZero[] = "-0.000000e+00";
      यदि (Style == FloatStyle::ExponentUpper)
        NegativeZero[म_माप(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      फर्काऊ;
    }
#अन्यथा
    पूर्णाङ्क fpcl = _fpclass(N);

    // negative zero
    यदि (fpcl == _FPCLASS_NZ) {
      वर्ण NegativeZero[] = "-0.000000e+00";
      यदि (Style == FloatStyle::ExponentUpper)
        NegativeZero[म_माप(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      फर्काऊ;
    }
#endif

    वर्ण buf[32];
    अचिन्हित len;
    len = format(Spec.c_str(), N).snprint(buf, परिमाण(buf));
    यदि (len <= परिमाण(buf) - 2) {
      यदि (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        पूर्णाङ्क cs = buf[len - 4];
        यदि (cs == '+' || cs == '-') {
          पूर्णाङ्क c1 = buf[len - 2];
          पूर्णाङ्क c0 = buf[len - 1];
          यदि (है_अंक(static_cast<अचिन्हित वर्ण>(c1)) &&
              है_अंक(static_cast<अचिन्हित वर्ण>(c0))) {
            // Trim leading '0': "...e+012" -> "...e+12\0"
            buf[len - 3] = c1;
            buf[len - 2] = c0;
            buf[--len] = 0;
          }
        }
      }
      S << buf;
      फर्काऊ;
    }
#endif
  }

  यदि (Style == FloatStyle::Percent)
    N *= 100.0;

  वर्ण Buf[32];
  format(Spec.c_str(), N).snprint(Buf, परिमाण(Buf));
  S << Buf;
  यदि (Style == FloatStyle::Percent)
    S << '%';
}

तार्किक llvm::isPrefixedHexStyle(HexPrintStyle S) {
  फर्काऊ (S == HexPrintStyle::PrefixLower || S == HexPrintStyle::PrefixUpper);
}

माप_प्रकार llvm::getDefaultPrecision(FloatStyle Style) {
  विकल्प (Style) {
  पक्ष FloatStyle::Exponent:
  पक्ष FloatStyle::ExponentUpper:
    फर्काऊ 6; // Number of decimal places.
  पक्ष FloatStyle::Fixed:
  पक्ष FloatStyle::Percent:
    फर्काऊ 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
