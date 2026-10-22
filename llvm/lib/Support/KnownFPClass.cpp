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

#அந்தர்பாவ "llvm/Support/KnownFPClass.h"
#அந்தர்பாவ "llvm/ADT/APFloat.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ "llvm/Support/KnownBits.h"

using நாமாகாஶ llvm;

KnownFPClass::KnownFPClass(மாறிலி APFloat &C)
    : Known_FPClasses_மதிப்பு(C.classify()) {
  setSignBit(C.isNegative());
}

KnownFPClass KnownFPClass::apply_உள்ளீடு_Denormal_Mode(மாறிலி KnownFPClass &KnownSrc,
                                                  DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  தேர்வு (Mode.உள்ளீடு) {
  நிலை DenormalMode::IEEE:
    திரும்பு Known;
  நிலை DenormalMode::PreserveSign:
    எனில் (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    எனில் (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    திரும்பு Known;
  நிலை DenormalMode::PositiveZero:
    எனில் (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      எனில் (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    திரும்பு Known;
  இயல்பு:
    எனில் (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      எனில் (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    எனில் (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    திரும்பு Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

KnownFPClass KnownFPClass::applyOutputDenormalMode(மாறிலி KnownFPClass &KnownSrc,
                                                   DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  தேர்வு (Mode.Output) {
  நிலை DenormalMode::IEEE:
    திரும்பு Known;
  நிலை DenormalMode::PreserveSign:
    எனில் (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    எனில் (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    திரும்பு Known;
  நிலை DenormalMode::PositiveZero:
    எனில் (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      எனில் (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    திரும்பு Known;
  இயல்பு:
    எனில் (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      எனில் (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    எனில் (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    திரும்பு Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

தருக்க KnownFPClass::isKnownNeverLogicalZero(DenormalMode Mode) மாறிலி {
  திரும்பு apply_உள்ளீடு_Denormal_Mode(*இது, Mode).isKnownNeverZero();
}

தருக்க KnownFPClass::isKnownNeverLogicalNegZero(DenormalMode Mode) மாறிலி {
  திரும்பு apply_உள்ளீடு_Denormal_Mode(*இது, Mode).isKnownNeverNegZero();
}

தருக்க KnownFPClass::isKnownNeverLogicalPosZero(DenormalMode Mode) மாறிலி {
  திரும்பு apply_உள்ளீடு_Denormal_Mode(*இது, Mode).isKnownNeverPosZero();
}

KnownFPClass KnownFPClass::minMaxLike(மாறிலி KnownFPClass &KnownLHS_,
                                      மாறிலி KnownFPClass &KnownRHS_,
                                      MinMaxKind Kind, DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_உள்ளீடு_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_உள்ளீடு_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonNaN(KnownLHS, KnownRHS);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  எனில் ((KnownLHS.isKnownNeverNaN() || KnownRHS.isKnownNeverNaN()) &&
      (Kind == MinMaxKind::minnum || Kind == MinMaxKind::maxnum ||
       Kind == MinMaxKind::minimumnum || Kind == MinMaxKind::maximumnum))
    Known.knownNot(fcNan);

  தேர்வு (Kind) {
  நிலை MinMaxKind::maxnum: {
    எனில் (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/பொய்));
    எனில் (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/பொய்));
    நிறுத்து;
  }
  நிலை MinMaxKind::minnum: {
    எனில் (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/பொய்));
    எனில் (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/பொய்));
    நிறுத்து;
  }
  நிலை MinMaxKind::maximumnum: {
    எனில் (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/உண்மை));
    எனில் (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/உண்மை));
    நிறுத்து;
  }
  நிலை MinMaxKind::minimumnum: {
    எனில் (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/உண்மை));
    எனில் (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/உண்மை));
    நிறுத்து;
  }
  நிலை MinMaxKind::maximum: {
    Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/உண்மை) |
                   orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/உண்மை));
    நிறுத்து;
  }
  நிலை MinMaxKind::minimum: {
    Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/உண்மை) |
                   orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/உண்மை));
    நிறுத்து;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      KnownLHS.getKnownFPClasses() | KnownRHS.getKnownFPClasses() | fcNan;
  Known.knownNot(~PossibleClasses);

  திரும்பு applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::canonicalize(மாறிலி KnownFPClass &KnownSrc,
                                        DenormalMode DenormMode) {
  KnownFPClass Known;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  எனில் (KnownSrc.isKnownNeverNaN())
    Known.knownNot(fcNan);
  இல்லையெனில்
    Known.knownNot(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  எனில் (DenormMode == DenormalMode::getIEEE()) {
    எனில் (KnownSrc.isKnownNever(fcPosZero))
      Known.knownNot(fcPosZero);
    எனில் (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);
    திரும்பு Known;
  }

  எனில் (DenormMode.inputsAreZero() || DenormMode.outputsAreZero())
    Known.knownNot(fcSubnormal);

  எனில் (DenormMode == DenormalMode::getPreserveSign()) {
    எனில் (KnownSrc.isKnownNever(fcPosZero | fcPosSubnormal))
      Known.knownNot(fcPosZero);
    எனில் (KnownSrc.isKnownNever(fcNegZero | fcNegSubnormal))
      Known.knownNot(fcNegZero);
    திரும்பு Known;
  }

  எனில் (DenormMode.உள்ளீடு == DenormalMode::PositiveZero ||
      (DenormMode.Output == DenormalMode::PositiveZero &&
       DenormMode.உள்ளீடு == DenormalMode::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    எனில் (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    எனில் (KnownSrc.isKnownNever(fcPosZero | fcSubnormal))
      Known.knownNot(fcPosZero);
  }

  திரும்பு Known;
}

KnownFPClass KnownFPClass::bitcast(மாறிலி fltSemantics &FltSemantics,
                                   மாறிலி KnownBits &Bits) {
  ப்ரதிஜ்ஞா(FltSemantics.sizeInBits == Bits.getBitWidth() &&
         "Bitcast operand has incorrect bit width");
  KnownFPClass Known;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  எனில் (Bits.hasConflict())
    திரும்பு Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](மாறிலி fltSemantics &Semantics) {
    தேர்வு (APFloat::SemanticsToEnum(Semantics)) {
    நிலை APFloatBase::S_IEEEhalf:
    நிலை APFloatBase::S_BFloat:
    நிலை APFloatBase::S_IEEEsingle:
    நிலை APFloatBase::S_IEEEdouble:
    நிலை APFloatBase::S_IEEEquad:
    நிலை APFloatBase::S_x87DoubleExtended:
      திரும்பு உண்மை;
    இயல்பு:
      திரும்பு பொய்;
    }
  };
  எனில் (!IsSupported(FltSemantics))
    திரும்பு Known;

  // Transfer information from the sign bit.
  எனில் (Bits.isNonNegative())
    Known.signBitMustBeZero();
  இல்லையெனில் எனில் (Bits.isNegative())
    Known.signBitMustBeOne();

  தேர்வு (APFloatBase::SemanticsToEnum(FltSemantics)) {
  நிலை APFloatBase::S_IEEEhalf:
  நிலை APFloatBase::S_BFloat:
  நிலை APFloatBase::S_IEEEsingle:
  நிலை APFloatBase::S_IEEEdouble:
  நிலை APFloatBase::S_IEEEquad: {
    மாறிலி unsigned MantissaBits = FltSemantics.precision - 1;
    மாறிலி APInt ExponentMask = APInt::getBitsSet(
        FltSemantics.sizeInBits, MantissaBits, FltSemantics.sizeInBits - 1);
    மாறிலி APInt MantissaMask =
        APInt::get_கீழ்_Bits_Set(FltSemantics.sizeInBits, MantissaBits);

    மாறிலி தருக்க ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    மாறிலி தருக்க ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    மாறிலி தருக்க ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    மாறிலி தருக்க ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();

    மாறிலி தருக்க MantissaKnownAllZeros =
        (Bits.Zero & MantissaMask) == MantissaMask;
    மாறிலி தருக்க MantissaKnownNotAllZeros = !(Bits.One & MantissaMask).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    எனில் (ExponentKnownNotAllZeros)
      Known.knownNot(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    எனில் (ExponentKnownNotAllOnes)
      Known.knownNot(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    எனில் (ExponentKnownAllZeros || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // Zero and infinity require a mantissa with all zero bits.
    எனில் (MantissaKnownNotAllZeros)
      Known.knownNot(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    எனில் (MantissaKnownAllZeros)
      Known.knownNot(fcSubnormal | fcNan);

    மாறிலி தருக்க QuietBitKnownSet = Bits.One[MantissaBits - 1];
    மாறிலி தருக்க QuietBitKnownClear = Bits.Zero[MantissaBits - 1];

    எனில் (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    இல்லையெனில் எனில் (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    நிறுத்து;
  }
  நிலை APFloatBase::S_x87DoubleExtended: {
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
    மாறிலி APInt FractionMask = APInt::get_கீழ்_Bits_Set(BitWidth, FractionBits);
    மாறிலி APInt ExponentMask =
        APInt::getBitsSet(BitWidth, FractionBits + 1, BitWidth - 1);

    மாறிலி தருக்க ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    மாறிலி தருக்க ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    மாறிலி தருக்க ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    மாறிலி தருக்க ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();
    மாறிலி தருக்க FractionKnownAllZeros =
        (Bits.Zero & FractionMask) == FractionMask;
    மாறிலி தருக்க FractionKnownNotAllZeros = !(Bits.One & FractionMask).isZero();
    மாறிலி தருக்க IntegerBitKnownSet = Bits.One[IntegerBitIndex];
    மாறிலி தருக்க IntegerBitKnownClear = Bits.Zero[IntegerBitIndex];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    எனில் (ExponentKnownNotAllZeros || IntegerBitKnownSet)
      Known.knownNot(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    எனில் (FractionKnownNotAllZeros)
      Known.knownNot(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    எனில் (ExponentKnownNotAllOnes || IntegerBitKnownClear ||
        FractionKnownNotAllZeros)
      Known.knownNot(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    எனில் (IntegerBitKnownClear || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    எனில் (FractionKnownAllZeros)
      Known.knownNot(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    எனில் (ExponentKnownAllZeros ||
        (IntegerBitKnownSet &&
         (ExponentKnownNotAllOnes || FractionKnownAllZeros)))
      Known.knownNot(fcNan);

    மாறிலி தருக்க QuietBitKnownSet = Bits.One[FractionBits - 1];
    மாறிலி தருக்க QuietBitKnownClear = Bits.Zero[FractionBits - 1];

    எனில் (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    இல்லையெனில் எனில் (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    நிறுத்து;
  }
  இயல்பு:
    llvm_unreachable("unhandled supported type");
  }

  திரும்பு Known;
}

KnownBits KnownFPClass::toKnownBits(மாறிலி fltSemantics &FltSemantics) மாறிலி {
  KnownBits Known(FltSemantics.sizeInBits);
  மாறிலி FPClassTest FPClasses = getKnownFPClasses();

  // Return unknown if poison.
  எனில் (FPClasses == fcNone)
    திரும்பு Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](மாறிலி fltSemantics &Semantics) {
    தேர்வு (APFloat::SemanticsToEnum(Semantics)) {
    நிலை APFloatBase::S_IEEEhalf:
    நிலை APFloatBase::S_BFloat:
    நிலை APFloatBase::S_IEEEsingle:
    நிலை APFloatBase::S_IEEEdouble:
    நிலை APFloatBase::S_IEEEquad:
    நிலை APFloatBase::S_x87DoubleExtended:
      திரும்பு உண்மை;
    இயல்பு:
      திரும்பு பொய்;
    }
  };
  எனில் (!IsSupported(FltSemantics))
    திரும்பு Known;

  தேர்வு (APFloat::SemanticsToEnum(FltSemantics)) {
  நிலை APFloatBase::S_IEEEhalf:
  நிலை APFloatBase::S_BFloat:
  நிலை APFloatBase::S_IEEEsingle:
  நிலை APFloatBase::S_IEEEdouble:
  நிலை APFloatBase::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    எனில் (FPClasses & fcNormal)
      நிறுத்து;

    Known.setAllConflict();

    மாறிலி unsigned BitWidth = FltSemantics.sizeInBits;
    மாறிலி unsigned MantissaBits = FltSemantics.precision - 1;
    மாறிலி unsigned ExponentBits = BitWidth - MantissaBits - 1;

    APInt MantissaMask = APInt::get_கீழ்_Bits_Set(BitWidth, MantissaBits);
    APInt ExponentMask =
        APInt::getBitsSet(BitWidth, MantissaBits, MantissaBits + ExponentBits);

    மாறிலி unsigned QuietBitIndex = MantissaBits - 1;
    APInt PayloadMask = MantissaMask;
    PayloadMask.clearBit(QuietBitIndex);

    எனில் (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // No individual payload bit is known.
      Known.Zero &= ~PayloadMask;
      Known.One &= ~PayloadMask;

      எனில் (FPClasses & fcQNan)
        Known.Zero.clearBit(QuietBitIndex);
      எனில் (FPClasses & fcSNan)
        Known.One.clearBit(QuietBitIndex);
    }
    எனில் (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // Mantissa bits cannot be ones.
      Known.One &= ~MantissaMask;
    }
    எனில் (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Unknown mantissa.
      Known.One &= ~MantissaMask;
      Known.Zero &= ~MantissaMask;
    }
    எனில் (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Mantissa cannot be ones.
      Known.One &= ~MantissaMask;
    }

    நிறுத்து;
  }
  நிலை APFloatBase::S_x87DoubleExtended: {
    எனில் (!isKnownNever(fcNormal | fcSubnormal | fcNan))
      நிறுத்து;

    Known.setAllConflict();

    எனில் (FPClasses & fcInf)
      Known = Known.intersectWith(KnownBits::makeConstant(
          APFloat::getInf(FltSemantics).bitcastToAPInt()));

    எனில் (FPClasses & fcZero)
      Known = Known.intersectWith(
          KnownBits::makeConstant(APInt::getZero(FltSemantics.sizeInBits)));

    நிறுத்து;
  }
  இயல்பு:
    llvm_unreachable("unhandled supported semantics");
  }

  Known.Zero.clearSignBit();
  Known.One.clearSignBit();
  எனில் (std::optional<தருக்க> Sign = getSignBit()) {
    எனில் (*Sign)
      Known.makeNegative();
    இல்லையெனில்
      Known.makeNonNegative();
  }

  திரும்பு Known;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
நிலையான KnownFPClass fadd_impl(மாறிலி KnownFPClass &KnownLHS,
                              மாறிலி KnownFPClass &KnownRHS, தருக்க IsFMA) {
  KnownFPClass Known;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  எனில் (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNever(fcPosInf) || KnownRHS.isKnownNever(fcNegInf)) &&
      (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosInf)))
    Known.knownNot(fcNan);

  எனில் (KnownLHS.cannotBeOrderedLessThanZero() &&
      KnownRHS.cannotBeOrderedLessThanZero()) {
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    எனில் (KnownLHS.isKnownNever(fcZero | fcPosSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcPosSubnormal))
      Known.knownNot(fcZero | fcPosSubnormal);
  }

  எனில் (KnownLHS.cannotBeOrderedGreaterThanZero() &&
      KnownRHS.cannotBeOrderedGreaterThanZero()) {
    Known.knownNot(KnownFPClass::OrderedGreaterThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    எனில் (KnownLHS.isKnownNever(fcZero | fcNegSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcNegSubnormal))
      Known.knownNot(fcZero | fcNegSubnormal);
  }

  எனில் (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    எனில் (KnownLHS.isKnownNever(fcNegZero) || KnownRHS.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    Known.propagateNonSNaN(KnownLHS, KnownRHS);
  }

  திரும்பு Known;
}

KnownFPClass KnownFPClass::fadd(மாறிலி KnownFPClass &KnownLHS_,
                                மாறிலி KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_உள்ளீடு_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_உள்ளீடு_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/பொய்);

  திரும்பு applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fadd_self(மாறிலி KnownFPClass &KnownSrc_,
                                     DenormalMode Mode) {
  KnownFPClass KnownSrc = apply_உள்ளீடு_Denormal_Mode(KnownSrc_, Mode);

  KnownFPClass Known = fadd_impl(KnownSrc, KnownSrc, /*IsFMA=*/பொய்);

  எனில் (KnownSrc.isKnownNever(fcPosSubnormal))
    Known.knownNot(fcPosSubnormal);
  எனில் (KnownSrc.isKnownNever(fcNegSubnormal))
    Known.knownNot(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  எனில் (KnownSrc.isKnownNever(fcPosZero))
    Known.knownNot(fcPosZero);

  திரும்பு applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fsub(மாறிலி KnownFPClass &KnownLHS_,
                                மாறிலி KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_உள்ளீடு_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_உள்ளீடு_Denormal_Mode(KnownRHS_, Mode);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  KnownRHS.fneg();

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/பொய்);

  திரும்பு applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fmul(மாறிலி KnownFPClass &KnownLHS_,
                                மாறிலி KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_உள்ளீடு_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_உள்ளீடு_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  எனில் ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  எனில் ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Inf * Y => Inf or NaN
  எனில் (KnownLHS.isKnownAlways(fcInf | fcNan) ||
      KnownRHS.isKnownAlways(fcInf | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  எனில் (KnownRHS.isKnownAlways(fcZero | fcNan) ||
      KnownLHS.isKnownAlways(fcZero | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  எனில் (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownRHS.isKnownNeverInfinity() || KnownLHS.isKnownNeverZero()) &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverZero()))
    Known.knownNot(fcNan);

  திரும்பு applyOutputDenormalMode(Known, Mode);
}

// TODO: This generalizes to known ranges
KnownFPClass KnownFPClass::fmul(மாறிலி KnownFPClass &KnownLHS,
                                மாறிலி APFloat &CRHS, DenormalMode Mode) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  மாறிலி fltSemantics &Flt = CRHS.getSemantics();
  unsigned Precision = APFloat::semanticsPrecision(Flt);
  மாறிலி முழு MantissaBits = Precision - 1;

  முழு MinKnownExponent = ilogb(CRHS);
  தருக்க CannotBeSubnormal = (MinKnownExponent >= MantissaBits);

  KnownFPClass Known = KnownFPClass::fmul(KnownLHS, KnownFPClass(CRHS), Mode);
  எனில் (CannotBeSubnormal)
    Known.knownNot(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  எனில் (KnownLHS.isKnownNever(fcInf)) {
    எனில் (MinKnownExponent < 0)
      Known.knownNot(fcInf);
    இல்லையெனில் எனில் (MinKnownExponent == 0 && CRHS.compare_Absolute_மதிப்பு(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      Known.knownNot(fcInf);
  }

  திரும்பு Known;
}

KnownFPClass KnownFPClass::fdiv(மாறிலி KnownFPClass &KnownLHS_,
                                மாறிலி KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_உள்ளீடு_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_உள்ளீடு_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // Only 0/0, Inf/Inf produce NaN.
  எனில் (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverInfinity()) &&
      (KnownLHS.isKnownNeverZero() || KnownRHS.isKnownNeverZero())) {
    Known.knownNot(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  எனில் ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  எனில் ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Normal and subnormal results require two non-zero finite operands.
  எனில் ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)))
    Known.knownNot(fcNegNormal | fcNegSubnormal);
  எனில் ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosNormal | fcPosSubnormal);

  // 0 / X => 0 or NaN
  எனில் (KnownLHS.isKnownAlways(fcZero))
    Known.knownNot(fcSubnormal | fcNormal | fcInf);

  // X / 0 => NaN or Inf
  எனில் (KnownRHS.isKnownAlways(fcZero))
    Known.knownNot(fcFinite);

  திரும்பு applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fdiv_self(மாறிலி KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X / X is always exactly 1.0 or a NaN.
  KnownFPClass Known(fcNan | fcPosNormal);

  Known.propagateNonSNaN(KnownSrc);

  எனில் (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::frem(மாறிலி KnownFPClass &KnownLHS,
                                மாறிலி KnownFPClass &KnownRHS,
                                DenormalMode Mode) {
  KnownFPClass Known;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  எனில் (KnownLHS.isKnownNeverInfOrNaN() && KnownRHS.isKnownNeverNaN() &&
      KnownRHS.isKnownNeverLogicalZero(Mode)) {
    Known.knownNot(fcNan);
  }

  // The sign for frem is the same as the first operand.
  எனில் (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal | fcPosSubnormal);
  எனில் (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  எனில் (KnownLHS.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  எனில் (KnownLHS.isKnownNever(fcPosFinite) &&
      KnownLHS.isKnownNeverLogicalPosZero(Mode) &&
      (!Mode.outputsMayBePositiveZero() ||
       KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosZero);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::frem_self(மாறிலி KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X % X is always exactly [+-]0.0 or a NaN.
  KnownFPClass Known(fcNan | fcZero);

  Known.propagateNonSNaN(KnownSrc);

  எனில் (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  எனில் (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosZero);
  எனில் (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegZero);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::fma(மாறிலி KnownFPClass &KnownLHS,
                               மாறிலி KnownFPClass &KnownRHS,
                               மாறிலி KnownFPClass &KnownAddend,
                               DenormalMode Mode) {
  KnownFPClass Mul = fmul(KnownLHS, KnownRHS, Mode);

  // FMA differs from the base fmul + fadd handling only in the treatment of -0
  // results.
  //
  // If the multiply is a -0 due to rounding, the final -0 + 0 will be -0,
  // unlike for a separate fadd.
  KnownFPClass Known = fadd_impl(Mul, KnownAddend, /*IsFMA=*/உண்மை);

  // propagateNonSNaN for 3 arguments.
  எனில் (KnownLHS.isKnownNever(fcSNan) && KnownRHS.isKnownNever(fcSNan) &&
      KnownAddend.isKnownNever(fcSNan))
    Known.knownNot(fcSNan);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::fmuladd(மாறிலி KnownFPClass &KnownLHS,
                                   மாறிலி KnownFPClass &KnownRHS,
                                   மாறிலி KnownFPClass &KnownAddend,
                                   DenormalMode Mode) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  KnownFPClass FMA = fma(KnownLHS, KnownRHS, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(fmul(KnownLHS, KnownRHS, Mode), KnownAddend, Mode);
  திரும்பு FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::fma_square(மாறிலி KnownFPClass &KnownSquared,
                                      மாறிலி KnownFPClass &KnownAddend,
                                      DenormalMode Mode) {
  KnownFPClass Squared = square(KnownSquared, Mode);
  KnownFPClass Known = fadd_impl(Squared, KnownAddend, /*IsFMA=*/உண்மை);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  எனில் (KnownAddend.isKnownNever(fcNegInf | fcNan) && Squared.isKnownNever(fcNan))
    Known.knownNot(fcNan);

  Known.propagateNonSNaN(KnownSquared, KnownAddend);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::fmuladd_square(மாறிலி KnownFPClass &KnownSquared,
                                          மாறிலி KnownFPClass &KnownAddend,
                                          DenormalMode Mode) {
  KnownFPClass FMA = fma_square(KnownSquared, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(square(KnownSquared, Mode), KnownAddend, Mode);
  திரும்பு FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::exp(மாறிலி KnownFPClass &KnownSrc) {
  KnownFPClass Known;
  Known.knownNot(fcNegative);

  Known.propagateNonNaN(KnownSrc);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  எனில் (KnownSrc.isKnownNever(fcNegNormal | fcNegInf))
    Known.knownNot(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  எனில் (KnownSrc.isKnownNever(fcNegNormal))
    Known.knownNot(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  எனில் (KnownSrc.isKnownNever(fcPosNormal | fcPosInf))
    Known.knownNot(fcPosInf);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::log(மாறிலி KnownFPClass &KnownSrc, DenormalMode Mode,
                               தருக்க IsMultiUnitFPType) {
  KnownFPClass Known;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  எனில் (!IsMultiUnitFPType)
    Known.knownNot(fcNegZero | fcSubnormal);

  Known.propagateNonSNaN(KnownSrc);

  எனில் (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  எனில் (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  எனில் (KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNegInf);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::sqrt(மாறிலி KnownFPClass &KnownSrc,
                                DenormalMode Mode) {
  KnownFPClass Known;
  Known.knownNot(fcPosSubnormal);

  எனில் (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  Known.propagateNonSNaN(KnownSrc);

  // Any negative value besides -0 returns a nan.
  எனில் (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  Known.knownNot(fcNegInf | fcNegSubnormal | fcNegNormal);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  எனில் (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::sin(மாறிலி KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownSrc);

  // Return NaN on infinite inputs.
  Known.knownNot(fcInf);
  எனில் (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::cos(மாறிலி KnownFPClass &KnownSrc) {
  திரும்பு sin(KnownSrc);
}

KnownFPClass KnownFPClass::tan(மாறிலி KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. tan(+-Inf) is NaN.
  எனில் (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::sinh(மாறிலி KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  எனில் (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  எனில் (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal | fcPosInf);

  Known.propagateNonNaN(KnownSrc);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::cosh(மாறிலி KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  Known.knownNot(fcNegative | fcZero | fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::tanh(மாறிலி KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tanh is bounded to (-1, 1), never Inf.
  Known.knownNot(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  எனில் (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf)) {
    Known.knownNot(fcNegSubnormal | fcNegNormal);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    எனில் (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  எனில் (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  Known.propagateNonNaN(KnownSrc);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::asin(மாறிலி KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // asin is sign-preserving for finite arguments.
  எனில் (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegFinite);

  எனில் (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  திரும்பு Known;
}

KnownFPClass KnownFPClass::acos(மாறிலி KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // acos(x) is bounded to [0, pi] for -1 <= x <= 1, and is never negative,
  // infinite, or subnormal. The smallest non-zero value occurs when x is
  // close to 1.0, where acos(x) can be approximated by sqrt(2 * (1 - x)).
  // Since sqrt cannot produce a subnormal result, we can conclude that
  // acos(x) will also never produce a subnormal result.
  Known.knownNot(fcNegative | fcInf | fcSubnormal);

  // acos(x) == +0.0 iff x == +1.0
  எனில் (KnownSrc.isKnownNever(fcPosNormal))
    Known.knownNot(fcZero);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  திரும்பு Known;
}

KnownFPClass KnownFPClass::atan(மாறிலி KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  Known.knownNot(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  எனில் (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf))
    Known.knownNot(fcNegSubnormal | fcNegNormal);

  எனில் (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  எனில் (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal | fcNegZero))
    Known.knownNot(fcNegZero);

  Known.propagateNonNaN(KnownSrc);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::atan2(மாறிலி KnownFPClass &KnownY_,
                                 மாறிலி KnownFPClass &KnownX_,
                                 DenormalMode Mode) {
  KnownFPClass KnownY = apply_உள்ளீடு_Denormal_Mode(KnownY_, Mode);
  KnownFPClass KnownX = apply_உள்ளீடு_Denormal_Mode(KnownX_, Mode);
  KnownFPClass Known;

  // Even though these deductions are correct, we are ignoring the following
  // potentially erroneous cases:
  //   * atan2(y, inf) is not subnormal
  //   * atan2(inf, x) is not zero or subnormal

  // atan2 result is in (-pi, pi], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonNaN(KnownY, KnownX);

  // If x <= -0.0, then |atan2(y, x)| >= pi/2
  எனில் (KnownX.isKnownNever(fcPositive))
    Known.knownNot(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  எனில் (KnownY.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  எனில் (KnownY.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);

  திரும்பு applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fpext(மாறிலி KnownFPClass &KnownSrc,
                                 மாறிலி fltSemantics &DstTy,
                                 மாறிலி fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  KnownFPClass Known = KnownSrc;

  // All subnormal inputs should be in the normal range in the result type.
  எனில் (APFloat::isRepresentableAsNormalIn(SrcTy, DstTy)) {
    எனில் (Known.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosNormal);
    எனில் (Known.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegNormal);
    Known.knownNot(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  எனில் (!Known.isKnownNeverNaN())
    Known.setSignBit(std::nullopt);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::fptrunc(மாறிலி KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  எனில் (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

  Known.propagateNonNaN(KnownSrc);

  // Infinity needs a range check.
  திரும்பு Known;
}

KnownFPClass KnownFPClass::roundToIntegral(மாறிலி KnownFPClass &KnownSrc,
                                           தருக்க MayBeMultiUnitFPType,
                                           DenormalMode Mode) {
  KnownFPClass Known;

  // Integer results cannot be subnormal.
  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  எனில் (!MayBeMultiUnitFPType) {
    எனில் (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    எனில் (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  }

  எனில் (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  எனில் (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  // Negative round ups towards zero produce negative zero.
  எனில் (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // Negative subnormals may flush to positive zero.
  எனில் (KnownSrc.isKnownNever(fcPosFinite) &&
      (KnownSrc.isKnownNever(fcNegSubnormal) ||
       !Mode.inputsMayBePositiveZero()))
    Known.knownNot(fcPosZero);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::trunc(மாறிலி KnownFPClass &KnownSrc,
                                 DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc,
                                       /*MayBeMultiUnitFPType=*/உண்மை, Mode);

  எனில் (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);
  எனில் (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::floor(மாறிலி KnownFPClass &KnownSrc,
                                 தருக்க IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  எனில் (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::ceil(மாறிலி KnownFPClass &KnownSrc,
                                தருக்க IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  எனில் (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::frexp_mant(மாறிலி KnownFPClass &KnownSrc,
                                      DenormalMode Mode) {
  KnownFPClass Known;

  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  எனில் (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  எனில் (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  எனில் (KnownSrc.isKnownNeverLogicalPosZero(Mode))
    Known.knownNot(fcPosZero);

  எனில் (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  எனில் (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  எனில் (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  திரும்பு Known;
}

KnownFPClass KnownFPClass::ldexp(மாறிலி KnownFPClass &KnownSrc_,
                                 மாறிலி APInt &ConstantRangeExpMin,
                                 மாறிலி APInt &ConstantRangeExpMax,
                                 மாறிலி fltSemantics &Flt, DenormalMode Mode) {
  KnownFPClass Known;

  KnownFPClass KnownSrc = apply_உள்ளீடு_Denormal_Mode(KnownSrc_, Mode);

  எனில் (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcNan);
    Known.propagateNonNaN(KnownSrc);
    திரும்பு applyOutputDenormalMode(Known, Mode);
  }

  Known.propagateNonNaN(KnownSrc);

  // Sign is preserved, but underflows may produce zeroes.
  எனில் (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);
  இல்லையெனில் எனில் (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(OrderedLessThanZeroMask);

  எனில் (KnownSrc.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);
  இல்லையெனில் எனில் (KnownSrc.cannotBeOrderedGreaterThanZero())
    Known.knownNot(OrderedGreaterThanZeroMask);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  எனில் (&Flt != &APFloat::PPCDoubleDouble()) {
    unsigned Precision = APFloat::semanticsPrecision(Flt);
    மாறிலி முழு MantissaBits = Precision - 1;
    எனில் (ConstantRangeExpMin.sge(MantissaBits))
      Known.knownNot(fcSubnormal);
  }

  எனில் (ConstantRangeExpMax.isNonPositive()) {
    // If we know the power is <= 0, can't introduce inf
    எனில் (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    எனில் (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  } இல்லையெனில் எனில் (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    எனில் (KnownSrc.isKnownNeverPosSubnormal())
      Known.knownNot(fcPosSubnormal);
    எனில் (KnownSrc.isKnownNeverNegSubnormal())
      Known.knownNot(fcNegSubnormal);
    எனில் (KnownSrc.isKnownNeverPosZero())
      Known.knownNot(fcPosZero);
    எனில் (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  திரும்பு applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::ldexp(மாறிலி KnownFPClass &KnownSrc,
                                 மாறிலி KnownBits &ExpBits,
                                 மாறிலி fltSemantics &Flt, DenormalMode Mode) {
  திரும்பு ldexp(KnownSrc, ExpBits.get_Signed_Min_மதிப்பு(),
               ExpBits.get_Signed_Max_மதிப்பு(), Flt, Mode);
}

KnownFPClass KnownFPClass::pow(மாறிலி KnownFPClass &KnownLHS,
                               மாறிலி KnownFPClass &KnownRHS) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  எனில் (KnownLHS.isKnownNever(fcNan | fcNegNormal | fcNegSubnormal) &&
      KnownRHS.isKnownNeverNaN())
    Known.knownNot(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  எனில் (KnownLHS.isKnownNever(fcNegative)) {
    Known.knownNot(fcNegative);
  } இல்லையெனில் எனில் (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    எனில் ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcPosNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcNegNormal)))
      Known.knownNot(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    எனில் ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcNegNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosNormal)))
      Known.knownNot(fcNegInf);
  }

  திரும்பு Known;
}

KnownFPClass KnownFPClass::powi(மாறிலி KnownFPClass &KnownSrc,
                                மாறிலி KnownBits &ExponentKnownBits) {
  KnownFPClass Known;
  Known.propagateNonNaN(KnownSrc);

  எனில் (ExponentKnownBits.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    எனில் (KnownSrc.isKnownNever(fcSNan)) {
      Known.knownNot(~fcPosNormal);
      திரும்பு Known;
    }

    Known.knownNot(~(fcPosNormal | fcNan));
    திரும்பு Known;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  எனில் (KnownSrc.isKnownNever(fcNegative) || ExponentKnownBits.isEven()) {
    Known.knownNot(fcNegative);
  } இல்லையெனில் எனில் (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    மாறிலி தருக்க IsKnownNeverOddPositive = ExponentKnownBits.isNegative();
    மாறிலி தருக்க IsKnownNeverOddNegative = ExponentKnownBits.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    எனில் ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddPositive) &&
        (KnownSrc.isKnownNever(fcNegInf) || IsKnownNeverOddNegative))
      Known.knownNot(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    எனில் ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddNegative) &&
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
    APInt MinExp = ExponentKnownBits.get_Signed_Min_மதிப்பு();
    APInt MaxExp = ExponentKnownBits.get_Signed_Max_மதிப்பு();

    // powi(inf, exp), exp > 0
    தருக்க MayInfSrc =
        !KnownSrc.isKnownNever(fcInf) && MaxExp.isStrictlyPositive();

    // powi(+/-0, exp), exp < 0
    தருக்க MayDivByZero = !KnownSrc.isKnownNever(fcZero) && MinExp.isNegative();

    // powi(finite, exp), |exp| > 1
    தருக்க MayFinite = !KnownSrc.isKnownNever(fcNormal | fcSubnormal);
    தருக்க MayAbsExpGT1 = MinExp.slt(-1) || MaxExp.sgt(1);
    தருக்க MayFiniteOverflow = MayFinite && MayAbsExpGT1;

    // powi(subnormal, -1)
    தருக்க MayBeNegOne = ExponentKnownBits.Zero.isZero();
    தருக்க MaySubnormInv = !KnownSrc.isKnownNever(fcSubnormal) && MayBeNegOne;

    எனில் (!MayInfSrc && !MayDivByZero && !MayFiniteOverflow && !MaySubnormInv)
      Known.knownNot(fcInf);
  }

  திரும்பு Known;
}
