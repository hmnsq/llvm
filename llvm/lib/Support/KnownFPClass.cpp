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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/KnownFPClass.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/APFloat.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/KnownBits.h"

using ନାମାକାଶ llvm;

KnownFPClass::KnownFPClass(ଅଚଳ APFloat &C)
    : Known_FPClasses_ମୂଲ୍ୟ(C.classify()) {
  setSignBit(C.isNegative());
}

KnownFPClass KnownFPClass::apply_ନିବେଶ_Denormal_Mode(ଅଚଳ KnownFPClass &KnownSrc,
                                                  DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  ବାଛ (Mode.ନିବେଶ) {
  ସ୍ଥିତି DenormalMode::IEEE:
    ଫେରନ୍ତୁ Known;
  ସ୍ଥିତି DenormalMode::PreserveSign:
    ଯଦି (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    ଯଦି (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    ଫେରନ୍ତୁ Known;
  ସ୍ଥିତି DenormalMode::PositiveZero:
    ଯଦି (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      ଯଦି (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    ଫେରନ୍ତୁ Known;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଯଦି (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      ଯଦି (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    ଯଦି (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    ଫେରନ୍ତୁ Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

KnownFPClass KnownFPClass::applyOutputDenormalMode(ଅଚଳ KnownFPClass &KnownSrc,
                                                   DenormalMode Mode) {
  KnownFPClass Known = KnownSrc;
  ବାଛ (Mode.Output) {
  ସ୍ଥିତି DenormalMode::IEEE:
    ଫେରନ୍ତୁ Known;
  ସ୍ଥିତି DenormalMode::PreserveSign:
    ଯଦି (KnownSrc.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
    ଯଦି (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    ଫେରନ୍ତୁ Known;
  ସ୍ଥିତି DenormalMode::PositiveZero:
    ଯଦି (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      ଯଦି (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    ଫେରନ୍ତୁ Known;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    ଯଦି (KnownSrc.getKnownFPClasses() & fcSubnormal) {
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosZero);
      ଯଦି (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
        Known.setSignBit(std::nullopt);
    }
    ଯଦି (KnownSrc.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegZero);
    ଫେରନ୍ତୁ Known;
  }

  llvm_unreachable("covered switch over denormal mode");
}

ତାର୍କିକ KnownFPClass::isKnownNeverLogicalZero(DenormalMode Mode) ଅଚଳ {
  ଫେରନ୍ତୁ apply_ନିବେଶ_Denormal_Mode(*ଏହି, Mode).isKnownNeverZero();
}

ତାର୍କିକ KnownFPClass::isKnownNeverLogicalNegZero(DenormalMode Mode) ଅଚଳ {
  ଫେରନ୍ତୁ apply_ନିବେଶ_Denormal_Mode(*ଏହି, Mode).isKnownNeverNegZero();
}

ତାର୍କିକ KnownFPClass::isKnownNeverLogicalPosZero(DenormalMode Mode) ଅଚଳ {
  ଫେରନ୍ତୁ apply_ନିବେଶ_Denormal_Mode(*ଏହି, Mode).isKnownNeverPosZero();
}

KnownFPClass KnownFPClass::minMaxLike(ଅଚଳ KnownFPClass &KnownLHS_,
                                      ଅଚଳ KnownFPClass &KnownRHS_,
                                      MinMaxKind Kind, DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_ନିବେଶ_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_ନିବେଶ_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonNaN(KnownLHS, KnownRHS);

  // The *num functions return the non-NaN operand if only one operand is NaN.
  // FIXME: minnum/maxnum may return a NaN if either operand is a signaling NaN.
  ଯଦି ((KnownLHS.isKnownNeverNaN() || KnownRHS.isKnownNeverNaN()) &&
      (Kind == MinMaxKind::minnum || Kind == MinMaxKind::maxnum ||
       Kind == MinMaxKind::minimumnum || Kind == MinMaxKind::maximumnum))
    Known.knownNot(fcNan);

  ବାଛ (Kind) {
  ସ୍ଥିତି MinMaxKind::maxnum: {
    ଯଦି (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/ମିଥ୍ୟା));
    ଯଦି (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/ମିଥ୍ୟା));
    ଭାଙ୍ଗ;
  }
  ସ୍ଥିତି MinMaxKind::minnum: {
    ଯଦି (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/ମିଥ୍ୟା));
    ଯଦି (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/ମିଥ୍ୟା));
    ଭାଙ୍ଗ;
  }
  ସ୍ଥିତି MinMaxKind::maximumnum: {
    ଯଦି (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/ସତ୍ୟ));
    ଯଦି (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                         /*OrderedZeroSign=*/ସତ୍ୟ));
    ଭାଙ୍ଗ;
  }
  ସ୍ଥିତି MinMaxKind::minimumnum: {
    ଯଦି (KnownLHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/ସତ୍ୟ));
    ଯଦି (KnownRHS.isKnownNeverNaN())
      Known.knownNot(orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                            /*OrderedZeroSign=*/ସତ୍ୟ));
    ଭାଙ୍ଗ;
  }
  ସ୍ଥିତି MinMaxKind::maximum: {
    Known.knownNot(orderedStrictlyLess(KnownLHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/ସତ୍ୟ) |
                   orderedStrictlyLess(KnownRHS.getKnownFPClasses(),
                                       /*OrderedZeroSign=*/ସତ୍ୟ));
    ଭାଙ୍ଗ;
  }
  ସ୍ଥିତି MinMaxKind::minimum: {
    Known.knownNot(orderedStrictlyGreater(KnownLHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/ସତ୍ୟ) |
                   orderedStrictlyGreater(KnownRHS.getKnownFPClasses(),
                                          /*OrderedZeroSign=*/ସତ୍ୟ));
    ଭାଙ୍ଗ;
  }
  }

  // Remove impossible non-NaN classes.
  FPClassTest PossibleClasses =
      KnownLHS.getKnownFPClasses() | KnownRHS.getKnownFPClasses() | fcNan;
  Known.knownNot(~PossibleClasses);

  ଫେରନ୍ତୁ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::canonicalize(ଅଚଳ KnownFPClass &KnownSrc,
                                        DenormalMode DenormMode) {
  KnownFPClass Known;

  // Canonicalize may flush denormals to zero, so we have to consider the
  // denormal mode to preserve known-not-0 knowledge.
  Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcZero | fcQNan);

  // Stronger version of propagateNaN
  // Canonicalize is guaranteed to quiet signaling nans.
  ଯଦି (KnownSrc.isKnownNeverNaN())
    Known.knownNot(fcNan);
  ଅନ୍ୟଥା
    Known.knownNot(fcSNan);

  // FIXME: Missing check of IEEE like types.

  // If the parent function flushes denormals, the canonical output cannot be a
  // denormal.
  ଯଦି (DenormMode == DenormalMode::getIEEE()) {
    ଯଦି (KnownSrc.isKnownNever(fcPosZero))
      Known.knownNot(fcPosZero);
    ଯଦି (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);
    ଫେରନ୍ତୁ Known;
  }

  ଯଦି (DenormMode.inputsAreZero() || DenormMode.outputsAreZero())
    Known.knownNot(fcSubnormal);

  ଯଦି (DenormMode == DenormalMode::getPreserveSign()) {
    ଯଦି (KnownSrc.isKnownNever(fcPosZero | fcPosSubnormal))
      Known.knownNot(fcPosZero);
    ଯଦି (KnownSrc.isKnownNever(fcNegZero | fcNegSubnormal))
      Known.knownNot(fcNegZero);
    ଫେରନ୍ତୁ Known;
  }

  ଯଦି (DenormMode.ନିବେଶ == DenormalMode::PositiveZero ||
      (DenormMode.Output == DenormalMode::PositiveZero &&
       DenormMode.ନିବେଶ == DenormalMode::IEEE)) {
    // -0.0 is not a subnormal and should not be flushed.
    ଯଦି (KnownSrc.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    ଯଦି (KnownSrc.isKnownNever(fcPosZero | fcSubnormal))
      Known.knownNot(fcPosZero);
  }

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::bitcast(ଅଚଳ fltSemantics &FltSemantics,
                                   ଅଚଳ KnownBits &Bits) {
  ପ୍ରତିଜ୍ଞା(FltSemantics.sizeInBits == Bits.getBitWidth() &&
         "Bitcast operand has incorrect bit width");
  KnownFPClass Known;

  // Conflicting known bits do not describe a concrete value. Return unknown.
  ଯଦି (Bits.hasConflict())
    ଫେରନ୍ତୁ Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](ଅଚଳ fltSemantics &Semantics) {
    ବାଛ (APFloat::SemanticsToEnum(Semantics)) {
    ସ୍ଥିତି APFloatBase::S_IEEEhalf:
    ସ୍ଥିତି APFloatBase::S_BFloat:
    ସ୍ଥିତି APFloatBase::S_IEEEsingle:
    ସ୍ଥିତି APFloatBase::S_IEEEdouble:
    ସ୍ଥିତି APFloatBase::S_IEEEquad:
    ସ୍ଥିତି APFloatBase::S_x87DoubleExtended:
      ଫେରନ୍ତୁ ସତ୍ୟ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }
  };
  ଯଦି (!IsSupported(FltSemantics))
    ଫେରନ୍ତୁ Known;

  // Transfer information from the sign bit.
  ଯଦି (Bits.isNonNegative())
    Known.signBitMustBeZero();
  ଅନ୍ୟଥା ଯଦି (Bits.isNegative())
    Known.signBitMustBeOne();

  ବାଛ (APFloatBase::SemanticsToEnum(FltSemantics)) {
  ସ୍ଥିତି APFloatBase::S_IEEEhalf:
  ସ୍ଥିତି APFloatBase::S_BFloat:
  ସ୍ଥିତି APFloatBase::S_IEEEsingle:
  ସ୍ଥିତି APFloatBase::S_IEEEdouble:
  ସ୍ଥିତି APFloatBase::S_IEEEquad: {
    ଅଚଳ unsigned MantissaBits = FltSemantics.precision - 1;
    ଅଚଳ APInt ExponentMask = APInt::getBitsSet(
        FltSemantics.sizeInBits, MantissaBits, FltSemantics.sizeInBits - 1);
    ଅଚଳ APInt MantissaMask =
        APInt::get_ତଳ_Bits_Set(FltSemantics.sizeInBits, MantissaBits);

    ଅଚଳ ତାର୍କିକ ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    ଅଚଳ ତାର୍କିକ ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    ଅଚଳ ତାର୍କିକ ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    ଅଚଳ ତାର୍କିକ ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();

    ଅଚଳ ତାର୍କିକ MantissaKnownAllZeros =
        (Bits.Zero & MantissaMask) == MantissaMask;
    ଅଚଳ ତାର୍କିକ MantissaKnownNotAllZeros = !(Bits.One & MantissaMask).isZero();

    // Zero and subnormal require an exponent with all zero bits.
    ଯଦି (ExponentKnownNotAllZeros)
      Known.knownNot(fcZero | fcSubnormal);

    // Infinity and NaN require an exponent with all one bits.
    ଯଦି (ExponentKnownNotAllOnes)
      Known.knownNot(fcInf | fcNan);

    // Normal values have an exponent that is not all zeros or all ones.
    ଯଦି (ExponentKnownAllZeros || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // Zero and infinity require a mantissa with all zero bits.
    ଯଦି (MantissaKnownNotAllZeros)
      Known.knownNot(fcZero | fcInf);

    // Subnormal and NaN require a non-zero mantissa.
    ଯଦି (MantissaKnownAllZeros)
      Known.knownNot(fcSubnormal | fcNan);

    ଅଚଳ ତାର୍କିକ QuietBitKnownSet = Bits.One[MantissaBits - 1];
    ଅଚଳ ତାର୍କିକ QuietBitKnownClear = Bits.Zero[MantissaBits - 1];

    ଯଦି (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    ଅନ୍ୟଥା ଯଦି (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    ଭାଙ୍ଗ;
  }
  ସ୍ଥିତି APFloatBase::S_x87DoubleExtended: {
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
    ଅଚଳ APInt FractionMask = APInt::get_ତଳ_Bits_Set(BitWidth, FractionBits);
    ଅଚଳ APInt ExponentMask =
        APInt::getBitsSet(BitWidth, FractionBits + 1, BitWidth - 1);

    ଅଚଳ ତାର୍କିକ ExponentKnownAllZeros =
        (Bits.Zero & ExponentMask) == ExponentMask;
    ଅଚଳ ତାର୍କିକ ExponentKnownAllOnes = (Bits.One & ExponentMask) == ExponentMask;
    ଅଚଳ ତାର୍କିକ ExponentKnownNotAllZeros = !(Bits.One & ExponentMask).isZero();
    ଅଚଳ ତାର୍କିକ ExponentKnownNotAllOnes = !(Bits.Zero & ExponentMask).isZero();
    ଅଚଳ ତାର୍କିକ FractionKnownAllZeros =
        (Bits.Zero & FractionMask) == FractionMask;
    ଅଚଳ ତାର୍କିକ FractionKnownNotAllZeros = !(Bits.One & FractionMask).isZero();
    ଅଚଳ ତାର୍କିକ IntegerBitKnownSet = Bits.One[IntegerBitIndex];
    ଅଚଳ ତାର୍କିକ IntegerBitKnownClear = Bits.Zero[IntegerBitIndex];

    // zero and subnormal require an exponent with all zero bits and a cleared
    // integer bit.
    ଯଦି (ExponentKnownNotAllZeros || IntegerBitKnownSet)
      Known.knownNot(fcZero | fcSubnormal);

    // Zero requires a zero fraction.
    ଯଦି (FractionKnownNotAllZeros)
      Known.knownNot(fcZero);

    // Infinity requires an exponent with all one bits, a set integer bit, and
    // a zero fraction.
    ଯଦି (ExponentKnownNotAllOnes || IntegerBitKnownClear ||
        FractionKnownNotAllZeros)
      Known.knownNot(fcInf);

    // A normal x87 value has its integer bit set. A zero exponent with that
    // bit set is the x87 pseudodenormal encoding, which APFloat treats as
    // normal.
    ଯଦି (IntegerBitKnownClear || ExponentKnownAllOnes)
      Known.knownNot(fcNormal);

    // A subnormal has a nonzero fraction; NaNs are possible with a zero
    // fraction through pseudoinfinity and unnormal encodings.
    ଯଦି (FractionKnownAllZeros)
      Known.knownNot(fcSubnormal);

    // We can rule out NaN if the value has all exponent bits set to zero, or
    // if the value is a canonical normal or infinity.
    ଯଦି (ExponentKnownAllZeros ||
        (IntegerBitKnownSet &&
         (ExponentKnownNotAllOnes || FractionKnownAllZeros)))
      Known.knownNot(fcNan);

    ଅଚଳ ତାର୍କିକ QuietBitKnownSet = Bits.One[FractionBits - 1];
    ଅଚଳ ତାର୍କିକ QuietBitKnownClear = Bits.Zero[FractionBits - 1];

    ଯଦି (QuietBitKnownSet)
      Known.knownNot(fcSNan);
    ଅନ୍ୟଥା ଯଦି (QuietBitKnownClear)
      Known.knownNot(fcQNan);

    ଭାଙ୍ଗ;
  }
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    llvm_unreachable("unhandled supported type");
  }

  ଫେରନ୍ତୁ Known;
}

KnownBits KnownFPClass::toKnownBits(ଅଚଳ fltSemantics &FltSemantics) ଅଚଳ {
  KnownBits Known(FltSemantics.sizeInBits);
  ଅଚଳ FPClassTest FPClasses = getKnownFPClasses();

  // Return unknown if poison.
  ଯଦି (FPClasses == fcNone)
    ଫେରନ୍ତୁ Known;

  // Return unknown for types we have not validated.
  auto IsSupported = [](ଅଚଳ fltSemantics &Semantics) {
    ବାଛ (APFloat::SemanticsToEnum(Semantics)) {
    ସ୍ଥିତି APFloatBase::S_IEEEhalf:
    ସ୍ଥିତି APFloatBase::S_BFloat:
    ସ୍ଥିତି APFloatBase::S_IEEEsingle:
    ସ୍ଥିତି APFloatBase::S_IEEEdouble:
    ସ୍ଥିତି APFloatBase::S_IEEEquad:
    ସ୍ଥିତି APFloatBase::S_x87DoubleExtended:
      ଫେରନ୍ତୁ ସତ୍ୟ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }
  };
  ଯଦି (!IsSupported(FltSemantics))
    ଫେରନ୍ତୁ Known;

  ବାଛ (APFloat::SemanticsToEnum(FltSemantics)) {
  ସ୍ଥିତି APFloatBase::S_IEEEhalf:
  ସ୍ଥିତି APFloatBase::S_BFloat:
  ସ୍ଥିତି APFloatBase::S_IEEEsingle:
  ସ୍ଥିତି APFloatBase::S_IEEEdouble:
  ସ୍ଥିତି APFloatBase::S_IEEEquad: {
    // For ieee types, we cannot deduce anything if the source could be normal.
    ଯଦି (FPClasses & fcNormal)
      ଭାଙ୍ଗ;

    Known.setAllConflict();

    ଅଚଳ unsigned BitWidth = FltSemantics.sizeInBits;
    ଅଚଳ unsigned MantissaBits = FltSemantics.precision - 1;
    ଅଚଳ unsigned ExponentBits = BitWidth - MantissaBits - 1;

    APInt MantissaMask = APInt::get_ତଳ_Bits_Set(BitWidth, MantissaBits);
    APInt ExponentMask =
        APInt::getBitsSet(BitWidth, MantissaBits, MantissaBits + ExponentBits);

    ଅଚଳ unsigned QuietBitIndex = MantissaBits - 1;
    APInt PayloadMask = MantissaMask;
    PayloadMask.clearBit(QuietBitIndex);

    ଯଦି (FPClasses & fcNan) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // No individual payload bit is known.
      Known.Zero &= ~PayloadMask;
      Known.One &= ~PayloadMask;

      ଯଦି (FPClasses & fcQNan)
        Known.Zero.clearBit(QuietBitIndex);
      ଯଦି (FPClasses & fcSNan)
        Known.One.clearBit(QuietBitIndex);
    }
    ଯଦି (FPClasses & fcInf) {
      // Exponent bits cannot be zeros.
      Known.Zero &= ~ExponentMask;
      // Mantissa bits cannot be ones.
      Known.One &= ~MantissaMask;
    }
    ଯଦି (FPClasses & fcSubnormal) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Unknown mantissa.
      Known.One &= ~MantissaMask;
      Known.Zero &= ~MantissaMask;
    }
    ଯଦି (FPClasses & fcZero) {
      // Exponent bits cannot be ones.
      Known.One &= ~ExponentMask;
      // Mantissa cannot be ones.
      Known.One &= ~MantissaMask;
    }

    ଭାଙ୍ଗ;
  }
  ସ୍ଥିତି APFloatBase::S_x87DoubleExtended: {
    ଯଦି (!isKnownNever(fcNormal | fcSubnormal | fcNan))
      ଭାଙ୍ଗ;

    Known.setAllConflict();

    ଯଦି (FPClasses & fcInf)
      Known = Known.intersectWith(KnownBits::makeConstant(
          APFloat::getInf(FltSemantics).bitcastToAPInt()));

    ଯଦି (FPClasses & fcZero)
      Known = Known.intersectWith(
          KnownBits::makeConstant(APInt::getZero(FltSemantics.sizeInBits)));

    ଭାଙ୍ଗ;
  }
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    llvm_unreachable("unhandled supported semantics");
  }

  Known.Zero.clearSignBit();
  Known.One.clearSignBit();
  ଯଦି (std::optional<ତାର୍କିକ> Sign = getSignBit()) {
    ଯଦି (*Sign)
      Known.makeNegative();
    ଅନ୍ୟଥା
      Known.makeNonNegative();
  }

  ଫେରନ୍ତୁ Known;
}

// Handle known sign bit and nan cases for fadd, fsub, and fma.
ସ୍ଥିର KnownFPClass fadd_impl(ଅଚଳ KnownFPClass &KnownLHS,
                              ଅଚଳ KnownFPClass &KnownRHS, ତାର୍କିକ IsFMA) {
  KnownFPClass Known;

  // Adding positive and negative infinity produces NaN, but only if both
  // opposite-sign infinity combinations are possible.
  ଯଦି (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNever(fcPosInf) || KnownRHS.isKnownNever(fcNegInf)) &&
      (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosInf)))
    Known.knownNot(fcNan);

  ଯଦି (KnownLHS.cannotBeOrderedLessThanZero() &&
      KnownRHS.cannotBeOrderedLessThanZero()) {
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    ଯଦି (KnownLHS.isKnownNever(fcZero | fcPosSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcPosSubnormal))
      Known.knownNot(fcZero | fcPosSubnormal);
  }

  ଯଦି (KnownLHS.cannotBeOrderedGreaterThanZero() &&
      KnownRHS.cannotBeOrderedGreaterThanZero()) {
    Known.knownNot(KnownFPClass::OrderedGreaterThanZeroMask);

    // This can't underflow if one of the operands is known normal.
    ଯଦି (KnownLHS.isKnownNever(fcZero | fcNegSubnormal) ||
        KnownRHS.isKnownNever(fcZero | fcNegSubnormal))
      Known.knownNot(fcZero | fcNegSubnormal);
  }

  ଯଦି (!IsFMA) {
    // (fadd x, +0.0) is guaranteed to return +0.0, not -0.0.
    ଯଦି (KnownLHS.isKnownNever(fcNegZero) || KnownRHS.isKnownNever(fcNegZero))
      Known.knownNot(fcNegZero);

    Known.propagateNonSNaN(KnownLHS, KnownRHS);
  }

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::fadd(ଅଚଳ KnownFPClass &KnownLHS_,
                                ଅଚଳ KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_ନିବେଶ_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_ନିବେଶ_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/ମିଥ୍ୟା);

  ଫେରନ୍ତୁ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fadd_self(ଅଚଳ KnownFPClass &KnownSrc_,
                                     DenormalMode Mode) {
  KnownFPClass KnownSrc = apply_ନିବେଶ_Denormal_Mode(KnownSrc_, Mode);

  KnownFPClass Known = fadd_impl(KnownSrc, KnownSrc, /*IsFMA=*/ମିଥ୍ୟା);

  ଯଦି (KnownSrc.isKnownNever(fcPosSubnormal))
    Known.knownNot(fcPosSubnormal);
  ଯଦି (KnownSrc.isKnownNever(fcNegSubnormal))
    Known.knownNot(fcNegSubnormal);

  // Doubling 0 will give the same 0.
  ଯଦି (KnownSrc.isKnownNever(fcPosZero))
    Known.knownNot(fcPosZero);

  ଫେରନ୍ତୁ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fsub(ଅଚଳ KnownFPClass &KnownLHS_,
                                ଅଚଳ KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_ନିବେଶ_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_ନିବେଶ_Denormal_Mode(KnownRHS_, Mode);

  // fsub(x, y) == fadd(x, -y) as long as we negate after subnormals are
  // flushed.
  KnownRHS.fneg();

  KnownFPClass Known = fadd_impl(KnownLHS, KnownRHS, /*IsFMA=*/ମିଥ୍ୟା);

  ଫେରନ୍ତୁ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fmul(ଅଚଳ KnownFPClass &KnownLHS_,
                                ଅଚଳ KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_ନିବେଶ_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_ନିବେଶ_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // +X * +Y or -X * -Y => +Q
  // +X * -Y or -X * +Y => -Q
  ଯଦି ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  ଯଦି ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Inf * Y => Inf or NaN
  ଯଦି (KnownLHS.isKnownAlways(fcInf | fcNan) ||
      KnownRHS.isKnownAlways(fcInf | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcZero);

  // 0 * Y => 0 or NaN
  ଯଦି (KnownRHS.isKnownAlways(fcZero | fcNan) ||
      KnownLHS.isKnownAlways(fcZero | fcNan))
    Known.knownNot(fcNormal | fcSubnormal | fcInf);

  // 0 * +/-inf => NaN
  ଯଦି (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownRHS.isKnownNeverInfinity() || KnownLHS.isKnownNeverZero()) &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverZero()))
    Known.knownNot(fcNan);

  ଫେରନ୍ତୁ applyOutputDenormalMode(Known, Mode);
}

// TODO: This generalizes to known ranges
KnownFPClass KnownFPClass::fmul(ଅଚଳ KnownFPClass &KnownLHS,
                                ଅଚଳ APFloat &CRHS, DenormalMode Mode) {
  // Match denormal scaling pattern, similar to the case in ldexp. If the
  // constant's exponent is sufficiently large, the result cannot be subnormal.

  ଅଚଳ fltSemantics &Flt = CRHS.getSemantics();
  unsigned Precision = APFloat::semanticsPrecision(Flt);
  ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା MantissaBits = Precision - 1;

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା MinKnownExponent = ilogb(CRHS);
  ତାର୍କିକ CannotBeSubnormal = (MinKnownExponent >= MantissaBits);

  KnownFPClass Known = KnownFPClass::fmul(KnownLHS, KnownFPClass(CRHS), Mode);
  ଯଦି (CannotBeSubnormal)
    Known.knownNot(fcSubnormal);

  // Multiply of values <= 1 cannot introduce overflow.
  ଯଦି (KnownLHS.isKnownNever(fcInf)) {
    ଯଦି (MinKnownExponent < 0)
      Known.knownNot(fcInf);
    ଅନ୍ୟଥା ଯଦି (MinKnownExponent == 0 && CRHS.compare_Absolute_ମୂଲ୍ୟ(APFloat::getOne(
                                          Flt)) == APFloat::cmpEqual)
      Known.knownNot(fcInf);
  }

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::fdiv(ଅଚଳ KnownFPClass &KnownLHS_,
                                ଅଚଳ KnownFPClass &KnownRHS_,
                                DenormalMode Mode) {
  KnownFPClass KnownLHS = apply_ନିବେଶ_Denormal_Mode(KnownLHS_, Mode);
  KnownFPClass KnownRHS = apply_ନିବେଶ_Denormal_Mode(KnownRHS_, Mode);

  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // Only 0/0, Inf/Inf produce NaN.
  ଯଦି (KnownLHS.isKnownNeverNaN() && KnownRHS.isKnownNeverNaN() &&
      (KnownLHS.isKnownNeverInfinity() || KnownRHS.isKnownNeverInfinity()) &&
      (KnownLHS.isKnownNeverZero() || KnownRHS.isKnownNeverZero())) {
    Known.knownNot(fcNan);
  }

  //  X / -0.0 => -Inf (or NaN)
  // +X / +Y or -X / -Y => +Q
  // +X / -Y or -X / +Y => -Q
  ଯଦି ((KnownLHS.isKnownNever(fcNegative) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcPositive) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcNegative);
  ଯଦି ((KnownLHS.isKnownNever(fcPositive) &&
       KnownRHS.isKnownNever(fcNegative)) ||
      (KnownLHS.isKnownNever(fcNegative) && KnownRHS.isKnownNever(fcPositive)))
    Known.knownNot(fcPositive);

  // Normal and subnormal results require two non-zero finite operands.
  ଯଦି ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)))
    Known.knownNot(fcNegNormal | fcNegSubnormal);
  ଯଦି ((KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal) &&
       KnownRHS.isKnownNever(fcPosNormal | fcPosSubnormal)) ||
      (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal) &&
       KnownRHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosNormal | fcPosSubnormal);

  // 0 / X => 0 or NaN
  ଯଦି (KnownLHS.isKnownAlways(fcZero))
    Known.knownNot(fcSubnormal | fcNormal | fcInf);

  // X / 0 => NaN or Inf
  ଯଦି (KnownRHS.isKnownAlways(fcZero))
    Known.knownNot(fcFinite);

  ଫେରନ୍ତୁ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fdiv_self(ଅଚଳ KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X / X is always exactly 1.0 or a NaN.
  KnownFPClass Known(fcNan | fcPosNormal);

  Known.propagateNonSNaN(KnownSrc);

  ଯଦି (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::frem(ଅଚଳ KnownFPClass &KnownLHS,
                                ଅଚଳ KnownFPClass &KnownRHS,
                                DenormalMode Mode) {
  KnownFPClass Known;

  // We are assuming that frem(x, Inf) is already handled elsewhere.

  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // frem(Inf, x) and frem(x, 0) produce NaN.
  ଯଦି (KnownLHS.isKnownNeverInfOrNaN() && KnownRHS.isKnownNeverNaN() &&
      KnownRHS.isKnownNeverLogicalZero(Mode)) {
    Known.knownNot(fcNan);
  }

  // The sign for frem is the same as the first operand.
  ଯଦି (KnownLHS.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal | fcPosSubnormal);
  ଯଦି (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal | fcNegSubnormal);

  // A negative zero result requires a negative finite first operand.
  ଯଦି (KnownLHS.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // A positive zero result can additionally come from a negative finite
  // result being flushed to positive zero.
  ଯଦି (KnownLHS.isKnownNever(fcPosFinite) &&
      KnownLHS.isKnownNeverLogicalPosZero(Mode) &&
      (!Mode.outputsMayBePositiveZero() ||
       KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)))
    Known.knownNot(fcPosZero);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::frem_self(ଅଚଳ KnownFPClass &KnownSrc,
                                     DenormalMode Mode) {
  // X % X is always exactly [+-]0.0 or a NaN.
  KnownFPClass Known(fcNan | fcZero);

  Known.propagateNonSNaN(KnownSrc);

  ଯଦି (KnownSrc.isKnownNeverInfOrNaN() && KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNan);

  // The sign of a zero result is the sign of the finite non-zero input.
  // We do not have to worry about subnormal values flushing to zero, since
  // frem(+-0.0, +-0.0) = NaN.
  ଯଦି (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosZero);
  ଯଦି (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegZero);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::fma(ଅଚଳ KnownFPClass &KnownLHS,
                               ଅଚଳ KnownFPClass &KnownRHS,
                               ଅଚଳ KnownFPClass &KnownAddend,
                               DenormalMode Mode) {
  KnownFPClass Mul = fmul(KnownLHS, KnownRHS, Mode);

  // FMA differs from the base fmul + fadd handling only in the treatment of -0
  // results.
  //
  // If the multiply is a -0 due to rounding, the final -0 + 0 will be -0,
  // unlike for a separate fadd.
  KnownFPClass Known = fadd_impl(Mul, KnownAddend, /*IsFMA=*/ସତ୍ୟ);

  // propagateNonSNaN for 3 arguments.
  ଯଦି (KnownLHS.isKnownNever(fcSNan) && KnownRHS.isKnownNever(fcSNan) &&
      KnownAddend.isKnownNever(fcSNan))
    Known.knownNot(fcSNan);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::fmuladd(ଅଚଳ KnownFPClass &KnownLHS,
                                   ଅଚଳ KnownFPClass &KnownRHS,
                                   ଅଚଳ KnownFPClass &KnownAddend,
                                   DenormalMode Mode) {
  // fmuladd can lower to either fma(x, y, z) or fadd(fmul(x, y), z). Compute
  // the intersection of both possibilities.
  KnownFPClass FMA = fma(KnownLHS, KnownRHS, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(fmul(KnownLHS, KnownRHS, Mode), KnownAddend, Mode);
  ଫେରନ୍ତୁ FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::fma_square(ଅଚଳ KnownFPClass &KnownSquared,
                                      ଅଚଳ KnownFPClass &KnownAddend,
                                      DenormalMode Mode) {
  KnownFPClass Squared = square(KnownSquared, Mode);
  KnownFPClass Known = fadd_impl(Squared, KnownAddend, /*IsFMA=*/ସତ୍ୟ);

  // Since we know the squared input must be positive, the add of opposite sign
  // infinities nan hazard only applies for negative inf.
  //
  // TODO: Alternatively to proving addend is not -inf, we could know Squared is
  // not pinf. Other than the degenerate always-subnormal input case, we can't
  // prove that without a known range.
  ଯଦି (KnownAddend.isKnownNever(fcNegInf | fcNan) && Squared.isKnownNever(fcNan))
    Known.knownNot(fcNan);

  Known.propagateNonSNaN(KnownSquared, KnownAddend);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::fmuladd_square(ଅଚଳ KnownFPClass &KnownSquared,
                                          ଅଚଳ KnownFPClass &KnownAddend,
                                          DenormalMode Mode) {
  KnownFPClass FMA = fma_square(KnownSquared, KnownAddend, Mode);
  KnownFPClass MulAdd = fadd(square(KnownSquared, Mode), KnownAddend, Mode);
  ଫେରନ୍ତୁ FMA.intersectWith(MulAdd);
}

KnownFPClass KnownFPClass::exp(ଅଚଳ KnownFPClass &KnownSrc) {
  KnownFPClass Known;
  Known.knownNot(fcNegative);

  Known.propagateNonNaN(KnownSrc);

  // The following deductions assume that exp10(-1.0) = +0.1 and
  // exp10(+1.0) = +10.0 are both finite normal values.

  // Zero can only be produced if x < -1.0.
  ଯଦି (KnownSrc.isKnownNever(fcNegNormal | fcNegInf))
    Known.knownNot(fcPosZero);

  // Subnormals can only be produced if x < -1.0 and x is finite.
  ଯଦି (KnownSrc.isKnownNever(fcNegNormal))
    Known.knownNot(fcPosSubnormal);

  // Infinity can only be produced if x > +1.0.
  ଯଦି (KnownSrc.isKnownNever(fcPosNormal | fcPosInf))
    Known.knownNot(fcPosInf);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::log(ଅଚଳ KnownFPClass &KnownSrc, DenormalMode Mode,
                               ତାର୍କିକ IsMultiUnitFPType) {
  KnownFPClass Known;

  // PPCDoubleDouble can represent the exact value of 1.0 + subnormal, and
  // log(1.0 + subnormal) = subnormal.
  ଯଦି (!IsMultiUnitFPType)
    Known.knownNot(fcNegZero | fcSubnormal);

  Known.propagateNonSNaN(KnownSrc);

  ଯଦି (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  ଯଦି (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  ଯଦି (KnownSrc.isKnownNeverLogicalZero(Mode))
    Known.knownNot(fcNegInf);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::sqrt(ଅଚଳ KnownFPClass &KnownSrc,
                                DenormalMode Mode) {
  KnownFPClass Known;
  Known.knownNot(fcPosSubnormal);

  ଯଦି (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  Known.propagateNonSNaN(KnownSrc);

  // Any negative value besides -0 returns a nan.
  ଯଦି (KnownSrc.isKnownNeverNaN() && KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(fcNan);

  // The only negative value that can be returned is -0 for -0 inputs.
  Known.knownNot(fcNegInf | fcNegSubnormal | fcNegNormal);

  // If the input denormal mode could be PreserveSign, a negative
  // subnormal input could produce a negative zero output.
  ଯଦି (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::sin(ଅଚଳ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownSrc);

  // Return NaN on infinite inputs.
  Known.knownNot(fcInf);
  ଯଦି (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::cos(ଅଚଳ KnownFPClass &KnownSrc) {
  ଫେରନ୍ତୁ sin(KnownSrc);
}

KnownFPClass KnownFPClass::tan(ଅଚଳ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tan never returns Inf (tan(+-Inf) = NaN; tan(finite) = finite).
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. tan(+-Inf) is NaN.
  ଯଦି (KnownSrc.isKnownNeverNaN() && KnownSrc.isKnownNeverInfinity())
    Known.knownNot(fcNan);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::sinh(ଅଚଳ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // sinh is sign-preserving: sinh(x) < 0 iff x < 0.
  ଯଦି (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // A nonzero positive result requires a nonzero positive input.
  // Zero remains possible: sinh(+/-0.0) = +/-0.0, and flushing a subnormal
  // may produce +0.0.
  ଯଦି (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal | fcPosInf);

  Known.propagateNonNaN(KnownSrc);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::cosh(ଅଚଳ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // cosh(x) >= 1 for all real x; cosh(+-Inf) = +Inf. Never negative,
  // zero, or subnormal.
  Known.knownNot(fcNegative | fcZero | fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::tanh(ଅଚଳ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // tanh is bounded to (-1, 1), never Inf.
  Known.knownNot(fcInf);

  // tanh is sign-preserving: tanh(x) < 0 iff x < 0.
  ଯଦି (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf)) {
    Known.knownNot(fcNegSubnormal | fcNegNormal);

    // -0.0 is preserved: tanh(-0.0) = -0.0.
    ଯଦି (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  // Likewise, tanh(x) > 0 iff x > 0. Zero is not ruled out: tanh(+-0.0) =
  // +-0.0, and a flushed subnormal may become +0.0.
  ଯଦି (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  Known.propagateNonNaN(KnownSrc);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::asin(ଅଚଳ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // asin is bounded to [-pi/2, pi/2], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonSNaN(KnownSrc);

  // asin is sign-preserving for finite arguments.
  ଯଦି (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegFinite);

  ଯଦି (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // NaN propagates. asin(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::acos(ଅଚଳ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // acos(x) is bounded to [0, pi] for -1 <= x <= 1, and is never negative,
  // infinite, or subnormal. The smallest non-zero value occurs when x is
  // close to 1.0, where acos(x) can be approximated by sqrt(2 * (1 - x)).
  // Since sqrt cannot produce a subnormal result, we can conclude that
  // acos(x) will also never produce a subnormal result.
  Known.knownNot(fcNegative | fcInf | fcSubnormal);

  // acos(x) == +0.0 iff x == +1.0
  ଯଦି (KnownSrc.isKnownNever(fcPosNormal))
    Known.knownNot(fcZero);

  Known.propagateNonSNaN(KnownSrc);

  // NaN propagates. acos(x) is also NaN for |x| > 1, so we cannot rule
  // out NaN without knowing the source is in [-1, 1].
  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::atan(ଅଚଳ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // atan is bounded to (-pi/2, pi/2), never Inf. atan(+-Inf) = +-pi/2 (finite).
  Known.knownNot(fcInf);

  // Ruling out Subnormal/Normal from the result requires the
  // input to never be finite-nonzero and never inf, since
  // atan(+-inf) = +-pi/2
  ଯଦି (KnownSrc.isKnownNever(fcNegSubnormal | fcNegNormal | fcNegInf))
    Known.knownNot(fcNegSubnormal | fcNegNormal);

  ଯଦି (KnownSrc.isKnownNever(fcPosSubnormal | fcPosNormal | fcPosInf))
    Known.knownNot(fcPosSubnormal | fcPosNormal);

  // atan(-0.0) = -0.0. Negative finite inputs may also produce -0.0
  // if the result rounds to a negative subnormal that is flushed to zero.
  ଯଦି (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal | fcNegZero))
    Known.knownNot(fcNegZero);

  Known.propagateNonNaN(KnownSrc);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::atan2(ଅଚଳ KnownFPClass &KnownY_,
                                 ଅଚଳ KnownFPClass &KnownX_,
                                 DenormalMode Mode) {
  KnownFPClass KnownY = apply_ନିବେଶ_Denormal_Mode(KnownY_, Mode);
  KnownFPClass KnownX = apply_ନିବେଶ_Denormal_Mode(KnownX_, Mode);
  KnownFPClass Known;

  // Even though these deductions are correct, we are ignoring the following
  // potentially erroneous cases:
  //   * atan2(y, inf) is not subnormal
  //   * atan2(inf, x) is not zero or subnormal

  // atan2 result is in (-pi, pi], never Inf.
  Known.knownNot(fcInf);

  Known.propagateNonNaN(KnownY, KnownX);

  // If x <= -0.0, then |atan2(y, x)| >= pi/2
  ଯଦି (KnownX.isKnownNever(fcPositive))
    Known.knownNot(fcZero | fcSubnormal);

  // If y >= +0.0, then atan2(y, x) >= +0.0
  ଯଦି (KnownY.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);

  // If y <= -0.0, then atan2(y, x) <= -0.0
  ଯଦି (KnownY.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);

  ଫେରନ୍ତୁ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::fpext(ଅଚଳ KnownFPClass &KnownSrc,
                                 ଅଚଳ fltSemantics &DstTy,
                                 ଅଚଳ fltSemantics &SrcTy) {
  // Infinity, nan and zero propagate from source.
  KnownFPClass Known = KnownSrc;

  // All subnormal inputs should be in the normal range in the result type.
  ଯଦି (APFloat::isRepresentableAsNormalIn(SrcTy, DstTy)) {
    ଯଦି (Known.getKnownFPClasses() & fcPosSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcPosNormal);
    ଯଦି (Known.getKnownFPClasses() & fcNegSubnormal)
      Known.setKnownFPClasses(Known.getKnownFPClasses() | fcNegNormal);
    Known.knownNot(fcSubnormal);
  }

  // Sign bit of a nan isn't guaranteed.
  ଯଦି (!Known.isKnownNeverNaN())
    Known.setSignBit(std::nullopt);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::fptrunc(ଅଚଳ KnownFPClass &KnownSrc) {
  KnownFPClass Known;

  // Sign should be preserved
  // TODO: Handle cannot be ordered greater than zero
  ଯଦି (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(KnownFPClass::OrderedLessThanZeroMask);

  Known.propagateNonNaN(KnownSrc);

  // Infinity needs a range check.
  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::roundToIntegral(ଅଚଳ KnownFPClass &KnownSrc,
                                           ତାର୍କିକ MayBeMultiUnitFPType,
                                           DenormalMode Mode) {
  KnownFPClass Known;

  // Integer results cannot be subnormal.
  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  // Pass through infinities, except PPC_FP128 is a special case for intrinsics.
  ଯଦି (!MayBeMultiUnitFPType) {
    ଯଦି (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    ଯଦି (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  }

  ଯଦି (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  ଯଦି (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  // Negative round ups towards zero produce negative zero.
  ଯଦି (KnownSrc.isKnownNever(fcNegFinite))
    Known.knownNot(fcNegZero);

  // Negative subnormals may flush to positive zero.
  ଯଦି (KnownSrc.isKnownNever(fcPosFinite) &&
      (KnownSrc.isKnownNever(fcNegSubnormal) ||
       !Mode.inputsMayBePositiveZero()))
    Known.knownNot(fcPosZero);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::trunc(ଅଚଳ KnownFPClass &KnownSrc,
                                 DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc,
                                       /*MayBeMultiUnitFPType=*/ସତ୍ୟ, Mode);

  ଯଦି (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);
  ଯଦି (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::floor(ଅଚଳ KnownFPClass &KnownSrc,
                                 ତାର୍କିକ IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  ଯଦି (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::ceil(ଅଚଳ KnownFPClass &KnownSrc,
                                ତାର୍କିକ IsMultiUnitFPType, DenormalMode Mode) {
  KnownFPClass Known = roundToIntegral(KnownSrc, IsMultiUnitFPType, Mode);

  // Applicable for multi-unit floating point types.
  ଯଦି (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::frexp_mant(ଅଚଳ KnownFPClass &KnownSrc,
                                      DenormalMode Mode) {
  KnownFPClass Known;

  Known.knownNot(fcSubnormal);

  Known.propagateNonNaN(KnownSrc);

  ଯଦି (KnownSrc.isKnownNeverPosInfinity())
    Known.knownNot(fcPosInf);

  ଯଦି (KnownSrc.isKnownNeverNegInfinity())
    Known.knownNot(fcNegInf);

  ଯଦି (KnownSrc.isKnownNeverLogicalPosZero(Mode))
    Known.knownNot(fcPosZero);

  ଯଦି (KnownSrc.isKnownNeverLogicalNegZero(Mode))
    Known.knownNot(fcNegZero);

  // TODO: These deductions can be improved if subnormal inputs are guaranteed
  // to be flushed to zero.
  ଯଦି (KnownSrc.isKnownNever(fcPosNormal | fcPosSubnormal))
    Known.knownNot(fcPosNormal);

  ଯଦି (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal))
    Known.knownNot(fcNegNormal);

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::ldexp(ଅଚଳ KnownFPClass &KnownSrc_,
                                 ଅଚଳ APInt &ConstantRangeExpMin,
                                 ଅଚଳ APInt &ConstantRangeExpMax,
                                 ଅଚଳ fltSemantics &Flt, DenormalMode Mode) {
  KnownFPClass Known;

  KnownFPClass KnownSrc = apply_ନିବେଶ_Denormal_Mode(KnownSrc_, Mode);

  ଯଦି (ConstantRangeExpMin.isZero() && ConstantRangeExpMax.isZero()) {
    // ldexp(x, 0) -> x
    // Here we make sure to properly propagate sNaN and denormals.
    Known.setKnownFPClasses(KnownSrc.getKnownFPClasses() | fcNan);
    Known.propagateNonNaN(KnownSrc);
    ଫେରନ୍ତୁ applyOutputDenormalMode(Known, Mode);
  }

  Known.propagateNonNaN(KnownSrc);

  // Sign is preserved, but underflows may produce zeroes.
  ଯଦି (KnownSrc.isKnownNever(fcNegative))
    Known.knownNot(fcNegative);
  ଅନ୍ୟଥା ଯଦି (KnownSrc.cannotBeOrderedLessThanZero())
    Known.knownNot(OrderedLessThanZeroMask);

  ଯଦି (KnownSrc.isKnownNever(fcPositive))
    Known.knownNot(fcPositive);
  ଅନ୍ୟଥା ଯଦି (KnownSrc.cannotBeOrderedGreaterThanZero())
    Known.knownNot(OrderedGreaterThanZeroMask);

  // TODO: determine when it is safe to rule out subnormal for ppcf128.
  ଯଦି (&Flt != &APFloat::PPCDoubleDouble()) {
    unsigned Precision = APFloat::semanticsPrecision(Flt);
    ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା MantissaBits = Precision - 1;
    ଯଦି (ConstantRangeExpMin.sge(MantissaBits))
      Known.knownNot(fcSubnormal);
  }

  ଯଦି (ConstantRangeExpMax.isNonPositive()) {
    // If we know the power is <= 0, can't introduce inf
    ଯଦି (KnownSrc.isKnownNeverPosInfinity())
      Known.knownNot(fcPosInf);
    ଯଦି (KnownSrc.isKnownNeverNegInfinity())
      Known.knownNot(fcNegInf);
  } ଅନ୍ୟଥା ଯଦି (ConstantRangeExpMin.isNonNegative()) {
    // If we know the power is >= 0, can't introduce subnormal or zero
    ଯଦି (KnownSrc.isKnownNeverPosSubnormal())
      Known.knownNot(fcPosSubnormal);
    ଯଦି (KnownSrc.isKnownNeverNegSubnormal())
      Known.knownNot(fcNegSubnormal);
    ଯଦି (KnownSrc.isKnownNeverPosZero())
      Known.knownNot(fcPosZero);
    ଯଦି (KnownSrc.isKnownNeverNegZero())
      Known.knownNot(fcNegZero);
  }

  ଫେରନ୍ତୁ applyOutputDenormalMode(Known, Mode);
}

KnownFPClass KnownFPClass::ldexp(ଅଚଳ KnownFPClass &KnownSrc,
                                 ଅଚଳ KnownBits &ExpBits,
                                 ଅଚଳ fltSemantics &Flt, DenormalMode Mode) {
  ଫେରନ୍ତୁ ldexp(KnownSrc, ExpBits.get_Signed_Min_ମୂଲ୍ୟ(),
               ExpBits.get_Signed_Max_ମୂଲ୍ୟ(), Flt, Mode);
}

KnownFPClass KnownFPClass::pow(ଅଚଳ KnownFPClass &KnownLHS,
                               ଅଚଳ KnownFPClass &KnownRHS) {
  KnownFPClass Known;

  Known.propagateNonSNaN(KnownLHS, KnownRHS);

  // pow may return NaN if one of the arguments is NaN. NaN may be produced from
  // a non-zero-finite-negative base and a non-integer exponent.
  ଯଦି (KnownLHS.isKnownNever(fcNan | fcNegNormal | fcNegSubnormal) &&
      KnownRHS.isKnownNeverNaN())
    Known.knownNot(fcNan);

  // We could rule out negative and subnormal results when exponent is known to
  // never be a normal value, but having either argument being known to never be
  // normal is unlikely and not worth considering.

  // Only a negative base raised to an odd power returns a negative value.
  ଯଦି (KnownLHS.isKnownNever(fcNegative)) {
    Known.knownNot(fcNegative);
  } ଅନ୍ୟଥା ଯଦି (KnownLHS.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // pow(-0.0, odd-positive) = -0.0
    // pow(-inf, odd-negative) = -0.0
    ଯଦି ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcPosNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcNegNormal)))
      Known.knownNot(fcNegZero);

    // pow(-0.0, odd-negative) = -inf
    // pow(-inf, odd-positive) = -inf
    ଯଦି ((KnownLHS.isKnownNever(fcNegZero) ||
         KnownRHS.isKnownNever(fcNegNormal)) &&
        (KnownLHS.isKnownNever(fcNegInf) || KnownRHS.isKnownNever(fcPosNormal)))
      Known.knownNot(fcNegInf);
  }

  ଫେରନ୍ତୁ Known;
}

KnownFPClass KnownFPClass::powi(ଅଚଳ KnownFPClass &KnownSrc,
                                ଅଚଳ KnownBits &ExponentKnownBits) {
  KnownFPClass Known;
  Known.propagateNonNaN(KnownSrc);

  ଯଦି (ExponentKnownBits.isZero()) {
    // powi(QNaN, 0) returns 1.0, and powi(SNaN, 0) may non-deterministically
    // return 1.0 or a NaN.
    ଯଦି (KnownSrc.isKnownNever(fcSNan)) {
      Known.knownNot(~fcPosNormal);
      ଫେରନ୍ତୁ Known;
    }

    Known.knownNot(~(fcPosNormal | fcNan));
    ଫେରନ୍ତୁ Known;
  }

  // Given that exp is an integer, here are the
  // ways that powi can return a negative value:
  //
  //   powi(x, exp)    --> negative if exp is odd and x is negative.
  //   powi(-0, exp)   --> -inf if exp is negative odd.
  //   powi(-0, exp)   --> -0 if exp is positive odd.
  //   powi(-inf, exp) --> -0 if exp is negative odd.
  //   powi(-inf, exp) --> -inf if exp is positive odd.
  ଯଦି (KnownSrc.isKnownNever(fcNegative) || ExponentKnownBits.isEven()) {
    Known.knownNot(fcNegative);
  } ଅନ୍ୟଥା ଯଦି (KnownSrc.isKnownNever(fcNegNormal | fcNegSubnormal)) {
    Known.knownNot(fcNegNormal | fcNegSubnormal);
    // See if we can also rule out -0.0 or -inf.
    // Here at least one of -0.0 or -inf is a possible base.

    // We already know that ExponentKnownBits.isEven() is false here.
    ଅଚଳ ତାର୍କିକ IsKnownNeverOddPositive = ExponentKnownBits.isNegative();
    ଅଚଳ ତାର୍କିକ IsKnownNeverOddNegative = ExponentKnownBits.isNonNegative();

    // powi(-0.0, odd-positive) = -0.0
    // powi(-inf, odd-negative) = -0.0
    ଯଦି ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddPositive) &&
        (KnownSrc.isKnownNever(fcNegInf) || IsKnownNeverOddNegative))
      Known.knownNot(fcNegZero);

    // powi(-0.0, odd-negative) = -inf
    // powi(-inf, odd-positive) = -inf
    ଯଦି ((KnownSrc.isKnownNever(fcNegZero) || IsKnownNeverOddNegative) &&
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
    APInt MinExp = ExponentKnownBits.get_Signed_Min_ମୂଲ୍ୟ();
    APInt MaxExp = ExponentKnownBits.get_Signed_Max_ମୂଲ୍ୟ();

    // powi(inf, exp), exp > 0
    ତାର୍କିକ MayInfSrc =
        !KnownSrc.isKnownNever(fcInf) && MaxExp.isStrictlyPositive();

    // powi(+/-0, exp), exp < 0
    ତାର୍କିକ MayDivByZero = !KnownSrc.isKnownNever(fcZero) && MinExp.isNegative();

    // powi(finite, exp), |exp| > 1
    ତାର୍କିକ MayFinite = !KnownSrc.isKnownNever(fcNormal | fcSubnormal);
    ତାର୍କିକ MayAbsExpGT1 = MinExp.slt(-1) || MaxExp.sgt(1);
    ତାର୍କିକ MayFiniteOverflow = MayFinite && MayAbsExpGT1;

    // powi(subnormal, -1)
    ତାର୍କିକ MayBeNegOne = ExponentKnownBits.Zero.isZero();
    ତାର୍କିକ MaySubnormInv = !KnownSrc.isKnownNever(fcSubnormal) && MayBeNegOne;

    ଯଦି (!MayInfSrc && !MayDivByZero && !MayFiniteOverflow && !MaySubnormInv)
      Known.knownNot(fcInf);
  }

  ଫେରନ୍ତୁ Known;
}
