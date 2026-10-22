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

#شامل "llvm/Support/ScaledNumber.h"
#شامل "llvm/ADT/APFloat.h"
#شامل "llvm/ADT/ArrayRef.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/raw_ostream.h"

using فضاینام llvm;
using فضاینام llvm::ScaledNumbers;

std::pair<uint64_t, int16_t> ScaledNumbers::multiply64(uint64_t LHS,
                                                       uint64_t RHS) {
  uint64_t Upper, Lower;
#اگر defined(__SIZEOF_INT128__) ||                                              \
    (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)
  auto Product = __uint128_t(LHS) * RHS;
  Upper = uint64_t(Product >> 64);
  Lower = uint64_t(Product);
#وگرنه
  // Separate into two 32-bit digits (U.L).
  auto getU = [](uint64_t N) { بازگشت N >> 32; };
  auto getL = [](uint64_t N) { بازگشت N & UINT32_MAX; };
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
  اگر (!Upper)
    بازگشت {Lower, 0};

  // Shift as little as possible to maximize precision.
  unsigned LeadingZeros = llvm::countl_zero(Upper);
  صحیح Shift = 64 - LeadingZeros;
  اگر (LeadingZeros)
    Upper = Upper << LeadingZeros | Lower >> Shift;
  بازگشت getRounded(Upper, Shift,
                    Shift && (Lower & UINT64_C(1) << (Shift - 1)));
}

ایستا uint64_t getHalf(uint64_t N) { بازگشت (N >> 1) + (N & 1); }

std::pair<uint32_t, int16_t> ScaledNumbers::divide32(uint32_t Dividend,
                                                     uint32_t مقسوم_علیه_2) {
  أكد(Dividend && "expected non-zero dividend");
  أكد(مقسوم_علیه_2 && "expected non-zero divisor");

  // Use 64-bit math and canonicalize the dividend to gain precision.
  uint64_t Dividend64 = Dividend;
  صحیح Shift = 0;
  اگر (صحیح Zeros = llvm::countl_zero(Dividend64)) {
    Shift -= Zeros;
    Dividend64 <<= Zeros;
  }
  uint64_t Quotient = Dividend64 / مقسوم_علیه_2;
  uint64_t Remainder = Dividend64 % مقسوم_علیه_2;

  // If Quotient needs to be shifted, leave the rounding to getAdjusted().
  اگر (Quotient > UINT32_MAX)
    بازگشت getAdjusted<uint32_t>(Quotient, Shift);

  // Round based on the value of the next bit.
  بازگشت getRounded<uint32_t>(Quotient, Shift, Remainder >= getHalf(مقسوم_علیه_2));
}

std::pair<uint64_t, int16_t> ScaledNumbers::divide64(uint64_t Dividend,
                                                     uint64_t مقسوم_علیه_2) {
  أكد(Dividend && "expected non-zero dividend");
  أكد(مقسوم_علیه_2 && "expected non-zero divisor");

  // Minimize size of divisor.
  صحیح Shift = 0;
  اگر (صحیح Zeros = llvm::countr_zero(مقسوم_علیه_2)) {
    Shift -= Zeros;
    مقسوم_علیه_2 >>= Zeros;
  }

  // Check for powers of two.
  اگر (مقسوم_علیه_2 == 1)
    بازگشت {Dividend, Shift};

  // Maximize size of dividend.
  اگر (صحیح Zeros = llvm::countl_zero(Dividend)) {
    Shift -= Zeros;
    Dividend <<= Zeros;
  }

  // Start with the result of a divide.
  uint64_t Quotient = Dividend / مقسوم_علیه_2;
  Dividend %= مقسوم_علیه_2;

  // Continue building the quotient with long division.
  وقتی (!(Quotient >> 63) && Dividend) {
    // Shift Dividend and check for overflow.
    منطقی IsOverflow = Dividend >> 63;
    Dividend <<= 1;
    --Shift;

    // Get the next bit of Quotient.
    Quotient <<= 1;
    اگر (IsOverflow || مقسوم_علیه_2 <= Dividend) {
      Quotient |= 1;
      Dividend -= مقسوم_علیه_2;
    }
  }

  بازگشت getRounded(Quotient, Shift, Dividend >= getHalf(مقسوم_علیه_2));
}

صحیح ScaledNumbers::compareImpl(uint64_t L, uint64_t R, صحیح ScaleDiff) {
  أكد(ScaleDiff >= 0 && "wrong argument order");
  أكد(ScaleDiff < 64 && "numbers too far apart");

  uint64_t L_adjusted = L >> ScaleDiff;
  اگر (L_adjusted < R)
    بازگشت -1;
  اگر (L_adjusted > R)
    بازگشت 1;

  بازگشت L > L_adjusted << ScaleDiff ? 1 : 0;
}

ایستا تهی appendDigit(std::string &Str, unsigned D) {
  أكد(D < 10);
  Str += '0' + D % 10;
}

ایستا تهی append_عدد(std::string &Str, uint64_t N) {
  وقتی (N) {
    appendDigit(Str, N % 10);
    N /= 10;
  }
}

ایستا منطقی doesRoundUp(نویسه Digit) {
  انتخاب (Digit) {
  حالت '5':
  حالت '6':
  حالت '7':
  حالت '8':
  حالت '9':
    بازگشت درست;
  پیشفرض:
    بازگشت نادرست;
  }
}

ایستا std::string toStringAPFloat(uint64_t D, صحیح E, unsigned Precision) {
  أكد(E >= ScaledNumbers::MinScale);
  أكد(E <= ScaledNumbers::MaxScale);

  // Find a new E, but don't let it increase past MaxScale.
  صحیح LeadingZeros = Scaled_عدد_Base::شمار_Leading_Zeros64(D);
  صحیح NewE = std::min(ScaledNumbers::MaxScale, E + 63 - LeadingZeros);
  صحیح Shift = 63 - (NewE - E);
  أكد(Shift <= LeadingZeros);
  أكد(Shift == LeadingZeros || NewE == ScaledNumbers::MaxScale);
  أكد(Shift >= 0 && Shift < 64 && "undefined behavior");
  D <<= Shift;
  E = NewE;

  // Check for a denormal.
  unsigned AdjustedE = E + 16383;
  اگر (!(D >> 63)) {
    أكد(E == ScaledNumbers::MaxScale);
    AdjustedE = 0;
  }

  // Build the float and print it.
  uint64_t RawBits[2] = {D, AdjustedE};
  APFloat Float(APFloat::x87DoubleExtended(), APInt(80, RawBits));
  SmallVector<نویسه, 24> Chars;
  Float.toString(Chars, Precision, 0);
  بازگشت std::string(Chars.begin(), Chars.end());
}

ایستا std::string stripTrailingZeros(ثابت std::string &Float) {
  size_t NonZero = Float.find_last_not_of('0');
  أكد(NonZero != std::string::npos && "no . in floating point string");

  اگر (Float[NonZero] == '.')
    ++NonZero;

  بازگشت Float.substr(0, NonZero + 1);
}

std::string Scaled_عدد_Base::toString(uint64_t D, int16_t E, صحیح Width,
                                       unsigned Precision) {
  اگر (!D)
    بازگشت "0.0";

  // Canonicalize exponent and digits.
  uint64_t Above0 = 0;
  uint64_t Below0 = 0;
  uint64_t Extra = 0;
  صحیح ExtraShift = 0;
  اگر (E == 0) {
    Above0 = D;
  } وگرنه اگر (E > 0) {
    اگر (صحیح Shift = std::min(int16_t(شمار_Leading_Zeros64(D)), E)) {
      D <<= Shift;
      E -= Shift;

      اگر (!E)
        Above0 = D;
    }
  } وگرنه اگر (E > -64) {
    Above0 = D >> -E;
    Below0 = D << (64 + E);
  } وگرنه اگر (E == -64) {
    // Special case: shift by 64 bits is undefined behavior.
    Below0 = D;
  } وگرنه اگر (E > -120) {
    Below0 = D >> (-E - 64);
    Extra = D << (128 + E);
    ExtraShift = -64 - E;
  }

  // Fall back on APFloat for very small and very large numbers.
  اگر (!Above0 && !Below0)
    بازگشت toStringAPFloat(D, E, Precision);

  // Append the digits before the decimal.
  std::string Str;
  size_t DigitsOut = 0;
  اگر (Above0) {
    append_عدد(Str, Above0);
    DigitsOut = Str.size();
  } وگرنه {
    appendDigit(Str, 0);
  }
  std::reverse(Str.begin(), Str.end());

  // Return early if there's nothing after the decimal.
  اگر (!Below0)
    بازگشت Str + ".0";

  // Append the decimal and beyond.
  Str += '.';
  uint64_t Error = UINT64_C(1) << (64 - Width);

  // We need to shift Below0 to the right to make space for calculating
  // digits.  Save the precision we're losing in Extra.
  Extra = (Below0 & 0xf) << 56 | (Extra >> 8);
  Below0 >>= 4;
  size_t SinceDot = 0;
  size_t AfterDot = Str.size();
  انجامبده {
    اگر (ExtraShift) {
      --ExtraShift;
      Error *= 5;
    } وگرنه {
      Error *= 10;
    }

    Below0 *= 10;
    Extra *= 10;
    Below0 += (Extra >> 60);
    Extra = Extra & (UINT64_MAX >> 4);
    appendDigit(Str, Below0 >> 60);
    Below0 = Below0 & (UINT64_MAX >> 4);
    اگر (DigitsOut || Str.back() != '0')
      ++DigitsOut;
    ++SinceDot;
  } وقتی (Error && (Below0 << 4 | Extra >> 60) >= Error / 2 &&
           (!Precision || DigitsOut <= Precision || SinceDot < 2));

  // Return early for maximum precision.
  اگر (!Precision || DigitsOut <= Precision)
    بازگشت stripTrailingZeros(Str);

  // Find where to truncate.
  size_t Truncate =
      std::max(Str.size() - (DigitsOut - Precision), AfterDot + 1);

  // Check if there's anything to truncate.
  اگر (Truncate >= Str.size())
    بازگشت stripTrailingZeros(Str);

  منطقی Carry = doesRoundUp(Str[Truncate]);
  اگر (!Carry)
    بازگشت stripTrailingZeros(Str.substr(0, Truncate));

  // Round with the first truncated digit.
  برای (std::string::reverse_iterator I(Str.begin() + Truncate), E = Str.rend();
       I != E; ++I) {
    اگر (*I == '.')
      ادامه;
    اگر (*I == '9') {
      *I = '0';
      ادامه;
    }

    ++*I;
    Carry = نادرست;
    بشکن;
  }

  // Add "1" in front if we still need to carry.
  بازگشت stripTrailingZeros(std::string(Carry, '1') + Str.substr(0, Truncate));
}

raw_ostream &Scaled_عدد_Base::print(raw_ostream &OS, uint64_t D, int16_t E,
                                     صحیح Width, unsigned Precision) {
  بازگشت OS << toString(D, E, Width, Precision);
}

تهی Scaled_عدد_Base::dump(uint64_t D, int16_t E, صحیح Width) {
  print(dbgs(), D, E, Width, 0) << "[" << Width << ":" << D << "*2^" << E
                                << "]";
}
