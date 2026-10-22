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

#अन्तर्भाव "llvm/ADT/APFloat.h"
#अन्तर्भाव "llvm/ADT/APSInt.h"
#अन्तर्भाव "llvm/ADT/ArrayRef.h"
#अन्तर्भाव "llvm/ADT/FloatingPointMode.h"
#अन्तर्भाव "llvm/ADT/FoldingSet.h"
#अन्तर्भाव "llvm/ADT/Hashing.h"
#अन्तर्भाव "llvm/ADT/STLExtras.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/ADT/StringSwitch.h"
#अन्तर्भाव "llvm/Config/llvm-config.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव "llvm/Support/MathExtras.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <cstring>
#अन्तर्भाव <iterator>
#अन्तर्भाव <limits.h>

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

#अन्तर्भाव "shared/math.h"
#अन्तर्भाव "shared/math_check_exceptions.h"

#define APFLOAT_DISPATCH_ON_SEMANTICS(METHOD_CALL)                             \
  करोहि {                                                                         \
    सचे (usesLayout<IEEEFloat>(getSemantics()))                                 \
      पटिनिवत्त U.IEEE.METHOD_CALL;                                               \
    सचे (usesLayout<DoubleAPFloat>(getSemantics()))                             \
      पटिनिवत्त U.Double.METHOD_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } याव (मिच्छा)

using नामाकाश llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#define Pack_Categories_Into_कुञ्चिका(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloatBase::integerPartWidth % 4 == 0, "Part width must be divisible by 4!");

नामाकाश llvm {

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
    सच्च,
    सच्च,
    सच्च,
    सच्च,
    सच्च};
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
    मिच्छा,
    मिच्छा,
    मिच्छा,
    मिच्छा};

constexpr fltSemantics APFloatBase::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    सच्च,
    मिच्छा,
    मिच्छा};

constexpr fltSemantics APFloatBase::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semBogus = {0, 0, 0, 0};

ध्रुव llvm::fltSemantics &APFloatBase::EnumToSemantics(Semantics S) {
  विकल्प (S) {
  पक्ष S_IEEEhalf:
    पटिनिवत्त IEEEhalf();
  पक्ष S_BFloat:
    पटिनिवत्त BFloat();
  पक्ष S_IEEEsingle:
    पटिनिवत्त IEEEsingle();
  पक्ष S_IEEEdouble:
    पटिनिवत्त IEEEdouble();
  पक्ष S_IEEEquad:
    पटिनिवत्त IEEEquad();
  पक्ष S_x87DoubleExtended:
    पटिनिवत्त x87DoubleExtended();
  पक्ष S_PPCDoubleDouble:
    पटिनिवत्त PPCDoubleDouble();
  पक्ष S_PPCDoubleDoubleLegacy:
    पटिनिवत्त PPCDoubleDoubleLegacy();
  पक्ष S_Float8E5M2:
    पटिनिवत्त Float8E5M2();
  पक्ष S_Float8E5M2FNUZ:
    पटिनिवत्त Float8E5M2FNUZ();
  पक्ष S_Float8E4M3:
    पटिनिवत्त Float8E4M3();
  पक्ष S_Float8E4M3FN:
    पटिनिवत्त Float8E4M3FN();
  पक्ष S_Float8E4M3FNUZ:
    पटिनिवत्त Float8E4M3FNUZ();
  पक्ष S_Float8E4M3B11FNUZ:
    पटिनिवत्त Float8E4M3B11FNUZ();
  पक्ष S_Float8E3M4:
    पटिनिवत्त Float8E3M4();
  पक्ष S_FloatTF32:
    पटिनिवत्त FloatTF32();
  पक्ष S_Float8E8M0FNU:
    पटिनिवत्त Float8E8M0FNU();
  पक्ष S_Float8E5M3FNU:
    पटिनिवत्त Float8E5M3FNU();
  पक्ष S_Float6E3M2FN:
    पटिनिवत्त Float6E3M2FN();
  पक्ष S_Float6E2M3FN:
    पटिनिवत्त Float6E2M3FN();
  पक्ष S_Float4E2M1FN:
    पटिनिवत्त Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloatBase::Semantics
APFloatBase::SemanticsToEnum(ध्रुव llvm::fltSemantics &Sem) {
  सचे (&Sem == &llvm::APFloat::IEEEhalf())
    पटिनिवत्त S_IEEEhalf;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::BFloat())
    पटिनिवत्त S_BFloat;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::IEEEsingle())
    पटिनिवत्त S_IEEEsingle;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::IEEEdouble())
    पटिनिवत्त S_IEEEdouble;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::IEEEquad())
    पटिनिवत्त S_IEEEquad;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::x87DoubleExtended())
    पटिनिवत्त S_x87DoubleExtended;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::PPCDoubleDouble())
    पटिनिवत्त S_PPCDoubleDouble;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    पटिनिवत्त S_PPCDoubleDoubleLegacy;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::Float8E5M2())
    पटिनिवत्त S_Float8E5M2;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    पटिनिवत्त S_Float8E5M2FNUZ;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::Float8E4M3())
    पटिनिवत्त S_Float8E4M3;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::Float8E4M3FN())
    पटिनिवत्त S_Float8E4M3FN;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    पटिनिवत्त S_Float8E4M3FNUZ;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    पटिनिवत्त S_Float8E4M3B11FNUZ;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::Float8E3M4())
    पटिनिवत्त S_Float8E3M4;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::FloatTF32())
    पटिनिवत्त S_FloatTF32;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::Float8E8M0FNU())
    पटिनिवत्त S_Float8E8M0FNU;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::Float8E5M3FNU())
    पटिनिवत्त S_Float8E5M3FNU;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::Float6E3M2FN())
    पटिनिवत्त S_Float6E3M2FN;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::Float6E2M3FN())
    पटिनिवत्त S_Float6E2M3FN;
  अञ्ञथा सचे (&Sem == &llvm::APFloat::Float4E2M1FN())
    पटिनिवत्त S_Float4E2M1FN;
  अञ्ञथा
    llvm_unreachable("Unknown floating semantics");
}

तार्किक APFloatBase::isRepresentableBy(ध्रुव fltSemantics &A,
                                    ध्रुव fltSemantics &B) {
  पटिनिवत्त A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

तार्किक APFloatBase::isLosslesslyConvertibleTo(ध्रुव fltSemantics &From,
                                            ध्रुव fltSemantics &InitialTo,
                                            तार्किक IgnoreNaNs) {
  सचे (&From == &InitialTo)
    पटिनिवत्त सच्च;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  सचे (&From == &semPPCDoubleDouble)
    पटिनिवत्त मिच्छा;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  ध्रुव fltSemantics &To =
      &InitialTo == &semPPCDoubleDouble ? semIEEEdouble : InitialTo;

  सचे (!isRepresentableBy(From, To))
    पटिनिवत्त मिच्छा;

  सचे ((From.hasZero && !To.hasZero) ||
      (From.hasSignedRepr && !To.hasSignedRepr))
    पटिनिवत्त मिच्छा;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  तार्किक FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  तार्किक ToHasSignedZero = To.hasZero && To.hasSignedRepr &&
                         To.nanEncoding != fltNanEncoding::NegativeZero;
  सचे (FromHasSignedZero && !ToHasSignedZero)
    पटिनिवत्त मिच्छा;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat SmallestFrom = APFloat::getSmallest(From);
  तार्किक LosesInfo = मिच्छा;
  (शून्य)SmallestFrom.convert(To, APFloat::rmNearestTiesToEven, &LosesInfo);
  सचे (LosesInfo)
    पटिनिवत्त मिच्छा;

  सचे (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    पटिनिवत्त सच्च;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  सचे (IgnoreNaNs) {
    सचे (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      पटिनिवत्त To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    पटिनिवत्त To.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  सचे (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    पटिनिवत्त मिच्छा;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  सचे (To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    पटिनिवत्त सच्च;
  पटिनिवत्त To.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
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
स्थिर constexpr unsigned अङ्कः_Pow5_Parts_On_राशिः = 16;

स्थिर unsigned पूर्णाङ्क powerOfFivePartsForExponent(unsigned पूर्णाङ्क power) {
  प्रतिज्ञा(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  पटिनिवत्त 2 + ((power * 815) / (351 * APFloatBase::integerPartWidth));
}

unsigned पूर्णाङ्क APFloatBase::semanticsPrecision(ध्रुव fltSemantics &semantics) {
  पटिनिवत्त semantics.precision;
}
APFloatBase::ExponentType
APFloatBase::semanticsMaxExponent(ध्रुव fltSemantics &semantics) {
  पटिनिवत्त semantics.maxExponent;
}
APFloatBase::ExponentType
APFloatBase::semanticsMinExponent(ध्रुव fltSemantics &semantics) {
  पटिनिवत्त semantics.minExponent;
}
unsigned पूर्णाङ्क APFloatBase::semanticsSizeInBits(ध्रुव fltSemantics &semantics) {
  पटिनिवत्त semantics.sizeInBits;
}
unsigned पूर्णाङ्क APFloatBase::semanticsIntSizeInBits(ध्रुव fltSemantics &semantics,
                                                 तार्किक isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  unsigned पूर्णाङ्क MinBitWidth = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  सचे (isSigned)
    ++MinBitWidth;
  पटिनिवत्त MinBitWidth;
}

तार्किक APFloatBase::semanticsHasZero(ध्रुव fltSemantics &semantics) {
  पटिनिवत्त semantics.hasZero;
}

तार्किक APFloatBase::semanticsHasSignedRepr(ध्रुव fltSemantics &semantics) {
  पटिनिवत्त semantics.hasSignedRepr;
}

तार्किक APFloatBase::semanticsHasInf(ध्रुव fltSemantics &semantics) {
  पटिनिवत्त semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

तार्किक APFloatBase::semanticsHasNaN(ध्रुव fltSemantics &semantics) {
  पटिनिवत्त semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

तार्किक APFloatBase::isIEEELikeFP(ध्रुव fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  पटिनिवत्त SemanticsToEnum(semantics) <= S_IEEEquad;
}

तार्किक APFloatBase::hasSignBitInMSB(ध्रुव fltSemantics &semantics) {
  पटिनिवत्त semantics.hasSignBitInMSB;
}

तार्किक APFloatBase::isRepresentableAsNormalIn(ध्रुव fltSemantics &Src,
                                            ध्रुव fltSemantics &Dst) {
  // Exponent range must be larger.
  सचे (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    पटिनिवत्त मिच्छा;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  पटिनिवत्त Dst.precision >= Src.precision;
}

unsigned APFloatBase::getSizeInBits(ध्रुव fltSemantics &Sem) {
  पटिनिवत्त Sem.sizeInBits;
}

स्थिर constexpr APFloatBase::ExponentType
exponentZero(ध्रुव fltSemantics &semantics) {
  पटिनिवत्त semantics.minExponent - 1;
}

स्थिर constexpr APFloatBase::ExponentType
exponentInf(ध्रुव fltSemantics &semantics) {
  पटिनिवत्त semantics.maxExponent + 1;
}

स्थिर constexpr APFloatBase::ExponentType
exponentNaN(ध्रुव fltSemantics &semantics) {
  सचे (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    सचे (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      पटिनिवत्त exponentZero(semantics);
    सचे (semantics.hasSignedRepr || semantics.precision > 1)
      पटिनिवत्त semantics.maxExponent;
  }
  पटिनिवत्त semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

स्थिर अन्तःस्थ Error createError(ध्रुव Twine &Err) {
  पटिनिवत्त make_error<StringError>(Err, inconvertibleErrorCode());
}

स्थिर constexpr अन्तःस्थ unsigned पूर्णाङ्क part_गणनम्_For_Bits(unsigned पूर्णाङ्क bits) {
  पटिनिवत्त std::max(1u, (bits + APFloatBase::integerPartWidth - 1) /
                          APFloatBase::integerPartWidth);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
स्थिर अन्तःस्थ unsigned पूर्णाङ्क
dec_Digit_मानम्(unsigned पूर्णाङ्क c)
{
  पटिनिवत्त c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
स्थिर Expected<पूर्णाङ्क> readExponent(StringRef::iterator begin,
                                  StringRef::iterator end) {
  ध्रुव unsigned पूर्णाङ्क overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  सचे (p == end || ((*p == '-' || *p == '+') && (p + 1) == end))
    पटिनिवत्त 0;

  तार्किक isNegative = *p == '-';
  सचे (*p == '-' || *p == '+') {
    p++;
    सचे (p == end)
      पटिनिवत्त createError("Exponent has no digits");
  }

  unsigned absExponent = dec_Digit_मानम्(*p++);
  सचे (absExponent >= 10U)
    पटिनिवत्त createError("Invalid character in exponent");

  अत्थाय (; p != end; ++p) {
    unsigned मानम्_2 = dec_Digit_मानम्(*p);
    सचे (मानम्_2 >= 10U)
      पटिनिवत्त createError("Invalid character in exponent");

    absExponent = absExponent * 10U + मानम्_2;
    सचे (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      भङ्ग;
    }
  }

  सचे (isNegative)
    पटिनिवत्त -(पूर्णाङ्क) absExponent;
  अञ्ञथा
    पटिनिवत्त (पूर्णाङ्क) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
स्थिर Expected<पूर्णाङ्क> totalExponent(StringRef::iterator p,
                                   StringRef::iterator end,
                                   पूर्णाङ्क exponentAdjustment) {
  पूर्णाङ्क exponent = 0;

  सचे (p == end)
    पटिनिवत्त createError("Exponent has no digits");

  तार्किक negative = *p == '-';
  सचे (*p == '-' || *p == '+') {
    p++;
    सचे (p == end)
      पटिनिवत्त createError("Exponent has no digits");
  }

  पूर्णाङ्क unsignedExponent = 0;
  तार्किक overflow = मिच्छा;
  अत्थाय (; p != end; ++p) {
    unsigned पूर्णाङ्क मानम्_2;

    मानम्_2 = dec_Digit_मानम्(*p);
    सचे (मानम्_2 >= 10U)
      पटिनिवत्त createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + मानम्_2;
    सचे (unsignedExponent > 32767) {
      overflow = सच्च;
      भङ्ग;
    }
  }

  सचे (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    overflow = सच्च;

  सचे (!overflow) {
    exponent = unsignedExponent;
    सचे (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    सचे (exponent > 32767 || exponent < -32768)
      overflow = सच्च;
  }

  सचे (overflow)
    exponent = negative ? -32768: 32767;

  पटिनिवत्त exponent;
}

स्थिर Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator end,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = end;
  याव (p != end && *p == '0')
    p++;

  सचे (p != end && *p == '.') {
    *dot = p++;

    सचे (end - begin == 1)
      पटिनिवत्त createError("Significand has no digits");

    याव (p != end && *p == '0')
      p++;
  }

  पटिनिवत्त p;
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
संरचना decimalInfo {
  ध्रुव वर्ण *firstSigDigit;
  ध्रुव वर्ण *lastSigDigit;
  पूर्णाङ्क exponent;
  पूर्णाङ्क normalizedExponent;
};

स्थिर Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator end, decimalInfo *D) {
  StringRef::iterator dot = end;

  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  सचे (!PtrOrErr)
    पटिनिवत्त PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->firstSigDigit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  अत्थाय (; p != end; ++p) {
    सचे (*p == '.') {
      सचे (dot != end)
        पटिनिवत्त createError("String contains multiple dots");
      dot = p++;
      सचे (p == end)
        भङ्ग;
    }
    सचे (dec_Digit_मानम्(*p) >= 10U)
      भङ्ग;
  }

  सचे (p != end) {
    सचे (*p != 'e' && *p != 'E')
      पटिनिवत्त createError("Invalid character in significand");
    सचे (p == begin)
      पटिनिवत्त createError("Significand has no digits");
    सचे (dot != end && p - begin == 1)
      पटिनिवत्त createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    auto ExpOrErr = readExponent(p + 1, end);
    सचे (!ExpOrErr)
      पटिनिवत्त ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    सचे (dot == end)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  सचे (p != D->firstSigDigit) {
    /* Drop insignificant trailing zeroes.  */
    सचे (p != begin) {
      करोहि
        करोहि
          p--;
        याव (p != begin && *p == '0');
      याव (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->firstSigDigit)
                                      - (dot > D->firstSigDigit && dot < p)));
  }

  D->lastSigDigit = p;
  पटिनिवत्त Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
स्थिर Expected<lostFraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator end,
                            unsigned पूर्णाङ्क digit_मानम्) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  सचे (digit_मानम् > 8)
    पटिनिवत्त lfMoreThanHalf;
  अञ्ञथा सचे (digit_मानम् < 8 && digit_मानम् > 0)
    पटिनिवत्त lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  याव (p != end && (*p == '0' || *p == '.'))
    p++;

  सचे (p == end)
    पटिनिवत्त createError("Invalid trailing hexadecimal fraction!");

  unsigned hexDigit = hex_Digit_मानम्(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  सचे (hexDigit == UINT_MAX)
    पटिनिवत्त digit_मानम् == 0 ? lfExactlyZero: lfExactlyHalf;
  अञ्ञथा
    पटिनिवत्त digit_मानम् == 0 ? lfLessThanHalf: lfMoreThanHalf;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
स्थिर lostFraction
lostFractionThroughTruncation(ध्रुव APFloatBase::integerPart *parts,
                              unsigned पूर्णाङ्क part_गणनम्,
                              unsigned पूर्णाङ्क bits)
{
  unsigned lsb = APInt::tcLSB(parts, part_गणनम्);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  सचे (bits <= lsb)
    पटिनिवत्त lfExactlyZero;
  सचे (bits == lsb + 1)
    पटिनिवत्त lfExactlyHalf;
  सचे (bits <= part_गणनम् * APFloatBase::integerPartWidth &&
      APInt::tcExtractBit(parts, bits - 1))
    पटिनिवत्त lfMoreThanHalf;

  पटिनिवत्त lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
स्थिर lostFraction
shift_दक्षिणः(APFloatBase::integerPart *dst, unsigned पूर्णाङ्क parts, unsigned पूर्णाङ्क bits)
{
  lostFraction lost_fraction = lostFractionThroughTruncation(dst, parts, bits);

  APInt::tc_Shift_दक्षिणः(dst, parts, bits);

  पटिनिवत्त lost_fraction;
}

/* Combine the effect of two lost fractions.  */
स्थिर lostFraction
combineLostFractions(lostFraction moreSignificant,
                     lostFraction lessSignificant)
{
  सचे (lessSignificant != lfExactlyZero) {
    सचे (moreSignificant == lfExactlyZero)
      moreSignificant = lfLessThanHalf;
    अञ्ञथा सचे (moreSignificant == lfExactlyHalf)
      moreSignificant = lfMoreThanHalf;
  }

  पटिनिवत्त moreSignificant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
स्थिर unsigned पूर्णाङ्क
HUerrBound(तार्किक inexactMultiply, unsigned पूर्णाङ्क HUerr1, unsigned पूर्णाङ्क HUerr2)
{
  प्रतिज्ञा(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  सचे (HUerr1 + HUerr2 == 0)
    पटिनिवत्त inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  अञ्ञथा
    पटिनिवत्त inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
स्थिर APFloatBase::integerPart
ulpsFromBoundary(ध्रुव APFloatBase::integerPart *parts, unsigned पूर्णाङ्क bits,
                 तार्किक isNearest) {
  प्रतिज्ञा(bits != 0);

  bits--;
  unsigned गणनम् = bits / APFloatBase::integerPartWidth;
  unsigned partBits = bits % APFloatBase::integerPartWidth + 1;

  APFloatBase::integerPart part =
      parts[गणनम्] & (~(APFloatBase::integerPart)0 >>
                      (APFloatBase::integerPartWidth - partBits));

  APFloatBase::integerPart boundary;
  सचे (isNearest)
    boundary = (APFloatBase::integerPart) 1 << (partBits - 1);
  अञ्ञथा
    boundary = 0;

  सचे (गणनम् == 0) {
    सचे (part - boundary <= boundary - part)
      पटिनिवत्त part - boundary;
    अञ्ञथा
      पटिनिवत्त boundary - part;
  }

  सचे (part == boundary) {
    याव (--गणनम्)
      सचे (parts[गणनम्])
        पटिनिवत्त ~(APFloatBase::integerPart) 0; /* A lot.  */

    पटिनिवत्त parts[0];
  } अञ्ञथा सचे (part == boundary - 1) {
    याव (--गणनम्)
      सचे (~parts[गणनम्])
        पटिनिवत्त ~(APFloatBase::integerPart) 0; /* A lot.  */

    पटिनिवत्त -parts[0];
  }

  पटिनिवत्त ~(APFloatBase::integerPart) 0; /* A lot.  */
}

स्थिर constexpr unsigned PowerOfFivePartCounts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

स्थिर constexpr APFloatBase::integerPart PowerOfFiveParts[] = {
#अन्तर्भाव "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  unsigned Sum = 0;
  अत्थाय (unsigned गणनम्_2 : PowerOfFivePartCounts)
    Sum += गणनम्_2;
  पटिनिवत्त Sum;
}() == std::size(PowerOfFiveParts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
स्थिर unsigned पूर्णाङ्क
powerOf5(APFloatBase::integerPart *dst, unsigned पूर्णाङ्क power) {
  स्थिर ध्रुव APFloatBase::integerPart firstEightPowers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  SmallVector<APFloatBase::integerPart, अङ्कः_Pow5_Parts_On_राशिः> scratch;
  scratch.resize_for_overwrite(powerOfFivePartsForExponent(power));

  APFloatBase::integerPart *p1, *p2;
  ध्रुव APFloatBase::integerPart *pow5;

  p1 = dst;
  p2 = scratch.data();

  *p1 = firstEightPowers[power & 7];
  power >>= 3;

  unsigned result = 1;
  pow5 = PowerOfFiveParts;

  अत्थाय (unsigned पूर्णाङ्क n = 0; power; power >>= 1, n++) {
    unsigned parts_गणनम् = PowerOfFivePartCounts[n];

    सचे (power & 1) {
      APFloatBase::integerPart *tmp;

      APInt::tcFullMultiply(p2, p1, pow5, result, parts_गणनम्);
      result += parts_गणनम्;
      सचे (p2[result - 1] == 0)
        result--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_गणनम्;
  }

  सचे (p1 != dst)
    APInt::tcAssign(dst, p1, result);

  पटिनिवत्त result;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
स्थिर ध्रुव वर्ण hexDigitsLower[] = "0123456789abcdef0";
स्थिर ध्रुव वर्ण hexDigitsUpper[] = "0123456789ABCDEF0";
स्थिर ध्रुव वर्ण infinityL[] = "infinity";
स्थिर ध्रुव वर्ण infinityU[] = "INFINITY";
स्थिर ध्रुव वर्ण NaNL[] = "nan";
स्थिर ध्रुव वर्ण NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
स्थिर unsigned पूर्णाङ्क
partAsHex (वर्ण *dst, APFloatBase::integerPart part, unsigned पूर्णाङ्क गणनम्,
           ध्रुव वर्ण *hexDigitChars)
{
  unsigned पूर्णाङ्क result = गणनम्;

  प्रतिज्ञा(गणनम् != 0 && गणनम् <= APFloatBase::integerPartWidth / 4);

  part >>= (APFloatBase::integerPartWidth - 4 * गणनम्);
  याव (गणनम्--) {
    dst[गणनम्] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  पटिनिवत्त result;
}

/* Write out an unsigned decimal integer.  */
स्थिर वर्ण *writeUnsignedDecimal(वर्ण *dst, unsigned पूर्णाङ्क n) {
  वर्ण buff[40], *p;

  p = buff;
  करोहि
    *p++ = '0' + n % 10;
  याव (n /= 10);

  करोहि
    *dst++ = *--p;
  याव (p != buff);

  पटिनिवत्त dst;
}

/* Write out a signed decimal integer.  */
स्थिर वर्ण *writeSignedDecimal(वर्ण *dst, पूर्णाङ्क मानम्_2) {
  सचे (मानम्_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(unsigned) मानम्_2);
  } अञ्ञथा {
    dst = writeUnsignedDecimal(dst, मानम्_2);
  }

  पटिनिवत्त dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
स्थिर APFloat harrisonUlp(ध्रुव APFloat &X) {
  ध्रुव fltSemantics &Sem = X.getSemantics();
  विकल्प (X.getCategory()) {
  पक्ष APFloat::fcNaN:
    पटिनिवत्त APFloat::getQNaN(Sem);
  पक्ष APFloat::fcInfinity:
    पटिनिवत्त APFloat::getInf(Sem);
  पक्ष APFloat::fcZero:
    पटिनिवत्त APFloat::getSmallest(Sem);
  पक्ष APFloat::fcNormal:
    भङ्ग;
  }
  सचे (X.isDenormal() || X.isSmallestNormalized())
    पटिनिवत्त APFloat::getSmallest(Sem);
  पूर्णाङ्क Exp = ilogb(X);
  सचे (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  पटिनिवत्त scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rmNearestTiesToEven);
}

नामाकाश detail {
/* Constructors.  */
शून्य IEEEFloat::initialize(ध्रुव fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  unsigned गणनम् = part_गणनम्();
  सचे (गणनम् > 1)
    significand.parts = नूतन integerPart[गणनम्];
}

शून्य IEEEFloat::freeSignificand() {
  सचे (needsCleanup())
    delete [] significand.parts;
}

शून्य IEEEFloat::assign(ध्रुव IEEEFloat &rhs) {
  प्रतिज्ञा(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  सचे (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

शून्य IEEEFloat::copySignificand(ध्रुव IEEEFloat &rhs) {
  प्रतिज्ञा(isFiniteNonZero() || category == fcNaN);
  प्रतिज्ञा(rhs.part_गणनम्() >= part_गणनम्());

  APInt::tcAssign(significandParts(), rhs.significandParts(),
                  part_गणनम्());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
शून्य IEEEFloat::makeNaN(तार्किक SNaN, तार्किक Negative, ध्रुव APInt *fill) {
  सचे (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  सचे (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  unsigned अङ्कः_Parts = part_गणनम्();

  APInt fill_storage;
  सचे (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = मिच्छा;
    सचे (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = सच्च;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } अञ्ञथा {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  सचे (!fill || fill->get_अङ्कः_शब्दाः() < अङ्कः_Parts)
    APInt::tcSet(significand, 0, अङ्कः_Parts);
  सचे (fill) {
    APInt::tcAssign(significand, fill->getRawData(),
                    std::min(fill->get_अङ्कः_शब्दाः(), अङ्कः_Parts));

    // Zero out the excess bits of the significand.
    unsigned bitsToPreserve = semantics->precision - 1;
    unsigned part = bitsToPreserve / 64;
    bitsToPreserve %= 64;
    significand[part] &= ((1ULL << bitsToPreserve) - 1);
    अत्थाय (part++; part != अङ्कः_Parts; ++part)
      significand[part] = 0;
  }

  unsigned QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  सचे (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tcClearBit(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    सचे (APInt::tcIsZero(significand, अङ्कः_Parts))
      APInt::tcSetBit(significand, QNaNBit - 1);
  } अञ्ञथा सचे (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } अञ्ञथा {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tcSetBit(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  सचे (semantics == &APFloatBase::semX87DoubleExtended)
    APInt::tcSetBit(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::operator=(ध्रुव IEEEFloat &rhs) {
  सचे (एतत् != &rhs) {
    सचे (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    assign(rhs);
  }

  पटिनिवत्त *एतत्;
}

IEEEFloat &IEEEFloat::operator=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloatBase::semBogus;
  पटिनिवत्त *एतत्;
}

तार्किक IEEEFloat::isDenormal() ध्रुव {
  पटिनिवत्त getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tcExtractBit(significandParts(), semantics->precision - 1) ==
          0);
}

तार्किक IEEEFloat::isSmallest() ध्रुव {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  पटिनिवत्त isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

तार्किक IEEEFloat::isSmallestNormalized() ध्रुव {
  पटिनिवत्त getCategory() == fcNormal && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

unsigned पूर्णाङ्क IEEEFloat::get_अङ्कः_उच्चः_Bits() ध्रुव {
  ध्रुव unsigned पूर्णाङ्क Part_गणनम् = part_गणनम्_For_Bits(semantics->precision);
  ध्रुव unsigned पूर्णाङ्क Bits = Part_गणनम् * integerPartWidth;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  ध्रुव unsigned पूर्णाङ्क अङ्कः_उच्चः_Bits = (semantics->precision > 1)
                                       ? (Bits - semantics->precision + 1)
                                       : (Bits - semantics->precision);
  पटिनिवत्त अङ्कः_उच्चः_Bits;
}

तार्किक IEEEFloat::isSignificandAllOnes() ध्रुव {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  ध्रुव integerPart *Parts = significandParts();
  ध्रुव unsigned Part_गणनम् = part_गणनम्_For_Bits(semantics->precision);
  अत्थाय (unsigned i = 0; i < Part_गणनम् - 1; i++)
    सचे (~Parts[i])
      पटिनिवत्त मिच्छा;

  // Set the unused high bits to all ones when we compare.
  ध्रुव unsigned अङ्कः_उच्चः_Bits = get_अङ्कः_उच्चः_Bits();
  प्रतिज्ञा(अङ्कः_उच्चः_Bits <= integerPartWidth && अङ्कः_उच्चः_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ध्रुव integerPart उच्चः_Bit_Fill =
    ~integerPart(0) << (integerPartWidth - अङ्कः_उच्चः_Bits);
  सचे ((semantics->precision <= 1) || (~(Parts[Part_गणनम् - 1] | उच्चः_Bit_Fill)))
    पटिनिवत्त मिच्छा;

  पटिनिवत्त सच्च;
}

तार्किक IEEEFloat::isSignificandAllOnesExceptLSB() ध्रुव {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  ध्रुव integerPart *Parts = significandParts();

  सचे (Parts[0] & 1)
    पटिनिवत्त मिच्छा;

  ध्रुव unsigned Part_गणनम् = part_गणनम्_For_Bits(semantics->precision);
  अत्थाय (unsigned i = 0; i < Part_गणनम् - 1; i++) {
    सचे (~Parts[i] & ~unsigned{!i})
      पटिनिवत्त मिच्छा;
  }

  // Set the unused high bits to all ones when we compare.
  ध्रुव unsigned अङ्कः_उच्चः_Bits = get_अङ्कः_उच्चः_Bits();
  प्रतिज्ञा(अङ्कः_उच्चः_Bits <= integerPartWidth && अङ्कः_उच्चः_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ध्रुव integerPart उच्चः_Bit_Fill = ~integerPart(0)
                                  << (integerPartWidth - अङ्कः_उच्चः_Bits);
  सचे (~(Parts[Part_गणनम् - 1] | उच्चः_Bit_Fill | 0x1))
    पटिनिवत्त मिच्छा;

  पटिनिवत्त सच्च;
}

तार्किक IEEEFloat::isSignificandAllZeros() ध्रुव {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  ध्रुव integerPart *Parts = significandParts();
  ध्रुव unsigned Part_गणनम् = part_गणनम्_For_Bits(semantics->precision);

  अत्थाय (unsigned i = 0; i < Part_गणनम् - 1; i++)
    सचे (Parts[i])
      पटिनिवत्त मिच्छा;

  // Compute how many bits are used in the final word.
  ध्रुव unsigned अङ्कः_उच्चः_Bits = get_अङ्कः_उच्चः_Bits();
  प्रतिज्ञा(अङ्कः_उच्चः_Bits < integerPartWidth && "Can not have more high bits to "
         "clear than integerPartWidth");
  ध्रुव integerPart उच्चः_Bit_Mask = ~integerPart(0) >> अङ्कः_उच्चः_Bits;

  सचे ((semantics->precision > 1) && (Parts[Part_गणनम् - 1] & उच्चः_Bit_Mask))
    पटिनिवत्त मिच्छा;

  पटिनिवत्त सच्च;
}

तार्किक IEEEFloat::isSignificandAllZerosExceptMSB() ध्रुव {
  ध्रुव integerPart *Parts = significandParts();
  ध्रुव unsigned Part_गणनम् = part_गणनम्_For_Bits(semantics->precision);

  अत्थाय (unsigned i = 0; i < Part_गणनम् - 1; i++) {
    सचे (Parts[i])
      पटिनिवत्त मिच्छा;
  }

  ध्रुव unsigned अङ्कः_उच्चः_Bits = get_अङ्कः_उच्चः_Bits();
  ध्रुव integerPart MSBMask = integerPart(1)
                              << (integerPartWidth - अङ्कः_उच्चः_Bits);
  पटिनिवत्त ((semantics->precision <= 1) || (Parts[Part_गणनम् - 1] == MSBMask));
}

तार्किक IEEEFloat::isLargest() ध्रुव {
  तार्किक IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  सचे (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    पटिनिवत्त (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } अञ्ञथा {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    पटिनिवत्त IsMaxExp && isSignificandAllOnes();
  }
}

तार्किक IEEEFloat::isInteger() ध्रुव {
  // This could be made more efficient; I'm going for obviously correct.
  सचे (!isFinite()) पटिनिवत्त मिच्छा;
  IEEEFloat truncated = *एतत्;
  truncated.roundToIntegral(rmTowardZero);
  पटिनिवत्त compare(truncated) == cmpEqual;
}

तार्किक IEEEFloat::bitwiseIsEqual(ध्रुव IEEEFloat &rhs) ध्रुव {
  सचे (एतत् == &rhs)
    पटिनिवत्त सच्च;
  सचे (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    पटिनिवत्त मिच्छा;
  सचे (category==fcZero || category==fcInfinity)
    पटिनिवत्त सच्च;

  सचे (isFiniteNonZero() && exponent != rhs.exponent)
    पटिनिवत्त मिच्छा;

  पटिनिवत्त std::equal(significandParts(), significandParts() + part_गणनम्(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(ध्रुव fltSemantics &ourSemantics, integerPart मानम्_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fcNormal;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = मानम्_2;
  normalize(rmNearestTiesToEven, lfExactlyZero);
}

IEEEFloat::IEEEFloat(ध्रुव fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(मिच्छा) : makeSmallestNormalized(मिच्छा);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(ध्रुव fltSemantics &ourSemantics, uninitializedTag tag)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(ध्रुव IEEEFloat &rhs) {
  initialize(rhs.semantics);
  assign(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloatBase::semBogus) {
  *एतत् = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

unsigned पूर्णाङ्क IEEEFloat::part_गणनम्() ध्रुव {
  पटिनिवत्त part_गणनम्_For_Bits(semantics->precision + 1);
}

ध्रुव APFloat::integerPart *IEEEFloat::significandParts() ध्रुव {
  पटिनिवत्त const_cast<IEEEFloat *>(एतत्)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  सचे (part_गणनम्() > 1)
    पटिनिवत्त significand.parts;
  अञ्ञथा
    पटिनिवत्त &significand.part;
}

शून्य IEEEFloat::zeroSignificand() {
  APInt::tcSet(significandParts(), 0, part_गणनम्());
}

/* Increment an fcNormal floating point number's significand.  */
शून्य IEEEFloat::incrementSignificand() {
  [[maybe_unused]] integerPart carry =
      APInt::tcIncrement(significandParts(), part_गणनम्());

  /* Our callers should never cause us to overflow.  */
  प्रतिज्ञा(carry == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::addSignificand(ध्रुव IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  प्रतिज्ञा(semantics == rhs.semantics);
  प्रतिज्ञा(exponent == rhs.exponent);

  पटिनिवत्त APInt::tcAdd(parts, rhs.significandParts(), 0, part_गणनम्());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::subtractSignificand(ध्रुव IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  प्रतिज्ञा(semantics == rhs.semantics);
  प्रतिज्ञा(exponent == rhs.exponent);

  पटिनिवत्त APInt::tcSubtract(parts, rhs.significandParts(), borrow,
                           part_गणनम्());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
lostFraction IEEEFloat::multiplySignificand(ध्रुव IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            तार्किक ignoreAddend) {
  integerPart scratch[4];
  तार्किक ignored;

  प्रतिज्ञा(semantics == rhs.semantics);

  unsigned precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  unsigned new_Parts_गणनम् = part_गणनम्_For_Bits(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *fullSignificand =
      new_Parts_गणनम् > 4 ? नूतन integerPart[new_Parts_गणनम्] : scratch;

  integerPart *lhsSignificand = significandParts();
  unsigned parts_गणनम् = part_गणनम्();

  APInt::tcFullMultiply(fullSignificand, lhsSignificand,
                        rhs.significandParts(), parts_गणनम्, parts_गणनम्);

  lostFraction lost_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  unsigned omsb = APInt::tcMSB(fullSignificand, new_Parts_गणनम्) + 1;
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

  सचे (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    ध्रुव fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    unsigned extendedPrecision = 2 * precision + 1;
    सचे (omsb != extendedPrecision - 1) {
      प्रतिज्ञा(extendedPrecision > omsb);
      APInt::tc_Shift_वामः(fullSignificand, new_Parts_गणनम्,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    सचे (new_Parts_गणनम् == 1)
      significand.part = fullSignificand[0];
    अञ्ञथा
      significand.parts = fullSignificand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_unused]] opStatus status = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    प्रतिज्ञा(status == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    lost_fraction = extendedAddend.shift_Significand_दक्षिणः(1);
    प्रतिज्ञा(lost_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    lost_fraction = addOrSubtractSignificand(extendedAddend, मिच्छा);

    /* Restore our state.  */
    सचे (new_Parts_गणनम् == 1)
      fullSignificand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(fullSignificand, new_Parts_गणनम्) + 1;
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
  सचे (omsb > precision) {
    unsigned पूर्णाङ्क bits, significantParts;
    lostFraction lf;

    bits = omsb - precision;
    significantParts = part_गणनम्_For_Bits(omsb);
    lf = shift_दक्षिणः(fullSignificand, significantParts, bits);
    lost_fraction = combineLostFractions(lf, lost_fraction);
    exponent += bits;
  }

  APInt::tcAssign(lhsSignificand, fullSignificand, parts_गणनम्);

  सचे (new_Parts_गणनम् > 4)
    delete [] fullSignificand;

  पटिनिवत्त lost_fraction;
}

lostFraction IEEEFloat::multiplySignificand(ध्रुव IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  पटिनिवत्त multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
lostFraction IEEEFloat::divideSignificand(ध्रुव IEEEFloat &rhs) {
  integerPart scratch[4];

  प्रतिज्ञा(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  ध्रुव integerPart *rhsSignificand = rhs.significandParts();
  unsigned parts_गणनम् = part_गणनम्();

  integerPart *dividend =
      parts_गणनम् > 2 ? नूतन integerPart[parts_गणनम् * 2] : scratch;
  integerPart *भाजकः = dividend + parts_गणनम्;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  अत्थाय (unsigned i = 0; i < parts_गणनम्; i++) {
    dividend[i] = lhsSignificand[i];
    भाजकः[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  unsigned पूर्णाङ्क precision = semantics->precision;

  /* Normalize the divisor.  */
  unsigned bit = precision - APInt::tcMSB(भाजकः, parts_गणनम्) - 1;
  सचे (bit) {
    exponent += bit;
    APInt::tc_Shift_वामः(भाजकः, parts_गणनम्, bit);
  }

  /* Normalize the dividend.  */
  bit = precision - APInt::tcMSB(dividend, parts_गणनम्) - 1;
  सचे (bit) {
    exponent -= bit;
    APInt::tc_Shift_वामः(dividend, parts_गणनम्, bit);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  सचे (APInt::tcCompare(dividend, भाजकः, parts_गणनम्) < 0) {
    exponent--;
    APInt::tc_Shift_वामः(dividend, parts_गणनम्, 1);
    प्रतिज्ञा(APInt::tcCompare(dividend, भाजकः, parts_गणनम्) >= 0);
  }

  /* Long division.  */
  अत्थाय (bit = precision; bit; bit -= 1) {
    सचे (APInt::tcCompare(dividend, भाजकः, parts_गणनम्) >= 0) {
      APInt::tcSubtract(dividend, भाजकः, 0, parts_गणनम्);
      APInt::tcSetBit(lhsSignificand, bit - 1);
    }

    APInt::tc_Shift_वामः(dividend, parts_गणनम्, 1);
  }

  /* Figure out the lost fraction.  */
  पूर्णाङ्क cmp = APInt::tcCompare(dividend, भाजकः, parts_गणनम्);

  lostFraction lost_fraction;
  सचे (cmp > 0)
    lost_fraction = lfMoreThanHalf;
  अञ्ञथा सचे (cmp == 0)
    lost_fraction = lfExactlyHalf;
  अञ्ञथा सचे (APInt::tcIsZero(dividend, parts_गणनम्))
    lost_fraction = lfExactlyZero;
  अञ्ञथा
    lost_fraction = lfLessThanHalf;

  सचे (parts_गणनम् > 2)
    delete [] dividend;

  पटिनिवत्त lost_fraction;
}

unsigned पूर्णाङ्क IEEEFloat::significandMSB() ध्रुव {
  पटिनिवत्त APInt::tcMSB(significandParts(), part_गणनम्());
}

unsigned पूर्णाङ्क IEEEFloat::significandLSB() ध्रुव {
  पटिनिवत्त APInt::tcLSB(significandParts(), part_गणनम्());
}

/* Note that a zero result is NOT normalized to fcZero.  */
lostFraction IEEEFloat::shift_Significand_दक्षिणः(unsigned पूर्णाङ्क bits) {
  /* Our exponent should not overflow.  */
  प्रतिज्ञा((ExponentType) (exponent + bits) >= exponent);

  exponent += bits;

  पटिनिवत्त shift_दक्षिणः(significandParts(), part_गणनम्(), bits);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
शून्य IEEEFloat::shift_Significand_वामः(unsigned पूर्णाङ्क bits) {
  प्रतिज्ञा(bits < semantics->precision ||
         (semantics->precision == 1 && bits <= 1));

  सचे (bits) {
    unsigned पूर्णाङ्क parts_गणनम् = part_गणनम्();

    APInt::tc_Shift_वामः(significandParts(), parts_गणनम्, bits);
    exponent -= bits;

    प्रतिज्ञा(!APInt::tcIsZero(significandParts(), parts_गणनम्));
  }
}

APFloat::cmpResult IEEEFloat::compare_Absolute_मानम्(ध्रुव IEEEFloat &rhs) ध्रुव {
  प्रतिज्ञा(semantics == rhs.semantics);
  प्रतिज्ञा(isFiniteNonZero());
  प्रतिज्ञा(rhs.isFiniteNonZero());

  पूर्णाङ्क compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  सचे (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_गणनम्());

  सचे (compare > 0)
    पटिनिवत्त cmpGreaterThan;
  अञ्ञथा सचे (compare < 0)
    पटिनिवत्त cmpLessThan;
  अञ्ञथा
    पटिनिवत्त cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
स्थिर शून्य tcSetLeastSignificantBits(APInt::WordType *dst, unsigned parts,
                                      unsigned bits) {
  unsigned i = 0;
  याव (bits > APInt::APINT_BITS_PER_WORD) {
    dst[i++] = ~(APInt::WordType)0;
    bits -= APInt::APINT_BITS_PER_WORD;
  }

  सचे (bits)
    dst[i++] = ~(APInt::WordType)0 >> (APInt::APINT_BITS_PER_WORD - bits);

  याव (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::opStatus IEEEFloat::handleOverflow(roundingMode rounding_mode) {
  सचे (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    सचे (rounding_mode == rmNearestTiesToEven ||
        rounding_mode == rmNearestTiesToAway ||
        (rounding_mode == rmTowardPositive && !sign) ||
        (rounding_mode == rmTowardNegative && sign)) {
      सचे (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(मिच्छा, sign);
      अञ्ञथा
        category = fcInfinity;
      पटिनिवत्त static_cast<opStatus>(opOverflow | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fcNormal;
  exponent = semantics->maxExponent;
  tcSetLeastSignificantBits(significandParts(), part_गणनम्(),
                            semantics->precision);
  सचे (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tcClearBit(significandParts(), 0);

  पटिनिवत्त opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
तार्किक IEEEFloat::roundAwayFromZero(roundingMode rounding_mode,
                                  lostFraction lost_fraction,
                                  unsigned पूर्णाङ्क bit) ध्रुव {
  /* NaNs and infinities should not have lost fractions.  */
  प्रतिज्ञा(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  प्रतिज्ञा(lost_fraction != lfExactlyZero);

  विकल्प (rounding_mode) {
  पक्ष rmNearestTiesToAway:
    पटिनिवत्त lost_fraction == lfExactlyHalf || lost_fraction == lfMoreThanHalf;

  पक्ष rmNearestTiesToEven:
    सचे (lost_fraction == lfMoreThanHalf)
      पटिनिवत्त सच्च;

    /* Our zeroes don't have a significand to test.  */
    सचे (lost_fraction == lfExactlyHalf && category != fcZero)
      पटिनिवत्त APInt::tcExtractBit(significandParts(), bit);

    पटिनिवत्त मिच्छा;

  पक्ष rmTowardZero:
    पटिनिवत्त मिच्छा;

  पक्ष rmTowardPositive:
    पटिनिवत्त !sign;

  पक्ष rmTowardNegative:
    पटिनिवत्त sign;

  स्वभाव:
    भङ्ग;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::opStatus IEEEFloat::normalize(roundingMode rounding_mode,
                                       lostFraction lost_fraction) {
  सचे (!isFiniteNonZero())
    पटिनिवत्त opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  unsigned omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  सचे (omsb || lost_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    पूर्णाङ्क exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    सचे (exponent + exponentChange > semantics->maxExponent)
      पटिनिवत्त handleOverflow(rounding_mode);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    सचे (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    सचे (exponentChange < 0) {
      प्रतिज्ञा(lost_fraction == lfExactlyZero);

      shift_Significand_वामः(-exponentChange);

      पटिनिवत्त opOK;
    }

    सचे (exponentChange > 0) {
      lostFraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = shift_Significand_दक्षिणः(exponentChange);

      lost_fraction = combineLostFractions(lf, lost_fraction);

      /* Keep OMSB up-to-date.  */
      सचे (omsb > (unsigned) exponentChange)
        omsb -= exponentChange;
      अञ्ञथा
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  सचे (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    पटिनिवत्त handleOverflow(rounding_mode);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  सचे (lost_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    सचे (omsb == 0) {
      category = fcZero;
      सचे (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = मिच्छा;
      सचे (!semantics->hasZero)
        makeSmallestNormalized(मिच्छा);
    }

    पटिनिवत्त opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  सचे (roundAwayFromZero(rounding_mode, lost_fraction, 0)) {
    सचे (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    सचे (omsb == (unsigned) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      सचे (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        पटिनिवत्त handleOverflow(sign ? rmTowardNegative : rmTowardPositive);

      shift_Significand_दक्षिणः(1);

      पटिनिवत्त opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    सचे (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      पटिनिवत्त handleOverflow(rounding_mode);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  सचे (omsb == semantics->precision)
    पटिनिवत्त opInexact;

  /* We have a non-zero denormal.  */
  प्रतिज्ञा(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  सचे (omsb == 0) {
    category = fcZero;
    सचे (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = मिच्छा;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    सचे (!semantics->hasZero)
      makeSmallestNormalized(मिच्छा);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  पटिनिवत्त (opStatus) (opUnderflow | opInexact);
}

APFloat::opStatus IEEEFloat::addOrSubtractSpecials(ध्रुव IEEEFloat &rhs,
                                                   तार्किक subtract) {
  विकल्प (Pack_Categories_Into_कुञ्चिका(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(nullptr);

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNaN):
    सचे (isSignaling()) {
      makeQuiet();
      पटिनिवत्त opInvalidOp;
    }
    पटिनिवत्त rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
    पटिनिवत्त opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ subtract;
    पटिनिवत्त opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNormal):
    assign(rhs);
    sign = rhs.sign ^ subtract;
    पटिनिवत्त opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    पटिनिवत्त opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    सचे (((sign ^ rhs.sign)!=0) != subtract) {
      makeNaN();
      पटिनिवत्त opInvalidOp;
    }

    पटिनिवत्त opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNormal):
    पटिनिवत्त opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
lostFraction IEEEFloat::addOrSubtractSignificand(ध्रुव IEEEFloat &rhs,
                                                 तार्किक subtract) {
  [[maybe_unused]] integerPart carry = 0;
  lostFraction lost_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  subtract ^= static_cast<तार्किक>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  पूर्णाङ्क bits = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  सचे (subtract) {
    सचे ((bits < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat temp_rhs(rhs);
    तार्किक lost_fraction_is_from_rhs = मिच्छा;

    सचे (bits == 0)
      lost_fraction = lfExactlyZero;
    अञ्ञथा सचे (bits > 0) {
      lost_fraction = temp_rhs.shift_Significand_दक्षिणः(bits - 1);
      lost_fraction_is_from_rhs = सच्च;
      shift_Significand_वामः(1);
    } अञ्ञथा {
      lost_fraction = shift_Significand_दक्षिणः(-bits - 1);
      temp_rhs.shift_Significand_वामः(1);
    }

    // Should we reverse the subtraction.
    cmpResult cmp_result = compare_Absolute_मानम्(temp_rhs);
    सचे (cmp_result == cmpLessThan) {
      तार्किक borrow =
          lost_fraction != lfExactlyZero && !lost_fraction_is_from_rhs;
      सचे (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        सचे (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        अञ्ञथा सचे (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = temp_rhs.subtractSignificand(*एतत्, borrow);
      copySignificand(temp_rhs);
      sign = !sign;
    } अञ्ञथा सचे (cmp_result == cmpGreaterThan) {
      तार्किक borrow = lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs;
      सचे (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        सचे (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        अञ्ञथा सचे (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = subtractSignificand(temp_rhs, borrow);
    } अञ्ञथा { // cmpEqual
      zeroSignificand();
      सचे (lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    प्रतिज्ञा(!carry);
  } अञ्ञथा {
    सचे (bits > 0) {
      IEEEFloat temp_rhs(rhs);

      lost_fraction = temp_rhs.shift_Significand_दक्षिणः(bits);
      carry = addSignificand(temp_rhs);
    } अञ्ञथा {
      lost_fraction = shift_Significand_दक्षिणः(-bits);
      carry = addSignificand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    प्रतिज्ञा(!carry);
  }

  पटिनिवत्त lost_fraction;
}

APFloat::opStatus IEEEFloat::multiplySpecials(ध्रुव IEEEFloat &rhs) {
  विकल्प (Pack_Categories_Into_कुञ्चिका(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(nullptr);

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNaN):
    assign(rhs);
    sign = मिच्छा;
    [[fallthrough]];
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    सचे (isSignaling()) {
      makeQuiet();
      पटिनिवत्त opInvalidOp;
    }
    पटिनिवत्त rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
    category = fcInfinity;
    पटिनिवत्त opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    category = fcZero;
    पटिनिवत्त opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
    makeNaN();
    पटिनिवत्त opInvalidOp;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNormal):
    पटिनिवत्त opOK;
  }
}

APFloat::opStatus IEEEFloat::divideSpecials(ध्रुव IEEEFloat &rhs) {
  विकल्प (Pack_Categories_Into_कुञ्चिका(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(nullptr);

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNaN):
    assign(rhs);
    sign = मिच्छा;
    [[fallthrough]];
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    सचे (isSignaling()) {
      makeQuiet();
      पटिनिवत्त opInvalidOp;
    }
    पटिनिवत्त rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNormal):
    पटिनिवत्त opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcInfinity):
    category = fcZero;
    पटिनिवत्त opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcZero):
    सचे (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(मिच्छा, sign);
    अञ्ञथा
      category = fcInfinity;
    पटिनिवत्त opDivByZero;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    makeNaN();
    पटिनिवत्त opInvalidOp;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNormal):
    पटिनिवत्त opOK;
  }
}

APFloat::opStatus IEEEFloat::modSpecials(ध्रुव IEEEFloat &rhs) {
  विकल्प (Pack_Categories_Into_कुञ्चिका(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(nullptr);

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNaN):
    सचे (isSignaling()) {
      makeQuiet();
      पटिनिवत्त opInvalidOp;
    }
    पटिनिवत्त rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcInfinity):
    पटिनिवत्त opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    makeNaN();
    पटिनिवत्त opInvalidOp;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNormal):
    पटिनिवत्त opOK;
  }
}

APFloat::opStatus IEEEFloat::remainderSpecials(ध्रुव IEEEFloat &rhs) {
  विकल्प (Pack_Categories_Into_कुञ्चिका(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(nullptr);

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNaN):
    सचे (isSignaling()) {
      makeQuiet();
      पटिनिवत्त opInvalidOp;
    }
    पटिनिवत्त rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcInfinity):
    पटिनिवत्त opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    makeNaN();
    पटिनिवत्त opInvalidOp;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNormal):
    पटिनिवत्त opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
शून्य IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  सचे (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    पटिनिवत्त;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::opStatus IEEEFloat::addOrSubtract(ध्रुव IEEEFloat &rhs,
                                           roundingMode rounding_mode,
                                           तार्किक subtract) {
  opStatus fs = addOrSubtractSpecials(rhs, subtract);

  /* This return code means it was not a simple case.  */
  सचे (fs == opDivByZero) {
    lostFraction lost_fraction;

    lost_fraction = addOrSubtractSignificand(rhs, subtract);
    fs = normalize(rounding_mode, lost_fraction);

    /* Can only be zero if we lost no fraction.  */
    प्रतिज्ञा(category != fcZero || lost_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  सचे (category == fcZero) {
    सचे (rhs.category != fcZero || (sign == rhs.sign) == subtract)
      sign = (rounding_mode == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    सचे (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = मिच्छा;
  }

  पटिनिवत्त fs;
}

/* Normalized addition.  */
APFloat::opStatus IEEEFloat::add(ध्रुव IEEEFloat &rhs,
                                 roundingMode rounding_mode) {
  पटिनिवत्त addOrSubtract(rhs, rounding_mode, मिच्छा);
}

/* Normalized subtraction.  */
APFloat::opStatus IEEEFloat::subtract(ध्रुव IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  पटिनिवत्त addOrSubtract(rhs, rounding_mode, सच्च);
}

/* Normalized multiply.  */
APFloat::opStatus IEEEFloat::multiply(ध्रुव IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = multiplySpecials(rhs);

  सचे (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = मिच्छा;
  सचे (isFiniteNonZero()) {
    lostFraction lost_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    सचे (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  पटिनिवत्त fs;
}

/* Normalized divide.  */
APFloat::opStatus IEEEFloat::divide(ध्रुव IEEEFloat &rhs,
                                    roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = divideSpecials(rhs);

  सचे (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = मिच्छा;
  सचे (isFiniteNonZero()) {
    lostFraction lost_fraction = divideSignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    सचे (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  पटिनिवत्त fs;
}

/* Normalized remainder.  */
APFloat::opStatus IEEEFloat::remainder(ध्रुव IEEEFloat &rhs) {
  unsigned पूर्णाङ्क origSign = sign;

  // First handle the special cases.
  opStatus fs = remainderSpecials(rhs);
  सचे (fs != opDivByZero)
    पटिनिवत्त fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  सचे (P2.add(rhs, rmNearestTiesToEven) == opOK) {
    fs = mod(P2);
    प्रतिज्ञा(fs == opOK);
  }

  // Lets work with absolute numbers.
  IEEEFloat P = rhs;
  P.sign = मिच्छा;
  sign = मिच्छा;

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
  तार्किक losesInfo;
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.maxExponent++;
  extendedSemantics.minExponent--;
  extendedSemantics.precision += 2;

  IEEEFloat VEx = *एतत्;
  fs = VEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  प्रतिज्ञा(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  प्रतिज्ञा(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.add(VEx, rmNearestTiesToEven);
  प्रतिज्ञा(fs == opOK);

  सचे (VEx.compare(PEx) == cmpGreaterThan) {
    fs = subtract(P, rmNearestTiesToEven);
    प्रतिज्ञा(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    प्रतिज्ञा(fs == opOK);
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    प्रतिज्ञा(fs == opOK);

    cmpResult result = VEx.compare(PEx);
    सचे (result == cmpGreaterThan || result == cmpEqual) {
      fs = subtract(P, rmNearestTiesToEven);
      प्रतिज्ञा(fs == opOK);
    }
  }

  सचे (isZero()) {
    sign = origSign;    // IEEE754 requires this
    सचे (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = मिच्छा;
  } अञ्ञथा {
    sign ^= origSign;
  }
  पटिनिवत्त fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::opStatus IEEEFloat::mod(ध्रुव IEEEFloat &rhs) {
  opStatus fs = modSpecials(rhs);
  unsigned पूर्णाङ्क origSign = sign;

  याव (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_मानम्(rhs) != cmpLessThan) {
    पूर्णाङ्क Exp = ilogb(*एतत्) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rmNearestTiesToEven);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    सचे (V.isNaN() || compare_Absolute_मानम्(V) == cmpLessThan)
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
    सचे (!semantics->hasZero && एतत्->isSmallest())
      भङ्ग;

    प्रतिज्ञा(fs==opOK);
  }
  सचे (isZero()) {
    sign = origSign; // fmod requires this
    सचे (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = मिच्छा;
  }
  पटिनिवत्त fs;
}

/* Normalized fused-multiply-add.  */
APFloat::opStatus IEEEFloat::fusedMultiplyAdd(ध्रुव IEEEFloat &multiplicand,
                                              ध्रुव IEEEFloat &addend,
                                              roundingMode rounding_mode) {
  opStatus fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  सचे (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    lostFraction lost_fraction;

    lost_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_mode, lost_fraction);
    सचे (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    सचे (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_mode == rmTowardNegative);
      सचे (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = मिच्छा;
    }
  } अञ्ञथा {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    सचे (fs == opOK)
      fs = addOrSubtract(addend, rounding_mode, मिच्छा);
  }

  पटिनिवत्त fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::opStatus IEEEFloat::roundToIntegral(roundingMode rounding_mode) {
  सचे (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    पटिनिवत्त opOK;

  सचे (isNaN()) {
    सचे (isSignaling()) {
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
      पटिनिवत्त opInvalidOp;
    } अञ्ञथा {
      // [IEEE Std 754-2008 6.2]:
      // For an operation with quiet NaN inputs, other than maximum and minimum
      // operations, if a floating-point result is to be delivered the result
      // shall be a quiet NaN which should be one of the input NaNs.
      // ...
      // Every general-computational and quiet-computational operation involving
      // one or more input NaNs, none of them signaling, shall signal no
      // exception, except fusedMultiplyAdd might signal the invalid operation
      // exception(see 7.2).
      पटिनिवत्त opOK;
    }
  }

  सचे (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    पटिनिवत्त opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  सचे (exponent + 1 >= (पूर्णाङ्क)APFloat::semanticsPrecision(*semantics))
    पटिनिवत्त opOK;

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
  opStatus fs = MagicConstant.convertFromAPInt(IntegerConstant, मिच्छा,
                                               rmNearestTiesToEven);
  प्रतिज्ञा(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  तार्किक निवेशः_Sign = isNegative();

  fs = add(MagicConstant, rounding_mode);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  subtract(MagicConstant, rounding_mode);

  // Restore the input sign.
  सचे (निवेशः_Sign != isNegative())
    changeSign();

  पटिनिवत्त fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmpResult IEEEFloat::compare(ध्रुव IEEEFloat &rhs) ध्रुव {
  प्रतिज्ञा(semantics == rhs.semantics);

  विकल्प (Pack_Categories_Into_कुञ्चिका(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(nullptr);

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNaN):
    पटिनिवत्त cmpUnordered;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcZero):
    सचे (sign)
      पटिनिवत्त cmpLessThan;
    अञ्ञथा
      पटिनिवत्त cmpGreaterThan;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNormal):
    सचे (rhs.sign)
      पटिनिवत्त cmpGreaterThan;
    अञ्ञथा
      पटिनिवत्त cmpLessThan;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
    सचे (sign == rhs.sign)
      पटिनिवत्त cmpEqual;
    अञ्ञथा सचे (sign)
      पटिनिवत्त cmpLessThan;
    अञ्ञथा
      पटिनिवत्त cmpGreaterThan;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    पटिनिवत्त cmpEqual;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNormal):
    भङ्ग;
  }

  cmpResult result;
  /* Two normal numbers.  Do they have the same sign?  */
  सचे (sign != rhs.sign) {
    सचे (sign)
      result = cmpLessThan;
    अञ्ञथा
      result = cmpGreaterThan;
  } अञ्ञथा {
    /* Compare absolute values; invert result if negative.  */
    result = compare_Absolute_मानम्(rhs);

    सचे (sign) {
      सचे (result == cmpLessThan)
        result = cmpGreaterThan;
      अञ्ञथा सचे (result == cmpGreaterThan)
        result = cmpLessThan;
    }
  }

  पटिनिवत्त result;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::opStatus IEEEFloat::convert(ध्रुव fltSemantics &toSemantics,
                                     roundingMode rounding_mode,
                                     तार्किक *losesInfo) {
  opStatus fs;
  ध्रुव fltSemantics &fromSemantics = *semantics;
  तार्किक is_signaling = isSignaling();

  lostFraction lostFraction = lfExactlyZero;
  unsigned new_Part_गणनम् = part_गणनम्_For_Bits(toSemantics.precision + 1);
  unsigned old_Part_गणनम् = part_गणनम्();
  पूर्णाङ्क shift = toSemantics.precision - fromSemantics.precision;

  तार्किक X86SpecialNan = मिच्छा;
  सचे (&fromSemantics == &APFloatBase::semX87DoubleExtended &&
      &toSemantics != &APFloatBase::semX87DoubleExtended && category == fcNaN &&
      (!(*significandParts() & 0x8000000000000000ULL) ||
       !(*significandParts() & 0x4000000000000000ULL))) {
    // x86 has some unusual NaNs which cannot be represented in any other
    // format; note them here.
    X86SpecialNan = सच्च;
  }

  // If this is a truncation of a denormal number, and the target semantics
  // has larger exponent range than the source semantics (this can happen
  // when truncating from PowerPC double-double to double format), the
  // right shift could lose result mantissa bits.  Adjust exponent instead
  // of performing excessive shift.
  // Also do a similar trick in case shifting denormal would produce zero
  // significand as this case isn't handled correctly by normalize.
  सचे (shift < 0 && isFiniteNonZero()) {
    पूर्णाङ्क omsb = significandMSB() + 1;
    पूर्णाङ्क exponentChange = omsb - fromSemantics.precision;
    सचे (exponent + exponentChange < toSemantics.minExponent)
      exponentChange = toSemantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, shift);
    सचे (exponentChange < 0) {
      shift -= exponentChange;
      exponent += exponentChange;
    } अञ्ञथा सचे (omsb <= -shift) {
      exponentChange = omsb + shift - 1; // leave at least one bit set
      shift -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  सचे (shift < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    lostFraction = shift_दक्षिणः(significandParts(), old_Part_गणनम्, -shift);

  // Fix the storage so it can hold to new value.
  सचे (new_Part_गणनम् > old_Part_गणनम्) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = नूतन integerPart[new_Part_गणनम्];
    APInt::tcSet(newParts, 0, new_Part_गणनम्);
    सचे (isFiniteNonZero() || category==fcNaN)
      APInt::tcAssign(newParts, significandParts(), old_Part_गणनम्);
    freeSignificand();
    significand.parts = newParts;
  } अञ्ञथा सचे (new_Part_गणनम् == 1 && old_Part_गणनम् != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    सचे (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &toSemantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  सचे (shift > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_Shift_वामः(significandParts(), new_Part_गणनम्, shift);

  सचे (isFiniteNonZero()) {
    fs = normalize(rounding_mode, lostFraction);
    *losesInfo = (fs != opOK);
  } अञ्ञथा सचे (category == fcNaN) {
    सचे (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(मिच्छा, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } अञ्ञथा {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      सचे (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(मिच्छा, मिच्छा);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      सचे (!APFloat::hasSignificand(fromSemantics))
        makeNaN(मिच्छा, sign);

      *losesInfo = lostFraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      सचे (!X86SpecialNan && semantics == &APFloatBase::semX87DoubleExtended)
        APInt::tcSetBit(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      सचे (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } अञ्ञथा {
        fs = opOK;
      }
    }
  } अञ्ञथा सचे (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(मिच्छा, sign);
    *losesInfo = सच्च;
    fs = opInexact;
  } अञ्ञथा सचे (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = मिच्छा;
  } अञ्ञथा {
    *losesInfo = मिच्छा;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  सचे ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = सच्च;
    सचे (fs == opOK)
      fs = opInexact;
  }

  सचे (category == fcZero && !semantics->hasZero)
    makeSmallestNormalized(मिच्छा);
  पटिनिवत्त fs;
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
    MutableArrayRef<integerPart> parts, unsigned पूर्णाङ्क width, तार्किक isSigned,
    roundingMode rounding_mode, तार्किक *isExact) ध्रुव {
  *isExact = मिच्छा;

  /* Handle the three special cases first.  */
  सचे (category == fcInfinity || category == fcNaN)
    पटिनिवत्त opInvalidOp;

  unsigned dst_Parts_गणनम् = part_गणनम्_For_Bits(width);
  प्रतिज्ञा(dst_Parts_गणनम् <= parts.size() && "Integer too big");

  सचे (category == fcZero) {
    APInt::tcSet(parts.data(), 0, dst_Parts_गणनम्);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    पटिनिवत्त opOK;
  }

  ध्रुव integerPart *src = significandParts();

  unsigned truncatedBits;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  सचे (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tcSet(parts.data(), 0, dst_Parts_गणनम्);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncatedBits = semantics->precision -1U - exponent;
  } अञ्ञथा {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    unsigned पूर्णाङ्क bits = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    सचे (bits > width)
      पटिनिवत्त opInvalidOp;

    सचे (bits < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncatedBits = semantics->precision - bits;
      APInt::tcExtract(parts.data(), dst_Parts_गणनम्, src, bits, truncatedBits);
    } अञ्ञथा {
      /* We want at least as many bits as are available.  */
      APInt::tcExtract(parts.data(), dst_Parts_गणनम्, src, semantics->precision,
                       0);
      APInt::tc_Shift_वामः(parts.data(), dst_Parts_गणनम्,
                         bits - semantics->precision);
      truncatedBits = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  lostFraction lost_fraction;
  सचे (truncatedBits) {
    lost_fraction = lostFractionThroughTruncation(src, part_गणनम्(),
                                                  truncatedBits);
    सचे (lost_fraction != lfExactlyZero &&
        roundAwayFromZero(rounding_mode, lost_fraction, truncatedBits)) {
      सचे (APInt::tcIncrement(parts.data(), dst_Parts_गणनम्))
        पटिनिवत्त opInvalidOp;     /* Overflow.  */
    }
  } अञ्ञथा {
    lost_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  unsigned पूर्णाङ्क omsb = APInt::tcMSB(parts.data(), dst_Parts_गणनम्) + 1;

  सचे (sign) {
    सचे (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      सचे (omsb != 0)
        पटिनिवत्त opInvalidOp;
    } अञ्ञथा {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      सचे (omsb == width &&
          APInt::tcLSB(parts.data(), dst_Parts_गणनम्) + 1 != omsb)
        पटिनिवत्त opInvalidOp;

      /* This case can happen because of rounding.  */
      सचे (omsb > width)
        पटिनिवत्त opInvalidOp;
    }

    APInt::tcNegate (parts.data(), dst_Parts_गणनम्);
  } अञ्ञथा {
    सचे (omsb >= width + !isSigned)
      पटिनिवत्त opInvalidOp;
  }

  सचे (lost_fraction == lfExactlyZero) {
    *isExact = सच्च;
    पटिनिवत्त opOK;
  }
  पटिनिवत्त opInexact;
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
                            unsigned पूर्णाङ्क width, तार्किक isSigned,
                            roundingMode rounding_mode, तार्किक *isExact) ध्रुव {
  opStatus fs = convertToSignExtendedInteger(parts, width, isSigned,
                                             rounding_mode, isExact);

  सचे (fs == opInvalidOp) {
    unsigned पूर्णाङ्क bits, dst_Parts_गणनम्;

    dst_Parts_गणनम् = part_गणनम्_For_Bits(width);
    प्रतिज्ञा(dst_Parts_गणनम् <= parts.size() && "Integer too big");

    सचे (category == fcNaN)
      bits = 0;
    अञ्ञथा सचे (sign)
      bits = isSigned;
    अञ्ञथा
      bits = width - isSigned;

    tcSetLeastSignificantBits(parts.data(), dst_Parts_गणनम्, bits);
    सचे (sign && isSigned)
      APInt::tc_Shift_वामः(parts.data(), dst_Parts_गणनम्, width - 1);
  }

  पटिनिवत्त fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::opStatus IEEEFloat::convertFromUnsignedParts(
    ध्रुव integerPart *src, unsigned पूर्णाङ्क src_गणनम्, roundingMode rounding_mode) {
  category = fcNormal;
  unsigned omsb = APInt::tcMSB(src, src_गणनम्) + 1;
  integerPart *dst = significandParts();
  unsigned dst_गणनम् = part_गणनम्();
  unsigned precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  lostFraction lost_fraction;
  सचे (precision <= omsb) {
    exponent = omsb - 1;
    lost_fraction = lostFractionThroughTruncation(src, src_गणनम्,
                                                  omsb - precision);
    APInt::tcExtract(dst, dst_गणनम्, src, precision, omsb - precision);
  } अञ्ञथा {
    exponent = precision - 1;
    lost_fraction = lfExactlyZero;
    APInt::tcExtract(dst, dst_गणनम्, src, omsb, 0);
  }

  पटिनिवत्त normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus IEEEFloat::convertFromAPInt(ध्रुव APInt &Val, तार्किक isSigned,
                                              roundingMode rounding_mode) {
  unsigned पूर्णाङ्क part_गणनम् = Val.get_अङ्कः_शब्दाः();
  APInt api = Val;

  sign = मिच्छा;
  सचे (isSigned && api.isNegative()) {
    sign = सच्च;
    api = -api;
  }

  पटिनिवत्त convertFromUnsignedParts(api.getRawData(), part_गणनम्, rounding_mode);
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        roundingMode rounding_mode) {
  lostFraction lost_fraction = lfExactlyZero;

  category = fcNormal;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  unsigned parts_गणनम् = part_गणनम्();
  unsigned bitPos = parts_गणनम् * integerPartWidth;
  तार्किक computedTrailingFraction = मिच्छा;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator end = s.end();
  StringRef::iterator dot;
  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  सचे (!PtrOrErr)
    पटिनिवत्त PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator firstSignificantDigit = p;

  याव (p != end) {
    integerPart hex_मानम्;

    सचे (*p == '.') {
      सचे (dot != end)
        पटिनिवत्त createError("String contains multiple dots");
      dot = p++;
      अनुवर्त;
    }

    hex_मानम् = hex_Digit_मानम्(*p);
    सचे (hex_मानम् == UINT_MAX)
      भङ्ग;

    p++;

    // Store the number while we have space.
    सचे (bitPos) {
      bitPos -= 4;
      hex_मानम् <<= bitPos % integerPartWidth;
      significand[bitPos / integerPartWidth] |= hex_मानम्;
    } अञ्ञथा सचे (!computedTrailingFraction) {
      auto FractOrErr = trailingHexadecimalFraction(p, end, hex_मानम्);
      सचे (!FractOrErr)
        पटिनिवत्त FractOrErr.takeError();
      lost_fraction = *FractOrErr;
      computedTrailingFraction = सच्च;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  सचे (p == end)
    पटिनिवत्त createError("Hex strings require an exponent");
  सचे (*p != 'p' && *p != 'P')
    पटिनिवत्त createError("Invalid character in significand");
  सचे (p == begin)
    पटिनिवत्त createError("Significand has no digits");
  सचे (dot != end && p - begin == 1)
    पटिनिवत्त createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  सचे (p != firstSignificantDigit) {
    पूर्णाङ्क expAdjustment;

    /* Implicit hexadecimal point?  */
    सचे (dot == end)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<पूर्णाङ्क>(dot - firstSignificantDigit);
    सचे (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_गणनम् * integerPartWidth;

    /* Adjust for the given exponent.  */
    auto ExpOrErr = totalExponent(p + 1, end, expAdjustment);
    सचे (!ExpOrErr)
      पटिनिवत्त ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  पटिनिवत्त normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus
IEEEFloat::roundSignificandWithExponent(ध्रुव integerPart *decSigParts,
                                        unsigned sig_Part_गणनम्, पूर्णाङ्क exp,
                                        roundingMode rounding_mode) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  unsigned पूर्णाङ्क power = exp >= 0 ? exp : -exp;
  unsigned पूर्णाङ्क pow5PartCapacity = powerOfFivePartsForExponent(power);
  SmallVector<integerPart, अङ्कः_Pow5_Parts_On_राशिः> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5PartCapacity);

  तार्किक isNearest = rounding_mode == rmNearestTiesToEven ||
                   rounding_mode == rmNearestTiesToAway;

  unsigned parts = part_गणनम्_For_Bits(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  unsigned pow5_Part_गणनम् = powerOf5(pow5Parts.data(), power);

  अत्थाय (;; parts *= 2) {
    unsigned पूर्णाङ्क excessPrecision, truncatedBits;

    calcSemantics.precision = parts * integerPartWidth - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncatedBits = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    opStatus sigStatus = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_गणनम्, rmNearestTiesToEven);
    opStatus powStatus = pow5.convertFromUnsignedParts(
        pow5Parts.data(), pow5_Part_गणनम्, rmNearestTiesToEven);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += exp;

    lostFraction calcLostFraction;
    integerPart HUerr, HUdistance;
    unsigned पूर्णाङ्क powHUerr;

    सचे (exp >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calcLostFraction = decSig.multiplySignificand(pow5);
      powHUerr = powStatus != opOK;
    } अञ्ञथा {
      calcLostFraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      सचे (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncatedBits = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (powStatus == opOK && calcLostFraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    प्रतिज्ञा(APInt::tcExtractBit
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calcLostFraction != lfExactlyZero, sigStatus != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, isNearest);

    /* Are we guaranteed to round correctly if we truncate?  */
    सचे (HUdistance >= HUerr) {
      APInt::tcExtract(significandParts(), part_गणनम्(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calcLostFraction = lostFractionThroughTruncation(decSig.significandParts(),
                                                       decSig.part_गणनम्(),
                                                       truncatedBits);
      पटिनिवत्त static_cast<opStatus>(normalize(rounding_mode, calcLostFraction) |
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
  सचे (Error Err = interpretDecimal(p, str.end(), &D))
    पटिनिवत्त std::move(Err);

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
  सचे (D.firstSigDigit == str.end() || dec_Digit_मानम्(*D.firstSigDigit) >= 10U) {
    category = fcZero;
    fs = opOK;
    सचे (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = मिच्छा;
    सचे (!semantics->hasZero)
      makeSmallestNormalized(मिच्छा);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } अञ्ञथा सचे (D.normalizedExponent - 1 > INT_MAX / 42039) {
    fs = handleOverflow(rounding_mode);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } अञ्ञथा सचे (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (पूर्णाङ्क) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fcNormal;
    zeroSignificand();
    fs = normalize(rounding_mode, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } अञ्ञथा सचे ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handleOverflow(rounding_mode);
  } अञ्ञथा {
    integerPart *decSignificand;
    unsigned पूर्णाङ्क part_गणनम्;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_गणनम् = static_cast<unsigned पूर्णाङ्क>(D.lastSigDigit - D.firstSigDigit) + 1;
    part_गणनम् = part_गणनम्_For_Bits(1 + 196 * part_गणनम् / 59);
    decSignificand = नूतन integerPart[part_गणनम् + 1];
    part_गणनम् = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    करोहि {
      integerPart dec_मानम्, val, multiplier;

      val = 0;
      multiplier = 1;

      करोहि {
        सचे (*p == '.') {
          p++;
          सचे (p == str.end()) {
            भङ्ग;
          }
        }
        dec_मानम् = dec_Digit_मानम्(*p++);
        सचे (dec_मानम् >= 10U) {
          delete[] decSignificand;
          पटिनिवत्त createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_मानम्;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } याव (p <= D.lastSigDigit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_गणनम्, part_गणनम् + 1, मिच्छा);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      सचे (decSignificand[part_गणनम्])
        part_गणनम्++;
    } याव (p <= D.lastSigDigit);

    category = fcNormal;
    fs = roundSignificandWithExponent(decSignificand, part_गणनम्,
                                      D.exponent, rounding_mode);

    delete [] decSignificand;
  }

  पटिनिवत्त fs;
}

तार्किक IEEEFloat::convertFromStringSpecials(StringRef str) {
  ध्रुव size_t MIN_NAME_SIZE = 3;

  सचे (str.size() < MIN_NAME_SIZE)
    पटिनिवत्त मिच्छा;

  सचे (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(मिच्छा);
    पटिनिवत्त सच्च;
  }

  तार्किक IsNegative = str.consume_front("-");
  सचे (IsNegative) {
    सचे (str.size() < MIN_NAME_SIZE)
      पटिनिवत्त मिच्छा;

    सचे (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(सच्च);
      पटिनिवत्त सच्च;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  तार्किक IsSignaling = str.consume_front_insensitive("s");
  सचे (IsSignaling) {
    सचे (str.size() < MIN_NAME_SIZE)
      पटिनिवत्त मिच्छा;
  }

  सचे (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    सचे (str.empty()) {
      makeNaN(IsSignaling, IsNegative);
      पटिनिवत्त सच्च;
    }

    // Allow the payload to be inside parentheses.
    सचे (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      सचे (str.size() <= 2 || str.back() != ')')
        पटिनिवत्त मिच्छा;

      str = str.slice(1, str.size() - 1);
    }

    // Determine the payload number's radix.
    unsigned Radix = 10;
    सचे (str[0] == '0') {
      सचे (str.size() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } अञ्ञथा {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    सचे (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      पटिनिवत्त सच्च;
    }
  }

  पटिनिवत्त मिच्छा;
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromString(StringRef str, roundingMode rounding_mode) {
  सचे (str.empty())
    पटिनिवत्त createError("Invalid string length");

  // Handle special cases.
  सचे (convertFromStringSpecials(str))
    पटिनिवत्त opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  sign = *p == '-' ? 1 : 0;
  सचे (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  सचे (*p == '-' || *p == '+') {
    p++;
    slen--;
    सचे (!slen)
      पटिनिवत्त createError("String has no digits");
  }

  सचे (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    सचे (slen == 2)
      पटिनिवत्त createError("Invalid string");
    पटिनिवत्त convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_mode);
  }

  पटिनिवत्त convertFromDecimalString(StringRef(p, slen), rounding_mode);
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
unsigned पूर्णाङ्क IEEEFloat::convertToHexString(वर्ण *dst, unsigned पूर्णाङ्क hexDigits,
                                           तार्किक upperCase,
                                           roundingMode rounding_mode) ध्रुव {
  वर्ण *p = dst;
  सचे (sign)
    *dst++ = '-';

  विकल्प (category) {
  पक्ष fcInfinity:
    memcpy (dst, upperCase ? infinityU: infinityL, परिमाण infinityU - 1);
    dst += परिमाण infinityL - 1;
    भङ्ग;

  पक्ष fcNaN:
    memcpy (dst, upperCase ? NaNU: NaNL, परिमाण NaNU - 1);
    dst += परिमाण NaNU - 1;
    भङ्ग;

  पक्ष fcZero:
    *dst++ = '0';
    *dst++ = upperCase ? 'X': 'x';
    *dst++ = '0';
    सचे (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = upperCase ? 'P': 'p';
    *dst++ = '0';
    भङ्ग;

  पक्ष fcNormal:
    dst = convertNormalToHexString (dst, hexDigits, upperCase, rounding_mode);
    भङ्ग;
  }

  *dst = 0;

  पटिनिवत्त static_cast<unsigned पूर्णाङ्क>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
वर्ण *IEEEFloat::convertNormalToHexString(वर्ण *dst, unsigned पूर्णाङ्क hexDigits,
                                          तार्किक upperCase,
                                          roundingMode rounding_mode) ध्रुव {
  *dst++ = '0';
  *dst++ = upperCase ? 'X': 'x';

  तार्किक roundUp = मिच्छा;
  ध्रुव वर्ण *hexDigitChars = upperCase ? hexDigitsUpper : hexDigitsLower;

  ध्रुव integerPart *significand = significandParts();
  unsigned parts_गणनम् = part_गणनम्();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  unsigned मानम्_Bits = semantics->precision + 3;
  unsigned shift = integerPartWidth - मानम्_Bits % integerPartWidth;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  unsigned outputDigits = (मानम्_Bits - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  सचे (hexDigits) {
    सचे (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      unsigned पूर्णाङ्क bits;
      lostFraction fraction;

      bits = मानम्_Bits - hexDigits * 4;
      fraction = lostFractionThroughTruncation (significand, parts_गणनम्, bits);
      roundUp = roundAwayFromZero(rounding_mode, fraction, bits);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  वर्ण *p = ++dst;

  unsigned गणनम् = (मानम्_Bits + integerPartWidth - 1) / integerPartWidth;

  याव (outputDigits && गणनम्) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    सचे (--गणनम् == parts_गणनम्)
      part = 0;  /* An imaginary higher zero part.  */
    अञ्ञथा
      part = significand[गणनम्] << shift;

    सचे (गणनम् && shift)
      part |= significand[गणनम् - 1] >> (integerPartWidth - shift);

    /* Convert as much of "part" to hexdigits as we can.  */
    unsigned पूर्णाङ्क curDigits = integerPartWidth / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  सचे (roundUp) {
    वर्ण *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    करोहि {
      q--;
      *q = hexDigitChars[hex_Digit_मानम् (*q) + 1];
    } याव (*q == '0');
    प्रतिज्ञा(q >= p);
  } अञ्ञथा {
    /* Add trailing zeroes.  */
    memset (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  सचे (dst -1 == p)
    dst--;
  अञ्ञथा
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = upperCase ? 'P': 'p';

  पटिनिवत्त writeSignedDecimal (dst, exponent);
}

hash_code hash_मानम्(ध्रुव IEEEFloat &Arg) {
  सचे (!Arg.isFiniteNonZero())
    पटिनिवत्त hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  पटिनिवत्त hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.part_गणनम्()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convertF80LongDoubleAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 2);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semX87DoubleExtended>();
}

APInt IEEEFloat::convertPPCDoubleDoubleLegacyAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(semantics ==
         (ध्रुव llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy);
  प्रतिज्ञा(part_गणनम्()==2);

  uint64_t शब्दाः[2];
  तार्किक losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloatBase::semIEEEdouble.minExponent;
  IEEEFloat extended(*एतत्);
  [[maybe_unused]] opStatus fs =
      extended.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  प्रतिज्ञा(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
  प्रतिज्ञा(fs == opOK || fs == opInexact);
  शब्दाः[0] = *u.convertDoubleAPFloatToAPInt().getRawData();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  सचे (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
    प्रतिज्ञा(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.subtract(u, rmNearestTiesToEven);
    fs = v.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
    प्रतिज्ञा(fs == opOK && !losesInfo);
    शब्दाः[1] = *v.convertDoubleAPFloatToAPInt().getRawData();
  } अञ्ञथा {
    शब्दाः[1] = 0;
  }

  पटिनिवत्त APInt(128, शब्दाः);
}

template <ध्रुव fltSemantics &S>
APInt IEEEFloat::convertIEEEFloatToAPInt() ध्रुव {
  प्रतिज्ञा(semantics == &S);
  constexpr unsigned पूर्णाङ्क trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr पूर्णाङ्क integer_bit_part = (S.precision - 1) / integerPartWidth;
  constexpr integerPart integer_bit = integerPart{1}
                                      << ((S.precision - 1) % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned पूर्णाङ्क exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr तार्किक is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr पूर्णाङ्क bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_गणनम्_For_Bits(trailing_significand_bits)>
      mysignificand;

  सचे (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
    सचे (myexponent == 1 &&
        !(significandParts()[integer_bit_part] & integer_bit))
      myexponent = 0; // denormal
  } अञ्ञथा सचे (category == fcZero) {
    सचे (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } अञ्ञथा सचे (category == fcInfinity) {
    सचे (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    सचे constexpr (S.hasExplicitIntegerBit) {
      mysignificand[0] = integerPart{1} << (trailing_significand_bits - 1);
    }
  } अञ्ञथा {
    प्रतिज्ञा(category == fcNaN && "Unknown category!");
    सचे (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.sizeInBits + 63) / 64> शब्दाः;
  auto शब्दाः_iter =
      std::copy_n(mysignificand.begin(), mysignificand.size(), शब्दाः.begin());
  सचे constexpr (!S.hasExplicitIntegerBit) {
    सचे constexpr (significand_mask != 0 || trailing_significand_bits == 0) {
      // Clear the integer bit.
      शब्दाः[mysignificand.size() - 1] &= significand_mask;
    }
  }
  std::fill(शब्दाः_iter, शब्दाः.end(), uint64_t{0});
  constexpr size_t last_word = शब्दाः.size() - 1;
  सचे constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.sizeInBits - 1) % 64);
    शब्दाः[last_word] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_mask)
                              << (trailing_significand_bits % 64);
  शब्दाः[last_word] |= shifted_exponent;
  सचे constexpr (last_word == 0) {
    पटिनिवत्त APInt(S.sizeInBits, शब्दाः[0]);
  }
  पटिनिवत्त APInt(S.sizeInBits, शब्दाः);
}

APInt IEEEFloat::convertQuadrupleAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 2);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semIEEEquad>();
}

APInt IEEEFloat::convertDoubleAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्()==1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semIEEEdouble>();
}

APInt IEEEFloat::convertFloatAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्()==1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semIEEEsingle>();
}

APInt IEEEFloat::convertBFloatAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semBFloat>();
}

APInt IEEEFloat::convertHalfAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्()==1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::APFloatBase::semIEEEhalf>();
}

APInt IEEEFloat::convertFloat8E5M2APFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2>();
}

APInt IEEEFloat::convertFloat8E5M2FNUZAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3APFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3>();
}

APInt IEEEFloat::convertFloat8E4M3FNAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FN>();
}

APInt IEEEFloat::convertFloat8E4M3FNUZAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3B11FNUZAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convertFloat8E3M4APFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semFloat8E3M4>();
}

APInt IEEEFloat::convertFloatTF32APFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semFloatTF32>();
}

APInt IEEEFloat::convertFloat8E8M0FNUAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convertFloat8E5M3FNUAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convertFloat6E3M2FNAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semFloat6E3M2FN>();
}

APInt IEEEFloat::convertFloat6E2M3FNAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semFloat6E2M3FN>();
}

APInt IEEEFloat::convertFloat4E2M1FNAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  पटिनिवत्त convertIEEEFloatToAPInt<APFloatBase::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcastToAPInt() ध्रुव {
  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semIEEEhalf)
    पटिनिवत्त convertHalfAPFloatToAPInt();

  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semBFloat)
    पटिनिवत्त convertBFloatAPFloatToAPInt();

  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semIEEEsingle)
    पटिनिवत्त convertFloatAPFloatToAPInt();

  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semIEEEdouble)
    पटिनिवत्त convertDoubleAPFloatToAPInt();

  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semIEEEquad)
    पटिनिवत्त convertQuadrupleAPFloatToAPInt();

  सचे (semantics ==
      (ध्रुव llvm::fltSemantics *)&APFloatBase::semX87DoubleExtended)
    पटिनिवत्त convertF80LongDoubleAPFloatToAPInt();

  सचे (semantics ==
      (ध्रुव llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy)
    पटिनिवत्त convertPPCDoubleDoubleLegacyAPFloatToAPInt();

  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E5M2)
    पटिनिवत्त convertFloat8E5M2APFloatToAPInt();

  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E5M2FNUZ)
    पटिनिवत्त convertFloat8E5M2FNUZAPFloatToAPInt();

  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E4M3)
    पटिनिवत्त convertFloat8E4M3APFloatToAPInt();

  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FN)
    पटिनिवत्त convertFloat8E4M3FNAPFloatToAPInt();

  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FNUZ)
    पटिनिवत्त convertFloat8E4M3FNUZAPFloatToAPInt();

  सचे (semantics ==
      (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E4M3B11FNUZ)
    पटिनिवत्त convertFloat8E4M3B11FNUZAPFloatToAPInt();

  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E3M4)
    पटिनिवत्त convertFloat8E3M4APFloatToAPInt();

  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloatTF32)
    पटिनिवत्त convertFloatTF32APFloatToAPInt();

  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E8M0FNU)
    पटिनिवत्त convertFloat8E8M0FNUAPFloatToAPInt();

  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E5M3FNU)
    पटिनिवत्त convertFloat8E5M3FNUAPFloatToAPInt();

  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat6E3M2FN)
    पटिनिवत्त convertFloat6E3M2FNAPFloatToAPInt();

  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat6E2M3FN)
    पटिनिवत्त convertFloat6E2M3FNAPFloatToAPInt();

  सचे (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat4E2M1FN)
    पटिनिवत्त convertFloat4E2M1FNAPFloatToAPInt();

  llvm_unreachable("unknown format!");
}

भिन्न IEEEFloat::convertToFloat() ध्रुव {
  प्रतिज्ञा(semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcastToAPInt();
  पटिनिवत्त api.bitsToFloat();
}

द्विगुण IEEEFloat::convertToDouble() ध्रुव {
  प्रतिज्ञा(semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcastToAPInt();
  पटिनिवत्त api.bitsToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 IEEEFloat::convertToQuad() ध्रुव {
  प्रतिज्ञा(semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcastToAPInt();
  पटिनिवत्त api.bitsToQuad();
}
#endif

शून्य IEEEFloat::initFromF80LongDoubleAPInt(ध्रुव APInt &api) {
  पटिनिवत्त initFromIEEEAPInt<APFloatBase::semX87DoubleExtended>(api);
}

शून्य IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(ध्रुव APInt &api) {
  uint64_t i1 = api.getRawData()[0];
  uint64_t i2 = api.getRawData()[1];
  तार्किक losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_unused]] opStatus fs = convert(APFloatBase::semPPCDoubleDoubleLegacy,
                                         rmNearestTiesToEven, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  प्रतिज्ञा((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  सचे (isFiniteNonZero()) {
    IEEEFloat v(APFloatBase::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    सचे (v.isFinite()) {
      fs = v.convert(APFloatBase::semPPCDoubleDoubleLegacy, rmNearestTiesToEven,
                     &losesInfo);
      प्रतिज्ञा(fs == opOK && !losesInfo);

      add(v, rmNearestTiesToEven);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
शून्य IEEEFloat::initFromFloat8E8M0FNUAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E8M0FNU>(api);
}

शून्य IEEEFloat::initFromFloat8E5M3FNUAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M3FNU>(api);
}

template <ध्रुव fltSemantics &S>
शून्य IEEEFloat::initFromIEEEAPInt(ध्रुव APInt &api) {
  प्रतिज्ञा(api.getBitWidth() == S.sizeInBits);

  constexpr unsigned पूर्णाङ्क trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr integerPart integer_bit =
      integerPart{1} << (trailing_significand_bits % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned पूर्णाङ्क exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr unsigned पूर्णाङ्क stored_significand_parts =
      part_गणनम्_For_Bits(trailing_significand_bits + 1);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr तार्किक is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr पूर्णाङ्क bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr तार्किक has_significand = trailing_significand_bits > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  सचे constexpr (has_significand) {
    std::copy_n(api.getRawData(), mysignificand.size(), mysignificand.begin());
    सचे constexpr (significand_mask != 0 || S.precision >= integerPartWidth) {
      mysignificand[mysignificand.size() - 1] &= significand_mask;
    }
  } अञ्ञथा {
    std::fill_n(mysignificand.begin(), mysignificand.size(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t last_word = api.getRawData()[api.get_अङ्कः_शब्दाः() - 1];
  uint64_t myexponent =
      (last_word >> (trailing_significand_bits % 64)) & exponent_mask;

  initialize(&S);
  प्रतिज्ञा(part_गणनम्() == mysignificand.size());

  sign = S.hasSignedRepr
             ? static_cast<unsigned पूर्णाङ्क>(last_word >> ((S.sizeInBits - 1) % 64))
             : 0;

  तार्किक all_zero_significand =
      has_significand && llvm::all_of(mysignificand, equal_to(0));

  तार्किक is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  सचे constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    तार्किक is_inf = मिच्छा;

    सचे constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;
      ध्रुव integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_mask_no_int_bit) == 0;
    } अञ्ञथा {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    सचे (is_inf) {
      makeInf(sign);
      पटिनिवत्त;
    }
  }

  तार्किक is_nan = मिच्छा;

  सचे constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    सचे constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      ध्रुव integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;

      सचे (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_mask_no_int_bit) != 0) {
        // regular NaN and pseudoNaN
        is_nan = सच्च;
      } अञ्ञथा सचे (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_mask_no_int_bit) == 0) {
        // pseudoinfinity
        is_nan = सच्च;
      } अञ्ञथा सचे (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = सच्च;
      }
    } अञ्ञथा {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } अञ्ञथा सचे constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    तार्किक all_ones_significand =
        std::all_of(mysignificand.begin(), mysignificand.end() - 1,
                    [](integerPart bits) { पटिनिवत्त bits == ~integerPart{0}; }) &&
        (!significand_mask ||
         mysignificand[mysignificand.size() - 1] == significand_mask);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } अञ्ञथा सचे constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  सचे (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.size(),
                significandParts());
    पटिनिवत्त;
  }

  सचे (is_zero) {
    makeZero(sign);
    पटिनिवत्त;
  }

  category = fcNormal;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.size(), significandParts());
  सचे (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  अञ्ञथा {
    सचे constexpr (!S.hasExplicitIntegerBit) {
      significandParts()[mysignificand.size() - 1] |= integer_bit;
    }
  }
}

शून्य IEEEFloat::initFromQuadrupleAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEquad>(api);
}

शून्य IEEEFloat::initFromDoubleAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEdouble>(api);
}

शून्य IEEEFloat::initFromFloatAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEsingle>(api);
}

शून्य IEEEFloat::initFromBFloatAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semBFloat>(api);
}

शून्य IEEEFloat::initFromHalfAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEhalf>(api);
}

शून्य IEEEFloat::initFromFloat8E5M2APInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2>(api);
}

शून्य IEEEFloat::initFromFloat8E5M2FNUZAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2FNUZ>(api);
}

शून्य IEEEFloat::initFromFloat8E4M3APInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3>(api);
}

शून्य IEEEFloat::initFromFloat8E4M3FNAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FN>(api);
}

शून्य IEEEFloat::initFromFloat8E4M3FNUZAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FNUZ>(api);
}

शून्य IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3B11FNUZ>(api);
}

शून्य IEEEFloat::initFromFloat8E3M4APInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E3M4>(api);
}

शून्य IEEEFloat::initFromFloatTF32APInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloatTF32>(api);
}

शून्य IEEEFloat::initFromFloat6E3M2FNAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E3M2FN>(api);
}

शून्य IEEEFloat::initFromFloat6E2M3FNAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E2M3FN>(api);
}

शून्य IEEEFloat::initFromFloat4E2M1FNAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
शून्य IEEEFloat::initFromAPInt(ध्रुव fltSemantics *Sem, ध्रुव APInt &api) {
  प्रतिज्ञा(api.getBitWidth() == Sem->sizeInBits);
  सचे (Sem == &APFloatBase::semIEEEhalf)
    पटिनिवत्त initFromHalfAPInt(api);
  सचे (Sem == &APFloatBase::semBFloat)
    पटिनिवत्त initFromBFloatAPInt(api);
  सचे (Sem == &APFloatBase::semIEEEsingle)
    पटिनिवत्त initFromFloatAPInt(api);
  सचे (Sem == &APFloatBase::semIEEEdouble)
    पटिनिवत्त initFromDoubleAPInt(api);
  सचे (Sem == &APFloatBase::semIEEEquad)
    पटिनिवत्त initFromQuadrupleAPInt(api);
  सचे (Sem == &APFloatBase::semX87DoubleExtended)
    पटिनिवत्त initFromF80LongDoubleAPInt(api);
  सचे (Sem == &APFloatBase::semPPCDoubleDoubleLegacy)
    पटिनिवत्त initFromPPCDoubleDoubleLegacyAPInt(api);
  सचे (Sem == &APFloatBase::semFloat8E5M2)
    पटिनिवत्त initFromFloat8E5M2APInt(api);
  सचे (Sem == &APFloatBase::semFloat8E5M2FNUZ)
    पटिनिवत्त initFromFloat8E5M2FNUZAPInt(api);
  सचे (Sem == &APFloatBase::semFloat8E4M3)
    पटिनिवत्त initFromFloat8E4M3APInt(api);
  सचे (Sem == &APFloatBase::semFloat8E4M3FN)
    पटिनिवत्त initFromFloat8E4M3FNAPInt(api);
  सचे (Sem == &APFloatBase::semFloat8E4M3FNUZ)
    पटिनिवत्त initFromFloat8E4M3FNUZAPInt(api);
  सचे (Sem == &APFloatBase::semFloat8E4M3B11FNUZ)
    पटिनिवत्त initFromFloat8E4M3B11FNUZAPInt(api);
  सचे (Sem == &APFloatBase::semFloat8E3M4)
    पटिनिवत्त initFromFloat8E3M4APInt(api);
  सचे (Sem == &APFloatBase::semFloatTF32)
    पटिनिवत्त initFromFloatTF32APInt(api);
  सचे (Sem == &APFloatBase::semFloat8E8M0FNU)
    पटिनिवत्त initFromFloat8E8M0FNUAPInt(api);
  सचे (Sem == &APFloatBase::semFloat8E5M3FNU)
    पटिनिवत्त initFromFloat8E5M3FNUAPInt(api);
  सचे (Sem == &APFloatBase::semFloat6E3M2FN)
    पटिनिवत्त initFromFloat6E3M2FNAPInt(api);
  सचे (Sem == &APFloatBase::semFloat6E2M3FN)
    पटिनिवत्त initFromFloat6E2M3FNAPInt(api);
  सचे (Sem == &APFloatBase::semFloat4E2M1FN)
    पटिनिवत्त initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
शून्य IEEEFloat::makeLargest(तार्किक Negative) {
  सचे (Negative && !semantics->hasSignedRepr)
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
  unsigned Part_गणनम् = part_गणनम्();
  memset(significand, 0xFF, परिमाण(integerPart)*(Part_गणनम् - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  ध्रुव unsigned अङ्कः_Unused_उच्चः_Bits =
    Part_गणनम्*integerPartWidth - semantics->precision;
  significand[Part_गणनम् - 1] = (अङ्कः_Unused_उच्चः_Bits < integerPartWidth)
                                   ? (~integerPart(0) >> अङ्कः_Unused_उच्चः_Bits)
                                   : 0;
  सचे (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
शून्य IEEEFloat::makeSmallest(तार्किक Negative) {
  सचे (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fcNormal;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tcSet(significandParts(), 1, part_गणनम्());
}

शून्य IEEEFloat::makeSmallestNormalized(तार्किक Negative) {
  सचे (Negative && !semantics->hasSignedRepr)
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

IEEEFloat::IEEEFloat(ध्रुव fltSemantics &Sem, ध्रुव APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(भिन्न f) {
  initFromAPInt(&APFloatBase::semIEEEsingle, APInt::floatToBits(f));
}

IEEEFloat::IEEEFloat(द्विगुण d) {
  initFromAPInt(&APFloatBase::semIEEEdouble, APInt::doubleToBits(d));
}

नामाकाश {
  शून्य append(SmallVectorImpl<वर्ण> &Buffer, StringRef Str) {
    Buffer.append(Str.begin(), Str.end());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  शून्य AdjustToPrecision(APInt &significand,
                         पूर्णाङ्क &exp, unsigned FormatPrecision) {
    unsigned bits = significand.getActiveBits();

    // 196/59 is a very slight overestimate of lg_2(10).
    unsigned bitsRequired = (FormatPrecision * 196 + 58) / 59;

    सचे (bits <= bitsRequired) पटिनिवत्त;

    unsigned tensRemovable = (bits - bitsRequired) * 59 / 196;
    सचे (!tensRemovable) पटिनिवत्त;

    exp += tensRemovable;

    APInt भाजकः(significand.getBitWidth(), 1);
    APInt powten(significand.getBitWidth(), 10);
    याव (सच्च) {
      सचे (tensRemovable & 1)
        भाजकः *= powten;
      tensRemovable >>= 1;
      सचे (!tensRemovable) भङ्ग;
      powten *= powten;
    }

    significand = significand.udiv(भाजकः);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.getActiveBits());
  }


  शून्य AdjustToPrecision(SmallVectorImpl<वर्ण> &buffer,
                         पूर्णाङ्क &exp, unsigned FormatPrecision) {
    unsigned N = buffer.size();
    सचे (N <= FormatPrecision) पटिनिवत्त;

    // The most significant figures are the last ones in the buffer.
    unsigned FirstSignificant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    सचे (buffer[FirstSignificant - 1] < '5') {
      याव (FirstSignificant < N && buffer[FirstSignificant] == '0')
        FirstSignificant++;

      exp += FirstSignificant;
      buffer.erase(&buffer[0], &buffer[FirstSignificant]);
      पटिनिवत्त;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    अत्थाय (unsigned I = FirstSignificant; I != N; ++I) {
      सचे (buffer[I] == '9') {
        FirstSignificant++;
      } अञ्ञथा {
        buffer[I]++;
        भङ्ग;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    सचे (FirstSignificant == N) {
      exp += FirstSignificant;
      buffer.clear();
      buffer.प्रक्षिप_back('1');
      पटिनिवत्त;
    }

    exp += FirstSignificant;
    buffer.erase(&buffer[0], &buffer[FirstSignificant]);
  }

  शून्य toStringImpl(SmallVectorImpl<वर्ण> &Str, ध्रुव तार्किक isNeg, पूर्णाङ्क exp,
                    APInt significand, unsigned FormatPrecision,
                    unsigned FormatMaxPadding, तार्किक TruncateZero) {
    ध्रुव पूर्णाङ्क semanticsPrecision = significand.getBitWidth();

    सचे (isNeg)
      Str.प्रक्षिप_back('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    सचे (!FormatPrecision) {
      // We use enough digits so the number can be round-tripped back to an
      // APFloat. The formula comes from "How to Print Floating-Point Numbers
      // Accurately" by Steele and White.
      // FIXME: Using a formula based purely on the precision is conservative;
      // we can print fewer digits depending on the actual value being printed.

      // FormatPrecision = 2 + floor(significandBits / lg_2(10))
      FormatPrecision = 2 + semanticsPrecision * 59 / 196;
    }

    // Ignore trailing binary zeros.
    पूर्णाङ्क trailingZeros = significand.countr_zero();
    exp += trailingZeros;
    significand.lshrInPlace(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    सचे (exp == 0) {
      // Nothing to do.
    } अञ्ञथा सचे (exp > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + exp);
      significand <<= exp;
      exp = 0;
    } अञ्ञथा { /* exp < 0 */
      पूर्णाङ्क texp = -exp;

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
      याव (सच्च) {
        सचे (texp & 1)
          significand *= five_to_the_i;

        texp >>= 1;
        सचे (!texp)
          भङ्ग;
        five_to_the_i *= five_to_the_i;
      }
    }

    AdjustToPrecision(significand, exp, FormatPrecision);

    SmallVector<वर्ण, 256> buffer;

    // Fill the buffer.
    unsigned precision = significand.getBitWidth();
    सचे (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    तार्किक inTrail = सच्च;
    याव (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      unsigned d = digit.get_ZExt_मानम्();

      // Drop trailing zeros.
      सचे (inTrail && !d)
        exp++;
      अञ्ञथा {
        buffer.प्रक्षिप_back((वर्ण) ('0' + d));
        inTrail = मिच्छा;
      }
    }

    प्रतिज्ञा(!buffer.empty() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    AdjustToPrecision(buffer, exp, FormatPrecision);

    unsigned NDigits = buffer.size();

    // Check whether we should use scientific notation.
    तार्किक FormatScientific;
    सचे (!FormatMaxPadding) {
      FormatScientific = सच्च;
    } अञ्ञथा {
      सचे (exp >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((unsigned) exp > FormatMaxPadding ||
                            NDigits + (unsigned) exp > FormatPrecision);
      } अञ्ञथा {
        // Power of the most significant digit.
        पूर्णाङ्क MSD = exp + (पूर्णाङ्क) (NDigits - 1);
        सचे (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = मिच्छा;
        } अञ्ञथा {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((unsigned) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    सचे (FormatScientific) {
      exp += (NDigits - 1);

      Str.प्रक्षिप_back(buffer[NDigits-1]);
      Str.प्रक्षिप_back('.');
      सचे (NDigits == 1 && TruncateZero)
        Str.प्रक्षिप_back('0');
      अञ्ञथा
        अत्थाय (unsigned I = 1; I != NDigits; ++I)
          Str.प्रक्षिप_back(buffer[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      सचे (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.प्रक्षिप_back(TruncateZero ? 'E' : 'e');

      Str.प्रक्षिप_back(exp >= 0 ? '+' : '-');
      सचे (exp < 0)
        exp = -exp;
      SmallVector<वर्ण, 6> expbuf;
      करोहि {
        expbuf.प्रक्षिप_back((वर्ण) ('0' + (exp % 10)));
        exp /= 10;
      } याव (exp);
      // Exponent always at least two digits if we do not truncate zeros.
      सचे (!TruncateZero && expbuf.size() < 2)
        expbuf.प्रक्षिप_back('0');
      अत्थाय (unsigned I = 0, E = expbuf.size(); I != E; ++I)
        Str.प्रक्षिप_back(expbuf[E-1-I]);
      पटिनिवत्त;
    }

    // Non-scientific, positive exponents.
    सचे (exp >= 0) {
      अत्थाय (unsigned I = 0; I != NDigits; ++I)
        Str.प्रक्षिप_back(buffer[NDigits-1-I]);
      अत्थाय (unsigned I = 0; I != (unsigned) exp; ++I)
        Str.प्रक्षिप_back('0');
      पटिनिवत्त;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    पूर्णाङ्क NWholeDigits = exp + (पूर्णाङ्क) NDigits;

    unsigned I = 0;
    सचे (NWholeDigits > 0) {
      अत्थाय (; I != (unsigned) NWholeDigits; ++I)
        Str.प्रक्षिप_back(buffer[NDigits-I-1]);
      Str.प्रक्षिप_back('.');
    } अञ्ञथा {
      unsigned NZeros = 1 + (unsigned) -NWholeDigits;

      Str.प्रक्षिप_back('0');
      Str.प्रक्षिप_back('.');
      अत्थाय (unsigned Z = 1; Z != NZeros; ++Z)
        Str.प्रक्षिप_back('0');
    }

    अत्थाय (; I != NDigits; ++I)
      Str.प्रक्षिप_back(buffer[NDigits-I-1]);

  }
} // namespace

शून्य IEEEFloat::toString(SmallVectorImpl<वर्ण> &Str, unsigned FormatPrecision,
                         unsigned FormatMaxPadding, तार्किक TruncateZero) ध्रुव {
  विकल्प (category) {
  पक्ष fcInfinity:
    सचे (isNegative())
      पटिनिवत्त append(Str, "-Inf");
    अञ्ञथा
      पटिनिवत्त append(Str, "+Inf");

  पक्ष fcNaN: पटिनिवत्त append(Str, "NaN");

  पक्ष fcZero:
    सचे (isNegative())
      Str.प्रक्षिप_back('-');

    सचे (!FormatMaxPadding) {
      सचे (TruncateZero)
        append(Str, "0.0E+0");
      अञ्ञथा {
        append(Str, "0.0");
        सचे (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } अञ्ञथा {
      Str.प्रक्षिप_back('0');
    }
    पटिनिवत्त;

  पक्ष fcNormal:
    भङ्ग;
  }

  // Decompose the number into an APInt and an exponent.
  पूर्णाङ्क exp = exponent - ((पूर्णाङ्क) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_गणनम्_For_Bits(semantics->precision)));

  toStringImpl(Str, isNegative(), exp, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

पूर्णाङ्क IEEEFloat::getExactLog2Abs() ध्रुव {
  सचे (!isFinite() || isZero())
    पटिनिवत्त INT_MIN;

  ध्रुव integerPart *Parts = significandParts();
  ध्रुव पूर्णाङ्क Part_गणनम् = part_गणनम्_For_Bits(semantics->precision);

  पूर्णाङ्क निष्कासय_गणनम् = 0;
  अत्थाय (पूर्णाङ्क i = 0; i < Part_गणनम्; ++i) {
    निष्कासय_गणनम् += llvm::popcount(Parts[i]);
    सचे (निष्कासय_गणनम् > 1)
      पटिनिवत्त INT_MIN;
  }

  सचे (exponent != semantics->minExponent)
    पटिनिवत्त exponent;

  पूर्णाङ्क CountrParts = 0;
  अत्थाय (पूर्णाङ्क i = 0; i < Part_गणनम्;
       ++i, CountrParts += APInt::APINT_BITS_PER_WORD) {
    सचे (Parts[i] != 0) {
      पटिनिवत्त exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

तार्किक IEEEFloat::isSignaling() ध्रुव {
  सचे (!isNaN())
    पटिनिवत्त मिच्छा;
  सचे (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    पटिनिवत्त मिच्छा;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  पटिनिवत्त !APInt::tcExtractBit(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::opStatus IEEEFloat::next(तार्किक nextDown) {
  // If we are performing nextDown, swap sign so we have -x.
  सचे (nextDown)
    changeSign();

  // Compute nextUp(x)
  opStatus result = opOK;

  // Handle each float category separately.
  विकल्प (category) {
  पक्ष fcInfinity:
    // nextUp(+inf) = +inf
    सचे (!isNegative())
      भङ्ग;
    // nextUp(-inf) = -getLargest()
    makeLargest(सच्च);
    भङ्ग;
  पक्ष fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    सचे (isSignaling()) {
      result = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(मिच्छा, isNegative(), nullptr);
    }
    भङ्ग;
  पक्ष fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(मिच्छा);
    भङ्ग;
  पक्ष fcNormal:
    // nextUp(-getSmallest()) = -0
    सचे (isSmallest() && isNegative()) {
      APInt::tcSet(significandParts(), 0, part_गणनम्());
      category = fcZero;
      exponent = 0;
      सचे (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = मिच्छा;
      सचे (!semantics->hasZero)
        makeSmallestNormalized(मिच्छा);
      भङ्ग;
    }

    सचे (isLargest() && !isNegative()) {
      सचे (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        भङ्ग;
      } अञ्ञथा सचे (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        भङ्ग;
      } अञ्ञथा {
        // nextUp(getLargest()) == INFINITY
        APInt::tcSet(significandParts(), 0, part_गणनम्());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        भङ्ग;
      }
    }

    // nextUp(normal) == normal + inc.
    सचे (isNegative()) {
      // If we are negative, we need to decrement the significand.

      // We only cross a binade boundary that requires adjusting the exponent
      // if:
      //   1. exponent != semantics->minExponent. This implies we are not in the
      //   smallest binade or are dealing with denormals.
      //   2. Our significand excluding the integral bit is all zeros.
      तार्किक WillCrossBinadeBoundary =
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
      APInt::tcDecrement(Parts, part_गणनम्());

      सचे (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tcSetBit(Parts, semantics->precision - 1);
        exponent--;
      }
    } अञ्ञथा {
      // If we are positive, we need to increment the significand.

      // We only cross a binade boundary that requires adjusting the exponent if
      // the input is not a denormal and all of said input's significand bits
      // are set. If all of said conditions are true: clear the significand, set
      // the integral bit to 1, and increment the exponent. If we have a
      // denormal always increment since moving denormals and the numbers in the
      // smallest normal binade have the same exponent in our representation.
      // If there are only exponents, any increment always crosses the
      // BinadeBoundary.
      तार्किक WillCrossBinadeBoundary = !APFloat::hasSignificand(*semantics) ||
                                     (!isDenormal() && isSignificandAllOnes());

      सचे (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tcSet(Parts, 0, part_गणनम्());
        APInt::tcSetBit(Parts, semantics->precision - 1);
        प्रतिज्ञा(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } अञ्ञथा {
        incrementSignificand();
      }
    }
    भङ्ग;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  सचे (nextDown)
    changeSign();

  पटिनिवत्त result;
}

APInt IEEEFloat::getNaNPayload() ध्रुव {
  प्रतिज्ञा(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  unsigned Bits = semantics->precision - 1;
  पटिनिवत्त APInt(Bits, ArrayRef(significandParts(), part_गणनम्_For_Bits(Bits)));
}

APFloatBase::ExponentType IEEEFloat::exponentNaN() ध्रुव {
  पटिनिवत्त ::exponentNaN(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentInf() ध्रुव {
  पटिनिवत्त ::exponentInf(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentZero() ध्रुव {
  पटिनिवत्त ::exponentZero(*semantics);
}

शून्य IEEEFloat::makeInf(तार्किक Negative) {
  सचे (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  सचे (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(मिच्छा, Negative);
    पटिनिवत्त;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tcSet(significandParts(), 0, part_गणनम्());
}

शून्य IEEEFloat::makeZero(तार्किक Negative) {
  सचे (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  सचे (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = मिच्छा;
  }
  exponent = exponentZero();
  APInt::tcSet(significandParts(), 0, part_गणनम्());
}

शून्य IEEEFloat::makeQuiet() {
  प्रतिज्ञा(isNaN());
  सचे (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tcSetBit(significandParts(), semantics->precision - 2);
}

पूर्णाङ्क ilogb(ध्रुव IEEEFloat &Arg) {
  सचे (Arg.isNaN())
    पटिनिवत्त APFloat::IEK_NaN;
  सचे (Arg.isZero())
    पटिनिवत्त APFloat::IEK_Zero;
  सचे (Arg.isInfinity())
    पटिनिवत्त APFloat::IEK_Inf;
  सचे (!Arg.isDenormal())
    पटिनिवत्त Arg.exponent;

  IEEEFloat Normalized(Arg);
  पूर्णाङ्क SignificandBits = Arg.getSemantics().precision - 1;

  Normalized.exponent += SignificandBits;
  Normalized.normalize(APFloat::rmNearestTiesToEven, lfExactlyZero);
  पटिनिवत्त Normalized.exponent - SignificandBits;
}

IEEEFloat scalbn(IEEEFloat X, पूर्णाङ्क Exp, roundingMode RoundingMode) {
  auto MaxExp = X.getSemantics().maxExponent;
  auto MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  पूर्णाङ्क SignificandBits = X.getSemantics().precision - 1;
  पूर्णाङ्क MaxIncrement = MaxExp - (MinExp - SignificandBits) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(RoundingMode, lfExactlyZero);
  सचे (X.isNaN())
    X.makeQuiet();
  पटिनिवत्त X;
}

IEEEFloat frexp(ध्रुव IEEEFloat &Val, पूर्णाङ्क &Exp, roundingMode RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  सचे (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    पटिनिवत्त Quiet;
  }

  सचे (Exp == APFloat::IEK_Inf)
    पटिनिवत्त Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  पटिनिवत्त scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(ध्रुव fltSemantics &S)
    : Semantics(&S),
      Floats(नूतन APFloat[2]{APFloat(APFloatBase::semIEEEdouble),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ध्रुव fltSemantics &S, uninitializedTag)
    : Semantics(&S), Floats(नूतन APFloat[2]{
                         APFloat(APFloatBase::semIEEEdouble, uninitialized),
                         APFloat(APFloatBase::semIEEEdouble, uninitialized)}) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ध्रुव fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(नूतन APFloat[2]{APFloat(APFloatBase::semIEEEdouble, I),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ध्रुव fltSemantics &S, ध्रुव APInt &I)
    : Semantics(&S),
      Floats(नूतन APFloat[2]{
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[0])),
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[1]))}) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ध्रुव fltSemantics &S, APFloat &&First,
                             APFloat &&Second)
    : Semantics(&S),
      Floats(नूतन APFloat[2]{std::move(First), std::move(Second)}) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
  प्रतिज्ञा(&Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  प्रतिज्ञा(&Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(ध्रुव DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? नूतन APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : nullptr) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloatBase::semBogus;
  RHS.Floats = nullptr;
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::operator=(ध्रुव DoubleAPFloat &RHS) {
  सचे (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } अञ्ञथा सचे (एतत् != &RHS) {
    एतत्->~DoubleAPFloat();
    नूतन (एतत्) DoubleAPFloat(RHS);
  }
  पटिनिवत्त *एतत्;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
स्थिर std::pair<APFloat, APFloat> fastTwoSum(APFloat X, APFloat Y) {
  सचे (!X.isFinite())
    पटिनिवत्त {X, APFloat::getZero(X.getSemantics(), /*Negative=*/मिच्छा)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  पटिनिवत्त {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::opStatus DoubleAPFloat::addImpl(ध्रुव APFloat &a, ध्रुव APFloat &aa,
                                         ध्रुव APFloat &c, ध्रुव APFloat &cc,
                                         roundingMode RM) {
  पूर्णाङ्क Status = opOK;
  APFloat z = a;
  Status |= z.add(c, RM);
  सचे (!z.isFinite()) {
    सचे (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ मिच्छा);
      पटिनिवत्त (opStatus)Status;
    }
    Status = opOK;
    auto AComparedToC = a.compare_Absolute_मानम्(c);
    z = cc;
    Status |= z.add(aa, RM);
    सचे (AComparedToC == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      Status |= z.add(c, RM);
      Status |= z.add(a, RM);
    } अञ्ञथा {
      // z = cc + aa + a + c;
      Status |= z.add(a, RM);
      Status |= z.add(c, RM);
    }
    सचे (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ मिच्छा);
      पटिनिवत्त (opStatus)Status;
    }
    Floats[0] = z;
    APFloat zz = aa;
    Status |= zz.add(cc, RM);
    सचे (AComparedToC == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(c, RM);
      Status |= Floats[1].add(zz, RM);
    } अञ्ञथा {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(a, RM);
      Status |= Floats[1].add(zz, RM);
    }
  } अञ्ञथा {
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
    सचे (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ मिच्छा);
      पटिनिवत्त opOK;
    }
    Floats[0] = z;
    Status |= Floats[0].add(zz, RM);
    सचे (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ मिच्छा);
      पटिनिवत्त (opStatus)Status;
    }
    Floats[1] = std::move(z);
    Status |= Floats[1].subtract(Floats[0], RM);
    Status |= Floats[1].add(zz, RM);
  }
  पटिनिवत्त (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::addWithSpecial(ध्रुव DoubleAPFloat &LHS,
                                                ध्रुव DoubleAPFloat &RHS,
                                                DoubleAPFloat &Out,
                                                roundingMode RM) {
  सचे (LHS.getCategory() == fcNaN) {
    Out = LHS;
    पटिनिवत्त opOK;
  }
  सचे (RHS.getCategory() == fcNaN) {
    Out = RHS;
    पटिनिवत्त opOK;
  }
  सचे (LHS.getCategory() == fcZero) {
    Out = RHS;
    पटिनिवत्त opOK;
  }
  सचे (RHS.getCategory() == fcZero) {
    Out = LHS;
    पटिनिवत्त opOK;
  }
  सचे (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    Out.makeNaN(मिच्छा, Out.isNegative(), nullptr);
    पटिनिवत्त opInvalidOp;
  }
  सचे (LHS.getCategory() == fcInfinity) {
    Out = LHS;
    पटिनिवत्त opOK;
  }
  सचे (RHS.getCategory() == fcInfinity) {
    Out = RHS;
    पटिनिवत्त opOK;
  }
  प्रतिज्ञा(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  प्रतिज्ञा(&A.getSemantics() == &APFloatBase::semIEEEdouble);
  प्रतिज्ञा(&AA.getSemantics() == &APFloatBase::semIEEEdouble);
  प्रतिज्ञा(&C.getSemantics() == &APFloatBase::semIEEEdouble);
  प्रतिज्ञा(&CC.getSemantics() == &APFloatBase::semIEEEdouble);
  प्रतिज्ञा(&Out.Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  प्रतिज्ञा(&Out.Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
  पटिनिवत्त Out.addImpl(A, AA, C, CC, RM);
}

APFloat::opStatus DoubleAPFloat::add(ध्रुव DoubleAPFloat &RHS,
                                     roundingMode RM) {
  पटिनिवत्त addWithSpecial(*एतत्, RHS, *एतत्, RM);
}

APFloat::opStatus DoubleAPFloat::subtract(ध्रुव DoubleAPFloat &RHS,
                                          roundingMode RM) {
  changeSign();
  auto Ret = add(RHS, RM);
  changeSign();
  पटिनिवत्त Ret;
}

APFloat::opStatus DoubleAPFloat::multiply(ध्रुव DoubleAPFloat &RHS,
                                          APFloat::roundingMode RM) {
  ध्रुव auto &LHS = *एतत्;
  auto &Out = *एतत्;
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
  सचे (LHS.getCategory() == fcNaN) {
    Out = LHS;
    पटिनिवत्त opOK;
  }
  सचे (RHS.getCategory() == fcNaN) {
    Out = RHS;
    पटिनिवत्त opOK;
  }
  सचे ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    Out.makeNaN(मिच्छा, मिच्छा, nullptr);
    पटिनिवत्त opOK;
  }
  सचे (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    Out = LHS;
    पटिनिवत्त opOK;
  }
  सचे (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    Out = RHS;
    पटिनिवत्त opOK;
  }
  प्रतिज्ञा(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal &&
         "Special cases not handled exhaustively");

  पूर्णाङ्क Status = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  Status |= T.multiply(C, RM);
  सचे (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ मिच्छा);
    पटिनिवत्त (opStatus)Status;
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
  सचे (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ मिच्छा);
  } अञ्ञथा {
    // Floats[1] = (t - u) + tau
    Status |= T.subtract(U, RM);
    Status |= T.add(Tau, RM);
    Floats[1] = std::move(T);
  }
  पटिनिवत्त (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::divide(ध्रुव DoubleAPFloat &RHS,
                                        APFloat::roundingMode RM) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.divide(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()), RM);
  *एतत् = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  पटिनिवत्त Ret;
}

APFloat::opStatus DoubleAPFloat::remainder(ध्रुव DoubleAPFloat &RHS) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.remainder(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *एतत् = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  पटिनिवत्त Ret;
}

APFloat::opStatus DoubleAPFloat::mod(ध्रुव DoubleAPFloat &RHS) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.mod(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *एतत् = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  पटिनिवत्त Ret;
}

APFloat::opStatus
DoubleAPFloat::fusedMultiplyAdd(ध्रुव DoubleAPFloat &Multiplicand,
                                ध्रुव DoubleAPFloat &Addend,
                                APFloat::roundingMode RM) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.fusedMultiplyAdd(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcastToAPInt()),
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, Addend.bitcastToAPInt()),
      RM);
  *एतत् = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  पटिनिवत्त Ret;
}

APFloat::opStatus DoubleAPFloat::roundToIntegral(APFloat::roundingMode RM) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ध्रुव APFloat &Hi = getFirst();
  ध्रुव APFloat &Lo = getSecond();

  APFloat RoundedHi = Hi;
  ध्रुव opStatus HiStatus = RoundedHi.roundToIntegral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  सचे (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/मिच्छा);
    पटिनिवत्त HiStatus;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  auto RoundToNearestHelper = [](APFloat ToRound, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    ध्रुव APFloat RoundingError = Rounded - ToRound;
    सचे (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        abs(RoundingError).is_Exactly_मानम्(0.5))
      Rounded.add(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rmNearestTiesToEven);
    पटिनिवत्त Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  सचे (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    सचे (RM == rmNearestTiesToAway || RM == rmNearestTiesToEven)
      RoundedHi = RoundToNearestHelper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/मिच्छा);
    पटिनिवत्त HiStatus;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  RoundingMode LoRoundingMode;
  सचे (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    LoRoundingMode = isNegative() ? rmTowardPositive : rmTowardNegative;
  अञ्ञथा
    LoRoundingMode = RM;

  APFloat RoundedLo = Lo;
  ध्रुव opStatus LoStatus = RoundedLo.roundToIntegral(LoRoundingMode);
  सचे (LoRoundingMode == rmNearestTiesToAway)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = RoundToNearestHelper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fastTwoSum(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  पटिनिवत्त LoStatus;
}

शून्य DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmpResult
DoubleAPFloat::compare_Absolute_मानम्(ध्रुव DoubleAPFloat &RHS) ध्रुव {
  // Compare absolute values of the high parts.
  ध्रुव cmpResult HiPartCmp = Floats[0].compare_Absolute_मानम्(RHS.Floats[0]);
  सचे (HiPartCmp != cmpEqual)
    पटिनिवत्त HiPartCmp;

  // Zero, regardless of sign, is equal.
  सचे (Floats[1].isZero() && RHS.Floats[1].isZero())
    पटिनिवत्त cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  ध्रुव तार्किक ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  ध्रुव तार्किक RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  सचे (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    पटिनिवत्त RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  सचे (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    पटिनिवत्त ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  सचे (ThisIsSubtractive != RHSIsSubtractive)
    पटिनिवत्त ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  ध्रुव cmpResult LoPartCmp = Floats[1].compare_Absolute_मानम्(RHS.Floats[1]);

  सचे (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    सचे (LoPartCmp == cmpLessThan)
      पटिनिवत्त cmpGreaterThan;
    सचे (LoPartCmp == cmpGreaterThan)
      पटिनिवत्त cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  पटिनिवत्त LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() ध्रुव {
  पटिनिवत्त Floats[0].getCategory();
}

तार्किक DoubleAPFloat::isNegative() ध्रुव { पटिनिवत्त Floats[0].isNegative(); }

शून्य DoubleAPFloat::makeInf(तार्किक Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ मिच्छा);
}

शून्य DoubleAPFloat::makeZero(तार्किक Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ मिच्छा);
}

शून्य DoubleAPFloat::makeLargest(तार्किक Neg) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  सचे (Neg)
    changeSign();
}

शून्य DoubleAPFloat::makeSmallest(तार्किक Neg) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].makeSmallest(Neg);
  Floats[1].makeZero(/* Neg = */ मिच्छा);
}

शून्य DoubleAPFloat::makeSmallestNormalized(तार्किक Neg) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  सचे (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ मिच्छा);
}

शून्य DoubleAPFloat::makeNaN(तार्किक SNaN, तार्किक Neg, ध्रुव APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ मिच्छा);
}

APFloat::cmpResult DoubleAPFloat::compare(ध्रुव DoubleAPFloat &RHS) ध्रुव {
  auto Result = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  सचे (Result == APFloat::cmpEqual)
    पटिनिवत्त Floats[1].compare(RHS.Floats[1]);
  पटिनिवत्त Result;
}

तार्किक DoubleAPFloat::bitwiseIsEqual(ध्रुव DoubleAPFloat &RHS) ध्रुव {
  पटिनिवत्त Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_code hash_मानम्(ध्रुव DoubleAPFloat &Arg) {
  सचे (Arg.Floats)
    पटिनिवत्त hash_combine(hash_मानम्(Arg.Floats[0]), hash_मानम्(Arg.Floats[1]));
  पटिनिवत्त hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcastToAPInt() ध्रुव {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t Data[] = {
      Floats[0].bitcastToAPInt().getRawData()[0],
      Floats[1].bitcastToAPInt().getRawData()[0],
  };
  पटिनिवत्त APInt(128, Data);
}

Expected<APFloat::opStatus> DoubleAPFloat::convertFromString(StringRef S,
                                                             roundingMode RM) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy);
  auto Ret = Tmp.convertFromString(S, RM);
  *एतत् = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  पटिनिवत्त Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::opStatus DoubleAPFloat::next(तार्किक nextDown) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  सचे (nextDown) {
    changeSign();
    APFloat::opStatus Result = next(/*nextDown=*/मिच्छा);
    changeSign();
    पटिनिवत्त Result;
  }
  विकल्प (getCategory()) {
  पक्ष fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    सचे (isNegative())
      makeLargest(सच्च);
    पटिनिवत्त opOK;

  पक्ष fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    सचे (getFirst().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(मिच्छा, isNegative(), nullptr);
      पटिनिवत्त opInvalidOp;
    }
    पटिनिवत्त opOK;

  पक्ष fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(मिच्छा);
    पटिनिवत्त opOK;

  पक्ष fcNormal:
    भङ्ग;
  }

  ध्रुव APFloat &HiOld = getFirst();
  ध्रुव APFloat &LoOld = getSecond();

  APFloat NextLo = LoOld;
  NextLo.next(/*nextDown=*/मिच्छा);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  auto InLattice = [](ध्रुव APFloat &Hi, ध्रुव APFloat &Lo) {
    पटिनिवत्त Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  सचे (InLattice(HiOld, NextLo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(NextLo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat Largest{*Semantics, uninitialized};
    Largest.makeLargest(/*Neg=*/मिच्छा);
    सचे (compare(Largest) == cmpGreaterThan)
      makeInf(/*Neg=*/मिच्छा);

    पटिनिवत्त opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat NextHi = HiOld;
  NextHi.next(/*nextDown=*/मिच्छा);

  // nextUp(getLargest()) == INFINITY
  सचे (NextHi.isInfinity()) {
    makeInf(/*Neg=*/मिच्छा);
    पटिनिवत्त opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  सचे (NextHi.isZero()) {
    makeZero(/*Neg=*/सच्च);
    पटिनिवत्त opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  NextLo = neg(scalbn(harrisonUlp(NextHi), -1, rmTowardZero));
  सचे (!InLattice(NextHi, NextLo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    NextLo.next(/*nextDown=*/मिच्छा);

  Floats[0] = std::move(NextHi);
  Floats[1] = std::move(NextLo);

  पटिनिवत्त opOK;
}

APFloat::opStatus DoubleAPFloat::convertToSignExtendedInteger(
    MutableArrayRef<integerPart> निवेशः, unsigned पूर्णाङ्क Width, तार्किक IsSigned,
    roundingMode RM, तार्किक *IsExact) ध्रुव {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  सचे (!getFirst().isFiniteNonZero() || getSecond().isZero())
    पटिनिवत्त getFirst().convertToInteger(निवेशः, Width, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = मिच्छा;
  DoubleAPFloat Integral = *एतत्;
  ध्रुव opStatus RoundStatus = Integral.roundToIntegral(RM);
  सचे (RoundStatus == opInvalidOp)
    पटिनिवत्त opInvalidOp;
  ध्रुव APFloat &IntegralHi = Integral.getFirst();
  ध्रुव APFloat &IntegralLo = Integral.getSecond();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  तार्किक HiIsExact;
  सचे (IntegralHi.isZero() || IntegralLo.isZero()) {
    ध्रुव opStatus HiStatus =
        IntegralHi.convertToInteger(निवेशः, Width, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    पटिनिवत्त HiStatus == opOK ? opInexact : HiStatus;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  सचे (!IsSigned && IntegralHi.isNegative())
    पटिनिवत्त opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  तार्किक LoIsExact;
  ध्रुव पूर्णाङ्क HiExactLog2 = IntegralHi.getExactLog2Abs();
  ध्रुव unsigned PositiveOverflowWidth = IsSigned ? Width - 1 : Width;
  सचे (HiExactLog2 >= 0 &&
      static_cast<unsigned>(HiExactLog2) == PositiveOverflowWidth) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    सचे (IntegralHi.isNegative() == IntegralLo.isNegative())
      पटिनिवत्त opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    ध्रुव opStatus LoStatus = IntegralLo.convertToInteger(
        निवेशः, Width, /*IsSigned=*/सच्च, RM, &LoIsExact);
    सचे (LoStatus == opInvalidOp)
      पटिनिवत्त opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    सचे (IsSigned && !IntegralHi.isNegative())
      APInt::tcClearBit(निवेशः.data(), PositiveOverflowWidth);
    *IsExact = RoundStatus == opOK;
    पटिनिवत्त RoundStatus;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  ध्रुव opStatus HiStatus = IntegralHi.convertToInteger(
      निवेशः, Width, IsSigned, rmTowardZero, &HiIsExact);
  सचे (HiStatus == opInvalidOp)
    पटिनिवत्त HiStatus;

  // Convert Lo into a temporary integer of the same width.
  APSInt LoResult{Width, /*isUnsigned=*/!IsSigned};
  ध्रुव opStatus LoStatus =
      IntegralLo.convertToInteger(LoResult, rmTowardZero, &LoIsExact);
  सचे (LoStatus == opInvalidOp)
    पटिनिवत्त LoStatus;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tcAdd(निवेशः.data(), LoResult.getRawData(), /*carry=*/0, निवेशः.size());

  *IsExact = RoundStatus == opOK;
  पटिनिवत्त RoundStatus;
}

APFloat::opStatus
DoubleAPFloat::convertToInteger(MutableArrayRef<integerPart> निवेशः,
                                unsigned पूर्णाङ्क Width, तार्किक IsSigned,
                                roundingMode RM, तार्किक *IsExact) ध्रुव {
  opStatus FS =
      convertToSignExtendedInteger(निवेशः, Width, IsSigned, RM, IsExact);

  सचे (FS == opInvalidOp) {
    ध्रुव unsigned Dst_Parts_गणनम् = part_गणनम्_For_Bits(Width);
    प्रतिज्ञा(Dst_Parts_गणनम् <= निवेशः.size() && "Integer too big");

    unsigned Bits;
    सचे (getCategory() == fcNaN)
      Bits = 0;
    अञ्ञथा सचे (isNegative())
      Bits = IsSigned;
    अञ्ञथा
      Bits = Width - IsSigned;

    tcSetLeastSignificantBits(निवेशः.data(), Dst_Parts_गणनम्, Bits);
    सचे (isNegative() && IsSigned)
      APInt::tc_Shift_वामः(निवेशः.data(), Dst_Parts_गणनम्, Width - 1);
  }

  पटिनिवत्त FS;
}

APFloat::opStatus DoubleAPFloat::handleOverflow(roundingMode RM) {
  विकल्प (RM) {
  पक्ष APFloat::rmTowardZero:
    makeLargest(/*Neg=*/isNegative());
    भङ्ग;
  पक्ष APFloat::rmTowardNegative:
    सचे (isNegative())
      makeInf(/*Neg=*/सच्च);
    अञ्ञथा
      makeLargest(/*Neg=*/मिच्छा);
    भङ्ग;
  पक्ष APFloat::rmTowardPositive:
    सचे (isNegative())
      makeLargest(/*Neg=*/सच्च);
    अञ्ञथा
      makeInf(/*Neg=*/मिच्छा);
    भङ्ग;
  पक्ष APFloat::rmNearestTiesToAway:
  पक्ष APFloat::rmNearestTiesToEven:
    makeInf(/*Neg=*/isNegative());
    भङ्ग;
  स्वभाव:
    llvm_unreachable("Invalid rounding mode found");
  }
  opStatus S = opInexact;
  सचे (!getFirst().isFinite())
    S = static_cast<opStatus>(S | opOverflow);
  पटिनिवत्त S;
}

APFloat::opStatus DoubleAPFloat::convertFromUnsignedParts(
    ध्रुव integerPart *Src, unsigned पूर्णाङ्क Src_गणनम्, roundingMode RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  ध्रुव unsigned SrcMSB = APInt::tcMSB(Src, Src_गणनम्);
  सचे (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/मिच्छा);
    पटिनिवत्त opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  ध्रुव unsigned SrcBitWidth = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/SrcBitWidth, ArrayRef(Src, Src_गणनम्)},
                /*isUnsigned=*/सच्च};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{getFirst().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/मिच्छा, rmNearestTiesToEven);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  सचे (!Hi.isFinite())
    पटिनिवत्त handleOverflow(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  तार्किक HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/सच्च};
  Hi.convertToInteger(HiAsInt, rmNearestTiesToEven, &HiAsIntIsExact);
  ध्रुव APInt Error = SrcInt.zext(HiAsInt.getBitWidth()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  roundingMode LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  सचे (Error.isNegative()) {
    सचे (RM == rmNearestTiesToAway) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      ध्रुव unsigned ErrorActiveBits = Error.getSignificantBits() - 1;
      ध्रुव unsigned LoPrecision = getSecond().getSemantics().precision;
      सचे (ErrorActiveBits > LoPrecision) {
        ध्रुव unsigned RoundingBoundary = ErrorActiveBits - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        सचे (Error.गणनम्_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rmTowardPositive;
      }
    } अञ्ञथा सचे (RM == rmTowardZero) {
      // For rmTowardZero, the final positive result must be truncated (rounded
      // down). When Hi is an overestimate, Error is negative. A standard
      // rmTowardZero conversion of Error would make it *less* negative,
      // effectively rounding the final sum Hi + Lo *up*. To ensure the sum
      // rounds down correctly, we force Lo to round toward -infinity.
      LoRM = rmTowardNegative;
    }
  }

  APFloat Lo{getSecond().getSemantics()};
  opStatus Status = Lo.convertFromAPInt(Error, /*IsSigned=*/सच्च, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fastTwoSum(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  सचे (!getFirst().isFinite())
    पटिनिवत्त handleOverflow(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  सचे (getFirst().isFiniteNonZero() && Floats[0].isLargest()) {
    DoubleAPFloat Largest{*Semantics};
    Largest.makeLargest(/*Neg=*/मिच्छा);
    सचे (compare(Largest) == APFloat::cmpGreaterThan)
      पटिनिवत्त handleOverflow(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  पटिनिवत्त Status;
}

APFloat::opStatus DoubleAPFloat::convertFromAPInt(ध्रुव APInt &निवेशः,
                                                  तार्किक IsSigned,
                                                  roundingMode RM) {
  ध्रुव तार्किक Negate_निवेशः = IsSigned && निवेशः.isNegative();
  APInt API = निवेशः;
  सचे (Negate_निवेशः)
    API.negate();

  ध्रुव APFloat::opStatus Status =
      convertFromUnsignedParts(API.getRawData(), API.get_अङ्कः_शब्दाः(), RM);
  सचे (Negate_निवेशः)
    changeSign();
  पटिनिवत्त Status;
}

unsigned पूर्णाङ्क DoubleAPFloat::convertToHexString(वर्ण *DST,
                                               unsigned पूर्णाङ्क HexDigits,
                                               तार्किक UpperCase,
                                               roundingMode RM) ध्रुव {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  पटिनिवत्त APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .convertToHexString(DST, HexDigits, UpperCase, RM);
}

तार्किक DoubleAPFloat::isDenormal() ध्रुव {
  पटिनिवत्त getCategory() == fcNormal &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

तार्किक DoubleAPFloat::isSmallest() ध्रुव {
  सचे (getCategory() != fcNormal)
    पटिनिवत्त मिच्छा;
  DoubleAPFloat Tmp(*एतत्);
  Tmp.makeSmallest(एतत्->isNegative());
  पटिनिवत्त Tmp.compare(*एतत्) == cmpEqual;
}

तार्किक DoubleAPFloat::isSmallestNormalized() ध्रुव {
  सचे (getCategory() != fcNormal)
    पटिनिवत्त मिच्छा;

  DoubleAPFloat Tmp(*एतत्);
  Tmp.makeSmallestNormalized(एतत्->isNegative());
  पटिनिवत्त Tmp.compare(*एतत्) == cmpEqual;
}

तार्किक DoubleAPFloat::isLargest() ध्रुव {
  सचे (getCategory() != fcNormal)
    पटिनिवत्त मिच्छा;
  DoubleAPFloat Tmp(*एतत्);
  Tmp.makeLargest(एतत्->isNegative());
  पटिनिवत्त Tmp.compare(*एतत्) == cmpEqual;
}

तार्किक DoubleAPFloat::isInteger() ध्रुव {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  पटिनिवत्त Floats[0].isInteger() && Floats[1].isInteger();
}

शून्य DoubleAPFloat::toString(SmallVectorImpl<वर्ण> &Str,
                             unsigned FormatPrecision,
                             unsigned FormatMaxPadding,
                             तार्किक TruncateZero) ध्रुव {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .toString(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

पूर्णाङ्क DoubleAPFloat::getExactLog2Abs() ध्रुव {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  सचे (getSecond().isNonZero())
    पटिनिवत्त INT_MIN;
  पटिनिवत्त getFirst().getExactLog2Abs();
}

पूर्णाङ्क ilogb(ध्रुव DoubleAPFloat &Arg) {
  ध्रुव APFloat &Hi = Arg.getFirst();
  ध्रुव APFloat &Lo = Arg.getSecond();
  पूर्णाङ्क IlogbResult = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  सचे (Arg.getCategory() != fcNormal)
    पटिनिवत्त IlogbResult;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  सचे (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    पटिनिवत्त IlogbResult;
  सचे (Hi.getExactLog2Abs() == INT_MIN)
    पटिनिवत्त IlogbResult;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  पटिनिवत्त IlogbResult - 1;
}

DoubleAPFloat scalbn(ध्रुव DoubleAPFloat &Arg, पूर्णाङ्क Exp,
                     APFloat::roundingMode RM) {
  प्रतिज्ञा(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");
  पटिनिवत्त DoubleAPFloat(APFloatBase::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(ध्रुव DoubleAPFloat &Arg, पूर्णाङ्क &Exp,
                    APFloat::roundingMode RM) {
  प्रतिज्ञा(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  सचे (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.getFirst() = Quiet.getFirst().makeQuiet();
    पटिनिवत्त Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  सचे (Exp == APFloat::IEK_Inf)
    पटिनिवत्त Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  सचे (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    पटिनिवत्त Arg;
  }

  ध्रुव APFloat &Hi = Arg.getFirst();
  ध्रुव APFloat &Lo = Arg.getSecond();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  ध्रुव तार्किक SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat Second = Lo;
  सचे (Arg.getCategory() == APFloat::fcNormal && Lo.isFiniteNonZero()) {
    roundingMode LoRoundingMode;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    सचे (RM == rmTowardZero)
      LoRoundingMode = Arg.isNegative() ? rmTowardPositive : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    अञ्ञथा सचे (RM == rmNearestTiesToAway && SignsDisagree && Exp > 0)
      LoRoundingMode = rmNearestTiesToEven;
    अञ्ञथा
      LoRoundingMode = RM;
    Second = scalbn(Lo, -Exp, LoRoundingMode);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    सचे (RM == rmNearestTiesToAway && LoRoundingMode == rmNearestTiesToEven) {
      // Re-scale the result back to check if rounding occurred.
      ध्रुव APFloat RecomposedLo = scalbn(Second, Exp, rmNearestTiesToEven);
      सचे (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        ध्रुव APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        ध्रुव APFloat UlpOfSecond = harrisonUlp(Second);
        ध्रुव APFloat ScaledUlpOfSecond =
            scalbn(UlpOfSecond, Exp - 1, rmNearestTiesToEven);
        ध्रुव तार्किक IsMidpoint = abs(RoundingError) == ScaledUlpOfSecond;
        ध्रुव तार्किक RoundedLoAway =
            Second.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        सचे (IsMidpoint && RoundedLoAway)
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
    सचे (Second.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat First = scalbn(Hi, -Exp, RM);
  पटिनिवत्त DoubleAPFloat(APFloatBase::PPCDoubleDouble(), std::move(First),
                       std::move(Second));
}

APInt DoubleAPFloat::getNaNPayload() ध्रुव { पटिनिवत्त Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, ध्रुव fltSemantics &Semantics) {
  सचे (usesLayout<IEEEFloat>(Semantics)) {
    नूतन (&IEEE) IEEEFloat(std::move(F));
    पटिनिवत्त;
  }
  सचे (usesLayout<DoubleAPFloat>(Semantics)) {
    ध्रुव fltSemantics& S = F.getSemantics();
    नूतन (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloatBase::IEEEdouble()));
    पटिनिवत्त;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::opStatus> APFloat::convertFromString(StringRef Str,
                                                       roundingMode RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_code hash_मानम्(ध्रुव APFloat &Arg) {
  सचे (APFloat::usesLayout<detail::IEEEFloat>(Arg.getSemantics()))
    पटिनिवत्त hash_मानम्(Arg.U.IEEE);
  सचे (APFloat::usesLayout<detail::DoubleAPFloat>(Arg.getSemantics()))
    पटिनिवत्त hash_मानम्(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(ध्रुव fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  auto StatusOrErr = convertFromString(S, rmNearestTiesToEven);
  प्रतिज्ञा(StatusOrErr && "Invalid floating point representation");
  consumeError(StatusOrErr.takeError());
}

FPClassTest APFloat::classify() ध्रुव {
  सचे (isZero())
    पटिनिवत्त isNegative() ? fcNegZero : fcPosZero;
  सचे (isNormal())
    पटिनिवत्त isNegative() ? fcNegNormal : fcPosNormal;
  सचे (isDenormal())
    पटिनिवत्त isNegative() ? fcNegSubnormal : fcPosSubnormal;
  सचे (isInfinity())
    पटिनिवत्त isNegative() ? fcNegInf : fcPosInf;
  प्रतिज्ञा(isNaN() && "Other class of FP constant");
  पटिनिवत्त isSignaling() ? fcSNan : fcQNan;
}

तार्किक APFloat::getExactInverse(APFloat *Inv) ध्रुव {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  सचे (!isFiniteNonZero())
    पटिनिवत्त मिच्छा;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  सचे (isDenormal())
    पटिनिवत्त मिच्छा;

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
  ध्रुव पूर्णाङ्क Exp = getExactLog2Abs();
  सचे (Exp == INT_MIN)
    पटिनिवत्त मिच्छा;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  सचे (Reciprocal.getExactLog2Abs() != -Exp)
    पटिनिवत्त मिच्छा;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  सचे (Reciprocal.isDenormal())
    पटिनिवत्त मिच्छा;

  प्रतिज्ञा(Reciprocal.isFiniteNonZero());

  सचे (Inv)
    *Inv = std::move(Reciprocal);

  पटिनिवत्त सच्च;
}

APFloat::opStatus APFloat::convert(ध्रुव fltSemantics &ToSemantics,
                                   roundingMode RM, तार्किक *losesInfo) {
  सचे (&getSemantics() == &ToSemantics) {
    *losesInfo = मिच्छा;
    पटिनिवत्त opOK;
  }
  सचे (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics))
    पटिनिवत्त U.IEEE.convert(ToSemantics, RM, losesInfo);
  सचे (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<DoubleAPFloat>(ToSemantics)) {
    प्रतिज्ञा(&ToSemantics == &APFloatBase::semPPCDoubleDouble);
    auto Ret =
        U.IEEE.convert(APFloatBase::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *एतत् = APFloat(ToSemantics, U.IEEE.bitcastToAPInt());
    पटिनिवत्त Ret;
  }
  सचे (usesLayout<DoubleAPFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics)) {
    auto Ret = getIEEE().convert(ToSemantics, RM, losesInfo);
    *एतत् = APFloat(std::move(getIEEE()), ToSemantics);
    पटिनिवत्त Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_मानम्(ध्रुव fltSemantics &Semantics) {
  पटिनिवत्त APFloat(Semantics, APInt::getAllOnes(Semantics.sizeInBits));
}

शून्य APFloat::print(raw_ostream &OS) ध्रुव {
  SmallVector<वर्ण, 16> Buffer;
  toString(Buffer);
  OS << Buffer;
}

#सचे !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य APFloat::dump() ध्रुव {
  print(dbgs());
  dbgs() << '\n';
}
#endif

शून्य APFloat::Profile(Folding_Set_ग्रन्थिः_ID &NID) ध्रुव {
  NID.Add(bitcastToAPInt());
}

APFloat::opStatus APFloat::convertToInteger(APSInt &result,
                                            roundingMode rounding_mode,
                                            तार्किक *isExact) ध्रुव {
  unsigned bitWidth = result.getBitWidth();
  SmallVector<uint64_t, 4> parts(result.get_अङ्कः_शब्दाः());
  opStatus status = convertToInteger(parts, bitWidth, result.isSigned(),
                                     rounding_mode, isExact);
  // Keeps the original signed-ness.
  result = APInt(bitWidth, parts);
  पटिनिवत्त status;
}

द्विगुण APFloat::convertToDouble() ध्रुव {
  सचे (&getSemantics() == &APFloatBase::semIEEEdouble)
    पटिनिवत्त getIEEE().convertToDouble();
  प्रतिज्ञा(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat Temp = *एतत्;
  तार्किक LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &LosesInfo);
  प्रतिज्ञा(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  पटिनिवत्त Temp.getIEEE().convertToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 APFloat::convertToQuad() ध्रुव {
  सचे (&getSemantics() == &APFloatBase::semIEEEquad)
    पटिनिवत्त getIEEE().convertToQuad();
  प्रतिज्ञा(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat Temp = *एतत्;
  तार्किक LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEquad, rmNearestTiesToEven, &LosesInfo);
  प्रतिज्ञा(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  पटिनिवत्त Temp.getIEEE().convertToQuad();
}
#endif

भिन्न APFloat::convertToFloat() ध्रुव {
  सचे (&getSemantics() == &APFloatBase::semIEEEsingle)
    पटिनिवत्त getIEEE().convertToFloat();
  प्रतिज्ञा(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat Temp = *एतत्;
  तार्किक LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEsingle, rmNearestTiesToEven, &LosesInfo);
  प्रतिज्ञा(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  पटिनिवत्त Temp.getIEEE().convertToFloat();
}

unsigned APFloatBase::getArbitraryFPFormatSizeInBits(StringRef Format) {
  पटिनिवत्त StringSwitch<unsigned>(Format)
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

तार्किक APFloatBase::isValidArbitraryFPFormat(StringRef Format) {
  पटिनिवत्त getArbitraryFPFormatSizeInBits(Format) != 0;
}

ध्रुव fltSemantics *APFloatBase::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  पटिनिवत्त StringSwitch<ध्रुव fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(nullptr);
}

APFloat::Storage::~Storage() {
  सचे (usesLayout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    पटिनिवत्त;
  }
  सचे (usesLayout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    पटिनिवत्त;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(ध्रुव APFloat::Storage &RHS) {
  सचे (usesLayout<IEEEFloat>(*RHS.semantics)) {
    नूतन (एतत्) IEEEFloat(RHS.IEEE);
    पटिनिवत्त;
  }
  सचे (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    नूतन (एतत्) DoubleAPFloat(RHS.Double);
    पटिनिवत्त;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  सचे (usesLayout<IEEEFloat>(*RHS.semantics)) {
    नूतन (एतत्) IEEEFloat(std::move(RHS.IEEE));
    पटिनिवत्त;
  }
  सचे (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    नूतन (एतत्) DoubleAPFloat(std::move(RHS.Double));
    पटिनिवत्त;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::operator=(ध्रुव APFloat::Storage &RHS) {
  सचे (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } अञ्ञथा सचे (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } अञ्ञथा सचे (एतत् != &RHS) {
    एतत्->~Storage();
    नूतन (एतत्) Storage(RHS);
  }
  पटिनिवत्त *एतत्;
}

APFloat::Storage &APFloat::Storage::operator=(APFloat::Storage &&RHS) {
  सचे (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } अञ्ञथा सचे (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } अञ्ञथा सचे (एतत् != &RHS) {
    एतत्->~Storage();
    नूतन (एतत्) Storage(std::move(RHS));
  }
  पटिनिवत्त *एतत्;
}

नामाकाश {

APFloat::opStatus getOpStatusFromLibc(पूर्णाङ्क libc_exceptions) {
  APFloat::opStatus status = APFloat::opOK;
  सचे (libc_exceptions & FE_INVALID)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInvalidOp);
  सचे (libc_exceptions & FE_DIVBYZERO)
    status = static_cast<APFloat::opStatus>(status | APFloat::opDivByZero);
  सचे (libc_exceptions & FE_OVERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opOverflow);
  सचे (libc_exceptions & FE_UNDERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opUnderflow);
  सचे (libc_exceptions & FE_INEXACT)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInexact);
  पटिनिवत्त status;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> exp(ध्रुव APFloat &x, RoundingMode rounding_mode,
                           APFloat::opStatus *status) {

  सचे (rounding_mode == APFloatBase::rmNearestTiesToEven) {
    सचे (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEsingle) {
      भिन्न x_val = x.convertToFloat();
      पूर्णाङ्क exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      सचे (status) {
        *status = getOpStatusFromLibc(exc);
        सचे (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      भिन्न result = LIBC_NAMESPACE::shared::expf(x_val);
      पटिनिवत्त APFloat(result);
    }
    सचे (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEdouble) {
      द्विगुण x_val = x.convertToDouble();
      पूर्णाङ्क exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      सचे (status) {
        *status = getOpStatusFromLibc(exc);
        सचे (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      द्विगुण result = LIBC_NAMESPACE::shared::exp(x_val);
      पटिनिवत्त APFloat(result);
    }
  }
  पटिनिवत्त std::nullopt;
}

} // namespace llvm

#undef APFLOAT_DISPATCH_ON_SEMANTICS
