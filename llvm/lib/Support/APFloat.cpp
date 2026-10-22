//===-- APFloat.cpp - Implement APFloat class -----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a class to represent arbitrary precision floating
// point values and provide a variety of arithmetic operations on them.
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/ADT/APFloat.h"
#അന്തര്ഭാവ "llvm/ADT/APSInt.h"
#അന്തര്ഭാവ "llvm/ADT/ArrayRef.h"
#അന്തര്ഭാവ "llvm/ADT/FloatingPointMode.h"
#അന്തര്ഭാവ "llvm/ADT/FoldingSet.h"
#അന്തര്ഭാവ "llvm/ADT/Hashing.h"
#അന്തര്ഭാവ "llvm/ADT/STLExtras.h"
#അന്തര്ഭാവ "llvm/ADT/SmallVector.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/ADT/StringSwitch.h"
#അന്തര്ഭാവ "llvm/Config/llvm-config.h"
#അന്തര്ഭാവ "llvm/Support/Debug.h"
#അന്തര്ഭാവ "llvm/Support/Error.h"
#അന്തര്ഭാവ "llvm/Support/MathExtras.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <cstring>
#അന്തര്ഭാവ <iterator>
#അന്തര്ഭാവ <limits.h>

/// Shared headers from LLVM libc
/// Make sure to add ${LLVM_SOURCE_DIR}/../libc to include directories.
///
/// Notes: So far it looks like APFloat does not check errnos or floating-point
/// exceptions after calling the math functions, so we will configure LLVM libc
/// math functions to skip setting errnos and floating-point exceptions
/// explicitly.  We also put them in a separate namespace so that the symbols
/// do not clash with other libc math builds just in case.
#define LIBC_NAMESPACE __llvm_libc_apfloat
#define LIBC_MATH (LIBC_MATH_NO_ERRNO | LIBC_MATH_NO_EXCEPT)

#അന്തര്ഭാവ "shared/math.h"
#അന്തര്ഭാവ "shared/math_check_exceptions.h"

#define APFLOAT_DISPATCH_ON_SEMANTICS(METHOD_CALL)                             \
  ചെയ്യുക {                                                                         \
    എങ്കിൽ (usesLayout<IEEEFloat>(getSemantics()))                                 \
      തിരികെ U.IEEE.METHOD_CALL;                                               \
    എങ്കിൽ (usesLayout<DoubleAPFloat>(getSemantics()))                             \
      തിരികെ U.Double.METHOD_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } ആകുമ്പോൾ (അസത്യം)

using നാമാകാശ llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#define Pack_Categories_Into_താക്കോൽ(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloatBase::integerPartWidth % 4 == 0, "Part width must be divisible by 4!");

നാമാകാശ llvm {

constexpr fltSemantics APFloatBase::semIEEEhalf = {15, -14, 11, 16};
constexpr fltSemantics APFloatBase::semBFloat = {127, -126, 8, 16};
constexpr fltSemantics APFloatBase::semIEEEsingle = {127, -126, 24, 32};
constexpr fltSemantics APFloatBase::semIEEEdouble = {1023, -1022, 53, 64};
constexpr fltSemantics APFloatBase::semIEEEquad = {16383, -16382, 113, 128};
constexpr fltSemantics APFloatBase::semX87DoubleExtended = {
    16383,
    -16382,
    64,
    80,
    fltNonfiniteBehavior::IEEE754,
    fltNanEncoding::IEEE,
    സത്യം,
    സത്യം,
    സത്യം,
    സത്യം,
    സത്യം};
constexpr fltSemantics APFloatBase::semPPCDoubleDouble = {-1, 0, 0, 128};
constexpr fltSemantics APFloatBase::semPPCDoubleDoubleLegacy = {
    1023, -1022 + 53, 53 + 53, 128};
constexpr fltSemantics APFloatBase::semFloat8E5M2 = {15, -14, 3, 8};
constexpr fltSemantics APFloatBase::semFloat8E5M2FNUZ = {
    15, -15, 3, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloatBase::semFloat8E4M3 = {7, -6, 4, 8};
constexpr fltSemantics APFloatBase::semFloat8E4M3FN = {
    8, -6, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::AllOnes};
constexpr fltSemantics APFloatBase::semFloat8E4M3FNUZ = {
    7, -7, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloatBase::semFloat8E4M3B11FNUZ = {
    4, -10, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloatBase::semFloat8E3M4 = {3, -2, 5, 8};
constexpr fltSemantics APFloatBase::semFloatTF32 = {127, -126, 11, 19};
constexpr fltSemantics APFloatBase::semFloat8E8M0FNU = {
    127,
    -127,
    1,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    അസത്യം,
    അസത്യം,
    അസത്യം,
    അസത്യം};

constexpr fltSemantics APFloatBase::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    സത്യം,
    അസത്യം,
    അസത്യം};

constexpr fltSemantics APFloatBase::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semBogus = {0, 0, 0, 0};

സ്ഥിരം llvm::fltSemantics &APFloatBase::EnumToSemantics(Semantics S) {
  തിരഞ്ഞെടുപ്പ് (S) {
  അവസ്ഥ S_IEEEhalf:
    തിരികെ IEEEhalf();
  അവസ്ഥ S_BFloat:
    തിരികെ BFloat();
  അവസ്ഥ S_IEEEsingle:
    തിരികെ IEEEsingle();
  അവസ്ഥ S_IEEEdouble:
    തിരികെ IEEEdouble();
  അവസ്ഥ S_IEEEquad:
    തിരികെ IEEEquad();
  അവസ്ഥ S_x87DoubleExtended:
    തിരികെ x87DoubleExtended();
  അവസ്ഥ S_PPCDoubleDouble:
    തിരികെ PPCDoubleDouble();
  അവസ്ഥ S_PPCDoubleDoubleLegacy:
    തിരികെ PPCDoubleDoubleLegacy();
  അവസ്ഥ S_Float8E5M2:
    തിരികെ Float8E5M2();
  അവസ്ഥ S_Float8E5M2FNUZ:
    തിരികെ Float8E5M2FNUZ();
  അവസ്ഥ S_Float8E4M3:
    തിരികെ Float8E4M3();
  അവസ്ഥ S_Float8E4M3FN:
    തിരികെ Float8E4M3FN();
  അവസ്ഥ S_Float8E4M3FNUZ:
    തിരികെ Float8E4M3FNUZ();
  അവസ്ഥ S_Float8E4M3B11FNUZ:
    തിരികെ Float8E4M3B11FNUZ();
  അവസ്ഥ S_Float8E3M4:
    തിരികെ Float8E3M4();
  അവസ്ഥ S_FloatTF32:
    തിരികെ FloatTF32();
  അവസ്ഥ S_Float8E8M0FNU:
    തിരികെ Float8E8M0FNU();
  അവസ്ഥ S_Float8E5M3FNU:
    തിരികെ Float8E5M3FNU();
  അവസ്ഥ S_Float6E3M2FN:
    തിരികെ Float6E3M2FN();
  അവസ്ഥ S_Float6E2M3FN:
    തിരികെ Float6E2M3FN();
  അവസ്ഥ S_Float4E2M1FN:
    തിരികെ Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloatBase::Semantics
APFloatBase::SemanticsToEnum(സ്ഥിരം llvm::fltSemantics &Sem) {
  എങ്കിൽ (&Sem == &llvm::APFloat::IEEEhalf())
    തിരികെ S_IEEEhalf;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::BFloat())
    തിരികെ S_BFloat;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::IEEEsingle())
    തിരികെ S_IEEEsingle;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::IEEEdouble())
    തിരികെ S_IEEEdouble;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::IEEEquad())
    തിരികെ S_IEEEquad;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::x87DoubleExtended())
    തിരികെ S_x87DoubleExtended;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::PPCDoubleDouble())
    തിരികെ S_PPCDoubleDouble;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    തിരികെ S_PPCDoubleDoubleLegacy;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::Float8E5M2())
    തിരികെ S_Float8E5M2;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    തിരികെ S_Float8E5M2FNUZ;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::Float8E4M3())
    തിരികെ S_Float8E4M3;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::Float8E4M3FN())
    തിരികെ S_Float8E4M3FN;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    തിരികെ S_Float8E4M3FNUZ;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    തിരികെ S_Float8E4M3B11FNUZ;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::Float8E3M4())
    തിരികെ S_Float8E3M4;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::FloatTF32())
    തിരികെ S_FloatTF32;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::Float8E8M0FNU())
    തിരികെ S_Float8E8M0FNU;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::Float8E5M3FNU())
    തിരികെ S_Float8E5M3FNU;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::Float6E3M2FN())
    തിരികെ S_Float6E3M2FN;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::Float6E2M3FN())
    തിരികെ S_Float6E2M3FN;
  അല്ലെങ്കിൽ എങ്കിൽ (&Sem == &llvm::APFloat::Float4E2M1FN())
    തിരികെ S_Float4E2M1FN;
  അല്ലെങ്കിൽ
    llvm_unreachable("Unknown floating semantics");
}

തർക്കം APFloatBase::isRepresentableBy(സ്ഥിരം fltSemantics &A,
                                    സ്ഥിരം fltSemantics &B) {
  തിരികെ A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

തർക്കം APFloatBase::isLosslesslyConvertibleTo(സ്ഥിരം fltSemantics &From,
                                            സ്ഥിരം fltSemantics &InitialTo,
                                            തർക്കം IgnoreNaNs) {
  എങ്കിൽ (&From == &InitialTo)
    തിരികെ സത്യം;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  എങ്കിൽ (&From == &semPPCDoubleDouble)
    തിരികെ അസത്യം;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  സ്ഥിരം fltSemantics &To =
      &InitialTo == &semPPCDoubleDouble ? semIEEEdouble : InitialTo;

  എങ്കിൽ (!isRepresentableBy(From, To))
    തിരികെ അസത്യം;

  എങ്കിൽ ((From.hasZero && !To.hasZero) ||
      (From.hasSignedRepr && !To.hasSignedRepr))
    തിരികെ അസത്യം;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  തർക്കം FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  തർക്കം ToHasSignedZero = To.hasZero && To.hasSignedRepr &&
                         To.nanEncoding != fltNanEncoding::NegativeZero;
  എങ്കിൽ (FromHasSignedZero && !ToHasSignedZero)
    തിരികെ അസത്യം;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat SmallestFrom = APFloat::getSmallest(From);
  തർക്കം LosesInfo = അസത്യം;
  (ശൂന്യം)SmallestFrom.convert(To, APFloat::rmNearestTiesToEven, &LosesInfo);
  എങ്കിൽ (LosesInfo)
    തിരികെ അസത്യം;

  എങ്കിൽ (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    തിരികെ സത്യം;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  എങ്കിൽ (IgnoreNaNs) {
    എങ്കിൽ (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      തിരികെ To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    തിരികെ To.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  എങ്കിൽ (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    തിരികെ അസത്യം;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  എങ്കിൽ (To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    തിരികെ സത്യം;
  തിരികെ To.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
         From.nanEncoding == To.nanEncoding;
}

/* A tight upper bound on number of parts required to hold the value
   pow(5, power) is

     power * 815 / (351 * integerPartWidth) + 1

   However, whilst the result may require only this many parts,
   because we are multiplying two values to get it, the
   multiplication may require an extra part with the excess part
   being zero (consider the trivial case of 1 * 1, tcFullMultiply
   requires two parts to hold the single-part result).  So we add an
   extra one to guarantee enough space whilst multiplying.  */
[[maybe_unused]] constexpr unsigned maxExponent = 16383;

// Decimal exponents in the f16 and f32 ranges need at most 2 and 6 parts.
// Keep 16 parts (128 bytes) inline: enough for 5^413, but not a large stack
// frame. Longer fractional spellings use heap storage.
സ്ഥിതം constexpr unsigned അക്കം_Pow5_Parts_On_അട്ടി = 16;

സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ powerOfFivePartsForExponent(unsigned പൂർണ്ണസംഖ്യ power) {
  പ്രതിജ്ഞാ(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  തിരികെ 2 + ((power * 815) / (351 * APFloatBase::integerPartWidth));
}

unsigned പൂർണ്ണസംഖ്യ APFloatBase::semanticsPrecision(സ്ഥിരം fltSemantics &semantics) {
  തിരികെ semantics.precision;
}
APFloatBase::ExponentType
APFloatBase::semanticsMaxExponent(സ്ഥിരം fltSemantics &semantics) {
  തിരികെ semantics.maxExponent;
}
APFloatBase::ExponentType
APFloatBase::semanticsMinExponent(സ്ഥിരം fltSemantics &semantics) {
  തിരികെ semantics.minExponent;
}
unsigned പൂർണ്ണസംഖ്യ APFloatBase::semanticsSizeInBits(സ്ഥിരം fltSemantics &semantics) {
  തിരികെ semantics.sizeInBits;
}
unsigned പൂർണ്ണസംഖ്യ APFloatBase::semanticsIntSizeInBits(സ്ഥിരം fltSemantics &semantics,
                                                 തർക്കം isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  unsigned പൂർണ്ണസംഖ്യ MinBitWidth = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  എങ്കിൽ (isSigned)
    ++MinBitWidth;
  തിരികെ MinBitWidth;
}

തർക്കം APFloatBase::semanticsHasZero(സ്ഥിരം fltSemantics &semantics) {
  തിരികെ semantics.hasZero;
}

തർക്കം APFloatBase::semanticsHasSignedRepr(സ്ഥിരം fltSemantics &semantics) {
  തിരികെ semantics.hasSignedRepr;
}

തർക്കം APFloatBase::semanticsHasInf(സ്ഥിരം fltSemantics &semantics) {
  തിരികെ semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

തർക്കം APFloatBase::semanticsHasNaN(സ്ഥിരം fltSemantics &semantics) {
  തിരികെ semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

തർക്കം APFloatBase::isIEEELikeFP(സ്ഥിരം fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  തിരികെ SemanticsToEnum(semantics) <= S_IEEEquad;
}

തർക്കം APFloatBase::hasSignBitInMSB(സ്ഥിരം fltSemantics &semantics) {
  തിരികെ semantics.hasSignBitInMSB;
}

തർക്കം APFloatBase::isRepresentableAsNormalIn(സ്ഥിരം fltSemantics &Src,
                                            സ്ഥിരം fltSemantics &Dst) {
  // Exponent range must be larger.
  എങ്കിൽ (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    തിരികെ അസത്യം;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  തിരികെ Dst.precision >= Src.precision;
}

unsigned APFloatBase::getSizeInBits(സ്ഥിരം fltSemantics &Sem) {
  തിരികെ Sem.sizeInBits;
}

സ്ഥിതം constexpr APFloatBase::ExponentType
exponentZero(സ്ഥിരം fltSemantics &semantics) {
  തിരികെ semantics.minExponent - 1;
}

സ്ഥിതം constexpr APFloatBase::ExponentType
exponentInf(സ്ഥിരം fltSemantics &semantics) {
  തിരികെ semantics.maxExponent + 1;
}

സ്ഥിതം constexpr APFloatBase::ExponentType
exponentNaN(സ്ഥിരം fltSemantics &semantics) {
  എങ്കിൽ (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    എങ്കിൽ (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      തിരികെ exponentZero(semantics);
    എങ്കിൽ (semantics.hasSignedRepr || semantics.precision > 1)
      തിരികെ semantics.maxExponent;
  }
  തിരികെ semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

സ്ഥിതം വരിയിൽ Error createError(സ്ഥിരം Twine &Err) {
  തിരികെ make_error<StringError>(Err, inconvertibleErrorCode());
}

സ്ഥിതം constexpr വരിയിൽ unsigned പൂർണ്ണസംഖ്യ part_എണ്ണം_For_Bits(unsigned പൂർണ്ണസംഖ്യ bits) {
  തിരികെ std::max(1u, (bits + APFloatBase::integerPartWidth - 1) /
                          APFloatBase::integerPartWidth);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
സ്ഥിതം വരിയിൽ unsigned പൂർണ്ണസംഖ്യ
dec_Digit_മൂല്യം(unsigned പൂർണ്ണസംഖ്യ c)
{
  തിരികെ c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
സ്ഥിതം Expected<പൂർണ്ണസംഖ്യ> readExponent(StringRef::iterator begin,
                                  StringRef::iterator end) {
  സ്ഥിരം unsigned പൂർണ്ണസംഖ്യ overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  എങ്കിൽ (p == end || ((*p == '-' || *p == '+') && (p + 1) == end))
    തിരികെ 0;

  തർക്കം isNegative = *p == '-';
  എങ്കിൽ (*p == '-' || *p == '+') {
    p++;
    എങ്കിൽ (p == end)
      തിരികെ createError("Exponent has no digits");
  }

  unsigned absExponent = dec_Digit_മൂല്യം(*p++);
  എങ്കിൽ (absExponent >= 10U)
    തിരികെ createError("Invalid character in exponent");

  വേണ്ടി (; p != end; ++p) {
    unsigned മൂല്യം_2 = dec_Digit_മൂല്യം(*p);
    എങ്കിൽ (മൂല്യം_2 >= 10U)
      തിരികെ createError("Invalid character in exponent");

    absExponent = absExponent * 10U + മൂല്യം_2;
    എങ്കിൽ (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      നിർത്തുക;
    }
  }

  എങ്കിൽ (isNegative)
    തിരികെ -(പൂർണ്ണസംഖ്യ) absExponent;
  അല്ലെങ്കിൽ
    തിരികെ (പൂർണ്ണസംഖ്യ) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
സ്ഥിതം Expected<പൂർണ്ണസംഖ്യ> totalExponent(StringRef::iterator p,
                                   StringRef::iterator end,
                                   പൂർണ്ണസംഖ്യ exponentAdjustment) {
  പൂർണ്ണസംഖ്യ exponent = 0;

  എങ്കിൽ (p == end)
    തിരികെ createError("Exponent has no digits");

  തർക്കം negative = *p == '-';
  എങ്കിൽ (*p == '-' || *p == '+') {
    p++;
    എങ്കിൽ (p == end)
      തിരികെ createError("Exponent has no digits");
  }

  പൂർണ്ണസംഖ്യ unsignedExponent = 0;
  തർക്കം overflow = അസത്യം;
  വേണ്ടി (; p != end; ++p) {
    unsigned പൂർണ്ണസംഖ്യ മൂല്യം_2;

    മൂല്യം_2 = dec_Digit_മൂല്യം(*p);
    എങ്കിൽ (മൂല്യം_2 >= 10U)
      തിരികെ createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + മൂല്യം_2;
    എങ്കിൽ (unsignedExponent > 32767) {
      overflow = സത്യം;
      നിർത്തുക;
    }
  }

  എങ്കിൽ (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    overflow = സത്യം;

  എങ്കിൽ (!overflow) {
    exponent = unsignedExponent;
    എങ്കിൽ (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    എങ്കിൽ (exponent > 32767 || exponent < -32768)
      overflow = സത്യം;
  }

  എങ്കിൽ (overflow)
    exponent = negative ? -32768: 32767;

  തിരികെ exponent;
}

സ്ഥിതം Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator end,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = end;
  ആകുമ്പോൾ (p != end && *p == '0')
    p++;

  എങ്കിൽ (p != end && *p == '.') {
    *dot = p++;

    എങ്കിൽ (end - begin == 1)
      തിരികെ createError("Significand has no digits");

    ആകുമ്പോൾ (p != end && *p == '0')
      p++;
  }

  തിരികെ p;
}

/* Given a normal decimal floating point number of the form

     dddd.dddd[eE][+-]ddd

   where the decimal point and exponent are optional, fill out the
   structure D.  Exponent is appropriate if the significand is
   treated as an integer, and normalizedExponent if the significand
   is taken to have the decimal point after a single leading
   non-zero digit.

   If the value is zero, V->firstSigDigit points to a non-digit, and
   the return exponent is zero.
*/
ഘടന decimalInfo {
  സ്ഥിരം അക്ഷരം *firstSigDigit;
  സ്ഥിരം അക്ഷരം *lastSigDigit;
  പൂർണ്ണസംഖ്യ exponent;
  പൂർണ്ണസംഖ്യ normalizedExponent;
};

സ്ഥിതം Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator end, decimalInfo *D) {
  StringRef::iterator dot = end;

  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  എങ്കിൽ (!PtrOrErr)
    തിരികെ PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->firstSigDigit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  വേണ്ടി (; p != end; ++p) {
    എങ്കിൽ (*p == '.') {
      എങ്കിൽ (dot != end)
        തിരികെ createError("String contains multiple dots");
      dot = p++;
      എങ്കിൽ (p == end)
        നിർത്തുക;
    }
    എങ്കിൽ (dec_Digit_മൂല്യം(*p) >= 10U)
      നിർത്തുക;
  }

  എങ്കിൽ (p != end) {
    എങ്കിൽ (*p != 'e' && *p != 'E')
      തിരികെ createError("Invalid character in significand");
    എങ്കിൽ (p == begin)
      തിരികെ createError("Significand has no digits");
    എങ്കിൽ (dot != end && p - begin == 1)
      തിരികെ createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    auto ExpOrErr = readExponent(p + 1, end);
    എങ്കിൽ (!ExpOrErr)
      തിരികെ ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    എങ്കിൽ (dot == end)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  എങ്കിൽ (p != D->firstSigDigit) {
    /* Drop insignificant trailing zeroes.  */
    എങ്കിൽ (p != begin) {
      ചെയ്യുക
        ചെയ്യുക
          p--;
        ആകുമ്പോൾ (p != begin && *p == '0');
      ആകുമ്പോൾ (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->firstSigDigit)
                                      - (dot > D->firstSigDigit && dot < p)));
  }

  D->lastSigDigit = p;
  തിരികെ Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
സ്ഥിതം Expected<lostFraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator end,
                            unsigned പൂർണ്ണസംഖ്യ digit_മൂല്യം) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  എങ്കിൽ (digit_മൂല്യം > 8)
    തിരികെ lfMoreThanHalf;
  അല്ലെങ്കിൽ എങ്കിൽ (digit_മൂല്യം < 8 && digit_മൂല്യം > 0)
    തിരികെ lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  ആകുമ്പോൾ (p != end && (*p == '0' || *p == '.'))
    p++;

  എങ്കിൽ (p == end)
    തിരികെ createError("Invalid trailing hexadecimal fraction!");

  unsigned hexDigit = hex_Digit_മൂല്യം(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  എങ്കിൽ (hexDigit == UINT_MAX)
    തിരികെ digit_മൂല്യം == 0 ? lfExactlyZero: lfExactlyHalf;
  അല്ലെങ്കിൽ
    തിരികെ digit_മൂല്യം == 0 ? lfLessThanHalf: lfMoreThanHalf;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
സ്ഥിതം lostFraction
lostFractionThroughTruncation(സ്ഥിരം APFloatBase::integerPart *parts,
                              unsigned പൂർണ്ണസംഖ്യ part_എണ്ണം,
                              unsigned പൂർണ്ണസംഖ്യ bits)
{
  unsigned lsb = APInt::tcLSB(parts, part_എണ്ണം);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  എങ്കിൽ (bits <= lsb)
    തിരികെ lfExactlyZero;
  എങ്കിൽ (bits == lsb + 1)
    തിരികെ lfExactlyHalf;
  എങ്കിൽ (bits <= part_എണ്ണം * APFloatBase::integerPartWidth &&
      APInt::tcExtractBit(parts, bits - 1))
    തിരികെ lfMoreThanHalf;

  തിരികെ lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
സ്ഥിതം lostFraction
shift_വലത്(APFloatBase::integerPart *dst, unsigned പൂർണ്ണസംഖ്യ parts, unsigned പൂർണ്ണസംഖ്യ bits)
{
  lostFraction lost_fraction = lostFractionThroughTruncation(dst, parts, bits);

  APInt::tc_Shift_വലത്(dst, parts, bits);

  തിരികെ lost_fraction;
}

/* Combine the effect of two lost fractions.  */
സ്ഥിതം lostFraction
combineLostFractions(lostFraction moreSignificant,
                     lostFraction lessSignificant)
{
  എങ്കിൽ (lessSignificant != lfExactlyZero) {
    എങ്കിൽ (moreSignificant == lfExactlyZero)
      moreSignificant = lfLessThanHalf;
    അല്ലെങ്കിൽ എങ്കിൽ (moreSignificant == lfExactlyHalf)
      moreSignificant = lfMoreThanHalf;
  }

  തിരികെ moreSignificant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ
HUerrBound(തർക്കം inexactMultiply, unsigned പൂർണ്ണസംഖ്യ HUerr1, unsigned പൂർണ്ണസംഖ്യ HUerr2)
{
  പ്രതിജ്ഞാ(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  എങ്കിൽ (HUerr1 + HUerr2 == 0)
    തിരികെ inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  അല്ലെങ്കിൽ
    തിരികെ inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
സ്ഥിതം APFloatBase::integerPart
ulpsFromBoundary(സ്ഥിരം APFloatBase::integerPart *parts, unsigned പൂർണ്ണസംഖ്യ bits,
                 തർക്കം isNearest) {
  പ്രതിജ്ഞാ(bits != 0);

  bits--;
  unsigned എണ്ണം = bits / APFloatBase::integerPartWidth;
  unsigned partBits = bits % APFloatBase::integerPartWidth + 1;

  APFloatBase::integerPart part =
      parts[എണ്ണം] & (~(APFloatBase::integerPart)0 >>
                      (APFloatBase::integerPartWidth - partBits));

  APFloatBase::integerPart boundary;
  എങ്കിൽ (isNearest)
    boundary = (APFloatBase::integerPart) 1 << (partBits - 1);
  അല്ലെങ്കിൽ
    boundary = 0;

  എങ്കിൽ (എണ്ണം == 0) {
    എങ്കിൽ (part - boundary <= boundary - part)
      തിരികെ part - boundary;
    അല്ലെങ്കിൽ
      തിരികെ boundary - part;
  }

  എങ്കിൽ (part == boundary) {
    ആകുമ്പോൾ (--എണ്ണം)
      എങ്കിൽ (parts[എണ്ണം])
        തിരികെ ~(APFloatBase::integerPart) 0; /* A lot.  */

    തിരികെ parts[0];
  } അല്ലെങ്കിൽ എങ്കിൽ (part == boundary - 1) {
    ആകുമ്പോൾ (--എണ്ണം)
      എങ്കിൽ (~parts[എണ്ണം])
        തിരികെ ~(APFloatBase::integerPart) 0; /* A lot.  */

    തിരികെ -parts[0];
  }

  തിരികെ ~(APFloatBase::integerPart) 0; /* A lot.  */
}

സ്ഥിതം constexpr unsigned PowerOfFivePartCounts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

സ്ഥിതം constexpr APFloatBase::integerPart PowerOfFiveParts[] = {
#അന്തര്ഭാവ "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  unsigned Sum = 0;
  വേണ്ടി (unsigned എണ്ണം_2 : PowerOfFivePartCounts)
    Sum += എണ്ണം_2;
  തിരികെ Sum;
}() == std::size(PowerOfFiveParts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ
powerOf5(APFloatBase::integerPart *dst, unsigned പൂർണ്ണസംഖ്യ power) {
  സ്ഥിതം സ്ഥിരം APFloatBase::integerPart firstEightPowers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  SmallVector<APFloatBase::integerPart, അക്കം_Pow5_Parts_On_അട്ടി> scratch;
  scratch.resize_for_overwrite(powerOfFivePartsForExponent(power));

  APFloatBase::integerPart *p1, *p2;
  സ്ഥിരം APFloatBase::integerPart *pow5;

  p1 = dst;
  p2 = scratch.data();

  *p1 = firstEightPowers[power & 7];
  power >>= 3;

  unsigned result = 1;
  pow5 = PowerOfFiveParts;

  വേണ്ടി (unsigned പൂർണ്ണസംഖ്യ n = 0; power; power >>= 1, n++) {
    unsigned parts_എണ്ണം = PowerOfFivePartCounts[n];

    എങ്കിൽ (power & 1) {
      APFloatBase::integerPart *tmp;

      APInt::tcFullMultiply(p2, p1, pow5, result, parts_എണ്ണം);
      result += parts_എണ്ണം;
      എങ്കിൽ (p2[result - 1] == 0)
        result--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_എണ്ണം;
  }

  എങ്കിൽ (p1 != dst)
    APInt::tcAssign(dst, p1, result);

  തിരികെ result;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
സ്ഥിതം സ്ഥിരം അക്ഷരം hexDigitsLower[] = "0123456789abcdef0";
സ്ഥിതം സ്ഥിരം അക്ഷരം hexDigitsUpper[] = "0123456789ABCDEF0";
സ്ഥിതം സ്ഥിരം അക്ഷരം infinityL[] = "infinity";
സ്ഥിതം സ്ഥിരം അക്ഷരം infinityU[] = "INFINITY";
സ്ഥിതം സ്ഥിരം അക്ഷരം NaNL[] = "nan";
സ്ഥിതം സ്ഥിരം അക്ഷരം NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ
partAsHex (അക്ഷരം *dst, APFloatBase::integerPart part, unsigned പൂർണ്ണസംഖ്യ എണ്ണം,
           സ്ഥിരം അക്ഷരം *hexDigitChars)
{
  unsigned പൂർണ്ണസംഖ്യ result = എണ്ണം;

  പ്രതിജ്ഞാ(എണ്ണം != 0 && എണ്ണം <= APFloatBase::integerPartWidth / 4);

  part >>= (APFloatBase::integerPartWidth - 4 * എണ്ണം);
  ആകുമ്പോൾ (എണ്ണം--) {
    dst[എണ്ണം] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  തിരികെ result;
}

/* Write out an unsigned decimal integer.  */
സ്ഥിതം അക്ഷരം *writeUnsignedDecimal(അക്ഷരം *dst, unsigned പൂർണ്ണസംഖ്യ n) {
  അക്ഷരം buff[40], *p;

  p = buff;
  ചെയ്യുക
    *p++ = '0' + n % 10;
  ആകുമ്പോൾ (n /= 10);

  ചെയ്യുക
    *dst++ = *--p;
  ആകുമ്പോൾ (p != buff);

  തിരികെ dst;
}

/* Write out a signed decimal integer.  */
സ്ഥിതം അക്ഷരം *writeSignedDecimal(അക്ഷരം *dst, പൂർണ്ണസംഖ്യ മൂല്യം_2) {
  എങ്കിൽ (മൂല്യം_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(unsigned) മൂല്യം_2);
  } അല്ലെങ്കിൽ {
    dst = writeUnsignedDecimal(dst, മൂല്യം_2);
  }

  തിരികെ dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
സ്ഥിതം APFloat harrisonUlp(സ്ഥിരം APFloat &X) {
  സ്ഥിരം fltSemantics &Sem = X.getSemantics();
  തിരഞ്ഞെടുപ്പ് (X.getCategory()) {
  അവസ്ഥ APFloat::fcNaN:
    തിരികെ APFloat::getQNaN(Sem);
  അവസ്ഥ APFloat::fcInfinity:
    തിരികെ APFloat::getInf(Sem);
  അവസ്ഥ APFloat::fcZero:
    തിരികെ APFloat::getSmallest(Sem);
  അവസ്ഥ APFloat::fcNormal:
    നിർത്തുക;
  }
  എങ്കിൽ (X.isDenormal() || X.isSmallestNormalized())
    തിരികെ APFloat::getSmallest(Sem);
  പൂർണ്ണസംഖ്യ Exp = ilogb(X);
  എങ്കിൽ (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  തിരികെ scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rmNearestTiesToEven);
}

നാമാകാശ detail {
/* Constructors.  */
ശൂന്യം IEEEFloat::initialize(സ്ഥിരം fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  unsigned എണ്ണം = part_എണ്ണം();
  എങ്കിൽ (എണ്ണം > 1)
    significand.parts = പുതിയ integerPart[എണ്ണം];
}

ശൂന്യം IEEEFloat::freeSignificand() {
  എങ്കിൽ (needsCleanup())
    delete [] significand.parts;
}

ശൂന്യം IEEEFloat::assign(സ്ഥിരം IEEEFloat &rhs) {
  പ്രതിജ്ഞാ(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  എങ്കിൽ (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

ശൂന്യം IEEEFloat::copySignificand(സ്ഥിരം IEEEFloat &rhs) {
  പ്രതിജ്ഞാ(isFiniteNonZero() || category == fcNaN);
  പ്രതിജ്ഞാ(rhs.part_എണ്ണം() >= part_എണ്ണം());

  APInt::tcAssign(significandParts(), rhs.significandParts(),
                  part_എണ്ണം());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
ശൂന്യം IEEEFloat::makeNaN(തർക്കം SNaN, തർക്കം Negative, സ്ഥിരം APInt *fill) {
  എങ്കിൽ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  എങ്കിൽ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  unsigned അക്കം_Parts = part_എണ്ണം();

  APInt fill_storage;
  എങ്കിൽ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = അസത്യം;
    എങ്കിൽ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = സത്യം;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } അല്ലെങ്കിൽ {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  എങ്കിൽ (!fill || fill->get_അക്കം_വാക്കുകൾ() < അക്കം_Parts)
    APInt::tcSet(significand, 0, അക്കം_Parts);
  എങ്കിൽ (fill) {
    APInt::tcAssign(significand, fill->getRawData(),
                    std::min(fill->get_അക്കം_വാക്കുകൾ(), അക്കം_Parts));

    // Zero out the excess bits of the significand.
    unsigned bitsToPreserve = semantics->precision - 1;
    unsigned part = bitsToPreserve / 64;
    bitsToPreserve %= 64;
    significand[part] &= ((1ULL << bitsToPreserve) - 1);
    വേണ്ടി (part++; part != അക്കം_Parts; ++part)
      significand[part] = 0;
  }

  unsigned QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  എങ്കിൽ (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tcClearBit(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    എങ്കിൽ (APInt::tcIsZero(significand, അക്കം_Parts))
      APInt::tcSetBit(significand, QNaNBit - 1);
  } അല്ലെങ്കിൽ എങ്കിൽ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } അല്ലെങ്കിൽ {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tcSetBit(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  എങ്കിൽ (semantics == &APFloatBase::semX87DoubleExtended)
    APInt::tcSetBit(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::operator=(സ്ഥിരം IEEEFloat &rhs) {
  എങ്കിൽ (ഇത് != &rhs) {
    എങ്കിൽ (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    assign(rhs);
  }

  തിരികെ *ഇത്;
}

IEEEFloat &IEEEFloat::operator=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloatBase::semBogus;
  തിരികെ *ഇത്;
}

തർക്കം IEEEFloat::isDenormal() സ്ഥിരം {
  തിരികെ getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tcExtractBit(significandParts(), semantics->precision - 1) ==
          0);
}

തർക്കം IEEEFloat::isSmallest() സ്ഥിരം {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  തിരികെ isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

തർക്കം IEEEFloat::isSmallestNormalized() സ്ഥിരം {
  തിരികെ getCategory() == fcNormal && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

unsigned പൂർണ്ണസംഖ്യ IEEEFloat::get_അക്കം_മുകളിലെ_Bits() സ്ഥിരം {
  സ്ഥിരം unsigned പൂർണ്ണസംഖ്യ Part_എണ്ണം = part_എണ്ണം_For_Bits(semantics->precision);
  സ്ഥിരം unsigned പൂർണ്ണസംഖ്യ Bits = Part_എണ്ണം * integerPartWidth;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  സ്ഥിരം unsigned പൂർണ്ണസംഖ്യ അക്കം_മുകളിലെ_Bits = (semantics->precision > 1)
                                       ? (Bits - semantics->precision + 1)
                                       : (Bits - semantics->precision);
  തിരികെ അക്കം_മുകളിലെ_Bits;
}

തർക്കം IEEEFloat::isSignificandAllOnes() സ്ഥിരം {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  സ്ഥിരം integerPart *Parts = significandParts();
  സ്ഥിരം unsigned Part_എണ്ണം = part_എണ്ണം_For_Bits(semantics->precision);
  വേണ്ടി (unsigned i = 0; i < Part_എണ്ണം - 1; i++)
    എങ്കിൽ (~Parts[i])
      തിരികെ അസത്യം;

  // Set the unused high bits to all ones when we compare.
  സ്ഥിരം unsigned അക്കം_മുകളിലെ_Bits = get_അക്കം_മുകളിലെ_Bits();
  പ്രതിജ്ഞാ(അക്കം_മുകളിലെ_Bits <= integerPartWidth && അക്കം_മുകളിലെ_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  സ്ഥിരം integerPart മുകളിലെ_Bit_Fill =
    ~integerPart(0) << (integerPartWidth - അക്കം_മുകളിലെ_Bits);
  എങ്കിൽ ((semantics->precision <= 1) || (~(Parts[Part_എണ്ണം - 1] | മുകളിലെ_Bit_Fill)))
    തിരികെ അസത്യം;

  തിരികെ സത്യം;
}

തർക്കം IEEEFloat::isSignificandAllOnesExceptLSB() സ്ഥിരം {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  സ്ഥിരം integerPart *Parts = significandParts();

  എങ്കിൽ (Parts[0] & 1)
    തിരികെ അസത്യം;

  സ്ഥിരം unsigned Part_എണ്ണം = part_എണ്ണം_For_Bits(semantics->precision);
  വേണ്ടി (unsigned i = 0; i < Part_എണ്ണം - 1; i++) {
    എങ്കിൽ (~Parts[i] & ~unsigned{!i})
      തിരികെ അസത്യം;
  }

  // Set the unused high bits to all ones when we compare.
  സ്ഥിരം unsigned അക്കം_മുകളിലെ_Bits = get_അക്കം_മുകളിലെ_Bits();
  പ്രതിജ്ഞാ(അക്കം_മുകളിലെ_Bits <= integerPartWidth && അക്കം_മുകളിലെ_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  സ്ഥിരം integerPart മുകളിലെ_Bit_Fill = ~integerPart(0)
                                  << (integerPartWidth - അക്കം_മുകളിലെ_Bits);
  എങ്കിൽ (~(Parts[Part_എണ്ണം - 1] | മുകളിലെ_Bit_Fill | 0x1))
    തിരികെ അസത്യം;

  തിരികെ സത്യം;
}

തർക്കം IEEEFloat::isSignificandAllZeros() സ്ഥിരം {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  സ്ഥിരം integerPart *Parts = significandParts();
  സ്ഥിരം unsigned Part_എണ്ണം = part_എണ്ണം_For_Bits(semantics->precision);

  വേണ്ടി (unsigned i = 0; i < Part_എണ്ണം - 1; i++)
    എങ്കിൽ (Parts[i])
      തിരികെ അസത്യം;

  // Compute how many bits are used in the final word.
  സ്ഥിരം unsigned അക്കം_മുകളിലെ_Bits = get_അക്കം_മുകളിലെ_Bits();
  പ്രതിജ്ഞാ(അക്കം_മുകളിലെ_Bits < integerPartWidth && "Can not have more high bits to "
         "clear than integerPartWidth");
  സ്ഥിരം integerPart മുകളിലെ_Bit_Mask = ~integerPart(0) >> അക്കം_മുകളിലെ_Bits;

  എങ്കിൽ ((semantics->precision > 1) && (Parts[Part_എണ്ണം - 1] & മുകളിലെ_Bit_Mask))
    തിരികെ അസത്യം;

  തിരികെ സത്യം;
}

തർക്കം IEEEFloat::isSignificandAllZerosExceptMSB() സ്ഥിരം {
  സ്ഥിരം integerPart *Parts = significandParts();
  സ്ഥിരം unsigned Part_എണ്ണം = part_എണ്ണം_For_Bits(semantics->precision);

  വേണ്ടി (unsigned i = 0; i < Part_എണ്ണം - 1; i++) {
    എങ്കിൽ (Parts[i])
      തിരികെ അസത്യം;
  }

  സ്ഥിരം unsigned അക്കം_മുകളിലെ_Bits = get_അക്കം_മുകളിലെ_Bits();
  സ്ഥിരം integerPart MSBMask = integerPart(1)
                              << (integerPartWidth - അക്കം_മുകളിലെ_Bits);
  തിരികെ ((semantics->precision <= 1) || (Parts[Part_എണ്ണം - 1] == MSBMask));
}

തർക്കം IEEEFloat::isLargest() സ്ഥിരം {
  തർക്കം IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  എങ്കിൽ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    തിരികെ (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } അല്ലെങ്കിൽ {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    തിരികെ IsMaxExp && isSignificandAllOnes();
  }
}

തർക്കം IEEEFloat::isInteger() സ്ഥിരം {
  // This could be made more efficient; I'm going for obviously correct.
  എങ്കിൽ (!isFinite()) തിരികെ അസത്യം;
  IEEEFloat truncated = *ഇത്;
  truncated.roundToIntegral(rmTowardZero);
  തിരികെ compare(truncated) == cmpEqual;
}

തർക്കം IEEEFloat::bitwiseIsEqual(സ്ഥിരം IEEEFloat &rhs) സ്ഥിരം {
  എങ്കിൽ (ഇത് == &rhs)
    തിരികെ സത്യം;
  എങ്കിൽ (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    തിരികെ അസത്യം;
  എങ്കിൽ (category==fcZero || category==fcInfinity)
    തിരികെ സത്യം;

  എങ്കിൽ (isFiniteNonZero() && exponent != rhs.exponent)
    തിരികെ അസത്യം;

  തിരികെ std::equal(significandParts(), significandParts() + part_എണ്ണം(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(സ്ഥിരം fltSemantics &ourSemantics, integerPart മൂല്യം_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fcNormal;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = മൂല്യം_2;
  normalize(rmNearestTiesToEven, lfExactlyZero);
}

IEEEFloat::IEEEFloat(സ്ഥിരം fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(അസത്യം) : makeSmallestNormalized(അസത്യം);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(സ്ഥിരം fltSemantics &ourSemantics, uninitializedTag tag)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(സ്ഥിരം IEEEFloat &rhs) {
  initialize(rhs.semantics);
  assign(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloatBase::semBogus) {
  *ഇത് = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

unsigned പൂർണ്ണസംഖ്യ IEEEFloat::part_എണ്ണം() സ്ഥിരം {
  തിരികെ part_എണ്ണം_For_Bits(semantics->precision + 1);
}

സ്ഥിരം APFloat::integerPart *IEEEFloat::significandParts() സ്ഥിരം {
  തിരികെ const_cast<IEEEFloat *>(ഇത്)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  എങ്കിൽ (part_എണ്ണം() > 1)
    തിരികെ significand.parts;
  അല്ലെങ്കിൽ
    തിരികെ &significand.part;
}

ശൂന്യം IEEEFloat::zeroSignificand() {
  APInt::tcSet(significandParts(), 0, part_എണ്ണം());
}

/* Increment an fcNormal floating point number's significand.  */
ശൂന്യം IEEEFloat::incrementSignificand() {
  [[maybe_unused]] integerPart carry =
      APInt::tcIncrement(significandParts(), part_എണ്ണം());

  /* Our callers should never cause us to overflow.  */
  പ്രതിജ്ഞാ(carry == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::addSignificand(സ്ഥിരം IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  പ്രതിജ്ഞാ(semantics == rhs.semantics);
  പ്രതിജ്ഞാ(exponent == rhs.exponent);

  തിരികെ APInt::tcAdd(parts, rhs.significandParts(), 0, part_എണ്ണം());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::subtractSignificand(സ്ഥിരം IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  പ്രതിജ്ഞാ(semantics == rhs.semantics);
  പ്രതിജ്ഞാ(exponent == rhs.exponent);

  തിരികെ APInt::tcSubtract(parts, rhs.significandParts(), borrow,
                           part_എണ്ണം());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
lostFraction IEEEFloat::multiplySignificand(സ്ഥിരം IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            തർക്കം ignoreAddend) {
  integerPart scratch[4];
  തർക്കം ignored;

  പ്രതിജ്ഞാ(semantics == rhs.semantics);

  unsigned precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  unsigned new_Parts_എണ്ണം = part_എണ്ണം_For_Bits(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *fullSignificand =
      new_Parts_എണ്ണം > 4 ? പുതിയ integerPart[new_Parts_എണ്ണം] : scratch;

  integerPart *lhsSignificand = significandParts();
  unsigned parts_എണ്ണം = part_എണ്ണം();

  APInt::tcFullMultiply(fullSignificand, lhsSignificand,
                        rhs.significandParts(), parts_എണ്ണം, parts_എണ്ണം);

  lostFraction lost_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  unsigned omsb = APInt::tcMSB(fullSignificand, new_Parts_എണ്ണം) + 1;
  exponent += rhs.exponent;

  // Assume the operands involved in the multiplication are single-precision
  // FP, and the two multiplicants are:
  //   *this = a23 . a22 ... a0 * 2^e1
  //     rhs = b23 . b22 ... b0 * 2^e2
  // the result of multiplication is:
  //   *this = c48 c47 c46 . c45 ... c0 * 2^(e1+e2)
  // Note that there are three significant bits at the left-hand side of the
  // radix point: two for the multiplication, and an overflow bit for the
  // addition (that will always be zero at this point). Move the radix point
  // toward left by two bits, and adjust exponent accordingly.
  exponent += 2;

  എങ്കിൽ (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    സ്ഥിരം fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    unsigned extendedPrecision = 2 * precision + 1;
    എങ്കിൽ (omsb != extendedPrecision - 1) {
      പ്രതിജ്ഞാ(extendedPrecision > omsb);
      APInt::tc_Shift_ഇടത്(fullSignificand, new_Parts_എണ്ണം,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    എങ്കിൽ (new_Parts_എണ്ണം == 1)
      significand.part = fullSignificand[0];
    അല്ലെങ്കിൽ
      significand.parts = fullSignificand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_unused]] opStatus status = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    പ്രതിജ്ഞാ(status == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    lost_fraction = extendedAddend.shift_Significand_വലത്(1);
    പ്രതിജ്ഞാ(lost_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    lost_fraction = addOrSubtractSignificand(extendedAddend, അസത്യം);

    /* Restore our state.  */
    എങ്കിൽ (new_Parts_എണ്ണം == 1)
      fullSignificand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(fullSignificand, new_Parts_എണ്ണം) + 1;
  }

  // Convert the result having "2 * precision" significant-bits back to the one
  // having "precision" significant-bits. First, move the radix point from
  // poision "2*precision - 1" to "precision - 1". The exponent need to be
  // adjusted by "2*precision - 1" - "precision - 1" = "precision".
  exponent -= precision + 1;

  // In case MSB resides at the left-hand side of radix point, shift the
  // mantissa right by some amount to make sure the MSB reside right before
  // the radix point (i.e. "MSB . rest-significant-bits").
  //
  // Note that the result is not normalized when "omsb < precision". So, the
  // caller needs to call IEEEFloat::normalize() if normalized value is
  // expected.
  എങ്കിൽ (omsb > precision) {
    unsigned പൂർണ്ണസംഖ്യ bits, significantParts;
    lostFraction lf;

    bits = omsb - precision;
    significantParts = part_എണ്ണം_For_Bits(omsb);
    lf = shift_വലത്(fullSignificand, significantParts, bits);
    lost_fraction = combineLostFractions(lf, lost_fraction);
    exponent += bits;
  }

  APInt::tcAssign(lhsSignificand, fullSignificand, parts_എണ്ണം);

  എങ്കിൽ (new_Parts_എണ്ണം > 4)
    delete [] fullSignificand;

  തിരികെ lost_fraction;
}

lostFraction IEEEFloat::multiplySignificand(സ്ഥിരം IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  തിരികെ multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
lostFraction IEEEFloat::divideSignificand(സ്ഥിരം IEEEFloat &rhs) {
  integerPart scratch[4];

  പ്രതിജ്ഞാ(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  സ്ഥിരം integerPart *rhsSignificand = rhs.significandParts();
  unsigned parts_എണ്ണം = part_എണ്ണം();

  integerPart *dividend =
      parts_എണ്ണം > 2 ? പുതിയ integerPart[parts_എണ്ണം * 2] : scratch;
  integerPart *ഹാരകം = dividend + parts_എണ്ണം;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  വേണ്ടി (unsigned i = 0; i < parts_എണ്ണം; i++) {
    dividend[i] = lhsSignificand[i];
    ഹാരകം[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  unsigned പൂർണ്ണസംഖ്യ precision = semantics->precision;

  /* Normalize the divisor.  */
  unsigned bit = precision - APInt::tcMSB(ഹാരകം, parts_എണ്ണം) - 1;
  എങ്കിൽ (bit) {
    exponent += bit;
    APInt::tc_Shift_ഇടത്(ഹാരകം, parts_എണ്ണം, bit);
  }

  /* Normalize the dividend.  */
  bit = precision - APInt::tcMSB(dividend, parts_എണ്ണം) - 1;
  എങ്കിൽ (bit) {
    exponent -= bit;
    APInt::tc_Shift_ഇടത്(dividend, parts_എണ്ണം, bit);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  എങ്കിൽ (APInt::tcCompare(dividend, ഹാരകം, parts_എണ്ണം) < 0) {
    exponent--;
    APInt::tc_Shift_ഇടത്(dividend, parts_എണ്ണം, 1);
    പ്രതിജ്ഞാ(APInt::tcCompare(dividend, ഹാരകം, parts_എണ്ണം) >= 0);
  }

  /* Long division.  */
  വേണ്ടി (bit = precision; bit; bit -= 1) {
    എങ്കിൽ (APInt::tcCompare(dividend, ഹാരകം, parts_എണ്ണം) >= 0) {
      APInt::tcSubtract(dividend, ഹാരകം, 0, parts_എണ്ണം);
      APInt::tcSetBit(lhsSignificand, bit - 1);
    }

    APInt::tc_Shift_ഇടത്(dividend, parts_എണ്ണം, 1);
  }

  /* Figure out the lost fraction.  */
  പൂർണ്ണസംഖ്യ cmp = APInt::tcCompare(dividend, ഹാരകം, parts_എണ്ണം);

  lostFraction lost_fraction;
  എങ്കിൽ (cmp > 0)
    lost_fraction = lfMoreThanHalf;
  അല്ലെങ്കിൽ എങ്കിൽ (cmp == 0)
    lost_fraction = lfExactlyHalf;
  അല്ലെങ്കിൽ എങ്കിൽ (APInt::tcIsZero(dividend, parts_എണ്ണം))
    lost_fraction = lfExactlyZero;
  അല്ലെങ്കിൽ
    lost_fraction = lfLessThanHalf;

  എങ്കിൽ (parts_എണ്ണം > 2)
    delete [] dividend;

  തിരികെ lost_fraction;
}

unsigned പൂർണ്ണസംഖ്യ IEEEFloat::significandMSB() സ്ഥിരം {
  തിരികെ APInt::tcMSB(significandParts(), part_എണ്ണം());
}

unsigned പൂർണ്ണസംഖ്യ IEEEFloat::significandLSB() സ്ഥിരം {
  തിരികെ APInt::tcLSB(significandParts(), part_എണ്ണം());
}

/* Note that a zero result is NOT normalized to fcZero.  */
lostFraction IEEEFloat::shift_Significand_വലത്(unsigned പൂർണ്ണസംഖ്യ bits) {
  /* Our exponent should not overflow.  */
  പ്രതിജ്ഞാ((ExponentType) (exponent + bits) >= exponent);

  exponent += bits;

  തിരികെ shift_വലത്(significandParts(), part_എണ്ണം(), bits);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
ശൂന്യം IEEEFloat::shift_Significand_ഇടത്(unsigned പൂർണ്ണസംഖ്യ bits) {
  പ്രതിജ്ഞാ(bits < semantics->precision ||
         (semantics->precision == 1 && bits <= 1));

  എങ്കിൽ (bits) {
    unsigned പൂർണ്ണസംഖ്യ parts_എണ്ണം = part_എണ്ണം();

    APInt::tc_Shift_ഇടത്(significandParts(), parts_എണ്ണം, bits);
    exponent -= bits;

    പ്രതിജ്ഞാ(!APInt::tcIsZero(significandParts(), parts_എണ്ണം));
  }
}

APFloat::cmpResult IEEEFloat::compare_Absolute_മൂല്യം(സ്ഥിരം IEEEFloat &rhs) സ്ഥിരം {
  പ്രതിജ്ഞാ(semantics == rhs.semantics);
  പ്രതിജ്ഞാ(isFiniteNonZero());
  പ്രതിജ്ഞാ(rhs.isFiniteNonZero());

  പൂർണ്ണസംഖ്യ compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  എങ്കിൽ (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_എണ്ണം());

  എങ്കിൽ (compare > 0)
    തിരികെ cmpGreaterThan;
  അല്ലെങ്കിൽ എങ്കിൽ (compare < 0)
    തിരികെ cmpLessThan;
  അല്ലെങ്കിൽ
    തിരികെ cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
സ്ഥിതം ശൂന്യം tcSetLeastSignificantBits(APInt::WordType *dst, unsigned parts,
                                      unsigned bits) {
  unsigned i = 0;
  ആകുമ്പോൾ (bits > APInt::APINT_BITS_PER_WORD) {
    dst[i++] = ~(APInt::WordType)0;
    bits -= APInt::APINT_BITS_PER_WORD;
  }

  എങ്കിൽ (bits)
    dst[i++] = ~(APInt::WordType)0 >> (APInt::APINT_BITS_PER_WORD - bits);

  ആകുമ്പോൾ (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::opStatus IEEEFloat::handleOverflow(roundingMode rounding_mode) {
  എങ്കിൽ (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    എങ്കിൽ (rounding_mode == rmNearestTiesToEven ||
        rounding_mode == rmNearestTiesToAway ||
        (rounding_mode == rmTowardPositive && !sign) ||
        (rounding_mode == rmTowardNegative && sign)) {
      എങ്കിൽ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(അസത്യം, sign);
      അല്ലെങ്കിൽ
        category = fcInfinity;
      തിരികെ static_cast<opStatus>(opOverflow | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fcNormal;
  exponent = semantics->maxExponent;
  tcSetLeastSignificantBits(significandParts(), part_എണ്ണം(),
                            semantics->precision);
  എങ്കിൽ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tcClearBit(significandParts(), 0);

  തിരികെ opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
തർക്കം IEEEFloat::roundAwayFromZero(roundingMode rounding_mode,
                                  lostFraction lost_fraction,
                                  unsigned പൂർണ്ണസംഖ്യ bit) സ്ഥിരം {
  /* NaNs and infinities should not have lost fractions.  */
  പ്രതിജ്ഞാ(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  പ്രതിജ്ഞാ(lost_fraction != lfExactlyZero);

  തിരഞ്ഞെടുപ്പ് (rounding_mode) {
  അവസ്ഥ rmNearestTiesToAway:
    തിരികെ lost_fraction == lfExactlyHalf || lost_fraction == lfMoreThanHalf;

  അവസ്ഥ rmNearestTiesToEven:
    എങ്കിൽ (lost_fraction == lfMoreThanHalf)
      തിരികെ സത്യം;

    /* Our zeroes don't have a significand to test.  */
    എങ്കിൽ (lost_fraction == lfExactlyHalf && category != fcZero)
      തിരികെ APInt::tcExtractBit(significandParts(), bit);

    തിരികെ അസത്യം;

  അവസ്ഥ rmTowardZero:
    തിരികെ അസത്യം;

  അവസ്ഥ rmTowardPositive:
    തിരികെ !sign;

  അവസ്ഥ rmTowardNegative:
    തിരികെ sign;

  സ്ഥിരസ്ഥിതി:
    നിർത്തുക;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::opStatus IEEEFloat::normalize(roundingMode rounding_mode,
                                       lostFraction lost_fraction) {
  എങ്കിൽ (!isFiniteNonZero())
    തിരികെ opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  unsigned omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  എങ്കിൽ (omsb || lost_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    പൂർണ്ണസംഖ്യ exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    എങ്കിൽ (exponent + exponentChange > semantics->maxExponent)
      തിരികെ handleOverflow(rounding_mode);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    എങ്കിൽ (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    എങ്കിൽ (exponentChange < 0) {
      പ്രതിജ്ഞാ(lost_fraction == lfExactlyZero);

      shift_Significand_ഇടത്(-exponentChange);

      തിരികെ opOK;
    }

    എങ്കിൽ (exponentChange > 0) {
      lostFraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = shift_Significand_വലത്(exponentChange);

      lost_fraction = combineLostFractions(lf, lost_fraction);

      /* Keep OMSB up-to-date.  */
      എങ്കിൽ (omsb > (unsigned) exponentChange)
        omsb -= exponentChange;
      അല്ലെങ്കിൽ
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  എങ്കിൽ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    തിരികെ handleOverflow(rounding_mode);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  എങ്കിൽ (lost_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    എങ്കിൽ (omsb == 0) {
      category = fcZero;
      എങ്കിൽ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = അസത്യം;
      എങ്കിൽ (!semantics->hasZero)
        makeSmallestNormalized(അസത്യം);
    }

    തിരികെ opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  എങ്കിൽ (roundAwayFromZero(rounding_mode, lost_fraction, 0)) {
    എങ്കിൽ (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    എങ്കിൽ (omsb == (unsigned) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      എങ്കിൽ (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        തിരികെ handleOverflow(sign ? rmTowardNegative : rmTowardPositive);

      shift_Significand_വലത്(1);

      തിരികെ opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    എങ്കിൽ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      തിരികെ handleOverflow(rounding_mode);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  എങ്കിൽ (omsb == semantics->precision)
    തിരികെ opInexact;

  /* We have a non-zero denormal.  */
  പ്രതിജ്ഞാ(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  എങ്കിൽ (omsb == 0) {
    category = fcZero;
    എങ്കിൽ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = അസത്യം;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    എങ്കിൽ (!semantics->hasZero)
      makeSmallestNormalized(അസത്യം);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  തിരികെ (opStatus) (opUnderflow | opInexact);
}

APFloat::opStatus IEEEFloat::addOrSubtractSpecials(സ്ഥിരം IEEEFloat &rhs,
                                                   തർക്കം subtract) {
  തിരഞ്ഞെടുപ്പ് (Pack_Categories_Into_താക്കോൽ(category, rhs.category)) {
  സ്ഥിരസ്ഥിതി:
    llvm_unreachable(nullptr);

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcNaN):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcNaN):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcZero):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcNormal):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcNaN):
    എങ്കിൽ (isSignaling()) {
      makeQuiet();
      തിരികെ opInvalidOp;
    }
    തിരികെ rhs.isSignaling() ? opInvalidOp : opOK;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcZero):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcNormal):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcZero):
    തിരികെ opOK;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ subtract;
    തിരികെ opOK;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcNormal):
    assign(rhs);
    sign = rhs.sign ^ subtract;
    തിരികെ opOK;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    തിരികെ opOK;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    എങ്കിൽ (((sign ^ rhs.sign)!=0) != subtract) {
      makeNaN();
      തിരികെ opInvalidOp;
    }

    തിരികെ opOK;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcNormal):
    തിരികെ opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
lostFraction IEEEFloat::addOrSubtractSignificand(സ്ഥിരം IEEEFloat &rhs,
                                                 തർക്കം subtract) {
  [[maybe_unused]] integerPart carry = 0;
  lostFraction lost_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  subtract ^= static_cast<തർക്കം>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  പൂർണ്ണസംഖ്യ bits = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  എങ്കിൽ (subtract) {
    എങ്കിൽ ((bits < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat temp_rhs(rhs);
    തർക്കം lost_fraction_is_from_rhs = അസത്യം;

    എങ്കിൽ (bits == 0)
      lost_fraction = lfExactlyZero;
    അല്ലെങ്കിൽ എങ്കിൽ (bits > 0) {
      lost_fraction = temp_rhs.shift_Significand_വലത്(bits - 1);
      lost_fraction_is_from_rhs = സത്യം;
      shift_Significand_ഇടത്(1);
    } അല്ലെങ്കിൽ {
      lost_fraction = shift_Significand_വലത്(-bits - 1);
      temp_rhs.shift_Significand_ഇടത്(1);
    }

    // Should we reverse the subtraction.
    cmpResult cmp_result = compare_Absolute_മൂല്യം(temp_rhs);
    എങ്കിൽ (cmp_result == cmpLessThan) {
      തർക്കം borrow =
          lost_fraction != lfExactlyZero && !lost_fraction_is_from_rhs;
      എങ്കിൽ (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        എങ്കിൽ (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        അല്ലെങ്കിൽ എങ്കിൽ (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = temp_rhs.subtractSignificand(*ഇത്, borrow);
      copySignificand(temp_rhs);
      sign = !sign;
    } അല്ലെങ്കിൽ എങ്കിൽ (cmp_result == cmpGreaterThan) {
      തർക്കം borrow = lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs;
      എങ്കിൽ (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        എങ്കിൽ (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        അല്ലെങ്കിൽ എങ്കിൽ (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = subtractSignificand(temp_rhs, borrow);
    } അല്ലെങ്കിൽ { // cmpEqual
      zeroSignificand();
      എങ്കിൽ (lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    പ്രതിജ്ഞാ(!carry);
  } അല്ലെങ്കിൽ {
    എങ്കിൽ (bits > 0) {
      IEEEFloat temp_rhs(rhs);

      lost_fraction = temp_rhs.shift_Significand_വലത്(bits);
      carry = addSignificand(temp_rhs);
    } അല്ലെങ്കിൽ {
      lost_fraction = shift_Significand_വലത്(-bits);
      carry = addSignificand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    പ്രതിജ്ഞാ(!carry);
  }

  തിരികെ lost_fraction;
}

APFloat::opStatus IEEEFloat::multiplySpecials(സ്ഥിരം IEEEFloat &rhs) {
  തിരഞ്ഞെടുപ്പ് (Pack_Categories_Into_താക്കോൽ(category, rhs.category)) {
  സ്ഥിരസ്ഥിതി:
    llvm_unreachable(nullptr);

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcNaN):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcNaN):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcNaN):
    assign(rhs);
    sign = അസത്യം;
    [[fallthrough]];
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcZero):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcNormal):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    എങ്കിൽ (isSignaling()) {
      makeQuiet();
      തിരികെ opInvalidOp;
    }
    തിരികെ rhs.isSignaling() ? opInvalidOp : opOK;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcNormal):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcInfinity):
    category = fcInfinity;
    തിരികെ opOK;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcNormal):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcZero):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcZero):
    category = fcZero;
    തിരികെ opOK;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcZero):
    makeNaN();
    തിരികെ opInvalidOp;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcNormal):
    തിരികെ opOK;
  }
}

APFloat::opStatus IEEEFloat::divideSpecials(സ്ഥിരം IEEEFloat &rhs) {
  തിരഞ്ഞെടുപ്പ് (Pack_Categories_Into_താക്കോൽ(category, rhs.category)) {
  സ്ഥിരസ്ഥിതി:
    llvm_unreachable(nullptr);

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcNaN):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcNaN):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcNaN):
    assign(rhs);
    sign = അസത്യം;
    [[fallthrough]];
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcZero):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcNormal):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    എങ്കിൽ (isSignaling()) {
      makeQuiet();
      തിരികെ opInvalidOp;
    }
    തിരികെ rhs.isSignaling() ? opInvalidOp : opOK;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcZero):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcNormal):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcNormal):
    തിരികെ opOK;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcInfinity):
    category = fcZero;
    തിരികെ opOK;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcZero):
    എങ്കിൽ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(അസത്യം, sign);
    അല്ലെങ്കിൽ
      category = fcInfinity;
    തിരികെ opDivByZero;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcZero):
    makeNaN();
    തിരികെ opInvalidOp;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcNormal):
    തിരികെ opOK;
  }
}

APFloat::opStatus IEEEFloat::modSpecials(സ്ഥിരം IEEEFloat &rhs) {
  തിരഞ്ഞെടുപ്പ് (Pack_Categories_Into_താക്കോൽ(category, rhs.category)) {
  സ്ഥിരസ്ഥിതി:
    llvm_unreachable(nullptr);

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcNaN):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcNaN):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcZero):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcNormal):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcNaN):
    എങ്കിൽ (isSignaling()) {
      makeQuiet();
      തിരികെ opInvalidOp;
    }
    തിരികെ rhs.isSignaling() ? opInvalidOp : opOK;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcNormal):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcInfinity):
    തിരികെ opOK;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcZero):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcZero):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcNormal):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcZero):
    makeNaN();
    തിരികെ opInvalidOp;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcNormal):
    തിരികെ opOK;
  }
}

APFloat::opStatus IEEEFloat::remainderSpecials(സ്ഥിരം IEEEFloat &rhs) {
  തിരഞ്ഞെടുപ്പ് (Pack_Categories_Into_താക്കോൽ(category, rhs.category)) {
  സ്ഥിരസ്ഥിതി:
    llvm_unreachable(nullptr);

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcNaN):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcNaN):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcZero):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcNormal):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcNaN):
    എങ്കിൽ (isSignaling()) {
      makeQuiet();
      തിരികെ opInvalidOp;
    }
    തിരികെ rhs.isSignaling() ? opInvalidOp : opOK;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcNormal):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcInfinity):
    തിരികെ opOK;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcZero):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcZero):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcNormal):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcZero):
    makeNaN();
    തിരികെ opInvalidOp;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcNormal):
    തിരികെ opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
ശൂന്യം IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  എങ്കിൽ (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    തിരികെ;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::opStatus IEEEFloat::addOrSubtract(സ്ഥിരം IEEEFloat &rhs,
                                           roundingMode rounding_mode,
                                           തർക്കം subtract) {
  opStatus fs = addOrSubtractSpecials(rhs, subtract);

  /* This return code means it was not a simple case.  */
  എങ്കിൽ (fs == opDivByZero) {
    lostFraction lost_fraction;

    lost_fraction = addOrSubtractSignificand(rhs, subtract);
    fs = normalize(rounding_mode, lost_fraction);

    /* Can only be zero if we lost no fraction.  */
    പ്രതിജ്ഞാ(category != fcZero || lost_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  എങ്കിൽ (category == fcZero) {
    എങ്കിൽ (rhs.category != fcZero || (sign == rhs.sign) == subtract)
      sign = (rounding_mode == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    എങ്കിൽ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = അസത്യം;
  }

  തിരികെ fs;
}

/* Normalized addition.  */
APFloat::opStatus IEEEFloat::add(സ്ഥിരം IEEEFloat &rhs,
                                 roundingMode rounding_mode) {
  തിരികെ addOrSubtract(rhs, rounding_mode, അസത്യം);
}

/* Normalized subtraction.  */
APFloat::opStatus IEEEFloat::subtract(സ്ഥിരം IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  തിരികെ addOrSubtract(rhs, rounding_mode, സത്യം);
}

/* Normalized multiply.  */
APFloat::opStatus IEEEFloat::multiply(സ്ഥിരം IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = multiplySpecials(rhs);

  എങ്കിൽ (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = അസത്യം;
  എങ്കിൽ (isFiniteNonZero()) {
    lostFraction lost_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    എങ്കിൽ (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  തിരികെ fs;
}

/* Normalized divide.  */
APFloat::opStatus IEEEFloat::divide(സ്ഥിരം IEEEFloat &rhs,
                                    roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = divideSpecials(rhs);

  എങ്കിൽ (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = അസത്യം;
  എങ്കിൽ (isFiniteNonZero()) {
    lostFraction lost_fraction = divideSignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    എങ്കിൽ (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  തിരികെ fs;
}

/* Normalized remainder.  */
APFloat::opStatus IEEEFloat::remainder(സ്ഥിരം IEEEFloat &rhs) {
  unsigned പൂർണ്ണസംഖ്യ origSign = sign;

  // First handle the special cases.
  opStatus fs = remainderSpecials(rhs);
  എങ്കിൽ (fs != opDivByZero)
    തിരികെ fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  എങ്കിൽ (P2.add(rhs, rmNearestTiesToEven) == opOK) {
    fs = mod(P2);
    പ്രതിജ്ഞാ(fs == opOK);
  }

  // Lets work with absolute numbers.
  IEEEFloat P = rhs;
  P.sign = അസത്യം;
  sign = അസത്യം;

  //
  // To calculate the remainder we use the following scheme.
  //
  // The remainder is defained as follows:
  //
  // remainder = numer - rquot * denom = x - r * p
  //
  // Where r is the result of: x/p, rounded toward the nearest integral value
  // (with halfway cases rounded toward the even number).
  //
  // Currently, (after x mod 2p):
  // r is the number of 2p's present inside x, which is inherently, an even
  // number of p's.
  //
  // We may split the remaining calculation into 4 options:
  // - if x < 0.5p then we round to the nearest number with is 0, and are done.
  // - if x == 0.5p then we round to the nearest even number which is 0, and we
  //   are done as well.
  // - if 0.5p < x < p then we round to nearest number which is 1, and we have
  //   to subtract 1p at least once.
  // - if x >= p then we must subtract p at least once, as x must be a
  //   remainder.
  //
  // By now, we were done, or we added 1 to r, which in turn, now an odd number.
  //
  // We can now split the remaining calculation to the following 3 options:
  // - if x < 0.5p then we round to the nearest number with is 0, and are done.
  // - if x == 0.5p then we round to the nearest even number. As r is odd, we
  //   must round up to the next even number. so we must subtract p once more.
  // - if x > 0.5p (and inherently x < p) then we must round r up to the next
  //   integral, and subtract p once more.
  //

  // Extend the semantics to prevent an overflow/underflow or inexact result.
  തർക്കം losesInfo;
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.maxExponent++;
  extendedSemantics.minExponent--;
  extendedSemantics.precision += 2;

  IEEEFloat VEx = *ഇത്;
  fs = VEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  പ്രതിജ്ഞാ(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  പ്രതിജ്ഞാ(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.add(VEx, rmNearestTiesToEven);
  പ്രതിജ്ഞാ(fs == opOK);

  എങ്കിൽ (VEx.compare(PEx) == cmpGreaterThan) {
    fs = subtract(P, rmNearestTiesToEven);
    പ്രതിജ്ഞാ(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    പ്രതിജ്ഞാ(fs == opOK);
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    പ്രതിജ്ഞാ(fs == opOK);

    cmpResult result = VEx.compare(PEx);
    എങ്കിൽ (result == cmpGreaterThan || result == cmpEqual) {
      fs = subtract(P, rmNearestTiesToEven);
      പ്രതിജ്ഞാ(fs == opOK);
    }
  }

  എങ്കിൽ (isZero()) {
    sign = origSign;    // IEEE754 requires this
    എങ്കിൽ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = അസത്യം;
  } അല്ലെങ്കിൽ {
    sign ^= origSign;
  }
  തിരികെ fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::opStatus IEEEFloat::mod(സ്ഥിരം IEEEFloat &rhs) {
  opStatus fs = modSpecials(rhs);
  unsigned പൂർണ്ണസംഖ്യ origSign = sign;

  ആകുമ്പോൾ (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_മൂല്യം(rhs) != cmpLessThan) {
    പൂർണ്ണസംഖ്യ Exp = ilogb(*ഇത്) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rmNearestTiesToEven);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    എങ്കിൽ (V.isNaN() || compare_Absolute_മൂല്യം(V) == cmpLessThan)
      V = scalbn(rhs, Exp - 1, rmNearestTiesToEven);
    V.sign = sign;

    fs = subtract(V, rmNearestTiesToEven);

    // When the semantics supports zero, this loop's
    // exit-condition is handled by the 'isFiniteNonZero'
    // category check above. However, when the semantics
    // does not have 'fcZero' and we have reached the
    // minimum possible value, (and any further subtract
    // will underflow to the same value) explicitly
    // provide an exit-path here.
    എങ്കിൽ (!semantics->hasZero && ഇത്->isSmallest())
      നിർത്തുക;

    പ്രതിജ്ഞാ(fs==opOK);
  }
  എങ്കിൽ (isZero()) {
    sign = origSign; // fmod requires this
    എങ്കിൽ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = അസത്യം;
  }
  തിരികെ fs;
}

/* Normalized fused-multiply-add.  */
APFloat::opStatus IEEEFloat::fusedMultiplyAdd(സ്ഥിരം IEEEFloat &multiplicand,
                                              സ്ഥിരം IEEEFloat &addend,
                                              roundingMode rounding_mode) {
  opStatus fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  എങ്കിൽ (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    lostFraction lost_fraction;

    lost_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_mode, lost_fraction);
    എങ്കിൽ (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    എങ്കിൽ (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_mode == rmTowardNegative);
      എങ്കിൽ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = അസത്യം;
    }
  } അല്ലെങ്കിൽ {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    എങ്കിൽ (fs == opOK)
      fs = addOrSubtract(addend, rounding_mode, അസത്യം);
  }

  തിരികെ fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::opStatus IEEEFloat::roundToIntegral(roundingMode rounding_mode) {
  എങ്കിൽ (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    തിരികെ opOK;

  എങ്കിൽ (isNaN()) {
    എങ്കിൽ (isSignaling()) {
      // [IEEE Std 754-2008 6.2]:
      // Under default exception handling, any operation signaling an invalid
      // operation exception and for which a floating-point result is to be
      // delivered shall deliver a quiet NaN.
      makeQuiet();
      // [IEEE Std 754-2008 6.2]:
      // Signaling NaNs shall be reserved operands that, under default exception
      // handling, signal the invalid operation exception(see 7.2) for every
      // general-computational and signaling-computational operation except for
      // the conversions described in 5.12.
      തിരികെ opInvalidOp;
    } അല്ലെങ്കിൽ {
      // [IEEE Std 754-2008 6.2]:
      // For an operation with quiet NaN inputs, other than maximum and minimum
      // operations, if a floating-point result is to be delivered the result
      // shall be a quiet NaN which should be one of the input NaNs.
      // ...
      // Every general-computational and quiet-computational operation involving
      // one or more input NaNs, none of them signaling, shall signal no
      // exception, except fusedMultiplyAdd might signal the invalid operation
      // exception(see 7.2).
      തിരികെ opOK;
    }
  }

  എങ്കിൽ (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    തിരികെ opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  എങ്കിൽ (exponent + 1 >= (പൂർണ്ണസംഖ്യ)APFloat::semanticsPrecision(*semantics))
    തിരികെ opOK;

  // The algorithm here is quite simple: we add 2^(p-1), where p is the
  // precision of our format, and then subtract it back off again.  The choice
  // of rounding modes for the addition/subtraction determines the rounding mode
  // for our integral rounding as well.
  // NOTE: When the input value is negative, we do subtraction followed by
  // addition instead.
  APInt IntegerConstant(NextPowerOf2(APFloat::semanticsPrecision(*semantics)),
                        1);
  IntegerConstant <<= APFloat::semanticsPrecision(*semantics) - 1;
  IEEEFloat MagicConstant(*semantics);
  opStatus fs = MagicConstant.convertFromAPInt(IntegerConstant, അസത്യം,
                                               rmNearestTiesToEven);
  പ്രതിജ്ഞാ(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  തർക്കം നിവേശം_Sign = isNegative();

  fs = add(MagicConstant, rounding_mode);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  subtract(MagicConstant, rounding_mode);

  // Restore the input sign.
  എങ്കിൽ (നിവേശം_Sign != isNegative())
    changeSign();

  തിരികെ fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmpResult IEEEFloat::compare(സ്ഥിരം IEEEFloat &rhs) സ്ഥിരം {
  പ്രതിജ്ഞാ(semantics == rhs.semantics);

  തിരഞ്ഞെടുപ്പ് (Pack_Categories_Into_താക്കോൽ(category, rhs.category)) {
  സ്ഥിരസ്ഥിതി:
    llvm_unreachable(nullptr);

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcZero):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcNormal):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNaN, fcNaN):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcNaN):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcNaN):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcNaN):
    തിരികെ cmpUnordered;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcNormal):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcZero):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcZero):
    എങ്കിൽ (sign)
      തിരികെ cmpLessThan;
    അല്ലെങ്കിൽ
      തിരികെ cmpGreaterThan;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcInfinity):
  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcNormal):
    എങ്കിൽ (rhs.sign)
      തിരികെ cmpGreaterThan;
    അല്ലെങ്കിൽ
      തിരികെ cmpLessThan;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcInfinity, fcInfinity):
    എങ്കിൽ (sign == rhs.sign)
      തിരികെ cmpEqual;
    അല്ലെങ്കിൽ എങ്കിൽ (sign)
      തിരികെ cmpLessThan;
    അല്ലെങ്കിൽ
      തിരികെ cmpGreaterThan;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcZero, fcZero):
    തിരികെ cmpEqual;

  അവസ്ഥ Pack_Categories_Into_താക്കോൽ(fcNormal, fcNormal):
    നിർത്തുക;
  }

  cmpResult result;
  /* Two normal numbers.  Do they have the same sign?  */
  എങ്കിൽ (sign != rhs.sign) {
    എങ്കിൽ (sign)
      result = cmpLessThan;
    അല്ലെങ്കിൽ
      result = cmpGreaterThan;
  } അല്ലെങ്കിൽ {
    /* Compare absolute values; invert result if negative.  */
    result = compare_Absolute_മൂല്യം(rhs);

    എങ്കിൽ (sign) {
      എങ്കിൽ (result == cmpLessThan)
        result = cmpGreaterThan;
      അല്ലെങ്കിൽ എങ്കിൽ (result == cmpGreaterThan)
        result = cmpLessThan;
    }
  }

  തിരികെ result;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::opStatus IEEEFloat::convert(സ്ഥിരം fltSemantics &toSemantics,
                                     roundingMode rounding_mode,
                                     തർക്കം *losesInfo) {
  opStatus fs;
  സ്ഥിരം fltSemantics &fromSemantics = *semantics;
  തർക്കം is_signaling = isSignaling();

  lostFraction lostFraction = lfExactlyZero;
  unsigned new_Part_എണ്ണം = part_എണ്ണം_For_Bits(toSemantics.precision + 1);
  unsigned old_Part_എണ്ണം = part_എണ്ണം();
  പൂർണ്ണസംഖ്യ shift = toSemantics.precision - fromSemantics.precision;

  തർക്കം X86SpecialNan = അസത്യം;
  എങ്കിൽ (&fromSemantics == &APFloatBase::semX87DoubleExtended &&
      &toSemantics != &APFloatBase::semX87DoubleExtended && category == fcNaN &&
      (!(*significandParts() & 0x8000000000000000ULL) ||
       !(*significandParts() & 0x4000000000000000ULL))) {
    // x86 has some unusual NaNs which cannot be represented in any other
    // format; note them here.
    X86SpecialNan = സത്യം;
  }

  // If this is a truncation of a denormal number, and the target semantics
  // has larger exponent range than the source semantics (this can happen
  // when truncating from PowerPC double-double to double format), the
  // right shift could lose result mantissa bits.  Adjust exponent instead
  // of performing excessive shift.
  // Also do a similar trick in case shifting denormal would produce zero
  // significand as this case isn't handled correctly by normalize.
  എങ്കിൽ (shift < 0 && isFiniteNonZero()) {
    പൂർണ്ണസംഖ്യ omsb = significandMSB() + 1;
    പൂർണ്ണസംഖ്യ exponentChange = omsb - fromSemantics.precision;
    എങ്കിൽ (exponent + exponentChange < toSemantics.minExponent)
      exponentChange = toSemantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, shift);
    എങ്കിൽ (exponentChange < 0) {
      shift -= exponentChange;
      exponent += exponentChange;
    } അല്ലെങ്കിൽ എങ്കിൽ (omsb <= -shift) {
      exponentChange = omsb + shift - 1; // leave at least one bit set
      shift -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  എങ്കിൽ (shift < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    lostFraction = shift_വലത്(significandParts(), old_Part_എണ്ണം, -shift);

  // Fix the storage so it can hold to new value.
  എങ്കിൽ (new_Part_എണ്ണം > old_Part_എണ്ണം) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = പുതിയ integerPart[new_Part_എണ്ണം];
    APInt::tcSet(newParts, 0, new_Part_എണ്ണം);
    എങ്കിൽ (isFiniteNonZero() || category==fcNaN)
      APInt::tcAssign(newParts, significandParts(), old_Part_എണ്ണം);
    freeSignificand();
    significand.parts = newParts;
  } അല്ലെങ്കിൽ എങ്കിൽ (new_Part_എണ്ണം == 1 && old_Part_എണ്ണം != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    എങ്കിൽ (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &toSemantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  എങ്കിൽ (shift > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_Shift_ഇടത്(significandParts(), new_Part_എണ്ണം, shift);

  എങ്കിൽ (isFiniteNonZero()) {
    fs = normalize(rounding_mode, lostFraction);
    *losesInfo = (fs != opOK);
  } അല്ലെങ്കിൽ എങ്കിൽ (category == fcNaN) {
    എങ്കിൽ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(അസത്യം, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } അല്ലെങ്കിൽ {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      എങ്കിൽ (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(അസത്യം, അസത്യം);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      എങ്കിൽ (!APFloat::hasSignificand(fromSemantics))
        makeNaN(അസത്യം, sign);

      *losesInfo = lostFraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      എങ്കിൽ (!X86SpecialNan && semantics == &APFloatBase::semX87DoubleExtended)
        APInt::tcSetBit(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      എങ്കിൽ (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } അല്ലെങ്കിൽ {
        fs = opOK;
      }
    }
  } അല്ലെങ്കിൽ എങ്കിൽ (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(അസത്യം, sign);
    *losesInfo = സത്യം;
    fs = opInexact;
  } അല്ലെങ്കിൽ എങ്കിൽ (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = അസത്യം;
  } അല്ലെങ്കിൽ {
    *losesInfo = അസത്യം;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  എങ്കിൽ ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = സത്യം;
    എങ്കിൽ (fs == opOK)
      fs = opInexact;
  }

  എങ്കിൽ (category == fcZero && !semantics->hasZero)
    makeSmallestNormalized(അസത്യം);
  തിരികെ fs;
}

/* Convert a floating point number to an integer according to the
   rounding mode.  If the rounded integer value is out of range this
   returns an invalid operation exception and the contents of the
   destination parts are unspecified.  If the rounded value is in
   range but the floating point number is not the exact integer, the C
   standard doesn't require an inexact exception to be raised.  IEEE
   854 does require it so we do that.

   Note that for conversions to integer type the C standard requires
   round-to-zero to always be used.  */
APFloat::opStatus IEEEFloat::convertToSignExtendedInteger(
    MutableArrayRef<integerPart> parts, unsigned പൂർണ്ണസംഖ്യ width, തർക്കം isSigned,
    roundingMode rounding_mode, തർക്കം *isExact) സ്ഥിരം {
  *isExact = അസത്യം;

  /* Handle the three special cases first.  */
  എങ്കിൽ (category == fcInfinity || category == fcNaN)
    തിരികെ opInvalidOp;

  unsigned dst_Parts_എണ്ണം = part_എണ്ണം_For_Bits(width);
  പ്രതിജ്ഞാ(dst_Parts_എണ്ണം <= parts.size() && "Integer too big");

  എങ്കിൽ (category == fcZero) {
    APInt::tcSet(parts.data(), 0, dst_Parts_എണ്ണം);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    തിരികെ opOK;
  }

  സ്ഥിരം integerPart *src = significandParts();

  unsigned truncatedBits;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  എങ്കിൽ (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tcSet(parts.data(), 0, dst_Parts_എണ്ണം);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncatedBits = semantics->precision -1U - exponent;
  } അല്ലെങ്കിൽ {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    unsigned പൂർണ്ണസംഖ്യ bits = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    എങ്കിൽ (bits > width)
      തിരികെ opInvalidOp;

    എങ്കിൽ (bits < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncatedBits = semantics->precision - bits;
      APInt::tcExtract(parts.data(), dst_Parts_എണ്ണം, src, bits, truncatedBits);
    } അല്ലെങ്കിൽ {
      /* We want at least as many bits as are available.  */
      APInt::tcExtract(parts.data(), dst_Parts_എണ്ണം, src, semantics->precision,
                       0);
      APInt::tc_Shift_ഇടത്(parts.data(), dst_Parts_എണ്ണം,
                         bits - semantics->precision);
      truncatedBits = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  lostFraction lost_fraction;
  എങ്കിൽ (truncatedBits) {
    lost_fraction = lostFractionThroughTruncation(src, part_എണ്ണം(),
                                                  truncatedBits);
    എങ്കിൽ (lost_fraction != lfExactlyZero &&
        roundAwayFromZero(rounding_mode, lost_fraction, truncatedBits)) {
      എങ്കിൽ (APInt::tcIncrement(parts.data(), dst_Parts_എണ്ണം))
        തിരികെ opInvalidOp;     /* Overflow.  */
    }
  } അല്ലെങ്കിൽ {
    lost_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  unsigned പൂർണ്ണസംഖ്യ omsb = APInt::tcMSB(parts.data(), dst_Parts_എണ്ണം) + 1;

  എങ്കിൽ (sign) {
    എങ്കിൽ (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      എങ്കിൽ (omsb != 0)
        തിരികെ opInvalidOp;
    } അല്ലെങ്കിൽ {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      എങ്കിൽ (omsb == width &&
          APInt::tcLSB(parts.data(), dst_Parts_എണ്ണം) + 1 != omsb)
        തിരികെ opInvalidOp;

      /* This case can happen because of rounding.  */
      എങ്കിൽ (omsb > width)
        തിരികെ opInvalidOp;
    }

    APInt::tcNegate (parts.data(), dst_Parts_എണ്ണം);
  } അല്ലെങ്കിൽ {
    എങ്കിൽ (omsb >= width + !isSigned)
      തിരികെ opInvalidOp;
  }

  എങ്കിൽ (lost_fraction == lfExactlyZero) {
    *isExact = സത്യം;
    തിരികെ opOK;
  }
  തിരികെ opInexact;
}

/* Same as convertToSignExtendedInteger, except we provide
   deterministic values in case of an invalid operation exception,
   namely zero for NaNs and the minimal or maximal value respectively
   for underflow or overflow.
   The *isExact output tells whether the result is exact, in the sense
   that converting it back to the original floating point type produces
   the original value.  This is almost equivalent to result==opOK,
   except for negative zeroes.
*/
APFloat::opStatus
IEEEFloat::convertToInteger(MutableArrayRef<integerPart> parts,
                            unsigned പൂർണ്ണസംഖ്യ width, തർക്കം isSigned,
                            roundingMode rounding_mode, തർക്കം *isExact) സ്ഥിരം {
  opStatus fs = convertToSignExtendedInteger(parts, width, isSigned,
                                             rounding_mode, isExact);

  എങ്കിൽ (fs == opInvalidOp) {
    unsigned പൂർണ്ണസംഖ്യ bits, dst_Parts_എണ്ണം;

    dst_Parts_എണ്ണം = part_എണ്ണം_For_Bits(width);
    പ്രതിജ്ഞാ(dst_Parts_എണ്ണം <= parts.size() && "Integer too big");

    എങ്കിൽ (category == fcNaN)
      bits = 0;
    അല്ലെങ്കിൽ എങ്കിൽ (sign)
      bits = isSigned;
    അല്ലെങ്കിൽ
      bits = width - isSigned;

    tcSetLeastSignificantBits(parts.data(), dst_Parts_എണ്ണം, bits);
    എങ്കിൽ (sign && isSigned)
      APInt::tc_Shift_ഇടത്(parts.data(), dst_Parts_എണ്ണം, width - 1);
  }

  തിരികെ fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::opStatus IEEEFloat::convertFromUnsignedParts(
    സ്ഥിരം integerPart *src, unsigned പൂർണ്ണസംഖ്യ src_എണ്ണം, roundingMode rounding_mode) {
  category = fcNormal;
  unsigned omsb = APInt::tcMSB(src, src_എണ്ണം) + 1;
  integerPart *dst = significandParts();
  unsigned dst_എണ്ണം = part_എണ്ണം();
  unsigned precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  lostFraction lost_fraction;
  എങ്കിൽ (precision <= omsb) {
    exponent = omsb - 1;
    lost_fraction = lostFractionThroughTruncation(src, src_എണ്ണം,
                                                  omsb - precision);
    APInt::tcExtract(dst, dst_എണ്ണം, src, precision, omsb - precision);
  } അല്ലെങ്കിൽ {
    exponent = precision - 1;
    lost_fraction = lfExactlyZero;
    APInt::tcExtract(dst, dst_എണ്ണം, src, omsb, 0);
  }

  തിരികെ normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus IEEEFloat::convertFromAPInt(സ്ഥിരം APInt &Val, തർക്കം isSigned,
                                              roundingMode rounding_mode) {
  unsigned പൂർണ്ണസംഖ്യ part_എണ്ണം = Val.get_അക്കം_വാക്കുകൾ();
  APInt api = Val;

  sign = അസത്യം;
  എങ്കിൽ (isSigned && api.isNegative()) {
    sign = സത്യം;
    api = -api;
  }

  തിരികെ convertFromUnsignedParts(api.getRawData(), part_എണ്ണം, rounding_mode);
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        roundingMode rounding_mode) {
  lostFraction lost_fraction = lfExactlyZero;

  category = fcNormal;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  unsigned parts_എണ്ണം = part_എണ്ണം();
  unsigned bitPos = parts_എണ്ണം * integerPartWidth;
  തർക്കം computedTrailingFraction = അസത്യം;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator end = s.end();
  StringRef::iterator dot;
  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  എങ്കിൽ (!PtrOrErr)
    തിരികെ PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator firstSignificantDigit = p;

  ആകുമ്പോൾ (p != end) {
    integerPart hex_മൂല്യം;

    എങ്കിൽ (*p == '.') {
      എങ്കിൽ (dot != end)
        തിരികെ createError("String contains multiple dots");
      dot = p++;
      തുടരുക;
    }

    hex_മൂല്യം = hex_Digit_മൂല്യം(*p);
    എങ്കിൽ (hex_മൂല്യം == UINT_MAX)
      നിർത്തുക;

    p++;

    // Store the number while we have space.
    എങ്കിൽ (bitPos) {
      bitPos -= 4;
      hex_മൂല്യം <<= bitPos % integerPartWidth;
      significand[bitPos / integerPartWidth] |= hex_മൂല്യം;
    } അല്ലെങ്കിൽ എങ്കിൽ (!computedTrailingFraction) {
      auto FractOrErr = trailingHexadecimalFraction(p, end, hex_മൂല്യം);
      എങ്കിൽ (!FractOrErr)
        തിരികെ FractOrErr.takeError();
      lost_fraction = *FractOrErr;
      computedTrailingFraction = സത്യം;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  എങ്കിൽ (p == end)
    തിരികെ createError("Hex strings require an exponent");
  എങ്കിൽ (*p != 'p' && *p != 'P')
    തിരികെ createError("Invalid character in significand");
  എങ്കിൽ (p == begin)
    തിരികെ createError("Significand has no digits");
  എങ്കിൽ (dot != end && p - begin == 1)
    തിരികെ createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  എങ്കിൽ (p != firstSignificantDigit) {
    പൂർണ്ണസംഖ്യ expAdjustment;

    /* Implicit hexadecimal point?  */
    എങ്കിൽ (dot == end)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<പൂർണ്ണസംഖ്യ>(dot - firstSignificantDigit);
    എങ്കിൽ (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_എണ്ണം * integerPartWidth;

    /* Adjust for the given exponent.  */
    auto ExpOrErr = totalExponent(p + 1, end, expAdjustment);
    എങ്കിൽ (!ExpOrErr)
      തിരികെ ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  തിരികെ normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus
IEEEFloat::roundSignificandWithExponent(സ്ഥിരം integerPart *decSigParts,
                                        unsigned sig_Part_എണ്ണം, പൂർണ്ണസംഖ്യ exp,
                                        roundingMode rounding_mode) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  unsigned പൂർണ്ണസംഖ്യ power = exp >= 0 ? exp : -exp;
  unsigned പൂർണ്ണസംഖ്യ pow5PartCapacity = powerOfFivePartsForExponent(power);
  SmallVector<integerPart, അക്കം_Pow5_Parts_On_അട്ടി> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5PartCapacity);

  തർക്കം isNearest = rounding_mode == rmNearestTiesToEven ||
                   rounding_mode == rmNearestTiesToAway;

  unsigned parts = part_എണ്ണം_For_Bits(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  unsigned pow5_Part_എണ്ണം = powerOf5(pow5Parts.data(), power);

  വേണ്ടി (;; parts *= 2) {
    unsigned പൂർണ്ണസംഖ്യ excessPrecision, truncatedBits;

    calcSemantics.precision = parts * integerPartWidth - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncatedBits = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    opStatus sigStatus = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_എണ്ണം, rmNearestTiesToEven);
    opStatus powStatus = pow5.convertFromUnsignedParts(
        pow5Parts.data(), pow5_Part_എണ്ണം, rmNearestTiesToEven);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += exp;

    lostFraction calcLostFraction;
    integerPart HUerr, HUdistance;
    unsigned പൂർണ്ണസംഖ്യ powHUerr;

    എങ്കിൽ (exp >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calcLostFraction = decSig.multiplySignificand(pow5);
      powHUerr = powStatus != opOK;
    } അല്ലെങ്കിൽ {
      calcLostFraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      എങ്കിൽ (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncatedBits = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (powStatus == opOK && calcLostFraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    പ്രതിജ്ഞാ(APInt::tcExtractBit
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calcLostFraction != lfExactlyZero, sigStatus != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, isNearest);

    /* Are we guaranteed to round correctly if we truncate?  */
    എങ്കിൽ (HUdistance >= HUerr) {
      APInt::tcExtract(significandParts(), part_എണ്ണം(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calcLostFraction = lostFractionThroughTruncation(decSig.significandParts(),
                                                       decSig.part_എണ്ണം(),
                                                       truncatedBits);
      തിരികെ static_cast<opStatus>(normalize(rounding_mode, calcLostFraction) |
                                   ((sigStatus | powStatus) & opInexact));
    }
  }
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromDecimalString(StringRef str, roundingMode rounding_mode) {
  decimalInfo D;
  opStatus fs;

  /* Scan the text.  */
  StringRef::iterator p = str.begin();
  എങ്കിൽ (Error Err = interpretDecimal(p, str.end(), &D))
    തിരികെ std::move(Err);

  /* Handle the quick cases.  First the case of no significant digits,
     i.e. zero, and then exponents that are obviously too large or too
     small.  Writing L for log 10 / log 2, a number d.ddddd*10^exp
     definitely overflows if

           (exp - 1) * L >= maxExponent

     and definitely underflows to zero where

           (exp + 1) * L <= minExponent - precision

     With integer arithmetic the tightest bounds for L are

           93/28 < L < 196/59            [ numerator <= 256 ]
           42039/12655 < L < 28738/8651  [ numerator <= 65536 ]
  */

  // Test if we have a zero number allowing for strings with no null terminators
  // and zero decimals with non-zero exponents.
  //
  // We computed firstSigDigit by ignoring all zeros and dots. Thus if
  // D->firstSigDigit equals str.end(), every digit must be a zero and there can
  // be at most one dot. On the other hand, if we have a zero with a non-zero
  // exponent, then we know that D.firstSigDigit will be non-numeric.
  എങ്കിൽ (D.firstSigDigit == str.end() || dec_Digit_മൂല്യം(*D.firstSigDigit) >= 10U) {
    category = fcZero;
    fs = opOK;
    എങ്കിൽ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = അസത്യം;
    എങ്കിൽ (!semantics->hasZero)
      makeSmallestNormalized(അസത്യം);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } അല്ലെങ്കിൽ എങ്കിൽ (D.normalizedExponent - 1 > INT_MAX / 42039) {
    fs = handleOverflow(rounding_mode);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } അല്ലെങ്കിൽ എങ്കിൽ (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (പൂർണ്ണസംഖ്യ) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fcNormal;
    zeroSignificand();
    fs = normalize(rounding_mode, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } അല്ലെങ്കിൽ എങ്കിൽ ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handleOverflow(rounding_mode);
  } അല്ലെങ്കിൽ {
    integerPart *decSignificand;
    unsigned പൂർണ്ണസംഖ്യ part_എണ്ണം;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_എണ്ണം = static_cast<unsigned പൂർണ്ണസംഖ്യ>(D.lastSigDigit - D.firstSigDigit) + 1;
    part_എണ്ണം = part_എണ്ണം_For_Bits(1 + 196 * part_എണ്ണം / 59);
    decSignificand = പുതിയ integerPart[part_എണ്ണം + 1];
    part_എണ്ണം = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    ചെയ്യുക {
      integerPart dec_മൂല്യം, val, multiplier;

      val = 0;
      multiplier = 1;

      ചെയ്യുക {
        എങ്കിൽ (*p == '.') {
          p++;
          എങ്കിൽ (p == str.end()) {
            നിർത്തുക;
          }
        }
        dec_മൂല്യം = dec_Digit_മൂല്യം(*p++);
        എങ്കിൽ (dec_മൂല്യം >= 10U) {
          delete[] decSignificand;
          തിരികെ createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_മൂല്യം;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } ആകുമ്പോൾ (p <= D.lastSigDigit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_എണ്ണം, part_എണ്ണം + 1, അസത്യം);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      എങ്കിൽ (decSignificand[part_എണ്ണം])
        part_എണ്ണം++;
    } ആകുമ്പോൾ (p <= D.lastSigDigit);

    category = fcNormal;
    fs = roundSignificandWithExponent(decSignificand, part_എണ്ണം,
                                      D.exponent, rounding_mode);

    delete [] decSignificand;
  }

  തിരികെ fs;
}

തർക്കം IEEEFloat::convertFromStringSpecials(StringRef str) {
  സ്ഥിരം size_t MIN_NAME_SIZE = 3;

  എങ്കിൽ (str.size() < MIN_NAME_SIZE)
    തിരികെ അസത്യം;

  എങ്കിൽ (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(അസത്യം);
    തിരികെ സത്യം;
  }

  തർക്കം IsNegative = str.consume_front("-");
  എങ്കിൽ (IsNegative) {
    എങ്കിൽ (str.size() < MIN_NAME_SIZE)
      തിരികെ അസത്യം;

    എങ്കിൽ (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(സത്യം);
      തിരികെ സത്യം;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  തർക്കം IsSignaling = str.consume_front_insensitive("s");
  എങ്കിൽ (IsSignaling) {
    എങ്കിൽ (str.size() < MIN_NAME_SIZE)
      തിരികെ അസത്യം;
  }

  എങ്കിൽ (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    എങ്കിൽ (str.empty()) {
      makeNaN(IsSignaling, IsNegative);
      തിരികെ സത്യം;
    }

    // Allow the payload to be inside parentheses.
    എങ്കിൽ (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      എങ്കിൽ (str.size() <= 2 || str.back() != ')')
        തിരികെ അസത്യം;

      str = str.slice(1, str.size() - 1);
    }

    // Determine the payload number's radix.
    unsigned Radix = 10;
    എങ്കിൽ (str[0] == '0') {
      എങ്കിൽ (str.size() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } അല്ലെങ്കിൽ {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    എങ്കിൽ (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      തിരികെ സത്യം;
    }
  }

  തിരികെ അസത്യം;
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromString(StringRef str, roundingMode rounding_mode) {
  എങ്കിൽ (str.empty())
    തിരികെ createError("Invalid string length");

  // Handle special cases.
  എങ്കിൽ (convertFromStringSpecials(str))
    തിരികെ opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  sign = *p == '-' ? 1 : 0;
  എങ്കിൽ (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  എങ്കിൽ (*p == '-' || *p == '+') {
    p++;
    slen--;
    എങ്കിൽ (!slen)
      തിരികെ createError("String has no digits");
  }

  എങ്കിൽ (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    എങ്കിൽ (slen == 2)
      തിരികെ createError("Invalid string");
    തിരികെ convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_mode);
  }

  തിരികെ convertFromDecimalString(StringRef(p, slen), rounding_mode);
}

/* Write out a hexadecimal representation of the floating point value
   to DST, which must be of sufficient size, in the C99 form
   [-]0xh.hhhhp[+-]d.  Return the number of characters written,
   excluding the terminating NUL.

   If UPPERCASE, the output is in upper case, otherwise in lower case.

   HEXDIGITS digits appear altogether, rounding the value if
   necessary.  If HEXDIGITS is 0, the minimal precision to display the
   number precisely is used instead.  If nothing would appear after
   the decimal point it is suppressed.

   The decimal exponent is always printed and has at least one digit.
   Zero values display an exponent of zero.  Infinities and NaNs
   appear as "infinity" or "nan" respectively.

   The above rules are as specified by C99.  There is ambiguity about
   what the leading hexadecimal digit should be.  This implementation
   uses whatever is necessary so that the exponent is displayed as
   stored.  This implies the exponent will fall within the IEEE format
   range, and the leading hexadecimal digit will be 0 (for denormals),
   1 (normal numbers) or 2 (normal numbers rounded-away-from-zero with
   any other digits zero).
*/
unsigned പൂർണ്ണസംഖ്യ IEEEFloat::convertToHexString(അക്ഷരം *dst, unsigned പൂർണ്ണസംഖ്യ hexDigits,
                                           തർക്കം upperCase,
                                           roundingMode rounding_mode) സ്ഥിരം {
  അക്ഷരം *p = dst;
  എങ്കിൽ (sign)
    *dst++ = '-';

  തിരഞ്ഞെടുപ്പ് (category) {
  അവസ്ഥ fcInfinity:
    memcpy (dst, upperCase ? infinityU: infinityL, വലുപ്പം infinityU - 1);
    dst += വലുപ്പം infinityL - 1;
    നിർത്തുക;

  അവസ്ഥ fcNaN:
    memcpy (dst, upperCase ? NaNU: NaNL, വലുപ്പം NaNU - 1);
    dst += വലുപ്പം NaNU - 1;
    നിർത്തുക;

  അവസ്ഥ fcZero:
    *dst++ = '0';
    *dst++ = upperCase ? 'X': 'x';
    *dst++ = '0';
    എങ്കിൽ (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = upperCase ? 'P': 'p';
    *dst++ = '0';
    നിർത്തുക;

  അവസ്ഥ fcNormal:
    dst = convertNormalToHexString (dst, hexDigits, upperCase, rounding_mode);
    നിർത്തുക;
  }

  *dst = 0;

  തിരികെ static_cast<unsigned പൂർണ്ണസംഖ്യ>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
അക്ഷരം *IEEEFloat::convertNormalToHexString(അക്ഷരം *dst, unsigned പൂർണ്ണസംഖ്യ hexDigits,
                                          തർക്കം upperCase,
                                          roundingMode rounding_mode) സ്ഥിരം {
  *dst++ = '0';
  *dst++ = upperCase ? 'X': 'x';

  തർക്കം roundUp = അസത്യം;
  സ്ഥിരം അക്ഷരം *hexDigitChars = upperCase ? hexDigitsUpper : hexDigitsLower;

  സ്ഥിരം integerPart *significand = significandParts();
  unsigned parts_എണ്ണം = part_എണ്ണം();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  unsigned മൂല്യം_Bits = semantics->precision + 3;
  unsigned shift = integerPartWidth - മൂല്യം_Bits % integerPartWidth;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  unsigned outputDigits = (മൂല്യം_Bits - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  എങ്കിൽ (hexDigits) {
    എങ്കിൽ (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      unsigned പൂർണ്ണസംഖ്യ bits;
      lostFraction fraction;

      bits = മൂല്യം_Bits - hexDigits * 4;
      fraction = lostFractionThroughTruncation (significand, parts_എണ്ണം, bits);
      roundUp = roundAwayFromZero(rounding_mode, fraction, bits);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  അക്ഷരം *p = ++dst;

  unsigned എണ്ണം = (മൂല്യം_Bits + integerPartWidth - 1) / integerPartWidth;

  ആകുമ്പോൾ (outputDigits && എണ്ണം) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    എങ്കിൽ (--എണ്ണം == parts_എണ്ണം)
      part = 0;  /* An imaginary higher zero part.  */
    അല്ലെങ്കിൽ
      part = significand[എണ്ണം] << shift;

    എങ്കിൽ (എണ്ണം && shift)
      part |= significand[എണ്ണം - 1] >> (integerPartWidth - shift);

    /* Convert as much of "part" to hexdigits as we can.  */
    unsigned പൂർണ്ണസംഖ്യ curDigits = integerPartWidth / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  എങ്കിൽ (roundUp) {
    അക്ഷരം *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    ചെയ്യുക {
      q--;
      *q = hexDigitChars[hex_Digit_മൂല്യം (*q) + 1];
    } ആകുമ്പോൾ (*q == '0');
    പ്രതിജ്ഞാ(q >= p);
  } അല്ലെങ്കിൽ {
    /* Add trailing zeroes.  */
    memset (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  എങ്കിൽ (dst -1 == p)
    dst--;
  അല്ലെങ്കിൽ
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = upperCase ? 'P': 'p';

  തിരികെ writeSignedDecimal (dst, exponent);
}

hash_code hash_മൂല്യം(സ്ഥിരം IEEEFloat &Arg) {
  എങ്കിൽ (!Arg.isFiniteNonZero())
    തിരികെ hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  തിരികെ hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.part_എണ്ണം()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convertF80LongDoubleAPFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം() == 2);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semX87DoubleExtended>();
}

APInt IEEEFloat::convertPPCDoubleDoubleLegacyAPFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(semantics ==
         (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy);
  പ്രതിജ്ഞാ(part_എണ്ണം()==2);

  uint64_t വാക്കുകൾ[2];
  തർക്കം losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloatBase::semIEEEdouble.minExponent;
  IEEEFloat extended(*ഇത്);
  [[maybe_unused]] opStatus fs =
      extended.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  പ്രതിജ്ഞാ(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
  പ്രതിജ്ഞാ(fs == opOK || fs == opInexact);
  വാക്കുകൾ[0] = *u.convertDoubleAPFloatToAPInt().getRawData();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  എങ്കിൽ (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
    പ്രതിജ്ഞാ(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.subtract(u, rmNearestTiesToEven);
    fs = v.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
    പ്രതിജ്ഞാ(fs == opOK && !losesInfo);
    വാക്കുകൾ[1] = *v.convertDoubleAPFloatToAPInt().getRawData();
  } അല്ലെങ്കിൽ {
    വാക്കുകൾ[1] = 0;
  }

  തിരികെ APInt(128, വാക്കുകൾ);
}

template <സ്ഥിരം fltSemantics &S>
APInt IEEEFloat::convertIEEEFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(semantics == &S);
  constexpr unsigned പൂർണ്ണസംഖ്യ trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr പൂർണ്ണസംഖ്യ integer_bit_part = (S.precision - 1) / integerPartWidth;
  constexpr integerPart integer_bit = integerPart{1}
                                      << ((S.precision - 1) % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned പൂർണ്ണസംഖ്യ exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr തർക്കം is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr പൂർണ്ണസംഖ്യ bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_എണ്ണം_For_Bits(trailing_significand_bits)>
      mysignificand;

  എങ്കിൽ (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
    എങ്കിൽ (myexponent == 1 &&
        !(significandParts()[integer_bit_part] & integer_bit))
      myexponent = 0; // denormal
  } അല്ലെങ്കിൽ എങ്കിൽ (category == fcZero) {
    എങ്കിൽ (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } അല്ലെങ്കിൽ എങ്കിൽ (category == fcInfinity) {
    എങ്കിൽ (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    എങ്കിൽ constexpr (S.hasExplicitIntegerBit) {
      mysignificand[0] = integerPart{1} << (trailing_significand_bits - 1);
    }
  } അല്ലെങ്കിൽ {
    പ്രതിജ്ഞാ(category == fcNaN && "Unknown category!");
    എങ്കിൽ (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.sizeInBits + 63) / 64> വാക്കുകൾ;
  auto വാക്കുകൾ_iter =
      std::copy_n(mysignificand.begin(), mysignificand.size(), വാക്കുകൾ.begin());
  എങ്കിൽ constexpr (!S.hasExplicitIntegerBit) {
    എങ്കിൽ constexpr (significand_mask != 0 || trailing_significand_bits == 0) {
      // Clear the integer bit.
      വാക്കുകൾ[mysignificand.size() - 1] &= significand_mask;
    }
  }
  std::fill(വാക്കുകൾ_iter, വാക്കുകൾ.end(), uint64_t{0});
  constexpr size_t last_word = വാക്കുകൾ.size() - 1;
  എങ്കിൽ constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.sizeInBits - 1) % 64);
    വാക്കുകൾ[last_word] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_mask)
                              << (trailing_significand_bits % 64);
  വാക്കുകൾ[last_word] |= shifted_exponent;
  എങ്കിൽ constexpr (last_word == 0) {
    തിരികെ APInt(S.sizeInBits, വാക്കുകൾ[0]);
  }
  തിരികെ APInt(S.sizeInBits, വാക്കുകൾ);
}

APInt IEEEFloat::convertQuadrupleAPFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം() == 2);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semIEEEquad>();
}

APInt IEEEFloat::convertDoubleAPFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം()==1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semIEEEdouble>();
}

APInt IEEEFloat::convertFloatAPFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം()==1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semIEEEsingle>();
}

APInt IEEEFloat::convertBFloatAPFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം() == 1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semBFloat>();
}

APInt IEEEFloat::convertHalfAPFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം()==1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::APFloatBase::semIEEEhalf>();
}

APInt IEEEFloat::convertFloat8E5M2APFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം() == 1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2>();
}

APInt IEEEFloat::convertFloat8E5M2FNUZAPFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം() == 1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3APFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം() == 1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3>();
}

APInt IEEEFloat::convertFloat8E4M3FNAPFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം() == 1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FN>();
}

APInt IEEEFloat::convertFloat8E4M3FNUZAPFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം() == 1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3B11FNUZAPFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം() == 1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convertFloat8E3M4APFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം() == 1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semFloat8E3M4>();
}

APInt IEEEFloat::convertFloatTF32APFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം() == 1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semFloatTF32>();
}

APInt IEEEFloat::convertFloat8E8M0FNUAPFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം() == 1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convertFloat8E5M3FNUAPFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം() == 1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convertFloat6E3M2FNAPFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം() == 1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semFloat6E3M2FN>();
}

APInt IEEEFloat::convertFloat6E2M3FNAPFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം() == 1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semFloat6E2M3FN>();
}

APInt IEEEFloat::convertFloat4E2M1FNAPFloatToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(part_എണ്ണം() == 1);
  തിരികെ convertIEEEFloatToAPInt<APFloatBase::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcastToAPInt() സ്ഥിരം {
  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semIEEEhalf)
    തിരികെ convertHalfAPFloatToAPInt();

  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semBFloat)
    തിരികെ convertBFloatAPFloatToAPInt();

  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semIEEEsingle)
    തിരികെ convertFloatAPFloatToAPInt();

  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semIEEEdouble)
    തിരികെ convertDoubleAPFloatToAPInt();

  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semIEEEquad)
    തിരികെ convertQuadrupleAPFloatToAPInt();

  എങ്കിൽ (semantics ==
      (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semX87DoubleExtended)
    തിരികെ convertF80LongDoubleAPFloatToAPInt();

  എങ്കിൽ (semantics ==
      (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy)
    തിരികെ convertPPCDoubleDoubleLegacyAPFloatToAPInt();

  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semFloat8E5M2)
    തിരികെ convertFloat8E5M2APFloatToAPInt();

  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semFloat8E5M2FNUZ)
    തിരികെ convertFloat8E5M2FNUZAPFloatToAPInt();

  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semFloat8E4M3)
    തിരികെ convertFloat8E4M3APFloatToAPInt();

  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FN)
    തിരികെ convertFloat8E4M3FNAPFloatToAPInt();

  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FNUZ)
    തിരികെ convertFloat8E4M3FNUZAPFloatToAPInt();

  എങ്കിൽ (semantics ==
      (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semFloat8E4M3B11FNUZ)
    തിരികെ convertFloat8E4M3B11FNUZAPFloatToAPInt();

  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semFloat8E3M4)
    തിരികെ convertFloat8E3M4APFloatToAPInt();

  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semFloatTF32)
    തിരികെ convertFloatTF32APFloatToAPInt();

  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semFloat8E8M0FNU)
    തിരികെ convertFloat8E8M0FNUAPFloatToAPInt();

  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semFloat8E5M3FNU)
    തിരികെ convertFloat8E5M3FNUAPFloatToAPInt();

  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semFloat6E3M2FN)
    തിരികെ convertFloat6E3M2FNAPFloatToAPInt();

  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semFloat6E2M3FN)
    തിരികെ convertFloat6E2M3FNAPFloatToAPInt();

  എങ്കിൽ (semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semFloat4E2M1FN)
    തിരികെ convertFloat4E2M1FNAPFloatToAPInt();

  llvm_unreachable("unknown format!");
}

ദശാംശം IEEEFloat::convertToFloat() സ്ഥിരം {
  പ്രതിജ്ഞാ(semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcastToAPInt();
  തിരികെ api.bitsToFloat();
}

ഇരട്ട IEEEFloat::convertToDouble() സ്ഥിരം {
  പ്രതിജ്ഞാ(semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcastToAPInt();
  തിരികെ api.bitsToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 IEEEFloat::convertToQuad() സ്ഥിരം {
  പ്രതിജ്ഞാ(semantics == (സ്ഥിരം llvm::fltSemantics *)&APFloatBase::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcastToAPInt();
  തിരികെ api.bitsToQuad();
}
#endif

ശൂന്യം IEEEFloat::initFromF80LongDoubleAPInt(സ്ഥിരം APInt &api) {
  തിരികെ initFromIEEEAPInt<APFloatBase::semX87DoubleExtended>(api);
}

ശൂന്യം IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(സ്ഥിരം APInt &api) {
  uint64_t i1 = api.getRawData()[0];
  uint64_t i2 = api.getRawData()[1];
  തർക്കം losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_unused]] opStatus fs = convert(APFloatBase::semPPCDoubleDoubleLegacy,
                                         rmNearestTiesToEven, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  പ്രതിജ്ഞാ((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  എങ്കിൽ (isFiniteNonZero()) {
    IEEEFloat v(APFloatBase::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    എങ്കിൽ (v.isFinite()) {
      fs = v.convert(APFloatBase::semPPCDoubleDoubleLegacy, rmNearestTiesToEven,
                     &losesInfo);
      പ്രതിജ്ഞാ(fs == opOK && !losesInfo);

      add(v, rmNearestTiesToEven);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
ശൂന്യം IEEEFloat::initFromFloat8E8M0FNUAPInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E8M0FNU>(api);
}

ശൂന്യം IEEEFloat::initFromFloat8E5M3FNUAPInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M3FNU>(api);
}

template <സ്ഥിരം fltSemantics &S>
ശൂന്യം IEEEFloat::initFromIEEEAPInt(സ്ഥിരം APInt &api) {
  പ്രതിജ്ഞാ(api.getBitWidth() == S.sizeInBits);

  constexpr unsigned പൂർണ്ണസംഖ്യ trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr integerPart integer_bit =
      integerPart{1} << (trailing_significand_bits % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned പൂർണ്ണസംഖ്യ exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr unsigned പൂർണ്ണസംഖ്യ stored_significand_parts =
      part_എണ്ണം_For_Bits(trailing_significand_bits + 1);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr തർക്കം is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr പൂർണ്ണസംഖ്യ bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr തർക്കം has_significand = trailing_significand_bits > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  എങ്കിൽ constexpr (has_significand) {
    std::copy_n(api.getRawData(), mysignificand.size(), mysignificand.begin());
    എങ്കിൽ constexpr (significand_mask != 0 || S.precision >= integerPartWidth) {
      mysignificand[mysignificand.size() - 1] &= significand_mask;
    }
  } അല്ലെങ്കിൽ {
    std::fill_n(mysignificand.begin(), mysignificand.size(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t last_word = api.getRawData()[api.get_അക്കം_വാക്കുകൾ() - 1];
  uint64_t myexponent =
      (last_word >> (trailing_significand_bits % 64)) & exponent_mask;

  initialize(&S);
  പ്രതിജ്ഞാ(part_എണ്ണം() == mysignificand.size());

  sign = S.hasSignedRepr
             ? static_cast<unsigned പൂർണ്ണസംഖ്യ>(last_word >> ((S.sizeInBits - 1) % 64))
             : 0;

  തർക്കം all_zero_significand =
      has_significand && llvm::all_of(mysignificand, equal_to(0));

  തർക്കം is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  എങ്കിൽ constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    തർക്കം is_inf = അസത്യം;

    എങ്കിൽ constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;
      സ്ഥിരം integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_mask_no_int_bit) == 0;
    } അല്ലെങ്കിൽ {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    എങ്കിൽ (is_inf) {
      makeInf(sign);
      തിരികെ;
    }
  }

  തർക്കം is_nan = അസത്യം;

  എങ്കിൽ constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    എങ്കിൽ constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      സ്ഥിരം integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;

      എങ്കിൽ (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_mask_no_int_bit) != 0) {
        // regular NaN and pseudoNaN
        is_nan = സത്യം;
      } അല്ലെങ്കിൽ എങ്കിൽ (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_mask_no_int_bit) == 0) {
        // pseudoinfinity
        is_nan = സത്യം;
      } അല്ലെങ്കിൽ എങ്കിൽ (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = സത്യം;
      }
    } അല്ലെങ്കിൽ {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } അല്ലെങ്കിൽ എങ്കിൽ constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    തർക്കം all_ones_significand =
        std::all_of(mysignificand.begin(), mysignificand.end() - 1,
                    [](integerPart bits) { തിരികെ bits == ~integerPart{0}; }) &&
        (!significand_mask ||
         mysignificand[mysignificand.size() - 1] == significand_mask);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } അല്ലെങ്കിൽ എങ്കിൽ constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  എങ്കിൽ (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.size(),
                significandParts());
    തിരികെ;
  }

  എങ്കിൽ (is_zero) {
    makeZero(sign);
    തിരികെ;
  }

  category = fcNormal;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.size(), significandParts());
  എങ്കിൽ (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  അല്ലെങ്കിൽ {
    എങ്കിൽ constexpr (!S.hasExplicitIntegerBit) {
      significandParts()[mysignificand.size() - 1] |= integer_bit;
    }
  }
}

ശൂന്യം IEEEFloat::initFromQuadrupleAPInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEquad>(api);
}

ശൂന്യം IEEEFloat::initFromDoubleAPInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEdouble>(api);
}

ശൂന്യം IEEEFloat::initFromFloatAPInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEsingle>(api);
}

ശൂന്യം IEEEFloat::initFromBFloatAPInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semBFloat>(api);
}

ശൂന്യം IEEEFloat::initFromHalfAPInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEhalf>(api);
}

ശൂന്യം IEEEFloat::initFromFloat8E5M2APInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2>(api);
}

ശൂന്യം IEEEFloat::initFromFloat8E5M2FNUZAPInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2FNUZ>(api);
}

ശൂന്യം IEEEFloat::initFromFloat8E4M3APInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3>(api);
}

ശൂന്യം IEEEFloat::initFromFloat8E4M3FNAPInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FN>(api);
}

ശൂന്യം IEEEFloat::initFromFloat8E4M3FNUZAPInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FNUZ>(api);
}

ശൂന്യം IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3B11FNUZ>(api);
}

ശൂന്യം IEEEFloat::initFromFloat8E3M4APInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E3M4>(api);
}

ശൂന്യം IEEEFloat::initFromFloatTF32APInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloatTF32>(api);
}

ശൂന്യം IEEEFloat::initFromFloat6E3M2FNAPInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E3M2FN>(api);
}

ശൂന്യം IEEEFloat::initFromFloat6E2M3FNAPInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E2M3FN>(api);
}

ശൂന്യം IEEEFloat::initFromFloat4E2M1FNAPInt(സ്ഥിരം APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
ശൂന്യം IEEEFloat::initFromAPInt(സ്ഥിരം fltSemantics *Sem, സ്ഥിരം APInt &api) {
  പ്രതിജ്ഞാ(api.getBitWidth() == Sem->sizeInBits);
  എങ്കിൽ (Sem == &APFloatBase::semIEEEhalf)
    തിരികെ initFromHalfAPInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semBFloat)
    തിരികെ initFromBFloatAPInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semIEEEsingle)
    തിരികെ initFromFloatAPInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semIEEEdouble)
    തിരികെ initFromDoubleAPInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semIEEEquad)
    തിരികെ initFromQuadrupleAPInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semX87DoubleExtended)
    തിരികെ initFromF80LongDoubleAPInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semPPCDoubleDoubleLegacy)
    തിരികെ initFromPPCDoubleDoubleLegacyAPInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semFloat8E5M2)
    തിരികെ initFromFloat8E5M2APInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semFloat8E5M2FNUZ)
    തിരികെ initFromFloat8E5M2FNUZAPInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semFloat8E4M3)
    തിരികെ initFromFloat8E4M3APInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semFloat8E4M3FN)
    തിരികെ initFromFloat8E4M3FNAPInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semFloat8E4M3FNUZ)
    തിരികെ initFromFloat8E4M3FNUZAPInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semFloat8E4M3B11FNUZ)
    തിരികെ initFromFloat8E4M3B11FNUZAPInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semFloat8E3M4)
    തിരികെ initFromFloat8E3M4APInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semFloatTF32)
    തിരികെ initFromFloatTF32APInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semFloat8E8M0FNU)
    തിരികെ initFromFloat8E8M0FNUAPInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semFloat8E5M3FNU)
    തിരികെ initFromFloat8E5M3FNUAPInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semFloat6E3M2FN)
    തിരികെ initFromFloat6E3M2FNAPInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semFloat6E2M3FN)
    തിരികെ initFromFloat6E2M3FNAPInt(api);
  എങ്കിൽ (Sem == &APFloatBase::semFloat4E2M1FN)
    തിരികെ initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
ശൂന്യം IEEEFloat::makeLargest(തർക്കം Negative) {
  എങ്കിൽ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 1..10
  //   significand = 1..1
  category = fcNormal;
  sign = Negative;
  exponent = semantics->maxExponent;

  // Use memset to set all but the highest integerPart to all ones.
  integerPart *significand = significandParts();
  unsigned Part_എണ്ണം = part_എണ്ണം();
  memset(significand, 0xFF, വലുപ്പം(integerPart)*(Part_എണ്ണം - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  സ്ഥിരം unsigned അക്കം_Unused_മുകളിലെ_Bits =
    Part_എണ്ണം*integerPartWidth - semantics->precision;
  significand[Part_എണ്ണം - 1] = (അക്കം_Unused_മുകളിലെ_Bits < integerPartWidth)
                                   ? (~integerPart(0) >> അക്കം_Unused_മുകളിലെ_Bits)
                                   : 0;
  എങ്കിൽ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
ശൂന്യം IEEEFloat::makeSmallest(തർക്കം Negative) {
  എങ്കിൽ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fcNormal;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tcSet(significandParts(), 1, part_എണ്ണം());
}

ശൂന്യം IEEEFloat::makeSmallestNormalized(തർക്കം Negative) {
  എങ്കിൽ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 10..0

  category = fcNormal;
  zeroSignificand();
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tcSetBit(significandParts(), semantics->precision - 1);
}

IEEEFloat::IEEEFloat(സ്ഥിരം fltSemantics &Sem, സ്ഥിരം APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(ദശാംശം f) {
  initFromAPInt(&APFloatBase::semIEEEsingle, APInt::floatToBits(f));
}

IEEEFloat::IEEEFloat(ഇരട്ട d) {
  initFromAPInt(&APFloatBase::semIEEEdouble, APInt::doubleToBits(d));
}

നാമാകാശ {
  ശൂന്യം append(SmallVectorImpl<അക്ഷരം> &Buffer, StringRef Str) {
    Buffer.append(Str.begin(), Str.end());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  ശൂന്യം AdjustToPrecision(APInt &significand,
                         പൂർണ്ണസംഖ്യ &exp, unsigned FormatPrecision) {
    unsigned bits = significand.getActiveBits();

    // 196/59 is a very slight overestimate of lg_2(10).
    unsigned bitsRequired = (FormatPrecision * 196 + 58) / 59;

    എങ്കിൽ (bits <= bitsRequired) തിരികെ;

    unsigned tensRemovable = (bits - bitsRequired) * 59 / 196;
    എങ്കിൽ (!tensRemovable) തിരികെ;

    exp += tensRemovable;

    APInt ഹാരകം(significand.getBitWidth(), 1);
    APInt powten(significand.getBitWidth(), 10);
    ആകുമ്പോൾ (സത്യം) {
      എങ്കിൽ (tensRemovable & 1)
        ഹാരകം *= powten;
      tensRemovable >>= 1;
      എങ്കിൽ (!tensRemovable) നിർത്തുക;
      powten *= powten;
    }

    significand = significand.udiv(ഹാരകം);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.getActiveBits());
  }


  ശൂന്യം AdjustToPrecision(SmallVectorImpl<അക്ഷരം> &buffer,
                         പൂർണ്ണസംഖ്യ &exp, unsigned FormatPrecision) {
    unsigned N = buffer.size();
    എങ്കിൽ (N <= FormatPrecision) തിരികെ;

    // The most significant figures are the last ones in the buffer.
    unsigned FirstSignificant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    എങ്കിൽ (buffer[FirstSignificant - 1] < '5') {
      ആകുമ്പോൾ (FirstSignificant < N && buffer[FirstSignificant] == '0')
        FirstSignificant++;

      exp += FirstSignificant;
      buffer.erase(&buffer[0], &buffer[FirstSignificant]);
      തിരികെ;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    വേണ്ടി (unsigned I = FirstSignificant; I != N; ++I) {
      എങ്കിൽ (buffer[I] == '9') {
        FirstSignificant++;
      } അല്ലെങ്കിൽ {
        buffer[I]++;
        നിർത്തുക;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    എങ്കിൽ (FirstSignificant == N) {
      exp += FirstSignificant;
      buffer.clear();
      buffer.തള്ളുക_back('1');
      തിരികെ;
    }

    exp += FirstSignificant;
    buffer.erase(&buffer[0], &buffer[FirstSignificant]);
  }

  ശൂന്യം toStringImpl(SmallVectorImpl<അക്ഷരം> &Str, സ്ഥിരം തർക്കം isNeg, പൂർണ്ണസംഖ്യ exp,
                    APInt significand, unsigned FormatPrecision,
                    unsigned FormatMaxPadding, തർക്കം TruncateZero) {
    സ്ഥിരം പൂർണ്ണസംഖ്യ semanticsPrecision = significand.getBitWidth();

    എങ്കിൽ (isNeg)
      Str.തള്ളുക_back('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    എങ്കിൽ (!FormatPrecision) {
      // We use enough digits so the number can be round-tripped back to an
      // APFloat. The formula comes from "How to Print Floating-Point Numbers
      // Accurately" by Steele and White.
      // FIXME: Using a formula based purely on the precision is conservative;
      // we can print fewer digits depending on the actual value being printed.

      // FormatPrecision = 2 + floor(significandBits / lg_2(10))
      FormatPrecision = 2 + semanticsPrecision * 59 / 196;
    }

    // Ignore trailing binary zeros.
    പൂർണ്ണസംഖ്യ trailingZeros = significand.countr_zero();
    exp += trailingZeros;
    significand.lshrInPlace(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    എങ്കിൽ (exp == 0) {
      // Nothing to do.
    } അല്ലെങ്കിൽ എങ്കിൽ (exp > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + exp);
      significand <<= exp;
      exp = 0;
    } അല്ലെങ്കിൽ { /* exp < 0 */
      പൂർണ്ണസംഖ്യ texp = -exp;

      // We transform this using the identity:
      //   (N)(2^-e) == (N)(5^e)(10^-e)
      // This means we have to multiply N (the significand) by 5^e.
      // To avoid overflow, we have to operate on numbers large
      // enough to store N * 5^e:
      //   log2(N * 5^e) == log2(N) + e * log2(5)
      //                 <= semantics->precision + e * 137 / 59
      //   (log_2(5) ~ 2.321928 < 2.322034 ~ 137/59)

      unsigned precision = semanticsPrecision + (137 * texp + 136) / 59;

      // Multiply significand by 5^e.
      //   N * 5^0101 == N * 5^(1*1) * 5^(0*2) * 5^(1*4) * 5^(0*8)
      significand = significand.zext(precision);
      APInt five_to_the_i(precision, 5);
      ആകുമ്പോൾ (സത്യം) {
        എങ്കിൽ (texp & 1)
          significand *= five_to_the_i;

        texp >>= 1;
        എങ്കിൽ (!texp)
          നിർത്തുക;
        five_to_the_i *= five_to_the_i;
      }
    }

    AdjustToPrecision(significand, exp, FormatPrecision);

    SmallVector<അക്ഷരം, 256> buffer;

    // Fill the buffer.
    unsigned precision = significand.getBitWidth();
    എങ്കിൽ (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    തർക്കം inTrail = സത്യം;
    ആകുമ്പോൾ (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      unsigned d = digit.get_ZExt_മൂല്യം();

      // Drop trailing zeros.
      എങ്കിൽ (inTrail && !d)
        exp++;
      അല്ലെങ്കിൽ {
        buffer.തള്ളുക_back((അക്ഷരം) ('0' + d));
        inTrail = അസത്യം;
      }
    }

    പ്രതിജ്ഞാ(!buffer.empty() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    AdjustToPrecision(buffer, exp, FormatPrecision);

    unsigned NDigits = buffer.size();

    // Check whether we should use scientific notation.
    തർക്കം FormatScientific;
    എങ്കിൽ (!FormatMaxPadding) {
      FormatScientific = സത്യം;
    } അല്ലെങ്കിൽ {
      എങ്കിൽ (exp >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((unsigned) exp > FormatMaxPadding ||
                            NDigits + (unsigned) exp > FormatPrecision);
      } അല്ലെങ്കിൽ {
        // Power of the most significant digit.
        പൂർണ്ണസംഖ്യ MSD = exp + (പൂർണ്ണസംഖ്യ) (NDigits - 1);
        എങ്കിൽ (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = അസത്യം;
        } അല്ലെങ്കിൽ {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((unsigned) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    എങ്കിൽ (FormatScientific) {
      exp += (NDigits - 1);

      Str.തള്ളുക_back(buffer[NDigits-1]);
      Str.തള്ളുക_back('.');
      എങ്കിൽ (NDigits == 1 && TruncateZero)
        Str.തള്ളുക_back('0');
      അല്ലെങ്കിൽ
        വേണ്ടി (unsigned I = 1; I != NDigits; ++I)
          Str.തള്ളുക_back(buffer[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      എങ്കിൽ (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.തള്ളുക_back(TruncateZero ? 'E' : 'e');

      Str.തള്ളുക_back(exp >= 0 ? '+' : '-');
      എങ്കിൽ (exp < 0)
        exp = -exp;
      SmallVector<അക്ഷരം, 6> expbuf;
      ചെയ്യുക {
        expbuf.തള്ളുക_back((അക്ഷരം) ('0' + (exp % 10)));
        exp /= 10;
      } ആകുമ്പോൾ (exp);
      // Exponent always at least two digits if we do not truncate zeros.
      എങ്കിൽ (!TruncateZero && expbuf.size() < 2)
        expbuf.തള്ളുക_back('0');
      വേണ്ടി (unsigned I = 0, E = expbuf.size(); I != E; ++I)
        Str.തള്ളുക_back(expbuf[E-1-I]);
      തിരികെ;
    }

    // Non-scientific, positive exponents.
    എങ്കിൽ (exp >= 0) {
      വേണ്ടി (unsigned I = 0; I != NDigits; ++I)
        Str.തള്ളുക_back(buffer[NDigits-1-I]);
      വേണ്ടി (unsigned I = 0; I != (unsigned) exp; ++I)
        Str.തള്ളുക_back('0');
      തിരികെ;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    പൂർണ്ണസംഖ്യ NWholeDigits = exp + (പൂർണ്ണസംഖ്യ) NDigits;

    unsigned I = 0;
    എങ്കിൽ (NWholeDigits > 0) {
      വേണ്ടി (; I != (unsigned) NWholeDigits; ++I)
        Str.തള്ളുക_back(buffer[NDigits-I-1]);
      Str.തള്ളുക_back('.');
    } അല്ലെങ്കിൽ {
      unsigned NZeros = 1 + (unsigned) -NWholeDigits;

      Str.തള്ളുക_back('0');
      Str.തള്ളുക_back('.');
      വേണ്ടി (unsigned Z = 1; Z != NZeros; ++Z)
        Str.തള്ളുക_back('0');
    }

    വേണ്ടി (; I != NDigits; ++I)
      Str.തള്ളുക_back(buffer[NDigits-I-1]);

  }
} // namespace

ശൂന്യം IEEEFloat::toString(SmallVectorImpl<അക്ഷരം> &Str, unsigned FormatPrecision,
                         unsigned FormatMaxPadding, തർക്കം TruncateZero) സ്ഥിരം {
  തിരഞ്ഞെടുപ്പ് (category) {
  അവസ്ഥ fcInfinity:
    എങ്കിൽ (isNegative())
      തിരികെ append(Str, "-Inf");
    അല്ലെങ്കിൽ
      തിരികെ append(Str, "+Inf");

  അവസ്ഥ fcNaN: തിരികെ append(Str, "NaN");

  അവസ്ഥ fcZero:
    എങ്കിൽ (isNegative())
      Str.തള്ളുക_back('-');

    എങ്കിൽ (!FormatMaxPadding) {
      എങ്കിൽ (TruncateZero)
        append(Str, "0.0E+0");
      അല്ലെങ്കിൽ {
        append(Str, "0.0");
        എങ്കിൽ (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } അല്ലെങ്കിൽ {
      Str.തള്ളുക_back('0');
    }
    തിരികെ;

  അവസ്ഥ fcNormal:
    നിർത്തുക;
  }

  // Decompose the number into an APInt and an exponent.
  പൂർണ്ണസംഖ്യ exp = exponent - ((പൂർണ്ണസംഖ്യ) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_എണ്ണം_For_Bits(semantics->precision)));

  toStringImpl(Str, isNegative(), exp, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

പൂർണ്ണസംഖ്യ IEEEFloat::getExactLog2Abs() സ്ഥിരം {
  എങ്കിൽ (!isFinite() || isZero())
    തിരികെ INT_MIN;

  സ്ഥിരം integerPart *Parts = significandParts();
  സ്ഥിരം പൂർണ്ണസംഖ്യ Part_എണ്ണം = part_എണ്ണം_For_Bits(semantics->precision);

  പൂർണ്ണസംഖ്യ എടുക്കുക_എണ്ണം = 0;
  വേണ്ടി (പൂർണ്ണസംഖ്യ i = 0; i < Part_എണ്ണം; ++i) {
    എടുക്കുക_എണ്ണം += llvm::popcount(Parts[i]);
    എങ്കിൽ (എടുക്കുക_എണ്ണം > 1)
      തിരികെ INT_MIN;
  }

  എങ്കിൽ (exponent != semantics->minExponent)
    തിരികെ exponent;

  പൂർണ്ണസംഖ്യ CountrParts = 0;
  വേണ്ടി (പൂർണ്ണസംഖ്യ i = 0; i < Part_എണ്ണം;
       ++i, CountrParts += APInt::APINT_BITS_PER_WORD) {
    എങ്കിൽ (Parts[i] != 0) {
      തിരികെ exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

തർക്കം IEEEFloat::isSignaling() സ്ഥിരം {
  എങ്കിൽ (!isNaN())
    തിരികെ അസത്യം;
  എങ്കിൽ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    തിരികെ അസത്യം;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  തിരികെ !APInt::tcExtractBit(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::opStatus IEEEFloat::next(തർക്കം nextDown) {
  // If we are performing nextDown, swap sign so we have -x.
  എങ്കിൽ (nextDown)
    changeSign();

  // Compute nextUp(x)
  opStatus result = opOK;

  // Handle each float category separately.
  തിരഞ്ഞെടുപ്പ് (category) {
  അവസ്ഥ fcInfinity:
    // nextUp(+inf) = +inf
    എങ്കിൽ (!isNegative())
      നിർത്തുക;
    // nextUp(-inf) = -getLargest()
    makeLargest(സത്യം);
    നിർത്തുക;
  അവസ്ഥ fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    എങ്കിൽ (isSignaling()) {
      result = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(അസത്യം, isNegative(), nullptr);
    }
    നിർത്തുക;
  അവസ്ഥ fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(അസത്യം);
    നിർത്തുക;
  അവസ്ഥ fcNormal:
    // nextUp(-getSmallest()) = -0
    എങ്കിൽ (isSmallest() && isNegative()) {
      APInt::tcSet(significandParts(), 0, part_എണ്ണം());
      category = fcZero;
      exponent = 0;
      എങ്കിൽ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = അസത്യം;
      എങ്കിൽ (!semantics->hasZero)
        makeSmallestNormalized(അസത്യം);
      നിർത്തുക;
    }

    എങ്കിൽ (isLargest() && !isNegative()) {
      എങ്കിൽ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        നിർത്തുക;
      } അല്ലെങ്കിൽ എങ്കിൽ (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        നിർത്തുക;
      } അല്ലെങ്കിൽ {
        // nextUp(getLargest()) == INFINITY
        APInt::tcSet(significandParts(), 0, part_എണ്ണം());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        നിർത്തുക;
      }
    }

    // nextUp(normal) == normal + inc.
    എങ്കിൽ (isNegative()) {
      // If we are negative, we need to decrement the significand.

      // We only cross a binade boundary that requires adjusting the exponent
      // if:
      //   1. exponent != semantics->minExponent. This implies we are not in the
      //   smallest binade or are dealing with denormals.
      //   2. Our significand excluding the integral bit is all zeros.
      തർക്കം WillCrossBinadeBoundary =
        exponent != semantics->minExponent && isSignificandAllZeros();

      // Decrement the significand.
      //
      // We always do this since:
      //   1. If we are dealing with a non-binade decrement, by definition we
      //   just decrement the significand.
      //   2. If we are dealing with a normal -> normal binade decrement, since
      //   we have an explicit integral bit the fact that all bits but the
      //   integral bit are zero implies that subtracting one will yield a
      //   significand with 0 integral bit and 1 in all other spots. Thus we
      //   must just adjust the exponent and set the integral bit to 1.
      //   3. If we are dealing with a normal -> denormal binade decrement,
      //   since we set the integral bit to 0 when we represent denormals, we
      //   just decrement the significand.
      integerPart *Parts = significandParts();
      APInt::tcDecrement(Parts, part_എണ്ണം());

      എങ്കിൽ (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tcSetBit(Parts, semantics->precision - 1);
        exponent--;
      }
    } അല്ലെങ്കിൽ {
      // If we are positive, we need to increment the significand.

      // We only cross a binade boundary that requires adjusting the exponent if
      // the input is not a denormal and all of said input's significand bits
      // are set. If all of said conditions are true: clear the significand, set
      // the integral bit to 1, and increment the exponent. If we have a
      // denormal always increment since moving denormals and the numbers in the
      // smallest normal binade have the same exponent in our representation.
      // If there are only exponents, any increment always crosses the
      // BinadeBoundary.
      തർക്കം WillCrossBinadeBoundary = !APFloat::hasSignificand(*semantics) ||
                                     (!isDenormal() && isSignificandAllOnes());

      എങ്കിൽ (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tcSet(Parts, 0, part_എണ്ണം());
        APInt::tcSetBit(Parts, semantics->precision - 1);
        പ്രതിജ്ഞാ(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } അല്ലെങ്കിൽ {
        incrementSignificand();
      }
    }
    നിർത്തുക;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  എങ്കിൽ (nextDown)
    changeSign();

  തിരികെ result;
}

APInt IEEEFloat::getNaNPayload() സ്ഥിരം {
  പ്രതിജ്ഞാ(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  unsigned Bits = semantics->precision - 1;
  തിരികെ APInt(Bits, ArrayRef(significandParts(), part_എണ്ണം_For_Bits(Bits)));
}

APFloatBase::ExponentType IEEEFloat::exponentNaN() സ്ഥിരം {
  തിരികെ ::exponentNaN(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentInf() സ്ഥിരം {
  തിരികെ ::exponentInf(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentZero() സ്ഥിരം {
  തിരികെ ::exponentZero(*semantics);
}

ശൂന്യം IEEEFloat::makeInf(തർക്കം Negative) {
  എങ്കിൽ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  എങ്കിൽ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(അസത്യം, Negative);
    തിരികെ;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tcSet(significandParts(), 0, part_എണ്ണം());
}

ശൂന്യം IEEEFloat::makeZero(തർക്കം Negative) {
  എങ്കിൽ (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  എങ്കിൽ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = അസത്യം;
  }
  exponent = exponentZero();
  APInt::tcSet(significandParts(), 0, part_എണ്ണം());
}

ശൂന്യം IEEEFloat::makeQuiet() {
  പ്രതിജ്ഞാ(isNaN());
  എങ്കിൽ (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tcSetBit(significandParts(), semantics->precision - 2);
}

പൂർണ്ണസംഖ്യ ilogb(സ്ഥിരം IEEEFloat &Arg) {
  എങ്കിൽ (Arg.isNaN())
    തിരികെ APFloat::IEK_NaN;
  എങ്കിൽ (Arg.isZero())
    തിരികെ APFloat::IEK_Zero;
  എങ്കിൽ (Arg.isInfinity())
    തിരികെ APFloat::IEK_Inf;
  എങ്കിൽ (!Arg.isDenormal())
    തിരികെ Arg.exponent;

  IEEEFloat Normalized(Arg);
  പൂർണ്ണസംഖ്യ SignificandBits = Arg.getSemantics().precision - 1;

  Normalized.exponent += SignificandBits;
  Normalized.normalize(APFloat::rmNearestTiesToEven, lfExactlyZero);
  തിരികെ Normalized.exponent - SignificandBits;
}

IEEEFloat scalbn(IEEEFloat X, പൂർണ്ണസംഖ്യ Exp, roundingMode RoundingMode) {
  auto MaxExp = X.getSemantics().maxExponent;
  auto MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  പൂർണ്ണസംഖ്യ SignificandBits = X.getSemantics().precision - 1;
  പൂർണ്ണസംഖ്യ MaxIncrement = MaxExp - (MinExp - SignificandBits) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(RoundingMode, lfExactlyZero);
  എങ്കിൽ (X.isNaN())
    X.makeQuiet();
  തിരികെ X;
}

IEEEFloat frexp(സ്ഥിരം IEEEFloat &Val, പൂർണ്ണസംഖ്യ &Exp, roundingMode RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  എങ്കിൽ (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    തിരികെ Quiet;
  }

  എങ്കിൽ (Exp == APFloat::IEK_Inf)
    തിരികെ Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  തിരികെ scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(സ്ഥിരം fltSemantics &S)
    : Semantics(&S),
      Floats(പുതിയ APFloat[2]{APFloat(APFloatBase::semIEEEdouble),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(സ്ഥിരം fltSemantics &S, uninitializedTag)
    : Semantics(&S), Floats(പുതിയ APFloat[2]{
                         APFloat(APFloatBase::semIEEEdouble, uninitialized),
                         APFloat(APFloatBase::semIEEEdouble, uninitialized)}) {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(സ്ഥിരം fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(പുതിയ APFloat[2]{APFloat(APFloatBase::semIEEEdouble, I),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(സ്ഥിരം fltSemantics &S, സ്ഥിരം APInt &I)
    : Semantics(&S),
      Floats(പുതിയ APFloat[2]{
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[0])),
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[1]))}) {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(സ്ഥിരം fltSemantics &S, APFloat &&First,
                             APFloat &&Second)
    : Semantics(&S),
      Floats(പുതിയ APFloat[2]{std::move(First), std::move(Second)}) {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble);
  പ്രതിജ്ഞാ(&Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  പ്രതിജ്ഞാ(&Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(സ്ഥിരം DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? പുതിയ APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : nullptr) {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloatBase::semBogus;
  RHS.Floats = nullptr;
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::operator=(സ്ഥിരം DoubleAPFloat &RHS) {
  എങ്കിൽ (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } അല്ലെങ്കിൽ എങ്കിൽ (ഇത് != &RHS) {
    ഇത്->~DoubleAPFloat();
    പുതിയ (ഇത്) DoubleAPFloat(RHS);
  }
  തിരികെ *ഇത്;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
സ്ഥിതം std::pair<APFloat, APFloat> fastTwoSum(APFloat X, APFloat Y) {
  എങ്കിൽ (!X.isFinite())
    തിരികെ {X, APFloat::getZero(X.getSemantics(), /*Negative=*/അസത്യം)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  തിരികെ {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::opStatus DoubleAPFloat::addImpl(സ്ഥിരം APFloat &a, സ്ഥിരം APFloat &aa,
                                         സ്ഥിരം APFloat &c, സ്ഥിരം APFloat &cc,
                                         roundingMode RM) {
  പൂർണ്ണസംഖ്യ Status = opOK;
  APFloat z = a;
  Status |= z.add(c, RM);
  എങ്കിൽ (!z.isFinite()) {
    എങ്കിൽ (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ അസത്യം);
      തിരികെ (opStatus)Status;
    }
    Status = opOK;
    auto AComparedToC = a.compare_Absolute_മൂല്യം(c);
    z = cc;
    Status |= z.add(aa, RM);
    എങ്കിൽ (AComparedToC == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      Status |= z.add(c, RM);
      Status |= z.add(a, RM);
    } അല്ലെങ്കിൽ {
      // z = cc + aa + a + c;
      Status |= z.add(a, RM);
      Status |= z.add(c, RM);
    }
    എങ്കിൽ (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ അസത്യം);
      തിരികെ (opStatus)Status;
    }
    Floats[0] = z;
    APFloat zz = aa;
    Status |= zz.add(cc, RM);
    എങ്കിൽ (AComparedToC == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(c, RM);
      Status |= Floats[1].add(zz, RM);
    } അല്ലെങ്കിൽ {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(a, RM);
      Status |= Floats[1].add(zz, RM);
    }
  } അല്ലെങ്കിൽ {
    // q = a - z;
    APFloat q = a;
    Status |= q.subtract(z, RM);

    // zz = q + c + (a - (q + z)) + aa + cc;
    // Compute a - (q + z) as -((q + z) - a) to avoid temporary copies.
    auto zz = q;
    Status |= zz.add(c, RM);
    Status |= q.add(z, RM);
    Status |= q.subtract(a, RM);
    q.changeSign();
    Status |= zz.add(q, RM);
    Status |= zz.add(aa, RM);
    Status |= zz.add(cc, RM);
    എങ്കിൽ (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ അസത്യം);
      തിരികെ opOK;
    }
    Floats[0] = z;
    Status |= Floats[0].add(zz, RM);
    എങ്കിൽ (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ അസത്യം);
      തിരികെ (opStatus)Status;
    }
    Floats[1] = std::move(z);
    Status |= Floats[1].subtract(Floats[0], RM);
    Status |= Floats[1].add(zz, RM);
  }
  തിരികെ (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::addWithSpecial(സ്ഥിരം DoubleAPFloat &LHS,
                                                സ്ഥിരം DoubleAPFloat &RHS,
                                                DoubleAPFloat &Out,
                                                roundingMode RM) {
  എങ്കിൽ (LHS.getCategory() == fcNaN) {
    Out = LHS;
    തിരികെ opOK;
  }
  എങ്കിൽ (RHS.getCategory() == fcNaN) {
    Out = RHS;
    തിരികെ opOK;
  }
  എങ്കിൽ (LHS.getCategory() == fcZero) {
    Out = RHS;
    തിരികെ opOK;
  }
  എങ്കിൽ (RHS.getCategory() == fcZero) {
    Out = LHS;
    തിരികെ opOK;
  }
  എങ്കിൽ (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    Out.makeNaN(അസത്യം, Out.isNegative(), nullptr);
    തിരികെ opInvalidOp;
  }
  എങ്കിൽ (LHS.getCategory() == fcInfinity) {
    Out = LHS;
    തിരികെ opOK;
  }
  എങ്കിൽ (RHS.getCategory() == fcInfinity) {
    Out = RHS;
    തിരികെ opOK;
  }
  പ്രതിജ്ഞാ(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  പ്രതിജ്ഞാ(&A.getSemantics() == &APFloatBase::semIEEEdouble);
  പ്രതിജ്ഞാ(&AA.getSemantics() == &APFloatBase::semIEEEdouble);
  പ്രതിജ്ഞാ(&C.getSemantics() == &APFloatBase::semIEEEdouble);
  പ്രതിജ്ഞാ(&CC.getSemantics() == &APFloatBase::semIEEEdouble);
  പ്രതിജ്ഞാ(&Out.Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  പ്രതിജ്ഞാ(&Out.Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
  തിരികെ Out.addImpl(A, AA, C, CC, RM);
}

APFloat::opStatus DoubleAPFloat::add(സ്ഥിരം DoubleAPFloat &RHS,
                                     roundingMode RM) {
  തിരികെ addWithSpecial(*ഇത്, RHS, *ഇത്, RM);
}

APFloat::opStatus DoubleAPFloat::subtract(സ്ഥിരം DoubleAPFloat &RHS,
                                          roundingMode RM) {
  changeSign();
  auto Ret = add(RHS, RM);
  changeSign();
  തിരികെ Ret;
}

APFloat::opStatus DoubleAPFloat::multiply(സ്ഥിരം DoubleAPFloat &RHS,
                                          APFloat::roundingMode RM) {
  സ്ഥിരം auto &LHS = *ഇത്;
  auto &Out = *ഇത്;
  /* Interesting observation: For special categories, finding the lowest
     common ancestor of the following layered graph gives the correct
     return category:

        NaN
       /   \
     Zero  Inf
       \   /
       Normal

     e.g. NaN * NaN = NaN
          Zero * Inf = NaN
          Normal * Zero = Zero
          Normal * Inf = Inf
  */
  എങ്കിൽ (LHS.getCategory() == fcNaN) {
    Out = LHS;
    തിരികെ opOK;
  }
  എങ്കിൽ (RHS.getCategory() == fcNaN) {
    Out = RHS;
    തിരികെ opOK;
  }
  എങ്കിൽ ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    Out.makeNaN(അസത്യം, അസത്യം, nullptr);
    തിരികെ opOK;
  }
  എങ്കിൽ (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    Out = LHS;
    തിരികെ opOK;
  }
  എങ്കിൽ (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    Out = RHS;
    തിരികെ opOK;
  }
  പ്രതിജ്ഞാ(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal &&
         "Special cases not handled exhaustively");

  പൂർണ്ണസംഖ്യ Status = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  Status |= T.multiply(C, RM);
  എങ്കിൽ (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ അസത്യം);
    തിരികെ (opStatus)Status;
  }

  // tau = fmsub(a, c, t), that is -fmadd(-a, c, t).
  APFloat Tau = A;
  T.changeSign();
  Status |= Tau.fusedMultiplyAdd(C, T, RM);
  T.changeSign();
  {
    // v = a * d
    APFloat V = A;
    Status |= V.multiply(D, RM);
    // w = b * c
    APFloat W = B;
    Status |= W.multiply(C, RM);
    Status |= V.add(W, RM);
    // tau += v + w
    Status |= Tau.add(V, RM);
  }
  // u = t + tau
  APFloat U = T;
  Status |= U.add(Tau, RM);

  Floats[0] = U;
  എങ്കിൽ (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ അസത്യം);
  } അല്ലെങ്കിൽ {
    // Floats[1] = (t - u) + tau
    Status |= T.subtract(U, RM);
    Status |= T.add(Tau, RM);
    Floats[1] = std::move(T);
  }
  തിരികെ (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::divide(സ്ഥിരം DoubleAPFloat &RHS,
                                        APFloat::roundingMode RM) {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.divide(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()), RM);
  *ഇത് = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  തിരികെ Ret;
}

APFloat::opStatus DoubleAPFloat::remainder(സ്ഥിരം DoubleAPFloat &RHS) {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.remainder(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *ഇത് = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  തിരികെ Ret;
}

APFloat::opStatus DoubleAPFloat::mod(സ്ഥിരം DoubleAPFloat &RHS) {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.mod(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *ഇത് = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  തിരികെ Ret;
}

APFloat::opStatus
DoubleAPFloat::fusedMultiplyAdd(സ്ഥിരം DoubleAPFloat &Multiplicand,
                                സ്ഥിരം DoubleAPFloat &Addend,
                                APFloat::roundingMode RM) {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.fusedMultiplyAdd(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcastToAPInt()),
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, Addend.bitcastToAPInt()),
      RM);
  *ഇത് = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  തിരികെ Ret;
}

APFloat::opStatus DoubleAPFloat::roundToIntegral(APFloat::roundingMode RM) {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  സ്ഥിരം APFloat &Hi = getFirst();
  സ്ഥിരം APFloat &Lo = getSecond();

  APFloat RoundedHi = Hi;
  സ്ഥിരം opStatus HiStatus = RoundedHi.roundToIntegral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  എങ്കിൽ (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/അസത്യം);
    തിരികെ HiStatus;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  auto RoundToNearestHelper = [](APFloat ToRound, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    സ്ഥിരം APFloat RoundingError = Rounded - ToRound;
    എങ്കിൽ (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        abs(RoundingError).is_Exactly_മൂല്യം(0.5))
      Rounded.add(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rmNearestTiesToEven);
    തിരികെ Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  എങ്കിൽ (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    എങ്കിൽ (RM == rmNearestTiesToAway || RM == rmNearestTiesToEven)
      RoundedHi = RoundToNearestHelper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/അസത്യം);
    തിരികെ HiStatus;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  RoundingMode LoRoundingMode;
  എങ്കിൽ (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    LoRoundingMode = isNegative() ? rmTowardPositive : rmTowardNegative;
  അല്ലെങ്കിൽ
    LoRoundingMode = RM;

  APFloat RoundedLo = Lo;
  സ്ഥിരം opStatus LoStatus = RoundedLo.roundToIntegral(LoRoundingMode);
  എങ്കിൽ (LoRoundingMode == rmNearestTiesToAway)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = RoundToNearestHelper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fastTwoSum(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  തിരികെ LoStatus;
}

ശൂന്യം DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmpResult
DoubleAPFloat::compare_Absolute_മൂല്യം(സ്ഥിരം DoubleAPFloat &RHS) സ്ഥിരം {
  // Compare absolute values of the high parts.
  സ്ഥിരം cmpResult HiPartCmp = Floats[0].compare_Absolute_മൂല്യം(RHS.Floats[0]);
  എങ്കിൽ (HiPartCmp != cmpEqual)
    തിരികെ HiPartCmp;

  // Zero, regardless of sign, is equal.
  എങ്കിൽ (Floats[1].isZero() && RHS.Floats[1].isZero())
    തിരികെ cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  സ്ഥിരം തർക്കം ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  സ്ഥിരം തർക്കം RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  എങ്കിൽ (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    തിരികെ RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  എങ്കിൽ (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    തിരികെ ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  എങ്കിൽ (ThisIsSubtractive != RHSIsSubtractive)
    തിരികെ ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  സ്ഥിരം cmpResult LoPartCmp = Floats[1].compare_Absolute_മൂല്യം(RHS.Floats[1]);

  എങ്കിൽ (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    എങ്കിൽ (LoPartCmp == cmpLessThan)
      തിരികെ cmpGreaterThan;
    എങ്കിൽ (LoPartCmp == cmpGreaterThan)
      തിരികെ cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  തിരികെ LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() സ്ഥിരം {
  തിരികെ Floats[0].getCategory();
}

തർക്കം DoubleAPFloat::isNegative() സ്ഥിരം { തിരികെ Floats[0].isNegative(); }

ശൂന്യം DoubleAPFloat::makeInf(തർക്കം Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ അസത്യം);
}

ശൂന്യം DoubleAPFloat::makeZero(തർക്കം Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ അസത്യം);
}

ശൂന്യം DoubleAPFloat::makeLargest(തർക്കം Neg) {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  എങ്കിൽ (Neg)
    changeSign();
}

ശൂന്യം DoubleAPFloat::makeSmallest(തർക്കം Neg) {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].makeSmallest(Neg);
  Floats[1].makeZero(/* Neg = */ അസത്യം);
}

ശൂന്യം DoubleAPFloat::makeSmallestNormalized(തർക്കം Neg) {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  എങ്കിൽ (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ അസത്യം);
}

ശൂന്യം DoubleAPFloat::makeNaN(തർക്കം SNaN, തർക്കം Neg, സ്ഥിരം APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ അസത്യം);
}

APFloat::cmpResult DoubleAPFloat::compare(സ്ഥിരം DoubleAPFloat &RHS) സ്ഥിരം {
  auto Result = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  എങ്കിൽ (Result == APFloat::cmpEqual)
    തിരികെ Floats[1].compare(RHS.Floats[1]);
  തിരികെ Result;
}

തർക്കം DoubleAPFloat::bitwiseIsEqual(സ്ഥിരം DoubleAPFloat &RHS) സ്ഥിരം {
  തിരികെ Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_code hash_മൂല്യം(സ്ഥിരം DoubleAPFloat &Arg) {
  എങ്കിൽ (Arg.Floats)
    തിരികെ hash_combine(hash_മൂല്യം(Arg.Floats[0]), hash_മൂല്യം(Arg.Floats[1]));
  തിരികെ hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcastToAPInt() സ്ഥിരം {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t Data[] = {
      Floats[0].bitcastToAPInt().getRawData()[0],
      Floats[1].bitcastToAPInt().getRawData()[0],
  };
  തിരികെ APInt(128, Data);
}

Expected<APFloat::opStatus> DoubleAPFloat::convertFromString(StringRef S,
                                                             roundingMode RM) {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy);
  auto Ret = Tmp.convertFromString(S, RM);
  *ഇത് = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  തിരികെ Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::opStatus DoubleAPFloat::next(തർക്കം nextDown) {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  എങ്കിൽ (nextDown) {
    changeSign();
    APFloat::opStatus Result = next(/*nextDown=*/അസത്യം);
    changeSign();
    തിരികെ Result;
  }
  തിരഞ്ഞെടുപ്പ് (getCategory()) {
  അവസ്ഥ fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    എങ്കിൽ (isNegative())
      makeLargest(സത്യം);
    തിരികെ opOK;

  അവസ്ഥ fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    എങ്കിൽ (getFirst().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(അസത്യം, isNegative(), nullptr);
      തിരികെ opInvalidOp;
    }
    തിരികെ opOK;

  അവസ്ഥ fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(അസത്യം);
    തിരികെ opOK;

  അവസ്ഥ fcNormal:
    നിർത്തുക;
  }

  സ്ഥിരം APFloat &HiOld = getFirst();
  സ്ഥിരം APFloat &LoOld = getSecond();

  APFloat NextLo = LoOld;
  NextLo.next(/*nextDown=*/അസത്യം);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  auto InLattice = [](സ്ഥിരം APFloat &Hi, സ്ഥിരം APFloat &Lo) {
    തിരികെ Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  എങ്കിൽ (InLattice(HiOld, NextLo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(NextLo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat Largest{*Semantics, uninitialized};
    Largest.makeLargest(/*Neg=*/അസത്യം);
    എങ്കിൽ (compare(Largest) == cmpGreaterThan)
      makeInf(/*Neg=*/അസത്യം);

    തിരികെ opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat NextHi = HiOld;
  NextHi.next(/*nextDown=*/അസത്യം);

  // nextUp(getLargest()) == INFINITY
  എങ്കിൽ (NextHi.isInfinity()) {
    makeInf(/*Neg=*/അസത്യം);
    തിരികെ opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  എങ്കിൽ (NextHi.isZero()) {
    makeZero(/*Neg=*/സത്യം);
    തിരികെ opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  NextLo = neg(scalbn(harrisonUlp(NextHi), -1, rmTowardZero));
  എങ്കിൽ (!InLattice(NextHi, NextLo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    NextLo.next(/*nextDown=*/അസത്യം);

  Floats[0] = std::move(NextHi);
  Floats[1] = std::move(NextLo);

  തിരികെ opOK;
}

APFloat::opStatus DoubleAPFloat::convertToSignExtendedInteger(
    MutableArrayRef<integerPart> നിവേശം, unsigned പൂർണ്ണസംഖ്യ Width, തർക്കം IsSigned,
    roundingMode RM, തർക്കം *IsExact) സ്ഥിരം {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  എങ്കിൽ (!getFirst().isFiniteNonZero() || getSecond().isZero())
    തിരികെ getFirst().convertToInteger(നിവേശം, Width, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = അസത്യം;
  DoubleAPFloat Integral = *ഇത്;
  സ്ഥിരം opStatus RoundStatus = Integral.roundToIntegral(RM);
  എങ്കിൽ (RoundStatus == opInvalidOp)
    തിരികെ opInvalidOp;
  സ്ഥിരം APFloat &IntegralHi = Integral.getFirst();
  സ്ഥിരം APFloat &IntegralLo = Integral.getSecond();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  തർക്കം HiIsExact;
  എങ്കിൽ (IntegralHi.isZero() || IntegralLo.isZero()) {
    സ്ഥിരം opStatus HiStatus =
        IntegralHi.convertToInteger(നിവേശം, Width, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    തിരികെ HiStatus == opOK ? opInexact : HiStatus;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  എങ്കിൽ (!IsSigned && IntegralHi.isNegative())
    തിരികെ opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  തർക്കം LoIsExact;
  സ്ഥിരം പൂർണ്ണസംഖ്യ HiExactLog2 = IntegralHi.getExactLog2Abs();
  സ്ഥിരം unsigned PositiveOverflowWidth = IsSigned ? Width - 1 : Width;
  എങ്കിൽ (HiExactLog2 >= 0 &&
      static_cast<unsigned>(HiExactLog2) == PositiveOverflowWidth) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    എങ്കിൽ (IntegralHi.isNegative() == IntegralLo.isNegative())
      തിരികെ opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    സ്ഥിരം opStatus LoStatus = IntegralLo.convertToInteger(
        നിവേശം, Width, /*IsSigned=*/സത്യം, RM, &LoIsExact);
    എങ്കിൽ (LoStatus == opInvalidOp)
      തിരികെ opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    എങ്കിൽ (IsSigned && !IntegralHi.isNegative())
      APInt::tcClearBit(നിവേശം.data(), PositiveOverflowWidth);
    *IsExact = RoundStatus == opOK;
    തിരികെ RoundStatus;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  സ്ഥിരം opStatus HiStatus = IntegralHi.convertToInteger(
      നിവേശം, Width, IsSigned, rmTowardZero, &HiIsExact);
  എങ്കിൽ (HiStatus == opInvalidOp)
    തിരികെ HiStatus;

  // Convert Lo into a temporary integer of the same width.
  APSInt LoResult{Width, /*isUnsigned=*/!IsSigned};
  സ്ഥിരം opStatus LoStatus =
      IntegralLo.convertToInteger(LoResult, rmTowardZero, &LoIsExact);
  എങ്കിൽ (LoStatus == opInvalidOp)
    തിരികെ LoStatus;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tcAdd(നിവേശം.data(), LoResult.getRawData(), /*carry=*/0, നിവേശം.size());

  *IsExact = RoundStatus == opOK;
  തിരികെ RoundStatus;
}

APFloat::opStatus
DoubleAPFloat::convertToInteger(MutableArrayRef<integerPart> നിവേശം,
                                unsigned പൂർണ്ണസംഖ്യ Width, തർക്കം IsSigned,
                                roundingMode RM, തർക്കം *IsExact) സ്ഥിരം {
  opStatus FS =
      convertToSignExtendedInteger(നിവേശം, Width, IsSigned, RM, IsExact);

  എങ്കിൽ (FS == opInvalidOp) {
    സ്ഥിരം unsigned Dst_Parts_എണ്ണം = part_എണ്ണം_For_Bits(Width);
    പ്രതിജ്ഞാ(Dst_Parts_എണ്ണം <= നിവേശം.size() && "Integer too big");

    unsigned Bits;
    എങ്കിൽ (getCategory() == fcNaN)
      Bits = 0;
    അല്ലെങ്കിൽ എങ്കിൽ (isNegative())
      Bits = IsSigned;
    അല്ലെങ്കിൽ
      Bits = Width - IsSigned;

    tcSetLeastSignificantBits(നിവേശം.data(), Dst_Parts_എണ്ണം, Bits);
    എങ്കിൽ (isNegative() && IsSigned)
      APInt::tc_Shift_ഇടത്(നിവേശം.data(), Dst_Parts_എണ്ണം, Width - 1);
  }

  തിരികെ FS;
}

APFloat::opStatus DoubleAPFloat::handleOverflow(roundingMode RM) {
  തിരഞ്ഞെടുപ്പ് (RM) {
  അവസ്ഥ APFloat::rmTowardZero:
    makeLargest(/*Neg=*/isNegative());
    നിർത്തുക;
  അവസ്ഥ APFloat::rmTowardNegative:
    എങ്കിൽ (isNegative())
      makeInf(/*Neg=*/സത്യം);
    അല്ലെങ്കിൽ
      makeLargest(/*Neg=*/അസത്യം);
    നിർത്തുക;
  അവസ്ഥ APFloat::rmTowardPositive:
    എങ്കിൽ (isNegative())
      makeLargest(/*Neg=*/സത്യം);
    അല്ലെങ്കിൽ
      makeInf(/*Neg=*/അസത്യം);
    നിർത്തുക;
  അവസ്ഥ APFloat::rmNearestTiesToAway:
  അവസ്ഥ APFloat::rmNearestTiesToEven:
    makeInf(/*Neg=*/isNegative());
    നിർത്തുക;
  സ്ഥിരസ്ഥിതി:
    llvm_unreachable("Invalid rounding mode found");
  }
  opStatus S = opInexact;
  എങ്കിൽ (!getFirst().isFinite())
    S = static_cast<opStatus>(S | opOverflow);
  തിരികെ S;
}

APFloat::opStatus DoubleAPFloat::convertFromUnsignedParts(
    സ്ഥിരം integerPart *Src, unsigned പൂർണ്ണസംഖ്യ Src_എണ്ണം, roundingMode RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  സ്ഥിരം unsigned SrcMSB = APInt::tcMSB(Src, Src_എണ്ണം);
  എങ്കിൽ (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/അസത്യം);
    തിരികെ opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  സ്ഥിരം unsigned SrcBitWidth = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/SrcBitWidth, ArrayRef(Src, Src_എണ്ണം)},
                /*isUnsigned=*/സത്യം};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{getFirst().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/അസത്യം, rmNearestTiesToEven);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  എങ്കിൽ (!Hi.isFinite())
    തിരികെ handleOverflow(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  തർക്കം HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/സത്യം};
  Hi.convertToInteger(HiAsInt, rmNearestTiesToEven, &HiAsIntIsExact);
  സ്ഥിരം APInt Error = SrcInt.zext(HiAsInt.getBitWidth()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  roundingMode LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  എങ്കിൽ (Error.isNegative()) {
    എങ്കിൽ (RM == rmNearestTiesToAway) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      സ്ഥിരം unsigned ErrorActiveBits = Error.getSignificantBits() - 1;
      സ്ഥിരം unsigned LoPrecision = getSecond().getSemantics().precision;
      എങ്കിൽ (ErrorActiveBits > LoPrecision) {
        സ്ഥിരം unsigned RoundingBoundary = ErrorActiveBits - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        എങ്കിൽ (Error.എണ്ണം_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rmTowardPositive;
      }
    } അല്ലെങ്കിൽ എങ്കിൽ (RM == rmTowardZero) {
      // For rmTowardZero, the final positive result must be truncated (rounded
      // down). When Hi is an overestimate, Error is negative. A standard
      // rmTowardZero conversion of Error would make it *less* negative,
      // effectively rounding the final sum Hi + Lo *up*. To ensure the sum
      // rounds down correctly, we force Lo to round toward -infinity.
      LoRM = rmTowardNegative;
    }
  }

  APFloat Lo{getSecond().getSemantics()};
  opStatus Status = Lo.convertFromAPInt(Error, /*IsSigned=*/സത്യം, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fastTwoSum(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  എങ്കിൽ (!getFirst().isFinite())
    തിരികെ handleOverflow(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  എങ്കിൽ (getFirst().isFiniteNonZero() && Floats[0].isLargest()) {
    DoubleAPFloat Largest{*Semantics};
    Largest.makeLargest(/*Neg=*/അസത്യം);
    എങ്കിൽ (compare(Largest) == APFloat::cmpGreaterThan)
      തിരികെ handleOverflow(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  തിരികെ Status;
}

APFloat::opStatus DoubleAPFloat::convertFromAPInt(സ്ഥിരം APInt &നിവേശം,
                                                  തർക്കം IsSigned,
                                                  roundingMode RM) {
  സ്ഥിരം തർക്കം Negate_നിവേശം = IsSigned && നിവേശം.isNegative();
  APInt API = നിവേശം;
  എങ്കിൽ (Negate_നിവേശം)
    API.negate();

  സ്ഥിരം APFloat::opStatus Status =
      convertFromUnsignedParts(API.getRawData(), API.get_അക്കം_വാക്കുകൾ(), RM);
  എങ്കിൽ (Negate_നിവേശം)
    changeSign();
  തിരികെ Status;
}

unsigned പൂർണ്ണസംഖ്യ DoubleAPFloat::convertToHexString(അക്ഷരം *DST,
                                               unsigned പൂർണ്ണസംഖ്യ HexDigits,
                                               തർക്കം UpperCase,
                                               roundingMode RM) സ്ഥിരം {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  തിരികെ APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .convertToHexString(DST, HexDigits, UpperCase, RM);
}

തർക്കം DoubleAPFloat::isDenormal() സ്ഥിരം {
  തിരികെ getCategory() == fcNormal &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

തർക്കം DoubleAPFloat::isSmallest() സ്ഥിരം {
  എങ്കിൽ (getCategory() != fcNormal)
    തിരികെ അസത്യം;
  DoubleAPFloat Tmp(*ഇത്);
  Tmp.makeSmallest(ഇത്->isNegative());
  തിരികെ Tmp.compare(*ഇത്) == cmpEqual;
}

തർക്കം DoubleAPFloat::isSmallestNormalized() സ്ഥിരം {
  എങ്കിൽ (getCategory() != fcNormal)
    തിരികെ അസത്യം;

  DoubleAPFloat Tmp(*ഇത്);
  Tmp.makeSmallestNormalized(ഇത്->isNegative());
  തിരികെ Tmp.compare(*ഇത്) == cmpEqual;
}

തർക്കം DoubleAPFloat::isLargest() സ്ഥിരം {
  എങ്കിൽ (getCategory() != fcNormal)
    തിരികെ അസത്യം;
  DoubleAPFloat Tmp(*ഇത്);
  Tmp.makeLargest(ഇത്->isNegative());
  തിരികെ Tmp.compare(*ഇത്) == cmpEqual;
}

തർക്കം DoubleAPFloat::isInteger() സ്ഥിരം {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  തിരികെ Floats[0].isInteger() && Floats[1].isInteger();
}

ശൂന്യം DoubleAPFloat::toString(SmallVectorImpl<അക്ഷരം> &Str,
                             unsigned FormatPrecision,
                             unsigned FormatMaxPadding,
                             തർക്കം TruncateZero) സ്ഥിരം {
  പ്രതിജ്ഞാ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .toString(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

പൂർണ്ണസംഖ്യ DoubleAPFloat::getExactLog2Abs() സ്ഥിരം {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  എങ്കിൽ (getSecond().isNonZero())
    തിരികെ INT_MIN;
  തിരികെ getFirst().getExactLog2Abs();
}

പൂർണ്ണസംഖ്യ ilogb(സ്ഥിരം DoubleAPFloat &Arg) {
  സ്ഥിരം APFloat &Hi = Arg.getFirst();
  സ്ഥിരം APFloat &Lo = Arg.getSecond();
  പൂർണ്ണസംഖ്യ IlogbResult = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  എങ്കിൽ (Arg.getCategory() != fcNormal)
    തിരികെ IlogbResult;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  എങ്കിൽ (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    തിരികെ IlogbResult;
  എങ്കിൽ (Hi.getExactLog2Abs() == INT_MIN)
    തിരികെ IlogbResult;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  തിരികെ IlogbResult - 1;
}

DoubleAPFloat scalbn(സ്ഥിരം DoubleAPFloat &Arg, പൂർണ്ണസംഖ്യ Exp,
                     APFloat::roundingMode RM) {
  പ്രതിജ്ഞാ(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");
  തിരികെ DoubleAPFloat(APFloatBase::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(സ്ഥിരം DoubleAPFloat &Arg, പൂർണ്ണസംഖ്യ &Exp,
                    APFloat::roundingMode RM) {
  പ്രതിജ്ഞാ(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  എങ്കിൽ (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.getFirst() = Quiet.getFirst().makeQuiet();
    തിരികെ Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  എങ്കിൽ (Exp == APFloat::IEK_Inf)
    തിരികെ Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  എങ്കിൽ (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    തിരികെ Arg;
  }

  സ്ഥിരം APFloat &Hi = Arg.getFirst();
  സ്ഥിരം APFloat &Lo = Arg.getSecond();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  സ്ഥിരം തർക്കം SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat Second = Lo;
  എങ്കിൽ (Arg.getCategory() == APFloat::fcNormal && Lo.isFiniteNonZero()) {
    roundingMode LoRoundingMode;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    എങ്കിൽ (RM == rmTowardZero)
      LoRoundingMode = Arg.isNegative() ? rmTowardPositive : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    അല്ലെങ്കിൽ എങ്കിൽ (RM == rmNearestTiesToAway && SignsDisagree && Exp > 0)
      LoRoundingMode = rmNearestTiesToEven;
    അല്ലെങ്കിൽ
      LoRoundingMode = RM;
    Second = scalbn(Lo, -Exp, LoRoundingMode);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    എങ്കിൽ (RM == rmNearestTiesToAway && LoRoundingMode == rmNearestTiesToEven) {
      // Re-scale the result back to check if rounding occurred.
      സ്ഥിരം APFloat RecomposedLo = scalbn(Second, Exp, rmNearestTiesToEven);
      എങ്കിൽ (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        സ്ഥിരം APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        സ്ഥിരം APFloat UlpOfSecond = harrisonUlp(Second);
        സ്ഥിരം APFloat ScaledUlpOfSecond =
            scalbn(UlpOfSecond, Exp - 1, rmNearestTiesToEven);
        സ്ഥിരം തർക്കം IsMidpoint = abs(RoundingError) == ScaledUlpOfSecond;
        സ്ഥിരം തർക്കം RoundedLoAway =
            Second.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        എങ്കിൽ (IsMidpoint && RoundedLoAway)
          Second.next(/*nextDown=*/!Second.isNegative());
      }
    }
    // Handle a tricky edge case where Arg is slightly less than a power of two
    // (e.g., Arg = 2^k - epsilon). In this situation:
    // 1. Hi is 2^k, and Lo is a small negative value -epsilon.
    // 2. ilogb(Arg) correctly returns k-1.
    // 3. Our initial Exp becomes (k-1) + 1 = k.
    // 4. Scaling Hi (2^k) by 2^-k would yield a magnitude of 1.0 and
    //    scaling Lo by 2^-k would yield zero. This would make the result 1.0
    //    which is an invalid fraction, as the required interval is [0.5, 1.0).
    // We detect this specific case by checking if Hi is a power of two and if
    // the scaled Lo underflowed to zero. The fix: Increment Exp to k+1. This
    // adjusts the scale factor, causing Hi to be scaled to 0.5, which is a
    // valid fraction.
    എങ്കിൽ (Second.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat First = scalbn(Hi, -Exp, RM);
  തിരികെ DoubleAPFloat(APFloatBase::PPCDoubleDouble(), std::move(First),
                       std::move(Second));
}

APInt DoubleAPFloat::getNaNPayload() സ്ഥിരം { തിരികെ Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, സ്ഥിരം fltSemantics &Semantics) {
  എങ്കിൽ (usesLayout<IEEEFloat>(Semantics)) {
    പുതിയ (&IEEE) IEEEFloat(std::move(F));
    തിരികെ;
  }
  എങ്കിൽ (usesLayout<DoubleAPFloat>(Semantics)) {
    സ്ഥിരം fltSemantics& S = F.getSemantics();
    പുതിയ (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloatBase::IEEEdouble()));
    തിരികെ;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::opStatus> APFloat::convertFromString(StringRef Str,
                                                       roundingMode RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_code hash_മൂല്യം(സ്ഥിരം APFloat &Arg) {
  എങ്കിൽ (APFloat::usesLayout<detail::IEEEFloat>(Arg.getSemantics()))
    തിരികെ hash_മൂല്യം(Arg.U.IEEE);
  എങ്കിൽ (APFloat::usesLayout<detail::DoubleAPFloat>(Arg.getSemantics()))
    തിരികെ hash_മൂല്യം(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(സ്ഥിരം fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  auto StatusOrErr = convertFromString(S, rmNearestTiesToEven);
  പ്രതിജ്ഞാ(StatusOrErr && "Invalid floating point representation");
  consumeError(StatusOrErr.takeError());
}

FPClassTest APFloat::classify() സ്ഥിരം {
  എങ്കിൽ (isZero())
    തിരികെ isNegative() ? fcNegZero : fcPosZero;
  എങ്കിൽ (isNormal())
    തിരികെ isNegative() ? fcNegNormal : fcPosNormal;
  എങ്കിൽ (isDenormal())
    തിരികെ isNegative() ? fcNegSubnormal : fcPosSubnormal;
  എങ്കിൽ (isInfinity())
    തിരികെ isNegative() ? fcNegInf : fcPosInf;
  പ്രതിജ്ഞാ(isNaN() && "Other class of FP constant");
  തിരികെ isSignaling() ? fcSNan : fcQNan;
}

തർക്കം APFloat::getExactInverse(APFloat *Inv) സ്ഥിരം {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  എങ്കിൽ (!isFiniteNonZero())
    തിരികെ അസത്യം;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  എങ്കിൽ (isDenormal())
    തിരികെ അസത്യം;

  // A number has an exact, representable inverse if and only if it is a power
  // of two.
  //
  // Mathematical Rationale:
  // 1. A binary floating-point number x is a dyadic rational, meaning it can
  //    be written as x = M / 2^k for integers M (the significand) and k.
  // 2. The inverse is 1/x = 2^k / M.
  // 3. For 1/x to also be a dyadic rational (and thus exactly representable
  //    in binary), its denominator M must also be a power of two.
  //    Let's say M = 2^m.
  // 4. Substituting this back into the formula for x, we get
  //    x = (2^m) / (2^k) = 2^(m-k).
  //
  // This proves that x must be a power of two.

  // getExactLog2Abs() returns the integer exponent if the number is a power of
  // two or INT_MIN if it is not.
  സ്ഥിരം പൂർണ്ണസംഖ്യ Exp = getExactLog2Abs();
  എങ്കിൽ (Exp == INT_MIN)
    തിരികെ അസത്യം;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  എങ്കിൽ (Reciprocal.getExactLog2Abs() != -Exp)
    തിരികെ അസത്യം;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  എങ്കിൽ (Reciprocal.isDenormal())
    തിരികെ അസത്യം;

  പ്രതിജ്ഞാ(Reciprocal.isFiniteNonZero());

  എങ്കിൽ (Inv)
    *Inv = std::move(Reciprocal);

  തിരികെ സത്യം;
}

APFloat::opStatus APFloat::convert(സ്ഥിരം fltSemantics &ToSemantics,
                                   roundingMode RM, തർക്കം *losesInfo) {
  എങ്കിൽ (&getSemantics() == &ToSemantics) {
    *losesInfo = അസത്യം;
    തിരികെ opOK;
  }
  എങ്കിൽ (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics))
    തിരികെ U.IEEE.convert(ToSemantics, RM, losesInfo);
  എങ്കിൽ (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<DoubleAPFloat>(ToSemantics)) {
    പ്രതിജ്ഞാ(&ToSemantics == &APFloatBase::semPPCDoubleDouble);
    auto Ret =
        U.IEEE.convert(APFloatBase::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *ഇത് = APFloat(ToSemantics, U.IEEE.bitcastToAPInt());
    തിരികെ Ret;
  }
  എങ്കിൽ (usesLayout<DoubleAPFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics)) {
    auto Ret = getIEEE().convert(ToSemantics, RM, losesInfo);
    *ഇത് = APFloat(std::move(getIEEE()), ToSemantics);
    തിരികെ Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_മൂല്യം(സ്ഥിരം fltSemantics &Semantics) {
  തിരികെ APFloat(Semantics, APInt::getAllOnes(Semantics.sizeInBits));
}

ശൂന്യം APFloat::print(raw_ostream &OS) സ്ഥിരം {
  SmallVector<അക്ഷരം, 16> Buffer;
  toString(Buffer);
  OS << Buffer;
}

#എങ്കിൽ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ശൂന്യം APFloat::dump() സ്ഥിരം {
  print(dbgs());
  dbgs() << '\n';
}
#endif

ശൂന്യം APFloat::Profile(Folding_Set_കണ്ണി_ID &NID) സ്ഥിരം {
  NID.Add(bitcastToAPInt());
}

APFloat::opStatus APFloat::convertToInteger(APSInt &result,
                                            roundingMode rounding_mode,
                                            തർക്കം *isExact) സ്ഥിരം {
  unsigned bitWidth = result.getBitWidth();
  SmallVector<uint64_t, 4> parts(result.get_അക്കം_വാക്കുകൾ());
  opStatus status = convertToInteger(parts, bitWidth, result.isSigned(),
                                     rounding_mode, isExact);
  // Keeps the original signed-ness.
  result = APInt(bitWidth, parts);
  തിരികെ status;
}

ഇരട്ട APFloat::convertToDouble() സ്ഥിരം {
  എങ്കിൽ (&getSemantics() == &APFloatBase::semIEEEdouble)
    തിരികെ getIEEE().convertToDouble();
  പ്രതിജ്ഞാ(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat Temp = *ഇത്;
  തർക്കം LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &LosesInfo);
  പ്രതിജ്ഞാ(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  തിരികെ Temp.getIEEE().convertToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 APFloat::convertToQuad() സ്ഥിരം {
  എങ്കിൽ (&getSemantics() == &APFloatBase::semIEEEquad)
    തിരികെ getIEEE().convertToQuad();
  പ്രതിജ്ഞാ(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat Temp = *ഇത്;
  തർക്കം LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEquad, rmNearestTiesToEven, &LosesInfo);
  പ്രതിജ്ഞാ(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  തിരികെ Temp.getIEEE().convertToQuad();
}
#endif

ദശാംശം APFloat::convertToFloat() സ്ഥിരം {
  എങ്കിൽ (&getSemantics() == &APFloatBase::semIEEEsingle)
    തിരികെ getIEEE().convertToFloat();
  പ്രതിജ്ഞാ(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat Temp = *ഇത്;
  തർക്കം LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEsingle, rmNearestTiesToEven, &LosesInfo);
  പ്രതിജ്ഞാ(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  തിരികെ Temp.getIEEE().convertToFloat();
}

unsigned APFloatBase::getArbitraryFPFormatSizeInBits(StringRef Format) {
  തിരികെ StringSwitch<unsigned>(Format)
      .Case("Float8E5M2", getSizeInBits(semFloat8E5M2))
      .Case("Float8E5M2FNUZ", getSizeInBits(semFloat8E5M2FNUZ))
      .Case("Float8E4M3", getSizeInBits(semFloat8E4M3))
      .Case("Float8E4M3FN", getSizeInBits(semFloat8E4M3FN))
      .Case("Float8E4M3FNUZ", getSizeInBits(semFloat8E4M3FNUZ))
      .Case("Float8E4M3B11FNUZ", getSizeInBits(semFloat8E4M3B11FNUZ))
      .Case("Float8E3M4", getSizeInBits(semFloat8E3M4))
      .Case("Float8E8M0FNU", getSizeInBits(semFloat8E8M0FNU))
      .Case("Float6E3M2FN", getSizeInBits(semFloat6E3M2FN))
      .Case("Float6E2M3FN", getSizeInBits(semFloat6E2M3FN))
      .Case("Float4E2M1FN", getSizeInBits(semFloat4E2M1FN))
      .Case("Float8E5M3FNU", getSizeInBits(semFloat8E5M3FNU))
      .Default(0);
}

തർക്കം APFloatBase::isValidArbitraryFPFormat(StringRef Format) {
  തിരികെ getArbitraryFPFormatSizeInBits(Format) != 0;
}

സ്ഥിരം fltSemantics *APFloatBase::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  തിരികെ StringSwitch<സ്ഥിരം fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(nullptr);
}

APFloat::Storage::~Storage() {
  എങ്കിൽ (usesLayout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    തിരികെ;
  }
  എങ്കിൽ (usesLayout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    തിരികെ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(സ്ഥിരം APFloat::Storage &RHS) {
  എങ്കിൽ (usesLayout<IEEEFloat>(*RHS.semantics)) {
    പുതിയ (ഇത്) IEEEFloat(RHS.IEEE);
    തിരികെ;
  }
  എങ്കിൽ (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    പുതിയ (ഇത്) DoubleAPFloat(RHS.Double);
    തിരികെ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  എങ്കിൽ (usesLayout<IEEEFloat>(*RHS.semantics)) {
    പുതിയ (ഇത്) IEEEFloat(std::move(RHS.IEEE));
    തിരികെ;
  }
  എങ്കിൽ (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    പുതിയ (ഇത്) DoubleAPFloat(std::move(RHS.Double));
    തിരികെ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::operator=(സ്ഥിരം APFloat::Storage &RHS) {
  എങ്കിൽ (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } അല്ലെങ്കിൽ എങ്കിൽ (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } അല്ലെങ്കിൽ എങ്കിൽ (ഇത് != &RHS) {
    ഇത്->~Storage();
    പുതിയ (ഇത്) Storage(RHS);
  }
  തിരികെ *ഇത്;
}

APFloat::Storage &APFloat::Storage::operator=(APFloat::Storage &&RHS) {
  എങ്കിൽ (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } അല്ലെങ്കിൽ എങ്കിൽ (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } അല്ലെങ്കിൽ എങ്കിൽ (ഇത് != &RHS) {
    ഇത്->~Storage();
    പുതിയ (ഇത്) Storage(std::move(RHS));
  }
  തിരികെ *ഇത്;
}

നാമാകാശ {

APFloat::opStatus getOpStatusFromLibc(പൂർണ്ണസംഖ്യ libc_exceptions) {
  APFloat::opStatus status = APFloat::opOK;
  എങ്കിൽ (libc_exceptions & FE_INVALID)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInvalidOp);
  എങ്കിൽ (libc_exceptions & FE_DIVBYZERO)
    status = static_cast<APFloat::opStatus>(status | APFloat::opDivByZero);
  എങ്കിൽ (libc_exceptions & FE_OVERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opOverflow);
  എങ്കിൽ (libc_exceptions & FE_UNDERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opUnderflow);
  എങ്കിൽ (libc_exceptions & FE_INEXACT)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInexact);
  തിരികെ status;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> exp(സ്ഥിരം APFloat &x, RoundingMode rounding_mode,
                           APFloat::opStatus *status) {

  എങ്കിൽ (rounding_mode == APFloatBase::rmNearestTiesToEven) {
    എങ്കിൽ (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEsingle) {
      ദശാംശം x_val = x.convertToFloat();
      പൂർണ്ണസംഖ്യ exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      എങ്കിൽ (status) {
        *status = getOpStatusFromLibc(exc);
        എങ്കിൽ (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      ദശാംശം result = LIBC_NAMESPACE::shared::expf(x_val);
      തിരികെ APFloat(result);
    }
    എങ്കിൽ (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEdouble) {
      ഇരട്ട x_val = x.convertToDouble();
      പൂർണ്ണസംഖ്യ exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      എങ്കിൽ (status) {
        *status = getOpStatusFromLibc(exc);
        എങ്കിൽ (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      ഇരട്ട result = LIBC_NAMESPACE::shared::exp(x_val);
      തിരികെ APFloat(result);
    }
  }
  തിരികെ std::nullopt;
}

} // namespace llvm

#undef APFLOAT_DISPATCH_ON_SEMANTICS
