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

उपयुज्य नामाकाश llvm;

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::flipSignBit(ध्रुव ज्ञातम्_द्वयङ्काः &Val) {
  अचिह्नित SignBitPosition = Val.get_Bit_विस्तारः() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.समुच्चयः_Bit_Val(SignBitPosition, Val.One[SignBitPosition]);
  One.समुच्चयः_Bit_Val(SignBitPosition, Val.Zero[SignBitPosition]);
  प्रत्यागम ज्ञातम्_द्वयङ्काः(Zero, One);
}

स्थिर ज्ञातम्_द्वयङ्काः compute_For_सङ्कलय_Carry(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS,
                                    तार्किक CarryZero, तार्किक CarryOne) {

  APInt Possible_योग_Zero = LHS.get_Max_मानम्() + RHS.get_Max_मानम्() + !CarryZero;
  APInt Possible_योग_One = LHS.get_Min_मानम्() + RHS.get_Min_मानम्() + CarryOne;

  // Compute known bits of the carry.
  APInt Carry_ज्ञातम्_Zero = ~(Possible_योग_Zero ^ LHS.Zero ^ RHS.Zero);
  APInt Carry_ज्ञातम्_One = Possible_योग_One ^ LHS.One ^ RHS.One;

  // Compute set of known bits (where all three relevant bits are known).
  APInt LHSKnownUnion = LHS.Zero | LHS.One;
  APInt RHSKnownUnion = RHS.Zero | RHS.One;
  APInt Carry_ज्ञातम्_Union = std::move(Carry_ज्ञातम्_Zero) | Carry_ज्ञातम्_One;
  APInt ज्ञातम् = std::move(LHSKnownUnion) & RHSKnownUnion & Carry_ज्ञातम्_Union;

  // Compute known bits of the result.
  ज्ञातम्_द्वयङ्काः ज्ञातम्_बहिर्गमनम्;
  ज्ञातम्_बहिर्गमनम्.Zero = ~std::move(Possible_योग_Zero) & ज्ञातम्;
  ज्ञातम्_बहिर्गमनम्.One = std::move(Possible_योग_One) & ज्ञातम्;
  प्रत्यागम ज्ञातम्_बहिर्गमनम्;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::compute_For_सङ्कलय_Carry(
    ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS, ध्रुव ज्ञातम्_द्वयङ्काः &Carry) {
  प्रतिज्ञा(Carry.get_Bit_विस्तारः() == 1 && "Carry must be 1-bit");
  प्रत्यागम ::compute_For_सङ्कलय_Carry(
      LHS, RHS, Carry.Zero.get_Bool_मानम्(), Carry.One.get_Bool_मानम्());
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::compute_For_सङ्कलय_Sub(तार्किक सङ्कलय_2, तार्किक NSW, तार्किक NUW,
                                      ध्रुव ज्ञातम्_द्वयङ्काः &LHS,
                                      ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  अचिह्नित Bit_विस्तारः = LHS.get_Bit_विस्तारः();
  ज्ञातम्_द्वयङ्काः ज्ञातम्_बहिर्गमनम्(Bit_विस्तारः);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  यदि (LHS.isUnknown() && RHS.isUnknown())
    प्रत्यागम ज्ञातम्_बहिर्गमनम्;

  यदि (!LHS.isUnknown() && !RHS.isUnknown()) {
    यदि (सङ्कलय_2) {
      // Sum = LHS + RHS + 0
      ज्ञातम्_बहिर्गमनम् = ::compute_For_सङ्कलय_Carry(LHS, RHS, /*CarryZero=*/सत्य,
                                      /*CarryOne=*/असत्य);
    } अन्यथा {
      // Sum = LHS + ~RHS + 1
      ज्ञातम्_द्वयङ्काः NotRHS = RHS;
      std::swap(NotRHS.Zero, NotRHS.One);
      ज्ञातम्_बहिर्गमनम् = ::compute_For_सङ्कलय_Carry(LHS, NotRHS, /*CarryZero=*/असत्य,
                                      /*CarryOne=*/सत्य);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  यदि (NUW) {
    यदि (सङ्कलय_2) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_मानम्().uadd_sat(RHS.get_Min_मानम्());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      यदि (NSW) {
        अचिह्नित अङ्कः_द्वयङ्काः_2 = MinVal.trunc(Bit_विस्तारः - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        ज्ञातम्_बहिर्गमनम्.One.समुच्चयः_द्वयङ्काः(Bit_विस्तारः - 1 - अङ्कः_द्वयङ्काः_2, Bit_विस्तारः - 1);
      }
      ज्ञातम्_बहिर्गमनम्.One.समुच्चयः_उच्चः_द्वयङ्काः(MinVal.countl_one());
    } अन्यथा {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_मानम्().usub_sat(RHS.get_Min_मानम्());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      यदि (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        अचिह्नित अङ्कः_द्वयङ्काः_2 = MaxVal.trunc(Bit_विस्तारः - 1).countl_zero();
        ज्ञातम्_बहिर्गमनम्.Zero.समुच्चयः_द्वयङ्काः(Bit_विस्तारः - 1 - अङ्कः_द्वयङ्काः_2, Bit_विस्तारः - 1);
      }
      ज्ञातम्_बहिर्गमनम्.Zero.समुच्चयः_उच्चः_द्वयङ्काः(MaxVal.countl_zero());
    }
  }

  यदि (NSW) {
    APInt MinVal;
    APInt MaxVal;
    यदि (सङ्कलय_2) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_मानम्().sadd_sat(RHS.get_Signed_Min_मानम्());
      MaxVal = LHS.get_Signed_Max_मानम्().sadd_sat(RHS.get_Signed_Max_मानम्());
    } अन्यथा {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_मानम्().ssub_sat(RHS.get_Signed_Max_मानम्());
      MaxVal = LHS.get_Signed_Max_मानम्().ssub_sat(RHS.get_Signed_Min_मानम्());
    }
    यदि (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      अचिह्नित अङ्कः_द्वयङ्काः_2 = MinVal.trunc(Bit_विस्तारः - 1).countl_one();
      ज्ञातम्_बहिर्गमनम्.One.समुच्चयः_द्वयङ्काः(Bit_विस्तारः - 1 - अङ्कः_द्वयङ्काः_2, Bit_विस्तारः - 1);
      ज्ञातम्_बहिर्गमनम्.Zero.समुच्चयः_Sign_Bit();
    }
    यदि (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      अचिह्नित अङ्कः_द्वयङ्काः_2 = MaxVal.trunc(Bit_विस्तारः - 1).countl_zero();
      ज्ञातम्_बहिर्गमनम्.Zero.समुच्चयः_द्वयङ्काः(Bit_विस्तारः - 1 - अङ्कः_द्वयङ्काः_2, Bit_विस्तारः - 1);
      ज्ञातम्_बहिर्गमनम्.One.समुच्चयः_Sign_Bit();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  यदि (ज्ञातम्_बहिर्गमनम्.hasConflict())
    ज्ञातम्_बहिर्गमनम्.समुच्चयः_All_Zero();
  प्रत्यागम ज्ञातम्_बहिर्गमनम्;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::computeForSubBorrow(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ज्ञातम्_द्वयङ्काः RHS,
                                         ध्रुव ज्ञातम्_द्वयङ्काः &Borrow) {
  प्रतिज्ञा(Borrow.get_Bit_विस्तारः() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::swap(RHS.Zero, RHS.One);
  प्रत्यागम ::compute_For_सङ्कलय_Carry(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_मानम्(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_मानम्());
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::truncSSat(अचिह्नित Bit_विस्तारः) ध्रुव {
  अचिह्नित निवेशः_द्वयङ्काः = get_Bit_विस्तारः();
  APInt Min_अन्तर्_Range = APInt::get_Signed_Min_मानम्(Bit_विस्तारः).sext(निवेशः_द्वयङ्काः);
  APInt Max_अन्तर्_Range = APInt::get_Signed_Max_मानम्(Bit_विस्तारः).sext(निवेशः_द्वयङ्काः);
  APInt निवेशः_Min = get_Signed_Min_मानम्();
  APInt निवेशः_Max = get_Signed_Max_मानम्();
  ज्ञातम्_द्वयङ्काः ज्ञातम्(Bit_विस्तारः);

  // Case 1: All values fit - just truncate
  यदि (निवेशः_Min.sge(Min_अन्तर्_Range) && निवेशः_Max.sle(Max_अन्तर्_Range)) {
    ज्ञातम् = trunc(Bit_विस्तारः);
  }
  // Case 2: All saturate to min
  अन्यथा यदि (निवेशः_Max.slt(Min_अन्तर्_Range)) {
    ज्ञातम् = ज्ञातम्_द्वयङ्काः::makeConstant(APInt::get_Signed_Min_मानम्(Bit_विस्तारः));
  }
  // Case 3: All saturate to max
  अन्यथा यदि (निवेशः_Min.sgt(Max_अन्तर्_Range)) {
    ज्ञातम् = ज्ञातम्_द्वयङ्काः::makeConstant(APInt::get_Signed_Max_मानम्(Bit_विस्तारः));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  अन्यथा यदि (निवेशः_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    ज्ञातम्.Zero = APInt(Bit_विस्तारः, 0);
    ज्ञातम्.Zero.समुच्चयः_Bit(Bit_विस्तारः - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    ज्ञातम्.One = One.trunc(Bit_विस्तारः);
    ज्ञातम्.One.clearBit(Bit_विस्तारः - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  अन्यथा यदि (निवेशः_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    ज्ञातम्.Zero = Zero.trunc(Bit_विस्तारः);
    ज्ञातम्.Zero.clearBit(Bit_विस्तारः - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    ज्ञातम्.One = APInt(Bit_विस्तारः, 0);
    ज्ञातम्.One.समुच्चयः_Bit(Bit_विस्तारः - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  अन्यथा {
    // Output: min saturation, truncated, or max saturation
    APInt निवेशः_Zero_अधस्तनम् = Zero.trunc(Bit_विस्तारः);
    APInt निवेशः_One_अधस्तनम् = One.trunc(Bit_विस्तारः);
    APInt MinSat = APInt::get_Signed_Min_मानम्(Bit_विस्तारः);
    APInt MaxSat = APInt::get_Signed_Max_मानम्(Bit_विस्तारः);
    ज्ञातम्_द्वयङ्काः MinSatKB = ज्ञातम्_द्वयङ्काः::makeConstant(MinSat);
    ज्ञातम्_द्वयङ्काः MaxSatKB = ज्ञातम्_द्वयङ्काः::makeConstant(MaxSat);
    यदि (निवेशः_Max.sle(Max_अन्तर्_Range)) {
      // Positive values fit, only negatives saturate to min
      ज्ञातम्.Zero = निवेशः_Zero_अधस्तनम् & MinSatKB.Zero;
      ज्ञातम्.One = निवेशः_One_अधस्तनम् & MinSatKB.One;
    } अन्यथा यदि (निवेशः_Min.sge(Min_अन्तर्_Range)) {
      // Negative values fit, only positives saturate to max
      ज्ञातम्.Zero = निवेशः_Zero_अधस्तनम् & MaxSatKB.Zero;
      ज्ञातम्.One = निवेशः_One_अधस्तनम् & MaxSatKB.One;
    } अन्यथा {
      // Both positive and negative values might saturate
      ज्ञातम्.Zero = निवेशः_Zero_अधस्तनम् & MinSatKB.Zero & MaxSatKB.Zero;
      ज्ञातम्.One = निवेशः_One_अधस्तनम् & MinSatKB.One & MaxSatKB.One;
    }
  }
  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::truncSSatU(अचिह्नित Bit_विस्तारः) ध्रुव {
  अचिह्नित निवेशः_द्वयङ्काः = get_Bit_विस्तारः();
  APInt Max_अन्तर्_Range = APInt::getAllOnes(Bit_विस्तारः).zext(निवेशः_द्वयङ्काः);
  APInt निवेशः_Min = get_Signed_Min_मानम्();
  APInt निवेशः_Max = get_Signed_Max_मानम्();
  ज्ञातम्_द्वयङ्काः ज्ञातम्(Bit_विस्तारः);

  यदि (isNegative()) {
    ज्ञातम्.समुच्चयः_All_Zero();
  } अन्यथा यदि (निवेशः_Min.isNonNegative() && निवेशः_Max.ule(Max_अन्तर्_Range)) {
    ज्ञातम् = trunc(Bit_विस्तारः);
  } अन्यथा यदि (निवेशः_Min.isNonNegative() && निवेशः_Min.ugt(Max_अन्तर्_Range)) {
    ज्ञातम्.समुच्चयः_All_Ones();
  } अन्यथा यदि (निवेशः_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    ज्ञातम्.Zero = APInt(Bit_विस्तारः, 0);
    // Common one bits: bits that are 1 in input stay 1
    ज्ञातम्.One = One.trunc(Bit_विस्तारः);
  } अन्यथा {
    // Mixed: sign bit unknown
    यदि (निवेशः_Max.ule(Max_अन्तर्_Range)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      ज्ञातम्.Zero = Zero.trunc(Bit_विस्तारः);
      ज्ञातम्.One = APInt(Bit_विस्तारः, 0);
    } अन्यथा {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      ज्ञातम्.Zero = APInt(Bit_विस्तारः, 0);
      ज्ञातम्.One = APInt(Bit_विस्तारः, 0);
    }
  }
  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::truncUSat(अचिह्नित Bit_विस्तारः) ध्रुव {
  अचिह्नित निवेशः_द्वयङ्काः = get_Bit_विस्तारः();
  APInt Max_अन्तर्_Range = APInt::get_नीचः_द्वयङ्काः_समुच्चयः(निवेशः_द्वयङ्काः, Bit_विस्तारः);
  APInt निवेशः_Max = get_Max_मानम्();
  APInt निवेशः_Min = get_Min_मानम्();
  ज्ञातम्_द्वयङ्काः ज्ञातम्(Bit_विस्तारः);

  यदि (निवेशः_Max.ule(Max_अन्तर्_Range)) {
    ज्ञातम् = trunc(Bit_विस्तारः);
  } अन्यथा यदि (निवेशः_Min.ugt(Max_अन्तर्_Range)) {
    ज्ञातम्.समुच्चयः_All_Ones();
  } अन्यथा {
    ज्ञातम्.resetAll();
    ज्ञातम्.One = One.trunc(Bit_विस्तारः);
  }
  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::sext_अन्तर्_Reg(अचिह्नित Src_Bit_विस्तारः) ध्रुव {
  अचिह्नित Bit_विस्तारः = get_Bit_विस्तारः();
  प्रतिज्ञा(0 < Src_Bit_विस्तारः && Src_Bit_विस्तारः <= Bit_विस्तारः &&
         "Illegal sext-in-register");

  यदि (Src_Bit_विस्तारः == Bit_विस्तारः)
    प्रत्यागम *एतत्;

  अचिह्नित Ext_द्वयङ्काः = Bit_विस्तारः - Src_Bit_विस्तारः;
  ज्ञातम्_द्वयङ्काः परिणाम;
  परिणाम.One = One << Ext_द्वयङ्काः;
  परिणाम.Zero = Zero << Ext_द्वयङ्काः;
  परिणाम.One.ashr_अन्तर्_Place(Ext_द्वयङ्काः);
  परिणाम.Zero.ashr_अन्तर्_Place(Ext_द्वयङ्काः);
  प्रत्यागम परिणाम;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::makeGE(ध्रुव APInt &Val) ध्रुव {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  अचिह्नित N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_नीचः_द्वयङ्काः(get_Bit_विस्तारः() - N);
  प्रत्यागम ज्ञातम्_द्वयङ्काः(Zero, One | MaskedVal);
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::umax(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  यदि (LHS.get_Min_मानम्().uge(RHS.get_Max_मानम्()))
    प्रत्यागम LHS;
  यदि (RHS.get_Min_मानम्().uge(LHS.get_Max_मानम्()))
    प्रत्यागम RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  ज्ञातम्_द्वयङ्काः L = LHS.makeGE(RHS.get_Min_मानम्());
  ज्ञातम्_द्वयङ्काः R = RHS.makeGE(LHS.get_Min_मानम्());
  प्रत्यागम L.intersectWith(R);
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::umin(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  स्वचालित Flip = [](ध्रुव ज्ञातम्_द्वयङ्काः &Val) { प्रत्यागम ज्ञातम्_द्वयङ्काः(Val.One, Val.Zero); };
  प्रत्यागम Flip(umax(Flip(LHS), Flip(RHS)));
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::smax(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  प्रत्यागम flipSignBit(umax(flipSignBit(LHS), flipSignBit(RHS)));
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::smin(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  स्वचालित Flip = [](ध्रुव ज्ञातम्_द्वयङ्काः &Val) {
    अचिह्नित SignBitPosition = Val.get_Bit_विस्तारः() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.समुच्चयः_Bit_Val(SignBitPosition, Val.Zero[SignBitPosition]);
    One.समुच्चयः_Bit_Val(SignBitPosition, Val.One[SignBitPosition]);
    प्रत्यागम ज्ञातम्_द्वयङ्काः(Zero, One);
  };
  प्रत्यागम Flip(umax(Flip(LHS), Flip(RHS)));
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::abdu(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  यदि (LHS.get_Min_मानम्().uge(RHS.get_Max_मानम्()))
    प्रत्यागम compute_For_सङ्कलय_Sub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/असत्य, LHS,
                            RHS);
  यदि (RHS.get_Min_मानम्().uge(LHS.get_Max_मानम्()))
    प्रत्यागम compute_For_सङ्कलय_Sub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/असत्य, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  ज्ञातम्_द्वयङ्काः Diff0 =
      compute_For_सङ्कलय_Sub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/सत्य, LHS, RHS);
  ज्ञातम्_द्वयङ्काः Diff1 =
      compute_For_सङ्कलय_Sub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/सत्य, RHS, LHS);
  प्रत्यागम Diff0.intersectWith(Diff1);
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::abds(ज्ञातम्_द्वयङ्काः LHS, ज्ञातम्_द्वयङ्काः RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  यदि (LHS.get_Signed_Min_मानम्().sge(RHS.get_Signed_Max_मानम्()))
    प्रत्यागम compute_For_सङ्कलय_Sub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/असत्य, LHS,
                            RHS);
  यदि (RHS.get_Signed_Min_मानम्().sge(LHS.get_Signed_Max_मानम्()))
    प्रत्यागम compute_For_सङ्कलय_Sub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/असत्य, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  अचिह्नित SignBitPosition = LHS.get_Bit_विस्तारः() - 1;
  प्रति (स्वचालित Arg : {&LHS, &RHS}) {
    तार्किक Tmp = Arg->Zero[SignBitPosition];
    Arg->Zero.समुच्चयः_Bit_Val(SignBitPosition, Arg->One[SignBitPosition]);
    Arg->One.समुच्चयः_Bit_Val(SignBitPosition, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  ज्ञातम्_द्वयङ्काः Diff0 =
      compute_For_सङ्कलय_Sub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/सत्य, LHS, RHS);
  ज्ञातम्_द्वयङ्काः Diff1 =
      compute_For_सङ्कलय_Sub(/*Add=*/असत्य, /*NSW=*/असत्य, /*NUW=*/सत्य, RHS, LHS);
  प्रत्यागम Diff0.intersectWith(Diff1);
}

स्थिर अचिह्नित get_Max_सरणम्_Amount(ध्रुव APInt &Max_मानम्, अचिह्नित Bit_विस्तारः) {
  यदि (isPowerOf2_32(Bit_विस्तारः))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    प्रत्यागम Max_मानम्.extract_द्वयङ्काः_As_ZExt_मानम्(
        std::min(Log2_32(Bit_विस्तारः), Max_मानम्.get_Bit_विस्तारः()), 0);
  // This is only an approximate upper bound.
  प्रत्यागम Max_मानम्.get_Limited_मानम्(Bit_विस्तारः - 1);
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::shl(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS, तार्किक NUW,
                         तार्किक NSW, तार्किक ShAmtNonZero) {
  अचिह्नित Bit_विस्तारः = LHS.get_Bit_विस्तारः();
  स्वचालित सरणम्_By_Const = [&](ध्रुव ज्ञातम्_द्वयङ्काः &LHS, अचिह्नित सरणम्_Amt_2) {
    ज्ञातम्_द्वयङ्काः ज्ञातम्;
    तार्किक Shifted_बहिर्गमनम्_Zero, Shifted_बहिर्गमनम्_One;
    ज्ञातम्.Zero = LHS.Zero.ushl_ov(सरणम्_Amt_2, Shifted_बहिर्गमनम्_Zero);
    ज्ञातम्.Zero.समुच्चयः_नीचः_द्वयङ्काः(सरणम्_Amt_2);
    ज्ञातम्.One = LHS.One.ushl_ov(सरणम्_Amt_2, Shifted_बहिर्गमनम्_One);

    // All cases returning poison have been handled by MaxShiftAmount already.
    यदि (NSW) {
      यदि (NUW && सरणम्_Amt_2 != 0)
        // NUW means we can assume anything shifted out was a zero.
        Shifted_बहिर्गमनम्_Zero = सत्य;

      यदि (Shifted_बहिर्गमनम्_Zero)
        ज्ञातम्.makeNonNegative();
      अन्यथा यदि (Shifted_बहिर्गमनम्_One)
        ज्ञातम्.makeNegative();
    }
    प्रत्यागम ज्ञातम्;
  };

  // Fast path for a common case when LHS is completely unknown.
  ज्ञातम्_द्वयङ्काः ज्ञातम्(Bit_विस्तारः);
  अचिह्नित Min_सरणम्_Amount = RHS.get_Min_मानम्().get_Limited_मानम्(Bit_विस्तारः);
  यदि (Min_सरणम्_Amount == 0 && ShAmtNonZero)
    Min_सरणम्_Amount = 1;
  यदि (LHS.isUnknown()) {
    ज्ञातम्.Zero.समुच्चयः_नीचः_द्वयङ्काः(Min_सरणम्_Amount);
    यदि (NUW && NSW && Min_सरणम्_Amount != 0)
      ज्ञातम्.makeNonNegative();
    प्रत्यागम ज्ञातम्;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_मानम् = RHS.get_Max_मानम्();
  अचिह्नित Max_सरणम्_Amount = get_Max_सरणम्_Amount(Max_मानम्, Bit_विस्तारः);
  यदि (NUW && NSW)
    Max_सरणम्_Amount = std::min(Max_सरणम्_Amount, LHS.गणनम्_Max_Leading_Zeros() - 1);
  यदि (NUW)
    Max_सरणम्_Amount = std::min(Max_सरणम्_Amount, LHS.गणनम्_Max_Leading_Zeros());
  यदि (NSW)
    Max_सरणम्_Amount = std::min(
        Max_सरणम्_Amount,
        std::max(LHS.गणनम्_Max_Leading_Zeros(), LHS.गणनम्_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  यदि (Min_सरणम्_Amount == 0 && Max_सरणम्_Amount == Bit_विस्तारः - 1 &&
      isPowerOf2_32(Bit_विस्तारः)) {
    ज्ञातम्.Zero.समुच्चयः_नीचः_द्वयङ्काः(LHS.गणनम्_Min_Trailing_Zeros());
    यदि (LHS.isAllOnes())
      ज्ञातम्.One.समुच्चयः_Sign_Bit();
    यदि (NSW) {
      यदि (LHS.isNonNegative())
        ज्ञातम्.makeNonNegative();
      यदि (LHS.isNegative())
        ज्ञातम्.makeNegative();
    }
    प्रत्यागम ज्ञातम्;
  }

  // Find the common bits from all possible shifts.
  अचिह्नित सरणम्_Amt_Zero_आवरणम् = RHS.Zero.zextOrTrunc(32).get_ZExt_मानम्();
  अचिह्नित सरणम्_Amt_One_आवरणम् = RHS.One.zextOrTrunc(32).get_ZExt_मानम्();
  ज्ञातम्.समुच्चयः_All_Conflict();
  प्रति (अचिह्नित सरणम्_Amt_2 = Min_सरणम्_Amount; सरणम्_Amt_2 <= Max_सरणम्_Amount;
       ++सरणम्_Amt_2) {
    // Skip if the shift amount is impossible.
    यदि ((सरणम्_Amt_Zero_आवरणम् & सरणम्_Amt_2) != 0 ||
        (सरणम्_Amt_One_आवरणम् | सरणम्_Amt_2) != सरणम्_Amt_2)
      अनुवर्त;
    ज्ञातम् = ज्ञातम्.intersectWith(सरणम्_By_Const(LHS, सरणम्_Amt_2));
    यदि (ज्ञातम्.isUnknown())
      भङ्ग;
  }

  // All shift amounts may result in poison.
  यदि (ज्ञातम्.hasConflict())
    ज्ञातम्.समुच्चयः_All_Zero();
  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::lshr(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS,
                          तार्किक ShAmtNonZero, तार्किक Exact) {
  अचिह्नित Bit_विस्तारः = LHS.get_Bit_विस्तारः();
  स्वचालित सरणम्_By_Const = [&](ध्रुव ज्ञातम्_द्वयङ्काः &LHS, अचिह्नित सरणम्_Amt_2) {
    ज्ञातम्_द्वयङ्काः ज्ञातम् = LHS;
    ज्ञातम् >>= सरणम्_Amt_2;
    // High bits are known zero.
    ज्ञातम्.Zero.समुच्चयः_उच्चः_द्वयङ्काः(सरणम्_Amt_2);
    प्रत्यागम ज्ञातम्;
  };

  // Fast path for a common case when LHS is completely unknown.
  ज्ञातम्_द्वयङ्काः ज्ञातम्(Bit_विस्तारः);
  अचिह्नित Min_सरणम्_Amount = RHS.get_Min_मानम्().get_Limited_मानम्(Bit_विस्तारः);
  यदि (Min_सरणम्_Amount == 0 && ShAmtNonZero)
    Min_सरणम्_Amount = 1;
  यदि (LHS.isUnknown()) {
    ज्ञातम्.Zero.समुच्चयः_उच्चः_द्वयङ्काः(Min_सरणम्_Amount);
    प्रत्यागम ज्ञातम्;
  }

  // Find the common bits from all possible shifts.
  APInt Max_मानम् = RHS.get_Max_मानम्();
  अचिह्नित Max_सरणम्_Amount = get_Max_सरणम्_Amount(Max_मानम्, Bit_विस्तारः);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  यदि (Exact) {
    अचिह्नित FirstOne = LHS.गणनम्_Max_Trailing_Zeros();
    यदि (FirstOne < Min_सरणम्_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      ज्ञातम्.समुच्चयः_All_Zero();
      प्रत्यागम ज्ञातम्;
    }
    Max_सरणम्_Amount = std::min(Max_सरणम्_Amount, FirstOne);
  }

  अचिह्नित सरणम्_Amt_Zero_आवरणम् = RHS.Zero.zextOrTrunc(32).get_ZExt_मानम्();
  अचिह्नित सरणम्_Amt_One_आवरणम् = RHS.One.zextOrTrunc(32).get_ZExt_मानम्();
  ज्ञातम्.समुच्चयः_All_Conflict();
  प्रति (अचिह्नित सरणम्_Amt_2 = Min_सरणम्_Amount; सरणम्_Amt_2 <= Max_सरणम्_Amount;
       ++सरणम्_Amt_2) {
    // Skip if the shift amount is impossible.
    यदि ((सरणम्_Amt_Zero_आवरणम् & सरणम्_Amt_2) != 0 ||
        (सरणम्_Amt_One_आवरणम् | सरणम्_Amt_2) != सरणम्_Amt_2)
      अनुवर्त;
    ज्ञातम् = ज्ञातम्.intersectWith(सरणम्_By_Const(LHS, सरणम्_Amt_2));
    यदि (ज्ञातम्.isUnknown())
      भङ्ग;
  }

  // All shift amounts may result in poison.
  यदि (ज्ञातम्.hasConflict())
    ज्ञातम्.समुच्चयः_All_Zero();
  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::ashr(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS,
                          तार्किक ShAmtNonZero, तार्किक Exact) {
  अचिह्नित Bit_विस्तारः = LHS.get_Bit_विस्तारः();
  स्वचालित सरणम्_By_Const = [&](ध्रुव ज्ञातम्_द्वयङ्काः &LHS, अचिह्नित सरणम्_Amt_2) {
    ज्ञातम्_द्वयङ्काः ज्ञातम् = LHS;
    ज्ञातम्.Zero.ashr_अन्तर्_Place(सरणम्_Amt_2);
    ज्ञातम्.One.ashr_अन्तर्_Place(सरणम्_Amt_2);
    प्रत्यागम ज्ञातम्;
  };

  // Fast path for a common case when LHS is completely unknown.
  ज्ञातम्_द्वयङ्काः ज्ञातम्(Bit_विस्तारः);
  अचिह्नित Min_सरणम्_Amount = RHS.get_Min_मानम्().get_Limited_मानम्(Bit_विस्तारः);
  यदि (Min_सरणम्_Amount == 0 && ShAmtNonZero)
    Min_सरणम्_Amount = 1;
  यदि (LHS.isUnknown()) {
    यदि (Min_सरणम्_Amount == Bit_विस्तारः) {
      // Always poison. Return zero because we don't like returning conflict.
      ज्ञातम्.समुच्चयः_All_Zero();
      प्रत्यागम ज्ञातम्;
    }
    प्रत्यागम ज्ञातम्;
  }

  // Find the common bits from all possible shifts.
  APInt Max_मानम् = RHS.get_Max_मानम्();
  अचिह्नित Max_सरणम्_Amount = get_Max_सरणम्_Amount(Max_मानम्, Bit_विस्तारः);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  यदि (Exact) {
    अचिह्नित FirstOne = LHS.गणनम्_Max_Trailing_Zeros();
    यदि (FirstOne < Min_सरणम्_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      ज्ञातम्.समुच्चयः_All_Zero();
      प्रत्यागम ज्ञातम्;
    }
    Max_सरणम्_Amount = std::min(Max_सरणम्_Amount, FirstOne);
  }

  अचिह्नित सरणम्_Amt_Zero_आवरणम् = RHS.Zero.zextOrTrunc(32).get_ZExt_मानम्();
  अचिह्नित सरणम्_Amt_One_आवरणम् = RHS.One.zextOrTrunc(32).get_ZExt_मानम्();
  ज्ञातम्.समुच्चयः_All_Conflict();
  प्रति (अचिह्नित सरणम्_Amt_2 = Min_सरणम्_Amount; सरणम्_Amt_2 <= Max_सरणम्_Amount;
      ++सरणम्_Amt_2) {
    // Skip if the shift amount is impossible.
    यदि ((सरणम्_Amt_Zero_आवरणम् & सरणम्_Amt_2) != 0 ||
        (सरणम्_Amt_One_आवरणम् | सरणम्_Amt_2) != सरणम्_Amt_2)
      अनुवर्त;
    ज्ञातम् = ज्ञातम्.intersectWith(सरणम्_By_Const(LHS, सरणम्_Amt_2));
    यदि (ज्ञातम्.isUnknown())
      भङ्ग;
  }

  // All shift amounts may result in poison.
  यदि (ज्ञातम्.hasConflict())
    ज्ञातम्.समुच्चयः_All_Zero();
  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::fshl(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS,
                          ध्रुव APInt &Amt) {
  प्रत्यागम ज्ञातम्_द्वयङ्काः(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::fshr(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS,
                          ध्रुव APInt &Amt) {
  प्रत्यागम ज्ञातम्_द्वयङ्काः(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::clmul(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  ज्ञातम्_द्वयङ्काः Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_मानम्(), RHS.get_Min_मानम्()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  स्वचालित ClMulOr = [](ध्रुव APInt &LHS, ध्रुव APInt &RHS) {
    अचिह्नित BW = LHS.get_Bit_विस्तारः();
    प्रतिज्ञा(BW == RHS.get_Bit_विस्तारः() && "Operand mismatch");
    APInt परिणाम(BW, 0);
    प्रति (अचिह्नित I :
         seq(std::min(RHS.get_सक्रिय_द्वयङ्काः(), BW - LHS.countr_zero())))
      यदि (RHS[I])
        परिणाम |= LHS << I;
    प्रत्यागम परिणाम;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt ज्ञातम् = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= ज्ञातम्;
  Res.One &= ज्ञातम्;

  प्रत्यागम Res;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::pext(ध्रुव ज्ञातम्_द्वयङ्काः &Val, ध्रुव ज्ञातम्_द्वयङ्काः &आवरणम्) {
  अचिह्नित Bit_विस्तारः = Val.get_Bit_विस्तारः();
  ज्ञातम्_द्वयङ्काः Res(Bit_विस्तारः);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.समुच्चयः_All_Conflict();

  // For each source position I where Mask[I] could be 1, the output position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible output positions widens.
  अचिह्नित M0 = 0, M1 = 0;
  प्रति (अचिह्नित I = 0; I < Bit_विस्तारः; ++I) {
    यदि (!आवरणम्.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      यदि (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clear_द्वयङ्काः(M0, M1 + 1);
      यदि (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clear_द्वयङ्काः(M0, M1 + 1);
    }
    यदि (आवरणम्.One[I])
      ++M0, ++M1;
    अन्यथा यदि (!आवरणम्.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clear_द्वयङ्काः(M0, Bit_विस्तारः);
  प्रत्यागम Res;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::pdep(ध्रुव ज्ञातम्_द्वयङ्काः &Val, ध्रुव ज्ञातम्_द्वयङ्काः &आवरणम्) {
  अचिह्नित Bit_विस्तारः = Val.get_Bit_विस्तारः();
  ज्ञातम्_द्वयङ्काः Res(Bit_विस्तारः);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.समुच्चयः_All_Conflict();

  // For each output position I where Mask[I] could be 1, the source position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible source positions widens.
  अचिह्नित M0 = 0, M1 = 0;
  प्रति (अचिह्नित I = 0; I < Bit_विस्तारः; ++I) {
    यदि (!आवरणम्.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clearBit(I);
    यदि (!आवरणम्.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::get_द्वयङ्काः_समुच्चयः(Bit_विस्तारः, M0, M1 + 1);
      यदि (!Range.is_Subset_सम्बन्धि(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clearBit(I);
      यदि (!Range.is_Subset_सम्बन्धि(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clearBit(I);
    }
    यदि (आवरणम्.One[I])
      ++M0, ++M1;
    अन्यथा यदि (!आवरणम्.Zero[I])
      ++M1;
  }
  प्रत्यागम Res;
}

std::optional<तार्किक> ज्ञातम्_द्वयङ्काः::eq(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  यदि (LHS.isConstant() && RHS.isConstant())
    प्रत्यागम LHS.getConstant() == RHS.getConstant();
  यदि (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    प्रत्यागम असत्य;
  प्रत्यागम std::nullopt;
}

std::optional<तार्किक> ज्ञातम्_द्वयङ्काः::ne(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  यदि (std::optional<तार्किक> ज्ञातम्_EQ = eq(LHS, RHS))
    प्रत्यागम !*ज्ञातम्_EQ;
  प्रत्यागम std::nullopt;
}

std::optional<तार्किक> ज्ञातम्_द्वयङ्काः::ugt(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  यदि (LHS.get_Max_मानम्().ule(RHS.get_Min_मानम्()))
    प्रत्यागम असत्य;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  यदि (LHS.get_Min_मानम्().ugt(RHS.get_Max_मानम्()))
    प्रत्यागम सत्य;
  प्रत्यागम std::nullopt;
}

std::optional<तार्किक> ज्ञातम्_द्वयङ्काः::uge(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  यदि (std::optional<तार्किक> IsUGT = ugt(RHS, LHS))
    प्रत्यागम !*IsUGT;
  प्रत्यागम std::nullopt;
}

std::optional<तार्किक> ज्ञातम्_द्वयङ्काः::ult(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  प्रत्यागम ugt(RHS, LHS);
}

std::optional<तार्किक> ज्ञातम्_द्वयङ्काः::ule(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  प्रत्यागम uge(RHS, LHS);
}

std::optional<तार्किक> ज्ञातम्_द्वयङ्काः::sgt(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  यदि (LHS.get_Signed_Max_मानम्().sle(RHS.get_Signed_Min_मानम्()))
    प्रत्यागम असत्य;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  यदि (LHS.get_Signed_Min_मानम्().sgt(RHS.get_Signed_Max_मानम्()))
    प्रत्यागम सत्य;
  प्रत्यागम std::nullopt;
}

std::optional<तार्किक> ज्ञातम्_द्वयङ्काः::sge(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  यदि (std::optional<तार्किक> ज्ञातम्_SGT = sgt(RHS, LHS))
    प्रत्यागम !*ज्ञातम्_SGT;
  प्रत्यागम std::nullopt;
}

std::optional<तार्किक> ज्ञातम्_द्वयङ्काः::slt(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  प्रत्यागम sgt(RHS, LHS);
}

std::optional<तार्किक> ज्ञातम्_द्वयङ्काः::sle(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  प्रत्यागम sge(RHS, LHS);
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::निरपेक्षम्(तार्किक IntMinIsPoison) ध्रुव {
  // If the source's MSB is zero then we know the rest of the bits already.
  यदि (isNonNegative())
    प्रत्यागम *एतत्;

  // Absolute value preserves trailing zero count.
  ज्ञातम्_द्वयङ्काः ज्ञातम्_Abs(get_Bit_विस्तारः());

  // If the input is negative, then abs(x) == -x.
  यदि (isNegative()) {
    ज्ञातम्_द्वयङ्काः Tmp = *एतत्;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    यदि (IntMinIsPoison && (Zero.popcount() + 2) == get_Bit_विस्तारः())
      Tmp.One.समुच्चयः_Bit(गणनम्_Min_Trailing_Zeros());

    ज्ञातम्_Abs = compute_For_सङ्कलय_Sub(
        /*सङ्कलय*/ असत्य, IntMinIsPoison, /*NUW=*/असत्य,
        ज्ञातम्_द्वयङ्काः::makeConstant(APInt(get_Bit_विस्तारः(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    यदि (IntMinIsPoison && Tmp.गणनम्_Min_Population() == 1 &&
        Tmp.गणनम्_Max_Population() != 1) {
      Tmp.One.clearSignBit();
      Tmp.Zero.समुच्चयः_Sign_Bit();
      ज्ञातम्_Abs.One.समुच्चयः_द्वयङ्काः(get_Bit_विस्तारः() - Tmp.गणनम्_Min_Leading_Zeros(),
                           get_Bit_विस्तारः() - 1);
    }

  } अन्यथा {
    अचिह्नित MaxTZ = गणनम्_Max_Trailing_Zeros();
    अचिह्नित MinTZ = गणनम्_Min_Trailing_Zeros();

    ज्ञातम्_Abs.Zero.समुच्चयः_नीचः_द्वयङ्काः(MinTZ);
    // If we know the lowest set 1, then preserve it.
    यदि (MaxTZ == MinTZ && MaxTZ < get_Bit_विस्तारः())
      ज्ञातम्_Abs.One.समुच्चयः_Bit(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    यदि (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_मानम्())) {
      ज्ञातम्_Abs.One.clearSignBit();
      ज्ञातम्_Abs.Zero.समुच्चयः_Sign_Bit();
    }
  }

  प्रत्यागम ज्ञातम्_Abs;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::reduce_सङ्कलय(अचिह्नित अङ्कः_Elts) ध्रुव {
  यदि (अङ्कः_Elts == 0)
    प्रत्यागम ज्ञातम्_द्वयङ्काः(get_Bit_विस्तारः());

  अचिह्नित Bit_विस्तारः = get_Bit_विस्तारः();
  ज्ञातम्_द्वयङ्काः परिणाम(Bit_विस्तारः);

  यदि (isConstant())
    // If all elements are the same constant, we can simply compute it
    प्रत्यागम ज्ञातम्_द्वयङ्काः::makeConstant(अङ्कः_Elts * getConstant());

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
  अचिह्नित नष्टम्_द्वयङ्काः = Log2_32_Ceil(अङ्कः_Elts);

  यदि (isNonNegative()) {
    अचिह्नित LeadingZeros = गणनम्_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > नष्टम्_द्वयङ्काः ? LeadingZeros - नष्टम्_द्वयङ्काः : 0;
    परिणाम.Zero.समुच्चयः_उच्चः_द्वयङ्काः(LeadingZeros);
  } अन्यथा यदि (isNegative()) {
    अचिह्नित LeadingOnes = गणनम्_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > नष्टम्_द्वयङ्काः ? LeadingOnes - नष्टम्_द्वयङ्काः : 0;
    परिणाम.One.समुच्चयः_उच्चः_द्वयङ्काः(LeadingOnes);
  }

  प्रत्यागम परिणाम;
}

स्थिर ज्ञातम्_द्वयङ्काः compute_For_Sat_सङ्कलय_Sub(तार्किक सङ्कलय_2, तार्किक Signed,
                                     ध्रुव ज्ञातम्_द्वयङ्काः &LHS,
                                     ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  अचिह्नित Bit_विस्तारः = LHS.get_Bit_विस्तारः();

  std::optional<तार्किक> अतिप्रवाहः;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  तार्किक MayNegClamp = सत्य;
  तार्किक MayPosClamp = सत्य;
  यदि (Signed) {
    // Easy cases we can rule out any overflow.
    यदि (सङ्कलय_2 && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      अतिप्रवाहः = असत्य;
    अन्यथा यदि (!सङ्कलय_2 && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      अतिप्रवाहः = असत्य;
    अन्यथा {
      // Check if we may overflow. If we can't rule out overflow then check if
      // we can rule out a direction at least.
      ज्ञातम्_द्वयङ्काः UnsignedLHS = LHS;
      ज्ञातम्_द्वयङ्काः UnsignedRHS = RHS;
      // Get version of LHS/RHS with clearer signbit. This allows us to detect
      // how the addition/subtraction might overflow into the signbit. Then
      // using the actual known signbits of LHS/RHS, we can figure out which
      // overflows are/aren't possible.
      UnsignedLHS.One.clearSignBit();
      UnsignedLHS.Zero.समुच्चयः_Sign_Bit();
      UnsignedRHS.One.clearSignBit();
      UnsignedRHS.Zero.समुच्चयः_Sign_Bit();
      ज्ञातम्_द्वयङ्काः Res =
          ज्ञातम्_द्वयङ्काः::compute_For_सङ्कलय_Sub(सङ्कलय_2, /*NSW=*/असत्य,
                                      /*NUW=*/असत्य, UnsignedLHS, UnsignedRHS);
      यदि (सङ्कलय_2) {
        यदि (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = असत्य;
          // Pos + Pos will overflow with extra signbit.
          यदि (LHS.isNonNegative() && RHS.isNonNegative())
            अतिप्रवाहः = सत्य;
        } अन्यथा यदि (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = असत्य;
          // Neg + Neg will overflow without extra signbit.
          यदि (LHS.isNegative() && RHS.isNegative())
            अतिप्रवाहः = सत्य;
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
            अतिप्रवाहः = सत्य;
        } अन्यथा यदि (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = असत्य;
          // Pos - Neg will overflow without extra signbit.
          यदि (LHS.isNonNegative() && RHS.isNegative())
            अतिप्रवाहः = सत्य;
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
      अतिप्रवाहः = असत्य;
  } अन्यथा यदि (सङ्कलय_2) {
    // uadd.sat
    तार्किक सम्बन्धि;
    (शून्य)LHS.get_Max_मानम्().uadd_ov(RHS.get_Max_मानम्(), सम्बन्धि);
    यदि (!सम्बन्धि) {
      अतिप्रवाहः = असत्य;
    } अन्यथा {
      (शून्य)LHS.get_Min_मानम्().uadd_ov(RHS.get_Min_मानम्(), सम्बन्धि);
      यदि (सम्बन्धि)
        अतिप्रवाहः = सत्य;
    }
  } अन्यथा {
    // usub.sat
    तार्किक सम्बन्धि;
    (शून्य)LHS.get_Min_मानम्().usub_ov(RHS.get_Max_मानम्(), सम्बन्धि);
    यदि (!सम्बन्धि) {
      अतिप्रवाहः = असत्य;
    } अन्यथा {
      (शून्य)LHS.get_Max_मानम्().usub_ov(RHS.get_Min_मानम्(), सम्बन्धि);
      यदि (सम्बन्धि)
        अतिप्रवाहः = सत्य;
    }
  }

  ज्ञातम्_द्वयङ्काः Res = ज्ञातम्_द्वयङ्काः::compute_For_सङ्कलय_Sub(सङ्कलय_2, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  यदि (अतिप्रवाहः) {
    // We know whether or not we overflowed.
    यदि (!(*अतिप्रवाहः)) {
      // No overflow.
      प्रत्यागम Res;
    }

    // We overflowed
    APInt C;
    यदि (Signed) {
      // sadd.sat / ssub.sat
      प्रतिज्ञा(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_मानम्(Bit_विस्तारः)
                           : APInt::get_Signed_Max_मानम्(Bit_विस्तारः);
    } अन्यथा यदि (सङ्कलय_2) {
      // uadd.sat
      C = APInt::get_Max_मानम्(Bit_विस्तारः);
    } अन्यथा {
      // uadd.sat
      C = APInt::get_Min_मानम्(Bit_विस्तारः);
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
      Res.Zero.clear_नीचः_द्वयङ्काः(Bit_विस्तारः - 1);
    यदि (MayNegClamp)
      Res.One.clear_नीचः_द्वयङ्काः(Bit_विस्तारः - 1);
  } अन्यथा यदि (सङ्कलय_2) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clear_All_द्वयङ्काः();
  } अन्यथा {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clear_All_द्वयङ्काः();
  }

  प्रत्यागम Res;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::sadd_sat(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  प्रत्यागम compute_For_Sat_सङ्कलय_Sub(/*सङ्कलय*/ सत्य, /*Signed*/ सत्य, LHS, RHS);
}
ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::ssub_sat(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  प्रत्यागम compute_For_Sat_सङ्कलय_Sub(/*सङ्कलय*/ असत्य, /*Signed*/ सत्य, LHS, RHS);
}
ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::uadd_sat(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  प्रत्यागम compute_For_Sat_सङ्कलय_Sub(/*सङ्कलय*/ सत्य, /*Signed*/ असत्य, LHS, RHS);
}
ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::usub_sat(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  प्रत्यागम compute_For_Sat_सङ्कलय_Sub(/*सङ्कलय*/ असत्य, /*Signed*/ असत्य, LHS, RHS);
}

स्थिर ज्ञातम्_द्वयङ्काः avgComputeU(ज्ञातम्_द्वयङ्काः LHS, ज्ञातम्_द्वयङ्काः RHS, तार्किक IsCeil) {
  अचिह्नित Bit_विस्तारः = LHS.get_Bit_विस्तारः();
  LHS = LHS.zext(Bit_विस्तारः + 1);
  RHS = RHS.zext(Bit_विस्तारः + 1);
  LHS =
      compute_For_सङ्कलय_Carry(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.extract_द्वयङ्काः(Bit_विस्तारः, 1);
  प्रत्यागम LHS;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::avgFloorS(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  प्रत्यागम flipSignBit(avgFloorU(flipSignBit(LHS), flipSignBit(RHS)));
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::avgFloorU(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  प्रत्यागम avgComputeU(LHS, RHS, /*IsCeil=*/असत्य);
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::avgCeilS(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  प्रत्यागम flipSignBit(avgCeilU(flipSignBit(LHS), flipSignBit(RHS)));
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::avgCeilU(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  प्रत्यागम avgComputeU(LHS, RHS, /*IsCeil=*/सत्य);
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::mul(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS,
                         तार्किक NoUndefSelfMultiply) {
  अचिह्नित Bit_विस्तारः = LHS.get_Bit_विस्तारः();
  प्रतिज्ञा(Bit_विस्तारः == RHS.get_Bit_विस्तारः() && "Operand mismatch");
  प्रतिज्ञा((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.get_Max_मानम्();
  APInt UMaxRHS = RHS.get_Max_मानम्();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  तार्किक Has_अतिप्रवाहः;
  APInt UMax_परिणाम = UMaxLHS.umul_ov(UMaxRHS, Has_अतिप्रवाहः);
  अचिह्नित LeadZ = Has_अतिप्रवाहः ? 0 : UMax_परिणाम.countl_zero();

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
  अचिह्नित Trail_द्वयङ्काः_ज्ञातम्_LHS = (LHS.Zero | LHS.One).countr_one();
  अचिह्नित Trail_द्वयङ्काः_ज्ञातम्_RHS = (RHS.Zero | RHS.One).countr_one();
  अचिह्नित TrailZeroLHS = LHS.गणनम्_Min_Trailing_Zeros();
  अचिह्नित TrailZeroRHS = RHS.गणनम्_Min_Trailing_Zeros();
  अचिह्नित TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  अचिह्नित Smallest_सङ्कार्यम् = std::min(Trail_द्वयङ्काः_ज्ञातम्_LHS - TrailZeroLHS,
                                      Trail_द्वयङ्काः_ज्ञातम्_RHS - TrailZeroRHS);
  अचिह्नित परिणाम_द्वयङ्काः_ज्ञातम् = std::min(Smallest_सङ्कार्यम् + TrailZ, Bit_विस्तारः);

  // The lower ResultBitsKnown bits of this are known.
  APInt Bottom_ज्ञातम् = LHS.One * RHS.One;

  ज्ञातम्_द्वयङ्काः Res(Bit_विस्तारः);
  Res.Zero.समुच्चयः_उच्चः_द्वयङ्काः(LeadZ);
  Res.Zero |= (~Bottom_ज्ञातम्).get_Lo_द्वयङ्काः(परिणाम_द्वयङ्काः_ज्ञातम्);
  Res.One = Bottom_ज्ञातम्.get_Lo_द्वयङ्काः(परिणाम_द्वयङ्काः_ज्ञातम्);

  यदि (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    अचिह्नित TwoTZP1 = 2 * TrailZeroLHS + 1;
    यदि (TwoTZP1 < Bit_विस्तारः)
      Res.Zero.समुच्चयः_Bit(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    यदि (TrailZeroLHS < Bit_विस्तारः && LHS.One[TrailZeroLHS]) {
      अचिह्नित TwoTZP2 = TwoTZP1 + 1;
      यदि (TwoTZP2 < Bit_विस्तारः)
        Res.Zero.समुच्चयः_Bit(TwoTZP2);
    }
  }

  प्रत्यागम Res;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::mulhs(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  अचिह्नित Bit_विस्तारः = LHS.get_Bit_विस्तारः();
  प्रतिज्ञा(Bit_विस्तारः == RHS.get_Bit_विस्तारः() && "Operand mismatch");
  ज्ञातम्_द्वयङ्काः WideLHS = LHS.sext(2 * Bit_विस्तारः);
  ज्ञातम्_द्वयङ्काः WideRHS = RHS.sext(2 * Bit_विस्तारः);
  प्रत्यागम mul(WideLHS, WideRHS).extract_द्वयङ्काः(Bit_विस्तारः, Bit_विस्तारः);
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::mulhu(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  अचिह्नित Bit_विस्तारः = LHS.get_Bit_विस्तारः();
  प्रतिज्ञा(Bit_विस्तारः == RHS.get_Bit_विस्तारः() && "Operand mismatch");
  ज्ञातम्_द्वयङ्काः WideLHS = LHS.zext(2 * Bit_विस्तारः);
  ज्ञातम्_द्वयङ्काः WideRHS = RHS.zext(2 * Bit_विस्तारः);
  प्रत्यागम mul(WideLHS, WideRHS).extract_द्वयङ्काः(Bit_विस्तारः, Bit_विस्तारः);
}

स्थिर ज्ञातम्_द्वयङ्काः div_Compute_नीचः_Bit(ज्ञातम्_द्वयङ्काः ज्ञातम्, ध्रुव ज्ञातम्_द्वयङ्काः &LHS,
                                  ध्रुव ज्ञातम्_द्वयङ्काः &RHS, तार्किक Exact) {

  यदि (!Exact)
    प्रत्यागम ज्ञातम्;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  यदि (LHS.One[0])
    ज्ञातम्.One.समुच्चयः_Bit(0);

  पूर्णाङ्क MinTZ =
      (पूर्णाङ्क)LHS.गणनम्_Min_Trailing_Zeros() - (पूर्णाङ्क)RHS.गणनम्_Max_Trailing_Zeros();
  पूर्णाङ्क MaxTZ =
      (पूर्णाङ्क)LHS.गणनम्_Max_Trailing_Zeros() - (पूर्णाङ्क)RHS.गणनम्_Min_Trailing_Zeros();
  यदि (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    ज्ञातम्.Zero.समुच्चयः_नीचः_द्वयङ्काः(MinTZ);
    यदि (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      ज्ञातम्.One.समुच्चयः_Bit(MinTZ);
    }
  } अन्यथा यदि (MaxTZ < 0) {
    // Poison Result
    ज्ञातम्.समुच्चयः_All_Zero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  यदि (ज्ञातम्.hasConflict())
    ज्ञातम्.समुच्चयः_All_Zero();

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::sdiv(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS,
                          तार्किक Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  यदि (LHS.isNonNegative() && RHS.isNonNegative())
    प्रत्यागम udiv(LHS, RHS, Exact);

  अचिह्नित Bit_विस्तारः = LHS.get_Bit_विस्तारः();
  ज्ञातम्_द्वयङ्काः ज्ञातम्(Bit_विस्तारः);

  यदि (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    ज्ञातम्.समुच्चयः_All_Zero();
    प्रत्यागम ज्ञातम्;
  }

  std::optional<APInt> Res;
  यदि (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_मानम्();
    APInt अङ्कः = LHS.get_Signed_Min_मानम्();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (अङ्कः.is_Min_Signed_मानम्() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_मानम्(Bit_विस्तारः)
              : अङ्कः.sdiv(Denom);
  } अन्यथा यदि (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    यदि (Exact || (-LHS.get_Signed_Max_मानम्()).uge(RHS.get_Signed_Max_मानम्())) {
      APInt Denom = RHS.get_Signed_Min_मानम्();
      APInt अङ्कः = LHS.get_Signed_Min_मानम्();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      यदि (Denom.isZero())
        Denom.समुच्चयः_Bit(RHS.गणनम्_Min_Trailing_Zeros());
      Res = अङ्कः.sdiv(Denom);
    }
  } अन्यथा यदि (LHS.is_Strictly_धनात्मकम्() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    यदि (Exact || LHS.get_Signed_Min_मानम्().uge(-RHS.get_Signed_Min_मानम्())) {
      APInt Denom = RHS.get_Signed_Max_मानम्();
      APInt अङ्कः = LHS.get_Signed_Max_मानम्();
      Res = अङ्कः.sdiv(Denom);
    }
  }

  यदि (Res) {
    यदि (Res->isNonNegative()) {
      अचिह्नित LeadZ = Res->गणनम्_Leading_Zeros();
      ज्ञातम्.Zero.समुच्चयः_उच्चः_द्वयङ्काः(LeadZ);
    } अन्यथा {
      अचिह्नित LeadO = Res->गणनम्_Leading_Ones();
      ज्ञातम्.One.समुच्चयः_उच्चः_द्वयङ्काः(LeadO);
    }
  }

  ज्ञातम् = div_Compute_नीचः_Bit(ज्ञातम्, LHS, RHS, Exact);
  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::udiv(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS,
                          तार्किक Exact) {
  अचिह्नित Bit_विस्तारः = LHS.get_Bit_विस्तारः();
  ज्ञातम्_द्वयङ्काः ज्ञातम्(Bit_विस्तारः);

  यदि (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    ज्ञातम्.समुच्चयः_All_Zero();
    प्रत्यागम ज्ञातम्;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_मानम्();
  यदि (MinDenom.isZero())
    MinDenom.समुच्चयः_Bit(RHS.गणनम्_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_मानम्().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_मानम्().udiv(RHS.get_Max_मानम्());

  ज्ञातम् = ज्ञातम्_द्वयङ्काः::makeConstant(MinRes);
  यदि (std::optional<अचिह्नित> DifferentBit =
          APIntOps::GetMostSignificantDifferentBit(MinRes, MaxRes)) {
    ज्ञातम्.Zero.clear_नीचः_द्वयङ्काः(*DifferentBit + 1);
    ज्ञातम्.One.clear_नीचः_द्वयङ्काः(*DifferentBit + 1);
  }

  ज्ञातम् = div_Compute_नीचः_Bit(ज्ञातम्, LHS, RHS, Exact);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::rem_Get_नीचः_द्वयङ्काः(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  अचिह्नित Bit_विस्तारः = LHS.get_Bit_विस्तारः();
  यदि (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    अचिह्नित RHSZeros = RHS.गणनम्_Min_Trailing_Zeros();
    APInt आवरणम् = APInt::get_नीचः_द्वयङ्काः_समुच्चयः(Bit_विस्तारः, RHSZeros);
    APInt Ones_आवरणम् = LHS.One & आवरणम्;
    APInt Zeros_आवरणम् = LHS.Zero & आवरणम्;
    प्रत्यागम ज्ञातम्_द्वयङ्काः(Zeros_आवरणम्, Ones_आवरणम्);
  }
  प्रत्यागम ज्ञातम्_द्वयङ्काः(Bit_विस्तारः);
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::urem(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  यदि (LHS.get_Max_मानम्().ult(RHS.get_Min_मानम्()))
    प्रत्यागम LHS;

  ज्ञातम्_द्वयङ्काः ज्ञातम् = rem_Get_नीचः_द्वयङ्काः(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_मानम्(), RHS.get_Max_मानम्() - 1);
  ज्ञातम्.Zero.समुच्चयः_उच्चः_द्वयङ्काः(MaxRes.गणनम्_Leading_Zeros());
  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::srem(ध्रुव ज्ञातम्_द्वयङ्काः &LHS, ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  ज्ञातम्_द्वयङ्काः ज्ञातम् = rem_Get_नीचः_द्वयङ्काः(LHS, RHS);
  यदि (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt नीचः_द्वयङ्काः = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    यदि (LHS.isNonNegative() || नीचः_द्वयङ्काः.is_Subset_सम्बन्धि(LHS.Zero))
      ज्ञातम्.Zero |= ~नीचः_द्वयङ्काः;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    यदि (LHS.isNegative() && नीचः_द्वयङ्काः.intersects(LHS.One))
      ज्ञातम्.One |= ~नीचः_द्वयङ्काः;
    प्रत्यागम ज्ञातम्;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  यदि (LHS.isNegative() && ज्ञातम्.isNonZero())
    ज्ञातम्.One.समुच्चयः_उच्चः_द्वयङ्काः(
        std::max(LHS.गणनम्_Min_Leading_Ones(), RHS.गणनम्_Min_Sign_द्वयङ्काः()));
  अन्यथा यदि (LHS.isNonNegative())
    ज्ञातम्.Zero.समुच्चयः_उच्चः_द्वयङ्काः(
        std::max(LHS.गणनम्_Min_Leading_Zeros(), RHS.गणनम्_Min_Sign_द्वयङ्काः()));
  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_द्वयङ्काः &ज्ञातम्_द्वयङ्काः::संक्रियकः&=(ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  प्रत्यागम *एतत्;
}

ज्ञातम्_द्वयङ्काः &ज्ञातम्_द्वयङ्काः::संक्रियकः|=(ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  प्रत्यागम *एतत्;
}

ज्ञातम्_द्वयङ्काः &ज्ञातम्_द्वयङ्काः::संक्रियकः^=(ध्रुव ज्ञातम्_द्वयङ्काः &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  प्रत्यागम *एतत्;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::blsi() ध्रुव {
  अचिह्नित Bit_विस्तारः = get_Bit_विस्तारः();
  ज्ञातम्_द्वयङ्काः ज्ञातम्(Zero, APInt(Bit_विस्तारः, 0));
  अचिह्नित Max = गणनम्_Max_Trailing_Zeros();
  ज्ञातम्.Zero.समुच्चयः_द्वयङ्काः_From(std::min(Max + 1, Bit_विस्तारः));
  अचिह्नित Min = गणनम्_Min_Trailing_Zeros();
  यदि (Max == Min && Max < Bit_विस्तारः)
    ज्ञातम्.One.समुच्चयः_Bit(Max);
  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_द्वयङ्काः::blsmsk() ध्रुव {
  अचिह्नित Bit_विस्तारः = get_Bit_विस्तारः();
  ज्ञातम्_द्वयङ्काः ज्ञातम्(Bit_विस्तारः);
  अचिह्नित Max = गणनम्_Max_Trailing_Zeros();
  ज्ञातम्.Zero.समुच्चयः_द्वयङ्काः_From(std::min(Max + 1, Bit_विस्तारः));
  अचिह्नित Min = गणनम्_Min_Trailing_Zeros();
  ज्ञातम्.One.समुच्चयः_नीचः_द्वयङ्काः(std::min(Min + 1, Bit_विस्तारः));
  प्रत्यागम ज्ञातम्;
}

शून्य ज्ञातम्_द्वयङ्काः::print(raw_ostream &OS) ध्रुव {
  अचिह्नित Bit_विस्तारः = get_Bit_विस्तारः();
  प्रति (अचिह्नित I = 0; I < Bit_विस्तारः; ++I) {
    अचिह्नित N = Bit_विस्तारः - I - 1;
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
LLVM_DUMP_विधानम् शून्य ज्ञातम्_द्वयङ्काः::dump() ध्रुव {
  print(dbgs());
  dbgs() << "\n";
}
#यदि_अन्तः

अचिह्नित llvm::Sign_द्वयङ्काः_Ops::rot(अचिह्नित Src_Sign_द्वयङ्काः, अचिह्नित Bit_विस्तारः,
                                std::optional<APInt> RotAmt,
                                तार्किक Is_Rotate_दक्षिणः) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  यदि (Src_Sign_द्वयङ्काः == Bit_विस्तारः)
    प्रत्यागम Bit_विस्तारः;

  यदि (!RotAmt)
    प्रत्यागम 1;

  अचिह्नित Amt = RotAmt->urem(Bit_विस्तारः);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  यदि (Is_Rotate_दक्षिणः)
    Amt = (Bit_विस्तारः - Amt) % Bit_विस्तारः;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  यदि (Src_Sign_द्वयङ्काः > Amt + 1)
    प्रत्यागम Src_Sign_द्वयङ्काः - Amt;
  प्रत्यागम 1;
}
