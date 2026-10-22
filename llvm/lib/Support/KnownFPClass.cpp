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

#ضمن "llvm/Support/KnownFPClass.h"
#ضمن "llvm/ADT/APFloat.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/KnownBits.h"

کارول نطاق llvm;

معلوم_FPClass::معلوم_FPClass(ثابت APFloat &C)
    : معلوم_FPClasses_ارزښت(C.classify()) {
  سېټ_Sign_بټ(C.isNegative());
}

معلوم_FPClass معلوم_FPClass::apply_ننوت_Denormal_ډېرپېښ(ثابت معلوم_FPClass &معلوم_Src,
                                                  Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم = معلوم_Src;
  بدل (ډېرپېښ.ننوت) {
  حالة Denormal_ډېرپېښ::IEEE:
    أرجع معلوم;
  حالة Denormal_ډېرپېښ::PreserveSign:
    إذا (معلوم_Src.get_معلوم_FPClasses() & fcPosSubnormal)
      معلوم.سېټ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcPosZero);
    إذا (معلوم_Src.get_معلوم_FPClasses() & fcNegSubnormal)
      معلوم.سېټ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcNegZero);
    أرجع معلوم;
  حالة Denormal_ډېرپېښ::مثبت_Zero:
    إذا (معلوم_Src.get_معلوم_FPClasses() & fcSubnormal) {
      معلوم.سېټ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcPosZero);
      إذا (معلوم_Src.get_معلوم_FPClasses() & fcNegSubnormal)
        معلوم.سېټ_Sign_بټ(std::nullopt);
    }
    أرجع معلوم;
  افتراضي:
    إذا (معلوم_Src.get_معلوم_FPClasses() & fcSubnormal) {
      معلوم.سېټ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcPosZero);
      إذا (معلوم_Src.get_معلوم_FPClasses() & fcNegSubnormal)
        معلوم.سېټ_Sign_بټ(std::nullopt);
    }
    إذا (معلوم_Src.get_معلوم_FPClasses() & fcNegSubnormal)
      معلوم.سېټ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcNegZero);
    أرجع معلوم;
  }

  llvm_unreachable("covered switch over denormal mode");
}

معلوم_FPClass معلوم_FPClass::apply_Output_Denormal_ډېرپېښ(ثابت معلوم_FPClass &معلوم_Src,
                                                   Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم = معلوم_Src;
  بدل (ډېرپېښ.Output) {
  حالة Denormal_ډېرپېښ::IEEE:
    أرجع معلوم;
  حالة Denormal_ډېرپېښ::PreserveSign:
    إذا (معلوم_Src.get_معلوم_FPClasses() & fcPosSubnormal)
      معلوم.سېټ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcPosZero);
    إذا (معلوم_Src.get_معلوم_FPClasses() & fcNegSubnormal)
      معلوم.سېټ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcNegZero);
    أرجع معلوم;
  حالة Denormal_ډېرپېښ::مثبت_Zero:
    إذا (معلوم_Src.get_معلوم_FPClasses() & fcSubnormal) {
      معلوم.سېټ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcPosZero);
      إذا (معلوم_Src.get_معلوم_FPClasses() & fcNegSubnormal)
        معلوم.سېټ_Sign_بټ(std::nullopt);
    }
    أرجع معلوم;
  افتراضي:
    إذا (معلوم_Src.get_معلوم_FPClasses() & fcSubnormal) {
      معلوم.سېټ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcPosZero);
      إذا (معلوم_Src.get_معلوم_FPClasses() & fcNegSubnormal)
        معلوم.سېټ_Sign_بټ(std::nullopt);
    }
    إذا (معلوم_Src.get_معلوم_FPClasses() & fcNegSubnormal)
      معلوم.سېټ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcNegZero);
    أرجع معلوم;
  }

  llvm_unreachable("covered switch over denormal mode");
}

منطقي معلوم_FPClass::is_معلوم_Never_Logical_Zero(Denormal_ډېرپېښ ډېرپېښ) ثابت {
  أرجع apply_ننوت_Denormal_ډېرپېښ(*هذا, ډېرپېښ).is_معلوم_Never_Zero();
}

منطقي معلوم_FPClass::is_معلوم_Never_Logical_Neg_Zero(Denormal_ډېرپېښ ډېرپېښ) ثابت {
  أرجع apply_ننوت_Denormal_ډېرپېښ(*هذا, ډېرپېښ).is_معلوم_Never_Neg_Zero();
}

منطقي معلوم_FPClass::is_معلوم_Never_Logical_Pos_Zero(Denormal_ډېرپېښ ډېرپېښ) ثابت {
  أرجع apply_ننوت_Denormal_ډېرپېښ(*هذا, ډېرپېښ).is_معلوم_Never_Pos_Zero();
}

معلوم_FPClass معلوم_FPClass::minMaxLike(ثابت معلوم_FPClass &معلوم_LHS,
                                      ثابت معلوم_FPClass &معلوم_RHS,
                                      Min_Max_ډول ډول_2, Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم_LHS_2 = apply_ننوت_Denormal_ډېرپېښ(معلوم_LHS, ډېرپېښ);
  معلوم_FPClass معلوم_RHS_2 = apply_ننوت_Denormal_ډېرپېښ(معلوم_RHS, ډېرپېښ);

  معلوم_FPClass معلوم;

  معلوم.propagateNonNaN(معلوم_LHS_2, معلوم_RHS_2);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  إذا ((معلوم_LHS_2.is_معلوم_Never_Na_N() || معلوم_RHS_2.is_معلوم_Never_Na_N()) &&
      (ډول_2 == Min_Max_ډول::minnum || ډول_2 == Min_Max_ډول::maxnum ||
       ډول_2 == Min_Max_ډول::minimumnum || ډول_2 == Min_Max_ډول::maximumnum))
    معلوم.معلوم_Not(fcNan);

  بدل (ډول_2) {
  حالة Min_Max_ډول::maxnum: {
    إذا (معلوم_LHS_2.is_معلوم_Never_Na_N())
      معلوم.معلوم_Not(orderedStrictlyLess(معلوم_LHS_2.get_معلوم_FPClasses(),
                                         /*OrderedZeroSign=*/خطأ));
    إذا (معلوم_RHS_2.is_معلوم_Never_Na_N())
      معلوم.معلوم_Not(orderedStrictlyLess(معلوم_RHS_2.get_معلوم_FPClasses(),
                                         /*OrderedZeroSign=*/خطأ));
    اكسر;
  }
  حالة Min_Max_ډول::minnum: {
    إذا (معلوم_LHS_2.is_معلوم_Never_Na_N())
      معلوم.معلوم_Not(orderedStrictlyGreater(معلوم_LHS_2.get_معلوم_FPClasses(),
                                            /*OrderedZeroSign=*/خطأ));
    إذا (معلوم_RHS_2.is_معلوم_Never_Na_N())
      معلوم.معلوم_Not(orderedStrictlyGreater(معلوم_RHS_2.get_معلوم_FPClasses(),
                                            /*OrderedZeroSign=*/خطأ));
    اكسر;
  }
  حالة Min_Max_ډول::maximumnum: {
    إذا (معلوم_LHS_2.is_معلوم_Never_Na_N())
      معلوم.معلوم_Not(orderedStrictlyLess(معلوم_LHS_2.get_معلوم_FPClasses(),
                                         /*OrderedZeroSign=*/ریښتیا));
    إذا (معلوم_RHS_2.is_معلوم_Never_Na_N())
      معلوم.معلوم_Not(orderedStrictlyLess(معلوم_RHS_2.get_معلوم_FPClasses(),
                                         /*OrderedZeroSign=*/ریښتیا));
    اكسر;
  }
  حالة Min_Max_ډول::minimumnum: {
    إذا (معلوم_LHS_2.is_معلوم_Never_Na_N())
      معلوم.معلوم_Not(orderedStrictlyGreater(معلوم_LHS_2.get_معلوم_FPClasses(),
                                            /*OrderedZeroSign=*/ریښتیا));
    إذا (معلوم_RHS_2.is_معلوم_Never_Na_N())
      معلوم.معلوم_Not(orderedStrictlyGreater(معلوم_RHS_2.get_معلوم_FPClasses(),
                                            /*OrderedZeroSign=*/ریښتیا));
    اكسر;
  }
  حالة Min_Max_ډول::اعظمي: {
    معلوم.معلوم_Not(orderedStrictlyLess(معلوم_LHS_2.get_معلوم_FPClasses(),
                                       /*OrderedZeroSign=*/ریښتیا) |
                   orderedStrictlyLess(معلوم_RHS_2.get_معلوم_FPClasses(),
                                       /*OrderedZeroSign=*/ریښتیا));
    اكسر;
  }
  حالة Min_Max_ډول::minimum: {
    معلوم.معلوم_Not(orderedStrictlyGreater(معلوم_LHS_2.get_معلوم_FPClasses(),
                                          /*OrderedZeroSign=*/ریښتیا) |
                   orderedStrictlyGreater(معلوم_RHS_2.get_معلوم_FPClasses(),
                                          /*OrderedZeroSign=*/ریښتیا));
    اكسر;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      معلوم_LHS_2.get_معلوم_FPClasses() | معلوم_RHS_2.get_معلوم_FPClasses() | fcNan;
  معلوم.معلوم_Not(~PossibleClasses);

  أرجع apply_Output_Denormal_ډېرپېښ(معلوم, ډېرپېښ);
}

معلوم_FPClass معلوم_FPClass::canonicalize(ثابت معلوم_FPClass &معلوم_Src,
                                        Denormal_ډېرپېښ Denorm_ډېرپېښ) {
  معلوم_FPClass معلوم;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  معلوم.سېټ_معلوم_FPClasses(معلوم_Src.get_معلوم_FPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  إذا (معلوم_Src.is_معلوم_Never_Na_N())
    معلوم.معلوم_Not(fcNan);
  وإلا
    معلوم.معلوم_Not(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  إذا (Denorm_ډېرپېښ == Denormal_ډېرپېښ::getIEEE()) {
    إذا (معلوم_Src.is_معلوم_Never(fcPosZero))
      معلوم.معلوم_Not(fcPosZero);
    إذا (معلوم_Src.is_معلوم_Never(fcNegZero))
      معلوم.معلوم_Not(fcNegZero);
    أرجع معلوم;
  }

  إذا (Denorm_ډېرپېښ.inputsAreZero() || Denorm_ډېرپېښ.outputsAreZero())
    معلوم.معلوم_Not(fcSubnormal);

  إذا (Denorm_ډېرپېښ == Denormal_ډېرپېښ::getPreserveSign()) {
    إذا (معلوم_Src.is_معلوم_Never(fcPosZero | fcPosSubnormal))
      معلوم.معلوم_Not(fcPosZero);
    إذا (معلوم_Src.is_معلوم_Never(fcNegZero | fcNegSubnormal))
      معلوم.معلوم_Not(fcNegZero);
    أرجع معلوم;
  }

  إذا (Denorm_ډېرپېښ.ننوت == Denormal_ډېرپېښ::مثبت_Zero ||
      (Denorm_ډېرپېښ.Output == Denormal_ډېرپېښ::مثبت_Zero &&
       Denorm_ډېرپېښ.ننوت == Denormal_ډېرپېښ::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    إذا (معلوم_Src.is_معلوم_Never(fcNegZero))
      معلوم.معلوم_Not(fcNegZero);

    إذا (معلوم_Src.is_معلوم_Never(fcPosZero | fcSubnormal))
      معلوم.معلوم_Not(fcPosZero);
  }

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::bitcast(ثابت fltSemantics &FltSemantics,
                                   ثابت معلوم_بټونه &بټونه_2) {
  أكد(FltSemantics.اندازه_دننه_بټونه == بټونه_2.get_بټ_پلنوالی() &&
         "Bitcast operand has incorrect bit width");
  معلوم_FPClass معلوم;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  إذا (بټونه_2.hasConflict())
    أرجع معلوم;

  // Return unknown for types we have not validated.
  خپلکاری IsSupported = [](ثابت fltSemantics &Semantics) {
    بدل (APFloat::Semantics_منزل_Enum(Semantics)) {
    حالة APFloat_بنسټ::S_IEEEhalf:
    حالة APFloat_بنسټ::S_BFloat:
    حالة APFloat_بنسټ::S_IEEEsingle:
    حالة APFloat_بنسټ::S_IEEEdouble:
    حالة APFloat_بنسټ::S_IEEEquad:
    حالة APFloat_بنسټ::S_x87DoubleExtended:
      أرجع ریښتیا;
    افتراضي:
      أرجع خطأ;
    }
  };
  إذا (!IsSupported(FltSemantics))
    أرجع معلوم;

  // Transfer information from the sign bit.
  إذا (بټونه_2.isNonNegative())
    معلوم.sign_بټ_Must_Be_Zero();
  وإلا إذا (بټونه_2.isNegative())
    معلوم.sign_بټ_Must_Be_One();

  بدل (APFloat_بنسټ::Semantics_منزل_Enum(FltSemantics)) {
  حالة APFloat_بنسټ::S_IEEEhalf:
  حالة APFloat_بنسټ::S_BFloat:
  حالة APFloat_بنسټ::S_IEEEsingle:
  حالة APFloat_بنسټ::S_IEEEdouble:
  حالة APFloat_بنسټ::S_IEEEquad: {
    ثابت بینښه Mantissa_بټونه = FltSemantics.precision - 1;
    ثابت APInt Exponent_ماسک = APInt::get_بټونه_سېټ(
        FltSemantics.اندازه_دننه_بټونه, Mantissa_بټونه, FltSemantics.اندازه_دننه_بټونه - 1);
    ثابت APInt Mantissa_ماسک =
        APInt::get_ټیټ_بټونه_سېټ(FltSemantics.اندازه_دننه_بټونه, Mantissa_بټونه);

    ثابت منطقي Exponent_معلوم_All_Zeros =
        (بټونه_2.Zero & Exponent_ماسک) == Exponent_ماسک;
    ثابت منطقي Exponent_معلوم_All_Ones = (بټونه_2.One & Exponent_ماسک) == Exponent_ماسک;
    ثابت منطقي Exponent_معلوم_Not_All_Zeros = !(بټونه_2.One & Exponent_ماسک).isZero();
    ثابت منطقي Exponent_معلوم_Not_All_Ones = !(بټونه_2.Zero & Exponent_ماسک).isZero();

    ثابت منطقي Mantissa_معلوم_All_Zeros =
        (بټونه_2.Zero & Mantissa_ماسک) == Mantissa_ماسک;
    ثابت منطقي Mantissa_معلوم_Not_All_Zeros = !(بټونه_2.One & Mantissa_ماسک).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    إذا (Exponent_معلوم_Not_All_Zeros)
      معلوم.معلوم_Not(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    إذا (Exponent_معلوم_Not_All_Ones)
      معلوم.معلوم_Not(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    إذا (Exponent_معلوم_All_Zeros || Exponent_معلوم_All_Ones)
      معلوم.معلوم_Not(fc_نورمال);

    // Zero and infinity require a mantissa with all zero bits.
    إذا (Mantissa_معلوم_Not_All_Zeros)
      معلوم.معلوم_Not(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    إذا (Mantissa_معلوم_All_Zeros)
      معلوم.معلوم_Not(fcSubnormal | fcNan);

    ثابت منطقي Quiet_بټ_معلوم_سېټ = بټونه_2.One[Mantissa_بټونه - 1];
    ثابت منطقي Quiet_بټ_معلوم_Clear = بټونه_2.Zero[Mantissa_بټونه - 1];

    إذا (Quiet_بټ_معلوم_سېټ)
      معلوم.معلوم_Not(fcSNan);
    وإلا إذا (Quiet_بټ_معلوم_Clear)
      معلوم.معلوم_Not(fcQNan);

    اكسر;
  }
  حالة APFloat_بنسټ::S_x87DoubleExtended: {
    // Layout of the x87 floating point type:
    // [0:62] fraction
    // [63] explicit integer bit
    // [64:78] exponent
    // [79] sign bit
    // APFloat treats the x87 pseudo encodings as documented in
    // fltSemantics::hasExplicitIntegerBit.
    constexpr بینښه بټ_پلنوالی_2 = 80;
    constexpr بینښه Fraction_بټونه = 63;
    constexpr بینښه Integer_بټ_شاخص = Fraction_بټونه;
    ثابت APInt Fraction_ماسک = APInt::get_ټیټ_بټونه_سېټ(بټ_پلنوالی_2, Fraction_بټونه);
    ثابت APInt Exponent_ماسک =
        APInt::get_بټونه_سېټ(بټ_پلنوالی_2, Fraction_بټونه + 1, بټ_پلنوالی_2 - 1);

    ثابت منطقي Exponent_معلوم_All_Zeros =
        (بټونه_2.Zero & Exponent_ماسک) == Exponent_ماسک;
    ثابت منطقي Exponent_معلوم_All_Ones = (بټونه_2.One & Exponent_ماسک) == Exponent_ماسک;
    ثابت منطقي Exponent_معلوم_Not_All_Zeros = !(بټونه_2.One & Exponent_ماسک).isZero();
    ثابت منطقي Exponent_معلوم_Not_All_Ones = !(بټونه_2.Zero & Exponent_ماسک).isZero();
    ثابت منطقي Fraction_معلوم_All_Zeros =
        (بټونه_2.Zero & Fraction_ماسک) == Fraction_ماسک;
    ثابت منطقي Fraction_معلوم_Not_All_Zeros = !(بټونه_2.One & Fraction_ماسک).isZero();
    ثابت منطقي Integer_بټ_معلوم_سېټ = بټونه_2.One[Integer_بټ_شاخص];
    ثابت منطقي Integer_بټ_معلوم_Clear = بټونه_2.Zero[Integer_بټ_شاخص];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    إذا (Exponent_معلوم_Not_All_Zeros || Integer_بټ_معلوم_سېټ)
      معلوم.معلوم_Not(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    إذا (Fraction_معلوم_Not_All_Zeros)
      معلوم.معلوم_Not(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    إذا (Exponent_معلوم_Not_All_Ones || Integer_بټ_معلوم_Clear ||
        Fraction_معلوم_Not_All_Zeros)
      معلوم.معلوم_Not(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    إذا (Integer_بټ_معلوم_Clear || Exponent_معلوم_All_Ones)
      معلوم.معلوم_Not(fc_نورمال);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    إذا (Fraction_معلوم_All_Zeros)
      معلوم.معلوم_Not(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    إذا (Exponent_معلوم_All_Zeros ||
        (Integer_بټ_معلوم_سېټ &&
         (Exponent_معلوم_Not_All_Ones || Fraction_معلوم_All_Zeros)))
      معلوم.معلوم_Not(fcNan);

    ثابت منطقي Quiet_بټ_معلوم_سېټ = بټونه_2.One[Fraction_بټونه - 1];
    ثابت منطقي Quiet_بټ_معلوم_Clear = بټونه_2.Zero[Fraction_بټونه - 1];

    إذا (Quiet_بټ_معلوم_سېټ)
      معلوم.معلوم_Not(fcSNan);
    وإلا إذا (Quiet_بټ_معلوم_Clear)
      معلوم.معلوم_Not(fcQNan);

    اكسر;
  }
  افتراضي:
    llvm_unreachable("unhandled supported type");
  }

  أرجع معلوم;
}

معلوم_بټونه معلوم_FPClass::منزل_معلوم_بټونه(ثابت fltSemantics &FltSemantics) ثابت {
  معلوم_بټونه معلوم(FltSemantics.اندازه_دننه_بټونه);
  ثابت FPClassTest FPClasses = get_معلوم_FPClasses();

  // Return unknown if poison.
  إذا (FPClasses == fcNone)
    أرجع معلوم;

  // Return unknown for types we have not validated.
  خپلکاری IsSupported = [](ثابت fltSemantics &Semantics) {
    بدل (APFloat::Semantics_منزل_Enum(Semantics)) {
    حالة APFloat_بنسټ::S_IEEEhalf:
    حالة APFloat_بنسټ::S_BFloat:
    حالة APFloat_بنسټ::S_IEEEsingle:
    حالة APFloat_بنسټ::S_IEEEdouble:
    حالة APFloat_بنسټ::S_IEEEquad:
    حالة APFloat_بنسټ::S_x87DoubleExtended:
      أرجع ریښتیا;
    افتراضي:
      أرجع خطأ;
    }
  };
  إذا (!IsSupported(FltSemantics))
    أرجع معلوم;

  بدل (APFloat::Semantics_منزل_Enum(FltSemantics)) {
  حالة APFloat_بنسټ::S_IEEEhalf:
  حالة APFloat_بنسټ::S_BFloat:
  حالة APFloat_بنسټ::S_IEEEsingle:
  حالة APFloat_بنسټ::S_IEEEdouble:
  حالة APFloat_بنسټ::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    إذا (FPClasses & fc_نورمال)
      اكسر;

    معلوم.سېټ_All_Conflict();

    ثابت بینښه بټ_پلنوالی_2 = FltSemantics.اندازه_دننه_بټونه;
    ثابت بینښه Mantissa_بټونه = FltSemantics.precision - 1;
    ثابت بینښه Exponent_بټونه = بټ_پلنوالی_2 - Mantissa_بټونه - 1;

    APInt Mantissa_ماسک = APInt::get_ټیټ_بټونه_سېټ(بټ_پلنوالی_2, Mantissa_بټونه);
    APInt Exponent_ماسک =
        APInt::get_بټونه_سېټ(بټ_پلنوالی_2, Mantissa_بټونه, Mantissa_بټونه + Exponent_بټونه);

    ثابت بینښه Quiet_بټ_شاخص = Mantissa_بټونه - 1;
    APInt Payload_ماسک = Mantissa_ماسک;
    Payload_ماسک.clear_بټ(Quiet_بټ_شاخص);

    إذا (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      معلوم.Zero &= ~Exponent_ماسک;
      // No individual payload bit is known.
      معلوم.Zero &= ~Payload_ماسک;
      معلوم.One &= ~Payload_ماسک;

      إذا (FPClasses & fcQNan)
        معلوم.Zero.clear_بټ(Quiet_بټ_شاخص);
      إذا (FPClasses & fcSNan)
        معلوم.One.clear_بټ(Quiet_بټ_شاخص);
    }
    إذا (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      معلوم.Zero &= ~Exponent_ماسک;
      // Mantissa bits cannot be ones.
      معلوم.One &= ~Mantissa_ماسک;
    }
    إذا (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      معلوم.One &= ~Exponent_ماسک;
      // Unknown mantissa.
      معلوم.One &= ~Mantissa_ماسک;
      معلوم.Zero &= ~Mantissa_ماسک;
    }
    إذا (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      معلوم.One &= ~Exponent_ماسک;
      // Mantissa cannot be ones.
      معلوم.One &= ~Mantissa_ماسک;
    }

    اكسر;
  }
  حالة APFloat_بنسټ::S_x87DoubleExtended: {
    إذا (!is_معلوم_Never(fc_نورمال | fcSubnormal | fcNan))
      اكسر;

    معلوم.سېټ_All_Conflict();

    إذا (FPClasses & fcInf)
      معلوم = معلوم.intersectWith(معلوم_بټونه::makeConstant(
          APFloat::getInf(FltSemantics).bitcast_منزل_APInt()));

    إذا (FPClasses & fcZero)
      معلوم = معلوم.intersectWith(
          معلوم_بټونه::makeConstant(APInt::getZero(FltSemantics.اندازه_دننه_بټونه)));

    اكسر;
  }
  افتراضي:
    llvm_unreachable("unhandled supported semantics");
  }

  معلوم.Zero.clear_Sign_بټ();
  معلوم.One.clear_Sign_بټ();
  إذا (std::optional<منطقي> Sign = get_Sign_بټ()) {
    إذا (*Sign)
      معلوم.makeNegative();
    وإلا
      معلوم.makeNonNegative();
  }

  أرجع معلوم;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
ساكن معلوم_FPClass fadd_impl(ثابت معلوم_FPClass &معلوم_LHS_2,
                              ثابت معلوم_FPClass &معلوم_RHS_2, منطقي IsFMA) {
  معلوم_FPClass معلوم;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  إذا (معلوم_LHS_2.is_معلوم_Never_Na_N() && معلوم_RHS_2.is_معلوم_Never_Na_N() &&
      (معلوم_LHS_2.is_معلوم_Never(fcPosInf) || معلوم_RHS_2.is_معلوم_Never(fcNegInf)) &&
      (معلوم_LHS_2.is_معلوم_Never(fcNegInf) || معلوم_RHS_2.is_معلوم_Never(fcPosInf)))
    معلوم.معلوم_Not(fcNan);

  إذا (معلوم_LHS_2.cannotBeOrderedLessThanZero() &&
      معلوم_RHS_2.cannotBeOrderedLessThanZero()) {
    معلوم.معلوم_Not(معلوم_FPClass::Ordered_Less_Than_Zero_ماسک);

    // This can't underflow if one of the operands is known normal.
    إذا (معلوم_LHS_2.is_معلوم_Never(fcZero | fcPosSubnormal) ||
        معلوم_RHS_2.is_معلوم_Never(fcZero | fcPosSubnormal))
      معلوم.معلوم_Not(fcZero | fcPosSubnormal);
  }

  إذا (معلوم_LHS_2.cannotBeOrderedGreaterThanZero() &&
      معلوم_RHS_2.cannotBeOrderedGreaterThanZero()) {
    معلوم.معلوم_Not(معلوم_FPClass::Ordered_Greater_Than_Zero_ماسک);

    // This can't underflow if one of the operands is known normal.
    إذا (معلوم_LHS_2.is_معلوم_Never(fcZero | fcNegSubnormal) ||
        معلوم_RHS_2.is_معلوم_Never(fcZero | fcNegSubnormal))
      معلوم.معلوم_Not(fcZero | fcNegSubnormal);
  }

  إذا (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    إذا (معلوم_LHS_2.is_معلوم_Never(fcNegZero) || معلوم_RHS_2.is_معلوم_Never(fcNegZero))
      معلوم.معلوم_Not(fcNegZero);

    معلوم.propagateNonSNaN(معلوم_LHS_2, معلوم_RHS_2);
  }

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::fadd(ثابت معلوم_FPClass &معلوم_LHS,
                                ثابت معلوم_FPClass &معلوم_RHS,
                                Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم_LHS_2 = apply_ننوت_Denormal_ډېرپېښ(معلوم_LHS, ډېرپېښ);
  معلوم_FPClass معلوم_RHS_2 = apply_ننوت_Denormal_ډېرپېښ(معلوم_RHS, ډېرپېښ);

  معلوم_FPClass معلوم = fadd_impl(معلوم_LHS_2, معلوم_RHS_2, /*IsFMA=*/خطأ);

  أرجع apply_Output_Denormal_ډېرپېښ(معلوم, ډېرپېښ);
}

معلوم_FPClass معلوم_FPClass::fadd_self(ثابت معلوم_FPClass &معلوم_Src_2,
                                     Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم_Src = apply_ننوت_Denormal_ډېرپېښ(معلوم_Src_2, ډېرپېښ);

  معلوم_FPClass معلوم = fadd_impl(معلوم_Src, معلوم_Src, /*IsFMA=*/خطأ);

  إذا (معلوم_Src.is_معلوم_Never(fcPosSubnormal))
    معلوم.معلوم_Not(fcPosSubnormal);
  إذا (معلوم_Src.is_معلوم_Never(fcNegSubnormal))
    معلوم.معلوم_Not(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  إذا (معلوم_Src.is_معلوم_Never(fcPosZero))
    معلوم.معلوم_Not(fcPosZero);

  أرجع apply_Output_Denormal_ډېرپېښ(معلوم, ډېرپېښ);
}

معلوم_FPClass معلوم_FPClass::fsub(ثابت معلوم_FPClass &معلوم_LHS,
                                ثابت معلوم_FPClass &معلوم_RHS,
                                Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم_LHS_2 = apply_ننوت_Denormal_ډېرپېښ(معلوم_LHS, ډېرپېښ);
  معلوم_FPClass معلوم_RHS_2 = apply_ننوت_Denormal_ډېرپېښ(معلوم_RHS, ډېرپېښ);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  معلوم_RHS_2.fneg();

  معلوم_FPClass معلوم = fadd_impl(معلوم_LHS_2, معلوم_RHS_2, /*IsFMA=*/خطأ);

  أرجع apply_Output_Denormal_ډېرپېښ(معلوم, ډېرپېښ);
}

معلوم_FPClass معلوم_FPClass::fmul(ثابت معلوم_FPClass &معلوم_LHS,
                                ثابت معلوم_FPClass &معلوم_RHS,
                                Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم_LHS_2 = apply_ننوت_Denormal_ډېرپېښ(معلوم_LHS, ډېرپېښ);
  معلوم_FPClass معلوم_RHS_2 = apply_ننوت_Denormal_ډېرپېښ(معلوم_RHS, ډېرپېښ);

  معلوم_FPClass معلوم;

  معلوم.propagateNonSNaN(معلوم_LHS_2, معلوم_RHS_2);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  إذا ((معلوم_LHS_2.is_معلوم_Never(fcNegative) &&
       معلوم_RHS_2.is_معلوم_Never(fcNegative)) ||
      (معلوم_LHS_2.is_معلوم_Never(fc_مثبت) && معلوم_RHS_2.is_معلوم_Never(fc_مثبت)))
    معلوم.معلوم_Not(fcNegative);
  إذا ((معلوم_LHS_2.is_معلوم_Never(fc_مثبت) &&
       معلوم_RHS_2.is_معلوم_Never(fcNegative)) ||
      (معلوم_LHS_2.is_معلوم_Never(fcNegative) && معلوم_RHS_2.is_معلوم_Never(fc_مثبت)))
    معلوم.معلوم_Not(fc_مثبت);

  // Inf * Y => Inf or NaN
  إذا (معلوم_LHS_2.is_معلوم_Always(fcInf | fcNan) ||
      معلوم_RHS_2.is_معلوم_Always(fcInf | fcNan))
    معلوم.معلوم_Not(fc_نورمال | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  إذا (معلوم_RHS_2.is_معلوم_Always(fcZero | fcNan) ||
      معلوم_LHS_2.is_معلوم_Always(fcZero | fcNan))
    معلوم.معلوم_Not(fc_نورمال | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  إذا (معلوم_LHS_2.is_معلوم_Never_Na_N() && معلوم_RHS_2.is_معلوم_Never_Na_N() &&
      (معلوم_RHS_2.is_معلوم_Never_Infinity() || معلوم_LHS_2.is_معلوم_Never_Zero()) &&
      (معلوم_LHS_2.is_معلوم_Never_Infinity() || معلوم_RHS_2.is_معلوم_Never_Zero()))
    معلوم.معلوم_Not(fcNan);

  أرجع apply_Output_Denormal_ډېرپېښ(معلوم, ډېرپېښ);
}

// TODO: This generalizes to known ranges
معلوم_FPClass معلوم_FPClass::fmul(ثابت معلوم_FPClass &معلوم_LHS_2,
                                ثابت APFloat &CRHS, Denormal_ډېرپېښ ډېرپېښ) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  ثابت fltSemantics &Flt = CRHS.getSemantics();
  بینښه Precision = APFloat::semanticsPrecision(Flt);
  ثابت صحيح Mantissa_بټونه = Precision - 1;

  صحيح Min_معلوم_Exponent = ilogb(CRHS);
  منطقي CannotBeSubnormal = (Min_معلوم_Exponent >= Mantissa_بټونه);

  معلوم_FPClass معلوم = معلوم_FPClass::fmul(معلوم_LHS_2, معلوم_FPClass(CRHS), ډېرپېښ);
  إذا (CannotBeSubnormal)
    معلوم.معلوم_Not(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  إذا (معلوم_LHS_2.is_معلوم_Never(fcInf)) {
    إذا (Min_معلوم_Exponent < 0)
      معلوم.معلوم_Not(fcInf);
    وإلا إذا (Min_معلوم_Exponent == 0 && CRHS.compare_Absolute_ارزښت(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      معلوم.معلوم_Not(fcInf);
  }

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::fdiv(ثابت معلوم_FPClass &معلوم_LHS,
                                ثابت معلوم_FPClass &معلوم_RHS,
                                Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم_LHS_2 = apply_ننوت_Denormal_ډېرپېښ(معلوم_LHS, ډېرپېښ);
  معلوم_FPClass معلوم_RHS_2 = apply_ننوت_Denormal_ډېرپېښ(معلوم_RHS, ډېرپېښ);

  معلوم_FPClass معلوم;

  معلوم.propagateNonSNaN(معلوم_LHS_2, معلوم_RHS_2);

  // Only 0/0, Inf/Inf produce NaN.
  إذا (معلوم_LHS_2.is_معلوم_Never_Na_N() && معلوم_RHS_2.is_معلوم_Never_Na_N() &&
      (معلوم_LHS_2.is_معلوم_Never_Infinity() || معلوم_RHS_2.is_معلوم_Never_Infinity()) &&
      (معلوم_LHS_2.is_معلوم_Never_Zero() || معلوم_RHS_2.is_معلوم_Never_Zero())) {
    معلوم.معلوم_Not(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  إذا ((معلوم_LHS_2.is_معلوم_Never(fcNegative) &&
       معلوم_RHS_2.is_معلوم_Never(fcNegative)) ||
      (معلوم_LHS_2.is_معلوم_Never(fc_مثبت) && معلوم_RHS_2.is_معلوم_Never(fc_مثبت)))
    معلوم.معلوم_Not(fcNegative);
  إذا ((معلوم_LHS_2.is_معلوم_Never(fc_مثبت) &&
       معلوم_RHS_2.is_معلوم_Never(fcNegative)) ||
      (معلوم_LHS_2.is_معلوم_Never(fcNegative) && معلوم_RHS_2.is_معلوم_Never(fc_مثبت)))
    معلوم.معلوم_Not(fc_مثبت);

  // Normal and subnormal results require two non-zero finite operands.
  إذا ((معلوم_LHS_2.is_معلوم_Never(fc_Neg_نورمال | fcNegSubnormal) &&
       معلوم_RHS_2.is_معلوم_Never(fc_Neg_نورمال | fcNegSubnormal)) ||
      (معلوم_LHS_2.is_معلوم_Never(fc_Pos_نورمال | fcPosSubnormal) &&
       معلوم_RHS_2.is_معلوم_Never(fc_Pos_نورمال | fcPosSubnormal)))
    معلوم.معلوم_Not(fc_Neg_نورمال | fcNegSubnormal);
  إذا ((معلوم_LHS_2.is_معلوم_Never(fc_Neg_نورمال | fcNegSubnormal) &&
       معلوم_RHS_2.is_معلوم_Never(fc_Pos_نورمال | fcPosSubnormal)) ||
      (معلوم_LHS_2.is_معلوم_Never(fc_Pos_نورمال | fcPosSubnormal) &&
       معلوم_RHS_2.is_معلوم_Never(fc_Neg_نورمال | fcNegSubnormal)))
    معلوم.معلوم_Not(fc_Pos_نورمال | fcPosSubnormal);

  // 0 / X => 0 or NaN
  إذا (معلوم_LHS_2.is_معلوم_Always(fcZero))
    معلوم.معلوم_Not(fcSubnormal | fc_نورمال | fcInf);

  // X / 0 => NaN or Inf
  إذا (معلوم_RHS_2.is_معلوم_Always(fcZero))
    معلوم.معلوم_Not(fcFinite);

  أرجع apply_Output_Denormal_ډېرپېښ(معلوم, ډېرپېښ);
}

معلوم_FPClass معلوم_FPClass::fdiv_self(ثابت معلوم_FPClass &معلوم_Src,
                                     Denormal_ډېرپېښ ډېرپېښ) {
  // X / X is always exactly 1.0 or a NaN.
  معلوم_FPClass معلوم(fcNan | fc_Pos_نورمال);

  معلوم.propagateNonSNaN(معلوم_Src);

  إذا (معلوم_Src.is_معلوم_Never_Inf_Or_Na_N() && معلوم_Src.is_معلوم_Never_Logical_Zero(ډېرپېښ))
    معلوم.معلوم_Not(fcNan);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::frem(ثابت معلوم_FPClass &معلوم_LHS_2,
                                ثابت معلوم_FPClass &معلوم_RHS_2,
                                Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  معلوم.معلوم_Not(fcInf);

  معلوم.propagateNonSNaN(معلوم_LHS_2, معلوم_RHS_2);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  إذا (معلوم_LHS_2.is_معلوم_Never_Inf_Or_Na_N() && معلوم_RHS_2.is_معلوم_Never_Na_N() &&
      معلوم_RHS_2.is_معلوم_Never_Logical_Zero(ډېرپېښ)) {
    معلوم.معلوم_Not(fcNan);
  }

  // The sign for frem is the same as the first operand.
  إذا (معلوم_LHS_2.is_معلوم_Never(fc_Pos_نورمال | fcPosSubnormal))
    معلوم.معلوم_Not(fc_Pos_نورمال | fcPosSubnormal);
  إذا (معلوم_LHS_2.is_معلوم_Never(fc_Neg_نورمال | fcNegSubnormal))
    معلوم.معلوم_Not(fc_Neg_نورمال | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  إذا (معلوم_LHS_2.is_معلوم_Never(fcNegFinite))
    معلوم.معلوم_Not(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  إذا (معلوم_LHS_2.is_معلوم_Never(fcPosFinite) &&
      معلوم_LHS_2.is_معلوم_Never_Logical_Pos_Zero(ډېرپېښ) &&
      (!ډېرپېښ.outputs_May_Be_مثبت_Zero() ||
       معلوم_LHS_2.is_معلوم_Never(fc_Neg_نورمال | fcNegSubnormal)))
    معلوم.معلوم_Not(fcPosZero);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::frem_self(ثابت معلوم_FPClass &معلوم_Src,
                                     Denormal_ډېرپېښ ډېرپېښ) {
  // X % X is always exactly [+-]0.0 or a NaN.
  معلوم_FPClass معلوم(fcNan | fcZero);

  معلوم.propagateNonSNaN(معلوم_Src);

  إذا (معلوم_Src.is_معلوم_Never_Inf_Or_Na_N() && معلوم_Src.is_معلوم_Never_Logical_Zero(ډېرپېښ))
    معلوم.معلوم_Not(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  إذا (معلوم_Src.is_معلوم_Never(fc_Pos_نورمال | fcPosSubnormal))
    معلوم.معلوم_Not(fcPosZero);
  إذا (معلوم_Src.is_معلوم_Never(fc_Neg_نورمال | fcNegSubnormal))
    معلوم.معلوم_Not(fcNegZero);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::fma(ثابت معلوم_FPClass &معلوم_LHS_2,
                               ثابت معلوم_FPClass &معلوم_RHS_2,
                               ثابت معلوم_FPClass &معلوم_Addend,
                               Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass Mul = fmul(معلوم_LHS_2, معلوم_RHS_2, ډېرپېښ);

  // FMA differs from the base fmul + fadd handling only in the treatment of -0
  // results.
  //
  // If the multiply is a -0 due to rounding, the final -0 + 0 will be -0,
  // unlike for a separate fadd.
  معلوم_FPClass معلوم = fadd_impl(Mul, معلوم_Addend, /*IsFMA=*/ریښتیا);

  // propagateNonSNaN for 3 arguments.
  إذا (معلوم_LHS_2.is_معلوم_Never(fcSNan) && معلوم_RHS_2.is_معلوم_Never(fcSNan) &&
      معلوم_Addend.is_معلوم_Never(fcSNan))
    معلوم.معلوم_Not(fcSNan);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::fmuladd(ثابت معلوم_FPClass &معلوم_LHS_2,
                                   ثابت معلوم_FPClass &معلوم_RHS_2,
                                   ثابت معلوم_FPClass &معلوم_Addend,
                                   Denormal_ډېرپېښ ډېرپېښ) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  معلوم_FPClass FMA = fma(معلوم_LHS_2, معلوم_RHS_2, معلوم_Addend, ډېرپېښ);
  معلوم_FPClass Mul_جمع_کړه = fadd(fmul(معلوم_LHS_2, معلوم_RHS_2, ډېرپېښ), معلوم_Addend, ډېرپېښ);
  أرجع FMA.intersectWith(Mul_جمع_کړه);
}

معلوم_FPClass معلوم_FPClass::fma_مربع(ثابت معلوم_FPClass &معلوم_Squared,
                                      ثابت معلوم_FPClass &معلوم_Addend,
                                      Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass Squared = مربع(معلوم_Squared, ډېرپېښ);
  معلوم_FPClass معلوم = fadd_impl(Squared, معلوم_Addend, /*IsFMA=*/ریښتیا);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  إذا (معلوم_Addend.is_معلوم_Never(fcNegInf | fcNan) && Squared.is_معلوم_Never(fcNan))
    معلوم.معلوم_Not(fcNan);

  معلوم.propagateNonSNaN(معلوم_Squared, معلوم_Addend);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::fmuladd_مربع(ثابت معلوم_FPClass &معلوم_Squared,
                                          ثابت معلوم_FPClass &معلوم_Addend,
                                          Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass FMA = fma_مربع(معلوم_Squared, معلوم_Addend, ډېرپېښ);
  معلوم_FPClass Mul_جمع_کړه = fadd(مربع(معلوم_Squared, ډېرپېښ), معلوم_Addend, ډېرپېښ);
  أرجع FMA.intersectWith(Mul_جمع_کړه);
}

معلوم_FPClass معلوم_FPClass::توانیز_تابع(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;
  معلوم.معلوم_Not(fcNegative);

  معلوم.propagateNonNaN(معلوم_Src);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  إذا (معلوم_Src.is_معلوم_Never(fc_Neg_نورمال | fcNegInf))
    معلوم.معلوم_Not(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  إذا (معلوم_Src.is_معلوم_Never(fc_Neg_نورمال))
    معلوم.معلوم_Not(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  إذا (معلوم_Src.is_معلوم_Never(fc_Pos_نورمال | fcPosInf))
    معلوم.معلوم_Not(fcPosInf);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::لوګاریتم(ثابت معلوم_FPClass &معلوم_Src, Denormal_ډېرپېښ ډېرپېښ,
                               منطقي IsMultiUnitFPType) {
  معلوم_FPClass معلوم;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  إذا (!IsMultiUnitFPType)
    معلوم.معلوم_Not(fcNegZero | fcSubnormal);

  معلوم.propagateNonSNaN(معلوم_Src);

  إذا (معلوم_Src.is_معلوم_Never_Pos_Infinity())
    معلوم.معلوم_Not(fcPosInf);

  إذا (معلوم_Src.is_معلوم_Never_Na_N() && معلوم_Src.cannotBeOrderedLessThanZero())
    معلوم.معلوم_Not(fcNan);

  إذا (معلوم_Src.is_معلوم_Never_Logical_Zero(ډېرپېښ))
    معلوم.معلوم_Not(fcNegInf);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::جذر(ثابت معلوم_FPClass &معلوم_Src,
                                Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم;
  معلوم.معلوم_Not(fcPosSubnormal);

  إذا (معلوم_Src.is_معلوم_Never_Pos_Infinity())
    معلوم.معلوم_Not(fcPosInf);

  معلوم.propagateNonSNaN(معلوم_Src);

  // Any negative value besides -0 returns a nan.
  إذا (معلوم_Src.is_معلوم_Never_Na_N() && معلوم_Src.cannotBeOrderedLessThanZero())
    معلوم.معلوم_Not(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  معلوم.معلوم_Not(fcNegInf | fcNegSubnormal | fc_Neg_نورمال);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  إذا (معلوم_Src.is_معلوم_Never_Logical_Neg_Zero(ډېرپېښ))
    معلوم.معلوم_Not(fcNegZero);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::ساین(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  معلوم.propagateNonSNaN(معلوم_Src);

  // Return NaN on infinite inputs.
  معلوم.معلوم_Not(fcInf);
  إذا (معلوم_Src.is_معلوم_Never_Na_N() && معلوم_Src.is_معلوم_Never_Infinity())
    معلوم.معلوم_Not(fcNan);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::کوساین(ثابت معلوم_FPClass &معلوم_Src) {
  أرجع ساین(معلوم_Src);
}

معلوم_FPClass معلوم_FPClass::tan(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  معلوم.معلوم_Not(fcInf);

  معلوم.propagateNonSNaN(معلوم_Src);

  // NaN propagates. tan(+-Inf) is NaN.
  إذا (معلوم_Src.is_معلوم_Never_Na_N() && معلوم_Src.is_معلوم_Never_Infinity())
    معلوم.معلوم_Not(fcNan);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::sinh(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  إذا (معلوم_Src.is_معلوم_Never(fcNegative))
    معلوم.معلوم_Not(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  إذا (معلوم_Src.is_معلوم_Never(fcPosSubnormal | fc_Pos_نورمال | fcPosInf))
    معلوم.معلوم_Not(fcPosSubnormal | fc_Pos_نورمال | fcPosInf);

  معلوم.propagateNonNaN(معلوم_Src);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::cosh(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  معلوم.معلوم_Not(fcNegative | fcZero | fcSubnormal);

  معلوم.propagateNonNaN(معلوم_Src);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::tanh(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  // tanh is bounded to (-1, 1), never Inf.
  معلوم.معلوم_Not(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  إذا (معلوم_Src.is_معلوم_Never(fcNegSubnormal | fc_Neg_نورمال | fcNegInf)) {
    معلوم.معلوم_Not(fcNegSubnormal | fc_Neg_نورمال);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    إذا (معلوم_Src.is_معلوم_Never_Neg_Zero())
      معلوم.معلوم_Not(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  إذا (معلوم_Src.is_معلوم_Never(fcPosSubnormal | fc_Pos_نورمال | fcPosInf))
    معلوم.معلوم_Not(fcPosSubnormal | fc_Pos_نورمال);

  معلوم.propagateNonNaN(معلوم_Src);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::asin(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  معلوم.معلوم_Not(fcInf);

  معلوم.propagateNonSNaN(معلوم_Src);

  // asin is sign-preserving for finite arguments.
  إذا (معلوم_Src.is_معلوم_Never(fcNegFinite))
    معلوم.معلوم_Not(fcNegFinite);

  إذا (معلوم_Src.is_معلوم_Never(fcPosSubnormal | fc_Pos_نورمال))
    معلوم.معلوم_Not(fcPosSubnormal | fc_Pos_نورمال);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::acos(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  // acos(x) is bounded to [0, pi] for -1 <= x <= 1, and is never negative,
  // infinite, or subnormal. The smallest non-zero value occurs when x is
  // close to 1.0, where acos(x) can be approximated by sqrt(2 * (1 - x)).
  // Since sqrt cannot produce a subnormal result, we can conclude that
  // acos(x) will also never produce a subnormal result.
  معلوم.معلوم_Not(fcNegative | fcInf | fcSubnormal);

  // acos(x) == +0.0 iff x == +1.0
  إذا (معلوم_Src.is_معلوم_Never(fc_Pos_نورمال))
    معلوم.معلوم_Not(fcZero);

  معلوم.propagateNonSNaN(معلوم_Src);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::atan(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  معلوم.معلوم_Not(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  إذا (معلوم_Src.is_معلوم_Never(fcNegSubnormal | fc_Neg_نورمال | fcNegInf))
    معلوم.معلوم_Not(fcNegSubnormal | fc_Neg_نورمال);

  إذا (معلوم_Src.is_معلوم_Never(fcPosSubnormal | fc_Pos_نورمال | fcPosInf))
    معلوم.معلوم_Not(fcPosSubnormal | fc_Pos_نورمال);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  إذا (معلوم_Src.is_معلوم_Never(fc_Neg_نورمال | fcNegSubnormal | fcNegZero))
    معلوم.معلوم_Not(fcNegZero);

  معلوم.propagateNonNaN(معلوم_Src);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::atan2(ثابت معلوم_FPClass &معلوم_Y,
                                 ثابت معلوم_FPClass &معلوم_X,
                                 Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم_Y_2 = apply_ننوت_Denormal_ډېرپېښ(معلوم_Y, ډېرپېښ);
  معلوم_FPClass معلوم_X_2 = apply_ننوت_Denormal_ډېرپېښ(معلوم_X, ډېرپېښ);
  معلوم_FPClass معلوم;

  // Even though these deductions are correct, we are ignoring the following
  // potentially erroneous cases:
  //   * atan2(y, inf) is not subnormal
  //   * atan2(inf, x) is not zero or subnormal

  // atan2 result is in (-pi, pi], never Inf.
  معلوم.معلوم_Not(fcInf);

  معلوم.propagateNonNaN(معلوم_Y_2, معلوم_X_2);

  // If x <= -0.0, then |atan2(y, x)| >= pi/2
  إذا (معلوم_X_2.is_معلوم_Never(fc_مثبت))
    معلوم.معلوم_Not(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  إذا (معلوم_Y_2.is_معلوم_Never(fcNegative))
    معلوم.معلوم_Not(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  إذا (معلوم_Y_2.is_معلوم_Never(fc_مثبت))
    معلوم.معلوم_Not(fc_مثبت);

  أرجع apply_Output_Denormal_ډېرپېښ(معلوم, ډېرپېښ);
}

معلوم_FPClass معلوم_FPClass::fpext(ثابت معلوم_FPClass &معلوم_Src,
                                 ثابت fltSemantics &DstTy,
                                 ثابت fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  معلوم_FPClass معلوم = معلوم_Src;

  // All subnormal inputs should be in the normal range in the result type.
  إذا (APFloat::is_Representable_As_نورمال_دننه(SrcTy, DstTy)) {
    إذا (معلوم.get_معلوم_FPClasses() & fcPosSubnormal)
      معلوم.سېټ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fc_Pos_نورمال);
    إذا (معلوم.get_معلوم_FPClasses() & fcNegSubnormal)
      معلوم.سېټ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fc_Neg_نورمال);
    معلوم.معلوم_Not(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  إذا (!معلوم.is_معلوم_Never_Na_N())
    معلوم.سېټ_Sign_بټ(std::nullopt);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::fptrunc(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  إذا (معلوم_Src.cannotBeOrderedLessThanZero())
    معلوم.معلوم_Not(معلوم_FPClass::Ordered_Less_Than_Zero_ماسک);

  معلوم.propagateNonNaN(معلوم_Src);

  // Infinity needs a range check.
  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::پړاو_منزل_Integral(ثابت معلوم_FPClass &معلوم_Src,
                                           منطقي MayBeMultiUnitFPType,
                                           Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم;

  // Integer results cannot be subnormal.
  معلوم.معلوم_Not(fcSubnormal);

  معلوم.propagateNonNaN(معلوم_Src);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  إذا (!MayBeMultiUnitFPType) {
    إذا (معلوم_Src.is_معلوم_Never_Pos_Infinity())
      معلوم.معلوم_Not(fcPosInf);
    إذا (معلوم_Src.is_معلوم_Never_Neg_Infinity())
      معلوم.معلوم_Not(fcNegInf);
  }

  إذا (معلوم_Src.is_معلوم_Never(fc_Pos_نورمال | fcPosSubnormal))
    معلوم.معلوم_Not(fc_Pos_نورمال);

  إذا (معلوم_Src.is_معلوم_Never(fc_Neg_نورمال | fcNegSubnormal))
    معلوم.معلوم_Not(fc_Neg_نورمال);

  // Negative round ups towards zero produce negative zero.
  إذا (معلوم_Src.is_معلوم_Never(fcNegFinite))
    معلوم.معلوم_Not(fcNegZero);

  // Negative subnormals may flush to positive zero.
  إذا (معلوم_Src.is_معلوم_Never(fcPosFinite) &&
      (معلوم_Src.is_معلوم_Never(fcNegSubnormal) ||
       !ډېرپېښ.inputs_May_Be_مثبت_Zero()))
    معلوم.معلوم_Not(fcPosZero);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::trunc(ثابت معلوم_FPClass &معلوم_Src,
                                 Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم = پړاو_منزل_Integral(معلوم_Src,
                                       /*MayBeMultiUnitFPType=*/ریښتیا, ډېرپېښ);

  إذا (معلوم_Src.is_معلوم_Never_Pos_Infinity())
    معلوم.معلوم_Not(fcPosInf);
  إذا (معلوم_Src.is_معلوم_Never_Neg_Infinity())
    معلوم.معلوم_Not(fcNegInf);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::floor(ثابت معلوم_FPClass &معلوم_Src,
                                 منطقي IsMultiUnitFPType, Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم = پړاو_منزل_Integral(معلوم_Src, IsMultiUnitFPType, ډېرپېښ);

  // Applicable for multi-unit floating point types.
  إذا (معلوم_Src.is_معلوم_Never_Pos_Infinity())
    معلوم.معلوم_Not(fcPosInf);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::ceil(ثابت معلوم_FPClass &معلوم_Src,
                                منطقي IsMultiUnitFPType, Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم = پړاو_منزل_Integral(معلوم_Src, IsMultiUnitFPType, ډېرپېښ);

  // Applicable for multi-unit floating point types.
  إذا (معلوم_Src.is_معلوم_Never_Neg_Infinity())
    معلوم.معلوم_Not(fcNegInf);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::frexp_mant(ثابت معلوم_FPClass &معلوم_Src,
                                      Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم;

  معلوم.معلوم_Not(fcSubnormal);

  معلوم.propagateNonNaN(معلوم_Src);

  إذا (معلوم_Src.is_معلوم_Never_Pos_Infinity())
    معلوم.معلوم_Not(fcPosInf);

  إذا (معلوم_Src.is_معلوم_Never_Neg_Infinity())
    معلوم.معلوم_Not(fcNegInf);

  إذا (معلوم_Src.is_معلوم_Never_Logical_Pos_Zero(ډېرپېښ))
    معلوم.معلوم_Not(fcPosZero);

  إذا (معلوم_Src.is_معلوم_Never_Logical_Neg_Zero(ډېرپېښ))
    معلوم.معلوم_Not(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  إذا (معلوم_Src.is_معلوم_Never(fc_Pos_نورمال | fcPosSubnormal))
    معلوم.معلوم_Not(fc_Pos_نورمال);

  إذا (معلوم_Src.is_معلوم_Never(fc_Neg_نورمال | fcNegSubnormal))
    معلوم.معلوم_Not(fc_Neg_نورمال);

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::ldexp(ثابت معلوم_FPClass &معلوم_Src_2,
                                 ثابت APInt &ConstantRangeExpMin,
                                 ثابت APInt &ConstantRangeExpMax,
                                 ثابت fltSemantics &Flt, Denormal_ډېرپېښ ډېرپېښ) {
  معلوم_FPClass معلوم;

  معلوم_FPClass معلوم_Src = apply_ننوت_Denormal_ډېرپېښ(معلوم_Src_2, ډېرپېښ);

  إذا (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    معلوم.سېټ_معلوم_FPClasses(معلوم_Src.get_معلوم_FPClasses() | fcNan);
    معلوم.propagateNonNaN(معلوم_Src);
    أرجع apply_Output_Denormal_ډېرپېښ(معلوم, ډېرپېښ);
  }

  معلوم.propagateNonNaN(معلوم_Src);

  // Sign is preserved, but underflows may produce zeroes.
  إذا (معلوم_Src.is_معلوم_Never(fcNegative))
    معلوم.معلوم_Not(fcNegative);
  وإلا إذا (معلوم_Src.cannotBeOrderedLessThanZero())
    معلوم.معلوم_Not(Ordered_Less_Than_Zero_ماسک);

  إذا (معلوم_Src.is_معلوم_Never(fc_مثبت))
    معلوم.معلوم_Not(fc_مثبت);
  وإلا إذا (معلوم_Src.cannotBeOrderedGreaterThanZero())
    معلوم.معلوم_Not(Ordered_Greater_Than_Zero_ماسک);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  إذا (&Flt != &APFloat::PPCDoubleDouble()) {
    بینښه Precision = APFloat::semanticsPrecision(Flt);
    ثابت صحيح Mantissa_بټونه = Precision - 1;
    إذا (ConstantRangeExpMin.sge(Mantissa_بټونه))
      معلوم.معلوم_Not(fcSubnormal);
  }

  إذا (ConstantRangeExpMax.is_Non_مثبت()) {
    // If we know the power is <= 0, can't introduce inf
    إذا (معلوم_Src.is_معلوم_Never_Pos_Infinity())
      معلوم.معلوم_Not(fcPosInf);
    إذا (معلوم_Src.is_معلوم_Never_Neg_Infinity())
      معلوم.معلوم_Not(fcNegInf);
  } وإلا إذا (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    إذا (معلوم_Src.is_معلوم_Never_Pos_Subnormal())
      معلوم.معلوم_Not(fcPosSubnormal);
    إذا (معلوم_Src.is_معلوم_Never_Neg_Subnormal())
      معلوم.معلوم_Not(fcNegSubnormal);
    إذا (معلوم_Src.is_معلوم_Never_Pos_Zero())
      معلوم.معلوم_Not(fcPosZero);
    إذا (معلوم_Src.is_معلوم_Never_Neg_Zero())
      معلوم.معلوم_Not(fcNegZero);
  }

  أرجع apply_Output_Denormal_ډېرپېښ(معلوم, ډېرپېښ);
}

معلوم_FPClass معلوم_FPClass::ldexp(ثابت معلوم_FPClass &معلوم_Src,
                                 ثابت معلوم_بټونه &Exp_بټونه,
                                 ثابت fltSemantics &Flt, Denormal_ډېرپېښ ډېرپېښ) {
  أرجع ldexp(معلوم_Src, Exp_بټونه.get_Signed_Min_ارزښت(),
               Exp_بټونه.get_Signed_Max_ارزښت(), Flt, ډېرپېښ);
}

معلوم_FPClass معلوم_FPClass::توان(ثابت معلوم_FPClass &معلوم_LHS_2,
                               ثابت معلوم_FPClass &معلوم_RHS_2) {
  معلوم_FPClass معلوم;

  معلوم.propagateNonSNaN(معلوم_LHS_2, معلوم_RHS_2);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  إذا (معلوم_LHS_2.is_معلوم_Never(fcNan | fc_Neg_نورمال | fcNegSubnormal) &&
      معلوم_RHS_2.is_معلوم_Never_Na_N())
    معلوم.معلوم_Not(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  إذا (معلوم_LHS_2.is_معلوم_Never(fcNegative)) {
    معلوم.معلوم_Not(fcNegative);
  } وإلا إذا (معلوم_LHS_2.is_معلوم_Never(fc_Neg_نورمال | fcNegSubnormal)) {
    معلوم.معلوم_Not(fc_Neg_نورمال | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    إذا ((معلوم_LHS_2.is_معلوم_Never(fcNegZero) ||
         معلوم_RHS_2.is_معلوم_Never(fc_Pos_نورمال)) &&
        (معلوم_LHS_2.is_معلوم_Never(fcNegInf) || معلوم_RHS_2.is_معلوم_Never(fc_Neg_نورمال)))
      معلوم.معلوم_Not(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    إذا ((معلوم_LHS_2.is_معلوم_Never(fcNegZero) ||
         معلوم_RHS_2.is_معلوم_Never(fc_Neg_نورمال)) &&
        (معلوم_LHS_2.is_معلوم_Never(fcNegInf) || معلوم_RHS_2.is_معلوم_Never(fc_Pos_نورمال)))
      معلوم.معلوم_Not(fcNegInf);
  }

  أرجع معلوم;
}

معلوم_FPClass معلوم_FPClass::powi(ثابت معلوم_FPClass &معلوم_Src,
                                ثابت معلوم_بټونه &Exponent_معلوم_بټونه) {
  معلوم_FPClass معلوم;
  معلوم.propagateNonNaN(معلوم_Src);

  إذا (Exponent_معلوم_بټونه.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    إذا (معلوم_Src.is_معلوم_Never(fcSNan)) {
      معلوم.معلوم_Not(~fc_Pos_نورمال);
      أرجع معلوم;
    }

    معلوم.معلوم_Not(~(fc_Pos_نورمال | fcNan));
    أرجع معلوم;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  إذا (معلوم_Src.is_معلوم_Never(fcNegative) || Exponent_معلوم_بټونه.isEven()) {
    معلوم.معلوم_Not(fcNegative);
  } وإلا إذا (معلوم_Src.is_معلوم_Never(fc_Neg_نورمال | fcNegSubnormal)) {
    معلوم.معلوم_Not(fc_Neg_نورمال | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    ثابت منطقي Is_معلوم_Never_Odd_مثبت = Exponent_معلوم_بټونه.isNegative();
    ثابت منطقي Is_معلوم_Never_Odd_Negative = Exponent_معلوم_بټونه.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    إذا ((معلوم_Src.is_معلوم_Never(fcNegZero) || Is_معلوم_Never_Odd_مثبت) &&
        (معلوم_Src.is_معلوم_Never(fcNegInf) || Is_معلوم_Never_Odd_Negative))
      معلوم.معلوم_Not(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    إذا ((معلوم_Src.is_معلوم_Never(fcNegZero) || Is_معلوم_Never_Odd_Negative) &&
        (معلوم_Src.is_معلوم_Never(fcNegInf) || Is_معلوم_Never_Odd_مثبت))
      معلوم.معلوم_Not(fcNegInf);
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
    APInt MinExp = Exponent_معلوم_بټونه.get_Signed_Min_ارزښت();
    APInt MaxExp = Exponent_معلوم_بټونه.get_Signed_Max_ارزښت();

    // powi(inf, exp), exp > 0
    منطقي MayInfSrc =
        !معلوم_Src.is_معلوم_Never(fcInf) && MaxExp.is_Strictly_مثبت();

    // powi(+/-0, exp), exp < 0
    منطقي MayDivByZero = !معلوم_Src.is_معلوم_Never(fcZero) && MinExp.isNegative();

    // powi(finite, exp), |exp| > 1
    منطقي MayFinite = !معلوم_Src.is_معلوم_Never(fc_نورمال | fcSubnormal);
    منطقي MayAbsExpGT1 = MinExp.slt(-1) || MaxExp.sgt(1);
    منطقي May_Finite_ډکېدنه = MayFinite && MayAbsExpGT1;

    // powi(subnormal, -1)
    منطقي MayBeNegOne = Exponent_معلوم_بټونه.Zero.isZero();
    منطقي MaySubnormInv = !معلوم_Src.is_معلوم_Never(fcSubnormal) && MayBeNegOne;

    إذا (!MayInfSrc && !MayDivByZero && !May_Finite_ډکېدنه && !MaySubnormInv)
      معلوم.معلوم_Not(fcInf);
  }

  أرجع معلوم;
}
