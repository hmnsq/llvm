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

#अन्तर्भाव "llvm/Support/KnownBits.h"
#अन्तर्भाव "llvm/ADT/Sequence.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cassert>

using नामाकाश llvm;

KnownBits KnownBits::flipSignBit(ध्रुव KnownBits &Val) {
  unsigned SignBitPosition = Val.getBitWidth() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
  One.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
  पटिनिवत्त KnownBits(Zero, One);
}

स्थिर KnownBits computeForAddCarry(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS,
                                    तार्किक CarryZero, तार्किक CarryOne) {

  APInt PossibleSumZero = LHS.get_Max_मानम्() + RHS.get_Max_मानम्() + !CarryZero;
  APInt PossibleSumOne = LHS.get_Min_मानम्() + RHS.get_Min_मानम्() + CarryOne;

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
  पटिनिवत्त KnownOut;
}

KnownBits KnownBits::computeForAddCarry(
    ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS, ध्रुव KnownBits &Carry) {
  प्रतिज्ञा(Carry.getBitWidth() == 1 && "Carry must be 1-bit");
  पटिनिवत्त ::computeForAddCarry(
      LHS, RHS, Carry.Zero.get_Bool_मानम्(), Carry.One.get_Bool_मानम्());
}

KnownBits KnownBits::computeForAddSub(तार्किक Add, तार्किक NSW, तार्किक NUW,
                                      ध्रुव KnownBits &LHS,
                                      ध्रुव KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits KnownOut(BitWidth);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  सचे (LHS.isUnknown() && RHS.isUnknown())
    पटिनिवत्त KnownOut;

  सचे (!LHS.isUnknown() && !RHS.isUnknown()) {
    सचे (Add) {
      // Sum = LHS + RHS + 0
      KnownOut = ::computeForAddCarry(LHS, RHS, /*CarryZero=*/सच्च,
                                      /*CarryOne=*/मिच्छा);
    } अञ्ञथा {
      // Sum = LHS + ~RHS + 1
      KnownBits NotRHS = RHS;
      std::swap(NotRHS.Zero, NotRHS.One);
      KnownOut = ::computeForAddCarry(LHS, NotRHS, /*CarryZero=*/मिच्छा,
                                      /*CarryOne=*/सच्च);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  सचे (NUW) {
    सचे (Add) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_मानम्().uadd_sat(RHS.get_Min_मानम्());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      सचे (NSW) {
        unsigned अङ्कः_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        KnownOut.One.setBits(BitWidth - 1 - अङ्कः_Bits_2, BitWidth - 1);
      }
      KnownOut.One.set_उच्चः_Bits(MinVal.countl_one());
    } अञ्ञथा {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_मानम्().usub_sat(RHS.get_Min_मानम्());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      सचे (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        unsigned अङ्कः_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
        KnownOut.Zero.setBits(BitWidth - 1 - अङ्कः_Bits_2, BitWidth - 1);
      }
      KnownOut.Zero.set_उच्चः_Bits(MaxVal.countl_zero());
    }
  }

  सचे (NSW) {
    APInt MinVal;
    APInt MaxVal;
    सचे (Add) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_मानम्().sadd_sat(RHS.get_Signed_Min_मानम्());
      MaxVal = LHS.get_Signed_Max_मानम्().sadd_sat(RHS.get_Signed_Max_मानम्());
    } अञ्ञथा {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_मानम्().ssub_sat(RHS.get_Signed_Max_मानम्());
      MaxVal = LHS.get_Signed_Max_मानम्().ssub_sat(RHS.get_Signed_Min_मानम्());
    }
    सचे (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      unsigned अङ्कः_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
      KnownOut.One.setBits(BitWidth - 1 - अङ्कः_Bits_2, BitWidth - 1);
      KnownOut.Zero.setSignBit();
    }
    सचे (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      unsigned अङ्कः_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
      KnownOut.Zero.setBits(BitWidth - 1 - अङ्कः_Bits_2, BitWidth - 1);
      KnownOut.One.setSignBit();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  सचे (KnownOut.hasConflict())
    KnownOut.setAllZero();
  पटिनिवत्त KnownOut;
}

KnownBits KnownBits::computeForSubBorrow(ध्रुव KnownBits &LHS, KnownBits RHS,
                                         ध्रुव KnownBits &Borrow) {
  प्रतिज्ञा(Borrow.getBitWidth() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::swap(RHS.Zero, RHS.One);
  पटिनिवत्त ::computeForAddCarry(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_मानम्(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_मानम्());
}

KnownBits KnownBits::truncSSat(unsigned BitWidth) ध्रुव {
  unsigned निवेशः_Bits = getBitWidth();
  APInt MinInRange = APInt::get_Signed_Min_मानम्(BitWidth).sext(निवेशः_Bits);
  APInt MaxInRange = APInt::get_Signed_Max_मानम्(BitWidth).sext(निवेशः_Bits);
  APInt निवेशः_Min = get_Signed_Min_मानम्();
  APInt निवेशः_Max = get_Signed_Max_मानम्();
  KnownBits Known(BitWidth);

  // Case 1: All values fit - just truncate
  सचे (निवेशः_Min.sge(MinInRange) && निवेशः_Max.sle(MaxInRange)) {
    Known = trunc(BitWidth);
  }
  // Case 2: All saturate to min
  अञ्ञथा सचे (निवेशः_Max.slt(MinInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Min_मानम्(BitWidth));
  }
  // Case 3: All saturate to max
  अञ्ञथा सचे (निवेशः_Min.sgt(MaxInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Max_मानम्(BitWidth));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  अञ्ञथा सचे (निवेशः_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    Known.Zero = APInt(BitWidth, 0);
    Known.Zero.setBit(BitWidth - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    Known.One = One.trunc(BitWidth);
    Known.One.clearBit(BitWidth - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  अञ्ञथा सचे (निवेशः_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    Known.Zero = Zero.trunc(BitWidth);
    Known.Zero.clearBit(BitWidth - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    Known.One = APInt(BitWidth, 0);
    Known.One.setBit(BitWidth - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  अञ्ञथा {
    // Output: min saturation, truncated, or max saturation
    APInt निवेशः_Zero_Lower = Zero.trunc(BitWidth);
    APInt निवेशः_One_Lower = One.trunc(BitWidth);
    APInt MinSat = APInt::get_Signed_Min_मानम्(BitWidth);
    APInt MaxSat = APInt::get_Signed_Max_मानम्(BitWidth);
    KnownBits MinSatKB = KnownBits::makeConstant(MinSat);
    KnownBits MaxSatKB = KnownBits::makeConstant(MaxSat);
    सचे (निवेशः_Max.sle(MaxInRange)) {
      // Positive values fit, only negatives saturate to min
      Known.Zero = निवेशः_Zero_Lower & MinSatKB.Zero;
      Known.One = निवेशः_One_Lower & MinSatKB.One;
    } अञ्ञथा सचे (निवेशः_Min.sge(MinInRange)) {
      // Negative values fit, only positives saturate to max
      Known.Zero = निवेशः_Zero_Lower & MaxSatKB.Zero;
      Known.One = निवेशः_One_Lower & MaxSatKB.One;
    } अञ्ञथा {
      // Both positive and negative values might saturate
      Known.Zero = निवेशः_Zero_Lower & MinSatKB.Zero & MaxSatKB.Zero;
      Known.One = निवेशः_One_Lower & MinSatKB.One & MaxSatKB.One;
    }
  }
  पटिनिवत्त Known;
}

KnownBits KnownBits::truncSSatU(unsigned BitWidth) ध्रुव {
  unsigned निवेशः_Bits = getBitWidth();
  APInt MaxInRange = APInt::getAllOnes(BitWidth).zext(निवेशः_Bits);
  APInt निवेशः_Min = get_Signed_Min_मानम्();
  APInt निवेशः_Max = get_Signed_Max_मानम्();
  KnownBits Known(BitWidth);

  सचे (isNegative()) {
    Known.setAllZero();
  } अञ्ञथा सचे (निवेशः_Min.isNonNegative() && निवेशः_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } अञ्ञथा सचे (निवेशः_Min.isNonNegative() && निवेशः_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } अञ्ञथा सचे (निवेशः_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    Known.Zero = APInt(BitWidth, 0);
    // Common one bits: bits that are 1 in input stay 1
    Known.One = One.trunc(BitWidth);
  } अञ्ञथा {
    // Mixed: sign bit unknown
    सचे (निवेशः_Max.ule(MaxInRange)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      Known.Zero = Zero.trunc(BitWidth);
      Known.One = APInt(BitWidth, 0);
    } अञ्ञथा {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      Known.Zero = APInt(BitWidth, 0);
      Known.One = APInt(BitWidth, 0);
    }
  }
  पटिनिवत्त Known;
}

KnownBits KnownBits::truncUSat(unsigned BitWidth) ध्रुव {
  unsigned निवेशः_Bits = getBitWidth();
  APInt MaxInRange = APInt::get_नीचः_Bits_Set(निवेशः_Bits, BitWidth);
  APInt निवेशः_Max = get_Max_मानम्();
  APInt निवेशः_Min = get_Min_मानम्();
  KnownBits Known(BitWidth);

  सचे (निवेशः_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } अञ्ञथा सचे (निवेशः_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } अञ्ञथा {
    Known.resetAll();
    Known.One = One.trunc(BitWidth);
  }
  पटिनिवत्त Known;
}

KnownBits KnownBits::sextInReg(unsigned SrcBitWidth) ध्रुव {
  unsigned BitWidth = getBitWidth();
  प्रतिज्ञा(0 < SrcBitWidth && SrcBitWidth <= BitWidth &&
         "Illegal sext-in-register");

  सचे (SrcBitWidth == BitWidth)
    पटिनिवत्त *एतत्;

  unsigned ExtBits = BitWidth - SrcBitWidth;
  KnownBits Result;
  Result.One = One << ExtBits;
  Result.Zero = Zero << ExtBits;
  Result.One.ashrInPlace(ExtBits);
  Result.Zero.ashrInPlace(ExtBits);
  पटिनिवत्त Result;
}

KnownBits KnownBits::makeGE(ध्रुव APInt &Val) ध्रुव {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  unsigned N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_नीचः_Bits(getBitWidth() - N);
  पटिनिवत्त KnownBits(Zero, One | MaskedVal);
}

KnownBits KnownBits::umax(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  सचे (LHS.get_Min_मानम्().uge(RHS.get_Max_मानम्()))
    पटिनिवत्त LHS;
  सचे (RHS.get_Min_मानम्().uge(LHS.get_Max_मानम्()))
    पटिनिवत्त RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  KnownBits L = LHS.makeGE(RHS.get_Min_मानम्());
  KnownBits R = RHS.makeGE(LHS.get_Min_मानम्());
  पटिनिवत्त L.intersectWith(R);
}

KnownBits KnownBits::umin(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](ध्रुव KnownBits &Val) { पटिनिवत्त KnownBits(Val.One, Val.Zero); };
  पटिनिवत्त Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::smax(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  पटिनिवत्त flipSignBit(umax(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::smin(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](ध्रुव KnownBits &Val) {
    unsigned SignBitPosition = Val.getBitWidth() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
    One.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
    पटिनिवत्त KnownBits(Zero, One);
  };
  पटिनिवत्त Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::abdu(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  सचे (LHS.get_Min_मानम्().uge(RHS.get_Max_मानम्()))
    पटिनिवत्त computeForAddSub(/*Add=*/मिच्छा, /*NSW=*/मिच्छा, /*NUW=*/मिच्छा, LHS,
                            RHS);
  सचे (RHS.get_Min_मानम्().uge(LHS.get_Max_मानम्()))
    पटिनिवत्त computeForAddSub(/*Add=*/मिच्छा, /*NSW=*/मिच्छा, /*NUW=*/मिच्छा, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/मिच्छा, /*NSW=*/मिच्छा, /*NUW=*/सच्च, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/मिच्छा, /*NSW=*/मिच्छा, /*NUW=*/सच्च, RHS, LHS);
  पटिनिवत्त Diff0.intersectWith(Diff1);
}

KnownBits KnownBits::abds(KnownBits LHS, KnownBits RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  सचे (LHS.get_Signed_Min_मानम्().sge(RHS.get_Signed_Max_मानम्()))
    पटिनिवत्त computeForAddSub(/*Add=*/मिच्छा, /*NSW=*/मिच्छा, /*NUW=*/मिच्छा, LHS,
                            RHS);
  सचे (RHS.get_Signed_Min_मानम्().sge(LHS.get_Signed_Max_मानम्()))
    पटिनिवत्त computeForAddSub(/*Add=*/मिच्छा, /*NSW=*/मिच्छा, /*NUW=*/मिच्छा, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  unsigned SignBitPosition = LHS.getBitWidth() - 1;
  अत्थाय (auto Arg : {&LHS, &RHS}) {
    तार्किक Tmp = Arg->Zero[SignBitPosition];
    Arg->Zero.setBitVal(SignBitPosition, Arg->One[SignBitPosition]);
    Arg->One.setBitVal(SignBitPosition, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/मिच्छा, /*NSW=*/मिच्छा, /*NUW=*/सच्च, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/मिच्छा, /*NSW=*/मिच्छा, /*NUW=*/सच्च, RHS, LHS);
  पटिनिवत्त Diff0.intersectWith(Diff1);
}

स्थिर unsigned getMaxShiftAmount(ध्रुव APInt &Max_मानम्, unsigned BitWidth) {
  सचे (isPowerOf2_32(BitWidth))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    पटिनिवत्त Max_मानम्.extract_Bits_As_ZExt_मानम्(
        std::min(Log2_32(BitWidth), Max_मानम्.getBitWidth()), 0);
  // This is only an approximate upper bound.
  पटिनिवत्त Max_मानम्.get_Limited_मानम्(BitWidth - 1);
}

KnownBits KnownBits::shl(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS, तार्किक NUW,
                         तार्किक NSW, तार्किक ShAmtNonZero) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ध्रुव KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known;
    तार्किक ShiftedOutZero, ShiftedOutOne;
    Known.Zero = LHS.Zero.ushl_ov(ShiftAmt, ShiftedOutZero);
    Known.Zero.set_नीचः_Bits(ShiftAmt);
    Known.One = LHS.One.ushl_ov(ShiftAmt, ShiftedOutOne);

    // All cases returning poison have been handled by MaxShiftAmount already.
    सचे (NSW) {
      सचे (NUW && ShiftAmt != 0)
        // NUW means we can assume anything shifted out was a zero.
        ShiftedOutZero = सच्च;

      सचे (ShiftedOutZero)
        Known.makeNonNegative();
      अञ्ञथा सचे (ShiftedOutOne)
        Known.makeNegative();
    }
    पटिनिवत्त Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_मानम्().get_Limited_मानम्(BitWidth);
  सचे (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  सचे (LHS.isUnknown()) {
    Known.Zero.set_नीचः_Bits(MinShiftAmount);
    सचे (NUW && NSW && MinShiftAmount != 0)
      Known.makeNonNegative();
    पटिनिवत्त Known;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_मानम् = RHS.get_Max_मानम्();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_मानम्, BitWidth);
  सचे (NUW && NSW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.गणनम्_Max_Leading_Zeros() - 1);
  सचे (NUW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.गणनम्_Max_Leading_Zeros());
  सचे (NSW)
    MaxShiftAmount = std::min(
        MaxShiftAmount,
        std::max(LHS.गणनम्_Max_Leading_Zeros(), LHS.गणनम्_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  सचे (MinShiftAmount == 0 && MaxShiftAmount == BitWidth - 1 &&
      isPowerOf2_32(BitWidth)) {
    Known.Zero.set_नीचः_Bits(LHS.गणनम्_Min_Trailing_Zeros());
    सचे (LHS.isAllOnes())
      Known.One.setSignBit();
    सचे (NSW) {
      सचे (LHS.isNonNegative())
        Known.makeNonNegative();
      सचे (LHS.isNegative())
        Known.makeNegative();
    }
    पटिनिवत्त Known;
  }

  // Find the common bits from all possible shifts.
  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_मानम्();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_मानम्();
  Known.setAllConflict();
  अत्थाय (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    सचे ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      अनुवर्त;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    सचे (Known.isUnknown())
      भङ्ग;
  }

  // All shift amounts may result in poison.
  सचे (Known.hasConflict())
    Known.setAllZero();
  पटिनिवत्त Known;
}

KnownBits KnownBits::lshr(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS,
                          तार्किक ShAmtNonZero, तार्किक Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ध्रुव KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known >>= ShiftAmt;
    // High bits are known zero.
    Known.Zero.set_उच्चः_Bits(ShiftAmt);
    पटिनिवत्त Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_मानम्().get_Limited_मानम्(BitWidth);
  सचे (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  सचे (LHS.isUnknown()) {
    Known.Zero.set_उच्चः_Bits(MinShiftAmount);
    पटिनिवत्त Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_मानम् = RHS.get_Max_मानम्();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_मानम्, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  सचे (Exact) {
    unsigned FirstOne = LHS.गणनम्_Max_Trailing_Zeros();
    सचे (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      पटिनिवत्त Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_मानम्();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_मानम्();
  Known.setAllConflict();
  अत्थाय (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    सचे ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      अनुवर्त;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    सचे (Known.isUnknown())
      भङ्ग;
  }

  // All shift amounts may result in poison.
  सचे (Known.hasConflict())
    Known.setAllZero();
  पटिनिवत्त Known;
}

KnownBits KnownBits::ashr(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS,
                          तार्किक ShAmtNonZero, तार्किक Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ध्रुव KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known.Zero.ashrInPlace(ShiftAmt);
    Known.One.ashrInPlace(ShiftAmt);
    पटिनिवत्त Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_मानम्().get_Limited_मानम्(BitWidth);
  सचे (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  सचे (LHS.isUnknown()) {
    सचे (MinShiftAmount == BitWidth) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      पटिनिवत्त Known;
    }
    पटिनिवत्त Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_मानम् = RHS.get_Max_मानम्();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_मानम्, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  सचे (Exact) {
    unsigned FirstOne = LHS.गणनम्_Max_Trailing_Zeros();
    सचे (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      पटिनिवत्त Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_मानम्();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_मानम्();
  Known.setAllConflict();
  अत्थाय (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
      ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    सचे ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      अनुवर्त;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    सचे (Known.isUnknown())
      भङ्ग;
  }

  // All shift amounts may result in poison.
  सचे (Known.hasConflict())
    Known.setAllZero();
  पटिनिवत्त Known;
}

KnownBits KnownBits::fshl(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS,
                          ध्रुव APInt &Amt) {
  पटिनिवत्त KnownBits(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::fshr(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS,
                          ध्रुव APInt &Amt) {
  पटिनिवत्त KnownBits(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::clmul(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  KnownBits Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_मानम्(), RHS.get_Min_मानम्()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  auto ClMulOr = [](ध्रुव APInt &LHS, ध्रुव APInt &RHS) {
    unsigned BW = LHS.getBitWidth();
    प्रतिज्ञा(BW == RHS.getBitWidth() && "Operand mismatch");
    APInt Result(BW, 0);
    अत्थाय (unsigned I :
         seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
      सचे (RHS[I])
        Result |= LHS << I;
    पटिनिवत्त Result;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt Known = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= Known;
  Res.One &= Known;

  पटिनिवत्त Res;
}

KnownBits KnownBits::pext(ध्रुव KnownBits &Val, ध्रुव KnownBits &Mask) {
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
  अत्थाय (unsigned I = 0; I < BitWidth; ++I) {
    सचे (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      सचे (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clearBits(M0, M1 + 1);
      सचे (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clearBits(M0, M1 + 1);
    }
    सचे (Mask.One[I])
      ++M0, ++M1;
    अञ्ञथा सचे (!Mask.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clearBits(M0, BitWidth);
  पटिनिवत्त Res;
}

KnownBits KnownBits::pdep(ध्रुव KnownBits &Val, ध्रुव KnownBits &Mask) {
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
  अत्थाय (unsigned I = 0; I < BitWidth; ++I) {
    सचे (!Mask.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clearBit(I);
    सचे (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::getBitsSet(BitWidth, M0, M1 + 1);
      सचे (!Range.isSubsetOf(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clearBit(I);
      सचे (!Range.isSubsetOf(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clearBit(I);
    }
    सचे (Mask.One[I])
      ++M0, ++M1;
    अञ्ञथा सचे (!Mask.Zero[I])
      ++M1;
  }
  पटिनिवत्त Res;
}

std::optional<तार्किक> KnownBits::eq(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  सचे (LHS.isConstant() && RHS.isConstant())
    पटिनिवत्त LHS.getConstant() == RHS.getConstant();
  सचे (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    पटिनिवत्त मिच्छा;
  पटिनिवत्त std::nullopt;
}

std::optional<तार्किक> KnownBits::ne(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  सचे (std::optional<तार्किक> KnownEQ = eq(LHS, RHS))
    पटिनिवत्त !*KnownEQ;
  पटिनिवत्त std::nullopt;
}

std::optional<तार्किक> KnownBits::ugt(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  सचे (LHS.get_Max_मानम्().ule(RHS.get_Min_मानम्()))
    पटिनिवत्त मिच्छा;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  सचे (LHS.get_Min_मानम्().ugt(RHS.get_Max_मानम्()))
    पटिनिवत्त सच्च;
  पटिनिवत्त std::nullopt;
}

std::optional<तार्किक> KnownBits::uge(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  सचे (std::optional<तार्किक> IsUGT = ugt(RHS, LHS))
    पटिनिवत्त !*IsUGT;
  पटिनिवत्त std::nullopt;
}

std::optional<तार्किक> KnownBits::ult(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  पटिनिवत्त ugt(RHS, LHS);
}

std::optional<तार्किक> KnownBits::ule(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  पटिनिवत्त uge(RHS, LHS);
}

std::optional<तार्किक> KnownBits::sgt(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  सचे (LHS.get_Signed_Max_मानम्().sle(RHS.get_Signed_Min_मानम्()))
    पटिनिवत्त मिच्छा;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  सचे (LHS.get_Signed_Min_मानम्().sgt(RHS.get_Signed_Max_मानम्()))
    पटिनिवत्त सच्च;
  पटिनिवत्त std::nullopt;
}

std::optional<तार्किक> KnownBits::sge(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  सचे (std::optional<तार्किक> KnownSGT = sgt(RHS, LHS))
    पटिनिवत्त !*KnownSGT;
  पटिनिवत्त std::nullopt;
}

std::optional<तार्किक> KnownBits::slt(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  पटिनिवत्त sgt(RHS, LHS);
}

std::optional<तार्किक> KnownBits::sle(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  पटिनिवत्त sge(RHS, LHS);
}

KnownBits KnownBits::abs(तार्किक IntMinIsPoison) ध्रुव {
  // If the source's MSB is zero then we know the rest of the bits already.
  सचे (isNonNegative())
    पटिनिवत्त *एतत्;

  // Absolute value preserves trailing zero count.
  KnownBits KnownAbs(getBitWidth());

  // If the input is negative, then abs(x) == -x.
  सचे (isNegative()) {
    KnownBits Tmp = *एतत्;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    सचे (IntMinIsPoison && (Zero.popcount() + 2) == getBitWidth())
      Tmp.One.setBit(गणनम्_Min_Trailing_Zeros());

    KnownAbs = computeForAddSub(
        /*Add*/ मिच्छा, IntMinIsPoison, /*NUW=*/मिच्छा,
        KnownBits::makeConstant(APInt(getBitWidth(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    सचे (IntMinIsPoison && Tmp.गणनम्_Min_Population() == 1 &&
        Tmp.गणनम्_Max_Population() != 1) {
      Tmp.One.clearSignBit();
      Tmp.Zero.setSignBit();
      KnownAbs.One.setBits(getBitWidth() - Tmp.गणनम्_Min_Leading_Zeros(),
                           getBitWidth() - 1);
    }

  } अञ्ञथा {
    unsigned MaxTZ = गणनम्_Max_Trailing_Zeros();
    unsigned MinTZ = गणनम्_Min_Trailing_Zeros();

    KnownAbs.Zero.set_नीचः_Bits(MinTZ);
    // If we know the lowest set 1, then preserve it.
    सचे (MaxTZ == MinTZ && MaxTZ < getBitWidth())
      KnownAbs.One.setBit(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    सचे (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_मानम्())) {
      KnownAbs.One.clearSignBit();
      KnownAbs.Zero.setSignBit();
    }
  }

  पटिनिवत्त KnownAbs;
}

KnownBits KnownBits::reduceAdd(unsigned अङ्कः_Elts) ध्रुव {
  सचे (अङ्कः_Elts == 0)
    पटिनिवत्त KnownBits(getBitWidth());

  unsigned BitWidth = getBitWidth();
  KnownBits Result(BitWidth);

  सचे (isConstant())
    // If all elements are the same constant, we can simply compute it
    पटिनिवत्त KnownBits::makeConstant(अङ्कः_Elts * getConstant());

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
  unsigned LostBits = Log2_32_Ceil(अङ्कः_Elts);

  सचे (isNonNegative()) {
    unsigned LeadingZeros = गणनम्_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > LostBits ? LeadingZeros - LostBits : 0;
    Result.Zero.set_उच्चः_Bits(LeadingZeros);
  } अञ्ञथा सचे (isNegative()) {
    unsigned LeadingOnes = गणनम्_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > LostBits ? LeadingOnes - LostBits : 0;
    Result.One.set_उच्चः_Bits(LeadingOnes);
  }

  पटिनिवत्त Result;
}

स्थिर KnownBits computeForSatAddSub(तार्किक Add, तार्किक Signed,
                                     ध्रुव KnownBits &LHS,
                                     ध्रुव KnownBits &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  unsigned BitWidth = LHS.getBitWidth();

  std::optional<तार्किक> Overflow;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  तार्किक MayNegClamp = सच्च;
  तार्किक MayPosClamp = सच्च;
  सचे (Signed) {
    // Easy cases we can rule out any overflow.
    सचे (Add && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      Overflow = मिच्छा;
    अञ्ञथा सचे (!Add && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      Overflow = मिच्छा;
    अञ्ञथा {
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
          KnownBits::computeForAddSub(Add, /*NSW=*/मिच्छा,
                                      /*NUW=*/मिच्छा, UnsignedLHS, UnsignedRHS);
      सचे (Add) {
        सचे (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = मिच्छा;
          // Pos + Pos will overflow with extra signbit.
          सचे (LHS.isNonNegative() && RHS.isNonNegative())
            Overflow = सच्च;
        } अञ्ञथा सचे (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = मिच्छा;
          // Neg + Neg will overflow without extra signbit.
          सचे (LHS.isNegative() && RHS.isNegative())
            Overflow = सच्च;
        }
        // We will never clamp to the opposite sign of N-bit result.
        सचे (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = मिच्छा;
        सचे (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = मिच्छा;
      } अञ्ञथा {
        सचे (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = मिच्छा;
          // Neg - Pos will overflow with extra signbit.
          सचे (LHS.isNegative() && RHS.isNonNegative())
            Overflow = सच्च;
        } अञ्ञथा सचे (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = मिच्छा;
          // Pos - Neg will overflow without extra signbit.
          सचे (LHS.isNonNegative() && RHS.isNegative())
            Overflow = सच्च;
        }
        // We will never clamp to the opposite sign of N-bit result.
        सचे (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = मिच्छा;
        सचे (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = मिच्छा;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    सचे (!MayNegClamp && !MayPosClamp)
      Overflow = मिच्छा;
  } अञ्ञथा सचे (Add) {
    // uadd.sat
    तार्किक Of;
    (शून्य)LHS.get_Max_मानम्().uadd_ov(RHS.get_Max_मानम्(), Of);
    सचे (!Of) {
      Overflow = मिच्छा;
    } अञ्ञथा {
      (शून्य)LHS.get_Min_मानम्().uadd_ov(RHS.get_Min_मानम्(), Of);
      सचे (Of)
        Overflow = सच्च;
    }
  } अञ्ञथा {
    // usub.sat
    तार्किक Of;
    (शून्य)LHS.get_Min_मानम्().usub_ov(RHS.get_Max_मानम्(), Of);
    सचे (!Of) {
      Overflow = मिच्छा;
    } अञ्ञथा {
      (शून्य)LHS.get_Max_मानम्().usub_ov(RHS.get_Min_मानम्(), Of);
      सचे (Of)
        Overflow = सच्च;
    }
  }

  KnownBits Res = KnownBits::computeForAddSub(Add, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  सचे (Overflow) {
    // We know whether or not we overflowed.
    सचे (!(*Overflow)) {
      // No overflow.
      पटिनिवत्त Res;
    }

    // We overflowed
    APInt C;
    सचे (Signed) {
      // sadd.sat / ssub.sat
      प्रतिज्ञा(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_मानम्(BitWidth)
                           : APInt::get_Signed_Max_मानम्(BitWidth);
    } अञ्ञथा सचे (Add) {
      // uadd.sat
      C = APInt::get_Max_मानम्(BitWidth);
    } अञ्ञथा {
      // uadd.sat
      C = APInt::get_Min_मानम्(BitWidth);
    }

    Res.One = C;
    Res.Zero = ~C;
    पटिनिवत्त Res;
  }

  // We don't know if we overflowed.
  सचे (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    सचे (MayPosClamp)
      Res.Zero.clear_नीचः_Bits(BitWidth - 1);
    सचे (MayNegClamp)
      Res.One.clear_नीचः_Bits(BitWidth - 1);
  } अञ्ञथा सचे (Add) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clearAllBits();
  } अञ्ञथा {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clearAllBits();
  }

  पटिनिवत्त Res;
}

KnownBits KnownBits::sadd_sat(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  पटिनिवत्त computeForSatAddSub(/*Add*/ सच्च, /*Signed*/ सच्च, LHS, RHS);
}
KnownBits KnownBits::ssub_sat(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  पटिनिवत्त computeForSatAddSub(/*Add*/ मिच्छा, /*Signed*/ सच्च, LHS, RHS);
}
KnownBits KnownBits::uadd_sat(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  पटिनिवत्त computeForSatAddSub(/*Add*/ सच्च, /*Signed*/ मिच्छा, LHS, RHS);
}
KnownBits KnownBits::usub_sat(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  पटिनिवत्त computeForSatAddSub(/*Add*/ मिच्छा, /*Signed*/ मिच्छा, LHS, RHS);
}

स्थिर KnownBits avgComputeU(KnownBits LHS, KnownBits RHS, तार्किक IsCeil) {
  unsigned BitWidth = LHS.getBitWidth();
  LHS = LHS.zext(BitWidth + 1);
  RHS = RHS.zext(BitWidth + 1);
  LHS =
      computeForAddCarry(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.extractBits(BitWidth, 1);
  पटिनिवत्त LHS;
}

KnownBits KnownBits::avgFloorS(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  पटिनिवत्त flipSignBit(avgFloorU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgFloorU(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  पटिनिवत्त avgComputeU(LHS, RHS, /*IsCeil=*/मिच्छा);
}

KnownBits KnownBits::avgCeilS(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  पटिनिवत्त flipSignBit(avgCeilU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgCeilU(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  पटिनिवत्त avgComputeU(LHS, RHS, /*IsCeil=*/सच्च);
}

KnownBits KnownBits::mul(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS,
                         तार्किक NoUndefSelfMultiply) {
  unsigned BitWidth = LHS.getBitWidth();
  प्रतिज्ञा(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  प्रतिज्ञा((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.get_Max_मानम्();
  APInt UMaxRHS = RHS.get_Max_मानम्();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  तार्किक HasOverflow;
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
  unsigned TrailZeroLHS = LHS.गणनम्_Min_Trailing_Zeros();
  unsigned TrailZeroRHS = RHS.गणनम्_Min_Trailing_Zeros();
  unsigned TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  unsigned SmallestOperand = std::min(TrailBitsKnownLHS - TrailZeroLHS,
                                      TrailBitsKnownRHS - TrailZeroRHS);
  unsigned ResultBitsKnown = std::min(SmallestOperand + TrailZ, BitWidth);

  // The lower ResultBitsKnown bits of this are known.
  APInt BottomKnown = LHS.One * RHS.One;

  KnownBits Res(BitWidth);
  Res.Zero.set_उच्चः_Bits(LeadZ);
  Res.Zero |= (~BottomKnown).getLoBits(ResultBitsKnown);
  Res.One = BottomKnown.getLoBits(ResultBitsKnown);

  सचे (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    unsigned TwoTZP1 = 2 * TrailZeroLHS + 1;
    सचे (TwoTZP1 < BitWidth)
      Res.Zero.setBit(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    सचे (TrailZeroLHS < BitWidth && LHS.One[TrailZeroLHS]) {
      unsigned TwoTZP2 = TwoTZP1 + 1;
      सचे (TwoTZP2 < BitWidth)
        Res.Zero.setBit(TwoTZP2);
    }
  }

  पटिनिवत्त Res;
}

KnownBits KnownBits::mulhs(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  प्रतिज्ञा(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.sext(2 * BitWidth);
  KnownBits WideRHS = RHS.sext(2 * BitWidth);
  पटिनिवत्त mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

KnownBits KnownBits::mulhu(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  प्रतिज्ञा(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.zext(2 * BitWidth);
  KnownBits WideRHS = RHS.zext(2 * BitWidth);
  पटिनिवत्त mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

स्थिर KnownBits div_Compute_नीचः_Bit(KnownBits Known, ध्रुव KnownBits &LHS,
                                  ध्रुव KnownBits &RHS, तार्किक Exact) {

  सचे (!Exact)
    पटिनिवत्त Known;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  सचे (LHS.One[0])
    Known.One.setBit(0);

  पूर्णाङ्क MinTZ =
      (पूर्णाङ्क)LHS.गणनम्_Min_Trailing_Zeros() - (पूर्णाङ्क)RHS.गणनम्_Max_Trailing_Zeros();
  पूर्णाङ्क MaxTZ =
      (पूर्णाङ्क)LHS.गणनम्_Max_Trailing_Zeros() - (पूर्णाङ्क)RHS.गणनम्_Min_Trailing_Zeros();
  सचे (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    Known.Zero.set_नीचः_Bits(MinTZ);
    सचे (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      Known.One.setBit(MinTZ);
    }
  } अञ्ञथा सचे (MaxTZ < 0) {
    // Poison Result
    Known.setAllZero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  सचे (Known.hasConflict())
    Known.setAllZero();

  पटिनिवत्त Known;
}

KnownBits KnownBits::sdiv(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS,
                          तार्किक Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  सचे (LHS.isNonNegative() && RHS.isNonNegative())
    पटिनिवत्त udiv(LHS, RHS, Exact);

  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  सचे (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    पटिनिवत्त Known;
  }

  std::optional<APInt> Res;
  सचे (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_मानम्();
    APInt अङ्कः = LHS.get_Signed_Min_मानम्();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (अङ्कः.is_Min_Signed_मानम्() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_मानम्(BitWidth)
              : अङ्कः.sdiv(Denom);
  } अञ्ञथा सचे (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    सचे (Exact || (-LHS.get_Signed_Max_मानम्()).uge(RHS.get_Signed_Max_मानम्())) {
      APInt Denom = RHS.get_Signed_Min_मानम्();
      APInt अङ्कः = LHS.get_Signed_Min_मानम्();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      सचे (Denom.isZero())
        Denom.setBit(RHS.गणनम्_Min_Trailing_Zeros());
      Res = अङ्कः.sdiv(Denom);
    }
  } अञ्ञथा सचे (LHS.isStrictlyPositive() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    सचे (Exact || LHS.get_Signed_Min_मानम्().uge(-RHS.get_Signed_Min_मानम्())) {
      APInt Denom = RHS.get_Signed_Max_मानम्();
      APInt अङ्कः = LHS.get_Signed_Max_मानम्();
      Res = अङ्कः.sdiv(Denom);
    }
  }

  सचे (Res) {
    सचे (Res->isNonNegative()) {
      unsigned LeadZ = Res->गणनम्_Leading_Zeros();
      Known.Zero.set_उच्चः_Bits(LeadZ);
    } अञ्ञथा {
      unsigned LeadO = Res->गणनम्_Leading_Ones();
      Known.One.set_उच्चः_Bits(LeadO);
    }
  }

  Known = div_Compute_नीचः_Bit(Known, LHS, RHS, Exact);
  पटिनिवत्त Known;
}

KnownBits KnownBits::udiv(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS,
                          तार्किक Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  सचे (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    पटिनिवत्त Known;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_मानम्();
  सचे (MinDenom.isZero())
    MinDenom.setBit(RHS.गणनम्_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_मानम्().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_मानम्().udiv(RHS.get_Max_मानम्());

  Known = KnownBits::makeConstant(MinRes);
  सचे (std::optional<unsigned> DifferentBit =
          APIntOps::GetMostSignificantDifferentBit(MinRes, MaxRes)) {
    Known.Zero.clear_नीचः_Bits(*DifferentBit + 1);
    Known.One.clear_नीचः_Bits(*DifferentBit + 1);
  }

  Known = div_Compute_नीचः_Bit(Known, LHS, RHS, Exact);

  पटिनिवत्त Known;
}

KnownBits KnownBits::rem_Get_नीचः_Bits(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  सचे (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    unsigned RHSZeros = RHS.गणनम्_Min_Trailing_Zeros();
    APInt Mask = APInt::get_नीचः_Bits_Set(BitWidth, RHSZeros);
    APInt OnesMask = LHS.One & Mask;
    APInt ZerosMask = LHS.Zero & Mask;
    पटिनिवत्त KnownBits(ZerosMask, OnesMask);
  }
  पटिनिवत्त KnownBits(BitWidth);
}

KnownBits KnownBits::urem(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  सचे (LHS.get_Max_मानम्().ult(RHS.get_Min_मानम्()))
    पटिनिवत्त LHS;

  KnownBits Known = rem_Get_नीचः_Bits(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_मानम्(), RHS.get_Max_मानम्() - 1);
  Known.Zero.set_उच्चः_Bits(MaxRes.गणनम्_Leading_Zeros());
  पटिनिवत्त Known;
}

KnownBits KnownBits::srem(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  KnownBits Known = rem_Get_नीचः_Bits(LHS, RHS);
  सचे (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt नीचः_Bits = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    सचे (LHS.isNonNegative() || नीचः_Bits.isSubsetOf(LHS.Zero))
      Known.Zero |= ~नीचः_Bits;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    सचे (LHS.isNegative() && नीचः_Bits.intersects(LHS.One))
      Known.One |= ~नीचः_Bits;
    पटिनिवत्त Known;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  सचे (LHS.isNegative() && Known.isNonZero())
    Known.One.set_उच्चः_Bits(
        std::max(LHS.गणनम्_Min_Leading_Ones(), RHS.गणनम्_Min_Sign_Bits()));
  अञ्ञथा सचे (LHS.isNonNegative())
    Known.Zero.set_उच्चः_Bits(
        std::max(LHS.गणनम्_Min_Leading_Zeros(), RHS.गणनम्_Min_Sign_Bits()));
  पटिनिवत्त Known;
}

KnownBits &KnownBits::operator&=(ध्रुव KnownBits &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  पटिनिवत्त *एतत्;
}

KnownBits &KnownBits::operator|=(ध्रुव KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  पटिनिवत्त *एतत्;
}

KnownBits &KnownBits::operator^=(ध्रुव KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  पटिनिवत्त *एतत्;
}

KnownBits KnownBits::blsi() ध्रुव {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(Zero, APInt(BitWidth, 0));
  unsigned Max = गणनम्_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = गणनम्_Min_Trailing_Zeros();
  सचे (Max == Min && Max < BitWidth)
    Known.One.setBit(Max);
  पटिनिवत्त Known;
}

KnownBits KnownBits::blsmsk() ध्रुव {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(BitWidth);
  unsigned Max = गणनम्_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = गणनम्_Min_Trailing_Zeros();
  Known.One.set_नीचः_Bits(std::min(Min + 1, BitWidth));
  पटिनिवत्त Known;
}

शून्य KnownBits::print(raw_ostream &OS) ध्रुव {
  unsigned BitWidth = getBitWidth();
  अत्थाय (unsigned I = 0; I < BitWidth; ++I) {
    unsigned N = BitWidth - I - 1;
    सचे (Zero[N] && One[N])
      OS << "!";
    अञ्ञथा सचे (Zero[N])
      OS << "0";
    अञ्ञथा सचे (One[N])
      OS << "1";
    अञ्ञथा
      OS << "?";
  }
}

#सचे !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य KnownBits::dump() ध्रुव {
  print(dbgs());
  dbgs() << "\n";
}
#endif

unsigned llvm::SignBitsOps::rot(unsigned SrcSignBits, unsigned BitWidth,
                                std::optional<APInt> RotAmt,
                                तार्किक Is_Rotate_दक्षिणः) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  सचे (SrcSignBits == BitWidth)
    पटिनिवत्त BitWidth;

  सचे (!RotAmt)
    पटिनिवत्त 1;

  unsigned Amt = RotAmt->urem(BitWidth);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  सचे (Is_Rotate_दक्षिणः)
    Amt = (BitWidth - Amt) % BitWidth;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  सचे (SrcSignBits > Amt + 1)
    पटिनिवत्त SrcSignBits - Amt;
  पटिनिवत्त 1;
}
