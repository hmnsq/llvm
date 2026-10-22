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

#অন্তর্ভাব "llvm/Support/KnownFPClass.h"
#অন্তর্ভাব "llvm/ADT/APFloat.h"
#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
#অন্তর্ভাব "llvm/Support/KnownBits.h"

using নামাকাশ llvm;

KnownFPClass::KnownFPClass(ধ্রুবক APFloat &C)
    : Known_FPClasses_মান(C.classify()) {
  setSignBit(C.isNegative());
}

KnownFPClass KnownFPClass::apply_প্রবেশ_Denormal_Mode(ধ্রুবক KnownFPClass &KnownSrc,
                                                  DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  নির্বাচন (Mode.প্রবেশ) {
  ক্ষেত্র DenormalMode::IEEE:
    ফেরত Known;
  ক্ষেত্র DenormalMode::PreserveSign:
    যদি (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    যদি (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    ফেরত Known;
  ক্ষেত্র DenormalMode::PositiveZero:
    যদি (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      যদি (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    ফেরত Known;
  পূর্বনির্ধারিত:
    যদি (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      যদি (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    যদি (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    ফেরত Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

KnownFPClass KnownFPClass::applyOutputDenormalMode(ধ্রুবক KnownFPClass &KnownSrc,
                                                   DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  নির্বাচন (Mode.Output) {
  ক্ষেত্র DenormalMode::IEEE:
    ফেরত Known;
  ক্ষেত্র DenormalMode::PreserveSign:
    যদি (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    যদি (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    ফেরত Known;
  ক্ষেত্র DenormalMode::PositiveZero:
    যদি (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      যদি (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    ফেরত Known;
  পূর্বনির্ধারিত:
    যদি (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      যদি (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    যদি (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    ফেরত Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

যৌক্তিক KnownFPClass::isKnownNeverLogicalZero(DenormalMode Mode) ধ্রুবক {
  ফেরত apply_প্রবেশ_Denormal_Mode(*এই, Mode).isKnownNeverZero();
}

যৌক্তিক KnownFPClass::isKnownNeverLogicalNegZero(DenormalMode Mode) ধ্রুবক {
  ফেরত apply_প্রবেশ_Denormal_Mode(*এই, Mode).isKnownNeverNegZero();
}

যৌক্তিক KnownFPClass::isKnownNeverLogicalPosZero(DenormalMode Mode) ধ্রুবক {
  ফেরত apply_প্রবেশ_Denormal_Mode(*এই, Mode).isKnownNeverPosZero();
}

KnownFPClass KnownFPClass::minMaxLike(ধ্রুবক KnownFPClass &KnownLHS_,
                                      ধ্রুবক KnownFPClass &KnownRHS_,
                                      MinMaxKind Kind, DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_প্রবেশ_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_প্রবেশ_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonNaN(KnownLHS, KnownRHS);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  যদি ((KnownLHS.isKnownNeverNaN() || KnownRHS.isKnownNeverNaN()) &&
      (Kind == MinMaxKind::minnum || Kind == MinMaxKind::maxnum ||
       Kind == MinMaxKind::minimumnum || Kind == MinMaxKind::maximumnum))
    Known.knownNot(fcNan);

  নির্বাচন (Kind) {
  ক্ষেত্র MinMaxKind::maxnum: {
    যদি (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/মিথ্যা));
    যদি (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/মিথ্যা));
    বিরতি;
  }
  ক্ষেত্র MinMaxKind::minnum: {
    যদি (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/মিথ্যা));
    যদি (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/মিথ্যা));
    বিরতি;
  }
  ক্ষেত্র MinMaxKind::maximumnum: {
    যদি (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/সত্য));
    যদি (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/সত্য));
    বিরতি;
  }
  ক্ষেত্র MinMaxKind::minimumnum: {
    যদি (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/সত্য));
    যদি (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/সত্য));
    বিরতি;
  }
  ক্ষেত্র MinMaxKind::maximum: {
    Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/সত্য) |
                   orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/সত্য));
    বিরতি;
  }
  ক্ষেত্র MinMaxKind::minimum: {
    Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/সত্য) |
                   orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/সত্য));
    বিরতি;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      KnownLHS.getKnownFPClasses() | KnownRHS.getKnownFPClasses() | fcNan;
  Known.knownNot(~PossibleClasses);

  ফেরত applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::canonicalize(ধ্রুবক KnownFPClass &KnownSrc,
                                        DenormalMode DenormMode) {
  KnownFPClass Known;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  যদি (KnownSrc.isKnownNeverNaN())
    Known.knownNot(fcNan);
  অন্যথা
    Known.knownNot(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  যদি (DenormMode == DenormalMode::getIEEE()) {
    যদি (KnownSrc.isKnownNever(fcPosZero))
      Known.knownNot(fcPosZero);
    যদি (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);
    ফেরত Known;
  }

  যদি (DenormMode.inputsAreZero() || DenormMode.outputsAreZero())
    Known.knownNot(fcSubnormal);

  যদি (DenormMode == DenormalMode::getPreserveSign()) {
    যদি (KnownSrc.isKnownNever(fcPosZero | fcPosSubnormal))
      Known.knownNot(fcPosZero);
    যদি (KnownSrc.isKnownNever(fcNegZero | fcNegSubnormal))
      Known.knownNot(fcNegZero);
    ফেরত Known;
  }

  যদি (DenormMode.প্রবেশ == DenormalMode::PositiveZero ||
      (DenormMode.Output == DenormalMode::PositiveZero &&
       DenormMode.প্রবেশ == DenormalMode::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    যদি (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    যদি (KnownSrc.isKnownNever(fcPosZero | fcSubnormal))
      Known.knownNot(fcPosZero);
  }

  ফেরত Known;
}

KnownFPClass KnownFPClass::bitcast(ধ্রুবক fltSemantics &FltSemantics,
                                   ধ্রুবক KnownBits &Bits) {
  প্রতিজ্ঞা(FltSemantics.আকার_In_Bits == Bits.getBitWidth() &&
         "Bitcast operand has incorrect bit width");
  KnownFPClass Known;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  যদি (Bits.hasConflict())
    ফেরত Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](ধ্রুবক fltSemantics &Semantics) {
    নির্বাচন (APFloat::SemanticsToEnum(Semantics)) {
    ক্ষেত্র APFloatBase::S_IEEEhalf:
    ক্ষেত্র APFloatBase::S_BFloat:
    ক্ষেত্র APFloatBase::S_IEEEsingle:
    ক্ষেত্র APFloatBase::S_IEEEdouble:
    ক্ষেত্র APFloatBase::S_IEEEquad:
    ক্ষেত্র APFloatBase::S_x87DoubleExtended:
      ফেরত সত্য;
    পূর্বনির্ধারিত:
      ফেরত মিথ্যা;
    }
  };
  যদি (!IsSupported(FltSemantics))
    ফেরত Known;

  // Transfer information from the sign bit.
  যদি (Bits.isNonNegative())
    Known.signBitMustBeZero();
  অন্যথা যদি (Bits.isNegative())
    Known.signBitMustBeOne();

  নির্বাচন (APFloatBase::SemanticsToEnum(FltSemantics)) {
  ক্ষেত্র APFloatBase::S_IEEEhalf:
  ক্ষেত্র APFloatBase::S_BFloat:
  ক্ষেত্র APFloatBase::S_IEEEsingle:
  ক্ষেত্র APFloatBase::S_IEEEdouble:
  ক্ষেত্র APFloatBase::S_IEEEquad: {
    ধ্রুবক unsigned MantissaBits = FltSemantics.precision - 1;
    ধ্রুবক APInt ExponentMask = APInt::getBitsSet(
        FltSemantics.আকার_In_Bits, MantissaBits, FltSemantics.আকার_In_Bits - 1);
    ধ্রুবক APInt MantissaMask =
        APInt::get_নিচের_Bits_Set(FltSemantics.আকার_In_Bits, MantissaBits);

    ধ্রুবক যৌক্তিক ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    ধ্রুবক যৌক্তিক ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    ধ্রুবক যৌক্তিক ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    ধ্রুবক যৌক্তিক ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();

    ধ্রুবক যৌক্তিক MantissaKnownAllZeros =
        (Bits.Zero & MantissaMask) == MantissaMask;
    ধ্রুবক যৌক্তিক MantissaKnownNotAllZeros = !(Bits.One & MantissaMask).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    যদি (ExponentKnownNotAllZeros)
      Known.knownNot(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    যদি (ExponentKnownNotAllOnes)
      Known.knownNot(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    যদি (ExponentKnownAllZeros || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // Zero and infinity require a mantissa with all zero bits.
    যদি (MantissaKnownNotAllZeros)
      Known.knownNot(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    যদি (MantissaKnownAllZeros)
      Known.knownNot(fcSubnormal | fcNan);

    ধ্রুবক যৌক্তিক QuietBitKnownSet = Bits.One[MantissaBits - 1];
    ধ্রুবক যৌক্তিক QuietBitKnownClear = Bits.Zero[MantissaBits - 1];

    যদি (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    অন্যথা যদি (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    বিরতি;
  }
  ক্ষেত্র APFloatBase::S_x87DoubleExtended: {
    // Layout of the x87 floating point type:
    // [0:62] fraction
    // [63] explicit integer bit
    // [64:78] exponent
    // [79] sign bit
    // APFloat treats the x87 pseudo encodings as documented in
    // fltSemantics::hasExplicitIntegerBit.
    constexpr unsigned BitWidth = 80;
    constexpr unsigned FractionBits = 63;
    constexpr unsigned Integer_Bit_সূচক = FractionBits;
    ধ্রুবক APInt FractionMask = APInt::get_নিচের_Bits_Set(BitWidth, FractionBits);
    ধ্রুবক APInt ExponentMask =
        APInt::getBitsSet(BitWidth, FractionBits + 1, BitWidth - 1);

    ধ্রুবক যৌক্তিক ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    ধ্রুবক যৌক্তিক ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    ধ্রুবক যৌক্তিক ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    ধ্রুবক যৌক্তিক ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();
    ধ্রুবক যৌক্তিক FractionKnownAllZeros =
        (Bits.Zero & FractionMask) == FractionMask;
    ধ্রুবক যৌক্তিক FractionKnownNotAllZeros = !(Bits.One & FractionMask).isZero();
    ধ্রুবক যৌক্তিক IntegerBitKnownSet = Bits.One[Integer_Bit_সূচক];
    ধ্রুবক যৌক্তিক IntegerBitKnownClear = Bits.Zero[Integer_Bit_সূচক];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    যদি (ExponentKnownNotAllZeros || IntegerBitKnownSet)
      Known.knownNot(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    যদি (FractionKnownNotAllZeros)
      Known.knownNot(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    যদি (ExponentKnownNotAllOnes || IntegerBitKnownClear ||
        FractionKnownNotAllZeros)
      Known.knownNot(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    যদি (IntegerBitKnownClear || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    যদি (FractionKnownAllZeros)
      Known.knownNot(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    যদি (ExponentKnownAllZeros ||
        (IntegerBitKnownSet &&
         (ExponentKnownNotAllOnes || FractionKnownAllZeros)))
      Known.knownNot(fcNan);

    ধ্রুবক যৌক্তিক QuietBitKnownSet = Bits.One[FractionBits - 1];
    ধ্রুবক যৌক্তিক QuietBitKnownClear = Bits.Zero[FractionBits - 1];

    যদি (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    অন্যথা যদি (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    বিরতি;
  }
  পূর্বনির্ধারিত:
    llvm_unreachable("unhandled supported type");
  }

  ফেরত Known;
}

KnownBits KnownFPClass::toKnownBits(ধ্রুবক fltSemantics &FltSemantics) ধ্রুবক {
  KnownBits Known(FltSemantics.আকার_In_Bits);
  ধ্রুবক FPClassTest FPClasses = getKnownFPClasses();

  // Return unknown if poison.
  যদি (FPClasses == fcNone)
    ফেরত Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](ধ্রুবক fltSemantics &Semantics) {
    নির্বাচন (APFloat::SemanticsToEnum(Semantics)) {
    ক্ষেত্র APFloatBase::S_IEEEhalf:
    ক্ষেত্র APFloatBase::S_BFloat:
    ক্ষেত্র APFloatBase::S_IEEEsingle:
    ক্ষেত্র APFloatBase::S_IEEEdouble:
    ক্ষেত্র APFloatBase::S_IEEEquad:
    ক্ষেত্র APFloatBase::S_x87DoubleExtended:
      ফেরত সত্য;
    পূর্বনির্ধারিত:
      ফেরত মিথ্যা;
    }
  };
  যদি (!IsSupported(FltSemantics))
    ফেরত Known;

  নির্বাচন (APFloat::SemanticsToEnum(FltSemantics)) {
  ক্ষেত্র APFloatBase::S_IEEEhalf:
  ক্ষেত্র APFloatBase::S_BFloat:
  ক্ষেত্র APFloatBase::S_IEEEsingle:
  ক্ষেত্র APFloatBase::S_IEEEdouble:
  ক্ষেত্র APFloatBase::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    যদি (FPClasses & fcNormal)
      বিরতি;

    Known.setAllConflict();

    ধ্রুবক unsigned BitWidth = FltSemantics.আকার_In_Bits;
    ধ্রুবক unsigned MantissaBits = FltSemantics.precision - 1;
    ধ্রুবক unsigned ExponentBits = BitWidth - MantissaBits - 1;

    APInt MantissaMask = APInt::get_নিচের_Bits_Set(BitWidth, MantissaBits);
    APInt ExponentMask =
        APInt::getBitsSet(BitWidth, MantissaBits, MantissaBits + ExponentBits);

    ধ্রুবক unsigned Quiet_Bit_সূচক = MantissaBits - 1;
    APInt PayloadMask = MantissaMask;
    PayloadMask.clearBit(Quiet_Bit_সূচক);

    যদি (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // No individual payload bit is known.
      Known.Zero &= ~PayloadMask;
      Known.One &= ~PayloadMask;

      যদি (FPClasses & fcQNan)
        Known.Zero.clearBit(Quiet_Bit_সূচক);
      যদি (FPClasses & fcSNan)
        Known.One.clearBit(Quiet_Bit_সূচক);
    }
    যদি (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // Mantissa bits cannot be ones.
      Known.One &= ~MantissaMask;
    }
    যদি (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Unknown mantissa.
      Known.One &= ~MantissaMask;
      Known.Zero &= ~MantissaMask;
    }
    যদি (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Mantissa cannot be ones.
      Known.One &= ~MantissaMask;
    }

    বিরতি;
  }
  ক্ষেত্র APFloatBase::S_x87DoubleExtended: {
    যদি (!isKnownNever(fcNormal | fcSubnormal | fcNan))
      বিরতি;

    Known.setAllConflict();

    যদি (FPClasses & fcInf)
      Known = Known.intersectWith(KnownBits::makeConstant(
          APFloat::getInf(FltSemantics).bitcastToAPInt()));

    যদি (FPClasses & fcZero)
      Known = Known.intersectWith(
          KnownBits::makeConstant(APInt::getZero(FltSemantics.আকার_In_Bits)));

    বিরতি;
  }
  পূর্বনির্ধারিত:
    llvm_unreachable("unhandled supported semantics");
  }

  Known.Zero.clearSignBit();
  Known.One.clearSignBit();
  যদি (std::optional<যৌক্তিক> Sign = getSignBit()) {
    যদি (*Sign)
      Known.makeNegative();
    অন্যথা
      Known.makeNonNegative();
  }

  ফেরত Known;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
স্থির KnownFPClass fadd_impl(ধ্রুবক KnownFPClass &KnownLHS,
                              ধ্রুবক KnownFPClass &KnownRHS, যৌক্তিক IsFMA) {
  KnownFPClass Known;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  যদি (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNever(fcPosInf) || KnownRHS.isKnownNever(fcNegInf)) &&
      (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosInf)))
    Known.knownNot(fcNan);

  যদি (KnownLHS.cannotBeOrderedLessThanZero() &&
      KnownRHS.cannotBeOrderedLessThanZero()) {
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    যদি (KnownLHS.isKnownNever(fcZero | fcPosSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcPosSubnormal))
      Known.knownNot(fcZero | fcPosSubnormal);
  }

  যদি (KnownLHS.cannotBeOrderedGreaterThanZero() &&
      KnownRHS.cannotBeOrderedGreaterThanZero()) {
    Known.knownNot(KnownFPClass::OrderedGreaterThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    যদি (KnownLHS.isKnownNever(fcZero | fcNegSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcNegSubnormal))
      Known.knownNot(fcZero | fcNegSubnormal);
  }

  যদি (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    যদি (KnownLHS.isKnownNever(fcNegZero) || KnownRHS.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    Known.propagateNonSNaN(KnownLHS, KnownRHS);
  }

  ফেরত Known;
}

KnownFPClass KnownFPClass::fadd(ধ্রুবক KnownFPClass &KnownLHS_,
                                ধ্রুবক KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_প্রবেশ_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_প্রবেশ_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/মিথ্যা);

  ফেরত applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fadd_self(ধ্রুবক KnownFPClass &KnownSrc_,
                                     DenormalMode Mode) {
  KnownFPClass KnownSrc = apply_প্রবেশ_Denormal_Mode(KnownSrc_, Mode);

  KnownFPClass Known = fadd_impl(KnownSrc, KnownSrc, /*IsFMA=*/মিথ্যা);

  যদি (KnownSrc.isKnownNever(fcPosSubnormal))
    Known.knownNot(fcPosSubnormal);
  যদি (KnownSrc.isKnownNever(fcNegSubnormal))
    Known.knownNot(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  যদি (KnownSrc.isKnownNever(fcPosZero))
    Known.knownNot(fcPosZero);

  ফেরত applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fsub(ধ্রুবক KnownFPClass &KnownLHS_,
                                ধ্রুবক KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_প্রবেশ_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_প্রবেশ_Denormal_Mode(KnownRHS_, Mode);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  KnownRHS.fneg();

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/মিথ্যা);

  ফেরত applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fmul(ধ্রুবক KnownFPClass &KnownLHS_,
                                ধ্রুবক KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_প্রবেশ_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_প্রবেশ_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  যদি ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  যদি ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Inf * Y => Inf or NaN
  যদি (KnownLHS.isKnownAlways(fcInf | fcNan) ||
      KnownRHS.isKnownAlways(fcInf | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  যদি (KnownRHS.isKnownAlways(fcZero | fcNan) ||
      KnownLHS.isKnownAlways(fcZero | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  যদি (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownRHS.isKnownNeverInfinity() || KnownLHS.isKnownNeverZero()) &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverZero()))
    Known.knownNot(fcNan);

  ফেরত applyOutputDenormalMode(Known, Mode);
}

// TODO: This generalizes to known ranges
KnownFPClass KnownFPClass::fmul(ধ্রুবক KnownFPClass &KnownLHS,
                                ধ্রুবক APFloat &CRHS, DenormalMode Mode) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  ধ্রুবক fltSemantics &Flt = CRHS.getSemantics();
  unsigned Precision = APFloat::semanticsPrecision(Flt);
  ধ্রুবক পূর্ণসংখ্যা MantissaBits = Precision - 1;

  পূর্ণসংখ্যা MinKnownExponent = ilogb(CRHS);
  যৌক্তিক CannotBeSubnormal = (MinKnownExponent >= MantissaBits);

  KnownFPClass Known = KnownFPClass::fmul(KnownLHS, KnownFPClass(CRHS), Mode);
  যদি (CannotBeSubnormal)
    Known.knownNot(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  যদি (KnownLHS.isKnownNever(fcInf)) {
    যদি (MinKnownExponent < 0)
      Known.knownNot(fcInf);
    অন্যথা যদি (MinKnownExponent == 0 && CRHS.compare_Absolute_মান(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      Known.knownNot(fcInf);
  }

  ফেরত Known;
}

KnownFPClass KnownFPClass::fdiv(ধ্রুবক KnownFPClass &KnownLHS_,
                                ধ্রুবক KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_প্রবেশ_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_প্রবেশ_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // Only 0/0, Inf/Inf produce NaN.
  যদি (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverInfinity()) &&
      (KnownLHS.isKnownNeverZero() || KnownRHS.isKnownNeverZero())) {
    Known.knownNot(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  যদি ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  যদি ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Normal and subnormal results require two non-zero finite operands.
  যদি ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)))
    Known.knownNot(fcNegNormal | fcNegSubnormal);
  যদি ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosNormal | fcPosSubnormal);

  // 0 / X => 0 or NaN
  যদি (KnownLHS.isKnownAlways(fcZero))
    Known.knownNot(fcSubnormal | fcNormal | fcInf);

  // X / 0 => NaN or Inf
  যদি (KnownRHS.isKnownAlways(fcZero))
    Known.knownNot(fcFinite);

  ফেরত applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fdiv_self(ধ্রুবক KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X / X is always exactly 1.0 or a NaN.
  KnownFPClass Known(fcNan | fcPosNormal);

  Known.propagateNonSNaN(KnownSrc);

  যদি (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  ফেরত Known;
}

KnownFPClass KnownFPClass::frem(ধ্রুবক KnownFPClass &KnownLHS,
                                ধ্রুবক KnownFPClass &KnownRHS,
                                DenormalMode Mode) {
  KnownFPClass Known;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  যদি (KnownLHS.isKnownNeverInfOrNaN() && KnownRHS.isKnownNeverNaN() &&
      KnownRHS.isKnownNeverLogicalZero(Mode)) {
    Known.knownNot(fcNan);
  }

  // The sign for frem is the same as the first operand.
  যদি (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal | fcPosSubnormal);
  যদি (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  যদি (KnownLHS.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  যদি (KnownLHS.isKnownNever(fcPosFinite) &&
      KnownLHS.isKnownNeverLogicalPosZero(Mode) &&
      (!Mode.outputsMayBePositiveZero() ||
       KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosZero);

  ফেরত Known;
}

KnownFPClass KnownFPClass::frem_self(ধ্রুবক KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X % X is always exactly [+-]0.0 or a NaN.
  KnownFPClass Known(fcNan | fcZero);

  Known.propagateNonSNaN(KnownSrc);

  যদি (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  যদি (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosZero);
  যদি (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegZero);

  ফেরত Known;
}

KnownFPClass KnownFPClass::fma(ধ্রুবক KnownFPClass &KnownLHS,
                               ধ্রুবক KnownFPClass &KnownRHS,
                               ধ্রুবক KnownFPClass &KnownAddend,
                               DenormalMode Mode) {
  KnownFPClass Mul = fmul(KnownLHS, KnownRHS, Mode);

  // FMA differs from the base fmul + fadd handling only in the treatment of -0
  // results.
  //
  // If the multiply is a -0 due to rounding, the final -0 + 0 will be -0,
  // unlike for a separate fadd.
  KnownFPClass Known = fadd_impl(Mul, KnownAddend, /*IsFMA=*/সত্য);

  // propagateNonSNaN for 3 arguments.
  যদি (KnownLHS.isKnownNever(fcSNan) && KnownRHS.isKnownNever(fcSNan) &&
      KnownAddend.isKnownNever(fcSNan))
    Known.knownNot(fcSNan);

  ফেরত Known;
}

KnownFPClass KnownFPClass::fmuladd(ধ্রুবক KnownFPClass &KnownLHS,
                                   ধ্রুবক KnownFPClass &KnownRHS,
                                   ধ্রুবক KnownFPClass &KnownAddend,
                                   DenormalMode Mode) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  KnownFPClass FMA = fma(KnownLHS, KnownRHS, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(fmul(KnownLHS, KnownRHS, Mode), KnownAddend, Mode);
  ফেরত FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::fma_square(ধ্রুবক KnownFPClass &KnownSquared,
                                      ধ্রুবক KnownFPClass &KnownAddend,
                                      DenormalMode Mode) {
  KnownFPClass Squared = square(KnownSquared, Mode);
  KnownFPClass Known = fadd_impl(Squared, KnownAddend, /*IsFMA=*/সত্য);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  যদি (KnownAddend.isKnownNever(fcNegInf | fcNan) && Squared.isKnownNever(fcNan))
    Known.knownNot(fcNan);

  Known.propagateNonSNaN(KnownSquared, KnownAddend);

  ফেরত Known;
}

KnownFPClass KnownFPClass::fmuladd_square(ধ্রুবক KnownFPClass &KnownSquared,
                                          ধ্রুবক KnownFPClass &KnownAddend,
                                          DenormalMode Mode) {
  KnownFPClass FMA = fma_square(KnownSquared, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(square(KnownSquared, Mode), KnownAddend, Mode);
  ফেরত FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::exp(ধ্রুবক KnownFPClass &KnownSrc) {
  KnownFPClass Known;
  Known.knownNot(fcNegative);

  Known.propagateNonNaN(KnownSrc);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  যদি (KnownSrc.isKnownNever(fcNegNormal | fcNegInf))
    Known.knownNot(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  যদি (KnownSrc.isKnownNever(fcNegNormal))
    Known.knownNot(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  যদি (KnownSrc.isKnownNever(fcPosNormal | fcPosInf))
    Known.knownNot(fcPosInf);

  ফেরত Known;
}

KnownFPClass KnownFPClass::log(ধ্রুবক KnownFPClass &KnownSrc, DenormalMode Mode,
                               যৌক্তিক IsMultiUnitFPType) {
  KnownFPClass Known;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  যদি (!IsMultiUnitFPType)
    Known.knownNot(fcNegZero | fcSubnormal);

  Known.propagateNonSNaN(KnownSrc);

  যদি (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  যদি (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  যদি (KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNegInf);

  ফেরত Known;
}

KnownFPClass KnownFPClass::sqrt(ধ্রুবক KnownFPClass &KnownSrc,
                                DenormalMode Mode) {
  KnownFPClass Known;
  Known.knownNot(fcPosSubnormal);

  যদি (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  Known.propagateNonSNaN(KnownSrc);

  // Any negative value besides -0 returns a nan.
  যদি (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  Known.knownNot(fcNegInf | fcNegSubnormal | fcNegNormal);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  যদি (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  ফেরত Known;
}

KnownFPClass KnownFPClass::sin(ধ্রুবক KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownSrc);

  // Return NaN on infinite inputs.
  Known.knownNot(fcInf);
  যদি (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  ফেরত Known;
}

KnownFPClass KnownFPClass::cos(ধ্রুবক KnownFPClass &KnownSrc) {
  ফেরত sin(KnownSrc);
}

KnownFPClass KnownFPClass::tan(ধ্রুবক KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. tan(+-Inf) is NaN.
  যদি (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  ফেরত Known;
}

KnownFPClass KnownFPClass::sinh(ধ্রুবক KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  যদি (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  যদি (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal | fcPosInf);

  Known.propagateNonNaN(KnownSrc);

  ফেরত Known;
}

KnownFPClass KnownFPClass::cosh(ধ্রুবক KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  Known.knownNot(fcNegative | fcZero | fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  ফেরত Known;
}

KnownFPClass KnownFPClass::tanh(ধ্রুবক KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tanh is bounded to (-1, 1), never Inf.
  Known.knownNot(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  যদি (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf)) {
    Known.knownNot(fcNegSubnormal | fcNegNormal);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    যদি (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  যদি (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  Known.propagateNonNaN(KnownSrc);

  ফেরত Known;
}

KnownFPClass KnownFPClass::asin(ধ্রুবক KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // asin is sign-preserving for finite arguments.
  যদি (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegFinite);

  যদি (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  ফেরত Known;
}

KnownFPClass KnownFPClass::acos(ধ্রুবক KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // acos(x) is bounded to [0, pi] for -1 <= x <= 1, and is never negative,
  // infinite, or subnormal. The smallest non-zero value occurs when x is
  // close to 1.0, where acos(x) can be approximated by sqrt(2 * (1 - x)).
  // Since sqrt cannot produce a subnormal result, we can conclude that
  // acos(x) will also never produce a subnormal result.
  Known.knownNot(fcNegative | fcInf | fcSubnormal);

  // acos(x) == +0.0 iff x == +1.0
  যদি (KnownSrc.isKnownNever(fcPosNormal))
    Known.knownNot(fcZero);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  ফেরত Known;
}

KnownFPClass KnownFPClass::atan(ধ্রুবক KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  Known.knownNot(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  যদি (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf))
    Known.knownNot(fcNegSubnormal | fcNegNormal);

  যদি (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  যদি (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal | fcNegZero))
    Known.knownNot(fcNegZero);

  Known.propagateNonNaN(KnownSrc);

  ফেরত Known;
}

KnownFPClass KnownFPClass::atan2(ধ্রুবক KnownFPClass &KnownY_,
                                 ধ্রুবক KnownFPClass &KnownX_,
                                 DenormalMode Mode) {
  KnownFPClass KnownY = apply_প্রবেশ_Denormal_Mode(KnownY_, Mode);
  KnownFPClass KnownX = apply_প্রবেশ_Denormal_Mode(KnownX_, Mode);
  KnownFPClass Known;

  // Even though these deductions are correct, we are ignoring the following
  // potentially erroneous cases:
  //   * atan2(y, inf) is not subnormal
  //   * atan2(inf, x) is not zero or subnormal

  // atan2 result is in (-pi, pi], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonNaN(KnownY, KnownX);

  // If x <= -0.0, then |atan2(y, x)| >= pi/2
  যদি (KnownX.isKnownNever(fcPositive))
    Known.knownNot(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  যদি (KnownY.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  যদি (KnownY.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);

  ফেরত applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fpext(ধ্রুবক KnownFPClass &KnownSrc,
                                 ধ্রুবক fltSemantics &DstTy,
                                 ধ্রুবক fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  KnownFPClass Known = KnownSrc;

  // All subnormal inputs should be in the normal range in the result type.
  যদি (APFloat::isRepresentableAsNormalIn(SrcTy, DstTy)) {
    যদি (Known.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosNormal);
    যদি (Known.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegNormal);
    Known.knownNot(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  যদি (!Known.isKnownNeverNaN())
    Known.setSignBit(std::nullopt);

  ফেরত Known;
}

KnownFPClass KnownFPClass::fptrunc(ধ্রুবক KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  যদি (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

  Known.propagateNonNaN(KnownSrc);

  // Infinity needs a range check.
  ফেরত Known;
}

KnownFPClass KnownFPClass::roundToIntegral(ধ্রুবক KnownFPClass &KnownSrc,
                                           যৌক্তিক MayBeMultiUnitFPType,
                                           DenormalMode Mode) {
  KnownFPClass Known;

  // Integer results cannot be subnormal.
  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  যদি (!MayBeMultiUnitFPType) {
    যদি (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    যদি (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  }

  যদি (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  যদি (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  // Negative round ups towards zero produce negative zero.
  যদি (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // Negative subnormals may flush to positive zero.
  যদি (KnownSrc.isKnownNever(fcPosFinite) &&
      (KnownSrc.isKnownNever(fcNegSubnormal) ||
       !Mode.inputsMayBePositiveZero()))
    Known.knownNot(fcPosZero);

  ফেরত Known;
}

KnownFPClass KnownFPClass::trunc(ধ্রুবক KnownFPClass &KnownSrc,
                                 DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc,
                                       /*MayBeMultiUnitFPType=*/সত্য, Mode);

  যদি (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);
  যদি (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  ফেরত Known;
}

KnownFPClass KnownFPClass::floor(ধ্রুবক KnownFPClass &KnownSrc,
                                 যৌক্তিক IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  যদি (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  ফেরত Known;
}

KnownFPClass KnownFPClass::ceil(ধ্রুবক KnownFPClass &KnownSrc,
                                যৌক্তিক IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  যদি (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  ফেরত Known;
}

KnownFPClass KnownFPClass::frexp_mant(ধ্রুবক KnownFPClass &KnownSrc,
                                      DenormalMode Mode) {
  KnownFPClass Known;

  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  যদি (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  যদি (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  যদি (KnownSrc.isKnownNeverLogicalPosZero(Mode))
    Known.knownNot(fcPosZero);

  যদি (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  যদি (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  যদি (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  ফেরত Known;
}

KnownFPClass KnownFPClass::ldexp(ধ্রুবক KnownFPClass &KnownSrc_,
                                 ধ্রুবক APInt &ConstantRangeExpMin,
                                 ধ্রুবক APInt &ConstantRangeExpMax,
                                 ধ্রুবক fltSemantics &Flt, DenormalMode Mode) {
  KnownFPClass Known;

  KnownFPClass KnownSrc = apply_প্রবেশ_Denormal_Mode(KnownSrc_, Mode);

  যদি (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcNan);
    Known.propagateNonNaN(KnownSrc);
    ফেরত applyOutputDenormalMode(Known, Mode);
  }

  Known.propagateNonNaN(KnownSrc);

  // Sign is preserved, but underflows may produce zeroes.
  যদি (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);
  অন্যথা যদি (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(OrderedLessThanZeroMask);

  যদি (KnownSrc.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);
  অন্যথা যদি (KnownSrc.cannotBeOrderedGreaterThanZero())
    Known.knownNot(OrderedGreaterThanZeroMask);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  যদি (&Flt != &APFloat::PPCDoubleDouble()) {
    unsigned Precision = APFloat::semanticsPrecision(Flt);
    ধ্রুবক পূর্ণসংখ্যা MantissaBits = Precision - 1;
    যদি (ConstantRangeExpMin.sge(MantissaBits))
      Known.knownNot(fcSubnormal);
  }

  যদি (ConstantRangeExpMax.isNonPositive()) {
    // If we know the power is <= 0, can't introduce inf
    যদি (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    যদি (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  } অন্যথা যদি (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    যদি (KnownSrc.isKnownNeverPosSubnormal())
      Known.knownNot(fcPosSubnormal);
    যদি (KnownSrc.isKnownNeverNegSubnormal())
      Known.knownNot(fcNegSubnormal);
    যদি (KnownSrc.isKnownNeverPosZero())
      Known.knownNot(fcPosZero);
    যদি (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  ফেরত applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::ldexp(ধ্রুবক KnownFPClass &KnownSrc,
                                 ধ্রুবক KnownBits &ExpBits,
                                 ধ্রুবক fltSemantics &Flt, DenormalMode Mode) {
  ফেরত ldexp(KnownSrc, ExpBits.get_Signed_Min_মান(),
               ExpBits.get_Signed_Max_মান(), Flt, Mode);
}

KnownFPClass KnownFPClass::pow(ধ্রুবক KnownFPClass &KnownLHS,
                               ধ্রুবক KnownFPClass &KnownRHS) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  যদি (KnownLHS.isKnownNever(fcNan | fcNegNormal | fcNegSubnormal) &&
      KnownRHS.isKnownNeverNaN())
    Known.knownNot(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  যদি (KnownLHS.isKnownNever(fcNegative)) {
    Known.knownNot(fcNegative);
  } অন্যথা যদি (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    যদি ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcPosNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcNegNormal)))
      Known.knownNot(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    যদি ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcNegNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosNormal)))
      Known.knownNot(fcNegInf);
  }

  ফেরত Known;
}

KnownFPClass KnownFPClass::powi(ধ্রুবক KnownFPClass &KnownSrc,
                                ধ্রুবক KnownBits &ExponentKnownBits) {
  KnownFPClass Known;
  Known.propagateNonNaN(KnownSrc);

  যদি (ExponentKnownBits.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    যদি (KnownSrc.isKnownNever(fcSNan)) {
      Known.knownNot(~fcPosNormal);
      ফেরত Known;
    }

    Known.knownNot(~(fcPosNormal | fcNan));
    ফেরত Known;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  যদি (KnownSrc.isKnownNever(fcNegative) || ExponentKnownBits.isEven()) {
    Known.knownNot(fcNegative);
  } অন্যথা যদি (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    ধ্রুবক যৌক্তিক IsKnownNeverOddPositive = ExponentKnownBits.isNegative();
    ধ্রুবক যৌক্তিক IsKnownNeverOddNegative = ExponentKnownBits.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    যদি ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddPositive) &&
        (KnownSrc.isKnownNever(fcNegInf) || IsKnownNeverOddNegative))
      Known.knownNot(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    যদি ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddNegative) &&
        (KnownSrc.isKnownNever(fcNegInf) || IsKnownNeverOddPositive))
      Known.knownNot(fcNegInf);
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
    APInt MinExp = ExponentKnownBits.get_Signed_Min_মান();
    APInt MaxExp = ExponentKnownBits.get_Signed_Max_মান();

    // powi(inf, exp), exp > 0
    যৌক্তিক MayInfSrc =
        !KnownSrc.isKnownNever(fcInf) && MaxExp.isStrictlyPositive();

    // powi(+/-0, exp), exp < 0
    যৌক্তিক MayDivByZero = !KnownSrc.isKnownNever(fcZero) && MinExp.isNegative();

    // powi(finite, exp), |exp| > 1
    যৌক্তিক MayFinite = !KnownSrc.isKnownNever(fcNormal | fcSubnormal);
    যৌক্তিক MayAbsExpGT1 = MinExp.slt(-1) || MaxExp.sgt(1);
    যৌক্তিক MayFiniteOverflow = MayFinite && MayAbsExpGT1;

    // powi(subnormal, -1)
    যৌক্তিক MayBeNegOne = ExponentKnownBits.Zero.isZero();
    যৌক্তিক MaySubnormInv = !KnownSrc.isKnownNever(fcSubnormal) && MayBeNegOne;

    যদি (!MayInfSrc && !MayDivByZero && !MayFiniteOverflow && !MaySubnormInv)
      Known.knownNot(fcInf);
  }

  ফেরত Known;
}
