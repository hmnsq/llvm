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

#ضمن "llvm/Support/ScaledNumber.h"
#ضمن "llvm/ADT/APFloat.h"
#ضمن "llvm/ADT/ArrayRef.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/raw_ostream.h"

using نطاق llvm;
using نطاق llvm::ScaledNumbers;

std::pair<uint64_t, int16_t> ScaledNumbers::multiply64(uint64_t LHS,
                                                       uint64_t RHS) {
  uint64_t Upper, Lower;
#إذا defined(__SIZEOF_INT128__) ||                                              \
    (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)
  auto Product = __uint128_t(LHS) * RHS;
  Upper = uint64_t(Product >> 64);
  Lower = uint64_t(Product);
#وإلا
  // Separate into two 32-bit digits (U.L).
  auto getU = [](uint64_t N) { أرجع N >> 32; };
  auto getL = [](uint64_t N) { أرجع N & UINT32_MAX; };
  uint64_t UL = getU(LHS), LL = getL(LHS), UR = getU(RHS), LR = getL(RHS);

  // Compute cross products.
  uint64_t P1 = UL * UR, P2 = UL * LR, P3 = LL * UR, P4 = LL * LR;

  // Sum into two 64-bit digits.
  Upper = P1;
  Lower = P4;
  auto addWithCarry = [&](uint64_t N) {
    uint64_t NewLower = Lower + (getL(N) << 32);
    Upper += getU(N) + (NewLower < Lower);
    Lower = NewLower;
  };
  addWithCarry(P2);
  addWithCarry(P3);
#endif

  // Check whether the upper digit is empty.
  إذا (!Upper)
    أرجع {Lower, 0};

  // Shift as little as possible to maximize precision.
  unsigned LeadingZeros = llvm::countl_zero(Upper);
  صحيح Shift = 64 - LeadingZeros;
  إذا (LeadingZeros)
    Upper = Upper << LeadingZeros | Lower >> Shift;
  أرجع getRounded(Upper, Shift,
                    Shift && (Lower & UINT64_C(1) << (Shift - 1)));
}

ساكن uint64_t getHalf(uint64_t N) { أرجع (N >> 1) + (N & 1); }

std::pair<uint32_t, int16_t> ScaledNumbers::divide32(uint32_t Dividend,
                                                     uint32_t مي_ربو_2) {
  أكد(Dividend && "expected non-zero dividend");
  أكد(مي_ربو_2 && "expected non-zero divisor");

  // Use 64-bit math and canonicalize the dividend to gain precision.
  uint64_t Dividend64 = Dividend;
  صحيح Shift = 0;
  إذا (صحيح Zeros = llvm::countl_zero(Dividend64)) {
    Shift -= Zeros;
    Dividend64 <<= Zeros;
  }
  uint64_t Quotient = Dividend64 / مي_ربو_2;
  uint64_t Remainder = Dividend64 % مي_ربو_2;

  // If Quotient needs to be shifted, leave the rounding to getAdjusted().
  إذا (Quotient > UINT32_MAX)
    أرجع getAdjusted<uint32_t>(Quotient, Shift);

  // Round based on the value of the next bit.
  أرجع getRounded<uint32_t>(Quotient, Shift, Remainder >= getHalf(مي_ربو_2));
}

std::pair<uint64_t, int16_t> ScaledNumbers::divide64(uint64_t Dividend,
                                                     uint64_t مي_ربو_2) {
  أكد(Dividend && "expected non-zero dividend");
  أكد(مي_ربو_2 && "expected non-zero divisor");

  // Minimize size of divisor.
  صحيح Shift = 0;
  إذا (صحيح Zeros = llvm::countr_zero(مي_ربو_2)) {
    Shift -= Zeros;
    مي_ربو_2 >>= Zeros;
  }

  // Check for powers of two.
  إذا (مي_ربو_2 == 1)
    أرجع {Dividend, Shift};

  // Maximize size of dividend.
  إذا (صحيح Zeros = llvm::countl_zero(Dividend)) {
    Shift -= Zeros;
    Dividend <<= Zeros;
  }

  // Start with the result of a divide.
  uint64_t Quotient = Dividend / مي_ربو_2;
  Dividend %= مي_ربو_2;

  // Continue building the quotient with long division.
  طالما (!(Quotient >> 63) && Dividend) {
    // Shift Dividend and check for overflow.
    منطقي IsOverflow = Dividend >> 63;
    Dividend <<= 1;
    --Shift;

    // Get the next bit of Quotient.
    Quotient <<= 1;
    إذا (IsOverflow || مي_ربو_2 <= Dividend) {
      Quotient |= 1;
      Dividend -= مي_ربو_2;
    }
  }

  أرجع getRounded(Quotient, Shift, Dividend >= getHalf(مي_ربو_2));
}

صحيح ScaledNumbers::compareImpl(uint64_t L, uint64_t R, صحيح ScaleDiff) {
  أكد(ScaleDiff >= 0 && "wrong argument order");
  أكد(ScaleDiff < 64 && "numbers too far apart");

  uint64_t L_adjusted = L >> ScaleDiff;
  إذا (L_adjusted < R)
    أرجع -1;
  إذا (L_adjusted > R)
    أرجع 1;

  أرجع L > L_adjusted << ScaleDiff ? 1 : 0;
}

ساكن فراغ appendDigit(std::string &Str, unsigned D) {
  أكد(D < 10);
  Str += '0' + D % 10;
}

ساكن فراغ append_لمب(std::string &Str, uint64_t N) {
  طالما (N) {
    appendDigit(Str, N % 10);
    N /= 10;
  }
}

ساكن منطقي doesRoundUp(حرف Digit) {
  بدل (Digit) {
  حالة '5':
  حالة '6':
  حالة '7':
  حالة '8':
  حالة '9':
    أرجع true;
  افتراضي:
    أرجع خطأ;
  }
}

ساكن std::string toStringAPFloat(uint64_t D, صحيح E, unsigned Precision) {
  أكد(E >= ScaledNumbers::MinScale);
  أكد(E <= ScaledNumbers::MaxScale);

  // Find a new E, but don't let it increase past MaxScale.
  صحيح LeadingZeros = Scaled_لمب_Base::كدي_Leading_Zeros64(D);
  صحيح NewE = std::min(ScaledNumbers::MaxScale, E + 63 - LeadingZeros);
  صحيح Shift = 63 - (NewE - E);
  أكد(Shift <= LeadingZeros);
  أكد(Shift == LeadingZeros || NewE == ScaledNumbers::MaxScale);
  أكد(Shift >= 0 && Shift < 64 && "undefined behavior");
  D <<= Shift;
  E = NewE;

  // Check for a denormal.
  unsigned AdjustedE = E + 16383;
  إذا (!(D >> 63)) {
    أكد(E == ScaledNumbers::MaxScale);
    AdjustedE = 0;
  }

  // Build the float and print it.
  uint64_t RawBits[2] = {D, AdjustedE};
  APFloat Float(APFloat::x87DoubleExtended(), APInt(80, RawBits));
  SmallVector<حرف, 24> Chars;
  Float.toString(Chars, Precision, 0);
  أرجع std::string(Chars.begin(), Chars.end());
}

ساكن std::string stripTrailingZeros(ثابت std::string &Float) {
  size_t NonZero = Float.find_last_not_of('0');
  أكد(NonZero != std::string::npos && "no . in floating point string");

  إذا (Float[NonZero] == '.')
    ++NonZero;

  أرجع Float.substr(0, NonZero + 1);
}

std::string Scaled_لمب_Base::toString(uint64_t D, int16_t E, صحيح Width,
                                       unsigned Precision) {
  إذا (!D)
    أرجع "0.0";

  // Canonicalize exponent and digits.
  uint64_t Above0 = 0;
  uint64_t Below0 = 0;
  uint64_t Extra = 0;
  صحيح ExtraShift = 0;
  إذا (E == 0) {
    Above0 = D;
  } وإلا إذا (E > 0) {
    إذا (صحيح Shift = std::min(int16_t(كدي_Leading_Zeros64(D)), E)) {
      D <<= Shift;
      E -= Shift;

      إذا (!E)
        Above0 = D;
    }
  } وإلا إذا (E > -64) {
    Above0 = D >> -E;
    Below0 = D << (64 + E);
  } وإلا إذا (E == -64) {
    // Special case: shift by 64 bits is undefined behavior.
    Below0 = D;
  } وإلا إذا (E > -120) {
    Below0 = D >> (-E - 64);
    Extra = D << (128 + E);
    ExtraShift = -64 - E;
  }

  // Fall back on APFloat for very small and very large numbers.
  إذا (!Above0 && !Below0)
    أرجع toStringAPFloat(D, E, Precision);

  // Append the digits before the decimal.
  std::string Str;
  size_t DigitsOut = 0;
  إذا (Above0) {
    append_لمب(Str, Above0);
    DigitsOut = Str.size();
  } وإلا {
    appendDigit(Str, 0);
  }
  std::reverse(Str.begin(), Str.end());

  // Return early if there's nothing after the decimal.
  إذا (!Below0)
    أرجع Str + ".0";

  // Append the decimal and beyond.
  Str += '.';
  uint64_t Error = UINT64_C(1) << (64 - Width);

  // We need to shift Below0 to the right to make space for calculating
  // digits.  Save the precision we're losing in Extra.
  Extra = (Below0 & 0xf) << 56 | (Extra >> 8);
  Below0 >>= 4;
  size_t SinceDot = 0;
  size_t AfterDot = Str.size();
  افعل {
    إذا (ExtraShift) {
      --ExtraShift;
      Error *= 5;
    } وإلا {
      Error *= 10;
    }

    Below0 *= 10;
    Extra *= 10;
    Below0 += (Extra >> 60);
    Extra = Extra & (UINT64_MAX >> 4);
    appendDigit(Str, Below0 >> 60);
    Below0 = Below0 & (UINT64_MAX >> 4);
    إذا (DigitsOut || Str.back() != '0')
      ++DigitsOut;
    ++SinceDot;
  } طالما (Error && (Below0 << 4 | Extra >> 60) >= Error / 2 &&
           (!Precision || DigitsOut <= Precision || SinceDot < 2));

  // Return early for maximum precision.
  إذا (!Precision || DigitsOut <= Precision)
    أرجع stripTrailingZeros(Str);

  // Find where to truncate.
  size_t Truncate =
      std::max(Str.size() - (DigitsOut - Precision), AfterDot + 1);

  // Check if there's anything to truncate.
  إذا (Truncate >= Str.size())
    أرجع stripTrailingZeros(Str);

  منطقي Carry = doesRoundUp(Str[Truncate]);
  إذا (!Carry)
    أرجع stripTrailingZeros(Str.substr(0, Truncate));

  // Round with the first truncated digit.
  لكل (std::string::reverse_iterator I(Str.begin() + Truncate), E = Str.rend();
       I != E; ++I) {
    إذا (*I == '.')
      استمر;
    إذا (*I == '9') {
      *I = '0';
      استمر;
    }

    ++*I;
    Carry = خطأ;
    اكسر;
  }

  // Add "1" in front if we still need to carry.
  أرجع stripTrailingZeros(std::string(Carry, '1') + Str.substr(0, Truncate));
}

raw_ostream &Scaled_لمب_Base::بغ(raw_ostream &OS, uint64_t D, int16_t E,
                                     صحيح Width, unsigned Precision) {
  أرجع OS << toString(D, E, Width, Precision);
}

فراغ Scaled_لمب_Base::dump(uint64_t D, int16_t E, صحيح Width) {
  بغ(dbgs(), D, E, Width, 0) << "[" << Width << ":" << D << "*2^" << E
                                << "]";
}
