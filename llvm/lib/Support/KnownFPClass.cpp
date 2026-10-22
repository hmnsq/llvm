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

#ಅನ್ತರ್ಭಾವ "llvm/Support/KnownFPClass.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/APFloat.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/KnownBits.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ತಿಳಿದ_FPClass::ತಿಳಿದ_FPClass(ಸ್ಥಿರಾಂಕ APFloat &C)
    : ತಿಳಿದ_FPClasses_ಮೌಲ್ಯ(C.classify()) {
  ಗಣ_Sign_ಬಿಟ್(C.isNegative());
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src,
                                                  Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ = ತಿಳಿದ_Src;
  ಆಯ್ಕೆ (ಬಹುಲಕ.ಒಳಹರಿವು) {
  ಸ್ಥಿತಿ Denormal_ಬಹುಲಕ::IEEE:
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  ಸ್ಥಿತಿ Denormal_ಬಹುಲಕ::PreserveSign:
    ಆದರೆ (ತಿಳಿದ_Src.get_ತಿಳಿದ_FPClasses() & fcPosSubnormal)
      ತಿಳಿದ.ಗಣ_ತಿಳಿದ_FPClasses(ತಿಳಿದ.get_ತಿಳಿದ_FPClasses() | fcPosZero);
    ಆದರೆ (ತಿಳಿದ_Src.get_ತಿಳಿದ_FPClasses() & fcNegSubnormal)
      ತಿಳಿದ.ಗಣ_ತಿಳಿದ_FPClasses(ತಿಳಿದ.get_ತಿಳಿದ_FPClasses() | fcNegZero);
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  ಸ್ಥಿತಿ Denormal_ಬಹುಲಕ::ಧನಾತ್ಮಕ_Zero:
    ಆದರೆ (ತಿಳಿದ_Src.get_ತಿಳಿದ_FPClasses() & fcSubnormal) {
      ತಿಳಿದ.ಗಣ_ತಿಳಿದ_FPClasses(ತಿಳಿದ.get_ತಿಳಿದ_FPClasses() | fcPosZero);
      ಆದರೆ (ತಿಳಿದ_Src.get_ತಿಳಿದ_FPClasses() & fcNegSubnormal)
        ತಿಳಿದ.ಗಣ_Sign_ಬಿಟ್(std::nullopt);
    }
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಆದರೆ (ತಿಳಿದ_Src.get_ತಿಳಿದ_FPClasses() & fcSubnormal) {
      ತಿಳಿದ.ಗಣ_ತಿಳಿದ_FPClasses(ತಿಳಿದ.get_ತಿಳಿದ_FPClasses() | fcPosZero);
      ಆದರೆ (ತಿಳಿದ_Src.get_ತಿಳಿದ_FPClasses() & fcNegSubnormal)
        ತಿಳಿದ.ಗಣ_Sign_ಬಿಟ್(std::nullopt);
    }
    ಆದರೆ (ತಿಳಿದ_Src.get_ತಿಳಿದ_FPClasses() & fcNegSubnormal)
      ತಿಳಿದ.ಗಣ_ತಿಳಿದ_FPClasses(ತಿಳಿದ.get_ತಿಳಿದ_FPClasses() | fcNegZero);
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  }

  llvm_unreachable("covered switch over denormal mode");
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::apply_Output_Denormal_ಬಹುಲಕ(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src,
                                                   Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ = ತಿಳಿದ_Src;
  ಆಯ್ಕೆ (ಬಹುಲಕ.Output) {
  ಸ್ಥಿತಿ Denormal_ಬಹುಲಕ::IEEE:
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  ಸ್ಥಿತಿ Denormal_ಬಹುಲಕ::PreserveSign:
    ಆದರೆ (ತಿಳಿದ_Src.get_ತಿಳಿದ_FPClasses() & fcPosSubnormal)
      ತಿಳಿದ.ಗಣ_ತಿಳಿದ_FPClasses(ತಿಳಿದ.get_ತಿಳಿದ_FPClasses() | fcPosZero);
    ಆದರೆ (ತಿಳಿದ_Src.get_ತಿಳಿದ_FPClasses() & fcNegSubnormal)
      ತಿಳಿದ.ಗಣ_ತಿಳಿದ_FPClasses(ತಿಳಿದ.get_ತಿಳಿದ_FPClasses() | fcNegZero);
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  ಸ್ಥಿತಿ Denormal_ಬಹುಲಕ::ಧನಾತ್ಮಕ_Zero:
    ಆದರೆ (ತಿಳಿದ_Src.get_ತಿಳಿದ_FPClasses() & fcSubnormal) {
      ತಿಳಿದ.ಗಣ_ತಿಳಿದ_FPClasses(ತಿಳಿದ.get_ತಿಳಿದ_FPClasses() | fcPosZero);
      ಆದರೆ (ತಿಳಿದ_Src.get_ತಿಳಿದ_FPClasses() & fcNegSubnormal)
        ತಿಳಿದ.ಗಣ_Sign_ಬಿಟ್(std::nullopt);
    }
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಆದರೆ (ತಿಳಿದ_Src.get_ತಿಳಿದ_FPClasses() & fcSubnormal) {
      ತಿಳಿದ.ಗಣ_ತಿಳಿದ_FPClasses(ತಿಳಿದ.get_ತಿಳಿದ_FPClasses() | fcPosZero);
      ಆದರೆ (ತಿಳಿದ_Src.get_ತಿಳಿದ_FPClasses() & fcNegSubnormal)
        ತಿಳಿದ.ಗಣ_Sign_ಬಿಟ್(std::nullopt);
    }
    ಆದರೆ (ತಿಳಿದ_Src.get_ತಿಳಿದ_FPClasses() & fcNegSubnormal)
      ತಿಳಿದ.ಗಣ_ತಿಳಿದ_FPClasses(ತಿಳಿದ.get_ತಿಳಿದ_FPClasses() | fcNegZero);
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  }

  llvm_unreachable("covered switch over denormal mode");
}

ತಾರ್ಕಿಕ ತಿಳಿದ_FPClass::is_ತಿಳಿದ_Never_Logical_Zero(Denormal_ಬಹುಲಕ ಬಹುಲಕ) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(*ಇದು, ಬಹುಲಕ).is_ತಿಳಿದ_Never_Zero();
}

ತಾರ್ಕಿಕ ತಿಳಿದ_FPClass::is_ತಿಳಿದ_Never_Logical_Neg_Zero(Denormal_ಬಹುಲಕ ಬಹುಲಕ) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(*ಇದು, ಬಹುಲಕ).is_ತಿಳಿದ_Never_Neg_Zero();
}

ತಾರ್ಕಿಕ ತಿಳಿದ_FPClass::is_ತಿಳಿದ_Never_Logical_Pos_Zero(Denormal_ಬಹುಲಕ ಬಹುಲಕ) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(*ಇದು, ಬಹುಲಕ).is_ತಿಳಿದ_Never_Pos_Zero();
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::minMaxLike(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_LHS,
                                      ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_RHS,
                                      Min_Max_ಬಗೆ ಬಗೆ_2, Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ_LHS_2 = apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(ತಿಳಿದ_LHS, ಬಹುಲಕ);
  ತಿಳಿದ_FPClass ತಿಳಿದ_RHS_2 = apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(ತಿಳಿದ_RHS, ಬಹುಲಕ);

  ತಿಳಿದ_FPClass ತಿಳಿದ;

  ತಿಳಿದ.propagateNonNaN(ತಿಳಿದ_LHS_2, ತಿಳಿದ_RHS_2);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  ಆದರೆ ((ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never_Na_N() || ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never_Na_N()) &&
      (ಬಗೆ_2 == Min_Max_ಬಗೆ::minnum || ಬಗೆ_2 == Min_Max_ಬಗೆ::maxnum ||
       ಬಗೆ_2 == Min_Max_ಬಗೆ::minimumnum || ಬಗೆ_2 == Min_Max_ಬಗೆ::maximumnum))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNan);

  ಆಯ್ಕೆ (ಬಗೆ_2) {
  ಸ್ಥಿತಿ Min_Max_ಬಗೆ::maxnum: {
    ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never_Na_N())
      ತಿಳಿದ.ತಿಳಿದ_Not(orderedStrictlyLess(ತಿಳಿದ_LHS_2.get_ತಿಳಿದ_FPClasses(),
                                         /*OrderedZeroSign=*/ಸುಳ್ಳು));
    ಆದರೆ (ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never_Na_N())
      ತಿಳಿದ.ತಿಳಿದ_Not(orderedStrictlyLess(ತಿಳಿದ_RHS_2.get_ತಿಳಿದ_FPClasses(),
                                         /*OrderedZeroSign=*/ಸುಳ್ಳು));
    ನಿಲ್ಲಿಸು;
  }
  ಸ್ಥಿತಿ Min_Max_ಬಗೆ::minnum: {
    ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never_Na_N())
      ತಿಳಿದ.ತಿಳಿದ_Not(orderedStrictlyGreater(ತಿಳಿದ_LHS_2.get_ತಿಳಿದ_FPClasses(),
                                            /*OrderedZeroSign=*/ಸುಳ್ಳು));
    ಆದರೆ (ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never_Na_N())
      ತಿಳಿದ.ತಿಳಿದ_Not(orderedStrictlyGreater(ತಿಳಿದ_RHS_2.get_ತಿಳಿದ_FPClasses(),
                                            /*OrderedZeroSign=*/ಸುಳ್ಳು));
    ನಿಲ್ಲಿಸು;
  }
  ಸ್ಥಿತಿ Min_Max_ಬಗೆ::maximumnum: {
    ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never_Na_N())
      ತಿಳಿದ.ತಿಳಿದ_Not(orderedStrictlyLess(ತಿಳಿದ_LHS_2.get_ತಿಳಿದ_FPClasses(),
                                         /*OrderedZeroSign=*/ನಿಜ));
    ಆದರೆ (ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never_Na_N())
      ತಿಳಿದ.ತಿಳಿದ_Not(orderedStrictlyLess(ತಿಳಿದ_RHS_2.get_ತಿಳಿದ_FPClasses(),
                                         /*OrderedZeroSign=*/ನಿಜ));
    ನಿಲ್ಲಿಸು;
  }
  ಸ್ಥಿತಿ Min_Max_ಬಗೆ::minimumnum: {
    ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never_Na_N())
      ತಿಳಿದ.ತಿಳಿದ_Not(orderedStrictlyGreater(ತಿಳಿದ_LHS_2.get_ತಿಳಿದ_FPClasses(),
                                            /*OrderedZeroSign=*/ನಿಜ));
    ಆದರೆ (ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never_Na_N())
      ತಿಳಿದ.ತಿಳಿದ_Not(orderedStrictlyGreater(ತಿಳಿದ_RHS_2.get_ತಿಳಿದ_FPClasses(),
                                            /*OrderedZeroSign=*/ನಿಜ));
    ನಿಲ್ಲಿಸು;
  }
  ಸ್ಥಿತಿ Min_Max_ಬಗೆ::ಗರಿಷ್ಠ: {
    ತಿಳಿದ.ತಿಳಿದ_Not(orderedStrictlyLess(ತಿಳಿದ_LHS_2.get_ತಿಳಿದ_FPClasses(),
                                       /*OrderedZeroSign=*/ನಿಜ) |
                   orderedStrictlyLess(ತಿಳಿದ_RHS_2.get_ತಿಳಿದ_FPClasses(),
                                       /*OrderedZeroSign=*/ನಿಜ));
    ನಿಲ್ಲಿಸು;
  }
  ಸ್ಥಿತಿ Min_Max_ಬಗೆ::minimum: {
    ತಿಳಿದ.ತಿಳಿದ_Not(orderedStrictlyGreater(ತಿಳಿದ_LHS_2.get_ತಿಳಿದ_FPClasses(),
                                          /*OrderedZeroSign=*/ನಿಜ) |
                   orderedStrictlyGreater(ತಿಳಿದ_RHS_2.get_ತಿಳಿದ_FPClasses(),
                                          /*OrderedZeroSign=*/ನಿಜ));
    ನಿಲ್ಲಿಸು;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      ತಿಳಿದ_LHS_2.get_ತಿಳಿದ_FPClasses() | ತಿಳಿದ_RHS_2.get_ತಿಳಿದ_FPClasses() | fcNan;
  ತಿಳಿದ.ತಿಳಿದ_Not(~PossibleClasses);

  ಹಿಂತಿರುಗಿ apply_Output_Denormal_ಬಹುಲಕ(ತಿಳಿದ, ಬಹುಲಕ);
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::canonicalize(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src,
                                        Denormal_ಬಹುಲಕ Denorm_ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  ತಿಳಿದ.ಗಣ_ತಿಳಿದ_FPClasses(ತಿಳಿದ_Src.get_ತಿಳಿದ_FPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Na_N())
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNan);
  ಇಲ್ಲದಿದ್ದರೆ
    ತಿಳಿದ.ತಿಳಿದ_Not(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  ಆದರೆ (Denorm_ಬಹುಲಕ == Denormal_ಬಹುಲಕ::getIEEE()) {
    ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcPosZero))
      ತಿಳಿದ.ತಿಳಿದ_Not(fcPosZero);
    ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcNegZero))
      ತಿಳಿದ.ತಿಳಿದ_Not(fcNegZero);
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  }

  ಆದರೆ (Denorm_ಬಹುಲಕ.inputsAreZero() || Denorm_ಬಹುಲಕ.outputsAreZero())
    ತಿಳಿದ.ತಿಳಿದ_Not(fcSubnormal);

  ಆದರೆ (Denorm_ಬಹುಲಕ == Denormal_ಬಹುಲಕ::getPreserveSign()) {
    ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcPosZero | fcPosSubnormal))
      ತಿಳಿದ.ತಿಳಿದ_Not(fcPosZero);
    ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcNegZero | fcNegSubnormal))
      ತಿಳಿದ.ತಿಳಿದ_Not(fcNegZero);
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  }

  ಆದರೆ (Denorm_ಬಹುಲಕ.ಒಳಹರಿವು == Denormal_ಬಹುಲಕ::ಧನಾತ್ಮಕ_Zero ||
      (Denorm_ಬಹುಲಕ.Output == Denormal_ಬಹುಲಕ::ಧನಾತ್ಮಕ_Zero &&
       Denorm_ಬಹುಲಕ.ಒಳಹರಿವು == Denormal_ಬಹುಲಕ::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcNegZero))
      ತಿಳಿದ.ತಿಳಿದ_Not(fcNegZero);

    ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcPosZero | fcSubnormal))
      ತಿಳಿದ.ತಿಳಿದ_Not(fcPosZero);
  }

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::bitcast(ಸ್ಥಿರಾಂಕ fltSemantics &FltSemantics,
                                   ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &ಬಿಟ್ಟುಗಳು_2) {
  ಪ್ರತಿಜ್ಞಾ(FltSemantics.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು == ಬಿಟ್ಟುಗಳು_2.get_ಬಿಟ್_ಅಗಲ() &&
         "Bitcast operand has incorrect bit width");
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  ಆದರೆ (ಬಿಟ್ಟುಗಳು_2.hasConflict())
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;

  // Return unknown for types we have not validated.
  ಸ್ವಯಂಚಾಲಿತ IsSupported = [](ಸ್ಥಿರಾಂಕ fltSemantics &Semantics) {
    ಆಯ್ಕೆ (APFloat::Semantics_ಗಮ್ಯ_Enum(Semantics)) {
    ಸ್ಥಿತಿ APFloat_ತಳ::S_IEEEhalf:
    ಸ್ಥಿತಿ APFloat_ತಳ::S_BFloat:
    ಸ್ಥಿತಿ APFloat_ತಳ::S_IEEEsingle:
    ಸ್ಥಿತಿ APFloat_ತಳ::S_IEEEdouble:
    ಸ್ಥಿತಿ APFloat_ತಳ::S_IEEEquad:
    ಸ್ಥಿತಿ APFloat_ತಳ::S_x87DoubleExtended:
      ಹಿಂತಿರುಗಿ ನಿಜ;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }
  };
  ಆದರೆ (!IsSupported(FltSemantics))
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;

  // Transfer information from the sign bit.
  ಆದರೆ (ಬಿಟ್ಟುಗಳು_2.isNonNegative())
    ತಿಳಿದ.sign_ಬಿಟ್_Must_Be_Zero();
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಬಿಟ್ಟುಗಳು_2.isNegative())
    ತಿಳಿದ.sign_ಬಿಟ್_Must_Be_One();

  ಆಯ್ಕೆ (APFloat_ತಳ::Semantics_ಗಮ್ಯ_Enum(FltSemantics)) {
  ಸ್ಥಿತಿ APFloat_ತಳ::S_IEEEhalf:
  ಸ್ಥಿತಿ APFloat_ತಳ::S_BFloat:
  ಸ್ಥಿತಿ APFloat_ತಳ::S_IEEEsingle:
  ಸ್ಥಿತಿ APFloat_ತಳ::S_IEEEdouble:
  ಸ್ಥಿತಿ APFloat_ತಳ::S_IEEEquad: {
    ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ Mantissa_ಬಿಟ್ಟುಗಳು = FltSemantics.precision - 1;
    ಸ್ಥಿರಾಂಕ APInt Exponent_ಮುಖವಾಡ = APInt::get_ಬಿಟ್ಟುಗಳು_ಗಣ(
        FltSemantics.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು, Mantissa_ಬಿಟ್ಟುಗಳು, FltSemantics.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು - 1);
    ಸ್ಥಿರಾಂಕ APInt Mantissa_ಮುಖವಾಡ =
        APInt::get_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು_ಗಣ(FltSemantics.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು, Mantissa_ಬಿಟ್ಟುಗಳು);

    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Exponent_ತಿಳಿದ_All_Zeros =
        (ಬಿಟ್ಟುಗಳು_2.Zero & Exponent_ಮುಖವಾಡ) == Exponent_ಮುಖವಾಡ;
    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Exponent_ತಿಳಿದ_All_Ones = (ಬಿಟ್ಟುಗಳು_2.One & Exponent_ಮುಖವಾಡ) == Exponent_ಮುಖವಾಡ;
    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Exponent_ತಿಳಿದ_Not_All_Zeros = !(ಬಿಟ್ಟುಗಳು_2.One & Exponent_ಮುಖವಾಡ).isZero();
    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Exponent_ತಿಳಿದ_Not_All_Ones = !(ಬಿಟ್ಟುಗಳು_2.Zero & Exponent_ಮುಖವಾಡ).isZero();

    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Mantissa_ತಿಳಿದ_All_Zeros =
        (ಬಿಟ್ಟುಗಳು_2.Zero & Mantissa_ಮುಖವಾಡ) == Mantissa_ಮುಖವಾಡ;
    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Mantissa_ತಿಳಿದ_Not_All_Zeros = !(ಬಿಟ್ಟುಗಳು_2.One & Mantissa_ಮುಖವಾಡ).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    ಆದರೆ (Exponent_ತಿಳಿದ_Not_All_Zeros)
      ತಿಳಿದ.ತಿಳಿದ_Not(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    ಆದರೆ (Exponent_ತಿಳಿದ_Not_All_Ones)
      ತಿಳಿದ.ತಿಳಿದ_Not(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    ಆದರೆ (Exponent_ತಿಳಿದ_All_Zeros || Exponent_ತಿಳಿದ_All_Ones)
      ತಿಳಿದ.ತಿಳಿದ_Not(fc_ಸಾಮಾನ್ಯ);

    // Zero and infinity require a mantissa with all zero bits.
    ಆದರೆ (Mantissa_ತಿಳಿದ_Not_All_Zeros)
      ತಿಳಿದ.ತಿಳಿದ_Not(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    ಆದರೆ (Mantissa_ತಿಳಿದ_All_Zeros)
      ತಿಳಿದ.ತಿಳಿದ_Not(fcSubnormal | fcNan);

    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Quiet_ಬಿಟ್_ತಿಳಿದ_ಗಣ = ಬಿಟ್ಟುಗಳು_2.One[Mantissa_ಬಿಟ್ಟುಗಳು - 1];
    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Quiet_ಬಿಟ್_ತಿಳಿದ_Clear = ಬಿಟ್ಟುಗಳು_2.Zero[Mantissa_ಬಿಟ್ಟುಗಳು - 1];

    ಆದರೆ (Quiet_ಬಿಟ್_ತಿಳಿದ_ಗಣ)
      ತಿಳಿದ.ತಿಳಿದ_Not(fcSNan);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Quiet_ಬಿಟ್_ತಿಳಿದ_Clear)
      ತಿಳಿದ.ತಿಳಿದ_Not(fcQNan);

    ನಿಲ್ಲಿಸು;
  }
  ಸ್ಥಿತಿ APFloat_ತಳ::S_x87DoubleExtended: {
    // Layout of the x87 floating point type:
    // [0:62] fraction
    // [63] explicit integer bit
    // [64:78] exponent
    // [79] sign bit
    // APFloat treats the x87 pseudo encodings as documented in
    // fltSemantics::hasExplicitIntegerBit.
    constexpr ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = 80;
    constexpr ಚಿಹ್ನೆರಹಿತ Fraction_ಬಿಟ್ಟುಗಳು = 63;
    constexpr ಚಿಹ್ನೆರಹಿತ Integer_ಬಿಟ್_ಸೂಚ್ಯಂಕ = Fraction_ಬಿಟ್ಟುಗಳು;
    ಸ್ಥಿರಾಂಕ APInt Fraction_ಮುಖವಾಡ = APInt::get_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು_ಗಣ(ಬಿಟ್_ಅಗಲ_2, Fraction_ಬಿಟ್ಟುಗಳು);
    ಸ್ಥಿರಾಂಕ APInt Exponent_ಮುಖವಾಡ =
        APInt::get_ಬಿಟ್ಟುಗಳು_ಗಣ(ಬಿಟ್_ಅಗಲ_2, Fraction_ಬಿಟ್ಟುಗಳು + 1, ಬಿಟ್_ಅಗಲ_2 - 1);

    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Exponent_ತಿಳಿದ_All_Zeros =
        (ಬಿಟ್ಟುಗಳು_2.Zero & Exponent_ಮುಖವಾಡ) == Exponent_ಮುಖವಾಡ;
    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Exponent_ತಿಳಿದ_All_Ones = (ಬಿಟ್ಟುಗಳು_2.One & Exponent_ಮುಖವಾಡ) == Exponent_ಮುಖವಾಡ;
    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Exponent_ತಿಳಿದ_Not_All_Zeros = !(ಬಿಟ್ಟುಗಳು_2.One & Exponent_ಮುಖವಾಡ).isZero();
    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Exponent_ತಿಳಿದ_Not_All_Ones = !(ಬಿಟ್ಟುಗಳು_2.Zero & Exponent_ಮುಖವಾಡ).isZero();
    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Fraction_ತಿಳಿದ_All_Zeros =
        (ಬಿಟ್ಟುಗಳು_2.Zero & Fraction_ಮುಖವಾಡ) == Fraction_ಮುಖವಾಡ;
    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Fraction_ತಿಳಿದ_Not_All_Zeros = !(ಬಿಟ್ಟುಗಳು_2.One & Fraction_ಮುಖವಾಡ).isZero();
    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Integer_ಬಿಟ್_ತಿಳಿದ_ಗಣ = ಬಿಟ್ಟುಗಳು_2.One[Integer_ಬಿಟ್_ಸೂಚ್ಯಂಕ];
    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Integer_ಬಿಟ್_ತಿಳಿದ_Clear = ಬಿಟ್ಟುಗಳು_2.Zero[Integer_ಬಿಟ್_ಸೂಚ್ಯಂಕ];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    ಆದರೆ (Exponent_ತಿಳಿದ_Not_All_Zeros || Integer_ಬಿಟ್_ತಿಳಿದ_ಗಣ)
      ತಿಳಿದ.ತಿಳಿದ_Not(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    ಆದರೆ (Fraction_ತಿಳಿದ_Not_All_Zeros)
      ತಿಳಿದ.ತಿಳಿದ_Not(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    ಆದರೆ (Exponent_ತಿಳಿದ_Not_All_Ones || Integer_ಬಿಟ್_ತಿಳಿದ_Clear ||
        Fraction_ತಿಳಿದ_Not_All_Zeros)
      ತಿಳಿದ.ತಿಳಿದ_Not(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    ಆದರೆ (Integer_ಬಿಟ್_ತಿಳಿದ_Clear || Exponent_ತಿಳಿದ_All_Ones)
      ತಿಳಿದ.ತಿಳಿದ_Not(fc_ಸಾಮಾನ್ಯ);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    ಆದರೆ (Fraction_ತಿಳಿದ_All_Zeros)
      ತಿಳಿದ.ತಿಳಿದ_Not(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    ಆದರೆ (Exponent_ತಿಳಿದ_All_Zeros ||
        (Integer_ಬಿಟ್_ತಿಳಿದ_ಗಣ &&
         (Exponent_ತಿಳಿದ_Not_All_Ones || Fraction_ತಿಳಿದ_All_Zeros)))
      ತಿಳಿದ.ತಿಳಿದ_Not(fcNan);

    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Quiet_ಬಿಟ್_ತಿಳಿದ_ಗಣ = ಬಿಟ್ಟುಗಳು_2.One[Fraction_ಬಿಟ್ಟುಗಳು - 1];
    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Quiet_ಬಿಟ್_ತಿಳಿದ_Clear = ಬಿಟ್ಟುಗಳು_2.Zero[Fraction_ಬಿಟ್ಟುಗಳು - 1];

    ಆದರೆ (Quiet_ಬಿಟ್_ತಿಳಿದ_ಗಣ)
      ತಿಳಿದ.ತಿಳಿದ_Not(fcSNan);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Quiet_ಬಿಟ್_ತಿಳಿದ_Clear)
      ತಿಳಿದ.ತಿಳಿದ_Not(fcQNan);

    ನಿಲ್ಲಿಸು;
  }
  ಪೂರ್ವನಿಯೋಜಿತ:
    llvm_unreachable("unhandled supported type");
  }

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ_FPClass::ಗಮ್ಯ_ತಿಳಿದ_ಬಿಟ್ಟುಗಳು(ಸ್ಥಿರಾಂಕ fltSemantics &FltSemantics) ಸ್ಥಿರಾಂಕ {
  ತಿಳಿದ_ಬಿಟ್ಟುಗಳು ತಿಳಿದ(FltSemantics.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು);
  ಸ್ಥಿರಾಂಕ FPClassTest FPClasses = get_ತಿಳಿದ_FPClasses();

  // Return unknown if poison.
  ಆದರೆ (FPClasses == fcNone)
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;

  // Return unknown for types we have not validated.
  ಸ್ವಯಂಚಾಲಿತ IsSupported = [](ಸ್ಥಿರಾಂಕ fltSemantics &Semantics) {
    ಆಯ್ಕೆ (APFloat::Semantics_ಗಮ್ಯ_Enum(Semantics)) {
    ಸ್ಥಿತಿ APFloat_ತಳ::S_IEEEhalf:
    ಸ್ಥಿತಿ APFloat_ತಳ::S_BFloat:
    ಸ್ಥಿತಿ APFloat_ತಳ::S_IEEEsingle:
    ಸ್ಥಿತಿ APFloat_ತಳ::S_IEEEdouble:
    ಸ್ಥಿತಿ APFloat_ತಳ::S_IEEEquad:
    ಸ್ಥಿತಿ APFloat_ತಳ::S_x87DoubleExtended:
      ಹಿಂತಿರುಗಿ ನಿಜ;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }
  };
  ಆದರೆ (!IsSupported(FltSemantics))
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;

  ಆಯ್ಕೆ (APFloat::Semantics_ಗಮ್ಯ_Enum(FltSemantics)) {
  ಸ್ಥಿತಿ APFloat_ತಳ::S_IEEEhalf:
  ಸ್ಥಿತಿ APFloat_ತಳ::S_BFloat:
  ಸ್ಥಿತಿ APFloat_ತಳ::S_IEEEsingle:
  ಸ್ಥಿತಿ APFloat_ತಳ::S_IEEEdouble:
  ಸ್ಥಿತಿ APFloat_ತಳ::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    ಆದರೆ (FPClasses & fc_ಸಾಮಾನ್ಯ)
      ನಿಲ್ಲಿಸು;

    ತಿಳಿದ.ಗಣ_All_Conflict();

    ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2 = FltSemantics.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು;
    ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ Mantissa_ಬಿಟ್ಟುಗಳು = FltSemantics.precision - 1;
    ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ Exponent_ಬಿಟ್ಟುಗಳು = ಬಿಟ್_ಅಗಲ_2 - Mantissa_ಬಿಟ್ಟುಗಳು - 1;

    APInt Mantissa_ಮುಖವಾಡ = APInt::get_ಕೆಳಗಿನ_ಬಿಟ್ಟುಗಳು_ಗಣ(ಬಿಟ್_ಅಗಲ_2, Mantissa_ಬಿಟ್ಟುಗಳು);
    APInt Exponent_ಮುಖವಾಡ =
        APInt::get_ಬಿಟ್ಟುಗಳು_ಗಣ(ಬಿಟ್_ಅಗಲ_2, Mantissa_ಬಿಟ್ಟುಗಳು, Mantissa_ಬಿಟ್ಟುಗಳು + Exponent_ಬಿಟ್ಟುಗಳು);

    ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ Quiet_ಬಿಟ್_ಸೂಚ್ಯಂಕ = Mantissa_ಬಿಟ್ಟುಗಳು - 1;
    APInt Payload_ಮುಖವಾಡ = Mantissa_ಮುಖವಾಡ;
    Payload_ಮುಖವಾಡ.clear_ಬಿಟ್(Quiet_ಬಿಟ್_ಸೂಚ್ಯಂಕ);

    ಆದರೆ (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      ತಿಳಿದ.Zero &= ~Exponent_ಮುಖವಾಡ;
      // No individual payload bit is known.
      ತಿಳಿದ.Zero &= ~Payload_ಮುಖವಾಡ;
      ತಿಳಿದ.One &= ~Payload_ಮುಖವಾಡ;

      ಆದರೆ (FPClasses & fcQNan)
        ತಿಳಿದ.Zero.clear_ಬಿಟ್(Quiet_ಬಿಟ್_ಸೂಚ್ಯಂಕ);
      ಆದರೆ (FPClasses & fcSNan)
        ತಿಳಿದ.One.clear_ಬಿಟ್(Quiet_ಬಿಟ್_ಸೂಚ್ಯಂಕ);
    }
    ಆದರೆ (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      ತಿಳಿದ.Zero &= ~Exponent_ಮುಖವಾಡ;
      // Mantissa bits cannot be ones.
      ತಿಳಿದ.One &= ~Mantissa_ಮುಖವಾಡ;
    }
    ಆದರೆ (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      ತಿಳಿದ.One &= ~Exponent_ಮುಖವಾಡ;
      // Unknown mantissa.
      ತಿಳಿದ.One &= ~Mantissa_ಮುಖವಾಡ;
      ತಿಳಿದ.Zero &= ~Mantissa_ಮುಖವಾಡ;
    }
    ಆದರೆ (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      ತಿಳಿದ.One &= ~Exponent_ಮುಖವಾಡ;
      // Mantissa cannot be ones.
      ತಿಳಿದ.One &= ~Mantissa_ಮುಖವಾಡ;
    }

    ನಿಲ್ಲಿಸು;
  }
  ಸ್ಥಿತಿ APFloat_ತಳ::S_x87DoubleExtended: {
    ಆದರೆ (!is_ತಿಳಿದ_Never(fc_ಸಾಮಾನ್ಯ | fcSubnormal | fcNan))
      ನಿಲ್ಲಿಸು;

    ತಿಳಿದ.ಗಣ_All_Conflict();

    ಆದರೆ (FPClasses & fcInf)
      ತಿಳಿದ = ತಿಳಿದ.intersectWith(ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::makeConstant(
          APFloat::getInf(FltSemantics).bitcast_ಗಮ್ಯ_APInt()));

    ಆದರೆ (FPClasses & fcZero)
      ತಿಳಿದ = ತಿಳಿದ.intersectWith(
          ತಿಳಿದ_ಬಿಟ್ಟುಗಳು::makeConstant(APInt::getZero(FltSemantics.ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು)));

    ನಿಲ್ಲಿಸು;
  }
  ಪೂರ್ವನಿಯೋಜಿತ:
    llvm_unreachable("unhandled supported semantics");
  }

  ತಿಳಿದ.Zero.clear_Sign_ಬಿಟ್();
  ತಿಳಿದ.One.clear_Sign_ಬಿಟ್();
  ಆದರೆ (std::optional<ತಾರ್ಕಿಕ> Sign = get_Sign_ಬಿಟ್()) {
    ಆದರೆ (*Sign)
      ತಿಳಿದ.makeNegative();
    ಇಲ್ಲದಿದ್ದರೆ
      ತಿಳಿದ.makeNonNegative();
  }

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
ಸ್ಥಿರ ತಿಳಿದ_FPClass fadd_impl(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_LHS_2,
                              ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_RHS_2, ತಾರ್ಕಿಕ IsFMA) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never_Na_N() && ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never_Na_N() &&
      (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcPosInf) || ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fcNegInf)) &&
      (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcNegInf) || ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fcPosInf)))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNan);

  ಆದರೆ (ತಿಳಿದ_LHS_2.cannotBeOrderedLessThanZero() &&
      ತಿಳಿದ_RHS_2.cannotBeOrderedLessThanZero()) {
    ತಿಳಿದ.ತಿಳಿದ_Not(ತಿಳಿದ_FPClass::Ordered_Less_Than_Zero_ಮುಖವಾಡ);

    // This can't underflow if one of the operands is known normal.
    ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcZero | fcPosSubnormal) ||
        ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fcZero | fcPosSubnormal))
      ತಿಳಿದ.ತಿಳಿದ_Not(fcZero | fcPosSubnormal);
  }

  ಆದರೆ (ತಿಳಿದ_LHS_2.cannotBeOrderedGreaterThanZero() &&
      ತಿಳಿದ_RHS_2.cannotBeOrderedGreaterThanZero()) {
    ತಿಳಿದ.ತಿಳಿದ_Not(ತಿಳಿದ_FPClass::Ordered_Greater_Than_Zero_ಮುಖವಾಡ);

    // This can't underflow if one of the operands is known normal.
    ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcZero | fcNegSubnormal) ||
        ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fcZero | fcNegSubnormal))
      ತಿಳಿದ.ತಿಳಿದ_Not(fcZero | fcNegSubnormal);
  }

  ಆದರೆ (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcNegZero) || ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fcNegZero))
      ತಿಳಿದ.ತಿಳಿದ_Not(fcNegZero);

    ತಿಳಿದ.propagateNonSNaN(ತಿಳಿದ_LHS_2, ತಿಳಿದ_RHS_2);
  }

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::fadd(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_LHS,
                                ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_RHS,
                                Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ_LHS_2 = apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(ತಿಳಿದ_LHS, ಬಹುಲಕ);
  ತಿಳಿದ_FPClass ತಿಳಿದ_RHS_2 = apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(ತಿಳಿದ_RHS, ಬಹುಲಕ);

  ತಿಳಿದ_FPClass ತಿಳಿದ = fadd_impl(ತಿಳಿದ_LHS_2, ತಿಳಿದ_RHS_2, /*IsFMA=*/ಸುಳ್ಳು);

  ಹಿಂತಿರುಗಿ apply_Output_Denormal_ಬಹುಲಕ(ತಿಳಿದ, ಬಹುಲಕ);
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::fadd_self(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src_2,
                                     Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ_Src = apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(ತಿಳಿದ_Src_2, ಬಹುಲಕ);

  ತಿಳಿದ_FPClass ತಿಳಿದ = fadd_impl(ತಿಳಿದ_Src, ತಿಳಿದ_Src, /*IsFMA=*/ಸುಳ್ಳು);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcPosSubnormal))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosSubnormal);
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcNegSubnormal))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcPosZero))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosZero);

  ಹಿಂತಿರುಗಿ apply_Output_Denormal_ಬಹುಲಕ(ತಿಳಿದ, ಬಹುಲಕ);
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::fsub(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_LHS,
                                ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_RHS,
                                Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ_LHS_2 = apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(ತಿಳಿದ_LHS, ಬಹುಲಕ);
  ತಿಳಿದ_FPClass ತಿಳಿದ_RHS_2 = apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(ತಿಳಿದ_RHS, ಬಹುಲಕ);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  ತಿಳಿದ_RHS_2.fneg();

  ತಿಳಿದ_FPClass ತಿಳಿದ = fadd_impl(ತಿಳಿದ_LHS_2, ತಿಳಿದ_RHS_2, /*IsFMA=*/ಸುಳ್ಳು);

  ಹಿಂತಿರುಗಿ apply_Output_Denormal_ಬಹುಲಕ(ತಿಳಿದ, ಬಹುಲಕ);
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::fmul(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_LHS,
                                ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_RHS,
                                Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ_LHS_2 = apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(ತಿಳಿದ_LHS, ಬಹುಲಕ);
  ತಿಳಿದ_FPClass ತಿಳಿದ_RHS_2 = apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(ತಿಳಿದ_RHS, ಬಹುಲಕ);

  ತಿಳಿದ_FPClass ತಿಳಿದ;

  ತಿಳಿದ.propagateNonSNaN(ತಿಳಿದ_LHS_2, ತಿಳಿದ_RHS_2);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  ಆದರೆ ((ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcNegative) &&
       ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fcNegative)) ||
      (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fc_ಧನಾತ್ಮಕ) && ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fc_ಧನಾತ್ಮಕ)))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegative);
  ಆದರೆ ((ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fc_ಧನಾತ್ಮಕ) &&
       ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fcNegative)) ||
      (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcNegative) && ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fc_ಧನಾತ್ಮಕ)))
    ತಿಳಿದ.ತಿಳಿದ_Not(fc_ಧನಾತ್ಮಕ);

  // Inf * Y => Inf or NaN
  ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Always(fcInf | fcNan) ||
      ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Always(fcInf | fcNan))
    ತಿಳಿದ.ತಿಳಿದ_Not(fc_ಸಾಮಾನ್ಯ | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  ಆದರೆ (ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Always(fcZero | fcNan) ||
      ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Always(fcZero | fcNan))
    ತಿಳಿದ.ತಿಳಿದ_Not(fc_ಸಾಮಾನ್ಯ | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never_Na_N() && ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never_Na_N() &&
      (ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never_Infinity() || ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never_Zero()) &&
      (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never_Infinity() || ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never_Zero()))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNan);

  ಹಿಂತಿರುಗಿ apply_Output_Denormal_ಬಹುಲಕ(ತಿಳಿದ, ಬಹುಲಕ);
}

// TODO: This generalizes to known ranges
ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::fmul(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_LHS_2,
                                ಸ್ಥಿರಾಂಕ APFloat &CRHS, Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  ಸ್ಥಿರಾಂಕ fltSemantics &Flt = CRHS.getSemantics();
  ಚಿಹ್ನೆರಹಿತ Precision = APFloat::semanticsPrecision(Flt);
  ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ Mantissa_ಬಿಟ್ಟುಗಳು = Precision - 1;

  ಪೂರ್ಣಾಂಕ Min_ತಿಳಿದ_Exponent = ilogb(CRHS);
  ತಾರ್ಕಿಕ CannotBeSubnormal = (Min_ತಿಳಿದ_Exponent >= Mantissa_ಬಿಟ್ಟುಗಳು);

  ತಿಳಿದ_FPClass ತಿಳಿದ = ತಿಳಿದ_FPClass::fmul(ತಿಳಿದ_LHS_2, ತಿಳಿದ_FPClass(CRHS), ಬಹುಲಕ);
  ಆದರೆ (CannotBeSubnormal)
    ತಿಳಿದ.ತಿಳಿದ_Not(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcInf)) {
    ಆದರೆ (Min_ತಿಳಿದ_Exponent < 0)
      ತಿಳಿದ.ತಿಳಿದ_Not(fcInf);
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (Min_ತಿಳಿದ_Exponent == 0 && CRHS.compare_Absolute_ಮೌಲ್ಯ(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      ತಿಳಿದ.ತಿಳಿದ_Not(fcInf);
  }

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::fdiv(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_LHS,
                                ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_RHS,
                                Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ_LHS_2 = apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(ತಿಳಿದ_LHS, ಬಹುಲಕ);
  ತಿಳಿದ_FPClass ತಿಳಿದ_RHS_2 = apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(ತಿಳಿದ_RHS, ಬಹುಲಕ);

  ತಿಳಿದ_FPClass ತಿಳಿದ;

  ತಿಳಿದ.propagateNonSNaN(ತಿಳಿದ_LHS_2, ತಿಳಿದ_RHS_2);

  // Only 0/0, Inf/Inf produce NaN.
  ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never_Na_N() && ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never_Na_N() &&
      (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never_Infinity() || ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never_Infinity()) &&
      (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never_Zero() || ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never_Zero())) {
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  ಆದರೆ ((ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcNegative) &&
       ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fcNegative)) ||
      (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fc_ಧನಾತ್ಮಕ) && ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fc_ಧನಾತ್ಮಕ)))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegative);
  ಆದರೆ ((ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fc_ಧನಾತ್ಮಕ) &&
       ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fcNegative)) ||
      (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcNegative) && ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fc_ಧನಾತ್ಮಕ)))
    ತಿಳಿದ.ತಿಳಿದ_Not(fc_ಧನಾತ್ಮಕ);

  // Normal and subnormal results require two non-zero finite operands.
  ಆದರೆ ((ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal) &&
       ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal)) ||
      (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fc_Pos_ಸಾಮಾನ್ಯ | fcPosSubnormal) &&
       ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fc_Pos_ಸಾಮಾನ್ಯ | fcPosSubnormal)))
    ತಿಳಿದ.ತಿಳಿದ_Not(fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal);
  ಆದರೆ ((ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal) &&
       ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fc_Pos_ಸಾಮಾನ್ಯ | fcPosSubnormal)) ||
      (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fc_Pos_ಸಾಮಾನ್ಯ | fcPosSubnormal) &&
       ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal)))
    ತಿಳಿದ.ತಿಳಿದ_Not(fc_Pos_ಸಾಮಾನ್ಯ | fcPosSubnormal);

  // 0 / X => 0 or NaN
  ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Always(fcZero))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcSubnormal | fc_ಸಾಮಾನ್ಯ | fcInf);

  // X / 0 => NaN or Inf
  ಆದರೆ (ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Always(fcZero))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcFinite);

  ಹಿಂತಿರುಗಿ apply_Output_Denormal_ಬಹುಲಕ(ತಿಳಿದ, ಬಹುಲಕ);
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::fdiv_self(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src,
                                     Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  // X / X is always exactly 1.0 or a NaN.
  ತಿಳಿದ_FPClass ತಿಳಿದ(fcNan | fc_Pos_ಸಾಮಾನ್ಯ);

  ತಿಳಿದ.propagateNonSNaN(ತಿಳಿದ_Src);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Inf_Or_Na_N() && ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Logical_Zero(ಬಹುಲಕ))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNan);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::frem(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_LHS_2,
                                ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_RHS_2,
                                Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  ತಿಳಿದ.ತಿಳಿದ_Not(fcInf);

  ತಿಳಿದ.propagateNonSNaN(ತಿಳಿದ_LHS_2, ತಿಳಿದ_RHS_2);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never_Inf_Or_Na_N() && ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never_Na_N() &&
      ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never_Logical_Zero(ಬಹುಲಕ)) {
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNan);
  }

  // The sign for frem is the same as the first operand.
  ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fc_Pos_ಸಾಮಾನ್ಯ | fcPosSubnormal))
    ತಿಳಿದ.ತಿಳಿದ_Not(fc_Pos_ಸಾಮಾನ್ಯ | fcPosSubnormal);
  ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal))
    ತಿಳಿದ.ತಿಳಿದ_Not(fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcNegFinite))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcPosFinite) &&
      ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never_Logical_Pos_Zero(ಬಹುಲಕ) &&
      (!ಬಹುಲಕ.outputs_May_Be_ಧನಾತ್ಮಕ_Zero() ||
       ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal)))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosZero);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::frem_self(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src,
                                     Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  // X % X is always exactly [+-]0.0 or a NaN.
  ತಿಳಿದ_FPClass ತಿಳಿದ(fcNan | fcZero);

  ತಿಳಿದ.propagateNonSNaN(ತಿಳಿದ_Src);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Inf_Or_Na_N() && ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Logical_Zero(ಬಹುಲಕ))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fc_Pos_ಸಾಮಾನ್ಯ | fcPosSubnormal))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosZero);
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegZero);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::fma(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_LHS_2,
                               ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_RHS_2,
                               ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Addend,
                               Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass Mul = fmul(ತಿಳಿದ_LHS_2, ತಿಳಿದ_RHS_2, ಬಹುಲಕ);

  // FMA differs from the base fmul + fadd handling only in the treatment of -0
  // results.
  //
  // If the multiply is a -0 due to rounding, the final -0 + 0 will be -0,
  // unlike for a separate fadd.
  ತಿಳಿದ_FPClass ತಿಳಿದ = fadd_impl(Mul, ತಿಳಿದ_Addend, /*IsFMA=*/ನಿಜ);

  // propagateNonSNaN for 3 arguments.
  ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcSNan) && ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fcSNan) &&
      ತಿಳಿದ_Addend.is_ತಿಳಿದ_Never(fcSNan))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcSNan);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::fmuladd(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_LHS_2,
                                   ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_RHS_2,
                                   ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Addend,
                                   Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  ತಿಳಿದ_FPClass FMA = fma(ತಿಳಿದ_LHS_2, ತಿಳಿದ_RHS_2, ತಿಳಿದ_Addend, ಬಹುಲಕ);
  ತಿಳಿದ_FPClass Mul_ಕೂಡಿಸು = fadd(fmul(ತಿಳಿದ_LHS_2, ತಿಳಿದ_RHS_2, ಬಹುಲಕ), ತಿಳಿದ_Addend, ಬಹುಲಕ);
  ಹಿಂತಿರುಗಿ FMA.intersectWith(Mul_ಕೂಡಿಸು);
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::fma_ವರ್ಗಫಲ(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Squared,
                                      ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Addend,
                                      Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass Squared = ವರ್ಗಫಲ(ತಿಳಿದ_Squared, ಬಹುಲಕ);
  ತಿಳಿದ_FPClass ತಿಳಿದ = fadd_impl(Squared, ತಿಳಿದ_Addend, /*IsFMA=*/ನಿಜ);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  ಆದರೆ (ತಿಳಿದ_Addend.is_ತಿಳಿದ_Never(fcNegInf | fcNan) && Squared.is_ತಿಳಿದ_Never(fcNan))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNan);

  ತಿಳಿದ.propagateNonSNaN(ತಿಳಿದ_Squared, ತಿಳಿದ_Addend);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::fmuladd_ವರ್ಗಫಲ(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Squared,
                                          ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Addend,
                                          Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass FMA = fma_ವರ್ಗಫಲ(ತಿಳಿದ_Squared, ತಿಳಿದ_Addend, ಬಹುಲಕ);
  ತಿಳಿದ_FPClass Mul_ಕೂಡಿಸು = fadd(ವರ್ಗಫಲ(ತಿಳಿದ_Squared, ಬಹುಲಕ), ತಿಳಿದ_Addend, ಬಹುಲಕ);
  ಹಿಂತಿರುಗಿ FMA.intersectWith(Mul_ಕೂಡಿಸು);
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::ಘಾತಾಂಕ(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;
  ತಿಳಿದ.ತಿಳಿದ_Not(fcNegative);

  ತಿಳಿದ.propagateNonNaN(ತಿಳಿದ_Src);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fc_Neg_ಸಾಮಾನ್ಯ | fcNegInf))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fc_Neg_ಸಾಮಾನ್ಯ))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fc_Pos_ಸಾಮಾನ್ಯ | fcPosInf))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosInf);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::ಲಘುಗಣಕ(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src, Denormal_ಬಹುಲಕ ಬಹುಲಕ,
                               ತಾರ್ಕಿಕ IsMultiUnitFPType) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  ಆದರೆ (!IsMultiUnitFPType)
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegZero | fcSubnormal);

  ತಿಳಿದ.propagateNonSNaN(ತಿಳಿದ_Src);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Pos_Infinity())
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosInf);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Na_N() && ತಿಳಿದ_Src.cannotBeOrderedLessThanZero())
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNan);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Logical_Zero(ಬಹುಲಕ))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegInf);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::ವರ್ಗಮೂಲ(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src,
                                Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;
  ತಿಳಿದ.ತಿಳಿದ_Not(fcPosSubnormal);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Pos_Infinity())
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosInf);

  ತಿಳಿದ.propagateNonSNaN(ತಿಳಿದ_Src);

  // Any negative value besides -0 returns a nan.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Na_N() && ತಿಳಿದ_Src.cannotBeOrderedLessThanZero())
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  ತಿಳಿದ.ತಿಳಿದ_Not(fcNegInf | fcNegSubnormal | fc_Neg_ಸಾಮಾನ್ಯ);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Logical_Neg_Zero(ಬಹುಲಕ))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegZero);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::ಜ್ಯಾ(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  ತಿಳಿದ.propagateNonSNaN(ತಿಳಿದ_Src);

  // Return NaN on infinite inputs.
  ತಿಳಿದ.ತಿಳಿದ_Not(fcInf);
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Na_N() && ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Infinity())
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNan);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::ಕೋಜ್ಯಾ(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src) {
  ಹಿಂತಿರುಗಿ ಜ್ಯಾ(ತಿಳಿದ_Src);
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::tan(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  ತಿಳಿದ.ತಿಳಿದ_Not(fcInf);

  ತಿಳಿದ.propagateNonSNaN(ತಿಳಿದ_Src);

  // NaN propagates. tan(+-Inf) is NaN.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Na_N() && ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Infinity())
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNan);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::sinh(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcNegative))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcPosSubnormal | fc_Pos_ಸಾಮಾನ್ಯ | fcPosInf))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosSubnormal | fc_Pos_ಸಾಮಾನ್ಯ | fcPosInf);

  ತಿಳಿದ.propagateNonNaN(ತಿಳಿದ_Src);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::cosh(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  ತಿಳಿದ.ತಿಳಿದ_Not(fcNegative | fcZero | fcSubnormal);

  ತಿಳಿದ.propagateNonNaN(ತಿಳಿದ_Src);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::tanh(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  // tanh is bounded to (-1, 1), never Inf.
  ತಿಳಿದ.ತಿಳಿದ_Not(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcNegSubnormal | fc_Neg_ಸಾಮಾನ್ಯ | fcNegInf)) {
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegSubnormal | fc_Neg_ಸಾಮಾನ್ಯ);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Neg_Zero())
      ತಿಳಿದ.ತಿಳಿದ_Not(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcPosSubnormal | fc_Pos_ಸಾಮಾನ್ಯ | fcPosInf))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosSubnormal | fc_Pos_ಸಾಮಾನ್ಯ);

  ತಿಳಿದ.propagateNonNaN(ತಿಳಿದ_Src);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::asin(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  ತಿಳಿದ.ತಿಳಿದ_Not(fcInf);

  ತಿಳಿದ.propagateNonSNaN(ತಿಳಿದ_Src);

  // asin is sign-preserving for finite arguments.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcNegFinite))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegFinite);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcPosSubnormal | fc_Pos_ಸಾಮಾನ್ಯ))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosSubnormal | fc_Pos_ಸಾಮಾನ್ಯ);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::acos(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  // acos(x) is bounded to [0, pi] for -1 <= x <= 1, and is never negative,
  // infinite, or subnormal. The smallest non-zero value occurs when x is
  // close to 1.0, where acos(x) can be approximated by sqrt(2 * (1 - x)).
  // Since sqrt cannot produce a subnormal result, we can conclude that
  // acos(x) will also never produce a subnormal result.
  ತಿಳಿದ.ತಿಳಿದ_Not(fcNegative | fcInf | fcSubnormal);

  // acos(x) == +0.0 iff x == +1.0
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fc_Pos_ಸಾಮಾನ್ಯ))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcZero);

  ತಿಳಿದ.propagateNonSNaN(ತಿಳಿದ_Src);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::atan(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  ತಿಳಿದ.ತಿಳಿದ_Not(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcNegSubnormal | fc_Neg_ಸಾಮಾನ್ಯ | fcNegInf))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegSubnormal | fc_Neg_ಸಾಮಾನ್ಯ);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcPosSubnormal | fc_Pos_ಸಾಮಾನ್ಯ | fcPosInf))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosSubnormal | fc_Pos_ಸಾಮಾನ್ಯ);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal | fcNegZero))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegZero);

  ತಿಳಿದ.propagateNonNaN(ತಿಳಿದ_Src);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::atan2(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Y,
                                 ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_X,
                                 Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ_Y_2 = apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(ತಿಳಿದ_Y, ಬಹುಲಕ);
  ತಿಳಿದ_FPClass ತಿಳಿದ_X_2 = apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(ತಿಳಿದ_X, ಬಹುಲಕ);
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  // Even though these deductions are correct, we are ignoring the following
  // potentially erroneous cases:
  //   * atan2(y, inf) is not subnormal
  //   * atan2(inf, x) is not zero or subnormal

  // atan2 result is in (-pi, pi], never Inf.
  ತಿಳಿದ.ತಿಳಿದ_Not(fcInf);

  ತಿಳಿದ.propagateNonNaN(ತಿಳಿದ_Y_2, ತಿಳಿದ_X_2);

  // If x <= -0.0, then |atan2(y, x)| >= pi/2
  ಆದರೆ (ತಿಳಿದ_X_2.is_ತಿಳಿದ_Never(fc_ಧನಾತ್ಮಕ))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  ಆದರೆ (ತಿಳಿದ_Y_2.is_ತಿಳಿದ_Never(fcNegative))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  ಆದರೆ (ತಿಳಿದ_Y_2.is_ತಿಳಿದ_Never(fc_ಧನಾತ್ಮಕ))
    ತಿಳಿದ.ತಿಳಿದ_Not(fc_ಧನಾತ್ಮಕ);

  ಹಿಂತಿರುಗಿ apply_Output_Denormal_ಬಹುಲಕ(ತಿಳಿದ, ಬಹುಲಕ);
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::fpext(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src,
                                 ಸ್ಥಿರಾಂಕ fltSemantics &DstTy,
                                 ಸ್ಥಿರಾಂಕ fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  ತಿಳಿದ_FPClass ತಿಳಿದ = ತಿಳಿದ_Src;

  // All subnormal inputs should be in the normal range in the result type.
  ಆದರೆ (APFloat::is_Representable_As_ಸಾಮಾನ್ಯ_ಒಳ(SrcTy, DstTy)) {
    ಆದರೆ (ತಿಳಿದ.get_ತಿಳಿದ_FPClasses() & fcPosSubnormal)
      ತಿಳಿದ.ಗಣ_ತಿಳಿದ_FPClasses(ತಿಳಿದ.get_ತಿಳಿದ_FPClasses() | fc_Pos_ಸಾಮಾನ್ಯ);
    ಆದರೆ (ತಿಳಿದ.get_ತಿಳಿದ_FPClasses() & fcNegSubnormal)
      ತಿಳಿದ.ಗಣ_ತಿಳಿದ_FPClasses(ತಿಳಿದ.get_ತಿಳಿದ_FPClasses() | fc_Neg_ಸಾಮಾನ್ಯ);
    ತಿಳಿದ.ತಿಳಿದ_Not(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  ಆದರೆ (!ತಿಳಿದ.is_ತಿಳಿದ_Never_Na_N())
    ತಿಳಿದ.ಗಣ_Sign_ಬಿಟ್(std::nullopt);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::fptrunc(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  ಆದರೆ (ತಿಳಿದ_Src.cannotBeOrderedLessThanZero())
    ತಿಳಿದ.ತಿಳಿದ_Not(ತಿಳಿದ_FPClass::Ordered_Less_Than_Zero_ಮುಖವಾಡ);

  ತಿಳಿದ.propagateNonNaN(ತಿಳಿದ_Src);

  // Infinity needs a range check.
  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::ಸುತ್ತು_ಗಮ್ಯ_Integral(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src,
                                           ತಾರ್ಕಿಕ MayBeMultiUnitFPType,
                                           Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  // Integer results cannot be subnormal.
  ತಿಳಿದ.ತಿಳಿದ_Not(fcSubnormal);

  ತಿಳಿದ.propagateNonNaN(ತಿಳಿದ_Src);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  ಆದರೆ (!MayBeMultiUnitFPType) {
    ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Pos_Infinity())
      ತಿಳಿದ.ತಿಳಿದ_Not(fcPosInf);
    ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Neg_Infinity())
      ತಿಳಿದ.ತಿಳಿದ_Not(fcNegInf);
  }

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fc_Pos_ಸಾಮಾನ್ಯ | fcPosSubnormal))
    ತಿಳಿದ.ತಿಳಿದ_Not(fc_Pos_ಸಾಮಾನ್ಯ);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal))
    ತಿಳಿದ.ತಿಳಿದ_Not(fc_Neg_ಸಾಮಾನ್ಯ);

  // Negative round ups towards zero produce negative zero.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcNegFinite))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegZero);

  // Negative subnormals may flush to positive zero.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcPosFinite) &&
      (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcNegSubnormal) ||
       !ಬಹುಲಕ.inputs_May_Be_ಧನಾತ್ಮಕ_Zero()))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosZero);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::trunc(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src,
                                 Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ = ಸುತ್ತು_ಗಮ್ಯ_Integral(ತಿಳಿದ_Src,
                                       /*MayBeMultiUnitFPType=*/ನಿಜ, ಬಹುಲಕ);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Pos_Infinity())
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosInf);
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Neg_Infinity())
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegInf);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::floor(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src,
                                 ತಾರ್ಕಿಕ IsMultiUnitFPType, Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ = ಸುತ್ತು_ಗಮ್ಯ_Integral(ತಿಳಿದ_Src, IsMultiUnitFPType, ಬಹುಲಕ);

  // Applicable for multi-unit floating point types.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Pos_Infinity())
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosInf);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::ceil(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src,
                                ತಾರ್ಕಿಕ IsMultiUnitFPType, Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ = ಸುತ್ತು_ಗಮ್ಯ_Integral(ತಿಳಿದ_Src, IsMultiUnitFPType, ಬಹುಲಕ);

  // Applicable for multi-unit floating point types.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Neg_Infinity())
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegInf);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::frexp_mant(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src,
                                      Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  ತಿಳಿದ.ತಿಳಿದ_Not(fcSubnormal);

  ತಿಳಿದ.propagateNonNaN(ತಿಳಿದ_Src);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Pos_Infinity())
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosInf);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Neg_Infinity())
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegInf);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Logical_Pos_Zero(ಬಹುಲಕ))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcPosZero);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Logical_Neg_Zero(ಬಹುಲಕ))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fc_Pos_ಸಾಮಾನ್ಯ | fcPosSubnormal))
    ತಿಳಿದ.ತಿಳಿದ_Not(fc_Pos_ಸಾಮಾನ್ಯ);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal))
    ತಿಳಿದ.ತಿಳಿದ_Not(fc_Neg_ಸಾಮಾನ್ಯ);

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::ldexp(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src_2,
                                 ಸ್ಥಿರಾಂಕ APInt &ConstantRangeExpMin,
                                 ಸ್ಥಿರಾಂಕ APInt &ConstantRangeExpMax,
                                 ಸ್ಥಿರಾಂಕ fltSemantics &Flt, Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  ತಿಳಿದ_FPClass ತಿಳಿದ_Src = apply_ಒಳಹರಿವು_Denormal_ಬಹುಲಕ(ತಿಳಿದ_Src_2, ಬಹುಲಕ);

  ಆದರೆ (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    ತಿಳಿದ.ಗಣ_ತಿಳಿದ_FPClasses(ತಿಳಿದ_Src.get_ತಿಳಿದ_FPClasses() | fcNan);
    ತಿಳಿದ.propagateNonNaN(ತಿಳಿದ_Src);
    ಹಿಂತಿರುಗಿ apply_Output_Denormal_ಬಹುಲಕ(ತಿಳಿದ, ಬಹುಲಕ);
  }

  ತಿಳಿದ.propagateNonNaN(ತಿಳಿದ_Src);

  // Sign is preserved, but underflows may produce zeroes.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcNegative))
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegative);
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ತಿಳಿದ_Src.cannotBeOrderedLessThanZero())
    ತಿಳಿದ.ತಿಳಿದ_Not(Ordered_Less_Than_Zero_ಮುಖವಾಡ);

  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fc_ಧನಾತ್ಮಕ))
    ತಿಳಿದ.ತಿಳಿದ_Not(fc_ಧನಾತ್ಮಕ);
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ತಿಳಿದ_Src.cannotBeOrderedGreaterThanZero())
    ತಿಳಿದ.ತಿಳಿದ_Not(Ordered_Greater_Than_Zero_ಮುಖವಾಡ);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  ಆದರೆ (&Flt != &APFloat::PPCDoubleDouble()) {
    ಚಿಹ್ನೆರಹಿತ Precision = APFloat::semanticsPrecision(Flt);
    ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ Mantissa_ಬಿಟ್ಟುಗಳು = Precision - 1;
    ಆದರೆ (ConstantRangeExpMin.sge(Mantissa_ಬಿಟ್ಟುಗಳು))
      ತಿಳಿದ.ತಿಳಿದ_Not(fcSubnormal);
  }

  ಆದರೆ (ConstantRangeExpMax.is_Non_ಧನಾತ್ಮಕ()) {
    // If we know the power is <= 0, can't introduce inf
    ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Pos_Infinity())
      ತಿಳಿದ.ತಿಳಿದ_Not(fcPosInf);
    ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Neg_Infinity())
      ತಿಳಿದ.ತಿಳಿದ_Not(fcNegInf);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Pos_Subnormal())
      ತಿಳಿದ.ತಿಳಿದ_Not(fcPosSubnormal);
    ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Neg_Subnormal())
      ತಿಳಿದ.ತಿಳಿದ_Not(fcNegSubnormal);
    ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Pos_Zero())
      ತಿಳಿದ.ತಿಳಿದ_Not(fcPosZero);
    ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never_Neg_Zero())
      ತಿಳಿದ.ತಿಳಿದ_Not(fcNegZero);
  }

  ಹಿಂತಿರುಗಿ apply_Output_Denormal_ಬಹುಲಕ(ತಿಳಿದ, ಬಹುಲಕ);
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::ldexp(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src,
                                 ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &Exp_ಬಿಟ್ಟುಗಳು,
                                 ಸ್ಥಿರಾಂಕ fltSemantics &Flt, Denormal_ಬಹುಲಕ ಬಹುಲಕ) {
  ಹಿಂತಿರುಗಿ ldexp(ತಿಳಿದ_Src, Exp_ಬಿಟ್ಟುಗಳು.get_Signed_Min_ಮೌಲ್ಯ(),
               Exp_ಬಿಟ್ಟುಗಳು.get_Signed_Max_ಮೌಲ್ಯ(), Flt, ಬಹುಲಕ);
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::ಘಾತ(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_LHS_2,
                               ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_RHS_2) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;

  ತಿಳಿದ.propagateNonSNaN(ತಿಳಿದ_LHS_2, ತಿಳಿದ_RHS_2);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcNan | fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal) &&
      ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never_Na_N())
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcNegative)) {
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegative);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal)) {
    ತಿಳಿದ.ತಿಳಿದ_Not(fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    ಆದರೆ ((ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcNegZero) ||
         ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fc_Pos_ಸಾಮಾನ್ಯ)) &&
        (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcNegInf) || ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fc_Neg_ಸಾಮಾನ್ಯ)))
      ತಿಳಿದ.ತಿಳಿದ_Not(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    ಆದರೆ ((ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcNegZero) ||
         ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fc_Neg_ಸಾಮಾನ್ಯ)) &&
        (ತಿಳಿದ_LHS_2.is_ತಿಳಿದ_Never(fcNegInf) || ತಿಳಿದ_RHS_2.is_ತಿಳಿದ_Never(fc_Pos_ಸಾಮಾನ್ಯ)))
      ತಿಳಿದ.ತಿಳಿದ_Not(fcNegInf);
  }

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}

ತಿಳಿದ_FPClass ತಿಳಿದ_FPClass::powi(ಸ್ಥಿರಾಂಕ ತಿಳಿದ_FPClass &ತಿಳಿದ_Src,
                                ಸ್ಥಿರಾಂಕ ತಿಳಿದ_ಬಿಟ್ಟುಗಳು &Exponent_ತಿಳಿದ_ಬಿಟ್ಟುಗಳು) {
  ತಿಳಿದ_FPClass ತಿಳಿದ;
  ತಿಳಿದ.propagateNonNaN(ತಿಳಿದ_Src);

  ಆದರೆ (Exponent_ತಿಳಿದ_ಬಿಟ್ಟುಗಳು.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcSNan)) {
      ತಿಳಿದ.ತಿಳಿದ_Not(~fc_Pos_ಸಾಮಾನ್ಯ);
      ಹಿಂತಿರುಗಿ ತಿಳಿದ;
    }

    ತಿಳಿದ.ತಿಳಿದ_Not(~(fc_Pos_ಸಾಮಾನ್ಯ | fcNan));
    ಹಿಂತಿರುಗಿ ತಿಳಿದ;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcNegative) || Exponent_ತಿಳಿದ_ಬಿಟ್ಟುಗಳು.isEven()) {
    ತಿಳಿದ.ತಿಳಿದ_Not(fcNegative);
  } ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal)) {
    ತಿಳಿದ.ತಿಳಿದ_Not(fc_Neg_ಸಾಮಾನ್ಯ | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Is_ತಿಳಿದ_Never_Odd_ಧನಾತ್ಮಕ = Exponent_ತಿಳಿದ_ಬಿಟ್ಟುಗಳು.isNegative();
    ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ Is_ತಿಳಿದ_Never_Odd_Negative = Exponent_ತಿಳಿದ_ಬಿಟ್ಟುಗಳು.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    ಆದರೆ ((ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcNegZero) || Is_ತಿಳಿದ_Never_Odd_ಧನಾತ್ಮಕ) &&
        (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcNegInf) || Is_ತಿಳಿದ_Never_Odd_Negative))
      ತಿಳಿದ.ತಿಳಿದ_Not(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    ಆದರೆ ((ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcNegZero) || Is_ತಿಳಿದ_Never_Odd_Negative) &&
        (ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcNegInf) || Is_ತಿಳಿದ_Never_Odd_ಧನಾತ್ಮಕ))
      ತಿಳಿದ.ತಿಳಿದ_Not(fcNegInf);
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
    APInt MinExp = Exponent_ತಿಳಿದ_ಬಿಟ್ಟುಗಳು.get_Signed_Min_ಮೌಲ್ಯ();
    APInt MaxExp = Exponent_ತಿಳಿದ_ಬಿಟ್ಟುಗಳು.get_Signed_Max_ಮೌಲ್ಯ();

    // powi(inf, exp), exp > 0
    ತಾರ್ಕಿಕ MayInfSrc =
        !ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcInf) && MaxExp.is_Strictly_ಧನಾತ್ಮಕ();

    // powi(+/-0, exp), exp < 0
    ತಾರ್ಕಿಕ MayDivByZero = !ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcZero) && MinExp.isNegative();

    // powi(finite, exp), |exp| > 1
    ತಾರ್ಕಿಕ MayFinite = !ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fc_ಸಾಮಾನ್ಯ | fcSubnormal);
    ತಾರ್ಕಿಕ MayAbsExpGT1 = MinExp.slt(-1) || MaxExp.sgt(1);
    ತಾರ್ಕಿಕ May_Finite_ಅತಿಪ್ರವಾಹ = MayFinite && MayAbsExpGT1;

    // powi(subnormal, -1)
    ತಾರ್ಕಿಕ MayBeNegOne = Exponent_ತಿಳಿದ_ಬಿಟ್ಟುಗಳು.Zero.isZero();
    ತಾರ್ಕಿಕ MaySubnormInv = !ತಿಳಿದ_Src.is_ತಿಳಿದ_Never(fcSubnormal) && MayBeNegOne;

    ಆದರೆ (!MayInfSrc && !MayDivByZero && !May_Finite_ಅತಿಪ್ರವಾಹ && !MaySubnormInv)
      ತಿಳಿದ.ತಿಳಿದ_Not(fcInf);
  }

  ಹಿಂತಿರುಗಿ ತಿಳಿದ;
}
