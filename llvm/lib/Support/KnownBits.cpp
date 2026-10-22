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

استعمال نامخلا llvm;

معلوم_بٹ معلوم_بٹ::flip_Sign_ثنائی_ہندسہ(ثابت معلوم_بٹ &Val) {
  غیرعلامتی Sign_ثنائی_ہندسہ_Position = Val.get_ثنائی_ہندسہ_چوڑائی() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.سیٹ_ثنائی_ہندسہ_Val(Sign_ثنائی_ہندسہ_Position, Val.One[Sign_ثنائی_ہندسہ_Position]);
  One.سیٹ_ثنائی_ہندسہ_Val(Sign_ثنائی_ہندسہ_Position, Val.Zero[Sign_ثنائی_ہندسہ_Position]);
  واپس معلوم_بٹ(Zero, One);
}

ساکن معلوم_بٹ compute_For_جوڑو_حاصل(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS,
                                    منطقی حاصل_Zero, منطقی حاصل_One) {

  APInt Possible_مجموع_Zero = LHS.get_Max_قدر() + RHS.get_Max_قدر() + !حاصل_Zero;
  APInt Possible_مجموع_One = LHS.get_Min_قدر() + RHS.get_Min_قدر() + حاصل_One;

  // Compute known bits of the carry.
  APInt حاصل_معلوم_Zero = ~(Possible_مجموع_Zero ^ LHS.Zero ^ RHS.Zero);
  APInt حاصل_معلوم_One = Possible_مجموع_One ^ LHS.One ^ RHS.One;

  // Compute set of known bits (where all three relevant bits are known).
  APInt LHSKnownUnion = LHS.Zero | LHS.One;
  APInt RHSKnownUnion = RHS.Zero | RHS.One;
  APInt حاصل_معلوم_Union = std::move(حاصل_معلوم_Zero) | حاصل_معلوم_One;
  APInt معلوم = std::move(LHSKnownUnion) & RHSKnownUnion & حاصل_معلوم_Union;

  // Compute known bits of the result.
  معلوم_بٹ معلوم_برآمد;
  معلوم_برآمد.Zero = ~std::move(Possible_مجموع_Zero) & معلوم;
  معلوم_برآمد.One = std::move(Possible_مجموع_One) & معلوم;
  واپس معلوم_برآمد;
}

معلوم_بٹ معلوم_بٹ::compute_For_جوڑو_حاصل(
    ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS, ثابت معلوم_بٹ &حاصل_2) {
  أكد(حاصل_2.get_ثنائی_ہندسہ_چوڑائی() == 1 && "Carry must be 1-bit");
  واپس ::compute_For_جوڑو_حاصل(
      LHS, RHS, حاصل_2.Zero.get_Bool_قدر(), حاصل_2.One.get_Bool_قدر());
}

معلوم_بٹ معلوم_بٹ::compute_For_جوڑو_Sub(منطقی جوڑو_2, منطقی NSW, منطقی NUW,
                                      ثابت معلوم_بٹ &LHS,
                                      ثابت معلوم_بٹ &RHS) {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  معلوم_بٹ معلوم_برآمد(ثنائی_ہندسہ_چوڑائی_2);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  اگر (LHS.isUnknown() && RHS.isUnknown())
    واپس معلوم_برآمد;

  اگر (!LHS.isUnknown() && !RHS.isUnknown()) {
    اگر (جوڑو_2) {
      // Sum = LHS + RHS + 0
      معلوم_برآمد = ::compute_For_جوڑو_حاصل(LHS, RHS, /*CarryZero=*/سچ,
                                      /*CarryOne=*/جھوٹ);
    } ورنہ {
      // Sum = LHS + ~RHS + 1
      معلوم_بٹ NotRHS = RHS;
      std::تبادلہ(NotRHS.Zero, NotRHS.One);
      معلوم_برآمد = ::compute_For_جوڑو_حاصل(LHS, NotRHS, /*CarryZero=*/جھوٹ,
                                      /*CarryOne=*/سچ);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  اگر (NUW) {
    اگر (جوڑو_2) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_قدر().uadd_sat(RHS.get_Min_قدر());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      اگر (NSW) {
        غیرعلامتی عدد_بٹ_2 = MinVal.trunc(ثنائی_ہندسہ_چوڑائی_2 - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        معلوم_برآمد.One.سیٹ_بٹ(ثنائی_ہندسہ_چوڑائی_2 - 1 - عدد_بٹ_2, ثنائی_ہندسہ_چوڑائی_2 - 1);
      }
      معلوم_برآمد.One.سیٹ_بلند_بٹ(MinVal.countl_one());
    } ورنہ {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_قدر().usub_sat(RHS.get_Min_قدر());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      اگر (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        غیرعلامتی عدد_بٹ_2 = MaxVal.trunc(ثنائی_ہندسہ_چوڑائی_2 - 1).countl_zero();
        معلوم_برآمد.Zero.سیٹ_بٹ(ثنائی_ہندسہ_چوڑائی_2 - 1 - عدد_بٹ_2, ثنائی_ہندسہ_چوڑائی_2 - 1);
      }
      معلوم_برآمد.Zero.سیٹ_بلند_بٹ(MaxVal.countl_zero());
    }
  }

  اگر (NSW) {
    APInt MinVal;
    APInt MaxVal;
    اگر (جوڑو_2) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_قدر().sadd_sat(RHS.get_Signed_Min_قدر());
      MaxVal = LHS.get_Signed_Max_قدر().sadd_sat(RHS.get_Signed_Max_قدر());
    } ورنہ {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_قدر().ssub_sat(RHS.get_Signed_Max_قدر());
      MaxVal = LHS.get_Signed_Max_قدر().ssub_sat(RHS.get_Signed_Min_قدر());
    }
    اگر (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      غیرعلامتی عدد_بٹ_2 = MinVal.trunc(ثنائی_ہندسہ_چوڑائی_2 - 1).countl_one();
      معلوم_برآمد.One.سیٹ_بٹ(ثنائی_ہندسہ_چوڑائی_2 - 1 - عدد_بٹ_2, ثنائی_ہندسہ_چوڑائی_2 - 1);
      معلوم_برآمد.Zero.سیٹ_Sign_ثنائی_ہندسہ();
    }
    اگر (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      غیرعلامتی عدد_بٹ_2 = MaxVal.trunc(ثنائی_ہندسہ_چوڑائی_2 - 1).countl_zero();
      معلوم_برآمد.Zero.سیٹ_بٹ(ثنائی_ہندسہ_چوڑائی_2 - 1 - عدد_بٹ_2, ثنائی_ہندسہ_چوڑائی_2 - 1);
      معلوم_برآمد.One.سیٹ_Sign_ثنائی_ہندسہ();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  اگر (معلوم_برآمد.hasConflict())
    معلوم_برآمد.سیٹ_All_Zero();
  واپس معلوم_برآمد;
}

معلوم_بٹ معلوم_بٹ::computeForSubBorrow(ثابت معلوم_بٹ &LHS, معلوم_بٹ RHS,
                                         ثابت معلوم_بٹ &Borrow) {
  أكد(Borrow.get_ثنائی_ہندسہ_چوڑائی() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::تبادلہ(RHS.Zero, RHS.One);
  واپس ::compute_For_جوڑو_حاصل(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_قدر(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_قدر());
}

معلوم_بٹ معلوم_بٹ::truncSSat(غیرعلامتی ثنائی_ہندسہ_چوڑائی_2) ثابت {
  غیرعلامتی داخلہ_بٹ = get_ثنائی_ہندسہ_چوڑائی();
  APInt Min_اندر_Range = APInt::get_Signed_Min_قدر(ثنائی_ہندسہ_چوڑائی_2).sext(داخلہ_بٹ);
  APInt Max_اندر_Range = APInt::get_Signed_Max_قدر(ثنائی_ہندسہ_چوڑائی_2).sext(داخلہ_بٹ);
  APInt داخلہ_Min = get_Signed_Min_قدر();
  APInt داخلہ_Max = get_Signed_Max_قدر();
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);

  // Case 1: All values fit - just truncate
  اگر (داخلہ_Min.sge(Min_اندر_Range) && داخلہ_Max.sle(Max_اندر_Range)) {
    معلوم = trunc(ثنائی_ہندسہ_چوڑائی_2);
  }
  // Case 2: All saturate to min
  ورنہ اگر (داخلہ_Max.slt(Min_اندر_Range)) {
    معلوم = معلوم_بٹ::makeConstant(APInt::get_Signed_Min_قدر(ثنائی_ہندسہ_چوڑائی_2));
  }
  // Case 3: All saturate to max
  ورنہ اگر (داخلہ_Min.sgt(Max_اندر_Range)) {
    معلوم = معلوم_بٹ::makeConstant(APInt::get_Signed_Max_قدر(ثنائی_ہندسہ_چوڑائی_2));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  ورنہ اگر (داخلہ_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    معلوم.Zero = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
    معلوم.Zero.سیٹ_ثنائی_ہندسہ(ثنائی_ہندسہ_چوڑائی_2 - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    معلوم.One = One.trunc(ثنائی_ہندسہ_چوڑائی_2);
    معلوم.One.clear_ثنائی_ہندسہ(ثنائی_ہندسہ_چوڑائی_2 - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  ورنہ اگر (داخلہ_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    معلوم.Zero = Zero.trunc(ثنائی_ہندسہ_چوڑائی_2);
    معلوم.Zero.clear_ثنائی_ہندسہ(ثنائی_ہندسہ_چوڑائی_2 - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    معلوم.One = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
    معلوم.One.سیٹ_ثنائی_ہندسہ(ثنائی_ہندسہ_چوڑائی_2 - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  ورنہ {
    // Output: min saturation, truncated, or max saturation
    APInt داخلہ_Zero_زیریں = Zero.trunc(ثنائی_ہندسہ_چوڑائی_2);
    APInt داخلہ_One_زیریں = One.trunc(ثنائی_ہندسہ_چوڑائی_2);
    APInt MinSat = APInt::get_Signed_Min_قدر(ثنائی_ہندسہ_چوڑائی_2);
    APInt MaxSat = APInt::get_Signed_Max_قدر(ثنائی_ہندسہ_چوڑائی_2);
    معلوم_بٹ MinSatKB = معلوم_بٹ::makeConstant(MinSat);
    معلوم_بٹ MaxSatKB = معلوم_بٹ::makeConstant(MaxSat);
    اگر (داخلہ_Max.sle(Max_اندر_Range)) {
      // Positive values fit, only negatives saturate to min
      معلوم.Zero = داخلہ_Zero_زیریں & MinSatKB.Zero;
      معلوم.One = داخلہ_One_زیریں & MinSatKB.One;
    } ورنہ اگر (داخلہ_Min.sge(Min_اندر_Range)) {
      // Negative values fit, only positives saturate to max
      معلوم.Zero = داخلہ_Zero_زیریں & MaxSatKB.Zero;
      معلوم.One = داخلہ_One_زیریں & MaxSatKB.One;
    } ورنہ {
      // Both positive and negative values might saturate
      معلوم.Zero = داخلہ_Zero_زیریں & MinSatKB.Zero & MaxSatKB.Zero;
      معلوم.One = داخلہ_One_زیریں & MinSatKB.One & MaxSatKB.One;
    }
  }
  واپس معلوم;
}

معلوم_بٹ معلوم_بٹ::truncSSatU(غیرعلامتی ثنائی_ہندسہ_چوڑائی_2) ثابت {
  غیرعلامتی داخلہ_بٹ = get_ثنائی_ہندسہ_چوڑائی();
  APInt Max_اندر_Range = APInt::getAllOnes(ثنائی_ہندسہ_چوڑائی_2).zext(داخلہ_بٹ);
  APInt داخلہ_Min = get_Signed_Min_قدر();
  APInt داخلہ_Max = get_Signed_Max_قدر();
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);

  اگر (isNegative()) {
    معلوم.سیٹ_All_Zero();
  } ورنہ اگر (داخلہ_Min.isNonNegative() && داخلہ_Max.ule(Max_اندر_Range)) {
    معلوم = trunc(ثنائی_ہندسہ_چوڑائی_2);
  } ورنہ اگر (داخلہ_Min.isNonNegative() && داخلہ_Min.ugt(Max_اندر_Range)) {
    معلوم.سیٹ_All_Ones();
  } ورنہ اگر (داخلہ_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    معلوم.Zero = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
    // Common one bits: bits that are 1 in input stay 1
    معلوم.One = One.trunc(ثنائی_ہندسہ_چوڑائی_2);
  } ورنہ {
    // Mixed: sign bit unknown
    اگر (داخلہ_Max.ule(Max_اندر_Range)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      معلوم.Zero = Zero.trunc(ثنائی_ہندسہ_چوڑائی_2);
      معلوم.One = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
    } ورنہ {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      معلوم.Zero = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
      معلوم.One = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
    }
  }
  واپس معلوم;
}

معلوم_بٹ معلوم_بٹ::truncUSat(غیرعلامتی ثنائی_ہندسہ_چوڑائی_2) ثابت {
  غیرعلامتی داخلہ_بٹ = get_ثنائی_ہندسہ_چوڑائی();
  APInt Max_اندر_Range = APInt::get_کم_بٹ_سیٹ(داخلہ_بٹ, ثنائی_ہندسہ_چوڑائی_2);
  APInt داخلہ_Max = get_Max_قدر();
  APInt داخلہ_Min = get_Min_قدر();
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);

  اگر (داخلہ_Max.ule(Max_اندر_Range)) {
    معلوم = trunc(ثنائی_ہندسہ_چوڑائی_2);
  } ورنہ اگر (داخلہ_Min.ugt(Max_اندر_Range)) {
    معلوم.سیٹ_All_Ones();
  } ورنہ {
    معلوم.resetAll();
    معلوم.One = One.trunc(ثنائی_ہندسہ_چوڑائی_2);
  }
  واپس معلوم;
}

معلوم_بٹ معلوم_بٹ::sext_اندر_Reg(غیرعلامتی Src_ثنائی_ہندسہ_چوڑائی) ثابت {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = get_ثنائی_ہندسہ_چوڑائی();
  أكد(0 < Src_ثنائی_ہندسہ_چوڑائی && Src_ثنائی_ہندسہ_چوڑائی <= ثنائی_ہندسہ_چوڑائی_2 &&
         "Illegal sext-in-register");

  اگر (Src_ثنائی_ہندسہ_چوڑائی == ثنائی_ہندسہ_چوڑائی_2)
    واپس *یہ;

  غیرعلامتی Ext_بٹ = ثنائی_ہندسہ_چوڑائی_2 - Src_ثنائی_ہندسہ_چوڑائی;
  معلوم_بٹ نتیجہ;
  نتیجہ.One = One << Ext_بٹ;
  نتیجہ.Zero = Zero << Ext_بٹ;
  نتیجہ.One.ashr_اندر_رکھو(Ext_بٹ);
  نتیجہ.Zero.ashr_اندر_رکھو(Ext_بٹ);
  واپس نتیجہ;
}

معلوم_بٹ معلوم_بٹ::makeGE(ثابت APInt &Val) ثابت {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  غیرعلامتی N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_کم_بٹ(get_ثنائی_ہندسہ_چوڑائی() - N);
  واپس معلوم_بٹ(Zero, One | MaskedVal);
}

معلوم_بٹ معلوم_بٹ::umax(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  اگر (LHS.get_Min_قدر().uge(RHS.get_Max_قدر()))
    واپس LHS;
  اگر (RHS.get_Min_قدر().uge(LHS.get_Max_قدر()))
    واپس RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  معلوم_بٹ L = LHS.makeGE(RHS.get_Min_قدر());
  معلوم_بٹ R = RHS.makeGE(LHS.get_Min_قدر());
  واپس L.intersectWith(R);
}

معلوم_بٹ معلوم_بٹ::umin(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  خودکار Flip = [](ثابت معلوم_بٹ &Val) { واپس معلوم_بٹ(Val.One, Val.Zero); };
  واپس Flip(umax(Flip(LHS), Flip(RHS)));
}

معلوم_بٹ معلوم_بٹ::smax(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  واپس flip_Sign_ثنائی_ہندسہ(umax(flip_Sign_ثنائی_ہندسہ(LHS), flip_Sign_ثنائی_ہندسہ(RHS)));
}

معلوم_بٹ معلوم_بٹ::smin(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  خودکار Flip = [](ثابت معلوم_بٹ &Val) {
    غیرعلامتی Sign_ثنائی_ہندسہ_Position = Val.get_ثنائی_ہندسہ_چوڑائی() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.سیٹ_ثنائی_ہندسہ_Val(Sign_ثنائی_ہندسہ_Position, Val.Zero[Sign_ثنائی_ہندسہ_Position]);
    One.سیٹ_ثنائی_ہندسہ_Val(Sign_ثنائی_ہندسہ_Position, Val.One[Sign_ثنائی_ہندسہ_Position]);
    واپس معلوم_بٹ(Zero, One);
  };
  واپس Flip(umax(Flip(LHS), Flip(RHS)));
}

معلوم_بٹ معلوم_بٹ::abdu(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  اگر (LHS.get_Min_قدر().uge(RHS.get_Max_قدر()))
    واپس compute_For_جوڑو_Sub(/*Add=*/جھوٹ, /*NSW=*/جھوٹ, /*NUW=*/جھوٹ, LHS,
                            RHS);
  اگر (RHS.get_Min_قدر().uge(LHS.get_Max_قدر()))
    واپس compute_For_جوڑو_Sub(/*Add=*/جھوٹ, /*NSW=*/جھوٹ, /*NUW=*/جھوٹ, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  معلوم_بٹ Diff0 =
      compute_For_جوڑو_Sub(/*Add=*/جھوٹ, /*NSW=*/جھوٹ, /*NUW=*/سچ, LHS, RHS);
  معلوم_بٹ Diff1 =
      compute_For_جوڑو_Sub(/*Add=*/جھوٹ, /*NSW=*/جھوٹ, /*NUW=*/سچ, RHS, LHS);
  واپس Diff0.intersectWith(Diff1);
}

معلوم_بٹ معلوم_بٹ::abds(معلوم_بٹ LHS, معلوم_بٹ RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  اگر (LHS.get_Signed_Min_قدر().sge(RHS.get_Signed_Max_قدر()))
    واپس compute_For_جوڑو_Sub(/*Add=*/جھوٹ, /*NSW=*/جھوٹ, /*NUW=*/جھوٹ, LHS,
                            RHS);
  اگر (RHS.get_Signed_Min_قدر().sge(LHS.get_Signed_Max_قدر()))
    واپس compute_For_جوڑو_Sub(/*Add=*/جھوٹ, /*NSW=*/جھوٹ, /*NUW=*/جھوٹ, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  غیرعلامتی Sign_ثنائی_ہندسہ_Position = LHS.get_ثنائی_ہندسہ_چوڑائی() - 1;
  برائے (خودکار Arg : {&LHS, &RHS}) {
    منطقی Tmp = Arg->Zero[Sign_ثنائی_ہندسہ_Position];
    Arg->Zero.سیٹ_ثنائی_ہندسہ_Val(Sign_ثنائی_ہندسہ_Position, Arg->One[Sign_ثنائی_ہندسہ_Position]);
    Arg->One.سیٹ_ثنائی_ہندسہ_Val(Sign_ثنائی_ہندسہ_Position, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  معلوم_بٹ Diff0 =
      compute_For_جوڑو_Sub(/*Add=*/جھوٹ, /*NSW=*/جھوٹ, /*NUW=*/سچ, LHS, RHS);
  معلوم_بٹ Diff1 =
      compute_For_جوڑو_Sub(/*Add=*/جھوٹ, /*NSW=*/جھوٹ, /*NUW=*/سچ, RHS, LHS);
  واپس Diff0.intersectWith(Diff1);
}

ساکن غیرعلامتی get_Max_سرکاؤ_Amount(ثابت APInt &Max_قدر, غیرعلامتی ثنائی_ہندسہ_چوڑائی_2) {
  اگر (isPowerOf2_32(ثنائی_ہندسہ_چوڑائی_2))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    واپس Max_قدر.نکالو_بٹ_As_ZExt_قدر(
        std::min(Log2_32(ثنائی_ہندسہ_چوڑائی_2), Max_قدر.get_ثنائی_ہندسہ_چوڑائی()), 0);
  // This is only an approximate upper bound.
  واپس Max_قدر.get_Limited_قدر(ثنائی_ہندسہ_چوڑائی_2 - 1);
}

معلوم_بٹ معلوم_بٹ::shl(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS, منطقی NUW,
                         منطقی NSW, منطقی ShAmtNonZero) {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  خودکار سرکاؤ_By_Const = [&](ثابت معلوم_بٹ &LHS, غیرعلامتی سرکاؤ_Amt_2) {
    معلوم_بٹ معلوم;
    منطقی Shifted_برآمد_Zero, Shifted_برآمد_One;
    معلوم.Zero = LHS.Zero.ushl_ov(سرکاؤ_Amt_2, Shifted_برآمد_Zero);
    معلوم.Zero.سیٹ_کم_بٹ(سرکاؤ_Amt_2);
    معلوم.One = LHS.One.ushl_ov(سرکاؤ_Amt_2, Shifted_برآمد_One);

    // All cases returning poison have been handled by MaxShiftAmount already.
    اگر (NSW) {
      اگر (NUW && سرکاؤ_Amt_2 != 0)
        // NUW means we can assume anything shifted out was a zero.
        Shifted_برآمد_Zero = سچ;

      اگر (Shifted_برآمد_Zero)
        معلوم.makeNonNegative();
      ورنہ اگر (Shifted_برآمد_One)
        معلوم.makeNegative();
    }
    واپس معلوم;
  };

  // Fast path for a common case when LHS is completely unknown.
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);
  غیرعلامتی Min_سرکاؤ_Amount = RHS.get_Min_قدر().get_Limited_قدر(ثنائی_ہندسہ_چوڑائی_2);
  اگر (Min_سرکاؤ_Amount == 0 && ShAmtNonZero)
    Min_سرکاؤ_Amount = 1;
  اگر (LHS.isUnknown()) {
    معلوم.Zero.سیٹ_کم_بٹ(Min_سرکاؤ_Amount);
    اگر (NUW && NSW && Min_سرکاؤ_Amount != 0)
      معلوم.makeNonNegative();
    واپس معلوم;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_قدر = RHS.get_Max_قدر();
  غیرعلامتی Max_سرکاؤ_Amount = get_Max_سرکاؤ_Amount(Max_قدر, ثنائی_ہندسہ_چوڑائی_2);
  اگر (NUW && NSW)
    Max_سرکاؤ_Amount = std::min(Max_سرکاؤ_Amount, LHS.گنتی_Max_Leading_Zeros() - 1);
  اگر (NUW)
    Max_سرکاؤ_Amount = std::min(Max_سرکاؤ_Amount, LHS.گنتی_Max_Leading_Zeros());
  اگر (NSW)
    Max_سرکاؤ_Amount = std::min(
        Max_سرکاؤ_Amount,
        std::max(LHS.گنتی_Max_Leading_Zeros(), LHS.گنتی_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  اگر (Min_سرکاؤ_Amount == 0 && Max_سرکاؤ_Amount == ثنائی_ہندسہ_چوڑائی_2 - 1 &&
      isPowerOf2_32(ثنائی_ہندسہ_چوڑائی_2)) {
    معلوم.Zero.سیٹ_کم_بٹ(LHS.گنتی_Min_Trailing_Zeros());
    اگر (LHS.isAllOnes())
      معلوم.One.سیٹ_Sign_ثنائی_ہندسہ();
    اگر (NSW) {
      اگر (LHS.isNonNegative())
        معلوم.makeNonNegative();
      اگر (LHS.isNegative())
        معلوم.makeNegative();
    }
    واپس معلوم;
  }

  // Find the common bits from all possible shifts.
  غیرعلامتی سرکاؤ_Amt_Zero_نقاب = RHS.Zero.zextOrTrunc(32).get_ZExt_قدر();
  غیرعلامتی سرکاؤ_Amt_One_نقاب = RHS.One.zextOrTrunc(32).get_ZExt_قدر();
  معلوم.سیٹ_All_Conflict();
  برائے (غیرعلامتی سرکاؤ_Amt_2 = Min_سرکاؤ_Amount; سرکاؤ_Amt_2 <= Max_سرکاؤ_Amount;
       ++سرکاؤ_Amt_2) {
    // Skip if the shift amount is impossible.
    اگر ((سرکاؤ_Amt_Zero_نقاب & سرکاؤ_Amt_2) != 0 ||
        (سرکاؤ_Amt_One_نقاب | سرکاؤ_Amt_2) != سرکاؤ_Amt_2)
      جاری;
    معلوم = معلوم.intersectWith(سرکاؤ_By_Const(LHS, سرکاؤ_Amt_2));
    اگر (معلوم.isUnknown())
      توڑو;
  }

  // All shift amounts may result in poison.
  اگر (معلوم.hasConflict())
    معلوم.سیٹ_All_Zero();
  واپس معلوم;
}

معلوم_بٹ معلوم_بٹ::lshr(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS,
                          منطقی ShAmtNonZero, منطقی Exact) {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  خودکار سرکاؤ_By_Const = [&](ثابت معلوم_بٹ &LHS, غیرعلامتی سرکاؤ_Amt_2) {
    معلوم_بٹ معلوم = LHS;
    معلوم >>= سرکاؤ_Amt_2;
    // High bits are known zero.
    معلوم.Zero.سیٹ_بلند_بٹ(سرکاؤ_Amt_2);
    واپس معلوم;
  };

  // Fast path for a common case when LHS is completely unknown.
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);
  غیرعلامتی Min_سرکاؤ_Amount = RHS.get_Min_قدر().get_Limited_قدر(ثنائی_ہندسہ_چوڑائی_2);
  اگر (Min_سرکاؤ_Amount == 0 && ShAmtNonZero)
    Min_سرکاؤ_Amount = 1;
  اگر (LHS.isUnknown()) {
    معلوم.Zero.سیٹ_بلند_بٹ(Min_سرکاؤ_Amount);
    واپس معلوم;
  }

  // Find the common bits from all possible shifts.
  APInt Max_قدر = RHS.get_Max_قدر();
  غیرعلامتی Max_سرکاؤ_Amount = get_Max_سرکاؤ_Amount(Max_قدر, ثنائی_ہندسہ_چوڑائی_2);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  اگر (Exact) {
    غیرعلامتی پہلا_One = LHS.گنتی_Max_Trailing_Zeros();
    اگر (پہلا_One < Min_سرکاؤ_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      معلوم.سیٹ_All_Zero();
      واپس معلوم;
    }
    Max_سرکاؤ_Amount = std::min(Max_سرکاؤ_Amount, پہلا_One);
  }

  غیرعلامتی سرکاؤ_Amt_Zero_نقاب = RHS.Zero.zextOrTrunc(32).get_ZExt_قدر();
  غیرعلامتی سرکاؤ_Amt_One_نقاب = RHS.One.zextOrTrunc(32).get_ZExt_قدر();
  معلوم.سیٹ_All_Conflict();
  برائے (غیرعلامتی سرکاؤ_Amt_2 = Min_سرکاؤ_Amount; سرکاؤ_Amt_2 <= Max_سرکاؤ_Amount;
       ++سرکاؤ_Amt_2) {
    // Skip if the shift amount is impossible.
    اگر ((سرکاؤ_Amt_Zero_نقاب & سرکاؤ_Amt_2) != 0 ||
        (سرکاؤ_Amt_One_نقاب | سرکاؤ_Amt_2) != سرکاؤ_Amt_2)
      جاری;
    معلوم = معلوم.intersectWith(سرکاؤ_By_Const(LHS, سرکاؤ_Amt_2));
    اگر (معلوم.isUnknown())
      توڑو;
  }

  // All shift amounts may result in poison.
  اگر (معلوم.hasConflict())
    معلوم.سیٹ_All_Zero();
  واپس معلوم;
}

معلوم_بٹ معلوم_بٹ::ashr(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS,
                          منطقی ShAmtNonZero, منطقی Exact) {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  خودکار سرکاؤ_By_Const = [&](ثابت معلوم_بٹ &LHS, غیرعلامتی سرکاؤ_Amt_2) {
    معلوم_بٹ معلوم = LHS;
    معلوم.Zero.ashr_اندر_رکھو(سرکاؤ_Amt_2);
    معلوم.One.ashr_اندر_رکھو(سرکاؤ_Amt_2);
    واپس معلوم;
  };

  // Fast path for a common case when LHS is completely unknown.
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);
  غیرعلامتی Min_سرکاؤ_Amount = RHS.get_Min_قدر().get_Limited_قدر(ثنائی_ہندسہ_چوڑائی_2);
  اگر (Min_سرکاؤ_Amount == 0 && ShAmtNonZero)
    Min_سرکاؤ_Amount = 1;
  اگر (LHS.isUnknown()) {
    اگر (Min_سرکاؤ_Amount == ثنائی_ہندسہ_چوڑائی_2) {
      // Always poison. Return zero because we don't like returning conflict.
      معلوم.سیٹ_All_Zero();
      واپس معلوم;
    }
    واپس معلوم;
  }

  // Find the common bits from all possible shifts.
  APInt Max_قدر = RHS.get_Max_قدر();
  غیرعلامتی Max_سرکاؤ_Amount = get_Max_سرکاؤ_Amount(Max_قدر, ثنائی_ہندسہ_چوڑائی_2);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  اگر (Exact) {
    غیرعلامتی پہلا_One = LHS.گنتی_Max_Trailing_Zeros();
    اگر (پہلا_One < Min_سرکاؤ_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      معلوم.سیٹ_All_Zero();
      واپس معلوم;
    }
    Max_سرکاؤ_Amount = std::min(Max_سرکاؤ_Amount, پہلا_One);
  }

  غیرعلامتی سرکاؤ_Amt_Zero_نقاب = RHS.Zero.zextOrTrunc(32).get_ZExt_قدر();
  غیرعلامتی سرکاؤ_Amt_One_نقاب = RHS.One.zextOrTrunc(32).get_ZExt_قدر();
  معلوم.سیٹ_All_Conflict();
  برائے (غیرعلامتی سرکاؤ_Amt_2 = Min_سرکاؤ_Amount; سرکاؤ_Amt_2 <= Max_سرکاؤ_Amount;
      ++سرکاؤ_Amt_2) {
    // Skip if the shift amount is impossible.
    اگر ((سرکاؤ_Amt_Zero_نقاب & سرکاؤ_Amt_2) != 0 ||
        (سرکاؤ_Amt_One_نقاب | سرکاؤ_Amt_2) != سرکاؤ_Amt_2)
      جاری;
    معلوم = معلوم.intersectWith(سرکاؤ_By_Const(LHS, سرکاؤ_Amt_2));
    اگر (معلوم.isUnknown())
      توڑو;
  }

  // All shift amounts may result in poison.
  اگر (معلوم.hasConflict())
    معلوم.سیٹ_All_Zero();
  واپس معلوم;
}

معلوم_بٹ معلوم_بٹ::fshl(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS,
                          ثابت APInt &Amt) {
  واپس معلوم_بٹ(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

معلوم_بٹ معلوم_بٹ::fshr(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS,
                          ثابت APInt &Amt) {
  واپس معلوم_بٹ(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

معلوم_بٹ معلوم_بٹ::clmul(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  معلوم_بٹ Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_قدر(), RHS.get_Min_قدر()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  خودکار ClMulOr = [](ثابت APInt &LHS, ثابت APInt &RHS) {
    غیرعلامتی BW = LHS.get_ثنائی_ہندسہ_چوڑائی();
    أكد(BW == RHS.get_ثنائی_ہندسہ_چوڑائی() && "Operand mismatch");
    APInt نتیجہ(BW, 0);
    برائے (غیرعلامتی I :
         seq(std::min(RHS.get_فعال_بٹ(), BW - LHS.countr_zero())))
      اگر (RHS[I])
        نتیجہ |= LHS << I;
    واپس نتیجہ;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt معلوم = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= معلوم;
  Res.One &= معلوم;

  واپس Res;
}

معلوم_بٹ معلوم_بٹ::pext(ثابت معلوم_بٹ &Val, ثابت معلوم_بٹ &نقاب) {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = Val.get_ثنائی_ہندسہ_چوڑائی();
  معلوم_بٹ Res(ثنائی_ہندسہ_چوڑائی_2);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.سیٹ_All_Conflict();

  // For each source position I where Mask[I] could be 1, the output position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible output positions widens.
  غیرعلامتی M0 = 0, M1 = 0;
  برائے (غیرعلامتی I = 0; I < ثنائی_ہندسہ_چوڑائی_2; ++I) {
    اگر (!نقاب.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      اگر (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clear_بٹ(M0, M1 + 1);
      اگر (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clear_بٹ(M0, M1 + 1);
    }
    اگر (نقاب.One[I])
      ++M0, ++M1;
    ورنہ اگر (!نقاب.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clear_بٹ(M0, ثنائی_ہندسہ_چوڑائی_2);
  واپس Res;
}

معلوم_بٹ معلوم_بٹ::pdep(ثابت معلوم_بٹ &Val, ثابت معلوم_بٹ &نقاب) {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = Val.get_ثنائی_ہندسہ_چوڑائی();
  معلوم_بٹ Res(ثنائی_ہندسہ_چوڑائی_2);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.سیٹ_All_Conflict();

  // For each output position I where Mask[I] could be 1, the source position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible source positions widens.
  غیرعلامتی M0 = 0, M1 = 0;
  برائے (غیرعلامتی I = 0; I < ثنائی_ہندسہ_چوڑائی_2; ++I) {
    اگر (!نقاب.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clear_ثنائی_ہندسہ(I);
    اگر (!نقاب.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::get_بٹ_سیٹ(ثنائی_ہندسہ_چوڑائی_2, M0, M1 + 1);
      اگر (!Range.is_Subset_کا(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clear_ثنائی_ہندسہ(I);
      اگر (!Range.is_Subset_کا(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clear_ثنائی_ہندسہ(I);
    }
    اگر (نقاب.One[I])
      ++M0, ++M1;
    ورنہ اگر (!نقاب.Zero[I])
      ++M1;
  }
  واپس Res;
}

std::optional<منطقی> معلوم_بٹ::eq(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  اگر (LHS.isConstant() && RHS.isConstant())
    واپس LHS.getConstant() == RHS.getConstant();
  اگر (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    واپس جھوٹ;
  واپس std::nullopt;
}

std::optional<منطقی> معلوم_بٹ::ne(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  اگر (std::optional<منطقی> معلوم_EQ = eq(LHS, RHS))
    واپس !*معلوم_EQ;
  واپس std::nullopt;
}

std::optional<منطقی> معلوم_بٹ::ugt(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  اگر (LHS.get_Max_قدر().ule(RHS.get_Min_قدر()))
    واپس جھوٹ;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  اگر (LHS.get_Min_قدر().ugt(RHS.get_Max_قدر()))
    واپس سچ;
  واپس std::nullopt;
}

std::optional<منطقی> معلوم_بٹ::uge(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  اگر (std::optional<منطقی> IsUGT = ugt(RHS, LHS))
    واپس !*IsUGT;
  واپس std::nullopt;
}

std::optional<منطقی> معلوم_بٹ::ult(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  واپس ugt(RHS, LHS);
}

std::optional<منطقی> معلوم_بٹ::ule(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  واپس uge(RHS, LHS);
}

std::optional<منطقی> معلوم_بٹ::sgt(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  اگر (LHS.get_Signed_Max_قدر().sle(RHS.get_Signed_Min_قدر()))
    واپس جھوٹ;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  اگر (LHS.get_Signed_Min_قدر().sgt(RHS.get_Signed_Max_قدر()))
    واپس سچ;
  واپس std::nullopt;
}

std::optional<منطقی> معلوم_بٹ::sge(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  اگر (std::optional<منطقی> معلوم_SGT = sgt(RHS, LHS))
    واپس !*معلوم_SGT;
  واپس std::nullopt;
}

std::optional<منطقی> معلوم_بٹ::slt(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  واپس sgt(RHS, LHS);
}

std::optional<منطقی> معلوم_بٹ::sle(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  واپس sge(RHS, LHS);
}

معلوم_بٹ معلوم_بٹ::مطلق_قدر(منطقی IntMinIsPoison) ثابت {
  // If the source's MSB is zero then we know the rest of the bits already.
  اگر (isNonNegative())
    واپس *یہ;

  // Absolute value preserves trailing zero count.
  معلوم_بٹ معلوم_Abs(get_ثنائی_ہندسہ_چوڑائی());

  // If the input is negative, then abs(x) == -x.
  اگر (isNegative()) {
    معلوم_بٹ Tmp = *یہ;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    اگر (IntMinIsPoison && (Zero.popcount() + 2) == get_ثنائی_ہندسہ_چوڑائی())
      Tmp.One.سیٹ_ثنائی_ہندسہ(گنتی_Min_Trailing_Zeros());

    معلوم_Abs = compute_For_جوڑو_Sub(
        /*جوڑو*/ جھوٹ, IntMinIsPoison, /*NUW=*/جھوٹ,
        معلوم_بٹ::makeConstant(APInt(get_ثنائی_ہندسہ_چوڑائی(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    اگر (IntMinIsPoison && Tmp.گنتی_Min_Population() == 1 &&
        Tmp.گنتی_Max_Population() != 1) {
      Tmp.One.clear_Sign_ثنائی_ہندسہ();
      Tmp.Zero.سیٹ_Sign_ثنائی_ہندسہ();
      معلوم_Abs.One.سیٹ_بٹ(get_ثنائی_ہندسہ_چوڑائی() - Tmp.گنتی_Min_Leading_Zeros(),
                           get_ثنائی_ہندسہ_چوڑائی() - 1);
    }

  } ورنہ {
    غیرعلامتی MaxTZ = گنتی_Max_Trailing_Zeros();
    غیرعلامتی MinTZ = گنتی_Min_Trailing_Zeros();

    معلوم_Abs.Zero.سیٹ_کم_بٹ(MinTZ);
    // If we know the lowest set 1, then preserve it.
    اگر (MaxTZ == MinTZ && MaxTZ < get_ثنائی_ہندسہ_چوڑائی())
      معلوم_Abs.One.سیٹ_ثنائی_ہندسہ(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    اگر (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_قدر())) {
      معلوم_Abs.One.clear_Sign_ثنائی_ہندسہ();
      معلوم_Abs.Zero.سیٹ_Sign_ثنائی_ہندسہ();
    }
  }

  واپس معلوم_Abs;
}

معلوم_بٹ معلوم_بٹ::reduce_جوڑو(غیرعلامتی عدد_Elts) ثابت {
  اگر (عدد_Elts == 0)
    واپس معلوم_بٹ(get_ثنائی_ہندسہ_چوڑائی());

  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = get_ثنائی_ہندسہ_چوڑائی();
  معلوم_بٹ نتیجہ(ثنائی_ہندسہ_چوڑائی_2);

  اگر (isConstant())
    // If all elements are the same constant, we can simply compute it
    واپس معلوم_بٹ::makeConstant(عدد_Elts * getConstant());

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
  غیرعلامتی گمشدہ_بٹ = Log2_32_Ceil(عدد_Elts);

  اگر (isNonNegative()) {
    غیرعلامتی LeadingZeros = گنتی_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > گمشدہ_بٹ ? LeadingZeros - گمشدہ_بٹ : 0;
    نتیجہ.Zero.سیٹ_بلند_بٹ(LeadingZeros);
  } ورنہ اگر (isNegative()) {
    غیرعلامتی LeadingOnes = گنتی_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > گمشدہ_بٹ ? LeadingOnes - گمشدہ_بٹ : 0;
    نتیجہ.One.سیٹ_بلند_بٹ(LeadingOnes);
  }

  واپس نتیجہ;
}

ساکن معلوم_بٹ compute_For_Sat_جوڑو_Sub(منطقی جوڑو_2, منطقی Signed,
                                     ثابت معلوم_بٹ &LHS,
                                     ثابت معلوم_بٹ &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();

  std::optional<منطقی> حد_سے_تجاوز;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  منطقی MayNegClamp = سچ;
  منطقی MayPosClamp = سچ;
  اگر (Signed) {
    // Easy cases we can rule out any overflow.
    اگر (جوڑو_2 && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      حد_سے_تجاوز = جھوٹ;
    ورنہ اگر (!جوڑو_2 && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      حد_سے_تجاوز = جھوٹ;
    ورنہ {
      // Check if we may overflow. If we can't rule out overflow then check if
      // we can rule out a direction at least.
      معلوم_بٹ UnsignedLHS = LHS;
      معلوم_بٹ UnsignedRHS = RHS;
      // Get version of LHS/RHS with clearer signbit. This allows us to detect
      // how the addition/subtraction might overflow into the signbit. Then
      // using the actual known signbits of LHS/RHS, we can figure out which
      // overflows are/aren't possible.
      UnsignedLHS.One.clear_Sign_ثنائی_ہندسہ();
      UnsignedLHS.Zero.سیٹ_Sign_ثنائی_ہندسہ();
      UnsignedRHS.One.clear_Sign_ثنائی_ہندسہ();
      UnsignedRHS.Zero.سیٹ_Sign_ثنائی_ہندسہ();
      معلوم_بٹ Res =
          معلوم_بٹ::compute_For_جوڑو_Sub(جوڑو_2, /*NSW=*/جھوٹ,
                                      /*NUW=*/جھوٹ, UnsignedLHS, UnsignedRHS);
      اگر (جوڑو_2) {
        اگر (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = جھوٹ;
          // Pos + Pos will overflow with extra signbit.
          اگر (LHS.isNonNegative() && RHS.isNonNegative())
            حد_سے_تجاوز = سچ;
        } ورنہ اگر (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = جھوٹ;
          // Neg + Neg will overflow without extra signbit.
          اگر (LHS.isNegative() && RHS.isNegative())
            حد_سے_تجاوز = سچ;
        }
        // We will never clamp to the opposite sign of N-bit result.
        اگر (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = جھوٹ;
        اگر (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = جھوٹ;
      } ورنہ {
        اگر (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = جھوٹ;
          // Neg - Pos will overflow with extra signbit.
          اگر (LHS.isNegative() && RHS.isNonNegative())
            حد_سے_تجاوز = سچ;
        } ورنہ اگر (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = جھوٹ;
          // Pos - Neg will overflow without extra signbit.
          اگر (LHS.isNonNegative() && RHS.isNegative())
            حد_سے_تجاوز = سچ;
        }
        // We will never clamp to the opposite sign of N-bit result.
        اگر (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = جھوٹ;
        اگر (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = جھوٹ;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    اگر (!MayNegClamp && !MayPosClamp)
      حد_سے_تجاوز = جھوٹ;
  } ورنہ اگر (جوڑو_2) {
    // uadd.sat
    منطقی کا;
    (خالی)LHS.get_Max_قدر().uadd_ov(RHS.get_Max_قدر(), کا);
    اگر (!کا) {
      حد_سے_تجاوز = جھوٹ;
    } ورنہ {
      (خالی)LHS.get_Min_قدر().uadd_ov(RHS.get_Min_قدر(), کا);
      اگر (کا)
        حد_سے_تجاوز = سچ;
    }
  } ورنہ {
    // usub.sat
    منطقی کا;
    (خالی)LHS.get_Min_قدر().usub_ov(RHS.get_Max_قدر(), کا);
    اگر (!کا) {
      حد_سے_تجاوز = جھوٹ;
    } ورنہ {
      (خالی)LHS.get_Max_قدر().usub_ov(RHS.get_Min_قدر(), کا);
      اگر (کا)
        حد_سے_تجاوز = سچ;
    }
  }

  معلوم_بٹ Res = معلوم_بٹ::compute_For_جوڑو_Sub(جوڑو_2, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  اگر (حد_سے_تجاوز) {
    // We know whether or not we overflowed.
    اگر (!(*حد_سے_تجاوز)) {
      // No overflow.
      واپس Res;
    }

    // We overflowed
    APInt C;
    اگر (Signed) {
      // sadd.sat / ssub.sat
      أكد(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_قدر(ثنائی_ہندسہ_چوڑائی_2)
                           : APInt::get_Signed_Max_قدر(ثنائی_ہندسہ_چوڑائی_2);
    } ورنہ اگر (جوڑو_2) {
      // uadd.sat
      C = APInt::get_Max_قدر(ثنائی_ہندسہ_چوڑائی_2);
    } ورنہ {
      // uadd.sat
      C = APInt::get_Min_قدر(ثنائی_ہندسہ_چوڑائی_2);
    }

    Res.One = C;
    Res.Zero = ~C;
    واپس Res;
  }

  // We don't know if we overflowed.
  اگر (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    اگر (MayPosClamp)
      Res.Zero.clear_کم_بٹ(ثنائی_ہندسہ_چوڑائی_2 - 1);
    اگر (MayNegClamp)
      Res.One.clear_کم_بٹ(ثنائی_ہندسہ_چوڑائی_2 - 1);
  } ورنہ اگر (جوڑو_2) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clear_All_بٹ();
  } ورنہ {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clear_All_بٹ();
  }

  واپس Res;
}

معلوم_بٹ معلوم_بٹ::sadd_sat(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  واپس compute_For_Sat_جوڑو_Sub(/*جوڑو*/ سچ, /*Signed*/ سچ, LHS, RHS);
}
معلوم_بٹ معلوم_بٹ::ssub_sat(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  واپس compute_For_Sat_جوڑو_Sub(/*جوڑو*/ جھوٹ, /*Signed*/ سچ, LHS, RHS);
}
معلوم_بٹ معلوم_بٹ::uadd_sat(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  واپس compute_For_Sat_جوڑو_Sub(/*جوڑو*/ سچ, /*Signed*/ جھوٹ, LHS, RHS);
}
معلوم_بٹ معلوم_بٹ::usub_sat(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  واپس compute_For_Sat_جوڑو_Sub(/*جوڑو*/ جھوٹ, /*Signed*/ جھوٹ, LHS, RHS);
}

ساکن معلوم_بٹ avgComputeU(معلوم_بٹ LHS, معلوم_بٹ RHS, منطقی IsCeil) {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  LHS = LHS.zext(ثنائی_ہندسہ_چوڑائی_2 + 1);
  RHS = RHS.zext(ثنائی_ہندسہ_چوڑائی_2 + 1);
  LHS =
      compute_For_جوڑو_حاصل(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.نکالو_بٹ(ثنائی_ہندسہ_چوڑائی_2, 1);
  واپس LHS;
}

معلوم_بٹ معلوم_بٹ::avgFloorS(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  واپس flip_Sign_ثنائی_ہندسہ(avgFloorU(flip_Sign_ثنائی_ہندسہ(LHS), flip_Sign_ثنائی_ہندسہ(RHS)));
}

معلوم_بٹ معلوم_بٹ::avgFloorU(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  واپس avgComputeU(LHS, RHS, /*IsCeil=*/جھوٹ);
}

معلوم_بٹ معلوم_بٹ::avgCeilS(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  واپس flip_Sign_ثنائی_ہندسہ(avgCeilU(flip_Sign_ثنائی_ہندسہ(LHS), flip_Sign_ثنائی_ہندسہ(RHS)));
}

معلوم_بٹ معلوم_بٹ::avgCeilU(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  واپس avgComputeU(LHS, RHS, /*IsCeil=*/سچ);
}

معلوم_بٹ معلوم_بٹ::mul(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS,
                         منطقی NoUndefSelfMultiply) {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  أكد(ثنائی_ہندسہ_چوڑائی_2 == RHS.get_ثنائی_ہندسہ_چوڑائی() && "Operand mismatch");
  أكد((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.get_Max_قدر();
  APInt UMaxRHS = RHS.get_Max_قدر();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  منطقی Has_حد_سے_تجاوز;
  APInt UMax_نتیجہ = UMaxLHS.umul_ov(UMaxRHS, Has_حد_سے_تجاوز);
  غیرعلامتی LeadZ = Has_حد_سے_تجاوز ? 0 : UMax_نتیجہ.countl_zero();

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
  غیرعلامتی Trail_بٹ_معلوم_LHS = (LHS.Zero | LHS.One).countr_one();
  غیرعلامتی Trail_بٹ_معلوم_RHS = (RHS.Zero | RHS.One).countr_one();
  غیرعلامتی TrailZeroLHS = LHS.گنتی_Min_Trailing_Zeros();
  غیرعلامتی TrailZeroRHS = RHS.گنتی_Min_Trailing_Zeros();
  غیرعلامتی TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  غیرعلامتی سب_سے_چھوٹا_عملیہ = std::min(Trail_بٹ_معلوم_LHS - TrailZeroLHS,
                                      Trail_بٹ_معلوم_RHS - TrailZeroRHS);
  غیرعلامتی نتیجہ_بٹ_معلوم = std::min(سب_سے_چھوٹا_عملیہ + TrailZ, ثنائی_ہندسہ_چوڑائی_2);

  // The lower ResultBitsKnown bits of this are known.
  APInt Bottom_معلوم = LHS.One * RHS.One;

  معلوم_بٹ Res(ثنائی_ہندسہ_چوڑائی_2);
  Res.Zero.سیٹ_بلند_بٹ(LeadZ);
  Res.Zero |= (~Bottom_معلوم).get_Lo_بٹ(نتیجہ_بٹ_معلوم);
  Res.One = Bottom_معلوم.get_Lo_بٹ(نتیجہ_بٹ_معلوم);

  اگر (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    غیرعلامتی TwoTZP1 = 2 * TrailZeroLHS + 1;
    اگر (TwoTZP1 < ثنائی_ہندسہ_چوڑائی_2)
      Res.Zero.سیٹ_ثنائی_ہندسہ(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    اگر (TrailZeroLHS < ثنائی_ہندسہ_چوڑائی_2 && LHS.One[TrailZeroLHS]) {
      غیرعلامتی TwoTZP2 = TwoTZP1 + 1;
      اگر (TwoTZP2 < ثنائی_ہندسہ_چوڑائی_2)
        Res.Zero.سیٹ_ثنائی_ہندسہ(TwoTZP2);
    }
  }

  واپس Res;
}

معلوم_بٹ معلوم_بٹ::mulhs(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  أكد(ثنائی_ہندسہ_چوڑائی_2 == RHS.get_ثنائی_ہندسہ_چوڑائی() && "Operand mismatch");
  معلوم_بٹ WideLHS = LHS.sext(2 * ثنائی_ہندسہ_چوڑائی_2);
  معلوم_بٹ WideRHS = RHS.sext(2 * ثنائی_ہندسہ_چوڑائی_2);
  واپس mul(WideLHS, WideRHS).نکالو_بٹ(ثنائی_ہندسہ_چوڑائی_2, ثنائی_ہندسہ_چوڑائی_2);
}

معلوم_بٹ معلوم_بٹ::mulhu(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  أكد(ثنائی_ہندسہ_چوڑائی_2 == RHS.get_ثنائی_ہندسہ_چوڑائی() && "Operand mismatch");
  معلوم_بٹ WideLHS = LHS.zext(2 * ثنائی_ہندسہ_چوڑائی_2);
  معلوم_بٹ WideRHS = RHS.zext(2 * ثنائی_ہندسہ_چوڑائی_2);
  واپس mul(WideLHS, WideRHS).نکالو_بٹ(ثنائی_ہندسہ_چوڑائی_2, ثنائی_ہندسہ_چوڑائی_2);
}

ساکن معلوم_بٹ div_Compute_کم_ثنائی_ہندسہ(معلوم_بٹ معلوم, ثابت معلوم_بٹ &LHS,
                                  ثابت معلوم_بٹ &RHS, منطقی Exact) {

  اگر (!Exact)
    واپس معلوم;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  اگر (LHS.One[0])
    معلوم.One.سیٹ_ثنائی_ہندسہ(0);

  صحیح MinTZ =
      (صحیح)LHS.گنتی_Min_Trailing_Zeros() - (صحیح)RHS.گنتی_Max_Trailing_Zeros();
  صحیح MaxTZ =
      (صحیح)LHS.گنتی_Max_Trailing_Zeros() - (صحیح)RHS.گنتی_Min_Trailing_Zeros();
  اگر (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    معلوم.Zero.سیٹ_کم_بٹ(MinTZ);
    اگر (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      معلوم.One.سیٹ_ثنائی_ہندسہ(MinTZ);
    }
  } ورنہ اگر (MaxTZ < 0) {
    // Poison Result
    معلوم.سیٹ_All_Zero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  اگر (معلوم.hasConflict())
    معلوم.سیٹ_All_Zero();

  واپس معلوم;
}

معلوم_بٹ معلوم_بٹ::sdiv(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS,
                          منطقی Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  اگر (LHS.isNonNegative() && RHS.isNonNegative())
    واپس udiv(LHS, RHS, Exact);

  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);

  اگر (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    معلوم.سیٹ_All_Zero();
    واپس معلوم;
  }

  std::optional<APInt> Res;
  اگر (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_قدر();
    APInt عدد = LHS.get_Signed_Min_قدر();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (عدد.is_Min_Signed_قدر() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_قدر(ثنائی_ہندسہ_چوڑائی_2)
              : عدد.sdiv(Denom);
  } ورنہ اگر (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    اگر (Exact || (-LHS.get_Signed_Max_قدر()).uge(RHS.get_Signed_Max_قدر())) {
      APInt Denom = RHS.get_Signed_Min_قدر();
      APInt عدد = LHS.get_Signed_Min_قدر();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      اگر (Denom.isZero())
        Denom.سیٹ_ثنائی_ہندسہ(RHS.گنتی_Min_Trailing_Zeros());
      Res = عدد.sdiv(Denom);
    }
  } ورنہ اگر (LHS.is_Strictly_مثبت() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    اگر (Exact || LHS.get_Signed_Min_قدر().uge(-RHS.get_Signed_Min_قدر())) {
      APInt Denom = RHS.get_Signed_Max_قدر();
      APInt عدد = LHS.get_Signed_Max_قدر();
      Res = عدد.sdiv(Denom);
    }
  }

  اگر (Res) {
    اگر (Res->isNonNegative()) {
      غیرعلامتی LeadZ = Res->گنتی_Leading_Zeros();
      معلوم.Zero.سیٹ_بلند_بٹ(LeadZ);
    } ورنہ {
      غیرعلامتی LeadO = Res->گنتی_Leading_Ones();
      معلوم.One.سیٹ_بلند_بٹ(LeadO);
    }
  }

  معلوم = div_Compute_کم_ثنائی_ہندسہ(معلوم, LHS, RHS, Exact);
  واپس معلوم;
}

معلوم_بٹ معلوم_بٹ::udiv(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS,
                          منطقی Exact) {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);

  اگر (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    معلوم.سیٹ_All_Zero();
    واپس معلوم;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_قدر();
  اگر (MinDenom.isZero())
    MinDenom.سیٹ_ثنائی_ہندسہ(RHS.گنتی_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_قدر().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_قدر().udiv(RHS.get_Max_قدر());

  معلوم = معلوم_بٹ::makeConstant(MinRes);
  اگر (std::optional<غیرعلامتی> Different_ثنائی_ہندسہ =
          APIntOps::Get_Most_Significant_Different_ثنائی_ہندسہ(MinRes, MaxRes)) {
    معلوم.Zero.clear_کم_بٹ(*Different_ثنائی_ہندسہ + 1);
    معلوم.One.clear_کم_بٹ(*Different_ثنائی_ہندسہ + 1);
  }

  معلوم = div_Compute_کم_ثنائی_ہندسہ(معلوم, LHS, RHS, Exact);

  واپس معلوم;
}

معلوم_بٹ معلوم_بٹ::rem_Get_کم_بٹ(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  اگر (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    غیرعلامتی RHSZeros = RHS.گنتی_Min_Trailing_Zeros();
    APInt نقاب = APInt::get_کم_بٹ_سیٹ(ثنائی_ہندسہ_چوڑائی_2, RHSZeros);
    APInt Ones_نقاب = LHS.One & نقاب;
    APInt Zeros_نقاب = LHS.Zero & نقاب;
    واپس معلوم_بٹ(Zeros_نقاب, Ones_نقاب);
  }
  واپس معلوم_بٹ(ثنائی_ہندسہ_چوڑائی_2);
}

معلوم_بٹ معلوم_بٹ::urem(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  اگر (LHS.get_Max_قدر().ult(RHS.get_Min_قدر()))
    واپس LHS;

  معلوم_بٹ معلوم = rem_Get_کم_بٹ(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_قدر(), RHS.get_Max_قدر() - 1);
  معلوم.Zero.سیٹ_بلند_بٹ(MaxRes.گنتی_Leading_Zeros());
  واپس معلوم;
}

معلوم_بٹ معلوم_بٹ::srem(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  معلوم_بٹ معلوم = rem_Get_کم_بٹ(LHS, RHS);
  اگر (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt کم_بٹ = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    اگر (LHS.isNonNegative() || کم_بٹ.is_Subset_کا(LHS.Zero))
      معلوم.Zero |= ~کم_بٹ;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    اگر (LHS.isNegative() && کم_بٹ.intersects(LHS.One))
      معلوم.One |= ~کم_بٹ;
    واپس معلوم;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  اگر (LHS.isNegative() && معلوم.isNonZero())
    معلوم.One.سیٹ_بلند_بٹ(
        std::max(LHS.گنتی_Min_Leading_Ones(), RHS.گنتی_Min_Sign_بٹ()));
  ورنہ اگر (LHS.isNonNegative())
    معلوم.Zero.سیٹ_بلند_بٹ(
        std::max(LHS.گنتی_Min_Leading_Zeros(), RHS.گنتی_Min_Sign_بٹ()));
  واپس معلوم;
}

معلوم_بٹ &معلوم_بٹ::عامل&=(ثابت معلوم_بٹ &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  واپس *یہ;
}

معلوم_بٹ &معلوم_بٹ::عامل|=(ثابت معلوم_بٹ &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  واپس *یہ;
}

معلوم_بٹ &معلوم_بٹ::عامل^=(ثابت معلوم_بٹ &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  واپس *یہ;
}

معلوم_بٹ معلوم_بٹ::blsi() ثابت {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = get_ثنائی_ہندسہ_چوڑائی();
  معلوم_بٹ معلوم(Zero, APInt(ثنائی_ہندسہ_چوڑائی_2, 0));
  غیرعلامتی Max = گنتی_Max_Trailing_Zeros();
  معلوم.Zero.سیٹ_بٹ_From(std::min(Max + 1, ثنائی_ہندسہ_چوڑائی_2));
  غیرعلامتی Min = گنتی_Min_Trailing_Zeros();
  اگر (Max == Min && Max < ثنائی_ہندسہ_چوڑائی_2)
    معلوم.One.سیٹ_ثنائی_ہندسہ(Max);
  واپس معلوم;
}

معلوم_بٹ معلوم_بٹ::blsmsk() ثابت {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = get_ثنائی_ہندسہ_چوڑائی();
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);
  غیرعلامتی Max = گنتی_Max_Trailing_Zeros();
  معلوم.Zero.سیٹ_بٹ_From(std::min(Max + 1, ثنائی_ہندسہ_چوڑائی_2));
  غیرعلامتی Min = گنتی_Min_Trailing_Zeros();
  معلوم.One.سیٹ_کم_بٹ(std::min(Min + 1, ثنائی_ہندسہ_چوڑائی_2));
  واپس معلوم;
}

خالی معلوم_بٹ::print(raw_ostream &OS) ثابت {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = get_ثنائی_ہندسہ_چوڑائی();
  برائے (غیرعلامتی I = 0; I < ثنائی_ہندسہ_چوڑائی_2; ++I) {
    غیرعلامتی N = ثنائی_ہندسہ_چوڑائی_2 - I - 1;
    اگر (Zero[N] && One[N])
      OS << "!";
    ورنہ اگر (Zero[N])
      OS << "0";
    ورنہ اگر (One[N])
      OS << "1";
    ورنہ
      OS << "?";
  }
}

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طریقہ خالی معلوم_بٹ::dump() ثابت {
  print(dbgs());
  dbgs() << "\n";
}
#اگر_ختم

غیرعلامتی llvm::Sign_بٹ_Ops::rot(غیرعلامتی Src_Sign_بٹ, غیرعلامتی ثنائی_ہندسہ_چوڑائی_2,
                                std::optional<APInt> RotAmt,
                                منطقی Is_Rotate_دایاں) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  اگر (Src_Sign_بٹ == ثنائی_ہندسہ_چوڑائی_2)
    واپس ثنائی_ہندسہ_چوڑائی_2;

  اگر (!RotAmt)
    واپس 1;

  غیرعلامتی Amt = RotAmt->urem(ثنائی_ہندسہ_چوڑائی_2);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  اگر (Is_Rotate_دایاں)
    Amt = (ثنائی_ہندسہ_چوڑائی_2 - Amt) % ثنائی_ہندسہ_چوڑائی_2;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  اگر (Src_Sign_بٹ > Amt + 1)
    واپس Src_Sign_بٹ - Amt;
  واپس 1;
}
