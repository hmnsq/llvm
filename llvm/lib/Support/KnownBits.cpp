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

#અન્તર્ભાવ "llvm/Support/KnownBits.h"
#અન્તર્ભાવ "llvm/ADT/Sequence.h"
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <cassert>

using નામાકાશ llvm;

KnownBits KnownBits::flipSignBit(અચલ KnownBits &Val) {
  unsigned SignBitPosition = Val.getBitWidth() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
  One.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
  પાછા KnownBits(Zero, One);
}

સ્થિર KnownBits computeForAddCarry(અચલ KnownBits &LHS, અચલ KnownBits &RHS,
                                    તાર્કિક CarryZero, તાર્કિક CarryOne) {

  APInt PossibleSumZero = LHS.get_Max_કિંમત() + RHS.get_Max_કિંમત() + !CarryZero;
  APInt PossibleSumOne = LHS.get_Min_કિંમત() + RHS.get_Min_કિંમત() + CarryOne;

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
  પાછા KnownOut;
}

KnownBits KnownBits::computeForAddCarry(
    અચલ KnownBits &LHS, અચલ KnownBits &RHS, અચલ KnownBits &Carry) {
  પ્રતિજ્ઞા(Carry.getBitWidth() == 1 && "Carry must be 1-bit");
  પાછા ::computeForAddCarry(
      LHS, RHS, Carry.Zero.get_Bool_કિંમત(), Carry.One.get_Bool_કિંમત());
}

KnownBits KnownBits::computeForAddSub(તાર્કિક Add, તાર્કિક NSW, તાર્કિક NUW,
                                      અચલ KnownBits &LHS,
                                      અચલ KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits KnownOut(BitWidth);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  જો (LHS.isUnknown() && RHS.isUnknown())
    પાછા KnownOut;

  જો (!LHS.isUnknown() && !RHS.isUnknown()) {
    જો (Add) {
      // Sum = LHS + RHS + 0
      KnownOut = ::computeForAddCarry(LHS, RHS, /*CarryZero=*/સાચું,
                                      /*CarryOne=*/ખોટું);
    } નહીંતર {
      // Sum = LHS + ~RHS + 1
      KnownBits NotRHS = RHS;
      std::swap(NotRHS.Zero, NotRHS.One);
      KnownOut = ::computeForAddCarry(LHS, NotRHS, /*CarryZero=*/ખોટું,
                                      /*CarryOne=*/સાચું);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  જો (NUW) {
    જો (Add) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_કિંમત().uadd_sat(RHS.get_Min_કિંમત());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      જો (NSW) {
        unsigned અંક_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        KnownOut.One.setBits(BitWidth - 1 - અંક_Bits_2, BitWidth - 1);
      }
      KnownOut.One.set_ઉપલું_Bits(MinVal.countl_one());
    } નહીંતર {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_કિંમત().usub_sat(RHS.get_Min_કિંમત());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      જો (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        unsigned અંક_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
        KnownOut.Zero.setBits(BitWidth - 1 - અંક_Bits_2, BitWidth - 1);
      }
      KnownOut.Zero.set_ઉપલું_Bits(MaxVal.countl_zero());
    }
  }

  જો (NSW) {
    APInt MinVal;
    APInt MaxVal;
    જો (Add) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_કિંમત().sadd_sat(RHS.get_Signed_Min_કિંમત());
      MaxVal = LHS.get_Signed_Max_કિંમત().sadd_sat(RHS.get_Signed_Max_કિંમત());
    } નહીંતર {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_કિંમત().ssub_sat(RHS.get_Signed_Max_કિંમત());
      MaxVal = LHS.get_Signed_Max_કિંમત().ssub_sat(RHS.get_Signed_Min_કિંમત());
    }
    જો (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      unsigned અંક_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
      KnownOut.One.setBits(BitWidth - 1 - અંક_Bits_2, BitWidth - 1);
      KnownOut.Zero.setSignBit();
    }
    જો (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      unsigned અંક_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
      KnownOut.Zero.setBits(BitWidth - 1 - અંક_Bits_2, BitWidth - 1);
      KnownOut.One.setSignBit();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  જો (KnownOut.hasConflict())
    KnownOut.setAllZero();
  પાછા KnownOut;
}

KnownBits KnownBits::computeForSubBorrow(અચલ KnownBits &LHS, KnownBits RHS,
                                         અચલ KnownBits &Borrow) {
  પ્રતિજ્ઞા(Borrow.getBitWidth() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::swap(RHS.Zero, RHS.One);
  પાછા ::computeForAddCarry(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_કિંમત(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_કિંમત());
}

KnownBits KnownBits::truncSSat(unsigned BitWidth) અચલ {
  unsigned આગત_Bits = getBitWidth();
  APInt MinInRange = APInt::get_Signed_Min_કિંમત(BitWidth).sext(આગત_Bits);
  APInt MaxInRange = APInt::get_Signed_Max_કિંમત(BitWidth).sext(આગત_Bits);
  APInt આગત_Min = get_Signed_Min_કિંમત();
  APInt આગત_Max = get_Signed_Max_કિંમત();
  KnownBits Known(BitWidth);

  // Case 1: All values fit - just truncate
  જો (આગત_Min.sge(MinInRange) && આગત_Max.sle(MaxInRange)) {
    Known = trunc(BitWidth);
  }
  // Case 2: All saturate to min
  નહીંતર જો (આગત_Max.slt(MinInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Min_કિંમત(BitWidth));
  }
  // Case 3: All saturate to max
  નહીંતર જો (આગત_Min.sgt(MaxInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Max_કિંમત(BitWidth));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  નહીંતર જો (આગત_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    Known.Zero = APInt(BitWidth, 0);
    Known.Zero.setBit(BitWidth - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    Known.One = One.trunc(BitWidth);
    Known.One.clearBit(BitWidth - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  નહીંતર જો (આગત_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    Known.Zero = Zero.trunc(BitWidth);
    Known.Zero.clearBit(BitWidth - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    Known.One = APInt(BitWidth, 0);
    Known.One.setBit(BitWidth - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  નહીંતર {
    // Output: min saturation, truncated, or max saturation
    APInt આગત_Zero_Lower = Zero.trunc(BitWidth);
    APInt આગત_One_Lower = One.trunc(BitWidth);
    APInt MinSat = APInt::get_Signed_Min_કિંમત(BitWidth);
    APInt MaxSat = APInt::get_Signed_Max_કિંમત(BitWidth);
    KnownBits MinSatKB = KnownBits::makeConstant(MinSat);
    KnownBits MaxSatKB = KnownBits::makeConstant(MaxSat);
    જો (આગત_Max.sle(MaxInRange)) {
      // Positive values fit, only negatives saturate to min
      Known.Zero = આગત_Zero_Lower & MinSatKB.Zero;
      Known.One = આગત_One_Lower & MinSatKB.One;
    } નહીંતર જો (આગત_Min.sge(MinInRange)) {
      // Negative values fit, only positives saturate to max
      Known.Zero = આગત_Zero_Lower & MaxSatKB.Zero;
      Known.One = આગત_One_Lower & MaxSatKB.One;
    } નહીંતર {
      // Both positive and negative values might saturate
      Known.Zero = આગત_Zero_Lower & MinSatKB.Zero & MaxSatKB.Zero;
      Known.One = આગત_One_Lower & MinSatKB.One & MaxSatKB.One;
    }
  }
  પાછા Known;
}

KnownBits KnownBits::truncSSatU(unsigned BitWidth) અચલ {
  unsigned આગત_Bits = getBitWidth();
  APInt MaxInRange = APInt::getAllOnes(BitWidth).zext(આગત_Bits);
  APInt આગત_Min = get_Signed_Min_કિંમત();
  APInt આગત_Max = get_Signed_Max_કિંમત();
  KnownBits Known(BitWidth);

  જો (isNegative()) {
    Known.setAllZero();
  } નહીંતર જો (આગત_Min.isNonNegative() && આગત_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } નહીંતર જો (આગત_Min.isNonNegative() && આગત_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } નહીંતર જો (આગત_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    Known.Zero = APInt(BitWidth, 0);
    // Common one bits: bits that are 1 in input stay 1
    Known.One = One.trunc(BitWidth);
  } નહીંતર {
    // Mixed: sign bit unknown
    જો (આગત_Max.ule(MaxInRange)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      Known.Zero = Zero.trunc(BitWidth);
      Known.One = APInt(BitWidth, 0);
    } નહીંતર {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      Known.Zero = APInt(BitWidth, 0);
      Known.One = APInt(BitWidth, 0);
    }
  }
  પાછા Known;
}

KnownBits KnownBits::truncUSat(unsigned BitWidth) અચલ {
  unsigned આગત_Bits = getBitWidth();
  APInt MaxInRange = APInt::get_નીચલું_Bits_Set(આગત_Bits, BitWidth);
  APInt આગત_Max = get_Max_કિંમત();
  APInt આગત_Min = get_Min_કિંમત();
  KnownBits Known(BitWidth);

  જો (આગત_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } નહીંતર જો (આગત_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } નહીંતર {
    Known.resetAll();
    Known.One = One.trunc(BitWidth);
  }
  પાછા Known;
}

KnownBits KnownBits::sextInReg(unsigned SrcBitWidth) અચલ {
  unsigned BitWidth = getBitWidth();
  પ્રતિજ્ઞા(0 < SrcBitWidth && SrcBitWidth <= BitWidth &&
         "Illegal sext-in-register");

  જો (SrcBitWidth == BitWidth)
    પાછા *આ;

  unsigned ExtBits = BitWidth - SrcBitWidth;
  KnownBits Result;
  Result.One = One << ExtBits;
  Result.Zero = Zero << ExtBits;
  Result.One.ashrInPlace(ExtBits);
  Result.Zero.ashrInPlace(ExtBits);
  પાછા Result;
}

KnownBits KnownBits::makeGE(અચલ APInt &Val) અચલ {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  unsigned N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_નીચલું_Bits(getBitWidth() - N);
  પાછા KnownBits(Zero, One | MaskedVal);
}

KnownBits KnownBits::umax(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  જો (LHS.get_Min_કિંમત().uge(RHS.get_Max_કિંમત()))
    પાછા LHS;
  જો (RHS.get_Min_કિંમત().uge(LHS.get_Max_કિંમત()))
    પાછા RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  KnownBits L = LHS.makeGE(RHS.get_Min_કિંમત());
  KnownBits R = RHS.makeGE(LHS.get_Min_કિંમત());
  પાછા L.intersectWith(R);
}

KnownBits KnownBits::umin(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](અચલ KnownBits &Val) { પાછા KnownBits(Val.One, Val.Zero); };
  પાછા Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::smax(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  પાછા flipSignBit(umax(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::smin(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](અચલ KnownBits &Val) {
    unsigned SignBitPosition = Val.getBitWidth() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
    One.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
    પાછા KnownBits(Zero, One);
  };
  પાછા Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::abdu(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  જો (LHS.get_Min_કિંમત().uge(RHS.get_Max_કિંમત()))
    પાછા computeForAddSub(/*Add=*/ખોટું, /*NSW=*/ખોટું, /*NUW=*/ખોટું, LHS,
                            RHS);
  જો (RHS.get_Min_કિંમત().uge(LHS.get_Max_કિંમત()))
    પાછા computeForAddSub(/*Add=*/ખોટું, /*NSW=*/ખોટું, /*NUW=*/ખોટું, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/ખોટું, /*NSW=*/ખોટું, /*NUW=*/સાચું, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/ખોટું, /*NSW=*/ખોટું, /*NUW=*/સાચું, RHS, LHS);
  પાછા Diff0.intersectWith(Diff1);
}

KnownBits KnownBits::abds(KnownBits LHS, KnownBits RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  જો (LHS.get_Signed_Min_કિંમત().sge(RHS.get_Signed_Max_કિંમત()))
    પાછા computeForAddSub(/*Add=*/ખોટું, /*NSW=*/ખોટું, /*NUW=*/ખોટું, LHS,
                            RHS);
  જો (RHS.get_Signed_Min_કિંમત().sge(LHS.get_Signed_Max_કિંમત()))
    પાછા computeForAddSub(/*Add=*/ખોટું, /*NSW=*/ખોટું, /*NUW=*/ખોટું, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  unsigned SignBitPosition = LHS.getBitWidth() - 1;
  માટે (auto Arg : {&LHS, &RHS}) {
    તાર્કિક Tmp = Arg->Zero[SignBitPosition];
    Arg->Zero.setBitVal(SignBitPosition, Arg->One[SignBitPosition]);
    Arg->One.setBitVal(SignBitPosition, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/ખોટું, /*NSW=*/ખોટું, /*NUW=*/સાચું, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/ખોટું, /*NSW=*/ખોટું, /*NUW=*/સાચું, RHS, LHS);
  પાછા Diff0.intersectWith(Diff1);
}

સ્થિર unsigned getMaxShiftAmount(અચલ APInt &Max_કિંમત, unsigned BitWidth) {
  જો (isPowerOf2_32(BitWidth))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    પાછા Max_કિંમત.extract_Bits_As_ZExt_કિંમત(
        std::min(Log2_32(BitWidth), Max_કિંમત.getBitWidth()), 0);
  // This is only an approximate upper bound.
  પાછા Max_કિંમત.get_Limited_કિંમત(BitWidth - 1);
}

KnownBits KnownBits::shl(અચલ KnownBits &LHS, અચલ KnownBits &RHS, તાર્કિક NUW,
                         તાર્કિક NSW, તાર્કિક ShAmtNonZero) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](અચલ KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known;
    તાર્કિક ShiftedOutZero, ShiftedOutOne;
    Known.Zero = LHS.Zero.ushl_ov(ShiftAmt, ShiftedOutZero);
    Known.Zero.set_નીચલું_Bits(ShiftAmt);
    Known.One = LHS.One.ushl_ov(ShiftAmt, ShiftedOutOne);

    // All cases returning poison have been handled by MaxShiftAmount already.
    જો (NSW) {
      જો (NUW && ShiftAmt != 0)
        // NUW means we can assume anything shifted out was a zero.
        ShiftedOutZero = સાચું;

      જો (ShiftedOutZero)
        Known.makeNonNegative();
      નહીંતર જો (ShiftedOutOne)
        Known.makeNegative();
    }
    પાછા Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_કિંમત().get_Limited_કિંમત(BitWidth);
  જો (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  જો (LHS.isUnknown()) {
    Known.Zero.set_નીચલું_Bits(MinShiftAmount);
    જો (NUW && NSW && MinShiftAmount != 0)
      Known.makeNonNegative();
    પાછા Known;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_કિંમત = RHS.get_Max_કિંમત();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_કિંમત, BitWidth);
  જો (NUW && NSW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.ગણતરી_Max_Leading_Zeros() - 1);
  જો (NUW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.ગણતરી_Max_Leading_Zeros());
  જો (NSW)
    MaxShiftAmount = std::min(
        MaxShiftAmount,
        std::max(LHS.ગણતરી_Max_Leading_Zeros(), LHS.ગણતરી_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  જો (MinShiftAmount == 0 && MaxShiftAmount == BitWidth - 1 &&
      isPowerOf2_32(BitWidth)) {
    Known.Zero.set_નીચલું_Bits(LHS.ગણતરી_Min_Trailing_Zeros());
    જો (LHS.isAllOnes())
      Known.One.setSignBit();
    જો (NSW) {
      જો (LHS.isNonNegative())
        Known.makeNonNegative();
      જો (LHS.isNegative())
        Known.makeNegative();
    }
    પાછા Known;
  }

  // Find the common bits from all possible shifts.
  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_કિંમત();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_કિંમત();
  Known.setAllConflict();
  માટે (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    જો ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      ચાલુ;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    જો (Known.isUnknown())
      તોડો;
  }

  // All shift amounts may result in poison.
  જો (Known.hasConflict())
    Known.setAllZero();
  પાછા Known;
}

KnownBits KnownBits::lshr(અચલ KnownBits &LHS, અચલ KnownBits &RHS,
                          તાર્કિક ShAmtNonZero, તાર્કિક Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](અચલ KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known >>= ShiftAmt;
    // High bits are known zero.
    Known.Zero.set_ઉપલું_Bits(ShiftAmt);
    પાછા Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_કિંમત().get_Limited_કિંમત(BitWidth);
  જો (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  જો (LHS.isUnknown()) {
    Known.Zero.set_ઉપલું_Bits(MinShiftAmount);
    પાછા Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_કિંમત = RHS.get_Max_કિંમત();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_કિંમત, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  જો (Exact) {
    unsigned FirstOne = LHS.ગણતરી_Max_Trailing_Zeros();
    જો (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      પાછા Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_કિંમત();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_કિંમત();
  Known.setAllConflict();
  માટે (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    જો ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      ચાલુ;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    જો (Known.isUnknown())
      તોડો;
  }

  // All shift amounts may result in poison.
  જો (Known.hasConflict())
    Known.setAllZero();
  પાછા Known;
}

KnownBits KnownBits::ashr(અચલ KnownBits &LHS, અચલ KnownBits &RHS,
                          તાર્કિક ShAmtNonZero, તાર્કિક Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](અચલ KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known.Zero.ashrInPlace(ShiftAmt);
    Known.One.ashrInPlace(ShiftAmt);
    પાછા Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_કિંમત().get_Limited_કિંમત(BitWidth);
  જો (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  જો (LHS.isUnknown()) {
    જો (MinShiftAmount == BitWidth) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      પાછા Known;
    }
    પાછા Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_કિંમત = RHS.get_Max_કિંમત();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_કિંમત, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  જો (Exact) {
    unsigned FirstOne = LHS.ગણતરી_Max_Trailing_Zeros();
    જો (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      પાછા Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_કિંમત();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_કિંમત();
  Known.setAllConflict();
  માટે (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
      ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    જો ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      ચાલુ;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    જો (Known.isUnknown())
      તોડો;
  }

  // All shift amounts may result in poison.
  જો (Known.hasConflict())
    Known.setAllZero();
  પાછા Known;
}

KnownBits KnownBits::fshl(અચલ KnownBits &LHS, અચલ KnownBits &RHS,
                          અચલ APInt &Amt) {
  પાછા KnownBits(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::fshr(અચલ KnownBits &LHS, અચલ KnownBits &RHS,
                          અચલ APInt &Amt) {
  પાછા KnownBits(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::clmul(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  KnownBits Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_કિંમત(), RHS.get_Min_કિંમત()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  auto ClMulOr = [](અચલ APInt &LHS, અચલ APInt &RHS) {
    unsigned BW = LHS.getBitWidth();
    પ્રતિજ્ઞા(BW == RHS.getBitWidth() && "Operand mismatch");
    APInt Result(BW, 0);
    માટે (unsigned I :
         seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
      જો (RHS[I])
        Result |= LHS << I;
    પાછા Result;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt Known = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= Known;
  Res.One &= Known;

  પાછા Res;
}

KnownBits KnownBits::pext(અચલ KnownBits &Val, અચલ KnownBits &Mask) {
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
  માટે (unsigned I = 0; I < BitWidth; ++I) {
    જો (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      જો (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clearBits(M0, M1 + 1);
      જો (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clearBits(M0, M1 + 1);
    }
    જો (Mask.One[I])
      ++M0, ++M1;
    નહીંતર જો (!Mask.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clearBits(M0, BitWidth);
  પાછા Res;
}

KnownBits KnownBits::pdep(અચલ KnownBits &Val, અચલ KnownBits &Mask) {
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
  માટે (unsigned I = 0; I < BitWidth; ++I) {
    જો (!Mask.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clearBit(I);
    જો (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::getBitsSet(BitWidth, M0, M1 + 1);
      જો (!Range.isSubsetOf(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clearBit(I);
      જો (!Range.isSubsetOf(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clearBit(I);
    }
    જો (Mask.One[I])
      ++M0, ++M1;
    નહીંતર જો (!Mask.Zero[I])
      ++M1;
  }
  પાછા Res;
}

std::optional<તાર્કિક> KnownBits::eq(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  જો (LHS.isConstant() && RHS.isConstant())
    પાછા LHS.getConstant() == RHS.getConstant();
  જો (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    પાછા ખોટું;
  પાછા std::nullopt;
}

std::optional<તાર્કિક> KnownBits::ne(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  જો (std::optional<તાર્કિક> KnownEQ = eq(LHS, RHS))
    પાછા !*KnownEQ;
  પાછા std::nullopt;
}

std::optional<તાર્કિક> KnownBits::ugt(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  જો (LHS.get_Max_કિંમત().ule(RHS.get_Min_કિંમત()))
    પાછા ખોટું;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  જો (LHS.get_Min_કિંમત().ugt(RHS.get_Max_કિંમત()))
    પાછા સાચું;
  પાછા std::nullopt;
}

std::optional<તાર્કિક> KnownBits::uge(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  જો (std::optional<તાર્કિક> IsUGT = ugt(RHS, LHS))
    પાછા !*IsUGT;
  પાછા std::nullopt;
}

std::optional<તાર્કિક> KnownBits::ult(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  પાછા ugt(RHS, LHS);
}

std::optional<તાર્કિક> KnownBits::ule(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  પાછા uge(RHS, LHS);
}

std::optional<તાર્કિક> KnownBits::sgt(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  જો (LHS.get_Signed_Max_કિંમત().sle(RHS.get_Signed_Min_કિંમત()))
    પાછા ખોટું;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  જો (LHS.get_Signed_Min_કિંમત().sgt(RHS.get_Signed_Max_કિંમત()))
    પાછા સાચું;
  પાછા std::nullopt;
}

std::optional<તાર્કિક> KnownBits::sge(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  જો (std::optional<તાર્કિક> KnownSGT = sgt(RHS, LHS))
    પાછા !*KnownSGT;
  પાછા std::nullopt;
}

std::optional<તાર્કિક> KnownBits::slt(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  પાછા sgt(RHS, LHS);
}

std::optional<તાર્કિક> KnownBits::sle(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  પાછા sge(RHS, LHS);
}

KnownBits KnownBits::abs(તાર્કિક IntMinIsPoison) અચલ {
  // If the source's MSB is zero then we know the rest of the bits already.
  જો (isNonNegative())
    પાછા *આ;

  // Absolute value preserves trailing zero count.
  KnownBits KnownAbs(getBitWidth());

  // If the input is negative, then abs(x) == -x.
  જો (isNegative()) {
    KnownBits Tmp = *આ;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    જો (IntMinIsPoison && (Zero.popcount() + 2) == getBitWidth())
      Tmp.One.setBit(ગણતરી_Min_Trailing_Zeros());

    KnownAbs = computeForAddSub(
        /*Add*/ ખોટું, IntMinIsPoison, /*NUW=*/ખોટું,
        KnownBits::makeConstant(APInt(getBitWidth(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    જો (IntMinIsPoison && Tmp.ગણતરી_Min_Population() == 1 &&
        Tmp.ગણતરી_Max_Population() != 1) {
      Tmp.One.clearSignBit();
      Tmp.Zero.setSignBit();
      KnownAbs.One.setBits(getBitWidth() - Tmp.ગણતરી_Min_Leading_Zeros(),
                           getBitWidth() - 1);
    }

  } નહીંતર {
    unsigned MaxTZ = ગણતરી_Max_Trailing_Zeros();
    unsigned MinTZ = ગણતરી_Min_Trailing_Zeros();

    KnownAbs.Zero.set_નીચલું_Bits(MinTZ);
    // If we know the lowest set 1, then preserve it.
    જો (MaxTZ == MinTZ && MaxTZ < getBitWidth())
      KnownAbs.One.setBit(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    જો (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_કિંમત())) {
      KnownAbs.One.clearSignBit();
      KnownAbs.Zero.setSignBit();
    }
  }

  પાછા KnownAbs;
}

KnownBits KnownBits::reduceAdd(unsigned અંક_Elts) અચલ {
  જો (અંક_Elts == 0)
    પાછા KnownBits(getBitWidth());

  unsigned BitWidth = getBitWidth();
  KnownBits Result(BitWidth);

  જો (isConstant())
    // If all elements are the same constant, we can simply compute it
    પાછા KnownBits::makeConstant(અંક_Elts * getConstant());

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
  unsigned LostBits = Log2_32_Ceil(અંક_Elts);

  જો (isNonNegative()) {
    unsigned LeadingZeros = ગણતરી_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > LostBits ? LeadingZeros - LostBits : 0;
    Result.Zero.set_ઉપલું_Bits(LeadingZeros);
  } નહીંતર જો (isNegative()) {
    unsigned LeadingOnes = ગણતરી_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > LostBits ? LeadingOnes - LostBits : 0;
    Result.One.set_ઉપલું_Bits(LeadingOnes);
  }

  પાછા Result;
}

સ્થિર KnownBits computeForSatAddSub(તાર્કિક Add, તાર્કિક Signed,
                                     અચલ KnownBits &LHS,
                                     અચલ KnownBits &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  unsigned BitWidth = LHS.getBitWidth();

  std::optional<તાર્કિક> Overflow;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  તાર્કિક MayNegClamp = સાચું;
  તાર્કિક MayPosClamp = સાચું;
  જો (Signed) {
    // Easy cases we can rule out any overflow.
    જો (Add && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      Overflow = ખોટું;
    નહીંતર જો (!Add && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      Overflow = ખોટું;
    નહીંતર {
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
          KnownBits::computeForAddSub(Add, /*NSW=*/ખોટું,
                                      /*NUW=*/ખોટું, UnsignedLHS, UnsignedRHS);
      જો (Add) {
        જો (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = ખોટું;
          // Pos + Pos will overflow with extra signbit.
          જો (LHS.isNonNegative() && RHS.isNonNegative())
            Overflow = સાચું;
        } નહીંતર જો (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = ખોટું;
          // Neg + Neg will overflow without extra signbit.
          જો (LHS.isNegative() && RHS.isNegative())
            Overflow = સાચું;
        }
        // We will never clamp to the opposite sign of N-bit result.
        જો (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = ખોટું;
        જો (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = ખોટું;
      } નહીંતર {
        જો (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = ખોટું;
          // Neg - Pos will overflow with extra signbit.
          જો (LHS.isNegative() && RHS.isNonNegative())
            Overflow = સાચું;
        } નહીંતર જો (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = ખોટું;
          // Pos - Neg will overflow without extra signbit.
          જો (LHS.isNonNegative() && RHS.isNegative())
            Overflow = સાચું;
        }
        // We will never clamp to the opposite sign of N-bit result.
        જો (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = ખોટું;
        જો (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = ખોટું;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    જો (!MayNegClamp && !MayPosClamp)
      Overflow = ખોટું;
  } નહીંતર જો (Add) {
    // uadd.sat
    તાર્કિક Of;
    (શૂન્ય)LHS.get_Max_કિંમત().uadd_ov(RHS.get_Max_કિંમત(), Of);
    જો (!Of) {
      Overflow = ખોટું;
    } નહીંતર {
      (શૂન્ય)LHS.get_Min_કિંમત().uadd_ov(RHS.get_Min_કિંમત(), Of);
      જો (Of)
        Overflow = સાચું;
    }
  } નહીંતર {
    // usub.sat
    તાર્કિક Of;
    (શૂન્ય)LHS.get_Min_કિંમત().usub_ov(RHS.get_Max_કિંમત(), Of);
    જો (!Of) {
      Overflow = ખોટું;
    } નહીંતર {
      (શૂન્ય)LHS.get_Max_કિંમત().usub_ov(RHS.get_Min_કિંમત(), Of);
      જો (Of)
        Overflow = સાચું;
    }
  }

  KnownBits Res = KnownBits::computeForAddSub(Add, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  જો (Overflow) {
    // We know whether or not we overflowed.
    જો (!(*Overflow)) {
      // No overflow.
      પાછા Res;
    }

    // We overflowed
    APInt C;
    જો (Signed) {
      // sadd.sat / ssub.sat
      પ્રતિજ્ઞા(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_કિંમત(BitWidth)
                           : APInt::get_Signed_Max_કિંમત(BitWidth);
    } નહીંતર જો (Add) {
      // uadd.sat
      C = APInt::get_Max_કિંમત(BitWidth);
    } નહીંતર {
      // uadd.sat
      C = APInt::get_Min_કિંમત(BitWidth);
    }

    Res.One = C;
    Res.Zero = ~C;
    પાછા Res;
  }

  // We don't know if we overflowed.
  જો (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    જો (MayPosClamp)
      Res.Zero.clear_નીચલું_Bits(BitWidth - 1);
    જો (MayNegClamp)
      Res.One.clear_નીચલું_Bits(BitWidth - 1);
  } નહીંતર જો (Add) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clearAllBits();
  } નહીંતર {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clearAllBits();
  }

  પાછા Res;
}

KnownBits KnownBits::sadd_sat(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  પાછા computeForSatAddSub(/*Add*/ સાચું, /*Signed*/ સાચું, LHS, RHS);
}
KnownBits KnownBits::ssub_sat(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  પાછા computeForSatAddSub(/*Add*/ ખોટું, /*Signed*/ સાચું, LHS, RHS);
}
KnownBits KnownBits::uadd_sat(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  પાછા computeForSatAddSub(/*Add*/ સાચું, /*Signed*/ ખોટું, LHS, RHS);
}
KnownBits KnownBits::usub_sat(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  પાછા computeForSatAddSub(/*Add*/ ખોટું, /*Signed*/ ખોટું, LHS, RHS);
}

સ્થિર KnownBits avgComputeU(KnownBits LHS, KnownBits RHS, તાર્કિક IsCeil) {
  unsigned BitWidth = LHS.getBitWidth();
  LHS = LHS.zext(BitWidth + 1);
  RHS = RHS.zext(BitWidth + 1);
  LHS =
      computeForAddCarry(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.extractBits(BitWidth, 1);
  પાછા LHS;
}

KnownBits KnownBits::avgFloorS(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  પાછા flipSignBit(avgFloorU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgFloorU(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  પાછા avgComputeU(LHS, RHS, /*IsCeil=*/ખોટું);
}

KnownBits KnownBits::avgCeilS(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  પાછા flipSignBit(avgCeilU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgCeilU(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  પાછા avgComputeU(LHS, RHS, /*IsCeil=*/સાચું);
}

KnownBits KnownBits::mul(અચલ KnownBits &LHS, અચલ KnownBits &RHS,
                         તાર્કિક NoUndefSelfMultiply) {
  unsigned BitWidth = LHS.getBitWidth();
  પ્રતિજ્ઞા(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  પ્રતિજ્ઞા((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.get_Max_કિંમત();
  APInt UMaxRHS = RHS.get_Max_કિંમત();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  તાર્કિક HasOverflow;
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
  unsigned TrailZeroLHS = LHS.ગણતરી_Min_Trailing_Zeros();
  unsigned TrailZeroRHS = RHS.ગણતરી_Min_Trailing_Zeros();
  unsigned TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  unsigned SmallestOperand = std::min(TrailBitsKnownLHS - TrailZeroLHS,
                                      TrailBitsKnownRHS - TrailZeroRHS);
  unsigned ResultBitsKnown = std::min(SmallestOperand + TrailZ, BitWidth);

  // The lower ResultBitsKnown bits of this are known.
  APInt BottomKnown = LHS.One * RHS.One;

  KnownBits Res(BitWidth);
  Res.Zero.set_ઉપલું_Bits(LeadZ);
  Res.Zero |= (~BottomKnown).getLoBits(ResultBitsKnown);
  Res.One = BottomKnown.getLoBits(ResultBitsKnown);

  જો (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    unsigned TwoTZP1 = 2 * TrailZeroLHS + 1;
    જો (TwoTZP1 < BitWidth)
      Res.Zero.setBit(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    જો (TrailZeroLHS < BitWidth && LHS.One[TrailZeroLHS]) {
      unsigned TwoTZP2 = TwoTZP1 + 1;
      જો (TwoTZP2 < BitWidth)
        Res.Zero.setBit(TwoTZP2);
    }
  }

  પાછા Res;
}

KnownBits KnownBits::mulhs(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  પ્રતિજ્ઞા(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.sext(2 * BitWidth);
  KnownBits WideRHS = RHS.sext(2 * BitWidth);
  પાછા mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

KnownBits KnownBits::mulhu(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  પ્રતિજ્ઞા(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.zext(2 * BitWidth);
  KnownBits WideRHS = RHS.zext(2 * BitWidth);
  પાછા mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

સ્થિર KnownBits div_Compute_નીચલું_Bit(KnownBits Known, અચલ KnownBits &LHS,
                                  અચલ KnownBits &RHS, તાર્કિક Exact) {

  જો (!Exact)
    પાછા Known;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  જો (LHS.One[0])
    Known.One.setBit(0);

  પૂર્ણાંક MinTZ =
      (પૂર્ણાંક)LHS.ગણતરી_Min_Trailing_Zeros() - (પૂર્ણાંક)RHS.ગણતરી_Max_Trailing_Zeros();
  પૂર્ણાંક MaxTZ =
      (પૂર્ણાંક)LHS.ગણતરી_Max_Trailing_Zeros() - (પૂર્ણાંક)RHS.ગણતરી_Min_Trailing_Zeros();
  જો (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    Known.Zero.set_નીચલું_Bits(MinTZ);
    જો (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      Known.One.setBit(MinTZ);
    }
  } નહીંતર જો (MaxTZ < 0) {
    // Poison Result
    Known.setAllZero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  જો (Known.hasConflict())
    Known.setAllZero();

  પાછા Known;
}

KnownBits KnownBits::sdiv(અચલ KnownBits &LHS, અચલ KnownBits &RHS,
                          તાર્કિક Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  જો (LHS.isNonNegative() && RHS.isNonNegative())
    પાછા udiv(LHS, RHS, Exact);

  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  જો (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    પાછા Known;
  }

  std::optional<APInt> Res;
  જો (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_કિંમત();
    APInt અંક = LHS.get_Signed_Min_કિંમત();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (અંક.is_Min_Signed_કિંમત() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_કિંમત(BitWidth)
              : અંક.sdiv(Denom);
  } નહીંતર જો (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    જો (Exact || (-LHS.get_Signed_Max_કિંમત()).uge(RHS.get_Signed_Max_કિંમત())) {
      APInt Denom = RHS.get_Signed_Min_કિંમત();
      APInt અંક = LHS.get_Signed_Min_કિંમત();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      જો (Denom.isZero())
        Denom.setBit(RHS.ગણતરી_Min_Trailing_Zeros());
      Res = અંક.sdiv(Denom);
    }
  } નહીંતર જો (LHS.isStrictlyPositive() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    જો (Exact || LHS.get_Signed_Min_કિંમત().uge(-RHS.get_Signed_Min_કિંમત())) {
      APInt Denom = RHS.get_Signed_Max_કિંમત();
      APInt અંક = LHS.get_Signed_Max_કિંમત();
      Res = અંક.sdiv(Denom);
    }
  }

  જો (Res) {
    જો (Res->isNonNegative()) {
      unsigned LeadZ = Res->ગણતરી_Leading_Zeros();
      Known.Zero.set_ઉપલું_Bits(LeadZ);
    } નહીંતર {
      unsigned LeadO = Res->ગણતરી_Leading_Ones();
      Known.One.set_ઉપલું_Bits(LeadO);
    }
  }

  Known = div_Compute_નીચલું_Bit(Known, LHS, RHS, Exact);
  પાછા Known;
}

KnownBits KnownBits::udiv(અચલ KnownBits &LHS, અચલ KnownBits &RHS,
                          તાર્કિક Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  જો (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    પાછા Known;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_કિંમત();
  જો (MinDenom.isZero())
    MinDenom.setBit(RHS.ગણતરી_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_કિંમત().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_કિંમત().udiv(RHS.get_Max_કિંમત());

  Known = KnownBits::makeConstant(MinRes);
  જો (std::optional<unsigned> DifferentBit =
          APIntOps::GetMostSignificantDifferentBit(MinRes, MaxRes)) {
    Known.Zero.clear_નીચલું_Bits(*DifferentBit + 1);
    Known.One.clear_નીચલું_Bits(*DifferentBit + 1);
  }

  Known = div_Compute_નીચલું_Bit(Known, LHS, RHS, Exact);

  પાછા Known;
}

KnownBits KnownBits::rem_Get_નીચલું_Bits(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  જો (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    unsigned RHSZeros = RHS.ગણતરી_Min_Trailing_Zeros();
    APInt Mask = APInt::get_નીચલું_Bits_Set(BitWidth, RHSZeros);
    APInt OnesMask = LHS.One & Mask;
    APInt ZerosMask = LHS.Zero & Mask;
    પાછા KnownBits(ZerosMask, OnesMask);
  }
  પાછા KnownBits(BitWidth);
}

KnownBits KnownBits::urem(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  જો (LHS.get_Max_કિંમત().ult(RHS.get_Min_કિંમત()))
    પાછા LHS;

  KnownBits Known = rem_Get_નીચલું_Bits(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_કિંમત(), RHS.get_Max_કિંમત() - 1);
  Known.Zero.set_ઉપલું_Bits(MaxRes.ગણતરી_Leading_Zeros());
  પાછા Known;
}

KnownBits KnownBits::srem(અચલ KnownBits &LHS, અચલ KnownBits &RHS) {
  KnownBits Known = rem_Get_નીચલું_Bits(LHS, RHS);
  જો (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt નીચલું_Bits = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    જો (LHS.isNonNegative() || નીચલું_Bits.isSubsetOf(LHS.Zero))
      Known.Zero |= ~નીચલું_Bits;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    જો (LHS.isNegative() && નીચલું_Bits.intersects(LHS.One))
      Known.One |= ~નીચલું_Bits;
    પાછા Known;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  જો (LHS.isNegative() && Known.isNonZero())
    Known.One.set_ઉપલું_Bits(
        std::max(LHS.ગણતરી_Min_Leading_Ones(), RHS.ગણતરી_Min_Sign_Bits()));
  નહીંતર જો (LHS.isNonNegative())
    Known.Zero.set_ઉપલું_Bits(
        std::max(LHS.ગણતરી_Min_Leading_Zeros(), RHS.ગણતરી_Min_Sign_Bits()));
  પાછા Known;
}

KnownBits &KnownBits::operator&=(અચલ KnownBits &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  પાછા *આ;
}

KnownBits &KnownBits::operator|=(અચલ KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  પાછા *આ;
}

KnownBits &KnownBits::operator^=(અચલ KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  પાછા *આ;
}

KnownBits KnownBits::blsi() અચલ {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(Zero, APInt(BitWidth, 0));
  unsigned Max = ગણતરી_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = ગણતરી_Min_Trailing_Zeros();
  જો (Max == Min && Max < BitWidth)
    Known.One.setBit(Max);
  પાછા Known;
}

KnownBits KnownBits::blsmsk() અચલ {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(BitWidth);
  unsigned Max = ગણતરી_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = ગણતરી_Min_Trailing_Zeros();
  Known.One.set_નીચલું_Bits(std::min(Min + 1, BitWidth));
  પાછા Known;
}

શૂન્ય KnownBits::print(raw_ostream &OS) અચલ {
  unsigned BitWidth = getBitWidth();
  માટે (unsigned I = 0; I < BitWidth; ++I) {
    unsigned N = BitWidth - I - 1;
    જો (Zero[N] && One[N])
      OS << "!";
    નહીંતર જો (Zero[N])
      OS << "0";
    નહીંતર જો (One[N])
      OS << "1";
    નહીંતર
      OS << "?";
  }
}

#જો !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD શૂન્ય KnownBits::dump() અચલ {
  print(dbgs());
  dbgs() << "\n";
}
#endif

unsigned llvm::SignBitsOps::rot(unsigned SrcSignBits, unsigned BitWidth,
                                std::optional<APInt> RotAmt,
                                તાર્કિક Is_Rotate_જમણું) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  જો (SrcSignBits == BitWidth)
    પાછા BitWidth;

  જો (!RotAmt)
    પાછા 1;

  unsigned Amt = RotAmt->urem(BitWidth);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  જો (Is_Rotate_જમણું)
    Amt = (BitWidth - Amt) % BitWidth;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  જો (SrcSignBits > Amt + 1)
    પાછા SrcSignBits - Amt;
  પાછા 1;
}
