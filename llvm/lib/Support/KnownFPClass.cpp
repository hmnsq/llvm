//===- llvm/Support/KnownFPClass.h - Stores known fplcass -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a class for representing known fpclasses used by
// computeKnownFPClass.
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/KnownFPClass.h"
#అన్తర్భావ "llvm/ADT/APFloat.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ "llvm/Support/KnownBits.h"

ఉపయోగించి నామాకాశ llvm;

తెలిసిన_FPClass::తెలిసిన_FPClass(స్థిరరాశి APFloat &C)
    : తెలిసిన_FPClasses_విలువ(C.classify()) {
  సమితి_Sign_బిట్టు(C.isNegative());
}

తెలిసిన_FPClass తెలిసిన_FPClass::apply_ఆదానం_Denormal_బహుళకం(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src,
                                                  Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన = తెలిసిన_Src;
  ఎంపిక (బహుళకం.ఆదానం) {
  స్థితి Denormal_బహుళకం::IEEE:
    తిరిగి తెలిసిన;
  స్థితి Denormal_బహుళకం::PreserveSign:
    ఒకవేళ (తెలిసిన_Src.get_తెలిసిన_FPClasses() & fcPosSubnormal)
      తెలిసిన.సమితి_తెలిసిన_FPClasses(తెలిసిన.get_తెలిసిన_FPClasses() | fcPosZero);
    ఒకవేళ (తెలిసిన_Src.get_తెలిసిన_FPClasses() & fcNegSubnormal)
      తెలిసిన.సమితి_తెలిసిన_FPClasses(తెలిసిన.get_తెలిసిన_FPClasses() | fcNegZero);
    తిరిగి తెలిసిన;
  స్థితి Denormal_బహుళకం::ధనాత్మక_Zero:
    ఒకవేళ (తెలిసిన_Src.get_తెలిసిన_FPClasses() & fcSubnormal) {
      తెలిసిన.సమితి_తెలిసిన_FPClasses(తెలిసిన.get_తెలిసిన_FPClasses() | fcPosZero);
      ఒకవేళ (తెలిసిన_Src.get_తెలిసిన_FPClasses() & fcNegSubnormal)
        తెలిసిన.సమితి_Sign_బిట్టు(std::nullopt);
    }
    తిరిగి తెలిసిన;
  అప్రమేయ:
    ఒకవేళ (తెలిసిన_Src.get_తెలిసిన_FPClasses() & fcSubnormal) {
      తెలిసిన.సమితి_తెలిసిన_FPClasses(తెలిసిన.get_తెలిసిన_FPClasses() | fcPosZero);
      ఒకవేళ (తెలిసిన_Src.get_తెలిసిన_FPClasses() & fcNegSubnormal)
        తెలిసిన.సమితి_Sign_బిట్టు(std::nullopt);
    }
    ఒకవేళ (తెలిసిన_Src.get_తెలిసిన_FPClasses() & fcNegSubnormal)
      తెలిసిన.సమితి_తెలిసిన_FPClasses(తెలిసిన.get_తెలిసిన_FPClasses() | fcNegZero);
    తిరిగి తెలిసిన;
  }

  llvm_unreachable("covered switch over denormal mode");
}

తెలిసిన_FPClass తెలిసిన_FPClass::apply_Output_Denormal_బహుళకం(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src,
                                                   Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన = తెలిసిన_Src;
  ఎంపిక (బహుళకం.Output) {
  స్థితి Denormal_బహుళకం::IEEE:
    తిరిగి తెలిసిన;
  స్థితి Denormal_బహుళకం::PreserveSign:
    ఒకవేళ (తెలిసిన_Src.get_తెలిసిన_FPClasses() & fcPosSubnormal)
      తెలిసిన.సమితి_తెలిసిన_FPClasses(తెలిసిన.get_తెలిసిన_FPClasses() | fcPosZero);
    ఒకవేళ (తెలిసిన_Src.get_తెలిసిన_FPClasses() & fcNegSubnormal)
      తెలిసిన.సమితి_తెలిసిన_FPClasses(తెలిసిన.get_తెలిసిన_FPClasses() | fcNegZero);
    తిరిగి తెలిసిన;
  స్థితి Denormal_బహుళకం::ధనాత్మక_Zero:
    ఒకవేళ (తెలిసిన_Src.get_తెలిసిన_FPClasses() & fcSubnormal) {
      తెలిసిన.సమితి_తెలిసిన_FPClasses(తెలిసిన.get_తెలిసిన_FPClasses() | fcPosZero);
      ఒకవేళ (తెలిసిన_Src.get_తెలిసిన_FPClasses() & fcNegSubnormal)
        తెలిసిన.సమితి_Sign_బిట్టు(std::nullopt);
    }
    తిరిగి తెలిసిన;
  అప్రమేయ:
    ఒకవేళ (తెలిసిన_Src.get_తెలిసిన_FPClasses() & fcSubnormal) {
      తెలిసిన.సమితి_తెలిసిన_FPClasses(తెలిసిన.get_తెలిసిన_FPClasses() | fcPosZero);
      ఒకవేళ (తెలిసిన_Src.get_తెలిసిన_FPClasses() & fcNegSubnormal)
        తెలిసిన.సమితి_Sign_బిట్టు(std::nullopt);
    }
    ఒకవేళ (తెలిసిన_Src.get_తెలిసిన_FPClasses() & fcNegSubnormal)
      తెలిసిన.సమితి_తెలిసిన_FPClasses(తెలిసిన.get_తెలిసిన_FPClasses() | fcNegZero);
    తిరిగి తెలిసిన;
  }

  llvm_unreachable("covered switch over denormal mode");
}

తార్కిక తెలిసిన_FPClass::is_తెలిసిన_Never_Logical_Zero(Denormal_బహుళకం బహుళకం) స్థిరరాశి {
  తిరిగి apply_ఆదానం_Denormal_బహుళకం(*ఇది, బహుళకం).is_తెలిసిన_Never_Zero();
}

తార్కిక తెలిసిన_FPClass::is_తెలిసిన_Never_Logical_Neg_Zero(Denormal_బహుళకం బహుళకం) స్థిరరాశి {
  తిరిగి apply_ఆదానం_Denormal_బహుళకం(*ఇది, బహుళకం).is_తెలిసిన_Never_Neg_Zero();
}

తార్కిక తెలిసిన_FPClass::is_తెలిసిన_Never_Logical_Pos_Zero(Denormal_బహుళకం బహుళకం) స్థిరరాశి {
  తిరిగి apply_ఆదానం_Denormal_బహుళకం(*ఇది, బహుళకం).is_తెలిసిన_Never_Pos_Zero();
}

తెలిసిన_FPClass తెలిసిన_FPClass::minMaxLike(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_LHS,
                                      స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_RHS,
                                      Min_Max_రకం రకం_3, Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన_LHS_2 = apply_ఆదానం_Denormal_బహుళకం(తెలిసిన_LHS, బహుళకం);
  తెలిసిన_FPClass తెలిసిన_RHS_2 = apply_ఆదానం_Denormal_బహుళకం(తెలిసిన_RHS, బహుళకం);

  తెలిసిన_FPClass తెలిసిన;

  తెలిసిన.propagateNonNaN(తెలిసిన_LHS_2, తెలిసిన_RHS_2);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  ఒకవేళ ((తెలిసిన_LHS_2.is_తెలిసిన_Never_Na_N() || తెలిసిన_RHS_2.is_తెలిసిన_Never_Na_N()) &&
      (రకం_3 == Min_Max_రకం::minnum || రకం_3 == Min_Max_రకం::maxnum ||
       రకం_3 == Min_Max_రకం::minimumnum || రకం_3 == Min_Max_రకం::maximumnum))
    తెలిసిన.తెలిసిన_Not(fcNan);

  ఎంపిక (రకం_3) {
  స్థితి Min_Max_రకం::maxnum: {
    ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never_Na_N())
      తెలిసిన.తెలిసిన_Not(orderedStrictlyLess(తెలిసిన_LHS_2.get_తెలిసిన_FPClasses(),
                                         /*OrderedZeroSign=*/అబద్ధం));
    ఒకవేళ (తెలిసిన_RHS_2.is_తెలిసిన_Never_Na_N())
      తెలిసిన.తెలిసిన_Not(orderedStrictlyLess(తెలిసిన_RHS_2.get_తెలిసిన_FPClasses(),
                                         /*OrderedZeroSign=*/అబద్ధం));
    ఆపు;
  }
  స్థితి Min_Max_రకం::minnum: {
    ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never_Na_N())
      తెలిసిన.తెలిసిన_Not(orderedStrictlyGreater(తెలిసిన_LHS_2.get_తెలిసిన_FPClasses(),
                                            /*OrderedZeroSign=*/అబద్ధం));
    ఒకవేళ (తెలిసిన_RHS_2.is_తెలిసిన_Never_Na_N())
      తెలిసిన.తెలిసిన_Not(orderedStrictlyGreater(తెలిసిన_RHS_2.get_తెలిసిన_FPClasses(),
                                            /*OrderedZeroSign=*/అబద్ధం));
    ఆపు;
  }
  స్థితి Min_Max_రకం::maximumnum: {
    ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never_Na_N())
      తెలిసిన.తెలిసిన_Not(orderedStrictlyLess(తెలిసిన_LHS_2.get_తెలిసిన_FPClasses(),
                                         /*OrderedZeroSign=*/నిజం));
    ఒకవేళ (తెలిసిన_RHS_2.is_తెలిసిన_Never_Na_N())
      తెలిసిన.తెలిసిన_Not(orderedStrictlyLess(తెలిసిన_RHS_2.get_తెలిసిన_FPClasses(),
                                         /*OrderedZeroSign=*/నిజం));
    ఆపు;
  }
  స్థితి Min_Max_రకం::minimumnum: {
    ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never_Na_N())
      తెలిసిన.తెలిసిన_Not(orderedStrictlyGreater(తెలిసిన_LHS_2.get_తెలిసిన_FPClasses(),
                                            /*OrderedZeroSign=*/నిజం));
    ఒకవేళ (తెలిసిన_RHS_2.is_తెలిసిన_Never_Na_N())
      తెలిసిన.తెలిసిన_Not(orderedStrictlyGreater(తెలిసిన_RHS_2.get_తెలిసిన_FPClasses(),
                                            /*OrderedZeroSign=*/నిజం));
    ఆపు;
  }
  స్థితి Min_Max_రకం::గరిష్ఠ: {
    తెలిసిన.తెలిసిన_Not(orderedStrictlyLess(తెలిసిన_LHS_2.get_తెలిసిన_FPClasses(),
                                       /*OrderedZeroSign=*/నిజం) |
                   orderedStrictlyLess(తెలిసిన_RHS_2.get_తెలిసిన_FPClasses(),
                                       /*OrderedZeroSign=*/నిజం));
    ఆపు;
  }
  స్థితి Min_Max_రకం::minimum: {
    తెలిసిన.తెలిసిన_Not(orderedStrictlyGreater(తెలిసిన_LHS_2.get_తెలిసిన_FPClasses(),
                                          /*OrderedZeroSign=*/నిజం) |
                   orderedStrictlyGreater(తెలిసిన_RHS_2.get_తెలిసిన_FPClasses(),
                                          /*OrderedZeroSign=*/నిజం));
    ఆపు;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      తెలిసిన_LHS_2.get_తెలిసిన_FPClasses() | తెలిసిన_RHS_2.get_తెలిసిన_FPClasses() | fcNan;
  తెలిసిన.తెలిసిన_Not(~PossibleClasses);

  తిరిగి apply_Output_Denormal_బహుళకం(తెలిసిన, బహుళకం);
}

తెలిసిన_FPClass తెలిసిన_FPClass::canonicalize(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src,
                                        Denormal_బహుళకం Denorm_బహుళకం) {
  తెలిసిన_FPClass తెలిసిన;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  తెలిసిన.సమితి_తెలిసిన_FPClasses(తెలిసిన_Src.get_తెలిసిన_FPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Na_N())
    తెలిసిన.తెలిసిన_Not(fcNan);
  లేకపోతే
    తెలిసిన.తెలిసిన_Not(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  ఒకవేళ (Denorm_బహుళకం == Denormal_బహుళకం::getIEEE()) {
    ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcPosZero))
      తెలిసిన.తెలిసిన_Not(fcPosZero);
    ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcNegZero))
      తెలిసిన.తెలిసిన_Not(fcNegZero);
    తిరిగి తెలిసిన;
  }

  ఒకవేళ (Denorm_బహుళకం.inputsAreZero() || Denorm_బహుళకం.outputsAreZero())
    తెలిసిన.తెలిసిన_Not(fcSubnormal);

  ఒకవేళ (Denorm_బహుళకం == Denormal_బహుళకం::getPreserveSign()) {
    ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcPosZero | fcPosSubnormal))
      తెలిసిన.తెలిసిన_Not(fcPosZero);
    ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcNegZero | fcNegSubnormal))
      తెలిసిన.తెలిసిన_Not(fcNegZero);
    తిరిగి తెలిసిన;
  }

  ఒకవేళ (Denorm_బహుళకం.ఆదానం == Denormal_బహుళకం::ధనాత్మక_Zero ||
      (Denorm_బహుళకం.Output == Denormal_బహుళకం::ధనాత్మక_Zero &&
       Denorm_బహుళకం.ఆదానం == Denormal_బహుళకం::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcNegZero))
      తెలిసిన.తెలిసిన_Not(fcNegZero);

    ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcPosZero | fcSubnormal))
      తెలిసిన.తెలిసిన_Not(fcPosZero);
  }

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::bitcast(స్థిరరాశి fltSemantics &FltSemantics,
                                   స్థిరరాశి తెలిసిన_బిట్లు &బిట్లు_2) {
  ప్రతిజ్ఞా(FltSemantics.కొలత_లోపల_బిట్లు == బిట్లు_2.get_బిట్టు_వెడల్పు() &&
         "Bitcast operand has incorrect bit width");
  తెలిసిన_FPClass తెలిసిన;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  ఒకవేళ (బిట్లు_2.hasConflict())
    తిరిగి తెలిసిన;

  // Return unknown for types we have not validated.
  స్వయంచాలక IsSupported = [](స్థిరరాశి fltSemantics &Semantics) {
    ఎంపిక (APFloat::Semantics_గమ్యం_Enum(Semantics)) {
    స్థితి APFloat_పునాది::S_IEEEhalf:
    స్థితి APFloat_పునాది::S_BFloat:
    స్థితి APFloat_పునాది::S_IEEEsingle:
    స్థితి APFloat_పునాది::S_IEEEdouble:
    స్థితి APFloat_పునాది::S_IEEEquad:
    స్థితి APFloat_పునాది::S_x87DoubleExtended:
      తిరిగి నిజం;
    అప్రమేయ:
      తిరిగి అబద్ధం;
    }
  };
  ఒకవేళ (!IsSupported(FltSemantics))
    తిరిగి తెలిసిన;

  // Transfer information from the sign bit.
  ఒకవేళ (బిట్లు_2.isNonNegative())
    తెలిసిన.sign_బిట్టు_Must_Be_Zero();
  లేకపోతే ఒకవేళ (బిట్లు_2.isNegative())
    తెలిసిన.sign_బిట్టు_Must_Be_One();

  ఎంపిక (APFloat_పునాది::Semantics_గమ్యం_Enum(FltSemantics)) {
  స్థితి APFloat_పునాది::S_IEEEhalf:
  స్థితి APFloat_పునాది::S_BFloat:
  స్థితి APFloat_పునాది::S_IEEEsingle:
  స్థితి APFloat_పునాది::S_IEEEdouble:
  స్థితి APFloat_పునాది::S_IEEEquad: {
    స్థిరరాశి గుర్తులేని Mantissa_బిట్లు = FltSemantics.precision - 1;
    స్థిరరాశి APInt Exponent_ముసుగు = APInt::get_బిట్లు_సమితి(
        FltSemantics.కొలత_లోపల_బిట్లు, Mantissa_బిట్లు, FltSemantics.కొలత_లోపల_బిట్లు - 1);
    స్థిరరాశి APInt Mantissa_ముసుగు =
        APInt::get_దిగువ_బిట్లు_సమితి(FltSemantics.కొలత_లోపల_బిట్లు, Mantissa_బిట్లు);

    స్థిరరాశి తార్కిక Exponent_తెలిసిన_All_Zeros =
        (బిట్లు_2.Zero & Exponent_ముసుగు) == Exponent_ముసుగు;
    స్థిరరాశి తార్కిక Exponent_తెలిసిన_All_Ones = (బిట్లు_2.One & Exponent_ముసుగు) == Exponent_ముసుగు;
    స్థిరరాశి తార్కిక Exponent_తెలిసిన_Not_All_Zeros = !(బిట్లు_2.One & Exponent_ముసుగు).isZero();
    స్థిరరాశి తార్కిక Exponent_తెలిసిన_Not_All_Ones = !(బిట్లు_2.Zero & Exponent_ముసుగు).isZero();

    స్థిరరాశి తార్కిక Mantissa_తెలిసిన_All_Zeros =
        (బిట్లు_2.Zero & Mantissa_ముసుగు) == Mantissa_ముసుగు;
    స్థిరరాశి తార్కిక Mantissa_తెలిసిన_Not_All_Zeros = !(బిట్లు_2.One & Mantissa_ముసుగు).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    ఒకవేళ (Exponent_తెలిసిన_Not_All_Zeros)
      తెలిసిన.తెలిసిన_Not(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    ఒకవేళ (Exponent_తెలిసిన_Not_All_Ones)
      తెలిసిన.తెలిసిన_Not(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    ఒకవేళ (Exponent_తెలిసిన_All_Zeros || Exponent_తెలిసిన_All_Ones)
      తెలిసిన.తెలిసిన_Not(fc_సాధారణ);

    // Zero and infinity require a mantissa with all zero bits.
    ఒకవేళ (Mantissa_తెలిసిన_Not_All_Zeros)
      తెలిసిన.తెలిసిన_Not(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    ఒకవేళ (Mantissa_తెలిసిన_All_Zeros)
      తెలిసిన.తెలిసిన_Not(fcSubnormal | fcNan);

    స్థిరరాశి తార్కిక Quiet_బిట్టు_తెలిసిన_సమితి = బిట్లు_2.One[Mantissa_బిట్లు - 1];
    స్థిరరాశి తార్కిక Quiet_బిట్టు_తెలిసిన_Clear = బిట్లు_2.Zero[Mantissa_బిట్లు - 1];

    ఒకవేళ (Quiet_బిట్టు_తెలిసిన_సమితి)
      తెలిసిన.తెలిసిన_Not(fcSNan);
    లేకపోతే ఒకవేళ (Quiet_బిట్టు_తెలిసిన_Clear)
      తెలిసిన.తెలిసిన_Not(fcQNan);

    ఆపు;
  }
  స్థితి APFloat_పునాది::S_x87DoubleExtended: {
    // Layout of the x87 floating point type:
    // [0:62] fraction
    // [63] explicit integer bit
    // [64:78] exponent
    // [79] sign bit
    // APFloat treats the x87 pseudo encodings as documented in
    // fltSemantics::hasExplicitIntegerBit.
    constexpr గుర్తులేని బిట్టు_వెడల్పు_2 = 80;
    constexpr గుర్తులేని Fraction_బిట్లు = 63;
    constexpr గుర్తులేని Integer_బిట్టు_సూచీ = Fraction_బిట్లు;
    స్థిరరాశి APInt Fraction_ముసుగు = APInt::get_దిగువ_బిట్లు_సమితి(బిట్టు_వెడల్పు_2, Fraction_బిట్లు);
    స్థిరరాశి APInt Exponent_ముసుగు =
        APInt::get_బిట్లు_సమితి(బిట్టు_వెడల్పు_2, Fraction_బిట్లు + 1, బిట్టు_వెడల్పు_2 - 1);

    స్థిరరాశి తార్కిక Exponent_తెలిసిన_All_Zeros =
        (బిట్లు_2.Zero & Exponent_ముసుగు) == Exponent_ముసుగు;
    స్థిరరాశి తార్కిక Exponent_తెలిసిన_All_Ones = (బిట్లు_2.One & Exponent_ముసుగు) == Exponent_ముసుగు;
    స్థిరరాశి తార్కిక Exponent_తెలిసిన_Not_All_Zeros = !(బిట్లు_2.One & Exponent_ముసుగు).isZero();
    స్థిరరాశి తార్కిక Exponent_తెలిసిన_Not_All_Ones = !(బిట్లు_2.Zero & Exponent_ముసుగు).isZero();
    స్థిరరాశి తార్కిక Fraction_తెలిసిన_All_Zeros =
        (బిట్లు_2.Zero & Fraction_ముసుగు) == Fraction_ముసుగు;
    స్థిరరాశి తార్కిక Fraction_తెలిసిన_Not_All_Zeros = !(బిట్లు_2.One & Fraction_ముసుగు).isZero();
    స్థిరరాశి తార్కిక Integer_బిట్టు_తెలిసిన_సమితి = బిట్లు_2.One[Integer_బిట్టు_సూచీ];
    స్థిరరాశి తార్కిక Integer_బిట్టు_తెలిసిన_Clear = బిట్లు_2.Zero[Integer_బిట్టు_సూచీ];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    ఒకవేళ (Exponent_తెలిసిన_Not_All_Zeros || Integer_బిట్టు_తెలిసిన_సమితి)
      తెలిసిన.తెలిసిన_Not(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    ఒకవేళ (Fraction_తెలిసిన_Not_All_Zeros)
      తెలిసిన.తెలిసిన_Not(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    ఒకవేళ (Exponent_తెలిసిన_Not_All_Ones || Integer_బిట్టు_తెలిసిన_Clear ||
        Fraction_తెలిసిన_Not_All_Zeros)
      తెలిసిన.తెలిసిన_Not(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    ఒకవేళ (Integer_బిట్టు_తెలిసిన_Clear || Exponent_తెలిసిన_All_Ones)
      తెలిసిన.తెలిసిన_Not(fc_సాధారణ);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    ఒకవేళ (Fraction_తెలిసిన_All_Zeros)
      తెలిసిన.తెలిసిన_Not(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    ఒకవేళ (Exponent_తెలిసిన_All_Zeros ||
        (Integer_బిట్టు_తెలిసిన_సమితి &&
         (Exponent_తెలిసిన_Not_All_Ones || Fraction_తెలిసిన_All_Zeros)))
      తెలిసిన.తెలిసిన_Not(fcNan);

    స్థిరరాశి తార్కిక Quiet_బిట్టు_తెలిసిన_సమితి = బిట్లు_2.One[Fraction_బిట్లు - 1];
    స్థిరరాశి తార్కిక Quiet_బిట్టు_తెలిసిన_Clear = బిట్లు_2.Zero[Fraction_బిట్లు - 1];

    ఒకవేళ (Quiet_బిట్టు_తెలిసిన_సమితి)
      తెలిసిన.తెలిసిన_Not(fcSNan);
    లేకపోతే ఒకవేళ (Quiet_బిట్టు_తెలిసిన_Clear)
      తెలిసిన.తెలిసిన_Not(fcQNan);

    ఆపు;
  }
  అప్రమేయ:
    llvm_unreachable("unhandled supported type");
  }

  తిరిగి తెలిసిన;
}

తెలిసిన_బిట్లు తెలిసిన_FPClass::గమ్యం_తెలిసిన_బిట్లు(స్థిరరాశి fltSemantics &FltSemantics) స్థిరరాశి {
  తెలిసిన_బిట్లు తెలిసిన(FltSemantics.కొలత_లోపల_బిట్లు);
  స్థిరరాశి FPClassTest FPClasses = get_తెలిసిన_FPClasses();

  // Return unknown if poison.
  ఒకవేళ (FPClasses == fcNone)
    తిరిగి తెలిసిన;

  // Return unknown for types we have not validated.
  స్వయంచాలక IsSupported = [](స్థిరరాశి fltSemantics &Semantics) {
    ఎంపిక (APFloat::Semantics_గమ్యం_Enum(Semantics)) {
    స్థితి APFloat_పునాది::S_IEEEhalf:
    స్థితి APFloat_పునాది::S_BFloat:
    స్థితి APFloat_పునాది::S_IEEEsingle:
    స్థితి APFloat_పునాది::S_IEEEdouble:
    స్థితి APFloat_పునాది::S_IEEEquad:
    స్థితి APFloat_పునాది::S_x87DoubleExtended:
      తిరిగి నిజం;
    అప్రమేయ:
      తిరిగి అబద్ధం;
    }
  };
  ఒకవేళ (!IsSupported(FltSemantics))
    తిరిగి తెలిసిన;

  ఎంపిక (APFloat::Semantics_గమ్యం_Enum(FltSemantics)) {
  స్థితి APFloat_పునాది::S_IEEEhalf:
  స్థితి APFloat_పునాది::S_BFloat:
  స్థితి APFloat_పునాది::S_IEEEsingle:
  స్థితి APFloat_పునాది::S_IEEEdouble:
  స్థితి APFloat_పునాది::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    ఒకవేళ (FPClasses & fc_సాధారణ)
      ఆపు;

    తెలిసిన.సమితి_All_Conflict();

    స్థిరరాశి గుర్తులేని బిట్టు_వెడల్పు_2 = FltSemantics.కొలత_లోపల_బిట్లు;
    స్థిరరాశి గుర్తులేని Mantissa_బిట్లు = FltSemantics.precision - 1;
    స్థిరరాశి గుర్తులేని Exponent_బిట్లు = బిట్టు_వెడల్పు_2 - Mantissa_బిట్లు - 1;

    APInt Mantissa_ముసుగు = APInt::get_దిగువ_బిట్లు_సమితి(బిట్టు_వెడల్పు_2, Mantissa_బిట్లు);
    APInt Exponent_ముసుగు =
        APInt::get_బిట్లు_సమితి(బిట్టు_వెడల్పు_2, Mantissa_బిట్లు, Mantissa_బిట్లు + Exponent_బిట్లు);

    స్థిరరాశి గుర్తులేని Quiet_బిట్టు_సూచీ = Mantissa_బిట్లు - 1;
    APInt Payload_ముసుగు = Mantissa_ముసుగు;
    Payload_ముసుగు.clear_బిట్టు(Quiet_బిట్టు_సూచీ);

    ఒకవేళ (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      తెలిసిన.Zero &= ~Exponent_ముసుగు;
      // No individual payload bit is known.
      తెలిసిన.Zero &= ~Payload_ముసుగు;
      తెలిసిన.One &= ~Payload_ముసుగు;

      ఒకవేళ (FPClasses & fcQNan)
        తెలిసిన.Zero.clear_బిట్టు(Quiet_బిట్టు_సూచీ);
      ఒకవేళ (FPClasses & fcSNan)
        తెలిసిన.One.clear_బిట్టు(Quiet_బిట్టు_సూచీ);
    }
    ఒకవేళ (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      తెలిసిన.Zero &= ~Exponent_ముసుగు;
      // Mantissa bits cannot be ones.
      తెలిసిన.One &= ~Mantissa_ముసుగు;
    }
    ఒకవేళ (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      తెలిసిన.One &= ~Exponent_ముసుగు;
      // Unknown mantissa.
      తెలిసిన.One &= ~Mantissa_ముసుగు;
      తెలిసిన.Zero &= ~Mantissa_ముసుగు;
    }
    ఒకవేళ (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      తెలిసిన.One &= ~Exponent_ముసుగు;
      // Mantissa cannot be ones.
      తెలిసిన.One &= ~Mantissa_ముసుగు;
    }

    ఆపు;
  }
  స్థితి APFloat_పునాది::S_x87DoubleExtended: {
    ఒకవేళ (!is_తెలిసిన_Never(fc_సాధారణ | fcSubnormal | fcNan))
      ఆపు;

    తెలిసిన.సమితి_All_Conflict();

    ఒకవేళ (FPClasses & fcInf)
      తెలిసిన = తెలిసిన.intersectWith(తెలిసిన_బిట్లు::makeConstant(
          APFloat::getInf(FltSemantics).bitcast_గమ్యం_APInt()));

    ఒకవేళ (FPClasses & fcZero)
      తెలిసిన = తెలిసిన.intersectWith(
          తెలిసిన_బిట్లు::makeConstant(APInt::getZero(FltSemantics.కొలత_లోపల_బిట్లు)));

    ఆపు;
  }
  అప్రమేయ:
    llvm_unreachable("unhandled supported semantics");
  }

  తెలిసిన.Zero.clear_Sign_బిట్టు();
  తెలిసిన.One.clear_Sign_బిట్టు();
  ఒకవేళ (std::optional<తార్కిక> Sign = get_Sign_బిట్టు()) {
    ఒకవేళ (*Sign)
      తెలిసిన.makeNegative();
    లేకపోతే
      తెలిసిన.makeNonNegative();
  }

  తిరిగి తెలిసిన;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
స్థిర తెలిసిన_FPClass fadd_impl(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_LHS_2,
                              స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_RHS_2, తార్కిక IsFMA) {
  తెలిసిన_FPClass తెలిసిన;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never_Na_N() && తెలిసిన_RHS_2.is_తెలిసిన_Never_Na_N() &&
      (తెలిసిన_LHS_2.is_తెలిసిన_Never(fcPosInf) || తెలిసిన_RHS_2.is_తెలిసిన_Never(fcNegInf)) &&
      (తెలిసిన_LHS_2.is_తెలిసిన_Never(fcNegInf) || తెలిసిన_RHS_2.is_తెలిసిన_Never(fcPosInf)))
    తెలిసిన.తెలిసిన_Not(fcNan);

  ఒకవేళ (తెలిసిన_LHS_2.cannotBeOrderedLessThanZero() &&
      తెలిసిన_RHS_2.cannotBeOrderedLessThanZero()) {
    తెలిసిన.తెలిసిన_Not(తెలిసిన_FPClass::Ordered_Less_Than_Zero_ముసుగు);

    // This can't underflow if one of the operands is known normal.
    ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never(fcZero | fcPosSubnormal) ||
        తెలిసిన_RHS_2.is_తెలిసిన_Never(fcZero | fcPosSubnormal))
      తెలిసిన.తెలిసిన_Not(fcZero | fcPosSubnormal);
  }

  ఒకవేళ (తెలిసిన_LHS_2.cannotBeOrderedGreaterThanZero() &&
      తెలిసిన_RHS_2.cannotBeOrderedGreaterThanZero()) {
    తెలిసిన.తెలిసిన_Not(తెలిసిన_FPClass::Ordered_Greater_Than_Zero_ముసుగు);

    // This can't underflow if one of the operands is known normal.
    ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never(fcZero | fcNegSubnormal) ||
        తెలిసిన_RHS_2.is_తెలిసిన_Never(fcZero | fcNegSubnormal))
      తెలిసిన.తెలిసిన_Not(fcZero | fcNegSubnormal);
  }

  ఒకవేళ (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never(fcNegZero) || తెలిసిన_RHS_2.is_తెలిసిన_Never(fcNegZero))
      తెలిసిన.తెలిసిన_Not(fcNegZero);

    తెలిసిన.propagateNonSNaN(తెలిసిన_LHS_2, తెలిసిన_RHS_2);
  }

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::fadd(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_LHS,
                                స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_RHS,
                                Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన_LHS_2 = apply_ఆదానం_Denormal_బహుళకం(తెలిసిన_LHS, బహుళకం);
  తెలిసిన_FPClass తెలిసిన_RHS_2 = apply_ఆదానం_Denormal_బహుళకం(తెలిసిన_RHS, బహుళకం);

  తెలిసిన_FPClass తెలిసిన = fadd_impl(తెలిసిన_LHS_2, తెలిసిన_RHS_2, /*IsFMA=*/అబద్ధం);

  తిరిగి apply_Output_Denormal_బహుళకం(తెలిసిన, బహుళకం);
}

తెలిసిన_FPClass తెలిసిన_FPClass::fadd_self(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src_2,
                                     Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన_Src = apply_ఆదానం_Denormal_బహుళకం(తెలిసిన_Src_2, బహుళకం);

  తెలిసిన_FPClass తెలిసిన = fadd_impl(తెలిసిన_Src, తెలిసిన_Src, /*IsFMA=*/అబద్ధం);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcPosSubnormal))
    తెలిసిన.తెలిసిన_Not(fcPosSubnormal);
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcNegSubnormal))
    తెలిసిన.తెలిసిన_Not(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcPosZero))
    తెలిసిన.తెలిసిన_Not(fcPosZero);

  తిరిగి apply_Output_Denormal_బహుళకం(తెలిసిన, బహుళకం);
}

తెలిసిన_FPClass తెలిసిన_FPClass::fsub(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_LHS,
                                స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_RHS,
                                Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన_LHS_2 = apply_ఆదానం_Denormal_బహుళకం(తెలిసిన_LHS, బహుళకం);
  తెలిసిన_FPClass తెలిసిన_RHS_2 = apply_ఆదానం_Denormal_బహుళకం(తెలిసిన_RHS, బహుళకం);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  తెలిసిన_RHS_2.fneg();

  తెలిసిన_FPClass తెలిసిన = fadd_impl(తెలిసిన_LHS_2, తెలిసిన_RHS_2, /*IsFMA=*/అబద్ధం);

  తిరిగి apply_Output_Denormal_బహుళకం(తెలిసిన, బహుళకం);
}

తెలిసిన_FPClass తెలిసిన_FPClass::fmul(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_LHS,
                                స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_RHS,
                                Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన_LHS_2 = apply_ఆదానం_Denormal_బహుళకం(తెలిసిన_LHS, బహుళకం);
  తెలిసిన_FPClass తెలిసిన_RHS_2 = apply_ఆదానం_Denormal_బహుళకం(తెలిసిన_RHS, బహుళకం);

  తెలిసిన_FPClass తెలిసిన;

  తెలిసిన.propagateNonSNaN(తెలిసిన_LHS_2, తెలిసిన_RHS_2);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  ఒకవేళ ((తెలిసిన_LHS_2.is_తెలిసిన_Never(fcNegative) &&
       తెలిసిన_RHS_2.is_తెలిసిన_Never(fcNegative)) ||
      (తెలిసిన_LHS_2.is_తెలిసిన_Never(fc_ధనాత్మక) && తెలిసిన_RHS_2.is_తెలిసిన_Never(fc_ధనాత్మక)))
    తెలిసిన.తెలిసిన_Not(fcNegative);
  ఒకవేళ ((తెలిసిన_LHS_2.is_తెలిసిన_Never(fc_ధనాత్మక) &&
       తెలిసిన_RHS_2.is_తెలిసిన_Never(fcNegative)) ||
      (తెలిసిన_LHS_2.is_తెలిసిన_Never(fcNegative) && తెలిసిన_RHS_2.is_తెలిసిన_Never(fc_ధనాత్మక)))
    తెలిసిన.తెలిసిన_Not(fc_ధనాత్మక);

  // Inf * Y => Inf or NaN
  ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Always(fcInf | fcNan) ||
      తెలిసిన_RHS_2.is_తెలిసిన_Always(fcInf | fcNan))
    తెలిసిన.తెలిసిన_Not(fc_సాధారణ | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  ఒకవేళ (తెలిసిన_RHS_2.is_తెలిసిన_Always(fcZero | fcNan) ||
      తెలిసిన_LHS_2.is_తెలిసిన_Always(fcZero | fcNan))
    తెలిసిన.తెలిసిన_Not(fc_సాధారణ | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never_Na_N() && తెలిసిన_RHS_2.is_తెలిసిన_Never_Na_N() &&
      (తెలిసిన_RHS_2.is_తెలిసిన_Never_Infinity() || తెలిసిన_LHS_2.is_తెలిసిన_Never_Zero()) &&
      (తెలిసిన_LHS_2.is_తెలిసిన_Never_Infinity() || తెలిసిన_RHS_2.is_తెలిసిన_Never_Zero()))
    తెలిసిన.తెలిసిన_Not(fcNan);

  తిరిగి apply_Output_Denormal_బహుళకం(తెలిసిన, బహుళకం);
}

// TODO: This generalizes to known ranges
తెలిసిన_FPClass తెలిసిన_FPClass::fmul(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_LHS_2,
                                స్థిరరాశి APFloat &CRHS, Denormal_బహుళకం బహుళకం) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  స్థిరరాశి fltSemantics &Flt = CRHS.getSemantics();
  గుర్తులేని Precision = APFloat::semanticsPrecision(Flt);
  స్థిరరాశి పూర్ణాంకం Mantissa_బిట్లు = Precision - 1;

  పూర్ణాంకం Min_తెలిసిన_Exponent = ilogb(CRHS);
  తార్కిక CannotBeSubnormal = (Min_తెలిసిన_Exponent >= Mantissa_బిట్లు);

  తెలిసిన_FPClass తెలిసిన = తెలిసిన_FPClass::fmul(తెలిసిన_LHS_2, తెలిసిన_FPClass(CRHS), బహుళకం);
  ఒకవేళ (CannotBeSubnormal)
    తెలిసిన.తెలిసిన_Not(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never(fcInf)) {
    ఒకవేళ (Min_తెలిసిన_Exponent < 0)
      తెలిసిన.తెలిసిన_Not(fcInf);
    లేకపోతే ఒకవేళ (Min_తెలిసిన_Exponent == 0 && CRHS.compare_Absolute_విలువ(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      తెలిసిన.తెలిసిన_Not(fcInf);
  }

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::fdiv(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_LHS,
                                స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_RHS,
                                Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన_LHS_2 = apply_ఆదానం_Denormal_బహుళకం(తెలిసిన_LHS, బహుళకం);
  తెలిసిన_FPClass తెలిసిన_RHS_2 = apply_ఆదానం_Denormal_బహుళకం(తెలిసిన_RHS, బహుళకం);

  తెలిసిన_FPClass తెలిసిన;

  తెలిసిన.propagateNonSNaN(తెలిసిన_LHS_2, తెలిసిన_RHS_2);

  // Only 0/0, Inf/Inf produce NaN.
  ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never_Na_N() && తెలిసిన_RHS_2.is_తెలిసిన_Never_Na_N() &&
      (తెలిసిన_LHS_2.is_తెలిసిన_Never_Infinity() || తెలిసిన_RHS_2.is_తెలిసిన_Never_Infinity()) &&
      (తెలిసిన_LHS_2.is_తెలిసిన_Never_Zero() || తెలిసిన_RHS_2.is_తెలిసిన_Never_Zero())) {
    తెలిసిన.తెలిసిన_Not(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  ఒకవేళ ((తెలిసిన_LHS_2.is_తెలిసిన_Never(fcNegative) &&
       తెలిసిన_RHS_2.is_తెలిసిన_Never(fcNegative)) ||
      (తెలిసిన_LHS_2.is_తెలిసిన_Never(fc_ధనాత్మక) && తెలిసిన_RHS_2.is_తెలిసిన_Never(fc_ధనాత్మక)))
    తెలిసిన.తెలిసిన_Not(fcNegative);
  ఒకవేళ ((తెలిసిన_LHS_2.is_తెలిసిన_Never(fc_ధనాత్మక) &&
       తెలిసిన_RHS_2.is_తెలిసిన_Never(fcNegative)) ||
      (తెలిసిన_LHS_2.is_తెలిసిన_Never(fcNegative) && తెలిసిన_RHS_2.is_తెలిసిన_Never(fc_ధనాత్మక)))
    తెలిసిన.తెలిసిన_Not(fc_ధనాత్మక);

  // Normal and subnormal results require two non-zero finite operands.
  ఒకవేళ ((తెలిసిన_LHS_2.is_తెలిసిన_Never(fc_Neg_సాధారణ | fcNegSubnormal) &&
       తెలిసిన_RHS_2.is_తెలిసిన_Never(fc_Neg_సాధారణ | fcNegSubnormal)) ||
      (తెలిసిన_LHS_2.is_తెలిసిన_Never(fc_Pos_సాధారణ | fcPosSubnormal) &&
       తెలిసిన_RHS_2.is_తెలిసిన_Never(fc_Pos_సాధారణ | fcPosSubnormal)))
    తెలిసిన.తెలిసిన_Not(fc_Neg_సాధారణ | fcNegSubnormal);
  ఒకవేళ ((తెలిసిన_LHS_2.is_తెలిసిన_Never(fc_Neg_సాధారణ | fcNegSubnormal) &&
       తెలిసిన_RHS_2.is_తెలిసిన_Never(fc_Pos_సాధారణ | fcPosSubnormal)) ||
      (తెలిసిన_LHS_2.is_తెలిసిన_Never(fc_Pos_సాధారణ | fcPosSubnormal) &&
       తెలిసిన_RHS_2.is_తెలిసిన_Never(fc_Neg_సాధారణ | fcNegSubnormal)))
    తెలిసిన.తెలిసిన_Not(fc_Pos_సాధారణ | fcPosSubnormal);

  // 0 / X => 0 or NaN
  ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Always(fcZero))
    తెలిసిన.తెలిసిన_Not(fcSubnormal | fc_సాధారణ | fcInf);

  // X / 0 => NaN or Inf
  ఒకవేళ (తెలిసిన_RHS_2.is_తెలిసిన_Always(fcZero))
    తెలిసిన.తెలిసిన_Not(fcFinite);

  తిరిగి apply_Output_Denormal_బహుళకం(తెలిసిన, బహుళకం);
}

తెలిసిన_FPClass తెలిసిన_FPClass::fdiv_self(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src,
                                     Denormal_బహుళకం బహుళకం) {
  // X / X is always exactly 1.0 or a NaN.
  తెలిసిన_FPClass తెలిసిన(fcNan | fc_Pos_సాధారణ);

  తెలిసిన.propagateNonSNaN(తెలిసిన_Src);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Inf_Or_Na_N() && తెలిసిన_Src.is_తెలిసిన_Never_Logical_Zero(బహుళకం))
    తెలిసిన.తెలిసిన_Not(fcNan);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::frem(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_LHS_2,
                                స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_RHS_2,
                                Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  తెలిసిన.తెలిసిన_Not(fcInf);

  తెలిసిన.propagateNonSNaN(తెలిసిన_LHS_2, తెలిసిన_RHS_2);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never_Inf_Or_Na_N() && తెలిసిన_RHS_2.is_తెలిసిన_Never_Na_N() &&
      తెలిసిన_RHS_2.is_తెలిసిన_Never_Logical_Zero(బహుళకం)) {
    తెలిసిన.తెలిసిన_Not(fcNan);
  }

  // The sign for frem is the same as the first operand.
  ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never(fc_Pos_సాధారణ | fcPosSubnormal))
    తెలిసిన.తెలిసిన_Not(fc_Pos_సాధారణ | fcPosSubnormal);
  ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never(fc_Neg_సాధారణ | fcNegSubnormal))
    తెలిసిన.తెలిసిన_Not(fc_Neg_సాధారణ | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never(fcNegFinite))
    తెలిసిన.తెలిసిన_Not(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never(fcPosFinite) &&
      తెలిసిన_LHS_2.is_తెలిసిన_Never_Logical_Pos_Zero(బహుళకం) &&
      (!బహుళకం.outputs_May_Be_ధనాత్మక_Zero() ||
       తెలిసిన_LHS_2.is_తెలిసిన_Never(fc_Neg_సాధారణ | fcNegSubnormal)))
    తెలిసిన.తెలిసిన_Not(fcPosZero);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::frem_self(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src,
                                     Denormal_బహుళకం బహుళకం) {
  // X % X is always exactly [+-]0.0 or a NaN.
  తెలిసిన_FPClass తెలిసిన(fcNan | fcZero);

  తెలిసిన.propagateNonSNaN(తెలిసిన_Src);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Inf_Or_Na_N() && తెలిసిన_Src.is_తెలిసిన_Never_Logical_Zero(బహుళకం))
    తెలిసిన.తెలిసిన_Not(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fc_Pos_సాధారణ | fcPosSubnormal))
    తెలిసిన.తెలిసిన_Not(fcPosZero);
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fc_Neg_సాధారణ | fcNegSubnormal))
    తెలిసిన.తెలిసిన_Not(fcNegZero);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::fma(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_LHS_2,
                               స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_RHS_2,
                               స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Addend,
                               Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass Mul = fmul(తెలిసిన_LHS_2, తెలిసిన_RHS_2, బహుళకం);

  // FMA differs from the base fmul + fadd handling only in the treatment of -0
  // results.
  //
  // If the multiply is a -0 due to rounding, the final -0 + 0 will be -0,
  // unlike for a separate fadd.
  తెలిసిన_FPClass తెలిసిన = fadd_impl(Mul, తెలిసిన_Addend, /*IsFMA=*/నిజం);

  // propagateNonSNaN for 3 arguments.
  ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never(fcSNan) && తెలిసిన_RHS_2.is_తెలిసిన_Never(fcSNan) &&
      తెలిసిన_Addend.is_తెలిసిన_Never(fcSNan))
    తెలిసిన.తెలిసిన_Not(fcSNan);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::fmuladd(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_LHS_2,
                                   స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_RHS_2,
                                   స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Addend,
                                   Denormal_బహుళకం బహుళకం) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  తెలిసిన_FPClass FMA = fma(తెలిసిన_LHS_2, తెలిసిన_RHS_2, తెలిసిన_Addend, బహుళకం);
  తెలిసిన_FPClass Mul_కూడు = fadd(fmul(తెలిసిన_LHS_2, తెలిసిన_RHS_2, బహుళకం), తెలిసిన_Addend, బహుళకం);
  తిరిగి FMA.intersectWith(Mul_కూడు);
}

తెలిసిన_FPClass తెలిసిన_FPClass::fma_వర్గం(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Squared,
                                      స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Addend,
                                      Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass Squared = వర్గం(తెలిసిన_Squared, బహుళకం);
  తెలిసిన_FPClass తెలిసిన = fadd_impl(Squared, తెలిసిన_Addend, /*IsFMA=*/నిజం);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  ఒకవేళ (తెలిసిన_Addend.is_తెలిసిన_Never(fcNegInf | fcNan) && Squared.is_తెలిసిన_Never(fcNan))
    తెలిసిన.తెలిసిన_Not(fcNan);

  తెలిసిన.propagateNonSNaN(తెలిసిన_Squared, తెలిసిన_Addend);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::fmuladd_వర్గం(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Squared,
                                          స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Addend,
                                          Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass FMA = fma_వర్గం(తెలిసిన_Squared, తెలిసిన_Addend, బహుళకం);
  తెలిసిన_FPClass Mul_కూడు = fadd(వర్గం(తెలిసిన_Squared, బహుళకం), తెలిసిన_Addend, బహుళకం);
  తిరిగి FMA.intersectWith(Mul_కూడు);
}

తెలిసిన_FPClass తెలిసిన_FPClass::ఘాతాంకం(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src) {
  తెలిసిన_FPClass తెలిసిన;
  తెలిసిన.తెలిసిన_Not(fcNegative);

  తెలిసిన.propagateNonNaN(తెలిసిన_Src);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fc_Neg_సాధారణ | fcNegInf))
    తెలిసిన.తెలిసిన_Not(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fc_Neg_సాధారణ))
    తెలిసిన.తెలిసిన_Not(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fc_Pos_సాధారణ | fcPosInf))
    తెలిసిన.తెలిసిన_Not(fcPosInf);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::సంవర్గమానం(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src, Denormal_బహుళకం బహుళకం,
                               తార్కిక IsMultiUnitFPType) {
  తెలిసిన_FPClass తెలిసిన;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  ఒకవేళ (!IsMultiUnitFPType)
    తెలిసిన.తెలిసిన_Not(fcNegZero | fcSubnormal);

  తెలిసిన.propagateNonSNaN(తెలిసిన_Src);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Pos_Infinity())
    తెలిసిన.తెలిసిన_Not(fcPosInf);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Na_N() && తెలిసిన_Src.cannotBeOrderedLessThanZero())
    తెలిసిన.తెలిసిన_Not(fcNan);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Logical_Zero(బహుళకం))
    తెలిసిన.తెలిసిన_Not(fcNegInf);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::వర్గమూలం(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src,
                                Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన;
  తెలిసిన.తెలిసిన_Not(fcPosSubnormal);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Pos_Infinity())
    తెలిసిన.తెలిసిన_Not(fcPosInf);

  తెలిసిన.propagateNonSNaN(తెలిసిన_Src);

  // Any negative value besides -0 returns a nan.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Na_N() && తెలిసిన_Src.cannotBeOrderedLessThanZero())
    తెలిసిన.తెలిసిన_Not(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  తెలిసిన.తెలిసిన_Not(fcNegInf | fcNegSubnormal | fc_Neg_సాధారణ);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Logical_Neg_Zero(బహుళకం))
    తెలిసిన.తెలిసిన_Not(fcNegZero);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::జ్యా(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src) {
  తెలిసిన_FPClass తెలిసిన;

  తెలిసిన.propagateNonSNaN(తెలిసిన_Src);

  // Return NaN on infinite inputs.
  తెలిసిన.తెలిసిన_Not(fcInf);
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Na_N() && తెలిసిన_Src.is_తెలిసిన_Never_Infinity())
    తెలిసిన.తెలిసిన_Not(fcNan);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::కోజ్యా(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src) {
  తిరిగి జ్యా(తెలిసిన_Src);
}

తెలిసిన_FPClass తెలిసిన_FPClass::tan(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src) {
  తెలిసిన_FPClass తెలిసిన;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  తెలిసిన.తెలిసిన_Not(fcInf);

  తెలిసిన.propagateNonSNaN(తెలిసిన_Src);

  // NaN propagates. tan(+-Inf) is NaN.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Na_N() && తెలిసిన_Src.is_తెలిసిన_Never_Infinity())
    తెలిసిన.తెలిసిన_Not(fcNan);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::sinh(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src) {
  తెలిసిన_FPClass తెలిసిన;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcNegative))
    తెలిసిన.తెలిసిన_Not(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcPosSubnormal | fc_Pos_సాధారణ | fcPosInf))
    తెలిసిన.తెలిసిన_Not(fcPosSubnormal | fc_Pos_సాధారణ | fcPosInf);

  తెలిసిన.propagateNonNaN(తెలిసిన_Src);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::cosh(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src) {
  తెలిసిన_FPClass తెలిసిన;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  తెలిసిన.తెలిసిన_Not(fcNegative | fcZero | fcSubnormal);

  తెలిసిన.propagateNonNaN(తెలిసిన_Src);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::tanh(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src) {
  తెలిసిన_FPClass తెలిసిన;

  // tanh is bounded to (-1, 1), never Inf.
  తెలిసిన.తెలిసిన_Not(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcNegSubnormal | fc_Neg_సాధారణ | fcNegInf)) {
    తెలిసిన.తెలిసిన_Not(fcNegSubnormal | fc_Neg_సాధారణ);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Neg_Zero())
      తెలిసిన.తెలిసిన_Not(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcPosSubnormal | fc_Pos_సాధారణ | fcPosInf))
    తెలిసిన.తెలిసిన_Not(fcPosSubnormal | fc_Pos_సాధారణ);

  తెలిసిన.propagateNonNaN(తెలిసిన_Src);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::asin(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src) {
  తెలిసిన_FPClass తెలిసిన;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  తెలిసిన.తెలిసిన_Not(fcInf);

  తెలిసిన.propagateNonSNaN(తెలిసిన_Src);

  // asin is sign-preserving for finite arguments.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcNegFinite))
    తెలిసిన.తెలిసిన_Not(fcNegFinite);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcPosSubnormal | fc_Pos_సాధారణ))
    తెలిసిన.తెలిసిన_Not(fcPosSubnormal | fc_Pos_సాధారణ);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::acos(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src) {
  తెలిసిన_FPClass తెలిసిన;

  // acos(x) is bounded to [0, pi] for -1 <= x <= 1, and is never negative,
  // infinite, or subnormal. The smallest non-zero value occurs when x is
  // close to 1.0, where acos(x) can be approximated by sqrt(2 * (1 - x)).
  // Since sqrt cannot produce a subnormal result, we can conclude that
  // acos(x) will also never produce a subnormal result.
  తెలిసిన.తెలిసిన_Not(fcNegative | fcInf | fcSubnormal);

  // acos(x) == +0.0 iff x == +1.0
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fc_Pos_సాధారణ))
    తెలిసిన.తెలిసిన_Not(fcZero);

  తెలిసిన.propagateNonSNaN(తెలిసిన_Src);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::atan(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src) {
  తెలిసిన_FPClass తెలిసిన;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  తెలిసిన.తెలిసిన_Not(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcNegSubnormal | fc_Neg_సాధారణ | fcNegInf))
    తెలిసిన.తెలిసిన_Not(fcNegSubnormal | fc_Neg_సాధారణ);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcPosSubnormal | fc_Pos_సాధారణ | fcPosInf))
    తెలిసిన.తెలిసిన_Not(fcPosSubnormal | fc_Pos_సాధారణ);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fc_Neg_సాధారణ | fcNegSubnormal | fcNegZero))
    తెలిసిన.తెలిసిన_Not(fcNegZero);

  తెలిసిన.propagateNonNaN(తెలిసిన_Src);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::atan2(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Y,
                                 స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_X,
                                 Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన_Y_2 = apply_ఆదానం_Denormal_బహుళకం(తెలిసిన_Y, బహుళకం);
  తెలిసిన_FPClass తెలిసిన_X_2 = apply_ఆదానం_Denormal_బహుళకం(తెలిసిన_X, బహుళకం);
  తెలిసిన_FPClass తెలిసిన;

  // Even though these deductions are correct, we are ignoring the following
  // potentially erroneous cases:
  //   * atan2(y, inf) is not subnormal
  //   * atan2(inf, x) is not zero or subnormal

  // atan2 result is in (-pi, pi], never Inf.
  తెలిసిన.తెలిసిన_Not(fcInf);

  తెలిసిన.propagateNonNaN(తెలిసిన_Y_2, తెలిసిన_X_2);

  // If x <= -0.0, then |atan2(y, x)| >= pi/2
  ఒకవేళ (తెలిసిన_X_2.is_తెలిసిన_Never(fc_ధనాత్మక))
    తెలిసిన.తెలిసిన_Not(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  ఒకవేళ (తెలిసిన_Y_2.is_తెలిసిన_Never(fcNegative))
    తెలిసిన.తెలిసిన_Not(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  ఒకవేళ (తెలిసిన_Y_2.is_తెలిసిన_Never(fc_ధనాత్మక))
    తెలిసిన.తెలిసిన_Not(fc_ధనాత్మక);

  తిరిగి apply_Output_Denormal_బహుళకం(తెలిసిన, బహుళకం);
}

తెలిసిన_FPClass తెలిసిన_FPClass::fpext(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src,
                                 స్థిరరాశి fltSemantics &DstTy,
                                 స్థిరరాశి fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  తెలిసిన_FPClass తెలిసిన = తెలిసిన_Src;

  // All subnormal inputs should be in the normal range in the result type.
  ఒకవేళ (APFloat::is_Representable_As_సాధారణ_లోపల(SrcTy, DstTy)) {
    ఒకవేళ (తెలిసిన.get_తెలిసిన_FPClasses() & fcPosSubnormal)
      తెలిసిన.సమితి_తెలిసిన_FPClasses(తెలిసిన.get_తెలిసిన_FPClasses() | fc_Pos_సాధారణ);
    ఒకవేళ (తెలిసిన.get_తెలిసిన_FPClasses() & fcNegSubnormal)
      తెలిసిన.సమితి_తెలిసిన_FPClasses(తెలిసిన.get_తెలిసిన_FPClasses() | fc_Neg_సాధారణ);
    తెలిసిన.తెలిసిన_Not(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  ఒకవేళ (!తెలిసిన.is_తెలిసిన_Never_Na_N())
    తెలిసిన.సమితి_Sign_బిట్టు(std::nullopt);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::fptrunc(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src) {
  తెలిసిన_FPClass తెలిసిన;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  ఒకవేళ (తెలిసిన_Src.cannotBeOrderedLessThanZero())
    తెలిసిన.తెలిసిన_Not(తెలిసిన_FPClass::Ordered_Less_Than_Zero_ముసుగు);

  తెలిసిన.propagateNonNaN(తెలిసిన_Src);

  // Infinity needs a range check.
  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::విడత_గమ్యం_Integral(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src,
                                           తార్కిక MayBeMultiUnitFPType,
                                           Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన;

  // Integer results cannot be subnormal.
  తెలిసిన.తెలిసిన_Not(fcSubnormal);

  తెలిసిన.propagateNonNaN(తెలిసిన_Src);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  ఒకవేళ (!MayBeMultiUnitFPType) {
    ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Pos_Infinity())
      తెలిసిన.తెలిసిన_Not(fcPosInf);
    ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Neg_Infinity())
      తెలిసిన.తెలిసిన_Not(fcNegInf);
  }

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fc_Pos_సాధారణ | fcPosSubnormal))
    తెలిసిన.తెలిసిన_Not(fc_Pos_సాధారణ);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fc_Neg_సాధారణ | fcNegSubnormal))
    తెలిసిన.తెలిసిన_Not(fc_Neg_సాధారణ);

  // Negative round ups towards zero produce negative zero.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcNegFinite))
    తెలిసిన.తెలిసిన_Not(fcNegZero);

  // Negative subnormals may flush to positive zero.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcPosFinite) &&
      (తెలిసిన_Src.is_తెలిసిన_Never(fcNegSubnormal) ||
       !బహుళకం.inputs_May_Be_ధనాత్మక_Zero()))
    తెలిసిన.తెలిసిన_Not(fcPosZero);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::trunc(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src,
                                 Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన = విడత_గమ్యం_Integral(తెలిసిన_Src,
                                       /*MayBeMultiUnitFPType=*/నిజం, బహుళకం);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Pos_Infinity())
    తెలిసిన.తెలిసిన_Not(fcPosInf);
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Neg_Infinity())
    తెలిసిన.తెలిసిన_Not(fcNegInf);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::floor(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src,
                                 తార్కిక IsMultiUnitFPType, Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన = విడత_గమ్యం_Integral(తెలిసిన_Src, IsMultiUnitFPType, బహుళకం);

  // Applicable for multi-unit floating point types.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Pos_Infinity())
    తెలిసిన.తెలిసిన_Not(fcPosInf);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::ceil(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src,
                                తార్కిక IsMultiUnitFPType, Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన = విడత_గమ్యం_Integral(తెలిసిన_Src, IsMultiUnitFPType, బహుళకం);

  // Applicable for multi-unit floating point types.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Neg_Infinity())
    తెలిసిన.తెలిసిన_Not(fcNegInf);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::frexp_mant(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src,
                                      Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన;

  తెలిసిన.తెలిసిన_Not(fcSubnormal);

  తెలిసిన.propagateNonNaN(తెలిసిన_Src);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Pos_Infinity())
    తెలిసిన.తెలిసిన_Not(fcPosInf);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Neg_Infinity())
    తెలిసిన.తెలిసిన_Not(fcNegInf);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Logical_Pos_Zero(బహుళకం))
    తెలిసిన.తెలిసిన_Not(fcPosZero);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Logical_Neg_Zero(బహుళకం))
    తెలిసిన.తెలిసిన_Not(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fc_Pos_సాధారణ | fcPosSubnormal))
    తెలిసిన.తెలిసిన_Not(fc_Pos_సాధారణ);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fc_Neg_సాధారణ | fcNegSubnormal))
    తెలిసిన.తెలిసిన_Not(fc_Neg_సాధారణ);

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::ldexp(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src_2,
                                 స్థిరరాశి APInt &ConstantRangeExpMin,
                                 స్థిరరాశి APInt &ConstantRangeExpMax,
                                 స్థిరరాశి fltSemantics &Flt, Denormal_బహుళకం బహుళకం) {
  తెలిసిన_FPClass తెలిసిన;

  తెలిసిన_FPClass తెలిసిన_Src = apply_ఆదానం_Denormal_బహుళకం(తెలిసిన_Src_2, బహుళకం);

  ఒకవేళ (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    తెలిసిన.సమితి_తెలిసిన_FPClasses(తెలిసిన_Src.get_తెలిసిన_FPClasses() | fcNan);
    తెలిసిన.propagateNonNaN(తెలిసిన_Src);
    తిరిగి apply_Output_Denormal_బహుళకం(తెలిసిన, బహుళకం);
  }

  తెలిసిన.propagateNonNaN(తెలిసిన_Src);

  // Sign is preserved, but underflows may produce zeroes.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcNegative))
    తెలిసిన.తెలిసిన_Not(fcNegative);
  లేకపోతే ఒకవేళ (తెలిసిన_Src.cannotBeOrderedLessThanZero())
    తెలిసిన.తెలిసిన_Not(Ordered_Less_Than_Zero_ముసుగు);

  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fc_ధనాత్మక))
    తెలిసిన.తెలిసిన_Not(fc_ధనాత్మక);
  లేకపోతే ఒకవేళ (తెలిసిన_Src.cannotBeOrderedGreaterThanZero())
    తెలిసిన.తెలిసిన_Not(Ordered_Greater_Than_Zero_ముసుగు);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  ఒకవేళ (&Flt != &APFloat::PPCDoubleDouble()) {
    గుర్తులేని Precision = APFloat::semanticsPrecision(Flt);
    స్థిరరాశి పూర్ణాంకం Mantissa_బిట్లు = Precision - 1;
    ఒకవేళ (ConstantRangeExpMin.sge(Mantissa_బిట్లు))
      తెలిసిన.తెలిసిన_Not(fcSubnormal);
  }

  ఒకవేళ (ConstantRangeExpMax.is_Non_ధనాత్మక()) {
    // If we know the power is <= 0, can't introduce inf
    ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Pos_Infinity())
      తెలిసిన.తెలిసిన_Not(fcPosInf);
    ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Neg_Infinity())
      తెలిసిన.తెలిసిన_Not(fcNegInf);
  } లేకపోతే ఒకవేళ (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Pos_Subnormal())
      తెలిసిన.తెలిసిన_Not(fcPosSubnormal);
    ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Neg_Subnormal())
      తెలిసిన.తెలిసిన_Not(fcNegSubnormal);
    ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Pos_Zero())
      తెలిసిన.తెలిసిన_Not(fcPosZero);
    ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never_Neg_Zero())
      తెలిసిన.తెలిసిన_Not(fcNegZero);
  }

  తిరిగి apply_Output_Denormal_బహుళకం(తెలిసిన, బహుళకం);
}

తెలిసిన_FPClass తెలిసిన_FPClass::ldexp(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src,
                                 స్థిరరాశి తెలిసిన_బిట్లు &Exp_బిట్లు,
                                 స్థిరరాశి fltSemantics &Flt, Denormal_బహుళకం బహుళకం) {
  తిరిగి ldexp(తెలిసిన_Src, Exp_బిట్లు.get_Signed_Min_విలువ(),
               Exp_బిట్లు.get_Signed_Max_విలువ(), Flt, బహుళకం);
}

తెలిసిన_FPClass తెలిసిన_FPClass::ఘాతం(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_LHS_2,
                               స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_RHS_2) {
  తెలిసిన_FPClass తెలిసిన;

  తెలిసిన.propagateNonSNaN(తెలిసిన_LHS_2, తెలిసిన_RHS_2);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never(fcNan | fc_Neg_సాధారణ | fcNegSubnormal) &&
      తెలిసిన_RHS_2.is_తెలిసిన_Never_Na_N())
    తెలిసిన.తెలిసిన_Not(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never(fcNegative)) {
    తెలిసిన.తెలిసిన_Not(fcNegative);
  } లేకపోతే ఒకవేళ (తెలిసిన_LHS_2.is_తెలిసిన_Never(fc_Neg_సాధారణ | fcNegSubnormal)) {
    తెలిసిన.తెలిసిన_Not(fc_Neg_సాధారణ | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    ఒకవేళ ((తెలిసిన_LHS_2.is_తెలిసిన_Never(fcNegZero) ||
         తెలిసిన_RHS_2.is_తెలిసిన_Never(fc_Pos_సాధారణ)) &&
        (తెలిసిన_LHS_2.is_తెలిసిన_Never(fcNegInf) || తెలిసిన_RHS_2.is_తెలిసిన_Never(fc_Neg_సాధారణ)))
      తెలిసిన.తెలిసిన_Not(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    ఒకవేళ ((తెలిసిన_LHS_2.is_తెలిసిన_Never(fcNegZero) ||
         తెలిసిన_RHS_2.is_తెలిసిన_Never(fc_Neg_సాధారణ)) &&
        (తెలిసిన_LHS_2.is_తెలిసిన_Never(fcNegInf) || తెలిసిన_RHS_2.is_తెలిసిన_Never(fc_Pos_సాధారణ)))
      తెలిసిన.తెలిసిన_Not(fcNegInf);
  }

  తిరిగి తెలిసిన;
}

తెలిసిన_FPClass తెలిసిన_FPClass::powi(స్థిరరాశి తెలిసిన_FPClass &తెలిసిన_Src,
                                స్థిరరాశి తెలిసిన_బిట్లు &Exponent_తెలిసిన_బిట్లు) {
  తెలిసిన_FPClass తెలిసిన;
  తెలిసిన.propagateNonNaN(తెలిసిన_Src);

  ఒకవేళ (Exponent_తెలిసిన_బిట్లు.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcSNan)) {
      తెలిసిన.తెలిసిన_Not(~fc_Pos_సాధారణ);
      తిరిగి తెలిసిన;
    }

    తెలిసిన.తెలిసిన_Not(~(fc_Pos_సాధారణ | fcNan));
    తిరిగి తెలిసిన;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fcNegative) || Exponent_తెలిసిన_బిట్లు.isEven()) {
    తెలిసిన.తెలిసిన_Not(fcNegative);
  } లేకపోతే ఒకవేళ (తెలిసిన_Src.is_తెలిసిన_Never(fc_Neg_సాధారణ | fcNegSubnormal)) {
    తెలిసిన.తెలిసిన_Not(fc_Neg_సాధారణ | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    స్థిరరాశి తార్కిక Is_తెలిసిన_Never_Odd_ధనాత్మక = Exponent_తెలిసిన_బిట్లు.isNegative();
    స్థిరరాశి తార్కిక Is_తెలిసిన_Never_Odd_Negative = Exponent_తెలిసిన_బిట్లు.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    ఒకవేళ ((తెలిసిన_Src.is_తెలిసిన_Never(fcNegZero) || Is_తెలిసిన_Never_Odd_ధనాత్మక) &&
        (తెలిసిన_Src.is_తెలిసిన_Never(fcNegInf) || Is_తెలిసిన_Never_Odd_Negative))
      తెలిసిన.తెలిసిన_Not(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    ఒకవేళ ((తెలిసిన_Src.is_తెలిసిన_Never(fcNegZero) || Is_తెలిసిన_Never_Odd_Negative) &&
        (తెలిసిన_Src.is_తెలిసిన_Never(fcNegInf) || Is_తెలిసిన_Never_Odd_ధనాత్మక))
      తెలిసిన.తెలిసిన_Not(fcNegInf);
  }

  // powi(x, exp) --> inf
  // when:
  //   * powi(inf, exp), exp > 0
  //   * powi(+/-0, exp), exp < 0
  //   * powi(finite, exp), |exp| > 1
  //   * powi(subnormal, -1)
  // TODO:
  //   1. This simple all or nothing approach. We can do better
  //      and cover sign/parity and exp > 1 vs exp < -1 separately.
  //   2. powi(0/nan, exp), exp > 0 can be refinable
  //      to fcNan | fcZero | fcPosNormal.
  {
    APInt MinExp = Exponent_తెలిసిన_బిట్లు.get_Signed_Min_విలువ();
    APInt MaxExp = Exponent_తెలిసిన_బిట్లు.get_Signed_Max_విలువ();

    // powi(inf, exp), exp > 0
    తార్కిక MayInfSrc =
        !తెలిసిన_Src.is_తెలిసిన_Never(fcInf) && MaxExp.is_Strictly_ధనాత్మక();

    // powi(+/-0, exp), exp < 0
    తార్కిక MayDivByZero = !తెలిసిన_Src.is_తెలిసిన_Never(fcZero) && MinExp.isNegative();

    // powi(finite, exp), |exp| > 1
    తార్కిక MayFinite = !తెలిసిన_Src.is_తెలిసిన_Never(fc_సాధారణ | fcSubnormal);
    తార్కిక MayAbsExpGT1 = MinExp.slt(-1) || MaxExp.sgt(1);
    తార్కిక May_Finite_అధికప్రవాహం = MayFinite && MayAbsExpGT1;

    // powi(subnormal, -1)
    తార్కిక MayBeNegOne = Exponent_తెలిసిన_బిట్లు.Zero.isZero();
    తార్కిక MaySubnormInv = !తెలిసిన_Src.is_తెలిసిన_Never(fcSubnormal) && MayBeNegOne;

    ఒకవేళ (!MayInfSrc && !MayDivByZero && !May_Finite_అధికప్రవాహం && !MaySubnormInv)
      తెలిసిన.తెలిసిన_Not(fcInf);
  }

  తిరిగి తెలిసిన;
}
