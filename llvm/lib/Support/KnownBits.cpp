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

استفاده نطاق llvm;

معلوم_بیتها معلوم_بیتها::flip_Sign_بیت(ثابت معلوم_بیتها &Val) {
  بیعلامت Sign_بیت_Position = Val.get_بیت_عرض() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.مجموعه_بیت_Val(Sign_بیت_Position, Val.One[Sign_بیت_Position]);
  One.مجموعه_بیت_Val(Sign_بیت_Position, Val.Zero[Sign_بیت_Position]);
  أرجع معلوم_بیتها(Zero, One);
}

ساكن معلوم_بیتها compute_For_جمع_کن_انتقال(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS,
                                    منطقي انتقال_Zero, منطقي انتقال_One) {

  APInt Possible_جمع_Zero = LHS.get_Max_مقدار() + RHS.get_Max_مقدار() + !انتقال_Zero;
  APInt Possible_جمع_One = LHS.get_Min_مقدار() + RHS.get_Min_مقدار() + انتقال_One;

  // Compute known bits of the carry.
  APInt انتقال_معلوم_Zero = ~(Possible_جمع_Zero ^ LHS.Zero ^ RHS.Zero);
  APInt انتقال_معلوم_One = Possible_جمع_One ^ LHS.One ^ RHS.One;

  // Compute set of known bits (where all three relevant bits are known).
  APInt LHSKnownUnion = LHS.Zero | LHS.One;
  APInt RHSKnownUnion = RHS.Zero | RHS.One;
  APInt انتقال_معلوم_Union = std::move(انتقال_معلوم_Zero) | انتقال_معلوم_One;
  APInt معلوم = std::move(LHSKnownUnion) & RHSKnownUnion & انتقال_معلوم_Union;

  // Compute known bits of the result.
  معلوم_بیتها معلوم_بیرون;
  معلوم_بیرون.Zero = ~std::move(Possible_جمع_Zero) & معلوم;
  معلوم_بیرون.One = std::move(Possible_جمع_One) & معلوم;
  أرجع معلوم_بیرون;
}

معلوم_بیتها معلوم_بیتها::compute_For_جمع_کن_انتقال(
    ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS, ثابت معلوم_بیتها &انتقال_2) {
  أكد(انتقال_2.get_بیت_عرض() == 1 && "Carry must be 1-bit");
  أرجع ::compute_For_جمع_کن_انتقال(
      LHS, RHS, انتقال_2.Zero.get_Bool_مقدار(), انتقال_2.One.get_Bool_مقدار());
}

معلوم_بیتها معلوم_بیتها::compute_For_جمع_کن_Sub(منطقي جمع_کن_2, منطقي NSW, منطقي NUW,
                                      ثابت معلوم_بیتها &LHS,
                                      ثابت معلوم_بیتها &RHS) {
  بیعلامت بیت_عرض_2 = LHS.get_بیت_عرض();
  معلوم_بیتها معلوم_بیرون(بیت_عرض_2);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  إذا (LHS.isUnknown() && RHS.isUnknown())
    أرجع معلوم_بیرون;

  إذا (!LHS.isUnknown() && !RHS.isUnknown()) {
    إذا (جمع_کن_2) {
      // Sum = LHS + RHS + 0
      معلوم_بیرون = ::compute_For_جمع_کن_انتقال(LHS, RHS, /*CarryZero=*/درست,
                                      /*CarryOne=*/خطأ);
    } وإلا {
      // Sum = LHS + ~RHS + 1
      معلوم_بیتها NotRHS = RHS;
      std::جابجا(NotRHS.Zero, NotRHS.One);
      معلوم_بیرون = ::compute_For_جمع_کن_انتقال(LHS, NotRHS, /*CarryZero=*/خطأ,
                                      /*CarryOne=*/درست);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  إذا (NUW) {
    إذا (جمع_کن_2) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_مقدار().uadd_sat(RHS.get_Min_مقدار());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      إذا (NSW) {
        بیعلامت رقم_بیتها_2 = MinVal.trunc(بیت_عرض_2 - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        معلوم_بیرون.One.مجموعه_بیتها(بیت_عرض_2 - 1 - رقم_بیتها_2, بیت_عرض_2 - 1);
      }
      معلوم_بیرون.One.مجموعه_بالا_بیتها(MinVal.countl_one());
    } وإلا {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_مقدار().usub_sat(RHS.get_Min_مقدار());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      إذا (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        بیعلامت رقم_بیتها_2 = MaxVal.trunc(بیت_عرض_2 - 1).countl_zero();
        معلوم_بیرون.Zero.مجموعه_بیتها(بیت_عرض_2 - 1 - رقم_بیتها_2, بیت_عرض_2 - 1);
      }
      معلوم_بیرون.Zero.مجموعه_بالا_بیتها(MaxVal.countl_zero());
    }
  }

  إذا (NSW) {
    APInt MinVal;
    APInt MaxVal;
    إذا (جمع_کن_2) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_مقدار().sadd_sat(RHS.get_Signed_Min_مقدار());
      MaxVal = LHS.get_Signed_Max_مقدار().sadd_sat(RHS.get_Signed_Max_مقدار());
    } وإلا {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_مقدار().ssub_sat(RHS.get_Signed_Max_مقدار());
      MaxVal = LHS.get_Signed_Max_مقدار().ssub_sat(RHS.get_Signed_Min_مقدار());
    }
    إذا (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      بیعلامت رقم_بیتها_2 = MinVal.trunc(بیت_عرض_2 - 1).countl_one();
      معلوم_بیرون.One.مجموعه_بیتها(بیت_عرض_2 - 1 - رقم_بیتها_2, بیت_عرض_2 - 1);
      معلوم_بیرون.Zero.مجموعه_Sign_بیت();
    }
    إذا (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      بیعلامت رقم_بیتها_2 = MaxVal.trunc(بیت_عرض_2 - 1).countl_zero();
      معلوم_بیرون.Zero.مجموعه_بیتها(بیت_عرض_2 - 1 - رقم_بیتها_2, بیت_عرض_2 - 1);
      معلوم_بیرون.One.مجموعه_Sign_بیت();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  إذا (معلوم_بیرون.hasConflict())
    معلوم_بیرون.مجموعه_All_Zero();
  أرجع معلوم_بیرون;
}

معلوم_بیتها معلوم_بیتها::computeForSubBorrow(ثابت معلوم_بیتها &LHS, معلوم_بیتها RHS,
                                         ثابت معلوم_بیتها &Borrow) {
  أكد(Borrow.get_بیت_عرض() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::جابجا(RHS.Zero, RHS.One);
  أرجع ::compute_For_جمع_کن_انتقال(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_مقدار(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_مقدار());
}

معلوم_بیتها معلوم_بیتها::truncSSat(بیعلامت بیت_عرض_2) ثابت {
  بیعلامت درون_داد_بیتها = get_بیت_عرض();
  APInt Min_درون_Range = APInt::get_Signed_Min_مقدار(بیت_عرض_2).sext(درون_داد_بیتها);
  APInt Max_درون_Range = APInt::get_Signed_Max_مقدار(بیت_عرض_2).sext(درون_داد_بیتها);
  APInt درون_داد_Min = get_Signed_Min_مقدار();
  APInt درون_داد_Max = get_Signed_Max_مقدار();
  معلوم_بیتها معلوم(بیت_عرض_2);

  // Case 1: All values fit - just truncate
  إذا (درون_داد_Min.sge(Min_درون_Range) && درون_داد_Max.sle(Max_درون_Range)) {
    معلوم = trunc(بیت_عرض_2);
  }
  // Case 2: All saturate to min
  وإلا إذا (درون_داد_Max.slt(Min_درون_Range)) {
    معلوم = معلوم_بیتها::makeConstant(APInt::get_Signed_Min_مقدار(بیت_عرض_2));
  }
  // Case 3: All saturate to max
  وإلا إذا (درون_داد_Min.sgt(Max_درون_Range)) {
    معلوم = معلوم_بیتها::makeConstant(APInt::get_Signed_Max_مقدار(بیت_عرض_2));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  وإلا إذا (درون_داد_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    معلوم.Zero = APInt(بیت_عرض_2, 0);
    معلوم.Zero.مجموعه_بیت(بیت_عرض_2 - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    معلوم.One = One.trunc(بیت_عرض_2);
    معلوم.One.clear_بیت(بیت_عرض_2 - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  وإلا إذا (درون_داد_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    معلوم.Zero = Zero.trunc(بیت_عرض_2);
    معلوم.Zero.clear_بیت(بیت_عرض_2 - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    معلوم.One = APInt(بیت_عرض_2, 0);
    معلوم.One.مجموعه_بیت(بیت_عرض_2 - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  وإلا {
    // Output: min saturation, truncated, or max saturation
    APInt درون_داد_Zero_پایینی = Zero.trunc(بیت_عرض_2);
    APInt درون_داد_One_پایینی = One.trunc(بیت_عرض_2);
    APInt MinSat = APInt::get_Signed_Min_مقدار(بیت_عرض_2);
    APInt MaxSat = APInt::get_Signed_Max_مقدار(بیت_عرض_2);
    معلوم_بیتها MinSatKB = معلوم_بیتها::makeConstant(MinSat);
    معلوم_بیتها MaxSatKB = معلوم_بیتها::makeConstant(MaxSat);
    إذا (درون_داد_Max.sle(Max_درون_Range)) {
      // Positive values fit, only negatives saturate to min
      معلوم.Zero = درون_داد_Zero_پایینی & MinSatKB.Zero;
      معلوم.One = درون_داد_One_پایینی & MinSatKB.One;
    } وإلا إذا (درون_داد_Min.sge(Min_درون_Range)) {
      // Negative values fit, only positives saturate to max
      معلوم.Zero = درون_داد_Zero_پایینی & MaxSatKB.Zero;
      معلوم.One = درون_داد_One_پایینی & MaxSatKB.One;
    } وإلا {
      // Both positive and negative values might saturate
      معلوم.Zero = درون_داد_Zero_پایینی & MinSatKB.Zero & MaxSatKB.Zero;
      معلوم.One = درون_داد_One_پایینی & MinSatKB.One & MaxSatKB.One;
    }
  }
  أرجع معلوم;
}

معلوم_بیتها معلوم_بیتها::truncSSatU(بیعلامت بیت_عرض_2) ثابت {
  بیعلامت درون_داد_بیتها = get_بیت_عرض();
  APInt Max_درون_Range = APInt::getAllOnes(بیت_عرض_2).zext(درون_داد_بیتها);
  APInt درون_داد_Min = get_Signed_Min_مقدار();
  APInt درون_داد_Max = get_Signed_Max_مقدار();
  معلوم_بیتها معلوم(بیت_عرض_2);

  إذا (isNegative()) {
    معلوم.مجموعه_All_Zero();
  } وإلا إذا (درون_داد_Min.isNonNegative() && درون_داد_Max.ule(Max_درون_Range)) {
    معلوم = trunc(بیت_عرض_2);
  } وإلا إذا (درون_داد_Min.isNonNegative() && درون_داد_Min.ugt(Max_درون_Range)) {
    معلوم.مجموعه_All_Ones();
  } وإلا إذا (درون_داد_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    معلوم.Zero = APInt(بیت_عرض_2, 0);
    // Common one bits: bits that are 1 in input stay 1
    معلوم.One = One.trunc(بیت_عرض_2);
  } وإلا {
    // Mixed: sign bit unknown
    إذا (درون_داد_Max.ule(Max_درون_Range)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      معلوم.Zero = Zero.trunc(بیت_عرض_2);
      معلوم.One = APInt(بیت_عرض_2, 0);
    } وإلا {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      معلوم.Zero = APInt(بیت_عرض_2, 0);
      معلوم.One = APInt(بیت_عرض_2, 0);
    }
  }
  أرجع معلوم;
}

معلوم_بیتها معلوم_بیتها::truncUSat(بیعلامت بیت_عرض_2) ثابت {
  بیعلامت درون_داد_بیتها = get_بیت_عرض();
  APInt Max_درون_Range = APInt::get_کم_بیتها_مجموعه(درون_داد_بیتها, بیت_عرض_2);
  APInt درون_داد_Max = get_Max_مقدار();
  APInt درون_داد_Min = get_Min_مقدار();
  معلوم_بیتها معلوم(بیت_عرض_2);

  إذا (درون_داد_Max.ule(Max_درون_Range)) {
    معلوم = trunc(بیت_عرض_2);
  } وإلا إذا (درون_داد_Min.ugt(Max_درون_Range)) {
    معلوم.مجموعه_All_Ones();
  } وإلا {
    معلوم.resetAll();
    معلوم.One = One.trunc(بیت_عرض_2);
  }
  أرجع معلوم;
}

معلوم_بیتها معلوم_بیتها::sext_درون_Reg(بیعلامت Src_بیت_عرض) ثابت {
  بیعلامت بیت_عرض_2 = get_بیت_عرض();
  أكد(0 < Src_بیت_عرض && Src_بیت_عرض <= بیت_عرض_2 &&
         "Illegal sext-in-register");

  إذا (Src_بیت_عرض == بیت_عرض_2)
    أرجع *هذا;

  بیعلامت Ext_بیتها = بیت_عرض_2 - Src_بیت_عرض;
  معلوم_بیتها نتیجه;
  نتیجه.One = One << Ext_بیتها;
  نتیجه.Zero = Zero << Ext_بیتها;
  نتیجه.One.ashr_درون_قراربده(Ext_بیتها);
  نتیجه.Zero.ashr_درون_قراربده(Ext_بیتها);
  أرجع نتیجه;
}

معلوم_بیتها معلوم_بیتها::makeGE(ثابت APInt &Val) ثابت {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  بیعلامت N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_کم_بیتها(get_بیت_عرض() - N);
  أرجع معلوم_بیتها(Zero, One | MaskedVal);
}

معلوم_بیتها معلوم_بیتها::umax(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  إذا (LHS.get_Min_مقدار().uge(RHS.get_Max_مقدار()))
    أرجع LHS;
  إذا (RHS.get_Min_مقدار().uge(LHS.get_Max_مقدار()))
    أرجع RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  معلوم_بیتها L = LHS.makeGE(RHS.get_Min_مقدار());
  معلوم_بیتها R = RHS.makeGE(LHS.get_Min_مقدار());
  أرجع L.intersectWith(R);
}

معلوم_بیتها معلوم_بیتها::umin(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  خودکار Flip = [](ثابت معلوم_بیتها &Val) { أرجع معلوم_بیتها(Val.One, Val.Zero); };
  أرجع Flip(umax(Flip(LHS), Flip(RHS)));
}

معلوم_بیتها معلوم_بیتها::smax(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  أرجع flip_Sign_بیت(umax(flip_Sign_بیت(LHS), flip_Sign_بیت(RHS)));
}

معلوم_بیتها معلوم_بیتها::smin(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  خودکار Flip = [](ثابت معلوم_بیتها &Val) {
    بیعلامت Sign_بیت_Position = Val.get_بیت_عرض() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.مجموعه_بیت_Val(Sign_بیت_Position, Val.Zero[Sign_بیت_Position]);
    One.مجموعه_بیت_Val(Sign_بیت_Position, Val.One[Sign_بیت_Position]);
    أرجع معلوم_بیتها(Zero, One);
  };
  أرجع Flip(umax(Flip(LHS), Flip(RHS)));
}

معلوم_بیتها معلوم_بیتها::abdu(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  إذا (LHS.get_Min_مقدار().uge(RHS.get_Max_مقدار()))
    أرجع compute_For_جمع_کن_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, LHS,
                            RHS);
  إذا (RHS.get_Min_مقدار().uge(LHS.get_Max_مقدار()))
    أرجع compute_For_جمع_کن_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  معلوم_بیتها Diff0 =
      compute_For_جمع_کن_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/درست, LHS, RHS);
  معلوم_بیتها Diff1 =
      compute_For_جمع_کن_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/درست, RHS, LHS);
  أرجع Diff0.intersectWith(Diff1);
}

معلوم_بیتها معلوم_بیتها::abds(معلوم_بیتها LHS, معلوم_بیتها RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  إذا (LHS.get_Signed_Min_مقدار().sge(RHS.get_Signed_Max_مقدار()))
    أرجع compute_For_جمع_کن_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, LHS,
                            RHS);
  إذا (RHS.get_Signed_Min_مقدار().sge(LHS.get_Signed_Max_مقدار()))
    أرجع compute_For_جمع_کن_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/خطأ, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  بیعلامت Sign_بیت_Position = LHS.get_بیت_عرض() - 1;
  لكل (خودکار Arg : {&LHS, &RHS}) {
    منطقي Tmp = Arg->Zero[Sign_بیت_Position];
    Arg->Zero.مجموعه_بیت_Val(Sign_بیت_Position, Arg->One[Sign_بیت_Position]);
    Arg->One.مجموعه_بیت_Val(Sign_بیت_Position, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  معلوم_بیتها Diff0 =
      compute_For_جمع_کن_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/درست, LHS, RHS);
  معلوم_بیتها Diff1 =
      compute_For_جمع_کن_Sub(/*Add=*/خطأ, /*NSW=*/خطأ, /*NUW=*/درست, RHS, LHS);
  أرجع Diff0.intersectWith(Diff1);
}

ساكن بیعلامت get_Max_لغزش_Amount(ثابت APInt &Max_مقدار, بیعلامت بیت_عرض_2) {
  إذا (isPowerOf2_32(بیت_عرض_2))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    أرجع Max_مقدار.استخراج_بیتها_As_ZExt_مقدار(
        std::min(Log2_32(بیت_عرض_2), Max_مقدار.get_بیت_عرض()), 0);
  // This is only an approximate upper bound.
  أرجع Max_مقدار.get_Limited_مقدار(بیت_عرض_2 - 1);
}

معلوم_بیتها معلوم_بیتها::shl(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS, منطقي NUW,
                         منطقي NSW, منطقي ShAmtNonZero) {
  بیعلامت بیت_عرض_2 = LHS.get_بیت_عرض();
  خودکار لغزش_By_Const = [&](ثابت معلوم_بیتها &LHS, بیعلامت لغزش_Amt_2) {
    معلوم_بیتها معلوم;
    منطقي Shifted_بیرون_Zero, Shifted_بیرون_One;
    معلوم.Zero = LHS.Zero.ushl_ov(لغزش_Amt_2, Shifted_بیرون_Zero);
    معلوم.Zero.مجموعه_کم_بیتها(لغزش_Amt_2);
    معلوم.One = LHS.One.ushl_ov(لغزش_Amt_2, Shifted_بیرون_One);

    // All cases returning poison have been handled by MaxShiftAmount already.
    إذا (NSW) {
      إذا (NUW && لغزش_Amt_2 != 0)
        // NUW means we can assume anything shifted out was a zero.
        Shifted_بیرون_Zero = درست;

      إذا (Shifted_بیرون_Zero)
        معلوم.makeNonNegative();
      وإلا إذا (Shifted_بیرون_One)
        معلوم.makeNegative();
    }
    أرجع معلوم;
  };

  // Fast path for a common case when LHS is completely unknown.
  معلوم_بیتها معلوم(بیت_عرض_2);
  بیعلامت Min_لغزش_Amount = RHS.get_Min_مقدار().get_Limited_مقدار(بیت_عرض_2);
  إذا (Min_لغزش_Amount == 0 && ShAmtNonZero)
    Min_لغزش_Amount = 1;
  إذا (LHS.isUnknown()) {
    معلوم.Zero.مجموعه_کم_بیتها(Min_لغزش_Amount);
    إذا (NUW && NSW && Min_لغزش_Amount != 0)
      معلوم.makeNonNegative();
    أرجع معلوم;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_مقدار = RHS.get_Max_مقدار();
  بیعلامت Max_لغزش_Amount = get_Max_لغزش_Amount(Max_مقدار, بیت_عرض_2);
  إذا (NUW && NSW)
    Max_لغزش_Amount = std::min(Max_لغزش_Amount, LHS.شمار_Max_Leading_Zeros() - 1);
  إذا (NUW)
    Max_لغزش_Amount = std::min(Max_لغزش_Amount, LHS.شمار_Max_Leading_Zeros());
  إذا (NSW)
    Max_لغزش_Amount = std::min(
        Max_لغزش_Amount,
        std::max(LHS.شمار_Max_Leading_Zeros(), LHS.شمار_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  إذا (Min_لغزش_Amount == 0 && Max_لغزش_Amount == بیت_عرض_2 - 1 &&
      isPowerOf2_32(بیت_عرض_2)) {
    معلوم.Zero.مجموعه_کم_بیتها(LHS.شمار_Min_Trailing_Zeros());
    إذا (LHS.isAllOnes())
      معلوم.One.مجموعه_Sign_بیت();
    إذا (NSW) {
      إذا (LHS.isNonNegative())
        معلوم.makeNonNegative();
      إذا (LHS.isNegative())
        معلوم.makeNegative();
    }
    أرجع معلوم;
  }

  // Find the common bits from all possible shifts.
  بیعلامت لغزش_Amt_Zero_ماسک = RHS.Zero.zextOrTrunc(32).get_ZExt_مقدار();
  بیعلامت لغزش_Amt_One_ماسک = RHS.One.zextOrTrunc(32).get_ZExt_مقدار();
  معلوم.مجموعه_All_Conflict();
  لكل (بیعلامت لغزش_Amt_2 = Min_لغزش_Amount; لغزش_Amt_2 <= Max_لغزش_Amount;
       ++لغزش_Amt_2) {
    // Skip if the shift amount is impossible.
    إذا ((لغزش_Amt_Zero_ماسک & لغزش_Amt_2) != 0 ||
        (لغزش_Amt_One_ماسک | لغزش_Amt_2) != لغزش_Amt_2)
      استمر;
    معلوم = معلوم.intersectWith(لغزش_By_Const(LHS, لغزش_Amt_2));
    إذا (معلوم.isUnknown())
      اكسر;
  }

  // All shift amounts may result in poison.
  إذا (معلوم.hasConflict())
    معلوم.مجموعه_All_Zero();
  أرجع معلوم;
}

معلوم_بیتها معلوم_بیتها::lshr(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS,
                          منطقي ShAmtNonZero, منطقي Exact) {
  بیعلامت بیت_عرض_2 = LHS.get_بیت_عرض();
  خودکار لغزش_By_Const = [&](ثابت معلوم_بیتها &LHS, بیعلامت لغزش_Amt_2) {
    معلوم_بیتها معلوم = LHS;
    معلوم >>= لغزش_Amt_2;
    // High bits are known zero.
    معلوم.Zero.مجموعه_بالا_بیتها(لغزش_Amt_2);
    أرجع معلوم;
  };

  // Fast path for a common case when LHS is completely unknown.
  معلوم_بیتها معلوم(بیت_عرض_2);
  بیعلامت Min_لغزش_Amount = RHS.get_Min_مقدار().get_Limited_مقدار(بیت_عرض_2);
  إذا (Min_لغزش_Amount == 0 && ShAmtNonZero)
    Min_لغزش_Amount = 1;
  إذا (LHS.isUnknown()) {
    معلوم.Zero.مجموعه_بالا_بیتها(Min_لغزش_Amount);
    أرجع معلوم;
  }

  // Find the common bits from all possible shifts.
  APInt Max_مقدار = RHS.get_Max_مقدار();
  بیعلامت Max_لغزش_Amount = get_Max_لغزش_Amount(Max_مقدار, بیت_عرض_2);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  إذا (Exact) {
    بیعلامت اولی_One = LHS.شمار_Max_Trailing_Zeros();
    إذا (اولی_One < Min_لغزش_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      معلوم.مجموعه_All_Zero();
      أرجع معلوم;
    }
    Max_لغزش_Amount = std::min(Max_لغزش_Amount, اولی_One);
  }

  بیعلامت لغزش_Amt_Zero_ماسک = RHS.Zero.zextOrTrunc(32).get_ZExt_مقدار();
  بیعلامت لغزش_Amt_One_ماسک = RHS.One.zextOrTrunc(32).get_ZExt_مقدار();
  معلوم.مجموعه_All_Conflict();
  لكل (بیعلامت لغزش_Amt_2 = Min_لغزش_Amount; لغزش_Amt_2 <= Max_لغزش_Amount;
       ++لغزش_Amt_2) {
    // Skip if the shift amount is impossible.
    إذا ((لغزش_Amt_Zero_ماسک & لغزش_Amt_2) != 0 ||
        (لغزش_Amt_One_ماسک | لغزش_Amt_2) != لغزش_Amt_2)
      استمر;
    معلوم = معلوم.intersectWith(لغزش_By_Const(LHS, لغزش_Amt_2));
    إذا (معلوم.isUnknown())
      اكسر;
  }

  // All shift amounts may result in poison.
  إذا (معلوم.hasConflict())
    معلوم.مجموعه_All_Zero();
  أرجع معلوم;
}

معلوم_بیتها معلوم_بیتها::ashr(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS,
                          منطقي ShAmtNonZero, منطقي Exact) {
  بیعلامت بیت_عرض_2 = LHS.get_بیت_عرض();
  خودکار لغزش_By_Const = [&](ثابت معلوم_بیتها &LHS, بیعلامت لغزش_Amt_2) {
    معلوم_بیتها معلوم = LHS;
    معلوم.Zero.ashr_درون_قراربده(لغزش_Amt_2);
    معلوم.One.ashr_درون_قراربده(لغزش_Amt_2);
    أرجع معلوم;
  };

  // Fast path for a common case when LHS is completely unknown.
  معلوم_بیتها معلوم(بیت_عرض_2);
  بیعلامت Min_لغزش_Amount = RHS.get_Min_مقدار().get_Limited_مقدار(بیت_عرض_2);
  إذا (Min_لغزش_Amount == 0 && ShAmtNonZero)
    Min_لغزش_Amount = 1;
  إذا (LHS.isUnknown()) {
    إذا (Min_لغزش_Amount == بیت_عرض_2) {
      // Always poison. Return zero because we don't like returning conflict.
      معلوم.مجموعه_All_Zero();
      أرجع معلوم;
    }
    أرجع معلوم;
  }

  // Find the common bits from all possible shifts.
  APInt Max_مقدار = RHS.get_Max_مقدار();
  بیعلامت Max_لغزش_Amount = get_Max_لغزش_Amount(Max_مقدار, بیت_عرض_2);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  إذا (Exact) {
    بیعلامت اولی_One = LHS.شمار_Max_Trailing_Zeros();
    إذا (اولی_One < Min_لغزش_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      معلوم.مجموعه_All_Zero();
      أرجع معلوم;
    }
    Max_لغزش_Amount = std::min(Max_لغزش_Amount, اولی_One);
  }

  بیعلامت لغزش_Amt_Zero_ماسک = RHS.Zero.zextOrTrunc(32).get_ZExt_مقدار();
  بیعلامت لغزش_Amt_One_ماسک = RHS.One.zextOrTrunc(32).get_ZExt_مقدار();
  معلوم.مجموعه_All_Conflict();
  لكل (بیعلامت لغزش_Amt_2 = Min_لغزش_Amount; لغزش_Amt_2 <= Max_لغزش_Amount;
      ++لغزش_Amt_2) {
    // Skip if the shift amount is impossible.
    إذا ((لغزش_Amt_Zero_ماسک & لغزش_Amt_2) != 0 ||
        (لغزش_Amt_One_ماسک | لغزش_Amt_2) != لغزش_Amt_2)
      استمر;
    معلوم = معلوم.intersectWith(لغزش_By_Const(LHS, لغزش_Amt_2));
    إذا (معلوم.isUnknown())
      اكسر;
  }

  // All shift amounts may result in poison.
  إذا (معلوم.hasConflict())
    معلوم.مجموعه_All_Zero();
  أرجع معلوم;
}

معلوم_بیتها معلوم_بیتها::fshl(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS,
                          ثابت APInt &Amt) {
  أرجع معلوم_بیتها(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

معلوم_بیتها معلوم_بیتها::fshr(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS,
                          ثابت APInt &Amt) {
  أرجع معلوم_بیتها(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

معلوم_بیتها معلوم_بیتها::clmul(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  معلوم_بیتها Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_مقدار(), RHS.get_Min_مقدار()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  خودکار ClMulOr = [](ثابت APInt &LHS, ثابت APInt &RHS) {
    بیعلامت BW = LHS.get_بیت_عرض();
    أكد(BW == RHS.get_بیت_عرض() && "Operand mismatch");
    APInt نتیجه(BW, 0);
    لكل (بیعلامت I :
         seq(std::min(RHS.get_فعال_بیتها(), BW - LHS.countr_zero())))
      إذا (RHS[I])
        نتیجه |= LHS << I;
    أرجع نتیجه;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt معلوم = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= معلوم;
  Res.One &= معلوم;

  أرجع Res;
}

معلوم_بیتها معلوم_بیتها::pext(ثابت معلوم_بیتها &Val, ثابت معلوم_بیتها &ماسک) {
  بیعلامت بیت_عرض_2 = Val.get_بیت_عرض();
  معلوم_بیتها Res(بیت_عرض_2);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.مجموعه_All_Conflict();

  // For each source position I where Mask[I] could be 1, the output position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible output positions widens.
  بیعلامت M0 = 0, M1 = 0;
  لكل (بیعلامت I = 0; I < بیت_عرض_2; ++I) {
    إذا (!ماسک.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      إذا (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clear_بیتها(M0, M1 + 1);
      إذا (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clear_بیتها(M0, M1 + 1);
    }
    إذا (ماسک.One[I])
      ++M0, ++M1;
    وإلا إذا (!ماسک.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clear_بیتها(M0, بیت_عرض_2);
  أرجع Res;
}

معلوم_بیتها معلوم_بیتها::pdep(ثابت معلوم_بیتها &Val, ثابت معلوم_بیتها &ماسک) {
  بیعلامت بیت_عرض_2 = Val.get_بیت_عرض();
  معلوم_بیتها Res(بیت_عرض_2);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.مجموعه_All_Conflict();

  // For each output position I where Mask[I] could be 1, the source position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible source positions widens.
  بیعلامت M0 = 0, M1 = 0;
  لكل (بیعلامت I = 0; I < بیت_عرض_2; ++I) {
    إذا (!ماسک.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clear_بیت(I);
    إذا (!ماسک.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::get_بیتها_مجموعه(بیت_عرض_2, M0, M1 + 1);
      إذا (!Range.is_Subset_از(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clear_بیت(I);
      إذا (!Range.is_Subset_از(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clear_بیت(I);
    }
    إذا (ماسک.One[I])
      ++M0, ++M1;
    وإلا إذا (!ماسک.Zero[I])
      ++M1;
  }
  أرجع Res;
}

std::optional<منطقي> معلوم_بیتها::eq(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  إذا (LHS.isConstant() && RHS.isConstant())
    أرجع LHS.getConstant() == RHS.getConstant();
  إذا (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    أرجع خطأ;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بیتها::ne(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  إذا (std::optional<منطقي> معلوم_EQ = eq(LHS, RHS))
    أرجع !*معلوم_EQ;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بیتها::ugt(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  إذا (LHS.get_Max_مقدار().ule(RHS.get_Min_مقدار()))
    أرجع خطأ;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  إذا (LHS.get_Min_مقدار().ugt(RHS.get_Max_مقدار()))
    أرجع درست;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بیتها::uge(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  إذا (std::optional<منطقي> IsUGT = ugt(RHS, LHS))
    أرجع !*IsUGT;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بیتها::ult(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  أرجع ugt(RHS, LHS);
}

std::optional<منطقي> معلوم_بیتها::ule(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  أرجع uge(RHS, LHS);
}

std::optional<منطقي> معلوم_بیتها::sgt(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  إذا (LHS.get_Signed_Max_مقدار().sle(RHS.get_Signed_Min_مقدار()))
    أرجع خطأ;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  إذا (LHS.get_Signed_Min_مقدار().sgt(RHS.get_Signed_Max_مقدار()))
    أرجع درست;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بیتها::sge(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  إذا (std::optional<منطقي> معلوم_SGT = sgt(RHS, LHS))
    أرجع !*معلوم_SGT;
  أرجع std::nullopt;
}

std::optional<منطقي> معلوم_بیتها::slt(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  أرجع sgt(RHS, LHS);
}

std::optional<منطقي> معلوم_بیتها::sle(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  أرجع sge(RHS, LHS);
}

معلوم_بیتها معلوم_بیتها::قدر_مطلق(منطقي IntMinIsPoison) ثابت {
  // If the source's MSB is zero then we know the rest of the bits already.
  إذا (isNonNegative())
    أرجع *هذا;

  // Absolute value preserves trailing zero count.
  معلوم_بیتها معلوم_Abs(get_بیت_عرض());

  // If the input is negative, then abs(x) == -x.
  إذا (isNegative()) {
    معلوم_بیتها Tmp = *هذا;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    إذا (IntMinIsPoison && (Zero.popcount() + 2) == get_بیت_عرض())
      Tmp.One.مجموعه_بیت(شمار_Min_Trailing_Zeros());

    معلوم_Abs = compute_For_جمع_کن_Sub(
        /*جمع_کن*/ خطأ, IntMinIsPoison, /*NUW=*/خطأ,
        معلوم_بیتها::makeConstant(APInt(get_بیت_عرض(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    إذا (IntMinIsPoison && Tmp.شمار_Min_Population() == 1 &&
        Tmp.شمار_Max_Population() != 1) {
      Tmp.One.clear_Sign_بیت();
      Tmp.Zero.مجموعه_Sign_بیت();
      معلوم_Abs.One.مجموعه_بیتها(get_بیت_عرض() - Tmp.شمار_Min_Leading_Zeros(),
                           get_بیت_عرض() - 1);
    }

  } وإلا {
    بیعلامت MaxTZ = شمار_Max_Trailing_Zeros();
    بیعلامت MinTZ = شمار_Min_Trailing_Zeros();

    معلوم_Abs.Zero.مجموعه_کم_بیتها(MinTZ);
    // If we know the lowest set 1, then preserve it.
    إذا (MaxTZ == MinTZ && MaxTZ < get_بیت_عرض())
      معلوم_Abs.One.مجموعه_بیت(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    إذا (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_مقدار())) {
      معلوم_Abs.One.clear_Sign_بیت();
      معلوم_Abs.Zero.مجموعه_Sign_بیت();
    }
  }

  أرجع معلوم_Abs;
}

معلوم_بیتها معلوم_بیتها::reduce_جمع_کن(بیعلامت رقم_Elts) ثابت {
  إذا (رقم_Elts == 0)
    أرجع معلوم_بیتها(get_بیت_عرض());

  بیعلامت بیت_عرض_2 = get_بیت_عرض();
  معلوم_بیتها نتیجه(بیت_عرض_2);

  إذا (isConstant())
    // If all elements are the same constant, we can simply compute it
    أرجع معلوم_بیتها::makeConstant(رقم_Elts * getConstant());

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
  بیعلامت گمشده_بیتها = Log2_32_Ceil(رقم_Elts);

  إذا (isNonNegative()) {
    بیعلامت LeadingZeros = شمار_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > گمشده_بیتها ? LeadingZeros - گمشده_بیتها : 0;
    نتیجه.Zero.مجموعه_بالا_بیتها(LeadingZeros);
  } وإلا إذا (isNegative()) {
    بیعلامت LeadingOnes = شمار_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > گمشده_بیتها ? LeadingOnes - گمشده_بیتها : 0;
    نتیجه.One.مجموعه_بالا_بیتها(LeadingOnes);
  }

  أرجع نتیجه;
}

ساكن معلوم_بیتها compute_For_Sat_جمع_کن_Sub(منطقي جمع_کن_2, منطقي Signed,
                                     ثابت معلوم_بیتها &LHS,
                                     ثابت معلوم_بیتها &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  بیعلامت بیت_عرض_2 = LHS.get_بیت_عرض();

  std::optional<منطقي> سرریز;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  منطقي MayNegClamp = درست;
  منطقي MayPosClamp = درست;
  إذا (Signed) {
    // Easy cases we can rule out any overflow.
    إذا (جمع_کن_2 && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      سرریز = خطأ;
    وإلا إذا (!جمع_کن_2 && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      سرریز = خطأ;
    وإلا {
      // Check if we may overflow. If we can't rule out overflow then check if
      // we can rule out a direction at least.
      معلوم_بیتها UnsignedLHS = LHS;
      معلوم_بیتها UnsignedRHS = RHS;
      // Get version of LHS/RHS with clearer signbit. This allows us to detect
      // how the addition/subtraction might overflow into the signbit. Then
      // using the actual known signbits of LHS/RHS, we can figure out which
      // overflows are/aren't possible.
      UnsignedLHS.One.clear_Sign_بیت();
      UnsignedLHS.Zero.مجموعه_Sign_بیت();
      UnsignedRHS.One.clear_Sign_بیت();
      UnsignedRHS.Zero.مجموعه_Sign_بیت();
      معلوم_بیتها Res =
          معلوم_بیتها::compute_For_جمع_کن_Sub(جمع_کن_2, /*NSW=*/خطأ,
                                      /*NUW=*/خطأ, UnsignedLHS, UnsignedRHS);
      إذا (جمع_کن_2) {
        إذا (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = خطأ;
          // Pos + Pos will overflow with extra signbit.
          إذا (LHS.isNonNegative() && RHS.isNonNegative())
            سرریز = درست;
        } وإلا إذا (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = خطأ;
          // Neg + Neg will overflow without extra signbit.
          إذا (LHS.isNegative() && RHS.isNegative())
            سرریز = درست;
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
            سرریز = درست;
        } وإلا إذا (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = خطأ;
          // Pos - Neg will overflow without extra signbit.
          إذا (LHS.isNonNegative() && RHS.isNegative())
            سرریز = درست;
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
      سرریز = خطأ;
  } وإلا إذا (جمع_کن_2) {
    // uadd.sat
    منطقي از;
    (فراغ)LHS.get_Max_مقدار().uadd_ov(RHS.get_Max_مقدار(), از);
    إذا (!از) {
      سرریز = خطأ;
    } وإلا {
      (فراغ)LHS.get_Min_مقدار().uadd_ov(RHS.get_Min_مقدار(), از);
      إذا (از)
        سرریز = درست;
    }
  } وإلا {
    // usub.sat
    منطقي از;
    (فراغ)LHS.get_Min_مقدار().usub_ov(RHS.get_Max_مقدار(), از);
    إذا (!از) {
      سرریز = خطأ;
    } وإلا {
      (فراغ)LHS.get_Max_مقدار().usub_ov(RHS.get_Min_مقدار(), از);
      إذا (از)
        سرریز = درست;
    }
  }

  معلوم_بیتها Res = معلوم_بیتها::compute_For_جمع_کن_Sub(جمع_کن_2, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  إذا (سرریز) {
    // We know whether or not we overflowed.
    إذا (!(*سرریز)) {
      // No overflow.
      أرجع Res;
    }

    // We overflowed
    APInt C;
    إذا (Signed) {
      // sadd.sat / ssub.sat
      أكد(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_مقدار(بیت_عرض_2)
                           : APInt::get_Signed_Max_مقدار(بیت_عرض_2);
    } وإلا إذا (جمع_کن_2) {
      // uadd.sat
      C = APInt::get_Max_مقدار(بیت_عرض_2);
    } وإلا {
      // uadd.sat
      C = APInt::get_Min_مقدار(بیت_عرض_2);
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
      Res.Zero.clear_کم_بیتها(بیت_عرض_2 - 1);
    إذا (MayNegClamp)
      Res.One.clear_کم_بیتها(بیت_عرض_2 - 1);
  } وإلا إذا (جمع_کن_2) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clear_All_بیتها();
  } وإلا {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clear_All_بیتها();
  }

  أرجع Res;
}

معلوم_بیتها معلوم_بیتها::sadd_sat(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  أرجع compute_For_Sat_جمع_کن_Sub(/*جمع_کن*/ درست, /*Signed*/ درست, LHS, RHS);
}
معلوم_بیتها معلوم_بیتها::ssub_sat(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  أرجع compute_For_Sat_جمع_کن_Sub(/*جمع_کن*/ خطأ, /*Signed*/ درست, LHS, RHS);
}
معلوم_بیتها معلوم_بیتها::uadd_sat(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  أرجع compute_For_Sat_جمع_کن_Sub(/*جمع_کن*/ درست, /*Signed*/ خطأ, LHS, RHS);
}
معلوم_بیتها معلوم_بیتها::usub_sat(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  أرجع compute_For_Sat_جمع_کن_Sub(/*جمع_کن*/ خطأ, /*Signed*/ خطأ, LHS, RHS);
}

ساكن معلوم_بیتها avgComputeU(معلوم_بیتها LHS, معلوم_بیتها RHS, منطقي IsCeil) {
  بیعلامت بیت_عرض_2 = LHS.get_بیت_عرض();
  LHS = LHS.zext(بیت_عرض_2 + 1);
  RHS = RHS.zext(بیت_عرض_2 + 1);
  LHS =
      compute_For_جمع_کن_انتقال(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.استخراج_بیتها(بیت_عرض_2, 1);
  أرجع LHS;
}

معلوم_بیتها معلوم_بیتها::avgFloorS(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  أرجع flip_Sign_بیت(avgFloorU(flip_Sign_بیت(LHS), flip_Sign_بیت(RHS)));
}

معلوم_بیتها معلوم_بیتها::avgFloorU(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  أرجع avgComputeU(LHS, RHS, /*IsCeil=*/خطأ);
}

معلوم_بیتها معلوم_بیتها::avgCeilS(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  أرجع flip_Sign_بیت(avgCeilU(flip_Sign_بیت(LHS), flip_Sign_بیت(RHS)));
}

معلوم_بیتها معلوم_بیتها::avgCeilU(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  أرجع avgComputeU(LHS, RHS, /*IsCeil=*/درست);
}

معلوم_بیتها معلوم_بیتها::mul(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS,
                         منطقي NoUndefSelfMultiply) {
  بیعلامت بیت_عرض_2 = LHS.get_بیت_عرض();
  أكد(بیت_عرض_2 == RHS.get_بیت_عرض() && "Operand mismatch");
  أكد((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.get_Max_مقدار();
  APInt UMaxRHS = RHS.get_Max_مقدار();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  منطقي Has_سرریز;
  APInt UMax_نتیجه = UMaxLHS.umul_ov(UMaxRHS, Has_سرریز);
  بیعلامت LeadZ = Has_سرریز ? 0 : UMax_نتیجه.countl_zero();

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
  بیعلامت Trail_بیتها_معلوم_LHS = (LHS.Zero | LHS.One).countr_one();
  بیعلامت Trail_بیتها_معلوم_RHS = (RHS.Zero | RHS.One).countr_one();
  بیعلامت TrailZeroLHS = LHS.شمار_Min_Trailing_Zeros();
  بیعلامت TrailZeroRHS = RHS.شمار_Min_Trailing_Zeros();
  بیعلامت TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  بیعلامت کوچکترین_عملوند = std::min(Trail_بیتها_معلوم_LHS - TrailZeroLHS,
                                      Trail_بیتها_معلوم_RHS - TrailZeroRHS);
  بیعلامت نتیجه_بیتها_معلوم = std::min(کوچکترین_عملوند + TrailZ, بیت_عرض_2);

  // The lower ResultBitsKnown bits of this are known.
  APInt Bottom_معلوم = LHS.One * RHS.One;

  معلوم_بیتها Res(بیت_عرض_2);
  Res.Zero.مجموعه_بالا_بیتها(LeadZ);
  Res.Zero |= (~Bottom_معلوم).get_Lo_بیتها(نتیجه_بیتها_معلوم);
  Res.One = Bottom_معلوم.get_Lo_بیتها(نتیجه_بیتها_معلوم);

  إذا (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    بیعلامت TwoTZP1 = 2 * TrailZeroLHS + 1;
    إذا (TwoTZP1 < بیت_عرض_2)
      Res.Zero.مجموعه_بیت(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    إذا (TrailZeroLHS < بیت_عرض_2 && LHS.One[TrailZeroLHS]) {
      بیعلامت TwoTZP2 = TwoTZP1 + 1;
      إذا (TwoTZP2 < بیت_عرض_2)
        Res.Zero.مجموعه_بیت(TwoTZP2);
    }
  }

  أرجع Res;
}

معلوم_بیتها معلوم_بیتها::mulhs(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  بیعلامت بیت_عرض_2 = LHS.get_بیت_عرض();
  أكد(بیت_عرض_2 == RHS.get_بیت_عرض() && "Operand mismatch");
  معلوم_بیتها WideLHS = LHS.sext(2 * بیت_عرض_2);
  معلوم_بیتها WideRHS = RHS.sext(2 * بیت_عرض_2);
  أرجع mul(WideLHS, WideRHS).استخراج_بیتها(بیت_عرض_2, بیت_عرض_2);
}

معلوم_بیتها معلوم_بیتها::mulhu(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  بیعلامت بیت_عرض_2 = LHS.get_بیت_عرض();
  أكد(بیت_عرض_2 == RHS.get_بیت_عرض() && "Operand mismatch");
  معلوم_بیتها WideLHS = LHS.zext(2 * بیت_عرض_2);
  معلوم_بیتها WideRHS = RHS.zext(2 * بیت_عرض_2);
  أرجع mul(WideLHS, WideRHS).استخراج_بیتها(بیت_عرض_2, بیت_عرض_2);
}

ساكن معلوم_بیتها div_Compute_کم_بیت(معلوم_بیتها معلوم, ثابت معلوم_بیتها &LHS,
                                  ثابت معلوم_بیتها &RHS, منطقي Exact) {

  إذا (!Exact)
    أرجع معلوم;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  إذا (LHS.One[0])
    معلوم.One.مجموعه_بیت(0);

  صحيح MinTZ =
      (صحيح)LHS.شمار_Min_Trailing_Zeros() - (صحيح)RHS.شمار_Max_Trailing_Zeros();
  صحيح MaxTZ =
      (صحيح)LHS.شمار_Max_Trailing_Zeros() - (صحيح)RHS.شمار_Min_Trailing_Zeros();
  إذا (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    معلوم.Zero.مجموعه_کم_بیتها(MinTZ);
    إذا (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      معلوم.One.مجموعه_بیت(MinTZ);
    }
  } وإلا إذا (MaxTZ < 0) {
    // Poison Result
    معلوم.مجموعه_All_Zero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  إذا (معلوم.hasConflict())
    معلوم.مجموعه_All_Zero();

  أرجع معلوم;
}

معلوم_بیتها معلوم_بیتها::sdiv(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS,
                          منطقي Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  إذا (LHS.isNonNegative() && RHS.isNonNegative())
    أرجع udiv(LHS, RHS, Exact);

  بیعلامت بیت_عرض_2 = LHS.get_بیت_عرض();
  معلوم_بیتها معلوم(بیت_عرض_2);

  إذا (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    معلوم.مجموعه_All_Zero();
    أرجع معلوم;
  }

  std::optional<APInt> Res;
  إذا (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_مقدار();
    APInt رقم = LHS.get_Signed_Min_مقدار();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (رقم.is_Min_Signed_مقدار() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_مقدار(بیت_عرض_2)
              : رقم.sdiv(Denom);
  } وإلا إذا (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    إذا (Exact || (-LHS.get_Signed_Max_مقدار()).uge(RHS.get_Signed_Max_مقدار())) {
      APInt Denom = RHS.get_Signed_Min_مقدار();
      APInt رقم = LHS.get_Signed_Min_مقدار();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      إذا (Denom.isZero())
        Denom.مجموعه_بیت(RHS.شمار_Min_Trailing_Zeros());
      Res = رقم.sdiv(Denom);
    }
  } وإلا إذا (LHS.is_Strictly_مثبت() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    إذا (Exact || LHS.get_Signed_Min_مقدار().uge(-RHS.get_Signed_Min_مقدار())) {
      APInt Denom = RHS.get_Signed_Max_مقدار();
      APInt رقم = LHS.get_Signed_Max_مقدار();
      Res = رقم.sdiv(Denom);
    }
  }

  إذا (Res) {
    إذا (Res->isNonNegative()) {
      بیعلامت LeadZ = Res->شمار_Leading_Zeros();
      معلوم.Zero.مجموعه_بالا_بیتها(LeadZ);
    } وإلا {
      بیعلامت LeadO = Res->شمار_Leading_Ones();
      معلوم.One.مجموعه_بالا_بیتها(LeadO);
    }
  }

  معلوم = div_Compute_کم_بیت(معلوم, LHS, RHS, Exact);
  أرجع معلوم;
}

معلوم_بیتها معلوم_بیتها::udiv(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS,
                          منطقي Exact) {
  بیعلامت بیت_عرض_2 = LHS.get_بیت_عرض();
  معلوم_بیتها معلوم(بیت_عرض_2);

  إذا (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    معلوم.مجموعه_All_Zero();
    أرجع معلوم;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_مقدار();
  إذا (MinDenom.isZero())
    MinDenom.مجموعه_بیت(RHS.شمار_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_مقدار().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_مقدار().udiv(RHS.get_Max_مقدار());

  معلوم = معلوم_بیتها::makeConstant(MinRes);
  إذا (std::optional<بیعلامت> Different_بیت =
          APIntOps::Get_Most_Significant_Different_بیت(MinRes, MaxRes)) {
    معلوم.Zero.clear_کم_بیتها(*Different_بیت + 1);
    معلوم.One.clear_کم_بیتها(*Different_بیت + 1);
  }

  معلوم = div_Compute_کم_بیت(معلوم, LHS, RHS, Exact);

  أرجع معلوم;
}

معلوم_بیتها معلوم_بیتها::rem_Get_کم_بیتها(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  بیعلامت بیت_عرض_2 = LHS.get_بیت_عرض();
  إذا (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    بیعلامت RHSZeros = RHS.شمار_Min_Trailing_Zeros();
    APInt ماسک = APInt::get_کم_بیتها_مجموعه(بیت_عرض_2, RHSZeros);
    APInt Ones_ماسک = LHS.One & ماسک;
    APInt Zeros_ماسک = LHS.Zero & ماسک;
    أرجع معلوم_بیتها(Zeros_ماسک, Ones_ماسک);
  }
  أرجع معلوم_بیتها(بیت_عرض_2);
}

معلوم_بیتها معلوم_بیتها::urem(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  إذا (LHS.get_Max_مقدار().ult(RHS.get_Min_مقدار()))
    أرجع LHS;

  معلوم_بیتها معلوم = rem_Get_کم_بیتها(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_مقدار(), RHS.get_Max_مقدار() - 1);
  معلوم.Zero.مجموعه_بالا_بیتها(MaxRes.شمار_Leading_Zeros());
  أرجع معلوم;
}

معلوم_بیتها معلوم_بیتها::srem(ثابت معلوم_بیتها &LHS, ثابت معلوم_بیتها &RHS) {
  معلوم_بیتها معلوم = rem_Get_کم_بیتها(LHS, RHS);
  إذا (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt کم_بیتها = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    إذا (LHS.isNonNegative() || کم_بیتها.is_Subset_از(LHS.Zero))
      معلوم.Zero |= ~کم_بیتها;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    إذا (LHS.isNegative() && کم_بیتها.intersects(LHS.One))
      معلوم.One |= ~کم_بیتها;
    أرجع معلوم;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  إذا (LHS.isNegative() && معلوم.isNonZero())
    معلوم.One.مجموعه_بالا_بیتها(
        std::max(LHS.شمار_Min_Leading_Ones(), RHS.شمار_Min_Sign_بیتها()));
  وإلا إذا (LHS.isNonNegative())
    معلوم.Zero.مجموعه_بالا_بیتها(
        std::max(LHS.شمار_Min_Leading_Zeros(), RHS.شمار_Min_Sign_بیتها()));
  أرجع معلوم;
}

معلوم_بیتها &معلوم_بیتها::عملگر&=(ثابت معلوم_بیتها &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  أرجع *هذا;
}

معلوم_بیتها &معلوم_بیتها::عملگر|=(ثابت معلوم_بیتها &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  أرجع *هذا;
}

معلوم_بیتها &معلوم_بیتها::عملگر^=(ثابت معلوم_بیتها &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  أرجع *هذا;
}

معلوم_بیتها معلوم_بیتها::blsi() ثابت {
  بیعلامت بیت_عرض_2 = get_بیت_عرض();
  معلوم_بیتها معلوم(Zero, APInt(بیت_عرض_2, 0));
  بیعلامت Max = شمار_Max_Trailing_Zeros();
  معلوم.Zero.مجموعه_بیتها_From(std::min(Max + 1, بیت_عرض_2));
  بیعلامت Min = شمار_Min_Trailing_Zeros();
  إذا (Max == Min && Max < بیت_عرض_2)
    معلوم.One.مجموعه_بیت(Max);
  أرجع معلوم;
}

معلوم_بیتها معلوم_بیتها::blsmsk() ثابت {
  بیعلامت بیت_عرض_2 = get_بیت_عرض();
  معلوم_بیتها معلوم(بیت_عرض_2);
  بیعلامت Max = شمار_Max_Trailing_Zeros();
  معلوم.Zero.مجموعه_بیتها_From(std::min(Max + 1, بیت_عرض_2));
  بیعلامت Min = شمار_Min_Trailing_Zeros();
  معلوم.One.مجموعه_کم_بیتها(std::min(Min + 1, بیت_عرض_2));
  أرجع معلوم;
}

فراغ معلوم_بیتها::print(raw_ostream &OS) ثابت {
  بیعلامت بیت_عرض_2 = get_بیت_عرض();
  لكل (بیعلامت I = 0; I < بیت_عرض_2; ++I) {
    بیعلامت N = بیت_عرض_2 - I - 1;
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
LLVM_DUMP_روش فراغ معلوم_بیتها::dump() ثابت {
  print(dbgs());
  dbgs() << "\n";
}
#پایان_اگر

بیعلامت llvm::Sign_بیتها_Ops::rot(بیعلامت Src_Sign_بیتها, بیعلامت بیت_عرض_2,
                                std::optional<APInt> RotAmt,
                                منطقي Is_Rotate_راست) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  إذا (Src_Sign_بیتها == بیت_عرض_2)
    أرجع بیت_عرض_2;

  إذا (!RotAmt)
    أرجع 1;

  بیعلامت Amt = RotAmt->urem(بیت_عرض_2);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  إذا (Is_Rotate_راست)
    Amt = (بیت_عرض_2 - Amt) % بیت_عرض_2;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  إذا (Src_Sign_بیتها > Amt + 1)
    أرجع Src_Sign_بیتها - Amt;
  أرجع 1;
}
