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

using नामाकाश llvm;

KnownFPClass::KnownFPClass(ध्रुव APFloat &C)
    : Known_FPClasses_मानम्(C.classify()) {
  setSignBit(C.isNegative());
}

KnownFPClass KnownFPClass::apply_निवेशः_Denormal_Mode(ध्रुव KnownFPClass &KnownSrc,
                                                  DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  विकल्प (Mode.निवेशः) {
  पक्ष DenormalMode::IEEE:
    पटिनिवत्त Known;
  पक्ष DenormalMode::PreserveSign:
    सचे (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    सचे (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    पटिनिवत्त Known;
  पक्ष DenormalMode::PositiveZero:
    सचे (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      सचे (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    पटिनिवत्त Known;
  स्वभाव:
    सचे (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      सचे (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    सचे (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    पटिनिवत्त Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

KnownFPClass KnownFPClass::applyOutputDenormalMode(ध्रुव KnownFPClass &KnownSrc,
                                                   DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  विकल्प (Mode.Output) {
  पक्ष DenormalMode::IEEE:
    पटिनिवत्त Known;
  पक्ष DenormalMode::PreserveSign:
    सचे (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    सचे (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    पटिनिवत्त Known;
  पक्ष DenormalMode::PositiveZero:
    सचे (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      सचे (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    पटिनिवत्त Known;
  स्वभाव:
    सचे (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      सचे (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    सचे (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    पटिनिवत्त Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

तार्किक KnownFPClass::isKnownNeverLogicalZero(DenormalMode Mode) ध्रुव {
  पटिनिवत्त apply_निवेशः_Denormal_Mode(*एतत्, Mode).isKnownNeverZero();
}

तार्किक KnownFPClass::isKnownNeverLogicalNegZero(DenormalMode Mode) ध्रुव {
  पटिनिवत्त apply_निवेशः_Denormal_Mode(*एतत्, Mode).isKnownNeverNegZero();
}

तार्किक KnownFPClass::isKnownNeverLogicalPosZero(DenormalMode Mode) ध्रुव {
  पटिनिवत्त apply_निवेशः_Denormal_Mode(*एतत्, Mode).isKnownNeverPosZero();
}

KnownFPClass KnownFPClass::minMaxLike(ध्रुव KnownFPClass &KnownLHS_,
                                      ध्रुव KnownFPClass &KnownRHS_,
                                      MinMaxKind Kind, DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_निवेशः_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_निवेशः_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonNaN(KnownLHS, KnownRHS);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  सचे ((KnownLHS.isKnownNeverNaN() || KnownRHS.isKnownNeverNaN()) &&
      (Kind == MinMaxKind::minnum || Kind == MinMaxKind::maxnum ||
       Kind == MinMaxKind::minimumnum || Kind == MinMaxKind::maximumnum))
    Known.knownNot(fcNan);

  विकल्प (Kind) {
  पक्ष MinMaxKind::maxnum: {
    सचे (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/मिच्छा));
    सचे (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/मिच्छा));
    भङ्ग;
  }
  पक्ष MinMaxKind::minnum: {
    सचे (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/मिच्छा));
    सचे (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/मिच्छा));
    भङ्ग;
  }
  पक्ष MinMaxKind::maximumnum: {
    सचे (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/सच्च));
    सचे (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/सच्च));
    भङ्ग;
  }
  पक्ष MinMaxKind::minimumnum: {
    सचे (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/सच्च));
    सचे (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/सच्च));
    भङ्ग;
  }
  पक्ष MinMaxKind::maximum: {
    Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/सच्च) |
                   orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/सच्च));
    भङ्ग;
  }
  पक्ष MinMaxKind::minimum: {
    Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/सच्च) |
                   orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/सच्च));
    भङ्ग;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      KnownLHS.getKnownFPClasses() | KnownRHS.getKnownFPClasses() | fcNan;
  Known.knownNot(~PossibleClasses);

  पटिनिवत्त applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::canonicalize(ध्रुव KnownFPClass &KnownSrc,
                                        DenormalMode DenormMode) {
  KnownFPClass Known;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  सचे (KnownSrc.isKnownNeverNaN())
    Known.knownNot(fcNan);
  अञ्ञथा
    Known.knownNot(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  सचे (DenormMode == DenormalMode::getIEEE()) {
    सचे (KnownSrc.isKnownNever(fcPosZero))
      Known.knownNot(fcPosZero);
    सचे (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);
    पटिनिवत्त Known;
  }

  सचे (DenormMode.inputsAreZero() || DenormMode.outputsAreZero())
    Known.knownNot(fcSubnormal);

  सचे (DenormMode == DenormalMode::getPreserveSign()) {
    सचे (KnownSrc.isKnownNever(fcPosZero | fcPosSubnormal))
      Known.knownNot(fcPosZero);
    सचे (KnownSrc.isKnownNever(fcNegZero | fcNegSubnormal))
      Known.knownNot(fcNegZero);
    पटिनिवत्त Known;
  }

  सचे (DenormMode.निवेशः == DenormalMode::PositiveZero ||
      (DenormMode.Output == DenormalMode::PositiveZero &&
       DenormMode.निवेशः == DenormalMode::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    सचे (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    सचे (KnownSrc.isKnownNever(fcPosZero | fcSubnormal))
      Known.knownNot(fcPosZero);
  }

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::bitcast(ध्रुव fltSemantics &FltSemantics,
                                   ध्रुव KnownBits &Bits) {
  प्रतिज्ञा(FltSemantics.sizeInBits == Bits.getBitWidth() &&
         "Bitcast operand has incorrect bit width");
  KnownFPClass Known;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  सचे (Bits.hasConflict())
    पटिनिवत्त Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](ध्रुव fltSemantics &Semantics) {
    विकल्प (APFloat::SemanticsToEnum(Semantics)) {
    पक्ष APFloatBase::S_IEEEhalf:
    पक्ष APFloatBase::S_BFloat:
    पक्ष APFloatBase::S_IEEEsingle:
    पक्ष APFloatBase::S_IEEEdouble:
    पक्ष APFloatBase::S_IEEEquad:
    पक्ष APFloatBase::S_x87DoubleExtended:
      पटिनिवत्त सच्च;
    स्वभाव:
      पटिनिवत्त मिच्छा;
    }
  };
  सचे (!IsSupported(FltSemantics))
    पटिनिवत्त Known;

  // Transfer information from the sign bit.
  सचे (Bits.isNonNegative())
    Known.signBitMustBeZero();
  अञ्ञथा सचे (Bits.isNegative())
    Known.signBitMustBeOne();

  विकल्प (APFloatBase::SemanticsToEnum(FltSemantics)) {
  पक्ष APFloatBase::S_IEEEhalf:
  पक्ष APFloatBase::S_BFloat:
  पक्ष APFloatBase::S_IEEEsingle:
  पक्ष APFloatBase::S_IEEEdouble:
  पक्ष APFloatBase::S_IEEEquad: {
    ध्रुव unsigned MantissaBits = FltSemantics.precision - 1;
    ध्रुव APInt ExponentMask = APInt::getBitsSet(
        FltSemantics.sizeInBits, MantissaBits, FltSemantics.sizeInBits - 1);
    ध्रुव APInt MantissaMask =
        APInt::get_नीचः_Bits_Set(FltSemantics.sizeInBits, MantissaBits);

    ध्रुव तार्किक ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    ध्रुव तार्किक ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    ध्रुव तार्किक ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    ध्रुव तार्किक ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();

    ध्रुव तार्किक MantissaKnownAllZeros =
        (Bits.Zero & MantissaMask) == MantissaMask;
    ध्रुव तार्किक MantissaKnownNotAllZeros = !(Bits.One & MantissaMask).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    सचे (ExponentKnownNotAllZeros)
      Known.knownNot(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    सचे (ExponentKnownNotAllOnes)
      Known.knownNot(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    सचे (ExponentKnownAllZeros || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // Zero and infinity require a mantissa with all zero bits.
    सचे (MantissaKnownNotAllZeros)
      Known.knownNot(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    सचे (MantissaKnownAllZeros)
      Known.knownNot(fcSubnormal | fcNan);

    ध्रुव तार्किक QuietBitKnownSet = Bits.One[MantissaBits - 1];
    ध्रुव तार्किक QuietBitKnownClear = Bits.Zero[MantissaBits - 1];

    सचे (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    अञ्ञथा सचे (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    भङ्ग;
  }
  पक्ष APFloatBase::S_x87DoubleExtended: {
    // Layout of the x87 floating point type:
    // [0:62] fraction
    // [63] explicit integer bit
    // [64:78] exponent
    // [79] sign bit
    // APFloat treats the x87 pseudo encodings as documented in
    // fltSemantics::hasExplicitIntegerBit.
    constexpr unsigned BitWidth = 80;
    constexpr unsigned FractionBits = 63;
    constexpr unsigned IntegerBitIndex = FractionBits;
    ध्रुव APInt FractionMask = APInt::get_नीचः_Bits_Set(BitWidth, FractionBits);
    ध्रुव APInt ExponentMask =
        APInt::getBitsSet(BitWidth, FractionBits + 1, BitWidth - 1);

    ध्रुव तार्किक ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    ध्रुव तार्किक ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    ध्रुव तार्किक ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    ध्रुव तार्किक ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();
    ध्रुव तार्किक FractionKnownAllZeros =
        (Bits.Zero & FractionMask) == FractionMask;
    ध्रुव तार्किक FractionKnownNotAllZeros = !(Bits.One & FractionMask).isZero();
    ध्रुव तार्किक IntegerBitKnownSet = Bits.One[IntegerBitIndex];
    ध्रुव तार्किक IntegerBitKnownClear = Bits.Zero[IntegerBitIndex];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    सचे (ExponentKnownNotAllZeros || IntegerBitKnownSet)
      Known.knownNot(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    सचे (FractionKnownNotAllZeros)
      Known.knownNot(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    सचे (ExponentKnownNotAllOnes || IntegerBitKnownClear ||
        FractionKnownNotAllZeros)
      Known.knownNot(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    सचे (IntegerBitKnownClear || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    सचे (FractionKnownAllZeros)
      Known.knownNot(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    सचे (ExponentKnownAllZeros ||
        (IntegerBitKnownSet &&
         (ExponentKnownNotAllOnes || FractionKnownAllZeros)))
      Known.knownNot(fcNan);

    ध्रुव तार्किक QuietBitKnownSet = Bits.One[FractionBits - 1];
    ध्रुव तार्किक QuietBitKnownClear = Bits.Zero[FractionBits - 1];

    सचे (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    अञ्ञथा सचे (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    भङ्ग;
  }
  स्वभाव:
    llvm_unreachable("unhandled supported type");
  }

  पटिनिवत्त Known;
}

KnownBits KnownFPClass::toKnownBits(ध्रुव fltSemantics &FltSemantics) ध्रुव {
  KnownBits Known(FltSemantics.sizeInBits);
  ध्रुव FPClassTest FPClasses = getKnownFPClasses();

  // Return unknown if poison.
  सचे (FPClasses == fcNone)
    पटिनिवत्त Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](ध्रुव fltSemantics &Semantics) {
    विकल्प (APFloat::SemanticsToEnum(Semantics)) {
    पक्ष APFloatBase::S_IEEEhalf:
    पक्ष APFloatBase::S_BFloat:
    पक्ष APFloatBase::S_IEEEsingle:
    पक्ष APFloatBase::S_IEEEdouble:
    पक्ष APFloatBase::S_IEEEquad:
    पक्ष APFloatBase::S_x87DoubleExtended:
      पटिनिवत्त सच्च;
    स्वभाव:
      पटिनिवत्त मिच्छा;
    }
  };
  सचे (!IsSupported(FltSemantics))
    पटिनिवत्त Known;

  विकल्प (APFloat::SemanticsToEnum(FltSemantics)) {
  पक्ष APFloatBase::S_IEEEhalf:
  पक्ष APFloatBase::S_BFloat:
  पक्ष APFloatBase::S_IEEEsingle:
  पक्ष APFloatBase::S_IEEEdouble:
  पक्ष APFloatBase::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    सचे (FPClasses & fcNormal)
      भङ्ग;

    Known.setAllConflict();

    ध्रुव unsigned BitWidth = FltSemantics.sizeInBits;
    ध्रुव unsigned MantissaBits = FltSemantics.precision - 1;
    ध्रुव unsigned ExponentBits = BitWidth - MantissaBits - 1;

    APInt MantissaMask = APInt::get_नीचः_Bits_Set(BitWidth, MantissaBits);
    APInt ExponentMask =
        APInt::getBitsSet(BitWidth, MantissaBits, MantissaBits + ExponentBits);

    ध्रुव unsigned QuietBitIndex = MantissaBits - 1;
    APInt PayloadMask = MantissaMask;
    PayloadMask.clearBit(QuietBitIndex);

    सचे (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // No individual payload bit is known.
      Known.Zero &= ~PayloadMask;
      Known.One &= ~PayloadMask;

      सचे (FPClasses & fcQNan)
        Known.Zero.clearBit(QuietBitIndex);
      सचे (FPClasses & fcSNan)
        Known.One.clearBit(QuietBitIndex);
    }
    सचे (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // Mantissa bits cannot be ones.
      Known.One &= ~MantissaMask;
    }
    सचे (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Unknown mantissa.
      Known.One &= ~MantissaMask;
      Known.Zero &= ~MantissaMask;
    }
    सचे (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Mantissa cannot be ones.
      Known.One &= ~MantissaMask;
    }

    भङ्ग;
  }
  पक्ष APFloatBase::S_x87DoubleExtended: {
    सचे (!isKnownNever(fcNormal | fcSubnormal | fcNan))
      भङ्ग;

    Known.setAllConflict();

    सचे (FPClasses & fcInf)
      Known = Known.intersectWith(KnownBits::makeConstant(
          APFloat::getInf(FltSemantics).bitcastToAPInt()));

    सचे (FPClasses & fcZero)
      Known = Known.intersectWith(
          KnownBits::makeConstant(APInt::getZero(FltSemantics.sizeInBits)));

    भङ्ग;
  }
  स्वभाव:
    llvm_unreachable("unhandled supported semantics");
  }

  Known.Zero.clearSignBit();
  Known.One.clearSignBit();
  सचे (std::optional<तार्किक> Sign = getSignBit()) {
    सचे (*Sign)
      Known.makeNegative();
    अञ्ञथा
      Known.makeNonNegative();
  }

  पटिनिवत्त Known;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
स्थिर KnownFPClass fadd_impl(ध्रुव KnownFPClass &KnownLHS,
                              ध्रुव KnownFPClass &KnownRHS, तार्किक IsFMA) {
  KnownFPClass Known;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  सचे (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNever(fcPosInf) || KnownRHS.isKnownNever(fcNegInf)) &&
      (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosInf)))
    Known.knownNot(fcNan);

  सचे (KnownLHS.cannotBeOrderedLessThanZero() &&
      KnownRHS.cannotBeOrderedLessThanZero()) {
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    सचे (KnownLHS.isKnownNever(fcZero | fcPosSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcPosSubnormal))
      Known.knownNot(fcZero | fcPosSubnormal);
  }

  सचे (KnownLHS.cannotBeOrderedGreaterThanZero() &&
      KnownRHS.cannotBeOrderedGreaterThanZero()) {
    Known.knownNot(KnownFPClass::OrderedGreaterThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    सचे (KnownLHS.isKnownNever(fcZero | fcNegSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcNegSubnormal))
      Known.knownNot(fcZero | fcNegSubnormal);
  }

  सचे (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    सचे (KnownLHS.isKnownNever(fcNegZero) || KnownRHS.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    Known.propagateNonSNaN(KnownLHS, KnownRHS);
  }

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::fadd(ध्रुव KnownFPClass &KnownLHS_,
                                ध्रुव KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_निवेशः_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_निवेशः_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/मिच्छा);

  पटिनिवत्त applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fadd_self(ध्रुव KnownFPClass &KnownSrc_,
                                     DenormalMode Mode) {
  KnownFPClass KnownSrc = apply_निवेशः_Denormal_Mode(KnownSrc_, Mode);

  KnownFPClass Known = fadd_impl(KnownSrc, KnownSrc, /*IsFMA=*/मिच्छा);

  सचे (KnownSrc.isKnownNever(fcPosSubnormal))
    Known.knownNot(fcPosSubnormal);
  सचे (KnownSrc.isKnownNever(fcNegSubnormal))
    Known.knownNot(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  सचे (KnownSrc.isKnownNever(fcPosZero))
    Known.knownNot(fcPosZero);

  पटिनिवत्त applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fsub(ध्रुव KnownFPClass &KnownLHS_,
                                ध्रुव KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_निवेशः_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_निवेशः_Denormal_Mode(KnownRHS_, Mode);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  KnownRHS.fneg();

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/मिच्छा);

  पटिनिवत्त applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fmul(ध्रुव KnownFPClass &KnownLHS_,
                                ध्रुव KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_निवेशः_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_निवेशः_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  सचे ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  सचे ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Inf * Y => Inf or NaN
  सचे (KnownLHS.isKnownAlways(fcInf | fcNan) ||
      KnownRHS.isKnownAlways(fcInf | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  सचे (KnownRHS.isKnownAlways(fcZero | fcNan) ||
      KnownLHS.isKnownAlways(fcZero | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  सचे (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownRHS.isKnownNeverInfinity() || KnownLHS.isKnownNeverZero()) &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverZero()))
    Known.knownNot(fcNan);

  पटिनिवत्त applyOutputDenormalMode(Known, Mode);
}

// TODO: This generalizes to known ranges
KnownFPClass KnownFPClass::fmul(ध्रुव KnownFPClass &KnownLHS,
                                ध्रुव APFloat &CRHS, DenormalMode Mode) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  ध्रुव fltSemantics &Flt = CRHS.getSemantics();
  unsigned Precision = APFloat::semanticsPrecision(Flt);
  ध्रुव पूर्णाङ्क MantissaBits = Precision - 1;

  पूर्णाङ्क MinKnownExponent = ilogb(CRHS);
  तार्किक CannotBeSubnormal = (MinKnownExponent >= MantissaBits);

  KnownFPClass Known = KnownFPClass::fmul(KnownLHS, KnownFPClass(CRHS), Mode);
  सचे (CannotBeSubnormal)
    Known.knownNot(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  सचे (KnownLHS.isKnownNever(fcInf)) {
    सचे (MinKnownExponent < 0)
      Known.knownNot(fcInf);
    अञ्ञथा सचे (MinKnownExponent == 0 && CRHS.compare_Absolute_मानम्(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      Known.knownNot(fcInf);
  }

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::fdiv(ध्रुव KnownFPClass &KnownLHS_,
                                ध्रुव KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_निवेशः_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_निवेशः_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // Only 0/0, Inf/Inf produce NaN.
  सचे (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverInfinity()) &&
      (KnownLHS.isKnownNeverZero() || KnownRHS.isKnownNeverZero())) {
    Known.knownNot(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  सचे ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  सचे ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Normal and subnormal results require two non-zero finite operands.
  सचे ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)))
    Known.knownNot(fcNegNormal | fcNegSubnormal);
  सचे ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosNormal | fcPosSubnormal);

  // 0 / X => 0 or NaN
  सचे (KnownLHS.isKnownAlways(fcZero))
    Known.knownNot(fcSubnormal | fcNormal | fcInf);

  // X / 0 => NaN or Inf
  सचे (KnownRHS.isKnownAlways(fcZero))
    Known.knownNot(fcFinite);

  पटिनिवत्त applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fdiv_self(ध्रुव KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X / X is always exactly 1.0 or a NaN.
  KnownFPClass Known(fcNan | fcPosNormal);

  Known.propagateNonSNaN(KnownSrc);

  सचे (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::frem(ध्रुव KnownFPClass &KnownLHS,
                                ध्रुव KnownFPClass &KnownRHS,
                                DenormalMode Mode) {
  KnownFPClass Known;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  सचे (KnownLHS.isKnownNeverInfOrNaN() && KnownRHS.isKnownNeverNaN() &&
      KnownRHS.isKnownNeverLogicalZero(Mode)) {
    Known.knownNot(fcNan);
  }

  // The sign for frem is the same as the first operand.
  सचे (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal | fcPosSubnormal);
  सचे (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  सचे (KnownLHS.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  सचे (KnownLHS.isKnownNever(fcPosFinite) &&
      KnownLHS.isKnownNeverLogicalPosZero(Mode) &&
      (!Mode.outputsMayBePositiveZero() ||
       KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosZero);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::frem_self(ध्रुव KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X % X is always exactly [+-]0.0 or a NaN.
  KnownFPClass Known(fcNan | fcZero);

  Known.propagateNonSNaN(KnownSrc);

  सचे (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  सचे (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosZero);
  सचे (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegZero);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::fma(ध्रुव KnownFPClass &KnownLHS,
                               ध्रुव KnownFPClass &KnownRHS,
                               ध्रुव KnownFPClass &KnownAddend,
                               DenormalMode Mode) {
  KnownFPClass Mul = fmul(KnownLHS, KnownRHS, Mode);

  // FMA differs from the base fmul + fadd handling only in the treatment of -0
  // results.
  //
  // If the multiply is a -0 due to rounding, the final -0 + 0 will be -0,
  // unlike for a separate fadd.
  KnownFPClass Known = fadd_impl(Mul, KnownAddend, /*IsFMA=*/सच्च);

  // propagateNonSNaN for 3 arguments.
  सचे (KnownLHS.isKnownNever(fcSNan) && KnownRHS.isKnownNever(fcSNan) &&
      KnownAddend.isKnownNever(fcSNan))
    Known.knownNot(fcSNan);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::fmuladd(ध्रुव KnownFPClass &KnownLHS,
                                   ध्रुव KnownFPClass &KnownRHS,
                                   ध्रुव KnownFPClass &KnownAddend,
                                   DenormalMode Mode) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  KnownFPClass FMA = fma(KnownLHS, KnownRHS, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(fmul(KnownLHS, KnownRHS, Mode), KnownAddend, Mode);
  पटिनिवत्त FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::fma_square(ध्रुव KnownFPClass &KnownSquared,
                                      ध्रुव KnownFPClass &KnownAddend,
                                      DenormalMode Mode) {
  KnownFPClass Squared = square(KnownSquared, Mode);
  KnownFPClass Known = fadd_impl(Squared, KnownAddend, /*IsFMA=*/सच्च);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  सचे (KnownAddend.isKnownNever(fcNegInf | fcNan) && Squared.isKnownNever(fcNan))
    Known.knownNot(fcNan);

  Known.propagateNonSNaN(KnownSquared, KnownAddend);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::fmuladd_square(ध्रुव KnownFPClass &KnownSquared,
                                          ध्रुव KnownFPClass &KnownAddend,
                                          DenormalMode Mode) {
  KnownFPClass FMA = fma_square(KnownSquared, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(square(KnownSquared, Mode), KnownAddend, Mode);
  पटिनिवत्त FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::exp(ध्रुव KnownFPClass &KnownSrc) {
  KnownFPClass Known;
  Known.knownNot(fcNegative);

  Known.propagateNonNaN(KnownSrc);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  सचे (KnownSrc.isKnownNever(fcNegNormal | fcNegInf))
    Known.knownNot(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  सचे (KnownSrc.isKnownNever(fcNegNormal))
    Known.knownNot(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  सचे (KnownSrc.isKnownNever(fcPosNormal | fcPosInf))
    Known.knownNot(fcPosInf);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::log(ध्रुव KnownFPClass &KnownSrc, DenormalMode Mode,
                               तार्किक IsMultiUnitFPType) {
  KnownFPClass Known;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  सचे (!IsMultiUnitFPType)
    Known.knownNot(fcNegZero | fcSubnormal);

  Known.propagateNonSNaN(KnownSrc);

  सचे (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  सचे (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  सचे (KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNegInf);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::sqrt(ध्रुव KnownFPClass &KnownSrc,
                                DenormalMode Mode) {
  KnownFPClass Known;
  Known.knownNot(fcPosSubnormal);

  सचे (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  Known.propagateNonSNaN(KnownSrc);

  // Any negative value besides -0 returns a nan.
  सचे (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  Known.knownNot(fcNegInf | fcNegSubnormal | fcNegNormal);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  सचे (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::sin(ध्रुव KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownSrc);

  // Return NaN on infinite inputs.
  Known.knownNot(fcInf);
  सचे (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::cos(ध्रुव KnownFPClass &KnownSrc) {
  पटिनिवत्त sin(KnownSrc);
}

KnownFPClass KnownFPClass::tan(ध्रुव KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. tan(+-Inf) is NaN.
  सचे (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::sinh(ध्रुव KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  सचे (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  सचे (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal | fcPosInf);

  Known.propagateNonNaN(KnownSrc);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::cosh(ध्रुव KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  Known.knownNot(fcNegative | fcZero | fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::tanh(ध्रुव KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tanh is bounded to (-1, 1), never Inf.
  Known.knownNot(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  सचे (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf)) {
    Known.knownNot(fcNegSubnormal | fcNegNormal);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    सचे (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  सचे (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  Known.propagateNonNaN(KnownSrc);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::asin(ध्रुव KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // asin is sign-preserving for finite arguments.
  सचे (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegFinite);

  सचे (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::acos(ध्रुव KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // acos(x) is bounded to [0, pi] for -1 <= x <= 1, and is never negative,
  // infinite, or subnormal. The smallest non-zero value occurs when x is
  // close to 1.0, where acos(x) can be approximated by sqrt(2 * (1 - x)).
  // Since sqrt cannot produce a subnormal result, we can conclude that
  // acos(x) will also never produce a subnormal result.
  Known.knownNot(fcNegative | fcInf | fcSubnormal);

  // acos(x) == +0.0 iff x == +1.0
  सचे (KnownSrc.isKnownNever(fcPosNormal))
    Known.knownNot(fcZero);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::atan(ध्रुव KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  Known.knownNot(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  सचे (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf))
    Known.knownNot(fcNegSubnormal | fcNegNormal);

  सचे (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  सचे (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal | fcNegZero))
    Known.knownNot(fcNegZero);

  Known.propagateNonNaN(KnownSrc);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::atan2(ध्रुव KnownFPClass &KnownY_,
                                 ध्रुव KnownFPClass &KnownX_,
                                 DenormalMode Mode) {
  KnownFPClass KnownY = apply_निवेशः_Denormal_Mode(KnownY_, Mode);
  KnownFPClass KnownX = apply_निवेशः_Denormal_Mode(KnownX_, Mode);
  KnownFPClass Known;

  // Even though these deductions are correct, we are ignoring the following
  // potentially erroneous cases:
  //   * atan2(y, inf) is not subnormal
  //   * atan2(inf, x) is not zero or subnormal

  // atan2 result is in (-pi, pi], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonNaN(KnownY, KnownX);

  // If x <= -0.0, then |atan2(y, x)| >= pi/2
  सचे (KnownX.isKnownNever(fcPositive))
    Known.knownNot(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  सचे (KnownY.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  सचे (KnownY.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);

  पटिनिवत्त applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fpext(ध्रुव KnownFPClass &KnownSrc,
                                 ध्रुव fltSemantics &DstTy,
                                 ध्रुव fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  KnownFPClass Known = KnownSrc;

  // All subnormal inputs should be in the normal range in the result type.
  सचे (APFloat::isRepresentableAsNormalIn(SrcTy, DstTy)) {
    सचे (Known.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosNormal);
    सचे (Known.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegNormal);
    Known.knownNot(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  सचे (!Known.isKnownNeverNaN())
    Known.setSignBit(std::nullopt);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::fptrunc(ध्रुव KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  सचे (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

  Known.propagateNonNaN(KnownSrc);

  // Infinity needs a range check.
  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::roundToIntegral(ध्रुव KnownFPClass &KnownSrc,
                                           तार्किक MayBeMultiUnitFPType,
                                           DenormalMode Mode) {
  KnownFPClass Known;

  // Integer results cannot be subnormal.
  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  सचे (!MayBeMultiUnitFPType) {
    सचे (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    सचे (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  }

  सचे (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  सचे (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  // Negative round ups towards zero produce negative zero.
  सचे (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // Negative subnormals may flush to positive zero.
  सचे (KnownSrc.isKnownNever(fcPosFinite) &&
      (KnownSrc.isKnownNever(fcNegSubnormal) ||
       !Mode.inputsMayBePositiveZero()))
    Known.knownNot(fcPosZero);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::trunc(ध्रुव KnownFPClass &KnownSrc,
                                 DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc,
                                       /*MayBeMultiUnitFPType=*/सच्च, Mode);

  सचे (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);
  सचे (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::floor(ध्रुव KnownFPClass &KnownSrc,
                                 तार्किक IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  सचे (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::ceil(ध्रुव KnownFPClass &KnownSrc,
                                तार्किक IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  सचे (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::frexp_mant(ध्रुव KnownFPClass &KnownSrc,
                                      DenormalMode Mode) {
  KnownFPClass Known;

  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  सचे (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  सचे (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  सचे (KnownSrc.isKnownNeverLogicalPosZero(Mode))
    Known.knownNot(fcPosZero);

  सचे (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  सचे (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  सचे (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::ldexp(ध्रुव KnownFPClass &KnownSrc_,
                                 ध्रुव APInt &ConstantRangeExpMin,
                                 ध्रुव APInt &ConstantRangeExpMax,
                                 ध्रुव fltSemantics &Flt, DenormalMode Mode) {
  KnownFPClass Known;

  KnownFPClass KnownSrc = apply_निवेशः_Denormal_Mode(KnownSrc_, Mode);

  सचे (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcNan);
    Known.propagateNonNaN(KnownSrc);
    पटिनिवत्त applyOutputDenormalMode(Known, Mode);
  }

  Known.propagateNonNaN(KnownSrc);

  // Sign is preserved, but underflows may produce zeroes.
  सचे (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);
  अञ्ञथा सचे (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(OrderedLessThanZeroMask);

  सचे (KnownSrc.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);
  अञ्ञथा सचे (KnownSrc.cannotBeOrderedGreaterThanZero())
    Known.knownNot(OrderedGreaterThanZeroMask);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  सचे (&Flt != &APFloat::PPCDoubleDouble()) {
    unsigned Precision = APFloat::semanticsPrecision(Flt);
    ध्रुव पूर्णाङ्क MantissaBits = Precision - 1;
    सचे (ConstantRangeExpMin.sge(MantissaBits))
      Known.knownNot(fcSubnormal);
  }

  सचे (ConstantRangeExpMax.isNonPositive()) {
    // If we know the power is <= 0, can't introduce inf
    सचे (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    सचे (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  } अञ्ञथा सचे (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    सचे (KnownSrc.isKnownNeverPosSubnormal())
      Known.knownNot(fcPosSubnormal);
    सचे (KnownSrc.isKnownNeverNegSubnormal())
      Known.knownNot(fcNegSubnormal);
    सचे (KnownSrc.isKnownNeverPosZero())
      Known.knownNot(fcPosZero);
    सचे (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  पटिनिवत्त applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::ldexp(ध्रुव KnownFPClass &KnownSrc,
                                 ध्रुव KnownBits &ExpBits,
                                 ध्रुव fltSemantics &Flt, DenormalMode Mode) {
  पटिनिवत्त ldexp(KnownSrc, ExpBits.get_Signed_Min_मानम्(),
               ExpBits.get_Signed_Max_मानम्(), Flt, Mode);
}

KnownFPClass KnownFPClass::pow(ध्रुव KnownFPClass &KnownLHS,
                               ध्रुव KnownFPClass &KnownRHS) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  सचे (KnownLHS.isKnownNever(fcNan | fcNegNormal | fcNegSubnormal) &&
      KnownRHS.isKnownNeverNaN())
    Known.knownNot(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  सचे (KnownLHS.isKnownNever(fcNegative)) {
    Known.knownNot(fcNegative);
  } अञ्ञथा सचे (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    सचे ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcPosNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcNegNormal)))
      Known.knownNot(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    सचे ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcNegNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosNormal)))
      Known.knownNot(fcNegInf);
  }

  पटिनिवत्त Known;
}

KnownFPClass KnownFPClass::powi(ध्रुव KnownFPClass &KnownSrc,
                                ध्रुव KnownBits &ExponentKnownBits) {
  KnownFPClass Known;
  Known.propagateNonNaN(KnownSrc);

  सचे (ExponentKnownBits.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    सचे (KnownSrc.isKnownNever(fcSNan)) {
      Known.knownNot(~fcPosNormal);
      पटिनिवत्त Known;
    }

    Known.knownNot(~(fcPosNormal | fcNan));
    पटिनिवत्त Known;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  सचे (KnownSrc.isKnownNever(fcNegative) || ExponentKnownBits.isEven()) {
    Known.knownNot(fcNegative);
  } अञ्ञथा सचे (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    ध्रुव तार्किक IsKnownNeverOddPositive = ExponentKnownBits.isNegative();
    ध्रुव तार्किक IsKnownNeverOddNegative = ExponentKnownBits.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    सचे ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddPositive) &&
        (KnownSrc.isKnownNever(fcNegInf) || IsKnownNeverOddNegative))
      Known.knownNot(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    सचे ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddNegative) &&
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
    APInt MinExp = ExponentKnownBits.get_Signed_Min_मानम्();
    APInt MaxExp = ExponentKnownBits.get_Signed_Max_मानम्();

    // powi(inf, exp), exp > 0
    तार्किक MayInfSrc =
        !KnownSrc.isKnownNever(fcInf) && MaxExp.isStrictlyPositive();

    // powi(+/-0, exp), exp < 0
    तार्किक MayDivByZero = !KnownSrc.isKnownNever(fcZero) && MinExp.isNegative();

    // powi(finite, exp), |exp| > 1
    तार्किक MayFinite = !KnownSrc.isKnownNever(fcNormal | fcSubnormal);
    तार्किक MayAbsExpGT1 = MinExp.slt(-1) || MaxExp.sgt(1);
    तार्किक MayFiniteOverflow = MayFinite && MayAbsExpGT1;

    // powi(subnormal, -1)
    तार्किक MayBeNegOne = ExponentKnownBits.Zero.isZero();
    तार्किक MaySubnormInv = !KnownSrc.isKnownNever(fcSubnormal) && MayBeNegOne;

    सचे (!MayInfSrc && !MayDivByZero && !MayFiniteOverflow && !MaySubnormInv)
      Known.knownNot(fcInf);
  }

  पटिनिवत्त Known;
}
