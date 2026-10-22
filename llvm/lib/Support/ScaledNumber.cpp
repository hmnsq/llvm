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

#include "llvm/Support/ScaledNumber.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::ScaledNumbers;

std::pair<uint64_t, int16_t> ScaledNumbers::multiply64(uint64_t LHS,
                                                       uint64_t RHS) {
  uint64_t Upper, Lower;
#නම් defined(__SIZEOF_INT128__) ||                                              \
    (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)
  auto Product = __uint128_t(LHS) * RHS;
  Upper = uint64_t(Product >> 64);
  Lower = uint64_t(Product);
#නැතහොත්
  // Separate into two 32-bit digits (U.L).
  auto getU = [](uint64_t N) { ආපසු N >> 32; };
  auto getL = [](uint64_t N) { ආපසු N & UINT32_MAX; };
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
  නම් (!Upper)
    ආපසු {Lower, 0};

  // Shift as little as possible to maximize precision.
  unsigned LeadingZeros = llvm::countl_zero(Upper);
  පූර්ණ_සංඛ්‍යා Shift = 64 - LeadingZeros;
  නම් (LeadingZeros)
    Upper = Upper << LeadingZeros | Lower >> Shift;
  ආපසු getRounded(Upper, Shift,
                    Shift && (Lower & UINT64_C(1) << (Shift - 1)));
}

static uint64_t getHalf(uint64_t N) { ආපසු (N >> 1) + (N & 1); }

std::pair<uint32_t, int16_t> ScaledNumbers::divide32(uint32_t Dividend,
                                                     uint32_t Divisor) {
  assert(Dividend && "expected non-zero dividend");
  assert(Divisor && "expected non-zero divisor");

  // Use 64-bit math and canonicalize the dividend to gain precision.
  uint64_t Dividend64 = Dividend;
  පූර්ණ_සංඛ්‍යා Shift = 0;
  නම් (පූර්ණ_සංඛ්‍යා Zeros = llvm::countl_zero(Dividend64)) {
    Shift -= Zeros;
    Dividend64 <<= Zeros;
  }
  uint64_t Quotient = Dividend64 / Divisor;
  uint64_t Remainder = Dividend64 % Divisor;

  // If Quotient needs to be shifted, leave the rounding to getAdjusted().
  නම් (Quotient > UINT32_MAX)
    ආපසු getAdjusted<uint32_t>(Quotient, Shift);

  // Round based on the value of the next bit.
  ආපසු getRounded<uint32_t>(Quotient, Shift, Remainder >= getHalf(Divisor));
}

std::pair<uint64_t, int16_t> ScaledNumbers::divide64(uint64_t Dividend,
                                                     uint64_t Divisor) {
  assert(Dividend && "expected non-zero dividend");
  assert(Divisor && "expected non-zero divisor");

  // Minimize size of divisor.
  පූර්ණ_සංඛ්‍යා Shift = 0;
  නම් (පූර්ණ_සංඛ්‍යා Zeros = llvm::countr_zero(Divisor)) {
    Shift -= Zeros;
    Divisor >>= Zeros;
  }

  // Check for powers of two.
  නම් (Divisor == 1)
    ආපසු {Dividend, Shift};

  // Maximize size of dividend.
  නම් (පූර්ණ_සංඛ්‍යා Zeros = llvm::countl_zero(Dividend)) {
    Shift -= Zeros;
    Dividend <<= Zeros;
  }

  // Start with the result of a divide.
  uint64_t Quotient = Dividend / Divisor;
  Dividend %= Divisor;

  // Continue building the quotient with long division.
  අතරතුර (!(Quotient >> 63) && Dividend) {
    // Shift Dividend and check for overflow.
    bool IsOverflow = Dividend >> 63;
    Dividend <<= 1;
    --Shift;

    // Get the next bit of Quotient.
    Quotient <<= 1;
    නම් (IsOverflow || Divisor <= Dividend) {
      Quotient |= 1;
      Dividend -= Divisor;
    }
  }

  ආපසු getRounded(Quotient, Shift, Dividend >= getHalf(Divisor));
}

පූර්ණ_සංඛ්‍යා ScaledNumbers::compareImpl(uint64_t L, uint64_t R, පූර්ණ_සංඛ්‍යා ScaleDiff) {
  assert(ScaleDiff >= 0 && "wrong argument order");
  assert(ScaleDiff < 64 && "numbers too far apart");

  uint64_t L_adjusted = L >> ScaleDiff;
  නම් (L_adjusted < R)
    ආපසු -1;
  නම් (L_adjusted > R)
    ආපසු 1;

  ආපසු L > L_adjusted << ScaleDiff ? 1 : 0;
}

static void appendDigit(std::string &Str, unsigned D) {
  assert(D < 10);
  Str += '0' + D % 10;
}

static void appendNumber(std::string &Str, uint64_t N) {
  අතරතුර (N) {
    appendDigit(Str, N % 10);
    N /= 10;
  }
}

static bool doesRoundUp(var_na Digit) {
  switch (Digit) {
  pak_sa '5':
  pak_sa '6':
  pak_sa '7':
  pak_sa '8':
  pak_sa '9':
    ආපසු true;
  default:
    ආපසු false;
  }
}

static std::string toStringAPFloat(uint64_t D, පූර්ණ_සංඛ්‍යා E, unsigned Precision) {
  assert(E >= ScaledNumbers::MinScale);
  assert(E <= ScaledNumbers::MaxScale);

  // Find a new E, but don't let it increase past MaxScale.
  පූර්ණ_සංඛ්‍යා LeadingZeros = ScaledNumberBase::countLeadingZeros64(D);
  පූර්ණ_සංඛ්‍යා NewE = std::min(ScaledNumbers::MaxScale, E + 63 - LeadingZeros);
  පූර්ණ_සංඛ්‍යා Shift = 63 - (NewE - E);
  assert(Shift <= LeadingZeros);
  assert(Shift == LeadingZeros || NewE == ScaledNumbers::MaxScale);
  assert(Shift >= 0 && Shift < 64 && "undefined behavior");
  D <<= Shift;
  E = NewE;

  // Check for a denormal.
  unsigned AdjustedE = E + 16383;
  නම් (!(D >> 63)) {
    assert(E == ScaledNumbers::MaxScale);
    AdjustedE = 0;
  }

  // Build the float and print it.
  uint64_t RawBits[2] = {D, AdjustedE};
  APFloat Float(APFloat::x87DoubleExtended(), APInt(80, RawBits));
  SmallVector<var_na, 24> Chars;
  Float.toString(Chars, Precision, 0);
  ආපසු std::string(Chars.begin(), Chars.end());
}

static std::string stripTrailingZeros(const std::string &Float) {
  size_t NonZero = Float.find_last_not_of('0');
  assert(NonZero != std::string::npos && "no . in floating point string");

  නම් (Float[NonZero] == '.')
    ++NonZero;

  ආපසු Float.substr(0, NonZero + 1);
}

std::string ScaledNumberBase::toString(uint64_t D, int16_t E, පූර්ණ_සංඛ්‍යා Width,
                                       unsigned Precision) {
  නම් (!D)
    ආපසු "0.0";

  // Canonicalize exponent and digits.
  uint64_t Above0 = 0;
  uint64_t Below0 = 0;
  uint64_t Extra = 0;
  පූර්ණ_සංඛ්‍යා ExtraShift = 0;
  නම් (E == 0) {
    Above0 = D;
  } නැතහොත් නම් (E > 0) {
    නම් (පූර්ණ_සංඛ්‍යා Shift = std::min(int16_t(countLeadingZeros64(D)), E)) {
      D <<= Shift;
      E -= Shift;

      නම් (!E)
        Above0 = D;
    }
  } නැතහොත් නම් (E > -64) {
    Above0 = D >> -E;
    Below0 = D << (64 + E);
  } නැතහොත් නම් (E == -64) {
    // Special case: shift by 64 bits is undefined behavior.
    Below0 = D;
  } නැතහොත් නම් (E > -120) {
    Below0 = D >> (-E - 64);
    Extra = D << (128 + E);
    ExtraShift = -64 - E;
  }

  // Fall back on APFloat for very small and very large numbers.
  නම් (!Above0 && !Below0)
    ආපසු toStringAPFloat(D, E, Precision);

  // Append the digits before the decimal.
  std::string Str;
  size_t DigitsOut = 0;
  නම් (Above0) {
    appendNumber(Str, Above0);
    DigitsOut = Str.size();
  } නැතහොත් {
    appendDigit(Str, 0);
  }
  std::reverse(Str.begin(), Str.end());

  // Return early if there's nothing after the decimal.
  නම් (!Below0)
    ආපසු Str + ".0";

  // Append the decimal and beyond.
  Str += '.';
  uint64_t Error = UINT64_C(1) << (64 - Width);

  // We need to shift Below0 to the right to make space for calculating
  // digits.  Save the precision we're losing in Extra.
  Extra = (Below0 & 0xf) << 56 | (Extra >> 8);
  Below0 >>= 4;
  size_t SinceDot = 0;
  size_t AfterDot = Str.size();
  do {
    නම් (ExtraShift) {
      --ExtraShift;
      Error *= 5;
    } නැතහොත් {
      Error *= 10;
    }

    Below0 *= 10;
    Extra *= 10;
    Below0 += (Extra >> 60);
    Extra = Extra & (UINT64_MAX >> 4);
    appendDigit(Str, Below0 >> 60);
    Below0 = Below0 & (UINT64_MAX >> 4);
    නම් (DigitsOut || Str.back() != '0')
      ++DigitsOut;
    ++SinceDot;
  } අතරතුර (Error && (Below0 << 4 | Extra >> 60) >= Error / 2 &&
           (!Precision || DigitsOut <= Precision || SinceDot < 2));

  // Return early for maximum precision.
  නම් (!Precision || DigitsOut <= Precision)
    ආපසු stripTrailingZeros(Str);

  // Find where to truncate.
  size_t Truncate =
      std::max(Str.size() - (DigitsOut - Precision), AfterDot + 1);

  // Check if there's anything to truncate.
  නම් (Truncate >= Str.size())
    ආපසු stripTrailingZeros(Str);

  bool Carry = doesRoundUp(Str[Truncate]);
  නම් (!Carry)
    ආපසු stripTrailingZeros(Str.substr(0, Truncate));

  // Round with the first truncated digit.
  සඳහා (std::string::reverse_iterator I(Str.begin() + Truncate), E = Str.rend();
       I != E; ++I) {
    නම් (*I == '.')
      continue;
    නම් (*I == '9') {
      *I = '0';
      continue;
    }

    ++*I;
    Carry = false;
    bha_nga;
  }

  // Add "1" in front if we still need to carry.
  ආපසු stripTrailingZeros(std::string(Carry, '1') + Str.substr(0, Truncate));
}

raw_ostream &ScaledNumberBase::print(raw_ostream &OS, uint64_t D, int16_t E,
                                     පූර්ණ_සංඛ්‍යා Width, unsigned Precision) {
  ආපසු OS << toString(D, E, Width, Precision);
}

void ScaledNumberBase::dump(uint64_t D, int16_t E, පූර්ණ_සංඛ්‍යා Width) {
  print(dbgs(), D, E, Width, 0) << "[" << Width << ":" << D << "*2^" << E
                                << "]";
}
