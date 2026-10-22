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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/KnownBits.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Sequence.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Debug.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <cassert>

using ନାମାକାଶ llvm;

KnownBits KnownBits::flipSignBit(ଅଚଳ KnownBits &Val) {
  unsigned SignBitPosition = Val.getBitWidth() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
  One.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
  ଫେରନ୍ତୁ KnownBits(Zero, One);
}

ସ୍ଥିର KnownBits computeForAddCarry(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS,
                                    ତାର୍କିକ CarryZero, ତାର୍କିକ CarryOne) {

  APInt PossibleSumZero = LHS.get_Max_ମୂଲ୍ୟ() + RHS.get_Max_ମୂଲ୍ୟ() + !CarryZero;
  APInt PossibleSumOne = LHS.get_Min_ମୂଲ୍ୟ() + RHS.get_Min_ମୂଲ୍ୟ() + CarryOne;

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
  ଫେରନ୍ତୁ KnownOut;
}

KnownBits KnownBits::computeForAddCarry(
    ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS, ଅଚଳ KnownBits &Carry) {
  ପ୍ରତିଜ୍ଞା(Carry.getBitWidth() == 1 && "Carry must be 1-bit");
  ଫେରନ୍ତୁ ::computeForAddCarry(
      LHS, RHS, Carry.Zero.get_Bool_ମୂଲ୍ୟ(), Carry.One.get_Bool_ମୂଲ୍ୟ());
}

KnownBits KnownBits::computeForAddSub(ତାର୍କିକ Add, ତାର୍କିକ NSW, ତାର୍କିକ NUW,
                                      ଅଚଳ KnownBits &LHS,
                                      ଅଚଳ KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits KnownOut(BitWidth);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  ଯଦି (LHS.isUnknown() && RHS.isUnknown())
    ଫେରନ୍ତୁ KnownOut;

  ଯଦି (!LHS.isUnknown() && !RHS.isUnknown()) {
    ଯଦି (Add) {
      // Sum = LHS + RHS + 0
      KnownOut = ::computeForAddCarry(LHS, RHS, /*CarryZero=*/ସତ୍ୟ,
                                      /*CarryOne=*/ମିଥ୍ୟା);
    } ଅନ୍ୟଥା {
      // Sum = LHS + ~RHS + 1
      KnownBits NotRHS = RHS;
      std::swap(NotRHS.Zero, NotRHS.One);
      KnownOut = ::computeForAddCarry(LHS, NotRHS, /*CarryZero=*/ମିଥ୍ୟା,
                                      /*CarryOne=*/ସତ୍ୟ);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  ଯଦି (NUW) {
    ଯଦି (Add) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_ମୂଲ୍ୟ().uadd_sat(RHS.get_Min_ମୂଲ୍ୟ());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      ଯଦି (NSW) {
        unsigned ଅଙ୍କ_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        KnownOut.One.setBits(BitWidth - 1 - ଅଙ୍କ_Bits_2, BitWidth - 1);
      }
      KnownOut.One.set_ଉପର_Bits(MinVal.countl_one());
    } ଅନ୍ୟଥା {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_ମୂଲ୍ୟ().usub_sat(RHS.get_Min_ମୂଲ୍ୟ());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      ଯଦି (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        unsigned ଅଙ୍କ_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
        KnownOut.Zero.setBits(BitWidth - 1 - ଅଙ୍କ_Bits_2, BitWidth - 1);
      }
      KnownOut.Zero.set_ଉପର_Bits(MaxVal.countl_zero());
    }
  }

  ଯଦି (NSW) {
    APInt MinVal;
    APInt MaxVal;
    ଯଦି (Add) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_ମୂଲ୍ୟ().sadd_sat(RHS.get_Signed_Min_ମୂଲ୍ୟ());
      MaxVal = LHS.get_Signed_Max_ମୂଲ୍ୟ().sadd_sat(RHS.get_Signed_Max_ମୂଲ୍ୟ());
    } ଅନ୍ୟଥା {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_ମୂଲ୍ୟ().ssub_sat(RHS.get_Signed_Max_ମୂଲ୍ୟ());
      MaxVal = LHS.get_Signed_Max_ମୂଲ୍ୟ().ssub_sat(RHS.get_Signed_Min_ମୂଲ୍ୟ());
    }
    ଯଦି (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      unsigned ଅଙ୍କ_Bits_2 = MinVal.trunc(BitWidth - 1).countl_one();
      KnownOut.One.setBits(BitWidth - 1 - ଅଙ୍କ_Bits_2, BitWidth - 1);
      KnownOut.Zero.setSignBit();
    }
    ଯଦି (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      unsigned ଅଙ୍କ_Bits_2 = MaxVal.trunc(BitWidth - 1).countl_zero();
      KnownOut.Zero.setBits(BitWidth - 1 - ଅଙ୍କ_Bits_2, BitWidth - 1);
      KnownOut.One.setSignBit();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  ଯଦି (KnownOut.hasConflict())
    KnownOut.setAllZero();
  ଫେରନ୍ତୁ KnownOut;
}

KnownBits KnownBits::computeForSubBorrow(ଅଚଳ KnownBits &LHS, KnownBits RHS,
                                         ଅଚଳ KnownBits &Borrow) {
  ପ୍ରତିଜ୍ଞା(Borrow.getBitWidth() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::swap(RHS.Zero, RHS.One);
  ଫେରନ୍ତୁ ::computeForAddCarry(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_ମୂଲ୍ୟ(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_ମୂଲ୍ୟ());
}

KnownBits KnownBits::truncSSat(unsigned BitWidth) ଅଚଳ {
  unsigned ନିବେଶ_Bits = getBitWidth();
  APInt MinInRange = APInt::get_Signed_Min_ମୂଲ୍ୟ(BitWidth).sext(ନିବେଶ_Bits);
  APInt MaxInRange = APInt::get_Signed_Max_ମୂଲ୍ୟ(BitWidth).sext(ନିବେଶ_Bits);
  APInt ନିବେଶ_Min = get_Signed_Min_ମୂଲ୍ୟ();
  APInt ନିବେଶ_Max = get_Signed_Max_ମୂଲ୍ୟ();
  KnownBits Known(BitWidth);

  // Case 1: All values fit - just truncate
  ଯଦି (ନିବେଶ_Min.sge(MinInRange) && ନିବେଶ_Max.sle(MaxInRange)) {
    Known = trunc(BitWidth);
  }
  // Case 2: All saturate to min
  ଅନ୍ୟଥା ଯଦି (ନିବେଶ_Max.slt(MinInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Min_ମୂଲ୍ୟ(BitWidth));
  }
  // Case 3: All saturate to max
  ଅନ୍ୟଥା ଯଦି (ନିବେଶ_Min.sgt(MaxInRange)) {
    Known = KnownBits::makeConstant(APInt::get_Signed_Max_ମୂଲ୍ୟ(BitWidth));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  ଅନ୍ୟଥା ଯଦି (ନିବେଶ_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    Known.Zero = APInt(BitWidth, 0);
    Known.Zero.setBit(BitWidth - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    Known.One = One.trunc(BitWidth);
    Known.One.clearBit(BitWidth - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  ଅନ୍ୟଥା ଯଦି (ନିବେଶ_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    Known.Zero = Zero.trunc(BitWidth);
    Known.Zero.clearBit(BitWidth - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    Known.One = APInt(BitWidth, 0);
    Known.One.setBit(BitWidth - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  ଅନ୍ୟଥା {
    // Output: min saturation, truncated, or max saturation
    APInt ନିବେଶ_Zero_Lower = Zero.trunc(BitWidth);
    APInt ନିବେଶ_One_Lower = One.trunc(BitWidth);
    APInt MinSat = APInt::get_Signed_Min_ମୂଲ୍ୟ(BitWidth);
    APInt MaxSat = APInt::get_Signed_Max_ମୂଲ୍ୟ(BitWidth);
    KnownBits MinSatKB = KnownBits::makeConstant(MinSat);
    KnownBits MaxSatKB = KnownBits::makeConstant(MaxSat);
    ଯଦି (ନିବେଶ_Max.sle(MaxInRange)) {
      // Positive values fit, only negatives saturate to min
      Known.Zero = ନିବେଶ_Zero_Lower & MinSatKB.Zero;
      Known.One = ନିବେଶ_One_Lower & MinSatKB.One;
    } ଅନ୍ୟଥା ଯଦି (ନିବେଶ_Min.sge(MinInRange)) {
      // Negative values fit, only positives saturate to max
      Known.Zero = ନିବେଶ_Zero_Lower & MaxSatKB.Zero;
      Known.One = ନିବେଶ_One_Lower & MaxSatKB.One;
    } ଅନ୍ୟଥା {
      // Both positive and negative values might saturate
      Known.Zero = ନିବେଶ_Zero_Lower & MinSatKB.Zero & MaxSatKB.Zero;
      Known.One = ନିବେଶ_One_Lower & MinSatKB.One & MaxSatKB.One;
    }
  }
  ଫେରନ୍ତୁ Known;
}

KnownBits KnownBits::truncSSatU(unsigned BitWidth) ଅଚଳ {
  unsigned ନିବେଶ_Bits = getBitWidth();
  APInt MaxInRange = APInt::getAllOnes(BitWidth).zext(ନିବେଶ_Bits);
  APInt ନିବେଶ_Min = get_Signed_Min_ମୂଲ୍ୟ();
  APInt ନିବେଶ_Max = get_Signed_Max_ମୂଲ୍ୟ();
  KnownBits Known(BitWidth);

  ଯଦି (isNegative()) {
    Known.setAllZero();
  } ଅନ୍ୟଥା ଯଦି (ନିବେଶ_Min.isNonNegative() && ନିବେଶ_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } ଅନ୍ୟଥା ଯଦି (ନିବେଶ_Min.isNonNegative() && ନିବେଶ_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } ଅନ୍ୟଥା ଯଦି (ନିବେଶ_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    Known.Zero = APInt(BitWidth, 0);
    // Common one bits: bits that are 1 in input stay 1
    Known.One = One.trunc(BitWidth);
  } ଅନ୍ୟଥା {
    // Mixed: sign bit unknown
    ଯଦି (ନିବେଶ_Max.ule(MaxInRange)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      Known.Zero = Zero.trunc(BitWidth);
      Known.One = APInt(BitWidth, 0);
    } ଅନ୍ୟଥା {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      Known.Zero = APInt(BitWidth, 0);
      Known.One = APInt(BitWidth, 0);
    }
  }
  ଫେରନ୍ତୁ Known;
}

KnownBits KnownBits::truncUSat(unsigned BitWidth) ଅଚଳ {
  unsigned ନିବେଶ_Bits = getBitWidth();
  APInt MaxInRange = APInt::get_ତଳ_Bits_Set(ନିବେଶ_Bits, BitWidth);
  APInt ନିବେଶ_Max = get_Max_ମୂଲ୍ୟ();
  APInt ନିବେଶ_Min = get_Min_ମୂଲ୍ୟ();
  KnownBits Known(BitWidth);

  ଯଦି (ନିବେଶ_Max.ule(MaxInRange)) {
    Known = trunc(BitWidth);
  } ଅନ୍ୟଥା ଯଦି (ନିବେଶ_Min.ugt(MaxInRange)) {
    Known.setAllOnes();
  } ଅନ୍ୟଥା {
    Known.resetAll();
    Known.One = One.trunc(BitWidth);
  }
  ଫେରନ୍ତୁ Known;
}

KnownBits KnownBits::sextInReg(unsigned SrcBitWidth) ଅଚଳ {
  unsigned BitWidth = getBitWidth();
  ପ୍ରତିଜ୍ଞା(0 < SrcBitWidth && SrcBitWidth <= BitWidth &&
         "Illegal sext-in-register");

  ଯଦି (SrcBitWidth == BitWidth)
    ଫେରନ୍ତୁ *ଏହି;

  unsigned ExtBits = BitWidth - SrcBitWidth;
  KnownBits Result;
  Result.One = One << ExtBits;
  Result.Zero = Zero << ExtBits;
  Result.One.ashrInPlace(ExtBits);
  Result.Zero.ashrInPlace(ExtBits);
  ଫେରନ୍ତୁ Result;
}

KnownBits KnownBits::makeGE(ଅଚଳ APInt &Val) ଅଚଳ {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  unsigned N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_ତଳ_Bits(getBitWidth() - N);
  ଫେରନ୍ତୁ KnownBits(Zero, One | MaskedVal);
}

KnownBits KnownBits::umax(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  ଯଦି (LHS.get_Min_ମୂଲ୍ୟ().uge(RHS.get_Max_ମୂଲ୍ୟ()))
    ଫେରନ୍ତୁ LHS;
  ଯଦି (RHS.get_Min_ମୂଲ୍ୟ().uge(LHS.get_Max_ମୂଲ୍ୟ()))
    ଫେରନ୍ତୁ RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  KnownBits L = LHS.makeGE(RHS.get_Min_ମୂଲ୍ୟ());
  KnownBits R = RHS.makeGE(LHS.get_Min_ମୂଲ୍ୟ());
  ଫେରନ୍ତୁ L.intersectWith(R);
}

KnownBits KnownBits::umin(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](ଅଚଳ KnownBits &Val) { ଫେରନ୍ତୁ KnownBits(Val.One, Val.Zero); };
  ଫେରନ୍ତୁ Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::smax(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଫେରନ୍ତୁ flipSignBit(umax(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::smin(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  auto Flip = [](ଅଚଳ KnownBits &Val) {
    unsigned SignBitPosition = Val.getBitWidth() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.setBitVal(SignBitPosition, Val.Zero[SignBitPosition]);
    One.setBitVal(SignBitPosition, Val.One[SignBitPosition]);
    ଫେରନ୍ତୁ KnownBits(Zero, One);
  };
  ଫେରନ୍ତୁ Flip(umax(Flip(LHS), Flip(RHS)));
}

KnownBits KnownBits::abdu(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  ଯଦି (LHS.get_Min_ମୂଲ୍ୟ().uge(RHS.get_Max_ମୂଲ୍ୟ()))
    ଫେରନ୍ତୁ computeForAddSub(/*Add=*/ମିଥ୍ୟା, /*NSW=*/ମିଥ୍ୟା, /*NUW=*/ମିଥ୍ୟା, LHS,
                            RHS);
  ଯଦି (RHS.get_Min_ମୂଲ୍ୟ().uge(LHS.get_Max_ମୂଲ୍ୟ()))
    ଫେରନ୍ତୁ computeForAddSub(/*Add=*/ମିଥ୍ୟା, /*NSW=*/ମିଥ୍ୟା, /*NUW=*/ମିଥ୍ୟା, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/ମିଥ୍ୟା, /*NSW=*/ମିଥ୍ୟା, /*NUW=*/ସତ୍ୟ, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/ମିଥ୍ୟା, /*NSW=*/ମିଥ୍ୟା, /*NUW=*/ସତ୍ୟ, RHS, LHS);
  ଫେରନ୍ତୁ Diff0.intersectWith(Diff1);
}

KnownBits KnownBits::abds(KnownBits LHS, KnownBits RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  ଯଦି (LHS.get_Signed_Min_ମୂଲ୍ୟ().sge(RHS.get_Signed_Max_ମୂଲ୍ୟ()))
    ଫେରନ୍ତୁ computeForAddSub(/*Add=*/ମିଥ୍ୟା, /*NSW=*/ମିଥ୍ୟା, /*NUW=*/ମିଥ୍ୟା, LHS,
                            RHS);
  ଯଦି (RHS.get_Signed_Min_ମୂଲ୍ୟ().sge(LHS.get_Signed_Max_ମୂଲ୍ୟ()))
    ଫେରନ୍ତୁ computeForAddSub(/*Add=*/ମିଥ୍ୟା, /*NSW=*/ମିଥ୍ୟା, /*NUW=*/ମିଥ୍ୟା, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  unsigned SignBitPosition = LHS.getBitWidth() - 1;
  ପାଇଁ (auto Arg : {&LHS, &RHS}) {
    ତାର୍କିକ Tmp = Arg->Zero[SignBitPosition];
    Arg->Zero.setBitVal(SignBitPosition, Arg->One[SignBitPosition]);
    Arg->One.setBitVal(SignBitPosition, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  KnownBits Diff0 =
      computeForAddSub(/*Add=*/ମିଥ୍ୟା, /*NSW=*/ମିଥ୍ୟା, /*NUW=*/ସତ୍ୟ, LHS, RHS);
  KnownBits Diff1 =
      computeForAddSub(/*Add=*/ମିଥ୍ୟା, /*NSW=*/ମିଥ୍ୟା, /*NUW=*/ସତ୍ୟ, RHS, LHS);
  ଫେରନ୍ତୁ Diff0.intersectWith(Diff1);
}

ସ୍ଥିର unsigned getMaxShiftAmount(ଅଚଳ APInt &Max_ମୂଲ୍ୟ, unsigned BitWidth) {
  ଯଦି (isPowerOf2_32(BitWidth))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    ଫେରନ୍ତୁ Max_ମୂଲ୍ୟ.extract_Bits_As_ZExt_ମୂଲ୍ୟ(
        std::min(Log2_32(BitWidth), Max_ମୂଲ୍ୟ.getBitWidth()), 0);
  // This is only an approximate upper bound.
  ଫେରନ୍ତୁ Max_ମୂଲ୍ୟ.get_Limited_ମୂଲ୍ୟ(BitWidth - 1);
}

KnownBits KnownBits::shl(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS, ତାର୍କିକ NUW,
                         ତାର୍କିକ NSW, ତାର୍କିକ ShAmtNonZero) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ଅଚଳ KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known;
    ତାର୍କିକ ShiftedOutZero, ShiftedOutOne;
    Known.Zero = LHS.Zero.ushl_ov(ShiftAmt, ShiftedOutZero);
    Known.Zero.set_ତଳ_Bits(ShiftAmt);
    Known.One = LHS.One.ushl_ov(ShiftAmt, ShiftedOutOne);

    // All cases returning poison have been handled by MaxShiftAmount already.
    ଯଦି (NSW) {
      ଯଦି (NUW && ShiftAmt != 0)
        // NUW means we can assume anything shifted out was a zero.
        ShiftedOutZero = ସତ୍ୟ;

      ଯଦି (ShiftedOutZero)
        Known.makeNonNegative();
      ଅନ୍ୟଥା ଯଦି (ShiftedOutOne)
        Known.makeNegative();
    }
    ଫେରନ୍ତୁ Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_ମୂଲ୍ୟ().get_Limited_ମୂଲ୍ୟ(BitWidth);
  ଯଦି (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  ଯଦି (LHS.isUnknown()) {
    Known.Zero.set_ତଳ_Bits(MinShiftAmount);
    ଯଦି (NUW && NSW && MinShiftAmount != 0)
      Known.makeNonNegative();
    ଫେରନ୍ତୁ Known;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_ମୂଲ୍ୟ = RHS.get_Max_ମୂଲ୍ୟ();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_ମୂଲ୍ୟ, BitWidth);
  ଯଦି (NUW && NSW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.ଗଣନା_Max_Leading_Zeros() - 1);
  ଯଦି (NUW)
    MaxShiftAmount = std::min(MaxShiftAmount, LHS.ଗଣନା_Max_Leading_Zeros());
  ଯଦି (NSW)
    MaxShiftAmount = std::min(
        MaxShiftAmount,
        std::max(LHS.ଗଣନା_Max_Leading_Zeros(), LHS.ଗଣନା_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  ଯଦି (MinShiftAmount == 0 && MaxShiftAmount == BitWidth - 1 &&
      isPowerOf2_32(BitWidth)) {
    Known.Zero.set_ତଳ_Bits(LHS.ଗଣନା_Min_Trailing_Zeros());
    ଯଦି (LHS.isAllOnes())
      Known.One.setSignBit();
    ଯଦି (NSW) {
      ଯଦି (LHS.isNonNegative())
        Known.makeNonNegative();
      ଯଦି (LHS.isNegative())
        Known.makeNegative();
    }
    ଫେରନ୍ତୁ Known;
  }

  // Find the common bits from all possible shifts.
  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_ମୂଲ୍ୟ();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_ମୂଲ୍ୟ();
  Known.setAllConflict();
  ପାଇଁ (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    ଯଦି ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      ଜାରି;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    ଯଦି (Known.isUnknown())
      ଭାଙ୍ଗ;
  }

  // All shift amounts may result in poison.
  ଯଦି (Known.hasConflict())
    Known.setAllZero();
  ଫେରନ୍ତୁ Known;
}

KnownBits KnownBits::lshr(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS,
                          ତାର୍କିକ ShAmtNonZero, ତାର୍କିକ Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ଅଚଳ KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known >>= ShiftAmt;
    // High bits are known zero.
    Known.Zero.set_ଉପର_Bits(ShiftAmt);
    ଫେରନ୍ତୁ Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_ମୂଲ୍ୟ().get_Limited_ମୂଲ୍ୟ(BitWidth);
  ଯଦି (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  ଯଦି (LHS.isUnknown()) {
    Known.Zero.set_ଉପର_Bits(MinShiftAmount);
    ଫେରନ୍ତୁ Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_ମୂଲ୍ୟ = RHS.get_Max_ମୂଲ୍ୟ();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_ମୂଲ୍ୟ, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  ଯଦି (Exact) {
    unsigned FirstOne = LHS.ଗଣନା_Max_Trailing_Zeros();
    ଯଦି (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      ଫେରନ୍ତୁ Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_ମୂଲ୍ୟ();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_ମୂଲ୍ୟ();
  Known.setAllConflict();
  ପାଇଁ (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
       ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    ଯଦି ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      ଜାରି;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    ଯଦି (Known.isUnknown())
      ଭାଙ୍ଗ;
  }

  // All shift amounts may result in poison.
  ଯଦି (Known.hasConflict())
    Known.setAllZero();
  ଫେରନ୍ତୁ Known;
}

KnownBits KnownBits::ashr(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS,
                          ତାର୍କିକ ShAmtNonZero, ତାର୍କିକ Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  auto ShiftByConst = [&](ଅଚଳ KnownBits &LHS, unsigned ShiftAmt) {
    KnownBits Known = LHS;
    Known.Zero.ashrInPlace(ShiftAmt);
    Known.One.ashrInPlace(ShiftAmt);
    ଫେରନ୍ତୁ Known;
  };

  // Fast path for a common case when LHS is completely unknown.
  KnownBits Known(BitWidth);
  unsigned MinShiftAmount = RHS.get_Min_ମୂଲ୍ୟ().get_Limited_ମୂଲ୍ୟ(BitWidth);
  ଯଦି (MinShiftAmount == 0 && ShAmtNonZero)
    MinShiftAmount = 1;
  ଯଦି (LHS.isUnknown()) {
    ଯଦି (MinShiftAmount == BitWidth) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      ଫେରନ୍ତୁ Known;
    }
    ଫେରନ୍ତୁ Known;
  }

  // Find the common bits from all possible shifts.
  APInt Max_ମୂଲ୍ୟ = RHS.get_Max_ମୂଲ୍ୟ();
  unsigned MaxShiftAmount = getMaxShiftAmount(Max_ମୂଲ୍ୟ, BitWidth);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  ଯଦି (Exact) {
    unsigned FirstOne = LHS.ଗଣନା_Max_Trailing_Zeros();
    ଯଦି (FirstOne < MinShiftAmount) {
      // Always poison. Return zero because we don't like returning conflict.
      Known.setAllZero();
      ଫେରନ୍ତୁ Known;
    }
    MaxShiftAmount = std::min(MaxShiftAmount, FirstOne);
  }

  unsigned ShiftAmtZeroMask = RHS.Zero.zextOrTrunc(32).get_ZExt_ମୂଲ୍ୟ();
  unsigned ShiftAmtOneMask = RHS.One.zextOrTrunc(32).get_ZExt_ମୂଲ୍ୟ();
  Known.setAllConflict();
  ପାଇଁ (unsigned ShiftAmt = MinShiftAmount; ShiftAmt <= MaxShiftAmount;
      ++ShiftAmt) {
    // Skip if the shift amount is impossible.
    ଯଦି ((ShiftAmtZeroMask & ShiftAmt) != 0 ||
        (ShiftAmtOneMask | ShiftAmt) != ShiftAmt)
      ଜାରି;
    Known = Known.intersectWith(ShiftByConst(LHS, ShiftAmt));
    ଯଦି (Known.isUnknown())
      ଭାଙ୍ଗ;
  }

  // All shift amounts may result in poison.
  ଯଦି (Known.hasConflict())
    Known.setAllZero();
  ଫେରନ୍ତୁ Known;
}

KnownBits KnownBits::fshl(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS,
                          ଅଚଳ APInt &Amt) {
  ଫେରନ୍ତୁ KnownBits(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::fshr(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS,
                          ଅଚଳ APInt &Amt) {
  ଫେରନ୍ତୁ KnownBits(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

KnownBits KnownBits::clmul(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  KnownBits Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_ମୂଲ୍ୟ(), RHS.get_Min_ମୂଲ୍ୟ()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  auto ClMulOr = [](ଅଚଳ APInt &LHS, ଅଚଳ APInt &RHS) {
    unsigned BW = LHS.getBitWidth();
    ପ୍ରତିଜ୍ଞା(BW == RHS.getBitWidth() && "Operand mismatch");
    APInt Result(BW, 0);
    ପାଇଁ (unsigned I :
         seq(std::min(RHS.getActiveBits(), BW - LHS.countr_zero())))
      ଯଦି (RHS[I])
        Result |= LHS << I;
    ଫେରନ୍ତୁ Result;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt Known = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= Known;
  Res.One &= Known;

  ଫେରନ୍ତୁ Res;
}

KnownBits KnownBits::pext(ଅଚଳ KnownBits &Val, ଅଚଳ KnownBits &Mask) {
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
  ପାଇଁ (unsigned I = 0; I < BitWidth; ++I) {
    ଯଦି (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      ଯଦି (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clearBits(M0, M1 + 1);
      ଯଦି (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clearBits(M0, M1 + 1);
    }
    ଯଦି (Mask.One[I])
      ++M0, ++M1;
    ଅନ୍ୟଥା ଯଦି (!Mask.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clearBits(M0, BitWidth);
  ଫେରନ୍ତୁ Res;
}

KnownBits KnownBits::pdep(ଅଚଳ KnownBits &Val, ଅଚଳ KnownBits &Mask) {
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
  ପାଇଁ (unsigned I = 0; I < BitWidth; ++I) {
    ଯଦି (!Mask.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clearBit(I);
    ଯଦି (!Mask.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::getBitsSet(BitWidth, M0, M1 + 1);
      ଯଦି (!Range.isSubsetOf(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clearBit(I);
      ଯଦି (!Range.isSubsetOf(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clearBit(I);
    }
    ଯଦି (Mask.One[I])
      ++M0, ++M1;
    ଅନ୍ୟଥା ଯଦି (!Mask.Zero[I])
      ++M1;
  }
  ଫେରନ୍ତୁ Res;
}

std::optional<ତାର୍କିକ> KnownBits::eq(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଯଦି (LHS.isConstant() && RHS.isConstant())
    ଫେରନ୍ତୁ LHS.getConstant() == RHS.getConstant();
  ଯଦି (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଫେରନ୍ତୁ std::nullopt;
}

std::optional<ତାର୍କିକ> KnownBits::ne(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଯଦି (std::optional<ତାର୍କିକ> KnownEQ = eq(LHS, RHS))
    ଫେରନ୍ତୁ !*KnownEQ;
  ଫେରନ୍ତୁ std::nullopt;
}

std::optional<ତାର୍କିକ> KnownBits::ugt(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  ଯଦି (LHS.get_Max_ମୂଲ୍ୟ().ule(RHS.get_Min_ମୂଲ୍ୟ()))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  ଯଦି (LHS.get_Min_ମୂଲ୍ୟ().ugt(RHS.get_Max_ମୂଲ୍ୟ()))
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଫେରନ୍ତୁ std::nullopt;
}

std::optional<ତାର୍କିକ> KnownBits::uge(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଯଦି (std::optional<ତାର୍କିକ> IsUGT = ugt(RHS, LHS))
    ଫେରନ୍ତୁ !*IsUGT;
  ଫେରନ୍ତୁ std::nullopt;
}

std::optional<ତାର୍କିକ> KnownBits::ult(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଫେରନ୍ତୁ ugt(RHS, LHS);
}

std::optional<ତାର୍କିକ> KnownBits::ule(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଫେରନ୍ତୁ uge(RHS, LHS);
}

std::optional<ତାର୍କିକ> KnownBits::sgt(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  ଯଦି (LHS.get_Signed_Max_ମୂଲ୍ୟ().sle(RHS.get_Signed_Min_ମୂଲ୍ୟ()))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  ଯଦି (LHS.get_Signed_Min_ମୂଲ୍ୟ().sgt(RHS.get_Signed_Max_ମୂଲ୍ୟ()))
    ଫେରନ୍ତୁ ସତ୍ୟ;
  ଫେରନ୍ତୁ std::nullopt;
}

std::optional<ତାର୍କିକ> KnownBits::sge(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଯଦି (std::optional<ତାର୍କିକ> KnownSGT = sgt(RHS, LHS))
    ଫେରନ୍ତୁ !*KnownSGT;
  ଫେରନ୍ତୁ std::nullopt;
}

std::optional<ତାର୍କିକ> KnownBits::slt(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଫେରନ୍ତୁ sgt(RHS, LHS);
}

std::optional<ତାର୍କିକ> KnownBits::sle(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଫେରନ୍ତୁ sge(RHS, LHS);
}

KnownBits KnownBits::abs(ତାର୍କିକ IntMinIsPoison) ଅଚଳ {
  // If the source's MSB is zero then we know the rest of the bits already.
  ଯଦି (isNonNegative())
    ଫେରନ୍ତୁ *ଏହି;

  // Absolute value preserves trailing zero count.
  KnownBits KnownAbs(getBitWidth());

  // If the input is negative, then abs(x) == -x.
  ଯଦି (isNegative()) {
    KnownBits Tmp = *ଏହି;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    ଯଦି (IntMinIsPoison && (Zero.popcount() + 2) == getBitWidth())
      Tmp.One.setBit(ଗଣନା_Min_Trailing_Zeros());

    KnownAbs = computeForAddSub(
        /*Add*/ ମିଥ୍ୟା, IntMinIsPoison, /*NUW=*/ମିଥ୍ୟା,
        KnownBits::makeConstant(APInt(getBitWidth(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    ଯଦି (IntMinIsPoison && Tmp.ଗଣନା_Min_Population() == 1 &&
        Tmp.ଗଣନା_Max_Population() != 1) {
      Tmp.One.clearSignBit();
      Tmp.Zero.setSignBit();
      KnownAbs.One.setBits(getBitWidth() - Tmp.ଗଣନା_Min_Leading_Zeros(),
                           getBitWidth() - 1);
    }

  } ଅନ୍ୟଥା {
    unsigned MaxTZ = ଗଣନା_Max_Trailing_Zeros();
    unsigned MinTZ = ଗଣନା_Min_Trailing_Zeros();

    KnownAbs.Zero.set_ତଳ_Bits(MinTZ);
    // If we know the lowest set 1, then preserve it.
    ଯଦି (MaxTZ == MinTZ && MaxTZ < getBitWidth())
      KnownAbs.One.setBit(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    ଯଦି (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_ମୂଲ୍ୟ())) {
      KnownAbs.One.clearSignBit();
      KnownAbs.Zero.setSignBit();
    }
  }

  ଫେରନ୍ତୁ KnownAbs;
}

KnownBits KnownBits::reduceAdd(unsigned ଅଙ୍କ_Elts) ଅଚଳ {
  ଯଦି (ଅଙ୍କ_Elts == 0)
    ଫେରନ୍ତୁ KnownBits(getBitWidth());

  unsigned BitWidth = getBitWidth();
  KnownBits Result(BitWidth);

  ଯଦି (isConstant())
    // If all elements are the same constant, we can simply compute it
    ଫେରନ୍ତୁ KnownBits::makeConstant(ଅଙ୍କ_Elts * getConstant());

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
  unsigned LostBits = Log2_32_Ceil(ଅଙ୍କ_Elts);

  ଯଦି (isNonNegative()) {
    unsigned LeadingZeros = ଗଣନା_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > LostBits ? LeadingZeros - LostBits : 0;
    Result.Zero.set_ଉପର_Bits(LeadingZeros);
  } ଅନ୍ୟଥା ଯଦି (isNegative()) {
    unsigned LeadingOnes = ଗଣନା_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > LostBits ? LeadingOnes - LostBits : 0;
    Result.One.set_ଉପର_Bits(LeadingOnes);
  }

  ଫେରନ୍ତୁ Result;
}

ସ୍ଥିର KnownBits computeForSatAddSub(ତାର୍କିକ Add, ତାର୍କିକ Signed,
                                     ଅଚଳ KnownBits &LHS,
                                     ଅଚଳ KnownBits &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  unsigned BitWidth = LHS.getBitWidth();

  std::optional<ତାର୍କିକ> Overflow;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  ତାର୍କିକ MayNegClamp = ସତ୍ୟ;
  ତାର୍କିକ MayPosClamp = ସତ୍ୟ;
  ଯଦି (Signed) {
    // Easy cases we can rule out any overflow.
    ଯଦି (Add && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      Overflow = ମିଥ୍ୟା;
    ଅନ୍ୟଥା ଯଦି (!Add && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      Overflow = ମିଥ୍ୟା;
    ଅନ୍ୟଥା {
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
          KnownBits::computeForAddSub(Add, /*NSW=*/ମିଥ୍ୟା,
                                      /*NUW=*/ମିଥ୍ୟା, UnsignedLHS, UnsignedRHS);
      ଯଦି (Add) {
        ଯଦି (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = ମିଥ୍ୟା;
          // Pos + Pos will overflow with extra signbit.
          ଯଦି (LHS.isNonNegative() && RHS.isNonNegative())
            Overflow = ସତ୍ୟ;
        } ଅନ୍ୟଥା ଯଦି (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = ମିଥ୍ୟା;
          // Neg + Neg will overflow without extra signbit.
          ଯଦି (LHS.isNegative() && RHS.isNegative())
            Overflow = ସତ୍ୟ;
        }
        // We will never clamp to the opposite sign of N-bit result.
        ଯଦି (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = ମିଥ୍ୟା;
        ଯଦି (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = ମିଥ୍ୟା;
      } ଅନ୍ୟଥା {
        ଯଦି (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = ମିଥ୍ୟା;
          // Neg - Pos will overflow with extra signbit.
          ଯଦି (LHS.isNegative() && RHS.isNonNegative())
            Overflow = ସତ୍ୟ;
        } ଅନ୍ୟଥା ଯଦି (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = ମିଥ୍ୟା;
          // Pos - Neg will overflow without extra signbit.
          ଯଦି (LHS.isNonNegative() && RHS.isNegative())
            Overflow = ସତ୍ୟ;
        }
        // We will never clamp to the opposite sign of N-bit result.
        ଯଦି (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = ମିଥ୍ୟା;
        ଯଦି (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = ମିଥ୍ୟା;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    ଯଦି (!MayNegClamp && !MayPosClamp)
      Overflow = ମିଥ୍ୟା;
  } ଅନ୍ୟଥା ଯଦି (Add) {
    // uadd.sat
    ତାର୍କିକ Of;
    (ଶୂନ୍ୟ)LHS.get_Max_ମୂଲ୍ୟ().uadd_ov(RHS.get_Max_ମୂଲ୍ୟ(), Of);
    ଯଦି (!Of) {
      Overflow = ମିଥ୍ୟା;
    } ଅନ୍ୟଥା {
      (ଶୂନ୍ୟ)LHS.get_Min_ମୂଲ୍ୟ().uadd_ov(RHS.get_Min_ମୂଲ୍ୟ(), Of);
      ଯଦି (Of)
        Overflow = ସତ୍ୟ;
    }
  } ଅନ୍ୟଥା {
    // usub.sat
    ତାର୍କିକ Of;
    (ଶୂନ୍ୟ)LHS.get_Min_ମୂଲ୍ୟ().usub_ov(RHS.get_Max_ମୂଲ୍ୟ(), Of);
    ଯଦି (!Of) {
      Overflow = ମିଥ୍ୟା;
    } ଅନ୍ୟଥା {
      (ଶୂନ୍ୟ)LHS.get_Max_ମୂଲ୍ୟ().usub_ov(RHS.get_Min_ମୂଲ୍ୟ(), Of);
      ଯଦି (Of)
        Overflow = ସତ୍ୟ;
    }
  }

  KnownBits Res = KnownBits::computeForAddSub(Add, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  ଯଦି (Overflow) {
    // We know whether or not we overflowed.
    ଯଦି (!(*Overflow)) {
      // No overflow.
      ଫେରନ୍ତୁ Res;
    }

    // We overflowed
    APInt C;
    ଯଦି (Signed) {
      // sadd.sat / ssub.sat
      ପ୍ରତିଜ୍ଞା(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_ମୂଲ୍ୟ(BitWidth)
                           : APInt::get_Signed_Max_ମୂଲ୍ୟ(BitWidth);
    } ଅନ୍ୟଥା ଯଦି (Add) {
      // uadd.sat
      C = APInt::get_Max_ମୂଲ୍ୟ(BitWidth);
    } ଅନ୍ୟଥା {
      // uadd.sat
      C = APInt::get_Min_ମୂଲ୍ୟ(BitWidth);
    }

    Res.One = C;
    Res.Zero = ~C;
    ଫେରନ୍ତୁ Res;
  }

  // We don't know if we overflowed.
  ଯଦି (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    ଯଦି (MayPosClamp)
      Res.Zero.clear_ତଳ_Bits(BitWidth - 1);
    ଯଦି (MayNegClamp)
      Res.One.clear_ତଳ_Bits(BitWidth - 1);
  } ଅନ୍ୟଥା ଯଦି (Add) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clearAllBits();
  } ଅନ୍ୟଥା {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clearAllBits();
  }

  ଫେରନ୍ତୁ Res;
}

KnownBits KnownBits::sadd_sat(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଫେରନ୍ତୁ computeForSatAddSub(/*Add*/ ସତ୍ୟ, /*Signed*/ ସତ୍ୟ, LHS, RHS);
}
KnownBits KnownBits::ssub_sat(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଫେରନ୍ତୁ computeForSatAddSub(/*Add*/ ମିଥ୍ୟା, /*Signed*/ ସତ୍ୟ, LHS, RHS);
}
KnownBits KnownBits::uadd_sat(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଫେରନ୍ତୁ computeForSatAddSub(/*Add*/ ସତ୍ୟ, /*Signed*/ ମିଥ୍ୟା, LHS, RHS);
}
KnownBits KnownBits::usub_sat(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଫେରନ୍ତୁ computeForSatAddSub(/*Add*/ ମିଥ୍ୟା, /*Signed*/ ମିଥ୍ୟା, LHS, RHS);
}

ସ୍ଥିର KnownBits avgComputeU(KnownBits LHS, KnownBits RHS, ତାର୍କିକ IsCeil) {
  unsigned BitWidth = LHS.getBitWidth();
  LHS = LHS.zext(BitWidth + 1);
  RHS = RHS.zext(BitWidth + 1);
  LHS =
      computeForAddCarry(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.extractBits(BitWidth, 1);
  ଫେରନ୍ତୁ LHS;
}

KnownBits KnownBits::avgFloorS(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଫେରନ୍ତୁ flipSignBit(avgFloorU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgFloorU(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଫେରନ୍ତୁ avgComputeU(LHS, RHS, /*IsCeil=*/ମିଥ୍ୟା);
}

KnownBits KnownBits::avgCeilS(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଫେରନ୍ତୁ flipSignBit(avgCeilU(flipSignBit(LHS), flipSignBit(RHS)));
}

KnownBits KnownBits::avgCeilU(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  ଫେରନ୍ତୁ avgComputeU(LHS, RHS, /*IsCeil=*/ସତ୍ୟ);
}

KnownBits KnownBits::mul(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS,
                         ତାର୍କିକ NoUndefSelfMultiply) {
  unsigned BitWidth = LHS.getBitWidth();
  ପ୍ରତିଜ୍ଞା(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  ପ୍ରତିଜ୍ଞା((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.get_Max_ମୂଲ୍ୟ();
  APInt UMaxRHS = RHS.get_Max_ମୂଲ୍ୟ();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  ତାର୍କିକ HasOverflow;
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
  unsigned TrailZeroLHS = LHS.ଗଣନା_Min_Trailing_Zeros();
  unsigned TrailZeroRHS = RHS.ଗଣନା_Min_Trailing_Zeros();
  unsigned TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  unsigned SmallestOperand = std::min(TrailBitsKnownLHS - TrailZeroLHS,
                                      TrailBitsKnownRHS - TrailZeroRHS);
  unsigned ResultBitsKnown = std::min(SmallestOperand + TrailZ, BitWidth);

  // The lower ResultBitsKnown bits of this are known.
  APInt BottomKnown = LHS.One * RHS.One;

  KnownBits Res(BitWidth);
  Res.Zero.set_ଉପର_Bits(LeadZ);
  Res.Zero |= (~BottomKnown).getLoBits(ResultBitsKnown);
  Res.One = BottomKnown.getLoBits(ResultBitsKnown);

  ଯଦି (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    unsigned TwoTZP1 = 2 * TrailZeroLHS + 1;
    ଯଦି (TwoTZP1 < BitWidth)
      Res.Zero.setBit(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    ଯଦି (TrailZeroLHS < BitWidth && LHS.One[TrailZeroLHS]) {
      unsigned TwoTZP2 = TwoTZP1 + 1;
      ଯଦି (TwoTZP2 < BitWidth)
        Res.Zero.setBit(TwoTZP2);
    }
  }

  ଫେରନ୍ତୁ Res;
}

KnownBits KnownBits::mulhs(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  ପ୍ରତିଜ୍ଞା(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.sext(2 * BitWidth);
  KnownBits WideRHS = RHS.sext(2 * BitWidth);
  ଫେରନ୍ତୁ mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

KnownBits KnownBits::mulhu(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  ପ୍ରତିଜ୍ଞା(BitWidth == RHS.getBitWidth() && "Operand mismatch");
  KnownBits WideLHS = LHS.zext(2 * BitWidth);
  KnownBits WideRHS = RHS.zext(2 * BitWidth);
  ଫେରନ୍ତୁ mul(WideLHS, WideRHS).extractBits(BitWidth, BitWidth);
}

ସ୍ଥିର KnownBits div_Compute_ତଳ_Bit(KnownBits Known, ଅଚଳ KnownBits &LHS,
                                  ଅଚଳ KnownBits &RHS, ତାର୍କିକ Exact) {

  ଯଦି (!Exact)
    ଫେରନ୍ତୁ Known;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  ଯଦି (LHS.One[0])
    Known.One.setBit(0);

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା MinTZ =
      (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)LHS.ଗଣନା_Min_Trailing_Zeros() - (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)RHS.ଗଣନା_Max_Trailing_Zeros();
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା MaxTZ =
      (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)LHS.ଗଣନା_Max_Trailing_Zeros() - (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)RHS.ଗଣନା_Min_Trailing_Zeros();
  ଯଦି (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    Known.Zero.set_ତଳ_Bits(MinTZ);
    ଯଦି (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      Known.One.setBit(MinTZ);
    }
  } ଅନ୍ୟଥା ଯଦି (MaxTZ < 0) {
    // Poison Result
    Known.setAllZero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  ଯଦି (Known.hasConflict())
    Known.setAllZero();

  ଫେରନ୍ତୁ Known;
}

KnownBits KnownBits::sdiv(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS,
                          ତାର୍କିକ Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  ଯଦି (LHS.isNonNegative() && RHS.isNonNegative())
    ଫେରନ୍ତୁ udiv(LHS, RHS, Exact);

  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  ଯଦି (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    ଫେରନ୍ତୁ Known;
  }

  std::optional<APInt> Res;
  ଯଦି (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_ମୂଲ୍ୟ();
    APInt ଅଙ୍କ = LHS.get_Signed_Min_ମୂଲ୍ୟ();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (ଅଙ୍କ.is_Min_Signed_ମୂଲ୍ୟ() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_ମୂଲ୍ୟ(BitWidth)
              : ଅଙ୍କ.sdiv(Denom);
  } ଅନ୍ୟଥା ଯଦି (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    ଯଦି (Exact || (-LHS.get_Signed_Max_ମୂଲ୍ୟ()).uge(RHS.get_Signed_Max_ମୂଲ୍ୟ())) {
      APInt Denom = RHS.get_Signed_Min_ମୂଲ୍ୟ();
      APInt ଅଙ୍କ = LHS.get_Signed_Min_ମୂଲ୍ୟ();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      ଯଦି (Denom.isZero())
        Denom.setBit(RHS.ଗଣନା_Min_Trailing_Zeros());
      Res = ଅଙ୍କ.sdiv(Denom);
    }
  } ଅନ୍ୟଥା ଯଦି (LHS.isStrictlyPositive() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    ଯଦି (Exact || LHS.get_Signed_Min_ମୂଲ୍ୟ().uge(-RHS.get_Signed_Min_ମୂଲ୍ୟ())) {
      APInt Denom = RHS.get_Signed_Max_ମୂଲ୍ୟ();
      APInt ଅଙ୍କ = LHS.get_Signed_Max_ମୂଲ୍ୟ();
      Res = ଅଙ୍କ.sdiv(Denom);
    }
  }

  ଯଦି (Res) {
    ଯଦି (Res->isNonNegative()) {
      unsigned LeadZ = Res->ଗଣନା_Leading_Zeros();
      Known.Zero.set_ଉପର_Bits(LeadZ);
    } ଅନ୍ୟଥା {
      unsigned LeadO = Res->ଗଣନା_Leading_Ones();
      Known.One.set_ଉପର_Bits(LeadO);
    }
  }

  Known = div_Compute_ତଳ_Bit(Known, LHS, RHS, Exact);
  ଫେରନ୍ତୁ Known;
}

KnownBits KnownBits::udiv(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS,
                          ତାର୍କିକ Exact) {
  unsigned BitWidth = LHS.getBitWidth();
  KnownBits Known(BitWidth);

  ଯଦି (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    Known.setAllZero();
    ଫେରନ୍ତୁ Known;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_ମୂଲ୍ୟ();
  ଯଦି (MinDenom.isZero())
    MinDenom.setBit(RHS.ଗଣନା_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_ମୂଲ୍ୟ().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_ମୂଲ୍ୟ().udiv(RHS.get_Max_ମୂଲ୍ୟ());

  Known = KnownBits::makeConstant(MinRes);
  ଯଦି (std::optional<unsigned> DifferentBit =
          APIntOps::GetMostSignificantDifferentBit(MinRes, MaxRes)) {
    Known.Zero.clear_ତଳ_Bits(*DifferentBit + 1);
    Known.One.clear_ତଳ_Bits(*DifferentBit + 1);
  }

  Known = div_Compute_ତଳ_Bit(Known, LHS, RHS, Exact);

  ଫେରନ୍ତୁ Known;
}

KnownBits KnownBits::rem_Get_ତଳ_Bits(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  unsigned BitWidth = LHS.getBitWidth();
  ଯଦି (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    unsigned RHSZeros = RHS.ଗଣନା_Min_Trailing_Zeros();
    APInt Mask = APInt::get_ତଳ_Bits_Set(BitWidth, RHSZeros);
    APInt OnesMask = LHS.One & Mask;
    APInt ZerosMask = LHS.Zero & Mask;
    ଫେରନ୍ତୁ KnownBits(ZerosMask, OnesMask);
  }
  ଫେରନ୍ତୁ KnownBits(BitWidth);
}

KnownBits KnownBits::urem(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  ଯଦି (LHS.get_Max_ମୂଲ୍ୟ().ult(RHS.get_Min_ମୂଲ୍ୟ()))
    ଫେରନ୍ତୁ LHS;

  KnownBits Known = rem_Get_ତଳ_Bits(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_ମୂଲ୍ୟ(), RHS.get_Max_ମୂଲ୍ୟ() - 1);
  Known.Zero.set_ଉପର_Bits(MaxRes.ଗଣନା_Leading_Zeros());
  ଫେରନ୍ତୁ Known;
}

KnownBits KnownBits::srem(ଅଚଳ KnownBits &LHS, ଅଚଳ KnownBits &RHS) {
  KnownBits Known = rem_Get_ତଳ_Bits(LHS, RHS);
  ଯଦି (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt ତଳ_Bits = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    ଯଦି (LHS.isNonNegative() || ତଳ_Bits.isSubsetOf(LHS.Zero))
      Known.Zero |= ~ତଳ_Bits;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    ଯଦି (LHS.isNegative() && ତଳ_Bits.intersects(LHS.One))
      Known.One |= ~ତଳ_Bits;
    ଫେରନ୍ତୁ Known;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  ଯଦି (LHS.isNegative() && Known.isNonZero())
    Known.One.set_ଉପର_Bits(
        std::max(LHS.ଗଣନା_Min_Leading_Ones(), RHS.ଗଣନା_Min_Sign_Bits()));
  ଅନ୍ୟଥା ଯଦି (LHS.isNonNegative())
    Known.Zero.set_ଉପର_Bits(
        std::max(LHS.ଗଣନା_Min_Leading_Zeros(), RHS.ଗଣନା_Min_Sign_Bits()));
  ଫେରନ୍ତୁ Known;
}

KnownBits &KnownBits::operator&=(ଅଚଳ KnownBits &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  ଫେରନ୍ତୁ *ଏହି;
}

KnownBits &KnownBits::operator|=(ଅଚଳ KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  ଫେରନ୍ତୁ *ଏହି;
}

KnownBits &KnownBits::operator^=(ଅଚଳ KnownBits &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  ଫେରନ୍ତୁ *ଏହି;
}

KnownBits KnownBits::blsi() ଅଚଳ {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(Zero, APInt(BitWidth, 0));
  unsigned Max = ଗଣନା_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = ଗଣନା_Min_Trailing_Zeros();
  ଯଦି (Max == Min && Max < BitWidth)
    Known.One.setBit(Max);
  ଫେରନ୍ତୁ Known;
}

KnownBits KnownBits::blsmsk() ଅଚଳ {
  unsigned BitWidth = getBitWidth();
  KnownBits Known(BitWidth);
  unsigned Max = ଗଣନା_Max_Trailing_Zeros();
  Known.Zero.setBitsFrom(std::min(Max + 1, BitWidth));
  unsigned Min = ଗଣନା_Min_Trailing_Zeros();
  Known.One.set_ତଳ_Bits(std::min(Min + 1, BitWidth));
  ଫେରନ୍ତୁ Known;
}

ଶୂନ୍ୟ KnownBits::print(raw_ostream &OS) ଅଚଳ {
  unsigned BitWidth = getBitWidth();
  ପାଇଁ (unsigned I = 0; I < BitWidth; ++I) {
    unsigned N = BitWidth - I - 1;
    ଯଦି (Zero[N] && One[N])
      OS << "!";
    ଅନ୍ୟଥା ଯଦି (Zero[N])
      OS << "0";
    ଅନ୍ୟଥା ଯଦି (One[N])
      OS << "1";
    ଅନ୍ୟଥା
      OS << "?";
  }
}

#ଯଦି !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ଶୂନ୍ୟ KnownBits::dump() ଅଚଳ {
  print(dbgs());
  dbgs() << "\n";
}
#endif

unsigned llvm::SignBitsOps::rot(unsigned SrcSignBits, unsigned BitWidth,
                                std::optional<APInt> RotAmt,
                                ତାର୍କିକ Is_Rotate_ଡାହାଣ) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  ଯଦି (SrcSignBits == BitWidth)
    ଫେରନ୍ତୁ BitWidth;

  ଯଦି (!RotAmt)
    ଫେରନ୍ତୁ 1;

  unsigned Amt = RotAmt->urem(BitWidth);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  ଯଦି (Is_Rotate_ଡାହାଣ)
    Amt = (BitWidth - Amt) % BitWidth;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  ଯଦି (SrcSignBits > Amt + 1)
    ଫେରନ୍ତୁ SrcSignBits - Amt;
  ଫେରନ୍ତୁ 1;
}
