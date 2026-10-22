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

#અન્તર્ભાવ "llvm/Support/ScaledNumber.h"
#અન્તર્ભાવ "llvm/ADT/APFloat.h"
#અન્તર્ભાવ "llvm/ADT/ArrayRef.h"
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"

using નામાકાશ llvm;
using નામાકાશ llvm::ScaledNumbers;

std::pair<uint64_t, int16_t> ScaledNumbers::multiply64(uint64_t LHS,
                                                       uint64_t RHS) {
  uint64_t Upper, Lower;
#જો defined(__SIZEOF_INT128__) ||                                              \
    (defined(_INTEGRAL_MAX_BITS) && _INTEGRAL_MAX_BITS >= 128)
  auto Product = __uint128_t(LHS) * RHS;
  Upper = uint64_t(Product >> 64);
  Lower = uint64_t(Product);
#નહીંતર
  // Separate into two 32-bit digits (U.L).
  auto getU = [](uint64_t N) { પાછા N >> 32; };
  auto getL = [](uint64_t N) { પાછા N & UINT32_MAX; };
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
  જો (!Upper)
    પાછા {Lower, 0};

  // Shift as little as possible to maximize precision.
  unsigned LeadingZeros = llvm::countl_zero(Upper);
  પૂર્ણાંક Shift = 64 - LeadingZeros;
  જો (LeadingZeros)
    Upper = Upper << LeadingZeros | Lower >> Shift;
  પાછા getRounded(Upper, Shift,
                    Shift && (Lower & UINT64_C(1) << (Shift - 1)));
}

સ્થિર uint64_t getHalf(uint64_t N) { પાછા (N >> 1) + (N & 1); }

std::pair<uint32_t, int16_t> ScaledNumbers::divide32(uint32_t Dividend,
                                                     uint32_t ભાજક_2) {
  પ્રતિજ્ઞા(Dividend && "expected non-zero dividend");
  પ્રતિજ્ઞા(ભાજક_2 && "expected non-zero divisor");

  // Use 64-bit math and canonicalize the dividend to gain precision.
  uint64_t Dividend64 = Dividend;
  પૂર્ણાંક Shift = 0;
  જો (પૂર્ણાંક Zeros = llvm::countl_zero(Dividend64)) {
    Shift -= Zeros;
    Dividend64 <<= Zeros;
  }
  uint64_t Quotient = Dividend64 / ભાજક_2;
  uint64_t Remainder = Dividend64 % ભાજક_2;

  // If Quotient needs to be shifted, leave the rounding to getAdjusted().
  જો (Quotient > UINT32_MAX)
    પાછા getAdjusted<uint32_t>(Quotient, Shift);

  // Round based on the value of the next bit.
  પાછા getRounded<uint32_t>(Quotient, Shift, Remainder >= getHalf(ભાજક_2));
}

std::pair<uint64_t, int16_t> ScaledNumbers::divide64(uint64_t Dividend,
                                                     uint64_t ભાજક_2) {
  પ્રતિજ્ઞા(Dividend && "expected non-zero dividend");
  પ્રતિજ્ઞા(ભાજક_2 && "expected non-zero divisor");

  // Minimize size of divisor.
  પૂર્ણાંક Shift = 0;
  જો (પૂર્ણાંક Zeros = llvm::countr_zero(ભાજક_2)) {
    Shift -= Zeros;
    ભાજક_2 >>= Zeros;
  }

  // Check for powers of two.
  જો (ભાજક_2 == 1)
    પાછા {Dividend, Shift};

  // Maximize size of dividend.
  જો (પૂર્ણાંક Zeros = llvm::countl_zero(Dividend)) {
    Shift -= Zeros;
    Dividend <<= Zeros;
  }

  // Start with the result of a divide.
  uint64_t Quotient = Dividend / ભાજક_2;
  Dividend %= ભાજક_2;

  // Continue building the quotient with long division.
  જ્યારે (!(Quotient >> 63) && Dividend) {
    // Shift Dividend and check for overflow.
    તાર્કિક IsOverflow = Dividend >> 63;
    Dividend <<= 1;
    --Shift;

    // Get the next bit of Quotient.
    Quotient <<= 1;
    જો (IsOverflow || ભાજક_2 <= Dividend) {
      Quotient |= 1;
      Dividend -= ભાજક_2;
    }
  }

  પાછા getRounded(Quotient, Shift, Dividend >= getHalf(ભાજક_2));
}

પૂર્ણાંક ScaledNumbers::compareImpl(uint64_t L, uint64_t R, પૂર્ણાંક ScaleDiff) {
  પ્રતિજ્ઞા(ScaleDiff >= 0 && "wrong argument order");
  પ્રતિજ્ઞા(ScaleDiff < 64 && "numbers too far apart");

  uint64_t L_adjusted = L >> ScaleDiff;
  જો (L_adjusted < R)
    પાછા -1;
  જો (L_adjusted > R)
    પાછા 1;

  પાછા L > L_adjusted << ScaleDiff ? 1 : 0;
}

સ્થિર શૂન્ય appendDigit(std::string &Str, unsigned D) {
  પ્રતિજ્ઞા(D < 10);
  Str += '0' + D % 10;
}

સ્થિર શૂન્ય append_સંખ્યા(std::string &Str, uint64_t N) {
  જ્યારે (N) {
    appendDigit(Str, N % 10);
    N /= 10;
  }
}

સ્થિર તાર્કિક doesRoundUp(અક્ષર Digit) {
  પસંદગી (Digit) {
  સ્થિતિ '5':
  સ્થિતિ '6':
  સ્થિતિ '7':
  સ્થિતિ '8':
  સ્થિતિ '9':
    પાછા સાચું;
  મૂળભૂત:
    પાછા ખોટું;
  }
}

સ્થિર std::string toStringAPFloat(uint64_t D, પૂર્ણાંક E, unsigned Precision) {
  પ્રતિજ્ઞા(E >= ScaledNumbers::MinScale);
  પ્રતિજ્ઞા(E <= ScaledNumbers::MaxScale);

  // Find a new E, but don't let it increase past MaxScale.
  પૂર્ણાંક LeadingZeros = Scaled_સંખ્યા_Base::ગણતરી_Leading_Zeros64(D);
  પૂર્ણાંક NewE = std::min(ScaledNumbers::MaxScale, E + 63 - LeadingZeros);
  પૂર્ણાંક Shift = 63 - (NewE - E);
  પ્રતિજ્ઞા(Shift <= LeadingZeros);
  પ્રતિજ્ઞા(Shift == LeadingZeros || NewE == ScaledNumbers::MaxScale);
  પ્રતિજ્ઞા(Shift >= 0 && Shift < 64 && "undefined behavior");
  D <<= Shift;
  E = NewE;

  // Check for a denormal.
  unsigned AdjustedE = E + 16383;
  જો (!(D >> 63)) {
    પ્રતિજ્ઞા(E == ScaledNumbers::MaxScale);
    AdjustedE = 0;
  }

  // Build the float and print it.
  uint64_t RawBits[2] = {D, AdjustedE};
  APFloat Float(APFloat::x87DoubleExtended(), APInt(80, RawBits));
  SmallVector<અક્ષર, 24> Chars;
  Float.toString(Chars, Precision, 0);
  પાછા std::string(Chars.begin(), Chars.end());
}

સ્થિર std::string stripTrailingZeros(અચલ std::string &Float) {
  size_t NonZero = Float.find_last_not_of('0');
  પ્રતિજ્ઞા(NonZero != std::string::npos && "no . in floating point string");

  જો (Float[NonZero] == '.')
    ++NonZero;

  પાછા Float.substr(0, NonZero + 1);
}

std::string Scaled_સંખ્યા_Base::toString(uint64_t D, int16_t E, પૂર્ણાંક Width,
                                       unsigned Precision) {
  જો (!D)
    પાછા "0.0";

  // Canonicalize exponent and digits.
  uint64_t Above0 = 0;
  uint64_t Below0 = 0;
  uint64_t Extra = 0;
  પૂર્ણાંક ExtraShift = 0;
  જો (E == 0) {
    Above0 = D;
  } નહીંતર જો (E > 0) {
    જો (પૂર્ણાંક Shift = std::min(int16_t(ગણતરી_Leading_Zeros64(D)), E)) {
      D <<= Shift;
      E -= Shift;

      જો (!E)
        Above0 = D;
    }
  } નહીંતર જો (E > -64) {
    Above0 = D >> -E;
    Below0 = D << (64 + E);
  } નહીંતર જો (E == -64) {
    // Special case: shift by 64 bits is undefined behavior.
    Below0 = D;
  } નહીંતર જો (E > -120) {
    Below0 = D >> (-E - 64);
    Extra = D << (128 + E);
    ExtraShift = -64 - E;
  }

  // Fall back on APFloat for very small and very large numbers.
  જો (!Above0 && !Below0)
    પાછા toStringAPFloat(D, E, Precision);

  // Append the digits before the decimal.
  std::string Str;
  size_t DigitsOut = 0;
  જો (Above0) {
    append_સંખ્યા(Str, Above0);
    DigitsOut = Str.size();
  } નહીંતર {
    appendDigit(Str, 0);
  }
  std::reverse(Str.begin(), Str.end());

  // Return early if there's nothing after the decimal.
  જો (!Below0)
    પાછા Str + ".0";

  // Append the decimal and beyond.
  Str += '.';
  uint64_t Error = UINT64_C(1) << (64 - Width);

  // We need to shift Below0 to the right to make space for calculating
  // digits.  Save the precision we're losing in Extra.
  Extra = (Below0 & 0xf) << 56 | (Extra >> 8);
  Below0 >>= 4;
  size_t SinceDot = 0;
  size_t AfterDot = Str.size();
  કરો {
    જો (ExtraShift) {
      --ExtraShift;
      Error *= 5;
    } નહીંતર {
      Error *= 10;
    }

    Below0 *= 10;
    Extra *= 10;
    Below0 += (Extra >> 60);
    Extra = Extra & (UINT64_MAX >> 4);
    appendDigit(Str, Below0 >> 60);
    Below0 = Below0 & (UINT64_MAX >> 4);
    જો (DigitsOut || Str.back() != '0')
      ++DigitsOut;
    ++SinceDot;
  } જ્યારે (Error && (Below0 << 4 | Extra >> 60) >= Error / 2 &&
           (!Precision || DigitsOut <= Precision || SinceDot < 2));

  // Return early for maximum precision.
  જો (!Precision || DigitsOut <= Precision)
    પાછા stripTrailingZeros(Str);

  // Find where to truncate.
  size_t Truncate =
      std::max(Str.size() - (DigitsOut - Precision), AfterDot + 1);

  // Check if there's anything to truncate.
  જો (Truncate >= Str.size())
    પાછા stripTrailingZeros(Str);

  તાર્કિક Carry = doesRoundUp(Str[Truncate]);
  જો (!Carry)
    પાછા stripTrailingZeros(Str.substr(0, Truncate));

  // Round with the first truncated digit.
  માટે (std::string::reverse_iterator I(Str.begin() + Truncate), E = Str.rend();
       I != E; ++I) {
    જો (*I == '.')
      ચાલુ;
    જો (*I == '9') {
      *I = '0';
      ચાલુ;
    }

    ++*I;
    Carry = ખોટું;
    તોડો;
  }

  // Add "1" in front if we still need to carry.
  પાછા stripTrailingZeros(std::string(Carry, '1') + Str.substr(0, Truncate));
}

raw_ostream &Scaled_સંખ્યા_Base::print(raw_ostream &OS, uint64_t D, int16_t E,
                                     પૂર્ણાંક Width, unsigned Precision) {
  પાછા OS << toString(D, E, Width, Precision);
}

શૂન્ય Scaled_સંખ્યા_Base::dump(uint64_t D, int16_t E, પૂર્ણાંક Width) {
  print(dbgs(), D, E, Width, 0) << "[" << Width << ":" << D << "*2^" << E
                                << "]";
}
