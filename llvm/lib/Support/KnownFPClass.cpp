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

#समावेश "llvm/Support/KnownFPClass.h"
#समावेश "llvm/ADT/APFloat.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश "llvm/Support/KnownBits.h"

using namespace llvm;

KnownFPClass::KnownFPClass(स्थिर APFloat &C)
    : Known_FPClasses_मूल्य(C.classify()) {
  setSignBit(C.isNegative());
}

KnownFPClass KnownFPClass::apply_निवेश_Denormal_Mode(स्थिर KnownFPClass &KnownSrc,
                                                  DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  चयन (Mode.निवेश) {
  हाल DenormalMode::IEEE:
    वापस Known;
  हाल DenormalMode::PreserveSign:
    अगर (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    अगर (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    वापस Known;
  हाल DenormalMode::PositiveZero:
    अगर (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      अगर (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    वापस Known;
  शेष:
    अगर (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      अगर (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    अगर (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    वापस Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

KnownFPClass KnownFPClass::applyOutputDenormalMode(स्थिर KnownFPClass &KnownSrc,
                                                   DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  चयन (Mode.Output) {
  हाल DenormalMode::IEEE:
    वापस Known;
  हाल DenormalMode::PreserveSign:
    अगर (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    अगर (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    वापस Known;
  हाल DenormalMode::PositiveZero:
    अगर (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      अगर (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    वापस Known;
  शेष:
    अगर (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      अगर (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    अगर (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    वापस Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

bool KnownFPClass::isKnownNeverLogicalZero(DenormalMode Mode) स्थिर {
  वापस apply_निवेश_Denormal_Mode(*यह, Mode).isKnownNeverZero();
}

bool KnownFPClass::isKnownNeverLogicalNegZero(DenormalMode Mode) स्थिर {
  वापस apply_निवेश_Denormal_Mode(*यह, Mode).isKnownNeverNegZero();
}

bool KnownFPClass::isKnownNeverLogicalPosZero(DenormalMode Mode) स्थिर {
  वापस apply_निवेश_Denormal_Mode(*यह, Mode).isKnownNeverPosZero();
}

KnownFPClass KnownFPClass::minMaxLike(स्थिर KnownFPClass &KnownLHS_,
                                      स्थिर KnownFPClass &KnownRHS_,
                                      MinMaxKind Kind, DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_निवेश_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_निवेश_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonNaN(KnownLHS, KnownRHS);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  अगर ((KnownLHS.isKnownNeverNaN() || KnownRHS.isKnownNeverNaN()) &&
      (Kind == MinMaxKind::minnum || Kind == MinMaxKind::maxnum ||
       Kind == MinMaxKind::minimumnum || Kind == MinMaxKind::maximumnum))
    Known.knownNot(fcNan);

  चयन (Kind) {
  हाल MinMaxKind::maxnum: {
    अगर (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/false));
    अगर (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/false));
    अवरोध;
  }
  हाल MinMaxKind::minnum: {
    अगर (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/false));
    अगर (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/false));
    अवरोध;
  }
  हाल MinMaxKind::maximumnum: {
    अगर (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/true));
    अगर (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/true));
    अवरोध;
  }
  हाल MinMaxKind::minimumnum: {
    अगर (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/true));
    अगर (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/true));
    अवरोध;
  }
  हाल MinMaxKind::maximum: {
    Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/true) |
                   orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/true));
    अवरोध;
  }
  हाल MinMaxKind::minimum: {
    Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/true) |
                   orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/true));
    अवरोध;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      KnownLHS.getKnownFPClasses() | KnownRHS.getKnownFPClasses() | fcNan;
  Known.knownNot(~PossibleClasses);

  वापस applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::canonicalize(स्थिर KnownFPClass &KnownSrc,
                                        DenormalMode DenormMode) {
  KnownFPClass Known;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  अगर (KnownSrc.isKnownNeverNaN())
    Known.knownNot(fcNan);
  अन्यथा
    Known.knownNot(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  अगर (DenormMode == DenormalMode::getIEEE()) {
    अगर (KnownSrc.isKnownNever(fcPosZero))
      Known.knownNot(fcPosZero);
    अगर (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);
    वापस Known;
  }

  अगर (DenormMode.inputsAreZero() || DenormMode.outputsAreZero())
    Known.knownNot(fcSubnormal);

  अगर (DenormMode == DenormalMode::getPreserveSign()) {
    अगर (KnownSrc.isKnownNever(fcPosZero | fcPosSubnormal))
      Known.knownNot(fcPosZero);
    अगर (KnownSrc.isKnownNever(fcNegZero | fcNegSubnormal))
      Known.knownNot(fcNegZero);
    वापस Known;
  }

  अगर (DenormMode.निवेश == DenormalMode::PositiveZero ||
      (DenormMode.Output == DenormalMode::PositiveZero &&
       DenormMode.निवेश == DenormalMode::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    अगर (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    अगर (KnownSrc.isKnownNever(fcPosZero | fcSubnormal))
      Known.knownNot(fcPosZero);
  }

  वापस Known;
}

KnownFPClass KnownFPClass::bitcast(स्थिर fltSemantics &FltSemantics,
                                   स्थिर KnownBits &Bits) {
  निश्चित(FltSemantics.आकार_In_Bits == Bits.getBitWidth() &&
         "Bitcast operand has incorrect bit width");
  KnownFPClass Known;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  अगर (Bits.hasConflict())
    वापस Known;

  // Return unknown for types we have not validated.
  स्वतः IsSupported = [](स्थिर fltSemantics &Semantics) {
    चयन (APFloat::SemanticsToEnum(Semantics)) {
    हाल APFloatBase::S_IEEEhalf:
    हाल APFloatBase::S_BFloat:
    हाल APFloatBase::S_IEEEsingle:
    हाल APFloatBase::S_IEEEdouble:
    हाल APFloatBase::S_IEEEquad:
    हाल APFloatBase::S_x87DoubleExtended:
      वापस true;
    शेष:
      वापस false;
    }
  };
  अगर (!IsSupported(FltSemantics))
    वापस Known;

  // Transfer information from the sign bit.
  अगर (Bits.isNonNegative())
    Known.signBitMustBeZero();
  अन्यथा अगर (Bits.isNegative())
    Known.signBitMustBeOne();

  चयन (APFloatBase::SemanticsToEnum(FltSemantics)) {
  हाल APFloatBase::S_IEEEhalf:
  हाल APFloatBase::S_BFloat:
  हाल APFloatBase::S_IEEEsingle:
  हाल APFloatBase::S_IEEEdouble:
  हाल APFloatBase::S_IEEEquad: {
    स्थिर अचिन्हित MantissaBits = FltSemantics.precision - 1;
    स्थिर APInt ExponentMask = APInt::getBitsSet(
        FltSemantics.आकार_In_Bits, MantissaBits, FltSemantics.आकार_In_Bits - 1);
    स्थिर APInt MantissaMask =
        APInt::get_निचला_Bits_Set(FltSemantics.आकार_In_Bits, MantissaBits);

    स्थिर bool ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    स्थिर bool ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    स्थिर bool ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    स्थिर bool ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();

    स्थिर bool MantissaKnownAllZeros =
        (Bits.Zero & MantissaMask) == MantissaMask;
    स्थिर bool MantissaKnownNotAllZeros = !(Bits.One & MantissaMask).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    अगर (ExponentKnownNotAllZeros)
      Known.knownNot(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    अगर (ExponentKnownNotAllOnes)
      Known.knownNot(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    अगर (ExponentKnownAllZeros || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // Zero and infinity require a mantissa with all zero bits.
    अगर (MantissaKnownNotAllZeros)
      Known.knownNot(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    अगर (MantissaKnownAllZeros)
      Known.knownNot(fcSubnormal | fcNan);

    स्थिर bool QuietBitKnownSet = Bits.One[MantissaBits - 1];
    स्थिर bool QuietBitKnownClear = Bits.Zero[MantissaBits - 1];

    अगर (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    अन्यथा अगर (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    अवरोध;
  }
  हाल APFloatBase::S_x87DoubleExtended: {
    // Layout of the x87 floating point type:
    // [0:62] fraction
    // [63] explicit integer bit
    // [64:78] exponent
    // [79] sign bit
    // APFloat treats the x87 pseudo encodings as documented in
    // fltSemantics::hasExplicitIntegerBit.
    constexpr अचिन्हित BitWidth = 80;
    constexpr अचिन्हित FractionBits = 63;
    constexpr अचिन्हित Integer_Bit_सूचकांक = FractionBits;
    स्थिर APInt FractionMask = APInt::get_निचला_Bits_Set(BitWidth, FractionBits);
    स्थिर APInt ExponentMask =
        APInt::getBitsSet(BitWidth, FractionBits + 1, BitWidth - 1);

    स्थिर bool ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    स्थिर bool ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    स्थिर bool ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    स्थिर bool ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();
    स्थिर bool FractionKnownAllZeros =
        (Bits.Zero & FractionMask) == FractionMask;
    स्थिर bool FractionKnownNotAllZeros = !(Bits.One & FractionMask).isZero();
    स्थिर bool IntegerBitKnownSet = Bits.One[Integer_Bit_सूचकांक];
    स्थिर bool IntegerBitKnownClear = Bits.Zero[Integer_Bit_सूचकांक];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    अगर (ExponentKnownNotAllZeros || IntegerBitKnownSet)
      Known.knownNot(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    अगर (FractionKnownNotAllZeros)
      Known.knownNot(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    अगर (ExponentKnownNotAllOnes || IntegerBitKnownClear ||
        FractionKnownNotAllZeros)
      Known.knownNot(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    अगर (IntegerBitKnownClear || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    अगर (FractionKnownAllZeros)
      Known.knownNot(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    अगर (ExponentKnownAllZeros ||
        (IntegerBitKnownSet &&
         (ExponentKnownNotAllOnes || FractionKnownAllZeros)))
      Known.knownNot(fcNan);

    स्थिर bool QuietBitKnownSet = Bits.One[FractionBits - 1];
    स्थिर bool QuietBitKnownClear = Bits.Zero[FractionBits - 1];

    अगर (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    अन्यथा अगर (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    अवरोध;
  }
  शेष:
    llvm_unreachable("unhandled supported type");
  }

  वापस Known;
}

KnownBits KnownFPClass::toKnownBits(स्थिर fltSemantics &FltSemantics) स्थिर {
  KnownBits Known(FltSemantics.आकार_In_Bits);
  स्थिर FPClassTest FPClasses = getKnownFPClasses();

  // Return unknown if poison.
  अगर (FPClasses == fcNone)
    वापस Known;

  // Return unknown for types we have not validated.
  स्वतः IsSupported = [](स्थिर fltSemantics &Semantics) {
    चयन (APFloat::SemanticsToEnum(Semantics)) {
    हाल APFloatBase::S_IEEEhalf:
    हाल APFloatBase::S_BFloat:
    हाल APFloatBase::S_IEEEsingle:
    हाल APFloatBase::S_IEEEdouble:
    हाल APFloatBase::S_IEEEquad:
    हाल APFloatBase::S_x87DoubleExtended:
      वापस true;
    शेष:
      वापस false;
    }
  };
  अगर (!IsSupported(FltSemantics))
    वापस Known;

  चयन (APFloat::SemanticsToEnum(FltSemantics)) {
  हाल APFloatBase::S_IEEEhalf:
  हाल APFloatBase::S_BFloat:
  हाल APFloatBase::S_IEEEsingle:
  हाल APFloatBase::S_IEEEdouble:
  हाल APFloatBase::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    अगर (FPClasses & fcNormal)
      अवरोध;

    Known.setAllConflict();

    स्थिर अचिन्हित BitWidth = FltSemantics.आकार_In_Bits;
    स्थिर अचिन्हित MantissaBits = FltSemantics.precision - 1;
    स्थिर अचिन्हित ExponentBits = BitWidth - MantissaBits - 1;

    APInt MantissaMask = APInt::get_निचला_Bits_Set(BitWidth, MantissaBits);
    APInt ExponentMask =
        APInt::getBitsSet(BitWidth, MantissaBits, MantissaBits + ExponentBits);

    स्थिर अचिन्हित Quiet_Bit_सूचकांक = MantissaBits - 1;
    APInt PayloadMask = MantissaMask;
    PayloadMask.clearBit(Quiet_Bit_सूचकांक);

    अगर (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // No individual payload bit is known.
      Known.Zero &= ~PayloadMask;
      Known.One &= ~PayloadMask;

      अगर (FPClasses & fcQNan)
        Known.Zero.clearBit(Quiet_Bit_सूचकांक);
      अगर (FPClasses & fcSNan)
        Known.One.clearBit(Quiet_Bit_सूचकांक);
    }
    अगर (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // Mantissa bits cannot be ones.
      Known.One &= ~MantissaMask;
    }
    अगर (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Unknown mantissa.
      Known.One &= ~MantissaMask;
      Known.Zero &= ~MantissaMask;
    }
    अगर (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Mantissa cannot be ones.
      Known.One &= ~MantissaMask;
    }

    अवरोध;
  }
  हाल APFloatBase::S_x87DoubleExtended: {
    अगर (!isKnownNever(fcNormal | fcSubnormal | fcNan))
      अवरोध;

    Known.setAllConflict();

    अगर (FPClasses & fcInf)
      Known = Known.intersectWith(KnownBits::makeConstant(
          APFloat::getInf(FltSemantics).bitcastToAPInt()));

    अगर (FPClasses & fcZero)
      Known = Known.intersectWith(
          KnownBits::makeConstant(APInt::getZero(FltSemantics.आकार_In_Bits)));

    अवरोध;
  }
  शेष:
    llvm_unreachable("unhandled supported semantics");
  }

  Known.Zero.clearSignBit();
  Known.One.clearSignBit();
  अगर (std::optional<bool> Sign = getSignBit()) {
    अगर (*Sign)
      Known.makeNegative();
    अन्यथा
      Known.makeNonNegative();
  }

  वापस Known;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
अटल KnownFPClass fadd_impl(स्थिर KnownFPClass &KnownLHS,
                              स्थिर KnownFPClass &KnownRHS, bool IsFMA) {
  KnownFPClass Known;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  अगर (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNever(fcPosInf) || KnownRHS.isKnownNever(fcNegInf)) &&
      (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosInf)))
    Known.knownNot(fcNan);

  अगर (KnownLHS.cannotBeOrderedLessThanZero() &&
      KnownRHS.cannotBeOrderedLessThanZero()) {
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    अगर (KnownLHS.isKnownNever(fcZero | fcPosSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcPosSubnormal))
      Known.knownNot(fcZero | fcPosSubnormal);
  }

  अगर (KnownLHS.cannotBeOrderedGreaterThanZero() &&
      KnownRHS.cannotBeOrderedGreaterThanZero()) {
    Known.knownNot(KnownFPClass::OrderedGreaterThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    अगर (KnownLHS.isKnownNever(fcZero | fcNegSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcNegSubnormal))
      Known.knownNot(fcZero | fcNegSubnormal);
  }

  अगर (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    अगर (KnownLHS.isKnownNever(fcNegZero) || KnownRHS.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    Known.propagateNonSNaN(KnownLHS, KnownRHS);
  }

  वापस Known;
}

KnownFPClass KnownFPClass::fadd(स्थिर KnownFPClass &KnownLHS_,
                                स्थिर KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_निवेश_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_निवेश_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/false);

  वापस applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fadd_self(स्थिर KnownFPClass &KnownSrc_,
                                     DenormalMode Mode) {
  KnownFPClass KnownSrc = apply_निवेश_Denormal_Mode(KnownSrc_, Mode);

  KnownFPClass Known = fadd_impl(KnownSrc, KnownSrc, /*IsFMA=*/false);

  अगर (KnownSrc.isKnownNever(fcPosSubnormal))
    Known.knownNot(fcPosSubnormal);
  अगर (KnownSrc.isKnownNever(fcNegSubnormal))
    Known.knownNot(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  अगर (KnownSrc.isKnownNever(fcPosZero))
    Known.knownNot(fcPosZero);

  वापस applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fsub(स्थिर KnownFPClass &KnownLHS_,
                                स्थिर KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_निवेश_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_निवेश_Denormal_Mode(KnownRHS_, Mode);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  KnownRHS.fneg();

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/false);

  वापस applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fmul(स्थिर KnownFPClass &KnownLHS_,
                                स्थिर KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_निवेश_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_निवेश_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  अगर ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  अगर ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Inf * Y => Inf or NaN
  अगर (KnownLHS.isKnownAlways(fcInf | fcNan) ||
      KnownRHS.isKnownAlways(fcInf | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  अगर (KnownRHS.isKnownAlways(fcZero | fcNan) ||
      KnownLHS.isKnownAlways(fcZero | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  अगर (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownRHS.isKnownNeverInfinity() || KnownLHS.isKnownNeverZero()) &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverZero()))
    Known.knownNot(fcNan);

  वापस applyOutputDenormalMode(Known, Mode);
}

// TODO: This generalizes to known ranges
KnownFPClass KnownFPClass::fmul(स्थिर KnownFPClass &KnownLHS,
                                स्थिर APFloat &CRHS, DenormalMode Mode) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  स्थिर fltSemantics &Flt = CRHS.getSemantics();
  अचिन्हित Precision = APFloat::semanticsPrecision(Flt);
  स्थिर पूर्णांक MantissaBits = Precision - 1;

  पूर्णांक MinKnownExponent = ilogb(CRHS);
  bool CannotBeSubnormal = (MinKnownExponent >= MantissaBits);

  KnownFPClass Known = KnownFPClass::fmul(KnownLHS, KnownFPClass(CRHS), Mode);
  अगर (CannotBeSubnormal)
    Known.knownNot(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  अगर (KnownLHS.isKnownNever(fcInf)) {
    अगर (MinKnownExponent < 0)
      Known.knownNot(fcInf);
    अन्यथा अगर (MinKnownExponent == 0 && CRHS.compare_Absolute_मूल्य(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      Known.knownNot(fcInf);
  }

  वापस Known;
}

KnownFPClass KnownFPClass::fdiv(स्थिर KnownFPClass &KnownLHS_,
                                स्थिर KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_निवेश_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_निवेश_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // Only 0/0, Inf/Inf produce NaN.
  अगर (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverInfinity()) &&
      (KnownLHS.isKnownNeverZero() || KnownRHS.isKnownNeverZero())) {
    Known.knownNot(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  अगर ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  अगर ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Normal and subnormal results require two non-zero finite operands.
  अगर ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)))
    Known.knownNot(fcNegNormal | fcNegSubnormal);
  अगर ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosNormal | fcPosSubnormal);

  // 0 / X => 0 or NaN
  अगर (KnownLHS.isKnownAlways(fcZero))
    Known.knownNot(fcSubnormal | fcNormal | fcInf);

  // X / 0 => NaN or Inf
  अगर (KnownRHS.isKnownAlways(fcZero))
    Known.knownNot(fcFinite);

  वापस applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fdiv_self(स्थिर KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X / X is always exactly 1.0 or a NaN.
  KnownFPClass Known(fcNan | fcPosNormal);

  Known.propagateNonSNaN(KnownSrc);

  अगर (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  वापस Known;
}

KnownFPClass KnownFPClass::frem(स्थिर KnownFPClass &KnownLHS,
                                स्थिर KnownFPClass &KnownRHS,
                                DenormalMode Mode) {
  KnownFPClass Known;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  अगर (KnownLHS.isKnownNeverInfOrNaN() && KnownRHS.isKnownNeverNaN() &&
      KnownRHS.isKnownNeverLogicalZero(Mode)) {
    Known.knownNot(fcNan);
  }

  // The sign for frem is the same as the first operand.
  अगर (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal | fcPosSubnormal);
  अगर (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  अगर (KnownLHS.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  अगर (KnownLHS.isKnownNever(fcPosFinite) &&
      KnownLHS.isKnownNeverLogicalPosZero(Mode) &&
      (!Mode.outputsMayBePositiveZero() ||
       KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosZero);

  वापस Known;
}

KnownFPClass KnownFPClass::frem_self(स्थिर KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X % X is always exactly [+-]0.0 or a NaN.
  KnownFPClass Known(fcNan | fcZero);

  Known.propagateNonSNaN(KnownSrc);

  अगर (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  अगर (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosZero);
  अगर (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegZero);

  वापस Known;
}

KnownFPClass KnownFPClass::fma(स्थिर KnownFPClass &KnownLHS,
                               स्थिर KnownFPClass &KnownRHS,
                               स्थिर KnownFPClass &KnownAddend,
                               DenormalMode Mode) {
  KnownFPClass Mul = fmul(KnownLHS, KnownRHS, Mode);

  // FMA differs from the base fmul + fadd handling only in the treatment of -0
  // results.
  //
  // If the multiply is a -0 due to rounding, the final -0 + 0 will be -0,
  // unlike for a separate fadd.
  KnownFPClass Known = fadd_impl(Mul, KnownAddend, /*IsFMA=*/true);

  // propagateNonSNaN for 3 arguments.
  अगर (KnownLHS.isKnownNever(fcSNan) && KnownRHS.isKnownNever(fcSNan) &&
      KnownAddend.isKnownNever(fcSNan))
    Known.knownNot(fcSNan);

  वापस Known;
}

KnownFPClass KnownFPClass::fmuladd(स्थिर KnownFPClass &KnownLHS,
                                   स्थिर KnownFPClass &KnownRHS,
                                   स्थिर KnownFPClass &KnownAddend,
                                   DenormalMode Mode) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  KnownFPClass FMA = fma(KnownLHS, KnownRHS, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(fmul(KnownLHS, KnownRHS, Mode), KnownAddend, Mode);
  वापस FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::fma_square(स्थिर KnownFPClass &KnownSquared,
                                      स्थिर KnownFPClass &KnownAddend,
                                      DenormalMode Mode) {
  KnownFPClass Squared = square(KnownSquared, Mode);
  KnownFPClass Known = fadd_impl(Squared, KnownAddend, /*IsFMA=*/true);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  अगर (KnownAddend.isKnownNever(fcNegInf | fcNan) && Squared.isKnownNever(fcNan))
    Known.knownNot(fcNan);

  Known.propagateNonSNaN(KnownSquared, KnownAddend);

  वापस Known;
}

KnownFPClass KnownFPClass::fmuladd_square(स्थिर KnownFPClass &KnownSquared,
                                          स्थिर KnownFPClass &KnownAddend,
                                          DenormalMode Mode) {
  KnownFPClass FMA = fma_square(KnownSquared, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(square(KnownSquared, Mode), KnownAddend, Mode);
  वापस FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::exp(स्थिर KnownFPClass &KnownSrc) {
  KnownFPClass Known;
  Known.knownNot(fcNegative);

  Known.propagateNonNaN(KnownSrc);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  अगर (KnownSrc.isKnownNever(fcNegNormal | fcNegInf))
    Known.knownNot(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  अगर (KnownSrc.isKnownNever(fcNegNormal))
    Known.knownNot(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  अगर (KnownSrc.isKnownNever(fcPosNormal | fcPosInf))
    Known.knownNot(fcPosInf);

  वापस Known;
}

KnownFPClass KnownFPClass::log(स्थिर KnownFPClass &KnownSrc, DenormalMode Mode,
                               bool IsMultiUnitFPType) {
  KnownFPClass Known;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  अगर (!IsMultiUnitFPType)
    Known.knownNot(fcNegZero | fcSubnormal);

  Known.propagateNonSNaN(KnownSrc);

  अगर (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  अगर (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  अगर (KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNegInf);

  वापस Known;
}

KnownFPClass KnownFPClass::वर्ग_मूल(स्थिर KnownFPClass &KnownSrc,
                                DenormalMode Mode) {
  KnownFPClass Known;
  Known.knownNot(fcPosSubnormal);

  अगर (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  Known.propagateNonSNaN(KnownSrc);

  // Any negative value besides -0 returns a nan.
  अगर (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  Known.knownNot(fcNegInf | fcNegSubnormal | fcNegNormal);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  अगर (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  वापस Known;
}

KnownFPClass KnownFPClass::sin(स्थिर KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownSrc);

  // Return NaN on infinite inputs.
  Known.knownNot(fcInf);
  अगर (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  वापस Known;
}

KnownFPClass KnownFPClass::cos(स्थिर KnownFPClass &KnownSrc) {
  वापस sin(KnownSrc);
}

KnownFPClass KnownFPClass::tan(स्थिर KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. tan(+-Inf) is NaN.
  अगर (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  वापस Known;
}

KnownFPClass KnownFPClass::sinh(स्थिर KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  अगर (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  अगर (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal | fcPosInf);

  Known.propagateNonNaN(KnownSrc);

  वापस Known;
}

KnownFPClass KnownFPClass::cosh(स्थिर KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  Known.knownNot(fcNegative | fcZero | fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  वापस Known;
}

KnownFPClass KnownFPClass::tanh(स्थिर KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tanh is bounded to (-1, 1), never Inf.
  Known.knownNot(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  अगर (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf)) {
    Known.knownNot(fcNegSubnormal | fcNegNormal);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    अगर (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  अगर (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  Known.propagateNonNaN(KnownSrc);

  वापस Known;
}

KnownFPClass KnownFPClass::asin(स्थिर KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // asin is sign-preserving for finite arguments.
  अगर (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegFinite);

  अगर (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  वापस Known;
}

KnownFPClass KnownFPClass::acos(स्थिर KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // acos(x) is bounded to [0, pi] for -1 <= x <= 1, and is never negative,
  // infinite, or subnormal. The smallest non-zero value occurs when x is
  // close to 1.0, where acos(x) can be approximated by sqrt(2 * (1 - x)).
  // Since sqrt cannot produce a subnormal result, we can conclude that
  // acos(x) will also never produce a subnormal result.
  Known.knownNot(fcNegative | fcInf | fcSubnormal);

  // acos(x) == +0.0 iff x == +1.0
  अगर (KnownSrc.isKnownNever(fcPosNormal))
    Known.knownNot(fcZero);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  वापस Known;
}

KnownFPClass KnownFPClass::atan(स्थिर KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  Known.knownNot(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  अगर (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf))
    Known.knownNot(fcNegSubnormal | fcNegNormal);

  अगर (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  अगर (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal | fcNegZero))
    Known.knownNot(fcNegZero);

  Known.propagateNonNaN(KnownSrc);

  वापस Known;
}

KnownFPClass KnownFPClass::atan2(स्थिर KnownFPClass &KnownY_,
                                 स्थिर KnownFPClass &KnownX_,
                                 DenormalMode Mode) {
  KnownFPClass KnownY = apply_निवेश_Denormal_Mode(KnownY_, Mode);
  KnownFPClass KnownX = apply_निवेश_Denormal_Mode(KnownX_, Mode);
  KnownFPClass Known;

  // Even though these deductions are correct, we are ignoring the following
  // potentially erroneous cases:
  //   * atan2(y, inf) is not subnormal
  //   * atan2(inf, x) is not zero or subnormal

  // atan2 result is in (-pi, pi], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonNaN(KnownY, KnownX);

  // If x <= -0.0, then |atan2(y, x)| >= pi/2
  अगर (KnownX.isKnownNever(fcPositive))
    Known.knownNot(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  अगर (KnownY.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  अगर (KnownY.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);

  वापस applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fpext(स्थिर KnownFPClass &KnownSrc,
                                 स्थिर fltSemantics &DstTy,
                                 स्थिर fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  KnownFPClass Known = KnownSrc;

  // All subnormal inputs should be in the normal range in the result type.
  अगर (APFloat::isRepresentableAsNormalIn(SrcTy, DstTy)) {
    अगर (Known.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosNormal);
    अगर (Known.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegNormal);
    Known.knownNot(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  अगर (!Known.isKnownNeverNaN())
    Known.setSignBit(std::nullopt);

  वापस Known;
}

KnownFPClass KnownFPClass::fptrunc(स्थिर KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  अगर (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

  Known.propagateNonNaN(KnownSrc);

  // Infinity needs a range check.
  वापस Known;
}

KnownFPClass KnownFPClass::roundToIntegral(स्थिर KnownFPClass &KnownSrc,
                                           bool MayBeMultiUnitFPType,
                                           DenormalMode Mode) {
  KnownFPClass Known;

  // Integer results cannot be subnormal.
  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  अगर (!MayBeMultiUnitFPType) {
    अगर (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    अगर (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  }

  अगर (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  अगर (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  // Negative round ups towards zero produce negative zero.
  अगर (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // Negative subnormals may flush to positive zero.
  अगर (KnownSrc.isKnownNever(fcPosFinite) &&
      (KnownSrc.isKnownNever(fcNegSubnormal) ||
       !Mode.inputsMayBePositiveZero()))
    Known.knownNot(fcPosZero);

  वापस Known;
}

KnownFPClass KnownFPClass::छाँटो(स्थिर KnownFPClass &KnownSrc,
                                 DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc,
                                       /*MayBeMultiUnitFPType=*/true, Mode);

  अगर (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);
  अगर (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  वापस Known;
}

KnownFPClass KnownFPClass::न्यूनमान(स्थिर KnownFPClass &KnownSrc,
                                 bool IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  अगर (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  वापस Known;
}

KnownFPClass KnownFPClass::उच्चमान(स्थिर KnownFPClass &KnownSrc,
                                bool IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  अगर (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  वापस Known;
}

KnownFPClass KnownFPClass::frexp_mant(स्थिर KnownFPClass &KnownSrc,
                                      DenormalMode Mode) {
  KnownFPClass Known;

  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  अगर (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  अगर (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  अगर (KnownSrc.isKnownNeverLogicalPosZero(Mode))
    Known.knownNot(fcPosZero);

  अगर (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  अगर (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  अगर (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  वापस Known;
}

KnownFPClass KnownFPClass::ldexp(स्थिर KnownFPClass &KnownSrc_,
                                 स्थिर APInt &ConstantRangeExpMin,
                                 स्थिर APInt &ConstantRangeExpMax,
                                 स्थिर fltSemantics &Flt, DenormalMode Mode) {
  KnownFPClass Known;

  KnownFPClass KnownSrc = apply_निवेश_Denormal_Mode(KnownSrc_, Mode);

  अगर (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcNan);
    Known.propagateNonNaN(KnownSrc);
    वापस applyOutputDenormalMode(Known, Mode);
  }

  Known.propagateNonNaN(KnownSrc);

  // Sign is preserved, but underflows may produce zeroes.
  अगर (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);
  अन्यथा अगर (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(OrderedLessThanZeroMask);

  अगर (KnownSrc.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);
  अन्यथा अगर (KnownSrc.cannotBeOrderedGreaterThanZero())
    Known.knownNot(OrderedGreaterThanZeroMask);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  अगर (&Flt != &APFloat::PPCDoubleDouble()) {
    अचिन्हित Precision = APFloat::semanticsPrecision(Flt);
    स्थिर पूर्णांक MantissaBits = Precision - 1;
    अगर (ConstantRangeExpMin.sge(MantissaBits))
      Known.knownNot(fcSubnormal);
  }

  अगर (ConstantRangeExpMax.isNonPositive()) {
    // If we know the power is <= 0, can't introduce inf
    अगर (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    अगर (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  } अन्यथा अगर (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    अगर (KnownSrc.isKnownNeverPosSubnormal())
      Known.knownNot(fcPosSubnormal);
    अगर (KnownSrc.isKnownNeverNegSubnormal())
      Known.knownNot(fcNegSubnormal);
    अगर (KnownSrc.isKnownNeverPosZero())
      Known.knownNot(fcPosZero);
    अगर (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  वापस applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::ldexp(स्थिर KnownFPClass &KnownSrc,
                                 स्थिर KnownBits &ExpBits,
                                 स्थिर fltSemantics &Flt, DenormalMode Mode) {
  वापस ldexp(KnownSrc, ExpBits.get_Signed_Min_मूल्य(),
               ExpBits.get_Signed_Max_मूल्य(), Flt, Mode);
}

KnownFPClass KnownFPClass::घात(स्थिर KnownFPClass &KnownLHS,
                               स्थिर KnownFPClass &KnownRHS) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  अगर (KnownLHS.isKnownNever(fcNan | fcNegNormal | fcNegSubnormal) &&
      KnownRHS.isKnownNeverNaN())
    Known.knownNot(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  अगर (KnownLHS.isKnownNever(fcNegative)) {
    Known.knownNot(fcNegative);
  } अन्यथा अगर (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    अगर ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcPosNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcNegNormal)))
      Known.knownNot(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    अगर ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcNegNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosNormal)))
      Known.knownNot(fcNegInf);
  }

  वापस Known;
}

KnownFPClass KnownFPClass::powi(स्थिर KnownFPClass &KnownSrc,
                                स्थिर KnownBits &ExponentKnownBits) {
  KnownFPClass Known;
  Known.propagateNonNaN(KnownSrc);

  अगर (ExponentKnownBits.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    अगर (KnownSrc.isKnownNever(fcSNan)) {
      Known.knownNot(~fcPosNormal);
      वापस Known;
    }

    Known.knownNot(~(fcPosNormal | fcNan));
    वापस Known;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  अगर (KnownSrc.isKnownNever(fcNegative) || ExponentKnownBits.isEven()) {
    Known.knownNot(fcNegative);
  } अन्यथा अगर (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    स्थिर bool IsKnownNeverOddPositive = ExponentKnownBits.isNegative();
    स्थिर bool IsKnownNeverOddNegative = ExponentKnownBits.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    अगर ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddPositive) &&
        (KnownSrc.isKnownNever(fcNegInf) || IsKnownNeverOddNegative))
      Known.knownNot(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    अगर ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddNegative) &&
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
    APInt MinExp = ExponentKnownBits.get_Signed_Min_मूल्य();
    APInt MaxExp = ExponentKnownBits.get_Signed_Max_मूल्य();

    // powi(inf, exp), exp > 0
    bool MayInfSrc =
        !KnownSrc.isKnownNever(fcInf) && MaxExp.isStrictlyPositive();

    // powi(+/-0, exp), exp < 0
    bool MayDivByZero = !KnownSrc.isKnownNever(fcZero) && MinExp.isNegative();

    // powi(finite, exp), |exp| > 1
    bool MayFinite = !KnownSrc.isKnownNever(fcNormal | fcSubnormal);
    bool MayAbsExpGT1 = MinExp.slt(-1) || MaxExp.sgt(1);
    bool MayFiniteOverflow = MayFinite && MayAbsExpGT1;

    // powi(subnormal, -1)
    bool MayBeNegOne = ExponentKnownBits.Zero.isZero();
    bool MaySubnormInv = !KnownSrc.isKnownNever(fcSubnormal) && MayBeNegOne;

    अगर (!MayInfSrc && !MayDivByZero && !MayFiniteOverflow && !MaySubnormInv)
      Known.knownNot(fcInf);
  }

  वापस Known;
}
