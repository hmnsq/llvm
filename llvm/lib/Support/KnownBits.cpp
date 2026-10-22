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
  प्रत्यागम KnownBits(Zero, One);
}

स्थिर KnownBits computeForAddCarry(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS,
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
  प्रत्यागम KnownOut;
}

KnownBits KnownBits::computeForAddCarry(
    ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS, ध्रुव KnownBits &Carry) {
  प्रतिज्ञा(Carry.getBitWidth() == 1 && "Carry must be 1-bit");
  प्रत्यागम ::computeForAddCarry(
      LHS, RHS, Carry.Zero.get_Bool_मूल्य(), Carry.One.get_Bool_मूल्य());
}

KnownBits KnownBits::computeForAddSub(तार्किक Add, तार्किक NSW, तार्किक NUW,
                                      ध्रुव KnownBits &LHS,
                                      ध्रुव KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits KnownOut(BitWidth);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  यदि (LHS.isUnknown() && RHS.isUnknown())
    प्रत्यागम KnownOut;

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
      APInt MinVal = LHS.get_Min_मूल्य().uadd_sat(RHS.get_Min_मूल्य());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      यदि (NSW) {
        unsigned अंक_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
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
      यदि (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        unsigned अंक_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
        KnownOut.Zero.setBits(BitWidth - 1 - अंक_Bits_2, BitWidth - 1);
      }
      KnownOut.Zero.set_वरचा_Bits(MaxVal.countl_zero());
    }
  }

  यदि (NSW) {
    APInt MinVal;
    APInt MaxVal;
    यदि (Add) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_मूल्य().sadd_sat(RHS.get_Signed_Min_मूल्य());
      MaxVal = LHS.get_Signed_Max_मूल्य().sadd_sat(RHS.get_Signed_Max_मूल्य());
    } अन्यथा {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_मूल्य().ssub_sat(RHS.get_Signed_Max_मूल्य());
      MaxVal = LHS.get_Signed_Max_मूल्य().ssub_sat(RHS.get_Signed_Min_मूल्य());
    }
    यदि (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      unsigned अंक_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
      KnownOut.One.setBits(BitWidth - 1 - अंक_Bits_2, BitWidth - 1);
      KnownOut.Zero.setSignBit();
    }
    यदि (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      unsigned अंक_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
      KnownOut.Zero.setBits(BitWidth - 1 - अंक_Bits_2, BitWidth - 1);
      KnownOut.One.setSignBit();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  यदि (KnownOut.hasConflict())
    KnownOut.setAllZero();
  प्रत्यागम KnownOut;
}

KnownBits KnownBits::computeForSubBorrow(ध्रुव KnownBits &LHS, KnownBits RHS,
                                         ध्रुव KnownBits &Borrow) {
  प्रतिज्ञा(Borrow.getBitWidth() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::swap(RHS.Zero, RHS.One);
  प्रत्यागम ::computeForAddCarry(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_मूल्य(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_मूल्य());
}

KnownBits KnownBits::truncSSat(unsigned BitWidth) ध्रुव {
  unsigned आदान_Bits = getBitWidth();
  APInt MinInRange = APInt::get_Signed_Min_मूल्य(BitWidth).sext(आदान_Bits);
  APInt MaxInRange = APInt::get_Signed_Max_मूल्य(BitWidth).sext(आदान_Bits);
  APInt आदान_Min = get_Signed_Min_मूल्य();
  APInt आदान_Max = get_Signed_Max_मूल्य();
  KnownBits Known(BitWidth);

  // Case 1: All values fit - just truncate
  यदि (आदान_Min.sge(MinInRange) && आदान_Max.sle(MaxInRange)) {
    Known = trunc(BitWidth);
  }
  // Case 2: All saturate to min
  अन्यथा यदि (आदान_Max.slt(MinInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Min_मूल्य(BitWidth));
  }
  // Case 3: All saturate to max
  अन्यथा यदि (आदान_Min.sgt(MaxInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Max_मूल्य(BitWidth));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  अन्यथा यदि (आदान_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    Known.Zero = APInt(BitWidth, 0);
    Known.Zero.setBit(BitWidth - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    Known.One = One.trunc(BitWidth);
    Known.One.clearBit(BitWidth - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  अन्यथा यदि (आदान_Max.isNegative()) {
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
    यदि (आदान_Max.sle(MaxInRange)) {
      // Positive values fit, only negatives saturate to min
      Known.Zero = आदान_Zero_Lower & MinSatKB.Zero;
      Known.One = आदान_One_Lower & MinSatKB.One;
    } अन्यथा यदि (आदान_Min.sge(MinInRange)) {
      // Negative values fit, only positives saturate to max
      Known.Zero = आदान_Zero_Lower & MaxSatKB.Zero;
      Known.One = आदान_One_Lower & MaxSatKB.One;
    } अन्यथा {
      // Both positive and negative values might saturate
      Known.Zero = आदान_Zero_Lower & MinSatKB.Zero & MaxSatKB.Zero;
      Known.One = आदान_One_Lower & MinSatKB.One & MaxSatKB.One;
    }
  }
  प्रत्यागम Known;
}

KnownBits KnownBits::truncSSatU(unsigned BitWidth) ध्रुव {
  unsigned आदान_Bits = getBitWidth();
  APInt MaxInRange = APInt::getAllOnes(BitWidth).zext(आदान_Bits);
  APInt आदान_Min = get_Signed_Min_मूल्य();
  APInt आदान_Max = get_Signed_Max_मूल्य();
  KnownBits Known(BitWidth);

  यदि (isNegative()) {
    Known.setAllZero();
  } अन्यथा यदि (आदान_Min.isNonNegative() && आदान_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } अन्यथा यदि (आदान_Min.isNonNegative() && आदान_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } अन्यथा यदि (आदान_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    Known.Zero = APInt(BitWidth, 0);
    // Common one bits: bits that are 1 in input stay 1
    Known.One = One.trunc(BitWidth);
  } अन्यथा {
    // Mixed: sign bit unknown
    यदि (आदान_Max.ule(MaxInRange)) {
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
  प्रत्यागम Known;
}

KnownBits KnownBits::truncUSat(unsigned BitWidth) ध्रुव {
  unsigned आदान_Bits = getBitWidth();
  APInt MaxInRange = APInt::get_खालचा_Bits_Set(आदान_Bits, BitWidth);
  APInt आदान_Max = get_Max_मूल्य();
  APInt आदान_Min = get_Min_मूल्य();
  KnownBits Known(BitWidth);

  यदि (आदान_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } अन्यथा यदि (आदान_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } अन्यथा {
    Known.resetAll();
    Known.One = One.trunc(BitWidth);
  }
  प्रत्यागम Known;
}

KnownBits KnownBits::sextInReg(unsigned SrcBitWidth) ध्रुव {
  unsigned BitWidth = getBitWidth();
  प्रतिज्ञा(0 < SrcBitWidth && SrcBitWidth <= BitWidth &&
         "Illegal sext-in-register");

  यदि (SrcBitWidth == BitWidth)
    प्रत्यागम *एतत्;

  unsigned ExtBits = BitWidth - SrcBitWidth;
  KnownBits Result;
  Result.One = One << ExtBits;
  Result.Zero = Zero << ExtBits;
  Result.One.ashrInPlace(ExtBits);
  Result.Zero.ashrInPlace(ExtBits);
  प्रत्यागम Result;
}

KnownBits KnownBits::makeGE(ध्रुव APInt &Val) ध्रुव {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  unsigned N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_खालचा_Bits(getBitWidth() - N);
  प्रत्यागम KnownBits(Zero, One | MaskedVal);
}

KnownBits KnownBits::umax(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  यदि (LHS.get_Min_मूल्य().uge(RHS.get_Max_मूल्य()))
    प्रत्यागम LHS;
  यदि (RHS.get_Min_मूल्य().uge(LHS.get_Max_मूल्य()))
    प्रत्यागम RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  KnownBits L = LHS.makeGE(RHS.get_Min_मूल्य());
  KnownBits R = RHS.makeGE(LHS.get_Min_मूल्य());
  प्रत्यागम L.intersectWith(R);
}

KnownBits KnownBits::umin(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](ध्रुव KnownBits &Val) { प्रत्यागम KnownBits(Val.One, Val.Zero); };
  प्रत्यागम Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::smax(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  प्रत्यागम flipSignBit(umax(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::smin(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](ध्रुव KnownBits &Val) {
    unsigned SignBitPosition = Val.getBitWidth() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
    One.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
    प्रत्यागम KnownBits(Zero, One);
  };
  प्रत्यागम Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::abdu(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  यदि (LHS.get_Min_मूल्य().uge(RHS.get_Max_मूल्य()))
    प्रत्यागम computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/असत्य, LHS,
                            RHS);
  यदि (RHS.get_Min_मूल्य().uge(LHS.get_Max_मूल्य()))
    प्रत्यागम computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/असत्य, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/सत्य, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/सत्य, RHS, LHS);
  प्रत्यागम Diff0.intersectWith(Diff1);
}

KnownBits KnownBits::abds(KnownBits LHS, KnownBits RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  यदि (LHS.get_Signed_Min_मूल्य().sge(RHS.get_Signed_Max_मूल्य()))
    प्रत्यागम computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/असत्य, LHS,
                            RHS);
  यदि (RHS.get_Signed_Min_मूल्य().sge(LHS.get_Signed_Max_मूल्य()))
    प्रत्यागम computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/असत्य, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  unsigned SignBitPosition = LHS.getBitWidth() - 1;
  प्रति (auto Arg : {&LHS, &RHS}) {
    तार्किक Tmp = Arg->Zero[SignBitPosition];
    Arg->Zero.setBitVal(SignBitPosition, Arg->One[SignBitPosition]);
    Arg->One.setBitVal(SignBitPosition, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/सत्य, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/सत्य, RHS, LHS);
  प्रत्यागम Diff0.intersectWith(Diff1);
}

स्थिर unsigned getMaxShiftAmount(ध्रुव APInt &Max_मूल्य, unsigned BitWidth) {
  यदि (isPowerOf2_32(BitWidth))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    प्रत्यागम Max_मूल्य.extract_Bits_As_ZExt_मूल्य(
        std::min(Log2_32(BitWidth), Max_मूल्य.getBitWidth()), 0);
  // This is only an approximate upper bound.
  प्रत्यागम Max_मूल्य.get_Limited_मूल्य(BitWidth - 1);
}

KnownBits KnownBits::shl(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS, तार्किक NUW,
                         तार्किक NSW, तार्किक ShAmtNonZero) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ध्रुव KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known;
    तार्किक ShiftedOutZero, ShiftedOutOne;
    Known.Zero = LHS.Zero.ushl_ov(ShiftAmt, ShiftedOutZero);
    Known.Zero.set_खालचा_Bits(ShiftAmt);
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
    प्रत्यागम Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_मूल्य().get_Limited_मूल्य(BitWidth);
  यदि (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  यदि (LHS.isUnknown()) {
    Known.Zero.set_खालचा_Bits(MinShiftAmount);
    यदि (NUW && NSW && MinShiftAmount != 0)
      Known.makeNonNegative();
    प्रत्यागम Known;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_मूल्य = RHS.get_Max_मूल्य();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_मूल्य, BitWidth);
  यदि (NUW && NSW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.मोजणी_Max_Leading_Zeros() - 1);
  यदि (NUW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.मोजणी_Max_Leading_Zeros());
  यदि (NSW)
    MaxShiftAmount = std::min(
        MaxShiftAmount,
        std::max(LHS.मोजणी_Max_Leading_Zeros(), LHS.मोजणी_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  यदि (MinShiftAmount == 0 && MaxShiftAmount == BitWidth - 1 &&
      isPowerOf2_32(BitWidth)) {
    Known.Zero.set_खालचा_Bits(LHS.मोजणी_Min_Trailing_Zeros());
    यदि (LHS.isAllOnes())
      Known.One.setSignBit();
    यदि (NSW) {
      यदि (LHS.isNonNegative())
        Known.makeNonNegative();
      यदि (LHS.isNegative())
        Known.makeNegative();
    }
    प्रत्यागम Known;
  }

  // Find the common bits from all possible shifts.
  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_मूल्य();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_मूल्य();
  Known.setAllConflict();
  प्रति (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    यदि ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      अनुवर्त;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    यदि (Known.isUnknown())
      भङ्ग;
  }

  // All shift amounts may result in poison.
  यदि (Known.hasConflict())
    Known.setAllZero();
  प्रत्यागम Known;
}

KnownBits KnownBits::lshr(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS,
                          तार्किक ShAmtNonZero, तार्किक Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ध्रुव KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known >>= ShiftAmt;
    // High bits are known zero.
    Known.Zero.set_वरचा_Bits(ShiftAmt);
    प्रत्यागम Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_मूल्य().get_Limited_मूल्य(BitWidth);
  यदि (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  यदि (LHS.isUnknown()) {
    Known.Zero.set_वरचा_Bits(MinShiftAmount);
    प्रत्यागम Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_मूल्य = RHS.get_Max_मूल्य();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_मूल्य, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  यदि (Exact) {
    unsigned FirstOne = LHS.मोजणी_Max_Trailing_Zeros();
    यदि (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      प्रत्यागम Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_मूल्य();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_मूल्य();
  Known.setAllConflict();
  प्रति (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    यदि ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      अनुवर्त;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    यदि (Known.isUnknown())
      भङ्ग;
  }

  // All shift amounts may result in poison.
  यदि (Known.hasConflict())
    Known.setAllZero();
  प्रत्यागम Known;
}

KnownBits KnownBits::ashr(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS,
                          तार्किक ShAmtNonZero, तार्किक Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ध्रुव KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known.Zero.ashrInPlace(ShiftAmt);
    Known.One.ashrInPlace(ShiftAmt);
    प्रत्यागम Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_मूल्य().get_Limited_मूल्य(BitWidth);
  यदि (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  यदि (LHS.isUnknown()) {
    यदि (MinShiftAmount == BitWidth) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      प्रत्यागम Known;
    }
    प्रत्यागम Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_मूल्य = RHS.get_Max_मूल्य();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_मूल्य, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  यदि (Exact) {
    unsigned FirstOne = LHS.मोजणी_Max_Trailing_Zeros();
    यदि (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      प्रत्यागम Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_मूल्य();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_मूल्य();
  Known.setAllConflict();
  प्रति (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
      ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    यदि ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      अनुवर्त;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    यदि (Known.isUnknown())
      भङ्ग;
  }

  // All shift amounts may result in poison.
  यदि (Known.hasConflict())
    Known.setAllZero();
  प्रत्यागम Known;
}

KnownBits KnownBits::fshl(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS,
                          ध्रुव APInt &Amt) {
  प्रत्यागम KnownBits(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::fshr(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS,
                          ध्रुव APInt &Amt) {
  प्रत्यागम KnownBits(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::clmul(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  KnownBits Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_मूल्य(), RHS.get_Min_मूल्य()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  auto ClMulOr = [](ध्रुव APInt &LHS, ध्रुव APInt &RHS) {
    unsigned BW = LHS.getBitWidth();
    प्रतिज्ञा(BW == RHS.getBitWidth() && "Operand mismatch");
    APInt Result(BW, 0);
    प्रति (unsigned I :
         seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
      यदि (RHS[I])
        Result |= LHS << I;
    प्रत्यागम Result;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt Known = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= Known;
  Res.One &= Known;

  प्रत्यागम Res;
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
  प्रति (unsigned I = 0; I < BitWidth; ++I) {
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
  प्रत्यागम Res;
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
  प्रति (unsigned I = 0; I < BitWidth; ++I) {
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
  प्रत्यागम Res;
}

std::optional<तार्किक> KnownBits::eq(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  यदि (LHS.isConstant() && RHS.isConstant())
    प्रत्यागम LHS.getConstant() == RHS.getConstant();
  यदि (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    प्रत्यागम असत्य;
  प्रत्यागम std::nullopt;
}

std::optional<तार्किक> KnownBits::ne(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  यदि (std::optional<तार्किक> KnownEQ = eq(LHS, RHS))
    प्रत्यागम !*KnownEQ;
  प्रत्यागम std::nullopt;
}

std::optional<तार्किक> KnownBits::ugt(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  यदि (LHS.get_Max_मूल्य().ule(RHS.get_Min_मूल्य()))
    प्रत्यागम असत्य;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  यदि (LHS.get_Min_मूल्य().ugt(RHS.get_Max_मूल्य()))
    प्रत्यागम सत्य;
  प्रत्यागम std::nullopt;
}

std::optional<तार्किक> KnownBits::uge(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  यदि (std::optional<तार्किक> IsUGT = ugt(RHS, LHS))
    प्रत्यागम !*IsUGT;
  प्रत्यागम std::nullopt;
}

std::optional<तार्किक> KnownBits::ult(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  प्रत्यागम ugt(RHS, LHS);
}

std::optional<तार्किक> KnownBits::ule(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  प्रत्यागम uge(RHS, LHS);
}

std::optional<तार्किक> KnownBits::sgt(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  यदि (LHS.get_Signed_Max_मूल्य().sle(RHS.get_Signed_Min_मूल्य()))
    प्रत्यागम असत्य;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  यदि (LHS.get_Signed_Min_मूल्य().sgt(RHS.get_Signed_Max_मूल्य()))
    प्रत्यागम सत्य;
  प्रत्यागम std::nullopt;
}

std::optional<तार्किक> KnownBits::sge(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  यदि (std::optional<तार्किक> KnownSGT = sgt(RHS, LHS))
    प्रत्यागम !*KnownSGT;
  प्रत्यागम std::nullopt;
}

std::optional<तार्किक> KnownBits::slt(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  प्रत्यागम sgt(RHS, LHS);
}

std::optional<तार्किक> KnownBits::sle(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  प्रत्यागम sge(RHS, LHS);
}

KnownBits KnownBits::abs(तार्किक IntMinIsPoison) ध्रुव {
  // If the source's MSB is zero then we know the rest of the bits already.
  यदि (isNonNegative())
    प्रत्यागम *एतत्;

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
    यदि (IntMinIsPoison && Tmp.मोजणी_Min_Population() == 1 &&
        Tmp.मोजणी_Max_Population() != 1) {
      Tmp.One.clearSignBit();
      Tmp.Zero.setSignBit();
      KnownAbs.One.setBits(getBitWidth() - Tmp.मोजणी_Min_Leading_Zeros(),
                           getBitWidth() - 1);
    }

  } अन्यथा {
    unsigned MaxTZ = मोजणी_Max_Trailing_Zeros();
    unsigned MinTZ = मोजणी_Min_Trailing_Zeros();

    KnownAbs.Zero.set_खालचा_Bits(MinTZ);
    // If we know the lowest set 1, then preserve it.
    यदि (MaxTZ == MinTZ && MaxTZ < getBitWidth())
      KnownAbs.One.setBit(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    यदि (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_मूल्य())) {
      KnownAbs.One.clearSignBit();
      KnownAbs.Zero.setSignBit();
    }
  }

  प्रत्यागम KnownAbs;
}

KnownBits KnownBits::reduceAdd(unsigned अंक_Elts) ध्रुव {
  यदि (अंक_Elts == 0)
    प्रत्यागम KnownBits(getBitWidth());

  unsigned BitWidth = getBitWidth();
  KnownBits Result(BitWidth);

  यदि (isConstant())
    // If all elements are the same constant, we can simply compute it
    प्रत्यागम KnownBits::makeConstant(अंक_Elts * getConstant());

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
  unsigned LostBits = Log2_32_Ceil(अंक_Elts);

  यदि (isNonNegative()) {
    unsigned LeadingZeros = मोजणी_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > LostBits ? LeadingZeros - LostBits : 0;
    Result.Zero.set_वरचा_Bits(LeadingZeros);
  } अन्यथा यदि (isNegative()) {
    unsigned LeadingOnes = मोजणी_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > LostBits ? LeadingOnes - LostBits : 0;
    Result.One.set_वरचा_Bits(LeadingOnes);
  }

  प्रत्यागम Result;
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
    (शून्य)LHS.get_Max_मूल्य().uadd_ov(RHS.get_Max_मूल्य(), Of);
    यदि (!Of) {
      Overflow = असत्य;
    } अन्यथा {
      (शून्य)LHS.get_Min_मूल्य().uadd_ov(RHS.get_Min_मूल्य(), Of);
      यदि (Of)
        Overflow = सत्य;
    }
  } अन्यथा {
    // usub.sat
    तार्किक Of;
    (शून्य)LHS.get_Min_मूल्य().usub_ov(RHS.get_Max_मूल्य(), Of);
    यदि (!Of) {
      Overflow = असत्य;
    } अन्यथा {
      (शून्य)LHS.get_Max_मूल्य().usub_ov(RHS.get_Min_मूल्य(), Of);
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
      प्रत्यागम Res;
    }

    // We overflowed
    APInt C;
    यदि (Signed) {
      // sadd.sat / ssub.sat
      प्रतिज्ञा(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_मूल्य(BitWidth)
                           : APInt::get_Signed_Max_मूल्य(BitWidth);
    } अन्यथा यदि (Add) {
      // uadd.sat
      C = APInt::get_Max_मूल्य(BitWidth);
    } अन्यथा {
      // uadd.sat
      C = APInt::get_Min_मूल्य(BitWidth);
    }

    Res.One = C;
    Res.Zero = ~C;
    प्रत्यागम Res;
  }

  // We don't know if we overflowed.
  यदि (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    यदि (MayPosClamp)
      Res.Zero.clear_खालचा_Bits(BitWidth - 1);
    यदि (MayNegClamp)
      Res.One.clear_खालचा_Bits(BitWidth - 1);
  } अन्यथा यदि (Add) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clearAllBits();
  } अन्यथा {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clearAllBits();
  }

  प्रत्यागम Res;
}

KnownBits KnownBits::sadd_sat(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  प्रत्यागम computeForSatAddSub(/*Add*/ सत्य, /*Signed*/ सत्य, LHS, RHS);
}
KnownBits KnownBits::ssub_sat(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  प्रत्यागम computeForSatAddSub(/*Add*/ असत्य, /*Signed*/ सत्य, LHS, RHS);
}
KnownBits KnownBits::uadd_sat(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  प्रत्यागम computeForSatAddSub(/*Add*/ सत्य, /*Signed*/ असत्य, LHS, RHS);
}
KnownBits KnownBits::usub_sat(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  प्रत्यागम computeForSatAddSub(/*Add*/ असत्य, /*Signed*/ असत्य, LHS, RHS);
}

स्थिर KnownBits avgComputeU(KnownBits LHS, KnownBits RHS, तार्किक IsCeil) {
  unsigned BitWidth = LHS.getBitWidth();
  LHS = LHS.zext(BitWidth + 1);
  RHS = RHS.zext(BitWidth + 1);
  LHS =
      computeForAddCarry(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.extractBits(BitWidth, 1);
  प्रत्यागम LHS;
}

KnownBits KnownBits::avgFloorS(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  प्रत्यागम flipSignBit(avgFloorU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgFloorU(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  प्रत्यागम avgComputeU(LHS, RHS, /*IsCeil=*/असत्य);
}

KnownBits KnownBits::avgCeilS(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  प्रत्यागम flipSignBit(avgCeilU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgCeilU(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  प्रत्यागम avgComputeU(LHS, RHS, /*IsCeil=*/सत्य);
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
  APInt UMaxLHS = LHS.get_Max_मूल्य();
  APInt UMaxRHS = RHS.get_Max_मूल्य();

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
  unsigned TrailZeroLHS = LHS.मोजणी_Min_Trailing_Zeros();
  unsigned TrailZeroRHS = RHS.मोजणी_Min_Trailing_Zeros();
  unsigned TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  unsigned SmallestOperand = std::min(TrailBitsKnownLHS - TrailZeroLHS,
                                      TrailBitsKnownRHS - TrailZeroRHS);
  unsigned ResultBitsKnown = std::min(SmallestOperand + TrailZ, BitWidth);

  // The lower ResultBitsKnown bits of this are known.
  APInt BottomKnown = LHS.One * RHS.One;

  KnownBits Res(BitWidth);
  Res.Zero.set_वरचा_Bits(LeadZ);
  Res.Zero |= (~BottomKnown).getLoBits(ResultBitsKnown);
  Res.One = BottomKnown.getLoBits(ResultBitsKnown);

  यदि (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    unsigned TwoTZP1 = 2 * TrailZeroLHS + 1;
    यदि (TwoTZP1 < BitWidth)
      Res.Zero.setBit(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    यदि (TrailZeroLHS < BitWidth && LHS.One[TrailZeroLHS]) {
      unsigned TwoTZP2 = TwoTZP1 + 1;
      यदि (TwoTZP2 < BitWidth)
        Res.Zero.setBit(TwoTZP2);
    }
  }

  प्रत्यागम Res;
}

KnownBits KnownBits::mulhs(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  प्रतिज्ञा(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.sext(2 * BitWidth);
  KnownBits WideRHS = RHS.sext(2 * BitWidth);
  प्रत्यागम mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

KnownBits KnownBits::mulhu(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  प्रतिज्ञा(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.zext(2 * BitWidth);
  KnownBits WideRHS = RHS.zext(2 * BitWidth);
  प्रत्यागम mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

स्थिर KnownBits div_Compute_खालचा_Bit(KnownBits Known, ध्रुव KnownBits &LHS,
                                  ध्रुव KnownBits &RHS, तार्किक Exact) {

  यदि (!Exact)
    प्रत्यागम Known;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  यदि (LHS.One[0])
    Known.One.setBit(0);

  पूर्णाङ्क MinTZ =
      (पूर्णाङ्क)LHS.मोजणी_Min_Trailing_Zeros() - (पूर्णाङ्क)RHS.मोजणी_Max_Trailing_Zeros();
  पूर्णाङ्क MaxTZ =
      (पूर्णाङ्क)LHS.मोजणी_Max_Trailing_Zeros() - (पूर्णाङ्क)RHS.मोजणी_Min_Trailing_Zeros();
  यदि (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    Known.Zero.set_खालचा_Bits(MinTZ);
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

  प्रत्यागम Known;
}

KnownBits KnownBits::sdiv(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS,
                          तार्किक Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  यदि (LHS.isNonNegative() && RHS.isNonNegative())
    प्रत्यागम udiv(LHS, RHS, Exact);

  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  यदि (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    प्रत्यागम Known;
  }

  std::optional<APInt> Res;
  यदि (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_मूल्य();
    APInt अंक = LHS.get_Signed_Min_मूल्य();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (अंक.is_Min_Signed_मूल्य() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_मूल्य(BitWidth)
              : अंक.sdiv(Denom);
  } अन्यथा यदि (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    यदि (Exact || (-LHS.get_Signed_Max_मूल्य()).uge(RHS.get_Signed_Max_मूल्य())) {
      APInt Denom = RHS.get_Signed_Min_मूल्य();
      APInt अंक = LHS.get_Signed_Min_मूल्य();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      यदि (Denom.isZero())
        Denom.setBit(RHS.मोजणी_Min_Trailing_Zeros());
      Res = अंक.sdiv(Denom);
    }
  } अन्यथा यदि (LHS.isStrictlyPositive() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    यदि (Exact || LHS.get_Signed_Min_मूल्य().uge(-RHS.get_Signed_Min_मूल्य())) {
      APInt Denom = RHS.get_Signed_Max_मूल्य();
      APInt अंक = LHS.get_Signed_Max_मूल्य();
      Res = अंक.sdiv(Denom);
    }
  }

  यदि (Res) {
    यदि (Res->isNonNegative()) {
      unsigned LeadZ = Res->मोजणी_Leading_Zeros();
      Known.Zero.set_वरचा_Bits(LeadZ);
    } अन्यथा {
      unsigned LeadO = Res->मोजणी_Leading_Ones();
      Known.One.set_वरचा_Bits(LeadO);
    }
  }

  Known = div_Compute_खालचा_Bit(Known, LHS, RHS, Exact);
  प्रत्यागम Known;
}

KnownBits KnownBits::udiv(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS,
                          तार्किक Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  यदि (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    प्रत्यागम Known;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_मूल्य();
  यदि (MinDenom.isZero())
    MinDenom.setBit(RHS.मोजणी_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_मूल्य().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_मूल्य().udiv(RHS.get_Max_मूल्य());

  Known = KnownBits::makeConstant(MinRes);
  यदि (std::optional<unsigned> DifferentBit =
          APIntOps::GetMostSignificantDifferentBit(MinRes, MaxRes)) {
    Known.Zero.clear_खालचा_Bits(*DifferentBit + 1);
    Known.One.clear_खालचा_Bits(*DifferentBit + 1);
  }

  Known = div_Compute_खालचा_Bit(Known, LHS, RHS, Exact);

  प्रत्यागम Known;
}

KnownBits KnownBits::rem_Get_खालचा_Bits(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  यदि (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    unsigned RHSZeros = RHS.मोजणी_Min_Trailing_Zeros();
    APInt Mask = APInt::get_खालचा_Bits_Set(BitWidth, RHSZeros);
    APInt OnesMask = LHS.One & Mask;
    APInt ZerosMask = LHS.Zero & Mask;
    प्रत्यागम KnownBits(ZerosMask, OnesMask);
  }
  प्रत्यागम KnownBits(BitWidth);
}

KnownBits KnownBits::urem(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  यदि (LHS.get_Max_मूल्य().ult(RHS.get_Min_मूल्य()))
    प्रत्यागम LHS;

  KnownBits Known = rem_Get_खालचा_Bits(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_मूल्य(), RHS.get_Max_मूल्य() - 1);
  Known.Zero.set_वरचा_Bits(MaxRes.मोजणी_Leading_Zeros());
  प्रत्यागम Known;
}

KnownBits KnownBits::srem(ध्रुव KnownBits &LHS, ध्रुव KnownBits &RHS) {
  KnownBits Known = rem_Get_खालचा_Bits(LHS, RHS);
  यदि (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt खालचा_Bits = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    यदि (LHS.isNonNegative() || खालचा_Bits.isSubsetOf(LHS.Zero))
      Known.Zero |= ~खालचा_Bits;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    यदि (LHS.isNegative() && खालचा_Bits.intersects(LHS.One))
      Known.One |= ~खालचा_Bits;
    प्रत्यागम Known;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  यदि (LHS.isNegative() && Known.isNonZero())
    Known.One.set_वरचा_Bits(
        std::max(LHS.मोजणी_Min_Leading_Ones(), RHS.मोजणी_Min_Sign_Bits()));
  अन्यथा यदि (LHS.isNonNegative())
    Known.Zero.set_वरचा_Bits(
        std::max(LHS.मोजणी_Min_Leading_Zeros(), RHS.मोजणी_Min_Sign_Bits()));
  प्रत्यागम Known;
}

KnownBits &KnownBits::operator&=(ध्रुव KnownBits &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  प्रत्यागम *एतत्;
}

KnownBits &KnownBits::operator|=(ध्रुव KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  प्रत्यागम *एतत्;
}

KnownBits &KnownBits::operator^=(ध्रुव KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  प्रत्यागम *एतत्;
}

KnownBits KnownBits::blsi() ध्रुव {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(Zero, APInt(BitWidth, 0));
  unsigned Max = मोजणी_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = मोजणी_Min_Trailing_Zeros();
  यदि (Max == Min && Max < BitWidth)
    Known.One.setBit(Max);
  प्रत्यागम Known;
}

KnownBits KnownBits::blsmsk() ध्रुव {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(BitWidth);
  unsigned Max = मोजणी_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = मोजणी_Min_Trailing_Zeros();
  Known.One.set_खालचा_Bits(std::min(Min + 1, BitWidth));
  प्रत्यागम Known;
}

शून्य KnownBits::print(raw_ostream &OS) ध्रुव {
  unsigned BitWidth = getBitWidth();
  प्रति (unsigned I = 0; I < BitWidth; ++I) {
    unsigned N = BitWidth - I - 1;
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

#यदि !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य KnownBits::dump() ध्रुव {
  print(dbgs());
  dbgs() << "\n";
}
#endif

unsigned llvm::SignBitsOps::rot(unsigned SrcSignBits, unsigned BitWidth,
                                std::optional<APInt> RotAmt,
                                तार्किक Is_Rotate_उजवा) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  यदि (SrcSignBits == BitWidth)
    प्रत्यागम BitWidth;

  यदि (!RotAmt)
    प्रत्यागम 1;

  unsigned Amt = RotAmt->urem(BitWidth);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  यदि (Is_Rotate_उजवा)
    Amt = (BitWidth - Amt) % BitWidth;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  यदि (SrcSignBits > Amt + 1)
    प्रत्यागम SrcSignBits - Amt;
  प्रत्यागम 1;
}
