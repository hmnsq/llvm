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

using ನಾಮಾಕಾಶ llvm;

KnownFPClass::KnownFPClass(ಧ್ರುವ APFloat &C)
    : Known_FPClasses_ಮೌಲ್ಯ(C.classify()) {
  setSignBit(C.isNegative());
}

KnownFPClass KnownFPClass::apply_ಒಳಹರಿವು_Denormal_Mode(ಧ್ರುವ KnownFPClass &KnownSrc,
                                                  DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  ವಿಕಲ್ಪ (Mode.ಒಳಹರಿವು) {
  ಪಕ್ಷ DenormalMode::IEEE:
    ಪ್ರತ್ಯಾಗಮ Known;
  ಪಕ್ಷ DenormalMode::PreserveSign:
    ಯದಿ (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    ಯದಿ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    ಪ್ರತ್ಯಾಗಮ Known;
  ಪಕ್ಷ DenormalMode::PositiveZero:
    ಯದಿ (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      ಯದಿ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    ಪ್ರತ್ಯಾಗಮ Known;
  ಸ್ವಭಾವ:
    ಯದಿ (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      ಯದಿ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    ಯದಿ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    ಪ್ರತ್ಯಾಗಮ Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

KnownFPClass KnownFPClass::applyOutputDenormalMode(ಧ್ರುವ KnownFPClass &KnownSrc,
                                                   DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  ವಿಕಲ್ಪ (Mode.Output) {
  ಪಕ್ಷ DenormalMode::IEEE:
    ಪ್ರತ್ಯಾಗಮ Known;
  ಪಕ್ಷ DenormalMode::PreserveSign:
    ಯದಿ (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    ಯದಿ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    ಪ್ರತ್ಯಾಗಮ Known;
  ಪಕ್ಷ DenormalMode::PositiveZero:
    ಯದಿ (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      ಯದಿ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    ಪ್ರತ್ಯಾಗಮ Known;
  ಸ್ವಭಾವ:
    ಯದಿ (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      ಯದಿ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    ಯದಿ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    ಪ್ರತ್ಯಾಗಮ Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

ತಾರ್ಕಿಕ KnownFPClass::isKnownNeverLogicalZero(DenormalMode Mode) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ apply_ಒಳಹರಿವು_Denormal_Mode(*ಏತತ್, Mode).isKnownNeverZero();
}

ತಾರ್ಕಿಕ KnownFPClass::isKnownNeverLogicalNegZero(DenormalMode Mode) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ apply_ಒಳಹರಿವು_Denormal_Mode(*ಏತತ್, Mode).isKnownNeverNegZero();
}

ತಾರ್ಕಿಕ KnownFPClass::isKnownNeverLogicalPosZero(DenormalMode Mode) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ apply_ಒಳಹರಿವು_Denormal_Mode(*ಏತತ್, Mode).isKnownNeverPosZero();
}

KnownFPClass KnownFPClass::minMaxLike(ಧ್ರುವ KnownFPClass &KnownLHS_,
                                      ಧ್ರುವ KnownFPClass &KnownRHS_,
                                      MinMaxKind Kind, DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_ಒಳಹರಿವು_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_ಒಳಹರಿವು_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonNaN(KnownLHS, KnownRHS);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  ಯದಿ ((KnownLHS.isKnownNeverNaN() || KnownRHS.isKnownNeverNaN()) &&
      (Kind == MinMaxKind::minnum || Kind == MinMaxKind::maxnum ||
       Kind == MinMaxKind::minimumnum || Kind == MinMaxKind::maximumnum))
    Known.knownNot(fcNan);

  ವಿಕಲ್ಪ (Kind) {
  ಪಕ್ಷ MinMaxKind::maxnum: {
    ಯದಿ (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/ಅಸತ್ಯ));
    ಯದಿ (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/ಅಸತ್ಯ));
    ಭಙ್ಗ;
  }
  ಪಕ್ಷ MinMaxKind::minnum: {
    ಯದಿ (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/ಅಸತ್ಯ));
    ಯದಿ (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/ಅಸತ್ಯ));
    ಭಙ್ಗ;
  }
  ಪಕ್ಷ MinMaxKind::maximumnum: {
    ಯದಿ (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/ಸತ್ಯ));
    ಯದಿ (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/ಸತ್ಯ));
    ಭಙ್ಗ;
  }
  ಪಕ್ಷ MinMaxKind::minimumnum: {
    ಯದಿ (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/ಸತ್ಯ));
    ಯದಿ (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/ಸತ್ಯ));
    ಭಙ್ಗ;
  }
  ಪಕ್ಷ MinMaxKind::maximum: {
    Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/ಸತ್ಯ) |
                   orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/ಸತ್ಯ));
    ಭಙ್ಗ;
  }
  ಪಕ್ಷ MinMaxKind::minimum: {
    Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/ಸತ್ಯ) |
                   orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/ಸತ್ಯ));
    ಭಙ್ಗ;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      KnownLHS.getKnownFPClasses() | KnownRHS.getKnownFPClasses() | fcNan;
  Known.knownNot(~PossibleClasses);

  ಪ್ರತ್ಯಾಗಮ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::canonicalize(ಧ್ರುವ KnownFPClass &KnownSrc,
                                        DenormalMode DenormMode) {
  KnownFPClass Known;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  ಯದಿ (KnownSrc.isKnownNeverNaN())
    Known.knownNot(fcNan);
  ಅನ್ಯಥಾ
    Known.knownNot(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  ಯದಿ (DenormMode == DenormalMode::getIEEE()) {
    ಯದಿ (KnownSrc.isKnownNever(fcPosZero))
      Known.knownNot(fcPosZero);
    ಯದಿ (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);
    ಪ್ರತ್ಯಾಗಮ Known;
  }

  ಯದಿ (DenormMode.inputsAreZero() || DenormMode.outputsAreZero())
    Known.knownNot(fcSubnormal);

  ಯದಿ (DenormMode == DenormalMode::getPreserveSign()) {
    ಯದಿ (KnownSrc.isKnownNever(fcPosZero | fcPosSubnormal))
      Known.knownNot(fcPosZero);
    ಯದಿ (KnownSrc.isKnownNever(fcNegZero | fcNegSubnormal))
      Known.knownNot(fcNegZero);
    ಪ್ರತ್ಯಾಗಮ Known;
  }

  ಯದಿ (DenormMode.ಒಳಹರಿವು == DenormalMode::PositiveZero ||
      (DenormMode.Output == DenormalMode::PositiveZero &&
       DenormMode.ಒಳಹರಿವು == DenormalMode::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    ಯದಿ (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    ಯದಿ (KnownSrc.isKnownNever(fcPosZero | fcSubnormal))
      Known.knownNot(fcPosZero);
  }

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::bitcast(ಧ್ರುವ fltSemantics &FltSemantics,
                                   ಧ್ರುವ KnownBits &Bits) {
  ಪ್ರತಿಜ್ಞಾ(FltSemantics.sizeInBits == Bits.getBitWidth() &&
         "Bitcast operand has incorrect bit width");
  KnownFPClass Known;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  ಯದಿ (Bits.hasConflict())
    ಪ್ರತ್ಯಾಗಮ Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](ಧ್ರುವ fltSemantics &Semantics) {
    ವಿಕಲ್ಪ (APFloat::SemanticsToEnum(Semantics)) {
    ಪಕ್ಷ APFloatBase::S_IEEEhalf:
    ಪಕ್ಷ APFloatBase::S_BFloat:
    ಪಕ್ಷ APFloatBase::S_IEEEsingle:
    ಪಕ್ಷ APFloatBase::S_IEEEdouble:
    ಪಕ್ಷ APFloatBase::S_IEEEquad:
    ಪಕ್ಷ APFloatBase::S_x87DoubleExtended:
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }
  };
  ಯದಿ (!IsSupported(FltSemantics))
    ಪ್ರತ್ಯಾಗಮ Known;

  // Transfer information from the sign bit.
  ಯದಿ (Bits.isNonNegative())
    Known.signBitMustBeZero();
  ಅನ್ಯಥಾ ಯದಿ (Bits.isNegative())
    Known.signBitMustBeOne();

  ವಿಕಲ್ಪ (APFloatBase::SemanticsToEnum(FltSemantics)) {
  ಪಕ್ಷ APFloatBase::S_IEEEhalf:
  ಪಕ್ಷ APFloatBase::S_BFloat:
  ಪಕ್ಷ APFloatBase::S_IEEEsingle:
  ಪಕ್ಷ APFloatBase::S_IEEEdouble:
  ಪಕ್ಷ APFloatBase::S_IEEEquad: {
    ಧ್ರುವ unsigned MantissaBits = FltSemantics.precision - 1;
    ಧ್ರುವ APInt ExponentMask = APInt::getBitsSet(
        FltSemantics.sizeInBits, MantissaBits, FltSemantics.sizeInBits - 1);
    ಧ್ರುವ APInt MantissaMask =
        APInt::get_ಕೆಳಗಿನ_Bits_Set(FltSemantics.sizeInBits, MantissaBits);

    ಧ್ರುವ ತಾರ್ಕಿಕ ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    ಧ್ರುವ ತಾರ್ಕಿಕ ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    ಧ್ರುವ ತಾರ್ಕಿಕ ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    ಧ್ರುವ ತಾರ್ಕಿಕ ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();

    ಧ್ರುವ ತಾರ್ಕಿಕ MantissaKnownAllZeros =
        (Bits.Zero & MantissaMask) == MantissaMask;
    ಧ್ರುವ ತಾರ್ಕಿಕ MantissaKnownNotAllZeros = !(Bits.One & MantissaMask).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    ಯದಿ (ExponentKnownNotAllZeros)
      Known.knownNot(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    ಯದಿ (ExponentKnownNotAllOnes)
      Known.knownNot(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    ಯದಿ (ExponentKnownAllZeros || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // Zero and infinity require a mantissa with all zero bits.
    ಯದಿ (MantissaKnownNotAllZeros)
      Known.knownNot(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    ಯದಿ (MantissaKnownAllZeros)
      Known.knownNot(fcSubnormal | fcNan);

    ಧ್ರುವ ತಾರ್ಕಿಕ QuietBitKnownSet = Bits.One[MantissaBits - 1];
    ಧ್ರುವ ತಾರ್ಕಿಕ QuietBitKnownClear = Bits.Zero[MantissaBits - 1];

    ಯದಿ (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    ಅನ್ಯಥಾ ಯದಿ (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    ಭಙ್ಗ;
  }
  ಪಕ್ಷ APFloatBase::S_x87DoubleExtended: {
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
    ಧ್ರುವ APInt FractionMask = APInt::get_ಕೆಳಗಿನ_Bits_Set(BitWidth, FractionBits);
    ಧ್ರುವ APInt ExponentMask =
        APInt::getBitsSet(BitWidth, FractionBits + 1, BitWidth - 1);

    ಧ್ರುವ ತಾರ್ಕಿಕ ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    ಧ್ರುವ ತಾರ್ಕಿಕ ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    ಧ್ರುವ ತಾರ್ಕಿಕ ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    ಧ್ರುವ ತಾರ್ಕಿಕ ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();
    ಧ್ರುವ ತಾರ್ಕಿಕ FractionKnownAllZeros =
        (Bits.Zero & FractionMask) == FractionMask;
    ಧ್ರುವ ತಾರ್ಕಿಕ FractionKnownNotAllZeros = !(Bits.One & FractionMask).isZero();
    ಧ್ರುವ ತಾರ್ಕಿಕ IntegerBitKnownSet = Bits.One[IntegerBitIndex];
    ಧ್ರುವ ತಾರ್ಕಿಕ IntegerBitKnownClear = Bits.Zero[IntegerBitIndex];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    ಯದಿ (ExponentKnownNotAllZeros || IntegerBitKnownSet)
      Known.knownNot(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    ಯದಿ (FractionKnownNotAllZeros)
      Known.knownNot(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    ಯದಿ (ExponentKnownNotAllOnes || IntegerBitKnownClear ||
        FractionKnownNotAllZeros)
      Known.knownNot(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    ಯದಿ (IntegerBitKnownClear || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    ಯದಿ (FractionKnownAllZeros)
      Known.knownNot(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    ಯದಿ (ExponentKnownAllZeros ||
        (IntegerBitKnownSet &&
         (ExponentKnownNotAllOnes || FractionKnownAllZeros)))
      Known.knownNot(fcNan);

    ಧ್ರುವ ತಾರ್ಕಿಕ QuietBitKnownSet = Bits.One[FractionBits - 1];
    ಧ್ರುವ ತಾರ್ಕಿಕ QuietBitKnownClear = Bits.Zero[FractionBits - 1];

    ಯದಿ (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    ಅನ್ಯಥಾ ಯದಿ (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    ಭಙ್ಗ;
  }
  ಸ್ವಭಾವ:
    llvm_unreachable("unhandled supported type");
  }

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownBits KnownFPClass::toKnownBits(ಧ್ರುವ fltSemantics &FltSemantics) ಧ್ರುವ {
  KnownBits Known(FltSemantics.sizeInBits);
  ಧ್ರುವ FPClassTest FPClasses = getKnownFPClasses();

  // Return unknown if poison.
  ಯದಿ (FPClasses == fcNone)
    ಪ್ರತ್ಯಾಗಮ Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](ಧ್ರುವ fltSemantics &Semantics) {
    ವಿಕಲ್ಪ (APFloat::SemanticsToEnum(Semantics)) {
    ಪಕ್ಷ APFloatBase::S_IEEEhalf:
    ಪಕ್ಷ APFloatBase::S_BFloat:
    ಪಕ್ಷ APFloatBase::S_IEEEsingle:
    ಪಕ್ಷ APFloatBase::S_IEEEdouble:
    ಪಕ್ಷ APFloatBase::S_IEEEquad:
    ಪಕ್ಷ APFloatBase::S_x87DoubleExtended:
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    ಸ್ವಭಾವ:
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }
  };
  ಯದಿ (!IsSupported(FltSemantics))
    ಪ್ರತ್ಯಾಗಮ Known;

  ವಿಕಲ್ಪ (APFloat::SemanticsToEnum(FltSemantics)) {
  ಪಕ್ಷ APFloatBase::S_IEEEhalf:
  ಪಕ್ಷ APFloatBase::S_BFloat:
  ಪಕ್ಷ APFloatBase::S_IEEEsingle:
  ಪಕ್ಷ APFloatBase::S_IEEEdouble:
  ಪಕ್ಷ APFloatBase::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    ಯದಿ (FPClasses & fcNormal)
      ಭಙ್ಗ;

    Known.setAllConflict();

    ಧ್ರುವ unsigned BitWidth = FltSemantics.sizeInBits;
    ಧ್ರುವ unsigned MantissaBits = FltSemantics.precision - 1;
    ಧ್ರುವ unsigned ExponentBits = BitWidth - MantissaBits - 1;

    APInt MantissaMask = APInt::get_ಕೆಳಗಿನ_Bits_Set(BitWidth, MantissaBits);
    APInt ExponentMask =
        APInt::getBitsSet(BitWidth, MantissaBits, MantissaBits + ExponentBits);

    ಧ್ರುವ unsigned QuietBitIndex = MantissaBits - 1;
    APInt PayloadMask = MantissaMask;
    PayloadMask.clearBit(QuietBitIndex);

    ಯದಿ (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // No individual payload bit is known.
      Known.Zero &= ~PayloadMask;
      Known.One &= ~PayloadMask;

      ಯದಿ (FPClasses & fcQNan)
        Known.Zero.clearBit(QuietBitIndex);
      ಯದಿ (FPClasses & fcSNan)
        Known.One.clearBit(QuietBitIndex);
    }
    ಯದಿ (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // Mantissa bits cannot be ones.
      Known.One &= ~MantissaMask;
    }
    ಯದಿ (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Unknown mantissa.
      Known.One &= ~MantissaMask;
      Known.Zero &= ~MantissaMask;
    }
    ಯದಿ (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Mantissa cannot be ones.
      Known.One &= ~MantissaMask;
    }

    ಭಙ್ಗ;
  }
  ಪಕ್ಷ APFloatBase::S_x87DoubleExtended: {
    ಯದಿ (!isKnownNever(fcNormal | fcSubnormal | fcNan))
      ಭಙ್ಗ;

    Known.setAllConflict();

    ಯದಿ (FPClasses & fcInf)
      Known = Known.intersectWith(KnownBits::makeConstant(
          APFloat::getInf(FltSemantics).bitcastToAPInt()));

    ಯದಿ (FPClasses & fcZero)
      Known = Known.intersectWith(
          KnownBits::makeConstant(APInt::getZero(FltSemantics.sizeInBits)));

    ಭಙ್ಗ;
  }
  ಸ್ವಭಾವ:
    llvm_unreachable("unhandled supported semantics");
  }

  Known.Zero.clearSignBit();
  Known.One.clearSignBit();
  ಯದಿ (std::optional<ತಾರ್ಕಿಕ> Sign = getSignBit()) {
    ಯದಿ (*Sign)
      Known.makeNegative();
    ಅನ್ಯಥಾ
      Known.makeNonNegative();
  }

  ಪ್ರತ್ಯಾಗಮ Known;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
ಸ್ಥಿರ KnownFPClass fadd_impl(ಧ್ರುವ KnownFPClass &KnownLHS,
                              ಧ್ರುವ KnownFPClass &KnownRHS, ತಾರ್ಕಿಕ IsFMA) {
  KnownFPClass Known;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  ಯದಿ (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNever(fcPosInf) || KnownRHS.isKnownNever(fcNegInf)) &&
      (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosInf)))
    Known.knownNot(fcNan);

  ಯದಿ (KnownLHS.cannotBeOrderedLessThanZero() &&
      KnownRHS.cannotBeOrderedLessThanZero()) {
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    ಯದಿ (KnownLHS.isKnownNever(fcZero | fcPosSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcPosSubnormal))
      Known.knownNot(fcZero | fcPosSubnormal);
  }

  ಯದಿ (KnownLHS.cannotBeOrderedGreaterThanZero() &&
      KnownRHS.cannotBeOrderedGreaterThanZero()) {
    Known.knownNot(KnownFPClass::OrderedGreaterThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    ಯದಿ (KnownLHS.isKnownNever(fcZero | fcNegSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcNegSubnormal))
      Known.knownNot(fcZero | fcNegSubnormal);
  }

  ಯದಿ (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    ಯದಿ (KnownLHS.isKnownNever(fcNegZero) || KnownRHS.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    Known.propagateNonSNaN(KnownLHS, KnownRHS);
  }

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::fadd(ಧ್ರುವ KnownFPClass &KnownLHS_,
                                ಧ್ರುವ KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_ಒಳಹರಿವು_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_ಒಳಹರಿವು_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/ಅಸತ್ಯ);

  ಪ್ರತ್ಯಾಗಮ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fadd_self(ಧ್ರುವ KnownFPClass &KnownSrc_,
                                     DenormalMode Mode) {
  KnownFPClass KnownSrc = apply_ಒಳಹರಿವು_Denormal_Mode(KnownSrc_, Mode);

  KnownFPClass Known = fadd_impl(KnownSrc, KnownSrc, /*IsFMA=*/ಅಸತ್ಯ);

  ಯದಿ (KnownSrc.isKnownNever(fcPosSubnormal))
    Known.knownNot(fcPosSubnormal);
  ಯದಿ (KnownSrc.isKnownNever(fcNegSubnormal))
    Known.knownNot(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  ಯದಿ (KnownSrc.isKnownNever(fcPosZero))
    Known.knownNot(fcPosZero);

  ಪ್ರತ್ಯಾಗಮ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fsub(ಧ್ರುವ KnownFPClass &KnownLHS_,
                                ಧ್ರುವ KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_ಒಳಹರಿವು_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_ಒಳಹರಿವು_Denormal_Mode(KnownRHS_, Mode);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  KnownRHS.fneg();

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/ಅಸತ್ಯ);

  ಪ್ರತ್ಯಾಗಮ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fmul(ಧ್ರುವ KnownFPClass &KnownLHS_,
                                ಧ್ರುವ KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_ಒಳಹರಿವು_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_ಒಳಹರಿವು_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  ಯದಿ ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  ಯದಿ ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Inf * Y => Inf or NaN
  ಯದಿ (KnownLHS.isKnownAlways(fcInf | fcNan) ||
      KnownRHS.isKnownAlways(fcInf | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  ಯದಿ (KnownRHS.isKnownAlways(fcZero | fcNan) ||
      KnownLHS.isKnownAlways(fcZero | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  ಯದಿ (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownRHS.isKnownNeverInfinity() || KnownLHS.isKnownNeverZero()) &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverZero()))
    Known.knownNot(fcNan);

  ಪ್ರತ್ಯಾಗಮ applyOutputDenormalMode(Known, Mode);
}

// TODO: This generalizes to known ranges
KnownFPClass KnownFPClass::fmul(ಧ್ರುವ KnownFPClass &KnownLHS,
                                ಧ್ರುವ APFloat &CRHS, DenormalMode Mode) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  ಧ್ರುವ fltSemantics &Flt = CRHS.getSemantics();
  unsigned Precision = APFloat::semanticsPrecision(Flt);
  ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ MantissaBits = Precision - 1;

  ಪೂರ್ಣಾಙ್ಕ MinKnownExponent = ilogb(CRHS);
  ತಾರ್ಕಿಕ CannotBeSubnormal = (MinKnownExponent >= MantissaBits);

  KnownFPClass Known = KnownFPClass::fmul(KnownLHS, KnownFPClass(CRHS), Mode);
  ಯದಿ (CannotBeSubnormal)
    Known.knownNot(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  ಯದಿ (KnownLHS.isKnownNever(fcInf)) {
    ಯದಿ (MinKnownExponent < 0)
      Known.knownNot(fcInf);
    ಅನ್ಯಥಾ ಯದಿ (MinKnownExponent == 0 && CRHS.compare_Absolute_ಮೌಲ್ಯ(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      Known.knownNot(fcInf);
  }

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::fdiv(ಧ್ರುವ KnownFPClass &KnownLHS_,
                                ಧ್ರುವ KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_ಒಳಹರಿವು_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_ಒಳಹರಿವು_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // Only 0/0, Inf/Inf produce NaN.
  ಯದಿ (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverInfinity()) &&
      (KnownLHS.isKnownNeverZero() || KnownRHS.isKnownNeverZero())) {
    Known.knownNot(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  ಯದಿ ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  ಯದಿ ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Normal and subnormal results require two non-zero finite operands.
  ಯದಿ ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)))
    Known.knownNot(fcNegNormal | fcNegSubnormal);
  ಯದಿ ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosNormal | fcPosSubnormal);

  // 0 / X => 0 or NaN
  ಯದಿ (KnownLHS.isKnownAlways(fcZero))
    Known.knownNot(fcSubnormal | fcNormal | fcInf);

  // X / 0 => NaN or Inf
  ಯದಿ (KnownRHS.isKnownAlways(fcZero))
    Known.knownNot(fcFinite);

  ಪ್ರತ್ಯಾಗಮ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fdiv_self(ಧ್ರುವ KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X / X is always exactly 1.0 or a NaN.
  KnownFPClass Known(fcNan | fcPosNormal);

  Known.propagateNonSNaN(KnownSrc);

  ಯದಿ (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::frem(ಧ್ರುವ KnownFPClass &KnownLHS,
                                ಧ್ರುವ KnownFPClass &KnownRHS,
                                DenormalMode Mode) {
  KnownFPClass Known;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  ಯದಿ (KnownLHS.isKnownNeverInfOrNaN() && KnownRHS.isKnownNeverNaN() &&
      KnownRHS.isKnownNeverLogicalZero(Mode)) {
    Known.knownNot(fcNan);
  }

  // The sign for frem is the same as the first operand.
  ಯದಿ (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal | fcPosSubnormal);
  ಯದಿ (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  ಯದಿ (KnownLHS.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  ಯದಿ (KnownLHS.isKnownNever(fcPosFinite) &&
      KnownLHS.isKnownNeverLogicalPosZero(Mode) &&
      (!Mode.outputsMayBePositiveZero() ||
       KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosZero);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::frem_self(ಧ್ರುವ KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X % X is always exactly [+-]0.0 or a NaN.
  KnownFPClass Known(fcNan | fcZero);

  Known.propagateNonSNaN(KnownSrc);

  ಯದಿ (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  ಯದಿ (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosZero);
  ಯದಿ (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegZero);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::fma(ಧ್ರುವ KnownFPClass &KnownLHS,
                               ಧ್ರುವ KnownFPClass &KnownRHS,
                               ಧ್ರುವ KnownFPClass &KnownAddend,
                               DenormalMode Mode) {
  KnownFPClass Mul = fmul(KnownLHS, KnownRHS, Mode);

  // FMA differs from the base fmul + fadd handling only in the treatment of -0
  // results.
  //
  // If the multiply is a -0 due to rounding, the final -0 + 0 will be -0,
  // unlike for a separate fadd.
  KnownFPClass Known = fadd_impl(Mul, KnownAddend, /*IsFMA=*/ಸತ್ಯ);

  // propagateNonSNaN for 3 arguments.
  ಯದಿ (KnownLHS.isKnownNever(fcSNan) && KnownRHS.isKnownNever(fcSNan) &&
      KnownAddend.isKnownNever(fcSNan))
    Known.knownNot(fcSNan);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::fmuladd(ಧ್ರುವ KnownFPClass &KnownLHS,
                                   ಧ್ರುವ KnownFPClass &KnownRHS,
                                   ಧ್ರುವ KnownFPClass &KnownAddend,
                                   DenormalMode Mode) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  KnownFPClass FMA = fma(KnownLHS, KnownRHS, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(fmul(KnownLHS, KnownRHS, Mode), KnownAddend, Mode);
  ಪ್ರತ್ಯಾಗಮ FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::fma_square(ಧ್ರುವ KnownFPClass &KnownSquared,
                                      ಧ್ರುವ KnownFPClass &KnownAddend,
                                      DenormalMode Mode) {
  KnownFPClass Squared = square(KnownSquared, Mode);
  KnownFPClass Known = fadd_impl(Squared, KnownAddend, /*IsFMA=*/ಸತ್ಯ);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  ಯದಿ (KnownAddend.isKnownNever(fcNegInf | fcNan) && Squared.isKnownNever(fcNan))
    Known.knownNot(fcNan);

  Known.propagateNonSNaN(KnownSquared, KnownAddend);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::fmuladd_square(ಧ್ರುವ KnownFPClass &KnownSquared,
                                          ಧ್ರುವ KnownFPClass &KnownAddend,
                                          DenormalMode Mode) {
  KnownFPClass FMA = fma_square(KnownSquared, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(square(KnownSquared, Mode), KnownAddend, Mode);
  ಪ್ರತ್ಯಾಗಮ FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::exp(ಧ್ರುವ KnownFPClass &KnownSrc) {
  KnownFPClass Known;
  Known.knownNot(fcNegative);

  Known.propagateNonNaN(KnownSrc);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  ಯದಿ (KnownSrc.isKnownNever(fcNegNormal | fcNegInf))
    Known.knownNot(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  ಯದಿ (KnownSrc.isKnownNever(fcNegNormal))
    Known.knownNot(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  ಯದಿ (KnownSrc.isKnownNever(fcPosNormal | fcPosInf))
    Known.knownNot(fcPosInf);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::log(ಧ್ರುವ KnownFPClass &KnownSrc, DenormalMode Mode,
                               ತಾರ್ಕಿಕ IsMultiUnitFPType) {
  KnownFPClass Known;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  ಯದಿ (!IsMultiUnitFPType)
    Known.knownNot(fcNegZero | fcSubnormal);

  Known.propagateNonSNaN(KnownSrc);

  ಯದಿ (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  ಯದಿ (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  ಯದಿ (KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNegInf);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::sqrt(ಧ್ರುವ KnownFPClass &KnownSrc,
                                DenormalMode Mode) {
  KnownFPClass Known;
  Known.knownNot(fcPosSubnormal);

  ಯದಿ (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  Known.propagateNonSNaN(KnownSrc);

  // Any negative value besides -0 returns a nan.
  ಯದಿ (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  Known.knownNot(fcNegInf | fcNegSubnormal | fcNegNormal);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  ಯದಿ (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::sin(ಧ್ರುವ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownSrc);

  // Return NaN on infinite inputs.
  Known.knownNot(fcInf);
  ಯದಿ (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::cos(ಧ್ರುವ KnownFPClass &KnownSrc) {
  ಪ್ರತ್ಯಾಗಮ sin(KnownSrc);
}

KnownFPClass KnownFPClass::tan(ಧ್ರುವ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. tan(+-Inf) is NaN.
  ಯದಿ (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::sinh(ಧ್ರುವ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  ಯದಿ (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  ಯದಿ (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal | fcPosInf);

  Known.propagateNonNaN(KnownSrc);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::cosh(ಧ್ರುವ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  Known.knownNot(fcNegative | fcZero | fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::tanh(ಧ್ರುವ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tanh is bounded to (-1, 1), never Inf.
  Known.knownNot(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  ಯದಿ (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf)) {
    Known.knownNot(fcNegSubnormal | fcNegNormal);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    ಯದಿ (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  ಯದಿ (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  Known.propagateNonNaN(KnownSrc);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::asin(ಧ್ರುವ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // asin is sign-preserving for finite arguments.
  ಯದಿ (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegFinite);

  ಯದಿ (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::acos(ಧ್ರುವ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // acos(x) is bounded to [0, pi] for -1 <= x <= 1, and is never negative,
  // infinite, or subnormal. The smallest non-zero value occurs when x is
  // close to 1.0, where acos(x) can be approximated by sqrt(2 * (1 - x)).
  // Since sqrt cannot produce a subnormal result, we can conclude that
  // acos(x) will also never produce a subnormal result.
  Known.knownNot(fcNegative | fcInf | fcSubnormal);

  // acos(x) == +0.0 iff x == +1.0
  ಯದಿ (KnownSrc.isKnownNever(fcPosNormal))
    Known.knownNot(fcZero);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::atan(ಧ್ರುವ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  Known.knownNot(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  ಯದಿ (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf))
    Known.knownNot(fcNegSubnormal | fcNegNormal);

  ಯದಿ (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  ಯದಿ (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal | fcNegZero))
    Known.knownNot(fcNegZero);

  Known.propagateNonNaN(KnownSrc);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::atan2(ಧ್ರುವ KnownFPClass &KnownY_,
                                 ಧ್ರುವ KnownFPClass &KnownX_,
                                 DenormalMode Mode) {
  KnownFPClass KnownY = apply_ಒಳಹರಿವು_Denormal_Mode(KnownY_, Mode);
  KnownFPClass KnownX = apply_ಒಳಹರಿವು_Denormal_Mode(KnownX_, Mode);
  KnownFPClass Known;

  // Even though these deductions are correct, we are ignoring the following
  // potentially erroneous cases:
  //   * atan2(y, inf) is not subnormal
  //   * atan2(inf, x) is not zero or subnormal

  // atan2 result is in (-pi, pi], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonNaN(KnownY, KnownX);

  // If x <= -0.0, then |atan2(y, x)| >= pi/2
  ಯದಿ (KnownX.isKnownNever(fcPositive))
    Known.knownNot(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  ಯದಿ (KnownY.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  ಯದಿ (KnownY.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);

  ಪ್ರತ್ಯಾಗಮ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fpext(ಧ್ರುವ KnownFPClass &KnownSrc,
                                 ಧ್ರುವ fltSemantics &DstTy,
                                 ಧ್ರುವ fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  KnownFPClass Known = KnownSrc;

  // All subnormal inputs should be in the normal range in the result type.
  ಯದಿ (APFloat::isRepresentableAsNormalIn(SrcTy, DstTy)) {
    ಯದಿ (Known.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosNormal);
    ಯದಿ (Known.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegNormal);
    Known.knownNot(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  ಯದಿ (!Known.isKnownNeverNaN())
    Known.setSignBit(std::nullopt);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::fptrunc(ಧ್ರುವ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  ಯದಿ (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

  Known.propagateNonNaN(KnownSrc);

  // Infinity needs a range check.
  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::roundToIntegral(ಧ್ರುವ KnownFPClass &KnownSrc,
                                           ತಾರ್ಕಿಕ MayBeMultiUnitFPType,
                                           DenormalMode Mode) {
  KnownFPClass Known;

  // Integer results cannot be subnormal.
  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  ಯದಿ (!MayBeMultiUnitFPType) {
    ಯದಿ (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    ಯದಿ (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  }

  ಯದಿ (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  ಯದಿ (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  // Negative round ups towards zero produce negative zero.
  ಯದಿ (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // Negative subnormals may flush to positive zero.
  ಯದಿ (KnownSrc.isKnownNever(fcPosFinite) &&
      (KnownSrc.isKnownNever(fcNegSubnormal) ||
       !Mode.inputsMayBePositiveZero()))
    Known.knownNot(fcPosZero);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::trunc(ಧ್ರುವ KnownFPClass &KnownSrc,
                                 DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc,
                                       /*MayBeMultiUnitFPType=*/ಸತ್ಯ, Mode);

  ಯದಿ (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);
  ಯದಿ (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::floor(ಧ್ರುವ KnownFPClass &KnownSrc,
                                 ತಾರ್ಕಿಕ IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  ಯದಿ (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::ceil(ಧ್ರುವ KnownFPClass &KnownSrc,
                                ತಾರ್ಕಿಕ IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  ಯದಿ (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::frexp_mant(ಧ್ರುವ KnownFPClass &KnownSrc,
                                      DenormalMode Mode) {
  KnownFPClass Known;

  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  ಯದಿ (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  ಯದಿ (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  ಯದಿ (KnownSrc.isKnownNeverLogicalPosZero(Mode))
    Known.knownNot(fcPosZero);

  ಯದಿ (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  ಯದಿ (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  ಯದಿ (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::ldexp(ಧ್ರುವ KnownFPClass &KnownSrc_,
                                 ಧ್ರುವ APInt &ConstantRangeExpMin,
                                 ಧ್ರುವ APInt &ConstantRangeExpMax,
                                 ಧ್ರುವ fltSemantics &Flt, DenormalMode Mode) {
  KnownFPClass Known;

  KnownFPClass KnownSrc = apply_ಒಳಹರಿವು_Denormal_Mode(KnownSrc_, Mode);

  ಯದಿ (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcNan);
    Known.propagateNonNaN(KnownSrc);
    ಪ್ರತ್ಯಾಗಮ applyOutputDenormalMode(Known, Mode);
  }

  Known.propagateNonNaN(KnownSrc);

  // Sign is preserved, but underflows may produce zeroes.
  ಯದಿ (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);
  ಅನ್ಯಥಾ ಯದಿ (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(OrderedLessThanZeroMask);

  ಯದಿ (KnownSrc.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);
  ಅನ್ಯಥಾ ಯದಿ (KnownSrc.cannotBeOrderedGreaterThanZero())
    Known.knownNot(OrderedGreaterThanZeroMask);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  ಯದಿ (&Flt != &APFloat::PPCDoubleDouble()) {
    unsigned Precision = APFloat::semanticsPrecision(Flt);
    ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ MantissaBits = Precision - 1;
    ಯದಿ (ConstantRangeExpMin.sge(MantissaBits))
      Known.knownNot(fcSubnormal);
  }

  ಯದಿ (ConstantRangeExpMax.isNonPositive()) {
    // If we know the power is <= 0, can't introduce inf
    ಯದಿ (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    ಯದಿ (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  } ಅನ್ಯಥಾ ಯದಿ (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    ಯದಿ (KnownSrc.isKnownNeverPosSubnormal())
      Known.knownNot(fcPosSubnormal);
    ಯದಿ (KnownSrc.isKnownNeverNegSubnormal())
      Known.knownNot(fcNegSubnormal);
    ಯದಿ (KnownSrc.isKnownNeverPosZero())
      Known.knownNot(fcPosZero);
    ಯದಿ (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  ಪ್ರತ್ಯಾಗಮ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::ldexp(ಧ್ರುವ KnownFPClass &KnownSrc,
                                 ಧ್ರುವ KnownBits &ExpBits,
                                 ಧ್ರುವ fltSemantics &Flt, DenormalMode Mode) {
  ಪ್ರತ್ಯಾಗಮ ldexp(KnownSrc, ExpBits.get_Signed_Min_ಮೌಲ್ಯ(),
               ExpBits.get_Signed_Max_ಮೌಲ್ಯ(), Flt, Mode);
}

KnownFPClass KnownFPClass::pow(ಧ್ರುವ KnownFPClass &KnownLHS,
                               ಧ್ರುವ KnownFPClass &KnownRHS) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  ಯದಿ (KnownLHS.isKnownNever(fcNan | fcNegNormal | fcNegSubnormal) &&
      KnownRHS.isKnownNeverNaN())
    Known.knownNot(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  ಯದಿ (KnownLHS.isKnownNever(fcNegative)) {
    Known.knownNot(fcNegative);
  } ಅನ್ಯಥಾ ಯದಿ (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    ಯದಿ ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcPosNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcNegNormal)))
      Known.knownNot(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    ಯದಿ ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcNegNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosNormal)))
      Known.knownNot(fcNegInf);
  }

  ಪ್ರತ್ಯಾಗಮ Known;
}

KnownFPClass KnownFPClass::powi(ಧ್ರುವ KnownFPClass &KnownSrc,
                                ಧ್ರುವ KnownBits &ExponentKnownBits) {
  KnownFPClass Known;
  Known.propagateNonNaN(KnownSrc);

  ಯದಿ (ExponentKnownBits.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    ಯದಿ (KnownSrc.isKnownNever(fcSNan)) {
      Known.knownNot(~fcPosNormal);
      ಪ್ರತ್ಯಾಗಮ Known;
    }

    Known.knownNot(~(fcPosNormal | fcNan));
    ಪ್ರತ್ಯಾಗಮ Known;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  ಯದಿ (KnownSrc.isKnownNever(fcNegative) || ExponentKnownBits.isEven()) {
    Known.knownNot(fcNegative);
  } ಅನ್ಯಥಾ ಯದಿ (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    ಧ್ರುವ ತಾರ್ಕಿಕ IsKnownNeverOddPositive = ExponentKnownBits.isNegative();
    ಧ್ರುವ ತಾರ್ಕಿಕ IsKnownNeverOddNegative = ExponentKnownBits.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    ಯದಿ ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddPositive) &&
        (KnownSrc.isKnownNever(fcNegInf) || IsKnownNeverOddNegative))
      Known.knownNot(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    ಯದಿ ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddNegative) &&
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
    APInt MinExp = ExponentKnownBits.get_Signed_Min_ಮೌಲ್ಯ();
    APInt MaxExp = ExponentKnownBits.get_Signed_Max_ಮೌಲ್ಯ();

    // powi(inf, exp), exp > 0
    ತಾರ್ಕಿಕ MayInfSrc =
        !KnownSrc.isKnownNever(fcInf) && MaxExp.isStrictlyPositive();

    // powi(+/-0, exp), exp < 0
    ತಾರ್ಕಿಕ MayDivByZero = !KnownSrc.isKnownNever(fcZero) && MinExp.isNegative();

    // powi(finite, exp), |exp| > 1
    ತಾರ್ಕಿಕ MayFinite = !KnownSrc.isKnownNever(fcNormal | fcSubnormal);
    ತಾರ್ಕಿಕ MayAbsExpGT1 = MinExp.slt(-1) || MaxExp.sgt(1);
    ತಾರ್ಕಿಕ MayFiniteOverflow = MayFinite && MayAbsExpGT1;

    // powi(subnormal, -1)
    ತಾರ್ಕಿಕ MayBeNegOne = ExponentKnownBits.Zero.isZero();
    ತಾರ್ಕಿಕ MaySubnormInv = !KnownSrc.isKnownNever(fcSubnormal) && MayBeNegOne;

    ಯದಿ (!MayInfSrc && !MayDivByZero && !MayFiniteOverflow && !MaySubnormInv)
      Known.knownNot(fcInf);
  }

  ಪ್ರತ್ಯಾಗಮ Known;
}
