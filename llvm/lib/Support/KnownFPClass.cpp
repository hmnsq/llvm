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

KnownFPClass::KnownFPClass(स्थिरांक APFloat &C)
    : Known_FPClasses_मूल्य(C.classify()) {
  setSignBit(C.isNegative());
}

KnownFPClass KnownFPClass::apply_आदान_Denormal_Mode(स्थिरांक KnownFPClass &KnownSrc,
                                                  DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  निवड (Mode.आदान) {
  स्थिती DenormalMode::IEEE:
    परत Known;
  स्थिती DenormalMode::PreserveSign:
    जर (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    जर (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    परत Known;
  स्थिती DenormalMode::PositiveZero:
    जर (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      जर (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    परत Known;
  पूर्वनिर्धारित:
    जर (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      जर (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    जर (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    परत Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

KnownFPClass KnownFPClass::applyOutputDenormalMode(स्थिरांक KnownFPClass &KnownSrc,
                                                   DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  निवड (Mode.Output) {
  स्थिती DenormalMode::IEEE:
    परत Known;
  स्थिती DenormalMode::PreserveSign:
    जर (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    जर (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    परत Known;
  स्थिती DenormalMode::PositiveZero:
    जर (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      जर (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    परत Known;
  पूर्वनिर्धारित:
    जर (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      जर (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    जर (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    परत Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

तार्किक KnownFPClass::isKnownNeverLogicalZero(DenormalMode Mode) स्थिरांक {
  परत apply_आदान_Denormal_Mode(*हे, Mode).isKnownNeverZero();
}

तार्किक KnownFPClass::isKnownNeverLogicalNegZero(DenormalMode Mode) स्थिरांक {
  परत apply_आदान_Denormal_Mode(*हे, Mode).isKnownNeverNegZero();
}

तार्किक KnownFPClass::isKnownNeverLogicalPosZero(DenormalMode Mode) स्थिरांक {
  परत apply_आदान_Denormal_Mode(*हे, Mode).isKnownNeverPosZero();
}

KnownFPClass KnownFPClass::minMaxLike(स्थिरांक KnownFPClass &KnownLHS_,
                                      स्थिरांक KnownFPClass &KnownRHS_,
                                      MinMaxKind Kind, DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_आदान_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_आदान_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonNaN(KnownLHS, KnownRHS);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  जर ((KnownLHS.isKnownNeverNaN() || KnownRHS.isKnownNeverNaN()) &&
      (Kind == MinMaxKind::minnum || Kind == MinMaxKind::maxnum ||
       Kind == MinMaxKind::minimumnum || Kind == MinMaxKind::maximumnum))
    Known.knownNot(fcNan);

  निवड (Kind) {
  स्थिती MinMaxKind::maxnum: {
    जर (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/असत्य));
    जर (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/असत्य));
    थांबा;
  }
  स्थिती MinMaxKind::minnum: {
    जर (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/असत्य));
    जर (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/असत्य));
    थांबा;
  }
  स्थिती MinMaxKind::maximumnum: {
    जर (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/सत्य));
    जर (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/सत्य));
    थांबा;
  }
  स्थिती MinMaxKind::minimumnum: {
    जर (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/सत्य));
    जर (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/सत्य));
    थांबा;
  }
  स्थिती MinMaxKind::maximum: {
    Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/सत्य) |
                   orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/सत्य));
    थांबा;
  }
  स्थिती MinMaxKind::minimum: {
    Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/सत्य) |
                   orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/सत्य));
    थांबा;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      KnownLHS.getKnownFPClasses() | KnownRHS.getKnownFPClasses() | fcNan;
  Known.knownNot(~PossibleClasses);

  परत applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::canonicalize(स्थिरांक KnownFPClass &KnownSrc,
                                        DenormalMode DenormMode) {
  KnownFPClass Known;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  जर (KnownSrc.isKnownNeverNaN())
    Known.knownNot(fcNan);
  अन्यथा
    Known.knownNot(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  जर (DenormMode == DenormalMode::getIEEE()) {
    जर (KnownSrc.isKnownNever(fcPosZero))
      Known.knownNot(fcPosZero);
    जर (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);
    परत Known;
  }

  जर (DenormMode.inputsAreZero() || DenormMode.outputsAreZero())
    Known.knownNot(fcSubnormal);

  जर (DenormMode == DenormalMode::getPreserveSign()) {
    जर (KnownSrc.isKnownNever(fcPosZero | fcPosSubnormal))
      Known.knownNot(fcPosZero);
    जर (KnownSrc.isKnownNever(fcNegZero | fcNegSubnormal))
      Known.knownNot(fcNegZero);
    परत Known;
  }

  जर (DenormMode.आदान == DenormalMode::PositiveZero ||
      (DenormMode.Output == DenormalMode::PositiveZero &&
       DenormMode.आदान == DenormalMode::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    जर (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    जर (KnownSrc.isKnownNever(fcPosZero | fcSubnormal))
      Known.knownNot(fcPosZero);
  }

  परत Known;
}

KnownFPClass KnownFPClass::bitcast(स्थिरांक fltSemantics &FltSemantics,
                                   स्थिरांक KnownBits &Bits) {
  प्रतिज्ञा(FltSemantics.sizeInBits == Bits.getBitWidth() &&
         "Bitcast operand has incorrect bit width");
  KnownFPClass Known;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  जर (Bits.hasConflict())
    परत Known;

  // Return unknown for types we have not validated.
  स्वतः IsSupported = [](स्थिरांक fltSemantics &Semantics) {
    निवड (APFloat::SemanticsToEnum(Semantics)) {
    स्थिती APFloatBase::S_IEEEhalf:
    स्थिती APFloatBase::S_BFloat:
    स्थिती APFloatBase::S_IEEEsingle:
    स्थिती APFloatBase::S_IEEEdouble:
    स्थिती APFloatBase::S_IEEEquad:
    स्थिती APFloatBase::S_x87DoubleExtended:
      परत सत्य;
    पूर्वनिर्धारित:
      परत असत्य;
    }
  };
  जर (!IsSupported(FltSemantics))
    परत Known;

  // Transfer information from the sign bit.
  जर (Bits.isNonNegative())
    Known.signBitMustBeZero();
  अन्यथा जर (Bits.isNegative())
    Known.signBitMustBeOne();

  निवड (APFloatBase::SemanticsToEnum(FltSemantics)) {
  स्थिती APFloatBase::S_IEEEhalf:
  स्थिती APFloatBase::S_BFloat:
  स्थिती APFloatBase::S_IEEEsingle:
  स्थिती APFloatBase::S_IEEEdouble:
  स्थिती APFloatBase::S_IEEEquad: {
    स्थिरांक अचिन्हित MantissaBits = FltSemantics.precision - 1;
    स्थिरांक APInt ExponentMask = APInt::getBitsSet(
        FltSemantics.sizeInBits, MantissaBits, FltSemantics.sizeInBits - 1);
    स्थिरांक APInt MantissaMask =
        APInt::get_खालचा_Bits_Set(FltSemantics.sizeInBits, MantissaBits);

    स्थिरांक तार्किक ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    स्थिरांक तार्किक ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    स्थिरांक तार्किक ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    स्थिरांक तार्किक ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();

    स्थिरांक तार्किक MantissaKnownAllZeros =
        (Bits.Zero & MantissaMask) == MantissaMask;
    स्थिरांक तार्किक MantissaKnownNotAllZeros = !(Bits.One & MantissaMask).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    जर (ExponentKnownNotAllZeros)
      Known.knownNot(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    जर (ExponentKnownNotAllOnes)
      Known.knownNot(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    जर (ExponentKnownAllZeros || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // Zero and infinity require a mantissa with all zero bits.
    जर (MantissaKnownNotAllZeros)
      Known.knownNot(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    जर (MantissaKnownAllZeros)
      Known.knownNot(fcSubnormal | fcNan);

    स्थिरांक तार्किक QuietBitKnownSet = Bits.One[MantissaBits - 1];
    स्थिरांक तार्किक QuietBitKnownClear = Bits.Zero[MantissaBits - 1];

    जर (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    अन्यथा जर (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    थांबा;
  }
  स्थिती APFloatBase::S_x87DoubleExtended: {
    // Layout of the x87 floating point type:
    // [0:62] fraction
    // [63] explicit integer bit
    // [64:78] exponent
    // [79] sign bit
    // APFloat treats the x87 pseudo encodings as documented in
    // fltSemantics::hasExplicitIntegerBit.
    constexpr अचिन्हित BitWidth = 80;
    constexpr अचिन्हित FractionBits = 63;
    constexpr अचिन्हित IntegerBitIndex = FractionBits;
    स्थिरांक APInt FractionMask = APInt::get_खालचा_Bits_Set(BitWidth, FractionBits);
    स्थिरांक APInt ExponentMask =
        APInt::getBitsSet(BitWidth, FractionBits + 1, BitWidth - 1);

    स्थिरांक तार्किक ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    स्थिरांक तार्किक ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    स्थिरांक तार्किक ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    स्थिरांक तार्किक ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();
    स्थिरांक तार्किक FractionKnownAllZeros =
        (Bits.Zero & FractionMask) == FractionMask;
    स्थिरांक तार्किक FractionKnownNotAllZeros = !(Bits.One & FractionMask).isZero();
    स्थिरांक तार्किक IntegerBitKnownSet = Bits.One[IntegerBitIndex];
    स्थिरांक तार्किक IntegerBitKnownClear = Bits.Zero[IntegerBitIndex];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    जर (ExponentKnownNotAllZeros || IntegerBitKnownSet)
      Known.knownNot(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    जर (FractionKnownNotAllZeros)
      Known.knownNot(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    जर (ExponentKnownNotAllOnes || IntegerBitKnownClear ||
        FractionKnownNotAllZeros)
      Known.knownNot(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    जर (IntegerBitKnownClear || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    जर (FractionKnownAllZeros)
      Known.knownNot(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    जर (ExponentKnownAllZeros ||
        (IntegerBitKnownSet &&
         (ExponentKnownNotAllOnes || FractionKnownAllZeros)))
      Known.knownNot(fcNan);

    स्थिरांक तार्किक QuietBitKnownSet = Bits.One[FractionBits - 1];
    स्थिरांक तार्किक QuietBitKnownClear = Bits.Zero[FractionBits - 1];

    जर (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    अन्यथा जर (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    थांबा;
  }
  पूर्वनिर्धारित:
    llvm_unreachable("unhandled supported type");
  }

  परत Known;
}

KnownBits KnownFPClass::toKnownBits(स्थिरांक fltSemantics &FltSemantics) स्थिरांक {
  KnownBits Known(FltSemantics.sizeInBits);
  स्थिरांक FPClassTest FPClasses = getKnownFPClasses();

  // Return unknown if poison.
  जर (FPClasses == fcNone)
    परत Known;

  // Return unknown for types we have not validated.
  स्वतः IsSupported = [](स्थिरांक fltSemantics &Semantics) {
    निवड (APFloat::SemanticsToEnum(Semantics)) {
    स्थिती APFloatBase::S_IEEEhalf:
    स्थिती APFloatBase::S_BFloat:
    स्थिती APFloatBase::S_IEEEsingle:
    स्थिती APFloatBase::S_IEEEdouble:
    स्थिती APFloatBase::S_IEEEquad:
    स्थिती APFloatBase::S_x87DoubleExtended:
      परत सत्य;
    पूर्वनिर्धारित:
      परत असत्य;
    }
  };
  जर (!IsSupported(FltSemantics))
    परत Known;

  निवड (APFloat::SemanticsToEnum(FltSemantics)) {
  स्थिती APFloatBase::S_IEEEhalf:
  स्थिती APFloatBase::S_BFloat:
  स्थिती APFloatBase::S_IEEEsingle:
  स्थिती APFloatBase::S_IEEEdouble:
  स्थिती APFloatBase::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    जर (FPClasses & fcNormal)
      थांबा;

    Known.setAllConflict();

    स्थिरांक अचिन्हित BitWidth = FltSemantics.sizeInBits;
    स्थिरांक अचिन्हित MantissaBits = FltSemantics.precision - 1;
    स्थिरांक अचिन्हित ExponentBits = BitWidth - MantissaBits - 1;

    APInt MantissaMask = APInt::get_खालचा_Bits_Set(BitWidth, MantissaBits);
    APInt ExponentMask =
        APInt::getBitsSet(BitWidth, MantissaBits, MantissaBits + ExponentBits);

    स्थिरांक अचिन्हित QuietBitIndex = MantissaBits - 1;
    APInt PayloadMask = MantissaMask;
    PayloadMask.clearBit(QuietBitIndex);

    जर (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // No individual payload bit is known.
      Known.Zero &= ~PayloadMask;
      Known.One &= ~PayloadMask;

      जर (FPClasses & fcQNan)
        Known.Zero.clearBit(QuietBitIndex);
      जर (FPClasses & fcSNan)
        Known.One.clearBit(QuietBitIndex);
    }
    जर (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // Mantissa bits cannot be ones.
      Known.One &= ~MantissaMask;
    }
    जर (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Unknown mantissa.
      Known.One &= ~MantissaMask;
      Known.Zero &= ~MantissaMask;
    }
    जर (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Mantissa cannot be ones.
      Known.One &= ~MantissaMask;
    }

    थांबा;
  }
  स्थिती APFloatBase::S_x87DoubleExtended: {
    जर (!isKnownNever(fcNormal | fcSubnormal | fcNan))
      थांबा;

    Known.setAllConflict();

    जर (FPClasses & fcInf)
      Known = Known.intersectWith(KnownBits::makeConstant(
          APFloat::getInf(FltSemantics).bitcastToAPInt()));

    जर (FPClasses & fcZero)
      Known = Known.intersectWith(
          KnownBits::makeConstant(APInt::getZero(FltSemantics.sizeInBits)));

    थांबा;
  }
  पूर्वनिर्धारित:
    llvm_unreachable("unhandled supported semantics");
  }

  Known.Zero.clearSignBit();
  Known.One.clearSignBit();
  जर (std::optional<तार्किक> Sign = getSignBit()) {
    जर (*Sign)
      Known.makeNegative();
    अन्यथा
      Known.makeNonNegative();
  }

  परत Known;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
स्थिर KnownFPClass fadd_impl(स्थिरांक KnownFPClass &KnownLHS,
                              स्थिरांक KnownFPClass &KnownRHS, तार्किक IsFMA) {
  KnownFPClass Known;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  जर (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNever(fcPosInf) || KnownRHS.isKnownNever(fcNegInf)) &&
      (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosInf)))
    Known.knownNot(fcNan);

  जर (KnownLHS.cannotBeOrderedLessThanZero() &&
      KnownRHS.cannotBeOrderedLessThanZero()) {
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    जर (KnownLHS.isKnownNever(fcZero | fcPosSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcPosSubnormal))
      Known.knownNot(fcZero | fcPosSubnormal);
  }

  जर (KnownLHS.cannotBeOrderedGreaterThanZero() &&
      KnownRHS.cannotBeOrderedGreaterThanZero()) {
    Known.knownNot(KnownFPClass::OrderedGreaterThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    जर (KnownLHS.isKnownNever(fcZero | fcNegSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcNegSubnormal))
      Known.knownNot(fcZero | fcNegSubnormal);
  }

  जर (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    जर (KnownLHS.isKnownNever(fcNegZero) || KnownRHS.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    Known.propagateNonSNaN(KnownLHS, KnownRHS);
  }

  परत Known;
}

KnownFPClass KnownFPClass::fadd(स्थिरांक KnownFPClass &KnownLHS_,
                                स्थिरांक KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_आदान_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_आदान_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/असत्य);

  परत applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fadd_self(स्थिरांक KnownFPClass &KnownSrc_,
                                     DenormalMode Mode) {
  KnownFPClass KnownSrc = apply_आदान_Denormal_Mode(KnownSrc_, Mode);

  KnownFPClass Known = fadd_impl(KnownSrc, KnownSrc, /*IsFMA=*/असत्य);

  जर (KnownSrc.isKnownNever(fcPosSubnormal))
    Known.knownNot(fcPosSubnormal);
  जर (KnownSrc.isKnownNever(fcNegSubnormal))
    Known.knownNot(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  जर (KnownSrc.isKnownNever(fcPosZero))
    Known.knownNot(fcPosZero);

  परत applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fsub(स्थिरांक KnownFPClass &KnownLHS_,
                                स्थिरांक KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_आदान_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_आदान_Denormal_Mode(KnownRHS_, Mode);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  KnownRHS.fneg();

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/असत्य);

  परत applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fmul(स्थिरांक KnownFPClass &KnownLHS_,
                                स्थिरांक KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_आदान_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_आदान_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  जर ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  जर ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Inf * Y => Inf or NaN
  जर (KnownLHS.isKnownAlways(fcInf | fcNan) ||
      KnownRHS.isKnownAlways(fcInf | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  जर (KnownRHS.isKnownAlways(fcZero | fcNan) ||
      KnownLHS.isKnownAlways(fcZero | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  जर (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownRHS.isKnownNeverInfinity() || KnownLHS.isKnownNeverZero()) &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverZero()))
    Known.knownNot(fcNan);

  परत applyOutputDenormalMode(Known, Mode);
}

// TODO: This generalizes to known ranges
KnownFPClass KnownFPClass::fmul(स्थिरांक KnownFPClass &KnownLHS,
                                स्थिरांक APFloat &CRHS, DenormalMode Mode) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  स्थिरांक fltSemantics &Flt = CRHS.getSemantics();
  अचिन्हित Precision = APFloat::semanticsPrecision(Flt);
  स्थिरांक पूर्णांक MantissaBits = Precision - 1;

  पूर्णांक MinKnownExponent = ilogb(CRHS);
  तार्किक CannotBeSubnormal = (MinKnownExponent >= MantissaBits);

  KnownFPClass Known = KnownFPClass::fmul(KnownLHS, KnownFPClass(CRHS), Mode);
  जर (CannotBeSubnormal)
    Known.knownNot(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  जर (KnownLHS.isKnownNever(fcInf)) {
    जर (MinKnownExponent < 0)
      Known.knownNot(fcInf);
    अन्यथा जर (MinKnownExponent == 0 && CRHS.compare_Absolute_मूल्य(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      Known.knownNot(fcInf);
  }

  परत Known;
}

KnownFPClass KnownFPClass::fdiv(स्थिरांक KnownFPClass &KnownLHS_,
                                स्थिरांक KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_आदान_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_आदान_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // Only 0/0, Inf/Inf produce NaN.
  जर (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverInfinity()) &&
      (KnownLHS.isKnownNeverZero() || KnownRHS.isKnownNeverZero())) {
    Known.knownNot(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  जर ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  जर ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Normal and subnormal results require two non-zero finite operands.
  जर ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)))
    Known.knownNot(fcNegNormal | fcNegSubnormal);
  जर ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosNormal | fcPosSubnormal);

  // 0 / X => 0 or NaN
  जर (KnownLHS.isKnownAlways(fcZero))
    Known.knownNot(fcSubnormal | fcNormal | fcInf);

  // X / 0 => NaN or Inf
  जर (KnownRHS.isKnownAlways(fcZero))
    Known.knownNot(fcFinite);

  परत applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fdiv_self(स्थिरांक KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X / X is always exactly 1.0 or a NaN.
  KnownFPClass Known(fcNan | fcPosNormal);

  Known.propagateNonSNaN(KnownSrc);

  जर (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  परत Known;
}

KnownFPClass KnownFPClass::frem(स्थिरांक KnownFPClass &KnownLHS,
                                स्थिरांक KnownFPClass &KnownRHS,
                                DenormalMode Mode) {
  KnownFPClass Known;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  जर (KnownLHS.isKnownNeverInfOrNaN() && KnownRHS.isKnownNeverNaN() &&
      KnownRHS.isKnownNeverLogicalZero(Mode)) {
    Known.knownNot(fcNan);
  }

  // The sign for frem is the same as the first operand.
  जर (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal | fcPosSubnormal);
  जर (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  जर (KnownLHS.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  जर (KnownLHS.isKnownNever(fcPosFinite) &&
      KnownLHS.isKnownNeverLogicalPosZero(Mode) &&
      (!Mode.outputsMayBePositiveZero() ||
       KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosZero);

  परत Known;
}

KnownFPClass KnownFPClass::frem_self(स्थिरांक KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X % X is always exactly [+-]0.0 or a NaN.
  KnownFPClass Known(fcNan | fcZero);

  Known.propagateNonSNaN(KnownSrc);

  जर (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  जर (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosZero);
  जर (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegZero);

  परत Known;
}

KnownFPClass KnownFPClass::fma(स्थिरांक KnownFPClass &KnownLHS,
                               स्थिरांक KnownFPClass &KnownRHS,
                               स्थिरांक KnownFPClass &KnownAddend,
                               DenormalMode Mode) {
  KnownFPClass Mul = fmul(KnownLHS, KnownRHS, Mode);

  // FMA differs from the base fmul + fadd handling only in the treatment of -0
  // results.
  //
  // If the multiply is a -0 due to rounding, the final -0 + 0 will be -0,
  // unlike for a separate fadd.
  KnownFPClass Known = fadd_impl(Mul, KnownAddend, /*IsFMA=*/सत्य);

  // propagateNonSNaN for 3 arguments.
  जर (KnownLHS.isKnownNever(fcSNan) && KnownRHS.isKnownNever(fcSNan) &&
      KnownAddend.isKnownNever(fcSNan))
    Known.knownNot(fcSNan);

  परत Known;
}

KnownFPClass KnownFPClass::fmuladd(स्थिरांक KnownFPClass &KnownLHS,
                                   स्थिरांक KnownFPClass &KnownRHS,
                                   स्थिरांक KnownFPClass &KnownAddend,
                                   DenormalMode Mode) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  KnownFPClass FMA = fma(KnownLHS, KnownRHS, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(fmul(KnownLHS, KnownRHS, Mode), KnownAddend, Mode);
  परत FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::fma_square(स्थिरांक KnownFPClass &KnownSquared,
                                      स्थिरांक KnownFPClass &KnownAddend,
                                      DenormalMode Mode) {
  KnownFPClass Squared = square(KnownSquared, Mode);
  KnownFPClass Known = fadd_impl(Squared, KnownAddend, /*IsFMA=*/सत्य);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  जर (KnownAddend.isKnownNever(fcNegInf | fcNan) && Squared.isKnownNever(fcNan))
    Known.knownNot(fcNan);

  Known.propagateNonSNaN(KnownSquared, KnownAddend);

  परत Known;
}

KnownFPClass KnownFPClass::fmuladd_square(स्थिरांक KnownFPClass &KnownSquared,
                                          स्थिरांक KnownFPClass &KnownAddend,
                                          DenormalMode Mode) {
  KnownFPClass FMA = fma_square(KnownSquared, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(square(KnownSquared, Mode), KnownAddend, Mode);
  परत FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::exp(स्थिरांक KnownFPClass &KnownSrc) {
  KnownFPClass Known;
  Known.knownNot(fcNegative);

  Known.propagateNonNaN(KnownSrc);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  जर (KnownSrc.isKnownNever(fcNegNormal | fcNegInf))
    Known.knownNot(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  जर (KnownSrc.isKnownNever(fcNegNormal))
    Known.knownNot(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  जर (KnownSrc.isKnownNever(fcPosNormal | fcPosInf))
    Known.knownNot(fcPosInf);

  परत Known;
}

KnownFPClass KnownFPClass::log(स्थिरांक KnownFPClass &KnownSrc, DenormalMode Mode,
                               तार्किक IsMultiUnitFPType) {
  KnownFPClass Known;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  जर (!IsMultiUnitFPType)
    Known.knownNot(fcNegZero | fcSubnormal);

  Known.propagateNonSNaN(KnownSrc);

  जर (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  जर (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  जर (KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNegInf);

  परत Known;
}

KnownFPClass KnownFPClass::वर्ग_मूल(स्थिरांक KnownFPClass &KnownSrc,
                                DenormalMode Mode) {
  KnownFPClass Known;
  Known.knownNot(fcPosSubnormal);

  जर (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  Known.propagateNonSNaN(KnownSrc);

  // Any negative value besides -0 returns a nan.
  जर (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  Known.knownNot(fcNegInf | fcNegSubnormal | fcNegNormal);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  जर (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  परत Known;
}

KnownFPClass KnownFPClass::sin(स्थिरांक KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownSrc);

  // Return NaN on infinite inputs.
  Known.knownNot(fcInf);
  जर (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  परत Known;
}

KnownFPClass KnownFPClass::cos(स्थिरांक KnownFPClass &KnownSrc) {
  परत sin(KnownSrc);
}

KnownFPClass KnownFPClass::tan(स्थिरांक KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. tan(+-Inf) is NaN.
  जर (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  परत Known;
}

KnownFPClass KnownFPClass::sinh(स्थिरांक KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  जर (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  जर (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal | fcPosInf);

  Known.propagateNonNaN(KnownSrc);

  परत Known;
}

KnownFPClass KnownFPClass::cosh(स्थिरांक KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  Known.knownNot(fcNegative | fcZero | fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  परत Known;
}

KnownFPClass KnownFPClass::tanh(स्थिरांक KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tanh is bounded to (-1, 1), never Inf.
  Known.knownNot(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  जर (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf)) {
    Known.knownNot(fcNegSubnormal | fcNegNormal);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    जर (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  जर (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  Known.propagateNonNaN(KnownSrc);

  परत Known;
}

KnownFPClass KnownFPClass::asin(स्थिरांक KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // asin is sign-preserving for finite arguments.
  जर (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegFinite);

  जर (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  परत Known;
}

KnownFPClass KnownFPClass::acos(स्थिरांक KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // acos(x) is bounded to [0, pi] for -1 <= x <= 1, and is never negative,
  // infinite, or subnormal. The smallest non-zero value occurs when x is
  // close to 1.0, where acos(x) can be approximated by sqrt(2 * (1 - x)).
  // Since sqrt cannot produce a subnormal result, we can conclude that
  // acos(x) will also never produce a subnormal result.
  Known.knownNot(fcNegative | fcInf | fcSubnormal);

  // acos(x) == +0.0 iff x == +1.0
  जर (KnownSrc.isKnownNever(fcPosNormal))
    Known.knownNot(fcZero);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  परत Known;
}

KnownFPClass KnownFPClass::atan(स्थिरांक KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  Known.knownNot(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  जर (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf))
    Known.knownNot(fcNegSubnormal | fcNegNormal);

  जर (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  जर (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal | fcNegZero))
    Known.knownNot(fcNegZero);

  Known.propagateNonNaN(KnownSrc);

  परत Known;
}

KnownFPClass KnownFPClass::atan2(स्थिरांक KnownFPClass &KnownY_,
                                 स्थिरांक KnownFPClass &KnownX_,
                                 DenormalMode Mode) {
  KnownFPClass KnownY = apply_आदान_Denormal_Mode(KnownY_, Mode);
  KnownFPClass KnownX = apply_आदान_Denormal_Mode(KnownX_, Mode);
  KnownFPClass Known;

  // Even though these deductions are correct, we are ignoring the following
  // potentially erroneous cases:
  //   * atan2(y, inf) is not subnormal
  //   * atan2(inf, x) is not zero or subnormal

  // atan2 result is in (-pi, pi], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonNaN(KnownY, KnownX);

  // If x <= -0.0, then |atan2(y, x)| >= pi/2
  जर (KnownX.isKnownNever(fcPositive))
    Known.knownNot(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  जर (KnownY.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  जर (KnownY.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);

  परत applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fpext(स्थिरांक KnownFPClass &KnownSrc,
                                 स्थिरांक fltSemantics &DstTy,
                                 स्थिरांक fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  KnownFPClass Known = KnownSrc;

  // All subnormal inputs should be in the normal range in the result type.
  जर (APFloat::isRepresentableAsNormalIn(SrcTy, DstTy)) {
    जर (Known.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosNormal);
    जर (Known.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegNormal);
    Known.knownNot(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  जर (!Known.isKnownNeverNaN())
    Known.setSignBit(std::nullopt);

  परत Known;
}

KnownFPClass KnownFPClass::fptrunc(स्थिरांक KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  जर (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

  Known.propagateNonNaN(KnownSrc);

  // Infinity needs a range check.
  परत Known;
}

KnownFPClass KnownFPClass::roundToIntegral(स्थिरांक KnownFPClass &KnownSrc,
                                           तार्किक MayBeMultiUnitFPType,
                                           DenormalMode Mode) {
  KnownFPClass Known;

  // Integer results cannot be subnormal.
  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  जर (!MayBeMultiUnitFPType) {
    जर (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    जर (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  }

  जर (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  जर (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  // Negative round ups towards zero produce negative zero.
  जर (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // Negative subnormals may flush to positive zero.
  जर (KnownSrc.isKnownNever(fcPosFinite) &&
      (KnownSrc.isKnownNever(fcNegSubnormal) ||
       !Mode.inputsMayBePositiveZero()))
    Known.knownNot(fcPosZero);

  परत Known;
}

KnownFPClass KnownFPClass::trunc(स्थिरांक KnownFPClass &KnownSrc,
                                 DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc,
                                       /*MayBeMultiUnitFPType=*/सत्य, Mode);

  जर (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);
  जर (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  परत Known;
}

KnownFPClass KnownFPClass::न्यूनमान(स्थिरांक KnownFPClass &KnownSrc,
                                 तार्किक IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  जर (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  परत Known;
}

KnownFPClass KnownFPClass::उच्चमान(स्थिरांक KnownFPClass &KnownSrc,
                                तार्किक IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  जर (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  परत Known;
}

KnownFPClass KnownFPClass::frexp_mant(स्थिरांक KnownFPClass &KnownSrc,
                                      DenormalMode Mode) {
  KnownFPClass Known;

  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  जर (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  जर (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  जर (KnownSrc.isKnownNeverLogicalPosZero(Mode))
    Known.knownNot(fcPosZero);

  जर (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  जर (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  जर (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  परत Known;
}

KnownFPClass KnownFPClass::ldexp(स्थिरांक KnownFPClass &KnownSrc_,
                                 स्थिरांक APInt &ConstantRangeExpMin,
                                 स्थिरांक APInt &ConstantRangeExpMax,
                                 स्थिरांक fltSemantics &Flt, DenormalMode Mode) {
  KnownFPClass Known;

  KnownFPClass KnownSrc = apply_आदान_Denormal_Mode(KnownSrc_, Mode);

  जर (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcNan);
    Known.propagateNonNaN(KnownSrc);
    परत applyOutputDenormalMode(Known, Mode);
  }

  Known.propagateNonNaN(KnownSrc);

  // Sign is preserved, but underflows may produce zeroes.
  जर (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);
  अन्यथा जर (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(OrderedLessThanZeroMask);

  जर (KnownSrc.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);
  अन्यथा जर (KnownSrc.cannotBeOrderedGreaterThanZero())
    Known.knownNot(OrderedGreaterThanZeroMask);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  जर (&Flt != &APFloat::PPCDoubleDouble()) {
    अचिन्हित Precision = APFloat::semanticsPrecision(Flt);
    स्थिरांक पूर्णांक MantissaBits = Precision - 1;
    जर (ConstantRangeExpMin.sge(MantissaBits))
      Known.knownNot(fcSubnormal);
  }

  जर (ConstantRangeExpMax.isNonPositive()) {
    // If we know the power is <= 0, can't introduce inf
    जर (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    जर (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  } अन्यथा जर (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    जर (KnownSrc.isKnownNeverPosSubnormal())
      Known.knownNot(fcPosSubnormal);
    जर (KnownSrc.isKnownNeverNegSubnormal())
      Known.knownNot(fcNegSubnormal);
    जर (KnownSrc.isKnownNeverPosZero())
      Known.knownNot(fcPosZero);
    जर (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  परत applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::ldexp(स्थिरांक KnownFPClass &KnownSrc,
                                 स्थिरांक KnownBits &ExpBits,
                                 स्थिरांक fltSemantics &Flt, DenormalMode Mode) {
  परत ldexp(KnownSrc, ExpBits.get_Signed_Min_मूल्य(),
               ExpBits.get_Signed_Max_मूल्य(), Flt, Mode);
}

KnownFPClass KnownFPClass::घात(स्थिरांक KnownFPClass &KnownLHS,
                               स्थिरांक KnownFPClass &KnownRHS) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  जर (KnownLHS.isKnownNever(fcNan | fcNegNormal | fcNegSubnormal) &&
      KnownRHS.isKnownNeverNaN())
    Known.knownNot(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  जर (KnownLHS.isKnownNever(fcNegative)) {
    Known.knownNot(fcNegative);
  } अन्यथा जर (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    जर ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcPosNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcNegNormal)))
      Known.knownNot(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    जर ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcNegNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosNormal)))
      Known.knownNot(fcNegInf);
  }

  परत Known;
}

KnownFPClass KnownFPClass::powi(स्थिरांक KnownFPClass &KnownSrc,
                                स्थिरांक KnownBits &ExponentKnownBits) {
  KnownFPClass Known;
  Known.propagateNonNaN(KnownSrc);

  जर (ExponentKnownBits.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    जर (KnownSrc.isKnownNever(fcSNan)) {
      Known.knownNot(~fcPosNormal);
      परत Known;
    }

    Known.knownNot(~(fcPosNormal | fcNan));
    परत Known;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  जर (KnownSrc.isKnownNever(fcNegative) || ExponentKnownBits.isEven()) {
    Known.knownNot(fcNegative);
  } अन्यथा जर (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    स्थिरांक तार्किक IsKnownNeverOddPositive = ExponentKnownBits.isNegative();
    स्थिरांक तार्किक IsKnownNeverOddNegative = ExponentKnownBits.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    जर ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddPositive) &&
        (KnownSrc.isKnownNever(fcNegInf) || IsKnownNeverOddNegative))
      Known.knownNot(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    जर ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddNegative) &&
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

    जर (!MayInfSrc && !MayDivByZero && !MayFiniteOverflow && !MaySubnormInv)
      Known.knownNot(fcInf);
  }

  परत Known;
}
