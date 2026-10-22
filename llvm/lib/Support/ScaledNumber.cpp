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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ScaledNumber.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/APFloat.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ArrayRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::ScaledNumbers;

std::pair<uint64_t, int16_t> ScaledNumbers::multiply64(uint64_t LHS,
                                                       uint64_t RHS) {
  uint64_t ਉੱਤਲਾ_2, ਹੇਠਲਾ_2;
#ਜੇ defined(__SIZEOF_INT128__) ||                                              \
    (defined(INTEGRAL_MAX_ਬਿੱਟ) && INTEGRAL_MAX_ਬਿੱਟ >= 128)
  ਸਵੈਚਾਲਿਤ ਗੁਣਨਫਲ = __uint128_t(LHS) * RHS;
  ਉੱਤਲਾ_2 = uint64_t(ਗੁਣਨਫਲ >> 64);
  ਹੇਠਲਾ_2 = uint64_t(ਗੁਣਨਫਲ);
#ਨਹੀਂਤਾਂ
  // Separate into two 32-bit digits (U.L).
  ਸਵੈਚਾਲਿਤ getU = [](uint64_t N) { ਵਾਪਸ N >> 32; };
  ਸਵੈਚਾਲਿਤ getL = [](uint64_t N) { ਵਾਪਸ N & UINT32_MAX; };
  uint64_t UL = getU(LHS), LL = getL(LHS), UR = getU(RHS), LR = getL(RHS);

  // Compute cross products.
  uint64_t P1 = UL * UR, P2 = UL * LR, P3 = LL * UR, P4 = LL * LR;

  // Sum into two 64-bit digits.
  ਉੱਤਲਾ_2 = P1;
  ਹੇਠਲਾ_2 = P4;
  ਸਵੈਚਾਲਿਤ ਜੋੜੋ_With_ਹਾਸਲ = [&](uint64_t N) {
    uint64_t New_ਹੇਠਲਾ = ਹੇਠਲਾ_2 + (getL(N) << 32);
    ਉੱਤਲਾ_2 += getU(N) + (New_ਹੇਠਲਾ < ਹੇਠਲਾ_2);
    ਹੇਠਲਾ_2 = New_ਹੇਠਲਾ;
  };
  ਜੋੜੋ_With_ਹਾਸਲ(P2);
  ਜੋੜੋ_With_ਹਾਸਲ(P3);
#ਜੇ_ਅੰਤ

  // Check whether the upper digit is empty.
  ਜੇ (!ਉੱਤਲਾ_2)
    ਵਾਪਸ {ਹੇਠਲਾ_2, 0};

  // Shift as little as possible to maximize precision.
  ਬਿਨਾਨਿਸ਼ਾਨੀ LeadingZeros = llvm::countl_zero(ਉੱਤਲਾ_2);
  ਪੂਰਨ ਖਿਸਕਾਓ_2 = 64 - LeadingZeros;
  ਜੇ (LeadingZeros)
    ਉੱਤਲਾ_2 = ਉੱਤਲਾ_2 << LeadingZeros | ਹੇਠਲਾ_2 >> ਖਿਸਕਾਓ_2;
  ਵਾਪਸ getRounded(ਉੱਤਲਾ_2, ਖਿਸਕਾਓ_2,
                    ਖਿਸਕਾਓ_2 && (ਹੇਠਲਾ_2 & UINT64_C(1) << (ਖਿਸਕਾਓ_2 - 1)));
}

ਸਥਿਰ uint64_t getHalf(uint64_t N) { ਵਾਪਸ (N >> 1) + (N & 1); }

std::pair<uint32_t, int16_t> ScaledNumbers::divide32(uint32_t Dividend,
                                                     uint32_t ਭਾਜਕ_2) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Dividend && "expected non-zero dividend");
  ਪ੍ਰਤਿਜ੍ਞਾ(ਭਾਜਕ_2 && "expected non-zero divisor");

  // Use 64-bit math and canonicalize the dividend to gain precision.
  uint64_t Dividend64 = Dividend;
  ਪੂਰਨ ਖਿਸਕਾਓ_2 = 0;
  ਜੇ (ਪੂਰਨ Zeros = llvm::countl_zero(Dividend64)) {
    ਖਿਸਕਾਓ_2 -= Zeros;
    Dividend64 <<= Zeros;
  }
  uint64_t Quotient = Dividend64 / ਭਾਜਕ_2;
  uint64_t Remainder = Dividend64 % ਭਾਜਕ_2;

  // If Quotient needs to be shifted, leave the rounding to getAdjusted().
  ਜੇ (Quotient > UINT32_MAX)
    ਵਾਪਸ getAdjusted<uint32_t>(Quotient, ਖਿਸਕਾਓ_2);

  // Round based on the value of the next bit.
  ਵਾਪਸ getRounded<uint32_t>(Quotient, ਖਿਸਕਾਓ_2, Remainder >= getHalf(ਭਾਜਕ_2));
}

std::pair<uint64_t, int16_t> ScaledNumbers::divide64(uint64_t Dividend,
                                                     uint64_t ਭਾਜਕ_2) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Dividend && "expected non-zero dividend");
  ਪ੍ਰਤਿਜ੍ਞਾ(ਭਾਜਕ_2 && "expected non-zero divisor");

  // Minimize size of divisor.
  ਪੂਰਨ ਖਿਸਕਾਓ_2 = 0;
  ਜੇ (ਪੂਰਨ Zeros = llvm::countr_zero(ਭਾਜਕ_2)) {
    ਖਿਸਕਾਓ_2 -= Zeros;
    ਭਾਜਕ_2 >>= Zeros;
  }

  // Check for powers of two.
  ਜੇ (ਭਾਜਕ_2 == 1)
    ਵਾਪਸ {Dividend, ਖਿਸਕਾਓ_2};

  // Maximize size of dividend.
  ਜੇ (ਪੂਰਨ Zeros = llvm::countl_zero(Dividend)) {
    ਖਿਸਕਾਓ_2 -= Zeros;
    Dividend <<= Zeros;
  }

  // Start with the result of a divide.
  uint64_t Quotient = Dividend / ਭਾਜਕ_2;
  Dividend %= ਭਾਜਕ_2;

  // Continue building the quotient with long division.
  ਜਦੋਂ (!(Quotient >> 63) && Dividend) {
    // Shift Dividend and check for overflow.
    ਤਾਰਕਿਕ Is_ਉਛਾਲ = Dividend >> 63;
    Dividend <<= 1;
    --ਖਿਸਕਾਓ_2;

    // Get the next bit of Quotient.
    Quotient <<= 1;
    ਜੇ (Is_ਉਛਾਲ || ਭਾਜਕ_2 <= Dividend) {
      Quotient |= 1;
      Dividend -= ਭਾਜਕ_2;
    }
  }

  ਵਾਪਸ getRounded(Quotient, ਖਿਸਕਾਓ_2, Dividend >= getHalf(ਭਾਜਕ_2));
}

ਪੂਰਨ ScaledNumbers::compareImpl(uint64_t L, uint64_t R, ਪੂਰਨ ScaleDiff) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ScaleDiff >= 0 && "wrong argument order");
  ਪ੍ਰਤਿਜ੍ਞਾ(ScaleDiff < 64 && "numbers too far apart");

  uint64_t L_adjusted = L >> ScaleDiff;
  ਜੇ (L_adjusted < R)
    ਵਾਪਸ -1;
  ਜੇ (L_adjusted > R)
    ਵਾਪਸ 1;

  ਵਾਪਸ L > L_adjusted << ScaleDiff ? 1 : 0;
}

ਸਥਿਰ ਖਾਲੀ appendDigit(std::string &Str, ਬਿਨਾਨਿਸ਼ਾਨੀ D) {
  ਪ੍ਰਤਿਜ੍ਞਾ(D < 10);
  Str += '0' + D % 10;
}

ਸਥਿਰ ਖਾਲੀ append_ਸੰਖਿਆ(std::string &Str, uint64_t N) {
  ਜਦੋਂ (N) {
    appendDigit(Str, N % 10);
    N /= 10;
  }
}

ਸਥਿਰ ਤਾਰਕਿਕ does_ਗੇੜਾ_ਉੱਪਰ(ਅੱਖਰ Digit) {
  ਚੋਣ (Digit) {
  ਹਾਲਤ '5':
  ਹਾਲਤ '6':
  ਹਾਲਤ '7':
  ਹਾਲਤ '8':
  ਹਾਲਤ '9':
    ਵਾਪਸ ਸੱਚ;
  ਮੂਲ:
    ਵਾਪਸ ਝੂਠ;
  }
}

ਸਥਿਰ std::string ਮੰਜ਼ਿਲ_String_APFloat(uint64_t D, ਪੂਰਨ E, ਬਿਨਾਨਿਸ਼ਾਨੀ Precision) {
  ਪ੍ਰਤਿਜ੍ਞਾ(E >= ScaledNumbers::MinScale);
  ਪ੍ਰਤਿਜ੍ਞਾ(E <= ScaledNumbers::MaxScale);

  // Find a new E, but don't let it increase past MaxScale.
  ਪੂਰਨ LeadingZeros = Scaled_ਸੰਖਿਆ_ਅਧਾਰ::ਗਿਣਤੀ_Leading_Zeros64(D);
  ਪੂਰਨ NewE = std::min(ScaledNumbers::MaxScale, E + 63 - LeadingZeros);
  ਪੂਰਨ ਖਿਸਕਾਓ_2 = 63 - (NewE - E);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਖਿਸਕਾਓ_2 <= LeadingZeros);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਖਿਸਕਾਓ_2 == LeadingZeros || NewE == ScaledNumbers::MaxScale);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਖਿਸਕਾਓ_2 >= 0 && ਖਿਸਕਾਓ_2 < 64 && "undefined behavior");
  D <<= ਖਿਸਕਾਓ_2;
  E = NewE;

  // Check for a denormal.
  ਬਿਨਾਨਿਸ਼ਾਨੀ AdjustedE = E + 16383;
  ਜੇ (!(D >> 63)) {
    ਪ੍ਰਤਿਜ੍ਞਾ(E == ScaledNumbers::MaxScale);
    AdjustedE = 0;
  }

  // Build the float and print it.
  uint64_t Raw_ਬਿੱਟ[2] = {D, AdjustedE};
  APFloat Float(APFloat::x87DoubleExtended(), APInt(80, Raw_ਬਿੱਟ));
  Small_ਸਦਿਸ਼<ਅੱਖਰ, 24> Chars;
  Float.ਮੰਜ਼ਿਲ_String(Chars, Precision, 0);
  ਵਾਪਸ std::string(Chars.begin(), Chars.ਸਿਰਾ());
}

ਸਥਿਰ std::string stripTrailingZeros(ਅਚਲ std::string &Float) {
  size_t NonZero = Float.ਲੱਭੋ_ਆਖਰੀ_not_ਦਾ('0');
  ਪ੍ਰਤਿਜ੍ਞਾ(NonZero != std::string::npos && "no . in floating point string");

  ਜੇ (Float[NonZero] == '.')
    ++NonZero;

  ਵਾਪਸ Float.substr(0, NonZero + 1);
}

std::string Scaled_ਸੰਖਿਆ_ਅਧਾਰ::ਮੰਜ਼ਿਲ_String(uint64_t D, int16_t E, ਪੂਰਨ ਚੌੜਾਈ,
                                       ਬਿਨਾਨਿਸ਼ਾਨੀ Precision) {
  ਜੇ (!D)
    ਵਾਪਸ "0.0";

  // Canonicalize exponent and digits.
  uint64_t Above0 = 0;
  uint64_t Below0 = 0;
  uint64_t Extra = 0;
  ਪੂਰਨ Extra_ਖਿਸਕਾਓ = 0;
  ਜੇ (E == 0) {
    Above0 = D;
  } ਨਹੀਂਤਾਂ ਜੇ (E > 0) {
    ਜੇ (ਪੂਰਨ ਖਿਸਕਾਓ_2 = std::min(int16_t(ਗਿਣਤੀ_Leading_Zeros64(D)), E)) {
      D <<= ਖਿਸਕਾਓ_2;
      E -= ਖਿਸਕਾਓ_2;

      ਜੇ (!E)
        Above0 = D;
    }
  } ਨਹੀਂਤਾਂ ਜੇ (E > -64) {
    Above0 = D >> -E;
    Below0 = D << (64 + E);
  } ਨਹੀਂਤਾਂ ਜੇ (E == -64) {
    // Special case: shift by 64 bits is undefined behavior.
    Below0 = D;
  } ਨਹੀਂਤਾਂ ਜੇ (E > -120) {
    Below0 = D >> (-E - 64);
    Extra = D << (128 + E);
    Extra_ਖਿਸਕਾਓ = -64 - E;
  }

  // Fall back on APFloat for very small and very large numbers.
  ਜੇ (!Above0 && !Below0)
    ਵਾਪਸ ਮੰਜ਼ਿਲ_String_APFloat(D, E, Precision);

  // Append the digits before the decimal.
  std::string Str;
  size_t Digits_ਬਾਹਰਲਾ = 0;
  ਜੇ (Above0) {
    append_ਸੰਖਿਆ(Str, Above0);
    Digits_ਬਾਹਰਲਾ = Str.ਮਾਪ_2();
  } ਨਹੀਂਤਾਂ {
    appendDigit(Str, 0);
  }
  std::ਉਲਟ(Str.begin(), Str.ਸਿਰਾ());

  // Return early if there's nothing after the decimal.
  ਜੇ (!Below0)
    ਵਾਪਸ Str + ".0";

  // Append the decimal and beyond.
  Str += '.';
  uint64_t Error = UINT64_C(1) << (64 - ਚੌੜਾਈ);

  // We need to shift Below0 to the right to make space for calculating
  // digits.  Save the precision we're losing in Extra.
  Extra = (Below0 & 0xf) << 56 | (Extra >> 8);
  Below0 >>= 4;
  size_t SinceDot = 0;
  size_t AfterDot = Str.ਮਾਪ_2();
  ਕਰੋ {
    ਜੇ (Extra_ਖਿਸਕਾਓ) {
      --Extra_ਖਿਸਕਾਓ;
      Error *= 5;
    } ਨਹੀਂਤਾਂ {
      Error *= 10;
    }

    Below0 *= 10;
    Extra *= 10;
    Below0 += (Extra >> 60);
    Extra = Extra & (UINT64_MAX >> 4);
    appendDigit(Str, Below0 >> 60);
    Below0 = Below0 & (UINT64_MAX >> 4);
    ਜੇ (Digits_ਬਾਹਰਲਾ || Str.ਪਿੱਠ() != '0')
      ++Digits_ਬਾਹਰਲਾ;
    ++SinceDot;
  } ਜਦੋਂ (Error && (Below0 << 4 | Extra >> 60) >= Error / 2 &&
           (!Precision || Digits_ਬਾਹਰਲਾ <= Precision || SinceDot < 2));

  // Return early for maximum precision.
  ਜੇ (!Precision || Digits_ਬਾਹਰਲਾ <= Precision)
    ਵਾਪਸ stripTrailingZeros(Str);

  // Find where to truncate.
  size_t Truncate =
      std::max(Str.ਮਾਪ_2() - (Digits_ਬਾਹਰਲਾ - Precision), AfterDot + 1);

  // Check if there's anything to truncate.
  ਜੇ (Truncate >= Str.ਮਾਪ_2())
    ਵਾਪਸ stripTrailingZeros(Str);

  ਤਾਰਕਿਕ ਹਾਸਲ_2 = does_ਗੇੜਾ_ਉੱਪਰ(Str[Truncate]);
  ਜੇ (!ਹਾਸਲ_2)
    ਵਾਪਸ stripTrailingZeros(Str.substr(0, Truncate));

  // Round with the first truncated digit.
  ਲਈ (std::string::ਉਲਟ_iterator I(Str.begin() + Truncate), E = Str.rend();
       I != E; ++I) {
    ਜੇ (*I == '.')
      ਜਾਰੀ;
    ਜੇ (*I == '9') {
      *I = '0';
      ਜਾਰੀ;
    }

    ++*I;
    ਹਾਸਲ_2 = ਝੂਠ;
    ਰੋਕੋ;
  }

  // Add "1" in front if we still need to carry.
  ਵਾਪਸ stripTrailingZeros(std::string(ਹਾਸਲ_2, '1') + Str.substr(0, Truncate));
}

raw_ostream &Scaled_ਸੰਖਿਆ_ਅਧਾਰ::print(raw_ostream &OS, uint64_t D, int16_t E,
                                     ਪੂਰਨ ਚੌੜਾਈ, ਬਿਨਾਨਿਸ਼ਾਨੀ Precision) {
  ਵਾਪਸ OS << ਮੰਜ਼ਿਲ_String(D, E, ਚੌੜਾਈ, Precision);
}

ਖਾਲੀ Scaled_ਸੰਖਿਆ_ਅਧਾਰ::dump(uint64_t D, int16_t E, ਪੂਰਨ ਚੌੜਾਈ) {
  print(dbgs(), D, E, ਚੌੜਾਈ, 0) << "[" << ਚੌੜਾਈ << ":" << D << "*2^" << E
                                << "]";
}
