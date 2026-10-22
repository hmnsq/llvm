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

#अन्तर्भाव "llvm/Support/KnownFPClass.h"
#अन्तर्भाव "llvm/ADT/APFloat.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/KnownBits.h"

उपयुज्य नामाकाश llvm;

ज्ञातम्_FPClass::ज्ञातम्_FPClass(ध्रुव APFloat &C)
    : ज्ञातम्_FPClasses_मानम्(C.classify()) {
  समुच्चयः_Sign_Bit(C.isNegative());
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::apply_निवेशः_Denormal_बहुलकम्(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src,
                                                  Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम् = ज्ञातम्_Src;
  विकल्प (बहुलकम्.निवेशः) {
  पक्ष Denormal_बहुलकम्::IEEE:
    प्रत्यागम ज्ञातम्;
  पक्ष Denormal_बहुलकम्::PreserveSign:
    यदि (ज्ञातम्_Src.get_ज्ञातम्_FPClasses() & fcPosSubnormal)
      ज्ञातम्.समुच्चयः_ज्ञातम्_FPClasses(ज्ञातम्.get_ज्ञातम्_FPClasses() | fcPosZero);
    यदि (ज्ञातम्_Src.get_ज्ञातम्_FPClasses() & fcNegSubnormal)
      ज्ञातम्.समुच्चयः_ज्ञातम्_FPClasses(ज्ञातम्.get_ज्ञातम्_FPClasses() | fcNegZero);
    प्रत्यागम ज्ञातम्;
  पक्ष Denormal_बहुलकम्::धनात्मकम्_Zero:
    यदि (ज्ञातम्_Src.get_ज्ञातम्_FPClasses() & fcSubnormal) {
      ज्ञातम्.समुच्चयः_ज्ञातम्_FPClasses(ज्ञातम्.get_ज्ञातम्_FPClasses() | fcPosZero);
      यदि (ज्ञातम्_Src.get_ज्ञातम्_FPClasses() & fcNegSubnormal)
        ज्ञातम्.समुच्चयः_Sign_Bit(std::nullopt);
    }
    प्रत्यागम ज्ञातम्;
  स्वभाव:
    यदि (ज्ञातम्_Src.get_ज्ञातम्_FPClasses() & fcSubnormal) {
      ज्ञातम्.समुच्चयः_ज्ञातम्_FPClasses(ज्ञातम्.get_ज्ञातम्_FPClasses() | fcPosZero);
      यदि (ज्ञातम्_Src.get_ज्ञातम्_FPClasses() & fcNegSubnormal)
        ज्ञातम्.समुच्चयः_Sign_Bit(std::nullopt);
    }
    यदि (ज्ञातम्_Src.get_ज्ञातम्_FPClasses() & fcNegSubnormal)
      ज्ञातम्.समुच्चयः_ज्ञातम्_FPClasses(ज्ञातम्.get_ज्ञातम्_FPClasses() | fcNegZero);
    प्रत्यागम ज्ञातम्;
  }

  llvm_unreachable("covered switch over denormal mode");
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::apply_Output_Denormal_बहुलकम्(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src,
                                                   Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम् = ज्ञातम्_Src;
  विकल्प (बहुलकम्.Output) {
  पक्ष Denormal_बहुलकम्::IEEE:
    प्रत्यागम ज्ञातम्;
  पक्ष Denormal_बहुलकम्::PreserveSign:
    यदि (ज्ञातम्_Src.get_ज्ञातम्_FPClasses() & fcPosSubnormal)
      ज्ञातम्.समुच्चयः_ज्ञातम्_FPClasses(ज्ञातम्.get_ज्ञातम्_FPClasses() | fcPosZero);
    यदि (ज्ञातम्_Src.get_ज्ञातम्_FPClasses() & fcNegSubnormal)
      ज्ञातम्.समुच्चयः_ज्ञातम्_FPClasses(ज्ञातम्.get_ज्ञातम्_FPClasses() | fcNegZero);
    प्रत्यागम ज्ञातम्;
  पक्ष Denormal_बहुलकम्::धनात्मकम्_Zero:
    यदि (ज्ञातम्_Src.get_ज्ञातम्_FPClasses() & fcSubnormal) {
      ज्ञातम्.समुच्चयः_ज्ञातम्_FPClasses(ज्ञातम्.get_ज्ञातम्_FPClasses() | fcPosZero);
      यदि (ज्ञातम्_Src.get_ज्ञातम्_FPClasses() & fcNegSubnormal)
        ज्ञातम्.समुच्चयः_Sign_Bit(std::nullopt);
    }
    प्रत्यागम ज्ञातम्;
  स्वभाव:
    यदि (ज्ञातम्_Src.get_ज्ञातम्_FPClasses() & fcSubnormal) {
      ज्ञातम्.समुच्चयः_ज्ञातम्_FPClasses(ज्ञातम्.get_ज्ञातम्_FPClasses() | fcPosZero);
      यदि (ज्ञातम्_Src.get_ज्ञातम्_FPClasses() & fcNegSubnormal)
        ज्ञातम्.समुच्चयः_Sign_Bit(std::nullopt);
    }
    यदि (ज्ञातम्_Src.get_ज्ञातम्_FPClasses() & fcNegSubnormal)
      ज्ञातम्.समुच्चयः_ज्ञातम्_FPClasses(ज्ञातम्.get_ज्ञातम्_FPClasses() | fcNegZero);
    प्रत्यागम ज्ञातम्;
  }

  llvm_unreachable("covered switch over denormal mode");
}

तार्किक ज्ञातम्_FPClass::is_ज्ञातम्_Never_Logical_Zero(Denormal_बहुलकम् बहुलकम्) ध्रुव {
  प्रत्यागम apply_निवेशः_Denormal_बहुलकम्(*एतत्, बहुलकम्).is_ज्ञातम्_Never_Zero();
}

तार्किक ज्ञातम्_FPClass::is_ज्ञातम्_Never_Logical_Neg_Zero(Denormal_बहुलकम् बहुलकम्) ध्रुव {
  प्रत्यागम apply_निवेशः_Denormal_बहुलकम्(*एतत्, बहुलकम्).is_ज्ञातम्_Never_Neg_Zero();
}

तार्किक ज्ञातम्_FPClass::is_ज्ञातम्_Never_Logical_Pos_Zero(Denormal_बहुलकम् बहुलकम्) ध्रुव {
  प्रत्यागम apply_निवेशः_Denormal_बहुलकम्(*एतत्, बहुलकम्).is_ज्ञातम्_Never_Pos_Zero();
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::minMaxLike(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_LHS,
                                      ध्रुव ज्ञातम्_FPClass &ज्ञातम्_RHS,
                                      Min_Max_विधा विधा_2, Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम्_LHS_2 = apply_निवेशः_Denormal_बहुलकम्(ज्ञातम्_LHS, बहुलकम्);
  ज्ञातम्_FPClass ज्ञातम्_RHS_2 = apply_निवेशः_Denormal_बहुलकम्(ज्ञातम्_RHS, बहुलकम्);

  ज्ञातम्_FPClass ज्ञातम्;

  ज्ञातम्.propagateNonNaN(ज्ञातम्_LHS_2, ज्ञातम्_RHS_2);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  यदि ((ज्ञातम्_LHS_2.is_ज्ञातम्_Never_Na_N() || ज्ञातम्_RHS_2.is_ज्ञातम्_Never_Na_N()) &&
      (विधा_2 == Min_Max_विधा::minnum || विधा_2 == Min_Max_विधा::maxnum ||
       विधा_2 == Min_Max_विधा::minimumnum || विधा_2 == Min_Max_विधा::maximumnum))
    ज्ञातम्.ज्ञातम्_Not(fcNan);

  विकल्प (विधा_2) {
  पक्ष Min_Max_विधा::maxnum: {
    यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never_Na_N())
      ज्ञातम्.ज्ञातम्_Not(orderedStrictlyLess(ज्ञातम्_LHS_2.get_ज्ञातम्_FPClasses(),
                                         /*OrderedZeroSign=*/असत्य));
    यदि (ज्ञातम्_RHS_2.is_ज्ञातम्_Never_Na_N())
      ज्ञातम्.ज्ञातम्_Not(orderedStrictlyLess(ज्ञातम्_RHS_2.get_ज्ञातम्_FPClasses(),
                                         /*OrderedZeroSign=*/असत्य));
    भङ्ग;
  }
  पक्ष Min_Max_विधा::minnum: {
    यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never_Na_N())
      ज्ञातम्.ज्ञातम्_Not(orderedStrictlyGreater(ज्ञातम्_LHS_2.get_ज्ञातम्_FPClasses(),
                                            /*OrderedZeroSign=*/असत्य));
    यदि (ज्ञातम्_RHS_2.is_ज्ञातम्_Never_Na_N())
      ज्ञातम्.ज्ञातम्_Not(orderedStrictlyGreater(ज्ञातम्_RHS_2.get_ज्ञातम्_FPClasses(),
                                            /*OrderedZeroSign=*/असत्य));
    भङ्ग;
  }
  पक्ष Min_Max_विधा::maximumnum: {
    यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never_Na_N())
      ज्ञातम्.ज्ञातम्_Not(orderedStrictlyLess(ज्ञातम्_LHS_2.get_ज्ञातम्_FPClasses(),
                                         /*OrderedZeroSign=*/सत्य));
    यदि (ज्ञातम्_RHS_2.is_ज्ञातम्_Never_Na_N())
      ज्ञातम्.ज्ञातम्_Not(orderedStrictlyLess(ज्ञातम्_RHS_2.get_ज्ञातम्_FPClasses(),
                                         /*OrderedZeroSign=*/सत्य));
    भङ्ग;
  }
  पक्ष Min_Max_विधा::minimumnum: {
    यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never_Na_N())
      ज्ञातम्.ज्ञातम्_Not(orderedStrictlyGreater(ज्ञातम्_LHS_2.get_ज्ञातम्_FPClasses(),
                                            /*OrderedZeroSign=*/सत्य));
    यदि (ज्ञातम्_RHS_2.is_ज्ञातम्_Never_Na_N())
      ज्ञातम्.ज्ञातम्_Not(orderedStrictlyGreater(ज्ञातम्_RHS_2.get_ज्ञातम्_FPClasses(),
                                            /*OrderedZeroSign=*/सत्य));
    भङ्ग;
  }
  पक्ष Min_Max_विधा::अधिकतमम्: {
    ज्ञातम्.ज्ञातम्_Not(orderedStrictlyLess(ज्ञातम्_LHS_2.get_ज्ञातम्_FPClasses(),
                                       /*OrderedZeroSign=*/सत्य) |
                   orderedStrictlyLess(ज्ञातम्_RHS_2.get_ज्ञातम्_FPClasses(),
                                       /*OrderedZeroSign=*/सत्य));
    भङ्ग;
  }
  पक्ष Min_Max_विधा::minimum: {
    ज्ञातम्.ज्ञातम्_Not(orderedStrictlyGreater(ज्ञातम्_LHS_2.get_ज्ञातम्_FPClasses(),
                                          /*OrderedZeroSign=*/सत्य) |
                   orderedStrictlyGreater(ज्ञातम्_RHS_2.get_ज्ञातम्_FPClasses(),
                                          /*OrderedZeroSign=*/सत्य));
    भङ्ग;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      ज्ञातम्_LHS_2.get_ज्ञातम्_FPClasses() | ज्ञातम्_RHS_2.get_ज्ञातम्_FPClasses() | fcNan;
  ज्ञातम्.ज्ञातम्_Not(~PossibleClasses);

  प्रत्यागम apply_Output_Denormal_बहुलकम्(ज्ञातम्, बहुलकम्);
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::canonicalize(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src,
                                        Denormal_बहुलकम् Denorm_बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम्;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  ज्ञातम्.समुच्चयः_ज्ञातम्_FPClasses(ज्ञातम्_Src.get_ज्ञातम्_FPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Na_N())
    ज्ञातम्.ज्ञातम्_Not(fcNan);
  अन्यथा
    ज्ञातम्.ज्ञातम्_Not(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  यदि (Denorm_बहुलकम् == Denormal_बहुलकम्::getIEEE()) {
    यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcPosZero))
      ज्ञातम्.ज्ञातम्_Not(fcPosZero);
    यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcNegZero))
      ज्ञातम्.ज्ञातम्_Not(fcNegZero);
    प्रत्यागम ज्ञातम्;
  }

  यदि (Denorm_बहुलकम्.inputsAreZero() || Denorm_बहुलकम्.outputsAreZero())
    ज्ञातम्.ज्ञातम्_Not(fcSubnormal);

  यदि (Denorm_बहुलकम् == Denormal_बहुलकम्::getPreserveSign()) {
    यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcPosZero | fcPosSubnormal))
      ज्ञातम्.ज्ञातम्_Not(fcPosZero);
    यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcNegZero | fcNegSubnormal))
      ज्ञातम्.ज्ञातम्_Not(fcNegZero);
    प्रत्यागम ज्ञातम्;
  }

  यदि (Denorm_बहुलकम्.निवेशः == Denormal_बहुलकम्::धनात्मकम्_Zero ||
      (Denorm_बहुलकम्.Output == Denormal_बहुलकम्::धनात्मकम्_Zero &&
       Denorm_बहुलकम्.निवेशः == Denormal_बहुलकम्::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcNegZero))
      ज्ञातम्.ज्ञातम्_Not(fcNegZero);

    यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcPosZero | fcSubnormal))
      ज्ञातम्.ज्ञातम्_Not(fcPosZero);
  }

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::bitcast(ध्रुव fltSemantics &FltSemantics,
                                   ध्रुव ज्ञातम्_द्वयङ्काः &द्वयङ्काः_2) {
  प्रतिज्ञा(FltSemantics.आकार_अन्तर्_द्वयङ्काः == द्वयङ्काः_2.get_Bit_विस्तारः() &&
         "Bitcast operand has incorrect bit width");
  ज्ञातम्_FPClass ज्ञातम्;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  यदि (द्वयङ्काः_2.hasConflict())
    प्रत्यागम ज्ञातम्;

  // Return unknown for types we have not validated.
  स्वचालित IsSupported = [](ध्रुव fltSemantics &Semantics) {
    विकल्प (APFloat::Semantics_गन्तव्यम्_Enum(Semantics)) {
    पक्ष APFloat_आधारः::S_IEEEhalf:
    पक्ष APFloat_आधारः::S_BFloat:
    पक्ष APFloat_आधारः::S_IEEEsingle:
    पक्ष APFloat_आधारः::S_IEEEdouble:
    पक्ष APFloat_आधारः::S_IEEEquad:
    पक्ष APFloat_आधारः::S_x87DoubleExtended:
      प्रत्यागम सत्य;
    स्वभाव:
      प्रत्यागम असत्य;
    }
  };
  यदि (!IsSupported(FltSemantics))
    प्रत्यागम ज्ञातम्;

  // Transfer information from the sign bit.
  यदि (द्वयङ्काः_2.isNonNegative())
    ज्ञातम्.signBitMustBeZero();
  अन्यथा यदि (द्वयङ्काः_2.isNegative())
    ज्ञातम्.signBitMustBeOne();

  विकल्प (APFloat_आधारः::Semantics_गन्तव्यम्_Enum(FltSemantics)) {
  पक्ष APFloat_आधारः::S_IEEEhalf:
  पक्ष APFloat_आधारः::S_BFloat:
  पक्ष APFloat_आधारः::S_IEEEsingle:
  पक्ष APFloat_आधारः::S_IEEEdouble:
  पक्ष APFloat_आधारः::S_IEEEquad: {
    ध्रुव अचिह्नित Mantissa_द्वयङ्काः = FltSemantics.precision - 1;
    ध्रुव APInt Exponent_आवरणम् = APInt::get_द्वयङ्काः_समुच्चयः(
        FltSemantics.आकार_अन्तर्_द्वयङ्काः, Mantissa_द्वयङ्काः, FltSemantics.आकार_अन्तर्_द्वयङ्काः - 1);
    ध्रुव APInt Mantissa_आवरणम् =
        APInt::get_नीचः_द्वयङ्काः_समुच्चयः(FltSemantics.आकार_अन्तर्_द्वयङ्काः, Mantissa_द्वयङ्काः);

    ध्रुव तार्किक Exponent_ज्ञातम्_All_Zeros =
        (द्वयङ्काः_2.Zero & Exponent_आवरणम्) == Exponent_आवरणम्;
    ध्रुव तार्किक Exponent_ज्ञातम्_All_Ones = (द्वयङ्काः_2.One & Exponent_आवरणम्) == Exponent_आवरणम्;
    ध्रुव तार्किक Exponent_ज्ञातम्_Not_All_Zeros = !(द्वयङ्काः_2.One & Exponent_आवरणम्).isZero();
    ध्रुव तार्किक Exponent_ज्ञातम्_Not_All_Ones = !(द्वयङ्काः_2.Zero & Exponent_आवरणम्).isZero();

    ध्रुव तार्किक Mantissa_ज्ञातम्_All_Zeros =
        (द्वयङ्काः_2.Zero & Mantissa_आवरणम्) == Mantissa_आवरणम्;
    ध्रुव तार्किक Mantissa_ज्ञातम्_Not_All_Zeros = !(द्वयङ्काः_2.One & Mantissa_आवरणम्).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    यदि (Exponent_ज्ञातम्_Not_All_Zeros)
      ज्ञातम्.ज्ञातम्_Not(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    यदि (Exponent_ज्ञातम्_Not_All_Ones)
      ज्ञातम्.ज्ञातम्_Not(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    यदि (Exponent_ज्ञातम्_All_Zeros || Exponent_ज्ञातम्_All_Ones)
      ज्ञातम्.ज्ञातम्_Not(fc_प्रसामान्यम्);

    // Zero and infinity require a mantissa with all zero bits.
    यदि (Mantissa_ज्ञातम्_Not_All_Zeros)
      ज्ञातम्.ज्ञातम्_Not(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    यदि (Mantissa_ज्ञातम्_All_Zeros)
      ज्ञातम्.ज्ञातम्_Not(fcSubnormal | fcNan);

    ध्रुव तार्किक Quiet_Bit_ज्ञातम्_समुच्चयः = द्वयङ्काः_2.One[Mantissa_द्वयङ्काः - 1];
    ध्रुव तार्किक Quiet_Bit_ज्ञातम्_Clear = द्वयङ्काः_2.Zero[Mantissa_द्वयङ्काः - 1];

    यदि (Quiet_Bit_ज्ञातम्_समुच्चयः)
      ज्ञातम्.ज्ञातम्_Not(fcSNan);
    अन्यथा यदि (Quiet_Bit_ज्ञातम्_Clear)
      ज्ञातम्.ज्ञातम्_Not(fcQNan);

    भङ्ग;
  }
  पक्ष APFloat_आधारः::S_x87DoubleExtended: {
    // Layout of the x87 floating point type:
    // [0:62] fraction
    // [63] explicit integer bit
    // [64:78] exponent
    // [79] sign bit
    // APFloat treats the x87 pseudo encodings as documented in
    // fltSemantics::hasExplicitIntegerBit.
    constexpr अचिह्नित Bit_विस्तारः = 80;
    constexpr अचिह्नित Fraction_द्वयङ्काः = 63;
    constexpr अचिह्नित Integer_Bit_सूचकांक = Fraction_द्वयङ्काः;
    ध्रुव APInt Fraction_आवरणम् = APInt::get_नीचः_द्वयङ्काः_समुच्चयः(Bit_विस्तारः, Fraction_द्वयङ्काः);
    ध्रुव APInt Exponent_आवरणम् =
        APInt::get_द्वयङ्काः_समुच्चयः(Bit_विस्तारः, Fraction_द्वयङ्काः + 1, Bit_विस्तारः - 1);

    ध्रुव तार्किक Exponent_ज्ञातम्_All_Zeros =
        (द्वयङ्काः_2.Zero & Exponent_आवरणम्) == Exponent_आवरणम्;
    ध्रुव तार्किक Exponent_ज्ञातम्_All_Ones = (द्वयङ्काः_2.One & Exponent_आवरणम्) == Exponent_आवरणम्;
    ध्रुव तार्किक Exponent_ज्ञातम्_Not_All_Zeros = !(द्वयङ्काः_2.One & Exponent_आवरणम्).isZero();
    ध्रुव तार्किक Exponent_ज्ञातम्_Not_All_Ones = !(द्वयङ्काः_2.Zero & Exponent_आवरणम्).isZero();
    ध्रुव तार्किक Fraction_ज्ञातम्_All_Zeros =
        (द्वयङ्काः_2.Zero & Fraction_आवरणम्) == Fraction_आवरणम्;
    ध्रुव तार्किक Fraction_ज्ञातम्_Not_All_Zeros = !(द्वयङ्काः_2.One & Fraction_आवरणम्).isZero();
    ध्रुव तार्किक Integer_Bit_ज्ञातम्_समुच्चयः = द्वयङ्काः_2.One[Integer_Bit_सूचकांक];
    ध्रुव तार्किक Integer_Bit_ज्ञातम्_Clear = द्वयङ्काः_2.Zero[Integer_Bit_सूचकांक];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    यदि (Exponent_ज्ञातम्_Not_All_Zeros || Integer_Bit_ज्ञातम्_समुच्चयः)
      ज्ञातम्.ज्ञातम्_Not(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    यदि (Fraction_ज्ञातम्_Not_All_Zeros)
      ज्ञातम्.ज्ञातम्_Not(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    यदि (Exponent_ज्ञातम्_Not_All_Ones || Integer_Bit_ज्ञातम्_Clear ||
        Fraction_ज्ञातम्_Not_All_Zeros)
      ज्ञातम्.ज्ञातम्_Not(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    यदि (Integer_Bit_ज्ञातम्_Clear || Exponent_ज्ञातम्_All_Ones)
      ज्ञातम्.ज्ञातम्_Not(fc_प्रसामान्यम्);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    यदि (Fraction_ज्ञातम्_All_Zeros)
      ज्ञातम्.ज्ञातम्_Not(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    यदि (Exponent_ज्ञातम्_All_Zeros ||
        (Integer_Bit_ज्ञातम्_समुच्चयः &&
         (Exponent_ज्ञातम्_Not_All_Ones || Fraction_ज्ञातम्_All_Zeros)))
      ज्ञातम्.ज्ञातम्_Not(fcNan);

    ध्रुव तार्किक Quiet_Bit_ज्ञातम्_समुच्चयः = द्वयङ्काः_2.One[Fraction_द्वयङ्काः - 1];
    ध्रुव तार्किक Quiet_Bit_ज्ञातम्_Clear = द्वयङ्काः_2.Zero[Fraction_द्वयङ्काः - 1];

    यदि (Quiet_Bit_ज्ञातम्_समुच्चयः)
      ज्ञातम्.ज्ञातम्_Not(fcSNan);
    अन्यथा यदि (Quiet_Bit_ज्ञातम्_Clear)
      ज्ञातम्.ज्ञातम्_Not(fcQNan);

    भङ्ग;
  }
  स्वभाव:
    llvm_unreachable("unhandled supported type");
  }

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_द्वयङ्काः ज्ञातम्_FPClass::गन्तव्यम्_ज्ञातम्_द्वयङ्काः(ध्रुव fltSemantics &FltSemantics) ध्रुव {
  ज्ञातम्_द्वयङ्काः ज्ञातम्(FltSemantics.आकार_अन्तर्_द्वयङ्काः);
  ध्रुव FPClassTest FPClasses = get_ज्ञातम्_FPClasses();

  // Return unknown if poison.
  यदि (FPClasses == fcNone)
    प्रत्यागम ज्ञातम्;

  // Return unknown for types we have not validated.
  स्वचालित IsSupported = [](ध्रुव fltSemantics &Semantics) {
    विकल्प (APFloat::Semantics_गन्तव्यम्_Enum(Semantics)) {
    पक्ष APFloat_आधारः::S_IEEEhalf:
    पक्ष APFloat_आधारः::S_BFloat:
    पक्ष APFloat_आधारः::S_IEEEsingle:
    पक्ष APFloat_आधारः::S_IEEEdouble:
    पक्ष APFloat_आधारः::S_IEEEquad:
    पक्ष APFloat_आधारः::S_x87DoubleExtended:
      प्रत्यागम सत्य;
    स्वभाव:
      प्रत्यागम असत्य;
    }
  };
  यदि (!IsSupported(FltSemantics))
    प्रत्यागम ज्ञातम्;

  विकल्प (APFloat::Semantics_गन्तव्यम्_Enum(FltSemantics)) {
  पक्ष APFloat_आधारः::S_IEEEhalf:
  पक्ष APFloat_आधारः::S_BFloat:
  पक्ष APFloat_आधारः::S_IEEEsingle:
  पक्ष APFloat_आधारः::S_IEEEdouble:
  पक्ष APFloat_आधारः::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    यदि (FPClasses & fc_प्रसामान्यम्)
      भङ्ग;

    ज्ञातम्.समुच्चयः_All_Conflict();

    ध्रुव अचिह्नित Bit_विस्तारः = FltSemantics.आकार_अन्तर्_द्वयङ्काः;
    ध्रुव अचिह्नित Mantissa_द्वयङ्काः = FltSemantics.precision - 1;
    ध्रुव अचिह्नित Exponent_द्वयङ्काः = Bit_विस्तारः - Mantissa_द्वयङ्काः - 1;

    APInt Mantissa_आवरणम् = APInt::get_नीचः_द्वयङ्काः_समुच्चयः(Bit_विस्तारः, Mantissa_द्वयङ्काः);
    APInt Exponent_आवरणम् =
        APInt::get_द्वयङ्काः_समुच्चयः(Bit_विस्तारः, Mantissa_द्वयङ्काः, Mantissa_द्वयङ्काः + Exponent_द्वयङ्काः);

    ध्रुव अचिह्नित Quiet_Bit_सूचकांक = Mantissa_द्वयङ्काः - 1;
    APInt Payload_आवरणम् = Mantissa_आवरणम्;
    Payload_आवरणम्.clearBit(Quiet_Bit_सूचकांक);

    यदि (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      ज्ञातम्.Zero &= ~Exponent_आवरणम्;
      // No individual payload bit is known.
      ज्ञातम्.Zero &= ~Payload_आवरणम्;
      ज्ञातम्.One &= ~Payload_आवरणम्;

      यदि (FPClasses & fcQNan)
        ज्ञातम्.Zero.clearBit(Quiet_Bit_सूचकांक);
      यदि (FPClasses & fcSNan)
        ज्ञातम्.One.clearBit(Quiet_Bit_सूचकांक);
    }
    यदि (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      ज्ञातम्.Zero &= ~Exponent_आवरणम्;
      // Mantissa bits cannot be ones.
      ज्ञातम्.One &= ~Mantissa_आवरणम्;
    }
    यदि (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      ज्ञातम्.One &= ~Exponent_आवरणम्;
      // Unknown mantissa.
      ज्ञातम्.One &= ~Mantissa_आवरणम्;
      ज्ञातम्.Zero &= ~Mantissa_आवरणम्;
    }
    यदि (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      ज्ञातम्.One &= ~Exponent_आवरणम्;
      // Mantissa cannot be ones.
      ज्ञातम्.One &= ~Mantissa_आवरणम्;
    }

    भङ्ग;
  }
  पक्ष APFloat_आधारः::S_x87DoubleExtended: {
    यदि (!is_ज्ञातम्_Never(fc_प्रसामान्यम् | fcSubnormal | fcNan))
      भङ्ग;

    ज्ञातम्.समुच्चयः_All_Conflict();

    यदि (FPClasses & fcInf)
      ज्ञातम् = ज्ञातम्.intersectWith(ज्ञातम्_द्वयङ्काः::makeConstant(
          APFloat::getInf(FltSemantics).bitcast_गन्तव्यम्_APInt()));

    यदि (FPClasses & fcZero)
      ज्ञातम् = ज्ञातम्.intersectWith(
          ज्ञातम्_द्वयङ्काः::makeConstant(APInt::getZero(FltSemantics.आकार_अन्तर्_द्वयङ्काः)));

    भङ्ग;
  }
  स्वभाव:
    llvm_unreachable("unhandled supported semantics");
  }

  ज्ञातम्.Zero.clearSignBit();
  ज्ञातम्.One.clearSignBit();
  यदि (std::optional<तार्किक> Sign = getSignBit()) {
    यदि (*Sign)
      ज्ञातम्.makeNegative();
    अन्यथा
      ज्ञातम्.makeNonNegative();
  }

  प्रत्यागम ज्ञातम्;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
स्थिर ज्ञातम्_FPClass fadd_impl(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_LHS_2,
                              ध्रुव ज्ञातम्_FPClass &ज्ञातम्_RHS_2, तार्किक IsFMA) {
  ज्ञातम्_FPClass ज्ञातम्;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never_Na_N() && ज्ञातम्_RHS_2.is_ज्ञातम्_Never_Na_N() &&
      (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcPosInf) || ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fcNegInf)) &&
      (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcNegInf) || ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fcPosInf)))
    ज्ञातम्.ज्ञातम्_Not(fcNan);

  यदि (ज्ञातम्_LHS_2.cannotBeOrderedLessThanZero() &&
      ज्ञातम्_RHS_2.cannotBeOrderedLessThanZero()) {
    ज्ञातम्.ज्ञातम्_Not(ज्ञातम्_FPClass::Ordered_Less_Than_Zero_आवरणम्);

    // This can't underflow if one of the operands is known normal.
    यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcZero | fcPosSubnormal) ||
        ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fcZero | fcPosSubnormal))
      ज्ञातम्.ज्ञातम्_Not(fcZero | fcPosSubnormal);
  }

  यदि (ज्ञातम्_LHS_2.cannotBeOrderedGreaterThanZero() &&
      ज्ञातम्_RHS_2.cannotBeOrderedGreaterThanZero()) {
    ज्ञातम्.ज्ञातम्_Not(ज्ञातम्_FPClass::Ordered_Greater_Than_Zero_आवरणम्);

    // This can't underflow if one of the operands is known normal.
    यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcZero | fcNegSubnormal) ||
        ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fcZero | fcNegSubnormal))
      ज्ञातम्.ज्ञातम्_Not(fcZero | fcNegSubnormal);
  }

  यदि (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcNegZero) || ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fcNegZero))
      ज्ञातम्.ज्ञातम्_Not(fcNegZero);

    ज्ञातम्.propagateNonSNaN(ज्ञातम्_LHS_2, ज्ञातम्_RHS_2);
  }

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::fadd(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_LHS,
                                ध्रुव ज्ञातम्_FPClass &ज्ञातम्_RHS,
                                Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम्_LHS_2 = apply_निवेशः_Denormal_बहुलकम्(ज्ञातम्_LHS, बहुलकम्);
  ज्ञातम्_FPClass ज्ञातम्_RHS_2 = apply_निवेशः_Denormal_बहुलकम्(ज्ञातम्_RHS, बहुलकम्);

  ज्ञातम्_FPClass ज्ञातम् = fadd_impl(ज्ञातम्_LHS_2, ज्ञातम्_RHS_2, /*IsFMA=*/असत्य);

  प्रत्यागम apply_Output_Denormal_बहुलकम्(ज्ञातम्, बहुलकम्);
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::fadd_self(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src_2,
                                     Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम्_Src = apply_निवेशः_Denormal_बहुलकम्(ज्ञातम्_Src_2, बहुलकम्);

  ज्ञातम्_FPClass ज्ञातम् = fadd_impl(ज्ञातम्_Src, ज्ञातम्_Src, /*IsFMA=*/असत्य);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcPosSubnormal))
    ज्ञातम्.ज्ञातम्_Not(fcPosSubnormal);
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcNegSubnormal))
    ज्ञातम्.ज्ञातम्_Not(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcPosZero))
    ज्ञातम्.ज्ञातम्_Not(fcPosZero);

  प्रत्यागम apply_Output_Denormal_बहुलकम्(ज्ञातम्, बहुलकम्);
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::fsub(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_LHS,
                                ध्रुव ज्ञातम्_FPClass &ज्ञातम्_RHS,
                                Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम्_LHS_2 = apply_निवेशः_Denormal_बहुलकम्(ज्ञातम्_LHS, बहुलकम्);
  ज्ञातम्_FPClass ज्ञातम्_RHS_2 = apply_निवेशः_Denormal_बहुलकम्(ज्ञातम्_RHS, बहुलकम्);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  ज्ञातम्_RHS_2.fneg();

  ज्ञातम्_FPClass ज्ञातम् = fadd_impl(ज्ञातम्_LHS_2, ज्ञातम्_RHS_2, /*IsFMA=*/असत्य);

  प्रत्यागम apply_Output_Denormal_बहुलकम्(ज्ञातम्, बहुलकम्);
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::fmul(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_LHS,
                                ध्रुव ज्ञातम्_FPClass &ज्ञातम्_RHS,
                                Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम्_LHS_2 = apply_निवेशः_Denormal_बहुलकम्(ज्ञातम्_LHS, बहुलकम्);
  ज्ञातम्_FPClass ज्ञातम्_RHS_2 = apply_निवेशः_Denormal_बहुलकम्(ज्ञातम्_RHS, बहुलकम्);

  ज्ञातम्_FPClass ज्ञातम्;

  ज्ञातम्.propagateNonSNaN(ज्ञातम्_LHS_2, ज्ञातम्_RHS_2);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  यदि ((ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcNegative) &&
       ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fcNegative)) ||
      (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fc_धनात्मकम्) && ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fc_धनात्मकम्)))
    ज्ञातम्.ज्ञातम्_Not(fcNegative);
  यदि ((ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fc_धनात्मकम्) &&
       ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fcNegative)) ||
      (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcNegative) && ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fc_धनात्मकम्)))
    ज्ञातम्.ज्ञातम्_Not(fc_धनात्मकम्);

  // Inf * Y => Inf or NaN
  यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Always(fcInf | fcNan) ||
      ज्ञातम्_RHS_2.is_ज्ञातम्_Always(fcInf | fcNan))
    ज्ञातम्.ज्ञातम्_Not(fc_प्रसामान्यम् | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  यदि (ज्ञातम्_RHS_2.is_ज्ञातम्_Always(fcZero | fcNan) ||
      ज्ञातम्_LHS_2.is_ज्ञातम्_Always(fcZero | fcNan))
    ज्ञातम्.ज्ञातम्_Not(fc_प्रसामान्यम् | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never_Na_N() && ज्ञातम्_RHS_2.is_ज्ञातम्_Never_Na_N() &&
      (ज्ञातम्_RHS_2.is_ज्ञातम्_Never_Infinity() || ज्ञातम्_LHS_2.is_ज्ञातम्_Never_Zero()) &&
      (ज्ञातम्_LHS_2.is_ज्ञातम्_Never_Infinity() || ज्ञातम्_RHS_2.is_ज्ञातम्_Never_Zero()))
    ज्ञातम्.ज्ञातम्_Not(fcNan);

  प्रत्यागम apply_Output_Denormal_बहुलकम्(ज्ञातम्, बहुलकम्);
}

// TODO: This generalizes to known ranges
ज्ञातम्_FPClass ज्ञातम्_FPClass::fmul(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_LHS_2,
                                ध्रुव APFloat &CRHS, Denormal_बहुलकम् बहुलकम्) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  ध्रुव fltSemantics &Flt = CRHS.getSemantics();
  अचिह्नित Precision = APFloat::semanticsPrecision(Flt);
  ध्रुव पूर्णाङ्क Mantissa_द्वयङ्काः = Precision - 1;

  पूर्णाङ्क Min_ज्ञातम्_Exponent = ilogb(CRHS);
  तार्किक CannotBeSubnormal = (Min_ज्ञातम्_Exponent >= Mantissa_द्वयङ्काः);

  ज्ञातम्_FPClass ज्ञातम् = ज्ञातम्_FPClass::fmul(ज्ञातम्_LHS_2, ज्ञातम्_FPClass(CRHS), बहुलकम्);
  यदि (CannotBeSubnormal)
    ज्ञातम्.ज्ञातम्_Not(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcInf)) {
    यदि (Min_ज्ञातम्_Exponent < 0)
      ज्ञातम्.ज्ञातम्_Not(fcInf);
    अन्यथा यदि (Min_ज्ञातम्_Exponent == 0 && CRHS.compare_Absolute_मानम्(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      ज्ञातम्.ज्ञातम्_Not(fcInf);
  }

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::fdiv(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_LHS,
                                ध्रुव ज्ञातम्_FPClass &ज्ञातम्_RHS,
                                Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम्_LHS_2 = apply_निवेशः_Denormal_बहुलकम्(ज्ञातम्_LHS, बहुलकम्);
  ज्ञातम्_FPClass ज्ञातम्_RHS_2 = apply_निवेशः_Denormal_बहुलकम्(ज्ञातम्_RHS, बहुलकम्);

  ज्ञातम्_FPClass ज्ञातम्;

  ज्ञातम्.propagateNonSNaN(ज्ञातम्_LHS_2, ज्ञातम्_RHS_2);

  // Only 0/0, Inf/Inf produce NaN.
  यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never_Na_N() && ज्ञातम्_RHS_2.is_ज्ञातम्_Never_Na_N() &&
      (ज्ञातम्_LHS_2.is_ज्ञातम्_Never_Infinity() || ज्ञातम्_RHS_2.is_ज्ञातम्_Never_Infinity()) &&
      (ज्ञातम्_LHS_2.is_ज्ञातम्_Never_Zero() || ज्ञातम्_RHS_2.is_ज्ञातम्_Never_Zero())) {
    ज्ञातम्.ज्ञातम्_Not(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  यदि ((ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcNegative) &&
       ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fcNegative)) ||
      (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fc_धनात्मकम्) && ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fc_धनात्मकम्)))
    ज्ञातम्.ज्ञातम्_Not(fcNegative);
  यदि ((ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fc_धनात्मकम्) &&
       ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fcNegative)) ||
      (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcNegative) && ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fc_धनात्मकम्)))
    ज्ञातम्.ज्ञातम्_Not(fc_धनात्मकम्);

  // Normal and subnormal results require two non-zero finite operands.
  यदि ((ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fc_Neg_प्रसामान्यम् | fcNegSubnormal) &&
       ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fc_Neg_प्रसामान्यम् | fcNegSubnormal)) ||
      (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fc_Pos_प्रसामान्यम् | fcPosSubnormal) &&
       ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fc_Pos_प्रसामान्यम् | fcPosSubnormal)))
    ज्ञातम्.ज्ञातम्_Not(fc_Neg_प्रसामान्यम् | fcNegSubnormal);
  यदि ((ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fc_Neg_प्रसामान्यम् | fcNegSubnormal) &&
       ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fc_Pos_प्रसामान्यम् | fcPosSubnormal)) ||
      (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fc_Pos_प्रसामान्यम् | fcPosSubnormal) &&
       ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fc_Neg_प्रसामान्यम् | fcNegSubnormal)))
    ज्ञातम्.ज्ञातम्_Not(fc_Pos_प्रसामान्यम् | fcPosSubnormal);

  // 0 / X => 0 or NaN
  यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Always(fcZero))
    ज्ञातम्.ज्ञातम्_Not(fcSubnormal | fc_प्रसामान्यम् | fcInf);

  // X / 0 => NaN or Inf
  यदि (ज्ञातम्_RHS_2.is_ज्ञातम्_Always(fcZero))
    ज्ञातम्.ज्ञातम्_Not(fcFinite);

  प्रत्यागम apply_Output_Denormal_बहुलकम्(ज्ञातम्, बहुलकम्);
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::fdiv_self(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src,
                                     Denormal_बहुलकम् बहुलकम्) {
  // X / X is always exactly 1.0 or a NaN.
  ज्ञातम्_FPClass ज्ञातम्(fcNan | fc_Pos_प्रसामान्यम्);

  ज्ञातम्.propagateNonSNaN(ज्ञातम्_Src);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Inf_Or_Na_N() && ज्ञातम्_Src.is_ज्ञातम्_Never_Logical_Zero(बहुलकम्))
    ज्ञातम्.ज्ञातम्_Not(fcNan);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::frem(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_LHS_2,
                                ध्रुव ज्ञातम्_FPClass &ज्ञातम्_RHS_2,
                                Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम्;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  ज्ञातम्.ज्ञातम्_Not(fcInf);

  ज्ञातम्.propagateNonSNaN(ज्ञातम्_LHS_2, ज्ञातम्_RHS_2);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never_Inf_Or_Na_N() && ज्ञातम्_RHS_2.is_ज्ञातम्_Never_Na_N() &&
      ज्ञातम्_RHS_2.is_ज्ञातम्_Never_Logical_Zero(बहुलकम्)) {
    ज्ञातम्.ज्ञातम्_Not(fcNan);
  }

  // The sign for frem is the same as the first operand.
  यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fc_Pos_प्रसामान्यम् | fcPosSubnormal))
    ज्ञातम्.ज्ञातम्_Not(fc_Pos_प्रसामान्यम् | fcPosSubnormal);
  यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fc_Neg_प्रसामान्यम् | fcNegSubnormal))
    ज्ञातम्.ज्ञातम्_Not(fc_Neg_प्रसामान्यम् | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcNegFinite))
    ज्ञातम्.ज्ञातम्_Not(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcPosFinite) &&
      ज्ञातम्_LHS_2.is_ज्ञातम्_Never_Logical_Pos_Zero(बहुलकम्) &&
      (!बहुलकम्.outputs_May_Be_धनात्मकम्_Zero() ||
       ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fc_Neg_प्रसामान्यम् | fcNegSubnormal)))
    ज्ञातम्.ज्ञातम्_Not(fcPosZero);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::frem_self(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src,
                                     Denormal_बहुलकम् बहुलकम्) {
  // X % X is always exactly [+-]0.0 or a NaN.
  ज्ञातम्_FPClass ज्ञातम्(fcNan | fcZero);

  ज्ञातम्.propagateNonSNaN(ज्ञातम्_Src);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Inf_Or_Na_N() && ज्ञातम्_Src.is_ज्ञातम्_Never_Logical_Zero(बहुलकम्))
    ज्ञातम्.ज्ञातम्_Not(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fc_Pos_प्रसामान्यम् | fcPosSubnormal))
    ज्ञातम्.ज्ञातम्_Not(fcPosZero);
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fc_Neg_प्रसामान्यम् | fcNegSubnormal))
    ज्ञातम्.ज्ञातम्_Not(fcNegZero);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::fma(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_LHS_2,
                               ध्रुव ज्ञातम्_FPClass &ज्ञातम्_RHS_2,
                               ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Addend,
                               Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass Mul = fmul(ज्ञातम्_LHS_2, ज्ञातम्_RHS_2, बहुलकम्);

  // FMA differs from the base fmul + fadd handling only in the treatment of -0
  // results.
  //
  // If the multiply is a -0 due to rounding, the final -0 + 0 will be -0,
  // unlike for a separate fadd.
  ज्ञातम्_FPClass ज्ञातम् = fadd_impl(Mul, ज्ञातम्_Addend, /*IsFMA=*/सत्य);

  // propagateNonSNaN for 3 arguments.
  यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcSNan) && ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fcSNan) &&
      ज्ञातम्_Addend.is_ज्ञातम्_Never(fcSNan))
    ज्ञातम्.ज्ञातम्_Not(fcSNan);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::fmuladd(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_LHS_2,
                                   ध्रुव ज्ञातम्_FPClass &ज्ञातम्_RHS_2,
                                   ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Addend,
                                   Denormal_बहुलकम् बहुलकम्) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  ज्ञातम्_FPClass FMA = fma(ज्ञातम्_LHS_2, ज्ञातम्_RHS_2, ज्ञातम्_Addend, बहुलकम्);
  ज्ञातम्_FPClass Mul_सङ्कलय = fadd(fmul(ज्ञातम्_LHS_2, ज्ञातम्_RHS_2, बहुलकम्), ज्ञातम्_Addend, बहुलकम्);
  प्रत्यागम FMA.intersectWith(Mul_सङ्कलय);
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::fma_वर्गफलम्(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Squared,
                                      ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Addend,
                                      Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass Squared = वर्गफलम्(ज्ञातम्_Squared, बहुलकम्);
  ज्ञातम्_FPClass ज्ञातम् = fadd_impl(Squared, ज्ञातम्_Addend, /*IsFMA=*/सत्य);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  यदि (ज्ञातम्_Addend.is_ज्ञातम्_Never(fcNegInf | fcNan) && Squared.is_ज्ञातम्_Never(fcNan))
    ज्ञातम्.ज्ञातम्_Not(fcNan);

  ज्ञातम्.propagateNonSNaN(ज्ञातम्_Squared, ज्ञातम्_Addend);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::fmuladd_वर्गफलम्(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Squared,
                                          ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Addend,
                                          Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass FMA = fma_वर्गफलम्(ज्ञातम्_Squared, ज्ञातम्_Addend, बहुलकम्);
  ज्ञातम्_FPClass Mul_सङ्कलय = fadd(वर्गफलम्(ज्ञातम्_Squared, बहुलकम्), ज्ञातम्_Addend, बहुलकम्);
  प्रत्यागम FMA.intersectWith(Mul_सङ्कलय);
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::घातांकः(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src) {
  ज्ञातम्_FPClass ज्ञातम्;
  ज्ञातम्.ज्ञातम्_Not(fcNegative);

  ज्ञातम्.propagateNonNaN(ज्ञातम्_Src);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fc_Neg_प्रसामान्यम् | fcNegInf))
    ज्ञातम्.ज्ञातम्_Not(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fc_Neg_प्रसामान्यम्))
    ज्ञातम्.ज्ञातम्_Not(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fc_Pos_प्रसामान्यम् | fcPosInf))
    ज्ञातम्.ज्ञातम्_Not(fcPosInf);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::लघुगणकः(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src, Denormal_बहुलकम् बहुलकम्,
                               तार्किक IsMultiUnitFPType) {
  ज्ञातम्_FPClass ज्ञातम्;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  यदि (!IsMultiUnitFPType)
    ज्ञातम्.ज्ञातम्_Not(fcNegZero | fcSubnormal);

  ज्ञातम्.propagateNonSNaN(ज्ञातम्_Src);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Pos_Infinity())
    ज्ञातम्.ज्ञातम्_Not(fcPosInf);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Na_N() && ज्ञातम्_Src.cannotBeOrderedLessThanZero())
    ज्ञातम्.ज्ञातम्_Not(fcNan);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Logical_Zero(बहुलकम्))
    ज्ञातम्.ज्ञातम्_Not(fcNegInf);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::वर्गमूलम्(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src,
                                Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम्;
  ज्ञातम्.ज्ञातम्_Not(fcPosSubnormal);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Pos_Infinity())
    ज्ञातम्.ज्ञातम्_Not(fcPosInf);

  ज्ञातम्.propagateNonSNaN(ज्ञातम्_Src);

  // Any negative value besides -0 returns a nan.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Na_N() && ज्ञातम्_Src.cannotBeOrderedLessThanZero())
    ज्ञातम्.ज्ञातम्_Not(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  ज्ञातम्.ज्ञातम्_Not(fcNegInf | fcNegSubnormal | fc_Neg_प्रसामान्यम्);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Logical_Neg_Zero(बहुलकम्))
    ज्ञातम्.ज्ञातम्_Not(fcNegZero);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::ज्या(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src) {
  ज्ञातम्_FPClass ज्ञातम्;

  ज्ञातम्.propagateNonSNaN(ज्ञातम्_Src);

  // Return NaN on infinite inputs.
  ज्ञातम्.ज्ञातम्_Not(fcInf);
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Na_N() && ज्ञातम्_Src.is_ज्ञातम्_Never_Infinity())
    ज्ञातम्.ज्ञातम्_Not(fcNan);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::कोटिज्या(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src) {
  प्रत्यागम ज्या(ज्ञातम्_Src);
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::tan(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src) {
  ज्ञातम्_FPClass ज्ञातम्;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  ज्ञातम्.ज्ञातम्_Not(fcInf);

  ज्ञातम्.propagateNonSNaN(ज्ञातम्_Src);

  // NaN propagates. tan(+-Inf) is NaN.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Na_N() && ज्ञातम्_Src.is_ज्ञातम्_Never_Infinity())
    ज्ञातम्.ज्ञातम्_Not(fcNan);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::sinh(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src) {
  ज्ञातम्_FPClass ज्ञातम्;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcNegative))
    ज्ञातम्.ज्ञातम्_Not(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcPosSubnormal | fc_Pos_प्रसामान्यम् | fcPosInf))
    ज्ञातम्.ज्ञातम्_Not(fcPosSubnormal | fc_Pos_प्रसामान्यम् | fcPosInf);

  ज्ञातम्.propagateNonNaN(ज्ञातम्_Src);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::cosh(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src) {
  ज्ञातम्_FPClass ज्ञातम्;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  ज्ञातम्.ज्ञातम्_Not(fcNegative | fcZero | fcSubnormal);

  ज्ञातम्.propagateNonNaN(ज्ञातम्_Src);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::tanh(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src) {
  ज्ञातम्_FPClass ज्ञातम्;

  // tanh is bounded to (-1, 1), never Inf.
  ज्ञातम्.ज्ञातम्_Not(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcNegSubnormal | fc_Neg_प्रसामान्यम् | fcNegInf)) {
    ज्ञातम्.ज्ञातम्_Not(fcNegSubnormal | fc_Neg_प्रसामान्यम्);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Neg_Zero())
      ज्ञातम्.ज्ञातम्_Not(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcPosSubnormal | fc_Pos_प्रसामान्यम् | fcPosInf))
    ज्ञातम्.ज्ञातम्_Not(fcPosSubnormal | fc_Pos_प्रसामान्यम्);

  ज्ञातम्.propagateNonNaN(ज्ञातम्_Src);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::asin(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src) {
  ज्ञातम्_FPClass ज्ञातम्;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  ज्ञातम्.ज्ञातम्_Not(fcInf);

  ज्ञातम्.propagateNonSNaN(ज्ञातम्_Src);

  // asin is sign-preserving for finite arguments.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcNegFinite))
    ज्ञातम्.ज्ञातम्_Not(fcNegFinite);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcPosSubnormal | fc_Pos_प्रसामान्यम्))
    ज्ञातम्.ज्ञातम्_Not(fcPosSubnormal | fc_Pos_प्रसामान्यम्);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::acos(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src) {
  ज्ञातम्_FPClass ज्ञातम्;

  // acos(x) is bounded to [0, pi] for -1 <= x <= 1, and is never negative,
  // infinite, or subnormal. The smallest non-zero value occurs when x is
  // close to 1.0, where acos(x) can be approximated by sqrt(2 * (1 - x)).
  // Since sqrt cannot produce a subnormal result, we can conclude that
  // acos(x) will also never produce a subnormal result.
  ज्ञातम्.ज्ञातम्_Not(fcNegative | fcInf | fcSubnormal);

  // acos(x) == +0.0 iff x == +1.0
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fc_Pos_प्रसामान्यम्))
    ज्ञातम्.ज्ञातम्_Not(fcZero);

  ज्ञातम्.propagateNonSNaN(ज्ञातम्_Src);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::atan(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src) {
  ज्ञातम्_FPClass ज्ञातम्;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  ज्ञातम्.ज्ञातम्_Not(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcNegSubnormal | fc_Neg_प्रसामान्यम् | fcNegInf))
    ज्ञातम्.ज्ञातम्_Not(fcNegSubnormal | fc_Neg_प्रसामान्यम्);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcPosSubnormal | fc_Pos_प्रसामान्यम् | fcPosInf))
    ज्ञातम्.ज्ञातम्_Not(fcPosSubnormal | fc_Pos_प्रसामान्यम्);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fc_Neg_प्रसामान्यम् | fcNegSubnormal | fcNegZero))
    ज्ञातम्.ज्ञातम्_Not(fcNegZero);

  ज्ञातम्.propagateNonNaN(ज्ञातम्_Src);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::atan2(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Y,
                                 ध्रुव ज्ञातम्_FPClass &ज्ञातम्_X,
                                 Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम्_Y_2 = apply_निवेशः_Denormal_बहुलकम्(ज्ञातम्_Y, बहुलकम्);
  ज्ञातम्_FPClass ज्ञातम्_X_2 = apply_निवेशः_Denormal_बहुलकम्(ज्ञातम्_X, बहुलकम्);
  ज्ञातम्_FPClass ज्ञातम्;

  // Even though these deductions are correct, we are ignoring the following
  // potentially erroneous cases:
  //   * atan2(y, inf) is not subnormal
  //   * atan2(inf, x) is not zero or subnormal

  // atan2 result is in (-pi, pi], never Inf.
  ज्ञातम्.ज्ञातम्_Not(fcInf);

  ज्ञातम्.propagateNonNaN(ज्ञातम्_Y_2, ज्ञातम्_X_2);

  // If x <= -0.0, then |atan2(y, x)| >= pi/2
  यदि (ज्ञातम्_X_2.is_ज्ञातम्_Never(fc_धनात्मकम्))
    ज्ञातम्.ज्ञातम्_Not(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  यदि (ज्ञातम्_Y_2.is_ज्ञातम्_Never(fcNegative))
    ज्ञातम्.ज्ञातम्_Not(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  यदि (ज्ञातम्_Y_2.is_ज्ञातम्_Never(fc_धनात्मकम्))
    ज्ञातम्.ज्ञातम्_Not(fc_धनात्मकम्);

  प्रत्यागम apply_Output_Denormal_बहुलकम्(ज्ञातम्, बहुलकम्);
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::fpext(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src,
                                 ध्रुव fltSemantics &DstTy,
                                 ध्रुव fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  ज्ञातम्_FPClass ज्ञातम् = ज्ञातम्_Src;

  // All subnormal inputs should be in the normal range in the result type.
  यदि (APFloat::is_Representable_As_प्रसामान्यम्_अन्तर्(SrcTy, DstTy)) {
    यदि (ज्ञातम्.get_ज्ञातम्_FPClasses() & fcPosSubnormal)
      ज्ञातम्.समुच्चयः_ज्ञातम्_FPClasses(ज्ञातम्.get_ज्ञातम्_FPClasses() | fc_Pos_प्रसामान्यम्);
    यदि (ज्ञातम्.get_ज्ञातम्_FPClasses() & fcNegSubnormal)
      ज्ञातम्.समुच्चयः_ज्ञातम्_FPClasses(ज्ञातम्.get_ज्ञातम्_FPClasses() | fc_Neg_प्रसामान्यम्);
    ज्ञातम्.ज्ञातम्_Not(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  यदि (!ज्ञातम्.is_ज्ञातम्_Never_Na_N())
    ज्ञातम्.समुच्चयः_Sign_Bit(std::nullopt);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::fptrunc(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src) {
  ज्ञातम्_FPClass ज्ञातम्;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  यदि (ज्ञातम्_Src.cannotBeOrderedLessThanZero())
    ज्ञातम्.ज्ञातम्_Not(ज्ञातम्_FPClass::Ordered_Less_Than_Zero_आवरणम्);

  ज्ञातम्.propagateNonNaN(ज्ञातम्_Src);

  // Infinity needs a range check.
  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::आवर्तनम्_गन्तव्यम्_Integral(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src,
                                           तार्किक MayBeMultiUnitFPType,
                                           Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम्;

  // Integer results cannot be subnormal.
  ज्ञातम्.ज्ञातम्_Not(fcSubnormal);

  ज्ञातम्.propagateNonNaN(ज्ञातम्_Src);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  यदि (!MayBeMultiUnitFPType) {
    यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Pos_Infinity())
      ज्ञातम्.ज्ञातम्_Not(fcPosInf);
    यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Neg_Infinity())
      ज्ञातम्.ज्ञातम्_Not(fcNegInf);
  }

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fc_Pos_प्रसामान्यम् | fcPosSubnormal))
    ज्ञातम्.ज्ञातम्_Not(fc_Pos_प्रसामान्यम्);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fc_Neg_प्रसामान्यम् | fcNegSubnormal))
    ज्ञातम्.ज्ञातम्_Not(fc_Neg_प्रसामान्यम्);

  // Negative round ups towards zero produce negative zero.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcNegFinite))
    ज्ञातम्.ज्ञातम्_Not(fcNegZero);

  // Negative subnormals may flush to positive zero.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcPosFinite) &&
      (ज्ञातम्_Src.is_ज्ञातम्_Never(fcNegSubnormal) ||
       !बहुलकम्.inputs_May_Be_धनात्मकम्_Zero()))
    ज्ञातम्.ज्ञातम्_Not(fcPosZero);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::trunc(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src,
                                 Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम् = आवर्तनम्_गन्तव्यम्_Integral(ज्ञातम्_Src,
                                       /*MayBeMultiUnitFPType=*/सत्य, बहुलकम्);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Pos_Infinity())
    ज्ञातम्.ज्ञातम्_Not(fcPosInf);
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Neg_Infinity())
    ज्ञातम्.ज्ञातम्_Not(fcNegInf);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::floor(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src,
                                 तार्किक IsMultiUnitFPType, Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम् = आवर्तनम्_गन्तव्यम्_Integral(ज्ञातम्_Src, IsMultiUnitFPType, बहुलकम्);

  // Applicable for multi-unit floating point types.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Pos_Infinity())
    ज्ञातम्.ज्ञातम्_Not(fcPosInf);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::ceil(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src,
                                तार्किक IsMultiUnitFPType, Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम् = आवर्तनम्_गन्तव्यम्_Integral(ज्ञातम्_Src, IsMultiUnitFPType, बहुलकम्);

  // Applicable for multi-unit floating point types.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Neg_Infinity())
    ज्ञातम्.ज्ञातम्_Not(fcNegInf);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::frexp_mant(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src,
                                      Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम्;

  ज्ञातम्.ज्ञातम्_Not(fcSubnormal);

  ज्ञातम्.propagateNonNaN(ज्ञातम्_Src);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Pos_Infinity())
    ज्ञातम्.ज्ञातम्_Not(fcPosInf);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Neg_Infinity())
    ज्ञातम्.ज्ञातम्_Not(fcNegInf);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Logical_Pos_Zero(बहुलकम्))
    ज्ञातम्.ज्ञातम्_Not(fcPosZero);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Logical_Neg_Zero(बहुलकम्))
    ज्ञातम्.ज्ञातम्_Not(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fc_Pos_प्रसामान्यम् | fcPosSubnormal))
    ज्ञातम्.ज्ञातम्_Not(fc_Pos_प्रसामान्यम्);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fc_Neg_प्रसामान्यम् | fcNegSubnormal))
    ज्ञातम्.ज्ञातम्_Not(fc_Neg_प्रसामान्यम्);

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::ldexp(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src_2,
                                 ध्रुव APInt &ConstantRangeExpMin,
                                 ध्रुव APInt &ConstantRangeExpMax,
                                 ध्रुव fltSemantics &Flt, Denormal_बहुलकम् बहुलकम्) {
  ज्ञातम्_FPClass ज्ञातम्;

  ज्ञातम्_FPClass ज्ञातम्_Src = apply_निवेशः_Denormal_बहुलकम्(ज्ञातम्_Src_2, बहुलकम्);

  यदि (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    ज्ञातम्.समुच्चयः_ज्ञातम्_FPClasses(ज्ञातम्_Src.get_ज्ञातम्_FPClasses() | fcNan);
    ज्ञातम्.propagateNonNaN(ज्ञातम्_Src);
    प्रत्यागम apply_Output_Denormal_बहुलकम्(ज्ञातम्, बहुलकम्);
  }

  ज्ञातम्.propagateNonNaN(ज्ञातम्_Src);

  // Sign is preserved, but underflows may produce zeroes.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcNegative))
    ज्ञातम्.ज्ञातम्_Not(fcNegative);
  अन्यथा यदि (ज्ञातम्_Src.cannotBeOrderedLessThanZero())
    ज्ञातम्.ज्ञातम्_Not(Ordered_Less_Than_Zero_आवरणम्);

  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fc_धनात्मकम्))
    ज्ञातम्.ज्ञातम्_Not(fc_धनात्मकम्);
  अन्यथा यदि (ज्ञातम्_Src.cannotBeOrderedGreaterThanZero())
    ज्ञातम्.ज्ञातम्_Not(Ordered_Greater_Than_Zero_आवरणम्);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  यदि (&Flt != &APFloat::PPCDoubleDouble()) {
    अचिह्नित Precision = APFloat::semanticsPrecision(Flt);
    ध्रुव पूर्णाङ्क Mantissa_द्वयङ्काः = Precision - 1;
    यदि (ConstantRangeExpMin.sge(Mantissa_द्वयङ्काः))
      ज्ञातम्.ज्ञातम्_Not(fcSubnormal);
  }

  यदि (ConstantRangeExpMax.is_Non_धनात्मकम्()) {
    // If we know the power is <= 0, can't introduce inf
    यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Pos_Infinity())
      ज्ञातम्.ज्ञातम्_Not(fcPosInf);
    यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Neg_Infinity())
      ज्ञातम्.ज्ञातम्_Not(fcNegInf);
  } अन्यथा यदि (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Pos_Subnormal())
      ज्ञातम्.ज्ञातम्_Not(fcPosSubnormal);
    यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Neg_Subnormal())
      ज्ञातम्.ज्ञातम्_Not(fcNegSubnormal);
    यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Pos_Zero())
      ज्ञातम्.ज्ञातम्_Not(fcPosZero);
    यदि (ज्ञातम्_Src.is_ज्ञातम्_Never_Neg_Zero())
      ज्ञातम्.ज्ञातम्_Not(fcNegZero);
  }

  प्रत्यागम apply_Output_Denormal_बहुलकम्(ज्ञातम्, बहुलकम्);
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::ldexp(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src,
                                 ध्रुव ज्ञातम्_द्वयङ्काः &Exp_द्वयङ्काः,
                                 ध्रुव fltSemantics &Flt, Denormal_बहुलकम् बहुलकम्) {
  प्रत्यागम ldexp(ज्ञातम्_Src, Exp_द्वयङ्काः.get_Signed_Min_मानम्(),
               Exp_द्वयङ्काः.get_Signed_Max_मानम्(), Flt, बहुलकम्);
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::घातः(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_LHS_2,
                               ध्रुव ज्ञातम्_FPClass &ज्ञातम्_RHS_2) {
  ज्ञातम्_FPClass ज्ञातम्;

  ज्ञातम्.propagateNonSNaN(ज्ञातम्_LHS_2, ज्ञातम्_RHS_2);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcNan | fc_Neg_प्रसामान्यम् | fcNegSubnormal) &&
      ज्ञातम्_RHS_2.is_ज्ञातम्_Never_Na_N())
    ज्ञातम्.ज्ञातम्_Not(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcNegative)) {
    ज्ञातम्.ज्ञातम्_Not(fcNegative);
  } अन्यथा यदि (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fc_Neg_प्रसामान्यम् | fcNegSubnormal)) {
    ज्ञातम्.ज्ञातम्_Not(fc_Neg_प्रसामान्यम् | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    यदि ((ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcNegZero) ||
         ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fc_Pos_प्रसामान्यम्)) &&
        (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcNegInf) || ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fc_Neg_प्रसामान्यम्)))
      ज्ञातम्.ज्ञातम्_Not(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    यदि ((ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcNegZero) ||
         ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fc_Neg_प्रसामान्यम्)) &&
        (ज्ञातम्_LHS_2.is_ज्ञातम्_Never(fcNegInf) || ज्ञातम्_RHS_2.is_ज्ञातम्_Never(fc_Pos_प्रसामान्यम्)))
      ज्ञातम्.ज्ञातम्_Not(fcNegInf);
  }

  प्रत्यागम ज्ञातम्;
}

ज्ञातम्_FPClass ज्ञातम्_FPClass::powi(ध्रुव ज्ञातम्_FPClass &ज्ञातम्_Src,
                                ध्रुव ज्ञातम्_द्वयङ्काः &Exponent_ज्ञातम्_द्वयङ्काः) {
  ज्ञातम्_FPClass ज्ञातम्;
  ज्ञातम्.propagateNonNaN(ज्ञातम्_Src);

  यदि (Exponent_ज्ञातम्_द्वयङ्काः.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcSNan)) {
      ज्ञातम्.ज्ञातम्_Not(~fc_Pos_प्रसामान्यम्);
      प्रत्यागम ज्ञातम्;
    }

    ज्ञातम्.ज्ञातम्_Not(~(fc_Pos_प्रसामान्यम् | fcNan));
    प्रत्यागम ज्ञातम्;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fcNegative) || Exponent_ज्ञातम्_द्वयङ्काः.isEven()) {
    ज्ञातम्.ज्ञातम्_Not(fcNegative);
  } अन्यथा यदि (ज्ञातम्_Src.is_ज्ञातम्_Never(fc_Neg_प्रसामान्यम् | fcNegSubnormal)) {
    ज्ञातम्.ज्ञातम्_Not(fc_Neg_प्रसामान्यम् | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    ध्रुव तार्किक Is_ज्ञातम्_Never_Odd_धनात्मकम् = Exponent_ज्ञातम्_द्वयङ्काः.isNegative();
    ध्रुव तार्किक Is_ज्ञातम्_Never_Odd_Negative = Exponent_ज्ञातम्_द्वयङ्काः.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    यदि ((ज्ञातम्_Src.is_ज्ञातम्_Never(fcNegZero) || Is_ज्ञातम्_Never_Odd_धनात्मकम्) &&
        (ज्ञातम्_Src.is_ज्ञातम्_Never(fcNegInf) || Is_ज्ञातम्_Never_Odd_Negative))
      ज्ञातम्.ज्ञातम्_Not(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    यदि ((ज्ञातम्_Src.is_ज्ञातम्_Never(fcNegZero) || Is_ज्ञातम्_Never_Odd_Negative) &&
        (ज्ञातम्_Src.is_ज्ञातम्_Never(fcNegInf) || Is_ज्ञातम्_Never_Odd_धनात्मकम्))
      ज्ञातम्.ज्ञातम्_Not(fcNegInf);
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
    APInt MinExp = Exponent_ज्ञातम्_द्वयङ्काः.get_Signed_Min_मानम्();
    APInt MaxExp = Exponent_ज्ञातम्_द्वयङ्काः.get_Signed_Max_मानम्();

    // powi(inf, exp), exp > 0
    तार्किक MayInfSrc =
        !ज्ञातम्_Src.is_ज्ञातम्_Never(fcInf) && MaxExp.is_Strictly_धनात्मकम्();

    // powi(+/-0, exp), exp < 0
    तार्किक MayDivByZero = !ज्ञातम्_Src.is_ज्ञातम्_Never(fcZero) && MinExp.isNegative();

    // powi(finite, exp), |exp| > 1
    तार्किक MayFinite = !ज्ञातम्_Src.is_ज्ञातम्_Never(fc_प्रसामान्यम् | fcSubnormal);
    तार्किक MayAbsExpGT1 = MinExp.slt(-1) || MaxExp.sgt(1);
    तार्किक May_Finite_अतिप्रवाहः = MayFinite && MayAbsExpGT1;

    // powi(subnormal, -1)
    तार्किक MayBeNegOne = Exponent_ज्ञातम्_द्वयङ्काः.Zero.isZero();
    तार्किक MaySubnormInv = !ज्ञातम्_Src.is_ज्ञातम्_Never(fcSubnormal) && MayBeNegOne;

    यदि (!MayInfSrc && !MayDivByZero && !May_Finite_अतिप्रवाहः && !MaySubnormInv)
      ज्ञातम्.ज्ञातम्_Not(fcInf);
  }

  प्रत्यागम ज्ञातम्;
}
