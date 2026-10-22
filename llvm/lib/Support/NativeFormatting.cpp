//===- NativeFormatting.cpp - Low level formatting helpers -------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/NativeFormatting.h"
#समावेश "llvm/ADT/ArrayRef.h"
#समावेश "llvm/ADT/SmallString.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/Support/Format.h"
#समावेश "llvm/Support/raw_ostream.h"

#समावेश <cmath>

#अगर defined(_WIN32) && !defined(__MINGW32__)
#समावेश <float.h> // For _fpclass in llvm::write_double.
#पूर्ण_अगर

using namespace llvm;

ढाँचा<typename T, std::माप_प्रकार N>
अटल पूर्णांक format_to_बफर(T मूल्य, अक्षर (&बफर)[N]) {
  अक्षर *EndPtr = std::end(बफर);
  अक्षर *CurPtr = EndPtr;

  करो {
    *--CurPtr = '0' + अक्षर(मूल्य % 10);
    मूल्य /= 10;
  } जबतक (मूल्य);
  वापस EndPtr - CurPtr;
}

अटल व्योम writeWithCommas(raw_ostream &S, ArrayRef<अक्षर> बफर) {
  निश्चित(!बफर.empty());

  ArrayRef<अक्षर> ThisGroup;
  पूर्णांक InitialDigits = ((बफर.आकार_2() - 1) % 3) + 1;
  ThisGroup = बफर.take_front(InitialDigits);
  S.ग_लिखो(ThisGroup.तथ्य(), ThisGroup.आकार_2());

  बफर = बफर.drop_front(InitialDigits);
  निश्चित(बफर.आकार_2() % 3 == 0);
  जबतक (!बफर.empty()) {
    S << ',';
    ThisGroup = बफर.take_front(3);
    S.ग_लिखो(ThisGroup.तथ्य(), 3);
    बफर = बफर.drop_front(3);
  }
}

ढाँचा <typename T>
अटल व्योम write_unsigned_impl(raw_ostream &S, T N, माप_प्रकार MinDigits,
                                IntegerStyle Style, bool IsNegative,
                                bool NonNegativePlus) {
  static_assert(std::is_unsigned_v<T>, "Value is not unsigned!");

  अक्षर संख्या_बफर[128];
  माप_प्रकार Len = format_to_बफर(N, संख्या_बफर);

  अगर (IsNegative)
    S << '-';
  अन्यथा अगर (NonNegativePlus)
    S << '+';

  अगर (Len < MinDigits && Style != IntegerStyle::संख्या) {
    क्रम (माप_प्रकार I = Len; I < MinDigits; ++I)
      S << '0';
  }

  अगर (Style == IntegerStyle::संख्या) {
    writeWithCommas(S, ArrayRef<अक्षर>(std::end(संख्या_बफर) - Len, Len));
  } अन्यथा {
    S.ग_लिखो(std::end(संख्या_बफर) - Len, Len);
  }
}

ढाँचा <typename T>
अटल व्योम write_unsigned(raw_ostream &S, T N, माप_प्रकार MinDigits,
                           IntegerStyle Style, bool IsNegative = false,
                           bool NonNegativePlus = false) {
  // Output using 32-bit div/mod if possible.
  अगर (N == static_cast<uint32_t>(N))
    write_unsigned_impl(S, static_cast<uint32_t>(N), MinDigits, Style,
                        IsNegative, NonNegativePlus);
  अन्यथा
    write_unsigned_impl(S, N, MinDigits, Style, IsNegative, NonNegativePlus);
}

ढाँचा <typename T>
अटल व्योम write_signed(raw_ostream &S, T N, माप_प्रकार MinDigits,
                         IntegerStyle Style, bool NonNegativePlus = false) {
  static_assert(std::is_signed_v<T>, "Value is not signed!");

  using UnsignedT = std::make_unsigned_t<T>;

  अगर (N >= 0) {
    write_unsigned(S, static_cast<UnsignedT>(N), MinDigits, Style, false,
                   NonNegativePlus);
    वापस;
  }

  UnsignedT UN = -(UnsignedT)N;
  write_unsigned(S, UN, MinDigits, Style, true, NonNegativePlus);
}

व्योम llvm::write_integer(raw_ostream &S, अचिन्हित पूर्णांक N, माप_प्रकार MinDigits,
                         IntegerStyle Style, bool NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, false, NonNegativePlus);
}

व्योम llvm::write_integer(raw_ostream &S, पूर्णांक N, माप_प्रकार MinDigits,
                         IntegerStyle Style, bool NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

व्योम llvm::write_integer(raw_ostream &S, अचिन्हित दीर्घ N, माप_प्रकार MinDigits,
                         IntegerStyle Style, bool NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, false, NonNegativePlus);
}

व्योम llvm::write_integer(raw_ostream &S, दीर्घ N, माप_प्रकार MinDigits,
                         IntegerStyle Style, bool NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

व्योम llvm::write_integer(raw_ostream &S, अचिन्हित दीर्घ दीर्घ N, माप_प्रकार MinDigits,
                         IntegerStyle Style, bool NonNegativePlus) {
  write_unsigned(S, N, MinDigits, Style, false, NonNegativePlus);
}

व्योम llvm::write_integer(raw_ostream &S, दीर्घ दीर्घ N, माप_प्रकार MinDigits,
                         IntegerStyle Style, bool NonNegativePlus) {
  write_signed(S, N, MinDigits, Style, NonNegativePlus);
}

व्योम llvm::write_hex(raw_ostream &S, uint64_t N, HexPrintStyle Style,
                     std::optional<माप_प्रकार> Width) {
  स्थिर माप_प्रकार kMaxWidth = 128u;

  माप_प्रकार W = std::min(kMaxWidth, Width.मूल्य_or(0u));

  अचिन्हित Nibbles = (llvm::bit_width(N) + 3) / 4;
  bool Prefix = (Style == HexPrintStyle::PrefixLower ||
                 Style == HexPrintStyle::PrefixUpper);
  bool Upper =
      (Style == HexPrintStyle::Upper || Style == HexPrintStyle::PrefixUpper);
  अचिन्हित PrefixChars = Prefix ? 2 : 0;
  अचिन्हित अंक_Chars =
      std::max(static_cast<अचिन्हित>(W), std::max(1u, Nibbles) + PrefixChars);

  अक्षर संख्या_बफर[kMaxWidth];
  ::स_रखो(संख्या_बफर, '0', std::आकार_2(संख्या_बफर));
  अगर (Prefix)
    संख्या_बफर[1] = 'x';
  अक्षर *EndPtr = संख्या_बफर + अंक_Chars;
  अक्षर *CurPtr = EndPtr;
  जबतक (N) {
    अचिन्हित अक्षर x = static_cast<अचिन्हित अक्षर>(N) % 16;
    *--CurPtr = hexdigit(x, !Upper);
    N /= 16;
  }

  S.ग_लिखो(संख्या_बफर, अंक_Chars);
}

व्योम llvm::write_double(raw_ostream &S, द्विगुन N, FloatStyle Style,
                        std::optional<माप_प्रकार> Precision) {
  माप_प्रकार Prec = Precision.मूल्य_or(getDefaultPrecision(Style));

  अगर (std::isnan(N)) {
    S << "nan";
    वापस;
  } अन्यथा अगर (std::isinf(N)) {
    S << (std::signbit(N) ? "-INF" : "INF");
    वापस;
  }

  अक्षर Letter;
  अगर (Style == FloatStyle::Exponent)
    Letter = 'e';
  अन्यथा अगर (Style == FloatStyle::ExponentUpper)
    Letter = 'E';
  अन्यथा
    Letter = 'f';

  SmallString<8> Spec;
  llvm::raw_svector_ostream Out(Spec);
  Out << "%." << Prec << Letter;

  अगर (Style == FloatStyle::Exponent || Style == FloatStyle::ExponentUpper) {
#अगर_घोषित _WIN32
// On MSVCRT and compatible, output of %e is incompatible to Posix
// by default. Number of exponent digits should be at least 2. "%+03d"
// FIXME: Implement our formatter to here or Support/Format.h!
#अगर defined(__MINGW32__)
    // FIXME: It should be generic to C++11.
    अगर (N == 0.0 && std::signbit(N)) {
      अक्षर NegativeZero[] = "-0.000000e+00";
      अगर (Style == FloatStyle::ExponentUpper)
        NegativeZero[म_माप(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      वापस;
    }
#अन्यथा
    पूर्णांक fpcl = _fpclass(N);

    // negative zero
    अगर (fpcl == _FPCLASS_NZ) {
      अक्षर NegativeZero[] = "-0.000000e+00";
      अगर (Style == FloatStyle::ExponentUpper)
        NegativeZero[म_माप(NegativeZero) - 4] = 'E';
      S << NegativeZero;
      वापस;
    }
#पूर्ण_अगर

    अक्षर buf[32];
    अचिन्हित len;
    len = format(Spec.c_str(), N).snprint(buf, माप(buf));
    अगर (len <= माप(buf) - 2) {
      अगर (len >= 5 && (buf[len - 5] == 'e' || buf[len - 5] == 'E') &&
          buf[len - 3] == '0') {
        पूर्णांक cs = buf[len - 4];
        अगर (cs == '+' || cs == '-') {
          पूर्णांक c1 = buf[len - 2];
          पूर्णांक c0 = buf[len - 1];
          अगर (है_अंक(static_cast<अचिन्हित अक्षर>(c1)) &&
              है_अंक(static_cast<अचिन्हित अक्षर>(c0))) {
            // Trim leading '0': "...e+012" -> "...e+12\0"
            buf[len - 3] = c1;
            buf[len - 2] = c0;
            buf[--len] = 0;
          }
        }
      }
      S << buf;
      वापस;
    }
#पूर्ण_अगर
  }

  अगर (Style == FloatStyle::Percent)
    N *= 100.0;

  अक्षर Buf[32];
  format(Spec.c_str(), N).snprint(Buf, माप(Buf));
  S << Buf;
  अगर (Style == FloatStyle::Percent)
    S << '%';
}

bool llvm::isPrefixedHexStyle(HexPrintStyle S) {
  वापस (S == HexPrintStyle::PrefixLower || S == HexPrintStyle::PrefixUpper);
}

माप_प्रकार llvm::getDefaultPrecision(FloatStyle Style) {
  चयन (Style) {
  हाल FloatStyle::Exponent:
  हाल FloatStyle::ExponentUpper:
    वापस 6; // Number of decimal places.
  हाल FloatStyle::Fixed:
  हाल FloatStyle::Percent:
    वापस 2; // Number of decimal places.
  }
  llvm_unreachable("Unknown FloatStyle enum");
}
