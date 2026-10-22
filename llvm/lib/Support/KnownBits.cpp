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

استعمال نطاق llvm;

معلوم_بٽ معلوم_بٽ::flip_Sign_ٻٽو_انگ(ثابت معلوم_بٽ &Val) {
  بنانشان Sign_ٻٽو_انگ_Position = Val.get_ٻٽو_انگ_ويڪر() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.سيٽ_ٻٽو_انگ_Val(Sign_ٻٽو_انگ_Position, Val.One[Sign_ٻٽو_انگ_Position]);
  One.سيٽ_ٻٽو_انگ_Val(Sign_ٻٽو_انگ_Position, Val.Zero[Sign_ٻٽو_انگ_Position]);
  أرجع معلوم_بٽ(Zero, One);
}

ساكن معلوم_بٽ compute_For_جوڙيو_حاصل(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS,
                                    منطقي حاصل_Zero, منطقي حاصل_One) {

  APInt Possible_جوڙ_Zero = LHS.get_Max_قدر() + RHS.get_Max_قدر() + !حاصل_Zero;
  APInt Possible_جوڙ_One = LHS.get_Min_قدر() + RHS.get_Min_قدر() + حاصل_One;

  // Compute known bits of the carry.
  APInt حاصل_معلوم_Zero = ~(Possible_جوڙ_Zero ^ LHS.Zero ^ RHS.Zero);
  APInt حاصل_معلوم_One = Possible_جوڙ_One ^ LHS.One ^ RHS.One;

  // Compute set of known bits (where all three relevant bits are known).
  APInt LHSKnownUnion = LHS.Zero | LHS.One;
  APInt RHSKnownUnion = RHS.Zero | RHS.One;
  APInt حاصل_معلوم_Union = std::move(حاصل_معلوم_Zero) | حاصل_معلوم_One;
  APInt معلوم = std::move(LHSKnownUnion) & RHSKnownUnion & حاصل_معلوم_Union;

  // Compute known bits of the result.
  معلوم_بٽ معلوم_ٻاهريون;
  معلوم_ٻاهريون.Zero = ~std::move(Possible_جوڙ_Zero) & معلوم;
  معلوم_ٻاهريون.One = std::move(Possible_جوڙ_One) & معلوم;
  أرجع معلوم_ٻاهريون;
}

معلوم_بٽ معلوم_بٽ::compute_For_جوڙيو_حاصل(
    ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS, ثابت معلوم_بٽ &حاصل_2) {
  أكد(حاصل_2.get_ٻٽو_انگ_ويڪر() == 1 && "Carry must be 1-bit");
  أرجع ::compute_For_جوڙيو_حاصل(
      LHS, RHS, حاصل_2.Zero.get_Bool_قدر(), حاصل_2.One.get_Bool_قدر());
}

معلوم_بٽ معلوم_بٽ::compute_For_جوڙيو_Sub(منطقي جوڙيو_2, منطقي NSW, منطقي NUW,
                                      ثابت معلوم_بٽ &LHS,
                                      ثابت معلوم_بٽ &RHS) {
  بنانشان ٻٽو_انگ_ويڪر_2 = LHS.get_ٻٽو_انگ_ويڪر();
  معلوم_بٽ معلوم_ٻاهريون(ٻٽو_انگ_ويڪر_2);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  إذا (LHS.isUnknown() && RHS.isUnknown())
    أرجع معلوم_ٻاهريون;

  إذا (!LHS.isUnknown() && !RHS.isUnknown()) {
    إذا (جوڙيو_2) {
      // Sum = LHS + RHS + 0
      معلوم_ٻاهريون = ::compute_For_جوڙيو_حاصل(LHS, RHS, /*CarryZero=*/سچ,
                                      /*CarryOne=*/خطأ);
    } وإلا {
      // Sum = LHS + ~RHS + 1
      معلوم_بٽ NotRHS = RHS;
      std::مٽا_سٽا(NotRHS.Zero, NotRHS.One);
      معلوم_ٻاهريون = ::compute_For_جوڙيو_حاصل(LHS, NotRHS, /*CarryZero=*/خطأ,
                                      /*CarryOne=*/سچ);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  إذا (NUW) {
    إذا (جوڙيو_2) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_قدر().uadd_sat(RHS.get_Min_قدر());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      إذا (NSW) {
        بنانشان انگ_بٽ_2 = MinVal.trunc(ٻٽو_انگ_ويڪر_2 - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        معلوم_ٻاهريون.One.سيٽ_بٽ(ٻٽو_انگ_ويڪر_2 - 1 - انگ_بٽ_2, ٻٽو_انگ_ويڪر_2 - 1);
      }
      معلوم_ٻاهريون.One.سيٽ_مٿاهون_بٽ(MinVal.countl_one());
    } وإلا {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_قدر().usub_sat(RHS.get_Min_قدر());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      إذا (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        بنانشان انگ_بٽ_2 = MaxVal.trunc(ٻٽو_انگ_ويڪر_2 - 1).countl_zero();
        معلوم_ٻاهريون.Zero.سيٽ_بٽ(ٻٽو_انگ_ويڪر_2 - 1 - انگ_بٽ_2, ٻٽو_انگ_ويڪر_2 - 1);
      }
      معلوم_ٻاهريون.Zero.سيٽ_مٿاهون_بٽ(MaxVal.countl_zero());
    }
  }

  إذا (NSW) {
    APInt MinVal;
    APInt MaxVal;
    إذا (جوڙيو_2) {
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
      بنانشان انگ_بٽ_2 = MinVal.trunc(ٻٽو_انگ_ويڪر_2 - 1).countl_one();
      معلوم_ٻاهريون.One.سيٽ_بٽ(ٻٽو_انگ_ويڪر_2 - 1 - انگ_بٽ_2, ٻٽو_انگ_ويڪر_2 - 1);
      معلوم_ٻاهريون.Zero.سيٽ_Sign_ٻٽو_انگ();
    }
    إذا (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      بنانشان انگ_بٽ_2 = MaxVal.trunc(ٻٽو_انگ_ويڪر_2 - 1).countl_zero();
      معلوم_ٻاهريون.Zero.سيٽ_بٽ(ٻٽو_انگ_ويڪر_2 - 1 - انگ_بٽ_2, ٻٽو_انگ_ويڪر_2 - 1);
      معلوم_ٻاهريون.One.سيٽ_Sign_ٻٽو_انگ();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  إذا (معلوم_ٻاهريون.hasConflict())
    معلوم_ٻاهريون.سيٽ_All_Zero();
  أرجع معلوم_ٻاهريون;
}

معلوم_بٽ معلوم_بٽ::computeForSubBorrow(ثابت معلوم_بٽ &LHS, معلوم_بٽ RHS,
                                         ثابت معلوم_بٽ &Borrow) {
  أكد(Borrow.get_ٻٽو_انگ_ويڪر() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::مٽا_سٽا(RHS.Zero, RHS.One);
  أرجع ::compute_For_جوڙيو_حاصل(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_قدر(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_قدر());
}

معلوم_بٽ معلوم_بٽ::truncSSat(بنانشان ٻٽو_انگ_ويڪر_2) ثابت {
  بنانشان داخلا_بٽ = get_ٻٽو_انگ_ويڪر();
  APInt Min_اندريون_Range = APInt::get_Signed_Min_قدر(ٻٽو_انگ_ويڪر_2).sext(داخلا_بٽ);
  APInt Max_اندريون_Range = APInt::get_Signed_Max_قدر(ٻٽو_انگ_ويڪر_2).sext(داخلا_بٽ);
  APInt داخلا_Min = get_Signed_Min_قدر();
  APInt داخلا_Max = get_Signed_Max_قدر();
  معلوم_بٽ معلوم(ٻٽو_انگ_ويڪر_2);

  // Case 1: All values fit - just truncate
  إذا (داخلا_Min.sge(Min_اندريون_Range) && داخلا_Max.sle(Max_اندريون_Range)) {
    معلوم = trunc(ٻٽو_انگ_ويڪر_2);
  }
  // Case 2: All saturate to min
  وإلا إذا (داخلا_Max.slt(Min_اندريون_Range)) {
    معلوم = معلوم_بٽ::makeConstant(APInt::get_Signed_Min_قدر(ٻٽو_انگ_ويڪر_2));
  }
  // Case 3: All saturate to max
  وإلا إذا (داخلا_Min.sgt(Max_اندريون_Range)) {
    معلوم = معلوم_بٽ::makeConstant(APInt::get_Signed_Max_قدر(ٻٽو_انگ_ويڪر_2));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  وإلا إذا (داخلا_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    معلوم.Zero = APInt(ٻٽو_انگ_ويڪر_2, 0);
    معلوم.Zero.سيٽ_ٻٽو_انگ(ٻٽو_انگ_ويڪر_2 - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    معلوم.One = One.trunc(ٻٽو_انگ_ويڪر_2);
    معلوم.One.clear_ٻٽو_انگ(ٻٽو_انگ_ويڪر_2 - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  وإلا إذا (داخلا_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    معلوم.Zero = Zero.trunc(ٻٽو_انگ_ويڪر_2);
    معلوم.Zero.clear_ٻٽو_انگ(ٻٽو_انگ_ويڪر_2 - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    معلوم.One = APInt(ٻٽو_انگ_ويڪر_2, 0);
    معلوم.One.سيٽ_ٻٽو_انگ(ٻٽو_انگ_ويڪر_2 - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  وإلا {
    // Output: min saturation, truncated, or max saturation
    APInt داخلا_Zero_هيٺاهون = Zero.trunc(ٻٽو_انگ_ويڪر_2);
    APInt داخلا_One_هيٺاهون = One.trunc(ٻٽو_انگ_ويڪر_2);
    APInt MinSat = APInt::get_Signed_Min_قدر(ٻٽو_انگ_ويڪر_2);
    APInt MaxSat = APInt::get_Signed_Max_قدر(ٻٽو_انگ_ويڪر_2);
    معلوم_بٽ MinSatKB = معلوم_بٽ::makeConstant(MinSat);
    معلوم_بٽ MaxSatKB = معلوم_بٽ::makeConstant(MaxSat);
    إذا (داخلا_Max.sle(Max_اندريون_Range)) {
      // Positive values fit, only negatives saturate to min
      معلوم.Zero = داخلا_Zero_هيٺاهون & MinSatKB.Zero;
      معلوم.One = داخلا_One_هيٺاهون & MinSatKB.One;
    } وإلا إذا (داخلا_Min.sge(Min_اندريون_Range)) {
      // Negative values fit, only positives saturate to max
      معلوم.Zero = داخلا_Zero_هيٺاهون & MaxSatKB.Zero;
      معلوم.One = داخلا_One_هيٺاهون & MaxSatKB.One;
    } وإلا {
      // Both positive and negative values might saturate
      معلوم.Zero = داخلا_Zero_هيٺاهون & MinSatKB.Zero & MaxSatKB.Zero;
      معلوم.One = داخلا_One_هيٺاهون & MinSatKB.One & MaxSatKB.One;
    }
  }
  أرجع معلوم;
}

معلوم_بٽ معلوم_بٽ::truncSSatU(بنانشان ٻٽو_انگ_ويڪر_2) ثابت {
  بنانشان داخلا_بٽ = get_ٻٽو_انگ_ويڪر();
  APInt Max_اندريون_Range = APInt::getAllOnes(ٻٽو_انگ_ويڪر_2).zext(داخلا_بٽ);
  APInt داخلا_Min = get_Signed_Min_قدر();
  APInt داخلا_Max = get_Signed_Max_قدر();
  معلوم_بٽ معلوم(ٻٽو_انگ_ويڪر_2);

  إذا (isNegative()) {
    معلوم.سيٽ_All_Zero();
  } وإلا إذا (داخلا_Min.isNonNegative() && داخلا_Max.ule(Max_اندريون_Range)) {
    معلوم = trunc(ٻٽو_انگ_ويڪر_2);
  } وإلا إذا (داخلا_Min.isNonNegative() && داخلا_Min.ugt(Max_اندريون_Range)) {
    معلوم.سيٽ_All_Ones();
  } وإلا إذا (داخلا_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    معلوم.Zero = APInt(ٻٽو_انگ_ويڪر_2, 0);
    // Common one bits: bits that are 1 in input stay 1
    معلوم.One = One.trunc(ٻٽو_انگ_ويڪر_2);
  } وإلا {
    // Mixed: sign bit unknown
    إذا (داخلا_Max.ule(Max_اندريون_Range)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      معلوم.Zero = Zero.trunc(ٻٽو_انگ_ويڪر_2);
      معلوم.One = APInt(ٻٽو_انگ_ويڪر_2, 0);
    } وإلا {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      معلوم.Zero = APInt(ٻٽو_انگ_ويڪر_2, 0);
      معلوم.One = APInt(ٻٽو_انگ_ويڪر_2, 0);
    }
  }
  أرجع معلوم;
}

معلوم_بٽ معلوم_بٽ::truncUSat(بنانشان ٻٽو_انگ_ويڪر_2) ثابت {
  بنانشان داخلا_بٽ = get_ٻٽو_انگ_ويڪر();
  APInt Max_اندريون_Range = APInt::get_هيٺيون_بٽ_سيٽ(داخلا_بٽ, ٻٽو_انگ_ويڪر_2);
  APInt داخلا_Max = get_Max_قدر();
  APInt داخلا_Min = get_Min_قدر();
  معلوم_بٽ معلوم(ٻٽو_انگ_ويڪر_2);

  إذا (داخلا_Max.ule(Max_اندريون_Range)) {
    معلوم = trunc(ٻٽو_انگ_ويڪر_2);
  } وإلا إذا (داخلا_Min.ugt(Max_اندريون_Range)) {
    معلوم.سيٽ_All_Ones();
  } وإلا {
    معلوم.resetAll();
    معلوم.One = One.trunc(ٻٽو_انگ_ويڪر_2);
  }
  أرجع معلوم;
}

معلوم_بٽ معلوم_بٽ::sext_اندريون_Reg(بنانشان Src_ٻٽو_انگ_ويڪر) ثابت {
  بنانشان ٻٽو_انگ_ويڪر_2 = get_ٻٽو_انگ_ويڪر();
  أكد(0 < Src_ٻٽو_انگ_ويڪر && Src_ٻٽو_انگ_ويڪر <= ٻٽو_انگ_ويڪر_2 &&
         "Illegal sext-in-register");

  إذا (Src_ٻٽو_انگ_ويڪر == ٻٽو_انگ_ويڪر_2)
    أرجع *هذا;

  بنانشان Ext_بٽ = ٻٽو_انگ_ويڪر_2 - Src_ٻٽو_انگ_ويڪر;
  معلوم_بٽ نتيجو;
  نتيجو.One = One << Ext_بٽ;
  نتيجو.Zero = Zero << Ext_بٽ;
  نتيجو.One.ashr_اندريون_رکو(Ext_بٽ);
  نتيجو.Zero.ashr_اندريون_رکو(Ext_بٽ);
  أرجع نتيجو;
}

معلوم_بٽ معلوم_بٽ::makeGE(ثابت APInt &Val) ثابت {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  بنانشان N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_هيٺيون_بٽ(get_ٻٽو_انگ_ويڪر() - N);
  أرجع معلوم_بٽ(Zero, One | MaskedVal);
}

معلوم_بٽ معلوم_بٽ::umax(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
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
  معلوم_بٽ L = LHS.makeGE(RHS.get_Min_قدر());
  معلوم_بٽ R = RHS.makeGE(LHS.get_Min_قدر());
  أرجع L.intersectWith(R);
}

معلوم_بٽ معلوم_بٽ::umin(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  خودڪار Flip = [](ثابت معلوم_بٽ &Val) { أرجع معلوم_بٽ(Val.One, Val.Zero); };
  أرجع Flip(umax(Flip(LHS), Flip(RHS)));
}

معلوم_بٽ معلوم_بٽ::smax(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  أرجع flip_Sign_ٻٽو_انگ(umax(flip_Sign_ٻٽو_انگ(LHS), flip_Sign_ٻٽو_انگ(RHS)));
}

معلوم_بٽ معلوم_بٽ::smin(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  خودڪار Flip = [](ثابت معلوم_بٽ &Val) {
    بنانشان Sign_ٻٽو_انگ_Position = Val.get_ٻٽو_انگ_ويڪر() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.سيٽ_ٻٽو_انگ_Val(Sign_ٻٽو_انگ_Position, Val.Zero[Sign_ٻٽو_انگ_Position]);
    One.سيٽ_ٻٽو_انگ_Val(Sign_ٻٽو_انگ_Position, Val.One[Sign_ٻٽو_انگ_Position]);
    أرجع معلوم_بٽ(Zero, One);
  };
  أرجع Flip(umax(Flip(LHS), Flip(RHS)));
}

معلوم_بٽ معلوم_بٽ::abdu(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  إذا (LHS.get_Min_قدر().uge(RHS.get_Max_قدر()))
    أرجع compute_For_جوڙيو_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, LHS,
                            RHS);
  إذا (RHS.get_Min_قدر().uge(LHS.get_Max_قدر()))
    أرجع compute_For_جوڙيو_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  معلوم_بٽ Diff0 =
      compute_For_جوڙيو_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/سچ, LHS, RHS);
  معلوم_بٽ Diff1 =
      compute_For_جوڙيو_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/سچ, RHS, LHS);
  أرجع Diff0.intersectWith(Diff1);
}

معلوم_بٽ معلوم_بٽ::abds(معلوم_بٽ LHS, معلوم_بٽ RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  إذا (LHS.get_Signed_Min_قدر().sge(RHS.get_Signed_Max_قدر()))
    أرجع compute_For_جوڙيو_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, LHS,
                            RHS);
  إذا (RHS.get_Signed_Min_قدر().sge(LHS.get_Signed_Max_قدر()))
    أرجع compute_For_جوڙيو_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  بنانشان Sign_ٻٽو_انگ_Position = LHS.get_ٻٽو_انگ_ويڪر() - 1;
  لكل (خودڪار Arg : {&LHS, &RHS}) {
    منطقي Tmp = Arg->Zero[Sign_ٻٽو_انگ_Position];
    Arg->Zero.سيٽ_ٻٽو_انگ_Val(Sign_ٻٽو_انگ_Position, Arg->One[Sign_ٻٽو_انگ_Position]);
    Arg->One.سيٽ_ٻٽو_انگ_Val(Sign_ٻٽو_انگ_Position, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  معلوم_بٽ Diff0 =
      compute_For_جوڙيو_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/سچ, LHS, RHS);
  معلوم_بٽ Diff1 =
      compute_For_جوڙيو_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/سچ, RHS, LHS);
  أرجع Diff0.intersectWith(Diff1);
}

ساكن بنانشان get_Max_سرڪايو_Amount(ثابت APInt &Max_قدر, بنانشان ٻٽو_انگ_ويڪر_2) {
  إذا (isPowerOf2_32(ٻٽو_انگ_ويڪر_2))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    أرجع Max_قدر.ڪڍو_بٽ_As_ZExt_قدر(
        std::min(Log2_32(ٻٽو_انگ_ويڪر_2), Max_قدر.get_ٻٽو_انگ_ويڪر()), 0);
  // This is only an approximate upper bound.
  أرجع Max_قدر.get_Limited_قدر(ٻٽو_انگ_ويڪر_2 - 1);
}

معلوم_بٽ معلوم_بٽ::shl(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS, منطقي NUW,
                         منطقي NSW, منطقي ShAmtNonZero) {
  بنانشان ٻٽو_انگ_ويڪر_2 = LHS.get_ٻٽو_انگ_ويڪر();
  خودڪار سرڪايو_By_Const = [&](ثابت معلوم_بٽ &LHS, بنانشان سرڪايو_Amt_2) {
    معلوم_بٽ معلوم;
    منطقي Shifted_ٻاهريون_Zero, Shifted_ٻاهريون_One;
    معلوم.Zero = LHS.Zero.ushl_ov(سرڪايو_Amt_2, Shifted_ٻاهريون_Zero);
    معلوم.Zero.سيٽ_هيٺيون_بٽ(سرڪايو_Amt_2);
    معلوم.One = LHS.One.ushl_ov(سرڪايو_Amt_2, Shifted_ٻاهريون_One);

    // All cases returning poison have been handled by MaxShiftAmount already.
    إذا (NSW) {
      إذا (NUW && سرڪايو_Amt_2 != 0)
        // NUW means we can assume anything shifted out was a zero.
        Shifted_ٻاهريون_Zero = سچ;

      إذا (Shifted_ٻاهريون_Zero)
        معلوم.makeNonNegative();
      وإلا إذا (Shifted_ٻاهريون_One)
        معلوم.makeNegative();
    }
    أرجع معلوم;
  };

  // Fast path for a common case when LHS is completely unknown.
  معلوم_بٽ معلوم(ٻٽو_انگ_ويڪر_2);
  بنانشان Min_سرڪايو_Amount = RHS.get_Min_قدر().get_Limited_قدر(ٻٽو_انگ_ويڪر_2);
  إذا (Min_سرڪايو_Amount == 0 && ShAmtNonZero)
    Min_سرڪايو_Amount = 1;
  إذا (LHS.isUnknown()) {
    معلوم.Zero.سيٽ_هيٺيون_بٽ(Min_سرڪايو_Amount);
    إذا (NUW && NSW && Min_سرڪايو_Amount != 0)
      معلوم.makeNonNegative();
    أرجع معلوم;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_قدر = RHS.get_Max_قدر();
  بنانشان Max_سرڪايو_Amount = get_Max_سرڪايو_Amount(Max_قدر, ٻٽو_انگ_ويڪر_2);
  إذا (NUW && NSW)
    Max_سرڪايو_Amount = std::min(Max_سرڪايو_Amount, LHS.ڳڻپ_Max_Leading_Zeros() - 1);
  إذا (NUW)
    Max_سرڪايو_Amount = std::min(Max_سرڪايو_Amount, LHS.ڳڻپ_Max_Leading_Zeros());
  إذا (NSW)
    Max_سرڪايو_Amount = std::min(
        Max_سرڪايو_Amount,
        std::max(LHS.ڳڻپ_Max_Leading_Zeros(), LHS.ڳڻپ_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  إذا (Min_سرڪايو_Amount == 0 && Max_سرڪايو_Amount == ٻٽو_انگ_ويڪر_2 - 1 &&
      isPowerOf2_32(ٻٽو_انگ_ويڪر_2)) {
    معلوم.Zero.سيٽ_هيٺيون_بٽ(LHS.ڳڻپ_Min_Trailing_Zeros());
    إذا (LHS.isAllOnes())
      معلوم.One.سيٽ_Sign_ٻٽو_انگ();
    إذا (NSW) {
      إذا (LHS.isNonNegative())
        معلوم.makeNonNegative();
      إذا (LHS.isNegative())
        معلوم.makeNegative();
    }
    أرجع معلوم;
  }

  // Find the common bits from all possible shifts.
  بنانشان سرڪايو_Amt_Zero_نقاب = RHS.Zero.zextOrTrunc(32).get_ZExt_قدر();
  بنانشان سرڪايو_Amt_One_نقاب = RHS.One.zextOrTrunc(32).get_ZExt_قدر();
  معلوم.سيٽ_All_Conflict();
  لكل (بنانشان سرڪايو_Amt_2 = Min_سرڪايو_Amount; سرڪايو_Amt_2 <= Max_سرڪايو_Amount;
       ++سرڪايو_Amt_2) {
    // Skip if the shift amount is impossible.
    إذا ((سرڪايو_Amt_Zero_نقاب & سرڪايو_Amt_2) != 0 ||
        (سرڪايو_Amt_One_نقاب | سرڪايو_Amt_2) != سرڪايو_Amt_2)
      استمر;
    معلوم = معلوم.intersectWith(سرڪايو_By_Const(LHS, سرڪايو_Amt_2));
    إذا (معلوم.isUnknown())
      اكسر;
  }

  // All shift amounts may result in poison.
  إذا (معلوم.hasConflict())
    معلوم.سيٽ_All_Zero();
  أرجع معلوم;
}

معلوم_بٽ معلوم_بٽ::lshr(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS,
                          منطقي ShAmtNonZero, منطقي Exact) {
  بنانشان ٻٽو_انگ_ويڪر_2 = LHS.get_ٻٽو_انگ_ويڪر();
  خودڪار سرڪايو_By_Const = [&](ثابت معلوم_بٽ &LHS, بنانشان سرڪايو_Amt_2) {
    معلوم_بٽ معلوم = LHS;
    معلوم >>= سرڪايو_Amt_2;
    // High bits are known zero.
    معلوم.Zero.سيٽ_مٿاهون_بٽ(سرڪايو_Amt_2);
    أرجع معلوم;
  };

  // Fast path for a common case when LHS is completely unknown.
  معلوم_بٽ معلوم(ٻٽو_انگ_ويڪر_2);
  بنانشان Min_سرڪايو_Amount = RHS.get_Min_قدر().get_Limited_قدر(ٻٽو_انگ_ويڪر_2);
  إذا (Min_سرڪايو_Amount == 0 && ShAmtNonZero)
    Min_سرڪايو_Amount = 1;
  إذا (LHS.isUnknown()) {
    معلوم.Zero.سيٽ_مٿاهون_بٽ(Min_سرڪايو_Amount);
    أرجع معلوم;
  }

  // Find the common bits from all possible shifts.
  APInt Max_قدر = RHS.get_Max_قدر();
  بنانشان Max_سرڪايو_Amount = get_Max_سرڪايو_Amount(Max_قدر, ٻٽو_انگ_ويڪر_2);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  إذا (Exact) {
    بنانشان پهريون_One = LHS.ڳڻپ_Max_Trailing_Zeros();
    إذا (پهريون_One < Min_سرڪايو_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      معلوم.سيٽ_All_Zero();
      أرجع معلوم;
    }
    Max_سرڪايو_Amount = std::min(Max_سرڪايو_Amount, پهريون_One);
  }

  بنانشان سرڪايو_Amt_Zero_نقاب = RHS.Zero.zextOrTrunc(32).get_ZExt_قدر();
  بنانشان سرڪايو_Amt_One_نقاب = RHS.One.zextOrTrunc(32).get_ZExt_قدر();
  معلوم.سيٽ_All_Conflict();
  لكل (بنانشان سرڪايو_Amt_2 = Min_سرڪايو_Amount; سرڪايو_Amt_2 <= Max_سرڪايو_Amount;
       ++سرڪايو_Amt_2) {
    // Skip if the shift amount is impossible.
    إذا ((سرڪايو_Amt_Zero_نقاب & سرڪايو_Amt_2) != 0 ||
        (سرڪايو_Amt_One_نقاب | سرڪايو_Amt_2) != سرڪايو_Amt_2)
      استمر;
    معلوم = معلوم.intersectWith(سرڪايو_By_Const(LHS, سرڪايو_Amt_2));
    إذا (معلوم.isUnknown())
      اكسر;
  }

  // All shift amounts may result in poison.
  إذا (معلوم.hasConflict())
    معلوم.سيٽ_All_Zero();
  أرجع معلوم;
}

معلوم_بٽ معلوم_بٽ::ashr(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS,
                          منطقي ShAmtNonZero, منطقي Exact) {
  بنانشان ٻٽو_انگ_ويڪر_2 = LHS.get_ٻٽو_انگ_ويڪر();
  خودڪار سرڪايو_By_Const = [&](ثابت معلوم_بٽ &LHS, بنانشان سرڪايو_Amt_2) {
    معلوم_بٽ معلوم = LHS;
    معلوم.Zero.ashr_اندريون_رکو(سرڪايو_Amt_2);
    معلوم.One.ashr_اندريون_رکو(سرڪايو_Amt_2);
    أرجع معلوم;
  };

  // Fast path for a common case when LHS is completely unknown.
  معلوم_بٽ معلوم(ٻٽو_انگ_ويڪر_2);
  بنانشان Min_سرڪايو_Amount = RHS.get_Min_قدر().get_Limited_قدر(ٻٽو_انگ_ويڪر_2);
  إذا (Min_سرڪايو_Amount == 0 && ShAmtNonZero)
    Min_سرڪايو_Amount = 1;
  إذا (LHS.isUnknown()) {
    إذا (Min_سرڪايو_Amount == ٻٽو_انگ_ويڪر_2) {
      // Always poison. Return zero because we don't like returning conflict.
      معلوم.سيٽ_All_Zero();
      أرجع معلوم;
    }
    أرجع معلوم;
  }

  // Find the common bits from all possible shifts.
  APInt Max_قدر = RHS.get_Max_قدر();
  بنانشان Max_سرڪايو_Amount = get_Max_سرڪايو_Amount(Max_قدر, ٻٽو_انگ_ويڪر_2);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  إذا (Exact) {
    بنانشان پهريون_One = LHS.ڳڻپ_Max_Trailing_Zeros();
    إذا (پهريون_One < Min_سرڪايو_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      معلوم.سيٽ_All_Zero();
      أرجع معلوم;
    }
    Max_سرڪايو_Amount = std::min(Max_سرڪايو_Amount, پهريون_One);
  }

  بنانشان سرڪايو_Amt_Zero_نقاب = RHS.Zero.zextOrTrunc(32).get_ZExt_قدر();
  بنانشان سرڪايو_Amt_One_نقاب = RHS.One.zextOrTrunc(32).get_ZExt_قدر();
  معلوم.سيٽ_All_Conflict();
  لكل (بنانشان سرڪايو_Amt_2 = Min_سرڪايو_Amount; سرڪايو_Amt_2 <= Max_سرڪايو_Amount;
      ++سرڪايو_Amt_2) {
    // Skip if the shift amount is impossible.
    إذا ((سرڪايو_Amt_Zero_نقاب & سرڪايو_Amt_2) != 0 ||
        (سرڪايو_Amt_One_نقاب | سرڪايو_Amt_2) != سرڪايو_Amt_2)
      استمر;
    معلوم = معلوم.intersectWith(سرڪايو_By_Const(LHS, سرڪايو_Amt_2));
    إذا (معلوم.isUnknown())
      اكسر;
  }

  // All shift amounts may result in poison.
  إذا (معلوم.hasConflict())
    معلوم.سيٽ_All_Zero();
  أرجع معلوم;
}

معلوم_بٽ معلوم_بٽ::fshl(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS,
                          ثابت APInt &Amt) {
  أرجع معلوم_بٽ(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

معلوم_بٽ معلوم_بٽ::fshr(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS,
                          ثابت APInt &Amt) {
  أرجع معلوم_بٽ(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

معلوم_بٽ معلوم_بٽ::clmul(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  معلوم_بٽ Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_قدر(), RHS.get_Min_قدر()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  خودڪار ClMulOr = [](ثابت APInt &LHS, ثابت APInt &RHS) {
    بنانشان BW = LHS.get_ٻٽو_انگ_ويڪر();
    أكد(BW == RHS.get_ٻٽو_انگ_ويڪر() && "Operand mismatch");
    APInt نتيجو(BW, 0);
    لكل (بنانشان I :
         seq(std::min(RHS.get_سرگرم_بٽ(), BW - LHS.countr_zero())))
      إذا (RHS[I])
        نتيجو |= LHS << I;
    أرجع نتيجو;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt معلوم = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= معلوم;
  Res.One &= معلوم;

  أرجع Res;
}

معلوم_بٽ معلوم_بٽ::pext(ثابت معلوم_بٽ &Val, ثابت معلوم_بٽ &نقاب) {
  بنانشان ٻٽو_انگ_ويڪر_2 = Val.get_ٻٽو_انگ_ويڪر();
  معلوم_بٽ Res(ٻٽو_انگ_ويڪر_2);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.سيٽ_All_Conflict();

  // For each source position I where Mask[I] could be 1, the output position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible output positions widens.
  بنانشان M0 = 0, M1 = 0;
  لكل (بنانشان I = 0; I < ٻٽو_انگ_ويڪر_2; ++I) {
    إذا (!نقاب.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      إذا (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clear_بٽ(M0, M1 + 1);
      إذا (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clear_بٽ(M0, M1 + 1);
    }
    إذا (نقاب.One[I])
      ++M0, ++M1;
    وإلا إذا (!نقاب.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clear_بٽ(M0, ٻٽو_انگ_ويڪر_2);
  أرجع Res;
}

معلوم_بٽ معلوم_بٽ::pdep(ثابت معلوم_بٽ &Val, ثابت معلوم_بٽ &نقاب) {
  بنانشان ٻٽو_انگ_ويڪر_2 = Val.get_ٻٽو_انگ_ويڪر();
  معلوم_بٽ Res(ٻٽو_انگ_ويڪر_2);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.سيٽ_All_Conflict();

  // For each output position I where Mask[I] could be 1, the source position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible source positions widens.
  بنانشان M0 = 0, M1 = 0;
  لكل (بنانشان I = 0; I < ٻٽو_انگ_ويڪر_2; ++I) {
    إذا (!نقاب.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clear_ٻٽو_انگ(I);
    إذا (!نقاب.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::get_بٽ_سيٽ(ٻٽو_انگ_ويڪر_2, M0, M1 + 1);
      إذا (!Range.is_Subset_جو(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clear_ٻٽو_انگ(I);
      إذا (!Range.is_Subset_جو(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clear_ٻٽو_انگ(I);
    }
    إذا (نقاب.One[I])
      ++M0, ++M1;
    وإلا إذا (!نقاب.Zero[I])
      ++M1;
  }
  أرجع Res;
}

std::optional<منطقي> معلوم_بٽ::eq(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  إذا (LHS.isConstant() && RHS.isConstant())
    أرجع LHS.getConstant() == RHS.getConstant();
  إذا (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    أرجع خطأ;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بٽ::ne(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  إذا (std::optional<منطقي> معلوم_EQ = eq(LHS, RHS))
    أرجع !*معلوم_EQ;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بٽ::ugt(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  إذا (LHS.get_Max_قدر().ule(RHS.get_Min_قدر()))
    أرجع خطأ;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  إذا (LHS.get_Min_قدر().ugt(RHS.get_Max_قدر()))
    أرجع سچ;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بٽ::uge(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  إذا (std::optional<منطقي> IsUGT = ugt(RHS, LHS))
    أرجع !*IsUGT;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بٽ::ult(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  أرجع ugt(RHS, LHS);
}

std::optional<منطقي> معلوم_بٽ::ule(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  أرجع uge(RHS, LHS);
}

std::optional<منطقي> معلوم_بٽ::sgt(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  إذا (LHS.get_Signed_Max_قدر().sle(RHS.get_Signed_Min_قدر()))
    أرجع خطأ;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  إذا (LHS.get_Signed_Min_قدر().sgt(RHS.get_Signed_Max_قدر()))
    أرجع سچ;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بٽ::sge(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  إذا (std::optional<منطقي> معلوم_SGT = sgt(RHS, LHS))
    أرجع !*معلوم_SGT;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بٽ::slt(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  أرجع sgt(RHS, LHS);
}

std::optional<منطقي> معلوم_بٽ::sle(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  أرجع sge(RHS, LHS);
}

معلوم_بٽ معلوم_بٽ::مطلق_قدر(منطقي IntMinIsPoison) ثابت {
  // If the source's MSB is zero then we know the rest of the bits already.
  إذا (isNonNegative())
    أرجع *هذا;

  // Absolute value preserves trailing zero count.
  معلوم_بٽ معلوم_Abs(get_ٻٽو_انگ_ويڪر());

  // If the input is negative, then abs(x) == -x.
  إذا (isNegative()) {
    معلوم_بٽ Tmp = *هذا;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    إذا (IntMinIsPoison && (Zero.popcount() + 2) == get_ٻٽو_انگ_ويڪر())
      Tmp.One.سيٽ_ٻٽو_انگ(ڳڻپ_Min_Trailing_Zeros());

    معلوم_Abs = compute_For_جوڙيو_Sub(
        /*جوڙيو*/ خطأ, IntMinIsPoison, /*NUW=*/خطأ,
        معلوم_بٽ::makeConstant(APInt(get_ٻٽو_انگ_ويڪر(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    إذا (IntMinIsPoison && Tmp.ڳڻپ_Min_Population() == 1 &&
        Tmp.ڳڻپ_Max_Population() != 1) {
      Tmp.One.clear_Sign_ٻٽو_انگ();
      Tmp.Zero.سيٽ_Sign_ٻٽو_انگ();
      معلوم_Abs.One.سيٽ_بٽ(get_ٻٽو_انگ_ويڪر() - Tmp.ڳڻپ_Min_Leading_Zeros(),
                           get_ٻٽو_انگ_ويڪر() - 1);
    }

  } وإلا {
    بنانشان MaxTZ = ڳڻپ_Max_Trailing_Zeros();
    بنانشان MinTZ = ڳڻپ_Min_Trailing_Zeros();

    معلوم_Abs.Zero.سيٽ_هيٺيون_بٽ(MinTZ);
    // If we know the lowest set 1, then preserve it.
    إذا (MaxTZ == MinTZ && MaxTZ < get_ٻٽو_انگ_ويڪر())
      معلوم_Abs.One.سيٽ_ٻٽو_انگ(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    إذا (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_قدر())) {
      معلوم_Abs.One.clear_Sign_ٻٽو_انگ();
      معلوم_Abs.Zero.سيٽ_Sign_ٻٽو_انگ();
    }
  }

  أرجع معلوم_Abs;
}

معلوم_بٽ معلوم_بٽ::reduce_جوڙيو(بنانشان انگ_Elts) ثابت {
  إذا (انگ_Elts == 0)
    أرجع معلوم_بٽ(get_ٻٽو_انگ_ويڪر());

  بنانشان ٻٽو_انگ_ويڪر_2 = get_ٻٽو_انگ_ويڪر();
  معلوم_بٽ نتيجو(ٻٽو_انگ_ويڪر_2);

  إذا (isConstant())
    // If all elements are the same constant, we can simply compute it
    أرجع معلوم_بٽ::makeConstant(انگ_Elts * getConstant());

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
  بنانشان گم_بٽ = Log2_32_Ceil(انگ_Elts);

  إذا (isNonNegative()) {
    بنانشان LeadingZeros = ڳڻپ_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > گم_بٽ ? LeadingZeros - گم_بٽ : 0;
    نتيجو.Zero.سيٽ_مٿاهون_بٽ(LeadingZeros);
  } وإلا إذا (isNegative()) {
    بنانشان LeadingOnes = ڳڻپ_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > گم_بٽ ? LeadingOnes - گم_بٽ : 0;
    نتيجو.One.سيٽ_مٿاهون_بٽ(LeadingOnes);
  }

  أرجع نتيجو;
}

ساكن معلوم_بٽ compute_For_Sat_جوڙيو_Sub(منطقي جوڙيو_2, منطقي Signed,
                                     ثابت معلوم_بٽ &LHS,
                                     ثابت معلوم_بٽ &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  بنانشان ٻٽو_انگ_ويڪر_2 = LHS.get_ٻٽو_انگ_ويڪر();

  std::optional<منطقي> حد_کان_مٿي;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  منطقي MayNegClamp = سچ;
  منطقي MayPosClamp = سچ;
  إذا (Signed) {
    // Easy cases we can rule out any overflow.
    إذا (جوڙيو_2 && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      حد_کان_مٿي = خطأ;
    وإلا إذا (!جوڙيو_2 && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      حد_کان_مٿي = خطأ;
    وإلا {
      // Check if we may overflow. If we can't rule out overflow then check if
      // we can rule out a direction at least.
      معلوم_بٽ UnsignedLHS = LHS;
      معلوم_بٽ UnsignedRHS = RHS;
      // Get version of LHS/RHS with clearer signbit. This allows us to detect
      // how the addition/subtraction might overflow into the signbit. Then
      // using the actual known signbits of LHS/RHS, we can figure out which
      // overflows are/aren't possible.
      UnsignedLHS.One.clear_Sign_ٻٽو_انگ();
      UnsignedLHS.Zero.سيٽ_Sign_ٻٽو_انگ();
      UnsignedRHS.One.clear_Sign_ٻٽو_انگ();
      UnsignedRHS.Zero.سيٽ_Sign_ٻٽو_انگ();
      معلوم_بٽ Res =
          معلوم_بٽ::compute_For_جوڙيو_Sub(جوڙيو_2, /*NSW=*/خطأ,
                                      /*NUW=*/خطأ, UnsignedLHS, UnsignedRHS);
      إذا (جوڙيو_2) {
        إذا (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = خطأ;
          // Pos + Pos will overflow with extra signbit.
          إذا (LHS.isNonNegative() && RHS.isNonNegative())
            حد_کان_مٿي = سچ;
        } وإلا إذا (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = خطأ;
          // Neg + Neg will overflow without extra signbit.
          إذا (LHS.isNegative() && RHS.isNegative())
            حد_کان_مٿي = سچ;
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
            حد_کان_مٿي = سچ;
        } وإلا إذا (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = خطأ;
          // Pos - Neg will overflow without extra signbit.
          إذا (LHS.isNonNegative() && RHS.isNegative())
            حد_کان_مٿي = سچ;
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
      حد_کان_مٿي = خطأ;
  } وإلا إذا (جوڙيو_2) {
    // uadd.sat
    منطقي جو;
    (فراغ)LHS.get_Max_قدر().uadd_ov(RHS.get_Max_قدر(), جو);
    إذا (!جو) {
      حد_کان_مٿي = خطأ;
    } وإلا {
      (فراغ)LHS.get_Min_قدر().uadd_ov(RHS.get_Min_قدر(), جو);
      إذا (جو)
        حد_کان_مٿي = سچ;
    }
  } وإلا {
    // usub.sat
    منطقي جو;
    (فراغ)LHS.get_Min_قدر().usub_ov(RHS.get_Max_قدر(), جو);
    إذا (!جو) {
      حد_کان_مٿي = خطأ;
    } وإلا {
      (فراغ)LHS.get_Max_قدر().usub_ov(RHS.get_Min_قدر(), جو);
      إذا (جو)
        حد_کان_مٿي = سچ;
    }
  }

  معلوم_بٽ Res = معلوم_بٽ::compute_For_جوڙيو_Sub(جوڙيو_2, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  إذا (حد_کان_مٿي) {
    // We know whether or not we overflowed.
    إذا (!(*حد_کان_مٿي)) {
      // No overflow.
      أرجع Res;
    }

    // We overflowed
    APInt C;
    إذا (Signed) {
      // sadd.sat / ssub.sat
      أكد(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_قدر(ٻٽو_انگ_ويڪر_2)
                           : APInt::get_Signed_Max_قدر(ٻٽو_انگ_ويڪر_2);
    } وإلا إذا (جوڙيو_2) {
      // uadd.sat
      C = APInt::get_Max_قدر(ٻٽو_انگ_ويڪر_2);
    } وإلا {
      // uadd.sat
      C = APInt::get_Min_قدر(ٻٽو_انگ_ويڪر_2);
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
      Res.Zero.clear_هيٺيون_بٽ(ٻٽو_انگ_ويڪر_2 - 1);
    إذا (MayNegClamp)
      Res.One.clear_هيٺيون_بٽ(ٻٽو_انگ_ويڪر_2 - 1);
  } وإلا إذا (جوڙيو_2) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clear_All_بٽ();
  } وإلا {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clear_All_بٽ();
  }

  أرجع Res;
}

معلوم_بٽ معلوم_بٽ::sadd_sat(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  أرجع compute_For_Sat_جوڙيو_Sub(/*جوڙيو*/ سچ, /*Signed*/ سچ, LHS, RHS);
}
معلوم_بٽ معلوم_بٽ::ssub_sat(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  أرجع compute_For_Sat_جوڙيو_Sub(/*جوڙيو*/ خطأ, /*Signed*/ سچ, LHS, RHS);
}
معلوم_بٽ معلوم_بٽ::uadd_sat(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  أرجع compute_For_Sat_جوڙيو_Sub(/*جوڙيو*/ سچ, /*Signed*/ خطأ, LHS, RHS);
}
معلوم_بٽ معلوم_بٽ::usub_sat(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  أرجع compute_For_Sat_جوڙيو_Sub(/*جوڙيو*/ خطأ, /*Signed*/ خطأ, LHS, RHS);
}

ساكن معلوم_بٽ avgComputeU(معلوم_بٽ LHS, معلوم_بٽ RHS, منطقي IsCeil) {
  بنانشان ٻٽو_انگ_ويڪر_2 = LHS.get_ٻٽو_انگ_ويڪر();
  LHS = LHS.zext(ٻٽو_انگ_ويڪر_2 + 1);
  RHS = RHS.zext(ٻٽو_انگ_ويڪر_2 + 1);
  LHS =
      compute_For_جوڙيو_حاصل(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.ڪڍو_بٽ(ٻٽو_انگ_ويڪر_2, 1);
  أرجع LHS;
}

معلوم_بٽ معلوم_بٽ::avgFloorS(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  أرجع flip_Sign_ٻٽو_انگ(avgFloorU(flip_Sign_ٻٽو_انگ(LHS), flip_Sign_ٻٽو_انگ(RHS)));
}

معلوم_بٽ معلوم_بٽ::avgFloorU(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  أرجع avgComputeU(LHS, RHS, /*IsCeil=*/خطأ);
}

معلوم_بٽ معلوم_بٽ::avgCeilS(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  أرجع flip_Sign_ٻٽو_انگ(avgCeilU(flip_Sign_ٻٽو_انگ(LHS), flip_Sign_ٻٽو_انگ(RHS)));
}

معلوم_بٽ معلوم_بٽ::avgCeilU(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  أرجع avgComputeU(LHS, RHS, /*IsCeil=*/سچ);
}

معلوم_بٽ معلوم_بٽ::mul(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS,
                         منطقي NoUndefSelfMultiply) {
  بنانشان ٻٽو_انگ_ويڪر_2 = LHS.get_ٻٽو_انگ_ويڪر();
  أكد(ٻٽو_انگ_ويڪر_2 == RHS.get_ٻٽو_انگ_ويڪر() && "Operand mismatch");
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
  منطقي Has_حد_کان_مٿي;
  APInt UMax_نتيجو = UMaxLHS.umul_ov(UMaxRHS, Has_حد_کان_مٿي);
  بنانشان LeadZ = Has_حد_کان_مٿي ? 0 : UMax_نتيجو.countl_zero();

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
  بنانشان Trail_بٽ_معلوم_LHS = (LHS.Zero | LHS.One).countr_one();
  بنانشان Trail_بٽ_معلوم_RHS = (RHS.Zero | RHS.One).countr_one();
  بنانشان TrailZeroLHS = LHS.ڳڻپ_Min_Trailing_Zeros();
  بنانشان TrailZeroRHS = RHS.ڳڻپ_Min_Trailing_Zeros();
  بنانشان TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  بنانشان سڀ_کان_ننڍو_عمليو = std::min(Trail_بٽ_معلوم_LHS - TrailZeroLHS,
                                      Trail_بٽ_معلوم_RHS - TrailZeroRHS);
  بنانشان نتيجو_بٽ_معلوم = std::min(سڀ_کان_ننڍو_عمليو + TrailZ, ٻٽو_انگ_ويڪر_2);

  // The lower ResultBitsKnown bits of this are known.
  APInt Bottom_معلوم = LHS.One * RHS.One;

  معلوم_بٽ Res(ٻٽو_انگ_ويڪر_2);
  Res.Zero.سيٽ_مٿاهون_بٽ(LeadZ);
  Res.Zero |= (~Bottom_معلوم).get_Lo_بٽ(نتيجو_بٽ_معلوم);
  Res.One = Bottom_معلوم.get_Lo_بٽ(نتيجو_بٽ_معلوم);

  إذا (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    بنانشان TwoTZP1 = 2 * TrailZeroLHS + 1;
    إذا (TwoTZP1 < ٻٽو_انگ_ويڪر_2)
      Res.Zero.سيٽ_ٻٽو_انگ(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    إذا (TrailZeroLHS < ٻٽو_انگ_ويڪر_2 && LHS.One[TrailZeroLHS]) {
      بنانشان TwoTZP2 = TwoTZP1 + 1;
      إذا (TwoTZP2 < ٻٽو_انگ_ويڪر_2)
        Res.Zero.سيٽ_ٻٽو_انگ(TwoTZP2);
    }
  }

  أرجع Res;
}

معلوم_بٽ معلوم_بٽ::mulhs(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  بنانشان ٻٽو_انگ_ويڪر_2 = LHS.get_ٻٽو_انگ_ويڪر();
  أكد(ٻٽو_انگ_ويڪر_2 == RHS.get_ٻٽو_انگ_ويڪر() && "Operand mismatch");
  معلوم_بٽ WideLHS = LHS.sext(2 * ٻٽو_انگ_ويڪر_2);
  معلوم_بٽ WideRHS = RHS.sext(2 * ٻٽو_انگ_ويڪر_2);
  أرجع mul(WideLHS, WideRHS).ڪڍو_بٽ(ٻٽو_انگ_ويڪر_2, ٻٽو_انگ_ويڪر_2);
}

معلوم_بٽ معلوم_بٽ::mulhu(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  بنانشان ٻٽو_انگ_ويڪر_2 = LHS.get_ٻٽو_انگ_ويڪر();
  أكد(ٻٽو_انگ_ويڪر_2 == RHS.get_ٻٽو_انگ_ويڪر() && "Operand mismatch");
  معلوم_بٽ WideLHS = LHS.zext(2 * ٻٽو_انگ_ويڪر_2);
  معلوم_بٽ WideRHS = RHS.zext(2 * ٻٽو_انگ_ويڪر_2);
  أرجع mul(WideLHS, WideRHS).ڪڍو_بٽ(ٻٽو_انگ_ويڪر_2, ٻٽو_انگ_ويڪر_2);
}

ساكن معلوم_بٽ div_Compute_هيٺيون_ٻٽو_انگ(معلوم_بٽ معلوم, ثابت معلوم_بٽ &LHS,
                                  ثابت معلوم_بٽ &RHS, منطقي Exact) {

  إذا (!Exact)
    أرجع معلوم;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  إذا (LHS.One[0])
    معلوم.One.سيٽ_ٻٽو_انگ(0);

  صحيح MinTZ =
      (صحيح)LHS.ڳڻپ_Min_Trailing_Zeros() - (صحيح)RHS.ڳڻپ_Max_Trailing_Zeros();
  صحيح MaxTZ =
      (صحيح)LHS.ڳڻپ_Max_Trailing_Zeros() - (صحيح)RHS.ڳڻپ_Min_Trailing_Zeros();
  إذا (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    معلوم.Zero.سيٽ_هيٺيون_بٽ(MinTZ);
    إذا (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      معلوم.One.سيٽ_ٻٽو_انگ(MinTZ);
    }
  } وإلا إذا (MaxTZ < 0) {
    // Poison Result
    معلوم.سيٽ_All_Zero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  إذا (معلوم.hasConflict())
    معلوم.سيٽ_All_Zero();

  أرجع معلوم;
}

معلوم_بٽ معلوم_بٽ::sdiv(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS,
                          منطقي Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  إذا (LHS.isNonNegative() && RHS.isNonNegative())
    أرجع udiv(LHS, RHS, Exact);

  بنانشان ٻٽو_انگ_ويڪر_2 = LHS.get_ٻٽو_انگ_ويڪر();
  معلوم_بٽ معلوم(ٻٽو_انگ_ويڪر_2);

  إذا (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    معلوم.سيٽ_All_Zero();
    أرجع معلوم;
  }

  std::optional<APInt> Res;
  إذا (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_قدر();
    APInt انگ = LHS.get_Signed_Min_قدر();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (انگ.is_Min_Signed_قدر() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_قدر(ٻٽو_انگ_ويڪر_2)
              : انگ.sdiv(Denom);
  } وإلا إذا (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    إذا (Exact || (-LHS.get_Signed_Max_قدر()).uge(RHS.get_Signed_Max_قدر())) {
      APInt Denom = RHS.get_Signed_Min_قدر();
      APInt انگ = LHS.get_Signed_Min_قدر();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      إذا (Denom.isZero())
        Denom.سيٽ_ٻٽو_انگ(RHS.ڳڻپ_Min_Trailing_Zeros());
      Res = انگ.sdiv(Denom);
    }
  } وإلا إذا (LHS.is_Strictly_مثبت() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    إذا (Exact || LHS.get_Signed_Min_قدر().uge(-RHS.get_Signed_Min_قدر())) {
      APInt Denom = RHS.get_Signed_Max_قدر();
      APInt انگ = LHS.get_Signed_Max_قدر();
      Res = انگ.sdiv(Denom);
    }
  }

  إذا (Res) {
    إذا (Res->isNonNegative()) {
      بنانشان LeadZ = Res->ڳڻپ_Leading_Zeros();
      معلوم.Zero.سيٽ_مٿاهون_بٽ(LeadZ);
    } وإلا {
      بنانشان LeadO = Res->ڳڻپ_Leading_Ones();
      معلوم.One.سيٽ_مٿاهون_بٽ(LeadO);
    }
  }

  معلوم = div_Compute_هيٺيون_ٻٽو_انگ(معلوم, LHS, RHS, Exact);
  أرجع معلوم;
}

معلوم_بٽ معلوم_بٽ::udiv(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS,
                          منطقي Exact) {
  بنانشان ٻٽو_انگ_ويڪر_2 = LHS.get_ٻٽو_انگ_ويڪر();
  معلوم_بٽ معلوم(ٻٽو_انگ_ويڪر_2);

  إذا (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    معلوم.سيٽ_All_Zero();
    أرجع معلوم;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_قدر();
  إذا (MinDenom.isZero())
    MinDenom.سيٽ_ٻٽو_انگ(RHS.ڳڻپ_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_قدر().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_قدر().udiv(RHS.get_Max_قدر());

  معلوم = معلوم_بٽ::makeConstant(MinRes);
  إذا (std::optional<بنانشان> Different_ٻٽو_انگ =
          APIntOps::Get_Most_Significant_Different_ٻٽو_انگ(MinRes, MaxRes)) {
    معلوم.Zero.clear_هيٺيون_بٽ(*Different_ٻٽو_انگ + 1);
    معلوم.One.clear_هيٺيون_بٽ(*Different_ٻٽو_انگ + 1);
  }

  معلوم = div_Compute_هيٺيون_ٻٽو_انگ(معلوم, LHS, RHS, Exact);

  أرجع معلوم;
}

معلوم_بٽ معلوم_بٽ::rem_Get_هيٺيون_بٽ(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  بنانشان ٻٽو_انگ_ويڪر_2 = LHS.get_ٻٽو_انگ_ويڪر();
  إذا (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    بنانشان RHSZeros = RHS.ڳڻپ_Min_Trailing_Zeros();
    APInt نقاب = APInt::get_هيٺيون_بٽ_سيٽ(ٻٽو_انگ_ويڪر_2, RHSZeros);
    APInt Ones_نقاب = LHS.One & نقاب;
    APInt Zeros_نقاب = LHS.Zero & نقاب;
    أرجع معلوم_بٽ(Zeros_نقاب, Ones_نقاب);
  }
  أرجع معلوم_بٽ(ٻٽو_انگ_ويڪر_2);
}

معلوم_بٽ معلوم_بٽ::urem(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  إذا (LHS.get_Max_قدر().ult(RHS.get_Min_قدر()))
    أرجع LHS;

  معلوم_بٽ معلوم = rem_Get_هيٺيون_بٽ(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_قدر(), RHS.get_Max_قدر() - 1);
  معلوم.Zero.سيٽ_مٿاهون_بٽ(MaxRes.ڳڻپ_Leading_Zeros());
  أرجع معلوم;
}

معلوم_بٽ معلوم_بٽ::srem(ثابت معلوم_بٽ &LHS, ثابت معلوم_بٽ &RHS) {
  معلوم_بٽ معلوم = rem_Get_هيٺيون_بٽ(LHS, RHS);
  إذا (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt هيٺيون_بٽ = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    إذا (LHS.isNonNegative() || هيٺيون_بٽ.is_Subset_جو(LHS.Zero))
      معلوم.Zero |= ~هيٺيون_بٽ;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    إذا (LHS.isNegative() && هيٺيون_بٽ.intersects(LHS.One))
      معلوم.One |= ~هيٺيون_بٽ;
    أرجع معلوم;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  إذا (LHS.isNegative() && معلوم.isNonZero())
    معلوم.One.سيٽ_مٿاهون_بٽ(
        std::max(LHS.ڳڻپ_Min_Leading_Ones(), RHS.ڳڻپ_Min_Sign_بٽ()));
  وإلا إذا (LHS.isNonNegative())
    معلوم.Zero.سيٽ_مٿاهون_بٽ(
        std::max(LHS.ڳڻپ_Min_Leading_Zeros(), RHS.ڳڻپ_Min_Sign_بٽ()));
  أرجع معلوم;
}

معلوم_بٽ &معلوم_بٽ::عامل&=(ثابت معلوم_بٽ &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  أرجع *هذا;
}

معلوم_بٽ &معلوم_بٽ::عامل|=(ثابت معلوم_بٽ &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  أرجع *هذا;
}

معلوم_بٽ &معلوم_بٽ::عامل^=(ثابت معلوم_بٽ &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  أرجع *هذا;
}

معلوم_بٽ معلوم_بٽ::blsi() ثابت {
  بنانشان ٻٽو_انگ_ويڪر_2 = get_ٻٽو_انگ_ويڪر();
  معلوم_بٽ معلوم(Zero, APInt(ٻٽو_انگ_ويڪر_2, 0));
  بنانشان Max = ڳڻپ_Max_Trailing_Zeros();
  معلوم.Zero.سيٽ_بٽ_From(std::min(Max + 1, ٻٽو_انگ_ويڪر_2));
  بنانشان Min = ڳڻپ_Min_Trailing_Zeros();
  إذا (Max == Min && Max < ٻٽو_انگ_ويڪر_2)
    معلوم.One.سيٽ_ٻٽو_انگ(Max);
  أرجع معلوم;
}

معلوم_بٽ معلوم_بٽ::blsmsk() ثابت {
  بنانشان ٻٽو_انگ_ويڪر_2 = get_ٻٽو_انگ_ويڪر();
  معلوم_بٽ معلوم(ٻٽو_انگ_ويڪر_2);
  بنانشان Max = ڳڻپ_Max_Trailing_Zeros();
  معلوم.Zero.سيٽ_بٽ_From(std::min(Max + 1, ٻٽو_انگ_ويڪر_2));
  بنانشان Min = ڳڻپ_Min_Trailing_Zeros();
  معلوم.One.سيٽ_هيٺيون_بٽ(std::min(Min + 1, ٻٽو_انگ_ويڪر_2));
  أرجع معلوم;
}

فراغ معلوم_بٽ::print(raw_ostream &OS) ثابت {
  بنانشان ٻٽو_انگ_ويڪر_2 = get_ٻٽو_انگ_ويڪر();
  لكل (بنانشان I = 0; I < ٻٽو_انگ_ويڪر_2; ++I) {
    بنانشان N = ٻٽو_انگ_ويڪر_2 - I - 1;
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
LLVM_DUMP_طريقو فراغ معلوم_بٽ::dump() ثابت {
  print(dbgs());
  dbgs() << "\n";
}
#جيڪڏهن_پورو

بنانشان llvm::Sign_بٽ_Ops::rot(بنانشان Src_Sign_بٽ, بنانشان ٻٽو_انگ_ويڪر_2,
                                std::optional<APInt> RotAmt,
                                منطقي Is_Rotate_ساڄو) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  إذا (Src_Sign_بٽ == ٻٽو_انگ_ويڪر_2)
    أرجع ٻٽو_انگ_ويڪر_2;

  إذا (!RotAmt)
    أرجع 1;

  بنانشان Amt = RotAmt->urem(ٻٽو_انگ_ويڪر_2);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  إذا (Is_Rotate_ساڄو)
    Amt = (ٻٽو_انگ_ويڪر_2 - Amt) % ٻٽو_انگ_ويڪر_2;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  إذا (Src_Sign_بٽ > Amt + 1)
    أرجع Src_Sign_بٽ - Amt;
  أرجع 1;
}
