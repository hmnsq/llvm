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

#include "llvm/Support/KnownFPClass.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/KnownBits.h"

using namespace llvm;

KnownFPClass::KnownFPClass(const APFloat &C)
    : KnownFPClassesValue(C.classify()) {
  setSignBit(C.isNegative());
}

KnownFPClass KnownFPClass::applyInputDenormalMode(const KnownFPClass &KnownSrc,
                                                  DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  switch (Mode.Input) {
  pak_sa DenormalMode::IEEE:
    ပြန်ပေး Known;
  pak_sa DenormalMode::PreserveSign:
    အကယ်၍ (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    အကယ်၍ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    ပြန်ပေး Known;
  pak_sa DenormalMode::PositiveZero:
    အကယ်၍ (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      အကယ်၍ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    ပြန်ပေး Known;
  default:
    အကယ်၍ (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      အကယ်၍ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    အကယ်၍ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    ပြန်ပေး Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

KnownFPClass KnownFPClass::applyOutputDenormalMode(const KnownFPClass &KnownSrc,
                                                   DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  switch (Mode.Output) {
  pak_sa DenormalMode::IEEE:
    ပြန်ပေး Known;
  pak_sa DenormalMode::PreserveSign:
    အကယ်၍ (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    အကယ်၍ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    ပြန်ပေး Known;
  pak_sa DenormalMode::PositiveZero:
    အကယ်၍ (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      အကယ်၍ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    ပြန်ပေး Known;
  default:
    အကယ်၍ (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      အကယ်၍ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    အကယ်၍ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    ပြန်ပေး Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

bool KnownFPClass::isKnownNeverLogicalZero(DenormalMode Mode) const {
  ပြန်ပေး applyInputDenormalMode(*this, Mode).isKnownNeverZero();
}

bool KnownFPClass::isKnownNeverLogicalNegZero(DenormalMode Mode) const {
  ပြန်ပေး applyInputDenormalMode(*this, Mode).isKnownNeverNegZero();
}

bool KnownFPClass::isKnownNeverLogicalPosZero(DenormalMode Mode) const {
  ပြန်ပေး applyInputDenormalMode(*this, Mode).isKnownNeverPosZero();
}

KnownFPClass KnownFPClass::minMaxLike(const KnownFPClass &KnownLHS_,
                                      const KnownFPClass &KnownRHS_,
                                      MinMaxKind Kind, DenormalMode Mode) {
  KnownFPClass KnownLHS = applyInputDenormalMode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = applyInputDenormalMode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonNaN(KnownLHS, KnownRHS);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  အကယ်၍ ((KnownLHS.isKnownNeverNaN() || KnownRHS.isKnownNeverNaN()) &&
      (Kind == MinMaxKind::minnum || Kind == MinMaxKind::maxnum ||
       Kind == MinMaxKind::minimumnum || Kind == MinMaxKind::maximumnum))
    Known.knownNot(fcNan);

  switch (Kind) {
  pak_sa MinMaxKind::maxnum: {
    အကယ်၍ (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/false));
    အကယ်၍ (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/false));
    bha_nga;
  }
  pak_sa MinMaxKind::minnum: {
    အကယ်၍ (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/false));
    အကယ်၍ (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/false));
    bha_nga;
  }
  pak_sa MinMaxKind::maximumnum: {
    အကယ်၍ (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/true));
    အကယ်၍ (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/true));
    bha_nga;
  }
  pak_sa MinMaxKind::minimumnum: {
    အကယ်၍ (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/true));
    အကယ်၍ (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/true));
    bha_nga;
  }
  pak_sa MinMaxKind::maximum: {
    Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/true) |
                   orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/true));
    bha_nga;
  }
  pak_sa MinMaxKind::minimum: {
    Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/true) |
                   orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/true));
    bha_nga;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      KnownLHS.getKnownFPClasses() | KnownRHS.getKnownFPClasses() | fcNan;
  Known.knownNot(~PossibleClasses);

  ပြန်ပေး applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::canonicalize(const KnownFPClass &KnownSrc,
                                        DenormalMode DenormMode) {
  KnownFPClass Known;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  အကယ်၍ (KnownSrc.isKnownNeverNaN())
    Known.knownNot(fcNan);
  မဟုတ်ပါက
    Known.knownNot(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  အကယ်၍ (DenormMode == DenormalMode::getIEEE()) {
    အကယ်၍ (KnownSrc.isKnownNever(fcPosZero))
      Known.knownNot(fcPosZero);
    အကယ်၍ (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);
    ပြန်ပေး Known;
  }

  အကယ်၍ (DenormMode.inputsAreZero() || DenormMode.outputsAreZero())
    Known.knownNot(fcSubnormal);

  အကယ်၍ (DenormMode == DenormalMode::getPreserveSign()) {
    အကယ်၍ (KnownSrc.isKnownNever(fcPosZero | fcPosSubnormal))
      Known.knownNot(fcPosZero);
    အကယ်၍ (KnownSrc.isKnownNever(fcNegZero | fcNegSubnormal))
      Known.knownNot(fcNegZero);
    ပြန်ပေး Known;
  }

  အကယ်၍ (DenormMode.Input == DenormalMode::PositiveZero ||
      (DenormMode.Output == DenormalMode::PositiveZero &&
       DenormMode.Input == DenormalMode::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    အကယ်၍ (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    အကယ်၍ (KnownSrc.isKnownNever(fcPosZero | fcSubnormal))
      Known.knownNot(fcPosZero);
  }

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::bitcast(const fltSemantics &FltSemantics,
                                   const KnownBits &Bits) {
  assert(FltSemantics.sizeInBits == Bits.getBitWidth() &&
         "Bitcast operand has incorrect bit width");
  KnownFPClass Known;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  အကယ်၍ (Bits.hasConflict())
    ပြန်ပေး Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](const fltSemantics &Semantics) {
    switch (APFloat::SemanticsToEnum(Semantics)) {
    pak_sa APFloatBase::S_IEEEhalf:
    pak_sa APFloatBase::S_BFloat:
    pak_sa APFloatBase::S_IEEEsingle:
    pak_sa APFloatBase::S_IEEEdouble:
    pak_sa APFloatBase::S_IEEEquad:
    pak_sa APFloatBase::S_x87DoubleExtended:
      ပြန်ပေး true;
    default:
      ပြန်ပေး false;
    }
  };
  အကယ်၍ (!IsSupported(FltSemantics))
    ပြန်ပေး Known;

  // Transfer information from the sign bit.
  အကယ်၍ (Bits.isNonNegative())
    Known.signBitMustBeZero();
  မဟုတ်ပါက အကယ်၍ (Bits.isNegative())
    Known.signBitMustBeOne();

  switch (APFloatBase::SemanticsToEnum(FltSemantics)) {
  pak_sa APFloatBase::S_IEEEhalf:
  pak_sa APFloatBase::S_BFloat:
  pak_sa APFloatBase::S_IEEEsingle:
  pak_sa APFloatBase::S_IEEEdouble:
  pak_sa APFloatBase::S_IEEEquad: {
    const unsigned MantissaBits = FltSemantics.precision - 1;
    const APInt ExponentMask = APInt::getBitsSet(
        FltSemantics.sizeInBits, MantissaBits, FltSemantics.sizeInBits - 1);
    const APInt MantissaMask =
        APInt::getLowBitsSet(FltSemantics.sizeInBits, MantissaBits);

    const bool ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    const bool ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    const bool ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    const bool ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();

    const bool MantissaKnownAllZeros =
        (Bits.Zero & MantissaMask) == MantissaMask;
    const bool MantissaKnownNotAllZeros = !(Bits.One & MantissaMask).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    အကယ်၍ (ExponentKnownNotAllZeros)
      Known.knownNot(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    အကယ်၍ (ExponentKnownNotAllOnes)
      Known.knownNot(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    အကယ်၍ (ExponentKnownAllZeros || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // Zero and infinity require a mantissa with all zero bits.
    အကယ်၍ (MantissaKnownNotAllZeros)
      Known.knownNot(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    အကယ်၍ (MantissaKnownAllZeros)
      Known.knownNot(fcSubnormal | fcNan);

    const bool QuietBitKnownSet = Bits.One[MantissaBits - 1];
    const bool QuietBitKnownClear = Bits.Zero[MantissaBits - 1];

    အကယ်၍ (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    မဟုတ်ပါက အကယ်၍ (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    bha_nga;
  }
  pak_sa APFloatBase::S_x87DoubleExtended: {
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
    const APInt FractionMask = APInt::getLowBitsSet(BitWidth, FractionBits);
    const APInt ExponentMask =
        APInt::getBitsSet(BitWidth, FractionBits + 1, BitWidth - 1);

    const bool ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    const bool ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    const bool ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    const bool ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();
    const bool FractionKnownAllZeros =
        (Bits.Zero & FractionMask) == FractionMask;
    const bool FractionKnownNotAllZeros = !(Bits.One & FractionMask).isZero();
    const bool IntegerBitKnownSet = Bits.One[IntegerBitIndex];
    const bool IntegerBitKnownClear = Bits.Zero[IntegerBitIndex];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    အကယ်၍ (ExponentKnownNotAllZeros || IntegerBitKnownSet)
      Known.knownNot(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    အကယ်၍ (FractionKnownNotAllZeros)
      Known.knownNot(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    အကယ်၍ (ExponentKnownNotAllOnes || IntegerBitKnownClear ||
        FractionKnownNotAllZeros)
      Known.knownNot(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    အကယ်၍ (IntegerBitKnownClear || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    အကယ်၍ (FractionKnownAllZeros)
      Known.knownNot(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    အကယ်၍ (ExponentKnownAllZeros ||
        (IntegerBitKnownSet &&
         (ExponentKnownNotAllOnes || FractionKnownAllZeros)))
      Known.knownNot(fcNan);

    const bool QuietBitKnownSet = Bits.One[FractionBits - 1];
    const bool QuietBitKnownClear = Bits.Zero[FractionBits - 1];

    အကယ်၍ (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    မဟုတ်ပါက အကယ်၍ (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    bha_nga;
  }
  default:
    llvm_unreachable("unhandled supported type");
  }

  ပြန်ပေး Known;
}

KnownBits KnownFPClass::toKnownBits(const fltSemantics &FltSemantics) const {
  KnownBits Known(FltSemantics.sizeInBits);
  const FPClassTest FPClasses = getKnownFPClasses();

  // Return unknown if poison.
  အကယ်၍ (FPClasses == fcNone)
    ပြန်ပေး Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](const fltSemantics &Semantics) {
    switch (APFloat::SemanticsToEnum(Semantics)) {
    pak_sa APFloatBase::S_IEEEhalf:
    pak_sa APFloatBase::S_BFloat:
    pak_sa APFloatBase::S_IEEEsingle:
    pak_sa APFloatBase::S_IEEEdouble:
    pak_sa APFloatBase::S_IEEEquad:
    pak_sa APFloatBase::S_x87DoubleExtended:
      ပြန်ပေး true;
    default:
      ပြန်ပေး false;
    }
  };
  အကယ်၍ (!IsSupported(FltSemantics))
    ပြန်ပေး Known;

  switch (APFloat::SemanticsToEnum(FltSemantics)) {
  pak_sa APFloatBase::S_IEEEhalf:
  pak_sa APFloatBase::S_BFloat:
  pak_sa APFloatBase::S_IEEEsingle:
  pak_sa APFloatBase::S_IEEEdouble:
  pak_sa APFloatBase::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    အကယ်၍ (FPClasses & fcNormal)
      bha_nga;

    Known.setAllConflict();

    const unsigned BitWidth = FltSemantics.sizeInBits;
    const unsigned MantissaBits = FltSemantics.precision - 1;
    const unsigned ExponentBits = BitWidth - MantissaBits - 1;

    APInt MantissaMask = APInt::getLowBitsSet(BitWidth, MantissaBits);
    APInt ExponentMask =
        APInt::getBitsSet(BitWidth, MantissaBits, MantissaBits + ExponentBits);

    const unsigned QuietBitIndex = MantissaBits - 1;
    APInt PayloadMask = MantissaMask;
    PayloadMask.clearBit(QuietBitIndex);

    အကယ်၍ (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // No individual payload bit is known.
      Known.Zero &= ~PayloadMask;
      Known.One &= ~PayloadMask;

      အကယ်၍ (FPClasses & fcQNan)
        Known.Zero.clearBit(QuietBitIndex);
      အကယ်၍ (FPClasses & fcSNan)
        Known.One.clearBit(QuietBitIndex);
    }
    အကယ်၍ (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // Mantissa bits cannot be ones.
      Known.One &= ~MantissaMask;
    }
    အကယ်၍ (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Unknown mantissa.
      Known.One &= ~MantissaMask;
      Known.Zero &= ~MantissaMask;
    }
    အကယ်၍ (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Mantissa cannot be ones.
      Known.One &= ~MantissaMask;
    }

    bha_nga;
  }
  pak_sa APFloatBase::S_x87DoubleExtended: {
    အကယ်၍ (!isKnownNever(fcNormal | fcSubnormal | fcNan))
      bha_nga;

    Known.setAllConflict();

    အကယ်၍ (FPClasses & fcInf)
      Known = Known.intersectWith(KnownBits::makeConstant(
          APFloat::getInf(FltSemantics).bitcastToAPInt()));

    အကယ်၍ (FPClasses & fcZero)
      Known = Known.intersectWith(
          KnownBits::makeConstant(APInt::getZero(FltSemantics.sizeInBits)));

    bha_nga;
  }
  default:
    llvm_unreachable("unhandled supported semantics");
  }

  Known.Zero.clearSignBit();
  Known.One.clearSignBit();
  အကယ်၍ (std::optional<bool> Sign = getSignBit()) {
    အကယ်၍ (*Sign)
      Known.makeNegative();
    မဟုတ်ပါက
      Known.makeNonNegative();
  }

  ပြန်ပေး Known;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
static KnownFPClass fadd_impl(const KnownFPClass &KnownLHS,
                              const KnownFPClass &KnownRHS, bool IsFMA) {
  KnownFPClass Known;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  အကယ်၍ (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNever(fcPosInf) || KnownRHS.isKnownNever(fcNegInf)) &&
      (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosInf)))
    Known.knownNot(fcNan);

  အကယ်၍ (KnownLHS.cannotBeOrderedLessThanZero() &&
      KnownRHS.cannotBeOrderedLessThanZero()) {
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    အကယ်၍ (KnownLHS.isKnownNever(fcZero | fcPosSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcPosSubnormal))
      Known.knownNot(fcZero | fcPosSubnormal);
  }

  အကယ်၍ (KnownLHS.cannotBeOrderedGreaterThanZero() &&
      KnownRHS.cannotBeOrderedGreaterThanZero()) {
    Known.knownNot(KnownFPClass::OrderedGreaterThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    အကယ်၍ (KnownLHS.isKnownNever(fcZero | fcNegSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcNegSubnormal))
      Known.knownNot(fcZero | fcNegSubnormal);
  }

  အကယ်၍ (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    အကယ်၍ (KnownLHS.isKnownNever(fcNegZero) || KnownRHS.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    Known.propagateNonSNaN(KnownLHS, KnownRHS);
  }

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::fadd(const KnownFPClass &KnownLHS_,
                                const KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = applyInputDenormalMode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = applyInputDenormalMode(KnownRHS_, Mode);

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/false);

  ပြန်ပေး applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fadd_self(const KnownFPClass &KnownSrc_,
                                     DenormalMode Mode) {
  KnownFPClass KnownSrc = applyInputDenormalMode(KnownSrc_, Mode);

  KnownFPClass Known = fadd_impl(KnownSrc, KnownSrc, /*IsFMA=*/false);

  အကယ်၍ (KnownSrc.isKnownNever(fcPosSubnormal))
    Known.knownNot(fcPosSubnormal);
  အကယ်၍ (KnownSrc.isKnownNever(fcNegSubnormal))
    Known.knownNot(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  အကယ်၍ (KnownSrc.isKnownNever(fcPosZero))
    Known.knownNot(fcPosZero);

  ပြန်ပေး applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fsub(const KnownFPClass &KnownLHS_,
                                const KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = applyInputDenormalMode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = applyInputDenormalMode(KnownRHS_, Mode);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  KnownRHS.fneg();

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/false);

  ပြန်ပေး applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fmul(const KnownFPClass &KnownLHS_,
                                const KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = applyInputDenormalMode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = applyInputDenormalMode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  အကယ်၍ ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  အကယ်၍ ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Inf * Y => Inf or NaN
  အကယ်၍ (KnownLHS.isKnownAlways(fcInf | fcNan) ||
      KnownRHS.isKnownAlways(fcInf | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  အကယ်၍ (KnownRHS.isKnownAlways(fcZero | fcNan) ||
      KnownLHS.isKnownAlways(fcZero | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  အကယ်၍ (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownRHS.isKnownNeverInfinity() || KnownLHS.isKnownNeverZero()) &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverZero()))
    Known.knownNot(fcNan);

  ပြန်ပေး applyOutputDenormalMode(Known, Mode);
}

// TODO: This generalizes to known ranges
KnownFPClass KnownFPClass::fmul(const KnownFPClass &KnownLHS,
                                const APFloat &CRHS, DenormalMode Mode) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  const fltSemantics &Flt = CRHS.getSemantics();
  unsigned Precision = APFloat::semanticsPrecision(Flt);
  const ကိန်းပြည့် MantissaBits = Precision - 1;

  ကိန်းပြည့် MinKnownExponent = ilogb(CRHS);
  bool CannotBeSubnormal = (MinKnownExponent >= MantissaBits);

  KnownFPClass Known = KnownFPClass::fmul(KnownLHS, KnownFPClass(CRHS), Mode);
  အကယ်၍ (CannotBeSubnormal)
    Known.knownNot(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  အကယ်၍ (KnownLHS.isKnownNever(fcInf)) {
    အကယ်၍ (MinKnownExponent < 0)
      Known.knownNot(fcInf);
    မဟုတ်ပါက အကယ်၍ (MinKnownExponent == 0 && CRHS.compareAbsoluteValue(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      Known.knownNot(fcInf);
  }

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::fdiv(const KnownFPClass &KnownLHS_,
                                const KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = applyInputDenormalMode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = applyInputDenormalMode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // Only 0/0, Inf/Inf produce NaN.
  အကယ်၍ (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverInfinity()) &&
      (KnownLHS.isKnownNeverZero() || KnownRHS.isKnownNeverZero())) {
    Known.knownNot(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  အကယ်၍ ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  အကယ်၍ ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Normal and subnormal results require two non-zero finite operands.
  အကယ်၍ ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)))
    Known.knownNot(fcNegNormal | fcNegSubnormal);
  အကယ်၍ ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosNormal | fcPosSubnormal);

  // 0 / X => 0 or NaN
  အကယ်၍ (KnownLHS.isKnownAlways(fcZero))
    Known.knownNot(fcSubnormal | fcNormal | fcInf);

  // X / 0 => NaN or Inf
  အကယ်၍ (KnownRHS.isKnownAlways(fcZero))
    Known.knownNot(fcFinite);

  ပြန်ပေး applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fdiv_self(const KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X / X is always exactly 1.0 or a NaN.
  KnownFPClass Known(fcNan | fcPosNormal);

  Known.propagateNonSNaN(KnownSrc);

  အကယ်၍ (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::frem(const KnownFPClass &KnownLHS,
                                const KnownFPClass &KnownRHS,
                                DenormalMode Mode) {
  KnownFPClass Known;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  အကယ်၍ (KnownLHS.isKnownNeverInfOrNaN() && KnownRHS.isKnownNeverNaN() &&
      KnownRHS.isKnownNeverLogicalZero(Mode)) {
    Known.knownNot(fcNan);
  }

  // The sign for frem is the same as the first operand.
  အကယ်၍ (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal | fcPosSubnormal);
  အကယ်၍ (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  အကယ်၍ (KnownLHS.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  အကယ်၍ (KnownLHS.isKnownNever(fcPosFinite) &&
      KnownLHS.isKnownNeverLogicalPosZero(Mode) &&
      (!Mode.outputsMayBePositiveZero() ||
       KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosZero);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::frem_self(const KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X % X is always exactly [+-]0.0 or a NaN.
  KnownFPClass Known(fcNan | fcZero);

  Known.propagateNonSNaN(KnownSrc);

  အကယ်၍ (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  အကယ်၍ (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosZero);
  အကယ်၍ (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegZero);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::fma(const KnownFPClass &KnownLHS,
                               const KnownFPClass &KnownRHS,
                               const KnownFPClass &KnownAddend,
                               DenormalMode Mode) {
  KnownFPClass Mul = fmul(KnownLHS, KnownRHS, Mode);

  // FMA differs from the base fmul + fadd handling only in the treatment of -0
  // results.
  //
  // If the multiply is a -0 due to rounding, the final -0 + 0 will be -0,
  // unlike for a separate fadd.
  KnownFPClass Known = fadd_impl(Mul, KnownAddend, /*IsFMA=*/true);

  // propagateNonSNaN for 3 arguments.
  အကယ်၍ (KnownLHS.isKnownNever(fcSNan) && KnownRHS.isKnownNever(fcSNan) &&
      KnownAddend.isKnownNever(fcSNan))
    Known.knownNot(fcSNan);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::fmuladd(const KnownFPClass &KnownLHS,
                                   const KnownFPClass &KnownRHS,
                                   const KnownFPClass &KnownAddend,
                                   DenormalMode Mode) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  KnownFPClass FMA = fma(KnownLHS, KnownRHS, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(fmul(KnownLHS, KnownRHS, Mode), KnownAddend, Mode);
  ပြန်ပေး FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::fma_square(const KnownFPClass &KnownSquared,
                                      const KnownFPClass &KnownAddend,
                                      DenormalMode Mode) {
  KnownFPClass Squared = square(KnownSquared, Mode);
  KnownFPClass Known = fadd_impl(Squared, KnownAddend, /*IsFMA=*/true);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  အကယ်၍ (KnownAddend.isKnownNever(fcNegInf | fcNan) && Squared.isKnownNever(fcNan))
    Known.knownNot(fcNan);

  Known.propagateNonSNaN(KnownSquared, KnownAddend);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::fmuladd_square(const KnownFPClass &KnownSquared,
                                          const KnownFPClass &KnownAddend,
                                          DenormalMode Mode) {
  KnownFPClass FMA = fma_square(KnownSquared, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(square(KnownSquared, Mode), KnownAddend, Mode);
  ပြန်ပေး FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::exp(const KnownFPClass &KnownSrc) {
  KnownFPClass Known;
  Known.knownNot(fcNegative);

  Known.propagateNonNaN(KnownSrc);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  အကယ်၍ (KnownSrc.isKnownNever(fcNegNormal | fcNegInf))
    Known.knownNot(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  အကယ်၍ (KnownSrc.isKnownNever(fcNegNormal))
    Known.knownNot(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  အကယ်၍ (KnownSrc.isKnownNever(fcPosNormal | fcPosInf))
    Known.knownNot(fcPosInf);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::log(const KnownFPClass &KnownSrc, DenormalMode Mode,
                               bool IsMultiUnitFPType) {
  KnownFPClass Known;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  အကယ်၍ (!IsMultiUnitFPType)
    Known.knownNot(fcNegZero | fcSubnormal);

  Known.propagateNonSNaN(KnownSrc);

  အကယ်၍ (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  အကယ်၍ (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  အကယ်၍ (KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNegInf);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::sqrt(const KnownFPClass &KnownSrc,
                                DenormalMode Mode) {
  KnownFPClass Known;
  Known.knownNot(fcPosSubnormal);

  အကယ်၍ (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  Known.propagateNonSNaN(KnownSrc);

  // Any negative value besides -0 returns a nan.
  အကယ်၍ (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  Known.knownNot(fcNegInf | fcNegSubnormal | fcNegNormal);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  အကယ်၍ (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::sin(const KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownSrc);

  // Return NaN on infinite inputs.
  Known.knownNot(fcInf);
  အကယ်၍ (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::cos(const KnownFPClass &KnownSrc) {
  ပြန်ပေး sin(KnownSrc);
}

KnownFPClass KnownFPClass::tan(const KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. tan(+-Inf) is NaN.
  အကယ်၍ (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::sinh(const KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  အကယ်၍ (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  အကယ်၍ (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal | fcPosInf);

  Known.propagateNonNaN(KnownSrc);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::cosh(const KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  Known.knownNot(fcNegative | fcZero | fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::tanh(const KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tanh is bounded to (-1, 1), never Inf.
  Known.knownNot(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  အကယ်၍ (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf)) {
    Known.knownNot(fcNegSubnormal | fcNegNormal);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    အကယ်၍ (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  အကယ်၍ (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  Known.propagateNonNaN(KnownSrc);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::asin(const KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // asin is sign-preserving for finite arguments.
  အကယ်၍ (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegFinite);

  အကယ်၍ (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::acos(const KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // acos(x) is bounded to [0, pi] for -1 <= x <= 1, and is never negative,
  // infinite, or subnormal. The smallest non-zero value occurs when x is
  // close to 1.0, where acos(x) can be approximated by sqrt(2 * (1 - x)).
  // Since sqrt cannot produce a subnormal result, we can conclude that
  // acos(x) will also never produce a subnormal result.
  Known.knownNot(fcNegative | fcInf | fcSubnormal);

  // acos(x) == +0.0 iff x == +1.0
  အကယ်၍ (KnownSrc.isKnownNever(fcPosNormal))
    Known.knownNot(fcZero);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::atan(const KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  Known.knownNot(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  အကယ်၍ (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf))
    Known.knownNot(fcNegSubnormal | fcNegNormal);

  အကယ်၍ (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  အကယ်၍ (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal | fcNegZero))
    Known.knownNot(fcNegZero);

  Known.propagateNonNaN(KnownSrc);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::atan2(const KnownFPClass &KnownY_,
                                 const KnownFPClass &KnownX_,
                                 DenormalMode Mode) {
  KnownFPClass KnownY = applyInputDenormalMode(KnownY_, Mode);
  KnownFPClass KnownX = applyInputDenormalMode(KnownX_, Mode);
  KnownFPClass Known;

  // Even though these deductions are correct, we are ignoring the following
  // potentially erroneous cases:
  //   * atan2(y, inf) is not subnormal
  //   * atan2(inf, x) is not zero or subnormal

  // atan2 result is in (-pi, pi], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonNaN(KnownY, KnownX);

  // If x <= -0.0, then |atan2(y, x)| >= pi/2
  အကယ်၍ (KnownX.isKnownNever(fcPositive))
    Known.knownNot(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  အကယ်၍ (KnownY.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  အကယ်၍ (KnownY.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);

  ပြန်ပေး applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fpext(const KnownFPClass &KnownSrc,
                                 const fltSemantics &DstTy,
                                 const fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  KnownFPClass Known = KnownSrc;

  // All subnormal inputs should be in the normal range in the result type.
  အကယ်၍ (APFloat::isRepresentableAsNormalIn(SrcTy, DstTy)) {
    အကယ်၍ (Known.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosNormal);
    အကယ်၍ (Known.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegNormal);
    Known.knownNot(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  အကယ်၍ (!Known.isKnownNeverNaN())
    Known.setSignBit(std::nullopt);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::fptrunc(const KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  အကယ်၍ (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

  Known.propagateNonNaN(KnownSrc);

  // Infinity needs a range check.
  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::roundToIntegral(const KnownFPClass &KnownSrc,
                                           bool MayBeMultiUnitFPType,
                                           DenormalMode Mode) {
  KnownFPClass Known;

  // Integer results cannot be subnormal.
  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  အကယ်၍ (!MayBeMultiUnitFPType) {
    အကယ်၍ (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    အကယ်၍ (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  }

  အကယ်၍ (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  အကယ်၍ (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  // Negative round ups towards zero produce negative zero.
  အကယ်၍ (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // Negative subnormals may flush to positive zero.
  အကယ်၍ (KnownSrc.isKnownNever(fcPosFinite) &&
      (KnownSrc.isKnownNever(fcNegSubnormal) ||
       !Mode.inputsMayBePositiveZero()))
    Known.knownNot(fcPosZero);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::trunc(const KnownFPClass &KnownSrc,
                                 DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc,
                                       /*MayBeMultiUnitFPType=*/true, Mode);

  အကယ်၍ (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);
  အကယ်၍ (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::floor(const KnownFPClass &KnownSrc,
                                 bool IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  အကယ်၍ (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::ceil(const KnownFPClass &KnownSrc,
                                bool IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  အကယ်၍ (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::frexp_mant(const KnownFPClass &KnownSrc,
                                      DenormalMode Mode) {
  KnownFPClass Known;

  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  အကယ်၍ (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  အကယ်၍ (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  အကယ်၍ (KnownSrc.isKnownNeverLogicalPosZero(Mode))
    Known.knownNot(fcPosZero);

  အကယ်၍ (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  အကယ်၍ (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  အကယ်၍ (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::ldexp(const KnownFPClass &KnownSrc_,
                                 const APInt &ConstantRangeExpMin,
                                 const APInt &ConstantRangeExpMax,
                                 const fltSemantics &Flt, DenormalMode Mode) {
  KnownFPClass Known;

  KnownFPClass KnownSrc = applyInputDenormalMode(KnownSrc_, Mode);

  အကယ်၍ (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcNan);
    Known.propagateNonNaN(KnownSrc);
    ပြန်ပေး applyOutputDenormalMode(Known, Mode);
  }

  Known.propagateNonNaN(KnownSrc);

  // Sign is preserved, but underflows may produce zeroes.
  အကယ်၍ (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);
  မဟုတ်ပါက အကယ်၍ (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(OrderedLessThanZeroMask);

  အကယ်၍ (KnownSrc.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);
  မဟုတ်ပါက အကယ်၍ (KnownSrc.cannotBeOrderedGreaterThanZero())
    Known.knownNot(OrderedGreaterThanZeroMask);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  အကယ်၍ (&Flt != &APFloat::PPCDoubleDouble()) {
    unsigned Precision = APFloat::semanticsPrecision(Flt);
    const ကိန်းပြည့် MantissaBits = Precision - 1;
    အကယ်၍ (ConstantRangeExpMin.sge(MantissaBits))
      Known.knownNot(fcSubnormal);
  }

  အကယ်၍ (ConstantRangeExpMax.isNonPositive()) {
    // If we know the power is <= 0, can't introduce inf
    အကယ်၍ (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    အကယ်၍ (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  } မဟုတ်ပါက အကယ်၍ (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    အကယ်၍ (KnownSrc.isKnownNeverPosSubnormal())
      Known.knownNot(fcPosSubnormal);
    အကယ်၍ (KnownSrc.isKnownNeverNegSubnormal())
      Known.knownNot(fcNegSubnormal);
    အကယ်၍ (KnownSrc.isKnownNeverPosZero())
      Known.knownNot(fcPosZero);
    အကယ်၍ (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  ပြန်ပေး applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::ldexp(const KnownFPClass &KnownSrc,
                                 const KnownBits &ExpBits,
                                 const fltSemantics &Flt, DenormalMode Mode) {
  ပြန်ပေး ldexp(KnownSrc, ExpBits.getSignedMinValue(),
               ExpBits.getSignedMaxValue(), Flt, Mode);
}

KnownFPClass KnownFPClass::pow(const KnownFPClass &KnownLHS,
                               const KnownFPClass &KnownRHS) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  အကယ်၍ (KnownLHS.isKnownNever(fcNan | fcNegNormal | fcNegSubnormal) &&
      KnownRHS.isKnownNeverNaN())
    Known.knownNot(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  အကယ်၍ (KnownLHS.isKnownNever(fcNegative)) {
    Known.knownNot(fcNegative);
  } မဟုတ်ပါက အကယ်၍ (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    အကယ်၍ ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcPosNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcNegNormal)))
      Known.knownNot(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    အကယ်၍ ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcNegNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosNormal)))
      Known.knownNot(fcNegInf);
  }

  ပြန်ပေး Known;
}

KnownFPClass KnownFPClass::powi(const KnownFPClass &KnownSrc,
                                const KnownBits &ExponentKnownBits) {
  KnownFPClass Known;
  Known.propagateNonNaN(KnownSrc);

  အကယ်၍ (ExponentKnownBits.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    အကယ်၍ (KnownSrc.isKnownNever(fcSNan)) {
      Known.knownNot(~fcPosNormal);
      ပြန်ပေး Known;
    }

    Known.knownNot(~(fcPosNormal | fcNan));
    ပြန်ပေး Known;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  အကယ်၍ (KnownSrc.isKnownNever(fcNegative) || ExponentKnownBits.isEven()) {
    Known.knownNot(fcNegative);
  } မဟုတ်ပါက အကယ်၍ (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    const bool IsKnownNeverOddPositive = ExponentKnownBits.isNegative();
    const bool IsKnownNeverOddNegative = ExponentKnownBits.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    အကယ်၍ ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddPositive) &&
        (KnownSrc.isKnownNever(fcNegInf) || IsKnownNeverOddNegative))
      Known.knownNot(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    အကယ်၍ ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddNegative) &&
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
    APInt MinExp = ExponentKnownBits.getSignedMinValue();
    APInt MaxExp = ExponentKnownBits.getSignedMaxValue();

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

    အကယ်၍ (!MayInfSrc && !MayDivByZero && !MayFiniteOverflow && !MaySubnormInv)
      Known.knownNot(fcInf);
  }

  ပြန်ပေး Known;
}
