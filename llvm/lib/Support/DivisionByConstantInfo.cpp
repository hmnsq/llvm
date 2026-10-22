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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/DivisionByConstantInfo.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

/// Calculate the magic numbers required to implement a signed integer division
/// by a constant as a sequence of multiplies, adds and shifts.  Requires that
/// the divisor not be 0, 1, or -1.  Taken from "Hacker's Delight", Henry S.
/// Warren, Jr., Chapter 10.
SignedDivisionByConstantInfo SignedDivisionByConstantInfo::get(ਅਚਲ APInt &D) {
  ਪ੍ਰਤਿਜ੍ਞਾ(!D.isZero() && "Precondition violation.");

  // We'd be endlessly stuck in the loop.
  ਪ੍ਰਤਿਜ੍ਞਾ(D.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() >= 3 && "Does not work at smaller bitwidths.");

  APInt Delta;
  APInt SignedMin = APInt::get_Signed_Min_ਮੁੱਲ(D.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());
  ਬਣਤਰ SignedDivisionByConstantInfo Retval;

  APInt AD = D.ਨਿਰਪੇਖ();
  APInt T = SignedMin + (D.lshr(D.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() - 1));
  APInt ANC = T - 1 - T.urem(AD);   // absolute value of NC
  ਬਿਨਾਨਿਸ਼ਾਨੀ P = D.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() - 1; // initialize P
  APInt Q1, R1, Q2, R2;
  // initialize Q1 = 2P/abs(NC); R1 = rem(2P,abs(NC))
  APInt::udivrem(SignedMin, ANC, Q1, R1);
  // initialize Q2 = 2P/abs(D); R2 = rem(2P,abs(D))
  APInt::udivrem(SignedMin, AD, Q2, R2);
  ਕਰੋ {
    P = P + 1;
    Q1 <<= 1;      // update Q1 = 2P/abs(NC)
    R1 <<= 1;      // update R1 = rem(2P/abs(NC))
    ਜੇ (R1.uge(ANC)) { // must be unsigned comparison
      ++Q1;
      R1 -= ANC;
    }
    Q2 <<= 1;     // update Q2 = 2P/abs(D)
    R2 <<= 1;     // update R2 = rem(2P/abs(D))
    ਜੇ (R2.uge(AD)) { // must be unsigned comparison
      ++Q2;
      R2 -= AD;
    }
    // Delta = AD - R2
    Delta = AD;
    Delta -= R2;
  } ਜਦੋਂ (Q1.ult(Delta) || (Q1 == Delta && R1.isZero()));

  Retval.Magic = std::move(Q2);
  ++Retval.Magic;
  ਜੇ (D.isNegative())
    Retval.Magic.negate();                  // resulting magic number
  Retval.ਖਿਸਕਾਓ_Amount = P - D.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(); // resulting shift
  ਵਾਪਸ Retval;
}

/// Calculate the magic numbers required to implement an unsigned integer
/// division by a constant as a sequence of multiplies, adds and shifts.
/// Requires that the divisor not be 0.  Taken from "Hacker's Delight", Henry
/// S. Warren, Jr., chapter 10.
/// LeadingZeros can be used to simplify the calculation if the upper bits
/// of the divided value are known zero.
UnsignedDivisionByConstantInfo
UnsignedDivisionByConstantInfo::get(ਅਚਲ APInt &D, ਬਿਨਾਨਿਸ਼ਾਨੀ LeadingZeros,
                                    ਤਾਰਕਿਕ Allow_Even_ਭਾਜਕ_Optimization,
                                    ਤਾਰਕਿਕ AllowWidenOptimization) {
  ਪ੍ਰਤਿਜ੍ਞਾ(!D.isZero() && !D.isOne() && "Precondition violation.");
  ਪ੍ਰਤਿਜ੍ਞਾ(D.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() > 1 && "Does not work at smaller bitwidths.");

  APInt Delta;
  ਬਣਤਰ UnsignedDivisionByConstantInfo Retval;
  Retval.Is_ਜੋੜੋ = ਝੂਠ; // initialize "add" indicator
  Retval.Widen = ਝੂਠ; // initialize widen indicator
  APInt AllOnes =
      APInt::get_ਨੀਵਾਂ_ਬਿੱਟ_ਸੈੱਟ(D.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(), D.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() - LeadingZeros);
  APInt SignedMin = APInt::get_Signed_Min_ਮੁੱਲ(D.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());
  APInt SignedMax = APInt::get_Signed_Max_ਮੁੱਲ(D.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());

  // Calculate NC, the largest dividend such that NC.urem(D) == D-1.
  APInt NC = AllOnes - (AllOnes + 1 - D).urem(D);
  ਪ੍ਰਤਿਜ੍ਞਾ(NC.urem(D) == D - 1 && "Unexpected NC value");
  ਬਿਨਾਨਿਸ਼ਾਨੀ P = D.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() - 1; // initialize P
  APInt Q1, R1, Q2, R2;
  // initialize Q1 = 2P/NC; R1 = rem(2P,NC)
  APInt::udivrem(SignedMin, NC, Q1, R1);
  // initialize Q2 = (2P-1)/D; R2 = rem((2P-1),D)
  APInt::udivrem(SignedMax, D, Q2, R2);
  ਕਰੋ {
    P = P + 1;
    ਜੇ (R1.uge(NC - R1)) {
      // update Q1
      Q1 <<= 1;
      ++Q1;
      // update R1
      R1 <<= 1;
      R1 -= NC;
    } ਨਹੀਂਤਾਂ {
      Q1 <<= 1; // update Q1
      R1 <<= 1; // update R1
    }
    ਜੇ ((R2 + 1).uge(D - R2)) {
      ਜੇ (Q2.uge(SignedMax))
        Retval.Is_ਜੋੜੋ = ਸੱਚ;
      // update Q2
      Q2 <<= 1;
      ++Q2;
      // update R2
      R2 <<= 1;
      ++R2;
      R2 -= D;
    } ਨਹੀਂਤਾਂ {
      ਜੇ (Q2.uge(SignedMin))
        Retval.Is_ਜੋੜੋ = ਸੱਚ;
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
  } ਜਦੋਂ (P < D.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() * 2 &&
           (Q1.ult(Delta) || (Q1 == Delta && R1.isZero())));

  ਜੇ (Retval.Is_ਜੋੜੋ && !D[0] && Allow_Even_ਭਾਜਕ_Optimization) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ Pre_ਖਿਸਕਾਓ = D.countr_zero();
    APInt ShiftedD = D.lshr(Pre_ਖਿਸਕਾਓ);
    Retval =
        UnsignedDivisionByConstantInfo::get(ShiftedD, LeadingZeros + Pre_ਖਿਸਕਾਓ);
    ਪ੍ਰਤਿਜ੍ਞਾ(!Retval.Is_ਜੋੜੋ && Retval.Pre_ਖਿਸਕਾਓ == 0);
    Retval.Pre_ਖਿਸਕਾਓ = Pre_ਖਿਸਕਾਓ;
    ਵਾਪਸ Retval;
  }

  Retval.Magic = std::move(Q2);             // resulting magic number
  ++Retval.Magic;
  Retval.Post_ਖਿਸਕਾਓ = P - D.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(); // resulting shift
  // Reduce shift amount for IsAdd.
  ਜੇ (Retval.Is_ਜੋੜੋ) {
    ਪ੍ਰਤਿਜ੍ਞਾ(Retval.Post_ਖਿਸਕਾਓ > 0 && "Unexpected shift");
    Retval.Post_ਖਿਸਕਾਓ -= 1;
  }
  Retval.Pre_ਖਿਸਕਾਓ = 0;

  // For IsAdd case with AllowWidenOptimization, compute widened magic.
  // This is for optimizing 32-bit division using 64-bit multiplication.
  // The actual magic constant is 2^W + Magic ((W+1)-bit).
  // We pre-shift it left by (W*2 - OriginalShift) to avoid runtime shift.
  ਜੇ (Retval.Is_ਜੋੜੋ && AllowWidenOptimization) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ W = D.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
    ਬਿਨਾਨਿਸ਼ਾਨੀ Original_ਖਿਸਕਾਓ = Retval.Post_ਖਿਸਕਾਓ + W + 1;
    // Since PostShift >= 1, shift amount is at most W-2, so W*2 bits suffice.
    Retval.Magic = (APInt::get_One_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੈੱਟ(W * 2, W) + Retval.Magic.zext(W * 2))
                       .shl(W * 2 - Original_ਖਿਸਕਾਓ);
    Retval.Is_ਜੋੜੋ = ਝੂਠ;
    Retval.Post_ਖਿਸਕਾਓ = 0;
    Retval.Widen = ਸੱਚ;
  }

  ਵਾਪਸ Retval;
}
