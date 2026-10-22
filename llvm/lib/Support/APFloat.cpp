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

#ضمن "llvm/ADT/APFloat.h"
#ضمن "llvm/ADT/APSInt.h"
#ضمن "llvm/ADT/ArrayRef.h"
#ضمن "llvm/ADT/FloatingPointMode.h"
#ضمن "llvm/ADT/FoldingSet.h"
#ضمن "llvm/ADT/Hashing.h"
#ضمن "llvm/ADT/STLExtras.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/ADT/StringSwitch.h"
#ضمن "llvm/Config/llvm-config.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/MathExtras.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <cstring>
#ضمن <iterator>
#ضمن <limits.h>

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

#ضمن "shared/math.h"
#ضمن "shared/math_check_exceptions.h"

#define APFLOAT_DISPATCH_ON_SEMANTICS(METHOD_CALL)                             \
  𐡏𐡁𐡃 {                                                                         \
    𐡄𐡍 (usesLayout<IEEEFloat>(getSemantics()))                                 \
      أرجع U.IEEE.METHOD_CALL;                                               \
    𐡄𐡍 (usesLayout<DoubleAPFloat>(getSemantics()))                             \
      أرجع U.Double.METHOD_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } طالما (خطأ)

using نطاق llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#define PackCategoriesIntoKey(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloatBase::integerPartWidth % 4 == 0, "Part width must be divisible by 4!");

نطاق llvm {

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
    𐡒𐡔𐡈,
    𐡒𐡔𐡈,
    𐡒𐡔𐡈,
    𐡒𐡔𐡈,
    𐡒𐡔𐡈};
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
    خطأ,
    خطأ,
    خطأ,
    خطأ};

constexpr fltSemantics APFloatBase::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    𐡒𐡔𐡈,
    خطأ,
    خطأ};

constexpr fltSemantics APFloatBase::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semBogus = {0, 0, 0, 0};

ثابت llvm::fltSemantics &APFloatBase::EnumToSemantics(Semantics S) {
  بدل (S) {
  حالة S_IEEEhalf:
    أرجع IEEEhalf();
  حالة S_BFloat:
    أرجع BFloat();
  حالة S_IEEEsingle:
    أرجع IEEEsingle();
  حالة S_IEEEdouble:
    أرجع IEEEdouble();
  حالة S_IEEEquad:
    أرجع IEEEquad();
  حالة S_x87DoubleExtended:
    أرجع x87DoubleExtended();
  حالة S_PPCDoubleDouble:
    أرجع PPCDoubleDouble();
  حالة S_PPCDoubleDoubleLegacy:
    أرجع PPCDoubleDoubleLegacy();
  حالة S_Float8E5M2:
    أرجع Float8E5M2();
  حالة S_Float8E5M2FNUZ:
    أرجع Float8E5M2FNUZ();
  حالة S_Float8E4M3:
    أرجع Float8E4M3();
  حالة S_Float8E4M3FN:
    أرجع Float8E4M3FN();
  حالة S_Float8E4M3FNUZ:
    أرجع Float8E4M3FNUZ();
  حالة S_Float8E4M3B11FNUZ:
    أرجع Float8E4M3B11FNUZ();
  حالة S_Float8E3M4:
    أرجع Float8E3M4();
  حالة S_FloatTF32:
    أرجع FloatTF32();
  حالة S_Float8E8M0FNU:
    أرجع Float8E8M0FNU();
  حالة S_Float8E5M3FNU:
    أرجع Float8E5M3FNU();
  حالة S_Float6E3M2FN:
    أرجع Float6E3M2FN();
  حالة S_Float6E2M3FN:
    أرجع Float6E2M3FN();
  حالة S_Float4E2M1FN:
    أرجع Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloatBase::Semantics
APFloatBase::SemanticsToEnum(ثابت llvm::fltSemantics &Sem) {
  𐡄𐡍 (&Sem == &llvm::APFloat::IEEEhalf())
    أرجع S_IEEEhalf;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::BFloat())
    أرجع S_BFloat;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::IEEEsingle())
    أرجع S_IEEEsingle;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::IEEEdouble())
    أرجع S_IEEEdouble;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::IEEEquad())
    أرجع S_IEEEquad;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::x87DoubleExtended())
    أرجع S_x87DoubleExtended;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::PPCDoubleDouble())
    أرجع S_PPCDoubleDouble;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    أرجع S_PPCDoubleDoubleLegacy;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::Float8E5M2())
    أرجع S_Float8E5M2;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    أرجع S_Float8E5M2FNUZ;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::Float8E4M3())
    أرجع S_Float8E4M3;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::Float8E4M3FN())
    أرجع S_Float8E4M3FN;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    أرجع S_Float8E4M3FNUZ;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    أرجع S_Float8E4M3B11FNUZ;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::Float8E3M4())
    أرجع S_Float8E3M4;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::FloatTF32())
    أرجع S_FloatTF32;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::Float8E8M0FNU())
    أرجع S_Float8E8M0FNU;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::Float8E5M3FNU())
    أرجع S_Float8E5M3FNU;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::Float6E3M2FN())
    أرجع S_Float6E3M2FN;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::Float6E2M3FN())
    أرجع S_Float6E2M3FN;
  وإلا 𐡄𐡍 (&Sem == &llvm::APFloat::Float4E2M1FN())
    أرجع S_Float4E2M1FN;
  وإلا
    llvm_unreachable("Unknown floating semantics");
}

منطقي APFloatBase::isRepresentableBy(ثابت fltSemantics &A,
                                    ثابت fltSemantics &B) {
  أرجع A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

منطقي APFloatBase::isLosslesslyConvertibleTo(ثابت fltSemantics &From,
                                            ثابت fltSemantics &InitialTo,
                                            منطقي IgnoreNaNs) {
  𐡄𐡍 (&From == &InitialTo)
    أرجع 𐡒𐡔𐡈;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  𐡄𐡍 (&From == &semPPCDoubleDouble)
    أرجع خطأ;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  ثابت fltSemantics &To =
      &InitialTo == &semPPCDoubleDouble ? semIEEEdouble : InitialTo;

  𐡄𐡍 (!isRepresentableBy(From, To))
    أرجع خطأ;

  𐡄𐡍 ((From.hasZero && !To.hasZero) ||
      (From.hasSignedRepr && !To.hasSignedRepr))
    أرجع خطأ;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  منطقي FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  منطقي ToHasSignedZero = To.hasZero && To.hasSignedRepr &&
                         To.nanEncoding != fltNanEncoding::NegativeZero;
  𐡄𐡍 (FromHasSignedZero && !ToHasSignedZero)
    أرجع خطأ;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat SmallestFrom = APFloat::getSmallest(From);
  منطقي LosesInfo = خطأ;
  (فراغ)SmallestFrom.convert(To, APFloat::rmNearestTiesToEven, &LosesInfo);
  𐡄𐡍 (LosesInfo)
    أرجع خطأ;

  𐡄𐡍 (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    أرجع 𐡒𐡔𐡈;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  𐡄𐡍 (IgnoreNaNs) {
    𐡄𐡍 (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      أرجع To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    أرجع To.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  𐡄𐡍 (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    أرجع خطأ;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  𐡄𐡍 (To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    أرجع 𐡒𐡔𐡈;
  أرجع To.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
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
ساكن constexpr unsigned NumPow5PartsOnStack = 16;

ساكن unsigned صحيح powerOfFivePartsForExponent(unsigned صحيح power) {
  أكد(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  أرجع 2 + ((power * 815) / (351 * APFloatBase::integerPartWidth));
}

unsigned صحيح APFloatBase::semanticsPrecision(ثابت fltSemantics &semantics) {
  أرجع semantics.precision;
}
APFloatBase::ExponentType
APFloatBase::semanticsMaxExponent(ثابت fltSemantics &semantics) {
  أرجع semantics.maxExponent;
}
APFloatBase::ExponentType
APFloatBase::semanticsMinExponent(ثابت fltSemantics &semantics) {
  أرجع semantics.minExponent;
}
unsigned صحيح APFloatBase::semanticsSizeInBits(ثابت fltSemantics &semantics) {
  أرجع semantics.sizeInBits;
}
unsigned صحيح APFloatBase::semanticsIntSizeInBits(ثابت fltSemantics &semantics,
                                                 منطقي isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  unsigned صحيح MinBitWidth = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  𐡄𐡍 (isSigned)
    ++MinBitWidth;
  أرجع MinBitWidth;
}

منطقي APFloatBase::semanticsHasZero(ثابت fltSemantics &semantics) {
  أرجع semantics.hasZero;
}

منطقي APFloatBase::semanticsHasSignedRepr(ثابت fltSemantics &semantics) {
  أرجع semantics.hasSignedRepr;
}

منطقي APFloatBase::semanticsHasInf(ثابت fltSemantics &semantics) {
  أرجع semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

منطقي APFloatBase::semanticsHasNaN(ثابت fltSemantics &semantics) {
  أرجع semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

منطقي APFloatBase::isIEEELikeFP(ثابت fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  أرجع SemanticsToEnum(semantics) <= S_IEEEquad;
}

منطقي APFloatBase::hasSignBitInMSB(ثابت fltSemantics &semantics) {
  أرجع semantics.hasSignBitInMSB;
}

منطقي APFloatBase::isRepresentableAsNormalIn(ثابت fltSemantics &Src,
                                            ثابت fltSemantics &Dst) {
  // Exponent range must be larger.
  𐡄𐡍 (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    أرجع خطأ;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  أرجع Dst.precision >= Src.precision;
}

unsigned APFloatBase::getSizeInBits(ثابت fltSemantics &Sem) {
  أرجع Sem.sizeInBits;
}

ساكن constexpr APFloatBase::ExponentType
exponentZero(ثابت fltSemantics &semantics) {
  أرجع semantics.minExponent - 1;
}

ساكن constexpr APFloatBase::ExponentType
exponentInf(ثابت fltSemantics &semantics) {
  أرجع semantics.maxExponent + 1;
}

ساكن constexpr APFloatBase::ExponentType
exponentNaN(ثابت fltSemantics &semantics) {
  𐡄𐡍 (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    𐡄𐡍 (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      أرجع exponentZero(semantics);
    𐡄𐡍 (semantics.hasSignedRepr || semantics.precision > 1)
      أرجع semantics.maxExponent;
  }
  أرجع semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

ساكن مضمن Error createError(ثابت Twine &Err) {
  أرجع make_error<StringError>(Err, inconvertibleErrorCode());
}

ساكن constexpr مضمن unsigned صحيح partCountForBits(unsigned صحيح bits) {
  أرجع std::max(1u, (bits + APFloatBase::integerPartWidth - 1) /
                          APFloatBase::integerPartWidth);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
ساكن مضمن unsigned صحيح
decDigitValue(unsigned صحيح c)
{
  أرجع c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
ساكن Expected<صحيح> readExponent(StringRef::iterator begin,
                                  StringRef::iterator end) {
  ثابت unsigned صحيح overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  𐡄𐡍 (p == end || ((*p == '-' || *p == '+') && (p + 1) == end))
    أرجع 0;

  منطقي isNegative = *p == '-';
  𐡄𐡍 (*p == '-' || *p == '+') {
    p++;
    𐡄𐡍 (p == end)
      أرجع createError("Exponent has no digits");
  }

  unsigned absExponent = decDigitValue(*p++);
  𐡄𐡍 (absExponent >= 10U)
    أرجع createError("Invalid character in exponent");

  لكل (; p != end; ++p) {
    unsigned value = decDigitValue(*p);
    𐡄𐡍 (value >= 10U)
      أرجع createError("Invalid character in exponent");

    absExponent = absExponent * 10U + value;
    𐡄𐡍 (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      اكسر;
    }
  }

  𐡄𐡍 (isNegative)
    أرجع -(صحيح) absExponent;
  وإلا
    أرجع (صحيح) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
ساكن Expected<صحيح> totalExponent(StringRef::iterator p,
                                   StringRef::iterator end,
                                   صحيح exponentAdjustment) {
  صحيح exponent = 0;

  𐡄𐡍 (p == end)
    أرجع createError("Exponent has no digits");

  منطقي negative = *p == '-';
  𐡄𐡍 (*p == '-' || *p == '+') {
    p++;
    𐡄𐡍 (p == end)
      أرجع createError("Exponent has no digits");
  }

  صحيح unsignedExponent = 0;
  منطقي overflow = خطأ;
  لكل (; p != end; ++p) {
    unsigned صحيح value;

    value = decDigitValue(*p);
    𐡄𐡍 (value >= 10U)
      أرجع createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + value;
    𐡄𐡍 (unsignedExponent > 32767) {
      overflow = 𐡒𐡔𐡈;
      اكسر;
    }
  }

  𐡄𐡍 (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    overflow = 𐡒𐡔𐡈;

  𐡄𐡍 (!overflow) {
    exponent = unsignedExponent;
    𐡄𐡍 (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    𐡄𐡍 (exponent > 32767 || exponent < -32768)
      overflow = 𐡒𐡔𐡈;
  }

  𐡄𐡍 (overflow)
    exponent = negative ? -32768: 32767;

  أرجع exponent;
}

ساكن Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator end,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = end;
  طالما (p != end && *p == '0')
    p++;

  𐡄𐡍 (p != end && *p == '.') {
    *dot = p++;

    𐡄𐡍 (end - begin == 1)
      أرجع createError("Significand has no digits");

    طالما (p != end && *p == '0')
      p++;
  }

  أرجع p;
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
بنية decimalInfo {
  ثابت حرف *firstSigDigit;
  ثابت حرف *lastSigDigit;
  صحيح exponent;
  صحيح normalizedExponent;
};

ساكن Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator end, decimalInfo *D) {
  StringRef::iterator dot = end;

  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  𐡄𐡍 (!PtrOrErr)
    أرجع PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->firstSigDigit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  لكل (; p != end; ++p) {
    𐡄𐡍 (*p == '.') {
      𐡄𐡍 (dot != end)
        أرجع createError("String contains multiple dots");
      dot = p++;
      𐡄𐡍 (p == end)
        اكسر;
    }
    𐡄𐡍 (decDigitValue(*p) >= 10U)
      اكسر;
  }

  𐡄𐡍 (p != end) {
    𐡄𐡍 (*p != 'e' && *p != 'E')
      أرجع createError("Invalid character in significand");
    𐡄𐡍 (p == begin)
      أرجع createError("Significand has no digits");
    𐡄𐡍 (dot != end && p - begin == 1)
      أرجع createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    auto ExpOrErr = readExponent(p + 1, end);
    𐡄𐡍 (!ExpOrErr)
      أرجع ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    𐡄𐡍 (dot == end)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  𐡄𐡍 (p != D->firstSigDigit) {
    /* Drop insignificant trailing zeroes.  */
    𐡄𐡍 (p != begin) {
      𐡏𐡁𐡃
        𐡏𐡁𐡃
          p--;
        طالما (p != begin && *p == '0');
      طالما (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->firstSigDigit)
                                      - (dot > D->firstSigDigit && dot < p)));
  }

  D->lastSigDigit = p;
  أرجع Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
ساكن Expected<lostFraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator end,
                            unsigned صحيح digitValue) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  𐡄𐡍 (digitValue > 8)
    أرجع lfMoreThanHalf;
  وإلا 𐡄𐡍 (digitValue < 8 && digitValue > 0)
    أرجع lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  طالما (p != end && (*p == '0' || *p == '.'))
    p++;

  𐡄𐡍 (p == end)
    أرجع createError("Invalid trailing hexadecimal fraction!");

  unsigned hexDigit = hexDigitValue(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  𐡄𐡍 (hexDigit == UINT_MAX)
    أرجع digitValue == 0 ? lfExactlyZero: lfExactlyHalf;
  وإلا
    أرجع digitValue == 0 ? lfLessThanHalf: lfMoreThanHalf;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
ساكن lostFraction
lostFractionThroughTruncation(ثابت APFloatBase::integerPart *parts,
                              unsigned صحيح partCount,
                              unsigned صحيح bits)
{
  unsigned lsb = APInt::tcLSB(parts, partCount);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  𐡄𐡍 (bits <= lsb)
    أرجع lfExactlyZero;
  𐡄𐡍 (bits == lsb + 1)
    أرجع lfExactlyHalf;
  𐡄𐡍 (bits <= partCount * APFloatBase::integerPartWidth &&
      APInt::tcExtractBit(parts, bits - 1))
    أرجع lfMoreThanHalf;

  أرجع lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
ساكن lostFraction
shiftRight(APFloatBase::integerPart *dst, unsigned صحيح parts, unsigned صحيح bits)
{
  lostFraction lost_fraction = lostFractionThroughTruncation(dst, parts, bits);

  APInt::tcShiftRight(dst, parts, bits);

  أرجع lost_fraction;
}

/* Combine the effect of two lost fractions.  */
ساكن lostFraction
combineLostFractions(lostFraction moreSignificant,
                     lostFraction lessSignificant)
{
  𐡄𐡍 (lessSignificant != lfExactlyZero) {
    𐡄𐡍 (moreSignificant == lfExactlyZero)
      moreSignificant = lfLessThanHalf;
    وإلا 𐡄𐡍 (moreSignificant == lfExactlyHalf)
      moreSignificant = lfMoreThanHalf;
  }

  أرجع moreSignificant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
ساكن unsigned صحيح
HUerrBound(منطقي inexactMultiply, unsigned صحيح HUerr1, unsigned صحيح HUerr2)
{
  أكد(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  𐡄𐡍 (HUerr1 + HUerr2 == 0)
    أرجع inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  وإلا
    أرجع inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
ساكن APFloatBase::integerPart
ulpsFromBoundary(ثابت APFloatBase::integerPart *parts, unsigned صحيح bits,
                 منطقي isNearest) {
  أكد(bits != 0);

  bits--;
  unsigned count = bits / APFloatBase::integerPartWidth;
  unsigned partBits = bits % APFloatBase::integerPartWidth + 1;

  APFloatBase::integerPart part =
      parts[count] & (~(APFloatBase::integerPart)0 >>
                      (APFloatBase::integerPartWidth - partBits));

  APFloatBase::integerPart boundary;
  𐡄𐡍 (isNearest)
    boundary = (APFloatBase::integerPart) 1 << (partBits - 1);
  وإلا
    boundary = 0;

  𐡄𐡍 (count == 0) {
    𐡄𐡍 (part - boundary <= boundary - part)
      أرجع part - boundary;
    وإلا
      أرجع boundary - part;
  }

  𐡄𐡍 (part == boundary) {
    طالما (--count)
      𐡄𐡍 (parts[count])
        أرجع ~(APFloatBase::integerPart) 0; /* A lot.  */

    أرجع parts[0];
  } وإلا 𐡄𐡍 (part == boundary - 1) {
    طالما (--count)
      𐡄𐡍 (~parts[count])
        أرجع ~(APFloatBase::integerPart) 0; /* A lot.  */

    أرجع -parts[0];
  }

  أرجع ~(APFloatBase::integerPart) 0; /* A lot.  */
}

ساكن constexpr unsigned PowerOfFivePartCounts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

ساكن constexpr APFloatBase::integerPart PowerOfFiveParts[] = {
#ضمن "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  unsigned Sum = 0;
  لكل (unsigned Count : PowerOfFivePartCounts)
    Sum += Count;
  أرجع Sum;
}() == std::size(PowerOfFiveParts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
ساكن unsigned صحيح
powerOf5(APFloatBase::integerPart *dst, unsigned صحيح power) {
  ساكن ثابت APFloatBase::integerPart firstEightPowers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  SmallVector<APFloatBase::integerPart, NumPow5PartsOnStack> scratch;
  scratch.resize_for_overwrite(powerOfFivePartsForExponent(power));

  APFloatBase::integerPart *p1, *p2;
  ثابت APFloatBase::integerPart *pow5;

  p1 = dst;
  p2 = scratch.data();

  *p1 = firstEightPowers[power & 7];
  power >>= 3;

  unsigned result = 1;
  pow5 = PowerOfFiveParts;

  لكل (unsigned صحيح n = 0; power; power >>= 1, n++) {
    unsigned partsCount = PowerOfFivePartCounts[n];

    𐡄𐡍 (power & 1) {
      APFloatBase::integerPart *tmp;

      APInt::tcFullMultiply(p2, p1, pow5, result, partsCount);
      result += partsCount;
      𐡄𐡍 (p2[result - 1] == 0)
        result--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += partsCount;
  }

  𐡄𐡍 (p1 != dst)
    APInt::tcAssign(dst, p1, result);

  أرجع result;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
ساكن ثابت حرف hexDigitsLower[] = "0123456789abcdef0";
ساكن ثابت حرف hexDigitsUpper[] = "0123456789ABCDEF0";
ساكن ثابت حرف infinityL[] = "infinity";
ساكن ثابت حرف infinityU[] = "INFINITY";
ساكن ثابت حرف NaNL[] = "nan";
ساكن ثابت حرف NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
ساكن unsigned صحيح
partAsHex (حرف *dst, APFloatBase::integerPart part, unsigned صحيح count,
           ثابت حرف *hexDigitChars)
{
  unsigned صحيح result = count;

  أكد(count != 0 && count <= APFloatBase::integerPartWidth / 4);

  part >>= (APFloatBase::integerPartWidth - 4 * count);
  طالما (count--) {
    dst[count] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  أرجع result;
}

/* Write out an unsigned decimal integer.  */
ساكن حرف *writeUnsignedDecimal(حرف *dst, unsigned صحيح n) {
  حرف buff[40], *p;

  p = buff;
  𐡏𐡁𐡃
    *p++ = '0' + n % 10;
  طالما (n /= 10);

  𐡏𐡁𐡃
    *dst++ = *--p;
  طالما (p != buff);

  أرجع dst;
}

/* Write out a signed decimal integer.  */
ساكن حرف *writeSignedDecimal(حرف *dst, صحيح value) {
  𐡄𐡍 (value < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(unsigned) value);
  } وإلا {
    dst = writeUnsignedDecimal(dst, value);
  }

  أرجع dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
ساكن APFloat harrisonUlp(ثابت APFloat &X) {
  ثابت fltSemantics &Sem = X.getSemantics();
  بدل (X.getCategory()) {
  حالة APFloat::fcNaN:
    أرجع APFloat::getQNaN(Sem);
  حالة APFloat::fcInfinity:
    أرجع APFloat::getInf(Sem);
  حالة APFloat::fcZero:
    أرجع APFloat::getSmallest(Sem);
  حالة APFloat::fcNormal:
    اكسر;
  }
  𐡄𐡍 (X.isDenormal() || X.isSmallestNormalized())
    أرجع APFloat::getSmallest(Sem);
  صحيح Exp = ilogb(X);
  𐡄𐡍 (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  أرجع scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rmNearestTiesToEven);
}

نطاق detail {
/* Constructors.  */
فراغ IEEEFloat::initialize(ثابت fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  unsigned count = partCount();
  𐡄𐡍 (count > 1)
    significand.parts = جديد integerPart[count];
}

فراغ IEEEFloat::freeSignificand() {
  𐡄𐡍 (needsCleanup())
    delete [] significand.parts;
}

فراغ IEEEFloat::assign(ثابت IEEEFloat &rhs) {
  أكد(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  𐡄𐡍 (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

فراغ IEEEFloat::copySignificand(ثابت IEEEFloat &rhs) {
  أكد(isFiniteNonZero() || category == fcNaN);
  أكد(rhs.partCount() >= partCount());

  APInt::tcAssign(significandParts(), rhs.significandParts(),
                  partCount());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
فراغ IEEEFloat::makeNaN(منطقي SNaN, منطقي Negative, ثابت APInt *fill) {
  𐡄𐡍 (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  𐡄𐡍 (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  unsigned numParts = partCount();

  APInt fill_storage;
  𐡄𐡍 (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = خطأ;
    𐡄𐡍 (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = 𐡒𐡔𐡈;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } وإلا {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  𐡄𐡍 (!fill || fill->getNumWords() < numParts)
    APInt::tcSet(significand, 0, numParts);
  𐡄𐡍 (fill) {
    APInt::tcAssign(significand, fill->getRawData(),
                    std::min(fill->getNumWords(), numParts));

    // Zero out the excess bits of the significand.
    unsigned bitsToPreserve = semantics->precision - 1;
    unsigned part = bitsToPreserve / 64;
    bitsToPreserve %= 64;
    significand[part] &= ((1ULL << bitsToPreserve) - 1);
    لكل (part++; part != numParts; ++part)
      significand[part] = 0;
  }

  unsigned QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  𐡄𐡍 (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tcClearBit(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    𐡄𐡍 (APInt::tcIsZero(significand, numParts))
      APInt::tcSetBit(significand, QNaNBit - 1);
  } وإلا 𐡄𐡍 (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } وإلا {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tcSetBit(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  𐡄𐡍 (semantics == &APFloatBase::semX87DoubleExtended)
    APInt::tcSetBit(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::operator=(ثابت IEEEFloat &rhs) {
  𐡄𐡍 (هذا != &rhs) {
    𐡄𐡍 (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    assign(rhs);
  }

  أرجع *هذا;
}

IEEEFloat &IEEEFloat::operator=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloatBase::semBogus;
  أرجع *هذا;
}

منطقي IEEEFloat::isDenormal() ثابت {
  أرجع getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tcExtractBit(significandParts(), semantics->precision - 1) ==
          0);
}

منطقي IEEEFloat::isSmallest() ثابت {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  أرجع isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

منطقي IEEEFloat::isSmallestNormalized() ثابت {
  أرجع getCategory() == fcNormal && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

unsigned صحيح IEEEFloat::getNumHighBits() ثابت {
  ثابت unsigned صحيح PartCount = partCountForBits(semantics->precision);
  ثابت unsigned صحيح Bits = PartCount * integerPartWidth;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  ثابت unsigned صحيح NumHighBits = (semantics->precision > 1)
                                       ? (Bits - semantics->precision + 1)
                                       : (Bits - semantics->precision);
  أرجع NumHighBits;
}

منطقي IEEEFloat::isSignificandAllOnes() ثابت {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  ثابت integerPart *Parts = significandParts();
  ثابت unsigned PartCount = partCountForBits(semantics->precision);
  لكل (unsigned i = 0; i < PartCount - 1; i++)
    𐡄𐡍 (~Parts[i])
      أرجع خطأ;

  // Set the unused high bits to all ones when we compare.
  ثابت unsigned NumHighBits = getNumHighBits();
  أكد(NumHighBits <= integerPartWidth && NumHighBits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ثابت integerPart HighBitFill =
    ~integerPart(0) << (integerPartWidth - NumHighBits);
  𐡄𐡍 ((semantics->precision <= 1) || (~(Parts[PartCount - 1] | HighBitFill)))
    أرجع خطأ;

  أرجع 𐡒𐡔𐡈;
}

منطقي IEEEFloat::isSignificandAllOnesExceptLSB() ثابت {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  ثابت integerPart *Parts = significandParts();

  𐡄𐡍 (Parts[0] & 1)
    أرجع خطأ;

  ثابت unsigned PartCount = partCountForBits(semantics->precision);
  لكل (unsigned i = 0; i < PartCount - 1; i++) {
    𐡄𐡍 (~Parts[i] & ~unsigned{!i})
      أرجع خطأ;
  }

  // Set the unused high bits to all ones when we compare.
  ثابت unsigned NumHighBits = getNumHighBits();
  أكد(NumHighBits <= integerPartWidth && NumHighBits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ثابت integerPart HighBitFill = ~integerPart(0)
                                  << (integerPartWidth - NumHighBits);
  𐡄𐡍 (~(Parts[PartCount - 1] | HighBitFill | 0x1))
    أرجع خطأ;

  أرجع 𐡒𐡔𐡈;
}

منطقي IEEEFloat::isSignificandAllZeros() ثابت {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  ثابت integerPart *Parts = significandParts();
  ثابت unsigned PartCount = partCountForBits(semantics->precision);

  لكل (unsigned i = 0; i < PartCount - 1; i++)
    𐡄𐡍 (Parts[i])
      أرجع خطأ;

  // Compute how many bits are used in the final word.
  ثابت unsigned NumHighBits = getNumHighBits();
  أكد(NumHighBits < integerPartWidth && "Can not have more high bits to "
         "clear than integerPartWidth");
  ثابت integerPart HighBitMask = ~integerPart(0) >> NumHighBits;

  𐡄𐡍 ((semantics->precision > 1) && (Parts[PartCount - 1] & HighBitMask))
    أرجع خطأ;

  أرجع 𐡒𐡔𐡈;
}

منطقي IEEEFloat::isSignificandAllZerosExceptMSB() ثابت {
  ثابت integerPart *Parts = significandParts();
  ثابت unsigned PartCount = partCountForBits(semantics->precision);

  لكل (unsigned i = 0; i < PartCount - 1; i++) {
    𐡄𐡍 (Parts[i])
      أرجع خطأ;
  }

  ثابت unsigned NumHighBits = getNumHighBits();
  ثابت integerPart MSBMask = integerPart(1)
                              << (integerPartWidth - NumHighBits);
  أرجع ((semantics->precision <= 1) || (Parts[PartCount - 1] == MSBMask));
}

منطقي IEEEFloat::isLargest() ثابت {
  منطقي IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  𐡄𐡍 (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    أرجع (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } وإلا {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    أرجع IsMaxExp && isSignificandAllOnes();
  }
}

منطقي IEEEFloat::isInteger() ثابت {
  // This could be made more efficient; I'm going for obviously correct.
  𐡄𐡍 (!isFinite()) أرجع خطأ;
  IEEEFloat truncated = *هذا;
  truncated.roundToIntegral(rmTowardZero);
  أرجع compare(truncated) == cmpEqual;
}

منطقي IEEEFloat::bitwiseIsEqual(ثابت IEEEFloat &rhs) ثابت {
  𐡄𐡍 (هذا == &rhs)
    أرجع 𐡒𐡔𐡈;
  𐡄𐡍 (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    أرجع خطأ;
  𐡄𐡍 (category==fcZero || category==fcInfinity)
    أرجع 𐡒𐡔𐡈;

  𐡄𐡍 (isFiniteNonZero() && exponent != rhs.exponent)
    أرجع خطأ;

  أرجع std::equal(significandParts(), significandParts() + partCount(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics, integerPart value) {
  initialize(&ourSemantics);
  sign = 0;
  category = fcNormal;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = value;
  normalize(rmNearestTiesToEven, lfExactlyZero);
}

IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(خطأ) : makeSmallestNormalized(خطأ);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics, uninitializedTag tag)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(ثابت IEEEFloat &rhs) {
  initialize(rhs.semantics);
  assign(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloatBase::semBogus) {
  *هذا = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

unsigned صحيح IEEEFloat::partCount() ثابت {
  أرجع partCountForBits(semantics->precision + 1);
}

ثابت APFloat::integerPart *IEEEFloat::significandParts() ثابت {
  أرجع const_cast<IEEEFloat *>(هذا)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  𐡄𐡍 (partCount() > 1)
    أرجع significand.parts;
  وإلا
    أرجع &significand.part;
}

فراغ IEEEFloat::zeroSignificand() {
  APInt::tcSet(significandParts(), 0, partCount());
}

/* Increment an fcNormal floating point number's significand.  */
فراغ IEEEFloat::incrementSignificand() {
  [[maybe_unused]] integerPart carry =
      APInt::tcIncrement(significandParts(), partCount());

  /* Our callers should never cause us to overflow.  */
  أكد(carry == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::addSignificand(ثابت IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  أكد(semantics == rhs.semantics);
  أكد(exponent == rhs.exponent);

  أرجع APInt::tcAdd(parts, rhs.significandParts(), 0, partCount());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::subtractSignificand(ثابت IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  أكد(semantics == rhs.semantics);
  أكد(exponent == rhs.exponent);

  أرجع APInt::tcSubtract(parts, rhs.significandParts(), borrow,
                           partCount());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
lostFraction IEEEFloat::multiplySignificand(ثابت IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            منطقي ignoreAddend) {
  integerPart scratch[4];
  منطقي ignored;

  أكد(semantics == rhs.semantics);

  unsigned precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  unsigned newPartsCount = partCountForBits(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *fullSignificand =
      newPartsCount > 4 ? جديد integerPart[newPartsCount] : scratch;

  integerPart *lhsSignificand = significandParts();
  unsigned partsCount = partCount();

  APInt::tcFullMultiply(fullSignificand, lhsSignificand,
                        rhs.significandParts(), partsCount, partsCount);

  lostFraction lost_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  unsigned omsb = APInt::tcMSB(fullSignificand, newPartsCount) + 1;
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

  𐡄𐡍 (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    ثابت fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    unsigned extendedPrecision = 2 * precision + 1;
    𐡄𐡍 (omsb != extendedPrecision - 1) {
      أكد(extendedPrecision > omsb);
      APInt::tcShiftLeft(fullSignificand, newPartsCount,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    𐡄𐡍 (newPartsCount == 1)
      significand.part = fullSignificand[0];
    وإلا
      significand.parts = fullSignificand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_unused]] opStatus status = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    أكد(status == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    lost_fraction = extendedAddend.shiftSignificandRight(1);
    أكد(lost_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    lost_fraction = addOrSubtractSignificand(extendedAddend, خطأ);

    /* Restore our state.  */
    𐡄𐡍 (newPartsCount == 1)
      fullSignificand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(fullSignificand, newPartsCount) + 1;
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
  𐡄𐡍 (omsb > precision) {
    unsigned صحيح bits, significantParts;
    lostFraction lf;

    bits = omsb - precision;
    significantParts = partCountForBits(omsb);
    lf = shiftRight(fullSignificand, significantParts, bits);
    lost_fraction = combineLostFractions(lf, lost_fraction);
    exponent += bits;
  }

  APInt::tcAssign(lhsSignificand, fullSignificand, partsCount);

  𐡄𐡍 (newPartsCount > 4)
    delete [] fullSignificand;

  أرجع lost_fraction;
}

lostFraction IEEEFloat::multiplySignificand(ثابت IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  أرجع multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
lostFraction IEEEFloat::divideSignificand(ثابت IEEEFloat &rhs) {
  integerPart scratch[4];

  أكد(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  ثابت integerPart *rhsSignificand = rhs.significandParts();
  unsigned partsCount = partCount();

  integerPart *dividend =
      partsCount > 2 ? جديد integerPart[partsCount * 2] : scratch;
  integerPart *divisor = dividend + partsCount;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  لكل (unsigned i = 0; i < partsCount; i++) {
    dividend[i] = lhsSignificand[i];
    divisor[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  unsigned صحيح precision = semantics->precision;

  /* Normalize the divisor.  */
  unsigned bit = precision - APInt::tcMSB(divisor, partsCount) - 1;
  𐡄𐡍 (bit) {
    exponent += bit;
    APInt::tcShiftLeft(divisor, partsCount, bit);
  }

  /* Normalize the dividend.  */
  bit = precision - APInt::tcMSB(dividend, partsCount) - 1;
  𐡄𐡍 (bit) {
    exponent -= bit;
    APInt::tcShiftLeft(dividend, partsCount, bit);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  𐡄𐡍 (APInt::tcCompare(dividend, divisor, partsCount) < 0) {
    exponent--;
    APInt::tcShiftLeft(dividend, partsCount, 1);
    أكد(APInt::tcCompare(dividend, divisor, partsCount) >= 0);
  }

  /* Long division.  */
  لكل (bit = precision; bit; bit -= 1) {
    𐡄𐡍 (APInt::tcCompare(dividend, divisor, partsCount) >= 0) {
      APInt::tcSubtract(dividend, divisor, 0, partsCount);
      APInt::tcSetBit(lhsSignificand, bit - 1);
    }

    APInt::tcShiftLeft(dividend, partsCount, 1);
  }

  /* Figure out the lost fraction.  */
  صحيح cmp = APInt::tcCompare(dividend, divisor, partsCount);

  lostFraction lost_fraction;
  𐡄𐡍 (cmp > 0)
    lost_fraction = lfMoreThanHalf;
  وإلا 𐡄𐡍 (cmp == 0)
    lost_fraction = lfExactlyHalf;
  وإلا 𐡄𐡍 (APInt::tcIsZero(dividend, partsCount))
    lost_fraction = lfExactlyZero;
  وإلا
    lost_fraction = lfLessThanHalf;

  𐡄𐡍 (partsCount > 2)
    delete [] dividend;

  أرجع lost_fraction;
}

unsigned صحيح IEEEFloat::significandMSB() ثابت {
  أرجع APInt::tcMSB(significandParts(), partCount());
}

unsigned صحيح IEEEFloat::significandLSB() ثابت {
  أرجع APInt::tcLSB(significandParts(), partCount());
}

/* Note that a zero result is NOT normalized to fcZero.  */
lostFraction IEEEFloat::shiftSignificandRight(unsigned صحيح bits) {
  /* Our exponent should not overflow.  */
  أكد((ExponentType) (exponent + bits) >= exponent);

  exponent += bits;

  أرجع shiftRight(significandParts(), partCount(), bits);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
فراغ IEEEFloat::shiftSignificandLeft(unsigned صحيح bits) {
  أكد(bits < semantics->precision ||
         (semantics->precision == 1 && bits <= 1));

  𐡄𐡍 (bits) {
    unsigned صحيح partsCount = partCount();

    APInt::tcShiftLeft(significandParts(), partsCount, bits);
    exponent -= bits;

    أكد(!APInt::tcIsZero(significandParts(), partsCount));
  }
}

APFloat::cmpResult IEEEFloat::compareAbsoluteValue(ثابت IEEEFloat &rhs) ثابت {
  أكد(semantics == rhs.semantics);
  أكد(isFiniteNonZero());
  أكد(rhs.isFiniteNonZero());

  صحيح compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  𐡄𐡍 (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               partCount());

  𐡄𐡍 (compare > 0)
    أرجع cmpGreaterThan;
  وإلا 𐡄𐡍 (compare < 0)
    أرجع cmpLessThan;
  وإلا
    أرجع cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
ساكن فراغ tcSetLeastSignificantBits(APInt::WordType *dst, unsigned parts,
                                      unsigned bits) {
  unsigned i = 0;
  طالما (bits > APInt::APINT_BITS_PER_WORD) {
    dst[i++] = ~(APInt::WordType)0;
    bits -= APInt::APINT_BITS_PER_WORD;
  }

  𐡄𐡍 (bits)
    dst[i++] = ~(APInt::WordType)0 >> (APInt::APINT_BITS_PER_WORD - bits);

  طالما (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::opStatus IEEEFloat::handleOverflow(roundingMode rounding_mode) {
  𐡄𐡍 (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    𐡄𐡍 (rounding_mode == rmNearestTiesToEven ||
        rounding_mode == rmNearestTiesToAway ||
        (rounding_mode == rmTowardPositive && !sign) ||
        (rounding_mode == rmTowardNegative && sign)) {
      𐡄𐡍 (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(خطأ, sign);
      وإلا
        category = fcInfinity;
      أرجع static_cast<opStatus>(opOverflow | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fcNormal;
  exponent = semantics->maxExponent;
  tcSetLeastSignificantBits(significandParts(), partCount(),
                            semantics->precision);
  𐡄𐡍 (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tcClearBit(significandParts(), 0);

  أرجع opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
منطقي IEEEFloat::roundAwayFromZero(roundingMode rounding_mode,
                                  lostFraction lost_fraction,
                                  unsigned صحيح bit) ثابت {
  /* NaNs and infinities should not have lost fractions.  */
  أكد(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  أكد(lost_fraction != lfExactlyZero);

  بدل (rounding_mode) {
  حالة rmNearestTiesToAway:
    أرجع lost_fraction == lfExactlyHalf || lost_fraction == lfMoreThanHalf;

  حالة rmNearestTiesToEven:
    𐡄𐡍 (lost_fraction == lfMoreThanHalf)
      أرجع 𐡒𐡔𐡈;

    /* Our zeroes don't have a significand to test.  */
    𐡄𐡍 (lost_fraction == lfExactlyHalf && category != fcZero)
      أرجع APInt::tcExtractBit(significandParts(), bit);

    أرجع خطأ;

  حالة rmTowardZero:
    أرجع خطأ;

  حالة rmTowardPositive:
    أرجع !sign;

  حالة rmTowardNegative:
    أرجع sign;

  افتراضي:
    اكسر;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::opStatus IEEEFloat::normalize(roundingMode rounding_mode,
                                       lostFraction lost_fraction) {
  𐡄𐡍 (!isFiniteNonZero())
    أرجع opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  unsigned omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  𐡄𐡍 (omsb || lost_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    صحيح exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    𐡄𐡍 (exponent + exponentChange > semantics->maxExponent)
      أرجع handleOverflow(rounding_mode);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    𐡄𐡍 (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    𐡄𐡍 (exponentChange < 0) {
      أكد(lost_fraction == lfExactlyZero);

      shiftSignificandLeft(-exponentChange);

      أرجع opOK;
    }

    𐡄𐡍 (exponentChange > 0) {
      lostFraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = shiftSignificandRight(exponentChange);

      lost_fraction = combineLostFractions(lf, lost_fraction);

      /* Keep OMSB up-to-date.  */
      𐡄𐡍 (omsb > (unsigned) exponentChange)
        omsb -= exponentChange;
      وإلا
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  𐡄𐡍 (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    أرجع handleOverflow(rounding_mode);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  𐡄𐡍 (lost_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    𐡄𐡍 (omsb == 0) {
      category = fcZero;
      𐡄𐡍 (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = خطأ;
      𐡄𐡍 (!semantics->hasZero)
        makeSmallestNormalized(خطأ);
    }

    أرجع opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  𐡄𐡍 (roundAwayFromZero(rounding_mode, lost_fraction, 0)) {
    𐡄𐡍 (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    𐡄𐡍 (omsb == (unsigned) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      𐡄𐡍 (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        أرجع handleOverflow(sign ? rmTowardNegative : rmTowardPositive);

      shiftSignificandRight(1);

      أرجع opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    𐡄𐡍 (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      أرجع handleOverflow(rounding_mode);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  𐡄𐡍 (omsb == semantics->precision)
    أرجع opInexact;

  /* We have a non-zero denormal.  */
  أكد(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  𐡄𐡍 (omsb == 0) {
    category = fcZero;
    𐡄𐡍 (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = خطأ;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    𐡄𐡍 (!semantics->hasZero)
      makeSmallestNormalized(خطأ);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  أرجع (opStatus) (opUnderflow | opInexact);
}

APFloat::opStatus IEEEFloat::addOrSubtractSpecials(ثابت IEEEFloat &rhs,
                                                   منطقي subtract) {
  بدل (PackCategoriesIntoKey(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(nullptr);

  حالة PackCategoriesIntoKey(fcZero, fcNaN):
  حالة PackCategoriesIntoKey(fcNormal, fcNaN):
  حالة PackCategoriesIntoKey(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  حالة PackCategoriesIntoKey(fcNaN, fcZero):
  حالة PackCategoriesIntoKey(fcNaN, fcNormal):
  حالة PackCategoriesIntoKey(fcNaN, fcInfinity):
  حالة PackCategoriesIntoKey(fcNaN, fcNaN):
    𐡄𐡍 (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة PackCategoriesIntoKey(fcNormal, fcZero):
  حالة PackCategoriesIntoKey(fcInfinity, fcNormal):
  حالة PackCategoriesIntoKey(fcInfinity, fcZero):
    أرجع opOK;

  حالة PackCategoriesIntoKey(fcNormal, fcInfinity):
  حالة PackCategoriesIntoKey(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ subtract;
    أرجع opOK;

  حالة PackCategoriesIntoKey(fcZero, fcNormal):
    assign(rhs);
    sign = rhs.sign ^ subtract;
    أرجع opOK;

  حالة PackCategoriesIntoKey(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    أرجع opOK;

  حالة PackCategoriesIntoKey(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    𐡄𐡍 (((sign ^ rhs.sign)!=0) != subtract) {
      makeNaN();
      أرجع opInvalidOp;
    }

    أرجع opOK;

  حالة PackCategoriesIntoKey(fcNormal, fcNormal):
    أرجع opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
lostFraction IEEEFloat::addOrSubtractSignificand(ثابت IEEEFloat &rhs,
                                                 منطقي subtract) {
  [[maybe_unused]] integerPart carry = 0;
  lostFraction lost_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  subtract ^= static_cast<منطقي>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  صحيح bits = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  𐡄𐡍 (subtract) {
    𐡄𐡍 ((bits < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat temp_rhs(rhs);
    منطقي lost_fraction_is_from_rhs = خطأ;

    𐡄𐡍 (bits == 0)
      lost_fraction = lfExactlyZero;
    وإلا 𐡄𐡍 (bits > 0) {
      lost_fraction = temp_rhs.shiftSignificandRight(bits - 1);
      lost_fraction_is_from_rhs = 𐡒𐡔𐡈;
      shiftSignificandLeft(1);
    } وإلا {
      lost_fraction = shiftSignificandRight(-bits - 1);
      temp_rhs.shiftSignificandLeft(1);
    }

    // Should we reverse the subtraction.
    cmpResult cmp_result = compareAbsoluteValue(temp_rhs);
    𐡄𐡍 (cmp_result == cmpLessThan) {
      منطقي borrow =
          lost_fraction != lfExactlyZero && !lost_fraction_is_from_rhs;
      𐡄𐡍 (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        𐡄𐡍 (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        وإلا 𐡄𐡍 (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = temp_rhs.subtractSignificand(*هذا, borrow);
      copySignificand(temp_rhs);
      sign = !sign;
    } وإلا 𐡄𐡍 (cmp_result == cmpGreaterThan) {
      منطقي borrow = lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs;
      𐡄𐡍 (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        𐡄𐡍 (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        وإلا 𐡄𐡍 (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = subtractSignificand(temp_rhs, borrow);
    } وإلا { // cmpEqual
      zeroSignificand();
      𐡄𐡍 (lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    أكد(!carry);
  } وإلا {
    𐡄𐡍 (bits > 0) {
      IEEEFloat temp_rhs(rhs);

      lost_fraction = temp_rhs.shiftSignificandRight(bits);
      carry = addSignificand(temp_rhs);
    } وإلا {
      lost_fraction = shiftSignificandRight(-bits);
      carry = addSignificand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    أكد(!carry);
  }

  أرجع lost_fraction;
}

APFloat::opStatus IEEEFloat::multiplySpecials(ثابت IEEEFloat &rhs) {
  بدل (PackCategoriesIntoKey(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(nullptr);

  حالة PackCategoriesIntoKey(fcZero, fcNaN):
  حالة PackCategoriesIntoKey(fcNormal, fcNaN):
  حالة PackCategoriesIntoKey(fcInfinity, fcNaN):
    assign(rhs);
    sign = خطأ;
    [[fallthrough]];
  حالة PackCategoriesIntoKey(fcNaN, fcZero):
  حالة PackCategoriesIntoKey(fcNaN, fcNormal):
  حالة PackCategoriesIntoKey(fcNaN, fcInfinity):
  حالة PackCategoriesIntoKey(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    𐡄𐡍 (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة PackCategoriesIntoKey(fcNormal, fcInfinity):
  حالة PackCategoriesIntoKey(fcInfinity, fcNormal):
  حالة PackCategoriesIntoKey(fcInfinity, fcInfinity):
    category = fcInfinity;
    أرجع opOK;

  حالة PackCategoriesIntoKey(fcZero, fcNormal):
  حالة PackCategoriesIntoKey(fcNormal, fcZero):
  حالة PackCategoriesIntoKey(fcZero, fcZero):
    category = fcZero;
    أرجع opOK;

  حالة PackCategoriesIntoKey(fcZero, fcInfinity):
  حالة PackCategoriesIntoKey(fcInfinity, fcZero):
    makeNaN();
    أرجع opInvalidOp;

  حالة PackCategoriesIntoKey(fcNormal, fcNormal):
    أرجع opOK;
  }
}

APFloat::opStatus IEEEFloat::divideSpecials(ثابت IEEEFloat &rhs) {
  بدل (PackCategoriesIntoKey(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(nullptr);

  حالة PackCategoriesIntoKey(fcZero, fcNaN):
  حالة PackCategoriesIntoKey(fcNormal, fcNaN):
  حالة PackCategoriesIntoKey(fcInfinity, fcNaN):
    assign(rhs);
    sign = خطأ;
    [[fallthrough]];
  حالة PackCategoriesIntoKey(fcNaN, fcZero):
  حالة PackCategoriesIntoKey(fcNaN, fcNormal):
  حالة PackCategoriesIntoKey(fcNaN, fcInfinity):
  حالة PackCategoriesIntoKey(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    𐡄𐡍 (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة PackCategoriesIntoKey(fcInfinity, fcZero):
  حالة PackCategoriesIntoKey(fcInfinity, fcNormal):
  حالة PackCategoriesIntoKey(fcZero, fcInfinity):
  حالة PackCategoriesIntoKey(fcZero, fcNormal):
    أرجع opOK;

  حالة PackCategoriesIntoKey(fcNormal, fcInfinity):
    category = fcZero;
    أرجع opOK;

  حالة PackCategoriesIntoKey(fcNormal, fcZero):
    𐡄𐡍 (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(خطأ, sign);
    وإلا
      category = fcInfinity;
    أرجع opDivByZero;

  حالة PackCategoriesIntoKey(fcInfinity, fcInfinity):
  حالة PackCategoriesIntoKey(fcZero, fcZero):
    makeNaN();
    أرجع opInvalidOp;

  حالة PackCategoriesIntoKey(fcNormal, fcNormal):
    أرجع opOK;
  }
}

APFloat::opStatus IEEEFloat::modSpecials(ثابت IEEEFloat &rhs) {
  بدل (PackCategoriesIntoKey(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(nullptr);

  حالة PackCategoriesIntoKey(fcZero, fcNaN):
  حالة PackCategoriesIntoKey(fcNormal, fcNaN):
  حالة PackCategoriesIntoKey(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  حالة PackCategoriesIntoKey(fcNaN, fcZero):
  حالة PackCategoriesIntoKey(fcNaN, fcNormal):
  حالة PackCategoriesIntoKey(fcNaN, fcInfinity):
  حالة PackCategoriesIntoKey(fcNaN, fcNaN):
    𐡄𐡍 (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة PackCategoriesIntoKey(fcZero, fcInfinity):
  حالة PackCategoriesIntoKey(fcZero, fcNormal):
  حالة PackCategoriesIntoKey(fcNormal, fcInfinity):
    أرجع opOK;

  حالة PackCategoriesIntoKey(fcNormal, fcZero):
  حالة PackCategoriesIntoKey(fcInfinity, fcZero):
  حالة PackCategoriesIntoKey(fcInfinity, fcNormal):
  حالة PackCategoriesIntoKey(fcInfinity, fcInfinity):
  حالة PackCategoriesIntoKey(fcZero, fcZero):
    makeNaN();
    أرجع opInvalidOp;

  حالة PackCategoriesIntoKey(fcNormal, fcNormal):
    أرجع opOK;
  }
}

APFloat::opStatus IEEEFloat::remainderSpecials(ثابت IEEEFloat &rhs) {
  بدل (PackCategoriesIntoKey(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(nullptr);

  حالة PackCategoriesIntoKey(fcZero, fcNaN):
  حالة PackCategoriesIntoKey(fcNormal, fcNaN):
  حالة PackCategoriesIntoKey(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  حالة PackCategoriesIntoKey(fcNaN, fcZero):
  حالة PackCategoriesIntoKey(fcNaN, fcNormal):
  حالة PackCategoriesIntoKey(fcNaN, fcInfinity):
  حالة PackCategoriesIntoKey(fcNaN, fcNaN):
    𐡄𐡍 (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة PackCategoriesIntoKey(fcZero, fcInfinity):
  حالة PackCategoriesIntoKey(fcZero, fcNormal):
  حالة PackCategoriesIntoKey(fcNormal, fcInfinity):
    أرجع opOK;

  حالة PackCategoriesIntoKey(fcNormal, fcZero):
  حالة PackCategoriesIntoKey(fcInfinity, fcZero):
  حالة PackCategoriesIntoKey(fcInfinity, fcNormal):
  حالة PackCategoriesIntoKey(fcInfinity, fcInfinity):
  حالة PackCategoriesIntoKey(fcZero, fcZero):
    makeNaN();
    أرجع opInvalidOp;

  حالة PackCategoriesIntoKey(fcNormal, fcNormal):
    أرجع opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
فراغ IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  𐡄𐡍 (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    أرجع;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::opStatus IEEEFloat::addOrSubtract(ثابت IEEEFloat &rhs,
                                           roundingMode rounding_mode,
                                           منطقي subtract) {
  opStatus fs = addOrSubtractSpecials(rhs, subtract);

  /* This return code means it was not a simple case.  */
  𐡄𐡍 (fs == opDivByZero) {
    lostFraction lost_fraction;

    lost_fraction = addOrSubtractSignificand(rhs, subtract);
    fs = normalize(rounding_mode, lost_fraction);

    /* Can only be zero if we lost no fraction.  */
    أكد(category != fcZero || lost_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  𐡄𐡍 (category == fcZero) {
    𐡄𐡍 (rhs.category != fcZero || (sign == rhs.sign) == subtract)
      sign = (rounding_mode == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    𐡄𐡍 (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = خطأ;
  }

  أرجع fs;
}

/* Normalized addition.  */
APFloat::opStatus IEEEFloat::add(ثابت IEEEFloat &rhs,
                                 roundingMode rounding_mode) {
  أرجع addOrSubtract(rhs, rounding_mode, خطأ);
}

/* Normalized subtraction.  */
APFloat::opStatus IEEEFloat::subtract(ثابت IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  أرجع addOrSubtract(rhs, rounding_mode, 𐡒𐡔𐡈);
}

/* Normalized multiply.  */
APFloat::opStatus IEEEFloat::multiply(ثابت IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = multiplySpecials(rhs);

  𐡄𐡍 (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = خطأ;
  𐡄𐡍 (isFiniteNonZero()) {
    lostFraction lost_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    𐡄𐡍 (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  أرجع fs;
}

/* Normalized divide.  */
APFloat::opStatus IEEEFloat::divide(ثابت IEEEFloat &rhs,
                                    roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = divideSpecials(rhs);

  𐡄𐡍 (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = خطأ;
  𐡄𐡍 (isFiniteNonZero()) {
    lostFraction lost_fraction = divideSignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    𐡄𐡍 (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  أرجع fs;
}

/* Normalized remainder.  */
APFloat::opStatus IEEEFloat::remainder(ثابت IEEEFloat &rhs) {
  unsigned صحيح origSign = sign;

  // First handle the special cases.
  opStatus fs = remainderSpecials(rhs);
  𐡄𐡍 (fs != opDivByZero)
    أرجع fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  𐡄𐡍 (P2.add(rhs, rmNearestTiesToEven) == opOK) {
    fs = mod(P2);
    أكد(fs == opOK);
  }

  // Lets work with absolute numbers.
  IEEEFloat P = rhs;
  P.sign = خطأ;
  sign = خطأ;

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
  منطقي losesInfo;
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.maxExponent++;
  extendedSemantics.minExponent--;
  extendedSemantics.precision += 2;

  IEEEFloat VEx = *هذا;
  fs = VEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  أكد(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  أكد(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.add(VEx, rmNearestTiesToEven);
  أكد(fs == opOK);

  𐡄𐡍 (VEx.compare(PEx) == cmpGreaterThan) {
    fs = subtract(P, rmNearestTiesToEven);
    أكد(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    أكد(fs == opOK);
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    أكد(fs == opOK);

    cmpResult result = VEx.compare(PEx);
    𐡄𐡍 (result == cmpGreaterThan || result == cmpEqual) {
      fs = subtract(P, rmNearestTiesToEven);
      أكد(fs == opOK);
    }
  }

  𐡄𐡍 (isZero()) {
    sign = origSign;    // IEEE754 requires this
    𐡄𐡍 (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = خطأ;
  } وإلا {
    sign ^= origSign;
  }
  أرجع fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::opStatus IEEEFloat::mod(ثابت IEEEFloat &rhs) {
  opStatus fs = modSpecials(rhs);
  unsigned صحيح origSign = sign;

  طالما (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compareAbsoluteValue(rhs) != cmpLessThan) {
    صحيح Exp = ilogb(*هذا) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rmNearestTiesToEven);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    𐡄𐡍 (V.isNaN() || compareAbsoluteValue(V) == cmpLessThan)
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
    𐡄𐡍 (!semantics->hasZero && هذا->isSmallest())
      اكسر;

    أكد(fs==opOK);
  }
  𐡄𐡍 (isZero()) {
    sign = origSign; // fmod requires this
    𐡄𐡍 (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = خطأ;
  }
  أرجع fs;
}

/* Normalized fused-multiply-add.  */
APFloat::opStatus IEEEFloat::fusedMultiplyAdd(ثابت IEEEFloat &multiplicand,
                                              ثابت IEEEFloat &addend,
                                              roundingMode rounding_mode) {
  opStatus fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  𐡄𐡍 (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    lostFraction lost_fraction;

    lost_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_mode, lost_fraction);
    𐡄𐡍 (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    𐡄𐡍 (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_mode == rmTowardNegative);
      𐡄𐡍 (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = خطأ;
    }
  } وإلا {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    𐡄𐡍 (fs == opOK)
      fs = addOrSubtract(addend, rounding_mode, خطأ);
  }

  أرجع fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::opStatus IEEEFloat::roundToIntegral(roundingMode rounding_mode) {
  𐡄𐡍 (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    أرجع opOK;

  𐡄𐡍 (isNaN()) {
    𐡄𐡍 (isSignaling()) {
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
      أرجع opInvalidOp;
    } وإلا {
      // [IEEE Std 754-2008 6.2]:
      // For an operation with quiet NaN inputs, other than maximum and minimum
      // operations, if a floating-point result is to be delivered the result
      // shall be a quiet NaN which should be one of the input NaNs.
      // ...
      // Every general-computational and quiet-computational operation involving
      // one or more input NaNs, none of them signaling, shall signal no
      // exception, except fusedMultiplyAdd might signal the invalid operation
      // exception(see 7.2).
      أرجع opOK;
    }
  }

  𐡄𐡍 (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    أرجع opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  𐡄𐡍 (exponent + 1 >= (صحيح)APFloat::semanticsPrecision(*semantics))
    أرجع opOK;

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
  opStatus fs = MagicConstant.convertFromAPInt(IntegerConstant, خطأ,
                                               rmNearestTiesToEven);
  أكد(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  منطقي inputSign = isNegative();

  fs = add(MagicConstant, rounding_mode);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  subtract(MagicConstant, rounding_mode);

  // Restore the input sign.
  𐡄𐡍 (inputSign != isNegative())
    changeSign();

  أرجع fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmpResult IEEEFloat::compare(ثابت IEEEFloat &rhs) ثابت {
  أكد(semantics == rhs.semantics);

  بدل (PackCategoriesIntoKey(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(nullptr);

  حالة PackCategoriesIntoKey(fcNaN, fcZero):
  حالة PackCategoriesIntoKey(fcNaN, fcNormal):
  حالة PackCategoriesIntoKey(fcNaN, fcInfinity):
  حالة PackCategoriesIntoKey(fcNaN, fcNaN):
  حالة PackCategoriesIntoKey(fcZero, fcNaN):
  حالة PackCategoriesIntoKey(fcNormal, fcNaN):
  حالة PackCategoriesIntoKey(fcInfinity, fcNaN):
    أرجع cmpUnordered;

  حالة PackCategoriesIntoKey(fcInfinity, fcNormal):
  حالة PackCategoriesIntoKey(fcInfinity, fcZero):
  حالة PackCategoriesIntoKey(fcNormal, fcZero):
    𐡄𐡍 (sign)
      أرجع cmpLessThan;
    وإلا
      أرجع cmpGreaterThan;

  حالة PackCategoriesIntoKey(fcNormal, fcInfinity):
  حالة PackCategoriesIntoKey(fcZero, fcInfinity):
  حالة PackCategoriesIntoKey(fcZero, fcNormal):
    𐡄𐡍 (rhs.sign)
      أرجع cmpGreaterThan;
    وإلا
      أرجع cmpLessThan;

  حالة PackCategoriesIntoKey(fcInfinity, fcInfinity):
    𐡄𐡍 (sign == rhs.sign)
      أرجع cmpEqual;
    وإلا 𐡄𐡍 (sign)
      أرجع cmpLessThan;
    وإلا
      أرجع cmpGreaterThan;

  حالة PackCategoriesIntoKey(fcZero, fcZero):
    أرجع cmpEqual;

  حالة PackCategoriesIntoKey(fcNormal, fcNormal):
    اكسر;
  }

  cmpResult result;
  /* Two normal numbers.  Do they have the same sign?  */
  𐡄𐡍 (sign != rhs.sign) {
    𐡄𐡍 (sign)
      result = cmpLessThan;
    وإلا
      result = cmpGreaterThan;
  } وإلا {
    /* Compare absolute values; invert result if negative.  */
    result = compareAbsoluteValue(rhs);

    𐡄𐡍 (sign) {
      𐡄𐡍 (result == cmpLessThan)
        result = cmpGreaterThan;
      وإلا 𐡄𐡍 (result == cmpGreaterThan)
        result = cmpLessThan;
    }
  }

  أرجع result;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::opStatus IEEEFloat::convert(ثابت fltSemantics &toSemantics,
                                     roundingMode rounding_mode,
                                     منطقي *losesInfo) {
  opStatus fs;
  ثابت fltSemantics &fromSemantics = *semantics;
  منطقي is_signaling = isSignaling();

  lostFraction lostFraction = lfExactlyZero;
  unsigned newPartCount = partCountForBits(toSemantics.precision + 1);
  unsigned oldPartCount = partCount();
  صحيح shift = toSemantics.precision - fromSemantics.precision;

  منطقي X86SpecialNan = خطأ;
  𐡄𐡍 (&fromSemantics == &APFloatBase::semX87DoubleExtended &&
      &toSemantics != &APFloatBase::semX87DoubleExtended && category == fcNaN &&
      (!(*significandParts() & 0x8000000000000000ULL) ||
       !(*significandParts() & 0x4000000000000000ULL))) {
    // x86 has some unusual NaNs which cannot be represented in any other
    // format; note them here.
    X86SpecialNan = 𐡒𐡔𐡈;
  }

  // If this is a truncation of a denormal number, and the target semantics
  // has larger exponent range than the source semantics (this can happen
  // when truncating from PowerPC double-double to double format), the
  // right shift could lose result mantissa bits.  Adjust exponent instead
  // of performing excessive shift.
  // Also do a similar trick in case shifting denormal would produce zero
  // significand as this case isn't handled correctly by normalize.
  𐡄𐡍 (shift < 0 && isFiniteNonZero()) {
    صحيح omsb = significandMSB() + 1;
    صحيح exponentChange = omsb - fromSemantics.precision;
    𐡄𐡍 (exponent + exponentChange < toSemantics.minExponent)
      exponentChange = toSemantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, shift);
    𐡄𐡍 (exponentChange < 0) {
      shift -= exponentChange;
      exponent += exponentChange;
    } وإلا 𐡄𐡍 (omsb <= -shift) {
      exponentChange = omsb + shift - 1; // leave at least one bit set
      shift -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  𐡄𐡍 (shift < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    lostFraction = shiftRight(significandParts(), oldPartCount, -shift);

  // Fix the storage so it can hold to new value.
  𐡄𐡍 (newPartCount > oldPartCount) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = جديد integerPart[newPartCount];
    APInt::tcSet(newParts, 0, newPartCount);
    𐡄𐡍 (isFiniteNonZero() || category==fcNaN)
      APInt::tcAssign(newParts, significandParts(), oldPartCount);
    freeSignificand();
    significand.parts = newParts;
  } وإلا 𐡄𐡍 (newPartCount == 1 && oldPartCount != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    𐡄𐡍 (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &toSemantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  𐡄𐡍 (shift > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tcShiftLeft(significandParts(), newPartCount, shift);

  𐡄𐡍 (isFiniteNonZero()) {
    fs = normalize(rounding_mode, lostFraction);
    *losesInfo = (fs != opOK);
  } وإلا 𐡄𐡍 (category == fcNaN) {
    𐡄𐡍 (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(خطأ, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } وإلا {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      𐡄𐡍 (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(خطأ, خطأ);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      𐡄𐡍 (!APFloat::hasSignificand(fromSemantics))
        makeNaN(خطأ, sign);

      *losesInfo = lostFraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      𐡄𐡍 (!X86SpecialNan && semantics == &APFloatBase::semX87DoubleExtended)
        APInt::tcSetBit(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      𐡄𐡍 (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } وإلا {
        fs = opOK;
      }
    }
  } وإلا 𐡄𐡍 (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(خطأ, sign);
    *losesInfo = 𐡒𐡔𐡈;
    fs = opInexact;
  } وإلا 𐡄𐡍 (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = خطأ;
  } وإلا {
    *losesInfo = خطأ;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  𐡄𐡍 ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = 𐡒𐡔𐡈;
    𐡄𐡍 (fs == opOK)
      fs = opInexact;
  }

  𐡄𐡍 (category == fcZero && !semantics->hasZero)
    makeSmallestNormalized(خطأ);
  أرجع fs;
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
    MutableArrayRef<integerPart> parts, unsigned صحيح width, منطقي isSigned,
    roundingMode rounding_mode, منطقي *isExact) ثابت {
  *isExact = خطأ;

  /* Handle the three special cases first.  */
  𐡄𐡍 (category == fcInfinity || category == fcNaN)
    أرجع opInvalidOp;

  unsigned dstPartsCount = partCountForBits(width);
  أكد(dstPartsCount <= parts.size() && "Integer too big");

  𐡄𐡍 (category == fcZero) {
    APInt::tcSet(parts.data(), 0, dstPartsCount);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    أرجع opOK;
  }

  ثابت integerPart *src = significandParts();

  unsigned truncatedBits;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  𐡄𐡍 (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tcSet(parts.data(), 0, dstPartsCount);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncatedBits = semantics->precision -1U - exponent;
  } وإلا {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    unsigned صحيح bits = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    𐡄𐡍 (bits > width)
      أرجع opInvalidOp;

    𐡄𐡍 (bits < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncatedBits = semantics->precision - bits;
      APInt::tcExtract(parts.data(), dstPartsCount, src, bits, truncatedBits);
    } وإلا {
      /* We want at least as many bits as are available.  */
      APInt::tcExtract(parts.data(), dstPartsCount, src, semantics->precision,
                       0);
      APInt::tcShiftLeft(parts.data(), dstPartsCount,
                         bits - semantics->precision);
      truncatedBits = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  lostFraction lost_fraction;
  𐡄𐡍 (truncatedBits) {
    lost_fraction = lostFractionThroughTruncation(src, partCount(),
                                                  truncatedBits);
    𐡄𐡍 (lost_fraction != lfExactlyZero &&
        roundAwayFromZero(rounding_mode, lost_fraction, truncatedBits)) {
      𐡄𐡍 (APInt::tcIncrement(parts.data(), dstPartsCount))
        أرجع opInvalidOp;     /* Overflow.  */
    }
  } وإلا {
    lost_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  unsigned صحيح omsb = APInt::tcMSB(parts.data(), dstPartsCount) + 1;

  𐡄𐡍 (sign) {
    𐡄𐡍 (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      𐡄𐡍 (omsb != 0)
        أرجع opInvalidOp;
    } وإلا {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      𐡄𐡍 (omsb == width &&
          APInt::tcLSB(parts.data(), dstPartsCount) + 1 != omsb)
        أرجع opInvalidOp;

      /* This case can happen because of rounding.  */
      𐡄𐡍 (omsb > width)
        أرجع opInvalidOp;
    }

    APInt::tcNegate (parts.data(), dstPartsCount);
  } وإلا {
    𐡄𐡍 (omsb >= width + !isSigned)
      أرجع opInvalidOp;
  }

  𐡄𐡍 (lost_fraction == lfExactlyZero) {
    *isExact = 𐡒𐡔𐡈;
    أرجع opOK;
  }
  أرجع opInexact;
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
                            unsigned صحيح width, منطقي isSigned,
                            roundingMode rounding_mode, منطقي *isExact) ثابت {
  opStatus fs = convertToSignExtendedInteger(parts, width, isSigned,
                                             rounding_mode, isExact);

  𐡄𐡍 (fs == opInvalidOp) {
    unsigned صحيح bits, dstPartsCount;

    dstPartsCount = partCountForBits(width);
    أكد(dstPartsCount <= parts.size() && "Integer too big");

    𐡄𐡍 (category == fcNaN)
      bits = 0;
    وإلا 𐡄𐡍 (sign)
      bits = isSigned;
    وإلا
      bits = width - isSigned;

    tcSetLeastSignificantBits(parts.data(), dstPartsCount, bits);
    𐡄𐡍 (sign && isSigned)
      APInt::tcShiftLeft(parts.data(), dstPartsCount, width - 1);
  }

  أرجع fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::opStatus IEEEFloat::convertFromUnsignedParts(
    ثابت integerPart *src, unsigned صحيح srcCount, roundingMode rounding_mode) {
  category = fcNormal;
  unsigned omsb = APInt::tcMSB(src, srcCount) + 1;
  integerPart *dst = significandParts();
  unsigned dstCount = partCount();
  unsigned precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  lostFraction lost_fraction;
  𐡄𐡍 (precision <= omsb) {
    exponent = omsb - 1;
    lost_fraction = lostFractionThroughTruncation(src, srcCount,
                                                  omsb - precision);
    APInt::tcExtract(dst, dstCount, src, precision, omsb - precision);
  } وإلا {
    exponent = precision - 1;
    lost_fraction = lfExactlyZero;
    APInt::tcExtract(dst, dstCount, src, omsb, 0);
  }

  أرجع normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus IEEEFloat::convertFromAPInt(ثابت APInt &Val, منطقي isSigned,
                                              roundingMode rounding_mode) {
  unsigned صحيح partCount = Val.getNumWords();
  APInt api = Val;

  sign = خطأ;
  𐡄𐡍 (isSigned && api.isNegative()) {
    sign = 𐡒𐡔𐡈;
    api = -api;
  }

  أرجع convertFromUnsignedParts(api.getRawData(), partCount, rounding_mode);
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        roundingMode rounding_mode) {
  lostFraction lost_fraction = lfExactlyZero;

  category = fcNormal;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  unsigned partsCount = partCount();
  unsigned bitPos = partsCount * integerPartWidth;
  منطقي computedTrailingFraction = خطأ;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator end = s.end();
  StringRef::iterator dot;
  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  𐡄𐡍 (!PtrOrErr)
    أرجع PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator firstSignificantDigit = p;

  طالما (p != end) {
    integerPart hex_value;

    𐡄𐡍 (*p == '.') {
      𐡄𐡍 (dot != end)
        أرجع createError("String contains multiple dots");
      dot = p++;
      استمر;
    }

    hex_value = hexDigitValue(*p);
    𐡄𐡍 (hex_value == UINT_MAX)
      اكسر;

    p++;

    // Store the number while we have space.
    𐡄𐡍 (bitPos) {
      bitPos -= 4;
      hex_value <<= bitPos % integerPartWidth;
      significand[bitPos / integerPartWidth] |= hex_value;
    } وإلا 𐡄𐡍 (!computedTrailingFraction) {
      auto FractOrErr = trailingHexadecimalFraction(p, end, hex_value);
      𐡄𐡍 (!FractOrErr)
        أرجع FractOrErr.takeError();
      lost_fraction = *FractOrErr;
      computedTrailingFraction = 𐡒𐡔𐡈;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  𐡄𐡍 (p == end)
    أرجع createError("Hex strings require an exponent");
  𐡄𐡍 (*p != 'p' && *p != 'P')
    أرجع createError("Invalid character in significand");
  𐡄𐡍 (p == begin)
    أرجع createError("Significand has no digits");
  𐡄𐡍 (dot != end && p - begin == 1)
    أرجع createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  𐡄𐡍 (p != firstSignificantDigit) {
    صحيح expAdjustment;

    /* Implicit hexadecimal point?  */
    𐡄𐡍 (dot == end)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<صحيح>(dot - firstSignificantDigit);
    𐡄𐡍 (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= partsCount * integerPartWidth;

    /* Adjust for the given exponent.  */
    auto ExpOrErr = totalExponent(p + 1, end, expAdjustment);
    𐡄𐡍 (!ExpOrErr)
      أرجع ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  أرجع normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus
IEEEFloat::roundSignificandWithExponent(ثابت integerPart *decSigParts,
                                        unsigned sigPartCount, صحيح exp,
                                        roundingMode rounding_mode) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  unsigned صحيح power = exp >= 0 ? exp : -exp;
  unsigned صحيح pow5PartCapacity = powerOfFivePartsForExponent(power);
  SmallVector<integerPart, NumPow5PartsOnStack> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5PartCapacity);

  منطقي isNearest = rounding_mode == rmNearestTiesToEven ||
                   rounding_mode == rmNearestTiesToAway;

  unsigned parts = partCountForBits(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  unsigned pow5PartCount = powerOf5(pow5Parts.data(), power);

  لكل (;; parts *= 2) {
    unsigned صحيح excessPrecision, truncatedBits;

    calcSemantics.precision = parts * integerPartWidth - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncatedBits = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    opStatus sigStatus = decSig.convertFromUnsignedParts(
        decSigParts, sigPartCount, rmNearestTiesToEven);
    opStatus powStatus = pow5.convertFromUnsignedParts(
        pow5Parts.data(), pow5PartCount, rmNearestTiesToEven);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += exp;

    lostFraction calcLostFraction;
    integerPart HUerr, HUdistance;
    unsigned صحيح powHUerr;

    𐡄𐡍 (exp >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calcLostFraction = decSig.multiplySignificand(pow5);
      powHUerr = powStatus != opOK;
    } وإلا {
      calcLostFraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      𐡄𐡍 (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncatedBits = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (powStatus == opOK && calcLostFraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    أكد(APInt::tcExtractBit
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calcLostFraction != lfExactlyZero, sigStatus != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, isNearest);

    /* Are we guaranteed to round correctly if we truncate?  */
    𐡄𐡍 (HUdistance >= HUerr) {
      APInt::tcExtract(significandParts(), partCount(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calcLostFraction = lostFractionThroughTruncation(decSig.significandParts(),
                                                       decSig.partCount(),
                                                       truncatedBits);
      أرجع static_cast<opStatus>(normalize(rounding_mode, calcLostFraction) |
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
  𐡄𐡍 (Error Err = interpretDecimal(p, str.end(), &D))
    أرجع std::move(Err);

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
  𐡄𐡍 (D.firstSigDigit == str.end() || decDigitValue(*D.firstSigDigit) >= 10U) {
    category = fcZero;
    fs = opOK;
    𐡄𐡍 (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = خطأ;
    𐡄𐡍 (!semantics->hasZero)
      makeSmallestNormalized(خطأ);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } وإلا 𐡄𐡍 (D.normalizedExponent - 1 > INT_MAX / 42039) {
    fs = handleOverflow(rounding_mode);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } وإلا 𐡄𐡍 (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (صحيح) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fcNormal;
    zeroSignificand();
    fs = normalize(rounding_mode, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } وإلا 𐡄𐡍 ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handleOverflow(rounding_mode);
  } وإلا {
    integerPart *decSignificand;
    unsigned صحيح partCount;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    partCount = static_cast<unsigned صحيح>(D.lastSigDigit - D.firstSigDigit) + 1;
    partCount = partCountForBits(1 + 196 * partCount / 59);
    decSignificand = جديد integerPart[partCount + 1];
    partCount = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    𐡏𐡁𐡃 {
      integerPart decValue, val, multiplier;

      val = 0;
      multiplier = 1;

      𐡏𐡁𐡃 {
        𐡄𐡍 (*p == '.') {
          p++;
          𐡄𐡍 (p == str.end()) {
            اكسر;
          }
        }
        decValue = decDigitValue(*p++);
        𐡄𐡍 (decValue >= 10U) {
          delete[] decSignificand;
          أرجع createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + decValue;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } طالما (p <= D.lastSigDigit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            partCount, partCount + 1, خطأ);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      𐡄𐡍 (decSignificand[partCount])
        partCount++;
    } طالما (p <= D.lastSigDigit);

    category = fcNormal;
    fs = roundSignificandWithExponent(decSignificand, partCount,
                                      D.exponent, rounding_mode);

    delete [] decSignificand;
  }

  أرجع fs;
}

منطقي IEEEFloat::convertFromStringSpecials(StringRef str) {
  ثابت size_t MIN_NAME_SIZE = 3;

  𐡄𐡍 (str.size() < MIN_NAME_SIZE)
    أرجع خطأ;

  𐡄𐡍 (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(خطأ);
    أرجع 𐡒𐡔𐡈;
  }

  منطقي IsNegative = str.consume_front("-");
  𐡄𐡍 (IsNegative) {
    𐡄𐡍 (str.size() < MIN_NAME_SIZE)
      أرجع خطأ;

    𐡄𐡍 (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(𐡒𐡔𐡈);
      أرجع 𐡒𐡔𐡈;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  منطقي IsSignaling = str.consume_front_insensitive("s");
  𐡄𐡍 (IsSignaling) {
    𐡄𐡍 (str.size() < MIN_NAME_SIZE)
      أرجع خطأ;
  }

  𐡄𐡍 (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    𐡄𐡍 (str.empty()) {
      makeNaN(IsSignaling, IsNegative);
      أرجع 𐡒𐡔𐡈;
    }

    // Allow the payload to be inside parentheses.
    𐡄𐡍 (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      𐡄𐡍 (str.size() <= 2 || str.back() != ')')
        أرجع خطأ;

      str = str.slice(1, str.size() - 1);
    }

    // Determine the payload number's radix.
    unsigned Radix = 10;
    𐡄𐡍 (str[0] == '0') {
      𐡄𐡍 (str.size() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } وإلا {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    𐡄𐡍 (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      أرجع 𐡒𐡔𐡈;
    }
  }

  أرجع خطأ;
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromString(StringRef str, roundingMode rounding_mode) {
  𐡄𐡍 (str.empty())
    أرجع createError("Invalid string length");

  // Handle special cases.
  𐡄𐡍 (convertFromStringSpecials(str))
    أرجع opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  sign = *p == '-' ? 1 : 0;
  𐡄𐡍 (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  𐡄𐡍 (*p == '-' || *p == '+') {
    p++;
    slen--;
    𐡄𐡍 (!slen)
      أرجع createError("String has no digits");
  }

  𐡄𐡍 (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    𐡄𐡍 (slen == 2)
      أرجع createError("Invalid string");
    أرجع convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_mode);
  }

  أرجع convertFromDecimalString(StringRef(p, slen), rounding_mode);
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
unsigned صحيح IEEEFloat::convertToHexString(حرف *dst, unsigned صحيح hexDigits,
                                           منطقي upperCase,
                                           roundingMode rounding_mode) ثابت {
  حرف *p = dst;
  𐡄𐡍 (sign)
    *dst++ = '-';

  بدل (category) {
  حالة fcInfinity:
    memcpy (dst, upperCase ? infinityU: infinityL, حجم infinityU - 1);
    dst += حجم infinityL - 1;
    اكسر;

  حالة fcNaN:
    memcpy (dst, upperCase ? NaNU: NaNL, حجم NaNU - 1);
    dst += حجم NaNU - 1;
    اكسر;

  حالة fcZero:
    *dst++ = '0';
    *dst++ = upperCase ? 'X': 'x';
    *dst++ = '0';
    𐡄𐡍 (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = upperCase ? 'P': 'p';
    *dst++ = '0';
    اكسر;

  حالة fcNormal:
    dst = convertNormalToHexString (dst, hexDigits, upperCase, rounding_mode);
    اكسر;
  }

  *dst = 0;

  أرجع static_cast<unsigned صحيح>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
حرف *IEEEFloat::convertNormalToHexString(حرف *dst, unsigned صحيح hexDigits,
                                          منطقي upperCase,
                                          roundingMode rounding_mode) ثابت {
  *dst++ = '0';
  *dst++ = upperCase ? 'X': 'x';

  منطقي roundUp = خطأ;
  ثابت حرف *hexDigitChars = upperCase ? hexDigitsUpper : hexDigitsLower;

  ثابت integerPart *significand = significandParts();
  unsigned partsCount = partCount();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  unsigned valueBits = semantics->precision + 3;
  unsigned shift = integerPartWidth - valueBits % integerPartWidth;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  unsigned outputDigits = (valueBits - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  𐡄𐡍 (hexDigits) {
    𐡄𐡍 (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      unsigned صحيح bits;
      lostFraction fraction;

      bits = valueBits - hexDigits * 4;
      fraction = lostFractionThroughTruncation (significand, partsCount, bits);
      roundUp = roundAwayFromZero(rounding_mode, fraction, bits);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  حرف *p = ++dst;

  unsigned count = (valueBits + integerPartWidth - 1) / integerPartWidth;

  طالما (outputDigits && count) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    𐡄𐡍 (--count == partsCount)
      part = 0;  /* An imaginary higher zero part.  */
    وإلا
      part = significand[count] << shift;

    𐡄𐡍 (count && shift)
      part |= significand[count - 1] >> (integerPartWidth - shift);

    /* Convert as much of "part" to hexdigits as we can.  */
    unsigned صحيح curDigits = integerPartWidth / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  𐡄𐡍 (roundUp) {
    حرف *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    𐡏𐡁𐡃 {
      q--;
      *q = hexDigitChars[hexDigitValue (*q) + 1];
    } طالما (*q == '0');
    أكد(q >= p);
  } وإلا {
    /* Add trailing zeroes.  */
    memset (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  𐡄𐡍 (dst -1 == p)
    dst--;
  وإلا
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = upperCase ? 'P': 'p';

  أرجع writeSignedDecimal (dst, exponent);
}

hash_code hash_value(ثابت IEEEFloat &Arg) {
  𐡄𐡍 (!Arg.isFiniteNonZero())
    أرجع hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  أرجع hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.partCount()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convertF80LongDoubleAPFloatToAPInt() ثابت {
  أكد(partCount() == 2);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semX87DoubleExtended>();
}

APInt IEEEFloat::convertPPCDoubleDoubleLegacyAPFloatToAPInt() ثابت {
  أكد(semantics ==
         (ثابت llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy);
  أكد(partCount()==2);

  uint64_t words[2];
  منطقي losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloatBase::semIEEEdouble.minExponent;
  IEEEFloat extended(*هذا);
  [[maybe_unused]] opStatus fs =
      extended.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  أكد(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
  أكد(fs == opOK || fs == opInexact);
  words[0] = *u.convertDoubleAPFloatToAPInt().getRawData();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  𐡄𐡍 (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
    أكد(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.subtract(u, rmNearestTiesToEven);
    fs = v.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
    أكد(fs == opOK && !losesInfo);
    words[1] = *v.convertDoubleAPFloatToAPInt().getRawData();
  } وإلا {
    words[1] = 0;
  }

  أرجع APInt(128, words);
}

template <ثابت fltSemantics &S>
APInt IEEEFloat::convertIEEEFloatToAPInt() ثابت {
  أكد(semantics == &S);
  constexpr unsigned صحيح trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr صحيح integer_bit_part = (S.precision - 1) / integerPartWidth;
  constexpr integerPart integer_bit = integerPart{1}
                                      << ((S.precision - 1) % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned صحيح exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr منطقي is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr صحيح bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, partCountForBits(trailing_significand_bits)>
      mysignificand;

  𐡄𐡍 (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
    𐡄𐡍 (myexponent == 1 &&
        !(significandParts()[integer_bit_part] & integer_bit))
      myexponent = 0; // denormal
  } وإلا 𐡄𐡍 (category == fcZero) {
    𐡄𐡍 (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } وإلا 𐡄𐡍 (category == fcInfinity) {
    𐡄𐡍 (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    𐡄𐡍 constexpr (S.hasExplicitIntegerBit) {
      mysignificand[0] = integerPart{1} << (trailing_significand_bits - 1);
    }
  } وإلا {
    أكد(category == fcNaN && "Unknown category!");
    𐡄𐡍 (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.sizeInBits + 63) / 64> words;
  auto words_iter =
      std::copy_n(mysignificand.begin(), mysignificand.size(), words.begin());
  𐡄𐡍 constexpr (!S.hasExplicitIntegerBit) {
    𐡄𐡍 constexpr (significand_mask != 0 || trailing_significand_bits == 0) {
      // Clear the integer bit.
      words[mysignificand.size() - 1] &= significand_mask;
    }
  }
  std::fill(words_iter, words.end(), uint64_t{0});
  constexpr size_t last_word = words.size() - 1;
  𐡄𐡍 constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.sizeInBits - 1) % 64);
    words[last_word] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_mask)
                              << (trailing_significand_bits % 64);
  words[last_word] |= shifted_exponent;
  𐡄𐡍 constexpr (last_word == 0) {
    أرجع APInt(S.sizeInBits, words[0]);
  }
  أرجع APInt(S.sizeInBits, words);
}

APInt IEEEFloat::convertQuadrupleAPFloatToAPInt() ثابت {
  أكد(partCount() == 2);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semIEEEquad>();
}

APInt IEEEFloat::convertDoubleAPFloatToAPInt() ثابت {
  أكد(partCount()==1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semIEEEdouble>();
}

APInt IEEEFloat::convertFloatAPFloatToAPInt() ثابت {
  أكد(partCount()==1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semIEEEsingle>();
}

APInt IEEEFloat::convertBFloatAPFloatToAPInt() ثابت {
  أكد(partCount() == 1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semBFloat>();
}

APInt IEEEFloat::convertHalfAPFloatToAPInt() ثابت {
  أكد(partCount()==1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::APFloatBase::semIEEEhalf>();
}

APInt IEEEFloat::convertFloat8E5M2APFloatToAPInt() ثابت {
  أكد(partCount() == 1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2>();
}

APInt IEEEFloat::convertFloat8E5M2FNUZAPFloatToAPInt() ثابت {
  أكد(partCount() == 1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3APFloatToAPInt() ثابت {
  أكد(partCount() == 1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3>();
}

APInt IEEEFloat::convertFloat8E4M3FNAPFloatToAPInt() ثابت {
  أكد(partCount() == 1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FN>();
}

APInt IEEEFloat::convertFloat8E4M3FNUZAPFloatToAPInt() ثابت {
  أكد(partCount() == 1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3B11FNUZAPFloatToAPInt() ثابت {
  أكد(partCount() == 1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convertFloat8E3M4APFloatToAPInt() ثابت {
  أكد(partCount() == 1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semFloat8E3M4>();
}

APInt IEEEFloat::convertFloatTF32APFloatToAPInt() ثابت {
  أكد(partCount() == 1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semFloatTF32>();
}

APInt IEEEFloat::convertFloat8E8M0FNUAPFloatToAPInt() ثابت {
  أكد(partCount() == 1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convertFloat8E5M3FNUAPFloatToAPInt() ثابت {
  أكد(partCount() == 1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convertFloat6E3M2FNAPFloatToAPInt() ثابت {
  أكد(partCount() == 1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semFloat6E3M2FN>();
}

APInt IEEEFloat::convertFloat6E2M3FNAPFloatToAPInt() ثابت {
  أكد(partCount() == 1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semFloat6E2M3FN>();
}

APInt IEEEFloat::convertFloat4E2M1FNAPFloatToAPInt() ثابت {
  أكد(partCount() == 1);
  أرجع convertIEEEFloatToAPInt<APFloatBase::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcastToAPInt() ثابت {
  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEhalf)
    أرجع convertHalfAPFloatToAPInt();

  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semBFloat)
    أرجع convertBFloatAPFloatToAPInt();

  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEsingle)
    أرجع convertFloatAPFloatToAPInt();

  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEdouble)
    أرجع convertDoubleAPFloatToAPInt();

  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEquad)
    أرجع convertQuadrupleAPFloatToAPInt();

  𐡄𐡍 (semantics ==
      (ثابت llvm::fltSemantics *)&APFloatBase::semX87DoubleExtended)
    أرجع convertF80LongDoubleAPFloatToAPInt();

  𐡄𐡍 (semantics ==
      (ثابت llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy)
    أرجع convertPPCDoubleDoubleLegacyAPFloatToAPInt();

  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E5M2)
    أرجع convertFloat8E5M2APFloatToAPInt();

  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E5M2FNUZ)
    أرجع convertFloat8E5M2FNUZAPFloatToAPInt();

  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E4M3)
    أرجع convertFloat8E4M3APFloatToAPInt();

  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FN)
    أرجع convertFloat8E4M3FNAPFloatToAPInt();

  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FNUZ)
    أرجع convertFloat8E4M3FNUZAPFloatToAPInt();

  𐡄𐡍 (semantics ==
      (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E4M3B11FNUZ)
    أرجع convertFloat8E4M3B11FNUZAPFloatToAPInt();

  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E3M4)
    أرجع convertFloat8E3M4APFloatToAPInt();

  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloatTF32)
    أرجع convertFloatTF32APFloatToAPInt();

  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E8M0FNU)
    أرجع convertFloat8E8M0FNUAPFloatToAPInt();

  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E5M3FNU)
    أرجع convertFloat8E5M3FNUAPFloatToAPInt();

  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat6E3M2FN)
    أرجع convertFloat6E3M2FNAPFloatToAPInt();

  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat6E2M3FN)
    أرجع convertFloat6E2M3FNAPFloatToAPInt();

  𐡄𐡍 (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat4E2M1FN)
    أرجع convertFloat4E2M1FNAPFloatToAPInt();

  llvm_unreachable("unknown format!");
}

عائم IEEEFloat::convertToFloat() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcastToAPInt();
  أرجع api.bitsToFloat();
}

مزدوج IEEEFloat::convertToDouble() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcastToAPInt();
  أرجع api.bitsToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 IEEEFloat::convertToQuad() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcastToAPInt();
  أرجع api.bitsToQuad();
}
#endif

فراغ IEEEFloat::initFromF80LongDoubleAPInt(ثابت APInt &api) {
  أرجع initFromIEEEAPInt<APFloatBase::semX87DoubleExtended>(api);
}

فراغ IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(ثابت APInt &api) {
  uint64_t i1 = api.getRawData()[0];
  uint64_t i2 = api.getRawData()[1];
  منطقي losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_unused]] opStatus fs = convert(APFloatBase::semPPCDoubleDoubleLegacy,
                                         rmNearestTiesToEven, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  أكد((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  𐡄𐡍 (isFiniteNonZero()) {
    IEEEFloat v(APFloatBase::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    𐡄𐡍 (v.isFinite()) {
      fs = v.convert(APFloatBase::semPPCDoubleDoubleLegacy, rmNearestTiesToEven,
                     &losesInfo);
      أكد(fs == opOK && !losesInfo);

      add(v, rmNearestTiesToEven);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
فراغ IEEEFloat::initFromFloat8E8M0FNUAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E8M0FNU>(api);
}

فراغ IEEEFloat::initFromFloat8E5M3FNUAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M3FNU>(api);
}

template <ثابت fltSemantics &S>
فراغ IEEEFloat::initFromIEEEAPInt(ثابت APInt &api) {
  أكد(api.getBitWidth() == S.sizeInBits);

  constexpr unsigned صحيح trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr integerPart integer_bit =
      integerPart{1} << (trailing_significand_bits % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned صحيح exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr unsigned صحيح stored_significand_parts =
      partCountForBits(trailing_significand_bits + 1);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr منطقي is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr صحيح bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr منطقي has_significand = trailing_significand_bits > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  𐡄𐡍 constexpr (has_significand) {
    std::copy_n(api.getRawData(), mysignificand.size(), mysignificand.begin());
    𐡄𐡍 constexpr (significand_mask != 0 || S.precision >= integerPartWidth) {
      mysignificand[mysignificand.size() - 1] &= significand_mask;
    }
  } وإلا {
    std::fill_n(mysignificand.begin(), mysignificand.size(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t last_word = api.getRawData()[api.getNumWords() - 1];
  uint64_t myexponent =
      (last_word >> (trailing_significand_bits % 64)) & exponent_mask;

  initialize(&S);
  أكد(partCount() == mysignificand.size());

  sign = S.hasSignedRepr
             ? static_cast<unsigned صحيح>(last_word >> ((S.sizeInBits - 1) % 64))
             : 0;

  منطقي all_zero_significand =
      has_significand && llvm::all_of(mysignificand, equal_to(0));

  منطقي is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  𐡄𐡍 constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    منطقي is_inf = خطأ;

    𐡄𐡍 constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;
      ثابت integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_mask_no_int_bit) == 0;
    } وإلا {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    𐡄𐡍 (is_inf) {
      makeInf(sign);
      أرجع;
    }
  }

  منطقي is_nan = خطأ;

  𐡄𐡍 constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    𐡄𐡍 constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      ثابت integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;

      𐡄𐡍 (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_mask_no_int_bit) != 0) {
        // regular NaN and pseudoNaN
        is_nan = 𐡒𐡔𐡈;
      } وإلا 𐡄𐡍 (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_mask_no_int_bit) == 0) {
        // pseudoinfinity
        is_nan = 𐡒𐡔𐡈;
      } وإلا 𐡄𐡍 (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = 𐡒𐡔𐡈;
      }
    } وإلا {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } وإلا 𐡄𐡍 constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    منطقي all_ones_significand =
        std::all_of(mysignificand.begin(), mysignificand.end() - 1,
                    [](integerPart bits) { أرجع bits == ~integerPart{0}; }) &&
        (!significand_mask ||
         mysignificand[mysignificand.size() - 1] == significand_mask);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } وإلا 𐡄𐡍 constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  𐡄𐡍 (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.size(),
                significandParts());
    أرجع;
  }

  𐡄𐡍 (is_zero) {
    makeZero(sign);
    أرجع;
  }

  category = fcNormal;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.size(), significandParts());
  𐡄𐡍 (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  وإلا {
    𐡄𐡍 constexpr (!S.hasExplicitIntegerBit) {
      significandParts()[mysignificand.size() - 1] |= integer_bit;
    }
  }
}

فراغ IEEEFloat::initFromQuadrupleAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEquad>(api);
}

فراغ IEEEFloat::initFromDoubleAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEdouble>(api);
}

فراغ IEEEFloat::initFromFloatAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEsingle>(api);
}

فراغ IEEEFloat::initFromBFloatAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semBFloat>(api);
}

فراغ IEEEFloat::initFromHalfAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEhalf>(api);
}

فراغ IEEEFloat::initFromFloat8E5M2APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2>(api);
}

فراغ IEEEFloat::initFromFloat8E5M2FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2FNUZ>(api);
}

فراغ IEEEFloat::initFromFloat8E4M3APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3>(api);
}

فراغ IEEEFloat::initFromFloat8E4M3FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FN>(api);
}

فراغ IEEEFloat::initFromFloat8E4M3FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FNUZ>(api);
}

فراغ IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3B11FNUZ>(api);
}

فراغ IEEEFloat::initFromFloat8E3M4APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E3M4>(api);
}

فراغ IEEEFloat::initFromFloatTF32APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloatTF32>(api);
}

فراغ IEEEFloat::initFromFloat6E3M2FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E3M2FN>(api);
}

فراغ IEEEFloat::initFromFloat6E2M3FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E2M3FN>(api);
}

فراغ IEEEFloat::initFromFloat4E2M1FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
فراغ IEEEFloat::initFromAPInt(ثابت fltSemantics *Sem, ثابت APInt &api) {
  أكد(api.getBitWidth() == Sem->sizeInBits);
  𐡄𐡍 (Sem == &APFloatBase::semIEEEhalf)
    أرجع initFromHalfAPInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semBFloat)
    أرجع initFromBFloatAPInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semIEEEsingle)
    أرجع initFromFloatAPInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semIEEEdouble)
    أرجع initFromDoubleAPInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semIEEEquad)
    أرجع initFromQuadrupleAPInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semX87DoubleExtended)
    أرجع initFromF80LongDoubleAPInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semPPCDoubleDoubleLegacy)
    أرجع initFromPPCDoubleDoubleLegacyAPInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semFloat8E5M2)
    أرجع initFromFloat8E5M2APInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semFloat8E5M2FNUZ)
    أرجع initFromFloat8E5M2FNUZAPInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semFloat8E4M3)
    أرجع initFromFloat8E4M3APInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semFloat8E4M3FN)
    أرجع initFromFloat8E4M3FNAPInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semFloat8E4M3FNUZ)
    أرجع initFromFloat8E4M3FNUZAPInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semFloat8E4M3B11FNUZ)
    أرجع initFromFloat8E4M3B11FNUZAPInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semFloat8E3M4)
    أرجع initFromFloat8E3M4APInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semFloatTF32)
    أرجع initFromFloatTF32APInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semFloat8E8M0FNU)
    أرجع initFromFloat8E8M0FNUAPInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semFloat8E5M3FNU)
    أرجع initFromFloat8E5M3FNUAPInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semFloat6E3M2FN)
    أرجع initFromFloat6E3M2FNAPInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semFloat6E2M3FN)
    أرجع initFromFloat6E2M3FNAPInt(api);
  𐡄𐡍 (Sem == &APFloatBase::semFloat4E2M1FN)
    أرجع initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
فراغ IEEEFloat::makeLargest(منطقي Negative) {
  𐡄𐡍 (Negative && !semantics->hasSignedRepr)
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
  unsigned PartCount = partCount();
  memset(significand, 0xFF, حجم(integerPart)*(PartCount - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  ثابت unsigned NumUnusedHighBits =
    PartCount*integerPartWidth - semantics->precision;
  significand[PartCount - 1] = (NumUnusedHighBits < integerPartWidth)
                                   ? (~integerPart(0) >> NumUnusedHighBits)
                                   : 0;
  𐡄𐡍 (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
فراغ IEEEFloat::makeSmallest(منطقي Negative) {
  𐡄𐡍 (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fcNormal;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tcSet(significandParts(), 1, partCount());
}

فراغ IEEEFloat::makeSmallestNormalized(منطقي Negative) {
  𐡄𐡍 (Negative && !semantics->hasSignedRepr)
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

IEEEFloat::IEEEFloat(ثابت fltSemantics &Sem, ثابت APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(عائم f) {
  initFromAPInt(&APFloatBase::semIEEEsingle, APInt::floatToBits(f));
}

IEEEFloat::IEEEFloat(مزدوج d) {
  initFromAPInt(&APFloatBase::semIEEEdouble, APInt::doubleToBits(d));
}

نطاق {
  فراغ append(SmallVectorImpl<حرف> &Buffer, StringRef Str) {
    Buffer.append(Str.begin(), Str.end());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  فراغ AdjustToPrecision(APInt &significand,
                         صحيح &exp, unsigned FormatPrecision) {
    unsigned bits = significand.getActiveBits();

    // 196/59 is a very slight overestimate of lg_2(10).
    unsigned bitsRequired = (FormatPrecision * 196 + 58) / 59;

    𐡄𐡍 (bits <= bitsRequired) أرجع;

    unsigned tensRemovable = (bits - bitsRequired) * 59 / 196;
    𐡄𐡍 (!tensRemovable) أرجع;

    exp += tensRemovable;

    APInt divisor(significand.getBitWidth(), 1);
    APInt powten(significand.getBitWidth(), 10);
    طالما (𐡒𐡔𐡈) {
      𐡄𐡍 (tensRemovable & 1)
        divisor *= powten;
      tensRemovable >>= 1;
      𐡄𐡍 (!tensRemovable) اكسر;
      powten *= powten;
    }

    significand = significand.udiv(divisor);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.getActiveBits());
  }


  فراغ AdjustToPrecision(SmallVectorImpl<حرف> &buffer,
                         صحيح &exp, unsigned FormatPrecision) {
    unsigned N = buffer.size();
    𐡄𐡍 (N <= FormatPrecision) أرجع;

    // The most significant figures are the last ones in the buffer.
    unsigned FirstSignificant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    𐡄𐡍 (buffer[FirstSignificant - 1] < '5') {
      طالما (FirstSignificant < N && buffer[FirstSignificant] == '0')
        FirstSignificant++;

      exp += FirstSignificant;
      buffer.erase(&buffer[0], &buffer[FirstSignificant]);
      أرجع;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    لكل (unsigned I = FirstSignificant; I != N; ++I) {
      𐡄𐡍 (buffer[I] == '9') {
        FirstSignificant++;
      } وإلا {
        buffer[I]++;
        اكسر;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    𐡄𐡍 (FirstSignificant == N) {
      exp += FirstSignificant;
      buffer.clear();
      buffer.push_back('1');
      أرجع;
    }

    exp += FirstSignificant;
    buffer.erase(&buffer[0], &buffer[FirstSignificant]);
  }

  فراغ toStringImpl(SmallVectorImpl<حرف> &Str, ثابت منطقي isNeg, صحيح exp,
                    APInt significand, unsigned FormatPrecision,
                    unsigned FormatMaxPadding, منطقي TruncateZero) {
    ثابت صحيح semanticsPrecision = significand.getBitWidth();

    𐡄𐡍 (isNeg)
      Str.push_back('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    𐡄𐡍 (!FormatPrecision) {
      // We use enough digits so the number can be round-tripped back to an
      // APFloat. The formula comes from "How to Print Floating-Point Numbers
      // Accurately" by Steele and White.
      // FIXME: Using a formula based purely on the precision is conservative;
      // we can print fewer digits depending on the actual value being printed.

      // FormatPrecision = 2 + floor(significandBits / lg_2(10))
      FormatPrecision = 2 + semanticsPrecision * 59 / 196;
    }

    // Ignore trailing binary zeros.
    صحيح trailingZeros = significand.countr_zero();
    exp += trailingZeros;
    significand.lshrInPlace(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    𐡄𐡍 (exp == 0) {
      // Nothing to do.
    } وإلا 𐡄𐡍 (exp > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + exp);
      significand <<= exp;
      exp = 0;
    } وإلا { /* exp < 0 */
      صحيح texp = -exp;

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
      طالما (𐡒𐡔𐡈) {
        𐡄𐡍 (texp & 1)
          significand *= five_to_the_i;

        texp >>= 1;
        𐡄𐡍 (!texp)
          اكسر;
        five_to_the_i *= five_to_the_i;
      }
    }

    AdjustToPrecision(significand, exp, FormatPrecision);

    SmallVector<حرف, 256> buffer;

    // Fill the buffer.
    unsigned precision = significand.getBitWidth();
    𐡄𐡍 (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    منطقي inTrail = 𐡒𐡔𐡈;
    طالما (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      unsigned d = digit.getZExtValue();

      // Drop trailing zeros.
      𐡄𐡍 (inTrail && !d)
        exp++;
      وإلا {
        buffer.push_back((حرف) ('0' + d));
        inTrail = خطأ;
      }
    }

    أكد(!buffer.empty() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    AdjustToPrecision(buffer, exp, FormatPrecision);

    unsigned NDigits = buffer.size();

    // Check whether we should use scientific notation.
    منطقي FormatScientific;
    𐡄𐡍 (!FormatMaxPadding) {
      FormatScientific = 𐡒𐡔𐡈;
    } وإلا {
      𐡄𐡍 (exp >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((unsigned) exp > FormatMaxPadding ||
                            NDigits + (unsigned) exp > FormatPrecision);
      } وإلا {
        // Power of the most significant digit.
        صحيح MSD = exp + (صحيح) (NDigits - 1);
        𐡄𐡍 (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = خطأ;
        } وإلا {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((unsigned) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    𐡄𐡍 (FormatScientific) {
      exp += (NDigits - 1);

      Str.push_back(buffer[NDigits-1]);
      Str.push_back('.');
      𐡄𐡍 (NDigits == 1 && TruncateZero)
        Str.push_back('0');
      وإلا
        لكل (unsigned I = 1; I != NDigits; ++I)
          Str.push_back(buffer[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      𐡄𐡍 (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.push_back(TruncateZero ? 'E' : 'e');

      Str.push_back(exp >= 0 ? '+' : '-');
      𐡄𐡍 (exp < 0)
        exp = -exp;
      SmallVector<حرف, 6> expbuf;
      𐡏𐡁𐡃 {
        expbuf.push_back((حرف) ('0' + (exp % 10)));
        exp /= 10;
      } طالما (exp);
      // Exponent always at least two digits if we do not truncate zeros.
      𐡄𐡍 (!TruncateZero && expbuf.size() < 2)
        expbuf.push_back('0');
      لكل (unsigned I = 0, E = expbuf.size(); I != E; ++I)
        Str.push_back(expbuf[E-1-I]);
      أرجع;
    }

    // Non-scientific, positive exponents.
    𐡄𐡍 (exp >= 0) {
      لكل (unsigned I = 0; I != NDigits; ++I)
        Str.push_back(buffer[NDigits-1-I]);
      لكل (unsigned I = 0; I != (unsigned) exp; ++I)
        Str.push_back('0');
      أرجع;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    صحيح NWholeDigits = exp + (صحيح) NDigits;

    unsigned I = 0;
    𐡄𐡍 (NWholeDigits > 0) {
      لكل (; I != (unsigned) NWholeDigits; ++I)
        Str.push_back(buffer[NDigits-I-1]);
      Str.push_back('.');
    } وإلا {
      unsigned NZeros = 1 + (unsigned) -NWholeDigits;

      Str.push_back('0');
      Str.push_back('.');
      لكل (unsigned Z = 1; Z != NZeros; ++Z)
        Str.push_back('0');
    }

    لكل (; I != NDigits; ++I)
      Str.push_back(buffer[NDigits-I-1]);

  }
} // namespace

فراغ IEEEFloat::toString(SmallVectorImpl<حرف> &Str, unsigned FormatPrecision,
                         unsigned FormatMaxPadding, منطقي TruncateZero) ثابت {
  بدل (category) {
  حالة fcInfinity:
    𐡄𐡍 (isNegative())
      أرجع append(Str, "-Inf");
    وإلا
      أرجع append(Str, "+Inf");

  حالة fcNaN: أرجع append(Str, "NaN");

  حالة fcZero:
    𐡄𐡍 (isNegative())
      Str.push_back('-');

    𐡄𐡍 (!FormatMaxPadding) {
      𐡄𐡍 (TruncateZero)
        append(Str, "0.0E+0");
      وإلا {
        append(Str, "0.0");
        𐡄𐡍 (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } وإلا {
      Str.push_back('0');
    }
    أرجع;

  حالة fcNormal:
    اكسر;
  }

  // Decompose the number into an APInt and an exponent.
  صحيح exp = exponent - ((صحيح) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), partCountForBits(semantics->precision)));

  toStringImpl(Str, isNegative(), exp, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

صحيح IEEEFloat::getExactLog2Abs() ثابت {
  𐡄𐡍 (!isFinite() || isZero())
    أرجع INT_MIN;

  ثابت integerPart *Parts = significandParts();
  ثابت صحيح PartCount = partCountForBits(semantics->precision);

  صحيح PopCount = 0;
  لكل (صحيح i = 0; i < PartCount; ++i) {
    PopCount += llvm::popcount(Parts[i]);
    𐡄𐡍 (PopCount > 1)
      أرجع INT_MIN;
  }

  𐡄𐡍 (exponent != semantics->minExponent)
    أرجع exponent;

  صحيح CountrParts = 0;
  لكل (صحيح i = 0; i < PartCount;
       ++i, CountrParts += APInt::APINT_BITS_PER_WORD) {
    𐡄𐡍 (Parts[i] != 0) {
      أرجع exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

منطقي IEEEFloat::isSignaling() ثابت {
  𐡄𐡍 (!isNaN())
    أرجع خطأ;
  𐡄𐡍 (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    أرجع خطأ;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  أرجع !APInt::tcExtractBit(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::opStatus IEEEFloat::next(منطقي nextDown) {
  // If we are performing nextDown, swap sign so we have -x.
  𐡄𐡍 (nextDown)
    changeSign();

  // Compute nextUp(x)
  opStatus result = opOK;

  // Handle each float category separately.
  بدل (category) {
  حالة fcInfinity:
    // nextUp(+inf) = +inf
    𐡄𐡍 (!isNegative())
      اكسر;
    // nextUp(-inf) = -getLargest()
    makeLargest(𐡒𐡔𐡈);
    اكسر;
  حالة fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    𐡄𐡍 (isSignaling()) {
      result = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(خطأ, isNegative(), nullptr);
    }
    اكسر;
  حالة fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(خطأ);
    اكسر;
  حالة fcNormal:
    // nextUp(-getSmallest()) = -0
    𐡄𐡍 (isSmallest() && isNegative()) {
      APInt::tcSet(significandParts(), 0, partCount());
      category = fcZero;
      exponent = 0;
      𐡄𐡍 (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = خطأ;
      𐡄𐡍 (!semantics->hasZero)
        makeSmallestNormalized(خطأ);
      اكسر;
    }

    𐡄𐡍 (isLargest() && !isNegative()) {
      𐡄𐡍 (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        اكسر;
      } وإلا 𐡄𐡍 (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        اكسر;
      } وإلا {
        // nextUp(getLargest()) == INFINITY
        APInt::tcSet(significandParts(), 0, partCount());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        اكسر;
      }
    }

    // nextUp(normal) == normal + inc.
    𐡄𐡍 (isNegative()) {
      // If we are negative, we need to decrement the significand.

      // We only cross a binade boundary that requires adjusting the exponent
      // if:
      //   1. exponent != semantics->minExponent. This implies we are not in the
      //   smallest binade or are dealing with denormals.
      //   2. Our significand excluding the integral bit is all zeros.
      منطقي WillCrossBinadeBoundary =
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
      APInt::tcDecrement(Parts, partCount());

      𐡄𐡍 (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tcSetBit(Parts, semantics->precision - 1);
        exponent--;
      }
    } وإلا {
      // If we are positive, we need to increment the significand.

      // We only cross a binade boundary that requires adjusting the exponent if
      // the input is not a denormal and all of said input's significand bits
      // are set. If all of said conditions are true: clear the significand, set
      // the integral bit to 1, and increment the exponent. If we have a
      // denormal always increment since moving denormals and the numbers in the
      // smallest normal binade have the same exponent in our representation.
      // If there are only exponents, any increment always crosses the
      // BinadeBoundary.
      منطقي WillCrossBinadeBoundary = !APFloat::hasSignificand(*semantics) ||
                                     (!isDenormal() && isSignificandAllOnes());

      𐡄𐡍 (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tcSet(Parts, 0, partCount());
        APInt::tcSetBit(Parts, semantics->precision - 1);
        أكد(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } وإلا {
        incrementSignificand();
      }
    }
    اكسر;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  𐡄𐡍 (nextDown)
    changeSign();

  أرجع result;
}

APInt IEEEFloat::getNaNPayload() ثابت {
  أكد(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  unsigned Bits = semantics->precision - 1;
  أرجع APInt(Bits, ArrayRef(significandParts(), partCountForBits(Bits)));
}

APFloatBase::ExponentType IEEEFloat::exponentNaN() ثابت {
  أرجع ::exponentNaN(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentInf() ثابت {
  أرجع ::exponentInf(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentZero() ثابت {
  أرجع ::exponentZero(*semantics);
}

فراغ IEEEFloat::makeInf(منطقي Negative) {
  𐡄𐡍 (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  𐡄𐡍 (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(خطأ, Negative);
    أرجع;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tcSet(significandParts(), 0, partCount());
}

فراغ IEEEFloat::makeZero(منطقي Negative) {
  𐡄𐡍 (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  𐡄𐡍 (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = خطأ;
  }
  exponent = exponentZero();
  APInt::tcSet(significandParts(), 0, partCount());
}

فراغ IEEEFloat::makeQuiet() {
  أكد(isNaN());
  𐡄𐡍 (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tcSetBit(significandParts(), semantics->precision - 2);
}

صحيح ilogb(ثابت IEEEFloat &Arg) {
  𐡄𐡍 (Arg.isNaN())
    أرجع APFloat::IEK_NaN;
  𐡄𐡍 (Arg.isZero())
    أرجع APFloat::IEK_Zero;
  𐡄𐡍 (Arg.isInfinity())
    أرجع APFloat::IEK_Inf;
  𐡄𐡍 (!Arg.isDenormal())
    أرجع Arg.exponent;

  IEEEFloat Normalized(Arg);
  صحيح SignificandBits = Arg.getSemantics().precision - 1;

  Normalized.exponent += SignificandBits;
  Normalized.normalize(APFloat::rmNearestTiesToEven, lfExactlyZero);
  أرجع Normalized.exponent - SignificandBits;
}

IEEEFloat scalbn(IEEEFloat X, صحيح Exp, roundingMode RoundingMode) {
  auto MaxExp = X.getSemantics().maxExponent;
  auto MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  صحيح SignificandBits = X.getSemantics().precision - 1;
  صحيح MaxIncrement = MaxExp - (MinExp - SignificandBits) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(RoundingMode, lfExactlyZero);
  𐡄𐡍 (X.isNaN())
    X.makeQuiet();
  أرجع X;
}

IEEEFloat frexp(ثابت IEEEFloat &Val, صحيح &Exp, roundingMode RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  𐡄𐡍 (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    أرجع Quiet;
  }

  𐡄𐡍 (Exp == APFloat::IEK_Inf)
    أرجع Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  أرجع scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S)
    : Semantics(&S),
      Floats(جديد APFloat[2]{APFloat(APFloatBase::semIEEEdouble),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, uninitializedTag)
    : Semantics(&S), Floats(جديد APFloat[2]{
                         APFloat(APFloatBase::semIEEEdouble, uninitialized),
                         APFloat(APFloatBase::semIEEEdouble, uninitialized)}) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(جديد APFloat[2]{APFloat(APFloatBase::semIEEEdouble, I),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, ثابت APInt &I)
    : Semantics(&S),
      Floats(جديد APFloat[2]{
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[0])),
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[1]))}) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, APFloat &&First,
                             APFloat &&Second)
    : Semantics(&S),
      Floats(جديد APFloat[2]{std::move(First), std::move(Second)}) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble);
  أكد(&Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  أكد(&Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? جديد APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : nullptr) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloatBase::semBogus;
  RHS.Floats = nullptr;
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::operator=(ثابت DoubleAPFloat &RHS) {
  𐡄𐡍 (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } وإلا 𐡄𐡍 (هذا != &RHS) {
    هذا->~DoubleAPFloat();
    جديد (هذا) DoubleAPFloat(RHS);
  }
  أرجع *هذا;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
ساكن std::pair<APFloat, APFloat> fastTwoSum(APFloat X, APFloat Y) {
  𐡄𐡍 (!X.isFinite())
    أرجع {X, APFloat::getZero(X.getSemantics(), /*Negative=*/خطأ)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  أرجع {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::opStatus DoubleAPFloat::addImpl(ثابت APFloat &a, ثابت APFloat &aa,
                                         ثابت APFloat &c, ثابت APFloat &cc,
                                         roundingMode RM) {
  صحيح Status = opOK;
  APFloat z = a;
  Status |= z.add(c, RM);
  𐡄𐡍 (!z.isFinite()) {
    𐡄𐡍 (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ خطأ);
      أرجع (opStatus)Status;
    }
    Status = opOK;
    auto AComparedToC = a.compareAbsoluteValue(c);
    z = cc;
    Status |= z.add(aa, RM);
    𐡄𐡍 (AComparedToC == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      Status |= z.add(c, RM);
      Status |= z.add(a, RM);
    } وإلا {
      // z = cc + aa + a + c;
      Status |= z.add(a, RM);
      Status |= z.add(c, RM);
    }
    𐡄𐡍 (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ خطأ);
      أرجع (opStatus)Status;
    }
    Floats[0] = z;
    APFloat zz = aa;
    Status |= zz.add(cc, RM);
    𐡄𐡍 (AComparedToC == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(c, RM);
      Status |= Floats[1].add(zz, RM);
    } وإلا {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(a, RM);
      Status |= Floats[1].add(zz, RM);
    }
  } وإلا {
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
    𐡄𐡍 (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ خطأ);
      أرجع opOK;
    }
    Floats[0] = z;
    Status |= Floats[0].add(zz, RM);
    𐡄𐡍 (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ خطأ);
      أرجع (opStatus)Status;
    }
    Floats[1] = std::move(z);
    Status |= Floats[1].subtract(Floats[0], RM);
    Status |= Floats[1].add(zz, RM);
  }
  أرجع (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::addWithSpecial(ثابت DoubleAPFloat &LHS,
                                                ثابت DoubleAPFloat &RHS,
                                                DoubleAPFloat &Out,
                                                roundingMode RM) {
  𐡄𐡍 (LHS.getCategory() == fcNaN) {
    Out = LHS;
    أرجع opOK;
  }
  𐡄𐡍 (RHS.getCategory() == fcNaN) {
    Out = RHS;
    أرجع opOK;
  }
  𐡄𐡍 (LHS.getCategory() == fcZero) {
    Out = RHS;
    أرجع opOK;
  }
  𐡄𐡍 (RHS.getCategory() == fcZero) {
    Out = LHS;
    أرجع opOK;
  }
  𐡄𐡍 (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    Out.makeNaN(خطأ, Out.isNegative(), nullptr);
    أرجع opInvalidOp;
  }
  𐡄𐡍 (LHS.getCategory() == fcInfinity) {
    Out = LHS;
    أرجع opOK;
  }
  𐡄𐡍 (RHS.getCategory() == fcInfinity) {
    Out = RHS;
    أرجع opOK;
  }
  أكد(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  أكد(&A.getSemantics() == &APFloatBase::semIEEEdouble);
  أكد(&AA.getSemantics() == &APFloatBase::semIEEEdouble);
  أكد(&C.getSemantics() == &APFloatBase::semIEEEdouble);
  أكد(&CC.getSemantics() == &APFloatBase::semIEEEdouble);
  أكد(&Out.Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  أكد(&Out.Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
  أرجع Out.addImpl(A, AA, C, CC, RM);
}

APFloat::opStatus DoubleAPFloat::add(ثابت DoubleAPFloat &RHS,
                                     roundingMode RM) {
  أرجع addWithSpecial(*هذا, RHS, *هذا, RM);
}

APFloat::opStatus DoubleAPFloat::subtract(ثابت DoubleAPFloat &RHS,
                                          roundingMode RM) {
  changeSign();
  auto Ret = add(RHS, RM);
  changeSign();
  أرجع Ret;
}

APFloat::opStatus DoubleAPFloat::multiply(ثابت DoubleAPFloat &RHS,
                                          APFloat::roundingMode RM) {
  ثابت auto &LHS = *هذا;
  auto &Out = *هذا;
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
  𐡄𐡍 (LHS.getCategory() == fcNaN) {
    Out = LHS;
    أرجع opOK;
  }
  𐡄𐡍 (RHS.getCategory() == fcNaN) {
    Out = RHS;
    أرجع opOK;
  }
  𐡄𐡍 ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    Out.makeNaN(خطأ, خطأ, nullptr);
    أرجع opOK;
  }
  𐡄𐡍 (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    Out = LHS;
    أرجع opOK;
  }
  𐡄𐡍 (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    Out = RHS;
    أرجع opOK;
  }
  أكد(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal &&
         "Special cases not handled exhaustively");

  صحيح Status = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  Status |= T.multiply(C, RM);
  𐡄𐡍 (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ خطأ);
    أرجع (opStatus)Status;
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
  𐡄𐡍 (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ خطأ);
  } وإلا {
    // Floats[1] = (t - u) + tau
    Status |= T.subtract(U, RM);
    Status |= T.add(Tau, RM);
    Floats[1] = std::move(T);
  }
  أرجع (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::divide(ثابت DoubleAPFloat &RHS,
                                        APFloat::roundingMode RM) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.divide(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()), RM);
  *هذا = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  أرجع Ret;
}

APFloat::opStatus DoubleAPFloat::remainder(ثابت DoubleAPFloat &RHS) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.remainder(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *هذا = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  أرجع Ret;
}

APFloat::opStatus DoubleAPFloat::mod(ثابت DoubleAPFloat &RHS) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.mod(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *هذا = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  أرجع Ret;
}

APFloat::opStatus
DoubleAPFloat::fusedMultiplyAdd(ثابت DoubleAPFloat &Multiplicand,
                                ثابت DoubleAPFloat &Addend,
                                APFloat::roundingMode RM) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.fusedMultiplyAdd(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcastToAPInt()),
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, Addend.bitcastToAPInt()),
      RM);
  *هذا = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  أرجع Ret;
}

APFloat::opStatus DoubleAPFloat::roundToIntegral(APFloat::roundingMode RM) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ثابت APFloat &Hi = getFirst();
  ثابت APFloat &Lo = getSecond();

  APFloat RoundedHi = Hi;
  ثابت opStatus HiStatus = RoundedHi.roundToIntegral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  𐡄𐡍 (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/خطأ);
    أرجع HiStatus;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  auto RoundToNearestHelper = [](APFloat ToRound, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    ثابت APFloat RoundingError = Rounded - ToRound;
    𐡄𐡍 (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        abs(RoundingError).isExactlyValue(0.5))
      Rounded.add(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rmNearestTiesToEven);
    أرجع Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  𐡄𐡍 (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    𐡄𐡍 (RM == rmNearestTiesToAway || RM == rmNearestTiesToEven)
      RoundedHi = RoundToNearestHelper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/خطأ);
    أرجع HiStatus;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  RoundingMode LoRoundingMode;
  𐡄𐡍 (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    LoRoundingMode = isNegative() ? rmTowardPositive : rmTowardNegative;
  وإلا
    LoRoundingMode = RM;

  APFloat RoundedLo = Lo;
  ثابت opStatus LoStatus = RoundedLo.roundToIntegral(LoRoundingMode);
  𐡄𐡍 (LoRoundingMode == rmNearestTiesToAway)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = RoundToNearestHelper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fastTwoSum(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  أرجع LoStatus;
}

فراغ DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmpResult
DoubleAPFloat::compareAbsoluteValue(ثابت DoubleAPFloat &RHS) ثابت {
  // Compare absolute values of the high parts.
  ثابت cmpResult HiPartCmp = Floats[0].compareAbsoluteValue(RHS.Floats[0]);
  𐡄𐡍 (HiPartCmp != cmpEqual)
    أرجع HiPartCmp;

  // Zero, regardless of sign, is equal.
  𐡄𐡍 (Floats[1].isZero() && RHS.Floats[1].isZero())
    أرجع cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  ثابت منطقي ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  ثابت منطقي RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  𐡄𐡍 (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    أرجع RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  𐡄𐡍 (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    أرجع ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  𐡄𐡍 (ThisIsSubtractive != RHSIsSubtractive)
    أرجع ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  ثابت cmpResult LoPartCmp = Floats[1].compareAbsoluteValue(RHS.Floats[1]);

  𐡄𐡍 (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    𐡄𐡍 (LoPartCmp == cmpLessThan)
      أرجع cmpGreaterThan;
    𐡄𐡍 (LoPartCmp == cmpGreaterThan)
      أرجع cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  أرجع LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() ثابت {
  أرجع Floats[0].getCategory();
}

منطقي DoubleAPFloat::isNegative() ثابت { أرجع Floats[0].isNegative(); }

فراغ DoubleAPFloat::makeInf(منطقي Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ خطأ);
}

فراغ DoubleAPFloat::makeZero(منطقي Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ خطأ);
}

فراغ DoubleAPFloat::makeLargest(منطقي Neg) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  𐡄𐡍 (Neg)
    changeSign();
}

فراغ DoubleAPFloat::makeSmallest(منطقي Neg) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].makeSmallest(Neg);
  Floats[1].makeZero(/* Neg = */ خطأ);
}

فراغ DoubleAPFloat::makeSmallestNormalized(منطقي Neg) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  𐡄𐡍 (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ خطأ);
}

فراغ DoubleAPFloat::makeNaN(منطقي SNaN, منطقي Neg, ثابت APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ خطأ);
}

APFloat::cmpResult DoubleAPFloat::compare(ثابت DoubleAPFloat &RHS) ثابت {
  auto Result = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  𐡄𐡍 (Result == APFloat::cmpEqual)
    أرجع Floats[1].compare(RHS.Floats[1]);
  أرجع Result;
}

منطقي DoubleAPFloat::bitwiseIsEqual(ثابت DoubleAPFloat &RHS) ثابت {
  أرجع Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_code hash_value(ثابت DoubleAPFloat &Arg) {
  𐡄𐡍 (Arg.Floats)
    أرجع hash_combine(hash_value(Arg.Floats[0]), hash_value(Arg.Floats[1]));
  أرجع hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcastToAPInt() ثابت {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t Data[] = {
      Floats[0].bitcastToAPInt().getRawData()[0],
      Floats[1].bitcastToAPInt().getRawData()[0],
  };
  أرجع APInt(128, Data);
}

Expected<APFloat::opStatus> DoubleAPFloat::convertFromString(StringRef S,
                                                             roundingMode RM) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy);
  auto Ret = Tmp.convertFromString(S, RM);
  *هذا = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  أرجع Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::opStatus DoubleAPFloat::next(منطقي nextDown) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  𐡄𐡍 (nextDown) {
    changeSign();
    APFloat::opStatus Result = next(/*nextDown=*/خطأ);
    changeSign();
    أرجع Result;
  }
  بدل (getCategory()) {
  حالة fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    𐡄𐡍 (isNegative())
      makeLargest(𐡒𐡔𐡈);
    أرجع opOK;

  حالة fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    𐡄𐡍 (getFirst().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(خطأ, isNegative(), nullptr);
      أرجع opInvalidOp;
    }
    أرجع opOK;

  حالة fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(خطأ);
    أرجع opOK;

  حالة fcNormal:
    اكسر;
  }

  ثابت APFloat &HiOld = getFirst();
  ثابت APFloat &LoOld = getSecond();

  APFloat NextLo = LoOld;
  NextLo.next(/*nextDown=*/خطأ);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  auto InLattice = [](ثابت APFloat &Hi, ثابت APFloat &Lo) {
    أرجع Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  𐡄𐡍 (InLattice(HiOld, NextLo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(NextLo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat Largest{*Semantics, uninitialized};
    Largest.makeLargest(/*Neg=*/خطأ);
    𐡄𐡍 (compare(Largest) == cmpGreaterThan)
      makeInf(/*Neg=*/خطأ);

    أرجع opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat NextHi = HiOld;
  NextHi.next(/*nextDown=*/خطأ);

  // nextUp(getLargest()) == INFINITY
  𐡄𐡍 (NextHi.isInfinity()) {
    makeInf(/*Neg=*/خطأ);
    أرجع opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  𐡄𐡍 (NextHi.isZero()) {
    makeZero(/*Neg=*/𐡒𐡔𐡈);
    أرجع opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  NextLo = neg(scalbn(harrisonUlp(NextHi), -1, rmTowardZero));
  𐡄𐡍 (!InLattice(NextHi, NextLo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    NextLo.next(/*nextDown=*/خطأ);

  Floats[0] = std::move(NextHi);
  Floats[1] = std::move(NextLo);

  أرجع opOK;
}

APFloat::opStatus DoubleAPFloat::convertToSignExtendedInteger(
    MutableArrayRef<integerPart> Input, unsigned صحيح Width, منطقي IsSigned,
    roundingMode RM, منطقي *IsExact) ثابت {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  𐡄𐡍 (!getFirst().isFiniteNonZero() || getSecond().isZero())
    أرجع getFirst().convertToInteger(Input, Width, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = خطأ;
  DoubleAPFloat Integral = *هذا;
  ثابت opStatus RoundStatus = Integral.roundToIntegral(RM);
  𐡄𐡍 (RoundStatus == opInvalidOp)
    أرجع opInvalidOp;
  ثابت APFloat &IntegralHi = Integral.getFirst();
  ثابت APFloat &IntegralLo = Integral.getSecond();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  منطقي HiIsExact;
  𐡄𐡍 (IntegralHi.isZero() || IntegralLo.isZero()) {
    ثابت opStatus HiStatus =
        IntegralHi.convertToInteger(Input, Width, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    أرجع HiStatus == opOK ? opInexact : HiStatus;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  𐡄𐡍 (!IsSigned && IntegralHi.isNegative())
    أرجع opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  منطقي LoIsExact;
  ثابت صحيح HiExactLog2 = IntegralHi.getExactLog2Abs();
  ثابت unsigned PositiveOverflowWidth = IsSigned ? Width - 1 : Width;
  𐡄𐡍 (HiExactLog2 >= 0 &&
      static_cast<unsigned>(HiExactLog2) == PositiveOverflowWidth) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    𐡄𐡍 (IntegralHi.isNegative() == IntegralLo.isNegative())
      أرجع opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    ثابت opStatus LoStatus = IntegralLo.convertToInteger(
        Input, Width, /*IsSigned=*/𐡒𐡔𐡈, RM, &LoIsExact);
    𐡄𐡍 (LoStatus == opInvalidOp)
      أرجع opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    𐡄𐡍 (IsSigned && !IntegralHi.isNegative())
      APInt::tcClearBit(Input.data(), PositiveOverflowWidth);
    *IsExact = RoundStatus == opOK;
    أرجع RoundStatus;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  ثابت opStatus HiStatus = IntegralHi.convertToInteger(
      Input, Width, IsSigned, rmTowardZero, &HiIsExact);
  𐡄𐡍 (HiStatus == opInvalidOp)
    أرجع HiStatus;

  // Convert Lo into a temporary integer of the same width.
  APSInt LoResult{Width, /*isUnsigned=*/!IsSigned};
  ثابت opStatus LoStatus =
      IntegralLo.convertToInteger(LoResult, rmTowardZero, &LoIsExact);
  𐡄𐡍 (LoStatus == opInvalidOp)
    أرجع LoStatus;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tcAdd(Input.data(), LoResult.getRawData(), /*carry=*/0, Input.size());

  *IsExact = RoundStatus == opOK;
  أرجع RoundStatus;
}

APFloat::opStatus
DoubleAPFloat::convertToInteger(MutableArrayRef<integerPart> Input,
                                unsigned صحيح Width, منطقي IsSigned,
                                roundingMode RM, منطقي *IsExact) ثابت {
  opStatus FS =
      convertToSignExtendedInteger(Input, Width, IsSigned, RM, IsExact);

  𐡄𐡍 (FS == opInvalidOp) {
    ثابت unsigned DstPartsCount = partCountForBits(Width);
    أكد(DstPartsCount <= Input.size() && "Integer too big");

    unsigned Bits;
    𐡄𐡍 (getCategory() == fcNaN)
      Bits = 0;
    وإلا 𐡄𐡍 (isNegative())
      Bits = IsSigned;
    وإلا
      Bits = Width - IsSigned;

    tcSetLeastSignificantBits(Input.data(), DstPartsCount, Bits);
    𐡄𐡍 (isNegative() && IsSigned)
      APInt::tcShiftLeft(Input.data(), DstPartsCount, Width - 1);
  }

  أرجع FS;
}

APFloat::opStatus DoubleAPFloat::handleOverflow(roundingMode RM) {
  بدل (RM) {
  حالة APFloat::rmTowardZero:
    makeLargest(/*Neg=*/isNegative());
    اكسر;
  حالة APFloat::rmTowardNegative:
    𐡄𐡍 (isNegative())
      makeInf(/*Neg=*/𐡒𐡔𐡈);
    وإلا
      makeLargest(/*Neg=*/خطأ);
    اكسر;
  حالة APFloat::rmTowardPositive:
    𐡄𐡍 (isNegative())
      makeLargest(/*Neg=*/𐡒𐡔𐡈);
    وإلا
      makeInf(/*Neg=*/خطأ);
    اكسر;
  حالة APFloat::rmNearestTiesToAway:
  حالة APFloat::rmNearestTiesToEven:
    makeInf(/*Neg=*/isNegative());
    اكسر;
  افتراضي:
    llvm_unreachable("Invalid rounding mode found");
  }
  opStatus S = opInexact;
  𐡄𐡍 (!getFirst().isFinite())
    S = static_cast<opStatus>(S | opOverflow);
  أرجع S;
}

APFloat::opStatus DoubleAPFloat::convertFromUnsignedParts(
    ثابت integerPart *Src, unsigned صحيح SrcCount, roundingMode RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  ثابت unsigned SrcMSB = APInt::tcMSB(Src, SrcCount);
  𐡄𐡍 (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/خطأ);
    أرجع opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  ثابت unsigned SrcBitWidth = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/SrcBitWidth, ArrayRef(Src, SrcCount)},
                /*isUnsigned=*/𐡒𐡔𐡈};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{getFirst().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/خطأ, rmNearestTiesToEven);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  𐡄𐡍 (!Hi.isFinite())
    أرجع handleOverflow(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  منطقي HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/𐡒𐡔𐡈};
  Hi.convertToInteger(HiAsInt, rmNearestTiesToEven, &HiAsIntIsExact);
  ثابت APInt Error = SrcInt.zext(HiAsInt.getBitWidth()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  roundingMode LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  𐡄𐡍 (Error.isNegative()) {
    𐡄𐡍 (RM == rmNearestTiesToAway) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      ثابت unsigned ErrorActiveBits = Error.getSignificantBits() - 1;
      ثابت unsigned LoPrecision = getSecond().getSemantics().precision;
      𐡄𐡍 (ErrorActiveBits > LoPrecision) {
        ثابت unsigned RoundingBoundary = ErrorActiveBits - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        𐡄𐡍 (Error.countTrailingZeros() == RoundingBoundary - 1)
          LoRM = rmTowardPositive;
      }
    } وإلا 𐡄𐡍 (RM == rmTowardZero) {
      // For rmTowardZero, the final positive result must be truncated (rounded
      // down). When Hi is an overestimate, Error is negative. A standard
      // rmTowardZero conversion of Error would make it *less* negative,
      // effectively rounding the final sum Hi + Lo *up*. To ensure the sum
      // rounds down correctly, we force Lo to round toward -infinity.
      LoRM = rmTowardNegative;
    }
  }

  APFloat Lo{getSecond().getSemantics()};
  opStatus Status = Lo.convertFromAPInt(Error, /*IsSigned=*/𐡒𐡔𐡈, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fastTwoSum(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  𐡄𐡍 (!getFirst().isFinite())
    أرجع handleOverflow(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  𐡄𐡍 (getFirst().isFiniteNonZero() && Floats[0].isLargest()) {
    DoubleAPFloat Largest{*Semantics};
    Largest.makeLargest(/*Neg=*/خطأ);
    𐡄𐡍 (compare(Largest) == APFloat::cmpGreaterThan)
      أرجع handleOverflow(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  أرجع Status;
}

APFloat::opStatus DoubleAPFloat::convertFromAPInt(ثابت APInt &Input,
                                                  منطقي IsSigned,
                                                  roundingMode RM) {
  ثابت منطقي NegateInput = IsSigned && Input.isNegative();
  APInt API = Input;
  𐡄𐡍 (NegateInput)
    API.negate();

  ثابت APFloat::opStatus Status =
      convertFromUnsignedParts(API.getRawData(), API.getNumWords(), RM);
  𐡄𐡍 (NegateInput)
    changeSign();
  أرجع Status;
}

unsigned صحيح DoubleAPFloat::convertToHexString(حرف *DST,
                                               unsigned صحيح HexDigits,
                                               منطقي UpperCase,
                                               roundingMode RM) ثابت {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  أرجع APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .convertToHexString(DST, HexDigits, UpperCase, RM);
}

منطقي DoubleAPFloat::isDenormal() ثابت {
  أرجع getCategory() == fcNormal &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

منطقي DoubleAPFloat::isSmallest() ثابت {
  𐡄𐡍 (getCategory() != fcNormal)
    أرجع خطأ;
  DoubleAPFloat Tmp(*هذا);
  Tmp.makeSmallest(هذا->isNegative());
  أرجع Tmp.compare(*هذا) == cmpEqual;
}

منطقي DoubleAPFloat::isSmallestNormalized() ثابت {
  𐡄𐡍 (getCategory() != fcNormal)
    أرجع خطأ;

  DoubleAPFloat Tmp(*هذا);
  Tmp.makeSmallestNormalized(هذا->isNegative());
  أرجع Tmp.compare(*هذا) == cmpEqual;
}

منطقي DoubleAPFloat::isLargest() ثابت {
  𐡄𐡍 (getCategory() != fcNormal)
    أرجع خطأ;
  DoubleAPFloat Tmp(*هذا);
  Tmp.makeLargest(هذا->isNegative());
  أرجع Tmp.compare(*هذا) == cmpEqual;
}

منطقي DoubleAPFloat::isInteger() ثابت {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  أرجع Floats[0].isInteger() && Floats[1].isInteger();
}

فراغ DoubleAPFloat::toString(SmallVectorImpl<حرف> &Str,
                             unsigned FormatPrecision,
                             unsigned FormatMaxPadding,
                             منطقي TruncateZero) ثابت {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .toString(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

صحيح DoubleAPFloat::getExactLog2Abs() ثابت {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  𐡄𐡍 (getSecond().isNonZero())
    أرجع INT_MIN;
  أرجع getFirst().getExactLog2Abs();
}

صحيح ilogb(ثابت DoubleAPFloat &Arg) {
  ثابت APFloat &Hi = Arg.getFirst();
  ثابت APFloat &Lo = Arg.getSecond();
  صحيح IlogbResult = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  𐡄𐡍 (Arg.getCategory() != fcNormal)
    أرجع IlogbResult;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  𐡄𐡍 (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    أرجع IlogbResult;
  𐡄𐡍 (Hi.getExactLog2Abs() == INT_MIN)
    أرجع IlogbResult;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  أرجع IlogbResult - 1;
}

DoubleAPFloat scalbn(ثابت DoubleAPFloat &Arg, صحيح Exp,
                     APFloat::roundingMode RM) {
  أكد(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");
  أرجع DoubleAPFloat(APFloatBase::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(ثابت DoubleAPFloat &Arg, صحيح &Exp,
                    APFloat::roundingMode RM) {
  أكد(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  𐡄𐡍 (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.getFirst() = Quiet.getFirst().makeQuiet();
    أرجع Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  𐡄𐡍 (Exp == APFloat::IEK_Inf)
    أرجع Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  𐡄𐡍 (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    أرجع Arg;
  }

  ثابت APFloat &Hi = Arg.getFirst();
  ثابت APFloat &Lo = Arg.getSecond();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  ثابت منطقي SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat Second = Lo;
  𐡄𐡍 (Arg.getCategory() == APFloat::fcNormal && Lo.isFiniteNonZero()) {
    roundingMode LoRoundingMode;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    𐡄𐡍 (RM == rmTowardZero)
      LoRoundingMode = Arg.isNegative() ? rmTowardPositive : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    وإلا 𐡄𐡍 (RM == rmNearestTiesToAway && SignsDisagree && Exp > 0)
      LoRoundingMode = rmNearestTiesToEven;
    وإلا
      LoRoundingMode = RM;
    Second = scalbn(Lo, -Exp, LoRoundingMode);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    𐡄𐡍 (RM == rmNearestTiesToAway && LoRoundingMode == rmNearestTiesToEven) {
      // Re-scale the result back to check if rounding occurred.
      ثابت APFloat RecomposedLo = scalbn(Second, Exp, rmNearestTiesToEven);
      𐡄𐡍 (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        ثابت APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        ثابت APFloat UlpOfSecond = harrisonUlp(Second);
        ثابت APFloat ScaledUlpOfSecond =
            scalbn(UlpOfSecond, Exp - 1, rmNearestTiesToEven);
        ثابت منطقي IsMidpoint = abs(RoundingError) == ScaledUlpOfSecond;
        ثابت منطقي RoundedLoAway =
            Second.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        𐡄𐡍 (IsMidpoint && RoundedLoAway)
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
    𐡄𐡍 (Second.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat First = scalbn(Hi, -Exp, RM);
  أرجع DoubleAPFloat(APFloatBase::PPCDoubleDouble(), std::move(First),
                       std::move(Second));
}

APInt DoubleAPFloat::getNaNPayload() ثابت { أرجع Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, ثابت fltSemantics &Semantics) {
  𐡄𐡍 (usesLayout<IEEEFloat>(Semantics)) {
    جديد (&IEEE) IEEEFloat(std::move(F));
    أرجع;
  }
  𐡄𐡍 (usesLayout<DoubleAPFloat>(Semantics)) {
    ثابت fltSemantics& S = F.getSemantics();
    جديد (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloatBase::IEEEdouble()));
    أرجع;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::opStatus> APFloat::convertFromString(StringRef Str,
                                                       roundingMode RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_code hash_value(ثابت APFloat &Arg) {
  𐡄𐡍 (APFloat::usesLayout<detail::IEEEFloat>(Arg.getSemantics()))
    أرجع hash_value(Arg.U.IEEE);
  𐡄𐡍 (APFloat::usesLayout<detail::DoubleAPFloat>(Arg.getSemantics()))
    أرجع hash_value(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(ثابت fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  auto StatusOrErr = convertFromString(S, rmNearestTiesToEven);
  أكد(StatusOrErr && "Invalid floating point representation");
  consumeError(StatusOrErr.takeError());
}

FPClassTest APFloat::classify() ثابت {
  𐡄𐡍 (isZero())
    أرجع isNegative() ? fcNegZero : fcPosZero;
  𐡄𐡍 (isNormal())
    أرجع isNegative() ? fcNegNormal : fcPosNormal;
  𐡄𐡍 (isDenormal())
    أرجع isNegative() ? fcNegSubnormal : fcPosSubnormal;
  𐡄𐡍 (isInfinity())
    أرجع isNegative() ? fcNegInf : fcPosInf;
  أكد(isNaN() && "Other class of FP constant");
  أرجع isSignaling() ? fcSNan : fcQNan;
}

منطقي APFloat::getExactInverse(APFloat *Inv) ثابت {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  𐡄𐡍 (!isFiniteNonZero())
    أرجع خطأ;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  𐡄𐡍 (isDenormal())
    أرجع خطأ;

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
  ثابت صحيح Exp = getExactLog2Abs();
  𐡄𐡍 (Exp == INT_MIN)
    أرجع خطأ;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  𐡄𐡍 (Reciprocal.getExactLog2Abs() != -Exp)
    أرجع خطأ;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  𐡄𐡍 (Reciprocal.isDenormal())
    أرجع خطأ;

  أكد(Reciprocal.isFiniteNonZero());

  𐡄𐡍 (Inv)
    *Inv = std::move(Reciprocal);

  أرجع 𐡒𐡔𐡈;
}

APFloat::opStatus APFloat::convert(ثابت fltSemantics &ToSemantics,
                                   roundingMode RM, منطقي *losesInfo) {
  𐡄𐡍 (&getSemantics() == &ToSemantics) {
    *losesInfo = خطأ;
    أرجع opOK;
  }
  𐡄𐡍 (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics))
    أرجع U.IEEE.convert(ToSemantics, RM, losesInfo);
  𐡄𐡍 (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<DoubleAPFloat>(ToSemantics)) {
    أكد(&ToSemantics == &APFloatBase::semPPCDoubleDouble);
    auto Ret =
        U.IEEE.convert(APFloatBase::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *هذا = APFloat(ToSemantics, U.IEEE.bitcastToAPInt());
    أرجع Ret;
  }
  𐡄𐡍 (usesLayout<DoubleAPFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics)) {
    auto Ret = getIEEE().convert(ToSemantics, RM, losesInfo);
    *هذا = APFloat(std::move(getIEEE()), ToSemantics);
    أرجع Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::getAllOnesValue(ثابت fltSemantics &Semantics) {
  أرجع APFloat(Semantics, APInt::getAllOnes(Semantics.sizeInBits));
}

فراغ APFloat::print(raw_ostream &OS) ثابت {
  SmallVector<حرف, 16> Buffer;
  toString(Buffer);
  OS << Buffer;
}

#𐡄𐡍 !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD فراغ APFloat::dump() ثابت {
  print(dbgs());
  dbgs() << '\n';
}
#endif

فراغ APFloat::Profile(FoldingSetNodeID &NID) ثابت {
  NID.Add(bitcastToAPInt());
}

APFloat::opStatus APFloat::convertToInteger(APSInt &result,
                                            roundingMode rounding_mode,
                                            منطقي *isExact) ثابت {
  unsigned bitWidth = result.getBitWidth();
  SmallVector<uint64_t, 4> parts(result.getNumWords());
  opStatus status = convertToInteger(parts, bitWidth, result.isSigned(),
                                     rounding_mode, isExact);
  // Keeps the original signed-ness.
  result = APInt(bitWidth, parts);
  أرجع status;
}

مزدوج APFloat::convertToDouble() ثابت {
  𐡄𐡍 (&getSemantics() == &APFloatBase::semIEEEdouble)
    أرجع getIEEE().convertToDouble();
  أكد(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat Temp = *هذا;
  منطقي LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  أرجع Temp.getIEEE().convertToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 APFloat::convertToQuad() ثابت {
  𐡄𐡍 (&getSemantics() == &APFloatBase::semIEEEquad)
    أرجع getIEEE().convertToQuad();
  أكد(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat Temp = *هذا;
  منطقي LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEquad, rmNearestTiesToEven, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  أرجع Temp.getIEEE().convertToQuad();
}
#endif

عائم APFloat::convertToFloat() ثابت {
  𐡄𐡍 (&getSemantics() == &APFloatBase::semIEEEsingle)
    أرجع getIEEE().convertToFloat();
  أكد(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat Temp = *هذا;
  منطقي LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEsingle, rmNearestTiesToEven, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  أرجع Temp.getIEEE().convertToFloat();
}

unsigned APFloatBase::getArbitraryFPFormatSizeInBits(StringRef Format) {
  أرجع StringSwitch<unsigned>(Format)
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

منطقي APFloatBase::isValidArbitraryFPFormat(StringRef Format) {
  أرجع getArbitraryFPFormatSizeInBits(Format) != 0;
}

ثابت fltSemantics *APFloatBase::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  أرجع StringSwitch<ثابت fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(nullptr);
}

APFloat::Storage::~Storage() {
  𐡄𐡍 (usesLayout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    أرجع;
  }
  𐡄𐡍 (usesLayout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    أرجع;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(ثابت APFloat::Storage &RHS) {
  𐡄𐡍 (usesLayout<IEEEFloat>(*RHS.semantics)) {
    جديد (هذا) IEEEFloat(RHS.IEEE);
    أرجع;
  }
  𐡄𐡍 (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    جديد (هذا) DoubleAPFloat(RHS.Double);
    أرجع;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  𐡄𐡍 (usesLayout<IEEEFloat>(*RHS.semantics)) {
    جديد (هذا) IEEEFloat(std::move(RHS.IEEE));
    أرجع;
  }
  𐡄𐡍 (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    جديد (هذا) DoubleAPFloat(std::move(RHS.Double));
    أرجع;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::operator=(ثابت APFloat::Storage &RHS) {
  𐡄𐡍 (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } وإلا 𐡄𐡍 (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } وإلا 𐡄𐡍 (هذا != &RHS) {
    هذا->~Storage();
    جديد (هذا) Storage(RHS);
  }
  أرجع *هذا;
}

APFloat::Storage &APFloat::Storage::operator=(APFloat::Storage &&RHS) {
  𐡄𐡍 (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } وإلا 𐡄𐡍 (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } وإلا 𐡄𐡍 (هذا != &RHS) {
    هذا->~Storage();
    جديد (هذا) Storage(std::move(RHS));
  }
  أرجع *هذا;
}

نطاق {

APFloat::opStatus getOpStatusFromLibc(صحيح libc_exceptions) {
  APFloat::opStatus status = APFloat::opOK;
  𐡄𐡍 (libc_exceptions & FE_INVALID)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInvalidOp);
  𐡄𐡍 (libc_exceptions & FE_DIVBYZERO)
    status = static_cast<APFloat::opStatus>(status | APFloat::opDivByZero);
  𐡄𐡍 (libc_exceptions & FE_OVERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opOverflow);
  𐡄𐡍 (libc_exceptions & FE_UNDERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opUnderflow);
  𐡄𐡍 (libc_exceptions & FE_INEXACT)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInexact);
  أرجع status;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> exp(ثابت APFloat &x, RoundingMode rounding_mode,
                           APFloat::opStatus *status) {

  𐡄𐡍 (rounding_mode == APFloatBase::rmNearestTiesToEven) {
    𐡄𐡍 (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEsingle) {
      عائم x_val = x.convertToFloat();
      صحيح exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      𐡄𐡍 (status) {
        *status = getOpStatusFromLibc(exc);
        𐡄𐡍 (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      عائم result = LIBC_NAMESPACE::shared::expf(x_val);
      أرجع APFloat(result);
    }
    𐡄𐡍 (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEdouble) {
      مزدوج x_val = x.convertToDouble();
      صحيح exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      𐡄𐡍 (status) {
        *status = getOpStatusFromLibc(exc);
        𐡄𐡍 (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      مزدوج result = LIBC_NAMESPACE::shared::exp(x_val);
      أرجع APFloat(result);
    }
  }
  أرجع std::nullopt;
}

} // namespace llvm

#undef APFLOAT_DISPATCH_ON_SEMANTICS
