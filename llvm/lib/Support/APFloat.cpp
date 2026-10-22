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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/APFloat.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/APSInt.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ArrayRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/FloatingPointMode.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/FoldingSet.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Hashing.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/STLExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringSwitch.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MathExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <cstring>
#ಅನ್ತರ್ಭಾವ <iterator>
#ಅನ್ತರ್ಭಾವ <limits.h>

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

#ಅನ್ತರ್ಭಾವ "shared/math.h"
#ಅನ್ತರ್ಭಾವ "shared/math_check_exceptions.h"

#define APFLOAT_DISPATCH_ON_SEMANTICS(METHOD_CALL)                             \
  ಕುರು {                                                                         \
    ಯದಿ (usesLayout<IEEEFloat>(getSemantics()))                                 \
      ಪ್ರತ್ಯಾಗಮ U.IEEE.METHOD_CALL;                                               \
    ಯದಿ (usesLayout<DoubleAPFloat>(getSemantics()))                             \
      ಪ್ರತ್ಯಾಗಮ U.Double.METHOD_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } ಯಾವತ್ (ಅಸತ್ಯ)

using ನಾಮಾಕಾಶ llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#define Pack_Categories_Into_ಕೀಲಿ(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloatBase::integerPartWidth % 4 == 0, "Part width must be divisible by 4!");

ನಾಮಾಕಾಶ llvm {

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
    ಸತ್ಯ,
    ಸತ್ಯ,
    ಸತ್ಯ,
    ಸತ್ಯ,
    ಸತ್ಯ};
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
    ಅಸತ್ಯ,
    ಅಸತ್ಯ,
    ಅಸತ್ಯ,
    ಅಸತ್ಯ};

constexpr fltSemantics APFloatBase::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    ಸತ್ಯ,
    ಅಸತ್ಯ,
    ಅಸತ್ಯ};

constexpr fltSemantics APFloatBase::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semBogus = {0, 0, 0, 0};

ಧ್ರುವ llvm::fltSemantics &APFloatBase::EnumToSemantics(Semantics S) {
  ವಿಕಲ್ಪ (S) {
  ಪಕ್ಷ S_IEEEhalf:
    ಪ್ರತ್ಯಾಗಮ IEEEhalf();
  ಪಕ್ಷ S_BFloat:
    ಪ್ರತ್ಯಾಗಮ BFloat();
  ಪಕ್ಷ S_IEEEsingle:
    ಪ್ರತ್ಯಾಗಮ IEEEsingle();
  ಪಕ್ಷ S_IEEEdouble:
    ಪ್ರತ್ಯಾಗಮ IEEEdouble();
  ಪಕ್ಷ S_IEEEquad:
    ಪ್ರತ್ಯಾಗಮ IEEEquad();
  ಪಕ್ಷ S_x87DoubleExtended:
    ಪ್ರತ್ಯಾಗಮ x87DoubleExtended();
  ಪಕ್ಷ S_PPCDoubleDouble:
    ಪ್ರತ್ಯಾಗಮ PPCDoubleDouble();
  ಪಕ್ಷ S_PPCDoubleDoubleLegacy:
    ಪ್ರತ್ಯಾಗಮ PPCDoubleDoubleLegacy();
  ಪಕ್ಷ S_Float8E5M2:
    ಪ್ರತ್ಯಾಗಮ Float8E5M2();
  ಪಕ್ಷ S_Float8E5M2FNUZ:
    ಪ್ರತ್ಯಾಗಮ Float8E5M2FNUZ();
  ಪಕ್ಷ S_Float8E4M3:
    ಪ್ರತ್ಯಾಗಮ Float8E4M3();
  ಪಕ್ಷ S_Float8E4M3FN:
    ಪ್ರತ್ಯಾಗಮ Float8E4M3FN();
  ಪಕ್ಷ S_Float8E4M3FNUZ:
    ಪ್ರತ್ಯಾಗಮ Float8E4M3FNUZ();
  ಪಕ್ಷ S_Float8E4M3B11FNUZ:
    ಪ್ರತ್ಯಾಗಮ Float8E4M3B11FNUZ();
  ಪಕ್ಷ S_Float8E3M4:
    ಪ್ರತ್ಯಾಗಮ Float8E3M4();
  ಪಕ್ಷ S_FloatTF32:
    ಪ್ರತ್ಯಾಗಮ FloatTF32();
  ಪಕ್ಷ S_Float8E8M0FNU:
    ಪ್ರತ್ಯಾಗಮ Float8E8M0FNU();
  ಪಕ್ಷ S_Float8E5M3FNU:
    ಪ್ರತ್ಯಾಗಮ Float8E5M3FNU();
  ಪಕ್ಷ S_Float6E3M2FN:
    ಪ್ರತ್ಯಾಗಮ Float6E3M2FN();
  ಪಕ್ಷ S_Float6E2M3FN:
    ಪ್ರತ್ಯಾಗಮ Float6E2M3FN();
  ಪಕ್ಷ S_Float4E2M1FN:
    ಪ್ರತ್ಯಾಗಮ Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloatBase::Semantics
APFloatBase::SemanticsToEnum(ಧ್ರುವ llvm::fltSemantics &Sem) {
  ಯದಿ (&Sem == &llvm::APFloat::IEEEhalf())
    ಪ್ರತ್ಯಾಗಮ S_IEEEhalf;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::BFloat())
    ಪ್ರತ್ಯಾಗಮ S_BFloat;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::IEEEsingle())
    ಪ್ರತ್ಯಾಗಮ S_IEEEsingle;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::IEEEdouble())
    ಪ್ರತ್ಯಾಗಮ S_IEEEdouble;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::IEEEquad())
    ಪ್ರತ್ಯಾಗಮ S_IEEEquad;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::x87DoubleExtended())
    ಪ್ರತ್ಯಾಗಮ S_x87DoubleExtended;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::PPCDoubleDouble())
    ಪ್ರತ್ಯಾಗಮ S_PPCDoubleDouble;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    ಪ್ರತ್ಯಾಗಮ S_PPCDoubleDoubleLegacy;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::Float8E5M2())
    ಪ್ರತ್ಯಾಗಮ S_Float8E5M2;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    ಪ್ರತ್ಯಾಗಮ S_Float8E5M2FNUZ;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::Float8E4M3())
    ಪ್ರತ್ಯಾಗಮ S_Float8E4M3;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::Float8E4M3FN())
    ಪ್ರತ್ಯಾಗಮ S_Float8E4M3FN;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    ಪ್ರತ್ಯಾಗಮ S_Float8E4M3FNUZ;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    ಪ್ರತ್ಯಾಗಮ S_Float8E4M3B11FNUZ;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::Float8E3M4())
    ಪ್ರತ್ಯಾಗಮ S_Float8E3M4;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::FloatTF32())
    ಪ್ರತ್ಯಾಗಮ S_FloatTF32;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::Float8E8M0FNU())
    ಪ್ರತ್ಯಾಗಮ S_Float8E8M0FNU;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::Float8E5M3FNU())
    ಪ್ರತ್ಯಾಗಮ S_Float8E5M3FNU;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::Float6E3M2FN())
    ಪ್ರತ್ಯಾಗಮ S_Float6E3M2FN;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::Float6E2M3FN())
    ಪ್ರತ್ಯಾಗಮ S_Float6E2M3FN;
  ಅನ್ಯಥಾ ಯದಿ (&Sem == &llvm::APFloat::Float4E2M1FN())
    ಪ್ರತ್ಯಾಗಮ S_Float4E2M1FN;
  ಅನ್ಯಥಾ
    llvm_unreachable("Unknown floating semantics");
}

ತಾರ್ಕಿಕ APFloatBase::isRepresentableBy(ಧ್ರುವ fltSemantics &A,
                                    ಧ್ರುವ fltSemantics &B) {
  ಪ್ರತ್ಯಾಗಮ A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

ತಾರ್ಕಿಕ APFloatBase::isLosslesslyConvertibleTo(ಧ್ರುವ fltSemantics &From,
                                            ಧ್ರುವ fltSemantics &InitialTo,
                                            ತಾರ್ಕಿಕ IgnoreNaNs) {
  ಯದಿ (&From == &InitialTo)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  ಯದಿ (&From == &semPPCDoubleDouble)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  ಧ್ರುವ fltSemantics &To =
      &InitialTo == &semPPCDoubleDouble ? semIEEEdouble : InitialTo;

  ಯದಿ (!isRepresentableBy(From, To))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಯದಿ ((From.hasZero && !To.hasZero) ||
      (From.hasSignedRepr && !To.hasSignedRepr))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  ತಾರ್ಕಿಕ FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  ತಾರ್ಕಿಕ ToHasSignedZero = To.hasZero && To.hasSignedRepr &&
                         To.nanEncoding != fltNanEncoding::NegativeZero;
  ಯದಿ (FromHasSignedZero && !ToHasSignedZero)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat SmallestFrom = APFloat::getSmallest(From);
  ತಾರ್ಕಿಕ LosesInfo = ಅಸತ್ಯ;
  (ಶೂನ್ಯ)SmallestFrom.convert(To, APFloat::rmNearestTiesToEven, &LosesInfo);
  ಯದಿ (LosesInfo)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಯದಿ (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  ಯದಿ (IgnoreNaNs) {
    ಯದಿ (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      ಪ್ರತ್ಯಾಗಮ To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    ಪ್ರತ್ಯಾಗಮ To.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  ಯದಿ (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  ಯದಿ (To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ To.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
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
ಸ್ಥಿರ constexpr unsigned ಅಂಕೆ_Pow5_Parts_On_ರಾಶಿ = 16;

ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ powerOfFivePartsForExponent(unsigned ಪೂರ್ಣಾಙ್ಕ power) {
  ಪ್ರತಿಜ್ಞಾ(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  ಪ್ರತ್ಯಾಗಮ 2 + ((power * 815) / (351 * APFloatBase::integerPartWidth));
}

unsigned ಪೂರ್ಣಾಙ್ಕ APFloatBase::semanticsPrecision(ಧ್ರುವ fltSemantics &semantics) {
  ಪ್ರತ್ಯಾಗಮ semantics.precision;
}
APFloatBase::ExponentType
APFloatBase::semanticsMaxExponent(ಧ್ರುವ fltSemantics &semantics) {
  ಪ್ರತ್ಯಾಗಮ semantics.maxExponent;
}
APFloatBase::ExponentType
APFloatBase::semanticsMinExponent(ಧ್ರುವ fltSemantics &semantics) {
  ಪ್ರತ್ಯಾಗಮ semantics.minExponent;
}
unsigned ಪೂರ್ಣಾಙ್ಕ APFloatBase::semanticsSizeInBits(ಧ್ರುವ fltSemantics &semantics) {
  ಪ್ರತ್ಯಾಗಮ semantics.sizeInBits;
}
unsigned ಪೂರ್ಣಾಙ್ಕ APFloatBase::semanticsIntSizeInBits(ಧ್ರುವ fltSemantics &semantics,
                                                 ತಾರ್ಕಿಕ isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  unsigned ಪೂರ್ಣಾಙ್ಕ MinBitWidth = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  ಯದಿ (isSigned)
    ++MinBitWidth;
  ಪ್ರತ್ಯಾಗಮ MinBitWidth;
}

ತಾರ್ಕಿಕ APFloatBase::semanticsHasZero(ಧ್ರುವ fltSemantics &semantics) {
  ಪ್ರತ್ಯಾಗಮ semantics.hasZero;
}

ತಾರ್ಕಿಕ APFloatBase::semanticsHasSignedRepr(ಧ್ರುವ fltSemantics &semantics) {
  ಪ್ರತ್ಯಾಗಮ semantics.hasSignedRepr;
}

ತಾರ್ಕಿಕ APFloatBase::semanticsHasInf(ಧ್ರುವ fltSemantics &semantics) {
  ಪ್ರತ್ಯಾಗಮ semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

ತಾರ್ಕಿಕ APFloatBase::semanticsHasNaN(ಧ್ರುವ fltSemantics &semantics) {
  ಪ್ರತ್ಯಾಗಮ semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

ತಾರ್ಕಿಕ APFloatBase::isIEEELikeFP(ಧ್ರುವ fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  ಪ್ರತ್ಯಾಗಮ SemanticsToEnum(semantics) <= S_IEEEquad;
}

ತಾರ್ಕಿಕ APFloatBase::hasSignBitInMSB(ಧ್ರುವ fltSemantics &semantics) {
  ಪ್ರತ್ಯಾಗಮ semantics.hasSignBitInMSB;
}

ತಾರ್ಕಿಕ APFloatBase::isRepresentableAsNormalIn(ಧ್ರುವ fltSemantics &Src,
                                            ಧ್ರುವ fltSemantics &Dst) {
  // Exponent range must be larger.
  ಯದಿ (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  ಪ್ರತ್ಯಾಗಮ Dst.precision >= Src.precision;
}

unsigned APFloatBase::getSizeInBits(ಧ್ರುವ fltSemantics &Sem) {
  ಪ್ರತ್ಯಾಗಮ Sem.sizeInBits;
}

ಸ್ಥಿರ constexpr APFloatBase::ExponentType
exponentZero(ಧ್ರುವ fltSemantics &semantics) {
  ಪ್ರತ್ಯಾಗಮ semantics.minExponent - 1;
}

ಸ್ಥಿರ constexpr APFloatBase::ExponentType
exponentInf(ಧ್ರುವ fltSemantics &semantics) {
  ಪ್ರತ್ಯಾಗಮ semantics.maxExponent + 1;
}

ಸ್ಥಿರ constexpr APFloatBase::ExponentType
exponentNaN(ಧ್ರುವ fltSemantics &semantics) {
  ಯದಿ (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    ಯದಿ (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      ಪ್ರತ್ಯಾಗಮ exponentZero(semantics);
    ಯದಿ (semantics.hasSignedRepr || semantics.precision > 1)
      ಪ್ರತ್ಯಾಗಮ semantics.maxExponent;
  }
  ಪ್ರತ್ಯಾಗಮ semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ Error createError(ಧ್ರುವ Twine &Err) {
  ಪ್ರತ್ಯಾಗಮ make_error<StringError>(Err, inconvertibleErrorCode());
}

ಸ್ಥಿರ constexpr ಅನ್ತಃಸ್ಥ unsigned ಪೂರ್ಣಾಙ್ಕ part_ಎಣಿಕೆ_For_Bits(unsigned ಪೂರ್ಣಾಙ್ಕ bits) {
  ಪ್ರತ್ಯಾಗಮ std::max(1u, (bits + APFloatBase::integerPartWidth - 1) /
                          APFloatBase::integerPartWidth);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ unsigned ಪೂರ್ಣಾಙ್ಕ
dec_Digit_ಮೌಲ್ಯ(unsigned ಪೂರ್ಣಾಙ್ಕ c)
{
  ಪ್ರತ್ಯಾಗಮ c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
ಸ್ಥಿರ Expected<ಪೂರ್ಣಾಙ್ಕ> readExponent(StringRef::iterator begin,
                                  StringRef::iterator end) {
  ಧ್ರುವ unsigned ಪೂರ್ಣಾಙ್ಕ overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  ಯದಿ (p == end || ((*p == '-' || *p == '+') && (p + 1) == end))
    ಪ್ರತ್ಯಾಗಮ 0;

  ತಾರ್ಕಿಕ isNegative = *p == '-';
  ಯದಿ (*p == '-' || *p == '+') {
    p++;
    ಯದಿ (p == end)
      ಪ್ರತ್ಯಾಗಮ createError("Exponent has no digits");
  }

  unsigned absExponent = dec_Digit_ಮೌಲ್ಯ(*p++);
  ಯದಿ (absExponent >= 10U)
    ಪ್ರತ್ಯಾಗಮ createError("Invalid character in exponent");

  ಪ್ರತಿ (; p != end; ++p) {
    unsigned ಮೌಲ್ಯ_2 = dec_Digit_ಮೌಲ್ಯ(*p);
    ಯದಿ (ಮೌಲ್ಯ_2 >= 10U)
      ಪ್ರತ್ಯಾಗಮ createError("Invalid character in exponent");

    absExponent = absExponent * 10U + ಮೌಲ್ಯ_2;
    ಯದಿ (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      ಭಙ್ಗ;
    }
  }

  ಯದಿ (isNegative)
    ಪ್ರತ್ಯಾಗಮ -(ಪೂರ್ಣಾಙ್ಕ) absExponent;
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
ಸ್ಥಿರ Expected<ಪೂರ್ಣಾಙ್ಕ> totalExponent(StringRef::iterator p,
                                   StringRef::iterator end,
                                   ಪೂರ್ಣಾಙ್ಕ exponentAdjustment) {
  ಪೂರ್ಣಾಙ್ಕ exponent = 0;

  ಯದಿ (p == end)
    ಪ್ರತ್ಯಾಗಮ createError("Exponent has no digits");

  ತಾರ್ಕಿಕ negative = *p == '-';
  ಯದಿ (*p == '-' || *p == '+') {
    p++;
    ಯದಿ (p == end)
      ಪ್ರತ್ಯಾಗಮ createError("Exponent has no digits");
  }

  ಪೂರ್ಣಾಙ್ಕ unsignedExponent = 0;
  ತಾರ್ಕಿಕ overflow = ಅಸತ್ಯ;
  ಪ್ರತಿ (; p != end; ++p) {
    unsigned ಪೂರ್ಣಾಙ್ಕ ಮೌಲ್ಯ_2;

    ಮೌಲ್ಯ_2 = dec_Digit_ಮೌಲ್ಯ(*p);
    ಯದಿ (ಮೌಲ್ಯ_2 >= 10U)
      ಪ್ರತ್ಯಾಗಮ createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + ಮೌಲ್ಯ_2;
    ಯದಿ (unsignedExponent > 32767) {
      overflow = ಸತ್ಯ;
      ಭಙ್ಗ;
    }
  }

  ಯದಿ (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    overflow = ಸತ್ಯ;

  ಯದಿ (!overflow) {
    exponent = unsignedExponent;
    ಯದಿ (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    ಯದಿ (exponent > 32767 || exponent < -32768)
      overflow = ಸತ್ಯ;
  }

  ಯದಿ (overflow)
    exponent = negative ? -32768: 32767;

  ಪ್ರತ್ಯಾಗಮ exponent;
}

ಸ್ಥಿರ Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator end,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = end;
  ಯಾವತ್ (p != end && *p == '0')
    p++;

  ಯದಿ (p != end && *p == '.') {
    *dot = p++;

    ಯದಿ (end - begin == 1)
      ಪ್ರತ್ಯಾಗಮ createError("Significand has no digits");

    ಯಾವತ್ (p != end && *p == '0')
      p++;
  }

  ಪ್ರತ್ಯಾಗಮ p;
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
ಸಂರಚನಾ decimalInfo {
  ಧ್ರುವ ವರ್ಣ *firstSigDigit;
  ಧ್ರುವ ವರ್ಣ *lastSigDigit;
  ಪೂರ್ಣಾಙ್ಕ exponent;
  ಪೂರ್ಣಾಙ್ಕ normalizedExponent;
};

ಸ್ಥಿರ Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator end, decimalInfo *D) {
  StringRef::iterator dot = end;

  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  ಯದಿ (!PtrOrErr)
    ಪ್ರತ್ಯಾಗಮ PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->firstSigDigit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  ಪ್ರತಿ (; p != end; ++p) {
    ಯದಿ (*p == '.') {
      ಯದಿ (dot != end)
        ಪ್ರತ್ಯಾಗಮ createError("String contains multiple dots");
      dot = p++;
      ಯದಿ (p == end)
        ಭಙ್ಗ;
    }
    ಯದಿ (dec_Digit_ಮೌಲ್ಯ(*p) >= 10U)
      ಭಙ್ಗ;
  }

  ಯದಿ (p != end) {
    ಯದಿ (*p != 'e' && *p != 'E')
      ಪ್ರತ್ಯಾಗಮ createError("Invalid character in significand");
    ಯದಿ (p == begin)
      ಪ್ರತ್ಯಾಗಮ createError("Significand has no digits");
    ಯದಿ (dot != end && p - begin == 1)
      ಪ್ರತ್ಯಾಗಮ createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    auto ExpOrErr = readExponent(p + 1, end);
    ಯದಿ (!ExpOrErr)
      ಪ್ರತ್ಯಾಗಮ ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    ಯದಿ (dot == end)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  ಯದಿ (p != D->firstSigDigit) {
    /* Drop insignificant trailing zeroes.  */
    ಯದಿ (p != begin) {
      ಕುರು
        ಕುರು
          p--;
        ಯಾವತ್ (p != begin && *p == '0');
      ಯಾವತ್ (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->firstSigDigit)
                                      - (dot > D->firstSigDigit && dot < p)));
  }

  D->lastSigDigit = p;
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
ಸ್ಥಿರ Expected<lostFraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator end,
                            unsigned ಪೂರ್ಣಾಙ್ಕ digit_ಮೌಲ್ಯ) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  ಯದಿ (digit_ಮೌಲ್ಯ > 8)
    ಪ್ರತ್ಯಾಗಮ lfMoreThanHalf;
  ಅನ್ಯಥಾ ಯದಿ (digit_ಮೌಲ್ಯ < 8 && digit_ಮೌಲ್ಯ > 0)
    ಪ್ರತ್ಯಾಗಮ lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  ಯಾವತ್ (p != end && (*p == '0' || *p == '.'))
    p++;

  ಯದಿ (p == end)
    ಪ್ರತ್ಯಾಗಮ createError("Invalid trailing hexadecimal fraction!");

  unsigned hexDigit = hex_Digit_ಮೌಲ್ಯ(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  ಯದಿ (hexDigit == UINT_MAX)
    ಪ್ರತ್ಯಾಗಮ digit_ಮೌಲ್ಯ == 0 ? lfExactlyZero: lfExactlyHalf;
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ digit_ಮೌಲ್ಯ == 0 ? lfLessThanHalf: lfMoreThanHalf;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
ಸ್ಥಿರ lostFraction
lostFractionThroughTruncation(ಧ್ರುವ APFloatBase::integerPart *parts,
                              unsigned ಪೂರ್ಣಾಙ್ಕ part_ಎಣಿಕೆ,
                              unsigned ಪೂರ್ಣಾಙ್ಕ bits)
{
  unsigned lsb = APInt::tcLSB(parts, part_ಎಣಿಕೆ);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  ಯದಿ (bits <= lsb)
    ಪ್ರತ್ಯಾಗಮ lfExactlyZero;
  ಯದಿ (bits == lsb + 1)
    ಪ್ರತ್ಯಾಗಮ lfExactlyHalf;
  ಯದಿ (bits <= part_ಎಣಿಕೆ * APFloatBase::integerPartWidth &&
      APInt::tcExtractBit(parts, bits - 1))
    ಪ್ರತ್ಯಾಗಮ lfMoreThanHalf;

  ಪ್ರತ್ಯಾಗಮ lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
ಸ್ಥಿರ lostFraction
shift_ಬಲ(APFloatBase::integerPart *dst, unsigned ಪೂರ್ಣಾಙ್ಕ parts, unsigned ಪೂರ್ಣಾಙ್ಕ bits)
{
  lostFraction lost_fraction = lostFractionThroughTruncation(dst, parts, bits);

  APInt::tc_Shift_ಬಲ(dst, parts, bits);

  ಪ್ರತ್ಯಾಗಮ lost_fraction;
}

/* Combine the effect of two lost fractions.  */
ಸ್ಥಿರ lostFraction
combineLostFractions(lostFraction moreSignificant,
                     lostFraction lessSignificant)
{
  ಯದಿ (lessSignificant != lfExactlyZero) {
    ಯದಿ (moreSignificant == lfExactlyZero)
      moreSignificant = lfLessThanHalf;
    ಅನ್ಯಥಾ ಯದಿ (moreSignificant == lfExactlyHalf)
      moreSignificant = lfMoreThanHalf;
  }

  ಪ್ರತ್ಯಾಗಮ moreSignificant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ
HUerrBound(ತಾರ್ಕಿಕ inexactMultiply, unsigned ಪೂರ್ಣಾಙ್ಕ HUerr1, unsigned ಪೂರ್ಣಾಙ್ಕ HUerr2)
{
  ಪ್ರತಿಜ್ಞಾ(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  ಯದಿ (HUerr1 + HUerr2 == 0)
    ಪ್ರತ್ಯಾಗಮ inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
ಸ್ಥಿರ APFloatBase::integerPart
ulpsFromBoundary(ಧ್ರುವ APFloatBase::integerPart *parts, unsigned ಪೂರ್ಣಾಙ್ಕ bits,
                 ತಾರ್ಕಿಕ isNearest) {
  ಪ್ರತಿಜ್ಞಾ(bits != 0);

  bits--;
  unsigned ಎಣಿಕೆ = bits / APFloatBase::integerPartWidth;
  unsigned partBits = bits % APFloatBase::integerPartWidth + 1;

  APFloatBase::integerPart part =
      parts[ಎಣಿಕೆ] & (~(APFloatBase::integerPart)0 >>
                      (APFloatBase::integerPartWidth - partBits));

  APFloatBase::integerPart boundary;
  ಯದಿ (isNearest)
    boundary = (APFloatBase::integerPart) 1 << (partBits - 1);
  ಅನ್ಯಥಾ
    boundary = 0;

  ಯದಿ (ಎಣಿಕೆ == 0) {
    ಯದಿ (part - boundary <= boundary - part)
      ಪ್ರತ್ಯಾಗಮ part - boundary;
    ಅನ್ಯಥಾ
      ಪ್ರತ್ಯಾಗಮ boundary - part;
  }

  ಯದಿ (part == boundary) {
    ಯಾವತ್ (--ಎಣಿಕೆ)
      ಯದಿ (parts[ಎಣಿಕೆ])
        ಪ್ರತ್ಯಾಗಮ ~(APFloatBase::integerPart) 0; /* A lot.  */

    ಪ್ರತ್ಯಾಗಮ parts[0];
  } ಅನ್ಯಥಾ ಯದಿ (part == boundary - 1) {
    ಯಾವತ್ (--ಎಣಿಕೆ)
      ಯದಿ (~parts[ಎಣಿಕೆ])
        ಪ್ರತ್ಯಾಗಮ ~(APFloatBase::integerPart) 0; /* A lot.  */

    ಪ್ರತ್ಯಾಗಮ -parts[0];
  }

  ಪ್ರತ್ಯಾಗಮ ~(APFloatBase::integerPart) 0; /* A lot.  */
}

ಸ್ಥಿರ constexpr unsigned PowerOfFivePartCounts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

ಸ್ಥಿರ constexpr APFloatBase::integerPart PowerOfFiveParts[] = {
#ಅನ್ತರ್ಭಾವ "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  unsigned Sum = 0;
  ಪ್ರತಿ (unsigned ಎಣಿಕೆ_2 : PowerOfFivePartCounts)
    Sum += ಎಣಿಕೆ_2;
  ಪ್ರತ್ಯಾಗಮ Sum;
}() == std::size(PowerOfFiveParts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ
powerOf5(APFloatBase::integerPart *dst, unsigned ಪೂರ್ಣಾಙ್ಕ power) {
  ಸ್ಥಿರ ಧ್ರುವ APFloatBase::integerPart firstEightPowers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  SmallVector<APFloatBase::integerPart, ಅಂಕೆ_Pow5_Parts_On_ರಾಶಿ> scratch;
  scratch.resize_for_overwrite(powerOfFivePartsForExponent(power));

  APFloatBase::integerPart *p1, *p2;
  ಧ್ರುವ APFloatBase::integerPart *pow5;

  p1 = dst;
  p2 = scratch.data();

  *p1 = firstEightPowers[power & 7];
  power >>= 3;

  unsigned result = 1;
  pow5 = PowerOfFiveParts;

  ಪ್ರತಿ (unsigned ಪೂರ್ಣಾಙ್ಕ n = 0; power; power >>= 1, n++) {
    unsigned parts_ಎಣಿಕೆ = PowerOfFivePartCounts[n];

    ಯದಿ (power & 1) {
      APFloatBase::integerPart *tmp;

      APInt::tcFullMultiply(p2, p1, pow5, result, parts_ಎಣಿಕೆ);
      result += parts_ಎಣಿಕೆ;
      ಯದಿ (p2[result - 1] == 0)
        result--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_ಎಣಿಕೆ;
  }

  ಯದಿ (p1 != dst)
    APInt::tcAssign(dst, p1, result);

  ಪ್ರತ್ಯಾಗಮ result;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ hexDigitsLower[] = "0123456789abcdef0";
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ hexDigitsUpper[] = "0123456789ABCDEF0";
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ infinityL[] = "infinity";
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ infinityU[] = "INFINITY";
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ NaNL[] = "nan";
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ
partAsHex (ವರ್ಣ *dst, APFloatBase::integerPart part, unsigned ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ,
           ಧ್ರುವ ವರ್ಣ *hexDigitChars)
{
  unsigned ಪೂರ್ಣಾಙ್ಕ result = ಎಣಿಕೆ;

  ಪ್ರತಿಜ್ಞಾ(ಎಣಿಕೆ != 0 && ಎಣಿಕೆ <= APFloatBase::integerPartWidth / 4);

  part >>= (APFloatBase::integerPartWidth - 4 * ಎಣಿಕೆ);
  ಯಾವತ್ (ಎಣಿಕೆ--) {
    dst[ಎಣಿಕೆ] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  ಪ್ರತ್ಯಾಗಮ result;
}

/* Write out an unsigned decimal integer.  */
ಸ್ಥಿರ ವರ್ಣ *writeUnsignedDecimal(ವರ್ಣ *dst, unsigned ಪೂರ್ಣಾಙ್ಕ n) {
  ವರ್ಣ buff[40], *p;

  p = buff;
  ಕುರು
    *p++ = '0' + n % 10;
  ಯಾವತ್ (n /= 10);

  ಕುರು
    *dst++ = *--p;
  ಯಾವತ್ (p != buff);

  ಪ್ರತ್ಯಾಗಮ dst;
}

/* Write out a signed decimal integer.  */
ಸ್ಥಿರ ವರ್ಣ *writeSignedDecimal(ವರ್ಣ *dst, ಪೂರ್ಣಾಙ್ಕ ಮೌಲ್ಯ_2) {
  ಯದಿ (ಮೌಲ್ಯ_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(unsigned) ಮೌಲ್ಯ_2);
  } ಅನ್ಯಥಾ {
    dst = writeUnsignedDecimal(dst, ಮೌಲ್ಯ_2);
  }

  ಪ್ರತ್ಯಾಗಮ dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
ಸ್ಥಿರ APFloat harrisonUlp(ಧ್ರುವ APFloat &X) {
  ಧ್ರುವ fltSemantics &Sem = X.getSemantics();
  ವಿಕಲ್ಪ (X.getCategory()) {
  ಪಕ್ಷ APFloat::fcNaN:
    ಪ್ರತ್ಯಾಗಮ APFloat::getQNaN(Sem);
  ಪಕ್ಷ APFloat::fcInfinity:
    ಪ್ರತ್ಯಾಗಮ APFloat::getInf(Sem);
  ಪಕ್ಷ APFloat::fcZero:
    ಪ್ರತ್ಯಾಗಮ APFloat::getSmallest(Sem);
  ಪಕ್ಷ APFloat::fcNormal:
    ಭಙ್ಗ;
  }
  ಯದಿ (X.isDenormal() || X.isSmallestNormalized())
    ಪ್ರತ್ಯಾಗಮ APFloat::getSmallest(Sem);
  ಪೂರ್ಣಾಙ್ಕ Exp = ilogb(X);
  ಯದಿ (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  ಪ್ರತ್ಯಾಗಮ scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rmNearestTiesToEven);
}

ನಾಮಾಕಾಶ detail {
/* Constructors.  */
ಶೂನ್ಯ IEEEFloat::initialize(ಧ್ರುವ fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  unsigned ಎಣಿಕೆ = part_ಎಣಿಕೆ();
  ಯದಿ (ಎಣಿಕೆ > 1)
    significand.parts = ನೂತನ integerPart[ಎಣಿಕೆ];
}

ಶೂನ್ಯ IEEEFloat::freeSignificand() {
  ಯದಿ (needsCleanup())
    delete [] significand.parts;
}

ಶೂನ್ಯ IEEEFloat::assign(ಧ್ರುವ IEEEFloat &rhs) {
  ಪ್ರತಿಜ್ಞಾ(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  ಯದಿ (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

ಶೂನ್ಯ IEEEFloat::copySignificand(ಧ್ರುವ IEEEFloat &rhs) {
  ಪ್ರತಿಜ್ಞಾ(isFiniteNonZero() || category == fcNaN);
  ಪ್ರತಿಜ್ಞಾ(rhs.part_ಎಣಿಕೆ() >= part_ಎಣಿಕೆ());

  APInt::tcAssign(significandParts(), rhs.significandParts(),
                  part_ಎಣಿಕೆ());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
ಶೂನ್ಯ IEEEFloat::makeNaN(ತಾರ್ಕಿಕ SNaN, ತಾರ್ಕಿಕ Negative, ಧ್ರುವ APInt *fill) {
  ಯದಿ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  ಯದಿ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  unsigned ಅಂಕೆ_Parts = part_ಎಣಿಕೆ();

  APInt fill_storage;
  ಯದಿ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = ಅಸತ್ಯ;
    ಯದಿ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = ಸತ್ಯ;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } ಅನ್ಯಥಾ {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  ಯದಿ (!fill || fill->get_ಅಂಕೆ_ಪದಗಳು() < ಅಂಕೆ_Parts)
    APInt::tcSet(significand, 0, ಅಂಕೆ_Parts);
  ಯದಿ (fill) {
    APInt::tcAssign(significand, fill->getRawData(),
                    std::min(fill->get_ಅಂಕೆ_ಪದಗಳು(), ಅಂಕೆ_Parts));

    // Zero out the excess bits of the significand.
    unsigned bitsToPreserve = semantics->precision - 1;
    unsigned part = bitsToPreserve / 64;
    bitsToPreserve %= 64;
    significand[part] &= ((1ULL << bitsToPreserve) - 1);
    ಪ್ರತಿ (part++; part != ಅಂಕೆ_Parts; ++part)
      significand[part] = 0;
  }

  unsigned QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  ಯದಿ (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tcClearBit(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    ಯದಿ (APInt::tcIsZero(significand, ಅಂಕೆ_Parts))
      APInt::tcSetBit(significand, QNaNBit - 1);
  } ಅನ್ಯಥಾ ಯದಿ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } ಅನ್ಯಥಾ {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tcSetBit(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  ಯದಿ (semantics == &APFloatBase::semX87DoubleExtended)
    APInt::tcSetBit(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::operator=(ಧ್ರುವ IEEEFloat &rhs) {
  ಯದಿ (ಏತತ್ != &rhs) {
    ಯದಿ (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    assign(rhs);
  }

  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

IEEEFloat &IEEEFloat::operator=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloatBase::semBogus;
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

ತಾರ್ಕಿಕ IEEEFloat::isDenormal() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tcExtractBit(significandParts(), semantics->precision - 1) ==
          0);
}

ತಾರ್ಕಿಕ IEEEFloat::isSmallest() ಧ್ರುವ {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  ಪ್ರತ್ಯಾಗಮ isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

ತಾರ್ಕಿಕ IEEEFloat::isSmallestNormalized() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ getCategory() == fcNormal && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

unsigned ಪೂರ್ಣಾಙ್ಕ IEEEFloat::get_ಅಂಕೆ_ಮೇಲಿನ_Bits() ಧ್ರುವ {
  ಧ್ರುವ unsigned ಪೂರ್ಣಾಙ್ಕ Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_Bits(semantics->precision);
  ಧ್ರುವ unsigned ಪೂರ್ಣಾಙ್ಕ Bits = Part_ಎಣಿಕೆ * integerPartWidth;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  ಧ್ರುವ unsigned ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_ಮೇಲಿನ_Bits = (semantics->precision > 1)
                                       ? (Bits - semantics->precision + 1)
                                       : (Bits - semantics->precision);
  ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ_ಮೇಲಿನ_Bits;
}

ತಾರ್ಕಿಕ IEEEFloat::isSignificandAllOnes() ಧ್ರುವ {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  ಧ್ರುವ integerPart *Parts = significandParts();
  ಧ್ರುವ unsigned Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_Bits(semantics->precision);
  ಪ್ರತಿ (unsigned i = 0; i < Part_ಎಣಿಕೆ - 1; i++)
    ಯದಿ (~Parts[i])
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // Set the unused high bits to all ones when we compare.
  ಧ್ರುವ unsigned ಅಂಕೆ_ಮೇಲಿನ_Bits = get_ಅಂಕೆ_ಮೇಲಿನ_Bits();
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_ಮೇಲಿನ_Bits <= integerPartWidth && ಅಂಕೆ_ಮೇಲಿನ_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ಧ್ರುವ integerPart ಮೇಲಿನ_Bit_Fill =
    ~integerPart(0) << (integerPartWidth - ಅಂಕೆ_ಮೇಲಿನ_Bits);
  ಯದಿ ((semantics->precision <= 1) || (~(Parts[Part_ಎಣಿಕೆ - 1] | ಮೇಲಿನ_Bit_Fill)))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ IEEEFloat::isSignificandAllOnesExceptLSB() ಧ್ರುವ {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  ಧ್ರುವ integerPart *Parts = significandParts();

  ಯದಿ (Parts[0] & 1)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಧ್ರುವ unsigned Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_Bits(semantics->precision);
  ಪ್ರತಿ (unsigned i = 0; i < Part_ಎಣಿಕೆ - 1; i++) {
    ಯದಿ (~Parts[i] & ~unsigned{!i})
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  // Set the unused high bits to all ones when we compare.
  ಧ್ರುವ unsigned ಅಂಕೆ_ಮೇಲಿನ_Bits = get_ಅಂಕೆ_ಮೇಲಿನ_Bits();
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_ಮೇಲಿನ_Bits <= integerPartWidth && ಅಂಕೆ_ಮೇಲಿನ_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ಧ್ರುವ integerPart ಮೇಲಿನ_Bit_Fill = ~integerPart(0)
                                  << (integerPartWidth - ಅಂಕೆ_ಮೇಲಿನ_Bits);
  ಯದಿ (~(Parts[Part_ಎಣಿಕೆ - 1] | ಮೇಲಿನ_Bit_Fill | 0x1))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ IEEEFloat::isSignificandAllZeros() ಧ್ರುವ {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  ಧ್ರುವ integerPart *Parts = significandParts();
  ಧ್ರುವ unsigned Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_Bits(semantics->precision);

  ಪ್ರತಿ (unsigned i = 0; i < Part_ಎಣಿಕೆ - 1; i++)
    ಯದಿ (Parts[i])
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // Compute how many bits are used in the final word.
  ಧ್ರುವ unsigned ಅಂಕೆ_ಮೇಲಿನ_Bits = get_ಅಂಕೆ_ಮೇಲಿನ_Bits();
  ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_ಮೇಲಿನ_Bits < integerPartWidth && "Can not have more high bits to "
         "clear than integerPartWidth");
  ಧ್ರುವ integerPart ಮೇಲಿನ_Bit_Mask = ~integerPart(0) >> ಅಂಕೆ_ಮೇಲಿನ_Bits;

  ಯದಿ ((semantics->precision > 1) && (Parts[Part_ಎಣಿಕೆ - 1] & ಮೇಲಿನ_Bit_Mask))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

ತಾರ್ಕಿಕ IEEEFloat::isSignificandAllZerosExceptMSB() ಧ್ರುವ {
  ಧ್ರುವ integerPart *Parts = significandParts();
  ಧ್ರುವ unsigned Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_Bits(semantics->precision);

  ಪ್ರತಿ (unsigned i = 0; i < Part_ಎಣಿಕೆ - 1; i++) {
    ಯದಿ (Parts[i])
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  ಧ್ರುವ unsigned ಅಂಕೆ_ಮೇಲಿನ_Bits = get_ಅಂಕೆ_ಮೇಲಿನ_Bits();
  ಧ್ರುವ integerPart MSBMask = integerPart(1)
                              << (integerPartWidth - ಅಂಕೆ_ಮೇಲಿನ_Bits);
  ಪ್ರತ್ಯಾಗಮ ((semantics->precision <= 1) || (Parts[Part_ಎಣಿಕೆ - 1] == MSBMask));
}

ತಾರ್ಕಿಕ IEEEFloat::isLargest() ಧ್ರುವ {
  ತಾರ್ಕಿಕ IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  ಯದಿ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    ಪ್ರತ್ಯಾಗಮ (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } ಅನ್ಯಥಾ {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    ಪ್ರತ್ಯಾಗಮ IsMaxExp && isSignificandAllOnes();
  }
}

ತಾರ್ಕಿಕ IEEEFloat::isInteger() ಧ್ರುವ {
  // This could be made more efficient; I'm going for obviously correct.
  ಯದಿ (!isFinite()) ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  IEEEFloat truncated = *ಏತತ್;
  truncated.roundToIntegral(rmTowardZero);
  ಪ್ರತ್ಯಾಗಮ compare(truncated) == cmpEqual;
}

ತಾರ್ಕಿಕ IEEEFloat::bitwiseIsEqual(ಧ್ರುವ IEEEFloat &rhs) ಧ್ರುವ {
  ಯದಿ (ಏತತ್ == &rhs)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  ಯದಿ (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (category==fcZero || category==fcInfinity)
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

  ಯದಿ (isFiniteNonZero() && exponent != rhs.exponent)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಪ್ರತ್ಯಾಗಮ std::equal(significandParts(), significandParts() + part_ಎಣಿಕೆ(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(ಧ್ರುವ fltSemantics &ourSemantics, integerPart ಮೌಲ್ಯ_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fcNormal;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = ಮೌಲ್ಯ_2;
  normalize(rmNearestTiesToEven, lfExactlyZero);
}

IEEEFloat::IEEEFloat(ಧ್ರುವ fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(ಅಸತ್ಯ) : makeSmallestNormalized(ಅಸತ್ಯ);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(ಧ್ರುವ fltSemantics &ourSemantics, uninitializedTag tag)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(ಧ್ರುವ IEEEFloat &rhs) {
  initialize(rhs.semantics);
  assign(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloatBase::semBogus) {
  *ಏತತ್ = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

unsigned ಪೂರ್ಣಾಙ್ಕ IEEEFloat::part_ಎಣಿಕೆ() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ part_ಎಣಿಕೆ_For_Bits(semantics->precision + 1);
}

ಧ್ರುವ APFloat::integerPart *IEEEFloat::significandParts() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ const_cast<IEEEFloat *>(ಏತತ್)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  ಯದಿ (part_ಎಣಿಕೆ() > 1)
    ಪ್ರತ್ಯಾಗಮ significand.parts;
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ &significand.part;
}

ಶೂನ್ಯ IEEEFloat::zeroSignificand() {
  APInt::tcSet(significandParts(), 0, part_ಎಣಿಕೆ());
}

/* Increment an fcNormal floating point number's significand.  */
ಶೂನ್ಯ IEEEFloat::incrementSignificand() {
  [[maybe_unused]] integerPart carry =
      APInt::tcIncrement(significandParts(), part_ಎಣಿಕೆ());

  /* Our callers should never cause us to overflow.  */
  ಪ್ರತಿಜ್ಞಾ(carry == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::addSignificand(ಧ್ರುವ IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  ಪ್ರತಿಜ್ಞಾ(semantics == rhs.semantics);
  ಪ್ರತಿಜ್ಞಾ(exponent == rhs.exponent);

  ಪ್ರತ್ಯಾಗಮ APInt::tcAdd(parts, rhs.significandParts(), 0, part_ಎಣಿಕೆ());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::subtractSignificand(ಧ್ರುವ IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  ಪ್ರತಿಜ್ಞಾ(semantics == rhs.semantics);
  ಪ್ರತಿಜ್ಞಾ(exponent == rhs.exponent);

  ಪ್ರತ್ಯಾಗಮ APInt::tcSubtract(parts, rhs.significandParts(), borrow,
                           part_ಎಣಿಕೆ());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
lostFraction IEEEFloat::multiplySignificand(ಧ್ರುವ IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            ತಾರ್ಕಿಕ ignoreAddend) {
  integerPart scratch[4];
  ತಾರ್ಕಿಕ ignored;

  ಪ್ರತಿಜ್ಞಾ(semantics == rhs.semantics);

  unsigned precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  unsigned new_Parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_Bits(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *fullSignificand =
      new_Parts_ಎಣಿಕೆ > 4 ? ನೂತನ integerPart[new_Parts_ಎಣಿಕೆ] : scratch;

  integerPart *lhsSignificand = significandParts();
  unsigned parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ();

  APInt::tcFullMultiply(fullSignificand, lhsSignificand,
                        rhs.significandParts(), parts_ಎಣಿಕೆ, parts_ಎಣಿಕೆ);

  lostFraction lost_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  unsigned omsb = APInt::tcMSB(fullSignificand, new_Parts_ಎಣಿಕೆ) + 1;
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

  ಯದಿ (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    ಧ್ರುವ fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    unsigned extendedPrecision = 2 * precision + 1;
    ಯದಿ (omsb != extendedPrecision - 1) {
      ಪ್ರತಿಜ್ಞಾ(extendedPrecision > omsb);
      APInt::tc_Shift_ಎಡ(fullSignificand, new_Parts_ಎಣಿಕೆ,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    ಯದಿ (new_Parts_ಎಣಿಕೆ == 1)
      significand.part = fullSignificand[0];
    ಅನ್ಯಥಾ
      significand.parts = fullSignificand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_unused]] opStatus status = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    ಪ್ರತಿಜ್ಞಾ(status == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    lost_fraction = extendedAddend.shift_Significand_ಬಲ(1);
    ಪ್ರತಿಜ್ಞಾ(lost_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    lost_fraction = addOrSubtractSignificand(extendedAddend, ಅಸತ್ಯ);

    /* Restore our state.  */
    ಯದಿ (new_Parts_ಎಣಿಕೆ == 1)
      fullSignificand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(fullSignificand, new_Parts_ಎಣಿಕೆ) + 1;
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
  ಯದಿ (omsb > precision) {
    unsigned ಪೂರ್ಣಾಙ್ಕ bits, significantParts;
    lostFraction lf;

    bits = omsb - precision;
    significantParts = part_ಎಣಿಕೆ_For_Bits(omsb);
    lf = shift_ಬಲ(fullSignificand, significantParts, bits);
    lost_fraction = combineLostFractions(lf, lost_fraction);
    exponent += bits;
  }

  APInt::tcAssign(lhsSignificand, fullSignificand, parts_ಎಣಿಕೆ);

  ಯದಿ (new_Parts_ಎಣಿಕೆ > 4)
    delete [] fullSignificand;

  ಪ್ರತ್ಯಾಗಮ lost_fraction;
}

lostFraction IEEEFloat::multiplySignificand(ಧ್ರುವ IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  ಪ್ರತ್ಯಾಗಮ multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
lostFraction IEEEFloat::divideSignificand(ಧ್ರುವ IEEEFloat &rhs) {
  integerPart scratch[4];

  ಪ್ರತಿಜ್ಞಾ(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  ಧ್ರುವ integerPart *rhsSignificand = rhs.significandParts();
  unsigned parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ();

  integerPart *dividend =
      parts_ಎಣಿಕೆ > 2 ? ನೂತನ integerPart[parts_ಎಣಿಕೆ * 2] : scratch;
  integerPart *ಭಾಜಕ = dividend + parts_ಎಣಿಕೆ;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  ಪ್ರತಿ (unsigned i = 0; i < parts_ಎಣಿಕೆ; i++) {
    dividend[i] = lhsSignificand[i];
    ಭಾಜಕ[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  unsigned ಪೂರ್ಣಾಙ್ಕ precision = semantics->precision;

  /* Normalize the divisor.  */
  unsigned bit = precision - APInt::tcMSB(ಭಾಜಕ, parts_ಎಣಿಕೆ) - 1;
  ಯದಿ (bit) {
    exponent += bit;
    APInt::tc_Shift_ಎಡ(ಭಾಜಕ, parts_ಎಣಿಕೆ, bit);
  }

  /* Normalize the dividend.  */
  bit = precision - APInt::tcMSB(dividend, parts_ಎಣಿಕೆ) - 1;
  ಯದಿ (bit) {
    exponent -= bit;
    APInt::tc_Shift_ಎಡ(dividend, parts_ಎಣಿಕೆ, bit);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  ಯದಿ (APInt::tcCompare(dividend, ಭಾಜಕ, parts_ಎಣಿಕೆ) < 0) {
    exponent--;
    APInt::tc_Shift_ಎಡ(dividend, parts_ಎಣಿಕೆ, 1);
    ಪ್ರತಿಜ್ಞಾ(APInt::tcCompare(dividend, ಭಾಜಕ, parts_ಎಣಿಕೆ) >= 0);
  }

  /* Long division.  */
  ಪ್ರತಿ (bit = precision; bit; bit -= 1) {
    ಯದಿ (APInt::tcCompare(dividend, ಭಾಜಕ, parts_ಎಣಿಕೆ) >= 0) {
      APInt::tcSubtract(dividend, ಭಾಜಕ, 0, parts_ಎಣಿಕೆ);
      APInt::tcSetBit(lhsSignificand, bit - 1);
    }

    APInt::tc_Shift_ಎಡ(dividend, parts_ಎಣಿಕೆ, 1);
  }

  /* Figure out the lost fraction.  */
  ಪೂರ್ಣಾಙ್ಕ cmp = APInt::tcCompare(dividend, ಭಾಜಕ, parts_ಎಣಿಕೆ);

  lostFraction lost_fraction;
  ಯದಿ (cmp > 0)
    lost_fraction = lfMoreThanHalf;
  ಅನ್ಯಥಾ ಯದಿ (cmp == 0)
    lost_fraction = lfExactlyHalf;
  ಅನ್ಯಥಾ ಯದಿ (APInt::tcIsZero(dividend, parts_ಎಣಿಕೆ))
    lost_fraction = lfExactlyZero;
  ಅನ್ಯಥಾ
    lost_fraction = lfLessThanHalf;

  ಯದಿ (parts_ಎಣಿಕೆ > 2)
    delete [] dividend;

  ಪ್ರತ್ಯಾಗಮ lost_fraction;
}

unsigned ಪೂರ್ಣಾಙ್ಕ IEEEFloat::significandMSB() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ APInt::tcMSB(significandParts(), part_ಎಣಿಕೆ());
}

unsigned ಪೂರ್ಣಾಙ್ಕ IEEEFloat::significandLSB() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ APInt::tcLSB(significandParts(), part_ಎಣಿಕೆ());
}

/* Note that a zero result is NOT normalized to fcZero.  */
lostFraction IEEEFloat::shift_Significand_ಬಲ(unsigned ಪೂರ್ಣಾಙ್ಕ bits) {
  /* Our exponent should not overflow.  */
  ಪ್ರತಿಜ್ಞಾ((ExponentType) (exponent + bits) >= exponent);

  exponent += bits;

  ಪ್ರತ್ಯಾಗಮ shift_ಬಲ(significandParts(), part_ಎಣಿಕೆ(), bits);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
ಶೂನ್ಯ IEEEFloat::shift_Significand_ಎಡ(unsigned ಪೂರ್ಣಾಙ್ಕ bits) {
  ಪ್ರತಿಜ್ಞಾ(bits < semantics->precision ||
         (semantics->precision == 1 && bits <= 1));

  ಯದಿ (bits) {
    unsigned ಪೂರ್ಣಾಙ್ಕ parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ();

    APInt::tc_Shift_ಎಡ(significandParts(), parts_ಎಣಿಕೆ, bits);
    exponent -= bits;

    ಪ್ರತಿಜ್ಞಾ(!APInt::tcIsZero(significandParts(), parts_ಎಣಿಕೆ));
  }
}

APFloat::cmpResult IEEEFloat::compare_Absolute_ಮೌಲ್ಯ(ಧ್ರುವ IEEEFloat &rhs) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(semantics == rhs.semantics);
  ಪ್ರತಿಜ್ಞಾ(isFiniteNonZero());
  ಪ್ರತಿಜ್ಞಾ(rhs.isFiniteNonZero());

  ಪೂರ್ಣಾಙ್ಕ compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  ಯದಿ (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_ಎಣಿಕೆ());

  ಯದಿ (compare > 0)
    ಪ್ರತ್ಯಾಗಮ cmpGreaterThan;
  ಅನ್ಯಥಾ ಯದಿ (compare < 0)
    ಪ್ರತ್ಯಾಗಮ cmpLessThan;
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
ಸ್ಥಿರ ಶೂನ್ಯ tcSetLeastSignificantBits(APInt::WordType *dst, unsigned parts,
                                      unsigned bits) {
  unsigned i = 0;
  ಯಾವತ್ (bits > APInt::APINT_BITS_PER_WORD) {
    dst[i++] = ~(APInt::WordType)0;
    bits -= APInt::APINT_BITS_PER_WORD;
  }

  ಯದಿ (bits)
    dst[i++] = ~(APInt::WordType)0 >> (APInt::APINT_BITS_PER_WORD - bits);

  ಯಾವತ್ (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::opStatus IEEEFloat::handleOverflow(roundingMode rounding_mode) {
  ಯದಿ (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    ಯದಿ (rounding_mode == rmNearestTiesToEven ||
        rounding_mode == rmNearestTiesToAway ||
        (rounding_mode == rmTowardPositive && !sign) ||
        (rounding_mode == rmTowardNegative && sign)) {
      ಯದಿ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(ಅಸತ್ಯ, sign);
      ಅನ್ಯಥಾ
        category = fcInfinity;
      ಪ್ರತ್ಯಾಗಮ static_cast<opStatus>(opOverflow | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fcNormal;
  exponent = semantics->maxExponent;
  tcSetLeastSignificantBits(significandParts(), part_ಎಣಿಕೆ(),
                            semantics->precision);
  ಯದಿ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tcClearBit(significandParts(), 0);

  ಪ್ರತ್ಯಾಗಮ opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
ತಾರ್ಕಿಕ IEEEFloat::roundAwayFromZero(roundingMode rounding_mode,
                                  lostFraction lost_fraction,
                                  unsigned ಪೂರ್ಣಾಙ್ಕ bit) ಧ್ರುವ {
  /* NaNs and infinities should not have lost fractions.  */
  ಪ್ರತಿಜ್ಞಾ(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  ಪ್ರತಿಜ್ಞಾ(lost_fraction != lfExactlyZero);

  ವಿಕಲ್ಪ (rounding_mode) {
  ಪಕ್ಷ rmNearestTiesToAway:
    ಪ್ರತ್ಯಾಗಮ lost_fraction == lfExactlyHalf || lost_fraction == lfMoreThanHalf;

  ಪಕ್ಷ rmNearestTiesToEven:
    ಯದಿ (lost_fraction == lfMoreThanHalf)
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

    /* Our zeroes don't have a significand to test.  */
    ಯದಿ (lost_fraction == lfExactlyHalf && category != fcZero)
      ಪ್ರತ್ಯಾಗಮ APInt::tcExtractBit(significandParts(), bit);

    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಪಕ್ಷ rmTowardZero:
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಪಕ್ಷ rmTowardPositive:
    ಪ್ರತ್ಯಾಗಮ !sign;

  ಪಕ್ಷ rmTowardNegative:
    ಪ್ರತ್ಯಾಗಮ sign;

  ಸ್ವಭಾವ:
    ಭಙ್ಗ;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::opStatus IEEEFloat::normalize(roundingMode rounding_mode,
                                       lostFraction lost_fraction) {
  ಯದಿ (!isFiniteNonZero())
    ಪ್ರತ್ಯಾಗಮ opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  unsigned omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  ಯದಿ (omsb || lost_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    ಪೂರ್ಣಾಙ್ಕ exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    ಯದಿ (exponent + exponentChange > semantics->maxExponent)
      ಪ್ರತ್ಯಾಗಮ handleOverflow(rounding_mode);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    ಯದಿ (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    ಯದಿ (exponentChange < 0) {
      ಪ್ರತಿಜ್ಞಾ(lost_fraction == lfExactlyZero);

      shift_Significand_ಎಡ(-exponentChange);

      ಪ್ರತ್ಯಾಗಮ opOK;
    }

    ಯದಿ (exponentChange > 0) {
      lostFraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = shift_Significand_ಬಲ(exponentChange);

      lost_fraction = combineLostFractions(lf, lost_fraction);

      /* Keep OMSB up-to-date.  */
      ಯದಿ (omsb > (unsigned) exponentChange)
        omsb -= exponentChange;
      ಅನ್ಯಥಾ
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  ಯದಿ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    ಪ್ರತ್ಯಾಗಮ handleOverflow(rounding_mode);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  ಯದಿ (lost_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    ಯದಿ (omsb == 0) {
      category = fcZero;
      ಯದಿ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = ಅಸತ್ಯ;
      ಯದಿ (!semantics->hasZero)
        makeSmallestNormalized(ಅಸತ್ಯ);
    }

    ಪ್ರತ್ಯಾಗಮ opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  ಯದಿ (roundAwayFromZero(rounding_mode, lost_fraction, 0)) {
    ಯದಿ (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    ಯದಿ (omsb == (unsigned) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      ಯದಿ (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        ಪ್ರತ್ಯಾಗಮ handleOverflow(sign ? rmTowardNegative : rmTowardPositive);

      shift_Significand_ಬಲ(1);

      ಪ್ರತ್ಯಾಗಮ opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    ಯದಿ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      ಪ್ರತ್ಯಾಗಮ handleOverflow(rounding_mode);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  ಯದಿ (omsb == semantics->precision)
    ಪ್ರತ್ಯಾಗಮ opInexact;

  /* We have a non-zero denormal.  */
  ಪ್ರತಿಜ್ಞಾ(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  ಯದಿ (omsb == 0) {
    category = fcZero;
    ಯದಿ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ಅಸತ್ಯ;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    ಯದಿ (!semantics->hasZero)
      makeSmallestNormalized(ಅಸತ್ಯ);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  ಪ್ರತ್ಯಾಗಮ (opStatus) (opUnderflow | opInexact);
}

APFloat::opStatus IEEEFloat::addOrSubtractSpecials(ಧ್ರುವ IEEEFloat &rhs,
                                                   ತಾರ್ಕಿಕ subtract) {
  ವಿಕಲ್ಪ (Pack_Categories_Into_ಕೀಲಿ(category, rhs.category)) {
  ಸ್ವಭಾವ:
    llvm_unreachable(nullptr);

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNaN):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcNaN):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcZero):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNormal):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNaN):
    ಯದಿ (isSignaling()) {
      makeQuiet();
      ಪ್ರತ್ಯಾಗಮ opInvalidOp;
    }
    ಪ್ರತ್ಯಾಗಮ rhs.isSignaling() ? opInvalidOp : opOK;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcZero):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNormal):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcZero):
    ಪ್ರತ್ಯಾಗಮ opOK;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ subtract;
    ಪ್ರತ್ಯಾಗಮ opOK;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNormal):
    assign(rhs);
    sign = rhs.sign ^ subtract;
    ಪ್ರತ್ಯಾಗಮ opOK;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    ಪ್ರತ್ಯಾಗಮ opOK;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    ಯದಿ (((sign ^ rhs.sign)!=0) != subtract) {
      makeNaN();
      ಪ್ರತ್ಯಾಗಮ opInvalidOp;
    }

    ಪ್ರತ್ಯಾಗಮ opOK;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcNormal):
    ಪ್ರತ್ಯಾಗಮ opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
lostFraction IEEEFloat::addOrSubtractSignificand(ಧ್ರುವ IEEEFloat &rhs,
                                                 ತಾರ್ಕಿಕ subtract) {
  [[maybe_unused]] integerPart carry = 0;
  lostFraction lost_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  subtract ^= static_cast<ತಾರ್ಕಿಕ>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  ಪೂರ್ಣಾಙ್ಕ bits = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  ಯದಿ (subtract) {
    ಯದಿ ((bits < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat temp_rhs(rhs);
    ತಾರ್ಕಿಕ lost_fraction_is_from_rhs = ಅಸತ್ಯ;

    ಯದಿ (bits == 0)
      lost_fraction = lfExactlyZero;
    ಅನ್ಯಥಾ ಯದಿ (bits > 0) {
      lost_fraction = temp_rhs.shift_Significand_ಬಲ(bits - 1);
      lost_fraction_is_from_rhs = ಸತ್ಯ;
      shift_Significand_ಎಡ(1);
    } ಅನ್ಯಥಾ {
      lost_fraction = shift_Significand_ಬಲ(-bits - 1);
      temp_rhs.shift_Significand_ಎಡ(1);
    }

    // Should we reverse the subtraction.
    cmpResult cmp_result = compare_Absolute_ಮೌಲ್ಯ(temp_rhs);
    ಯದಿ (cmp_result == cmpLessThan) {
      ತಾರ್ಕಿಕ borrow =
          lost_fraction != lfExactlyZero && !lost_fraction_is_from_rhs;
      ಯದಿ (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        ಯದಿ (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        ಅನ್ಯಥಾ ಯದಿ (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = temp_rhs.subtractSignificand(*ಏತತ್, borrow);
      copySignificand(temp_rhs);
      sign = !sign;
    } ಅನ್ಯಥಾ ಯದಿ (cmp_result == cmpGreaterThan) {
      ತಾರ್ಕಿಕ borrow = lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs;
      ಯದಿ (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        ಯದಿ (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        ಅನ್ಯಥಾ ಯದಿ (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = subtractSignificand(temp_rhs, borrow);
    } ಅನ್ಯಥಾ { // cmpEqual
      zeroSignificand();
      ಯದಿ (lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    ಪ್ರತಿಜ್ಞಾ(!carry);
  } ಅನ್ಯಥಾ {
    ಯದಿ (bits > 0) {
      IEEEFloat temp_rhs(rhs);

      lost_fraction = temp_rhs.shift_Significand_ಬಲ(bits);
      carry = addSignificand(temp_rhs);
    } ಅನ್ಯಥಾ {
      lost_fraction = shift_Significand_ಬಲ(-bits);
      carry = addSignificand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    ಪ್ರತಿಜ್ಞಾ(!carry);
  }

  ಪ್ರತ್ಯಾಗಮ lost_fraction;
}

APFloat::opStatus IEEEFloat::multiplySpecials(ಧ್ರುವ IEEEFloat &rhs) {
  ವಿಕಲ್ಪ (Pack_Categories_Into_ಕೀಲಿ(category, rhs.category)) {
  ಸ್ವಭಾವ:
    llvm_unreachable(nullptr);

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNaN):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcNaN):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNaN):
    assign(rhs);
    sign = ಅಸತ್ಯ;
    [[fallthrough]];
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcZero):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNormal):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    ಯದಿ (isSignaling()) {
      makeQuiet();
      ಪ್ರತ್ಯಾಗಮ opInvalidOp;
    }
    ಪ್ರತ್ಯಾಗಮ rhs.isSignaling() ? opInvalidOp : opOK;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNormal):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcInfinity):
    category = fcInfinity;
    ಪ್ರತ್ಯಾಗಮ opOK;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNormal):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcZero):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcZero):
    category = fcZero;
    ಪ್ರತ್ಯಾಗಮ opOK;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcZero):
    makeNaN();
    ಪ್ರತ್ಯಾಗಮ opInvalidOp;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcNormal):
    ಪ್ರತ್ಯಾಗಮ opOK;
  }
}

APFloat::opStatus IEEEFloat::divideSpecials(ಧ್ರುವ IEEEFloat &rhs) {
  ವಿಕಲ್ಪ (Pack_Categories_Into_ಕೀಲಿ(category, rhs.category)) {
  ಸ್ವಭಾವ:
    llvm_unreachable(nullptr);

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNaN):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcNaN):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNaN):
    assign(rhs);
    sign = ಅಸತ್ಯ;
    [[fallthrough]];
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcZero):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNormal):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    ಯದಿ (isSignaling()) {
      makeQuiet();
      ಪ್ರತ್ಯಾಗಮ opInvalidOp;
    }
    ಪ್ರತ್ಯಾಗಮ rhs.isSignaling() ? opInvalidOp : opOK;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcZero):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNormal):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNormal):
    ಪ್ರತ್ಯಾಗಮ opOK;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcInfinity):
    category = fcZero;
    ಪ್ರತ್ಯಾಗಮ opOK;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcZero):
    ಯದಿ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(ಅಸತ್ಯ, sign);
    ಅನ್ಯಥಾ
      category = fcInfinity;
    ಪ್ರತ್ಯಾಗಮ opDivByZero;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcZero):
    makeNaN();
    ಪ್ರತ್ಯಾಗಮ opInvalidOp;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcNormal):
    ಪ್ರತ್ಯಾಗಮ opOK;
  }
}

APFloat::opStatus IEEEFloat::modSpecials(ಧ್ರುವ IEEEFloat &rhs) {
  ವಿಕಲ್ಪ (Pack_Categories_Into_ಕೀಲಿ(category, rhs.category)) {
  ಸ್ವಭಾವ:
    llvm_unreachable(nullptr);

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNaN):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcNaN):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcZero):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNormal):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNaN):
    ಯದಿ (isSignaling()) {
      makeQuiet();
      ಪ್ರತ್ಯಾಗಮ opInvalidOp;
    }
    ಪ್ರತ್ಯಾಗಮ rhs.isSignaling() ? opInvalidOp : opOK;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNormal):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcInfinity):
    ಪ್ರತ್ಯಾಗಮ opOK;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcZero):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcZero):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNormal):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcZero):
    makeNaN();
    ಪ್ರತ್ಯಾಗಮ opInvalidOp;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcNormal):
    ಪ್ರತ್ಯಾಗಮ opOK;
  }
}

APFloat::opStatus IEEEFloat::remainderSpecials(ಧ್ರುವ IEEEFloat &rhs) {
  ವಿಕಲ್ಪ (Pack_Categories_Into_ಕೀಲಿ(category, rhs.category)) {
  ಸ್ವಭಾವ:
    llvm_unreachable(nullptr);

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNaN):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcNaN):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcZero):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNormal):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNaN):
    ಯದಿ (isSignaling()) {
      makeQuiet();
      ಪ್ರತ್ಯಾಗಮ opInvalidOp;
    }
    ಪ್ರತ್ಯಾಗಮ rhs.isSignaling() ? opInvalidOp : opOK;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNormal):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcInfinity):
    ಪ್ರತ್ಯಾಗಮ opOK;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcZero):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcZero):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNormal):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcZero):
    makeNaN();
    ಪ್ರತ್ಯಾಗಮ opInvalidOp;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcNormal):
    ಪ್ರತ್ಯಾಗಮ opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
ಶೂನ್ಯ IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  ಯದಿ (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    ಪ್ರತ್ಯಾಗಮ;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::opStatus IEEEFloat::addOrSubtract(ಧ್ರುವ IEEEFloat &rhs,
                                           roundingMode rounding_mode,
                                           ತಾರ್ಕಿಕ subtract) {
  opStatus fs = addOrSubtractSpecials(rhs, subtract);

  /* This return code means it was not a simple case.  */
  ಯದಿ (fs == opDivByZero) {
    lostFraction lost_fraction;

    lost_fraction = addOrSubtractSignificand(rhs, subtract);
    fs = normalize(rounding_mode, lost_fraction);

    /* Can only be zero if we lost no fraction.  */
    ಪ್ರತಿಜ್ಞಾ(category != fcZero || lost_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  ಯದಿ (category == fcZero) {
    ಯದಿ (rhs.category != fcZero || (sign == rhs.sign) == subtract)
      sign = (rounding_mode == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    ಯದಿ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ಅಸತ್ಯ;
  }

  ಪ್ರತ್ಯಾಗಮ fs;
}

/* Normalized addition.  */
APFloat::opStatus IEEEFloat::add(ಧ್ರುವ IEEEFloat &rhs,
                                 roundingMode rounding_mode) {
  ಪ್ರತ್ಯಾಗಮ addOrSubtract(rhs, rounding_mode, ಅಸತ್ಯ);
}

/* Normalized subtraction.  */
APFloat::opStatus IEEEFloat::subtract(ಧ್ರುವ IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  ಪ್ರತ್ಯಾಗಮ addOrSubtract(rhs, rounding_mode, ಸತ್ಯ);
}

/* Normalized multiply.  */
APFloat::opStatus IEEEFloat::multiply(ಧ್ರುವ IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = multiplySpecials(rhs);

  ಯದಿ (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = ಅಸತ್ಯ;
  ಯದಿ (isFiniteNonZero()) {
    lostFraction lost_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    ಯದಿ (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  ಪ್ರತ್ಯಾಗಮ fs;
}

/* Normalized divide.  */
APFloat::opStatus IEEEFloat::divide(ಧ್ರುವ IEEEFloat &rhs,
                                    roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = divideSpecials(rhs);

  ಯದಿ (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = ಅಸತ್ಯ;
  ಯದಿ (isFiniteNonZero()) {
    lostFraction lost_fraction = divideSignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    ಯದಿ (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  ಪ್ರತ್ಯಾಗಮ fs;
}

/* Normalized remainder.  */
APFloat::opStatus IEEEFloat::remainder(ಧ್ರುವ IEEEFloat &rhs) {
  unsigned ಪೂರ್ಣಾಙ್ಕ origSign = sign;

  // First handle the special cases.
  opStatus fs = remainderSpecials(rhs);
  ಯದಿ (fs != opDivByZero)
    ಪ್ರತ್ಯಾಗಮ fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  ಯದಿ (P2.add(rhs, rmNearestTiesToEven) == opOK) {
    fs = mod(P2);
    ಪ್ರತಿಜ್ಞಾ(fs == opOK);
  }

  // Lets work with absolute numbers.
  IEEEFloat P = rhs;
  P.sign = ಅಸತ್ಯ;
  sign = ಅಸತ್ಯ;

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
  ತಾರ್ಕಿಕ losesInfo;
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.maxExponent++;
  extendedSemantics.minExponent--;
  extendedSemantics.precision += 2;

  IEEEFloat VEx = *ಏತತ್;
  fs = VEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  ಪ್ರತಿಜ್ಞಾ(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  ಪ್ರತಿಜ್ಞಾ(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.add(VEx, rmNearestTiesToEven);
  ಪ್ರತಿಜ್ಞಾ(fs == opOK);

  ಯದಿ (VEx.compare(PEx) == cmpGreaterThan) {
    fs = subtract(P, rmNearestTiesToEven);
    ಪ್ರತಿಜ್ಞಾ(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    ಪ್ರತಿಜ್ಞಾ(fs == opOK);
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    ಪ್ರತಿಜ್ಞಾ(fs == opOK);

    cmpResult result = VEx.compare(PEx);
    ಯದಿ (result == cmpGreaterThan || result == cmpEqual) {
      fs = subtract(P, rmNearestTiesToEven);
      ಪ್ರತಿಜ್ಞಾ(fs == opOK);
    }
  }

  ಯದಿ (isZero()) {
    sign = origSign;    // IEEE754 requires this
    ಯದಿ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = ಅಸತ್ಯ;
  } ಅನ್ಯಥಾ {
    sign ^= origSign;
  }
  ಪ್ರತ್ಯಾಗಮ fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::opStatus IEEEFloat::mod(ಧ್ರುವ IEEEFloat &rhs) {
  opStatus fs = modSpecials(rhs);
  unsigned ಪೂರ್ಣಾಙ್ಕ origSign = sign;

  ಯಾವತ್ (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_ಮೌಲ್ಯ(rhs) != cmpLessThan) {
    ಪೂರ್ಣಾಙ್ಕ Exp = ilogb(*ಏತತ್) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rmNearestTiesToEven);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    ಯದಿ (V.isNaN() || compare_Absolute_ಮೌಲ್ಯ(V) == cmpLessThan)
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
    ಯದಿ (!semantics->hasZero && ಏತತ್->isSmallest())
      ಭಙ್ಗ;

    ಪ್ರತಿಜ್ಞಾ(fs==opOK);
  }
  ಯದಿ (isZero()) {
    sign = origSign; // fmod requires this
    ಯದಿ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ಅಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ fs;
}

/* Normalized fused-multiply-add.  */
APFloat::opStatus IEEEFloat::fusedMultiplyAdd(ಧ್ರುವ IEEEFloat &multiplicand,
                                              ಧ್ರುವ IEEEFloat &addend,
                                              roundingMode rounding_mode) {
  opStatus fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  ಯದಿ (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    lostFraction lost_fraction;

    lost_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_mode, lost_fraction);
    ಯದಿ (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    ಯದಿ (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_mode == rmTowardNegative);
      ಯದಿ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = ಅಸತ್ಯ;
    }
  } ಅನ್ಯಥಾ {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    ಯದಿ (fs == opOK)
      fs = addOrSubtract(addend, rounding_mode, ಅಸತ್ಯ);
  }

  ಪ್ರತ್ಯಾಗಮ fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::opStatus IEEEFloat::roundToIntegral(roundingMode rounding_mode) {
  ಯದಿ (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    ಪ್ರತ್ಯಾಗಮ opOK;

  ಯದಿ (isNaN()) {
    ಯದಿ (isSignaling()) {
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
      ಪ್ರತ್ಯಾಗಮ opInvalidOp;
    } ಅನ್ಯಥಾ {
      // [IEEE Std 754-2008 6.2]:
      // For an operation with quiet NaN inputs, other than maximum and minimum
      // operations, if a floating-point result is to be delivered the result
      // shall be a quiet NaN which should be one of the input NaNs.
      // ...
      // Every general-computational and quiet-computational operation involving
      // one or more input NaNs, none of them signaling, shall signal no
      // exception, except fusedMultiplyAdd might signal the invalid operation
      // exception(see 7.2).
      ಪ್ರತ್ಯಾಗಮ opOK;
    }
  }

  ಯದಿ (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    ಪ್ರತ್ಯಾಗಮ opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  ಯದಿ (exponent + 1 >= (ಪೂರ್ಣಾಙ್ಕ)APFloat::semanticsPrecision(*semantics))
    ಪ್ರತ್ಯಾಗಮ opOK;

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
  opStatus fs = MagicConstant.convertFromAPInt(IntegerConstant, ಅಸತ್ಯ,
                                               rmNearestTiesToEven);
  ಪ್ರತಿಜ್ಞಾ(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  ತಾರ್ಕಿಕ ಒಳಹರಿವು_Sign = isNegative();

  fs = add(MagicConstant, rounding_mode);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  subtract(MagicConstant, rounding_mode);

  // Restore the input sign.
  ಯದಿ (ಒಳಹರಿವು_Sign != isNegative())
    changeSign();

  ಪ್ರತ್ಯಾಗಮ fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmpResult IEEEFloat::compare(ಧ್ರುವ IEEEFloat &rhs) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(semantics == rhs.semantics);

  ವಿಕಲ್ಪ (Pack_Categories_Into_ಕೀಲಿ(category, rhs.category)) {
  ಸ್ವಭಾವ:
    llvm_unreachable(nullptr);

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcZero):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNormal):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNaN, fcNaN):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNaN):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcNaN):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNaN):
    ಪ್ರತ್ಯಾಗಮ cmpUnordered;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcNormal):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcZero):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcZero):
    ಯದಿ (sign)
      ಪ್ರತ್ಯಾಗಮ cmpLessThan;
    ಅನ್ಯಥಾ
      ಪ್ರತ್ಯಾಗಮ cmpGreaterThan;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcInfinity):
  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcNormal):
    ಯದಿ (rhs.sign)
      ಪ್ರತ್ಯಾಗಮ cmpGreaterThan;
    ಅನ್ಯಥಾ
      ಪ್ರತ್ಯಾಗಮ cmpLessThan;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcInfinity, fcInfinity):
    ಯದಿ (sign == rhs.sign)
      ಪ್ರತ್ಯಾಗಮ cmpEqual;
    ಅನ್ಯಥಾ ಯದಿ (sign)
      ಪ್ರತ್ಯಾಗಮ cmpLessThan;
    ಅನ್ಯಥಾ
      ಪ್ರತ್ಯಾಗಮ cmpGreaterThan;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcZero, fcZero):
    ಪ್ರತ್ಯಾಗಮ cmpEqual;

  ಪಕ್ಷ Pack_Categories_Into_ಕೀಲಿ(fcNormal, fcNormal):
    ಭಙ್ಗ;
  }

  cmpResult result;
  /* Two normal numbers.  Do they have the same sign?  */
  ಯದಿ (sign != rhs.sign) {
    ಯದಿ (sign)
      result = cmpLessThan;
    ಅನ್ಯಥಾ
      result = cmpGreaterThan;
  } ಅನ್ಯಥಾ {
    /* Compare absolute values; invert result if negative.  */
    result = compare_Absolute_ಮೌಲ್ಯ(rhs);

    ಯದಿ (sign) {
      ಯದಿ (result == cmpLessThan)
        result = cmpGreaterThan;
      ಅನ್ಯಥಾ ಯದಿ (result == cmpGreaterThan)
        result = cmpLessThan;
    }
  }

  ಪ್ರತ್ಯಾಗಮ result;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::opStatus IEEEFloat::convert(ಧ್ರುವ fltSemantics &toSemantics,
                                     roundingMode rounding_mode,
                                     ತಾರ್ಕಿಕ *losesInfo) {
  opStatus fs;
  ಧ್ರುವ fltSemantics &fromSemantics = *semantics;
  ತಾರ್ಕಿಕ is_signaling = isSignaling();

  lostFraction lostFraction = lfExactlyZero;
  unsigned new_Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_Bits(toSemantics.precision + 1);
  unsigned old_Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ();
  ಪೂರ್ಣಾಙ್ಕ shift = toSemantics.precision - fromSemantics.precision;

  ತಾರ್ಕಿಕ X86SpecialNan = ಅಸತ್ಯ;
  ಯದಿ (&fromSemantics == &APFloatBase::semX87DoubleExtended &&
      &toSemantics != &APFloatBase::semX87DoubleExtended && category == fcNaN &&
      (!(*significandParts() & 0x8000000000000000ULL) ||
       !(*significandParts() & 0x4000000000000000ULL))) {
    // x86 has some unusual NaNs which cannot be represented in any other
    // format; note them here.
    X86SpecialNan = ಸತ್ಯ;
  }

  // If this is a truncation of a denormal number, and the target semantics
  // has larger exponent range than the source semantics (this can happen
  // when truncating from PowerPC double-double to double format), the
  // right shift could lose result mantissa bits.  Adjust exponent instead
  // of performing excessive shift.
  // Also do a similar trick in case shifting denormal would produce zero
  // significand as this case isn't handled correctly by normalize.
  ಯದಿ (shift < 0 && isFiniteNonZero()) {
    ಪೂರ್ಣಾಙ್ಕ omsb = significandMSB() + 1;
    ಪೂರ್ಣಾಙ್ಕ exponentChange = omsb - fromSemantics.precision;
    ಯದಿ (exponent + exponentChange < toSemantics.minExponent)
      exponentChange = toSemantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, shift);
    ಯದಿ (exponentChange < 0) {
      shift -= exponentChange;
      exponent += exponentChange;
    } ಅನ್ಯಥಾ ಯದಿ (omsb <= -shift) {
      exponentChange = omsb + shift - 1; // leave at least one bit set
      shift -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  ಯದಿ (shift < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    lostFraction = shift_ಬಲ(significandParts(), old_Part_ಎಣಿಕೆ, -shift);

  // Fix the storage so it can hold to new value.
  ಯದಿ (new_Part_ಎಣಿಕೆ > old_Part_ಎಣಿಕೆ) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = ನೂತನ integerPart[new_Part_ಎಣಿಕೆ];
    APInt::tcSet(newParts, 0, new_Part_ಎಣಿಕೆ);
    ಯದಿ (isFiniteNonZero() || category==fcNaN)
      APInt::tcAssign(newParts, significandParts(), old_Part_ಎಣಿಕೆ);
    freeSignificand();
    significand.parts = newParts;
  } ಅನ್ಯಥಾ ಯದಿ (new_Part_ಎಣಿಕೆ == 1 && old_Part_ಎಣಿಕೆ != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    ಯದಿ (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &toSemantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  ಯದಿ (shift > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_Shift_ಎಡ(significandParts(), new_Part_ಎಣಿಕೆ, shift);

  ಯದಿ (isFiniteNonZero()) {
    fs = normalize(rounding_mode, lostFraction);
    *losesInfo = (fs != opOK);
  } ಅನ್ಯಥಾ ಯದಿ (category == fcNaN) {
    ಯದಿ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(ಅಸತ್ಯ, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } ಅನ್ಯಥಾ {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      ಯದಿ (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(ಅಸತ್ಯ, ಅಸತ್ಯ);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      ಯದಿ (!APFloat::hasSignificand(fromSemantics))
        makeNaN(ಅಸತ್ಯ, sign);

      *losesInfo = lostFraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      ಯದಿ (!X86SpecialNan && semantics == &APFloatBase::semX87DoubleExtended)
        APInt::tcSetBit(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      ಯದಿ (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } ಅನ್ಯಥಾ {
        fs = opOK;
      }
    }
  } ಅನ್ಯಥಾ ಯದಿ (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(ಅಸತ್ಯ, sign);
    *losesInfo = ಸತ್ಯ;
    fs = opInexact;
  } ಅನ್ಯಥಾ ಯದಿ (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = ಅಸತ್ಯ;
  } ಅನ್ಯಥಾ {
    *losesInfo = ಅಸತ್ಯ;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  ಯದಿ ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = ಸತ್ಯ;
    ಯದಿ (fs == opOK)
      fs = opInexact;
  }

  ಯದಿ (category == fcZero && !semantics->hasZero)
    makeSmallestNormalized(ಅಸತ್ಯ);
  ಪ್ರತ್ಯಾಗಮ fs;
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
    MutableArrayRef<integerPart> parts, unsigned ಪೂರ್ಣಾಙ್ಕ width, ತಾರ್ಕಿಕ isSigned,
    roundingMode rounding_mode, ತಾರ್ಕಿಕ *isExact) ಧ್ರುವ {
  *isExact = ಅಸತ್ಯ;

  /* Handle the three special cases first.  */
  ಯದಿ (category == fcInfinity || category == fcNaN)
    ಪ್ರತ್ಯಾಗಮ opInvalidOp;

  unsigned dst_Parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_Bits(width);
  ಪ್ರತಿಜ್ಞಾ(dst_Parts_ಎಣಿಕೆ <= parts.size() && "Integer too big");

  ಯದಿ (category == fcZero) {
    APInt::tcSet(parts.data(), 0, dst_Parts_ಎಣಿಕೆ);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    ಪ್ರತ್ಯಾಗಮ opOK;
  }

  ಧ್ರುವ integerPart *src = significandParts();

  unsigned truncatedBits;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  ಯದಿ (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tcSet(parts.data(), 0, dst_Parts_ಎಣಿಕೆ);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncatedBits = semantics->precision -1U - exponent;
  } ಅನ್ಯಥಾ {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    unsigned ಪೂರ್ಣಾಙ್ಕ bits = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    ಯದಿ (bits > width)
      ಪ್ರತ್ಯಾಗಮ opInvalidOp;

    ಯದಿ (bits < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncatedBits = semantics->precision - bits;
      APInt::tcExtract(parts.data(), dst_Parts_ಎಣಿಕೆ, src, bits, truncatedBits);
    } ಅನ್ಯಥಾ {
      /* We want at least as many bits as are available.  */
      APInt::tcExtract(parts.data(), dst_Parts_ಎಣಿಕೆ, src, semantics->precision,
                       0);
      APInt::tc_Shift_ಎಡ(parts.data(), dst_Parts_ಎಣಿಕೆ,
                         bits - semantics->precision);
      truncatedBits = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  lostFraction lost_fraction;
  ಯದಿ (truncatedBits) {
    lost_fraction = lostFractionThroughTruncation(src, part_ಎಣಿಕೆ(),
                                                  truncatedBits);
    ಯದಿ (lost_fraction != lfExactlyZero &&
        roundAwayFromZero(rounding_mode, lost_fraction, truncatedBits)) {
      ಯದಿ (APInt::tcIncrement(parts.data(), dst_Parts_ಎಣಿಕೆ))
        ಪ್ರತ್ಯಾಗಮ opInvalidOp;     /* Overflow.  */
    }
  } ಅನ್ಯಥಾ {
    lost_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  unsigned ಪೂರ್ಣಾಙ್ಕ omsb = APInt::tcMSB(parts.data(), dst_Parts_ಎಣಿಕೆ) + 1;

  ಯದಿ (sign) {
    ಯದಿ (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      ಯದಿ (omsb != 0)
        ಪ್ರತ್ಯಾಗಮ opInvalidOp;
    } ಅನ್ಯಥಾ {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      ಯದಿ (omsb == width &&
          APInt::tcLSB(parts.data(), dst_Parts_ಎಣಿಕೆ) + 1 != omsb)
        ಪ್ರತ್ಯಾಗಮ opInvalidOp;

      /* This case can happen because of rounding.  */
      ಯದಿ (omsb > width)
        ಪ್ರತ್ಯಾಗಮ opInvalidOp;
    }

    APInt::tcNegate (parts.data(), dst_Parts_ಎಣಿಕೆ);
  } ಅನ್ಯಥಾ {
    ಯದಿ (omsb >= width + !isSigned)
      ಪ್ರತ್ಯಾಗಮ opInvalidOp;
  }

  ಯದಿ (lost_fraction == lfExactlyZero) {
    *isExact = ಸತ್ಯ;
    ಪ್ರತ್ಯಾಗಮ opOK;
  }
  ಪ್ರತ್ಯಾಗಮ opInexact;
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
                            unsigned ಪೂರ್ಣಾಙ್ಕ width, ತಾರ್ಕಿಕ isSigned,
                            roundingMode rounding_mode, ತಾರ್ಕಿಕ *isExact) ಧ್ರುವ {
  opStatus fs = convertToSignExtendedInteger(parts, width, isSigned,
                                             rounding_mode, isExact);

  ಯದಿ (fs == opInvalidOp) {
    unsigned ಪೂರ್ಣಾಙ್ಕ bits, dst_Parts_ಎಣಿಕೆ;

    dst_Parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_Bits(width);
    ಪ್ರತಿಜ್ಞಾ(dst_Parts_ಎಣಿಕೆ <= parts.size() && "Integer too big");

    ಯದಿ (category == fcNaN)
      bits = 0;
    ಅನ್ಯಥಾ ಯದಿ (sign)
      bits = isSigned;
    ಅನ್ಯಥಾ
      bits = width - isSigned;

    tcSetLeastSignificantBits(parts.data(), dst_Parts_ಎಣಿಕೆ, bits);
    ಯದಿ (sign && isSigned)
      APInt::tc_Shift_ಎಡ(parts.data(), dst_Parts_ಎಣಿಕೆ, width - 1);
  }

  ಪ್ರತ್ಯಾಗಮ fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::opStatus IEEEFloat::convertFromUnsignedParts(
    ಧ್ರುವ integerPart *src, unsigned ಪೂರ್ಣಾಙ್ಕ src_ಎಣಿಕೆ, roundingMode rounding_mode) {
  category = fcNormal;
  unsigned omsb = APInt::tcMSB(src, src_ಎಣಿಕೆ) + 1;
  integerPart *dst = significandParts();
  unsigned dst_ಎಣಿಕೆ = part_ಎಣಿಕೆ();
  unsigned precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  lostFraction lost_fraction;
  ಯದಿ (precision <= omsb) {
    exponent = omsb - 1;
    lost_fraction = lostFractionThroughTruncation(src, src_ಎಣಿಕೆ,
                                                  omsb - precision);
    APInt::tcExtract(dst, dst_ಎಣಿಕೆ, src, precision, omsb - precision);
  } ಅನ್ಯಥಾ {
    exponent = precision - 1;
    lost_fraction = lfExactlyZero;
    APInt::tcExtract(dst, dst_ಎಣಿಕೆ, src, omsb, 0);
  }

  ಪ್ರತ್ಯಾಗಮ normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus IEEEFloat::convertFromAPInt(ಧ್ರುವ APInt &Val, ತಾರ್ಕಿಕ isSigned,
                                              roundingMode rounding_mode) {
  unsigned ಪೂರ್ಣಾಙ್ಕ part_ಎಣಿಕೆ = Val.get_ಅಂಕೆ_ಪದಗಳು();
  APInt api = Val;

  sign = ಅಸತ್ಯ;
  ಯದಿ (isSigned && api.isNegative()) {
    sign = ಸತ್ಯ;
    api = -api;
  }

  ಪ್ರತ್ಯಾಗಮ convertFromUnsignedParts(api.getRawData(), part_ಎಣಿಕೆ, rounding_mode);
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        roundingMode rounding_mode) {
  lostFraction lost_fraction = lfExactlyZero;

  category = fcNormal;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  unsigned parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ();
  unsigned bitPos = parts_ಎಣಿಕೆ * integerPartWidth;
  ತಾರ್ಕಿಕ computedTrailingFraction = ಅಸತ್ಯ;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator end = s.end();
  StringRef::iterator dot;
  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  ಯದಿ (!PtrOrErr)
    ಪ್ರತ್ಯಾಗಮ PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator firstSignificantDigit = p;

  ಯಾವತ್ (p != end) {
    integerPart hex_ಮೌಲ್ಯ;

    ಯದಿ (*p == '.') {
      ಯದಿ (dot != end)
        ಪ್ರತ್ಯಾಗಮ createError("String contains multiple dots");
      dot = p++;
      ಅನುವರ್ತ;
    }

    hex_ಮೌಲ್ಯ = hex_Digit_ಮೌಲ್ಯ(*p);
    ಯದಿ (hex_ಮೌಲ್ಯ == UINT_MAX)
      ಭಙ್ಗ;

    p++;

    // Store the number while we have space.
    ಯದಿ (bitPos) {
      bitPos -= 4;
      hex_ಮೌಲ್ಯ <<= bitPos % integerPartWidth;
      significand[bitPos / integerPartWidth] |= hex_ಮೌಲ್ಯ;
    } ಅನ್ಯಥಾ ಯದಿ (!computedTrailingFraction) {
      auto FractOrErr = trailingHexadecimalFraction(p, end, hex_ಮೌಲ್ಯ);
      ಯದಿ (!FractOrErr)
        ಪ್ರತ್ಯಾಗಮ FractOrErr.takeError();
      lost_fraction = *FractOrErr;
      computedTrailingFraction = ಸತ್ಯ;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  ಯದಿ (p == end)
    ಪ್ರತ್ಯಾಗಮ createError("Hex strings require an exponent");
  ಯದಿ (*p != 'p' && *p != 'P')
    ಪ್ರತ್ಯಾಗಮ createError("Invalid character in significand");
  ಯದಿ (p == begin)
    ಪ್ರತ್ಯಾಗಮ createError("Significand has no digits");
  ಯದಿ (dot != end && p - begin == 1)
    ಪ್ರತ್ಯಾಗಮ createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  ಯದಿ (p != firstSignificantDigit) {
    ಪೂರ್ಣಾಙ್ಕ expAdjustment;

    /* Implicit hexadecimal point?  */
    ಯದಿ (dot == end)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<ಪೂರ್ಣಾಙ್ಕ>(dot - firstSignificantDigit);
    ಯದಿ (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_ಎಣಿಕೆ * integerPartWidth;

    /* Adjust for the given exponent.  */
    auto ExpOrErr = totalExponent(p + 1, end, expAdjustment);
    ಯದಿ (!ExpOrErr)
      ಪ್ರತ್ಯಾಗಮ ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  ಪ್ರತ್ಯಾಗಮ normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus
IEEEFloat::roundSignificandWithExponent(ಧ್ರುವ integerPart *decSigParts,
                                        unsigned sig_Part_ಎಣಿಕೆ, ಪೂರ್ಣಾಙ್ಕ exp,
                                        roundingMode rounding_mode) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  unsigned ಪೂರ್ಣಾಙ್ಕ power = exp >= 0 ? exp : -exp;
  unsigned ಪೂರ್ಣಾಙ್ಕ pow5PartCapacity = powerOfFivePartsForExponent(power);
  SmallVector<integerPart, ಅಂಕೆ_Pow5_Parts_On_ರಾಶಿ> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5PartCapacity);

  ತಾರ್ಕಿಕ isNearest = rounding_mode == rmNearestTiesToEven ||
                   rounding_mode == rmNearestTiesToAway;

  unsigned parts = part_ಎಣಿಕೆ_For_Bits(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  unsigned pow5_Part_ಎಣಿಕೆ = powerOf5(pow5Parts.data(), power);

  ಪ್ರತಿ (;; parts *= 2) {
    unsigned ಪೂರ್ಣಾಙ್ಕ excessPrecision, truncatedBits;

    calcSemantics.precision = parts * integerPartWidth - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncatedBits = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    opStatus sigStatus = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_ಎಣಿಕೆ, rmNearestTiesToEven);
    opStatus powStatus = pow5.convertFromUnsignedParts(
        pow5Parts.data(), pow5_Part_ಎಣಿಕೆ, rmNearestTiesToEven);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += exp;

    lostFraction calcLostFraction;
    integerPart HUerr, HUdistance;
    unsigned ಪೂರ್ಣಾಙ್ಕ powHUerr;

    ಯದಿ (exp >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calcLostFraction = decSig.multiplySignificand(pow5);
      powHUerr = powStatus != opOK;
    } ಅನ್ಯಥಾ {
      calcLostFraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      ಯದಿ (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncatedBits = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (powStatus == opOK && calcLostFraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    ಪ್ರತಿಜ್ಞಾ(APInt::tcExtractBit
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calcLostFraction != lfExactlyZero, sigStatus != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, isNearest);

    /* Are we guaranteed to round correctly if we truncate?  */
    ಯದಿ (HUdistance >= HUerr) {
      APInt::tcExtract(significandParts(), part_ಎಣಿಕೆ(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calcLostFraction = lostFractionThroughTruncation(decSig.significandParts(),
                                                       decSig.part_ಎಣಿಕೆ(),
                                                       truncatedBits);
      ಪ್ರತ್ಯಾಗಮ static_cast<opStatus>(normalize(rounding_mode, calcLostFraction) |
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
  ಯದಿ (Error Err = interpretDecimal(p, str.end(), &D))
    ಪ್ರತ್ಯಾಗಮ std::move(Err);

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
  ಯದಿ (D.firstSigDigit == str.end() || dec_Digit_ಮೌಲ್ಯ(*D.firstSigDigit) >= 10U) {
    category = fcZero;
    fs = opOK;
    ಯದಿ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = ಅಸತ್ಯ;
    ಯದಿ (!semantics->hasZero)
      makeSmallestNormalized(ಅಸತ್ಯ);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } ಅನ್ಯಥಾ ಯದಿ (D.normalizedExponent - 1 > INT_MAX / 42039) {
    fs = handleOverflow(rounding_mode);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } ಅನ್ಯಥಾ ಯದಿ (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (ಪೂರ್ಣಾಙ್ಕ) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fcNormal;
    zeroSignificand();
    fs = normalize(rounding_mode, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } ಅನ್ಯಥಾ ಯದಿ ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handleOverflow(rounding_mode);
  } ಅನ್ಯಥಾ {
    integerPart *decSignificand;
    unsigned ಪೂರ್ಣಾಙ್ಕ part_ಎಣಿಕೆ;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_ಎಣಿಕೆ = static_cast<unsigned ಪೂರ್ಣಾಙ್ಕ>(D.lastSigDigit - D.firstSigDigit) + 1;
    part_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_Bits(1 + 196 * part_ಎಣಿಕೆ / 59);
    decSignificand = ನೂತನ integerPart[part_ಎಣಿಕೆ + 1];
    part_ಎಣಿಕೆ = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    ಕುರು {
      integerPart dec_ಮೌಲ್ಯ, val, multiplier;

      val = 0;
      multiplier = 1;

      ಕುರು {
        ಯದಿ (*p == '.') {
          p++;
          ಯದಿ (p == str.end()) {
            ಭಙ್ಗ;
          }
        }
        dec_ಮೌಲ್ಯ = dec_Digit_ಮೌಲ್ಯ(*p++);
        ಯದಿ (dec_ಮೌಲ್ಯ >= 10U) {
          delete[] decSignificand;
          ಪ್ರತ್ಯಾಗಮ createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_ಮೌಲ್ಯ;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } ಯಾವತ್ (p <= D.lastSigDigit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_ಎಣಿಕೆ, part_ಎಣಿಕೆ + 1, ಅಸತ್ಯ);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      ಯದಿ (decSignificand[part_ಎಣಿಕೆ])
        part_ಎಣಿಕೆ++;
    } ಯಾವತ್ (p <= D.lastSigDigit);

    category = fcNormal;
    fs = roundSignificandWithExponent(decSignificand, part_ಎಣಿಕೆ,
                                      D.exponent, rounding_mode);

    delete [] decSignificand;
  }

  ಪ್ರತ್ಯಾಗಮ fs;
}

ತಾರ್ಕಿಕ IEEEFloat::convertFromStringSpecials(StringRef str) {
  ಧ್ರುವ size_t MIN_NAME_SIZE = 3;

  ಯದಿ (str.size() < MIN_NAME_SIZE)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಯದಿ (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(ಅಸತ್ಯ);
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

  ತಾರ್ಕಿಕ IsNegative = str.consume_front("-");
  ಯದಿ (IsNegative) {
    ಯದಿ (str.size() < MIN_NAME_SIZE)
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

    ಯದಿ (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(ಸತ್ಯ);
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  ತಾರ್ಕಿಕ IsSignaling = str.consume_front_insensitive("s");
  ಯದಿ (IsSignaling) {
    ಯದಿ (str.size() < MIN_NAME_SIZE)
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  }

  ಯದಿ (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    ಯದಿ (str.empty()) {
      makeNaN(IsSignaling, IsNegative);
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }

    // Allow the payload to be inside parentheses.
    ಯದಿ (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      ಯದಿ (str.size() <= 2 || str.back() != ')')
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

      str = str.slice(1, str.size() - 1);
    }

    // Determine the payload number's radix.
    unsigned Radix = 10;
    ಯದಿ (str[0] == '0') {
      ಯದಿ (str.size() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } ಅನ್ಯಥಾ {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    ಯದಿ (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }
  }

  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromString(StringRef str, roundingMode rounding_mode) {
  ಯದಿ (str.empty())
    ಪ್ರತ್ಯಾಗಮ createError("Invalid string length");

  // Handle special cases.
  ಯದಿ (convertFromStringSpecials(str))
    ಪ್ರತ್ಯಾಗಮ opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  sign = *p == '-' ? 1 : 0;
  ಯದಿ (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  ಯದಿ (*p == '-' || *p == '+') {
    p++;
    slen--;
    ಯದಿ (!slen)
      ಪ್ರತ್ಯಾಗಮ createError("String has no digits");
  }

  ಯದಿ (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    ಯದಿ (slen == 2)
      ಪ್ರತ್ಯಾಗಮ createError("Invalid string");
    ಪ್ರತ್ಯಾಗಮ convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_mode);
  }

  ಪ್ರತ್ಯಾಗಮ convertFromDecimalString(StringRef(p, slen), rounding_mode);
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
unsigned ಪೂರ್ಣಾಙ್ಕ IEEEFloat::convertToHexString(ವರ್ಣ *dst, unsigned ಪೂರ್ಣಾಙ್ಕ hexDigits,
                                           ತಾರ್ಕಿಕ upperCase,
                                           roundingMode rounding_mode) ಧ್ರುವ {
  ವರ್ಣ *p = dst;
  ಯದಿ (sign)
    *dst++ = '-';

  ವಿಕಲ್ಪ (category) {
  ಪಕ್ಷ fcInfinity:
    memcpy (dst, upperCase ? infinityU: infinityL, ಪರಿಮಾಣ infinityU - 1);
    dst += ಪರಿಮಾಣ infinityL - 1;
    ಭಙ್ಗ;

  ಪಕ್ಷ fcNaN:
    memcpy (dst, upperCase ? NaNU: NaNL, ಪರಿಮಾಣ NaNU - 1);
    dst += ಪರಿಮಾಣ NaNU - 1;
    ಭಙ್ಗ;

  ಪಕ್ಷ fcZero:
    *dst++ = '0';
    *dst++ = upperCase ? 'X': 'x';
    *dst++ = '0';
    ಯದಿ (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = upperCase ? 'P': 'p';
    *dst++ = '0';
    ಭಙ್ಗ;

  ಪಕ್ಷ fcNormal:
    dst = convertNormalToHexString (dst, hexDigits, upperCase, rounding_mode);
    ಭಙ್ಗ;
  }

  *dst = 0;

  ಪ್ರತ್ಯಾಗಮ static_cast<unsigned ಪೂರ್ಣಾಙ್ಕ>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
ವರ್ಣ *IEEEFloat::convertNormalToHexString(ವರ್ಣ *dst, unsigned ಪೂರ್ಣಾಙ್ಕ hexDigits,
                                          ತಾರ್ಕಿಕ upperCase,
                                          roundingMode rounding_mode) ಧ್ರುವ {
  *dst++ = '0';
  *dst++ = upperCase ? 'X': 'x';

  ತಾರ್ಕಿಕ roundUp = ಅಸತ್ಯ;
  ಧ್ರುವ ವರ್ಣ *hexDigitChars = upperCase ? hexDigitsUpper : hexDigitsLower;

  ಧ್ರುವ integerPart *significand = significandParts();
  unsigned parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  unsigned ಮೌಲ್ಯ_Bits = semantics->precision + 3;
  unsigned shift = integerPartWidth - ಮೌಲ್ಯ_Bits % integerPartWidth;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  unsigned outputDigits = (ಮೌಲ್ಯ_Bits - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  ಯದಿ (hexDigits) {
    ಯದಿ (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      unsigned ಪೂರ್ಣಾಙ್ಕ bits;
      lostFraction fraction;

      bits = ಮೌಲ್ಯ_Bits - hexDigits * 4;
      fraction = lostFractionThroughTruncation (significand, parts_ಎಣಿಕೆ, bits);
      roundUp = roundAwayFromZero(rounding_mode, fraction, bits);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  ವರ್ಣ *p = ++dst;

  unsigned ಎಣಿಕೆ = (ಮೌಲ್ಯ_Bits + integerPartWidth - 1) / integerPartWidth;

  ಯಾವತ್ (outputDigits && ಎಣಿಕೆ) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    ಯದಿ (--ಎಣಿಕೆ == parts_ಎಣಿಕೆ)
      part = 0;  /* An imaginary higher zero part.  */
    ಅನ್ಯಥಾ
      part = significand[ಎಣಿಕೆ] << shift;

    ಯದಿ (ಎಣಿಕೆ && shift)
      part |= significand[ಎಣಿಕೆ - 1] >> (integerPartWidth - shift);

    /* Convert as much of "part" to hexdigits as we can.  */
    unsigned ಪೂರ್ಣಾಙ್ಕ curDigits = integerPartWidth / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  ಯದಿ (roundUp) {
    ವರ್ಣ *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    ಕುರು {
      q--;
      *q = hexDigitChars[hex_Digit_ಮೌಲ್ಯ (*q) + 1];
    } ಯಾವತ್ (*q == '0');
    ಪ್ರತಿಜ್ಞಾ(q >= p);
  } ಅನ್ಯಥಾ {
    /* Add trailing zeroes.  */
    memset (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  ಯದಿ (dst -1 == p)
    dst--;
  ಅನ್ಯಥಾ
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = upperCase ? 'P': 'p';

  ಪ್ರತ್ಯಾಗಮ writeSignedDecimal (dst, exponent);
}

hash_code hash_ಮೌಲ್ಯ(ಧ್ರುವ IEEEFloat &Arg) {
  ಯದಿ (!Arg.isFiniteNonZero())
    ಪ್ರತ್ಯಾಗಮ hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  ಪ್ರತ್ಯಾಗಮ hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.part_ಎಣಿಕೆ()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convertF80LongDoubleAPFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 2);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semX87DoubleExtended>();
}

APInt IEEEFloat::convertPPCDoubleDoubleLegacyAPFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(semantics ==
         (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy);
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ()==2);

  uint64_t ಪದಗಳು[2];
  ತಾರ್ಕಿಕ losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloatBase::semIEEEdouble.minExponent;
  IEEEFloat extended(*ಏತತ್);
  [[maybe_unused]] opStatus fs =
      extended.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  ಪ್ರತಿಜ್ಞಾ(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
  ಪ್ರತಿಜ್ಞಾ(fs == opOK || fs == opInexact);
  ಪದಗಳು[0] = *u.convertDoubleAPFloatToAPInt().getRawData();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  ಯದಿ (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
    ಪ್ರತಿಜ್ಞಾ(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.subtract(u, rmNearestTiesToEven);
    fs = v.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
    ಪ್ರತಿಜ್ಞಾ(fs == opOK && !losesInfo);
    ಪದಗಳು[1] = *v.convertDoubleAPFloatToAPInt().getRawData();
  } ಅನ್ಯಥಾ {
    ಪದಗಳು[1] = 0;
  }

  ಪ್ರತ್ಯಾಗಮ APInt(128, ಪದಗಳು);
}

template <ಧ್ರುವ fltSemantics &S>
APInt IEEEFloat::convertIEEEFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(semantics == &S);
  constexpr unsigned ಪೂರ್ಣಾಙ್ಕ trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr ಪೂರ್ಣಾಙ್ಕ integer_bit_part = (S.precision - 1) / integerPartWidth;
  constexpr integerPart integer_bit = integerPart{1}
                                      << ((S.precision - 1) % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned ಪೂರ್ಣಾಙ್ಕ exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr ತಾರ್ಕಿಕ is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr ಪೂರ್ಣಾಙ್ಕ bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_ಎಣಿಕೆ_For_Bits(trailing_significand_bits)>
      mysignificand;

  ಯದಿ (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
    ಯದಿ (myexponent == 1 &&
        !(significandParts()[integer_bit_part] & integer_bit))
      myexponent = 0; // denormal
  } ಅನ್ಯಥಾ ಯದಿ (category == fcZero) {
    ಯದಿ (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } ಅನ್ಯಥಾ ಯದಿ (category == fcInfinity) {
    ಯದಿ (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    ಯದಿ constexpr (S.hasExplicitIntegerBit) {
      mysignificand[0] = integerPart{1} << (trailing_significand_bits - 1);
    }
  } ಅನ್ಯಥಾ {
    ಪ್ರತಿಜ್ಞಾ(category == fcNaN && "Unknown category!");
    ಯದಿ (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.sizeInBits + 63) / 64> ಪದಗಳು;
  auto ಪದಗಳು_iter =
      std::copy_n(mysignificand.begin(), mysignificand.size(), ಪದಗಳು.begin());
  ಯದಿ constexpr (!S.hasExplicitIntegerBit) {
    ಯದಿ constexpr (significand_mask != 0 || trailing_significand_bits == 0) {
      // Clear the integer bit.
      ಪದಗಳು[mysignificand.size() - 1] &= significand_mask;
    }
  }
  std::fill(ಪದಗಳು_iter, ಪದಗಳು.end(), uint64_t{0});
  constexpr size_t last_word = ಪದಗಳು.size() - 1;
  ಯದಿ constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.sizeInBits - 1) % 64);
    ಪದಗಳು[last_word] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_mask)
                              << (trailing_significand_bits % 64);
  ಪದಗಳು[last_word] |= shifted_exponent;
  ಯದಿ constexpr (last_word == 0) {
    ಪ್ರತ್ಯಾಗಮ APInt(S.sizeInBits, ಪದಗಳು[0]);
  }
  ಪ್ರತ್ಯಾಗಮ APInt(S.sizeInBits, ಪದಗಳು);
}

APInt IEEEFloat::convertQuadrupleAPFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 2);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semIEEEquad>();
}

APInt IEEEFloat::convertDoubleAPFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ()==1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semIEEEdouble>();
}

APInt IEEEFloat::convertFloatAPFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ()==1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semIEEEsingle>();
}

APInt IEEEFloat::convertBFloatAPFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semBFloat>();
}

APInt IEEEFloat::convertHalfAPFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ()==1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::APFloatBase::semIEEEhalf>();
}

APInt IEEEFloat::convertFloat8E5M2APFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2>();
}

APInt IEEEFloat::convertFloat8E5M2FNUZAPFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3APFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3>();
}

APInt IEEEFloat::convertFloat8E4M3FNAPFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FN>();
}

APInt IEEEFloat::convertFloat8E4M3FNUZAPFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3B11FNUZAPFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convertFloat8E3M4APFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semFloat8E3M4>();
}

APInt IEEEFloat::convertFloatTF32APFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semFloatTF32>();
}

APInt IEEEFloat::convertFloat8E8M0FNUAPFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convertFloat8E5M3FNUAPFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convertFloat6E3M2FNAPFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semFloat6E3M2FN>();
}

APInt IEEEFloat::convertFloat6E2M3FNAPFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semFloat6E2M3FN>();
}

APInt IEEEFloat::convertFloat4E2M1FNAPFloatToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == 1);
  ಪ್ರತ್ಯಾಗಮ convertIEEEFloatToAPInt<APFloatBase::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcastToAPInt() ಧ್ರುವ {
  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semIEEEhalf)
    ಪ್ರತ್ಯಾಗಮ convertHalfAPFloatToAPInt();

  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semBFloat)
    ಪ್ರತ್ಯಾಗಮ convertBFloatAPFloatToAPInt();

  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semIEEEsingle)
    ಪ್ರತ್ಯಾಗಮ convertFloatAPFloatToAPInt();

  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semIEEEdouble)
    ಪ್ರತ್ಯಾಗಮ convertDoubleAPFloatToAPInt();

  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semIEEEquad)
    ಪ್ರತ್ಯಾಗಮ convertQuadrupleAPFloatToAPInt();

  ಯದಿ (semantics ==
      (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semX87DoubleExtended)
    ಪ್ರತ್ಯಾಗಮ convertF80LongDoubleAPFloatToAPInt();

  ಯದಿ (semantics ==
      (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy)
    ಪ್ರತ್ಯಾಗಮ convertPPCDoubleDoubleLegacyAPFloatToAPInt();

  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semFloat8E5M2)
    ಪ್ರತ್ಯಾಗಮ convertFloat8E5M2APFloatToAPInt();

  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semFloat8E5M2FNUZ)
    ಪ್ರತ್ಯಾಗಮ convertFloat8E5M2FNUZAPFloatToAPInt();

  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semFloat8E4M3)
    ಪ್ರತ್ಯಾಗಮ convertFloat8E4M3APFloatToAPInt();

  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FN)
    ಪ್ರತ್ಯಾಗಮ convertFloat8E4M3FNAPFloatToAPInt();

  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FNUZ)
    ಪ್ರತ್ಯಾಗಮ convertFloat8E4M3FNUZAPFloatToAPInt();

  ಯದಿ (semantics ==
      (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semFloat8E4M3B11FNUZ)
    ಪ್ರತ್ಯಾಗಮ convertFloat8E4M3B11FNUZAPFloatToAPInt();

  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semFloat8E3M4)
    ಪ್ರತ್ಯಾಗಮ convertFloat8E3M4APFloatToAPInt();

  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semFloatTF32)
    ಪ್ರತ್ಯಾಗಮ convertFloatTF32APFloatToAPInt();

  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semFloat8E8M0FNU)
    ಪ್ರತ್ಯಾಗಮ convertFloat8E8M0FNUAPFloatToAPInt();

  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semFloat8E5M3FNU)
    ಪ್ರತ್ಯಾಗಮ convertFloat8E5M3FNUAPFloatToAPInt();

  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semFloat6E3M2FN)
    ಪ್ರತ್ಯಾಗಮ convertFloat6E3M2FNAPFloatToAPInt();

  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semFloat6E2M3FN)
    ಪ್ರತ್ಯಾಗಮ convertFloat6E2M3FNAPFloatToAPInt();

  ಯದಿ (semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semFloat4E2M1FN)
    ಪ್ರತ್ಯಾಗಮ convertFloat4E2M1FNAPFloatToAPInt();

  llvm_unreachable("unknown format!");
}

ಭಿನ್ನ IEEEFloat::convertToFloat() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcastToAPInt();
  ಪ್ರತ್ಯಾಗಮ api.bitsToFloat();
}

ದ್ವಿಗುಣ IEEEFloat::convertToDouble() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcastToAPInt();
  ಪ್ರತ್ಯಾಗಮ api.bitsToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 IEEEFloat::convertToQuad() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(semantics == (ಧ್ರುವ llvm::fltSemantics *)&APFloatBase::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcastToAPInt();
  ಪ್ರತ್ಯಾಗಮ api.bitsToQuad();
}
#endif

ಶೂನ್ಯ IEEEFloat::initFromF80LongDoubleAPInt(ಧ್ರುವ APInt &api) {
  ಪ್ರತ್ಯಾಗಮ initFromIEEEAPInt<APFloatBase::semX87DoubleExtended>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(ಧ್ರುವ APInt &api) {
  uint64_t i1 = api.getRawData()[0];
  uint64_t i2 = api.getRawData()[1];
  ತಾರ್ಕಿಕ losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_unused]] opStatus fs = convert(APFloatBase::semPPCDoubleDoubleLegacy,
                                         rmNearestTiesToEven, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  ಪ್ರತಿಜ್ಞಾ((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  ಯದಿ (isFiniteNonZero()) {
    IEEEFloat v(APFloatBase::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    ಯದಿ (v.isFinite()) {
      fs = v.convert(APFloatBase::semPPCDoubleDoubleLegacy, rmNearestTiesToEven,
                     &losesInfo);
      ಪ್ರತಿಜ್ಞಾ(fs == opOK && !losesInfo);

      add(v, rmNearestTiesToEven);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
ಶೂನ್ಯ IEEEFloat::initFromFloat8E8M0FNUAPInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E8M0FNU>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat8E5M3FNUAPInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M3FNU>(api);
}

template <ಧ್ರುವ fltSemantics &S>
ಶೂನ್ಯ IEEEFloat::initFromIEEEAPInt(ಧ್ರುವ APInt &api) {
  ಪ್ರತಿಜ್ಞಾ(api.getBitWidth() == S.sizeInBits);

  constexpr unsigned ಪೂರ್ಣಾಙ್ಕ trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr integerPart integer_bit =
      integerPart{1} << (trailing_significand_bits % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned ಪೂರ್ಣಾಙ್ಕ exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr unsigned ಪೂರ್ಣಾಙ್ಕ stored_significand_parts =
      part_ಎಣಿಕೆ_For_Bits(trailing_significand_bits + 1);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr ತಾರ್ಕಿಕ is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr ಪೂರ್ಣಾಙ್ಕ bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr ತಾರ್ಕಿಕ has_significand = trailing_significand_bits > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  ಯದಿ constexpr (has_significand) {
    std::copy_n(api.getRawData(), mysignificand.size(), mysignificand.begin());
    ಯದಿ constexpr (significand_mask != 0 || S.precision >= integerPartWidth) {
      mysignificand[mysignificand.size() - 1] &= significand_mask;
    }
  } ಅನ್ಯಥಾ {
    std::fill_n(mysignificand.begin(), mysignificand.size(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t last_word = api.getRawData()[api.get_ಅಂಕೆ_ಪದಗಳು() - 1];
  uint64_t myexponent =
      (last_word >> (trailing_significand_bits % 64)) & exponent_mask;

  initialize(&S);
  ಪ್ರತಿಜ್ಞಾ(part_ಎಣಿಕೆ() == mysignificand.size());

  sign = S.hasSignedRepr
             ? static_cast<unsigned ಪೂರ್ಣಾಙ್ಕ>(last_word >> ((S.sizeInBits - 1) % 64))
             : 0;

  ತಾರ್ಕಿಕ all_zero_significand =
      has_significand && llvm::all_of(mysignificand, equal_to(0));

  ತಾರ್ಕಿಕ is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  ಯದಿ constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    ತಾರ್ಕಿಕ is_inf = ಅಸತ್ಯ;

    ಯದಿ constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;
      ಧ್ರುವ integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_mask_no_int_bit) == 0;
    } ಅನ್ಯಥಾ {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    ಯದಿ (is_inf) {
      makeInf(sign);
      ಪ್ರತ್ಯಾಗಮ;
    }
  }

  ತಾರ್ಕಿಕ is_nan = ಅಸತ್ಯ;

  ಯದಿ constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    ಯದಿ constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      ಧ್ರುವ integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;

      ಯದಿ (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_mask_no_int_bit) != 0) {
        // regular NaN and pseudoNaN
        is_nan = ಸತ್ಯ;
      } ಅನ್ಯಥಾ ಯದಿ (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_mask_no_int_bit) == 0) {
        // pseudoinfinity
        is_nan = ಸತ್ಯ;
      } ಅನ್ಯಥಾ ಯದಿ (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = ಸತ್ಯ;
      }
    } ಅನ್ಯಥಾ {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } ಅನ್ಯಥಾ ಯದಿ constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    ತಾರ್ಕಿಕ all_ones_significand =
        std::all_of(mysignificand.begin(), mysignificand.end() - 1,
                    [](integerPart bits) { ಪ್ರತ್ಯಾಗಮ bits == ~integerPart{0}; }) &&
        (!significand_mask ||
         mysignificand[mysignificand.size() - 1] == significand_mask);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } ಅನ್ಯಥಾ ಯದಿ constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  ಯದಿ (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.size(),
                significandParts());
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಯದಿ (is_zero) {
    makeZero(sign);
    ಪ್ರತ್ಯಾಗಮ;
  }

  category = fcNormal;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.size(), significandParts());
  ಯದಿ (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  ಅನ್ಯಥಾ {
    ಯದಿ constexpr (!S.hasExplicitIntegerBit) {
      significandParts()[mysignificand.size() - 1] |= integer_bit;
    }
  }
}

ಶೂನ್ಯ IEEEFloat::initFromQuadrupleAPInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEquad>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromDoubleAPInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEdouble>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloatAPInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEsingle>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromBFloatAPInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semBFloat>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromHalfAPInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEhalf>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat8E5M2APInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat8E5M2FNUZAPInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2FNUZ>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat8E4M3APInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat8E4M3FNAPInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FN>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat8E4M3FNUZAPInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FNUZ>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3B11FNUZ>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat8E3M4APInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E3M4>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloatTF32APInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloatTF32>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat6E3M2FNAPInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E3M2FN>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat6E2M3FNAPInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E2M3FN>(api);
}

ಶೂನ್ಯ IEEEFloat::initFromFloat4E2M1FNAPInt(ಧ್ರುವ APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
ಶೂನ್ಯ IEEEFloat::initFromAPInt(ಧ್ರುವ fltSemantics *Sem, ಧ್ರುವ APInt &api) {
  ಪ್ರತಿಜ್ಞಾ(api.getBitWidth() == Sem->sizeInBits);
  ಯದಿ (Sem == &APFloatBase::semIEEEhalf)
    ಪ್ರತ್ಯಾಗಮ initFromHalfAPInt(api);
  ಯದಿ (Sem == &APFloatBase::semBFloat)
    ಪ್ರತ್ಯಾಗಮ initFromBFloatAPInt(api);
  ಯದಿ (Sem == &APFloatBase::semIEEEsingle)
    ಪ್ರತ್ಯಾಗಮ initFromFloatAPInt(api);
  ಯದಿ (Sem == &APFloatBase::semIEEEdouble)
    ಪ್ರತ್ಯಾಗಮ initFromDoubleAPInt(api);
  ಯದಿ (Sem == &APFloatBase::semIEEEquad)
    ಪ್ರತ್ಯಾಗಮ initFromQuadrupleAPInt(api);
  ಯದಿ (Sem == &APFloatBase::semX87DoubleExtended)
    ಪ್ರತ್ಯಾಗಮ initFromF80LongDoubleAPInt(api);
  ಯದಿ (Sem == &APFloatBase::semPPCDoubleDoubleLegacy)
    ಪ್ರತ್ಯಾಗಮ initFromPPCDoubleDoubleLegacyAPInt(api);
  ಯದಿ (Sem == &APFloatBase::semFloat8E5M2)
    ಪ್ರತ್ಯಾಗಮ initFromFloat8E5M2APInt(api);
  ಯದಿ (Sem == &APFloatBase::semFloat8E5M2FNUZ)
    ಪ್ರತ್ಯಾಗಮ initFromFloat8E5M2FNUZAPInt(api);
  ಯದಿ (Sem == &APFloatBase::semFloat8E4M3)
    ಪ್ರತ್ಯಾಗಮ initFromFloat8E4M3APInt(api);
  ಯದಿ (Sem == &APFloatBase::semFloat8E4M3FN)
    ಪ್ರತ್ಯಾಗಮ initFromFloat8E4M3FNAPInt(api);
  ಯದಿ (Sem == &APFloatBase::semFloat8E4M3FNUZ)
    ಪ್ರತ್ಯಾಗಮ initFromFloat8E4M3FNUZAPInt(api);
  ಯದಿ (Sem == &APFloatBase::semFloat8E4M3B11FNUZ)
    ಪ್ರತ್ಯಾಗಮ initFromFloat8E4M3B11FNUZAPInt(api);
  ಯದಿ (Sem == &APFloatBase::semFloat8E3M4)
    ಪ್ರತ್ಯಾಗಮ initFromFloat8E3M4APInt(api);
  ಯದಿ (Sem == &APFloatBase::semFloatTF32)
    ಪ್ರತ್ಯಾಗಮ initFromFloatTF32APInt(api);
  ಯದಿ (Sem == &APFloatBase::semFloat8E8M0FNU)
    ಪ್ರತ್ಯಾಗಮ initFromFloat8E8M0FNUAPInt(api);
  ಯದಿ (Sem == &APFloatBase::semFloat8E5M3FNU)
    ಪ್ರತ್ಯಾಗಮ initFromFloat8E5M3FNUAPInt(api);
  ಯದಿ (Sem == &APFloatBase::semFloat6E3M2FN)
    ಪ್ರತ್ಯಾಗಮ initFromFloat6E3M2FNAPInt(api);
  ಯದಿ (Sem == &APFloatBase::semFloat6E2M3FN)
    ಪ್ರತ್ಯಾಗಮ initFromFloat6E2M3FNAPInt(api);
  ಯದಿ (Sem == &APFloatBase::semFloat4E2M1FN)
    ಪ್ರತ್ಯಾಗಮ initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
ಶೂನ್ಯ IEEEFloat::makeLargest(ತಾರ್ಕಿಕ Negative) {
  ಯದಿ (Negative && !semantics->hasSignedRepr)
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
  unsigned Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ();
  memset(significand, 0xFF, ಪರಿಮಾಣ(integerPart)*(Part_ಎಣಿಕೆ - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  ಧ್ರುವ unsigned ಅಂಕೆ_Unused_ಮೇಲಿನ_Bits =
    Part_ಎಣಿಕೆ*integerPartWidth - semantics->precision;
  significand[Part_ಎಣಿಕೆ - 1] = (ಅಂಕೆ_Unused_ಮೇಲಿನ_Bits < integerPartWidth)
                                   ? (~integerPart(0) >> ಅಂಕೆ_Unused_ಮೇಲಿನ_Bits)
                                   : 0;
  ಯದಿ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
ಶೂನ್ಯ IEEEFloat::makeSmallest(ತಾರ್ಕಿಕ Negative) {
  ಯದಿ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fcNormal;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tcSet(significandParts(), 1, part_ಎಣಿಕೆ());
}

ಶೂನ್ಯ IEEEFloat::makeSmallestNormalized(ತಾರ್ಕಿಕ Negative) {
  ಯದಿ (Negative && !semantics->hasSignedRepr)
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

IEEEFloat::IEEEFloat(ಧ್ರುವ fltSemantics &Sem, ಧ್ರುವ APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(ಭಿನ್ನ f) {
  initFromAPInt(&APFloatBase::semIEEEsingle, APInt::floatToBits(f));
}

IEEEFloat::IEEEFloat(ದ್ವಿಗುಣ d) {
  initFromAPInt(&APFloatBase::semIEEEdouble, APInt::doubleToBits(d));
}

ನಾಮಾಕಾಶ {
  ಶೂನ್ಯ append(SmallVectorImpl<ವರ್ಣ> &Buffer, StringRef Str) {
    Buffer.append(Str.begin(), Str.end());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  ಶೂನ್ಯ AdjustToPrecision(APInt &significand,
                         ಪೂರ್ಣಾಙ್ಕ &exp, unsigned FormatPrecision) {
    unsigned bits = significand.getActiveBits();

    // 196/59 is a very slight overestimate of lg_2(10).
    unsigned bitsRequired = (FormatPrecision * 196 + 58) / 59;

    ಯದಿ (bits <= bitsRequired) ಪ್ರತ್ಯಾಗಮ;

    unsigned tensRemovable = (bits - bitsRequired) * 59 / 196;
    ಯದಿ (!tensRemovable) ಪ್ರತ್ಯಾಗಮ;

    exp += tensRemovable;

    APInt ಭಾಜಕ(significand.getBitWidth(), 1);
    APInt powten(significand.getBitWidth(), 10);
    ಯಾವತ್ (ಸತ್ಯ) {
      ಯದಿ (tensRemovable & 1)
        ಭಾಜಕ *= powten;
      tensRemovable >>= 1;
      ಯದಿ (!tensRemovable) ಭಙ್ಗ;
      powten *= powten;
    }

    significand = significand.udiv(ಭಾಜಕ);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.getActiveBits());
  }


  ಶೂನ್ಯ AdjustToPrecision(SmallVectorImpl<ವರ್ಣ> &buffer,
                         ಪೂರ್ಣಾಙ್ಕ &exp, unsigned FormatPrecision) {
    unsigned N = buffer.size();
    ಯದಿ (N <= FormatPrecision) ಪ್ರತ್ಯಾಗಮ;

    // The most significant figures are the last ones in the buffer.
    unsigned FirstSignificant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    ಯದಿ (buffer[FirstSignificant - 1] < '5') {
      ಯಾವತ್ (FirstSignificant < N && buffer[FirstSignificant] == '0')
        FirstSignificant++;

      exp += FirstSignificant;
      buffer.erase(&buffer[0], &buffer[FirstSignificant]);
      ಪ್ರತ್ಯಾಗಮ;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    ಪ್ರತಿ (unsigned I = FirstSignificant; I != N; ++I) {
      ಯದಿ (buffer[I] == '9') {
        FirstSignificant++;
      } ಅನ್ಯಥಾ {
        buffer[I]++;
        ಭಙ್ಗ;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    ಯದಿ (FirstSignificant == N) {
      exp += FirstSignificant;
      buffer.clear();
      buffer.ತಳ್ಳು_back('1');
      ಪ್ರತ್ಯಾಗಮ;
    }

    exp += FirstSignificant;
    buffer.erase(&buffer[0], &buffer[FirstSignificant]);
  }

  ಶೂನ್ಯ toStringImpl(SmallVectorImpl<ವರ್ಣ> &Str, ಧ್ರುವ ತಾರ್ಕಿಕ isNeg, ಪೂರ್ಣಾಙ್ಕ exp,
                    APInt significand, unsigned FormatPrecision,
                    unsigned FormatMaxPadding, ತಾರ್ಕಿಕ TruncateZero) {
    ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ semanticsPrecision = significand.getBitWidth();

    ಯದಿ (isNeg)
      Str.ತಳ್ಳು_back('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    ಯದಿ (!FormatPrecision) {
      // We use enough digits so the number can be round-tripped back to an
      // APFloat. The formula comes from "How to Print Floating-Point Numbers
      // Accurately" by Steele and White.
      // FIXME: Using a formula based purely on the precision is conservative;
      // we can print fewer digits depending on the actual value being printed.

      // FormatPrecision = 2 + floor(significandBits / lg_2(10))
      FormatPrecision = 2 + semanticsPrecision * 59 / 196;
    }

    // Ignore trailing binary zeros.
    ಪೂರ್ಣಾಙ್ಕ trailingZeros = significand.countr_zero();
    exp += trailingZeros;
    significand.lshrInPlace(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    ಯದಿ (exp == 0) {
      // Nothing to do.
    } ಅನ್ಯಥಾ ಯದಿ (exp > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + exp);
      significand <<= exp;
      exp = 0;
    } ಅನ್ಯಥಾ { /* exp < 0 */
      ಪೂರ್ಣಾಙ್ಕ texp = -exp;

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
      ಯಾವತ್ (ಸತ್ಯ) {
        ಯದಿ (texp & 1)
          significand *= five_to_the_i;

        texp >>= 1;
        ಯದಿ (!texp)
          ಭಙ್ಗ;
        five_to_the_i *= five_to_the_i;
      }
    }

    AdjustToPrecision(significand, exp, FormatPrecision);

    SmallVector<ವರ್ಣ, 256> buffer;

    // Fill the buffer.
    unsigned precision = significand.getBitWidth();
    ಯದಿ (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    ತಾರ್ಕಿಕ inTrail = ಸತ್ಯ;
    ಯಾವತ್ (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      unsigned d = digit.get_ZExt_ಮೌಲ್ಯ();

      // Drop trailing zeros.
      ಯದಿ (inTrail && !d)
        exp++;
      ಅನ್ಯಥಾ {
        buffer.ತಳ್ಳು_back((ವರ್ಣ) ('0' + d));
        inTrail = ಅಸತ್ಯ;
      }
    }

    ಪ್ರತಿಜ್ಞಾ(!buffer.empty() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    AdjustToPrecision(buffer, exp, FormatPrecision);

    unsigned NDigits = buffer.size();

    // Check whether we should use scientific notation.
    ತಾರ್ಕಿಕ FormatScientific;
    ಯದಿ (!FormatMaxPadding) {
      FormatScientific = ಸತ್ಯ;
    } ಅನ್ಯಥಾ {
      ಯದಿ (exp >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((unsigned) exp > FormatMaxPadding ||
                            NDigits + (unsigned) exp > FormatPrecision);
      } ಅನ್ಯಥಾ {
        // Power of the most significant digit.
        ಪೂರ್ಣಾಙ್ಕ MSD = exp + (ಪೂರ್ಣಾಙ್ಕ) (NDigits - 1);
        ಯದಿ (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = ಅಸತ್ಯ;
        } ಅನ್ಯಥಾ {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((unsigned) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    ಯದಿ (FormatScientific) {
      exp += (NDigits - 1);

      Str.ತಳ್ಳು_back(buffer[NDigits-1]);
      Str.ತಳ್ಳು_back('.');
      ಯದಿ (NDigits == 1 && TruncateZero)
        Str.ತಳ್ಳು_back('0');
      ಅನ್ಯಥಾ
        ಪ್ರತಿ (unsigned I = 1; I != NDigits; ++I)
          Str.ತಳ್ಳು_back(buffer[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      ಯದಿ (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.ತಳ್ಳು_back(TruncateZero ? 'E' : 'e');

      Str.ತಳ್ಳು_back(exp >= 0 ? '+' : '-');
      ಯದಿ (exp < 0)
        exp = -exp;
      SmallVector<ವರ್ಣ, 6> expbuf;
      ಕುರು {
        expbuf.ತಳ್ಳು_back((ವರ್ಣ) ('0' + (exp % 10)));
        exp /= 10;
      } ಯಾವತ್ (exp);
      // Exponent always at least two digits if we do not truncate zeros.
      ಯದಿ (!TruncateZero && expbuf.size() < 2)
        expbuf.ತಳ್ಳು_back('0');
      ಪ್ರತಿ (unsigned I = 0, E = expbuf.size(); I != E; ++I)
        Str.ತಳ್ಳು_back(expbuf[E-1-I]);
      ಪ್ರತ್ಯಾಗಮ;
    }

    // Non-scientific, positive exponents.
    ಯದಿ (exp >= 0) {
      ಪ್ರತಿ (unsigned I = 0; I != NDigits; ++I)
        Str.ತಳ್ಳು_back(buffer[NDigits-1-I]);
      ಪ್ರತಿ (unsigned I = 0; I != (unsigned) exp; ++I)
        Str.ತಳ್ಳು_back('0');
      ಪ್ರತ್ಯಾಗಮ;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    ಪೂರ್ಣಾಙ್ಕ NWholeDigits = exp + (ಪೂರ್ಣಾಙ್ಕ) NDigits;

    unsigned I = 0;
    ಯದಿ (NWholeDigits > 0) {
      ಪ್ರತಿ (; I != (unsigned) NWholeDigits; ++I)
        Str.ತಳ್ಳು_back(buffer[NDigits-I-1]);
      Str.ತಳ್ಳು_back('.');
    } ಅನ್ಯಥಾ {
      unsigned NZeros = 1 + (unsigned) -NWholeDigits;

      Str.ತಳ್ಳು_back('0');
      Str.ತಳ್ಳು_back('.');
      ಪ್ರತಿ (unsigned Z = 1; Z != NZeros; ++Z)
        Str.ತಳ್ಳು_back('0');
    }

    ಪ್ರತಿ (; I != NDigits; ++I)
      Str.ತಳ್ಳು_back(buffer[NDigits-I-1]);

  }
} // namespace

ಶೂನ್ಯ IEEEFloat::toString(SmallVectorImpl<ವರ್ಣ> &Str, unsigned FormatPrecision,
                         unsigned FormatMaxPadding, ತಾರ್ಕಿಕ TruncateZero) ಧ್ರುವ {
  ವಿಕಲ್ಪ (category) {
  ಪಕ್ಷ fcInfinity:
    ಯದಿ (isNegative())
      ಪ್ರತ್ಯಾಗಮ append(Str, "-Inf");
    ಅನ್ಯಥಾ
      ಪ್ರತ್ಯಾಗಮ append(Str, "+Inf");

  ಪಕ್ಷ fcNaN: ಪ್ರತ್ಯಾಗಮ append(Str, "NaN");

  ಪಕ್ಷ fcZero:
    ಯದಿ (isNegative())
      Str.ತಳ್ಳು_back('-');

    ಯದಿ (!FormatMaxPadding) {
      ಯದಿ (TruncateZero)
        append(Str, "0.0E+0");
      ಅನ್ಯಥಾ {
        append(Str, "0.0");
        ಯದಿ (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } ಅನ್ಯಥಾ {
      Str.ತಳ್ಳು_back('0');
    }
    ಪ್ರತ್ಯಾಗಮ;

  ಪಕ್ಷ fcNormal:
    ಭಙ್ಗ;
  }

  // Decompose the number into an APInt and an exponent.
  ಪೂರ್ಣಾಙ್ಕ exp = exponent - ((ಪೂರ್ಣಾಙ್ಕ) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_ಎಣಿಕೆ_For_Bits(semantics->precision)));

  toStringImpl(Str, isNegative(), exp, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

ಪೂರ್ಣಾಙ್ಕ IEEEFloat::getExactLog2Abs() ಧ್ರುವ {
  ಯದಿ (!isFinite() || isZero())
    ಪ್ರತ್ಯಾಗಮ INT_MIN;

  ಧ್ರುವ integerPart *Parts = significandParts();
  ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ Part_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_Bits(semantics->precision);

  ಪೂರ್ಣಾಙ್ಕ ತೆಗೆ_ಎಣಿಕೆ = 0;
  ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = 0; i < Part_ಎಣಿಕೆ; ++i) {
    ತೆಗೆ_ಎಣಿಕೆ += llvm::popcount(Parts[i]);
    ಯದಿ (ತೆಗೆ_ಎಣಿಕೆ > 1)
      ಪ್ರತ್ಯಾಗಮ INT_MIN;
  }

  ಯದಿ (exponent != semantics->minExponent)
    ಪ್ರತ್ಯಾಗಮ exponent;

  ಪೂರ್ಣಾಙ್ಕ CountrParts = 0;
  ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = 0; i < Part_ಎಣಿಕೆ;
       ++i, CountrParts += APInt::APINT_BITS_PER_WORD) {
    ಯದಿ (Parts[i] != 0) {
      ಪ್ರತ್ಯಾಗಮ exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

ತಾರ್ಕಿಕ IEEEFloat::isSignaling() ಧ್ರುವ {
  ಯದಿ (!isNaN())
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಯದಿ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  ಪ್ರತ್ಯಾಗಮ !APInt::tcExtractBit(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::opStatus IEEEFloat::next(ತಾರ್ಕಿಕ nextDown) {
  // If we are performing nextDown, swap sign so we have -x.
  ಯದಿ (nextDown)
    changeSign();

  // Compute nextUp(x)
  opStatus result = opOK;

  // Handle each float category separately.
  ವಿಕಲ್ಪ (category) {
  ಪಕ್ಷ fcInfinity:
    // nextUp(+inf) = +inf
    ಯದಿ (!isNegative())
      ಭಙ್ಗ;
    // nextUp(-inf) = -getLargest()
    makeLargest(ಸತ್ಯ);
    ಭಙ್ಗ;
  ಪಕ್ಷ fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    ಯದಿ (isSignaling()) {
      result = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(ಅಸತ್ಯ, isNegative(), nullptr);
    }
    ಭಙ್ಗ;
  ಪಕ್ಷ fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(ಅಸತ್ಯ);
    ಭಙ್ಗ;
  ಪಕ್ಷ fcNormal:
    // nextUp(-getSmallest()) = -0
    ಯದಿ (isSmallest() && isNegative()) {
      APInt::tcSet(significandParts(), 0, part_ಎಣಿಕೆ());
      category = fcZero;
      exponent = 0;
      ಯದಿ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = ಅಸತ್ಯ;
      ಯದಿ (!semantics->hasZero)
        makeSmallestNormalized(ಅಸತ್ಯ);
      ಭಙ್ಗ;
    }

    ಯದಿ (isLargest() && !isNegative()) {
      ಯದಿ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        ಭಙ್ಗ;
      } ಅನ್ಯಥಾ ಯದಿ (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        ಭಙ್ಗ;
      } ಅನ್ಯಥಾ {
        // nextUp(getLargest()) == INFINITY
        APInt::tcSet(significandParts(), 0, part_ಎಣಿಕೆ());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        ಭಙ್ಗ;
      }
    }

    // nextUp(normal) == normal + inc.
    ಯದಿ (isNegative()) {
      // If we are negative, we need to decrement the significand.

      // We only cross a binade boundary that requires adjusting the exponent
      // if:
      //   1. exponent != semantics->minExponent. This implies we are not in the
      //   smallest binade or are dealing with denormals.
      //   2. Our significand excluding the integral bit is all zeros.
      ತಾರ್ಕಿಕ WillCrossBinadeBoundary =
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
      APInt::tcDecrement(Parts, part_ಎಣಿಕೆ());

      ಯದಿ (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tcSetBit(Parts, semantics->precision - 1);
        exponent--;
      }
    } ಅನ್ಯಥಾ {
      // If we are positive, we need to increment the significand.

      // We only cross a binade boundary that requires adjusting the exponent if
      // the input is not a denormal and all of said input's significand bits
      // are set. If all of said conditions are true: clear the significand, set
      // the integral bit to 1, and increment the exponent. If we have a
      // denormal always increment since moving denormals and the numbers in the
      // smallest normal binade have the same exponent in our representation.
      // If there are only exponents, any increment always crosses the
      // BinadeBoundary.
      ತಾರ್ಕಿಕ WillCrossBinadeBoundary = !APFloat::hasSignificand(*semantics) ||
                                     (!isDenormal() && isSignificandAllOnes());

      ಯದಿ (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tcSet(Parts, 0, part_ಎಣಿಕೆ());
        APInt::tcSetBit(Parts, semantics->precision - 1);
        ಪ್ರತಿಜ್ಞಾ(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } ಅನ್ಯಥಾ {
        incrementSignificand();
      }
    }
    ಭಙ್ಗ;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  ಯದಿ (nextDown)
    changeSign();

  ಪ್ರತ್ಯಾಗಮ result;
}

APInt IEEEFloat::getNaNPayload() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  unsigned Bits = semantics->precision - 1;
  ಪ್ರತ್ಯಾಗಮ APInt(Bits, ArrayRef(significandParts(), part_ಎಣಿಕೆ_For_Bits(Bits)));
}

APFloatBase::ExponentType IEEEFloat::exponentNaN() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ ::exponentNaN(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentInf() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ ::exponentInf(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentZero() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ ::exponentZero(*semantics);
}

ಶೂನ್ಯ IEEEFloat::makeInf(ತಾರ್ಕಿಕ Negative) {
  ಯದಿ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  ಯದಿ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(ಅಸತ್ಯ, Negative);
    ಪ್ರತ್ಯಾಗಮ;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tcSet(significandParts(), 0, part_ಎಣಿಕೆ());
}

ಶೂನ್ಯ IEEEFloat::makeZero(ತಾರ್ಕಿಕ Negative) {
  ಯದಿ (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  ಯದಿ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = ಅಸತ್ಯ;
  }
  exponent = exponentZero();
  APInt::tcSet(significandParts(), 0, part_ಎಣಿಕೆ());
}

ಶೂನ್ಯ IEEEFloat::makeQuiet() {
  ಪ್ರತಿಜ್ಞಾ(isNaN());
  ಯದಿ (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tcSetBit(significandParts(), semantics->precision - 2);
}

ಪೂರ್ಣಾಙ್ಕ ilogb(ಧ್ರುವ IEEEFloat &Arg) {
  ಯದಿ (Arg.isNaN())
    ಪ್ರತ್ಯಾಗಮ APFloat::IEK_NaN;
  ಯದಿ (Arg.isZero())
    ಪ್ರತ್ಯಾಗಮ APFloat::IEK_Zero;
  ಯದಿ (Arg.isInfinity())
    ಪ್ರತ್ಯಾಗಮ APFloat::IEK_Inf;
  ಯದಿ (!Arg.isDenormal())
    ಪ್ರತ್ಯಾಗಮ Arg.exponent;

  IEEEFloat Normalized(Arg);
  ಪೂರ್ಣಾಙ್ಕ SignificandBits = Arg.getSemantics().precision - 1;

  Normalized.exponent += SignificandBits;
  Normalized.normalize(APFloat::rmNearestTiesToEven, lfExactlyZero);
  ಪ್ರತ್ಯಾಗಮ Normalized.exponent - SignificandBits;
}

IEEEFloat scalbn(IEEEFloat X, ಪೂರ್ಣಾಙ್ಕ Exp, roundingMode RoundingMode) {
  auto MaxExp = X.getSemantics().maxExponent;
  auto MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  ಪೂರ್ಣಾಙ್ಕ SignificandBits = X.getSemantics().precision - 1;
  ಪೂರ್ಣಾಙ್ಕ MaxIncrement = MaxExp - (MinExp - SignificandBits) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(RoundingMode, lfExactlyZero);
  ಯದಿ (X.isNaN())
    X.makeQuiet();
  ಪ್ರತ್ಯಾಗಮ X;
}

IEEEFloat frexp(ಧ್ರುವ IEEEFloat &Val, ಪೂರ್ಣಾಙ್ಕ &Exp, roundingMode RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  ಯದಿ (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    ಪ್ರತ್ಯಾಗಮ Quiet;
  }

  ಯದಿ (Exp == APFloat::IEK_Inf)
    ಪ್ರತ್ಯಾಗಮ Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  ಪ್ರತ್ಯಾಗಮ scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(ಧ್ರುವ fltSemantics &S)
    : Semantics(&S),
      Floats(ನೂತನ APFloat[2]{APFloat(APFloatBase::semIEEEdouble),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ಧ್ರುವ fltSemantics &S, uninitializedTag)
    : Semantics(&S), Floats(ನೂತನ APFloat[2]{
                         APFloat(APFloatBase::semIEEEdouble, uninitialized),
                         APFloat(APFloatBase::semIEEEdouble, uninitialized)}) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ಧ್ರುವ fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(ನೂತನ APFloat[2]{APFloat(APFloatBase::semIEEEdouble, I),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ಧ್ರುವ fltSemantics &S, ಧ್ರುವ APInt &I)
    : Semantics(&S),
      Floats(ನೂತನ APFloat[2]{
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[0])),
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[1]))}) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ಧ್ರುವ fltSemantics &S, APFloat &&First,
                             APFloat &&Second)
    : Semantics(&S),
      Floats(ನೂತನ APFloat[2]{std::move(First), std::move(Second)}) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble);
  ಪ್ರತಿಜ್ಞಾ(&Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  ಪ್ರತಿಜ್ಞಾ(&Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(ಧ್ರುವ DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? ನೂತನ APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : nullptr) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloatBase::semBogus;
  RHS.Floats = nullptr;
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::operator=(ಧ್ರುವ DoubleAPFloat &RHS) {
  ಯದಿ (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } ಅನ್ಯಥಾ ಯದಿ (ಏತತ್ != &RHS) {
    ಏತತ್->~DoubleAPFloat();
    ನೂತನ (ಏತತ್) DoubleAPFloat(RHS);
  }
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
ಸ್ಥಿರ std::pair<APFloat, APFloat> fastTwoSum(APFloat X, APFloat Y) {
  ಯದಿ (!X.isFinite())
    ಪ್ರತ್ಯಾಗಮ {X, APFloat::getZero(X.getSemantics(), /*Negative=*/ಅಸತ್ಯ)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  ಪ್ರತ್ಯಾಗಮ {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::opStatus DoubleAPFloat::addImpl(ಧ್ರುವ APFloat &a, ಧ್ರುವ APFloat &aa,
                                         ಧ್ರುವ APFloat &c, ಧ್ರುವ APFloat &cc,
                                         roundingMode RM) {
  ಪೂರ್ಣಾಙ್ಕ Status = opOK;
  APFloat z = a;
  Status |= z.add(c, RM);
  ಯದಿ (!z.isFinite()) {
    ಯದಿ (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ ಅಸತ್ಯ);
      ಪ್ರತ್ಯಾಗಮ (opStatus)Status;
    }
    Status = opOK;
    auto AComparedToC = a.compare_Absolute_ಮೌಲ್ಯ(c);
    z = cc;
    Status |= z.add(aa, RM);
    ಯದಿ (AComparedToC == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      Status |= z.add(c, RM);
      Status |= z.add(a, RM);
    } ಅನ್ಯಥಾ {
      // z = cc + aa + a + c;
      Status |= z.add(a, RM);
      Status |= z.add(c, RM);
    }
    ಯದಿ (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ ಅಸತ್ಯ);
      ಪ್ರತ್ಯಾಗಮ (opStatus)Status;
    }
    Floats[0] = z;
    APFloat zz = aa;
    Status |= zz.add(cc, RM);
    ಯದಿ (AComparedToC == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(c, RM);
      Status |= Floats[1].add(zz, RM);
    } ಅನ್ಯಥಾ {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(a, RM);
      Status |= Floats[1].add(zz, RM);
    }
  } ಅನ್ಯಥಾ {
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
    ಯದಿ (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ ಅಸತ್ಯ);
      ಪ್ರತ್ಯಾಗಮ opOK;
    }
    Floats[0] = z;
    Status |= Floats[0].add(zz, RM);
    ಯದಿ (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ ಅಸತ್ಯ);
      ಪ್ರತ್ಯಾಗಮ (opStatus)Status;
    }
    Floats[1] = std::move(z);
    Status |= Floats[1].subtract(Floats[0], RM);
    Status |= Floats[1].add(zz, RM);
  }
  ಪ್ರತ್ಯಾಗಮ (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::addWithSpecial(ಧ್ರುವ DoubleAPFloat &LHS,
                                                ಧ್ರುವ DoubleAPFloat &RHS,
                                                DoubleAPFloat &Out,
                                                roundingMode RM) {
  ಯದಿ (LHS.getCategory() == fcNaN) {
    Out = LHS;
    ಪ್ರತ್ಯಾಗಮ opOK;
  }
  ಯದಿ (RHS.getCategory() == fcNaN) {
    Out = RHS;
    ಪ್ರತ್ಯಾಗಮ opOK;
  }
  ಯದಿ (LHS.getCategory() == fcZero) {
    Out = RHS;
    ಪ್ರತ್ಯಾಗಮ opOK;
  }
  ಯದಿ (RHS.getCategory() == fcZero) {
    Out = LHS;
    ಪ್ರತ್ಯಾಗಮ opOK;
  }
  ಯದಿ (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    Out.makeNaN(ಅಸತ್ಯ, Out.isNegative(), nullptr);
    ಪ್ರತ್ಯಾಗಮ opInvalidOp;
  }
  ಯದಿ (LHS.getCategory() == fcInfinity) {
    Out = LHS;
    ಪ್ರತ್ಯಾಗಮ opOK;
  }
  ಯದಿ (RHS.getCategory() == fcInfinity) {
    Out = RHS;
    ಪ್ರತ್ಯಾಗಮ opOK;
  }
  ಪ್ರತಿಜ್ಞಾ(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  ಪ್ರತಿಜ್ಞಾ(&A.getSemantics() == &APFloatBase::semIEEEdouble);
  ಪ್ರತಿಜ್ಞಾ(&AA.getSemantics() == &APFloatBase::semIEEEdouble);
  ಪ್ರತಿಜ್ಞಾ(&C.getSemantics() == &APFloatBase::semIEEEdouble);
  ಪ್ರತಿಜ್ಞಾ(&CC.getSemantics() == &APFloatBase::semIEEEdouble);
  ಪ್ರತಿಜ್ಞಾ(&Out.Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  ಪ್ರತಿಜ್ಞಾ(&Out.Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
  ಪ್ರತ್ಯಾಗಮ Out.addImpl(A, AA, C, CC, RM);
}

APFloat::opStatus DoubleAPFloat::add(ಧ್ರುವ DoubleAPFloat &RHS,
                                     roundingMode RM) {
  ಪ್ರತ್ಯಾಗಮ addWithSpecial(*ಏತತ್, RHS, *ಏತತ್, RM);
}

APFloat::opStatus DoubleAPFloat::subtract(ಧ್ರುವ DoubleAPFloat &RHS,
                                          roundingMode RM) {
  changeSign();
  auto Ret = add(RHS, RM);
  changeSign();
  ಪ್ರತ್ಯಾಗಮ Ret;
}

APFloat::opStatus DoubleAPFloat::multiply(ಧ್ರುವ DoubleAPFloat &RHS,
                                          APFloat::roundingMode RM) {
  ಧ್ರುವ auto &LHS = *ಏತತ್;
  auto &Out = *ಏತತ್;
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
  ಯದಿ (LHS.getCategory() == fcNaN) {
    Out = LHS;
    ಪ್ರತ್ಯಾಗಮ opOK;
  }
  ಯದಿ (RHS.getCategory() == fcNaN) {
    Out = RHS;
    ಪ್ರತ್ಯಾಗಮ opOK;
  }
  ಯದಿ ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    Out.makeNaN(ಅಸತ್ಯ, ಅಸತ್ಯ, nullptr);
    ಪ್ರತ್ಯಾಗಮ opOK;
  }
  ಯದಿ (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    Out = LHS;
    ಪ್ರತ್ಯಾಗಮ opOK;
  }
  ಯದಿ (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    Out = RHS;
    ಪ್ರತ್ಯಾಗಮ opOK;
  }
  ಪ್ರತಿಜ್ಞಾ(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal &&
         "Special cases not handled exhaustively");

  ಪೂರ್ಣಾಙ್ಕ Status = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  Status |= T.multiply(C, RM);
  ಯದಿ (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ ಅಸತ್ಯ);
    ಪ್ರತ್ಯಾಗಮ (opStatus)Status;
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
  ಯದಿ (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ ಅಸತ್ಯ);
  } ಅನ್ಯಥಾ {
    // Floats[1] = (t - u) + tau
    Status |= T.subtract(U, RM);
    Status |= T.add(Tau, RM);
    Floats[1] = std::move(T);
  }
  ಪ್ರತ್ಯಾಗಮ (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::divide(ಧ್ರುವ DoubleAPFloat &RHS,
                                        APFloat::roundingMode RM) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.divide(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()), RM);
  *ಏತತ್ = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  ಪ್ರತ್ಯಾಗಮ Ret;
}

APFloat::opStatus DoubleAPFloat::remainder(ಧ್ರುವ DoubleAPFloat &RHS) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.remainder(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *ಏತತ್ = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  ಪ್ರತ್ಯಾಗಮ Ret;
}

APFloat::opStatus DoubleAPFloat::mod(ಧ್ರುವ DoubleAPFloat &RHS) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.mod(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *ಏತತ್ = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  ಪ್ರತ್ಯಾಗಮ Ret;
}

APFloat::opStatus
DoubleAPFloat::fusedMultiplyAdd(ಧ್ರುವ DoubleAPFloat &Multiplicand,
                                ಧ್ರುವ DoubleAPFloat &Addend,
                                APFloat::roundingMode RM) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.fusedMultiplyAdd(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcastToAPInt()),
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, Addend.bitcastToAPInt()),
      RM);
  *ಏತತ್ = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  ಪ್ರತ್ಯಾಗಮ Ret;
}

APFloat::opStatus DoubleAPFloat::roundToIntegral(APFloat::roundingMode RM) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ಧ್ರುವ APFloat &Hi = getFirst();
  ಧ್ರುವ APFloat &Lo = getSecond();

  APFloat RoundedHi = Hi;
  ಧ್ರುವ opStatus HiStatus = RoundedHi.roundToIntegral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  ಯದಿ (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/ಅಸತ್ಯ);
    ಪ್ರತ್ಯಾಗಮ HiStatus;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  auto RoundToNearestHelper = [](APFloat ToRound, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    ಧ್ರುವ APFloat RoundingError = Rounded - ToRound;
    ಯದಿ (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        abs(RoundingError).is_Exactly_ಮೌಲ್ಯ(0.5))
      Rounded.add(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rmNearestTiesToEven);
    ಪ್ರತ್ಯಾಗಮ Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  ಯದಿ (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    ಯದಿ (RM == rmNearestTiesToAway || RM == rmNearestTiesToEven)
      RoundedHi = RoundToNearestHelper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/ಅಸತ್ಯ);
    ಪ್ರತ್ಯಾಗಮ HiStatus;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  RoundingMode LoRoundingMode;
  ಯದಿ (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    LoRoundingMode = isNegative() ? rmTowardPositive : rmTowardNegative;
  ಅನ್ಯಥಾ
    LoRoundingMode = RM;

  APFloat RoundedLo = Lo;
  ಧ್ರುವ opStatus LoStatus = RoundedLo.roundToIntegral(LoRoundingMode);
  ಯದಿ (LoRoundingMode == rmNearestTiesToAway)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = RoundToNearestHelper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fastTwoSum(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  ಪ್ರತ್ಯಾಗಮ LoStatus;
}

ಶೂನ್ಯ DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmpResult
DoubleAPFloat::compare_Absolute_ಮೌಲ್ಯ(ಧ್ರುವ DoubleAPFloat &RHS) ಧ್ರುವ {
  // Compare absolute values of the high parts.
  ಧ್ರುವ cmpResult HiPartCmp = Floats[0].compare_Absolute_ಮೌಲ್ಯ(RHS.Floats[0]);
  ಯದಿ (HiPartCmp != cmpEqual)
    ಪ್ರತ್ಯಾಗಮ HiPartCmp;

  // Zero, regardless of sign, is equal.
  ಯದಿ (Floats[1].isZero() && RHS.Floats[1].isZero())
    ಪ್ರತ್ಯಾಗಮ cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  ಧ್ರುವ ತಾರ್ಕಿಕ ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  ಧ್ರುವ ತಾರ್ಕಿಕ RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  ಯದಿ (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    ಪ್ರತ್ಯಾಗಮ RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  ಯದಿ (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    ಪ್ರತ್ಯಾಗಮ ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  ಯದಿ (ThisIsSubtractive != RHSIsSubtractive)
    ಪ್ರತ್ಯಾಗಮ ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  ಧ್ರುವ cmpResult LoPartCmp = Floats[1].compare_Absolute_ಮೌಲ್ಯ(RHS.Floats[1]);

  ಯದಿ (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    ಯದಿ (LoPartCmp == cmpLessThan)
      ಪ್ರತ್ಯಾಗಮ cmpGreaterThan;
    ಯದಿ (LoPartCmp == cmpGreaterThan)
      ಪ್ರತ್ಯಾಗಮ cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  ಪ್ರತ್ಯಾಗಮ LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ Floats[0].getCategory();
}

ತಾರ್ಕಿಕ DoubleAPFloat::isNegative() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Floats[0].isNegative(); }

ಶೂನ್ಯ DoubleAPFloat::makeInf(ತಾರ್ಕಿಕ Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ ಅಸತ್ಯ);
}

ಶೂನ್ಯ DoubleAPFloat::makeZero(ತಾರ್ಕಿಕ Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ ಅಸತ್ಯ);
}

ಶೂನ್ಯ DoubleAPFloat::makeLargest(ತಾರ್ಕಿಕ Neg) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  ಯದಿ (Neg)
    changeSign();
}

ಶೂನ್ಯ DoubleAPFloat::makeSmallest(ತಾರ್ಕಿಕ Neg) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].makeSmallest(Neg);
  Floats[1].makeZero(/* Neg = */ ಅಸತ್ಯ);
}

ಶೂನ್ಯ DoubleAPFloat::makeSmallestNormalized(ತಾರ್ಕಿಕ Neg) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  ಯದಿ (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ ಅಸತ್ಯ);
}

ಶೂನ್ಯ DoubleAPFloat::makeNaN(ತಾರ್ಕಿಕ SNaN, ತಾರ್ಕಿಕ Neg, ಧ್ರುವ APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ ಅಸತ್ಯ);
}

APFloat::cmpResult DoubleAPFloat::compare(ಧ್ರುವ DoubleAPFloat &RHS) ಧ್ರುವ {
  auto Result = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  ಯದಿ (Result == APFloat::cmpEqual)
    ಪ್ರತ್ಯಾಗಮ Floats[1].compare(RHS.Floats[1]);
  ಪ್ರತ್ಯಾಗಮ Result;
}

ತಾರ್ಕಿಕ DoubleAPFloat::bitwiseIsEqual(ಧ್ರುವ DoubleAPFloat &RHS) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_code hash_ಮೌಲ್ಯ(ಧ್ರುವ DoubleAPFloat &Arg) {
  ಯದಿ (Arg.Floats)
    ಪ್ರತ್ಯಾಗಮ hash_combine(hash_ಮೌಲ್ಯ(Arg.Floats[0]), hash_ಮೌಲ್ಯ(Arg.Floats[1]));
  ಪ್ರತ್ಯಾಗಮ hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcastToAPInt() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t Data[] = {
      Floats[0].bitcastToAPInt().getRawData()[0],
      Floats[1].bitcastToAPInt().getRawData()[0],
  };
  ಪ್ರತ್ಯಾಗಮ APInt(128, Data);
}

Expected<APFloat::opStatus> DoubleAPFloat::convertFromString(StringRef S,
                                                             roundingMode RM) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy);
  auto Ret = Tmp.convertFromString(S, RM);
  *ಏತತ್ = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  ಪ್ರತ್ಯಾಗಮ Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::opStatus DoubleAPFloat::next(ತಾರ್ಕಿಕ nextDown) {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  ಯದಿ (nextDown) {
    changeSign();
    APFloat::opStatus Result = next(/*nextDown=*/ಅಸತ್ಯ);
    changeSign();
    ಪ್ರತ್ಯಾಗಮ Result;
  }
  ವಿಕಲ್ಪ (getCategory()) {
  ಪಕ್ಷ fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    ಯದಿ (isNegative())
      makeLargest(ಸತ್ಯ);
    ಪ್ರತ್ಯಾಗಮ opOK;

  ಪಕ್ಷ fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    ಯದಿ (getFirst().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(ಅಸತ್ಯ, isNegative(), nullptr);
      ಪ್ರತ್ಯಾಗಮ opInvalidOp;
    }
    ಪ್ರತ್ಯಾಗಮ opOK;

  ಪಕ್ಷ fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(ಅಸತ್ಯ);
    ಪ್ರತ್ಯಾಗಮ opOK;

  ಪಕ್ಷ fcNormal:
    ಭಙ್ಗ;
  }

  ಧ್ರುವ APFloat &HiOld = getFirst();
  ಧ್ರುವ APFloat &LoOld = getSecond();

  APFloat NextLo = LoOld;
  NextLo.next(/*nextDown=*/ಅಸತ್ಯ);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  auto InLattice = [](ಧ್ರುವ APFloat &Hi, ಧ್ರುವ APFloat &Lo) {
    ಪ್ರತ್ಯಾಗಮ Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  ಯದಿ (InLattice(HiOld, NextLo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(NextLo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat Largest{*Semantics, uninitialized};
    Largest.makeLargest(/*Neg=*/ಅಸತ್ಯ);
    ಯದಿ (compare(Largest) == cmpGreaterThan)
      makeInf(/*Neg=*/ಅಸತ್ಯ);

    ಪ್ರತ್ಯಾಗಮ opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat NextHi = HiOld;
  NextHi.next(/*nextDown=*/ಅಸತ್ಯ);

  // nextUp(getLargest()) == INFINITY
  ಯದಿ (NextHi.isInfinity()) {
    makeInf(/*Neg=*/ಅಸತ್ಯ);
    ಪ್ರತ್ಯಾಗಮ opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  ಯದಿ (NextHi.isZero()) {
    makeZero(/*Neg=*/ಸತ್ಯ);
    ಪ್ರತ್ಯಾಗಮ opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  NextLo = neg(scalbn(harrisonUlp(NextHi), -1, rmTowardZero));
  ಯದಿ (!InLattice(NextHi, NextLo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    NextLo.next(/*nextDown=*/ಅಸತ್ಯ);

  Floats[0] = std::move(NextHi);
  Floats[1] = std::move(NextLo);

  ಪ್ರತ್ಯಾಗಮ opOK;
}

APFloat::opStatus DoubleAPFloat::convertToSignExtendedInteger(
    MutableArrayRef<integerPart> ಒಳಹರಿವು, unsigned ಪೂರ್ಣಾಙ್ಕ Width, ತಾರ್ಕಿಕ IsSigned,
    roundingMode RM, ತಾರ್ಕಿಕ *IsExact) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  ಯದಿ (!getFirst().isFiniteNonZero() || getSecond().isZero())
    ಪ್ರತ್ಯಾಗಮ getFirst().convertToInteger(ಒಳಹರಿವು, Width, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = ಅಸತ್ಯ;
  DoubleAPFloat Integral = *ಏತತ್;
  ಧ್ರುವ opStatus RoundStatus = Integral.roundToIntegral(RM);
  ಯದಿ (RoundStatus == opInvalidOp)
    ಪ್ರತ್ಯಾಗಮ opInvalidOp;
  ಧ್ರುವ APFloat &IntegralHi = Integral.getFirst();
  ಧ್ರುವ APFloat &IntegralLo = Integral.getSecond();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  ತಾರ್ಕಿಕ HiIsExact;
  ಯದಿ (IntegralHi.isZero() || IntegralLo.isZero()) {
    ಧ್ರುವ opStatus HiStatus =
        IntegralHi.convertToInteger(ಒಳಹರಿವು, Width, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    ಪ್ರತ್ಯಾಗಮ HiStatus == opOK ? opInexact : HiStatus;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  ಯದಿ (!IsSigned && IntegralHi.isNegative())
    ಪ್ರತ್ಯಾಗಮ opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  ತಾರ್ಕಿಕ LoIsExact;
  ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ HiExactLog2 = IntegralHi.getExactLog2Abs();
  ಧ್ರುವ unsigned PositiveOverflowWidth = IsSigned ? Width - 1 : Width;
  ಯದಿ (HiExactLog2 >= 0 &&
      static_cast<unsigned>(HiExactLog2) == PositiveOverflowWidth) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    ಯದಿ (IntegralHi.isNegative() == IntegralLo.isNegative())
      ಪ್ರತ್ಯಾಗಮ opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    ಧ್ರುವ opStatus LoStatus = IntegralLo.convertToInteger(
        ಒಳಹರಿವು, Width, /*IsSigned=*/ಸತ್ಯ, RM, &LoIsExact);
    ಯದಿ (LoStatus == opInvalidOp)
      ಪ್ರತ್ಯಾಗಮ opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    ಯದಿ (IsSigned && !IntegralHi.isNegative())
      APInt::tcClearBit(ಒಳಹರಿವು.data(), PositiveOverflowWidth);
    *IsExact = RoundStatus == opOK;
    ಪ್ರತ್ಯಾಗಮ RoundStatus;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  ಧ್ರುವ opStatus HiStatus = IntegralHi.convertToInteger(
      ಒಳಹರಿವು, Width, IsSigned, rmTowardZero, &HiIsExact);
  ಯದಿ (HiStatus == opInvalidOp)
    ಪ್ರತ್ಯಾಗಮ HiStatus;

  // Convert Lo into a temporary integer of the same width.
  APSInt LoResult{Width, /*isUnsigned=*/!IsSigned};
  ಧ್ರುವ opStatus LoStatus =
      IntegralLo.convertToInteger(LoResult, rmTowardZero, &LoIsExact);
  ಯದಿ (LoStatus == opInvalidOp)
    ಪ್ರತ್ಯಾಗಮ LoStatus;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tcAdd(ಒಳಹರಿವು.data(), LoResult.getRawData(), /*carry=*/0, ಒಳಹರಿವು.size());

  *IsExact = RoundStatus == opOK;
  ಪ್ರತ್ಯಾಗಮ RoundStatus;
}

APFloat::opStatus
DoubleAPFloat::convertToInteger(MutableArrayRef<integerPart> ಒಳಹರಿವು,
                                unsigned ಪೂರ್ಣಾಙ್ಕ Width, ತಾರ್ಕಿಕ IsSigned,
                                roundingMode RM, ತಾರ್ಕಿಕ *IsExact) ಧ್ರುವ {
  opStatus FS =
      convertToSignExtendedInteger(ಒಳಹರಿವು, Width, IsSigned, RM, IsExact);

  ಯದಿ (FS == opInvalidOp) {
    ಧ್ರುವ unsigned Dst_Parts_ಎಣಿಕೆ = part_ಎಣಿಕೆ_For_Bits(Width);
    ಪ್ರತಿಜ್ಞಾ(Dst_Parts_ಎಣಿಕೆ <= ಒಳಹರಿವು.size() && "Integer too big");

    unsigned Bits;
    ಯದಿ (getCategory() == fcNaN)
      Bits = 0;
    ಅನ್ಯಥಾ ಯದಿ (isNegative())
      Bits = IsSigned;
    ಅನ್ಯಥಾ
      Bits = Width - IsSigned;

    tcSetLeastSignificantBits(ಒಳಹರಿವು.data(), Dst_Parts_ಎಣಿಕೆ, Bits);
    ಯದಿ (isNegative() && IsSigned)
      APInt::tc_Shift_ಎಡ(ಒಳಹರಿವು.data(), Dst_Parts_ಎಣಿಕೆ, Width - 1);
  }

  ಪ್ರತ್ಯಾಗಮ FS;
}

APFloat::opStatus DoubleAPFloat::handleOverflow(roundingMode RM) {
  ವಿಕಲ್ಪ (RM) {
  ಪಕ್ಷ APFloat::rmTowardZero:
    makeLargest(/*Neg=*/isNegative());
    ಭಙ್ಗ;
  ಪಕ್ಷ APFloat::rmTowardNegative:
    ಯದಿ (isNegative())
      makeInf(/*Neg=*/ಸತ್ಯ);
    ಅನ್ಯಥಾ
      makeLargest(/*Neg=*/ಅಸತ್ಯ);
    ಭಙ್ಗ;
  ಪಕ್ಷ APFloat::rmTowardPositive:
    ಯದಿ (isNegative())
      makeLargest(/*Neg=*/ಸತ್ಯ);
    ಅನ್ಯಥಾ
      makeInf(/*Neg=*/ಅಸತ್ಯ);
    ಭಙ್ಗ;
  ಪಕ್ಷ APFloat::rmNearestTiesToAway:
  ಪಕ್ಷ APFloat::rmNearestTiesToEven:
    makeInf(/*Neg=*/isNegative());
    ಭಙ್ಗ;
  ಸ್ವಭಾವ:
    llvm_unreachable("Invalid rounding mode found");
  }
  opStatus S = opInexact;
  ಯದಿ (!getFirst().isFinite())
    S = static_cast<opStatus>(S | opOverflow);
  ಪ್ರತ್ಯಾಗಮ S;
}

APFloat::opStatus DoubleAPFloat::convertFromUnsignedParts(
    ಧ್ರುವ integerPart *Src, unsigned ಪೂರ್ಣಾಙ್ಕ Src_ಎಣಿಕೆ, roundingMode RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  ಧ್ರುವ unsigned SrcMSB = APInt::tcMSB(Src, Src_ಎಣಿಕೆ);
  ಯದಿ (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/ಅಸತ್ಯ);
    ಪ್ರತ್ಯಾಗಮ opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  ಧ್ರುವ unsigned SrcBitWidth = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/SrcBitWidth, ArrayRef(Src, Src_ಎಣಿಕೆ)},
                /*isUnsigned=*/ಸತ್ಯ};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{getFirst().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/ಅಸತ್ಯ, rmNearestTiesToEven);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  ಯದಿ (!Hi.isFinite())
    ಪ್ರತ್ಯಾಗಮ handleOverflow(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  ತಾರ್ಕಿಕ HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/ಸತ್ಯ};
  Hi.convertToInteger(HiAsInt, rmNearestTiesToEven, &HiAsIntIsExact);
  ಧ್ರುವ APInt Error = SrcInt.zext(HiAsInt.getBitWidth()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  roundingMode LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  ಯದಿ (Error.isNegative()) {
    ಯದಿ (RM == rmNearestTiesToAway) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      ಧ್ರುವ unsigned ErrorActiveBits = Error.getSignificantBits() - 1;
      ಧ್ರುವ unsigned LoPrecision = getSecond().getSemantics().precision;
      ಯದಿ (ErrorActiveBits > LoPrecision) {
        ಧ್ರುವ unsigned RoundingBoundary = ErrorActiveBits - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        ಯದಿ (Error.ಎಣಿಕೆ_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rmTowardPositive;
      }
    } ಅನ್ಯಥಾ ಯದಿ (RM == rmTowardZero) {
      // For rmTowardZero, the final positive result must be truncated (rounded
      // down). When Hi is an overestimate, Error is negative. A standard
      // rmTowardZero conversion of Error would make it *less* negative,
      // effectively rounding the final sum Hi + Lo *up*. To ensure the sum
      // rounds down correctly, we force Lo to round toward -infinity.
      LoRM = rmTowardNegative;
    }
  }

  APFloat Lo{getSecond().getSemantics()};
  opStatus Status = Lo.convertFromAPInt(Error, /*IsSigned=*/ಸತ್ಯ, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fastTwoSum(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  ಯದಿ (!getFirst().isFinite())
    ಪ್ರತ್ಯಾಗಮ handleOverflow(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  ಯದಿ (getFirst().isFiniteNonZero() && Floats[0].isLargest()) {
    DoubleAPFloat Largest{*Semantics};
    Largest.makeLargest(/*Neg=*/ಅಸತ್ಯ);
    ಯದಿ (compare(Largest) == APFloat::cmpGreaterThan)
      ಪ್ರತ್ಯಾಗಮ handleOverflow(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  ಪ್ರತ್ಯಾಗಮ Status;
}

APFloat::opStatus DoubleAPFloat::convertFromAPInt(ಧ್ರುವ APInt &ಒಳಹರಿವು,
                                                  ತಾರ್ಕಿಕ IsSigned,
                                                  roundingMode RM) {
  ಧ್ರುವ ತಾರ್ಕಿಕ Negate_ಒಳಹರಿವು = IsSigned && ಒಳಹರಿವು.isNegative();
  APInt API = ಒಳಹರಿವು;
  ಯದಿ (Negate_ಒಳಹರಿವು)
    API.negate();

  ಧ್ರುವ APFloat::opStatus Status =
      convertFromUnsignedParts(API.getRawData(), API.get_ಅಂಕೆ_ಪದಗಳು(), RM);
  ಯದಿ (Negate_ಒಳಹರಿವು)
    changeSign();
  ಪ್ರತ್ಯಾಗಮ Status;
}

unsigned ಪೂರ್ಣಾಙ್ಕ DoubleAPFloat::convertToHexString(ವರ್ಣ *DST,
                                               unsigned ಪೂರ್ಣಾಙ್ಕ HexDigits,
                                               ತಾರ್ಕಿಕ UpperCase,
                                               roundingMode RM) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ಪ್ರತ್ಯಾಗಮ APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .convertToHexString(DST, HexDigits, UpperCase, RM);
}

ತಾರ್ಕಿಕ DoubleAPFloat::isDenormal() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ getCategory() == fcNormal &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

ತಾರ್ಕಿಕ DoubleAPFloat::isSmallest() ಧ್ರುವ {
  ಯದಿ (getCategory() != fcNormal)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  DoubleAPFloat Tmp(*ಏತತ್);
  Tmp.makeSmallest(ಏತತ್->isNegative());
  ಪ್ರತ್ಯಾಗಮ Tmp.compare(*ಏತತ್) == cmpEqual;
}

ತಾರ್ಕಿಕ DoubleAPFloat::isSmallestNormalized() ಧ್ರುವ {
  ಯದಿ (getCategory() != fcNormal)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  DoubleAPFloat Tmp(*ಏತತ್);
  Tmp.makeSmallestNormalized(ಏತತ್->isNegative());
  ಪ್ರತ್ಯಾಗಮ Tmp.compare(*ಏತತ್) == cmpEqual;
}

ತಾರ್ಕಿಕ DoubleAPFloat::isLargest() ಧ್ರುವ {
  ಯದಿ (getCategory() != fcNormal)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  DoubleAPFloat Tmp(*ಏತತ್);
  Tmp.makeLargest(ಏತತ್->isNegative());
  ಪ್ರತ್ಯಾಗಮ Tmp.compare(*ಏತತ್) == cmpEqual;
}

ತಾರ್ಕಿಕ DoubleAPFloat::isInteger() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ಪ್ರತ್ಯಾಗಮ Floats[0].isInteger() && Floats[1].isInteger();
}

ಶೂನ್ಯ DoubleAPFloat::toString(SmallVectorImpl<ವರ್ಣ> &Str,
                             unsigned FormatPrecision,
                             unsigned FormatMaxPadding,
                             ತಾರ್ಕಿಕ TruncateZero) ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .toString(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

ಪೂರ್ಣಾಙ್ಕ DoubleAPFloat::getExactLog2Abs() ಧ್ರುವ {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  ಯದಿ (getSecond().isNonZero())
    ಪ್ರತ್ಯಾಗಮ INT_MIN;
  ಪ್ರತ್ಯಾಗಮ getFirst().getExactLog2Abs();
}

ಪೂರ್ಣಾಙ್ಕ ilogb(ಧ್ರುವ DoubleAPFloat &Arg) {
  ಧ್ರುವ APFloat &Hi = Arg.getFirst();
  ಧ್ರುವ APFloat &Lo = Arg.getSecond();
  ಪೂರ್ಣಾಙ್ಕ IlogbResult = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  ಯದಿ (Arg.getCategory() != fcNormal)
    ಪ್ರತ್ಯಾಗಮ IlogbResult;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  ಯದಿ (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    ಪ್ರತ್ಯಾಗಮ IlogbResult;
  ಯದಿ (Hi.getExactLog2Abs() == INT_MIN)
    ಪ್ರತ್ಯಾಗಮ IlogbResult;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  ಪ್ರತ್ಯಾಗಮ IlogbResult - 1;
}

DoubleAPFloat scalbn(ಧ್ರುವ DoubleAPFloat &Arg, ಪೂರ್ಣಾಙ್ಕ Exp,
                     APFloat::roundingMode RM) {
  ಪ್ರತಿಜ್ಞಾ(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");
  ಪ್ರತ್ಯಾಗಮ DoubleAPFloat(APFloatBase::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(ಧ್ರುವ DoubleAPFloat &Arg, ಪೂರ್ಣಾಙ್ಕ &Exp,
                    APFloat::roundingMode RM) {
  ಪ್ರತಿಜ್ಞಾ(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  ಯದಿ (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.getFirst() = Quiet.getFirst().makeQuiet();
    ಪ್ರತ್ಯಾಗಮ Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  ಯದಿ (Exp == APFloat::IEK_Inf)
    ಪ್ರತ್ಯಾಗಮ Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  ಯದಿ (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    ಪ್ರತ್ಯಾಗಮ Arg;
  }

  ಧ್ರುವ APFloat &Hi = Arg.getFirst();
  ಧ್ರುವ APFloat &Lo = Arg.getSecond();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  ಧ್ರುವ ತಾರ್ಕಿಕ SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat Second = Lo;
  ಯದಿ (Arg.getCategory() == APFloat::fcNormal && Lo.isFiniteNonZero()) {
    roundingMode LoRoundingMode;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    ಯದಿ (RM == rmTowardZero)
      LoRoundingMode = Arg.isNegative() ? rmTowardPositive : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    ಅನ್ಯಥಾ ಯದಿ (RM == rmNearestTiesToAway && SignsDisagree && Exp > 0)
      LoRoundingMode = rmNearestTiesToEven;
    ಅನ್ಯಥಾ
      LoRoundingMode = RM;
    Second = scalbn(Lo, -Exp, LoRoundingMode);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    ಯದಿ (RM == rmNearestTiesToAway && LoRoundingMode == rmNearestTiesToEven) {
      // Re-scale the result back to check if rounding occurred.
      ಧ್ರುವ APFloat RecomposedLo = scalbn(Second, Exp, rmNearestTiesToEven);
      ಯದಿ (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        ಧ್ರುವ APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        ಧ್ರುವ APFloat UlpOfSecond = harrisonUlp(Second);
        ಧ್ರುವ APFloat ScaledUlpOfSecond =
            scalbn(UlpOfSecond, Exp - 1, rmNearestTiesToEven);
        ಧ್ರುವ ತಾರ್ಕಿಕ IsMidpoint = abs(RoundingError) == ScaledUlpOfSecond;
        ಧ್ರುವ ತಾರ್ಕಿಕ RoundedLoAway =
            Second.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        ಯದಿ (IsMidpoint && RoundedLoAway)
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
    ಯದಿ (Second.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat First = scalbn(Hi, -Exp, RM);
  ಪ್ರತ್ಯಾಗಮ DoubleAPFloat(APFloatBase::PPCDoubleDouble(), std::move(First),
                       std::move(Second));
}

APInt DoubleAPFloat::getNaNPayload() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, ಧ್ರುವ fltSemantics &Semantics) {
  ಯದಿ (usesLayout<IEEEFloat>(Semantics)) {
    ನೂತನ (&IEEE) IEEEFloat(std::move(F));
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ (usesLayout<DoubleAPFloat>(Semantics)) {
    ಧ್ರುವ fltSemantics& S = F.getSemantics();
    ನೂತನ (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloatBase::IEEEdouble()));
    ಪ್ರತ್ಯಾಗಮ;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::opStatus> APFloat::convertFromString(StringRef Str,
                                                       roundingMode RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_code hash_ಮೌಲ್ಯ(ಧ್ರುವ APFloat &Arg) {
  ಯದಿ (APFloat::usesLayout<detail::IEEEFloat>(Arg.getSemantics()))
    ಪ್ರತ್ಯಾಗಮ hash_ಮೌಲ್ಯ(Arg.U.IEEE);
  ಯದಿ (APFloat::usesLayout<detail::DoubleAPFloat>(Arg.getSemantics()))
    ಪ್ರತ್ಯಾಗಮ hash_ಮೌಲ್ಯ(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(ಧ್ರುವ fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  auto StatusOrErr = convertFromString(S, rmNearestTiesToEven);
  ಪ್ರತಿಜ್ಞಾ(StatusOrErr && "Invalid floating point representation");
  consumeError(StatusOrErr.takeError());
}

FPClassTest APFloat::classify() ಧ್ರುವ {
  ಯದಿ (isZero())
    ಪ್ರತ್ಯಾಗಮ isNegative() ? fcNegZero : fcPosZero;
  ಯದಿ (isNormal())
    ಪ್ರತ್ಯಾಗಮ isNegative() ? fcNegNormal : fcPosNormal;
  ಯದಿ (isDenormal())
    ಪ್ರತ್ಯಾಗಮ isNegative() ? fcNegSubnormal : fcPosSubnormal;
  ಯದಿ (isInfinity())
    ಪ್ರತ್ಯಾಗಮ isNegative() ? fcNegInf : fcPosInf;
  ಪ್ರತಿಜ್ಞಾ(isNaN() && "Other class of FP constant");
  ಪ್ರತ್ಯಾಗಮ isSignaling() ? fcSNan : fcQNan;
}

ತಾರ್ಕಿಕ APFloat::getExactInverse(APFloat *Inv) ಧ್ರುವ {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  ಯದಿ (!isFiniteNonZero())
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  ಯದಿ (isDenormal())
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

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
  ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ Exp = getExactLog2Abs();
  ಯದಿ (Exp == INT_MIN)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  ಯದಿ (Reciprocal.getExactLog2Abs() != -Exp)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  ಯದಿ (Reciprocal.isDenormal())
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಪ್ರತಿಜ್ಞಾ(Reciprocal.isFiniteNonZero());

  ಯದಿ (Inv)
    *Inv = std::move(Reciprocal);

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

APFloat::opStatus APFloat::convert(ಧ್ರುವ fltSemantics &ToSemantics,
                                   roundingMode RM, ತಾರ್ಕಿಕ *losesInfo) {
  ಯದಿ (&getSemantics() == &ToSemantics) {
    *losesInfo = ಅಸತ್ಯ;
    ಪ್ರತ್ಯಾಗಮ opOK;
  }
  ಯದಿ (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics))
    ಪ್ರತ್ಯಾಗಮ U.IEEE.convert(ToSemantics, RM, losesInfo);
  ಯದಿ (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<DoubleAPFloat>(ToSemantics)) {
    ಪ್ರತಿಜ್ಞಾ(&ToSemantics == &APFloatBase::semPPCDoubleDouble);
    auto Ret =
        U.IEEE.convert(APFloatBase::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *ಏತತ್ = APFloat(ToSemantics, U.IEEE.bitcastToAPInt());
    ಪ್ರತ್ಯಾಗಮ Ret;
  }
  ಯದಿ (usesLayout<DoubleAPFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics)) {
    auto Ret = getIEEE().convert(ToSemantics, RM, losesInfo);
    *ಏತತ್ = APFloat(std::move(getIEEE()), ToSemantics);
    ಪ್ರತ್ಯಾಗಮ Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_ಮೌಲ್ಯ(ಧ್ರುವ fltSemantics &Semantics) {
  ಪ್ರತ್ಯಾಗಮ APFloat(Semantics, APInt::getAllOnes(Semantics.sizeInBits));
}

ಶೂನ್ಯ APFloat::print(raw_ostream &OS) ಧ್ರುವ {
  SmallVector<ವರ್ಣ, 16> Buffer;
  toString(Buffer);
  OS << Buffer;
}

#ಯದಿ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ಶೂನ್ಯ APFloat::dump() ಧ್ರುವ {
  print(dbgs());
  dbgs() << '\n';
}
#endif

ಶೂನ್ಯ APFloat::Profile(Folding_Set_ಗಂಟು_ID &NID) ಧ್ರುವ {
  NID.Add(bitcastToAPInt());
}

APFloat::opStatus APFloat::convertToInteger(APSInt &result,
                                            roundingMode rounding_mode,
                                            ತಾರ್ಕಿಕ *isExact) ಧ್ರುವ {
  unsigned bitWidth = result.getBitWidth();
  SmallVector<uint64_t, 4> parts(result.get_ಅಂಕೆ_ಪದಗಳು());
  opStatus status = convertToInteger(parts, bitWidth, result.isSigned(),
                                     rounding_mode, isExact);
  // Keeps the original signed-ness.
  result = APInt(bitWidth, parts);
  ಪ್ರತ್ಯಾಗಮ status;
}

ದ್ವಿಗುಣ APFloat::convertToDouble() ಧ್ರುವ {
  ಯದಿ (&getSemantics() == &APFloatBase::semIEEEdouble)
    ಪ್ರತ್ಯಾಗಮ getIEEE().convertToDouble();
  ಪ್ರತಿಜ್ಞಾ(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat Temp = *ಏತತ್;
  ತಾರ್ಕಿಕ LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &LosesInfo);
  ಪ್ರತಿಜ್ಞಾ(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  ಪ್ರತ್ಯಾಗಮ Temp.getIEEE().convertToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 APFloat::convertToQuad() ಧ್ರುವ {
  ಯದಿ (&getSemantics() == &APFloatBase::semIEEEquad)
    ಪ್ರತ್ಯಾಗಮ getIEEE().convertToQuad();
  ಪ್ರತಿಜ್ಞಾ(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat Temp = *ಏತತ್;
  ತಾರ್ಕಿಕ LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEquad, rmNearestTiesToEven, &LosesInfo);
  ಪ್ರತಿಜ್ಞಾ(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  ಪ್ರತ್ಯಾಗಮ Temp.getIEEE().convertToQuad();
}
#endif

ಭಿನ್ನ APFloat::convertToFloat() ಧ್ರುವ {
  ಯದಿ (&getSemantics() == &APFloatBase::semIEEEsingle)
    ಪ್ರತ್ಯಾಗಮ getIEEE().convertToFloat();
  ಪ್ರತಿಜ್ಞಾ(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat Temp = *ಏತತ್;
  ತಾರ್ಕಿಕ LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEsingle, rmNearestTiesToEven, &LosesInfo);
  ಪ್ರತಿಜ್ಞಾ(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  ಪ್ರತ್ಯಾಗಮ Temp.getIEEE().convertToFloat();
}

unsigned APFloatBase::getArbitraryFPFormatSizeInBits(StringRef Format) {
  ಪ್ರತ್ಯಾಗಮ StringSwitch<unsigned>(Format)
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

ತಾರ್ಕಿಕ APFloatBase::isValidArbitraryFPFormat(StringRef Format) {
  ಪ್ರತ್ಯಾಗಮ getArbitraryFPFormatSizeInBits(Format) != 0;
}

ಧ್ರುವ fltSemantics *APFloatBase::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  ಪ್ರತ್ಯಾಗಮ StringSwitch<ಧ್ರುವ fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(nullptr);
}

APFloat::Storage::~Storage() {
  ಯದಿ (usesLayout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ (usesLayout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    ಪ್ರತ್ಯಾಗಮ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(ಧ್ರುವ APFloat::Storage &RHS) {
  ಯದಿ (usesLayout<IEEEFloat>(*RHS.semantics)) {
    ನೂತನ (ಏತತ್) IEEEFloat(RHS.IEEE);
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    ನೂತನ (ಏತತ್) DoubleAPFloat(RHS.Double);
    ಪ್ರತ್ಯಾಗಮ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  ಯದಿ (usesLayout<IEEEFloat>(*RHS.semantics)) {
    ನೂತನ (ಏತತ್) IEEEFloat(std::move(RHS.IEEE));
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    ನೂತನ (ಏತತ್) DoubleAPFloat(std::move(RHS.Double));
    ಪ್ರತ್ಯಾಗಮ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::operator=(ಧ್ರುವ APFloat::Storage &RHS) {
  ಯದಿ (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } ಅನ್ಯಥಾ ಯದಿ (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } ಅನ್ಯಥಾ ಯದಿ (ಏತತ್ != &RHS) {
    ಏತತ್->~Storage();
    ನೂತನ (ಏತತ್) Storage(RHS);
  }
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

APFloat::Storage &APFloat::Storage::operator=(APFloat::Storage &&RHS) {
  ಯದಿ (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } ಅನ್ಯಥಾ ಯದಿ (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } ಅನ್ಯಥಾ ಯದಿ (ಏತತ್ != &RHS) {
    ಏತತ್->~Storage();
    ನೂತನ (ಏತತ್) Storage(std::move(RHS));
  }
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

ನಾಮಾಕಾಶ {

APFloat::opStatus getOpStatusFromLibc(ಪೂರ್ಣಾಙ್ಕ libc_exceptions) {
  APFloat::opStatus status = APFloat::opOK;
  ಯದಿ (libc_exceptions & FE_INVALID)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInvalidOp);
  ಯದಿ (libc_exceptions & FE_DIVBYZERO)
    status = static_cast<APFloat::opStatus>(status | APFloat::opDivByZero);
  ಯದಿ (libc_exceptions & FE_OVERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opOverflow);
  ಯದಿ (libc_exceptions & FE_UNDERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opUnderflow);
  ಯದಿ (libc_exceptions & FE_INEXACT)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInexact);
  ಪ್ರತ್ಯಾಗಮ status;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> exp(ಧ್ರುವ APFloat &x, RoundingMode rounding_mode,
                           APFloat::opStatus *status) {

  ಯದಿ (rounding_mode == APFloatBase::rmNearestTiesToEven) {
    ಯದಿ (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEsingle) {
      ಭಿನ್ನ x_val = x.convertToFloat();
      ಪೂರ್ಣಾಙ್ಕ exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      ಯದಿ (status) {
        *status = getOpStatusFromLibc(exc);
        ಯದಿ (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      ಭಿನ್ನ result = LIBC_NAMESPACE::shared::expf(x_val);
      ಪ್ರತ್ಯಾಗಮ APFloat(result);
    }
    ಯದಿ (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEdouble) {
      ದ್ವಿಗುಣ x_val = x.convertToDouble();
      ಪೂರ್ಣಾಙ್ಕ exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      ಯದಿ (status) {
        *status = getOpStatusFromLibc(exc);
        ಯದಿ (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      ದ್ವಿಗುಣ result = LIBC_NAMESPACE::shared::exp(x_val);
      ಪ್ರತ್ಯಾಗಮ APFloat(result);
    }
  }
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}

} // namespace llvm

#undef APFLOAT_DISPATCH_ON_SEMANTICS
