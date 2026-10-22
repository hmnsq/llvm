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

استفاده نطاق llvm;
استفاده نطاق llvm::ScaledNumbers;

std::pair<uint64_t, int16_t> ScaledNumbers::multiply64(uint64_t LHS,
                                                       uint64_t RHS) {
  uint64_t فوقانی_2, پایینی_2;
#إذا defined(__SIZEOF_INT128__) ||                                              \
    (defined(INTEGRAL_MAX_بیتها) && INTEGRAL_MAX_بیتها >= 128)
  خودکار حاصلضرب = __uint128_t(LHS) * RHS;
  فوقانی_2 = uint64_t(حاصلضرب >> 64);
  پایینی_2 = uint64_t(حاصلضرب);
#وإلا
  // Separate into two 32-bit digits (U.L).
  خودکار getU = [](uint64_t N) { أرجع N >> 32; };
  خودکار getL = [](uint64_t N) { أرجع N & UINT32_MAX; };
  uint64_t UL = getU(LHS), LL = getL(LHS), UR = getU(RHS), LR = getL(RHS);

  // Compute cross products.
  uint64_t P1 = UL * UR, P2 = UL * LR, P3 = LL * UR, P4 = LL * LR;

  // Sum into two 64-bit digits.
  فوقانی_2 = P1;
  پایینی_2 = P4;
  خودکار جمع_کن_With_انتقال = [&](uint64_t N) {
    uint64_t New_پایینی = پایینی_2 + (getL(N) << 32);
    فوقانی_2 += getU(N) + (New_پایینی < پایینی_2);
    پایینی_2 = New_پایینی;
  };
  جمع_کن_With_انتقال(P2);
  جمع_کن_With_انتقال(P3);
#پایان_اگر

  // Check whether the upper digit is empty.
  إذا (!فوقانی_2)
    أرجع {پایینی_2, 0};

  // Shift as little as possible to maximize precision.
  بیعلامت LeadingZeros = llvm::countl_zero(فوقانی_2);
  صحيح لغزش_2 = 64 - LeadingZeros;
  إذا (LeadingZeros)
    فوقانی_2 = فوقانی_2 << LeadingZeros | پایینی_2 >> لغزش_2;
  أرجع getRounded(فوقانی_2, لغزش_2,
                    لغزش_2 && (پایینی_2 & UINT64_C(1) << (لغزش_2 - 1)));
}

ساكن uint64_t getHalf(uint64_t N) { أرجع (N >> 1) + (N & 1); }

std::pair<uint32_t, int16_t> ScaledNumbers::divide32(uint32_t Dividend,
                                                     uint32_t مقسوم_علیه_2) {
  أكد(Dividend && "expected non-zero dividend");
  أكد(مقسوم_علیه_2 && "expected non-zero divisor");

  // Use 64-bit math and canonicalize the dividend to gain precision.
  uint64_t Dividend64 = Dividend;
  صحيح لغزش_2 = 0;
  إذا (صحيح Zeros = llvm::countl_zero(Dividend64)) {
    لغزش_2 -= Zeros;
    Dividend64 <<= Zeros;
  }
  uint64_t Quotient = Dividend64 / مقسوم_علیه_2;
  uint64_t Remainder = Dividend64 % مقسوم_علیه_2;

  // If Quotient needs to be shifted, leave the rounding to getAdjusted().
  إذا (Quotient > UINT32_MAX)
    أرجع getAdjusted<uint32_t>(Quotient, لغزش_2);

  // Round based on the value of the next bit.
  أرجع getRounded<uint32_t>(Quotient, لغزش_2, Remainder >= getHalf(مقسوم_علیه_2));
}

std::pair<uint64_t, int16_t> ScaledNumbers::divide64(uint64_t Dividend,
                                                     uint64_t مقسوم_علیه_2) {
  أكد(Dividend && "expected non-zero dividend");
  أكد(مقسوم_علیه_2 && "expected non-zero divisor");

  // Minimize size of divisor.
  صحيح لغزش_2 = 0;
  إذا (صحيح Zeros = llvm::countr_zero(مقسوم_علیه_2)) {
    لغزش_2 -= Zeros;
    مقسوم_علیه_2 >>= Zeros;
  }

  // Check for powers of two.
  إذا (مقسوم_علیه_2 == 1)
    أرجع {Dividend, لغزش_2};

  // Maximize size of dividend.
  إذا (صحيح Zeros = llvm::countl_zero(Dividend)) {
    لغزش_2 -= Zeros;
    Dividend <<= Zeros;
  }

  // Start with the result of a divide.
  uint64_t Quotient = Dividend / مقسوم_علیه_2;
  Dividend %= مقسوم_علیه_2;

  // Continue building the quotient with long division.
  طالما (!(Quotient >> 63) && Dividend) {
    // Shift Dividend and check for overflow.
    منطقي Is_سرریز = Dividend >> 63;
    Dividend <<= 1;
    --لغزش_2;

    // Get the next bit of Quotient.
    Quotient <<= 1;
    إذا (Is_سرریز || مقسوم_علیه_2 <= Dividend) {
      Quotient |= 1;
      Dividend -= مقسوم_علیه_2;
    }
  }

  أرجع getRounded(Quotient, لغزش_2, Dividend >= getHalf(مقسوم_علیه_2));
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

ساكن فراغ appendDigit(std::string &Str, بیعلامت D) {
  أكد(D < 10);
  Str += '0' + D % 10;
}

ساكن فراغ append_عدد(std::string &Str, uint64_t N) {
  طالما (N) {
    appendDigit(Str, N % 10);
    N /= 10;
  }
}

ساكن منطقي does_دور_به_بالا(حرف Digit) {
  بدل (Digit) {
  حالة '5':
  حالة '6':
  حالة '7':
  حالة '8':
  حالة '9':
    أرجع درست;
  افتراضي:
    أرجع خطأ;
  }
}

ساكن std::string مقصد_String_APFloat(uint64_t D, صحيح E, بیعلامت Precision) {
  أكد(E >= ScaledNumbers::MinScale);
  أكد(E <= ScaledNumbers::MaxScale);

  // Find a new E, but don't let it increase past MaxScale.
  صحيح LeadingZeros = Scaled_عدد_پایه::شمار_Leading_Zeros64(D);
  صحيح NewE = std::min(ScaledNumbers::MaxScale, E + 63 - LeadingZeros);
  صحيح لغزش_2 = 63 - (NewE - E);
  أكد(لغزش_2 <= LeadingZeros);
  أكد(لغزش_2 == LeadingZeros || NewE == ScaledNumbers::MaxScale);
  أكد(لغزش_2 >= 0 && لغزش_2 < 64 && "undefined behavior");
  D <<= لغزش_2;
  E = NewE;

  // Check for a denormal.
  بیعلامت AdjustedE = E + 16383;
  إذا (!(D >> 63)) {
    أكد(E == ScaledNumbers::MaxScale);
    AdjustedE = 0;
  }

  // Build the float and print it.
  uint64_t Raw_بیتها[2] = {D, AdjustedE};
  APFloat Float(APFloat::x87DoubleExtended(), APInt(80, Raw_بیتها));
  Small_وکتور<حرف, 24> Chars;
  Float.مقصد_String(Chars, Precision, 0);
  أرجع std::string(Chars.begin(), Chars.انتها());
}

ساكن std::string stripTrailingZeros(ثابت std::string &Float) {
  size_t NonZero = Float.پیدا_کن_آخرین_not_از('0');
  أكد(NonZero != std::string::npos && "no . in floating point string");

  إذا (Float[NonZero] == '.')
    ++NonZero;

  أرجع Float.substr(0, NonZero + 1);
}

std::string Scaled_عدد_پایه::مقصد_String(uint64_t D, int16_t E, صحيح عرض,
                                       بیعلامت Precision) {
  إذا (!D)
    أرجع "0.0";

  // Canonicalize exponent and digits.
  uint64_t Above0 = 0;
  uint64_t Below0 = 0;
  uint64_t Extra = 0;
  صحيح Extra_لغزش = 0;
  إذا (E == 0) {
    Above0 = D;
  } وإلا إذا (E > 0) {
    إذا (صحيح لغزش_2 = std::min(int16_t(شمار_Leading_Zeros64(D)), E)) {
      D <<= لغزش_2;
      E -= لغزش_2;

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
    Extra_لغزش = -64 - E;
  }

  // Fall back on APFloat for very small and very large numbers.
  إذا (!Above0 && !Below0)
    أرجع مقصد_String_APFloat(D, E, Precision);

  // Append the digits before the decimal.
  std::string Str;
  size_t Digits_بیرون = 0;
  إذا (Above0) {
    append_عدد(Str, Above0);
    Digits_بیرون = Str.اندازه_2();
  } وإلا {
    appendDigit(Str, 0);
  }
  std::معکوس(Str.begin(), Str.انتها());

  // Return early if there's nothing after the decimal.
  إذا (!Below0)
    أرجع Str + ".0";

  // Append the decimal and beyond.
  Str += '.';
  uint64_t Error = UINT64_C(1) << (64 - عرض);

  // We need to shift Below0 to the right to make space for calculating
  // digits.  Save the precision we're losing in Extra.
  Extra = (Below0 & 0xf) << 56 | (Extra >> 8);
  Below0 >>= 4;
  size_t SinceDot = 0;
  size_t AfterDot = Str.اندازه_2();
  افعل {
    إذا (Extra_لغزش) {
      --Extra_لغزش;
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
    إذا (Digits_بیرون || Str.پشت() != '0')
      ++Digits_بیرون;
    ++SinceDot;
  } طالما (Error && (Below0 << 4 | Extra >> 60) >= Error / 2 &&
           (!Precision || Digits_بیرون <= Precision || SinceDot < 2));

  // Return early for maximum precision.
  إذا (!Precision || Digits_بیرون <= Precision)
    أرجع stripTrailingZeros(Str);

  // Find where to truncate.
  size_t Truncate =
      std::max(Str.اندازه_2() - (Digits_بیرون - Precision), AfterDot + 1);

  // Check if there's anything to truncate.
  إذا (Truncate >= Str.اندازه_2())
    أرجع stripTrailingZeros(Str);

  منطقي انتقال_2 = does_دور_به_بالا(Str[Truncate]);
  إذا (!انتقال_2)
    أرجع stripTrailingZeros(Str.substr(0, Truncate));

  // Round with the first truncated digit.
  لكل (std::string::معکوس_iterator I(Str.begin() + Truncate), E = Str.rend();
       I != E; ++I) {
    إذا (*I == '.')
      استمر;
    إذا (*I == '9') {
      *I = '0';
      استمر;
    }

    ++*I;
    انتقال_2 = خطأ;
    اكسر;
  }

  // Add "1" in front if we still need to carry.
  أرجع stripTrailingZeros(std::string(انتقال_2, '1') + Str.substr(0, Truncate));
}

raw_ostream &Scaled_عدد_پایه::print(raw_ostream &OS, uint64_t D, int16_t E,
                                     صحيح عرض, بیعلامت Precision) {
  أرجع OS << مقصد_String(D, E, عرض, Precision);
}

فراغ Scaled_عدد_پایه::dump(uint64_t D, int16_t E, صحيح عرض) {
  print(dbgs(), D, E, عرض, 0) << "[" << عرض << ":" << D << "*2^" << E
                                << "]";
}
