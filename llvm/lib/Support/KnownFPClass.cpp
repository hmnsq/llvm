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

using نطاق llvm;

KnownFPClass::KnownFPClass(ثابت APFloat &C)
    : KnownFPClassesValue(C.classify()) {
  setSignBit(C.isNegative());
}

KnownFPClass KnownFPClass::applyInputDenormalMode(ثابت KnownFPClass &KnownSrc,
                                                  DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  بدل (Mode.Input) {
  حالة DenormalMode::IEEE:
    أرجع Known;
  حالة DenormalMode::PreserveSign:
    𐤀𐤌 (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    𐤀𐤌 (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    أرجع Known;
  حالة DenormalMode::PositiveZero:
    𐤀𐤌 (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      𐤀𐤌 (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    أرجع Known;
  افتراضي:
    𐤀𐤌 (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      𐤀𐤌 (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    𐤀𐤌 (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    أرجع Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

KnownFPClass KnownFPClass::applyOutputDenormalMode(ثابت KnownFPClass &KnownSrc,
                                                   DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  بدل (Mode.Output) {
  حالة DenormalMode::IEEE:
    أرجع Known;
  حالة DenormalMode::PreserveSign:
    𐤀𐤌 (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    𐤀𐤌 (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    أرجع Known;
  حالة DenormalMode::PositiveZero:
    𐤀𐤌 (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      𐤀𐤌 (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    أرجع Known;
  افتراضي:
    𐤀𐤌 (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      𐤀𐤌 (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    𐤀𐤌 (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    أرجع Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

منطقي KnownFPClass::isKnownNeverLogicalZero(DenormalMode Mode) ثابت {
  أرجع applyInputDenormalMode(*هذا, Mode).isKnownNeverZero();
}

منطقي KnownFPClass::isKnownNeverLogicalNegZero(DenormalMode Mode) ثابت {
  أرجع applyInputDenormalMode(*هذا, Mode).isKnownNeverNegZero();
}

منطقي KnownFPClass::isKnownNeverLogicalPosZero(DenormalMode Mode) ثابت {
  أرجع applyInputDenormalMode(*هذا, Mode).isKnownNeverPosZero();
}

KnownFPClass KnownFPClass::minMaxLike(ثابت KnownFPClass &KnownLHS_,
                                      ثابت KnownFPClass &KnownRHS_,
                                      MinMaxKind Kind, DenormalMode Mode) {
  KnownFPClass KnownLHS = applyInputDenormalMode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = applyInputDenormalMode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonNaN(KnownLHS, KnownRHS);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  𐤀𐤌 ((KnownLHS.isKnownNeverNaN() || KnownRHS.isKnownNeverNaN()) &&
      (Kind == MinMaxKind::minnum || Kind == MinMaxKind::maxnum ||
       Kind == MinMaxKind::minimumnum || Kind == MinMaxKind::maximumnum))
    Known.knownNot(fcNan);

  بدل (Kind) {
  حالة MinMaxKind::maxnum: {
    𐤀𐤌 (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/خطأ));
    𐤀𐤌 (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/خطأ));
    اكسر;
  }
  حالة MinMaxKind::minnum: {
    𐤀𐤌 (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/خطأ));
    𐤀𐤌 (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/خطأ));
    اكسر;
  }
  حالة MinMaxKind::maximumnum: {
    𐤀𐤌 (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/true));
    𐤀𐤌 (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/true));
    اكسر;
  }
  حالة MinMaxKind::minimumnum: {
    𐤀𐤌 (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/true));
    𐤀𐤌 (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/true));
    اكسر;
  }
  حالة MinMaxKind::maximum: {
    Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/true) |
                   orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/true));
    اكسر;
  }
  حالة MinMaxKind::minimum: {
    Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/true) |
                   orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/true));
    اكسر;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      KnownLHS.getKnownFPClasses() | KnownRHS.getKnownFPClasses() | fcNan;
  Known.knownNot(~PossibleClasses);

  أرجع applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::canonicalize(ثابت KnownFPClass &KnownSrc,
                                        DenormalMode DenormMode) {
  KnownFPClass Known;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  𐤀𐤌 (KnownSrc.isKnownNeverNaN())
    Known.knownNot(fcNan);
  وإلا
    Known.knownNot(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  𐤀𐤌 (DenormMode == DenormalMode::getIEEE()) {
    𐤀𐤌 (KnownSrc.isKnownNever(fcPosZero))
      Known.knownNot(fcPosZero);
    𐤀𐤌 (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);
    أرجع Known;
  }

  𐤀𐤌 (DenormMode.inputsAreZero() || DenormMode.outputsAreZero())
    Known.knownNot(fcSubnormal);

  𐤀𐤌 (DenormMode == DenormalMode::getPreserveSign()) {
    𐤀𐤌 (KnownSrc.isKnownNever(fcPosZero | fcPosSubnormal))
      Known.knownNot(fcPosZero);
    𐤀𐤌 (KnownSrc.isKnownNever(fcNegZero | fcNegSubnormal))
      Known.knownNot(fcNegZero);
    أرجع Known;
  }

  𐤀𐤌 (DenormMode.Input == DenormalMode::PositiveZero ||
      (DenormMode.Output == DenormalMode::PositiveZero &&
       DenormMode.Input == DenormalMode::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    𐤀𐤌 (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    𐤀𐤌 (KnownSrc.isKnownNever(fcPosZero | fcSubnormal))
      Known.knownNot(fcPosZero);
  }

  أرجع Known;
}

KnownFPClass KnownFPClass::bitcast(ثابت fltSemantics &FltSemantics,
                                   ثابت KnownBits &Bits) {
  أكد(FltSemantics.sizeInBits == Bits.getBitWidth() &&
         "Bitcast operand has incorrect bit width");
  KnownFPClass Known;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  𐤀𐤌 (Bits.hasConflict())
    أرجع Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](ثابت fltSemantics &Semantics) {
    بدل (APFloat::SemanticsToEnum(Semantics)) {
    حالة APFloatBase::S_IEEEhalf:
    حالة APFloatBase::S_BFloat:
    حالة APFloatBase::S_IEEEsingle:
    حالة APFloatBase::S_IEEEdouble:
    حالة APFloatBase::S_IEEEquad:
    حالة APFloatBase::S_x87DoubleExtended:
      أرجع true;
    افتراضي:
      أرجع خطأ;
    }
  };
  𐤀𐤌 (!IsSupported(FltSemantics))
    أرجع Known;

  // Transfer information from the sign bit.
  𐤀𐤌 (Bits.isNonNegative())
    Known.signBitMustBeZero();
  وإلا 𐤀𐤌 (Bits.isNegative())
    Known.signBitMustBeOne();

  بدل (APFloatBase::SemanticsToEnum(FltSemantics)) {
  حالة APFloatBase::S_IEEEhalf:
  حالة APFloatBase::S_BFloat:
  حالة APFloatBase::S_IEEEsingle:
  حالة APFloatBase::S_IEEEdouble:
  حالة APFloatBase::S_IEEEquad: {
    ثابت unsigned MantissaBits = FltSemantics.precision - 1;
    ثابت APInt ExponentMask = APInt::getBitsSet(
        FltSemantics.sizeInBits, MantissaBits, FltSemantics.sizeInBits - 1);
    ثابت APInt MantissaMask =
        APInt::getLowBitsSet(FltSemantics.sizeInBits, MantissaBits);

    ثابت منطقي ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    ثابت منطقي ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    ثابت منطقي ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    ثابت منطقي ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();

    ثابت منطقي MantissaKnownAllZeros =
        (Bits.Zero & MantissaMask) == MantissaMask;
    ثابت منطقي MantissaKnownNotAllZeros = !(Bits.One & MantissaMask).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    𐤀𐤌 (ExponentKnownNotAllZeros)
      Known.knownNot(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    𐤀𐤌 (ExponentKnownNotAllOnes)
      Known.knownNot(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    𐤀𐤌 (ExponentKnownAllZeros || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // Zero and infinity require a mantissa with all zero bits.
    𐤀𐤌 (MantissaKnownNotAllZeros)
      Known.knownNot(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    𐤀𐤌 (MantissaKnownAllZeros)
      Known.knownNot(fcSubnormal | fcNan);

    ثابت منطقي QuietBitKnownSet = Bits.One[MantissaBits - 1];
    ثابت منطقي QuietBitKnownClear = Bits.Zero[MantissaBits - 1];

    𐤀𐤌 (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    وإلا 𐤀𐤌 (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    اكسر;
  }
  حالة APFloatBase::S_x87DoubleExtended: {
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
    ثابت APInt FractionMask = APInt::getLowBitsSet(BitWidth, FractionBits);
    ثابت APInt ExponentMask =
        APInt::getBitsSet(BitWidth, FractionBits + 1, BitWidth - 1);

    ثابت منطقي ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    ثابت منطقي ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    ثابت منطقي ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    ثابت منطقي ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();
    ثابت منطقي FractionKnownAllZeros =
        (Bits.Zero & FractionMask) == FractionMask;
    ثابت منطقي FractionKnownNotAllZeros = !(Bits.One & FractionMask).isZero();
    ثابت منطقي IntegerBitKnownSet = Bits.One[IntegerBitIndex];
    ثابت منطقي IntegerBitKnownClear = Bits.Zero[IntegerBitIndex];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    𐤀𐤌 (ExponentKnownNotAllZeros || IntegerBitKnownSet)
      Known.knownNot(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    𐤀𐤌 (FractionKnownNotAllZeros)
      Known.knownNot(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    𐤀𐤌 (ExponentKnownNotAllOnes || IntegerBitKnownClear ||
        FractionKnownNotAllZeros)
      Known.knownNot(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    𐤀𐤌 (IntegerBitKnownClear || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    𐤀𐤌 (FractionKnownAllZeros)
      Known.knownNot(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    𐤀𐤌 (ExponentKnownAllZeros ||
        (IntegerBitKnownSet &&
         (ExponentKnownNotAllOnes || FractionKnownAllZeros)))
      Known.knownNot(fcNan);

    ثابت منطقي QuietBitKnownSet = Bits.One[FractionBits - 1];
    ثابت منطقي QuietBitKnownClear = Bits.Zero[FractionBits - 1];

    𐤀𐤌 (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    وإلا 𐤀𐤌 (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    اكسر;
  }
  افتراضي:
    llvm_unreachable("unhandled supported type");
  }

  أرجع Known;
}

KnownBits KnownFPClass::toKnownBits(ثابت fltSemantics &FltSemantics) ثابت {
  KnownBits Known(FltSemantics.sizeInBits);
  ثابت FPClassTest FPClasses = getKnownFPClasses();

  // Return unknown if poison.
  𐤀𐤌 (FPClasses == fcNone)
    أرجع Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](ثابت fltSemantics &Semantics) {
    بدل (APFloat::SemanticsToEnum(Semantics)) {
    حالة APFloatBase::S_IEEEhalf:
    حالة APFloatBase::S_BFloat:
    حالة APFloatBase::S_IEEEsingle:
    حالة APFloatBase::S_IEEEdouble:
    حالة APFloatBase::S_IEEEquad:
    حالة APFloatBase::S_x87DoubleExtended:
      أرجع true;
    افتراضي:
      أرجع خطأ;
    }
  };
  𐤀𐤌 (!IsSupported(FltSemantics))
    أرجع Known;

  بدل (APFloat::SemanticsToEnum(FltSemantics)) {
  حالة APFloatBase::S_IEEEhalf:
  حالة APFloatBase::S_BFloat:
  حالة APFloatBase::S_IEEEsingle:
  حالة APFloatBase::S_IEEEdouble:
  حالة APFloatBase::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    𐤀𐤌 (FPClasses & fcNormal)
      اكسر;

    Known.setAllConflict();

    ثابت unsigned BitWidth = FltSemantics.sizeInBits;
    ثابت unsigned MantissaBits = FltSemantics.precision - 1;
    ثابت unsigned ExponentBits = BitWidth - MantissaBits - 1;

    APInt MantissaMask = APInt::getLowBitsSet(BitWidth, MantissaBits);
    APInt ExponentMask =
        APInt::getBitsSet(BitWidth, MantissaBits, MantissaBits + ExponentBits);

    ثابت unsigned QuietBitIndex = MantissaBits - 1;
    APInt PayloadMask = MantissaMask;
    PayloadMask.clearBit(QuietBitIndex);

    𐤀𐤌 (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // No individual payload bit is known.
      Known.Zero &= ~PayloadMask;
      Known.One &= ~PayloadMask;

      𐤀𐤌 (FPClasses & fcQNan)
        Known.Zero.clearBit(QuietBitIndex);
      𐤀𐤌 (FPClasses & fcSNan)
        Known.One.clearBit(QuietBitIndex);
    }
    𐤀𐤌 (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // Mantissa bits cannot be ones.
      Known.One &= ~MantissaMask;
    }
    𐤀𐤌 (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Unknown mantissa.
      Known.One &= ~MantissaMask;
      Known.Zero &= ~MantissaMask;
    }
    𐤀𐤌 (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Mantissa cannot be ones.
      Known.One &= ~MantissaMask;
    }

    اكسر;
  }
  حالة APFloatBase::S_x87DoubleExtended: {
    𐤀𐤌 (!isKnownNever(fcNormal | fcSubnormal | fcNan))
      اكسر;

    Known.setAllConflict();

    𐤀𐤌 (FPClasses & fcInf)
      Known = Known.intersectWith(KnownBits::makeConstant(
          APFloat::getInf(FltSemantics).bitcastToAPInt()));

    𐤀𐤌 (FPClasses & fcZero)
      Known = Known.intersectWith(
          KnownBits::makeConstant(APInt::getZero(FltSemantics.sizeInBits)));

    اكسر;
  }
  افتراضي:
    llvm_unreachable("unhandled supported semantics");
  }

  Known.Zero.clearSignBit();
  Known.One.clearSignBit();
  𐤀𐤌 (std::optional<منطقي> Sign = getSignBit()) {
    𐤀𐤌 (*Sign)
      Known.makeNegative();
    وإلا
      Known.makeNonNegative();
  }

  أرجع Known;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
ساكن KnownFPClass fadd_impl(ثابت KnownFPClass &KnownLHS,
                              ثابت KnownFPClass &KnownRHS, منطقي IsFMA) {
  KnownFPClass Known;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  𐤀𐤌 (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNever(fcPosInf) || KnownRHS.isKnownNever(fcNegInf)) &&
      (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosInf)))
    Known.knownNot(fcNan);

  𐤀𐤌 (KnownLHS.cannotBeOrderedLessThanZero() &&
      KnownRHS.cannotBeOrderedLessThanZero()) {
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    𐤀𐤌 (KnownLHS.isKnownNever(fcZero | fcPosSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcPosSubnormal))
      Known.knownNot(fcZero | fcPosSubnormal);
  }

  𐤀𐤌 (KnownLHS.cannotBeOrderedGreaterThanZero() &&
      KnownRHS.cannotBeOrderedGreaterThanZero()) {
    Known.knownNot(KnownFPClass::OrderedGreaterThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    𐤀𐤌 (KnownLHS.isKnownNever(fcZero | fcNegSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcNegSubnormal))
      Known.knownNot(fcZero | fcNegSubnormal);
  }

  𐤀𐤌 (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    𐤀𐤌 (KnownLHS.isKnownNever(fcNegZero) || KnownRHS.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    Known.propagateNonSNaN(KnownLHS, KnownRHS);
  }

  أرجع Known;
}

KnownFPClass KnownFPClass::fadd(ثابت KnownFPClass &KnownLHS_,
                                ثابت KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = applyInputDenormalMode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = applyInputDenormalMode(KnownRHS_, Mode);

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/خطأ);

  أرجع applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fadd_self(ثابت KnownFPClass &KnownSrc_,
                                     DenormalMode Mode) {
  KnownFPClass KnownSrc = applyInputDenormalMode(KnownSrc_, Mode);

  KnownFPClass Known = fadd_impl(KnownSrc, KnownSrc, /*IsFMA=*/خطأ);

  𐤀𐤌 (KnownSrc.isKnownNever(fcPosSubnormal))
    Known.knownNot(fcPosSubnormal);
  𐤀𐤌 (KnownSrc.isKnownNever(fcNegSubnormal))
    Known.knownNot(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  𐤀𐤌 (KnownSrc.isKnownNever(fcPosZero))
    Known.knownNot(fcPosZero);

  أرجع applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fsub(ثابت KnownFPClass &KnownLHS_,
                                ثابت KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = applyInputDenormalMode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = applyInputDenormalMode(KnownRHS_, Mode);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  KnownRHS.fneg();

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/خطأ);

  أرجع applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fmul(ثابت KnownFPClass &KnownLHS_,
                                ثابت KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = applyInputDenormalMode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = applyInputDenormalMode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  𐤀𐤌 ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  𐤀𐤌 ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Inf * Y => Inf or NaN
  𐤀𐤌 (KnownLHS.isKnownAlways(fcInf | fcNan) ||
      KnownRHS.isKnownAlways(fcInf | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  𐤀𐤌 (KnownRHS.isKnownAlways(fcZero | fcNan) ||
      KnownLHS.isKnownAlways(fcZero | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  𐤀𐤌 (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownRHS.isKnownNeverInfinity() || KnownLHS.isKnownNeverZero()) &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverZero()))
    Known.knownNot(fcNan);

  أرجع applyOutputDenormalMode(Known, Mode);
}

// TODO: This generalizes to known ranges
KnownFPClass KnownFPClass::fmul(ثابت KnownFPClass &KnownLHS,
                                ثابت APFloat &CRHS, DenormalMode Mode) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  ثابت fltSemantics &Flt = CRHS.getSemantics();
  unsigned Precision = APFloat::semanticsPrecision(Flt);
  ثابت صحيح MantissaBits = Precision - 1;

  صحيح MinKnownExponent = ilogb(CRHS);
  منطقي CannotBeSubnormal = (MinKnownExponent >= MantissaBits);

  KnownFPClass Known = KnownFPClass::fmul(KnownLHS, KnownFPClass(CRHS), Mode);
  𐤀𐤌 (CannotBeSubnormal)
    Known.knownNot(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  𐤀𐤌 (KnownLHS.isKnownNever(fcInf)) {
    𐤀𐤌 (MinKnownExponent < 0)
      Known.knownNot(fcInf);
    وإلا 𐤀𐤌 (MinKnownExponent == 0 && CRHS.compareAbsoluteValue(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      Known.knownNot(fcInf);
  }

  أرجع Known;
}

KnownFPClass KnownFPClass::fdiv(ثابت KnownFPClass &KnownLHS_,
                                ثابت KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = applyInputDenormalMode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = applyInputDenormalMode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // Only 0/0, Inf/Inf produce NaN.
  𐤀𐤌 (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverInfinity()) &&
      (KnownLHS.isKnownNeverZero() || KnownRHS.isKnownNeverZero())) {
    Known.knownNot(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  𐤀𐤌 ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  𐤀𐤌 ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Normal and subnormal results require two non-zero finite operands.
  𐤀𐤌 ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)))
    Known.knownNot(fcNegNormal | fcNegSubnormal);
  𐤀𐤌 ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosNormal | fcPosSubnormal);

  // 0 / X => 0 or NaN
  𐤀𐤌 (KnownLHS.isKnownAlways(fcZero))
    Known.knownNot(fcSubnormal | fcNormal | fcInf);

  // X / 0 => NaN or Inf
  𐤀𐤌 (KnownRHS.isKnownAlways(fcZero))
    Known.knownNot(fcFinite);

  أرجع applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fdiv_self(ثابت KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X / X is always exactly 1.0 or a NaN.
  KnownFPClass Known(fcNan | fcPosNormal);

  Known.propagateNonSNaN(KnownSrc);

  𐤀𐤌 (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  أرجع Known;
}

KnownFPClass KnownFPClass::frem(ثابت KnownFPClass &KnownLHS,
                                ثابت KnownFPClass &KnownRHS,
                                DenormalMode Mode) {
  KnownFPClass Known;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  𐤀𐤌 (KnownLHS.isKnownNeverInfOrNaN() && KnownRHS.isKnownNeverNaN() &&
      KnownRHS.isKnownNeverLogicalZero(Mode)) {
    Known.knownNot(fcNan);
  }

  // The sign for frem is the same as the first operand.
  𐤀𐤌 (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal | fcPosSubnormal);
  𐤀𐤌 (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  𐤀𐤌 (KnownLHS.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  𐤀𐤌 (KnownLHS.isKnownNever(fcPosFinite) &&
      KnownLHS.isKnownNeverLogicalPosZero(Mode) &&
      (!Mode.outputsMayBePositiveZero() ||
       KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosZero);

  أرجع Known;
}

KnownFPClass KnownFPClass::frem_self(ثابت KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X % X is always exactly [+-]0.0 or a NaN.
  KnownFPClass Known(fcNan | fcZero);

  Known.propagateNonSNaN(KnownSrc);

  𐤀𐤌 (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  𐤀𐤌 (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosZero);
  𐤀𐤌 (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegZero);

  أرجع Known;
}

KnownFPClass KnownFPClass::fma(ثابت KnownFPClass &KnownLHS,
                               ثابت KnownFPClass &KnownRHS,
                               ثابت KnownFPClass &KnownAddend,
                               DenormalMode Mode) {
  KnownFPClass Mul = fmul(KnownLHS, KnownRHS, Mode);

  // FMA differs from the base fmul + fadd handling only in the treatment of -0
  // results.
  //
  // If the multiply is a -0 due to rounding, the final -0 + 0 will be -0,
  // unlike for a separate fadd.
  KnownFPClass Known = fadd_impl(Mul, KnownAddend, /*IsFMA=*/true);

  // propagateNonSNaN for 3 arguments.
  𐤀𐤌 (KnownLHS.isKnownNever(fcSNan) && KnownRHS.isKnownNever(fcSNan) &&
      KnownAddend.isKnownNever(fcSNan))
    Known.knownNot(fcSNan);

  أرجع Known;
}

KnownFPClass KnownFPClass::fmuladd(ثابت KnownFPClass &KnownLHS,
                                   ثابت KnownFPClass &KnownRHS,
                                   ثابت KnownFPClass &KnownAddend,
                                   DenormalMode Mode) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  KnownFPClass FMA = fma(KnownLHS, KnownRHS, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(fmul(KnownLHS, KnownRHS, Mode), KnownAddend, Mode);
  أرجع FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::fma_square(ثابت KnownFPClass &KnownSquared,
                                      ثابت KnownFPClass &KnownAddend,
                                      DenormalMode Mode) {
  KnownFPClass Squared = square(KnownSquared, Mode);
  KnownFPClass Known = fadd_impl(Squared, KnownAddend, /*IsFMA=*/true);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  𐤀𐤌 (KnownAddend.isKnownNever(fcNegInf | fcNan) && Squared.isKnownNever(fcNan))
    Known.knownNot(fcNan);

  Known.propagateNonSNaN(KnownSquared, KnownAddend);

  أرجع Known;
}

KnownFPClass KnownFPClass::fmuladd_square(ثابت KnownFPClass &KnownSquared,
                                          ثابت KnownFPClass &KnownAddend,
                                          DenormalMode Mode) {
  KnownFPClass FMA = fma_square(KnownSquared, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(square(KnownSquared, Mode), KnownAddend, Mode);
  أرجع FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::exp(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;
  Known.knownNot(fcNegative);

  Known.propagateNonNaN(KnownSrc);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  𐤀𐤌 (KnownSrc.isKnownNever(fcNegNormal | fcNegInf))
    Known.knownNot(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  𐤀𐤌 (KnownSrc.isKnownNever(fcNegNormal))
    Known.knownNot(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  𐤀𐤌 (KnownSrc.isKnownNever(fcPosNormal | fcPosInf))
    Known.knownNot(fcPosInf);

  أرجع Known;
}

KnownFPClass KnownFPClass::log(ثابت KnownFPClass &KnownSrc, DenormalMode Mode,
                               منطقي IsMultiUnitFPType) {
  KnownFPClass Known;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  𐤀𐤌 (!IsMultiUnitFPType)
    Known.knownNot(fcNegZero | fcSubnormal);

  Known.propagateNonSNaN(KnownSrc);

  𐤀𐤌 (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  𐤀𐤌 (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  𐤀𐤌 (KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNegInf);

  أرجع Known;
}

KnownFPClass KnownFPClass::sqrt(ثابت KnownFPClass &KnownSrc,
                                DenormalMode Mode) {
  KnownFPClass Known;
  Known.knownNot(fcPosSubnormal);

  𐤀𐤌 (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  Known.propagateNonSNaN(KnownSrc);

  // Any negative value besides -0 returns a nan.
  𐤀𐤌 (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  Known.knownNot(fcNegInf | fcNegSubnormal | fcNegNormal);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  𐤀𐤌 (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  أرجع Known;
}

KnownFPClass KnownFPClass::sin(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownSrc);

  // Return NaN on infinite inputs.
  Known.knownNot(fcInf);
  𐤀𐤌 (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  أرجع Known;
}

KnownFPClass KnownFPClass::cos(ثابت KnownFPClass &KnownSrc) {
  أرجع sin(KnownSrc);
}

KnownFPClass KnownFPClass::tan(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. tan(+-Inf) is NaN.
  𐤀𐤌 (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  أرجع Known;
}

KnownFPClass KnownFPClass::sinh(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  𐤀𐤌 (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  𐤀𐤌 (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal | fcPosInf);

  Known.propagateNonNaN(KnownSrc);

  أرجع Known;
}

KnownFPClass KnownFPClass::cosh(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  Known.knownNot(fcNegative | fcZero | fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  أرجع Known;
}

KnownFPClass KnownFPClass::tanh(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tanh is bounded to (-1, 1), never Inf.
  Known.knownNot(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  𐤀𐤌 (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf)) {
    Known.knownNot(fcNegSubnormal | fcNegNormal);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    𐤀𐤌 (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  𐤀𐤌 (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  Known.propagateNonNaN(KnownSrc);

  أرجع Known;
}

KnownFPClass KnownFPClass::asin(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // asin is sign-preserving for finite arguments.
  𐤀𐤌 (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegFinite);

  𐤀𐤌 (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  أرجع Known;
}

KnownFPClass KnownFPClass::acos(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // acos(x) is bounded to [0, pi] for -1 <= x <= 1, and is never negative,
  // infinite, or subnormal. The smallest non-zero value occurs when x is
  // close to 1.0, where acos(x) can be approximated by sqrt(2 * (1 - x)).
  // Since sqrt cannot produce a subnormal result, we can conclude that
  // acos(x) will also never produce a subnormal result.
  Known.knownNot(fcNegative | fcInf | fcSubnormal);

  // acos(x) == +0.0 iff x == +1.0
  𐤀𐤌 (KnownSrc.isKnownNever(fcPosNormal))
    Known.knownNot(fcZero);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  أرجع Known;
}

KnownFPClass KnownFPClass::atan(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  Known.knownNot(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  𐤀𐤌 (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf))
    Known.knownNot(fcNegSubnormal | fcNegNormal);

  𐤀𐤌 (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  𐤀𐤌 (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal | fcNegZero))
    Known.knownNot(fcNegZero);

  Known.propagateNonNaN(KnownSrc);

  أرجع Known;
}

KnownFPClass KnownFPClass::atan2(ثابت KnownFPClass &KnownY_,
                                 ثابت KnownFPClass &KnownX_,
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
  𐤀𐤌 (KnownX.isKnownNever(fcPositive))
    Known.knownNot(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  𐤀𐤌 (KnownY.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  𐤀𐤌 (KnownY.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);

  أرجع applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fpext(ثابت KnownFPClass &KnownSrc,
                                 ثابت fltSemantics &DstTy,
                                 ثابت fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  KnownFPClass Known = KnownSrc;

  // All subnormal inputs should be in the normal range in the result type.
  𐤀𐤌 (APFloat::isRepresentableAsNormalIn(SrcTy, DstTy)) {
    𐤀𐤌 (Known.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosNormal);
    𐤀𐤌 (Known.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegNormal);
    Known.knownNot(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  𐤀𐤌 (!Known.isKnownNeverNaN())
    Known.setSignBit(std::nullopt);

  أرجع Known;
}

KnownFPClass KnownFPClass::fptrunc(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  𐤀𐤌 (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

  Known.propagateNonNaN(KnownSrc);

  // Infinity needs a range check.
  أرجع Known;
}

KnownFPClass KnownFPClass::roundToIntegral(ثابت KnownFPClass &KnownSrc,
                                           منطقي MayBeMultiUnitFPType,
                                           DenormalMode Mode) {
  KnownFPClass Known;

  // Integer results cannot be subnormal.
  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  𐤀𐤌 (!MayBeMultiUnitFPType) {
    𐤀𐤌 (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    𐤀𐤌 (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  }

  𐤀𐤌 (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  𐤀𐤌 (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  // Negative round ups towards zero produce negative zero.
  𐤀𐤌 (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // Negative subnormals may flush to positive zero.
  𐤀𐤌 (KnownSrc.isKnownNever(fcPosFinite) &&
      (KnownSrc.isKnownNever(fcNegSubnormal) ||
       !Mode.inputsMayBePositiveZero()))
    Known.knownNot(fcPosZero);

  أرجع Known;
}

KnownFPClass KnownFPClass::trunc(ثابت KnownFPClass &KnownSrc,
                                 DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc,
                                       /*MayBeMultiUnitFPType=*/true, Mode);

  𐤀𐤌 (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);
  𐤀𐤌 (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  أرجع Known;
}

KnownFPClass KnownFPClass::floor(ثابت KnownFPClass &KnownSrc,
                                 منطقي IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  𐤀𐤌 (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  أرجع Known;
}

KnownFPClass KnownFPClass::ceil(ثابت KnownFPClass &KnownSrc,
                                منطقي IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  𐤀𐤌 (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  أرجع Known;
}

KnownFPClass KnownFPClass::frexp_mant(ثابت KnownFPClass &KnownSrc,
                                      DenormalMode Mode) {
  KnownFPClass Known;

  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  𐤀𐤌 (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  𐤀𐤌 (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  𐤀𐤌 (KnownSrc.isKnownNeverLogicalPosZero(Mode))
    Known.knownNot(fcPosZero);

  𐤀𐤌 (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  𐤀𐤌 (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  𐤀𐤌 (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  أرجع Known;
}

KnownFPClass KnownFPClass::ldexp(ثابت KnownFPClass &KnownSrc_,
                                 ثابت APInt &ConstantRangeExpMin,
                                 ثابت APInt &ConstantRangeExpMax,
                                 ثابت fltSemantics &Flt, DenormalMode Mode) {
  KnownFPClass Known;

  KnownFPClass KnownSrc = applyInputDenormalMode(KnownSrc_, Mode);

  𐤀𐤌 (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcNan);
    Known.propagateNonNaN(KnownSrc);
    أرجع applyOutputDenormalMode(Known, Mode);
  }

  Known.propagateNonNaN(KnownSrc);

  // Sign is preserved, but underflows may produce zeroes.
  𐤀𐤌 (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);
  وإلا 𐤀𐤌 (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(OrderedLessThanZeroMask);

  𐤀𐤌 (KnownSrc.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);
  وإلا 𐤀𐤌 (KnownSrc.cannotBeOrderedGreaterThanZero())
    Known.knownNot(OrderedGreaterThanZeroMask);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  𐤀𐤌 (&Flt != &APFloat::PPCDoubleDouble()) {
    unsigned Precision = APFloat::semanticsPrecision(Flt);
    ثابت صحيح MantissaBits = Precision - 1;
    𐤀𐤌 (ConstantRangeExpMin.sge(MantissaBits))
      Known.knownNot(fcSubnormal);
  }

  𐤀𐤌 (ConstantRangeExpMax.isNonPositive()) {
    // If we know the power is <= 0, can't introduce inf
    𐤀𐤌 (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    𐤀𐤌 (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  } وإلا 𐤀𐤌 (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    𐤀𐤌 (KnownSrc.isKnownNeverPosSubnormal())
      Known.knownNot(fcPosSubnormal);
    𐤀𐤌 (KnownSrc.isKnownNeverNegSubnormal())
      Known.knownNot(fcNegSubnormal);
    𐤀𐤌 (KnownSrc.isKnownNeverPosZero())
      Known.knownNot(fcPosZero);
    𐤀𐤌 (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  أرجع applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::ldexp(ثابت KnownFPClass &KnownSrc,
                                 ثابت KnownBits &ExpBits,
                                 ثابت fltSemantics &Flt, DenormalMode Mode) {
  أرجع ldexp(KnownSrc, ExpBits.getSignedMinValue(),
               ExpBits.getSignedMaxValue(), Flt, Mode);
}

KnownFPClass KnownFPClass::pow(ثابت KnownFPClass &KnownLHS,
                               ثابت KnownFPClass &KnownRHS) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  𐤀𐤌 (KnownLHS.isKnownNever(fcNan | fcNegNormal | fcNegSubnormal) &&
      KnownRHS.isKnownNeverNaN())
    Known.knownNot(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  𐤀𐤌 (KnownLHS.isKnownNever(fcNegative)) {
    Known.knownNot(fcNegative);
  } وإلا 𐤀𐤌 (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    𐤀𐤌 ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcPosNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcNegNormal)))
      Known.knownNot(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    𐤀𐤌 ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcNegNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosNormal)))
      Known.knownNot(fcNegInf);
  }

  أرجع Known;
}

KnownFPClass KnownFPClass::powi(ثابت KnownFPClass &KnownSrc,
                                ثابت KnownBits &ExponentKnownBits) {
  KnownFPClass Known;
  Known.propagateNonNaN(KnownSrc);

  𐤀𐤌 (ExponentKnownBits.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    𐤀𐤌 (KnownSrc.isKnownNever(fcSNan)) {
      Known.knownNot(~fcPosNormal);
      أرجع Known;
    }

    Known.knownNot(~(fcPosNormal | fcNan));
    أرجع Known;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  𐤀𐤌 (KnownSrc.isKnownNever(fcNegative) || ExponentKnownBits.isEven()) {
    Known.knownNot(fcNegative);
  } وإلا 𐤀𐤌 (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    ثابت منطقي IsKnownNeverOddPositive = ExponentKnownBits.isNegative();
    ثابت منطقي IsKnownNeverOddNegative = ExponentKnownBits.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    𐤀𐤌 ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddPositive) &&
        (KnownSrc.isKnownNever(fcNegInf) || IsKnownNeverOddNegative))
      Known.knownNot(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    𐤀𐤌 ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddNegative) &&
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
    منطقي MayInfSrc =
        !KnownSrc.isKnownNever(fcInf) && MaxExp.isStrictlyPositive();

    // powi(+/-0, exp), exp < 0
    منطقي MayDivByZero = !KnownSrc.isKnownNever(fcZero) && MinExp.isNegative();

    // powi(finite, exp), |exp| > 1
    منطقي MayFinite = !KnownSrc.isKnownNever(fcNormal | fcSubnormal);
    منطقي MayAbsExpGT1 = MinExp.slt(-1) || MaxExp.sgt(1);
    منطقي MayFiniteOverflow = MayFinite && MayAbsExpGT1;

    // powi(subnormal, -1)
    منطقي MayBeNegOne = ExponentKnownBits.Zero.isZero();
    منطقي MaySubnormInv = !KnownSrc.isKnownNever(fcSubnormal) && MayBeNegOne;

    𐤀𐤌 (!MayInfSrc && !MayDivByZero && !MayFiniteOverflow && !MaySubnormInv)
      Known.knownNot(fcInf);
  }

  أرجع Known;
}
