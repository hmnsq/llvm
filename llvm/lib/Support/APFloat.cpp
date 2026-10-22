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
  افعل {                                                                         \
    إذا (usesLayout<IEEEFloat>(getSemantics()))                                 \
      إرجاع U.IEEE.METHOD_CALL;                                               \
    إذا (usesLayout<DoubleAPFloat>(getSemantics()))                             \
      إرجاع U.Double.METHOD_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } بينما (خطأ)

using نطاق llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#define Pack_Categories_Into_مفتاح(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

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
    true,
    true,
    true,
    true,
    true};
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
    true,
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
    إرجاع IEEEhalf();
  حالة S_BFloat:
    إرجاع BFloat();
  حالة S_IEEEsingle:
    إرجاع IEEEsingle();
  حالة S_IEEEdouble:
    إرجاع IEEEdouble();
  حالة S_IEEEquad:
    إرجاع IEEEquad();
  حالة S_x87DoubleExtended:
    إرجاع x87DoubleExtended();
  حالة S_PPCDoubleDouble:
    إرجاع PPCDoubleDouble();
  حالة S_PPCDoubleDoubleLegacy:
    إرجاع PPCDoubleDoubleLegacy();
  حالة S_Float8E5M2:
    إرجاع Float8E5M2();
  حالة S_Float8E5M2FNUZ:
    إرجاع Float8E5M2FNUZ();
  حالة S_Float8E4M3:
    إرجاع Float8E4M3();
  حالة S_Float8E4M3FN:
    إرجاع Float8E4M3FN();
  حالة S_Float8E4M3FNUZ:
    إرجاع Float8E4M3FNUZ();
  حالة S_Float8E4M3B11FNUZ:
    إرجاع Float8E4M3B11FNUZ();
  حالة S_Float8E3M4:
    إرجاع Float8E3M4();
  حالة S_FloatTF32:
    إرجاع FloatTF32();
  حالة S_Float8E8M0FNU:
    إرجاع Float8E8M0FNU();
  حالة S_Float8E5M3FNU:
    إرجاع Float8E5M3FNU();
  حالة S_Float6E3M2FN:
    إرجاع Float6E3M2FN();
  حالة S_Float6E2M3FN:
    إرجاع Float6E2M3FN();
  حالة S_Float4E2M1FN:
    إرجاع Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloatBase::Semantics
APFloatBase::SemanticsToEnum(ثابت llvm::fltSemantics &Sem) {
  إذا (&Sem == &llvm::APFloat::IEEEhalf())
    إرجاع S_IEEEhalf;
  وإلا إذا (&Sem == &llvm::APFloat::BFloat())
    إرجاع S_BFloat;
  وإلا إذا (&Sem == &llvm::APFloat::IEEEsingle())
    إرجاع S_IEEEsingle;
  وإلا إذا (&Sem == &llvm::APFloat::IEEEdouble())
    إرجاع S_IEEEdouble;
  وإلا إذا (&Sem == &llvm::APFloat::IEEEquad())
    إرجاع S_IEEEquad;
  وإلا إذا (&Sem == &llvm::APFloat::x87DoubleExtended())
    إرجاع S_x87DoubleExtended;
  وإلا إذا (&Sem == &llvm::APFloat::PPCDoubleDouble())
    إرجاع S_PPCDoubleDouble;
  وإلا إذا (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    إرجاع S_PPCDoubleDoubleLegacy;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E5M2())
    إرجاع S_Float8E5M2;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    إرجاع S_Float8E5M2FNUZ;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E4M3())
    إرجاع S_Float8E4M3;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E4M3FN())
    إرجاع S_Float8E4M3FN;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    إرجاع S_Float8E4M3FNUZ;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    إرجاع S_Float8E4M3B11FNUZ;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E3M4())
    إرجاع S_Float8E3M4;
  وإلا إذا (&Sem == &llvm::APFloat::FloatTF32())
    إرجاع S_FloatTF32;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E8M0FNU())
    إرجاع S_Float8E8M0FNU;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E5M3FNU())
    إرجاع S_Float8E5M3FNU;
  وإلا إذا (&Sem == &llvm::APFloat::Float6E3M2FN())
    إرجاع S_Float6E3M2FN;
  وإلا إذا (&Sem == &llvm::APFloat::Float6E2M3FN())
    إرجاع S_Float6E2M3FN;
  وإلا إذا (&Sem == &llvm::APFloat::Float4E2M1FN())
    إرجاع S_Float4E2M1FN;
  وإلا
    llvm_unreachable("Unknown floating semantics");
}

منطقي APFloatBase::isRepresentableBy(ثابت fltSemantics &A,
                                    ثابت fltSemantics &B) {
  إرجاع A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

منطقي APFloatBase::isLosslesslyConvertibleTo(ثابت fltSemantics &From,
                                            ثابت fltSemantics &InitialTo,
                                            منطقي IgnoreNaNs) {
  إذا (&From == &InitialTo)
    إرجاع true;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  إذا (&From == &semPPCDoubleDouble)
    إرجاع خطأ;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  ثابت fltSemantics &To =
      &InitialTo == &semPPCDoubleDouble ? semIEEEdouble : InitialTo;

  إذا (!isRepresentableBy(From, To))
    إرجاع خطأ;

  إذا ((From.hasZero && !To.hasZero) ||
      (From.hasSignedRepr && !To.hasSignedRepr))
    إرجاع خطأ;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  منطقي FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  منطقي ToHasSignedZero = To.hasZero && To.hasSignedRepr &&
                         To.nanEncoding != fltNanEncoding::NegativeZero;
  إذا (FromHasSignedZero && !ToHasSignedZero)
    إرجاع خطأ;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat SmallestFrom = APFloat::getSmallest(From);
  منطقي LosesInfo = خطأ;
  (فراغ)SmallestFrom.convert(To, APFloat::rmNearestTiesToEven, &LosesInfo);
  إذا (LosesInfo)
    إرجاع خطأ;

  إذا (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    إرجاع true;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  إذا (IgnoreNaNs) {
    إذا (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      إرجاع To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    إرجاع To.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  إذا (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    إرجاع خطأ;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  إذا (To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    إرجاع true;
  إرجاع To.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
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
ساكن constexpr unsigned رقم_Pow5_Parts_On_مكدس = 16;

ساكن unsigned صحيح powerOfFivePartsForExponent(unsigned صحيح power) {
  أكد(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  إرجاع 2 + ((power * 815) / (351 * APFloatBase::integerPartWidth));
}

unsigned صحيح APFloatBase::semanticsPrecision(ثابت fltSemantics &semantics) {
  إرجاع semantics.precision;
}
APFloatBase::ExponentType
APFloatBase::semanticsMaxExponent(ثابت fltSemantics &semantics) {
  إرجاع semantics.maxExponent;
}
APFloatBase::ExponentType
APFloatBase::semanticsMinExponent(ثابت fltSemantics &semantics) {
  إرجاع semantics.minExponent;
}
unsigned صحيح APFloatBase::semanticsSizeInBits(ثابت fltSemantics &semantics) {
  إرجاع semantics.sizeInBits;
}
unsigned صحيح APFloatBase::semanticsIntSizeInBits(ثابت fltSemantics &semantics,
                                                 منطقي isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  unsigned صحيح MinBitWidth = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  إذا (isSigned)
    ++MinBitWidth;
  إرجاع MinBitWidth;
}

منطقي APFloatBase::semanticsHasZero(ثابت fltSemantics &semantics) {
  إرجاع semantics.hasZero;
}

منطقي APFloatBase::semanticsHasSignedRepr(ثابت fltSemantics &semantics) {
  إرجاع semantics.hasSignedRepr;
}

منطقي APFloatBase::semanticsHasInf(ثابت fltSemantics &semantics) {
  إرجاع semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

منطقي APFloatBase::semanticsHasNaN(ثابت fltSemantics &semantics) {
  إرجاع semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

منطقي APFloatBase::isIEEELikeFP(ثابت fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  إرجاع SemanticsToEnum(semantics) <= S_IEEEquad;
}

منطقي APFloatBase::hasSignBitInMSB(ثابت fltSemantics &semantics) {
  إرجاع semantics.hasSignBitInMSB;
}

منطقي APFloatBase::isRepresentableAsNormalIn(ثابت fltSemantics &Src,
                                            ثابت fltSemantics &Dst) {
  // Exponent range must be larger.
  إذا (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    إرجاع خطأ;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  إرجاع Dst.precision >= Src.precision;
}

unsigned APFloatBase::getSizeInBits(ثابت fltSemantics &Sem) {
  إرجاع Sem.sizeInBits;
}

ساكن constexpr APFloatBase::ExponentType
exponentZero(ثابت fltSemantics &semantics) {
  إرجاع semantics.minExponent - 1;
}

ساكن constexpr APFloatBase::ExponentType
exponentInf(ثابت fltSemantics &semantics) {
  إرجاع semantics.maxExponent + 1;
}

ساكن constexpr APFloatBase::ExponentType
exponentNaN(ثابت fltSemantics &semantics) {
  إذا (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    إذا (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      إرجاع exponentZero(semantics);
    إذا (semantics.hasSignedRepr || semantics.precision > 1)
      إرجاع semantics.maxExponent;
  }
  إرجاع semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

ساكن مضمن Error createError(ثابت Twine &Err) {
  إرجاع make_error<StringError>(Err, inconvertibleErrorCode());
}

ساكن constexpr مضمن unsigned صحيح part_تعداد_For_Bits(unsigned صحيح bits) {
  إرجاع std::max(1u, (bits + APFloatBase::integerPartWidth - 1) /
                          APFloatBase::integerPartWidth);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
ساكن مضمن unsigned صحيح
dec_Digit_قيمة(unsigned صحيح c)
{
  إرجاع c - '0';
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
  إذا (p == end || ((*p == '-' || *p == '+') && (p + 1) == end))
    إرجاع 0;

  منطقي isNegative = *p == '-';
  إذا (*p == '-' || *p == '+') {
    p++;
    إذا (p == end)
      إرجاع createError("Exponent has no digits");
  }

  unsigned absExponent = dec_Digit_قيمة(*p++);
  إذا (absExponent >= 10U)
    إرجاع createError("Invalid character in exponent");

  لكل (; p != end; ++p) {
    unsigned قيمة_2 = dec_Digit_قيمة(*p);
    إذا (قيمة_2 >= 10U)
      إرجاع createError("Invalid character in exponent");

    absExponent = absExponent * 10U + قيمة_2;
    إذا (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      اكسر;
    }
  }

  إذا (isNegative)
    إرجاع -(صحيح) absExponent;
  وإلا
    إرجاع (صحيح) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
ساكن Expected<صحيح> totalExponent(StringRef::iterator p,
                                   StringRef::iterator end,
                                   صحيح exponentAdjustment) {
  صحيح exponent = 0;

  إذا (p == end)
    إرجاع createError("Exponent has no digits");

  منطقي negative = *p == '-';
  إذا (*p == '-' || *p == '+') {
    p++;
    إذا (p == end)
      إرجاع createError("Exponent has no digits");
  }

  صحيح unsignedExponent = 0;
  منطقي overflow = خطأ;
  لكل (; p != end; ++p) {
    unsigned صحيح قيمة_2;

    قيمة_2 = dec_Digit_قيمة(*p);
    إذا (قيمة_2 >= 10U)
      إرجاع createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + قيمة_2;
    إذا (unsignedExponent > 32767) {
      overflow = true;
      اكسر;
    }
  }

  إذا (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    overflow = true;

  إذا (!overflow) {
    exponent = unsignedExponent;
    إذا (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    إذا (exponent > 32767 || exponent < -32768)
      overflow = true;
  }

  إذا (overflow)
    exponent = negative ? -32768: 32767;

  إرجاع exponent;
}

ساكن Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator end,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = end;
  بينما (p != end && *p == '0')
    p++;

  إذا (p != end && *p == '.') {
    *dot = p++;

    إذا (end - begin == 1)
      إرجاع createError("Significand has no digits");

    بينما (p != end && *p == '0')
      p++;
  }

  إرجاع p;
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
  إذا (!PtrOrErr)
    إرجاع PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->firstSigDigit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  لكل (; p != end; ++p) {
    إذا (*p == '.') {
      إذا (dot != end)
        إرجاع createError("String contains multiple dots");
      dot = p++;
      إذا (p == end)
        اكسر;
    }
    إذا (dec_Digit_قيمة(*p) >= 10U)
      اكسر;
  }

  إذا (p != end) {
    إذا (*p != 'e' && *p != 'E')
      إرجاع createError("Invalid character in significand");
    إذا (p == begin)
      إرجاع createError("Significand has no digits");
    إذا (dot != end && p - begin == 1)
      إرجاع createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    auto ExpOrErr = readExponent(p + 1, end);
    إذا (!ExpOrErr)
      إرجاع ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    إذا (dot == end)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  إذا (p != D->firstSigDigit) {
    /* Drop insignificant trailing zeroes.  */
    إذا (p != begin) {
      افعل
        افعل
          p--;
        بينما (p != begin && *p == '0');
      بينما (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->firstSigDigit)
                                      - (dot > D->firstSigDigit && dot < p)));
  }

  D->lastSigDigit = p;
  إرجاع Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
ساكن Expected<lostFraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator end,
                            unsigned صحيح digit_قيمة) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  إذا (digit_قيمة > 8)
    إرجاع lfMoreThanHalf;
  وإلا إذا (digit_قيمة < 8 && digit_قيمة > 0)
    إرجاع lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  بينما (p != end && (*p == '0' || *p == '.'))
    p++;

  إذا (p == end)
    إرجاع createError("Invalid trailing hexadecimal fraction!");

  unsigned hexDigit = hex_Digit_قيمة(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  إذا (hexDigit == UINT_MAX)
    إرجاع digit_قيمة == 0 ? lfExactlyZero: lfExactlyHalf;
  وإلا
    إرجاع digit_قيمة == 0 ? lfLessThanHalf: lfMoreThanHalf;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
ساكن lostFraction
lostFractionThroughTruncation(ثابت APFloatBase::integerPart *parts,
                              unsigned صحيح part_تعداد,
                              unsigned صحيح bits)
{
  unsigned lsb = APInt::tcLSB(parts, part_تعداد);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  إذا (bits <= lsb)
    إرجاع lfExactlyZero;
  إذا (bits == lsb + 1)
    إرجاع lfExactlyHalf;
  إذا (bits <= part_تعداد * APFloatBase::integerPartWidth &&
      APInt::tcExtractBit(parts, bits - 1))
    إرجاع lfMoreThanHalf;

  إرجاع lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
ساكن lostFraction
shift_يمين(APFloatBase::integerPart *dst, unsigned صحيح parts, unsigned صحيح bits)
{
  lostFraction lost_fraction = lostFractionThroughTruncation(dst, parts, bits);

  APInt::tc_Shift_يمين(dst, parts, bits);

  إرجاع lost_fraction;
}

/* Combine the effect of two lost fractions.  */
ساكن lostFraction
combineLostFractions(lostFraction moreSignificant,
                     lostFraction lessSignificant)
{
  إذا (lessSignificant != lfExactlyZero) {
    إذا (moreSignificant == lfExactlyZero)
      moreSignificant = lfLessThanHalf;
    وإلا إذا (moreSignificant == lfExactlyHalf)
      moreSignificant = lfMoreThanHalf;
  }

  إرجاع moreSignificant;
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

  إذا (HUerr1 + HUerr2 == 0)
    إرجاع inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  وإلا
    إرجاع inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
ساكن APFloatBase::integerPart
ulpsFromBoundary(ثابت APFloatBase::integerPart *parts, unsigned صحيح bits,
                 منطقي isNearest) {
  أكد(bits != 0);

  bits--;
  unsigned تعداد_2 = bits / APFloatBase::integerPartWidth;
  unsigned partBits = bits % APFloatBase::integerPartWidth + 1;

  APFloatBase::integerPart part =
      parts[تعداد_2] & (~(APFloatBase::integerPart)0 >>
                      (APFloatBase::integerPartWidth - partBits));

  APFloatBase::integerPart boundary;
  إذا (isNearest)
    boundary = (APFloatBase::integerPart) 1 << (partBits - 1);
  وإلا
    boundary = 0;

  إذا (تعداد_2 == 0) {
    إذا (part - boundary <= boundary - part)
      إرجاع part - boundary;
    وإلا
      إرجاع boundary - part;
  }

  إذا (part == boundary) {
    بينما (--تعداد_2)
      إذا (parts[تعداد_2])
        إرجاع ~(APFloatBase::integerPart) 0; /* A lot.  */

    إرجاع parts[0];
  } وإلا إذا (part == boundary - 1) {
    بينما (--تعداد_2)
      إذا (~parts[تعداد_2])
        إرجاع ~(APFloatBase::integerPart) 0; /* A lot.  */

    إرجاع -parts[0];
  }

  إرجاع ~(APFloatBase::integerPart) 0; /* A lot.  */
}

ساكن constexpr unsigned PowerOfFivePartCounts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

ساكن constexpr APFloatBase::integerPart PowerOfFiveParts[] = {
#ضمن "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  unsigned Sum = 0;
  لكل (unsigned تعداد_3 : PowerOfFivePartCounts)
    Sum += تعداد_3;
  إرجاع Sum;
}() == std::size(PowerOfFiveParts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
ساكن unsigned صحيح
powerOf5(APFloatBase::integerPart *dst, unsigned صحيح power) {
  ساكن ثابت APFloatBase::integerPart firstEightPowers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  SmallVector<APFloatBase::integerPart, رقم_Pow5_Parts_On_مكدس> scratch;
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
    unsigned parts_تعداد = PowerOfFivePartCounts[n];

    إذا (power & 1) {
      APFloatBase::integerPart *tmp;

      APInt::tcFullMultiply(p2, p1, pow5, result, parts_تعداد);
      result += parts_تعداد;
      إذا (p2[result - 1] == 0)
        result--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_تعداد;
  }

  إذا (p1 != dst)
    APInt::tcAssign(dst, p1, result);

  إرجاع result;
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
partAsHex (حرف *dst, APFloatBase::integerPart part, unsigned صحيح تعداد_2,
           ثابت حرف *hexDigitChars)
{
  unsigned صحيح result = تعداد_2;

  أكد(تعداد_2 != 0 && تعداد_2 <= APFloatBase::integerPartWidth / 4);

  part >>= (APFloatBase::integerPartWidth - 4 * تعداد_2);
  بينما (تعداد_2--) {
    dst[تعداد_2] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  إرجاع result;
}

/* Write out an unsigned decimal integer.  */
ساكن حرف *writeUnsignedDecimal(حرف *dst, unsigned صحيح n) {
  حرف buff[40], *p;

  p = buff;
  افعل
    *p++ = '0' + n % 10;
  بينما (n /= 10);

  افعل
    *dst++ = *--p;
  بينما (p != buff);

  إرجاع dst;
}

/* Write out a signed decimal integer.  */
ساكن حرف *writeSignedDecimal(حرف *dst, صحيح قيمة_2) {
  إذا (قيمة_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(unsigned) قيمة_2);
  } وإلا {
    dst = writeUnsignedDecimal(dst, قيمة_2);
  }

  إرجاع dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
ساكن APFloat harrisonUlp(ثابت APFloat &X) {
  ثابت fltSemantics &Sem = X.getSemantics();
  بدل (X.getCategory()) {
  حالة APFloat::fcNaN:
    إرجاع APFloat::getQNaN(Sem);
  حالة APFloat::fcInfinity:
    إرجاع APFloat::getInf(Sem);
  حالة APFloat::fcZero:
    إرجاع APFloat::getSmallest(Sem);
  حالة APFloat::fcNormal:
    اكسر;
  }
  إذا (X.isDenormal() || X.isSmallestNormalized())
    إرجاع APFloat::getSmallest(Sem);
  صحيح Exp = ilogb(X);
  إذا (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  إرجاع scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rmNearestTiesToEven);
}

نطاق detail {
/* Constructors.  */
فراغ IEEEFloat::initialize(ثابت fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  unsigned تعداد_2 = part_تعداد();
  إذا (تعداد_2 > 1)
    significand.parts = جديد integerPart[تعداد_2];
}

فراغ IEEEFloat::freeSignificand() {
  إذا (needsCleanup())
    delete [] significand.parts;
}

فراغ IEEEFloat::assign(ثابت IEEEFloat &rhs) {
  أكد(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  إذا (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

فراغ IEEEFloat::copySignificand(ثابت IEEEFloat &rhs) {
  أكد(isFiniteNonZero() || category == fcNaN);
  أكد(rhs.part_تعداد() >= part_تعداد());

  APInt::tcAssign(significandParts(), rhs.significandParts(),
                  part_تعداد());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
فراغ IEEEFloat::makeNaN(منطقي SNaN, منطقي Negative, ثابت APInt *fill) {
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  إذا (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  unsigned رقم_Parts = part_تعداد();

  APInt fill_storage;
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = خطأ;
    إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = true;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } وإلا {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  إذا (!fill || fill->get_رقم_كلمات() < رقم_Parts)
    APInt::tcSet(significand, 0, رقم_Parts);
  إذا (fill) {
    APInt::tcAssign(significand, fill->getRawData(),
                    std::min(fill->get_رقم_كلمات(), رقم_Parts));

    // Zero out the excess bits of the significand.
    unsigned bitsToPreserve = semantics->precision - 1;
    unsigned part = bitsToPreserve / 64;
    bitsToPreserve %= 64;
    significand[part] &= ((1ULL << bitsToPreserve) - 1);
    لكل (part++; part != رقم_Parts; ++part)
      significand[part] = 0;
  }

  unsigned QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  إذا (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tcClearBit(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    إذا (APInt::tcIsZero(significand, رقم_Parts))
      APInt::tcSetBit(significand, QNaNBit - 1);
  } وإلا إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } وإلا {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tcSetBit(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  إذا (semantics == &APFloatBase::semX87DoubleExtended)
    APInt::tcSetBit(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::operator=(ثابت IEEEFloat &rhs) {
  إذا (هذا != &rhs) {
    إذا (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    assign(rhs);
  }

  إرجاع *هذا;
}

IEEEFloat &IEEEFloat::operator=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloatBase::semBogus;
  إرجاع *هذا;
}

منطقي IEEEFloat::isDenormal() ثابت {
  إرجاع getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tcExtractBit(significandParts(), semantics->precision - 1) ==
          0);
}

منطقي IEEEFloat::isSmallest() ثابت {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  إرجاع isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

منطقي IEEEFloat::isSmallestNormalized() ثابت {
  إرجاع getCategory() == fcNormal && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

unsigned صحيح IEEEFloat::get_رقم_أعلى_Bits() ثابت {
  ثابت unsigned صحيح Part_تعداد = part_تعداد_For_Bits(semantics->precision);
  ثابت unsigned صحيح Bits = Part_تعداد * integerPartWidth;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  ثابت unsigned صحيح رقم_أعلى_Bits = (semantics->precision > 1)
                                       ? (Bits - semantics->precision + 1)
                                       : (Bits - semantics->precision);
  إرجاع رقم_أعلى_Bits;
}

منطقي IEEEFloat::isSignificandAllOnes() ثابت {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  ثابت integerPart *Parts = significandParts();
  ثابت unsigned Part_تعداد = part_تعداد_For_Bits(semantics->precision);
  لكل (unsigned i = 0; i < Part_تعداد - 1; i++)
    إذا (~Parts[i])
      إرجاع خطأ;

  // Set the unused high bits to all ones when we compare.
  ثابت unsigned رقم_أعلى_Bits = get_رقم_أعلى_Bits();
  أكد(رقم_أعلى_Bits <= integerPartWidth && رقم_أعلى_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ثابت integerPart أعلى_Bit_Fill =
    ~integerPart(0) << (integerPartWidth - رقم_أعلى_Bits);
  إذا ((semantics->precision <= 1) || (~(Parts[Part_تعداد - 1] | أعلى_Bit_Fill)))
    إرجاع خطأ;

  إرجاع true;
}

منطقي IEEEFloat::isSignificandAllOnesExceptLSB() ثابت {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  ثابت integerPart *Parts = significandParts();

  إذا (Parts[0] & 1)
    إرجاع خطأ;

  ثابت unsigned Part_تعداد = part_تعداد_For_Bits(semantics->precision);
  لكل (unsigned i = 0; i < Part_تعداد - 1; i++) {
    إذا (~Parts[i] & ~unsigned{!i})
      إرجاع خطأ;
  }

  // Set the unused high bits to all ones when we compare.
  ثابت unsigned رقم_أعلى_Bits = get_رقم_أعلى_Bits();
  أكد(رقم_أعلى_Bits <= integerPartWidth && رقم_أعلى_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ثابت integerPart أعلى_Bit_Fill = ~integerPart(0)
                                  << (integerPartWidth - رقم_أعلى_Bits);
  إذا (~(Parts[Part_تعداد - 1] | أعلى_Bit_Fill | 0x1))
    إرجاع خطأ;

  إرجاع true;
}

منطقي IEEEFloat::isSignificandAllZeros() ثابت {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  ثابت integerPart *Parts = significandParts();
  ثابت unsigned Part_تعداد = part_تعداد_For_Bits(semantics->precision);

  لكل (unsigned i = 0; i < Part_تعداد - 1; i++)
    إذا (Parts[i])
      إرجاع خطأ;

  // Compute how many bits are used in the final word.
  ثابت unsigned رقم_أعلى_Bits = get_رقم_أعلى_Bits();
  أكد(رقم_أعلى_Bits < integerPartWidth && "Can not have more high bits to "
         "clear than integerPartWidth");
  ثابت integerPart أعلى_Bit_Mask = ~integerPart(0) >> رقم_أعلى_Bits;

  إذا ((semantics->precision > 1) && (Parts[Part_تعداد - 1] & أعلى_Bit_Mask))
    إرجاع خطأ;

  إرجاع true;
}

منطقي IEEEFloat::isSignificandAllZerosExceptMSB() ثابت {
  ثابت integerPart *Parts = significandParts();
  ثابت unsigned Part_تعداد = part_تعداد_For_Bits(semantics->precision);

  لكل (unsigned i = 0; i < Part_تعداد - 1; i++) {
    إذا (Parts[i])
      إرجاع خطأ;
  }

  ثابت unsigned رقم_أعلى_Bits = get_رقم_أعلى_Bits();
  ثابت integerPart MSBMask = integerPart(1)
                              << (integerPartWidth - رقم_أعلى_Bits);
  إرجاع ((semantics->precision <= 1) || (Parts[Part_تعداد - 1] == MSBMask));
}

منطقي IEEEFloat::isLargest() ثابت {
  منطقي IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    إرجاع (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } وإلا {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    إرجاع IsMaxExp && isSignificandAllOnes();
  }
}

منطقي IEEEFloat::isInteger() ثابت {
  // This could be made more efficient; I'm going for obviously correct.
  إذا (!isFinite()) إرجاع خطأ;
  IEEEFloat truncated = *هذا;
  truncated.roundToIntegral(rmTowardZero);
  إرجاع compare(truncated) == cmpEqual;
}

منطقي IEEEFloat::bitwiseIsEqual(ثابت IEEEFloat &rhs) ثابت {
  إذا (هذا == &rhs)
    إرجاع true;
  إذا (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    إرجاع خطأ;
  إذا (category==fcZero || category==fcInfinity)
    إرجاع true;

  إذا (isFiniteNonZero() && exponent != rhs.exponent)
    إرجاع خطأ;

  إرجاع std::equal(significandParts(), significandParts() + part_تعداد(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics, integerPart قيمة_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fcNormal;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = قيمة_2;
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

unsigned صحيح IEEEFloat::part_تعداد() ثابت {
  إرجاع part_تعداد_For_Bits(semantics->precision + 1);
}

ثابت APFloat::integerPart *IEEEFloat::significandParts() ثابت {
  إرجاع const_cast<IEEEFloat *>(هذا)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  إذا (part_تعداد() > 1)
    إرجاع significand.parts;
  وإلا
    إرجاع &significand.part;
}

فراغ IEEEFloat::zeroSignificand() {
  APInt::tcSet(significandParts(), 0, part_تعداد());
}

/* Increment an fcNormal floating point number's significand.  */
فراغ IEEEFloat::incrementSignificand() {
  [[maybe_unused]] integerPart carry =
      APInt::tcIncrement(significandParts(), part_تعداد());

  /* Our callers should never cause us to overflow.  */
  أكد(carry == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::addSignificand(ثابت IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  أكد(semantics == rhs.semantics);
  أكد(exponent == rhs.exponent);

  إرجاع APInt::tcAdd(parts, rhs.significandParts(), 0, part_تعداد());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::subtractSignificand(ثابت IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  أكد(semantics == rhs.semantics);
  أكد(exponent == rhs.exponent);

  إرجاع APInt::tcSubtract(parts, rhs.significandParts(), borrow,
                           part_تعداد());
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
  unsigned new_Parts_تعداد = part_تعداد_For_Bits(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *fullSignificand =
      new_Parts_تعداد > 4 ? جديد integerPart[new_Parts_تعداد] : scratch;

  integerPart *lhsSignificand = significandParts();
  unsigned parts_تعداد = part_تعداد();

  APInt::tcFullMultiply(fullSignificand, lhsSignificand,
                        rhs.significandParts(), parts_تعداد, parts_تعداد);

  lostFraction lost_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  unsigned omsb = APInt::tcMSB(fullSignificand, new_Parts_تعداد) + 1;
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

  إذا (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    ثابت fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    unsigned extendedPrecision = 2 * precision + 1;
    إذا (omsb != extendedPrecision - 1) {
      أكد(extendedPrecision > omsb);
      APInt::tc_Shift_يسار(fullSignificand, new_Parts_تعداد,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    إذا (new_Parts_تعداد == 1)
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
    lost_fraction = extendedAddend.shift_Significand_يمين(1);
    أكد(lost_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    lost_fraction = addOrSubtractSignificand(extendedAddend, خطأ);

    /* Restore our state.  */
    إذا (new_Parts_تعداد == 1)
      fullSignificand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(fullSignificand, new_Parts_تعداد) + 1;
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
  إذا (omsb > precision) {
    unsigned صحيح bits, significantParts;
    lostFraction lf;

    bits = omsb - precision;
    significantParts = part_تعداد_For_Bits(omsb);
    lf = shift_يمين(fullSignificand, significantParts, bits);
    lost_fraction = combineLostFractions(lf, lost_fraction);
    exponent += bits;
  }

  APInt::tcAssign(lhsSignificand, fullSignificand, parts_تعداد);

  إذا (new_Parts_تعداد > 4)
    delete [] fullSignificand;

  إرجاع lost_fraction;
}

lostFraction IEEEFloat::multiplySignificand(ثابت IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  إرجاع multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
lostFraction IEEEFloat::divideSignificand(ثابت IEEEFloat &rhs) {
  integerPart scratch[4];

  أكد(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  ثابت integerPart *rhsSignificand = rhs.significandParts();
  unsigned parts_تعداد = part_تعداد();

  integerPart *dividend =
      parts_تعداد > 2 ? جديد integerPart[parts_تعداد * 2] : scratch;
  integerPart *قاسم = dividend + parts_تعداد;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  لكل (unsigned i = 0; i < parts_تعداد; i++) {
    dividend[i] = lhsSignificand[i];
    قاسم[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  unsigned صحيح precision = semantics->precision;

  /* Normalize the divisor.  */
  unsigned bit = precision - APInt::tcMSB(قاسم, parts_تعداد) - 1;
  إذا (bit) {
    exponent += bit;
    APInt::tc_Shift_يسار(قاسم, parts_تعداد, bit);
  }

  /* Normalize the dividend.  */
  bit = precision - APInt::tcMSB(dividend, parts_تعداد) - 1;
  إذا (bit) {
    exponent -= bit;
    APInt::tc_Shift_يسار(dividend, parts_تعداد, bit);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  إذا (APInt::tcCompare(dividend, قاسم, parts_تعداد) < 0) {
    exponent--;
    APInt::tc_Shift_يسار(dividend, parts_تعداد, 1);
    أكد(APInt::tcCompare(dividend, قاسم, parts_تعداد) >= 0);
  }

  /* Long division.  */
  لكل (bit = precision; bit; bit -= 1) {
    إذا (APInt::tcCompare(dividend, قاسم, parts_تعداد) >= 0) {
      APInt::tcSubtract(dividend, قاسم, 0, parts_تعداد);
      APInt::tcSetBit(lhsSignificand, bit - 1);
    }

    APInt::tc_Shift_يسار(dividend, parts_تعداد, 1);
  }

  /* Figure out the lost fraction.  */
  صحيح cmp = APInt::tcCompare(dividend, قاسم, parts_تعداد);

  lostFraction lost_fraction;
  إذا (cmp > 0)
    lost_fraction = lfMoreThanHalf;
  وإلا إذا (cmp == 0)
    lost_fraction = lfExactlyHalf;
  وإلا إذا (APInt::tcIsZero(dividend, parts_تعداد))
    lost_fraction = lfExactlyZero;
  وإلا
    lost_fraction = lfLessThanHalf;

  إذا (parts_تعداد > 2)
    delete [] dividend;

  إرجاع lost_fraction;
}

unsigned صحيح IEEEFloat::significandMSB() ثابت {
  إرجاع APInt::tcMSB(significandParts(), part_تعداد());
}

unsigned صحيح IEEEFloat::significandLSB() ثابت {
  إرجاع APInt::tcLSB(significandParts(), part_تعداد());
}

/* Note that a zero result is NOT normalized to fcZero.  */
lostFraction IEEEFloat::shift_Significand_يمين(unsigned صحيح bits) {
  /* Our exponent should not overflow.  */
  أكد((ExponentType) (exponent + bits) >= exponent);

  exponent += bits;

  إرجاع shift_يمين(significandParts(), part_تعداد(), bits);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
فراغ IEEEFloat::shift_Significand_يسار(unsigned صحيح bits) {
  أكد(bits < semantics->precision ||
         (semantics->precision == 1 && bits <= 1));

  إذا (bits) {
    unsigned صحيح parts_تعداد = part_تعداد();

    APInt::tc_Shift_يسار(significandParts(), parts_تعداد, bits);
    exponent -= bits;

    أكد(!APInt::tcIsZero(significandParts(), parts_تعداد));
  }
}

APFloat::cmpResult IEEEFloat::compare_Absolute_قيمة(ثابت IEEEFloat &rhs) ثابت {
  أكد(semantics == rhs.semantics);
  أكد(isFiniteNonZero());
  أكد(rhs.isFiniteNonZero());

  صحيح compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  إذا (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_تعداد());

  إذا (compare > 0)
    إرجاع cmpGreaterThan;
  وإلا إذا (compare < 0)
    إرجاع cmpLessThan;
  وإلا
    إرجاع cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
ساكن فراغ tcSetLeastSignificantBits(APInt::WordType *dst, unsigned parts,
                                      unsigned bits) {
  unsigned i = 0;
  بينما (bits > APInt::APINT_BITS_PER_WORD) {
    dst[i++] = ~(APInt::WordType)0;
    bits -= APInt::APINT_BITS_PER_WORD;
  }

  إذا (bits)
    dst[i++] = ~(APInt::WordType)0 >> (APInt::APINT_BITS_PER_WORD - bits);

  بينما (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::opStatus IEEEFloat::handleOverflow(roundingMode rounding_mode) {
  إذا (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    إذا (rounding_mode == rmNearestTiesToEven ||
        rounding_mode == rmNearestTiesToAway ||
        (rounding_mode == rmTowardPositive && !sign) ||
        (rounding_mode == rmTowardNegative && sign)) {
      إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(خطأ, sign);
      وإلا
        category = fcInfinity;
      إرجاع static_cast<opStatus>(opOverflow | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fcNormal;
  exponent = semantics->maxExponent;
  tcSetLeastSignificantBits(significandParts(), part_تعداد(),
                            semantics->precision);
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tcClearBit(significandParts(), 0);

  إرجاع opInexact;
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
    إرجاع lost_fraction == lfExactlyHalf || lost_fraction == lfMoreThanHalf;

  حالة rmNearestTiesToEven:
    إذا (lost_fraction == lfMoreThanHalf)
      إرجاع true;

    /* Our zeroes don't have a significand to test.  */
    إذا (lost_fraction == lfExactlyHalf && category != fcZero)
      إرجاع APInt::tcExtractBit(significandParts(), bit);

    إرجاع خطأ;

  حالة rmTowardZero:
    إرجاع خطأ;

  حالة rmTowardPositive:
    إرجاع !sign;

  حالة rmTowardNegative:
    إرجاع sign;

  افتراضي:
    اكسر;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::opStatus IEEEFloat::normalize(roundingMode rounding_mode,
                                       lostFraction lost_fraction) {
  إذا (!isFiniteNonZero())
    إرجاع opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  unsigned omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  إذا (omsb || lost_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    صحيح exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    إذا (exponent + exponentChange > semantics->maxExponent)
      إرجاع handleOverflow(rounding_mode);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    إذا (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    إذا (exponentChange < 0) {
      أكد(lost_fraction == lfExactlyZero);

      shift_Significand_يسار(-exponentChange);

      إرجاع opOK;
    }

    إذا (exponentChange > 0) {
      lostFraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = shift_Significand_يمين(exponentChange);

      lost_fraction = combineLostFractions(lf, lost_fraction);

      /* Keep OMSB up-to-date.  */
      إذا (omsb > (unsigned) exponentChange)
        omsb -= exponentChange;
      وإلا
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    إرجاع handleOverflow(rounding_mode);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  إذا (lost_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    إذا (omsb == 0) {
      category = fcZero;
      إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = خطأ;
      إذا (!semantics->hasZero)
        makeSmallestNormalized(خطأ);
    }

    إرجاع opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  إذا (roundAwayFromZero(rounding_mode, lost_fraction, 0)) {
    إذا (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    إذا (omsb == (unsigned) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      إذا (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        إرجاع handleOverflow(sign ? rmTowardNegative : rmTowardPositive);

      shift_Significand_يمين(1);

      إرجاع opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      إرجاع handleOverflow(rounding_mode);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  إذا (omsb == semantics->precision)
    إرجاع opInexact;

  /* We have a non-zero denormal.  */
  أكد(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  إذا (omsb == 0) {
    category = fcZero;
    إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = خطأ;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    إذا (!semantics->hasZero)
      makeSmallestNormalized(خطأ);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  إرجاع (opStatus) (opUnderflow | opInexact);
}

APFloat::opStatus IEEEFloat::addOrSubtractSpecials(ثابت IEEEFloat &rhs,
                                                   منطقي subtract) {
  بدل (Pack_Categories_Into_مفتاح(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(nullptr);

  حالة Pack_Categories_Into_مفتاح(fcZero, fcNaN):
  حالة Pack_Categories_Into_مفتاح(fcNormal, fcNaN):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcZero):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcNormal):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcNaN):
    إذا (isSignaling()) {
      makeQuiet();
      إرجاع opInvalidOp;
    }
    إرجاع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_مفتاح(fcNormal, fcZero):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcNormal):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcZero):
    إرجاع opOK;

  حالة Pack_Categories_Into_مفتاح(fcNormal, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ subtract;
    إرجاع opOK;

  حالة Pack_Categories_Into_مفتاح(fcZero, fcNormal):
    assign(rhs);
    sign = rhs.sign ^ subtract;
    إرجاع opOK;

  حالة Pack_Categories_Into_مفتاح(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    إرجاع opOK;

  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    إذا (((sign ^ rhs.sign)!=0) != subtract) {
      makeNaN();
      إرجاع opInvalidOp;
    }

    إرجاع opOK;

  حالة Pack_Categories_Into_مفتاح(fcNormal, fcNormal):
    إرجاع opDivByZero;
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
  إذا (subtract) {
    إذا ((bits < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat temp_rhs(rhs);
    منطقي lost_fraction_is_from_rhs = خطأ;

    إذا (bits == 0)
      lost_fraction = lfExactlyZero;
    وإلا إذا (bits > 0) {
      lost_fraction = temp_rhs.shift_Significand_يمين(bits - 1);
      lost_fraction_is_from_rhs = true;
      shift_Significand_يسار(1);
    } وإلا {
      lost_fraction = shift_Significand_يمين(-bits - 1);
      temp_rhs.shift_Significand_يسار(1);
    }

    // Should we reverse the subtraction.
    cmpResult cmp_result = compare_Absolute_قيمة(temp_rhs);
    إذا (cmp_result == cmpLessThan) {
      منطقي borrow =
          lost_fraction != lfExactlyZero && !lost_fraction_is_from_rhs;
      إذا (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        إذا (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        وإلا إذا (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = temp_rhs.subtractSignificand(*هذا, borrow);
      copySignificand(temp_rhs);
      sign = !sign;
    } وإلا إذا (cmp_result == cmpGreaterThan) {
      منطقي borrow = lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs;
      إذا (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        إذا (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        وإلا إذا (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = subtractSignificand(temp_rhs, borrow);
    } وإلا { // cmpEqual
      zeroSignificand();
      إذا (lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    أكد(!carry);
  } وإلا {
    إذا (bits > 0) {
      IEEEFloat temp_rhs(rhs);

      lost_fraction = temp_rhs.shift_Significand_يمين(bits);
      carry = addSignificand(temp_rhs);
    } وإلا {
      lost_fraction = shift_Significand_يمين(-bits);
      carry = addSignificand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    أكد(!carry);
  }

  إرجاع lost_fraction;
}

APFloat::opStatus IEEEFloat::multiplySpecials(ثابت IEEEFloat &rhs) {
  بدل (Pack_Categories_Into_مفتاح(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(nullptr);

  حالة Pack_Categories_Into_مفتاح(fcZero, fcNaN):
  حالة Pack_Categories_Into_مفتاح(fcNormal, fcNaN):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcNaN):
    assign(rhs);
    sign = خطأ;
    [[fallthrough]];
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcZero):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcNormal):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    إذا (isSignaling()) {
      makeQuiet();
      إرجاع opInvalidOp;
    }
    إرجاع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_مفتاح(fcNormal, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcNormal):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcInfinity):
    category = fcInfinity;
    إرجاع opOK;

  حالة Pack_Categories_Into_مفتاح(fcZero, fcNormal):
  حالة Pack_Categories_Into_مفتاح(fcNormal, fcZero):
  حالة Pack_Categories_Into_مفتاح(fcZero, fcZero):
    category = fcZero;
    إرجاع opOK;

  حالة Pack_Categories_Into_مفتاح(fcZero, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcZero):
    makeNaN();
    إرجاع opInvalidOp;

  حالة Pack_Categories_Into_مفتاح(fcNormal, fcNormal):
    إرجاع opOK;
  }
}

APFloat::opStatus IEEEFloat::divideSpecials(ثابت IEEEFloat &rhs) {
  بدل (Pack_Categories_Into_مفتاح(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(nullptr);

  حالة Pack_Categories_Into_مفتاح(fcZero, fcNaN):
  حالة Pack_Categories_Into_مفتاح(fcNormal, fcNaN):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcNaN):
    assign(rhs);
    sign = خطأ;
    [[fallthrough]];
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcZero):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcNormal):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    إذا (isSignaling()) {
      makeQuiet();
      إرجاع opInvalidOp;
    }
    إرجاع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcZero):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcNormal):
  حالة Pack_Categories_Into_مفتاح(fcZero, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcZero, fcNormal):
    إرجاع opOK;

  حالة Pack_Categories_Into_مفتاح(fcNormal, fcInfinity):
    category = fcZero;
    إرجاع opOK;

  حالة Pack_Categories_Into_مفتاح(fcNormal, fcZero):
    إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(خطأ, sign);
    وإلا
      category = fcInfinity;
    إرجاع opDivByZero;

  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcZero, fcZero):
    makeNaN();
    إرجاع opInvalidOp;

  حالة Pack_Categories_Into_مفتاح(fcNormal, fcNormal):
    إرجاع opOK;
  }
}

APFloat::opStatus IEEEFloat::modSpecials(ثابت IEEEFloat &rhs) {
  بدل (Pack_Categories_Into_مفتاح(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(nullptr);

  حالة Pack_Categories_Into_مفتاح(fcZero, fcNaN):
  حالة Pack_Categories_Into_مفتاح(fcNormal, fcNaN):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcZero):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcNormal):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcNaN):
    إذا (isSignaling()) {
      makeQuiet();
      إرجاع opInvalidOp;
    }
    إرجاع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_مفتاح(fcZero, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcZero, fcNormal):
  حالة Pack_Categories_Into_مفتاح(fcNormal, fcInfinity):
    إرجاع opOK;

  حالة Pack_Categories_Into_مفتاح(fcNormal, fcZero):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcZero):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcNormal):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcZero, fcZero):
    makeNaN();
    إرجاع opInvalidOp;

  حالة Pack_Categories_Into_مفتاح(fcNormal, fcNormal):
    إرجاع opOK;
  }
}

APFloat::opStatus IEEEFloat::remainderSpecials(ثابت IEEEFloat &rhs) {
  بدل (Pack_Categories_Into_مفتاح(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(nullptr);

  حالة Pack_Categories_Into_مفتاح(fcZero, fcNaN):
  حالة Pack_Categories_Into_مفتاح(fcNormal, fcNaN):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcZero):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcNormal):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcNaN):
    إذا (isSignaling()) {
      makeQuiet();
      إرجاع opInvalidOp;
    }
    إرجاع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_مفتاح(fcZero, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcZero, fcNormal):
  حالة Pack_Categories_Into_مفتاح(fcNormal, fcInfinity):
    إرجاع opOK;

  حالة Pack_Categories_Into_مفتاح(fcNormal, fcZero):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcZero):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcNormal):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcZero, fcZero):
    makeNaN();
    إرجاع opInvalidOp;

  حالة Pack_Categories_Into_مفتاح(fcNormal, fcNormal):
    إرجاع opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
فراغ IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    إرجاع;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::opStatus IEEEFloat::addOrSubtract(ثابت IEEEFloat &rhs,
                                           roundingMode rounding_mode,
                                           منطقي subtract) {
  opStatus fs = addOrSubtractSpecials(rhs, subtract);

  /* This return code means it was not a simple case.  */
  إذا (fs == opDivByZero) {
    lostFraction lost_fraction;

    lost_fraction = addOrSubtractSignificand(rhs, subtract);
    fs = normalize(rounding_mode, lost_fraction);

    /* Can only be zero if we lost no fraction.  */
    أكد(category != fcZero || lost_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  إذا (category == fcZero) {
    إذا (rhs.category != fcZero || (sign == rhs.sign) == subtract)
      sign = (rounding_mode == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = خطأ;
  }

  إرجاع fs;
}

/* Normalized addition.  */
APFloat::opStatus IEEEFloat::add(ثابت IEEEFloat &rhs,
                                 roundingMode rounding_mode) {
  إرجاع addOrSubtract(rhs, rounding_mode, خطأ);
}

/* Normalized subtraction.  */
APFloat::opStatus IEEEFloat::subtract(ثابت IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  إرجاع addOrSubtract(rhs, rounding_mode, true);
}

/* Normalized multiply.  */
APFloat::opStatus IEEEFloat::multiply(ثابت IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = multiplySpecials(rhs);

  إذا (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = خطأ;
  إذا (isFiniteNonZero()) {
    lostFraction lost_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    إذا (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  إرجاع fs;
}

/* Normalized divide.  */
APFloat::opStatus IEEEFloat::divide(ثابت IEEEFloat &rhs,
                                    roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = divideSpecials(rhs);

  إذا (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = خطأ;
  إذا (isFiniteNonZero()) {
    lostFraction lost_fraction = divideSignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    إذا (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  إرجاع fs;
}

/* Normalized remainder.  */
APFloat::opStatus IEEEFloat::remainder(ثابت IEEEFloat &rhs) {
  unsigned صحيح origSign = sign;

  // First handle the special cases.
  opStatus fs = remainderSpecials(rhs);
  إذا (fs != opDivByZero)
    إرجاع fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  إذا (P2.add(rhs, rmNearestTiesToEven) == opOK) {
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

  إذا (VEx.compare(PEx) == cmpGreaterThan) {
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
    إذا (result == cmpGreaterThan || result == cmpEqual) {
      fs = subtract(P, rmNearestTiesToEven);
      أكد(fs == opOK);
    }
  }

  إذا (isZero()) {
    sign = origSign;    // IEEE754 requires this
    إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = خطأ;
  } وإلا {
    sign ^= origSign;
  }
  إرجاع fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::opStatus IEEEFloat::mod(ثابت IEEEFloat &rhs) {
  opStatus fs = modSpecials(rhs);
  unsigned صحيح origSign = sign;

  بينما (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_قيمة(rhs) != cmpLessThan) {
    صحيح Exp = ilogb(*هذا) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rmNearestTiesToEven);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    إذا (V.isNaN() || compare_Absolute_قيمة(V) == cmpLessThan)
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
    إذا (!semantics->hasZero && هذا->isSmallest())
      اكسر;

    أكد(fs==opOK);
  }
  إذا (isZero()) {
    sign = origSign; // fmod requires this
    إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = خطأ;
  }
  إرجاع fs;
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
  إذا (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    lostFraction lost_fraction;

    lost_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_mode, lost_fraction);
    إذا (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    إذا (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_mode == rmTowardNegative);
      إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
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
    إذا (fs == opOK)
      fs = addOrSubtract(addend, rounding_mode, خطأ);
  }

  إرجاع fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::opStatus IEEEFloat::roundToIntegral(roundingMode rounding_mode) {
  إذا (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    إرجاع opOK;

  إذا (isNaN()) {
    إذا (isSignaling()) {
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
      إرجاع opInvalidOp;
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
      إرجاع opOK;
    }
  }

  إذا (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    إرجاع opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  إذا (exponent + 1 >= (صحيح)APFloat::semanticsPrecision(*semantics))
    إرجاع opOK;

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
  منطقي إدخال_Sign = isNegative();

  fs = add(MagicConstant, rounding_mode);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  subtract(MagicConstant, rounding_mode);

  // Restore the input sign.
  إذا (إدخال_Sign != isNegative())
    changeSign();

  إرجاع fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmpResult IEEEFloat::compare(ثابت IEEEFloat &rhs) ثابت {
  أكد(semantics == rhs.semantics);

  بدل (Pack_Categories_Into_مفتاح(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(nullptr);

  حالة Pack_Categories_Into_مفتاح(fcNaN, fcZero):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcNormal):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcNaN, fcNaN):
  حالة Pack_Categories_Into_مفتاح(fcZero, fcNaN):
  حالة Pack_Categories_Into_مفتاح(fcNormal, fcNaN):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcNaN):
    إرجاع cmpUnordered;

  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcNormal):
  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcZero):
  حالة Pack_Categories_Into_مفتاح(fcNormal, fcZero):
    إذا (sign)
      إرجاع cmpLessThan;
    وإلا
      إرجاع cmpGreaterThan;

  حالة Pack_Categories_Into_مفتاح(fcNormal, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcZero, fcInfinity):
  حالة Pack_Categories_Into_مفتاح(fcZero, fcNormal):
    إذا (rhs.sign)
      إرجاع cmpGreaterThan;
    وإلا
      إرجاع cmpLessThan;

  حالة Pack_Categories_Into_مفتاح(fcInfinity, fcInfinity):
    إذا (sign == rhs.sign)
      إرجاع cmpEqual;
    وإلا إذا (sign)
      إرجاع cmpLessThan;
    وإلا
      إرجاع cmpGreaterThan;

  حالة Pack_Categories_Into_مفتاح(fcZero, fcZero):
    إرجاع cmpEqual;

  حالة Pack_Categories_Into_مفتاح(fcNormal, fcNormal):
    اكسر;
  }

  cmpResult result;
  /* Two normal numbers.  Do they have the same sign?  */
  إذا (sign != rhs.sign) {
    إذا (sign)
      result = cmpLessThan;
    وإلا
      result = cmpGreaterThan;
  } وإلا {
    /* Compare absolute values; invert result if negative.  */
    result = compare_Absolute_قيمة(rhs);

    إذا (sign) {
      إذا (result == cmpLessThan)
        result = cmpGreaterThan;
      وإلا إذا (result == cmpGreaterThan)
        result = cmpLessThan;
    }
  }

  إرجاع result;
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
  unsigned new_Part_تعداد = part_تعداد_For_Bits(toSemantics.precision + 1);
  unsigned old_Part_تعداد = part_تعداد();
  صحيح shift = toSemantics.precision - fromSemantics.precision;

  منطقي X86SpecialNan = خطأ;
  إذا (&fromSemantics == &APFloatBase::semX87DoubleExtended &&
      &toSemantics != &APFloatBase::semX87DoubleExtended && category == fcNaN &&
      (!(*significandParts() & 0x8000000000000000ULL) ||
       !(*significandParts() & 0x4000000000000000ULL))) {
    // x86 has some unusual NaNs which cannot be represented in any other
    // format; note them here.
    X86SpecialNan = true;
  }

  // If this is a truncation of a denormal number, and the target semantics
  // has larger exponent range than the source semantics (this can happen
  // when truncating from PowerPC double-double to double format), the
  // right shift could lose result mantissa bits.  Adjust exponent instead
  // of performing excessive shift.
  // Also do a similar trick in case shifting denormal would produce zero
  // significand as this case isn't handled correctly by normalize.
  إذا (shift < 0 && isFiniteNonZero()) {
    صحيح omsb = significandMSB() + 1;
    صحيح exponentChange = omsb - fromSemantics.precision;
    إذا (exponent + exponentChange < toSemantics.minExponent)
      exponentChange = toSemantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, shift);
    إذا (exponentChange < 0) {
      shift -= exponentChange;
      exponent += exponentChange;
    } وإلا إذا (omsb <= -shift) {
      exponentChange = omsb + shift - 1; // leave at least one bit set
      shift -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  إذا (shift < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    lostFraction = shift_يمين(significandParts(), old_Part_تعداد, -shift);

  // Fix the storage so it can hold to new value.
  إذا (new_Part_تعداد > old_Part_تعداد) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = جديد integerPart[new_Part_تعداد];
    APInt::tcSet(newParts, 0, new_Part_تعداد);
    إذا (isFiniteNonZero() || category==fcNaN)
      APInt::tcAssign(newParts, significandParts(), old_Part_تعداد);
    freeSignificand();
    significand.parts = newParts;
  } وإلا إذا (new_Part_تعداد == 1 && old_Part_تعداد != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    إذا (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &toSemantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  إذا (shift > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_Shift_يسار(significandParts(), new_Part_تعداد, shift);

  إذا (isFiniteNonZero()) {
    fs = normalize(rounding_mode, lostFraction);
    *losesInfo = (fs != opOK);
  } وإلا إذا (category == fcNaN) {
    إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(خطأ, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } وإلا {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      إذا (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(خطأ, خطأ);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      إذا (!APFloat::hasSignificand(fromSemantics))
        makeNaN(خطأ, sign);

      *losesInfo = lostFraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      إذا (!X86SpecialNan && semantics == &APFloatBase::semX87DoubleExtended)
        APInt::tcSetBit(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      إذا (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } وإلا {
        fs = opOK;
      }
    }
  } وإلا إذا (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(خطأ, sign);
    *losesInfo = true;
    fs = opInexact;
  } وإلا إذا (category == fcZero &&
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
  إذا ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = true;
    إذا (fs == opOK)
      fs = opInexact;
  }

  إذا (category == fcZero && !semantics->hasZero)
    makeSmallestNormalized(خطأ);
  إرجاع fs;
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
  إذا (category == fcInfinity || category == fcNaN)
    إرجاع opInvalidOp;

  unsigned dst_Parts_تعداد = part_تعداد_For_Bits(width);
  أكد(dst_Parts_تعداد <= parts.size() && "Integer too big");

  إذا (category == fcZero) {
    APInt::tcSet(parts.data(), 0, dst_Parts_تعداد);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    إرجاع opOK;
  }

  ثابت integerPart *src = significandParts();

  unsigned truncatedBits;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  إذا (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tcSet(parts.data(), 0, dst_Parts_تعداد);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncatedBits = semantics->precision -1U - exponent;
  } وإلا {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    unsigned صحيح bits = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    إذا (bits > width)
      إرجاع opInvalidOp;

    إذا (bits < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncatedBits = semantics->precision - bits;
      APInt::tcExtract(parts.data(), dst_Parts_تعداد, src, bits, truncatedBits);
    } وإلا {
      /* We want at least as many bits as are available.  */
      APInt::tcExtract(parts.data(), dst_Parts_تعداد, src, semantics->precision,
                       0);
      APInt::tc_Shift_يسار(parts.data(), dst_Parts_تعداد,
                         bits - semantics->precision);
      truncatedBits = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  lostFraction lost_fraction;
  إذا (truncatedBits) {
    lost_fraction = lostFractionThroughTruncation(src, part_تعداد(),
                                                  truncatedBits);
    إذا (lost_fraction != lfExactlyZero &&
        roundAwayFromZero(rounding_mode, lost_fraction, truncatedBits)) {
      إذا (APInt::tcIncrement(parts.data(), dst_Parts_تعداد))
        إرجاع opInvalidOp;     /* Overflow.  */
    }
  } وإلا {
    lost_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  unsigned صحيح omsb = APInt::tcMSB(parts.data(), dst_Parts_تعداد) + 1;

  إذا (sign) {
    إذا (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      إذا (omsb != 0)
        إرجاع opInvalidOp;
    } وإلا {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      إذا (omsb == width &&
          APInt::tcLSB(parts.data(), dst_Parts_تعداد) + 1 != omsb)
        إرجاع opInvalidOp;

      /* This case can happen because of rounding.  */
      إذا (omsb > width)
        إرجاع opInvalidOp;
    }

    APInt::tcNegate (parts.data(), dst_Parts_تعداد);
  } وإلا {
    إذا (omsb >= width + !isSigned)
      إرجاع opInvalidOp;
  }

  إذا (lost_fraction == lfExactlyZero) {
    *isExact = true;
    إرجاع opOK;
  }
  إرجاع opInexact;
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

  إذا (fs == opInvalidOp) {
    unsigned صحيح bits, dst_Parts_تعداد;

    dst_Parts_تعداد = part_تعداد_For_Bits(width);
    أكد(dst_Parts_تعداد <= parts.size() && "Integer too big");

    إذا (category == fcNaN)
      bits = 0;
    وإلا إذا (sign)
      bits = isSigned;
    وإلا
      bits = width - isSigned;

    tcSetLeastSignificantBits(parts.data(), dst_Parts_تعداد, bits);
    إذا (sign && isSigned)
      APInt::tc_Shift_يسار(parts.data(), dst_Parts_تعداد, width - 1);
  }

  إرجاع fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::opStatus IEEEFloat::convertFromUnsignedParts(
    ثابت integerPart *src, unsigned صحيح src_تعداد, roundingMode rounding_mode) {
  category = fcNormal;
  unsigned omsb = APInt::tcMSB(src, src_تعداد) + 1;
  integerPart *dst = significandParts();
  unsigned dst_تعداد = part_تعداد();
  unsigned precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  lostFraction lost_fraction;
  إذا (precision <= omsb) {
    exponent = omsb - 1;
    lost_fraction = lostFractionThroughTruncation(src, src_تعداد,
                                                  omsb - precision);
    APInt::tcExtract(dst, dst_تعداد, src, precision, omsb - precision);
  } وإلا {
    exponent = precision - 1;
    lost_fraction = lfExactlyZero;
    APInt::tcExtract(dst, dst_تعداد, src, omsb, 0);
  }

  إرجاع normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus IEEEFloat::convertFromAPInt(ثابت APInt &Val, منطقي isSigned,
                                              roundingMode rounding_mode) {
  unsigned صحيح part_تعداد = Val.get_رقم_كلمات();
  APInt api = Val;

  sign = خطأ;
  إذا (isSigned && api.isNegative()) {
    sign = true;
    api = -api;
  }

  إرجاع convertFromUnsignedParts(api.getRawData(), part_تعداد, rounding_mode);
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        roundingMode rounding_mode) {
  lostFraction lost_fraction = lfExactlyZero;

  category = fcNormal;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  unsigned parts_تعداد = part_تعداد();
  unsigned bitPos = parts_تعداد * integerPartWidth;
  منطقي computedTrailingFraction = خطأ;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator end = s.end();
  StringRef::iterator dot;
  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  إذا (!PtrOrErr)
    إرجاع PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator firstSignificantDigit = p;

  بينما (p != end) {
    integerPart hex_قيمة;

    إذا (*p == '.') {
      إذا (dot != end)
        إرجاع createError("String contains multiple dots");
      dot = p++;
      استمر;
    }

    hex_قيمة = hex_Digit_قيمة(*p);
    إذا (hex_قيمة == UINT_MAX)
      اكسر;

    p++;

    // Store the number while we have space.
    إذا (bitPos) {
      bitPos -= 4;
      hex_قيمة <<= bitPos % integerPartWidth;
      significand[bitPos / integerPartWidth] |= hex_قيمة;
    } وإلا إذا (!computedTrailingFraction) {
      auto FractOrErr = trailingHexadecimalFraction(p, end, hex_قيمة);
      إذا (!FractOrErr)
        إرجاع FractOrErr.takeError();
      lost_fraction = *FractOrErr;
      computedTrailingFraction = true;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  إذا (p == end)
    إرجاع createError("Hex strings require an exponent");
  إذا (*p != 'p' && *p != 'P')
    إرجاع createError("Invalid character in significand");
  إذا (p == begin)
    إرجاع createError("Significand has no digits");
  إذا (dot != end && p - begin == 1)
    إرجاع createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  إذا (p != firstSignificantDigit) {
    صحيح expAdjustment;

    /* Implicit hexadecimal point?  */
    إذا (dot == end)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<صحيح>(dot - firstSignificantDigit);
    إذا (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_تعداد * integerPartWidth;

    /* Adjust for the given exponent.  */
    auto ExpOrErr = totalExponent(p + 1, end, expAdjustment);
    إذا (!ExpOrErr)
      إرجاع ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  إرجاع normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus
IEEEFloat::roundSignificandWithExponent(ثابت integerPart *decSigParts,
                                        unsigned sig_Part_تعداد, صحيح exp,
                                        roundingMode rounding_mode) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  unsigned صحيح power = exp >= 0 ? exp : -exp;
  unsigned صحيح pow5PartCapacity = powerOfFivePartsForExponent(power);
  SmallVector<integerPart, رقم_Pow5_Parts_On_مكدس> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5PartCapacity);

  منطقي isNearest = rounding_mode == rmNearestTiesToEven ||
                   rounding_mode == rmNearestTiesToAway;

  unsigned parts = part_تعداد_For_Bits(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  unsigned pow5_Part_تعداد = powerOf5(pow5Parts.data(), power);

  لكل (;; parts *= 2) {
    unsigned صحيح excessPrecision, truncatedBits;

    calcSemantics.precision = parts * integerPartWidth - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncatedBits = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    opStatus sigStatus = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_تعداد, rmNearestTiesToEven);
    opStatus powStatus = pow5.convertFromUnsignedParts(
        pow5Parts.data(), pow5_Part_تعداد, rmNearestTiesToEven);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += exp;

    lostFraction calcLostFraction;
    integerPart HUerr, HUdistance;
    unsigned صحيح powHUerr;

    إذا (exp >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calcLostFraction = decSig.multiplySignificand(pow5);
      powHUerr = powStatus != opOK;
    } وإلا {
      calcLostFraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      إذا (decSig.exponent < semantics->minExponent) {
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
    إذا (HUdistance >= HUerr) {
      APInt::tcExtract(significandParts(), part_تعداد(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calcLostFraction = lostFractionThroughTruncation(decSig.significandParts(),
                                                       decSig.part_تعداد(),
                                                       truncatedBits);
      إرجاع static_cast<opStatus>(normalize(rounding_mode, calcLostFraction) |
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
  إذا (Error Err = interpretDecimal(p, str.end(), &D))
    إرجاع std::move(Err);

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
  إذا (D.firstSigDigit == str.end() || dec_Digit_قيمة(*D.firstSigDigit) >= 10U) {
    category = fcZero;
    fs = opOK;
    إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = خطأ;
    إذا (!semantics->hasZero)
      makeSmallestNormalized(خطأ);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } وإلا إذا (D.normalizedExponent - 1 > INT_MAX / 42039) {
    fs = handleOverflow(rounding_mode);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } وإلا إذا (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (صحيح) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fcNormal;
    zeroSignificand();
    fs = normalize(rounding_mode, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } وإلا إذا ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handleOverflow(rounding_mode);
  } وإلا {
    integerPart *decSignificand;
    unsigned صحيح part_تعداد;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_تعداد = static_cast<unsigned صحيح>(D.lastSigDigit - D.firstSigDigit) + 1;
    part_تعداد = part_تعداد_For_Bits(1 + 196 * part_تعداد / 59);
    decSignificand = جديد integerPart[part_تعداد + 1];
    part_تعداد = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    افعل {
      integerPart dec_قيمة, val, multiplier;

      val = 0;
      multiplier = 1;

      افعل {
        إذا (*p == '.') {
          p++;
          إذا (p == str.end()) {
            اكسر;
          }
        }
        dec_قيمة = dec_Digit_قيمة(*p++);
        إذا (dec_قيمة >= 10U) {
          delete[] decSignificand;
          إرجاع createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_قيمة;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } بينما (p <= D.lastSigDigit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_تعداد, part_تعداد + 1, خطأ);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      إذا (decSignificand[part_تعداد])
        part_تعداد++;
    } بينما (p <= D.lastSigDigit);

    category = fcNormal;
    fs = roundSignificandWithExponent(decSignificand, part_تعداد,
                                      D.exponent, rounding_mode);

    delete [] decSignificand;
  }

  إرجاع fs;
}

منطقي IEEEFloat::convertFromStringSpecials(StringRef str) {
  ثابت size_t MIN_NAME_SIZE = 3;

  إذا (str.size() < MIN_NAME_SIZE)
    إرجاع خطأ;

  إذا (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(خطأ);
    إرجاع true;
  }

  منطقي IsNegative = str.consume_front("-");
  إذا (IsNegative) {
    إذا (str.size() < MIN_NAME_SIZE)
      إرجاع خطأ;

    إذا (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(true);
      إرجاع true;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  منطقي IsSignaling = str.consume_front_insensitive("s");
  إذا (IsSignaling) {
    إذا (str.size() < MIN_NAME_SIZE)
      إرجاع خطأ;
  }

  إذا (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    إذا (str.empty()) {
      makeNaN(IsSignaling, IsNegative);
      إرجاع true;
    }

    // Allow the payload to be inside parentheses.
    إذا (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      إذا (str.size() <= 2 || str.back() != ')')
        إرجاع خطأ;

      str = str.slice(1, str.size() - 1);
    }

    // Determine the payload number's radix.
    unsigned Radix = 10;
    إذا (str[0] == '0') {
      إذا (str.size() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } وإلا {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    إذا (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      إرجاع true;
    }
  }

  إرجاع خطأ;
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromString(StringRef str, roundingMode rounding_mode) {
  إذا (str.empty())
    إرجاع createError("Invalid string length");

  // Handle special cases.
  إذا (convertFromStringSpecials(str))
    إرجاع opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  sign = *p == '-' ? 1 : 0;
  إذا (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  إذا (*p == '-' || *p == '+') {
    p++;
    slen--;
    إذا (!slen)
      إرجاع createError("String has no digits");
  }

  إذا (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    إذا (slen == 2)
      إرجاع createError("Invalid string");
    إرجاع convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_mode);
  }

  إرجاع convertFromDecimalString(StringRef(p, slen), rounding_mode);
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
  إذا (sign)
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
    إذا (hexDigits > 1) {
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

  إرجاع static_cast<unsigned صحيح>(dst - p);
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
  unsigned parts_تعداد = part_تعداد();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  unsigned قيمة_Bits = semantics->precision + 3;
  unsigned shift = integerPartWidth - قيمة_Bits % integerPartWidth;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  unsigned outputDigits = (قيمة_Bits - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  إذا (hexDigits) {
    إذا (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      unsigned صحيح bits;
      lostFraction fraction;

      bits = قيمة_Bits - hexDigits * 4;
      fraction = lostFractionThroughTruncation (significand, parts_تعداد, bits);
      roundUp = roundAwayFromZero(rounding_mode, fraction, bits);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  حرف *p = ++dst;

  unsigned تعداد_2 = (قيمة_Bits + integerPartWidth - 1) / integerPartWidth;

  بينما (outputDigits && تعداد_2) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    إذا (--تعداد_2 == parts_تعداد)
      part = 0;  /* An imaginary higher zero part.  */
    وإلا
      part = significand[تعداد_2] << shift;

    إذا (تعداد_2 && shift)
      part |= significand[تعداد_2 - 1] >> (integerPartWidth - shift);

    /* Convert as much of "part" to hexdigits as we can.  */
    unsigned صحيح curDigits = integerPartWidth / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  إذا (roundUp) {
    حرف *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    افعل {
      q--;
      *q = hexDigitChars[hex_Digit_قيمة (*q) + 1];
    } بينما (*q == '0');
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
  إذا (dst -1 == p)
    dst--;
  وإلا
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = upperCase ? 'P': 'p';

  إرجاع writeSignedDecimal (dst, exponent);
}

hash_code hash_قيمة(ثابت IEEEFloat &Arg) {
  إذا (!Arg.isFiniteNonZero())
    إرجاع hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  إرجاع hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.part_تعداد()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convertF80LongDoubleAPFloatToAPInt() ثابت {
  أكد(part_تعداد() == 2);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semX87DoubleExtended>();
}

APInt IEEEFloat::convertPPCDoubleDoubleLegacyAPFloatToAPInt() ثابت {
  أكد(semantics ==
         (ثابت llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy);
  أكد(part_تعداد()==2);

  uint64_t كلمات[2];
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
  كلمات[0] = *u.convertDoubleAPFloatToAPInt().getRawData();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  إذا (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
    أكد(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.subtract(u, rmNearestTiesToEven);
    fs = v.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
    أكد(fs == opOK && !losesInfo);
    كلمات[1] = *v.convertDoubleAPFloatToAPInt().getRawData();
  } وإلا {
    كلمات[1] = 0;
  }

  إرجاع APInt(128, كلمات);
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
  std::array<integerPart, part_تعداد_For_Bits(trailing_significand_bits)>
      mysignificand;

  إذا (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
    إذا (myexponent == 1 &&
        !(significandParts()[integer_bit_part] & integer_bit))
      myexponent = 0; // denormal
  } وإلا إذا (category == fcZero) {
    إذا (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } وإلا إذا (category == fcInfinity) {
    إذا (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    إذا constexpr (S.hasExplicitIntegerBit) {
      mysignificand[0] = integerPart{1} << (trailing_significand_bits - 1);
    }
  } وإلا {
    أكد(category == fcNaN && "Unknown category!");
    إذا (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.sizeInBits + 63) / 64> كلمات;
  auto كلمات_iter =
      std::copy_n(mysignificand.begin(), mysignificand.size(), كلمات.begin());
  إذا constexpr (!S.hasExplicitIntegerBit) {
    إذا constexpr (significand_mask != 0 || trailing_significand_bits == 0) {
      // Clear the integer bit.
      كلمات[mysignificand.size() - 1] &= significand_mask;
    }
  }
  std::fill(كلمات_iter, كلمات.end(), uint64_t{0});
  constexpr size_t last_word = كلمات.size() - 1;
  إذا constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.sizeInBits - 1) % 64);
    كلمات[last_word] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_mask)
                              << (trailing_significand_bits % 64);
  كلمات[last_word] |= shifted_exponent;
  إذا constexpr (last_word == 0) {
    إرجاع APInt(S.sizeInBits, كلمات[0]);
  }
  إرجاع APInt(S.sizeInBits, كلمات);
}

APInt IEEEFloat::convertQuadrupleAPFloatToAPInt() ثابت {
  أكد(part_تعداد() == 2);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semIEEEquad>();
}

APInt IEEEFloat::convertDoubleAPFloatToAPInt() ثابت {
  أكد(part_تعداد()==1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semIEEEdouble>();
}

APInt IEEEFloat::convertFloatAPFloatToAPInt() ثابت {
  أكد(part_تعداد()==1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semIEEEsingle>();
}

APInt IEEEFloat::convertBFloatAPFloatToAPInt() ثابت {
  أكد(part_تعداد() == 1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semBFloat>();
}

APInt IEEEFloat::convertHalfAPFloatToAPInt() ثابت {
  أكد(part_تعداد()==1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::APFloatBase::semIEEEhalf>();
}

APInt IEEEFloat::convertFloat8E5M2APFloatToAPInt() ثابت {
  أكد(part_تعداد() == 1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2>();
}

APInt IEEEFloat::convertFloat8E5M2FNUZAPFloatToAPInt() ثابت {
  أكد(part_تعداد() == 1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3APFloatToAPInt() ثابت {
  أكد(part_تعداد() == 1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3>();
}

APInt IEEEFloat::convertFloat8E4M3FNAPFloatToAPInt() ثابت {
  أكد(part_تعداد() == 1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FN>();
}

APInt IEEEFloat::convertFloat8E4M3FNUZAPFloatToAPInt() ثابت {
  أكد(part_تعداد() == 1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3B11FNUZAPFloatToAPInt() ثابت {
  أكد(part_تعداد() == 1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convertFloat8E3M4APFloatToAPInt() ثابت {
  أكد(part_تعداد() == 1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semFloat8E3M4>();
}

APInt IEEEFloat::convertFloatTF32APFloatToAPInt() ثابت {
  أكد(part_تعداد() == 1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semFloatTF32>();
}

APInt IEEEFloat::convertFloat8E8M0FNUAPFloatToAPInt() ثابت {
  أكد(part_تعداد() == 1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convertFloat8E5M3FNUAPFloatToAPInt() ثابت {
  أكد(part_تعداد() == 1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convertFloat6E3M2FNAPFloatToAPInt() ثابت {
  أكد(part_تعداد() == 1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semFloat6E3M2FN>();
}

APInt IEEEFloat::convertFloat6E2M3FNAPFloatToAPInt() ثابت {
  أكد(part_تعداد() == 1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semFloat6E2M3FN>();
}

APInt IEEEFloat::convertFloat4E2M1FNAPFloatToAPInt() ثابت {
  أكد(part_تعداد() == 1);
  إرجاع convertIEEEFloatToAPInt<APFloatBase::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcastToAPInt() ثابت {
  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEhalf)
    إرجاع convertHalfAPFloatToAPInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semBFloat)
    إرجاع convertBFloatAPFloatToAPInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEsingle)
    إرجاع convertFloatAPFloatToAPInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEdouble)
    إرجاع convertDoubleAPFloatToAPInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEquad)
    إرجاع convertQuadrupleAPFloatToAPInt();

  إذا (semantics ==
      (ثابت llvm::fltSemantics *)&APFloatBase::semX87DoubleExtended)
    إرجاع convertF80LongDoubleAPFloatToAPInt();

  إذا (semantics ==
      (ثابت llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy)
    إرجاع convertPPCDoubleDoubleLegacyAPFloatToAPInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E5M2)
    إرجاع convertFloat8E5M2APFloatToAPInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E5M2FNUZ)
    إرجاع convertFloat8E5M2FNUZAPFloatToAPInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E4M3)
    إرجاع convertFloat8E4M3APFloatToAPInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FN)
    إرجاع convertFloat8E4M3FNAPFloatToAPInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FNUZ)
    إرجاع convertFloat8E4M3FNUZAPFloatToAPInt();

  إذا (semantics ==
      (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E4M3B11FNUZ)
    إرجاع convertFloat8E4M3B11FNUZAPFloatToAPInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E3M4)
    إرجاع convertFloat8E3M4APFloatToAPInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloatTF32)
    إرجاع convertFloatTF32APFloatToAPInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E8M0FNU)
    إرجاع convertFloat8E8M0FNUAPFloatToAPInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E5M3FNU)
    إرجاع convertFloat8E5M3FNUAPFloatToAPInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat6E3M2FN)
    إرجاع convertFloat6E3M2FNAPFloatToAPInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat6E2M3FN)
    إرجاع convertFloat6E2M3FNAPFloatToAPInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat4E2M1FN)
    إرجاع convertFloat4E2M1FNAPFloatToAPInt();

  llvm_unreachable("unknown format!");
}

عائم IEEEFloat::convertToFloat() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcastToAPInt();
  إرجاع api.bitsToFloat();
}

مزدوج IEEEFloat::convertToDouble() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcastToAPInt();
  إرجاع api.bitsToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 IEEEFloat::convertToQuad() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcastToAPInt();
  إرجاع api.bitsToQuad();
}
#endif

فراغ IEEEFloat::initFromF80LongDoubleAPInt(ثابت APInt &api) {
  إرجاع initFromIEEEAPInt<APFloatBase::semX87DoubleExtended>(api);
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
  إذا (isFiniteNonZero()) {
    IEEEFloat v(APFloatBase::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    إذا (v.isFinite()) {
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
      part_تعداد_For_Bits(trailing_significand_bits + 1);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr منطقي is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr صحيح bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr منطقي has_significand = trailing_significand_bits > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  إذا constexpr (has_significand) {
    std::copy_n(api.getRawData(), mysignificand.size(), mysignificand.begin());
    إذا constexpr (significand_mask != 0 || S.precision >= integerPartWidth) {
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
  uint64_t last_word = api.getRawData()[api.get_رقم_كلمات() - 1];
  uint64_t myexponent =
      (last_word >> (trailing_significand_bits % 64)) & exponent_mask;

  initialize(&S);
  أكد(part_تعداد() == mysignificand.size());

  sign = S.hasSignedRepr
             ? static_cast<unsigned صحيح>(last_word >> ((S.sizeInBits - 1) % 64))
             : 0;

  منطقي all_zero_significand =
      has_significand && llvm::all_of(mysignificand, equal_to(0));

  منطقي is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  إذا constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    منطقي is_inf = خطأ;

    إذا constexpr (S.hasExplicitIntegerBit) {
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

    إذا (is_inf) {
      makeInf(sign);
      إرجاع;
    }
  }

  منطقي is_nan = خطأ;

  إذا constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    إذا constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      ثابت integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;

      إذا (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_mask_no_int_bit) != 0) {
        // regular NaN and pseudoNaN
        is_nan = true;
      } وإلا إذا (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_mask_no_int_bit) == 0) {
        // pseudoinfinity
        is_nan = true;
      } وإلا إذا (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = true;
      }
    } وإلا {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } وإلا إذا constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    منطقي all_ones_significand =
        std::all_of(mysignificand.begin(), mysignificand.end() - 1,
                    [](integerPart bits) { إرجاع bits == ~integerPart{0}; }) &&
        (!significand_mask ||
         mysignificand[mysignificand.size() - 1] == significand_mask);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } وإلا إذا constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  إذا (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.size(),
                significandParts());
    إرجاع;
  }

  إذا (is_zero) {
    makeZero(sign);
    إرجاع;
  }

  category = fcNormal;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.size(), significandParts());
  إذا (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  وإلا {
    إذا constexpr (!S.hasExplicitIntegerBit) {
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
  إذا (Sem == &APFloatBase::semIEEEhalf)
    إرجاع initFromHalfAPInt(api);
  إذا (Sem == &APFloatBase::semBFloat)
    إرجاع initFromBFloatAPInt(api);
  إذا (Sem == &APFloatBase::semIEEEsingle)
    إرجاع initFromFloatAPInt(api);
  إذا (Sem == &APFloatBase::semIEEEdouble)
    إرجاع initFromDoubleAPInt(api);
  إذا (Sem == &APFloatBase::semIEEEquad)
    إرجاع initFromQuadrupleAPInt(api);
  إذا (Sem == &APFloatBase::semX87DoubleExtended)
    إرجاع initFromF80LongDoubleAPInt(api);
  إذا (Sem == &APFloatBase::semPPCDoubleDoubleLegacy)
    إرجاع initFromPPCDoubleDoubleLegacyAPInt(api);
  إذا (Sem == &APFloatBase::semFloat8E5M2)
    إرجاع initFromFloat8E5M2APInt(api);
  إذا (Sem == &APFloatBase::semFloat8E5M2FNUZ)
    إرجاع initFromFloat8E5M2FNUZAPInt(api);
  إذا (Sem == &APFloatBase::semFloat8E4M3)
    إرجاع initFromFloat8E4M3APInt(api);
  إذا (Sem == &APFloatBase::semFloat8E4M3FN)
    إرجاع initFromFloat8E4M3FNAPInt(api);
  إذا (Sem == &APFloatBase::semFloat8E4M3FNUZ)
    إرجاع initFromFloat8E4M3FNUZAPInt(api);
  إذا (Sem == &APFloatBase::semFloat8E4M3B11FNUZ)
    إرجاع initFromFloat8E4M3B11FNUZAPInt(api);
  إذا (Sem == &APFloatBase::semFloat8E3M4)
    إرجاع initFromFloat8E3M4APInt(api);
  إذا (Sem == &APFloatBase::semFloatTF32)
    إرجاع initFromFloatTF32APInt(api);
  إذا (Sem == &APFloatBase::semFloat8E8M0FNU)
    إرجاع initFromFloat8E8M0FNUAPInt(api);
  إذا (Sem == &APFloatBase::semFloat8E5M3FNU)
    إرجاع initFromFloat8E5M3FNUAPInt(api);
  إذا (Sem == &APFloatBase::semFloat6E3M2FN)
    إرجاع initFromFloat6E3M2FNAPInt(api);
  إذا (Sem == &APFloatBase::semFloat6E2M3FN)
    إرجاع initFromFloat6E2M3FNAPInt(api);
  إذا (Sem == &APFloatBase::semFloat4E2M1FN)
    إرجاع initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
فراغ IEEEFloat::makeLargest(منطقي Negative) {
  إذا (Negative && !semantics->hasSignedRepr)
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
  unsigned Part_تعداد = part_تعداد();
  memset(significand, 0xFF, حجم(integerPart)*(Part_تعداد - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  ثابت unsigned رقم_Unused_أعلى_Bits =
    Part_تعداد*integerPartWidth - semantics->precision;
  significand[Part_تعداد - 1] = (رقم_Unused_أعلى_Bits < integerPartWidth)
                                   ? (~integerPart(0) >> رقم_Unused_أعلى_Bits)
                                   : 0;
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
فراغ IEEEFloat::makeSmallest(منطقي Negative) {
  إذا (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fcNormal;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tcSet(significandParts(), 1, part_تعداد());
}

فراغ IEEEFloat::makeSmallestNormalized(منطقي Negative) {
  إذا (Negative && !semantics->hasSignedRepr)
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

    إذا (bits <= bitsRequired) إرجاع;

    unsigned tensRemovable = (bits - bitsRequired) * 59 / 196;
    إذا (!tensRemovable) إرجاع;

    exp += tensRemovable;

    APInt قاسم(significand.getBitWidth(), 1);
    APInt powten(significand.getBitWidth(), 10);
    بينما (true) {
      إذا (tensRemovable & 1)
        قاسم *= powten;
      tensRemovable >>= 1;
      إذا (!tensRemovable) اكسر;
      powten *= powten;
    }

    significand = significand.udiv(قاسم);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.getActiveBits());
  }


  فراغ AdjustToPrecision(SmallVectorImpl<حرف> &buffer,
                         صحيح &exp, unsigned FormatPrecision) {
    unsigned N = buffer.size();
    إذا (N <= FormatPrecision) إرجاع;

    // The most significant figures are the last ones in the buffer.
    unsigned FirstSignificant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    إذا (buffer[FirstSignificant - 1] < '5') {
      بينما (FirstSignificant < N && buffer[FirstSignificant] == '0')
        FirstSignificant++;

      exp += FirstSignificant;
      buffer.erase(&buffer[0], &buffer[FirstSignificant]);
      إرجاع;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    لكل (unsigned I = FirstSignificant; I != N; ++I) {
      إذا (buffer[I] == '9') {
        FirstSignificant++;
      } وإلا {
        buffer[I]++;
        اكسر;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    إذا (FirstSignificant == N) {
      exp += FirstSignificant;
      buffer.clear();
      buffer.ادفع_back('1');
      إرجاع;
    }

    exp += FirstSignificant;
    buffer.erase(&buffer[0], &buffer[FirstSignificant]);
  }

  فراغ toStringImpl(SmallVectorImpl<حرف> &Str, ثابت منطقي isNeg, صحيح exp,
                    APInt significand, unsigned FormatPrecision,
                    unsigned FormatMaxPadding, منطقي TruncateZero) {
    ثابت صحيح semanticsPrecision = significand.getBitWidth();

    إذا (isNeg)
      Str.ادفع_back('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    إذا (!FormatPrecision) {
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
    إذا (exp == 0) {
      // Nothing to do.
    } وإلا إذا (exp > 0) {
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
      بينما (true) {
        إذا (texp & 1)
          significand *= five_to_the_i;

        texp >>= 1;
        إذا (!texp)
          اكسر;
        five_to_the_i *= five_to_the_i;
      }
    }

    AdjustToPrecision(significand, exp, FormatPrecision);

    SmallVector<حرف, 256> buffer;

    // Fill the buffer.
    unsigned precision = significand.getBitWidth();
    إذا (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    منطقي inTrail = true;
    بينما (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      unsigned d = digit.get_ZExt_قيمة();

      // Drop trailing zeros.
      إذا (inTrail && !d)
        exp++;
      وإلا {
        buffer.ادفع_back((حرف) ('0' + d));
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
    إذا (!FormatMaxPadding) {
      FormatScientific = true;
    } وإلا {
      إذا (exp >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((unsigned) exp > FormatMaxPadding ||
                            NDigits + (unsigned) exp > FormatPrecision);
      } وإلا {
        // Power of the most significant digit.
        صحيح MSD = exp + (صحيح) (NDigits - 1);
        إذا (MSD >= 0) {
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
    إذا (FormatScientific) {
      exp += (NDigits - 1);

      Str.ادفع_back(buffer[NDigits-1]);
      Str.ادفع_back('.');
      إذا (NDigits == 1 && TruncateZero)
        Str.ادفع_back('0');
      وإلا
        لكل (unsigned I = 1; I != NDigits; ++I)
          Str.ادفع_back(buffer[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      إذا (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.ادفع_back(TruncateZero ? 'E' : 'e');

      Str.ادفع_back(exp >= 0 ? '+' : '-');
      إذا (exp < 0)
        exp = -exp;
      SmallVector<حرف, 6> expbuf;
      افعل {
        expbuf.ادفع_back((حرف) ('0' + (exp % 10)));
        exp /= 10;
      } بينما (exp);
      // Exponent always at least two digits if we do not truncate zeros.
      إذا (!TruncateZero && expbuf.size() < 2)
        expbuf.ادفع_back('0');
      لكل (unsigned I = 0, E = expbuf.size(); I != E; ++I)
        Str.ادفع_back(expbuf[E-1-I]);
      إرجاع;
    }

    // Non-scientific, positive exponents.
    إذا (exp >= 0) {
      لكل (unsigned I = 0; I != NDigits; ++I)
        Str.ادفع_back(buffer[NDigits-1-I]);
      لكل (unsigned I = 0; I != (unsigned) exp; ++I)
        Str.ادفع_back('0');
      إرجاع;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    صحيح NWholeDigits = exp + (صحيح) NDigits;

    unsigned I = 0;
    إذا (NWholeDigits > 0) {
      لكل (; I != (unsigned) NWholeDigits; ++I)
        Str.ادفع_back(buffer[NDigits-I-1]);
      Str.ادفع_back('.');
    } وإلا {
      unsigned NZeros = 1 + (unsigned) -NWholeDigits;

      Str.ادفع_back('0');
      Str.ادفع_back('.');
      لكل (unsigned Z = 1; Z != NZeros; ++Z)
        Str.ادفع_back('0');
    }

    لكل (; I != NDigits; ++I)
      Str.ادفع_back(buffer[NDigits-I-1]);

  }
} // namespace

فراغ IEEEFloat::toString(SmallVectorImpl<حرف> &Str, unsigned FormatPrecision,
                         unsigned FormatMaxPadding, منطقي TruncateZero) ثابت {
  بدل (category) {
  حالة fcInfinity:
    إذا (isNegative())
      إرجاع append(Str, "-Inf");
    وإلا
      إرجاع append(Str, "+Inf");

  حالة fcNaN: إرجاع append(Str, "NaN");

  حالة fcZero:
    إذا (isNegative())
      Str.ادفع_back('-');

    إذا (!FormatMaxPadding) {
      إذا (TruncateZero)
        append(Str, "0.0E+0");
      وإلا {
        append(Str, "0.0");
        إذا (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } وإلا {
      Str.ادفع_back('0');
    }
    إرجاع;

  حالة fcNormal:
    اكسر;
  }

  // Decompose the number into an APInt and an exponent.
  صحيح exp = exponent - ((صحيح) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_تعداد_For_Bits(semantics->precision)));

  toStringImpl(Str, isNegative(), exp, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

صحيح IEEEFloat::getExactLog2Abs() ثابت {
  إذا (!isFinite() || isZero())
    إرجاع INT_MIN;

  ثابت integerPart *Parts = significandParts();
  ثابت صحيح Part_تعداد = part_تعداد_For_Bits(semantics->precision);

  صحيح اسحب_تعداد = 0;
  لكل (صحيح i = 0; i < Part_تعداد; ++i) {
    اسحب_تعداد += llvm::popcount(Parts[i]);
    إذا (اسحب_تعداد > 1)
      إرجاع INT_MIN;
  }

  إذا (exponent != semantics->minExponent)
    إرجاع exponent;

  صحيح CountrParts = 0;
  لكل (صحيح i = 0; i < Part_تعداد;
       ++i, CountrParts += APInt::APINT_BITS_PER_WORD) {
    إذا (Parts[i] != 0) {
      إرجاع exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

منطقي IEEEFloat::isSignaling() ثابت {
  إذا (!isNaN())
    إرجاع خطأ;
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    إرجاع خطأ;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  إرجاع !APInt::tcExtractBit(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::opStatus IEEEFloat::next(منطقي nextDown) {
  // If we are performing nextDown, swap sign so we have -x.
  إذا (nextDown)
    changeSign();

  // Compute nextUp(x)
  opStatus result = opOK;

  // Handle each float category separately.
  بدل (category) {
  حالة fcInfinity:
    // nextUp(+inf) = +inf
    إذا (!isNegative())
      اكسر;
    // nextUp(-inf) = -getLargest()
    makeLargest(true);
    اكسر;
  حالة fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    إذا (isSignaling()) {
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
    إذا (isSmallest() && isNegative()) {
      APInt::tcSet(significandParts(), 0, part_تعداد());
      category = fcZero;
      exponent = 0;
      إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = خطأ;
      إذا (!semantics->hasZero)
        makeSmallestNormalized(خطأ);
      اكسر;
    }

    إذا (isLargest() && !isNegative()) {
      إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        اكسر;
      } وإلا إذا (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        اكسر;
      } وإلا {
        // nextUp(getLargest()) == INFINITY
        APInt::tcSet(significandParts(), 0, part_تعداد());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        اكسر;
      }
    }

    // nextUp(normal) == normal + inc.
    إذا (isNegative()) {
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
      APInt::tcDecrement(Parts, part_تعداد());

      إذا (WillCrossBinadeBoundary) {
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

      إذا (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tcSet(Parts, 0, part_تعداد());
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
  إذا (nextDown)
    changeSign();

  إرجاع result;
}

APInt IEEEFloat::getNaNPayload() ثابت {
  أكد(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  unsigned Bits = semantics->precision - 1;
  إرجاع APInt(Bits, ArrayRef(significandParts(), part_تعداد_For_Bits(Bits)));
}

APFloatBase::ExponentType IEEEFloat::exponentNaN() ثابت {
  إرجاع ::exponentNaN(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentInf() ثابت {
  إرجاع ::exponentInf(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentZero() ثابت {
  إرجاع ::exponentZero(*semantics);
}

فراغ IEEEFloat::makeInf(منطقي Negative) {
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(خطأ, Negative);
    إرجاع;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tcSet(significandParts(), 0, part_تعداد());
}

فراغ IEEEFloat::makeZero(منطقي Negative) {
  إذا (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = خطأ;
  }
  exponent = exponentZero();
  APInt::tcSet(significandParts(), 0, part_تعداد());
}

فراغ IEEEFloat::makeQuiet() {
  أكد(isNaN());
  إذا (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tcSetBit(significandParts(), semantics->precision - 2);
}

صحيح ilogb(ثابت IEEEFloat &Arg) {
  إذا (Arg.isNaN())
    إرجاع APFloat::IEK_NaN;
  إذا (Arg.isZero())
    إرجاع APFloat::IEK_Zero;
  إذا (Arg.isInfinity())
    إرجاع APFloat::IEK_Inf;
  إذا (!Arg.isDenormal())
    إرجاع Arg.exponent;

  IEEEFloat Normalized(Arg);
  صحيح SignificandBits = Arg.getSemantics().precision - 1;

  Normalized.exponent += SignificandBits;
  Normalized.normalize(APFloat::rmNearestTiesToEven, lfExactlyZero);
  إرجاع Normalized.exponent - SignificandBits;
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
  إذا (X.isNaN())
    X.makeQuiet();
  إرجاع X;
}

IEEEFloat frexp(ثابت IEEEFloat &Val, صحيح &Exp, roundingMode RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  إذا (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    إرجاع Quiet;
  }

  إذا (Exp == APFloat::IEK_Inf)
    إرجاع Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  إرجاع scalbn(Val, -Exp, RM);
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
  إذا (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } وإلا إذا (هذا != &RHS) {
    هذا->~DoubleAPFloat();
    جديد (هذا) DoubleAPFloat(RHS);
  }
  إرجاع *هذا;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
ساكن std::pair<APFloat, APFloat> fastTwoSum(APFloat X, APFloat Y) {
  إذا (!X.isFinite())
    إرجاع {X, APFloat::getZero(X.getSemantics(), /*Negative=*/خطأ)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  إرجاع {Hi, Lo};
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
  إذا (!z.isFinite()) {
    إذا (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ خطأ);
      إرجاع (opStatus)Status;
    }
    Status = opOK;
    auto AComparedToC = a.compare_Absolute_قيمة(c);
    z = cc;
    Status |= z.add(aa, RM);
    إذا (AComparedToC == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      Status |= z.add(c, RM);
      Status |= z.add(a, RM);
    } وإلا {
      // z = cc + aa + a + c;
      Status |= z.add(a, RM);
      Status |= z.add(c, RM);
    }
    إذا (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ خطأ);
      إرجاع (opStatus)Status;
    }
    Floats[0] = z;
    APFloat zz = aa;
    Status |= zz.add(cc, RM);
    إذا (AComparedToC == APFloat::cmpGreaterThan) {
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
    إذا (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ خطأ);
      إرجاع opOK;
    }
    Floats[0] = z;
    Status |= Floats[0].add(zz, RM);
    إذا (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ خطأ);
      إرجاع (opStatus)Status;
    }
    Floats[1] = std::move(z);
    Status |= Floats[1].subtract(Floats[0], RM);
    Status |= Floats[1].add(zz, RM);
  }
  إرجاع (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::addWithSpecial(ثابت DoubleAPFloat &LHS,
                                                ثابت DoubleAPFloat &RHS,
                                                DoubleAPFloat &Out,
                                                roundingMode RM) {
  إذا (LHS.getCategory() == fcNaN) {
    Out = LHS;
    إرجاع opOK;
  }
  إذا (RHS.getCategory() == fcNaN) {
    Out = RHS;
    إرجاع opOK;
  }
  إذا (LHS.getCategory() == fcZero) {
    Out = RHS;
    إرجاع opOK;
  }
  إذا (RHS.getCategory() == fcZero) {
    Out = LHS;
    إرجاع opOK;
  }
  إذا (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    Out.makeNaN(خطأ, Out.isNegative(), nullptr);
    إرجاع opInvalidOp;
  }
  إذا (LHS.getCategory() == fcInfinity) {
    Out = LHS;
    إرجاع opOK;
  }
  إذا (RHS.getCategory() == fcInfinity) {
    Out = RHS;
    إرجاع opOK;
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
  إرجاع Out.addImpl(A, AA, C, CC, RM);
}

APFloat::opStatus DoubleAPFloat::add(ثابت DoubleAPFloat &RHS,
                                     roundingMode RM) {
  إرجاع addWithSpecial(*هذا, RHS, *هذا, RM);
}

APFloat::opStatus DoubleAPFloat::subtract(ثابت DoubleAPFloat &RHS,
                                          roundingMode RM) {
  changeSign();
  auto Ret = add(RHS, RM);
  changeSign();
  إرجاع Ret;
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
  إذا (LHS.getCategory() == fcNaN) {
    Out = LHS;
    إرجاع opOK;
  }
  إذا (RHS.getCategory() == fcNaN) {
    Out = RHS;
    إرجاع opOK;
  }
  إذا ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    Out.makeNaN(خطأ, خطأ, nullptr);
    إرجاع opOK;
  }
  إذا (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    Out = LHS;
    إرجاع opOK;
  }
  إذا (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    Out = RHS;
    إرجاع opOK;
  }
  أكد(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal &&
         "Special cases not handled exhaustively");

  صحيح Status = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  Status |= T.multiply(C, RM);
  إذا (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ خطأ);
    إرجاع (opStatus)Status;
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
  إذا (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ خطأ);
  } وإلا {
    // Floats[1] = (t - u) + tau
    Status |= T.subtract(U, RM);
    Status |= T.add(Tau, RM);
    Floats[1] = std::move(T);
  }
  إرجاع (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::divide(ثابت DoubleAPFloat &RHS,
                                        APFloat::roundingMode RM) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.divide(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()), RM);
  *هذا = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  إرجاع Ret;
}

APFloat::opStatus DoubleAPFloat::remainder(ثابت DoubleAPFloat &RHS) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.remainder(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *هذا = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  إرجاع Ret;
}

APFloat::opStatus DoubleAPFloat::mod(ثابت DoubleAPFloat &RHS) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.mod(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *هذا = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  إرجاع Ret;
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
  إرجاع Ret;
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
  إذا (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/خطأ);
    إرجاع HiStatus;
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
    إذا (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        abs(RoundingError).is_Exactly_قيمة(0.5))
      Rounded.add(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rmNearestTiesToEven);
    إرجاع Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  إذا (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    إذا (RM == rmNearestTiesToAway || RM == rmNearestTiesToEven)
      RoundedHi = RoundToNearestHelper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/خطأ);
    إرجاع HiStatus;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  RoundingMode LoRoundingMode;
  إذا (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    LoRoundingMode = isNegative() ? rmTowardPositive : rmTowardNegative;
  وإلا
    LoRoundingMode = RM;

  APFloat RoundedLo = Lo;
  ثابت opStatus LoStatus = RoundedLo.roundToIntegral(LoRoundingMode);
  إذا (LoRoundingMode == rmNearestTiesToAway)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = RoundToNearestHelper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fastTwoSum(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  إرجاع LoStatus;
}

فراغ DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmpResult
DoubleAPFloat::compare_Absolute_قيمة(ثابت DoubleAPFloat &RHS) ثابت {
  // Compare absolute values of the high parts.
  ثابت cmpResult HiPartCmp = Floats[0].compare_Absolute_قيمة(RHS.Floats[0]);
  إذا (HiPartCmp != cmpEqual)
    إرجاع HiPartCmp;

  // Zero, regardless of sign, is equal.
  إذا (Floats[1].isZero() && RHS.Floats[1].isZero())
    إرجاع cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  ثابت منطقي ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  ثابت منطقي RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  إذا (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    إرجاع RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  إذا (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    إرجاع ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  إذا (ThisIsSubtractive != RHSIsSubtractive)
    إرجاع ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  ثابت cmpResult LoPartCmp = Floats[1].compare_Absolute_قيمة(RHS.Floats[1]);

  إذا (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    إذا (LoPartCmp == cmpLessThan)
      إرجاع cmpGreaterThan;
    إذا (LoPartCmp == cmpGreaterThan)
      إرجاع cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  إرجاع LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() ثابت {
  إرجاع Floats[0].getCategory();
}

منطقي DoubleAPFloat::isNegative() ثابت { إرجاع Floats[0].isNegative(); }

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
  إذا (Neg)
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
  إذا (Neg)
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
  إذا (Result == APFloat::cmpEqual)
    إرجاع Floats[1].compare(RHS.Floats[1]);
  إرجاع Result;
}

منطقي DoubleAPFloat::bitwiseIsEqual(ثابت DoubleAPFloat &RHS) ثابت {
  إرجاع Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_code hash_قيمة(ثابت DoubleAPFloat &Arg) {
  إذا (Arg.Floats)
    إرجاع hash_combine(hash_قيمة(Arg.Floats[0]), hash_قيمة(Arg.Floats[1]));
  إرجاع hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcastToAPInt() ثابت {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t Data[] = {
      Floats[0].bitcastToAPInt().getRawData()[0],
      Floats[1].bitcastToAPInt().getRawData()[0],
  };
  إرجاع APInt(128, Data);
}

Expected<APFloat::opStatus> DoubleAPFloat::convertFromString(StringRef S,
                                                             roundingMode RM) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy);
  auto Ret = Tmp.convertFromString(S, RM);
  *هذا = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  إرجاع Ret;
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
  إذا (nextDown) {
    changeSign();
    APFloat::opStatus Result = next(/*nextDown=*/خطأ);
    changeSign();
    إرجاع Result;
  }
  بدل (getCategory()) {
  حالة fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    إذا (isNegative())
      makeLargest(true);
    إرجاع opOK;

  حالة fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    إذا (getFirst().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(خطأ, isNegative(), nullptr);
      إرجاع opInvalidOp;
    }
    إرجاع opOK;

  حالة fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(خطأ);
    إرجاع opOK;

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
    إرجاع Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  إذا (InLattice(HiOld, NextLo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(NextLo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat Largest{*Semantics, uninitialized};
    Largest.makeLargest(/*Neg=*/خطأ);
    إذا (compare(Largest) == cmpGreaterThan)
      makeInf(/*Neg=*/خطأ);

    إرجاع opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat NextHi = HiOld;
  NextHi.next(/*nextDown=*/خطأ);

  // nextUp(getLargest()) == INFINITY
  إذا (NextHi.isInfinity()) {
    makeInf(/*Neg=*/خطأ);
    إرجاع opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  إذا (NextHi.isZero()) {
    makeZero(/*Neg=*/true);
    إرجاع opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  NextLo = neg(scalbn(harrisonUlp(NextHi), -1, rmTowardZero));
  إذا (!InLattice(NextHi, NextLo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    NextLo.next(/*nextDown=*/خطأ);

  Floats[0] = std::move(NextHi);
  Floats[1] = std::move(NextLo);

  إرجاع opOK;
}

APFloat::opStatus DoubleAPFloat::convertToSignExtendedInteger(
    MutableArrayRef<integerPart> إدخال, unsigned صحيح Width, منطقي IsSigned,
    roundingMode RM, منطقي *IsExact) ثابت {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  إذا (!getFirst().isFiniteNonZero() || getSecond().isZero())
    إرجاع getFirst().convertToInteger(إدخال, Width, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = خطأ;
  DoubleAPFloat Integral = *هذا;
  ثابت opStatus RoundStatus = Integral.roundToIntegral(RM);
  إذا (RoundStatus == opInvalidOp)
    إرجاع opInvalidOp;
  ثابت APFloat &IntegralHi = Integral.getFirst();
  ثابت APFloat &IntegralLo = Integral.getSecond();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  منطقي HiIsExact;
  إذا (IntegralHi.isZero() || IntegralLo.isZero()) {
    ثابت opStatus HiStatus =
        IntegralHi.convertToInteger(إدخال, Width, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    إرجاع HiStatus == opOK ? opInexact : HiStatus;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  إذا (!IsSigned && IntegralHi.isNegative())
    إرجاع opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  منطقي LoIsExact;
  ثابت صحيح HiExactLog2 = IntegralHi.getExactLog2Abs();
  ثابت unsigned PositiveOverflowWidth = IsSigned ? Width - 1 : Width;
  إذا (HiExactLog2 >= 0 &&
      static_cast<unsigned>(HiExactLog2) == PositiveOverflowWidth) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    إذا (IntegralHi.isNegative() == IntegralLo.isNegative())
      إرجاع opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    ثابت opStatus LoStatus = IntegralLo.convertToInteger(
        إدخال, Width, /*IsSigned=*/true, RM, &LoIsExact);
    إذا (LoStatus == opInvalidOp)
      إرجاع opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    إذا (IsSigned && !IntegralHi.isNegative())
      APInt::tcClearBit(إدخال.data(), PositiveOverflowWidth);
    *IsExact = RoundStatus == opOK;
    إرجاع RoundStatus;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  ثابت opStatus HiStatus = IntegralHi.convertToInteger(
      إدخال, Width, IsSigned, rmTowardZero, &HiIsExact);
  إذا (HiStatus == opInvalidOp)
    إرجاع HiStatus;

  // Convert Lo into a temporary integer of the same width.
  APSInt LoResult{Width, /*isUnsigned=*/!IsSigned};
  ثابت opStatus LoStatus =
      IntegralLo.convertToInteger(LoResult, rmTowardZero, &LoIsExact);
  إذا (LoStatus == opInvalidOp)
    إرجاع LoStatus;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tcAdd(إدخال.data(), LoResult.getRawData(), /*carry=*/0, إدخال.size());

  *IsExact = RoundStatus == opOK;
  إرجاع RoundStatus;
}

APFloat::opStatus
DoubleAPFloat::convertToInteger(MutableArrayRef<integerPart> إدخال,
                                unsigned صحيح Width, منطقي IsSigned,
                                roundingMode RM, منطقي *IsExact) ثابت {
  opStatus FS =
      convertToSignExtendedInteger(إدخال, Width, IsSigned, RM, IsExact);

  إذا (FS == opInvalidOp) {
    ثابت unsigned Dst_Parts_تعداد = part_تعداد_For_Bits(Width);
    أكد(Dst_Parts_تعداد <= إدخال.size() && "Integer too big");

    unsigned Bits;
    إذا (getCategory() == fcNaN)
      Bits = 0;
    وإلا إذا (isNegative())
      Bits = IsSigned;
    وإلا
      Bits = Width - IsSigned;

    tcSetLeastSignificantBits(إدخال.data(), Dst_Parts_تعداد, Bits);
    إذا (isNegative() && IsSigned)
      APInt::tc_Shift_يسار(إدخال.data(), Dst_Parts_تعداد, Width - 1);
  }

  إرجاع FS;
}

APFloat::opStatus DoubleAPFloat::handleOverflow(roundingMode RM) {
  بدل (RM) {
  حالة APFloat::rmTowardZero:
    makeLargest(/*Neg=*/isNegative());
    اكسر;
  حالة APFloat::rmTowardNegative:
    إذا (isNegative())
      makeInf(/*Neg=*/true);
    وإلا
      makeLargest(/*Neg=*/خطأ);
    اكسر;
  حالة APFloat::rmTowardPositive:
    إذا (isNegative())
      makeLargest(/*Neg=*/true);
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
  إذا (!getFirst().isFinite())
    S = static_cast<opStatus>(S | opOverflow);
  إرجاع S;
}

APFloat::opStatus DoubleAPFloat::convertFromUnsignedParts(
    ثابت integerPart *Src, unsigned صحيح Src_تعداد, roundingMode RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  ثابت unsigned SrcMSB = APInt::tcMSB(Src, Src_تعداد);
  إذا (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/خطأ);
    إرجاع opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  ثابت unsigned SrcBitWidth = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/SrcBitWidth, ArrayRef(Src, Src_تعداد)},
                /*isUnsigned=*/true};

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
  إذا (!Hi.isFinite())
    إرجاع handleOverflow(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  منطقي HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/true};
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
  إذا (Error.isNegative()) {
    إذا (RM == rmNearestTiesToAway) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      ثابت unsigned ErrorActiveBits = Error.getSignificantBits() - 1;
      ثابت unsigned LoPrecision = getSecond().getSemantics().precision;
      إذا (ErrorActiveBits > LoPrecision) {
        ثابت unsigned RoundingBoundary = ErrorActiveBits - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        إذا (Error.تعداد_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rmTowardPositive;
      }
    } وإلا إذا (RM == rmTowardZero) {
      // For rmTowardZero, the final positive result must be truncated (rounded
      // down). When Hi is an overestimate, Error is negative. A standard
      // rmTowardZero conversion of Error would make it *less* negative,
      // effectively rounding the final sum Hi + Lo *up*. To ensure the sum
      // rounds down correctly, we force Lo to round toward -infinity.
      LoRM = rmTowardNegative;
    }
  }

  APFloat Lo{getSecond().getSemantics()};
  opStatus Status = Lo.convertFromAPInt(Error, /*IsSigned=*/true, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fastTwoSum(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  إذا (!getFirst().isFinite())
    إرجاع handleOverflow(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  إذا (getFirst().isFiniteNonZero() && Floats[0].isLargest()) {
    DoubleAPFloat Largest{*Semantics};
    Largest.makeLargest(/*Neg=*/خطأ);
    إذا (compare(Largest) == APFloat::cmpGreaterThan)
      إرجاع handleOverflow(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  إرجاع Status;
}

APFloat::opStatus DoubleAPFloat::convertFromAPInt(ثابت APInt &إدخال,
                                                  منطقي IsSigned,
                                                  roundingMode RM) {
  ثابت منطقي Negate_إدخال = IsSigned && إدخال.isNegative();
  APInt API = إدخال;
  إذا (Negate_إدخال)
    API.negate();

  ثابت APFloat::opStatus Status =
      convertFromUnsignedParts(API.getRawData(), API.get_رقم_كلمات(), RM);
  إذا (Negate_إدخال)
    changeSign();
  إرجاع Status;
}

unsigned صحيح DoubleAPFloat::convertToHexString(حرف *DST,
                                               unsigned صحيح HexDigits,
                                               منطقي UpperCase,
                                               roundingMode RM) ثابت {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  إرجاع APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .convertToHexString(DST, HexDigits, UpperCase, RM);
}

منطقي DoubleAPFloat::isDenormal() ثابت {
  إرجاع getCategory() == fcNormal &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

منطقي DoubleAPFloat::isSmallest() ثابت {
  إذا (getCategory() != fcNormal)
    إرجاع خطأ;
  DoubleAPFloat Tmp(*هذا);
  Tmp.makeSmallest(هذا->isNegative());
  إرجاع Tmp.compare(*هذا) == cmpEqual;
}

منطقي DoubleAPFloat::isSmallestNormalized() ثابت {
  إذا (getCategory() != fcNormal)
    إرجاع خطأ;

  DoubleAPFloat Tmp(*هذا);
  Tmp.makeSmallestNormalized(هذا->isNegative());
  إرجاع Tmp.compare(*هذا) == cmpEqual;
}

منطقي DoubleAPFloat::isLargest() ثابت {
  إذا (getCategory() != fcNormal)
    إرجاع خطأ;
  DoubleAPFloat Tmp(*هذا);
  Tmp.makeLargest(هذا->isNegative());
  إرجاع Tmp.compare(*هذا) == cmpEqual;
}

منطقي DoubleAPFloat::isInteger() ثابت {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  إرجاع Floats[0].isInteger() && Floats[1].isInteger();
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
  إذا (getSecond().isNonZero())
    إرجاع INT_MIN;
  إرجاع getFirst().getExactLog2Abs();
}

صحيح ilogb(ثابت DoubleAPFloat &Arg) {
  ثابت APFloat &Hi = Arg.getFirst();
  ثابت APFloat &Lo = Arg.getSecond();
  صحيح IlogbResult = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  إذا (Arg.getCategory() != fcNormal)
    إرجاع IlogbResult;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  إذا (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    إرجاع IlogbResult;
  إذا (Hi.getExactLog2Abs() == INT_MIN)
    إرجاع IlogbResult;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  إرجاع IlogbResult - 1;
}

DoubleAPFloat scalbn(ثابت DoubleAPFloat &Arg, صحيح Exp,
                     APFloat::roundingMode RM) {
  أكد(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");
  إرجاع DoubleAPFloat(APFloatBase::PPCDoubleDouble(),
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
  إذا (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.getFirst() = Quiet.getFirst().makeQuiet();
    إرجاع Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  إذا (Exp == APFloat::IEK_Inf)
    إرجاع Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  إذا (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    إرجاع Arg;
  }

  ثابت APFloat &Hi = Arg.getFirst();
  ثابت APFloat &Lo = Arg.getSecond();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  ثابت منطقي SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat Second = Lo;
  إذا (Arg.getCategory() == APFloat::fcNormal && Lo.isFiniteNonZero()) {
    roundingMode LoRoundingMode;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    إذا (RM == rmTowardZero)
      LoRoundingMode = Arg.isNegative() ? rmTowardPositive : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    وإلا إذا (RM == rmNearestTiesToAway && SignsDisagree && Exp > 0)
      LoRoundingMode = rmNearestTiesToEven;
    وإلا
      LoRoundingMode = RM;
    Second = scalbn(Lo, -Exp, LoRoundingMode);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    إذا (RM == rmNearestTiesToAway && LoRoundingMode == rmNearestTiesToEven) {
      // Re-scale the result back to check if rounding occurred.
      ثابت APFloat RecomposedLo = scalbn(Second, Exp, rmNearestTiesToEven);
      إذا (RecomposedLo != Lo) {
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
        إذا (IsMidpoint && RoundedLoAway)
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
    إذا (Second.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat First = scalbn(Hi, -Exp, RM);
  إرجاع DoubleAPFloat(APFloatBase::PPCDoubleDouble(), std::move(First),
                       std::move(Second));
}

APInt DoubleAPFloat::getNaNPayload() ثابت { إرجاع Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, ثابت fltSemantics &Semantics) {
  إذا (usesLayout<IEEEFloat>(Semantics)) {
    جديد (&IEEE) IEEEFloat(std::move(F));
    إرجاع;
  }
  إذا (usesLayout<DoubleAPFloat>(Semantics)) {
    ثابت fltSemantics& S = F.getSemantics();
    جديد (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloatBase::IEEEdouble()));
    إرجاع;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::opStatus> APFloat::convertFromString(StringRef Str,
                                                       roundingMode RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_code hash_قيمة(ثابت APFloat &Arg) {
  إذا (APFloat::usesLayout<detail::IEEEFloat>(Arg.getSemantics()))
    إرجاع hash_قيمة(Arg.U.IEEE);
  إذا (APFloat::usesLayout<detail::DoubleAPFloat>(Arg.getSemantics()))
    إرجاع hash_قيمة(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(ثابت fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  auto StatusOrErr = convertFromString(S, rmNearestTiesToEven);
  أكد(StatusOrErr && "Invalid floating point representation");
  consumeError(StatusOrErr.takeError());
}

FPClassTest APFloat::classify() ثابت {
  إذا (isZero())
    إرجاع isNegative() ? fcNegZero : fcPosZero;
  إذا (isNormal())
    إرجاع isNegative() ? fcNegNormal : fcPosNormal;
  إذا (isDenormal())
    إرجاع isNegative() ? fcNegSubnormal : fcPosSubnormal;
  إذا (isInfinity())
    إرجاع isNegative() ? fcNegInf : fcPosInf;
  أكد(isNaN() && "Other class of FP constant");
  إرجاع isSignaling() ? fcSNan : fcQNan;
}

منطقي APFloat::getExactInverse(APFloat *Inv) ثابت {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  إذا (!isFiniteNonZero())
    إرجاع خطأ;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  إذا (isDenormal())
    إرجاع خطأ;

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
  إذا (Exp == INT_MIN)
    إرجاع خطأ;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  إذا (Reciprocal.getExactLog2Abs() != -Exp)
    إرجاع خطأ;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  إذا (Reciprocal.isDenormal())
    إرجاع خطأ;

  أكد(Reciprocal.isFiniteNonZero());

  إذا (Inv)
    *Inv = std::move(Reciprocal);

  إرجاع true;
}

APFloat::opStatus APFloat::convert(ثابت fltSemantics &ToSemantics,
                                   roundingMode RM, منطقي *losesInfo) {
  إذا (&getSemantics() == &ToSemantics) {
    *losesInfo = خطأ;
    إرجاع opOK;
  }
  إذا (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics))
    إرجاع U.IEEE.convert(ToSemantics, RM, losesInfo);
  إذا (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<DoubleAPFloat>(ToSemantics)) {
    أكد(&ToSemantics == &APFloatBase::semPPCDoubleDouble);
    auto Ret =
        U.IEEE.convert(APFloatBase::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *هذا = APFloat(ToSemantics, U.IEEE.bitcastToAPInt());
    إرجاع Ret;
  }
  إذا (usesLayout<DoubleAPFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics)) {
    auto Ret = getIEEE().convert(ToSemantics, RM, losesInfo);
    *هذا = APFloat(std::move(getIEEE()), ToSemantics);
    إرجاع Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_قيمة(ثابت fltSemantics &Semantics) {
  إرجاع APFloat(Semantics, APInt::getAllOnes(Semantics.sizeInBits));
}

فراغ APFloat::print(raw_ostream &OS) ثابت {
  SmallVector<حرف, 16> Buffer;
  toString(Buffer);
  OS << Buffer;
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD فراغ APFloat::dump() ثابت {
  print(dbgs());
  dbgs() << '\n';
}
#endif

فراغ APFloat::Profile(Folding_Set_عقدة_ID &NID) ثابت {
  NID.Add(bitcastToAPInt());
}

APFloat::opStatus APFloat::convertToInteger(APSInt &result,
                                            roundingMode rounding_mode,
                                            منطقي *isExact) ثابت {
  unsigned bitWidth = result.getBitWidth();
  SmallVector<uint64_t, 4> parts(result.get_رقم_كلمات());
  opStatus status = convertToInteger(parts, bitWidth, result.isSigned(),
                                     rounding_mode, isExact);
  // Keeps the original signed-ness.
  result = APInt(bitWidth, parts);
  إرجاع status;
}

مزدوج APFloat::convertToDouble() ثابت {
  إذا (&getSemantics() == &APFloatBase::semIEEEdouble)
    إرجاع getIEEE().convertToDouble();
  أكد(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat Temp = *هذا;
  منطقي LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  إرجاع Temp.getIEEE().convertToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 APFloat::convertToQuad() ثابت {
  إذا (&getSemantics() == &APFloatBase::semIEEEquad)
    إرجاع getIEEE().convertToQuad();
  أكد(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat Temp = *هذا;
  منطقي LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEquad, rmNearestTiesToEven, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  إرجاع Temp.getIEEE().convertToQuad();
}
#endif

عائم APFloat::convertToFloat() ثابت {
  إذا (&getSemantics() == &APFloatBase::semIEEEsingle)
    إرجاع getIEEE().convertToFloat();
  أكد(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat Temp = *هذا;
  منطقي LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEsingle, rmNearestTiesToEven, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  إرجاع Temp.getIEEE().convertToFloat();
}

unsigned APFloatBase::getArbitraryFPFormatSizeInBits(StringRef Format) {
  إرجاع StringSwitch<unsigned>(Format)
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
  إرجاع getArbitraryFPFormatSizeInBits(Format) != 0;
}

ثابت fltSemantics *APFloatBase::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  إرجاع StringSwitch<ثابت fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(nullptr);
}

APFloat::Storage::~Storage() {
  إذا (usesLayout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    إرجاع;
  }
  إذا (usesLayout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    إرجاع;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(ثابت APFloat::Storage &RHS) {
  إذا (usesLayout<IEEEFloat>(*RHS.semantics)) {
    جديد (هذا) IEEEFloat(RHS.IEEE);
    إرجاع;
  }
  إذا (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    جديد (هذا) DoubleAPFloat(RHS.Double);
    إرجاع;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  إذا (usesLayout<IEEEFloat>(*RHS.semantics)) {
    جديد (هذا) IEEEFloat(std::move(RHS.IEEE));
    إرجاع;
  }
  إذا (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    جديد (هذا) DoubleAPFloat(std::move(RHS.Double));
    إرجاع;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::operator=(ثابت APFloat::Storage &RHS) {
  إذا (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } وإلا إذا (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } وإلا إذا (هذا != &RHS) {
    هذا->~Storage();
    جديد (هذا) Storage(RHS);
  }
  إرجاع *هذا;
}

APFloat::Storage &APFloat::Storage::operator=(APFloat::Storage &&RHS) {
  إذا (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } وإلا إذا (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } وإلا إذا (هذا != &RHS) {
    هذا->~Storage();
    جديد (هذا) Storage(std::move(RHS));
  }
  إرجاع *هذا;
}

نطاق {

APFloat::opStatus getOpStatusFromLibc(صحيح libc_exceptions) {
  APFloat::opStatus status = APFloat::opOK;
  إذا (libc_exceptions & FE_INVALID)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInvalidOp);
  إذا (libc_exceptions & FE_DIVBYZERO)
    status = static_cast<APFloat::opStatus>(status | APFloat::opDivByZero);
  إذا (libc_exceptions & FE_OVERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opOverflow);
  إذا (libc_exceptions & FE_UNDERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opUnderflow);
  إذا (libc_exceptions & FE_INEXACT)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInexact);
  إرجاع status;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> exp(ثابت APFloat &x, RoundingMode rounding_mode,
                           APFloat::opStatus *status) {

  إذا (rounding_mode == APFloatBase::rmNearestTiesToEven) {
    إذا (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEsingle) {
      عائم x_val = x.convertToFloat();
      صحيح exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      إذا (status) {
        *status = getOpStatusFromLibc(exc);
        إذا (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      عائم result = LIBC_NAMESPACE::shared::expf(x_val);
      إرجاع APFloat(result);
    }
    إذا (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEdouble) {
      مزدوج x_val = x.convertToDouble();
      صحيح exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      إذا (status) {
        *status = getOpStatusFromLibc(exc);
        إذا (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      مزدوج result = LIBC_NAMESPACE::shared::exp(x_val);
      إرجاع APFloat(result);
    }
  }
  إرجاع std::nullopt;
}

} // namespace llvm

#undef APFLOAT_DISPATCH_ON_SEMANTICS
