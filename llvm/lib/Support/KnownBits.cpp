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

KnownBits KnownBits::flipSignBit(स्थिरांक KnownBits &Val) {
  अचिन्हित SignBitPosition = Val.getBitWidth() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
  One.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
  परत KnownBits(Zero, One);
}

स्थिर KnownBits computeForAddCarry(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS,
                                    तार्किक CarryZero, तार्किक CarryOne) {

  APInt PossibleSumZero = LHS.get_Max_मूल्य() + RHS.get_Max_मूल्य() + !CarryZero;
  APInt PossibleSumOne = LHS.get_Min_मूल्य() + RHS.get_Min_मूल्य() + CarryOne;

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
  परत KnownOut;
}

KnownBits KnownBits::computeForAddCarry(
    स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS, स्थिरांक KnownBits &Carry) {
  प्रतिज्ञा(Carry.getBitWidth() == 1 && "Carry must be 1-bit");
  परत ::computeForAddCarry(
      LHS, RHS, Carry.Zero.get_Bool_मूल्य(), Carry.One.get_Bool_मूल्य());
}

KnownBits KnownBits::computeForAddSub(तार्किक Add, तार्किक NSW, तार्किक NUW,
                                      स्थिरांक KnownBits &LHS,
                                      स्थिरांक KnownBits &RHS) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  KnownBits KnownOut(BitWidth);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  जर (LHS.isUnknown() && RHS.isUnknown())
    परत KnownOut;

  जर (!LHS.isUnknown() && !RHS.isUnknown()) {
    जर (Add) {
      // Sum = LHS + RHS + 0
      KnownOut = ::computeForAddCarry(LHS, RHS, /*CarryZero=*/सत्य,
                                      /*CarryOne=*/असत्य);
    } अन्यथा {
      // Sum = LHS + ~RHS + 1
      KnownBits NotRHS = RHS;
      std::swap(NotRHS.Zero, NotRHS.One);
      KnownOut = ::computeForAddCarry(LHS, NotRHS, /*CarryZero=*/असत्य,
                                      /*CarryOne=*/सत्य);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  जर (NUW) {
    जर (Add) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_मूल्य().uadd_sat(RHS.get_Min_मूल्य());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      जर (NSW) {
        अचिन्हित अंक_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        KnownOut.One.setBits(BitWidth - 1 - अंक_Bits_2, BitWidth - 1);
      }
      KnownOut.One.set_वरचा_Bits(MinVal.countl_one());
    } अन्यथा {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_मूल्य().usub_sat(RHS.get_Min_मूल्य());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      जर (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        अचिन्हित अंक_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
        KnownOut.Zero.setBits(BitWidth - 1 - अंक_Bits_2, BitWidth - 1);
      }
      KnownOut.Zero.set_वरचा_Bits(MaxVal.countl_zero());
    }
  }

  जर (NSW) {
    APInt MinVal;
    APInt MaxVal;
    जर (Add) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_मूल्य().sadd_sat(RHS.get_Signed_Min_मूल्य());
      MaxVal = LHS.get_Signed_Max_मूल्य().sadd_sat(RHS.get_Signed_Max_मूल्य());
    } अन्यथा {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_मूल्य().ssub_sat(RHS.get_Signed_Max_मूल्य());
      MaxVal = LHS.get_Signed_Max_मूल्य().ssub_sat(RHS.get_Signed_Min_मूल्य());
    }
    जर (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      अचिन्हित अंक_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
      KnownOut.One.setBits(BitWidth - 1 - अंक_Bits_2, BitWidth - 1);
      KnownOut.Zero.setSignBit();
    }
    जर (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      अचिन्हित अंक_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
      KnownOut.Zero.setBits(BitWidth - 1 - अंक_Bits_2, BitWidth - 1);
      KnownOut.One.setSignBit();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  जर (KnownOut.hasConflict())
    KnownOut.setAllZero();
  परत KnownOut;
}

KnownBits KnownBits::computeForSubBorrow(स्थिरांक KnownBits &LHS, KnownBits RHS,
                                         स्थिरांक KnownBits &Borrow) {
  प्रतिज्ञा(Borrow.getBitWidth() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::swap(RHS.Zero, RHS.One);
  परत ::computeForAddCarry(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_मूल्य(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_मूल्य());
}

KnownBits KnownBits::truncSSat(अचिन्हित BitWidth) स्थिरांक {
  अचिन्हित आदान_Bits = getBitWidth();
  APInt MinInRange = APInt::get_Signed_Min_मूल्य(BitWidth).sext(आदान_Bits);
  APInt MaxInRange = APInt::get_Signed_Max_मूल्य(BitWidth).sext(आदान_Bits);
  APInt आदान_Min = get_Signed_Min_मूल्य();
  APInt आदान_Max = get_Signed_Max_मूल्य();
  KnownBits Known(BitWidth);

  // Case 1: All values fit - just truncate
  जर (आदान_Min.sge(MinInRange) && आदान_Max.sle(MaxInRange)) {
    Known = trunc(BitWidth);
  }
  // Case 2: All saturate to min
  अन्यथा जर (आदान_Max.slt(MinInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Min_मूल्य(BitWidth));
  }
  // Case 3: All saturate to max
  अन्यथा जर (आदान_Min.sgt(MaxInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Max_मूल्य(BitWidth));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  अन्यथा जर (आदान_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    Known.Zero = APInt(BitWidth, 0);
    Known.Zero.setBit(BitWidth - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    Known.One = One.trunc(BitWidth);
    Known.One.clearBit(BitWidth - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  अन्यथा जर (आदान_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    Known.Zero = Zero.trunc(BitWidth);
    Known.Zero.clearBit(BitWidth - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    Known.One = APInt(BitWidth, 0);
    Known.One.setBit(BitWidth - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  अन्यथा {
    // Output: min saturation, truncated, or max saturation
    APInt आदान_Zero_Lower = Zero.trunc(BitWidth);
    APInt आदान_One_Lower = One.trunc(BitWidth);
    APInt MinSat = APInt::get_Signed_Min_मूल्य(BitWidth);
    APInt MaxSat = APInt::get_Signed_Max_मूल्य(BitWidth);
    KnownBits MinSatKB = KnownBits::makeConstant(MinSat);
    KnownBits MaxSatKB = KnownBits::makeConstant(MaxSat);
    जर (आदान_Max.sle(MaxInRange)) {
      // Positive values fit, only negatives saturate to min
      Known.Zero = आदान_Zero_Lower & MinSatKB.Zero;
      Known.One = आदान_One_Lower & MinSatKB.One;
    } अन्यथा जर (आदान_Min.sge(MinInRange)) {
      // Negative values fit, only positives saturate to max
      Known.Zero = आदान_Zero_Lower & MaxSatKB.Zero;
      Known.One = आदान_One_Lower & MaxSatKB.One;
    } अन्यथा {
      // Both positive and negative values might saturate
      Known.Zero = आदान_Zero_Lower & MinSatKB.Zero & MaxSatKB.Zero;
      Known.One = आदान_One_Lower & MinSatKB.One & MaxSatKB.One;
    }
  }
  परत Known;
}

KnownBits KnownBits::truncSSatU(अचिन्हित BitWidth) स्थिरांक {
  अचिन्हित आदान_Bits = getBitWidth();
  APInt MaxInRange = APInt::getAllOnes(BitWidth).zext(आदान_Bits);
  APInt आदान_Min = get_Signed_Min_मूल्य();
  APInt आदान_Max = get_Signed_Max_मूल्य();
  KnownBits Known(BitWidth);

  जर (isNegative()) {
    Known.setAllZero();
  } अन्यथा जर (आदान_Min.isNonNegative() && आदान_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } अन्यथा जर (आदान_Min.isNonNegative() && आदान_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } अन्यथा जर (आदान_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    Known.Zero = APInt(BitWidth, 0);
    // Common one bits: bits that are 1 in input stay 1
    Known.One = One.trunc(BitWidth);
  } अन्यथा {
    // Mixed: sign bit unknown
    जर (आदान_Max.ule(MaxInRange)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      Known.Zero = Zero.trunc(BitWidth);
      Known.One = APInt(BitWidth, 0);
    } अन्यथा {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      Known.Zero = APInt(BitWidth, 0);
      Known.One = APInt(BitWidth, 0);
    }
  }
  परत Known;
}

KnownBits KnownBits::truncUSat(अचिन्हित BitWidth) स्थिरांक {
  अचिन्हित आदान_Bits = getBitWidth();
  APInt MaxInRange = APInt::get_खालचा_Bits_Set(आदान_Bits, BitWidth);
  APInt आदान_Max = get_Max_मूल्य();
  APInt आदान_Min = get_Min_मूल्य();
  KnownBits Known(BitWidth);

  जर (आदान_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } अन्यथा जर (आदान_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } अन्यथा {
    Known.resetAll();
    Known.One = One.trunc(BitWidth);
  }
  परत Known;
}

KnownBits KnownBits::sextInReg(अचिन्हित SrcBitWidth) स्थिरांक {
  अचिन्हित BitWidth = getBitWidth();
  प्रतिज्ञा(0 < SrcBitWidth && SrcBitWidth <= BitWidth &&
         "Illegal sext-in-register");

  जर (SrcBitWidth == BitWidth)
    परत *हे;

  अचिन्हित ExtBits = BitWidth - SrcBitWidth;
  KnownBits Result;
  Result.One = One << ExtBits;
  Result.Zero = Zero << ExtBits;
  Result.One.ashrInPlace(ExtBits);
  Result.Zero.ashrInPlace(ExtBits);
  परत Result;
}

KnownBits KnownBits::makeGE(स्थिरांक APInt &Val) स्थिरांक {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  अचिन्हित N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_खालचा_Bits(getBitWidth() - N);
  परत KnownBits(Zero, One | MaskedVal);
}

KnownBits KnownBits::umax(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  जर (LHS.get_Min_मूल्य().uge(RHS.get_Max_मूल्य()))
    परत LHS;
  जर (RHS.get_Min_मूल्य().uge(LHS.get_Max_मूल्य()))
    परत RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  KnownBits L = LHS.makeGE(RHS.get_Min_मूल्य());
  KnownBits R = RHS.makeGE(LHS.get_Min_मूल्य());
  परत L.intersectWith(R);
}

KnownBits KnownBits::umin(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  स्वतः Flip = [](स्थिरांक KnownBits &Val) { परत KnownBits(Val.One, Val.Zero); };
  परत Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::smax(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  परत flipSignBit(umax(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::smin(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  स्वतः Flip = [](स्थिरांक KnownBits &Val) {
    अचिन्हित SignBitPosition = Val.getBitWidth() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
    One.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
    परत KnownBits(Zero, One);
  };
  परत Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::abdu(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  जर (LHS.get_Min_मूल्य().uge(RHS.get_Max_मूल्य()))
    परत computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/असत्य, LHS,
                            RHS);
  जर (RHS.get_Min_मूल्य().uge(LHS.get_Max_मूल्य()))
    परत computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/असत्य, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/सत्य, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/सत्य, RHS, LHS);
  परत Diff0.intersectWith(Diff1);
}

KnownBits KnownBits::abds(KnownBits LHS, KnownBits RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  जर (LHS.get_Signed_Min_मूल्य().sge(RHS.get_Signed_Max_मूल्य()))
    परत computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/असत्य, LHS,
                            RHS);
  जर (RHS.get_Signed_Min_मूल्य().sge(LHS.get_Signed_Max_मूल्य()))
    परत computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/असत्य, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  अचिन्हित SignBitPosition = LHS.getBitWidth() - 1;
  साठी (स्वतः Arg : {&LHS, &RHS}) {
    तार्किक Tmp = Arg->Zero[SignBitPosition];
    Arg->Zero.setBitVal(SignBitPosition, Arg->One[SignBitPosition]);
    Arg->One.setBitVal(SignBitPosition, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/सत्य, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/सत्य, RHS, LHS);
  परत Diff0.intersectWith(Diff1);
}

स्थिर अचिन्हित getMaxShiftAmount(स्थिरांक APInt &Max_मूल्य, अचिन्हित BitWidth) {
  जर (isPowerOf2_32(BitWidth))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    परत Max_मूल्य.extract_Bits_As_ZExt_मूल्य(
        std::min(Log2_32(BitWidth), Max_मूल्य.getBitWidth()), 0);
  // This is only an approximate upper bound.
  परत Max_मूल्य.get_Limited_मूल्य(BitWidth - 1);
}

KnownBits KnownBits::shl(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS, तार्किक NUW,
                         तार्किक NSW, तार्किक ShAmtNonZero) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  स्वतः ShiftByConst = [&](स्थिरांक KnownBits &LHS, अचिन्हित ShiftAmt) {
    KnownBits Known;
    तार्किक ShiftedOutZero, ShiftedOutOne;
    Known.Zero = LHS.Zero.ushl_ov(ShiftAmt, ShiftedOutZero);
    Known.Zero.set_खालचा_Bits(ShiftAmt);
    Known.One = LHS.One.ushl_ov(ShiftAmt, ShiftedOutOne);

    // All cases returning poison have been handled by MaxShiftAmount already.
    जर (NSW) {
      जर (NUW && ShiftAmt != 0)
        // NUW means we can assume anything shifted out was a zero.
        ShiftedOutZero = सत्य;

      जर (ShiftedOutZero)
        Known.makeNonNegative();
      अन्यथा जर (ShiftedOutOne)
        Known.makeNegative();
    }
    परत Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  अचिन्हित MinShiftAmount = RHS.get_Min_मूल्य().get_Limited_मूल्य(BitWidth);
  जर (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  जर (LHS.isUnknown()) {
    Known.Zero.set_खालचा_Bits(MinShiftAmount);
    जर (NUW && NSW && MinShiftAmount != 0)
      Known.makeNonNegative();
    परत Known;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_मूल्य = RHS.get_Max_मूल्य();
  अचिन्हित MaxShiftAmount = getMaxShiftAmount(Max_मूल्य, BitWidth);
  जर (NUW && NSW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.मोजणी_Max_Leading_Zeros() - 1);
  जर (NUW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.मोजणी_Max_Leading_Zeros());
  जर (NSW)
    MaxShiftAmount = std::min(
        MaxShiftAmount,
        std::max(LHS.मोजणी_Max_Leading_Zeros(), LHS.मोजणी_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  जर (MinShiftAmount == 0 && MaxShiftAmount == BitWidth - 1 &&
      isPowerOf2_32(BitWidth)) {
    Known.Zero.set_खालचा_Bits(LHS.मोजणी_Min_Trailing_Zeros());
    जर (LHS.isAllOnes())
      Known.One.setSignBit();
    जर (NSW) {
      जर (LHS.isNonNegative())
        Known.makeNonNegative();
      जर (LHS.isNegative())
        Known.makeNegative();
    }
    परत Known;
  }

  // Find the common bits from all possible shifts.
  अचिन्हित ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_मूल्य();
  अचिन्हित ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_मूल्य();
  Known.setAllConflict();
  साठी (अचिन्हित ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    जर ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      सुरूठेवा;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    जर (Known.isUnknown())
      थांबा;
  }

  // All shift amounts may result in poison.
  जर (Known.hasConflict())
    Known.setAllZero();
  परत Known;
}

KnownBits KnownBits::lshr(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS,
                          तार्किक ShAmtNonZero, तार्किक Exact) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  स्वतः ShiftByConst = [&](स्थिरांक KnownBits &LHS, अचिन्हित ShiftAmt) {
    KnownBits Known = LHS;
    Known >>= ShiftAmt;
    // High bits are known zero.
    Known.Zero.set_वरचा_Bits(ShiftAmt);
    परत Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  अचिन्हित MinShiftAmount = RHS.get_Min_मूल्य().get_Limited_मूल्य(BitWidth);
  जर (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  जर (LHS.isUnknown()) {
    Known.Zero.set_वरचा_Bits(MinShiftAmount);
    परत Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_मूल्य = RHS.get_Max_मूल्य();
  अचिन्हित MaxShiftAmount = getMaxShiftAmount(Max_मूल्य, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  जर (Exact) {
    अचिन्हित FirstOne = LHS.मोजणी_Max_Trailing_Zeros();
    जर (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      परत Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  अचिन्हित ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_मूल्य();
  अचिन्हित ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_मूल्य();
  Known.setAllConflict();
  साठी (अचिन्हित ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    जर ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      सुरूठेवा;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    जर (Known.isUnknown())
      थांबा;
  }

  // All shift amounts may result in poison.
  जर (Known.hasConflict())
    Known.setAllZero();
  परत Known;
}

KnownBits KnownBits::ashr(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS,
                          तार्किक ShAmtNonZero, तार्किक Exact) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  स्वतः ShiftByConst = [&](स्थिरांक KnownBits &LHS, अचिन्हित ShiftAmt) {
    KnownBits Known = LHS;
    Known.Zero.ashrInPlace(ShiftAmt);
    Known.One.ashrInPlace(ShiftAmt);
    परत Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  अचिन्हित MinShiftAmount = RHS.get_Min_मूल्य().get_Limited_मूल्य(BitWidth);
  जर (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  जर (LHS.isUnknown()) {
    जर (MinShiftAmount == BitWidth) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      परत Known;
    }
    परत Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_मूल्य = RHS.get_Max_मूल्य();
  अचिन्हित MaxShiftAmount = getMaxShiftAmount(Max_मूल्य, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  जर (Exact) {
    अचिन्हित FirstOne = LHS.मोजणी_Max_Trailing_Zeros();
    जर (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      परत Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  अचिन्हित ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_मूल्य();
  अचिन्हित ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_मूल्य();
  Known.setAllConflict();
  साठी (अचिन्हित ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
      ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    जर ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      सुरूठेवा;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    जर (Known.isUnknown())
      थांबा;
  }

  // All shift amounts may result in poison.
  जर (Known.hasConflict())
    Known.setAllZero();
  परत Known;
}

KnownBits KnownBits::fshl(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS,
                          स्थिरांक APInt &Amt) {
  परत KnownBits(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::fshr(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS,
                          स्थिरांक APInt &Amt) {
  परत KnownBits(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::clmul(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  KnownBits Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_मूल्य(), RHS.get_Min_मूल्य()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  स्वतः ClMulOr = [](स्थिरांक APInt &LHS, स्थिरांक APInt &RHS) {
    अचिन्हित BW = LHS.getBitWidth();
    प्रतिज्ञा(BW == RHS.getBitWidth() && "Operand mismatch");
    APInt Result(BW, 0);
    साठी (अचिन्हित I :
         seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
      जर (RHS[I])
        Result |= LHS << I;
    परत Result;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt Known = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= Known;
  Res.One &= Known;

  परत Res;
}

KnownBits KnownBits::pext(स्थिरांक KnownBits &Val, स्थिरांक KnownBits &Mask) {
  अचिन्हित BitWidth = Val.getBitWidth();
  KnownBits Res(BitWidth);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.setAllConflict();

  // For each source position I where Mask[I] could be 1, the output position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible output positions widens.
  अचिन्हित M0 = 0, M1 = 0;
  साठी (अचिन्हित I = 0; I < BitWidth; ++I) {
    जर (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      जर (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clearBits(M0, M1 + 1);
      जर (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clearBits(M0, M1 + 1);
    }
    जर (Mask.One[I])
      ++M0, ++M1;
    अन्यथा जर (!Mask.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clearBits(M0, BitWidth);
  परत Res;
}

KnownBits KnownBits::pdep(स्थिरांक KnownBits &Val, स्थिरांक KnownBits &Mask) {
  अचिन्हित BitWidth = Val.getBitWidth();
  KnownBits Res(BitWidth);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.setAllConflict();

  // For each output position I where Mask[I] could be 1, the source position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible source positions widens.
  अचिन्हित M0 = 0, M1 = 0;
  साठी (अचिन्हित I = 0; I < BitWidth; ++I) {
    जर (!Mask.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clearBit(I);
    जर (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::getBitsSet(BitWidth, M0, M1 + 1);
      जर (!Range.isSubsetOf(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clearBit(I);
      जर (!Range.isSubsetOf(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clearBit(I);
    }
    जर (Mask.One[I])
      ++M0, ++M1;
    अन्यथा जर (!Mask.Zero[I])
      ++M1;
  }
  परत Res;
}

std::optional<तार्किक> KnownBits::eq(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  जर (LHS.isConstant() && RHS.isConstant())
    परत LHS.getConstant() == RHS.getConstant();
  जर (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    परत असत्य;
  परत std::nullopt;
}

std::optional<तार्किक> KnownBits::ne(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  जर (std::optional<तार्किक> KnownEQ = eq(LHS, RHS))
    परत !*KnownEQ;
  परत std::nullopt;
}

std::optional<तार्किक> KnownBits::ugt(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  जर (LHS.get_Max_मूल्य().ule(RHS.get_Min_मूल्य()))
    परत असत्य;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  जर (LHS.get_Min_मूल्य().ugt(RHS.get_Max_मूल्य()))
    परत सत्य;
  परत std::nullopt;
}

std::optional<तार्किक> KnownBits::uge(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  जर (std::optional<तार्किक> IsUGT = ugt(RHS, LHS))
    परत !*IsUGT;
  परत std::nullopt;
}

std::optional<तार्किक> KnownBits::ult(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  परत ugt(RHS, LHS);
}

std::optional<तार्किक> KnownBits::ule(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  परत uge(RHS, LHS);
}

std::optional<तार्किक> KnownBits::sgt(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  जर (LHS.get_Signed_Max_मूल्य().sle(RHS.get_Signed_Min_मूल्य()))
    परत असत्य;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  जर (LHS.get_Signed_Min_मूल्य().sgt(RHS.get_Signed_Max_मूल्य()))
    परत सत्य;
  परत std::nullopt;
}

std::optional<तार्किक> KnownBits::sge(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  जर (std::optional<तार्किक> KnownSGT = sgt(RHS, LHS))
    परत !*KnownSGT;
  परत std::nullopt;
}

std::optional<तार्किक> KnownBits::slt(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  परत sgt(RHS, LHS);
}

std::optional<तार्किक> KnownBits::sle(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  परत sge(RHS, LHS);
}

KnownBits KnownBits::असल(तार्किक IntMinIsPoison) स्थिरांक {
  // If the source's MSB is zero then we know the rest of the bits already.
  जर (isNonNegative())
    परत *हे;

  // Absolute value preserves trailing zero count.
  KnownBits KnownAbs(getBitWidth());

  // If the input is negative, then abs(x) == -x.
  जर (isNegative()) {
    KnownBits Tmp = *हे;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    जर (IntMinIsPoison && (Zero.popcount() + 2) == getBitWidth())
      Tmp.One.setBit(मोजणी_Min_Trailing_Zeros());

    KnownAbs = computeForAddSub(
        /*Add*/ असत्य, IntMinIsPoison, /*NUW=*/असत्य,
        KnownBits::makeConstant(APInt(getBitWidth(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    जर (IntMinIsPoison && Tmp.मोजणी_Min_Population() == 1 &&
        Tmp.मोजणी_Max_Population() != 1) {
      Tmp.One.clearSignBit();
      Tmp.Zero.setSignBit();
      KnownAbs.One.setBits(getBitWidth() - Tmp.मोजणी_Min_Leading_Zeros(),
                           getBitWidth() - 1);
    }

  } अन्यथा {
    अचिन्हित MaxTZ = मोजणी_Max_Trailing_Zeros();
    अचिन्हित MinTZ = मोजणी_Min_Trailing_Zeros();

    KnownAbs.Zero.set_खालचा_Bits(MinTZ);
    // If we know the lowest set 1, then preserve it.
    जर (MaxTZ == MinTZ && MaxTZ < getBitWidth())
      KnownAbs.One.setBit(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    जर (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_मूल्य())) {
      KnownAbs.One.clearSignBit();
      KnownAbs.Zero.setSignBit();
    }
  }

  परत KnownAbs;
}

KnownBits KnownBits::reduceAdd(अचिन्हित अंक_Elts) स्थिरांक {
  जर (अंक_Elts == 0)
    परत KnownBits(getBitWidth());

  अचिन्हित BitWidth = getBitWidth();
  KnownBits Result(BitWidth);

  जर (isConstant())
    // If all elements are the same constant, we can simply compute it
    परत KnownBits::makeConstant(अंक_Elts * getConstant());

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
  अचिन्हित LostBits = Log2_32_Ceil(अंक_Elts);

  जर (isNonNegative()) {
    अचिन्हित LeadingZeros = मोजणी_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > LostBits ? LeadingZeros - LostBits : 0;
    Result.Zero.set_वरचा_Bits(LeadingZeros);
  } अन्यथा जर (isNegative()) {
    अचिन्हित LeadingOnes = मोजणी_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > LostBits ? LeadingOnes - LostBits : 0;
    Result.One.set_वरचा_Bits(LeadingOnes);
  }

  परत Result;
}

स्थिर KnownBits computeForSatAddSub(तार्किक Add, तार्किक Signed,
                                     स्थिरांक KnownBits &LHS,
                                     स्थिरांक KnownBits &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  अचिन्हित BitWidth = LHS.getBitWidth();

  std::optional<तार्किक> Overflow;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  तार्किक MayNegClamp = सत्य;
  तार्किक MayPosClamp = सत्य;
  जर (Signed) {
    // Easy cases we can rule out any overflow.
    जर (Add && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      Overflow = असत्य;
    अन्यथा जर (!Add && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      Overflow = असत्य;
    अन्यथा {
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
          KnownBits::computeForAddSub(Add, /*NSW=*/असत्य,
                                      /*NUW=*/असत्य, UnsignedLHS, UnsignedRHS);
      जर (Add) {
        जर (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = असत्य;
          // Pos + Pos will overflow with extra signbit.
          जर (LHS.isNonNegative() && RHS.isNonNegative())
            Overflow = सत्य;
        } अन्यथा जर (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = असत्य;
          // Neg + Neg will overflow without extra signbit.
          जर (LHS.isNegative() && RHS.isNegative())
            Overflow = सत्य;
        }
        // We will never clamp to the opposite sign of N-bit result.
        जर (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = असत्य;
        जर (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = असत्य;
      } अन्यथा {
        जर (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = असत्य;
          // Neg - Pos will overflow with extra signbit.
          जर (LHS.isNegative() && RHS.isNonNegative())
            Overflow = सत्य;
        } अन्यथा जर (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = असत्य;
          // Pos - Neg will overflow without extra signbit.
          जर (LHS.isNonNegative() && RHS.isNegative())
            Overflow = सत्य;
        }
        // We will never clamp to the opposite sign of N-bit result.
        जर (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = असत्य;
        जर (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = असत्य;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    जर (!MayNegClamp && !MayPosClamp)
      Overflow = असत्य;
  } अन्यथा जर (Add) {
    // uadd.sat
    तार्किक Of;
    (शून्य)LHS.get_Max_मूल्य().uadd_ov(RHS.get_Max_मूल्य(), Of);
    जर (!Of) {
      Overflow = असत्य;
    } अन्यथा {
      (शून्य)LHS.get_Min_मूल्य().uadd_ov(RHS.get_Min_मूल्य(), Of);
      जर (Of)
        Overflow = सत्य;
    }
  } अन्यथा {
    // usub.sat
    तार्किक Of;
    (शून्य)LHS.get_Min_मूल्य().usub_ov(RHS.get_Max_मूल्य(), Of);
    जर (!Of) {
      Overflow = असत्य;
    } अन्यथा {
      (शून्य)LHS.get_Max_मूल्य().usub_ov(RHS.get_Min_मूल्य(), Of);
      जर (Of)
        Overflow = सत्य;
    }
  }

  KnownBits Res = KnownBits::computeForAddSub(Add, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  जर (Overflow) {
    // We know whether or not we overflowed.
    जर (!(*Overflow)) {
      // No overflow.
      परत Res;
    }

    // We overflowed
    APInt C;
    जर (Signed) {
      // sadd.sat / ssub.sat
      प्रतिज्ञा(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_मूल्य(BitWidth)
                           : APInt::get_Signed_Max_मूल्य(BitWidth);
    } अन्यथा जर (Add) {
      // uadd.sat
      C = APInt::get_Max_मूल्य(BitWidth);
    } अन्यथा {
      // uadd.sat
      C = APInt::get_Min_मूल्य(BitWidth);
    }

    Res.One = C;
    Res.Zero = ~C;
    परत Res;
  }

  // We don't know if we overflowed.
  जर (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    जर (MayPosClamp)
      Res.Zero.clear_खालचा_Bits(BitWidth - 1);
    जर (MayNegClamp)
      Res.One.clear_खालचा_Bits(BitWidth - 1);
  } अन्यथा जर (Add) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clearAllBits();
  } अन्यथा {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clearAllBits();
  }

  परत Res;
}

KnownBits KnownBits::sadd_sat(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  परत computeForSatAddSub(/*Add*/ सत्य, /*Signed*/ सत्य, LHS, RHS);
}
KnownBits KnownBits::ssub_sat(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  परत computeForSatAddSub(/*Add*/ असत्य, /*Signed*/ सत्य, LHS, RHS);
}
KnownBits KnownBits::uadd_sat(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  परत computeForSatAddSub(/*Add*/ सत्य, /*Signed*/ असत्य, LHS, RHS);
}
KnownBits KnownBits::usub_sat(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  परत computeForSatAddSub(/*Add*/ असत्य, /*Signed*/ असत्य, LHS, RHS);
}

स्थिर KnownBits avgComputeU(KnownBits LHS, KnownBits RHS, तार्किक IsCeil) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  LHS = LHS.zext(BitWidth + 1);
  RHS = RHS.zext(BitWidth + 1);
  LHS =
      computeForAddCarry(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.extractBits(BitWidth, 1);
  परत LHS;
}

KnownBits KnownBits::avgFloorS(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  परत flipSignBit(avgFloorU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgFloorU(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  परत avgComputeU(LHS, RHS, /*IsCeil=*/असत्य);
}

KnownBits KnownBits::avgCeilS(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  परत flipSignBit(avgCeilU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgCeilU(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  परत avgComputeU(LHS, RHS, /*IsCeil=*/सत्य);
}

KnownBits KnownBits::mul(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS,
                         तार्किक NoUndefSelfMultiply) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  प्रतिज्ञा(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  प्रतिज्ञा((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.get_Max_मूल्य();
  APInt UMaxRHS = RHS.get_Max_मूल्य();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  तार्किक HasOverflow;
  APInt UMaxResult = UMaxLHS.umul_ov(UMaxRHS, HasOverflow);
  अचिन्हित LeadZ = HasOverflow ? 0 : UMaxResult.countl_zero();

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
  अचिन्हित TrailBitsKnownLHS = (LHS.Zero | LHS.One).countr_one();
  अचिन्हित TrailBitsKnownRHS = (RHS.Zero | RHS.One).countr_one();
  अचिन्हित TrailZeroLHS = LHS.मोजणी_Min_Trailing_Zeros();
  अचिन्हित TrailZeroRHS = RHS.मोजणी_Min_Trailing_Zeros();
  अचिन्हित TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  अचिन्हित SmallestOperand = std::min(TrailBitsKnownLHS - TrailZeroLHS,
                                      TrailBitsKnownRHS - TrailZeroRHS);
  अचिन्हित ResultBitsKnown = std::min(SmallestOperand + TrailZ, BitWidth);

  // The lower ResultBitsKnown bits of this are known.
  APInt BottomKnown = LHS.One * RHS.One;

  KnownBits Res(BitWidth);
  Res.Zero.set_वरचा_Bits(LeadZ);
  Res.Zero |= (~BottomKnown).getLoBits(ResultBitsKnown);
  Res.One = BottomKnown.getLoBits(ResultBitsKnown);

  जर (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    अचिन्हित TwoTZP1 = 2 * TrailZeroLHS + 1;
    जर (TwoTZP1 < BitWidth)
      Res.Zero.setBit(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    जर (TrailZeroLHS < BitWidth && LHS.One[TrailZeroLHS]) {
      अचिन्हित TwoTZP2 = TwoTZP1 + 1;
      जर (TwoTZP2 < BitWidth)
        Res.Zero.setBit(TwoTZP2);
    }
  }

  परत Res;
}

KnownBits KnownBits::mulhs(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  प्रतिज्ञा(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.sext(2 * BitWidth);
  KnownBits WideRHS = RHS.sext(2 * BitWidth);
  परत mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

KnownBits KnownBits::mulhu(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  प्रतिज्ञा(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.zext(2 * BitWidth);
  KnownBits WideRHS = RHS.zext(2 * BitWidth);
  परत mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

स्थिर KnownBits div_Compute_खालचा_Bit(KnownBits Known, स्थिरांक KnownBits &LHS,
                                  स्थिरांक KnownBits &RHS, तार्किक Exact) {

  जर (!Exact)
    परत Known;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  जर (LHS.One[0])
    Known.One.setBit(0);

  पूर्णांक MinTZ =
      (पूर्णांक)LHS.मोजणी_Min_Trailing_Zeros() - (पूर्णांक)RHS.मोजणी_Max_Trailing_Zeros();
  पूर्णांक MaxTZ =
      (पूर्णांक)LHS.मोजणी_Max_Trailing_Zeros() - (पूर्णांक)RHS.मोजणी_Min_Trailing_Zeros();
  जर (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    Known.Zero.set_खालचा_Bits(MinTZ);
    जर (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      Known.One.setBit(MinTZ);
    }
  } अन्यथा जर (MaxTZ < 0) {
    // Poison Result
    Known.setAllZero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  जर (Known.hasConflict())
    Known.setAllZero();

  परत Known;
}

KnownBits KnownBits::sdiv(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS,
                          तार्किक Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  जर (LHS.isNonNegative() && RHS.isNonNegative())
    परत udiv(LHS, RHS, Exact);

  अचिन्हित BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  जर (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    परत Known;
  }

  std::optional<APInt> Res;
  जर (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_मूल्य();
    APInt अंक = LHS.get_Signed_Min_मूल्य();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (अंक.is_Min_Signed_मूल्य() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_मूल्य(BitWidth)
              : अंक.sdiv(Denom);
  } अन्यथा जर (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    जर (Exact || (-LHS.get_Signed_Max_मूल्य()).uge(RHS.get_Signed_Max_मूल्य())) {
      APInt Denom = RHS.get_Signed_Min_मूल्य();
      APInt अंक = LHS.get_Signed_Min_मूल्य();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      जर (Denom.isZero())
        Denom.setBit(RHS.मोजणी_Min_Trailing_Zeros());
      Res = अंक.sdiv(Denom);
    }
  } अन्यथा जर (LHS.isStrictlyPositive() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    जर (Exact || LHS.get_Signed_Min_मूल्य().uge(-RHS.get_Signed_Min_मूल्य())) {
      APInt Denom = RHS.get_Signed_Max_मूल्य();
      APInt अंक = LHS.get_Signed_Max_मूल्य();
      Res = अंक.sdiv(Denom);
    }
  }

  जर (Res) {
    जर (Res->isNonNegative()) {
      अचिन्हित LeadZ = Res->मोजणी_Leading_Zeros();
      Known.Zero.set_वरचा_Bits(LeadZ);
    } अन्यथा {
      अचिन्हित LeadO = Res->मोजणी_Leading_Ones();
      Known.One.set_वरचा_Bits(LeadO);
    }
  }

  Known = div_Compute_खालचा_Bit(Known, LHS, RHS, Exact);
  परत Known;
}

KnownBits KnownBits::udiv(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS,
                          तार्किक Exact) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  जर (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    परत Known;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_मूल्य();
  जर (MinDenom.isZero())
    MinDenom.setBit(RHS.मोजणी_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_मूल्य().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_मूल्य().udiv(RHS.get_Max_मूल्य());

  Known = KnownBits::makeConstant(MinRes);
  जर (std::optional<अचिन्हित> DifferentBit =
          APIntOps::GetMostSignificantDifferentBit(MinRes, MaxRes)) {
    Known.Zero.clear_खालचा_Bits(*DifferentBit + 1);
    Known.One.clear_खालचा_Bits(*DifferentBit + 1);
  }

  Known = div_Compute_खालचा_Bit(Known, LHS, RHS, Exact);

  परत Known;
}

KnownBits KnownBits::rem_Get_खालचा_Bits(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  जर (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    अचिन्हित RHSZeros = RHS.मोजणी_Min_Trailing_Zeros();
    APInt Mask = APInt::get_खालचा_Bits_Set(BitWidth, RHSZeros);
    APInt OnesMask = LHS.One & Mask;
    APInt ZerosMask = LHS.Zero & Mask;
    परत KnownBits(ZerosMask, OnesMask);
  }
  परत KnownBits(BitWidth);
}

KnownBits KnownBits::urem(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  जर (LHS.get_Max_मूल्य().ult(RHS.get_Min_मूल्य()))
    परत LHS;

  KnownBits Known = rem_Get_खालचा_Bits(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_मूल्य(), RHS.get_Max_मूल्य() - 1);
  Known.Zero.set_वरचा_Bits(MaxRes.मोजणी_Leading_Zeros());
  परत Known;
}

KnownBits KnownBits::srem(स्थिरांक KnownBits &LHS, स्थिरांक KnownBits &RHS) {
  KnownBits Known = rem_Get_खालचा_Bits(LHS, RHS);
  जर (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt खालचा_Bits = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    जर (LHS.isNonNegative() || खालचा_Bits.isSubsetOf(LHS.Zero))
      Known.Zero |= ~खालचा_Bits;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    जर (LHS.isNegative() && खालचा_Bits.intersects(LHS.One))
      Known.One |= ~खालचा_Bits;
    परत Known;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  जर (LHS.isNegative() && Known.isNonZero())
    Known.One.set_वरचा_Bits(
        std::max(LHS.मोजणी_Min_Leading_Ones(), RHS.मोजणी_Min_Sign_Bits()));
  अन्यथा जर (LHS.isNonNegative())
    Known.Zero.set_वरचा_Bits(
        std::max(LHS.मोजणी_Min_Leading_Zeros(), RHS.मोजणी_Min_Sign_Bits()));
  परत Known;
}

KnownBits &KnownBits::चालक&=(स्थिरांक KnownBits &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  परत *हे;
}

KnownBits &KnownBits::चालक|=(स्थिरांक KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  परत *हे;
}

KnownBits &KnownBits::चालक^=(स्थिरांक KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  परत *हे;
}

KnownBits KnownBits::blsi() स्थिरांक {
  अचिन्हित BitWidth = getBitWidth();
  KnownBits Known(Zero, APInt(BitWidth, 0));
  अचिन्हित Max = मोजणी_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  अचिन्हित Min = मोजणी_Min_Trailing_Zeros();
  जर (Max == Min && Max < BitWidth)
    Known.One.setBit(Max);
  परत Known;
}

KnownBits KnownBits::blsmsk() स्थिरांक {
  अचिन्हित BitWidth = getBitWidth();
  KnownBits Known(BitWidth);
  अचिन्हित Max = मोजणी_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  अचिन्हित Min = मोजणी_Min_Trailing_Zeros();
  Known.One.set_खालचा_Bits(std::min(Min + 1, BitWidth));
  परत Known;
}

शून्य KnownBits::print(raw_ostream &OS) स्थिरांक {
  अचिन्हित BitWidth = getBitWidth();
  साठी (अचिन्हित I = 0; I < BitWidth; ++I) {
    अचिन्हित N = BitWidth - I - 1;
    जर (Zero[N] && One[N])
      OS << "!";
    अन्यथा जर (Zero[N])
      OS << "0";
    अन्यथा जर (One[N])
      OS << "1";
    अन्यथा
      OS << "?";
  }
}

#जर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य KnownBits::dump() स्थिरांक {
  print(dbgs());
  dbgs() << "\n";
}
#endif

अचिन्हित llvm::SignBitsOps::rot(अचिन्हित SrcSignBits, अचिन्हित BitWidth,
                                std::optional<APInt> RotAmt,
                                तार्किक Is_Rotate_उजवा) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  जर (SrcSignBits == BitWidth)
    परत BitWidth;

  जर (!RotAmt)
    परत 1;

  अचिन्हित Amt = RotAmt->urem(BitWidth);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  जर (Is_Rotate_उजवा)
    Amt = (BitWidth - Amt) % BitWidth;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  जर (SrcSignBits > Amt + 1)
    परत SrcSignBits - Amt;
  परत 1;
}
