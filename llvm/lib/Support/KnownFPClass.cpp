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

using فضاینام llvm;

KnownFPClass::KnownFPClass(ثابت APFloat &C)
    : Known_FPClasses_مقدار(C.classify()) {
  setSignBit(C.isNegative());
}

KnownFPClass KnownFPClass::apply_درون_داد_Denormal_Mode(ثابت KnownFPClass &KnownSrc,
                                                  DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  انتخاب (Mode.درون_داد) {
  حالت DenormalMode::IEEE:
    بازگشت Known;
  حالت DenormalMode::PreserveSign:
    اگر (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    اگر (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    بازگشت Known;
  حالت DenormalMode::PositiveZero:
    اگر (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      اگر (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    بازگشت Known;
  پیشفرض:
    اگر (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      اگر (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    اگر (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    بازگشت Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

KnownFPClass KnownFPClass::applyOutputDenormalMode(ثابت KnownFPClass &KnownSrc,
                                                   DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  انتخاب (Mode.Output) {
  حالت DenormalMode::IEEE:
    بازگشت Known;
  حالت DenormalMode::PreserveSign:
    اگر (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    اگر (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    بازگشت Known;
  حالت DenormalMode::PositiveZero:
    اگر (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      اگر (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    بازگشت Known;
  پیشفرض:
    اگر (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      اگر (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    اگر (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    بازگشت Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

منطقی KnownFPClass::isKnownNeverLogicalZero(DenormalMode Mode) ثابت {
  بازگشت apply_درون_داد_Denormal_Mode(*این, Mode).isKnownNeverZero();
}

منطقی KnownFPClass::isKnownNeverLogicalNegZero(DenormalMode Mode) ثابت {
  بازگشت apply_درون_داد_Denormal_Mode(*این, Mode).isKnownNeverNegZero();
}

منطقی KnownFPClass::isKnownNeverLogicalPosZero(DenormalMode Mode) ثابت {
  بازگشت apply_درون_داد_Denormal_Mode(*این, Mode).isKnownNeverPosZero();
}

KnownFPClass KnownFPClass::minMaxLike(ثابت KnownFPClass &KnownLHS_,
                                      ثابت KnownFPClass &KnownRHS_,
                                      MinMaxKind Kind, DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_درون_داد_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_درون_داد_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonNaN(KnownLHS, KnownRHS);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  اگر ((KnownLHS.isKnownNeverNaN() || KnownRHS.isKnownNeverNaN()) &&
      (Kind == MinMaxKind::minnum || Kind == MinMaxKind::maxnum ||
       Kind == MinMaxKind::minimumnum || Kind == MinMaxKind::maximumnum))
    Known.knownNot(fcNan);

  انتخاب (Kind) {
  حالت MinMaxKind::maxnum: {
    اگر (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/نادرست));
    اگر (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/نادرست));
    بشکن;
  }
  حالت MinMaxKind::minnum: {
    اگر (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/نادرست));
    اگر (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/نادرست));
    بشکن;
  }
  حالت MinMaxKind::maximumnum: {
    اگر (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/درست));
    اگر (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/درست));
    بشکن;
  }
  حالت MinMaxKind::minimumnum: {
    اگر (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/درست));
    اگر (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/درست));
    بشکن;
  }
  حالت MinMaxKind::maximum: {
    Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/درست) |
                   orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/درست));
    بشکن;
  }
  حالت MinMaxKind::minimum: {
    Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/درست) |
                   orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/درست));
    بشکن;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      KnownLHS.getKnownFPClasses() | KnownRHS.getKnownFPClasses() | fcNan;
  Known.knownNot(~PossibleClasses);

  بازگشت applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::canonicalize(ثابت KnownFPClass &KnownSrc,
                                        DenormalMode DenormMode) {
  KnownFPClass Known;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  اگر (KnownSrc.isKnownNeverNaN())
    Known.knownNot(fcNan);
  وگرنه
    Known.knownNot(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  اگر (DenormMode == DenormalMode::getIEEE()) {
    اگر (KnownSrc.isKnownNever(fcPosZero))
      Known.knownNot(fcPosZero);
    اگر (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);
    بازگشت Known;
  }

  اگر (DenormMode.inputsAreZero() || DenormMode.outputsAreZero())
    Known.knownNot(fcSubnormal);

  اگر (DenormMode == DenormalMode::getPreserveSign()) {
    اگر (KnownSrc.isKnownNever(fcPosZero | fcPosSubnormal))
      Known.knownNot(fcPosZero);
    اگر (KnownSrc.isKnownNever(fcNegZero | fcNegSubnormal))
      Known.knownNot(fcNegZero);
    بازگشت Known;
  }

  اگر (DenormMode.درون_داد == DenormalMode::PositiveZero ||
      (DenormMode.Output == DenormalMode::PositiveZero &&
       DenormMode.درون_داد == DenormalMode::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    اگر (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    اگر (KnownSrc.isKnownNever(fcPosZero | fcSubnormal))
      Known.knownNot(fcPosZero);
  }

  بازگشت Known;
}

KnownFPClass KnownFPClass::bitcast(ثابت fltSemantics &FltSemantics,
                                   ثابت KnownBits &Bits) {
  أكد(FltSemantics.sizeInBits == Bits.getBitWidth() &&
         "Bitcast operand has incorrect bit width");
  KnownFPClass Known;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  اگر (Bits.hasConflict())
    بازگشت Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](ثابت fltSemantics &Semantics) {
    انتخاب (APFloat::SemanticsToEnum(Semantics)) {
    حالت APFloatBase::S_IEEEhalf:
    حالت APFloatBase::S_BFloat:
    حالت APFloatBase::S_IEEEsingle:
    حالت APFloatBase::S_IEEEdouble:
    حالت APFloatBase::S_IEEEquad:
    حالت APFloatBase::S_x87DoubleExtended:
      بازگشت درست;
    پیشفرض:
      بازگشت نادرست;
    }
  };
  اگر (!IsSupported(FltSemantics))
    بازگشت Known;

  // Transfer information from the sign bit.
  اگر (Bits.isNonNegative())
    Known.signBitMustBeZero();
  وگرنه اگر (Bits.isNegative())
    Known.signBitMustBeOne();

  انتخاب (APFloatBase::SemanticsToEnum(FltSemantics)) {
  حالت APFloatBase::S_IEEEhalf:
  حالت APFloatBase::S_BFloat:
  حالت APFloatBase::S_IEEEsingle:
  حالت APFloatBase::S_IEEEdouble:
  حالت APFloatBase::S_IEEEquad: {
    ثابت unsigned MantissaBits = FltSemantics.precision - 1;
    ثابت APInt ExponentMask = APInt::getBitsSet(
        FltSemantics.sizeInBits, MantissaBits, FltSemantics.sizeInBits - 1);
    ثابت APInt MantissaMask =
        APInt::get_کم_Bits_Set(FltSemantics.sizeInBits, MantissaBits);

    ثابت منطقی ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    ثابت منطقی ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    ثابت منطقی ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    ثابت منطقی ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();

    ثابت منطقی MantissaKnownAllZeros =
        (Bits.Zero & MantissaMask) == MantissaMask;
    ثابت منطقی MantissaKnownNotAllZeros = !(Bits.One & MantissaMask).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    اگر (ExponentKnownNotAllZeros)
      Known.knownNot(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    اگر (ExponentKnownNotAllOnes)
      Known.knownNot(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    اگر (ExponentKnownAllZeros || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // Zero and infinity require a mantissa with all zero bits.
    اگر (MantissaKnownNotAllZeros)
      Known.knownNot(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    اگر (MantissaKnownAllZeros)
      Known.knownNot(fcSubnormal | fcNan);

    ثابت منطقی QuietBitKnownSet = Bits.One[MantissaBits - 1];
    ثابت منطقی QuietBitKnownClear = Bits.Zero[MantissaBits - 1];

    اگر (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    وگرنه اگر (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    بشکن;
  }
  حالت APFloatBase::S_x87DoubleExtended: {
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
    ثابت APInt FractionMask = APInt::get_کم_Bits_Set(BitWidth, FractionBits);
    ثابت APInt ExponentMask =
        APInt::getBitsSet(BitWidth, FractionBits + 1, BitWidth - 1);

    ثابت منطقی ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    ثابت منطقی ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    ثابت منطقی ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    ثابت منطقی ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();
    ثابت منطقی FractionKnownAllZeros =
        (Bits.Zero & FractionMask) == FractionMask;
    ثابت منطقی FractionKnownNotAllZeros = !(Bits.One & FractionMask).isZero();
    ثابت منطقی IntegerBitKnownSet = Bits.One[IntegerBitIndex];
    ثابت منطقی IntegerBitKnownClear = Bits.Zero[IntegerBitIndex];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    اگر (ExponentKnownNotAllZeros || IntegerBitKnownSet)
      Known.knownNot(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    اگر (FractionKnownNotAllZeros)
      Known.knownNot(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    اگر (ExponentKnownNotAllOnes || IntegerBitKnownClear ||
        FractionKnownNotAllZeros)
      Known.knownNot(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    اگر (IntegerBitKnownClear || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    اگر (FractionKnownAllZeros)
      Known.knownNot(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    اگر (ExponentKnownAllZeros ||
        (IntegerBitKnownSet &&
         (ExponentKnownNotAllOnes || FractionKnownAllZeros)))
      Known.knownNot(fcNan);

    ثابت منطقی QuietBitKnownSet = Bits.One[FractionBits - 1];
    ثابت منطقی QuietBitKnownClear = Bits.Zero[FractionBits - 1];

    اگر (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    وگرنه اگر (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    بشکن;
  }
  پیشفرض:
    llvm_unreachable("unhandled supported type");
  }

  بازگشت Known;
}

KnownBits KnownFPClass::toKnownBits(ثابت fltSemantics &FltSemantics) ثابت {
  KnownBits Known(FltSemantics.sizeInBits);
  ثابت FPClassTest FPClasses = getKnownFPClasses();

  // Return unknown if poison.
  اگر (FPClasses == fcNone)
    بازگشت Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](ثابت fltSemantics &Semantics) {
    انتخاب (APFloat::SemanticsToEnum(Semantics)) {
    حالت APFloatBase::S_IEEEhalf:
    حالت APFloatBase::S_BFloat:
    حالت APFloatBase::S_IEEEsingle:
    حالت APFloatBase::S_IEEEdouble:
    حالت APFloatBase::S_IEEEquad:
    حالت APFloatBase::S_x87DoubleExtended:
      بازگشت درست;
    پیشفرض:
      بازگشت نادرست;
    }
  };
  اگر (!IsSupported(FltSemantics))
    بازگشت Known;

  انتخاب (APFloat::SemanticsToEnum(FltSemantics)) {
  حالت APFloatBase::S_IEEEhalf:
  حالت APFloatBase::S_BFloat:
  حالت APFloatBase::S_IEEEsingle:
  حالت APFloatBase::S_IEEEdouble:
  حالت APFloatBase::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    اگر (FPClasses & fcNormal)
      بشکن;

    Known.setAllConflict();

    ثابت unsigned BitWidth = FltSemantics.sizeInBits;
    ثابت unsigned MantissaBits = FltSemantics.precision - 1;
    ثابت unsigned ExponentBits = BitWidth - MantissaBits - 1;

    APInt MantissaMask = APInt::get_کم_Bits_Set(BitWidth, MantissaBits);
    APInt ExponentMask =
        APInt::getBitsSet(BitWidth, MantissaBits, MantissaBits + ExponentBits);

    ثابت unsigned QuietBitIndex = MantissaBits - 1;
    APInt PayloadMask = MantissaMask;
    PayloadMask.clearBit(QuietBitIndex);

    اگر (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // No individual payload bit is known.
      Known.Zero &= ~PayloadMask;
      Known.One &= ~PayloadMask;

      اگر (FPClasses & fcQNan)
        Known.Zero.clearBit(QuietBitIndex);
      اگر (FPClasses & fcSNan)
        Known.One.clearBit(QuietBitIndex);
    }
    اگر (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // Mantissa bits cannot be ones.
      Known.One &= ~MantissaMask;
    }
    اگر (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Unknown mantissa.
      Known.One &= ~MantissaMask;
      Known.Zero &= ~MantissaMask;
    }
    اگر (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Mantissa cannot be ones.
      Known.One &= ~MantissaMask;
    }

    بشکن;
  }
  حالت APFloatBase::S_x87DoubleExtended: {
    اگر (!isKnownNever(fcNormal | fcSubnormal | fcNan))
      بشکن;

    Known.setAllConflict();

    اگر (FPClasses & fcInf)
      Known = Known.intersectWith(KnownBits::makeConstant(
          APFloat::getInf(FltSemantics).bitcastToAPInt()));

    اگر (FPClasses & fcZero)
      Known = Known.intersectWith(
          KnownBits::makeConstant(APInt::getZero(FltSemantics.sizeInBits)));

    بشکن;
  }
  پیشفرض:
    llvm_unreachable("unhandled supported semantics");
  }

  Known.Zero.clearSignBit();
  Known.One.clearSignBit();
  اگر (std::optional<منطقی> Sign = getSignBit()) {
    اگر (*Sign)
      Known.makeNegative();
    وگرنه
      Known.makeNonNegative();
  }

  بازگشت Known;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
ایستا KnownFPClass fadd_impl(ثابت KnownFPClass &KnownLHS,
                              ثابت KnownFPClass &KnownRHS, منطقی IsFMA) {
  KnownFPClass Known;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  اگر (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNever(fcPosInf) || KnownRHS.isKnownNever(fcNegInf)) &&
      (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosInf)))
    Known.knownNot(fcNan);

  اگر (KnownLHS.cannotBeOrderedLessThanZero() &&
      KnownRHS.cannotBeOrderedLessThanZero()) {
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    اگر (KnownLHS.isKnownNever(fcZero | fcPosSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcPosSubnormal))
      Known.knownNot(fcZero | fcPosSubnormal);
  }

  اگر (KnownLHS.cannotBeOrderedGreaterThanZero() &&
      KnownRHS.cannotBeOrderedGreaterThanZero()) {
    Known.knownNot(KnownFPClass::OrderedGreaterThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    اگر (KnownLHS.isKnownNever(fcZero | fcNegSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcNegSubnormal))
      Known.knownNot(fcZero | fcNegSubnormal);
  }

  اگر (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    اگر (KnownLHS.isKnownNever(fcNegZero) || KnownRHS.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    Known.propagateNonSNaN(KnownLHS, KnownRHS);
  }

  بازگشت Known;
}

KnownFPClass KnownFPClass::fadd(ثابت KnownFPClass &KnownLHS_,
                                ثابت KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_درون_داد_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_درون_داد_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/نادرست);

  بازگشت applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fadd_self(ثابت KnownFPClass &KnownSrc_,
                                     DenormalMode Mode) {
  KnownFPClass KnownSrc = apply_درون_داد_Denormal_Mode(KnownSrc_, Mode);

  KnownFPClass Known = fadd_impl(KnownSrc, KnownSrc, /*IsFMA=*/نادرست);

  اگر (KnownSrc.isKnownNever(fcPosSubnormal))
    Known.knownNot(fcPosSubnormal);
  اگر (KnownSrc.isKnownNever(fcNegSubnormal))
    Known.knownNot(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  اگر (KnownSrc.isKnownNever(fcPosZero))
    Known.knownNot(fcPosZero);

  بازگشت applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fsub(ثابت KnownFPClass &KnownLHS_,
                                ثابت KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_درون_داد_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_درون_داد_Denormal_Mode(KnownRHS_, Mode);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  KnownRHS.fneg();

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/نادرست);

  بازگشت applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fmul(ثابت KnownFPClass &KnownLHS_,
                                ثابت KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_درون_داد_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_درون_داد_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  اگر ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  اگر ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Inf * Y => Inf or NaN
  اگر (KnownLHS.isKnownAlways(fcInf | fcNan) ||
      KnownRHS.isKnownAlways(fcInf | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  اگر (KnownRHS.isKnownAlways(fcZero | fcNan) ||
      KnownLHS.isKnownAlways(fcZero | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  اگر (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownRHS.isKnownNeverInfinity() || KnownLHS.isKnownNeverZero()) &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverZero()))
    Known.knownNot(fcNan);

  بازگشت applyOutputDenormalMode(Known, Mode);
}

// TODO: This generalizes to known ranges
KnownFPClass KnownFPClass::fmul(ثابت KnownFPClass &KnownLHS,
                                ثابت APFloat &CRHS, DenormalMode Mode) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  ثابت fltSemantics &Flt = CRHS.getSemantics();
  unsigned Precision = APFloat::semanticsPrecision(Flt);
  ثابت صحیح MantissaBits = Precision - 1;

  صحیح MinKnownExponent = ilogb(CRHS);
  منطقی CannotBeSubnormal = (MinKnownExponent >= MantissaBits);

  KnownFPClass Known = KnownFPClass::fmul(KnownLHS, KnownFPClass(CRHS), Mode);
  اگر (CannotBeSubnormal)
    Known.knownNot(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  اگر (KnownLHS.isKnownNever(fcInf)) {
    اگر (MinKnownExponent < 0)
      Known.knownNot(fcInf);
    وگرنه اگر (MinKnownExponent == 0 && CRHS.compare_Absolute_مقدار(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      Known.knownNot(fcInf);
  }

  بازگشت Known;
}

KnownFPClass KnownFPClass::fdiv(ثابت KnownFPClass &KnownLHS_,
                                ثابت KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_درون_داد_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_درون_داد_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // Only 0/0, Inf/Inf produce NaN.
  اگر (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverInfinity()) &&
      (KnownLHS.isKnownNeverZero() || KnownRHS.isKnownNeverZero())) {
    Known.knownNot(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  اگر ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  اگر ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Normal and subnormal results require two non-zero finite operands.
  اگر ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)))
    Known.knownNot(fcNegNormal | fcNegSubnormal);
  اگر ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosNormal | fcPosSubnormal);

  // 0 / X => 0 or NaN
  اگر (KnownLHS.isKnownAlways(fcZero))
    Known.knownNot(fcSubnormal | fcNormal | fcInf);

  // X / 0 => NaN or Inf
  اگر (KnownRHS.isKnownAlways(fcZero))
    Known.knownNot(fcFinite);

  بازگشت applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fdiv_self(ثابت KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X / X is always exactly 1.0 or a NaN.
  KnownFPClass Known(fcNan | fcPosNormal);

  Known.propagateNonSNaN(KnownSrc);

  اگر (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  بازگشت Known;
}

KnownFPClass KnownFPClass::frem(ثابت KnownFPClass &KnownLHS,
                                ثابت KnownFPClass &KnownRHS,
                                DenormalMode Mode) {
  KnownFPClass Known;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  اگر (KnownLHS.isKnownNeverInfOrNaN() && KnownRHS.isKnownNeverNaN() &&
      KnownRHS.isKnownNeverLogicalZero(Mode)) {
    Known.knownNot(fcNan);
  }

  // The sign for frem is the same as the first operand.
  اگر (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal | fcPosSubnormal);
  اگر (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  اگر (KnownLHS.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  اگر (KnownLHS.isKnownNever(fcPosFinite) &&
      KnownLHS.isKnownNeverLogicalPosZero(Mode) &&
      (!Mode.outputsMayBePositiveZero() ||
       KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosZero);

  بازگشت Known;
}

KnownFPClass KnownFPClass::frem_self(ثابت KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X % X is always exactly [+-]0.0 or a NaN.
  KnownFPClass Known(fcNan | fcZero);

  Known.propagateNonSNaN(KnownSrc);

  اگر (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  اگر (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosZero);
  اگر (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegZero);

  بازگشت Known;
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
  KnownFPClass Known = fadd_impl(Mul, KnownAddend, /*IsFMA=*/درست);

  // propagateNonSNaN for 3 arguments.
  اگر (KnownLHS.isKnownNever(fcSNan) && KnownRHS.isKnownNever(fcSNan) &&
      KnownAddend.isKnownNever(fcSNan))
    Known.knownNot(fcSNan);

  بازگشت Known;
}

KnownFPClass KnownFPClass::fmuladd(ثابت KnownFPClass &KnownLHS,
                                   ثابت KnownFPClass &KnownRHS,
                                   ثابت KnownFPClass &KnownAddend,
                                   DenormalMode Mode) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  KnownFPClass FMA = fma(KnownLHS, KnownRHS, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(fmul(KnownLHS, KnownRHS, Mode), KnownAddend, Mode);
  بازگشت FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::fma_square(ثابت KnownFPClass &KnownSquared,
                                      ثابت KnownFPClass &KnownAddend,
                                      DenormalMode Mode) {
  KnownFPClass Squared = square(KnownSquared, Mode);
  KnownFPClass Known = fadd_impl(Squared, KnownAddend, /*IsFMA=*/درست);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  اگر (KnownAddend.isKnownNever(fcNegInf | fcNan) && Squared.isKnownNever(fcNan))
    Known.knownNot(fcNan);

  Known.propagateNonSNaN(KnownSquared, KnownAddend);

  بازگشت Known;
}

KnownFPClass KnownFPClass::fmuladd_square(ثابت KnownFPClass &KnownSquared,
                                          ثابت KnownFPClass &KnownAddend,
                                          DenormalMode Mode) {
  KnownFPClass FMA = fma_square(KnownSquared, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(square(KnownSquared, Mode), KnownAddend, Mode);
  بازگشت FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::exp(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;
  Known.knownNot(fcNegative);

  Known.propagateNonNaN(KnownSrc);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  اگر (KnownSrc.isKnownNever(fcNegNormal | fcNegInf))
    Known.knownNot(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  اگر (KnownSrc.isKnownNever(fcNegNormal))
    Known.knownNot(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  اگر (KnownSrc.isKnownNever(fcPosNormal | fcPosInf))
    Known.knownNot(fcPosInf);

  بازگشت Known;
}

KnownFPClass KnownFPClass::log(ثابت KnownFPClass &KnownSrc, DenormalMode Mode,
                               منطقی IsMultiUnitFPType) {
  KnownFPClass Known;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  اگر (!IsMultiUnitFPType)
    Known.knownNot(fcNegZero | fcSubnormal);

  Known.propagateNonSNaN(KnownSrc);

  اگر (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  اگر (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  اگر (KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNegInf);

  بازگشت Known;
}

KnownFPClass KnownFPClass::sqrt(ثابت KnownFPClass &KnownSrc,
                                DenormalMode Mode) {
  KnownFPClass Known;
  Known.knownNot(fcPosSubnormal);

  اگر (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  Known.propagateNonSNaN(KnownSrc);

  // Any negative value besides -0 returns a nan.
  اگر (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  Known.knownNot(fcNegInf | fcNegSubnormal | fcNegNormal);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  اگر (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  بازگشت Known;
}

KnownFPClass KnownFPClass::sin(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownSrc);

  // Return NaN on infinite inputs.
  Known.knownNot(fcInf);
  اگر (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  بازگشت Known;
}

KnownFPClass KnownFPClass::cos(ثابت KnownFPClass &KnownSrc) {
  بازگشت sin(KnownSrc);
}

KnownFPClass KnownFPClass::tan(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. tan(+-Inf) is NaN.
  اگر (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  بازگشت Known;
}

KnownFPClass KnownFPClass::sinh(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  اگر (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  اگر (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal | fcPosInf);

  Known.propagateNonNaN(KnownSrc);

  بازگشت Known;
}

KnownFPClass KnownFPClass::cosh(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  Known.knownNot(fcNegative | fcZero | fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  بازگشت Known;
}

KnownFPClass KnownFPClass::tanh(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tanh is bounded to (-1, 1), never Inf.
  Known.knownNot(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  اگر (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf)) {
    Known.knownNot(fcNegSubnormal | fcNegNormal);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    اگر (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  اگر (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  Known.propagateNonNaN(KnownSrc);

  بازگشت Known;
}

KnownFPClass KnownFPClass::asin(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // asin is sign-preserving for finite arguments.
  اگر (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegFinite);

  اگر (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  بازگشت Known;
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
  اگر (KnownSrc.isKnownNever(fcPosNormal))
    Known.knownNot(fcZero);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  بازگشت Known;
}

KnownFPClass KnownFPClass::atan(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  Known.knownNot(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  اگر (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf))
    Known.knownNot(fcNegSubnormal | fcNegNormal);

  اگر (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  اگر (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal | fcNegZero))
    Known.knownNot(fcNegZero);

  Known.propagateNonNaN(KnownSrc);

  بازگشت Known;
}

KnownFPClass KnownFPClass::atan2(ثابت KnownFPClass &KnownY_,
                                 ثابت KnownFPClass &KnownX_,
                                 DenormalMode Mode) {
  KnownFPClass KnownY = apply_درون_داد_Denormal_Mode(KnownY_, Mode);
  KnownFPClass KnownX = apply_درون_داد_Denormal_Mode(KnownX_, Mode);
  KnownFPClass Known;

  // Even though these deductions are correct, we are ignoring the following
  // potentially erroneous cases:
  //   * atan2(y, inf) is not subnormal
  //   * atan2(inf, x) is not zero or subnormal

  // atan2 result is in (-pi, pi], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonNaN(KnownY, KnownX);

  // If x <= -0.0, then |atan2(y, x)| >= pi/2
  اگر (KnownX.isKnownNever(fcPositive))
    Known.knownNot(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  اگر (KnownY.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  اگر (KnownY.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);

  بازگشت applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fpext(ثابت KnownFPClass &KnownSrc,
                                 ثابت fltSemantics &DstTy,
                                 ثابت fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  KnownFPClass Known = KnownSrc;

  // All subnormal inputs should be in the normal range in the result type.
  اگر (APFloat::isRepresentableAsNormalIn(SrcTy, DstTy)) {
    اگر (Known.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosNormal);
    اگر (Known.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegNormal);
    Known.knownNot(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  اگر (!Known.isKnownNeverNaN())
    Known.setSignBit(std::nullopt);

  بازگشت Known;
}

KnownFPClass KnownFPClass::fptrunc(ثابت KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  اگر (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

  Known.propagateNonNaN(KnownSrc);

  // Infinity needs a range check.
  بازگشت Known;
}

KnownFPClass KnownFPClass::roundToIntegral(ثابت KnownFPClass &KnownSrc,
                                           منطقی MayBeMultiUnitFPType,
                                           DenormalMode Mode) {
  KnownFPClass Known;

  // Integer results cannot be subnormal.
  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  اگر (!MayBeMultiUnitFPType) {
    اگر (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    اگر (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  }

  اگر (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  اگر (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  // Negative round ups towards zero produce negative zero.
  اگر (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // Negative subnormals may flush to positive zero.
  اگر (KnownSrc.isKnownNever(fcPosFinite) &&
      (KnownSrc.isKnownNever(fcNegSubnormal) ||
       !Mode.inputsMayBePositiveZero()))
    Known.knownNot(fcPosZero);

  بازگشت Known;
}

KnownFPClass KnownFPClass::trunc(ثابت KnownFPClass &KnownSrc,
                                 DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc,
                                       /*MayBeMultiUnitFPType=*/درست, Mode);

  اگر (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);
  اگر (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  بازگشت Known;
}

KnownFPClass KnownFPClass::floor(ثابت KnownFPClass &KnownSrc,
                                 منطقی IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  اگر (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  بازگشت Known;
}

KnownFPClass KnownFPClass::ceil(ثابت KnownFPClass &KnownSrc,
                                منطقی IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  اگر (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  بازگشت Known;
}

KnownFPClass KnownFPClass::frexp_mant(ثابت KnownFPClass &KnownSrc,
                                      DenormalMode Mode) {
  KnownFPClass Known;

  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  اگر (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  اگر (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  اگر (KnownSrc.isKnownNeverLogicalPosZero(Mode))
    Known.knownNot(fcPosZero);

  اگر (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  اگر (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  اگر (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  بازگشت Known;
}

KnownFPClass KnownFPClass::ldexp(ثابت KnownFPClass &KnownSrc_,
                                 ثابت APInt &ConstantRangeExpMin,
                                 ثابت APInt &ConstantRangeExpMax,
                                 ثابت fltSemantics &Flt, DenormalMode Mode) {
  KnownFPClass Known;

  KnownFPClass KnownSrc = apply_درون_داد_Denormal_Mode(KnownSrc_, Mode);

  اگر (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcNan);
    Known.propagateNonNaN(KnownSrc);
    بازگشت applyOutputDenormalMode(Known, Mode);
  }

  Known.propagateNonNaN(KnownSrc);

  // Sign is preserved, but underflows may produce zeroes.
  اگر (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);
  وگرنه اگر (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(OrderedLessThanZeroMask);

  اگر (KnownSrc.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);
  وگرنه اگر (KnownSrc.cannotBeOrderedGreaterThanZero())
    Known.knownNot(OrderedGreaterThanZeroMask);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  اگر (&Flt != &APFloat::PPCDoubleDouble()) {
    unsigned Precision = APFloat::semanticsPrecision(Flt);
    ثابت صحیح MantissaBits = Precision - 1;
    اگر (ConstantRangeExpMin.sge(MantissaBits))
      Known.knownNot(fcSubnormal);
  }

  اگر (ConstantRangeExpMax.isNonPositive()) {
    // If we know the power is <= 0, can't introduce inf
    اگر (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    اگر (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  } وگرنه اگر (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    اگر (KnownSrc.isKnownNeverPosSubnormal())
      Known.knownNot(fcPosSubnormal);
    اگر (KnownSrc.isKnownNeverNegSubnormal())
      Known.knownNot(fcNegSubnormal);
    اگر (KnownSrc.isKnownNeverPosZero())
      Known.knownNot(fcPosZero);
    اگر (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  بازگشت applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::ldexp(ثابت KnownFPClass &KnownSrc,
                                 ثابت KnownBits &ExpBits,
                                 ثابت fltSemantics &Flt, DenormalMode Mode) {
  بازگشت ldexp(KnownSrc, ExpBits.get_Signed_Min_مقدار(),
               ExpBits.get_Signed_Max_مقدار(), Flt, Mode);
}

KnownFPClass KnownFPClass::pow(ثابت KnownFPClass &KnownLHS,
                               ثابت KnownFPClass &KnownRHS) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  اگر (KnownLHS.isKnownNever(fcNan | fcNegNormal | fcNegSubnormal) &&
      KnownRHS.isKnownNeverNaN())
    Known.knownNot(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  اگر (KnownLHS.isKnownNever(fcNegative)) {
    Known.knownNot(fcNegative);
  } وگرنه اگر (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    اگر ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcPosNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcNegNormal)))
      Known.knownNot(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    اگر ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcNegNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosNormal)))
      Known.knownNot(fcNegInf);
  }

  بازگشت Known;
}

KnownFPClass KnownFPClass::powi(ثابت KnownFPClass &KnownSrc,
                                ثابت KnownBits &ExponentKnownBits) {
  KnownFPClass Known;
  Known.propagateNonNaN(KnownSrc);

  اگر (ExponentKnownBits.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    اگر (KnownSrc.isKnownNever(fcSNan)) {
      Known.knownNot(~fcPosNormal);
      بازگشت Known;
    }

    Known.knownNot(~(fcPosNormal | fcNan));
    بازگشت Known;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  اگر (KnownSrc.isKnownNever(fcNegative) || ExponentKnownBits.isEven()) {
    Known.knownNot(fcNegative);
  } وگرنه اگر (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    ثابت منطقی IsKnownNeverOddPositive = ExponentKnownBits.isNegative();
    ثابت منطقی IsKnownNeverOddNegative = ExponentKnownBits.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    اگر ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddPositive) &&
        (KnownSrc.isKnownNever(fcNegInf) || IsKnownNeverOddNegative))
      Known.knownNot(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    اگر ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddNegative) &&
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
    APInt MinExp = ExponentKnownBits.get_Signed_Min_مقدار();
    APInt MaxExp = ExponentKnownBits.get_Signed_Max_مقدار();

    // powi(inf, exp), exp > 0
    منطقی MayInfSrc =
        !KnownSrc.isKnownNever(fcInf) && MaxExp.isStrictlyPositive();

    // powi(+/-0, exp), exp < 0
    منطقی MayDivByZero = !KnownSrc.isKnownNever(fcZero) && MinExp.isNegative();

    // powi(finite, exp), |exp| > 1
    منطقی MayFinite = !KnownSrc.isKnownNever(fcNormal | fcSubnormal);
    منطقی MayAbsExpGT1 = MinExp.slt(-1) || MaxExp.sgt(1);
    منطقی MayFiniteOverflow = MayFinite && MayAbsExpGT1;

    // powi(subnormal, -1)
    منطقی MayBeNegOne = ExponentKnownBits.Zero.isZero();
    منطقی MaySubnormInv = !KnownSrc.isKnownNever(fcSubnormal) && MayBeNegOne;

    اگر (!MayInfSrc && !MayDivByZero && !MayFiniteOverflow && !MaySubnormInv)
      Known.knownNot(fcInf);
  }

  بازگشت Known;
}
