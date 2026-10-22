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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/KnownFPClass.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/APFloat.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/KnownBits.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

ਗਿਆਤ_FPClass::ਗਿਆਤ_FPClass(ਅਚਲ APFloat &C)
    : ਗਿਆਤ_FPClasses_ਮੁੱਲ(C.classify()) {
  ਸੈੱਟ_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ(C.isNegative());
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src,
                                                  Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ = ਗਿਆਤ_Src;
  ਚੋਣ (ਬਹੁਲਕ.ਨਿਵੇਸ਼) {
  ਹਾਲਤ Denormal_ਬਹੁਲਕ::IEEE:
    ਵਾਪਸ ਗਿਆਤ;
  ਹਾਲਤ Denormal_ਬਹੁਲਕ::PreserveSign:
    ਜੇ (ਗਿਆਤ_Src.get_ਗਿਆਤ_FPClasses() & fcPosSubnormal)
      ਗਿਆਤ.ਸੈੱਟ_ਗਿਆਤ_FPClasses(ਗਿਆਤ.get_ਗਿਆਤ_FPClasses() | fcPosZero);
    ਜੇ (ਗਿਆਤ_Src.get_ਗਿਆਤ_FPClasses() & fcNegSubnormal)
      ਗਿਆਤ.ਸੈੱਟ_ਗਿਆਤ_FPClasses(ਗਿਆਤ.get_ਗਿਆਤ_FPClasses() | fcNegZero);
    ਵਾਪਸ ਗਿਆਤ;
  ਹਾਲਤ Denormal_ਬਹੁਲਕ::ਸਕਾਰਾਤਮਕ_Zero:
    ਜੇ (ਗਿਆਤ_Src.get_ਗਿਆਤ_FPClasses() & fcSubnormal) {
      ਗਿਆਤ.ਸੈੱਟ_ਗਿਆਤ_FPClasses(ਗਿਆਤ.get_ਗਿਆਤ_FPClasses() | fcPosZero);
      ਜੇ (ਗਿਆਤ_Src.get_ਗਿਆਤ_FPClasses() & fcNegSubnormal)
        ਗਿਆਤ.ਸੈੱਟ_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ(std::nullopt);
    }
    ਵਾਪਸ ਗਿਆਤ;
  ਮੂਲ:
    ਜੇ (ਗਿਆਤ_Src.get_ਗਿਆਤ_FPClasses() & fcSubnormal) {
      ਗਿਆਤ.ਸੈੱਟ_ਗਿਆਤ_FPClasses(ਗਿਆਤ.get_ਗਿਆਤ_FPClasses() | fcPosZero);
      ਜੇ (ਗਿਆਤ_Src.get_ਗਿਆਤ_FPClasses() & fcNegSubnormal)
        ਗਿਆਤ.ਸੈੱਟ_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ(std::nullopt);
    }
    ਜੇ (ਗਿਆਤ_Src.get_ਗਿਆਤ_FPClasses() & fcNegSubnormal)
      ਗਿਆਤ.ਸੈੱਟ_ਗਿਆਤ_FPClasses(ਗਿਆਤ.get_ਗਿਆਤ_FPClasses() | fcNegZero);
    ਵਾਪਸ ਗਿਆਤ;
  }

  llvm_unreachable("covered switch over denormal mode");
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::apply_Output_Denormal_ਬਹੁਲਕ(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src,
                                                   Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ = ਗਿਆਤ_Src;
  ਚੋਣ (ਬਹੁਲਕ.Output) {
  ਹਾਲਤ Denormal_ਬਹੁਲਕ::IEEE:
    ਵਾਪਸ ਗਿਆਤ;
  ਹਾਲਤ Denormal_ਬਹੁਲਕ::PreserveSign:
    ਜੇ (ਗਿਆਤ_Src.get_ਗਿਆਤ_FPClasses() & fcPosSubnormal)
      ਗਿਆਤ.ਸੈੱਟ_ਗਿਆਤ_FPClasses(ਗਿਆਤ.get_ਗਿਆਤ_FPClasses() | fcPosZero);
    ਜੇ (ਗਿਆਤ_Src.get_ਗਿਆਤ_FPClasses() & fcNegSubnormal)
      ਗਿਆਤ.ਸੈੱਟ_ਗਿਆਤ_FPClasses(ਗਿਆਤ.get_ਗਿਆਤ_FPClasses() | fcNegZero);
    ਵਾਪਸ ਗਿਆਤ;
  ਹਾਲਤ Denormal_ਬਹੁਲਕ::ਸਕਾਰਾਤਮਕ_Zero:
    ਜੇ (ਗਿਆਤ_Src.get_ਗਿਆਤ_FPClasses() & fcSubnormal) {
      ਗਿਆਤ.ਸੈੱਟ_ਗਿਆਤ_FPClasses(ਗਿਆਤ.get_ਗਿਆਤ_FPClasses() | fcPosZero);
      ਜੇ (ਗਿਆਤ_Src.get_ਗਿਆਤ_FPClasses() & fcNegSubnormal)
        ਗਿਆਤ.ਸੈੱਟ_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ(std::nullopt);
    }
    ਵਾਪਸ ਗਿਆਤ;
  ਮੂਲ:
    ਜੇ (ਗਿਆਤ_Src.get_ਗਿਆਤ_FPClasses() & fcSubnormal) {
      ਗਿਆਤ.ਸੈੱਟ_ਗਿਆਤ_FPClasses(ਗਿਆਤ.get_ਗਿਆਤ_FPClasses() | fcPosZero);
      ਜੇ (ਗਿਆਤ_Src.get_ਗਿਆਤ_FPClasses() & fcNegSubnormal)
        ਗਿਆਤ.ਸੈੱਟ_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ(std::nullopt);
    }
    ਜੇ (ਗਿਆਤ_Src.get_ਗਿਆਤ_FPClasses() & fcNegSubnormal)
      ਗਿਆਤ.ਸੈੱਟ_ਗਿਆਤ_FPClasses(ਗਿਆਤ.get_ਗਿਆਤ_FPClasses() | fcNegZero);
    ਵਾਪਸ ਗਿਆਤ;
  }

  llvm_unreachable("covered switch over denormal mode");
}

ਤਾਰਕਿਕ ਗਿਆਤ_FPClass::is_ਗਿਆਤ_Never_Logical_Zero(Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) ਅਚਲ {
  ਵਾਪਸ apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(*ਇਹ, ਬਹੁਲਕ).is_ਗਿਆਤ_Never_Zero();
}

ਤਾਰਕਿਕ ਗਿਆਤ_FPClass::is_ਗਿਆਤ_Never_Logical_Neg_Zero(Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) ਅਚਲ {
  ਵਾਪਸ apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(*ਇਹ, ਬਹੁਲਕ).is_ਗਿਆਤ_Never_Neg_Zero();
}

ਤਾਰਕਿਕ ਗਿਆਤ_FPClass::is_ਗਿਆਤ_Never_Logical_Pos_Zero(Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) ਅਚਲ {
  ਵਾਪਸ apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(*ਇਹ, ਬਹੁਲਕ).is_ਗਿਆਤ_Never_Pos_Zero();
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::minMaxLike(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_LHS,
                                      ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_RHS,
                                      Min_Max_ਕਿਸਮ ਕਿਸਮ_3, Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ_LHS_2 = apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(ਗਿਆਤ_LHS, ਬਹੁਲਕ);
  ਗਿਆਤ_FPClass ਗਿਆਤ_RHS_2 = apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(ਗਿਆਤ_RHS, ਬਹੁਲਕ);

  ਗਿਆਤ_FPClass ਗਿਆਤ;

  ਗਿਆਤ.propagateNonNaN(ਗਿਆਤ_LHS_2, ਗਿਆਤ_RHS_2);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  ਜੇ ((ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never_Na_N() || ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never_Na_N()) &&
      (ਕਿਸਮ_3 == Min_Max_ਕਿਸਮ::minnum || ਕਿਸਮ_3 == Min_Max_ਕਿਸਮ::maxnum ||
       ਕਿਸਮ_3 == Min_Max_ਕਿਸਮ::minimumnum || ਕਿਸਮ_3 == Min_Max_ਕਿਸਮ::maximumnum))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNan);

  ਚੋਣ (ਕਿਸਮ_3) {
  ਹਾਲਤ Min_Max_ਕਿਸਮ::maxnum: {
    ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never_Na_N())
      ਗਿਆਤ.ਗਿਆਤ_Not(orderedStrictlyLess(ਗਿਆਤ_LHS_2.get_ਗਿਆਤ_FPClasses(),
                                         /*OrderedZeroSign=*/ਝੂਠ));
    ਜੇ (ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never_Na_N())
      ਗਿਆਤ.ਗਿਆਤ_Not(orderedStrictlyLess(ਗਿਆਤ_RHS_2.get_ਗਿਆਤ_FPClasses(),
                                         /*OrderedZeroSign=*/ਝੂਠ));
    ਰੋਕੋ;
  }
  ਹਾਲਤ Min_Max_ਕਿਸਮ::minnum: {
    ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never_Na_N())
      ਗਿਆਤ.ਗਿਆਤ_Not(orderedStrictlyGreater(ਗਿਆਤ_LHS_2.get_ਗਿਆਤ_FPClasses(),
                                            /*OrderedZeroSign=*/ਝੂਠ));
    ਜੇ (ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never_Na_N())
      ਗਿਆਤ.ਗਿਆਤ_Not(orderedStrictlyGreater(ਗਿਆਤ_RHS_2.get_ਗਿਆਤ_FPClasses(),
                                            /*OrderedZeroSign=*/ਝੂਠ));
    ਰੋਕੋ;
  }
  ਹਾਲਤ Min_Max_ਕਿਸਮ::maximumnum: {
    ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never_Na_N())
      ਗਿਆਤ.ਗਿਆਤ_Not(orderedStrictlyLess(ਗਿਆਤ_LHS_2.get_ਗਿਆਤ_FPClasses(),
                                         /*OrderedZeroSign=*/ਸੱਚ));
    ਜੇ (ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never_Na_N())
      ਗਿਆਤ.ਗਿਆਤ_Not(orderedStrictlyLess(ਗਿਆਤ_RHS_2.get_ਗਿਆਤ_FPClasses(),
                                         /*OrderedZeroSign=*/ਸੱਚ));
    ਰੋਕੋ;
  }
  ਹਾਲਤ Min_Max_ਕਿਸਮ::minimumnum: {
    ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never_Na_N())
      ਗਿਆਤ.ਗਿਆਤ_Not(orderedStrictlyGreater(ਗਿਆਤ_LHS_2.get_ਗਿਆਤ_FPClasses(),
                                            /*OrderedZeroSign=*/ਸੱਚ));
    ਜੇ (ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never_Na_N())
      ਗਿਆਤ.ਗਿਆਤ_Not(orderedStrictlyGreater(ਗਿਆਤ_RHS_2.get_ਗਿਆਤ_FPClasses(),
                                            /*OrderedZeroSign=*/ਸੱਚ));
    ਰੋਕੋ;
  }
  ਹਾਲਤ Min_Max_ਕਿਸਮ::ਵੱਧ_ਤੋਂ_ਵੱਧ: {
    ਗਿਆਤ.ਗਿਆਤ_Not(orderedStrictlyLess(ਗਿਆਤ_LHS_2.get_ਗਿਆਤ_FPClasses(),
                                       /*OrderedZeroSign=*/ਸੱਚ) |
                   orderedStrictlyLess(ਗਿਆਤ_RHS_2.get_ਗਿਆਤ_FPClasses(),
                                       /*OrderedZeroSign=*/ਸੱਚ));
    ਰੋਕੋ;
  }
  ਹਾਲਤ Min_Max_ਕਿਸਮ::minimum: {
    ਗਿਆਤ.ਗਿਆਤ_Not(orderedStrictlyGreater(ਗਿਆਤ_LHS_2.get_ਗਿਆਤ_FPClasses(),
                                          /*OrderedZeroSign=*/ਸੱਚ) |
                   orderedStrictlyGreater(ਗਿਆਤ_RHS_2.get_ਗਿਆਤ_FPClasses(),
                                          /*OrderedZeroSign=*/ਸੱਚ));
    ਰੋਕੋ;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      ਗਿਆਤ_LHS_2.get_ਗਿਆਤ_FPClasses() | ਗਿਆਤ_RHS_2.get_ਗਿਆਤ_FPClasses() | fcNan;
  ਗਿਆਤ.ਗਿਆਤ_Not(~PossibleClasses);

  ਵਾਪਸ apply_Output_Denormal_ਬਹੁਲਕ(ਗਿਆਤ, ਬਹੁਲਕ);
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::canonicalize(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src,
                                        Denormal_ਬਹੁਲਕ Denorm_ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  ਗਿਆਤ.ਸੈੱਟ_ਗਿਆਤ_FPClasses(ਗਿਆਤ_Src.get_ਗਿਆਤ_FPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Na_N())
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNan);
  ਨਹੀਂਤਾਂ
    ਗਿਆਤ.ਗਿਆਤ_Not(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  ਜੇ (Denorm_ਬਹੁਲਕ == Denormal_ਬਹੁਲਕ::getIEEE()) {
    ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcPosZero))
      ਗਿਆਤ.ਗਿਆਤ_Not(fcPosZero);
    ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcNegZero))
      ਗਿਆਤ.ਗਿਆਤ_Not(fcNegZero);
    ਵਾਪਸ ਗਿਆਤ;
  }

  ਜੇ (Denorm_ਬਹੁਲਕ.inputsAreZero() || Denorm_ਬਹੁਲਕ.outputsAreZero())
    ਗਿਆਤ.ਗਿਆਤ_Not(fcSubnormal);

  ਜੇ (Denorm_ਬਹੁਲਕ == Denormal_ਬਹੁਲਕ::getPreserveSign()) {
    ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcPosZero | fcPosSubnormal))
      ਗਿਆਤ.ਗਿਆਤ_Not(fcPosZero);
    ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcNegZero | fcNegSubnormal))
      ਗਿਆਤ.ਗਿਆਤ_Not(fcNegZero);
    ਵਾਪਸ ਗਿਆਤ;
  }

  ਜੇ (Denorm_ਬਹੁਲਕ.ਨਿਵੇਸ਼ == Denormal_ਬਹੁਲਕ::ਸਕਾਰਾਤਮਕ_Zero ||
      (Denorm_ਬਹੁਲਕ.Output == Denormal_ਬਹੁਲਕ::ਸਕਾਰਾਤਮਕ_Zero &&
       Denorm_ਬਹੁਲਕ.ਨਿਵੇਸ਼ == Denormal_ਬਹੁਲਕ::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcNegZero))
      ਗਿਆਤ.ਗਿਆਤ_Not(fcNegZero);

    ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcPosZero | fcSubnormal))
      ਗਿਆਤ.ਗਿਆਤ_Not(fcPosZero);
  }

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::bitcast(ਅਚਲ fltSemantics &FltSemantics,
                                   ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &ਬਿੱਟ_2) {
  ਪ੍ਰਤਿਜ੍ਞਾ(FltSemantics.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ == ਬਿੱਟ_2.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() &&
         "Bitcast operand has incorrect bit width");
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  ਜੇ (ਬਿੱਟ_2.hasConflict())
    ਵਾਪਸ ਗਿਆਤ;

  // Return unknown for types we have not validated.
  ਸਵੈਚਾਲਿਤ IsSupported = [](ਅਚਲ fltSemantics &Semantics) {
    ਚੋਣ (APFloat::Semantics_ਮੰਜ਼ਿਲ_Enum(Semantics)) {
    ਹਾਲਤ APFloat_ਅਧਾਰ::S_IEEEhalf:
    ਹਾਲਤ APFloat_ਅਧਾਰ::S_BFloat:
    ਹਾਲਤ APFloat_ਅਧਾਰ::S_IEEEsingle:
    ਹਾਲਤ APFloat_ਅਧਾਰ::S_IEEEdouble:
    ਹਾਲਤ APFloat_ਅਧਾਰ::S_IEEEquad:
    ਹਾਲਤ APFloat_ਅਧਾਰ::S_x87DoubleExtended:
      ਵਾਪਸ ਸੱਚ;
    ਮੂਲ:
      ਵਾਪਸ ਝੂਠ;
    }
  };
  ਜੇ (!IsSupported(FltSemantics))
    ਵਾਪਸ ਗਿਆਤ;

  // Transfer information from the sign bit.
  ਜੇ (ਬਿੱਟ_2.isNonNegative())
    ਗਿਆਤ.sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Must_Be_Zero();
  ਨਹੀਂਤਾਂ ਜੇ (ਬਿੱਟ_2.isNegative())
    ਗਿਆਤ.sign_ਦੋਆਧਾਰੀ_ਅੰਕ_Must_Be_One();

  ਚੋਣ (APFloat_ਅਧਾਰ::Semantics_ਮੰਜ਼ਿਲ_Enum(FltSemantics)) {
  ਹਾਲਤ APFloat_ਅਧਾਰ::S_IEEEhalf:
  ਹਾਲਤ APFloat_ਅਧਾਰ::S_BFloat:
  ਹਾਲਤ APFloat_ਅਧਾਰ::S_IEEEsingle:
  ਹਾਲਤ APFloat_ਅਧਾਰ::S_IEEEdouble:
  ਹਾਲਤ APFloat_ਅਧਾਰ::S_IEEEquad: {
    ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ Mantissa_ਬਿੱਟ = FltSemantics.precision - 1;
    ਅਚਲ APInt Exponent_ਨਕਾਬ = APInt::get_ਬਿੱਟ_ਸੈੱਟ(
        FltSemantics.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ, Mantissa_ਬਿੱਟ, FltSemantics.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ - 1);
    ਅਚਲ APInt Mantissa_ਨਕਾਬ =
        APInt::get_ਨੀਵਾਂ_ਬਿੱਟ_ਸੈੱਟ(FltSemantics.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ, Mantissa_ਬਿੱਟ);

    ਅਚਲ ਤਾਰਕਿਕ Exponent_ਗਿਆਤ_All_Zeros =
        (ਬਿੱਟ_2.Zero & Exponent_ਨਕਾਬ) == Exponent_ਨਕਾਬ;
    ਅਚਲ ਤਾਰਕਿਕ Exponent_ਗਿਆਤ_All_Ones = (ਬਿੱਟ_2.One & Exponent_ਨਕਾਬ) == Exponent_ਨਕਾਬ;
    ਅਚਲ ਤਾਰਕਿਕ Exponent_ਗਿਆਤ_Not_All_Zeros = !(ਬਿੱਟ_2.One & Exponent_ਨਕਾਬ).isZero();
    ਅਚਲ ਤਾਰਕਿਕ Exponent_ਗਿਆਤ_Not_All_Ones = !(ਬਿੱਟ_2.Zero & Exponent_ਨਕਾਬ).isZero();

    ਅਚਲ ਤਾਰਕਿਕ Mantissa_ਗਿਆਤ_All_Zeros =
        (ਬਿੱਟ_2.Zero & Mantissa_ਨਕਾਬ) == Mantissa_ਨਕਾਬ;
    ਅਚਲ ਤਾਰਕਿਕ Mantissa_ਗਿਆਤ_Not_All_Zeros = !(ਬਿੱਟ_2.One & Mantissa_ਨਕਾਬ).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    ਜੇ (Exponent_ਗਿਆਤ_Not_All_Zeros)
      ਗਿਆਤ.ਗਿਆਤ_Not(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    ਜੇ (Exponent_ਗਿਆਤ_Not_All_Ones)
      ਗਿਆਤ.ਗਿਆਤ_Not(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    ਜੇ (Exponent_ਗਿਆਤ_All_Zeros || Exponent_ਗਿਆਤ_All_Ones)
      ਗਿਆਤ.ਗਿਆਤ_Not(fc_ਸਾਧਾਰਨ);

    // Zero and infinity require a mantissa with all zero bits.
    ਜੇ (Mantissa_ਗਿਆਤ_Not_All_Zeros)
      ਗਿਆਤ.ਗਿਆਤ_Not(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    ਜੇ (Mantissa_ਗਿਆਤ_All_Zeros)
      ਗਿਆਤ.ਗਿਆਤ_Not(fcSubnormal | fcNan);

    ਅਚਲ ਤਾਰਕਿਕ Quiet_ਦੋਆਧਾਰੀ_ਅੰਕ_ਗਿਆਤ_ਸੈੱਟ = ਬਿੱਟ_2.One[Mantissa_ਬਿੱਟ - 1];
    ਅਚਲ ਤਾਰਕਿਕ Quiet_ਦੋਆਧਾਰੀ_ਅੰਕ_ਗਿਆਤ_Clear = ਬਿੱਟ_2.Zero[Mantissa_ਬਿੱਟ - 1];

    ਜੇ (Quiet_ਦੋਆਧਾਰੀ_ਅੰਕ_ਗਿਆਤ_ਸੈੱਟ)
      ਗਿਆਤ.ਗਿਆਤ_Not(fcSNan);
    ਨਹੀਂਤਾਂ ਜੇ (Quiet_ਦੋਆਧਾਰੀ_ਅੰਕ_ਗਿਆਤ_Clear)
      ਗਿਆਤ.ਗਿਆਤ_Not(fcQNan);

    ਰੋਕੋ;
  }
  ਹਾਲਤ APFloat_ਅਧਾਰ::S_x87DoubleExtended: {
    // Layout of the x87 floating point type:
    // [0:62] fraction
    // [63] explicit integer bit
    // [64:78] exponent
    // [79] sign bit
    // APFloat treats the x87 pseudo encodings as documented in
    // fltSemantics::hasExplicitIntegerBit.
    constexpr ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = 80;
    constexpr ਬਿਨਾਨਿਸ਼ਾਨੀ Fraction_ਬਿੱਟ = 63;
    constexpr ਬਿਨਾਨਿਸ਼ਾਨੀ Integer_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੂਚਕਾਂਕ = Fraction_ਬਿੱਟ;
    ਅਚਲ APInt Fraction_ਨਕਾਬ = APInt::get_ਨੀਵਾਂ_ਬਿੱਟ_ਸੈੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, Fraction_ਬਿੱਟ);
    ਅਚਲ APInt Exponent_ਨਕਾਬ =
        APInt::get_ਬਿੱਟ_ਸੈੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, Fraction_ਬਿੱਟ + 1, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - 1);

    ਅਚਲ ਤਾਰਕਿਕ Exponent_ਗਿਆਤ_All_Zeros =
        (ਬਿੱਟ_2.Zero & Exponent_ਨਕਾਬ) == Exponent_ਨਕਾਬ;
    ਅਚਲ ਤਾਰਕਿਕ Exponent_ਗਿਆਤ_All_Ones = (ਬਿੱਟ_2.One & Exponent_ਨਕਾਬ) == Exponent_ਨਕਾਬ;
    ਅਚਲ ਤਾਰਕਿਕ Exponent_ਗਿਆਤ_Not_All_Zeros = !(ਬਿੱਟ_2.One & Exponent_ਨਕਾਬ).isZero();
    ਅਚਲ ਤਾਰਕਿਕ Exponent_ਗਿਆਤ_Not_All_Ones = !(ਬਿੱਟ_2.Zero & Exponent_ਨਕਾਬ).isZero();
    ਅਚਲ ਤਾਰਕਿਕ Fraction_ਗਿਆਤ_All_Zeros =
        (ਬਿੱਟ_2.Zero & Fraction_ਨਕਾਬ) == Fraction_ਨਕਾਬ;
    ਅਚਲ ਤਾਰਕਿਕ Fraction_ਗਿਆਤ_Not_All_Zeros = !(ਬਿੱਟ_2.One & Fraction_ਨਕਾਬ).isZero();
    ਅਚਲ ਤਾਰਕਿਕ Integer_ਦੋਆਧਾਰੀ_ਅੰਕ_ਗਿਆਤ_ਸੈੱਟ = ਬਿੱਟ_2.One[Integer_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੂਚਕਾਂਕ];
    ਅਚਲ ਤਾਰਕਿਕ Integer_ਦੋਆਧਾਰੀ_ਅੰਕ_ਗਿਆਤ_Clear = ਬਿੱਟ_2.Zero[Integer_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੂਚਕਾਂਕ];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    ਜੇ (Exponent_ਗਿਆਤ_Not_All_Zeros || Integer_ਦੋਆਧਾਰੀ_ਅੰਕ_ਗਿਆਤ_ਸੈੱਟ)
      ਗਿਆਤ.ਗਿਆਤ_Not(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    ਜੇ (Fraction_ਗਿਆਤ_Not_All_Zeros)
      ਗਿਆਤ.ਗਿਆਤ_Not(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    ਜੇ (Exponent_ਗਿਆਤ_Not_All_Ones || Integer_ਦੋਆਧਾਰੀ_ਅੰਕ_ਗਿਆਤ_Clear ||
        Fraction_ਗਿਆਤ_Not_All_Zeros)
      ਗਿਆਤ.ਗਿਆਤ_Not(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    ਜੇ (Integer_ਦੋਆਧਾਰੀ_ਅੰਕ_ਗਿਆਤ_Clear || Exponent_ਗਿਆਤ_All_Ones)
      ਗਿਆਤ.ਗਿਆਤ_Not(fc_ਸਾਧਾਰਨ);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    ਜੇ (Fraction_ਗਿਆਤ_All_Zeros)
      ਗਿਆਤ.ਗਿਆਤ_Not(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    ਜੇ (Exponent_ਗਿਆਤ_All_Zeros ||
        (Integer_ਦੋਆਧਾਰੀ_ਅੰਕ_ਗਿਆਤ_ਸੈੱਟ &&
         (Exponent_ਗਿਆਤ_Not_All_Ones || Fraction_ਗਿਆਤ_All_Zeros)))
      ਗਿਆਤ.ਗਿਆਤ_Not(fcNan);

    ਅਚਲ ਤਾਰਕਿਕ Quiet_ਦੋਆਧਾਰੀ_ਅੰਕ_ਗਿਆਤ_ਸੈੱਟ = ਬਿੱਟ_2.One[Fraction_ਬਿੱਟ - 1];
    ਅਚਲ ਤਾਰਕਿਕ Quiet_ਦੋਆਧਾਰੀ_ਅੰਕ_ਗਿਆਤ_Clear = ਬਿੱਟ_2.Zero[Fraction_ਬਿੱਟ - 1];

    ਜੇ (Quiet_ਦੋਆਧਾਰੀ_ਅੰਕ_ਗਿਆਤ_ਸੈੱਟ)
      ਗਿਆਤ.ਗਿਆਤ_Not(fcSNan);
    ਨਹੀਂਤਾਂ ਜੇ (Quiet_ਦੋਆਧਾਰੀ_ਅੰਕ_ਗਿਆਤ_Clear)
      ਗਿਆਤ.ਗਿਆਤ_Not(fcQNan);

    ਰੋਕੋ;
  }
  ਮੂਲ:
    llvm_unreachable("unhandled supported type");
  }

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ_FPClass::ਮੰਜ਼ਿਲ_ਗਿਆਤ_ਬਿੱਟ(ਅਚਲ fltSemantics &FltSemantics) ਅਚਲ {
  ਗਿਆਤ_ਬਿੱਟ ਗਿਆਤ(FltSemantics.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ);
  ਅਚਲ FPClassTest FPClasses = get_ਗਿਆਤ_FPClasses();

  // Return unknown if poison.
  ਜੇ (FPClasses == fcNone)
    ਵਾਪਸ ਗਿਆਤ;

  // Return unknown for types we have not validated.
  ਸਵੈਚਾਲਿਤ IsSupported = [](ਅਚਲ fltSemantics &Semantics) {
    ਚੋਣ (APFloat::Semantics_ਮੰਜ਼ਿਲ_Enum(Semantics)) {
    ਹਾਲਤ APFloat_ਅਧਾਰ::S_IEEEhalf:
    ਹਾਲਤ APFloat_ਅਧਾਰ::S_BFloat:
    ਹਾਲਤ APFloat_ਅਧਾਰ::S_IEEEsingle:
    ਹਾਲਤ APFloat_ਅਧਾਰ::S_IEEEdouble:
    ਹਾਲਤ APFloat_ਅਧਾਰ::S_IEEEquad:
    ਹਾਲਤ APFloat_ਅਧਾਰ::S_x87DoubleExtended:
      ਵਾਪਸ ਸੱਚ;
    ਮੂਲ:
      ਵਾਪਸ ਝੂਠ;
    }
  };
  ਜੇ (!IsSupported(FltSemantics))
    ਵਾਪਸ ਗਿਆਤ;

  ਚੋਣ (APFloat::Semantics_ਮੰਜ਼ਿਲ_Enum(FltSemantics)) {
  ਹਾਲਤ APFloat_ਅਧਾਰ::S_IEEEhalf:
  ਹਾਲਤ APFloat_ਅਧਾਰ::S_BFloat:
  ਹਾਲਤ APFloat_ਅਧਾਰ::S_IEEEsingle:
  ਹਾਲਤ APFloat_ਅਧਾਰ::S_IEEEdouble:
  ਹਾਲਤ APFloat_ਅਧਾਰ::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    ਜੇ (FPClasses & fc_ਸਾਧਾਰਨ)
      ਰੋਕੋ;

    ਗਿਆਤ.ਸੈੱਟ_All_Conflict();

    ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 = FltSemantics.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ;
    ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ Mantissa_ਬਿੱਟ = FltSemantics.precision - 1;
    ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ Exponent_ਬਿੱਟ = ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2 - Mantissa_ਬਿੱਟ - 1;

    APInt Mantissa_ਨਕਾਬ = APInt::get_ਨੀਵਾਂ_ਬਿੱਟ_ਸੈੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, Mantissa_ਬਿੱਟ);
    APInt Exponent_ਨਕਾਬ =
        APInt::get_ਬਿੱਟ_ਸੈੱਟ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2, Mantissa_ਬਿੱਟ, Mantissa_ਬਿੱਟ + Exponent_ਬਿੱਟ);

    ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ Quiet_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੂਚਕਾਂਕ = Mantissa_ਬਿੱਟ - 1;
    APInt Payload_ਨਕਾਬ = Mantissa_ਨਕਾਬ;
    Payload_ਨਕਾਬ.clear_ਦੋਆਧਾਰੀ_ਅੰਕ(Quiet_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੂਚਕਾਂਕ);

    ਜੇ (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      ਗਿਆਤ.Zero &= ~Exponent_ਨਕਾਬ;
      // No individual payload bit is known.
      ਗਿਆਤ.Zero &= ~Payload_ਨਕਾਬ;
      ਗਿਆਤ.One &= ~Payload_ਨਕਾਬ;

      ਜੇ (FPClasses & fcQNan)
        ਗਿਆਤ.Zero.clear_ਦੋਆਧਾਰੀ_ਅੰਕ(Quiet_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੂਚਕਾਂਕ);
      ਜੇ (FPClasses & fcSNan)
        ਗਿਆਤ.One.clear_ਦੋਆਧਾਰੀ_ਅੰਕ(Quiet_ਦੋਆਧਾਰੀ_ਅੰਕ_ਸੂਚਕਾਂਕ);
    }
    ਜੇ (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      ਗਿਆਤ.Zero &= ~Exponent_ਨਕਾਬ;
      // Mantissa bits cannot be ones.
      ਗਿਆਤ.One &= ~Mantissa_ਨਕਾਬ;
    }
    ਜੇ (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      ਗਿਆਤ.One &= ~Exponent_ਨਕਾਬ;
      // Unknown mantissa.
      ਗਿਆਤ.One &= ~Mantissa_ਨਕਾਬ;
      ਗਿਆਤ.Zero &= ~Mantissa_ਨਕਾਬ;
    }
    ਜੇ (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      ਗਿਆਤ.One &= ~Exponent_ਨਕਾਬ;
      // Mantissa cannot be ones.
      ਗਿਆਤ.One &= ~Mantissa_ਨਕਾਬ;
    }

    ਰੋਕੋ;
  }
  ਹਾਲਤ APFloat_ਅਧਾਰ::S_x87DoubleExtended: {
    ਜੇ (!is_ਗਿਆਤ_Never(fc_ਸਾਧਾਰਨ | fcSubnormal | fcNan))
      ਰੋਕੋ;

    ਗਿਆਤ.ਸੈੱਟ_All_Conflict();

    ਜੇ (FPClasses & fcInf)
      ਗਿਆਤ = ਗਿਆਤ.intersectWith(ਗਿਆਤ_ਬਿੱਟ::makeConstant(
          APFloat::getInf(FltSemantics).bitcast_ਮੰਜ਼ਿਲ_APInt()));

    ਜੇ (FPClasses & fcZero)
      ਗਿਆਤ = ਗਿਆਤ.intersectWith(
          ਗਿਆਤ_ਬਿੱਟ::makeConstant(APInt::getZero(FltSemantics.ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ)));

    ਰੋਕੋ;
  }
  ਮੂਲ:
    llvm_unreachable("unhandled supported semantics");
  }

  ਗਿਆਤ.Zero.clear_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ();
  ਗਿਆਤ.One.clear_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ();
  ਜੇ (std::optional<ਤਾਰਕਿਕ> Sign = get_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ()) {
    ਜੇ (*Sign)
      ਗਿਆਤ.makeNegative();
    ਨਹੀਂਤਾਂ
      ਗਿਆਤ.makeNonNegative();
  }

  ਵਾਪਸ ਗਿਆਤ;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
ਸਥਿਰ ਗਿਆਤ_FPClass fadd_impl(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_LHS_2,
                              ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_RHS_2, ਤਾਰਕਿਕ IsFMA) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never_Na_N() && ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never_Na_N() &&
      (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcPosInf) || ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fcNegInf)) &&
      (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcNegInf) || ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fcPosInf)))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNan);

  ਜੇ (ਗਿਆਤ_LHS_2.cannotBeOrderedLessThanZero() &&
      ਗਿਆਤ_RHS_2.cannotBeOrderedLessThanZero()) {
    ਗਿਆਤ.ਗਿਆਤ_Not(ਗਿਆਤ_FPClass::Ordered_Less_Than_Zero_ਨਕਾਬ);

    // This can't underflow if one of the operands is known normal.
    ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcZero | fcPosSubnormal) ||
        ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fcZero | fcPosSubnormal))
      ਗਿਆਤ.ਗਿਆਤ_Not(fcZero | fcPosSubnormal);
  }

  ਜੇ (ਗਿਆਤ_LHS_2.cannotBeOrderedGreaterThanZero() &&
      ਗਿਆਤ_RHS_2.cannotBeOrderedGreaterThanZero()) {
    ਗਿਆਤ.ਗਿਆਤ_Not(ਗਿਆਤ_FPClass::Ordered_Greater_Than_Zero_ਨਕਾਬ);

    // This can't underflow if one of the operands is known normal.
    ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcZero | fcNegSubnormal) ||
        ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fcZero | fcNegSubnormal))
      ਗਿਆਤ.ਗਿਆਤ_Not(fcZero | fcNegSubnormal);
  }

  ਜੇ (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcNegZero) || ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fcNegZero))
      ਗਿਆਤ.ਗਿਆਤ_Not(fcNegZero);

    ਗਿਆਤ.propagateNonSNaN(ਗਿਆਤ_LHS_2, ਗਿਆਤ_RHS_2);
  }

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::fadd(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_LHS,
                                ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_RHS,
                                Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ_LHS_2 = apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(ਗਿਆਤ_LHS, ਬਹੁਲਕ);
  ਗਿਆਤ_FPClass ਗਿਆਤ_RHS_2 = apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(ਗਿਆਤ_RHS, ਬਹੁਲਕ);

  ਗਿਆਤ_FPClass ਗਿਆਤ = fadd_impl(ਗਿਆਤ_LHS_2, ਗਿਆਤ_RHS_2, /*IsFMA=*/ਝੂਠ);

  ਵਾਪਸ apply_Output_Denormal_ਬਹੁਲਕ(ਗਿਆਤ, ਬਹੁਲਕ);
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::fadd_self(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src_2,
                                     Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ_Src = apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(ਗਿਆਤ_Src_2, ਬਹੁਲਕ);

  ਗਿਆਤ_FPClass ਗਿਆਤ = fadd_impl(ਗਿਆਤ_Src, ਗਿਆਤ_Src, /*IsFMA=*/ਝੂਠ);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcPosSubnormal))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosSubnormal);
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcNegSubnormal))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcPosZero))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosZero);

  ਵਾਪਸ apply_Output_Denormal_ਬਹੁਲਕ(ਗਿਆਤ, ਬਹੁਲਕ);
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::fsub(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_LHS,
                                ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_RHS,
                                Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ_LHS_2 = apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(ਗਿਆਤ_LHS, ਬਹੁਲਕ);
  ਗਿਆਤ_FPClass ਗਿਆਤ_RHS_2 = apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(ਗਿਆਤ_RHS, ਬਹੁਲਕ);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  ਗਿਆਤ_RHS_2.fneg();

  ਗਿਆਤ_FPClass ਗਿਆਤ = fadd_impl(ਗਿਆਤ_LHS_2, ਗਿਆਤ_RHS_2, /*IsFMA=*/ਝੂਠ);

  ਵਾਪਸ apply_Output_Denormal_ਬਹੁਲਕ(ਗਿਆਤ, ਬਹੁਲਕ);
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::fmul(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_LHS,
                                ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_RHS,
                                Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ_LHS_2 = apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(ਗਿਆਤ_LHS, ਬਹੁਲਕ);
  ਗਿਆਤ_FPClass ਗਿਆਤ_RHS_2 = apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(ਗਿਆਤ_RHS, ਬਹੁਲਕ);

  ਗਿਆਤ_FPClass ਗਿਆਤ;

  ਗਿਆਤ.propagateNonSNaN(ਗਿਆਤ_LHS_2, ਗਿਆਤ_RHS_2);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  ਜੇ ((ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcNegative) &&
       ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fcNegative)) ||
      (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fc_ਸਕਾਰਾਤਮਕ) && ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fc_ਸਕਾਰਾਤਮਕ)))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegative);
  ਜੇ ((ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fc_ਸਕਾਰਾਤਮਕ) &&
       ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fcNegative)) ||
      (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcNegative) && ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fc_ਸਕਾਰਾਤਮਕ)))
    ਗਿਆਤ.ਗਿਆਤ_Not(fc_ਸਕਾਰਾਤਮਕ);

  // Inf * Y => Inf or NaN
  ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Always(fcInf | fcNan) ||
      ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Always(fcInf | fcNan))
    ਗਿਆਤ.ਗਿਆਤ_Not(fc_ਸਾਧਾਰਨ | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  ਜੇ (ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Always(fcZero | fcNan) ||
      ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Always(fcZero | fcNan))
    ਗਿਆਤ.ਗਿਆਤ_Not(fc_ਸਾਧਾਰਨ | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never_Na_N() && ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never_Na_N() &&
      (ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never_Infinity() || ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never_Zero()) &&
      (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never_Infinity() || ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never_Zero()))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNan);

  ਵਾਪਸ apply_Output_Denormal_ਬਹੁਲਕ(ਗਿਆਤ, ਬਹੁਲਕ);
}

// TODO: This generalizes to known ranges
ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::fmul(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_LHS_2,
                                ਅਚਲ APFloat &CRHS, Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  ਅਚਲ fltSemantics &Flt = CRHS.getSemantics();
  ਬਿਨਾਨਿਸ਼ਾਨੀ Precision = APFloat::semanticsPrecision(Flt);
  ਅਚਲ ਪੂਰਨ Mantissa_ਬਿੱਟ = Precision - 1;

  ਪੂਰਨ Min_ਗਿਆਤ_Exponent = ilogb(CRHS);
  ਤਾਰਕਿਕ CannotBeSubnormal = (Min_ਗਿਆਤ_Exponent >= Mantissa_ਬਿੱਟ);

  ਗਿਆਤ_FPClass ਗਿਆਤ = ਗਿਆਤ_FPClass::fmul(ਗਿਆਤ_LHS_2, ਗਿਆਤ_FPClass(CRHS), ਬਹੁਲਕ);
  ਜੇ (CannotBeSubnormal)
    ਗਿਆਤ.ਗਿਆਤ_Not(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcInf)) {
    ਜੇ (Min_ਗਿਆਤ_Exponent < 0)
      ਗਿਆਤ.ਗਿਆਤ_Not(fcInf);
    ਨਹੀਂਤਾਂ ਜੇ (Min_ਗਿਆਤ_Exponent == 0 && CRHS.compare_Absolute_ਮੁੱਲ(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      ਗਿਆਤ.ਗਿਆਤ_Not(fcInf);
  }

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::fdiv(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_LHS,
                                ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_RHS,
                                Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ_LHS_2 = apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(ਗਿਆਤ_LHS, ਬਹੁਲਕ);
  ਗਿਆਤ_FPClass ਗਿਆਤ_RHS_2 = apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(ਗਿਆਤ_RHS, ਬਹੁਲਕ);

  ਗਿਆਤ_FPClass ਗਿਆਤ;

  ਗਿਆਤ.propagateNonSNaN(ਗਿਆਤ_LHS_2, ਗਿਆਤ_RHS_2);

  // Only 0/0, Inf/Inf produce NaN.
  ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never_Na_N() && ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never_Na_N() &&
      (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never_Infinity() || ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never_Infinity()) &&
      (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never_Zero() || ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never_Zero())) {
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  ਜੇ ((ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcNegative) &&
       ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fcNegative)) ||
      (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fc_ਸਕਾਰਾਤਮਕ) && ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fc_ਸਕਾਰਾਤਮਕ)))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegative);
  ਜੇ ((ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fc_ਸਕਾਰਾਤਮਕ) &&
       ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fcNegative)) ||
      (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcNegative) && ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fc_ਸਕਾਰਾਤਮਕ)))
    ਗਿਆਤ.ਗਿਆਤ_Not(fc_ਸਕਾਰਾਤਮਕ);

  // Normal and subnormal results require two non-zero finite operands.
  ਜੇ ((ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal) &&
       ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal)) ||
      (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fc_Pos_ਸਾਧਾਰਨ | fcPosSubnormal) &&
       ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fc_Pos_ਸਾਧਾਰਨ | fcPosSubnormal)))
    ਗਿਆਤ.ਗਿਆਤ_Not(fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal);
  ਜੇ ((ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal) &&
       ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fc_Pos_ਸਾਧਾਰਨ | fcPosSubnormal)) ||
      (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fc_Pos_ਸਾਧਾਰਨ | fcPosSubnormal) &&
       ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal)))
    ਗਿਆਤ.ਗਿਆਤ_Not(fc_Pos_ਸਾਧਾਰਨ | fcPosSubnormal);

  // 0 / X => 0 or NaN
  ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Always(fcZero))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcSubnormal | fc_ਸਾਧਾਰਨ | fcInf);

  // X / 0 => NaN or Inf
  ਜੇ (ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Always(fcZero))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcFinite);

  ਵਾਪਸ apply_Output_Denormal_ਬਹੁਲਕ(ਗਿਆਤ, ਬਹੁਲਕ);
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::fdiv_self(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src,
                                     Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  // X / X is always exactly 1.0 or a NaN.
  ਗਿਆਤ_FPClass ਗਿਆਤ(fcNan | fc_Pos_ਸਾਧਾਰਨ);

  ਗਿਆਤ.propagateNonSNaN(ਗਿਆਤ_Src);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Inf_Or_Na_N() && ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Logical_Zero(ਬਹੁਲਕ))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNan);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::frem(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_LHS_2,
                                ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_RHS_2,
                                Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  ਗਿਆਤ.ਗਿਆਤ_Not(fcInf);

  ਗਿਆਤ.propagateNonSNaN(ਗਿਆਤ_LHS_2, ਗਿਆਤ_RHS_2);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never_Inf_Or_Na_N() && ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never_Na_N() &&
      ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never_Logical_Zero(ਬਹੁਲਕ)) {
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNan);
  }

  // The sign for frem is the same as the first operand.
  ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fc_Pos_ਸਾਧਾਰਨ | fcPosSubnormal))
    ਗਿਆਤ.ਗਿਆਤ_Not(fc_Pos_ਸਾਧਾਰਨ | fcPosSubnormal);
  ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal))
    ਗਿਆਤ.ਗਿਆਤ_Not(fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcNegFinite))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcPosFinite) &&
      ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never_Logical_Pos_Zero(ਬਹੁਲਕ) &&
      (!ਬਹੁਲਕ.outputs_May_Be_ਸਕਾਰਾਤਮਕ_Zero() ||
       ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal)))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosZero);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::frem_self(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src,
                                     Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  // X % X is always exactly [+-]0.0 or a NaN.
  ਗਿਆਤ_FPClass ਗਿਆਤ(fcNan | fcZero);

  ਗਿਆਤ.propagateNonSNaN(ਗਿਆਤ_Src);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Inf_Or_Na_N() && ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Logical_Zero(ਬਹੁਲਕ))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fc_Pos_ਸਾਧਾਰਨ | fcPosSubnormal))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosZero);
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegZero);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::fma(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_LHS_2,
                               ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_RHS_2,
                               ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Addend,
                               Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass Mul = fmul(ਗਿਆਤ_LHS_2, ਗਿਆਤ_RHS_2, ਬਹੁਲਕ);

  // FMA differs from the base fmul + fadd handling only in the treatment of -0
  // results.
  //
  // If the multiply is a -0 due to rounding, the final -0 + 0 will be -0,
  // unlike for a separate fadd.
  ਗਿਆਤ_FPClass ਗਿਆਤ = fadd_impl(Mul, ਗਿਆਤ_Addend, /*IsFMA=*/ਸੱਚ);

  // propagateNonSNaN for 3 arguments.
  ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcSNan) && ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fcSNan) &&
      ਗਿਆਤ_Addend.is_ਗਿਆਤ_Never(fcSNan))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcSNan);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::fmuladd(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_LHS_2,
                                   ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_RHS_2,
                                   ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Addend,
                                   Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  ਗਿਆਤ_FPClass FMA = fma(ਗਿਆਤ_LHS_2, ਗਿਆਤ_RHS_2, ਗਿਆਤ_Addend, ਬਹੁਲਕ);
  ਗਿਆਤ_FPClass Mul_ਜੋੜੋ = fadd(fmul(ਗਿਆਤ_LHS_2, ਗਿਆਤ_RHS_2, ਬਹੁਲਕ), ਗਿਆਤ_Addend, ਬਹੁਲਕ);
  ਵਾਪਸ FMA.intersectWith(Mul_ਜੋੜੋ);
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::fma_ਵਰਗਫਲ(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Squared,
                                      ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Addend,
                                      Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass Squared = ਵਰਗਫਲ(ਗਿਆਤ_Squared, ਬਹੁਲਕ);
  ਗਿਆਤ_FPClass ਗਿਆਤ = fadd_impl(Squared, ਗਿਆਤ_Addend, /*IsFMA=*/ਸੱਚ);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  ਜੇ (ਗਿਆਤ_Addend.is_ਗਿਆਤ_Never(fcNegInf | fcNan) && Squared.is_ਗਿਆਤ_Never(fcNan))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNan);

  ਗਿਆਤ.propagateNonSNaN(ਗਿਆਤ_Squared, ਗਿਆਤ_Addend);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::fmuladd_ਵਰਗਫਲ(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Squared,
                                          ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Addend,
                                          Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass FMA = fma_ਵਰਗਫਲ(ਗਿਆਤ_Squared, ਗਿਆਤ_Addend, ਬਹੁਲਕ);
  ਗਿਆਤ_FPClass Mul_ਜੋੜੋ = fadd(ਵਰਗਫਲ(ਗਿਆਤ_Squared, ਬਹੁਲਕ), ਗਿਆਤ_Addend, ਬਹੁਲਕ);
  ਵਾਪਸ FMA.intersectWith(Mul_ਜੋੜੋ);
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::ਘਾਤਾਂਕ(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;
  ਗਿਆਤ.ਗਿਆਤ_Not(fcNegative);

  ਗਿਆਤ.propagateNonNaN(ਗਿਆਤ_Src);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fc_Neg_ਸਾਧਾਰਨ | fcNegInf))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fc_Neg_ਸਾਧਾਰਨ))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fc_Pos_ਸਾਧਾਰਨ | fcPosInf))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosInf);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::ਲਘੂਗਣਕ(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src, Denormal_ਬਹੁਲਕ ਬਹੁਲਕ,
                               ਤਾਰਕਿਕ IsMultiUnitFPType) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  ਜੇ (!IsMultiUnitFPType)
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegZero | fcSubnormal);

  ਗਿਆਤ.propagateNonSNaN(ਗਿਆਤ_Src);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Pos_Infinity())
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosInf);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Na_N() && ਗਿਆਤ_Src.cannotBeOrderedLessThanZero())
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNan);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Logical_Zero(ਬਹੁਲਕ))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegInf);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::ਵਰਗਮੂਲ(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src,
                                Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;
  ਗਿਆਤ.ਗਿਆਤ_Not(fcPosSubnormal);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Pos_Infinity())
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosInf);

  ਗਿਆਤ.propagateNonSNaN(ਗਿਆਤ_Src);

  // Any negative value besides -0 returns a nan.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Na_N() && ਗਿਆਤ_Src.cannotBeOrderedLessThanZero())
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  ਗਿਆਤ.ਗਿਆਤ_Not(fcNegInf | fcNegSubnormal | fc_Neg_ਸਾਧਾਰਨ);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Logical_Neg_Zero(ਬਹੁਲਕ))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegZero);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::ਜੀਵਾ(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  ਗਿਆਤ.propagateNonSNaN(ਗਿਆਤ_Src);

  // Return NaN on infinite inputs.
  ਗਿਆਤ.ਗਿਆਤ_Not(fcInf);
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Na_N() && ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Infinity())
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNan);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::ਕੋਜੀਵਾ(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src) {
  ਵਾਪਸ ਜੀਵਾ(ਗਿਆਤ_Src);
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::tan(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  ਗਿਆਤ.ਗਿਆਤ_Not(fcInf);

  ਗਿਆਤ.propagateNonSNaN(ਗਿਆਤ_Src);

  // NaN propagates. tan(+-Inf) is NaN.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Na_N() && ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Infinity())
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNan);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::sinh(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcNegative))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcPosSubnormal | fc_Pos_ਸਾਧਾਰਨ | fcPosInf))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosSubnormal | fc_Pos_ਸਾਧਾਰਨ | fcPosInf);

  ਗਿਆਤ.propagateNonNaN(ਗਿਆਤ_Src);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::cosh(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  ਗਿਆਤ.ਗਿਆਤ_Not(fcNegative | fcZero | fcSubnormal);

  ਗਿਆਤ.propagateNonNaN(ਗਿਆਤ_Src);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::tanh(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  // tanh is bounded to (-1, 1), never Inf.
  ਗਿਆਤ.ਗਿਆਤ_Not(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcNegSubnormal | fc_Neg_ਸਾਧਾਰਨ | fcNegInf)) {
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegSubnormal | fc_Neg_ਸਾਧਾਰਨ);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Neg_Zero())
      ਗਿਆਤ.ਗਿਆਤ_Not(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcPosSubnormal | fc_Pos_ਸਾਧਾਰਨ | fcPosInf))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosSubnormal | fc_Pos_ਸਾਧਾਰਨ);

  ਗਿਆਤ.propagateNonNaN(ਗਿਆਤ_Src);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::asin(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  ਗਿਆਤ.ਗਿਆਤ_Not(fcInf);

  ਗਿਆਤ.propagateNonSNaN(ਗਿਆਤ_Src);

  // asin is sign-preserving for finite arguments.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcNegFinite))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegFinite);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcPosSubnormal | fc_Pos_ਸਾਧਾਰਨ))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosSubnormal | fc_Pos_ਸਾਧਾਰਨ);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::acos(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  // acos(x) is bounded to [0, pi] for -1 <= x <= 1, and is never negative,
  // infinite, or subnormal. The smallest non-zero value occurs when x is
  // close to 1.0, where acos(x) can be approximated by sqrt(2 * (1 - x)).
  // Since sqrt cannot produce a subnormal result, we can conclude that
  // acos(x) will also never produce a subnormal result.
  ਗਿਆਤ.ਗਿਆਤ_Not(fcNegative | fcInf | fcSubnormal);

  // acos(x) == +0.0 iff x == +1.0
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fc_Pos_ਸਾਧਾਰਨ))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcZero);

  ਗਿਆਤ.propagateNonSNaN(ਗਿਆਤ_Src);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::atan(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  ਗਿਆਤ.ਗਿਆਤ_Not(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcNegSubnormal | fc_Neg_ਸਾਧਾਰਨ | fcNegInf))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegSubnormal | fc_Neg_ਸਾਧਾਰਨ);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcPosSubnormal | fc_Pos_ਸਾਧਾਰਨ | fcPosInf))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosSubnormal | fc_Pos_ਸਾਧਾਰਨ);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal | fcNegZero))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegZero);

  ਗਿਆਤ.propagateNonNaN(ਗਿਆਤ_Src);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::atan2(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Y,
                                 ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_X,
                                 Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ_Y_2 = apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(ਗਿਆਤ_Y, ਬਹੁਲਕ);
  ਗਿਆਤ_FPClass ਗਿਆਤ_X_2 = apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(ਗਿਆਤ_X, ਬਹੁਲਕ);
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  // Even though these deductions are correct, we are ignoring the following
  // potentially erroneous cases:
  //   * atan2(y, inf) is not subnormal
  //   * atan2(inf, x) is not zero or subnormal

  // atan2 result is in (-pi, pi], never Inf.
  ਗਿਆਤ.ਗਿਆਤ_Not(fcInf);

  ਗਿਆਤ.propagateNonNaN(ਗਿਆਤ_Y_2, ਗਿਆਤ_X_2);

  // If x <= -0.0, then |atan2(y, x)| >= pi/2
  ਜੇ (ਗਿਆਤ_X_2.is_ਗਿਆਤ_Never(fc_ਸਕਾਰਾਤਮਕ))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  ਜੇ (ਗਿਆਤ_Y_2.is_ਗਿਆਤ_Never(fcNegative))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  ਜੇ (ਗਿਆਤ_Y_2.is_ਗਿਆਤ_Never(fc_ਸਕਾਰਾਤਮਕ))
    ਗਿਆਤ.ਗਿਆਤ_Not(fc_ਸਕਾਰਾਤਮਕ);

  ਵਾਪਸ apply_Output_Denormal_ਬਹੁਲਕ(ਗਿਆਤ, ਬਹੁਲਕ);
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::fpext(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src,
                                 ਅਚਲ fltSemantics &DstTy,
                                 ਅਚਲ fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  ਗਿਆਤ_FPClass ਗਿਆਤ = ਗਿਆਤ_Src;

  // All subnormal inputs should be in the normal range in the result type.
  ਜੇ (APFloat::is_Representable_As_ਸਾਧਾਰਨ_ਵਿੱਚ(SrcTy, DstTy)) {
    ਜੇ (ਗਿਆਤ.get_ਗਿਆਤ_FPClasses() & fcPosSubnormal)
      ਗਿਆਤ.ਸੈੱਟ_ਗਿਆਤ_FPClasses(ਗਿਆਤ.get_ਗਿਆਤ_FPClasses() | fc_Pos_ਸਾਧਾਰਨ);
    ਜੇ (ਗਿਆਤ.get_ਗਿਆਤ_FPClasses() & fcNegSubnormal)
      ਗਿਆਤ.ਸੈੱਟ_ਗਿਆਤ_FPClasses(ਗਿਆਤ.get_ਗਿਆਤ_FPClasses() | fc_Neg_ਸਾਧਾਰਨ);
    ਗਿਆਤ.ਗਿਆਤ_Not(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  ਜੇ (!ਗਿਆਤ.is_ਗਿਆਤ_Never_Na_N())
    ਗਿਆਤ.ਸੈੱਟ_Sign_ਦੋਆਧਾਰੀ_ਅੰਕ(std::nullopt);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::fptrunc(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  ਜੇ (ਗਿਆਤ_Src.cannotBeOrderedLessThanZero())
    ਗਿਆਤ.ਗਿਆਤ_Not(ਗਿਆਤ_FPClass::Ordered_Less_Than_Zero_ਨਕਾਬ);

  ਗਿਆਤ.propagateNonNaN(ਗਿਆਤ_Src);

  // Infinity needs a range check.
  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::ਗੇੜਾ_ਮੰਜ਼ਿਲ_Integral(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src,
                                           ਤਾਰਕਿਕ MayBeMultiUnitFPType,
                                           Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  // Integer results cannot be subnormal.
  ਗਿਆਤ.ਗਿਆਤ_Not(fcSubnormal);

  ਗਿਆਤ.propagateNonNaN(ਗਿਆਤ_Src);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  ਜੇ (!MayBeMultiUnitFPType) {
    ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Pos_Infinity())
      ਗਿਆਤ.ਗਿਆਤ_Not(fcPosInf);
    ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Neg_Infinity())
      ਗਿਆਤ.ਗਿਆਤ_Not(fcNegInf);
  }

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fc_Pos_ਸਾਧਾਰਨ | fcPosSubnormal))
    ਗਿਆਤ.ਗਿਆਤ_Not(fc_Pos_ਸਾਧਾਰਨ);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal))
    ਗਿਆਤ.ਗਿਆਤ_Not(fc_Neg_ਸਾਧਾਰਨ);

  // Negative round ups towards zero produce negative zero.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcNegFinite))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegZero);

  // Negative subnormals may flush to positive zero.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcPosFinite) &&
      (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcNegSubnormal) ||
       !ਬਹੁਲਕ.inputs_May_Be_ਸਕਾਰਾਤਮਕ_Zero()))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosZero);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::trunc(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src,
                                 Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ = ਗੇੜਾ_ਮੰਜ਼ਿਲ_Integral(ਗਿਆਤ_Src,
                                       /*MayBeMultiUnitFPType=*/ਸੱਚ, ਬਹੁਲਕ);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Pos_Infinity())
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosInf);
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Neg_Infinity())
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegInf);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::floor(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src,
                                 ਤਾਰਕਿਕ IsMultiUnitFPType, Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ = ਗੇੜਾ_ਮੰਜ਼ਿਲ_Integral(ਗਿਆਤ_Src, IsMultiUnitFPType, ਬਹੁਲਕ);

  // Applicable for multi-unit floating point types.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Pos_Infinity())
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosInf);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::ceil(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src,
                                ਤਾਰਕਿਕ IsMultiUnitFPType, Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ = ਗੇੜਾ_ਮੰਜ਼ਿਲ_Integral(ਗਿਆਤ_Src, IsMultiUnitFPType, ਬਹੁਲਕ);

  // Applicable for multi-unit floating point types.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Neg_Infinity())
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegInf);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::frexp_mant(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src,
                                      Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  ਗਿਆਤ.ਗਿਆਤ_Not(fcSubnormal);

  ਗਿਆਤ.propagateNonNaN(ਗਿਆਤ_Src);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Pos_Infinity())
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosInf);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Neg_Infinity())
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegInf);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Logical_Pos_Zero(ਬਹੁਲਕ))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcPosZero);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Logical_Neg_Zero(ਬਹੁਲਕ))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fc_Pos_ਸਾਧਾਰਨ | fcPosSubnormal))
    ਗਿਆਤ.ਗਿਆਤ_Not(fc_Pos_ਸਾਧਾਰਨ);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal))
    ਗਿਆਤ.ਗਿਆਤ_Not(fc_Neg_ਸਾਧਾਰਨ);

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::ldexp(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src_2,
                                 ਅਚਲ APInt &ConstantRangeExpMin,
                                 ਅਚਲ APInt &ConstantRangeExpMax,
                                 ਅਚਲ fltSemantics &Flt, Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  ਗਿਆਤ_FPClass ਗਿਆਤ_Src = apply_ਨਿਵੇਸ਼_Denormal_ਬਹੁਲਕ(ਗਿਆਤ_Src_2, ਬਹੁਲਕ);

  ਜੇ (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    ਗਿਆਤ.ਸੈੱਟ_ਗਿਆਤ_FPClasses(ਗਿਆਤ_Src.get_ਗਿਆਤ_FPClasses() | fcNan);
    ਗਿਆਤ.propagateNonNaN(ਗਿਆਤ_Src);
    ਵਾਪਸ apply_Output_Denormal_ਬਹੁਲਕ(ਗਿਆਤ, ਬਹੁਲਕ);
  }

  ਗਿਆਤ.propagateNonNaN(ਗਿਆਤ_Src);

  // Sign is preserved, but underflows may produce zeroes.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcNegative))
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegative);
  ਨਹੀਂਤਾਂ ਜੇ (ਗਿਆਤ_Src.cannotBeOrderedLessThanZero())
    ਗਿਆਤ.ਗਿਆਤ_Not(Ordered_Less_Than_Zero_ਨਕਾਬ);

  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fc_ਸਕਾਰਾਤਮਕ))
    ਗਿਆਤ.ਗਿਆਤ_Not(fc_ਸਕਾਰਾਤਮਕ);
  ਨਹੀਂਤਾਂ ਜੇ (ਗਿਆਤ_Src.cannotBeOrderedGreaterThanZero())
    ਗਿਆਤ.ਗਿਆਤ_Not(Ordered_Greater_Than_Zero_ਨਕਾਬ);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  ਜੇ (&Flt != &APFloat::PPCDoubleDouble()) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ Precision = APFloat::semanticsPrecision(Flt);
    ਅਚਲ ਪੂਰਨ Mantissa_ਬਿੱਟ = Precision - 1;
    ਜੇ (ConstantRangeExpMin.sge(Mantissa_ਬਿੱਟ))
      ਗਿਆਤ.ਗਿਆਤ_Not(fcSubnormal);
  }

  ਜੇ (ConstantRangeExpMax.is_Non_ਸਕਾਰਾਤਮਕ()) {
    // If we know the power is <= 0, can't introduce inf
    ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Pos_Infinity())
      ਗਿਆਤ.ਗਿਆਤ_Not(fcPosInf);
    ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Neg_Infinity())
      ਗਿਆਤ.ਗਿਆਤ_Not(fcNegInf);
  } ਨਹੀਂਤਾਂ ਜੇ (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Pos_Subnormal())
      ਗਿਆਤ.ਗਿਆਤ_Not(fcPosSubnormal);
    ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Neg_Subnormal())
      ਗਿਆਤ.ਗਿਆਤ_Not(fcNegSubnormal);
    ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Pos_Zero())
      ਗਿਆਤ.ਗਿਆਤ_Not(fcPosZero);
    ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never_Neg_Zero())
      ਗਿਆਤ.ਗਿਆਤ_Not(fcNegZero);
  }

  ਵਾਪਸ apply_Output_Denormal_ਬਹੁਲਕ(ਗਿਆਤ, ਬਹੁਲਕ);
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::ldexp(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src,
                                 ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &Exp_ਬਿੱਟ,
                                 ਅਚਲ fltSemantics &Flt, Denormal_ਬਹੁਲਕ ਬਹੁਲਕ) {
  ਵਾਪਸ ldexp(ਗਿਆਤ_Src, Exp_ਬਿੱਟ.get_Signed_Min_ਮੁੱਲ(),
               Exp_ਬਿੱਟ.get_Signed_Max_ਮੁੱਲ(), Flt, ਬਹੁਲਕ);
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::ਘਾਤ(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_LHS_2,
                               ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_RHS_2) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;

  ਗਿਆਤ.propagateNonSNaN(ਗਿਆਤ_LHS_2, ਗਿਆਤ_RHS_2);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcNan | fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal) &&
      ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never_Na_N())
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcNegative)) {
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegative);
  } ਨਹੀਂਤਾਂ ਜੇ (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal)) {
    ਗਿਆਤ.ਗਿਆਤ_Not(fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    ਜੇ ((ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcNegZero) ||
         ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fc_Pos_ਸਾਧਾਰਨ)) &&
        (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcNegInf) || ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fc_Neg_ਸਾਧਾਰਨ)))
      ਗਿਆਤ.ਗਿਆਤ_Not(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    ਜੇ ((ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcNegZero) ||
         ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fc_Neg_ਸਾਧਾਰਨ)) &&
        (ਗਿਆਤ_LHS_2.is_ਗਿਆਤ_Never(fcNegInf) || ਗਿਆਤ_RHS_2.is_ਗਿਆਤ_Never(fc_Pos_ਸਾਧਾਰਨ)))
      ਗਿਆਤ.ਗਿਆਤ_Not(fcNegInf);
  }

  ਵਾਪਸ ਗਿਆਤ;
}

ਗਿਆਤ_FPClass ਗਿਆਤ_FPClass::powi(ਅਚਲ ਗਿਆਤ_FPClass &ਗਿਆਤ_Src,
                                ਅਚਲ ਗਿਆਤ_ਬਿੱਟ &Exponent_ਗਿਆਤ_ਬਿੱਟ) {
  ਗਿਆਤ_FPClass ਗਿਆਤ;
  ਗਿਆਤ.propagateNonNaN(ਗਿਆਤ_Src);

  ਜੇ (Exponent_ਗਿਆਤ_ਬਿੱਟ.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcSNan)) {
      ਗਿਆਤ.ਗਿਆਤ_Not(~fc_Pos_ਸਾਧਾਰਨ);
      ਵਾਪਸ ਗਿਆਤ;
    }

    ਗਿਆਤ.ਗਿਆਤ_Not(~(fc_Pos_ਸਾਧਾਰਨ | fcNan));
    ਵਾਪਸ ਗਿਆਤ;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcNegative) || Exponent_ਗਿਆਤ_ਬਿੱਟ.isEven()) {
    ਗਿਆਤ.ਗਿਆਤ_Not(fcNegative);
  } ਨਹੀਂਤਾਂ ਜੇ (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal)) {
    ਗਿਆਤ.ਗਿਆਤ_Not(fc_Neg_ਸਾਧਾਰਨ | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    ਅਚਲ ਤਾਰਕਿਕ Is_ਗਿਆਤ_Never_Odd_ਸਕਾਰਾਤਮਕ = Exponent_ਗਿਆਤ_ਬਿੱਟ.isNegative();
    ਅਚਲ ਤਾਰਕਿਕ Is_ਗਿਆਤ_Never_Odd_Negative = Exponent_ਗਿਆਤ_ਬਿੱਟ.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    ਜੇ ((ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcNegZero) || Is_ਗਿਆਤ_Never_Odd_ਸਕਾਰਾਤਮਕ) &&
        (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcNegInf) || Is_ਗਿਆਤ_Never_Odd_Negative))
      ਗਿਆਤ.ਗਿਆਤ_Not(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    ਜੇ ((ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcNegZero) || Is_ਗਿਆਤ_Never_Odd_Negative) &&
        (ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcNegInf) || Is_ਗਿਆਤ_Never_Odd_ਸਕਾਰਾਤਮਕ))
      ਗਿਆਤ.ਗਿਆਤ_Not(fcNegInf);
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
    APInt MinExp = Exponent_ਗਿਆਤ_ਬਿੱਟ.get_Signed_Min_ਮੁੱਲ();
    APInt MaxExp = Exponent_ਗਿਆਤ_ਬਿੱਟ.get_Signed_Max_ਮੁੱਲ();

    // powi(inf, exp), exp > 0
    ਤਾਰਕਿਕ MayInfSrc =
        !ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcInf) && MaxExp.is_Strictly_ਸਕਾਰਾਤਮਕ();

    // powi(+/-0, exp), exp < 0
    ਤਾਰਕਿਕ MayDivByZero = !ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcZero) && MinExp.isNegative();

    // powi(finite, exp), |exp| > 1
    ਤਾਰਕਿਕ MayFinite = !ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fc_ਸਾਧਾਰਨ | fcSubnormal);
    ਤਾਰਕਿਕ MayAbsExpGT1 = MinExp.slt(-1) || MaxExp.sgt(1);
    ਤਾਰਕਿਕ May_Finite_ਉਛਾਲ = MayFinite && MayAbsExpGT1;

    // powi(subnormal, -1)
    ਤਾਰਕਿਕ MayBeNegOne = Exponent_ਗਿਆਤ_ਬਿੱਟ.Zero.isZero();
    ਤਾਰਕਿਕ MaySubnormInv = !ਗਿਆਤ_Src.is_ਗਿਆਤ_Never(fcSubnormal) && MayBeNegOne;

    ਜੇ (!MayInfSrc && !MayDivByZero && !May_Finite_ਉਛਾਲ && !MaySubnormInv)
      ਗਿਆਤ.ਗਿਆਤ_Not(fcInf);
  }

  ਵਾਪਸ ਗਿਆਤ;
}
