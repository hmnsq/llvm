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

#जर defined(_WIN32) && !defined(__MINGW32__)
#अन्तर्भाव <float.h> // For _fpclass in llvm::write_double.
#endif

using नामाकाश llvm;

ढाँचा<typename T, std::माप_प्रकार N>
स्थिर पूर्णांक format_to_buffer(T मूल्य, अक्षर (&Buffer)[N]) {
  अक्षर *EndPtr = std::end(Buffer);
  अक्षर *CurPtr = EndPtr;

  करा {
    *--CurPtr = '0' + अक्षर(मूल्य % 10);
    मूल्य /= 10;
  } पर्यंत (मूल्य);
  परत EndPtr - CurPtr;
}

स्थिर शून्य writeWithCommas(raw_ostream &S, ArrayRef<अक्षर> Buffer) {
  प्रतिज्ञा(!Buffer.empty());

  ArrayRef<अक्षर> ThisGroup;
  पूर्णांक InitialDigits = ((Buffer.size() - 1) % 3) + 1;
  ThisGroup = Buffer.take_front(InitialDigits);
  S.लिख(ThisGroup.data(), ThisGroup.size());

  Buffer = Buffer.drop_front(InitialDigits);
  प्रतिज्ञा(Buffer.size() % 3 == 0);
  पर्यंत (!Buffer.empty()) {
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

  अक्षर संख्या_Buffer[128];
  माप_प्रकार Len = format_to_buffer(N, संख्या_Buffer);

  जर (IsNegative)
    S << '-';
  अन्यथा जर (NonNegativePlus)
    S << '+';

  जर (Len < MinDigits && Style != IntegerStyle::संख्या) {
    साठी (माप_प्रकार I = Len; I < MinDigits; ++I)
      S << '0';
  }

  जर (Style == IntegerStyle::संख्या) {
    writeWithCommas(S, ArrayRef<अक्षर>(std::end(संख्या_Buffer) - Len, Len));
  } अन्यथा {
    S.लिख(std::end(संख्या_Buffer) - Len, Len);
  }
}

ढाँचा <typename T>
स्थिर शून्य write_unsigned(raw_ostream &S, T N, माप_प्रकार MinDigits,
                           IntegerStyle Style, तार्किक IsNegative = असत्य,
                           तार्किक NonNegativePlus = असत्य) {
  // Output using 32-bit div/mod if possible.
  जर (N == static_cast<uint32_t>(N))
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

  जर (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, असत्य,
                   NonNegativePlus);
    परत;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, सत्य, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, अचिन्हित पूर्णांक N, माप_प्रकार MinDigits,
                         IntegerStyle Style, तार्किक NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, असत्य, NonNegativePlus);
}

शून्य llvm::write_integer(raw_ostream &S, पूर्णांक N, माप_प्रकार MinDigits,
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
  स्थिरांक माप_प्रकार kMaxWidth = 128u;

  माप_प्रकार W = std::min(kMaxWidth, Width.मूल्य_or(0u));

  अचिन्हित Nibbles = (llvm::bit_width(N) + 3) / 4;
  तार्किक Prefix = (Style == HexPrintStyle::PrefixLower ||
                 Style == HexPrintStyle::PrefixUpper);
  तार्किक Upper =
      (Style == HexPrintStyle::Upper || Style == HexPrintStyle::PrefixUpper);
  अचिन्हित PrefixChars = Prefix ? 2 : 0;
  अचिन्हित अंक_Chars =
      std::max(static_cast<अचिन्हित>(W), std::max(1u, Nibbles) + PrefixChars);

  अक्षर संख्या_Buffer[kMaxWidth];
  ::स_रखो(संख्या_Buffer, '0', std::size(संख्या_Buffer));
  जर (Prefix)
    संख्या_Buffer[1] = 'x';
  अक्षर *EndPtr = संख्या_Buffer + अंक_Chars;
  अक्षर *CurPtr = EndPtr;
  पर्यंत (N) {
    अचिन्हित अक्षर x = static_cast<अचिन्हित अक्षर>(N) % 16;
    *--CurPtr = hexdigit(x, !Upper);
    N /= 16;
  }

  S.लिख(संख्या_Buffer, अंक_Chars);
}

शून्य llvm::write_double(raw_ostream &S, द्विगुण N, FloatStyle Style,
                        std::optional<माप_प्रकार> Precision) {
  माप_प्रकार Prec = Precision.मूल्य_or(getDefaultPrecision(Style));

  जर (std::isnan(N)) {
    S << "nan";
    परत;
  } अन्यथा जर (std::isinf(N)) {
    S << (std::signbit(N) ? "-INF" : "INF");
    परत;
  }

  अक्षर Letter;
  जर (Style == FloatStyle::Exponent)
    Letter = 'e';
  अन्यथा जर (Style == FloatStyle::ExponentUpper)
    Letter = 'E';
  अन्यथा
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream Out(Spec);
  Out << "%." << Prec << Letter;

  जर (Style == FloatStyle::Exponent || Style == FloatStyle::ExponentUpper) {
#ifdef _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#जर defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    जर (N == 0.0 && std::signbit(N)) {
      अक्षर NegativeZero[] = "-0.000000e+00";
      जर (Style == FloatStyle::ExponentUpper)
        NegativeZero[म_माप(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      परत;
    }
#अन्यथा
    पूर्णांक fpcl = _fpclass(N);

    // negative zero
    जर (fpcl == _FPCLASS_NZ) {
      अक्षर NegativeZero[] = "-0.000000e+00";
      जर (Style == FloatStyle::ExponentUpper)
        NegativeZero[म_माप(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      परत;
    }
#endif

    अक्षर buf[32];
    अचिन्हित len;
    len = format(Spec.c_str(), N).snprint(buf, आकार(buf));
    जर (len <= आकार(buf) - 2) {
      जर (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        पूर्णांक cs = buf[len - 4];
        जर (cs == '+' || cs == '-') {
          पूर्णांक c1 = buf[len - 2];
          पूर्णांक c0 = buf[len - 1];
          जर (है_अंक(static_cast<अचिन्हित अक्षर>(c1)) &&
              है_अंक(static_cast<अचिन्हित अक्षर>(c0))) {
            // Trim leading '0': "...e+012" -> "...e+12\0"
            buf[len - 3] = c1;
            buf[len - 2] = c0;
            buf[--len] = 0;
          }
        }
      }
      S << buf;
      परत;
    }
#endif
  }

  जर (Style == FloatStyle::Percent)
    N *= 100.0;

  अक्षर Buf[32];
  format(Spec.c_str(), N).snprint(Buf, आकार(Buf));
  S << Buf;
  जर (Style == FloatStyle::Percent)
    S << '%';
}

तार्किक llvm::isPrefixedHexStyle(HexPrintStyle S) {
  परत (S == HexPrintStyle::PrefixLower || S == HexPrintStyle::PrefixUpper);
}

माप_प्रकार llvm::getDefaultPrecision(FloatStyle Style) {
  निवड (Style) {
  स्थिती FloatStyle::Exponent:
  स्थिती FloatStyle::ExponentUpper:
    परत 6; // Number of decimal places.
  स्थिती FloatStyle::Fixed:
  स्थिती FloatStyle::Percent:
    परत 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
