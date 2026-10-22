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
  करेहि {                                                                         \
    जइ (usesLayout<IEEEFloat>(getSemantics()))                                 \
      प्रत्यागम U.IEEE.METHOD_CALL;                                               \
    जइ (usesLayout<DoubleAPFloat>(getSemantics()))                             \
      प्रत्यागम U.Double.METHOD_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } जाव (मिच्छ)

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
    मिच्छ,
    मिच्छ,
    मिच्छ,
    मिच्छ};

constexpr fltSemantics APFloatBase::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    सच्च,
    मिच्छ,
    मिच्छ};

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
    प्रत्यागम IEEEhalf();
  पक्ष S_BFloat:
    प्रत्यागम BFloat();
  पक्ष S_IEEEsingle:
    प्रत्यागम IEEEsingle();
  पक्ष S_IEEEdouble:
    प्रत्यागम IEEEdouble();
  पक्ष S_IEEEquad:
    प्रत्यागम IEEEquad();
  पक्ष S_x87DoubleExtended:
    प्रत्यागम x87DoubleExtended();
  पक्ष S_PPCDoubleDouble:
    प्रत्यागम PPCDoubleDouble();
  पक्ष S_PPCDoubleDoubleLegacy:
    प्रत्यागम PPCDoubleDoubleLegacy();
  पक्ष S_Float8E5M2:
    प्रत्यागम Float8E5M2();
  पक्ष S_Float8E5M2FNUZ:
    प्रत्यागम Float8E5M2FNUZ();
  पक्ष S_Float8E4M3:
    प्रत्यागम Float8E4M3();
  पक्ष S_Float8E4M3FN:
    प्रत्यागम Float8E4M3FN();
  पक्ष S_Float8E4M3FNUZ:
    प्रत्यागम Float8E4M3FNUZ();
  पक्ष S_Float8E4M3B11FNUZ:
    प्रत्यागम Float8E4M3B11FNUZ();
  पक्ष S_Float8E3M4:
    प्रत्यागम Float8E3M4();
  पक्ष S_FloatTF32:
    प्रत्यागम FloatTF32();
  पक्ष S_Float8E8M0FNU:
    प्रत्यागम Float8E8M0FNU();
  पक्ष S_Float8E5M3FNU:
    प्रत्यागम Float8E5M3FNU();
  पक्ष S_Float6E3M2FN:
    प्रत्यागम Float6E3M2FN();
  पक्ष S_Float6E2M3FN:
    प्रत्यागम Float6E2M3FN();
  पक्ष S_Float4E2M1FN:
    प्रत्यागम Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloatBase::Semantics
APFloatBase::SemanticsToEnum(ध्रुव llvm::fltSemantics &Sem) {
  जइ (&Sem == &llvm::APFloat::IEEEhalf())
    प्रत्यागम S_IEEEhalf;
  अण्णहा जइ (&Sem == &llvm::APFloat::BFloat())
    प्रत्यागम S_BFloat;
  अण्णहा जइ (&Sem == &llvm::APFloat::IEEEsingle())
    प्रत्यागम S_IEEEsingle;
  अण्णहा जइ (&Sem == &llvm::APFloat::IEEEdouble())
    प्रत्यागम S_IEEEdouble;
  अण्णहा जइ (&Sem == &llvm::APFloat::IEEEquad())
    प्रत्यागम S_IEEEquad;
  अण्णहा जइ (&Sem == &llvm::APFloat::x87DoubleExtended())
    प्रत्यागम S_x87DoubleExtended;
  अण्णहा जइ (&Sem == &llvm::APFloat::PPCDoubleDouble())
    प्रत्यागम S_PPCDoubleDouble;
  अण्णहा जइ (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    प्रत्यागम S_PPCDoubleDoubleLegacy;
  अण्णहा जइ (&Sem == &llvm::APFloat::Float8E5M2())
    प्रत्यागम S_Float8E5M2;
  अण्णहा जइ (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    प्रत्यागम S_Float8E5M2FNUZ;
  अण्णहा जइ (&Sem == &llvm::APFloat::Float8E4M3())
    प्रत्यागम S_Float8E4M3;
  अण्णहा जइ (&Sem == &llvm::APFloat::Float8E4M3FN())
    प्रत्यागम S_Float8E4M3FN;
  अण्णहा जइ (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    प्रत्यागम S_Float8E4M3FNUZ;
  अण्णहा जइ (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    प्रत्यागम S_Float8E4M3B11FNUZ;
  अण्णहा जइ (&Sem == &llvm::APFloat::Float8E3M4())
    प्रत्यागम S_Float8E3M4;
  अण्णहा जइ (&Sem == &llvm::APFloat::FloatTF32())
    प्रत्यागम S_FloatTF32;
  अण्णहा जइ (&Sem == &llvm::APFloat::Float8E8M0FNU())
    प्रत्यागम S_Float8E8M0FNU;
  अण्णहा जइ (&Sem == &llvm::APFloat::Float8E5M3FNU())
    प्रत्यागम S_Float8E5M3FNU;
  अण्णहा जइ (&Sem == &llvm::APFloat::Float6E3M2FN())
    प्रत्यागम S_Float6E3M2FN;
  अण्णहा जइ (&Sem == &llvm::APFloat::Float6E2M3FN())
    प्रत्यागम S_Float6E2M3FN;
  अण्णहा जइ (&Sem == &llvm::APFloat::Float4E2M1FN())
    प्रत्यागम S_Float4E2M1FN;
  अण्णहा
    llvm_unreachable("Unknown floating semantics");
}

तार्किक APFloatBase::isRepresentableBy(ध्रुव fltSemantics &A,
                                    ध्रुव fltSemantics &B) {
  प्रत्यागम A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

तार्किक APFloatBase::isLosslesslyConvertibleTo(ध्रुव fltSemantics &From,
                                            ध्रुव fltSemantics &InitialTo,
                                            तार्किक IgnoreNaNs) {
  जइ (&From == &InitialTo)
    प्रत्यागम सच्च;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  जइ (&From == &semPPCDoubleDouble)
    प्रत्यागम मिच्छ;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  ध्रुव fltSemantics &To =
      &InitialTo == &semPPCDoubleDouble ? semIEEEdouble : InitialTo;

  जइ (!isRepresentableBy(From, To))
    प्रत्यागम मिच्छ;

  जइ ((From.hasZero && !To.hasZero) ||
      (From.hasSignedRepr && !To.hasSignedRepr))
    प्रत्यागम मिच्छ;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  तार्किक FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  तार्किक ToHasSignedZero = To.hasZero && To.hasSignedRepr &&
                         To.nanEncoding != fltNanEncoding::NegativeZero;
  जइ (FromHasSignedZero && !ToHasSignedZero)
    प्रत्यागम मिच्छ;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat SmallestFrom = APFloat::getSmallest(From);
  तार्किक LosesInfo = मिच्छ;
  (शून्य)SmallestFrom.convert(To, APFloat::rmNearestTiesToEven, &LosesInfo);
  जइ (LosesInfo)
    प्रत्यागम मिच्छ;

  जइ (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    प्रत्यागम सच्च;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  जइ (IgnoreNaNs) {
    जइ (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      प्रत्यागम To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    प्रत्यागम To.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  जइ (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    प्रत्यागम मिच्छ;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  जइ (To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    प्रत्यागम सच्च;
  प्रत्यागम To.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
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
  प्रत्यागम 2 + ((power * 815) / (351 * APFloatBase::integerPartWidth));
}

unsigned पूर्णाङ्क APFloatBase::semanticsPrecision(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.precision;
}
APFloatBase::ExponentType
APFloatBase::semanticsMaxExponent(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.maxExponent;
}
APFloatBase::ExponentType
APFloatBase::semanticsMinExponent(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.minExponent;
}
unsigned पूर्णाङ्क APFloatBase::semanticsSizeInBits(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.sizeInBits;
}
unsigned पूर्णाङ्क APFloatBase::semanticsIntSizeInBits(ध्रुव fltSemantics &semantics,
                                                 तार्किक isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  unsigned पूर्णाङ्क MinBitWidth = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  जइ (isSigned)
    ++MinBitWidth;
  प्रत्यागम MinBitWidth;
}

तार्किक APFloatBase::semanticsHasZero(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.hasZero;
}

तार्किक APFloatBase::semanticsHasSignedRepr(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.hasSignedRepr;
}

तार्किक APFloatBase::semanticsHasInf(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

तार्किक APFloatBase::semanticsHasNaN(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

तार्किक APFloatBase::isIEEELikeFP(ध्रुव fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  प्रत्यागम SemanticsToEnum(semantics) <= S_IEEEquad;
}

तार्किक APFloatBase::hasSignBitInMSB(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.hasSignBitInMSB;
}

तार्किक APFloatBase::isRepresentableAsNormalIn(ध्रुव fltSemantics &Src,
                                            ध्रुव fltSemantics &Dst) {
  // Exponent range must be larger.
  जइ (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    प्रत्यागम मिच्छ;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  प्रत्यागम Dst.precision >= Src.precision;
}

unsigned APFloatBase::getSizeInBits(ध्रुव fltSemantics &Sem) {
  प्रत्यागम Sem.sizeInBits;
}

स्थिर constexpr APFloatBase::ExponentType
exponentZero(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.minExponent - 1;
}

स्थिर constexpr APFloatBase::ExponentType
exponentInf(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.maxExponent + 1;
}

स्थिर constexpr APFloatBase::ExponentType
exponentNaN(ध्रुव fltSemantics &semantics) {
  जइ (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    जइ (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      प्रत्यागम exponentZero(semantics);
    जइ (semantics.hasSignedRepr || semantics.precision > 1)
      प्रत्यागम semantics.maxExponent;
  }
  प्रत्यागम semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

स्थिर अन्तःस्थ Error createError(ध्रुव Twine &Err) {
  प्रत्यागम make_error<StringError>(Err, inconvertibleErrorCode());
}

स्थिर constexpr अन्तःस्थ unsigned पूर्णाङ्क part_गणनम्_For_Bits(unsigned पूर्णाङ्क bits) {
  प्रत्यागम std::max(1u, (bits + APFloatBase::integerPartWidth - 1) /
                          APFloatBase::integerPartWidth);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
स्थिर अन्तःस्थ unsigned पूर्णाङ्क
dec_Digit_मानम्(unsigned पूर्णाङ्क c)
{
  प्रत्यागम c - '0';
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
  जइ (p == end || ((*p == '-' || *p == '+') && (p + 1) == end))
    प्रत्यागम 0;

  तार्किक isNegative = *p == '-';
  जइ (*p == '-' || *p == '+') {
    p++;
    जइ (p == end)
      प्रत्यागम createError("Exponent has no digits");
  }

  unsigned absExponent = dec_Digit_मानम्(*p++);
  जइ (absExponent >= 10U)
    प्रत्यागम createError("Invalid character in exponent");

  प्रति (; p != end; ++p) {
    unsigned मानम्_2 = dec_Digit_मानम्(*p);
    जइ (मानम्_2 >= 10U)
      प्रत्यागम createError("Invalid character in exponent");

    absExponent = absExponent * 10U + मानम्_2;
    जइ (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      भङ्ग;
    }
  }

  जइ (isNegative)
    प्रत्यागम -(पूर्णाङ्क) absExponent;
  अण्णहा
    प्रत्यागम (पूर्णाङ्क) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
स्थिर Expected<पूर्णाङ्क> totalExponent(StringRef::iterator p,
                                   StringRef::iterator end,
                                   पूर्णाङ्क exponentAdjustment) {
  पूर्णाङ्क exponent = 0;

  जइ (p == end)
    प्रत्यागम createError("Exponent has no digits");

  तार्किक negative = *p == '-';
  जइ (*p == '-' || *p == '+') {
    p++;
    जइ (p == end)
      प्रत्यागम createError("Exponent has no digits");
  }

  पूर्णाङ्क unsignedExponent = 0;
  तार्किक overflow = मिच्छ;
  प्रति (; p != end; ++p) {
    unsigned पूर्णाङ्क मानम्_2;

    मानम्_2 = dec_Digit_मानम्(*p);
    जइ (मानम्_2 >= 10U)
      प्रत्यागम createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + मानम्_2;
    जइ (unsignedExponent > 32767) {
      overflow = सच्च;
      भङ्ग;
    }
  }

  जइ (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    overflow = सच्च;

  जइ (!overflow) {
    exponent = unsignedExponent;
    जइ (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    जइ (exponent > 32767 || exponent < -32768)
      overflow = सच्च;
  }

  जइ (overflow)
    exponent = negative ? -32768: 32767;

  प्रत्यागम exponent;
}

स्थिर Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator end,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = end;
  जाव (p != end && *p == '0')
    p++;

  जइ (p != end && *p == '.') {
    *dot = p++;

    जइ (end - begin == 1)
      प्रत्यागम createError("Significand has no digits");

    जाव (p != end && *p == '0')
      p++;
  }

  प्रत्यागम p;
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
  जइ (!PtrOrErr)
    प्रत्यागम PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->firstSigDigit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  प्रति (; p != end; ++p) {
    जइ (*p == '.') {
      जइ (dot != end)
        प्रत्यागम createError("String contains multiple dots");
      dot = p++;
      जइ (p == end)
        भङ्ग;
    }
    जइ (dec_Digit_मानम्(*p) >= 10U)
      भङ्ग;
  }

  जइ (p != end) {
    जइ (*p != 'e' && *p != 'E')
      प्रत्यागम createError("Invalid character in significand");
    जइ (p == begin)
      प्रत्यागम createError("Significand has no digits");
    जइ (dot != end && p - begin == 1)
      प्रत्यागम createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    auto ExpOrErr = readExponent(p + 1, end);
    जइ (!ExpOrErr)
      प्रत्यागम ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    जइ (dot == end)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  जइ (p != D->firstSigDigit) {
    /* Drop insignificant trailing zeroes.  */
    जइ (p != begin) {
      करेहि
        करेहि
          p--;
        जाव (p != begin && *p == '0');
      जाव (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->firstSigDigit)
                                      - (dot > D->firstSigDigit && dot < p)));
  }

  D->lastSigDigit = p;
  प्रत्यागम Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
स्थिर Expected<lostFraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator end,
                            unsigned पूर्णाङ्क digit_मानम्) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  जइ (digit_मानम् > 8)
    प्रत्यागम lfMoreThanHalf;
  अण्णहा जइ (digit_मानम् < 8 && digit_मानम् > 0)
    प्रत्यागम lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  जाव (p != end && (*p == '0' || *p == '.'))
    p++;

  जइ (p == end)
    प्रत्यागम createError("Invalid trailing hexadecimal fraction!");

  unsigned hexDigit = hex_Digit_मानम्(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  जइ (hexDigit == UINT_MAX)
    प्रत्यागम digit_मानम् == 0 ? lfExactlyZero: lfExactlyHalf;
  अण्णहा
    प्रत्यागम digit_मानम् == 0 ? lfLessThanHalf: lfMoreThanHalf;
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
  जइ (bits <= lsb)
    प्रत्यागम lfExactlyZero;
  जइ (bits == lsb + 1)
    प्रत्यागम lfExactlyHalf;
  जइ (bits <= part_गणनम् * APFloatBase::integerPartWidth &&
      APInt::tcExtractBit(parts, bits - 1))
    प्रत्यागम lfMoreThanHalf;

  प्रत्यागम lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
स्थिर lostFraction
shift_दक्षिणः(APFloatBase::integerPart *dst, unsigned पूर्णाङ्क parts, unsigned पूर्णाङ्क bits)
{
  lostFraction lost_fraction = lostFractionThroughTruncation(dst, parts, bits);

  APInt::tc_Shift_दक्षिणः(dst, parts, bits);

  प्रत्यागम lost_fraction;
}

/* Combine the effect of two lost fractions.  */
स्थिर lostFraction
combineLostFractions(lostFraction moreSignificant,
                     lostFraction lessSignificant)
{
  जइ (lessSignificant != lfExactlyZero) {
    जइ (moreSignificant == lfExactlyZero)
      moreSignificant = lfLessThanHalf;
    अण्णहा जइ (moreSignificant == lfExactlyHalf)
      moreSignificant = lfMoreThanHalf;
  }

  प्रत्यागम moreSignificant;
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

  जइ (HUerr1 + HUerr2 == 0)
    प्रत्यागम inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  अण्णहा
    प्रत्यागम inexactMultiply + 2 * (HUerr1 + HUerr2);
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
  जइ (isNearest)
    boundary = (APFloatBase::integerPart) 1 << (partBits - 1);
  अण्णहा
    boundary = 0;

  जइ (गणनम् == 0) {
    जइ (part - boundary <= boundary - part)
      प्रत्यागम part - boundary;
    अण्णहा
      प्रत्यागम boundary - part;
  }

  जइ (part == boundary) {
    जाव (--गणनम्)
      जइ (parts[गणनम्])
        प्रत्यागम ~(APFloatBase::integerPart) 0; /* A lot.  */

    प्रत्यागम parts[0];
  } अण्णहा जइ (part == boundary - 1) {
    जाव (--गणनम्)
      जइ (~parts[गणनम्])
        प्रत्यागम ~(APFloatBase::integerPart) 0; /* A lot.  */

    प्रत्यागम -parts[0];
  }

  प्रत्यागम ~(APFloatBase::integerPart) 0; /* A lot.  */
}

स्थिर constexpr unsigned PowerOfFivePartCounts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

स्थिर constexpr APFloatBase::integerPart PowerOfFiveParts[] = {
#अन्तर्भाव "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  unsigned Sum = 0;
  प्रति (unsigned गणनम्_2 : PowerOfFivePartCounts)
    Sum += गणनम्_2;
  प्रत्यागम Sum;
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

  प्रति (unsigned पूर्णाङ्क n = 0; power; power >>= 1, n++) {
    unsigned parts_गणनम् = PowerOfFivePartCounts[n];

    जइ (power & 1) {
      APFloatBase::integerPart *tmp;

      APInt::tcFullMultiply(p2, p1, pow5, result, parts_गणनम्);
      result += parts_गणनम्;
      जइ (p2[result - 1] == 0)
        result--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_गणनम्;
  }

  जइ (p1 != dst)
    APInt::tcAssign(dst, p1, result);

  प्रत्यागम result;
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
  जाव (गणनम्--) {
    dst[गणनम्] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  प्रत्यागम result;
}

/* Write out an unsigned decimal integer.  */
स्थिर वर्ण *writeUnsignedDecimal(वर्ण *dst, unsigned पूर्णाङ्क n) {
  वर्ण buff[40], *p;

  p = buff;
  करेहि
    *p++ = '0' + n % 10;
  जाव (n /= 10);

  करेहि
    *dst++ = *--p;
  जाव (p != buff);

  प्रत्यागम dst;
}

/* Write out a signed decimal integer.  */
स्थिर वर्ण *writeSignedDecimal(वर्ण *dst, पूर्णाङ्क मानम्_2) {
  जइ (मानम्_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(unsigned) मानम्_2);
  } अण्णहा {
    dst = writeUnsignedDecimal(dst, मानम्_2);
  }

  प्रत्यागम dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
स्थिर APFloat harrisonUlp(ध्रुव APFloat &X) {
  ध्रुव fltSemantics &Sem = X.getSemantics();
  विकल्प (X.getCategory()) {
  पक्ष APFloat::fcNaN:
    प्रत्यागम APFloat::getQNaN(Sem);
  पक्ष APFloat::fcInfinity:
    प्रत्यागम APFloat::getInf(Sem);
  पक्ष APFloat::fcZero:
    प्रत्यागम APFloat::getSmallest(Sem);
  पक्ष APFloat::fcNormal:
    भङ्ग;
  }
  जइ (X.isDenormal() || X.isSmallestNormalized())
    प्रत्यागम APFloat::getSmallest(Sem);
  पूर्णाङ्क Exp = ilogb(X);
  जइ (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  प्रत्यागम scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rmNearestTiesToEven);
}

नामाकाश detail {
/* Constructors.  */
शून्य IEEEFloat::initialize(ध्रुव fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  unsigned गणनम् = part_गणनम्();
  जइ (गणनम् > 1)
    significand.parts = नूतन integerPart[गणनम्];
}

शून्य IEEEFloat::freeSignificand() {
  जइ (needsCleanup())
    delete [] significand.parts;
}

शून्य IEEEFloat::assign(ध्रुव IEEEFloat &rhs) {
  प्रतिज्ञा(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  जइ (isFiniteNonZero() || category == fcNaN)
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
  जइ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  जइ (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  unsigned अङ्कः_Parts = part_गणनम्();

  APInt fill_storage;
  जइ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = मिच्छ;
    जइ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = सच्च;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } अण्णहा {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  जइ (!fill || fill->get_अङ्कः_शब्दाः() < अङ्कः_Parts)
    APInt::tcSet(significand, 0, अङ्कः_Parts);
  जइ (fill) {
    APInt::tcAssign(significand, fill->getRawData(),
                    std::min(fill->get_अङ्कः_शब्दाः(), अङ्कः_Parts));

    // Zero out the excess bits of the significand.
    unsigned bitsToPreserve = semantics->precision - 1;
    unsigned part = bitsToPreserve / 64;
    bitsToPreserve %= 64;
    significand[part] &= ((1ULL << bitsToPreserve) - 1);
    प्रति (part++; part != अङ्कः_Parts; ++part)
      significand[part] = 0;
  }

  unsigned QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  जइ (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tcClearBit(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    जइ (APInt::tcIsZero(significand, अङ्कः_Parts))
      APInt::tcSetBit(significand, QNaNBit - 1);
  } अण्णहा जइ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } अण्णहा {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tcSetBit(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  जइ (semantics == &APFloatBase::semX87DoubleExtended)
    APInt::tcSetBit(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::operator=(ध्रुव IEEEFloat &rhs) {
  जइ (एतत् != &rhs) {
    जइ (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    assign(rhs);
  }

  प्रत्यागम *एतत्;
}

IEEEFloat &IEEEFloat::operator=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloatBase::semBogus;
  प्रत्यागम *एतत्;
}

तार्किक IEEEFloat::isDenormal() ध्रुव {
  प्रत्यागम getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tcExtractBit(significandParts(), semantics->precision - 1) ==
          0);
}

तार्किक IEEEFloat::isSmallest() ध्रुव {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  प्रत्यागम isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

तार्किक IEEEFloat::isSmallestNormalized() ध्रुव {
  प्रत्यागम getCategory() == fcNormal && exponent == semantics->minExponent &&
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
  प्रत्यागम अङ्कः_उच्चः_Bits;
}

तार्किक IEEEFloat::isSignificandAllOnes() ध्रुव {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  ध्रुव integerPart *Parts = significandParts();
  ध्रुव unsigned Part_गणनम् = part_गणनम्_For_Bits(semantics->precision);
  प्रति (unsigned i = 0; i < Part_गणनम् - 1; i++)
    जइ (~Parts[i])
      प्रत्यागम मिच्छ;

  // Set the unused high bits to all ones when we compare.
  ध्रुव unsigned अङ्कः_उच्चः_Bits = get_अङ्कः_उच्चः_Bits();
  प्रतिज्ञा(अङ्कः_उच्चः_Bits <= integerPartWidth && अङ्कः_उच्चः_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ध्रुव integerPart उच्चः_Bit_Fill =
    ~integerPart(0) << (integerPartWidth - अङ्कः_उच्चः_Bits);
  जइ ((semantics->precision <= 1) || (~(Parts[Part_गणनम् - 1] | उच्चः_Bit_Fill)))
    प्रत्यागम मिच्छ;

  प्रत्यागम सच्च;
}

तार्किक IEEEFloat::isSignificandAllOnesExceptLSB() ध्रुव {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  ध्रुव integerPart *Parts = significandParts();

  जइ (Parts[0] & 1)
    प्रत्यागम मिच्छ;

  ध्रुव unsigned Part_गणनम् = part_गणनम्_For_Bits(semantics->precision);
  प्रति (unsigned i = 0; i < Part_गणनम् - 1; i++) {
    जइ (~Parts[i] & ~unsigned{!i})
      प्रत्यागम मिच्छ;
  }

  // Set the unused high bits to all ones when we compare.
  ध्रुव unsigned अङ्कः_उच्चः_Bits = get_अङ्कः_उच्चः_Bits();
  प्रतिज्ञा(अङ्कः_उच्चः_Bits <= integerPartWidth && अङ्कः_उच्चः_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ध्रुव integerPart उच्चः_Bit_Fill = ~integerPart(0)
                                  << (integerPartWidth - अङ्कः_उच्चः_Bits);
  जइ (~(Parts[Part_गणनम् - 1] | उच्चः_Bit_Fill | 0x1))
    प्रत्यागम मिच्छ;

  प्रत्यागम सच्च;
}

तार्किक IEEEFloat::isSignificandAllZeros() ध्रुव {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  ध्रुव integerPart *Parts = significandParts();
  ध्रुव unsigned Part_गणनम् = part_गणनम्_For_Bits(semantics->precision);

  प्रति (unsigned i = 0; i < Part_गणनम् - 1; i++)
    जइ (Parts[i])
      प्रत्यागम मिच्छ;

  // Compute how many bits are used in the final word.
  ध्रुव unsigned अङ्कः_उच्चः_Bits = get_अङ्कः_उच्चः_Bits();
  प्रतिज्ञा(अङ्कः_उच्चः_Bits < integerPartWidth && "Can not have more high bits to "
         "clear than integerPartWidth");
  ध्रुव integerPart उच्चः_Bit_Mask = ~integerPart(0) >> अङ्कः_उच्चः_Bits;

  जइ ((semantics->precision > 1) && (Parts[Part_गणनम् - 1] & उच्चः_Bit_Mask))
    प्रत्यागम मिच्छ;

  प्रत्यागम सच्च;
}

तार्किक IEEEFloat::isSignificandAllZerosExceptMSB() ध्रुव {
  ध्रुव integerPart *Parts = significandParts();
  ध्रुव unsigned Part_गणनम् = part_गणनम्_For_Bits(semantics->precision);

  प्रति (unsigned i = 0; i < Part_गणनम् - 1; i++) {
    जइ (Parts[i])
      प्रत्यागम मिच्छ;
  }

  ध्रुव unsigned अङ्कः_उच्चः_Bits = get_अङ्कः_उच्चः_Bits();
  ध्रुव integerPart MSBMask = integerPart(1)
                              << (integerPartWidth - अङ्कः_उच्चः_Bits);
  प्रत्यागम ((semantics->precision <= 1) || (Parts[Part_गणनम् - 1] == MSBMask));
}

तार्किक IEEEFloat::isLargest() ध्रुव {
  तार्किक IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  जइ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    प्रत्यागम (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } अण्णहा {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    प्रत्यागम IsMaxExp && isSignificandAllOnes();
  }
}

तार्किक IEEEFloat::isInteger() ध्रुव {
  // This could be made more efficient; I'm going for obviously correct.
  जइ (!isFinite()) प्रत्यागम मिच्छ;
  IEEEFloat truncated = *एतत्;
  truncated.roundToIntegral(rmTowardZero);
  प्रत्यागम compare(truncated) == cmpEqual;
}

तार्किक IEEEFloat::bitwiseIsEqual(ध्रुव IEEEFloat &rhs) ध्रुव {
  जइ (एतत् == &rhs)
    प्रत्यागम सच्च;
  जइ (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    प्रत्यागम मिच्छ;
  जइ (category==fcZero || category==fcInfinity)
    प्रत्यागम सच्च;

  जइ (isFiniteNonZero() && exponent != rhs.exponent)
    प्रत्यागम मिच्छ;

  प्रत्यागम std::equal(significandParts(), significandParts() + part_गणनम्(),
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
  ourSemantics.hasZero ? makeZero(मिच्छ) : makeSmallestNormalized(मिच्छ);
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
  प्रत्यागम part_गणनम्_For_Bits(semantics->precision + 1);
}

ध्रुव APFloat::integerPart *IEEEFloat::significandParts() ध्रुव {
  प्रत्यागम const_cast<IEEEFloat *>(एतत्)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  जइ (part_गणनम्() > 1)
    प्रत्यागम significand.parts;
  अण्णहा
    प्रत्यागम &significand.part;
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

  प्रत्यागम APInt::tcAdd(parts, rhs.significandParts(), 0, part_गणनम्());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::subtractSignificand(ध्रुव IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  प्रतिज्ञा(semantics == rhs.semantics);
  प्रतिज्ञा(exponent == rhs.exponent);

  प्रत्यागम APInt::tcSubtract(parts, rhs.significandParts(), borrow,
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

  जइ (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    ध्रुव fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    unsigned extendedPrecision = 2 * precision + 1;
    जइ (omsb != extendedPrecision - 1) {
      प्रतिज्ञा(extendedPrecision > omsb);
      APInt::tc_Shift_वामः(fullSignificand, new_Parts_गणनम्,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    जइ (new_Parts_गणनम् == 1)
      significand.part = fullSignificand[0];
    अण्णहा
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

    lost_fraction = addOrSubtractSignificand(extendedAddend, मिच्छ);

    /* Restore our state.  */
    जइ (new_Parts_गणनम् == 1)
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
  जइ (omsb > precision) {
    unsigned पूर्णाङ्क bits, significantParts;
    lostFraction lf;

    bits = omsb - precision;
    significantParts = part_गणनम्_For_Bits(omsb);
    lf = shift_दक्षिणः(fullSignificand, significantParts, bits);
    lost_fraction = combineLostFractions(lf, lost_fraction);
    exponent += bits;
  }

  APInt::tcAssign(lhsSignificand, fullSignificand, parts_गणनम्);

  जइ (new_Parts_गणनम् > 4)
    delete [] fullSignificand;

  प्रत्यागम lost_fraction;
}

lostFraction IEEEFloat::multiplySignificand(ध्रुव IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  प्रत्यागम multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
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
  प्रति (unsigned i = 0; i < parts_गणनम्; i++) {
    dividend[i] = lhsSignificand[i];
    भाजकः[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  unsigned पूर्णाङ्क precision = semantics->precision;

  /* Normalize the divisor.  */
  unsigned bit = precision - APInt::tcMSB(भाजकः, parts_गणनम्) - 1;
  जइ (bit) {
    exponent += bit;
    APInt::tc_Shift_वामः(भाजकः, parts_गणनम्, bit);
  }

  /* Normalize the dividend.  */
  bit = precision - APInt::tcMSB(dividend, parts_गणनम्) - 1;
  जइ (bit) {
    exponent -= bit;
    APInt::tc_Shift_वामः(dividend, parts_गणनम्, bit);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  जइ (APInt::tcCompare(dividend, भाजकः, parts_गणनम्) < 0) {
    exponent--;
    APInt::tc_Shift_वामः(dividend, parts_गणनम्, 1);
    प्रतिज्ञा(APInt::tcCompare(dividend, भाजकः, parts_गणनम्) >= 0);
  }

  /* Long division.  */
  प्रति (bit = precision; bit; bit -= 1) {
    जइ (APInt::tcCompare(dividend, भाजकः, parts_गणनम्) >= 0) {
      APInt::tcSubtract(dividend, भाजकः, 0, parts_गणनम्);
      APInt::tcSetBit(lhsSignificand, bit - 1);
    }

    APInt::tc_Shift_वामः(dividend, parts_गणनम्, 1);
  }

  /* Figure out the lost fraction.  */
  पूर्णाङ्क cmp = APInt::tcCompare(dividend, भाजकः, parts_गणनम्);

  lostFraction lost_fraction;
  जइ (cmp > 0)
    lost_fraction = lfMoreThanHalf;
  अण्णहा जइ (cmp == 0)
    lost_fraction = lfExactlyHalf;
  अण्णहा जइ (APInt::tcIsZero(dividend, parts_गणनम्))
    lost_fraction = lfExactlyZero;
  अण्णहा
    lost_fraction = lfLessThanHalf;

  जइ (parts_गणनम् > 2)
    delete [] dividend;

  प्रत्यागम lost_fraction;
}

unsigned पूर्णाङ्क IEEEFloat::significandMSB() ध्रुव {
  प्रत्यागम APInt::tcMSB(significandParts(), part_गणनम्());
}

unsigned पूर्णाङ्क IEEEFloat::significandLSB() ध्रुव {
  प्रत्यागम APInt::tcLSB(significandParts(), part_गणनम्());
}

/* Note that a zero result is NOT normalized to fcZero.  */
lostFraction IEEEFloat::shift_Significand_दक्षिणः(unsigned पूर्णाङ्क bits) {
  /* Our exponent should not overflow.  */
  प्रतिज्ञा((ExponentType) (exponent + bits) >= exponent);

  exponent += bits;

  प्रत्यागम shift_दक्षिणः(significandParts(), part_गणनम्(), bits);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
शून्य IEEEFloat::shift_Significand_वामः(unsigned पूर्णाङ्क bits) {
  प्रतिज्ञा(bits < semantics->precision ||
         (semantics->precision == 1 && bits <= 1));

  जइ (bits) {
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
  जइ (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_गणनम्());

  जइ (compare > 0)
    प्रत्यागम cmpGreaterThan;
  अण्णहा जइ (compare < 0)
    प्रत्यागम cmpLessThan;
  अण्णहा
    प्रत्यागम cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
स्थिर शून्य tcSetLeastSignificantBits(APInt::WordType *dst, unsigned parts,
                                      unsigned bits) {
  unsigned i = 0;
  जाव (bits > APInt::APINT_BITS_PER_WORD) {
    dst[i++] = ~(APInt::WordType)0;
    bits -= APInt::APINT_BITS_PER_WORD;
  }

  जइ (bits)
    dst[i++] = ~(APInt::WordType)0 >> (APInt::APINT_BITS_PER_WORD - bits);

  जाव (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::opStatus IEEEFloat::handleOverflow(roundingMode rounding_mode) {
  जइ (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    जइ (rounding_mode == rmNearestTiesToEven ||
        rounding_mode == rmNearestTiesToAway ||
        (rounding_mode == rmTowardPositive && !sign) ||
        (rounding_mode == rmTowardNegative && sign)) {
      जइ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(मिच्छ, sign);
      अण्णहा
        category = fcInfinity;
      प्रत्यागम static_cast<opStatus>(opOverflow | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fcNormal;
  exponent = semantics->maxExponent;
  tcSetLeastSignificantBits(significandParts(), part_गणनम्(),
                            semantics->precision);
  जइ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tcClearBit(significandParts(), 0);

  प्रत्यागम opInexact;
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
    प्रत्यागम lost_fraction == lfExactlyHalf || lost_fraction == lfMoreThanHalf;

  पक्ष rmNearestTiesToEven:
    जइ (lost_fraction == lfMoreThanHalf)
      प्रत्यागम सच्च;

    /* Our zeroes don't have a significand to test.  */
    जइ (lost_fraction == lfExactlyHalf && category != fcZero)
      प्रत्यागम APInt::tcExtractBit(significandParts(), bit);

    प्रत्यागम मिच्छ;

  पक्ष rmTowardZero:
    प्रत्यागम मिच्छ;

  पक्ष rmTowardPositive:
    प्रत्यागम !sign;

  पक्ष rmTowardNegative:
    प्रत्यागम sign;

  स्वभाव:
    भङ्ग;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::opStatus IEEEFloat::normalize(roundingMode rounding_mode,
                                       lostFraction lost_fraction) {
  जइ (!isFiniteNonZero())
    प्रत्यागम opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  unsigned omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  जइ (omsb || lost_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    पूर्णाङ्क exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    जइ (exponent + exponentChange > semantics->maxExponent)
      प्रत्यागम handleOverflow(rounding_mode);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    जइ (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    जइ (exponentChange < 0) {
      प्रतिज्ञा(lost_fraction == lfExactlyZero);

      shift_Significand_वामः(-exponentChange);

      प्रत्यागम opOK;
    }

    जइ (exponentChange > 0) {
      lostFraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = shift_Significand_दक्षिणः(exponentChange);

      lost_fraction = combineLostFractions(lf, lost_fraction);

      /* Keep OMSB up-to-date.  */
      जइ (omsb > (unsigned) exponentChange)
        omsb -= exponentChange;
      अण्णहा
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  जइ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    प्रत्यागम handleOverflow(rounding_mode);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  जइ (lost_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    जइ (omsb == 0) {
      category = fcZero;
      जइ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = मिच्छ;
      जइ (!semantics->hasZero)
        makeSmallestNormalized(मिच्छ);
    }

    प्रत्यागम opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  जइ (roundAwayFromZero(rounding_mode, lost_fraction, 0)) {
    जइ (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    जइ (omsb == (unsigned) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      जइ (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        प्रत्यागम handleOverflow(sign ? rmTowardNegative : rmTowardPositive);

      shift_Significand_दक्षिणः(1);

      प्रत्यागम opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    जइ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      प्रत्यागम handleOverflow(rounding_mode);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  जइ (omsb == semantics->precision)
    प्रत्यागम opInexact;

  /* We have a non-zero denormal.  */
  प्रतिज्ञा(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  जइ (omsb == 0) {
    category = fcZero;
    जइ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = मिच्छ;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    जइ (!semantics->hasZero)
      makeSmallestNormalized(मिच्छ);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  प्रत्यागम (opStatus) (opUnderflow | opInexact);
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
    जइ (isSignaling()) {
      makeQuiet();
      प्रत्यागम opInvalidOp;
    }
    प्रत्यागम rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ subtract;
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNormal):
    assign(rhs);
    sign = rhs.sign ^ subtract;
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    जइ (((sign ^ rhs.sign)!=0) != subtract) {
      makeNaN();
      प्रत्यागम opInvalidOp;
    }

    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNormal):
    प्रत्यागम opDivByZero;
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
  जइ (subtract) {
    जइ ((bits < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat temp_rhs(rhs);
    तार्किक lost_fraction_is_from_rhs = मिच्छ;

    जइ (bits == 0)
      lost_fraction = lfExactlyZero;
    अण्णहा जइ (bits > 0) {
      lost_fraction = temp_rhs.shift_Significand_दक्षिणः(bits - 1);
      lost_fraction_is_from_rhs = सच्च;
      shift_Significand_वामः(1);
    } अण्णहा {
      lost_fraction = shift_Significand_दक्षिणः(-bits - 1);
      temp_rhs.shift_Significand_वामः(1);
    }

    // Should we reverse the subtraction.
    cmpResult cmp_result = compare_Absolute_मानम्(temp_rhs);
    जइ (cmp_result == cmpLessThan) {
      तार्किक borrow =
          lost_fraction != lfExactlyZero && !lost_fraction_is_from_rhs;
      जइ (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        जइ (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        अण्णहा जइ (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = temp_rhs.subtractSignificand(*एतत्, borrow);
      copySignificand(temp_rhs);
      sign = !sign;
    } अण्णहा जइ (cmp_result == cmpGreaterThan) {
      तार्किक borrow = lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs;
      जइ (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        जइ (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        अण्णहा जइ (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = subtractSignificand(temp_rhs, borrow);
    } अण्णहा { // cmpEqual
      zeroSignificand();
      जइ (lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    प्रतिज्ञा(!carry);
  } अण्णहा {
    जइ (bits > 0) {
      IEEEFloat temp_rhs(rhs);

      lost_fraction = temp_rhs.shift_Significand_दक्षिणः(bits);
      carry = addSignificand(temp_rhs);
    } अण्णहा {
      lost_fraction = shift_Significand_दक्षिणः(-bits);
      carry = addSignificand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    प्रतिज्ञा(!carry);
  }

  प्रत्यागम lost_fraction;
}

APFloat::opStatus IEEEFloat::multiplySpecials(ध्रुव IEEEFloat &rhs) {
  विकल्प (Pack_Categories_Into_कुञ्चिका(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(nullptr);

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNaN):
    assign(rhs);
    sign = मिच्छ;
    [[fallthrough]];
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    जइ (isSignaling()) {
      makeQuiet();
      प्रत्यागम opInvalidOp;
    }
    प्रत्यागम rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
    category = fcInfinity;
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    category = fcZero;
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
    makeNaN();
    प्रत्यागम opInvalidOp;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNormal):
    प्रत्यागम opOK;
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
    sign = मिच्छ;
    [[fallthrough]];
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    जइ (isSignaling()) {
      makeQuiet();
      प्रत्यागम opInvalidOp;
    }
    प्रत्यागम rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNormal):
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcInfinity):
    category = fcZero;
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcZero):
    जइ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(मिच्छ, sign);
    अण्णहा
      category = fcInfinity;
    प्रत्यागम opDivByZero;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    makeNaN();
    प्रत्यागम opInvalidOp;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNormal):
    प्रत्यागम opOK;
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
    जइ (isSignaling()) {
      makeQuiet();
      प्रत्यागम opInvalidOp;
    }
    प्रत्यागम rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcInfinity):
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    makeNaN();
    प्रत्यागम opInvalidOp;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNormal):
    प्रत्यागम opOK;
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
    जइ (isSignaling()) {
      makeQuiet();
      प्रत्यागम opInvalidOp;
    }
    प्रत्यागम rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcInfinity):
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    makeNaN();
    प्रत्यागम opInvalidOp;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNormal):
    प्रत्यागम opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
शून्य IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  जइ (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    प्रत्यागम;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::opStatus IEEEFloat::addOrSubtract(ध्रुव IEEEFloat &rhs,
                                           roundingMode rounding_mode,
                                           तार्किक subtract) {
  opStatus fs = addOrSubtractSpecials(rhs, subtract);

  /* This return code means it was not a simple case.  */
  जइ (fs == opDivByZero) {
    lostFraction lost_fraction;

    lost_fraction = addOrSubtractSignificand(rhs, subtract);
    fs = normalize(rounding_mode, lost_fraction);

    /* Can only be zero if we lost no fraction.  */
    प्रतिज्ञा(category != fcZero || lost_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  जइ (category == fcZero) {
    जइ (rhs.category != fcZero || (sign == rhs.sign) == subtract)
      sign = (rounding_mode == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    जइ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = मिच्छ;
  }

  प्रत्यागम fs;
}

/* Normalized addition.  */
APFloat::opStatus IEEEFloat::add(ध्रुव IEEEFloat &rhs,
                                 roundingMode rounding_mode) {
  प्रत्यागम addOrSubtract(rhs, rounding_mode, मिच्छ);
}

/* Normalized subtraction.  */
APFloat::opStatus IEEEFloat::subtract(ध्रुव IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  प्रत्यागम addOrSubtract(rhs, rounding_mode, सच्च);
}

/* Normalized multiply.  */
APFloat::opStatus IEEEFloat::multiply(ध्रुव IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = multiplySpecials(rhs);

  जइ (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = मिच्छ;
  जइ (isFiniteNonZero()) {
    lostFraction lost_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    जइ (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  प्रत्यागम fs;
}

/* Normalized divide.  */
APFloat::opStatus IEEEFloat::divide(ध्रुव IEEEFloat &rhs,
                                    roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = divideSpecials(rhs);

  जइ (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = मिच्छ;
  जइ (isFiniteNonZero()) {
    lostFraction lost_fraction = divideSignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    जइ (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  प्रत्यागम fs;
}

/* Normalized remainder.  */
APFloat::opStatus IEEEFloat::remainder(ध्रुव IEEEFloat &rhs) {
  unsigned पूर्णाङ्क origSign = sign;

  // First handle the special cases.
  opStatus fs = remainderSpecials(rhs);
  जइ (fs != opDivByZero)
    प्रत्यागम fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  जइ (P2.add(rhs, rmNearestTiesToEven) == opOK) {
    fs = mod(P2);
    प्रतिज्ञा(fs == opOK);
  }

  // Lets work with absolute numbers.
  IEEEFloat P = rhs;
  P.sign = मिच्छ;
  sign = मिच्छ;

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

  जइ (VEx.compare(PEx) == cmpGreaterThan) {
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
    जइ (result == cmpGreaterThan || result == cmpEqual) {
      fs = subtract(P, rmNearestTiesToEven);
      प्रतिज्ञा(fs == opOK);
    }
  }

  जइ (isZero()) {
    sign = origSign;    // IEEE754 requires this
    जइ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = मिच्छ;
  } अण्णहा {
    sign ^= origSign;
  }
  प्रत्यागम fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::opStatus IEEEFloat::mod(ध्रुव IEEEFloat &rhs) {
  opStatus fs = modSpecials(rhs);
  unsigned पूर्णाङ्क origSign = sign;

  जाव (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_मानम्(rhs) != cmpLessThan) {
    पूर्णाङ्क Exp = ilogb(*एतत्) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rmNearestTiesToEven);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    जइ (V.isNaN() || compare_Absolute_मानम्(V) == cmpLessThan)
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
    जइ (!semantics->hasZero && एतत्->isSmallest())
      भङ्ग;

    प्रतिज्ञा(fs==opOK);
  }
  जइ (isZero()) {
    sign = origSign; // fmod requires this
    जइ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = मिच्छ;
  }
  प्रत्यागम fs;
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
  जइ (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    lostFraction lost_fraction;

    lost_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_mode, lost_fraction);
    जइ (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    जइ (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_mode == rmTowardNegative);
      जइ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = मिच्छ;
    }
  } अण्णहा {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    जइ (fs == opOK)
      fs = addOrSubtract(addend, rounding_mode, मिच्छ);
  }

  प्रत्यागम fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::opStatus IEEEFloat::roundToIntegral(roundingMode rounding_mode) {
  जइ (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    प्रत्यागम opOK;

  जइ (isNaN()) {
    जइ (isSignaling()) {
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
      प्रत्यागम opInvalidOp;
    } अण्णहा {
      // [IEEE Std 754-2008 6.2]:
      // For an operation with quiet NaN inputs, other than maximum and minimum
      // operations, if a floating-point result is to be delivered the result
      // shall be a quiet NaN which should be one of the input NaNs.
      // ...
      // Every general-computational and quiet-computational operation involving
      // one or more input NaNs, none of them signaling, shall signal no
      // exception, except fusedMultiplyAdd might signal the invalid operation
      // exception(see 7.2).
      प्रत्यागम opOK;
    }
  }

  जइ (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    प्रत्यागम opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  जइ (exponent + 1 >= (पूर्णाङ्क)APFloat::semanticsPrecision(*semantics))
    प्रत्यागम opOK;

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
  opStatus fs = MagicConstant.convertFromAPInt(IntegerConstant, मिच्छ,
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
  जइ (निवेशः_Sign != isNegative())
    changeSign();

  प्रत्यागम fs;
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
    प्रत्यागम cmpUnordered;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcZero):
    जइ (sign)
      प्रत्यागम cmpLessThan;
    अण्णहा
      प्रत्यागम cmpGreaterThan;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNormal):
    जइ (rhs.sign)
      प्रत्यागम cmpGreaterThan;
    अण्णहा
      प्रत्यागम cmpLessThan;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
    जइ (sign == rhs.sign)
      प्रत्यागम cmpEqual;
    अण्णहा जइ (sign)
      प्रत्यागम cmpLessThan;
    अण्णहा
      प्रत्यागम cmpGreaterThan;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    प्रत्यागम cmpEqual;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNormal, fcNormal):
    भङ्ग;
  }

  cmpResult result;
  /* Two normal numbers.  Do they have the same sign?  */
  जइ (sign != rhs.sign) {
    जइ (sign)
      result = cmpLessThan;
    अण्णहा
      result = cmpGreaterThan;
  } अण्णहा {
    /* Compare absolute values; invert result if negative.  */
    result = compare_Absolute_मानम्(rhs);

    जइ (sign) {
      जइ (result == cmpLessThan)
        result = cmpGreaterThan;
      अण्णहा जइ (result == cmpGreaterThan)
        result = cmpLessThan;
    }
  }

  प्रत्यागम result;
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

  तार्किक X86SpecialNan = मिच्छ;
  जइ (&fromSemantics == &APFloatBase::semX87DoubleExtended &&
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
  जइ (shift < 0 && isFiniteNonZero()) {
    पूर्णाङ्क omsb = significandMSB() + 1;
    पूर्णाङ्क exponentChange = omsb - fromSemantics.precision;
    जइ (exponent + exponentChange < toSemantics.minExponent)
      exponentChange = toSemantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, shift);
    जइ (exponentChange < 0) {
      shift -= exponentChange;
      exponent += exponentChange;
    } अण्णहा जइ (omsb <= -shift) {
      exponentChange = omsb + shift - 1; // leave at least one bit set
      shift -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  जइ (shift < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    lostFraction = shift_दक्षिणः(significandParts(), old_Part_गणनम्, -shift);

  // Fix the storage so it can hold to new value.
  जइ (new_Part_गणनम् > old_Part_गणनम्) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = नूतन integerPart[new_Part_गणनम्];
    APInt::tcSet(newParts, 0, new_Part_गणनम्);
    जइ (isFiniteNonZero() || category==fcNaN)
      APInt::tcAssign(newParts, significandParts(), old_Part_गणनम्);
    freeSignificand();
    significand.parts = newParts;
  } अण्णहा जइ (new_Part_गणनम् == 1 && old_Part_गणनम् != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    जइ (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &toSemantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  जइ (shift > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_Shift_वामः(significandParts(), new_Part_गणनम्, shift);

  जइ (isFiniteNonZero()) {
    fs = normalize(rounding_mode, lostFraction);
    *losesInfo = (fs != opOK);
  } अण्णहा जइ (category == fcNaN) {
    जइ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(मिच्छ, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } अण्णहा {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      जइ (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(मिच्छ, मिच्छ);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      जइ (!APFloat::hasSignificand(fromSemantics))
        makeNaN(मिच्छ, sign);

      *losesInfo = lostFraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      जइ (!X86SpecialNan && semantics == &APFloatBase::semX87DoubleExtended)
        APInt::tcSetBit(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      जइ (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } अण्णहा {
        fs = opOK;
      }
    }
  } अण्णहा जइ (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(मिच्छ, sign);
    *losesInfo = सच्च;
    fs = opInexact;
  } अण्णहा जइ (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = मिच्छ;
  } अण्णहा {
    *losesInfo = मिच्छ;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  जइ ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = सच्च;
    जइ (fs == opOK)
      fs = opInexact;
  }

  जइ (category == fcZero && !semantics->hasZero)
    makeSmallestNormalized(मिच्छ);
  प्रत्यागम fs;
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
  *isExact = मिच्छ;

  /* Handle the three special cases first.  */
  जइ (category == fcInfinity || category == fcNaN)
    प्रत्यागम opInvalidOp;

  unsigned dst_Parts_गणनम् = part_गणनम्_For_Bits(width);
  प्रतिज्ञा(dst_Parts_गणनम् <= parts.size() && "Integer too big");

  जइ (category == fcZero) {
    APInt::tcSet(parts.data(), 0, dst_Parts_गणनम्);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    प्रत्यागम opOK;
  }

  ध्रुव integerPart *src = significandParts();

  unsigned truncatedBits;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  जइ (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tcSet(parts.data(), 0, dst_Parts_गणनम्);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncatedBits = semantics->precision -1U - exponent;
  } अण्णहा {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    unsigned पूर्णाङ्क bits = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    जइ (bits > width)
      प्रत्यागम opInvalidOp;

    जइ (bits < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncatedBits = semantics->precision - bits;
      APInt::tcExtract(parts.data(), dst_Parts_गणनम्, src, bits, truncatedBits);
    } अण्णहा {
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
  जइ (truncatedBits) {
    lost_fraction = lostFractionThroughTruncation(src, part_गणनम्(),
                                                  truncatedBits);
    जइ (lost_fraction != lfExactlyZero &&
        roundAwayFromZero(rounding_mode, lost_fraction, truncatedBits)) {
      जइ (APInt::tcIncrement(parts.data(), dst_Parts_गणनम्))
        प्रत्यागम opInvalidOp;     /* Overflow.  */
    }
  } अण्णहा {
    lost_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  unsigned पूर्णाङ्क omsb = APInt::tcMSB(parts.data(), dst_Parts_गणनम्) + 1;

  जइ (sign) {
    जइ (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      जइ (omsb != 0)
        प्रत्यागम opInvalidOp;
    } अण्णहा {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      जइ (omsb == width &&
          APInt::tcLSB(parts.data(), dst_Parts_गणनम्) + 1 != omsb)
        प्रत्यागम opInvalidOp;

      /* This case can happen because of rounding.  */
      जइ (omsb > width)
        प्रत्यागम opInvalidOp;
    }

    APInt::tcNegate (parts.data(), dst_Parts_गणनम्);
  } अण्णहा {
    जइ (omsb >= width + !isSigned)
      प्रत्यागम opInvalidOp;
  }

  जइ (lost_fraction == lfExactlyZero) {
    *isExact = सच्च;
    प्रत्यागम opOK;
  }
  प्रत्यागम opInexact;
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

  जइ (fs == opInvalidOp) {
    unsigned पूर्णाङ्क bits, dst_Parts_गणनम्;

    dst_Parts_गणनम् = part_गणनम्_For_Bits(width);
    प्रतिज्ञा(dst_Parts_गणनम् <= parts.size() && "Integer too big");

    जइ (category == fcNaN)
      bits = 0;
    अण्णहा जइ (sign)
      bits = isSigned;
    अण्णहा
      bits = width - isSigned;

    tcSetLeastSignificantBits(parts.data(), dst_Parts_गणनम्, bits);
    जइ (sign && isSigned)
      APInt::tc_Shift_वामः(parts.data(), dst_Parts_गणनम्, width - 1);
  }

  प्रत्यागम fs;
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
  जइ (precision <= omsb) {
    exponent = omsb - 1;
    lost_fraction = lostFractionThroughTruncation(src, src_गणनम्,
                                                  omsb - precision);
    APInt::tcExtract(dst, dst_गणनम्, src, precision, omsb - precision);
  } अण्णहा {
    exponent = precision - 1;
    lost_fraction = lfExactlyZero;
    APInt::tcExtract(dst, dst_गणनम्, src, omsb, 0);
  }

  प्रत्यागम normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus IEEEFloat::convertFromAPInt(ध्रुव APInt &Val, तार्किक isSigned,
                                              roundingMode rounding_mode) {
  unsigned पूर्णाङ्क part_गणनम् = Val.get_अङ्कः_शब्दाः();
  APInt api = Val;

  sign = मिच्छ;
  जइ (isSigned && api.isNegative()) {
    sign = सच्च;
    api = -api;
  }

  प्रत्यागम convertFromUnsignedParts(api.getRawData(), part_गणनम्, rounding_mode);
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
  तार्किक computedTrailingFraction = मिच्छ;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator end = s.end();
  StringRef::iterator dot;
  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  जइ (!PtrOrErr)
    प्रत्यागम PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator firstSignificantDigit = p;

  जाव (p != end) {
    integerPart hex_मानम्;

    जइ (*p == '.') {
      जइ (dot != end)
        प्रत्यागम createError("String contains multiple dots");
      dot = p++;
      अनुवर्त;
    }

    hex_मानम् = hex_Digit_मानम्(*p);
    जइ (hex_मानम् == UINT_MAX)
      भङ्ग;

    p++;

    // Store the number while we have space.
    जइ (bitPos) {
      bitPos -= 4;
      hex_मानम् <<= bitPos % integerPartWidth;
      significand[bitPos / integerPartWidth] |= hex_मानम्;
    } अण्णहा जइ (!computedTrailingFraction) {
      auto FractOrErr = trailingHexadecimalFraction(p, end, hex_मानम्);
      जइ (!FractOrErr)
        प्रत्यागम FractOrErr.takeError();
      lost_fraction = *FractOrErr;
      computedTrailingFraction = सच्च;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  जइ (p == end)
    प्रत्यागम createError("Hex strings require an exponent");
  जइ (*p != 'p' && *p != 'P')
    प्रत्यागम createError("Invalid character in significand");
  जइ (p == begin)
    प्रत्यागम createError("Significand has no digits");
  जइ (dot != end && p - begin == 1)
    प्रत्यागम createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  जइ (p != firstSignificantDigit) {
    पूर्णाङ्क expAdjustment;

    /* Implicit hexadecimal point?  */
    जइ (dot == end)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<पूर्णाङ्क>(dot - firstSignificantDigit);
    जइ (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_गणनम् * integerPartWidth;

    /* Adjust for the given exponent.  */
    auto ExpOrErr = totalExponent(p + 1, end, expAdjustment);
    जइ (!ExpOrErr)
      प्रत्यागम ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  प्रत्यागम normalize(rounding_mode, lost_fraction);
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

  प्रति (;; parts *= 2) {
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

    जइ (exp >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calcLostFraction = decSig.multiplySignificand(pow5);
      powHUerr = powStatus != opOK;
    } अण्णहा {
      calcLostFraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      जइ (decSig.exponent < semantics->minExponent) {
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
    जइ (HUdistance >= HUerr) {
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
      प्रत्यागम static_cast<opStatus>(normalize(rounding_mode, calcLostFraction) |
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
  जइ (Error Err = interpretDecimal(p, str.end(), &D))
    प्रत्यागम std::move(Err);

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
  जइ (D.firstSigDigit == str.end() || dec_Digit_मानम्(*D.firstSigDigit) >= 10U) {
    category = fcZero;
    fs = opOK;
    जइ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = मिच्छ;
    जइ (!semantics->hasZero)
      makeSmallestNormalized(मिच्छ);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } अण्णहा जइ (D.normalizedExponent - 1 > INT_MAX / 42039) {
    fs = handleOverflow(rounding_mode);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } अण्णहा जइ (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (पूर्णाङ्क) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fcNormal;
    zeroSignificand();
    fs = normalize(rounding_mode, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } अण्णहा जइ ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handleOverflow(rounding_mode);
  } अण्णहा {
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
    करेहि {
      integerPart dec_मानम्, val, multiplier;

      val = 0;
      multiplier = 1;

      करेहि {
        जइ (*p == '.') {
          p++;
          जइ (p == str.end()) {
            भङ्ग;
          }
        }
        dec_मानम् = dec_Digit_मानम्(*p++);
        जइ (dec_मानम् >= 10U) {
          delete[] decSignificand;
          प्रत्यागम createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_मानम्;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } जाव (p <= D.lastSigDigit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_गणनम्, part_गणनम् + 1, मिच्छ);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      जइ (decSignificand[part_गणनम्])
        part_गणनम्++;
    } जाव (p <= D.lastSigDigit);

    category = fcNormal;
    fs = roundSignificandWithExponent(decSignificand, part_गणनम्,
                                      D.exponent, rounding_mode);

    delete [] decSignificand;
  }

  प्रत्यागम fs;
}

तार्किक IEEEFloat::convertFromStringSpecials(StringRef str) {
  ध्रुव size_t MIN_NAME_SIZE = 3;

  जइ (str.size() < MIN_NAME_SIZE)
    प्रत्यागम मिच्छ;

  जइ (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(मिच्छ);
    प्रत्यागम सच्च;
  }

  तार्किक IsNegative = str.consume_front("-");
  जइ (IsNegative) {
    जइ (str.size() < MIN_NAME_SIZE)
      प्रत्यागम मिच्छ;

    जइ (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(सच्च);
      प्रत्यागम सच्च;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  तार्किक IsSignaling = str.consume_front_insensitive("s");
  जइ (IsSignaling) {
    जइ (str.size() < MIN_NAME_SIZE)
      प्रत्यागम मिच्छ;
  }

  जइ (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    जइ (str.empty()) {
      makeNaN(IsSignaling, IsNegative);
      प्रत्यागम सच्च;
    }

    // Allow the payload to be inside parentheses.
    जइ (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      जइ (str.size() <= 2 || str.back() != ')')
        प्रत्यागम मिच्छ;

      str = str.slice(1, str.size() - 1);
    }

    // Determine the payload number's radix.
    unsigned Radix = 10;
    जइ (str[0] == '0') {
      जइ (str.size() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } अण्णहा {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    जइ (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      प्रत्यागम सच्च;
    }
  }

  प्रत्यागम मिच्छ;
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromString(StringRef str, roundingMode rounding_mode) {
  जइ (str.empty())
    प्रत्यागम createError("Invalid string length");

  // Handle special cases.
  जइ (convertFromStringSpecials(str))
    प्रत्यागम opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  sign = *p == '-' ? 1 : 0;
  जइ (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  जइ (*p == '-' || *p == '+') {
    p++;
    slen--;
    जइ (!slen)
      प्रत्यागम createError("String has no digits");
  }

  जइ (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    जइ (slen == 2)
      प्रत्यागम createError("Invalid string");
    प्रत्यागम convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_mode);
  }

  प्रत्यागम convertFromDecimalString(StringRef(p, slen), rounding_mode);
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
  जइ (sign)
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
    जइ (hexDigits > 1) {
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

  प्रत्यागम static_cast<unsigned पूर्णाङ्क>(dst - p);
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

  तार्किक roundUp = मिच्छ;
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
  जइ (hexDigits) {
    जइ (hexDigits < outputDigits) {
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

  जाव (outputDigits && गणनम्) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    जइ (--गणनम् == parts_गणनम्)
      part = 0;  /* An imaginary higher zero part.  */
    अण्णहा
      part = significand[गणनम्] << shift;

    जइ (गणनम् && shift)
      part |= significand[गणनम् - 1] >> (integerPartWidth - shift);

    /* Convert as much of "part" to hexdigits as we can.  */
    unsigned पूर्णाङ्क curDigits = integerPartWidth / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  जइ (roundUp) {
    वर्ण *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    करेहि {
      q--;
      *q = hexDigitChars[hex_Digit_मानम् (*q) + 1];
    } जाव (*q == '0');
    प्रतिज्ञा(q >= p);
  } अण्णहा {
    /* Add trailing zeroes.  */
    memset (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  जइ (dst -1 == p)
    dst--;
  अण्णहा
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = upperCase ? 'P': 'p';

  प्रत्यागम writeSignedDecimal (dst, exponent);
}

hash_code hash_मानम्(ध्रुव IEEEFloat &Arg) {
  जइ (!Arg.isFiniteNonZero())
    प्रत्यागम hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  प्रत्यागम hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
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
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semX87DoubleExtended>();
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
  जइ (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
    प्रतिज्ञा(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.subtract(u, rmNearestTiesToEven);
    fs = v.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
    प्रतिज्ञा(fs == opOK && !losesInfo);
    शब्दाः[1] = *v.convertDoubleAPFloatToAPInt().getRawData();
  } अण्णहा {
    शब्दाः[1] = 0;
  }

  प्रत्यागम APInt(128, शब्दाः);
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

  जइ (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
    जइ (myexponent == 1 &&
        !(significandParts()[integer_bit_part] & integer_bit))
      myexponent = 0; // denormal
  } अण्णहा जइ (category == fcZero) {
    जइ (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } अण्णहा जइ (category == fcInfinity) {
    जइ (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    जइ constexpr (S.hasExplicitIntegerBit) {
      mysignificand[0] = integerPart{1} << (trailing_significand_bits - 1);
    }
  } अण्णहा {
    प्रतिज्ञा(category == fcNaN && "Unknown category!");
    जइ (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.sizeInBits + 63) / 64> शब्दाः;
  auto शब्दाः_iter =
      std::copy_n(mysignificand.begin(), mysignificand.size(), शब्दाः.begin());
  जइ constexpr (!S.hasExplicitIntegerBit) {
    जइ constexpr (significand_mask != 0 || trailing_significand_bits == 0) {
      // Clear the integer bit.
      शब्दाः[mysignificand.size() - 1] &= significand_mask;
    }
  }
  std::fill(शब्दाः_iter, शब्दाः.end(), uint64_t{0});
  constexpr size_t last_word = शब्दाः.size() - 1;
  जइ constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.sizeInBits - 1) % 64);
    शब्दाः[last_word] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_mask)
                              << (trailing_significand_bits % 64);
  शब्दाः[last_word] |= shifted_exponent;
  जइ constexpr (last_word == 0) {
    प्रत्यागम APInt(S.sizeInBits, शब्दाः[0]);
  }
  प्रत्यागम APInt(S.sizeInBits, शब्दाः);
}

APInt IEEEFloat::convertQuadrupleAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 2);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semIEEEquad>();
}

APInt IEEEFloat::convertDoubleAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्()==1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semIEEEdouble>();
}

APInt IEEEFloat::convertFloatAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्()==1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semIEEEsingle>();
}

APInt IEEEFloat::convertBFloatAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semBFloat>();
}

APInt IEEEFloat::convertHalfAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्()==1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::APFloatBase::semIEEEhalf>();
}

APInt IEEEFloat::convertFloat8E5M2APFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2>();
}

APInt IEEEFloat::convertFloat8E5M2FNUZAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3APFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3>();
}

APInt IEEEFloat::convertFloat8E4M3FNAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FN>();
}

APInt IEEEFloat::convertFloat8E4M3FNUZAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3B11FNUZAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convertFloat8E3M4APFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semFloat8E3M4>();
}

APInt IEEEFloat::convertFloatTF32APFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semFloatTF32>();
}

APInt IEEEFloat::convertFloat8E8M0FNUAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convertFloat8E5M3FNUAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convertFloat6E3M2FNAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semFloat6E3M2FN>();
}

APInt IEEEFloat::convertFloat6E2M3FNAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semFloat6E2M3FN>();
}

APInt IEEEFloat::convertFloat4E2M1FNAPFloatToAPInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convertIEEEFloatToAPInt<APFloatBase::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcastToAPInt() ध्रुव {
  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semIEEEhalf)
    प्रत्यागम convertHalfAPFloatToAPInt();

  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semBFloat)
    प्रत्यागम convertBFloatAPFloatToAPInt();

  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semIEEEsingle)
    प्रत्यागम convertFloatAPFloatToAPInt();

  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semIEEEdouble)
    प्रत्यागम convertDoubleAPFloatToAPInt();

  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semIEEEquad)
    प्रत्यागम convertQuadrupleAPFloatToAPInt();

  जइ (semantics ==
      (ध्रुव llvm::fltSemantics *)&APFloatBase::semX87DoubleExtended)
    प्रत्यागम convertF80LongDoubleAPFloatToAPInt();

  जइ (semantics ==
      (ध्रुव llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy)
    प्रत्यागम convertPPCDoubleDoubleLegacyAPFloatToAPInt();

  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E5M2)
    प्रत्यागम convertFloat8E5M2APFloatToAPInt();

  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E5M2FNUZ)
    प्रत्यागम convertFloat8E5M2FNUZAPFloatToAPInt();

  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E4M3)
    प्रत्यागम convertFloat8E4M3APFloatToAPInt();

  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FN)
    प्रत्यागम convertFloat8E4M3FNAPFloatToAPInt();

  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FNUZ)
    प्रत्यागम convertFloat8E4M3FNUZAPFloatToAPInt();

  जइ (semantics ==
      (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E4M3B11FNUZ)
    प्रत्यागम convertFloat8E4M3B11FNUZAPFloatToAPInt();

  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E3M4)
    प्रत्यागम convertFloat8E3M4APFloatToAPInt();

  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloatTF32)
    प्रत्यागम convertFloatTF32APFloatToAPInt();

  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E8M0FNU)
    प्रत्यागम convertFloat8E8M0FNUAPFloatToAPInt();

  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat8E5M3FNU)
    प्रत्यागम convertFloat8E5M3FNUAPFloatToAPInt();

  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat6E3M2FN)
    प्रत्यागम convertFloat6E3M2FNAPFloatToAPInt();

  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat6E2M3FN)
    प्रत्यागम convertFloat6E2M3FNAPFloatToAPInt();

  जइ (semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semFloat4E2M1FN)
    प्रत्यागम convertFloat4E2M1FNAPFloatToAPInt();

  llvm_unreachable("unknown format!");
}

भिन्न IEEEFloat::convertToFloat() ध्रुव {
  प्रतिज्ञा(semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcastToAPInt();
  प्रत्यागम api.bitsToFloat();
}

द्विगुण IEEEFloat::convertToDouble() ध्रुव {
  प्रतिज्ञा(semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcastToAPInt();
  प्रत्यागम api.bitsToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 IEEEFloat::convertToQuad() ध्रुव {
  प्रतिज्ञा(semantics == (ध्रुव llvm::fltSemantics *)&APFloatBase::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcastToAPInt();
  प्रत्यागम api.bitsToQuad();
}
#endif

शून्य IEEEFloat::initFromF80LongDoubleAPInt(ध्रुव APInt &api) {
  प्रत्यागम initFromIEEEAPInt<APFloatBase::semX87DoubleExtended>(api);
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
  जइ (isFiniteNonZero()) {
    IEEEFloat v(APFloatBase::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    जइ (v.isFinite()) {
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
  जइ constexpr (has_significand) {
    std::copy_n(api.getRawData(), mysignificand.size(), mysignificand.begin());
    जइ constexpr (significand_mask != 0 || S.precision >= integerPartWidth) {
      mysignificand[mysignificand.size() - 1] &= significand_mask;
    }
  } अण्णहा {
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

  जइ constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    तार्किक is_inf = मिच्छ;

    जइ constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;
      ध्रुव integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_mask_no_int_bit) == 0;
    } अण्णहा {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    जइ (is_inf) {
      makeInf(sign);
      प्रत्यागम;
    }
  }

  तार्किक is_nan = मिच्छ;

  जइ constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    जइ constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      ध्रुव integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;

      जइ (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_mask_no_int_bit) != 0) {
        // regular NaN and pseudoNaN
        is_nan = सच्च;
      } अण्णहा जइ (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_mask_no_int_bit) == 0) {
        // pseudoinfinity
        is_nan = सच्च;
      } अण्णहा जइ (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = सच्च;
      }
    } अण्णहा {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } अण्णहा जइ constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    तार्किक all_ones_significand =
        std::all_of(mysignificand.begin(), mysignificand.end() - 1,
                    [](integerPart bits) { प्रत्यागम bits == ~integerPart{0}; }) &&
        (!significand_mask ||
         mysignificand[mysignificand.size() - 1] == significand_mask);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } अण्णहा जइ constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  जइ (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.size(),
                significandParts());
    प्रत्यागम;
  }

  जइ (is_zero) {
    makeZero(sign);
    प्रत्यागम;
  }

  category = fcNormal;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.size(), significandParts());
  जइ (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  अण्णहा {
    जइ constexpr (!S.hasExplicitIntegerBit) {
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
  जइ (Sem == &APFloatBase::semIEEEhalf)
    प्रत्यागम initFromHalfAPInt(api);
  जइ (Sem == &APFloatBase::semBFloat)
    प्रत्यागम initFromBFloatAPInt(api);
  जइ (Sem == &APFloatBase::semIEEEsingle)
    प्रत्यागम initFromFloatAPInt(api);
  जइ (Sem == &APFloatBase::semIEEEdouble)
    प्रत्यागम initFromDoubleAPInt(api);
  जइ (Sem == &APFloatBase::semIEEEquad)
    प्रत्यागम initFromQuadrupleAPInt(api);
  जइ (Sem == &APFloatBase::semX87DoubleExtended)
    प्रत्यागम initFromF80LongDoubleAPInt(api);
  जइ (Sem == &APFloatBase::semPPCDoubleDoubleLegacy)
    प्रत्यागम initFromPPCDoubleDoubleLegacyAPInt(api);
  जइ (Sem == &APFloatBase::semFloat8E5M2)
    प्रत्यागम initFromFloat8E5M2APInt(api);
  जइ (Sem == &APFloatBase::semFloat8E5M2FNUZ)
    प्रत्यागम initFromFloat8E5M2FNUZAPInt(api);
  जइ (Sem == &APFloatBase::semFloat8E4M3)
    प्रत्यागम initFromFloat8E4M3APInt(api);
  जइ (Sem == &APFloatBase::semFloat8E4M3FN)
    प्रत्यागम initFromFloat8E4M3FNAPInt(api);
  जइ (Sem == &APFloatBase::semFloat8E4M3FNUZ)
    प्रत्यागम initFromFloat8E4M3FNUZAPInt(api);
  जइ (Sem == &APFloatBase::semFloat8E4M3B11FNUZ)
    प्रत्यागम initFromFloat8E4M3B11FNUZAPInt(api);
  जइ (Sem == &APFloatBase::semFloat8E3M4)
    प्रत्यागम initFromFloat8E3M4APInt(api);
  जइ (Sem == &APFloatBase::semFloatTF32)
    प्रत्यागम initFromFloatTF32APInt(api);
  जइ (Sem == &APFloatBase::semFloat8E8M0FNU)
    प्रत्यागम initFromFloat8E8M0FNUAPInt(api);
  जइ (Sem == &APFloatBase::semFloat8E5M3FNU)
    प्रत्यागम initFromFloat8E5M3FNUAPInt(api);
  जइ (Sem == &APFloatBase::semFloat6E3M2FN)
    प्रत्यागम initFromFloat6E3M2FNAPInt(api);
  जइ (Sem == &APFloatBase::semFloat6E2M3FN)
    प्रत्यागम initFromFloat6E2M3FNAPInt(api);
  जइ (Sem == &APFloatBase::semFloat4E2M1FN)
    प्रत्यागम initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
शून्य IEEEFloat::makeLargest(तार्किक Negative) {
  जइ (Negative && !semantics->hasSignedRepr)
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
  जइ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
शून्य IEEEFloat::makeSmallest(तार्किक Negative) {
  जइ (Negative && !semantics->hasSignedRepr)
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
  जइ (Negative && !semantics->hasSignedRepr)
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

    जइ (bits <= bitsRequired) प्रत्यागम;

    unsigned tensRemovable = (bits - bitsRequired) * 59 / 196;
    जइ (!tensRemovable) प्रत्यागम;

    exp += tensRemovable;

    APInt भाजकः(significand.getBitWidth(), 1);
    APInt powten(significand.getBitWidth(), 10);
    जाव (सच्च) {
      जइ (tensRemovable & 1)
        भाजकः *= powten;
      tensRemovable >>= 1;
      जइ (!tensRemovable) भङ्ग;
      powten *= powten;
    }

    significand = significand.udiv(भाजकः);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.getActiveBits());
  }


  शून्य AdjustToPrecision(SmallVectorImpl<वर्ण> &buffer,
                         पूर्णाङ्क &exp, unsigned FormatPrecision) {
    unsigned N = buffer.size();
    जइ (N <= FormatPrecision) प्रत्यागम;

    // The most significant figures are the last ones in the buffer.
    unsigned FirstSignificant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    जइ (buffer[FirstSignificant - 1] < '5') {
      जाव (FirstSignificant < N && buffer[FirstSignificant] == '0')
        FirstSignificant++;

      exp += FirstSignificant;
      buffer.erase(&buffer[0], &buffer[FirstSignificant]);
      प्रत्यागम;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    प्रति (unsigned I = FirstSignificant; I != N; ++I) {
      जइ (buffer[I] == '9') {
        FirstSignificant++;
      } अण्णहा {
        buffer[I]++;
        भङ्ग;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    जइ (FirstSignificant == N) {
      exp += FirstSignificant;
      buffer.clear();
      buffer.प्रक्षिप_back('1');
      प्रत्यागम;
    }

    exp += FirstSignificant;
    buffer.erase(&buffer[0], &buffer[FirstSignificant]);
  }

  शून्य toStringImpl(SmallVectorImpl<वर्ण> &Str, ध्रुव तार्किक isNeg, पूर्णाङ्क exp,
                    APInt significand, unsigned FormatPrecision,
                    unsigned FormatMaxPadding, तार्किक TruncateZero) {
    ध्रुव पूर्णाङ्क semanticsPrecision = significand.getBitWidth();

    जइ (isNeg)
      Str.प्रक्षिप_back('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    जइ (!FormatPrecision) {
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
    जइ (exp == 0) {
      // Nothing to do.
    } अण्णहा जइ (exp > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + exp);
      significand <<= exp;
      exp = 0;
    } अण्णहा { /* exp < 0 */
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
      जाव (सच्च) {
        जइ (texp & 1)
          significand *= five_to_the_i;

        texp >>= 1;
        जइ (!texp)
          भङ्ग;
        five_to_the_i *= five_to_the_i;
      }
    }

    AdjustToPrecision(significand, exp, FormatPrecision);

    SmallVector<वर्ण, 256> buffer;

    // Fill the buffer.
    unsigned precision = significand.getBitWidth();
    जइ (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    तार्किक inTrail = सच्च;
    जाव (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      unsigned d = digit.get_ZExt_मानम्();

      // Drop trailing zeros.
      जइ (inTrail && !d)
        exp++;
      अण्णहा {
        buffer.प्रक्षिप_back((वर्ण) ('0' + d));
        inTrail = मिच्छ;
      }
    }

    प्रतिज्ञा(!buffer.empty() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    AdjustToPrecision(buffer, exp, FormatPrecision);

    unsigned NDigits = buffer.size();

    // Check whether we should use scientific notation.
    तार्किक FormatScientific;
    जइ (!FormatMaxPadding) {
      FormatScientific = सच्च;
    } अण्णहा {
      जइ (exp >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((unsigned) exp > FormatMaxPadding ||
                            NDigits + (unsigned) exp > FormatPrecision);
      } अण्णहा {
        // Power of the most significant digit.
        पूर्णाङ्क MSD = exp + (पूर्णाङ्क) (NDigits - 1);
        जइ (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = मिच्छ;
        } अण्णहा {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((unsigned) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    जइ (FormatScientific) {
      exp += (NDigits - 1);

      Str.प्रक्षिप_back(buffer[NDigits-1]);
      Str.प्रक्षिप_back('.');
      जइ (NDigits == 1 && TruncateZero)
        Str.प्रक्षिप_back('0');
      अण्णहा
        प्रति (unsigned I = 1; I != NDigits; ++I)
          Str.प्रक्षिप_back(buffer[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      जइ (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.प्रक्षिप_back(TruncateZero ? 'E' : 'e');

      Str.प्रक्षिप_back(exp >= 0 ? '+' : '-');
      जइ (exp < 0)
        exp = -exp;
      SmallVector<वर्ण, 6> expbuf;
      करेहि {
        expbuf.प्रक्षिप_back((वर्ण) ('0' + (exp % 10)));
        exp /= 10;
      } जाव (exp);
      // Exponent always at least two digits if we do not truncate zeros.
      जइ (!TruncateZero && expbuf.size() < 2)
        expbuf.प्रक्षिप_back('0');
      प्रति (unsigned I = 0, E = expbuf.size(); I != E; ++I)
        Str.प्रक्षिप_back(expbuf[E-1-I]);
      प्रत्यागम;
    }

    // Non-scientific, positive exponents.
    जइ (exp >= 0) {
      प्रति (unsigned I = 0; I != NDigits; ++I)
        Str.प्रक्षिप_back(buffer[NDigits-1-I]);
      प्रति (unsigned I = 0; I != (unsigned) exp; ++I)
        Str.प्रक्षिप_back('0');
      प्रत्यागम;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    पूर्णाङ्क NWholeDigits = exp + (पूर्णाङ्क) NDigits;

    unsigned I = 0;
    जइ (NWholeDigits > 0) {
      प्रति (; I != (unsigned) NWholeDigits; ++I)
        Str.प्रक्षिप_back(buffer[NDigits-I-1]);
      Str.प्रक्षिप_back('.');
    } अण्णहा {
      unsigned NZeros = 1 + (unsigned) -NWholeDigits;

      Str.प्रक्षिप_back('0');
      Str.प्रक्षिप_back('.');
      प्रति (unsigned Z = 1; Z != NZeros; ++Z)
        Str.प्रक्षिप_back('0');
    }

    प्रति (; I != NDigits; ++I)
      Str.प्रक्षिप_back(buffer[NDigits-I-1]);

  }
} // namespace

शून्य IEEEFloat::toString(SmallVectorImpl<वर्ण> &Str, unsigned FormatPrecision,
                         unsigned FormatMaxPadding, तार्किक TruncateZero) ध्रुव {
  विकल्प (category) {
  पक्ष fcInfinity:
    जइ (isNegative())
      प्रत्यागम append(Str, "-Inf");
    अण्णहा
      प्रत्यागम append(Str, "+Inf");

  पक्ष fcNaN: प्रत्यागम append(Str, "NaN");

  पक्ष fcZero:
    जइ (isNegative())
      Str.प्रक्षिप_back('-');

    जइ (!FormatMaxPadding) {
      जइ (TruncateZero)
        append(Str, "0.0E+0");
      अण्णहा {
        append(Str, "0.0");
        जइ (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } अण्णहा {
      Str.प्रक्षिप_back('0');
    }
    प्रत्यागम;

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
  जइ (!isFinite() || isZero())
    प्रत्यागम INT_MIN;

  ध्रुव integerPart *Parts = significandParts();
  ध्रुव पूर्णाङ्क Part_गणनम् = part_गणनम्_For_Bits(semantics->precision);

  पूर्णाङ्क निष्कासय_गणनम् = 0;
  प्रति (पूर्णाङ्क i = 0; i < Part_गणनम्; ++i) {
    निष्कासय_गणनम् += llvm::popcount(Parts[i]);
    जइ (निष्कासय_गणनम् > 1)
      प्रत्यागम INT_MIN;
  }

  जइ (exponent != semantics->minExponent)
    प्रत्यागम exponent;

  पूर्णाङ्क CountrParts = 0;
  प्रति (पूर्णाङ्क i = 0; i < Part_गणनम्;
       ++i, CountrParts += APInt::APINT_BITS_PER_WORD) {
    जइ (Parts[i] != 0) {
      प्रत्यागम exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

तार्किक IEEEFloat::isSignaling() ध्रुव {
  जइ (!isNaN())
    प्रत्यागम मिच्छ;
  जइ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    प्रत्यागम मिच्छ;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  प्रत्यागम !APInt::tcExtractBit(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::opStatus IEEEFloat::next(तार्किक nextDown) {
  // If we are performing nextDown, swap sign so we have -x.
  जइ (nextDown)
    changeSign();

  // Compute nextUp(x)
  opStatus result = opOK;

  // Handle each float category separately.
  विकल्प (category) {
  पक्ष fcInfinity:
    // nextUp(+inf) = +inf
    जइ (!isNegative())
      भङ्ग;
    // nextUp(-inf) = -getLargest()
    makeLargest(सच्च);
    भङ्ग;
  पक्ष fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    जइ (isSignaling()) {
      result = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(मिच्छ, isNegative(), nullptr);
    }
    भङ्ग;
  पक्ष fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(मिच्छ);
    भङ्ग;
  पक्ष fcNormal:
    // nextUp(-getSmallest()) = -0
    जइ (isSmallest() && isNegative()) {
      APInt::tcSet(significandParts(), 0, part_गणनम्());
      category = fcZero;
      exponent = 0;
      जइ (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = मिच्छ;
      जइ (!semantics->hasZero)
        makeSmallestNormalized(मिच्छ);
      भङ्ग;
    }

    जइ (isLargest() && !isNegative()) {
      जइ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        भङ्ग;
      } अण्णहा जइ (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        भङ्ग;
      } अण्णहा {
        // nextUp(getLargest()) == INFINITY
        APInt::tcSet(significandParts(), 0, part_गणनम्());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        भङ्ग;
      }
    }

    // nextUp(normal) == normal + inc.
    जइ (isNegative()) {
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

      जइ (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tcSetBit(Parts, semantics->precision - 1);
        exponent--;
      }
    } अण्णहा {
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

      जइ (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tcSet(Parts, 0, part_गणनम्());
        APInt::tcSetBit(Parts, semantics->precision - 1);
        प्रतिज्ञा(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } अण्णहा {
        incrementSignificand();
      }
    }
    भङ्ग;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  जइ (nextDown)
    changeSign();

  प्रत्यागम result;
}

APInt IEEEFloat::getNaNPayload() ध्रुव {
  प्रतिज्ञा(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  unsigned Bits = semantics->precision - 1;
  प्रत्यागम APInt(Bits, ArrayRef(significandParts(), part_गणनम्_For_Bits(Bits)));
}

APFloatBase::ExponentType IEEEFloat::exponentNaN() ध्रुव {
  प्रत्यागम ::exponentNaN(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentInf() ध्रुव {
  प्रत्यागम ::exponentInf(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentZero() ध्रुव {
  प्रत्यागम ::exponentZero(*semantics);
}

शून्य IEEEFloat::makeInf(तार्किक Negative) {
  जइ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  जइ (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(मिच्छ, Negative);
    प्रत्यागम;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tcSet(significandParts(), 0, part_गणनम्());
}

शून्य IEEEFloat::makeZero(तार्किक Negative) {
  जइ (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  जइ (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = मिच्छ;
  }
  exponent = exponentZero();
  APInt::tcSet(significandParts(), 0, part_गणनम्());
}

शून्य IEEEFloat::makeQuiet() {
  प्रतिज्ञा(isNaN());
  जइ (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tcSetBit(significandParts(), semantics->precision - 2);
}

पूर्णाङ्क ilogb(ध्रुव IEEEFloat &Arg) {
  जइ (Arg.isNaN())
    प्रत्यागम APFloat::IEK_NaN;
  जइ (Arg.isZero())
    प्रत्यागम APFloat::IEK_Zero;
  जइ (Arg.isInfinity())
    प्रत्यागम APFloat::IEK_Inf;
  जइ (!Arg.isDenormal())
    प्रत्यागम Arg.exponent;

  IEEEFloat Normalized(Arg);
  पूर्णाङ्क SignificandBits = Arg.getSemantics().precision - 1;

  Normalized.exponent += SignificandBits;
  Normalized.normalize(APFloat::rmNearestTiesToEven, lfExactlyZero);
  प्रत्यागम Normalized.exponent - SignificandBits;
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
  जइ (X.isNaN())
    X.makeQuiet();
  प्रत्यागम X;
}

IEEEFloat frexp(ध्रुव IEEEFloat &Val, पूर्णाङ्क &Exp, roundingMode RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  जइ (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    प्रत्यागम Quiet;
  }

  जइ (Exp == APFloat::IEK_Inf)
    प्रत्यागम Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  प्रत्यागम scalbn(Val, -Exp, RM);
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
  जइ (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } अण्णहा जइ (एतत् != &RHS) {
    एतत्->~DoubleAPFloat();
    नूतन (एतत्) DoubleAPFloat(RHS);
  }
  प्रत्यागम *एतत्;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
स्थिर std::pair<APFloat, APFloat> fastTwoSum(APFloat X, APFloat Y) {
  जइ (!X.isFinite())
    प्रत्यागम {X, APFloat::getZero(X.getSemantics(), /*Negative=*/मिच्छ)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  प्रत्यागम {Hi, Lo};
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
  जइ (!z.isFinite()) {
    जइ (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ मिच्छ);
      प्रत्यागम (opStatus)Status;
    }
    Status = opOK;
    auto AComparedToC = a.compare_Absolute_मानम्(c);
    z = cc;
    Status |= z.add(aa, RM);
    जइ (AComparedToC == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      Status |= z.add(c, RM);
      Status |= z.add(a, RM);
    } अण्णहा {
      // z = cc + aa + a + c;
      Status |= z.add(a, RM);
      Status |= z.add(c, RM);
    }
    जइ (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ मिच्छ);
      प्रत्यागम (opStatus)Status;
    }
    Floats[0] = z;
    APFloat zz = aa;
    Status |= zz.add(cc, RM);
    जइ (AComparedToC == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(c, RM);
      Status |= Floats[1].add(zz, RM);
    } अण्णहा {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(a, RM);
      Status |= Floats[1].add(zz, RM);
    }
  } अण्णहा {
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
    जइ (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ मिच्छ);
      प्रत्यागम opOK;
    }
    Floats[0] = z;
    Status |= Floats[0].add(zz, RM);
    जइ (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ मिच्छ);
      प्रत्यागम (opStatus)Status;
    }
    Floats[1] = std::move(z);
    Status |= Floats[1].subtract(Floats[0], RM);
    Status |= Floats[1].add(zz, RM);
  }
  प्रत्यागम (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::addWithSpecial(ध्रुव DoubleAPFloat &LHS,
                                                ध्रुव DoubleAPFloat &RHS,
                                                DoubleAPFloat &Out,
                                                roundingMode RM) {
  जइ (LHS.getCategory() == fcNaN) {
    Out = LHS;
    प्रत्यागम opOK;
  }
  जइ (RHS.getCategory() == fcNaN) {
    Out = RHS;
    प्रत्यागम opOK;
  }
  जइ (LHS.getCategory() == fcZero) {
    Out = RHS;
    प्रत्यागम opOK;
  }
  जइ (RHS.getCategory() == fcZero) {
    Out = LHS;
    प्रत्यागम opOK;
  }
  जइ (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    Out.makeNaN(मिच्छ, Out.isNegative(), nullptr);
    प्रत्यागम opInvalidOp;
  }
  जइ (LHS.getCategory() == fcInfinity) {
    Out = LHS;
    प्रत्यागम opOK;
  }
  जइ (RHS.getCategory() == fcInfinity) {
    Out = RHS;
    प्रत्यागम opOK;
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
  प्रत्यागम Out.addImpl(A, AA, C, CC, RM);
}

APFloat::opStatus DoubleAPFloat::add(ध्रुव DoubleAPFloat &RHS,
                                     roundingMode RM) {
  प्रत्यागम addWithSpecial(*एतत्, RHS, *एतत्, RM);
}

APFloat::opStatus DoubleAPFloat::subtract(ध्रुव DoubleAPFloat &RHS,
                                          roundingMode RM) {
  changeSign();
  auto Ret = add(RHS, RM);
  changeSign();
  प्रत्यागम Ret;
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
  जइ (LHS.getCategory() == fcNaN) {
    Out = LHS;
    प्रत्यागम opOK;
  }
  जइ (RHS.getCategory() == fcNaN) {
    Out = RHS;
    प्रत्यागम opOK;
  }
  जइ ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    Out.makeNaN(मिच्छ, मिच्छ, nullptr);
    प्रत्यागम opOK;
  }
  जइ (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    Out = LHS;
    प्रत्यागम opOK;
  }
  जइ (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    Out = RHS;
    प्रत्यागम opOK;
  }
  प्रतिज्ञा(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal &&
         "Special cases not handled exhaustively");

  पूर्णाङ्क Status = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  Status |= T.multiply(C, RM);
  जइ (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ मिच्छ);
    प्रत्यागम (opStatus)Status;
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
  जइ (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ मिच्छ);
  } अण्णहा {
    // Floats[1] = (t - u) + tau
    Status |= T.subtract(U, RM);
    Status |= T.add(Tau, RM);
    Floats[1] = std::move(T);
  }
  प्रत्यागम (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::divide(ध्रुव DoubleAPFloat &RHS,
                                        APFloat::roundingMode RM) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.divide(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()), RM);
  *एतत् = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  प्रत्यागम Ret;
}

APFloat::opStatus DoubleAPFloat::remainder(ध्रुव DoubleAPFloat &RHS) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.remainder(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *एतत् = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  प्रत्यागम Ret;
}

APFloat::opStatus DoubleAPFloat::mod(ध्रुव DoubleAPFloat &RHS) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.mod(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *एतत् = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  प्रत्यागम Ret;
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
  प्रत्यागम Ret;
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
  जइ (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/मिच्छ);
    प्रत्यागम HiStatus;
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
    जइ (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        abs(RoundingError).is_Exactly_मानम्(0.5))
      Rounded.add(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rmNearestTiesToEven);
    प्रत्यागम Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  जइ (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    जइ (RM == rmNearestTiesToAway || RM == rmNearestTiesToEven)
      RoundedHi = RoundToNearestHelper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/मिच्छ);
    प्रत्यागम HiStatus;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  RoundingMode LoRoundingMode;
  जइ (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    LoRoundingMode = isNegative() ? rmTowardPositive : rmTowardNegative;
  अण्णहा
    LoRoundingMode = RM;

  APFloat RoundedLo = Lo;
  ध्रुव opStatus LoStatus = RoundedLo.roundToIntegral(LoRoundingMode);
  जइ (LoRoundingMode == rmNearestTiesToAway)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = RoundToNearestHelper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fastTwoSum(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  प्रत्यागम LoStatus;
}

शून्य DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmpResult
DoubleAPFloat::compare_Absolute_मानम्(ध्रुव DoubleAPFloat &RHS) ध्रुव {
  // Compare absolute values of the high parts.
  ध्रुव cmpResult HiPartCmp = Floats[0].compare_Absolute_मानम्(RHS.Floats[0]);
  जइ (HiPartCmp != cmpEqual)
    प्रत्यागम HiPartCmp;

  // Zero, regardless of sign, is equal.
  जइ (Floats[1].isZero() && RHS.Floats[1].isZero())
    प्रत्यागम cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  ध्रुव तार्किक ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  ध्रुव तार्किक RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  जइ (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    प्रत्यागम RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  जइ (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    प्रत्यागम ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  जइ (ThisIsSubtractive != RHSIsSubtractive)
    प्रत्यागम ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  ध्रुव cmpResult LoPartCmp = Floats[1].compare_Absolute_मानम्(RHS.Floats[1]);

  जइ (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    जइ (LoPartCmp == cmpLessThan)
      प्रत्यागम cmpGreaterThan;
    जइ (LoPartCmp == cmpGreaterThan)
      प्रत्यागम cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  प्रत्यागम LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() ध्रुव {
  प्रत्यागम Floats[0].getCategory();
}

तार्किक DoubleAPFloat::isNegative() ध्रुव { प्रत्यागम Floats[0].isNegative(); }

शून्य DoubleAPFloat::makeInf(तार्किक Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ मिच्छ);
}

शून्य DoubleAPFloat::makeZero(तार्किक Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ मिच्छ);
}

शून्य DoubleAPFloat::makeLargest(तार्किक Neg) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  जइ (Neg)
    changeSign();
}

शून्य DoubleAPFloat::makeSmallest(तार्किक Neg) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].makeSmallest(Neg);
  Floats[1].makeZero(/* Neg = */ मिच्छ);
}

शून्य DoubleAPFloat::makeSmallestNormalized(तार्किक Neg) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  जइ (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ मिच्छ);
}

शून्य DoubleAPFloat::makeNaN(तार्किक SNaN, तार्किक Neg, ध्रुव APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ मिच्छ);
}

APFloat::cmpResult DoubleAPFloat::compare(ध्रुव DoubleAPFloat &RHS) ध्रुव {
  auto Result = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  जइ (Result == APFloat::cmpEqual)
    प्रत्यागम Floats[1].compare(RHS.Floats[1]);
  प्रत्यागम Result;
}

तार्किक DoubleAPFloat::bitwiseIsEqual(ध्रुव DoubleAPFloat &RHS) ध्रुव {
  प्रत्यागम Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_code hash_मानम्(ध्रुव DoubleAPFloat &Arg) {
  जइ (Arg.Floats)
    प्रत्यागम hash_combine(hash_मानम्(Arg.Floats[0]), hash_मानम्(Arg.Floats[1]));
  प्रत्यागम hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcastToAPInt() ध्रुव {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t Data[] = {
      Floats[0].bitcastToAPInt().getRawData()[0],
      Floats[1].bitcastToAPInt().getRawData()[0],
  };
  प्रत्यागम APInt(128, Data);
}

Expected<APFloat::opStatus> DoubleAPFloat::convertFromString(StringRef S,
                                                             roundingMode RM) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy);
  auto Ret = Tmp.convertFromString(S, RM);
  *एतत् = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  प्रत्यागम Ret;
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
  जइ (nextDown) {
    changeSign();
    APFloat::opStatus Result = next(/*nextDown=*/मिच्छ);
    changeSign();
    प्रत्यागम Result;
  }
  विकल्प (getCategory()) {
  पक्ष fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    जइ (isNegative())
      makeLargest(सच्च);
    प्रत्यागम opOK;

  पक्ष fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    जइ (getFirst().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(मिच्छ, isNegative(), nullptr);
      प्रत्यागम opInvalidOp;
    }
    प्रत्यागम opOK;

  पक्ष fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(मिच्छ);
    प्रत्यागम opOK;

  पक्ष fcNormal:
    भङ्ग;
  }

  ध्रुव APFloat &HiOld = getFirst();
  ध्रुव APFloat &LoOld = getSecond();

  APFloat NextLo = LoOld;
  NextLo.next(/*nextDown=*/मिच्छ);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  auto InLattice = [](ध्रुव APFloat &Hi, ध्रुव APFloat &Lo) {
    प्रत्यागम Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  जइ (InLattice(HiOld, NextLo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(NextLo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat Largest{*Semantics, uninitialized};
    Largest.makeLargest(/*Neg=*/मिच्छ);
    जइ (compare(Largest) == cmpGreaterThan)
      makeInf(/*Neg=*/मिच्छ);

    प्रत्यागम opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat NextHi = HiOld;
  NextHi.next(/*nextDown=*/मिच्छ);

  // nextUp(getLargest()) == INFINITY
  जइ (NextHi.isInfinity()) {
    makeInf(/*Neg=*/मिच्छ);
    प्रत्यागम opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  जइ (NextHi.isZero()) {
    makeZero(/*Neg=*/सच्च);
    प्रत्यागम opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  NextLo = neg(scalbn(harrisonUlp(NextHi), -1, rmTowardZero));
  जइ (!InLattice(NextHi, NextLo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    NextLo.next(/*nextDown=*/मिच्छ);

  Floats[0] = std::move(NextHi);
  Floats[1] = std::move(NextLo);

  प्रत्यागम opOK;
}

APFloat::opStatus DoubleAPFloat::convertToSignExtendedInteger(
    MutableArrayRef<integerPart> निवेशः, unsigned पूर्णाङ्क Width, तार्किक IsSigned,
    roundingMode RM, तार्किक *IsExact) ध्रुव {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  जइ (!getFirst().isFiniteNonZero() || getSecond().isZero())
    प्रत्यागम getFirst().convertToInteger(निवेशः, Width, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = मिच्छ;
  DoubleAPFloat Integral = *एतत्;
  ध्रुव opStatus RoundStatus = Integral.roundToIntegral(RM);
  जइ (RoundStatus == opInvalidOp)
    प्रत्यागम opInvalidOp;
  ध्रुव APFloat &IntegralHi = Integral.getFirst();
  ध्रुव APFloat &IntegralLo = Integral.getSecond();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  तार्किक HiIsExact;
  जइ (IntegralHi.isZero() || IntegralLo.isZero()) {
    ध्रुव opStatus HiStatus =
        IntegralHi.convertToInteger(निवेशः, Width, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    प्रत्यागम HiStatus == opOK ? opInexact : HiStatus;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  जइ (!IsSigned && IntegralHi.isNegative())
    प्रत्यागम opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  तार्किक LoIsExact;
  ध्रुव पूर्णाङ्क HiExactLog2 = IntegralHi.getExactLog2Abs();
  ध्रुव unsigned PositiveOverflowWidth = IsSigned ? Width - 1 : Width;
  जइ (HiExactLog2 >= 0 &&
      static_cast<unsigned>(HiExactLog2) == PositiveOverflowWidth) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    जइ (IntegralHi.isNegative() == IntegralLo.isNegative())
      प्रत्यागम opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    ध्रुव opStatus LoStatus = IntegralLo.convertToInteger(
        निवेशः, Width, /*IsSigned=*/सच्च, RM, &LoIsExact);
    जइ (LoStatus == opInvalidOp)
      प्रत्यागम opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    जइ (IsSigned && !IntegralHi.isNegative())
      APInt::tcClearBit(निवेशः.data(), PositiveOverflowWidth);
    *IsExact = RoundStatus == opOK;
    प्रत्यागम RoundStatus;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  ध्रुव opStatus HiStatus = IntegralHi.convertToInteger(
      निवेशः, Width, IsSigned, rmTowardZero, &HiIsExact);
  जइ (HiStatus == opInvalidOp)
    प्रत्यागम HiStatus;

  // Convert Lo into a temporary integer of the same width.
  APSInt LoResult{Width, /*isUnsigned=*/!IsSigned};
  ध्रुव opStatus LoStatus =
      IntegralLo.convertToInteger(LoResult, rmTowardZero, &LoIsExact);
  जइ (LoStatus == opInvalidOp)
    प्रत्यागम LoStatus;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tcAdd(निवेशः.data(), LoResult.getRawData(), /*carry=*/0, निवेशः.size());

  *IsExact = RoundStatus == opOK;
  प्रत्यागम RoundStatus;
}

APFloat::opStatus
DoubleAPFloat::convertToInteger(MutableArrayRef<integerPart> निवेशः,
                                unsigned पूर्णाङ्क Width, तार्किक IsSigned,
                                roundingMode RM, तार्किक *IsExact) ध्रुव {
  opStatus FS =
      convertToSignExtendedInteger(निवेशः, Width, IsSigned, RM, IsExact);

  जइ (FS == opInvalidOp) {
    ध्रुव unsigned Dst_Parts_गणनम् = part_गणनम्_For_Bits(Width);
    प्रतिज्ञा(Dst_Parts_गणनम् <= निवेशः.size() && "Integer too big");

    unsigned Bits;
    जइ (getCategory() == fcNaN)
      Bits = 0;
    अण्णहा जइ (isNegative())
      Bits = IsSigned;
    अण्णहा
      Bits = Width - IsSigned;

    tcSetLeastSignificantBits(निवेशः.data(), Dst_Parts_गणनम्, Bits);
    जइ (isNegative() && IsSigned)
      APInt::tc_Shift_वामः(निवेशः.data(), Dst_Parts_गणनम्, Width - 1);
  }

  प्रत्यागम FS;
}

APFloat::opStatus DoubleAPFloat::handleOverflow(roundingMode RM) {
  विकल्प (RM) {
  पक्ष APFloat::rmTowardZero:
    makeLargest(/*Neg=*/isNegative());
    भङ्ग;
  पक्ष APFloat::rmTowardNegative:
    जइ (isNegative())
      makeInf(/*Neg=*/सच्च);
    अण्णहा
      makeLargest(/*Neg=*/मिच्छ);
    भङ्ग;
  पक्ष APFloat::rmTowardPositive:
    जइ (isNegative())
      makeLargest(/*Neg=*/सच्च);
    अण्णहा
      makeInf(/*Neg=*/मिच्छ);
    भङ्ग;
  पक्ष APFloat::rmNearestTiesToAway:
  पक्ष APFloat::rmNearestTiesToEven:
    makeInf(/*Neg=*/isNegative());
    भङ्ग;
  स्वभाव:
    llvm_unreachable("Invalid rounding mode found");
  }
  opStatus S = opInexact;
  जइ (!getFirst().isFinite())
    S = static_cast<opStatus>(S | opOverflow);
  प्रत्यागम S;
}

APFloat::opStatus DoubleAPFloat::convertFromUnsignedParts(
    ध्रुव integerPart *Src, unsigned पूर्णाङ्क Src_गणनम्, roundingMode RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  ध्रुव unsigned SrcMSB = APInt::tcMSB(Src, Src_गणनम्);
  जइ (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/मिच्छ);
    प्रत्यागम opOK;
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
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/मिच्छ, rmNearestTiesToEven);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  जइ (!Hi.isFinite())
    प्रत्यागम handleOverflow(RM);

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
  जइ (Error.isNegative()) {
    जइ (RM == rmNearestTiesToAway) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      ध्रुव unsigned ErrorActiveBits = Error.getSignificantBits() - 1;
      ध्रुव unsigned LoPrecision = getSecond().getSemantics().precision;
      जइ (ErrorActiveBits > LoPrecision) {
        ध्रुव unsigned RoundingBoundary = ErrorActiveBits - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        जइ (Error.गणनम्_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rmTowardPositive;
      }
    } अण्णहा जइ (RM == rmTowardZero) {
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
  जइ (!getFirst().isFinite())
    प्रत्यागम handleOverflow(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  जइ (getFirst().isFiniteNonZero() && Floats[0].isLargest()) {
    DoubleAPFloat Largest{*Semantics};
    Largest.makeLargest(/*Neg=*/मिच्छ);
    जइ (compare(Largest) == APFloat::cmpGreaterThan)
      प्रत्यागम handleOverflow(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  प्रत्यागम Status;
}

APFloat::opStatus DoubleAPFloat::convertFromAPInt(ध्रुव APInt &निवेशः,
                                                  तार्किक IsSigned,
                                                  roundingMode RM) {
  ध्रुव तार्किक Negate_निवेशः = IsSigned && निवेशः.isNegative();
  APInt API = निवेशः;
  जइ (Negate_निवेशः)
    API.negate();

  ध्रुव APFloat::opStatus Status =
      convertFromUnsignedParts(API.getRawData(), API.get_अङ्कः_शब्दाः(), RM);
  जइ (Negate_निवेशः)
    changeSign();
  प्रत्यागम Status;
}

unsigned पूर्णाङ्क DoubleAPFloat::convertToHexString(वर्ण *DST,
                                               unsigned पूर्णाङ्क HexDigits,
                                               तार्किक UpperCase,
                                               roundingMode RM) ध्रुव {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  प्रत्यागम APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .convertToHexString(DST, HexDigits, UpperCase, RM);
}

तार्किक DoubleAPFloat::isDenormal() ध्रुव {
  प्रत्यागम getCategory() == fcNormal &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

तार्किक DoubleAPFloat::isSmallest() ध्रुव {
  जइ (getCategory() != fcNormal)
    प्रत्यागम मिच्छ;
  DoubleAPFloat Tmp(*एतत्);
  Tmp.makeSmallest(एतत्->isNegative());
  प्रत्यागम Tmp.compare(*एतत्) == cmpEqual;
}

तार्किक DoubleAPFloat::isSmallestNormalized() ध्रुव {
  जइ (getCategory() != fcNormal)
    प्रत्यागम मिच्छ;

  DoubleAPFloat Tmp(*एतत्);
  Tmp.makeSmallestNormalized(एतत्->isNegative());
  प्रत्यागम Tmp.compare(*एतत्) == cmpEqual;
}

तार्किक DoubleAPFloat::isLargest() ध्रुव {
  जइ (getCategory() != fcNormal)
    प्रत्यागम मिच्छ;
  DoubleAPFloat Tmp(*एतत्);
  Tmp.makeLargest(एतत्->isNegative());
  प्रत्यागम Tmp.compare(*एतत्) == cmpEqual;
}

तार्किक DoubleAPFloat::isInteger() ध्रुव {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  प्रत्यागम Floats[0].isInteger() && Floats[1].isInteger();
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
  जइ (getSecond().isNonZero())
    प्रत्यागम INT_MIN;
  प्रत्यागम getFirst().getExactLog2Abs();
}

पूर्णाङ्क ilogb(ध्रुव DoubleAPFloat &Arg) {
  ध्रुव APFloat &Hi = Arg.getFirst();
  ध्रुव APFloat &Lo = Arg.getSecond();
  पूर्णाङ्क IlogbResult = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  जइ (Arg.getCategory() != fcNormal)
    प्रत्यागम IlogbResult;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  जइ (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    प्रत्यागम IlogbResult;
  जइ (Hi.getExactLog2Abs() == INT_MIN)
    प्रत्यागम IlogbResult;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  प्रत्यागम IlogbResult - 1;
}

DoubleAPFloat scalbn(ध्रुव DoubleAPFloat &Arg, पूर्णाङ्क Exp,
                     APFloat::roundingMode RM) {
  प्रतिज्ञा(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");
  प्रत्यागम DoubleAPFloat(APFloatBase::PPCDoubleDouble(),
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
  जइ (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.getFirst() = Quiet.getFirst().makeQuiet();
    प्रत्यागम Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  जइ (Exp == APFloat::IEK_Inf)
    प्रत्यागम Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  जइ (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    प्रत्यागम Arg;
  }

  ध्रुव APFloat &Hi = Arg.getFirst();
  ध्रुव APFloat &Lo = Arg.getSecond();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  ध्रुव तार्किक SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat Second = Lo;
  जइ (Arg.getCategory() == APFloat::fcNormal && Lo.isFiniteNonZero()) {
    roundingMode LoRoundingMode;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    जइ (RM == rmTowardZero)
      LoRoundingMode = Arg.isNegative() ? rmTowardPositive : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    अण्णहा जइ (RM == rmNearestTiesToAway && SignsDisagree && Exp > 0)
      LoRoundingMode = rmNearestTiesToEven;
    अण्णहा
      LoRoundingMode = RM;
    Second = scalbn(Lo, -Exp, LoRoundingMode);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    जइ (RM == rmNearestTiesToAway && LoRoundingMode == rmNearestTiesToEven) {
      // Re-scale the result back to check if rounding occurred.
      ध्रुव APFloat RecomposedLo = scalbn(Second, Exp, rmNearestTiesToEven);
      जइ (RecomposedLo != Lo) {
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
        जइ (IsMidpoint && RoundedLoAway)
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
    जइ (Second.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat First = scalbn(Hi, -Exp, RM);
  प्रत्यागम DoubleAPFloat(APFloatBase::PPCDoubleDouble(), std::move(First),
                       std::move(Second));
}

APInt DoubleAPFloat::getNaNPayload() ध्रुव { प्रत्यागम Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, ध्रुव fltSemantics &Semantics) {
  जइ (usesLayout<IEEEFloat>(Semantics)) {
    नूतन (&IEEE) IEEEFloat(std::move(F));
    प्रत्यागम;
  }
  जइ (usesLayout<DoubleAPFloat>(Semantics)) {
    ध्रुव fltSemantics& S = F.getSemantics();
    नूतन (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloatBase::IEEEdouble()));
    प्रत्यागम;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::opStatus> APFloat::convertFromString(StringRef Str,
                                                       roundingMode RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_code hash_मानम्(ध्रुव APFloat &Arg) {
  जइ (APFloat::usesLayout<detail::IEEEFloat>(Arg.getSemantics()))
    प्रत्यागम hash_मानम्(Arg.U.IEEE);
  जइ (APFloat::usesLayout<detail::DoubleAPFloat>(Arg.getSemantics()))
    प्रत्यागम hash_मानम्(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(ध्रुव fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  auto StatusOrErr = convertFromString(S, rmNearestTiesToEven);
  प्रतिज्ञा(StatusOrErr && "Invalid floating point representation");
  consumeError(StatusOrErr.takeError());
}

FPClassTest APFloat::classify() ध्रुव {
  जइ (isZero())
    प्रत्यागम isNegative() ? fcNegZero : fcPosZero;
  जइ (isNormal())
    प्रत्यागम isNegative() ? fcNegNormal : fcPosNormal;
  जइ (isDenormal())
    प्रत्यागम isNegative() ? fcNegSubnormal : fcPosSubnormal;
  जइ (isInfinity())
    प्रत्यागम isNegative() ? fcNegInf : fcPosInf;
  प्रतिज्ञा(isNaN() && "Other class of FP constant");
  प्रत्यागम isSignaling() ? fcSNan : fcQNan;
}

तार्किक APFloat::getExactInverse(APFloat *Inv) ध्रुव {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  जइ (!isFiniteNonZero())
    प्रत्यागम मिच्छ;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  जइ (isDenormal())
    प्रत्यागम मिच्छ;

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
  जइ (Exp == INT_MIN)
    प्रत्यागम मिच्छ;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  जइ (Reciprocal.getExactLog2Abs() != -Exp)
    प्रत्यागम मिच्छ;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  जइ (Reciprocal.isDenormal())
    प्रत्यागम मिच्छ;

  प्रतिज्ञा(Reciprocal.isFiniteNonZero());

  जइ (Inv)
    *Inv = std::move(Reciprocal);

  प्रत्यागम सच्च;
}

APFloat::opStatus APFloat::convert(ध्रुव fltSemantics &ToSemantics,
                                   roundingMode RM, तार्किक *losesInfo) {
  जइ (&getSemantics() == &ToSemantics) {
    *losesInfo = मिच्छ;
    प्रत्यागम opOK;
  }
  जइ (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics))
    प्रत्यागम U.IEEE.convert(ToSemantics, RM, losesInfo);
  जइ (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<DoubleAPFloat>(ToSemantics)) {
    प्रतिज्ञा(&ToSemantics == &APFloatBase::semPPCDoubleDouble);
    auto Ret =
        U.IEEE.convert(APFloatBase::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *एतत् = APFloat(ToSemantics, U.IEEE.bitcastToAPInt());
    प्रत्यागम Ret;
  }
  जइ (usesLayout<DoubleAPFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics)) {
    auto Ret = getIEEE().convert(ToSemantics, RM, losesInfo);
    *एतत् = APFloat(std::move(getIEEE()), ToSemantics);
    प्रत्यागम Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_मानम्(ध्रुव fltSemantics &Semantics) {
  प्रत्यागम APFloat(Semantics, APInt::getAllOnes(Semantics.sizeInBits));
}

शून्य APFloat::print(raw_ostream &OS) ध्रुव {
  SmallVector<वर्ण, 16> Buffer;
  toString(Buffer);
  OS << Buffer;
}

#जइ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
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
  प्रत्यागम status;
}

द्विगुण APFloat::convertToDouble() ध्रुव {
  जइ (&getSemantics() == &APFloatBase::semIEEEdouble)
    प्रत्यागम getIEEE().convertToDouble();
  प्रतिज्ञा(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat Temp = *एतत्;
  तार्किक LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &LosesInfo);
  प्रतिज्ञा(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  प्रत्यागम Temp.getIEEE().convertToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 APFloat::convertToQuad() ध्रुव {
  जइ (&getSemantics() == &APFloatBase::semIEEEquad)
    प्रत्यागम getIEEE().convertToQuad();
  प्रतिज्ञा(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat Temp = *एतत्;
  तार्किक LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEquad, rmNearestTiesToEven, &LosesInfo);
  प्रतिज्ञा(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  प्रत्यागम Temp.getIEEE().convertToQuad();
}
#endif

भिन्न APFloat::convertToFloat() ध्रुव {
  जइ (&getSemantics() == &APFloatBase::semIEEEsingle)
    प्रत्यागम getIEEE().convertToFloat();
  प्रतिज्ञा(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat Temp = *एतत्;
  तार्किक LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEsingle, rmNearestTiesToEven, &LosesInfo);
  प्रतिज्ञा(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  प्रत्यागम Temp.getIEEE().convertToFloat();
}

unsigned APFloatBase::getArbitraryFPFormatSizeInBits(StringRef Format) {
  प्रत्यागम StringSwitch<unsigned>(Format)
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
  प्रत्यागम getArbitraryFPFormatSizeInBits(Format) != 0;
}

ध्रुव fltSemantics *APFloatBase::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  प्रत्यागम StringSwitch<ध्रुव fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(nullptr);
}

APFloat::Storage::~Storage() {
  जइ (usesLayout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    प्रत्यागम;
  }
  जइ (usesLayout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    प्रत्यागम;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(ध्रुव APFloat::Storage &RHS) {
  जइ (usesLayout<IEEEFloat>(*RHS.semantics)) {
    नूतन (एतत्) IEEEFloat(RHS.IEEE);
    प्रत्यागम;
  }
  जइ (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    नूतन (एतत्) DoubleAPFloat(RHS.Double);
    प्रत्यागम;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  जइ (usesLayout<IEEEFloat>(*RHS.semantics)) {
    नूतन (एतत्) IEEEFloat(std::move(RHS.IEEE));
    प्रत्यागम;
  }
  जइ (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    नूतन (एतत्) DoubleAPFloat(std::move(RHS.Double));
    प्रत्यागम;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::operator=(ध्रुव APFloat::Storage &RHS) {
  जइ (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } अण्णहा जइ (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } अण्णहा जइ (एतत् != &RHS) {
    एतत्->~Storage();
    नूतन (एतत्) Storage(RHS);
  }
  प्रत्यागम *एतत्;
}

APFloat::Storage &APFloat::Storage::operator=(APFloat::Storage &&RHS) {
  जइ (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } अण्णहा जइ (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } अण्णहा जइ (एतत् != &RHS) {
    एतत्->~Storage();
    नूतन (एतत्) Storage(std::move(RHS));
  }
  प्रत्यागम *एतत्;
}

नामाकाश {

APFloat::opStatus getOpStatusFromLibc(पूर्णाङ्क libc_exceptions) {
  APFloat::opStatus status = APFloat::opOK;
  जइ (libc_exceptions & FE_INVALID)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInvalidOp);
  जइ (libc_exceptions & FE_DIVBYZERO)
    status = static_cast<APFloat::opStatus>(status | APFloat::opDivByZero);
  जइ (libc_exceptions & FE_OVERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opOverflow);
  जइ (libc_exceptions & FE_UNDERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opUnderflow);
  जइ (libc_exceptions & FE_INEXACT)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInexact);
  प्रत्यागम status;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> exp(ध्रुव APFloat &x, RoundingMode rounding_mode,
                           APFloat::opStatus *status) {

  जइ (rounding_mode == APFloatBase::rmNearestTiesToEven) {
    जइ (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEsingle) {
      भिन्न x_val = x.convertToFloat();
      पूर्णाङ्क exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      जइ (status) {
        *status = getOpStatusFromLibc(exc);
        जइ (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      भिन्न result = LIBC_NAMESPACE::shared::expf(x_val);
      प्रत्यागम APFloat(result);
    }
    जइ (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEdouble) {
      द्विगुण x_val = x.convertToDouble();
      पूर्णाङ्क exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      जइ (status) {
        *status = getOpStatusFromLibc(exc);
        जइ (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      द्विगुण result = LIBC_NAMESPACE::shared::exp(x_val);
      प्रत्यागम APFloat(result);
    }
  }
  प्रत्यागम std::nullopt;
}

} // namespace llvm

#undef APFLOAT_DISPATCH_ON_SEMANTICS
