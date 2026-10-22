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

#شامل "llvm/Support/DivisionByConstantInfo.h"

استعمال نامخلا llvm;

/// Calculate the magic numbers required to implement a signed integer division
/// by a constant as a sequence of multiplies, adds and shifts.  Requires that
/// the divisor not be 0, 1, or -1.  Taken from "Hacker's Delight", Henry S.
/// Warren, Jr., Chapter 10.
SignedDivisionByConstantInfo SignedDivisionByConstantInfo::get(ثابت APInt &D) {
  أكد(!D.isZero() && "Precondition violation.");

  // We'd be endlessly stuck in the loop.
  أكد(D.get_ثنائی_ہندسہ_چوڑائی() >= 3 && "Does not work at smaller bitwidths.");

  APInt Delta;
  APInt SignedMin = APInt::get_Signed_Min_قدر(D.get_ثنائی_ہندسہ_چوڑائی());
  ساخت SignedDivisionByConstantInfo Retval;

  APInt AD = D.مطلق_قدر();
  APInt T = SignedMin + (D.lshr(D.get_ثنائی_ہندسہ_چوڑائی() - 1));
  APInt ANC = T - 1 - T.urem(AD);   // absolute value of NC
  غیرعلامتی P = D.get_ثنائی_ہندسہ_چوڑائی() - 1; // initialize P
  APInt Q1, R1, Q2, R2;
  // initialize Q1 = 2P/abs(NC); R1 = rem(2P,abs(NC))
  APInt::udivrem(SignedMin, ANC, Q1, R1);
  // initialize Q2 = 2P/abs(D); R2 = rem(2P,abs(D))
  APInt::udivrem(SignedMin, AD, Q2, R2);
  کرو {
    P = P + 1;
    Q1 <<= 1;      // update Q1 = 2P/abs(NC)
    R1 <<= 1;      // update R1 = rem(2P/abs(NC))
    اگر (R1.uge(ANC)) { // must be unsigned comparison
      ++Q1;
      R1 -= ANC;
    }
    Q2 <<= 1;     // update Q2 = 2P/abs(D)
    R2 <<= 1;     // update R2 = rem(2P/abs(D))
    اگر (R2.uge(AD)) { // must be unsigned comparison
      ++Q2;
      R2 -= AD;
    }
    // Delta = AD - R2
    Delta = AD;
    Delta -= R2;
  } جب (Q1.ult(Delta) || (Q1 == Delta && R1.isZero()));

  Retval.Magic = std::move(Q2);
  ++Retval.Magic;
  اگر (D.isNegative())
    Retval.Magic.negate();                  // resulting magic number
  Retval.سرکاؤ_Amount = P - D.get_ثنائی_ہندسہ_چوڑائی(); // resulting shift
  واپس Retval;
}

/// Calculate the magic numbers required to implement an unsigned integer
/// division by a constant as a sequence of multiplies, adds and shifts.
/// Requires that the divisor not be 0.  Taken from "Hacker's Delight", Henry
/// S. Warren, Jr., chapter 10.
/// LeadingZeros can be used to simplify the calculation if the upper bits
/// of the divided value are known zero.
UnsignedDivisionByConstantInfo
UnsignedDivisionByConstantInfo::get(ثابت APInt &D, غیرعلامتی LeadingZeros,
                                    منطقی Allow_Even_مقسوم_علیہ_Optimization,
                                    منطقی AllowWidenOptimization) {
  أكد(!D.isZero() && !D.isOne() && "Precondition violation.");
  أكد(D.get_ثنائی_ہندسہ_چوڑائی() > 1 && "Does not work at smaller bitwidths.");

  APInt Delta;
  ساخت UnsignedDivisionByConstantInfo Retval;
  Retval.Is_جوڑو = جھوٹ; // initialize "add" indicator
  Retval.Widen = جھوٹ; // initialize widen indicator
  APInt AllOnes =
      APInt::get_کم_بٹ_سیٹ(D.get_ثنائی_ہندسہ_چوڑائی(), D.get_ثنائی_ہندسہ_چوڑائی() - LeadingZeros);
  APInt SignedMin = APInt::get_Signed_Min_قدر(D.get_ثنائی_ہندسہ_چوڑائی());
  APInt SignedMax = APInt::get_Signed_Max_قدر(D.get_ثنائی_ہندسہ_چوڑائی());

  // Calculate NC, the largest dividend such that NC.urem(D) == D-1.
  APInt NC = AllOnes - (AllOnes + 1 - D).urem(D);
  أكد(NC.urem(D) == D - 1 && "Unexpected NC value");
  غیرعلامتی P = D.get_ثنائی_ہندسہ_چوڑائی() - 1; // initialize P
  APInt Q1, R1, Q2, R2;
  // initialize Q1 = 2P/NC; R1 = rem(2P,NC)
  APInt::udivrem(SignedMin, NC, Q1, R1);
  // initialize Q2 = (2P-1)/D; R2 = rem((2P-1),D)
  APInt::udivrem(SignedMax, D, Q2, R2);
  کرو {
    P = P + 1;
    اگر (R1.uge(NC - R1)) {
      // update Q1
      Q1 <<= 1;
      ++Q1;
      // update R1
      R1 <<= 1;
      R1 -= NC;
    } ورنہ {
      Q1 <<= 1; // update Q1
      R1 <<= 1; // update R1
    }
    اگر ((R2 + 1).uge(D - R2)) {
      اگر (Q2.uge(SignedMax))
        Retval.Is_جوڑو = سچ;
      // update Q2
      Q2 <<= 1;
      ++Q2;
      // update R2
      R2 <<= 1;
      ++R2;
      R2 -= D;
    } ورنہ {
      اگر (Q2.uge(SignedMin))
        Retval.Is_جوڑو = سچ;
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
  } جب (P < D.get_ثنائی_ہندسہ_چوڑائی() * 2 &&
           (Q1.ult(Delta) || (Q1 == Delta && R1.isZero())));

  اگر (Retval.Is_جوڑو && !D[0] && Allow_Even_مقسوم_علیہ_Optimization) {
    غیرعلامتی Pre_سرکاؤ = D.countr_zero();
    APInt ShiftedD = D.lshr(Pre_سرکاؤ);
    Retval =
        UnsignedDivisionByConstantInfo::get(ShiftedD, LeadingZeros + Pre_سرکاؤ);
    أكد(!Retval.Is_جوڑو && Retval.Pre_سرکاؤ == 0);
    Retval.Pre_سرکاؤ = Pre_سرکاؤ;
    واپس Retval;
  }

  Retval.Magic = std::move(Q2);             // resulting magic number
  ++Retval.Magic;
  Retval.Post_سرکاؤ = P - D.get_ثنائی_ہندسہ_چوڑائی(); // resulting shift
  // Reduce shift amount for IsAdd.
  اگر (Retval.Is_جوڑو) {
    أكد(Retval.Post_سرکاؤ > 0 && "Unexpected shift");
    Retval.Post_سرکاؤ -= 1;
  }
  Retval.Pre_سرکاؤ = 0;

  // For IsAdd case with AllowWidenOptimization, compute widened magic.
  // This is for optimizing 32-bit division using 64-bit multiplication.
  // The actual magic constant is 2^W + Magic ((W+1)-bit).
  // We pre-shift it left by (W*2 - OriginalShift) to avoid runtime shift.
  اگر (Retval.Is_جوڑو && AllowWidenOptimization) {
    غیرعلامتی W = D.get_ثنائی_ہندسہ_چوڑائی();
    غیرعلامتی Original_سرکاؤ = Retval.Post_سرکاؤ + W + 1;
    // Since PostShift >= 1, shift amount is at most W-2, so W*2 bits suffice.
    Retval.Magic = (APInt::get_One_ثنائی_ہندسہ_سیٹ(W * 2, W) + Retval.Magic.zext(W * 2))
                       .shl(W * 2 - Original_سرکاؤ);
    Retval.Is_جوڑو = جھوٹ;
    Retval.Post_سرکاؤ = 0;
    Retval.Widen = سچ;
  }

  واپس Retval;
}
