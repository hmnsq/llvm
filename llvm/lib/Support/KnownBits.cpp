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

#समावेश "llvm/Support/KnownBits.h"
#समावेश "llvm/ADT/Sequence.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <cassert>

using namespace llvm;

KnownBits KnownBits::flipSignBit(स्थिर KnownBits &Val) {
  अचिन्हित SignBitPosition = Val.getBitWidth() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
  One.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
  वापस KnownBits(Zero, One);
}

अटल KnownBits computeForAddCarry(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS,
                                    bool CarryZero, bool CarryOne) {

  APInt Possible_योग_Zero = LHS.get_Max_मूल्य() + RHS.get_Max_मूल्य() + !CarryZero;
  APInt Possible_योग_One = LHS.get_Min_मूल्य() + RHS.get_Min_मूल्य() + CarryOne;

  // Compute known bits of the carry.
  APInt CarryKnownZero = ~(Possible_योग_Zero ^ LHS.Zero ^ RHS.Zero);
  APInt CarryKnownOne = Possible_योग_One ^ LHS.One ^ RHS.One;

  // Compute set of known bits (where all three relevant bits are known).
  APInt LHSKnownUnion = LHS.Zero | LHS.One;
  APInt RHSKnownUnion = RHS.Zero | RHS.One;
  APInt CarryKnownUnion = std::move(CarryKnownZero) | CarryKnownOne;
  APInt Known = std::move(LHSKnownUnion) & RHSKnownUnion & CarryKnownUnion;

  // Compute known bits of the result.
  KnownBits KnownOut;
  KnownOut.Zero = ~std::move(Possible_योग_Zero) & Known;
  KnownOut.One = std::move(Possible_योग_One) & Known;
  वापस KnownOut;
}

KnownBits KnownBits::computeForAddCarry(
    स्थिर KnownBits &LHS, स्थिर KnownBits &RHS, स्थिर KnownBits &Carry) {
  निश्चित(Carry.getBitWidth() == 1 && "Carry must be 1-bit");
  वापस ::computeForAddCarry(
      LHS, RHS, Carry.Zero.get_Bool_मूल्य(), Carry.One.get_Bool_मूल्य());
}

KnownBits KnownBits::computeForAddSub(bool Add, bool NSW, bool NUW,
                                      स्थिर KnownBits &LHS,
                                      स्थिर KnownBits &RHS) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  KnownBits KnownOut(BitWidth);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  अगर (LHS.isUnknown() && RHS.isUnknown())
    वापस KnownOut;

  अगर (!LHS.isUnknown() && !RHS.isUnknown()) {
    अगर (Add) {
      // Sum = LHS + RHS + 0
      KnownOut = ::computeForAddCarry(LHS, RHS, /*CarryZero=*/true,
                                      /*CarryOne=*/false);
    } अन्यथा {
      // Sum = LHS + ~RHS + 1
      KnownBits NotRHS = RHS;
      std::swap(NotRHS.Zero, NotRHS.One);
      KnownOut = ::computeForAddCarry(LHS, NotRHS, /*CarryZero=*/false,
                                      /*CarryOne=*/true);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  अगर (NUW) {
    अगर (Add) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_मूल्य().uadd_sat(RHS.get_Min_मूल्य());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      अगर (NSW) {
        अचिन्हित अंक_Bits_2 = MinVal.छाँटो(BitWidth - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        KnownOut.One.setBits(BitWidth - 1 - अंक_Bits_2, BitWidth - 1);
      }
      KnownOut.One.set_ऊपरी_Bits(MinVal.countl_one());
    } अन्यथा {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_मूल्य().usub_sat(RHS.get_Min_मूल्य());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      अगर (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        अचिन्हित अंक_Bits_2 = MaxVal.छाँटो(BitWidth - 1).countl_zero();
        KnownOut.Zero.setBits(BitWidth - 1 - अंक_Bits_2, BitWidth - 1);
      }
      KnownOut.Zero.set_ऊपरी_Bits(MaxVal.countl_zero());
    }
  }

  अगर (NSW) {
    APInt MinVal;
    APInt MaxVal;
    अगर (Add) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_मूल्य().sadd_sat(RHS.get_Signed_Min_मूल्य());
      MaxVal = LHS.get_Signed_Max_मूल्य().sadd_sat(RHS.get_Signed_Max_मूल्य());
    } अन्यथा {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_मूल्य().ssub_sat(RHS.get_Signed_Max_मूल्य());
      MaxVal = LHS.get_Signed_Max_मूल्य().ssub_sat(RHS.get_Signed_Min_मूल्य());
    }
    अगर (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      अचिन्हित अंक_Bits_2 = MinVal.छाँटो(BitWidth - 1).countl_one();
      KnownOut.One.setBits(BitWidth - 1 - अंक_Bits_2, BitWidth - 1);
      KnownOut.Zero.setSignBit();
    }
    अगर (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      अचिन्हित अंक_Bits_2 = MaxVal.छाँटो(BitWidth - 1).countl_zero();
      KnownOut.Zero.setBits(BitWidth - 1 - अंक_Bits_2, BitWidth - 1);
      KnownOut.One.setSignBit();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  अगर (KnownOut.hasConflict())
    KnownOut.setAllZero();
  वापस KnownOut;
}

KnownBits KnownBits::computeForSubBorrow(स्थिर KnownBits &LHS, KnownBits RHS,
                                         स्थिर KnownBits &Borrow) {
  निश्चित(Borrow.getBitWidth() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::swap(RHS.Zero, RHS.One);
  वापस ::computeForAddCarry(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_मूल्य(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_मूल्य());
}

KnownBits KnownBits::truncSSat(अचिन्हित BitWidth) स्थिर {
  अचिन्हित निवेश_Bits = getBitWidth();
  APInt MinInRange = APInt::get_Signed_Min_मूल्य(BitWidth).sext(निवेश_Bits);
  APInt MaxInRange = APInt::get_Signed_Max_मूल्य(BitWidth).sext(निवेश_Bits);
  APInt निवेश_Min = get_Signed_Min_मूल्य();
  APInt निवेश_Max = get_Signed_Max_मूल्य();
  KnownBits Known(BitWidth);

  // Case 1: All values fit - just truncate
  अगर (निवेश_Min.sge(MinInRange) && निवेश_Max.sle(MaxInRange)) {
    Known = छाँटो(BitWidth);
  }
  // Case 2: All saturate to min
  अन्यथा अगर (निवेश_Max.slt(MinInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Min_मूल्य(BitWidth));
  }
  // Case 3: All saturate to max
  अन्यथा अगर (निवेश_Min.sgt(MaxInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Max_मूल्य(BitWidth));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  अन्यथा अगर (निवेश_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    Known.Zero = APInt(BitWidth, 0);
    Known.Zero.setBit(BitWidth - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    Known.One = One.छाँटो(BitWidth);
    Known.One.clearBit(BitWidth - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  अन्यथा अगर (निवेश_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    Known.Zero = Zero.छाँटो(BitWidth);
    Known.Zero.clearBit(BitWidth - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    Known.One = APInt(BitWidth, 0);
    Known.One.setBit(BitWidth - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  अन्यथा {
    // Output: min saturation, truncated, or max saturation
    APInt निवेश_Zero_Lower = Zero.छाँटो(BitWidth);
    APInt निवेश_One_Lower = One.छाँटो(BitWidth);
    APInt MinSat = APInt::get_Signed_Min_मूल्य(BitWidth);
    APInt MaxSat = APInt::get_Signed_Max_मूल्य(BitWidth);
    KnownBits MinSatKB = KnownBits::makeConstant(MinSat);
    KnownBits MaxSatKB = KnownBits::makeConstant(MaxSat);
    अगर (निवेश_Max.sle(MaxInRange)) {
      // Positive values fit, only negatives saturate to min
      Known.Zero = निवेश_Zero_Lower & MinSatKB.Zero;
      Known.One = निवेश_One_Lower & MinSatKB.One;
    } अन्यथा अगर (निवेश_Min.sge(MinInRange)) {
      // Negative values fit, only positives saturate to max
      Known.Zero = निवेश_Zero_Lower & MaxSatKB.Zero;
      Known.One = निवेश_One_Lower & MaxSatKB.One;
    } अन्यथा {
      // Both positive and negative values might saturate
      Known.Zero = निवेश_Zero_Lower & MinSatKB.Zero & MaxSatKB.Zero;
      Known.One = निवेश_One_Lower & MinSatKB.One & MaxSatKB.One;
    }
  }
  वापस Known;
}

KnownBits KnownBits::truncSSatU(अचिन्हित BitWidth) स्थिर {
  अचिन्हित निवेश_Bits = getBitWidth();
  APInt MaxInRange = APInt::getAllOnes(BitWidth).zext(निवेश_Bits);
  APInt निवेश_Min = get_Signed_Min_मूल्य();
  APInt निवेश_Max = get_Signed_Max_मूल्य();
  KnownBits Known(BitWidth);

  अगर (isNegative()) {
    Known.setAllZero();
  } अन्यथा अगर (निवेश_Min.isNonNegative() && निवेश_Max.ule(MaxInRange)) {
    Known = छाँटो(BitWidth);
  } अन्यथा अगर (निवेश_Min.isNonNegative() && निवेश_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } अन्यथा अगर (निवेश_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    Known.Zero = APInt(BitWidth, 0);
    // Common one bits: bits that are 1 in input stay 1
    Known.One = One.छाँटो(BitWidth);
  } अन्यथा {
    // Mixed: sign bit unknown
    अगर (निवेश_Max.ule(MaxInRange)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      Known.Zero = Zero.छाँटो(BitWidth);
      Known.One = APInt(BitWidth, 0);
    } अन्यथा {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      Known.Zero = APInt(BitWidth, 0);
      Known.One = APInt(BitWidth, 0);
    }
  }
  वापस Known;
}

KnownBits KnownBits::truncUSat(अचिन्हित BitWidth) स्थिर {
  अचिन्हित निवेश_Bits = getBitWidth();
  APInt MaxInRange = APInt::get_निचला_Bits_Set(निवेश_Bits, BitWidth);
  APInt निवेश_Max = get_Max_मूल्य();
  APInt निवेश_Min = get_Min_मूल्य();
  KnownBits Known(BitWidth);

  अगर (निवेश_Max.ule(MaxInRange)) {
    Known = छाँटो(BitWidth);
  } अन्यथा अगर (निवेश_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } अन्यथा {
    Known.resetAll();
    Known.One = One.छाँटो(BitWidth);
  }
  वापस Known;
}

KnownBits KnownBits::sextInReg(अचिन्हित SrcBitWidth) स्थिर {
  अचिन्हित BitWidth = getBitWidth();
  निश्चित(0 < SrcBitWidth && SrcBitWidth <= BitWidth &&
         "Illegal sext-in-register");

  अगर (SrcBitWidth == BitWidth)
    वापस *यह;

  अचिन्हित ExtBits = BitWidth - SrcBitWidth;
  KnownBits परिणाम;
  परिणाम.One = One << ExtBits;
  परिणाम.Zero = Zero << ExtBits;
  परिणाम.One.ashrInPlace(ExtBits);
  परिणाम.Zero.ashrInPlace(ExtBits);
  वापस परिणाम;
}

KnownBits KnownBits::makeGE(स्थिर APInt &Val) स्थिर {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  अचिन्हित N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_निचला_Bits(getBitWidth() - N);
  वापस KnownBits(Zero, One | MaskedVal);
}

KnownBits KnownBits::umax(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  अगर (LHS.get_Min_मूल्य().uge(RHS.get_Max_मूल्य()))
    वापस LHS;
  अगर (RHS.get_Min_मूल्य().uge(LHS.get_Max_मूल्य()))
    वापस RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  KnownBits L = LHS.makeGE(RHS.get_Min_मूल्य());
  KnownBits R = RHS.makeGE(LHS.get_Min_मूल्य());
  वापस L.intersectWith(R);
}

KnownBits KnownBits::umin(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  स्वतः Flip = [](स्थिर KnownBits &Val) { वापस KnownBits(Val.One, Val.Zero); };
  वापस Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::smax(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  वापस flipSignBit(umax(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::smin(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  स्वतः Flip = [](स्थिर KnownBits &Val) {
    अचिन्हित SignBitPosition = Val.getBitWidth() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
    One.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
    वापस KnownBits(Zero, One);
  };
  वापस Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::abdu(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  अगर (LHS.get_Min_मूल्य().uge(RHS.get_Max_मूल्य()))
    वापस computeForAddSub(/*Add=*/false, /*NSW=*/false, /*NUW=*/false, LHS,
                            RHS);
  अगर (RHS.get_Min_मूल्य().uge(LHS.get_Max_मूल्य()))
    वापस computeForAddSub(/*Add=*/false, /*NSW=*/false, /*NUW=*/false, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/false, /*NSW=*/false, /*NUW=*/true, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/false, /*NSW=*/false, /*NUW=*/true, RHS, LHS);
  वापस Diff0.intersectWith(Diff1);
}

KnownBits KnownBits::abds(KnownBits LHS, KnownBits RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  अगर (LHS.get_Signed_Min_मूल्य().sge(RHS.get_Signed_Max_मूल्य()))
    वापस computeForAddSub(/*Add=*/false, /*NSW=*/false, /*NUW=*/false, LHS,
                            RHS);
  अगर (RHS.get_Signed_Min_मूल्य().sge(LHS.get_Signed_Max_मूल्य()))
    वापस computeForAddSub(/*Add=*/false, /*NSW=*/false, /*NUW=*/false, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  अचिन्हित SignBitPosition = LHS.getBitWidth() - 1;
  क्रम (स्वतः Arg : {&LHS, &RHS}) {
    bool Tmp = Arg->Zero[SignBitPosition];
    Arg->Zero.setBitVal(SignBitPosition, Arg->One[SignBitPosition]);
    Arg->One.setBitVal(SignBitPosition, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/false, /*NSW=*/false, /*NUW=*/true, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/false, /*NSW=*/false, /*NUW=*/true, RHS, LHS);
  वापस Diff0.intersectWith(Diff1);
}

अटल अचिन्हित getMaxShiftAmount(स्थिर APInt &Max_मूल्य, अचिन्हित BitWidth) {
  अगर (isPowerOf2_32(BitWidth))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    वापस Max_मूल्य.extract_Bits_As_ZExt_मूल्य(
        std::min(Log2_32(BitWidth), Max_मूल्य.getBitWidth()), 0);
  // This is only an approximate upper bound.
  वापस Max_मूल्य.get_Limited_मूल्य(BitWidth - 1);
}

KnownBits KnownBits::shl(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS, bool NUW,
                         bool NSW, bool ShAmtNonZero) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  स्वतः ShiftByConst = [&](स्थिर KnownBits &LHS, अचिन्हित ShiftAmt) {
    KnownBits Known;
    bool ShiftedOutZero, ShiftedOutOne;
    Known.Zero = LHS.Zero.ushl_ov(ShiftAmt, ShiftedOutZero);
    Known.Zero.set_निचला_Bits(ShiftAmt);
    Known.One = LHS.One.ushl_ov(ShiftAmt, ShiftedOutOne);

    // All cases returning poison have been handled by MaxShiftAmount already.
    अगर (NSW) {
      अगर (NUW && ShiftAmt != 0)
        // NUW means we can assume anything shifted out was a zero.
        ShiftedOutZero = true;

      अगर (ShiftedOutZero)
        Known.makeNonNegative();
      अन्यथा अगर (ShiftedOutOne)
        Known.makeNegative();
    }
    वापस Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  अचिन्हित MinShiftAmount = RHS.get_Min_मूल्य().get_Limited_मूल्य(BitWidth);
  अगर (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  अगर (LHS.isUnknown()) {
    Known.Zero.set_निचला_Bits(MinShiftAmount);
    अगर (NUW && NSW && MinShiftAmount != 0)
      Known.makeNonNegative();
    वापस Known;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_मूल्य = RHS.get_Max_मूल्य();
  अचिन्हित MaxShiftAmount = getMaxShiftAmount(Max_मूल्य, BitWidth);
  अगर (NUW && NSW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.गिनती_Max_Leading_Zeros() - 1);
  अगर (NUW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.गिनती_Max_Leading_Zeros());
  अगर (NSW)
    MaxShiftAmount = std::min(
        MaxShiftAmount,
        std::max(LHS.गिनती_Max_Leading_Zeros(), LHS.गिनती_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  अगर (MinShiftAmount == 0 && MaxShiftAmount == BitWidth - 1 &&
      isPowerOf2_32(BitWidth)) {
    Known.Zero.set_निचला_Bits(LHS.गिनती_Min_Trailing_Zeros());
    अगर (LHS.isAllOnes())
      Known.One.setSignBit();
    अगर (NSW) {
      अगर (LHS.isNonNegative())
        Known.makeNonNegative();
      अगर (LHS.isNegative())
        Known.makeNegative();
    }
    वापस Known;
  }

  // Find the common bits from all possible shifts.
  अचिन्हित ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_मूल्य();
  अचिन्हित ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_मूल्य();
  Known.setAllConflict();
  क्रम (अचिन्हित ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    अगर ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      जारी;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    अगर (Known.isUnknown())
      अवरोध;
  }

  // All shift amounts may result in poison.
  अगर (Known.hasConflict())
    Known.setAllZero();
  वापस Known;
}

KnownBits KnownBits::lshr(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS,
                          bool ShAmtNonZero, bool Exact) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  स्वतः ShiftByConst = [&](स्थिर KnownBits &LHS, अचिन्हित ShiftAmt) {
    KnownBits Known = LHS;
    Known >>= ShiftAmt;
    // High bits are known zero.
    Known.Zero.set_ऊपरी_Bits(ShiftAmt);
    वापस Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  अचिन्हित MinShiftAmount = RHS.get_Min_मूल्य().get_Limited_मूल्य(BitWidth);
  अगर (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  अगर (LHS.isUnknown()) {
    Known.Zero.set_ऊपरी_Bits(MinShiftAmount);
    वापस Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_मूल्य = RHS.get_Max_मूल्य();
  अचिन्हित MaxShiftAmount = getMaxShiftAmount(Max_मूल्य, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  अगर (Exact) {
    अचिन्हित FirstOne = LHS.गिनती_Max_Trailing_Zeros();
    अगर (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      वापस Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  अचिन्हित ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_मूल्य();
  अचिन्हित ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_मूल्य();
  Known.setAllConflict();
  क्रम (अचिन्हित ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    अगर ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      जारी;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    अगर (Known.isUnknown())
      अवरोध;
  }

  // All shift amounts may result in poison.
  अगर (Known.hasConflict())
    Known.setAllZero();
  वापस Known;
}

KnownBits KnownBits::ashr(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS,
                          bool ShAmtNonZero, bool Exact) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  स्वतः ShiftByConst = [&](स्थिर KnownBits &LHS, अचिन्हित ShiftAmt) {
    KnownBits Known = LHS;
    Known.Zero.ashrInPlace(ShiftAmt);
    Known.One.ashrInPlace(ShiftAmt);
    वापस Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  अचिन्हित MinShiftAmount = RHS.get_Min_मूल्य().get_Limited_मूल्य(BitWidth);
  अगर (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  अगर (LHS.isUnknown()) {
    अगर (MinShiftAmount == BitWidth) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      वापस Known;
    }
    वापस Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_मूल्य = RHS.get_Max_मूल्य();
  अचिन्हित MaxShiftAmount = getMaxShiftAmount(Max_मूल्य, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  अगर (Exact) {
    अचिन्हित FirstOne = LHS.गिनती_Max_Trailing_Zeros();
    अगर (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      वापस Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  अचिन्हित ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_मूल्य();
  अचिन्हित ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_मूल्य();
  Known.setAllConflict();
  क्रम (अचिन्हित ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
      ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    अगर ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      जारी;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    अगर (Known.isUnknown())
      अवरोध;
  }

  // All shift amounts may result in poison.
  अगर (Known.hasConflict())
    Known.setAllZero();
  वापस Known;
}

KnownBits KnownBits::fshl(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS,
                          स्थिर APInt &Amt) {
  वापस KnownBits(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::fshr(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS,
                          स्थिर APInt &Amt) {
  वापस KnownBits(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::clmul(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  KnownBits Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_मूल्य(), RHS.get_Min_मूल्य()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  स्वतः ClMulOr = [](स्थिर APInt &LHS, स्थिर APInt &RHS) {
    अचिन्हित BW = LHS.getBitWidth();
    निश्चित(BW == RHS.getBitWidth() && "Operand mismatch");
    APInt परिणाम(BW, 0);
    क्रम (अचिन्हित I :
         seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
      अगर (RHS[I])
        परिणाम |= LHS << I;
    वापस परिणाम;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt Known = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= Known;
  Res.One &= Known;

  वापस Res;
}

KnownBits KnownBits::pext(स्थिर KnownBits &Val, स्थिर KnownBits &Mask) {
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
  क्रम (अचिन्हित I = 0; I < BitWidth; ++I) {
    अगर (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      अगर (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clearBits(M0, M1 + 1);
      अगर (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clearBits(M0, M1 + 1);
    }
    अगर (Mask.One[I])
      ++M0, ++M1;
    अन्यथा अगर (!Mask.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clearBits(M0, BitWidth);
  वापस Res;
}

KnownBits KnownBits::pdep(स्थिर KnownBits &Val, स्थिर KnownBits &Mask) {
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
  क्रम (अचिन्हित I = 0; I < BitWidth; ++I) {
    अगर (!Mask.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clearBit(I);
    अगर (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::getBitsSet(BitWidth, M0, M1 + 1);
      अगर (!Range.isSubsetOf(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clearBit(I);
      अगर (!Range.isSubsetOf(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clearBit(I);
    }
    अगर (Mask.One[I])
      ++M0, ++M1;
    अन्यथा अगर (!Mask.Zero[I])
      ++M1;
  }
  वापस Res;
}

std::optional<bool> KnownBits::eq(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  अगर (LHS.isConstant() && RHS.isConstant())
    वापस LHS.getConstant() == RHS.getConstant();
  अगर (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    वापस false;
  वापस std::nullopt;
}

std::optional<bool> KnownBits::ne(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  अगर (std::optional<bool> KnownEQ = eq(LHS, RHS))
    वापस !*KnownEQ;
  वापस std::nullopt;
}

std::optional<bool> KnownBits::ugt(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  अगर (LHS.get_Max_मूल्य().ule(RHS.get_Min_मूल्य()))
    वापस false;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  अगर (LHS.get_Min_मूल्य().ugt(RHS.get_Max_मूल्य()))
    वापस true;
  वापस std::nullopt;
}

std::optional<bool> KnownBits::uge(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  अगर (std::optional<bool> IsUGT = ugt(RHS, LHS))
    वापस !*IsUGT;
  वापस std::nullopt;
}

std::optional<bool> KnownBits::ult(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  वापस ugt(RHS, LHS);
}

std::optional<bool> KnownBits::ule(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  वापस uge(RHS, LHS);
}

std::optional<bool> KnownBits::sgt(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  अगर (LHS.get_Signed_Max_मूल्य().sle(RHS.get_Signed_Min_मूल्य()))
    वापस false;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  अगर (LHS.get_Signed_Min_मूल्य().sgt(RHS.get_Signed_Max_मूल्य()))
    वापस true;
  वापस std::nullopt;
}

std::optional<bool> KnownBits::sge(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  अगर (std::optional<bool> KnownSGT = sgt(RHS, LHS))
    वापस !*KnownSGT;
  वापस std::nullopt;
}

std::optional<bool> KnownBits::slt(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  वापस sgt(RHS, LHS);
}

std::optional<bool> KnownBits::sle(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  वापस sge(RHS, LHS);
}

KnownBits KnownBits::असल(bool IntMinIsPoison) स्थिर {
  // If the source's MSB is zero then we know the rest of the bits already.
  अगर (isNonNegative())
    वापस *यह;

  // Absolute value preserves trailing zero count.
  KnownBits KnownAbs(getBitWidth());

  // If the input is negative, then abs(x) == -x.
  अगर (isNegative()) {
    KnownBits Tmp = *यह;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    अगर (IntMinIsPoison && (Zero.popcount() + 2) == getBitWidth())
      Tmp.One.setBit(गिनती_Min_Trailing_Zeros());

    KnownAbs = computeForAddSub(
        /*Add*/ false, IntMinIsPoison, /*NUW=*/false,
        KnownBits::makeConstant(APInt(getBitWidth(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    अगर (IntMinIsPoison && Tmp.गिनती_Min_Population() == 1 &&
        Tmp.गिनती_Max_Population() != 1) {
      Tmp.One.clearSignBit();
      Tmp.Zero.setSignBit();
      KnownAbs.One.setBits(getBitWidth() - Tmp.गिनती_Min_Leading_Zeros(),
                           getBitWidth() - 1);
    }

  } अन्यथा {
    अचिन्हित MaxTZ = गिनती_Max_Trailing_Zeros();
    अचिन्हित MinTZ = गिनती_Min_Trailing_Zeros();

    KnownAbs.Zero.set_निचला_Bits(MinTZ);
    // If we know the lowest set 1, then preserve it.
    अगर (MaxTZ == MinTZ && MaxTZ < getBitWidth())
      KnownAbs.One.setBit(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    अगर (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_मूल्य())) {
      KnownAbs.One.clearSignBit();
      KnownAbs.Zero.setSignBit();
    }
  }

  वापस KnownAbs;
}

KnownBits KnownBits::reduceAdd(अचिन्हित अंक_Elts) स्थिर {
  अगर (अंक_Elts == 0)
    वापस KnownBits(getBitWidth());

  अचिन्हित BitWidth = getBitWidth();
  KnownBits परिणाम(BitWidth);

  अगर (isConstant())
    // If all elements are the same constant, we can simply compute it
    वापस KnownBits::makeConstant(अंक_Elts * getConstant());

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

  अगर (isNonNegative()) {
    अचिन्हित LeadingZeros = गिनती_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > LostBits ? LeadingZeros - LostBits : 0;
    परिणाम.Zero.set_ऊपरी_Bits(LeadingZeros);
  } अन्यथा अगर (isNegative()) {
    अचिन्हित LeadingOnes = गिनती_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > LostBits ? LeadingOnes - LostBits : 0;
    परिणाम.One.set_ऊपरी_Bits(LeadingOnes);
  }

  वापस परिणाम;
}

अटल KnownBits computeForSatAddSub(bool Add, bool Signed,
                                     स्थिर KnownBits &LHS,
                                     स्थिर KnownBits &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  अचिन्हित BitWidth = LHS.getBitWidth();

  std::optional<bool> Overflow;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  bool MayNegClamp = true;
  bool MayPosClamp = true;
  अगर (Signed) {
    // Easy cases we can rule out any overflow.
    अगर (Add && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      Overflow = false;
    अन्यथा अगर (!Add && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      Overflow = false;
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
          KnownBits::computeForAddSub(Add, /*NSW=*/false,
                                      /*NUW=*/false, UnsignedLHS, UnsignedRHS);
      अगर (Add) {
        अगर (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = false;
          // Pos + Pos will overflow with extra signbit.
          अगर (LHS.isNonNegative() && RHS.isNonNegative())
            Overflow = true;
        } अन्यथा अगर (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = false;
          // Neg + Neg will overflow without extra signbit.
          अगर (LHS.isNegative() && RHS.isNegative())
            Overflow = true;
        }
        // We will never clamp to the opposite sign of N-bit result.
        अगर (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = false;
        अगर (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = false;
      } अन्यथा {
        अगर (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = false;
          // Neg - Pos will overflow with extra signbit.
          अगर (LHS.isNegative() && RHS.isNonNegative())
            Overflow = true;
        } अन्यथा अगर (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = false;
          // Pos - Neg will overflow without extra signbit.
          अगर (LHS.isNonNegative() && RHS.isNegative())
            Overflow = true;
        }
        // We will never clamp to the opposite sign of N-bit result.
        अगर (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = false;
        अगर (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = false;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    अगर (!MayNegClamp && !MayPosClamp)
      Overflow = false;
  } अन्यथा अगर (Add) {
    // uadd.sat
    bool Of;
    (व्योम)LHS.get_Max_मूल्य().uadd_ov(RHS.get_Max_मूल्य(), Of);
    अगर (!Of) {
      Overflow = false;
    } अन्यथा {
      (व्योम)LHS.get_Min_मूल्य().uadd_ov(RHS.get_Min_मूल्य(), Of);
      अगर (Of)
        Overflow = true;
    }
  } अन्यथा {
    // usub.sat
    bool Of;
    (व्योम)LHS.get_Min_मूल्य().usub_ov(RHS.get_Max_मूल्य(), Of);
    अगर (!Of) {
      Overflow = false;
    } अन्यथा {
      (व्योम)LHS.get_Max_मूल्य().usub_ov(RHS.get_Min_मूल्य(), Of);
      अगर (Of)
        Overflow = true;
    }
  }

  KnownBits Res = KnownBits::computeForAddSub(Add, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  अगर (Overflow) {
    // We know whether or not we overflowed.
    अगर (!(*Overflow)) {
      // No overflow.
      वापस Res;
    }

    // We overflowed
    APInt C;
    अगर (Signed) {
      // sadd.sat / ssub.sat
      निश्चित(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_मूल्य(BitWidth)
                           : APInt::get_Signed_Max_मूल्य(BitWidth);
    } अन्यथा अगर (Add) {
      // uadd.sat
      C = APInt::get_Max_मूल्य(BitWidth);
    } अन्यथा {
      // uadd.sat
      C = APInt::get_Min_मूल्य(BitWidth);
    }

    Res.One = C;
    Res.Zero = ~C;
    वापस Res;
  }

  // We don't know if we overflowed.
  अगर (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    अगर (MayPosClamp)
      Res.Zero.clear_निचला_Bits(BitWidth - 1);
    अगर (MayNegClamp)
      Res.One.clear_निचला_Bits(BitWidth - 1);
  } अन्यथा अगर (Add) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clearAllBits();
  } अन्यथा {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clearAllBits();
  }

  वापस Res;
}

KnownBits KnownBits::sadd_sat(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  वापस computeForSatAddSub(/*Add*/ true, /*Signed*/ true, LHS, RHS);
}
KnownBits KnownBits::ssub_sat(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  वापस computeForSatAddSub(/*Add*/ false, /*Signed*/ true, LHS, RHS);
}
KnownBits KnownBits::uadd_sat(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  वापस computeForSatAddSub(/*Add*/ true, /*Signed*/ false, LHS, RHS);
}
KnownBits KnownBits::usub_sat(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  वापस computeForSatAddSub(/*Add*/ false, /*Signed*/ false, LHS, RHS);
}

अटल KnownBits avgComputeU(KnownBits LHS, KnownBits RHS, bool IsCeil) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  LHS = LHS.zext(BitWidth + 1);
  RHS = RHS.zext(BitWidth + 1);
  LHS =
      computeForAddCarry(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.extractBits(BitWidth, 1);
  वापस LHS;
}

KnownBits KnownBits::avgFloorS(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  वापस flipSignBit(avgFloorU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgFloorU(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  वापस avgComputeU(LHS, RHS, /*IsCeil=*/false);
}

KnownBits KnownBits::avgCeilS(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  वापस flipSignBit(avgCeilU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgCeilU(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  वापस avgComputeU(LHS, RHS, /*IsCeil=*/true);
}

KnownBits KnownBits::mul(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS,
                         bool NoUndefSelfMultiply) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  निश्चित(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  निश्चित((!NoUndefSelfMultiply || LHS == RHS) &&
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
  bool HasOverflow;
  APInt UMax_परिणाम = UMaxLHS.umul_ov(UMaxRHS, HasOverflow);
  अचिन्हित LeadZ = HasOverflow ? 0 : UMax_परिणाम.countl_zero();

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
  अचिन्हित TrailZeroLHS = LHS.गिनती_Min_Trailing_Zeros();
  अचिन्हित TrailZeroRHS = RHS.गिनती_Min_Trailing_Zeros();
  अचिन्हित TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  अचिन्हित SmallestOperand = std::min(TrailBitsKnownLHS - TrailZeroLHS,
                                      TrailBitsKnownRHS - TrailZeroRHS);
  अचिन्हित परिणाम_Bits_Known = std::min(SmallestOperand + TrailZ, BitWidth);

  // The lower ResultBitsKnown bits of this are known.
  APInt BottomKnown = LHS.One * RHS.One;

  KnownBits Res(BitWidth);
  Res.Zero.set_ऊपरी_Bits(LeadZ);
  Res.Zero |= (~BottomKnown).getLoBits(परिणाम_Bits_Known);
  Res.One = BottomKnown.getLoBits(परिणाम_Bits_Known);

  अगर (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    अचिन्हित TwoTZP1 = 2 * TrailZeroLHS + 1;
    अगर (TwoTZP1 < BitWidth)
      Res.Zero.setBit(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    अगर (TrailZeroLHS < BitWidth && LHS.One[TrailZeroLHS]) {
      अचिन्हित TwoTZP2 = TwoTZP1 + 1;
      अगर (TwoTZP2 < BitWidth)
        Res.Zero.setBit(TwoTZP2);
    }
  }

  वापस Res;
}

KnownBits KnownBits::mulhs(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  निश्चित(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.sext(2 * BitWidth);
  KnownBits WideRHS = RHS.sext(2 * BitWidth);
  वापस mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

KnownBits KnownBits::mulhu(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  निश्चित(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.zext(2 * BitWidth);
  KnownBits WideRHS = RHS.zext(2 * BitWidth);
  वापस mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

अटल KnownBits div_Compute_निचला_Bit(KnownBits Known, स्थिर KnownBits &LHS,
                                  स्थिर KnownBits &RHS, bool Exact) {

  अगर (!Exact)
    वापस Known;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  अगर (LHS.One[0])
    Known.One.setBit(0);

  पूर्णांक MinTZ =
      (पूर्णांक)LHS.गिनती_Min_Trailing_Zeros() - (पूर्णांक)RHS.गिनती_Max_Trailing_Zeros();
  पूर्णांक MaxTZ =
      (पूर्णांक)LHS.गिनती_Max_Trailing_Zeros() - (पूर्णांक)RHS.गिनती_Min_Trailing_Zeros();
  अगर (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    Known.Zero.set_निचला_Bits(MinTZ);
    अगर (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      Known.One.setBit(MinTZ);
    }
  } अन्यथा अगर (MaxTZ < 0) {
    // Poison Result
    Known.setAllZero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  अगर (Known.hasConflict())
    Known.setAllZero();

  वापस Known;
}

KnownBits KnownBits::sdiv(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS,
                          bool Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  अगर (LHS.isNonNegative() && RHS.isNonNegative())
    वापस udiv(LHS, RHS, Exact);

  अचिन्हित BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  अगर (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    वापस Known;
  }

  std::optional<APInt> Res;
  अगर (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_मूल्य();
    APInt अंक = LHS.get_Signed_Min_मूल्य();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (अंक.is_Min_Signed_मूल्य() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_मूल्य(BitWidth)
              : अंक.sdiv(Denom);
  } अन्यथा अगर (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    अगर (Exact || (-LHS.get_Signed_Max_मूल्य()).uge(RHS.get_Signed_Max_मूल्य())) {
      APInt Denom = RHS.get_Signed_Min_मूल्य();
      APInt अंक = LHS.get_Signed_Min_मूल्य();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      अगर (Denom.isZero())
        Denom.setBit(RHS.गिनती_Min_Trailing_Zeros());
      Res = अंक.sdiv(Denom);
    }
  } अन्यथा अगर (LHS.isStrictlyPositive() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    अगर (Exact || LHS.get_Signed_Min_मूल्य().uge(-RHS.get_Signed_Min_मूल्य())) {
      APInt Denom = RHS.get_Signed_Max_मूल्य();
      APInt अंक = LHS.get_Signed_Max_मूल्य();
      Res = अंक.sdiv(Denom);
    }
  }

  अगर (Res) {
    अगर (Res->isNonNegative()) {
      अचिन्हित LeadZ = Res->गिनती_Leading_Zeros();
      Known.Zero.set_ऊपरी_Bits(LeadZ);
    } अन्यथा {
      अचिन्हित LeadO = Res->गिनती_Leading_Ones();
      Known.One.set_ऊपरी_Bits(LeadO);
    }
  }

  Known = div_Compute_निचला_Bit(Known, LHS, RHS, Exact);
  वापस Known;
}

KnownBits KnownBits::udiv(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS,
                          bool Exact) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  अगर (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    वापस Known;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_मूल्य();
  अगर (MinDenom.isZero())
    MinDenom.setBit(RHS.गिनती_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_मूल्य().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_मूल्य().udiv(RHS.get_Max_मूल्य());

  Known = KnownBits::makeConstant(MinRes);
  अगर (std::optional<अचिन्हित> DifferentBit =
          APIntOps::GetMostSignificantDifferentBit(MinRes, MaxRes)) {
    Known.Zero.clear_निचला_Bits(*DifferentBit + 1);
    Known.One.clear_निचला_Bits(*DifferentBit + 1);
  }

  Known = div_Compute_निचला_Bit(Known, LHS, RHS, Exact);

  वापस Known;
}

KnownBits KnownBits::rem_Get_निचला_Bits(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  अचिन्हित BitWidth = LHS.getBitWidth();
  अगर (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    अचिन्हित RHSZeros = RHS.गिनती_Min_Trailing_Zeros();
    APInt Mask = APInt::get_निचला_Bits_Set(BitWidth, RHSZeros);
    APInt OnesMask = LHS.One & Mask;
    APInt ZerosMask = LHS.Zero & Mask;
    वापस KnownBits(ZerosMask, OnesMask);
  }
  वापस KnownBits(BitWidth);
}

KnownBits KnownBits::urem(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  अगर (LHS.get_Max_मूल्य().ult(RHS.get_Min_मूल्य()))
    वापस LHS;

  KnownBits Known = rem_Get_निचला_Bits(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_मूल्य(), RHS.get_Max_मूल्य() - 1);
  Known.Zero.set_ऊपरी_Bits(MaxRes.गिनती_Leading_Zeros());
  वापस Known;
}

KnownBits KnownBits::srem(स्थिर KnownBits &LHS, स्थिर KnownBits &RHS) {
  KnownBits Known = rem_Get_निचला_Bits(LHS, RHS);
  अगर (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt निचला_Bits = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    अगर (LHS.isNonNegative() || निचला_Bits.isSubsetOf(LHS.Zero))
      Known.Zero |= ~निचला_Bits;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    अगर (LHS.isNegative() && निचला_Bits.intersects(LHS.One))
      Known.One |= ~निचला_Bits;
    वापस Known;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  अगर (LHS.isNegative() && Known.isNonZero())
    Known.One.set_ऊपरी_Bits(
        std::max(LHS.गिनती_Min_Leading_Ones(), RHS.गिनती_Min_Sign_Bits()));
  अन्यथा अगर (LHS.isNonNegative())
    Known.Zero.set_ऊपरी_Bits(
        std::max(LHS.गिनती_Min_Leading_Zeros(), RHS.गिनती_Min_Sign_Bits()));
  वापस Known;
}

KnownBits &KnownBits::चालक&=(स्थिर KnownBits &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  वापस *यह;
}

KnownBits &KnownBits::चालक|=(स्थिर KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  वापस *यह;
}

KnownBits &KnownBits::चालक^=(स्थिर KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  वापस *यह;
}

KnownBits KnownBits::blsi() स्थिर {
  अचिन्हित BitWidth = getBitWidth();
  KnownBits Known(Zero, APInt(BitWidth, 0));
  अचिन्हित Max = गिनती_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  अचिन्हित Min = गिनती_Min_Trailing_Zeros();
  अगर (Max == Min && Max < BitWidth)
    Known.One.setBit(Max);
  वापस Known;
}

KnownBits KnownBits::blsmsk() स्थिर {
  अचिन्हित BitWidth = getBitWidth();
  KnownBits Known(BitWidth);
  अचिन्हित Max = गिनती_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  अचिन्हित Min = गिनती_Min_Trailing_Zeros();
  Known.One.set_निचला_Bits(std::min(Min + 1, BitWidth));
  वापस Known;
}

व्योम KnownBits::print(raw_ostream &OS) स्थिर {
  अचिन्हित BitWidth = getBitWidth();
  क्रम (अचिन्हित I = 0; I < BitWidth; ++I) {
    अचिन्हित N = BitWidth - I - 1;
    अगर (Zero[N] && One[N])
      OS << "!";
    अन्यथा अगर (Zero[N])
      OS << "0";
    अन्यथा अगर (One[N])
      OS << "1";
    अन्यथा
      OS << "?";
  }
}

#अगर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD व्योम KnownBits::dump() स्थिर {
  print(dbgs());
  dbgs() << "\n";
}
#पूर्ण_अगर

अचिन्हित llvm::SignBitsOps::rot(अचिन्हित SrcSignBits, अचिन्हित BitWidth,
                                std::optional<APInt> RotAmt,
                                bool Is_Rotate_दायाँ) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  अगर (SrcSignBits == BitWidth)
    वापस BitWidth;

  अगर (!RotAmt)
    वापस 1;

  अचिन्हित Amt = RotAmt->urem(BitWidth);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  अगर (Is_Rotate_दायाँ)
    Amt = (BitWidth - Amt) % BitWidth;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  अगर (SrcSignBits > Amt + 1)
    वापस SrcSignBits - Amt;
  वापस 1;
}
