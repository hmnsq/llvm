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

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::ScaledNumbers;

std::pair<uint64_t, int16_t> ScaledNumbers::multiply64(uint64_t LHS,
                                                       uint64_t RHS) {
  uint64_t ऊर्ध्वस्थम्_2, अधस्तनम्_2;
#यदि defined(__SIZEOF_INT128__) ||                                              \
    (defined(INTEGRAL_MAX_द्वयङ्काः) && INTEGRAL_MAX_द्वयङ्काः >= 128)
  स्वचालित गुणनफलम् = __uint128_t(LHS) * RHS;
  ऊर्ध्वस्थम्_2 = uint64_t(गुणनफलम् >> 64);
  अधस्तनम्_2 = uint64_t(गुणनफलम्);
#अन्यथा
  // Separate into two 32-bit digits (U.L).
  स्वचालित getU = [](uint64_t N) { प्रत्यागम N >> 32; };
  स्वचालित getL = [](uint64_t N) { प्रत्यागम N & UINT32_MAX; };
  uint64_t UL = getU(LHS), LL = getL(LHS), UR = getU(RHS), LR = getL(RHS);

  // Compute cross products.
  uint64_t P1 = UL * UR, P2 = UL * LR, P3 = LL * UR, P4 = LL * LR;

  // Sum into two 64-bit digits.
  ऊर्ध्वस्थम्_2 = P1;
  अधस्तनम्_2 = P4;
  स्वचालित सङ्कलय_With_Carry = [&](uint64_t N) {
    uint64_t New_अधस्तनम् = अधस्तनम्_2 + (getL(N) << 32);
    ऊर्ध्वस्थम्_2 += getU(N) + (New_अधस्तनम् < अधस्तनम्_2);
    अधस्तनम्_2 = New_अधस्तनम्;
  };
  सङ्कलय_With_Carry(P2);
  सङ्कलय_With_Carry(P3);
#यदि_अन्तः

  // Check whether the upper digit is empty.
  यदि (!ऊर्ध्वस्थम्_2)
    प्रत्यागम {अधस्तनम्_2, 0};

  // Shift as little as possible to maximize precision.
  अचिह्नित LeadingZeros = llvm::countl_zero(ऊर्ध्वस्थम्_2);
  पूर्णाङ्क सरणम्_2 = 64 - LeadingZeros;
  यदि (LeadingZeros)
    ऊर्ध्वस्थम्_2 = ऊर्ध्वस्थम्_2 << LeadingZeros | अधस्तनम्_2 >> सरणम्_2;
  प्रत्यागम getRounded(ऊर्ध्वस्थम्_2, सरणम्_2,
                    सरणम्_2 && (अधस्तनम्_2 & UINT64_C(1) << (सरणम्_2 - 1)));
}

स्थिर uint64_t getHalf(uint64_t N) { प्रत्यागम (N >> 1) + (N & 1); }

std::pair<uint32_t, int16_t> ScaledNumbers::divide32(uint32_t Dividend,
                                                     uint32_t भाजकः_2) {
  प्रतिज्ञा(Dividend && "expected non-zero dividend");
  प्रतिज्ञा(भाजकः_2 && "expected non-zero divisor");

  // Use 64-bit math and canonicalize the dividend to gain precision.
  uint64_t Dividend64 = Dividend;
  पूर्णाङ्क सरणम्_2 = 0;
  यदि (पूर्णाङ्क Zeros = llvm::countl_zero(Dividend64)) {
    सरणम्_2 -= Zeros;
    Dividend64 <<= Zeros;
  }
  uint64_t Quotient = Dividend64 / भाजकः_2;
  uint64_t Remainder = Dividend64 % भाजकः_2;

  // If Quotient needs to be shifted, leave the rounding to getAdjusted().
  यदि (Quotient > UINT32_MAX)
    प्रत्यागम getAdjusted<uint32_t>(Quotient, सरणम्_2);

  // Round based on the value of the next bit.
  प्रत्यागम getRounded<uint32_t>(Quotient, सरणम्_2, Remainder >= getHalf(भाजकः_2));
}

std::pair<uint64_t, int16_t> ScaledNumbers::divide64(uint64_t Dividend,
                                                     uint64_t भाजकः_2) {
  प्रतिज्ञा(Dividend && "expected non-zero dividend");
  प्रतिज्ञा(भाजकः_2 && "expected non-zero divisor");

  // Minimize size of divisor.
  पूर्णाङ्क सरणम्_2 = 0;
  यदि (पूर्णाङ्क Zeros = llvm::countr_zero(भाजकः_2)) {
    सरणम्_2 -= Zeros;
    भाजकः_2 >>= Zeros;
  }

  // Check for powers of two.
  यदि (भाजकः_2 == 1)
    प्रत्यागम {Dividend, सरणम्_2};

  // Maximize size of dividend.
  यदि (पूर्णाङ्क Zeros = llvm::countl_zero(Dividend)) {
    सरणम्_2 -= Zeros;
    Dividend <<= Zeros;
  }

  // Start with the result of a divide.
  uint64_t Quotient = Dividend / भाजकः_2;
  Dividend %= भाजकः_2;

  // Continue building the quotient with long division.
  यावत् (!(Quotient >> 63) && Dividend) {
    // Shift Dividend and check for overflow.
    तार्किक Is_अतिप्रवाहः = Dividend >> 63;
    Dividend <<= 1;
    --सरणम्_2;

    // Get the next bit of Quotient.
    Quotient <<= 1;
    यदि (Is_अतिप्रवाहः || भाजकः_2 <= Dividend) {
      Quotient |= 1;
      Dividend -= भाजकः_2;
    }
  }

  प्रत्यागम getRounded(Quotient, सरणम्_2, Dividend >= getHalf(भाजकः_2));
}

पूर्णाङ्क ScaledNumbers::compareImpl(uint64_t L, uint64_t R, पूर्णाङ्क ScaleDiff) {
  प्रतिज्ञा(ScaleDiff >= 0 && "wrong argument order");
  प्रतिज्ञा(ScaleDiff < 64 && "numbers too far apart");

  uint64_t L_adjusted = L >> ScaleDiff;
  यदि (L_adjusted < R)
    प्रत्यागम -1;
  यदि (L_adjusted > R)
    प्रत्यागम 1;

  प्रत्यागम L > L_adjusted << ScaleDiff ? 1 : 0;
}

स्थिर शून्य appendDigit(std::string &Str, अचिह्नित D) {
  प्रतिज्ञा(D < 10);
  Str += '0' + D % 10;
}

स्थिर शून्य append_सङ्ख्या(std::string &Str, uint64_t N) {
  यावत् (N) {
    appendDigit(Str, N % 10);
    N /= 10;
  }
}

स्थिर तार्किक does_आवर्तनम्_ऊर्ध्वम्(वर्ण Digit) {
  विकल्प (Digit) {
  पक्ष '5':
  पक्ष '6':
  पक्ष '7':
  पक्ष '8':
  पक्ष '9':
    प्रत्यागम सत्य;
  स्वभाव:
    प्रत्यागम असत्य;
  }
}

स्थिर std::string गन्तव्यम्_String_APFloat(uint64_t D, पूर्णाङ्क E, अचिह्नित Precision) {
  प्रतिज्ञा(E >= ScaledNumbers::MinScale);
  प्रतिज्ञा(E <= ScaledNumbers::MaxScale);

  // Find a new E, but don't let it increase past MaxScale.
  पूर्णाङ्क LeadingZeros = Scaled_सङ्ख्या_आधारः::गणनम्_Leading_Zeros64(D);
  पूर्णाङ्क NewE = std::min(ScaledNumbers::MaxScale, E + 63 - LeadingZeros);
  पूर्णाङ्क सरणम्_2 = 63 - (NewE - E);
  प्रतिज्ञा(सरणम्_2 <= LeadingZeros);
  प्रतिज्ञा(सरणम्_2 == LeadingZeros || NewE == ScaledNumbers::MaxScale);
  प्रतिज्ञा(सरणम्_2 >= 0 && सरणम्_2 < 64 && "undefined behavior");
  D <<= सरणम्_2;
  E = NewE;

  // Check for a denormal.
  अचिह्नित AdjustedE = E + 16383;
  यदि (!(D >> 63)) {
    प्रतिज्ञा(E == ScaledNumbers::MaxScale);
    AdjustedE = 0;
  }

  // Build the float and print it.
  uint64_t Raw_द्वयङ्काः[2] = {D, AdjustedE};
  APFloat Float(APFloat::x87DoubleExtended(), APInt(80, Raw_द्वयङ्काः));
  Small_सदिशम्<वर्ण, 24> Chars;
  Float.गन्तव्यम्_String(Chars, Precision, 0);
  प्रत्यागम std::string(Chars.begin(), Chars.अन्तः());
}

स्थिर std::string stripTrailingZeros(ध्रुव std::string &Float) {
  size_t NonZero = Float.find_चरमः_not_सम्बन्धि('0');
  प्रतिज्ञा(NonZero != std::string::npos && "no . in floating point string");

  यदि (Float[NonZero] == '.')
    ++NonZero;

  प्रत्यागम Float.substr(0, NonZero + 1);
}

std::string Scaled_सङ्ख्या_आधारः::गन्तव्यम्_String(uint64_t D, int16_t E, पूर्णाङ्क विस्तारः,
                                       अचिह्नित Precision) {
  यदि (!D)
    प्रत्यागम "0.0";

  // Canonicalize exponent and digits.
  uint64_t Above0 = 0;
  uint64_t Below0 = 0;
  uint64_t Extra = 0;
  पूर्णाङ्क Extra_सरणम् = 0;
  यदि (E == 0) {
    Above0 = D;
  } अन्यथा यदि (E > 0) {
    यदि (पूर्णाङ्क सरणम्_2 = std::min(int16_t(गणनम्_Leading_Zeros64(D)), E)) {
      D <<= सरणम्_2;
      E -= सरणम्_2;

      यदि (!E)
        Above0 = D;
    }
  } अन्यथा यदि (E > -64) {
    Above0 = D >> -E;
    Below0 = D << (64 + E);
  } अन्यथा यदि (E == -64) {
    // Special case: shift by 64 bits is undefined behavior.
    Below0 = D;
  } अन्यथा यदि (E > -120) {
    Below0 = D >> (-E - 64);
    Extra = D << (128 + E);
    Extra_सरणम् = -64 - E;
  }

  // Fall back on APFloat for very small and very large numbers.
  यदि (!Above0 && !Below0)
    प्रत्यागम गन्तव्यम्_String_APFloat(D, E, Precision);

  // Append the digits before the decimal.
  std::string Str;
  size_t Digits_बहिर्गमनम् = 0;
  यदि (Above0) {
    append_सङ्ख्या(Str, Above0);
    Digits_बहिर्गमनम् = Str.आकार_2();
  } अन्यथा {
    appendDigit(Str, 0);
  }
  std::reverse(Str.begin(), Str.अन्तः());

  // Return early if there's nothing after the decimal.
  यदि (!Below0)
    प्रत्यागम Str + ".0";

  // Append the decimal and beyond.
  Str += '.';
  uint64_t Error = UINT64_C(1) << (64 - विस्तारः);

  // We need to shift Below0 to the right to make space for calculating
  // digits.  Save the precision we're losing in Extra.
  Extra = (Below0 & 0xf) << 56 | (Extra >> 8);
  Below0 >>= 4;
  size_t SinceDot = 0;
  size_t AfterDot = Str.आकार_2();
  कुरु {
    यदि (Extra_सरणम्) {
      --Extra_सरणम्;
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
    यदि (Digits_बहिर्गमनम् || Str.पृष्ठभागः() != '0')
      ++Digits_बहिर्गमनम्;
    ++SinceDot;
  } यावत् (Error && (Below0 << 4 | Extra >> 60) >= Error / 2 &&
           (!Precision || Digits_बहिर्गमनम् <= Precision || SinceDot < 2));

  // Return early for maximum precision.
  यदि (!Precision || Digits_बहिर्गमनम् <= Precision)
    प्रत्यागम stripTrailingZeros(Str);

  // Find where to truncate.
  size_t Truncate =
      std::max(Str.आकार_2() - (Digits_बहिर्गमनम् - Precision), AfterDot + 1);

  // Check if there's anything to truncate.
  यदि (Truncate >= Str.आकार_2())
    प्रत्यागम stripTrailingZeros(Str);

  तार्किक Carry = does_आवर्तनम्_ऊर्ध्वम्(Str[Truncate]);
  यदि (!Carry)
    प्रत्यागम stripTrailingZeros(Str.substr(0, Truncate));

  // Round with the first truncated digit.
  प्रति (std::string::reverse_iterator I(Str.begin() + Truncate), E = Str.rend();
       I != E; ++I) {
    यदि (*I == '.')
      अनुवर्त;
    यदि (*I == '9') {
      *I = '0';
      अनुवर्त;
    }

    ++*I;
    Carry = असत्य;
    भङ्ग;
  }

  // Add "1" in front if we still need to carry.
  प्रत्यागम stripTrailingZeros(std::string(Carry, '1') + Str.substr(0, Truncate));
}

raw_ostream &Scaled_सङ्ख्या_आधारः::print(raw_ostream &OS, uint64_t D, int16_t E,
                                     पूर्णाङ्क विस्तारः, अचिह्नित Precision) {
  प्रत्यागम OS << गन्तव्यम्_String(D, E, विस्तारः, Precision);
}

शून्य Scaled_सङ्ख्या_आधारः::dump(uint64_t D, int16_t E, पूर्णाङ्क विस्तारः) {
  print(dbgs(), D, E, विस्तारः, 0) << "[" << विस्तारः << ":" << D << "*2^" << E
                                << "]";
}
