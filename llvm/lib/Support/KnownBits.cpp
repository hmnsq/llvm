//===-- KnownBits.cpp - Stores known zeros/ones ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a class for representing known zeros and ones used by
// computeKnownBits.
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/KnownBits.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Sequence.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <cassert>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::flip_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &Val) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Position = Val.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ_Val(Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Position, Val.One[Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Position]);
  One.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ_Val(Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Position, Val.Zero[Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Position]);
  ਵਾਪਸ ਗਿਆਤ_ਬਿੱਟ(Zero, One);
}

ਸਥਿਰ ਗਿਆਤ_ਬਿੱਟ compute_For_ਜੋੜੋ_ਹਾਸਲ(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS,
                                    ਤਾਰਕਿਕ ਹਾਸਲ_Zero, ਤਾਰਕਿਕ ਹਾਸਲ_One) {

  APInt Possible_ਜੋੜ_Zero = LHS.get_Max_ਮੁੱਲ() + RHS.get_Max_ਮੁੱਲ() + !ਹਾਸਲ_Zero;
  APInt Possible_ਜੋੜ_One = LHS.get_Min_ਮੁੱਲ() + RHS.get_Min_ਮੁੱਲ() + ਹਾਸਲ_One;

  // Compute known bits of the carry.
  APInt ਹਾਸਲ_ਗਿਆਤ_Zero = ~(Possible_ਜੋੜ_Zero ^ LHS.Zero ^ RHS.Zero);
  APInt ਹਾਸਲ_ਗਿਆਤ_One = Possible_ਜੋੜ_One ^ LHS.One ^ RHS.One;

  // Compute set of known bits (where all three relevant bits are known).
  APInt LHSKnownUnion = LHS.Zero | LHS.One;
  APInt RHSKnownUnion = RHS.Zero | RHS.One;
  APInt ਹਾਸਲ_ਗਿਆਤ_Union = std::move(ਹਾਸਲ_ਗਿਆਤ_Zero) | ਹਾਸਲ_ਗਿਆਤ_One;
  APInt ਗਿਆਤ = std::move(LHSKnownUnion) & RHSKnownUnion & ਹਾਸਲ_ਗਿਆਤ_Union;

  // Compute known bits of the result.
  ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਾਹਰਲਾ;
  ਗਿਆਤ_ਬਾਹਰਲਾ.Zero = ~std::move(Possible_ਜੋੜ_Zero) & ਗਿਆਤ;
  ਗਿਆਤ_ਬਾਹਰਲਾ.One = std::move(Possible_ਜੋੜ_One) & ਗਿਆਤ;
  ਵਾਪਸ ਗਿਆਤ_ਬਾਹਰਲਾ;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::compute_For_ਜੋੜੋ_ਹਾਸਲ(
    ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &ਹਾਸਲ_2) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਹਾਸਲ_2.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() == 1 && "Carry must be 1-bit");
  ਵਾਪਸ ::compute_For_ਜੋੜੋ_ਹਾਸਲ(
      LHS, RHS, ਹਾਸਲ_2.Zero.get_Bool_ਮੁੱਲ(), ਹਾਸਲ_2.One.get_Bool_ਮੁੱਲ());
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::compute_For_ਜੋੜੋ_Sub(ਤਾਰਕਿਕ ਜੋੜੋ_2, ਤਾਰਕਿਕ NSW, ਤਾਰਕਿਕ NUW,
                                      ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS,
                                      ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਾਹਰਲਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  ਜੇ (LHS.isUnknown() && RHS.isUnknown())
    ਵਾਪਸ ਗਿਆਤ_ਬਾਹਰਲਾ;

  ਜੇ (!LHS.isUnknown() && !RHS.isUnknown()) {
    ਜੇ (ਜੋੜੋ_2) {
      // Sum = LHS + RHS + 0
      ਗਿਆਤ_ਬਾਹਰਲਾ = ::compute_For_ਜੋੜੋ_ਹਾਸਲ(LHS, RHS, /*CarryZero=*/ਸੱਚ,
                                      /*CarryOne=*/ਝੂਠ);
    } ਨਹੀਂਤਾਂ {
      // Sum = LHS + ~RHS + 1
      ਗਿਆਤ_ਬਿੱਟ NotRHS = RHS;
      std::ਅਦਲਾ_ਬਦਲੀ(NotRHS.Zero, NotRHS.One);
      ਗਿਆਤ_ਬਾਹਰਲਾ = ::compute_For_ਜੋੜੋ_ਹਾਸਲ(LHS, NotRHS, /*CarryZero=*/ਝੂਠ,
                                      /*CarryOne=*/ਸੱਚ);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  ਜੇ (NUW) {
    ਜੇ (ਜੋੜੋ_2) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_ਮੁੱਲ().uadd_sat(RHS.get_Min_ਮੁੱਲ());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      ਜੇ (NSW) {
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਿੱਟ_2 = MinVal.trunc(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        ਗਿਆਤ_ਬਾਹਰਲਾ.One.ਸੈੱਟ_ਬਿੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1 - ਅੰਕ_ਬਿੱਟ_2, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1);
      }
      ਗਿਆਤ_ਬਾਹਰਲਾ.One.ਸੈੱਟ_ਉੱਚਾ_ਬਿੱਟ(MinVal.countl_one());
    } ਨਹੀਂਤਾਂ {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_ਮੁੱਲ().usub_sat(RHS.get_Min_ਮੁੱਲ());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      ਜੇ (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਿੱਟ_2 = MaxVal.trunc(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1).countl_zero();
        ਗਿਆਤ_ਬਾਹਰਲਾ.Zero.ਸੈੱਟ_ਬਿੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1 - ਅੰਕ_ਬਿੱਟ_2, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1);
      }
      ਗਿਆਤ_ਬਾਹਰਲਾ.Zero.ਸੈੱਟ_ਉੱਚਾ_ਬਿੱਟ(MaxVal.countl_zero());
    }
  }

  ਜੇ (NSW) {
    APInt MinVal;
    APInt MaxVal;
    ਜੇ (ਜੋੜੋ_2) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_ਮੁੱਲ().sadd_sat(RHS.get_Signed_Min_ਮੁੱਲ());
      MaxVal = LHS.get_Signed_Max_ਮੁੱਲ().sadd_sat(RHS.get_Signed_Max_ਮੁੱਲ());
    } ਨਹੀਂਤਾਂ {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_ਮੁੱਲ().ssub_sat(RHS.get_Signed_Max_ਮੁੱਲ());
      MaxVal = LHS.get_Signed_Max_ਮੁੱਲ().ssub_sat(RHS.get_Signed_Min_ਮੁੱਲ());
    }
    ਜੇ (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਿੱਟ_2 = MinVal.trunc(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1).countl_one();
      ਗਿਆਤ_ਬਾਹਰਲਾ.One.ਸੈੱਟ_ਬਿੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1 - ਅੰਕ_ਬਿੱਟ_2, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1);
      ਗਿਆਤ_ਬਾਹਰਲਾ.Zero.ਸੈੱਟ_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ();
    }
    ਜੇ (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਿੱਟ_2 = MaxVal.trunc(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1).countl_zero();
      ਗਿਆਤ_ਬਾਹਰਲਾ.Zero.ਸੈੱਟ_ਬਿੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1 - ਅੰਕ_ਬਿੱਟ_2, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1);
      ਗਿਆਤ_ਬਾਹਰਲਾ.One.ਸੈੱਟ_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  ਜੇ (ਗਿਆਤ_ਬਾਹਰਲਾ.hasConflict())
    ਗਿਆਤ_ਬਾਹਰਲਾ.ਸੈੱਟ_All_Zero();
  ਵਾਪਸ ਗਿਆਤ_ਬਾਹਰਲਾ;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::computeForSubBorrow(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਗਿਆਤ_ਬਿੱਟ RHS,
                                         ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &Borrow) {
  ਪ੍ਰਤਿਜ੍ਞਾ(Borrow.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::ਅਦਲਾ_ਬਦਲੀ(RHS.Zero, RHS.One);
  ਵਾਪਸ ::compute_For_ਜੋੜੋ_ਹਾਸਲ(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_ਮੁੱਲ(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_ਮੁੱਲ());
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::truncSSat(ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਿਵੇਸ਼_ਬਿੱਟ = get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  APInt Min_ਵਿੱਚ_Range = APInt::get_Signed_Min_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2).sext(ਨਿਵੇਸ਼_ਬਿੱਟ);
  APInt Max_ਵਿੱਚ_Range = APInt::get_Signed_Max_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2).sext(ਨਿਵੇਸ਼_ਬਿੱਟ);
  APInt ਨਿਵੇਸ਼_Min = get_Signed_Min_ਮੁੱਲ();
  APInt ਨਿਵੇਸ਼_Max = get_Signed_Max_ਮੁੱਲ();
  ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);

  // Case 1: All values fit - just truncate
  ਜੇ (ਨਿਵੇਸ਼_Min.sge(Min_ਵਿੱਚ_Range) && ਨਿਵੇਸ਼_Max.sle(Max_ਵਿੱਚ_Range)) {
    ਗਿਆਤ = trunc(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  }
  // Case 2: All saturate to min
  ਨਹੀਂਤਾਂ ਜੇ (ਨਿਵੇਸ਼_Max.slt(Min_ਵਿੱਚ_Range)) {
    ਗਿਆਤ = ਗਿਆਤ_ਬਿੱਟ::makeConstant(APInt::get_Signed_Min_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2));
  }
  // Case 3: All saturate to max
  ਨਹੀਂਤਾਂ ਜੇ (ਨਿਵੇਸ਼_Min.sgt(Max_ਵਿੱਚ_Range)) {
    ਗਿਆਤ = ਗਿਆਤ_ਬਿੱਟ::makeConstant(APInt::get_Signed_Max_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  ਨਹੀਂਤਾਂ ਜੇ (ਨਿਵੇਸ਼_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    ਗਿਆਤ.Zero = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
    ਗਿਆਤ.Zero.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    ਗਿਆਤ.One = One.trunc(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
    ਗਿਆਤ.One.clear_ਦੋਆਧਾਰੀ_ਅੰਕ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  ਨਹੀਂਤਾਂ ਜੇ (ਨਿਵੇਸ਼_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    ਗਿਆਤ.Zero = Zero.trunc(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
    ਗਿਆਤ.Zero.clear_ਦੋਆਧਾਰੀ_ਅੰਕ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    ਗਿਆਤ.One = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
    ਗਿਆਤ.One.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  ਨਹੀਂਤਾਂ {
    // Output: min saturation, truncated, or max saturation
    APInt ਨਿਵੇਸ਼_Zero_ਹੇਠਲਾ = Zero.trunc(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
    APInt ਨਿਵੇਸ਼_One_ਹੇਠਲਾ = One.trunc(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
    APInt MinSat = APInt::get_Signed_Min_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
    APInt MaxSat = APInt::get_Signed_Max_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
    ਗਿਆਤ_ਬਿੱਟ MinSatKB = ਗਿਆਤ_ਬਿੱਟ::makeConstant(MinSat);
    ਗਿਆਤ_ਬਿੱਟ MaxSatKB = ਗਿਆਤ_ਬਿੱਟ::makeConstant(MaxSat);
    ਜੇ (ਨਿਵੇਸ਼_Max.sle(Max_ਵਿੱਚ_Range)) {
      // Positive values fit, only negatives saturate to min
      ਗਿਆਤ.Zero = ਨਿਵੇਸ਼_Zero_ਹੇਠਲਾ & MinSatKB.Zero;
      ਗਿਆਤ.One = ਨਿਵੇਸ਼_One_ਹੇਠਲਾ & MinSatKB.One;
    } ਨਹੀਂਤਾਂ ਜੇ (ਨਿਵੇਸ਼_Min.sge(Min_ਵਿੱਚ_Range)) {
      // Negative values fit, only positives saturate to max
      ਗਿਆਤ.Zero = ਨਿਵੇਸ਼_Zero_ਹੇਠਲਾ & MaxSatKB.Zero;
      ਗਿਆਤ.One = ਨਿਵੇਸ਼_One_ਹੇਠਲਾ & MaxSatKB.One;
    } ਨਹੀਂਤਾਂ {
      // Both positive and negative values might saturate
      ਗਿਆਤ.Zero = ਨਿਵੇਸ਼_Zero_ਹੇਠਲਾ & MinSatKB.Zero & MaxSatKB.Zero;
      ਗਿਆਤ.One = ਨਿਵੇਸ਼_One_ਹੇਠਲਾ & MinSatKB.One & MaxSatKB.One;
    }
  }
  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::truncSSatU(ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਿਵੇਸ਼_ਬਿੱਟ = get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  APInt Max_ਵਿੱਚ_Range = APInt::getAllOnes(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2).zext(ਨਿਵੇਸ਼_ਬਿੱਟ);
  APInt ਨਿਵੇਸ਼_Min = get_Signed_Min_ਮੁੱਲ();
  APInt ਨਿਵੇਸ਼_Max = get_Signed_Max_ਮੁੱਲ();
  ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);

  ਜੇ (isNegative()) {
    ਗਿਆਤ.ਸੈੱਟ_All_Zero();
  } ਨਹੀਂਤਾਂ ਜੇ (ਨਿਵੇਸ਼_Min.isNonNegative() && ਨਿਵੇਸ਼_Max.ule(Max_ਵਿੱਚ_Range)) {
    ਗਿਆਤ = trunc(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  } ਨਹੀਂਤਾਂ ਜੇ (ਨਿਵੇਸ਼_Min.isNonNegative() && ਨਿਵੇਸ਼_Min.ugt(Max_ਵਿੱਚ_Range)) {
    ਗਿਆਤ.ਸੈੱਟ_All_Ones();
  } ਨਹੀਂਤਾਂ ਜੇ (ਨਿਵੇਸ਼_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    ਗਿਆਤ.Zero = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
    // Common one bits: bits that are 1 in input stay 1
    ਗਿਆਤ.One = One.trunc(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  } ਨਹੀਂਤਾਂ {
    // Mixed: sign bit unknown
    ਜੇ (ਨਿਵੇਸ਼_Max.ule(Max_ਵਿੱਚ_Range)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      ਗਿਆਤ.Zero = Zero.trunc(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
      ਗਿਆਤ.One = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
    } ਨਹੀਂਤਾਂ {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      ਗਿਆਤ.Zero = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
      ਗਿਆਤ.One = APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0);
    }
  }
  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::truncUSat(ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਿਵੇਸ਼_ਬਿੱਟ = get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  APInt Max_ਵਿੱਚ_Range = APInt::get_ਨੀਵਾਂ_ਬਿੱਟ_ਸੈੱਟ(ਨਿਵੇਸ਼_ਬਿੱਟ, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  APInt ਨਿਵੇਸ਼_Max = get_Max_ਮੁੱਲ();
  APInt ਨਿਵੇਸ਼_Min = get_Min_ਮੁੱਲ();
  ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);

  ਜੇ (ਨਿਵੇਸ਼_Max.ule(Max_ਵਿੱਚ_Range)) {
    ਗਿਆਤ = trunc(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  } ਨਹੀਂਤਾਂ ਜੇ (ਨਿਵੇਸ਼_Min.ugt(Max_ਵਿੱਚ_Range)) {
    ਗਿਆਤ.ਸੈੱਟ_All_Ones();
  } ਨਹੀਂਤਾਂ {
    ਗਿਆਤ.resetAll();
    ਗਿਆਤ.One = One.trunc(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  }
  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::sext_ਵਿੱਚ_Reg(ਬਿਨਾਨਿਸ਼ਾਨੀ Src_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਪ੍ਰਤਿਜ੍ਞਾ(0 < Src_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ && Src_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ <= ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 &&
         "Illegal sext-in-register");

  ਜੇ (Src_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ == ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)
    ਵਾਪਸ *ਇਹ;

  ਬਿਨਾਨਿਸ਼ਾਨੀ Ext_ਬਿੱਟ = ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - Src_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ;
  ਗਿਆਤ_ਬਿੱਟ ਨਤੀਜਾ;
  ਨਤੀਜਾ.One = One << Ext_ਬਿੱਟ;
  ਨਤੀਜਾ.Zero = Zero << Ext_ਬਿੱਟ;
  ਨਤੀਜਾ.One.ashr_ਵਿੱਚ_ਰੱਖੋ(Ext_ਬਿੱਟ);
  ਨਤੀਜਾ.Zero.ashr_ਵਿੱਚ_ਰੱਖੋ(Ext_ਬਿੱਟ);
  ਵਾਪਸ ਨਤੀਜਾ;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::makeGE(ਅਚਲ APInt &Val) ਅਚਲ {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  ਬਿਨਾਨਿਸ਼ਾਨੀ N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_ਨੀਵਾਂ_ਬਿੱਟ(get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() - N);
  ਵਾਪਸ ਗਿਆਤ_ਬਿੱਟ(Zero, One | MaskedVal);
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::umax(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  ਜੇ (LHS.get_Min_ਮੁੱਲ().uge(RHS.get_Max_ਮੁੱਲ()))
    ਵਾਪਸ LHS;
  ਜੇ (RHS.get_Min_ਮੁੱਲ().uge(LHS.get_Max_ਮੁੱਲ()))
    ਵਾਪਸ RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  ਗਿਆਤ_ਬਿੱਟ L = LHS.makeGE(RHS.get_Min_ਮੁੱਲ());
  ਗਿਆਤ_ਬਿੱਟ R = RHS.makeGE(LHS.get_Min_ਮੁੱਲ());
  ਵਾਪਸ L.intersectWith(R);
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::umin(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  ਸਵੈਚਾਲਿਤ Flip = [](ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &Val) { ਵਾਪਸ ਗਿਆਤ_ਬਿੱਟ(Val.One, Val.Zero); };
  ਵਾਪਸ Flip(umax(Flip(LHS), Flip(RHS)));
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::smax(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਵਾਪਸ flip_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ(umax(flip_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ(LHS), flip_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ(RHS)));
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::smin(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  ਸਵੈਚਾਲਿਤ Flip = [](ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &Val) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Position = Val.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ_Val(Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Position, Val.Zero[Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Position]);
    One.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ_Val(Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Position, Val.One[Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Position]);
    ਵਾਪਸ ਗਿਆਤ_ਬਿੱਟ(Zero, One);
  };
  ਵਾਪਸ Flip(umax(Flip(LHS), Flip(RHS)));
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::abdu(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  ਜੇ (LHS.get_Min_ਮੁੱਲ().uge(RHS.get_Max_ਮੁੱਲ()))
    ਵਾਪਸ compute_For_ਜੋੜੋ_Sub(/*Add=*/ਝੂਠ, /*NSW=*/ਝੂਠ, /*NUW=*/ਝੂਠ, LHS,
                            RHS);
  ਜੇ (RHS.get_Min_ਮੁੱਲ().uge(LHS.get_Max_ਮੁੱਲ()))
    ਵਾਪਸ compute_For_ਜੋੜੋ_Sub(/*Add=*/ਝੂਠ, /*NSW=*/ਝੂਠ, /*NUW=*/ਝੂਠ, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  ਗਿਆਤ_ਬਿੱਟ Diff0 =
      compute_For_ਜੋੜੋ_Sub(/*Add=*/ਝੂਠ, /*NSW=*/ਝੂਠ, /*NUW=*/ਸੱਚ, LHS, RHS);
  ਗਿਆਤ_ਬਿੱਟ Diff1 =
      compute_For_ਜੋੜੋ_Sub(/*Add=*/ਝੂਠ, /*NSW=*/ਝੂਠ, /*NUW=*/ਸੱਚ, RHS, LHS);
  ਵਾਪਸ Diff0.intersectWith(Diff1);
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::abds(ਗਿਆਤ_ਬਿੱਟ LHS, ਗਿਆਤ_ਬਿੱਟ RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  ਜੇ (LHS.get_Signed_Min_ਮੁੱਲ().sge(RHS.get_Signed_Max_ਮੁੱਲ()))
    ਵਾਪਸ compute_For_ਜੋੜੋ_Sub(/*Add=*/ਝੂਠ, /*NSW=*/ਝੂਠ, /*NUW=*/ਝੂਠ, LHS,
                            RHS);
  ਜੇ (RHS.get_Signed_Min_ਮੁੱਲ().sge(LHS.get_Signed_Max_ਮੁੱਲ()))
    ਵਾਪਸ compute_For_ਜੋੜੋ_Sub(/*Add=*/ਝੂਠ, /*NSW=*/ਝੂਠ, /*NUW=*/ਝੂਠ, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  ਬਿਨਾਨਿਸ਼ਾਨੀ Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Position = LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() - 1;
  ਲਈ (ਸਵੈਚਾਲਿਤ Arg : {&LHS, &RHS}) {
    ਤਾਰਕਿਕ Tmp = Arg->Zero[Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Position];
    Arg->Zero.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ_Val(Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Position, Arg->One[Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Position]);
    Arg->One.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ_Val(Sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Position, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  ਗਿਆਤ_ਬਿੱਟ Diff0 =
      compute_For_ਜੋੜੋ_Sub(/*Add=*/ਝੂਠ, /*NSW=*/ਝੂਠ, /*NUW=*/ਸੱਚ, LHS, RHS);
  ਗਿਆਤ_ਬਿੱਟ Diff1 =
      compute_For_ਜੋੜੋ_Sub(/*Add=*/ਝੂਠ, /*NSW=*/ਝੂਠ, /*NUW=*/ਸੱਚ, RHS, LHS);
  ਵਾਪਸ Diff0.intersectWith(Diff1);
}

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ get_Max_ਖਿਸਕਾਓ_Amount(ਅਚਲ APInt &Max_ਮੁੱਲ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) {
  ਜੇ (isPowerOf2_32(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    ਵਾਪਸ Max_ਮੁੱਲ.ਕੱਢੋ_ਬਿੱਟ_As_ZExt_ਮੁੱਲ(
        std::min(Log2_32(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2), Max_ਮੁੱਲ.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ()), 0);
  // This is only an approximate upper bound.
  ਵਾਪਸ Max_ਮੁੱਲ.get_Limited_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1);
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::shl(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS, ਤਾਰਕਿਕ NUW,
                         ਤਾਰਕਿਕ NSW, ਤਾਰਕਿਕ ShAmtNonZero) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਸਵੈਚਾਲਿਤ ਖਿਸਕਾਓ_By_Const = [&](ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_2) {
    ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ;
    ਤਾਰਕਿਕ Shifted_ਬਾਹਰਲਾ_Zero, Shifted_ਬਾਹਰਲਾ_One;
    ਗਿਆਤ.Zero = LHS.Zero.ushl_ov(ਖਿਸਕਾਓ_Amt_2, Shifted_ਬਾਹਰਲਾ_Zero);
    ਗਿਆਤ.Zero.ਸੈੱਟ_ਨੀਵਾਂ_ਬਿੱਟ(ਖਿਸਕਾਓ_Amt_2);
    ਗਿਆਤ.One = LHS.One.ushl_ov(ਖਿਸਕਾਓ_Amt_2, Shifted_ਬਾਹਰਲਾ_One);

    // All cases returning poison have been handled by MaxShiftAmount already.
    ਜੇ (NSW) {
      ਜੇ (NUW && ਖਿਸਕਾਓ_Amt_2 != 0)
        // NUW means we can assume anything shifted out was a zero.
        Shifted_ਬਾਹਰਲਾ_Zero = ਸੱਚ;

      ਜੇ (Shifted_ਬਾਹਰਲਾ_Zero)
        ਗਿਆਤ.makeNonNegative();
      ਨਹੀਂਤਾਂ ਜੇ (Shifted_ਬਾਹਰਲਾ_One)
        ਗਿਆਤ.makeNegative();
    }
    ਵਾਪਸ ਗਿਆਤ;
  };

  // Fast path for a common case when LHS is completely unknown.
  ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  ਬਿਨਾਨਿਸ਼ਾਨੀ Min_ਖਿਸਕਾਓ_Amount = RHS.get_Min_ਮੁੱਲ().get_Limited_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  ਜੇ (Min_ਖਿਸਕਾਓ_Amount == 0 && ShAmtNonZero)
    Min_ਖਿਸਕਾਓ_Amount = 1;
  ਜੇ (LHS.isUnknown()) {
    ਗਿਆਤ.Zero.ਸੈੱਟ_ਨੀਵਾਂ_ਬਿੱਟ(Min_ਖਿਸਕਾਓ_Amount);
    ਜੇ (NUW && NSW && Min_ਖਿਸਕਾਓ_Amount != 0)
      ਗਿਆਤ.makeNonNegative();
    ਵਾਪਸ ਗਿਆਤ;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_ਮੁੱਲ = RHS.get_Max_ਮੁੱਲ();
  ਬਿਨਾਨਿਸ਼ਾਨੀ Max_ਖਿਸਕਾਓ_Amount = get_Max_ਖਿਸਕਾਓ_Amount(Max_ਮੁੱਲ, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  ਜੇ (NUW && NSW)
    Max_ਖਿਸਕਾਓ_Amount = std::min(Max_ਖਿਸਕਾਓ_Amount, LHS.ਗਿਣਤੀ_Max_Leading_Zeros() - 1);
  ਜੇ (NUW)
    Max_ਖਿਸਕਾਓ_Amount = std::min(Max_ਖਿਸਕਾਓ_Amount, LHS.ਗਿਣਤੀ_Max_Leading_Zeros());
  ਜੇ (NSW)
    Max_ਖਿਸਕਾਓ_Amount = std::min(
        Max_ਖਿਸਕਾਓ_Amount,
        std::max(LHS.ਗਿਣਤੀ_Max_Leading_Zeros(), LHS.ਗਿਣਤੀ_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  ਜੇ (Min_ਖਿਸਕਾਓ_Amount == 0 && Max_ਖਿਸਕਾਓ_Amount == ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1 &&
      isPowerOf2_32(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)) {
    ਗਿਆਤ.Zero.ਸੈੱਟ_ਨੀਵਾਂ_ਬਿੱਟ(LHS.ਗਿਣਤੀ_Min_Trailing_Zeros());
    ਜੇ (LHS.isAllOnes())
      ਗਿਆਤ.One.ਸੈੱਟ_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ();
    ਜੇ (NSW) {
      ਜੇ (LHS.isNonNegative())
        ਗਿਆਤ.makeNonNegative();
      ਜੇ (LHS.isNegative())
        ਗਿਆਤ.makeNegative();
    }
    ਵਾਪਸ ਗਿਆਤ;
  }

  // Find the common bits from all possible shifts.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_Zero_ਨਕਾਬ = RHS.Zero.zextOrTrunc(32).get_ZExt_ਮੁੱਲ();
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_One_ਨਕਾਬ = RHS.One.zextOrTrunc(32).get_ZExt_ਮੁੱਲ();
  ਗਿਆਤ.ਸੈੱਟ_All_Conflict();
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_2 = Min_ਖਿਸਕਾਓ_Amount; ਖਿਸਕਾਓ_Amt_2 <= Max_ਖਿਸਕਾਓ_Amount;
       ++ਖਿਸਕਾਓ_Amt_2) {
    // Skip if the shift amount is impossible.
    ਜੇ ((ਖਿਸਕਾਓ_Amt_Zero_ਨਕਾਬ & ਖਿਸਕਾਓ_Amt_2) != 0 ||
        (ਖਿਸਕਾਓ_Amt_One_ਨਕਾਬ | ਖਿਸਕਾਓ_Amt_2) != ਖਿਸਕਾਓ_Amt_2)
      ਜਾਰੀ;
    ਗਿਆਤ = ਗਿਆਤ.intersectWith(ਖਿਸਕਾਓ_By_Const(LHS, ਖਿਸਕਾਓ_Amt_2));
    ਜੇ (ਗਿਆਤ.isUnknown())
      ਰੋਕੋ;
  }

  // All shift amounts may result in poison.
  ਜੇ (ਗਿਆਤ.hasConflict())
    ਗਿਆਤ.ਸੈੱਟ_All_Zero();
  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::lshr(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS,
                          ਤਾਰਕਿਕ ShAmtNonZero, ਤਾਰਕਿਕ Exact) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਸਵੈਚਾਲਿਤ ਖਿਸਕਾਓ_By_Const = [&](ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_2) {
    ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ = LHS;
    ਗਿਆਤ >>= ਖਿਸਕਾਓ_Amt_2;
    // High bits are known zero.
    ਗਿਆਤ.Zero.ਸੈੱਟ_ਉੱਚਾ_ਬਿੱਟ(ਖਿਸਕਾਓ_Amt_2);
    ਵਾਪਸ ਗਿਆਤ;
  };

  // Fast path for a common case when LHS is completely unknown.
  ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  ਬਿਨਾਨਿਸ਼ਾਨੀ Min_ਖਿਸਕਾਓ_Amount = RHS.get_Min_ਮੁੱਲ().get_Limited_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  ਜੇ (Min_ਖਿਸਕਾਓ_Amount == 0 && ShAmtNonZero)
    Min_ਖਿਸਕਾਓ_Amount = 1;
  ਜੇ (LHS.isUnknown()) {
    ਗਿਆਤ.Zero.ਸੈੱਟ_ਉੱਚਾ_ਬਿੱਟ(Min_ਖਿਸਕਾਓ_Amount);
    ਵਾਪਸ ਗਿਆਤ;
  }

  // Find the common bits from all possible shifts.
  APInt Max_ਮੁੱਲ = RHS.get_Max_ਮੁੱਲ();
  ਬਿਨਾਨਿਸ਼ਾਨੀ Max_ਖਿਸਕਾਓ_Amount = get_Max_ਖਿਸਕਾਓ_Amount(Max_ਮੁੱਲ, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  ਜੇ (Exact) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪਹਿਲਾ_One = LHS.ਗਿਣਤੀ_Max_Trailing_Zeros();
    ਜੇ (ਪਹਿਲਾ_One < Min_ਖਿਸਕਾਓ_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      ਗਿਆਤ.ਸੈੱਟ_All_Zero();
      ਵਾਪਸ ਗਿਆਤ;
    }
    Max_ਖਿਸਕਾਓ_Amount = std::min(Max_ਖਿਸਕਾਓ_Amount, ਪਹਿਲਾ_One);
  }

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_Zero_ਨਕਾਬ = RHS.Zero.zextOrTrunc(32).get_ZExt_ਮੁੱਲ();
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_One_ਨਕਾਬ = RHS.One.zextOrTrunc(32).get_ZExt_ਮੁੱਲ();
  ਗਿਆਤ.ਸੈੱਟ_All_Conflict();
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_2 = Min_ਖਿਸਕਾਓ_Amount; ਖਿਸਕਾਓ_Amt_2 <= Max_ਖਿਸਕਾਓ_Amount;
       ++ਖਿਸਕਾਓ_Amt_2) {
    // Skip if the shift amount is impossible.
    ਜੇ ((ਖਿਸਕਾਓ_Amt_Zero_ਨਕਾਬ & ਖਿਸਕਾਓ_Amt_2) != 0 ||
        (ਖਿਸਕਾਓ_Amt_One_ਨਕਾਬ | ਖਿਸਕਾਓ_Amt_2) != ਖਿਸਕਾਓ_Amt_2)
      ਜਾਰੀ;
    ਗਿਆਤ = ਗਿਆਤ.intersectWith(ਖਿਸਕਾਓ_By_Const(LHS, ਖਿਸਕਾਓ_Amt_2));
    ਜੇ (ਗਿਆਤ.isUnknown())
      ਰੋਕੋ;
  }

  // All shift amounts may result in poison.
  ਜੇ (ਗਿਆਤ.hasConflict())
    ਗਿਆਤ.ਸੈੱਟ_All_Zero();
  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::ashr(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS,
                          ਤਾਰਕਿਕ ShAmtNonZero, ਤਾਰਕਿਕ Exact) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਸਵੈਚਾਲਿਤ ਖਿਸਕਾਓ_By_Const = [&](ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_2) {
    ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ = LHS;
    ਗਿਆਤ.Zero.ashr_ਵਿੱਚ_ਰੱਖੋ(ਖਿਸਕਾਓ_Amt_2);
    ਗਿਆਤ.One.ashr_ਵਿੱਚ_ਰੱਖੋ(ਖਿਸਕਾਓ_Amt_2);
    ਵਾਪਸ ਗਿਆਤ;
  };

  // Fast path for a common case when LHS is completely unknown.
  ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  ਬਿਨਾਨਿਸ਼ਾਨੀ Min_ਖਿਸਕਾਓ_Amount = RHS.get_Min_ਮੁੱਲ().get_Limited_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  ਜੇ (Min_ਖਿਸਕਾਓ_Amount == 0 && ShAmtNonZero)
    Min_ਖਿਸਕਾਓ_Amount = 1;
  ਜੇ (LHS.isUnknown()) {
    ਜੇ (Min_ਖਿਸਕਾਓ_Amount == ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) {
      // Always poison. Return zero because we don't like returning conflict.
      ਗਿਆਤ.ਸੈੱਟ_All_Zero();
      ਵਾਪਸ ਗਿਆਤ;
    }
    ਵਾਪਸ ਗਿਆਤ;
  }

  // Find the common bits from all possible shifts.
  APInt Max_ਮੁੱਲ = RHS.get_Max_ਮੁੱਲ();
  ਬਿਨਾਨਿਸ਼ਾਨੀ Max_ਖਿਸਕਾਓ_Amount = get_Max_ਖਿਸਕਾਓ_Amount(Max_ਮੁੱਲ, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  ਜੇ (Exact) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਪਹਿਲਾ_One = LHS.ਗਿਣਤੀ_Max_Trailing_Zeros();
    ਜੇ (ਪਹਿਲਾ_One < Min_ਖਿਸਕਾਓ_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      ਗਿਆਤ.ਸੈੱਟ_All_Zero();
      ਵਾਪਸ ਗਿਆਤ;
    }
    Max_ਖਿਸਕਾਓ_Amount = std::min(Max_ਖਿਸਕਾਓ_Amount, ਪਹਿਲਾ_One);
  }

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_Zero_ਨਕਾਬ = RHS.Zero.zextOrTrunc(32).get_ZExt_ਮੁੱਲ();
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_One_ਨਕਾਬ = RHS.One.zextOrTrunc(32).get_ZExt_ਮੁੱਲ();
  ਗਿਆਤ.ਸੈੱਟ_All_Conflict();
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਖਿਸਕਾਓ_Amt_2 = Min_ਖਿਸਕਾਓ_Amount; ਖਿਸਕਾਓ_Amt_2 <= Max_ਖਿਸਕਾਓ_Amount;
      ++ਖਿਸਕਾਓ_Amt_2) {
    // Skip if the shift amount is impossible.
    ਜੇ ((ਖਿਸਕਾਓ_Amt_Zero_ਨਕਾਬ & ਖਿਸਕਾਓ_Amt_2) != 0 ||
        (ਖਿਸਕਾਓ_Amt_One_ਨਕਾਬ | ਖਿਸਕਾਓ_Amt_2) != ਖਿਸਕਾਓ_Amt_2)
      ਜਾਰੀ;
    ਗਿਆਤ = ਗਿਆਤ.intersectWith(ਖਿਸਕਾਓ_By_Const(LHS, ਖਿਸਕਾਓ_Amt_2));
    ਜੇ (ਗਿਆਤ.isUnknown())
      ਰੋਕੋ;
  }

  // All shift amounts may result in poison.
  ਜੇ (ਗਿਆਤ.hasConflict())
    ਗਿਆਤ.ਸੈੱਟ_All_Zero();
  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::fshl(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS,
                          ਅਚਲ APInt &Amt) {
  ਵਾਪਸ ਗਿਆਤ_ਬਿੱਟ(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::fshr(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS,
                          ਅਚਲ APInt &Amt) {
  ਵਾਪਸ ਗਿਆਤ_ਬਿੱਟ(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::clmul(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਗਿਆਤ_ਬਿੱਟ Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_ਮੁੱਲ(), RHS.get_Min_ਮੁੱਲ()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  ਸਵੈਚਾਲਿਤ ClMulOr = [](ਅਚਲ APInt &LHS, ਅਚਲ APInt &RHS) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ BW = LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
    ਪ੍ਰਤਿਜ੍ਞਾ(BW == RHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() && "Operand mismatch");
    APInt ਨਤੀਜਾ(BW, 0);
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I :
         seq(std::min(RHS.get_ਸਰਗਰਮ_ਬਿੱਟ(), BW - LHS.countr_zero())))
      ਜੇ (RHS[I])
        ਨਤੀਜਾ |= LHS << I;
    ਵਾਪਸ ਨਤੀਜਾ;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt ਗਿਆਤ = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= ਗਿਆਤ;
  Res.One &= ਗਿਆਤ;

  ਵਾਪਸ Res;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::pext(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &Val, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &ਨਕਾਬ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = Val.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਗਿਆਤ_ਬਿੱਟ Res(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.ਸੈੱਟ_All_Conflict();

  // For each source position I where Mask[I] could be 1, the output position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible output positions widens.
  ਬਿਨਾਨਿਸ਼ਾਨੀ M0 = 0, M1 = 0;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I < ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2; ++I) {
    ਜੇ (!ਨਕਾਬ.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      ਜੇ (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clear_ਬਿੱਟ(M0, M1 + 1);
      ਜੇ (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clear_ਬਿੱਟ(M0, M1 + 1);
    }
    ਜੇ (ਨਕਾਬ.One[I])
      ++M0, ++M1;
    ਨਹੀਂਤਾਂ ਜੇ (!ਨਕਾਬ.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clear_ਬਿੱਟ(M0, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  ਵਾਪਸ Res;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::pdep(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &Val, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &ਨਕਾਬ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = Val.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਗਿਆਤ_ਬਿੱਟ Res(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.ਸੈੱਟ_All_Conflict();

  // For each output position I where Mask[I] could be 1, the source position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible source positions widens.
  ਬਿਨਾਨਿਸ਼ਾਨੀ M0 = 0, M1 = 0;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I < ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2; ++I) {
    ਜੇ (!ਨਕਾਬ.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clear_ਦੋਆਧਾਰੀ_ਅੰਕ(I);
    ਜੇ (!ਨਕਾਬ.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::get_ਬਿੱਟ_ਸੈੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, M0, M1 + 1);
      ਜੇ (!Range.is_Subset_ਦਾ(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clear_ਦੋਆਧਾਰੀ_ਅੰਕ(I);
      ਜੇ (!Range.is_Subset_ਦਾ(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clear_ਦੋਆਧਾਰੀ_ਅੰਕ(I);
    }
    ਜੇ (ਨਕਾਬ.One[I])
      ++M0, ++M1;
    ਨਹੀਂਤਾਂ ਜੇ (!ਨਕਾਬ.Zero[I])
      ++M1;
  }
  ਵਾਪਸ Res;
}

std::optional<ਤਾਰਕਿਕ> ਗਿਆਤ_ਬਿੱਟ::eq(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਜੇ (LHS.isConstant() && RHS.isConstant())
    ਵਾਪਸ LHS.getConstant() == RHS.getConstant();
  ਜੇ (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    ਵਾਪਸ ਝੂਠ;
  ਵਾਪਸ std::nullopt;
}

std::optional<ਤਾਰਕਿਕ> ਗਿਆਤ_ਬਿੱਟ::ne(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਜੇ (std::optional<ਤਾਰਕਿਕ> ਗਿਆਤ_EQ = eq(LHS, RHS))
    ਵਾਪਸ !*ਗਿਆਤ_EQ;
  ਵਾਪਸ std::nullopt;
}

std::optional<ਤਾਰਕਿਕ> ਗਿਆਤ_ਬਿੱਟ::ugt(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  ਜੇ (LHS.get_Max_ਮੁੱਲ().ule(RHS.get_Min_ਮੁੱਲ()))
    ਵਾਪਸ ਝੂਠ;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  ਜੇ (LHS.get_Min_ਮੁੱਲ().ugt(RHS.get_Max_ਮੁੱਲ()))
    ਵਾਪਸ ਸੱਚ;
  ਵਾਪਸ std::nullopt;
}

std::optional<ਤਾਰਕਿਕ> ਗਿਆਤ_ਬਿੱਟ::uge(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਜੇ (std::optional<ਤਾਰਕਿਕ> IsUGT = ugt(RHS, LHS))
    ਵਾਪਸ !*IsUGT;
  ਵਾਪਸ std::nullopt;
}

std::optional<ਤਾਰਕਿਕ> ਗਿਆਤ_ਬਿੱਟ::ult(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਵਾਪਸ ugt(RHS, LHS);
}

std::optional<ਤਾਰਕਿਕ> ਗਿਆਤ_ਬਿੱਟ::ule(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਵਾਪਸ uge(RHS, LHS);
}

std::optional<ਤਾਰਕਿਕ> ਗਿਆਤ_ਬਿੱਟ::sgt(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  ਜੇ (LHS.get_Signed_Max_ਮੁੱਲ().sle(RHS.get_Signed_Min_ਮੁੱਲ()))
    ਵਾਪਸ ਝੂਠ;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  ਜੇ (LHS.get_Signed_Min_ਮੁੱਲ().sgt(RHS.get_Signed_Max_ਮੁੱਲ()))
    ਵਾਪਸ ਸੱਚ;
  ਵਾਪਸ std::nullopt;
}

std::optional<ਤਾਰਕਿਕ> ਗਿਆਤ_ਬਿੱਟ::sge(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਜੇ (std::optional<ਤਾਰਕਿਕ> ਗਿਆਤ_SGT = sgt(RHS, LHS))
    ਵਾਪਸ !*ਗਿਆਤ_SGT;
  ਵਾਪਸ std::nullopt;
}

std::optional<ਤਾਰਕਿਕ> ਗਿਆਤ_ਬਿੱਟ::slt(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਵਾਪਸ sgt(RHS, LHS);
}

std::optional<ਤਾਰਕਿਕ> ਗਿਆਤ_ਬਿੱਟ::sle(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਵਾਪਸ sge(RHS, LHS);
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::ਨਿਰਪੇਖ(ਤਾਰਕਿਕ IntMinIsPoison) ਅਚਲ {
  // If the source's MSB is zero then we know the rest of the bits already.
  ਜੇ (isNonNegative())
    ਵਾਪਸ *ਇਹ;

  // Absolute value preserves trailing zero count.
  ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_Abs(get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());

  // If the input is negative, then abs(x) == -x.
  ਜੇ (isNegative()) {
    ਗਿਆਤ_ਬਿੱਟ Tmp = *ਇਹ;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    ਜੇ (IntMinIsPoison && (Zero.popcount() + 2) == get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ())
      Tmp.One.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(ਗਿਣਤੀ_Min_Trailing_Zeros());

    ਗਿਆਤ_Abs = compute_For_ਜੋੜੋ_Sub(
        /*ਜੋੜੋ*/ ਝੂਠ, IntMinIsPoison, /*NUW=*/ਝੂਠ,
        ਗਿਆਤ_ਬਿੱਟ::makeConstant(APInt(get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    ਜੇ (IntMinIsPoison && Tmp.ਗਿਣਤੀ_Min_Population() == 1 &&
        Tmp.ਗਿਣਤੀ_Max_Population() != 1) {
      Tmp.One.clear_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ();
      Tmp.Zero.ਸੈੱਟ_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ();
      ਗਿਆਤ_Abs.One.ਸੈੱਟ_ਬਿੱਟ(get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() - Tmp.ਗਿਣਤੀ_Min_Leading_Zeros(),
                           get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() - 1);
    }

  } ਨਹੀਂਤਾਂ {
    ਬਿਨਾਨਿਸ਼ਾਨੀ MaxTZ = ਗਿਣਤੀ_Max_Trailing_Zeros();
    ਬਿਨਾਨਿਸ਼ਾਨੀ MinTZ = ਗਿਣਤੀ_Min_Trailing_Zeros();

    ਗਿਆਤ_Abs.Zero.ਸੈੱਟ_ਨੀਵਾਂ_ਬਿੱਟ(MinTZ);
    // If we know the lowest set 1, then preserve it.
    ਜੇ (MaxTZ == MinTZ && MaxTZ < get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ())
      ਗਿਆਤ_Abs.One.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    ਜੇ (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_ਮੁੱਲ())) {
      ਗਿਆਤ_Abs.One.clear_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ();
      ਗਿਆਤ_Abs.Zero.ਸੈੱਟ_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ();
    }
  }

  ਵਾਪਸ ਗਿਆਤ_Abs;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::reduce_ਜੋੜੋ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Elts) ਅਚਲ {
  ਜੇ (ਅੰਕ_Elts == 0)
    ਵਾਪਸ ਗਿਆਤ_ਬਿੱਟ(get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਗਿਆਤ_ਬਿੱਟ ਨਤੀਜਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);

  ਜੇ (isConstant())
    // If all elements are the same constant, we can simply compute it
    ਵਾਪਸ ਗਿਆਤ_ਬਿੱਟ::makeConstant(ਅੰਕ_Elts * getConstant());

  // The main idea is as follows.
  //
  // If KnownBits for each element has L leading zeros then
  // X_i < 2^(W - L) for every i from [1, N].
  //
  //   ADD X_i <= ADD max(X_i) = N * max(X_i)
  //           <  N * 2^(W - L)
  //           <  2^(W - L + ceil(log2(N)))
  //
  // As the result, we can conclude that
  //
  //   L' = L - ceil(log2(N))
  //
  // Similar logic can be applied to leading ones.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਗੁਆਚਾ_ਬਿੱਟ = Log2_32_Ceil(ਅੰਕ_Elts);

  ਜੇ (isNonNegative()) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ LeadingZeros = ਗਿਣਤੀ_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > ਗੁਆਚਾ_ਬਿੱਟ ? LeadingZeros - ਗੁਆਚਾ_ਬਿੱਟ : 0;
    ਨਤੀਜਾ.Zero.ਸੈੱਟ_ਉੱਚਾ_ਬਿੱਟ(LeadingZeros);
  } ਨਹੀਂਤਾਂ ਜੇ (isNegative()) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ LeadingOnes = ਗਿਣਤੀ_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > ਗੁਆਚਾ_ਬਿੱਟ ? LeadingOnes - ਗੁਆਚਾ_ਬਿੱਟ : 0;
    ਨਤੀਜਾ.One.ਸੈੱਟ_ਉੱਚਾ_ਬਿੱਟ(LeadingOnes);
  }

  ਵਾਪਸ ਨਤੀਜਾ;
}

ਸਥਿਰ ਗਿਆਤ_ਬਿੱਟ compute_For_Sat_ਜੋੜੋ_Sub(ਤਾਰਕਿਕ ਜੋੜੋ_2, ਤਾਰਕਿਕ Signed,
                                     ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS,
                                     ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();

  std::optional<ਤਾਰਕਿਕ> ਉਛਾਲ;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  ਤਾਰਕਿਕ MayNegClamp = ਸੱਚ;
  ਤਾਰਕਿਕ MayPosClamp = ਸੱਚ;
  ਜੇ (Signed) {
    // Easy cases we can rule out any overflow.
    ਜੇ (ਜੋੜੋ_2 && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      ਉਛਾਲ = ਝੂਠ;
    ਨਹੀਂਤਾਂ ਜੇ (!ਜੋੜੋ_2 && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      ਉਛਾਲ = ਝੂਠ;
    ਨਹੀਂਤਾਂ {
      // Check if we may overflow. If we can't rule out overflow then check if
      // we can rule out a direction at least.
      ਗਿਆਤ_ਬਿੱਟ UnsignedLHS = LHS;
      ਗਿਆਤ_ਬਿੱਟ UnsignedRHS = RHS;
      // Get version of LHS/RHS with clearer signbit. This allows us to detect
      // how the addition/subtraction might overflow into the signbit. Then
      // using the actual known signbits of LHS/RHS, we can figure out which
      // overflows are/aren't possible.
      UnsignedLHS.One.clear_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ();
      UnsignedLHS.Zero.ਸੈੱਟ_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ();
      UnsignedRHS.One.clear_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ();
      UnsignedRHS.Zero.ਸੈੱਟ_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ();
      ਗਿਆਤ_ਬਿੱਟ Res =
          ਗਿਆਤ_ਬਿੱਟ::compute_For_ਜੋੜੋ_Sub(ਜੋੜੋ_2, /*NSW=*/ਝੂਠ,
                                      /*NUW=*/ਝੂਠ, UnsignedLHS, UnsignedRHS);
      ਜੇ (ਜੋੜੋ_2) {
        ਜੇ (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = ਝੂਠ;
          // Pos + Pos will overflow with extra signbit.
          ਜੇ (LHS.isNonNegative() && RHS.isNonNegative())
            ਉਛਾਲ = ਸੱਚ;
        } ਨਹੀਂਤਾਂ ਜੇ (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = ਝੂਠ;
          // Neg + Neg will overflow without extra signbit.
          ਜੇ (LHS.isNegative() && RHS.isNegative())
            ਉਛਾਲ = ਸੱਚ;
        }
        // We will never clamp to the opposite sign of N-bit result.
        ਜੇ (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = ਝੂਠ;
        ਜੇ (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = ਝੂਠ;
      } ਨਹੀਂਤਾਂ {
        ਜੇ (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = ਝੂਠ;
          // Neg - Pos will overflow with extra signbit.
          ਜੇ (LHS.isNegative() && RHS.isNonNegative())
            ਉਛਾਲ = ਸੱਚ;
        } ਨਹੀਂਤਾਂ ਜੇ (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = ਝੂਠ;
          // Pos - Neg will overflow without extra signbit.
          ਜੇ (LHS.isNonNegative() && RHS.isNegative())
            ਉਛਾਲ = ਸੱਚ;
        }
        // We will never clamp to the opposite sign of N-bit result.
        ਜੇ (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = ਝੂਠ;
        ਜੇ (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = ਝੂਠ;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    ਜੇ (!MayNegClamp && !MayPosClamp)
      ਉਛਾਲ = ਝੂਠ;
  } ਨਹੀਂਤਾਂ ਜੇ (ਜੋੜੋ_2) {
    // uadd.sat
    ਤਾਰਕਿਕ ਦਾ;
    (ਖਾਲੀ)LHS.get_Max_ਮੁੱਲ().uadd_ov(RHS.get_Max_ਮੁੱਲ(), ਦਾ);
    ਜੇ (!ਦਾ) {
      ਉਛਾਲ = ਝੂਠ;
    } ਨਹੀਂਤਾਂ {
      (ਖਾਲੀ)LHS.get_Min_ਮੁੱਲ().uadd_ov(RHS.get_Min_ਮੁੱਲ(), ਦਾ);
      ਜੇ (ਦਾ)
        ਉਛਾਲ = ਸੱਚ;
    }
  } ਨਹੀਂਤਾਂ {
    // usub.sat
    ਤਾਰਕਿਕ ਦਾ;
    (ਖਾਲੀ)LHS.get_Min_ਮੁੱਲ().usub_ov(RHS.get_Max_ਮੁੱਲ(), ਦਾ);
    ਜੇ (!ਦਾ) {
      ਉਛਾਲ = ਝੂਠ;
    } ਨਹੀਂਤਾਂ {
      (ਖਾਲੀ)LHS.get_Max_ਮੁੱਲ().usub_ov(RHS.get_Min_ਮੁੱਲ(), ਦਾ);
      ਜੇ (ਦਾ)
        ਉਛਾਲ = ਸੱਚ;
    }
  }

  ਗਿਆਤ_ਬਿੱਟ Res = ਗਿਆਤ_ਬਿੱਟ::compute_For_ਜੋੜੋ_Sub(ਜੋੜੋ_2, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  ਜੇ (ਉਛਾਲ) {
    // We know whether or not we overflowed.
    ਜੇ (!(*ਉਛਾਲ)) {
      // No overflow.
      ਵਾਪਸ Res;
    }

    // We overflowed
    APInt C;
    ਜੇ (Signed) {
      // sadd.sat / ssub.sat
      ਪ੍ਰਤਿਜ੍ਞਾ(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)
                           : APInt::get_Signed_Max_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
    } ਨਹੀਂਤਾਂ ਜੇ (ਜੋੜੋ_2) {
      // uadd.sat
      C = APInt::get_Max_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
    } ਨਹੀਂਤਾਂ {
      // uadd.sat
      C = APInt::get_Min_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
    }

    Res.One = C;
    Res.Zero = ~C;
    ਵਾਪਸ Res;
  }

  // We don't know if we overflowed.
  ਜੇ (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    ਜੇ (MayPosClamp)
      Res.Zero.clear_ਨੀਵਾਂ_ਬਿੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1);
    ਜੇ (MayNegClamp)
      Res.One.clear_ਨੀਵਾਂ_ਬਿੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1);
  } ਨਹੀਂਤਾਂ ਜੇ (ਜੋੜੋ_2) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clear_All_ਬਿੱਟ();
  } ਨਹੀਂਤਾਂ {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clear_All_ਬਿੱਟ();
  }

  ਵਾਪਸ Res;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::sadd_sat(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਵਾਪਸ compute_For_Sat_ਜੋੜੋ_Sub(/*ਜੋੜੋ*/ ਸੱਚ, /*Signed*/ ਸੱਚ, LHS, RHS);
}
ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::ssub_sat(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਵਾਪਸ compute_For_Sat_ਜੋੜੋ_Sub(/*ਜੋੜੋ*/ ਝੂਠ, /*Signed*/ ਸੱਚ, LHS, RHS);
}
ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::uadd_sat(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਵਾਪਸ compute_For_Sat_ਜੋੜੋ_Sub(/*ਜੋੜੋ*/ ਸੱਚ, /*Signed*/ ਝੂਠ, LHS, RHS);
}
ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::usub_sat(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਵਾਪਸ compute_For_Sat_ਜੋੜੋ_Sub(/*ਜੋੜੋ*/ ਝੂਠ, /*Signed*/ ਝੂਠ, LHS, RHS);
}

ਸਥਿਰ ਗਿਆਤ_ਬਿੱਟ avgComputeU(ਗਿਆਤ_ਬਿੱਟ LHS, ਗਿਆਤ_ਬਿੱਟ RHS, ਤਾਰਕਿਕ IsCeil) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  LHS = LHS.zext(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 + 1);
  RHS = RHS.zext(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 + 1);
  LHS =
      compute_For_ਜੋੜੋ_ਹਾਸਲ(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.ਕੱਢੋ_ਬਿੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 1);
  ਵਾਪਸ LHS;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::avgFloorS(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਵਾਪਸ flip_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ(avgFloorU(flip_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ(LHS), flip_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ(RHS)));
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::avgFloorU(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਵਾਪਸ avgComputeU(LHS, RHS, /*IsCeil=*/ਝੂਠ);
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::avgCeilS(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਵਾਪਸ flip_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ(avgCeilU(flip_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ(LHS), flip_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ(RHS)));
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::avgCeilU(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਵਾਪਸ avgComputeU(LHS, RHS, /*IsCeil=*/ਸੱਚ);
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::mul(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS,
                         ਤਾਰਕਿਕ NoUndefSelfMultiply) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਪ੍ਰਤਿਜ੍ਞਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == RHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() && "Operand mismatch");
  ਪ੍ਰਤਿਜ੍ਞਾ((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.get_Max_ਮੁੱਲ();
  APInt UMaxRHS = RHS.get_Max_ਮੁੱਲ();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  ਤਾਰਕਿਕ Has_ਉਛਾਲ;
  APInt UMax_ਨਤੀਜਾ = UMaxLHS.umul_ov(UMaxRHS, Has_ਉਛਾਲ);
  ਬਿਨਾਨਿਸ਼ਾਨੀ LeadZ = Has_ਉਛਾਲ ? 0 : UMax_ਨਤੀਜਾ.countl_zero();

  // The result of the bottom bits of an integer multiply can be
  // inferred by looking at the bottom bits of both operands and
  // multiplying them together.
  // We can infer at least the minimum number of known trailing bits
  // of both operands. Depending on number of trailing zeros, we can
  // infer more bits, because (a*b) <=> ((a/m) * (b/n)) * (m*n) assuming
  // a and b are divisible by m and n respectively.
  // We then calculate how many of those bits are inferrable and set
  // the output. For example, the i8 mul:
  //  a = XXXX1100 (12)
  //  b = XXXX1110 (14)
  // We know the bottom 3 bits are zero since the first can be divided by
  // 4 and the second by 2, thus having ((12/4) * (14/2)) * (2*4).
  // Applying the multiplication to the trimmed arguments gets:
  //    XX11 (3)
  //    X111 (7)
  // -------
  //    XX11
  //   XX11
  //  XX11
  // XX11
  // -------
  // XXXXX01
  // Which allows us to infer the 2 LSBs. Since we're multiplying the result
  // by 8, the bottom 3 bits will be 0, so we can infer a total of 5 bits.
  // The proof for this can be described as:
  // Pre: (C1 >= 0) && (C1 < (1 << C5)) && (C2 >= 0) && (C2 < (1 << C6)) &&
  //      (C7 == (1 << (umin(countTrailingZeros(C1), C5) +
  //                    umin(countTrailingZeros(C2), C6) +
  //                    umin(C5 - umin(countTrailingZeros(C1), C5),
  //                         C6 - umin(countTrailingZeros(C2), C6)))) - 1)
  // %aa = shl i8 %a, C5
  // %bb = shl i8 %b, C6
  // %aaa = or i8 %aa, C1
  // %bbb = or i8 %bb, C2
  // %mul = mul i8 %aaa, %bbb
  // %mask = and i8 %mul, C7
  //   =>
  // %mask = i8 ((C1*C2)&C7)
  // Where C5, C6 describe the known bits of %a, %b
  // C1, C2 describe the known bottom bits of %a, %b.
  // C7 describes the mask of the known bits of the result.

  // How many times we'd be able to divide each argument by 2 (shr by 1).
  // This gives us the number of trailing zeros on the multiplication result.
  ਬਿਨਾਨਿਸ਼ਾਨੀ Trail_ਬਿੱਟ_ਗਿਆਤ_LHS = (LHS.Zero | LHS.One).countr_one();
  ਬਿਨਾਨਿਸ਼ਾਨੀ Trail_ਬਿੱਟ_ਗਿਆਤ_RHS = (RHS.Zero | RHS.One).countr_one();
  ਬਿਨਾਨਿਸ਼ਾਨੀ TrailZeroLHS = LHS.ਗਿਣਤੀ_Min_Trailing_Zeros();
  ਬਿਨਾਨਿਸ਼ਾਨੀ TrailZeroRHS = RHS.ਗਿਣਤੀ_Min_Trailing_Zeros();
  ਬਿਨਾਨਿਸ਼ਾਨੀ TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਭ_ਤੋਂ_ਛੋਟਾ_ਸੰਕਾਰਜ = std::min(Trail_ਬਿੱਟ_ਗਿਆਤ_LHS - TrailZeroLHS,
                                      Trail_ਬਿੱਟ_ਗਿਆਤ_RHS - TrailZeroRHS);
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਤੀਜਾ_ਬਿੱਟ_ਗਿਆਤ = std::min(ਸਭ_ਤੋਂ_ਛੋਟਾ_ਸੰਕਾਰਜ + TrailZ, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);

  // The lower ResultBitsKnown bits of this are known.
  APInt Bottom_ਗਿਆਤ = LHS.One * RHS.One;

  ਗਿਆਤ_ਬਿੱਟ Res(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  Res.Zero.ਸੈੱਟ_ਉੱਚਾ_ਬਿੱਟ(LeadZ);
  Res.Zero |= (~Bottom_ਗਿਆਤ).get_Lo_ਬਿੱਟ(ਨਤੀਜਾ_ਬਿੱਟ_ਗਿਆਤ);
  Res.One = Bottom_ਗਿਆਤ.get_Lo_ਬਿੱਟ(ਨਤੀਜਾ_ਬਿੱਟ_ਗਿਆਤ);

  ਜੇ (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    ਬਿਨਾਨਿਸ਼ਾਨੀ TwoTZP1 = 2 * TrailZeroLHS + 1;
    ਜੇ (TwoTZP1 < ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)
      Res.Zero.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    ਜੇ (TrailZeroLHS < ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 && LHS.One[TrailZeroLHS]) {
      ਬਿਨਾਨਿਸ਼ਾਨੀ TwoTZP2 = TwoTZP1 + 1;
      ਜੇ (TwoTZP2 < ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)
        Res.Zero.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(TwoTZP2);
    }
  }

  ਵਾਪਸ Res;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::mulhs(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਪ੍ਰਤਿਜ੍ਞਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == RHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() && "Operand mismatch");
  ਗਿਆਤ_ਬਿੱਟ WideLHS = LHS.sext(2 * ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  ਗਿਆਤ_ਬਿੱਟ WideRHS = RHS.sext(2 * ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  ਵਾਪਸ mul(WideLHS, WideRHS).ਕੱਢੋ_ਬਿੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::mulhu(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਪ੍ਰਤਿਜ੍ਞਾ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 == RHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() && "Operand mismatch");
  ਗਿਆਤ_ਬਿੱਟ WideLHS = LHS.zext(2 * ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  ਗਿਆਤ_ਬਿੱਟ WideRHS = RHS.zext(2 * ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  ਵਾਪਸ mul(WideLHS, WideRHS).ਕੱਢੋ_ਬਿੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
}

ਸਥਿਰ ਗਿਆਤ_ਬਿੱਟ div_Compute_ਨੀਵਾਂ_ਦੋਆਧਾਰੀ_ਅੰਕ(ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS,
                                  ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS, ਤਾਰਕਿਕ Exact) {

  ਜੇ (!Exact)
    ਵਾਪਸ ਗਿਆਤ;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  ਜੇ (LHS.One[0])
    ਗਿਆਤ.One.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(0);

  ਪੂਰਨ MinTZ =
      (ਪੂਰਨ)LHS.ਗਿਣਤੀ_Min_Trailing_Zeros() - (ਪੂਰਨ)RHS.ਗਿਣਤੀ_Max_Trailing_Zeros();
  ਪੂਰਨ MaxTZ =
      (ਪੂਰਨ)LHS.ਗਿਣਤੀ_Max_Trailing_Zeros() - (ਪੂਰਨ)RHS.ਗਿਣਤੀ_Min_Trailing_Zeros();
  ਜੇ (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    ਗਿਆਤ.Zero.ਸੈੱਟ_ਨੀਵਾਂ_ਬਿੱਟ(MinTZ);
    ਜੇ (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      ਗਿਆਤ.One.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(MinTZ);
    }
  } ਨਹੀਂਤਾਂ ਜੇ (MaxTZ < 0) {
    // Poison Result
    ਗਿਆਤ.ਸੈੱਟ_All_Zero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  ਜੇ (ਗਿਆਤ.hasConflict())
    ਗਿਆਤ.ਸੈੱਟ_All_Zero();

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::sdiv(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS,
                          ਤਾਰਕਿਕ Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  ਜੇ (LHS.isNonNegative() && RHS.isNonNegative())
    ਵਾਪਸ udiv(LHS, RHS, Exact);

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);

  ਜੇ (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    ਗਿਆਤ.ਸੈੱਟ_All_Zero();
    ਵਾਪਸ ਗਿਆਤ;
  }

  std::optional<APInt> Res;
  ਜੇ (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_ਮੁੱਲ();
    APInt ਅੰਕ = LHS.get_Signed_Min_ਮੁੱਲ();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (ਅੰਕ.is_Min_Signed_ਮੁੱਲ() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_ਮੁੱਲ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)
              : ਅੰਕ.sdiv(Denom);
  } ਨਹੀਂਤਾਂ ਜੇ (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    ਜੇ (Exact || (-LHS.get_Signed_Max_ਮੁੱਲ()).uge(RHS.get_Signed_Max_ਮੁੱਲ())) {
      APInt Denom = RHS.get_Signed_Min_ਮੁੱਲ();
      APInt ਅੰਕ = LHS.get_Signed_Min_ਮੁੱਲ();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      ਜੇ (Denom.isZero())
        Denom.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(RHS.ਗਿਣਤੀ_Min_Trailing_Zeros());
      Res = ਅੰਕ.sdiv(Denom);
    }
  } ਨਹੀਂਤਾਂ ਜੇ (LHS.is_Strictly_ਸਕਾਰਾਤਮਕ() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    ਜੇ (Exact || LHS.get_Signed_Min_ਮੁੱਲ().uge(-RHS.get_Signed_Min_ਮੁੱਲ())) {
      APInt Denom = RHS.get_Signed_Max_ਮੁੱਲ();
      APInt ਅੰਕ = LHS.get_Signed_Max_ਮੁੱਲ();
      Res = ਅੰਕ.sdiv(Denom);
    }
  }

  ਜੇ (Res) {
    ਜੇ (Res->isNonNegative()) {
      ਬਿਨਾਨਿਸ਼ਾਨੀ LeadZ = Res->ਗਿਣਤੀ_Leading_Zeros();
      ਗਿਆਤ.Zero.ਸੈੱਟ_ਉੱਚਾ_ਬਿੱਟ(LeadZ);
    } ਨਹੀਂਤਾਂ {
      ਬਿਨਾਨਿਸ਼ਾਨੀ LeadO = Res->ਗਿਣਤੀ_Leading_Ones();
      ਗਿਆਤ.One.ਸੈੱਟ_ਉੱਚਾ_ਬਿੱਟ(LeadO);
    }
  }

  ਗਿਆਤ = div_Compute_ਨੀਵਾਂ_ਦੋਆਧਾਰੀ_ਅੰਕ(ਗਿਆਤ, LHS, RHS, Exact);
  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::udiv(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS,
                          ਤਾਰਕਿਕ Exact) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);

  ਜੇ (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    ਗਿਆਤ.ਸੈੱਟ_All_Zero();
    ਵਾਪਸ ਗਿਆਤ;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_ਮੁੱਲ();
  ਜੇ (MinDenom.isZero())
    MinDenom.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(RHS.ਗਿਣਤੀ_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_ਮੁੱਲ().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_ਮੁੱਲ().udiv(RHS.get_Max_ਮੁੱਲ());

  ਗਿਆਤ = ਗਿਆਤ_ਬਿੱਟ::makeConstant(MinRes);
  ਜੇ (std::optional<ਬਿਨਾਨਿਸ਼ਾਨੀ> Different_ਦੋਆਧਾਰੀ_ਅੰਕ =
          APIntOps::Get_Most_Significant_Different_ਦੋਆਧਾਰੀ_ਅੰਕ(MinRes, MaxRes)) {
    ਗਿਆਤ.Zero.clear_ਨੀਵਾਂ_ਬਿੱਟ(*Different_ਦੋਆਧਾਰੀ_ਅੰਕ + 1);
    ਗਿਆਤ.One.clear_ਨੀਵਾਂ_ਬਿੱਟ(*Different_ਦੋਆਧਾਰੀ_ਅੰਕ + 1);
  }

  ਗਿਆਤ = div_Compute_ਨੀਵਾਂ_ਦੋਆਧਾਰੀ_ਅੰਕ(ਗਿਆਤ, LHS, RHS, Exact);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::rem_Get_ਨੀਵਾਂ_ਬਿੱਟ(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = LHS.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਜੇ (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    ਬਿਨਾਨਿਸ਼ਾਨੀ RHSZeros = RHS.ਗਿਣਤੀ_Min_Trailing_Zeros();
    APInt ਨਕਾਬ = APInt::get_ਨੀਵਾਂ_ਬਿੱਟ_ਸੈੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, RHSZeros);
    APInt Ones_ਨਕਾਬ = LHS.One & ਨਕਾਬ;
    APInt Zeros_ਨਕਾਬ = LHS.Zero & ਨਕਾਬ;
    ਵਾਪਸ ਗਿਆਤ_ਬਿੱਟ(Zeros_ਨਕਾਬ, Ones_ਨਕਾਬ);
  }
  ਵਾਪਸ ਗਿਆਤ_ਬਿੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::urem(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  ਜੇ (LHS.get_Max_ਮੁੱਲ().ult(RHS.get_Min_ਮੁੱਲ()))
    ਵਾਪਸ LHS;

  ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ = rem_Get_ਨੀਵਾਂ_ਬਿੱਟ(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_ਮੁੱਲ(), RHS.get_Max_ਮੁੱਲ() - 1);
  ਗਿਆਤ.Zero.ਸੈੱਟ_ਉੱਚਾ_ਬਿੱਟ(MaxRes.ਗਿਣਤੀ_Leading_Zeros());
  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::srem(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &LHS, ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ = rem_Get_ਨੀਵਾਂ_ਬਿੱਟ(LHS, RHS);
  ਜੇ (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt ਨੀਵਾਂ_ਬਿੱਟ = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    ਜੇ (LHS.isNonNegative() || ਨੀਵਾਂ_ਬਿੱਟ.is_Subset_ਦਾ(LHS.Zero))
      ਗਿਆਤ.Zero |= ~ਨੀਵਾਂ_ਬਿੱਟ;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    ਜੇ (LHS.isNegative() && ਨੀਵਾਂ_ਬਿੱਟ.intersects(LHS.One))
      ਗਿਆਤ.One |= ~ਨੀਵਾਂ_ਬਿੱਟ;
    ਵਾਪਸ ਗਿਆਤ;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  ਜੇ (LHS.isNegative() && ਗਿਆਤ.isNonZero())
    ਗਿਆਤ.One.ਸੈੱਟ_ਉੱਚਾ_ਬਿੱਟ(
        std::max(LHS.ਗਿਣਤੀ_Min_Leading_Ones(), RHS.ਗਿਣਤੀ_Min_Sign_ਬਿੱਟ()));
  ਨਹੀਂਤਾਂ ਜੇ (LHS.isNonNegative())
    ਗਿਆਤ.Zero.ਸੈੱਟ_ਉੱਚਾ_ਬਿੱਟ(
        std::max(LHS.ਗਿਣਤੀ_Min_Leading_Zeros(), RHS.ਗਿਣਤੀ_Min_Sign_ਬਿੱਟ()));
  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_ਬਿੱਟ &ਗਿਆਤ_ਬਿੱਟ::ਸੰਚਾਲਕ&=(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  ਵਾਪਸ *ਇਹ;
}

ਗਿਆਤ_ਬਿੱਟ &ਗਿਆਤ_ਬਿੱਟ::ਸੰਚਾਲਕ|=(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  ਵਾਪਸ *ਇਹ;
}

ਗਿਆਤ_ਬਿੱਟ &ਗਿਆਤ_ਬਿੱਟ::ਸੰਚਾਲਕ^=(ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  ਵਾਪਸ *ਇਹ;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::blsi() ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ(Zero, APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, 0));
  ਬਿਨਾਨਿਸ਼ਾਨੀ Max = ਗਿਣਤੀ_Max_Trailing_Zeros();
  ਗਿਆਤ.Zero.ਸੈੱਟ_ਬਿੱਟ_From(std::min(Max + 1, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2));
  ਬਿਨਾਨਿਸ਼ਾਨੀ Min = ਗਿਣਤੀ_Min_Trailing_Zeros();
  ਜੇ (Max == Min && Max < ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)
    ਗਿਆਤ.One.ਸੈੱਟ_ਦੋਆਧਾਰੀ_ਅੰਕ(Max);
  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_ਬਿੱਟ::blsmsk() ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);
  ਬਿਨਾਨਿਸ਼ਾਨੀ Max = ਗਿਣਤੀ_Max_Trailing_Zeros();
  ਗਿਆਤ.Zero.ਸੈੱਟ_ਬਿੱਟ_From(std::min(Max + 1, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2));
  ਬਿਨਾਨਿਸ਼ਾਨੀ Min = ਗਿਣਤੀ_Min_Trailing_Zeros();
  ਗਿਆਤ.One.ਸੈੱਟ_ਨੀਵਾਂ_ਬਿੱਟ(std::min(Min + 1, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2));
  ਵਾਪਸ ਗਿਆਤ;
}

ਖਾਲੀ ਗਿਆਤ_ਬਿੱਟ::print(raw_ostream &OS) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ();
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ I = 0; I < ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2; ++I) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ N = ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - I - 1;
    ਜੇ (Zero[N] && One[N])
      OS << "!";
    ਨਹੀਂਤਾਂ ਜੇ (Zero[N])
      OS << "0";
    ਨਹੀਂਤਾਂ ਜੇ (One[N])
      OS << "1";
    ਨਹੀਂਤਾਂ
      OS << "?";
  }
}

#ਜੇ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ ਗਿਆਤ_ਬਿੱਟ::dump() ਅਚਲ {
  print(dbgs());
  dbgs() << "\n";
}
#ਜੇ_ਅੰਤ

ਬਿਨਾਨਿਸ਼ਾਨੀ llvm::Sign_ਬਿੱਟ_Ops::rot(ਬਿਨਾਨਿਸ਼ਾਨੀ Src_Sign_ਬਿੱਟ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2,
                                std::optional<APInt> RotAmt,
                                ਤਾਰਕਿਕ Is_Rotate_ਸੱਜਾ) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  ਜੇ (Src_Sign_ਬਿੱਟ == ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)
    ਵਾਪਸ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2;

  ਜੇ (!RotAmt)
    ਵਾਪਸ 1;

  ਬਿਨਾਨਿਸ਼ਾਨੀ Amt = RotAmt->urem(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  ਜੇ (Is_Rotate_ਸੱਜਾ)
    Amt = (ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - Amt) % ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  ਜੇ (Src_Sign_ਬਿੱਟ > Amt + 1)
    ਵਾਪਸ Src_Sign_ਬਿੱਟ - Amt;
  ਵਾਪਸ 1;
}
