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

#अन्तर्भाव "llvm/Support/DivisionByConstantInfo.h"

using नामाकाश llvm;

/// Calculate the magic numbers required to implement a signed integer division
/// by a constant as a sequence of multiplies, adds and shifts.  Requires that
/// the divisor not be 0, 1, or -1.  Taken from "Hacker's Delight", Henry S.
/// Warren, Jr., Chapter 10.
SignedDivisionByConstantInfo SignedDivisionByConstantInfo::get(ध्रुव APInt &D) {
  प्रतिज्ञा(!D.isZero() && "Precondition violation.");

  // We'd be endlessly stuck in the loop.
  प्रतिज्ञा(D.getBitWidth() >= 3 && "Does not work at smaller bitwidths.");

  APInt Delta;
  APInt SignedMin = APInt::get_Signed_Min_मूल्य(D.getBitWidth());
  संरचना SignedDivisionByConstantInfo Retval;

  APInt AD = D.abs();
  APInt T = SignedMin + (D.lshr(D.getBitWidth() - 1));
  APInt ANC = T - 1 - T.urem(AD);   // absolute value of NC
  unsigned P = D.getBitWidth() - 1; // initialize P
  APInt Q1, R1, Q2, R2;
  // initialize Q1 = 2P/abs(NC); R1 = rem(2P,abs(NC))
  APInt::udivrem(SignedMin, ANC, Q1, R1);
  // initialize Q2 = 2P/abs(D); R2 = rem(2P,abs(D))
  APInt::udivrem(SignedMin, AD, Q2, R2);
  कुरु {
    P = P + 1;
    Q1 <<= 1;      // update Q1 = 2P/abs(NC)
    R1 <<= 1;      // update R1 = rem(2P/abs(NC))
    यदि (R1.uge(ANC)) { // must be unsigned comparison
      ++Q1;
      R1 -= ANC;
    }
    Q2 <<= 1;     // update Q2 = 2P/abs(D)
    R2 <<= 1;     // update R2 = rem(2P/abs(D))
    यदि (R2.uge(AD)) { // must be unsigned comparison
      ++Q2;
      R2 -= AD;
    }
    // Delta = AD - R2
    Delta = AD;
    Delta -= R2;
  } यावत् (Q1.ult(Delta) || (Q1 == Delta && R1.isZero()));

  Retval.Magic = std::move(Q2);
  ++Retval.Magic;
  यदि (D.isNegative())
    Retval.Magic.negate();                  // resulting magic number
  Retval.ShiftAmount = P - D.getBitWidth(); // resulting shift
  प्रत्यागम Retval;
}

/// Calculate the magic numbers required to implement an unsigned integer
/// division by a constant as a sequence of multiplies, adds and shifts.
/// Requires that the divisor not be 0.  Taken from "Hacker's Delight", Henry
/// S. Warren, Jr., chapter 10.
/// LeadingZeros can be used to simplify the calculation if the upper bits
/// of the divided value are known zero.
UnsignedDivisionByConstantInfo
UnsignedDivisionByConstantInfo::get(ध्रुव APInt &D, unsigned LeadingZeros,
                                    तार्किक Allow_Even_भाजक_Optimization,
                                    तार्किक AllowWidenOptimization) {
  प्रतिज्ञा(!D.isZero() && !D.isOne() && "Precondition violation.");
  प्रतिज्ञा(D.getBitWidth() > 1 && "Does not work at smaller bitwidths.");

  APInt Delta;
  संरचना UnsignedDivisionByConstantInfo Retval;
  Retval.IsAdd = असत्य; // initialize "add" indicator
  Retval.Widen = असत्य; // initialize widen indicator
  APInt AllOnes =
      APInt::get_खालचा_Bits_Set(D.getBitWidth(), D.getBitWidth() - LeadingZeros);
  APInt SignedMin = APInt::get_Signed_Min_मूल्य(D.getBitWidth());
  APInt SignedMax = APInt::get_Signed_Max_मूल्य(D.getBitWidth());

  // Calculate NC, the largest dividend such that NC.urem(D) == D-1.
  APInt NC = AllOnes - (AllOnes + 1 - D).urem(D);
  प्रतिज्ञा(NC.urem(D) == D - 1 && "Unexpected NC value");
  unsigned P = D.getBitWidth() - 1; // initialize P
  APInt Q1, R1, Q2, R2;
  // initialize Q1 = 2P/NC; R1 = rem(2P,NC)
  APInt::udivrem(SignedMin, NC, Q1, R1);
  // initialize Q2 = (2P-1)/D; R2 = rem((2P-1),D)
  APInt::udivrem(SignedMax, D, Q2, R2);
  कुरु {
    P = P + 1;
    यदि (R1.uge(NC - R1)) {
      // update Q1
      Q1 <<= 1;
      ++Q1;
      // update R1
      R1 <<= 1;
      R1 -= NC;
    } अन्यथा {
      Q1 <<= 1; // update Q1
      R1 <<= 1; // update R1
    }
    यदि ((R2 + 1).uge(D - R2)) {
      यदि (Q2.uge(SignedMax))
        Retval.IsAdd = सत्य;
      // update Q2
      Q2 <<= 1;
      ++Q2;
      // update R2
      R2 <<= 1;
      ++R2;
      R2 -= D;
    } अन्यथा {
      यदि (Q2.uge(SignedMin))
        Retval.IsAdd = सत्य;
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
  } यावत् (P < D.getBitWidth() * 2 &&
           (Q1.ult(Delta) || (Q1 == Delta && R1.isZero())));

  यदि (Retval.IsAdd && !D[0] && Allow_Even_भाजक_Optimization) {
    unsigned PreShift = D.countr_zero();
    APInt ShiftedD = D.lshr(PreShift);
    Retval =
        UnsignedDivisionByConstantInfo::get(ShiftedD, LeadingZeros + PreShift);
    प्रतिज्ञा(!Retval.IsAdd && Retval.PreShift == 0);
    Retval.PreShift = PreShift;
    प्रत्यागम Retval;
  }

  Retval.Magic = std::move(Q2);             // resulting magic number
  ++Retval.Magic;
  Retval.PostShift = P - D.getBitWidth(); // resulting shift
  // Reduce shift amount for IsAdd.
  यदि (Retval.IsAdd) {
    प्रतिज्ञा(Retval.PostShift > 0 && "Unexpected shift");
    Retval.PostShift -= 1;
  }
  Retval.PreShift = 0;

  // For IsAdd case with AllowWidenOptimization, compute widened magic.
  // This is for optimizing 32-bit division using 64-bit multiplication.
  // The actual magic constant is 2^W + Magic ((W+1)-bit).
  // We pre-shift it left by (W*2 - OriginalShift) to avoid runtime shift.
  यदि (Retval.IsAdd && AllowWidenOptimization) {
    unsigned W = D.getBitWidth();
    unsigned OriginalShift = Retval.PostShift + W + 1;
    // Since PostShift >= 1, shift amount is at most W-2, so W*2 bits suffice.
    Retval.Magic = (APInt::getOneBitSet(W * 2, W) + Retval.Magic.zext(W * 2))
                       .shl(W * 2 - OriginalShift);
    Retval.IsAdd = असत्य;
    Retval.PostShift = 0;
    Retval.Widen = सत्य;
  }

  प्रत्यागम Retval;
}
