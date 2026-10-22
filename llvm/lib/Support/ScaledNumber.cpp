//==- lib/Support/ScaledNumber.cpp - Support for scaled numbers -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implementation of some scaled number algorithms.
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/ScaledNumber.h"
#समावेश "llvm/ADT/APFloat.h"
#समावेश "llvm/ADT/ArrayRef.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::ScaledNumbers;

std::pair<uint64_t, int16_t> ScaledNumbers::multiply64(uint64_t LHS,
                                                       uint64_t RHS) {
  uint64_t Upper, Lower;
#अगर defined(__SIZEOF_INT128__) ||                                              \
    (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)
  स्वतः Product = __uint128_t(LHS) * RHS;
  Upper = uint64_t(Product >> 64);
  Lower = uint64_t(Product);
#अन्यथा
  // Separate into two 32-bit digits (U.L).
  स्वतः getU = [](uint64_t N) { वापस N >> 32; };
  स्वतः getL = [](uint64_t N) { वापस N & UINT32_MAX; };
  uint64_t UL = getU(LHS), LL = getL(LHS), UR = getU(RHS), LR = getL(RHS);

  // Compute cross products.
  uint64_t P1 = UL * UR, P2 = UL * LR, P3 = LL * UR, P4 = LL * LR;

  // Sum into two 64-bit digits.
  Upper = P1;
  Lower = P4;
  स्वतः addWithCarry = [&](uint64_t N) {
    uint64_t NewLower = Lower + (getL(N) << 32);
    Upper += getU(N) + (NewLower < Lower);
    Lower = NewLower;
  };
  addWithCarry(P2);
  addWithCarry(P3);
#पूर्ण_अगर

  // Check whether the upper digit is empty.
  अगर (!Upper)
    वापस {Lower, 0};

  // Shift as little as possible to maximize precision.
  अचिन्हित LeadingZeros = llvm::countl_zero(Upper);
  पूर्णांक Shift = 64 - LeadingZeros;
  अगर (LeadingZeros)
    Upper = Upper << LeadingZeros | Lower >> Shift;
  वापस getRounded(Upper, Shift,
                    Shift && (Lower & UINT64_C(1) << (Shift - 1)));
}

अटल uint64_t getHalf(uint64_t N) { वापस (N >> 1) + (N & 1); }

std::pair<uint32_t, int16_t> ScaledNumbers::divide32(uint32_t Dividend,
                                                     uint32_t भाजक_2) {
  निश्चित(Dividend && "expected non-zero dividend");
  निश्चित(भाजक_2 && "expected non-zero divisor");

  // Use 64-bit math and canonicalize the dividend to gain precision.
  uint64_t Dividend64 = Dividend;
  पूर्णांक Shift = 0;
  अगर (पूर्णांक Zeros = llvm::countl_zero(Dividend64)) {
    Shift -= Zeros;
    Dividend64 <<= Zeros;
  }
  uint64_t Quotient = Dividend64 / भाजक_2;
  uint64_t Remainder = Dividend64 % भाजक_2;

  // If Quotient needs to be shifted, leave the rounding to getAdjusted().
  अगर (Quotient > UINT32_MAX)
    वापस getAdjusted<uint32_t>(Quotient, Shift);

  // Round based on the value of the next bit.
  वापस getRounded<uint32_t>(Quotient, Shift, Remainder >= getHalf(भाजक_2));
}

std::pair<uint64_t, int16_t> ScaledNumbers::divide64(uint64_t Dividend,
                                                     uint64_t भाजक_2) {
  निश्चित(Dividend && "expected non-zero dividend");
  निश्चित(भाजक_2 && "expected non-zero divisor");

  // Minimize size of divisor.
  पूर्णांक Shift = 0;
  अगर (पूर्णांक Zeros = llvm::countr_zero(भाजक_2)) {
    Shift -= Zeros;
    भाजक_2 >>= Zeros;
  }

  // Check for powers of two.
  अगर (भाजक_2 == 1)
    वापस {Dividend, Shift};

  // Maximize size of dividend.
  अगर (पूर्णांक Zeros = llvm::countl_zero(Dividend)) {
    Shift -= Zeros;
    Dividend <<= Zeros;
  }

  // Start with the result of a divide.
  uint64_t Quotient = Dividend / भाजक_2;
  Dividend %= भाजक_2;

  // Continue building the quotient with long division.
  जबतक (!(Quotient >> 63) && Dividend) {
    // Shift Dividend and check for overflow.
    bool IsOverflow = Dividend >> 63;
    Dividend <<= 1;
    --Shift;

    // Get the next bit of Quotient.
    Quotient <<= 1;
    अगर (IsOverflow || भाजक_2 <= Dividend) {
      Quotient |= 1;
      Dividend -= भाजक_2;
    }
  }

  वापस getRounded(Quotient, Shift, Dividend >= getHalf(भाजक_2));
}

पूर्णांक ScaledNumbers::compareImpl(uint64_t L, uint64_t R, पूर्णांक ScaleDiff) {
  निश्चित(ScaleDiff >= 0 && "wrong argument order");
  निश्चित(ScaleDiff < 64 && "numbers too far apart");

  uint64_t L_adjusted = L >> ScaleDiff;
  अगर (L_adjusted < R)
    वापस -1;
  अगर (L_adjusted > R)
    वापस 1;

  वापस L > L_adjusted << ScaleDiff ? 1 : 0;
}

अटल व्योम appendDigit(std::string &Str, अचिन्हित D) {
  निश्चित(D < 10);
  Str += '0' + D % 10;
}

अटल व्योम append_संख्या(std::string &Str, uint64_t N) {
  जबतक (N) {
    appendDigit(Str, N % 10);
    N /= 10;
  }
}

अटल bool doesRoundUp(अक्षर Digit) {
  चयन (Digit) {
  हाल '5':
  हाल '6':
  हाल '7':
  हाल '8':
  हाल '9':
    वापस true;
  शेष:
    वापस false;
  }
}

अटल std::string toStringAPFloat(uint64_t D, पूर्णांक E, अचिन्हित Precision) {
  निश्चित(E >= ScaledNumbers::MinScale);
  निश्चित(E <= ScaledNumbers::MaxScale);

  // Find a new E, but don't let it increase past MaxScale.
  पूर्णांक LeadingZeros = Scaled_संख्या_Base::गिनती_Leading_Zeros64(D);
  पूर्णांक NewE = std::min(ScaledNumbers::MaxScale, E + 63 - LeadingZeros);
  पूर्णांक Shift = 63 - (NewE - E);
  निश्चित(Shift <= LeadingZeros);
  निश्चित(Shift == LeadingZeros || NewE == ScaledNumbers::MaxScale);
  निश्चित(Shift >= 0 && Shift < 64 && "undefined behavior");
  D <<= Shift;
  E = NewE;

  // Check for a denormal.
  अचिन्हित AdjustedE = E + 16383;
  अगर (!(D >> 63)) {
    निश्चित(E == ScaledNumbers::MaxScale);
    AdjustedE = 0;
  }

  // Build the float and print it.
  uint64_t RawBits[2] = {D, AdjustedE};
  APFloat Float(APFloat::x87DoubleExtended(), APInt(80, RawBits));
  SmallVector<अक्षर, 24> Chars;
  Float.toString(Chars, Precision, 0);
  वापस std::string(Chars.begin(), Chars.end());
}

अटल std::string stripTrailingZeros(स्थिर std::string &Float) {
  माप_प्रकार NonZero = Float.find_last_not_of('0');
  निश्चित(NonZero != std::string::npos && "no . in floating point string");

  अगर (Float[NonZero] == '.')
    ++NonZero;

  वापस Float.substr(0, NonZero + 1);
}

std::string Scaled_संख्या_Base::toString(uint64_t D, int16_t E, पूर्णांक Width,
                                       अचिन्हित Precision) {
  अगर (!D)
    वापस "0.0";

  // Canonicalize exponent and digits.
  uint64_t Above0 = 0;
  uint64_t Below0 = 0;
  uint64_t Extra = 0;
  पूर्णांक ExtraShift = 0;
  अगर (E == 0) {
    Above0 = D;
  } अन्यथा अगर (E > 0) {
    अगर (पूर्णांक Shift = std::min(int16_t(गिनती_Leading_Zeros64(D)), E)) {
      D <<= Shift;
      E -= Shift;

      अगर (!E)
        Above0 = D;
    }
  } अन्यथा अगर (E > -64) {
    Above0 = D >> -E;
    Below0 = D << (64 + E);
  } अन्यथा अगर (E == -64) {
    // Special case: shift by 64 bits is undefined behavior.
    Below0 = D;
  } अन्यथा अगर (E > -120) {
    Below0 = D >> (-E - 64);
    Extra = D << (128 + E);
    ExtraShift = -64 - E;
  }

  // Fall back on APFloat for very small and very large numbers.
  अगर (!Above0 && !Below0)
    वापस toStringAPFloat(D, E, Precision);

  // Append the digits before the decimal.
  std::string Str;
  माप_प्रकार DigitsOut = 0;
  अगर (Above0) {
    append_संख्या(Str, Above0);
    DigitsOut = Str.आकार_2();
  } अन्यथा {
    appendDigit(Str, 0);
  }
  std::reverse(Str.begin(), Str.end());

  // Return early if there's nothing after the decimal.
  अगर (!Below0)
    वापस Str + ".0";

  // Append the decimal and beyond.
  Str += '.';
  uint64_t Error = UINT64_C(1) << (64 - Width);

  // We need to shift Below0 to the right to make space for calculating
  // digits.  Save the precision we're losing in Extra.
  Extra = (Below0 & 0xf) << 56 | (Extra >> 8);
  Below0 >>= 4;
  माप_प्रकार SinceDot = 0;
  माप_प्रकार AfterDot = Str.आकार_2();
  करो {
    अगर (ExtraShift) {
      --ExtraShift;
      Error *= 5;
    } अन्यथा {
      Error *= 10;
    }

    Below0 *= 10;
    Extra *= 10;
    Below0 += (Extra >> 60);
    Extra = Extra & (UINT64_MAX >> 4);
    appendDigit(Str, Below0 >> 60);
    Below0 = Below0 & (UINT64_MAX >> 4);
    अगर (DigitsOut || Str.back() != '0')
      ++DigitsOut;
    ++SinceDot;
  } जबतक (Error && (Below0 << 4 | Extra >> 60) >= Error / 2 &&
           (!Precision || DigitsOut <= Precision || SinceDot < 2));

  // Return early for maximum precision.
  अगर (!Precision || DigitsOut <= Precision)
    वापस stripTrailingZeros(Str);

  // Find where to truncate.
  माप_प्रकार Truncate =
      std::max(Str.आकार_2() - (DigitsOut - Precision), AfterDot + 1);

  // Check if there's anything to truncate.
  अगर (Truncate >= Str.आकार_2())
    वापस stripTrailingZeros(Str);

  bool Carry = doesRoundUp(Str[Truncate]);
  अगर (!Carry)
    वापस stripTrailingZeros(Str.substr(0, Truncate));

  // Round with the first truncated digit.
  क्रम (std::string::reverse_iterator I(Str.begin() + Truncate), E = Str.rend();
       I != E; ++I) {
    अगर (*I == '.')
      जारी;
    अगर (*I == '9') {
      *I = '0';
      जारी;
    }

    ++*I;
    Carry = false;
    अवरोध;
  }

  // Add "1" in front if we still need to carry.
  वापस stripTrailingZeros(std::string(Carry, '1') + Str.substr(0, Truncate));
}

raw_ostream &Scaled_संख्या_Base::print(raw_ostream &OS, uint64_t D, int16_t E,
                                     पूर्णांक Width, अचिन्हित Precision) {
  वापस OS << toString(D, E, Width, Precision);
}

व्योम Scaled_संख्या_Base::dump(uint64_t D, int16_t E, पूर्णांक Width) {
  print(dbgs(), D, E, Width, 0) << "[" << Width << ":" << D << "*2^" << E
                                << "]";
}
