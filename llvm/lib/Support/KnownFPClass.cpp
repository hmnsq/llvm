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

#شامل "llvm/Support/KnownFPClass.h"
#شامل "llvm/ADT/APFloat.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/KnownBits.h"

استعمال نامخلا llvm;

معلوم_FPClass::معلوم_FPClass(ثابت APFloat &C)
    : معلوم_FPClasses_قدر(C.classify()) {
  سیٹ_Sign_ثنائی_ہندسہ(C.isNegative());
}

معلوم_FPClass معلوم_FPClass::apply_داخلہ_Denormal_منوال(ثابت معلوم_FPClass &معلوم_Src,
                                                  Denormal_منوال منوال) {
  معلوم_FPClass معلوم = معلوم_Src;
  انتخاب (منوال.داخلہ) {
  حالت Denormal_منوال::IEEE:
    واپس معلوم;
  حالت Denormal_منوال::PreserveSign:
    اگر (معلوم_Src.get_معلوم_FPClasses() & fcPosSubnormal)
      معلوم.سیٹ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcPosZero);
    اگر (معلوم_Src.get_معلوم_FPClasses() & fcNegSubnormal)
      معلوم.سیٹ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcNegZero);
    واپس معلوم;
  حالت Denormal_منوال::مثبت_Zero:
    اگر (معلوم_Src.get_معلوم_FPClasses() & fcSubnormal) {
      معلوم.سیٹ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcPosZero);
      اگر (معلوم_Src.get_معلوم_FPClasses() & fcNegSubnormal)
        معلوم.سیٹ_Sign_ثنائی_ہندسہ(std::nullopt);
    }
    واپس معلوم;
  طےشدہ:
    اگر (معلوم_Src.get_معلوم_FPClasses() & fcSubnormal) {
      معلوم.سیٹ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcPosZero);
      اگر (معلوم_Src.get_معلوم_FPClasses() & fcNegSubnormal)
        معلوم.سیٹ_Sign_ثنائی_ہندسہ(std::nullopt);
    }
    اگر (معلوم_Src.get_معلوم_FPClasses() & fcNegSubnormal)
      معلوم.سیٹ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcNegZero);
    واپس معلوم;
  }

  llvm_unreachable("covered switch over denormal mode");
}

معلوم_FPClass معلوم_FPClass::apply_Output_Denormal_منوال(ثابت معلوم_FPClass &معلوم_Src,
                                                   Denormal_منوال منوال) {
  معلوم_FPClass معلوم = معلوم_Src;
  انتخاب (منوال.Output) {
  حالت Denormal_منوال::IEEE:
    واپس معلوم;
  حالت Denormal_منوال::PreserveSign:
    اگر (معلوم_Src.get_معلوم_FPClasses() & fcPosSubnormal)
      معلوم.سیٹ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcPosZero);
    اگر (معلوم_Src.get_معلوم_FPClasses() & fcNegSubnormal)
      معلوم.سیٹ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcNegZero);
    واپس معلوم;
  حالت Denormal_منوال::مثبت_Zero:
    اگر (معلوم_Src.get_معلوم_FPClasses() & fcSubnormal) {
      معلوم.سیٹ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcPosZero);
      اگر (معلوم_Src.get_معلوم_FPClasses() & fcNegSubnormal)
        معلوم.سیٹ_Sign_ثنائی_ہندسہ(std::nullopt);
    }
    واپس معلوم;
  طےشدہ:
    اگر (معلوم_Src.get_معلوم_FPClasses() & fcSubnormal) {
      معلوم.سیٹ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcPosZero);
      اگر (معلوم_Src.get_معلوم_FPClasses() & fcNegSubnormal)
        معلوم.سیٹ_Sign_ثنائی_ہندسہ(std::nullopt);
    }
    اگر (معلوم_Src.get_معلوم_FPClasses() & fcNegSubnormal)
      معلوم.سیٹ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fcNegZero);
    واپس معلوم;
  }

  llvm_unreachable("covered switch over denormal mode");
}

منطقی معلوم_FPClass::is_معلوم_Never_Logical_Zero(Denormal_منوال منوال) ثابت {
  واپس apply_داخلہ_Denormal_منوال(*یہ, منوال).is_معلوم_Never_Zero();
}

منطقی معلوم_FPClass::is_معلوم_Never_Logical_Neg_Zero(Denormal_منوال منوال) ثابت {
  واپس apply_داخلہ_Denormal_منوال(*یہ, منوال).is_معلوم_Never_Neg_Zero();
}

منطقی معلوم_FPClass::is_معلوم_Never_Logical_Pos_Zero(Denormal_منوال منوال) ثابت {
  واپس apply_داخلہ_Denormal_منوال(*یہ, منوال).is_معلوم_Never_Pos_Zero();
}

معلوم_FPClass معلوم_FPClass::minMaxLike(ثابت معلوم_FPClass &معلوم_LHS,
                                      ثابت معلوم_FPClass &معلوم_RHS,
                                      Min_Max_قسم قسم_3, Denormal_منوال منوال) {
  معلوم_FPClass معلوم_LHS_2 = apply_داخلہ_Denormal_منوال(معلوم_LHS, منوال);
  معلوم_FPClass معلوم_RHS_2 = apply_داخلہ_Denormal_منوال(معلوم_RHS, منوال);

  معلوم_FPClass معلوم;

  معلوم.propagateNonNaN(معلوم_LHS_2, معلوم_RHS_2);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  اگر ((معلوم_LHS_2.is_معلوم_Never_Na_N() || معلوم_RHS_2.is_معلوم_Never_Na_N()) &&
      (قسم_3 == Min_Max_قسم::minnum || قسم_3 == Min_Max_قسم::maxnum ||
       قسم_3 == Min_Max_قسم::minimumnum || قسم_3 == Min_Max_قسم::maximumnum))
    معلوم.معلوم_Not(fcNan);

  انتخاب (قسم_3) {
  حالت Min_Max_قسم::maxnum: {
    اگر (معلوم_LHS_2.is_معلوم_Never_Na_N())
      معلوم.معلوم_Not(orderedStrictlyLess(معلوم_LHS_2.get_معلوم_FPClasses(),
                                         /*OrderedZeroSign=*/جھوٹ));
    اگر (معلوم_RHS_2.is_معلوم_Never_Na_N())
      معلوم.معلوم_Not(orderedStrictlyLess(معلوم_RHS_2.get_معلوم_FPClasses(),
                                         /*OrderedZeroSign=*/جھوٹ));
    توڑو;
  }
  حالت Min_Max_قسم::minnum: {
    اگر (معلوم_LHS_2.is_معلوم_Never_Na_N())
      معلوم.معلوم_Not(orderedStrictlyGreater(معلوم_LHS_2.get_معلوم_FPClasses(),
                                            /*OrderedZeroSign=*/جھوٹ));
    اگر (معلوم_RHS_2.is_معلوم_Never_Na_N())
      معلوم.معلوم_Not(orderedStrictlyGreater(معلوم_RHS_2.get_معلوم_FPClasses(),
                                            /*OrderedZeroSign=*/جھوٹ));
    توڑو;
  }
  حالت Min_Max_قسم::maximumnum: {
    اگر (معلوم_LHS_2.is_معلوم_Never_Na_N())
      معلوم.معلوم_Not(orderedStrictlyLess(معلوم_LHS_2.get_معلوم_FPClasses(),
                                         /*OrderedZeroSign=*/سچ));
    اگر (معلوم_RHS_2.is_معلوم_Never_Na_N())
      معلوم.معلوم_Not(orderedStrictlyLess(معلوم_RHS_2.get_معلوم_FPClasses(),
                                         /*OrderedZeroSign=*/سچ));
    توڑو;
  }
  حالت Min_Max_قسم::minimumnum: {
    اگر (معلوم_LHS_2.is_معلوم_Never_Na_N())
      معلوم.معلوم_Not(orderedStrictlyGreater(معلوم_LHS_2.get_معلوم_FPClasses(),
                                            /*OrderedZeroSign=*/سچ));
    اگر (معلوم_RHS_2.is_معلوم_Never_Na_N())
      معلوم.معلوم_Not(orderedStrictlyGreater(معلوم_RHS_2.get_معلوم_FPClasses(),
                                            /*OrderedZeroSign=*/سچ));
    توڑو;
  }
  حالت Min_Max_قسم::زیادہ_سے_زیادہ: {
    معلوم.معلوم_Not(orderedStrictlyLess(معلوم_LHS_2.get_معلوم_FPClasses(),
                                       /*OrderedZeroSign=*/سچ) |
                   orderedStrictlyLess(معلوم_RHS_2.get_معلوم_FPClasses(),
                                       /*OrderedZeroSign=*/سچ));
    توڑو;
  }
  حالت Min_Max_قسم::minimum: {
    معلوم.معلوم_Not(orderedStrictlyGreater(معلوم_LHS_2.get_معلوم_FPClasses(),
                                          /*OrderedZeroSign=*/سچ) |
                   orderedStrictlyGreater(معلوم_RHS_2.get_معلوم_FPClasses(),
                                          /*OrderedZeroSign=*/سچ));
    توڑو;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      معلوم_LHS_2.get_معلوم_FPClasses() | معلوم_RHS_2.get_معلوم_FPClasses() | fcNan;
  معلوم.معلوم_Not(~PossibleClasses);

  واپس apply_Output_Denormal_منوال(معلوم, منوال);
}

معلوم_FPClass معلوم_FPClass::canonicalize(ثابت معلوم_FPClass &معلوم_Src,
                                        Denormal_منوال Denorm_منوال) {
  معلوم_FPClass معلوم;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  معلوم.سیٹ_معلوم_FPClasses(معلوم_Src.get_معلوم_FPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  اگر (معلوم_Src.is_معلوم_Never_Na_N())
    معلوم.معلوم_Not(fcNan);
  ورنہ
    معلوم.معلوم_Not(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  اگر (Denorm_منوال == Denormal_منوال::getIEEE()) {
    اگر (معلوم_Src.is_معلوم_Never(fcPosZero))
      معلوم.معلوم_Not(fcPosZero);
    اگر (معلوم_Src.is_معلوم_Never(fcNegZero))
      معلوم.معلوم_Not(fcNegZero);
    واپس معلوم;
  }

  اگر (Denorm_منوال.inputsAreZero() || Denorm_منوال.outputsAreZero())
    معلوم.معلوم_Not(fcSubnormal);

  اگر (Denorm_منوال == Denormal_منوال::getPreserveSign()) {
    اگر (معلوم_Src.is_معلوم_Never(fcPosZero | fcPosSubnormal))
      معلوم.معلوم_Not(fcPosZero);
    اگر (معلوم_Src.is_معلوم_Never(fcNegZero | fcNegSubnormal))
      معلوم.معلوم_Not(fcNegZero);
    واپس معلوم;
  }

  اگر (Denorm_منوال.داخلہ == Denormal_منوال::مثبت_Zero ||
      (Denorm_منوال.Output == Denormal_منوال::مثبت_Zero &&
       Denorm_منوال.داخلہ == Denormal_منوال::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    اگر (معلوم_Src.is_معلوم_Never(fcNegZero))
      معلوم.معلوم_Not(fcNegZero);

    اگر (معلوم_Src.is_معلوم_Never(fcPosZero | fcSubnormal))
      معلوم.معلوم_Not(fcPosZero);
  }

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::bitcast(ثابت fltSemantics &FltSemantics,
                                   ثابت معلوم_بٹ &بٹ_2) {
  أكد(FltSemantics.سائز_اندر_بٹ == بٹ_2.get_ثنائی_ہندسہ_چوڑائی() &&
         "Bitcast operand has incorrect bit width");
  معلوم_FPClass معلوم;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  اگر (بٹ_2.hasConflict())
    واپس معلوم;

  // Return unknown for types we have not validated.
  خودکار IsSupported = [](ثابت fltSemantics &Semantics) {
    انتخاب (APFloat::Semantics_مقصد_Enum(Semantics)) {
    حالت APFloat_بنیاد::S_IEEEhalf:
    حالت APFloat_بنیاد::S_BFloat:
    حالت APFloat_بنیاد::S_IEEEsingle:
    حالت APFloat_بنیاد::S_IEEEdouble:
    حالت APFloat_بنیاد::S_IEEEquad:
    حالت APFloat_بنیاد::S_x87DoubleExtended:
      واپس سچ;
    طےشدہ:
      واپس جھوٹ;
    }
  };
  اگر (!IsSupported(FltSemantics))
    واپس معلوم;

  // Transfer information from the sign bit.
  اگر (بٹ_2.isNonNegative())
    معلوم.sign_ثنائی_ہندسہ_Must_Be_Zero();
  ورنہ اگر (بٹ_2.isNegative())
    معلوم.sign_ثنائی_ہندسہ_Must_Be_One();

  انتخاب (APFloat_بنیاد::Semantics_مقصد_Enum(FltSemantics)) {
  حالت APFloat_بنیاد::S_IEEEhalf:
  حالت APFloat_بنیاد::S_BFloat:
  حالت APFloat_بنیاد::S_IEEEsingle:
  حالت APFloat_بنیاد::S_IEEEdouble:
  حالت APFloat_بنیاد::S_IEEEquad: {
    ثابت غیرعلامتی Mantissa_بٹ = FltSemantics.precision - 1;
    ثابت APInt Exponent_نقاب = APInt::get_بٹ_سیٹ(
        FltSemantics.سائز_اندر_بٹ, Mantissa_بٹ, FltSemantics.سائز_اندر_بٹ - 1);
    ثابت APInt Mantissa_نقاب =
        APInt::get_کم_بٹ_سیٹ(FltSemantics.سائز_اندر_بٹ, Mantissa_بٹ);

    ثابت منطقی Exponent_معلوم_All_Zeros =
        (بٹ_2.Zero & Exponent_نقاب) == Exponent_نقاب;
    ثابت منطقی Exponent_معلوم_All_Ones = (بٹ_2.One & Exponent_نقاب) == Exponent_نقاب;
    ثابت منطقی Exponent_معلوم_Not_All_Zeros = !(بٹ_2.One & Exponent_نقاب).isZero();
    ثابت منطقی Exponent_معلوم_Not_All_Ones = !(بٹ_2.Zero & Exponent_نقاب).isZero();

    ثابت منطقی Mantissa_معلوم_All_Zeros =
        (بٹ_2.Zero & Mantissa_نقاب) == Mantissa_نقاب;
    ثابت منطقی Mantissa_معلوم_Not_All_Zeros = !(بٹ_2.One & Mantissa_نقاب).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    اگر (Exponent_معلوم_Not_All_Zeros)
      معلوم.معلوم_Not(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    اگر (Exponent_معلوم_Not_All_Ones)
      معلوم.معلوم_Not(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    اگر (Exponent_معلوم_All_Zeros || Exponent_معلوم_All_Ones)
      معلوم.معلوم_Not(fc_معمول);

    // Zero and infinity require a mantissa with all zero bits.
    اگر (Mantissa_معلوم_Not_All_Zeros)
      معلوم.معلوم_Not(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    اگر (Mantissa_معلوم_All_Zeros)
      معلوم.معلوم_Not(fcSubnormal | fcNan);

    ثابت منطقی Quiet_ثنائی_ہندسہ_معلوم_سیٹ = بٹ_2.One[Mantissa_بٹ - 1];
    ثابت منطقی Quiet_ثنائی_ہندسہ_معلوم_Clear = بٹ_2.Zero[Mantissa_بٹ - 1];

    اگر (Quiet_ثنائی_ہندسہ_معلوم_سیٹ)
      معلوم.معلوم_Not(fcSNan);
    ورنہ اگر (Quiet_ثنائی_ہندسہ_معلوم_Clear)
      معلوم.معلوم_Not(fcQNan);

    توڑو;
  }
  حالت APFloat_بنیاد::S_x87DoubleExtended: {
    // Layout of the x87 floating point type:
    // [0:62] fraction
    // [63] explicit integer bit
    // [64:78] exponent
    // [79] sign bit
    // APFloat treats the x87 pseudo encodings as documented in
    // fltSemantics::hasExplicitIntegerBit.
    constexpr غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = 80;
    constexpr غیرعلامتی Fraction_بٹ = 63;
    constexpr غیرعلامتی Integer_ثنائی_ہندسہ_اشاریہ = Fraction_بٹ;
    ثابت APInt Fraction_نقاب = APInt::get_کم_بٹ_سیٹ(ثنائی_ہندسہ_چوڑائی_2, Fraction_بٹ);
    ثابت APInt Exponent_نقاب =
        APInt::get_بٹ_سیٹ(ثنائی_ہندسہ_چوڑائی_2, Fraction_بٹ + 1, ثنائی_ہندسہ_چوڑائی_2 - 1);

    ثابت منطقی Exponent_معلوم_All_Zeros =
        (بٹ_2.Zero & Exponent_نقاب) == Exponent_نقاب;
    ثابت منطقی Exponent_معلوم_All_Ones = (بٹ_2.One & Exponent_نقاب) == Exponent_نقاب;
    ثابت منطقی Exponent_معلوم_Not_All_Zeros = !(بٹ_2.One & Exponent_نقاب).isZero();
    ثابت منطقی Exponent_معلوم_Not_All_Ones = !(بٹ_2.Zero & Exponent_نقاب).isZero();
    ثابت منطقی Fraction_معلوم_All_Zeros =
        (بٹ_2.Zero & Fraction_نقاب) == Fraction_نقاب;
    ثابت منطقی Fraction_معلوم_Not_All_Zeros = !(بٹ_2.One & Fraction_نقاب).isZero();
    ثابت منطقی Integer_ثنائی_ہندسہ_معلوم_سیٹ = بٹ_2.One[Integer_ثنائی_ہندسہ_اشاریہ];
    ثابت منطقی Integer_ثنائی_ہندسہ_معلوم_Clear = بٹ_2.Zero[Integer_ثنائی_ہندسہ_اشاریہ];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    اگر (Exponent_معلوم_Not_All_Zeros || Integer_ثنائی_ہندسہ_معلوم_سیٹ)
      معلوم.معلوم_Not(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    اگر (Fraction_معلوم_Not_All_Zeros)
      معلوم.معلوم_Not(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    اگر (Exponent_معلوم_Not_All_Ones || Integer_ثنائی_ہندسہ_معلوم_Clear ||
        Fraction_معلوم_Not_All_Zeros)
      معلوم.معلوم_Not(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    اگر (Integer_ثنائی_ہندسہ_معلوم_Clear || Exponent_معلوم_All_Ones)
      معلوم.معلوم_Not(fc_معمول);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    اگر (Fraction_معلوم_All_Zeros)
      معلوم.معلوم_Not(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    اگر (Exponent_معلوم_All_Zeros ||
        (Integer_ثنائی_ہندسہ_معلوم_سیٹ &&
         (Exponent_معلوم_Not_All_Ones || Fraction_معلوم_All_Zeros)))
      معلوم.معلوم_Not(fcNan);

    ثابت منطقی Quiet_ثنائی_ہندسہ_معلوم_سیٹ = بٹ_2.One[Fraction_بٹ - 1];
    ثابت منطقی Quiet_ثنائی_ہندسہ_معلوم_Clear = بٹ_2.Zero[Fraction_بٹ - 1];

    اگر (Quiet_ثنائی_ہندسہ_معلوم_سیٹ)
      معلوم.معلوم_Not(fcSNan);
    ورنہ اگر (Quiet_ثنائی_ہندسہ_معلوم_Clear)
      معلوم.معلوم_Not(fcQNan);

    توڑو;
  }
  طےشدہ:
    llvm_unreachable("unhandled supported type");
  }

  واپس معلوم;
}

معلوم_بٹ معلوم_FPClass::مقصد_معلوم_بٹ(ثابت fltSemantics &FltSemantics) ثابت {
  معلوم_بٹ معلوم(FltSemantics.سائز_اندر_بٹ);
  ثابت FPClassTest FPClasses = get_معلوم_FPClasses();

  // Return unknown if poison.
  اگر (FPClasses == fcNone)
    واپس معلوم;

  // Return unknown for types we have not validated.
  خودکار IsSupported = [](ثابت fltSemantics &Semantics) {
    انتخاب (APFloat::Semantics_مقصد_Enum(Semantics)) {
    حالت APFloat_بنیاد::S_IEEEhalf:
    حالت APFloat_بنیاد::S_BFloat:
    حالت APFloat_بنیاد::S_IEEEsingle:
    حالت APFloat_بنیاد::S_IEEEdouble:
    حالت APFloat_بنیاد::S_IEEEquad:
    حالت APFloat_بنیاد::S_x87DoubleExtended:
      واپس سچ;
    طےشدہ:
      واپس جھوٹ;
    }
  };
  اگر (!IsSupported(FltSemantics))
    واپس معلوم;

  انتخاب (APFloat::Semantics_مقصد_Enum(FltSemantics)) {
  حالت APFloat_بنیاد::S_IEEEhalf:
  حالت APFloat_بنیاد::S_BFloat:
  حالت APFloat_بنیاد::S_IEEEsingle:
  حالت APFloat_بنیاد::S_IEEEdouble:
  حالت APFloat_بنیاد::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    اگر (FPClasses & fc_معمول)
      توڑو;

    معلوم.سیٹ_All_Conflict();

    ثابت غیرعلامتی ثنائی_ہندسہ_چوڑائی_2 = FltSemantics.سائز_اندر_بٹ;
    ثابت غیرعلامتی Mantissa_بٹ = FltSemantics.precision - 1;
    ثابت غیرعلامتی Exponent_بٹ = ثنائی_ہندسہ_چوڑائی_2 - Mantissa_بٹ - 1;

    APInt Mantissa_نقاب = APInt::get_کم_بٹ_سیٹ(ثنائی_ہندسہ_چوڑائی_2, Mantissa_بٹ);
    APInt Exponent_نقاب =
        APInt::get_بٹ_سیٹ(ثنائی_ہندسہ_چوڑائی_2, Mantissa_بٹ, Mantissa_بٹ + Exponent_بٹ);

    ثابت غیرعلامتی Quiet_ثنائی_ہندسہ_اشاریہ = Mantissa_بٹ - 1;
    APInt Payload_نقاب = Mantissa_نقاب;
    Payload_نقاب.clear_ثنائی_ہندسہ(Quiet_ثنائی_ہندسہ_اشاریہ);

    اگر (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      معلوم.Zero &= ~Exponent_نقاب;
      // No individual payload bit is known.
      معلوم.Zero &= ~Payload_نقاب;
      معلوم.One &= ~Payload_نقاب;

      اگر (FPClasses & fcQNan)
        معلوم.Zero.clear_ثنائی_ہندسہ(Quiet_ثنائی_ہندسہ_اشاریہ);
      اگر (FPClasses & fcSNan)
        معلوم.One.clear_ثنائی_ہندسہ(Quiet_ثنائی_ہندسہ_اشاریہ);
    }
    اگر (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      معلوم.Zero &= ~Exponent_نقاب;
      // Mantissa bits cannot be ones.
      معلوم.One &= ~Mantissa_نقاب;
    }
    اگر (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      معلوم.One &= ~Exponent_نقاب;
      // Unknown mantissa.
      معلوم.One &= ~Mantissa_نقاب;
      معلوم.Zero &= ~Mantissa_نقاب;
    }
    اگر (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      معلوم.One &= ~Exponent_نقاب;
      // Mantissa cannot be ones.
      معلوم.One &= ~Mantissa_نقاب;
    }

    توڑو;
  }
  حالت APFloat_بنیاد::S_x87DoubleExtended: {
    اگر (!is_معلوم_Never(fc_معمول | fcSubnormal | fcNan))
      توڑو;

    معلوم.سیٹ_All_Conflict();

    اگر (FPClasses & fcInf)
      معلوم = معلوم.intersectWith(معلوم_بٹ::makeConstant(
          APFloat::getInf(FltSemantics).bitcast_مقصد_APInt()));

    اگر (FPClasses & fcZero)
      معلوم = معلوم.intersectWith(
          معلوم_بٹ::makeConstant(APInt::getZero(FltSemantics.سائز_اندر_بٹ)));

    توڑو;
  }
  طےشدہ:
    llvm_unreachable("unhandled supported semantics");
  }

  معلوم.Zero.clear_Sign_ثنائی_ہندسہ();
  معلوم.One.clear_Sign_ثنائی_ہندسہ();
  اگر (std::optional<منطقی> Sign = get_Sign_ثنائی_ہندسہ()) {
    اگر (*Sign)
      معلوم.makeNegative();
    ورنہ
      معلوم.makeNonNegative();
  }

  واپس معلوم;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
ساکن معلوم_FPClass fadd_impl(ثابت معلوم_FPClass &معلوم_LHS_2,
                              ثابت معلوم_FPClass &معلوم_RHS_2, منطقی IsFMA) {
  معلوم_FPClass معلوم;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  اگر (معلوم_LHS_2.is_معلوم_Never_Na_N() && معلوم_RHS_2.is_معلوم_Never_Na_N() &&
      (معلوم_LHS_2.is_معلوم_Never(fcPosInf) || معلوم_RHS_2.is_معلوم_Never(fcNegInf)) &&
      (معلوم_LHS_2.is_معلوم_Never(fcNegInf) || معلوم_RHS_2.is_معلوم_Never(fcPosInf)))
    معلوم.معلوم_Not(fcNan);

  اگر (معلوم_LHS_2.cannotBeOrderedLessThanZero() &&
      معلوم_RHS_2.cannotBeOrderedLessThanZero()) {
    معلوم.معلوم_Not(معلوم_FPClass::Ordered_Less_Than_Zero_نقاب);

    // This can't underflow if one of the operands is known normal.
    اگر (معلوم_LHS_2.is_معلوم_Never(fcZero | fcPosSubnormal) ||
        معلوم_RHS_2.is_معلوم_Never(fcZero | fcPosSubnormal))
      معلوم.معلوم_Not(fcZero | fcPosSubnormal);
  }

  اگر (معلوم_LHS_2.cannotBeOrderedGreaterThanZero() &&
      معلوم_RHS_2.cannotBeOrderedGreaterThanZero()) {
    معلوم.معلوم_Not(معلوم_FPClass::Ordered_Greater_Than_Zero_نقاب);

    // This can't underflow if one of the operands is known normal.
    اگر (معلوم_LHS_2.is_معلوم_Never(fcZero | fcNegSubnormal) ||
        معلوم_RHS_2.is_معلوم_Never(fcZero | fcNegSubnormal))
      معلوم.معلوم_Not(fcZero | fcNegSubnormal);
  }

  اگر (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    اگر (معلوم_LHS_2.is_معلوم_Never(fcNegZero) || معلوم_RHS_2.is_معلوم_Never(fcNegZero))
      معلوم.معلوم_Not(fcNegZero);

    معلوم.propagateNonSNaN(معلوم_LHS_2, معلوم_RHS_2);
  }

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::fadd(ثابت معلوم_FPClass &معلوم_LHS,
                                ثابت معلوم_FPClass &معلوم_RHS,
                                Denormal_منوال منوال) {
  معلوم_FPClass معلوم_LHS_2 = apply_داخلہ_Denormal_منوال(معلوم_LHS, منوال);
  معلوم_FPClass معلوم_RHS_2 = apply_داخلہ_Denormal_منوال(معلوم_RHS, منوال);

  معلوم_FPClass معلوم = fadd_impl(معلوم_LHS_2, معلوم_RHS_2, /*IsFMA=*/جھوٹ);

  واپس apply_Output_Denormal_منوال(معلوم, منوال);
}

معلوم_FPClass معلوم_FPClass::fadd_self(ثابت معلوم_FPClass &معلوم_Src_2,
                                     Denormal_منوال منوال) {
  معلوم_FPClass معلوم_Src = apply_داخلہ_Denormal_منوال(معلوم_Src_2, منوال);

  معلوم_FPClass معلوم = fadd_impl(معلوم_Src, معلوم_Src, /*IsFMA=*/جھوٹ);

  اگر (معلوم_Src.is_معلوم_Never(fcPosSubnormal))
    معلوم.معلوم_Not(fcPosSubnormal);
  اگر (معلوم_Src.is_معلوم_Never(fcNegSubnormal))
    معلوم.معلوم_Not(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  اگر (معلوم_Src.is_معلوم_Never(fcPosZero))
    معلوم.معلوم_Not(fcPosZero);

  واپس apply_Output_Denormal_منوال(معلوم, منوال);
}

معلوم_FPClass معلوم_FPClass::fsub(ثابت معلوم_FPClass &معلوم_LHS,
                                ثابت معلوم_FPClass &معلوم_RHS,
                                Denormal_منوال منوال) {
  معلوم_FPClass معلوم_LHS_2 = apply_داخلہ_Denormal_منوال(معلوم_LHS, منوال);
  معلوم_FPClass معلوم_RHS_2 = apply_داخلہ_Denormal_منوال(معلوم_RHS, منوال);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  معلوم_RHS_2.fneg();

  معلوم_FPClass معلوم = fadd_impl(معلوم_LHS_2, معلوم_RHS_2, /*IsFMA=*/جھوٹ);

  واپس apply_Output_Denormal_منوال(معلوم, منوال);
}

معلوم_FPClass معلوم_FPClass::fmul(ثابت معلوم_FPClass &معلوم_LHS,
                                ثابت معلوم_FPClass &معلوم_RHS,
                                Denormal_منوال منوال) {
  معلوم_FPClass معلوم_LHS_2 = apply_داخلہ_Denormal_منوال(معلوم_LHS, منوال);
  معلوم_FPClass معلوم_RHS_2 = apply_داخلہ_Denormal_منوال(معلوم_RHS, منوال);

  معلوم_FPClass معلوم;

  معلوم.propagateNonSNaN(معلوم_LHS_2, معلوم_RHS_2);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  اگر ((معلوم_LHS_2.is_معلوم_Never(fcNegative) &&
       معلوم_RHS_2.is_معلوم_Never(fcNegative)) ||
      (معلوم_LHS_2.is_معلوم_Never(fc_مثبت) && معلوم_RHS_2.is_معلوم_Never(fc_مثبت)))
    معلوم.معلوم_Not(fcNegative);
  اگر ((معلوم_LHS_2.is_معلوم_Never(fc_مثبت) &&
       معلوم_RHS_2.is_معلوم_Never(fcNegative)) ||
      (معلوم_LHS_2.is_معلوم_Never(fcNegative) && معلوم_RHS_2.is_معلوم_Never(fc_مثبت)))
    معلوم.معلوم_Not(fc_مثبت);

  // Inf * Y => Inf or NaN
  اگر (معلوم_LHS_2.is_معلوم_Always(fcInf | fcNan) ||
      معلوم_RHS_2.is_معلوم_Always(fcInf | fcNan))
    معلوم.معلوم_Not(fc_معمول | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  اگر (معلوم_RHS_2.is_معلوم_Always(fcZero | fcNan) ||
      معلوم_LHS_2.is_معلوم_Always(fcZero | fcNan))
    معلوم.معلوم_Not(fc_معمول | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  اگر (معلوم_LHS_2.is_معلوم_Never_Na_N() && معلوم_RHS_2.is_معلوم_Never_Na_N() &&
      (معلوم_RHS_2.is_معلوم_Never_Infinity() || معلوم_LHS_2.is_معلوم_Never_Zero()) &&
      (معلوم_LHS_2.is_معلوم_Never_Infinity() || معلوم_RHS_2.is_معلوم_Never_Zero()))
    معلوم.معلوم_Not(fcNan);

  واپس apply_Output_Denormal_منوال(معلوم, منوال);
}

// TODO: This generalizes to known ranges
معلوم_FPClass معلوم_FPClass::fmul(ثابت معلوم_FPClass &معلوم_LHS_2,
                                ثابت APFloat &CRHS, Denormal_منوال منوال) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  ثابت fltSemantics &Flt = CRHS.getSemantics();
  غیرعلامتی Precision = APFloat::semanticsPrecision(Flt);
  ثابت صحیح Mantissa_بٹ = Precision - 1;

  صحیح Min_معلوم_Exponent = ilogb(CRHS);
  منطقی CannotBeSubnormal = (Min_معلوم_Exponent >= Mantissa_بٹ);

  معلوم_FPClass معلوم = معلوم_FPClass::fmul(معلوم_LHS_2, معلوم_FPClass(CRHS), منوال);
  اگر (CannotBeSubnormal)
    معلوم.معلوم_Not(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  اگر (معلوم_LHS_2.is_معلوم_Never(fcInf)) {
    اگر (Min_معلوم_Exponent < 0)
      معلوم.معلوم_Not(fcInf);
    ورنہ اگر (Min_معلوم_Exponent == 0 && CRHS.compare_Absolute_قدر(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      معلوم.معلوم_Not(fcInf);
  }

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::fdiv(ثابت معلوم_FPClass &معلوم_LHS,
                                ثابت معلوم_FPClass &معلوم_RHS,
                                Denormal_منوال منوال) {
  معلوم_FPClass معلوم_LHS_2 = apply_داخلہ_Denormal_منوال(معلوم_LHS, منوال);
  معلوم_FPClass معلوم_RHS_2 = apply_داخلہ_Denormal_منوال(معلوم_RHS, منوال);

  معلوم_FPClass معلوم;

  معلوم.propagateNonSNaN(معلوم_LHS_2, معلوم_RHS_2);

  // Only 0/0, Inf/Inf produce NaN.
  اگر (معلوم_LHS_2.is_معلوم_Never_Na_N() && معلوم_RHS_2.is_معلوم_Never_Na_N() &&
      (معلوم_LHS_2.is_معلوم_Never_Infinity() || معلوم_RHS_2.is_معلوم_Never_Infinity()) &&
      (معلوم_LHS_2.is_معلوم_Never_Zero() || معلوم_RHS_2.is_معلوم_Never_Zero())) {
    معلوم.معلوم_Not(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  اگر ((معلوم_LHS_2.is_معلوم_Never(fcNegative) &&
       معلوم_RHS_2.is_معلوم_Never(fcNegative)) ||
      (معلوم_LHS_2.is_معلوم_Never(fc_مثبت) && معلوم_RHS_2.is_معلوم_Never(fc_مثبت)))
    معلوم.معلوم_Not(fcNegative);
  اگر ((معلوم_LHS_2.is_معلوم_Never(fc_مثبت) &&
       معلوم_RHS_2.is_معلوم_Never(fcNegative)) ||
      (معلوم_LHS_2.is_معلوم_Never(fcNegative) && معلوم_RHS_2.is_معلوم_Never(fc_مثبت)))
    معلوم.معلوم_Not(fc_مثبت);

  // Normal and subnormal results require two non-zero finite operands.
  اگر ((معلوم_LHS_2.is_معلوم_Never(fc_Neg_معمول | fcNegSubnormal) &&
       معلوم_RHS_2.is_معلوم_Never(fc_Neg_معمول | fcNegSubnormal)) ||
      (معلوم_LHS_2.is_معلوم_Never(fc_Pos_معمول | fcPosSubnormal) &&
       معلوم_RHS_2.is_معلوم_Never(fc_Pos_معمول | fcPosSubnormal)))
    معلوم.معلوم_Not(fc_Neg_معمول | fcNegSubnormal);
  اگر ((معلوم_LHS_2.is_معلوم_Never(fc_Neg_معمول | fcNegSubnormal) &&
       معلوم_RHS_2.is_معلوم_Never(fc_Pos_معمول | fcPosSubnormal)) ||
      (معلوم_LHS_2.is_معلوم_Never(fc_Pos_معمول | fcPosSubnormal) &&
       معلوم_RHS_2.is_معلوم_Never(fc_Neg_معمول | fcNegSubnormal)))
    معلوم.معلوم_Not(fc_Pos_معمول | fcPosSubnormal);

  // 0 / X => 0 or NaN
  اگر (معلوم_LHS_2.is_معلوم_Always(fcZero))
    معلوم.معلوم_Not(fcSubnormal | fc_معمول | fcInf);

  // X / 0 => NaN or Inf
  اگر (معلوم_RHS_2.is_معلوم_Always(fcZero))
    معلوم.معلوم_Not(fcFinite);

  واپس apply_Output_Denormal_منوال(معلوم, منوال);
}

معلوم_FPClass معلوم_FPClass::fdiv_self(ثابت معلوم_FPClass &معلوم_Src,
                                     Denormal_منوال منوال) {
  // X / X is always exactly 1.0 or a NaN.
  معلوم_FPClass معلوم(fcNan | fc_Pos_معمول);

  معلوم.propagateNonSNaN(معلوم_Src);

  اگر (معلوم_Src.is_معلوم_Never_Inf_Or_Na_N() && معلوم_Src.is_معلوم_Never_Logical_Zero(منوال))
    معلوم.معلوم_Not(fcNan);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::frem(ثابت معلوم_FPClass &معلوم_LHS_2,
                                ثابت معلوم_FPClass &معلوم_RHS_2,
                                Denormal_منوال منوال) {
  معلوم_FPClass معلوم;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  معلوم.معلوم_Not(fcInf);

  معلوم.propagateNonSNaN(معلوم_LHS_2, معلوم_RHS_2);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  اگر (معلوم_LHS_2.is_معلوم_Never_Inf_Or_Na_N() && معلوم_RHS_2.is_معلوم_Never_Na_N() &&
      معلوم_RHS_2.is_معلوم_Never_Logical_Zero(منوال)) {
    معلوم.معلوم_Not(fcNan);
  }

  // The sign for frem is the same as the first operand.
  اگر (معلوم_LHS_2.is_معلوم_Never(fc_Pos_معمول | fcPosSubnormal))
    معلوم.معلوم_Not(fc_Pos_معمول | fcPosSubnormal);
  اگر (معلوم_LHS_2.is_معلوم_Never(fc_Neg_معمول | fcNegSubnormal))
    معلوم.معلوم_Not(fc_Neg_معمول | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  اگر (معلوم_LHS_2.is_معلوم_Never(fcNegFinite))
    معلوم.معلوم_Not(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  اگر (معلوم_LHS_2.is_معلوم_Never(fcPosFinite) &&
      معلوم_LHS_2.is_معلوم_Never_Logical_Pos_Zero(منوال) &&
      (!منوال.outputs_May_Be_مثبت_Zero() ||
       معلوم_LHS_2.is_معلوم_Never(fc_Neg_معمول | fcNegSubnormal)))
    معلوم.معلوم_Not(fcPosZero);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::frem_self(ثابت معلوم_FPClass &معلوم_Src,
                                     Denormal_منوال منوال) {
  // X % X is always exactly [+-]0.0 or a NaN.
  معلوم_FPClass معلوم(fcNan | fcZero);

  معلوم.propagateNonSNaN(معلوم_Src);

  اگر (معلوم_Src.is_معلوم_Never_Inf_Or_Na_N() && معلوم_Src.is_معلوم_Never_Logical_Zero(منوال))
    معلوم.معلوم_Not(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  اگر (معلوم_Src.is_معلوم_Never(fc_Pos_معمول | fcPosSubnormal))
    معلوم.معلوم_Not(fcPosZero);
  اگر (معلوم_Src.is_معلوم_Never(fc_Neg_معمول | fcNegSubnormal))
    معلوم.معلوم_Not(fcNegZero);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::fma(ثابت معلوم_FPClass &معلوم_LHS_2,
                               ثابت معلوم_FPClass &معلوم_RHS_2,
                               ثابت معلوم_FPClass &معلوم_Addend,
                               Denormal_منوال منوال) {
  معلوم_FPClass Mul = fmul(معلوم_LHS_2, معلوم_RHS_2, منوال);

  // FMA differs from the base fmul + fadd handling only in the treatment of -0
  // results.
  //
  // If the multiply is a -0 due to rounding, the final -0 + 0 will be -0,
  // unlike for a separate fadd.
  معلوم_FPClass معلوم = fadd_impl(Mul, معلوم_Addend, /*IsFMA=*/سچ);

  // propagateNonSNaN for 3 arguments.
  اگر (معلوم_LHS_2.is_معلوم_Never(fcSNan) && معلوم_RHS_2.is_معلوم_Never(fcSNan) &&
      معلوم_Addend.is_معلوم_Never(fcSNan))
    معلوم.معلوم_Not(fcSNan);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::fmuladd(ثابت معلوم_FPClass &معلوم_LHS_2,
                                   ثابت معلوم_FPClass &معلوم_RHS_2,
                                   ثابت معلوم_FPClass &معلوم_Addend,
                                   Denormal_منوال منوال) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  معلوم_FPClass FMA = fma(معلوم_LHS_2, معلوم_RHS_2, معلوم_Addend, منوال);
  معلوم_FPClass Mul_جوڑو = fadd(fmul(معلوم_LHS_2, معلوم_RHS_2, منوال), معلوم_Addend, منوال);
  واپس FMA.intersectWith(Mul_جوڑو);
}

معلوم_FPClass معلوم_FPClass::fma_مربع(ثابت معلوم_FPClass &معلوم_Squared,
                                      ثابت معلوم_FPClass &معلوم_Addend,
                                      Denormal_منوال منوال) {
  معلوم_FPClass Squared = مربع(معلوم_Squared, منوال);
  معلوم_FPClass معلوم = fadd_impl(Squared, معلوم_Addend, /*IsFMA=*/سچ);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  اگر (معلوم_Addend.is_معلوم_Never(fcNegInf | fcNan) && Squared.is_معلوم_Never(fcNan))
    معلوم.معلوم_Not(fcNan);

  معلوم.propagateNonSNaN(معلوم_Squared, معلوم_Addend);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::fmuladd_مربع(ثابت معلوم_FPClass &معلوم_Squared,
                                          ثابت معلوم_FPClass &معلوم_Addend,
                                          Denormal_منوال منوال) {
  معلوم_FPClass FMA = fma_مربع(معلوم_Squared, معلوم_Addend, منوال);
  معلوم_FPClass Mul_جوڑو = fadd(مربع(معلوم_Squared, منوال), معلوم_Addend, منوال);
  واپس FMA.intersectWith(Mul_جوڑو);
}

معلوم_FPClass معلوم_FPClass::قوت_نما(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;
  معلوم.معلوم_Not(fcNegative);

  معلوم.propagateNonNaN(معلوم_Src);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  اگر (معلوم_Src.is_معلوم_Never(fc_Neg_معمول | fcNegInf))
    معلوم.معلوم_Not(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  اگر (معلوم_Src.is_معلوم_Never(fc_Neg_معمول))
    معلوم.معلوم_Not(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  اگر (معلوم_Src.is_معلوم_Never(fc_Pos_معمول | fcPosInf))
    معلوم.معلوم_Not(fcPosInf);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::لوگارتھم(ثابت معلوم_FPClass &معلوم_Src, Denormal_منوال منوال,
                               منطقی IsMultiUnitFPType) {
  معلوم_FPClass معلوم;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  اگر (!IsMultiUnitFPType)
    معلوم.معلوم_Not(fcNegZero | fcSubnormal);

  معلوم.propagateNonSNaN(معلوم_Src);

  اگر (معلوم_Src.is_معلوم_Never_Pos_Infinity())
    معلوم.معلوم_Not(fcPosInf);

  اگر (معلوم_Src.is_معلوم_Never_Na_N() && معلوم_Src.cannotBeOrderedLessThanZero())
    معلوم.معلوم_Not(fcNan);

  اگر (معلوم_Src.is_معلوم_Never_Logical_Zero(منوال))
    معلوم.معلوم_Not(fcNegInf);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::جذر(ثابت معلوم_FPClass &معلوم_Src,
                                Denormal_منوال منوال) {
  معلوم_FPClass معلوم;
  معلوم.معلوم_Not(fcPosSubnormal);

  اگر (معلوم_Src.is_معلوم_Never_Pos_Infinity())
    معلوم.معلوم_Not(fcPosInf);

  معلوم.propagateNonSNaN(معلوم_Src);

  // Any negative value besides -0 returns a nan.
  اگر (معلوم_Src.is_معلوم_Never_Na_N() && معلوم_Src.cannotBeOrderedLessThanZero())
    معلوم.معلوم_Not(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  معلوم.معلوم_Not(fcNegInf | fcNegSubnormal | fc_Neg_معمول);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  اگر (معلوم_Src.is_معلوم_Never_Logical_Neg_Zero(منوال))
    معلوم.معلوم_Not(fcNegZero);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::جیب(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  معلوم.propagateNonSNaN(معلوم_Src);

  // Return NaN on infinite inputs.
  معلوم.معلوم_Not(fcInf);
  اگر (معلوم_Src.is_معلوم_Never_Na_N() && معلوم_Src.is_معلوم_Never_Infinity())
    معلوم.معلوم_Not(fcNan);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::جیب_تمام(ثابت معلوم_FPClass &معلوم_Src) {
  واپس جیب(معلوم_Src);
}

معلوم_FPClass معلوم_FPClass::tan(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  معلوم.معلوم_Not(fcInf);

  معلوم.propagateNonSNaN(معلوم_Src);

  // NaN propagates. tan(+-Inf) is NaN.
  اگر (معلوم_Src.is_معلوم_Never_Na_N() && معلوم_Src.is_معلوم_Never_Infinity())
    معلوم.معلوم_Not(fcNan);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::sinh(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  اگر (معلوم_Src.is_معلوم_Never(fcNegative))
    معلوم.معلوم_Not(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  اگر (معلوم_Src.is_معلوم_Never(fcPosSubnormal | fc_Pos_معمول | fcPosInf))
    معلوم.معلوم_Not(fcPosSubnormal | fc_Pos_معمول | fcPosInf);

  معلوم.propagateNonNaN(معلوم_Src);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::cosh(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  معلوم.معلوم_Not(fcNegative | fcZero | fcSubnormal);

  معلوم.propagateNonNaN(معلوم_Src);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::tanh(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  // tanh is bounded to (-1, 1), never Inf.
  معلوم.معلوم_Not(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  اگر (معلوم_Src.is_معلوم_Never(fcNegSubnormal | fc_Neg_معمول | fcNegInf)) {
    معلوم.معلوم_Not(fcNegSubnormal | fc_Neg_معمول);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    اگر (معلوم_Src.is_معلوم_Never_Neg_Zero())
      معلوم.معلوم_Not(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  اگر (معلوم_Src.is_معلوم_Never(fcPosSubnormal | fc_Pos_معمول | fcPosInf))
    معلوم.معلوم_Not(fcPosSubnormal | fc_Pos_معمول);

  معلوم.propagateNonNaN(معلوم_Src);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::asin(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  معلوم.معلوم_Not(fcInf);

  معلوم.propagateNonSNaN(معلوم_Src);

  // asin is sign-preserving for finite arguments.
  اگر (معلوم_Src.is_معلوم_Never(fcNegFinite))
    معلوم.معلوم_Not(fcNegFinite);

  اگر (معلوم_Src.is_معلوم_Never(fcPosSubnormal | fc_Pos_معمول))
    معلوم.معلوم_Not(fcPosSubnormal | fc_Pos_معمول);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  واپس معلوم;
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
  اگر (معلوم_Src.is_معلوم_Never(fc_Pos_معمول))
    معلوم.معلوم_Not(fcZero);

  معلوم.propagateNonSNaN(معلوم_Src);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::atan(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  معلوم.معلوم_Not(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  اگر (معلوم_Src.is_معلوم_Never(fcNegSubnormal | fc_Neg_معمول | fcNegInf))
    معلوم.معلوم_Not(fcNegSubnormal | fc_Neg_معمول);

  اگر (معلوم_Src.is_معلوم_Never(fcPosSubnormal | fc_Pos_معمول | fcPosInf))
    معلوم.معلوم_Not(fcPosSubnormal | fc_Pos_معمول);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  اگر (معلوم_Src.is_معلوم_Never(fc_Neg_معمول | fcNegSubnormal | fcNegZero))
    معلوم.معلوم_Not(fcNegZero);

  معلوم.propagateNonNaN(معلوم_Src);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::atan2(ثابت معلوم_FPClass &معلوم_Y,
                                 ثابت معلوم_FPClass &معلوم_X,
                                 Denormal_منوال منوال) {
  معلوم_FPClass معلوم_Y_2 = apply_داخلہ_Denormal_منوال(معلوم_Y, منوال);
  معلوم_FPClass معلوم_X_2 = apply_داخلہ_Denormal_منوال(معلوم_X, منوال);
  معلوم_FPClass معلوم;

  // Even though these deductions are correct, we are ignoring the following
  // potentially erroneous cases:
  //   * atan2(y, inf) is not subnormal
  //   * atan2(inf, x) is not zero or subnormal

  // atan2 result is in (-pi, pi], never Inf.
  معلوم.معلوم_Not(fcInf);

  معلوم.propagateNonNaN(معلوم_Y_2, معلوم_X_2);

  // If x <= -0.0, then |atan2(y, x)| >= pi/2
  اگر (معلوم_X_2.is_معلوم_Never(fc_مثبت))
    معلوم.معلوم_Not(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  اگر (معلوم_Y_2.is_معلوم_Never(fcNegative))
    معلوم.معلوم_Not(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  اگر (معلوم_Y_2.is_معلوم_Never(fc_مثبت))
    معلوم.معلوم_Not(fc_مثبت);

  واپس apply_Output_Denormal_منوال(معلوم, منوال);
}

معلوم_FPClass معلوم_FPClass::fpext(ثابت معلوم_FPClass &معلوم_Src,
                                 ثابت fltSemantics &DstTy,
                                 ثابت fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  معلوم_FPClass معلوم = معلوم_Src;

  // All subnormal inputs should be in the normal range in the result type.
  اگر (APFloat::is_Representable_As_معمول_اندر(SrcTy, DstTy)) {
    اگر (معلوم.get_معلوم_FPClasses() & fcPosSubnormal)
      معلوم.سیٹ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fc_Pos_معمول);
    اگر (معلوم.get_معلوم_FPClasses() & fcNegSubnormal)
      معلوم.سیٹ_معلوم_FPClasses(معلوم.get_معلوم_FPClasses() | fc_Neg_معمول);
    معلوم.معلوم_Not(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  اگر (!معلوم.is_معلوم_Never_Na_N())
    معلوم.سیٹ_Sign_ثنائی_ہندسہ(std::nullopt);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::fptrunc(ثابت معلوم_FPClass &معلوم_Src) {
  معلوم_FPClass معلوم;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  اگر (معلوم_Src.cannotBeOrderedLessThanZero())
    معلوم.معلوم_Not(معلوم_FPClass::Ordered_Less_Than_Zero_نقاب);

  معلوم.propagateNonNaN(معلوم_Src);

  // Infinity needs a range check.
  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::باری_مقصد_Integral(ثابت معلوم_FPClass &معلوم_Src,
                                           منطقی MayBeMultiUnitFPType,
                                           Denormal_منوال منوال) {
  معلوم_FPClass معلوم;

  // Integer results cannot be subnormal.
  معلوم.معلوم_Not(fcSubnormal);

  معلوم.propagateNonNaN(معلوم_Src);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  اگر (!MayBeMultiUnitFPType) {
    اگر (معلوم_Src.is_معلوم_Never_Pos_Infinity())
      معلوم.معلوم_Not(fcPosInf);
    اگر (معلوم_Src.is_معلوم_Never_Neg_Infinity())
      معلوم.معلوم_Not(fcNegInf);
  }

  اگر (معلوم_Src.is_معلوم_Never(fc_Pos_معمول | fcPosSubnormal))
    معلوم.معلوم_Not(fc_Pos_معمول);

  اگر (معلوم_Src.is_معلوم_Never(fc_Neg_معمول | fcNegSubnormal))
    معلوم.معلوم_Not(fc_Neg_معمول);

  // Negative round ups towards zero produce negative zero.
  اگر (معلوم_Src.is_معلوم_Never(fcNegFinite))
    معلوم.معلوم_Not(fcNegZero);

  // Negative subnormals may flush to positive zero.
  اگر (معلوم_Src.is_معلوم_Never(fcPosFinite) &&
      (معلوم_Src.is_معلوم_Never(fcNegSubnormal) ||
       !منوال.inputs_May_Be_مثبت_Zero()))
    معلوم.معلوم_Not(fcPosZero);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::trunc(ثابت معلوم_FPClass &معلوم_Src,
                                 Denormal_منوال منوال) {
  معلوم_FPClass معلوم = باری_مقصد_Integral(معلوم_Src,
                                       /*MayBeMultiUnitFPType=*/سچ, منوال);

  اگر (معلوم_Src.is_معلوم_Never_Pos_Infinity())
    معلوم.معلوم_Not(fcPosInf);
  اگر (معلوم_Src.is_معلوم_Never_Neg_Infinity())
    معلوم.معلوم_Not(fcNegInf);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::floor(ثابت معلوم_FPClass &معلوم_Src,
                                 منطقی IsMultiUnitFPType, Denormal_منوال منوال) {
  معلوم_FPClass معلوم = باری_مقصد_Integral(معلوم_Src, IsMultiUnitFPType, منوال);

  // Applicable for multi-unit floating point types.
  اگر (معلوم_Src.is_معلوم_Never_Pos_Infinity())
    معلوم.معلوم_Not(fcPosInf);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::ceil(ثابت معلوم_FPClass &معلوم_Src,
                                منطقی IsMultiUnitFPType, Denormal_منوال منوال) {
  معلوم_FPClass معلوم = باری_مقصد_Integral(معلوم_Src, IsMultiUnitFPType, منوال);

  // Applicable for multi-unit floating point types.
  اگر (معلوم_Src.is_معلوم_Never_Neg_Infinity())
    معلوم.معلوم_Not(fcNegInf);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::frexp_mant(ثابت معلوم_FPClass &معلوم_Src,
                                      Denormal_منوال منوال) {
  معلوم_FPClass معلوم;

  معلوم.معلوم_Not(fcSubnormal);

  معلوم.propagateNonNaN(معلوم_Src);

  اگر (معلوم_Src.is_معلوم_Never_Pos_Infinity())
    معلوم.معلوم_Not(fcPosInf);

  اگر (معلوم_Src.is_معلوم_Never_Neg_Infinity())
    معلوم.معلوم_Not(fcNegInf);

  اگر (معلوم_Src.is_معلوم_Never_Logical_Pos_Zero(منوال))
    معلوم.معلوم_Not(fcPosZero);

  اگر (معلوم_Src.is_معلوم_Never_Logical_Neg_Zero(منوال))
    معلوم.معلوم_Not(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  اگر (معلوم_Src.is_معلوم_Never(fc_Pos_معمول | fcPosSubnormal))
    معلوم.معلوم_Not(fc_Pos_معمول);

  اگر (معلوم_Src.is_معلوم_Never(fc_Neg_معمول | fcNegSubnormal))
    معلوم.معلوم_Not(fc_Neg_معمول);

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::ldexp(ثابت معلوم_FPClass &معلوم_Src_2,
                                 ثابت APInt &ConstantRangeExpMin,
                                 ثابت APInt &ConstantRangeExpMax,
                                 ثابت fltSemantics &Flt, Denormal_منوال منوال) {
  معلوم_FPClass معلوم;

  معلوم_FPClass معلوم_Src = apply_داخلہ_Denormal_منوال(معلوم_Src_2, منوال);

  اگر (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    معلوم.سیٹ_معلوم_FPClasses(معلوم_Src.get_معلوم_FPClasses() | fcNan);
    معلوم.propagateNonNaN(معلوم_Src);
    واپس apply_Output_Denormal_منوال(معلوم, منوال);
  }

  معلوم.propagateNonNaN(معلوم_Src);

  // Sign is preserved, but underflows may produce zeroes.
  اگر (معلوم_Src.is_معلوم_Never(fcNegative))
    معلوم.معلوم_Not(fcNegative);
  ورنہ اگر (معلوم_Src.cannotBeOrderedLessThanZero())
    معلوم.معلوم_Not(Ordered_Less_Than_Zero_نقاب);

  اگر (معلوم_Src.is_معلوم_Never(fc_مثبت))
    معلوم.معلوم_Not(fc_مثبت);
  ورنہ اگر (معلوم_Src.cannotBeOrderedGreaterThanZero())
    معلوم.معلوم_Not(Ordered_Greater_Than_Zero_نقاب);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  اگر (&Flt != &APFloat::PPCDoubleDouble()) {
    غیرعلامتی Precision = APFloat::semanticsPrecision(Flt);
    ثابت صحیح Mantissa_بٹ = Precision - 1;
    اگر (ConstantRangeExpMin.sge(Mantissa_بٹ))
      معلوم.معلوم_Not(fcSubnormal);
  }

  اگر (ConstantRangeExpMax.is_Non_مثبت()) {
    // If we know the power is <= 0, can't introduce inf
    اگر (معلوم_Src.is_معلوم_Never_Pos_Infinity())
      معلوم.معلوم_Not(fcPosInf);
    اگر (معلوم_Src.is_معلوم_Never_Neg_Infinity())
      معلوم.معلوم_Not(fcNegInf);
  } ورنہ اگر (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    اگر (معلوم_Src.is_معلوم_Never_Pos_Subnormal())
      معلوم.معلوم_Not(fcPosSubnormal);
    اگر (معلوم_Src.is_معلوم_Never_Neg_Subnormal())
      معلوم.معلوم_Not(fcNegSubnormal);
    اگر (معلوم_Src.is_معلوم_Never_Pos_Zero())
      معلوم.معلوم_Not(fcPosZero);
    اگر (معلوم_Src.is_معلوم_Never_Neg_Zero())
      معلوم.معلوم_Not(fcNegZero);
  }

  واپس apply_Output_Denormal_منوال(معلوم, منوال);
}

معلوم_FPClass معلوم_FPClass::ldexp(ثابت معلوم_FPClass &معلوم_Src,
                                 ثابت معلوم_بٹ &Exp_بٹ,
                                 ثابت fltSemantics &Flt, Denormal_منوال منوال) {
  واپس ldexp(معلوم_Src, Exp_بٹ.get_Signed_Min_قدر(),
               Exp_بٹ.get_Signed_Max_قدر(), Flt, منوال);
}

معلوم_FPClass معلوم_FPClass::قوت(ثابت معلوم_FPClass &معلوم_LHS_2,
                               ثابت معلوم_FPClass &معلوم_RHS_2) {
  معلوم_FPClass معلوم;

  معلوم.propagateNonSNaN(معلوم_LHS_2, معلوم_RHS_2);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  اگر (معلوم_LHS_2.is_معلوم_Never(fcNan | fc_Neg_معمول | fcNegSubnormal) &&
      معلوم_RHS_2.is_معلوم_Never_Na_N())
    معلوم.معلوم_Not(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  اگر (معلوم_LHS_2.is_معلوم_Never(fcNegative)) {
    معلوم.معلوم_Not(fcNegative);
  } ورنہ اگر (معلوم_LHS_2.is_معلوم_Never(fc_Neg_معمول | fcNegSubnormal)) {
    معلوم.معلوم_Not(fc_Neg_معمول | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    اگر ((معلوم_LHS_2.is_معلوم_Never(fcNegZero) ||
         معلوم_RHS_2.is_معلوم_Never(fc_Pos_معمول)) &&
        (معلوم_LHS_2.is_معلوم_Never(fcNegInf) || معلوم_RHS_2.is_معلوم_Never(fc_Neg_معمول)))
      معلوم.معلوم_Not(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    اگر ((معلوم_LHS_2.is_معلوم_Never(fcNegZero) ||
         معلوم_RHS_2.is_معلوم_Never(fc_Neg_معمول)) &&
        (معلوم_LHS_2.is_معلوم_Never(fcNegInf) || معلوم_RHS_2.is_معلوم_Never(fc_Pos_معمول)))
      معلوم.معلوم_Not(fcNegInf);
  }

  واپس معلوم;
}

معلوم_FPClass معلوم_FPClass::powi(ثابت معلوم_FPClass &معلوم_Src,
                                ثابت معلوم_بٹ &Exponent_معلوم_بٹ) {
  معلوم_FPClass معلوم;
  معلوم.propagateNonNaN(معلوم_Src);

  اگر (Exponent_معلوم_بٹ.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    اگر (معلوم_Src.is_معلوم_Never(fcSNan)) {
      معلوم.معلوم_Not(~fc_Pos_معمول);
      واپس معلوم;
    }

    معلوم.معلوم_Not(~(fc_Pos_معمول | fcNan));
    واپس معلوم;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  اگر (معلوم_Src.is_معلوم_Never(fcNegative) || Exponent_معلوم_بٹ.isEven()) {
    معلوم.معلوم_Not(fcNegative);
  } ورنہ اگر (معلوم_Src.is_معلوم_Never(fc_Neg_معمول | fcNegSubnormal)) {
    معلوم.معلوم_Not(fc_Neg_معمول | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    ثابت منطقی Is_معلوم_Never_Odd_مثبت = Exponent_معلوم_بٹ.isNegative();
    ثابت منطقی Is_معلوم_Never_Odd_Negative = Exponent_معلوم_بٹ.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    اگر ((معلوم_Src.is_معلوم_Never(fcNegZero) || Is_معلوم_Never_Odd_مثبت) &&
        (معلوم_Src.is_معلوم_Never(fcNegInf) || Is_معلوم_Never_Odd_Negative))
      معلوم.معلوم_Not(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    اگر ((معلوم_Src.is_معلوم_Never(fcNegZero) || Is_معلوم_Never_Odd_Negative) &&
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
    APInt MinExp = Exponent_معلوم_بٹ.get_Signed_Min_قدر();
    APInt MaxExp = Exponent_معلوم_بٹ.get_Signed_Max_قدر();

    // powi(inf, exp), exp > 0
    منطقی MayInfSrc =
        !معلوم_Src.is_معلوم_Never(fcInf) && MaxExp.is_Strictly_مثبت();

    // powi(+/-0, exp), exp < 0
    منطقی MayDivByZero = !معلوم_Src.is_معلوم_Never(fcZero) && MinExp.isNegative();

    // powi(finite, exp), |exp| > 1
    منطقی MayFinite = !معلوم_Src.is_معلوم_Never(fc_معمول | fcSubnormal);
    منطقی MayAbsExpGT1 = MinExp.slt(-1) || MaxExp.sgt(1);
    منطقی May_Finite_حد_سے_تجاوز = MayFinite && MayAbsExpGT1;

    // powi(subnormal, -1)
    منطقی MayBeNegOne = Exponent_معلوم_بٹ.Zero.isZero();
    منطقی MaySubnormInv = !معلوم_Src.is_معلوم_Never(fcSubnormal) && MayBeNegOne;

    اگر (!MayInfSrc && !MayDivByZero && !May_Finite_حد_سے_تجاوز && !MaySubnormInv)
      معلوم.معلوم_Not(fcInf);
  }

  واپس معلوم;
}
