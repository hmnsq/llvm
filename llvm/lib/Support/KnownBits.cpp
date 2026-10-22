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

using ನಾಮಾಕಾಶ llvm;

KnownBits KnownBits::flipSignBit(ಧ್ರುವ KnownBits &Val) {
  unsigned SignBitPosition = Val.getBitWidth() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
  One.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
  ಪ್ರತ್ಯಾಗಮ KnownBits(Zero, One);
}

ಸ್ಥಿರ KnownBits computeForAddCarry(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS,
                                    ತಾರ್ಕಿಕ CarryZero, ತಾರ್ಕಿಕ CarryOne) {

  APInt PossibleSumZero = LHS.get_Max_ಮೌಲ್ಯ() + RHS.get_Max_ಮೌಲ್ಯ() + !CarryZero;
  APInt PossibleSumOne = LHS.get_Min_ಮೌಲ್ಯ() + RHS.get_Min_ಮೌಲ್ಯ() + CarryOne;

  // Compute known bits of the carry.
  APInt CarryKnownZero = ~(PossibleSumZero ^ LHS.Zero ^ RHS.Zero);
  APInt CarryKnownOne = PossibleSumOne ^ LHS.One ^ RHS.One;

  // Compute set of known bits (where all three relevant bits are known).
  APInt LHSKnownUnion = LHS.Zero | LHS.One;
  APInt RHSKnownUnion = RHS.Zero | RHS.One;
  APInt CarryKnownUnion = std::move(CarryKnownZero) | CarryKnownOne;
  APInt Known = std::move(LHSKnownUnion) & RHSKnownUnion & CarryKnownUnion;

  // Compute known bits of the result.
  KnownBits KnownOut;
  KnownOut.Zero = ~std::move(PossibleSumZero) & Known;
  KnownOut.One = std::move(PossibleSumOne) & Known;
  ಪ್ರತ್ಯಾಗಮ KnownOut;
}

KnownBits KnownBits::computeForAddCarry(
    ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS, ಧ್ರುವ KnownBits &Carry) {
  ಪ್ರತಿಜ್ಞಾ(Carry.getBitWidth() == 1 && "Carry must be 1-bit");
  ಪ್ರತ್ಯಾಗಮ ::computeForAddCarry(
      LHS, RHS, Carry.Zero.get_Bool_ಮೌಲ್ಯ(), Carry.One.get_Bool_ಮೌಲ್ಯ());
}

KnownBits KnownBits::computeForAddSub(ತಾರ್ಕಿಕ Add, ತಾರ್ಕಿಕ NSW, ತಾರ್ಕಿಕ NUW,
                                      ಧ್ರುವ KnownBits &LHS,
                                      ಧ್ರುವ KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits KnownOut(BitWidth);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  ಯದಿ (LHS.isUnknown() && RHS.isUnknown())
    ಪ್ರತ್ಯಾಗಮ KnownOut;

  ಯದಿ (!LHS.isUnknown() && !RHS.isUnknown()) {
    ಯದಿ (Add) {
      // Sum = LHS + RHS + 0
      KnownOut = ::computeForAddCarry(LHS, RHS, /*CarryZero=*/ಸತ್ಯ,
                                      /*CarryOne=*/ಅಸತ್ಯ);
    } ಅನ್ಯಥಾ {
      // Sum = LHS + ~RHS + 1
      KnownBits NotRHS = RHS;
      std::swap(NotRHS.Zero, NotRHS.One);
      KnownOut = ::computeForAddCarry(LHS, NotRHS, /*CarryZero=*/ಅಸತ್ಯ,
                                      /*CarryOne=*/ಸತ್ಯ);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  ಯದಿ (NUW) {
    ಯದಿ (Add) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_ಮೌಲ್ಯ().uadd_sat(RHS.get_Min_ಮೌಲ್ಯ());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      ಯದಿ (NSW) {
        unsigned ಅಂಕೆ_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        KnownOut.One.setBits(BitWidth - 1 - ಅಂಕೆ_Bits_2, BitWidth - 1);
      }
      KnownOut.One.set_ಮೇಲಿನ_Bits(MinVal.countl_one());
    } ಅನ್ಯಥಾ {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_ಮೌಲ್ಯ().usub_sat(RHS.get_Min_ಮೌಲ್ಯ());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      ಯದಿ (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        unsigned ಅಂಕೆ_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
        KnownOut.Zero.setBits(BitWidth - 1 - ಅಂಕೆ_Bits_2, BitWidth - 1);
      }
      KnownOut.Zero.set_ಮೇಲಿನ_Bits(MaxVal.countl_zero());
    }
  }

  ಯದಿ (NSW) {
    APInt MinVal;
    APInt MaxVal;
    ಯದಿ (Add) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_ಮೌಲ್ಯ().sadd_sat(RHS.get_Signed_Min_ಮೌಲ್ಯ());
      MaxVal = LHS.get_Signed_Max_ಮೌಲ್ಯ().sadd_sat(RHS.get_Signed_Max_ಮೌಲ್ಯ());
    } ಅನ್ಯಥಾ {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_ಮೌಲ್ಯ().ssub_sat(RHS.get_Signed_Max_ಮೌಲ್ಯ());
      MaxVal = LHS.get_Signed_Max_ಮೌಲ್ಯ().ssub_sat(RHS.get_Signed_Min_ಮೌಲ್ಯ());
    }
    ಯದಿ (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      unsigned ಅಂಕೆ_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
      KnownOut.One.setBits(BitWidth - 1 - ಅಂಕೆ_Bits_2, BitWidth - 1);
      KnownOut.Zero.setSignBit();
    }
    ಯದಿ (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      unsigned ಅಂಕೆ_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
      KnownOut.Zero.setBits(BitWidth - 1 - ಅಂಕೆ_Bits_2, BitWidth - 1);
      KnownOut.One.setSignBit();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  ಯದಿ (KnownOut.hasConflict())
    KnownOut.setAllZero();
  ಪ್ರತ್ಯಾಗಮ KnownOut;
}

KnownBits KnownBits::computeForSubBorrow(ಧ್ರುವ KnownBits &LHS, KnownBits RHS,
                                         ಧ್ರುವ KnownBits &Borrow) {
  ಪ್ರತಿಜ್ಞಾ(Borrow.getBitWidth() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::swap(RHS.Zero, RHS.One);
  ಪ್ರತ್ಯಾಗಮ ::computeForAddCarry(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_ಮೌಲ್ಯ(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_ಮೌಲ್ಯ());
}

KnownBits KnownBits::truncSSat(unsigned BitWidth) ಧ್ರುವ {
  unsigned ಒಳಹರಿವು_Bits = getBitWidth();
  APInt MinInRange = APInt::get_Signed_Min_ಮೌಲ್ಯ(BitWidth).sext(ಒಳಹರಿವು_Bits);
  APInt MaxInRange = APInt::get_Signed_Max_ಮೌಲ್ಯ(BitWidth).sext(ಒಳಹರಿವು_Bits);
  APInt ಒಳಹರಿವು_Min = get_Signed_Min_ಮೌಲ್ಯ();
  APInt ಒಳಹರಿವು_Max = get_Signed_Max_ಮೌಲ್ಯ();
  KnownBits Known(BitWidth);

  // Case 1: All values fit - just truncate
  ಯದಿ (ಒಳಹರಿವು_Min.sge(MinInRange) && ಒಳಹರಿವು_Max.sle(MaxInRange)) {
    Known = trunc(BitWidth);
  }
  // Case 2: All saturate to min
  ಅನ್ಯಥಾ ಯದಿ (ಒಳಹರಿವು_Max.slt(MinInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Min_ಮೌಲ್ಯ(BitWidth));
  }
  // Case 3: All saturate to max
  ಅನ್ಯಥಾ ಯದಿ (ಒಳಹರಿವು_Min.sgt(MaxInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Max_ಮೌಲ್ಯ(BitWidth));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  ಅನ್ಯಥಾ ಯದಿ (ಒಳಹರಿವು_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    Known.Zero = APInt(BitWidth, 0);
    Known.Zero.setBit(BitWidth - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    Known.One = One.trunc(BitWidth);
    Known.One.clearBit(BitWidth - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  ಅನ್ಯಥಾ ಯದಿ (ಒಳಹರಿವು_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    Known.Zero = Zero.trunc(BitWidth);
    Known.Zero.clearBit(BitWidth - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    Known.One = APInt(BitWidth, 0);
    Known.One.setBit(BitWidth - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  ಅನ್ಯಥಾ {
    // Output: min saturation, truncated, or max saturation
    APInt ಒಳಹರಿವು_Zero_Lower = Zero.trunc(BitWidth);
    APInt ಒಳಹರಿವು_One_Lower = One.trunc(BitWidth);
    APInt MinSat = APInt::get_Signed_Min_ಮೌಲ್ಯ(BitWidth);
    APInt MaxSat = APInt::get_Signed_Max_ಮೌಲ್ಯ(BitWidth);
    KnownBits MinSatKB = KnownBits::makeConstant(MinSat);
    KnownBits MaxSatKB = KnownBits::makeConstant(MaxSat);
    ಯದಿ (ಒಳಹರಿವು_Max.sle(MaxInRange)) {
      // Positive values fit, only negatives saturate to min
      Known.Zero = ಒಳಹರಿವು_Zero_Lower & MinSatKB.Zero;
      Known.One = ಒಳಹರಿವು_One_Lower & MinSatKB.One;
    } ಅನ್ಯಥಾ ಯದಿ (ಒಳಹರಿವು_Min.sge(MinInRange)) {
      // Negative values fit, only positives saturate to max
      Known.Zero = ಒಳಹರಿವು_Zero_Lower & MaxSatKB.Zero;
      Known.One = ಒಳಹರಿವು_One_Lower & MaxSatKB.One;
    } ಅನ್ಯಥಾ {
      // Both positive and negative values might saturate
      Known.Zero = ಒಳಹರಿವು_Zero_Lower & MinSatKB.Zero & MaxSatKB.Zero;
      Known.One = ಒಳಹರಿವು_One_Lower & MinSatKB.One & MaxSatKB.One;
    }
  }
  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownBits KnownBits::truncSSatU(unsigned BitWidth) ಧ್ರುವ {
  unsigned ಒಳಹರಿವು_Bits = getBitWidth();
  APInt MaxInRange = APInt::getAllOnes(BitWidth).zext(ಒಳಹರಿವು_Bits);
  APInt ಒಳಹರಿವು_Min = get_Signed_Min_ಮೌಲ್ಯ();
  APInt ಒಳಹರಿವು_Max = get_Signed_Max_ಮೌಲ್ಯ();
  KnownBits Known(BitWidth);

  ಯದಿ (isNegative()) {
    Known.setAllZero();
  } ಅನ್ಯಥಾ ಯದಿ (ಒಳಹರಿವು_Min.isNonNegative() && ಒಳಹರಿವು_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } ಅನ್ಯಥಾ ಯದಿ (ಒಳಹರಿವು_Min.isNonNegative() && ಒಳಹರಿವು_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } ಅನ್ಯಥಾ ಯದಿ (ಒಳಹರಿವು_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    Known.Zero = APInt(BitWidth, 0);
    // Common one bits: bits that are 1 in input stay 1
    Known.One = One.trunc(BitWidth);
  } ಅನ್ಯಥಾ {
    // Mixed: sign bit unknown
    ಯದಿ (ಒಳಹರಿವು_Max.ule(MaxInRange)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      Known.Zero = Zero.trunc(BitWidth);
      Known.One = APInt(BitWidth, 0);
    } ಅನ್ಯಥಾ {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      Known.Zero = APInt(BitWidth, 0);
      Known.One = APInt(BitWidth, 0);
    }
  }
  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownBits KnownBits::truncUSat(unsigned BitWidth) ಧ್ರುವ {
  unsigned ಒಳಹರಿವು_Bits = getBitWidth();
  APInt MaxInRange = APInt::get_ಕೆಳಗಿನ_Bits_Set(ಒಳಹರಿವು_Bits, BitWidth);
  APInt ಒಳಹರಿವು_Max = get_Max_ಮೌಲ್ಯ();
  APInt ಒಳಹರಿವು_Min = get_Min_ಮೌಲ್ಯ();
  KnownBits Known(BitWidth);

  ಯದಿ (ಒಳಹರಿವು_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } ಅನ್ಯಥಾ ಯದಿ (ಒಳಹರಿವು_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } ಅನ್ಯಥಾ {
    Known.resetAll();
    Known.One = One.trunc(BitWidth);
  }
  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownBits KnownBits::sextInReg(unsigned SrcBitWidth) ಧ್ರುವ {
  unsigned BitWidth = getBitWidth();
  ಪ್ರತಿಜ್ಞಾ(0 < SrcBitWidth && SrcBitWidth <= BitWidth &&
         "Illegal sext-in-register");

  ಯದಿ (SrcBitWidth == BitWidth)
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;

  unsigned ExtBits = BitWidth - SrcBitWidth;
  KnownBits Result;
  Result.One = One << ExtBits;
  Result.Zero = Zero << ExtBits;
  Result.One.ashrInPlace(ExtBits);
  Result.Zero.ashrInPlace(ExtBits);
  ಪ್ರತ್ಯಾಗಮ Result;
}

KnownBits KnownBits::makeGE(ಧ್ರುವ APInt &Val) ಧ್ರುವ {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  unsigned N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_ಕೆಳಗಿನ_Bits(getBitWidth() - N);
  ಪ್ರತ್ಯಾಗಮ KnownBits(Zero, One | MaskedVal);
}

KnownBits KnownBits::umax(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  ಯದಿ (LHS.get_Min_ಮೌಲ್ಯ().uge(RHS.get_Max_ಮೌಲ್ಯ()))
    ಪ್ರತ್ಯಾಗಮ LHS;
  ಯದಿ (RHS.get_Min_ಮೌಲ್ಯ().uge(LHS.get_Max_ಮೌಲ್ಯ()))
    ಪ್ರತ್ಯಾಗಮ RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  KnownBits L = LHS.makeGE(RHS.get_Min_ಮೌಲ್ಯ());
  KnownBits R = RHS.makeGE(LHS.get_Min_ಮೌಲ್ಯ());
  ಪ್ರತ್ಯಾಗಮ L.intersectWith(R);
}

KnownBits KnownBits::umin(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](ಧ್ರುವ KnownBits &Val) { ಪ್ರತ್ಯಾಗಮ KnownBits(Val.One, Val.Zero); };
  ಪ್ರತ್ಯಾಗಮ Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::smax(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಪ್ರತ್ಯಾಗಮ flipSignBit(umax(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::smin(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](ಧ್ರುವ KnownBits &Val) {
    unsigned SignBitPosition = Val.getBitWidth() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
    One.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
    ಪ್ರತ್ಯಾಗಮ KnownBits(Zero, One);
  };
  ಪ್ರತ್ಯಾಗಮ Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::abdu(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  ಯದಿ (LHS.get_Min_ಮೌಲ್ಯ().uge(RHS.get_Max_ಮೌಲ್ಯ()))
    ಪ್ರತ್ಯಾಗಮ computeForAddSub(/*Add=*/ಅಸತ್ಯ, /*NSW=*/ಅಸತ್ಯ, /*NUW=*/ಅಸತ್ಯ, LHS,
                            RHS);
  ಯದಿ (RHS.get_Min_ಮೌಲ್ಯ().uge(LHS.get_Max_ಮೌಲ್ಯ()))
    ಪ್ರತ್ಯಾಗಮ computeForAddSub(/*Add=*/ಅಸತ್ಯ, /*NSW=*/ಅಸತ್ಯ, /*NUW=*/ಅಸತ್ಯ, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/ಅಸತ್ಯ, /*NSW=*/ಅಸತ್ಯ, /*NUW=*/ಸತ್ಯ, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/ಅಸತ್ಯ, /*NSW=*/ಅಸತ್ಯ, /*NUW=*/ಸತ್ಯ, RHS, LHS);
  ಪ್ರತ್ಯಾಗಮ Diff0.intersectWith(Diff1);
}

KnownBits KnownBits::abds(KnownBits LHS, KnownBits RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  ಯದಿ (LHS.get_Signed_Min_ಮೌಲ್ಯ().sge(RHS.get_Signed_Max_ಮೌಲ್ಯ()))
    ಪ್ರತ್ಯಾಗಮ computeForAddSub(/*Add=*/ಅಸತ್ಯ, /*NSW=*/ಅಸತ್ಯ, /*NUW=*/ಅಸತ್ಯ, LHS,
                            RHS);
  ಯದಿ (RHS.get_Signed_Min_ಮೌಲ್ಯ().sge(LHS.get_Signed_Max_ಮೌಲ್ಯ()))
    ಪ್ರತ್ಯಾಗಮ computeForAddSub(/*Add=*/ಅಸತ್ಯ, /*NSW=*/ಅಸತ್ಯ, /*NUW=*/ಅಸತ್ಯ, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  unsigned SignBitPosition = LHS.getBitWidth() - 1;
  ಪ್ರತಿ (auto Arg : {&LHS, &RHS}) {
    ತಾರ್ಕಿಕ Tmp = Arg->Zero[SignBitPosition];
    Arg->Zero.setBitVal(SignBitPosition, Arg->One[SignBitPosition]);
    Arg->One.setBitVal(SignBitPosition, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/ಅಸತ್ಯ, /*NSW=*/ಅಸತ್ಯ, /*NUW=*/ಸತ್ಯ, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/ಅಸತ್ಯ, /*NSW=*/ಅಸತ್ಯ, /*NUW=*/ಸತ್ಯ, RHS, LHS);
  ಪ್ರತ್ಯಾಗಮ Diff0.intersectWith(Diff1);
}

ಸ್ಥಿರ unsigned getMaxShiftAmount(ಧ್ರುವ APInt &Max_ಮೌಲ್ಯ, unsigned BitWidth) {
  ಯದಿ (isPowerOf2_32(BitWidth))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    ಪ್ರತ್ಯಾಗಮ Max_ಮೌಲ್ಯ.extract_Bits_As_ZExt_ಮೌಲ್ಯ(
        std::min(Log2_32(BitWidth), Max_ಮೌಲ್ಯ.getBitWidth()), 0);
  // This is only an approximate upper bound.
  ಪ್ರತ್ಯಾಗಮ Max_ಮೌಲ್ಯ.get_Limited_ಮೌಲ್ಯ(BitWidth - 1);
}

KnownBits KnownBits::shl(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS, ತಾರ್ಕಿಕ NUW,
                         ತಾರ್ಕಿಕ NSW, ತಾರ್ಕಿಕ ShAmtNonZero) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ಧ್ರುವ KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known;
    ತಾರ್ಕಿಕ ShiftedOutZero, ShiftedOutOne;
    Known.Zero = LHS.Zero.ushl_ov(ShiftAmt, ShiftedOutZero);
    Known.Zero.set_ಕೆಳಗಿನ_Bits(ShiftAmt);
    Known.One = LHS.One.ushl_ov(ShiftAmt, ShiftedOutOne);

    // All cases returning poison have been handled by MaxShiftAmount already.
    ಯದಿ (NSW) {
      ಯದಿ (NUW && ShiftAmt != 0)
        // NUW means we can assume anything shifted out was a zero.
        ShiftedOutZero = ಸತ್ಯ;

      ಯದಿ (ShiftedOutZero)
        Known.makeNonNegative();
      ಅನ್ಯಥಾ ಯದಿ (ShiftedOutOne)
        Known.makeNegative();
    }
    ಪ್ರತ್ಯಾಗಮ Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_ಮೌಲ್ಯ().get_Limited_ಮೌಲ್ಯ(BitWidth);
  ಯದಿ (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  ಯದಿ (LHS.isUnknown()) {
    Known.Zero.set_ಕೆಳಗಿನ_Bits(MinShiftAmount);
    ಯದಿ (NUW && NSW && MinShiftAmount != 0)
      Known.makeNonNegative();
    ಪ್ರತ್ಯಾಗಮ Known;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_ಮೌಲ್ಯ = RHS.get_Max_ಮೌಲ್ಯ();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_ಮೌಲ್ಯ, BitWidth);
  ಯದಿ (NUW && NSW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.ಎಣಿಕೆ_Max_Leading_Zeros() - 1);
  ಯದಿ (NUW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.ಎಣಿಕೆ_Max_Leading_Zeros());
  ಯದಿ (NSW)
    MaxShiftAmount = std::min(
        MaxShiftAmount,
        std::max(LHS.ಎಣಿಕೆ_Max_Leading_Zeros(), LHS.ಎಣಿಕೆ_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  ಯದಿ (MinShiftAmount == 0 && MaxShiftAmount == BitWidth - 1 &&
      isPowerOf2_32(BitWidth)) {
    Known.Zero.set_ಕೆಳಗಿನ_Bits(LHS.ಎಣಿಕೆ_Min_Trailing_Zeros());
    ಯದಿ (LHS.isAllOnes())
      Known.One.setSignBit();
    ಯದಿ (NSW) {
      ಯದಿ (LHS.isNonNegative())
        Known.makeNonNegative();
      ಯದಿ (LHS.isNegative())
        Known.makeNegative();
    }
    ಪ್ರತ್ಯಾಗಮ Known;
  }

  // Find the common bits from all possible shifts.
  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_ಮೌಲ್ಯ();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_ಮೌಲ್ಯ();
  Known.setAllConflict();
  ಪ್ರತಿ (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    ಯದಿ ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      ಅನುವರ್ತ;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    ಯದಿ (Known.isUnknown())
      ಭಙ್ಗ;
  }

  // All shift amounts may result in poison.
  ಯದಿ (Known.hasConflict())
    Known.setAllZero();
  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownBits KnownBits::lshr(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS,
                          ತಾರ್ಕಿಕ ShAmtNonZero, ತಾರ್ಕಿಕ Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ಧ್ರುವ KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known >>= ShiftAmt;
    // High bits are known zero.
    Known.Zero.set_ಮೇಲಿನ_Bits(ShiftAmt);
    ಪ್ರತ್ಯಾಗಮ Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_ಮೌಲ್ಯ().get_Limited_ಮೌಲ್ಯ(BitWidth);
  ಯದಿ (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  ಯದಿ (LHS.isUnknown()) {
    Known.Zero.set_ಮೇಲಿನ_Bits(MinShiftAmount);
    ಪ್ರತ್ಯಾಗಮ Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_ಮೌಲ್ಯ = RHS.get_Max_ಮೌಲ್ಯ();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_ಮೌಲ್ಯ, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  ಯದಿ (Exact) {
    unsigned FirstOne = LHS.ಎಣಿಕೆ_Max_Trailing_Zeros();
    ಯದಿ (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      ಪ್ರತ್ಯಾಗಮ Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_ಮೌಲ್ಯ();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_ಮೌಲ್ಯ();
  Known.setAllConflict();
  ಪ್ರತಿ (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    ಯದಿ ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      ಅನುವರ್ತ;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    ಯದಿ (Known.isUnknown())
      ಭಙ್ಗ;
  }

  // All shift amounts may result in poison.
  ಯದಿ (Known.hasConflict())
    Known.setAllZero();
  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownBits KnownBits::ashr(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS,
                          ತಾರ್ಕಿಕ ShAmtNonZero, ತಾರ್ಕಿಕ Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ಧ್ರುವ KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known.Zero.ashrInPlace(ShiftAmt);
    Known.One.ashrInPlace(ShiftAmt);
    ಪ್ರತ್ಯಾಗಮ Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_ಮೌಲ್ಯ().get_Limited_ಮೌಲ್ಯ(BitWidth);
  ಯದಿ (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  ಯದಿ (LHS.isUnknown()) {
    ಯದಿ (MinShiftAmount == BitWidth) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      ಪ್ರತ್ಯಾಗಮ Known;
    }
    ಪ್ರತ್ಯಾಗಮ Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_ಮೌಲ್ಯ = RHS.get_Max_ಮೌಲ್ಯ();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_ಮೌಲ್ಯ, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  ಯದಿ (Exact) {
    unsigned FirstOne = LHS.ಎಣಿಕೆ_Max_Trailing_Zeros();
    ಯದಿ (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      ಪ್ರತ್ಯಾಗಮ Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_ಮೌಲ್ಯ();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_ಮೌಲ್ಯ();
  Known.setAllConflict();
  ಪ್ರತಿ (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
      ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    ಯದಿ ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      ಅನುವರ್ತ;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    ಯದಿ (Known.isUnknown())
      ಭಙ್ಗ;
  }

  // All shift amounts may result in poison.
  ಯದಿ (Known.hasConflict())
    Known.setAllZero();
  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownBits KnownBits::fshl(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS,
                          ಧ್ರುವ APInt &Amt) {
  ಪ್ರತ್ಯಾಗಮ KnownBits(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::fshr(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS,
                          ಧ್ರುವ APInt &Amt) {
  ಪ್ರತ್ಯಾಗಮ KnownBits(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::clmul(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  KnownBits Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_ಮೌಲ್ಯ(), RHS.get_Min_ಮೌಲ್ಯ()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  auto ClMulOr = [](ಧ್ರುವ APInt &LHS, ಧ್ರುವ APInt &RHS) {
    unsigned BW = LHS.getBitWidth();
    ಪ್ರತಿಜ್ಞಾ(BW == RHS.getBitWidth() && "Operand mismatch");
    APInt Result(BW, 0);
    ಪ್ರತಿ (unsigned I :
         seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
      ಯದಿ (RHS[I])
        Result |= LHS << I;
    ಪ್ರತ್ಯಾಗಮ Result;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt Known = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= Known;
  Res.One &= Known;

  ಪ್ರತ್ಯಾಗಮ Res;
}

KnownBits KnownBits::pext(ಧ್ರುವ KnownBits &Val, ಧ್ರುವ KnownBits &Mask) {
  unsigned BitWidth = Val.getBitWidth();
  KnownBits Res(BitWidth);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.setAllConflict();

  // For each source position I where Mask[I] could be 1, the output position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible output positions widens.
  unsigned M0 = 0, M1 = 0;
  ಪ್ರತಿ (unsigned I = 0; I < BitWidth; ++I) {
    ಯದಿ (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      ಯದಿ (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clearBits(M0, M1 + 1);
      ಯದಿ (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clearBits(M0, M1 + 1);
    }
    ಯದಿ (Mask.One[I])
      ++M0, ++M1;
    ಅನ್ಯಥಾ ಯದಿ (!Mask.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clearBits(M0, BitWidth);
  ಪ್ರತ್ಯಾಗಮ Res;
}

KnownBits KnownBits::pdep(ಧ್ರುವ KnownBits &Val, ಧ್ರುವ KnownBits &Mask) {
  unsigned BitWidth = Val.getBitWidth();
  KnownBits Res(BitWidth);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.setAllConflict();

  // For each output position I where Mask[I] could be 1, the source position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible source positions widens.
  unsigned M0 = 0, M1 = 0;
  ಪ್ರತಿ (unsigned I = 0; I < BitWidth; ++I) {
    ಯದಿ (!Mask.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clearBit(I);
    ಯದಿ (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::getBitsSet(BitWidth, M0, M1 + 1);
      ಯದಿ (!Range.isSubsetOf(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clearBit(I);
      ಯದಿ (!Range.isSubsetOf(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clearBit(I);
    }
    ಯದಿ (Mask.One[I])
      ++M0, ++M1;
    ಅನ್ಯಥಾ ಯದಿ (!Mask.Zero[I])
      ++M1;
  }
  ಪ್ರತ್ಯಾಗಮ Res;
}

std::optional<ತಾರ್ಕಿಕ> KnownBits::eq(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಯದಿ (LHS.isConstant() && RHS.isConstant())
    ಪ್ರತ್ಯಾಗಮ LHS.getConstant() == RHS.getConstant();
  ಯದಿ (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}

std::optional<ತಾರ್ಕಿಕ> KnownBits::ne(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಯದಿ (std::optional<ತಾರ್ಕಿಕ> KnownEQ = eq(LHS, RHS))
    ಪ್ರತ್ಯಾಗಮ !*KnownEQ;
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}

std::optional<ತಾರ್ಕಿಕ> KnownBits::ugt(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  ಯದಿ (LHS.get_Max_ಮೌಲ್ಯ().ule(RHS.get_Min_ಮೌಲ್ಯ()))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  ಯದಿ (LHS.get_Min_ಮೌಲ್ಯ().ugt(RHS.get_Max_ಮೌಲ್ಯ()))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}

std::optional<ತಾರ್ಕಿಕ> KnownBits::uge(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಯದಿ (std::optional<ತಾರ್ಕಿಕ> IsUGT = ugt(RHS, LHS))
    ಪ್ರತ್ಯಾಗಮ !*IsUGT;
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}

std::optional<ತಾರ್ಕಿಕ> KnownBits::ult(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಪ್ರತ್ಯಾಗಮ ugt(RHS, LHS);
}

std::optional<ತಾರ್ಕಿಕ> KnownBits::ule(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಪ್ರತ್ಯಾಗಮ uge(RHS, LHS);
}

std::optional<ತಾರ್ಕಿಕ> KnownBits::sgt(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  ಯದಿ (LHS.get_Signed_Max_ಮೌಲ್ಯ().sle(RHS.get_Signed_Min_ಮೌಲ್ಯ()))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  ಯದಿ (LHS.get_Signed_Min_ಮೌಲ್ಯ().sgt(RHS.get_Signed_Max_ಮೌಲ್ಯ()))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}

std::optional<ತಾರ್ಕಿಕ> KnownBits::sge(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಯದಿ (std::optional<ತಾರ್ಕಿಕ> KnownSGT = sgt(RHS, LHS))
    ಪ್ರತ್ಯಾಗಮ !*KnownSGT;
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}

std::optional<ತಾರ್ಕಿಕ> KnownBits::slt(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಪ್ರತ್ಯಾಗಮ sgt(RHS, LHS);
}

std::optional<ತಾರ್ಕಿಕ> KnownBits::sle(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಪ್ರತ್ಯಾಗಮ sge(RHS, LHS);
}

KnownBits KnownBits::abs(ತಾರ್ಕಿಕ IntMinIsPoison) ಧ್ರುವ {
  // If the source's MSB is zero then we know the rest of the bits already.
  ಯದಿ (isNonNegative())
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;

  // Absolute value preserves trailing zero count.
  KnownBits KnownAbs(getBitWidth());

  // If the input is negative, then abs(x) == -x.
  ಯದಿ (isNegative()) {
    KnownBits Tmp = *ಏತತ್;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    ಯದಿ (IntMinIsPoison && (Zero.popcount() + 2) == getBitWidth())
      Tmp.One.setBit(ಎಣಿಕೆ_Min_Trailing_Zeros());

    KnownAbs = computeForAddSub(
        /*Add*/ ಅಸತ್ಯ, IntMinIsPoison, /*NUW=*/ಅಸತ್ಯ,
        KnownBits::makeConstant(APInt(getBitWidth(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    ಯದಿ (IntMinIsPoison && Tmp.ಎಣಿಕೆ_Min_Population() == 1 &&
        Tmp.ಎಣಿಕೆ_Max_Population() != 1) {
      Tmp.One.clearSignBit();
      Tmp.Zero.setSignBit();
      KnownAbs.One.setBits(getBitWidth() - Tmp.ಎಣಿಕೆ_Min_Leading_Zeros(),
                           getBitWidth() - 1);
    }

  } ಅನ್ಯಥಾ {
    unsigned MaxTZ = ಎಣಿಕೆ_Max_Trailing_Zeros();
    unsigned MinTZ = ಎಣಿಕೆ_Min_Trailing_Zeros();

    KnownAbs.Zero.set_ಕೆಳಗಿನ_Bits(MinTZ);
    // If we know the lowest set 1, then preserve it.
    ಯದಿ (MaxTZ == MinTZ && MaxTZ < getBitWidth())
      KnownAbs.One.setBit(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    ಯದಿ (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_ಮೌಲ್ಯ())) {
      KnownAbs.One.clearSignBit();
      KnownAbs.Zero.setSignBit();
    }
  }

  ಪ್ರತ್ಯಾಗಮ KnownAbs;
}

KnownBits KnownBits::reduceAdd(unsigned ಅಂಕೆ_Elts) ಧ್ರುವ {
  ಯದಿ (ಅಂಕೆ_Elts == 0)
    ಪ್ರತ್ಯಾಗಮ KnownBits(getBitWidth());

  unsigned BitWidth = getBitWidth();
  KnownBits Result(BitWidth);

  ಯದಿ (isConstant())
    // If all elements are the same constant, we can simply compute it
    ಪ್ರತ್ಯಾಗಮ KnownBits::makeConstant(ಅಂಕೆ_Elts * getConstant());

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
  unsigned LostBits = Log2_32_Ceil(ಅಂಕೆ_Elts);

  ಯದಿ (isNonNegative()) {
    unsigned LeadingZeros = ಎಣಿಕೆ_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > LostBits ? LeadingZeros - LostBits : 0;
    Result.Zero.set_ಮೇಲಿನ_Bits(LeadingZeros);
  } ಅನ್ಯಥಾ ಯದಿ (isNegative()) {
    unsigned LeadingOnes = ಎಣಿಕೆ_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > LostBits ? LeadingOnes - LostBits : 0;
    Result.One.set_ಮೇಲಿನ_Bits(LeadingOnes);
  }

  ಪ್ರತ್ಯಾಗಮ Result;
}

ಸ್ಥಿರ KnownBits computeForSatAddSub(ತಾರ್ಕಿಕ Add, ತಾರ್ಕಿಕ Signed,
                                     ಧ್ರುವ KnownBits &LHS,
                                     ಧ್ರುವ KnownBits &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  unsigned BitWidth = LHS.getBitWidth();

  std::optional<ತಾರ್ಕಿಕ> Overflow;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  ತಾರ್ಕಿಕ MayNegClamp = ಸತ್ಯ;
  ತಾರ್ಕಿಕ MayPosClamp = ಸತ್ಯ;
  ಯದಿ (Signed) {
    // Easy cases we can rule out any overflow.
    ಯದಿ (Add && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      Overflow = ಅಸತ್ಯ;
    ಅನ್ಯಥಾ ಯದಿ (!Add && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      Overflow = ಅಸತ್ಯ;
    ಅನ್ಯಥಾ {
      // Check if we may overflow. If we can't rule out overflow then check if
      // we can rule out a direction at least.
      KnownBits UnsignedLHS = LHS;
      KnownBits UnsignedRHS = RHS;
      // Get version of LHS/RHS with clearer signbit. This allows us to detect
      // how the addition/subtraction might overflow into the signbit. Then
      // using the actual known signbits of LHS/RHS, we can figure out which
      // overflows are/aren't possible.
      UnsignedLHS.One.clearSignBit();
      UnsignedLHS.Zero.setSignBit();
      UnsignedRHS.One.clearSignBit();
      UnsignedRHS.Zero.setSignBit();
      KnownBits Res =
          KnownBits::computeForAddSub(Add, /*NSW=*/ಅಸತ್ಯ,
                                      /*NUW=*/ಅಸತ್ಯ, UnsignedLHS, UnsignedRHS);
      ಯದಿ (Add) {
        ಯದಿ (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = ಅಸತ್ಯ;
          // Pos + Pos will overflow with extra signbit.
          ಯದಿ (LHS.isNonNegative() && RHS.isNonNegative())
            Overflow = ಸತ್ಯ;
        } ಅನ್ಯಥಾ ಯದಿ (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = ಅಸತ್ಯ;
          // Neg + Neg will overflow without extra signbit.
          ಯದಿ (LHS.isNegative() && RHS.isNegative())
            Overflow = ಸತ್ಯ;
        }
        // We will never clamp to the opposite sign of N-bit result.
        ಯದಿ (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = ಅಸತ್ಯ;
        ಯದಿ (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = ಅಸತ್ಯ;
      } ಅನ್ಯಥಾ {
        ಯದಿ (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = ಅಸತ್ಯ;
          // Neg - Pos will overflow with extra signbit.
          ಯದಿ (LHS.isNegative() && RHS.isNonNegative())
            Overflow = ಸತ್ಯ;
        } ಅನ್ಯಥಾ ಯದಿ (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = ಅಸತ್ಯ;
          // Pos - Neg will overflow without extra signbit.
          ಯದಿ (LHS.isNonNegative() && RHS.isNegative())
            Overflow = ಸತ್ಯ;
        }
        // We will never clamp to the opposite sign of N-bit result.
        ಯದಿ (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = ಅಸತ್ಯ;
        ಯದಿ (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = ಅಸತ್ಯ;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    ಯದಿ (!MayNegClamp && !MayPosClamp)
      Overflow = ಅಸತ್ಯ;
  } ಅನ್ಯಥಾ ಯದಿ (Add) {
    // uadd.sat
    ತಾರ್ಕಿಕ Of;
    (ಶೂನ್ಯ)LHS.get_Max_ಮೌಲ್ಯ().uadd_ov(RHS.get_Max_ಮೌಲ್ಯ(), Of);
    ಯದಿ (!Of) {
      Overflow = ಅಸತ್ಯ;
    } ಅನ್ಯಥಾ {
      (ಶೂನ್ಯ)LHS.get_Min_ಮೌಲ್ಯ().uadd_ov(RHS.get_Min_ಮೌಲ್ಯ(), Of);
      ಯದಿ (Of)
        Overflow = ಸತ್ಯ;
    }
  } ಅನ್ಯಥಾ {
    // usub.sat
    ತಾರ್ಕಿಕ Of;
    (ಶೂನ್ಯ)LHS.get_Min_ಮೌಲ್ಯ().usub_ov(RHS.get_Max_ಮೌಲ್ಯ(), Of);
    ಯದಿ (!Of) {
      Overflow = ಅಸತ್ಯ;
    } ಅನ್ಯಥಾ {
      (ಶೂನ್ಯ)LHS.get_Max_ಮೌಲ್ಯ().usub_ov(RHS.get_Min_ಮೌಲ್ಯ(), Of);
      ಯದಿ (Of)
        Overflow = ಸತ್ಯ;
    }
  }

  KnownBits Res = KnownBits::computeForAddSub(Add, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  ಯದಿ (Overflow) {
    // We know whether or not we overflowed.
    ಯದಿ (!(*Overflow)) {
      // No overflow.
      ಪ್ರತ್ಯಾಗಮ Res;
    }

    // We overflowed
    APInt C;
    ಯದಿ (Signed) {
      // sadd.sat / ssub.sat
      ಪ್ರತಿಜ್ಞಾ(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_ಮೌಲ್ಯ(BitWidth)
                           : APInt::get_Signed_Max_ಮೌಲ್ಯ(BitWidth);
    } ಅನ್ಯಥಾ ಯದಿ (Add) {
      // uadd.sat
      C = APInt::get_Max_ಮೌಲ್ಯ(BitWidth);
    } ಅನ್ಯಥಾ {
      // uadd.sat
      C = APInt::get_Min_ಮೌಲ್ಯ(BitWidth);
    }

    Res.One = C;
    Res.Zero = ~C;
    ಪ್ರತ್ಯಾಗಮ Res;
  }

  // We don't know if we overflowed.
  ಯದಿ (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    ಯದಿ (MayPosClamp)
      Res.Zero.clear_ಕೆಳಗಿನ_Bits(BitWidth - 1);
    ಯದಿ (MayNegClamp)
      Res.One.clear_ಕೆಳಗಿನ_Bits(BitWidth - 1);
  } ಅನ್ಯಥಾ ಯದಿ (Add) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clearAllBits();
  } ಅನ್ಯಥಾ {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clearAllBits();
  }

  ಪ್ರತ್ಯಾಗಮ Res;
}

KnownBits KnownBits::sadd_sat(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಪ್ರತ್ಯಾಗಮ computeForSatAddSub(/*Add*/ ಸತ್ಯ, /*Signed*/ ಸತ್ಯ, LHS, RHS);
}
KnownBits KnownBits::ssub_sat(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಪ್ರತ್ಯಾಗಮ computeForSatAddSub(/*Add*/ ಅಸತ್ಯ, /*Signed*/ ಸತ್ಯ, LHS, RHS);
}
KnownBits KnownBits::uadd_sat(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಪ್ರತ್ಯಾಗಮ computeForSatAddSub(/*Add*/ ಸತ್ಯ, /*Signed*/ ಅಸತ್ಯ, LHS, RHS);
}
KnownBits KnownBits::usub_sat(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಪ್ರತ್ಯಾಗಮ computeForSatAddSub(/*Add*/ ಅಸತ್ಯ, /*Signed*/ ಅಸತ್ಯ, LHS, RHS);
}

ಸ್ಥಿರ KnownBits avgComputeU(KnownBits LHS, KnownBits RHS, ತಾರ್ಕಿಕ IsCeil) {
  unsigned BitWidth = LHS.getBitWidth();
  LHS = LHS.zext(BitWidth + 1);
  RHS = RHS.zext(BitWidth + 1);
  LHS =
      computeForAddCarry(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.extractBits(BitWidth, 1);
  ಪ್ರತ್ಯಾಗಮ LHS;
}

KnownBits KnownBits::avgFloorS(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಪ್ರತ್ಯಾಗಮ flipSignBit(avgFloorU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgFloorU(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಪ್ರತ್ಯಾಗಮ avgComputeU(LHS, RHS, /*IsCeil=*/ಅಸತ್ಯ);
}

KnownBits KnownBits::avgCeilS(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಪ್ರತ್ಯಾಗಮ flipSignBit(avgCeilU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgCeilU(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  ಪ್ರತ್ಯಾಗಮ avgComputeU(LHS, RHS, /*IsCeil=*/ಸತ್ಯ);
}

KnownBits KnownBits::mul(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS,
                         ತಾರ್ಕಿಕ NoUndefSelfMultiply) {
  unsigned BitWidth = LHS.getBitWidth();
  ಪ್ರತಿಜ್ಞಾ(BitWidth == RHS.getBitWidth() && "Operand mismatch");
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
  ತಾರ್ಕಿಕ HasOverflow;
  APInt UMaxResult = UMaxLHS.umul_ov(UMaxRHS, HasOverflow);
  unsigned LeadZ = HasOverflow ? 0 : UMaxResult.countl_zero();

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
  unsigned TrailBitsKnownLHS = (LHS.Zero | LHS.One).countr_one();
  unsigned TrailBitsKnownRHS = (RHS.Zero | RHS.One).countr_one();
  unsigned TrailZeroLHS = LHS.ಎಣಿಕೆ_Min_Trailing_Zeros();
  unsigned TrailZeroRHS = RHS.ಎಣಿಕೆ_Min_Trailing_Zeros();
  unsigned TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  unsigned SmallestOperand = std::min(TrailBitsKnownLHS - TrailZeroLHS,
                                      TrailBitsKnownRHS - TrailZeroRHS);
  unsigned ResultBitsKnown = std::min(SmallestOperand + TrailZ, BitWidth);

  // The lower ResultBitsKnown bits of this are known.
  APInt BottomKnown = LHS.One * RHS.One;

  KnownBits Res(BitWidth);
  Res.Zero.set_ಮೇಲಿನ_Bits(LeadZ);
  Res.Zero |= (~BottomKnown).getLoBits(ResultBitsKnown);
  Res.One = BottomKnown.getLoBits(ResultBitsKnown);

  ಯದಿ (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    unsigned TwoTZP1 = 2 * TrailZeroLHS + 1;
    ಯದಿ (TwoTZP1 < BitWidth)
      Res.Zero.setBit(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    ಯದಿ (TrailZeroLHS < BitWidth && LHS.One[TrailZeroLHS]) {
      unsigned TwoTZP2 = TwoTZP1 + 1;
      ಯದಿ (TwoTZP2 < BitWidth)
        Res.Zero.setBit(TwoTZP2);
    }
  }

  ಪ್ರತ್ಯಾಗಮ Res;
}

KnownBits KnownBits::mulhs(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  ಪ್ರತಿಜ್ಞಾ(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.sext(2 * BitWidth);
  KnownBits WideRHS = RHS.sext(2 * BitWidth);
  ಪ್ರತ್ಯಾಗಮ mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

KnownBits KnownBits::mulhu(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  ಪ್ರತಿಜ್ಞಾ(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.zext(2 * BitWidth);
  KnownBits WideRHS = RHS.zext(2 * BitWidth);
  ಪ್ರತ್ಯಾಗಮ mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

ಸ್ಥಿರ KnownBits div_Compute_ಕೆಳಗಿನ_Bit(KnownBits Known, ಧ್ರುವ KnownBits &LHS,
                                  ಧ್ರುವ KnownBits &RHS, ತಾರ್ಕಿಕ Exact) {

  ಯದಿ (!Exact)
    ಪ್ರತ್ಯಾಗಮ Known;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  ಯದಿ (LHS.One[0])
    Known.One.setBit(0);

  ಪೂರ್ಣಾಙ್ಕ MinTZ =
      (ಪೂರ್ಣಾಙ್ಕ)LHS.ಎಣಿಕೆ_Min_Trailing_Zeros() - (ಪೂರ್ಣಾಙ್ಕ)RHS.ಎಣಿಕೆ_Max_Trailing_Zeros();
  ಪೂರ್ಣಾಙ್ಕ MaxTZ =
      (ಪೂರ್ಣಾಙ್ಕ)LHS.ಎಣಿಕೆ_Max_Trailing_Zeros() - (ಪೂರ್ಣಾಙ್ಕ)RHS.ಎಣಿಕೆ_Min_Trailing_Zeros();
  ಯದಿ (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    Known.Zero.set_ಕೆಳಗಿನ_Bits(MinTZ);
    ಯದಿ (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      Known.One.setBit(MinTZ);
    }
  } ಅನ್ಯಥಾ ಯದಿ (MaxTZ < 0) {
    // Poison Result
    Known.setAllZero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  ಯದಿ (Known.hasConflict())
    Known.setAllZero();

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownBits KnownBits::sdiv(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS,
                          ತಾರ್ಕಿಕ Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  ಯದಿ (LHS.isNonNegative() && RHS.isNonNegative())
    ಪ್ರತ್ಯಾಗಮ udiv(LHS, RHS, Exact);

  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  ಯದಿ (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    ಪ್ರತ್ಯಾಗಮ Known;
  }

  std::optional<APInt> Res;
  ಯದಿ (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_ಮೌಲ್ಯ();
    APInt ಅಂಕೆ = LHS.get_Signed_Min_ಮೌಲ್ಯ();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (ಅಂಕೆ.is_Min_Signed_ಮೌಲ್ಯ() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_ಮೌಲ್ಯ(BitWidth)
              : ಅಂಕೆ.sdiv(Denom);
  } ಅನ್ಯಥಾ ಯದಿ (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    ಯದಿ (Exact || (-LHS.get_Signed_Max_ಮೌಲ್ಯ()).uge(RHS.get_Signed_Max_ಮೌಲ್ಯ())) {
      APInt Denom = RHS.get_Signed_Min_ಮೌಲ್ಯ();
      APInt ಅಂಕೆ = LHS.get_Signed_Min_ಮೌಲ್ಯ();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      ಯದಿ (Denom.isZero())
        Denom.setBit(RHS.ಎಣಿಕೆ_Min_Trailing_Zeros());
      Res = ಅಂಕೆ.sdiv(Denom);
    }
  } ಅನ್ಯಥಾ ಯದಿ (LHS.isStrictlyPositive() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    ಯದಿ (Exact || LHS.get_Signed_Min_ಮೌಲ್ಯ().uge(-RHS.get_Signed_Min_ಮೌಲ್ಯ())) {
      APInt Denom = RHS.get_Signed_Max_ಮೌಲ್ಯ();
      APInt ಅಂಕೆ = LHS.get_Signed_Max_ಮೌಲ್ಯ();
      Res = ಅಂಕೆ.sdiv(Denom);
    }
  }

  ಯದಿ (Res) {
    ಯದಿ (Res->isNonNegative()) {
      unsigned LeadZ = Res->ಎಣಿಕೆ_Leading_Zeros();
      Known.Zero.set_ಮೇಲಿನ_Bits(LeadZ);
    } ಅನ್ಯಥಾ {
      unsigned LeadO = Res->ಎಣಿಕೆ_Leading_Ones();
      Known.One.set_ಮೇಲಿನ_Bits(LeadO);
    }
  }

  Known = div_Compute_ಕೆಳಗಿನ_Bit(Known, LHS, RHS, Exact);
  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownBits KnownBits::udiv(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS,
                          ತಾರ್ಕಿಕ Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  ಯದಿ (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    ಪ್ರತ್ಯಾಗಮ Known;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_ಮೌಲ್ಯ();
  ಯದಿ (MinDenom.isZero())
    MinDenom.setBit(RHS.ಎಣಿಕೆ_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_ಮೌಲ್ಯ().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_ಮೌಲ್ಯ().udiv(RHS.get_Max_ಮೌಲ್ಯ());

  Known = KnownBits::makeConstant(MinRes);
  ಯದಿ (std::optional<unsigned> DifferentBit =
          APIntOps::GetMostSignificantDifferentBit(MinRes, MaxRes)) {
    Known.Zero.clear_ಕೆಳಗಿನ_Bits(*DifferentBit + 1);
    Known.One.clear_ಕೆಳಗಿನ_Bits(*DifferentBit + 1);
  }

  Known = div_Compute_ಕೆಳಗಿನ_Bit(Known, LHS, RHS, Exact);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownBits KnownBits::rem_Get_ಕೆಳಗಿನ_Bits(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  ಯದಿ (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    unsigned RHSZeros = RHS.ಎಣಿಕೆ_Min_Trailing_Zeros();
    APInt Mask = APInt::get_ಕೆಳಗಿನ_Bits_Set(BitWidth, RHSZeros);
    APInt OnesMask = LHS.One & Mask;
    APInt ZerosMask = LHS.Zero & Mask;
    ಪ್ರತ್ಯಾಗಮ KnownBits(ZerosMask, OnesMask);
  }
  ಪ್ರತ್ಯಾಗಮ KnownBits(BitWidth);
}

KnownBits KnownBits::urem(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  ಯದಿ (LHS.get_Max_ಮೌಲ್ಯ().ult(RHS.get_Min_ಮೌಲ್ಯ()))
    ಪ್ರತ್ಯಾಗಮ LHS;

  KnownBits Known = rem_Get_ಕೆಳಗಿನ_Bits(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_ಮೌಲ್ಯ(), RHS.get_Max_ಮೌಲ್ಯ() - 1);
  Known.Zero.set_ಮೇಲಿನ_Bits(MaxRes.ಎಣಿಕೆ_Leading_Zeros());
  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownBits KnownBits::srem(ಧ್ರುವ KnownBits &LHS, ಧ್ರುವ KnownBits &RHS) {
  KnownBits Known = rem_Get_ಕೆಳಗಿನ_Bits(LHS, RHS);
  ಯದಿ (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt ಕೆಳಗಿನ_Bits = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    ಯದಿ (LHS.isNonNegative() || ಕೆಳಗಿನ_Bits.isSubsetOf(LHS.Zero))
      Known.Zero |= ~ಕೆಳಗಿನ_Bits;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    ಯದಿ (LHS.isNegative() && ಕೆಳಗಿನ_Bits.intersects(LHS.One))
      Known.One |= ~ಕೆಳಗಿನ_Bits;
    ಪ್ರತ್ಯಾಗಮ Known;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  ಯದಿ (LHS.isNegative() && Known.isNonZero())
    Known.One.set_ಮೇಲಿನ_Bits(
        std::max(LHS.ಎಣಿಕೆ_Min_Leading_Ones(), RHS.ಎಣಿಕೆ_Min_Sign_Bits()));
  ಅನ್ಯಥಾ ಯದಿ (LHS.isNonNegative())
    Known.Zero.set_ಮೇಲಿನ_Bits(
        std::max(LHS.ಎಣಿಕೆ_Min_Leading_Zeros(), RHS.ಎಣಿಕೆ_Min_Sign_Bits()));
  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownBits &KnownBits::operator&=(ಧ್ರುವ KnownBits &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

KnownBits &KnownBits::operator|=(ಧ್ರುವ KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

KnownBits &KnownBits::operator^=(ಧ್ರುವ KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

KnownBits KnownBits::blsi() ಧ್ರುವ {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(Zero, APInt(BitWidth, 0));
  unsigned Max = ಎಣಿಕೆ_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = ಎಣಿಕೆ_Min_Trailing_Zeros();
  ಯದಿ (Max == Min && Max < BitWidth)
    Known.One.setBit(Max);
  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownBits KnownBits::blsmsk() ಧ್ರುವ {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(BitWidth);
  unsigned Max = ಎಣಿಕೆ_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = ಎಣಿಕೆ_Min_Trailing_Zeros();
  Known.One.set_ಕೆಳಗಿನ_Bits(std::min(Min + 1, BitWidth));
  ಪ್ರತ್ಯಾಗಮ Known;
}

ಶೂನ್ಯ KnownBits::print(raw_ostream &OS) ಧ್ರುವ {
  unsigned BitWidth = getBitWidth();
  ಪ್ರತಿ (unsigned I = 0; I < BitWidth; ++I) {
    unsigned N = BitWidth - I - 1;
    ಯದಿ (Zero[N] && One[N])
      OS << "!";
    ಅನ್ಯಥಾ ಯದಿ (Zero[N])
      OS << "0";
    ಅನ್ಯಥಾ ಯದಿ (One[N])
      OS << "1";
    ಅನ್ಯಥಾ
      OS << "?";
  }
}

#ಯದಿ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ಶೂನ್ಯ KnownBits::dump() ಧ್ರುವ {
  print(dbgs());
  dbgs() << "\n";
}
#endif

unsigned llvm::SignBitsOps::rot(unsigned SrcSignBits, unsigned BitWidth,
                                std::optional<APInt> RotAmt,
                                ತಾರ್ಕಿಕ Is_Rotate_ಬಲ) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  ಯದಿ (SrcSignBits == BitWidth)
    ಪ್ರತ್ಯಾಗಮ BitWidth;

  ಯದಿ (!RotAmt)
    ಪ್ರತ್ಯಾಗಮ 1;

  unsigned Amt = RotAmt->urem(BitWidth);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  ಯದಿ (Is_Rotate_ಬಲ)
    Amt = (BitWidth - Amt) % BitWidth;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  ಯದಿ (SrcSignBits > Amt + 1)
    ಪ್ರತ್ಯಾಗಮ SrcSignBits - Amt;
  ಪ್ರತ್ಯಾಗಮ 1;
}
