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

KnownBits KnownBits::flipSignBit(अचर KnownBits &Val) {
  अचिन्हित SignBitPosition = Val.getBitWidth() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
  One.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
  फर्काऊ KnownBits(Zero, One);
}

स्थिर KnownBits computeForAddCarry(अचर KnownBits &LHS, अचर KnownBits &RHS,
                                    तार्किक CarryZero, तार्किक CarryOne) {

  APInt PossibleSumZero = LHS.get_Max_मान() + RHS.get_Max_मान() + !CarryZero;
  APInt PossibleSumOne = LHS.get_Min_मान() + RHS.get_Min_मान() + CarryOne;

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
  फर्काऊ KnownOut;
}

KnownBits KnownBits::computeForAddCarry(
    अचर KnownBits &LHS, अचर KnownBits &RHS, अचर KnownBits &Carry) {
  प्रतिज्ञा(Carry.getBitWidth() == 1 && "Carry must be 1-bit");
  फर्काऊ ::computeForAddCarry(
      LHS, RHS, Carry.Zero.get_Bool_मान(), Carry.One.get_Bool_मान());
}

KnownBits KnownBits::computeForAddSub(तार्किक Add, तार्किक NSW, तार्किक NUW,
                                      अचर KnownBits &LHS,
                                      अचर KnownBits &RHS) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  KnownBits KnownOut(BitWidth);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  यदि (LHS.isUnknown() && RHS.isUnknown())
    फर्काऊ KnownOut;

  यदि (!LHS.isUnknown() && !RHS.isUnknown()) {
    यदि (Add) {
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
  यदि (NUW) {
    यदि (Add) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_मान().uadd_sat(RHS.get_Min_मान());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      यदि (NSW) {
        अचिन्हित अङ्क_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        KnownOut.One.setBits(BitWidth - 1 - अङ्क_Bits_2, BitWidth - 1);
      }
      KnownOut.One.set_माथिल्लो_Bits(MinVal.countl_one());
    } अन्यथा {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_मान().usub_sat(RHS.get_Min_मान());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      यदि (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        अचिन्हित अङ्क_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
        KnownOut.Zero.setBits(BitWidth - 1 - अङ्क_Bits_2, BitWidth - 1);
      }
      KnownOut.Zero.set_माथिल्लो_Bits(MaxVal.countl_zero());
    }
  }

  यदि (NSW) {
    APInt MinVal;
    APInt MaxVal;
    यदि (Add) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_मान().sadd_sat(RHS.get_Signed_Min_मान());
      MaxVal = LHS.get_Signed_Max_मान().sadd_sat(RHS.get_Signed_Max_मान());
    } अन्यथा {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_मान().ssub_sat(RHS.get_Signed_Max_मान());
      MaxVal = LHS.get_Signed_Max_मान().ssub_sat(RHS.get_Signed_Min_मान());
    }
    यदि (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      अचिन्हित अङ्क_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
      KnownOut.One.setBits(BitWidth - 1 - अङ्क_Bits_2, BitWidth - 1);
      KnownOut.Zero.setSignBit();
    }
    यदि (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      अचिन्हित अङ्क_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
      KnownOut.Zero.setBits(BitWidth - 1 - अङ्क_Bits_2, BitWidth - 1);
      KnownOut.One.setSignBit();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  यदि (KnownOut.hasConflict())
    KnownOut.setAllZero();
  फर्काऊ KnownOut;
}

KnownBits KnownBits::computeForSubBorrow(अचर KnownBits &LHS, KnownBits RHS,
                                         अचर KnownBits &Borrow) {
  प्रतिज्ञा(Borrow.getBitWidth() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::swap(RHS.Zero, RHS.One);
  फर्काऊ ::computeForAddCarry(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_मान(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_मान());
}

KnownBits KnownBits::truncSSat(अचिन्हित BitWidth) अचर {
  अचिन्हित आगत_Bits = getBitWidth();
  APInt MinInRange = APInt::get_Signed_Min_मान(BitWidth).sext(आगत_Bits);
  APInt MaxInRange = APInt::get_Signed_Max_मान(BitWidth).sext(आगत_Bits);
  APInt आगत_Min = get_Signed_Min_मान();
  APInt आगत_Max = get_Signed_Max_मान();
  KnownBits Known(BitWidth);

  // Case 1: All values fit - just truncate
  यदि (आगत_Min.sge(MinInRange) && आगत_Max.sle(MaxInRange)) {
    Known = trunc(BitWidth);
  }
  // Case 2: All saturate to min
  अन्यथा यदि (आगत_Max.slt(MinInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Min_मान(BitWidth));
  }
  // Case 3: All saturate to max
  अन्यथा यदि (आगत_Min.sgt(MaxInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Max_मान(BitWidth));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  अन्यथा यदि (आगत_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    Known.Zero = APInt(BitWidth, 0);
    Known.Zero.setBit(BitWidth - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    Known.One = One.trunc(BitWidth);
    Known.One.clearBit(BitWidth - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  अन्यथा यदि (आगत_Max.isNegative()) {
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
    APInt आगत_Zero_Lower = Zero.trunc(BitWidth);
    APInt आगत_One_Lower = One.trunc(BitWidth);
    APInt MinSat = APInt::get_Signed_Min_मान(BitWidth);
    APInt MaxSat = APInt::get_Signed_Max_मान(BitWidth);
    KnownBits MinSatKB = KnownBits::makeConstant(MinSat);
    KnownBits MaxSatKB = KnownBits::makeConstant(MaxSat);
    यदि (आगत_Max.sle(MaxInRange)) {
      // Positive values fit, only negatives saturate to min
      Known.Zero = आगत_Zero_Lower & MinSatKB.Zero;
      Known.One = आगत_One_Lower & MinSatKB.One;
    } अन्यथा यदि (आगत_Min.sge(MinInRange)) {
      // Negative values fit, only positives saturate to max
      Known.Zero = आगत_Zero_Lower & MaxSatKB.Zero;
      Known.One = आगत_One_Lower & MaxSatKB.One;
    } अन्यथा {
      // Both positive and negative values might saturate
      Known.Zero = आगत_Zero_Lower & MinSatKB.Zero & MaxSatKB.Zero;
      Known.One = आगत_One_Lower & MinSatKB.One & MaxSatKB.One;
    }
  }
  फर्काऊ Known;
}

KnownBits KnownBits::truncSSatU(अचिन्हित BitWidth) अचर {
  अचिन्हित आगत_Bits = getBitWidth();
  APInt MaxInRange = APInt::getAllOnes(BitWidth).zext(आगत_Bits);
  APInt आगत_Min = get_Signed_Min_मान();
  APInt आगत_Max = get_Signed_Max_मान();
  KnownBits Known(BitWidth);

  यदि (isNegative()) {
    Known.setAllZero();
  } अन्यथा यदि (आगत_Min.isNonNegative() && आगत_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } अन्यथा यदि (आगत_Min.isNonNegative() && आगत_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } अन्यथा यदि (आगत_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    Known.Zero = APInt(BitWidth, 0);
    // Common one bits: bits that are 1 in input stay 1
    Known.One = One.trunc(BitWidth);
  } अन्यथा {
    // Mixed: sign bit unknown
    यदि (आगत_Max.ule(MaxInRange)) {
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
  फर्काऊ Known;
}

KnownBits KnownBits::truncUSat(अचिन्हित BitWidth) अचर {
  अचिन्हित आगत_Bits = getBitWidth();
  APInt MaxInRange = APInt::get_तल्लो_Bits_Set(आगत_Bits, BitWidth);
  APInt आगत_Max = get_Max_मान();
  APInt आगत_Min = get_Min_मान();
  KnownBits Known(BitWidth);

  यदि (आगत_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } अन्यथा यदि (आगत_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } अन्यथा {
    Known.resetAll();
    Known.One = One.trunc(BitWidth);
  }
  फर्काऊ Known;
}

KnownBits KnownBits::sextInReg(अचिन्हित SrcBitWidth) अचर {
  अचिन्हित BitWidth = getBitWidth();
  प्रतिज्ञा(0 < SrcBitWidth && SrcBitWidth <= BitWidth &&
         "Illegal sext-in-register");

  यदि (SrcBitWidth == BitWidth)
    फर्काऊ *एतत्;

  अचिन्हित ExtBits = BitWidth - SrcBitWidth;
  KnownBits Result;
  Result.One = One << ExtBits;
  Result.Zero = Zero << ExtBits;
  Result.One.ashrInPlace(ExtBits);
  Result.Zero.ashrInPlace(ExtBits);
  फर्काऊ Result;
}

KnownBits KnownBits::makeGE(अचर APInt &Val) अचर {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  अचिन्हित N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_तल्लो_Bits(getBitWidth() - N);
  फर्काऊ KnownBits(Zero, One | MaskedVal);
}

KnownBits KnownBits::umax(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  यदि (LHS.get_Min_मान().uge(RHS.get_Max_मान()))
    फर्काऊ LHS;
  यदि (RHS.get_Min_मान().uge(LHS.get_Max_मान()))
    फर्काऊ RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  KnownBits L = LHS.makeGE(RHS.get_Min_मान());
  KnownBits R = RHS.makeGE(LHS.get_Min_मान());
  फर्काऊ L.intersectWith(R);
}

KnownBits KnownBits::umin(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  स्वतः Flip = [](अचर KnownBits &Val) { फर्काऊ KnownBits(Val.One, Val.Zero); };
  फर्काऊ Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::smax(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  फर्काऊ flipSignBit(umax(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::smin(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  स्वतः Flip = [](अचर KnownBits &Val) {
    अचिन्हित SignBitPosition = Val.getBitWidth() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
    One.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
    फर्काऊ KnownBits(Zero, One);
  };
  फर्काऊ Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::abdu(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  यदि (LHS.get_Min_मान().uge(RHS.get_Max_मान()))
    फर्काऊ computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/असत्य, LHS,
                            RHS);
  यदि (RHS.get_Min_मान().uge(LHS.get_Max_मान()))
    फर्काऊ computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/असत्य, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/सत्य, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/सत्य, RHS, LHS);
  फर्काऊ Diff0.intersectWith(Diff1);
}

KnownBits KnownBits::abds(KnownBits LHS, KnownBits RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  यदि (LHS.get_Signed_Min_मान().sge(RHS.get_Signed_Max_मान()))
    फर्काऊ computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/असत्य, LHS,
                            RHS);
  यदि (RHS.get_Signed_Min_मान().sge(LHS.get_Signed_Max_मान()))
    फर्काऊ computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/असत्य, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  अचिन्हित SignBitPosition = LHS.getBitWidth() - 1;
  लागि (स्वतः Arg : {&LHS, &RHS}) {
    तार्किक Tmp = Arg->Zero[SignBitPosition];
    Arg->Zero.setBitVal(SignBitPosition, Arg->One[SignBitPosition]);
    Arg->One.setBitVal(SignBitPosition, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/सत्य, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/सत्य, RHS, LHS);
  फर्काऊ Diff0.intersectWith(Diff1);
}

स्थिर अचिन्हित getMaxShiftAmount(अचर APInt &Max_मान, अचिन्हित BitWidth) {
  यदि (isPowerOf2_32(BitWidth))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    फर्काऊ Max_मान.extract_Bits_As_ZExt_मान(
        std::min(Log2_32(BitWidth), Max_मान.getBitWidth()), 0);
  // This is only an approximate upper bound.
  फर्काऊ Max_मान.get_Limited_मान(BitWidth - 1);
}

KnownBits KnownBits::shl(अचर KnownBits &LHS, अचर KnownBits &RHS, तार्किक NUW,
                         तार्किक NSW, तार्किक ShAmtNonZero) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  स्वतः ShiftByConst = [&](अचर KnownBits &LHS, अचिन्हित ShiftAmt) {
    KnownBits Known;
    तार्किक ShiftedOutZero, ShiftedOutOne;
    Known.Zero = LHS.Zero.ushl_ov(ShiftAmt, ShiftedOutZero);
    Known.Zero.set_तल्लो_Bits(ShiftAmt);
    Known.One = LHS.One.ushl_ov(ShiftAmt, ShiftedOutOne);

    // All cases returning poison have been handled by MaxShiftAmount already.
    यदि (NSW) {
      यदि (NUW && ShiftAmt != 0)
        // NUW means we can assume anything shifted out was a zero.
        ShiftedOutZero = सत्य;

      यदि (ShiftedOutZero)
        Known.makeNonNegative();
      अन्यथा यदि (ShiftedOutOne)
        Known.makeNegative();
    }
    फर्काऊ Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  अचिन्हित MinShiftAmount = RHS.get_Min_मान().get_Limited_मान(BitWidth);
  यदि (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  यदि (LHS.isUnknown()) {
    Known.Zero.set_तल्लो_Bits(MinShiftAmount);
    यदि (NUW && NSW && MinShiftAmount != 0)
      Known.makeNonNegative();
    फर्काऊ Known;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_मान = RHS.get_Max_मान();
  अचिन्हित MaxShiftAmount = getMaxShiftAmount(Max_मान, BitWidth);
  यदि (NUW && NSW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.गन्ती_Max_Leading_Zeros() - 1);
  यदि (NUW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.गन्ती_Max_Leading_Zeros());
  यदि (NSW)
    MaxShiftAmount = std::min(
        MaxShiftAmount,
        std::max(LHS.गन्ती_Max_Leading_Zeros(), LHS.गन्ती_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  यदि (MinShiftAmount == 0 && MaxShiftAmount == BitWidth - 1 &&
      isPowerOf2_32(BitWidth)) {
    Known.Zero.set_तल्लो_Bits(LHS.गन्ती_Min_Trailing_Zeros());
    यदि (LHS.isAllOnes())
      Known.One.setSignBit();
    यदि (NSW) {
      यदि (LHS.isNonNegative())
        Known.makeNonNegative();
      यदि (LHS.isNegative())
        Known.makeNegative();
    }
    फर्काऊ Known;
  }

  // Find the common bits from all possible shifts.
  अचिन्हित ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_मान();
  अचिन्हित ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_मान();
  Known.setAllConflict();
  लागि (अचिन्हित ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    यदि ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      जारी;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    यदि (Known.isUnknown())
      रोक;
  }

  // All shift amounts may result in poison.
  यदि (Known.hasConflict())
    Known.setAllZero();
  फर्काऊ Known;
}

KnownBits KnownBits::lshr(अचर KnownBits &LHS, अचर KnownBits &RHS,
                          तार्किक ShAmtNonZero, तार्किक Exact) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  स्वतः ShiftByConst = [&](अचर KnownBits &LHS, अचिन्हित ShiftAmt) {
    KnownBits Known = LHS;
    Known >>= ShiftAmt;
    // High bits are known zero.
    Known.Zero.set_माथिल्लो_Bits(ShiftAmt);
    फर्काऊ Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  अचिन्हित MinShiftAmount = RHS.get_Min_मान().get_Limited_मान(BitWidth);
  यदि (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  यदि (LHS.isUnknown()) {
    Known.Zero.set_माथिल्लो_Bits(MinShiftAmount);
    फर्काऊ Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_मान = RHS.get_Max_मान();
  अचिन्हित MaxShiftAmount = getMaxShiftAmount(Max_मान, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  यदि (Exact) {
    अचिन्हित FirstOne = LHS.गन्ती_Max_Trailing_Zeros();
    यदि (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      फर्काऊ Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  अचिन्हित ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_मान();
  अचिन्हित ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_मान();
  Known.setAllConflict();
  लागि (अचिन्हित ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    यदि ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      जारी;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    यदि (Known.isUnknown())
      रोक;
  }

  // All shift amounts may result in poison.
  यदि (Known.hasConflict())
    Known.setAllZero();
  फर्काऊ Known;
}

KnownBits KnownBits::ashr(अचर KnownBits &LHS, अचर KnownBits &RHS,
                          तार्किक ShAmtNonZero, तार्किक Exact) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  स्वतः ShiftByConst = [&](अचर KnownBits &LHS, अचिन्हित ShiftAmt) {
    KnownBits Known = LHS;
    Known.Zero.ashrInPlace(ShiftAmt);
    Known.One.ashrInPlace(ShiftAmt);
    फर्काऊ Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  अचिन्हित MinShiftAmount = RHS.get_Min_मान().get_Limited_मान(BitWidth);
  यदि (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  यदि (LHS.isUnknown()) {
    यदि (MinShiftAmount == BitWidth) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      फर्काऊ Known;
    }
    फर्काऊ Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_मान = RHS.get_Max_मान();
  अचिन्हित MaxShiftAmount = getMaxShiftAmount(Max_मान, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  यदि (Exact) {
    अचिन्हित FirstOne = LHS.गन्ती_Max_Trailing_Zeros();
    यदि (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      फर्काऊ Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  अचिन्हित ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_मान();
  अचिन्हित ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_मान();
  Known.setAllConflict();
  लागि (अचिन्हित ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
      ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    यदि ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      जारी;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    यदि (Known.isUnknown())
      रोक;
  }

  // All shift amounts may result in poison.
  यदि (Known.hasConflict())
    Known.setAllZero();
  फर्काऊ Known;
}

KnownBits KnownBits::fshl(अचर KnownBits &LHS, अचर KnownBits &RHS,
                          अचर APInt &Amt) {
  फर्काऊ KnownBits(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::fshr(अचर KnownBits &LHS, अचर KnownBits &RHS,
                          अचर APInt &Amt) {
  फर्काऊ KnownBits(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::clmul(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  KnownBits Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_मान(), RHS.get_Min_मान()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  स्वतः ClMulOr = [](अचर APInt &LHS, अचर APInt &RHS) {
    अचिन्हित BW = LHS.getBitWidth();
    प्रतिज्ञा(BW == RHS.getBitWidth() && "Operand mismatch");
    APInt Result(BW, 0);
    लागि (अचिन्हित I :
         seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
      यदि (RHS[I])
        Result |= LHS << I;
    फर्काऊ Result;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt Known = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= Known;
  Res.One &= Known;

  फर्काऊ Res;
}

KnownBits KnownBits::pext(अचर KnownBits &Val, अचर KnownBits &Mask) {
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
  लागि (अचिन्हित I = 0; I < BitWidth; ++I) {
    यदि (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      यदि (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clearBits(M0, M1 + 1);
      यदि (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clearBits(M0, M1 + 1);
    }
    यदि (Mask.One[I])
      ++M0, ++M1;
    अन्यथा यदि (!Mask.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clearBits(M0, BitWidth);
  फर्काऊ Res;
}

KnownBits KnownBits::pdep(अचर KnownBits &Val, अचर KnownBits &Mask) {
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
  लागि (अचिन्हित I = 0; I < BitWidth; ++I) {
    यदि (!Mask.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clearBit(I);
    यदि (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::getBitsSet(BitWidth, M0, M1 + 1);
      यदि (!Range.isSubsetOf(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clearBit(I);
      यदि (!Range.isSubsetOf(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clearBit(I);
    }
    यदि (Mask.One[I])
      ++M0, ++M1;
    अन्यथा यदि (!Mask.Zero[I])
      ++M1;
  }
  फर्काऊ Res;
}

std::optional<तार्किक> KnownBits::eq(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  यदि (LHS.isConstant() && RHS.isConstant())
    फर्काऊ LHS.getConstant() == RHS.getConstant();
  यदि (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    फर्काऊ असत्य;
  फर्काऊ std::nullopt;
}

std::optional<तार्किक> KnownBits::ne(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  यदि (std::optional<तार्किक> KnownEQ = eq(LHS, RHS))
    फर्काऊ !*KnownEQ;
  फर्काऊ std::nullopt;
}

std::optional<तार्किक> KnownBits::ugt(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  यदि (LHS.get_Max_मान().ule(RHS.get_Min_मान()))
    फर्काऊ असत्य;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  यदि (LHS.get_Min_मान().ugt(RHS.get_Max_मान()))
    फर्काऊ सत्य;
  फर्काऊ std::nullopt;
}

std::optional<तार्किक> KnownBits::uge(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  यदि (std::optional<तार्किक> IsUGT = ugt(RHS, LHS))
    फर्काऊ !*IsUGT;
  फर्काऊ std::nullopt;
}

std::optional<तार्किक> KnownBits::ult(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  फर्काऊ ugt(RHS, LHS);
}

std::optional<तार्किक> KnownBits::ule(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  फर्काऊ uge(RHS, LHS);
}

std::optional<तार्किक> KnownBits::sgt(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  यदि (LHS.get_Signed_Max_मान().sle(RHS.get_Signed_Min_मान()))
    फर्काऊ असत्य;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  यदि (LHS.get_Signed_Min_मान().sgt(RHS.get_Signed_Max_मान()))
    फर्काऊ सत्य;
  फर्काऊ std::nullopt;
}

std::optional<तार्किक> KnownBits::sge(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  यदि (std::optional<तार्किक> KnownSGT = sgt(RHS, LHS))
    फर्काऊ !*KnownSGT;
  फर्काऊ std::nullopt;
}

std::optional<तार्किक> KnownBits::slt(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  फर्काऊ sgt(RHS, LHS);
}

std::optional<तार्किक> KnownBits::sle(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  फर्काऊ sge(RHS, LHS);
}

KnownBits KnownBits::असल(तार्किक IntMinIsPoison) अचर {
  // If the source's MSB is zero then we know the rest of the bits already.
  यदि (isNonNegative())
    फर्काऊ *एतत्;

  // Absolute value preserves trailing zero count.
  KnownBits KnownAbs(getBitWidth());

  // If the input is negative, then abs(x) == -x.
  यदि (isNegative()) {
    KnownBits Tmp = *एतत्;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    यदि (IntMinIsPoison && (Zero.popcount() + 2) == getBitWidth())
      Tmp.One.setBit(गन्ती_Min_Trailing_Zeros());

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
    यदि (IntMinIsPoison && Tmp.गन्ती_Min_Population() == 1 &&
        Tmp.गन्ती_Max_Population() != 1) {
      Tmp.One.clearSignBit();
      Tmp.Zero.setSignBit();
      KnownAbs.One.setBits(getBitWidth() - Tmp.गन्ती_Min_Leading_Zeros(),
                           getBitWidth() - 1);
    }

  } अन्यथा {
    अचिन्हित MaxTZ = गन्ती_Max_Trailing_Zeros();
    अचिन्हित MinTZ = गन्ती_Min_Trailing_Zeros();

    KnownAbs.Zero.set_तल्लो_Bits(MinTZ);
    // If we know the lowest set 1, then preserve it.
    यदि (MaxTZ == MinTZ && MaxTZ < getBitWidth())
      KnownAbs.One.setBit(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    यदि (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_मान())) {
      KnownAbs.One.clearSignBit();
      KnownAbs.Zero.setSignBit();
    }
  }

  फर्काऊ KnownAbs;
}

KnownBits KnownBits::reduceAdd(अचिन्हित अङ्क_Elts) अचर {
  यदि (अङ्क_Elts == 0)
    फर्काऊ KnownBits(getBitWidth());

  अचिन्हित BitWidth = getBitWidth();
  KnownBits Result(BitWidth);

  यदि (isConstant())
    // If all elements are the same constant, we can simply compute it
    फर्काऊ KnownBits::makeConstant(अङ्क_Elts * getConstant());

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
  अचिन्हित LostBits = Log2_32_Ceil(अङ्क_Elts);

  यदि (isNonNegative()) {
    अचिन्हित LeadingZeros = गन्ती_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > LostBits ? LeadingZeros - LostBits : 0;
    Result.Zero.set_माथिल्लो_Bits(LeadingZeros);
  } अन्यथा यदि (isNegative()) {
    अचिन्हित LeadingOnes = गन्ती_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > LostBits ? LeadingOnes - LostBits : 0;
    Result.One.set_माथिल्लो_Bits(LeadingOnes);
  }

  फर्काऊ Result;
}

स्थिर KnownBits computeForSatAddSub(तार्किक Add, तार्किक Signed,
                                     अचर KnownBits &LHS,
                                     अचर KnownBits &RHS) {
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
  यदि (Signed) {
    // Easy cases we can rule out any overflow.
    यदि (Add && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      Overflow = असत्य;
    अन्यथा यदि (!Add && (((LHS.isNegative() && RHS.isNegative()) ||
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
      यदि (Add) {
        यदि (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = असत्य;
          // Pos + Pos will overflow with extra signbit.
          यदि (LHS.isNonNegative() && RHS.isNonNegative())
            Overflow = सत्य;
        } अन्यथा यदि (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = असत्य;
          // Neg + Neg will overflow without extra signbit.
          यदि (LHS.isNegative() && RHS.isNegative())
            Overflow = सत्य;
        }
        // We will never clamp to the opposite sign of N-bit result.
        यदि (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = असत्य;
        यदि (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = असत्य;
      } अन्यथा {
        यदि (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = असत्य;
          // Neg - Pos will overflow with extra signbit.
          यदि (LHS.isNegative() && RHS.isNonNegative())
            Overflow = सत्य;
        } अन्यथा यदि (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = असत्य;
          // Pos - Neg will overflow without extra signbit.
          यदि (LHS.isNonNegative() && RHS.isNegative())
            Overflow = सत्य;
        }
        // We will never clamp to the opposite sign of N-bit result.
        यदि (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = असत्य;
        यदि (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = असत्य;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    यदि (!MayNegClamp && !MayPosClamp)
      Overflow = असत्य;
  } अन्यथा यदि (Add) {
    // uadd.sat
    तार्किक Of;
    (शून्य)LHS.get_Max_मान().uadd_ov(RHS.get_Max_मान(), Of);
    यदि (!Of) {
      Overflow = असत्य;
    } अन्यथा {
      (शून्य)LHS.get_Min_मान().uadd_ov(RHS.get_Min_मान(), Of);
      यदि (Of)
        Overflow = सत्य;
    }
  } अन्यथा {
    // usub.sat
    तार्किक Of;
    (शून्य)LHS.get_Min_मान().usub_ov(RHS.get_Max_मान(), Of);
    यदि (!Of) {
      Overflow = असत्य;
    } अन्यथा {
      (शून्य)LHS.get_Max_मान().usub_ov(RHS.get_Min_मान(), Of);
      यदि (Of)
        Overflow = सत्य;
    }
  }

  KnownBits Res = KnownBits::computeForAddSub(Add, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  यदि (Overflow) {
    // We know whether or not we overflowed.
    यदि (!(*Overflow)) {
      // No overflow.
      फर्काऊ Res;
    }

    // We overflowed
    APInt C;
    यदि (Signed) {
      // sadd.sat / ssub.sat
      प्रतिज्ञा(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_मान(BitWidth)
                           : APInt::get_Signed_Max_मान(BitWidth);
    } अन्यथा यदि (Add) {
      // uadd.sat
      C = APInt::get_Max_मान(BitWidth);
    } अन्यथा {
      // uadd.sat
      C = APInt::get_Min_मान(BitWidth);
    }

    Res.One = C;
    Res.Zero = ~C;
    फर्काऊ Res;
  }

  // We don't know if we overflowed.
  यदि (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    यदि (MayPosClamp)
      Res.Zero.clear_तल्लो_Bits(BitWidth - 1);
    यदि (MayNegClamp)
      Res.One.clear_तल्लो_Bits(BitWidth - 1);
  } अन्यथा यदि (Add) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clearAllBits();
  } अन्यथा {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clearAllBits();
  }

  फर्काऊ Res;
}

KnownBits KnownBits::sadd_sat(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  फर्काऊ computeForSatAddSub(/*Add*/ सत्य, /*Signed*/ सत्य, LHS, RHS);
}
KnownBits KnownBits::ssub_sat(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  फर्काऊ computeForSatAddSub(/*Add*/ असत्य, /*Signed*/ सत्य, LHS, RHS);
}
KnownBits KnownBits::uadd_sat(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  फर्काऊ computeForSatAddSub(/*Add*/ सत्य, /*Signed*/ असत्य, LHS, RHS);
}
KnownBits KnownBits::usub_sat(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  फर्काऊ computeForSatAddSub(/*Add*/ असत्य, /*Signed*/ असत्य, LHS, RHS);
}

स्थिर KnownBits avgComputeU(KnownBits LHS, KnownBits RHS, तार्किक IsCeil) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  LHS = LHS.zext(BitWidth + 1);
  RHS = RHS.zext(BitWidth + 1);
  LHS =
      computeForAddCarry(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.extractBits(BitWidth, 1);
  फर्काऊ LHS;
}

KnownBits KnownBits::avgFloorS(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  फर्काऊ flipSignBit(avgFloorU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgFloorU(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  फर्काऊ avgComputeU(LHS, RHS, /*IsCeil=*/असत्य);
}

KnownBits KnownBits::avgCeilS(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  फर्काऊ flipSignBit(avgCeilU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgCeilU(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  फर्काऊ avgComputeU(LHS, RHS, /*IsCeil=*/सत्य);
}

KnownBits KnownBits::mul(अचर KnownBits &LHS, अचर KnownBits &RHS,
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
  APInt UMaxLHS = LHS.get_Max_मान();
  APInt UMaxRHS = RHS.get_Max_मान();

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
  अचिन्हित TrailZeroLHS = LHS.गन्ती_Min_Trailing_Zeros();
  अचिन्हित TrailZeroRHS = RHS.गन्ती_Min_Trailing_Zeros();
  अचिन्हित TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  अचिन्हित SmallestOperand = std::min(TrailBitsKnownLHS - TrailZeroLHS,
                                      TrailBitsKnownRHS - TrailZeroRHS);
  अचिन्हित ResultBitsKnown = std::min(SmallestOperand + TrailZ, BitWidth);

  // The lower ResultBitsKnown bits of this are known.
  APInt BottomKnown = LHS.One * RHS.One;

  KnownBits Res(BitWidth);
  Res.Zero.set_माथिल्लो_Bits(LeadZ);
  Res.Zero |= (~BottomKnown).getLoBits(ResultBitsKnown);
  Res.One = BottomKnown.getLoBits(ResultBitsKnown);

  यदि (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    अचिन्हित TwoTZP1 = 2 * TrailZeroLHS + 1;
    यदि (TwoTZP1 < BitWidth)
      Res.Zero.setBit(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    यदि (TrailZeroLHS < BitWidth && LHS.One[TrailZeroLHS]) {
      अचिन्हित TwoTZP2 = TwoTZP1 + 1;
      यदि (TwoTZP2 < BitWidth)
        Res.Zero.setBit(TwoTZP2);
    }
  }

  फर्काऊ Res;
}

KnownBits KnownBits::mulhs(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  प्रतिज्ञा(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.sext(2 * BitWidth);
  KnownBits WideRHS = RHS.sext(2 * BitWidth);
  फर्काऊ mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

KnownBits KnownBits::mulhu(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  प्रतिज्ञा(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.zext(2 * BitWidth);
  KnownBits WideRHS = RHS.zext(2 * BitWidth);
  फर्काऊ mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

स्थिर KnownBits div_Compute_तल्लो_Bit(KnownBits Known, अचर KnownBits &LHS,
                                  अचर KnownBits &RHS, तार्किक Exact) {

  यदि (!Exact)
    फर्काऊ Known;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  यदि (LHS.One[0])
    Known.One.setBit(0);

  पूर्णाङ्क MinTZ =
      (पूर्णाङ्क)LHS.गन्ती_Min_Trailing_Zeros() - (पूर्णाङ्क)RHS.गन्ती_Max_Trailing_Zeros();
  पूर्णाङ्क MaxTZ =
      (पूर्णाङ्क)LHS.गन्ती_Max_Trailing_Zeros() - (पूर्णाङ्क)RHS.गन्ती_Min_Trailing_Zeros();
  यदि (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    Known.Zero.set_तल्लो_Bits(MinTZ);
    यदि (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      Known.One.setBit(MinTZ);
    }
  } अन्यथा यदि (MaxTZ < 0) {
    // Poison Result
    Known.setAllZero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  यदि (Known.hasConflict())
    Known.setAllZero();

  फर्काऊ Known;
}

KnownBits KnownBits::sdiv(अचर KnownBits &LHS, अचर KnownBits &RHS,
                          तार्किक Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  यदि (LHS.isNonNegative() && RHS.isNonNegative())
    फर्काऊ udiv(LHS, RHS, Exact);

  अचिन्हित BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  यदि (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    फर्काऊ Known;
  }

  std::optional<APInt> Res;
  यदि (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_मान();
    APInt अङ्क = LHS.get_Signed_Min_मान();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (अङ्क.is_Min_Signed_मान() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_मान(BitWidth)
              : अङ्क.sdiv(Denom);
  } अन्यथा यदि (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    यदि (Exact || (-LHS.get_Signed_Max_मान()).uge(RHS.get_Signed_Max_मान())) {
      APInt Denom = RHS.get_Signed_Min_मान();
      APInt अङ्क = LHS.get_Signed_Min_मान();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      यदि (Denom.isZero())
        Denom.setBit(RHS.गन्ती_Min_Trailing_Zeros());
      Res = अङ्क.sdiv(Denom);
    }
  } अन्यथा यदि (LHS.isStrictlyPositive() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    यदि (Exact || LHS.get_Signed_Min_मान().uge(-RHS.get_Signed_Min_मान())) {
      APInt Denom = RHS.get_Signed_Max_मान();
      APInt अङ्क = LHS.get_Signed_Max_मान();
      Res = अङ्क.sdiv(Denom);
    }
  }

  यदि (Res) {
    यदि (Res->isNonNegative()) {
      अचिन्हित LeadZ = Res->गन्ती_Leading_Zeros();
      Known.Zero.set_माथिल्लो_Bits(LeadZ);
    } अन्यथा {
      अचिन्हित LeadO = Res->गन्ती_Leading_Ones();
      Known.One.set_माथिल्लो_Bits(LeadO);
    }
  }

  Known = div_Compute_तल्लो_Bit(Known, LHS, RHS, Exact);
  फर्काऊ Known;
}

KnownBits KnownBits::udiv(अचर KnownBits &LHS, अचर KnownBits &RHS,
                          तार्किक Exact) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  यदि (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    फर्काऊ Known;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_मान();
  यदि (MinDenom.isZero())
    MinDenom.setBit(RHS.गन्ती_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_मान().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_मान().udiv(RHS.get_Max_मान());

  Known = KnownBits::makeConstant(MinRes);
  यदि (std::optional<अचिन्हित> DifferentBit =
          APIntOps::GetMostSignificantDifferentBit(MinRes, MaxRes)) {
    Known.Zero.clear_तल्लो_Bits(*DifferentBit + 1);
    Known.One.clear_तल्लो_Bits(*DifferentBit + 1);
  }

  Known = div_Compute_तल्लो_Bit(Known, LHS, RHS, Exact);

  फर्काऊ Known;
}

KnownBits KnownBits::rem_Get_तल्लो_Bits(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  यदि (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    अचिन्हित RHSZeros = RHS.गन्ती_Min_Trailing_Zeros();
    APInt Mask = APInt::get_तल्लो_Bits_Set(BitWidth, RHSZeros);
    APInt OnesMask = LHS.One & Mask;
    APInt ZerosMask = LHS.Zero & Mask;
    फर्काऊ KnownBits(ZerosMask, OnesMask);
  }
  फर्काऊ KnownBits(BitWidth);
}

KnownBits KnownBits::urem(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  यदि (LHS.get_Max_मान().ult(RHS.get_Min_मान()))
    फर्काऊ LHS;

  KnownBits Known = rem_Get_तल्लो_Bits(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_मान(), RHS.get_Max_मान() - 1);
  Known.Zero.set_माथिल्लो_Bits(MaxRes.गन्ती_Leading_Zeros());
  फर्काऊ Known;
}

KnownBits KnownBits::srem(अचर KnownBits &LHS, अचर KnownBits &RHS) {
  KnownBits Known = rem_Get_तल्लो_Bits(LHS, RHS);
  यदि (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt तल्लो_Bits = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    यदि (LHS.isNonNegative() || तल्लो_Bits.isSubsetOf(LHS.Zero))
      Known.Zero |= ~तल्लो_Bits;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    यदि (LHS.isNegative() && तल्लो_Bits.intersects(LHS.One))
      Known.One |= ~तल्लो_Bits;
    फर्काऊ Known;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  यदि (LHS.isNegative() && Known.isNonZero())
    Known.One.set_माथिल्लो_Bits(
        std::max(LHS.गन्ती_Min_Leading_Ones(), RHS.गन्ती_Min_Sign_Bits()));
  अन्यथा यदि (LHS.isNonNegative())
    Known.Zero.set_माथिल्लो_Bits(
        std::max(LHS.गन्ती_Min_Leading_Zeros(), RHS.गन्ती_Min_Sign_Bits()));
  फर्काऊ Known;
}

KnownBits &KnownBits::चालक&=(अचर KnownBits &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  फर्काऊ *एतत्;
}

KnownBits &KnownBits::चालक|=(अचर KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  फर्काऊ *एतत्;
}

KnownBits &KnownBits::चालक^=(अचर KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  फर्काऊ *एतत्;
}

KnownBits KnownBits::blsi() अचर {
  अचिन्हित BitWidth = getBitWidth();
  KnownBits Known(Zero, APInt(BitWidth, 0));
  अचिन्हित Max = गन्ती_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  अचिन्हित Min = गन्ती_Min_Trailing_Zeros();
  यदि (Max == Min && Max < BitWidth)
    Known.One.setBit(Max);
  फर्काऊ Known;
}

KnownBits KnownBits::blsmsk() अचर {
  अचिन्हित BitWidth = getBitWidth();
  KnownBits Known(BitWidth);
  अचिन्हित Max = गन्ती_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  अचिन्हित Min = गन्ती_Min_Trailing_Zeros();
  Known.One.set_तल्लो_Bits(std::min(Min + 1, BitWidth));
  फर्काऊ Known;
}

शून्य KnownBits::print(raw_ostream &OS) अचर {
  अचिन्हित BitWidth = getBitWidth();
  लागि (अचिन्हित I = 0; I < BitWidth; ++I) {
    अचिन्हित N = BitWidth - I - 1;
    यदि (Zero[N] && One[N])
      OS << "!";
    अन्यथा यदि (Zero[N])
      OS << "0";
    अन्यथा यदि (One[N])
      OS << "1";
    अन्यथा
      OS << "?";
  }
}

#यदि !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य KnownBits::dump() अचर {
  print(dbgs());
  dbgs() << "\n";
}
#endif

अचिन्हित llvm::SignBitsOps::rot(अचिन्हित SrcSignBits, अचिन्हित BitWidth,
                                std::optional<APInt> RotAmt,
                                तार्किक Is_Rotate_दायाँ) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  यदि (SrcSignBits == BitWidth)
    फर्काऊ BitWidth;

  यदि (!RotAmt)
    फर्काऊ 1;

  अचिन्हित Amt = RotAmt->urem(BitWidth);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  यदि (Is_Rotate_दायाँ)
    Amt = (BitWidth - Amt) % BitWidth;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  यदि (SrcSignBits > Amt + 1)
    फर्काऊ SrcSignBits - Amt;
  फर्काऊ 1;
}
