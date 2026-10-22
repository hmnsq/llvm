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

#அந்தர்பாவ "llvm/Support/KnownBits.h"
#அந்தர்பாவ "llvm/ADT/Sequence.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <cassert>

using நாமாகாஶ llvm;

KnownBits KnownBits::flipSignBit(மாறிலி KnownBits &Val) {
  unsigned SignBitPosition = Val.getBitWidth() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
  One.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
  திரும்பு KnownBits(Zero, One);
}

நிலையான KnownBits computeForAddCarry(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS,
                                    தருக்க CarryZero, தருக்க CarryOne) {

  APInt PossibleSumZero = LHS.get_Max_மதிப்பு() + RHS.get_Max_மதிப்பு() + !CarryZero;
  APInt PossibleSumOne = LHS.get_Min_மதிப்பு() + RHS.get_Min_மதிப்பு() + CarryOne;

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
  திரும்பு KnownOut;
}

KnownBits KnownBits::computeForAddCarry(
    மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS, மாறிலி KnownBits &Carry) {
  ப்ரதிஜ்ஞா(Carry.getBitWidth() == 1 && "Carry must be 1-bit");
  திரும்பு ::computeForAddCarry(
      LHS, RHS, Carry.Zero.get_Bool_மதிப்பு(), Carry.One.get_Bool_மதிப்பு());
}

KnownBits KnownBits::computeForAddSub(தருக்க Add, தருக்க NSW, தருக்க NUW,
                                      மாறிலி KnownBits &LHS,
                                      மாறிலி KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits KnownOut(BitWidth);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  எனில் (LHS.isUnknown() && RHS.isUnknown())
    திரும்பு KnownOut;

  எனில் (!LHS.isUnknown() && !RHS.isUnknown()) {
    எனில் (Add) {
      // Sum = LHS + RHS + 0
      KnownOut = ::computeForAddCarry(LHS, RHS, /*CarryZero=*/உண்மை,
                                      /*CarryOne=*/பொய்);
    } இல்லையெனில் {
      // Sum = LHS + ~RHS + 1
      KnownBits NotRHS = RHS;
      std::swap(NotRHS.Zero, NotRHS.One);
      KnownOut = ::computeForAddCarry(LHS, NotRHS, /*CarryZero=*/பொய்,
                                      /*CarryOne=*/உண்மை);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  எனில் (NUW) {
    எனில் (Add) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_மதிப்பு().uadd_sat(RHS.get_Min_மதிப்பு());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      எனில் (NSW) {
        unsigned இலக்கம்_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        KnownOut.One.setBits(BitWidth - 1 - இலக்கம்_Bits_2, BitWidth - 1);
      }
      KnownOut.One.set_உயர்வு_Bits(MinVal.countl_one());
    } இல்லையெனில் {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_மதிப்பு().usub_sat(RHS.get_Min_மதிப்பு());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      எனில் (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        unsigned இலக்கம்_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
        KnownOut.Zero.setBits(BitWidth - 1 - இலக்கம்_Bits_2, BitWidth - 1);
      }
      KnownOut.Zero.set_உயர்வு_Bits(MaxVal.countl_zero());
    }
  }

  எனில் (NSW) {
    APInt MinVal;
    APInt MaxVal;
    எனில் (Add) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_மதிப்பு().sadd_sat(RHS.get_Signed_Min_மதிப்பு());
      MaxVal = LHS.get_Signed_Max_மதிப்பு().sadd_sat(RHS.get_Signed_Max_மதிப்பு());
    } இல்லையெனில் {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_மதிப்பு().ssub_sat(RHS.get_Signed_Max_மதிப்பு());
      MaxVal = LHS.get_Signed_Max_மதிப்பு().ssub_sat(RHS.get_Signed_Min_மதிப்பு());
    }
    எனில் (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      unsigned இலக்கம்_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
      KnownOut.One.setBits(BitWidth - 1 - இலக்கம்_Bits_2, BitWidth - 1);
      KnownOut.Zero.setSignBit();
    }
    எனில் (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      unsigned இலக்கம்_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
      KnownOut.Zero.setBits(BitWidth - 1 - இலக்கம்_Bits_2, BitWidth - 1);
      KnownOut.One.setSignBit();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  எனில் (KnownOut.hasConflict())
    KnownOut.setAllZero();
  திரும்பு KnownOut;
}

KnownBits KnownBits::computeForSubBorrow(மாறிலி KnownBits &LHS, KnownBits RHS,
                                         மாறிலி KnownBits &Borrow) {
  ப்ரதிஜ்ஞா(Borrow.getBitWidth() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::swap(RHS.Zero, RHS.One);
  திரும்பு ::computeForAddCarry(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_மதிப்பு(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_மதிப்பு());
}

KnownBits KnownBits::truncSSat(unsigned BitWidth) மாறிலி {
  unsigned உள்ளீடு_Bits = getBitWidth();
  APInt MinInRange = APInt::get_Signed_Min_மதிப்பு(BitWidth).sext(உள்ளீடு_Bits);
  APInt MaxInRange = APInt::get_Signed_Max_மதிப்பு(BitWidth).sext(உள்ளீடு_Bits);
  APInt உள்ளீடு_Min = get_Signed_Min_மதிப்பு();
  APInt உள்ளீடு_Max = get_Signed_Max_மதிப்பு();
  KnownBits Known(BitWidth);

  // Case 1: All values fit - just truncate
  எனில் (உள்ளீடு_Min.sge(MinInRange) && உள்ளீடு_Max.sle(MaxInRange)) {
    Known = trunc(BitWidth);
  }
  // Case 2: All saturate to min
  இல்லையெனில் எனில் (உள்ளீடு_Max.slt(MinInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Min_மதிப்பு(BitWidth));
  }
  // Case 3: All saturate to max
  இல்லையெனில் எனில் (உள்ளீடு_Min.sgt(MaxInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Max_மதிப்பு(BitWidth));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  இல்லையெனில் எனில் (உள்ளீடு_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    Known.Zero = APInt(BitWidth, 0);
    Known.Zero.setBit(BitWidth - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    Known.One = One.trunc(BitWidth);
    Known.One.clearBit(BitWidth - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  இல்லையெனில் எனில் (உள்ளீடு_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    Known.Zero = Zero.trunc(BitWidth);
    Known.Zero.clearBit(BitWidth - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    Known.One = APInt(BitWidth, 0);
    Known.One.setBit(BitWidth - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  இல்லையெனில் {
    // Output: min saturation, truncated, or max saturation
    APInt உள்ளீடு_Zero_Lower = Zero.trunc(BitWidth);
    APInt உள்ளீடு_One_Lower = One.trunc(BitWidth);
    APInt MinSat = APInt::get_Signed_Min_மதிப்பு(BitWidth);
    APInt MaxSat = APInt::get_Signed_Max_மதிப்பு(BitWidth);
    KnownBits MinSatKB = KnownBits::makeConstant(MinSat);
    KnownBits MaxSatKB = KnownBits::makeConstant(MaxSat);
    எனில் (உள்ளீடு_Max.sle(MaxInRange)) {
      // Positive values fit, only negatives saturate to min
      Known.Zero = உள்ளீடு_Zero_Lower & MinSatKB.Zero;
      Known.One = உள்ளீடு_One_Lower & MinSatKB.One;
    } இல்லையெனில் எனில் (உள்ளீடு_Min.sge(MinInRange)) {
      // Negative values fit, only positives saturate to max
      Known.Zero = உள்ளீடு_Zero_Lower & MaxSatKB.Zero;
      Known.One = உள்ளீடு_One_Lower & MaxSatKB.One;
    } இல்லையெனில் {
      // Both positive and negative values might saturate
      Known.Zero = உள்ளீடு_Zero_Lower & MinSatKB.Zero & MaxSatKB.Zero;
      Known.One = உள்ளீடு_One_Lower & MinSatKB.One & MaxSatKB.One;
    }
  }
  திரும்பு Known;
}

KnownBits KnownBits::truncSSatU(unsigned BitWidth) மாறிலி {
  unsigned உள்ளீடு_Bits = getBitWidth();
  APInt MaxInRange = APInt::getAllOnes(BitWidth).zext(உள்ளீடு_Bits);
  APInt உள்ளீடு_Min = get_Signed_Min_மதிப்பு();
  APInt உள்ளீடு_Max = get_Signed_Max_மதிப்பு();
  KnownBits Known(BitWidth);

  எனில் (isNegative()) {
    Known.setAllZero();
  } இல்லையெனில் எனில் (உள்ளீடு_Min.isNonNegative() && உள்ளீடு_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } இல்லையெனில் எனில் (உள்ளீடு_Min.isNonNegative() && உள்ளீடு_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } இல்லையெனில் எனில் (உள்ளீடு_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    Known.Zero = APInt(BitWidth, 0);
    // Common one bits: bits that are 1 in input stay 1
    Known.One = One.trunc(BitWidth);
  } இல்லையெனில் {
    // Mixed: sign bit unknown
    எனில் (உள்ளீடு_Max.ule(MaxInRange)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      Known.Zero = Zero.trunc(BitWidth);
      Known.One = APInt(BitWidth, 0);
    } இல்லையெனில் {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      Known.Zero = APInt(BitWidth, 0);
      Known.One = APInt(BitWidth, 0);
    }
  }
  திரும்பு Known;
}

KnownBits KnownBits::truncUSat(unsigned BitWidth) மாறிலி {
  unsigned உள்ளீடு_Bits = getBitWidth();
  APInt MaxInRange = APInt::get_கீழ்_Bits_Set(உள்ளீடு_Bits, BitWidth);
  APInt உள்ளீடு_Max = get_Max_மதிப்பு();
  APInt உள்ளீடு_Min = get_Min_மதிப்பு();
  KnownBits Known(BitWidth);

  எனில் (உள்ளீடு_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } இல்லையெனில் எனில் (உள்ளீடு_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } இல்லையெனில் {
    Known.resetAll();
    Known.One = One.trunc(BitWidth);
  }
  திரும்பு Known;
}

KnownBits KnownBits::sextInReg(unsigned SrcBitWidth) மாறிலி {
  unsigned BitWidth = getBitWidth();
  ப்ரதிஜ்ஞா(0 < SrcBitWidth && SrcBitWidth <= BitWidth &&
         "Illegal sext-in-register");

  எனில் (SrcBitWidth == BitWidth)
    திரும்பு *இது;

  unsigned ExtBits = BitWidth - SrcBitWidth;
  KnownBits Result;
  Result.One = One << ExtBits;
  Result.Zero = Zero << ExtBits;
  Result.One.ashrInPlace(ExtBits);
  Result.Zero.ashrInPlace(ExtBits);
  திரும்பு Result;
}

KnownBits KnownBits::makeGE(மாறிலி APInt &Val) மாறிலி {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  unsigned N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_கீழ்_Bits(getBitWidth() - N);
  திரும்பு KnownBits(Zero, One | MaskedVal);
}

KnownBits KnownBits::umax(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  எனில் (LHS.get_Min_மதிப்பு().uge(RHS.get_Max_மதிப்பு()))
    திரும்பு LHS;
  எனில் (RHS.get_Min_மதிப்பு().uge(LHS.get_Max_மதிப்பு()))
    திரும்பு RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  KnownBits L = LHS.makeGE(RHS.get_Min_மதிப்பு());
  KnownBits R = RHS.makeGE(LHS.get_Min_மதிப்பு());
  திரும்பு L.intersectWith(R);
}

KnownBits KnownBits::umin(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](மாறிலி KnownBits &Val) { திரும்பு KnownBits(Val.One, Val.Zero); };
  திரும்பு Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::smax(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  திரும்பு flipSignBit(umax(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::smin(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](மாறிலி KnownBits &Val) {
    unsigned SignBitPosition = Val.getBitWidth() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
    One.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
    திரும்பு KnownBits(Zero, One);
  };
  திரும்பு Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::abdu(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  எனில் (LHS.get_Min_மதிப்பு().uge(RHS.get_Max_மதிப்பு()))
    திரும்பு computeForAddSub(/*Add=*/பொய், /*NSW=*/பொய், /*NUW=*/பொய், LHS,
                            RHS);
  எனில் (RHS.get_Min_மதிப்பு().uge(LHS.get_Max_மதிப்பு()))
    திரும்பு computeForAddSub(/*Add=*/பொய், /*NSW=*/பொய், /*NUW=*/பொய், RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/பொய், /*NSW=*/பொய், /*NUW=*/உண்மை, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/பொய், /*NSW=*/பொய், /*NUW=*/உண்மை, RHS, LHS);
  திரும்பு Diff0.intersectWith(Diff1);
}

KnownBits KnownBits::abds(KnownBits LHS, KnownBits RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  எனில் (LHS.get_Signed_Min_மதிப்பு().sge(RHS.get_Signed_Max_மதிப்பு()))
    திரும்பு computeForAddSub(/*Add=*/பொய், /*NSW=*/பொய், /*NUW=*/பொய், LHS,
                            RHS);
  எனில் (RHS.get_Signed_Min_மதிப்பு().sge(LHS.get_Signed_Max_மதிப்பு()))
    திரும்பு computeForAddSub(/*Add=*/பொய், /*NSW=*/பொய், /*NUW=*/பொய், RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  unsigned SignBitPosition = LHS.getBitWidth() - 1;
  க்கு (auto Arg : {&LHS, &RHS}) {
    தருக்க Tmp = Arg->Zero[SignBitPosition];
    Arg->Zero.setBitVal(SignBitPosition, Arg->One[SignBitPosition]);
    Arg->One.setBitVal(SignBitPosition, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/பொய், /*NSW=*/பொய், /*NUW=*/உண்மை, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/பொய், /*NSW=*/பொய், /*NUW=*/உண்மை, RHS, LHS);
  திரும்பு Diff0.intersectWith(Diff1);
}

நிலையான unsigned getMaxShiftAmount(மாறிலி APInt &Max_மதிப்பு, unsigned BitWidth) {
  எனில் (isPowerOf2_32(BitWidth))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    திரும்பு Max_மதிப்பு.extract_Bits_As_ZExt_மதிப்பு(
        std::min(Log2_32(BitWidth), Max_மதிப்பு.getBitWidth()), 0);
  // This is only an approximate upper bound.
  திரும்பு Max_மதிப்பு.get_Limited_மதிப்பு(BitWidth - 1);
}

KnownBits KnownBits::shl(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS, தருக்க NUW,
                         தருக்க NSW, தருக்க ShAmtNonZero) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](மாறிலி KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known;
    தருக்க ShiftedOutZero, ShiftedOutOne;
    Known.Zero = LHS.Zero.ushl_ov(ShiftAmt, ShiftedOutZero);
    Known.Zero.set_கீழ்_Bits(ShiftAmt);
    Known.One = LHS.One.ushl_ov(ShiftAmt, ShiftedOutOne);

    // All cases returning poison have been handled by MaxShiftAmount already.
    எனில் (NSW) {
      எனில் (NUW && ShiftAmt != 0)
        // NUW means we can assume anything shifted out was a zero.
        ShiftedOutZero = உண்மை;

      எனில் (ShiftedOutZero)
        Known.makeNonNegative();
      இல்லையெனில் எனில் (ShiftedOutOne)
        Known.makeNegative();
    }
    திரும்பு Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_மதிப்பு().get_Limited_மதிப்பு(BitWidth);
  எனில் (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  எனில் (LHS.isUnknown()) {
    Known.Zero.set_கீழ்_Bits(MinShiftAmount);
    எனில் (NUW && NSW && MinShiftAmount != 0)
      Known.makeNonNegative();
    திரும்பு Known;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_மதிப்பு = RHS.get_Max_மதிப்பு();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_மதிப்பு, BitWidth);
  எனில் (NUW && NSW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.மொத்தம்_Max_Leading_Zeros() - 1);
  எனில் (NUW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.மொத்தம்_Max_Leading_Zeros());
  எனில் (NSW)
    MaxShiftAmount = std::min(
        MaxShiftAmount,
        std::max(LHS.மொத்தம்_Max_Leading_Zeros(), LHS.மொத்தம்_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  எனில் (MinShiftAmount == 0 && MaxShiftAmount == BitWidth - 1 &&
      isPowerOf2_32(BitWidth)) {
    Known.Zero.set_கீழ்_Bits(LHS.மொத்தம்_Min_Trailing_Zeros());
    எனில் (LHS.isAllOnes())
      Known.One.setSignBit();
    எனில் (NSW) {
      எனில் (LHS.isNonNegative())
        Known.makeNonNegative();
      எனில் (LHS.isNegative())
        Known.makeNegative();
    }
    திரும்பு Known;
  }

  // Find the common bits from all possible shifts.
  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_மதிப்பு();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_மதிப்பு();
  Known.setAllConflict();
  க்கு (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    எனில் ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      தொடர்;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    எனில் (Known.isUnknown())
      நிறுத்து;
  }

  // All shift amounts may result in poison.
  எனில் (Known.hasConflict())
    Known.setAllZero();
  திரும்பு Known;
}

KnownBits KnownBits::lshr(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS,
                          தருக்க ShAmtNonZero, தருக்க Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](மாறிலி KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known >>= ShiftAmt;
    // High bits are known zero.
    Known.Zero.set_உயர்வு_Bits(ShiftAmt);
    திரும்பு Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_மதிப்பு().get_Limited_மதிப்பு(BitWidth);
  எனில் (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  எனில் (LHS.isUnknown()) {
    Known.Zero.set_உயர்வு_Bits(MinShiftAmount);
    திரும்பு Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_மதிப்பு = RHS.get_Max_மதிப்பு();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_மதிப்பு, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  எனில் (Exact) {
    unsigned FirstOne = LHS.மொத்தம்_Max_Trailing_Zeros();
    எனில் (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      திரும்பு Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_மதிப்பு();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_மதிப்பு();
  Known.setAllConflict();
  க்கு (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    எனில் ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      தொடர்;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    எனில் (Known.isUnknown())
      நிறுத்து;
  }

  // All shift amounts may result in poison.
  எனில் (Known.hasConflict())
    Known.setAllZero();
  திரும்பு Known;
}

KnownBits KnownBits::ashr(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS,
                          தருக்க ShAmtNonZero, தருக்க Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](மாறிலி KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known.Zero.ashrInPlace(ShiftAmt);
    Known.One.ashrInPlace(ShiftAmt);
    திரும்பு Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_மதிப்பு().get_Limited_மதிப்பு(BitWidth);
  எனில் (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  எனில் (LHS.isUnknown()) {
    எனில் (MinShiftAmount == BitWidth) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      திரும்பு Known;
    }
    திரும்பு Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_மதிப்பு = RHS.get_Max_மதிப்பு();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_மதிப்பு, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  எனில் (Exact) {
    unsigned FirstOne = LHS.மொத்தம்_Max_Trailing_Zeros();
    எனில் (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      திரும்பு Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_மதிப்பு();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_மதிப்பு();
  Known.setAllConflict();
  க்கு (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
      ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    எனில் ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      தொடர்;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    எனில் (Known.isUnknown())
      நிறுத்து;
  }

  // All shift amounts may result in poison.
  எனில் (Known.hasConflict())
    Known.setAllZero();
  திரும்பு Known;
}

KnownBits KnownBits::fshl(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS,
                          மாறிலி APInt &Amt) {
  திரும்பு KnownBits(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::fshr(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS,
                          மாறிலி APInt &Amt) {
  திரும்பு KnownBits(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::clmul(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  KnownBits Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_மதிப்பு(), RHS.get_Min_மதிப்பு()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  auto ClMulOr = [](மாறிலி APInt &LHS, மாறிலி APInt &RHS) {
    unsigned BW = LHS.getBitWidth();
    ப்ரதிஜ்ஞா(BW == RHS.getBitWidth() && "Operand mismatch");
    APInt Result(BW, 0);
    க்கு (unsigned I :
         seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
      எனில் (RHS[I])
        Result |= LHS << I;
    திரும்பு Result;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt Known = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= Known;
  Res.One &= Known;

  திரும்பு Res;
}

KnownBits KnownBits::pext(மாறிலி KnownBits &Val, மாறிலி KnownBits &Mask) {
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
  க்கு (unsigned I = 0; I < BitWidth; ++I) {
    எனில் (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      எனில் (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clearBits(M0, M1 + 1);
      எனில் (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clearBits(M0, M1 + 1);
    }
    எனில் (Mask.One[I])
      ++M0, ++M1;
    இல்லையெனில் எனில் (!Mask.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clearBits(M0, BitWidth);
  திரும்பு Res;
}

KnownBits KnownBits::pdep(மாறிலி KnownBits &Val, மாறிலி KnownBits &Mask) {
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
  க்கு (unsigned I = 0; I < BitWidth; ++I) {
    எனில் (!Mask.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clearBit(I);
    எனில் (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::getBitsSet(BitWidth, M0, M1 + 1);
      எனில் (!Range.isSubsetOf(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clearBit(I);
      எனில் (!Range.isSubsetOf(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clearBit(I);
    }
    எனில் (Mask.One[I])
      ++M0, ++M1;
    இல்லையெனில் எனில் (!Mask.Zero[I])
      ++M1;
  }
  திரும்பு Res;
}

std::optional<தருக்க> KnownBits::eq(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  எனில் (LHS.isConstant() && RHS.isConstant())
    திரும்பு LHS.getConstant() == RHS.getConstant();
  எனில் (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    திரும்பு பொய்;
  திரும்பு std::nullopt;
}

std::optional<தருக்க> KnownBits::ne(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  எனில் (std::optional<தருக்க> KnownEQ = eq(LHS, RHS))
    திரும்பு !*KnownEQ;
  திரும்பு std::nullopt;
}

std::optional<தருக்க> KnownBits::ugt(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  எனில் (LHS.get_Max_மதிப்பு().ule(RHS.get_Min_மதிப்பு()))
    திரும்பு பொய்;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  எனில் (LHS.get_Min_மதிப்பு().ugt(RHS.get_Max_மதிப்பு()))
    திரும்பு உண்மை;
  திரும்பு std::nullopt;
}

std::optional<தருக்க> KnownBits::uge(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  எனில் (std::optional<தருக்க> IsUGT = ugt(RHS, LHS))
    திரும்பு !*IsUGT;
  திரும்பு std::nullopt;
}

std::optional<தருக்க> KnownBits::ult(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  திரும்பு ugt(RHS, LHS);
}

std::optional<தருக்க> KnownBits::ule(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  திரும்பு uge(RHS, LHS);
}

std::optional<தருக்க> KnownBits::sgt(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  எனில் (LHS.get_Signed_Max_மதிப்பு().sle(RHS.get_Signed_Min_மதிப்பு()))
    திரும்பு பொய்;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  எனில் (LHS.get_Signed_Min_மதிப்பு().sgt(RHS.get_Signed_Max_மதிப்பு()))
    திரும்பு உண்மை;
  திரும்பு std::nullopt;
}

std::optional<தருக்க> KnownBits::sge(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  எனில் (std::optional<தருக்க> KnownSGT = sgt(RHS, LHS))
    திரும்பு !*KnownSGT;
  திரும்பு std::nullopt;
}

std::optional<தருக்க> KnownBits::slt(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  திரும்பு sgt(RHS, LHS);
}

std::optional<தருக்க> KnownBits::sle(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  திரும்பு sge(RHS, LHS);
}

KnownBits KnownBits::abs(தருக்க IntMinIsPoison) மாறிலி {
  // If the source's MSB is zero then we know the rest of the bits already.
  எனில் (isNonNegative())
    திரும்பு *இது;

  // Absolute value preserves trailing zero count.
  KnownBits KnownAbs(getBitWidth());

  // If the input is negative, then abs(x) == -x.
  எனில் (isNegative()) {
    KnownBits Tmp = *இது;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    எனில் (IntMinIsPoison && (Zero.popcount() + 2) == getBitWidth())
      Tmp.One.setBit(மொத்தம்_Min_Trailing_Zeros());

    KnownAbs = computeForAddSub(
        /*Add*/ பொய், IntMinIsPoison, /*NUW=*/பொய்,
        KnownBits::makeConstant(APInt(getBitWidth(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    எனில் (IntMinIsPoison && Tmp.மொத்தம்_Min_Population() == 1 &&
        Tmp.மொத்தம்_Max_Population() != 1) {
      Tmp.One.clearSignBit();
      Tmp.Zero.setSignBit();
      KnownAbs.One.setBits(getBitWidth() - Tmp.மொத்தம்_Min_Leading_Zeros(),
                           getBitWidth() - 1);
    }

  } இல்லையெனில் {
    unsigned MaxTZ = மொத்தம்_Max_Trailing_Zeros();
    unsigned MinTZ = மொத்தம்_Min_Trailing_Zeros();

    KnownAbs.Zero.set_கீழ்_Bits(MinTZ);
    // If we know the lowest set 1, then preserve it.
    எனில் (MaxTZ == MinTZ && MaxTZ < getBitWidth())
      KnownAbs.One.setBit(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    எனில் (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_மதிப்பு())) {
      KnownAbs.One.clearSignBit();
      KnownAbs.Zero.setSignBit();
    }
  }

  திரும்பு KnownAbs;
}

KnownBits KnownBits::reduceAdd(unsigned இலக்கம்_Elts) மாறிலி {
  எனில் (இலக்கம்_Elts == 0)
    திரும்பு KnownBits(getBitWidth());

  unsigned BitWidth = getBitWidth();
  KnownBits Result(BitWidth);

  எனில் (isConstant())
    // If all elements are the same constant, we can simply compute it
    திரும்பு KnownBits::makeConstant(இலக்கம்_Elts * getConstant());

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
  unsigned LostBits = Log2_32_Ceil(இலக்கம்_Elts);

  எனில் (isNonNegative()) {
    unsigned LeadingZeros = மொத்தம்_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > LostBits ? LeadingZeros - LostBits : 0;
    Result.Zero.set_உயர்வு_Bits(LeadingZeros);
  } இல்லையெனில் எனில் (isNegative()) {
    unsigned LeadingOnes = மொத்தம்_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > LostBits ? LeadingOnes - LostBits : 0;
    Result.One.set_உயர்வு_Bits(LeadingOnes);
  }

  திரும்பு Result;
}

நிலையான KnownBits computeForSatAddSub(தருக்க Add, தருக்க Signed,
                                     மாறிலி KnownBits &LHS,
                                     மாறிலி KnownBits &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  unsigned BitWidth = LHS.getBitWidth();

  std::optional<தருக்க> Overflow;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  தருக்க MayNegClamp = உண்மை;
  தருக்க MayPosClamp = உண்மை;
  எனில் (Signed) {
    // Easy cases we can rule out any overflow.
    எனில் (Add && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      Overflow = பொய்;
    இல்லையெனில் எனில் (!Add && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      Overflow = பொய்;
    இல்லையெனில் {
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
          KnownBits::computeForAddSub(Add, /*NSW=*/பொய்,
                                      /*NUW=*/பொய், UnsignedLHS, UnsignedRHS);
      எனில் (Add) {
        எனில் (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = பொய்;
          // Pos + Pos will overflow with extra signbit.
          எனில் (LHS.isNonNegative() && RHS.isNonNegative())
            Overflow = உண்மை;
        } இல்லையெனில் எனில் (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = பொய்;
          // Neg + Neg will overflow without extra signbit.
          எனில் (LHS.isNegative() && RHS.isNegative())
            Overflow = உண்மை;
        }
        // We will never clamp to the opposite sign of N-bit result.
        எனில் (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = பொய்;
        எனில் (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = பொய்;
      } இல்லையெனில் {
        எனில் (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = பொய்;
          // Neg - Pos will overflow with extra signbit.
          எனில் (LHS.isNegative() && RHS.isNonNegative())
            Overflow = உண்மை;
        } இல்லையெனில் எனில் (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = பொய்;
          // Pos - Neg will overflow without extra signbit.
          எனில் (LHS.isNonNegative() && RHS.isNegative())
            Overflow = உண்மை;
        }
        // We will never clamp to the opposite sign of N-bit result.
        எனில் (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = பொய்;
        எனில் (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = பொய்;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    எனில் (!MayNegClamp && !MayPosClamp)
      Overflow = பொய்;
  } இல்லையெனில் எனில் (Add) {
    // uadd.sat
    தருக்க Of;
    (வெறுமை)LHS.get_Max_மதிப்பு().uadd_ov(RHS.get_Max_மதிப்பு(), Of);
    எனில் (!Of) {
      Overflow = பொய்;
    } இல்லையெனில் {
      (வெறுமை)LHS.get_Min_மதிப்பு().uadd_ov(RHS.get_Min_மதிப்பு(), Of);
      எனில் (Of)
        Overflow = உண்மை;
    }
  } இல்லையெனில் {
    // usub.sat
    தருக்க Of;
    (வெறுமை)LHS.get_Min_மதிப்பு().usub_ov(RHS.get_Max_மதிப்பு(), Of);
    எனில் (!Of) {
      Overflow = பொய்;
    } இல்லையெனில் {
      (வெறுமை)LHS.get_Max_மதிப்பு().usub_ov(RHS.get_Min_மதிப்பு(), Of);
      எனில் (Of)
        Overflow = உண்மை;
    }
  }

  KnownBits Res = KnownBits::computeForAddSub(Add, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  எனில் (Overflow) {
    // We know whether or not we overflowed.
    எனில் (!(*Overflow)) {
      // No overflow.
      திரும்பு Res;
    }

    // We overflowed
    APInt C;
    எனில் (Signed) {
      // sadd.sat / ssub.sat
      ப்ரதிஜ்ஞா(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_மதிப்பு(BitWidth)
                           : APInt::get_Signed_Max_மதிப்பு(BitWidth);
    } இல்லையெனில் எனில் (Add) {
      // uadd.sat
      C = APInt::get_Max_மதிப்பு(BitWidth);
    } இல்லையெனில் {
      // uadd.sat
      C = APInt::get_Min_மதிப்பு(BitWidth);
    }

    Res.One = C;
    Res.Zero = ~C;
    திரும்பு Res;
  }

  // We don't know if we overflowed.
  எனில் (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    எனில் (MayPosClamp)
      Res.Zero.clear_கீழ்_Bits(BitWidth - 1);
    எனில் (MayNegClamp)
      Res.One.clear_கீழ்_Bits(BitWidth - 1);
  } இல்லையெனில் எனில் (Add) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clearAllBits();
  } இல்லையெனில் {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clearAllBits();
  }

  திரும்பு Res;
}

KnownBits KnownBits::sadd_sat(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  திரும்பு computeForSatAddSub(/*Add*/ உண்மை, /*Signed*/ உண்மை, LHS, RHS);
}
KnownBits KnownBits::ssub_sat(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  திரும்பு computeForSatAddSub(/*Add*/ பொய், /*Signed*/ உண்மை, LHS, RHS);
}
KnownBits KnownBits::uadd_sat(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  திரும்பு computeForSatAddSub(/*Add*/ உண்மை, /*Signed*/ பொய், LHS, RHS);
}
KnownBits KnownBits::usub_sat(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  திரும்பு computeForSatAddSub(/*Add*/ பொய், /*Signed*/ பொய், LHS, RHS);
}

நிலையான KnownBits avgComputeU(KnownBits LHS, KnownBits RHS, தருக்க IsCeil) {
  unsigned BitWidth = LHS.getBitWidth();
  LHS = LHS.zext(BitWidth + 1);
  RHS = RHS.zext(BitWidth + 1);
  LHS =
      computeForAddCarry(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.extractBits(BitWidth, 1);
  திரும்பு LHS;
}

KnownBits KnownBits::avgFloorS(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  திரும்பு flipSignBit(avgFloorU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgFloorU(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  திரும்பு avgComputeU(LHS, RHS, /*IsCeil=*/பொய்);
}

KnownBits KnownBits::avgCeilS(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  திரும்பு flipSignBit(avgCeilU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgCeilU(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  திரும்பு avgComputeU(LHS, RHS, /*IsCeil=*/உண்மை);
}

KnownBits KnownBits::mul(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS,
                         தருக்க NoUndefSelfMultiply) {
  unsigned BitWidth = LHS.getBitWidth();
  ப்ரதிஜ்ஞா(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  ப்ரதிஜ்ஞா((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.get_Max_மதிப்பு();
  APInt UMaxRHS = RHS.get_Max_மதிப்பு();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  தருக்க HasOverflow;
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
  unsigned TrailZeroLHS = LHS.மொத்தம்_Min_Trailing_Zeros();
  unsigned TrailZeroRHS = RHS.மொத்தம்_Min_Trailing_Zeros();
  unsigned TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  unsigned SmallestOperand = std::min(TrailBitsKnownLHS - TrailZeroLHS,
                                      TrailBitsKnownRHS - TrailZeroRHS);
  unsigned ResultBitsKnown = std::min(SmallestOperand + TrailZ, BitWidth);

  // The lower ResultBitsKnown bits of this are known.
  APInt BottomKnown = LHS.One * RHS.One;

  KnownBits Res(BitWidth);
  Res.Zero.set_உயர்வு_Bits(LeadZ);
  Res.Zero |= (~BottomKnown).getLoBits(ResultBitsKnown);
  Res.One = BottomKnown.getLoBits(ResultBitsKnown);

  எனில் (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    unsigned TwoTZP1 = 2 * TrailZeroLHS + 1;
    எனில் (TwoTZP1 < BitWidth)
      Res.Zero.setBit(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    எனில் (TrailZeroLHS < BitWidth && LHS.One[TrailZeroLHS]) {
      unsigned TwoTZP2 = TwoTZP1 + 1;
      எனில் (TwoTZP2 < BitWidth)
        Res.Zero.setBit(TwoTZP2);
    }
  }

  திரும்பு Res;
}

KnownBits KnownBits::mulhs(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  ப்ரதிஜ்ஞா(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.sext(2 * BitWidth);
  KnownBits WideRHS = RHS.sext(2 * BitWidth);
  திரும்பு mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

KnownBits KnownBits::mulhu(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  ப்ரதிஜ்ஞா(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.zext(2 * BitWidth);
  KnownBits WideRHS = RHS.zext(2 * BitWidth);
  திரும்பு mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

நிலையான KnownBits div_Compute_கீழ்_Bit(KnownBits Known, மாறிலி KnownBits &LHS,
                                  மாறிலி KnownBits &RHS, தருக்க Exact) {

  எனில் (!Exact)
    திரும்பு Known;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  எனில் (LHS.One[0])
    Known.One.setBit(0);

  முழு MinTZ =
      (முழு)LHS.மொத்தம்_Min_Trailing_Zeros() - (முழு)RHS.மொத்தம்_Max_Trailing_Zeros();
  முழு MaxTZ =
      (முழு)LHS.மொத்தம்_Max_Trailing_Zeros() - (முழு)RHS.மொத்தம்_Min_Trailing_Zeros();
  எனில் (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    Known.Zero.set_கீழ்_Bits(MinTZ);
    எனில் (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      Known.One.setBit(MinTZ);
    }
  } இல்லையெனில் எனில் (MaxTZ < 0) {
    // Poison Result
    Known.setAllZero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  எனில் (Known.hasConflict())
    Known.setAllZero();

  திரும்பு Known;
}

KnownBits KnownBits::sdiv(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS,
                          தருக்க Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  எனில் (LHS.isNonNegative() && RHS.isNonNegative())
    திரும்பு udiv(LHS, RHS, Exact);

  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  எனில் (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    திரும்பு Known;
  }

  std::optional<APInt> Res;
  எனில் (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_மதிப்பு();
    APInt இலக்கம் = LHS.get_Signed_Min_மதிப்பு();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (இலக்கம்.is_Min_Signed_மதிப்பு() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_மதிப்பு(BitWidth)
              : இலக்கம்.sdiv(Denom);
  } இல்லையெனில் எனில் (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    எனில் (Exact || (-LHS.get_Signed_Max_மதிப்பு()).uge(RHS.get_Signed_Max_மதிப்பு())) {
      APInt Denom = RHS.get_Signed_Min_மதிப்பு();
      APInt இலக்கம் = LHS.get_Signed_Min_மதிப்பு();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      எனில் (Denom.isZero())
        Denom.setBit(RHS.மொத்தம்_Min_Trailing_Zeros());
      Res = இலக்கம்.sdiv(Denom);
    }
  } இல்லையெனில் எனில் (LHS.isStrictlyPositive() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    எனில் (Exact || LHS.get_Signed_Min_மதிப்பு().uge(-RHS.get_Signed_Min_மதிப்பு())) {
      APInt Denom = RHS.get_Signed_Max_மதிப்பு();
      APInt இலக்கம் = LHS.get_Signed_Max_மதிப்பு();
      Res = இலக்கம்.sdiv(Denom);
    }
  }

  எனில் (Res) {
    எனில் (Res->isNonNegative()) {
      unsigned LeadZ = Res->மொத்தம்_Leading_Zeros();
      Known.Zero.set_உயர்வு_Bits(LeadZ);
    } இல்லையெனில் {
      unsigned LeadO = Res->மொத்தம்_Leading_Ones();
      Known.One.set_உயர்வு_Bits(LeadO);
    }
  }

  Known = div_Compute_கீழ்_Bit(Known, LHS, RHS, Exact);
  திரும்பு Known;
}

KnownBits KnownBits::udiv(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS,
                          தருக்க Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  எனில் (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    திரும்பு Known;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_மதிப்பு();
  எனில் (MinDenom.isZero())
    MinDenom.setBit(RHS.மொத்தம்_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_மதிப்பு().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_மதிப்பு().udiv(RHS.get_Max_மதிப்பு());

  Known = KnownBits::makeConstant(MinRes);
  எனில் (std::optional<unsigned> DifferentBit =
          APIntOps::GetMostSignificantDifferentBit(MinRes, MaxRes)) {
    Known.Zero.clear_கீழ்_Bits(*DifferentBit + 1);
    Known.One.clear_கீழ்_Bits(*DifferentBit + 1);
  }

  Known = div_Compute_கீழ்_Bit(Known, LHS, RHS, Exact);

  திரும்பு Known;
}

KnownBits KnownBits::rem_Get_கீழ்_Bits(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  எனில் (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    unsigned RHSZeros = RHS.மொத்தம்_Min_Trailing_Zeros();
    APInt Mask = APInt::get_கீழ்_Bits_Set(BitWidth, RHSZeros);
    APInt OnesMask = LHS.One & Mask;
    APInt ZerosMask = LHS.Zero & Mask;
    திரும்பு KnownBits(ZerosMask, OnesMask);
  }
  திரும்பு KnownBits(BitWidth);
}

KnownBits KnownBits::urem(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  எனில் (LHS.get_Max_மதிப்பு().ult(RHS.get_Min_மதிப்பு()))
    திரும்பு LHS;

  KnownBits Known = rem_Get_கீழ்_Bits(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_மதிப்பு(), RHS.get_Max_மதிப்பு() - 1);
  Known.Zero.set_உயர்வு_Bits(MaxRes.மொத்தம்_Leading_Zeros());
  திரும்பு Known;
}

KnownBits KnownBits::srem(மாறிலி KnownBits &LHS, மாறிலி KnownBits &RHS) {
  KnownBits Known = rem_Get_கீழ்_Bits(LHS, RHS);
  எனில் (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt கீழ்_Bits = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    எனில் (LHS.isNonNegative() || கீழ்_Bits.isSubsetOf(LHS.Zero))
      Known.Zero |= ~கீழ்_Bits;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    எனில் (LHS.isNegative() && கீழ்_Bits.intersects(LHS.One))
      Known.One |= ~கீழ்_Bits;
    திரும்பு Known;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  எனில் (LHS.isNegative() && Known.isNonZero())
    Known.One.set_உயர்வு_Bits(
        std::max(LHS.மொத்தம்_Min_Leading_Ones(), RHS.மொத்தம்_Min_Sign_Bits()));
  இல்லையெனில் எனில் (LHS.isNonNegative())
    Known.Zero.set_உயர்வு_Bits(
        std::max(LHS.மொத்தம்_Min_Leading_Zeros(), RHS.மொத்தம்_Min_Sign_Bits()));
  திரும்பு Known;
}

KnownBits &KnownBits::operator&=(மாறிலி KnownBits &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  திரும்பு *இது;
}

KnownBits &KnownBits::operator|=(மாறிலி KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  திரும்பு *இது;
}

KnownBits &KnownBits::operator^=(மாறிலி KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  திரும்பு *இது;
}

KnownBits KnownBits::blsi() மாறிலி {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(Zero, APInt(BitWidth, 0));
  unsigned Max = மொத்தம்_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = மொத்தம்_Min_Trailing_Zeros();
  எனில் (Max == Min && Max < BitWidth)
    Known.One.setBit(Max);
  திரும்பு Known;
}

KnownBits KnownBits::blsmsk() மாறிலி {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(BitWidth);
  unsigned Max = மொத்தம்_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = மொத்தம்_Min_Trailing_Zeros();
  Known.One.set_கீழ்_Bits(std::min(Min + 1, BitWidth));
  திரும்பு Known;
}

வெறுமை KnownBits::print(raw_ostream &OS) மாறிலி {
  unsigned BitWidth = getBitWidth();
  க்கு (unsigned I = 0; I < BitWidth; ++I) {
    unsigned N = BitWidth - I - 1;
    எனில் (Zero[N] && One[N])
      OS << "!";
    இல்லையெனில் எனில் (Zero[N])
      OS << "0";
    இல்லையெனில் எனில் (One[N])
      OS << "1";
    இல்லையெனில்
      OS << "?";
  }
}

#எனில் !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD வெறுமை KnownBits::dump() மாறிலி {
  print(dbgs());
  dbgs() << "\n";
}
#endif

unsigned llvm::SignBitsOps::rot(unsigned SrcSignBits, unsigned BitWidth,
                                std::optional<APInt> RotAmt,
                                தருக்க Is_Rotate_வலது) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  எனில் (SrcSignBits == BitWidth)
    திரும்பு BitWidth;

  எனில் (!RotAmt)
    திரும்பு 1;

  unsigned Amt = RotAmt->urem(BitWidth);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  எனில் (Is_Rotate_வலது)
    Amt = (BitWidth - Amt) % BitWidth;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  எனில் (SrcSignBits > Amt + 1)
    திரும்பு SrcSignBits - Amt;
  திரும்பு 1;
}
