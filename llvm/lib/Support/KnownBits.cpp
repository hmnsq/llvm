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

ورتوں نطاق llvm;

معلوم_بٹ معلوم_بٹ::flip_Sign_ثنائی_ہندسہ(ثابت معلوم_بٹ &Val) {
  بنانشانی Sign_ثنائی_ہندسہ_Position = Val.get_ثنائی_ہندسہ_چوڑائی() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.سیٹ_ثنائی_ہندسہ_Val(Sign_ثنائی_ہندسہ_Position, Val.One[Sign_ثنائی_ہندسہ_Position]);
  One.سیٹ_ثنائی_ہندسہ_Val(Sign_ثنائی_ہندسہ_Position, Val.Zero[Sign_ثنائی_ہندسہ_Position]);
  أرجع معلوم_بٹ(Zero, One);
}

ساكن معلوم_بٹ compute_For_جوڑو_حاصل(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS,
                                    منطقي حاصل_Zero, منطقي حاصل_One) {

  APInt Possible_جوڑ_Zero = LHS.get_Max_قدر() + RHS.get_Max_قدر() + !حاصل_Zero;
  APInt Possible_جوڑ_One = LHS.get_Min_قدر() + RHS.get_Min_قدر() + حاصل_One;

  // Compute known bits of the carry.
  APInt حاصل_معلوم_Zero = ~(Possible_جوڑ_Zero ^ LHS.Zero ^ RHS.Zero);
  APInt حاصل_معلوم_One = Possible_جوڑ_One ^ LHS.One ^ RHS.One;

  // Compute set of known bits (where all three relevant bits are known).
  APInt LHSKnownUnion = LHS.Zero | LHS.One;
  APInt RHSKnownUnion = RHS.Zero | RHS.One;
  APInt حاصل_معلوم_Union = std::move(حاصل_معلوم_Zero) | حاصل_معلوم_One;
  APInt معلوم = std::move(LHSKnownUnion) & RHSKnownUnion & حاصل_معلوم_Union;

  // Compute known bits of the result.
  معلوم_بٹ معلوم_باہرلا;
  معلوم_باہرلا.Zero = ~std::move(Possible_جوڑ_Zero) & معلوم;
  معلوم_باہرلا.One = std::move(Possible_جوڑ_One) & معلوم;
  أرجع معلوم_باہرلا;
}

معلوم_بٹ معلوم_بٹ::compute_For_جوڑو_حاصل(
    ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS, ثابت معلوم_بٹ &حاصل_2) {
  أكد(حاصل_2.get_ثنائی_ہندسہ_چوڑائی() == 1 && "Carry must be 1-bit");
  أرجع ::compute_For_جوڑو_حاصل(
      LHS, RHS, حاصل_2.Zero.get_Bool_قدر(), حاصل_2.One.get_Bool_قدر());
}

معلوم_بٹ معلوم_بٹ::compute_For_جوڑو_Sub(منطقي جوڑو_2, منطقي NSW, منطقي NUW,
                                      ثابت معلوم_بٹ &LHS,
                                      ثابت معلوم_بٹ &RHS) {
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  معلوم_بٹ معلوم_باہرلا(ثنائی_ہندسہ_چوڑائی_2);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  إذا (LHS.isUnknown() && RHS.isUnknown())
    أرجع معلوم_باہرلا;

  إذا (!LHS.isUnknown() && !RHS.isUnknown()) {
    إذا (جوڑو_2) {
      // Sum = LHS + RHS + 0
      معلوم_باہرلا = ::compute_For_جوڑو_حاصل(LHS, RHS, /*CarryZero=*/سچ,
                                      /*CarryOne=*/خطأ);
    } وإلا {
      // Sum = LHS + ~RHS + 1
      معلوم_بٹ NotRHS = RHS;
      std::وٹاندرا(NotRHS.Zero, NotRHS.One);
      معلوم_باہرلا = ::compute_For_جوڑو_حاصل(LHS, NotRHS, /*CarryZero=*/خطأ,
                                      /*CarryOne=*/سچ);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  إذا (NUW) {
    إذا (جوڑو_2) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_قدر().uadd_sat(RHS.get_Min_قدر());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      إذا (NSW) {
        بنانشانی عدد_بٹ_2 = MinVal.trunc(ثنائی_ہندسہ_چوڑائی_2 - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        معلوم_باہرلا.One.سیٹ_بٹ(ثنائی_ہندسہ_چوڑائی_2 - 1 - عدد_بٹ_2, ثنائی_ہندسہ_چوڑائی_2 - 1);
      }
      معلوم_باہرلا.One.سیٹ_بلند_بٹ(MinVal.countl_one());
    } وإلا {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_قدر().usub_sat(RHS.get_Min_قدر());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      إذا (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        بنانشانی عدد_بٹ_2 = MaxVal.trunc(ثنائی_ہندسہ_چوڑائی_2 - 1).countl_zero();
        معلوم_باہرلا.Zero.سیٹ_بٹ(ثنائی_ہندسہ_چوڑائی_2 - 1 - عدد_بٹ_2, ثنائی_ہندسہ_چوڑائی_2 - 1);
      }
      معلوم_باہرلا.Zero.سیٹ_بلند_بٹ(MaxVal.countl_zero());
    }
  }

  إذا (NSW) {
    APInt MinVal;
    APInt MaxVal;
    إذا (جوڑو_2) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_قدر().sadd_sat(RHS.get_Signed_Min_قدر());
      MaxVal = LHS.get_Signed_Max_قدر().sadd_sat(RHS.get_Signed_Max_قدر());
    } وإلا {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_قدر().ssub_sat(RHS.get_Signed_Max_قدر());
      MaxVal = LHS.get_Signed_Max_قدر().ssub_sat(RHS.get_Signed_Min_قدر());
    }
    إذا (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      بنانشانی عدد_بٹ_2 = MinVal.trunc(ثنائی_ہندسہ_چوڑائی_2 - 1).countl_one();
      معلوم_باہرلا.One.سیٹ_بٹ(ثنائی_ہندسہ_چوڑائی_2 - 1 - عدد_بٹ_2, ثنائی_ہندسہ_چوڑائی_2 - 1);
      معلوم_باہرلا.Zero.سیٹ_Sign_ثنائی_ہندسہ();
    }
    إذا (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      بنانشانی عدد_بٹ_2 = MaxVal.trunc(ثنائی_ہندسہ_چوڑائی_2 - 1).countl_zero();
      معلوم_باہرلا.Zero.سیٹ_بٹ(ثنائی_ہندسہ_چوڑائی_2 - 1 - عدد_بٹ_2, ثنائی_ہندسہ_چوڑائی_2 - 1);
      معلوم_باہرلا.One.سیٹ_Sign_ثنائی_ہندسہ();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  إذا (معلوم_باہرلا.hasConflict())
    معلوم_باہرلا.سیٹ_All_Zero();
  أرجع معلوم_باہرلا;
}

معلوم_بٹ معلوم_بٹ::computeForSubBorrow(ثابت معلوم_بٹ &LHS, معلوم_بٹ RHS,
                                         ثابت معلوم_بٹ &Borrow) {
  أكد(Borrow.get_ثنائی_ہندسہ_چوڑائی() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::وٹاندرا(RHS.Zero, RHS.One);
  أرجع ::compute_For_جوڑو_حاصل(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_قدر(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_قدر());
}

معلوم_بٹ معلوم_بٹ::truncSSat(بنانشانی ثنائی_ہندسہ_چوڑائی_2) ثابت {
  بنانشانی داخلہ_بٹ = get_ثنائی_ہندسہ_چوڑائی();
  APInt Min_وچ_Range = APInt::get_Signed_Min_قدر(ثنائی_ہندسہ_چوڑائی_2).sext(داخلہ_بٹ);
  APInt Max_وچ_Range = APInt::get_Signed_Max_قدر(ثنائی_ہندسہ_چوڑائی_2).sext(داخلہ_بٹ);
  APInt داخلہ_Min = get_Signed_Min_قدر();
  APInt داخلہ_Max = get_Signed_Max_قدر();
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);

  // Case 1: All values fit - just truncate
  إذا (داخلہ_Min.sge(Min_وچ_Range) && داخلہ_Max.sle(Max_وچ_Range)) {
    معلوم = trunc(ثنائی_ہندسہ_چوڑائی_2);
  }
  // Case 2: All saturate to min
  وإلا إذا (داخلہ_Max.slt(Min_وچ_Range)) {
    معلوم = معلوم_بٹ::makeConstant(APInt::get_Signed_Min_قدر(ثنائی_ہندسہ_چوڑائی_2));
  }
  // Case 3: All saturate to max
  وإلا إذا (داخلہ_Min.sgt(Max_وچ_Range)) {
    معلوم = معلوم_بٹ::makeConstant(APInt::get_Signed_Max_قدر(ثنائی_ہندسہ_چوڑائی_2));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  وإلا إذا (داخلہ_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    معلوم.Zero = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
    معلوم.Zero.سیٹ_ثنائی_ہندسہ(ثنائی_ہندسہ_چوڑائی_2 - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    معلوم.One = One.trunc(ثنائی_ہندسہ_چوڑائی_2);
    معلوم.One.clear_ثنائی_ہندسہ(ثنائی_ہندسہ_چوڑائی_2 - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  وإلا إذا (داخلہ_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    معلوم.Zero = Zero.trunc(ثنائی_ہندسہ_چوڑائی_2);
    معلوم.Zero.clear_ثنائی_ہندسہ(ثنائی_ہندسہ_چوڑائی_2 - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    معلوم.One = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
    معلوم.One.سیٹ_ثنائی_ہندسہ(ثنائی_ہندسہ_چوڑائی_2 - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  وإلا {
    // Output: min saturation, truncated, or max saturation
    APInt داخلہ_Zero_ہیٹھلا = Zero.trunc(ثنائی_ہندسہ_چوڑائی_2);
    APInt داخلہ_One_ہیٹھلا = One.trunc(ثنائی_ہندسہ_چوڑائی_2);
    APInt MinSat = APInt::get_Signed_Min_قدر(ثنائی_ہندسہ_چوڑائی_2);
    APInt MaxSat = APInt::get_Signed_Max_قدر(ثنائی_ہندسہ_چوڑائی_2);
    معلوم_بٹ MinSatKB = معلوم_بٹ::makeConstant(MinSat);
    معلوم_بٹ MaxSatKB = معلوم_بٹ::makeConstant(MaxSat);
    إذا (داخلہ_Max.sle(Max_وچ_Range)) {
      // Positive values fit, only negatives saturate to min
      معلوم.Zero = داخلہ_Zero_ہیٹھلا & MinSatKB.Zero;
      معلوم.One = داخلہ_One_ہیٹھلا & MinSatKB.One;
    } وإلا إذا (داخلہ_Min.sge(Min_وچ_Range)) {
      // Negative values fit, only positives saturate to max
      معلوم.Zero = داخلہ_Zero_ہیٹھلا & MaxSatKB.Zero;
      معلوم.One = داخلہ_One_ہیٹھلا & MaxSatKB.One;
    } وإلا {
      // Both positive and negative values might saturate
      معلوم.Zero = داخلہ_Zero_ہیٹھلا & MinSatKB.Zero & MaxSatKB.Zero;
      معلوم.One = داخلہ_One_ہیٹھلا & MinSatKB.One & MaxSatKB.One;
    }
  }
  أرجع معلوم;
}

معلوم_بٹ معلوم_بٹ::truncSSatU(بنانشانی ثنائی_ہندسہ_چوڑائی_2) ثابت {
  بنانشانی داخلہ_بٹ = get_ثنائی_ہندسہ_چوڑائی();
  APInt Max_وچ_Range = APInt::getAllOnes(ثنائی_ہندسہ_چوڑائی_2).zext(داخلہ_بٹ);
  APInt داخلہ_Min = get_Signed_Min_قدر();
  APInt داخلہ_Max = get_Signed_Max_قدر();
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);

  إذا (isNegative()) {
    معلوم.سیٹ_All_Zero();
  } وإلا إذا (داخلہ_Min.isNonNegative() && داخلہ_Max.ule(Max_وچ_Range)) {
    معلوم = trunc(ثنائی_ہندسہ_چوڑائی_2);
  } وإلا إذا (داخلہ_Min.isNonNegative() && داخلہ_Min.ugt(Max_وچ_Range)) {
    معلوم.سیٹ_All_Ones();
  } وإلا إذا (داخلہ_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    معلوم.Zero = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
    // Common one bits: bits that are 1 in input stay 1
    معلوم.One = One.trunc(ثنائی_ہندسہ_چوڑائی_2);
  } وإلا {
    // Mixed: sign bit unknown
    إذا (داخلہ_Max.ule(Max_وچ_Range)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      معلوم.Zero = Zero.trunc(ثنائی_ہندسہ_چوڑائی_2);
      معلوم.One = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
    } وإلا {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      معلوم.Zero = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
      معلوم.One = APInt(ثنائی_ہندسہ_چوڑائی_2, 0);
    }
  }
  أرجع معلوم;
}

معلوم_بٹ معلوم_بٹ::truncUSat(بنانشانی ثنائی_ہندسہ_چوڑائی_2) ثابت {
  بنانشانی داخلہ_بٹ = get_ثنائی_ہندسہ_چوڑائی();
  APInt Max_وچ_Range = APInt::get_کم_بٹ_سیٹ(داخلہ_بٹ, ثنائی_ہندسہ_چوڑائی_2);
  APInt داخلہ_Max = get_Max_قدر();
  APInt داخلہ_Min = get_Min_قدر();
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);

  إذا (داخلہ_Max.ule(Max_وچ_Range)) {
    معلوم = trunc(ثنائی_ہندسہ_چوڑائی_2);
  } وإلا إذا (داخلہ_Min.ugt(Max_وچ_Range)) {
    معلوم.سیٹ_All_Ones();
  } وإلا {
    معلوم.resetAll();
    معلوم.One = One.trunc(ثنائی_ہندسہ_چوڑائی_2);
  }
  أرجع معلوم;
}

معلوم_بٹ معلوم_بٹ::sext_وچ_Reg(بنانشانی Src_ثنائی_ہندسہ_چوڑائی) ثابت {
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = get_ثنائی_ہندسہ_چوڑائی();
  أكد(0 < Src_ثنائی_ہندسہ_چوڑائی && Src_ثنائی_ہندسہ_چوڑائی <= ثنائی_ہندسہ_چوڑائی_2 &&
         "Illegal sext-in-register");

  إذا (Src_ثنائی_ہندسہ_چوڑائی == ثنائی_ہندسہ_چوڑائی_2)
    أرجع *هذا;

  بنانشانی Ext_بٹ = ثنائی_ہندسہ_چوڑائی_2 - Src_ثنائی_ہندسہ_چوڑائی;
  معلوم_بٹ نتیجہ;
  نتیجہ.One = One << Ext_بٹ;
  نتیجہ.Zero = Zero << Ext_بٹ;
  نتیجہ.One.ashr_وچ_رکھو(Ext_بٹ);
  نتیجہ.Zero.ashr_وچ_رکھو(Ext_بٹ);
  أرجع نتیجہ;
}

معلوم_بٹ معلوم_بٹ::makeGE(ثابت APInt &Val) ثابت {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  بنانشانی N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_کم_بٹ(get_ثنائی_ہندسہ_چوڑائی() - N);
  أرجع معلوم_بٹ(Zero, One | MaskedVal);
}

معلوم_بٹ معلوم_بٹ::umax(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  إذا (LHS.get_Min_قدر().uge(RHS.get_Max_قدر()))
    أرجع LHS;
  إذا (RHS.get_Min_قدر().uge(LHS.get_Max_قدر()))
    أرجع RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  معلوم_بٹ L = LHS.makeGE(RHS.get_Min_قدر());
  معلوم_بٹ R = RHS.makeGE(LHS.get_Min_قدر());
  أرجع L.intersectWith(R);
}

معلوم_بٹ معلوم_بٹ::umin(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  خودکار Flip = [](ثابت معلوم_بٹ &Val) { أرجع معلوم_بٹ(Val.One, Val.Zero); };
  أرجع Flip(umax(Flip(LHS), Flip(RHS)));
}

معلوم_بٹ معلوم_بٹ::smax(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  أرجع flip_Sign_ثنائی_ہندسہ(umax(flip_Sign_ثنائی_ہندسہ(LHS), flip_Sign_ثنائی_ہندسہ(RHS)));
}

معلوم_بٹ معلوم_بٹ::smin(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  خودکار Flip = [](ثابت معلوم_بٹ &Val) {
    بنانشانی Sign_ثنائی_ہندسہ_Position = Val.get_ثنائی_ہندسہ_چوڑائی() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.سیٹ_ثنائی_ہندسہ_Val(Sign_ثنائی_ہندسہ_Position, Val.Zero[Sign_ثنائی_ہندسہ_Position]);
    One.سیٹ_ثنائی_ہندسہ_Val(Sign_ثنائی_ہندسہ_Position, Val.One[Sign_ثنائی_ہندسہ_Position]);
    أرجع معلوم_بٹ(Zero, One);
  };
  أرجع Flip(umax(Flip(LHS), Flip(RHS)));
}

معلوم_بٹ معلوم_بٹ::abdu(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  إذا (LHS.get_Min_قدر().uge(RHS.get_Max_قدر()))
    أرجع compute_For_جوڑو_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, LHS,
                            RHS);
  إذا (RHS.get_Min_قدر().uge(LHS.get_Max_قدر()))
    أرجع compute_For_جوڑو_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  معلوم_بٹ Diff0 =
      compute_For_جوڑو_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/سچ, LHS, RHS);
  معلوم_بٹ Diff1 =
      compute_For_جوڑو_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/سچ, RHS, LHS);
  أرجع Diff0.intersectWith(Diff1);
}

معلوم_بٹ معلوم_بٹ::abds(معلوم_بٹ LHS, معلوم_بٹ RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  إذا (LHS.get_Signed_Min_قدر().sge(RHS.get_Signed_Max_قدر()))
    أرجع compute_For_جوڑو_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, LHS,
                            RHS);
  إذا (RHS.get_Signed_Min_قدر().sge(LHS.get_Signed_Max_قدر()))
    أرجع compute_For_جوڑو_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  بنانشانی Sign_ثنائی_ہندسہ_Position = LHS.get_ثنائی_ہندسہ_چوڑائی() - 1;
  لكل (خودکار Arg : {&LHS, &RHS}) {
    منطقي Tmp = Arg->Zero[Sign_ثنائی_ہندسہ_Position];
    Arg->Zero.سیٹ_ثنائی_ہندسہ_Val(Sign_ثنائی_ہندسہ_Position, Arg->One[Sign_ثنائی_ہندسہ_Position]);
    Arg->One.سیٹ_ثنائی_ہندسہ_Val(Sign_ثنائی_ہندسہ_Position, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  معلوم_بٹ Diff0 =
      compute_For_جوڑو_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/سچ, LHS, RHS);
  معلوم_بٹ Diff1 =
      compute_For_جوڑو_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/سچ, RHS, LHS);
  أرجع Diff0.intersectWith(Diff1);
}

ساكن بنانشانی get_Max_کھسکاؤ_Amount(ثابت APInt &Max_قدر, بنانشانی ثنائی_ہندسہ_چوڑائی_2) {
  إذا (isPowerOf2_32(ثنائی_ہندسہ_چوڑائی_2))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    أرجع Max_قدر.کڈھو_بٹ_As_ZExt_قدر(
        std::min(Log2_32(ثنائی_ہندسہ_چوڑائی_2), Max_قدر.get_ثنائی_ہندسہ_چوڑائی()), 0);
  // This is only an approximate upper bound.
  أرجع Max_قدر.get_Limited_قدر(ثنائی_ہندسہ_چوڑائی_2 - 1);
}

معلوم_بٹ معلوم_بٹ::shl(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS, منطقي NUW,
                         منطقي NSW, منطقي ShAmtNonZero) {
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  خودکار کھسکاؤ_By_Const = [&](ثابت معلوم_بٹ &LHS, بنانشانی کھسکاؤ_Amt_2) {
    معلوم_بٹ معلوم;
    منطقي Shifted_باہرلا_Zero, Shifted_باہرلا_One;
    معلوم.Zero = LHS.Zero.ushl_ov(کھسکاؤ_Amt_2, Shifted_باہرلا_Zero);
    معلوم.Zero.سیٹ_کم_بٹ(کھسکاؤ_Amt_2);
    معلوم.One = LHS.One.ushl_ov(کھسکاؤ_Amt_2, Shifted_باہرلا_One);

    // All cases returning poison have been handled by MaxShiftAmount already.
    إذا (NSW) {
      إذا (NUW && کھسکاؤ_Amt_2 != 0)
        // NUW means we can assume anything shifted out was a zero.
        Shifted_باہرلا_Zero = سچ;

      إذا (Shifted_باہرلا_Zero)
        معلوم.makeNonNegative();
      وإلا إذا (Shifted_باہرلا_One)
        معلوم.makeNegative();
    }
    أرجع معلوم;
  };

  // Fast path for a common case when LHS is completely unknown.
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);
  بنانشانی Min_کھسکاؤ_Amount = RHS.get_Min_قدر().get_Limited_قدر(ثنائی_ہندسہ_چوڑائی_2);
  إذا (Min_کھسکاؤ_Amount == 0 && ShAmtNonZero)
    Min_کھسکاؤ_Amount = 1;
  إذا (LHS.isUnknown()) {
    معلوم.Zero.سیٹ_کم_بٹ(Min_کھسکاؤ_Amount);
    إذا (NUW && NSW && Min_کھسکاؤ_Amount != 0)
      معلوم.makeNonNegative();
    أرجع معلوم;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_قدر = RHS.get_Max_قدر();
  بنانشانی Max_کھسکاؤ_Amount = get_Max_کھسکاؤ_Amount(Max_قدر, ثنائی_ہندسہ_چوڑائی_2);
  إذا (NUW && NSW)
    Max_کھسکاؤ_Amount = std::min(Max_کھسکاؤ_Amount, LHS.گنتی_Max_Leading_Zeros() - 1);
  إذا (NUW)
    Max_کھسکاؤ_Amount = std::min(Max_کھسکاؤ_Amount, LHS.گنتی_Max_Leading_Zeros());
  إذا (NSW)
    Max_کھسکاؤ_Amount = std::min(
        Max_کھسکاؤ_Amount,
        std::max(LHS.گنتی_Max_Leading_Zeros(), LHS.گنتی_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  إذا (Min_کھسکاؤ_Amount == 0 && Max_کھسکاؤ_Amount == ثنائی_ہندسہ_چوڑائی_2 - 1 &&
      isPowerOf2_32(ثنائی_ہندسہ_چوڑائی_2)) {
    معلوم.Zero.سیٹ_کم_بٹ(LHS.گنتی_Min_Trailing_Zeros());
    إذا (LHS.isAllOnes())
      معلوم.One.سیٹ_Sign_ثنائی_ہندسہ();
    إذا (NSW) {
      إذا (LHS.isNonNegative())
        معلوم.makeNonNegative();
      إذا (LHS.isNegative())
        معلوم.makeNegative();
    }
    أرجع معلوم;
  }

  // Find the common bits from all possible shifts.
  بنانشانی کھسکاؤ_Amt_Zero_نقاب = RHS.Zero.zextOrTrunc(32).get_ZExt_قدر();
  بنانشانی کھسکاؤ_Amt_One_نقاب = RHS.One.zextOrTrunc(32).get_ZExt_قدر();
  معلوم.سیٹ_All_Conflict();
  لكل (بنانشانی کھسکاؤ_Amt_2 = Min_کھسکاؤ_Amount; کھسکاؤ_Amt_2 <= Max_کھسکاؤ_Amount;
       ++کھسکاؤ_Amt_2) {
    // Skip if the shift amount is impossible.
    إذا ((کھسکاؤ_Amt_Zero_نقاب & کھسکاؤ_Amt_2) != 0 ||
        (کھسکاؤ_Amt_One_نقاب | کھسکاؤ_Amt_2) != کھسکاؤ_Amt_2)
      استمر;
    معلوم = معلوم.intersectWith(کھسکاؤ_By_Const(LHS, کھسکاؤ_Amt_2));
    إذا (معلوم.isUnknown())
      اكسر;
  }

  // All shift amounts may result in poison.
  إذا (معلوم.hasConflict())
    معلوم.سیٹ_All_Zero();
  أرجع معلوم;
}

معلوم_بٹ معلوم_بٹ::lshr(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS,
                          منطقي ShAmtNonZero, منطقي Exact) {
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  خودکار کھسکاؤ_By_Const = [&](ثابت معلوم_بٹ &LHS, بنانشانی کھسکاؤ_Amt_2) {
    معلوم_بٹ معلوم = LHS;
    معلوم >>= کھسکاؤ_Amt_2;
    // High bits are known zero.
    معلوم.Zero.سیٹ_بلند_بٹ(کھسکاؤ_Amt_2);
    أرجع معلوم;
  };

  // Fast path for a common case when LHS is completely unknown.
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);
  بنانشانی Min_کھسکاؤ_Amount = RHS.get_Min_قدر().get_Limited_قدر(ثنائی_ہندسہ_چوڑائی_2);
  إذا (Min_کھسکاؤ_Amount == 0 && ShAmtNonZero)
    Min_کھسکاؤ_Amount = 1;
  إذا (LHS.isUnknown()) {
    معلوم.Zero.سیٹ_بلند_بٹ(Min_کھسکاؤ_Amount);
    أرجع معلوم;
  }

  // Find the common bits from all possible shifts.
  APInt Max_قدر = RHS.get_Max_قدر();
  بنانشانی Max_کھسکاؤ_Amount = get_Max_کھسکاؤ_Amount(Max_قدر, ثنائی_ہندسہ_چوڑائی_2);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  إذا (Exact) {
    بنانشانی پہلا_One = LHS.گنتی_Max_Trailing_Zeros();
    إذا (پہلا_One < Min_کھسکاؤ_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      معلوم.سیٹ_All_Zero();
      أرجع معلوم;
    }
    Max_کھسکاؤ_Amount = std::min(Max_کھسکاؤ_Amount, پہلا_One);
  }

  بنانشانی کھسکاؤ_Amt_Zero_نقاب = RHS.Zero.zextOrTrunc(32).get_ZExt_قدر();
  بنانشانی کھسکاؤ_Amt_One_نقاب = RHS.One.zextOrTrunc(32).get_ZExt_قدر();
  معلوم.سیٹ_All_Conflict();
  لكل (بنانشانی کھسکاؤ_Amt_2 = Min_کھسکاؤ_Amount; کھسکاؤ_Amt_2 <= Max_کھسکاؤ_Amount;
       ++کھسکاؤ_Amt_2) {
    // Skip if the shift amount is impossible.
    إذا ((کھسکاؤ_Amt_Zero_نقاب & کھسکاؤ_Amt_2) != 0 ||
        (کھسکاؤ_Amt_One_نقاب | کھسکاؤ_Amt_2) != کھسکاؤ_Amt_2)
      استمر;
    معلوم = معلوم.intersectWith(کھسکاؤ_By_Const(LHS, کھسکاؤ_Amt_2));
    إذا (معلوم.isUnknown())
      اكسر;
  }

  // All shift amounts may result in poison.
  إذا (معلوم.hasConflict())
    معلوم.سیٹ_All_Zero();
  أرجع معلوم;
}

معلوم_بٹ معلوم_بٹ::ashr(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS,
                          منطقي ShAmtNonZero, منطقي Exact) {
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  خودکار کھسکاؤ_By_Const = [&](ثابت معلوم_بٹ &LHS, بنانشانی کھسکاؤ_Amt_2) {
    معلوم_بٹ معلوم = LHS;
    معلوم.Zero.ashr_وچ_رکھو(کھسکاؤ_Amt_2);
    معلوم.One.ashr_وچ_رکھو(کھسکاؤ_Amt_2);
    أرجع معلوم;
  };

  // Fast path for a common case when LHS is completely unknown.
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);
  بنانشانی Min_کھسکاؤ_Amount = RHS.get_Min_قدر().get_Limited_قدر(ثنائی_ہندسہ_چوڑائی_2);
  إذا (Min_کھسکاؤ_Amount == 0 && ShAmtNonZero)
    Min_کھسکاؤ_Amount = 1;
  إذا (LHS.isUnknown()) {
    إذا (Min_کھسکاؤ_Amount == ثنائی_ہندسہ_چوڑائی_2) {
      // Always poison. Return zero because we don't like returning conflict.
      معلوم.سیٹ_All_Zero();
      أرجع معلوم;
    }
    أرجع معلوم;
  }

  // Find the common bits from all possible shifts.
  APInt Max_قدر = RHS.get_Max_قدر();
  بنانشانی Max_کھسکاؤ_Amount = get_Max_کھسکاؤ_Amount(Max_قدر, ثنائی_ہندسہ_چوڑائی_2);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  إذا (Exact) {
    بنانشانی پہلا_One = LHS.گنتی_Max_Trailing_Zeros();
    إذا (پہلا_One < Min_کھسکاؤ_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      معلوم.سیٹ_All_Zero();
      أرجع معلوم;
    }
    Max_کھسکاؤ_Amount = std::min(Max_کھسکاؤ_Amount, پہلا_One);
  }

  بنانشانی کھسکاؤ_Amt_Zero_نقاب = RHS.Zero.zextOrTrunc(32).get_ZExt_قدر();
  بنانشانی کھسکاؤ_Amt_One_نقاب = RHS.One.zextOrTrunc(32).get_ZExt_قدر();
  معلوم.سیٹ_All_Conflict();
  لكل (بنانشانی کھسکاؤ_Amt_2 = Min_کھسکاؤ_Amount; کھسکاؤ_Amt_2 <= Max_کھسکاؤ_Amount;
      ++کھسکاؤ_Amt_2) {
    // Skip if the shift amount is impossible.
    إذا ((کھسکاؤ_Amt_Zero_نقاب & کھسکاؤ_Amt_2) != 0 ||
        (کھسکاؤ_Amt_One_نقاب | کھسکاؤ_Amt_2) != کھسکاؤ_Amt_2)
      استمر;
    معلوم = معلوم.intersectWith(کھسکاؤ_By_Const(LHS, کھسکاؤ_Amt_2));
    إذا (معلوم.isUnknown())
      اكسر;
  }

  // All shift amounts may result in poison.
  إذا (معلوم.hasConflict())
    معلوم.سیٹ_All_Zero();
  أرجع معلوم;
}

معلوم_بٹ معلوم_بٹ::fshl(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS,
                          ثابت APInt &Amt) {
  أرجع معلوم_بٹ(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

معلوم_بٹ معلوم_بٹ::fshr(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS,
                          ثابت APInt &Amt) {
  أرجع معلوم_بٹ(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

معلوم_بٹ معلوم_بٹ::clmul(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  معلوم_بٹ Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_قدر(), RHS.get_Min_قدر()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  خودکار ClMulOr = [](ثابت APInt &LHS, ثابت APInt &RHS) {
    بنانشانی BW = LHS.get_ثنائی_ہندسہ_چوڑائی();
    أكد(BW == RHS.get_ثنائی_ہندسہ_چوڑائی() && "Operand mismatch");
    APInt نتیجہ(BW, 0);
    لكل (بنانشانی I :
         seq(std::min(RHS.get_سرگرم_بٹ(), BW - LHS.countr_zero())))
      إذا (RHS[I])
        نتیجہ |= LHS << I;
    أرجع نتیجہ;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt معلوم = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= معلوم;
  Res.One &= معلوم;

  أرجع Res;
}

معلوم_بٹ معلوم_بٹ::pext(ثابت معلوم_بٹ &Val, ثابت معلوم_بٹ &نقاب) {
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = Val.get_ثنائی_ہندسہ_چوڑائی();
  معلوم_بٹ Res(ثنائی_ہندسہ_چوڑائی_2);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.سیٹ_All_Conflict();

  // For each source position I where Mask[I] could be 1, the output position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible output positions widens.
  بنانشانی M0 = 0, M1 = 0;
  لكل (بنانشانی I = 0; I < ثنائی_ہندسہ_چوڑائی_2; ++I) {
    إذا (!نقاب.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      إذا (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clear_بٹ(M0, M1 + 1);
      إذا (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clear_بٹ(M0, M1 + 1);
    }
    إذا (نقاب.One[I])
      ++M0, ++M1;
    وإلا إذا (!نقاب.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clear_بٹ(M0, ثنائی_ہندسہ_چوڑائی_2);
  أرجع Res;
}

معلوم_بٹ معلوم_بٹ::pdep(ثابت معلوم_بٹ &Val, ثابت معلوم_بٹ &نقاب) {
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = Val.get_ثنائی_ہندسہ_چوڑائی();
  معلوم_بٹ Res(ثنائی_ہندسہ_چوڑائی_2);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.سیٹ_All_Conflict();

  // For each output position I where Mask[I] could be 1, the source position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible source positions widens.
  بنانشانی M0 = 0, M1 = 0;
  لكل (بنانشانی I = 0; I < ثنائی_ہندسہ_چوڑائی_2; ++I) {
    إذا (!نقاب.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clear_ثنائی_ہندسہ(I);
    إذا (!نقاب.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::get_بٹ_سیٹ(ثنائی_ہندسہ_چوڑائی_2, M0, M1 + 1);
      إذا (!Range.is_Subset_دا(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clear_ثنائی_ہندسہ(I);
      إذا (!Range.is_Subset_دا(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clear_ثنائی_ہندسہ(I);
    }
    إذا (نقاب.One[I])
      ++M0, ++M1;
    وإلا إذا (!نقاب.Zero[I])
      ++M1;
  }
  أرجع Res;
}

std::optional<منطقي> معلوم_بٹ::eq(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  إذا (LHS.isConstant() && RHS.isConstant())
    أرجع LHS.getConstant() == RHS.getConstant();
  إذا (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    أرجع خطأ;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بٹ::ne(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  إذا (std::optional<منطقي> معلوم_EQ = eq(LHS, RHS))
    أرجع !*معلوم_EQ;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بٹ::ugt(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  إذا (LHS.get_Max_قدر().ule(RHS.get_Min_قدر()))
    أرجع خطأ;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  إذا (LHS.get_Min_قدر().ugt(RHS.get_Max_قدر()))
    أرجع سچ;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بٹ::uge(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  إذا (std::optional<منطقي> IsUGT = ugt(RHS, LHS))
    أرجع !*IsUGT;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بٹ::ult(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  أرجع ugt(RHS, LHS);
}

std::optional<منطقي> معلوم_بٹ::ule(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  أرجع uge(RHS, LHS);
}

std::optional<منطقي> معلوم_بٹ::sgt(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  إذا (LHS.get_Signed_Max_قدر().sle(RHS.get_Signed_Min_قدر()))
    أرجع خطأ;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  إذا (LHS.get_Signed_Min_قدر().sgt(RHS.get_Signed_Max_قدر()))
    أرجع سچ;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بٹ::sge(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  إذا (std::optional<منطقي> معلوم_SGT = sgt(RHS, LHS))
    أرجع !*معلوم_SGT;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بٹ::slt(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  أرجع sgt(RHS, LHS);
}

std::optional<منطقي> معلوم_بٹ::sle(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  أرجع sge(RHS, LHS);
}

معلوم_بٹ معلوم_بٹ::مطلق_قدر(منطقي IntMinIsPoison) ثابت {
  // If the source's MSB is zero then we know the rest of the bits already.
  إذا (isNonNegative())
    أرجع *هذا;

  // Absolute value preserves trailing zero count.
  معلوم_بٹ معلوم_Abs(get_ثنائی_ہندسہ_چوڑائی());

  // If the input is negative, then abs(x) == -x.
  إذا (isNegative()) {
    معلوم_بٹ Tmp = *هذا;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    إذا (IntMinIsPoison && (Zero.popcount() + 2) == get_ثنائی_ہندسہ_چوڑائی())
      Tmp.One.سیٹ_ثنائی_ہندسہ(گنتی_Min_Trailing_Zeros());

    معلوم_Abs = compute_For_جوڑو_Sub(
        /*جوڑو*/ خطأ, IntMinIsPoison, /*NUW=*/خطأ,
        معلوم_بٹ::makeConstant(APInt(get_ثنائی_ہندسہ_چوڑائی(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    إذا (IntMinIsPoison && Tmp.گنتی_Min_Population() == 1 &&
        Tmp.گنتی_Max_Population() != 1) {
      Tmp.One.clear_Sign_ثنائی_ہندسہ();
      Tmp.Zero.سیٹ_Sign_ثنائی_ہندسہ();
      معلوم_Abs.One.سیٹ_بٹ(get_ثنائی_ہندسہ_چوڑائی() - Tmp.گنتی_Min_Leading_Zeros(),
                           get_ثنائی_ہندسہ_چوڑائی() - 1);
    }

  } وإلا {
    بنانشانی MaxTZ = گنتی_Max_Trailing_Zeros();
    بنانشانی MinTZ = گنتی_Min_Trailing_Zeros();

    معلوم_Abs.Zero.سیٹ_کم_بٹ(MinTZ);
    // If we know the lowest set 1, then preserve it.
    إذا (MaxTZ == MinTZ && MaxTZ < get_ثنائی_ہندسہ_چوڑائی())
      معلوم_Abs.One.سیٹ_ثنائی_ہندسہ(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    إذا (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_قدر())) {
      معلوم_Abs.One.clear_Sign_ثنائی_ہندسہ();
      معلوم_Abs.Zero.سیٹ_Sign_ثنائی_ہندسہ();
    }
  }

  أرجع معلوم_Abs;
}

معلوم_بٹ معلوم_بٹ::reduce_جوڑو(بنانشانی عدد_Elts) ثابت {
  إذا (عدد_Elts == 0)
    أرجع معلوم_بٹ(get_ثنائی_ہندسہ_چوڑائی());

  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = get_ثنائی_ہندسہ_چوڑائی();
  معلوم_بٹ نتیجہ(ثنائی_ہندسہ_چوڑائی_2);

  إذا (isConstant())
    // If all elements are the same constant, we can simply compute it
    أرجع معلوم_بٹ::makeConstant(عدد_Elts * getConstant());

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
  بنانشانی گواچا_بٹ = Log2_32_Ceil(عدد_Elts);

  إذا (isNonNegative()) {
    بنانشانی LeadingZeros = گنتی_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > گواچا_بٹ ? LeadingZeros - گواچا_بٹ : 0;
    نتیجہ.Zero.سیٹ_بلند_بٹ(LeadingZeros);
  } وإلا إذا (isNegative()) {
    بنانشانی LeadingOnes = گنتی_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > گواچا_بٹ ? LeadingOnes - گواچا_بٹ : 0;
    نتیجہ.One.سیٹ_بلند_بٹ(LeadingOnes);
  }

  أرجع نتیجہ;
}

ساكن معلوم_بٹ compute_For_Sat_جوڑو_Sub(منطقي جوڑو_2, منطقي Signed,
                                     ثابت معلوم_بٹ &LHS,
                                     ثابت معلوم_بٹ &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();

  std::optional<منطقي> حد_توں_ودھ;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  منطقي MayNegClamp = سچ;
  منطقي MayPosClamp = سچ;
  إذا (Signed) {
    // Easy cases we can rule out any overflow.
    إذا (جوڑو_2 && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      حد_توں_ودھ = خطأ;
    وإلا إذا (!جوڑو_2 && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      حد_توں_ودھ = خطأ;
    وإلا {
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
          معلوم_بٹ::compute_For_جوڑو_Sub(جوڑو_2, /*NSW=*/خطأ,
                                      /*NUW=*/خطأ, UnsignedLHS, UnsignedRHS);
      إذا (جوڑو_2) {
        إذا (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = خطأ;
          // Pos + Pos will overflow with extra signbit.
          إذا (LHS.isNonNegative() && RHS.isNonNegative())
            حد_توں_ودھ = سچ;
        } وإلا إذا (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = خطأ;
          // Neg + Neg will overflow without extra signbit.
          إذا (LHS.isNegative() && RHS.isNegative())
            حد_توں_ودھ = سچ;
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
            حد_توں_ودھ = سچ;
        } وإلا إذا (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = خطأ;
          // Pos - Neg will overflow without extra signbit.
          إذا (LHS.isNonNegative() && RHS.isNegative())
            حد_توں_ودھ = سچ;
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
      حد_توں_ودھ = خطأ;
  } وإلا إذا (جوڑو_2) {
    // uadd.sat
    منطقي دا;
    (فراغ)LHS.get_Max_قدر().uadd_ov(RHS.get_Max_قدر(), دا);
    إذا (!دا) {
      حد_توں_ودھ = خطأ;
    } وإلا {
      (فراغ)LHS.get_Min_قدر().uadd_ov(RHS.get_Min_قدر(), دا);
      إذا (دا)
        حد_توں_ودھ = سچ;
    }
  } وإلا {
    // usub.sat
    منطقي دا;
    (فراغ)LHS.get_Min_قدر().usub_ov(RHS.get_Max_قدر(), دا);
    إذا (!دا) {
      حد_توں_ودھ = خطأ;
    } وإلا {
      (فراغ)LHS.get_Max_قدر().usub_ov(RHS.get_Min_قدر(), دا);
      إذا (دا)
        حد_توں_ودھ = سچ;
    }
  }

  معلوم_بٹ Res = معلوم_بٹ::compute_For_جوڑو_Sub(جوڑو_2, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  إذا (حد_توں_ودھ) {
    // We know whether or not we overflowed.
    إذا (!(*حد_توں_ودھ)) {
      // No overflow.
      أرجع Res;
    }

    // We overflowed
    APInt C;
    إذا (Signed) {
      // sadd.sat / ssub.sat
      أكد(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_قدر(ثنائی_ہندسہ_چوڑائی_2)
                           : APInt::get_Signed_Max_قدر(ثنائی_ہندسہ_چوڑائی_2);
    } وإلا إذا (جوڑو_2) {
      // uadd.sat
      C = APInt::get_Max_قدر(ثنائی_ہندسہ_چوڑائی_2);
    } وإلا {
      // uadd.sat
      C = APInt::get_Min_قدر(ثنائی_ہندسہ_چوڑائی_2);
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
      Res.Zero.clear_کم_بٹ(ثنائی_ہندسہ_چوڑائی_2 - 1);
    إذا (MayNegClamp)
      Res.One.clear_کم_بٹ(ثنائی_ہندسہ_چوڑائی_2 - 1);
  } وإلا إذا (جوڑو_2) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clear_All_بٹ();
  } وإلا {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clear_All_بٹ();
  }

  أرجع Res;
}

معلوم_بٹ معلوم_بٹ::sadd_sat(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  أرجع compute_For_Sat_جوڑو_Sub(/*جوڑو*/ سچ, /*Signed*/ سچ, LHS, RHS);
}
معلوم_بٹ معلوم_بٹ::ssub_sat(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  أرجع compute_For_Sat_جوڑو_Sub(/*جوڑو*/ خطأ, /*Signed*/ سچ, LHS, RHS);
}
معلوم_بٹ معلوم_بٹ::uadd_sat(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  أرجع compute_For_Sat_جوڑو_Sub(/*جوڑو*/ سچ, /*Signed*/ خطأ, LHS, RHS);
}
معلوم_بٹ معلوم_بٹ::usub_sat(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  أرجع compute_For_Sat_جوڑو_Sub(/*جوڑو*/ خطأ, /*Signed*/ خطأ, LHS, RHS);
}

ساكن معلوم_بٹ avgComputeU(معلوم_بٹ LHS, معلوم_بٹ RHS, منطقي IsCeil) {
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  LHS = LHS.zext(ثنائی_ہندسہ_چوڑائی_2 + 1);
  RHS = RHS.zext(ثنائی_ہندسہ_چوڑائی_2 + 1);
  LHS =
      compute_For_جوڑو_حاصل(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.کڈھو_بٹ(ثنائی_ہندسہ_چوڑائی_2, 1);
  أرجع LHS;
}

معلوم_بٹ معلوم_بٹ::avgFloorS(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  أرجع flip_Sign_ثنائی_ہندسہ(avgFloorU(flip_Sign_ثنائی_ہندسہ(LHS), flip_Sign_ثنائی_ہندسہ(RHS)));
}

معلوم_بٹ معلوم_بٹ::avgFloorU(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  أرجع avgComputeU(LHS, RHS, /*IsCeil=*/خطأ);
}

معلوم_بٹ معلوم_بٹ::avgCeilS(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  أرجع flip_Sign_ثنائی_ہندسہ(avgCeilU(flip_Sign_ثنائی_ہندسہ(LHS), flip_Sign_ثنائی_ہندسہ(RHS)));
}

معلوم_بٹ معلوم_بٹ::avgCeilU(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  أرجع avgComputeU(LHS, RHS, /*IsCeil=*/سچ);
}

معلوم_بٹ معلوم_بٹ::mul(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS,
                         منطقي NoUndefSelfMultiply) {
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
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
  منطقي Has_حد_توں_ودھ;
  APInt UMax_نتیجہ = UMaxLHS.umul_ov(UMaxRHS, Has_حد_توں_ودھ);
  بنانشانی LeadZ = Has_حد_توں_ودھ ? 0 : UMax_نتیجہ.countl_zero();

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
  بنانشانی Trail_بٹ_معلوم_LHS = (LHS.Zero | LHS.One).countr_one();
  بنانشانی Trail_بٹ_معلوم_RHS = (RHS.Zero | RHS.One).countr_one();
  بنانشانی TrailZeroLHS = LHS.گنتی_Min_Trailing_Zeros();
  بنانشانی TrailZeroRHS = RHS.گنتی_Min_Trailing_Zeros();
  بنانشانی TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  بنانشانی سبھ_توں_چھوٹا_عملیہ = std::min(Trail_بٹ_معلوم_LHS - TrailZeroLHS,
                                      Trail_بٹ_معلوم_RHS - TrailZeroRHS);
  بنانشانی نتیجہ_بٹ_معلوم = std::min(سبھ_توں_چھوٹا_عملیہ + TrailZ, ثنائی_ہندسہ_چوڑائی_2);

  // The lower ResultBitsKnown bits of this are known.
  APInt Bottom_معلوم = LHS.One * RHS.One;

  معلوم_بٹ Res(ثنائی_ہندسہ_چوڑائی_2);
  Res.Zero.سیٹ_بلند_بٹ(LeadZ);
  Res.Zero |= (~Bottom_معلوم).get_Lo_بٹ(نتیجہ_بٹ_معلوم);
  Res.One = Bottom_معلوم.get_Lo_بٹ(نتیجہ_بٹ_معلوم);

  إذا (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    بنانشانی TwoTZP1 = 2 * TrailZeroLHS + 1;
    إذا (TwoTZP1 < ثنائی_ہندسہ_چوڑائی_2)
      Res.Zero.سیٹ_ثنائی_ہندسہ(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    إذا (TrailZeroLHS < ثنائی_ہندسہ_چوڑائی_2 && LHS.One[TrailZeroLHS]) {
      بنانشانی TwoTZP2 = TwoTZP1 + 1;
      إذا (TwoTZP2 < ثنائی_ہندسہ_چوڑائی_2)
        Res.Zero.سیٹ_ثنائی_ہندسہ(TwoTZP2);
    }
  }

  أرجع Res;
}

معلوم_بٹ معلوم_بٹ::mulhs(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  أكد(ثنائی_ہندسہ_چوڑائی_2 == RHS.get_ثنائی_ہندسہ_چوڑائی() && "Operand mismatch");
  معلوم_بٹ WideLHS = LHS.sext(2 * ثنائی_ہندسہ_چوڑائی_2);
  معلوم_بٹ WideRHS = RHS.sext(2 * ثنائی_ہندسہ_چوڑائی_2);
  أرجع mul(WideLHS, WideRHS).کڈھو_بٹ(ثنائی_ہندسہ_چوڑائی_2, ثنائی_ہندسہ_چوڑائی_2);
}

معلوم_بٹ معلوم_بٹ::mulhu(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  أكد(ثنائی_ہندسہ_چوڑائی_2 == RHS.get_ثنائی_ہندسہ_چوڑائی() && "Operand mismatch");
  معلوم_بٹ WideLHS = LHS.zext(2 * ثنائی_ہندسہ_چوڑائی_2);
  معلوم_بٹ WideRHS = RHS.zext(2 * ثنائی_ہندسہ_چوڑائی_2);
  أرجع mul(WideLHS, WideRHS).کڈھو_بٹ(ثنائی_ہندسہ_چوڑائی_2, ثنائی_ہندسہ_چوڑائی_2);
}

ساكن معلوم_بٹ div_Compute_کم_ثنائی_ہندسہ(معلوم_بٹ معلوم, ثابت معلوم_بٹ &LHS,
                                  ثابت معلوم_بٹ &RHS, منطقي Exact) {

  إذا (!Exact)
    أرجع معلوم;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  إذا (LHS.One[0])
    معلوم.One.سیٹ_ثنائی_ہندسہ(0);

  صحيح MinTZ =
      (صحيح)LHS.گنتی_Min_Trailing_Zeros() - (صحيح)RHS.گنتی_Max_Trailing_Zeros();
  صحيح MaxTZ =
      (صحيح)LHS.گنتی_Max_Trailing_Zeros() - (صحيح)RHS.گنتی_Min_Trailing_Zeros();
  إذا (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    معلوم.Zero.سیٹ_کم_بٹ(MinTZ);
    إذا (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      معلوم.One.سیٹ_ثنائی_ہندسہ(MinTZ);
    }
  } وإلا إذا (MaxTZ < 0) {
    // Poison Result
    معلوم.سیٹ_All_Zero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  إذا (معلوم.hasConflict())
    معلوم.سیٹ_All_Zero();

  أرجع معلوم;
}

معلوم_بٹ معلوم_بٹ::sdiv(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS,
                          منطقي Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  إذا (LHS.isNonNegative() && RHS.isNonNegative())
    أرجع udiv(LHS, RHS, Exact);

  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);

  إذا (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    معلوم.سیٹ_All_Zero();
    أرجع معلوم;
  }

  std::optional<APInt> Res;
  إذا (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_قدر();
    APInt عدد = LHS.get_Signed_Min_قدر();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (عدد.is_Min_Signed_قدر() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_قدر(ثنائی_ہندسہ_چوڑائی_2)
              : عدد.sdiv(Denom);
  } وإلا إذا (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    إذا (Exact || (-LHS.get_Signed_Max_قدر()).uge(RHS.get_Signed_Max_قدر())) {
      APInt Denom = RHS.get_Signed_Min_قدر();
      APInt عدد = LHS.get_Signed_Min_قدر();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      إذا (Denom.isZero())
        Denom.سیٹ_ثنائی_ہندسہ(RHS.گنتی_Min_Trailing_Zeros());
      Res = عدد.sdiv(Denom);
    }
  } وإلا إذا (LHS.is_Strictly_مثبت() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    إذا (Exact || LHS.get_Signed_Min_قدر().uge(-RHS.get_Signed_Min_قدر())) {
      APInt Denom = RHS.get_Signed_Max_قدر();
      APInt عدد = LHS.get_Signed_Max_قدر();
      Res = عدد.sdiv(Denom);
    }
  }

  إذا (Res) {
    إذا (Res->isNonNegative()) {
      بنانشانی LeadZ = Res->گنتی_Leading_Zeros();
      معلوم.Zero.سیٹ_بلند_بٹ(LeadZ);
    } وإلا {
      بنانشانی LeadO = Res->گنتی_Leading_Ones();
      معلوم.One.سیٹ_بلند_بٹ(LeadO);
    }
  }

  معلوم = div_Compute_کم_ثنائی_ہندسہ(معلوم, LHS, RHS, Exact);
  أرجع معلوم;
}

معلوم_بٹ معلوم_بٹ::udiv(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS,
                          منطقي Exact) {
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);

  إذا (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    معلوم.سیٹ_All_Zero();
    أرجع معلوم;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_قدر();
  إذا (MinDenom.isZero())
    MinDenom.سیٹ_ثنائی_ہندسہ(RHS.گنتی_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_قدر().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_قدر().udiv(RHS.get_Max_قدر());

  معلوم = معلوم_بٹ::makeConstant(MinRes);
  إذا (std::optional<بنانشانی> Different_ثنائی_ہندسہ =
          APIntOps::Get_Most_Significant_Different_ثنائی_ہندسہ(MinRes, MaxRes)) {
    معلوم.Zero.clear_کم_بٹ(*Different_ثنائی_ہندسہ + 1);
    معلوم.One.clear_کم_بٹ(*Different_ثنائی_ہندسہ + 1);
  }

  معلوم = div_Compute_کم_ثنائی_ہندسہ(معلوم, LHS, RHS, Exact);

  أرجع معلوم;
}

معلوم_بٹ معلوم_بٹ::rem_Get_کم_بٹ(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = LHS.get_ثنائی_ہندسہ_چوڑائی();
  إذا (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    بنانشانی RHSZeros = RHS.گنتی_Min_Trailing_Zeros();
    APInt نقاب = APInt::get_کم_بٹ_سیٹ(ثنائی_ہندسہ_چوڑائی_2, RHSZeros);
    APInt Ones_نقاب = LHS.One & نقاب;
    APInt Zeros_نقاب = LHS.Zero & نقاب;
    أرجع معلوم_بٹ(Zeros_نقاب, Ones_نقاب);
  }
  أرجع معلوم_بٹ(ثنائی_ہندسہ_چوڑائی_2);
}

معلوم_بٹ معلوم_بٹ::urem(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  إذا (LHS.get_Max_قدر().ult(RHS.get_Min_قدر()))
    أرجع LHS;

  معلوم_بٹ معلوم = rem_Get_کم_بٹ(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_قدر(), RHS.get_Max_قدر() - 1);
  معلوم.Zero.سیٹ_بلند_بٹ(MaxRes.گنتی_Leading_Zeros());
  أرجع معلوم;
}

معلوم_بٹ معلوم_بٹ::srem(ثابت معلوم_بٹ &LHS, ثابت معلوم_بٹ &RHS) {
  معلوم_بٹ معلوم = rem_Get_کم_بٹ(LHS, RHS);
  إذا (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt کم_بٹ = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    إذا (LHS.isNonNegative() || کم_بٹ.is_Subset_دا(LHS.Zero))
      معلوم.Zero |= ~کم_بٹ;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    إذا (LHS.isNegative() && کم_بٹ.intersects(LHS.One))
      معلوم.One |= ~کم_بٹ;
    أرجع معلوم;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  إذا (LHS.isNegative() && معلوم.isNonZero())
    معلوم.One.سیٹ_بلند_بٹ(
        std::max(LHS.گنتی_Min_Leading_Ones(), RHS.گنتی_Min_Sign_بٹ()));
  وإلا إذا (LHS.isNonNegative())
    معلوم.Zero.سیٹ_بلند_بٹ(
        std::max(LHS.گنتی_Min_Leading_Zeros(), RHS.گنتی_Min_Sign_بٹ()));
  أرجع معلوم;
}

معلوم_بٹ &معلوم_بٹ::عامل&=(ثابت معلوم_بٹ &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  أرجع *هذا;
}

معلوم_بٹ &معلوم_بٹ::عامل|=(ثابت معلوم_بٹ &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  أرجع *هذا;
}

معلوم_بٹ &معلوم_بٹ::عامل^=(ثابت معلوم_بٹ &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  أرجع *هذا;
}

معلوم_بٹ معلوم_بٹ::blsi() ثابت {
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = get_ثنائی_ہندسہ_چوڑائی();
  معلوم_بٹ معلوم(Zero, APInt(ثنائی_ہندسہ_چوڑائی_2, 0));
  بنانشانی Max = گنتی_Max_Trailing_Zeros();
  معلوم.Zero.سیٹ_بٹ_From(std::min(Max + 1, ثنائی_ہندسہ_چوڑائی_2));
  بنانشانی Min = گنتی_Min_Trailing_Zeros();
  إذا (Max == Min && Max < ثنائی_ہندسہ_چوڑائی_2)
    معلوم.One.سیٹ_ثنائی_ہندسہ(Max);
  أرجع معلوم;
}

معلوم_بٹ معلوم_بٹ::blsmsk() ثابت {
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = get_ثنائی_ہندسہ_چوڑائی();
  معلوم_بٹ معلوم(ثنائی_ہندسہ_چوڑائی_2);
  بنانشانی Max = گنتی_Max_Trailing_Zeros();
  معلوم.Zero.سیٹ_بٹ_From(std::min(Max + 1, ثنائی_ہندسہ_چوڑائی_2));
  بنانشانی Min = گنتی_Min_Trailing_Zeros();
  معلوم.One.سیٹ_کم_بٹ(std::min(Min + 1, ثنائی_ہندسہ_چوڑائی_2));
  أرجع معلوم;
}

فراغ معلوم_بٹ::print(raw_ostream &OS) ثابت {
  بنانشانی ثنائی_ہندسہ_چوڑائی_2 = get_ثنائی_ہندسہ_چوڑائی();
  لكل (بنانشانی I = 0; I < ثنائی_ہندسہ_چوڑائی_2; ++I) {
    بنانشانی N = ثنائی_ہندسہ_چوڑائی_2 - I - 1;
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
LLVM_DUMP_طریقہ فراغ معلوم_بٹ::dump() ثابت {
  print(dbgs());
  dbgs() << "\n";
}
#جے_انت

بنانشانی llvm::Sign_بٹ_Ops::rot(بنانشانی Src_Sign_بٹ, بنانشانی ثنائی_ہندسہ_چوڑائی_2,
                                std::optional<APInt> RotAmt,
                                منطقي Is_Rotate_دایاں) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  إذا (Src_Sign_بٹ == ثنائی_ہندسہ_چوڑائی_2)
    أرجع ثنائی_ہندسہ_چوڑائی_2;

  إذا (!RotAmt)
    أرجع 1;

  بنانشانی Amt = RotAmt->urem(ثنائی_ہندسہ_چوڑائی_2);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  إذا (Is_Rotate_دایاں)
    Amt = (ثنائی_ہندسہ_چوڑائی_2 - Amt) % ثنائی_ہندسہ_چوڑائی_2;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  إذا (Src_Sign_بٹ > Amt + 1)
    أرجع Src_Sign_بٹ - Amt;
  أرجع 1;
}
