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

#شامل "llvm/Support/KnownBits.h"
#شامل "llvm/ADT/Sequence.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cassert>

using فضاینام llvm;

KnownBits KnownBits::flipSignBit(ثابت KnownBits &Val) {
  unsigned SignBitPosition = Val.getBitWidth() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
  One.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
  بازگشت KnownBits(Zero, One);
}

ایستا KnownBits computeForAddCarry(ثابت KnownBits &LHS, ثابت KnownBits &RHS,
                                    منطقی CarryZero, منطقی CarryOne) {

  APInt PossibleSumZero = LHS.get_Max_مقدار() + RHS.get_Max_مقدار() + !CarryZero;
  APInt PossibleSumOne = LHS.get_Min_مقدار() + RHS.get_Min_مقدار() + CarryOne;

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
  بازگشت KnownOut;
}

KnownBits KnownBits::computeForAddCarry(
    ثابت KnownBits &LHS, ثابت KnownBits &RHS, ثابت KnownBits &Carry) {
  أكد(Carry.getBitWidth() == 1 && "Carry must be 1-bit");
  بازگشت ::computeForAddCarry(
      LHS, RHS, Carry.Zero.get_Bool_مقدار(), Carry.One.get_Bool_مقدار());
}

KnownBits KnownBits::computeForAddSub(منطقی Add, منطقی NSW, منطقی NUW,
                                      ثابت KnownBits &LHS,
                                      ثابت KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits KnownOut(BitWidth);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  اگر (LHS.isUnknown() && RHS.isUnknown())
    بازگشت KnownOut;

  اگر (!LHS.isUnknown() && !RHS.isUnknown()) {
    اگر (Add) {
      // Sum = LHS + RHS + 0
      KnownOut = ::computeForAddCarry(LHS, RHS, /*CarryZero=*/درست,
                                      /*CarryOne=*/نادرست);
    } وگرنه {
      // Sum = LHS + ~RHS + 1
      KnownBits NotRHS = RHS;
      std::swap(NotRHS.Zero, NotRHS.One);
      KnownOut = ::computeForAddCarry(LHS, NotRHS, /*CarryZero=*/نادرست,
                                      /*CarryOne=*/درست);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  اگر (NUW) {
    اگر (Add) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_مقدار().uadd_sat(RHS.get_Min_مقدار());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      اگر (NSW) {
        unsigned رقم_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        KnownOut.One.setBits(BitWidth - 1 - رقم_Bits_2, BitWidth - 1);
      }
      KnownOut.One.set_بالا_Bits(MinVal.countl_one());
    } وگرنه {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_مقدار().usub_sat(RHS.get_Min_مقدار());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      اگر (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        unsigned رقم_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
        KnownOut.Zero.setBits(BitWidth - 1 - رقم_Bits_2, BitWidth - 1);
      }
      KnownOut.Zero.set_بالا_Bits(MaxVal.countl_zero());
    }
  }

  اگر (NSW) {
    APInt MinVal;
    APInt MaxVal;
    اگر (Add) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_مقدار().sadd_sat(RHS.get_Signed_Min_مقدار());
      MaxVal = LHS.get_Signed_Max_مقدار().sadd_sat(RHS.get_Signed_Max_مقدار());
    } وگرنه {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_مقدار().ssub_sat(RHS.get_Signed_Max_مقدار());
      MaxVal = LHS.get_Signed_Max_مقدار().ssub_sat(RHS.get_Signed_Min_مقدار());
    }
    اگر (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      unsigned رقم_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
      KnownOut.One.setBits(BitWidth - 1 - رقم_Bits_2, BitWidth - 1);
      KnownOut.Zero.setSignBit();
    }
    اگر (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      unsigned رقم_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
      KnownOut.Zero.setBits(BitWidth - 1 - رقم_Bits_2, BitWidth - 1);
      KnownOut.One.setSignBit();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  اگر (KnownOut.hasConflict())
    KnownOut.setAllZero();
  بازگشت KnownOut;
}

KnownBits KnownBits::computeForSubBorrow(ثابت KnownBits &LHS, KnownBits RHS,
                                         ثابت KnownBits &Borrow) {
  أكد(Borrow.getBitWidth() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::swap(RHS.Zero, RHS.One);
  بازگشت ::computeForAddCarry(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_مقدار(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_مقدار());
}

KnownBits KnownBits::truncSSat(unsigned BitWidth) ثابت {
  unsigned درون_داد_Bits = getBitWidth();
  APInt MinInRange = APInt::get_Signed_Min_مقدار(BitWidth).sext(درون_داد_Bits);
  APInt MaxInRange = APInt::get_Signed_Max_مقدار(BitWidth).sext(درون_داد_Bits);
  APInt درون_داد_Min = get_Signed_Min_مقدار();
  APInt درون_داد_Max = get_Signed_Max_مقدار();
  KnownBits Known(BitWidth);

  // Case 1: All values fit - just truncate
  اگر (درون_داد_Min.sge(MinInRange) && درون_داد_Max.sle(MaxInRange)) {
    Known = trunc(BitWidth);
  }
  // Case 2: All saturate to min
  وگرنه اگر (درون_داد_Max.slt(MinInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Min_مقدار(BitWidth));
  }
  // Case 3: All saturate to max
  وگرنه اگر (درون_داد_Min.sgt(MaxInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Max_مقدار(BitWidth));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  وگرنه اگر (درون_داد_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    Known.Zero = APInt(BitWidth, 0);
    Known.Zero.setBit(BitWidth - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    Known.One = One.trunc(BitWidth);
    Known.One.clearBit(BitWidth - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  وگرنه اگر (درون_داد_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    Known.Zero = Zero.trunc(BitWidth);
    Known.Zero.clearBit(BitWidth - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    Known.One = APInt(BitWidth, 0);
    Known.One.setBit(BitWidth - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  وگرنه {
    // Output: min saturation, truncated, or max saturation
    APInt درون_داد_Zero_Lower = Zero.trunc(BitWidth);
    APInt درون_داد_One_Lower = One.trunc(BitWidth);
    APInt MinSat = APInt::get_Signed_Min_مقدار(BitWidth);
    APInt MaxSat = APInt::get_Signed_Max_مقدار(BitWidth);
    KnownBits MinSatKB = KnownBits::makeConstant(MinSat);
    KnownBits MaxSatKB = KnownBits::makeConstant(MaxSat);
    اگر (درون_داد_Max.sle(MaxInRange)) {
      // Positive values fit, only negatives saturate to min
      Known.Zero = درون_داد_Zero_Lower & MinSatKB.Zero;
      Known.One = درون_داد_One_Lower & MinSatKB.One;
    } وگرنه اگر (درون_داد_Min.sge(MinInRange)) {
      // Negative values fit, only positives saturate to max
      Known.Zero = درون_داد_Zero_Lower & MaxSatKB.Zero;
      Known.One = درون_داد_One_Lower & MaxSatKB.One;
    } وگرنه {
      // Both positive and negative values might saturate
      Known.Zero = درون_داد_Zero_Lower & MinSatKB.Zero & MaxSatKB.Zero;
      Known.One = درون_داد_One_Lower & MinSatKB.One & MaxSatKB.One;
    }
  }
  بازگشت Known;
}

KnownBits KnownBits::truncSSatU(unsigned BitWidth) ثابت {
  unsigned درون_داد_Bits = getBitWidth();
  APInt MaxInRange = APInt::getAllOnes(BitWidth).zext(درون_داد_Bits);
  APInt درون_داد_Min = get_Signed_Min_مقدار();
  APInt درون_داد_Max = get_Signed_Max_مقدار();
  KnownBits Known(BitWidth);

  اگر (isNegative()) {
    Known.setAllZero();
  } وگرنه اگر (درون_داد_Min.isNonNegative() && درون_داد_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } وگرنه اگر (درون_داد_Min.isNonNegative() && درون_داد_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } وگرنه اگر (درون_داد_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    Known.Zero = APInt(BitWidth, 0);
    // Common one bits: bits that are 1 in input stay 1
    Known.One = One.trunc(BitWidth);
  } وگرنه {
    // Mixed: sign bit unknown
    اگر (درون_داد_Max.ule(MaxInRange)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      Known.Zero = Zero.trunc(BitWidth);
      Known.One = APInt(BitWidth, 0);
    } وگرنه {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      Known.Zero = APInt(BitWidth, 0);
      Known.One = APInt(BitWidth, 0);
    }
  }
  بازگشت Known;
}

KnownBits KnownBits::truncUSat(unsigned BitWidth) ثابت {
  unsigned درون_داد_Bits = getBitWidth();
  APInt MaxInRange = APInt::get_کم_Bits_Set(درون_داد_Bits, BitWidth);
  APInt درون_داد_Max = get_Max_مقدار();
  APInt درون_داد_Min = get_Min_مقدار();
  KnownBits Known(BitWidth);

  اگر (درون_داد_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } وگرنه اگر (درون_داد_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } وگرنه {
    Known.resetAll();
    Known.One = One.trunc(BitWidth);
  }
  بازگشت Known;
}

KnownBits KnownBits::sextInReg(unsigned SrcBitWidth) ثابت {
  unsigned BitWidth = getBitWidth();
  أكد(0 < SrcBitWidth && SrcBitWidth <= BitWidth &&
         "Illegal sext-in-register");

  اگر (SrcBitWidth == BitWidth)
    بازگشت *این;

  unsigned ExtBits = BitWidth - SrcBitWidth;
  KnownBits Result;
  Result.One = One << ExtBits;
  Result.Zero = Zero << ExtBits;
  Result.One.ashrInPlace(ExtBits);
  Result.Zero.ashrInPlace(ExtBits);
  بازگشت Result;
}

KnownBits KnownBits::makeGE(ثابت APInt &Val) ثابت {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  unsigned N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_کم_Bits(getBitWidth() - N);
  بازگشت KnownBits(Zero, One | MaskedVal);
}

KnownBits KnownBits::umax(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  اگر (LHS.get_Min_مقدار().uge(RHS.get_Max_مقدار()))
    بازگشت LHS;
  اگر (RHS.get_Min_مقدار().uge(LHS.get_Max_مقدار()))
    بازگشت RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  KnownBits L = LHS.makeGE(RHS.get_Min_مقدار());
  KnownBits R = RHS.makeGE(LHS.get_Min_مقدار());
  بازگشت L.intersectWith(R);
}

KnownBits KnownBits::umin(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](ثابت KnownBits &Val) { بازگشت KnownBits(Val.One, Val.Zero); };
  بازگشت Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::smax(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  بازگشت flipSignBit(umax(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::smin(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](ثابت KnownBits &Val) {
    unsigned SignBitPosition = Val.getBitWidth() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
    One.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
    بازگشت KnownBits(Zero, One);
  };
  بازگشت Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::abdu(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  اگر (LHS.get_Min_مقدار().uge(RHS.get_Max_مقدار()))
    بازگشت computeForAddSub(/*Add=*/نادرست, /*NSW=*/نادرست, /*NUW=*/نادرست, LHS,
                            RHS);
  اگر (RHS.get_Min_مقدار().uge(LHS.get_Max_مقدار()))
    بازگشت computeForAddSub(/*Add=*/نادرست, /*NSW=*/نادرست, /*NUW=*/نادرست, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/نادرست, /*NSW=*/نادرست, /*NUW=*/درست, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/نادرست, /*NSW=*/نادرست, /*NUW=*/درست, RHS, LHS);
  بازگشت Diff0.intersectWith(Diff1);
}

KnownBits KnownBits::abds(KnownBits LHS, KnownBits RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  اگر (LHS.get_Signed_Min_مقدار().sge(RHS.get_Signed_Max_مقدار()))
    بازگشت computeForAddSub(/*Add=*/نادرست, /*NSW=*/نادرست, /*NUW=*/نادرست, LHS,
                            RHS);
  اگر (RHS.get_Signed_Min_مقدار().sge(LHS.get_Signed_Max_مقدار()))
    بازگشت computeForAddSub(/*Add=*/نادرست, /*NSW=*/نادرست, /*NUW=*/نادرست, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  unsigned SignBitPosition = LHS.getBitWidth() - 1;
  برای (auto Arg : {&LHS, &RHS}) {
    منطقی Tmp = Arg->Zero[SignBitPosition];
    Arg->Zero.setBitVal(SignBitPosition, Arg->One[SignBitPosition]);
    Arg->One.setBitVal(SignBitPosition, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/نادرست, /*NSW=*/نادرست, /*NUW=*/درست, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/نادرست, /*NSW=*/نادرست, /*NUW=*/درست, RHS, LHS);
  بازگشت Diff0.intersectWith(Diff1);
}

ایستا unsigned getMaxShiftAmount(ثابت APInt &Max_مقدار, unsigned BitWidth) {
  اگر (isPowerOf2_32(BitWidth))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    بازگشت Max_مقدار.extract_Bits_As_ZExt_مقدار(
        std::min(Log2_32(BitWidth), Max_مقدار.getBitWidth()), 0);
  // This is only an approximate upper bound.
  بازگشت Max_مقدار.get_Limited_مقدار(BitWidth - 1);
}

KnownBits KnownBits::shl(ثابت KnownBits &LHS, ثابت KnownBits &RHS, منطقی NUW,
                         منطقی NSW, منطقی ShAmtNonZero) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ثابت KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known;
    منطقی ShiftedOutZero, ShiftedOutOne;
    Known.Zero = LHS.Zero.ushl_ov(ShiftAmt, ShiftedOutZero);
    Known.Zero.set_کم_Bits(ShiftAmt);
    Known.One = LHS.One.ushl_ov(ShiftAmt, ShiftedOutOne);

    // All cases returning poison have been handled by MaxShiftAmount already.
    اگر (NSW) {
      اگر (NUW && ShiftAmt != 0)
        // NUW means we can assume anything shifted out was a zero.
        ShiftedOutZero = درست;

      اگر (ShiftedOutZero)
        Known.makeNonNegative();
      وگرنه اگر (ShiftedOutOne)
        Known.makeNegative();
    }
    بازگشت Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_مقدار().get_Limited_مقدار(BitWidth);
  اگر (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  اگر (LHS.isUnknown()) {
    Known.Zero.set_کم_Bits(MinShiftAmount);
    اگر (NUW && NSW && MinShiftAmount != 0)
      Known.makeNonNegative();
    بازگشت Known;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_مقدار = RHS.get_Max_مقدار();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_مقدار, BitWidth);
  اگر (NUW && NSW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.شمار_Max_Leading_Zeros() - 1);
  اگر (NUW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.شمار_Max_Leading_Zeros());
  اگر (NSW)
    MaxShiftAmount = std::min(
        MaxShiftAmount,
        std::max(LHS.شمار_Max_Leading_Zeros(), LHS.شمار_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  اگر (MinShiftAmount == 0 && MaxShiftAmount == BitWidth - 1 &&
      isPowerOf2_32(BitWidth)) {
    Known.Zero.set_کم_Bits(LHS.شمار_Min_Trailing_Zeros());
    اگر (LHS.isAllOnes())
      Known.One.setSignBit();
    اگر (NSW) {
      اگر (LHS.isNonNegative())
        Known.makeNonNegative();
      اگر (LHS.isNegative())
        Known.makeNegative();
    }
    بازگشت Known;
  }

  // Find the common bits from all possible shifts.
  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_مقدار();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_مقدار();
  Known.setAllConflict();
  برای (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    اگر ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      ادامه;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    اگر (Known.isUnknown())
      بشکن;
  }

  // All shift amounts may result in poison.
  اگر (Known.hasConflict())
    Known.setAllZero();
  بازگشت Known;
}

KnownBits KnownBits::lshr(ثابت KnownBits &LHS, ثابت KnownBits &RHS,
                          منطقی ShAmtNonZero, منطقی Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ثابت KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known >>= ShiftAmt;
    // High bits are known zero.
    Known.Zero.set_بالا_Bits(ShiftAmt);
    بازگشت Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_مقدار().get_Limited_مقدار(BitWidth);
  اگر (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  اگر (LHS.isUnknown()) {
    Known.Zero.set_بالا_Bits(MinShiftAmount);
    بازگشت Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_مقدار = RHS.get_Max_مقدار();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_مقدار, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  اگر (Exact) {
    unsigned FirstOne = LHS.شمار_Max_Trailing_Zeros();
    اگر (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      بازگشت Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_مقدار();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_مقدار();
  Known.setAllConflict();
  برای (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    اگر ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      ادامه;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    اگر (Known.isUnknown())
      بشکن;
  }

  // All shift amounts may result in poison.
  اگر (Known.hasConflict())
    Known.setAllZero();
  بازگشت Known;
}

KnownBits KnownBits::ashr(ثابت KnownBits &LHS, ثابت KnownBits &RHS,
                          منطقی ShAmtNonZero, منطقی Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ثابت KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known.Zero.ashrInPlace(ShiftAmt);
    Known.One.ashrInPlace(ShiftAmt);
    بازگشت Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_مقدار().get_Limited_مقدار(BitWidth);
  اگر (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  اگر (LHS.isUnknown()) {
    اگر (MinShiftAmount == BitWidth) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      بازگشت Known;
    }
    بازگشت Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_مقدار = RHS.get_Max_مقدار();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_مقدار, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  اگر (Exact) {
    unsigned FirstOne = LHS.شمار_Max_Trailing_Zeros();
    اگر (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      بازگشت Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_مقدار();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_مقدار();
  Known.setAllConflict();
  برای (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
      ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    اگر ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      ادامه;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    اگر (Known.isUnknown())
      بشکن;
  }

  // All shift amounts may result in poison.
  اگر (Known.hasConflict())
    Known.setAllZero();
  بازگشت Known;
}

KnownBits KnownBits::fshl(ثابت KnownBits &LHS, ثابت KnownBits &RHS,
                          ثابت APInt &Amt) {
  بازگشت KnownBits(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::fshr(ثابت KnownBits &LHS, ثابت KnownBits &RHS,
                          ثابت APInt &Amt) {
  بازگشت KnownBits(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::clmul(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  KnownBits Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_مقدار(), RHS.get_Min_مقدار()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  auto ClMulOr = [](ثابت APInt &LHS, ثابت APInt &RHS) {
    unsigned BW = LHS.getBitWidth();
    أكد(BW == RHS.getBitWidth() && "Operand mismatch");
    APInt Result(BW, 0);
    برای (unsigned I :
         seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
      اگر (RHS[I])
        Result |= LHS << I;
    بازگشت Result;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt Known = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= Known;
  Res.One &= Known;

  بازگشت Res;
}

KnownBits KnownBits::pext(ثابت KnownBits &Val, ثابت KnownBits &Mask) {
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
  برای (unsigned I = 0; I < BitWidth; ++I) {
    اگر (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      اگر (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clearBits(M0, M1 + 1);
      اگر (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clearBits(M0, M1 + 1);
    }
    اگر (Mask.One[I])
      ++M0, ++M1;
    وگرنه اگر (!Mask.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clearBits(M0, BitWidth);
  بازگشت Res;
}

KnownBits KnownBits::pdep(ثابت KnownBits &Val, ثابت KnownBits &Mask) {
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
  برای (unsigned I = 0; I < BitWidth; ++I) {
    اگر (!Mask.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clearBit(I);
    اگر (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::getBitsSet(BitWidth, M0, M1 + 1);
      اگر (!Range.isSubsetOf(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clearBit(I);
      اگر (!Range.isSubsetOf(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clearBit(I);
    }
    اگر (Mask.One[I])
      ++M0, ++M1;
    وگرنه اگر (!Mask.Zero[I])
      ++M1;
  }
  بازگشت Res;
}

std::optional<منطقی> KnownBits::eq(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  اگر (LHS.isConstant() && RHS.isConstant())
    بازگشت LHS.getConstant() == RHS.getConstant();
  اگر (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    بازگشت نادرست;
  بازگشت std::nullopt;
}

std::optional<منطقی> KnownBits::ne(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  اگر (std::optional<منطقی> KnownEQ = eq(LHS, RHS))
    بازگشت !*KnownEQ;
  بازگشت std::nullopt;
}

std::optional<منطقی> KnownBits::ugt(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  اگر (LHS.get_Max_مقدار().ule(RHS.get_Min_مقدار()))
    بازگشت نادرست;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  اگر (LHS.get_Min_مقدار().ugt(RHS.get_Max_مقدار()))
    بازگشت درست;
  بازگشت std::nullopt;
}

std::optional<منطقی> KnownBits::uge(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  اگر (std::optional<منطقی> IsUGT = ugt(RHS, LHS))
    بازگشت !*IsUGT;
  بازگشت std::nullopt;
}

std::optional<منطقی> KnownBits::ult(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  بازگشت ugt(RHS, LHS);
}

std::optional<منطقی> KnownBits::ule(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  بازگشت uge(RHS, LHS);
}

std::optional<منطقی> KnownBits::sgt(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  اگر (LHS.get_Signed_Max_مقدار().sle(RHS.get_Signed_Min_مقدار()))
    بازگشت نادرست;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  اگر (LHS.get_Signed_Min_مقدار().sgt(RHS.get_Signed_Max_مقدار()))
    بازگشت درست;
  بازگشت std::nullopt;
}

std::optional<منطقی> KnownBits::sge(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  اگر (std::optional<منطقی> KnownSGT = sgt(RHS, LHS))
    بازگشت !*KnownSGT;
  بازگشت std::nullopt;
}

std::optional<منطقی> KnownBits::slt(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  بازگشت sgt(RHS, LHS);
}

std::optional<منطقی> KnownBits::sle(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  بازگشت sge(RHS, LHS);
}

KnownBits KnownBits::abs(منطقی IntMinIsPoison) ثابت {
  // If the source's MSB is zero then we know the rest of the bits already.
  اگر (isNonNegative())
    بازگشت *این;

  // Absolute value preserves trailing zero count.
  KnownBits KnownAbs(getBitWidth());

  // If the input is negative, then abs(x) == -x.
  اگر (isNegative()) {
    KnownBits Tmp = *این;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    اگر (IntMinIsPoison && (Zero.popcount() + 2) == getBitWidth())
      Tmp.One.setBit(شمار_Min_Trailing_Zeros());

    KnownAbs = computeForAddSub(
        /*Add*/ نادرست, IntMinIsPoison, /*NUW=*/نادرست,
        KnownBits::makeConstant(APInt(getBitWidth(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    اگر (IntMinIsPoison && Tmp.شمار_Min_Population() == 1 &&
        Tmp.شمار_Max_Population() != 1) {
      Tmp.One.clearSignBit();
      Tmp.Zero.setSignBit();
      KnownAbs.One.setBits(getBitWidth() - Tmp.شمار_Min_Leading_Zeros(),
                           getBitWidth() - 1);
    }

  } وگرنه {
    unsigned MaxTZ = شمار_Max_Trailing_Zeros();
    unsigned MinTZ = شمار_Min_Trailing_Zeros();

    KnownAbs.Zero.set_کم_Bits(MinTZ);
    // If we know the lowest set 1, then preserve it.
    اگر (MaxTZ == MinTZ && MaxTZ < getBitWidth())
      KnownAbs.One.setBit(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    اگر (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_مقدار())) {
      KnownAbs.One.clearSignBit();
      KnownAbs.Zero.setSignBit();
    }
  }

  بازگشت KnownAbs;
}

KnownBits KnownBits::reduceAdd(unsigned رقم_Elts) ثابت {
  اگر (رقم_Elts == 0)
    بازگشت KnownBits(getBitWidth());

  unsigned BitWidth = getBitWidth();
  KnownBits Result(BitWidth);

  اگر (isConstant())
    // If all elements are the same constant, we can simply compute it
    بازگشت KnownBits::makeConstant(رقم_Elts * getConstant());

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
  unsigned LostBits = Log2_32_Ceil(رقم_Elts);

  اگر (isNonNegative()) {
    unsigned LeadingZeros = شمار_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > LostBits ? LeadingZeros - LostBits : 0;
    Result.Zero.set_بالا_Bits(LeadingZeros);
  } وگرنه اگر (isNegative()) {
    unsigned LeadingOnes = شمار_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > LostBits ? LeadingOnes - LostBits : 0;
    Result.One.set_بالا_Bits(LeadingOnes);
  }

  بازگشت Result;
}

ایستا KnownBits computeForSatAddSub(منطقی Add, منطقی Signed,
                                     ثابت KnownBits &LHS,
                                     ثابت KnownBits &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  unsigned BitWidth = LHS.getBitWidth();

  std::optional<منطقی> Overflow;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  منطقی MayNegClamp = درست;
  منطقی MayPosClamp = درست;
  اگر (Signed) {
    // Easy cases we can rule out any overflow.
    اگر (Add && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      Overflow = نادرست;
    وگرنه اگر (!Add && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      Overflow = نادرست;
    وگرنه {
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
          KnownBits::computeForAddSub(Add, /*NSW=*/نادرست,
                                      /*NUW=*/نادرست, UnsignedLHS, UnsignedRHS);
      اگر (Add) {
        اگر (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = نادرست;
          // Pos + Pos will overflow with extra signbit.
          اگر (LHS.isNonNegative() && RHS.isNonNegative())
            Overflow = درست;
        } وگرنه اگر (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = نادرست;
          // Neg + Neg will overflow without extra signbit.
          اگر (LHS.isNegative() && RHS.isNegative())
            Overflow = درست;
        }
        // We will never clamp to the opposite sign of N-bit result.
        اگر (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = نادرست;
        اگر (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = نادرست;
      } وگرنه {
        اگر (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = نادرست;
          // Neg - Pos will overflow with extra signbit.
          اگر (LHS.isNegative() && RHS.isNonNegative())
            Overflow = درست;
        } وگرنه اگر (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = نادرست;
          // Pos - Neg will overflow without extra signbit.
          اگر (LHS.isNonNegative() && RHS.isNegative())
            Overflow = درست;
        }
        // We will never clamp to the opposite sign of N-bit result.
        اگر (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = نادرست;
        اگر (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = نادرست;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    اگر (!MayNegClamp && !MayPosClamp)
      Overflow = نادرست;
  } وگرنه اگر (Add) {
    // uadd.sat
    منطقی Of;
    (تهی)LHS.get_Max_مقدار().uadd_ov(RHS.get_Max_مقدار(), Of);
    اگر (!Of) {
      Overflow = نادرست;
    } وگرنه {
      (تهی)LHS.get_Min_مقدار().uadd_ov(RHS.get_Min_مقدار(), Of);
      اگر (Of)
        Overflow = درست;
    }
  } وگرنه {
    // usub.sat
    منطقی Of;
    (تهی)LHS.get_Min_مقدار().usub_ov(RHS.get_Max_مقدار(), Of);
    اگر (!Of) {
      Overflow = نادرست;
    } وگرنه {
      (تهی)LHS.get_Max_مقدار().usub_ov(RHS.get_Min_مقدار(), Of);
      اگر (Of)
        Overflow = درست;
    }
  }

  KnownBits Res = KnownBits::computeForAddSub(Add, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  اگر (Overflow) {
    // We know whether or not we overflowed.
    اگر (!(*Overflow)) {
      // No overflow.
      بازگشت Res;
    }

    // We overflowed
    APInt C;
    اگر (Signed) {
      // sadd.sat / ssub.sat
      أكد(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_مقدار(BitWidth)
                           : APInt::get_Signed_Max_مقدار(BitWidth);
    } وگرنه اگر (Add) {
      // uadd.sat
      C = APInt::get_Max_مقدار(BitWidth);
    } وگرنه {
      // uadd.sat
      C = APInt::get_Min_مقدار(BitWidth);
    }

    Res.One = C;
    Res.Zero = ~C;
    بازگشت Res;
  }

  // We don't know if we overflowed.
  اگر (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    اگر (MayPosClamp)
      Res.Zero.clear_کم_Bits(BitWidth - 1);
    اگر (MayNegClamp)
      Res.One.clear_کم_Bits(BitWidth - 1);
  } وگرنه اگر (Add) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clearAllBits();
  } وگرنه {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clearAllBits();
  }

  بازگشت Res;
}

KnownBits KnownBits::sadd_sat(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  بازگشت computeForSatAddSub(/*Add*/ درست, /*Signed*/ درست, LHS, RHS);
}
KnownBits KnownBits::ssub_sat(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  بازگشت computeForSatAddSub(/*Add*/ نادرست, /*Signed*/ درست, LHS, RHS);
}
KnownBits KnownBits::uadd_sat(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  بازگشت computeForSatAddSub(/*Add*/ درست, /*Signed*/ نادرست, LHS, RHS);
}
KnownBits KnownBits::usub_sat(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  بازگشت computeForSatAddSub(/*Add*/ نادرست, /*Signed*/ نادرست, LHS, RHS);
}

ایستا KnownBits avgComputeU(KnownBits LHS, KnownBits RHS, منطقی IsCeil) {
  unsigned BitWidth = LHS.getBitWidth();
  LHS = LHS.zext(BitWidth + 1);
  RHS = RHS.zext(BitWidth + 1);
  LHS =
      computeForAddCarry(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.extractBits(BitWidth, 1);
  بازگشت LHS;
}

KnownBits KnownBits::avgFloorS(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  بازگشت flipSignBit(avgFloorU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgFloorU(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  بازگشت avgComputeU(LHS, RHS, /*IsCeil=*/نادرست);
}

KnownBits KnownBits::avgCeilS(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  بازگشت flipSignBit(avgCeilU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgCeilU(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  بازگشت avgComputeU(LHS, RHS, /*IsCeil=*/درست);
}

KnownBits KnownBits::mul(ثابت KnownBits &LHS, ثابت KnownBits &RHS,
                         منطقی NoUndefSelfMultiply) {
  unsigned BitWidth = LHS.getBitWidth();
  أكد(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  أكد((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.get_Max_مقدار();
  APInt UMaxRHS = RHS.get_Max_مقدار();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  منطقی HasOverflow;
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
  unsigned TrailZeroLHS = LHS.شمار_Min_Trailing_Zeros();
  unsigned TrailZeroRHS = RHS.شمار_Min_Trailing_Zeros();
  unsigned TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  unsigned SmallestOperand = std::min(TrailBitsKnownLHS - TrailZeroLHS,
                                      TrailBitsKnownRHS - TrailZeroRHS);
  unsigned ResultBitsKnown = std::min(SmallestOperand + TrailZ, BitWidth);

  // The lower ResultBitsKnown bits of this are known.
  APInt BottomKnown = LHS.One * RHS.One;

  KnownBits Res(BitWidth);
  Res.Zero.set_بالا_Bits(LeadZ);
  Res.Zero |= (~BottomKnown).getLoBits(ResultBitsKnown);
  Res.One = BottomKnown.getLoBits(ResultBitsKnown);

  اگر (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    unsigned TwoTZP1 = 2 * TrailZeroLHS + 1;
    اگر (TwoTZP1 < BitWidth)
      Res.Zero.setBit(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    اگر (TrailZeroLHS < BitWidth && LHS.One[TrailZeroLHS]) {
      unsigned TwoTZP2 = TwoTZP1 + 1;
      اگر (TwoTZP2 < BitWidth)
        Res.Zero.setBit(TwoTZP2);
    }
  }

  بازگشت Res;
}

KnownBits KnownBits::mulhs(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  أكد(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.sext(2 * BitWidth);
  KnownBits WideRHS = RHS.sext(2 * BitWidth);
  بازگشت mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

KnownBits KnownBits::mulhu(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  أكد(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.zext(2 * BitWidth);
  KnownBits WideRHS = RHS.zext(2 * BitWidth);
  بازگشت mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

ایستا KnownBits div_Compute_کم_Bit(KnownBits Known, ثابت KnownBits &LHS,
                                  ثابت KnownBits &RHS, منطقی Exact) {

  اگر (!Exact)
    بازگشت Known;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  اگر (LHS.One[0])
    Known.One.setBit(0);

  صحیح MinTZ =
      (صحیح)LHS.شمار_Min_Trailing_Zeros() - (صحیح)RHS.شمار_Max_Trailing_Zeros();
  صحیح MaxTZ =
      (صحیح)LHS.شمار_Max_Trailing_Zeros() - (صحیح)RHS.شمار_Min_Trailing_Zeros();
  اگر (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    Known.Zero.set_کم_Bits(MinTZ);
    اگر (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      Known.One.setBit(MinTZ);
    }
  } وگرنه اگر (MaxTZ < 0) {
    // Poison Result
    Known.setAllZero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  اگر (Known.hasConflict())
    Known.setAllZero();

  بازگشت Known;
}

KnownBits KnownBits::sdiv(ثابت KnownBits &LHS, ثابت KnownBits &RHS,
                          منطقی Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  اگر (LHS.isNonNegative() && RHS.isNonNegative())
    بازگشت udiv(LHS, RHS, Exact);

  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  اگر (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    بازگشت Known;
  }

  std::optional<APInt> Res;
  اگر (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_مقدار();
    APInt رقم = LHS.get_Signed_Min_مقدار();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (رقم.is_Min_Signed_مقدار() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_مقدار(BitWidth)
              : رقم.sdiv(Denom);
  } وگرنه اگر (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    اگر (Exact || (-LHS.get_Signed_Max_مقدار()).uge(RHS.get_Signed_Max_مقدار())) {
      APInt Denom = RHS.get_Signed_Min_مقدار();
      APInt رقم = LHS.get_Signed_Min_مقدار();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      اگر (Denom.isZero())
        Denom.setBit(RHS.شمار_Min_Trailing_Zeros());
      Res = رقم.sdiv(Denom);
    }
  } وگرنه اگر (LHS.isStrictlyPositive() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    اگر (Exact || LHS.get_Signed_Min_مقدار().uge(-RHS.get_Signed_Min_مقدار())) {
      APInt Denom = RHS.get_Signed_Max_مقدار();
      APInt رقم = LHS.get_Signed_Max_مقدار();
      Res = رقم.sdiv(Denom);
    }
  }

  اگر (Res) {
    اگر (Res->isNonNegative()) {
      unsigned LeadZ = Res->شمار_Leading_Zeros();
      Known.Zero.set_بالا_Bits(LeadZ);
    } وگرنه {
      unsigned LeadO = Res->شمار_Leading_Ones();
      Known.One.set_بالا_Bits(LeadO);
    }
  }

  Known = div_Compute_کم_Bit(Known, LHS, RHS, Exact);
  بازگشت Known;
}

KnownBits KnownBits::udiv(ثابت KnownBits &LHS, ثابت KnownBits &RHS,
                          منطقی Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  اگر (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    بازگشت Known;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_مقدار();
  اگر (MinDenom.isZero())
    MinDenom.setBit(RHS.شمار_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_مقدار().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_مقدار().udiv(RHS.get_Max_مقدار());

  Known = KnownBits::makeConstant(MinRes);
  اگر (std::optional<unsigned> DifferentBit =
          APIntOps::GetMostSignificantDifferentBit(MinRes, MaxRes)) {
    Known.Zero.clear_کم_Bits(*DifferentBit + 1);
    Known.One.clear_کم_Bits(*DifferentBit + 1);
  }

  Known = div_Compute_کم_Bit(Known, LHS, RHS, Exact);

  بازگشت Known;
}

KnownBits KnownBits::rem_Get_کم_Bits(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  اگر (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    unsigned RHSZeros = RHS.شمار_Min_Trailing_Zeros();
    APInt Mask = APInt::get_کم_Bits_Set(BitWidth, RHSZeros);
    APInt OnesMask = LHS.One & Mask;
    APInt ZerosMask = LHS.Zero & Mask;
    بازگشت KnownBits(ZerosMask, OnesMask);
  }
  بازگشت KnownBits(BitWidth);
}

KnownBits KnownBits::urem(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  اگر (LHS.get_Max_مقدار().ult(RHS.get_Min_مقدار()))
    بازگشت LHS;

  KnownBits Known = rem_Get_کم_Bits(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_مقدار(), RHS.get_Max_مقدار() - 1);
  Known.Zero.set_بالا_Bits(MaxRes.شمار_Leading_Zeros());
  بازگشت Known;
}

KnownBits KnownBits::srem(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  KnownBits Known = rem_Get_کم_Bits(LHS, RHS);
  اگر (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt کم_Bits = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    اگر (LHS.isNonNegative() || کم_Bits.isSubsetOf(LHS.Zero))
      Known.Zero |= ~کم_Bits;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    اگر (LHS.isNegative() && کم_Bits.intersects(LHS.One))
      Known.One |= ~کم_Bits;
    بازگشت Known;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  اگر (LHS.isNegative() && Known.isNonZero())
    Known.One.set_بالا_Bits(
        std::max(LHS.شمار_Min_Leading_Ones(), RHS.شمار_Min_Sign_Bits()));
  وگرنه اگر (LHS.isNonNegative())
    Known.Zero.set_بالا_Bits(
        std::max(LHS.شمار_Min_Leading_Zeros(), RHS.شمار_Min_Sign_Bits()));
  بازگشت Known;
}

KnownBits &KnownBits::operator&=(ثابت KnownBits &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  بازگشت *این;
}

KnownBits &KnownBits::operator|=(ثابت KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  بازگشت *این;
}

KnownBits &KnownBits::operator^=(ثابت KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  بازگشت *این;
}

KnownBits KnownBits::blsi() ثابت {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(Zero, APInt(BitWidth, 0));
  unsigned Max = شمار_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = شمار_Min_Trailing_Zeros();
  اگر (Max == Min && Max < BitWidth)
    Known.One.setBit(Max);
  بازگشت Known;
}

KnownBits KnownBits::blsmsk() ثابت {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(BitWidth);
  unsigned Max = شمار_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = شمار_Min_Trailing_Zeros();
  Known.One.set_کم_Bits(std::min(Min + 1, BitWidth));
  بازگشت Known;
}

تهی KnownBits::print(raw_ostream &OS) ثابت {
  unsigned BitWidth = getBitWidth();
  برای (unsigned I = 0; I < BitWidth; ++I) {
    unsigned N = BitWidth - I - 1;
    اگر (Zero[N] && One[N])
      OS << "!";
    وگرنه اگر (Zero[N])
      OS << "0";
    وگرنه اگر (One[N])
      OS << "1";
    وگرنه
      OS << "?";
  }
}

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD تهی KnownBits::dump() ثابت {
  print(dbgs());
  dbgs() << "\n";
}
#endif

unsigned llvm::SignBitsOps::rot(unsigned SrcSignBits, unsigned BitWidth,
                                std::optional<APInt> RotAmt,
                                منطقی Is_Rotate_راست) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  اگر (SrcSignBits == BitWidth)
    بازگشت BitWidth;

  اگر (!RotAmt)
    بازگشت 1;

  unsigned Amt = RotAmt->urem(BitWidth);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  اگر (Is_Rotate_راست)
    Amt = (BitWidth - Amt) % BitWidth;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  اگر (SrcSignBits > Amt + 1)
    بازگشت SrcSignBits - Amt;
  بازگشت 1;
}
