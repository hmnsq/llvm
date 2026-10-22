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

#అన్తర్భావ "llvm/Support/KnownBits.h"
#అన్తర్భావ "llvm/ADT/Sequence.h"
#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <cassert>

ఉపయోగించి నామాకాశ llvm;

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::flip_Sign_బిట్టు(స్థిరరాశి తెలిసిన_బిట్లు &Val) {
  గుర్తులేని Sign_బిట్టు_Position = Val.get_బిట్టు_వెడల్పు() - 1;
  APInt Zero = Val.Zero;
  APInt One = Val.One;
  Zero.సమితి_బిట్టు_Val(Sign_బిట్టు_Position, Val.One[Sign_బిట్టు_Position]);
  One.సమితి_బిట్టు_Val(Sign_బిట్టు_Position, Val.Zero[Sign_బిట్టు_Position]);
  తిరిగి తెలిసిన_బిట్లు(Zero, One);
}

స్థిర తెలిసిన_బిట్లు compute_For_కూడు_వాహకం(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS,
                                    తార్కిక వాహకం_Zero, తార్కిక వాహకం_One) {

  APInt Possible_కూడిక_Zero = LHS.get_Max_విలువ() + RHS.get_Max_విలువ() + !వాహకం_Zero;
  APInt Possible_కూడిక_One = LHS.get_Min_విలువ() + RHS.get_Min_విలువ() + వాహకం_One;

  // Compute known bits of the carry.
  APInt వాహకం_తెలిసిన_Zero = ~(Possible_కూడిక_Zero ^ LHS.Zero ^ RHS.Zero);
  APInt వాహకం_తెలిసిన_One = Possible_కూడిక_One ^ LHS.One ^ RHS.One;

  // Compute set of known bits (where all three relevant bits are known).
  APInt LHSKnownUnion = LHS.Zero | LHS.One;
  APInt RHSKnownUnion = RHS.Zero | RHS.One;
  APInt వాహకం_తెలిసిన_Union = std::move(వాహకం_తెలిసిన_Zero) | వాహకం_తెలిసిన_One;
  APInt తెలిసిన = std::move(LHSKnownUnion) & RHSKnownUnion & వాహకం_తెలిసిన_Union;

  // Compute known bits of the result.
  తెలిసిన_బిట్లు తెలిసిన_బహిర్గతం;
  తెలిసిన_బహిర్గతం.Zero = ~std::move(Possible_కూడిక_Zero) & తెలిసిన;
  తెలిసిన_బహిర్గతం.One = std::move(Possible_కూడిక_One) & తెలిసిన;
  తిరిగి తెలిసిన_బహిర్గతం;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::compute_For_కూడు_వాహకం(
    స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS, స్థిరరాశి తెలిసిన_బిట్లు &వాహకం_2) {
  ప్రతిజ్ఞా(వాహకం_2.get_బిట్టు_వెడల్పు() == 1 && "Carry must be 1-bit");
  తిరిగి ::compute_For_కూడు_వాహకం(
      LHS, RHS, వాహకం_2.Zero.get_Bool_విలువ(), వాహకం_2.One.get_Bool_విలువ());
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::compute_For_కూడు_Sub(తార్కిక కూడు_2, తార్కిక NSW, తార్కిక NUW,
                                      స్థిరరాశి తెలిసిన_బిట్లు &LHS,
                                      స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  గుర్తులేని బిట్టు_వెడల్పు_2 = LHS.get_బిట్టు_వెడల్పు();
  తెలిసిన_బిట్లు తెలిసిన_బహిర్గతం(బిట్టు_వెడల్పు_2);
  // This can be a relatively expensive helper, so optimistically save some
  // work.
  ఒకవేళ (LHS.isUnknown() && RHS.isUnknown())
    తిరిగి తెలిసిన_బహిర్గతం;

  ఒకవేళ (!LHS.isUnknown() && !RHS.isUnknown()) {
    ఒకవేళ (కూడు_2) {
      // Sum = LHS + RHS + 0
      తెలిసిన_బహిర్గతం = ::compute_For_కూడు_వాహకం(LHS, RHS, /*CarryZero=*/నిజం,
                                      /*CarryOne=*/అబద్ధం);
    } లేకపోతే {
      // Sum = LHS + ~RHS + 1
      తెలిసిన_బిట్లు NotRHS = RHS;
      std::మార్పిడి(NotRHS.Zero, NotRHS.One);
      తెలిసిన_బహిర్గతం = ::compute_For_కూడు_వాహకం(LHS, NotRHS, /*CarryZero=*/అబద్ధం,
                                      /*CarryOne=*/నిజం);
    }
  }

  // Handle add/sub given nsw and/or nuw.
  ఒకవేళ (NUW) {
    ఒకవేళ (కూడు_2) {
      // (add nuw X, Y)
      APInt MinVal = LHS.get_Min_విలువ().uadd_sat(RHS.get_Min_విలువ());
      // None of the adds can end up overflowing, so min consecutive highbits
      // in minimum possible of X + Y must all remain set.
      ఒకవేళ (NSW) {
        గుర్తులేని అంకె_బిట్లు_2 = MinVal.trunc(బిట్టు_వెడల్పు_2 - 1).countl_one();
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        తెలిసిన_బహిర్గతం.One.సమితి_బిట్లు(బిట్టు_వెడల్పు_2 - 1 - అంకె_బిట్లు_2, బిట్టు_వెడల్పు_2 - 1);
      }
      తెలిసిన_బహిర్గతం.One.సమితి_ఎగువ_బిట్లు(MinVal.countl_one());
    } లేకపోతే {
      // (sub nuw X, Y)
      APInt MaxVal = LHS.get_Max_విలువ().usub_sat(RHS.get_Min_విలువ());
      // None of the subs can overflow at any point, so any common high bits
      // will subtract away and result in zeros.
      ఒకవేళ (NSW) {
        // If we have NSW as well, we also know we can't overflow the signbit so
        // can start counting from 1 bit back.
        గుర్తులేని అంకె_బిట్లు_2 = MaxVal.trunc(బిట్టు_వెడల్పు_2 - 1).countl_zero();
        తెలిసిన_బహిర్గతం.Zero.సమితి_బిట్లు(బిట్టు_వెడల్పు_2 - 1 - అంకె_బిట్లు_2, బిట్టు_వెడల్పు_2 - 1);
      }
      తెలిసిన_బహిర్గతం.Zero.సమితి_ఎగువ_బిట్లు(MaxVal.countl_zero());
    }
  }

  ఒకవేళ (NSW) {
    APInt MinVal;
    APInt MaxVal;
    ఒకవేళ (కూడు_2) {
      // (add nsw X, Y)
      MinVal = LHS.get_Signed_Min_విలువ().sadd_sat(RHS.get_Signed_Min_విలువ());
      MaxVal = LHS.get_Signed_Max_విలువ().sadd_sat(RHS.get_Signed_Max_విలువ());
    } లేకపోతే {
      // (sub nsw X, Y)
      MinVal = LHS.get_Signed_Min_విలువ().ssub_sat(RHS.get_Signed_Max_విలువ());
      MaxVal = LHS.get_Signed_Max_విలువ().ssub_sat(RHS.get_Signed_Min_విలువ());
    }
    ఒకవేళ (MinVal.isNonNegative()) {
      // If min is non-negative, result will always be non-neg (can't overflow
      // around).
      గుర్తులేని అంకె_బిట్లు_2 = MinVal.trunc(బిట్టు_వెడల్పు_2 - 1).countl_one();
      తెలిసిన_బహిర్గతం.One.సమితి_బిట్లు(బిట్టు_వెడల్పు_2 - 1 - అంకె_బిట్లు_2, బిట్టు_వెడల్పు_2 - 1);
      తెలిసిన_బహిర్గతం.Zero.సమితి_Sign_బిట్టు();
    }
    ఒకవేళ (MaxVal.isNegative()) {
      // If max is negative, result will always be neg (can't overflow around).
      గుర్తులేని అంకె_బిట్లు_2 = MaxVal.trunc(బిట్టు_వెడల్పు_2 - 1).countl_zero();
      తెలిసిన_బహిర్గతం.Zero.సమితి_బిట్లు(బిట్టు_వెడల్పు_2 - 1 - అంకె_బిట్లు_2, బిట్టు_వెడల్పు_2 - 1);
      తెలిసిన_బహిర్గతం.One.సమితి_Sign_బిట్టు();
    }
  }

  // Just return 0 if the nsw/nuw is violated and we have poison.
  ఒకవేళ (తెలిసిన_బహిర్గతం.hasConflict())
    తెలిసిన_బహిర్గతం.సమితి_All_Zero();
  తిరిగి తెలిసిన_బహిర్గతం;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::computeForSubBorrow(స్థిరరాశి తెలిసిన_బిట్లు &LHS, తెలిసిన_బిట్లు RHS,
                                         స్థిరరాశి తెలిసిన_బిట్లు &Borrow) {
  ప్రతిజ్ఞా(Borrow.get_బిట్టు_వెడల్పు() == 1 && "Borrow must be 1-bit");

  // LHS - RHS = LHS + ~RHS + 1
  // Carry 1 - Borrow in ::computeForAddCarry
  std::మార్పిడి(RHS.Zero, RHS.One);
  తిరిగి ::compute_For_కూడు_వాహకం(LHS, RHS,
                              /*CarryZero=*/Borrow.One.get_Bool_విలువ(),
                              /*CarryOne=*/Borrow.Zero.get_Bool_విలువ());
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::truncSSat(గుర్తులేని బిట్టు_వెడల్పు_2) స్థిరరాశి {
  గుర్తులేని ఆదానం_బిట్లు = get_బిట్టు_వెడల్పు();
  APInt Min_లోపల_Range = APInt::get_Signed_Min_విలువ(బిట్టు_వెడల్పు_2).sext(ఆదానం_బిట్లు);
  APInt Max_లోపల_Range = APInt::get_Signed_Max_విలువ(బిట్టు_వెడల్పు_2).sext(ఆదానం_బిట్లు);
  APInt ఆదానం_Min = get_Signed_Min_విలువ();
  APInt ఆదానం_Max = get_Signed_Max_విలువ();
  తెలిసిన_బిట్లు తెలిసిన(బిట్టు_వెడల్పు_2);

  // Case 1: All values fit - just truncate
  ఒకవేళ (ఆదానం_Min.sge(Min_లోపల_Range) && ఆదానం_Max.sle(Max_లోపల_Range)) {
    తెలిసిన = trunc(బిట్టు_వెడల్పు_2);
  }
  // Case 2: All saturate to min
  లేకపోతే ఒకవేళ (ఆదానం_Max.slt(Min_లోపల_Range)) {
    తెలిసిన = తెలిసిన_బిట్లు::makeConstant(APInt::get_Signed_Min_విలువ(బిట్టు_వెడల్పు_2));
  }
  // Case 3: All saturate to max
  లేకపోతే ఒకవేళ (ఆదానం_Min.sgt(Max_లోపల_Range)) {
    తెలిసిన = తెలిసిన_బిట్లు::makeConstant(APInt::get_Signed_Max_విలువ(బిట్టు_వెడల్పు_2));
  }
  // Case 4: All non-negative, some fit, some saturate to max
  లేకపోతే ఒకవేళ (ఆదానం_Min.isNonNegative()) {
    // Output: truncated OR max saturation
    // Max saturation has only sign bit as 0
    తెలిసిన.Zero = APInt(బిట్టు_వెడల్పు_2, 0);
    తెలిసిన.Zero.సమితి_బిట్టు(బిట్టు_వెడల్పు_2 - 1); // Sign bit always 0
    // Max saturation has all lower bits as 1, so preserve InputOneLower
    తెలిసిన.One = One.trunc(బిట్టు_వెడల్పు_2);
    తెలిసిన.One.clear_బిట్టు(బిట్టు_వెడల్పు_2 - 1); // Sign bit is 0, not 1
  }
  // Case 5: All negative, some fit, some saturate to min
  లేకపోతే ఒకవేళ (ఆదానం_Max.isNegative()) {
    // Output: truncated OR min saturation
    // Min saturation has all lower bits as 0, so preserve InputZeroLower
    తెలిసిన.Zero = Zero.trunc(బిట్టు_వెడల్పు_2);
    తెలిసిన.Zero.clear_బిట్టు(బిట్టు_వెడల్పు_2 - 1); // Sign bit is 1, not 0
    // Min saturation has only sign bit as 1
    తెలిసిన.One = APInt(బిట్టు_వెడల్పు_2, 0);
    తెలిసిన.One.సమితి_బిట్టు(బిట్టు_వెడల్పు_2 - 1); // Sign bit always 1
  }
  // Case 6: Mixed positive and negative
  లేకపోతే {
    // Output: min saturation, truncated, or max saturation
    APInt ఆదానం_Zero_క్రింది = Zero.trunc(బిట్టు_వెడల్పు_2);
    APInt ఆదానం_One_క్రింది = One.trunc(బిట్టు_వెడల్పు_2);
    APInt MinSat = APInt::get_Signed_Min_విలువ(బిట్టు_వెడల్పు_2);
    APInt MaxSat = APInt::get_Signed_Max_విలువ(బిట్టు_వెడల్పు_2);
    తెలిసిన_బిట్లు MinSatKB = తెలిసిన_బిట్లు::makeConstant(MinSat);
    తెలిసిన_బిట్లు MaxSatKB = తెలిసిన_బిట్లు::makeConstant(MaxSat);
    ఒకవేళ (ఆదానం_Max.sle(Max_లోపల_Range)) {
      // Positive values fit, only negatives saturate to min
      తెలిసిన.Zero = ఆదానం_Zero_క్రింది & MinSatKB.Zero;
      తెలిసిన.One = ఆదానం_One_క్రింది & MinSatKB.One;
    } లేకపోతే ఒకవేళ (ఆదానం_Min.sge(Min_లోపల_Range)) {
      // Negative values fit, only positives saturate to max
      తెలిసిన.Zero = ఆదానం_Zero_క్రింది & MaxSatKB.Zero;
      తెలిసిన.One = ఆదానం_One_క్రింది & MaxSatKB.One;
    } లేకపోతే {
      // Both positive and negative values might saturate
      తెలిసిన.Zero = ఆదానం_Zero_క్రింది & MinSatKB.Zero & MaxSatKB.Zero;
      తెలిసిన.One = ఆదానం_One_క్రింది & MinSatKB.One & MaxSatKB.One;
    }
  }
  తిరిగి తెలిసిన;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::truncSSatU(గుర్తులేని బిట్టు_వెడల్పు_2) స్థిరరాశి {
  గుర్తులేని ఆదానం_బిట్లు = get_బిట్టు_వెడల్పు();
  APInt Max_లోపల_Range = APInt::getAllOnes(బిట్టు_వెడల్పు_2).zext(ఆదానం_బిట్లు);
  APInt ఆదానం_Min = get_Signed_Min_విలువ();
  APInt ఆదానం_Max = get_Signed_Max_విలువ();
  తెలిసిన_బిట్లు తెలిసిన(బిట్టు_వెడల్పు_2);

  ఒకవేళ (isNegative()) {
    తెలిసిన.సమితి_All_Zero();
  } లేకపోతే ఒకవేళ (ఆదానం_Min.isNonNegative() && ఆదానం_Max.ule(Max_లోపల_Range)) {
    తెలిసిన = trunc(బిట్టు_వెడల్పు_2);
  } లేకపోతే ఒకవేళ (ఆదానం_Min.isNonNegative() && ఆదానం_Min.ugt(Max_లోపల_Range)) {
    తెలిసిన.సమితి_All_Ones();
  } లేకపోతే ఒకవేళ (ఆదానం_Min.isNonNegative()) {
    // All non-negative but mixed: some fit, some saturate to all-ones
    // No common zero bits (saturation is all-ones)
    తెలిసిన.Zero = APInt(బిట్టు_వెడల్పు_2, 0);
    // Common one bits: bits that are 1 in input stay 1
    తెలిసిన.One = One.trunc(బిట్టు_వెడల్పు_2);
  } లేకపోతే {
    // Mixed: sign bit unknown
    ఒకవేళ (ఆదానం_Max.ule(Max_లోపల_Range)) {
      // Positive values all fit (no saturation to all-ones)
      // Output: all-zeros (negatives) OR truncated (positives)
      తెలిసిన.Zero = Zero.trunc(బిట్టు_వెడల్పు_2);
      తెలిసిన.One = APInt(బిట్టు_వెడల్పు_2, 0);
    } లేకపోతే {
      // Positive values might saturate to all-ones
      // Output: all-zeros OR truncated OR all-ones
      // No common bits
      తెలిసిన.Zero = APInt(బిట్టు_వెడల్పు_2, 0);
      తెలిసిన.One = APInt(బిట్టు_వెడల్పు_2, 0);
    }
  }
  తిరిగి తెలిసిన;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::truncUSat(గుర్తులేని బిట్టు_వెడల్పు_2) స్థిరరాశి {
  గుర్తులేని ఆదానం_బిట్లు = get_బిట్టు_వెడల్పు();
  APInt Max_లోపల_Range = APInt::get_దిగువ_బిట్లు_సమితి(ఆదానం_బిట్లు, బిట్టు_వెడల్పు_2);
  APInt ఆదానం_Max = get_Max_విలువ();
  APInt ఆదానం_Min = get_Min_విలువ();
  తెలిసిన_బిట్లు తెలిసిన(బిట్టు_వెడల్పు_2);

  ఒకవేళ (ఆదానం_Max.ule(Max_లోపల_Range)) {
    తెలిసిన = trunc(బిట్టు_వెడల్పు_2);
  } లేకపోతే ఒకవేళ (ఆదానం_Min.ugt(Max_లోపల_Range)) {
    తెలిసిన.సమితి_All_Ones();
  } లేకపోతే {
    తెలిసిన.resetAll();
    తెలిసిన.One = One.trunc(బిట్టు_వెడల్పు_2);
  }
  తిరిగి తెలిసిన;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::sext_లోపల_Reg(గుర్తులేని Src_బిట్టు_వెడల్పు) స్థిరరాశి {
  గుర్తులేని బిట్టు_వెడల్పు_2 = get_బిట్టు_వెడల్పు();
  ప్రతిజ్ఞా(0 < Src_బిట్టు_వెడల్పు && Src_బిట్టు_వెడల్పు <= బిట్టు_వెడల్పు_2 &&
         "Illegal sext-in-register");

  ఒకవేళ (Src_బిట్టు_వెడల్పు == బిట్టు_వెడల్పు_2)
    తిరిగి *ఇది;

  గుర్తులేని Ext_బిట్లు = బిట్టు_వెడల్పు_2 - Src_బిట్టు_వెడల్పు;
  తెలిసిన_బిట్లు ఫలితం;
  ఫలితం.One = One << Ext_బిట్లు;
  ఫలితం.Zero = Zero << Ext_బిట్లు;
  ఫలితం.One.ashr_లోపల_ఉంచు(Ext_బిట్లు);
  ఫలితం.Zero.ashr_లోపల_ఉంచు(Ext_బిట్లు);
  తిరిగి ఫలితం;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::makeGE(స్థిరరాశి APInt &Val) స్థిరరాశి {
  // Count the number of leading bit positions where our underlying value is
  // known to be less than or equal to Val.
  గుర్తులేని N = (Zero | Val).countl_one();

  // For each of those bit positions, if Val has a 1 in that bit then our
  // underlying value must also have a 1.
  APInt MaskedVal(Val);
  MaskedVal.clear_దిగువ_బిట్లు(get_బిట్టు_వెడల్పు() - N);
  తిరిగి తెలిసిన_బిట్లు(Zero, One | MaskedVal);
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::umax(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  // If we can prove that LHS >= RHS then use LHS as the result. Likewise for
  // RHS. Ideally our caller would already have spotted these cases and
  // optimized away the umax operation, but we handle them here for
  // completeness.
  ఒకవేళ (LHS.get_Min_విలువ().uge(RHS.get_Max_విలువ()))
    తిరిగి LHS;
  ఒకవేళ (RHS.get_Min_విలువ().uge(LHS.get_Max_విలువ()))
    తిరిగి RHS;

  // If the result of the umax is LHS then it must be greater than or equal to
  // the minimum possible value of RHS. Likewise for RHS. Any known bits that
  // are common to these two values are also known in the result.
  తెలిసిన_బిట్లు L = LHS.makeGE(RHS.get_Min_విలువ());
  తెలిసిన_బిట్లు R = RHS.makeGE(LHS.get_Min_విలువ());
  తిరిగి L.intersectWith(R);
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::umin(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  // Flip the range of values: [0, 0xFFFFFFFF] <-> [0xFFFFFFFF, 0]
  స్వయంచాలక Flip = [](స్థిరరాశి తెలిసిన_బిట్లు &Val) { తిరిగి తెలిసిన_బిట్లు(Val.One, Val.Zero); };
  తిరిగి Flip(umax(Flip(LHS), Flip(RHS)));
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::smax(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  తిరిగి flip_Sign_బిట్టు(umax(flip_Sign_బిట్టు(LHS), flip_Sign_బిట్టు(RHS)));
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::smin(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  // Flip the range of values: [-0x80000000, 0x7FFFFFFF] <-> [0xFFFFFFFF, 0]
  స్వయంచాలక Flip = [](స్థిరరాశి తెలిసిన_బిట్లు &Val) {
    గుర్తులేని Sign_బిట్టు_Position = Val.get_బిట్టు_వెడల్పు() - 1;
    APInt Zero = Val.One;
    APInt One = Val.Zero;
    Zero.సమితి_బిట్టు_Val(Sign_బిట్టు_Position, Val.Zero[Sign_బిట్టు_Position]);
    One.సమితి_బిట్టు_Val(Sign_బిట్టు_Position, Val.One[Sign_బిట్టు_Position]);
    తిరిగి తెలిసిన_బిట్లు(Zero, One);
  };
  తిరిగి Flip(umax(Flip(LHS), Flip(RHS)));
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::abdu(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  ఒకవేళ (LHS.get_Min_విలువ().uge(RHS.get_Max_విలువ()))
    తిరిగి compute_For_కూడు_Sub(/*Add=*/అబద్ధం, /*NSW=*/అబద్ధం, /*NUW=*/అబద్ధం, LHS,
                            RHS);
  ఒకవేళ (RHS.get_Min_విలువ().uge(LHS.get_Max_విలువ()))
    తిరిగి compute_For_కూడు_Sub(/*Add=*/అబద్ధం, /*NSW=*/అబద్ధం, /*NUW=*/అబద్ధం, RHS,
                            LHS);

  // By construction, the subtraction in abdu never has unsigned overflow.
  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  తెలిసిన_బిట్లు Diff0 =
      compute_For_కూడు_Sub(/*Add=*/అబద్ధం, /*NSW=*/అబద్ధం, /*NUW=*/నిజం, LHS, RHS);
  తెలిసిన_బిట్లు Diff1 =
      compute_For_కూడు_Sub(/*Add=*/అబద్ధం, /*NSW=*/అబద్ధం, /*NUW=*/నిజం, RHS, LHS);
  తిరిగి Diff0.intersectWith(Diff1);
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::abds(తెలిసిన_బిట్లు LHS, తెలిసిన_బిట్లు RHS) {
  // If we know which argument is larger, return (sub LHS, RHS) or
  // (sub RHS, LHS) directly.
  ఒకవేళ (LHS.get_Signed_Min_విలువ().sge(RHS.get_Signed_Max_విలువ()))
    తిరిగి compute_For_కూడు_Sub(/*Add=*/అబద్ధం, /*NSW=*/అబద్ధం, /*NUW=*/అబద్ధం, LHS,
                            RHS);
  ఒకవేళ (RHS.get_Signed_Min_విలువ().sge(LHS.get_Signed_Max_విలువ()))
    తిరిగి compute_For_కూడు_Sub(/*Add=*/అబద్ధం, /*NSW=*/అబద్ధం, /*NUW=*/అబద్ధం, RHS,
                            LHS);

  // Shift both arguments from the signed range to the unsigned range, e.g. from
  // [-0x80, 0x7F] to [0, 0xFF]. This allows us to use "sub nuw" below just like
  // abdu does.
  // Note that we can't just use "sub nsw" instead because abds has signed
  // inputs but an unsigned result, which makes the overflow conditions
  // different.
  గుర్తులేని Sign_బిట్టు_Position = LHS.get_బిట్టు_వెడల్పు() - 1;
  కొరకు (స్వయంచాలక Arg : {&LHS, &RHS}) {
    తార్కిక Tmp = Arg->Zero[Sign_బిట్టు_Position];
    Arg->Zero.సమితి_బిట్టు_Val(Sign_బిట్టు_Position, Arg->One[Sign_బిట్టు_Position]);
    Arg->One.సమితి_బిట్టు_Val(Sign_బిట్టు_Position, Tmp);
  }

  // Find the common bits between (sub nuw LHS, RHS) and (sub nuw RHS, LHS).
  తెలిసిన_బిట్లు Diff0 =
      compute_For_కూడు_Sub(/*Add=*/అబద్ధం, /*NSW=*/అబద్ధం, /*NUW=*/నిజం, LHS, RHS);
  తెలిసిన_బిట్లు Diff1 =
      compute_For_కూడు_Sub(/*Add=*/అబద్ధం, /*NSW=*/అబద్ధం, /*NUW=*/నిజం, RHS, LHS);
  తిరిగి Diff0.intersectWith(Diff1);
}

స్థిర గుర్తులేని get_Max_జరుపు_Amount(స్థిరరాశి APInt &Max_విలువ, గుర్తులేని బిట్టు_వెడల్పు_2) {
  ఒకవేళ (isPowerOf2_32(బిట్టు_వెడల్పు_2))
    // Clamp to the shift amount's width: a narrower amount is already
    // < BitWidth, so this stays a valid upper bound.
    తిరిగి Max_విలువ.వెలికితీయి_బిట్లు_As_ZExt_విలువ(
        std::min(Log2_32(బిట్టు_వెడల్పు_2), Max_విలువ.get_బిట్టు_వెడల్పు()), 0);
  // This is only an approximate upper bound.
  తిరిగి Max_విలువ.get_Limited_విలువ(బిట్టు_వెడల్పు_2 - 1);
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::shl(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS, తార్కిక NUW,
                         తార్కిక NSW, తార్కిక ShAmtNonZero) {
  గుర్తులేని బిట్టు_వెడల్పు_2 = LHS.get_బిట్టు_వెడల్పు();
  స్వయంచాలక జరుపు_By_Const = [&](స్థిరరాశి తెలిసిన_బిట్లు &LHS, గుర్తులేని జరుపు_Amt_2) {
    తెలిసిన_బిట్లు తెలిసిన;
    తార్కిక Shifted_బహిర్గతం_Zero, Shifted_బహిర్గతం_One;
    తెలిసిన.Zero = LHS.Zero.ushl_ov(జరుపు_Amt_2, Shifted_బహిర్గతం_Zero);
    తెలిసిన.Zero.సమితి_దిగువ_బిట్లు(జరుపు_Amt_2);
    తెలిసిన.One = LHS.One.ushl_ov(జరుపు_Amt_2, Shifted_బహిర్గతం_One);

    // All cases returning poison have been handled by MaxShiftAmount already.
    ఒకవేళ (NSW) {
      ఒకవేళ (NUW && జరుపు_Amt_2 != 0)
        // NUW means we can assume anything shifted out was a zero.
        Shifted_బహిర్గతం_Zero = నిజం;

      ఒకవేళ (Shifted_బహిర్గతం_Zero)
        తెలిసిన.makeNonNegative();
      లేకపోతే ఒకవేళ (Shifted_బహిర్గతం_One)
        తెలిసిన.makeNegative();
    }
    తిరిగి తెలిసిన;
  };

  // Fast path for a common case when LHS is completely unknown.
  తెలిసిన_బిట్లు తెలిసిన(బిట్టు_వెడల్పు_2);
  గుర్తులేని Min_జరుపు_Amount = RHS.get_Min_విలువ().get_Limited_విలువ(బిట్టు_వెడల్పు_2);
  ఒకవేళ (Min_జరుపు_Amount == 0 && ShAmtNonZero)
    Min_జరుపు_Amount = 1;
  ఒకవేళ (LHS.isUnknown()) {
    తెలిసిన.Zero.సమితి_దిగువ_బిట్లు(Min_జరుపు_Amount);
    ఒకవేళ (NUW && NSW && Min_జరుపు_Amount != 0)
      తెలిసిన.makeNonNegative();
    తిరిగి తెలిసిన;
  }

  // Determine maximum shift amount, taking NUW/NSW flags into account.
  APInt Max_విలువ = RHS.get_Max_విలువ();
  గుర్తులేని Max_జరుపు_Amount = get_Max_జరుపు_Amount(Max_విలువ, బిట్టు_వెడల్పు_2);
  ఒకవేళ (NUW && NSW)
    Max_జరుపు_Amount = std::min(Max_జరుపు_Amount, LHS.లెక్క_Max_Leading_Zeros() - 1);
  ఒకవేళ (NUW)
    Max_జరుపు_Amount = std::min(Max_జరుపు_Amount, LHS.లెక్క_Max_Leading_Zeros());
  ఒకవేళ (NSW)
    Max_జరుపు_Amount = std::min(
        Max_జరుపు_Amount,
        std::max(LHS.లెక్క_Max_Leading_Zeros(), LHS.లెక్క_Max_Leading_Ones()) - 1);

  // Fast path for common case where the shift amount is unknown.
  ఒకవేళ (Min_జరుపు_Amount == 0 && Max_జరుపు_Amount == బిట్టు_వెడల్పు_2 - 1 &&
      isPowerOf2_32(బిట్టు_వెడల్పు_2)) {
    తెలిసిన.Zero.సమితి_దిగువ_బిట్లు(LHS.లెక్క_Min_Trailing_Zeros());
    ఒకవేళ (LHS.isAllOnes())
      తెలిసిన.One.సమితి_Sign_బిట్టు();
    ఒకవేళ (NSW) {
      ఒకవేళ (LHS.isNonNegative())
        తెలిసిన.makeNonNegative();
      ఒకవేళ (LHS.isNegative())
        తెలిసిన.makeNegative();
    }
    తిరిగి తెలిసిన;
  }

  // Find the common bits from all possible shifts.
  గుర్తులేని జరుపు_Amt_Zero_ముసుగు = RHS.Zero.zextOrTrunc(32).get_ZExt_విలువ();
  గుర్తులేని జరుపు_Amt_One_ముసుగు = RHS.One.zextOrTrunc(32).get_ZExt_విలువ();
  తెలిసిన.సమితి_All_Conflict();
  కొరకు (గుర్తులేని జరుపు_Amt_2 = Min_జరుపు_Amount; జరుపు_Amt_2 <= Max_జరుపు_Amount;
       ++జరుపు_Amt_2) {
    // Skip if the shift amount is impossible.
    ఒకవేళ ((జరుపు_Amt_Zero_ముసుగు & జరుపు_Amt_2) != 0 ||
        (జరుపు_Amt_One_ముసుగు | జరుపు_Amt_2) != జరుపు_Amt_2)
      కొనసాగు;
    తెలిసిన = తెలిసిన.intersectWith(జరుపు_By_Const(LHS, జరుపు_Amt_2));
    ఒకవేళ (తెలిసిన.isUnknown())
      ఆపు;
  }

  // All shift amounts may result in poison.
  ఒకవేళ (తెలిసిన.hasConflict())
    తెలిసిన.సమితి_All_Zero();
  తిరిగి తెలిసిన;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::lshr(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS,
                          తార్కిక ShAmtNonZero, తార్కిక Exact) {
  గుర్తులేని బిట్టు_వెడల్పు_2 = LHS.get_బిట్టు_వెడల్పు();
  స్వయంచాలక జరుపు_By_Const = [&](స్థిరరాశి తెలిసిన_బిట్లు &LHS, గుర్తులేని జరుపు_Amt_2) {
    తెలిసిన_బిట్లు తెలిసిన = LHS;
    తెలిసిన >>= జరుపు_Amt_2;
    // High bits are known zero.
    తెలిసిన.Zero.సమితి_ఎగువ_బిట్లు(జరుపు_Amt_2);
    తిరిగి తెలిసిన;
  };

  // Fast path for a common case when LHS is completely unknown.
  తెలిసిన_బిట్లు తెలిసిన(బిట్టు_వెడల్పు_2);
  గుర్తులేని Min_జరుపు_Amount = RHS.get_Min_విలువ().get_Limited_విలువ(బిట్టు_వెడల్పు_2);
  ఒకవేళ (Min_జరుపు_Amount == 0 && ShAmtNonZero)
    Min_జరుపు_Amount = 1;
  ఒకవేళ (LHS.isUnknown()) {
    తెలిసిన.Zero.సమితి_ఎగువ_బిట్లు(Min_జరుపు_Amount);
    తిరిగి తెలిసిన;
  }

  // Find the common bits from all possible shifts.
  APInt Max_విలువ = RHS.get_Max_విలువ();
  గుర్తులేని Max_జరుపు_Amount = get_Max_జరుపు_Amount(Max_విలువ, బిట్టు_వెడల్పు_2);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  ఒకవేళ (Exact) {
    గుర్తులేని మొదటి_One = LHS.లెక్క_Max_Trailing_Zeros();
    ఒకవేళ (మొదటి_One < Min_జరుపు_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      తెలిసిన.సమితి_All_Zero();
      తిరిగి తెలిసిన;
    }
    Max_జరుపు_Amount = std::min(Max_జరుపు_Amount, మొదటి_One);
  }

  గుర్తులేని జరుపు_Amt_Zero_ముసుగు = RHS.Zero.zextOrTrunc(32).get_ZExt_విలువ();
  గుర్తులేని జరుపు_Amt_One_ముసుగు = RHS.One.zextOrTrunc(32).get_ZExt_విలువ();
  తెలిసిన.సమితి_All_Conflict();
  కొరకు (గుర్తులేని జరుపు_Amt_2 = Min_జరుపు_Amount; జరుపు_Amt_2 <= Max_జరుపు_Amount;
       ++జరుపు_Amt_2) {
    // Skip if the shift amount is impossible.
    ఒకవేళ ((జరుపు_Amt_Zero_ముసుగు & జరుపు_Amt_2) != 0 ||
        (జరుపు_Amt_One_ముసుగు | జరుపు_Amt_2) != జరుపు_Amt_2)
      కొనసాగు;
    తెలిసిన = తెలిసిన.intersectWith(జరుపు_By_Const(LHS, జరుపు_Amt_2));
    ఒకవేళ (తెలిసిన.isUnknown())
      ఆపు;
  }

  // All shift amounts may result in poison.
  ఒకవేళ (తెలిసిన.hasConflict())
    తెలిసిన.సమితి_All_Zero();
  తిరిగి తెలిసిన;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::ashr(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS,
                          తార్కిక ShAmtNonZero, తార్కిక Exact) {
  గుర్తులేని బిట్టు_వెడల్పు_2 = LHS.get_బిట్టు_వెడల్పు();
  స్వయంచాలక జరుపు_By_Const = [&](స్థిరరాశి తెలిసిన_బిట్లు &LHS, గుర్తులేని జరుపు_Amt_2) {
    తెలిసిన_బిట్లు తెలిసిన = LHS;
    తెలిసిన.Zero.ashr_లోపల_ఉంచు(జరుపు_Amt_2);
    తెలిసిన.One.ashr_లోపల_ఉంచు(జరుపు_Amt_2);
    తిరిగి తెలిసిన;
  };

  // Fast path for a common case when LHS is completely unknown.
  తెలిసిన_బిట్లు తెలిసిన(బిట్టు_వెడల్పు_2);
  గుర్తులేని Min_జరుపు_Amount = RHS.get_Min_విలువ().get_Limited_విలువ(బిట్టు_వెడల్పు_2);
  ఒకవేళ (Min_జరుపు_Amount == 0 && ShAmtNonZero)
    Min_జరుపు_Amount = 1;
  ఒకవేళ (LHS.isUnknown()) {
    ఒకవేళ (Min_జరుపు_Amount == బిట్టు_వెడల్పు_2) {
      // Always poison. Return zero because we don't like returning conflict.
      తెలిసిన.సమితి_All_Zero();
      తిరిగి తెలిసిన;
    }
    తిరిగి తెలిసిన;
  }

  // Find the common bits from all possible shifts.
  APInt Max_విలువ = RHS.get_Max_విలువ();
  గుర్తులేని Max_జరుపు_Amount = get_Max_జరుపు_Amount(Max_విలువ, బిట్టు_వెడల్పు_2);

  // If exact, bound MaxShiftAmount to first known 1 in LHS.
  ఒకవేళ (Exact) {
    గుర్తులేని మొదటి_One = LHS.లెక్క_Max_Trailing_Zeros();
    ఒకవేళ (మొదటి_One < Min_జరుపు_Amount) {
      // Always poison. Return zero because we don't like returning conflict.
      తెలిసిన.సమితి_All_Zero();
      తిరిగి తెలిసిన;
    }
    Max_జరుపు_Amount = std::min(Max_జరుపు_Amount, మొదటి_One);
  }

  గుర్తులేని జరుపు_Amt_Zero_ముసుగు = RHS.Zero.zextOrTrunc(32).get_ZExt_విలువ();
  గుర్తులేని జరుపు_Amt_One_ముసుగు = RHS.One.zextOrTrunc(32).get_ZExt_విలువ();
  తెలిసిన.సమితి_All_Conflict();
  కొరకు (గుర్తులేని జరుపు_Amt_2 = Min_జరుపు_Amount; జరుపు_Amt_2 <= Max_జరుపు_Amount;
      ++జరుపు_Amt_2) {
    // Skip if the shift amount is impossible.
    ఒకవేళ ((జరుపు_Amt_Zero_ముసుగు & జరుపు_Amt_2) != 0 ||
        (జరుపు_Amt_One_ముసుగు | జరుపు_Amt_2) != జరుపు_Amt_2)
      కొనసాగు;
    తెలిసిన = తెలిసిన.intersectWith(జరుపు_By_Const(LHS, జరుపు_Amt_2));
    ఒకవేళ (తెలిసిన.isUnknown())
      ఆపు;
  }

  // All shift amounts may result in poison.
  ఒకవేళ (తెలిసిన.hasConflict())
    తెలిసిన.సమితి_All_Zero();
  తిరిగి తెలిసిన;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::fshl(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS,
                          స్థిరరాశి APInt &Amt) {
  తిరిగి తెలిసిన_బిట్లు(APIntOps::fshl(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshl(LHS.One, RHS.One, Amt));
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::fshr(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS,
                          స్థిరరాశి APInt &Amt) {
  తిరిగి తెలిసిన_బిట్లు(APIntOps::fshr(LHS.Zero, RHS.Zero, Amt),
                   APIntOps::fshr(LHS.One, RHS.One, Amt));
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::clmul(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  తెలిసిన_బిట్లు Res =
      makeConstant(APIntOps::clmul(LHS.get_Min_విలువ(), RHS.get_Min_విలువ()));

  // This is the same operation as clmul except it accumulates the result with
  // an OR instead of an XOR.
  స్వయంచాలక ClMulOr = [](స్థిరరాశి APInt &LHS, స్థిరరాశి APInt &RHS) {
    గుర్తులేని BW = LHS.get_బిట్టు_వెడల్పు();
    ప్రతిజ్ఞా(BW == RHS.get_బిట్టు_వెడల్పు() && "Operand mismatch");
    APInt ఫలితం(BW, 0);
    కొరకు (గుర్తులేని I :
         seq(std::min(RHS.get_చురుకైన_బిట్లు(), BW - LHS.countr_zero())))
      ఒకవేళ (RHS[I])
        ఫలితం |= LHS << I;
    తిరిగి ఫలితం;
  };

  // Bits in the result are known if, for every corresponding pair of input
  // bits, both input bits are known or either input bit is known to be zero.
  APInt తెలిసిన = ~(ClMulOr(~LHS.Zero & ~LHS.One, ~RHS.Zero) |
                  ClMulOr(~LHS.Zero, ~RHS.Zero & ~RHS.One));
  Res.Zero &= తెలిసిన;
  Res.One &= తెలిసిన;

  తిరిగి Res;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::pext(స్థిరరాశి తెలిసిన_బిట్లు &Val, స్థిరరాశి తెలిసిన_బిట్లు &ముసుగు) {
  గుర్తులేని బిట్టు_వెడల్పు_2 = Val.get_బిట్టు_వెడల్పు();
  తెలిసిన_బిట్లు Res(బిట్టు_వెడల్పు_2);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.సమితి_All_Conflict();

  // For each source position I where Mask[I] could be 1, the output position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible output positions widens.
  గుర్తులేని M0 = 0, M1 = 0;
  కొరకు (గుర్తులేని I = 0; I < బిట్టు_వెడల్పు_2; ++I) {
    ఒకవేళ (!ముసుగు.Zero[I]) {
      // Mask[I] could be 1, so we decide what value the Res bits could have.
      ఒకవేళ (!Val.Zero[I])
        // Val[I] could be 1 => Res[J] for J in [M0, M1] could be 1
        Res.Zero.clear_బిట్లు(M0, M1 + 1);
      ఒకవేళ (!Val.One[I])
        // Val[I] could be 0 => Res[J] for J in [M0, M1] could be 0
        Res.One.clear_బిట్లు(M0, M1 + 1);
    }
    ఒకవేళ (ముసుగు.One[I])
      ++M0, ++M1;
    లేకపోతే ఒకవేళ (!ముసుగు.Zero[I])
      ++M1;
  }

  // Output bits at J >= M0 may have no source (popcount(Mask) may be <= J), so
  // they may be 0.
  Res.One.clear_బిట్లు(M0, బిట్టు_వెడల్పు_2);
  తిరిగి Res;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::pdep(స్థిరరాశి తెలిసిన_బిట్లు &Val, స్థిరరాశి తెలిసిన_బిట్లు &ముసుగు) {
  గుర్తులేని బిట్టు_వెడల్పు_2 = Val.get_బిట్టు_వెడల్పు();
  తెలిసిన_బిట్లు Res(బిట్టు_వెడల్పు_2);
  // We start by asserting that bits cannot be 0 and cannot be 1, then clear
  // Res bits where we know a bit could have some value.
  Res.సమితి_All_Conflict();

  // For each output position I where Mask[I] could be 1, the source position J
  // lies in [M0, M1], where M0 and M1 track the range of possible 1-bit counts
  // seen so far in Mask. Note that M0=M1 as long as bits in Mask are known;
  // otherwise, the range of possible source positions widens.
  గుర్తులేని M0 = 0, M1 = 0;
  కొరకు (గుర్తులేని I = 0; I < బిట్టు_వెడల్పు_2; ++I) {
    ఒకవేళ (!ముసుగు.One[I])
      // Mask[I] could be 0 => Res[I] could be 0
      Res.One.clear_బిట్టు(I);
    ఒకవేళ (!ముసుగు.Zero[I]) {
      // Mask[I] could be 1, so we check what value the Val bits could have.
      APInt Range = APInt::get_బిట్లు_సమితి(బిట్టు_వెడల్పు_2, M0, M1 + 1);
      ఒకవేళ (!Range.is_Subset_యొక్క(Val.One))
        // Any Val[J] for J in [M0, M1] could be 0 => Res[I] could be 0
        Res.One.clear_బిట్టు(I);
      ఒకవేళ (!Range.is_Subset_యొక్క(Val.Zero))
        // Any Val[J] for J in [M0, M1] could be 1 => Res[I] could be 1
        Res.Zero.clear_బిట్టు(I);
    }
    ఒకవేళ (ముసుగు.One[I])
      ++M0, ++M1;
    లేకపోతే ఒకవేళ (!ముసుగు.Zero[I])
      ++M1;
  }
  తిరిగి Res;
}

std::optional<తార్కిక> తెలిసిన_బిట్లు::eq(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  ఒకవేళ (LHS.isConstant() && RHS.isConstant())
    తిరిగి LHS.getConstant() == RHS.getConstant();
  ఒకవేళ (LHS.One.intersects(RHS.Zero) || RHS.One.intersects(LHS.Zero))
    తిరిగి అబద్ధం;
  తిరిగి std::nullopt;
}

std::optional<తార్కిక> తెలిసిన_బిట్లు::ne(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  ఒకవేళ (std::optional<తార్కిక> తెలిసిన_EQ = eq(LHS, RHS))
    తిరిగి !*తెలిసిన_EQ;
  తిరిగి std::nullopt;
}

std::optional<తార్కిక> తెలిసిన_బిట్లు::ugt(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  // LHS >u RHS -> false if umax(LHS) <= umax(RHS)
  ఒకవేళ (LHS.get_Max_విలువ().ule(RHS.get_Min_విలువ()))
    తిరిగి అబద్ధం;
  // LHS >u RHS -> true if umin(LHS) > umax(RHS)
  ఒకవేళ (LHS.get_Min_విలువ().ugt(RHS.get_Max_విలువ()))
    తిరిగి నిజం;
  తిరిగి std::nullopt;
}

std::optional<తార్కిక> తెలిసిన_బిట్లు::uge(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  ఒకవేళ (std::optional<తార్కిక> IsUGT = ugt(RHS, LHS))
    తిరిగి !*IsUGT;
  తిరిగి std::nullopt;
}

std::optional<తార్కిక> తెలిసిన_బిట్లు::ult(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  తిరిగి ugt(RHS, LHS);
}

std::optional<తార్కిక> తెలిసిన_బిట్లు::ule(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  తిరిగి uge(RHS, LHS);
}

std::optional<తార్కిక> తెలిసిన_బిట్లు::sgt(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  // LHS >s RHS -> false if smax(LHS) <= smax(RHS)
  ఒకవేళ (LHS.get_Signed_Max_విలువ().sle(RHS.get_Signed_Min_విలువ()))
    తిరిగి అబద్ధం;
  // LHS >s RHS -> true if smin(LHS) > smax(RHS)
  ఒకవేళ (LHS.get_Signed_Min_విలువ().sgt(RHS.get_Signed_Max_విలువ()))
    తిరిగి నిజం;
  తిరిగి std::nullopt;
}

std::optional<తార్కిక> తెలిసిన_బిట్లు::sge(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  ఒకవేళ (std::optional<తార్కిక> తెలిసిన_SGT = sgt(RHS, LHS))
    తిరిగి !*తెలిసిన_SGT;
  తిరిగి std::nullopt;
}

std::optional<తార్కిక> తెలిసిన_బిట్లు::slt(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  తిరిగి sgt(RHS, LHS);
}

std::optional<తార్కిక> తెలిసిన_బిట్లు::sle(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  తిరిగి sge(RHS, LHS);
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::నిరపేక్ష(తార్కిక IntMinIsPoison) స్థిరరాశి {
  // If the source's MSB is zero then we know the rest of the bits already.
  ఒకవేళ (isNonNegative())
    తిరిగి *ఇది;

  // Absolute value preserves trailing zero count.
  తెలిసిన_బిట్లు తెలిసిన_Abs(get_బిట్టు_వెడల్పు());

  // If the input is negative, then abs(x) == -x.
  ఒకవేళ (isNegative()) {
    తెలిసిన_బిట్లు Tmp = *ఇది;
    // Special case for IntMinIsPoison. We know the sign bit is set and we know
    // all the rest of the bits except one to be zero. Since we have
    // IntMinIsPoison, that final bit MUST be a one, as otherwise the input is
    // INT_MIN.
    ఒకవేళ (IntMinIsPoison && (Zero.popcount() + 2) == get_బిట్టు_వెడల్పు())
      Tmp.One.సమితి_బిట్టు(లెక్క_Min_Trailing_Zeros());

    తెలిసిన_Abs = compute_For_కూడు_Sub(
        /*కూడు*/ అబద్ధం, IntMinIsPoison, /*NUW=*/అబద్ధం,
        తెలిసిన_బిట్లు::makeConstant(APInt(get_బిట్టు_వెడల్పు(), 0)), Tmp);

    // One more special case for IntMinIsPoison. If we don't know any ones other
    // than the signbit, we know for certain that all the unknowns can't be
    // zero. So if we know high zero bits, but have unknown low bits, we know
    // for certain those high-zero bits will end up as one. This is because,
    // the low bits can't be all zeros, so the +1 in (~x + 1) cannot carry up
    // to the high bits. If we know a known INT_MIN input skip this. The result
    // is poison anyways.
    ఒకవేళ (IntMinIsPoison && Tmp.లెక్క_Min_Population() == 1 &&
        Tmp.లెక్క_Max_Population() != 1) {
      Tmp.One.clear_Sign_బిట్టు();
      Tmp.Zero.సమితి_Sign_బిట్టు();
      తెలిసిన_Abs.One.సమితి_బిట్లు(get_బిట్టు_వెడల్పు() - Tmp.లెక్క_Min_Leading_Zeros(),
                           get_బిట్టు_వెడల్పు() - 1);
    }

  } లేకపోతే {
    గుర్తులేని MaxTZ = లెక్క_Max_Trailing_Zeros();
    గుర్తులేని MinTZ = లెక్క_Min_Trailing_Zeros();

    తెలిసిన_Abs.Zero.సమితి_దిగువ_బిట్లు(MinTZ);
    // If we know the lowest set 1, then preserve it.
    ఒకవేళ (MaxTZ == MinTZ && MaxTZ < get_బిట్టు_వెడల్పు())
      తెలిసిన_Abs.One.సమితి_బిట్టు(MaxTZ);

    // We only know that the absolute values's MSB will be zero if INT_MIN is
    // poison, or there is a set bit that isn't the sign bit (otherwise it could
    // be INT_MIN).
    ఒకవేళ (IntMinIsPoison || (!One.isZero() && !One.is_Min_Signed_విలువ())) {
      తెలిసిన_Abs.One.clear_Sign_బిట్టు();
      తెలిసిన_Abs.Zero.సమితి_Sign_బిట్టు();
    }
  }

  తిరిగి తెలిసిన_Abs;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::reduce_కూడు(గుర్తులేని అంకె_Elts) స్థిరరాశి {
  ఒకవేళ (అంకె_Elts == 0)
    తిరిగి తెలిసిన_బిట్లు(get_బిట్టు_వెడల్పు());

  గుర్తులేని బిట్టు_వెడల్పు_2 = get_బిట్టు_వెడల్పు();
  తెలిసిన_బిట్లు ఫలితం(బిట్టు_వెడల్పు_2);

  ఒకవేళ (isConstant())
    // If all elements are the same constant, we can simply compute it
    తిరిగి తెలిసిన_బిట్లు::makeConstant(అంకె_Elts * getConstant());

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
  గుర్తులేని కోల్పోయిన_బిట్లు = Log2_32_Ceil(అంకె_Elts);

  ఒకవేళ (isNonNegative()) {
    గుర్తులేని LeadingZeros = లెక్క_Min_Leading_Zeros();
    LeadingZeros = LeadingZeros > కోల్పోయిన_బిట్లు ? LeadingZeros - కోల్పోయిన_బిట్లు : 0;
    ఫలితం.Zero.సమితి_ఎగువ_బిట్లు(LeadingZeros);
  } లేకపోతే ఒకవేళ (isNegative()) {
    గుర్తులేని LeadingOnes = లెక్క_Min_Leading_Ones();
    LeadingOnes = LeadingOnes > కోల్పోయిన_బిట్లు ? LeadingOnes - కోల్పోయిన_బిట్లు : 0;
    ఫలితం.One.సమితి_ఎగువ_బిట్లు(LeadingOnes);
  }

  తిరిగి ఫలితం;
}

స్థిర తెలిసిన_బిట్లు compute_For_Sat_కూడు_Sub(తార్కిక కూడు_2, తార్కిక Signed,
                                     స్థిరరాశి తెలిసిన_బిట్లు &LHS,
                                     స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  // We don't see NSW even for sadd/ssub as we want to check if the result has
  // signed overflow.
  గుర్తులేని బిట్టు_వెడల్పు_2 = LHS.get_బిట్టు_వెడల్పు();

  std::optional<తార్కిక> అధికప్రవాహం;
  // Even if we can't entirely rule out overflow, we may be able to rule out
  // overflow in one direction. This allows us to potentially keep some of the
  // add/sub bits. I.e if we can't overflow in the positive direction we won't
  // clamp to INT_MAX so we can keep low 0s from the add/sub result.
  తార్కిక MayNegClamp = నిజం;
  తార్కిక MayPosClamp = నిజం;
  ఒకవేళ (Signed) {
    // Easy cases we can rule out any overflow.
    ఒకవేళ (కూడు_2 && ((LHS.isNegative() && RHS.isNonNegative()) ||
                (LHS.isNonNegative() && RHS.isNegative())))
      అధికప్రవాహం = అబద్ధం;
    లేకపోతే ఒకవేళ (!కూడు_2 && (((LHS.isNegative() && RHS.isNegative()) ||
                       (LHS.isNonNegative() && RHS.isNonNegative()))))
      అధికప్రవాహం = అబద్ధం;
    లేకపోతే {
      // Check if we may overflow. If we can't rule out overflow then check if
      // we can rule out a direction at least.
      తెలిసిన_బిట్లు UnsignedLHS = LHS;
      తెలిసిన_బిట్లు UnsignedRHS = RHS;
      // Get version of LHS/RHS with clearer signbit. This allows us to detect
      // how the addition/subtraction might overflow into the signbit. Then
      // using the actual known signbits of LHS/RHS, we can figure out which
      // overflows are/aren't possible.
      UnsignedLHS.One.clear_Sign_బిట్టు();
      UnsignedLHS.Zero.సమితి_Sign_బిట్టు();
      UnsignedRHS.One.clear_Sign_బిట్టు();
      UnsignedRHS.Zero.సమితి_Sign_బిట్టు();
      తెలిసిన_బిట్లు Res =
          తెలిసిన_బిట్లు::compute_For_కూడు_Sub(కూడు_2, /*NSW=*/అబద్ధం,
                                      /*NUW=*/అబద్ధం, UnsignedLHS, UnsignedRHS);
      ఒకవేళ (కూడు_2) {
        ఒకవేళ (Res.isNegative()) {
          // Only overflow scenario is Pos + Pos.
          MayNegClamp = అబద్ధం;
          // Pos + Pos will overflow with extra signbit.
          ఒకవేళ (LHS.isNonNegative() && RHS.isNonNegative())
            అధికప్రవాహం = నిజం;
        } లేకపోతే ఒకవేళ (Res.isNonNegative()) {
          // Only overflow scenario is Neg + Neg
          MayPosClamp = అబద్ధం;
          // Neg + Neg will overflow without extra signbit.
          ఒకవేళ (LHS.isNegative() && RHS.isNegative())
            అధికప్రవాహం = నిజం;
        }
        // We will never clamp to the opposite sign of N-bit result.
        ఒకవేళ (LHS.isNegative() || RHS.isNegative())
          MayPosClamp = అబద్ధం;
        ఒకవేళ (LHS.isNonNegative() || RHS.isNonNegative())
          MayNegClamp = అబద్ధం;
      } లేకపోతే {
        ఒకవేళ (Res.isNegative()) {
          // Only overflow scenario is Neg - Pos.
          MayPosClamp = అబద్ధం;
          // Neg - Pos will overflow with extra signbit.
          ఒకవేళ (LHS.isNegative() && RHS.isNonNegative())
            అధికప్రవాహం = నిజం;
        } లేకపోతే ఒకవేళ (Res.isNonNegative()) {
          // Only overflow scenario is Pos - Neg.
          MayNegClamp = అబద్ధం;
          // Pos - Neg will overflow without extra signbit.
          ఒకవేళ (LHS.isNonNegative() && RHS.isNegative())
            అధికప్రవాహం = నిజం;
        }
        // We will never clamp to the opposite sign of N-bit result.
        ఒకవేళ (LHS.isNegative() || RHS.isNonNegative())
          MayPosClamp = అబద్ధం;
        ఒకవేళ (LHS.isNonNegative() || RHS.isNegative())
          MayNegClamp = అబద్ధం;
      }
    }
    // If we have ruled out all clamping, we will never overflow.
    ఒకవేళ (!MayNegClamp && !MayPosClamp)
      అధికప్రవాహం = అబద్ధం;
  } లేకపోతే ఒకవేళ (కూడు_2) {
    // uadd.sat
    తార్కిక యొక్క;
    (శూన్య)LHS.get_Max_విలువ().uadd_ov(RHS.get_Max_విలువ(), యొక్క);
    ఒకవేళ (!యొక్క) {
      అధికప్రవాహం = అబద్ధం;
    } లేకపోతే {
      (శూన్య)LHS.get_Min_విలువ().uadd_ov(RHS.get_Min_విలువ(), యొక్క);
      ఒకవేళ (యొక్క)
        అధికప్రవాహం = నిజం;
    }
  } లేకపోతే {
    // usub.sat
    తార్కిక యొక్క;
    (శూన్య)LHS.get_Min_విలువ().usub_ov(RHS.get_Max_విలువ(), యొక్క);
    ఒకవేళ (!యొక్క) {
      అధికప్రవాహం = అబద్ధం;
    } లేకపోతే {
      (శూన్య)LHS.get_Max_విలువ().usub_ov(RHS.get_Min_విలువ(), యొక్క);
      ఒకవేళ (యొక్క)
        అధికప్రవాహం = నిజం;
    }
  }

  తెలిసిన_బిట్లు Res = తెలిసిన_బిట్లు::compute_For_కూడు_Sub(కూడు_2, /*NSW=*/Signed,
                                              /*NUW=*/!Signed, LHS, RHS);

  ఒకవేళ (అధికప్రవాహం) {
    // We know whether or not we overflowed.
    ఒకవేళ (!(*అధికప్రవాహం)) {
      // No overflow.
      తిరిగి Res;
    }

    // We overflowed
    APInt C;
    ఒకవేళ (Signed) {
      // sadd.sat / ssub.sat
      ప్రతిజ్ఞా(!LHS.isSignUnknown() &&
             "We somehow know overflow without knowing input sign");
      C = LHS.isNegative() ? APInt::get_Signed_Min_విలువ(బిట్టు_వెడల్పు_2)
                           : APInt::get_Signed_Max_విలువ(బిట్టు_వెడల్పు_2);
    } లేకపోతే ఒకవేళ (కూడు_2) {
      // uadd.sat
      C = APInt::get_Max_విలువ(బిట్టు_వెడల్పు_2);
    } లేకపోతే {
      // uadd.sat
      C = APInt::get_Min_విలువ(బిట్టు_వెడల్పు_2);
    }

    Res.One = C;
    Res.Zero = ~C;
    తిరిగి Res;
  }

  // We don't know if we overflowed.
  ఒకవేళ (Signed) {
    // sadd.sat/ssub.sat
    // We can keep our information about the sign bits.
    ఒకవేళ (MayPosClamp)
      Res.Zero.clear_దిగువ_బిట్లు(బిట్టు_వెడల్పు_2 - 1);
    ఒకవేళ (MayNegClamp)
      Res.One.clear_దిగువ_బిట్లు(బిట్టు_వెడల్పు_2 - 1);
  } లేకపోతే ఒకవేళ (కూడు_2) {
    // uadd.sat
    // We need to clear all the known zeros as we can only use the leading ones.
    Res.Zero.clear_All_బిట్లు();
  } లేకపోతే {
    // usub.sat
    // We need to clear all the known ones as we can only use the leading zero.
    Res.One.clear_All_బిట్లు();
  }

  తిరిగి Res;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::sadd_sat(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  తిరిగి compute_For_Sat_కూడు_Sub(/*కూడు*/ నిజం, /*Signed*/ నిజం, LHS, RHS);
}
తెలిసిన_బిట్లు తెలిసిన_బిట్లు::ssub_sat(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  తిరిగి compute_For_Sat_కూడు_Sub(/*కూడు*/ అబద్ధం, /*Signed*/ నిజం, LHS, RHS);
}
తెలిసిన_బిట్లు తెలిసిన_బిట్లు::uadd_sat(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  తిరిగి compute_For_Sat_కూడు_Sub(/*కూడు*/ నిజం, /*Signed*/ అబద్ధం, LHS, RHS);
}
తెలిసిన_బిట్లు తెలిసిన_బిట్లు::usub_sat(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  తిరిగి compute_For_Sat_కూడు_Sub(/*కూడు*/ అబద్ధం, /*Signed*/ అబద్ధం, LHS, RHS);
}

స్థిర తెలిసిన_బిట్లు avgComputeU(తెలిసిన_బిట్లు LHS, తెలిసిన_బిట్లు RHS, తార్కిక IsCeil) {
  గుర్తులేని బిట్టు_వెడల్పు_2 = LHS.get_బిట్టు_వెడల్పు();
  LHS = LHS.zext(బిట్టు_వెడల్పు_2 + 1);
  RHS = RHS.zext(బిట్టు_వెడల్పు_2 + 1);
  LHS =
      compute_For_కూడు_వాహకం(LHS, RHS, /*CarryZero*/ !IsCeil, /*CarryOne*/ IsCeil);
  LHS = LHS.వెలికితీయి_బిట్లు(బిట్టు_వెడల్పు_2, 1);
  తిరిగి LHS;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::avgFloorS(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  తిరిగి flip_Sign_బిట్టు(avgFloorU(flip_Sign_బిట్టు(LHS), flip_Sign_బిట్టు(RHS)));
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::avgFloorU(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  తిరిగి avgComputeU(LHS, RHS, /*IsCeil=*/అబద్ధం);
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::avgCeilS(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  తిరిగి flip_Sign_బిట్టు(avgCeilU(flip_Sign_బిట్టు(LHS), flip_Sign_బిట్టు(RHS)));
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::avgCeilU(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  తిరిగి avgComputeU(LHS, RHS, /*IsCeil=*/నిజం);
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::mul(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS,
                         తార్కిక NoUndefSelfMultiply) {
  గుర్తులేని బిట్టు_వెడల్పు_2 = LHS.get_బిట్టు_వెడల్పు();
  ప్రతిజ్ఞా(బిట్టు_వెడల్పు_2 == RHS.get_బిట్టు_వెడల్పు() && "Operand mismatch");
  ప్రతిజ్ఞా((!NoUndefSelfMultiply || LHS == RHS) &&
         "Self multiplication knownbits mismatch");

  // Compute the high known-0 bits by multiplying the unsigned max of each side.
  // Conservatively, M active bits * N active bits results in M + N bits in the
  // result. But if we know a value is a power-of-2 for example, then this
  // computes one more leading zero.
  // TODO: This could be generalized to number of sign bits (negative numbers).
  APInt UMaxLHS = LHS.get_Max_విలువ();
  APInt UMaxRHS = RHS.get_Max_విలువ();

  // For leading zeros in the result to be valid, the unsigned max product must
  // fit in the bitwidth (it must not overflow).
  తార్కిక Has_అధికప్రవాహం;
  APInt UMax_ఫలితం = UMaxLHS.umul_ov(UMaxRHS, Has_అధికప్రవాహం);
  గుర్తులేని LeadZ = Has_అధికప్రవాహం ? 0 : UMax_ఫలితం.countl_zero();

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
  గుర్తులేని Trail_బిట్లు_తెలిసిన_LHS = (LHS.Zero | LHS.One).countr_one();
  గుర్తులేని Trail_బిట్లు_తెలిసిన_RHS = (RHS.Zero | RHS.One).countr_one();
  గుర్తులేని TrailZeroLHS = LHS.లెక్క_Min_Trailing_Zeros();
  గుర్తులేని TrailZeroRHS = RHS.లెక్క_Min_Trailing_Zeros();
  గుర్తులేని TrailZ = TrailZeroLHS + TrailZeroRHS;

  // Figure out the fewest known-bits operand.
  గుర్తులేని అతిచిన్న_నిర్వాహ్యం = std::min(Trail_బిట్లు_తెలిసిన_LHS - TrailZeroLHS,
                                      Trail_బిట్లు_తెలిసిన_RHS - TrailZeroRHS);
  గుర్తులేని ఫలితం_బిట్లు_తెలిసిన = std::min(అతిచిన్న_నిర్వాహ్యం + TrailZ, బిట్టు_వెడల్పు_2);

  // The lower ResultBitsKnown bits of this are known.
  APInt Bottom_తెలిసిన = LHS.One * RHS.One;

  తెలిసిన_బిట్లు Res(బిట్టు_వెడల్పు_2);
  Res.Zero.సమితి_ఎగువ_బిట్లు(LeadZ);
  Res.Zero |= (~Bottom_తెలిసిన).get_Lo_బిట్లు(ఫలితం_బిట్లు_తెలిసిన);
  Res.One = Bottom_తెలిసిన.get_Lo_బిట్లు(ఫలితం_బిట్లు_తెలిసిన);

  ఒకవేళ (NoUndefSelfMultiply) {
    // If X has at least TZ trailing zeroes, then bit (2 * TZ + 1) must be zero.
    గుర్తులేని TwoTZP1 = 2 * TrailZeroLHS + 1;
    ఒకవేళ (TwoTZP1 < బిట్టు_వెడల్పు_2)
      Res.Zero.సమితి_బిట్టు(TwoTZP1);

    // If X has exactly TZ trailing zeros, then bit (2 * TZ + 2) must also be
    // zero.
    ఒకవేళ (TrailZeroLHS < బిట్టు_వెడల్పు_2 && LHS.One[TrailZeroLHS]) {
      గుర్తులేని TwoTZP2 = TwoTZP1 + 1;
      ఒకవేళ (TwoTZP2 < బిట్టు_వెడల్పు_2)
        Res.Zero.సమితి_బిట్టు(TwoTZP2);
    }
  }

  తిరిగి Res;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::mulhs(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  గుర్తులేని బిట్టు_వెడల్పు_2 = LHS.get_బిట్టు_వెడల్పు();
  ప్రతిజ్ఞా(బిట్టు_వెడల్పు_2 == RHS.get_బిట్టు_వెడల్పు() && "Operand mismatch");
  తెలిసిన_బిట్లు WideLHS = LHS.sext(2 * బిట్టు_వెడల్పు_2);
  తెలిసిన_బిట్లు WideRHS = RHS.sext(2 * బిట్టు_వెడల్పు_2);
  తిరిగి mul(WideLHS, WideRHS).వెలికితీయి_బిట్లు(బిట్టు_వెడల్పు_2, బిట్టు_వెడల్పు_2);
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::mulhu(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  గుర్తులేని బిట్టు_వెడల్పు_2 = LHS.get_బిట్టు_వెడల్పు();
  ప్రతిజ్ఞా(బిట్టు_వెడల్పు_2 == RHS.get_బిట్టు_వెడల్పు() && "Operand mismatch");
  తెలిసిన_బిట్లు WideLHS = LHS.zext(2 * బిట్టు_వెడల్పు_2);
  తెలిసిన_బిట్లు WideRHS = RHS.zext(2 * బిట్టు_వెడల్పు_2);
  తిరిగి mul(WideLHS, WideRHS).వెలికితీయి_బిట్లు(బిట్టు_వెడల్పు_2, బిట్టు_వెడల్పు_2);
}

స్థిర తెలిసిన_బిట్లు div_Compute_దిగువ_బిట్టు(తెలిసిన_బిట్లు తెలిసిన, స్థిరరాశి తెలిసిన_బిట్లు &LHS,
                                  స్థిరరాశి తెలిసిన_బిట్లు &RHS, తార్కిక Exact) {

  ఒకవేళ (!Exact)
    తిరిగి తెలిసిన;

  // If LHS is Odd, the result is Odd no matter what.
  // Odd / Odd -> Odd
  // Odd / Even -> Impossible (because its exact division)
  ఒకవేళ (LHS.One[0])
    తెలిసిన.One.సమితి_బిట్టు(0);

  పూర్ణాంకం MinTZ =
      (పూర్ణాంకం)LHS.లెక్క_Min_Trailing_Zeros() - (పూర్ణాంకం)RHS.లెక్క_Max_Trailing_Zeros();
  పూర్ణాంకం MaxTZ =
      (పూర్ణాంకం)LHS.లెక్క_Max_Trailing_Zeros() - (పూర్ణాంకం)RHS.లెక్క_Min_Trailing_Zeros();
  ఒకవేళ (MinTZ >= 0) {
    // Result has at least MinTZ trailing zeros.
    తెలిసిన.Zero.సమితి_దిగువ_బిట్లు(MinTZ);
    ఒకవేళ (MinTZ == MaxTZ) {
      // Result has exactly MinTZ trailing zeros.
      తెలిసిన.One.సమితి_బిట్టు(MinTZ);
    }
  } లేకపోతే ఒకవేళ (MaxTZ < 0) {
    // Poison Result
    తెలిసిన.సమితి_All_Zero();
  }

  // In the KnownBits exhaustive tests, we have poison inputs for exact values
  // a LOT. If we have a conflict, just return all zeros.
  ఒకవేళ (తెలిసిన.hasConflict())
    తెలిసిన.సమితి_All_Zero();

  తిరిగి తెలిసిన;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::sdiv(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS,
                          తార్కిక Exact) {
  // Equivalent of `udiv`. We must have caught this before it was folded.
  ఒకవేళ (LHS.isNonNegative() && RHS.isNonNegative())
    తిరిగి udiv(LHS, RHS, Exact);

  గుర్తులేని బిట్టు_వెడల్పు_2 = LHS.get_బిట్టు_వెడల్పు();
  తెలిసిన_బిట్లు తెలిసిన(బిట్టు_వెడల్పు_2);

  ఒకవేళ (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    తెలిసిన.సమితి_All_Zero();
    తిరిగి తెలిసిన;
  }

  std::optional<APInt> Res;
  ఒకవేళ (LHS.isNegative() && RHS.isNegative()) {
    // Result non-negative.
    APInt Denom = RHS.get_Signed_Max_విలువ();
    APInt అంకె = LHS.get_Signed_Min_విలువ();
    // INT_MIN/-1 would be a poison result (impossible). Estimate the division
    // as signed max (we will only set sign bit in the result).
    Res = (అంకె.is_Min_Signed_విలువ() && Denom.isAllOnes())
              ? APInt::get_Signed_Max_విలువ(బిట్టు_వెడల్పు_2)
              : అంకె.sdiv(Denom);
  } లేకపోతే ఒకవేళ (LHS.isNegative() && RHS.isNonNegative()) {
    // Result is negative if Exact OR -LHS u>= RHS.
    ఒకవేళ (Exact || (-LHS.get_Signed_Max_విలువ()).uge(RHS.get_Signed_Max_విలువ())) {
      APInt Denom = RHS.get_Signed_Min_విలువ();
      APInt అంకె = LHS.get_Signed_Min_విలువ();
      // If the denominator may be zero, use the lowest unknown bit as the
      // minimum non-zero denominator, matching udiv.
      ఒకవేళ (Denom.isZero())
        Denom.సమితి_బిట్టు(RHS.లెక్క_Min_Trailing_Zeros());
      Res = అంకె.sdiv(Denom);
    }
  } లేకపోతే ఒకవేళ (LHS.is_Strictly_ధనాత్మక() && RHS.isNegative()) {
    // Result is negative if Exact OR LHS u>= -RHS.
    ఒకవేళ (Exact || LHS.get_Signed_Min_విలువ().uge(-RHS.get_Signed_Min_విలువ())) {
      APInt Denom = RHS.get_Signed_Max_విలువ();
      APInt అంకె = LHS.get_Signed_Max_విలువ();
      Res = అంకె.sdiv(Denom);
    }
  }

  ఒకవేళ (Res) {
    ఒకవేళ (Res->isNonNegative()) {
      గుర్తులేని LeadZ = Res->లెక్క_Leading_Zeros();
      తెలిసిన.Zero.సమితి_ఎగువ_బిట్లు(LeadZ);
    } లేకపోతే {
      గుర్తులేని LeadO = Res->లెక్క_Leading_Ones();
      తెలిసిన.One.సమితి_ఎగువ_బిట్లు(LeadO);
    }
  }

  తెలిసిన = div_Compute_దిగువ_బిట్టు(తెలిసిన, LHS, RHS, Exact);
  తిరిగి తెలిసిన;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::udiv(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS,
                          తార్కిక Exact) {
  గుర్తులేని బిట్టు_వెడల్పు_2 = LHS.get_బిట్టు_వెడల్పు();
  తెలిసిన_బిట్లు తెలిసిన(బిట్టు_వెడల్పు_2);

  ఒకవేళ (LHS.isZero() || RHS.isZero()) {
    // Result is either known Zero or UB. Return Zero either way.
    // Checking this earlier saves us a lot of special cases later on.
    తెలిసిన.సమితి_All_Zero();
    తిరిగి తెలిసిన;
  }

  // A zero denominator is UB, so the minimum attainable denominator is the
  // smallest non-zero value consistent with the known bits.
  APInt MinDenom = RHS.get_Min_విలువ();
  ఒకవేళ (MinDenom.isZero())
    MinDenom.సమితి_బిట్టు(RHS.లెక్క_Min_Trailing_Zeros());

  // The quotient grows when the numerator grows and shrinks when the
  // denominator grows, and all four operand bounds are attainable, so the
  // result is in [MinNum / MaxDenom, MaxNum / MinDenom]. Keep the common
  // leading bits of the two bounds (as ConstantRange::toKnownBits does),
  // which can include leading ones, not just leading zeros.
  APInt MaxRes = LHS.get_Max_విలువ().udiv(MinDenom);
  APInt MinRes = LHS.get_Min_విలువ().udiv(RHS.get_Max_విలువ());

  తెలిసిన = తెలిసిన_బిట్లు::makeConstant(MinRes);
  ఒకవేళ (std::optional<గుర్తులేని> Different_బిట్టు =
          APIntOps::Get_Most_Significant_Different_బిట్టు(MinRes, MaxRes)) {
    తెలిసిన.Zero.clear_దిగువ_బిట్లు(*Different_బిట్టు + 1);
    తెలిసిన.One.clear_దిగువ_బిట్లు(*Different_బిట్టు + 1);
  }

  తెలిసిన = div_Compute_దిగువ_బిట్టు(తెలిసిన, LHS, RHS, Exact);

  తిరిగి తెలిసిన;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::rem_Get_దిగువ_బిట్లు(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  గుర్తులేని బిట్టు_వెడల్పు_2 = LHS.get_బిట్టు_వెడల్పు();
  ఒకవేళ (!RHS.isZero() && RHS.Zero[0]) {
    // rem X, Y where Y[0:N] is zero will preserve X[0:N] in the result.
    గుర్తులేని RHSZeros = RHS.లెక్క_Min_Trailing_Zeros();
    APInt ముసుగు = APInt::get_దిగువ_బిట్లు_సమితి(బిట్టు_వెడల్పు_2, RHSZeros);
    APInt Ones_ముసుగు = LHS.One & ముసుగు;
    APInt Zeros_ముసుగు = LHS.Zero & ముసుగు;
    తిరిగి తెలిసిన_బిట్లు(Zeros_ముసుగు, Ones_ముసుగు);
  }
  తిరిగి తెలిసిన_బిట్లు(బిట్టు_వెడల్పు_2);
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::urem(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  // L % R for L < R is L. (A zero divisor is UB, and if MinR is zero this
  // condition cannot hold anyway.)
  ఒకవేళ (LHS.get_Max_విలువ().ult(RHS.get_Min_విలువ()))
    తిరిగి LHS;

  తెలిసిన_బిట్లు తెలిసిన = rem_Get_దిగువ_బిట్లు(LHS, RHS);

  // L % R is at most L and, since a zero divisor is UB, strictly less than
  // R, so the result has at least as many leading zeros as
  // umin(MaxL, MaxR - 1), as in ConstantRange::urem. For a power-of-two
  // constant R this computes the exact L & (R - 1): remGetLowBits preserved
  // the low bits and this clears the bits from log2(R) up. If R is known to
  // be zero, MaxR - 1 wraps around, which is harmless: every input is UB.
  APInt MaxRes = APIntOps::umin(LHS.get_Max_విలువ(), RHS.get_Max_విలువ() - 1);
  తెలిసిన.Zero.సమితి_ఎగువ_బిట్లు(MaxRes.లెక్క_Leading_Zeros());
  తిరిగి తెలిసిన;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::srem(స్థిరరాశి తెలిసిన_బిట్లు &LHS, స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  తెలిసిన_బిట్లు తెలిసిన = rem_Get_దిగువ_బిట్లు(LHS, RHS);
  ఒకవేళ (RHS.isConstant() && RHS.getConstant().isPowerOf2()) {
    // NB: Low bits are set in `remGetLowBits`.
    APInt దిగువ_బిట్లు = RHS.getConstant() - 1;
    // If the first operand is non-negative or has all low bits zero, then
    // the upper bits are all zero.
    ఒకవేళ (LHS.isNonNegative() || దిగువ_బిట్లు.is_Subset_యొక్క(LHS.Zero))
      తెలిసిన.Zero |= ~దిగువ_బిట్లు;

    // If the first operand is negative and not all low bits are zero, then
    // the upper bits are all one.
    ఒకవేళ (LHS.isNegative() && దిగువ_బిట్లు.intersects(LHS.One))
      తెలిసిన.One |= ~దిగువ_బిట్లు;
    తిరిగి తెలిసిన;
  }

  // The sign bit is the LHS's sign bit, except when the result of the
  // remainder is zero. The magnitude of the result should be less than or
  // equal to the magnitude of either operand.
  ఒకవేళ (LHS.isNegative() && తెలిసిన.isNonZero())
    తెలిసిన.One.సమితి_ఎగువ_బిట్లు(
        std::max(LHS.లెక్క_Min_Leading_Ones(), RHS.లెక్క_Min_Sign_బిట్లు()));
  లేకపోతే ఒకవేళ (LHS.isNonNegative())
    తెలిసిన.Zero.సమితి_ఎగువ_బిట్లు(
        std::max(LHS.లెక్క_Min_Leading_Zeros(), RHS.లెక్క_Min_Sign_బిట్లు()));
  తిరిగి తెలిసిన;
}

తెలిసిన_బిట్లు &తెలిసిన_బిట్లు::క్రియాకారకం&=(స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  // Result bit is 0 if either operand bit is 0.
  Zero |= RHS.Zero;
  // Result bit is 1 if both operand bits are 1.
  One &= RHS.One;
  తిరిగి *ఇది;
}

తెలిసిన_బిట్లు &తెలిసిన_బిట్లు::క్రియాకారకం|=(స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  // Result bit is 0 if both operand bits are 0.
  Zero &= RHS.Zero;
  // Result bit is 1 if either operand bit is 1.
  One |= RHS.One;
  తిరిగి *ఇది;
}

తెలిసిన_బిట్లు &తెలిసిన_బిట్లు::క్రియాకారకం^=(స్థిరరాశి తెలిసిన_బిట్లు &RHS) {
  // Result bit is 0 if both operand bits are 0 or both are 1.
  APInt Z = (Zero & RHS.Zero) | (One & RHS.One);
  // Result bit is 1 if one operand bit is 0 and the other is 1.
  One = (Zero & RHS.One) | (One & RHS.Zero);
  Zero = std::move(Z);
  తిరిగి *ఇది;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::blsi() స్థిరరాశి {
  గుర్తులేని బిట్టు_వెడల్పు_2 = get_బిట్టు_వెడల్పు();
  తెలిసిన_బిట్లు తెలిసిన(Zero, APInt(బిట్టు_వెడల్పు_2, 0));
  గుర్తులేని Max = లెక్క_Max_Trailing_Zeros();
  తెలిసిన.Zero.సమితి_బిట్లు_From(std::min(Max + 1, బిట్టు_వెడల్పు_2));
  గుర్తులేని Min = లెక్క_Min_Trailing_Zeros();
  ఒకవేళ (Max == Min && Max < బిట్టు_వెడల్పు_2)
    తెలిసిన.One.సమితి_బిట్టు(Max);
  తిరిగి తెలిసిన;
}

తెలిసిన_బిట్లు తెలిసిన_బిట్లు::blsmsk() స్థిరరాశి {
  గుర్తులేని బిట్టు_వెడల్పు_2 = get_బిట్టు_వెడల్పు();
  తెలిసిన_బిట్లు తెలిసిన(బిట్టు_వెడల్పు_2);
  గుర్తులేని Max = లెక్క_Max_Trailing_Zeros();
  తెలిసిన.Zero.సమితి_బిట్లు_From(std::min(Max + 1, బిట్టు_వెడల్పు_2));
  గుర్తులేని Min = లెక్క_Min_Trailing_Zeros();
  తెలిసిన.One.సమితి_దిగువ_బిట్లు(std::min(Min + 1, బిట్టు_వెడల్పు_2));
  తిరిగి తెలిసిన;
}

శూన్య తెలిసిన_బిట్లు::print(raw_ostream &OS) స్థిరరాశి {
  గుర్తులేని బిట్టు_వెడల్పు_2 = get_బిట్టు_వెడల్పు();
  కొరకు (గుర్తులేని I = 0; I < బిట్టు_వెడల్పు_2; ++I) {
    గుర్తులేని N = బిట్టు_వెడల్పు_2 - I - 1;
    ఒకవేళ (Zero[N] && One[N])
      OS << "!";
    లేకపోతే ఒకవేళ (Zero[N])
      OS << "0";
    లేకపోతే ఒకవేళ (One[N])
      OS << "1";
    లేకపోతే
      OS << "?";
  }
}

#ఒకవేళ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_పద్ధతి శూన్య తెలిసిన_బిట్లు::dump() స్థిరరాశి {
  print(dbgs());
  dbgs() << "\n";
}
#అయితే_ముగింపు

గుర్తులేని llvm::Sign_బిట్లు_Ops::rot(గుర్తులేని Src_Sign_బిట్లు, గుర్తులేని బిట్టు_వెడల్పు_2,
                                std::optional<APInt> RotAmt,
                                తార్కిక Is_Rotate_కుడి) {
  // If we're rotating an 0/-1 value, then it stays an 0/-1 value.
  ఒకవేళ (Src_Sign_బిట్లు == బిట్టు_వెడల్పు_2)
    తిరిగి బిట్టు_వెడల్పు_2;

  ఒకవేళ (!RotAmt)
    తిరిగి 1;

  గుర్తులేని Amt = RotAmt->urem(బిట్టు_వెడల్పు_2);

  // Handle rotate right by N like a rotate left by BitWidth-N.
  ఒకవేళ (Is_Rotate_కుడి)
    Amt = (బిట్టు_వెడల్పు_2 - Amt) % బిట్టు_వెడల్పు_2;

  // If we aren't rotating out all of the known-in sign bits, return the
  // number that are left. This handles rotl(sext(x), 1) for example.
  ఒకవేళ (Src_Sign_బిట్లు > Amt + 1)
    తిరిగి Src_Sign_బిట్లు - Amt;
  తిరిగి 1;
}
