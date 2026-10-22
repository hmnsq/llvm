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

#ಅನ್ತರ್ಭಾವ "llvm/Support/KnownBits.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Sequence.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <cassert>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::flip_Sign_ಬಿಟ್(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &Val) {
  ಚಿಹ್ನೆರಹಿತ Sign_ಬಿಟ್_Position = Val.get_ಬಿಟ್_ಅಗಲ() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.ಗಣ_ಬಿಟ್_Val(Sign_ಬಿಟ್_Position, Val.One[Sign_ಬಿಟ್_Position]);
  One.ಗಣ_ಬಿಟ್_Val(Sign_ಬಿಟ್_Position, Val.Zero[Sign_ಬಿಟ್_Position]);
  ಹಿಂತಿರುಗಿ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು(Zero, One);
}

ಸ್ಥಿರ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು compute_For_ಕೂಡಿಸು_ವಾಹಕ(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS,
                                    ತಾರ್ಕಿಕ ವಾಹಕ_Zero, ತಾರ್ಕಿಕ ವಾಹಕ_One) {

  APInt Possible_ಮೊತ್ತ_Zero = LHS.get_Max_ಮೌಲ್ಯ() + RHS.get_Max_ಮೌಲ್ಯ() + !ವಾಹಕ_Zero;
  APInt Possible_ಮೊತ್ತ_One = LHS.get_Min_ಮೌಲ್ಯ() + RHS.get_Min_ಮೌಲ್ಯ() + ವಾಹಕ_One;

  // Compute known bits of the carry.
  APInt ವಾಹಕ_ತಿಳಿದ_Zero = ~(Possible_ಮೊತ್ತ_Zero ^ LHS.Zero ^ RHS.Zero);
  APInt ವಾಹಕ_ತಿಳಿದ_One = Possible_ಮೊತ್ತ_One ^ LHS.One ^ RHS.One;

  // Compute set of known bits (where all three relevant bits are known).
  APInt LHSKnownUnion = LHS.Zero | LHS.One;
  APInt RHSKnownUnion = RHS.Zero | RHS.One;
  APInt ವಾಹಕ_ತಿಳಿದ_Union = std::move(ವಾಹಕ_ತಿಳಿದ_Zero) | ವಾಹಕ_ತಿಳಿದ_One;
  APInt ತಿಳಿದ = std::move(LHSKnownUnion) & RHSKnownUnion & ವಾಹಕ_ತಿಳಿದ_Union;

  // Compute known bits of the result.
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಹೊರಹೋಗು;
  ತಿಳಿದ_ಹೊರಹೋಗು.Zero = ~std::move(Possible_ಮೊತ್ತ_Zero) & ತಿಳಿದ;
  ತಿಳಿದ_ಹೊರಹೋಗು.One = std::move(Possible_ಮೊತ್ತ_One) & ತಿಳಿದ;
  ಹಿಂತಿರುಗಿ ತಿಳಿದ_ಹೊರಹೋಗು;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::compute_For_ಕೂಡಿಸು_ವಾಹಕ(
    ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &ವಾಹಕ_2) {
  ಪ್ರತಿಜ್ಞಾ(ವಾಹಕ_2.get_ಬಿಟ್_ಅಗಲ() == 1 && "Carry must be 1-bit");
  ಹಿಂತಿರುಗಿ ::compute_For_ಕೂಡಿಸು_ವಾಹಕ(
      LHS, RHS, ವಾಹಕ_2.Zero.get_Bool_ಮೌಲ್ಯ(), ವಾಹಕ_2.One.get_Bool_ಮೌಲ್ಯ());
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::compute_For_ಕೂಡಿಸು_Sub(ತಾರ್ಕಿಕ ಕೂಡಿಸು_2, ತಾರ್ಕಿಕ NSW, ತಾರ್ಕಿಕ NUW,
                                      ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS,
                                      ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = LHS.get_ಬಿಟ್_ಅಗಲ();
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಹೊರಹೋಗು(ಬಿಟ್_ಅಗಲ_2);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  ಆದರೆ (LHS.isUnknown() && RHS.isUnknown())
    ಹಿಂತಿರುಗಿ ತಿಳಿದ_ಹೊರಹೋಗು;

  ಆದರೆ (!LHS.isUnknown() && !RHS.isUnknown()) {
    ಆದರೆ (ಕೂಡಿಸು_2) {
      // Sum = LHS + RHS + 0
      ತಿಳಿದ_ಹೊರಹೋಗು = ::compute_For_ಕೂಡಿಸು_ವಾಹಕ(LHS, RHS, /*CarryZero=*/ನಿಜ,
                                      /*CarryOne=*/ಸುಳ್ಳು);
    } ಇಲ್ಲದಿದ್ದರೆ {
      // Sum = LHS + ~RHS + 1
      ತಿಳಿದ_ಬಿಟ್ಟುಗಳು NotRHS = RHS;
      std::ಅದಲುಬದಲು(NotRHS.Zero, NotRHS.One);
      ತಿಳಿದ_ಹೊರಹೋಗು = ::compute_For_ಕೂಡಿಸು_ವಾಹಕ(LHS, NotRHS, /*CarryZero=*/ಸುಳ್ಳು,
                                      /*CarryOne=*/ನಿಜ);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  ಆದರೆ (NUW) {
    ಆದರೆ (ಕೂಡಿಸು_2) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_ಮೌಲ್ಯ().uadd_sat(RHS.get_Min_ಮೌಲ್ಯ());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      ಆದರೆ (NSW) {
        ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2 = MinVal.trunc(ಬಿಟ್_ಅಗಲ_2 - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        ತಿಳಿದ_ಹೊರಹೋಗು.One.ಗಣ_ಬಿಟ್ಟುಗಳು(ಬಿಟ್_ಅಗಲ_2 - 1 - ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2, ಬಿಟ್_ಅಗಲ_2 - 1);
      }
      ತಿಳಿದ_ಹೊರಹೋಗು.One.ಗಣ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು(MinVal.countl_one());
    } ಇಲ್ಲದಿದ್ದರೆ {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_ಮೌಲ್ಯ().usub_sat(RHS.get_Min_ಮೌಲ್ಯ());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      ಆದರೆ (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2 = MaxVal.trunc(ಬಿಟ್_ಅಗಲ_2 - 1).countl_zero();
        ತಿಳಿದ_ಹೊರಹೋಗು.Zero.ಗಣ_ಬಿಟ್ಟುಗಳು(ಬಿಟ್_ಅಗಲ_2 - 1 - ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2, ಬಿಟ್_ಅಗಲ_2 - 1);
      }
      ತಿಳಿದ_ಹೊರಹೋಗು.Zero.ಗಣ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು(MaxVal.countl_zero());
    }
  }

  ಆದರೆ (NSW) {
    APInt MinVal;
    APInt MaxVal;
    ಆದರೆ (ಕೂಡಿಸು_2) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_ಮೌಲ್ಯ().sadd_sat(RHS.get_Signed_Min_ಮೌಲ್ಯ());
      MaxVal = LHS.get_Signed_Max_ಮೌಲ್ಯ().sadd_sat(RHS.get_Signed_Max_ಮೌಲ್ಯ());
    } ಇಲ್ಲದಿದ್ದರೆ {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_ಮೌಲ್ಯ().ssub_sat(RHS.get_Signed_Max_ಮೌಲ್ಯ());
      MaxVal = LHS.get_Signed_Max_ಮೌಲ್ಯ().ssub_sat(RHS.get_Signed_Min_ಮೌಲ್ಯ());
    }
    ಆದರೆ (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2 = MinVal.trunc(ಬಿಟ್_ಅಗಲ_2 - 1).countl_one();
      ತಿಳಿದ_ಹೊರಹೋಗು.One.ಗಣ_ಬಿಟ್ಟುಗಳು(ಬಿಟ್_ಅಗಲ_2 - 1 - ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2, ಬಿಟ್_ಅಗಲ_2 - 1);
      ತಿಳಿದ_ಹೊರಹೋಗು.Zero.ಗಣ_Sign_ಬಿಟ್();
    }
    ಆದರೆ (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2 = MaxVal.trunc(ಬಿಟ್_ಅಗಲ_2 - 1).countl_zero();
      ತಿಳಿದ_ಹೊರಹೋಗು.Zero.ಗಣ_ಬಿಟ್ಟುಗಳು(ಬಿಟ್_ಅಗಲ_2 - 1 - ಅಂಕೆ_ಬಿಟ್ಟುಗಳು_2, ಬಿಟ್_ಅಗಲ_2 - 1);
      ತಿಳಿದ_ಹೊರಹೋಗು.One.ಗಣ_Sign_ಬಿಟ್();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  ಆದರೆ (ತಿಳಿದ_ಹೊರಹೋಗು.hasConflict())
    ತಿಳಿದ_ಹೊರಹೋಗು.ಗಣ_All_Zero();
  ಹಿಂತಿರುಗಿ ತಿಳಿದ_ಹೊರಹೋಗು;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::computeForSubBorrow(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ತಿಳಿದ_ಬಿಟ್ಟುಗಳು RHS,
                                         ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &Borrow) {
  ಪ್ರತಿಜ್ಞಾ(Borrow.get_ಬಿಟ್_ಅಗಲ() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::ಅದಲುಬದಲು(RHS.Zero, RHS.One);
  ಹಿಂತಿರುಗಿ ::compute_For_ಕೂಡಿಸು_ವಾಹಕ(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_ಮೌಲ್ಯ(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_ಮೌಲ್ಯ());
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::truncSSat(ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಒಳಹರಿವು_ಬಿಟ್ಟುಗಳು = get_ಬಿಟ್_ಅಗಲ();
  APInt Min_ಒಳ_Range = APInt::get_Signed_Min_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2).sext(ಒಳಹರಿವು_ಬಿಟ್ಟುಗಳು);
  APInt Max_ಒಳ_Range = APInt::get_Signed_Max_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2).sext(ಒಳಹರಿವು_ಬಿಟ್ಟುಗಳು);
  APInt ಒಳಹರಿವು_Min = get_Signed_Min_ಮೌಲ್ಯ();
  APInt ಒಳಹರಿವು_Max = get_Signed_Max_ಮೌಲ್ಯ();
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ(ಬಿಟ್_ಅಗಲ_2);

  // Case 1: All values fit - just truncate
  ಆದರೆ (ಒಳಹರಿವು_Min.sge(Min_ಒಳ_Range) && ಒಳಹರಿವು_Max.sle(Max_ಒಳ_Range)) {
    ತಿಳಿದ = trunc(ಬಿಟ್_ಅಗಲ_2);
  }
  // Case 2: All saturate to min
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಒಳಹರಿವು_Max.slt(Min_ಒಳ_Range)) {
    ತಿಳಿದ = ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::makeConstant(APInt::get_Signed_Min_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2));
  }
  // Case 3: All saturate to max
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಒಳಹರಿವು_Min.sgt(Max_ಒಳ_Range)) {
    ತಿಳಿದ = ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::makeConstant(APInt::get_Signed_Max_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಒಳಹರಿವು_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    ತಿಳಿದ.Zero = APInt(ಬಿಟ್_ಅಗಲ_2, 0);
    ತಿಳಿದ.Zero.ಗಣ_ಬಿಟ್(ಬಿಟ್_ಅಗಲ_2 - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    ತಿಳಿದ.One = One.trunc(ಬಿಟ್_ಅಗಲ_2);
    ತಿಳಿದ.One.clear_ಬಿಟ್(ಬಿಟ್_ಅಗಲ_2 - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಒಳಹರಿವು_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    ತಿಳಿದ.Zero = Zero.trunc(ಬಿಟ್_ಅಗಲ_2);
    ತಿಳಿದ.Zero.clear_ಬಿಟ್(ಬಿಟ್_ಅಗಲ_2 - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    ತಿಳಿದ.One = APInt(ಬಿಟ್_ಅಗಲ_2, 0);
    ತಿಳಿದ.One.ಗಣ_ಬಿಟ್(ಬಿಟ್_ಅಗಲ_2 - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  ಇಲ್ಲದಿದ್ದರೆ {
    // Output: min saturation, truncated, or max saturation
    APInt ಒಳಹರಿವು_Zero_ಅಧೋ = Zero.trunc(ಬಿಟ್_ಅಗಲ_2);
    APInt ಒಳಹರಿವು_One_ಅಧೋ = One.trunc(ಬಿಟ್_ಅಗಲ_2);
    APInt MinSat = APInt::get_Signed_Min_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
    APInt MaxSat = APInt::get_Signed_Max_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
    ತಿಳಿದ_ಬಿಟ್ಟುಗಳು MinSatKB = ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::makeConstant(MinSat);
    ತಿಳಿದ_ಬಿಟ್ಟುಗಳು MaxSatKB = ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::makeConstant(MaxSat);
    ಆದರೆ (ಒಳಹರಿವು_Max.sle(Max_ಒಳ_Range)) {
      // Positive values fit, only negatives saturate to min
      ತಿಳಿದ.Zero = ಒಳಹರಿವು_Zero_ಅಧೋ & MinSatKB.Zero;
      ತಿಳಿದ.One = ಒಳಹರಿವು_One_ಅಧೋ & MinSatKB.One;
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಒಳಹರಿವು_Min.sge(Min_ಒಳ_Range)) {
      // Negative values fit, only positives saturate to max
      ತಿಳಿದ.Zero = ಒಳಹರಿವು_Zero_ಅಧೋ & MaxSatKB.Zero;
      ತಿಳಿದ.One = ಒಳಹರಿವು_One_ಅಧೋ & MaxSatKB.One;
    } ಇಲ್ಲದಿದ್ದರೆ {
      // Both positive and negative values might saturate
      ತಿಳಿದ.Zero = ಒಳಹರಿವು_Zero_ಅಧೋ & MinSatKB.Zero & MaxSatKB.Zero;
      ತಿಳಿದ.One = ಒಳಹರಿವು_One_ಅಧೋ & MinSatKB.One & MaxSatKB.One;
    }
  }
  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::truncSSatU(ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಒಳಹರಿವು_ಬಿಟ್ಟುಗಳು = get_ಬಿಟ್_ಅಗಲ();
  APInt Max_ಒಳ_Range = APInt::getAllOnes(ಬಿಟ್_ಅಗಲ_2).zext(ಒಳಹರಿವು_ಬಿಟ್ಟುಗಳು);
  APInt ಒಳಹರಿವು_Min = get_Signed_Min_ಮೌಲ್ಯ();
  APInt ಒಳಹರಿವು_Max = get_Signed_Max_ಮೌಲ್ಯ();
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ(ಬಿಟ್_ಅಗಲ_2);

  ಆದರೆ (isNegative()) {
    ತಿಳಿದ.ಗಣ_All_Zero();
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಒಳಹರಿವು_Min.isNonNegative() && ಒಳಹರಿವು_Max.ule(Max_ಒಳ_Range)) {
    ತಿಳಿದ = trunc(ಬಿಟ್_ಅಗಲ_2);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಒಳಹರಿವು_Min.isNonNegative() && ಒಳಹರಿವು_Min.ugt(Max_ಒಳ_Range)) {
    ತಿಳಿದ.ಗಣ_All_Ones();
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಒಳಹರಿವು_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    ತಿಳಿದ.Zero = APInt(ಬಿಟ್_ಅಗಲ_2, 0);
    // Common one bits: bits that are 1 in input stay 1
    ತಿಳಿದ.One = One.trunc(ಬಿಟ್_ಅಗಲ_2);
  } ಇಲ್ಲದಿದ್ದರೆ {
    // Mixed: sign bit unknown
    ಆದರೆ (ಒಳಹರಿವು_Max.ule(Max_ಒಳ_Range)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      ತಿಳಿದ.Zero = Zero.trunc(ಬಿಟ್_ಅಗಲ_2);
      ತಿಳಿದ.One = APInt(ಬಿಟ್_ಅಗಲ_2, 0);
    } ಇಲ್ಲದಿದ್ದರೆ {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      ತಿಳಿದ.Zero = APInt(ಬಿಟ್_ಅಗಲ_2, 0);
      ತಿಳಿದ.One = APInt(ಬಿಟ್_ಅಗಲ_2, 0);
    }
  }
  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::truncUSat(ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಒಳಹರಿವು_ಬಿಟ್ಟುಗಳು = get_ಬಿಟ್_ಅಗಲ();
  APInt Max_ಒಳ_Range = APInt::get_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು_ಗಣ(ಒಳಹರಿವು_ಬಿಟ್ಟುಗಳು, ಬಿಟ್_ಅಗಲ_2);
  APInt ಒಳಹರಿವು_Max = get_Max_ಮೌಲ್ಯ();
  APInt ಒಳಹರಿವು_Min = get_Min_ಮೌಲ್ಯ();
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ(ಬಿಟ್_ಅಗಲ_2);

  ಆದರೆ (ಒಳಹರಿವು_Max.ule(Max_ಒಳ_Range)) {
    ತಿಳಿದ = trunc(ಬಿಟ್_ಅಗಲ_2);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಒಳಹರಿವು_Min.ugt(Max_ಒಳ_Range)) {
    ತಿಳಿದ.ಗಣ_All_Ones();
  } ಇಲ್ಲದಿದ್ದರೆ {
    ತಿಳಿದ.resetAll();
    ತಿಳಿದ.One = One.trunc(ಬಿಟ್_ಅಗಲ_2);
  }
  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::sext_ಒಳ_Reg(ಚಿಹ್ನೆರಹಿತ Src_ಬಿಟ್_ಅಗಲ) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = get_ಬಿಟ್_ಅಗಲ();
  ಪ್ರತಿಜ್ಞಾ(0 < Src_ಬಿಟ್_ಅಗಲ && Src_ಬಿಟ್_ಅಗಲ <= ಬಿಟ್_ಅಗಲ_2 &&
         "Illegal sext-in-register");

  ಆದರೆ (Src_ಬಿಟ್_ಅಗಲ == ಬಿಟ್_ಅಗಲ_2)
    ಹಿಂತಿರುಗಿ *ಇದು;

  ಚಿಹ್ನೆರಹಿತ Ext_ಬಿಟ್ಟುಗಳು = ಬಿಟ್_ಅಗಲ_2 - Src_ಬಿಟ್_ಅಗಲ;
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ಫಲಿತಾಂಶ;
  ಫಲಿತಾಂಶ.One = One << Ext_ಬಿಟ್ಟುಗಳು;
  ಫಲಿತಾಂಶ.Zero = Zero << Ext_ಬಿಟ್ಟುಗಳು;
  ಫಲಿತಾಂಶ.One.ashr_ಒಳ_ಇರಿಸು(Ext_ಬಿಟ್ಟುಗಳು);
  ಫಲಿತಾಂಶ.Zero.ashr_ಒಳ_ಇರಿಸು(Ext_ಬಿಟ್ಟುಗಳು);
  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::makeGE(ಸ್ಥಿರಾಂಕ APInt &Val) ಸ್ಥಿರಾಂಕ {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  ಚಿಹ್ನೆರಹಿತ N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು(get_ಬಿಟ್_ಅಗಲ() - N);
  ಹಿಂತಿರುಗಿ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು(Zero, One | MaskedVal);
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::umax(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  ಆದರೆ (LHS.get_Min_ಮೌಲ್ಯ().uge(RHS.get_Max_ಮೌಲ್ಯ()))
    ಹಿಂತಿರುಗಿ LHS;
  ಆದರೆ (RHS.get_Min_ಮೌಲ್ಯ().uge(LHS.get_Max_ಮೌಲ್ಯ()))
    ಹಿಂತಿರುಗಿ RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು L = LHS.makeGE(RHS.get_Min_ಮೌಲ್ಯ());
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು R = RHS.makeGE(LHS.get_Min_ಮೌಲ್ಯ());
  ಹಿಂತಿರುಗಿ L.intersectWith(R);
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::umin(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  ಸ್ವಯಂಚಾಲಿತ Flip = [](ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &Val) { ಹಿಂತಿರುಗಿ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು(Val.One, Val.Zero); };
  ಹಿಂತಿರುಗಿ Flip(umax(Flip(LHS), Flip(RHS)));
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::smax(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಹಿಂತಿರುಗಿ flip_Sign_ಬಿಟ್(umax(flip_Sign_ಬಿಟ್(LHS), flip_Sign_ಬಿಟ್(RHS)));
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::smin(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  ಸ್ವಯಂಚಾಲಿತ Flip = [](ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &Val) {
    ಚಿಹ್ನೆರಹಿತ Sign_ಬಿಟ್_Position = Val.get_ಬಿಟ್_ಅಗಲ() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.ಗಣ_ಬಿಟ್_Val(Sign_ಬಿಟ್_Position, Val.Zero[Sign_ಬಿಟ್_Position]);
    One.ಗಣ_ಬಿಟ್_Val(Sign_ಬಿಟ್_Position, Val.One[Sign_ಬಿಟ್_Position]);
    ಹಿಂತಿರುಗಿ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು(Zero, One);
  };
  ಹಿಂತಿರುಗಿ Flip(umax(Flip(LHS), Flip(RHS)));
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::abdu(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  ಆದರೆ (LHS.get_Min_ಮೌಲ್ಯ().uge(RHS.get_Max_ಮೌಲ್ಯ()))
    ಹಿಂತಿರುಗಿ compute_For_ಕೂಡಿಸು_Sub(/*Add=*/ಸುಳ್ಳು, /*NSW=*/ಸುಳ್ಳು, /*NUW=*/ಸುಳ್ಳು, LHS,
                            RHS);
  ಆದರೆ (RHS.get_Min_ಮೌಲ್ಯ().uge(LHS.get_Max_ಮೌಲ್ಯ()))
    ಹಿಂತಿರುಗಿ compute_For_ಕೂಡಿಸು_Sub(/*Add=*/ಸುಳ್ಳು, /*NSW=*/ಸುಳ್ಳು, /*NUW=*/ಸುಳ್ಳು, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು Diff0 =
      compute_For_ಕೂಡಿಸು_Sub(/*Add=*/ಸುಳ್ಳು, /*NSW=*/ಸುಳ್ಳು, /*NUW=*/ನಿಜ, LHS, RHS);
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು Diff1 =
      compute_For_ಕೂಡಿಸು_Sub(/*Add=*/ಸುಳ್ಳು, /*NSW=*/ಸುಳ್ಳು, /*NUW=*/ನಿಜ, RHS, LHS);
  ಹಿಂತಿರುಗಿ Diff0.intersectWith(Diff1);
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::abds(ತಿಳಿದ_ಬಿಟ್ಟುಗಳು LHS, ತಿಳಿದ_ಬಿಟ್ಟುಗಳು RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  ಆದರೆ (LHS.get_Signed_Min_ಮೌಲ್ಯ().sge(RHS.get_Signed_Max_ಮೌಲ್ಯ()))
    ಹಿಂತಿರುಗಿ compute_For_ಕೂಡಿಸು_Sub(/*Add=*/ಸುಳ್ಳು, /*NSW=*/ಸುಳ್ಳು, /*NUW=*/ಸುಳ್ಳು, LHS,
                            RHS);
  ಆದರೆ (RHS.get_Signed_Min_ಮೌಲ್ಯ().sge(LHS.get_Signed_Max_ಮೌಲ್ಯ()))
    ಹಿಂತಿರುಗಿ compute_For_ಕೂಡಿಸು_Sub(/*Add=*/ಸುಳ್ಳು, /*NSW=*/ಸುಳ್ಳು, /*NUW=*/ಸುಳ್ಳು, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  ಚಿಹ್ನೆರಹಿತ Sign_ಬಿಟ್_Position = LHS.get_ಬಿಟ್_ಅಗಲ() - 1;
  ಗಾಗಿ (ಸ್ವಯಂಚಾಲಿತ Arg : {&LHS, &RHS}) {
    ತಾರ್ಕಿಕ Tmp = Arg->Zero[Sign_ಬಿಟ್_Position];
    Arg->Zero.ಗಣ_ಬಿಟ್_Val(Sign_ಬಿಟ್_Position, Arg->One[Sign_ಬಿಟ್_Position]);
    Arg->One.ಗಣ_ಬಿಟ್_Val(Sign_ಬಿಟ್_Position, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು Diff0 =
      compute_For_ಕೂಡಿಸು_Sub(/*Add=*/ಸುಳ್ಳು, /*NSW=*/ಸುಳ್ಳು, /*NUW=*/ನಿಜ, LHS, RHS);
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು Diff1 =
      compute_For_ಕೂಡಿಸು_Sub(/*Add=*/ಸುಳ್ಳು, /*NSW=*/ಸುಳ್ಳು, /*NUW=*/ನಿಜ, RHS, LHS);
  ಹಿಂತಿರುಗಿ Diff0.intersectWith(Diff1);
}

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ get_Max_ಸರಿಸು_Amount(ಸ್ಥಿರಾಂಕ APInt &Max_ಮೌಲ್ಯ, ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2) {
  ಆದರೆ (isPowerOf2_32(ಬಿಟ್_ಅಗಲ_2))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    ಹಿಂತಿರುಗಿ Max_ಮೌಲ್ಯ.ಹೊರತೆಗೆ_ಬಿಟ್ಟುಗಳು_As_ZExt_ಮೌಲ್ಯ(
        std::min(Log2_32(ಬಿಟ್_ಅಗಲ_2), Max_ಮೌಲ್ಯ.get_ಬಿಟ್_ಅಗಲ()), 0);
  // This is only an approximate upper bound.
  ಹಿಂತಿರುಗಿ Max_ಮೌಲ್ಯ.get_Limited_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2 - 1);
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::shl(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS, ತಾರ್ಕಿಕ NUW,
                         ತಾರ್ಕಿಕ NSW, ತಾರ್ಕಿಕ ShAmtNonZero) {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = LHS.get_ಬಿಟ್_ಅಗಲ();
  ಸ್ವಯಂಚಾಲಿತ ಸರಿಸು_By_Const = [&](ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_2) {
    ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ;
    ತಾರ್ಕಿಕ Shifted_ಹೊರಹೋಗು_Zero, Shifted_ಹೊರಹೋಗು_One;
    ತಿಳಿದ.Zero = LHS.Zero.ushl_ov(ಸರಿಸು_Amt_2, Shifted_ಹೊರಹೋಗು_Zero);
    ತಿಳಿದ.Zero.ಗಣ_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು(ಸರಿಸು_Amt_2);
    ತಿಳಿದ.One = LHS.One.ushl_ov(ಸರಿಸು_Amt_2, Shifted_ಹೊರಹೋಗು_One);

    // All cases returning poison have been handled by MaxShiftAmount already.
    ಆದರೆ (NSW) {
      ಆದರೆ (NUW && ಸರಿಸು_Amt_2 != 0)
        // NUW means we can assume anything shifted out was a zero.
        Shifted_ಹೊರಹೋಗು_Zero = ನಿಜ;

      ಆದರೆ (Shifted_ಹೊರಹೋಗು_Zero)
        ತಿಳಿದ.makeNonNegative();
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Shifted_ಹೊರಹೋಗು_One)
        ತಿಳಿದ.makeNegative();
    }
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  };

  // Fast path for a common case when LHS is completely unknown.
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ(ಬಿಟ್_ಅಗಲ_2);
  ಚಿಹ್ನೆರಹಿತ Min_ಸರಿಸು_Amount = RHS.get_Min_ಮೌಲ್ಯ().get_Limited_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
  ಆದರೆ (Min_ಸರಿಸು_Amount == 0 && ShAmtNonZero)
    Min_ಸರಿಸು_Amount = 1;
  ಆದರೆ (LHS.isUnknown()) {
    ತಿಳಿದ.Zero.ಗಣ_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು(Min_ಸರಿಸು_Amount);
    ಆದರೆ (NUW && NSW && Min_ಸರಿಸು_Amount != 0)
      ತಿಳಿದ.makeNonNegative();
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_ಮೌಲ್ಯ = RHS.get_Max_ಮೌಲ್ಯ();
  ಚಿಹ್ನೆರಹಿತ Max_ಸರಿಸು_Amount = get_Max_ಸರಿಸು_Amount(Max_ಮೌಲ್ಯ, ಬಿಟ್_ಅಗಲ_2);
  ಆದರೆ (NUW && NSW)
    Max_ಸರಿಸು_Amount = std::min(Max_ಸರಿಸು_Amount, LHS.ಎಣಿಕೆ_Max_Leading_Zeros() - 1);
  ಆದರೆ (NUW)
    Max_ಸರಿಸು_Amount = std::min(Max_ಸರಿಸು_Amount, LHS.ಎಣಿಕೆ_Max_Leading_Zeros());
  ಆದರೆ (NSW)
    Max_ಸರಿಸು_Amount = std::min(
        Max_ಸರಿಸು_Amount,
        std::max(LHS.ಎಣಿಕೆ_Max_Leading_Zeros(), LHS.ಎಣಿಕೆ_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  ಆದರೆ (Min_ಸರಿಸು_Amount == 0 && Max_ಸರಿಸು_Amount == ಬಿಟ್_ಅಗಲ_2 - 1 &&
      isPowerOf2_32(ಬಿಟ್_ಅಗಲ_2)) {
    ತಿಳಿದ.Zero.ಗಣ_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು(LHS.ಎಣಿಕೆ_Min_Trailing_Zeros());
    ಆದರೆ (LHS.isAllOnes())
      ತಿಳಿದ.One.ಗಣ_Sign_ಬಿಟ್();
    ಆದರೆ (NSW) {
      ಆದರೆ (LHS.isNonNegative())
        ತಿಳಿದ.makeNonNegative();
      ಆದರೆ (LHS.isNegative())
        ತಿಳಿದ.makeNegative();
    }
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  }

  // Find the common bits from all possible shifts.
  ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_Zero_ಮುಖವಾಡ = RHS.Zero.zextOrTrunc(32).get_ZExt_ಮೌಲ್ಯ();
  ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_One_ಮುಖವಾಡ = RHS.One.zextOrTrunc(32).get_ZExt_ಮೌಲ್ಯ();
  ತಿಳಿದ.ಗಣ_All_Conflict();
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_2 = Min_ಸರಿಸು_Amount; ಸರಿಸು_Amt_2 <= Max_ಸರಿಸು_Amount;
       ++ಸರಿಸು_Amt_2) {
    // Skip if the shift amount is impossible.
    ಆದರೆ ((ಸರಿಸು_Amt_Zero_ಮುಖವಾಡ & ಸರಿಸು_Amt_2) != 0 ||
        (ಸರಿಸು_Amt_One_ಮುಖವಾಡ | ಸರಿಸು_Amt_2) != ಸರಿಸು_Amt_2)
      ಮುಂದುವರಿ;
    ತಿಳಿದ = ತಿಳಿದ.intersectWith(ಸರಿಸು_By_Const(LHS, ಸರಿಸು_Amt_2));
    ಆದರೆ (ತಿಳಿದ.isUnknown())
      ನಿಲ್ಲಿಸು;
  }

  // All shift amounts may result in poison.
  ಆದರೆ (ತಿಳಿದ.hasConflict())
    ತಿಳಿದ.ಗಣ_All_Zero();
  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::lshr(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS,
                          ತಾರ್ಕಿಕ ShAmtNonZero, ತಾರ್ಕಿಕ Exact) {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = LHS.get_ಬಿಟ್_ಅಗಲ();
  ಸ್ವಯಂಚಾಲಿತ ಸರಿಸು_By_Const = [&](ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_2) {
    ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ = LHS;
    ತಿಳಿದ >>= ಸರಿಸು_Amt_2;
    // High bits are known zero.
    ತಿಳಿದ.Zero.ಗಣ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು(ಸರಿಸು_Amt_2);
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  };

  // Fast path for a common case when LHS is completely unknown.
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ(ಬಿಟ್_ಅಗಲ_2);
  ಚಿಹ್ನೆರಹಿತ Min_ಸರಿಸು_Amount = RHS.get_Min_ಮೌಲ್ಯ().get_Limited_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
  ಆದರೆ (Min_ಸರಿಸು_Amount == 0 && ShAmtNonZero)
    Min_ಸರಿಸು_Amount = 1;
  ಆದರೆ (LHS.isUnknown()) {
    ತಿಳಿದ.Zero.ಗಣ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು(Min_ಸರಿಸು_Amount);
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  }

  // Find the common bits from all possible shifts.
  APInt Max_ಮೌಲ್ಯ = RHS.get_Max_ಮೌಲ್ಯ();
  ಚಿಹ್ನೆರಹಿತ Max_ಸರಿಸು_Amount = get_Max_ಸರಿಸು_Amount(Max_ಮೌಲ್ಯ, ಬಿಟ್_ಅಗಲ_2);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  ಆದರೆ (Exact) {
    ಚಿಹ್ನೆರಹಿತ ಮೊದಲ_One = LHS.ಎಣಿಕೆ_Max_Trailing_Zeros();
    ಆದರೆ (ಮೊದಲ_One < Min_ಸರಿಸು_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      ತಿಳಿದ.ಗಣ_All_Zero();
      ಹಿಂತಿರುಗಿ ತಿಳಿದ;
    }
    Max_ಸರಿಸು_Amount = std::min(Max_ಸರಿಸು_Amount, ಮೊದಲ_One);
  }

  ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_Zero_ಮುಖವಾಡ = RHS.Zero.zextOrTrunc(32).get_ZExt_ಮೌಲ್ಯ();
  ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_One_ಮುಖವಾಡ = RHS.One.zextOrTrunc(32).get_ZExt_ಮೌಲ್ಯ();
  ತಿಳಿದ.ಗಣ_All_Conflict();
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_2 = Min_ಸರಿಸು_Amount; ಸರಿಸು_Amt_2 <= Max_ಸರಿಸು_Amount;
       ++ಸರಿಸು_Amt_2) {
    // Skip if the shift amount is impossible.
    ಆದರೆ ((ಸರಿಸು_Amt_Zero_ಮುಖವಾಡ & ಸರಿಸು_Amt_2) != 0 ||
        (ಸರಿಸು_Amt_One_ಮುಖವಾಡ | ಸರಿಸು_Amt_2) != ಸರಿಸು_Amt_2)
      ಮುಂದುವರಿ;
    ತಿಳಿದ = ತಿಳಿದ.intersectWith(ಸರಿಸು_By_Const(LHS, ಸರಿಸು_Amt_2));
    ಆದರೆ (ತಿಳಿದ.isUnknown())
      ನಿಲ್ಲಿಸು;
  }

  // All shift amounts may result in poison.
  ಆದರೆ (ತಿಳಿದ.hasConflict())
    ತಿಳಿದ.ಗಣ_All_Zero();
  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::ashr(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS,
                          ತಾರ್ಕಿಕ ShAmtNonZero, ತಾರ್ಕಿಕ Exact) {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = LHS.get_ಬಿಟ್_ಅಗಲ();
  ಸ್ವಯಂಚಾಲಿತ ಸರಿಸು_By_Const = [&](ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_2) {
    ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ = LHS;
    ತಿಳಿದ.Zero.ashr_ಒಳ_ಇರಿಸು(ಸರಿಸು_Amt_2);
    ತಿಳಿದ.One.ashr_ಒಳ_ಇರಿಸು(ಸರಿಸು_Amt_2);
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  };

  // Fast path for a common case when LHS is completely unknown.
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ(ಬಿಟ್_ಅಗಲ_2);
  ಚಿಹ್ನೆರಹಿತ Min_ಸರಿಸು_Amount = RHS.get_Min_ಮೌಲ್ಯ().get_Limited_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
  ಆದರೆ (Min_ಸರಿಸು_Amount == 0 && ShAmtNonZero)
    Min_ಸರಿಸು_Amount = 1;
  ಆದರೆ (LHS.isUnknown()) {
    ಆದರೆ (Min_ಸರಿಸು_Amount == ಬಿಟ್_ಅಗಲ_2) {
      // Always poison. Return zero because we don't like returning conflict.
      ತಿಳಿದ.ಗಣ_All_Zero();
      ಹಿಂತಿರುಗಿ ತಿಳಿದ;
    }
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  }

  // Find the common bits from all possible shifts.
  APInt Max_ಮೌಲ್ಯ = RHS.get_Max_ಮೌಲ್ಯ();
  ಚಿಹ್ನೆರಹಿತ Max_ಸರಿಸು_Amount = get_Max_ಸರಿಸು_Amount(Max_ಮೌಲ್ಯ, ಬಿಟ್_ಅಗಲ_2);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  ಆದರೆ (Exact) {
    ಚಿಹ್ನೆರಹಿತ ಮೊದಲ_One = LHS.ಎಣಿಕೆ_Max_Trailing_Zeros();
    ಆದರೆ (ಮೊದಲ_One < Min_ಸರಿಸು_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      ತಿಳಿದ.ಗಣ_All_Zero();
      ಹಿಂತಿರುಗಿ ತಿಳಿದ;
    }
    Max_ಸರಿಸು_Amount = std::min(Max_ಸರಿಸು_Amount, ಮೊದಲ_One);
  }

  ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_Zero_ಮುಖವಾಡ = RHS.Zero.zextOrTrunc(32).get_ZExt_ಮೌಲ್ಯ();
  ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_One_ಮುಖವಾಡ = RHS.One.zextOrTrunc(32).get_ZExt_ಮೌಲ್ಯ();
  ತಿಳಿದ.ಗಣ_All_Conflict();
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಸರಿಸು_Amt_2 = Min_ಸರಿಸು_Amount; ಸರಿಸು_Amt_2 <= Max_ಸರಿಸು_Amount;
      ++ಸರಿಸು_Amt_2) {
    // Skip if the shift amount is impossible.
    ಆದರೆ ((ಸರಿಸು_Amt_Zero_ಮುಖವಾಡ & ಸರಿಸು_Amt_2) != 0 ||
        (ಸರಿಸು_Amt_One_ಮುಖವಾಡ | ಸರಿಸು_Amt_2) != ಸರಿಸು_Amt_2)
      ಮುಂದುವರಿ;
    ತಿಳಿದ = ತಿಳಿದ.intersectWith(ಸರಿಸು_By_Const(LHS, ಸರಿಸು_Amt_2));
    ಆದರೆ (ತಿಳಿದ.isUnknown())
      ನಿಲ್ಲಿಸು;
  }

  // All shift amounts may result in poison.
  ಆದರೆ (ತಿಳಿದ.hasConflict())
    ತಿಳಿದ.ಗಣ_All_Zero();
  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::fshl(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS,
                          ಸ್ಥಿರಾಂಕ APInt &Amt) {
  ಹಿಂತಿರುಗಿ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::fshr(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS,
                          ಸ್ಥಿರಾಂಕ APInt &Amt) {
  ಹಿಂತಿರುಗಿ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::clmul(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_ಮೌಲ್ಯ(), RHS.get_Min_ಮೌಲ್ಯ()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  ಸ್ವಯಂಚಾಲಿತ ClMulOr = [](ಸ್ಥಿರಾಂಕ APInt &LHS, ಸ್ಥಿರಾಂಕ APInt &RHS) {
    ಚಿಹ್ನೆರಹಿತ BW = LHS.get_ಬಿಟ್_ಅಗಲ();
    ಪ್ರತಿಜ್ಞಾ(BW == RHS.get_ಬಿಟ್_ಅಗಲ() && "Operand mismatch");
    APInt ಫಲಿತಾಂಶ(BW, 0);
    ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I :
         seq(std::min(RHS.get_ಸಕ್ರಿಯ_ಬಿಟ್ಟುಗಳು(), BW - LHS.countr_zero())))
      ಆದರೆ (RHS[I])
        ಫಲಿತಾಂಶ |= LHS << I;
    ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt ತಿಳಿದ = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= ತಿಳಿದ;
  Res.One &= ತಿಳಿದ;

  ಹಿಂತಿರುಗಿ Res;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::pext(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &Val, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &ಮುಖವಾಡ) {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = Val.get_ಬಿಟ್_ಅಗಲ();
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು Res(ಬಿಟ್_ಅಗಲ_2);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.ಗಣ_All_Conflict();

  // For each source position I where Mask[I] could be 1, the output position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible output positions widens.
  ಚಿಹ್ನೆರಹಿತ M0 = 0, M1 = 0;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I < ಬಿಟ್_ಅಗಲ_2; ++I) {
    ಆದರೆ (!ಮುಖವಾಡ.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      ಆದರೆ (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clear_ಬಿಟ್ಟುಗಳು(M0, M1 + 1);
      ಆದರೆ (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clear_ಬಿಟ್ಟುಗಳು(M0, M1 + 1);
    }
    ಆದರೆ (ಮುಖವಾಡ.One[I])
      ++M0, ++M1;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!ಮುಖವಾಡ.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clear_ಬಿಟ್ಟುಗಳು(M0, ಬಿಟ್_ಅಗಲ_2);
  ಹಿಂತಿರುಗಿ Res;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::pdep(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &Val, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &ಮುಖವಾಡ) {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = Val.get_ಬಿಟ್_ಅಗಲ();
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು Res(ಬಿಟ್_ಅಗಲ_2);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.ಗಣ_All_Conflict();

  // For each output position I where Mask[I] could be 1, the source position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible source positions widens.
  ಚಿಹ್ನೆರಹಿತ M0 = 0, M1 = 0;
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I < ಬಿಟ್_ಅಗಲ_2; ++I) {
    ಆದರೆ (!ಮುಖವಾಡ.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clear_ಬಿಟ್(I);
    ಆದರೆ (!ಮುಖವಾಡ.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::get_ಬಿಟ್ಟುಗಳು_ಗಣ(ಬಿಟ್_ಅಗಲ_2, M0, M1 + 1);
      ಆದರೆ (!Range.is_Subset_ದ(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clear_ಬಿಟ್(I);
      ಆದರೆ (!Range.is_Subset_ದ(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clear_ಬಿಟ್(I);
    }
    ಆದರೆ (ಮುಖವಾಡ.One[I])
      ++M0, ++M1;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!ಮುಖವಾಡ.Zero[I])
      ++M1;
  }
  ಹಿಂತಿರುಗಿ Res;
}

std::optional<ತಾರ್ಕಿಕ> ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::eq(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಆದರೆ (LHS.isConstant() && RHS.isConstant())
    ಹಿಂತಿರುಗಿ LHS.getConstant() == RHS.getConstant();
  ಆದರೆ (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಹಿಂತಿರುಗಿ std::nullopt;
}

std::optional<ತಾರ್ಕಿಕ> ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::ne(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಆದರೆ (std::optional<ತಾರ್ಕಿಕ> ತಿಳಿದ_EQ = eq(LHS, RHS))
    ಹಿಂತಿರುಗಿ !*ತಿಳಿದ_EQ;
  ಹಿಂತಿರುಗಿ std::nullopt;
}

std::optional<ತಾರ್ಕಿಕ> ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::ugt(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  ಆದರೆ (LHS.get_Max_ಮೌಲ್ಯ().ule(RHS.get_Min_ಮೌಲ್ಯ()))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  ಆದರೆ (LHS.get_Min_ಮೌಲ್ಯ().ugt(RHS.get_Max_ಮೌಲ್ಯ()))
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಹಿಂತಿರುಗಿ std::nullopt;
}

std::optional<ತಾರ್ಕಿಕ> ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::uge(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಆದರೆ (std::optional<ತಾರ್ಕಿಕ> IsUGT = ugt(RHS, LHS))
    ಹಿಂತಿರುಗಿ !*IsUGT;
  ಹಿಂತಿರುಗಿ std::nullopt;
}

std::optional<ತಾರ್ಕಿಕ> ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::ult(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಹಿಂತಿರುಗಿ ugt(RHS, LHS);
}

std::optional<ತಾರ್ಕಿಕ> ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::ule(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಹಿಂತಿರುಗಿ uge(RHS, LHS);
}

std::optional<ತಾರ್ಕಿಕ> ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::sgt(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  ಆದರೆ (LHS.get_Signed_Max_ಮೌಲ್ಯ().sle(RHS.get_Signed_Min_ಮೌಲ್ಯ()))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  ಆದರೆ (LHS.get_Signed_Min_ಮೌಲ್ಯ().sgt(RHS.get_Signed_Max_ಮೌಲ್ಯ()))
    ಹಿಂತಿರುಗಿ ನಿಜ;
  ಹಿಂತಿರುಗಿ std::nullopt;
}

std::optional<ತಾರ್ಕಿಕ> ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::sge(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಆದರೆ (std::optional<ತಾರ್ಕಿಕ> ತಿಳಿದ_SGT = sgt(RHS, LHS))
    ಹಿಂತಿರುಗಿ !*ತಿಳಿದ_SGT;
  ಹಿಂತಿರುಗಿ std::nullopt;
}

std::optional<ತಾರ್ಕಿಕ> ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::slt(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಹಿಂತಿರುಗಿ sgt(RHS, LHS);
}

std::optional<ತಾರ್ಕಿಕ> ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::sle(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಹಿಂತಿರುಗಿ sge(RHS, LHS);
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::ನಿರಪೇಕ್ಷ(ತಾರ್ಕಿಕ IntMinIsPoison) ಸ್ಥಿರಾಂಕ {
  // If the source's MSB is zero then we know the rest of the bits already.
  ಆದರೆ (isNonNegative())
    ಹಿಂತಿರುಗಿ *ಇದು;

  // Absolute value preserves trailing zero count.
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_Abs(get_ಬಿಟ್_ಅಗಲ());

  // If the input is negative, then abs(x) == -x.
  ಆದರೆ (isNegative()) {
    ತಿಳಿದ_ಬಿಟ್ಟುಗಳು Tmp = *ಇದು;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    ಆದರೆ (IntMinIsPoison && (Zero.popcount() + 2) == get_ಬಿಟ್_ಅಗಲ())
      Tmp.One.ಗಣ_ಬಿಟ್(ಎಣಿಕೆ_Min_Trailing_Zeros());

    ತಿಳಿದ_Abs = compute_For_ಕೂಡಿಸು_Sub(
        /*ಕೂಡಿಸು*/ ಸುಳ್ಳು, IntMinIsPoison, /*NUW=*/ಸುಳ್ಳು,
        ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::makeConstant(APInt(get_ಬಿಟ್_ಅಗಲ(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    ಆದರೆ (IntMinIsPoison && Tmp.ಎಣಿಕೆ_Min_Population() == 1 &&
        Tmp.ಎಣಿಕೆ_Max_Population() != 1) {
      Tmp.One.clear_Sign_ಬಿಟ್();
      Tmp.Zero.ಗಣ_Sign_ಬಿಟ್();
      ತಿಳಿದ_Abs.One.ಗಣ_ಬಿಟ್ಟುಗಳು(get_ಬಿಟ್_ಅಗಲ() - Tmp.ಎಣಿಕೆ_Min_Leading_Zeros(),
                           get_ಬಿಟ್_ಅಗಲ() - 1);
    }

  } ಇಲ್ಲದಿದ್ದರೆ {
    ಚಿಹ್ನೆರಹಿತ MaxTZ = ಎಣಿಕೆ_Max_Trailing_Zeros();
    ಚಿಹ್ನೆರಹಿತ MinTZ = ಎಣಿಕೆ_Min_Trailing_Zeros();

    ತಿಳಿದ_Abs.Zero.ಗಣ_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು(MinTZ);
    // If we know the lowest set 1, then preserve it.
    ಆದರೆ (MaxTZ == MinTZ && MaxTZ < get_ಬಿಟ್_ಅಗಲ())
      ತಿಳಿದ_Abs.One.ಗಣ_ಬಿಟ್(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    ಆದರೆ (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_ಮೌಲ್ಯ())) {
      ತಿಳಿದ_Abs.One.clear_Sign_ಬಿಟ್();
      ತಿಳಿದ_Abs.Zero.ಗಣ_Sign_ಬಿಟ್();
    }
  }

  ಹಿಂತಿರುಗಿ ತಿಳಿದ_Abs;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::reduce_ಕೂಡಿಸು(ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Elts) ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (ಅಂಕೆ_Elts == 0)
    ಹಿಂತಿರುಗಿ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು(get_ಬಿಟ್_ಅಗಲ());

  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = get_ಬಿಟ್_ಅಗಲ();
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ಫಲಿತಾಂಶ(ಬಿಟ್_ಅಗಲ_2);

  ಆದರೆ (isConstant())
    // If all elements are the same constant, we can simply compute it
    ಹಿಂತಿರುಗಿ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::makeConstant(ಅಂಕೆ_Elts * getConstant());

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
  ಚಿಹ್ನೆರಹಿತ ಕಳೆದುಹೋದ_ಬಿಟ್ಟುಗಳು = Log2_32_Ceil(ಅಂಕೆ_Elts);

  ಆದರೆ (isNonNegative()) {
    ಚಿಹ್ನೆರಹಿತ LeadingZeros = ಎಣಿಕೆ_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > ಕಳೆದುಹೋದ_ಬಿಟ್ಟುಗಳು ? LeadingZeros - ಕಳೆದುಹೋದ_ಬಿಟ್ಟುಗಳು : 0;
    ಫಲಿತಾಂಶ.Zero.ಗಣ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು(LeadingZeros);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (isNegative()) {
    ಚಿಹ್ನೆರಹಿತ LeadingOnes = ಎಣಿಕೆ_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > ಕಳೆದುಹೋದ_ಬಿಟ್ಟುಗಳು ? LeadingOnes - ಕಳೆದುಹೋದ_ಬಿಟ್ಟುಗಳು : 0;
    ಫಲಿತಾಂಶ.One.ಗಣ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು(LeadingOnes);
  }

  ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ಸ್ಥಿರ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು compute_For_Sat_ಕೂಡಿಸು_Sub(ತಾರ್ಕಿಕ ಕೂಡಿಸು_2, ತಾರ್ಕಿಕ Signed,
                                     ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS,
                                     ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = LHS.get_ಬಿಟ್_ಅಗಲ();

  std::optional<ತಾರ್ಕಿಕ> ಅತಿಪ್ರವಾಹ;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  ತಾರ್ಕಿಕ MayNegClamp = ನಿಜ;
  ತಾರ್ಕಿಕ MayPosClamp = ನಿಜ;
  ಆದರೆ (Signed) {
    // Easy cases we can rule out any overflow.
    ಆದರೆ (ಕೂಡಿಸು_2 && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      ಅತಿಪ್ರವಾಹ = ಸುಳ್ಳು;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!ಕೂಡಿಸು_2 && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      ಅತಿಪ್ರವಾಹ = ಸುಳ್ಳು;
    ಇಲ್ಲದಿದ್ದರೆ {
      // Check if we may overflow. If we can't rule out overflow then check if
      // we can rule out a direction at least.
      ತಿಳಿದ_ಬಿಟ್ಟುಗಳು UnsignedLHS = LHS;
      ತಿಳಿದ_ಬಿಟ್ಟುಗಳು UnsignedRHS = RHS;
      // Get version of LHS/RHS with clearer signbit. This allows us to detect
      // how the addition/subtraction might overflow into the signbit. Then
      // using the actual known signbits of LHS/RHS, we can figure out which
      // overflows are/aren't possible.
      UnsignedLHS.One.clear_Sign_ಬಿಟ್();
      UnsignedLHS.Zero.ಗಣ_Sign_ಬಿಟ್();
      UnsignedRHS.One.clear_Sign_ಬಿಟ್();
      UnsignedRHS.Zero.ಗಣ_Sign_ಬಿಟ್();
      ತಿಳಿದ_ಬಿಟ್ಟುಗಳು Res =
          ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::compute_For_ಕೂಡಿಸು_Sub(ಕೂಡಿಸು_2, /*NSW=*/ಸುಳ್ಳು,
                                      /*NUW=*/ಸುಳ್ಳು, UnsignedLHS, UnsignedRHS);
      ಆದರೆ (ಕೂಡಿಸು_2) {
        ಆದರೆ (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = ಸುಳ್ಳು;
          // Pos + Pos will overflow with extra signbit.
          ಆದರೆ (LHS.isNonNegative() && RHS.isNonNegative())
            ಅತಿಪ್ರವಾಹ = ನಿಜ;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = ಸುಳ್ಳು;
          // Neg + Neg will overflow without extra signbit.
          ಆದರೆ (LHS.isNegative() && RHS.isNegative())
            ಅತಿಪ್ರವಾಹ = ನಿಜ;
        }
        // We will never clamp to the opposite sign of N-bit result.
        ಆದರೆ (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = ಸುಳ್ಳು;
        ಆದರೆ (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = ಸುಳ್ಳು;
      } ಇಲ್ಲದಿದ್ದರೆ {
        ಆದರೆ (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = ಸುಳ್ಳು;
          // Neg - Pos will overflow with extra signbit.
          ಆದರೆ (LHS.isNegative() && RHS.isNonNegative())
            ಅತಿಪ್ರವಾಹ = ನಿಜ;
        } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = ಸುಳ್ಳು;
          // Pos - Neg will overflow without extra signbit.
          ಆದರೆ (LHS.isNonNegative() && RHS.isNegative())
            ಅತಿಪ್ರವಾಹ = ನಿಜ;
        }
        // We will never clamp to the opposite sign of N-bit result.
        ಆದರೆ (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = ಸುಳ್ಳು;
        ಆದರೆ (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = ಸುಳ್ಳು;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    ಆದರೆ (!MayNegClamp && !MayPosClamp)
      ಅತಿಪ್ರವಾಹ = ಸುಳ್ಳು;
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೂಡಿಸು_2) {
    // uadd.sat
    ತಾರ್ಕಿಕ ದ;
    (ಶೂನ್ಯ)LHS.get_Max_ಮೌಲ್ಯ().uadd_ov(RHS.get_Max_ಮೌಲ್ಯ(), ದ);
    ಆದರೆ (!ದ) {
      ಅತಿಪ್ರವಾಹ = ಸುಳ್ಳು;
    } ಇಲ್ಲದಿದ್ದರೆ {
      (ಶೂನ್ಯ)LHS.get_Min_ಮೌಲ್ಯ().uadd_ov(RHS.get_Min_ಮೌಲ್ಯ(), ದ);
      ಆದರೆ (ದ)
        ಅತಿಪ್ರವಾಹ = ನಿಜ;
    }
  } ಇಲ್ಲದಿದ್ದರೆ {
    // usub.sat
    ತಾರ್ಕಿಕ ದ;
    (ಶೂನ್ಯ)LHS.get_Min_ಮೌಲ್ಯ().usub_ov(RHS.get_Max_ಮೌಲ್ಯ(), ದ);
    ಆದರೆ (!ದ) {
      ಅತಿಪ್ರವಾಹ = ಸುಳ್ಳು;
    } ಇಲ್ಲದಿದ್ದರೆ {
      (ಶೂನ್ಯ)LHS.get_Max_ಮೌಲ್ಯ().usub_ov(RHS.get_Min_ಮೌಲ್ಯ(), ದ);
      ಆದರೆ (ದ)
        ಅತಿಪ್ರವಾಹ = ನಿಜ;
    }
  }

  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು Res = ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::compute_For_ಕೂಡಿಸು_Sub(ಕೂಡಿಸು_2, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  ಆದರೆ (ಅತಿಪ್ರವಾಹ) {
    // We know whether or not we overflowed.
    ಆದರೆ (!(*ಅತಿಪ್ರವಾಹ)) {
      // No overflow.
      ಹಿಂತಿರುಗಿ Res;
    }

    // We overflowed
    APInt C;
    ಆದರೆ (Signed) {
      // sadd.sat / ssub.sat
      ಪ್ರತಿಜ್ಞಾ(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2)
                           : APInt::get_Signed_Max_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
    } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೂಡಿಸು_2) {
      // uadd.sat
      C = APInt::get_Max_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
    } ಇಲ್ಲದಿದ್ದರೆ {
      // uadd.sat
      C = APInt::get_Min_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2);
    }

    Res.One = C;
    Res.Zero = ~C;
    ಹಿಂತಿರುಗಿ Res;
  }

  // We don't know if we overflowed.
  ಆದರೆ (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    ಆದರೆ (MayPosClamp)
      Res.Zero.clear_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು(ಬಿಟ್_ಅಗಲ_2 - 1);
    ಆದರೆ (MayNegClamp)
      Res.One.clear_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು(ಬಿಟ್_ಅಗಲ_2 - 1);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕೂಡಿಸು_2) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clear_All_ಬಿಟ್ಟುಗಳು();
  } ಇಲ್ಲದಿದ್ದರೆ {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clear_All_ಬಿಟ್ಟುಗಳು();
  }

  ಹಿಂತಿರುಗಿ Res;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::sadd_sat(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಹಿಂತಿರುಗಿ compute_For_Sat_ಕೂಡಿಸು_Sub(/*ಕೂಡಿಸು*/ ನಿಜ, /*Signed*/ ನಿಜ, LHS, RHS);
}
ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::ssub_sat(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಹಿಂತಿರುಗಿ compute_For_Sat_ಕೂಡಿಸು_Sub(/*ಕೂಡಿಸು*/ ಸುಳ್ಳು, /*Signed*/ ನಿಜ, LHS, RHS);
}
ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::uadd_sat(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಹಿಂತಿರುಗಿ compute_For_Sat_ಕೂಡಿಸು_Sub(/*ಕೂಡಿಸು*/ ನಿಜ, /*Signed*/ ಸುಳ್ಳು, LHS, RHS);
}
ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::usub_sat(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಹಿಂತಿರುಗಿ compute_For_Sat_ಕೂಡಿಸು_Sub(/*ಕೂಡಿಸು*/ ಸುಳ್ಳು, /*Signed*/ ಸುಳ್ಳು, LHS, RHS);
}

ಸ್ಥಿರ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು avgComputeU(ತಿಳಿದ_ಬಿಟ್ಟುಗಳು LHS, ತಿಳಿದ_ಬಿಟ್ಟುಗಳು RHS, ತಾರ್ಕಿಕ IsCeil) {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = LHS.get_ಬಿಟ್_ಅಗಲ();
  LHS = LHS.zext(ಬಿಟ್_ಅಗಲ_2 + 1);
  RHS = RHS.zext(ಬಿಟ್_ಅಗಲ_2 + 1);
  LHS =
      compute_For_ಕೂಡಿಸು_ವಾಹಕ(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.ಹೊರತೆಗೆ_ಬಿಟ್ಟುಗಳು(ಬಿಟ್_ಅಗಲ_2, 1);
  ಹಿಂತಿರುಗಿ LHS;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::avgFloorS(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಹಿಂತಿರುಗಿ flip_Sign_ಬಿಟ್(avgFloorU(flip_Sign_ಬಿಟ್(LHS), flip_Sign_ಬಿಟ್(RHS)));
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::avgFloorU(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಹಿಂತಿರುಗಿ avgComputeU(LHS, RHS, /*IsCeil=*/ಸುಳ್ಳು);
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::avgCeilS(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಹಿಂತಿರುಗಿ flip_Sign_ಬಿಟ್(avgCeilU(flip_Sign_ಬಿಟ್(LHS), flip_Sign_ಬಿಟ್(RHS)));
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::avgCeilU(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಹಿಂತಿರುಗಿ avgComputeU(LHS, RHS, /*IsCeil=*/ನಿಜ);
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::mul(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS,
                         ತಾರ್ಕಿಕ NoUndefSelfMultiply) {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = LHS.get_ಬಿಟ್_ಅಗಲ();
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್_ಅಗಲ_2 == RHS.get_ಬಿಟ್_ಅಗಲ() && "Operand mismatch");
  ಪ್ರತಿಜ್ಞಾ((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.get_Max_ಮೌಲ್ಯ();
  APInt UMaxRHS = RHS.get_Max_ಮೌಲ್ಯ();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  ತಾರ್ಕಿಕ Has_ಅತಿಪ್ರವಾಹ;
  APInt UMax_ಫಲಿತಾಂಶ = UMaxLHS.umul_ov(UMaxRHS, Has_ಅತಿಪ್ರವಾಹ);
  ಚಿಹ್ನೆರಹಿತ LeadZ = Has_ಅತಿಪ್ರವಾಹ ? 0 : UMax_ಫಲಿತಾಂಶ.countl_zero();

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
  ಚಿಹ್ನೆರಹಿತ Trail_ಬಿಟ್ಟುಗಳು_ತಿಳಿದ_LHS = (LHS.Zero | LHS.One).countr_one();
  ಚಿಹ್ನೆರಹಿತ Trail_ಬಿಟ್ಟುಗಳು_ತಿಳಿದ_RHS = (RHS.Zero | RHS.One).countr_one();
  ಚಿಹ್ನೆರಹಿತ TrailZeroLHS = LHS.ಎಣಿಕೆ_Min_Trailing_Zeros();
  ಚಿಹ್ನೆರಹಿತ TrailZeroRHS = RHS.ಎಣಿಕೆ_Min_Trailing_Zeros();
  ಚಿಹ್ನೆರಹಿತ TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  ಚಿಹ್ನೆರಹಿತ ಅತಿಚಿಕ್ಕ_ಕಾರ್ಯಾಂಶ = std::min(Trail_ಬಿಟ್ಟುಗಳು_ತಿಳಿದ_LHS - TrailZeroLHS,
                                      Trail_ಬಿಟ್ಟುಗಳು_ತಿಳಿದ_RHS - TrailZeroRHS);
  ಚಿಹ್ನೆರಹಿತ ಫಲಿತಾಂಶ_ಬಿಟ್ಟುಗಳು_ತಿಳಿದ = std::min(ಅತಿಚಿಕ್ಕ_ಕಾರ್ಯಾಂಶ + TrailZ, ಬಿಟ್_ಅಗಲ_2);

  // The lower ResultBitsKnown bits of this are known.
  APInt Bottom_ತಿಳಿದ = LHS.One * RHS.One;

  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು Res(ಬಿಟ್_ಅಗಲ_2);
  Res.Zero.ಗಣ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು(LeadZ);
  Res.Zero |= (~Bottom_ತಿಳಿದ).get_Lo_ಬಿಟ್ಟುಗಳು(ಫಲಿತಾಂಶ_ಬಿಟ್ಟುಗಳು_ತಿಳಿದ);
  Res.One = Bottom_ತಿಳಿದ.get_Lo_ಬಿಟ್ಟುಗಳು(ಫಲಿತಾಂಶ_ಬಿಟ್ಟುಗಳು_ತಿಳಿದ);

  ಆದರೆ (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    ಚಿಹ್ನೆರಹಿತ TwoTZP1 = 2 * TrailZeroLHS + 1;
    ಆದರೆ (TwoTZP1 < ಬಿಟ್_ಅಗಲ_2)
      Res.Zero.ಗಣ_ಬಿಟ್(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    ಆದರೆ (TrailZeroLHS < ಬಿಟ್_ಅಗಲ_2 && LHS.One[TrailZeroLHS]) {
      ಚಿಹ್ನೆರಹಿತ TwoTZP2 = TwoTZP1 + 1;
      ಆದರೆ (TwoTZP2 < ಬಿಟ್_ಅಗಲ_2)
        Res.Zero.ಗಣ_ಬಿಟ್(TwoTZP2);
    }
  }

  ಹಿಂತಿರುಗಿ Res;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::mulhs(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = LHS.get_ಬಿಟ್_ಅಗಲ();
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್_ಅಗಲ_2 == RHS.get_ಬಿಟ್_ಅಗಲ() && "Operand mismatch");
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು WideLHS = LHS.sext(2 * ಬಿಟ್_ಅಗಲ_2);
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು WideRHS = RHS.sext(2 * ಬಿಟ್_ಅಗಲ_2);
  ಹಿಂತಿರುಗಿ mul(WideLHS, WideRHS).ಹೊರತೆಗೆ_ಬಿಟ್ಟುಗಳು(ಬಿಟ್_ಅಗಲ_2, ಬಿಟ್_ಅಗಲ_2);
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::mulhu(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = LHS.get_ಬಿಟ್_ಅಗಲ();
  ಪ್ರತಿಜ್ಞಾ(ಬಿಟ್_ಅಗಲ_2 == RHS.get_ಬಿಟ್_ಅಗಲ() && "Operand mismatch");
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು WideLHS = LHS.zext(2 * ಬಿಟ್_ಅಗಲ_2);
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು WideRHS = RHS.zext(2 * ಬಿಟ್_ಅಗಲ_2);
  ಹಿಂತಿರುಗಿ mul(WideLHS, WideRHS).ಹೊರತೆಗೆ_ಬಿಟ್ಟುಗಳು(ಬಿಟ್_ಅಗಲ_2, ಬಿಟ್_ಅಗಲ_2);
}

ಸ್ಥಿರ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು div_Compute_ಕೆಳಗಿನ_ಬಿಟ್(ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS,
                                  ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS, ತಾರ್ಕಿಕ Exact) {

  ಆದರೆ (!Exact)
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  ಆದರೆ (LHS.One[0])
    ತಿಳಿದ.One.ಗಣ_ಬಿಟ್(0);

  ಪೂರ್ಣಾಂಕ MinTZ =
      (ಪೂರ್ಣಾಂಕ)LHS.ಎಣಿಕೆ_Min_Trailing_Zeros() - (ಪೂರ್ಣಾಂಕ)RHS.ಎಣಿಕೆ_Max_Trailing_Zeros();
  ಪೂರ್ಣಾಂಕ MaxTZ =
      (ಪೂರ್ಣಾಂಕ)LHS.ಎಣಿಕೆ_Max_Trailing_Zeros() - (ಪೂರ್ಣಾಂಕ)RHS.ಎಣಿಕೆ_Min_Trailing_Zeros();
  ಆದರೆ (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    ತಿಳಿದ.Zero.ಗಣ_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು(MinTZ);
    ಆದರೆ (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      ತಿಳಿದ.One.ಗಣ_ಬಿಟ್(MinTZ);
    }
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (MaxTZ < 0) {
    // Poison Result
    ತಿಳಿದ.ಗಣ_All_Zero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  ಆದರೆ (ತಿಳಿದ.hasConflict())
    ತಿಳಿದ.ಗಣ_All_Zero();

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::sdiv(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS,
                          ತಾರ್ಕಿಕ Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  ಆದರೆ (LHS.isNonNegative() && RHS.isNonNegative())
    ಹಿಂತಿರುಗಿ udiv(LHS, RHS, Exact);

  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = LHS.get_ಬಿಟ್_ಅಗಲ();
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ(ಬಿಟ್_ಅಗಲ_2);

  ಆದರೆ (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    ತಿಳಿದ.ಗಣ_All_Zero();
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  }

  std::optional<APInt> Res;
  ಆದರೆ (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_ಮೌಲ್ಯ();
    APInt ಅಂಕೆ = LHS.get_Signed_Min_ಮೌಲ್ಯ();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (ಅಂಕೆ.is_Min_Signed_ಮೌಲ್ಯ() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_ಮೌಲ್ಯ(ಬಿಟ್_ಅಗಲ_2)
              : ಅಂಕೆ.sdiv(Denom);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    ಆದರೆ (Exact || (-LHS.get_Signed_Max_ಮೌಲ್ಯ()).uge(RHS.get_Signed_Max_ಮೌಲ್ಯ())) {
      APInt Denom = RHS.get_Signed_Min_ಮೌಲ್ಯ();
      APInt ಅಂಕೆ = LHS.get_Signed_Min_ಮೌಲ್ಯ();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      ಆದರೆ (Denom.isZero())
        Denom.ಗಣ_ಬಿಟ್(RHS.ಎಣಿಕೆ_Min_Trailing_Zeros());
      Res = ಅಂಕೆ.sdiv(Denom);
    }
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (LHS.is_Strictly_ಧನಾತ್ಮಕ() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    ಆದರೆ (Exact || LHS.get_Signed_Min_ಮೌಲ್ಯ().uge(-RHS.get_Signed_Min_ಮೌಲ್ಯ())) {
      APInt Denom = RHS.get_Signed_Max_ಮೌಲ್ಯ();
      APInt ಅಂಕೆ = LHS.get_Signed_Max_ಮೌಲ್ಯ();
      Res = ಅಂಕೆ.sdiv(Denom);
    }
  }

  ಆದರೆ (Res) {
    ಆದರೆ (Res->isNonNegative()) {
      ಚಿಹ್ನೆರಹಿತ LeadZ = Res->ಎಣಿಕೆ_Leading_Zeros();
      ತಿಳಿದ.Zero.ಗಣ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು(LeadZ);
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಚಿಹ್ನೆರಹಿತ LeadO = Res->ಎಣಿಕೆ_Leading_Ones();
      ತಿಳಿದ.One.ಗಣ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು(LeadO);
    }
  }

  ತಿಳಿದ = div_Compute_ಕೆಳಗಿನ_ಬಿಟ್(ತಿಳಿದ, LHS, RHS, Exact);
  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::udiv(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS,
                          ತಾರ್ಕಿಕ Exact) {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = LHS.get_ಬಿಟ್_ಅಗಲ();
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ(ಬಿಟ್_ಅಗಲ_2);

  ಆದರೆ (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    ತಿಳಿದ.ಗಣ_All_Zero();
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_ಮೌಲ್ಯ();
  ಆದರೆ (MinDenom.isZero())
    MinDenom.ಗಣ_ಬಿಟ್(RHS.ಎಣಿಕೆ_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_ಮೌಲ್ಯ().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_ಮೌಲ್ಯ().udiv(RHS.get_Max_ಮೌಲ್ಯ());

  ತಿಳಿದ = ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::makeConstant(MinRes);
  ಆದರೆ (std::optional<ಚಿಹ್ನೆರಹಿತ> Different_ಬಿಟ್ =
          APIntOps::Get_Most_Significant_Different_ಬಿಟ್(MinRes, MaxRes)) {
    ತಿಳಿದ.Zero.clear_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು(*Different_ಬಿಟ್ + 1);
    ತಿಳಿದ.One.clear_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು(*Different_ಬಿಟ್ + 1);
  }

  ತಿಳಿದ = div_Compute_ಕೆಳಗಿನ_ಬಿಟ್(ತಿಳಿದ, LHS, RHS, Exact);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::rem_Get_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = LHS.get_ಬಿಟ್_ಅಗಲ();
  ಆದರೆ (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    ಚಿಹ್ನೆರಹಿತ RHSZeros = RHS.ಎಣಿಕೆ_Min_Trailing_Zeros();
    APInt ಮುಖವಾಡ = APInt::get_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು_ಗಣ(ಬಿಟ್_ಅಗಲ_2, RHSZeros);
    APInt Ones_ಮುಖವಾಡ = LHS.One & ಮುಖವಾಡ;
    APInt Zeros_ಮುಖವಾಡ = LHS.Zero & ಮುಖವಾಡ;
    ಹಿಂತಿರುಗಿ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು(Zeros_ಮುಖವಾಡ, Ones_ಮುಖವಾಡ);
  }
  ಹಿಂತಿರುಗಿ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು(ಬಿಟ್_ಅಗಲ_2);
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::urem(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  ಆದರೆ (LHS.get_Max_ಮೌಲ್ಯ().ult(RHS.get_Min_ಮೌಲ್ಯ()))
    ಹಿಂತಿರುಗಿ LHS;

  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ = rem_Get_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_ಮೌಲ್ಯ(), RHS.get_Max_ಮೌಲ್ಯ() - 1);
  ತಿಳಿದ.Zero.ಗಣ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು(MaxRes.ಎಣಿಕೆ_Leading_Zeros());
  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::srem(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &LHS, ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ = rem_Get_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು(LHS, RHS);
  ಆದರೆ (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    ಆದರೆ (LHS.isNonNegative() || ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು.is_Subset_ದ(LHS.Zero))
      ತಿಳಿದ.Zero |= ~ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    ಆದರೆ (LHS.isNegative() && ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು.intersects(LHS.One))
      ತಿಳಿದ.One |= ~ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು;
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  ಆದರೆ (LHS.isNegative() && ತಿಳಿದ.isNonZero())
    ತಿಳಿದ.One.ಗಣ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು(
        std::max(LHS.ಎಣಿಕೆ_Min_Leading_Ones(), RHS.ಎಣಿಕೆ_Min_Sign_ಬಿಟ್ಟುಗಳು()));
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (LHS.isNonNegative())
    ತಿಳಿದ.Zero.ಗಣ_ಮೇಲಿನ_ಬಿಟ್ಟುಗಳು(
        std::max(LHS.ಎಣಿಕೆ_Min_Leading_Zeros(), RHS.ಎಣಿಕೆ_Min_Sign_ಬಿಟ್ಟುಗಳು()));
  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::ಕ್ರಿಯಾಕಾರಕ&=(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  ಹಿಂತಿರುಗಿ *ಇದು;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::ಕ್ರಿಯಾಕಾರಕ|=(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  ಹಿಂತಿರುಗಿ *ಇದು;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::ಕ್ರಿಯಾಕಾರಕ^=(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::blsi() ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = get_ಬಿಟ್_ಅಗಲ();
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ(Zero, APInt(ಬಿಟ್_ಅಗಲ_2, 0));
  ಚಿಹ್ನೆರಹಿತ Max = ಎಣಿಕೆ_Max_Trailing_Zeros();
  ತಿಳಿದ.Zero.ಗಣ_ಬಿಟ್ಟುಗಳು_From(std::min(Max + 1, ಬಿಟ್_ಅಗಲ_2));
  ಚಿಹ್ನೆರಹಿತ Min = ಎಣಿಕೆ_Min_Trailing_Zeros();
  ಆದರೆ (Max == Min && Max < ಬಿಟ್_ಅಗಲ_2)
    ತಿಳಿದ.One.ಗಣ_ಬಿಟ್(Max);
  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::blsmsk() ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = get_ಬಿಟ್_ಅಗಲ();
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ(ಬಿಟ್_ಅಗಲ_2);
  ಚಿಹ್ನೆರಹಿತ Max = ಎಣಿಕೆ_Max_Trailing_Zeros();
  ತಿಳಿದ.Zero.ಗಣ_ಬಿಟ್ಟುಗಳು_From(std::min(Max + 1, ಬಿಟ್_ಅಗಲ_2));
  ಚಿಹ್ನೆರಹಿತ Min = ಎಣಿಕೆ_Min_Trailing_Zeros();
  ತಿಳಿದ.One.ಗಣ_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು(std::min(Min + 1, ಬಿಟ್_ಅಗಲ_2));
  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ಶೂನ್ಯ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::print(raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = get_ಬಿಟ್_ಅಗಲ();
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ I = 0; I < ಬಿಟ್_ಅಗಲ_2; ++I) {
    ಚಿಹ್ನೆರಹಿತ N = ಬಿಟ್_ಅಗಲ_2 - I - 1;
    ಆದರೆ (Zero[N] && One[N])
      OS << "!";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Zero[N])
      OS << "0";
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (One[N])
      OS << "1";
    ಇಲ್ಲದಿದ್ದರೆ
      OS << "?";
  }
}

#ಆದರೆ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ವಿಧಾನ ಶೂನ್ಯ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::dump() ಸ್ಥಿರಾಂಕ {
  print(dbgs());
  dbgs() << "\n";
}
#ಆದರೆ_ಅಂತ್ಯ

ಚಿಹ್ನೆರಹಿತ llvm::Sign_ಬಿಟ್ಟುಗಳು_Ops::rot(ಚಿಹ್ನೆರಹಿತ Src_Sign_ಬಿಟ್ಟುಗಳು, ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2,
                                std::optional<APInt> RotAmt,
                                ತಾರ್ಕಿಕ Is_Rotate_ಬಲ) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  ಆದರೆ (Src_Sign_ಬಿಟ್ಟುಗಳು == ಬಿಟ್_ಅಗಲ_2)
    ಹಿಂತಿರುಗಿ ಬಿಟ್_ಅಗಲ_2;

  ಆದರೆ (!RotAmt)
    ಹಿಂತಿರುಗಿ 1;

  ಚಿಹ್ನೆರಹಿತ Amt = RotAmt->urem(ಬಿಟ್_ಅಗಲ_2);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  ಆದರೆ (Is_Rotate_ಬಲ)
    Amt = (ಬಿಟ್_ಅಗಲ_2 - Amt) % ಬಿಟ್_ಅಗಲ_2;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  ಆದರೆ (Src_Sign_ಬಿಟ್ಟುಗಳು > Amt + 1)
    ಹಿಂತಿರುಗಿ Src_Sign_ಬಿಟ್ಟುಗಳು - Amt;
  ಹಿಂತಿರುಗಿ 1;
}
