//===----- DivisionByConstantInfo.cpp - division by constant -*- C++ -*----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// This file implements support for optimizing divisions by a constant
///
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/DivisionByConstantInfo.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

/// Calculate the magic numbers required to implement a signed integer division
/// by a constant as a sequence of multiplies, adds and shifts.  Requires that
/// the divisor not be 0, 1, or -1.  Taken from "Hacker's Delight", Henry S.
/// Warren, Jr., Chapter 10.
SignedDivisionByConstantInfo SignedDivisionByConstantInfo::get(ಸ್ಥಿರಾಂಕ APInt &D) {
  ಪ್ರತಿಜ್ಞಾ(!D.isZero() && "Precondition violation.");

  // We'd be endlessly stuck in the loop.
  ಪ್ರತಿಜ್ಞಾ(D.get_ಬಿಟ್_ಅಗಲ() >= 3 && "Does not work at smaller bitwidths.");

  APInt Delta;
  APInt SignedMin = APInt::get_Signed_Min_ಮೌಲ್ಯ(D.get_ಬಿಟ್_ಅಗಲ());
  ರಚನೆ SignedDivisionByConstantInfo Retval;

  APInt AD = D.ನಿರಪೇಕ್ಷ();
  APInt T = SignedMin + (D.lshr(D.get_ಬಿಟ್_ಅಗಲ() - 1));
  APInt ANC = T - 1 - T.urem(AD);   // absolute value of NC
  ಚಿಹ್ನೆರಹಿತ P = D.get_ಬಿಟ್_ಅಗಲ() - 1; // initialize P
  APInt Q1, R1, Q2, R2;
  // initialize Q1 = 2P/abs(NC); R1 = rem(2P,abs(NC))
  APInt::udivrem(SignedMin, ANC, Q1, R1);
  // initialize Q2 = 2P/abs(D); R2 = rem(2P,abs(D))
  APInt::udivrem(SignedMin, AD, Q2, R2);
  ಮಾಡು {
    P = P + 1;
    Q1 <<= 1;      // update Q1 = 2P/abs(NC)
    R1 <<= 1;      // update R1 = rem(2P/abs(NC))
    ಆದರೆ (R1.uge(ANC)) { // must be unsigned comparison
      ++Q1;
      R1 -= ANC;
    }
    Q2 <<= 1;     // update Q2 = 2P/abs(D)
    R2 <<= 1;     // update R2 = rem(2P/abs(D))
    ಆದರೆ (R2.uge(AD)) { // must be unsigned comparison
      ++Q2;
      R2 -= AD;
    }
    // Delta = AD - R2
    Delta = AD;
    Delta -= R2;
  } ಆಗ (Q1.ult(Delta) || (Q1 == Delta && R1.isZero()));

  Retval.Magic = std::move(Q2);
  ++Retval.Magic;
  ಆದರೆ (D.isNegative())
    Retval.Magic.negate();                  // resulting magic number
  Retval.ಸರಿಸು_Amount = P - D.get_ಬಿಟ್_ಅಗಲ(); // resulting shift
  ಹಿಂತಿರುಗಿ Retval;
}

/// Calculate the magic numbers required to implement an unsigned integer
/// division by a constant as a sequence of multiplies, adds and shifts.
/// Requires that the divisor not be 0.  Taken from "Hacker's Delight", Henry
/// S. Warren, Jr., chapter 10.
/// LeadingZeros can be used to simplify the calculation if the upper bits
/// of the divided value are known zero.
UnsignedDivisionByConstantInfo
UnsignedDivisionByConstantInfo::get(ಸ್ಥಿರಾಂಕ APInt &D, ಚಿಹ್ನೆರಹಿತ LeadingZeros,
                                    ತಾರ್ಕಿಕ Allow_Even_ಭಾಜಕ_Optimization,
                                    ತಾರ್ಕಿಕ AllowWidenOptimization) {
  ಪ್ರತಿಜ್ಞಾ(!D.isZero() && !D.isOne() && "Precondition violation.");
  ಪ್ರತಿಜ್ಞಾ(D.get_ಬಿಟ್_ಅಗಲ() > 1 && "Does not work at smaller bitwidths.");

  APInt Delta;
  ರಚನೆ UnsignedDivisionByConstantInfo Retval;
  Retval.Is_ಕೂಡಿಸು = ಸುಳ್ಳು; // initialize "add" indicator
  Retval.Widen = ಸುಳ್ಳು; // initialize widen indicator
  APInt AllOnes =
      APInt::get_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು_ಗಣ(D.get_ಬಿಟ್_ಅಗಲ(), D.get_ಬಿಟ್_ಅಗಲ() - LeadingZeros);
  APInt SignedMin = APInt::get_Signed_Min_ಮೌಲ್ಯ(D.get_ಬಿಟ್_ಅಗಲ());
  APInt SignedMax = APInt::get_Signed_Max_ಮೌಲ್ಯ(D.get_ಬಿಟ್_ಅಗಲ());

  // Calculate NC, the largest dividend such that NC.urem(D) == D-1.
  APInt NC = AllOnes - (AllOnes + 1 - D).urem(D);
  ಪ್ರತಿಜ್ಞಾ(NC.urem(D) == D - 1 && "Unexpected NC value");
  ಚಿಹ್ನೆರಹಿತ P = D.get_ಬಿಟ್_ಅಗಲ() - 1; // initialize P
  APInt Q1, R1, Q2, R2;
  // initialize Q1 = 2P/NC; R1 = rem(2P,NC)
  APInt::udivrem(SignedMin, NC, Q1, R1);
  // initialize Q2 = (2P-1)/D; R2 = rem((2P-1),D)
  APInt::udivrem(SignedMax, D, Q2, R2);
  ಮಾಡು {
    P = P + 1;
    ಆದರೆ (R1.uge(NC - R1)) {
      // update Q1
      Q1 <<= 1;
      ++Q1;
      // update R1
      R1 <<= 1;
      R1 -= NC;
    } ಇಲ್ಲದಿದ್ದರೆ {
      Q1 <<= 1; // update Q1
      R1 <<= 1; // update R1
    }
    ಆದರೆ ((R2 + 1).uge(D - R2)) {
      ಆದರೆ (Q2.uge(SignedMax))
        Retval.Is_ಕೂಡಿಸು = ನಿಜ;
      // update Q2
      Q2 <<= 1;
      ++Q2;
      // update R2
      R2 <<= 1;
      ++R2;
      R2 -= D;
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಆದರೆ (Q2.uge(SignedMin))
        Retval.Is_ಕೂಡಿಸು = ನಿಜ;
      // update Q2
      Q2 <<= 1;
      // update R2
      R2 <<= 1;
      ++R2;
    }
    // Delta = D - 1 - R2
    Delta = D;
    --Delta;
    Delta -= R2;
  } ಆಗ (P < D.get_ಬಿಟ್_ಅಗಲ() * 2 &&
           (Q1.ult(Delta) || (Q1 == Delta && R1.isZero())));

  ಆದರೆ (Retval.Is_ಕೂಡಿಸು && !D[0] && Allow_Even_ಭಾಜಕ_Optimization) {
    ಚಿಹ್ನೆರಹಿತ Pre_ಸರಿಸು = D.countr_zero();
    APInt ShiftedD = D.lshr(Pre_ಸರಿಸು);
    Retval =
        UnsignedDivisionByConstantInfo::get(ShiftedD, LeadingZeros + Pre_ಸರಿಸು);
    ಪ್ರತಿಜ್ಞಾ(!Retval.Is_ಕೂಡಿಸು && Retval.Pre_ಸರಿಸು == 0);
    Retval.Pre_ಸರಿಸು = Pre_ಸರಿಸು;
    ಹಿಂತಿರುಗಿ Retval;
  }

  Retval.Magic = std::move(Q2);             // resulting magic number
  ++Retval.Magic;
  Retval.Post_ಸರಿಸು = P - D.get_ಬಿಟ್_ಅಗಲ(); // resulting shift
  // Reduce shift amount for IsAdd.
  ಆದರೆ (Retval.Is_ಕೂಡಿಸು) {
    ಪ್ರತಿಜ್ಞಾ(Retval.Post_ಸರಿಸು > 0 && "Unexpected shift");
    Retval.Post_ಸರಿಸು -= 1;
  }
  Retval.Pre_ಸರಿಸು = 0;

  // For IsAdd case with AllowWidenOptimization, compute widened magic.
  // This is for optimizing 32-bit division using 64-bit multiplication.
  // The actual magic constant is 2^W + Magic ((W+1)-bit).
  // We pre-shift it left by (W*2 - OriginalShift) to avoid runtime shift.
  ಆದರೆ (Retval.Is_ಕೂಡಿಸು && AllowWidenOptimization) {
    ಚಿಹ್ನೆರಹಿತ W = D.get_ಬಿಟ್_ಅಗಲ();
    ಚಿಹ್ನೆರಹಿತ Original_ಸರಿಸು = Retval.Post_ಸರಿಸು + W + 1;
    // Since PostShift >= 1, shift amount is at most W-2, so W*2 bits suffice.
    Retval.Magic = (APInt::get_One_ಬಿಟ್_ಗಣ(W * 2, W) + Retval.Magic.zext(W * 2))
                       .shl(W * 2 - Original_ಸರಿಸು);
    Retval.Is_ಕೂಡಿಸು = ಸುಳ್ಳು;
    Retval.Post_ಸರಿಸು = 0;
    Retval.Widen = ನಿಜ;
  }

  ಹಿಂತಿರುಗಿ Retval;
}
