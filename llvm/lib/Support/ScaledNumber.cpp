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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/ScaledNumber.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/APFloat.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ArrayRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Debug.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::ScaledNumbers;

std::pair<uint64_t, int16_t> ScaledNumbers::multiply64(uint64_t LHS,
                                                       uint64_t RHS) {
  uint64_t Upper, Lower;
#ଯଦି defined(__SIZEOF_INT128__) ||                                              \
    (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)
  auto Product = __uint128_t(LHS) * RHS;
  Upper = uint64_t(Product >> 64);
  Lower = uint64_t(Product);
#ଅନ୍ୟଥା
  // Separate into two 32-bit digits (U.L).
  auto getU = [](uint64_t N) { ଫେରନ୍ତୁ N >> 32; };
  auto getL = [](uint64_t N) { ଫେରନ୍ତୁ N & UINT32_MAX; };
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
  ଯଦି (!Upper)
    ଫେରନ୍ତୁ {Lower, 0};

  // Shift as little as possible to maximize precision.
  unsigned LeadingZeros = llvm::countl_zero(Upper);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Shift = 64 - LeadingZeros;
  ଯଦି (LeadingZeros)
    Upper = Upper << LeadingZeros | Lower >> Shift;
  ଫେରନ୍ତୁ getRounded(Upper, Shift,
                    Shift && (Lower & UINT64_C(1) << (Shift - 1)));
}

ସ୍ଥିର uint64_t getHalf(uint64_t N) { ଫେରନ୍ତୁ (N >> 1) + (N & 1); }

std::pair<uint32_t, int16_t> ScaledNumbers::divide32(uint32_t Dividend,
                                                     uint32_t ଭାଜକ_2) {
  ପ୍ରତିଜ୍ଞା(Dividend && "expected non-zero dividend");
  ପ୍ରତିଜ୍ଞା(ଭାଜକ_2 && "expected non-zero divisor");

  // Use 64-bit math and canonicalize the dividend to gain precision.
  uint64_t Dividend64 = Dividend;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Shift = 0;
  ଯଦି (ପୂର୍ଣ୍ଣସଂଖ୍ୟା Zeros = llvm::countl_zero(Dividend64)) {
    Shift -= Zeros;
    Dividend64 <<= Zeros;
  }
  uint64_t Quotient = Dividend64 / ଭାଜକ_2;
  uint64_t Remainder = Dividend64 % ଭାଜକ_2;

  // If Quotient needs to be shifted, leave the rounding to getAdjusted().
  ଯଦି (Quotient > UINT32_MAX)
    ଫେରନ୍ତୁ getAdjusted<uint32_t>(Quotient, Shift);

  // Round based on the value of the next bit.
  ଫେରନ୍ତୁ getRounded<uint32_t>(Quotient, Shift, Remainder >= getHalf(ଭାଜକ_2));
}

std::pair<uint64_t, int16_t> ScaledNumbers::divide64(uint64_t Dividend,
                                                     uint64_t ଭାଜକ_2) {
  ପ୍ରତିଜ୍ଞା(Dividend && "expected non-zero dividend");
  ପ୍ରତିଜ୍ଞା(ଭାଜକ_2 && "expected non-zero divisor");

  // Minimize size of divisor.
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Shift = 0;
  ଯଦି (ପୂର୍ଣ୍ଣସଂଖ୍ୟା Zeros = llvm::countr_zero(ଭାଜକ_2)) {
    Shift -= Zeros;
    ଭାଜକ_2 >>= Zeros;
  }

  // Check for powers of two.
  ଯଦି (ଭାଜକ_2 == 1)
    ଫେରନ୍ତୁ {Dividend, Shift};

  // Maximize size of dividend.
  ଯଦି (ପୂର୍ଣ୍ଣସଂଖ୍ୟା Zeros = llvm::countl_zero(Dividend)) {
    Shift -= Zeros;
    Dividend <<= Zeros;
  }

  // Start with the result of a divide.
  uint64_t Quotient = Dividend / ଭାଜକ_2;
  Dividend %= ଭାଜକ_2;

  // Continue building the quotient with long division.
  ଯେତେବେଳେ (!(Quotient >> 63) && Dividend) {
    // Shift Dividend and check for overflow.
    ତାର୍କିକ IsOverflow = Dividend >> 63;
    Dividend <<= 1;
    --Shift;

    // Get the next bit of Quotient.
    Quotient <<= 1;
    ଯଦି (IsOverflow || ଭାଜକ_2 <= Dividend) {
      Quotient |= 1;
      Dividend -= ଭାଜକ_2;
    }
  }

  ଫେରନ୍ତୁ getRounded(Quotient, Shift, Dividend >= getHalf(ଭାଜକ_2));
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ScaledNumbers::compareImpl(uint64_t L, uint64_t R, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ScaleDiff) {
  ପ୍ରତିଜ୍ଞା(ScaleDiff >= 0 && "wrong argument order");
  ପ୍ରତିଜ୍ଞା(ScaleDiff < 64 && "numbers too far apart");

  uint64_t L_adjusted = L >> ScaleDiff;
  ଯଦି (L_adjusted < R)
    ଫେରନ୍ତୁ -1;
  ଯଦି (L_adjusted > R)
    ଫେରନ୍ତୁ 1;

  ଫେରନ୍ତୁ L > L_adjusted << ScaleDiff ? 1 : 0;
}

ସ୍ଥିର ଶୂନ୍ୟ appendDigit(std::string &Str, unsigned D) {
  ପ୍ରତିଜ୍ଞା(D < 10);
  Str += '0' + D % 10;
}

ସ୍ଥିର ଶୂନ୍ୟ append_ସଂଖ୍ୟା(std::string &Str, uint64_t N) {
  ଯେତେବେଳେ (N) {
    appendDigit(Str, N % 10);
    N /= 10;
  }
}

ସ୍ଥିର ତାର୍କିକ doesRoundUp(ଅକ୍ଷର Digit) {
  ବାଛ (Digit) {
  ସ୍ଥିତି '5':
  ସ୍ଥିତି '6':
  ସ୍ଥିତି '7':
  ସ୍ଥିତି '8':
  ସ୍ଥିତି '9':
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  }
}

ସ୍ଥିର std::string toStringAPFloat(uint64_t D, ପୂର୍ଣ୍ଣସଂଖ୍ୟା E, unsigned Precision) {
  ପ୍ରତିଜ୍ଞା(E >= ScaledNumbers::MinScale);
  ପ୍ରତିଜ୍ଞା(E <= ScaledNumbers::MaxScale);

  // Find a new E, but don't let it increase past MaxScale.
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା LeadingZeros = Scaled_ସଂଖ୍ୟା_Base::ଗଣନା_Leading_Zeros64(D);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା NewE = std::min(ScaledNumbers::MaxScale, E + 63 - LeadingZeros);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Shift = 63 - (NewE - E);
  ପ୍ରତିଜ୍ଞା(Shift <= LeadingZeros);
  ପ୍ରତିଜ୍ଞା(Shift == LeadingZeros || NewE == ScaledNumbers::MaxScale);
  ପ୍ରତିଜ୍ଞା(Shift >= 0 && Shift < 64 && "undefined behavior");
  D <<= Shift;
  E = NewE;

  // Check for a denormal.
  unsigned AdjustedE = E + 16383;
  ଯଦି (!(D >> 63)) {
    ପ୍ରତିଜ୍ଞା(E == ScaledNumbers::MaxScale);
    AdjustedE = 0;
  }

  // Build the float and print it.
  uint64_t RawBits[2] = {D, AdjustedE};
  APFloat Float(APFloat::x87DoubleExtended(), APInt(80, RawBits));
  SmallVector<ଅକ୍ଷର, 24> Chars;
  Float.toString(Chars, Precision, 0);
  ଫେରନ୍ତୁ std::string(Chars.begin(), Chars.end());
}

ସ୍ଥିର std::string stripTrailingZeros(ଅଚଳ std::string &Float) {
  size_t NonZero = Float.find_last_not_of('0');
  ପ୍ରତିଜ୍ଞା(NonZero != std::string::npos && "no . in floating point string");

  ଯଦି (Float[NonZero] == '.')
    ++NonZero;

  ଫେରନ୍ତୁ Float.substr(0, NonZero + 1);
}

std::string Scaled_ସଂଖ୍ୟା_Base::toString(uint64_t D, int16_t E, ପୂର୍ଣ୍ଣସଂଖ୍ୟା Width,
                                       unsigned Precision) {
  ଯଦି (!D)
    ଫେରନ୍ତୁ "0.0";

  // Canonicalize exponent and digits.
  uint64_t Above0 = 0;
  uint64_t Below0 = 0;
  uint64_t Extra = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ExtraShift = 0;
  ଯଦି (E == 0) {
    Above0 = D;
  } ଅନ୍ୟଥା ଯଦି (E > 0) {
    ଯଦି (ପୂର୍ଣ୍ଣସଂଖ୍ୟା Shift = std::min(int16_t(ଗଣନା_Leading_Zeros64(D)), E)) {
      D <<= Shift;
      E -= Shift;

      ଯଦି (!E)
        Above0 = D;
    }
  } ଅନ୍ୟଥା ଯଦି (E > -64) {
    Above0 = D >> -E;
    Below0 = D << (64 + E);
  } ଅନ୍ୟଥା ଯଦି (E == -64) {
    // Special case: shift by 64 bits is undefined behavior.
    Below0 = D;
  } ଅନ୍ୟଥା ଯଦି (E > -120) {
    Below0 = D >> (-E - 64);
    Extra = D << (128 + E);
    ExtraShift = -64 - E;
  }

  // Fall back on APFloat for very small and very large numbers.
  ଯଦି (!Above0 && !Below0)
    ଫେରନ୍ତୁ toStringAPFloat(D, E, Precision);

  // Append the digits before the decimal.
  std::string Str;
  size_t DigitsOut = 0;
  ଯଦି (Above0) {
    append_ସଂଖ୍ୟା(Str, Above0);
    DigitsOut = Str.size();
  } ଅନ୍ୟଥା {
    appendDigit(Str, 0);
  }
  std::reverse(Str.begin(), Str.end());

  // Return early if there's nothing after the decimal.
  ଯଦି (!Below0)
    ଫେରନ୍ତୁ Str + ".0";

  // Append the decimal and beyond.
  Str += '.';
  uint64_t Error = UINT64_C(1) << (64 - Width);

  // We need to shift Below0 to the right to make space for calculating
  // digits.  Save the precision we're losing in Extra.
  Extra = (Below0 & 0xf) << 56 | (Extra >> 8);
  Below0 >>= 4;
  size_t SinceDot = 0;
  size_t AfterDot = Str.size();
  କର {
    ଯଦି (ExtraShift) {
      --ExtraShift;
      Error *= 5;
    } ଅନ୍ୟଥା {
      Error *= 10;
    }

    Below0 *= 10;
    Extra *= 10;
    Below0 += (Extra >> 60);
    Extra = Extra & (UINT64_MAX >> 4);
    appendDigit(Str, Below0 >> 60);
    Below0 = Below0 & (UINT64_MAX >> 4);
    ଯଦି (DigitsOut || Str.back() != '0')
      ++DigitsOut;
    ++SinceDot;
  } ଯେତେବେଳେ (Error && (Below0 << 4 | Extra >> 60) >= Error / 2 &&
           (!Precision || DigitsOut <= Precision || SinceDot < 2));

  // Return early for maximum precision.
  ଯଦି (!Precision || DigitsOut <= Precision)
    ଫେରନ୍ତୁ stripTrailingZeros(Str);

  // Find where to truncate.
  size_t Truncate =
      std::max(Str.size() - (DigitsOut - Precision), AfterDot + 1);

  // Check if there's anything to truncate.
  ଯଦି (Truncate >= Str.size())
    ଫେରନ୍ତୁ stripTrailingZeros(Str);

  ତାର୍କିକ Carry = doesRoundUp(Str[Truncate]);
  ଯଦି (!Carry)
    ଫେରନ୍ତୁ stripTrailingZeros(Str.substr(0, Truncate));

  // Round with the first truncated digit.
  ପାଇଁ (std::string::reverse_iterator I(Str.begin() + Truncate), E = Str.rend();
       I != E; ++I) {
    ଯଦି (*I == '.')
      ଜାରି;
    ଯଦି (*I == '9') {
      *I = '0';
      ଜାରି;
    }

    ++*I;
    Carry = ମିଥ୍ୟା;
    ଭାଙ୍ଗ;
  }

  // Add "1" in front if we still need to carry.
  ଫେରନ୍ତୁ stripTrailingZeros(std::string(Carry, '1') + Str.substr(0, Truncate));
}

raw_ostream &Scaled_ସଂଖ୍ୟା_Base::print(raw_ostream &OS, uint64_t D, int16_t E,
                                     ପୂର୍ଣ୍ଣସଂଖ୍ୟା Width, unsigned Precision) {
  ଫେରନ୍ତୁ OS << toString(D, E, Width, Precision);
}

ଶୂନ୍ୟ Scaled_ସଂଖ୍ୟା_Base::dump(uint64_t D, int16_t E, ପୂର୍ଣ୍ଣସଂଖ୍ୟା Width) {
  print(dbgs(), D, E, Width, 0) << "[" << Width << ":" << D << "*2^" << E
                                << "]";
}
