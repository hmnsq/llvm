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
  करा {                                                                         \
    जर (usesLayout<IEEEFloat>(getSemantics()))                                 \
      परत U.IEEE.METHOD_CALL;                                               \
    जर (usesLayout<DoubleAPFloat>(getSemantics()))                             \
      परत U.Double.METHOD_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } पर्यंत (असत्य)

using नामाकाश llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#define Pack_Categories_Into_किल्ली(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

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
    सत्य,
    सत्य,
    सत्य,
    सत्य,
    सत्य};
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
    असत्य,
    असत्य,
    असत्य,
    असत्य};

constexpr fltSemantics APFloatBase::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    सत्य,
    असत्य,
    असत्य};

constexpr fltSemantics APFloatBase::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semBogus = {0, 0, 0, 0};

स्थिरांक llvm::fltSemantics &APFloatBase::EnumToSemantics(Semantics S) {
  निवड (S) {
  स्थिती S_IEEEhalf:
    परत IEEEhalf();
  स्थिती S_BFloat:
    परत BFloat();
  स्थिती S_IEEEsingle:
    परत IEEEsingle();
  स्थिती S_IEEEdouble:
    परत IEEEdouble();
  स्थिती S_IEEEquad:
    परत IEEEquad();
  स्थिती S_x87DoubleExtended:
    परत x87DoubleExtended();
  स्थिती S_PPCDoubleDouble:
    परत PPCDoubleDouble();
  स्थिती S_PPCDoubleDoubleLegacy:
    परत PPCDoubleDoubleLegacy();
  स्थिती S_Float8E5M2:
    परत Float8E5M2();
  स्थिती S_Float8E5M2FNUZ:
    परत Float8E5M2FNUZ();
  स्थिती S_Float8E4M3:
    परत Float8E4M3();
  स्थिती S_Float8E4M3FN:
    परत Float8E4M3FN();
  स्थिती S_Float8E4M3FNUZ:
    परत Float8E4M3FNUZ();
  स्थिती S_Float8E4M3B11FNUZ:
    परत Float8E4M3B11FNUZ();
  स्थिती S_Float8E3M4:
    परत Float8E3M4();
  स्थिती S_FloatTF32:
    परत FloatTF32();
  स्थिती S_Float8E8M0FNU:
    परत Float8E8M0FNU();
  स्थिती S_Float8E5M3FNU:
    परत Float8E5M3FNU();
  स्थिती S_Float6E3M2FN:
    परत Float6E3M2FN();
  स्थिती S_Float6E2M3FN:
    परत Float6E2M3FN();
  स्थिती S_Float4E2M1FN:
    परत Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloatBase::Semantics
APFloatBase::SemanticsToEnum(स्थिरांक llvm::fltSemantics &Sem) {
  जर (&Sem == &llvm::APFloat::IEEEhalf())
    परत S_IEEEhalf;
  अन्यथा जर (&Sem == &llvm::APFloat::BFloat())
    परत S_BFloat;
  अन्यथा जर (&Sem == &llvm::APFloat::IEEEsingle())
    परत S_IEEEsingle;
  अन्यथा जर (&Sem == &llvm::APFloat::IEEEdouble())
    परत S_IEEEdouble;
  अन्यथा जर (&Sem == &llvm::APFloat::IEEEquad())
    परत S_IEEEquad;
  अन्यथा जर (&Sem == &llvm::APFloat::x87DoubleExtended())
    परत S_x87DoubleExtended;
  अन्यथा जर (&Sem == &llvm::APFloat::PPCDoubleDouble())
    परत S_PPCDoubleDouble;
  अन्यथा जर (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    परत S_PPCDoubleDoubleLegacy;
  अन्यथा जर (&Sem == &llvm::APFloat::Float8E5M2())
    परत S_Float8E5M2;
  अन्यथा जर (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    परत S_Float8E5M2FNUZ;
  अन्यथा जर (&Sem == &llvm::APFloat::Float8E4M3())
    परत S_Float8E4M3;
  अन्यथा जर (&Sem == &llvm::APFloat::Float8E4M3FN())
    परत S_Float8E4M3FN;
  अन्यथा जर (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    परत S_Float8E4M3FNUZ;
  अन्यथा जर (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    परत S_Float8E4M3B11FNUZ;
  अन्यथा जर (&Sem == &llvm::APFloat::Float8E3M4())
    परत S_Float8E3M4;
  अन्यथा जर (&Sem == &llvm::APFloat::FloatTF32())
    परत S_FloatTF32;
  अन्यथा जर (&Sem == &llvm::APFloat::Float8E8M0FNU())
    परत S_Float8E8M0FNU;
  अन्यथा जर (&Sem == &llvm::APFloat::Float8E5M3FNU())
    परत S_Float8E5M3FNU;
  अन्यथा जर (&Sem == &llvm::APFloat::Float6E3M2FN())
    परत S_Float6E3M2FN;
  अन्यथा जर (&Sem == &llvm::APFloat::Float6E2M3FN())
    परत S_Float6E2M3FN;
  अन्यथा जर (&Sem == &llvm::APFloat::Float4E2M1FN())
    परत S_Float4E2M1FN;
  अन्यथा
    llvm_unreachable("Unknown floating semantics");
}

तार्किक APFloatBase::isRepresentableBy(स्थिरांक fltSemantics &A,
                                    स्थिरांक fltSemantics &B) {
  परत A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

तार्किक APFloatBase::isLosslesslyConvertibleTo(स्थिरांक fltSemantics &From,
                                            स्थिरांक fltSemantics &InitialTo,
                                            तार्किक IgnoreNaNs) {
  जर (&From == &InitialTo)
    परत सत्य;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  जर (&From == &semPPCDoubleDouble)
    परत असत्य;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  स्थिरांक fltSemantics &To =
      &InitialTo == &semPPCDoubleDouble ? semIEEEdouble : InitialTo;

  जर (!isRepresentableBy(From, To))
    परत असत्य;

  जर ((From.hasZero && !To.hasZero) ||
      (From.hasSignedRepr && !To.hasSignedRepr))
    परत असत्य;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  तार्किक FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  तार्किक ToHasSignedZero = To.hasZero && To.hasSignedRepr &&
                         To.nanEncoding != fltNanEncoding::NegativeZero;
  जर (FromHasSignedZero && !ToHasSignedZero)
    परत असत्य;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat SmallestFrom = APFloat::getSmallest(From);
  तार्किक LosesInfo = असत्य;
  (शून्य)SmallestFrom.convert(To, APFloat::rmNearestTiesToEven, &LosesInfo);
  जर (LosesInfo)
    परत असत्य;

  जर (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    परत सत्य;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  जर (IgnoreNaNs) {
    जर (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      परत To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    परत To.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  जर (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    परत असत्य;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  जर (To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    परत सत्य;
  परत To.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
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
[[maybe_unused]] constexpr अचिन्हित maxExponent = 16383;

// Decimal exponents in the f16 and f32 ranges need at most 2 and 6 parts.
// Keep 16 parts (128 bytes) inline: enough for 5^413, but not a large stack
// frame. Longer fractional spellings use heap storage.
स्थिर constexpr अचिन्हित अंक_Pow5_Parts_On_चळत = 16;

स्थिर अचिन्हित पूर्णांक powerOfFivePartsForExponent(अचिन्हित पूर्णांक power) {
  प्रतिज्ञा(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  परत 2 + ((power * 815) / (351 * APFloatBase::integerPartWidth));
}

अचिन्हित पूर्णांक APFloatBase::semanticsPrecision(स्थिरांक fltSemantics &semantics) {
  परत semantics.precision;
}
APFloatBase::ExponentType
APFloatBase::semanticsMaxExponent(स्थिरांक fltSemantics &semantics) {
  परत semantics.maxExponent;
}
APFloatBase::ExponentType
APFloatBase::semanticsMinExponent(स्थिरांक fltSemantics &semantics) {
  परत semantics.minExponent;
}
अचिन्हित पूर्णांक APFloatBase::semanticsSizeInBits(स्थिरांक fltSemantics &semantics) {
  परत semantics.sizeInBits;
}
अचिन्हित पूर्णांक APFloatBase::semanticsIntSizeInBits(स्थिरांक fltSemantics &semantics,
                                                 तार्किक isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  अचिन्हित पूर्णांक MinBitWidth = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  जर (isSigned)
    ++MinBitWidth;
  परत MinBitWidth;
}

तार्किक APFloatBase::semanticsHasZero(स्थिरांक fltSemantics &semantics) {
  परत semantics.hasZero;
}

तार्किक APFloatBase::semanticsHasSignedRepr(स्थिरांक fltSemantics &semantics) {
  परत semantics.hasSignedRepr;
}

तार्किक APFloatBase::semanticsHasInf(स्थिरांक fltSemantics &semantics) {
  परत semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

तार्किक APFloatBase::semanticsHasNaN(स्थिरांक fltSemantics &semantics) {
  परत semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

तार्किक APFloatBase::isIEEELikeFP(स्थिरांक fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  परत SemanticsToEnum(semantics) <= S_IEEEquad;
}

तार्किक APFloatBase::hasSignBitInMSB(स्थिरांक fltSemantics &semantics) {
  परत semantics.hasSignBitInMSB;
}

तार्किक APFloatBase::isRepresentableAsNormalIn(स्थिरांक fltSemantics &Src,
                                            स्थिरांक fltSemantics &Dst) {
  // Exponent range must be larger.
  जर (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    परत असत्य;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  परत Dst.precision >= Src.precision;
}

अचिन्हित APFloatBase::getSizeInBits(स्थिरांक fltSemantics &Sem) {
  परत Sem.sizeInBits;
}

स्थिर constexpr APFloatBase::ExponentType
exponentZero(स्थिरांक fltSemantics &semantics) {
  परत semantics.minExponent - 1;
}

स्थिर constexpr APFloatBase::ExponentType
exponentInf(स्थिरांक fltSemantics &semantics) {
  परत semantics.maxExponent + 1;
}

स्थिर constexpr APFloatBase::ExponentType
exponentNaN(स्थिरांक fltSemantics &semantics) {
  जर (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    जर (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      परत exponentZero(semantics);
    जर (semantics.hasSignedRepr || semantics.precision > 1)
      परत semantics.maxExponent;
  }
  परत semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

स्थिर ओळीत Error createError(स्थिरांक Twine &Err) {
  परत make_error<StringError>(Err, inconvertibleErrorCode());
}

स्थिर constexpr ओळीत अचिन्हित पूर्णांक part_मोजणी_For_Bits(अचिन्हित पूर्णांक bits) {
  परत std::max(1u, (bits + APFloatBase::integerPartWidth - 1) /
                          APFloatBase::integerPartWidth);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
स्थिर ओळीत अचिन्हित पूर्णांक
dec_Digit_मूल्य(अचिन्हित पूर्णांक c)
{
  परत c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
स्थिर Expected<पूर्णांक> readExponent(StringRef::iterator begin,
                                  StringRef::iterator end) {
  स्थिरांक अचिन्हित पूर्णांक overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  जर (p == end || ((*p == '-' || *p == '+') && (p + 1) == end))
    परत 0;

  तार्किक isNegative = *p == '-';
  जर (*p == '-' || *p == '+') {
    p++;
    जर (p == end)
      परत createError("Exponent has no digits");
  }

  अचिन्हित absExponent = dec_Digit_मूल्य(*p++);
  जर (absExponent >= 10U)
    परत createError("Invalid character in exponent");

  साठी (; p != end; ++p) {
    अचिन्हित मूल्य_2 = dec_Digit_मूल्य(*p);
    जर (मूल्य_2 >= 10U)
      परत createError("Invalid character in exponent");

    absExponent = absExponent * 10U + मूल्य_2;
    जर (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      थांबा;
    }
  }

  जर (isNegative)
    परत -(पूर्णांक) absExponent;
  अन्यथा
    परत (पूर्णांक) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
स्थिर Expected<पूर्णांक> totalExponent(StringRef::iterator p,
                                   StringRef::iterator end,
                                   पूर्णांक exponentAdjustment) {
  पूर्णांक exponent = 0;

  जर (p == end)
    परत createError("Exponent has no digits");

  तार्किक negative = *p == '-';
  जर (*p == '-' || *p == '+') {
    p++;
    जर (p == end)
      परत createError("Exponent has no digits");
  }

  पूर्णांक unsignedExponent = 0;
  तार्किक overflow = असत्य;
  साठी (; p != end; ++p) {
    अचिन्हित पूर्णांक मूल्य_2;

    मूल्य_2 = dec_Digit_मूल्य(*p);
    जर (मूल्य_2 >= 10U)
      परत createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + मूल्य_2;
    जर (unsignedExponent > 32767) {
      overflow = सत्य;
      थांबा;
    }
  }

  जर (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    overflow = सत्य;

  जर (!overflow) {
    exponent = unsignedExponent;
    जर (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    जर (exponent > 32767 || exponent < -32768)
      overflow = सत्य;
  }

  जर (overflow)
    exponent = negative ? -32768: 32767;

  परत exponent;
}

स्थिर Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator end,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = end;
  पर्यंत (p != end && *p == '0')
    p++;

  जर (p != end && *p == '.') {
    *dot = p++;

    जर (end - begin == 1)
      परत createError("Significand has no digits");

    पर्यंत (p != end && *p == '0')
      p++;
  }

  परत p;
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
  स्थिरांक अक्षर *firstSigDigit;
  स्थिरांक अक्षर *lastSigDigit;
  पूर्णांक exponent;
  पूर्णांक normalizedExponent;
};

स्थिर Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator end, decimalInfo *D) {
  StringRef::iterator dot = end;

  स्वतः PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  जर (!PtrOrErr)
    परत PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->firstSigDigit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  साठी (; p != end; ++p) {
    जर (*p == '.') {
      जर (dot != end)
        परत createError("String contains multiple dots");
      dot = p++;
      जर (p == end)
        थांबा;
    }
    जर (dec_Digit_मूल्य(*p) >= 10U)
      थांबा;
  }

  जर (p != end) {
    जर (*p != 'e' && *p != 'E')
      परत createError("Invalid character in significand");
    जर (p == begin)
      परत createError("Significand has no digits");
    जर (dot != end && p - begin == 1)
      परत createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    स्वतः ExpOrErr = readExponent(p + 1, end);
    जर (!ExpOrErr)
      परत ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    जर (dot == end)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  जर (p != D->firstSigDigit) {
    /* Drop insignificant trailing zeroes.  */
    जर (p != begin) {
      करा
        करा
          p--;
        पर्यंत (p != begin && *p == '0');
      पर्यंत (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->firstSigDigit)
                                      - (dot > D->firstSigDigit && dot < p)));
  }

  D->lastSigDigit = p;
  परत Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
स्थिर Expected<lostFraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator end,
                            अचिन्हित पूर्णांक digit_मूल्य) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  जर (digit_मूल्य > 8)
    परत lfMoreThanHalf;
  अन्यथा जर (digit_मूल्य < 8 && digit_मूल्य > 0)
    परत lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  पर्यंत (p != end && (*p == '0' || *p == '.'))
    p++;

  जर (p == end)
    परत createError("Invalid trailing hexadecimal fraction!");

  अचिन्हित hexDigit = hex_Digit_मूल्य(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  जर (hexDigit == अच_पूर्णांक_उच्च)
    परत digit_मूल्य == 0 ? lfExactlyZero: lfExactlyHalf;
  अन्यथा
    परत digit_मूल्य == 0 ? lfLessThanHalf: lfMoreThanHalf;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
स्थिर lostFraction
lostFractionThroughTruncation(स्थिरांक APFloatBase::integerPart *parts,
                              अचिन्हित पूर्णांक part_मोजणी,
                              अचिन्हित पूर्णांक bits)
{
  अचिन्हित lsb = APInt::tcLSB(parts, part_मोजणी);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  जर (bits <= lsb)
    परत lfExactlyZero;
  जर (bits == lsb + 1)
    परत lfExactlyHalf;
  जर (bits <= part_मोजणी * APFloatBase::integerPartWidth &&
      APInt::tcExtractBit(parts, bits - 1))
    परत lfMoreThanHalf;

  परत lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
स्थिर lostFraction
shift_उजवा(APFloatBase::integerPart *dst, अचिन्हित पूर्णांक parts, अचिन्हित पूर्णांक bits)
{
  lostFraction lost_fraction = lostFractionThroughTruncation(dst, parts, bits);

  APInt::tc_Shift_उजवा(dst, parts, bits);

  परत lost_fraction;
}

/* Combine the effect of two lost fractions.  */
स्थिर lostFraction
combineLostFractions(lostFraction moreSignificant,
                     lostFraction lessSignificant)
{
  जर (lessSignificant != lfExactlyZero) {
    जर (moreSignificant == lfExactlyZero)
      moreSignificant = lfLessThanHalf;
    अन्यथा जर (moreSignificant == lfExactlyHalf)
      moreSignificant = lfMoreThanHalf;
  }

  परत moreSignificant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
स्थिर अचिन्हित पूर्णांक
HUerrBound(तार्किक inexactMultiply, अचिन्हित पूर्णांक HUerr1, अचिन्हित पूर्णांक HUerr2)
{
  प्रतिज्ञा(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  जर (HUerr1 + HUerr2 == 0)
    परत inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  अन्यथा
    परत inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
स्थिर APFloatBase::integerPart
ulpsFromBoundary(स्थिरांक APFloatBase::integerPart *parts, अचिन्हित पूर्णांक bits,
                 तार्किक isNearest) {
  प्रतिज्ञा(bits != 0);

  bits--;
  अचिन्हित मोजणी = bits / APFloatBase::integerPartWidth;
  अचिन्हित partBits = bits % APFloatBase::integerPartWidth + 1;

  APFloatBase::integerPart part =
      parts[मोजणी] & (~(APFloatBase::integerPart)0 >>
                      (APFloatBase::integerPartWidth - partBits));

  APFloatBase::integerPart boundary;
  जर (isNearest)
    boundary = (APFloatBase::integerPart) 1 << (partBits - 1);
  अन्यथा
    boundary = 0;

  जर (मोजणी == 0) {
    जर (part - boundary <= boundary - part)
      परत part - boundary;
    अन्यथा
      परत boundary - part;
  }

  जर (part == boundary) {
    पर्यंत (--मोजणी)
      जर (parts[मोजणी])
        परत ~(APFloatBase::integerPart) 0; /* A lot.  */

    परत parts[0];
  } अन्यथा जर (part == boundary - 1) {
    पर्यंत (--मोजणी)
      जर (~parts[मोजणी])
        परत ~(APFloatBase::integerPart) 0; /* A lot.  */

    परत -parts[0];
  }

  परत ~(APFloatBase::integerPart) 0; /* A lot.  */
}

स्थिर constexpr अचिन्हित PowerOfFivePartCounts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

स्थिर constexpr APFloatBase::integerPart PowerOfFiveParts[] = {
#अन्तर्भाव "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  अचिन्हित Sum = 0;
  साठी (अचिन्हित मोजणी_2 : PowerOfFivePartCounts)
    Sum += मोजणी_2;
  परत Sum;
}() == std::size(PowerOfFiveParts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
स्थिर अचिन्हित पूर्णांक
powerOf5(APFloatBase::integerPart *dst, अचिन्हित पूर्णांक power) {
  स्थिर स्थिरांक APFloatBase::integerPart firstEightPowers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  SmallVector<APFloatBase::integerPart, अंक_Pow5_Parts_On_चळत> scratch;
  scratch.resize_for_overwrite(powerOfFivePartsForExponent(power));

  APFloatBase::integerPart *p1, *p2;
  स्थिरांक APFloatBase::integerPart *pow5;

  p1 = dst;
  p2 = scratch.data();

  *p1 = firstEightPowers[power & 7];
  power >>= 3;

  अचिन्हित result = 1;
  pow5 = PowerOfFiveParts;

  साठी (अचिन्हित पूर्णांक n = 0; power; power >>= 1, n++) {
    अचिन्हित parts_मोजणी = PowerOfFivePartCounts[n];

    जर (power & 1) {
      APFloatBase::integerPart *tmp;

      APInt::tcFullMultiply(p2, p1, pow5, result, parts_मोजणी);
      result += parts_मोजणी;
      जर (p2[result - 1] == 0)
        result--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_मोजणी;
  }

  जर (p1 != dst)
    APInt::tcAssign(dst, p1, result);

  परत result;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
स्थिर स्थिरांक अक्षर hexDigitsLower[] = "0123456789abcdef0";
स्थिर स्थिरांक अक्षर hexDigitsUpper[] = "0123456789ABCDEF0";
स्थिर स्थिरांक अक्षर infinityL[] = "infinity";
स्थिर स्थिरांक अक्षर infinityU[] = "INFINITY";
स्थिर स्थिरांक अक्षर NaNL[] = "nan";
स्थिर स्थिरांक अक्षर NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
स्थिर अचिन्हित पूर्णांक
partAsHex (अक्षर *dst, APFloatBase::integerPart part, अचिन्हित पूर्णांक मोजणी,
           स्थिरांक अक्षर *hexDigitChars)
{
  अचिन्हित पूर्णांक result = मोजणी;

  प्रतिज्ञा(मोजणी != 0 && मोजणी <= APFloatBase::integerPartWidth / 4);

  part >>= (APFloatBase::integerPartWidth - 4 * मोजणी);
  पर्यंत (मोजणी--) {
    dst[मोजणी] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  परत result;
}

/* Write out an unsigned decimal integer.  */
स्थिर अक्षर *writeUnsignedDecimal(अक्षर *dst, अचिन्हित पूर्णांक n) {
  अक्षर buff[40], *p;

  p = buff;
  करा
    *p++ = '0' + n % 10;
  पर्यंत (n /= 10);

  करा
    *dst++ = *--p;
  पर्यंत (p != buff);

  परत dst;
}

/* Write out a signed decimal integer.  */
स्थिर अक्षर *writeSignedDecimal(अक्षर *dst, पूर्णांक मूल्य_2) {
  जर (मूल्य_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(अचिन्हित) मूल्य_2);
  } अन्यथा {
    dst = writeUnsignedDecimal(dst, मूल्य_2);
  }

  परत dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
स्थिर APFloat harrisonUlp(स्थिरांक APFloat &X) {
  स्थिरांक fltSemantics &Sem = X.getSemantics();
  निवड (X.getCategory()) {
  स्थिती APFloat::fcNaN:
    परत APFloat::getQNaN(Sem);
  स्थिती APFloat::fcInfinity:
    परत APFloat::getInf(Sem);
  स्थिती APFloat::fcZero:
    परत APFloat::getSmallest(Sem);
  स्थिती APFloat::fcNormal:
    थांबा;
  }
  जर (X.isDenormal() || X.isSmallestNormalized())
    परत APFloat::getSmallest(Sem);
  पूर्णांक Exp = ilogb(X);
  जर (X.getExactLog2() != पूर्णांक_न्यून)
    Exp -= 1;
  परत scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rmNearestTiesToEven);
}

नामाकाश detail {
/* Constructors.  */
शून्य IEEEFloat::initialize(स्थिरांक fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  अचिन्हित मोजणी = part_मोजणी();
  जर (मोजणी > 1)
    significand.parts = नवीन integerPart[मोजणी];
}

शून्य IEEEFloat::freeSignificand() {
  जर (needsCleanup())
    delete [] significand.parts;
}

शून्य IEEEFloat::assign(स्थिरांक IEEEFloat &rhs) {
  प्रतिज्ञा(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  जर (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

शून्य IEEEFloat::copySignificand(स्थिरांक IEEEFloat &rhs) {
  प्रतिज्ञा(isFiniteNonZero() || category == fcNaN);
  प्रतिज्ञा(rhs.part_मोजणी() >= part_मोजणी());

  APInt::tcAssign(significandParts(), rhs.significandParts(),
                  part_मोजणी());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
शून्य IEEEFloat::makeNaN(तार्किक SNaN, तार्किक Negative, स्थिरांक APInt *fill) {
  जर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  जर (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  अचिन्हित अंक_Parts = part_मोजणी();

  APInt fill_storage;
  जर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = असत्य;
    जर (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = सत्य;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } अन्यथा {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  जर (!fill || fill->get_अंक_शब्द() < अंक_Parts)
    APInt::tcSet(significand, 0, अंक_Parts);
  जर (fill) {
    APInt::tcAssign(significand, fill->getRawData(),
                    std::min(fill->get_अंक_शब्द(), अंक_Parts));

    // Zero out the excess bits of the significand.
    अचिन्हित bitsToPreserve = semantics->precision - 1;
    अचिन्हित part = bitsToPreserve / 64;
    bitsToPreserve %= 64;
    significand[part] &= ((1ULL << bitsToPreserve) - 1);
    साठी (part++; part != अंक_Parts; ++part)
      significand[part] = 0;
  }

  अचिन्हित QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  जर (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tcClearBit(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    जर (APInt::tcIsZero(significand, अंक_Parts))
      APInt::tcSetBit(significand, QNaNBit - 1);
  } अन्यथा जर (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } अन्यथा {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tcSetBit(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  जर (semantics == &APFloatBase::semX87DoubleExtended)
    APInt::tcSetBit(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::चालक=(स्थिरांक IEEEFloat &rhs) {
  जर (हे != &rhs) {
    जर (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    assign(rhs);
  }

  परत *हे;
}

IEEEFloat &IEEEFloat::चालक=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloatBase::semBogus;
  परत *हे;
}

तार्किक IEEEFloat::isDenormal() स्थिरांक {
  परत getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tcExtractBit(significandParts(), semantics->precision - 1) ==
          0);
}

तार्किक IEEEFloat::isSmallest() स्थिरांक {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  परत isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

तार्किक IEEEFloat::isSmallestNormalized() स्थिरांक {
  परत getCategory() == fcNormal && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

अचिन्हित पूर्णांक IEEEFloat::get_अंक_वरचा_Bits() स्थिरांक {
  स्थिरांक अचिन्हित पूर्णांक Part_मोजणी = part_मोजणी_For_Bits(semantics->precision);
  स्थिरांक अचिन्हित पूर्णांक Bits = Part_मोजणी * integerPartWidth;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  स्थिरांक अचिन्हित पूर्णांक अंक_वरचा_Bits = (semantics->precision > 1)
                                       ? (Bits - semantics->precision + 1)
                                       : (Bits - semantics->precision);
  परत अंक_वरचा_Bits;
}

तार्किक IEEEFloat::isSignificandAllOnes() स्थिरांक {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  स्थिरांक integerPart *Parts = significandParts();
  स्थिरांक अचिन्हित Part_मोजणी = part_मोजणी_For_Bits(semantics->precision);
  साठी (अचिन्हित i = 0; i < Part_मोजणी - 1; i++)
    जर (~Parts[i])
      परत असत्य;

  // Set the unused high bits to all ones when we compare.
  स्थिरांक अचिन्हित अंक_वरचा_Bits = get_अंक_वरचा_Bits();
  प्रतिज्ञा(अंक_वरचा_Bits <= integerPartWidth && अंक_वरचा_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  स्थिरांक integerPart वरचा_Bit_Fill =
    ~integerPart(0) << (integerPartWidth - अंक_वरचा_Bits);
  जर ((semantics->precision <= 1) || (~(Parts[Part_मोजणी - 1] | वरचा_Bit_Fill)))
    परत असत्य;

  परत सत्य;
}

तार्किक IEEEFloat::isSignificandAllOnesExceptLSB() स्थिरांक {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  स्थिरांक integerPart *Parts = significandParts();

  जर (Parts[0] & 1)
    परत असत्य;

  स्थिरांक अचिन्हित Part_मोजणी = part_मोजणी_For_Bits(semantics->precision);
  साठी (अचिन्हित i = 0; i < Part_मोजणी - 1; i++) {
    जर (~Parts[i] & ~अचिन्हित{!i})
      परत असत्य;
  }

  // Set the unused high bits to all ones when we compare.
  स्थिरांक अचिन्हित अंक_वरचा_Bits = get_अंक_वरचा_Bits();
  प्रतिज्ञा(अंक_वरचा_Bits <= integerPartWidth && अंक_वरचा_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  स्थिरांक integerPart वरचा_Bit_Fill = ~integerPart(0)
                                  << (integerPartWidth - अंक_वरचा_Bits);
  जर (~(Parts[Part_मोजणी - 1] | वरचा_Bit_Fill | 0x1))
    परत असत्य;

  परत सत्य;
}

तार्किक IEEEFloat::isSignificandAllZeros() स्थिरांक {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  स्थिरांक integerPart *Parts = significandParts();
  स्थिरांक अचिन्हित Part_मोजणी = part_मोजणी_For_Bits(semantics->precision);

  साठी (अचिन्हित i = 0; i < Part_मोजणी - 1; i++)
    जर (Parts[i])
      परत असत्य;

  // Compute how many bits are used in the final word.
  स्थिरांक अचिन्हित अंक_वरचा_Bits = get_अंक_वरचा_Bits();
  प्रतिज्ञा(अंक_वरचा_Bits < integerPartWidth && "Can not have more high bits to "
         "clear than integerPartWidth");
  स्थिरांक integerPart वरचा_Bit_Mask = ~integerPart(0) >> अंक_वरचा_Bits;

  जर ((semantics->precision > 1) && (Parts[Part_मोजणी - 1] & वरचा_Bit_Mask))
    परत असत्य;

  परत सत्य;
}

तार्किक IEEEFloat::isSignificandAllZerosExceptMSB() स्थिरांक {
  स्थिरांक integerPart *Parts = significandParts();
  स्थिरांक अचिन्हित Part_मोजणी = part_मोजणी_For_Bits(semantics->precision);

  साठी (अचिन्हित i = 0; i < Part_मोजणी - 1; i++) {
    जर (Parts[i])
      परत असत्य;
  }

  स्थिरांक अचिन्हित अंक_वरचा_Bits = get_अंक_वरचा_Bits();
  स्थिरांक integerPart MSBMask = integerPart(1)
                              << (integerPartWidth - अंक_वरचा_Bits);
  परत ((semantics->precision <= 1) || (Parts[Part_मोजणी - 1] == MSBMask));
}

तार्किक IEEEFloat::isLargest() स्थिरांक {
  तार्किक IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  जर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    परत (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } अन्यथा {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    परत IsMaxExp && isSignificandAllOnes();
  }
}

तार्किक IEEEFloat::isInteger() स्थिरांक {
  // This could be made more efficient; I'm going for obviously correct.
  जर (!isFinite()) परत असत्य;
  IEEEFloat truncated = *हे;
  truncated.roundToIntegral(rmTowardZero);
  परत compare(truncated) == cmpEqual;
}

तार्किक IEEEFloat::bitwiseIsEqual(स्थिरांक IEEEFloat &rhs) स्थिरांक {
  जर (हे == &rhs)
    परत सत्य;
  जर (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    परत असत्य;
  जर (category==fcZero || category==fcInfinity)
    परत सत्य;

  जर (isFiniteNonZero() && exponent != rhs.exponent)
    परत असत्य;

  परत std::equal(significandParts(), significandParts() + part_मोजणी(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(स्थिरांक fltSemantics &ourSemantics, integerPart मूल्य_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fcNormal;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = मूल्य_2;
  normalize(rmNearestTiesToEven, lfExactlyZero);
}

IEEEFloat::IEEEFloat(स्थिरांक fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(असत्य) : makeSmallestNormalized(असत्य);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(स्थिरांक fltSemantics &ourSemantics, uninitializedTag tag)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(स्थिरांक IEEEFloat &rhs) {
  initialize(rhs.semantics);
  assign(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloatBase::semBogus) {
  *हे = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

अचिन्हित पूर्णांक IEEEFloat::part_मोजणी() स्थिरांक {
  परत part_मोजणी_For_Bits(semantics->precision + 1);
}

स्थिरांक APFloat::integerPart *IEEEFloat::significandParts() स्थिरांक {
  परत const_cast<IEEEFloat *>(हे)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  जर (part_मोजणी() > 1)
    परत significand.parts;
  अन्यथा
    परत &significand.part;
}

शून्य IEEEFloat::zeroSignificand() {
  APInt::tcSet(significandParts(), 0, part_मोजणी());
}

/* Increment an fcNormal floating point number's significand.  */
शून्य IEEEFloat::incrementSignificand() {
  [[maybe_unused]] integerPart carry =
      APInt::tcIncrement(significandParts(), part_मोजणी());

  /* Our callers should never cause us to overflow.  */
  प्रतिज्ञा(carry == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::addSignificand(स्थिरांक IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  प्रतिज्ञा(semantics == rhs.semantics);
  प्रतिज्ञा(exponent == rhs.exponent);

  परत APInt::tcAdd(parts, rhs.significandParts(), 0, part_मोजणी());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::subtractSignificand(स्थिरांक IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  प्रतिज्ञा(semantics == rhs.semantics);
  प्रतिज्ञा(exponent == rhs.exponent);

  परत APInt::tcSubtract(parts, rhs.significandParts(), borrow,
                           part_मोजणी());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
lostFraction IEEEFloat::multiplySignificand(स्थिरांक IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            तार्किक ignoreAddend) {
  integerPart scratch[4];
  तार्किक ignored;

  प्रतिज्ञा(semantics == rhs.semantics);

  अचिन्हित precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  अचिन्हित new_Parts_मोजणी = part_मोजणी_For_Bits(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *fullSignificand =
      new_Parts_मोजणी > 4 ? नवीन integerPart[new_Parts_मोजणी] : scratch;

  integerPart *lhsSignificand = significandParts();
  अचिन्हित parts_मोजणी = part_मोजणी();

  APInt::tcFullMultiply(fullSignificand, lhsSignificand,
                        rhs.significandParts(), parts_मोजणी, parts_मोजणी);

  lostFraction lost_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  अचिन्हित omsb = APInt::tcMSB(fullSignificand, new_Parts_मोजणी) + 1;
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

  जर (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    स्थिरांक fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    अचिन्हित extendedPrecision = 2 * precision + 1;
    जर (omsb != extendedPrecision - 1) {
      प्रतिज्ञा(extendedPrecision > omsb);
      APInt::tc_Shift_डावा(fullSignificand, new_Parts_मोजणी,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    जर (new_Parts_मोजणी == 1)
      significand.part = fullSignificand[0];
    अन्यथा
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
    lost_fraction = extendedAddend.shift_Significand_उजवा(1);
    प्रतिज्ञा(lost_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    lost_fraction = addOrSubtractSignificand(extendedAddend, असत्य);

    /* Restore our state.  */
    जर (new_Parts_मोजणी == 1)
      fullSignificand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(fullSignificand, new_Parts_मोजणी) + 1;
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
  जर (omsb > precision) {
    अचिन्हित पूर्णांक bits, significantParts;
    lostFraction lf;

    bits = omsb - precision;
    significantParts = part_मोजणी_For_Bits(omsb);
    lf = shift_उजवा(fullSignificand, significantParts, bits);
    lost_fraction = combineLostFractions(lf, lost_fraction);
    exponent += bits;
  }

  APInt::tcAssign(lhsSignificand, fullSignificand, parts_मोजणी);

  जर (new_Parts_मोजणी > 4)
    delete [] fullSignificand;

  परत lost_fraction;
}

lostFraction IEEEFloat::multiplySignificand(स्थिरांक IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  परत multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
lostFraction IEEEFloat::divideSignificand(स्थिरांक IEEEFloat &rhs) {
  integerPart scratch[4];

  प्रतिज्ञा(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  स्थिरांक integerPart *rhsSignificand = rhs.significandParts();
  अचिन्हित parts_मोजणी = part_मोजणी();

  integerPart *dividend =
      parts_मोजणी > 2 ? नवीन integerPart[parts_मोजणी * 2] : scratch;
  integerPart *भाजक = dividend + parts_मोजणी;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  साठी (अचिन्हित i = 0; i < parts_मोजणी; i++) {
    dividend[i] = lhsSignificand[i];
    भाजक[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  अचिन्हित पूर्णांक precision = semantics->precision;

  /* Normalize the divisor.  */
  अचिन्हित bit = precision - APInt::tcMSB(भाजक, parts_मोजणी) - 1;
  जर (bit) {
    exponent += bit;
    APInt::tc_Shift_डावा(भाजक, parts_मोजणी, bit);
  }

  /* Normalize the dividend.  */
  bit = precision - APInt::tcMSB(dividend, parts_मोजणी) - 1;
  जर (bit) {
    exponent -= bit;
    APInt::tc_Shift_डावा(dividend, parts_मोजणी, bit);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  जर (APInt::tcCompare(dividend, भाजक, parts_मोजणी) < 0) {
    exponent--;
    APInt::tc_Shift_डावा(dividend, parts_मोजणी, 1);
    प्रतिज्ञा(APInt::tcCompare(dividend, भाजक, parts_मोजणी) >= 0);
  }

  /* Long division.  */
  साठी (bit = precision; bit; bit -= 1) {
    जर (APInt::tcCompare(dividend, भाजक, parts_मोजणी) >= 0) {
      APInt::tcSubtract(dividend, भाजक, 0, parts_मोजणी);
      APInt::tcSetBit(lhsSignificand, bit - 1);
    }

    APInt::tc_Shift_डावा(dividend, parts_मोजणी, 1);
  }

  /* Figure out the lost fraction.  */
  पूर्णांक cmp = APInt::tcCompare(dividend, भाजक, parts_मोजणी);

  lostFraction lost_fraction;
  जर (cmp > 0)
    lost_fraction = lfMoreThanHalf;
  अन्यथा जर (cmp == 0)
    lost_fraction = lfExactlyHalf;
  अन्यथा जर (APInt::tcIsZero(dividend, parts_मोजणी))
    lost_fraction = lfExactlyZero;
  अन्यथा
    lost_fraction = lfLessThanHalf;

  जर (parts_मोजणी > 2)
    delete [] dividend;

  परत lost_fraction;
}

अचिन्हित पूर्णांक IEEEFloat::significandMSB() स्थिरांक {
  परत APInt::tcMSB(significandParts(), part_मोजणी());
}

अचिन्हित पूर्णांक IEEEFloat::significandLSB() स्थिरांक {
  परत APInt::tcLSB(significandParts(), part_मोजणी());
}

/* Note that a zero result is NOT normalized to fcZero.  */
lostFraction IEEEFloat::shift_Significand_उजवा(अचिन्हित पूर्णांक bits) {
  /* Our exponent should not overflow.  */
  प्रतिज्ञा((ExponentType) (exponent + bits) >= exponent);

  exponent += bits;

  परत shift_उजवा(significandParts(), part_मोजणी(), bits);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
शून्य IEEEFloat::shift_Significand_डावा(अचिन्हित पूर्णांक bits) {
  प्रतिज्ञा(bits < semantics->precision ||
         (semantics->precision == 1 && bits <= 1));

  जर (bits) {
    अचिन्हित पूर्णांक parts_मोजणी = part_मोजणी();

    APInt::tc_Shift_डावा(significandParts(), parts_मोजणी, bits);
    exponent -= bits;

    प्रतिज्ञा(!APInt::tcIsZero(significandParts(), parts_मोजणी));
  }
}

APFloat::cmpResult IEEEFloat::compare_Absolute_मूल्य(स्थिरांक IEEEFloat &rhs) स्थिरांक {
  प्रतिज्ञा(semantics == rhs.semantics);
  प्रतिज्ञा(isFiniteNonZero());
  प्रतिज्ञा(rhs.isFiniteNonZero());

  पूर्णांक compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  जर (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_मोजणी());

  जर (compare > 0)
    परत cmpGreaterThan;
  अन्यथा जर (compare < 0)
    परत cmpLessThan;
  अन्यथा
    परत cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
स्थिर शून्य tcSetLeastSignificantBits(APInt::WordType *dst, अचिन्हित parts,
                                      अचिन्हित bits) {
  अचिन्हित i = 0;
  पर्यंत (bits > APInt::APINT_BITS_PER_WORD) {
    dst[i++] = ~(APInt::WordType)0;
    bits -= APInt::APINT_BITS_PER_WORD;
  }

  जर (bits)
    dst[i++] = ~(APInt::WordType)0 >> (APInt::APINT_BITS_PER_WORD - bits);

  पर्यंत (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::opStatus IEEEFloat::handleOverflow(roundingMode rounding_mode) {
  जर (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    जर (rounding_mode == rmNearestTiesToEven ||
        rounding_mode == rmNearestTiesToAway ||
        (rounding_mode == rmTowardPositive && !sign) ||
        (rounding_mode == rmTowardNegative && sign)) {
      जर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(असत्य, sign);
      अन्यथा
        category = fcInfinity;
      परत static_cast<opStatus>(opOverflow | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fcNormal;
  exponent = semantics->maxExponent;
  tcSetLeastSignificantBits(significandParts(), part_मोजणी(),
                            semantics->precision);
  जर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tcClearBit(significandParts(), 0);

  परत opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
तार्किक IEEEFloat::roundAwayFromZero(roundingMode rounding_mode,
                                  lostFraction lost_fraction,
                                  अचिन्हित पूर्णांक bit) स्थिरांक {
  /* NaNs and infinities should not have lost fractions.  */
  प्रतिज्ञा(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  प्रतिज्ञा(lost_fraction != lfExactlyZero);

  निवड (rounding_mode) {
  स्थिती rmNearestTiesToAway:
    परत lost_fraction == lfExactlyHalf || lost_fraction == lfMoreThanHalf;

  स्थिती rmNearestTiesToEven:
    जर (lost_fraction == lfMoreThanHalf)
      परत सत्य;

    /* Our zeroes don't have a significand to test.  */
    जर (lost_fraction == lfExactlyHalf && category != fcZero)
      परत APInt::tcExtractBit(significandParts(), bit);

    परत असत्य;

  स्थिती rmTowardZero:
    परत असत्य;

  स्थिती rmTowardPositive:
    परत !sign;

  स्थिती rmTowardNegative:
    परत sign;

  पूर्वनिर्धारित:
    थांबा;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::opStatus IEEEFloat::normalize(roundingMode rounding_mode,
                                       lostFraction lost_fraction) {
  जर (!isFiniteNonZero())
    परत opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  अचिन्हित omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  जर (omsb || lost_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    पूर्णांक exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    जर (exponent + exponentChange > semantics->maxExponent)
      परत handleOverflow(rounding_mode);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    जर (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    जर (exponentChange < 0) {
      प्रतिज्ञा(lost_fraction == lfExactlyZero);

      shift_Significand_डावा(-exponentChange);

      परत opOK;
    }

    जर (exponentChange > 0) {
      lostFraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = shift_Significand_उजवा(exponentChange);

      lost_fraction = combineLostFractions(lf, lost_fraction);

      /* Keep OMSB up-to-date.  */
      जर (omsb > (अचिन्हित) exponentChange)
        omsb -= exponentChange;
      अन्यथा
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  जर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    परत handleOverflow(rounding_mode);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  जर (lost_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    जर (omsb == 0) {
      category = fcZero;
      जर (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = असत्य;
      जर (!semantics->hasZero)
        makeSmallestNormalized(असत्य);
    }

    परत opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  जर (roundAwayFromZero(rounding_mode, lost_fraction, 0)) {
    जर (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    जर (omsb == (अचिन्हित) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      जर (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        परत handleOverflow(sign ? rmTowardNegative : rmTowardPositive);

      shift_Significand_उजवा(1);

      परत opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    जर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      परत handleOverflow(rounding_mode);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  जर (omsb == semantics->precision)
    परत opInexact;

  /* We have a non-zero denormal.  */
  प्रतिज्ञा(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  जर (omsb == 0) {
    category = fcZero;
    जर (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = असत्य;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    जर (!semantics->hasZero)
      makeSmallestNormalized(असत्य);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  परत (opStatus) (opUnderflow | opInexact);
}

APFloat::opStatus IEEEFloat::addOrSubtractSpecials(स्थिरांक IEEEFloat &rhs,
                                                   तार्किक subtract) {
  निवड (Pack_Categories_Into_किल्ली(category, rhs.category)) {
  पूर्वनिर्धारित:
    llvm_unreachable(nullptr);

  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcNaN):
  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcNaN):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcZero):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcNormal):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcNaN):
    जर (isSignaling()) {
      makeQuiet();
      परत opInvalidOp;
    }
    परत rhs.isSignaling() ? opInvalidOp : opOK;

  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcZero):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcNormal):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcZero):
    परत opOK;

  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ subtract;
    परत opOK;

  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcNormal):
    assign(rhs);
    sign = rhs.sign ^ subtract;
    परत opOK;

  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    परत opOK;

  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    जर (((sign ^ rhs.sign)!=0) != subtract) {
      makeNaN();
      परत opInvalidOp;
    }

    परत opOK;

  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcNormal):
    परत opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
lostFraction IEEEFloat::addOrSubtractSignificand(स्थिरांक IEEEFloat &rhs,
                                                 तार्किक subtract) {
  [[maybe_unused]] integerPart carry = 0;
  lostFraction lost_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  subtract ^= static_cast<तार्किक>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  पूर्णांक bits = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  जर (subtract) {
    जर ((bits < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat temp_rhs(rhs);
    तार्किक lost_fraction_is_from_rhs = असत्य;

    जर (bits == 0)
      lost_fraction = lfExactlyZero;
    अन्यथा जर (bits > 0) {
      lost_fraction = temp_rhs.shift_Significand_उजवा(bits - 1);
      lost_fraction_is_from_rhs = सत्य;
      shift_Significand_डावा(1);
    } अन्यथा {
      lost_fraction = shift_Significand_उजवा(-bits - 1);
      temp_rhs.shift_Significand_डावा(1);
    }

    // Should we reverse the subtraction.
    cmpResult cmp_result = compare_Absolute_मूल्य(temp_rhs);
    जर (cmp_result == cmpLessThan) {
      तार्किक borrow =
          lost_fraction != lfExactlyZero && !lost_fraction_is_from_rhs;
      जर (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        जर (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        अन्यथा जर (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = temp_rhs.subtractSignificand(*हे, borrow);
      copySignificand(temp_rhs);
      sign = !sign;
    } अन्यथा जर (cmp_result == cmpGreaterThan) {
      तार्किक borrow = lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs;
      जर (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        जर (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        अन्यथा जर (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = subtractSignificand(temp_rhs, borrow);
    } अन्यथा { // cmpEqual
      zeroSignificand();
      जर (lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    प्रतिज्ञा(!carry);
  } अन्यथा {
    जर (bits > 0) {
      IEEEFloat temp_rhs(rhs);

      lost_fraction = temp_rhs.shift_Significand_उजवा(bits);
      carry = addSignificand(temp_rhs);
    } अन्यथा {
      lost_fraction = shift_Significand_उजवा(-bits);
      carry = addSignificand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    प्रतिज्ञा(!carry);
  }

  परत lost_fraction;
}

APFloat::opStatus IEEEFloat::multiplySpecials(स्थिरांक IEEEFloat &rhs) {
  निवड (Pack_Categories_Into_किल्ली(category, rhs.category)) {
  पूर्वनिर्धारित:
    llvm_unreachable(nullptr);

  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcNaN):
  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcNaN):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcNaN):
    assign(rhs);
    sign = असत्य;
    [[fallthrough]];
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcZero):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcNormal):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    जर (isSignaling()) {
      makeQuiet();
      परत opInvalidOp;
    }
    परत rhs.isSignaling() ? opInvalidOp : opOK;

  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcNormal):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcInfinity):
    category = fcInfinity;
    परत opOK;

  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcNormal):
  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcZero):
  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcZero):
    category = fcZero;
    परत opOK;

  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcZero):
    makeNaN();
    परत opInvalidOp;

  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcNormal):
    परत opOK;
  }
}

APFloat::opStatus IEEEFloat::divideSpecials(स्थिरांक IEEEFloat &rhs) {
  निवड (Pack_Categories_Into_किल्ली(category, rhs.category)) {
  पूर्वनिर्धारित:
    llvm_unreachable(nullptr);

  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcNaN):
  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcNaN):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcNaN):
    assign(rhs);
    sign = असत्य;
    [[fallthrough]];
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcZero):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcNormal):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    जर (isSignaling()) {
      makeQuiet();
      परत opInvalidOp;
    }
    परत rhs.isSignaling() ? opInvalidOp : opOK;

  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcZero):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcNormal):
  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcNormal):
    परत opOK;

  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcInfinity):
    category = fcZero;
    परत opOK;

  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcZero):
    जर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(असत्य, sign);
    अन्यथा
      category = fcInfinity;
    परत opDivByZero;

  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcZero):
    makeNaN();
    परत opInvalidOp;

  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcNormal):
    परत opOK;
  }
}

APFloat::opStatus IEEEFloat::modSpecials(स्थिरांक IEEEFloat &rhs) {
  निवड (Pack_Categories_Into_किल्ली(category, rhs.category)) {
  पूर्वनिर्धारित:
    llvm_unreachable(nullptr);

  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcNaN):
  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcNaN):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcZero):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcNormal):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcNaN):
    जर (isSignaling()) {
      makeQuiet();
      परत opInvalidOp;
    }
    परत rhs.isSignaling() ? opInvalidOp : opOK;

  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcNormal):
  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcInfinity):
    परत opOK;

  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcZero):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcZero):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcNormal):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcZero):
    makeNaN();
    परत opInvalidOp;

  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcNormal):
    परत opOK;
  }
}

APFloat::opStatus IEEEFloat::remainderSpecials(स्थिरांक IEEEFloat &rhs) {
  निवड (Pack_Categories_Into_किल्ली(category, rhs.category)) {
  पूर्वनिर्धारित:
    llvm_unreachable(nullptr);

  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcNaN):
  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcNaN):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcZero):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcNormal):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcNaN):
    जर (isSignaling()) {
      makeQuiet();
      परत opInvalidOp;
    }
    परत rhs.isSignaling() ? opInvalidOp : opOK;

  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcNormal):
  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcInfinity):
    परत opOK;

  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcZero):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcZero):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcNormal):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcZero):
    makeNaN();
    परत opInvalidOp;

  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcNormal):
    परत opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
शून्य IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  जर (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    परत;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::opStatus IEEEFloat::addOrSubtract(स्थिरांक IEEEFloat &rhs,
                                           roundingMode rounding_mode,
                                           तार्किक subtract) {
  opStatus fs = addOrSubtractSpecials(rhs, subtract);

  /* This return code means it was not a simple case.  */
  जर (fs == opDivByZero) {
    lostFraction lost_fraction;

    lost_fraction = addOrSubtractSignificand(rhs, subtract);
    fs = normalize(rounding_mode, lost_fraction);

    /* Can only be zero if we lost no fraction.  */
    प्रतिज्ञा(category != fcZero || lost_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  जर (category == fcZero) {
    जर (rhs.category != fcZero || (sign == rhs.sign) == subtract)
      sign = (rounding_mode == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    जर (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = असत्य;
  }

  परत fs;
}

/* Normalized addition.  */
APFloat::opStatus IEEEFloat::add(स्थिरांक IEEEFloat &rhs,
                                 roundingMode rounding_mode) {
  परत addOrSubtract(rhs, rounding_mode, असत्य);
}

/* Normalized subtraction.  */
APFloat::opStatus IEEEFloat::subtract(स्थिरांक IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  परत addOrSubtract(rhs, rounding_mode, सत्य);
}

/* Normalized multiply.  */
APFloat::opStatus IEEEFloat::multiply(स्थिरांक IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = multiplySpecials(rhs);

  जर (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = असत्य;
  जर (isFiniteNonZero()) {
    lostFraction lost_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    जर (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  परत fs;
}

/* Normalized divide.  */
APFloat::opStatus IEEEFloat::divide(स्थिरांक IEEEFloat &rhs,
                                    roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = divideSpecials(rhs);

  जर (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = असत्य;
  जर (isFiniteNonZero()) {
    lostFraction lost_fraction = divideSignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    जर (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  परत fs;
}

/* Normalized remainder.  */
APFloat::opStatus IEEEFloat::remainder(स्थिरांक IEEEFloat &rhs) {
  अचिन्हित पूर्णांक origSign = sign;

  // First handle the special cases.
  opStatus fs = remainderSpecials(rhs);
  जर (fs != opDivByZero)
    परत fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  जर (P2.add(rhs, rmNearestTiesToEven) == opOK) {
    fs = mod(P2);
    प्रतिज्ञा(fs == opOK);
  }

  // Lets work with absolute numbers.
  IEEEFloat P = rhs;
  P.sign = असत्य;
  sign = असत्य;

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

  IEEEFloat VEx = *हे;
  fs = VEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  प्रतिज्ञा(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  प्रतिज्ञा(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.add(VEx, rmNearestTiesToEven);
  प्रतिज्ञा(fs == opOK);

  जर (VEx.compare(PEx) == cmpGreaterThan) {
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
    जर (result == cmpGreaterThan || result == cmpEqual) {
      fs = subtract(P, rmNearestTiesToEven);
      प्रतिज्ञा(fs == opOK);
    }
  }

  जर (isZero()) {
    sign = origSign;    // IEEE754 requires this
    जर (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = असत्य;
  } अन्यथा {
    sign ^= origSign;
  }
  परत fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::opStatus IEEEFloat::mod(स्थिरांक IEEEFloat &rhs) {
  opStatus fs = modSpecials(rhs);
  अचिन्हित पूर्णांक origSign = sign;

  पर्यंत (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_मूल्य(rhs) != cmpLessThan) {
    पूर्णांक Exp = ilogb(*हे) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rmNearestTiesToEven);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    जर (V.isNaN() || compare_Absolute_मूल्य(V) == cmpLessThan)
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
    जर (!semantics->hasZero && हे->isSmallest())
      थांबा;

    प्रतिज्ञा(fs==opOK);
  }
  जर (isZero()) {
    sign = origSign; // fmod requires this
    जर (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = असत्य;
  }
  परत fs;
}

/* Normalized fused-multiply-add.  */
APFloat::opStatus IEEEFloat::fusedMultiplyAdd(स्थिरांक IEEEFloat &multiplicand,
                                              स्थिरांक IEEEFloat &addend,
                                              roundingMode rounding_mode) {
  opStatus fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  जर (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    lostFraction lost_fraction;

    lost_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_mode, lost_fraction);
    जर (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    जर (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_mode == rmTowardNegative);
      जर (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = असत्य;
    }
  } अन्यथा {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    जर (fs == opOK)
      fs = addOrSubtract(addend, rounding_mode, असत्य);
  }

  परत fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::opStatus IEEEFloat::roundToIntegral(roundingMode rounding_mode) {
  जर (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    परत opOK;

  जर (isNaN()) {
    जर (isSignaling()) {
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
      परत opInvalidOp;
    } अन्यथा {
      // [IEEE Std 754-2008 6.2]:
      // For an operation with quiet NaN inputs, other than maximum and minimum
      // operations, if a floating-point result is to be delivered the result
      // shall be a quiet NaN which should be one of the input NaNs.
      // ...
      // Every general-computational and quiet-computational operation involving
      // one or more input NaNs, none of them signaling, shall signal no
      // exception, except fusedMultiplyAdd might signal the invalid operation
      // exception(see 7.2).
      परत opOK;
    }
  }

  जर (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    परत opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  जर (exponent + 1 >= (पूर्णांक)APFloat::semanticsPrecision(*semantics))
    परत opOK;

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
  opStatus fs = MagicConstant.convertFromAPInt(IntegerConstant, असत्य,
                                               rmNearestTiesToEven);
  प्रतिज्ञा(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  तार्किक आदान_Sign = isNegative();

  fs = add(MagicConstant, rounding_mode);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  subtract(MagicConstant, rounding_mode);

  // Restore the input sign.
  जर (आदान_Sign != isNegative())
    changeSign();

  परत fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmpResult IEEEFloat::compare(स्थिरांक IEEEFloat &rhs) स्थिरांक {
  प्रतिज्ञा(semantics == rhs.semantics);

  निवड (Pack_Categories_Into_किल्ली(category, rhs.category)) {
  पूर्वनिर्धारित:
    llvm_unreachable(nullptr);

  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcZero):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcNormal):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcNaN, fcNaN):
  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcNaN):
  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcNaN):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcNaN):
    परत cmpUnordered;

  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcNormal):
  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcZero):
  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcZero):
    जर (sign)
      परत cmpLessThan;
    अन्यथा
      परत cmpGreaterThan;

  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcInfinity):
  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcNormal):
    जर (rhs.sign)
      परत cmpGreaterThan;
    अन्यथा
      परत cmpLessThan;

  स्थिती Pack_Categories_Into_किल्ली(fcInfinity, fcInfinity):
    जर (sign == rhs.sign)
      परत cmpEqual;
    अन्यथा जर (sign)
      परत cmpLessThan;
    अन्यथा
      परत cmpGreaterThan;

  स्थिती Pack_Categories_Into_किल्ली(fcZero, fcZero):
    परत cmpEqual;

  स्थिती Pack_Categories_Into_किल्ली(fcNormal, fcNormal):
    थांबा;
  }

  cmpResult result;
  /* Two normal numbers.  Do they have the same sign?  */
  जर (sign != rhs.sign) {
    जर (sign)
      result = cmpLessThan;
    अन्यथा
      result = cmpGreaterThan;
  } अन्यथा {
    /* Compare absolute values; invert result if negative.  */
    result = compare_Absolute_मूल्य(rhs);

    जर (sign) {
      जर (result == cmpLessThan)
        result = cmpGreaterThan;
      अन्यथा जर (result == cmpGreaterThan)
        result = cmpLessThan;
    }
  }

  परत result;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::opStatus IEEEFloat::convert(स्थिरांक fltSemantics &toSemantics,
                                     roundingMode rounding_mode,
                                     तार्किक *losesInfo) {
  opStatus fs;
  स्थिरांक fltSemantics &fromSemantics = *semantics;
  तार्किक is_signaling = isSignaling();

  lostFraction lostFraction = lfExactlyZero;
  अचिन्हित new_Part_मोजणी = part_मोजणी_For_Bits(toSemantics.precision + 1);
  अचिन्हित old_Part_मोजणी = part_मोजणी();
  पूर्णांक shift = toSemantics.precision - fromSemantics.precision;

  तार्किक X86SpecialNan = असत्य;
  जर (&fromSemantics == &APFloatBase::semX87DoubleExtended &&
      &toSemantics != &APFloatBase::semX87DoubleExtended && category == fcNaN &&
      (!(*significandParts() & 0x8000000000000000ULL) ||
       !(*significandParts() & 0x4000000000000000ULL))) {
    // x86 has some unusual NaNs which cannot be represented in any other
    // format; note them here.
    X86SpecialNan = सत्य;
  }

  // If this is a truncation of a denormal number, and the target semantics
  // has larger exponent range than the source semantics (this can happen
  // when truncating from PowerPC double-double to double format), the
  // right shift could lose result mantissa bits.  Adjust exponent instead
  // of performing excessive shift.
  // Also do a similar trick in case shifting denormal would produce zero
  // significand as this case isn't handled correctly by normalize.
  जर (shift < 0 && isFiniteNonZero()) {
    पूर्णांक omsb = significandMSB() + 1;
    पूर्णांक exponentChange = omsb - fromSemantics.precision;
    जर (exponent + exponentChange < toSemantics.minExponent)
      exponentChange = toSemantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, shift);
    जर (exponentChange < 0) {
      shift -= exponentChange;
      exponent += exponentChange;
    } अन्यथा जर (omsb <= -shift) {
      exponentChange = omsb + shift - 1; // leave at least one bit set
      shift -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  जर (shift < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    lostFraction = shift_उजवा(significandParts(), old_Part_मोजणी, -shift);

  // Fix the storage so it can hold to new value.
  जर (new_Part_मोजणी > old_Part_मोजणी) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = नवीन integerPart[new_Part_मोजणी];
    APInt::tcSet(newParts, 0, new_Part_मोजणी);
    जर (isFiniteNonZero() || category==fcNaN)
      APInt::tcAssign(newParts, significandParts(), old_Part_मोजणी);
    freeSignificand();
    significand.parts = newParts;
  } अन्यथा जर (new_Part_मोजणी == 1 && old_Part_मोजणी != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    जर (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &toSemantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  जर (shift > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_Shift_डावा(significandParts(), new_Part_मोजणी, shift);

  जर (isFiniteNonZero()) {
    fs = normalize(rounding_mode, lostFraction);
    *losesInfo = (fs != opOK);
  } अन्यथा जर (category == fcNaN) {
    जर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(असत्य, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } अन्यथा {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      जर (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(असत्य, असत्य);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      जर (!APFloat::hasSignificand(fromSemantics))
        makeNaN(असत्य, sign);

      *losesInfo = lostFraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      जर (!X86SpecialNan && semantics == &APFloatBase::semX87DoubleExtended)
        APInt::tcSetBit(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      जर (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } अन्यथा {
        fs = opOK;
      }
    }
  } अन्यथा जर (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(असत्य, sign);
    *losesInfo = सत्य;
    fs = opInexact;
  } अन्यथा जर (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = असत्य;
  } अन्यथा {
    *losesInfo = असत्य;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  जर ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = सत्य;
    जर (fs == opOK)
      fs = opInexact;
  }

  जर (category == fcZero && !semantics->hasZero)
    makeSmallestNormalized(असत्य);
  परत fs;
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
    MutableArrayRef<integerPart> parts, अचिन्हित पूर्णांक width, तार्किक isSigned,
    roundingMode rounding_mode, तार्किक *isExact) स्थिरांक {
  *isExact = असत्य;

  /* Handle the three special cases first.  */
  जर (category == fcInfinity || category == fcNaN)
    परत opInvalidOp;

  अचिन्हित dst_Parts_मोजणी = part_मोजणी_For_Bits(width);
  प्रतिज्ञा(dst_Parts_मोजणी <= parts.size() && "Integer too big");

  जर (category == fcZero) {
    APInt::tcSet(parts.data(), 0, dst_Parts_मोजणी);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    परत opOK;
  }

  स्थिरांक integerPart *src = significandParts();

  अचिन्हित truncatedBits;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  जर (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tcSet(parts.data(), 0, dst_Parts_मोजणी);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncatedBits = semantics->precision -1U - exponent;
  } अन्यथा {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    अचिन्हित पूर्णांक bits = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    जर (bits > width)
      परत opInvalidOp;

    जर (bits < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncatedBits = semantics->precision - bits;
      APInt::tcExtract(parts.data(), dst_Parts_मोजणी, src, bits, truncatedBits);
    } अन्यथा {
      /* We want at least as many bits as are available.  */
      APInt::tcExtract(parts.data(), dst_Parts_मोजणी, src, semantics->precision,
                       0);
      APInt::tc_Shift_डावा(parts.data(), dst_Parts_मोजणी,
                         bits - semantics->precision);
      truncatedBits = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  lostFraction lost_fraction;
  जर (truncatedBits) {
    lost_fraction = lostFractionThroughTruncation(src, part_मोजणी(),
                                                  truncatedBits);
    जर (lost_fraction != lfExactlyZero &&
        roundAwayFromZero(rounding_mode, lost_fraction, truncatedBits)) {
      जर (APInt::tcIncrement(parts.data(), dst_Parts_मोजणी))
        परत opInvalidOp;     /* Overflow.  */
    }
  } अन्यथा {
    lost_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  अचिन्हित पूर्णांक omsb = APInt::tcMSB(parts.data(), dst_Parts_मोजणी) + 1;

  जर (sign) {
    जर (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      जर (omsb != 0)
        परत opInvalidOp;
    } अन्यथा {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      जर (omsb == width &&
          APInt::tcLSB(parts.data(), dst_Parts_मोजणी) + 1 != omsb)
        परत opInvalidOp;

      /* This case can happen because of rounding.  */
      जर (omsb > width)
        परत opInvalidOp;
    }

    APInt::tcNegate (parts.data(), dst_Parts_मोजणी);
  } अन्यथा {
    जर (omsb >= width + !isSigned)
      परत opInvalidOp;
  }

  जर (lost_fraction == lfExactlyZero) {
    *isExact = सत्य;
    परत opOK;
  }
  परत opInexact;
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
                            अचिन्हित पूर्णांक width, तार्किक isSigned,
                            roundingMode rounding_mode, तार्किक *isExact) स्थिरांक {
  opStatus fs = convertToSignExtendedInteger(parts, width, isSigned,
                                             rounding_mode, isExact);

  जर (fs == opInvalidOp) {
    अचिन्हित पूर्णांक bits, dst_Parts_मोजणी;

    dst_Parts_मोजणी = part_मोजणी_For_Bits(width);
    प्रतिज्ञा(dst_Parts_मोजणी <= parts.size() && "Integer too big");

    जर (category == fcNaN)
      bits = 0;
    अन्यथा जर (sign)
      bits = isSigned;
    अन्यथा
      bits = width - isSigned;

    tcSetLeastSignificantBits(parts.data(), dst_Parts_मोजणी, bits);
    जर (sign && isSigned)
      APInt::tc_Shift_डावा(parts.data(), dst_Parts_मोजणी, width - 1);
  }

  परत fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::opStatus IEEEFloat::convertFromUnsignedParts(
    स्थिरांक integerPart *src, अचिन्हित पूर्णांक src_मोजणी, roundingMode rounding_mode) {
  category = fcNormal;
  अचिन्हित omsb = APInt::tcMSB(src, src_मोजणी) + 1;
  integerPart *dst = significandParts();
  अचिन्हित dst_मोजणी = part_मोजणी();
  अचिन्हित precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  lostFraction lost_fraction;
  जर (precision <= omsb) {
    exponent = omsb - 1;
    lost_fraction = lostFractionThroughTruncation(src, src_मोजणी,
                                                  omsb - precision);
    APInt::tcExtract(dst, dst_मोजणी, src, precision, omsb - precision);
  } अन्यथा {
    exponent = precision - 1;
    lost_fraction = lfExactlyZero;
    APInt::tcExtract(dst, dst_मोजणी, src, omsb, 0);
  }

  परत normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus IEEEFloat::convertFromAPInt(स्थिरांक APInt &Val, तार्किक isSigned,
                                              roundingMode rounding_mode) {
  अचिन्हित पूर्णांक part_मोजणी = Val.get_अंक_शब्द();
  APInt api = Val;

  sign = असत्य;
  जर (isSigned && api.isNegative()) {
    sign = सत्य;
    api = -api;
  }

  परत convertFromUnsignedParts(api.getRawData(), part_मोजणी, rounding_mode);
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        roundingMode rounding_mode) {
  lostFraction lost_fraction = lfExactlyZero;

  category = fcNormal;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  अचिन्हित parts_मोजणी = part_मोजणी();
  अचिन्हित bitPos = parts_मोजणी * integerPartWidth;
  तार्किक computedTrailingFraction = असत्य;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator end = s.end();
  StringRef::iterator dot;
  स्वतः PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  जर (!PtrOrErr)
    परत PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator firstSignificantDigit = p;

  पर्यंत (p != end) {
    integerPart hex_मूल्य;

    जर (*p == '.') {
      जर (dot != end)
        परत createError("String contains multiple dots");
      dot = p++;
      सुरूठेवा;
    }

    hex_मूल्य = hex_Digit_मूल्य(*p);
    जर (hex_मूल्य == अच_पूर्णांक_उच्च)
      थांबा;

    p++;

    // Store the number while we have space.
    जर (bitPos) {
      bitPos -= 4;
      hex_मूल्य <<= bitPos % integerPartWidth;
      significand[bitPos / integerPartWidth] |= hex_मूल्य;
    } अन्यथा जर (!computedTrailingFraction) {
      स्वतः FractOrErr = trailingHexadecimalFraction(p, end, hex_मूल्य);
      जर (!FractOrErr)
        परत FractOrErr.takeError();
      lost_fraction = *FractOrErr;
      computedTrailingFraction = सत्य;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  जर (p == end)
    परत createError("Hex strings require an exponent");
  जर (*p != 'p' && *p != 'P')
    परत createError("Invalid character in significand");
  जर (p == begin)
    परत createError("Significand has no digits");
  जर (dot != end && p - begin == 1)
    परत createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  जर (p != firstSignificantDigit) {
    पूर्णांक expAdjustment;

    /* Implicit hexadecimal point?  */
    जर (dot == end)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<पूर्णांक>(dot - firstSignificantDigit);
    जर (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_मोजणी * integerPartWidth;

    /* Adjust for the given exponent.  */
    स्वतः ExpOrErr = totalExponent(p + 1, end, expAdjustment);
    जर (!ExpOrErr)
      परत ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  परत normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus
IEEEFloat::roundSignificandWithExponent(स्थिरांक integerPart *decSigParts,
                                        अचिन्हित sig_Part_मोजणी, पूर्णांक exp,
                                        roundingMode rounding_mode) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  अचिन्हित पूर्णांक power = exp >= 0 ? exp : -exp;
  अचिन्हित पूर्णांक pow5PartCapacity = powerOfFivePartsForExponent(power);
  SmallVector<integerPart, अंक_Pow5_Parts_On_चळत> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5PartCapacity);

  तार्किक isNearest = rounding_mode == rmNearestTiesToEven ||
                   rounding_mode == rmNearestTiesToAway;

  अचिन्हित parts = part_मोजणी_For_Bits(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  अचिन्हित pow5_Part_मोजणी = powerOf5(pow5Parts.data(), power);

  साठी (;; parts *= 2) {
    अचिन्हित पूर्णांक excessPrecision, truncatedBits;

    calcSemantics.precision = parts * integerPartWidth - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncatedBits = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    opStatus sigStatus = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_मोजणी, rmNearestTiesToEven);
    opStatus powStatus = pow5.convertFromUnsignedParts(
        pow5Parts.data(), pow5_Part_मोजणी, rmNearestTiesToEven);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += exp;

    lostFraction calcLostFraction;
    integerPart HUerr, HUdistance;
    अचिन्हित पूर्णांक powHUerr;

    जर (exp >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calcLostFraction = decSig.multiplySignificand(pow5);
      powHUerr = powStatus != opOK;
    } अन्यथा {
      calcLostFraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      जर (decSig.exponent < semantics->minExponent) {
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
    जर (HUdistance >= HUerr) {
      APInt::tcExtract(significandParts(), part_मोजणी(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calcLostFraction = lostFractionThroughTruncation(decSig.significandParts(),
                                                       decSig.part_मोजणी(),
                                                       truncatedBits);
      परत static_cast<opStatus>(normalize(rounding_mode, calcLostFraction) |
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
  जर (Error Err = interpretDecimal(p, str.end(), &D))
    परत std::move(Err);

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
  जर (D.firstSigDigit == str.end() || dec_Digit_मूल्य(*D.firstSigDigit) >= 10U) {
    category = fcZero;
    fs = opOK;
    जर (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = असत्य;
    जर (!semantics->hasZero)
      makeSmallestNormalized(असत्य);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } अन्यथा जर (D.normalizedExponent - 1 > पूर्णांक_उच्च / 42039) {
    fs = handleOverflow(rounding_mode);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } अन्यथा जर (D.normalizedExponent - 1 < पूर्णांक_न्यून / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (पूर्णांक) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fcNormal;
    zeroSignificand();
    fs = normalize(rounding_mode, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } अन्यथा जर ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handleOverflow(rounding_mode);
  } अन्यथा {
    integerPart *decSignificand;
    अचिन्हित पूर्णांक part_मोजणी;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_मोजणी = static_cast<अचिन्हित पूर्णांक>(D.lastSigDigit - D.firstSigDigit) + 1;
    part_मोजणी = part_मोजणी_For_Bits(1 + 196 * part_मोजणी / 59);
    decSignificand = नवीन integerPart[part_मोजणी + 1];
    part_मोजणी = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    करा {
      integerPart dec_मूल्य, val, multiplier;

      val = 0;
      multiplier = 1;

      करा {
        जर (*p == '.') {
          p++;
          जर (p == str.end()) {
            थांबा;
          }
        }
        dec_मूल्य = dec_Digit_मूल्य(*p++);
        जर (dec_मूल्य >= 10U) {
          delete[] decSignificand;
          परत createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_मूल्य;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } पर्यंत (p <= D.lastSigDigit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_मोजणी, part_मोजणी + 1, असत्य);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      जर (decSignificand[part_मोजणी])
        part_मोजणी++;
    } पर्यंत (p <= D.lastSigDigit);

    category = fcNormal;
    fs = roundSignificandWithExponent(decSignificand, part_मोजणी,
                                      D.exponent, rounding_mode);

    delete [] decSignificand;
  }

  परत fs;
}

तार्किक IEEEFloat::convertFromStringSpecials(StringRef str) {
  स्थिरांक माप_प्रकार MIN_NAME_SIZE = 3;

  जर (str.size() < MIN_NAME_SIZE)
    परत असत्य;

  जर (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(असत्य);
    परत सत्य;
  }

  तार्किक IsNegative = str.consume_front("-");
  जर (IsNegative) {
    जर (str.size() < MIN_NAME_SIZE)
      परत असत्य;

    जर (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(सत्य);
      परत सत्य;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  तार्किक IsSignaling = str.consume_front_insensitive("s");
  जर (IsSignaling) {
    जर (str.size() < MIN_NAME_SIZE)
      परत असत्य;
  }

  जर (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    जर (str.empty()) {
      makeNaN(IsSignaling, IsNegative);
      परत सत्य;
    }

    // Allow the payload to be inside parentheses.
    जर (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      जर (str.size() <= 2 || str.back() != ')')
        परत असत्य;

      str = str.slice(1, str.size() - 1);
    }

    // Determine the payload number's radix.
    अचिन्हित Radix = 10;
    जर (str[0] == '0') {
      जर (str.size() > 1 && छोटे(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } अन्यथा {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    जर (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      परत सत्य;
    }
  }

  परत असत्य;
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromString(StringRef str, roundingMode rounding_mode) {
  जर (str.empty())
    परत createError("Invalid string length");

  // Handle special cases.
  जर (convertFromStringSpecials(str))
    परत opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  माप_प्रकार slen = str.size();
  sign = *p == '-' ? 1 : 0;
  जर (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  जर (*p == '-' || *p == '+') {
    p++;
    slen--;
    जर (!slen)
      परत createError("String has no digits");
  }

  जर (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    जर (slen == 2)
      परत createError("Invalid string");
    परत convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_mode);
  }

  परत convertFromDecimalString(StringRef(p, slen), rounding_mode);
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
अचिन्हित पूर्णांक IEEEFloat::convertToHexString(अक्षर *dst, अचिन्हित पूर्णांक hexDigits,
                                           तार्किक upperCase,
                                           roundingMode rounding_mode) स्थिरांक {
  अक्षर *p = dst;
  जर (sign)
    *dst++ = '-';

  निवड (category) {
  स्थिती fcInfinity:
    स_नकल (dst, upperCase ? infinityU: infinityL, आकार infinityU - 1);
    dst += आकार infinityL - 1;
    थांबा;

  स्थिती fcNaN:
    स_नकल (dst, upperCase ? NaNU: NaNL, आकार NaNU - 1);
    dst += आकार NaNU - 1;
    थांबा;

  स्थिती fcZero:
    *dst++ = '0';
    *dst++ = upperCase ? 'X': 'x';
    *dst++ = '0';
    जर (hexDigits > 1) {
      *dst++ = '.';
      स_रखो (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = upperCase ? 'P': 'p';
    *dst++ = '0';
    थांबा;

  स्थिती fcNormal:
    dst = convertNormalToHexString (dst, hexDigits, upperCase, rounding_mode);
    थांबा;
  }

  *dst = 0;

  परत static_cast<अचिन्हित पूर्णांक>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
अक्षर *IEEEFloat::convertNormalToHexString(अक्षर *dst, अचिन्हित पूर्णांक hexDigits,
                                          तार्किक upperCase,
                                          roundingMode rounding_mode) स्थिरांक {
  *dst++ = '0';
  *dst++ = upperCase ? 'X': 'x';

  तार्किक roundUp = असत्य;
  स्थिरांक अक्षर *hexDigitChars = upperCase ? hexDigitsUpper : hexDigitsLower;

  स्थिरांक integerPart *significand = significandParts();
  अचिन्हित parts_मोजणी = part_मोजणी();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  अचिन्हित मूल्य_Bits = semantics->precision + 3;
  अचिन्हित shift = integerPartWidth - मूल्य_Bits % integerPartWidth;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  अचिन्हित outputDigits = (मूल्य_Bits - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  जर (hexDigits) {
    जर (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      अचिन्हित पूर्णांक bits;
      lostFraction fraction;

      bits = मूल्य_Bits - hexDigits * 4;
      fraction = lostFractionThroughTruncation (significand, parts_मोजणी, bits);
      roundUp = roundAwayFromZero(rounding_mode, fraction, bits);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  अक्षर *p = ++dst;

  अचिन्हित मोजणी = (मूल्य_Bits + integerPartWidth - 1) / integerPartWidth;

  पर्यंत (outputDigits && मोजणी) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    जर (--मोजणी == parts_मोजणी)
      part = 0;  /* An imaginary higher zero part.  */
    अन्यथा
      part = significand[मोजणी] << shift;

    जर (मोजणी && shift)
      part |= significand[मोजणी - 1] >> (integerPartWidth - shift);

    /* Convert as much of "part" to hexdigits as we can.  */
    अचिन्हित पूर्णांक curDigits = integerPartWidth / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  जर (roundUp) {
    अक्षर *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    करा {
      q--;
      *q = hexDigitChars[hex_Digit_मूल्य (*q) + 1];
    } पर्यंत (*q == '0');
    प्रतिज्ञा(q >= p);
  } अन्यथा {
    /* Add trailing zeroes.  */
    स_रखो (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  जर (dst -1 == p)
    dst--;
  अन्यथा
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = upperCase ? 'P': 'p';

  परत writeSignedDecimal (dst, exponent);
}

hash_code hash_मूल्य(स्थिरांक IEEEFloat &Arg) {
  जर (!Arg.isFiniteNonZero())
    परत hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  परत hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.part_मोजणी()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convertF80LongDoubleAPFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी() == 2);
  परत convertIEEEFloatToAPInt<APFloatBase::semX87DoubleExtended>();
}

APInt IEEEFloat::convertPPCDoubleDoubleLegacyAPFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(semantics ==
         (स्थिरांक llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy);
  प्रतिज्ञा(part_मोजणी()==2);

  uint64_t शब्द[2];
  तार्किक losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloatBase::semIEEEdouble.minExponent;
  IEEEFloat extended(*हे);
  [[maybe_unused]] opStatus fs =
      extended.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  प्रतिज्ञा(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
  प्रतिज्ञा(fs == opOK || fs == opInexact);
  शब्द[0] = *u.convertDoubleAPFloatToAPInt().getRawData();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  जर (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
    प्रतिज्ञा(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.subtract(u, rmNearestTiesToEven);
    fs = v.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
    प्रतिज्ञा(fs == opOK && !losesInfo);
    शब्द[1] = *v.convertDoubleAPFloatToAPInt().getRawData();
  } अन्यथा {
    शब्द[1] = 0;
  }

  परत APInt(128, शब्द);
}

ढाँचा <स्थिरांक fltSemantics &S>
APInt IEEEFloat::convertIEEEFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(semantics == &S);
  constexpr अचिन्हित पूर्णांक trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr पूर्णांक integer_bit_part = (S.precision - 1) / integerPartWidth;
  constexpr integerPart integer_bit = integerPart{1}
                                      << ((S.precision - 1) % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr अचिन्हित पूर्णांक exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr तार्किक is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr पूर्णांक bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_मोजणी_For_Bits(trailing_significand_bits)>
      mysignificand;

  जर (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
    जर (myexponent == 1 &&
        !(significandParts()[integer_bit_part] & integer_bit))
      myexponent = 0; // denormal
  } अन्यथा जर (category == fcZero) {
    जर (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } अन्यथा जर (category == fcInfinity) {
    जर (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    जर constexpr (S.hasExplicitIntegerBit) {
      mysignificand[0] = integerPart{1} << (trailing_significand_bits - 1);
    }
  } अन्यथा {
    प्रतिज्ञा(category == fcNaN && "Unknown category!");
    जर (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.sizeInBits + 63) / 64> शब्द;
  स्वतः शब्द_iter =
      std::copy_n(mysignificand.begin(), mysignificand.size(), शब्द.begin());
  जर constexpr (!S.hasExplicitIntegerBit) {
    जर constexpr (significand_mask != 0 || trailing_significand_bits == 0) {
      // Clear the integer bit.
      शब्द[mysignificand.size() - 1] &= significand_mask;
    }
  }
  std::fill(शब्द_iter, शब्द.end(), uint64_t{0});
  constexpr माप_प्रकार last_word = शब्द.size() - 1;
  जर constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.sizeInBits - 1) % 64);
    शब्द[last_word] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_mask)
                              << (trailing_significand_bits % 64);
  शब्द[last_word] |= shifted_exponent;
  जर constexpr (last_word == 0) {
    परत APInt(S.sizeInBits, शब्द[0]);
  }
  परत APInt(S.sizeInBits, शब्द);
}

APInt IEEEFloat::convertQuadrupleAPFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी() == 2);
  परत convertIEEEFloatToAPInt<APFloatBase::semIEEEquad>();
}

APInt IEEEFloat::convertDoubleAPFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी()==1);
  परत convertIEEEFloatToAPInt<APFloatBase::semIEEEdouble>();
}

APInt IEEEFloat::convertFloatAPFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी()==1);
  परत convertIEEEFloatToAPInt<APFloatBase::semIEEEsingle>();
}

APInt IEEEFloat::convertBFloatAPFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी() == 1);
  परत convertIEEEFloatToAPInt<APFloatBase::semBFloat>();
}

APInt IEEEFloat::convertHalfAPFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी()==1);
  परत convertIEEEFloatToAPInt<APFloatBase::APFloatBase::semIEEEhalf>();
}

APInt IEEEFloat::convertFloat8E5M2APFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी() == 1);
  परत convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2>();
}

APInt IEEEFloat::convertFloat8E5M2FNUZAPFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी() == 1);
  परत convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3APFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी() == 1);
  परत convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3>();
}

APInt IEEEFloat::convertFloat8E4M3FNAPFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी() == 1);
  परत convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FN>();
}

APInt IEEEFloat::convertFloat8E4M3FNUZAPFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी() == 1);
  परत convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3B11FNUZAPFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी() == 1);
  परत convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convertFloat8E3M4APFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी() == 1);
  परत convertIEEEFloatToAPInt<APFloatBase::semFloat8E3M4>();
}

APInt IEEEFloat::convertFloatTF32APFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी() == 1);
  परत convertIEEEFloatToAPInt<APFloatBase::semFloatTF32>();
}

APInt IEEEFloat::convertFloat8E8M0FNUAPFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी() == 1);
  परत convertIEEEFloatToAPInt<APFloatBase::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convertFloat8E5M3FNUAPFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी() == 1);
  परत convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convertFloat6E3M2FNAPFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी() == 1);
  परत convertIEEEFloatToAPInt<APFloatBase::semFloat6E3M2FN>();
}

APInt IEEEFloat::convertFloat6E2M3FNAPFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी() == 1);
  परत convertIEEEFloatToAPInt<APFloatBase::semFloat6E2M3FN>();
}

APInt IEEEFloat::convertFloat4E2M1FNAPFloatToAPInt() स्थिरांक {
  प्रतिज्ञा(part_मोजणी() == 1);
  परत convertIEEEFloatToAPInt<APFloatBase::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcastToAPInt() स्थिरांक {
  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semIEEEhalf)
    परत convertHalfAPFloatToAPInt();

  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semBFloat)
    परत convertBFloatAPFloatToAPInt();

  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semIEEEsingle)
    परत convertFloatAPFloatToAPInt();

  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semIEEEdouble)
    परत convertDoubleAPFloatToAPInt();

  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semIEEEquad)
    परत convertQuadrupleAPFloatToAPInt();

  जर (semantics ==
      (स्थिरांक llvm::fltSemantics *)&APFloatBase::semX87DoubleExtended)
    परत convertF80LongDoubleAPFloatToAPInt();

  जर (semantics ==
      (स्थिरांक llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy)
    परत convertPPCDoubleDoubleLegacyAPFloatToAPInt();

  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semFloat8E5M2)
    परत convertFloat8E5M2APFloatToAPInt();

  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semFloat8E5M2FNUZ)
    परत convertFloat8E5M2FNUZAPFloatToAPInt();

  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semFloat8E4M3)
    परत convertFloat8E4M3APFloatToAPInt();

  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FN)
    परत convertFloat8E4M3FNAPFloatToAPInt();

  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FNUZ)
    परत convertFloat8E4M3FNUZAPFloatToAPInt();

  जर (semantics ==
      (स्थिरांक llvm::fltSemantics *)&APFloatBase::semFloat8E4M3B11FNUZ)
    परत convertFloat8E4M3B11FNUZAPFloatToAPInt();

  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semFloat8E3M4)
    परत convertFloat8E3M4APFloatToAPInt();

  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semFloatTF32)
    परत convertFloatTF32APFloatToAPInt();

  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semFloat8E8M0FNU)
    परत convertFloat8E8M0FNUAPFloatToAPInt();

  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semFloat8E5M3FNU)
    परत convertFloat8E5M3FNUAPFloatToAPInt();

  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semFloat6E3M2FN)
    परत convertFloat6E3M2FNAPFloatToAPInt();

  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semFloat6E2M3FN)
    परत convertFloat6E2M3FNAPFloatToAPInt();

  जर (semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semFloat4E2M1FN)
    परत convertFloat4E2M1FNAPFloatToAPInt();

  llvm_unreachable("unknown format!");
}

दशांश IEEEFloat::convertToFloat() स्थिरांक {
  प्रतिज्ञा(semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcastToAPInt();
  परत api.bitsToFloat();
}

द्विगुण IEEEFloat::convertToDouble() स्थिरांक {
  प्रतिज्ञा(semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcastToAPInt();
  परत api.bitsToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 IEEEFloat::convertToQuad() स्थिरांक {
  प्रतिज्ञा(semantics == (स्थिरांक llvm::fltSemantics *)&APFloatBase::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcastToAPInt();
  परत api.bitsToQuad();
}
#endif

शून्य IEEEFloat::initFromF80LongDoubleAPInt(स्थिरांक APInt &api) {
  परत initFromIEEEAPInt<APFloatBase::semX87DoubleExtended>(api);
}

शून्य IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(स्थिरांक APInt &api) {
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
  जर (isFiniteNonZero()) {
    IEEEFloat v(APFloatBase::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    जर (v.isFinite()) {
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
शून्य IEEEFloat::initFromFloat8E8M0FNUAPInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E8M0FNU>(api);
}

शून्य IEEEFloat::initFromFloat8E5M3FNUAPInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M3FNU>(api);
}

ढाँचा <स्थिरांक fltSemantics &S>
शून्य IEEEFloat::initFromIEEEAPInt(स्थिरांक APInt &api) {
  प्रतिज्ञा(api.getBitWidth() == S.sizeInBits);

  constexpr अचिन्हित पूर्णांक trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr integerPart integer_bit =
      integerPart{1} << (trailing_significand_bits % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr अचिन्हित पूर्णांक exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr अचिन्हित पूर्णांक stored_significand_parts =
      part_मोजणी_For_Bits(trailing_significand_bits + 1);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr तार्किक is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr पूर्णांक bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr तार्किक has_significand = trailing_significand_bits > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  जर constexpr (has_significand) {
    std::copy_n(api.getRawData(), mysignificand.size(), mysignificand.begin());
    जर constexpr (significand_mask != 0 || S.precision >= integerPartWidth) {
      mysignificand[mysignificand.size() - 1] &= significand_mask;
    }
  } अन्यथा {
    std::fill_n(mysignificand.begin(), mysignificand.size(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t last_word = api.getRawData()[api.get_अंक_शब्द() - 1];
  uint64_t myexponent =
      (last_word >> (trailing_significand_bits % 64)) & exponent_mask;

  initialize(&S);
  प्रतिज्ञा(part_मोजणी() == mysignificand.size());

  sign = S.hasSignedRepr
             ? static_cast<अचिन्हित पूर्णांक>(last_word >> ((S.sizeInBits - 1) % 64))
             : 0;

  तार्किक all_zero_significand =
      has_significand && llvm::all_of(mysignificand, equal_to(0));

  तार्किक is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  जर constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    तार्किक is_inf = असत्य;

    जर constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;
      स्थिरांक integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_mask_no_int_bit) == 0;
    } अन्यथा {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    जर (is_inf) {
      makeInf(sign);
      परत;
    }
  }

  तार्किक is_nan = असत्य;

  जर constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    जर constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      स्थिरांक integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;

      जर (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_mask_no_int_bit) != 0) {
        // regular NaN and pseudoNaN
        is_nan = सत्य;
      } अन्यथा जर (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_mask_no_int_bit) == 0) {
        // pseudoinfinity
        is_nan = सत्य;
      } अन्यथा जर (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = सत्य;
      }
    } अन्यथा {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } अन्यथा जर constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    तार्किक all_ones_significand =
        std::all_of(mysignificand.begin(), mysignificand.end() - 1,
                    [](integerPart bits) { परत bits == ~integerPart{0}; }) &&
        (!significand_mask ||
         mysignificand[mysignificand.size() - 1] == significand_mask);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } अन्यथा जर constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  जर (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.size(),
                significandParts());
    परत;
  }

  जर (is_zero) {
    makeZero(sign);
    परत;
  }

  category = fcNormal;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.size(), significandParts());
  जर (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  अन्यथा {
    जर constexpr (!S.hasExplicitIntegerBit) {
      significandParts()[mysignificand.size() - 1] |= integer_bit;
    }
  }
}

शून्य IEEEFloat::initFromQuadrupleAPInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEquad>(api);
}

शून्य IEEEFloat::initFromDoubleAPInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEdouble>(api);
}

शून्य IEEEFloat::initFromFloatAPInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEsingle>(api);
}

शून्य IEEEFloat::initFromBFloatAPInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semBFloat>(api);
}

शून्य IEEEFloat::initFromHalfAPInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEhalf>(api);
}

शून्य IEEEFloat::initFromFloat8E5M2APInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2>(api);
}

शून्य IEEEFloat::initFromFloat8E5M2FNUZAPInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2FNUZ>(api);
}

शून्य IEEEFloat::initFromFloat8E4M3APInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3>(api);
}

शून्य IEEEFloat::initFromFloat8E4M3FNAPInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FN>(api);
}

शून्य IEEEFloat::initFromFloat8E4M3FNUZAPInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FNUZ>(api);
}

शून्य IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3B11FNUZ>(api);
}

शून्य IEEEFloat::initFromFloat8E3M4APInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E3M4>(api);
}

शून्य IEEEFloat::initFromFloatTF32APInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloatTF32>(api);
}

शून्य IEEEFloat::initFromFloat6E3M2FNAPInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E3M2FN>(api);
}

शून्य IEEEFloat::initFromFloat6E2M3FNAPInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E2M3FN>(api);
}

शून्य IEEEFloat::initFromFloat4E2M1FNAPInt(स्थिरांक APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
शून्य IEEEFloat::initFromAPInt(स्थिरांक fltSemantics *Sem, स्थिरांक APInt &api) {
  प्रतिज्ञा(api.getBitWidth() == Sem->sizeInBits);
  जर (Sem == &APFloatBase::semIEEEhalf)
    परत initFromHalfAPInt(api);
  जर (Sem == &APFloatBase::semBFloat)
    परत initFromBFloatAPInt(api);
  जर (Sem == &APFloatBase::semIEEEsingle)
    परत initFromFloatAPInt(api);
  जर (Sem == &APFloatBase::semIEEEdouble)
    परत initFromDoubleAPInt(api);
  जर (Sem == &APFloatBase::semIEEEquad)
    परत initFromQuadrupleAPInt(api);
  जर (Sem == &APFloatBase::semX87DoubleExtended)
    परत initFromF80LongDoubleAPInt(api);
  जर (Sem == &APFloatBase::semPPCDoubleDoubleLegacy)
    परत initFromPPCDoubleDoubleLegacyAPInt(api);
  जर (Sem == &APFloatBase::semFloat8E5M2)
    परत initFromFloat8E5M2APInt(api);
  जर (Sem == &APFloatBase::semFloat8E5M2FNUZ)
    परत initFromFloat8E5M2FNUZAPInt(api);
  जर (Sem == &APFloatBase::semFloat8E4M3)
    परत initFromFloat8E4M3APInt(api);
  जर (Sem == &APFloatBase::semFloat8E4M3FN)
    परत initFromFloat8E4M3FNAPInt(api);
  जर (Sem == &APFloatBase::semFloat8E4M3FNUZ)
    परत initFromFloat8E4M3FNUZAPInt(api);
  जर (Sem == &APFloatBase::semFloat8E4M3B11FNUZ)
    परत initFromFloat8E4M3B11FNUZAPInt(api);
  जर (Sem == &APFloatBase::semFloat8E3M4)
    परत initFromFloat8E3M4APInt(api);
  जर (Sem == &APFloatBase::semFloatTF32)
    परत initFromFloatTF32APInt(api);
  जर (Sem == &APFloatBase::semFloat8E8M0FNU)
    परत initFromFloat8E8M0FNUAPInt(api);
  जर (Sem == &APFloatBase::semFloat8E5M3FNU)
    परत initFromFloat8E5M3FNUAPInt(api);
  जर (Sem == &APFloatBase::semFloat6E3M2FN)
    परत initFromFloat6E3M2FNAPInt(api);
  जर (Sem == &APFloatBase::semFloat6E2M3FN)
    परत initFromFloat6E2M3FNAPInt(api);
  जर (Sem == &APFloatBase::semFloat4E2M1FN)
    परत initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
शून्य IEEEFloat::makeLargest(तार्किक Negative) {
  जर (Negative && !semantics->hasSignedRepr)
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
  अचिन्हित Part_मोजणी = part_मोजणी();
  स_रखो(significand, 0xFF, आकार(integerPart)*(Part_मोजणी - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  स्थिरांक अचिन्हित अंक_Unused_वरचा_Bits =
    Part_मोजणी*integerPartWidth - semantics->precision;
  significand[Part_मोजणी - 1] = (अंक_Unused_वरचा_Bits < integerPartWidth)
                                   ? (~integerPart(0) >> अंक_Unused_वरचा_Bits)
                                   : 0;
  जर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
शून्य IEEEFloat::makeSmallest(तार्किक Negative) {
  जर (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fcNormal;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tcSet(significandParts(), 1, part_मोजणी());
}

शून्य IEEEFloat::makeSmallestNormalized(तार्किक Negative) {
  जर (Negative && !semantics->hasSignedRepr)
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

IEEEFloat::IEEEFloat(स्थिरांक fltSemantics &Sem, स्थिरांक APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(दशांश f) {
  initFromAPInt(&APFloatBase::semIEEEsingle, APInt::floatToBits(f));
}

IEEEFloat::IEEEFloat(द्विगुण d) {
  initFromAPInt(&APFloatBase::semIEEEdouble, APInt::doubleToBits(d));
}

नामाकाश {
  शून्य append(SmallVectorImpl<अक्षर> &Buffer, StringRef Str) {
    Buffer.append(Str.begin(), Str.end());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  शून्य AdjustToPrecision(APInt &significand,
                         पूर्णांक &exp, अचिन्हित FormatPrecision) {
    अचिन्हित bits = significand.getActiveBits();

    // 196/59 is a very slight overestimate of lg_2(10).
    अचिन्हित bitsRequired = (FormatPrecision * 196 + 58) / 59;

    जर (bits <= bitsRequired) परत;

    अचिन्हित tensRemovable = (bits - bitsRequired) * 59 / 196;
    जर (!tensRemovable) परत;

    exp += tensRemovable;

    APInt भाजक(significand.getBitWidth(), 1);
    APInt powten(significand.getBitWidth(), 10);
    पर्यंत (सत्य) {
      जर (tensRemovable & 1)
        भाजक *= powten;
      tensRemovable >>= 1;
      जर (!tensRemovable) थांबा;
      powten *= powten;
    }

    significand = significand.udiv(भाजक);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.getActiveBits());
  }


  शून्य AdjustToPrecision(SmallVectorImpl<अक्षर> &buffer,
                         पूर्णांक &exp, अचिन्हित FormatPrecision) {
    अचिन्हित N = buffer.size();
    जर (N <= FormatPrecision) परत;

    // The most significant figures are the last ones in the buffer.
    अचिन्हित FirstSignificant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    जर (buffer[FirstSignificant - 1] < '5') {
      पर्यंत (FirstSignificant < N && buffer[FirstSignificant] == '0')
        FirstSignificant++;

      exp += FirstSignificant;
      buffer.erase(&buffer[0], &buffer[FirstSignificant]);
      परत;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    साठी (अचिन्हित I = FirstSignificant; I != N; ++I) {
      जर (buffer[I] == '9') {
        FirstSignificant++;
      } अन्यथा {
        buffer[I]++;
        थांबा;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    जर (FirstSignificant == N) {
      exp += FirstSignificant;
      buffer.clear();
      buffer.ढकला_back('1');
      परत;
    }

    exp += FirstSignificant;
    buffer.erase(&buffer[0], &buffer[FirstSignificant]);
  }

  शून्य toStringImpl(SmallVectorImpl<अक्षर> &Str, स्थिरांक तार्किक isNeg, पूर्णांक exp,
                    APInt significand, अचिन्हित FormatPrecision,
                    अचिन्हित FormatMaxPadding, तार्किक TruncateZero) {
    स्थिरांक पूर्णांक semanticsPrecision = significand.getBitWidth();

    जर (isNeg)
      Str.ढकला_back('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    जर (!FormatPrecision) {
      // We use enough digits so the number can be round-tripped back to an
      // APFloat. The formula comes from "How to Print Floating-Point Numbers
      // Accurately" by Steele and White.
      // FIXME: Using a formula based purely on the precision is conservative;
      // we can print fewer digits depending on the actual value being printed.

      // FormatPrecision = 2 + floor(significandBits / lg_2(10))
      FormatPrecision = 2 + semanticsPrecision * 59 / 196;
    }

    // Ignore trailing binary zeros.
    पूर्णांक trailingZeros = significand.countr_zero();
    exp += trailingZeros;
    significand.lshrInPlace(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    जर (exp == 0) {
      // Nothing to do.
    } अन्यथा जर (exp > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + exp);
      significand <<= exp;
      exp = 0;
    } अन्यथा { /* exp < 0 */
      पूर्णांक texp = -exp;

      // We transform this using the identity:
      //   (N)(2^-e) == (N)(5^e)(10^-e)
      // This means we have to multiply N (the significand) by 5^e.
      // To avoid overflow, we have to operate on numbers large
      // enough to store N * 5^e:
      //   log2(N * 5^e) == log2(N) + e * log2(5)
      //                 <= semantics->precision + e * 137 / 59
      //   (log_2(5) ~ 2.321928 < 2.322034 ~ 137/59)

      अचिन्हित precision = semanticsPrecision + (137 * texp + 136) / 59;

      // Multiply significand by 5^e.
      //   N * 5^0101 == N * 5^(1*1) * 5^(0*2) * 5^(1*4) * 5^(0*8)
      significand = significand.zext(precision);
      APInt five_to_the_i(precision, 5);
      पर्यंत (सत्य) {
        जर (texp & 1)
          significand *= five_to_the_i;

        texp >>= 1;
        जर (!texp)
          थांबा;
        five_to_the_i *= five_to_the_i;
      }
    }

    AdjustToPrecision(significand, exp, FormatPrecision);

    SmallVector<अक्षर, 256> buffer;

    // Fill the buffer.
    अचिन्हित precision = significand.getBitWidth();
    जर (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    तार्किक inTrail = सत्य;
    पर्यंत (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      अचिन्हित d = digit.get_ZExt_मूल्य();

      // Drop trailing zeros.
      जर (inTrail && !d)
        exp++;
      अन्यथा {
        buffer.ढकला_back((अक्षर) ('0' + d));
        inTrail = असत्य;
      }
    }

    प्रतिज्ञा(!buffer.empty() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    AdjustToPrecision(buffer, exp, FormatPrecision);

    अचिन्हित NDigits = buffer.size();

    // Check whether we should use scientific notation.
    तार्किक FormatScientific;
    जर (!FormatMaxPadding) {
      FormatScientific = सत्य;
    } अन्यथा {
      जर (exp >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((अचिन्हित) exp > FormatMaxPadding ||
                            NDigits + (अचिन्हित) exp > FormatPrecision);
      } अन्यथा {
        // Power of the most significant digit.
        पूर्णांक MSD = exp + (पूर्णांक) (NDigits - 1);
        जर (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = असत्य;
        } अन्यथा {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((अचिन्हित) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    जर (FormatScientific) {
      exp += (NDigits - 1);

      Str.ढकला_back(buffer[NDigits-1]);
      Str.ढकला_back('.');
      जर (NDigits == 1 && TruncateZero)
        Str.ढकला_back('0');
      अन्यथा
        साठी (अचिन्हित I = 1; I != NDigits; ++I)
          Str.ढकला_back(buffer[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      जर (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.ढकला_back(TruncateZero ? 'E' : 'e');

      Str.ढकला_back(exp >= 0 ? '+' : '-');
      जर (exp < 0)
        exp = -exp;
      SmallVector<अक्षर, 6> expbuf;
      करा {
        expbuf.ढकला_back((अक्षर) ('0' + (exp % 10)));
        exp /= 10;
      } पर्यंत (exp);
      // Exponent always at least two digits if we do not truncate zeros.
      जर (!TruncateZero && expbuf.size() < 2)
        expbuf.ढकला_back('0');
      साठी (अचिन्हित I = 0, E = expbuf.size(); I != E; ++I)
        Str.ढकला_back(expbuf[E-1-I]);
      परत;
    }

    // Non-scientific, positive exponents.
    जर (exp >= 0) {
      साठी (अचिन्हित I = 0; I != NDigits; ++I)
        Str.ढकला_back(buffer[NDigits-1-I]);
      साठी (अचिन्हित I = 0; I != (अचिन्हित) exp; ++I)
        Str.ढकला_back('0');
      परत;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    पूर्णांक NWholeDigits = exp + (पूर्णांक) NDigits;

    अचिन्हित I = 0;
    जर (NWholeDigits > 0) {
      साठी (; I != (अचिन्हित) NWholeDigits; ++I)
        Str.ढकला_back(buffer[NDigits-I-1]);
      Str.ढकला_back('.');
    } अन्यथा {
      अचिन्हित NZeros = 1 + (अचिन्हित) -NWholeDigits;

      Str.ढकला_back('0');
      Str.ढकला_back('.');
      साठी (अचिन्हित Z = 1; Z != NZeros; ++Z)
        Str.ढकला_back('0');
    }

    साठी (; I != NDigits; ++I)
      Str.ढकला_back(buffer[NDigits-I-1]);

  }
} // namespace

शून्य IEEEFloat::toString(SmallVectorImpl<अक्षर> &Str, अचिन्हित FormatPrecision,
                         अचिन्हित FormatMaxPadding, तार्किक TruncateZero) स्थिरांक {
  निवड (category) {
  स्थिती fcInfinity:
    जर (isNegative())
      परत append(Str, "-Inf");
    अन्यथा
      परत append(Str, "+Inf");

  स्थिती fcNaN: परत append(Str, "NaN");

  स्थिती fcZero:
    जर (isNegative())
      Str.ढकला_back('-');

    जर (!FormatMaxPadding) {
      जर (TruncateZero)
        append(Str, "0.0E+0");
      अन्यथा {
        append(Str, "0.0");
        जर (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } अन्यथा {
      Str.ढकला_back('0');
    }
    परत;

  स्थिती fcNormal:
    थांबा;
  }

  // Decompose the number into an APInt and an exponent.
  पूर्णांक exp = exponent - ((पूर्णांक) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_मोजणी_For_Bits(semantics->precision)));

  toStringImpl(Str, isNegative(), exp, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

पूर्णांक IEEEFloat::getExactLog2Abs() स्थिरांक {
  जर (!isFinite() || isZero())
    परत पूर्णांक_न्यून;

  स्थिरांक integerPart *Parts = significandParts();
  स्थिरांक पूर्णांक Part_मोजणी = part_मोजणी_For_Bits(semantics->precision);

  पूर्णांक काढा_मोजणी = 0;
  साठी (पूर्णांक i = 0; i < Part_मोजणी; ++i) {
    काढा_मोजणी += llvm::popcount(Parts[i]);
    जर (काढा_मोजणी > 1)
      परत पूर्णांक_न्यून;
  }

  जर (exponent != semantics->minExponent)
    परत exponent;

  पूर्णांक CountrParts = 0;
  साठी (पूर्णांक i = 0; i < Part_मोजणी;
       ++i, CountrParts += APInt::APINT_BITS_PER_WORD) {
    जर (Parts[i] != 0) {
      परत exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

तार्किक IEEEFloat::isSignaling() स्थिरांक {
  जर (!isNaN())
    परत असत्य;
  जर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    परत असत्य;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  परत !APInt::tcExtractBit(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::opStatus IEEEFloat::next(तार्किक nextDown) {
  // If we are performing nextDown, swap sign so we have -x.
  जर (nextDown)
    changeSign();

  // Compute nextUp(x)
  opStatus result = opOK;

  // Handle each float category separately.
  निवड (category) {
  स्थिती fcInfinity:
    // nextUp(+inf) = +inf
    जर (!isNegative())
      थांबा;
    // nextUp(-inf) = -getLargest()
    makeLargest(सत्य);
    थांबा;
  स्थिती fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    जर (isSignaling()) {
      result = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(असत्य, isNegative(), nullptr);
    }
    थांबा;
  स्थिती fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(असत्य);
    थांबा;
  स्थिती fcNormal:
    // nextUp(-getSmallest()) = -0
    जर (isSmallest() && isNegative()) {
      APInt::tcSet(significandParts(), 0, part_मोजणी());
      category = fcZero;
      exponent = 0;
      जर (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = असत्य;
      जर (!semantics->hasZero)
        makeSmallestNormalized(असत्य);
      थांबा;
    }

    जर (isLargest() && !isNegative()) {
      जर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        थांबा;
      } अन्यथा जर (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        थांबा;
      } अन्यथा {
        // nextUp(getLargest()) == INFINITY
        APInt::tcSet(significandParts(), 0, part_मोजणी());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        थांबा;
      }
    }

    // nextUp(normal) == normal + inc.
    जर (isNegative()) {
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
      APInt::tcDecrement(Parts, part_मोजणी());

      जर (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tcSetBit(Parts, semantics->precision - 1);
        exponent--;
      }
    } अन्यथा {
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

      जर (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tcSet(Parts, 0, part_मोजणी());
        APInt::tcSetBit(Parts, semantics->precision - 1);
        प्रतिज्ञा(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } अन्यथा {
        incrementSignificand();
      }
    }
    थांबा;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  जर (nextDown)
    changeSign();

  परत result;
}

APInt IEEEFloat::getNaNPayload() स्थिरांक {
  प्रतिज्ञा(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  अचिन्हित Bits = semantics->precision - 1;
  परत APInt(Bits, ArrayRef(significandParts(), part_मोजणी_For_Bits(Bits)));
}

APFloatBase::ExponentType IEEEFloat::exponentNaN() स्थिरांक {
  परत ::exponentNaN(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentInf() स्थिरांक {
  परत ::exponentInf(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentZero() स्थिरांक {
  परत ::exponentZero(*semantics);
}

शून्य IEEEFloat::makeInf(तार्किक Negative) {
  जर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  जर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(असत्य, Negative);
    परत;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tcSet(significandParts(), 0, part_मोजणी());
}

शून्य IEEEFloat::makeZero(तार्किक Negative) {
  जर (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  जर (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = असत्य;
  }
  exponent = exponentZero();
  APInt::tcSet(significandParts(), 0, part_मोजणी());
}

शून्य IEEEFloat::makeQuiet() {
  प्रतिज्ञा(isNaN());
  जर (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tcSetBit(significandParts(), semantics->precision - 2);
}

पूर्णांक ilogb(स्थिरांक IEEEFloat &Arg) {
  जर (Arg.isNaN())
    परत APFloat::IEK_NaN;
  जर (Arg.isZero())
    परत APFloat::IEK_Zero;
  जर (Arg.isInfinity())
    परत APFloat::IEK_Inf;
  जर (!Arg.isDenormal())
    परत Arg.exponent;

  IEEEFloat Normalized(Arg);
  पूर्णांक SignificandBits = Arg.getSemantics().precision - 1;

  Normalized.exponent += SignificandBits;
  Normalized.normalize(APFloat::rmNearestTiesToEven, lfExactlyZero);
  परत Normalized.exponent - SignificandBits;
}

IEEEFloat scalbn(IEEEFloat X, पूर्णांक Exp, roundingMode RoundingMode) {
  स्वतः MaxExp = X.getSemantics().maxExponent;
  स्वतः MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  पूर्णांक SignificandBits = X.getSemantics().precision - 1;
  पूर्णांक MaxIncrement = MaxExp - (MinExp - SignificandBits) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(RoundingMode, lfExactlyZero);
  जर (X.isNaN())
    X.makeQuiet();
  परत X;
}

IEEEFloat frexp(स्थिरांक IEEEFloat &Val, पूर्णांक &Exp, roundingMode RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  जर (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    परत Quiet;
  }

  जर (Exp == APFloat::IEK_Inf)
    परत Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  परत scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(स्थिरांक fltSemantics &S)
    : Semantics(&S),
      Floats(नवीन APFloat[2]{APFloat(APFloatBase::semIEEEdouble),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(स्थिरांक fltSemantics &S, uninitializedTag)
    : Semantics(&S), Floats(नवीन APFloat[2]{
                         APFloat(APFloatBase::semIEEEdouble, uninitialized),
                         APFloat(APFloatBase::semIEEEdouble, uninitialized)}) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(स्थिरांक fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(नवीन APFloat[2]{APFloat(APFloatBase::semIEEEdouble, I),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(स्थिरांक fltSemantics &S, स्थिरांक APInt &I)
    : Semantics(&S),
      Floats(नवीन APFloat[2]{
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[0])),
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[1]))}) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(स्थिरांक fltSemantics &S, APFloat &&First,
                             APFloat &&Second)
    : Semantics(&S),
      Floats(नवीन APFloat[2]{std::move(First), std::move(Second)}) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
  प्रतिज्ञा(&Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  प्रतिज्ञा(&Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(स्थिरांक DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? नवीन APFloat[2]{APFloat(RHS.Floats[0]),
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

DoubleAPFloat &DoubleAPFloat::चालक=(स्थिरांक DoubleAPFloat &RHS) {
  जर (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } अन्यथा जर (हे != &RHS) {
    हे->~DoubleAPFloat();
    नवीन (हे) DoubleAPFloat(RHS);
  }
  परत *हे;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
स्थिर std::pair<APFloat, APFloat> fastTwoSum(APFloat X, APFloat Y) {
  जर (!X.isFinite())
    परत {X, APFloat::getZero(X.getSemantics(), /*Negative=*/असत्य)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  परत {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::opStatus DoubleAPFloat::addImpl(स्थिरांक APFloat &a, स्थिरांक APFloat &aa,
                                         स्थिरांक APFloat &c, स्थिरांक APFloat &cc,
                                         roundingMode RM) {
  पूर्णांक Status = opOK;
  APFloat z = a;
  Status |= z.add(c, RM);
  जर (!z.isFinite()) {
    जर (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ असत्य);
      परत (opStatus)Status;
    }
    Status = opOK;
    स्वतः AComparedToC = a.compare_Absolute_मूल्य(c);
    z = cc;
    Status |= z.add(aa, RM);
    जर (AComparedToC == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      Status |= z.add(c, RM);
      Status |= z.add(a, RM);
    } अन्यथा {
      // z = cc + aa + a + c;
      Status |= z.add(a, RM);
      Status |= z.add(c, RM);
    }
    जर (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ असत्य);
      परत (opStatus)Status;
    }
    Floats[0] = z;
    APFloat zz = aa;
    Status |= zz.add(cc, RM);
    जर (AComparedToC == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(c, RM);
      Status |= Floats[1].add(zz, RM);
    } अन्यथा {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(a, RM);
      Status |= Floats[1].add(zz, RM);
    }
  } अन्यथा {
    // q = a - z;
    APFloat q = a;
    Status |= q.subtract(z, RM);

    // zz = q + c + (a - (q + z)) + aa + cc;
    // Compute a - (q + z) as -((q + z) - a) to avoid temporary copies.
    स्वतः zz = q;
    Status |= zz.add(c, RM);
    Status |= q.add(z, RM);
    Status |= q.subtract(a, RM);
    q.changeSign();
    Status |= zz.add(q, RM);
    Status |= zz.add(aa, RM);
    Status |= zz.add(cc, RM);
    जर (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ असत्य);
      परत opOK;
    }
    Floats[0] = z;
    Status |= Floats[0].add(zz, RM);
    जर (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ असत्य);
      परत (opStatus)Status;
    }
    Floats[1] = std::move(z);
    Status |= Floats[1].subtract(Floats[0], RM);
    Status |= Floats[1].add(zz, RM);
  }
  परत (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::addWithSpecial(स्थिरांक DoubleAPFloat &LHS,
                                                स्थिरांक DoubleAPFloat &RHS,
                                                DoubleAPFloat &Out,
                                                roundingMode RM) {
  जर (LHS.getCategory() == fcNaN) {
    Out = LHS;
    परत opOK;
  }
  जर (RHS.getCategory() == fcNaN) {
    Out = RHS;
    परत opOK;
  }
  जर (LHS.getCategory() == fcZero) {
    Out = RHS;
    परत opOK;
  }
  जर (RHS.getCategory() == fcZero) {
    Out = LHS;
    परत opOK;
  }
  जर (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    Out.makeNaN(असत्य, Out.isNegative(), nullptr);
    परत opInvalidOp;
  }
  जर (LHS.getCategory() == fcInfinity) {
    Out = LHS;
    परत opOK;
  }
  जर (RHS.getCategory() == fcInfinity) {
    Out = RHS;
    परत opOK;
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
  परत Out.addImpl(A, AA, C, CC, RM);
}

APFloat::opStatus DoubleAPFloat::add(स्थिरांक DoubleAPFloat &RHS,
                                     roundingMode RM) {
  परत addWithSpecial(*हे, RHS, *हे, RM);
}

APFloat::opStatus DoubleAPFloat::subtract(स्थिरांक DoubleAPFloat &RHS,
                                          roundingMode RM) {
  changeSign();
  स्वतः Ret = add(RHS, RM);
  changeSign();
  परत Ret;
}

APFloat::opStatus DoubleAPFloat::multiply(स्थिरांक DoubleAPFloat &RHS,
                                          APFloat::roundingMode RM) {
  स्थिरांक स्वतः &LHS = *हे;
  स्वतः &Out = *हे;
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
  जर (LHS.getCategory() == fcNaN) {
    Out = LHS;
    परत opOK;
  }
  जर (RHS.getCategory() == fcNaN) {
    Out = RHS;
    परत opOK;
  }
  जर ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    Out.makeNaN(असत्य, असत्य, nullptr);
    परत opOK;
  }
  जर (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    Out = LHS;
    परत opOK;
  }
  जर (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    Out = RHS;
    परत opOK;
  }
  प्रतिज्ञा(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal &&
         "Special cases not handled exhaustively");

  पूर्णांक Status = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  Status |= T.multiply(C, RM);
  जर (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ असत्य);
    परत (opStatus)Status;
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
  जर (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ असत्य);
  } अन्यथा {
    // Floats[1] = (t - u) + tau
    Status |= T.subtract(U, RM);
    Status |= T.add(Tau, RM);
    Floats[1] = std::move(T);
  }
  परत (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::divide(स्थिरांक DoubleAPFloat &RHS,
                                        APFloat::roundingMode RM) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  स्वतः Ret = Tmp.divide(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()), RM);
  *हे = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  परत Ret;
}

APFloat::opStatus DoubleAPFloat::remainder(स्थिरांक DoubleAPFloat &RHS) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  स्वतः Ret = Tmp.remainder(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *हे = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  परत Ret;
}

APFloat::opStatus DoubleAPFloat::mod(स्थिरांक DoubleAPFloat &RHS) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  स्वतः Ret = Tmp.mod(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *हे = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  परत Ret;
}

APFloat::opStatus
DoubleAPFloat::fusedMultiplyAdd(स्थिरांक DoubleAPFloat &Multiplicand,
                                स्थिरांक DoubleAPFloat &Addend,
                                APFloat::roundingMode RM) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  स्वतः Ret = Tmp.fusedMultiplyAdd(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcastToAPInt()),
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, Addend.bitcastToAPInt()),
      RM);
  *हे = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  परत Ret;
}

APFloat::opStatus DoubleAPFloat::roundToIntegral(APFloat::roundingMode RM) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  स्थिरांक APFloat &Hi = getFirst();
  स्थिरांक APFloat &Lo = getSecond();

  APFloat RoundedHi = Hi;
  स्थिरांक opStatus HiStatus = RoundedHi.roundToIntegral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  जर (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/असत्य);
    परत HiStatus;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  स्वतः RoundToNearestHelper = [](APFloat ToRound, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    स्थिरांक APFloat RoundingError = Rounded - ToRound;
    जर (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        असल(RoundingError).is_Exactly_मूल्य(0.5))
      Rounded.add(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rmNearestTiesToEven);
    परत Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  जर (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    जर (RM == rmNearestTiesToAway || RM == rmNearestTiesToEven)
      RoundedHi = RoundToNearestHelper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/असत्य);
    परत HiStatus;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  RoundingMode LoRoundingMode;
  जर (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    LoRoundingMode = isNegative() ? rmTowardPositive : rmTowardNegative;
  अन्यथा
    LoRoundingMode = RM;

  APFloat RoundedLo = Lo;
  स्थिरांक opStatus LoStatus = RoundedLo.roundToIntegral(LoRoundingMode);
  जर (LoRoundingMode == rmNearestTiesToAway)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = RoundToNearestHelper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fastTwoSum(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  परत LoStatus;
}

शून्य DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmpResult
DoubleAPFloat::compare_Absolute_मूल्य(स्थिरांक DoubleAPFloat &RHS) स्थिरांक {
  // Compare absolute values of the high parts.
  स्थिरांक cmpResult HiPartCmp = Floats[0].compare_Absolute_मूल्य(RHS.Floats[0]);
  जर (HiPartCmp != cmpEqual)
    परत HiPartCmp;

  // Zero, regardless of sign, is equal.
  जर (Floats[1].isZero() && RHS.Floats[1].isZero())
    परत cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  स्थिरांक तार्किक ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  स्थिरांक तार्किक RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  जर (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    परत RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  जर (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    परत ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  जर (ThisIsSubtractive != RHSIsSubtractive)
    परत ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  स्थिरांक cmpResult LoPartCmp = Floats[1].compare_Absolute_मूल्य(RHS.Floats[1]);

  जर (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    जर (LoPartCmp == cmpLessThan)
      परत cmpGreaterThan;
    जर (LoPartCmp == cmpGreaterThan)
      परत cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  परत LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() स्थिरांक {
  परत Floats[0].getCategory();
}

तार्किक DoubleAPFloat::isNegative() स्थिरांक { परत Floats[0].isNegative(); }

शून्य DoubleAPFloat::makeInf(तार्किक Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ असत्य);
}

शून्य DoubleAPFloat::makeZero(तार्किक Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ असत्य);
}

शून्य DoubleAPFloat::makeLargest(तार्किक Neg) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  जर (Neg)
    changeSign();
}

शून्य DoubleAPFloat::makeSmallest(तार्किक Neg) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].makeSmallest(Neg);
  Floats[1].makeZero(/* Neg = */ असत्य);
}

शून्य DoubleAPFloat::makeSmallestNormalized(तार्किक Neg) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  जर (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ असत्य);
}

शून्य DoubleAPFloat::makeNaN(तार्किक SNaN, तार्किक Neg, स्थिरांक APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ असत्य);
}

APFloat::cmpResult DoubleAPFloat::compare(स्थिरांक DoubleAPFloat &RHS) स्थिरांक {
  स्वतः Result = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  जर (Result == APFloat::cmpEqual)
    परत Floats[1].compare(RHS.Floats[1]);
  परत Result;
}

तार्किक DoubleAPFloat::bitwiseIsEqual(स्थिरांक DoubleAPFloat &RHS) स्थिरांक {
  परत Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_code hash_मूल्य(स्थिरांक DoubleAPFloat &Arg) {
  जर (Arg.Floats)
    परत hash_combine(hash_मूल्य(Arg.Floats[0]), hash_मूल्य(Arg.Floats[1]));
  परत hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcastToAPInt() स्थिरांक {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t Data[] = {
      Floats[0].bitcastToAPInt().getRawData()[0],
      Floats[1].bitcastToAPInt().getRawData()[0],
  };
  परत APInt(128, Data);
}

Expected<APFloat::opStatus> DoubleAPFloat::convertFromString(StringRef S,
                                                             roundingMode RM) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy);
  स्वतः Ret = Tmp.convertFromString(S, RM);
  *हे = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  परत Ret;
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
  जर (nextDown) {
    changeSign();
    APFloat::opStatus Result = next(/*nextDown=*/असत्य);
    changeSign();
    परत Result;
  }
  निवड (getCategory()) {
  स्थिती fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    जर (isNegative())
      makeLargest(सत्य);
    परत opOK;

  स्थिती fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    जर (getFirst().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(असत्य, isNegative(), nullptr);
      परत opInvalidOp;
    }
    परत opOK;

  स्थिती fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(असत्य);
    परत opOK;

  स्थिती fcNormal:
    थांबा;
  }

  स्थिरांक APFloat &HiOld = getFirst();
  स्थिरांक APFloat &LoOld = getSecond();

  APFloat NextLo = LoOld;
  NextLo.next(/*nextDown=*/असत्य);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  स्वतः InLattice = [](स्थिरांक APFloat &Hi, स्थिरांक APFloat &Lo) {
    परत Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  जर (InLattice(HiOld, NextLo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(NextLo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat Largest{*Semantics, uninitialized};
    Largest.makeLargest(/*Neg=*/असत्य);
    जर (compare(Largest) == cmpGreaterThan)
      makeInf(/*Neg=*/असत्य);

    परत opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat NextHi = HiOld;
  NextHi.next(/*nextDown=*/असत्य);

  // nextUp(getLargest()) == INFINITY
  जर (NextHi.isInfinity()) {
    makeInf(/*Neg=*/असत्य);
    परत opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  जर (NextHi.isZero()) {
    makeZero(/*Neg=*/सत्य);
    परत opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  NextLo = neg(scalbn(harrisonUlp(NextHi), -1, rmTowardZero));
  जर (!InLattice(NextHi, NextLo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    NextLo.next(/*nextDown=*/असत्य);

  Floats[0] = std::move(NextHi);
  Floats[1] = std::move(NextLo);

  परत opOK;
}

APFloat::opStatus DoubleAPFloat::convertToSignExtendedInteger(
    MutableArrayRef<integerPart> आदान, अचिन्हित पूर्णांक Width, तार्किक IsSigned,
    roundingMode RM, तार्किक *IsExact) स्थिरांक {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  जर (!getFirst().isFiniteNonZero() || getSecond().isZero())
    परत getFirst().convertToInteger(आदान, Width, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = असत्य;
  DoubleAPFloat Integral = *हे;
  स्थिरांक opStatus RoundStatus = Integral.roundToIntegral(RM);
  जर (RoundStatus == opInvalidOp)
    परत opInvalidOp;
  स्थिरांक APFloat &IntegralHi = Integral.getFirst();
  स्थिरांक APFloat &IntegralLo = Integral.getSecond();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  तार्किक HiIsExact;
  जर (IntegralHi.isZero() || IntegralLo.isZero()) {
    स्थिरांक opStatus HiStatus =
        IntegralHi.convertToInteger(आदान, Width, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    परत HiStatus == opOK ? opInexact : HiStatus;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  जर (!IsSigned && IntegralHi.isNegative())
    परत opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  तार्किक LoIsExact;
  स्थिरांक पूर्णांक HiExactLog2 = IntegralHi.getExactLog2Abs();
  स्थिरांक अचिन्हित PositiveOverflowWidth = IsSigned ? Width - 1 : Width;
  जर (HiExactLog2 >= 0 &&
      static_cast<अचिन्हित>(HiExactLog2) == PositiveOverflowWidth) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    जर (IntegralHi.isNegative() == IntegralLo.isNegative())
      परत opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    स्थिरांक opStatus LoStatus = IntegralLo.convertToInteger(
        आदान, Width, /*IsSigned=*/सत्य, RM, &LoIsExact);
    जर (LoStatus == opInvalidOp)
      परत opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    जर (IsSigned && !IntegralHi.isNegative())
      APInt::tcClearBit(आदान.data(), PositiveOverflowWidth);
    *IsExact = RoundStatus == opOK;
    परत RoundStatus;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  स्थिरांक opStatus HiStatus = IntegralHi.convertToInteger(
      आदान, Width, IsSigned, rmTowardZero, &HiIsExact);
  जर (HiStatus == opInvalidOp)
    परत HiStatus;

  // Convert Lo into a temporary integer of the same width.
  APSInt LoResult{Width, /*isUnsigned=*/!IsSigned};
  स्थिरांक opStatus LoStatus =
      IntegralLo.convertToInteger(LoResult, rmTowardZero, &LoIsExact);
  जर (LoStatus == opInvalidOp)
    परत LoStatus;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tcAdd(आदान.data(), LoResult.getRawData(), /*carry=*/0, आदान.size());

  *IsExact = RoundStatus == opOK;
  परत RoundStatus;
}

APFloat::opStatus
DoubleAPFloat::convertToInteger(MutableArrayRef<integerPart> आदान,
                                अचिन्हित पूर्णांक Width, तार्किक IsSigned,
                                roundingMode RM, तार्किक *IsExact) स्थिरांक {
  opStatus FS =
      convertToSignExtendedInteger(आदान, Width, IsSigned, RM, IsExact);

  जर (FS == opInvalidOp) {
    स्थिरांक अचिन्हित Dst_Parts_मोजणी = part_मोजणी_For_Bits(Width);
    प्रतिज्ञा(Dst_Parts_मोजणी <= आदान.size() && "Integer too big");

    अचिन्हित Bits;
    जर (getCategory() == fcNaN)
      Bits = 0;
    अन्यथा जर (isNegative())
      Bits = IsSigned;
    अन्यथा
      Bits = Width - IsSigned;

    tcSetLeastSignificantBits(आदान.data(), Dst_Parts_मोजणी, Bits);
    जर (isNegative() && IsSigned)
      APInt::tc_Shift_डावा(आदान.data(), Dst_Parts_मोजणी, Width - 1);
  }

  परत FS;
}

APFloat::opStatus DoubleAPFloat::handleOverflow(roundingMode RM) {
  निवड (RM) {
  स्थिती APFloat::rmTowardZero:
    makeLargest(/*Neg=*/isNegative());
    थांबा;
  स्थिती APFloat::rmTowardNegative:
    जर (isNegative())
      makeInf(/*Neg=*/सत्य);
    अन्यथा
      makeLargest(/*Neg=*/असत्य);
    थांबा;
  स्थिती APFloat::rmTowardPositive:
    जर (isNegative())
      makeLargest(/*Neg=*/सत्य);
    अन्यथा
      makeInf(/*Neg=*/असत्य);
    थांबा;
  स्थिती APFloat::rmNearestTiesToAway:
  स्थिती APFloat::rmNearestTiesToEven:
    makeInf(/*Neg=*/isNegative());
    थांबा;
  पूर्वनिर्धारित:
    llvm_unreachable("Invalid rounding mode found");
  }
  opStatus S = opInexact;
  जर (!getFirst().isFinite())
    S = static_cast<opStatus>(S | opOverflow);
  परत S;
}

APFloat::opStatus DoubleAPFloat::convertFromUnsignedParts(
    स्थिरांक integerPart *Src, अचिन्हित पूर्णांक Src_मोजणी, roundingMode RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  स्थिरांक अचिन्हित SrcMSB = APInt::tcMSB(Src, Src_मोजणी);
  जर (SrcMSB == अच_पूर्णांक_उच्च) {
    // The source integer is 0.
    makeZero(/*Neg=*/असत्य);
    परत opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  स्थिरांक अचिन्हित SrcBitWidth = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/SrcBitWidth, ArrayRef(Src, Src_मोजणी)},
                /*isUnsigned=*/सत्य};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{getFirst().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/असत्य, rmNearestTiesToEven);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  जर (!Hi.isFinite())
    परत handleOverflow(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  तार्किक HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/सत्य};
  Hi.convertToInteger(HiAsInt, rmNearestTiesToEven, &HiAsIntIsExact);
  स्थिरांक APInt Error = SrcInt.zext(HiAsInt.getBitWidth()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  roundingMode LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  जर (Error.isNegative()) {
    जर (RM == rmNearestTiesToAway) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      स्थिरांक अचिन्हित ErrorActiveBits = Error.getSignificantBits() - 1;
      स्थिरांक अचिन्हित LoPrecision = getSecond().getSemantics().precision;
      जर (ErrorActiveBits > LoPrecision) {
        स्थिरांक अचिन्हित RoundingBoundary = ErrorActiveBits - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        जर (Error.मोजणी_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rmTowardPositive;
      }
    } अन्यथा जर (RM == rmTowardZero) {
      // For rmTowardZero, the final positive result must be truncated (rounded
      // down). When Hi is an overestimate, Error is negative. A standard
      // rmTowardZero conversion of Error would make it *less* negative,
      // effectively rounding the final sum Hi + Lo *up*. To ensure the sum
      // rounds down correctly, we force Lo to round toward -infinity.
      LoRM = rmTowardNegative;
    }
  }

  APFloat Lo{getSecond().getSemantics()};
  opStatus Status = Lo.convertFromAPInt(Error, /*IsSigned=*/सत्य, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fastTwoSum(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  जर (!getFirst().isFinite())
    परत handleOverflow(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  जर (getFirst().isFiniteNonZero() && Floats[0].isLargest()) {
    DoubleAPFloat Largest{*Semantics};
    Largest.makeLargest(/*Neg=*/असत्य);
    जर (compare(Largest) == APFloat::cmpGreaterThan)
      परत handleOverflow(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  परत Status;
}

APFloat::opStatus DoubleAPFloat::convertFromAPInt(स्थिरांक APInt &आदान,
                                                  तार्किक IsSigned,
                                                  roundingMode RM) {
  स्थिरांक तार्किक Negate_आदान = IsSigned && आदान.isNegative();
  APInt API = आदान;
  जर (Negate_आदान)
    API.negate();

  स्थिरांक APFloat::opStatus Status =
      convertFromUnsignedParts(API.getRawData(), API.get_अंक_शब्द(), RM);
  जर (Negate_आदान)
    changeSign();
  परत Status;
}

अचिन्हित पूर्णांक DoubleAPFloat::convertToHexString(अक्षर *DST,
                                               अचिन्हित पूर्णांक HexDigits,
                                               तार्किक UpperCase,
                                               roundingMode RM) स्थिरांक {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  परत APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .convertToHexString(DST, HexDigits, UpperCase, RM);
}

तार्किक DoubleAPFloat::isDenormal() स्थिरांक {
  परत getCategory() == fcNormal &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

तार्किक DoubleAPFloat::isSmallest() स्थिरांक {
  जर (getCategory() != fcNormal)
    परत असत्य;
  DoubleAPFloat Tmp(*हे);
  Tmp.makeSmallest(हे->isNegative());
  परत Tmp.compare(*हे) == cmpEqual;
}

तार्किक DoubleAPFloat::isSmallestNormalized() स्थिरांक {
  जर (getCategory() != fcNormal)
    परत असत्य;

  DoubleAPFloat Tmp(*हे);
  Tmp.makeSmallestNormalized(हे->isNegative());
  परत Tmp.compare(*हे) == cmpEqual;
}

तार्किक DoubleAPFloat::isLargest() स्थिरांक {
  जर (getCategory() != fcNormal)
    परत असत्य;
  DoubleAPFloat Tmp(*हे);
  Tmp.makeLargest(हे->isNegative());
  परत Tmp.compare(*हे) == cmpEqual;
}

तार्किक DoubleAPFloat::isInteger() स्थिरांक {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  परत Floats[0].isInteger() && Floats[1].isInteger();
}

शून्य DoubleAPFloat::toString(SmallVectorImpl<अक्षर> &Str,
                             अचिन्हित FormatPrecision,
                             अचिन्हित FormatMaxPadding,
                             तार्किक TruncateZero) स्थिरांक {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .toString(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

पूर्णांक DoubleAPFloat::getExactLog2Abs() स्थिरांक {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  जर (getSecond().isNonZero())
    परत पूर्णांक_न्यून;
  परत getFirst().getExactLog2Abs();
}

पूर्णांक ilogb(स्थिरांक DoubleAPFloat &Arg) {
  स्थिरांक APFloat &Hi = Arg.getFirst();
  स्थिरांक APFloat &Lo = Arg.getSecond();
  पूर्णांक IlogbResult = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  जर (Arg.getCategory() != fcNormal)
    परत IlogbResult;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  जर (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    परत IlogbResult;
  जर (Hi.getExactLog2Abs() == पूर्णांक_न्यून)
    परत IlogbResult;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  परत IlogbResult - 1;
}

DoubleAPFloat scalbn(स्थिरांक DoubleAPFloat &Arg, पूर्णांक Exp,
                     APFloat::roundingMode RM) {
  प्रतिज्ञा(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");
  परत DoubleAPFloat(APFloatBase::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(स्थिरांक DoubleAPFloat &Arg, पूर्णांक &Exp,
                    APFloat::roundingMode RM) {
  प्रतिज्ञा(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  जर (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.getFirst() = Quiet.getFirst().makeQuiet();
    परत Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  जर (Exp == APFloat::IEK_Inf)
    परत Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  जर (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    परत Arg;
  }

  स्थिरांक APFloat &Hi = Arg.getFirst();
  स्थिरांक APFloat &Lo = Arg.getSecond();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  स्थिरांक तार्किक SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat Second = Lo;
  जर (Arg.getCategory() == APFloat::fcNormal && Lo.isFiniteNonZero()) {
    roundingMode LoRoundingMode;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    जर (RM == rmTowardZero)
      LoRoundingMode = Arg.isNegative() ? rmTowardPositive : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    अन्यथा जर (RM == rmNearestTiesToAway && SignsDisagree && Exp > 0)
      LoRoundingMode = rmNearestTiesToEven;
    अन्यथा
      LoRoundingMode = RM;
    Second = scalbn(Lo, -Exp, LoRoundingMode);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    जर (RM == rmNearestTiesToAway && LoRoundingMode == rmNearestTiesToEven) {
      // Re-scale the result back to check if rounding occurred.
      स्थिरांक APFloat RecomposedLo = scalbn(Second, Exp, rmNearestTiesToEven);
      जर (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        स्थिरांक APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        स्थिरांक APFloat UlpOfSecond = harrisonUlp(Second);
        स्थिरांक APFloat ScaledUlpOfSecond =
            scalbn(UlpOfSecond, Exp - 1, rmNearestTiesToEven);
        स्थिरांक तार्किक IsMidpoint = असल(RoundingError) == ScaledUlpOfSecond;
        स्थिरांक तार्किक RoundedLoAway =
            Second.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        जर (IsMidpoint && RoundedLoAway)
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
    जर (Second.isZero() && SignsDisagree && Hi.getExactLog2Abs() != पूर्णांक_न्यून)
      ++Exp;
  }

  APFloat First = scalbn(Hi, -Exp, RM);
  परत DoubleAPFloat(APFloatBase::PPCDoubleDouble(), std::move(First),
                       std::move(Second));
}

APInt DoubleAPFloat::getNaNPayload() स्थिरांक { परत Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, स्थिरांक fltSemantics &Semantics) {
  जर (usesLayout<IEEEFloat>(Semantics)) {
    नवीन (&IEEE) IEEEFloat(std::move(F));
    परत;
  }
  जर (usesLayout<DoubleAPFloat>(Semantics)) {
    स्थिरांक fltSemantics& S = F.getSemantics();
    नवीन (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloatBase::IEEEdouble()));
    परत;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::opStatus> APFloat::convertFromString(StringRef Str,
                                                       roundingMode RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_code hash_मूल्य(स्थिरांक APFloat &Arg) {
  जर (APFloat::usesLayout<detail::IEEEFloat>(Arg.getSemantics()))
    परत hash_मूल्य(Arg.U.IEEE);
  जर (APFloat::usesLayout<detail::DoubleAPFloat>(Arg.getSemantics()))
    परत hash_मूल्य(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(स्थिरांक fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  स्वतः StatusOrErr = convertFromString(S, rmNearestTiesToEven);
  प्रतिज्ञा(StatusOrErr && "Invalid floating point representation");
  consumeError(StatusOrErr.takeError());
}

FPClassTest APFloat::classify() स्थिरांक {
  जर (isZero())
    परत isNegative() ? fcNegZero : fcPosZero;
  जर (isNormal())
    परत isNegative() ? fcNegNormal : fcPosNormal;
  जर (isDenormal())
    परत isNegative() ? fcNegSubnormal : fcPosSubnormal;
  जर (isInfinity())
    परत isNegative() ? fcNegInf : fcPosInf;
  प्रतिज्ञा(isNaN() && "Other class of FP constant");
  परत isSignaling() ? fcSNan : fcQNan;
}

तार्किक APFloat::getExactInverse(APFloat *Inv) स्थिरांक {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  जर (!isFiniteNonZero())
    परत असत्य;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  जर (isDenormal())
    परत असत्य;

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
  स्थिरांक पूर्णांक Exp = getExactLog2Abs();
  जर (Exp == पूर्णांक_न्यून)
    परत असत्य;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  जर (Reciprocal.getExactLog2Abs() != -Exp)
    परत असत्य;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  जर (Reciprocal.isDenormal())
    परत असत्य;

  प्रतिज्ञा(Reciprocal.isFiniteNonZero());

  जर (Inv)
    *Inv = std::move(Reciprocal);

  परत सत्य;
}

APFloat::opStatus APFloat::convert(स्थिरांक fltSemantics &ToSemantics,
                                   roundingMode RM, तार्किक *losesInfo) {
  जर (&getSemantics() == &ToSemantics) {
    *losesInfo = असत्य;
    परत opOK;
  }
  जर (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics))
    परत U.IEEE.convert(ToSemantics, RM, losesInfo);
  जर (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<DoubleAPFloat>(ToSemantics)) {
    प्रतिज्ञा(&ToSemantics == &APFloatBase::semPPCDoubleDouble);
    स्वतः Ret =
        U.IEEE.convert(APFloatBase::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *हे = APFloat(ToSemantics, U.IEEE.bitcastToAPInt());
    परत Ret;
  }
  जर (usesLayout<DoubleAPFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics)) {
    स्वतः Ret = getIEEE().convert(ToSemantics, RM, losesInfo);
    *हे = APFloat(std::move(getIEEE()), ToSemantics);
    परत Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_मूल्य(स्थिरांक fltSemantics &Semantics) {
  परत APFloat(Semantics, APInt::getAllOnes(Semantics.sizeInBits));
}

शून्य APFloat::print(raw_ostream &OS) स्थिरांक {
  SmallVector<अक्षर, 16> Buffer;
  toString(Buffer);
  OS << Buffer;
}

#जर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य APFloat::dump() स्थिरांक {
  print(dbgs());
  dbgs() << '\n';
}
#endif

शून्य APFloat::Profile(Folding_Set_गाठ_ID &NID) स्थिरांक {
  NID.Add(bitcastToAPInt());
}

APFloat::opStatus APFloat::convertToInteger(APSInt &result,
                                            roundingMode rounding_mode,
                                            तार्किक *isExact) स्थिरांक {
  अचिन्हित bitWidth = result.getBitWidth();
  SmallVector<uint64_t, 4> parts(result.get_अंक_शब्द());
  opStatus status = convertToInteger(parts, bitWidth, result.isSigned(),
                                     rounding_mode, isExact);
  // Keeps the original signed-ness.
  result = APInt(bitWidth, parts);
  परत status;
}

द्विगुण APFloat::convertToDouble() स्थिरांक {
  जर (&getSemantics() == &APFloatBase::semIEEEdouble)
    परत getIEEE().convertToDouble();
  प्रतिज्ञा(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat Temp = *हे;
  तार्किक LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &LosesInfo);
  प्रतिज्ञा(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  परत Temp.getIEEE().convertToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 APFloat::convertToQuad() स्थिरांक {
  जर (&getSemantics() == &APFloatBase::semIEEEquad)
    परत getIEEE().convertToQuad();
  प्रतिज्ञा(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat Temp = *हे;
  तार्किक LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEquad, rmNearestTiesToEven, &LosesInfo);
  प्रतिज्ञा(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  परत Temp.getIEEE().convertToQuad();
}
#endif

दशांश APFloat::convertToFloat() स्थिरांक {
  जर (&getSemantics() == &APFloatBase::semIEEEsingle)
    परत getIEEE().convertToFloat();
  प्रतिज्ञा(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat Temp = *हे;
  तार्किक LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEsingle, rmNearestTiesToEven, &LosesInfo);
  प्रतिज्ञा(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  परत Temp.getIEEE().convertToFloat();
}

अचिन्हित APFloatBase::getArbitraryFPFormatSizeInBits(StringRef Format) {
  परत StringSwitch<अचिन्हित>(Format)
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
  परत getArbitraryFPFormatSizeInBits(Format) != 0;
}

स्थिरांक fltSemantics *APFloatBase::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  परत StringSwitch<स्थिरांक fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(nullptr);
}

APFloat::Storage::~Storage() {
  जर (usesLayout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    परत;
  }
  जर (usesLayout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    परत;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(स्थिरांक APFloat::Storage &RHS) {
  जर (usesLayout<IEEEFloat>(*RHS.semantics)) {
    नवीन (हे) IEEEFloat(RHS.IEEE);
    परत;
  }
  जर (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    नवीन (हे) DoubleAPFloat(RHS.Double);
    परत;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  जर (usesLayout<IEEEFloat>(*RHS.semantics)) {
    नवीन (हे) IEEEFloat(std::move(RHS.IEEE));
    परत;
  }
  जर (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    नवीन (हे) DoubleAPFloat(std::move(RHS.Double));
    परत;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::चालक=(स्थिरांक APFloat::Storage &RHS) {
  जर (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } अन्यथा जर (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } अन्यथा जर (हे != &RHS) {
    हे->~Storage();
    नवीन (हे) Storage(RHS);
  }
  परत *हे;
}

APFloat::Storage &APFloat::Storage::चालक=(APFloat::Storage &&RHS) {
  जर (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } अन्यथा जर (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } अन्यथा जर (हे != &RHS) {
    हे->~Storage();
    नवीन (हे) Storage(std::move(RHS));
  }
  परत *हे;
}

नामाकाश {

APFloat::opStatus getOpStatusFromLibc(पूर्णांक libc_exceptions) {
  APFloat::opStatus status = APFloat::opOK;
  जर (libc_exceptions & FE_INVALID)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInvalidOp);
  जर (libc_exceptions & FE_DIVBYZERO)
    status = static_cast<APFloat::opStatus>(status | APFloat::opDivByZero);
  जर (libc_exceptions & FE_OVERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opOverflow);
  जर (libc_exceptions & FE_UNDERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opUnderflow);
  जर (libc_exceptions & FE_INEXACT)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInexact);
  परत status;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> exp(स्थिरांक APFloat &x, RoundingMode rounding_mode,
                           APFloat::opStatus *status) {

  जर (rounding_mode == APFloatBase::rmNearestTiesToEven) {
    जर (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEsingle) {
      दशांश x_val = x.convertToFloat();
      पूर्णांक exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      जर (status) {
        *status = getOpStatusFromLibc(exc);
        जर (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      दशांश result = LIBC_NAMESPACE::shared::expf(x_val);
      परत APFloat(result);
    }
    जर (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEdouble) {
      द्विगुण x_val = x.convertToDouble();
      पूर्णांक exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      जर (status) {
        *status = getOpStatusFromLibc(exc);
        जर (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      द्विगुण result = LIBC_NAMESPACE::shared::exp(x_val);
      परत APFloat(result);
    }
  }
  परत std::nullopt;
}

} // namespace llvm

#undef APFLOAT_DISPATCH_ON_SEMANTICS
