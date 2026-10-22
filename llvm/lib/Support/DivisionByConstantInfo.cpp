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

#అన్తర్భావ "llvm/Support/DivisionByConstantInfo.h"

ఉపయోగించి నామాకాశ llvm;

/// Calculate the magic numbers required to implement a signed integer division
/// by a constant as a sequence of multiplies, adds and shifts.  Requires that
/// the divisor not be 0, 1, or -1.  Taken from "Hacker's Delight", Henry S.
/// Warren, Jr., Chapter 10.
SignedDivisionByConstantInfo SignedDivisionByConstantInfo::get(స్థిరరాశి APInt &D) {
  ప్రతిజ్ఞా(!D.isZero() && "Precondition violation.");

  // We'd be endlessly stuck in the loop.
  ప్రతిజ్ఞా(D.get_బిట్టు_వెడల్పు() >= 3 && "Does not work at smaller bitwidths.");

  APInt Delta;
  APInt SignedMin = APInt::get_Signed_Min_విలువ(D.get_బిట్టు_వెడల్పు());
  నిర్మాణం SignedDivisionByConstantInfo Retval;

  APInt AD = D.నిరపేక్ష();
  APInt T = SignedMin + (D.lshr(D.get_బిట్టు_వెడల్పు() - 1));
  APInt ANC = T - 1 - T.urem(AD);   // absolute value of NC
  గుర్తులేని P = D.get_బిట్టు_వెడల్పు() - 1; // initialize P
  APInt Q1, R1, Q2, R2;
  // initialize Q1 = 2P/abs(NC); R1 = rem(2P,abs(NC))
  APInt::udivrem(SignedMin, ANC, Q1, R1);
  // initialize Q2 = 2P/abs(D); R2 = rem(2P,abs(D))
  APInt::udivrem(SignedMin, AD, Q2, R2);
  చేయి {
    P = P + 1;
    Q1 <<= 1;      // update Q1 = 2P/abs(NC)
    R1 <<= 1;      // update R1 = rem(2P/abs(NC))
    ఒకవేళ (R1.uge(ANC)) { // must be unsigned comparison
      ++Q1;
      R1 -= ANC;
    }
    Q2 <<= 1;     // update Q2 = 2P/abs(D)
    R2 <<= 1;     // update R2 = rem(2P/abs(D))
    ఒకవేళ (R2.uge(AD)) { // must be unsigned comparison
      ++Q2;
      R2 -= AD;
    }
    // Delta = AD - R2
    Delta = AD;
    Delta -= R2;
  } అయితే (Q1.ult(Delta) || (Q1 == Delta && R1.isZero()));

  Retval.Magic = std::move(Q2);
  ++Retval.Magic;
  ఒకవేళ (D.isNegative())
    Retval.Magic.negate();                  // resulting magic number
  Retval.జరుపు_Amount = P - D.get_బిట్టు_వెడల్పు(); // resulting shift
  తిరిగి Retval;
}

/// Calculate the magic numbers required to implement an unsigned integer
/// division by a constant as a sequence of multiplies, adds and shifts.
/// Requires that the divisor not be 0.  Taken from "Hacker's Delight", Henry
/// S. Warren, Jr., chapter 10.
/// LeadingZeros can be used to simplify the calculation if the upper bits
/// of the divided value are known zero.
UnsignedDivisionByConstantInfo
UnsignedDivisionByConstantInfo::get(స్థిరరాశి APInt &D, గుర్తులేని LeadingZeros,
                                    తార్కిక Allow_Even_భాజకం_Optimization,
                                    తార్కిక AllowWidenOptimization) {
  ప్రతిజ్ఞా(!D.isZero() && !D.isOne() && "Precondition violation.");
  ప్రతిజ్ఞా(D.get_బిట్టు_వెడల్పు() > 1 && "Does not work at smaller bitwidths.");

  APInt Delta;
  నిర్మాణం UnsignedDivisionByConstantInfo Retval;
  Retval.Is_కూడు = అబద్ధం; // initialize "add" indicator
  Retval.Widen = అబద్ధం; // initialize widen indicator
  APInt AllOnes =
      APInt::get_దిగువ_బిట్లు_సమితి(D.get_బిట్టు_వెడల్పు(), D.get_బిట్టు_వెడల్పు() - LeadingZeros);
  APInt SignedMin = APInt::get_Signed_Min_విలువ(D.get_బిట్టు_వెడల్పు());
  APInt SignedMax = APInt::get_Signed_Max_విలువ(D.get_బిట్టు_వెడల్పు());

  // Calculate NC, the largest dividend such that NC.urem(D) == D-1.
  APInt NC = AllOnes - (AllOnes + 1 - D).urem(D);
  ప్రతిజ్ఞా(NC.urem(D) == D - 1 && "Unexpected NC value");
  గుర్తులేని P = D.get_బిట్టు_వెడల్పు() - 1; // initialize P
  APInt Q1, R1, Q2, R2;
  // initialize Q1 = 2P/NC; R1 = rem(2P,NC)
  APInt::udivrem(SignedMin, NC, Q1, R1);
  // initialize Q2 = (2P-1)/D; R2 = rem((2P-1),D)
  APInt::udivrem(SignedMax, D, Q2, R2);
  చేయి {
    P = P + 1;
    ఒకవేళ (R1.uge(NC - R1)) {
      // update Q1
      Q1 <<= 1;
      ++Q1;
      // update R1
      R1 <<= 1;
      R1 -= NC;
    } లేకపోతే {
      Q1 <<= 1; // update Q1
      R1 <<= 1; // update R1
    }
    ఒకవేళ ((R2 + 1).uge(D - R2)) {
      ఒకవేళ (Q2.uge(SignedMax))
        Retval.Is_కూడు = నిజం;
      // update Q2
      Q2 <<= 1;
      ++Q2;
      // update R2
      R2 <<= 1;
      ++R2;
      R2 -= D;
    } లేకపోతే {
      ఒకవేళ (Q2.uge(SignedMin))
        Retval.Is_కూడు = నిజం;
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
  } అయితే (P < D.get_బిట్టు_వెడల్పు() * 2 &&
           (Q1.ult(Delta) || (Q1 == Delta && R1.isZero())));

  ఒకవేళ (Retval.Is_కూడు && !D[0] && Allow_Even_భాజకం_Optimization) {
    గుర్తులేని Pre_జరుపు = D.countr_zero();
    APInt ShiftedD = D.lshr(Pre_జరుపు);
    Retval =
        UnsignedDivisionByConstantInfo::get(ShiftedD, LeadingZeros + Pre_జరుపు);
    ప్రతిజ్ఞా(!Retval.Is_కూడు && Retval.Pre_జరుపు == 0);
    Retval.Pre_జరుపు = Pre_జరుపు;
    తిరిగి Retval;
  }

  Retval.Magic = std::move(Q2);             // resulting magic number
  ++Retval.Magic;
  Retval.Post_జరుపు = P - D.get_బిట్టు_వెడల్పు(); // resulting shift
  // Reduce shift amount for IsAdd.
  ఒకవేళ (Retval.Is_కూడు) {
    ప్రతిజ్ఞా(Retval.Post_జరుపు > 0 && "Unexpected shift");
    Retval.Post_జరుపు -= 1;
  }
  Retval.Pre_జరుపు = 0;

  // For IsAdd case with AllowWidenOptimization, compute widened magic.
  // This is for optimizing 32-bit division using 64-bit multiplication.
  // The actual magic constant is 2^W + Magic ((W+1)-bit).
  // We pre-shift it left by (W*2 - OriginalShift) to avoid runtime shift.
  ఒకవేళ (Retval.Is_కూడు && AllowWidenOptimization) {
    గుర్తులేని W = D.get_బిట్టు_వెడల్పు();
    గుర్తులేని Original_జరుపు = Retval.Post_జరుపు + W + 1;
    // Since PostShift >= 1, shift amount is at most W-2, so W*2 bits suffice.
    Retval.Magic = (APInt::get_One_బిట్టు_సమితి(W * 2, W) + Retval.Magic.zext(W * 2))
                       .shl(W * 2 - Original_జరుపు);
    Retval.Is_కూడు = అబద్ధం;
    Retval.Post_జరుపు = 0;
    Retval.Widen = నిజం;
  }

  తిరిగి Retval;
}
