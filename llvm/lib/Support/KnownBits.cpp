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

#অন্তর্ভাৱ "llvm/Support/KnownBits.h"
#অন্তর্ভাৱ "llvm/ADT/Sequence.h"
#অন্তর্ভাৱ "llvm/Support/Debug.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"
#অন্তর্ভাৱ <cassert>

using নামাকাশ llvm;

KnownBits KnownBits::flipSignBit(ধ্রুৱ KnownBits &Val) {
  unsigned SignBitPosition = Val.getBitWidth() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
  One.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
  ঘূৰাই KnownBits(Zero, One);
}

স্থির KnownBits computeForAddCarry(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS,
                                    তার্কিক CarryZero, তার্কিক CarryOne) {

  APInt PossibleSumZero = LHS.get_Max_মান() + RHS.get_Max_মান() + !CarryZero;
  APInt PossibleSumOne = LHS.get_Min_মান() + RHS.get_Min_মান() + CarryOne;

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
  ঘূৰাই KnownOut;
}

KnownBits KnownBits::computeForAddCarry(
    ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS, ধ্রুৱ KnownBits &Carry) {
  প্রতিজ্ঞা(Carry.getBitWidth() == 1 && "Carry must be 1-bit");
  ঘূৰাই ::computeForAddCarry(
      LHS, RHS, Carry.Zero.get_Bool_মান(), Carry.One.get_Bool_মান());
}

KnownBits KnownBits::computeForAddSub(তার্কিক Add, তার্কিক NSW, তার্কিক NUW,
                                      ধ্রুৱ KnownBits &LHS,
                                      ধ্রুৱ KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits KnownOut(BitWidth);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  যদি (LHS.isUnknown() && RHS.isUnknown())
    ঘূৰাই KnownOut;

  যদি (!LHS.isUnknown() && !RHS.isUnknown()) {
    যদি (Add) {
      // Sum = LHS + RHS + 0
      KnownOut = ::computeForAddCarry(LHS, RHS, /*CarryZero=*/সঁচা,
                                      /*CarryOne=*/মিছা);
    } অন্যথা {
      // Sum = LHS + ~RHS + 1
      KnownBits NotRHS = RHS;
      std::swap(NotRHS.Zero, NotRHS.One);
      KnownOut = ::computeForAddCarry(LHS, NotRHS, /*CarryZero=*/মিছা,
                                      /*CarryOne=*/সঁচা);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  যদি (NUW) {
    যদি (Add) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_মান().uadd_sat(RHS.get_Min_মান());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      যদি (NSW) {
        unsigned অঙ্ক_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        KnownOut.One.setBits(BitWidth - 1 - অঙ্ক_Bits_2, BitWidth - 1);
      }
      KnownOut.One.set_উপৰেৰ_Bits(MinVal.countl_one());
    } অন্যথা {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_মান().usub_sat(RHS.get_Min_মান());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      যদি (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        unsigned অঙ্ক_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
        KnownOut.Zero.setBits(BitWidth - 1 - অঙ্ক_Bits_2, BitWidth - 1);
      }
      KnownOut.Zero.set_উপৰেৰ_Bits(MaxVal.countl_zero());
    }
  }

  যদি (NSW) {
    APInt MinVal;
    APInt MaxVal;
    যদি (Add) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_মান().sadd_sat(RHS.get_Signed_Min_মান());
      MaxVal = LHS.get_Signed_Max_মান().sadd_sat(RHS.get_Signed_Max_মান());
    } অন্যথা {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_মান().ssub_sat(RHS.get_Signed_Max_মান());
      MaxVal = LHS.get_Signed_Max_মান().ssub_sat(RHS.get_Signed_Min_মান());
    }
    যদি (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      unsigned অঙ্ক_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
      KnownOut.One.setBits(BitWidth - 1 - অঙ্ক_Bits_2, BitWidth - 1);
      KnownOut.Zero.setSignBit();
    }
    যদি (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      unsigned অঙ্ক_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
      KnownOut.Zero.setBits(BitWidth - 1 - অঙ্ক_Bits_2, BitWidth - 1);
      KnownOut.One.setSignBit();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  যদি (KnownOut.hasConflict())
    KnownOut.setAllZero();
  ঘূৰাই KnownOut;
}

KnownBits KnownBits::computeForSubBorrow(ধ্রুৱ KnownBits &LHS, KnownBits RHS,
                                         ধ্রুৱ KnownBits &Borrow) {
  প্রতিজ্ঞা(Borrow.getBitWidth() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::swap(RHS.Zero, RHS.One);
  ঘূৰাই ::computeForAddCarry(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_মান(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_মান());
}

KnownBits KnownBits::truncSSat(unsigned BitWidth) ধ্রুৱ {
  unsigned প্ৰবেশ_Bits = getBitWidth();
  APInt MinInRange = APInt::get_Signed_Min_মান(BitWidth).sext(প্ৰবেশ_Bits);
  APInt MaxInRange = APInt::get_Signed_Max_মান(BitWidth).sext(প্ৰবেশ_Bits);
  APInt প্ৰবেশ_Min = get_Signed_Min_মান();
  APInt প্ৰবেশ_Max = get_Signed_Max_মান();
  KnownBits Known(BitWidth);

  // Case 1: All values fit - just truncate
  যদি (প্ৰবেশ_Min.sge(MinInRange) && প্ৰবেশ_Max.sle(MaxInRange)) {
    Known = trunc(BitWidth);
  }
  // Case 2: All saturate to min
  অন্যথা যদি (প্ৰবেশ_Max.slt(MinInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Min_মান(BitWidth));
  }
  // Case 3: All saturate to max
  অন্যথা যদি (প্ৰবেশ_Min.sgt(MaxInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Max_মান(BitWidth));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  অন্যথা যদি (প্ৰবেশ_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    Known.Zero = APInt(BitWidth, 0);
    Known.Zero.setBit(BitWidth - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    Known.One = One.trunc(BitWidth);
    Known.One.clearBit(BitWidth - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  অন্যথা যদি (প্ৰবেশ_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    Known.Zero = Zero.trunc(BitWidth);
    Known.Zero.clearBit(BitWidth - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    Known.One = APInt(BitWidth, 0);
    Known.One.setBit(BitWidth - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  অন্যথা {
    // Output: min saturation, truncated, or max saturation
    APInt প্ৰবেশ_Zero_Lower = Zero.trunc(BitWidth);
    APInt প্ৰবেশ_One_Lower = One.trunc(BitWidth);
    APInt MinSat = APInt::get_Signed_Min_মান(BitWidth);
    APInt MaxSat = APInt::get_Signed_Max_মান(BitWidth);
    KnownBits MinSatKB = KnownBits::makeConstant(MinSat);
    KnownBits MaxSatKB = KnownBits::makeConstant(MaxSat);
    যদি (প্ৰবেশ_Max.sle(MaxInRange)) {
      // Positive values fit, only negatives saturate to min
      Known.Zero = প্ৰবেশ_Zero_Lower & MinSatKB.Zero;
      Known.One = প্ৰবেশ_One_Lower & MinSatKB.One;
    } অন্যথা যদি (প্ৰবেশ_Min.sge(MinInRange)) {
      // Negative values fit, only positives saturate to max
      Known.Zero = প্ৰবেশ_Zero_Lower & MaxSatKB.Zero;
      Known.One = প্ৰবেশ_One_Lower & MaxSatKB.One;
    } অন্যথা {
      // Both positive and negative values might saturate
      Known.Zero = প্ৰবেশ_Zero_Lower & MinSatKB.Zero & MaxSatKB.Zero;
      Known.One = প্ৰবেশ_One_Lower & MinSatKB.One & MaxSatKB.One;
    }
  }
  ঘূৰাই Known;
}

KnownBits KnownBits::truncSSatU(unsigned BitWidth) ধ্রুৱ {
  unsigned প্ৰবেশ_Bits = getBitWidth();
  APInt MaxInRange = APInt::getAllOnes(BitWidth).zext(প্ৰবেশ_Bits);
  APInt প্ৰবেশ_Min = get_Signed_Min_মান();
  APInt প্ৰবেশ_Max = get_Signed_Max_মান();
  KnownBits Known(BitWidth);

  যদি (isNegative()) {
    Known.setAllZero();
  } অন্যথা যদি (প্ৰবেশ_Min.isNonNegative() && প্ৰবেশ_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } অন্যথা যদি (প্ৰবেশ_Min.isNonNegative() && প্ৰবেশ_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } অন্যথা যদি (প্ৰবেশ_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    Known.Zero = APInt(BitWidth, 0);
    // Common one bits: bits that are 1 in input stay 1
    Known.One = One.trunc(BitWidth);
  } অন্যথা {
    // Mixed: sign bit unknown
    যদি (প্ৰবেশ_Max.ule(MaxInRange)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      Known.Zero = Zero.trunc(BitWidth);
      Known.One = APInt(BitWidth, 0);
    } অন্যথা {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      Known.Zero = APInt(BitWidth, 0);
      Known.One = APInt(BitWidth, 0);
    }
  }
  ঘূৰাই Known;
}

KnownBits KnownBits::truncUSat(unsigned BitWidth) ধ্রুৱ {
  unsigned প্ৰবেশ_Bits = getBitWidth();
  APInt MaxInRange = APInt::get_নিচেৰ_Bits_Set(প্ৰবেশ_Bits, BitWidth);
  APInt প্ৰবেশ_Max = get_Max_মান();
  APInt প্ৰবেশ_Min = get_Min_মান();
  KnownBits Known(BitWidth);

  যদি (প্ৰবেশ_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } অন্যথা যদি (প্ৰবেশ_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } অন্যথা {
    Known.resetAll();
    Known.One = One.trunc(BitWidth);
  }
  ঘূৰাই Known;
}

KnownBits KnownBits::sextInReg(unsigned SrcBitWidth) ধ্রুৱ {
  unsigned BitWidth = getBitWidth();
  প্রতিজ্ঞা(0 < SrcBitWidth && SrcBitWidth <= BitWidth &&
         "Illegal sext-in-register");

  যদি (SrcBitWidth == BitWidth)
    ঘূৰাই *এতত্;

  unsigned ExtBits = BitWidth - SrcBitWidth;
  KnownBits Result;
  Result.One = One << ExtBits;
  Result.Zero = Zero << ExtBits;
  Result.One.ashrInPlace(ExtBits);
  Result.Zero.ashrInPlace(ExtBits);
  ঘূৰাই Result;
}

KnownBits KnownBits::makeGE(ধ্রুৱ APInt &Val) ধ্রুৱ {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  unsigned N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_নিচেৰ_Bits(getBitWidth() - N);
  ঘূৰাই KnownBits(Zero, One | MaskedVal);
}

KnownBits KnownBits::umax(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  যদি (LHS.get_Min_মান().uge(RHS.get_Max_মান()))
    ঘূৰাই LHS;
  যদি (RHS.get_Min_মান().uge(LHS.get_Max_মান()))
    ঘূৰাই RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  KnownBits L = LHS.makeGE(RHS.get_Min_মান());
  KnownBits R = RHS.makeGE(LHS.get_Min_মান());
  ঘূৰাই L.intersectWith(R);
}

KnownBits KnownBits::umin(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](ধ্রুৱ KnownBits &Val) { ঘূৰাই KnownBits(Val.One, Val.Zero); };
  ঘূৰাই Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::smax(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  ঘূৰাই flipSignBit(umax(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::smin(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](ধ্রুৱ KnownBits &Val) {
    unsigned SignBitPosition = Val.getBitWidth() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
    One.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
    ঘূৰাই KnownBits(Zero, One);
  };
  ঘূৰাই Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::abdu(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  যদি (LHS.get_Min_মান().uge(RHS.get_Max_মান()))
    ঘূৰাই computeForAddSub(/*Add=*/মিছা, /*NSW=*/মিছা, /*NUW=*/মিছা, LHS,
                            RHS);
  যদি (RHS.get_Min_মান().uge(LHS.get_Max_মান()))
    ঘূৰাই computeForAddSub(/*Add=*/মিছা, /*NSW=*/মিছা, /*NUW=*/মিছা, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/মিছা, /*NSW=*/মিছা, /*NUW=*/সঁচা, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/মিছা, /*NSW=*/মিছা, /*NUW=*/সঁচা, RHS, LHS);
  ঘূৰাই Diff0.intersectWith(Diff1);
}

KnownBits KnownBits::abds(KnownBits LHS, KnownBits RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  যদি (LHS.get_Signed_Min_মান().sge(RHS.get_Signed_Max_মান()))
    ঘূৰাই computeForAddSub(/*Add=*/মিছা, /*NSW=*/মিছা, /*NUW=*/মিছা, LHS,
                            RHS);
  যদি (RHS.get_Signed_Min_মান().sge(LHS.get_Signed_Max_মান()))
    ঘূৰাই computeForAddSub(/*Add=*/মিছা, /*NSW=*/মিছা, /*NUW=*/মিছা, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  unsigned SignBitPosition = LHS.getBitWidth() - 1;
  বাবে (auto Arg : {&LHS, &RHS}) {
    তার্কিক Tmp = Arg->Zero[SignBitPosition];
    Arg->Zero.setBitVal(SignBitPosition, Arg->One[SignBitPosition]);
    Arg->One.setBitVal(SignBitPosition, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/মিছা, /*NSW=*/মিছা, /*NUW=*/সঁচা, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/মিছা, /*NSW=*/মিছা, /*NUW=*/সঁচা, RHS, LHS);
  ঘূৰাই Diff0.intersectWith(Diff1);
}

স্থির unsigned getMaxShiftAmount(ধ্রুৱ APInt &Max_মান, unsigned BitWidth) {
  যদি (isPowerOf2_32(BitWidth))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    ঘূৰাই Max_মান.extract_Bits_As_ZExt_মান(
        std::min(Log2_32(BitWidth), Max_মান.getBitWidth()), 0);
  // This is only an approximate upper bound.
  ঘূৰাই Max_মান.get_Limited_মান(BitWidth - 1);
}

KnownBits KnownBits::shl(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS, তার্কিক NUW,
                         তার্কিক NSW, তার্কিক ShAmtNonZero) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ধ্রুৱ KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known;
    তার্কিক ShiftedOutZero, ShiftedOutOne;
    Known.Zero = LHS.Zero.ushl_ov(ShiftAmt, ShiftedOutZero);
    Known.Zero.set_নিচেৰ_Bits(ShiftAmt);
    Known.One = LHS.One.ushl_ov(ShiftAmt, ShiftedOutOne);

    // All cases returning poison have been handled by MaxShiftAmount already.
    যদি (NSW) {
      যদি (NUW && ShiftAmt != 0)
        // NUW means we can assume anything shifted out was a zero.
        ShiftedOutZero = সঁচা;

      যদি (ShiftedOutZero)
        Known.makeNonNegative();
      অন্যথা যদি (ShiftedOutOne)
        Known.makeNegative();
    }
    ঘূৰাই Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_মান().get_Limited_মান(BitWidth);
  যদি (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  যদি (LHS.isUnknown()) {
    Known.Zero.set_নিচেৰ_Bits(MinShiftAmount);
    যদি (NUW && NSW && MinShiftAmount != 0)
      Known.makeNonNegative();
    ঘূৰাই Known;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_মান = RHS.get_Max_মান();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_মান, BitWidth);
  যদি (NUW && NSW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.গুনতি_Max_Leading_Zeros() - 1);
  যদি (NUW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.গুনতি_Max_Leading_Zeros());
  যদি (NSW)
    MaxShiftAmount = std::min(
        MaxShiftAmount,
        std::max(LHS.গুনতি_Max_Leading_Zeros(), LHS.গুনতি_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  যদি (MinShiftAmount == 0 && MaxShiftAmount == BitWidth - 1 &&
      isPowerOf2_32(BitWidth)) {
    Known.Zero.set_নিচেৰ_Bits(LHS.গুনতি_Min_Trailing_Zeros());
    যদি (LHS.isAllOnes())
      Known.One.setSignBit();
    যদি (NSW) {
      যদি (LHS.isNonNegative())
        Known.makeNonNegative();
      যদি (LHS.isNegative())
        Known.makeNegative();
    }
    ঘূৰাই Known;
  }

  // Find the common bits from all possible shifts.
  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_মান();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_মান();
  Known.setAllConflict();
  বাবে (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    যদি ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      অনুৱর্ত;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    যদি (Known.isUnknown())
      ভঙ্গ;
  }

  // All shift amounts may result in poison.
  যদি (Known.hasConflict())
    Known.setAllZero();
  ঘূৰাই Known;
}

KnownBits KnownBits::lshr(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS,
                          তার্কিক ShAmtNonZero, তার্কিক Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ধ্রুৱ KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known >>= ShiftAmt;
    // High bits are known zero.
    Known.Zero.set_উপৰেৰ_Bits(ShiftAmt);
    ঘূৰাই Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_মান().get_Limited_মান(BitWidth);
  যদি (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  যদি (LHS.isUnknown()) {
    Known.Zero.set_উপৰেৰ_Bits(MinShiftAmount);
    ঘূৰাই Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_মান = RHS.get_Max_মান();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_মান, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  যদি (Exact) {
    unsigned FirstOne = LHS.গুনতি_Max_Trailing_Zeros();
    যদি (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      ঘূৰাই Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_মান();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_মান();
  Known.setAllConflict();
  বাবে (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    যদি ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      অনুৱর্ত;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    যদি (Known.isUnknown())
      ভঙ্গ;
  }

  // All shift amounts may result in poison.
  যদি (Known.hasConflict())
    Known.setAllZero();
  ঘূৰাই Known;
}

KnownBits KnownBits::ashr(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS,
                          তার্কিক ShAmtNonZero, তার্কিক Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ধ্রুৱ KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known.Zero.ashrInPlace(ShiftAmt);
    Known.One.ashrInPlace(ShiftAmt);
    ঘূৰাই Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_মান().get_Limited_মান(BitWidth);
  যদি (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  যদি (LHS.isUnknown()) {
    যদি (MinShiftAmount == BitWidth) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      ঘূৰাই Known;
    }
    ঘূৰাই Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_মান = RHS.get_Max_মান();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_মান, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  যদি (Exact) {
    unsigned FirstOne = LHS.গুনতি_Max_Trailing_Zeros();
    যদি (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      ঘূৰাই Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_মান();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_মান();
  Known.setAllConflict();
  বাবে (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
      ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    যদি ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      অনুৱর্ত;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    যদি (Known.isUnknown())
      ভঙ্গ;
  }

  // All shift amounts may result in poison.
  যদি (Known.hasConflict())
    Known.setAllZero();
  ঘূৰাই Known;
}

KnownBits KnownBits::fshl(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS,
                          ধ্রুৱ APInt &Amt) {
  ঘূৰাই KnownBits(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::fshr(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS,
                          ধ্রুৱ APInt &Amt) {
  ঘূৰাই KnownBits(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::clmul(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  KnownBits Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_মান(), RHS.get_Min_মান()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  auto ClMulOr = [](ধ্রুৱ APInt &LHS, ধ্রুৱ APInt &RHS) {
    unsigned BW = LHS.getBitWidth();
    প্রতিজ্ঞা(BW == RHS.getBitWidth() && "Operand mismatch");
    APInt Result(BW, 0);
    বাবে (unsigned I :
         seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
      যদি (RHS[I])
        Result |= LHS << I;
    ঘূৰাই Result;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt Known = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= Known;
  Res.One &= Known;

  ঘূৰাই Res;
}

KnownBits KnownBits::pext(ধ্রুৱ KnownBits &Val, ধ্রুৱ KnownBits &Mask) {
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
  বাবে (unsigned I = 0; I < BitWidth; ++I) {
    যদি (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      যদি (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clearBits(M0, M1 + 1);
      যদি (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clearBits(M0, M1 + 1);
    }
    যদি (Mask.One[I])
      ++M0, ++M1;
    অন্যথা যদি (!Mask.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clearBits(M0, BitWidth);
  ঘূৰাই Res;
}

KnownBits KnownBits::pdep(ধ্রুৱ KnownBits &Val, ধ্রুৱ KnownBits &Mask) {
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
  বাবে (unsigned I = 0; I < BitWidth; ++I) {
    যদি (!Mask.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clearBit(I);
    যদি (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::getBitsSet(BitWidth, M0, M1 + 1);
      যদি (!Range.isSubsetOf(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clearBit(I);
      যদি (!Range.isSubsetOf(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clearBit(I);
    }
    যদি (Mask.One[I])
      ++M0, ++M1;
    অন্যথা যদি (!Mask.Zero[I])
      ++M1;
  }
  ঘূৰাই Res;
}

std::optional<তার্কিক> KnownBits::eq(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  যদি (LHS.isConstant() && RHS.isConstant())
    ঘূৰাই LHS.getConstant() == RHS.getConstant();
  যদি (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    ঘূৰাই মিছা;
  ঘূৰাই std::nullopt;
}

std::optional<তার্কিক> KnownBits::ne(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  যদি (std::optional<তার্কিক> KnownEQ = eq(LHS, RHS))
    ঘূৰাই !*KnownEQ;
  ঘূৰাই std::nullopt;
}

std::optional<তার্কিক> KnownBits::ugt(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  যদি (LHS.get_Max_মান().ule(RHS.get_Min_মান()))
    ঘূৰাই মিছা;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  যদি (LHS.get_Min_মান().ugt(RHS.get_Max_মান()))
    ঘূৰাই সঁচা;
  ঘূৰাই std::nullopt;
}

std::optional<তার্কিক> KnownBits::uge(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  যদি (std::optional<তার্কিক> IsUGT = ugt(RHS, LHS))
    ঘূৰাই !*IsUGT;
  ঘূৰাই std::nullopt;
}

std::optional<তার্কিক> KnownBits::ult(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  ঘূৰাই ugt(RHS, LHS);
}

std::optional<তার্কিক> KnownBits::ule(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  ঘূৰাই uge(RHS, LHS);
}

std::optional<তার্কিক> KnownBits::sgt(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  যদি (LHS.get_Signed_Max_মান().sle(RHS.get_Signed_Min_মান()))
    ঘূৰাই মিছা;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  যদি (LHS.get_Signed_Min_মান().sgt(RHS.get_Signed_Max_মান()))
    ঘূৰাই সঁচা;
  ঘূৰাই std::nullopt;
}

std::optional<তার্কিক> KnownBits::sge(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  যদি (std::optional<তার্কিক> KnownSGT = sgt(RHS, LHS))
    ঘূৰাই !*KnownSGT;
  ঘূৰাই std::nullopt;
}

std::optional<তার্কিক> KnownBits::slt(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  ঘূৰাই sgt(RHS, LHS);
}

std::optional<তার্কিক> KnownBits::sle(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  ঘূৰাই sge(RHS, LHS);
}

KnownBits KnownBits::abs(তার্কিক IntMinIsPoison) ধ্রুৱ {
  // If the source's MSB is zero then we know the rest of the bits already.
  যদি (isNonNegative())
    ঘূৰাই *এতত্;

  // Absolute value preserves trailing zero count.
  KnownBits KnownAbs(getBitWidth());

  // If the input is negative, then abs(x) == -x.
  যদি (isNegative()) {
    KnownBits Tmp = *এতত্;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    যদি (IntMinIsPoison && (Zero.popcount() + 2) == getBitWidth())
      Tmp.One.setBit(গুনতি_Min_Trailing_Zeros());

    KnownAbs = computeForAddSub(
        /*Add*/ মিছা, IntMinIsPoison, /*NUW=*/মিছা,
        KnownBits::makeConstant(APInt(getBitWidth(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    যদি (IntMinIsPoison && Tmp.গুনতি_Min_Population() == 1 &&
        Tmp.গুনতি_Max_Population() != 1) {
      Tmp.One.clearSignBit();
      Tmp.Zero.setSignBit();
      KnownAbs.One.setBits(getBitWidth() - Tmp.গুনতি_Min_Leading_Zeros(),
                           getBitWidth() - 1);
    }

  } অন্যথা {
    unsigned MaxTZ = গুনতি_Max_Trailing_Zeros();
    unsigned MinTZ = গুনতি_Min_Trailing_Zeros();

    KnownAbs.Zero.set_নিচেৰ_Bits(MinTZ);
    // If we know the lowest set 1, then preserve it.
    যদি (MaxTZ == MinTZ && MaxTZ < getBitWidth())
      KnownAbs.One.setBit(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    যদি (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_মান())) {
      KnownAbs.One.clearSignBit();
      KnownAbs.Zero.setSignBit();
    }
  }

  ঘূৰাই KnownAbs;
}

KnownBits KnownBits::reduceAdd(unsigned অঙ্ক_Elts) ধ্রুৱ {
  যদি (অঙ্ক_Elts == 0)
    ঘূৰাই KnownBits(getBitWidth());

  unsigned BitWidth = getBitWidth();
  KnownBits Result(BitWidth);

  যদি (isConstant())
    // If all elements are the same constant, we can simply compute it
    ঘূৰাই KnownBits::makeConstant(অঙ্ক_Elts * getConstant());

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
  unsigned LostBits = Log2_32_Ceil(অঙ্ক_Elts);

  যদি (isNonNegative()) {
    unsigned LeadingZeros = গুনতি_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > LostBits ? LeadingZeros - LostBits : 0;
    Result.Zero.set_উপৰেৰ_Bits(LeadingZeros);
  } অন্যথা যদি (isNegative()) {
    unsigned LeadingOnes = গুনতি_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > LostBits ? LeadingOnes - LostBits : 0;
    Result.One.set_উপৰেৰ_Bits(LeadingOnes);
  }

  ঘূৰাই Result;
}

স্থির KnownBits computeForSatAddSub(তার্কিক Add, তার্কিক Signed,
                                     ধ্রুৱ KnownBits &LHS,
                                     ধ্রুৱ KnownBits &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  unsigned BitWidth = LHS.getBitWidth();

  std::optional<তার্কিক> Overflow;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  তার্কিক MayNegClamp = সঁচা;
  তার্কিক MayPosClamp = সঁচা;
  যদি (Signed) {
    // Easy cases we can rule out any overflow.
    যদি (Add && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      Overflow = মিছা;
    অন্যথা যদি (!Add && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      Overflow = মিছা;
    অন্যথা {
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
          KnownBits::computeForAddSub(Add, /*NSW=*/মিছা,
                                      /*NUW=*/মিছা, UnsignedLHS, UnsignedRHS);
      যদি (Add) {
        যদি (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = মিছা;
          // Pos + Pos will overflow with extra signbit.
          যদি (LHS.isNonNegative() && RHS.isNonNegative())
            Overflow = সঁচা;
        } অন্যথা যদি (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = মিছা;
          // Neg + Neg will overflow without extra signbit.
          যদি (LHS.isNegative() && RHS.isNegative())
            Overflow = সঁচা;
        }
        // We will never clamp to the opposite sign of N-bit result.
        যদি (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = মিছা;
        যদি (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = মিছা;
      } অন্যথা {
        যদি (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = মিছা;
          // Neg - Pos will overflow with extra signbit.
          যদি (LHS.isNegative() && RHS.isNonNegative())
            Overflow = সঁচা;
        } অন্যথা যদি (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = মিছা;
          // Pos - Neg will overflow without extra signbit.
          যদি (LHS.isNonNegative() && RHS.isNegative())
            Overflow = সঁচা;
        }
        // We will never clamp to the opposite sign of N-bit result.
        যদি (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = মিছা;
        যদি (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = মিছা;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    যদি (!MayNegClamp && !MayPosClamp)
      Overflow = মিছা;
  } অন্যথা যদি (Add) {
    // uadd.sat
    তার্কিক Of;
    (শূন্য)LHS.get_Max_মান().uadd_ov(RHS.get_Max_মান(), Of);
    যদি (!Of) {
      Overflow = মিছা;
    } অন্যথা {
      (শূন্য)LHS.get_Min_মান().uadd_ov(RHS.get_Min_মান(), Of);
      যদি (Of)
        Overflow = সঁচা;
    }
  } অন্যথা {
    // usub.sat
    তার্কিক Of;
    (শূন্য)LHS.get_Min_মান().usub_ov(RHS.get_Max_মান(), Of);
    যদি (!Of) {
      Overflow = মিছা;
    } অন্যথা {
      (শূন্য)LHS.get_Max_মান().usub_ov(RHS.get_Min_মান(), Of);
      যদি (Of)
        Overflow = সঁচা;
    }
  }

  KnownBits Res = KnownBits::computeForAddSub(Add, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  যদি (Overflow) {
    // We know whether or not we overflowed.
    যদি (!(*Overflow)) {
      // No overflow.
      ঘূৰাই Res;
    }

    // We overflowed
    APInt C;
    যদি (Signed) {
      // sadd.sat / ssub.sat
      প্রতিজ্ঞা(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_মান(BitWidth)
                           : APInt::get_Signed_Max_মান(BitWidth);
    } অন্যথা যদি (Add) {
      // uadd.sat
      C = APInt::get_Max_মান(BitWidth);
    } অন্যথা {
      // uadd.sat
      C = APInt::get_Min_মান(BitWidth);
    }

    Res.One = C;
    Res.Zero = ~C;
    ঘূৰাই Res;
  }

  // We don't know if we overflowed.
  যদি (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    যদি (MayPosClamp)
      Res.Zero.clear_নিচেৰ_Bits(BitWidth - 1);
    যদি (MayNegClamp)
      Res.One.clear_নিচেৰ_Bits(BitWidth - 1);
  } অন্যথা যদি (Add) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clearAllBits();
  } অন্যথা {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clearAllBits();
  }

  ঘূৰাই Res;
}

KnownBits KnownBits::sadd_sat(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  ঘূৰাই computeForSatAddSub(/*Add*/ সঁচা, /*Signed*/ সঁচা, LHS, RHS);
}
KnownBits KnownBits::ssub_sat(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  ঘূৰাই computeForSatAddSub(/*Add*/ মিছা, /*Signed*/ সঁচা, LHS, RHS);
}
KnownBits KnownBits::uadd_sat(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  ঘূৰাই computeForSatAddSub(/*Add*/ সঁচা, /*Signed*/ মিছা, LHS, RHS);
}
KnownBits KnownBits::usub_sat(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  ঘূৰাই computeForSatAddSub(/*Add*/ মিছা, /*Signed*/ মিছা, LHS, RHS);
}

স্থির KnownBits avgComputeU(KnownBits LHS, KnownBits RHS, তার্কিক IsCeil) {
  unsigned BitWidth = LHS.getBitWidth();
  LHS = LHS.zext(BitWidth + 1);
  RHS = RHS.zext(BitWidth + 1);
  LHS =
      computeForAddCarry(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.extractBits(BitWidth, 1);
  ঘূৰাই LHS;
}

KnownBits KnownBits::avgFloorS(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  ঘূৰাই flipSignBit(avgFloorU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgFloorU(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  ঘূৰাই avgComputeU(LHS, RHS, /*IsCeil=*/মিছা);
}

KnownBits KnownBits::avgCeilS(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  ঘূৰাই flipSignBit(avgCeilU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgCeilU(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  ঘূৰাই avgComputeU(LHS, RHS, /*IsCeil=*/সঁচা);
}

KnownBits KnownBits::mul(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS,
                         তার্কিক NoUndefSelfMultiply) {
  unsigned BitWidth = LHS.getBitWidth();
  প্রতিজ্ঞা(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  প্রতিজ্ঞা((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.get_Max_মান();
  APInt UMaxRHS = RHS.get_Max_মান();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  তার্কিক HasOverflow;
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
  unsigned TrailZeroLHS = LHS.গুনতি_Min_Trailing_Zeros();
  unsigned TrailZeroRHS = RHS.গুনতি_Min_Trailing_Zeros();
  unsigned TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  unsigned SmallestOperand = std::min(TrailBitsKnownLHS - TrailZeroLHS,
                                      TrailBitsKnownRHS - TrailZeroRHS);
  unsigned ResultBitsKnown = std::min(SmallestOperand + TrailZ, BitWidth);

  // The lower ResultBitsKnown bits of this are known.
  APInt BottomKnown = LHS.One * RHS.One;

  KnownBits Res(BitWidth);
  Res.Zero.set_উপৰেৰ_Bits(LeadZ);
  Res.Zero |= (~BottomKnown).getLoBits(ResultBitsKnown);
  Res.One = BottomKnown.getLoBits(ResultBitsKnown);

  যদি (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    unsigned TwoTZP1 = 2 * TrailZeroLHS + 1;
    যদি (TwoTZP1 < BitWidth)
      Res.Zero.setBit(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    যদি (TrailZeroLHS < BitWidth && LHS.One[TrailZeroLHS]) {
      unsigned TwoTZP2 = TwoTZP1 + 1;
      যদি (TwoTZP2 < BitWidth)
        Res.Zero.setBit(TwoTZP2);
    }
  }

  ঘূৰাই Res;
}

KnownBits KnownBits::mulhs(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  প্রতিজ্ঞা(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.sext(2 * BitWidth);
  KnownBits WideRHS = RHS.sext(2 * BitWidth);
  ঘূৰাই mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

KnownBits KnownBits::mulhu(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  প্রতিজ্ঞা(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.zext(2 * BitWidth);
  KnownBits WideRHS = RHS.zext(2 * BitWidth);
  ঘূৰাই mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

স্থির KnownBits div_Compute_নিচেৰ_Bit(KnownBits Known, ধ্রুৱ KnownBits &LHS,
                                  ধ্রুৱ KnownBits &RHS, তার্কিক Exact) {

  যদি (!Exact)
    ঘূৰাই Known;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  যদি (LHS.One[0])
    Known.One.setBit(0);

  পূর্ণাঙ্ক MinTZ =
      (পূর্ণাঙ্ক)LHS.গুনতি_Min_Trailing_Zeros() - (পূর্ণাঙ্ক)RHS.গুনতি_Max_Trailing_Zeros();
  পূর্ণাঙ্ক MaxTZ =
      (পূর্ণাঙ্ক)LHS.গুনতি_Max_Trailing_Zeros() - (পূর্ণাঙ্ক)RHS.গুনতি_Min_Trailing_Zeros();
  যদি (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    Known.Zero.set_নিচেৰ_Bits(MinTZ);
    যদি (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      Known.One.setBit(MinTZ);
    }
  } অন্যথা যদি (MaxTZ < 0) {
    // Poison Result
    Known.setAllZero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  যদি (Known.hasConflict())
    Known.setAllZero();

  ঘূৰাই Known;
}

KnownBits KnownBits::sdiv(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS,
                          তার্কিক Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  যদি (LHS.isNonNegative() && RHS.isNonNegative())
    ঘূৰাই udiv(LHS, RHS, Exact);

  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  যদি (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    ঘূৰাই Known;
  }

  std::optional<APInt> Res;
  যদি (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_মান();
    APInt অঙ্ক = LHS.get_Signed_Min_মান();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (অঙ্ক.is_Min_Signed_মান() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_মান(BitWidth)
              : অঙ্ক.sdiv(Denom);
  } অন্যথা যদি (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    যদি (Exact || (-LHS.get_Signed_Max_মান()).uge(RHS.get_Signed_Max_মান())) {
      APInt Denom = RHS.get_Signed_Min_মান();
      APInt অঙ্ক = LHS.get_Signed_Min_মান();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      যদি (Denom.isZero())
        Denom.setBit(RHS.গুনতি_Min_Trailing_Zeros());
      Res = অঙ্ক.sdiv(Denom);
    }
  } অন্যথা যদি (LHS.isStrictlyPositive() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    যদি (Exact || LHS.get_Signed_Min_মান().uge(-RHS.get_Signed_Min_মান())) {
      APInt Denom = RHS.get_Signed_Max_মান();
      APInt অঙ্ক = LHS.get_Signed_Max_মান();
      Res = অঙ্ক.sdiv(Denom);
    }
  }

  যদি (Res) {
    যদি (Res->isNonNegative()) {
      unsigned LeadZ = Res->গুনতি_Leading_Zeros();
      Known.Zero.set_উপৰেৰ_Bits(LeadZ);
    } অন্যথা {
      unsigned LeadO = Res->গুনতি_Leading_Ones();
      Known.One.set_উপৰেৰ_Bits(LeadO);
    }
  }

  Known = div_Compute_নিচেৰ_Bit(Known, LHS, RHS, Exact);
  ঘূৰাই Known;
}

KnownBits KnownBits::udiv(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS,
                          তার্কিক Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  যদি (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    ঘূৰাই Known;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_মান();
  যদি (MinDenom.isZero())
    MinDenom.setBit(RHS.গুনতি_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_মান().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_মান().udiv(RHS.get_Max_মান());

  Known = KnownBits::makeConstant(MinRes);
  যদি (std::optional<unsigned> DifferentBit =
          APIntOps::GetMostSignificantDifferentBit(MinRes, MaxRes)) {
    Known.Zero.clear_নিচেৰ_Bits(*DifferentBit + 1);
    Known.One.clear_নিচেৰ_Bits(*DifferentBit + 1);
  }

  Known = div_Compute_নিচেৰ_Bit(Known, LHS, RHS, Exact);

  ঘূৰাই Known;
}

KnownBits KnownBits::rem_Get_নিচেৰ_Bits(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  যদি (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    unsigned RHSZeros = RHS.গুনতি_Min_Trailing_Zeros();
    APInt Mask = APInt::get_নিচেৰ_Bits_Set(BitWidth, RHSZeros);
    APInt OnesMask = LHS.One & Mask;
    APInt ZerosMask = LHS.Zero & Mask;
    ঘূৰাই KnownBits(ZerosMask, OnesMask);
  }
  ঘূৰাই KnownBits(BitWidth);
}

KnownBits KnownBits::urem(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  যদি (LHS.get_Max_মান().ult(RHS.get_Min_মান()))
    ঘূৰাই LHS;

  KnownBits Known = rem_Get_নিচেৰ_Bits(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_মান(), RHS.get_Max_মান() - 1);
  Known.Zero.set_উপৰেৰ_Bits(MaxRes.গুনতি_Leading_Zeros());
  ঘূৰাই Known;
}

KnownBits KnownBits::srem(ধ্রুৱ KnownBits &LHS, ধ্রুৱ KnownBits &RHS) {
  KnownBits Known = rem_Get_নিচেৰ_Bits(LHS, RHS);
  যদি (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt নিচেৰ_Bits = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    যদি (LHS.isNonNegative() || নিচেৰ_Bits.isSubsetOf(LHS.Zero))
      Known.Zero |= ~নিচেৰ_Bits;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    যদি (LHS.isNegative() && নিচেৰ_Bits.intersects(LHS.One))
      Known.One |= ~নিচেৰ_Bits;
    ঘূৰাই Known;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  যদি (LHS.isNegative() && Known.isNonZero())
    Known.One.set_উপৰেৰ_Bits(
        std::max(LHS.গুনতি_Min_Leading_Ones(), RHS.গুনতি_Min_Sign_Bits()));
  অন্যথা যদি (LHS.isNonNegative())
    Known.Zero.set_উপৰেৰ_Bits(
        std::max(LHS.গুনতি_Min_Leading_Zeros(), RHS.গুনতি_Min_Sign_Bits()));
  ঘূৰাই Known;
}

KnownBits &KnownBits::operator&=(ধ্রুৱ KnownBits &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  ঘূৰাই *এতত্;
}

KnownBits &KnownBits::operator|=(ধ্রুৱ KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  ঘূৰাই *এতত্;
}

KnownBits &KnownBits::operator^=(ধ্রুৱ KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  ঘূৰাই *এতত্;
}

KnownBits KnownBits::blsi() ধ্রুৱ {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(Zero, APInt(BitWidth, 0));
  unsigned Max = গুনতি_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = গুনতি_Min_Trailing_Zeros();
  যদি (Max == Min && Max < BitWidth)
    Known.One.setBit(Max);
  ঘূৰাই Known;
}

KnownBits KnownBits::blsmsk() ধ্রুৱ {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(BitWidth);
  unsigned Max = গুনতি_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = গুনতি_Min_Trailing_Zeros();
  Known.One.set_নিচেৰ_Bits(std::min(Min + 1, BitWidth));
  ঘূৰাই Known;
}

শূন্য KnownBits::print(raw_ostream &OS) ধ্রুৱ {
  unsigned BitWidth = getBitWidth();
  বাবে (unsigned I = 0; I < BitWidth; ++I) {
    unsigned N = BitWidth - I - 1;
    যদি (Zero[N] && One[N])
      OS << "!";
    অন্যথা যদি (Zero[N])
      OS << "0";
    অন্যথা যদি (One[N])
      OS << "1";
    অন্যথা
      OS << "?";
  }
}

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD শূন্য KnownBits::dump() ধ্রুৱ {
  print(dbgs());
  dbgs() << "\n";
}
#endif

unsigned llvm::SignBitsOps::rot(unsigned SrcSignBits, unsigned BitWidth,
                                std::optional<APInt> RotAmt,
                                তার্কিক Is_Rotate_ডান) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  যদি (SrcSignBits == BitWidth)
    ঘূৰাই BitWidth;

  যদি (!RotAmt)
    ঘূৰাই 1;

  unsigned Amt = RotAmt->urem(BitWidth);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  যদি (Is_Rotate_ডান)
    Amt = (BitWidth - Amt) % BitWidth;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  যদি (SrcSignBits > Amt + 1)
    ঘূৰাই SrcSignBits - Amt;
  ঘূৰাই 1;
}
