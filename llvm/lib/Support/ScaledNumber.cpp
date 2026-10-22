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

#अन्तर्भाव "llvm/Support/ScaledNumber.h"
#अन्तर्भाव "llvm/ADT/APFloat.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

using नामाकाश llvm;
using नामाकाश llvm::ScaledNumbers;

std::pair<uint64_t, int16_t> ScaledNumbers::multiply64(uint64_t LHS,
                                                       uint64_t RHS) {
  uint64_t Upper, Lower;
#जर defined(__SIZEOF_INT128__) ||                                              \
    (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)
  स्वतः Product = __uint128_t(LHS) * RHS;
  Upper = uint64_t(Product >> 64);
  Lower = uint64_t(Product);
#अन्यथा
  // Separate into two 32-bit digits (U.L).
  स्वतः getU = [](uint64_t N) { परत N >> 32; };
  स्वतः getL = [](uint64_t N) { परत N & UINT32_MAX; };
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
#endif

  // Check whether the upper digit is empty.
  जर (!Upper)
    परत {Lower, 0};

  // Shift as little as possible to maximize precision.
  अचिन्हित LeadingZeros = llvm::countl_zero(Upper);
  पूर्णांक Shift = 64 - LeadingZeros;
  जर (LeadingZeros)
    Upper = Upper << LeadingZeros | Lower >> Shift;
  परत getRounded(Upper, Shift,
                    Shift && (Lower & UINT64_C(1) << (Shift - 1)));
}

स्थिर uint64_t getHalf(uint64_t N) { परत (N >> 1) + (N & 1); }

std::pair<uint32_t, int16_t> ScaledNumbers::divide32(uint32_t Dividend,
                                                     uint32_t भाजक_2) {
  प्रतिज्ञा(Dividend && "expected non-zero dividend");
  प्रतिज्ञा(भाजक_2 && "expected non-zero divisor");

  // Use 64-bit math and canonicalize the dividend to gain precision.
  uint64_t Dividend64 = Dividend;
  पूर्णांक Shift = 0;
  जर (पूर्णांक Zeros = llvm::countl_zero(Dividend64)) {
    Shift -= Zeros;
    Dividend64 <<= Zeros;
  }
  uint64_t Quotient = Dividend64 / भाजक_2;
  uint64_t Remainder = Dividend64 % भाजक_2;

  // If Quotient needs to be shifted, leave the rounding to getAdjusted().
  जर (Quotient > UINT32_MAX)
    परत getAdjusted<uint32_t>(Quotient, Shift);

  // Round based on the value of the next bit.
  परत getRounded<uint32_t>(Quotient, Shift, Remainder >= getHalf(भाजक_2));
}

std::pair<uint64_t, int16_t> ScaledNumbers::divide64(uint64_t Dividend,
                                                     uint64_t भाजक_2) {
  प्रतिज्ञा(Dividend && "expected non-zero dividend");
  प्रतिज्ञा(भाजक_2 && "expected non-zero divisor");

  // Minimize size of divisor.
  पूर्णांक Shift = 0;
  जर (पूर्णांक Zeros = llvm::countr_zero(भाजक_2)) {
    Shift -= Zeros;
    भाजक_2 >>= Zeros;
  }

  // Check for powers of two.
  जर (भाजक_2 == 1)
    परत {Dividend, Shift};

  // Maximize size of dividend.
  जर (पूर्णांक Zeros = llvm::countl_zero(Dividend)) {
    Shift -= Zeros;
    Dividend <<= Zeros;
  }

  // Start with the result of a divide.
  uint64_t Quotient = Dividend / भाजक_2;
  Dividend %= भाजक_2;

  // Continue building the quotient with long division.
  पर्यंत (!(Quotient >> 63) && Dividend) {
    // Shift Dividend and check for overflow.
    तार्किक IsOverflow = Dividend >> 63;
    Dividend <<= 1;
    --Shift;

    // Get the next bit of Quotient.
    Quotient <<= 1;
    जर (IsOverflow || भाजक_2 <= Dividend) {
      Quotient |= 1;
      Dividend -= भाजक_2;
    }
  }

  परत getRounded(Quotient, Shift, Dividend >= getHalf(भाजक_2));
}

पूर्णांक ScaledNumbers::compareImpl(uint64_t L, uint64_t R, पूर्णांक ScaleDiff) {
  प्रतिज्ञा(ScaleDiff >= 0 && "wrong argument order");
  प्रतिज्ञा(ScaleDiff < 64 && "numbers too far apart");

  uint64_t L_adjusted = L >> ScaleDiff;
  जर (L_adjusted < R)
    परत -1;
  जर (L_adjusted > R)
    परत 1;

  परत L > L_adjusted << ScaleDiff ? 1 : 0;
}

स्थिर शून्य appendDigit(std::string &Str, अचिन्हित D) {
  प्रतिज्ञा(D < 10);
  Str += '0' + D % 10;
}

स्थिर शून्य append_संख्या(std::string &Str, uint64_t N) {
  पर्यंत (N) {
    appendDigit(Str, N % 10);
    N /= 10;
  }
}

स्थिर तार्किक doesRoundUp(अक्षर Digit) {
  निवड (Digit) {
  स्थिती '5':
  स्थिती '6':
  स्थिती '7':
  स्थिती '8':
  स्थिती '9':
    परत सत्य;
  पूर्वनिर्धारित:
    परत असत्य;
  }
}

स्थिर std::string toStringAPFloat(uint64_t D, पूर्णांक E, अचिन्हित Precision) {
  प्रतिज्ञा(E >= ScaledNumbers::MinScale);
  प्रतिज्ञा(E <= ScaledNumbers::MaxScale);

  // Find a new E, but don't let it increase past MaxScale.
  पूर्णांक LeadingZeros = Scaled_संख्या_Base::मोजणी_Leading_Zeros64(D);
  पूर्णांक NewE = std::min(ScaledNumbers::MaxScale, E + 63 - LeadingZeros);
  पूर्णांक Shift = 63 - (NewE - E);
  प्रतिज्ञा(Shift <= LeadingZeros);
  प्रतिज्ञा(Shift == LeadingZeros || NewE == ScaledNumbers::MaxScale);
  प्रतिज्ञा(Shift >= 0 && Shift < 64 && "undefined behavior");
  D <<= Shift;
  E = NewE;

  // Check for a denormal.
  अचिन्हित AdjustedE = E + 16383;
  जर (!(D >> 63)) {
    प्रतिज्ञा(E == ScaledNumbers::MaxScale);
    AdjustedE = 0;
  }

  // Build the float and print it.
  uint64_t RawBits[2] = {D, AdjustedE};
  APFloat Float(APFloat::x87DoubleExtended(), APInt(80, RawBits));
  SmallVector<अक्षर, 24> Chars;
  Float.toString(Chars, Precision, 0);
  परत std::string(Chars.begin(), Chars.end());
}

स्थिर std::string stripTrailingZeros(स्थिरांक std::string &Float) {
  माप_प्रकार NonZero = Float.find_last_not_of('0');
  प्रतिज्ञा(NonZero != std::string::npos && "no . in floating point string");

  जर (Float[NonZero] == '.')
    ++NonZero;

  परत Float.substr(0, NonZero + 1);
}

std::string Scaled_संख्या_Base::toString(uint64_t D, int16_t E, पूर्णांक Width,
                                       अचिन्हित Precision) {
  जर (!D)
    परत "0.0";

  // Canonicalize exponent and digits.
  uint64_t Above0 = 0;
  uint64_t Below0 = 0;
  uint64_t Extra = 0;
  पूर्णांक ExtraShift = 0;
  जर (E == 0) {
    Above0 = D;
  } अन्यथा जर (E > 0) {
    जर (पूर्णांक Shift = std::min(int16_t(मोजणी_Leading_Zeros64(D)), E)) {
      D <<= Shift;
      E -= Shift;

      जर (!E)
        Above0 = D;
    }
  } अन्यथा जर (E > -64) {
    Above0 = D >> -E;
    Below0 = D << (64 + E);
  } अन्यथा जर (E == -64) {
    // Special case: shift by 64 bits is undefined behavior.
    Below0 = D;
  } अन्यथा जर (E > -120) {
    Below0 = D >> (-E - 64);
    Extra = D << (128 + E);
    ExtraShift = -64 - E;
  }

  // Fall back on APFloat for very small and very large numbers.
  जर (!Above0 && !Below0)
    परत toStringAPFloat(D, E, Precision);

  // Append the digits before the decimal.
  std::string Str;
  माप_प्रकार DigitsOut = 0;
  जर (Above0) {
    append_संख्या(Str, Above0);
    DigitsOut = Str.size();
  } अन्यथा {
    appendDigit(Str, 0);
  }
  std::reverse(Str.begin(), Str.end());

  // Return early if there's nothing after the decimal.
  जर (!Below0)
    परत Str + ".0";

  // Append the decimal and beyond.
  Str += '.';
  uint64_t Error = UINT64_C(1) << (64 - Width);

  // We need to shift Below0 to the right to make space for calculating
  // digits.  Save the precision we're losing in Extra.
  Extra = (Below0 & 0xf) << 56 | (Extra >> 8);
  Below0 >>= 4;
  माप_प्रकार SinceDot = 0;
  माप_प्रकार AfterDot = Str.size();
  करा {
    जर (ExtraShift) {
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
    जर (DigitsOut || Str.back() != '0')
      ++DigitsOut;
    ++SinceDot;
  } पर्यंत (Error && (Below0 << 4 | Extra >> 60) >= Error / 2 &&
           (!Precision || DigitsOut <= Precision || SinceDot < 2));

  // Return early for maximum precision.
  जर (!Precision || DigitsOut <= Precision)
    परत stripTrailingZeros(Str);

  // Find where to truncate.
  माप_प्रकार Truncate =
      std::max(Str.size() - (DigitsOut - Precision), AfterDot + 1);

  // Check if there's anything to truncate.
  जर (Truncate >= Str.size())
    परत stripTrailingZeros(Str);

  तार्किक Carry = doesRoundUp(Str[Truncate]);
  जर (!Carry)
    परत stripTrailingZeros(Str.substr(0, Truncate));

  // Round with the first truncated digit.
  साठी (std::string::reverse_iterator I(Str.begin() + Truncate), E = Str.rend();
       I != E; ++I) {
    जर (*I == '.')
      सुरूठेवा;
    जर (*I == '9') {
      *I = '0';
      सुरूठेवा;
    }

    ++*I;
    Carry = असत्य;
    थांबा;
  }

  // Add "1" in front if we still need to carry.
  परत stripTrailingZeros(std::string(Carry, '1') + Str.substr(0, Truncate));
}

raw_ostream &Scaled_संख्या_Base::print(raw_ostream &OS, uint64_t D, int16_t E,
                                     पूर्णांक Width, अचिन्हित Precision) {
  परत OS << toString(D, E, Width, Precision);
}

शून्य Scaled_संख्या_Base::dump(uint64_t D, int16_t E, पूर्णांक Width) {
  print(dbgs(), D, E, Width, 0) << "[" << Width << ":" << D << "*2^" << E
                                << "]";
}
