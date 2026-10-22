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

کارول نطاق llvm;

معلوم_بټونه معلوم_بټونه::flip_Sign_بټ(ثابت معلوم_بټونه &Val) {
  بینښه Sign_بټ_Position = Val.get_بټ_پلنوالی() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.سېټ_بټ_Val(Sign_بټ_Position, Val.One[Sign_بټ_Position]);
  One.سېټ_بټ_Val(Sign_بټ_Position, Val.Zero[Sign_بټ_Position]);
  أرجع معلوم_بټونه(Zero, One);
}

ساكن معلوم_بټونه compute_For_جمع_کړه_لېږد(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS,
                                    منطقي لېږد_Zero, منطقي لېږد_One) {

  APInt Possible_مجموعه_Zero = LHS.get_Max_ارزښت() + RHS.get_Max_ارزښت() + !لېږد_Zero;
  APInt Possible_مجموعه_One = LHS.get_Min_ارزښت() + RHS.get_Min_ارزښت() + لېږد_One;

  // Compute known bits of the carry.
  APInt لېږد_معلوم_Zero = ~(Possible_مجموعه_Zero ^ LHS.Zero ^ RHS.Zero);
  APInt لېږد_معلوم_One = Possible_مجموعه_One ^ LHS.One ^ RHS.One;

  // Compute set of known bits (where all three relevant bits are known).
  APInt LHSKnownUnion = LHS.Zero | LHS.One;
  APInt RHSKnownUnion = RHS.Zero | RHS.One;
  APInt لېږد_معلوم_Union = std::move(لېږد_معلوم_Zero) | لېږد_معلوم_One;
  APInt معلوم = std::move(LHSKnownUnion) & RHSKnownUnion & لېږد_معلوم_Union;

  // Compute known bits of the result.
  معلوم_بټونه معلوم_بهر;
  معلوم_بهر.Zero = ~std::move(Possible_مجموعه_Zero) & معلوم;
  معلوم_بهر.One = std::move(Possible_مجموعه_One) & معلوم;
  أرجع معلوم_بهر;
}

معلوم_بټونه معلوم_بټونه::compute_For_جمع_کړه_لېږد(
    ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS, ثابت معلوم_بټونه &لېږد_2) {
  أكد(لېږد_2.get_بټ_پلنوالی() == 1 && "Carry must be 1-bit");
  أرجع ::compute_For_جمع_کړه_لېږد(
      LHS, RHS, لېږد_2.Zero.get_Bool_ارزښت(), لېږد_2.One.get_Bool_ارزښت());
}

معلوم_بټونه معلوم_بټونه::compute_For_جمع_کړه_Sub(منطقي جمع_کړه_2, منطقي NSW, منطقي NUW,
                                      ثابت معلوم_بټونه &LHS,
                                      ثابت معلوم_بټونه &RHS) {
  بینښه بټ_پلنوالی_2 = LHS.get_بټ_پلنوالی();
  معلوم_بټونه معلوم_بهر(بټ_پلنوالی_2);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  إذا (LHS.isUnknown() && RHS.isUnknown())
    أرجع معلوم_بهر;

  إذا (!LHS.isUnknown() && !RHS.isUnknown()) {
    إذا (جمع_کړه_2) {
      // Sum = LHS + RHS + 0
      معلوم_بهر = ::compute_For_جمع_کړه_لېږد(LHS, RHS, /*CarryZero=*/ریښتیا,
                                      /*CarryOne=*/خطأ);
    } وإلا {
      // Sum = LHS + ~RHS + 1
      معلوم_بټونه NotRHS = RHS;
      std::بدلون(NotRHS.Zero, NotRHS.One);
      معلوم_بهر = ::compute_For_جمع_کړه_لېږد(LHS, NotRHS, /*CarryZero=*/خطأ,
                                      /*CarryOne=*/ریښتیا);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  إذا (NUW) {
    إذا (جمع_کړه_2) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_ارزښت().uadd_sat(RHS.get_Min_ارزښت());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      إذا (NSW) {
        بینښه عدد_بټونه_2 = MinVal.trunc(بټ_پلنوالی_2 - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        معلوم_بهر.One.سېټ_بټونه(بټ_پلنوالی_2 - 1 - عدد_بټونه_2, بټ_پلنوالی_2 - 1);
      }
      معلوم_بهر.One.سېټ_لوړ_بټونه(MinVal.countl_one());
    } وإلا {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_ارزښت().usub_sat(RHS.get_Min_ارزښت());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      إذا (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        بینښه عدد_بټونه_2 = MaxVal.trunc(بټ_پلنوالی_2 - 1).countl_zero();
        معلوم_بهر.Zero.سېټ_بټونه(بټ_پلنوالی_2 - 1 - عدد_بټونه_2, بټ_پلنوالی_2 - 1);
      }
      معلوم_بهر.Zero.سېټ_لوړ_بټونه(MaxVal.countl_zero());
    }
  }

  إذا (NSW) {
    APInt MinVal;
    APInt MaxVal;
    إذا (جمع_کړه_2) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_ارزښت().sadd_sat(RHS.get_Signed_Min_ارزښت());
      MaxVal = LHS.get_Signed_Max_ارزښت().sadd_sat(RHS.get_Signed_Max_ارزښت());
    } وإلا {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_ارزښت().ssub_sat(RHS.get_Signed_Max_ارزښت());
      MaxVal = LHS.get_Signed_Max_ارزښت().ssub_sat(RHS.get_Signed_Min_ارزښت());
    }
    إذا (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      بینښه عدد_بټونه_2 = MinVal.trunc(بټ_پلنوالی_2 - 1).countl_one();
      معلوم_بهر.One.سېټ_بټونه(بټ_پلنوالی_2 - 1 - عدد_بټونه_2, بټ_پلنوالی_2 - 1);
      معلوم_بهر.Zero.سېټ_Sign_بټ();
    }
    إذا (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      بینښه عدد_بټونه_2 = MaxVal.trunc(بټ_پلنوالی_2 - 1).countl_zero();
      معلوم_بهر.Zero.سېټ_بټونه(بټ_پلنوالی_2 - 1 - عدد_بټونه_2, بټ_پلنوالی_2 - 1);
      معلوم_بهر.One.سېټ_Sign_بټ();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  إذا (معلوم_بهر.hasConflict())
    معلوم_بهر.سېټ_All_Zero();
  أرجع معلوم_بهر;
}

معلوم_بټونه معلوم_بټونه::computeForSubBorrow(ثابت معلوم_بټونه &LHS, معلوم_بټونه RHS,
                                         ثابت معلوم_بټونه &Borrow) {
  أكد(Borrow.get_بټ_پلنوالی() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::بدلون(RHS.Zero, RHS.One);
  أرجع ::compute_For_جمع_کړه_لېږد(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_ارزښت(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_ارزښت());
}

معلوم_بټونه معلوم_بټونه::truncSSat(بینښه بټ_پلنوالی_2) ثابت {
  بینښه ننوت_بټونه = get_بټ_پلنوالی();
  APInt Min_دننه_Range = APInt::get_Signed_Min_ارزښت(بټ_پلنوالی_2).sext(ننوت_بټونه);
  APInt Max_دننه_Range = APInt::get_Signed_Max_ارزښت(بټ_پلنوالی_2).sext(ننوت_بټونه);
  APInt ننوت_Min = get_Signed_Min_ارزښت();
  APInt ننوت_Max = get_Signed_Max_ارزښت();
  معلوم_بټونه معلوم(بټ_پلنوالی_2);

  // Case 1: All values fit - just truncate
  إذا (ننوت_Min.sge(Min_دننه_Range) && ننوت_Max.sle(Max_دننه_Range)) {
    معلوم = trunc(بټ_پلنوالی_2);
  }
  // Case 2: All saturate to min
  وإلا إذا (ننوت_Max.slt(Min_دننه_Range)) {
    معلوم = معلوم_بټونه::makeConstant(APInt::get_Signed_Min_ارزښت(بټ_پلنوالی_2));
  }
  // Case 3: All saturate to max
  وإلا إذا (ننوت_Min.sgt(Max_دننه_Range)) {
    معلوم = معلوم_بټونه::makeConstant(APInt::get_Signed_Max_ارزښت(بټ_پلنوالی_2));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  وإلا إذا (ننوت_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    معلوم.Zero = APInt(بټ_پلنوالی_2, 0);
    معلوم.Zero.سېټ_بټ(بټ_پلنوالی_2 - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    معلوم.One = One.trunc(بټ_پلنوالی_2);
    معلوم.One.clear_بټ(بټ_پلنوالی_2 - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  وإلا إذا (ننوت_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    معلوم.Zero = Zero.trunc(بټ_پلنوالی_2);
    معلوم.Zero.clear_بټ(بټ_پلنوالی_2 - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    معلوم.One = APInt(بټ_پلنوالی_2, 0);
    معلوم.One.سېټ_بټ(بټ_پلنوالی_2 - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  وإلا {
    // Output: min saturation, truncated, or max saturation
    APInt ننوت_Zero_ښکتنی = Zero.trunc(بټ_پلنوالی_2);
    APInt ننوت_One_ښکتنی = One.trunc(بټ_پلنوالی_2);
    APInt MinSat = APInt::get_Signed_Min_ارزښت(بټ_پلنوالی_2);
    APInt MaxSat = APInt::get_Signed_Max_ارزښت(بټ_پلنوالی_2);
    معلوم_بټونه MinSatKB = معلوم_بټونه::makeConstant(MinSat);
    معلوم_بټونه MaxSatKB = معلوم_بټونه::makeConstant(MaxSat);
    إذا (ننوت_Max.sle(Max_دننه_Range)) {
      // Positive values fit, only negatives saturate to min
      معلوم.Zero = ننوت_Zero_ښکتنی & MinSatKB.Zero;
      معلوم.One = ننوت_One_ښکتنی & MinSatKB.One;
    } وإلا إذا (ننوت_Min.sge(Min_دننه_Range)) {
      // Negative values fit, only positives saturate to max
      معلوم.Zero = ننوت_Zero_ښکتنی & MaxSatKB.Zero;
      معلوم.One = ننوت_One_ښکتنی & MaxSatKB.One;
    } وإلا {
      // Both positive and negative values might saturate
      معلوم.Zero = ننوت_Zero_ښکتنی & MinSatKB.Zero & MaxSatKB.Zero;
      معلوم.One = ننوت_One_ښکتنی & MinSatKB.One & MaxSatKB.One;
    }
  }
  أرجع معلوم;
}

معلوم_بټونه معلوم_بټونه::truncSSatU(بینښه بټ_پلنوالی_2) ثابت {
  بینښه ننوت_بټونه = get_بټ_پلنوالی();
  APInt Max_دننه_Range = APInt::getAllOnes(بټ_پلنوالی_2).zext(ننوت_بټونه);
  APInt ننوت_Min = get_Signed_Min_ارزښت();
  APInt ننوت_Max = get_Signed_Max_ارزښت();
  معلوم_بټونه معلوم(بټ_پلنوالی_2);

  إذا (isNegative()) {
    معلوم.سېټ_All_Zero();
  } وإلا إذا (ننوت_Min.isNonNegative() && ننوت_Max.ule(Max_دننه_Range)) {
    معلوم = trunc(بټ_پلنوالی_2);
  } وإلا إذا (ننوت_Min.isNonNegative() && ننوت_Min.ugt(Max_دننه_Range)) {
    معلوم.سېټ_All_Ones();
  } وإلا إذا (ننوت_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    معلوم.Zero = APInt(بټ_پلنوالی_2, 0);
    // Common one bits: bits that are 1 in input stay 1
    معلوم.One = One.trunc(بټ_پلنوالی_2);
  } وإلا {
    // Mixed: sign bit unknown
    إذا (ننوت_Max.ule(Max_دننه_Range)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      معلوم.Zero = Zero.trunc(بټ_پلنوالی_2);
      معلوم.One = APInt(بټ_پلنوالی_2, 0);
    } وإلا {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      معلوم.Zero = APInt(بټ_پلنوالی_2, 0);
      معلوم.One = APInt(بټ_پلنوالی_2, 0);
    }
  }
  أرجع معلوم;
}

معلوم_بټونه معلوم_بټونه::truncUSat(بینښه بټ_پلنوالی_2) ثابت {
  بینښه ننوت_بټونه = get_بټ_پلنوالی();
  APInt Max_دننه_Range = APInt::get_ټیټ_بټونه_سېټ(ننوت_بټونه, بټ_پلنوالی_2);
  APInt ننوت_Max = get_Max_ارزښت();
  APInt ننوت_Min = get_Min_ارزښت();
  معلوم_بټونه معلوم(بټ_پلنوالی_2);

  إذا (ننوت_Max.ule(Max_دننه_Range)) {
    معلوم = trunc(بټ_پلنوالی_2);
  } وإلا إذا (ننوت_Min.ugt(Max_دننه_Range)) {
    معلوم.سېټ_All_Ones();
  } وإلا {
    معلوم.resetAll();
    معلوم.One = One.trunc(بټ_پلنوالی_2);
  }
  أرجع معلوم;
}

معلوم_بټونه معلوم_بټونه::sext_دننه_Reg(بینښه Src_بټ_پلنوالی) ثابت {
  بینښه بټ_پلنوالی_2 = get_بټ_پلنوالی();
  أكد(0 < Src_بټ_پلنوالی && Src_بټ_پلنوالی <= بټ_پلنوالی_2 &&
         "Illegal sext-in-register");

  إذا (Src_بټ_پلنوالی == بټ_پلنوالی_2)
    أرجع *هذا;

  بینښه Ext_بټونه = بټ_پلنوالی_2 - Src_بټ_پلنوالی;
  معلوم_بټونه پایله;
  پایله.One = One << Ext_بټونه;
  پایله.Zero = Zero << Ext_بټونه;
  پایله.One.ashr_دننه_ځای_پرځای_کړه(Ext_بټونه);
  پایله.Zero.ashr_دننه_ځای_پرځای_کړه(Ext_بټونه);
  أرجع پایله;
}

معلوم_بټونه معلوم_بټونه::makeGE(ثابت APInt &Val) ثابت {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  بینښه N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_ټیټ_بټونه(get_بټ_پلنوالی() - N);
  أرجع معلوم_بټونه(Zero, One | MaskedVal);
}

معلوم_بټونه معلوم_بټونه::umax(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  إذا (LHS.get_Min_ارزښت().uge(RHS.get_Max_ارزښت()))
    أرجع LHS;
  إذا (RHS.get_Min_ارزښت().uge(LHS.get_Max_ارزښت()))
    أرجع RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  معلوم_بټونه L = LHS.makeGE(RHS.get_Min_ارزښت());
  معلوم_بټونه R = RHS.makeGE(LHS.get_Min_ارزښت());
  أرجع L.intersectWith(R);
}

معلوم_بټونه معلوم_بټونه::umin(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  خپلکاری Flip = [](ثابت معلوم_بټونه &Val) { أرجع معلوم_بټونه(Val.One, Val.Zero); };
  أرجع Flip(umax(Flip(LHS), Flip(RHS)));
}

معلوم_بټونه معلوم_بټونه::smax(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  أرجع flip_Sign_بټ(umax(flip_Sign_بټ(LHS), flip_Sign_بټ(RHS)));
}

معلوم_بټونه معلوم_بټونه::smin(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  خپلکاری Flip = [](ثابت معلوم_بټونه &Val) {
    بینښه Sign_بټ_Position = Val.get_بټ_پلنوالی() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.سېټ_بټ_Val(Sign_بټ_Position, Val.Zero[Sign_بټ_Position]);
    One.سېټ_بټ_Val(Sign_بټ_Position, Val.One[Sign_بټ_Position]);
    أرجع معلوم_بټونه(Zero, One);
  };
  أرجع Flip(umax(Flip(LHS), Flip(RHS)));
}

معلوم_بټونه معلوم_بټونه::abdu(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  إذا (LHS.get_Min_ارزښت().uge(RHS.get_Max_ارزښت()))
    أرجع compute_For_جمع_کړه_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, LHS,
                            RHS);
  إذا (RHS.get_Min_ارزښت().uge(LHS.get_Max_ارزښت()))
    أرجع compute_For_جمع_کړه_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  معلوم_بټونه Diff0 =
      compute_For_جمع_کړه_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/ریښتیا, LHS, RHS);
  معلوم_بټونه Diff1 =
      compute_For_جمع_کړه_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/ریښتیا, RHS, LHS);
  أرجع Diff0.intersectWith(Diff1);
}

معلوم_بټونه معلوم_بټونه::abds(معلوم_بټونه LHS, معلوم_بټونه RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  إذا (LHS.get_Signed_Min_ارزښت().sge(RHS.get_Signed_Max_ارزښت()))
    أرجع compute_For_جمع_کړه_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, LHS,
                            RHS);
  إذا (RHS.get_Signed_Min_ارزښت().sge(LHS.get_Signed_Max_ارزښت()))
    أرجع compute_For_جمع_کړه_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  بینښه Sign_بټ_Position = LHS.get_بټ_پلنوالی() - 1;
  لكل (خپلکاری Arg : {&LHS, &RHS}) {
    منطقي Tmp = Arg->Zero[Sign_بټ_Position];
    Arg->Zero.سېټ_بټ_Val(Sign_بټ_Position, Arg->One[Sign_بټ_Position]);
    Arg->One.سېټ_بټ_Val(Sign_بټ_Position, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  معلوم_بټونه Diff0 =
      compute_For_جمع_کړه_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/ریښتیا, LHS, RHS);
  معلوم_بټونه Diff1 =
      compute_For_جمع_کړه_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/ریښتیا, RHS, LHS);
  أرجع Diff0.intersectWith(Diff1);
}

ساكن بینښه get_Max_ښويدنه_Amount(ثابت APInt &Max_ارزښت, بینښه بټ_پلنوالی_2) {
  إذا (isPowerOf2_32(بټ_پلنوالی_2))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    أرجع Max_ارزښت.ایستل_بټونه_As_ZExt_ارزښت(
        std::min(Log2_32(بټ_پلنوالی_2), Max_ارزښت.get_بټ_پلنوالی()), 0);
  // This is only an approximate upper bound.
  أرجع Max_ارزښت.get_Limited_ارزښت(بټ_پلنوالی_2 - 1);
}

معلوم_بټونه معلوم_بټونه::shl(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS, منطقي NUW,
                         منطقي NSW, منطقي ShAmtNonZero) {
  بینښه بټ_پلنوالی_2 = LHS.get_بټ_پلنوالی();
  خپلکاری ښويدنه_By_Const = [&](ثابت معلوم_بټونه &LHS, بینښه ښويدنه_Amt_2) {
    معلوم_بټونه معلوم;
    منطقي Shifted_بهر_Zero, Shifted_بهر_One;
    معلوم.Zero = LHS.Zero.ushl_ov(ښويدنه_Amt_2, Shifted_بهر_Zero);
    معلوم.Zero.سېټ_ټیټ_بټونه(ښويدنه_Amt_2);
    معلوم.One = LHS.One.ushl_ov(ښويدنه_Amt_2, Shifted_بهر_One);

    // All cases returning poison have been handled by MaxShiftAmount already.
    إذا (NSW) {
      إذا (NUW && ښويدنه_Amt_2 != 0)
        // NUW means we can assume anything shifted out was a zero.
        Shifted_بهر_Zero = ریښتیا;

      إذا (Shifted_بهر_Zero)
        معلوم.makeNonNegative();
      وإلا إذا (Shifted_بهر_One)
        معلوم.makeNegative();
    }
    أرجع معلوم;
  };

  // Fast path for a common case when LHS is completely unknown.
  معلوم_بټونه معلوم(بټ_پلنوالی_2);
  بینښه Min_ښويدنه_Amount = RHS.get_Min_ارزښت().get_Limited_ارزښت(بټ_پلنوالی_2);
  إذا (Min_ښويدنه_Amount == 0 && ShAmtNonZero)
    Min_ښويدنه_Amount = 1;
  إذا (LHS.isUnknown()) {
    معلوم.Zero.سېټ_ټیټ_بټونه(Min_ښويدنه_Amount);
    إذا (NUW && NSW && Min_ښويدنه_Amount != 0)
      معلوم.makeNonNegative();
    أرجع معلوم;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_ارزښت = RHS.get_Max_ارزښت();
  بینښه Max_ښويدنه_Amount = get_Max_ښويدنه_Amount(Max_ارزښت, بټ_پلنوالی_2);
  إذا (NUW && NSW)
    Max_ښويدنه_Amount = std::min(Max_ښويدنه_Amount, LHS.شمېر_Max_Leading_Zeros() - 1);
  إذا (NUW)
    Max_ښويدنه_Amount = std::min(Max_ښويدنه_Amount, LHS.شمېر_Max_Leading_Zeros());
  إذا (NSW)
    Max_ښويدنه_Amount = std::min(
        Max_ښويدنه_Amount,
        std::max(LHS.شمېر_Max_Leading_Zeros(), LHS.شمېر_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  إذا (Min_ښويدنه_Amount == 0 && Max_ښويدنه_Amount == بټ_پلنوالی_2 - 1 &&
      isPowerOf2_32(بټ_پلنوالی_2)) {
    معلوم.Zero.سېټ_ټیټ_بټونه(LHS.شمېر_Min_Trailing_Zeros());
    إذا (LHS.isAllOnes())
      معلوم.One.سېټ_Sign_بټ();
    إذا (NSW) {
      إذا (LHS.isNonNegative())
        معلوم.makeNonNegative();
      إذا (LHS.isNegative())
        معلوم.makeNegative();
    }
    أرجع معلوم;
  }

  // Find the common bits from all possible shifts.
  بینښه ښويدنه_Amt_Zero_ماسک = RHS.Zero.zextOrTrunc(32).get_ZExt_ارزښت();
  بینښه ښويدنه_Amt_One_ماسک = RHS.One.zextOrTrunc(32).get_ZExt_ارزښت();
  معلوم.سېټ_All_Conflict();
  لكل (بینښه ښويدنه_Amt_2 = Min_ښويدنه_Amount; ښويدنه_Amt_2 <= Max_ښويدنه_Amount;
       ++ښويدنه_Amt_2) {
    // Skip if the shift amount is impossible.
    إذا ((ښويدنه_Amt_Zero_ماسک & ښويدنه_Amt_2) != 0 ||
        (ښويدنه_Amt_One_ماسک | ښويدنه_Amt_2) != ښويدنه_Amt_2)
      استمر;
    معلوم = معلوم.intersectWith(ښويدنه_By_Const(LHS, ښويدنه_Amt_2));
    إذا (معلوم.isUnknown())
      اكسر;
  }

  // All shift amounts may result in poison.
  إذا (معلوم.hasConflict())
    معلوم.سېټ_All_Zero();
  أرجع معلوم;
}

معلوم_بټونه معلوم_بټونه::lshr(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS,
                          منطقي ShAmtNonZero, منطقي Exact) {
  بینښه بټ_پلنوالی_2 = LHS.get_بټ_پلنوالی();
  خپلکاری ښويدنه_By_Const = [&](ثابت معلوم_بټونه &LHS, بینښه ښويدنه_Amt_2) {
    معلوم_بټونه معلوم = LHS;
    معلوم >>= ښويدنه_Amt_2;
    // High bits are known zero.
    معلوم.Zero.سېټ_لوړ_بټونه(ښويدنه_Amt_2);
    أرجع معلوم;
  };

  // Fast path for a common case when LHS is completely unknown.
  معلوم_بټونه معلوم(بټ_پلنوالی_2);
  بینښه Min_ښويدنه_Amount = RHS.get_Min_ارزښت().get_Limited_ارزښت(بټ_پلنوالی_2);
  إذا (Min_ښويدنه_Amount == 0 && ShAmtNonZero)
    Min_ښويدنه_Amount = 1;
  إذا (LHS.isUnknown()) {
    معلوم.Zero.سېټ_لوړ_بټونه(Min_ښويدنه_Amount);
    أرجع معلوم;
  }

  // Find the common bits from all possible shifts.
  APInt Max_ارزښت = RHS.get_Max_ارزښت();
  بینښه Max_ښويدنه_Amount = get_Max_ښويدنه_Amount(Max_ارزښت, بټ_پلنوالی_2);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  إذا (Exact) {
    بینښه لومړی_One = LHS.شمېر_Max_Trailing_Zeros();
    إذا (لومړی_One < Min_ښويدنه_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      معلوم.سېټ_All_Zero();
      أرجع معلوم;
    }
    Max_ښويدنه_Amount = std::min(Max_ښويدنه_Amount, لومړی_One);
  }

  بینښه ښويدنه_Amt_Zero_ماسک = RHS.Zero.zextOrTrunc(32).get_ZExt_ارزښت();
  بینښه ښويدنه_Amt_One_ماسک = RHS.One.zextOrTrunc(32).get_ZExt_ارزښت();
  معلوم.سېټ_All_Conflict();
  لكل (بینښه ښويدنه_Amt_2 = Min_ښويدنه_Amount; ښويدنه_Amt_2 <= Max_ښويدنه_Amount;
       ++ښويدنه_Amt_2) {
    // Skip if the shift amount is impossible.
    إذا ((ښويدنه_Amt_Zero_ماسک & ښويدنه_Amt_2) != 0 ||
        (ښويدنه_Amt_One_ماسک | ښويدنه_Amt_2) != ښويدنه_Amt_2)
      استمر;
    معلوم = معلوم.intersectWith(ښويدنه_By_Const(LHS, ښويدنه_Amt_2));
    إذا (معلوم.isUnknown())
      اكسر;
  }

  // All shift amounts may result in poison.
  إذا (معلوم.hasConflict())
    معلوم.سېټ_All_Zero();
  أرجع معلوم;
}

معلوم_بټونه معلوم_بټونه::ashr(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS,
                          منطقي ShAmtNonZero, منطقي Exact) {
  بینښه بټ_پلنوالی_2 = LHS.get_بټ_پلنوالی();
  خپلکاری ښويدنه_By_Const = [&](ثابت معلوم_بټونه &LHS, بینښه ښويدنه_Amt_2) {
    معلوم_بټونه معلوم = LHS;
    معلوم.Zero.ashr_دننه_ځای_پرځای_کړه(ښويدنه_Amt_2);
    معلوم.One.ashr_دننه_ځای_پرځای_کړه(ښويدنه_Amt_2);
    أرجع معلوم;
  };

  // Fast path for a common case when LHS is completely unknown.
  معلوم_بټونه معلوم(بټ_پلنوالی_2);
  بینښه Min_ښويدنه_Amount = RHS.get_Min_ارزښت().get_Limited_ارزښت(بټ_پلنوالی_2);
  إذا (Min_ښويدنه_Amount == 0 && ShAmtNonZero)
    Min_ښويدنه_Amount = 1;
  إذا (LHS.isUnknown()) {
    إذا (Min_ښويدنه_Amount == بټ_پلنوالی_2) {
      // Always poison. Return zero because we don't like returning conflict.
      معلوم.سېټ_All_Zero();
      أرجع معلوم;
    }
    أرجع معلوم;
  }

  // Find the common bits from all possible shifts.
  APInt Max_ارزښت = RHS.get_Max_ارزښت();
  بینښه Max_ښويدنه_Amount = get_Max_ښويدنه_Amount(Max_ارزښت, بټ_پلنوالی_2);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  إذا (Exact) {
    بینښه لومړی_One = LHS.شمېر_Max_Trailing_Zeros();
    إذا (لومړی_One < Min_ښويدنه_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      معلوم.سېټ_All_Zero();
      أرجع معلوم;
    }
    Max_ښويدنه_Amount = std::min(Max_ښويدنه_Amount, لومړی_One);
  }

  بینښه ښويدنه_Amt_Zero_ماسک = RHS.Zero.zextOrTrunc(32).get_ZExt_ارزښت();
  بینښه ښويدنه_Amt_One_ماسک = RHS.One.zextOrTrunc(32).get_ZExt_ارزښت();
  معلوم.سېټ_All_Conflict();
  لكل (بینښه ښويدنه_Amt_2 = Min_ښويدنه_Amount; ښويدنه_Amt_2 <= Max_ښويدنه_Amount;
      ++ښويدنه_Amt_2) {
    // Skip if the shift amount is impossible.
    إذا ((ښويدنه_Amt_Zero_ماسک & ښويدنه_Amt_2) != 0 ||
        (ښويدنه_Amt_One_ماسک | ښويدنه_Amt_2) != ښويدنه_Amt_2)
      استمر;
    معلوم = معلوم.intersectWith(ښويدنه_By_Const(LHS, ښويدنه_Amt_2));
    إذا (معلوم.isUnknown())
      اكسر;
  }

  // All shift amounts may result in poison.
  إذا (معلوم.hasConflict())
    معلوم.سېټ_All_Zero();
  أرجع معلوم;
}

معلوم_بټونه معلوم_بټونه::fshl(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS,
                          ثابت APInt &Amt) {
  أرجع معلوم_بټونه(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

معلوم_بټونه معلوم_بټونه::fshr(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS,
                          ثابت APInt &Amt) {
  أرجع معلوم_بټونه(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

معلوم_بټونه معلوم_بټونه::clmul(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  معلوم_بټونه Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_ارزښت(), RHS.get_Min_ارزښت()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  خپلکاری ClMulOr = [](ثابت APInt &LHS, ثابت APInt &RHS) {
    بینښه BW = LHS.get_بټ_پلنوالی();
    أكد(BW == RHS.get_بټ_پلنوالی() && "Operand mismatch");
    APInt پایله(BW, 0);
    لكل (بینښه I :
         seq(std::min(RHS.get_فعال_بټونه(), BW - LHS.countr_zero())))
      إذا (RHS[I])
        پایله |= LHS << I;
    أرجع پایله;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt معلوم = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= معلوم;
  Res.One &= معلوم;

  أرجع Res;
}

معلوم_بټونه معلوم_بټونه::pext(ثابت معلوم_بټونه &Val, ثابت معلوم_بټونه &ماسک) {
  بینښه بټ_پلنوالی_2 = Val.get_بټ_پلنوالی();
  معلوم_بټونه Res(بټ_پلنوالی_2);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.سېټ_All_Conflict();

  // For each source position I where Mask[I] could be 1, the output position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible output positions widens.
  بینښه M0 = 0, M1 = 0;
  لكل (بینښه I = 0; I < بټ_پلنوالی_2; ++I) {
    إذا (!ماسک.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      إذا (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clear_بټونه(M0, M1 + 1);
      إذا (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clear_بټونه(M0, M1 + 1);
    }
    إذا (ماسک.One[I])
      ++M0, ++M1;
    وإلا إذا (!ماسک.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clear_بټونه(M0, بټ_پلنوالی_2);
  أرجع Res;
}

معلوم_بټونه معلوم_بټونه::pdep(ثابت معلوم_بټونه &Val, ثابت معلوم_بټونه &ماسک) {
  بینښه بټ_پلنوالی_2 = Val.get_بټ_پلنوالی();
  معلوم_بټونه Res(بټ_پلنوالی_2);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.سېټ_All_Conflict();

  // For each output position I where Mask[I] could be 1, the source position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible source positions widens.
  بینښه M0 = 0, M1 = 0;
  لكل (بینښه I = 0; I < بټ_پلنوالی_2; ++I) {
    إذا (!ماسک.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clear_بټ(I);
    إذا (!ماسک.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::get_بټونه_سېټ(بټ_پلنوالی_2, M0, M1 + 1);
      إذا (!Range.is_Subset_د(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clear_بټ(I);
      إذا (!Range.is_Subset_د(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clear_بټ(I);
    }
    إذا (ماسک.One[I])
      ++M0, ++M1;
    وإلا إذا (!ماسک.Zero[I])
      ++M1;
  }
  أرجع Res;
}

std::optional<منطقي> معلوم_بټونه::eq(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  إذا (LHS.isConstant() && RHS.isConstant())
    أرجع LHS.getConstant() == RHS.getConstant();
  إذا (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    أرجع خطأ;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بټونه::ne(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  إذا (std::optional<منطقي> معلوم_EQ = eq(LHS, RHS))
    أرجع !*معلوم_EQ;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بټونه::ugt(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  إذا (LHS.get_Max_ارزښت().ule(RHS.get_Min_ارزښت()))
    أرجع خطأ;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  إذا (LHS.get_Min_ارزښت().ugt(RHS.get_Max_ارزښت()))
    أرجع ریښتیا;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بټونه::uge(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  إذا (std::optional<منطقي> IsUGT = ugt(RHS, LHS))
    أرجع !*IsUGT;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بټونه::ult(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  أرجع ugt(RHS, LHS);
}

std::optional<منطقي> معلوم_بټونه::ule(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  أرجع uge(RHS, LHS);
}

std::optional<منطقي> معلوم_بټونه::sgt(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  إذا (LHS.get_Signed_Max_ارزښت().sle(RHS.get_Signed_Min_ارزښت()))
    أرجع خطأ;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  إذا (LHS.get_Signed_Min_ارزښت().sgt(RHS.get_Signed_Max_ارزښت()))
    أرجع ریښتیا;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بټونه::sge(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  إذا (std::optional<منطقي> معلوم_SGT = sgt(RHS, LHS))
    أرجع !*معلوم_SGT;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بټونه::slt(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  أرجع sgt(RHS, LHS);
}

std::optional<منطقي> معلوم_بټونه::sle(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  أرجع sge(RHS, LHS);
}

معلوم_بټونه معلوم_بټونه::مطلق_قیمت(منطقي IntMinIsPoison) ثابت {
  // If the source's MSB is zero then we know the rest of the bits already.
  إذا (isNonNegative())
    أرجع *هذا;

  // Absolute value preserves trailing zero count.
  معلوم_بټونه معلوم_Abs(get_بټ_پلنوالی());

  // If the input is negative, then abs(x) == -x.
  إذا (isNegative()) {
    معلوم_بټونه Tmp = *هذا;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    إذا (IntMinIsPoison && (Zero.popcount() + 2) == get_بټ_پلنوالی())
      Tmp.One.سېټ_بټ(شمېر_Min_Trailing_Zeros());

    معلوم_Abs = compute_For_جمع_کړه_Sub(
        /*جمع_کړه*/ خطأ, IntMinIsPoison, /*NUW=*/خطأ,
        معلوم_بټونه::makeConstant(APInt(get_بټ_پلنوالی(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    إذا (IntMinIsPoison && Tmp.شمېر_Min_Population() == 1 &&
        Tmp.شمېر_Max_Population() != 1) {
      Tmp.One.clear_Sign_بټ();
      Tmp.Zero.سېټ_Sign_بټ();
      معلوم_Abs.One.سېټ_بټونه(get_بټ_پلنوالی() - Tmp.شمېر_Min_Leading_Zeros(),
                           get_بټ_پلنوالی() - 1);
    }

  } وإلا {
    بینښه MaxTZ = شمېر_Max_Trailing_Zeros();
    بینښه MinTZ = شمېر_Min_Trailing_Zeros();

    معلوم_Abs.Zero.سېټ_ټیټ_بټونه(MinTZ);
    // If we know the lowest set 1, then preserve it.
    إذا (MaxTZ == MinTZ && MaxTZ < get_بټ_پلنوالی())
      معلوم_Abs.One.سېټ_بټ(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    إذا (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_ارزښت())) {
      معلوم_Abs.One.clear_Sign_بټ();
      معلوم_Abs.Zero.سېټ_Sign_بټ();
    }
  }

  أرجع معلوم_Abs;
}

معلوم_بټونه معلوم_بټونه::reduce_جمع_کړه(بینښه عدد_Elts) ثابت {
  إذا (عدد_Elts == 0)
    أرجع معلوم_بټونه(get_بټ_پلنوالی());

  بینښه بټ_پلنوالی_2 = get_بټ_پلنوالی();
  معلوم_بټونه پایله(بټ_پلنوالی_2);

  إذا (isConstant())
    // If all elements are the same constant, we can simply compute it
    أرجع معلوم_بټونه::makeConstant(عدد_Elts * getConstant());

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
  بینښه ورک_بټونه = Log2_32_Ceil(عدد_Elts);

  إذا (isNonNegative()) {
    بینښه LeadingZeros = شمېر_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > ورک_بټونه ? LeadingZeros - ورک_بټونه : 0;
    پایله.Zero.سېټ_لوړ_بټونه(LeadingZeros);
  } وإلا إذا (isNegative()) {
    بینښه LeadingOnes = شمېر_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > ورک_بټونه ? LeadingOnes - ورک_بټونه : 0;
    پایله.One.سېټ_لوړ_بټونه(LeadingOnes);
  }

  أرجع پایله;
}

ساكن معلوم_بټونه compute_For_Sat_جمع_کړه_Sub(منطقي جمع_کړه_2, منطقي Signed,
                                     ثابت معلوم_بټونه &LHS,
                                     ثابت معلوم_بټونه &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  بینښه بټ_پلنوالی_2 = LHS.get_بټ_پلنوالی();

  std::optional<منطقي> ډکېدنه;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  منطقي MayNegClamp = ریښتیا;
  منطقي MayPosClamp = ریښتیا;
  إذا (Signed) {
    // Easy cases we can rule out any overflow.
    إذا (جمع_کړه_2 && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      ډکېدنه = خطأ;
    وإلا إذا (!جمع_کړه_2 && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      ډکېدنه = خطأ;
    وإلا {
      // Check if we may overflow. If we can't rule out overflow then check if
      // we can rule out a direction at least.
      معلوم_بټونه UnsignedLHS = LHS;
      معلوم_بټونه UnsignedRHS = RHS;
      // Get version of LHS/RHS with clearer signbit. This allows us to detect
      // how the addition/subtraction might overflow into the signbit. Then
      // using the actual known signbits of LHS/RHS, we can figure out which
      // overflows are/aren't possible.
      UnsignedLHS.One.clear_Sign_بټ();
      UnsignedLHS.Zero.سېټ_Sign_بټ();
      UnsignedRHS.One.clear_Sign_بټ();
      UnsignedRHS.Zero.سېټ_Sign_بټ();
      معلوم_بټونه Res =
          معلوم_بټونه::compute_For_جمع_کړه_Sub(جمع_کړه_2, /*NSW=*/خطأ,
                                      /*NUW=*/خطأ, UnsignedLHS, UnsignedRHS);
      إذا (جمع_کړه_2) {
        إذا (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = خطأ;
          // Pos + Pos will overflow with extra signbit.
          إذا (LHS.isNonNegative() && RHS.isNonNegative())
            ډکېدنه = ریښتیا;
        } وإلا إذا (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = خطأ;
          // Neg + Neg will overflow without extra signbit.
          إذا (LHS.isNegative() && RHS.isNegative())
            ډکېدنه = ریښتیا;
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
            ډکېدنه = ریښتیا;
        } وإلا إذا (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = خطأ;
          // Pos - Neg will overflow without extra signbit.
          إذا (LHS.isNonNegative() && RHS.isNegative())
            ډکېدنه = ریښتیا;
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
      ډکېدنه = خطأ;
  } وإلا إذا (جمع_کړه_2) {
    // uadd.sat
    منطقي د;
    (فراغ)LHS.get_Max_ارزښت().uadd_ov(RHS.get_Max_ارزښت(), د);
    إذا (!د) {
      ډکېدنه = خطأ;
    } وإلا {
      (فراغ)LHS.get_Min_ارزښت().uadd_ov(RHS.get_Min_ارزښت(), د);
      إذا (د)
        ډکېدنه = ریښتیا;
    }
  } وإلا {
    // usub.sat
    منطقي د;
    (فراغ)LHS.get_Min_ارزښت().usub_ov(RHS.get_Max_ارزښت(), د);
    إذا (!د) {
      ډکېدنه = خطأ;
    } وإلا {
      (فراغ)LHS.get_Max_ارزښت().usub_ov(RHS.get_Min_ارزښت(), د);
      إذا (د)
        ډکېدنه = ریښتیا;
    }
  }

  معلوم_بټونه Res = معلوم_بټونه::compute_For_جمع_کړه_Sub(جمع_کړه_2, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  إذا (ډکېدنه) {
    // We know whether or not we overflowed.
    إذا (!(*ډکېدنه)) {
      // No overflow.
      أرجع Res;
    }

    // We overflowed
    APInt C;
    إذا (Signed) {
      // sadd.sat / ssub.sat
      أكد(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_ارزښت(بټ_پلنوالی_2)
                           : APInt::get_Signed_Max_ارزښت(بټ_پلنوالی_2);
    } وإلا إذا (جمع_کړه_2) {
      // uadd.sat
      C = APInt::get_Max_ارزښت(بټ_پلنوالی_2);
    } وإلا {
      // uadd.sat
      C = APInt::get_Min_ارزښت(بټ_پلنوالی_2);
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
      Res.Zero.clear_ټیټ_بټونه(بټ_پلنوالی_2 - 1);
    إذا (MayNegClamp)
      Res.One.clear_ټیټ_بټونه(بټ_پلنوالی_2 - 1);
  } وإلا إذا (جمع_کړه_2) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clear_All_بټونه();
  } وإلا {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clear_All_بټونه();
  }

  أرجع Res;
}

معلوم_بټونه معلوم_بټونه::sadd_sat(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  أرجع compute_For_Sat_جمع_کړه_Sub(/*جمع_کړه*/ ریښتیا, /*Signed*/ ریښتیا, LHS, RHS);
}
معلوم_بټونه معلوم_بټونه::ssub_sat(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  أرجع compute_For_Sat_جمع_کړه_Sub(/*جمع_کړه*/ خطأ, /*Signed*/ ریښتیا, LHS, RHS);
}
معلوم_بټونه معلوم_بټونه::uadd_sat(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  أرجع compute_For_Sat_جمع_کړه_Sub(/*جمع_کړه*/ ریښتیا, /*Signed*/ خطأ, LHS, RHS);
}
معلوم_بټونه معلوم_بټونه::usub_sat(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  أرجع compute_For_Sat_جمع_کړه_Sub(/*جمع_کړه*/ خطأ, /*Signed*/ خطأ, LHS, RHS);
}

ساكن معلوم_بټونه avgComputeU(معلوم_بټونه LHS, معلوم_بټونه RHS, منطقي IsCeil) {
  بینښه بټ_پلنوالی_2 = LHS.get_بټ_پلنوالی();
  LHS = LHS.zext(بټ_پلنوالی_2 + 1);
  RHS = RHS.zext(بټ_پلنوالی_2 + 1);
  LHS =
      compute_For_جمع_کړه_لېږد(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.ایستل_بټونه(بټ_پلنوالی_2, 1);
  أرجع LHS;
}

معلوم_بټونه معلوم_بټونه::avgFloorS(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  أرجع flip_Sign_بټ(avgFloorU(flip_Sign_بټ(LHS), flip_Sign_بټ(RHS)));
}

معلوم_بټونه معلوم_بټونه::avgFloorU(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  أرجع avgComputeU(LHS, RHS, /*IsCeil=*/خطأ);
}

معلوم_بټونه معلوم_بټونه::avgCeilS(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  أرجع flip_Sign_بټ(avgCeilU(flip_Sign_بټ(LHS), flip_Sign_بټ(RHS)));
}

معلوم_بټونه معلوم_بټونه::avgCeilU(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  أرجع avgComputeU(LHS, RHS, /*IsCeil=*/ریښتیا);
}

معلوم_بټونه معلوم_بټونه::mul(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS,
                         منطقي NoUndefSelfMultiply) {
  بینښه بټ_پلنوالی_2 = LHS.get_بټ_پلنوالی();
  أكد(بټ_پلنوالی_2 == RHS.get_بټ_پلنوالی() && "Operand mismatch");
  أكد((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.get_Max_ارزښت();
  APInt UMaxRHS = RHS.get_Max_ارزښت();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  منطقي Has_ډکېدنه;
  APInt UMax_پایله = UMaxLHS.umul_ov(UMaxRHS, Has_ډکېدنه);
  بینښه LeadZ = Has_ډکېدنه ? 0 : UMax_پایله.countl_zero();

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
  بینښه Trail_بټونه_معلوم_LHS = (LHS.Zero | LHS.One).countr_one();
  بینښه Trail_بټونه_معلوم_RHS = (RHS.Zero | RHS.One).countr_one();
  بینښه TrailZeroLHS = LHS.شمېر_Min_Trailing_Zeros();
  بینښه TrailZeroRHS = RHS.شمېر_Min_Trailing_Zeros();
  بینښه TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  بینښه تر_ټولو_کوچنی_عملوند = std::min(Trail_بټونه_معلوم_LHS - TrailZeroLHS,
                                      Trail_بټونه_معلوم_RHS - TrailZeroRHS);
  بینښه پایله_بټونه_معلوم = std::min(تر_ټولو_کوچنی_عملوند + TrailZ, بټ_پلنوالی_2);

  // The lower ResultBitsKnown bits of this are known.
  APInt Bottom_معلوم = LHS.One * RHS.One;

  معلوم_بټونه Res(بټ_پلنوالی_2);
  Res.Zero.سېټ_لوړ_بټونه(LeadZ);
  Res.Zero |= (~Bottom_معلوم).get_Lo_بټونه(پایله_بټونه_معلوم);
  Res.One = Bottom_معلوم.get_Lo_بټونه(پایله_بټونه_معلوم);

  إذا (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    بینښه TwoTZP1 = 2 * TrailZeroLHS + 1;
    إذا (TwoTZP1 < بټ_پلنوالی_2)
      Res.Zero.سېټ_بټ(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    إذا (TrailZeroLHS < بټ_پلنوالی_2 && LHS.One[TrailZeroLHS]) {
      بینښه TwoTZP2 = TwoTZP1 + 1;
      إذا (TwoTZP2 < بټ_پلنوالی_2)
        Res.Zero.سېټ_بټ(TwoTZP2);
    }
  }

  أرجع Res;
}

معلوم_بټونه معلوم_بټونه::mulhs(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  بینښه بټ_پلنوالی_2 = LHS.get_بټ_پلنوالی();
  أكد(بټ_پلنوالی_2 == RHS.get_بټ_پلنوالی() && "Operand mismatch");
  معلوم_بټونه WideLHS = LHS.sext(2 * بټ_پلنوالی_2);
  معلوم_بټونه WideRHS = RHS.sext(2 * بټ_پلنوالی_2);
  أرجع mul(WideLHS, WideRHS).ایستل_بټونه(بټ_پلنوالی_2, بټ_پلنوالی_2);
}

معلوم_بټونه معلوم_بټونه::mulhu(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  بینښه بټ_پلنوالی_2 = LHS.get_بټ_پلنوالی();
  أكد(بټ_پلنوالی_2 == RHS.get_بټ_پلنوالی() && "Operand mismatch");
  معلوم_بټونه WideLHS = LHS.zext(2 * بټ_پلنوالی_2);
  معلوم_بټونه WideRHS = RHS.zext(2 * بټ_پلنوالی_2);
  أرجع mul(WideLHS, WideRHS).ایستل_بټونه(بټ_پلنوالی_2, بټ_پلنوالی_2);
}

ساكن معلوم_بټونه div_Compute_ټیټ_بټ(معلوم_بټونه معلوم, ثابت معلوم_بټونه &LHS,
                                  ثابت معلوم_بټونه &RHS, منطقي Exact) {

  إذا (!Exact)
    أرجع معلوم;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  إذا (LHS.One[0])
    معلوم.One.سېټ_بټ(0);

  صحيح MinTZ =
      (صحيح)LHS.شمېر_Min_Trailing_Zeros() - (صحيح)RHS.شمېر_Max_Trailing_Zeros();
  صحيح MaxTZ =
      (صحيح)LHS.شمېر_Max_Trailing_Zeros() - (صحيح)RHS.شمېر_Min_Trailing_Zeros();
  إذا (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    معلوم.Zero.سېټ_ټیټ_بټونه(MinTZ);
    إذا (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      معلوم.One.سېټ_بټ(MinTZ);
    }
  } وإلا إذا (MaxTZ < 0) {
    // Poison Result
    معلوم.سېټ_All_Zero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  إذا (معلوم.hasConflict())
    معلوم.سېټ_All_Zero();

  أرجع معلوم;
}

معلوم_بټونه معلوم_بټونه::sdiv(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS,
                          منطقي Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  إذا (LHS.isNonNegative() && RHS.isNonNegative())
    أرجع udiv(LHS, RHS, Exact);

  بینښه بټ_پلنوالی_2 = LHS.get_بټ_پلنوالی();
  معلوم_بټونه معلوم(بټ_پلنوالی_2);

  إذا (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    معلوم.سېټ_All_Zero();
    أرجع معلوم;
  }

  std::optional<APInt> Res;
  إذا (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_ارزښت();
    APInt عدد = LHS.get_Signed_Min_ارزښت();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (عدد.is_Min_Signed_ارزښت() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_ارزښت(بټ_پلنوالی_2)
              : عدد.sdiv(Denom);
  } وإلا إذا (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    إذا (Exact || (-LHS.get_Signed_Max_ارزښت()).uge(RHS.get_Signed_Max_ارزښت())) {
      APInt Denom = RHS.get_Signed_Min_ارزښت();
      APInt عدد = LHS.get_Signed_Min_ارزښت();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      إذا (Denom.isZero())
        Denom.سېټ_بټ(RHS.شمېر_Min_Trailing_Zeros());
      Res = عدد.sdiv(Denom);
    }
  } وإلا إذا (LHS.is_Strictly_مثبت() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    إذا (Exact || LHS.get_Signed_Min_ارزښت().uge(-RHS.get_Signed_Min_ارزښت())) {
      APInt Denom = RHS.get_Signed_Max_ارزښت();
      APInt عدد = LHS.get_Signed_Max_ارزښت();
      Res = عدد.sdiv(Denom);
    }
  }

  إذا (Res) {
    إذا (Res->isNonNegative()) {
      بینښه LeadZ = Res->شمېر_Leading_Zeros();
      معلوم.Zero.سېټ_لوړ_بټونه(LeadZ);
    } وإلا {
      بینښه LeadO = Res->شمېر_Leading_Ones();
      معلوم.One.سېټ_لوړ_بټونه(LeadO);
    }
  }

  معلوم = div_Compute_ټیټ_بټ(معلوم, LHS, RHS, Exact);
  أرجع معلوم;
}

معلوم_بټونه معلوم_بټونه::udiv(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS,
                          منطقي Exact) {
  بینښه بټ_پلنوالی_2 = LHS.get_بټ_پلنوالی();
  معلوم_بټونه معلوم(بټ_پلنوالی_2);

  إذا (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    معلوم.سېټ_All_Zero();
    أرجع معلوم;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_ارزښت();
  إذا (MinDenom.isZero())
    MinDenom.سېټ_بټ(RHS.شمېر_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_ارزښت().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_ارزښت().udiv(RHS.get_Max_ارزښت());

  معلوم = معلوم_بټونه::makeConstant(MinRes);
  إذا (std::optional<بینښه> Different_بټ =
          APIntOps::Get_Most_Significant_Different_بټ(MinRes, MaxRes)) {
    معلوم.Zero.clear_ټیټ_بټونه(*Different_بټ + 1);
    معلوم.One.clear_ټیټ_بټونه(*Different_بټ + 1);
  }

  معلوم = div_Compute_ټیټ_بټ(معلوم, LHS, RHS, Exact);

  أرجع معلوم;
}

معلوم_بټونه معلوم_بټونه::rem_Get_ټیټ_بټونه(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  بینښه بټ_پلنوالی_2 = LHS.get_بټ_پلنوالی();
  إذا (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    بینښه RHSZeros = RHS.شمېر_Min_Trailing_Zeros();
    APInt ماسک = APInt::get_ټیټ_بټونه_سېټ(بټ_پلنوالی_2, RHSZeros);
    APInt Ones_ماسک = LHS.One & ماسک;
    APInt Zeros_ماسک = LHS.Zero & ماسک;
    أرجع معلوم_بټونه(Zeros_ماسک, Ones_ماسک);
  }
  أرجع معلوم_بټونه(بټ_پلنوالی_2);
}

معلوم_بټونه معلوم_بټونه::urem(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  إذا (LHS.get_Max_ارزښت().ult(RHS.get_Min_ارزښت()))
    أرجع LHS;

  معلوم_بټونه معلوم = rem_Get_ټیټ_بټونه(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_ارزښت(), RHS.get_Max_ارزښت() - 1);
  معلوم.Zero.سېټ_لوړ_بټونه(MaxRes.شمېر_Leading_Zeros());
  أرجع معلوم;
}

معلوم_بټونه معلوم_بټونه::srem(ثابت معلوم_بټونه &LHS, ثابت معلوم_بټونه &RHS) {
  معلوم_بټونه معلوم = rem_Get_ټیټ_بټونه(LHS, RHS);
  إذا (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt ټیټ_بټونه = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    إذا (LHS.isNonNegative() || ټیټ_بټونه.is_Subset_د(LHS.Zero))
      معلوم.Zero |= ~ټیټ_بټونه;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    إذا (LHS.isNegative() && ټیټ_بټونه.intersects(LHS.One))
      معلوم.One |= ~ټیټ_بټونه;
    أرجع معلوم;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  إذا (LHS.isNegative() && معلوم.isNonZero())
    معلوم.One.سېټ_لوړ_بټونه(
        std::max(LHS.شمېر_Min_Leading_Ones(), RHS.شمېر_Min_Sign_بټونه()));
  وإلا إذا (LHS.isNonNegative())
    معلوم.Zero.سېټ_لوړ_بټونه(
        std::max(LHS.شمېر_Min_Leading_Zeros(), RHS.شمېر_Min_Sign_بټونه()));
  أرجع معلوم;
}

معلوم_بټونه &معلوم_بټونه::چلوونکی&=(ثابت معلوم_بټونه &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  أرجع *هذا;
}

معلوم_بټونه &معلوم_بټونه::چلوونکی|=(ثابت معلوم_بټونه &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  أرجع *هذا;
}

معلوم_بټونه &معلوم_بټونه::چلوونکی^=(ثابت معلوم_بټونه &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  أرجع *هذا;
}

معلوم_بټونه معلوم_بټونه::blsi() ثابت {
  بینښه بټ_پلنوالی_2 = get_بټ_پلنوالی();
  معلوم_بټونه معلوم(Zero, APInt(بټ_پلنوالی_2, 0));
  بینښه Max = شمېر_Max_Trailing_Zeros();
  معلوم.Zero.سېټ_بټونه_From(std::min(Max + 1, بټ_پلنوالی_2));
  بینښه Min = شمېر_Min_Trailing_Zeros();
  إذا (Max == Min && Max < بټ_پلنوالی_2)
    معلوم.One.سېټ_بټ(Max);
  أرجع معلوم;
}

معلوم_بټونه معلوم_بټونه::blsmsk() ثابت {
  بینښه بټ_پلنوالی_2 = get_بټ_پلنوالی();
  معلوم_بټونه معلوم(بټ_پلنوالی_2);
  بینښه Max = شمېر_Max_Trailing_Zeros();
  معلوم.Zero.سېټ_بټونه_From(std::min(Max + 1, بټ_پلنوالی_2));
  بینښه Min = شمېر_Min_Trailing_Zeros();
  معلوم.One.سېټ_ټیټ_بټونه(std::min(Min + 1, بټ_پلنوالی_2));
  أرجع معلوم;
}

فراغ معلوم_بټونه::print(raw_ostream &OS) ثابت {
  بینښه بټ_پلنوالی_2 = get_بټ_پلنوالی();
  لكل (بینښه I = 0; I < بټ_پلنوالی_2; ++I) {
    بینښه N = بټ_پلنوالی_2 - I - 1;
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
LLVM_DUMP_لاره فراغ معلوم_بټونه::dump() ثابت {
  print(dbgs());
  dbgs() << "\n";
}
#د_که_پای

بینښه llvm::Sign_بټونه_Ops::rot(بینښه Src_Sign_بټونه, بینښه بټ_پلنوالی_2,
                                std::optional<APInt> RotAmt,
                                منطقي Is_Rotate_ښی) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  إذا (Src_Sign_بټونه == بټ_پلنوالی_2)
    أرجع بټ_پلنوالی_2;

  إذا (!RotAmt)
    أرجع 1;

  بینښه Amt = RotAmt->urem(بټ_پلنوالی_2);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  إذا (Is_Rotate_ښی)
    Amt = (بټ_پلنوالی_2 - Amt) % بټ_پلنوالی_2;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  إذا (Src_Sign_بټونه > Amt + 1)
    أرجع Src_Sign_بټونه - Amt;
  أرجع 1;
}
