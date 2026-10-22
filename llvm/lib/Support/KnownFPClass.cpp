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

#അന്തര്ഭാവ "llvm/Support/KnownFPClass.h"
#അന്തര്ഭാവ "llvm/ADT/APFloat.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
#അന്തര്ഭാവ "llvm/Support/KnownBits.h"

using നാമാകാശ llvm;

KnownFPClass::KnownFPClass(സ്ഥിരം APFloat &C)
    : Known_FPClasses_മൂല്യം(C.classify()) {
  setSignBit(C.isNegative());
}

KnownFPClass KnownFPClass::apply_നിവേശം_Denormal_Mode(സ്ഥിരം KnownFPClass &KnownSrc,
                                                  DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  തിരഞ്ഞെടുപ്പ് (Mode.നിവേശം) {
  അവസ്ഥ DenormalMode::IEEE:
    തിരികെ Known;
  അവസ്ഥ DenormalMode::PreserveSign:
    എങ്കിൽ (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    എങ്കിൽ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    തിരികെ Known;
  അവസ്ഥ DenormalMode::PositiveZero:
    എങ്കിൽ (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      എങ്കിൽ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    തിരികെ Known;
  സ്ഥിരസ്ഥിതി:
    എങ്കിൽ (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      എങ്കിൽ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    എങ്കിൽ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    തിരികെ Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

KnownFPClass KnownFPClass::applyOutputDenormalMode(സ്ഥിരം KnownFPClass &KnownSrc,
                                                   DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  തിരഞ്ഞെടുപ്പ് (Mode.Output) {
  അവസ്ഥ DenormalMode::IEEE:
    തിരികെ Known;
  അവസ്ഥ DenormalMode::PreserveSign:
    എങ്കിൽ (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    എങ്കിൽ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    തിരികെ Known;
  അവസ്ഥ DenormalMode::PositiveZero:
    എങ്കിൽ (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      എങ്കിൽ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    തിരികെ Known;
  സ്ഥിരസ്ഥിതി:
    എങ്കിൽ (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      എങ്കിൽ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    എങ്കിൽ (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    തിരികെ Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

തർക്കം KnownFPClass::isKnownNeverLogicalZero(DenormalMode Mode) സ്ഥിരം {
  തിരികെ apply_നിവേശം_Denormal_Mode(*ഇത്, Mode).isKnownNeverZero();
}

തർക്കം KnownFPClass::isKnownNeverLogicalNegZero(DenormalMode Mode) സ്ഥിരം {
  തിരികെ apply_നിവേശം_Denormal_Mode(*ഇത്, Mode).isKnownNeverNegZero();
}

തർക്കം KnownFPClass::isKnownNeverLogicalPosZero(DenormalMode Mode) സ്ഥിരം {
  തിരികെ apply_നിവേശം_Denormal_Mode(*ഇത്, Mode).isKnownNeverPosZero();
}

KnownFPClass KnownFPClass::minMaxLike(സ്ഥിരം KnownFPClass &KnownLHS_,
                                      സ്ഥിരം KnownFPClass &KnownRHS_,
                                      MinMaxKind Kind, DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_നിവേശം_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_നിവേശം_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonNaN(KnownLHS, KnownRHS);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  എങ്കിൽ ((KnownLHS.isKnownNeverNaN() || KnownRHS.isKnownNeverNaN()) &&
      (Kind == MinMaxKind::minnum || Kind == MinMaxKind::maxnum ||
       Kind == MinMaxKind::minimumnum || Kind == MinMaxKind::maximumnum))
    Known.knownNot(fcNan);

  തിരഞ്ഞെടുപ്പ് (Kind) {
  അവസ്ഥ MinMaxKind::maxnum: {
    എങ്കിൽ (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/അസത്യം));
    എങ്കിൽ (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/അസത്യം));
    നിർത്തുക;
  }
  അവസ്ഥ MinMaxKind::minnum: {
    എങ്കിൽ (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/അസത്യം));
    എങ്കിൽ (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/അസത്യം));
    നിർത്തുക;
  }
  അവസ്ഥ MinMaxKind::maximumnum: {
    എങ്കിൽ (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/സത്യം));
    എങ്കിൽ (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/സത്യം));
    നിർത്തുക;
  }
  അവസ്ഥ MinMaxKind::minimumnum: {
    എങ്കിൽ (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/സത്യം));
    എങ്കിൽ (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/സത്യം));
    നിർത്തുക;
  }
  അവസ്ഥ MinMaxKind::maximum: {
    Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/സത്യം) |
                   orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/സത്യം));
    നിർത്തുക;
  }
  അവസ്ഥ MinMaxKind::minimum: {
    Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/സത്യം) |
                   orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/സത്യം));
    നിർത്തുക;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      KnownLHS.getKnownFPClasses() | KnownRHS.getKnownFPClasses() | fcNan;
  Known.knownNot(~PossibleClasses);

  തിരികെ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::canonicalize(സ്ഥിരം KnownFPClass &KnownSrc,
                                        DenormalMode DenormMode) {
  KnownFPClass Known;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  എങ്കിൽ (KnownSrc.isKnownNeverNaN())
    Known.knownNot(fcNan);
  അല്ലെങ്കിൽ
    Known.knownNot(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  എങ്കിൽ (DenormMode == DenormalMode::getIEEE()) {
    എങ്കിൽ (KnownSrc.isKnownNever(fcPosZero))
      Known.knownNot(fcPosZero);
    എങ്കിൽ (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);
    തിരികെ Known;
  }

  എങ്കിൽ (DenormMode.inputsAreZero() || DenormMode.outputsAreZero())
    Known.knownNot(fcSubnormal);

  എങ്കിൽ (DenormMode == DenormalMode::getPreserveSign()) {
    എങ്കിൽ (KnownSrc.isKnownNever(fcPosZero | fcPosSubnormal))
      Known.knownNot(fcPosZero);
    എങ്കിൽ (KnownSrc.isKnownNever(fcNegZero | fcNegSubnormal))
      Known.knownNot(fcNegZero);
    തിരികെ Known;
  }

  എങ്കിൽ (DenormMode.നിവേശം == DenormalMode::PositiveZero ||
      (DenormMode.Output == DenormalMode::PositiveZero &&
       DenormMode.നിവേശം == DenormalMode::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    എങ്കിൽ (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    എങ്കിൽ (KnownSrc.isKnownNever(fcPosZero | fcSubnormal))
      Known.knownNot(fcPosZero);
  }

  തിരികെ Known;
}

KnownFPClass KnownFPClass::bitcast(സ്ഥിരം fltSemantics &FltSemantics,
                                   സ്ഥിരം KnownBits &Bits) {
  പ്രതിജ്ഞാ(FltSemantics.sizeInBits == Bits.getBitWidth() &&
         "Bitcast operand has incorrect bit width");
  KnownFPClass Known;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  എങ്കിൽ (Bits.hasConflict())
    തിരികെ Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](സ്ഥിരം fltSemantics &Semantics) {
    തിരഞ്ഞെടുപ്പ് (APFloat::SemanticsToEnum(Semantics)) {
    അവസ്ഥ APFloatBase::S_IEEEhalf:
    അവസ്ഥ APFloatBase::S_BFloat:
    അവസ്ഥ APFloatBase::S_IEEEsingle:
    അവസ്ഥ APFloatBase::S_IEEEdouble:
    അവസ്ഥ APFloatBase::S_IEEEquad:
    അവസ്ഥ APFloatBase::S_x87DoubleExtended:
      തിരികെ സത്യം;
    സ്ഥിരസ്ഥിതി:
      തിരികെ അസത്യം;
    }
  };
  എങ്കിൽ (!IsSupported(FltSemantics))
    തിരികെ Known;

  // Transfer information from the sign bit.
  എങ്കിൽ (Bits.isNonNegative())
    Known.signBitMustBeZero();
  അല്ലെങ്കിൽ എങ്കിൽ (Bits.isNegative())
    Known.signBitMustBeOne();

  തിരഞ്ഞെടുപ്പ് (APFloatBase::SemanticsToEnum(FltSemantics)) {
  അവസ്ഥ APFloatBase::S_IEEEhalf:
  അവസ്ഥ APFloatBase::S_BFloat:
  അവസ്ഥ APFloatBase::S_IEEEsingle:
  അവസ്ഥ APFloatBase::S_IEEEdouble:
  അവസ്ഥ APFloatBase::S_IEEEquad: {
    സ്ഥിരം unsigned MantissaBits = FltSemantics.precision - 1;
    സ്ഥിരം APInt ExponentMask = APInt::getBitsSet(
        FltSemantics.sizeInBits, MantissaBits, FltSemantics.sizeInBits - 1);
    സ്ഥിരം APInt MantissaMask =
        APInt::get_താഴെയുള്ള_Bits_Set(FltSemantics.sizeInBits, MantissaBits);

    സ്ഥിരം തർക്കം ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    സ്ഥിരം തർക്കം ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    സ്ഥിരം തർക്കം ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    സ്ഥിരം തർക്കം ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();

    സ്ഥിരം തർക്കം MantissaKnownAllZeros =
        (Bits.Zero & MantissaMask) == MantissaMask;
    സ്ഥിരം തർക്കം MantissaKnownNotAllZeros = !(Bits.One & MantissaMask).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    എങ്കിൽ (ExponentKnownNotAllZeros)
      Known.knownNot(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    എങ്കിൽ (ExponentKnownNotAllOnes)
      Known.knownNot(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    എങ്കിൽ (ExponentKnownAllZeros || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // Zero and infinity require a mantissa with all zero bits.
    എങ്കിൽ (MantissaKnownNotAllZeros)
      Known.knownNot(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    എങ്കിൽ (MantissaKnownAllZeros)
      Known.knownNot(fcSubnormal | fcNan);

    സ്ഥിരം തർക്കം QuietBitKnownSet = Bits.One[MantissaBits - 1];
    സ്ഥിരം തർക്കം QuietBitKnownClear = Bits.Zero[MantissaBits - 1];

    എങ്കിൽ (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    അല്ലെങ്കിൽ എങ്കിൽ (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    നിർത്തുക;
  }
  അവസ്ഥ APFloatBase::S_x87DoubleExtended: {
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
    സ്ഥിരം APInt FractionMask = APInt::get_താഴെയുള്ള_Bits_Set(BitWidth, FractionBits);
    സ്ഥിരം APInt ExponentMask =
        APInt::getBitsSet(BitWidth, FractionBits + 1, BitWidth - 1);

    സ്ഥിരം തർക്കം ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    സ്ഥിരം തർക്കം ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    സ്ഥിരം തർക്കം ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    സ്ഥിരം തർക്കം ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();
    സ്ഥിരം തർക്കം FractionKnownAllZeros =
        (Bits.Zero & FractionMask) == FractionMask;
    സ്ഥിരം തർക്കം FractionKnownNotAllZeros = !(Bits.One & FractionMask).isZero();
    സ്ഥിരം തർക്കം IntegerBitKnownSet = Bits.One[IntegerBitIndex];
    സ്ഥിരം തർക്കം IntegerBitKnownClear = Bits.Zero[IntegerBitIndex];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    എങ്കിൽ (ExponentKnownNotAllZeros || IntegerBitKnownSet)
      Known.knownNot(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    എങ്കിൽ (FractionKnownNotAllZeros)
      Known.knownNot(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    എങ്കിൽ (ExponentKnownNotAllOnes || IntegerBitKnownClear ||
        FractionKnownNotAllZeros)
      Known.knownNot(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    എങ്കിൽ (IntegerBitKnownClear || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    എങ്കിൽ (FractionKnownAllZeros)
      Known.knownNot(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    എങ്കിൽ (ExponentKnownAllZeros ||
        (IntegerBitKnownSet &&
         (ExponentKnownNotAllOnes || FractionKnownAllZeros)))
      Known.knownNot(fcNan);

    സ്ഥിരം തർക്കം QuietBitKnownSet = Bits.One[FractionBits - 1];
    സ്ഥിരം തർക്കം QuietBitKnownClear = Bits.Zero[FractionBits - 1];

    എങ്കിൽ (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    അല്ലെങ്കിൽ എങ്കിൽ (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    നിർത്തുക;
  }
  സ്ഥിരസ്ഥിതി:
    llvm_unreachable("unhandled supported type");
  }

  തിരികെ Known;
}

KnownBits KnownFPClass::toKnownBits(സ്ഥിരം fltSemantics &FltSemantics) സ്ഥിരം {
  KnownBits Known(FltSemantics.sizeInBits);
  സ്ഥിരം FPClassTest FPClasses = getKnownFPClasses();

  // Return unknown if poison.
  എങ്കിൽ (FPClasses == fcNone)
    തിരികെ Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](സ്ഥിരം fltSemantics &Semantics) {
    തിരഞ്ഞെടുപ്പ് (APFloat::SemanticsToEnum(Semantics)) {
    അവസ്ഥ APFloatBase::S_IEEEhalf:
    അവസ്ഥ APFloatBase::S_BFloat:
    അവസ്ഥ APFloatBase::S_IEEEsingle:
    അവസ്ഥ APFloatBase::S_IEEEdouble:
    അവസ്ഥ APFloatBase::S_IEEEquad:
    അവസ്ഥ APFloatBase::S_x87DoubleExtended:
      തിരികെ സത്യം;
    സ്ഥിരസ്ഥിതി:
      തിരികെ അസത്യം;
    }
  };
  എങ്കിൽ (!IsSupported(FltSemantics))
    തിരികെ Known;

  തിരഞ്ഞെടുപ്പ് (APFloat::SemanticsToEnum(FltSemantics)) {
  അവസ്ഥ APFloatBase::S_IEEEhalf:
  അവസ്ഥ APFloatBase::S_BFloat:
  അവസ്ഥ APFloatBase::S_IEEEsingle:
  അവസ്ഥ APFloatBase::S_IEEEdouble:
  അവസ്ഥ APFloatBase::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    എങ്കിൽ (FPClasses & fcNormal)
      നിർത്തുക;

    Known.setAllConflict();

    സ്ഥിരം unsigned BitWidth = FltSemantics.sizeInBits;
    സ്ഥിരം unsigned MantissaBits = FltSemantics.precision - 1;
    സ്ഥിരം unsigned ExponentBits = BitWidth - MantissaBits - 1;

    APInt MantissaMask = APInt::get_താഴെയുള്ള_Bits_Set(BitWidth, MantissaBits);
    APInt ExponentMask =
        APInt::getBitsSet(BitWidth, MantissaBits, MantissaBits + ExponentBits);

    സ്ഥിരം unsigned QuietBitIndex = MantissaBits - 1;
    APInt PayloadMask = MantissaMask;
    PayloadMask.clearBit(QuietBitIndex);

    എങ്കിൽ (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // No individual payload bit is known.
      Known.Zero &= ~PayloadMask;
      Known.One &= ~PayloadMask;

      എങ്കിൽ (FPClasses & fcQNan)
        Known.Zero.clearBit(QuietBitIndex);
      എങ്കിൽ (FPClasses & fcSNan)
        Known.One.clearBit(QuietBitIndex);
    }
    എങ്കിൽ (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // Mantissa bits cannot be ones.
      Known.One &= ~MantissaMask;
    }
    എങ്കിൽ (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Unknown mantissa.
      Known.One &= ~MantissaMask;
      Known.Zero &= ~MantissaMask;
    }
    എങ്കിൽ (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Mantissa cannot be ones.
      Known.One &= ~MantissaMask;
    }

    നിർത്തുക;
  }
  അവസ്ഥ APFloatBase::S_x87DoubleExtended: {
    എങ്കിൽ (!isKnownNever(fcNormal | fcSubnormal | fcNan))
      നിർത്തുക;

    Known.setAllConflict();

    എങ്കിൽ (FPClasses & fcInf)
      Known = Known.intersectWith(KnownBits::makeConstant(
          APFloat::getInf(FltSemantics).bitcastToAPInt()));

    എങ്കിൽ (FPClasses & fcZero)
      Known = Known.intersectWith(
          KnownBits::makeConstant(APInt::getZero(FltSemantics.sizeInBits)));

    നിർത്തുക;
  }
  സ്ഥിരസ്ഥിതി:
    llvm_unreachable("unhandled supported semantics");
  }

  Known.Zero.clearSignBit();
  Known.One.clearSignBit();
  എങ്കിൽ (std::optional<തർക്കം> Sign = getSignBit()) {
    എങ്കിൽ (*Sign)
      Known.makeNegative();
    അല്ലെങ്കിൽ
      Known.makeNonNegative();
  }

  തിരികെ Known;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
സ്ഥിതം KnownFPClass fadd_impl(സ്ഥിരം KnownFPClass &KnownLHS,
                              സ്ഥിരം KnownFPClass &KnownRHS, തർക്കം IsFMA) {
  KnownFPClass Known;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  എങ്കിൽ (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNever(fcPosInf) || KnownRHS.isKnownNever(fcNegInf)) &&
      (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosInf)))
    Known.knownNot(fcNan);

  എങ്കിൽ (KnownLHS.cannotBeOrderedLessThanZero() &&
      KnownRHS.cannotBeOrderedLessThanZero()) {
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    എങ്കിൽ (KnownLHS.isKnownNever(fcZero | fcPosSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcPosSubnormal))
      Known.knownNot(fcZero | fcPosSubnormal);
  }

  എങ്കിൽ (KnownLHS.cannotBeOrderedGreaterThanZero() &&
      KnownRHS.cannotBeOrderedGreaterThanZero()) {
    Known.knownNot(KnownFPClass::OrderedGreaterThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    എങ്കിൽ (KnownLHS.isKnownNever(fcZero | fcNegSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcNegSubnormal))
      Known.knownNot(fcZero | fcNegSubnormal);
  }

  എങ്കിൽ (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    എങ്കിൽ (KnownLHS.isKnownNever(fcNegZero) || KnownRHS.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    Known.propagateNonSNaN(KnownLHS, KnownRHS);
  }

  തിരികെ Known;
}

KnownFPClass KnownFPClass::fadd(സ്ഥിരം KnownFPClass &KnownLHS_,
                                സ്ഥിരം KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_നിവേശം_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_നിവേശം_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/അസത്യം);

  തിരികെ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fadd_self(സ്ഥിരം KnownFPClass &KnownSrc_,
                                     DenormalMode Mode) {
  KnownFPClass KnownSrc = apply_നിവേശം_Denormal_Mode(KnownSrc_, Mode);

  KnownFPClass Known = fadd_impl(KnownSrc, KnownSrc, /*IsFMA=*/അസത്യം);

  എങ്കിൽ (KnownSrc.isKnownNever(fcPosSubnormal))
    Known.knownNot(fcPosSubnormal);
  എങ്കിൽ (KnownSrc.isKnownNever(fcNegSubnormal))
    Known.knownNot(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  എങ്കിൽ (KnownSrc.isKnownNever(fcPosZero))
    Known.knownNot(fcPosZero);

  തിരികെ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fsub(സ്ഥിരം KnownFPClass &KnownLHS_,
                                സ്ഥിരം KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_നിവേശം_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_നിവേശം_Denormal_Mode(KnownRHS_, Mode);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  KnownRHS.fneg();

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/അസത്യം);

  തിരികെ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fmul(സ്ഥിരം KnownFPClass &KnownLHS_,
                                സ്ഥിരം KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_നിവേശം_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_നിവേശം_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  എങ്കിൽ ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  എങ്കിൽ ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Inf * Y => Inf or NaN
  എങ്കിൽ (KnownLHS.isKnownAlways(fcInf | fcNan) ||
      KnownRHS.isKnownAlways(fcInf | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  എങ്കിൽ (KnownRHS.isKnownAlways(fcZero | fcNan) ||
      KnownLHS.isKnownAlways(fcZero | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  എങ്കിൽ (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownRHS.isKnownNeverInfinity() || KnownLHS.isKnownNeverZero()) &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverZero()))
    Known.knownNot(fcNan);

  തിരികെ applyOutputDenormalMode(Known, Mode);
}

// TODO: This generalizes to known ranges
KnownFPClass KnownFPClass::fmul(സ്ഥിരം KnownFPClass &KnownLHS,
                                സ്ഥിരം APFloat &CRHS, DenormalMode Mode) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  സ്ഥിരം fltSemantics &Flt = CRHS.getSemantics();
  unsigned Precision = APFloat::semanticsPrecision(Flt);
  സ്ഥിരം പൂർണ്ണസംഖ്യ MantissaBits = Precision - 1;

  പൂർണ്ണസംഖ്യ MinKnownExponent = ilogb(CRHS);
  തർക്കം CannotBeSubnormal = (MinKnownExponent >= MantissaBits);

  KnownFPClass Known = KnownFPClass::fmul(KnownLHS, KnownFPClass(CRHS), Mode);
  എങ്കിൽ (CannotBeSubnormal)
    Known.knownNot(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  എങ്കിൽ (KnownLHS.isKnownNever(fcInf)) {
    എങ്കിൽ (MinKnownExponent < 0)
      Known.knownNot(fcInf);
    അല്ലെങ്കിൽ എങ്കിൽ (MinKnownExponent == 0 && CRHS.compare_Absolute_മൂല്യം(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      Known.knownNot(fcInf);
  }

  തിരികെ Known;
}

KnownFPClass KnownFPClass::fdiv(സ്ഥിരം KnownFPClass &KnownLHS_,
                                സ്ഥിരം KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_നിവേശം_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_നിവേശം_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // Only 0/0, Inf/Inf produce NaN.
  എങ്കിൽ (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverInfinity()) &&
      (KnownLHS.isKnownNeverZero() || KnownRHS.isKnownNeverZero())) {
    Known.knownNot(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  എങ്കിൽ ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  എങ്കിൽ ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Normal and subnormal results require two non-zero finite operands.
  എങ്കിൽ ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)))
    Known.knownNot(fcNegNormal | fcNegSubnormal);
  എങ്കിൽ ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosNormal | fcPosSubnormal);

  // 0 / X => 0 or NaN
  എങ്കിൽ (KnownLHS.isKnownAlways(fcZero))
    Known.knownNot(fcSubnormal | fcNormal | fcInf);

  // X / 0 => NaN or Inf
  എങ്കിൽ (KnownRHS.isKnownAlways(fcZero))
    Known.knownNot(fcFinite);

  തിരികെ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fdiv_self(സ്ഥിരം KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X / X is always exactly 1.0 or a NaN.
  KnownFPClass Known(fcNan | fcPosNormal);

  Known.propagateNonSNaN(KnownSrc);

  എങ്കിൽ (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::frem(സ്ഥിരം KnownFPClass &KnownLHS,
                                സ്ഥിരം KnownFPClass &KnownRHS,
                                DenormalMode Mode) {
  KnownFPClass Known;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  എങ്കിൽ (KnownLHS.isKnownNeverInfOrNaN() && KnownRHS.isKnownNeverNaN() &&
      KnownRHS.isKnownNeverLogicalZero(Mode)) {
    Known.knownNot(fcNan);
  }

  // The sign for frem is the same as the first operand.
  എങ്കിൽ (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal | fcPosSubnormal);
  എങ്കിൽ (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  എങ്കിൽ (KnownLHS.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  എങ്കിൽ (KnownLHS.isKnownNever(fcPosFinite) &&
      KnownLHS.isKnownNeverLogicalPosZero(Mode) &&
      (!Mode.outputsMayBePositiveZero() ||
       KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosZero);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::frem_self(സ്ഥിരം KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X % X is always exactly [+-]0.0 or a NaN.
  KnownFPClass Known(fcNan | fcZero);

  Known.propagateNonSNaN(KnownSrc);

  എങ്കിൽ (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  എങ്കിൽ (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosZero);
  എങ്കിൽ (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegZero);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::fma(സ്ഥിരം KnownFPClass &KnownLHS,
                               സ്ഥിരം KnownFPClass &KnownRHS,
                               സ്ഥിരം KnownFPClass &KnownAddend,
                               DenormalMode Mode) {
  KnownFPClass Mul = fmul(KnownLHS, KnownRHS, Mode);

  // FMA differs from the base fmul + fadd handling only in the treatment of -0
  // results.
  //
  // If the multiply is a -0 due to rounding, the final -0 + 0 will be -0,
  // unlike for a separate fadd.
  KnownFPClass Known = fadd_impl(Mul, KnownAddend, /*IsFMA=*/സത്യം);

  // propagateNonSNaN for 3 arguments.
  എങ്കിൽ (KnownLHS.isKnownNever(fcSNan) && KnownRHS.isKnownNever(fcSNan) &&
      KnownAddend.isKnownNever(fcSNan))
    Known.knownNot(fcSNan);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::fmuladd(സ്ഥിരം KnownFPClass &KnownLHS,
                                   സ്ഥിരം KnownFPClass &KnownRHS,
                                   സ്ഥിരം KnownFPClass &KnownAddend,
                                   DenormalMode Mode) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  KnownFPClass FMA = fma(KnownLHS, KnownRHS, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(fmul(KnownLHS, KnownRHS, Mode), KnownAddend, Mode);
  തിരികെ FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::fma_square(സ്ഥിരം KnownFPClass &KnownSquared,
                                      സ്ഥിരം KnownFPClass &KnownAddend,
                                      DenormalMode Mode) {
  KnownFPClass Squared = square(KnownSquared, Mode);
  KnownFPClass Known = fadd_impl(Squared, KnownAddend, /*IsFMA=*/സത്യം);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  എങ്കിൽ (KnownAddend.isKnownNever(fcNegInf | fcNan) && Squared.isKnownNever(fcNan))
    Known.knownNot(fcNan);

  Known.propagateNonSNaN(KnownSquared, KnownAddend);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::fmuladd_square(സ്ഥിരം KnownFPClass &KnownSquared,
                                          സ്ഥിരം KnownFPClass &KnownAddend,
                                          DenormalMode Mode) {
  KnownFPClass FMA = fma_square(KnownSquared, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(square(KnownSquared, Mode), KnownAddend, Mode);
  തിരികെ FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::exp(സ്ഥിരം KnownFPClass &KnownSrc) {
  KnownFPClass Known;
  Known.knownNot(fcNegative);

  Known.propagateNonNaN(KnownSrc);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  എങ്കിൽ (KnownSrc.isKnownNever(fcNegNormal | fcNegInf))
    Known.knownNot(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  എങ്കിൽ (KnownSrc.isKnownNever(fcNegNormal))
    Known.knownNot(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  എങ്കിൽ (KnownSrc.isKnownNever(fcPosNormal | fcPosInf))
    Known.knownNot(fcPosInf);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::log(സ്ഥിരം KnownFPClass &KnownSrc, DenormalMode Mode,
                               തർക്കം IsMultiUnitFPType) {
  KnownFPClass Known;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  എങ്കിൽ (!IsMultiUnitFPType)
    Known.knownNot(fcNegZero | fcSubnormal);

  Known.propagateNonSNaN(KnownSrc);

  എങ്കിൽ (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  എങ്കിൽ (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  എങ്കിൽ (KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNegInf);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::sqrt(സ്ഥിരം KnownFPClass &KnownSrc,
                                DenormalMode Mode) {
  KnownFPClass Known;
  Known.knownNot(fcPosSubnormal);

  എങ്കിൽ (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  Known.propagateNonSNaN(KnownSrc);

  // Any negative value besides -0 returns a nan.
  എങ്കിൽ (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  Known.knownNot(fcNegInf | fcNegSubnormal | fcNegNormal);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  എങ്കിൽ (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::sin(സ്ഥിരം KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownSrc);

  // Return NaN on infinite inputs.
  Known.knownNot(fcInf);
  എങ്കിൽ (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::cos(സ്ഥിരം KnownFPClass &KnownSrc) {
  തിരികെ sin(KnownSrc);
}

KnownFPClass KnownFPClass::tan(സ്ഥിരം KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. tan(+-Inf) is NaN.
  എങ്കിൽ (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::sinh(സ്ഥിരം KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  എങ്കിൽ (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  എങ്കിൽ (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal | fcPosInf);

  Known.propagateNonNaN(KnownSrc);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::cosh(സ്ഥിരം KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  Known.knownNot(fcNegative | fcZero | fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::tanh(സ്ഥിരം KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tanh is bounded to (-1, 1), never Inf.
  Known.knownNot(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  എങ്കിൽ (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf)) {
    Known.knownNot(fcNegSubnormal | fcNegNormal);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    എങ്കിൽ (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  എങ്കിൽ (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  Known.propagateNonNaN(KnownSrc);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::asin(സ്ഥിരം KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // asin is sign-preserving for finite arguments.
  എങ്കിൽ (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegFinite);

  എങ്കിൽ (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  തിരികെ Known;
}

KnownFPClass KnownFPClass::acos(സ്ഥിരം KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // acos(x) is bounded to [0, pi] for -1 <= x <= 1, and is never negative,
  // infinite, or subnormal. The smallest non-zero value occurs when x is
  // close to 1.0, where acos(x) can be approximated by sqrt(2 * (1 - x)).
  // Since sqrt cannot produce a subnormal result, we can conclude that
  // acos(x) will also never produce a subnormal result.
  Known.knownNot(fcNegative | fcInf | fcSubnormal);

  // acos(x) == +0.0 iff x == +1.0
  എങ്കിൽ (KnownSrc.isKnownNever(fcPosNormal))
    Known.knownNot(fcZero);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  തിരികെ Known;
}

KnownFPClass KnownFPClass::atan(സ്ഥിരം KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  Known.knownNot(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  എങ്കിൽ (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf))
    Known.knownNot(fcNegSubnormal | fcNegNormal);

  എങ്കിൽ (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  എങ്കിൽ (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal | fcNegZero))
    Known.knownNot(fcNegZero);

  Known.propagateNonNaN(KnownSrc);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::atan2(സ്ഥിരം KnownFPClass &KnownY_,
                                 സ്ഥിരം KnownFPClass &KnownX_,
                                 DenormalMode Mode) {
  KnownFPClass KnownY = apply_നിവേശം_Denormal_Mode(KnownY_, Mode);
  KnownFPClass KnownX = apply_നിവേശം_Denormal_Mode(KnownX_, Mode);
  KnownFPClass Known;

  // Even though these deductions are correct, we are ignoring the following
  // potentially erroneous cases:
  //   * atan2(y, inf) is not subnormal
  //   * atan2(inf, x) is not zero or subnormal

  // atan2 result is in (-pi, pi], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonNaN(KnownY, KnownX);

  // If x <= -0.0, then |atan2(y, x)| >= pi/2
  എങ്കിൽ (KnownX.isKnownNever(fcPositive))
    Known.knownNot(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  എങ്കിൽ (KnownY.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  എങ്കിൽ (KnownY.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);

  തിരികെ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fpext(സ്ഥിരം KnownFPClass &KnownSrc,
                                 സ്ഥിരം fltSemantics &DstTy,
                                 സ്ഥിരം fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  KnownFPClass Known = KnownSrc;

  // All subnormal inputs should be in the normal range in the result type.
  എങ്കിൽ (APFloat::isRepresentableAsNormalIn(SrcTy, DstTy)) {
    എങ്കിൽ (Known.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosNormal);
    എങ്കിൽ (Known.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegNormal);
    Known.knownNot(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  എങ്കിൽ (!Known.isKnownNeverNaN())
    Known.setSignBit(std::nullopt);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::fptrunc(സ്ഥിരം KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  എങ്കിൽ (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

  Known.propagateNonNaN(KnownSrc);

  // Infinity needs a range check.
  തിരികെ Known;
}

KnownFPClass KnownFPClass::roundToIntegral(സ്ഥിരം KnownFPClass &KnownSrc,
                                           തർക്കം MayBeMultiUnitFPType,
                                           DenormalMode Mode) {
  KnownFPClass Known;

  // Integer results cannot be subnormal.
  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  എങ്കിൽ (!MayBeMultiUnitFPType) {
    എങ്കിൽ (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    എങ്കിൽ (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  }

  എങ്കിൽ (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  എങ്കിൽ (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  // Negative round ups towards zero produce negative zero.
  എങ്കിൽ (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // Negative subnormals may flush to positive zero.
  എങ്കിൽ (KnownSrc.isKnownNever(fcPosFinite) &&
      (KnownSrc.isKnownNever(fcNegSubnormal) ||
       !Mode.inputsMayBePositiveZero()))
    Known.knownNot(fcPosZero);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::trunc(സ്ഥിരം KnownFPClass &KnownSrc,
                                 DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc,
                                       /*MayBeMultiUnitFPType=*/സത്യം, Mode);

  എങ്കിൽ (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);
  എങ്കിൽ (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::floor(സ്ഥിരം KnownFPClass &KnownSrc,
                                 തർക്കം IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  എങ്കിൽ (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::ceil(സ്ഥിരം KnownFPClass &KnownSrc,
                                തർക്കം IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  എങ്കിൽ (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::frexp_mant(സ്ഥിരം KnownFPClass &KnownSrc,
                                      DenormalMode Mode) {
  KnownFPClass Known;

  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  എങ്കിൽ (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  എങ്കിൽ (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  എങ്കിൽ (KnownSrc.isKnownNeverLogicalPosZero(Mode))
    Known.knownNot(fcPosZero);

  എങ്കിൽ (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  എങ്കിൽ (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  എങ്കിൽ (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  തിരികെ Known;
}

KnownFPClass KnownFPClass::ldexp(സ്ഥിരം KnownFPClass &KnownSrc_,
                                 സ്ഥിരം APInt &ConstantRangeExpMin,
                                 സ്ഥിരം APInt &ConstantRangeExpMax,
                                 സ്ഥിരം fltSemantics &Flt, DenormalMode Mode) {
  KnownFPClass Known;

  KnownFPClass KnownSrc = apply_നിവേശം_Denormal_Mode(KnownSrc_, Mode);

  എങ്കിൽ (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcNan);
    Known.propagateNonNaN(KnownSrc);
    തിരികെ applyOutputDenormalMode(Known, Mode);
  }

  Known.propagateNonNaN(KnownSrc);

  // Sign is preserved, but underflows may produce zeroes.
  എങ്കിൽ (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);
  അല്ലെങ്കിൽ എങ്കിൽ (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(OrderedLessThanZeroMask);

  എങ്കിൽ (KnownSrc.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);
  അല്ലെങ്കിൽ എങ്കിൽ (KnownSrc.cannotBeOrderedGreaterThanZero())
    Known.knownNot(OrderedGreaterThanZeroMask);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  എങ്കിൽ (&Flt != &APFloat::PPCDoubleDouble()) {
    unsigned Precision = APFloat::semanticsPrecision(Flt);
    സ്ഥിരം പൂർണ്ണസംഖ്യ MantissaBits = Precision - 1;
    എങ്കിൽ (ConstantRangeExpMin.sge(MantissaBits))
      Known.knownNot(fcSubnormal);
  }

  എങ്കിൽ (ConstantRangeExpMax.isNonPositive()) {
    // If we know the power is <= 0, can't introduce inf
    എങ്കിൽ (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    എങ്കിൽ (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  } അല്ലെങ്കിൽ എങ്കിൽ (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    എങ്കിൽ (KnownSrc.isKnownNeverPosSubnormal())
      Known.knownNot(fcPosSubnormal);
    എങ്കിൽ (KnownSrc.isKnownNeverNegSubnormal())
      Known.knownNot(fcNegSubnormal);
    എങ്കിൽ (KnownSrc.isKnownNeverPosZero())
      Known.knownNot(fcPosZero);
    എങ്കിൽ (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  തിരികെ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::ldexp(സ്ഥിരം KnownFPClass &KnownSrc,
                                 സ്ഥിരം KnownBits &ExpBits,
                                 സ്ഥിരം fltSemantics &Flt, DenormalMode Mode) {
  തിരികെ ldexp(KnownSrc, ExpBits.get_Signed_Min_മൂല്യം(),
               ExpBits.get_Signed_Max_മൂല്യം(), Flt, Mode);
}

KnownFPClass KnownFPClass::pow(സ്ഥിരം KnownFPClass &KnownLHS,
                               സ്ഥിരം KnownFPClass &KnownRHS) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  എങ്കിൽ (KnownLHS.isKnownNever(fcNan | fcNegNormal | fcNegSubnormal) &&
      KnownRHS.isKnownNeverNaN())
    Known.knownNot(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  എങ്കിൽ (KnownLHS.isKnownNever(fcNegative)) {
    Known.knownNot(fcNegative);
  } അല്ലെങ്കിൽ എങ്കിൽ (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    എങ്കിൽ ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcPosNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcNegNormal)))
      Known.knownNot(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    എങ്കിൽ ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcNegNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosNormal)))
      Known.knownNot(fcNegInf);
  }

  തിരികെ Known;
}

KnownFPClass KnownFPClass::powi(സ്ഥിരം KnownFPClass &KnownSrc,
                                സ്ഥിരം KnownBits &ExponentKnownBits) {
  KnownFPClass Known;
  Known.propagateNonNaN(KnownSrc);

  എങ്കിൽ (ExponentKnownBits.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    എങ്കിൽ (KnownSrc.isKnownNever(fcSNan)) {
      Known.knownNot(~fcPosNormal);
      തിരികെ Known;
    }

    Known.knownNot(~(fcPosNormal | fcNan));
    തിരികെ Known;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  എങ്കിൽ (KnownSrc.isKnownNever(fcNegative) || ExponentKnownBits.isEven()) {
    Known.knownNot(fcNegative);
  } അല്ലെങ്കിൽ എങ്കിൽ (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    സ്ഥിരം തർക്കം IsKnownNeverOddPositive = ExponentKnownBits.isNegative();
    സ്ഥിരം തർക്കം IsKnownNeverOddNegative = ExponentKnownBits.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    എങ്കിൽ ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddPositive) &&
        (KnownSrc.isKnownNever(fcNegInf) || IsKnownNeverOddNegative))
      Known.knownNot(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    എങ്കിൽ ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddNegative) &&
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
    APInt MinExp = ExponentKnownBits.get_Signed_Min_മൂല്യം();
    APInt MaxExp = ExponentKnownBits.get_Signed_Max_മൂല്യം();

    // powi(inf, exp), exp > 0
    തർക്കം MayInfSrc =
        !KnownSrc.isKnownNever(fcInf) && MaxExp.isStrictlyPositive();

    // powi(+/-0, exp), exp < 0
    തർക്കം MayDivByZero = !KnownSrc.isKnownNever(fcZero) && MinExp.isNegative();

    // powi(finite, exp), |exp| > 1
    തർക്കം MayFinite = !KnownSrc.isKnownNever(fcNormal | fcSubnormal);
    തർക്കം MayAbsExpGT1 = MinExp.slt(-1) || MaxExp.sgt(1);
    തർക്കം MayFiniteOverflow = MayFinite && MayAbsExpGT1;

    // powi(subnormal, -1)
    തർക്കം MayBeNegOne = ExponentKnownBits.Zero.isZero();
    തർക്കം MaySubnormInv = !KnownSrc.isKnownNever(fcSubnormal) && MayBeNegOne;

    എങ്കിൽ (!MayInfSrc && !MayDivByZero && !MayFiniteOverflow && !MaySubnormInv)
      Known.knownNot(fcInf);
  }

  തിരികെ Known;
}
