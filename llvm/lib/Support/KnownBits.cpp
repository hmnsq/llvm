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

#ضمن "llvm/Support/KnownBits.h"
#ضمن "llvm/ADT/Sequence.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cassert>

using نطاق llvm;

KnownBits KnownBits::flipSignBit(ثابت KnownBits &Val) {
  unsigned SignBitPosition = Val.getBitWidth() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
  One.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
  أرجع KnownBits(Zero, One);
}

ساكن KnownBits computeForAddCarry(ثابت KnownBits &LHS, ثابت KnownBits &RHS,
                                    منطقي CarryZero, منطقي CarryOne) {

  APInt PossibleSumZero = LHS.get_Max_قيمة() + RHS.get_Max_قيمة() + !CarryZero;
  APInt PossibleSumOne = LHS.get_Min_قيمة() + RHS.get_Min_قيمة() + CarryOne;

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
  أرجع KnownOut;
}

KnownBits KnownBits::computeForAddCarry(
    ثابت KnownBits &LHS, ثابت KnownBits &RHS, ثابت KnownBits &Carry) {
  أكد(Carry.getBitWidth() == 1 && "Carry must be 1-bit");
  أرجع ::computeForAddCarry(
      LHS, RHS, Carry.Zero.get_Bool_قيمة(), Carry.One.get_Bool_قيمة());
}

KnownBits KnownBits::computeForAddSub(منطقي Add, منطقي NSW, منطقي NUW,
                                      ثابت KnownBits &LHS,
                                      ثابت KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits KnownOut(BitWidth);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  إذا (LHS.isUnknown() && RHS.isUnknown())
    أرجع KnownOut;

  إذا (!LHS.isUnknown() && !RHS.isUnknown()) {
    إذا (Add) {
      // Sum = LHS + RHS + 0
      KnownOut = ::computeForAddCarry(LHS, RHS, /*CarryZero=*/true,
                                      /*CarryOne=*/خطأ);
    } وإلا {
      // Sum = LHS + ~RHS + 1
      KnownBits NotRHS = RHS;
      std::swap(NotRHS.Zero, NotRHS.One);
      KnownOut = ::computeForAddCarry(LHS, NotRHS, /*CarryZero=*/خطأ,
                                      /*CarryOne=*/true);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  إذا (NUW) {
    إذا (Add) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_قيمة().uadd_sat(RHS.get_Min_قيمة());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      إذا (NSW) {
        unsigned رقم_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        KnownOut.One.setBits(BitWidth - 1 - رقم_Bits_2, BitWidth - 1);
      }
      KnownOut.One.set_عالي_Bits(MinVal.countl_one());
    } وإلا {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_قيمة().usub_sat(RHS.get_Min_قيمة());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      إذا (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        unsigned رقم_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
        KnownOut.Zero.setBits(BitWidth - 1 - رقم_Bits_2, BitWidth - 1);
      }
      KnownOut.Zero.set_عالي_Bits(MaxVal.countl_zero());
    }
  }

  إذا (NSW) {
    APInt MinVal;
    APInt MaxVal;
    إذا (Add) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_قيمة().sadd_sat(RHS.get_Signed_Min_قيمة());
      MaxVal = LHS.get_Signed_Max_قيمة().sadd_sat(RHS.get_Signed_Max_قيمة());
    } وإلا {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_قيمة().ssub_sat(RHS.get_Signed_Max_قيمة());
      MaxVal = LHS.get_Signed_Max_قيمة().ssub_sat(RHS.get_Signed_Min_قيمة());
    }
    إذا (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      unsigned رقم_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
      KnownOut.One.setBits(BitWidth - 1 - رقم_Bits_2, BitWidth - 1);
      KnownOut.Zero.setSignBit();
    }
    إذا (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      unsigned رقم_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
      KnownOut.Zero.setBits(BitWidth - 1 - رقم_Bits_2, BitWidth - 1);
      KnownOut.One.setSignBit();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  إذا (KnownOut.hasConflict())
    KnownOut.setAllZero();
  أرجع KnownOut;
}

KnownBits KnownBits::computeForSubBorrow(ثابت KnownBits &LHS, KnownBits RHS,
                                         ثابت KnownBits &Borrow) {
  أكد(Borrow.getBitWidth() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::swap(RHS.Zero, RHS.One);
  أرجع ::computeForAddCarry(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_قيمة(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_قيمة());
}

KnownBits KnownBits::truncSSat(unsigned BitWidth) ثابت {
  unsigned مدخل_Bits = getBitWidth();
  APInt MinInRange = APInt::get_Signed_Min_قيمة(BitWidth).sext(مدخل_Bits);
  APInt MaxInRange = APInt::get_Signed_Max_قيمة(BitWidth).sext(مدخل_Bits);
  APInt مدخل_Min = get_Signed_Min_قيمة();
  APInt مدخل_Max = get_Signed_Max_قيمة();
  KnownBits Known(BitWidth);

  // Case 1: All values fit - just truncate
  إذا (مدخل_Min.sge(MinInRange) && مدخل_Max.sle(MaxInRange)) {
    Known = trunc(BitWidth);
  }
  // Case 2: All saturate to min
  وإلا إذا (مدخل_Max.slt(MinInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Min_قيمة(BitWidth));
  }
  // Case 3: All saturate to max
  وإلا إذا (مدخل_Min.sgt(MaxInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Max_قيمة(BitWidth));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  وإلا إذا (مدخل_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    Known.Zero = APInt(BitWidth, 0);
    Known.Zero.setBit(BitWidth - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    Known.One = One.trunc(BitWidth);
    Known.One.clearBit(BitWidth - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  وإلا إذا (مدخل_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    Known.Zero = Zero.trunc(BitWidth);
    Known.Zero.clearBit(BitWidth - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    Known.One = APInt(BitWidth, 0);
    Known.One.setBit(BitWidth - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  وإلا {
    // Output: min saturation, truncated, or max saturation
    APInt مدخل_Zero_Lower = Zero.trunc(BitWidth);
    APInt مدخل_One_Lower = One.trunc(BitWidth);
    APInt MinSat = APInt::get_Signed_Min_قيمة(BitWidth);
    APInt MaxSat = APInt::get_Signed_Max_قيمة(BitWidth);
    KnownBits MinSatKB = KnownBits::makeConstant(MinSat);
    KnownBits MaxSatKB = KnownBits::makeConstant(MaxSat);
    إذا (مدخل_Max.sle(MaxInRange)) {
      // Positive values fit, only negatives saturate to min
      Known.Zero = مدخل_Zero_Lower & MinSatKB.Zero;
      Known.One = مدخل_One_Lower & MinSatKB.One;
    } وإلا إذا (مدخل_Min.sge(MinInRange)) {
      // Negative values fit, only positives saturate to max
      Known.Zero = مدخل_Zero_Lower & MaxSatKB.Zero;
      Known.One = مدخل_One_Lower & MaxSatKB.One;
    } وإلا {
      // Both positive and negative values might saturate
      Known.Zero = مدخل_Zero_Lower & MinSatKB.Zero & MaxSatKB.Zero;
      Known.One = مدخل_One_Lower & MinSatKB.One & MaxSatKB.One;
    }
  }
  أرجع Known;
}

KnownBits KnownBits::truncSSatU(unsigned BitWidth) ثابت {
  unsigned مدخل_Bits = getBitWidth();
  APInt MaxInRange = APInt::getAllOnes(BitWidth).zext(مدخل_Bits);
  APInt مدخل_Min = get_Signed_Min_قيمة();
  APInt مدخل_Max = get_Signed_Max_قيمة();
  KnownBits Known(BitWidth);

  إذا (isNegative()) {
    Known.setAllZero();
  } وإلا إذا (مدخل_Min.isNonNegative() && مدخل_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } وإلا إذا (مدخل_Min.isNonNegative() && مدخل_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } وإلا إذا (مدخل_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    Known.Zero = APInt(BitWidth, 0);
    // Common one bits: bits that are 1 in input stay 1
    Known.One = One.trunc(BitWidth);
  } وإلا {
    // Mixed: sign bit unknown
    إذا (مدخل_Max.ule(MaxInRange)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      Known.Zero = Zero.trunc(BitWidth);
      Known.One = APInt(BitWidth, 0);
    } وإلا {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      Known.Zero = APInt(BitWidth, 0);
      Known.One = APInt(BitWidth, 0);
    }
  }
  أرجع Known;
}

KnownBits KnownBits::truncUSat(unsigned BitWidth) ثابت {
  unsigned مدخل_Bits = getBitWidth();
  APInt MaxInRange = APInt::get_أدنى_Bits_Set(مدخل_Bits, BitWidth);
  APInt مدخل_Max = get_Max_قيمة();
  APInt مدخل_Min = get_Min_قيمة();
  KnownBits Known(BitWidth);

  إذا (مدخل_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } وإلا إذا (مدخل_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } وإلا {
    Known.resetAll();
    Known.One = One.trunc(BitWidth);
  }
  أرجع Known;
}

KnownBits KnownBits::sextInReg(unsigned SrcBitWidth) ثابت {
  unsigned BitWidth = getBitWidth();
  أكد(0 < SrcBitWidth && SrcBitWidth <= BitWidth &&
         "Illegal sext-in-register");

  إذا (SrcBitWidth == BitWidth)
    أرجع *هذا;

  unsigned ExtBits = BitWidth - SrcBitWidth;
  KnownBits Result;
  Result.One = One << ExtBits;
  Result.Zero = Zero << ExtBits;
  Result.One.ashrInPlace(ExtBits);
  Result.Zero.ashrInPlace(ExtBits);
  أرجع Result;
}

KnownBits KnownBits::makeGE(ثابت APInt &Val) ثابت {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  unsigned N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_أدنى_Bits(getBitWidth() - N);
  أرجع KnownBits(Zero, One | MaskedVal);
}

KnownBits KnownBits::umax(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  إذا (LHS.get_Min_قيمة().uge(RHS.get_Max_قيمة()))
    أرجع LHS;
  إذا (RHS.get_Min_قيمة().uge(LHS.get_Max_قيمة()))
    أرجع RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  KnownBits L = LHS.makeGE(RHS.get_Min_قيمة());
  KnownBits R = RHS.makeGE(LHS.get_Min_قيمة());
  أرجع L.intersectWith(R);
}

KnownBits KnownBits::umin(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](ثابت KnownBits &Val) { أرجع KnownBits(Val.One, Val.Zero); };
  أرجع Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::smax(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  أرجع flipSignBit(umax(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::smin(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](ثابت KnownBits &Val) {
    unsigned SignBitPosition = Val.getBitWidth() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
    One.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
    أرجع KnownBits(Zero, One);
  };
  أرجع Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::abdu(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  إذا (LHS.get_Min_قيمة().uge(RHS.get_Max_قيمة()))
    أرجع computeForAddSub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, LHS,
                            RHS);
  إذا (RHS.get_Min_قيمة().uge(LHS.get_Max_قيمة()))
    أرجع computeForAddSub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/true, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/true, RHS, LHS);
  أرجع Diff0.intersectWith(Diff1);
}

KnownBits KnownBits::abds(KnownBits LHS, KnownBits RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  إذا (LHS.get_Signed_Min_قيمة().sge(RHS.get_Signed_Max_قيمة()))
    أرجع computeForAddSub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, LHS,
                            RHS);
  إذا (RHS.get_Signed_Min_قيمة().sge(LHS.get_Signed_Max_قيمة()))
    أرجع computeForAddSub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  unsigned SignBitPosition = LHS.getBitWidth() - 1;
  لكل (auto Arg : {&LHS, &RHS}) {
    منطقي Tmp = Arg->Zero[SignBitPosition];
    Arg->Zero.setBitVal(SignBitPosition, Arg->One[SignBitPosition]);
    Arg->One.setBitVal(SignBitPosition, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/true, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/true, RHS, LHS);
  أرجع Diff0.intersectWith(Diff1);
}

ساكن unsigned getMaxShiftAmount(ثابت APInt &Max_قيمة, unsigned BitWidth) {
  إذا (isPowerOf2_32(BitWidth))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    أرجع Max_قيمة.extract_Bits_As_ZExt_قيمة(
        std::min(Log2_32(BitWidth), Max_قيمة.getBitWidth()), 0);
  // This is only an approximate upper bound.
  أرجع Max_قيمة.get_Limited_قيمة(BitWidth - 1);
}

KnownBits KnownBits::shl(ثابت KnownBits &LHS, ثابت KnownBits &RHS, منطقي NUW,
                         منطقي NSW, منطقي ShAmtNonZero) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ثابت KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known;
    منطقي ShiftedOutZero, ShiftedOutOne;
    Known.Zero = LHS.Zero.ushl_ov(ShiftAmt, ShiftedOutZero);
    Known.Zero.set_أدنى_Bits(ShiftAmt);
    Known.One = LHS.One.ushl_ov(ShiftAmt, ShiftedOutOne);

    // All cases returning poison have been handled by MaxShiftAmount already.
    إذا (NSW) {
      إذا (NUW && ShiftAmt != 0)
        // NUW means we can assume anything shifted out was a zero.
        ShiftedOutZero = true;

      إذا (ShiftedOutZero)
        Known.makeNonNegative();
      وإلا إذا (ShiftedOutOne)
        Known.makeNegative();
    }
    أرجع Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_قيمة().get_Limited_قيمة(BitWidth);
  إذا (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  إذا (LHS.isUnknown()) {
    Known.Zero.set_أدنى_Bits(MinShiftAmount);
    إذا (NUW && NSW && MinShiftAmount != 0)
      Known.makeNonNegative();
    أرجع Known;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_قيمة = RHS.get_Max_قيمة();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_قيمة, BitWidth);
  إذا (NUW && NSW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.عدد_Max_Leading_Zeros() - 1);
  إذا (NUW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.عدد_Max_Leading_Zeros());
  إذا (NSW)
    MaxShiftAmount = std::min(
        MaxShiftAmount,
        std::max(LHS.عدد_Max_Leading_Zeros(), LHS.عدد_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  إذا (MinShiftAmount == 0 && MaxShiftAmount == BitWidth - 1 &&
      isPowerOf2_32(BitWidth)) {
    Known.Zero.set_أدنى_Bits(LHS.عدد_Min_Trailing_Zeros());
    إذا (LHS.isAllOnes())
      Known.One.setSignBit();
    إذا (NSW) {
      إذا (LHS.isNonNegative())
        Known.makeNonNegative();
      إذا (LHS.isNegative())
        Known.makeNegative();
    }
    أرجع Known;
  }

  // Find the common bits from all possible shifts.
  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_قيمة();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_قيمة();
  Known.setAllConflict();
  لكل (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    إذا ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      استمر;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    إذا (Known.isUnknown())
      اكسر;
  }

  // All shift amounts may result in poison.
  إذا (Known.hasConflict())
    Known.setAllZero();
  أرجع Known;
}

KnownBits KnownBits::lshr(ثابت KnownBits &LHS, ثابت KnownBits &RHS,
                          منطقي ShAmtNonZero, منطقي Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ثابت KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known >>= ShiftAmt;
    // High bits are known zero.
    Known.Zero.set_عالي_Bits(ShiftAmt);
    أرجع Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_قيمة().get_Limited_قيمة(BitWidth);
  إذا (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  إذا (LHS.isUnknown()) {
    Known.Zero.set_عالي_Bits(MinShiftAmount);
    أرجع Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_قيمة = RHS.get_Max_قيمة();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_قيمة, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  إذا (Exact) {
    unsigned FirstOne = LHS.عدد_Max_Trailing_Zeros();
    إذا (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      أرجع Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_قيمة();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_قيمة();
  Known.setAllConflict();
  لكل (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    إذا ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      استمر;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    إذا (Known.isUnknown())
      اكسر;
  }

  // All shift amounts may result in poison.
  إذا (Known.hasConflict())
    Known.setAllZero();
  أرجع Known;
}

KnownBits KnownBits::ashr(ثابت KnownBits &LHS, ثابت KnownBits &RHS,
                          منطقي ShAmtNonZero, منطقي Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ثابت KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known.Zero.ashrInPlace(ShiftAmt);
    Known.One.ashrInPlace(ShiftAmt);
    أرجع Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_قيمة().get_Limited_قيمة(BitWidth);
  إذا (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  إذا (LHS.isUnknown()) {
    إذا (MinShiftAmount == BitWidth) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      أرجع Known;
    }
    أرجع Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_قيمة = RHS.get_Max_قيمة();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_قيمة, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  إذا (Exact) {
    unsigned FirstOne = LHS.عدد_Max_Trailing_Zeros();
    إذا (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      أرجع Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_قيمة();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_قيمة();
  Known.setAllConflict();
  لكل (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
      ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    إذا ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      استمر;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    إذا (Known.isUnknown())
      اكسر;
  }

  // All shift amounts may result in poison.
  إذا (Known.hasConflict())
    Known.setAllZero();
  أرجع Known;
}

KnownBits KnownBits::fshl(ثابت KnownBits &LHS, ثابت KnownBits &RHS,
                          ثابت APInt &Amt) {
  أرجع KnownBits(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::fshr(ثابت KnownBits &LHS, ثابت KnownBits &RHS,
                          ثابت APInt &Amt) {
  أرجع KnownBits(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::clmul(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  KnownBits Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_قيمة(), RHS.get_Min_قيمة()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  auto ClMulOr = [](ثابت APInt &LHS, ثابت APInt &RHS) {
    unsigned BW = LHS.getBitWidth();
    أكد(BW == RHS.getBitWidth() && "Operand mismatch");
    APInt Result(BW, 0);
    لكل (unsigned I :
         seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
      إذا (RHS[I])
        Result |= LHS << I;
    أرجع Result;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt Known = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= Known;
  Res.One &= Known;

  أرجع Res;
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
  لكل (unsigned I = 0; I < BitWidth; ++I) {
    إذا (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      إذا (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clearBits(M0, M1 + 1);
      إذا (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clearBits(M0, M1 + 1);
    }
    إذا (Mask.One[I])
      ++M0, ++M1;
    وإلا إذا (!Mask.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clearBits(M0, BitWidth);
  أرجع Res;
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
  لكل (unsigned I = 0; I < BitWidth; ++I) {
    إذا (!Mask.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clearBit(I);
    إذا (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::getBitsSet(BitWidth, M0, M1 + 1);
      إذا (!Range.isSubsetOf(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clearBit(I);
      إذا (!Range.isSubsetOf(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clearBit(I);
    }
    إذا (Mask.One[I])
      ++M0, ++M1;
    وإلا إذا (!Mask.Zero[I])
      ++M1;
  }
  أرجع Res;
}

std::optional<منطقي> KnownBits::eq(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  إذا (LHS.isConstant() && RHS.isConstant())
    أرجع LHS.getConstant() == RHS.getConstant();
  إذا (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    أرجع خطأ;
  أرجع std::nullopt;
}

std::optional<منطقي> KnownBits::ne(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  إذا (std::optional<منطقي> KnownEQ = eq(LHS, RHS))
    أرجع !*KnownEQ;
  أرجع std::nullopt;
}

std::optional<منطقي> KnownBits::ugt(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  إذا (LHS.get_Max_قيمة().ule(RHS.get_Min_قيمة()))
    أرجع خطأ;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  إذا (LHS.get_Min_قيمة().ugt(RHS.get_Max_قيمة()))
    أرجع true;
  أرجع std::nullopt;
}

std::optional<منطقي> KnownBits::uge(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  إذا (std::optional<منطقي> IsUGT = ugt(RHS, LHS))
    أرجع !*IsUGT;
  أرجع std::nullopt;
}

std::optional<منطقي> KnownBits::ult(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  أرجع ugt(RHS, LHS);
}

std::optional<منطقي> KnownBits::ule(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  أرجع uge(RHS, LHS);
}

std::optional<منطقي> KnownBits::sgt(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  إذا (LHS.get_Signed_Max_قيمة().sle(RHS.get_Signed_Min_قيمة()))
    أرجع خطأ;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  إذا (LHS.get_Signed_Min_قيمة().sgt(RHS.get_Signed_Max_قيمة()))
    أرجع true;
  أرجع std::nullopt;
}

std::optional<منطقي> KnownBits::sge(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  إذا (std::optional<منطقي> KnownSGT = sgt(RHS, LHS))
    أرجع !*KnownSGT;
  أرجع std::nullopt;
}

std::optional<منطقي> KnownBits::slt(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  أرجع sgt(RHS, LHS);
}

std::optional<منطقي> KnownBits::sle(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  أرجع sge(RHS, LHS);
}

KnownBits KnownBits::abs(منطقي IntMinIsPoison) ثابت {
  // If the source's MSB is zero then we know the rest of the bits already.
  إذا (isNonNegative())
    أرجع *هذا;

  // Absolute value preserves trailing zero count.
  KnownBits KnownAbs(getBitWidth());

  // If the input is negative, then abs(x) == -x.
  إذا (isNegative()) {
    KnownBits Tmp = *هذا;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    إذا (IntMinIsPoison && (Zero.popcount() + 2) == getBitWidth())
      Tmp.One.setBit(عدد_Min_Trailing_Zeros());

    KnownAbs = computeForAddSub(
        /*Add*/ خطأ, IntMinIsPoison, /*NUW=*/خطأ,
        KnownBits::makeConstant(APInt(getBitWidth(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    إذا (IntMinIsPoison && Tmp.عدد_Min_Population() == 1 &&
        Tmp.عدد_Max_Population() != 1) {
      Tmp.One.clearSignBit();
      Tmp.Zero.setSignBit();
      KnownAbs.One.setBits(getBitWidth() - Tmp.عدد_Min_Leading_Zeros(),
                           getBitWidth() - 1);
    }

  } وإلا {
    unsigned MaxTZ = عدد_Max_Trailing_Zeros();
    unsigned MinTZ = عدد_Min_Trailing_Zeros();

    KnownAbs.Zero.set_أدنى_Bits(MinTZ);
    // If we know the lowest set 1, then preserve it.
    إذا (MaxTZ == MinTZ && MaxTZ < getBitWidth())
      KnownAbs.One.setBit(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    إذا (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_قيمة())) {
      KnownAbs.One.clearSignBit();
      KnownAbs.Zero.setSignBit();
    }
  }

  أرجع KnownAbs;
}

KnownBits KnownBits::reduceAdd(unsigned رقم_Elts) ثابت {
  إذا (رقم_Elts == 0)
    أرجع KnownBits(getBitWidth());

  unsigned BitWidth = getBitWidth();
  KnownBits Result(BitWidth);

  إذا (isConstant())
    // If all elements are the same constant, we can simply compute it
    أرجع KnownBits::makeConstant(رقم_Elts * getConstant());

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

  إذا (isNonNegative()) {
    unsigned LeadingZeros = عدد_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > LostBits ? LeadingZeros - LostBits : 0;
    Result.Zero.set_عالي_Bits(LeadingZeros);
  } وإلا إذا (isNegative()) {
    unsigned LeadingOnes = عدد_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > LostBits ? LeadingOnes - LostBits : 0;
    Result.One.set_عالي_Bits(LeadingOnes);
  }

  أرجع Result;
}

ساكن KnownBits computeForSatAddSub(منطقي Add, منطقي Signed,
                                     ثابت KnownBits &LHS,
                                     ثابت KnownBits &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  unsigned BitWidth = LHS.getBitWidth();

  std::optional<منطقي> Overflow;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  منطقي MayNegClamp = true;
  منطقي MayPosClamp = true;
  إذا (Signed) {
    // Easy cases we can rule out any overflow.
    إذا (Add && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      Overflow = خطأ;
    وإلا إذا (!Add && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      Overflow = خطأ;
    وإلا {
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
          KnownBits::computeForAddSub(Add, /*NSW=*/خطأ,
                                      /*NUW=*/خطأ, UnsignedLHS, UnsignedRHS);
      إذا (Add) {
        إذا (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = خطأ;
          // Pos + Pos will overflow with extra signbit.
          إذا (LHS.isNonNegative() && RHS.isNonNegative())
            Overflow = true;
        } وإلا إذا (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = خطأ;
          // Neg + Neg will overflow without extra signbit.
          إذا (LHS.isNegative() && RHS.isNegative())
            Overflow = true;
        }
        // We will never clamp to the opposite sign of N-bit result.
        إذا (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = خطأ;
        إذا (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = خطأ;
      } وإلا {
        إذا (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = خطأ;
          // Neg - Pos will overflow with extra signbit.
          إذا (LHS.isNegative() && RHS.isNonNegative())
            Overflow = true;
        } وإلا إذا (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = خطأ;
          // Pos - Neg will overflow without extra signbit.
          إذا (LHS.isNonNegative() && RHS.isNegative())
            Overflow = true;
        }
        // We will never clamp to the opposite sign of N-bit result.
        إذا (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = خطأ;
        إذا (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = خطأ;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    إذا (!MayNegClamp && !MayPosClamp)
      Overflow = خطأ;
  } وإلا إذا (Add) {
    // uadd.sat
    منطقي Of;
    (فراغ)LHS.get_Max_قيمة().uadd_ov(RHS.get_Max_قيمة(), Of);
    إذا (!Of) {
      Overflow = خطأ;
    } وإلا {
      (فراغ)LHS.get_Min_قيمة().uadd_ov(RHS.get_Min_قيمة(), Of);
      إذا (Of)
        Overflow = true;
    }
  } وإلا {
    // usub.sat
    منطقي Of;
    (فراغ)LHS.get_Min_قيمة().usub_ov(RHS.get_Max_قيمة(), Of);
    إذا (!Of) {
      Overflow = خطأ;
    } وإلا {
      (فراغ)LHS.get_Max_قيمة().usub_ov(RHS.get_Min_قيمة(), Of);
      إذا (Of)
        Overflow = true;
    }
  }

  KnownBits Res = KnownBits::computeForAddSub(Add, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  إذا (Overflow) {
    // We know whether or not we overflowed.
    إذا (!(*Overflow)) {
      // No overflow.
      أرجع Res;
    }

    // We overflowed
    APInt C;
    إذا (Signed) {
      // sadd.sat / ssub.sat
      أكد(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_قيمة(BitWidth)
                           : APInt::get_Signed_Max_قيمة(BitWidth);
    } وإلا إذا (Add) {
      // uadd.sat
      C = APInt::get_Max_قيمة(BitWidth);
    } وإلا {
      // uadd.sat
      C = APInt::get_Min_قيمة(BitWidth);
    }

    Res.One = C;
    Res.Zero = ~C;
    أرجع Res;
  }

  // We don't know if we overflowed.
  إذا (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    إذا (MayPosClamp)
      Res.Zero.clear_أدنى_Bits(BitWidth - 1);
    إذا (MayNegClamp)
      Res.One.clear_أدنى_Bits(BitWidth - 1);
  } وإلا إذا (Add) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clearAllBits();
  } وإلا {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clearAllBits();
  }

  أرجع Res;
}

KnownBits KnownBits::sadd_sat(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  أرجع computeForSatAddSub(/*Add*/ true, /*Signed*/ true, LHS, RHS);
}
KnownBits KnownBits::ssub_sat(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  أرجع computeForSatAddSub(/*Add*/ خطأ, /*Signed*/ true, LHS, RHS);
}
KnownBits KnownBits::uadd_sat(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  أرجع computeForSatAddSub(/*Add*/ true, /*Signed*/ خطأ, LHS, RHS);
}
KnownBits KnownBits::usub_sat(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  أرجع computeForSatAddSub(/*Add*/ خطأ, /*Signed*/ خطأ, LHS, RHS);
}

ساكن KnownBits avgComputeU(KnownBits LHS, KnownBits RHS, منطقي IsCeil) {
  unsigned BitWidth = LHS.getBitWidth();
  LHS = LHS.zext(BitWidth + 1);
  RHS = RHS.zext(BitWidth + 1);
  LHS =
      computeForAddCarry(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.extractBits(BitWidth, 1);
  أرجع LHS;
}

KnownBits KnownBits::avgFloorS(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  أرجع flipSignBit(avgFloorU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgFloorU(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  أرجع avgComputeU(LHS, RHS, /*IsCeil=*/خطأ);
}

KnownBits KnownBits::avgCeilS(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  أرجع flipSignBit(avgCeilU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgCeilU(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  أرجع avgComputeU(LHS, RHS, /*IsCeil=*/true);
}

KnownBits KnownBits::mul(ثابت KnownBits &LHS, ثابت KnownBits &RHS,
                         منطقي NoUndefSelfMultiply) {
  unsigned BitWidth = LHS.getBitWidth();
  أكد(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  أكد((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.get_Max_قيمة();
  APInt UMaxRHS = RHS.get_Max_قيمة();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  منطقي HasOverflow;
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
  unsigned TrailZeroLHS = LHS.عدد_Min_Trailing_Zeros();
  unsigned TrailZeroRHS = RHS.عدد_Min_Trailing_Zeros();
  unsigned TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  unsigned SmallestOperand = std::min(TrailBitsKnownLHS - TrailZeroLHS,
                                      TrailBitsKnownRHS - TrailZeroRHS);
  unsigned ResultBitsKnown = std::min(SmallestOperand + TrailZ, BitWidth);

  // The lower ResultBitsKnown bits of this are known.
  APInt BottomKnown = LHS.One * RHS.One;

  KnownBits Res(BitWidth);
  Res.Zero.set_عالي_Bits(LeadZ);
  Res.Zero |= (~BottomKnown).getLoBits(ResultBitsKnown);
  Res.One = BottomKnown.getLoBits(ResultBitsKnown);

  إذا (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    unsigned TwoTZP1 = 2 * TrailZeroLHS + 1;
    إذا (TwoTZP1 < BitWidth)
      Res.Zero.setBit(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    إذا (TrailZeroLHS < BitWidth && LHS.One[TrailZeroLHS]) {
      unsigned TwoTZP2 = TwoTZP1 + 1;
      إذا (TwoTZP2 < BitWidth)
        Res.Zero.setBit(TwoTZP2);
    }
  }

  أرجع Res;
}

KnownBits KnownBits::mulhs(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  أكد(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.sext(2 * BitWidth);
  KnownBits WideRHS = RHS.sext(2 * BitWidth);
  أرجع mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

KnownBits KnownBits::mulhu(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  أكد(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.zext(2 * BitWidth);
  KnownBits WideRHS = RHS.zext(2 * BitWidth);
  أرجع mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

ساكن KnownBits div_Compute_أدنى_Bit(KnownBits Known, ثابت KnownBits &LHS,
                                  ثابت KnownBits &RHS, منطقي Exact) {

  إذا (!Exact)
    أرجع Known;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  إذا (LHS.One[0])
    Known.One.setBit(0);

  صحيح MinTZ =
      (صحيح)LHS.عدد_Min_Trailing_Zeros() - (صحيح)RHS.عدد_Max_Trailing_Zeros();
  صحيح MaxTZ =
      (صحيح)LHS.عدد_Max_Trailing_Zeros() - (صحيح)RHS.عدد_Min_Trailing_Zeros();
  إذا (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    Known.Zero.set_أدنى_Bits(MinTZ);
    إذا (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      Known.One.setBit(MinTZ);
    }
  } وإلا إذا (MaxTZ < 0) {
    // Poison Result
    Known.setAllZero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  إذا (Known.hasConflict())
    Known.setAllZero();

  أرجع Known;
}

KnownBits KnownBits::sdiv(ثابت KnownBits &LHS, ثابت KnownBits &RHS,
                          منطقي Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  إذا (LHS.isNonNegative() && RHS.isNonNegative())
    أرجع udiv(LHS, RHS, Exact);

  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  إذا (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    أرجع Known;
  }

  std::optional<APInt> Res;
  إذا (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_قيمة();
    APInt رقم = LHS.get_Signed_Min_قيمة();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (رقم.is_Min_Signed_قيمة() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_قيمة(BitWidth)
              : رقم.sdiv(Denom);
  } وإلا إذا (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    إذا (Exact || (-LHS.get_Signed_Max_قيمة()).uge(RHS.get_Signed_Max_قيمة())) {
      APInt Denom = RHS.get_Signed_Min_قيمة();
      APInt رقم = LHS.get_Signed_Min_قيمة();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      إذا (Denom.isZero())
        Denom.setBit(RHS.عدد_Min_Trailing_Zeros());
      Res = رقم.sdiv(Denom);
    }
  } وإلا إذا (LHS.isStrictlyPositive() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    إذا (Exact || LHS.get_Signed_Min_قيمة().uge(-RHS.get_Signed_Min_قيمة())) {
      APInt Denom = RHS.get_Signed_Max_قيمة();
      APInt رقم = LHS.get_Signed_Max_قيمة();
      Res = رقم.sdiv(Denom);
    }
  }

  إذا (Res) {
    إذا (Res->isNonNegative()) {
      unsigned LeadZ = Res->عدد_Leading_Zeros();
      Known.Zero.set_عالي_Bits(LeadZ);
    } وإلا {
      unsigned LeadO = Res->عدد_Leading_Ones();
      Known.One.set_عالي_Bits(LeadO);
    }
  }

  Known = div_Compute_أدنى_Bit(Known, LHS, RHS, Exact);
  أرجع Known;
}

KnownBits KnownBits::udiv(ثابت KnownBits &LHS, ثابت KnownBits &RHS,
                          منطقي Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  إذا (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    أرجع Known;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_قيمة();
  إذا (MinDenom.isZero())
    MinDenom.setBit(RHS.عدد_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_قيمة().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_قيمة().udiv(RHS.get_Max_قيمة());

  Known = KnownBits::makeConstant(MinRes);
  إذا (std::optional<unsigned> DifferentBit =
          APIntOps::GetMostSignificantDifferentBit(MinRes, MaxRes)) {
    Known.Zero.clear_أدنى_Bits(*DifferentBit + 1);
    Known.One.clear_أدنى_Bits(*DifferentBit + 1);
  }

  Known = div_Compute_أدنى_Bit(Known, LHS, RHS, Exact);

  أرجع Known;
}

KnownBits KnownBits::rem_Get_أدنى_Bits(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  إذا (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    unsigned RHSZeros = RHS.عدد_Min_Trailing_Zeros();
    APInt Mask = APInt::get_أدنى_Bits_Set(BitWidth, RHSZeros);
    APInt OnesMask = LHS.One & Mask;
    APInt ZerosMask = LHS.Zero & Mask;
    أرجع KnownBits(ZerosMask, OnesMask);
  }
  أرجع KnownBits(BitWidth);
}

KnownBits KnownBits::urem(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  إذا (LHS.get_Max_قيمة().ult(RHS.get_Min_قيمة()))
    أرجع LHS;

  KnownBits Known = rem_Get_أدنى_Bits(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_قيمة(), RHS.get_Max_قيمة() - 1);
  Known.Zero.set_عالي_Bits(MaxRes.عدد_Leading_Zeros());
  أرجع Known;
}

KnownBits KnownBits::srem(ثابت KnownBits &LHS, ثابت KnownBits &RHS) {
  KnownBits Known = rem_Get_أدنى_Bits(LHS, RHS);
  إذا (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt أدنى_Bits = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    إذا (LHS.isNonNegative() || أدنى_Bits.isSubsetOf(LHS.Zero))
      Known.Zero |= ~أدنى_Bits;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    إذا (LHS.isNegative() && أدنى_Bits.intersects(LHS.One))
      Known.One |= ~أدنى_Bits;
    أرجع Known;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  إذا (LHS.isNegative() && Known.isNonZero())
    Known.One.set_عالي_Bits(
        std::max(LHS.عدد_Min_Leading_Ones(), RHS.عدد_Min_Sign_Bits()));
  وإلا إذا (LHS.isNonNegative())
    Known.Zero.set_عالي_Bits(
        std::max(LHS.عدد_Min_Leading_Zeros(), RHS.عدد_Min_Sign_Bits()));
  أرجع Known;
}

KnownBits &KnownBits::operator&=(ثابت KnownBits &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  أرجع *هذا;
}

KnownBits &KnownBits::operator|=(ثابت KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  أرجع *هذا;
}

KnownBits &KnownBits::operator^=(ثابت KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  أرجع *هذا;
}

KnownBits KnownBits::blsi() ثابت {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(Zero, APInt(BitWidth, 0));
  unsigned Max = عدد_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = عدد_Min_Trailing_Zeros();
  إذا (Max == Min && Max < BitWidth)
    Known.One.setBit(Max);
  أرجع Known;
}

KnownBits KnownBits::blsmsk() ثابت {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(BitWidth);
  unsigned Max = عدد_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = عدد_Min_Trailing_Zeros();
  Known.One.set_أدنى_Bits(std::min(Min + 1, BitWidth));
  أرجع Known;
}

فراغ KnownBits::اطبع(raw_ostream &OS) ثابت {
  unsigned BitWidth = getBitWidth();
  لكل (unsigned I = 0; I < BitWidth; ++I) {
    unsigned N = BitWidth - I - 1;
    إذا (Zero[N] && One[N])
      OS << "!";
    وإلا إذا (Zero[N])
      OS << "0";
    وإلا إذا (One[N])
      OS << "1";
    وإلا
      OS << "?";
  }
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD فراغ KnownBits::dump() ثابت {
  اطبع(dbgs());
  dbgs() << "\n";
}
#endif

unsigned llvm::SignBitsOps::rot(unsigned SrcSignBits, unsigned BitWidth,
                                std::optional<APInt> RotAmt,
                                منطقي Is_Rotate_يمين) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  إذا (SrcSignBits == BitWidth)
    أرجع BitWidth;

  إذا (!RotAmt)
    أرجع 1;

  unsigned Amt = RotAmt->urem(BitWidth);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  إذا (Is_Rotate_يمين)
    Amt = (BitWidth - Amt) % BitWidth;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  إذا (SrcSignBits > Amt + 1)
    أرجع SrcSignBits - Amt;
  أرجع 1;
}
