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

#അന്തര്ഭാവ "llvm/Support/KnownBits.h"
#അന്തര്ഭാവ "llvm/ADT/Sequence.h"
#അന്തര്ഭാവ "llvm/Support/Debug.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <cassert>

using നാമാകാശ llvm;

KnownBits KnownBits::flipSignBit(സ്ഥിരം KnownBits &Val) {
  unsigned SignBitPosition = Val.getBitWidth() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
  One.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
  തിരികെ KnownBits(Zero, One);
}

സ്ഥിതം KnownBits computeForAddCarry(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS,
                                    തർക്കം CarryZero, തർക്കം CarryOne) {

  APInt PossibleSumZero = LHS.get_Max_മൂല്യം() + RHS.get_Max_മൂല്യം() + !CarryZero;
  APInt PossibleSumOne = LHS.get_Min_മൂല്യം() + RHS.get_Min_മൂല്യം() + CarryOne;

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
  തിരികെ KnownOut;
}

KnownBits KnownBits::computeForAddCarry(
    സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS, സ്ഥിരം KnownBits &Carry) {
  പ്രതിജ്ഞാ(Carry.getBitWidth() == 1 && "Carry must be 1-bit");
  തിരികെ ::computeForAddCarry(
      LHS, RHS, Carry.Zero.get_Bool_മൂല്യം(), Carry.One.get_Bool_മൂല്യം());
}

KnownBits KnownBits::computeForAddSub(തർക്കം Add, തർക്കം NSW, തർക്കം NUW,
                                      സ്ഥിരം KnownBits &LHS,
                                      സ്ഥിരം KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits KnownOut(BitWidth);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  എങ്കിൽ (LHS.isUnknown() && RHS.isUnknown())
    തിരികെ KnownOut;

  എങ്കിൽ (!LHS.isUnknown() && !RHS.isUnknown()) {
    എങ്കിൽ (Add) {
      // Sum = LHS + RHS + 0
      KnownOut = ::computeForAddCarry(LHS, RHS, /*CarryZero=*/സത്യം,
                                      /*CarryOne=*/അസത്യം);
    } അല്ലെങ്കിൽ {
      // Sum = LHS + ~RHS + 1
      KnownBits NotRHS = RHS;
      std::swap(NotRHS.Zero, NotRHS.One);
      KnownOut = ::computeForAddCarry(LHS, NotRHS, /*CarryZero=*/അസത്യം,
                                      /*CarryOne=*/സത്യം);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  എങ്കിൽ (NUW) {
    എങ്കിൽ (Add) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_മൂല്യം().uadd_sat(RHS.get_Min_മൂല്യം());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      എങ്കിൽ (NSW) {
        unsigned അക്കം_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        KnownOut.One.setBits(BitWidth - 1 - അക്കം_Bits_2, BitWidth - 1);
      }
      KnownOut.One.set_മുകളിലെ_Bits(MinVal.countl_one());
    } അല്ലെങ്കിൽ {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_മൂല്യം().usub_sat(RHS.get_Min_മൂല്യം());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      എങ്കിൽ (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        unsigned അക്കം_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
        KnownOut.Zero.setBits(BitWidth - 1 - അക്കം_Bits_2, BitWidth - 1);
      }
      KnownOut.Zero.set_മുകളിലെ_Bits(MaxVal.countl_zero());
    }
  }

  എങ്കിൽ (NSW) {
    APInt MinVal;
    APInt MaxVal;
    എങ്കിൽ (Add) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_മൂല്യം().sadd_sat(RHS.get_Signed_Min_മൂല്യം());
      MaxVal = LHS.get_Signed_Max_മൂല്യം().sadd_sat(RHS.get_Signed_Max_മൂല്യം());
    } അല്ലെങ്കിൽ {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_മൂല്യം().ssub_sat(RHS.get_Signed_Max_മൂല്യം());
      MaxVal = LHS.get_Signed_Max_മൂല്യം().ssub_sat(RHS.get_Signed_Min_മൂല്യം());
    }
    എങ്കിൽ (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      unsigned അക്കം_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
      KnownOut.One.setBits(BitWidth - 1 - അക്കം_Bits_2, BitWidth - 1);
      KnownOut.Zero.setSignBit();
    }
    എങ്കിൽ (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      unsigned അക്കം_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
      KnownOut.Zero.setBits(BitWidth - 1 - അക്കം_Bits_2, BitWidth - 1);
      KnownOut.One.setSignBit();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  എങ്കിൽ (KnownOut.hasConflict())
    KnownOut.setAllZero();
  തിരികെ KnownOut;
}

KnownBits KnownBits::computeForSubBorrow(സ്ഥിരം KnownBits &LHS, KnownBits RHS,
                                         സ്ഥിരം KnownBits &Borrow) {
  പ്രതിജ്ഞാ(Borrow.getBitWidth() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::swap(RHS.Zero, RHS.One);
  തിരികെ ::computeForAddCarry(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_മൂല്യം(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_മൂല്യം());
}

KnownBits KnownBits::truncSSat(unsigned BitWidth) സ്ഥിരം {
  unsigned നിവേശം_Bits = getBitWidth();
  APInt MinInRange = APInt::get_Signed_Min_മൂല്യം(BitWidth).sext(നിവേശം_Bits);
  APInt MaxInRange = APInt::get_Signed_Max_മൂല്യം(BitWidth).sext(നിവേശം_Bits);
  APInt നിവേശം_Min = get_Signed_Min_മൂല്യം();
  APInt നിവേശം_Max = get_Signed_Max_മൂല്യം();
  KnownBits Known(BitWidth);

  // Case 1: All values fit - just truncate
  എങ്കിൽ (നിവേശം_Min.sge(MinInRange) && നിവേശം_Max.sle(MaxInRange)) {
    Known = trunc(BitWidth);
  }
  // Case 2: All saturate to min
  അല്ലെങ്കിൽ എങ്കിൽ (നിവേശം_Max.slt(MinInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Min_മൂല്യം(BitWidth));
  }
  // Case 3: All saturate to max
  അല്ലെങ്കിൽ എങ്കിൽ (നിവേശം_Min.sgt(MaxInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Max_മൂല്യം(BitWidth));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  അല്ലെങ്കിൽ എങ്കിൽ (നിവേശം_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    Known.Zero = APInt(BitWidth, 0);
    Known.Zero.setBit(BitWidth - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    Known.One = One.trunc(BitWidth);
    Known.One.clearBit(BitWidth - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  അല്ലെങ്കിൽ എങ്കിൽ (നിവേശം_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    Known.Zero = Zero.trunc(BitWidth);
    Known.Zero.clearBit(BitWidth - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    Known.One = APInt(BitWidth, 0);
    Known.One.setBit(BitWidth - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  അല്ലെങ്കിൽ {
    // Output: min saturation, truncated, or max saturation
    APInt നിവേശം_Zero_Lower = Zero.trunc(BitWidth);
    APInt നിവേശം_One_Lower = One.trunc(BitWidth);
    APInt MinSat = APInt::get_Signed_Min_മൂല്യം(BitWidth);
    APInt MaxSat = APInt::get_Signed_Max_മൂല്യം(BitWidth);
    KnownBits MinSatKB = KnownBits::makeConstant(MinSat);
    KnownBits MaxSatKB = KnownBits::makeConstant(MaxSat);
    എങ്കിൽ (നിവേശം_Max.sle(MaxInRange)) {
      // Positive values fit, only negatives saturate to min
      Known.Zero = നിവേശം_Zero_Lower & MinSatKB.Zero;
      Known.One = നിവേശം_One_Lower & MinSatKB.One;
    } അല്ലെങ്കിൽ എങ്കിൽ (നിവേശം_Min.sge(MinInRange)) {
      // Negative values fit, only positives saturate to max
      Known.Zero = നിവേശം_Zero_Lower & MaxSatKB.Zero;
      Known.One = നിവേശം_One_Lower & MaxSatKB.One;
    } അല്ലെങ്കിൽ {
      // Both positive and negative values might saturate
      Known.Zero = നിവേശം_Zero_Lower & MinSatKB.Zero & MaxSatKB.Zero;
      Known.One = നിവേശം_One_Lower & MinSatKB.One & MaxSatKB.One;
    }
  }
  തിരികെ Known;
}

KnownBits KnownBits::truncSSatU(unsigned BitWidth) സ്ഥിരം {
  unsigned നിവേശം_Bits = getBitWidth();
  APInt MaxInRange = APInt::getAllOnes(BitWidth).zext(നിവേശം_Bits);
  APInt നിവേശം_Min = get_Signed_Min_മൂല്യം();
  APInt നിവേശം_Max = get_Signed_Max_മൂല്യം();
  KnownBits Known(BitWidth);

  എങ്കിൽ (isNegative()) {
    Known.setAllZero();
  } അല്ലെങ്കിൽ എങ്കിൽ (നിവേശം_Min.isNonNegative() && നിവേശം_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } അല്ലെങ്കിൽ എങ്കിൽ (നിവേശം_Min.isNonNegative() && നിവേശം_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } അല്ലെങ്കിൽ എങ്കിൽ (നിവേശം_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    Known.Zero = APInt(BitWidth, 0);
    // Common one bits: bits that are 1 in input stay 1
    Known.One = One.trunc(BitWidth);
  } അല്ലെങ്കിൽ {
    // Mixed: sign bit unknown
    എങ്കിൽ (നിവേശം_Max.ule(MaxInRange)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      Known.Zero = Zero.trunc(BitWidth);
      Known.One = APInt(BitWidth, 0);
    } അല്ലെങ്കിൽ {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      Known.Zero = APInt(BitWidth, 0);
      Known.One = APInt(BitWidth, 0);
    }
  }
  തിരികെ Known;
}

KnownBits KnownBits::truncUSat(unsigned BitWidth) സ്ഥിരം {
  unsigned നിവേശം_Bits = getBitWidth();
  APInt MaxInRange = APInt::get_താഴെയുള്ള_Bits_Set(നിവേശം_Bits, BitWidth);
  APInt നിവേശം_Max = get_Max_മൂല്യം();
  APInt നിവേശം_Min = get_Min_മൂല്യം();
  KnownBits Known(BitWidth);

  എങ്കിൽ (നിവേശം_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } അല്ലെങ്കിൽ എങ്കിൽ (നിവേശം_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } അല്ലെങ്കിൽ {
    Known.resetAll();
    Known.One = One.trunc(BitWidth);
  }
  തിരികെ Known;
}

KnownBits KnownBits::sextInReg(unsigned SrcBitWidth) സ്ഥിരം {
  unsigned BitWidth = getBitWidth();
  പ്രതിജ്ഞാ(0 < SrcBitWidth && SrcBitWidth <= BitWidth &&
         "Illegal sext-in-register");

  എങ്കിൽ (SrcBitWidth == BitWidth)
    തിരികെ *ഇത്;

  unsigned ExtBits = BitWidth - SrcBitWidth;
  KnownBits Result;
  Result.One = One << ExtBits;
  Result.Zero = Zero << ExtBits;
  Result.One.ashrInPlace(ExtBits);
  Result.Zero.ashrInPlace(ExtBits);
  തിരികെ Result;
}

KnownBits KnownBits::makeGE(സ്ഥിരം APInt &Val) സ്ഥിരം {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  unsigned N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_താഴെയുള്ള_Bits(getBitWidth() - N);
  തിരികെ KnownBits(Zero, One | MaskedVal);
}

KnownBits KnownBits::umax(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  എങ്കിൽ (LHS.get_Min_മൂല്യം().uge(RHS.get_Max_മൂല്യം()))
    തിരികെ LHS;
  എങ്കിൽ (RHS.get_Min_മൂല്യം().uge(LHS.get_Max_മൂല്യം()))
    തിരികെ RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  KnownBits L = LHS.makeGE(RHS.get_Min_മൂല്യം());
  KnownBits R = RHS.makeGE(LHS.get_Min_മൂല്യം());
  തിരികെ L.intersectWith(R);
}

KnownBits KnownBits::umin(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](സ്ഥിരം KnownBits &Val) { തിരികെ KnownBits(Val.One, Val.Zero); };
  തിരികെ Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::smax(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  തിരികെ flipSignBit(umax(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::smin(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](സ്ഥിരം KnownBits &Val) {
    unsigned SignBitPosition = Val.getBitWidth() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
    One.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
    തിരികെ KnownBits(Zero, One);
  };
  തിരികെ Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::abdu(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  എങ്കിൽ (LHS.get_Min_മൂല്യം().uge(RHS.get_Max_മൂല്യം()))
    തിരികെ computeForAddSub(/*Add=*/അസത്യം, /*NSW=*/അസത്യം, /*NUW=*/അസത്യം, LHS,
                            RHS);
  എങ്കിൽ (RHS.get_Min_മൂല്യം().uge(LHS.get_Max_മൂല്യം()))
    തിരികെ computeForAddSub(/*Add=*/അസത്യം, /*NSW=*/അസത്യം, /*NUW=*/അസത്യം, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/അസത്യം, /*NSW=*/അസത്യം, /*NUW=*/സത്യം, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/അസത്യം, /*NSW=*/അസത്യം, /*NUW=*/സത്യം, RHS, LHS);
  തിരികെ Diff0.intersectWith(Diff1);
}

KnownBits KnownBits::abds(KnownBits LHS, KnownBits RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  എങ്കിൽ (LHS.get_Signed_Min_മൂല്യം().sge(RHS.get_Signed_Max_മൂല്യം()))
    തിരികെ computeForAddSub(/*Add=*/അസത്യം, /*NSW=*/അസത്യം, /*NUW=*/അസത്യം, LHS,
                            RHS);
  എങ്കിൽ (RHS.get_Signed_Min_മൂല്യം().sge(LHS.get_Signed_Max_മൂല്യം()))
    തിരികെ computeForAddSub(/*Add=*/അസത്യം, /*NSW=*/അസത്യം, /*NUW=*/അസത്യം, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  unsigned SignBitPosition = LHS.getBitWidth() - 1;
  വേണ്ടി (auto Arg : {&LHS, &RHS}) {
    തർക്കം Tmp = Arg->Zero[SignBitPosition];
    Arg->Zero.setBitVal(SignBitPosition, Arg->One[SignBitPosition]);
    Arg->One.setBitVal(SignBitPosition, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/അസത്യം, /*NSW=*/അസത്യം, /*NUW=*/സത്യം, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/അസത്യം, /*NSW=*/അസത്യം, /*NUW=*/സത്യം, RHS, LHS);
  തിരികെ Diff0.intersectWith(Diff1);
}

സ്ഥിതം unsigned getMaxShiftAmount(സ്ഥിരം APInt &Max_മൂല്യം, unsigned BitWidth) {
  എങ്കിൽ (isPowerOf2_32(BitWidth))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    തിരികെ Max_മൂല്യം.extract_Bits_As_ZExt_മൂല്യം(
        std::min(Log2_32(BitWidth), Max_മൂല്യം.getBitWidth()), 0);
  // This is only an approximate upper bound.
  തിരികെ Max_മൂല്യം.get_Limited_മൂല്യം(BitWidth - 1);
}

KnownBits KnownBits::shl(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS, തർക്കം NUW,
                         തർക്കം NSW, തർക്കം ShAmtNonZero) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](സ്ഥിരം KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known;
    തർക്കം ShiftedOutZero, ShiftedOutOne;
    Known.Zero = LHS.Zero.ushl_ov(ShiftAmt, ShiftedOutZero);
    Known.Zero.set_താഴെയുള്ള_Bits(ShiftAmt);
    Known.One = LHS.One.ushl_ov(ShiftAmt, ShiftedOutOne);

    // All cases returning poison have been handled by MaxShiftAmount already.
    എങ്കിൽ (NSW) {
      എങ്കിൽ (NUW && ShiftAmt != 0)
        // NUW means we can assume anything shifted out was a zero.
        ShiftedOutZero = സത്യം;

      എങ്കിൽ (ShiftedOutZero)
        Known.makeNonNegative();
      അല്ലെങ്കിൽ എങ്കിൽ (ShiftedOutOne)
        Known.makeNegative();
    }
    തിരികെ Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_മൂല്യം().get_Limited_മൂല്യം(BitWidth);
  എങ്കിൽ (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  എങ്കിൽ (LHS.isUnknown()) {
    Known.Zero.set_താഴെയുള്ള_Bits(MinShiftAmount);
    എങ്കിൽ (NUW && NSW && MinShiftAmount != 0)
      Known.makeNonNegative();
    തിരികെ Known;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_മൂല്യം = RHS.get_Max_മൂല്യം();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_മൂല്യം, BitWidth);
  എങ്കിൽ (NUW && NSW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.എണ്ണം_Max_Leading_Zeros() - 1);
  എങ്കിൽ (NUW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.എണ്ണം_Max_Leading_Zeros());
  എങ്കിൽ (NSW)
    MaxShiftAmount = std::min(
        MaxShiftAmount,
        std::max(LHS.എണ്ണം_Max_Leading_Zeros(), LHS.എണ്ണം_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  എങ്കിൽ (MinShiftAmount == 0 && MaxShiftAmount == BitWidth - 1 &&
      isPowerOf2_32(BitWidth)) {
    Known.Zero.set_താഴെയുള്ള_Bits(LHS.എണ്ണം_Min_Trailing_Zeros());
    എങ്കിൽ (LHS.isAllOnes())
      Known.One.setSignBit();
    എങ്കിൽ (NSW) {
      എങ്കിൽ (LHS.isNonNegative())
        Known.makeNonNegative();
      എങ്കിൽ (LHS.isNegative())
        Known.makeNegative();
    }
    തിരികെ Known;
  }

  // Find the common bits from all possible shifts.
  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_മൂല്യം();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_മൂല്യം();
  Known.setAllConflict();
  വേണ്ടി (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    എങ്കിൽ ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      തുടരുക;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    എങ്കിൽ (Known.isUnknown())
      നിർത്തുക;
  }

  // All shift amounts may result in poison.
  എങ്കിൽ (Known.hasConflict())
    Known.setAllZero();
  തിരികെ Known;
}

KnownBits KnownBits::lshr(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS,
                          തർക്കം ShAmtNonZero, തർക്കം Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](സ്ഥിരം KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known >>= ShiftAmt;
    // High bits are known zero.
    Known.Zero.set_മുകളിലെ_Bits(ShiftAmt);
    തിരികെ Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_മൂല്യം().get_Limited_മൂല്യം(BitWidth);
  എങ്കിൽ (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  എങ്കിൽ (LHS.isUnknown()) {
    Known.Zero.set_മുകളിലെ_Bits(MinShiftAmount);
    തിരികെ Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_മൂല്യം = RHS.get_Max_മൂല്യം();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_മൂല്യം, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  എങ്കിൽ (Exact) {
    unsigned FirstOne = LHS.എണ്ണം_Max_Trailing_Zeros();
    എങ്കിൽ (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      തിരികെ Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_മൂല്യം();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_മൂല്യം();
  Known.setAllConflict();
  വേണ്ടി (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    എങ്കിൽ ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      തുടരുക;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    എങ്കിൽ (Known.isUnknown())
      നിർത്തുക;
  }

  // All shift amounts may result in poison.
  എങ്കിൽ (Known.hasConflict())
    Known.setAllZero();
  തിരികെ Known;
}

KnownBits KnownBits::ashr(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS,
                          തർക്കം ShAmtNonZero, തർക്കം Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](സ്ഥിരം KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known.Zero.ashrInPlace(ShiftAmt);
    Known.One.ashrInPlace(ShiftAmt);
    തിരികെ Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_മൂല്യം().get_Limited_മൂല്യം(BitWidth);
  എങ്കിൽ (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  എങ്കിൽ (LHS.isUnknown()) {
    എങ്കിൽ (MinShiftAmount == BitWidth) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      തിരികെ Known;
    }
    തിരികെ Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_മൂല്യം = RHS.get_Max_മൂല്യം();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_മൂല്യം, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  എങ്കിൽ (Exact) {
    unsigned FirstOne = LHS.എണ്ണം_Max_Trailing_Zeros();
    എങ്കിൽ (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      തിരികെ Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_മൂല്യം();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_മൂല്യം();
  Known.setAllConflict();
  വേണ്ടി (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
      ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    എങ്കിൽ ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      തുടരുക;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    എങ്കിൽ (Known.isUnknown())
      നിർത്തുക;
  }

  // All shift amounts may result in poison.
  എങ്കിൽ (Known.hasConflict())
    Known.setAllZero();
  തിരികെ Known;
}

KnownBits KnownBits::fshl(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS,
                          സ്ഥിരം APInt &Amt) {
  തിരികെ KnownBits(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::fshr(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS,
                          സ്ഥിരം APInt &Amt) {
  തിരികെ KnownBits(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::clmul(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  KnownBits Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_മൂല്യം(), RHS.get_Min_മൂല്യം()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  auto ClMulOr = [](സ്ഥിരം APInt &LHS, സ്ഥിരം APInt &RHS) {
    unsigned BW = LHS.getBitWidth();
    പ്രതിജ്ഞാ(BW == RHS.getBitWidth() && "Operand mismatch");
    APInt Result(BW, 0);
    വേണ്ടി (unsigned I :
         seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
      എങ്കിൽ (RHS[I])
        Result |= LHS << I;
    തിരികെ Result;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt Known = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= Known;
  Res.One &= Known;

  തിരികെ Res;
}

KnownBits KnownBits::pext(സ്ഥിരം KnownBits &Val, സ്ഥിരം KnownBits &Mask) {
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
  വേണ്ടി (unsigned I = 0; I < BitWidth; ++I) {
    എങ്കിൽ (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      എങ്കിൽ (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clearBits(M0, M1 + 1);
      എങ്കിൽ (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clearBits(M0, M1 + 1);
    }
    എങ്കിൽ (Mask.One[I])
      ++M0, ++M1;
    അല്ലെങ്കിൽ എങ്കിൽ (!Mask.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clearBits(M0, BitWidth);
  തിരികെ Res;
}

KnownBits KnownBits::pdep(സ്ഥിരം KnownBits &Val, സ്ഥിരം KnownBits &Mask) {
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
  വേണ്ടി (unsigned I = 0; I < BitWidth; ++I) {
    എങ്കിൽ (!Mask.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clearBit(I);
    എങ്കിൽ (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::getBitsSet(BitWidth, M0, M1 + 1);
      എങ്കിൽ (!Range.isSubsetOf(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clearBit(I);
      എങ്കിൽ (!Range.isSubsetOf(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clearBit(I);
    }
    എങ്കിൽ (Mask.One[I])
      ++M0, ++M1;
    അല്ലെങ്കിൽ എങ്കിൽ (!Mask.Zero[I])
      ++M1;
  }
  തിരികെ Res;
}

std::optional<തർക്കം> KnownBits::eq(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  എങ്കിൽ (LHS.isConstant() && RHS.isConstant())
    തിരികെ LHS.getConstant() == RHS.getConstant();
  എങ്കിൽ (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    തിരികെ അസത്യം;
  തിരികെ std::nullopt;
}

std::optional<തർക്കം> KnownBits::ne(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  എങ്കിൽ (std::optional<തർക്കം> KnownEQ = eq(LHS, RHS))
    തിരികെ !*KnownEQ;
  തിരികെ std::nullopt;
}

std::optional<തർക്കം> KnownBits::ugt(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  എങ്കിൽ (LHS.get_Max_മൂല്യം().ule(RHS.get_Min_മൂല്യം()))
    തിരികെ അസത്യം;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  എങ്കിൽ (LHS.get_Min_മൂല്യം().ugt(RHS.get_Max_മൂല്യം()))
    തിരികെ സത്യം;
  തിരികെ std::nullopt;
}

std::optional<തർക്കം> KnownBits::uge(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  എങ്കിൽ (std::optional<തർക്കം> IsUGT = ugt(RHS, LHS))
    തിരികെ !*IsUGT;
  തിരികെ std::nullopt;
}

std::optional<തർക്കം> KnownBits::ult(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  തിരികെ ugt(RHS, LHS);
}

std::optional<തർക്കം> KnownBits::ule(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  തിരികെ uge(RHS, LHS);
}

std::optional<തർക്കം> KnownBits::sgt(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  എങ്കിൽ (LHS.get_Signed_Max_മൂല്യം().sle(RHS.get_Signed_Min_മൂല്യം()))
    തിരികെ അസത്യം;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  എങ്കിൽ (LHS.get_Signed_Min_മൂല്യം().sgt(RHS.get_Signed_Max_മൂല്യം()))
    തിരികെ സത്യം;
  തിരികെ std::nullopt;
}

std::optional<തർക്കം> KnownBits::sge(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  എങ്കിൽ (std::optional<തർക്കം> KnownSGT = sgt(RHS, LHS))
    തിരികെ !*KnownSGT;
  തിരികെ std::nullopt;
}

std::optional<തർക്കം> KnownBits::slt(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  തിരികെ sgt(RHS, LHS);
}

std::optional<തർക്കം> KnownBits::sle(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  തിരികെ sge(RHS, LHS);
}

KnownBits KnownBits::abs(തർക്കം IntMinIsPoison) സ്ഥിരം {
  // If the source's MSB is zero then we know the rest of the bits already.
  എങ്കിൽ (isNonNegative())
    തിരികെ *ഇത്;

  // Absolute value preserves trailing zero count.
  KnownBits KnownAbs(getBitWidth());

  // If the input is negative, then abs(x) == -x.
  എങ്കിൽ (isNegative()) {
    KnownBits Tmp = *ഇത്;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    എങ്കിൽ (IntMinIsPoison && (Zero.popcount() + 2) == getBitWidth())
      Tmp.One.setBit(എണ്ണം_Min_Trailing_Zeros());

    KnownAbs = computeForAddSub(
        /*Add*/ അസത്യം, IntMinIsPoison, /*NUW=*/അസത്യം,
        KnownBits::makeConstant(APInt(getBitWidth(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    എങ്കിൽ (IntMinIsPoison && Tmp.എണ്ണം_Min_Population() == 1 &&
        Tmp.എണ്ണം_Max_Population() != 1) {
      Tmp.One.clearSignBit();
      Tmp.Zero.setSignBit();
      KnownAbs.One.setBits(getBitWidth() - Tmp.എണ്ണം_Min_Leading_Zeros(),
                           getBitWidth() - 1);
    }

  } അല്ലെങ്കിൽ {
    unsigned MaxTZ = എണ്ണം_Max_Trailing_Zeros();
    unsigned MinTZ = എണ്ണം_Min_Trailing_Zeros();

    KnownAbs.Zero.set_താഴെയുള്ള_Bits(MinTZ);
    // If we know the lowest set 1, then preserve it.
    എങ്കിൽ (MaxTZ == MinTZ && MaxTZ < getBitWidth())
      KnownAbs.One.setBit(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    എങ്കിൽ (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_മൂല്യം())) {
      KnownAbs.One.clearSignBit();
      KnownAbs.Zero.setSignBit();
    }
  }

  തിരികെ KnownAbs;
}

KnownBits KnownBits::reduceAdd(unsigned അക്കം_Elts) സ്ഥിരം {
  എങ്കിൽ (അക്കം_Elts == 0)
    തിരികെ KnownBits(getBitWidth());

  unsigned BitWidth = getBitWidth();
  KnownBits Result(BitWidth);

  എങ്കിൽ (isConstant())
    // If all elements are the same constant, we can simply compute it
    തിരികെ KnownBits::makeConstant(അക്കം_Elts * getConstant());

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
  unsigned LostBits = Log2_32_Ceil(അക്കം_Elts);

  എങ്കിൽ (isNonNegative()) {
    unsigned LeadingZeros = എണ്ണം_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > LostBits ? LeadingZeros - LostBits : 0;
    Result.Zero.set_മുകളിലെ_Bits(LeadingZeros);
  } അല്ലെങ്കിൽ എങ്കിൽ (isNegative()) {
    unsigned LeadingOnes = എണ്ണം_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > LostBits ? LeadingOnes - LostBits : 0;
    Result.One.set_മുകളിലെ_Bits(LeadingOnes);
  }

  തിരികെ Result;
}

സ്ഥിതം KnownBits computeForSatAddSub(തർക്കം Add, തർക്കം Signed,
                                     സ്ഥിരം KnownBits &LHS,
                                     സ്ഥിരം KnownBits &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  unsigned BitWidth = LHS.getBitWidth();

  std::optional<തർക്കം> Overflow;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  തർക്കം MayNegClamp = സത്യം;
  തർക്കം MayPosClamp = സത്യം;
  എങ്കിൽ (Signed) {
    // Easy cases we can rule out any overflow.
    എങ്കിൽ (Add && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      Overflow = അസത്യം;
    അല്ലെങ്കിൽ എങ്കിൽ (!Add && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      Overflow = അസത്യം;
    അല്ലെങ്കിൽ {
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
          KnownBits::computeForAddSub(Add, /*NSW=*/അസത്യം,
                                      /*NUW=*/അസത്യം, UnsignedLHS, UnsignedRHS);
      എങ്കിൽ (Add) {
        എങ്കിൽ (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = അസത്യം;
          // Pos + Pos will overflow with extra signbit.
          എങ്കിൽ (LHS.isNonNegative() && RHS.isNonNegative())
            Overflow = സത്യം;
        } അല്ലെങ്കിൽ എങ്കിൽ (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = അസത്യം;
          // Neg + Neg will overflow without extra signbit.
          എങ്കിൽ (LHS.isNegative() && RHS.isNegative())
            Overflow = സത്യം;
        }
        // We will never clamp to the opposite sign of N-bit result.
        എങ്കിൽ (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = അസത്യം;
        എങ്കിൽ (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = അസത്യം;
      } അല്ലെങ്കിൽ {
        എങ്കിൽ (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = അസത്യം;
          // Neg - Pos will overflow with extra signbit.
          എങ്കിൽ (LHS.isNegative() && RHS.isNonNegative())
            Overflow = സത്യം;
        } അല്ലെങ്കിൽ എങ്കിൽ (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = അസത്യം;
          // Pos - Neg will overflow without extra signbit.
          എങ്കിൽ (LHS.isNonNegative() && RHS.isNegative())
            Overflow = സത്യം;
        }
        // We will never clamp to the opposite sign of N-bit result.
        എങ്കിൽ (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = അസത്യം;
        എങ്കിൽ (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = അസത്യം;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    എങ്കിൽ (!MayNegClamp && !MayPosClamp)
      Overflow = അസത്യം;
  } അല്ലെങ്കിൽ എങ്കിൽ (Add) {
    // uadd.sat
    തർക്കം Of;
    (ശൂന്യം)LHS.get_Max_മൂല്യം().uadd_ov(RHS.get_Max_മൂല്യം(), Of);
    എങ്കിൽ (!Of) {
      Overflow = അസത്യം;
    } അല്ലെങ്കിൽ {
      (ശൂന്യം)LHS.get_Min_മൂല്യം().uadd_ov(RHS.get_Min_മൂല്യം(), Of);
      എങ്കിൽ (Of)
        Overflow = സത്യം;
    }
  } അല്ലെങ്കിൽ {
    // usub.sat
    തർക്കം Of;
    (ശൂന്യം)LHS.get_Min_മൂല്യം().usub_ov(RHS.get_Max_മൂല്യം(), Of);
    എങ്കിൽ (!Of) {
      Overflow = അസത്യം;
    } അല്ലെങ്കിൽ {
      (ശൂന്യം)LHS.get_Max_മൂല്യം().usub_ov(RHS.get_Min_മൂല്യം(), Of);
      എങ്കിൽ (Of)
        Overflow = സത്യം;
    }
  }

  KnownBits Res = KnownBits::computeForAddSub(Add, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  എങ്കിൽ (Overflow) {
    // We know whether or not we overflowed.
    എങ്കിൽ (!(*Overflow)) {
      // No overflow.
      തിരികെ Res;
    }

    // We overflowed
    APInt C;
    എങ്കിൽ (Signed) {
      // sadd.sat / ssub.sat
      പ്രതിജ്ഞാ(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_മൂല്യം(BitWidth)
                           : APInt::get_Signed_Max_മൂല്യം(BitWidth);
    } അല്ലെങ്കിൽ എങ്കിൽ (Add) {
      // uadd.sat
      C = APInt::get_Max_മൂല്യം(BitWidth);
    } അല്ലെങ്കിൽ {
      // uadd.sat
      C = APInt::get_Min_മൂല്യം(BitWidth);
    }

    Res.One = C;
    Res.Zero = ~C;
    തിരികെ Res;
  }

  // We don't know if we overflowed.
  എങ്കിൽ (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    എങ്കിൽ (MayPosClamp)
      Res.Zero.clear_താഴെയുള്ള_Bits(BitWidth - 1);
    എങ്കിൽ (MayNegClamp)
      Res.One.clear_താഴെയുള്ള_Bits(BitWidth - 1);
  } അല്ലെങ്കിൽ എങ്കിൽ (Add) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clearAllBits();
  } അല്ലെങ്കിൽ {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clearAllBits();
  }

  തിരികെ Res;
}

KnownBits KnownBits::sadd_sat(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  തിരികെ computeForSatAddSub(/*Add*/ സത്യം, /*Signed*/ സത്യം, LHS, RHS);
}
KnownBits KnownBits::ssub_sat(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  തിരികെ computeForSatAddSub(/*Add*/ അസത്യം, /*Signed*/ സത്യം, LHS, RHS);
}
KnownBits KnownBits::uadd_sat(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  തിരികെ computeForSatAddSub(/*Add*/ സത്യം, /*Signed*/ അസത്യം, LHS, RHS);
}
KnownBits KnownBits::usub_sat(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  തിരികെ computeForSatAddSub(/*Add*/ അസത്യം, /*Signed*/ അസത്യം, LHS, RHS);
}

സ്ഥിതം KnownBits avgComputeU(KnownBits LHS, KnownBits RHS, തർക്കം IsCeil) {
  unsigned BitWidth = LHS.getBitWidth();
  LHS = LHS.zext(BitWidth + 1);
  RHS = RHS.zext(BitWidth + 1);
  LHS =
      computeForAddCarry(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.extractBits(BitWidth, 1);
  തിരികെ LHS;
}

KnownBits KnownBits::avgFloorS(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  തിരികെ flipSignBit(avgFloorU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgFloorU(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  തിരികെ avgComputeU(LHS, RHS, /*IsCeil=*/അസത്യം);
}

KnownBits KnownBits::avgCeilS(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  തിരികെ flipSignBit(avgCeilU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgCeilU(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  തിരികെ avgComputeU(LHS, RHS, /*IsCeil=*/സത്യം);
}

KnownBits KnownBits::mul(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS,
                         തർക്കം NoUndefSelfMultiply) {
  unsigned BitWidth = LHS.getBitWidth();
  പ്രതിജ്ഞാ(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  പ്രതിജ്ഞാ((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.get_Max_മൂല്യം();
  APInt UMaxRHS = RHS.get_Max_മൂല്യം();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  തർക്കം HasOverflow;
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
  unsigned TrailZeroLHS = LHS.എണ്ണം_Min_Trailing_Zeros();
  unsigned TrailZeroRHS = RHS.എണ്ണം_Min_Trailing_Zeros();
  unsigned TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  unsigned SmallestOperand = std::min(TrailBitsKnownLHS - TrailZeroLHS,
                                      TrailBitsKnownRHS - TrailZeroRHS);
  unsigned ResultBitsKnown = std::min(SmallestOperand + TrailZ, BitWidth);

  // The lower ResultBitsKnown bits of this are known.
  APInt BottomKnown = LHS.One * RHS.One;

  KnownBits Res(BitWidth);
  Res.Zero.set_മുകളിലെ_Bits(LeadZ);
  Res.Zero |= (~BottomKnown).getLoBits(ResultBitsKnown);
  Res.One = BottomKnown.getLoBits(ResultBitsKnown);

  എങ്കിൽ (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    unsigned TwoTZP1 = 2 * TrailZeroLHS + 1;
    എങ്കിൽ (TwoTZP1 < BitWidth)
      Res.Zero.setBit(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    എങ്കിൽ (TrailZeroLHS < BitWidth && LHS.One[TrailZeroLHS]) {
      unsigned TwoTZP2 = TwoTZP1 + 1;
      എങ്കിൽ (TwoTZP2 < BitWidth)
        Res.Zero.setBit(TwoTZP2);
    }
  }

  തിരികെ Res;
}

KnownBits KnownBits::mulhs(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  പ്രതിജ്ഞാ(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.sext(2 * BitWidth);
  KnownBits WideRHS = RHS.sext(2 * BitWidth);
  തിരികെ mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

KnownBits KnownBits::mulhu(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  പ്രതിജ്ഞാ(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.zext(2 * BitWidth);
  KnownBits WideRHS = RHS.zext(2 * BitWidth);
  തിരികെ mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

സ്ഥിതം KnownBits div_Compute_താഴെയുള്ള_Bit(KnownBits Known, സ്ഥിരം KnownBits &LHS,
                                  സ്ഥിരം KnownBits &RHS, തർക്കം Exact) {

  എങ്കിൽ (!Exact)
    തിരികെ Known;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  എങ്കിൽ (LHS.One[0])
    Known.One.setBit(0);

  പൂർണ്ണസംഖ്യ MinTZ =
      (പൂർണ്ണസംഖ്യ)LHS.എണ്ണം_Min_Trailing_Zeros() - (പൂർണ്ണസംഖ്യ)RHS.എണ്ണം_Max_Trailing_Zeros();
  പൂർണ്ണസംഖ്യ MaxTZ =
      (പൂർണ്ണസംഖ്യ)LHS.എണ്ണം_Max_Trailing_Zeros() - (പൂർണ്ണസംഖ്യ)RHS.എണ്ണം_Min_Trailing_Zeros();
  എങ്കിൽ (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    Known.Zero.set_താഴെയുള്ള_Bits(MinTZ);
    എങ്കിൽ (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      Known.One.setBit(MinTZ);
    }
  } അല്ലെങ്കിൽ എങ്കിൽ (MaxTZ < 0) {
    // Poison Result
    Known.setAllZero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  എങ്കിൽ (Known.hasConflict())
    Known.setAllZero();

  തിരികെ Known;
}

KnownBits KnownBits::sdiv(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS,
                          തർക്കം Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  എങ്കിൽ (LHS.isNonNegative() && RHS.isNonNegative())
    തിരികെ udiv(LHS, RHS, Exact);

  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  എങ്കിൽ (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    തിരികെ Known;
  }

  std::optional<APInt> Res;
  എങ്കിൽ (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_മൂല്യം();
    APInt അക്കം = LHS.get_Signed_Min_മൂല്യം();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (അക്കം.is_Min_Signed_മൂല്യം() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_മൂല്യം(BitWidth)
              : അക്കം.sdiv(Denom);
  } അല്ലെങ്കിൽ എങ്കിൽ (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    എങ്കിൽ (Exact || (-LHS.get_Signed_Max_മൂല്യം()).uge(RHS.get_Signed_Max_മൂല്യം())) {
      APInt Denom = RHS.get_Signed_Min_മൂല്യം();
      APInt അക്കം = LHS.get_Signed_Min_മൂല്യം();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      എങ്കിൽ (Denom.isZero())
        Denom.setBit(RHS.എണ്ണം_Min_Trailing_Zeros());
      Res = അക്കം.sdiv(Denom);
    }
  } അല്ലെങ്കിൽ എങ്കിൽ (LHS.isStrictlyPositive() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    എങ്കിൽ (Exact || LHS.get_Signed_Min_മൂല്യം().uge(-RHS.get_Signed_Min_മൂല്യം())) {
      APInt Denom = RHS.get_Signed_Max_മൂല്യം();
      APInt അക്കം = LHS.get_Signed_Max_മൂല്യം();
      Res = അക്കം.sdiv(Denom);
    }
  }

  എങ്കിൽ (Res) {
    എങ്കിൽ (Res->isNonNegative()) {
      unsigned LeadZ = Res->എണ്ണം_Leading_Zeros();
      Known.Zero.set_മുകളിലെ_Bits(LeadZ);
    } അല്ലെങ്കിൽ {
      unsigned LeadO = Res->എണ്ണം_Leading_Ones();
      Known.One.set_മുകളിലെ_Bits(LeadO);
    }
  }

  Known = div_Compute_താഴെയുള്ള_Bit(Known, LHS, RHS, Exact);
  തിരികെ Known;
}

KnownBits KnownBits::udiv(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS,
                          തർക്കം Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  എങ്കിൽ (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    തിരികെ Known;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_മൂല്യം();
  എങ്കിൽ (MinDenom.isZero())
    MinDenom.setBit(RHS.എണ്ണം_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_മൂല്യം().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_മൂല്യം().udiv(RHS.get_Max_മൂല്യം());

  Known = KnownBits::makeConstant(MinRes);
  എങ്കിൽ (std::optional<unsigned> DifferentBit =
          APIntOps::GetMostSignificantDifferentBit(MinRes, MaxRes)) {
    Known.Zero.clear_താഴെയുള്ള_Bits(*DifferentBit + 1);
    Known.One.clear_താഴെയുള്ള_Bits(*DifferentBit + 1);
  }

  Known = div_Compute_താഴെയുള്ള_Bit(Known, LHS, RHS, Exact);

  തിരികെ Known;
}

KnownBits KnownBits::rem_Get_താഴെയുള്ള_Bits(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  എങ്കിൽ (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    unsigned RHSZeros = RHS.എണ്ണം_Min_Trailing_Zeros();
    APInt Mask = APInt::get_താഴെയുള്ള_Bits_Set(BitWidth, RHSZeros);
    APInt OnesMask = LHS.One & Mask;
    APInt ZerosMask = LHS.Zero & Mask;
    തിരികെ KnownBits(ZerosMask, OnesMask);
  }
  തിരികെ KnownBits(BitWidth);
}

KnownBits KnownBits::urem(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  എങ്കിൽ (LHS.get_Max_മൂല്യം().ult(RHS.get_Min_മൂല്യം()))
    തിരികെ LHS;

  KnownBits Known = rem_Get_താഴെയുള്ള_Bits(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_മൂല്യം(), RHS.get_Max_മൂല്യം() - 1);
  Known.Zero.set_മുകളിലെ_Bits(MaxRes.എണ്ണം_Leading_Zeros());
  തിരികെ Known;
}

KnownBits KnownBits::srem(സ്ഥിരം KnownBits &LHS, സ്ഥിരം KnownBits &RHS) {
  KnownBits Known = rem_Get_താഴെയുള്ള_Bits(LHS, RHS);
  എങ്കിൽ (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt താഴെയുള്ള_Bits = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    എങ്കിൽ (LHS.isNonNegative() || താഴെയുള്ള_Bits.isSubsetOf(LHS.Zero))
      Known.Zero |= ~താഴെയുള്ള_Bits;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    എങ്കിൽ (LHS.isNegative() && താഴെയുള്ള_Bits.intersects(LHS.One))
      Known.One |= ~താഴെയുള്ള_Bits;
    തിരികെ Known;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  എങ്കിൽ (LHS.isNegative() && Known.isNonZero())
    Known.One.set_മുകളിലെ_Bits(
        std::max(LHS.എണ്ണം_Min_Leading_Ones(), RHS.എണ്ണം_Min_Sign_Bits()));
  അല്ലെങ്കിൽ എങ്കിൽ (LHS.isNonNegative())
    Known.Zero.set_മുകളിലെ_Bits(
        std::max(LHS.എണ്ണം_Min_Leading_Zeros(), RHS.എണ്ണം_Min_Sign_Bits()));
  തിരികെ Known;
}

KnownBits &KnownBits::operator&=(സ്ഥിരം KnownBits &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  തിരികെ *ഇത്;
}

KnownBits &KnownBits::operator|=(സ്ഥിരം KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  തിരികെ *ഇത്;
}

KnownBits &KnownBits::operator^=(സ്ഥിരം KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  തിരികെ *ഇത്;
}

KnownBits KnownBits::blsi() സ്ഥിരം {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(Zero, APInt(BitWidth, 0));
  unsigned Max = എണ്ണം_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = എണ്ണം_Min_Trailing_Zeros();
  എങ്കിൽ (Max == Min && Max < BitWidth)
    Known.One.setBit(Max);
  തിരികെ Known;
}

KnownBits KnownBits::blsmsk() സ്ഥിരം {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(BitWidth);
  unsigned Max = എണ്ണം_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = എണ്ണം_Min_Trailing_Zeros();
  Known.One.set_താഴെയുള്ള_Bits(std::min(Min + 1, BitWidth));
  തിരികെ Known;
}

ശൂന്യം KnownBits::print(raw_ostream &OS) സ്ഥിരം {
  unsigned BitWidth = getBitWidth();
  വേണ്ടി (unsigned I = 0; I < BitWidth; ++I) {
    unsigned N = BitWidth - I - 1;
    എങ്കിൽ (Zero[N] && One[N])
      OS << "!";
    അല്ലെങ്കിൽ എങ്കിൽ (Zero[N])
      OS << "0";
    അല്ലെങ്കിൽ എങ്കിൽ (One[N])
      OS << "1";
    അല്ലെങ്കിൽ
      OS << "?";
  }
}

#എങ്കിൽ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ശൂന്യം KnownBits::dump() സ്ഥിരം {
  print(dbgs());
  dbgs() << "\n";
}
#endif

unsigned llvm::SignBitsOps::rot(unsigned SrcSignBits, unsigned BitWidth,
                                std::optional<APInt> RotAmt,
                                തർക്കം Is_Rotate_വലത്) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  എങ്കിൽ (SrcSignBits == BitWidth)
    തിരികെ BitWidth;

  എങ്കിൽ (!RotAmt)
    തിരികെ 1;

  unsigned Amt = RotAmt->urem(BitWidth);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  എങ്കിൽ (Is_Rotate_വലത്)
    Amt = (BitWidth - Amt) % BitWidth;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  എങ്കിൽ (SrcSignBits > Amt + 1)
    തിരികെ SrcSignBits - Amt;
  തിരികെ 1;
}
