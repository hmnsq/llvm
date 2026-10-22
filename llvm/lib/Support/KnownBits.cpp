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

#include "llvm/Support/KnownBits.h"
#include "llvm/ADT/Sequence.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>

using namespace llvm;

KnownBits KnownBits::flipSignBit(const KnownBits &Val) {
  unsigned SignBitPosition = Val.getBitWidth() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
  One.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
  ສົ່ງຄືນ KnownBits(Zero, One);
}

static KnownBits computeForAddCarry(const KnownBits &LHS, const KnownBits &RHS,
                                    bool CarryZero, bool CarryOne) {

  APInt PossibleSumZero = LHS.getMaxValue() + RHS.getMaxValue() + !CarryZero;
  APInt PossibleSumOne = LHS.getMinValue() + RHS.getMinValue() + CarryOne;

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
  ສົ່ງຄືນ KnownOut;
}

KnownBits KnownBits::computeForAddCarry(
    const KnownBits &LHS, const KnownBits &RHS, const KnownBits &Carry) {
  assert(Carry.getBitWidth() == 1 && "Carry must be 1-bit");
  ສົ່ງຄືນ ::computeForAddCarry(
      LHS, RHS, Carry.Zero.getBoolValue(), Carry.One.getBoolValue());
}

KnownBits KnownBits::computeForAddSub(bool Add, bool NSW, bool NUW,
                                      const KnownBits &LHS,
                                      const KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits KnownOut(BitWidth);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  ຖ້າ (LHS.isUnknown() && RHS.isUnknown())
    ສົ່ງຄືນ KnownOut;

  ຖ້າ (!LHS.isUnknown() && !RHS.isUnknown()) {
    ຖ້າ (Add) {
      // Sum = LHS + RHS + 0
      KnownOut = ::computeForAddCarry(LHS, RHS, /*CarryZero=*/true,
                                      /*CarryOne=*/false);
    } ບໍ່ດັ່ງນັ້ນ {
      // Sum = LHS + ~RHS + 1
      KnownBits NotRHS = RHS;
      std::swap(NotRHS.Zero, NotRHS.One);
      KnownOut = ::computeForAddCarry(LHS, NotRHS, /*CarryZero=*/false,
                                      /*CarryOne=*/true);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  ຖ້າ (NUW) {
    ຖ້າ (Add) {
      // (add nuw X, Y)
      APInt MinVal = LHS.getMinValue().uadd_sat(RHS.getMinValue());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      ຖ້າ (NSW) {
        unsigned NumBits = MinVal.trunc(BitWidth - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        KnownOut.One.setBits(BitWidth - 1 - NumBits, BitWidth - 1);
      }
      KnownOut.One.setHighBits(MinVal.countl_one());
    } ບໍ່ດັ່ງນັ້ນ {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.getMaxValue().usub_sat(RHS.getMinValue());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      ຖ້າ (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        unsigned NumBits = MaxVal.trunc(BitWidth - 1).countl_zero();
        KnownOut.Zero.setBits(BitWidth - 1 - NumBits, BitWidth - 1);
      }
      KnownOut.Zero.setHighBits(MaxVal.countl_zero());
    }
  }

  ຖ້າ (NSW) {
    APInt MinVal;
    APInt MaxVal;
    ຖ້າ (Add) {
      // (add nsw X, Y)
      MinVal = LHS.getSignedMinValue().sadd_sat(RHS.getSignedMinValue());
      MaxVal = LHS.getSignedMaxValue().sadd_sat(RHS.getSignedMaxValue());
    } ບໍ່ດັ່ງນັ້ນ {
      // (sub nsw X, Y)
      MinVal = LHS.getSignedMinValue().ssub_sat(RHS.getSignedMaxValue());
      MaxVal = LHS.getSignedMaxValue().ssub_sat(RHS.getSignedMinValue());
    }
    ຖ້າ (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      unsigned NumBits = MinVal.trunc(BitWidth - 1).countl_one();
      KnownOut.One.setBits(BitWidth - 1 - NumBits, BitWidth - 1);
      KnownOut.Zero.setSignBit();
    }
    ຖ້າ (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      unsigned NumBits = MaxVal.trunc(BitWidth - 1).countl_zero();
      KnownOut.Zero.setBits(BitWidth - 1 - NumBits, BitWidth - 1);
      KnownOut.One.setSignBit();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  ຖ້າ (KnownOut.hasConflict())
    KnownOut.setAllZero();
  ສົ່ງຄືນ KnownOut;
}

KnownBits KnownBits::computeForSubBorrow(const KnownBits &LHS, KnownBits RHS,
                                         const KnownBits &Borrow) {
  assert(Borrow.getBitWidth() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::swap(RHS.Zero, RHS.One);
  ສົ່ງຄືນ ::computeForAddCarry(LHS, RHS,
                              /*CarryZero=*/Borrow.One.getBoolValue(),
                              /*CarryOne=*/Borrow.Zero.getBoolValue());
}

KnownBits KnownBits::truncSSat(unsigned BitWidth) const {
  unsigned InputBits = getBitWidth();
  APInt MinInRange = APInt::getSignedMinValue(BitWidth).sext(InputBits);
  APInt MaxInRange = APInt::getSignedMaxValue(BitWidth).sext(InputBits);
  APInt InputMin = getSignedMinValue();
  APInt InputMax = getSignedMaxValue();
  KnownBits Known(BitWidth);

  // Case 1: All values fit - just truncate
  ຖ້າ (InputMin.sge(MinInRange) && InputMax.sle(MaxInRange)) {
    Known = trunc(BitWidth);
  }
  // Case 2: All saturate to min
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (InputMax.slt(MinInRange)) {
    Known = KnownBits::makeConstant(APInt::getSignedMinValue(BitWidth));
  }
  // Case 3: All saturate to max
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (InputMin.sgt(MaxInRange)) {
    Known = KnownBits::makeConstant(APInt::getSignedMaxValue(BitWidth));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (InputMin.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    Known.Zero = APInt(BitWidth, 0);
    Known.Zero.setBit(BitWidth - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    Known.One = One.trunc(BitWidth);
    Known.One.clearBit(BitWidth - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (InputMax.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    Known.Zero = Zero.trunc(BitWidth);
    Known.Zero.clearBit(BitWidth - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    Known.One = APInt(BitWidth, 0);
    Known.One.setBit(BitWidth - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  ບໍ່ດັ່ງນັ້ນ {
    // Output: min saturation, truncated, or max saturation
    APInt InputZeroLower = Zero.trunc(BitWidth);
    APInt InputOneLower = One.trunc(BitWidth);
    APInt MinSat = APInt::getSignedMinValue(BitWidth);
    APInt MaxSat = APInt::getSignedMaxValue(BitWidth);
    KnownBits MinSatKB = KnownBits::makeConstant(MinSat);
    KnownBits MaxSatKB = KnownBits::makeConstant(MaxSat);
    ຖ້າ (InputMax.sle(MaxInRange)) {
      // Positive values fit, only negatives saturate to min
      Known.Zero = InputZeroLower & MinSatKB.Zero;
      Known.One = InputOneLower & MinSatKB.One;
    } ບໍ່ດັ່ງນັ້ນ ຖ້າ (InputMin.sge(MinInRange)) {
      // Negative values fit, only positives saturate to max
      Known.Zero = InputZeroLower & MaxSatKB.Zero;
      Known.One = InputOneLower & MaxSatKB.One;
    } ບໍ່ດັ່ງນັ້ນ {
      // Both positive and negative values might saturate
      Known.Zero = InputZeroLower & MinSatKB.Zero & MaxSatKB.Zero;
      Known.One = InputOneLower & MinSatKB.One & MaxSatKB.One;
    }
  }
  ສົ່ງຄືນ Known;
}

KnownBits KnownBits::truncSSatU(unsigned BitWidth) const {
  unsigned InputBits = getBitWidth();
  APInt MaxInRange = APInt::getAllOnes(BitWidth).zext(InputBits);
  APInt InputMin = getSignedMinValue();
  APInt InputMax = getSignedMaxValue();
  KnownBits Known(BitWidth);

  ຖ້າ (isNegative()) {
    Known.setAllZero();
  } ບໍ່ດັ່ງນັ້ນ ຖ້າ (InputMin.isNonNegative() && InputMax.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } ບໍ່ດັ່ງນັ້ນ ຖ້າ (InputMin.isNonNegative() && InputMin.ugt(MaxInRange)) {
    Known.setAllOnes();
  } ບໍ່ດັ່ງນັ້ນ ຖ້າ (InputMin.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    Known.Zero = APInt(BitWidth, 0);
    // Common one bits: bits that are 1 in input stay 1
    Known.One = One.trunc(BitWidth);
  } ບໍ່ດັ່ງນັ້ນ {
    // Mixed: sign bit unknown
    ຖ້າ (InputMax.ule(MaxInRange)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      Known.Zero = Zero.trunc(BitWidth);
      Known.One = APInt(BitWidth, 0);
    } ບໍ່ດັ່ງນັ້ນ {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      Known.Zero = APInt(BitWidth, 0);
      Known.One = APInt(BitWidth, 0);
    }
  }
  ສົ່ງຄືນ Known;
}

KnownBits KnownBits::truncUSat(unsigned BitWidth) const {
  unsigned InputBits = getBitWidth();
  APInt MaxInRange = APInt::getLowBitsSet(InputBits, BitWidth);
  APInt InputMax = getMaxValue();
  APInt InputMin = getMinValue();
  KnownBits Known(BitWidth);

  ຖ້າ (InputMax.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } ບໍ່ດັ່ງນັ້ນ ຖ້າ (InputMin.ugt(MaxInRange)) {
    Known.setAllOnes();
  } ບໍ່ດັ່ງນັ້ນ {
    Known.resetAll();
    Known.One = One.trunc(BitWidth);
  }
  ສົ່ງຄືນ Known;
}

KnownBits KnownBits::sextInReg(unsigned SrcBitWidth) const {
  unsigned BitWidth = getBitWidth();
  assert(0 < SrcBitWidth && SrcBitWidth <= BitWidth &&
         "Illegal sext-in-register");

  ຖ້າ (SrcBitWidth == BitWidth)
    ສົ່ງຄືນ *this;

  unsigned ExtBits = BitWidth - SrcBitWidth;
  KnownBits Result;
  Result.One = One << ExtBits;
  Result.Zero = Zero << ExtBits;
  Result.One.ashrInPlace(ExtBits);
  Result.Zero.ashrInPlace(ExtBits);
  ສົ່ງຄືນ Result;
}

KnownBits KnownBits::makeGE(const APInt &Val) const {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  unsigned N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clearLowBits(getBitWidth() - N);
  ສົ່ງຄືນ KnownBits(Zero, One | MaskedVal);
}

KnownBits KnownBits::umax(const KnownBits &LHS, const KnownBits &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  ຖ້າ (LHS.getMinValue().uge(RHS.getMaxValue()))
    ສົ່ງຄືນ LHS;
  ຖ້າ (RHS.getMinValue().uge(LHS.getMaxValue()))
    ສົ່ງຄືນ RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  KnownBits L = LHS.makeGE(RHS.getMinValue());
  KnownBits R = RHS.makeGE(LHS.getMinValue());
  ສົ່ງຄືນ L.intersectWith(R);
}

KnownBits KnownBits::umin(const KnownBits &LHS, const KnownBits &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](const KnownBits &Val) { ສົ່ງຄືນ KnownBits(Val.One, Val.Zero); };
  ສົ່ງຄືນ Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::smax(const KnownBits &LHS, const KnownBits &RHS) {
  ສົ່ງຄືນ flipSignBit(umax(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::smin(const KnownBits &LHS, const KnownBits &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](const KnownBits &Val) {
    unsigned SignBitPosition = Val.getBitWidth() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
    One.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
    ສົ່ງຄືນ KnownBits(Zero, One);
  };
  ສົ່ງຄືນ Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::abdu(const KnownBits &LHS, const KnownBits &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  ຖ້າ (LHS.getMinValue().uge(RHS.getMaxValue()))
    ສົ່ງຄືນ computeForAddSub(/*Add=*/false, /*NSW=*/false, /*NUW=*/false, LHS,
                            RHS);
  ຖ້າ (RHS.getMinValue().uge(LHS.getMaxValue()))
    ສົ່ງຄືນ computeForAddSub(/*Add=*/false, /*NSW=*/false, /*NUW=*/false, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/false, /*NSW=*/false, /*NUW=*/true, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/false, /*NSW=*/false, /*NUW=*/true, RHS, LHS);
  ສົ່ງຄືນ Diff0.intersectWith(Diff1);
}

KnownBits KnownBits::abds(KnownBits LHS, KnownBits RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  ຖ້າ (LHS.getSignedMinValue().sge(RHS.getSignedMaxValue()))
    ສົ່ງຄືນ computeForAddSub(/*Add=*/false, /*NSW=*/false, /*NUW=*/false, LHS,
                            RHS);
  ຖ້າ (RHS.getSignedMinValue().sge(LHS.getSignedMaxValue()))
    ສົ່ງຄືນ computeForAddSub(/*Add=*/false, /*NSW=*/false, /*NUW=*/false, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  unsigned SignBitPosition = LHS.getBitWidth() - 1;
  ສຳລັບ (auto Arg : {&LHS, &RHS}) {
    bool Tmp = Arg->Zero[SignBitPosition];
    Arg->Zero.setBitVal(SignBitPosition, Arg->One[SignBitPosition]);
    Arg->One.setBitVal(SignBitPosition, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/false, /*NSW=*/false, /*NUW=*/true, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/false, /*NSW=*/false, /*NUW=*/true, RHS, LHS);
  ສົ່ງຄືນ Diff0.intersectWith(Diff1);
}

static unsigned getMaxShiftAmount(const APInt &MaxValue, unsigned BitWidth) {
  ຖ້າ (isPowerOf2_32(BitWidth))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    ສົ່ງຄືນ MaxValue.extractBitsAsZExtValue(
        std::min(Log2_32(BitWidth), MaxValue.getBitWidth()), 0);
  // This is only an approximate upper bound.
  ສົ່ງຄືນ MaxValue.getLimitedValue(BitWidth - 1);
}

KnownBits KnownBits::shl(const KnownBits &LHS, const KnownBits &RHS, bool NUW,
                         bool NSW, bool ShAmtNonZero) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](const KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known;
    bool ShiftedOutZero, ShiftedOutOne;
    Known.Zero = LHS.Zero.ushl_ov(ShiftAmt, ShiftedOutZero);
    Known.Zero.setLowBits(ShiftAmt);
    Known.One = LHS.One.ushl_ov(ShiftAmt, ShiftedOutOne);

    // All cases returning poison have been handled by MaxShiftAmount already.
    ຖ້າ (NSW) {
      ຖ້າ (NUW && ShiftAmt != 0)
        // NUW means we can assume anything shifted out was a zero.
        ShiftedOutZero = true;

      ຖ້າ (ShiftedOutZero)
        Known.makeNonNegative();
      ບໍ່ດັ່ງນັ້ນ ຖ້າ (ShiftedOutOne)
        Known.makeNegative();
    }
    ສົ່ງຄືນ Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.getMinValue().getLimitedValue(BitWidth);
  ຖ້າ (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  ຖ້າ (LHS.isUnknown()) {
    Known.Zero.setLowBits(MinShiftAmount);
    ຖ້າ (NUW && NSW && MinShiftAmount != 0)
      Known.makeNonNegative();
    ສົ່ງຄືນ Known;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt MaxValue = RHS.getMaxValue();
  unsigned MaxShiftAmount = getMaxShiftAmount(MaxValue, BitWidth);
  ຖ້າ (NUW && NSW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.countMaxLeadingZeros() - 1);
  ຖ້າ (NUW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.countMaxLeadingZeros());
  ຖ້າ (NSW)
    MaxShiftAmount = std::min(
        MaxShiftAmount,
        std::max(LHS.countMaxLeadingZeros(), LHS.countMaxLeadingOnes()) - 1);

  // Fast path for common case where the shift amount is unknown.
  ຖ້າ (MinShiftAmount == 0 && MaxShiftAmount == BitWidth - 1 &&
      isPowerOf2_32(BitWidth)) {
    Known.Zero.setLowBits(LHS.countMinTrailingZeros());
    ຖ້າ (LHS.isAllOnes())
      Known.One.setSignBit();
    ຖ້າ (NSW) {
      ຖ້າ (LHS.isNonNegative())
        Known.makeNonNegative();
      ຖ້າ (LHS.isNegative())
        Known.makeNegative();
    }
    ສົ່ງຄືນ Known;
  }

  // Find the common bits from all possible shifts.
  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).getZExtValue();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).getZExtValue();
  Known.setAllConflict();
  ສຳລັບ (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    ຖ້າ ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      continue;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    ຖ້າ (Known.isUnknown())
      bha_nga;
  }

  // All shift amounts may result in poison.
  ຖ້າ (Known.hasConflict())
    Known.setAllZero();
  ສົ່ງຄືນ Known;
}

KnownBits KnownBits::lshr(const KnownBits &LHS, const KnownBits &RHS,
                          bool ShAmtNonZero, bool Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](const KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known >>= ShiftAmt;
    // High bits are known zero.
    Known.Zero.setHighBits(ShiftAmt);
    ສົ່ງຄືນ Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.getMinValue().getLimitedValue(BitWidth);
  ຖ້າ (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  ຖ້າ (LHS.isUnknown()) {
    Known.Zero.setHighBits(MinShiftAmount);
    ສົ່ງຄືນ Known;
  }

  // Find the common bits from all possible shifts.
  APInt MaxValue = RHS.getMaxValue();
  unsigned MaxShiftAmount = getMaxShiftAmount(MaxValue, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  ຖ້າ (Exact) {
    unsigned FirstOne = LHS.countMaxTrailingZeros();
    ຖ້າ (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      ສົ່ງຄືນ Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).getZExtValue();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).getZExtValue();
  Known.setAllConflict();
  ສຳລັບ (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    ຖ້າ ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      continue;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    ຖ້າ (Known.isUnknown())
      bha_nga;
  }

  // All shift amounts may result in poison.
  ຖ້າ (Known.hasConflict())
    Known.setAllZero();
  ສົ່ງຄືນ Known;
}

KnownBits KnownBits::ashr(const KnownBits &LHS, const KnownBits &RHS,
                          bool ShAmtNonZero, bool Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](const KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known.Zero.ashrInPlace(ShiftAmt);
    Known.One.ashrInPlace(ShiftAmt);
    ສົ່ງຄືນ Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.getMinValue().getLimitedValue(BitWidth);
  ຖ້າ (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  ຖ້າ (LHS.isUnknown()) {
    ຖ້າ (MinShiftAmount == BitWidth) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      ສົ່ງຄືນ Known;
    }
    ສົ່ງຄືນ Known;
  }

  // Find the common bits from all possible shifts.
  APInt MaxValue = RHS.getMaxValue();
  unsigned MaxShiftAmount = getMaxShiftAmount(MaxValue, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  ຖ້າ (Exact) {
    unsigned FirstOne = LHS.countMaxTrailingZeros();
    ຖ້າ (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      ສົ່ງຄືນ Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).getZExtValue();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).getZExtValue();
  Known.setAllConflict();
  ສຳລັບ (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
      ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    ຖ້າ ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      continue;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    ຖ້າ (Known.isUnknown())
      bha_nga;
  }

  // All shift amounts may result in poison.
  ຖ້າ (Known.hasConflict())
    Known.setAllZero();
  ສົ່ງຄືນ Known;
}

KnownBits KnownBits::fshl(const KnownBits &LHS, const KnownBits &RHS,
                          const APInt &Amt) {
  ສົ່ງຄືນ KnownBits(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::fshr(const KnownBits &LHS, const KnownBits &RHS,
                          const APInt &Amt) {
  ສົ່ງຄືນ KnownBits(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::clmul(const KnownBits &LHS, const KnownBits &RHS) {
  KnownBits Res =
      makeConstant(APIntOps::clmul(LHS.getMinValue(), RHS.getMinValue()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  auto ClMulOr = [](const APInt &LHS, const APInt &RHS) {
    unsigned BW = LHS.getBitWidth();
    assert(BW == RHS.getBitWidth() && "Operand mismatch");
    APInt Result(BW, 0);
    ສຳລັບ (unsigned I :
         seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
      ຖ້າ (RHS[I])
        Result |= LHS << I;
    ສົ່ງຄືນ Result;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt Known = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= Known;
  Res.One &= Known;

  ສົ່ງຄືນ Res;
}

KnownBits KnownBits::pext(const KnownBits &Val, const KnownBits &Mask) {
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
  ສຳລັບ (unsigned I = 0; I < BitWidth; ++I) {
    ຖ້າ (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      ຖ້າ (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clearBits(M0, M1 + 1);
      ຖ້າ (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clearBits(M0, M1 + 1);
    }
    ຖ້າ (Mask.One[I])
      ++M0, ++M1;
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (!Mask.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clearBits(M0, BitWidth);
  ສົ່ງຄືນ Res;
}

KnownBits KnownBits::pdep(const KnownBits &Val, const KnownBits &Mask) {
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
  ສຳລັບ (unsigned I = 0; I < BitWidth; ++I) {
    ຖ້າ (!Mask.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clearBit(I);
    ຖ້າ (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::getBitsSet(BitWidth, M0, M1 + 1);
      ຖ້າ (!Range.isSubsetOf(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clearBit(I);
      ຖ້າ (!Range.isSubsetOf(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clearBit(I);
    }
    ຖ້າ (Mask.One[I])
      ++M0, ++M1;
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (!Mask.Zero[I])
      ++M1;
  }
  ສົ່ງຄືນ Res;
}

std::optional<bool> KnownBits::eq(const KnownBits &LHS, const KnownBits &RHS) {
  ຖ້າ (LHS.isConstant() && RHS.isConstant())
    ສົ່ງຄືນ LHS.getConstant() == RHS.getConstant();
  ຖ້າ (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    ສົ່ງຄືນ false;
  ສົ່ງຄືນ std::nullopt;
}

std::optional<bool> KnownBits::ne(const KnownBits &LHS, const KnownBits &RHS) {
  ຖ້າ (std::optional<bool> KnownEQ = eq(LHS, RHS))
    ສົ່ງຄືນ !*KnownEQ;
  ສົ່ງຄືນ std::nullopt;
}

std::optional<bool> KnownBits::ugt(const KnownBits &LHS, const KnownBits &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  ຖ້າ (LHS.getMaxValue().ule(RHS.getMinValue()))
    ສົ່ງຄືນ false;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  ຖ້າ (LHS.getMinValue().ugt(RHS.getMaxValue()))
    ສົ່ງຄືນ true;
  ສົ່ງຄືນ std::nullopt;
}

std::optional<bool> KnownBits::uge(const KnownBits &LHS, const KnownBits &RHS) {
  ຖ້າ (std::optional<bool> IsUGT = ugt(RHS, LHS))
    ສົ່ງຄືນ !*IsUGT;
  ສົ່ງຄືນ std::nullopt;
}

std::optional<bool> KnownBits::ult(const KnownBits &LHS, const KnownBits &RHS) {
  ສົ່ງຄືນ ugt(RHS, LHS);
}

std::optional<bool> KnownBits::ule(const KnownBits &LHS, const KnownBits &RHS) {
  ສົ່ງຄືນ uge(RHS, LHS);
}

std::optional<bool> KnownBits::sgt(const KnownBits &LHS, const KnownBits &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  ຖ້າ (LHS.getSignedMaxValue().sle(RHS.getSignedMinValue()))
    ສົ່ງຄືນ false;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  ຖ້າ (LHS.getSignedMinValue().sgt(RHS.getSignedMaxValue()))
    ສົ່ງຄືນ true;
  ສົ່ງຄືນ std::nullopt;
}

std::optional<bool> KnownBits::sge(const KnownBits &LHS, const KnownBits &RHS) {
  ຖ້າ (std::optional<bool> KnownSGT = sgt(RHS, LHS))
    ສົ່ງຄືນ !*KnownSGT;
  ສົ່ງຄືນ std::nullopt;
}

std::optional<bool> KnownBits::slt(const KnownBits &LHS, const KnownBits &RHS) {
  ສົ່ງຄືນ sgt(RHS, LHS);
}

std::optional<bool> KnownBits::sle(const KnownBits &LHS, const KnownBits &RHS) {
  ສົ່ງຄືນ sge(RHS, LHS);
}

KnownBits KnownBits::abs(bool IntMinIsPoison) const {
  // If the source's MSB is zero then we know the rest of the bits already.
  ຖ້າ (isNonNegative())
    ສົ່ງຄືນ *this;

  // Absolute value preserves trailing zero count.
  KnownBits KnownAbs(getBitWidth());

  // If the input is negative, then abs(x) == -x.
  ຖ້າ (isNegative()) {
    KnownBits Tmp = *this;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    ຖ້າ (IntMinIsPoison && (Zero.popcount() + 2) == getBitWidth())
      Tmp.One.setBit(countMinTrailingZeros());

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
    ຖ້າ (IntMinIsPoison && Tmp.countMinPopulation() == 1 &&
        Tmp.countMaxPopulation() != 1) {
      Tmp.One.clearSignBit();
      Tmp.Zero.setSignBit();
      KnownAbs.One.setBits(getBitWidth() - Tmp.countMinLeadingZeros(),
                           getBitWidth() - 1);
    }

  } ບໍ່ດັ່ງນັ້ນ {
    unsigned MaxTZ = countMaxTrailingZeros();
    unsigned MinTZ = countMinTrailingZeros();

    KnownAbs.Zero.setLowBits(MinTZ);
    // If we know the lowest set 1, then preserve it.
    ຖ້າ (MaxTZ == MinTZ && MaxTZ < getBitWidth())
      KnownAbs.One.setBit(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    ຖ້າ (IntMinIsPoison || (!One.isZero() && !One.isMinSignedValue())) {
      KnownAbs.One.clearSignBit();
      KnownAbs.Zero.setSignBit();
    }
  }

  ສົ່ງຄືນ KnownAbs;
}

KnownBits KnownBits::reduceAdd(unsigned NumElts) const {
  ຖ້າ (NumElts == 0)
    ສົ່ງຄືນ KnownBits(getBitWidth());

  unsigned BitWidth = getBitWidth();
  KnownBits Result(BitWidth);

  ຖ້າ (isConstant())
    // If all elements are the same constant, we can simply compute it
    ສົ່ງຄືນ KnownBits::makeConstant(NumElts * getConstant());

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
  unsigned LostBits = Log2_32_Ceil(NumElts);

  ຖ້າ (isNonNegative()) {
    unsigned LeadingZeros = countMinLeadingZeros();
    LeadingZeros = LeadingZeros > LostBits ? LeadingZeros - LostBits : 0;
    Result.Zero.setHighBits(LeadingZeros);
  } ບໍ່ດັ່ງນັ້ນ ຖ້າ (isNegative()) {
    unsigned LeadingOnes = countMinLeadingOnes();
    LeadingOnes = LeadingOnes > LostBits ? LeadingOnes - LostBits : 0;
    Result.One.setHighBits(LeadingOnes);
  }

  ສົ່ງຄືນ Result;
}

static KnownBits computeForSatAddSub(bool Add, bool Signed,
                                     const KnownBits &LHS,
                                     const KnownBits &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  unsigned BitWidth = LHS.getBitWidth();

  std::optional<bool> Overflow;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  bool MayNegClamp = true;
  bool MayPosClamp = true;
  ຖ້າ (Signed) {
    // Easy cases we can rule out any overflow.
    ຖ້າ (Add && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      Overflow = false;
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (!Add && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      Overflow = false;
    ບໍ່ດັ່ງນັ້ນ {
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
      ຖ້າ (Add) {
        ຖ້າ (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = false;
          // Pos + Pos will overflow with extra signbit.
          ຖ້າ (LHS.isNonNegative() && RHS.isNonNegative())
            Overflow = true;
        } ບໍ່ດັ່ງນັ້ນ ຖ້າ (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = false;
          // Neg + Neg will overflow without extra signbit.
          ຖ້າ (LHS.isNegative() && RHS.isNegative())
            Overflow = true;
        }
        // We will never clamp to the opposite sign of N-bit result.
        ຖ້າ (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = false;
        ຖ້າ (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = false;
      } ບໍ່ດັ່ງນັ້ນ {
        ຖ້າ (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = false;
          // Neg - Pos will overflow with extra signbit.
          ຖ້າ (LHS.isNegative() && RHS.isNonNegative())
            Overflow = true;
        } ບໍ່ດັ່ງນັ້ນ ຖ້າ (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = false;
          // Pos - Neg will overflow without extra signbit.
          ຖ້າ (LHS.isNonNegative() && RHS.isNegative())
            Overflow = true;
        }
        // We will never clamp to the opposite sign of N-bit result.
        ຖ້າ (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = false;
        ຖ້າ (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = false;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    ຖ້າ (!MayNegClamp && !MayPosClamp)
      Overflow = false;
  } ບໍ່ດັ່ງນັ້ນ ຖ້າ (Add) {
    // uadd.sat
    bool Of;
    (void)LHS.getMaxValue().uadd_ov(RHS.getMaxValue(), Of);
    ຖ້າ (!Of) {
      Overflow = false;
    } ບໍ່ດັ່ງນັ້ນ {
      (void)LHS.getMinValue().uadd_ov(RHS.getMinValue(), Of);
      ຖ້າ (Of)
        Overflow = true;
    }
  } ບໍ່ດັ່ງນັ້ນ {
    // usub.sat
    bool Of;
    (void)LHS.getMinValue().usub_ov(RHS.getMaxValue(), Of);
    ຖ້າ (!Of) {
      Overflow = false;
    } ບໍ່ດັ່ງນັ້ນ {
      (void)LHS.getMaxValue().usub_ov(RHS.getMinValue(), Of);
      ຖ້າ (Of)
        Overflow = true;
    }
  }

  KnownBits Res = KnownBits::computeForAddSub(Add, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  ຖ້າ (Overflow) {
    // We know whether or not we overflowed.
    ຖ້າ (!(*Overflow)) {
      // No overflow.
      ສົ່ງຄືນ Res;
    }

    // We overflowed
    APInt C;
    ຖ້າ (Signed) {
      // sadd.sat / ssub.sat
      assert(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::getSignedMinValue(BitWidth)
                           : APInt::getSignedMaxValue(BitWidth);
    } ບໍ່ດັ່ງນັ້ນ ຖ້າ (Add) {
      // uadd.sat
      C = APInt::getMaxValue(BitWidth);
    } ບໍ່ດັ່ງນັ້ນ {
      // uadd.sat
      C = APInt::getMinValue(BitWidth);
    }

    Res.One = C;
    Res.Zero = ~C;
    ສົ່ງຄືນ Res;
  }

  // We don't know if we overflowed.
  ຖ້າ (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    ຖ້າ (MayPosClamp)
      Res.Zero.clearLowBits(BitWidth - 1);
    ຖ້າ (MayNegClamp)
      Res.One.clearLowBits(BitWidth - 1);
  } ບໍ່ດັ່ງນັ້ນ ຖ້າ (Add) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clearAllBits();
  } ບໍ່ດັ່ງນັ້ນ {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clearAllBits();
  }

  ສົ່ງຄືນ Res;
}

KnownBits KnownBits::sadd_sat(const KnownBits &LHS, const KnownBits &RHS) {
  ສົ່ງຄືນ computeForSatAddSub(/*Add*/ true, /*Signed*/ true, LHS, RHS);
}
KnownBits KnownBits::ssub_sat(const KnownBits &LHS, const KnownBits &RHS) {
  ສົ່ງຄືນ computeForSatAddSub(/*Add*/ false, /*Signed*/ true, LHS, RHS);
}
KnownBits KnownBits::uadd_sat(const KnownBits &LHS, const KnownBits &RHS) {
  ສົ່ງຄືນ computeForSatAddSub(/*Add*/ true, /*Signed*/ false, LHS, RHS);
}
KnownBits KnownBits::usub_sat(const KnownBits &LHS, const KnownBits &RHS) {
  ສົ່ງຄືນ computeForSatAddSub(/*Add*/ false, /*Signed*/ false, LHS, RHS);
}

static KnownBits avgComputeU(KnownBits LHS, KnownBits RHS, bool IsCeil) {
  unsigned BitWidth = LHS.getBitWidth();
  LHS = LHS.zext(BitWidth + 1);
  RHS = RHS.zext(BitWidth + 1);
  LHS =
      computeForAddCarry(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.extractBits(BitWidth, 1);
  ສົ່ງຄືນ LHS;
}

KnownBits KnownBits::avgFloorS(const KnownBits &LHS, const KnownBits &RHS) {
  ສົ່ງຄືນ flipSignBit(avgFloorU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgFloorU(const KnownBits &LHS, const KnownBits &RHS) {
  ສົ່ງຄືນ avgComputeU(LHS, RHS, /*IsCeil=*/false);
}

KnownBits KnownBits::avgCeilS(const KnownBits &LHS, const KnownBits &RHS) {
  ສົ່ງຄືນ flipSignBit(avgCeilU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgCeilU(const KnownBits &LHS, const KnownBits &RHS) {
  ສົ່ງຄືນ avgComputeU(LHS, RHS, /*IsCeil=*/true);
}

KnownBits KnownBits::mul(const KnownBits &LHS, const KnownBits &RHS,
                         bool NoUndefSelfMultiply) {
  unsigned BitWidth = LHS.getBitWidth();
  assert(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  assert((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.getMaxValue();
  APInt UMaxRHS = RHS.getMaxValue();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  bool HasOverflow;
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
  unsigned TrailZeroLHS = LHS.countMinTrailingZeros();
  unsigned TrailZeroRHS = RHS.countMinTrailingZeros();
  unsigned TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  unsigned SmallestOperand = std::min(TrailBitsKnownLHS - TrailZeroLHS,
                                      TrailBitsKnownRHS - TrailZeroRHS);
  unsigned ResultBitsKnown = std::min(SmallestOperand + TrailZ, BitWidth);

  // The lower ResultBitsKnown bits of this are known.
  APInt BottomKnown = LHS.One * RHS.One;

  KnownBits Res(BitWidth);
  Res.Zero.setHighBits(LeadZ);
  Res.Zero |= (~BottomKnown).getLoBits(ResultBitsKnown);
  Res.One = BottomKnown.getLoBits(ResultBitsKnown);

  ຖ້າ (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    unsigned TwoTZP1 = 2 * TrailZeroLHS + 1;
    ຖ້າ (TwoTZP1 < BitWidth)
      Res.Zero.setBit(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    ຖ້າ (TrailZeroLHS < BitWidth && LHS.One[TrailZeroLHS]) {
      unsigned TwoTZP2 = TwoTZP1 + 1;
      ຖ້າ (TwoTZP2 < BitWidth)
        Res.Zero.setBit(TwoTZP2);
    }
  }

  ສົ່ງຄືນ Res;
}

KnownBits KnownBits::mulhs(const KnownBits &LHS, const KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  assert(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.sext(2 * BitWidth);
  KnownBits WideRHS = RHS.sext(2 * BitWidth);
  ສົ່ງຄືນ mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

KnownBits KnownBits::mulhu(const KnownBits &LHS, const KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  assert(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.zext(2 * BitWidth);
  KnownBits WideRHS = RHS.zext(2 * BitWidth);
  ສົ່ງຄືນ mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

static KnownBits divComputeLowBit(KnownBits Known, const KnownBits &LHS,
                                  const KnownBits &RHS, bool Exact) {

  ຖ້າ (!Exact)
    ສົ່ງຄືນ Known;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  ຖ້າ (LHS.One[0])
    Known.One.setBit(0);

  ຈຳນວນເຕັມ MinTZ =
      (ຈຳນວນເຕັມ)LHS.countMinTrailingZeros() - (ຈຳນວນເຕັມ)RHS.countMaxTrailingZeros();
  ຈຳນວນເຕັມ MaxTZ =
      (ຈຳນວນເຕັມ)LHS.countMaxTrailingZeros() - (ຈຳນວນເຕັມ)RHS.countMinTrailingZeros();
  ຖ້າ (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    Known.Zero.setLowBits(MinTZ);
    ຖ້າ (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      Known.One.setBit(MinTZ);
    }
  } ບໍ່ດັ່ງນັ້ນ ຖ້າ (MaxTZ < 0) {
    // Poison Result
    Known.setAllZero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  ຖ້າ (Known.hasConflict())
    Known.setAllZero();

  ສົ່ງຄືນ Known;
}

KnownBits KnownBits::sdiv(const KnownBits &LHS, const KnownBits &RHS,
                          bool Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  ຖ້າ (LHS.isNonNegative() && RHS.isNonNegative())
    ສົ່ງຄືນ udiv(LHS, RHS, Exact);

  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  ຖ້າ (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    ສົ່ງຄືນ Known;
  }

  std::optional<APInt> Res;
  ຖ້າ (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.getSignedMaxValue();
    APInt Num = LHS.getSignedMinValue();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (Num.isMinSignedValue() && Denom.isAllOnes())
              ? APInt::getSignedMaxValue(BitWidth)
              : Num.sdiv(Denom);
  } ບໍ່ດັ່ງນັ້ນ ຖ້າ (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    ຖ້າ (Exact || (-LHS.getSignedMaxValue()).uge(RHS.getSignedMaxValue())) {
      APInt Denom = RHS.getSignedMinValue();
      APInt Num = LHS.getSignedMinValue();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      ຖ້າ (Denom.isZero())
        Denom.setBit(RHS.countMinTrailingZeros());
      Res = Num.sdiv(Denom);
    }
  } ບໍ່ດັ່ງນັ້ນ ຖ້າ (LHS.isStrictlyPositive() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    ຖ້າ (Exact || LHS.getSignedMinValue().uge(-RHS.getSignedMinValue())) {
      APInt Denom = RHS.getSignedMaxValue();
      APInt Num = LHS.getSignedMaxValue();
      Res = Num.sdiv(Denom);
    }
  }

  ຖ້າ (Res) {
    ຖ້າ (Res->isNonNegative()) {
      unsigned LeadZ = Res->countLeadingZeros();
      Known.Zero.setHighBits(LeadZ);
    } ບໍ່ດັ່ງນັ້ນ {
      unsigned LeadO = Res->countLeadingOnes();
      Known.One.setHighBits(LeadO);
    }
  }

  Known = divComputeLowBit(Known, LHS, RHS, Exact);
  ສົ່ງຄືນ Known;
}

KnownBits KnownBits::udiv(const KnownBits &LHS, const KnownBits &RHS,
                          bool Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  ຖ້າ (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    ສົ່ງຄືນ Known;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.getMinValue();
  ຖ້າ (MinDenom.isZero())
    MinDenom.setBit(RHS.countMinTrailingZeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.getMaxValue().udiv(MinDenom);
  APInt MinRes = LHS.getMinValue().udiv(RHS.getMaxValue());

  Known = KnownBits::makeConstant(MinRes);
  ຖ້າ (std::optional<unsigned> DifferentBit =
          APIntOps::GetMostSignificantDifferentBit(MinRes, MaxRes)) {
    Known.Zero.clearLowBits(*DifferentBit + 1);
    Known.One.clearLowBits(*DifferentBit + 1);
  }

  Known = divComputeLowBit(Known, LHS, RHS, Exact);

  ສົ່ງຄືນ Known;
}

KnownBits KnownBits::remGetLowBits(const KnownBits &LHS, const KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  ຖ້າ (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    unsigned RHSZeros = RHS.countMinTrailingZeros();
    APInt Mask = APInt::getLowBitsSet(BitWidth, RHSZeros);
    APInt OnesMask = LHS.One & Mask;
    APInt ZerosMask = LHS.Zero & Mask;
    ສົ່ງຄືນ KnownBits(ZerosMask, OnesMask);
  }
  ສົ່ງຄືນ KnownBits(BitWidth);
}

KnownBits KnownBits::urem(const KnownBits &LHS, const KnownBits &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  ຖ້າ (LHS.getMaxValue().ult(RHS.getMinValue()))
    ສົ່ງຄືນ LHS;

  KnownBits Known = remGetLowBits(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.getMaxValue(), RHS.getMaxValue() - 1);
  Known.Zero.setHighBits(MaxRes.countLeadingZeros());
  ສົ່ງຄືນ Known;
}

KnownBits KnownBits::srem(const KnownBits &LHS, const KnownBits &RHS) {
  KnownBits Known = remGetLowBits(LHS, RHS);
  ຖ້າ (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt LowBits = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    ຖ້າ (LHS.isNonNegative() || LowBits.isSubsetOf(LHS.Zero))
      Known.Zero |= ~LowBits;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    ຖ້າ (LHS.isNegative() && LowBits.intersects(LHS.One))
      Known.One |= ~LowBits;
    ສົ່ງຄືນ Known;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  ຖ້າ (LHS.isNegative() && Known.isNonZero())
    Known.One.setHighBits(
        std::max(LHS.countMinLeadingOnes(), RHS.countMinSignBits()));
  ບໍ່ດັ່ງນັ້ນ ຖ້າ (LHS.isNonNegative())
    Known.Zero.setHighBits(
        std::max(LHS.countMinLeadingZeros(), RHS.countMinSignBits()));
  ສົ່ງຄືນ Known;
}

KnownBits &KnownBits::operator&=(const KnownBits &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  ສົ່ງຄືນ *this;
}

KnownBits &KnownBits::operator|=(const KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  ສົ່ງຄືນ *this;
}

KnownBits &KnownBits::operator^=(const KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  ສົ່ງຄືນ *this;
}

KnownBits KnownBits::blsi() const {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(Zero, APInt(BitWidth, 0));
  unsigned Max = countMaxTrailingZeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = countMinTrailingZeros();
  ຖ້າ (Max == Min && Max < BitWidth)
    Known.One.setBit(Max);
  ສົ່ງຄືນ Known;
}

KnownBits KnownBits::blsmsk() const {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(BitWidth);
  unsigned Max = countMaxTrailingZeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = countMinTrailingZeros();
  Known.One.setLowBits(std::min(Min + 1, BitWidth));
  ສົ່ງຄືນ Known;
}

void KnownBits::print(raw_ostream &OS) const {
  unsigned BitWidth = getBitWidth();
  ສຳລັບ (unsigned I = 0; I < BitWidth; ++I) {
    unsigned N = BitWidth - I - 1;
    ຖ້າ (Zero[N] && One[N])
      OS << "!";
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (Zero[N])
      OS << "0";
    ບໍ່ດັ່ງນັ້ນ ຖ້າ (One[N])
      OS << "1";
    ບໍ່ດັ່ງນັ້ນ
      OS << "?";
  }
}

#ຖ້າ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void KnownBits::dump() const {
  print(dbgs());
  dbgs() << "\n";
}
#endif

unsigned llvm::SignBitsOps::rot(unsigned SrcSignBits, unsigned BitWidth,
                                std::optional<APInt> RotAmt,
                                bool IsRotateRight) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  ຖ້າ (SrcSignBits == BitWidth)
    ສົ່ງຄືນ BitWidth;

  ຖ້າ (!RotAmt)
    ສົ່ງຄືນ 1;

  unsigned Amt = RotAmt->urem(BitWidth);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  ຖ້າ (IsRotateRight)
    Amt = (BitWidth - Amt) % BitWidth;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  ຖ້າ (SrcSignBits > Amt + 1)
    ສົ່ງຄືນ SrcSignBits - Amt;
  ສົ່ງຄືນ 1;
}
