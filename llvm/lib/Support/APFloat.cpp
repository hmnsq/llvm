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

#شامل "llvm/ADT/APFloat.h"
#شامل "llvm/ADT/APSInt.h"
#شامل "llvm/ADT/ArrayRef.h"
#شامل "llvm/ADT/FloatingPointMode.h"
#شامل "llvm/ADT/FoldingSet.h"
#شامل "llvm/ADT/Hashing.h"
#شامل "llvm/ADT/STLExtras.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/ADT/StringSwitch.h"
#شامل "llvm/Config/llvm-config.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/Error.h"
#شامل "llvm/Support/MathExtras.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <cstring>
#شامل <iterator>
#شامل <limits.h>

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

#شامل "shared/math.h"
#شامل "shared/math_check_exceptions.h"

#define APFLOAT_DISPATCH_ON_SEMANTICS(METHOD_CALL)                             \
  انجامبده {                                                                         \
    اگر (usesLayout<IEEEFloat>(getSemantics()))                                 \
      بازگشت U.IEEE.METHOD_CALL;                                               \
    اگر (usesLayout<DoubleAPFloat>(getSemantics()))                             \
      بازگشت U.Double.METHOD_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } وقتی (نادرست)

using فضاینام llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#define Pack_Categories_Into_کلید(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloatBase::integerPartWidth % 4 == 0, "Part width must be divisible by 4!");

فضاینام llvm {

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
    درست,
    درست,
    درست,
    درست,
    درست};
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
    نادرست,
    نادرست,
    نادرست,
    نادرست};

constexpr fltSemantics APFloatBase::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    درست,
    نادرست,
    نادرست};

constexpr fltSemantics APFloatBase::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semBogus = {0, 0, 0, 0};

ثابت llvm::fltSemantics &APFloatBase::EnumToSemantics(Semantics S) {
  انتخاب (S) {
  حالت S_IEEEhalf:
    بازگشت IEEEhalf();
  حالت S_BFloat:
    بازگشت BFloat();
  حالت S_IEEEsingle:
    بازگشت IEEEsingle();
  حالت S_IEEEdouble:
    بازگشت IEEEdouble();
  حالت S_IEEEquad:
    بازگشت IEEEquad();
  حالت S_x87DoubleExtended:
    بازگشت x87DoubleExtended();
  حالت S_PPCDoubleDouble:
    بازگشت PPCDoubleDouble();
  حالت S_PPCDoubleDoubleLegacy:
    بازگشت PPCDoubleDoubleLegacy();
  حالت S_Float8E5M2:
    بازگشت Float8E5M2();
  حالت S_Float8E5M2FNUZ:
    بازگشت Float8E5M2FNUZ();
  حالت S_Float8E4M3:
    بازگشت Float8E4M3();
  حالت S_Float8E4M3FN:
    بازگشت Float8E4M3FN();
  حالت S_Float8E4M3FNUZ:
    بازگشت Float8E4M3FNUZ();
  حالت S_Float8E4M3B11FNUZ:
    بازگشت Float8E4M3B11FNUZ();
  حالت S_Float8E3M4:
    بازگشت Float8E3M4();
  حالت S_FloatTF32:
    بازگشت FloatTF32();
  حالت S_Float8E8M0FNU:
    بازگشت Float8E8M0FNU();
  حالت S_Float8E5M3FNU:
    بازگشت Float8E5M3FNU();
  حالت S_Float6E3M2FN:
    بازگشت Float6E3M2FN();
  حالت S_Float6E2M3FN:
    بازگشت Float6E2M3FN();
  حالت S_Float4E2M1FN:
    بازگشت Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloatBase::Semantics
APFloatBase::SemanticsToEnum(ثابت llvm::fltSemantics &Sem) {
  اگر (&Sem == &llvm::APFloat::IEEEhalf())
    بازگشت S_IEEEhalf;
  وگرنه اگر (&Sem == &llvm::APFloat::BFloat())
    بازگشت S_BFloat;
  وگرنه اگر (&Sem == &llvm::APFloat::IEEEsingle())
    بازگشت S_IEEEsingle;
  وگرنه اگر (&Sem == &llvm::APFloat::IEEEdouble())
    بازگشت S_IEEEdouble;
  وگرنه اگر (&Sem == &llvm::APFloat::IEEEquad())
    بازگشت S_IEEEquad;
  وگرنه اگر (&Sem == &llvm::APFloat::x87DoubleExtended())
    بازگشت S_x87DoubleExtended;
  وگرنه اگر (&Sem == &llvm::APFloat::PPCDoubleDouble())
    بازگشت S_PPCDoubleDouble;
  وگرنه اگر (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    بازگشت S_PPCDoubleDoubleLegacy;
  وگرنه اگر (&Sem == &llvm::APFloat::Float8E5M2())
    بازگشت S_Float8E5M2;
  وگرنه اگر (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    بازگشت S_Float8E5M2FNUZ;
  وگرنه اگر (&Sem == &llvm::APFloat::Float8E4M3())
    بازگشت S_Float8E4M3;
  وگرنه اگر (&Sem == &llvm::APFloat::Float8E4M3FN())
    بازگشت S_Float8E4M3FN;
  وگرنه اگر (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    بازگشت S_Float8E4M3FNUZ;
  وگرنه اگر (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    بازگشت S_Float8E4M3B11FNUZ;
  وگرنه اگر (&Sem == &llvm::APFloat::Float8E3M4())
    بازگشت S_Float8E3M4;
  وگرنه اگر (&Sem == &llvm::APFloat::FloatTF32())
    بازگشت S_FloatTF32;
  وگرنه اگر (&Sem == &llvm::APFloat::Float8E8M0FNU())
    بازگشت S_Float8E8M0FNU;
  وگرنه اگر (&Sem == &llvm::APFloat::Float8E5M3FNU())
    بازگشت S_Float8E5M3FNU;
  وگرنه اگر (&Sem == &llvm::APFloat::Float6E3M2FN())
    بازگشت S_Float6E3M2FN;
  وگرنه اگر (&Sem == &llvm::APFloat::Float6E2M3FN())
    بازگشت S_Float6E2M3FN;
  وگرنه اگر (&Sem == &llvm::APFloat::Float4E2M1FN())
    بازگشت S_Float4E2M1FN;
  وگرنه
    llvm_unreachable("Unknown floating semantics");
}

منطقی APFloatBase::isRepresentableBy(ثابت fltSemantics &A,
                                    ثابت fltSemantics &B) {
  بازگشت A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

منطقی APFloatBase::isLosslesslyConvertibleTo(ثابت fltSemantics &From,
                                            ثابت fltSemantics &InitialTo,
                                            منطقی IgnoreNaNs) {
  اگر (&From == &InitialTo)
    بازگشت درست;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  اگر (&From == &semPPCDoubleDouble)
    بازگشت نادرست;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  ثابت fltSemantics &To =
      &InitialTo == &semPPCDoubleDouble ? semIEEEdouble : InitialTo;

  اگر (!isRepresentableBy(From, To))
    بازگشت نادرست;

  اگر ((From.hasZero && !To.hasZero) ||
      (From.hasSignedRepr && !To.hasSignedRepr))
    بازگشت نادرست;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  منطقی FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  منطقی ToHasSignedZero = To.hasZero && To.hasSignedRepr &&
                         To.nanEncoding != fltNanEncoding::NegativeZero;
  اگر (FromHasSignedZero && !ToHasSignedZero)
    بازگشت نادرست;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat SmallestFrom = APFloat::getSmallest(From);
  منطقی LosesInfo = نادرست;
  (تهی)SmallestFrom.convert(To, APFloat::rmNearestTiesToEven, &LosesInfo);
  اگر (LosesInfo)
    بازگشت نادرست;

  اگر (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    بازگشت درست;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  اگر (IgnoreNaNs) {
    اگر (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      بازگشت To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    بازگشت To.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  اگر (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    بازگشت نادرست;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  اگر (To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    بازگشت درست;
  بازگشت To.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
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
ایستا constexpr unsigned رقم_Pow5_Parts_On_پشته = 16;

ایستا unsigned صحیح powerOfFivePartsForExponent(unsigned صحیح power) {
  أكد(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  بازگشت 2 + ((power * 815) / (351 * APFloatBase::integerPartWidth));
}

unsigned صحیح APFloatBase::semanticsPrecision(ثابت fltSemantics &semantics) {
  بازگشت semantics.precision;
}
APFloatBase::ExponentType
APFloatBase::semanticsMaxExponent(ثابت fltSemantics &semantics) {
  بازگشت semantics.maxExponent;
}
APFloatBase::ExponentType
APFloatBase::semanticsMinExponent(ثابت fltSemantics &semantics) {
  بازگشت semantics.minExponent;
}
unsigned صحیح APFloatBase::semanticsSizeInBits(ثابت fltSemantics &semantics) {
  بازگشت semantics.sizeInBits;
}
unsigned صحیح APFloatBase::semanticsIntSizeInBits(ثابت fltSemantics &semantics,
                                                 منطقی isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  unsigned صحیح MinBitWidth = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  اگر (isSigned)
    ++MinBitWidth;
  بازگشت MinBitWidth;
}

منطقی APFloatBase::semanticsHasZero(ثابت fltSemantics &semantics) {
  بازگشت semantics.hasZero;
}

منطقی APFloatBase::semanticsHasSignedRepr(ثابت fltSemantics &semantics) {
  بازگشت semantics.hasSignedRepr;
}

منطقی APFloatBase::semanticsHasInf(ثابت fltSemantics &semantics) {
  بازگشت semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

منطقی APFloatBase::semanticsHasNaN(ثابت fltSemantics &semantics) {
  بازگشت semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

منطقی APFloatBase::isIEEELikeFP(ثابت fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  بازگشت SemanticsToEnum(semantics) <= S_IEEEquad;
}

منطقی APFloatBase::hasSignBitInMSB(ثابت fltSemantics &semantics) {
  بازگشت semantics.hasSignBitInMSB;
}

منطقی APFloatBase::isRepresentableAsNormalIn(ثابت fltSemantics &Src,
                                            ثابت fltSemantics &Dst) {
  // Exponent range must be larger.
  اگر (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    بازگشت نادرست;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  بازگشت Dst.precision >= Src.precision;
}

unsigned APFloatBase::getSizeInBits(ثابت fltSemantics &Sem) {
  بازگشت Sem.sizeInBits;
}

ایستا constexpr APFloatBase::ExponentType
exponentZero(ثابت fltSemantics &semantics) {
  بازگشت semantics.minExponent - 1;
}

ایستا constexpr APFloatBase::ExponentType
exponentInf(ثابت fltSemantics &semantics) {
  بازگشت semantics.maxExponent + 1;
}

ایستا constexpr APFloatBase::ExponentType
exponentNaN(ثابت fltSemantics &semantics) {
  اگر (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    اگر (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      بازگشت exponentZero(semantics);
    اگر (semantics.hasSignedRepr || semantics.precision > 1)
      بازگشت semantics.maxExponent;
  }
  بازگشت semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

ایستا درونخطی Error createError(ثابت Twine &Err) {
  بازگشت make_error<StringError>(Err, inconvertibleErrorCode());
}

ایستا constexpr درونخطی unsigned صحیح part_شمار_For_Bits(unsigned صحیح bits) {
  بازگشت std::max(1u, (bits + APFloatBase::integerPartWidth - 1) /
                          APFloatBase::integerPartWidth);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
ایستا درونخطی unsigned صحیح
dec_Digit_مقدار(unsigned صحیح c)
{
  بازگشت c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
ایستا Expected<صحیح> readExponent(StringRef::iterator begin,
                                  StringRef::iterator end) {
  ثابت unsigned صحیح overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  اگر (p == end || ((*p == '-' || *p == '+') && (p + 1) == end))
    بازگشت 0;

  منطقی isNegative = *p == '-';
  اگر (*p == '-' || *p == '+') {
    p++;
    اگر (p == end)
      بازگشت createError("Exponent has no digits");
  }

  unsigned absExponent = dec_Digit_مقدار(*p++);
  اگر (absExponent >= 10U)
    بازگشت createError("Invalid character in exponent");

  برای (; p != end; ++p) {
    unsigned مقدار_2 = dec_Digit_مقدار(*p);
    اگر (مقدار_2 >= 10U)
      بازگشت createError("Invalid character in exponent");

    absExponent = absExponent * 10U + مقدار_2;
    اگر (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      بشکن;
    }
  }

  اگر (isNegative)
    بازگشت -(صحیح) absExponent;
  وگرنه
    بازگشت (صحیح) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
ایستا Expected<صحیح> totalExponent(StringRef::iterator p,
                                   StringRef::iterator end,
                                   صحیح exponentAdjustment) {
  صحیح exponent = 0;

  اگر (p == end)
    بازگشت createError("Exponent has no digits");

  منطقی negative = *p == '-';
  اگر (*p == '-' || *p == '+') {
    p++;
    اگر (p == end)
      بازگشت createError("Exponent has no digits");
  }

  صحیح unsignedExponent = 0;
  منطقی overflow = نادرست;
  برای (; p != end; ++p) {
    unsigned صحیح مقدار_2;

    مقدار_2 = dec_Digit_مقدار(*p);
    اگر (مقدار_2 >= 10U)
      بازگشت createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + مقدار_2;
    اگر (unsignedExponent > 32767) {
      overflow = درست;
      بشکن;
    }
  }

  اگر (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    overflow = درست;

  اگر (!overflow) {
    exponent = unsignedExponent;
    اگر (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    اگر (exponent > 32767 || exponent < -32768)
      overflow = درست;
  }

  اگر (overflow)
    exponent = negative ? -32768: 32767;

  بازگشت exponent;
}

ایستا Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator end,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = end;
  وقتی (p != end && *p == '0')
    p++;

  اگر (p != end && *p == '.') {
    *dot = p++;

    اگر (end - begin == 1)
      بازگشت createError("Significand has no digits");

    وقتی (p != end && *p == '0')
      p++;
  }

  بازگشت p;
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
ساختار decimalInfo {
  ثابت نویسه *firstSigDigit;
  ثابت نویسه *lastSigDigit;
  صحیح exponent;
  صحیح normalizedExponent;
};

ایستا Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator end, decimalInfo *D) {
  StringRef::iterator dot = end;

  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  اگر (!PtrOrErr)
    بازگشت PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->firstSigDigit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  برای (; p != end; ++p) {
    اگر (*p == '.') {
      اگر (dot != end)
        بازگشت createError("String contains multiple dots");
      dot = p++;
      اگر (p == end)
        بشکن;
    }
    اگر (dec_Digit_مقدار(*p) >= 10U)
      بشکن;
  }

  اگر (p != end) {
    اگر (*p != 'e' && *p != 'E')
      بازگشت createError("Invalid character in significand");
    اگر (p == begin)
      بازگشت createError("Significand has no digits");
    اگر (dot != end && p - begin == 1)
      بازگشت createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    auto ExpOrErr = readExponent(p + 1, end);
    اگر (!ExpOrErr)
      بازگشت ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    اگر (dot == end)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  اگر (p != D->firstSigDigit) {
    /* Drop insignificant trailing zeroes.  */
    اگر (p != begin) {
      انجامبده
        انجامبده
          p--;
        وقتی (p != begin && *p == '0');
      وقتی (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->firstSigDigit)
                                      - (dot > D->firstSigDigit && dot < p)));
  }

  D->lastSigDigit = p;
  بازگشت Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
ایستا Expected<lostFraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator end,
                            unsigned صحیح digit_مقدار) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  اگر (digit_مقدار > 8)
    بازگشت lfMoreThanHalf;
  وگرنه اگر (digit_مقدار < 8 && digit_مقدار > 0)
    بازگشت lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  وقتی (p != end && (*p == '0' || *p == '.'))
    p++;

  اگر (p == end)
    بازگشت createError("Invalid trailing hexadecimal fraction!");

  unsigned hexDigit = hex_Digit_مقدار(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  اگر (hexDigit == UINT_MAX)
    بازگشت digit_مقدار == 0 ? lfExactlyZero: lfExactlyHalf;
  وگرنه
    بازگشت digit_مقدار == 0 ? lfLessThanHalf: lfMoreThanHalf;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
ایستا lostFraction
lostFractionThroughTruncation(ثابت APFloatBase::integerPart *parts,
                              unsigned صحیح part_شمار,
                              unsigned صحیح bits)
{
  unsigned lsb = APInt::tcLSB(parts, part_شمار);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  اگر (bits <= lsb)
    بازگشت lfExactlyZero;
  اگر (bits == lsb + 1)
    بازگشت lfExactlyHalf;
  اگر (bits <= part_شمار * APFloatBase::integerPartWidth &&
      APInt::tcExtractBit(parts, bits - 1))
    بازگشت lfMoreThanHalf;

  بازگشت lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
ایستا lostFraction
shift_راست(APFloatBase::integerPart *dst, unsigned صحیح parts, unsigned صحیح bits)
{
  lostFraction lost_fraction = lostFractionThroughTruncation(dst, parts, bits);

  APInt::tc_Shift_راست(dst, parts, bits);

  بازگشت lost_fraction;
}

/* Combine the effect of two lost fractions.  */
ایستا lostFraction
combineLostFractions(lostFraction moreSignificant,
                     lostFraction lessSignificant)
{
  اگر (lessSignificant != lfExactlyZero) {
    اگر (moreSignificant == lfExactlyZero)
      moreSignificant = lfLessThanHalf;
    وگرنه اگر (moreSignificant == lfExactlyHalf)
      moreSignificant = lfMoreThanHalf;
  }

  بازگشت moreSignificant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
ایستا unsigned صحیح
HUerrBound(منطقی inexactMultiply, unsigned صحیح HUerr1, unsigned صحیح HUerr2)
{
  أكد(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  اگر (HUerr1 + HUerr2 == 0)
    بازگشت inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  وگرنه
    بازگشت inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
ایستا APFloatBase::integerPart
ulpsFromBoundary(ثابت APFloatBase::integerPart *parts, unsigned صحیح bits,
                 منطقی isNearest) {
  أكد(bits != 0);

  bits--;
  unsigned شمار = bits / APFloatBase::integerPartWidth;
  unsigned partBits = bits % APFloatBase::integerPartWidth + 1;

  APFloatBase::integerPart part =
      parts[شمار] & (~(APFloatBase::integerPart)0 >>
                      (APFloatBase::integerPartWidth - partBits));

  APFloatBase::integerPart boundary;
  اگر (isNearest)
    boundary = (APFloatBase::integerPart) 1 << (partBits - 1);
  وگرنه
    boundary = 0;

  اگر (شمار == 0) {
    اگر (part - boundary <= boundary - part)
      بازگشت part - boundary;
    وگرنه
      بازگشت boundary - part;
  }

  اگر (part == boundary) {
    وقتی (--شمار)
      اگر (parts[شمار])
        بازگشت ~(APFloatBase::integerPart) 0; /* A lot.  */

    بازگشت parts[0];
  } وگرنه اگر (part == boundary - 1) {
    وقتی (--شمار)
      اگر (~parts[شمار])
        بازگشت ~(APFloatBase::integerPart) 0; /* A lot.  */

    بازگشت -parts[0];
  }

  بازگشت ~(APFloatBase::integerPart) 0; /* A lot.  */
}

ایستا constexpr unsigned PowerOfFivePartCounts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

ایستا constexpr APFloatBase::integerPart PowerOfFiveParts[] = {
#شامل "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  unsigned Sum = 0;
  برای (unsigned شمار_2 : PowerOfFivePartCounts)
    Sum += شمار_2;
  بازگشت Sum;
}() == std::size(PowerOfFiveParts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
ایستا unsigned صحیح
powerOf5(APFloatBase::integerPart *dst, unsigned صحیح power) {
  ایستا ثابت APFloatBase::integerPart firstEightPowers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  SmallVector<APFloatBase::integerPart, رقم_Pow5_Parts_On_پشته> scratch;
  scratch.resize_for_overwrite(powerOfFivePartsForExponent(power));

  APFloatBase::integerPart *p1, *p2;
  ثابت APFloatBase::integerPart *pow5;

  p1 = dst;
  p2 = scratch.data();

  *p1 = firstEightPowers[power & 7];
  power >>= 3;

  unsigned result = 1;
  pow5 = PowerOfFiveParts;

  برای (unsigned صحیح n = 0; power; power >>= 1, n++) {
    unsigned parts_شمار = PowerOfFivePartCounts[n];

    اگر (power & 1) {
      APFloatBase::integerPart *tmp;

      APInt::tcFullMultiply(p2, p1, pow5, result, parts_شمار);
      result += parts_شمار;
      اگر (p2[result - 1] == 0)
        result--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_شمار;
  }

  اگر (p1 != dst)
    APInt::tcAssign(dst, p1, result);

  بازگشت result;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
ایستا ثابت نویسه hexDigitsLower[] = "0123456789abcdef0";
ایستا ثابت نویسه hexDigitsUpper[] = "0123456789ABCDEF0";
ایستا ثابت نویسه infinityL[] = "infinity";
ایستا ثابت نویسه infinityU[] = "INFINITY";
ایستا ثابت نویسه NaNL[] = "nan";
ایستا ثابت نویسه NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
ایستا unsigned صحیح
partAsHex (نویسه *dst, APFloatBase::integerPart part, unsigned صحیح شمار,
           ثابت نویسه *hexDigitChars)
{
  unsigned صحیح result = شمار;

  أكد(شمار != 0 && شمار <= APFloatBase::integerPartWidth / 4);

  part >>= (APFloatBase::integerPartWidth - 4 * شمار);
  وقتی (شمار--) {
    dst[شمار] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  بازگشت result;
}

/* Write out an unsigned decimal integer.  */
ایستا نویسه *writeUnsignedDecimal(نویسه *dst, unsigned صحیح n) {
  نویسه buff[40], *p;

  p = buff;
  انجامبده
    *p++ = '0' + n % 10;
  وقتی (n /= 10);

  انجامبده
    *dst++ = *--p;
  وقتی (p != buff);

  بازگشت dst;
}

/* Write out a signed decimal integer.  */
ایستا نویسه *writeSignedDecimal(نویسه *dst, صحیح مقدار_2) {
  اگر (مقدار_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(unsigned) مقدار_2);
  } وگرنه {
    dst = writeUnsignedDecimal(dst, مقدار_2);
  }

  بازگشت dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
ایستا APFloat harrisonUlp(ثابت APFloat &X) {
  ثابت fltSemantics &Sem = X.getSemantics();
  انتخاب (X.getCategory()) {
  حالت APFloat::fcNaN:
    بازگشت APFloat::getQNaN(Sem);
  حالت APFloat::fcInfinity:
    بازگشت APFloat::getInf(Sem);
  حالت APFloat::fcZero:
    بازگشت APFloat::getSmallest(Sem);
  حالت APFloat::fcNormal:
    بشکن;
  }
  اگر (X.isDenormal() || X.isSmallestNormalized())
    بازگشت APFloat::getSmallest(Sem);
  صحیح Exp = ilogb(X);
  اگر (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  بازگشت scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rmNearestTiesToEven);
}

فضاینام detail {
/* Constructors.  */
تهی IEEEFloat::initialize(ثابت fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  unsigned شمار = part_شمار();
  اگر (شمار > 1)
    significand.parts = جدید integerPart[شمار];
}

تهی IEEEFloat::freeSignificand() {
  اگر (needsCleanup())
    delete [] significand.parts;
}

تهی IEEEFloat::assign(ثابت IEEEFloat &rhs) {
  أكد(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  اگر (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

تهی IEEEFloat::copySignificand(ثابت IEEEFloat &rhs) {
  أكد(isFiniteNonZero() || category == fcNaN);
  أكد(rhs.part_شمار() >= part_شمار());

  APInt::tcAssign(significandParts(), rhs.significandParts(),
                  part_شمار());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
تهی IEEEFloat::makeNaN(منطقی SNaN, منطقی Negative, ثابت APInt *fill) {
  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  اگر (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  unsigned رقم_Parts = part_شمار();

  APInt fill_storage;
  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = نادرست;
    اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = درست;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } وگرنه {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  اگر (!fill || fill->get_رقم_واژگان() < رقم_Parts)
    APInt::tcSet(significand, 0, رقم_Parts);
  اگر (fill) {
    APInt::tcAssign(significand, fill->getRawData(),
                    std::min(fill->get_رقم_واژگان(), رقم_Parts));

    // Zero out the excess bits of the significand.
    unsigned bitsToPreserve = semantics->precision - 1;
    unsigned part = bitsToPreserve / 64;
    bitsToPreserve %= 64;
    significand[part] &= ((1ULL << bitsToPreserve) - 1);
    برای (part++; part != رقم_Parts; ++part)
      significand[part] = 0;
  }

  unsigned QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  اگر (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tcClearBit(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    اگر (APInt::tcIsZero(significand, رقم_Parts))
      APInt::tcSetBit(significand, QNaNBit - 1);
  } وگرنه اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } وگرنه {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tcSetBit(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  اگر (semantics == &APFloatBase::semX87DoubleExtended)
    APInt::tcSetBit(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::operator=(ثابت IEEEFloat &rhs) {
  اگر (این != &rhs) {
    اگر (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    assign(rhs);
  }

  بازگشت *این;
}

IEEEFloat &IEEEFloat::operator=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloatBase::semBogus;
  بازگشت *این;
}

منطقی IEEEFloat::isDenormal() ثابت {
  بازگشت getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tcExtractBit(significandParts(), semantics->precision - 1) ==
          0);
}

منطقی IEEEFloat::isSmallest() ثابت {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  بازگشت isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

منطقی IEEEFloat::isSmallestNormalized() ثابت {
  بازگشت getCategory() == fcNormal && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

unsigned صحیح IEEEFloat::get_رقم_بالا_Bits() ثابت {
  ثابت unsigned صحیح Part_شمار = part_شمار_For_Bits(semantics->precision);
  ثابت unsigned صحیح Bits = Part_شمار * integerPartWidth;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  ثابت unsigned صحیح رقم_بالا_Bits = (semantics->precision > 1)
                                       ? (Bits - semantics->precision + 1)
                                       : (Bits - semantics->precision);
  بازگشت رقم_بالا_Bits;
}

منطقی IEEEFloat::isSignificandAllOnes() ثابت {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  ثابت integerPart *Parts = significandParts();
  ثابت unsigned Part_شمار = part_شمار_For_Bits(semantics->precision);
  برای (unsigned i = 0; i < Part_شمار - 1; i++)
    اگر (~Parts[i])
      بازگشت نادرست;

  // Set the unused high bits to all ones when we compare.
  ثابت unsigned رقم_بالا_Bits = get_رقم_بالا_Bits();
  أكد(رقم_بالا_Bits <= integerPartWidth && رقم_بالا_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ثابت integerPart بالا_Bit_Fill =
    ~integerPart(0) << (integerPartWidth - رقم_بالا_Bits);
  اگر ((semantics->precision <= 1) || (~(Parts[Part_شمار - 1] | بالا_Bit_Fill)))
    بازگشت نادرست;

  بازگشت درست;
}

منطقی IEEEFloat::isSignificandAllOnesExceptLSB() ثابت {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  ثابت integerPart *Parts = significandParts();

  اگر (Parts[0] & 1)
    بازگشت نادرست;

  ثابت unsigned Part_شمار = part_شمار_For_Bits(semantics->precision);
  برای (unsigned i = 0; i < Part_شمار - 1; i++) {
    اگر (~Parts[i] & ~unsigned{!i})
      بازگشت نادرست;
  }

  // Set the unused high bits to all ones when we compare.
  ثابت unsigned رقم_بالا_Bits = get_رقم_بالا_Bits();
  أكد(رقم_بالا_Bits <= integerPartWidth && رقم_بالا_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ثابت integerPart بالا_Bit_Fill = ~integerPart(0)
                                  << (integerPartWidth - رقم_بالا_Bits);
  اگر (~(Parts[Part_شمار - 1] | بالا_Bit_Fill | 0x1))
    بازگشت نادرست;

  بازگشت درست;
}

منطقی IEEEFloat::isSignificandAllZeros() ثابت {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  ثابت integerPart *Parts = significandParts();
  ثابت unsigned Part_شمار = part_شمار_For_Bits(semantics->precision);

  برای (unsigned i = 0; i < Part_شمار - 1; i++)
    اگر (Parts[i])
      بازگشت نادرست;

  // Compute how many bits are used in the final word.
  ثابت unsigned رقم_بالا_Bits = get_رقم_بالا_Bits();
  أكد(رقم_بالا_Bits < integerPartWidth && "Can not have more high bits to "
         "clear than integerPartWidth");
  ثابت integerPart بالا_Bit_Mask = ~integerPart(0) >> رقم_بالا_Bits;

  اگر ((semantics->precision > 1) && (Parts[Part_شمار - 1] & بالا_Bit_Mask))
    بازگشت نادرست;

  بازگشت درست;
}

منطقی IEEEFloat::isSignificandAllZerosExceptMSB() ثابت {
  ثابت integerPart *Parts = significandParts();
  ثابت unsigned Part_شمار = part_شمار_For_Bits(semantics->precision);

  برای (unsigned i = 0; i < Part_شمار - 1; i++) {
    اگر (Parts[i])
      بازگشت نادرست;
  }

  ثابت unsigned رقم_بالا_Bits = get_رقم_بالا_Bits();
  ثابت integerPart MSBMask = integerPart(1)
                              << (integerPartWidth - رقم_بالا_Bits);
  بازگشت ((semantics->precision <= 1) || (Parts[Part_شمار - 1] == MSBMask));
}

منطقی IEEEFloat::isLargest() ثابت {
  منطقی IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    بازگشت (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } وگرنه {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    بازگشت IsMaxExp && isSignificandAllOnes();
  }
}

منطقی IEEEFloat::isInteger() ثابت {
  // This could be made more efficient; I'm going for obviously correct.
  اگر (!isFinite()) بازگشت نادرست;
  IEEEFloat truncated = *این;
  truncated.roundToIntegral(rmTowardZero);
  بازگشت compare(truncated) == cmpEqual;
}

منطقی IEEEFloat::bitwiseIsEqual(ثابت IEEEFloat &rhs) ثابت {
  اگر (این == &rhs)
    بازگشت درست;
  اگر (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    بازگشت نادرست;
  اگر (category==fcZero || category==fcInfinity)
    بازگشت درست;

  اگر (isFiniteNonZero() && exponent != rhs.exponent)
    بازگشت نادرست;

  بازگشت std::equal(significandParts(), significandParts() + part_شمار(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics, integerPart مقدار_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fcNormal;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = مقدار_2;
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
  ourSemantics.hasZero ? makeZero(نادرست) : makeSmallestNormalized(نادرست);
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
  *این = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

unsigned صحیح IEEEFloat::part_شمار() ثابت {
  بازگشت part_شمار_For_Bits(semantics->precision + 1);
}

ثابت APFloat::integerPart *IEEEFloat::significandParts() ثابت {
  بازگشت const_cast<IEEEFloat *>(این)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  اگر (part_شمار() > 1)
    بازگشت significand.parts;
  وگرنه
    بازگشت &significand.part;
}

تهی IEEEFloat::zeroSignificand() {
  APInt::tcSet(significandParts(), 0, part_شمار());
}

/* Increment an fcNormal floating point number's significand.  */
تهی IEEEFloat::incrementSignificand() {
  [[maybe_unused]] integerPart carry =
      APInt::tcIncrement(significandParts(), part_شمار());

  /* Our callers should never cause us to overflow.  */
  أكد(carry == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::addSignificand(ثابت IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  أكد(semantics == rhs.semantics);
  أكد(exponent == rhs.exponent);

  بازگشت APInt::tcAdd(parts, rhs.significandParts(), 0, part_شمار());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::subtractSignificand(ثابت IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  أكد(semantics == rhs.semantics);
  أكد(exponent == rhs.exponent);

  بازگشت APInt::tcSubtract(parts, rhs.significandParts(), borrow,
                           part_شمار());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
lostFraction IEEEFloat::multiplySignificand(ثابت IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            منطقی ignoreAddend) {
  integerPart scratch[4];
  منطقی ignored;

  أكد(semantics == rhs.semantics);

  unsigned precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  unsigned new_Parts_شمار = part_شمار_For_Bits(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *fullSignificand =
      new_Parts_شمار > 4 ? جدید integerPart[new_Parts_شمار] : scratch;

  integerPart *lhsSignificand = significandParts();
  unsigned parts_شمار = part_شمار();

  APInt::tcFullMultiply(fullSignificand, lhsSignificand,
                        rhs.significandParts(), parts_شمار, parts_شمار);

  lostFraction lost_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  unsigned omsb = APInt::tcMSB(fullSignificand, new_Parts_شمار) + 1;
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

  اگر (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    ثابت fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    unsigned extendedPrecision = 2 * precision + 1;
    اگر (omsb != extendedPrecision - 1) {
      أكد(extendedPrecision > omsb);
      APInt::tc_Shift_چپ(fullSignificand, new_Parts_شمار,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    اگر (new_Parts_شمار == 1)
      significand.part = fullSignificand[0];
    وگرنه
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
    lost_fraction = extendedAddend.shift_Significand_راست(1);
    أكد(lost_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    lost_fraction = addOrSubtractSignificand(extendedAddend, نادرست);

    /* Restore our state.  */
    اگر (new_Parts_شمار == 1)
      fullSignificand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(fullSignificand, new_Parts_شمار) + 1;
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
  اگر (omsb > precision) {
    unsigned صحیح bits, significantParts;
    lostFraction lf;

    bits = omsb - precision;
    significantParts = part_شمار_For_Bits(omsb);
    lf = shift_راست(fullSignificand, significantParts, bits);
    lost_fraction = combineLostFractions(lf, lost_fraction);
    exponent += bits;
  }

  APInt::tcAssign(lhsSignificand, fullSignificand, parts_شمار);

  اگر (new_Parts_شمار > 4)
    delete [] fullSignificand;

  بازگشت lost_fraction;
}

lostFraction IEEEFloat::multiplySignificand(ثابت IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  بازگشت multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
lostFraction IEEEFloat::divideSignificand(ثابت IEEEFloat &rhs) {
  integerPart scratch[4];

  أكد(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  ثابت integerPart *rhsSignificand = rhs.significandParts();
  unsigned parts_شمار = part_شمار();

  integerPart *dividend =
      parts_شمار > 2 ? جدید integerPart[parts_شمار * 2] : scratch;
  integerPart *مقسوم_علیه = dividend + parts_شمار;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  برای (unsigned i = 0; i < parts_شمار; i++) {
    dividend[i] = lhsSignificand[i];
    مقسوم_علیه[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  unsigned صحیح precision = semantics->precision;

  /* Normalize the divisor.  */
  unsigned bit = precision - APInt::tcMSB(مقسوم_علیه, parts_شمار) - 1;
  اگر (bit) {
    exponent += bit;
    APInt::tc_Shift_چپ(مقسوم_علیه, parts_شمار, bit);
  }

  /* Normalize the dividend.  */
  bit = precision - APInt::tcMSB(dividend, parts_شمار) - 1;
  اگر (bit) {
    exponent -= bit;
    APInt::tc_Shift_چپ(dividend, parts_شمار, bit);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  اگر (APInt::tcCompare(dividend, مقسوم_علیه, parts_شمار) < 0) {
    exponent--;
    APInt::tc_Shift_چپ(dividend, parts_شمار, 1);
    أكد(APInt::tcCompare(dividend, مقسوم_علیه, parts_شمار) >= 0);
  }

  /* Long division.  */
  برای (bit = precision; bit; bit -= 1) {
    اگر (APInt::tcCompare(dividend, مقسوم_علیه, parts_شمار) >= 0) {
      APInt::tcSubtract(dividend, مقسوم_علیه, 0, parts_شمار);
      APInt::tcSetBit(lhsSignificand, bit - 1);
    }

    APInt::tc_Shift_چپ(dividend, parts_شمار, 1);
  }

  /* Figure out the lost fraction.  */
  صحیح cmp = APInt::tcCompare(dividend, مقسوم_علیه, parts_شمار);

  lostFraction lost_fraction;
  اگر (cmp > 0)
    lost_fraction = lfMoreThanHalf;
  وگرنه اگر (cmp == 0)
    lost_fraction = lfExactlyHalf;
  وگرنه اگر (APInt::tcIsZero(dividend, parts_شمار))
    lost_fraction = lfExactlyZero;
  وگرنه
    lost_fraction = lfLessThanHalf;

  اگر (parts_شمار > 2)
    delete [] dividend;

  بازگشت lost_fraction;
}

unsigned صحیح IEEEFloat::significandMSB() ثابت {
  بازگشت APInt::tcMSB(significandParts(), part_شمار());
}

unsigned صحیح IEEEFloat::significandLSB() ثابت {
  بازگشت APInt::tcLSB(significandParts(), part_شمار());
}

/* Note that a zero result is NOT normalized to fcZero.  */
lostFraction IEEEFloat::shift_Significand_راست(unsigned صحیح bits) {
  /* Our exponent should not overflow.  */
  أكد((ExponentType) (exponent + bits) >= exponent);

  exponent += bits;

  بازگشت shift_راست(significandParts(), part_شمار(), bits);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
تهی IEEEFloat::shift_Significand_چپ(unsigned صحیح bits) {
  أكد(bits < semantics->precision ||
         (semantics->precision == 1 && bits <= 1));

  اگر (bits) {
    unsigned صحیح parts_شمار = part_شمار();

    APInt::tc_Shift_چپ(significandParts(), parts_شمار, bits);
    exponent -= bits;

    أكد(!APInt::tcIsZero(significandParts(), parts_شمار));
  }
}

APFloat::cmpResult IEEEFloat::compare_Absolute_مقدار(ثابت IEEEFloat &rhs) ثابت {
  أكد(semantics == rhs.semantics);
  أكد(isFiniteNonZero());
  أكد(rhs.isFiniteNonZero());

  صحیح compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  اگر (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_شمار());

  اگر (compare > 0)
    بازگشت cmpGreaterThan;
  وگرنه اگر (compare < 0)
    بازگشت cmpLessThan;
  وگرنه
    بازگشت cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
ایستا تهی tcSetLeastSignificantBits(APInt::WordType *dst, unsigned parts,
                                      unsigned bits) {
  unsigned i = 0;
  وقتی (bits > APInt::APINT_BITS_PER_WORD) {
    dst[i++] = ~(APInt::WordType)0;
    bits -= APInt::APINT_BITS_PER_WORD;
  }

  اگر (bits)
    dst[i++] = ~(APInt::WordType)0 >> (APInt::APINT_BITS_PER_WORD - bits);

  وقتی (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::opStatus IEEEFloat::handleOverflow(roundingMode rounding_mode) {
  اگر (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    اگر (rounding_mode == rmNearestTiesToEven ||
        rounding_mode == rmNearestTiesToAway ||
        (rounding_mode == rmTowardPositive && !sign) ||
        (rounding_mode == rmTowardNegative && sign)) {
      اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(نادرست, sign);
      وگرنه
        category = fcInfinity;
      بازگشت static_cast<opStatus>(opOverflow | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fcNormal;
  exponent = semantics->maxExponent;
  tcSetLeastSignificantBits(significandParts(), part_شمار(),
                            semantics->precision);
  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tcClearBit(significandParts(), 0);

  بازگشت opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
منطقی IEEEFloat::roundAwayFromZero(roundingMode rounding_mode,
                                  lostFraction lost_fraction,
                                  unsigned صحیح bit) ثابت {
  /* NaNs and infinities should not have lost fractions.  */
  أكد(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  أكد(lost_fraction != lfExactlyZero);

  انتخاب (rounding_mode) {
  حالت rmNearestTiesToAway:
    بازگشت lost_fraction == lfExactlyHalf || lost_fraction == lfMoreThanHalf;

  حالت rmNearestTiesToEven:
    اگر (lost_fraction == lfMoreThanHalf)
      بازگشت درست;

    /* Our zeroes don't have a significand to test.  */
    اگر (lost_fraction == lfExactlyHalf && category != fcZero)
      بازگشت APInt::tcExtractBit(significandParts(), bit);

    بازگشت نادرست;

  حالت rmTowardZero:
    بازگشت نادرست;

  حالت rmTowardPositive:
    بازگشت !sign;

  حالت rmTowardNegative:
    بازگشت sign;

  پیشفرض:
    بشکن;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::opStatus IEEEFloat::normalize(roundingMode rounding_mode,
                                       lostFraction lost_fraction) {
  اگر (!isFiniteNonZero())
    بازگشت opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  unsigned omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  اگر (omsb || lost_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    صحیح exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    اگر (exponent + exponentChange > semantics->maxExponent)
      بازگشت handleOverflow(rounding_mode);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    اگر (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    اگر (exponentChange < 0) {
      أكد(lost_fraction == lfExactlyZero);

      shift_Significand_چپ(-exponentChange);

      بازگشت opOK;
    }

    اگر (exponentChange > 0) {
      lostFraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = shift_Significand_راست(exponentChange);

      lost_fraction = combineLostFractions(lf, lost_fraction);

      /* Keep OMSB up-to-date.  */
      اگر (omsb > (unsigned) exponentChange)
        omsb -= exponentChange;
      وگرنه
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    بازگشت handleOverflow(rounding_mode);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  اگر (lost_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    اگر (omsb == 0) {
      category = fcZero;
      اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = نادرست;
      اگر (!semantics->hasZero)
        makeSmallestNormalized(نادرست);
    }

    بازگشت opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  اگر (roundAwayFromZero(rounding_mode, lost_fraction, 0)) {
    اگر (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    اگر (omsb == (unsigned) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      اگر (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        بازگشت handleOverflow(sign ? rmTowardNegative : rmTowardPositive);

      shift_Significand_راست(1);

      بازگشت opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      بازگشت handleOverflow(rounding_mode);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  اگر (omsb == semantics->precision)
    بازگشت opInexact;

  /* We have a non-zero denormal.  */
  أكد(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  اگر (omsb == 0) {
    category = fcZero;
    اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = نادرست;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    اگر (!semantics->hasZero)
      makeSmallestNormalized(نادرست);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  بازگشت (opStatus) (opUnderflow | opInexact);
}

APFloat::opStatus IEEEFloat::addOrSubtractSpecials(ثابت IEEEFloat &rhs,
                                                   منطقی subtract) {
  انتخاب (Pack_Categories_Into_کلید(category, rhs.category)) {
  پیشفرض:
    llvm_unreachable(nullptr);

  حالت Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالت Pack_Categories_Into_کلید(fcNormal, fcNaN):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  حالت Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNormal):
  حالت Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNaN):
    اگر (isSignaling()) {
      makeQuiet();
      بازگشت opInvalidOp;
    }
    بازگشت rhs.isSignaling() ? opInvalidOp : opOK;

  حالت Pack_Categories_Into_کلید(fcNormal, fcZero):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNormal):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcZero):
    بازگشت opOK;

  حالت Pack_Categories_Into_کلید(fcNormal, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ subtract;
    بازگشت opOK;

  حالت Pack_Categories_Into_کلید(fcZero, fcNormal):
    assign(rhs);
    sign = rhs.sign ^ subtract;
    بازگشت opOK;

  حالت Pack_Categories_Into_کلید(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    بازگشت opOK;

  حالت Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    اگر (((sign ^ rhs.sign)!=0) != subtract) {
      makeNaN();
      بازگشت opInvalidOp;
    }

    بازگشت opOK;

  حالت Pack_Categories_Into_کلید(fcNormal, fcNormal):
    بازگشت opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
lostFraction IEEEFloat::addOrSubtractSignificand(ثابت IEEEFloat &rhs,
                                                 منطقی subtract) {
  [[maybe_unused]] integerPart carry = 0;
  lostFraction lost_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  subtract ^= static_cast<منطقی>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  صحیح bits = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  اگر (subtract) {
    اگر ((bits < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat temp_rhs(rhs);
    منطقی lost_fraction_is_from_rhs = نادرست;

    اگر (bits == 0)
      lost_fraction = lfExactlyZero;
    وگرنه اگر (bits > 0) {
      lost_fraction = temp_rhs.shift_Significand_راست(bits - 1);
      lost_fraction_is_from_rhs = درست;
      shift_Significand_چپ(1);
    } وگرنه {
      lost_fraction = shift_Significand_راست(-bits - 1);
      temp_rhs.shift_Significand_چپ(1);
    }

    // Should we reverse the subtraction.
    cmpResult cmp_result = compare_Absolute_مقدار(temp_rhs);
    اگر (cmp_result == cmpLessThan) {
      منطقی borrow =
          lost_fraction != lfExactlyZero && !lost_fraction_is_from_rhs;
      اگر (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        اگر (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        وگرنه اگر (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = temp_rhs.subtractSignificand(*این, borrow);
      copySignificand(temp_rhs);
      sign = !sign;
    } وگرنه اگر (cmp_result == cmpGreaterThan) {
      منطقی borrow = lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs;
      اگر (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        اگر (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        وگرنه اگر (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = subtractSignificand(temp_rhs, borrow);
    } وگرنه { // cmpEqual
      zeroSignificand();
      اگر (lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    أكد(!carry);
  } وگرنه {
    اگر (bits > 0) {
      IEEEFloat temp_rhs(rhs);

      lost_fraction = temp_rhs.shift_Significand_راست(bits);
      carry = addSignificand(temp_rhs);
    } وگرنه {
      lost_fraction = shift_Significand_راست(-bits);
      carry = addSignificand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    أكد(!carry);
  }

  بازگشت lost_fraction;
}

APFloat::opStatus IEEEFloat::multiplySpecials(ثابت IEEEFloat &rhs) {
  انتخاب (Pack_Categories_Into_کلید(category, rhs.category)) {
  پیشفرض:
    llvm_unreachable(nullptr);

  حالت Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالت Pack_Categories_Into_کلید(fcNormal, fcNaN):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    assign(rhs);
    sign = نادرست;
    [[fallthrough]];
  حالت Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNormal):
  حالت Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    اگر (isSignaling()) {
      makeQuiet();
      بازگشت opInvalidOp;
    }
    بازگشت rhs.isSignaling() ? opInvalidOp : opOK;

  حالت Pack_Categories_Into_کلید(fcNormal, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNormal):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
    category = fcInfinity;
    بازگشت opOK;

  حالت Pack_Categories_Into_کلید(fcZero, fcNormal):
  حالت Pack_Categories_Into_کلید(fcNormal, fcZero):
  حالت Pack_Categories_Into_کلید(fcZero, fcZero):
    category = fcZero;
    بازگشت opOK;

  حالت Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcZero):
    makeNaN();
    بازگشت opInvalidOp;

  حالت Pack_Categories_Into_کلید(fcNormal, fcNormal):
    بازگشت opOK;
  }
}

APFloat::opStatus IEEEFloat::divideSpecials(ثابت IEEEFloat &rhs) {
  انتخاب (Pack_Categories_Into_کلید(category, rhs.category)) {
  پیشفرض:
    llvm_unreachable(nullptr);

  حالت Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالت Pack_Categories_Into_کلید(fcNormal, fcNaN):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    assign(rhs);
    sign = نادرست;
    [[fallthrough]];
  حالت Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNormal):
  حالت Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    اگر (isSignaling()) {
      makeQuiet();
      بازگشت opInvalidOp;
    }
    بازگشت rhs.isSignaling() ? opInvalidOp : opOK;

  حالت Pack_Categories_Into_کلید(fcInfinity, fcZero):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNormal):
  حالت Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fcNormal):
    بازگشت opOK;

  حالت Pack_Categories_Into_کلید(fcNormal, fcInfinity):
    category = fcZero;
    بازگشت opOK;

  حالت Pack_Categories_Into_کلید(fcNormal, fcZero):
    اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(نادرست, sign);
    وگرنه
      category = fcInfinity;
    بازگشت opDivByZero;

  حالت Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fcZero):
    makeNaN();
    بازگشت opInvalidOp;

  حالت Pack_Categories_Into_کلید(fcNormal, fcNormal):
    بازگشت opOK;
  }
}

APFloat::opStatus IEEEFloat::modSpecials(ثابت IEEEFloat &rhs) {
  انتخاب (Pack_Categories_Into_کلید(category, rhs.category)) {
  پیشفرض:
    llvm_unreachable(nullptr);

  حالت Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالت Pack_Categories_Into_کلید(fcNormal, fcNaN):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  حالت Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNormal):
  حالت Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNaN):
    اگر (isSignaling()) {
      makeQuiet();
      بازگشت opInvalidOp;
    }
    بازگشت rhs.isSignaling() ? opInvalidOp : opOK;

  حالت Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fcNormal):
  حالت Pack_Categories_Into_کلید(fcNormal, fcInfinity):
    بازگشت opOK;

  حالت Pack_Categories_Into_کلید(fcNormal, fcZero):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcZero):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNormal):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fcZero):
    makeNaN();
    بازگشت opInvalidOp;

  حالت Pack_Categories_Into_کلید(fcNormal, fcNormal):
    بازگشت opOK;
  }
}

APFloat::opStatus IEEEFloat::remainderSpecials(ثابت IEEEFloat &rhs) {
  انتخاب (Pack_Categories_Into_کلید(category, rhs.category)) {
  پیشفرض:
    llvm_unreachable(nullptr);

  حالت Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالت Pack_Categories_Into_کلید(fcNormal, fcNaN):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  حالت Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNormal):
  حالت Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNaN):
    اگر (isSignaling()) {
      makeQuiet();
      بازگشت opInvalidOp;
    }
    بازگشت rhs.isSignaling() ? opInvalidOp : opOK;

  حالت Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fcNormal):
  حالت Pack_Categories_Into_کلید(fcNormal, fcInfinity):
    بازگشت opOK;

  حالت Pack_Categories_Into_کلید(fcNormal, fcZero):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcZero):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNormal):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fcZero):
    makeNaN();
    بازگشت opInvalidOp;

  حالت Pack_Categories_Into_کلید(fcNormal, fcNormal):
    بازگشت opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
تهی IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    بازگشت;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::opStatus IEEEFloat::addOrSubtract(ثابت IEEEFloat &rhs,
                                           roundingMode rounding_mode,
                                           منطقی subtract) {
  opStatus fs = addOrSubtractSpecials(rhs, subtract);

  /* This return code means it was not a simple case.  */
  اگر (fs == opDivByZero) {
    lostFraction lost_fraction;

    lost_fraction = addOrSubtractSignificand(rhs, subtract);
    fs = normalize(rounding_mode, lost_fraction);

    /* Can only be zero if we lost no fraction.  */
    أكد(category != fcZero || lost_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  اگر (category == fcZero) {
    اگر (rhs.category != fcZero || (sign == rhs.sign) == subtract)
      sign = (rounding_mode == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = نادرست;
  }

  بازگشت fs;
}

/* Normalized addition.  */
APFloat::opStatus IEEEFloat::add(ثابت IEEEFloat &rhs,
                                 roundingMode rounding_mode) {
  بازگشت addOrSubtract(rhs, rounding_mode, نادرست);
}

/* Normalized subtraction.  */
APFloat::opStatus IEEEFloat::subtract(ثابت IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  بازگشت addOrSubtract(rhs, rounding_mode, درست);
}

/* Normalized multiply.  */
APFloat::opStatus IEEEFloat::multiply(ثابت IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = multiplySpecials(rhs);

  اگر (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = نادرست;
  اگر (isFiniteNonZero()) {
    lostFraction lost_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    اگر (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  بازگشت fs;
}

/* Normalized divide.  */
APFloat::opStatus IEEEFloat::divide(ثابت IEEEFloat &rhs,
                                    roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = divideSpecials(rhs);

  اگر (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = نادرست;
  اگر (isFiniteNonZero()) {
    lostFraction lost_fraction = divideSignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    اگر (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  بازگشت fs;
}

/* Normalized remainder.  */
APFloat::opStatus IEEEFloat::remainder(ثابت IEEEFloat &rhs) {
  unsigned صحیح origSign = sign;

  // First handle the special cases.
  opStatus fs = remainderSpecials(rhs);
  اگر (fs != opDivByZero)
    بازگشت fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  اگر (P2.add(rhs, rmNearestTiesToEven) == opOK) {
    fs = mod(P2);
    أكد(fs == opOK);
  }

  // Lets work with absolute numbers.
  IEEEFloat P = rhs;
  P.sign = نادرست;
  sign = نادرست;

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
  منطقی losesInfo;
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.maxExponent++;
  extendedSemantics.minExponent--;
  extendedSemantics.precision += 2;

  IEEEFloat VEx = *این;
  fs = VEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  أكد(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  أكد(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.add(VEx, rmNearestTiesToEven);
  أكد(fs == opOK);

  اگر (VEx.compare(PEx) == cmpGreaterThan) {
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
    اگر (result == cmpGreaterThan || result == cmpEqual) {
      fs = subtract(P, rmNearestTiesToEven);
      أكد(fs == opOK);
    }
  }

  اگر (isZero()) {
    sign = origSign;    // IEEE754 requires this
    اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = نادرست;
  } وگرنه {
    sign ^= origSign;
  }
  بازگشت fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::opStatus IEEEFloat::mod(ثابت IEEEFloat &rhs) {
  opStatus fs = modSpecials(rhs);
  unsigned صحیح origSign = sign;

  وقتی (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_مقدار(rhs) != cmpLessThan) {
    صحیح Exp = ilogb(*این) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rmNearestTiesToEven);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    اگر (V.isNaN() || compare_Absolute_مقدار(V) == cmpLessThan)
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
    اگر (!semantics->hasZero && این->isSmallest())
      بشکن;

    أكد(fs==opOK);
  }
  اگر (isZero()) {
    sign = origSign; // fmod requires this
    اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = نادرست;
  }
  بازگشت fs;
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
  اگر (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    lostFraction lost_fraction;

    lost_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_mode, lost_fraction);
    اگر (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    اگر (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_mode == rmTowardNegative);
      اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = نادرست;
    }
  } وگرنه {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    اگر (fs == opOK)
      fs = addOrSubtract(addend, rounding_mode, نادرست);
  }

  بازگشت fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::opStatus IEEEFloat::roundToIntegral(roundingMode rounding_mode) {
  اگر (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    بازگشت opOK;

  اگر (isNaN()) {
    اگر (isSignaling()) {
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
      بازگشت opInvalidOp;
    } وگرنه {
      // [IEEE Std 754-2008 6.2]:
      // For an operation with quiet NaN inputs, other than maximum and minimum
      // operations, if a floating-point result is to be delivered the result
      // shall be a quiet NaN which should be one of the input NaNs.
      // ...
      // Every general-computational and quiet-computational operation involving
      // one or more input NaNs, none of them signaling, shall signal no
      // exception, except fusedMultiplyAdd might signal the invalid operation
      // exception(see 7.2).
      بازگشت opOK;
    }
  }

  اگر (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    بازگشت opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  اگر (exponent + 1 >= (صحیح)APFloat::semanticsPrecision(*semantics))
    بازگشت opOK;

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
  opStatus fs = MagicConstant.convertFromAPInt(IntegerConstant, نادرست,
                                               rmNearestTiesToEven);
  أكد(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  منطقی درون_داد_Sign = isNegative();

  fs = add(MagicConstant, rounding_mode);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  subtract(MagicConstant, rounding_mode);

  // Restore the input sign.
  اگر (درون_داد_Sign != isNegative())
    changeSign();

  بازگشت fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmpResult IEEEFloat::compare(ثابت IEEEFloat &rhs) ثابت {
  أكد(semantics == rhs.semantics);

  انتخاب (Pack_Categories_Into_کلید(category, rhs.category)) {
  پیشفرض:
    llvm_unreachable(nullptr);

  حالت Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNormal):
  حالت Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNaN):
  حالت Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالت Pack_Categories_Into_کلید(fcNormal, fcNaN):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    بازگشت cmpUnordered;

  حالت Pack_Categories_Into_کلید(fcInfinity, fcNormal):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcZero):
  حالت Pack_Categories_Into_کلید(fcNormal, fcZero):
    اگر (sign)
      بازگشت cmpLessThan;
    وگرنه
      بازگشت cmpGreaterThan;

  حالت Pack_Categories_Into_کلید(fcNormal, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fcNormal):
    اگر (rhs.sign)
      بازگشت cmpGreaterThan;
    وگرنه
      بازگشت cmpLessThan;

  حالت Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
    اگر (sign == rhs.sign)
      بازگشت cmpEqual;
    وگرنه اگر (sign)
      بازگشت cmpLessThan;
    وگرنه
      بازگشت cmpGreaterThan;

  حالت Pack_Categories_Into_کلید(fcZero, fcZero):
    بازگشت cmpEqual;

  حالت Pack_Categories_Into_کلید(fcNormal, fcNormal):
    بشکن;
  }

  cmpResult result;
  /* Two normal numbers.  Do they have the same sign?  */
  اگر (sign != rhs.sign) {
    اگر (sign)
      result = cmpLessThan;
    وگرنه
      result = cmpGreaterThan;
  } وگرنه {
    /* Compare absolute values; invert result if negative.  */
    result = compare_Absolute_مقدار(rhs);

    اگر (sign) {
      اگر (result == cmpLessThan)
        result = cmpGreaterThan;
      وگرنه اگر (result == cmpGreaterThan)
        result = cmpLessThan;
    }
  }

  بازگشت result;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::opStatus IEEEFloat::convert(ثابت fltSemantics &toSemantics,
                                     roundingMode rounding_mode,
                                     منطقی *losesInfo) {
  opStatus fs;
  ثابت fltSemantics &fromSemantics = *semantics;
  منطقی is_signaling = isSignaling();

  lostFraction lostFraction = lfExactlyZero;
  unsigned new_Part_شمار = part_شمار_For_Bits(toSemantics.precision + 1);
  unsigned old_Part_شمار = part_شمار();
  صحیح shift = toSemantics.precision - fromSemantics.precision;

  منطقی X86SpecialNan = نادرست;
  اگر (&fromSemantics == &APFloatBase::semX87DoubleExtended &&
      &toSemantics != &APFloatBase::semX87DoubleExtended && category == fcNaN &&
      (!(*significandParts() & 0x8000000000000000ULL) ||
       !(*significandParts() & 0x4000000000000000ULL))) {
    // x86 has some unusual NaNs which cannot be represented in any other
    // format; note them here.
    X86SpecialNan = درست;
  }

  // If this is a truncation of a denormal number, and the target semantics
  // has larger exponent range than the source semantics (this can happen
  // when truncating from PowerPC double-double to double format), the
  // right shift could lose result mantissa bits.  Adjust exponent instead
  // of performing excessive shift.
  // Also do a similar trick in case shifting denormal would produce zero
  // significand as this case isn't handled correctly by normalize.
  اگر (shift < 0 && isFiniteNonZero()) {
    صحیح omsb = significandMSB() + 1;
    صحیح exponentChange = omsb - fromSemantics.precision;
    اگر (exponent + exponentChange < toSemantics.minExponent)
      exponentChange = toSemantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, shift);
    اگر (exponentChange < 0) {
      shift -= exponentChange;
      exponent += exponentChange;
    } وگرنه اگر (omsb <= -shift) {
      exponentChange = omsb + shift - 1; // leave at least one bit set
      shift -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  اگر (shift < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    lostFraction = shift_راست(significandParts(), old_Part_شمار, -shift);

  // Fix the storage so it can hold to new value.
  اگر (new_Part_شمار > old_Part_شمار) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = جدید integerPart[new_Part_شمار];
    APInt::tcSet(newParts, 0, new_Part_شمار);
    اگر (isFiniteNonZero() || category==fcNaN)
      APInt::tcAssign(newParts, significandParts(), old_Part_شمار);
    freeSignificand();
    significand.parts = newParts;
  } وگرنه اگر (new_Part_شمار == 1 && old_Part_شمار != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    اگر (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &toSemantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  اگر (shift > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_Shift_چپ(significandParts(), new_Part_شمار, shift);

  اگر (isFiniteNonZero()) {
    fs = normalize(rounding_mode, lostFraction);
    *losesInfo = (fs != opOK);
  } وگرنه اگر (category == fcNaN) {
    اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(نادرست, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } وگرنه {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      اگر (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(نادرست, نادرست);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      اگر (!APFloat::hasSignificand(fromSemantics))
        makeNaN(نادرست, sign);

      *losesInfo = lostFraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      اگر (!X86SpecialNan && semantics == &APFloatBase::semX87DoubleExtended)
        APInt::tcSetBit(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      اگر (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } وگرنه {
        fs = opOK;
      }
    }
  } وگرنه اگر (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(نادرست, sign);
    *losesInfo = درست;
    fs = opInexact;
  } وگرنه اگر (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = نادرست;
  } وگرنه {
    *losesInfo = نادرست;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  اگر ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = درست;
    اگر (fs == opOK)
      fs = opInexact;
  }

  اگر (category == fcZero && !semantics->hasZero)
    makeSmallestNormalized(نادرست);
  بازگشت fs;
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
    MutableArrayRef<integerPart> parts, unsigned صحیح width, منطقی isSigned,
    roundingMode rounding_mode, منطقی *isExact) ثابت {
  *isExact = نادرست;

  /* Handle the three special cases first.  */
  اگر (category == fcInfinity || category == fcNaN)
    بازگشت opInvalidOp;

  unsigned dst_Parts_شمار = part_شمار_For_Bits(width);
  أكد(dst_Parts_شمار <= parts.size() && "Integer too big");

  اگر (category == fcZero) {
    APInt::tcSet(parts.data(), 0, dst_Parts_شمار);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    بازگشت opOK;
  }

  ثابت integerPart *src = significandParts();

  unsigned truncatedBits;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  اگر (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tcSet(parts.data(), 0, dst_Parts_شمار);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncatedBits = semantics->precision -1U - exponent;
  } وگرنه {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    unsigned صحیح bits = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    اگر (bits > width)
      بازگشت opInvalidOp;

    اگر (bits < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncatedBits = semantics->precision - bits;
      APInt::tcExtract(parts.data(), dst_Parts_شمار, src, bits, truncatedBits);
    } وگرنه {
      /* We want at least as many bits as are available.  */
      APInt::tcExtract(parts.data(), dst_Parts_شمار, src, semantics->precision,
                       0);
      APInt::tc_Shift_چپ(parts.data(), dst_Parts_شمار,
                         bits - semantics->precision);
      truncatedBits = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  lostFraction lost_fraction;
  اگر (truncatedBits) {
    lost_fraction = lostFractionThroughTruncation(src, part_شمار(),
                                                  truncatedBits);
    اگر (lost_fraction != lfExactlyZero &&
        roundAwayFromZero(rounding_mode, lost_fraction, truncatedBits)) {
      اگر (APInt::tcIncrement(parts.data(), dst_Parts_شمار))
        بازگشت opInvalidOp;     /* Overflow.  */
    }
  } وگرنه {
    lost_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  unsigned صحیح omsb = APInt::tcMSB(parts.data(), dst_Parts_شمار) + 1;

  اگر (sign) {
    اگر (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      اگر (omsb != 0)
        بازگشت opInvalidOp;
    } وگرنه {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      اگر (omsb == width &&
          APInt::tcLSB(parts.data(), dst_Parts_شمار) + 1 != omsb)
        بازگشت opInvalidOp;

      /* This case can happen because of rounding.  */
      اگر (omsb > width)
        بازگشت opInvalidOp;
    }

    APInt::tcNegate (parts.data(), dst_Parts_شمار);
  } وگرنه {
    اگر (omsb >= width + !isSigned)
      بازگشت opInvalidOp;
  }

  اگر (lost_fraction == lfExactlyZero) {
    *isExact = درست;
    بازگشت opOK;
  }
  بازگشت opInexact;
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
                            unsigned صحیح width, منطقی isSigned,
                            roundingMode rounding_mode, منطقی *isExact) ثابت {
  opStatus fs = convertToSignExtendedInteger(parts, width, isSigned,
                                             rounding_mode, isExact);

  اگر (fs == opInvalidOp) {
    unsigned صحیح bits, dst_Parts_شمار;

    dst_Parts_شمار = part_شمار_For_Bits(width);
    أكد(dst_Parts_شمار <= parts.size() && "Integer too big");

    اگر (category == fcNaN)
      bits = 0;
    وگرنه اگر (sign)
      bits = isSigned;
    وگرنه
      bits = width - isSigned;

    tcSetLeastSignificantBits(parts.data(), dst_Parts_شمار, bits);
    اگر (sign && isSigned)
      APInt::tc_Shift_چپ(parts.data(), dst_Parts_شمار, width - 1);
  }

  بازگشت fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::opStatus IEEEFloat::convertFromUnsignedParts(
    ثابت integerPart *src, unsigned صحیح src_شمار, roundingMode rounding_mode) {
  category = fcNormal;
  unsigned omsb = APInt::tcMSB(src, src_شمار) + 1;
  integerPart *dst = significandParts();
  unsigned dst_شمار = part_شمار();
  unsigned precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  lostFraction lost_fraction;
  اگر (precision <= omsb) {
    exponent = omsb - 1;
    lost_fraction = lostFractionThroughTruncation(src, src_شمار,
                                                  omsb - precision);
    APInt::tcExtract(dst, dst_شمار, src, precision, omsb - precision);
  } وگرنه {
    exponent = precision - 1;
    lost_fraction = lfExactlyZero;
    APInt::tcExtract(dst, dst_شمار, src, omsb, 0);
  }

  بازگشت normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus IEEEFloat::convertFromAPInt(ثابت APInt &Val, منطقی isSigned,
                                              roundingMode rounding_mode) {
  unsigned صحیح part_شمار = Val.get_رقم_واژگان();
  APInt api = Val;

  sign = نادرست;
  اگر (isSigned && api.isNegative()) {
    sign = درست;
    api = -api;
  }

  بازگشت convertFromUnsignedParts(api.getRawData(), part_شمار, rounding_mode);
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        roundingMode rounding_mode) {
  lostFraction lost_fraction = lfExactlyZero;

  category = fcNormal;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  unsigned parts_شمار = part_شمار();
  unsigned bitPos = parts_شمار * integerPartWidth;
  منطقی computedTrailingFraction = نادرست;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator end = s.end();
  StringRef::iterator dot;
  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  اگر (!PtrOrErr)
    بازگشت PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator firstSignificantDigit = p;

  وقتی (p != end) {
    integerPart hex_مقدار;

    اگر (*p == '.') {
      اگر (dot != end)
        بازگشت createError("String contains multiple dots");
      dot = p++;
      ادامه;
    }

    hex_مقدار = hex_Digit_مقدار(*p);
    اگر (hex_مقدار == UINT_MAX)
      بشکن;

    p++;

    // Store the number while we have space.
    اگر (bitPos) {
      bitPos -= 4;
      hex_مقدار <<= bitPos % integerPartWidth;
      significand[bitPos / integerPartWidth] |= hex_مقدار;
    } وگرنه اگر (!computedTrailingFraction) {
      auto FractOrErr = trailingHexadecimalFraction(p, end, hex_مقدار);
      اگر (!FractOrErr)
        بازگشت FractOrErr.takeError();
      lost_fraction = *FractOrErr;
      computedTrailingFraction = درست;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  اگر (p == end)
    بازگشت createError("Hex strings require an exponent");
  اگر (*p != 'p' && *p != 'P')
    بازگشت createError("Invalid character in significand");
  اگر (p == begin)
    بازگشت createError("Significand has no digits");
  اگر (dot != end && p - begin == 1)
    بازگشت createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  اگر (p != firstSignificantDigit) {
    صحیح expAdjustment;

    /* Implicit hexadecimal point?  */
    اگر (dot == end)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<صحیح>(dot - firstSignificantDigit);
    اگر (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_شمار * integerPartWidth;

    /* Adjust for the given exponent.  */
    auto ExpOrErr = totalExponent(p + 1, end, expAdjustment);
    اگر (!ExpOrErr)
      بازگشت ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  بازگشت normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus
IEEEFloat::roundSignificandWithExponent(ثابت integerPart *decSigParts,
                                        unsigned sig_Part_شمار, صحیح exp,
                                        roundingMode rounding_mode) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  unsigned صحیح power = exp >= 0 ? exp : -exp;
  unsigned صحیح pow5PartCapacity = powerOfFivePartsForExponent(power);
  SmallVector<integerPart, رقم_Pow5_Parts_On_پشته> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5PartCapacity);

  منطقی isNearest = rounding_mode == rmNearestTiesToEven ||
                   rounding_mode == rmNearestTiesToAway;

  unsigned parts = part_شمار_For_Bits(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  unsigned pow5_Part_شمار = powerOf5(pow5Parts.data(), power);

  برای (;; parts *= 2) {
    unsigned صحیح excessPrecision, truncatedBits;

    calcSemantics.precision = parts * integerPartWidth - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncatedBits = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    opStatus sigStatus = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_شمار, rmNearestTiesToEven);
    opStatus powStatus = pow5.convertFromUnsignedParts(
        pow5Parts.data(), pow5_Part_شمار, rmNearestTiesToEven);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += exp;

    lostFraction calcLostFraction;
    integerPart HUerr, HUdistance;
    unsigned صحیح powHUerr;

    اگر (exp >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calcLostFraction = decSig.multiplySignificand(pow5);
      powHUerr = powStatus != opOK;
    } وگرنه {
      calcLostFraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      اگر (decSig.exponent < semantics->minExponent) {
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
    اگر (HUdistance >= HUerr) {
      APInt::tcExtract(significandParts(), part_شمار(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calcLostFraction = lostFractionThroughTruncation(decSig.significandParts(),
                                                       decSig.part_شمار(),
                                                       truncatedBits);
      بازگشت static_cast<opStatus>(normalize(rounding_mode, calcLostFraction) |
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
  اگر (Error Err = interpretDecimal(p, str.end(), &D))
    بازگشت std::move(Err);

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
  اگر (D.firstSigDigit == str.end() || dec_Digit_مقدار(*D.firstSigDigit) >= 10U) {
    category = fcZero;
    fs = opOK;
    اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = نادرست;
    اگر (!semantics->hasZero)
      makeSmallestNormalized(نادرست);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } وگرنه اگر (D.normalizedExponent - 1 > INT_MAX / 42039) {
    fs = handleOverflow(rounding_mode);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } وگرنه اگر (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (صحیح) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fcNormal;
    zeroSignificand();
    fs = normalize(rounding_mode, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } وگرنه اگر ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handleOverflow(rounding_mode);
  } وگرنه {
    integerPart *decSignificand;
    unsigned صحیح part_شمار;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_شمار = static_cast<unsigned صحیح>(D.lastSigDigit - D.firstSigDigit) + 1;
    part_شمار = part_شمار_For_Bits(1 + 196 * part_شمار / 59);
    decSignificand = جدید integerPart[part_شمار + 1];
    part_شمار = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    انجامبده {
      integerPart dec_مقدار, val, multiplier;

      val = 0;
      multiplier = 1;

      انجامبده {
        اگر (*p == '.') {
          p++;
          اگر (p == str.end()) {
            بشکن;
          }
        }
        dec_مقدار = dec_Digit_مقدار(*p++);
        اگر (dec_مقدار >= 10U) {
          delete[] decSignificand;
          بازگشت createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_مقدار;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } وقتی (p <= D.lastSigDigit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_شمار, part_شمار + 1, نادرست);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      اگر (decSignificand[part_شمار])
        part_شمار++;
    } وقتی (p <= D.lastSigDigit);

    category = fcNormal;
    fs = roundSignificandWithExponent(decSignificand, part_شمار,
                                      D.exponent, rounding_mode);

    delete [] decSignificand;
  }

  بازگشت fs;
}

منطقی IEEEFloat::convertFromStringSpecials(StringRef str) {
  ثابت size_t MIN_NAME_SIZE = 3;

  اگر (str.size() < MIN_NAME_SIZE)
    بازگشت نادرست;

  اگر (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(نادرست);
    بازگشت درست;
  }

  منطقی IsNegative = str.consume_front("-");
  اگر (IsNegative) {
    اگر (str.size() < MIN_NAME_SIZE)
      بازگشت نادرست;

    اگر (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(درست);
      بازگشت درست;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  منطقی IsSignaling = str.consume_front_insensitive("s");
  اگر (IsSignaling) {
    اگر (str.size() < MIN_NAME_SIZE)
      بازگشت نادرست;
  }

  اگر (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    اگر (str.empty()) {
      makeNaN(IsSignaling, IsNegative);
      بازگشت درست;
    }

    // Allow the payload to be inside parentheses.
    اگر (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      اگر (str.size() <= 2 || str.back() != ')')
        بازگشت نادرست;

      str = str.slice(1, str.size() - 1);
    }

    // Determine the payload number's radix.
    unsigned Radix = 10;
    اگر (str[0] == '0') {
      اگر (str.size() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } وگرنه {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    اگر (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      بازگشت درست;
    }
  }

  بازگشت نادرست;
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromString(StringRef str, roundingMode rounding_mode) {
  اگر (str.empty())
    بازگشت createError("Invalid string length");

  // Handle special cases.
  اگر (convertFromStringSpecials(str))
    بازگشت opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  sign = *p == '-' ? 1 : 0;
  اگر (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  اگر (*p == '-' || *p == '+') {
    p++;
    slen--;
    اگر (!slen)
      بازگشت createError("String has no digits");
  }

  اگر (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    اگر (slen == 2)
      بازگشت createError("Invalid string");
    بازگشت convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_mode);
  }

  بازگشت convertFromDecimalString(StringRef(p, slen), rounding_mode);
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
unsigned صحیح IEEEFloat::convertToHexString(نویسه *dst, unsigned صحیح hexDigits,
                                           منطقی upperCase,
                                           roundingMode rounding_mode) ثابت {
  نویسه *p = dst;
  اگر (sign)
    *dst++ = '-';

  انتخاب (category) {
  حالت fcInfinity:
    memcpy (dst, upperCase ? infinityU: infinityL, اندازه infinityU - 1);
    dst += اندازه infinityL - 1;
    بشکن;

  حالت fcNaN:
    memcpy (dst, upperCase ? NaNU: NaNL, اندازه NaNU - 1);
    dst += اندازه NaNU - 1;
    بشکن;

  حالت fcZero:
    *dst++ = '0';
    *dst++ = upperCase ? 'X': 'x';
    *dst++ = '0';
    اگر (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = upperCase ? 'P': 'p';
    *dst++ = '0';
    بشکن;

  حالت fcNormal:
    dst = convertNormalToHexString (dst, hexDigits, upperCase, rounding_mode);
    بشکن;
  }

  *dst = 0;

  بازگشت static_cast<unsigned صحیح>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
نویسه *IEEEFloat::convertNormalToHexString(نویسه *dst, unsigned صحیح hexDigits,
                                          منطقی upperCase,
                                          roundingMode rounding_mode) ثابت {
  *dst++ = '0';
  *dst++ = upperCase ? 'X': 'x';

  منطقی roundUp = نادرست;
  ثابت نویسه *hexDigitChars = upperCase ? hexDigitsUpper : hexDigitsLower;

  ثابت integerPart *significand = significandParts();
  unsigned parts_شمار = part_شمار();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  unsigned مقدار_Bits = semantics->precision + 3;
  unsigned shift = integerPartWidth - مقدار_Bits % integerPartWidth;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  unsigned outputDigits = (مقدار_Bits - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  اگر (hexDigits) {
    اگر (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      unsigned صحیح bits;
      lostFraction fraction;

      bits = مقدار_Bits - hexDigits * 4;
      fraction = lostFractionThroughTruncation (significand, parts_شمار, bits);
      roundUp = roundAwayFromZero(rounding_mode, fraction, bits);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  نویسه *p = ++dst;

  unsigned شمار = (مقدار_Bits + integerPartWidth - 1) / integerPartWidth;

  وقتی (outputDigits && شمار) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    اگر (--شمار == parts_شمار)
      part = 0;  /* An imaginary higher zero part.  */
    وگرنه
      part = significand[شمار] << shift;

    اگر (شمار && shift)
      part |= significand[شمار - 1] >> (integerPartWidth - shift);

    /* Convert as much of "part" to hexdigits as we can.  */
    unsigned صحیح curDigits = integerPartWidth / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  اگر (roundUp) {
    نویسه *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    انجامبده {
      q--;
      *q = hexDigitChars[hex_Digit_مقدار (*q) + 1];
    } وقتی (*q == '0');
    أكد(q >= p);
  } وگرنه {
    /* Add trailing zeroes.  */
    memset (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  اگر (dst -1 == p)
    dst--;
  وگرنه
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = upperCase ? 'P': 'p';

  بازگشت writeSignedDecimal (dst, exponent);
}

hash_code hash_مقدار(ثابت IEEEFloat &Arg) {
  اگر (!Arg.isFiniteNonZero())
    بازگشت hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  بازگشت hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.part_شمار()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convertF80LongDoubleAPFloatToAPInt() ثابت {
  أكد(part_شمار() == 2);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semX87DoubleExtended>();
}

APInt IEEEFloat::convertPPCDoubleDoubleLegacyAPFloatToAPInt() ثابت {
  أكد(semantics ==
         (ثابت llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy);
  أكد(part_شمار()==2);

  uint64_t واژگان[2];
  منطقی losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloatBase::semIEEEdouble.minExponent;
  IEEEFloat extended(*این);
  [[maybe_unused]] opStatus fs =
      extended.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  أكد(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
  أكد(fs == opOK || fs == opInexact);
  واژگان[0] = *u.convertDoubleAPFloatToAPInt().getRawData();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  اگر (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
    أكد(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.subtract(u, rmNearestTiesToEven);
    fs = v.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
    أكد(fs == opOK && !losesInfo);
    واژگان[1] = *v.convertDoubleAPFloatToAPInt().getRawData();
  } وگرنه {
    واژگان[1] = 0;
  }

  بازگشت APInt(128, واژگان);
}

template <ثابت fltSemantics &S>
APInt IEEEFloat::convertIEEEFloatToAPInt() ثابت {
  أكد(semantics == &S);
  constexpr unsigned صحیح trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr صحیح integer_bit_part = (S.precision - 1) / integerPartWidth;
  constexpr integerPart integer_bit = integerPart{1}
                                      << ((S.precision - 1) % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned صحیح exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr منطقی is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr صحیح bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_شمار_For_Bits(trailing_significand_bits)>
      mysignificand;

  اگر (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
    اگر (myexponent == 1 &&
        !(significandParts()[integer_bit_part] & integer_bit))
      myexponent = 0; // denormal
  } وگرنه اگر (category == fcZero) {
    اگر (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } وگرنه اگر (category == fcInfinity) {
    اگر (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    اگر constexpr (S.hasExplicitIntegerBit) {
      mysignificand[0] = integerPart{1} << (trailing_significand_bits - 1);
    }
  } وگرنه {
    أكد(category == fcNaN && "Unknown category!");
    اگر (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.sizeInBits + 63) / 64> واژگان;
  auto واژگان_iter =
      std::copy_n(mysignificand.begin(), mysignificand.size(), واژگان.begin());
  اگر constexpr (!S.hasExplicitIntegerBit) {
    اگر constexpr (significand_mask != 0 || trailing_significand_bits == 0) {
      // Clear the integer bit.
      واژگان[mysignificand.size() - 1] &= significand_mask;
    }
  }
  std::fill(واژگان_iter, واژگان.end(), uint64_t{0});
  constexpr size_t last_word = واژگان.size() - 1;
  اگر constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.sizeInBits - 1) % 64);
    واژگان[last_word] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_mask)
                              << (trailing_significand_bits % 64);
  واژگان[last_word] |= shifted_exponent;
  اگر constexpr (last_word == 0) {
    بازگشت APInt(S.sizeInBits, واژگان[0]);
  }
  بازگشت APInt(S.sizeInBits, واژگان);
}

APInt IEEEFloat::convertQuadrupleAPFloatToAPInt() ثابت {
  أكد(part_شمار() == 2);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semIEEEquad>();
}

APInt IEEEFloat::convertDoubleAPFloatToAPInt() ثابت {
  أكد(part_شمار()==1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semIEEEdouble>();
}

APInt IEEEFloat::convertFloatAPFloatToAPInt() ثابت {
  أكد(part_شمار()==1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semIEEEsingle>();
}

APInt IEEEFloat::convertBFloatAPFloatToAPInt() ثابت {
  أكد(part_شمار() == 1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semBFloat>();
}

APInt IEEEFloat::convertHalfAPFloatToAPInt() ثابت {
  أكد(part_شمار()==1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::APFloatBase::semIEEEhalf>();
}

APInt IEEEFloat::convertFloat8E5M2APFloatToAPInt() ثابت {
  أكد(part_شمار() == 1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2>();
}

APInt IEEEFloat::convertFloat8E5M2FNUZAPFloatToAPInt() ثابت {
  أكد(part_شمار() == 1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3APFloatToAPInt() ثابت {
  أكد(part_شمار() == 1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3>();
}

APInt IEEEFloat::convertFloat8E4M3FNAPFloatToAPInt() ثابت {
  أكد(part_شمار() == 1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FN>();
}

APInt IEEEFloat::convertFloat8E4M3FNUZAPFloatToAPInt() ثابت {
  أكد(part_شمار() == 1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3B11FNUZAPFloatToAPInt() ثابت {
  أكد(part_شمار() == 1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convertFloat8E3M4APFloatToAPInt() ثابت {
  أكد(part_شمار() == 1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semFloat8E3M4>();
}

APInt IEEEFloat::convertFloatTF32APFloatToAPInt() ثابت {
  أكد(part_شمار() == 1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semFloatTF32>();
}

APInt IEEEFloat::convertFloat8E8M0FNUAPFloatToAPInt() ثابت {
  أكد(part_شمار() == 1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convertFloat8E5M3FNUAPFloatToAPInt() ثابت {
  أكد(part_شمار() == 1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convertFloat6E3M2FNAPFloatToAPInt() ثابت {
  أكد(part_شمار() == 1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semFloat6E3M2FN>();
}

APInt IEEEFloat::convertFloat6E2M3FNAPFloatToAPInt() ثابت {
  أكد(part_شمار() == 1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semFloat6E2M3FN>();
}

APInt IEEEFloat::convertFloat4E2M1FNAPFloatToAPInt() ثابت {
  أكد(part_شمار() == 1);
  بازگشت convertIEEEFloatToAPInt<APFloatBase::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcastToAPInt() ثابت {
  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEhalf)
    بازگشت convertHalfAPFloatToAPInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semBFloat)
    بازگشت convertBFloatAPFloatToAPInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEsingle)
    بازگشت convertFloatAPFloatToAPInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEdouble)
    بازگشت convertDoubleAPFloatToAPInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEquad)
    بازگشت convertQuadrupleAPFloatToAPInt();

  اگر (semantics ==
      (ثابت llvm::fltSemantics *)&APFloatBase::semX87DoubleExtended)
    بازگشت convertF80LongDoubleAPFloatToAPInt();

  اگر (semantics ==
      (ثابت llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy)
    بازگشت convertPPCDoubleDoubleLegacyAPFloatToAPInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E5M2)
    بازگشت convertFloat8E5M2APFloatToAPInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E5M2FNUZ)
    بازگشت convertFloat8E5M2FNUZAPFloatToAPInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E4M3)
    بازگشت convertFloat8E4M3APFloatToAPInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FN)
    بازگشت convertFloat8E4M3FNAPFloatToAPInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FNUZ)
    بازگشت convertFloat8E4M3FNUZAPFloatToAPInt();

  اگر (semantics ==
      (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E4M3B11FNUZ)
    بازگشت convertFloat8E4M3B11FNUZAPFloatToAPInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E3M4)
    بازگشت convertFloat8E3M4APFloatToAPInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloatTF32)
    بازگشت convertFloatTF32APFloatToAPInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E8M0FNU)
    بازگشت convertFloat8E8M0FNUAPFloatToAPInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat8E5M3FNU)
    بازگشت convertFloat8E5M3FNUAPFloatToAPInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat6E3M2FN)
    بازگشت convertFloat6E3M2FNAPFloatToAPInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat6E2M3FN)
    بازگشت convertFloat6E2M3FNAPFloatToAPInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semFloat4E2M1FN)
    بازگشت convertFloat4E2M1FNAPFloatToAPInt();

  llvm_unreachable("unknown format!");
}

اعشاری IEEEFloat::convertToFloat() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcastToAPInt();
  بازگشت api.bitsToFloat();
}

دوگانه IEEEFloat::convertToDouble() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcastToAPInt();
  بازگشت api.bitsToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 IEEEFloat::convertToQuad() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloatBase::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcastToAPInt();
  بازگشت api.bitsToQuad();
}
#endif

تهی IEEEFloat::initFromF80LongDoubleAPInt(ثابت APInt &api) {
  بازگشت initFromIEEEAPInt<APFloatBase::semX87DoubleExtended>(api);
}

تهی IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(ثابت APInt &api) {
  uint64_t i1 = api.getRawData()[0];
  uint64_t i2 = api.getRawData()[1];
  منطقی losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_unused]] opStatus fs = convert(APFloatBase::semPPCDoubleDoubleLegacy,
                                         rmNearestTiesToEven, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  أكد((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  اگر (isFiniteNonZero()) {
    IEEEFloat v(APFloatBase::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    اگر (v.isFinite()) {
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
تهی IEEEFloat::initFromFloat8E8M0FNUAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E8M0FNU>(api);
}

تهی IEEEFloat::initFromFloat8E5M3FNUAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M3FNU>(api);
}

template <ثابت fltSemantics &S>
تهی IEEEFloat::initFromIEEEAPInt(ثابت APInt &api) {
  أكد(api.getBitWidth() == S.sizeInBits);

  constexpr unsigned صحیح trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr integerPart integer_bit =
      integerPart{1} << (trailing_significand_bits % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned صحیح exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr unsigned صحیح stored_significand_parts =
      part_شمار_For_Bits(trailing_significand_bits + 1);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr منطقی is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr صحیح bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr منطقی has_significand = trailing_significand_bits > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  اگر constexpr (has_significand) {
    std::copy_n(api.getRawData(), mysignificand.size(), mysignificand.begin());
    اگر constexpr (significand_mask != 0 || S.precision >= integerPartWidth) {
      mysignificand[mysignificand.size() - 1] &= significand_mask;
    }
  } وگرنه {
    std::fill_n(mysignificand.begin(), mysignificand.size(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t last_word = api.getRawData()[api.get_رقم_واژگان() - 1];
  uint64_t myexponent =
      (last_word >> (trailing_significand_bits % 64)) & exponent_mask;

  initialize(&S);
  أكد(part_شمار() == mysignificand.size());

  sign = S.hasSignedRepr
             ? static_cast<unsigned صحیح>(last_word >> ((S.sizeInBits - 1) % 64))
             : 0;

  منطقی all_zero_significand =
      has_significand && llvm::all_of(mysignificand, equal_to(0));

  منطقی is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  اگر constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    منطقی is_inf = نادرست;

    اگر constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;
      ثابت integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_mask_no_int_bit) == 0;
    } وگرنه {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    اگر (is_inf) {
      makeInf(sign);
      بازگشت;
    }
  }

  منطقی is_nan = نادرست;

  اگر constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    اگر constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      ثابت integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;

      اگر (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_mask_no_int_bit) != 0) {
        // regular NaN and pseudoNaN
        is_nan = درست;
      } وگرنه اگر (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_mask_no_int_bit) == 0) {
        // pseudoinfinity
        is_nan = درست;
      } وگرنه اگر (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = درست;
      }
    } وگرنه {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } وگرنه اگر constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    منطقی all_ones_significand =
        std::all_of(mysignificand.begin(), mysignificand.end() - 1,
                    [](integerPart bits) { بازگشت bits == ~integerPart{0}; }) &&
        (!significand_mask ||
         mysignificand[mysignificand.size() - 1] == significand_mask);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } وگرنه اگر constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  اگر (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.size(),
                significandParts());
    بازگشت;
  }

  اگر (is_zero) {
    makeZero(sign);
    بازگشت;
  }

  category = fcNormal;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.size(), significandParts());
  اگر (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  وگرنه {
    اگر constexpr (!S.hasExplicitIntegerBit) {
      significandParts()[mysignificand.size() - 1] |= integer_bit;
    }
  }
}

تهی IEEEFloat::initFromQuadrupleAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEquad>(api);
}

تهی IEEEFloat::initFromDoubleAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEdouble>(api);
}

تهی IEEEFloat::initFromFloatAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEsingle>(api);
}

تهی IEEEFloat::initFromBFloatAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semBFloat>(api);
}

تهی IEEEFloat::initFromHalfAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEhalf>(api);
}

تهی IEEEFloat::initFromFloat8E5M2APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2>(api);
}

تهی IEEEFloat::initFromFloat8E5M2FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2FNUZ>(api);
}

تهی IEEEFloat::initFromFloat8E4M3APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3>(api);
}

تهی IEEEFloat::initFromFloat8E4M3FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FN>(api);
}

تهی IEEEFloat::initFromFloat8E4M3FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FNUZ>(api);
}

تهی IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3B11FNUZ>(api);
}

تهی IEEEFloat::initFromFloat8E3M4APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E3M4>(api);
}

تهی IEEEFloat::initFromFloatTF32APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloatTF32>(api);
}

تهی IEEEFloat::initFromFloat6E3M2FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E3M2FN>(api);
}

تهی IEEEFloat::initFromFloat6E2M3FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E2M3FN>(api);
}

تهی IEEEFloat::initFromFloat4E2M1FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
تهی IEEEFloat::initFromAPInt(ثابت fltSemantics *Sem, ثابت APInt &api) {
  أكد(api.getBitWidth() == Sem->sizeInBits);
  اگر (Sem == &APFloatBase::semIEEEhalf)
    بازگشت initFromHalfAPInt(api);
  اگر (Sem == &APFloatBase::semBFloat)
    بازگشت initFromBFloatAPInt(api);
  اگر (Sem == &APFloatBase::semIEEEsingle)
    بازگشت initFromFloatAPInt(api);
  اگر (Sem == &APFloatBase::semIEEEdouble)
    بازگشت initFromDoubleAPInt(api);
  اگر (Sem == &APFloatBase::semIEEEquad)
    بازگشت initFromQuadrupleAPInt(api);
  اگر (Sem == &APFloatBase::semX87DoubleExtended)
    بازگشت initFromF80LongDoubleAPInt(api);
  اگر (Sem == &APFloatBase::semPPCDoubleDoubleLegacy)
    بازگشت initFromPPCDoubleDoubleLegacyAPInt(api);
  اگر (Sem == &APFloatBase::semFloat8E5M2)
    بازگشت initFromFloat8E5M2APInt(api);
  اگر (Sem == &APFloatBase::semFloat8E5M2FNUZ)
    بازگشت initFromFloat8E5M2FNUZAPInt(api);
  اگر (Sem == &APFloatBase::semFloat8E4M3)
    بازگشت initFromFloat8E4M3APInt(api);
  اگر (Sem == &APFloatBase::semFloat8E4M3FN)
    بازگشت initFromFloat8E4M3FNAPInt(api);
  اگر (Sem == &APFloatBase::semFloat8E4M3FNUZ)
    بازگشت initFromFloat8E4M3FNUZAPInt(api);
  اگر (Sem == &APFloatBase::semFloat8E4M3B11FNUZ)
    بازگشت initFromFloat8E4M3B11FNUZAPInt(api);
  اگر (Sem == &APFloatBase::semFloat8E3M4)
    بازگشت initFromFloat8E3M4APInt(api);
  اگر (Sem == &APFloatBase::semFloatTF32)
    بازگشت initFromFloatTF32APInt(api);
  اگر (Sem == &APFloatBase::semFloat8E8M0FNU)
    بازگشت initFromFloat8E8M0FNUAPInt(api);
  اگر (Sem == &APFloatBase::semFloat8E5M3FNU)
    بازگشت initFromFloat8E5M3FNUAPInt(api);
  اگر (Sem == &APFloatBase::semFloat6E3M2FN)
    بازگشت initFromFloat6E3M2FNAPInt(api);
  اگر (Sem == &APFloatBase::semFloat6E2M3FN)
    بازگشت initFromFloat6E2M3FNAPInt(api);
  اگر (Sem == &APFloatBase::semFloat4E2M1FN)
    بازگشت initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
تهی IEEEFloat::makeLargest(منطقی Negative) {
  اگر (Negative && !semantics->hasSignedRepr)
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
  unsigned Part_شمار = part_شمار();
  memset(significand, 0xFF, اندازه(integerPart)*(Part_شمار - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  ثابت unsigned رقم_Unused_بالا_Bits =
    Part_شمار*integerPartWidth - semantics->precision;
  significand[Part_شمار - 1] = (رقم_Unused_بالا_Bits < integerPartWidth)
                                   ? (~integerPart(0) >> رقم_Unused_بالا_Bits)
                                   : 0;
  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
تهی IEEEFloat::makeSmallest(منطقی Negative) {
  اگر (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fcNormal;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tcSet(significandParts(), 1, part_شمار());
}

تهی IEEEFloat::makeSmallestNormalized(منطقی Negative) {
  اگر (Negative && !semantics->hasSignedRepr)
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

IEEEFloat::IEEEFloat(اعشاری f) {
  initFromAPInt(&APFloatBase::semIEEEsingle, APInt::floatToBits(f));
}

IEEEFloat::IEEEFloat(دوگانه d) {
  initFromAPInt(&APFloatBase::semIEEEdouble, APInt::doubleToBits(d));
}

فضاینام {
  تهی append(SmallVectorImpl<نویسه> &Buffer, StringRef Str) {
    Buffer.append(Str.begin(), Str.end());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  تهی AdjustToPrecision(APInt &significand,
                         صحیح &exp, unsigned FormatPrecision) {
    unsigned bits = significand.getActiveBits();

    // 196/59 is a very slight overestimate of lg_2(10).
    unsigned bitsRequired = (FormatPrecision * 196 + 58) / 59;

    اگر (bits <= bitsRequired) بازگشت;

    unsigned tensRemovable = (bits - bitsRequired) * 59 / 196;
    اگر (!tensRemovable) بازگشت;

    exp += tensRemovable;

    APInt مقسوم_علیه(significand.getBitWidth(), 1);
    APInt powten(significand.getBitWidth(), 10);
    وقتی (درست) {
      اگر (tensRemovable & 1)
        مقسوم_علیه *= powten;
      tensRemovable >>= 1;
      اگر (!tensRemovable) بشکن;
      powten *= powten;
    }

    significand = significand.udiv(مقسوم_علیه);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.getActiveBits());
  }


  تهی AdjustToPrecision(SmallVectorImpl<نویسه> &buffer,
                         صحیح &exp, unsigned FormatPrecision) {
    unsigned N = buffer.size();
    اگر (N <= FormatPrecision) بازگشت;

    // The most significant figures are the last ones in the buffer.
    unsigned FirstSignificant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    اگر (buffer[FirstSignificant - 1] < '5') {
      وقتی (FirstSignificant < N && buffer[FirstSignificant] == '0')
        FirstSignificant++;

      exp += FirstSignificant;
      buffer.erase(&buffer[0], &buffer[FirstSignificant]);
      بازگشت;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    برای (unsigned I = FirstSignificant; I != N; ++I) {
      اگر (buffer[I] == '9') {
        FirstSignificant++;
      } وگرنه {
        buffer[I]++;
        بشکن;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    اگر (FirstSignificant == N) {
      exp += FirstSignificant;
      buffer.clear();
      buffer.افزودن_back('1');
      بازگشت;
    }

    exp += FirstSignificant;
    buffer.erase(&buffer[0], &buffer[FirstSignificant]);
  }

  تهی toStringImpl(SmallVectorImpl<نویسه> &Str, ثابت منطقی isNeg, صحیح exp,
                    APInt significand, unsigned FormatPrecision,
                    unsigned FormatMaxPadding, منطقی TruncateZero) {
    ثابت صحیح semanticsPrecision = significand.getBitWidth();

    اگر (isNeg)
      Str.افزودن_back('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    اگر (!FormatPrecision) {
      // We use enough digits so the number can be round-tripped back to an
      // APFloat. The formula comes from "How to Print Floating-Point Numbers
      // Accurately" by Steele and White.
      // FIXME: Using a formula based purely on the precision is conservative;
      // we can print fewer digits depending on the actual value being printed.

      // FormatPrecision = 2 + floor(significandBits / lg_2(10))
      FormatPrecision = 2 + semanticsPrecision * 59 / 196;
    }

    // Ignore trailing binary zeros.
    صحیح trailingZeros = significand.countr_zero();
    exp += trailingZeros;
    significand.lshrInPlace(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    اگر (exp == 0) {
      // Nothing to do.
    } وگرنه اگر (exp > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + exp);
      significand <<= exp;
      exp = 0;
    } وگرنه { /* exp < 0 */
      صحیح texp = -exp;

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
      وقتی (درست) {
        اگر (texp & 1)
          significand *= five_to_the_i;

        texp >>= 1;
        اگر (!texp)
          بشکن;
        five_to_the_i *= five_to_the_i;
      }
    }

    AdjustToPrecision(significand, exp, FormatPrecision);

    SmallVector<نویسه, 256> buffer;

    // Fill the buffer.
    unsigned precision = significand.getBitWidth();
    اگر (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    منطقی inTrail = درست;
    وقتی (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      unsigned d = digit.get_ZExt_مقدار();

      // Drop trailing zeros.
      اگر (inTrail && !d)
        exp++;
      وگرنه {
        buffer.افزودن_back((نویسه) ('0' + d));
        inTrail = نادرست;
      }
    }

    أكد(!buffer.empty() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    AdjustToPrecision(buffer, exp, FormatPrecision);

    unsigned NDigits = buffer.size();

    // Check whether we should use scientific notation.
    منطقی FormatScientific;
    اگر (!FormatMaxPadding) {
      FormatScientific = درست;
    } وگرنه {
      اگر (exp >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((unsigned) exp > FormatMaxPadding ||
                            NDigits + (unsigned) exp > FormatPrecision);
      } وگرنه {
        // Power of the most significant digit.
        صحیح MSD = exp + (صحیح) (NDigits - 1);
        اگر (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = نادرست;
        } وگرنه {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((unsigned) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    اگر (FormatScientific) {
      exp += (NDigits - 1);

      Str.افزودن_back(buffer[NDigits-1]);
      Str.افزودن_back('.');
      اگر (NDigits == 1 && TruncateZero)
        Str.افزودن_back('0');
      وگرنه
        برای (unsigned I = 1; I != NDigits; ++I)
          Str.افزودن_back(buffer[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      اگر (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.افزودن_back(TruncateZero ? 'E' : 'e');

      Str.افزودن_back(exp >= 0 ? '+' : '-');
      اگر (exp < 0)
        exp = -exp;
      SmallVector<نویسه, 6> expbuf;
      انجامبده {
        expbuf.افزودن_back((نویسه) ('0' + (exp % 10)));
        exp /= 10;
      } وقتی (exp);
      // Exponent always at least two digits if we do not truncate zeros.
      اگر (!TruncateZero && expbuf.size() < 2)
        expbuf.افزودن_back('0');
      برای (unsigned I = 0, E = expbuf.size(); I != E; ++I)
        Str.افزودن_back(expbuf[E-1-I]);
      بازگشت;
    }

    // Non-scientific, positive exponents.
    اگر (exp >= 0) {
      برای (unsigned I = 0; I != NDigits; ++I)
        Str.افزودن_back(buffer[NDigits-1-I]);
      برای (unsigned I = 0; I != (unsigned) exp; ++I)
        Str.افزودن_back('0');
      بازگشت;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    صحیح NWholeDigits = exp + (صحیح) NDigits;

    unsigned I = 0;
    اگر (NWholeDigits > 0) {
      برای (; I != (unsigned) NWholeDigits; ++I)
        Str.افزودن_back(buffer[NDigits-I-1]);
      Str.افزودن_back('.');
    } وگرنه {
      unsigned NZeros = 1 + (unsigned) -NWholeDigits;

      Str.افزودن_back('0');
      Str.افزودن_back('.');
      برای (unsigned Z = 1; Z != NZeros; ++Z)
        Str.افزودن_back('0');
    }

    برای (; I != NDigits; ++I)
      Str.افزودن_back(buffer[NDigits-I-1]);

  }
} // namespace

تهی IEEEFloat::toString(SmallVectorImpl<نویسه> &Str, unsigned FormatPrecision,
                         unsigned FormatMaxPadding, منطقی TruncateZero) ثابت {
  انتخاب (category) {
  حالت fcInfinity:
    اگر (isNegative())
      بازگشت append(Str, "-Inf");
    وگرنه
      بازگشت append(Str, "+Inf");

  حالت fcNaN: بازگشت append(Str, "NaN");

  حالت fcZero:
    اگر (isNegative())
      Str.افزودن_back('-');

    اگر (!FormatMaxPadding) {
      اگر (TruncateZero)
        append(Str, "0.0E+0");
      وگرنه {
        append(Str, "0.0");
        اگر (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } وگرنه {
      Str.افزودن_back('0');
    }
    بازگشت;

  حالت fcNormal:
    بشکن;
  }

  // Decompose the number into an APInt and an exponent.
  صحیح exp = exponent - ((صحیح) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_شمار_For_Bits(semantics->precision)));

  toStringImpl(Str, isNegative(), exp, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

صحیح IEEEFloat::getExactLog2Abs() ثابت {
  اگر (!isFinite() || isZero())
    بازگشت INT_MIN;

  ثابت integerPart *Parts = significandParts();
  ثابت صحیح Part_شمار = part_شمار_For_Bits(semantics->precision);

  صحیح برداشت_شمار = 0;
  برای (صحیح i = 0; i < Part_شمار; ++i) {
    برداشت_شمار += llvm::popcount(Parts[i]);
    اگر (برداشت_شمار > 1)
      بازگشت INT_MIN;
  }

  اگر (exponent != semantics->minExponent)
    بازگشت exponent;

  صحیح CountrParts = 0;
  برای (صحیح i = 0; i < Part_شمار;
       ++i, CountrParts += APInt::APINT_BITS_PER_WORD) {
    اگر (Parts[i] != 0) {
      بازگشت exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

منطقی IEEEFloat::isSignaling() ثابت {
  اگر (!isNaN())
    بازگشت نادرست;
  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    بازگشت نادرست;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  بازگشت !APInt::tcExtractBit(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::opStatus IEEEFloat::next(منطقی nextDown) {
  // If we are performing nextDown, swap sign so we have -x.
  اگر (nextDown)
    changeSign();

  // Compute nextUp(x)
  opStatus result = opOK;

  // Handle each float category separately.
  انتخاب (category) {
  حالت fcInfinity:
    // nextUp(+inf) = +inf
    اگر (!isNegative())
      بشکن;
    // nextUp(-inf) = -getLargest()
    makeLargest(درست);
    بشکن;
  حالت fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    اگر (isSignaling()) {
      result = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(نادرست, isNegative(), nullptr);
    }
    بشکن;
  حالت fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(نادرست);
    بشکن;
  حالت fcNormal:
    // nextUp(-getSmallest()) = -0
    اگر (isSmallest() && isNegative()) {
      APInt::tcSet(significandParts(), 0, part_شمار());
      category = fcZero;
      exponent = 0;
      اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = نادرست;
      اگر (!semantics->hasZero)
        makeSmallestNormalized(نادرست);
      بشکن;
    }

    اگر (isLargest() && !isNegative()) {
      اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        بشکن;
      } وگرنه اگر (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        بشکن;
      } وگرنه {
        // nextUp(getLargest()) == INFINITY
        APInt::tcSet(significandParts(), 0, part_شمار());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        بشکن;
      }
    }

    // nextUp(normal) == normal + inc.
    اگر (isNegative()) {
      // If we are negative, we need to decrement the significand.

      // We only cross a binade boundary that requires adjusting the exponent
      // if:
      //   1. exponent != semantics->minExponent. This implies we are not in the
      //   smallest binade or are dealing with denormals.
      //   2. Our significand excluding the integral bit is all zeros.
      منطقی WillCrossBinadeBoundary =
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
      APInt::tcDecrement(Parts, part_شمار());

      اگر (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tcSetBit(Parts, semantics->precision - 1);
        exponent--;
      }
    } وگرنه {
      // If we are positive, we need to increment the significand.

      // We only cross a binade boundary that requires adjusting the exponent if
      // the input is not a denormal and all of said input's significand bits
      // are set. If all of said conditions are true: clear the significand, set
      // the integral bit to 1, and increment the exponent. If we have a
      // denormal always increment since moving denormals and the numbers in the
      // smallest normal binade have the same exponent in our representation.
      // If there are only exponents, any increment always crosses the
      // BinadeBoundary.
      منطقی WillCrossBinadeBoundary = !APFloat::hasSignificand(*semantics) ||
                                     (!isDenormal() && isSignificandAllOnes());

      اگر (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tcSet(Parts, 0, part_شمار());
        APInt::tcSetBit(Parts, semantics->precision - 1);
        أكد(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } وگرنه {
        incrementSignificand();
      }
    }
    بشکن;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  اگر (nextDown)
    changeSign();

  بازگشت result;
}

APInt IEEEFloat::getNaNPayload() ثابت {
  أكد(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  unsigned Bits = semantics->precision - 1;
  بازگشت APInt(Bits, ArrayRef(significandParts(), part_شمار_For_Bits(Bits)));
}

APFloatBase::ExponentType IEEEFloat::exponentNaN() ثابت {
  بازگشت ::exponentNaN(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentInf() ثابت {
  بازگشت ::exponentInf(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentZero() ثابت {
  بازگشت ::exponentZero(*semantics);
}

تهی IEEEFloat::makeInf(منطقی Negative) {
  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(نادرست, Negative);
    بازگشت;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tcSet(significandParts(), 0, part_شمار());
}

تهی IEEEFloat::makeZero(منطقی Negative) {
  اگر (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = نادرست;
  }
  exponent = exponentZero();
  APInt::tcSet(significandParts(), 0, part_شمار());
}

تهی IEEEFloat::makeQuiet() {
  أكد(isNaN());
  اگر (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tcSetBit(significandParts(), semantics->precision - 2);
}

صحیح ilogb(ثابت IEEEFloat &Arg) {
  اگر (Arg.isNaN())
    بازگشت APFloat::IEK_NaN;
  اگر (Arg.isZero())
    بازگشت APFloat::IEK_Zero;
  اگر (Arg.isInfinity())
    بازگشت APFloat::IEK_Inf;
  اگر (!Arg.isDenormal())
    بازگشت Arg.exponent;

  IEEEFloat Normalized(Arg);
  صحیح SignificandBits = Arg.getSemantics().precision - 1;

  Normalized.exponent += SignificandBits;
  Normalized.normalize(APFloat::rmNearestTiesToEven, lfExactlyZero);
  بازگشت Normalized.exponent - SignificandBits;
}

IEEEFloat scalbn(IEEEFloat X, صحیح Exp, roundingMode RoundingMode) {
  auto MaxExp = X.getSemantics().maxExponent;
  auto MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  صحیح SignificandBits = X.getSemantics().precision - 1;
  صحیح MaxIncrement = MaxExp - (MinExp - SignificandBits) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(RoundingMode, lfExactlyZero);
  اگر (X.isNaN())
    X.makeQuiet();
  بازگشت X;
}

IEEEFloat frexp(ثابت IEEEFloat &Val, صحیح &Exp, roundingMode RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  اگر (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    بازگشت Quiet;
  }

  اگر (Exp == APFloat::IEK_Inf)
    بازگشت Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  بازگشت scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S)
    : Semantics(&S),
      Floats(جدید APFloat[2]{APFloat(APFloatBase::semIEEEdouble),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, uninitializedTag)
    : Semantics(&S), Floats(جدید APFloat[2]{
                         APFloat(APFloatBase::semIEEEdouble, uninitialized),
                         APFloat(APFloatBase::semIEEEdouble, uninitialized)}) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(جدید APFloat[2]{APFloat(APFloatBase::semIEEEdouble, I),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, ثابت APInt &I)
    : Semantics(&S),
      Floats(جدید APFloat[2]{
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[0])),
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[1]))}) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, APFloat &&First,
                             APFloat &&Second)
    : Semantics(&S),
      Floats(جدید APFloat[2]{std::move(First), std::move(Second)}) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble);
  أكد(&Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  أكد(&Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? جدید APFloat[2]{APFloat(RHS.Floats[0]),
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
  اگر (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } وگرنه اگر (این != &RHS) {
    این->~DoubleAPFloat();
    جدید (این) DoubleAPFloat(RHS);
  }
  بازگشت *این;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
ایستا std::pair<APFloat, APFloat> fastTwoSum(APFloat X, APFloat Y) {
  اگر (!X.isFinite())
    بازگشت {X, APFloat::getZero(X.getSemantics(), /*Negative=*/نادرست)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  بازگشت {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::opStatus DoubleAPFloat::addImpl(ثابت APFloat &a, ثابت APFloat &aa,
                                         ثابت APFloat &c, ثابت APFloat &cc,
                                         roundingMode RM) {
  صحیح Status = opOK;
  APFloat z = a;
  Status |= z.add(c, RM);
  اگر (!z.isFinite()) {
    اگر (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ نادرست);
      بازگشت (opStatus)Status;
    }
    Status = opOK;
    auto AComparedToC = a.compare_Absolute_مقدار(c);
    z = cc;
    Status |= z.add(aa, RM);
    اگر (AComparedToC == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      Status |= z.add(c, RM);
      Status |= z.add(a, RM);
    } وگرنه {
      // z = cc + aa + a + c;
      Status |= z.add(a, RM);
      Status |= z.add(c, RM);
    }
    اگر (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ نادرست);
      بازگشت (opStatus)Status;
    }
    Floats[0] = z;
    APFloat zz = aa;
    Status |= zz.add(cc, RM);
    اگر (AComparedToC == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(c, RM);
      Status |= Floats[1].add(zz, RM);
    } وگرنه {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(a, RM);
      Status |= Floats[1].add(zz, RM);
    }
  } وگرنه {
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
    اگر (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ نادرست);
      بازگشت opOK;
    }
    Floats[0] = z;
    Status |= Floats[0].add(zz, RM);
    اگر (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ نادرست);
      بازگشت (opStatus)Status;
    }
    Floats[1] = std::move(z);
    Status |= Floats[1].subtract(Floats[0], RM);
    Status |= Floats[1].add(zz, RM);
  }
  بازگشت (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::addWithSpecial(ثابت DoubleAPFloat &LHS,
                                                ثابت DoubleAPFloat &RHS,
                                                DoubleAPFloat &Out,
                                                roundingMode RM) {
  اگر (LHS.getCategory() == fcNaN) {
    Out = LHS;
    بازگشت opOK;
  }
  اگر (RHS.getCategory() == fcNaN) {
    Out = RHS;
    بازگشت opOK;
  }
  اگر (LHS.getCategory() == fcZero) {
    Out = RHS;
    بازگشت opOK;
  }
  اگر (RHS.getCategory() == fcZero) {
    Out = LHS;
    بازگشت opOK;
  }
  اگر (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    Out.makeNaN(نادرست, Out.isNegative(), nullptr);
    بازگشت opInvalidOp;
  }
  اگر (LHS.getCategory() == fcInfinity) {
    Out = LHS;
    بازگشت opOK;
  }
  اگر (RHS.getCategory() == fcInfinity) {
    Out = RHS;
    بازگشت opOK;
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
  بازگشت Out.addImpl(A, AA, C, CC, RM);
}

APFloat::opStatus DoubleAPFloat::add(ثابت DoubleAPFloat &RHS,
                                     roundingMode RM) {
  بازگشت addWithSpecial(*این, RHS, *این, RM);
}

APFloat::opStatus DoubleAPFloat::subtract(ثابت DoubleAPFloat &RHS,
                                          roundingMode RM) {
  changeSign();
  auto Ret = add(RHS, RM);
  changeSign();
  بازگشت Ret;
}

APFloat::opStatus DoubleAPFloat::multiply(ثابت DoubleAPFloat &RHS,
                                          APFloat::roundingMode RM) {
  ثابت auto &LHS = *این;
  auto &Out = *این;
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
  اگر (LHS.getCategory() == fcNaN) {
    Out = LHS;
    بازگشت opOK;
  }
  اگر (RHS.getCategory() == fcNaN) {
    Out = RHS;
    بازگشت opOK;
  }
  اگر ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    Out.makeNaN(نادرست, نادرست, nullptr);
    بازگشت opOK;
  }
  اگر (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    Out = LHS;
    بازگشت opOK;
  }
  اگر (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    Out = RHS;
    بازگشت opOK;
  }
  أكد(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal &&
         "Special cases not handled exhaustively");

  صحیح Status = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  Status |= T.multiply(C, RM);
  اگر (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ نادرست);
    بازگشت (opStatus)Status;
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
  اگر (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ نادرست);
  } وگرنه {
    // Floats[1] = (t - u) + tau
    Status |= T.subtract(U, RM);
    Status |= T.add(Tau, RM);
    Floats[1] = std::move(T);
  }
  بازگشت (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::divide(ثابت DoubleAPFloat &RHS,
                                        APFloat::roundingMode RM) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.divide(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()), RM);
  *این = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  بازگشت Ret;
}

APFloat::opStatus DoubleAPFloat::remainder(ثابت DoubleAPFloat &RHS) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.remainder(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *این = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  بازگشت Ret;
}

APFloat::opStatus DoubleAPFloat::mod(ثابت DoubleAPFloat &RHS) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.mod(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *این = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  بازگشت Ret;
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
  *این = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  بازگشت Ret;
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
  اگر (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/نادرست);
    بازگشت HiStatus;
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
    اگر (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        abs(RoundingError).is_Exactly_مقدار(0.5))
      Rounded.add(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rmNearestTiesToEven);
    بازگشت Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  اگر (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    اگر (RM == rmNearestTiesToAway || RM == rmNearestTiesToEven)
      RoundedHi = RoundToNearestHelper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/نادرست);
    بازگشت HiStatus;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  RoundingMode LoRoundingMode;
  اگر (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    LoRoundingMode = isNegative() ? rmTowardPositive : rmTowardNegative;
  وگرنه
    LoRoundingMode = RM;

  APFloat RoundedLo = Lo;
  ثابت opStatus LoStatus = RoundedLo.roundToIntegral(LoRoundingMode);
  اگر (LoRoundingMode == rmNearestTiesToAway)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = RoundToNearestHelper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fastTwoSum(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  بازگشت LoStatus;
}

تهی DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmpResult
DoubleAPFloat::compare_Absolute_مقدار(ثابت DoubleAPFloat &RHS) ثابت {
  // Compare absolute values of the high parts.
  ثابت cmpResult HiPartCmp = Floats[0].compare_Absolute_مقدار(RHS.Floats[0]);
  اگر (HiPartCmp != cmpEqual)
    بازگشت HiPartCmp;

  // Zero, regardless of sign, is equal.
  اگر (Floats[1].isZero() && RHS.Floats[1].isZero())
    بازگشت cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  ثابت منطقی ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  ثابت منطقی RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  اگر (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    بازگشت RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  اگر (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    بازگشت ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  اگر (ThisIsSubtractive != RHSIsSubtractive)
    بازگشت ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  ثابت cmpResult LoPartCmp = Floats[1].compare_Absolute_مقدار(RHS.Floats[1]);

  اگر (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    اگر (LoPartCmp == cmpLessThan)
      بازگشت cmpGreaterThan;
    اگر (LoPartCmp == cmpGreaterThan)
      بازگشت cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  بازگشت LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() ثابت {
  بازگشت Floats[0].getCategory();
}

منطقی DoubleAPFloat::isNegative() ثابت { بازگشت Floats[0].isNegative(); }

تهی DoubleAPFloat::makeInf(منطقی Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ نادرست);
}

تهی DoubleAPFloat::makeZero(منطقی Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ نادرست);
}

تهی DoubleAPFloat::makeLargest(منطقی Neg) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  اگر (Neg)
    changeSign();
}

تهی DoubleAPFloat::makeSmallest(منطقی Neg) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].makeSmallest(Neg);
  Floats[1].makeZero(/* Neg = */ نادرست);
}

تهی DoubleAPFloat::makeSmallestNormalized(منطقی Neg) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  اگر (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ نادرست);
}

تهی DoubleAPFloat::makeNaN(منطقی SNaN, منطقی Neg, ثابت APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ نادرست);
}

APFloat::cmpResult DoubleAPFloat::compare(ثابت DoubleAPFloat &RHS) ثابت {
  auto Result = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  اگر (Result == APFloat::cmpEqual)
    بازگشت Floats[1].compare(RHS.Floats[1]);
  بازگشت Result;
}

منطقی DoubleAPFloat::bitwiseIsEqual(ثابت DoubleAPFloat &RHS) ثابت {
  بازگشت Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_code hash_مقدار(ثابت DoubleAPFloat &Arg) {
  اگر (Arg.Floats)
    بازگشت hash_combine(hash_مقدار(Arg.Floats[0]), hash_مقدار(Arg.Floats[1]));
  بازگشت hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcastToAPInt() ثابت {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t Data[] = {
      Floats[0].bitcastToAPInt().getRawData()[0],
      Floats[1].bitcastToAPInt().getRawData()[0],
  };
  بازگشت APInt(128, Data);
}

Expected<APFloat::opStatus> DoubleAPFloat::convertFromString(StringRef S,
                                                             roundingMode RM) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy);
  auto Ret = Tmp.convertFromString(S, RM);
  *این = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  بازگشت Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::opStatus DoubleAPFloat::next(منطقی nextDown) {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  اگر (nextDown) {
    changeSign();
    APFloat::opStatus Result = next(/*nextDown=*/نادرست);
    changeSign();
    بازگشت Result;
  }
  انتخاب (getCategory()) {
  حالت fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    اگر (isNegative())
      makeLargest(درست);
    بازگشت opOK;

  حالت fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    اگر (getFirst().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(نادرست, isNegative(), nullptr);
      بازگشت opInvalidOp;
    }
    بازگشت opOK;

  حالت fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(نادرست);
    بازگشت opOK;

  حالت fcNormal:
    بشکن;
  }

  ثابت APFloat &HiOld = getFirst();
  ثابت APFloat &LoOld = getSecond();

  APFloat NextLo = LoOld;
  NextLo.next(/*nextDown=*/نادرست);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  auto InLattice = [](ثابت APFloat &Hi, ثابت APFloat &Lo) {
    بازگشت Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  اگر (InLattice(HiOld, NextLo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(NextLo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat Largest{*Semantics, uninitialized};
    Largest.makeLargest(/*Neg=*/نادرست);
    اگر (compare(Largest) == cmpGreaterThan)
      makeInf(/*Neg=*/نادرست);

    بازگشت opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat NextHi = HiOld;
  NextHi.next(/*nextDown=*/نادرست);

  // nextUp(getLargest()) == INFINITY
  اگر (NextHi.isInfinity()) {
    makeInf(/*Neg=*/نادرست);
    بازگشت opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  اگر (NextHi.isZero()) {
    makeZero(/*Neg=*/درست);
    بازگشت opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  NextLo = neg(scalbn(harrisonUlp(NextHi), -1, rmTowardZero));
  اگر (!InLattice(NextHi, NextLo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    NextLo.next(/*nextDown=*/نادرست);

  Floats[0] = std::move(NextHi);
  Floats[1] = std::move(NextLo);

  بازگشت opOK;
}

APFloat::opStatus DoubleAPFloat::convertToSignExtendedInteger(
    MutableArrayRef<integerPart> درون_داد, unsigned صحیح Width, منطقی IsSigned,
    roundingMode RM, منطقی *IsExact) ثابت {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  اگر (!getFirst().isFiniteNonZero() || getSecond().isZero())
    بازگشت getFirst().convertToInteger(درون_داد, Width, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = نادرست;
  DoubleAPFloat Integral = *این;
  ثابت opStatus RoundStatus = Integral.roundToIntegral(RM);
  اگر (RoundStatus == opInvalidOp)
    بازگشت opInvalidOp;
  ثابت APFloat &IntegralHi = Integral.getFirst();
  ثابت APFloat &IntegralLo = Integral.getSecond();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  منطقی HiIsExact;
  اگر (IntegralHi.isZero() || IntegralLo.isZero()) {
    ثابت opStatus HiStatus =
        IntegralHi.convertToInteger(درون_داد, Width, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    بازگشت HiStatus == opOK ? opInexact : HiStatus;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  اگر (!IsSigned && IntegralHi.isNegative())
    بازگشت opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  منطقی LoIsExact;
  ثابت صحیح HiExactLog2 = IntegralHi.getExactLog2Abs();
  ثابت unsigned PositiveOverflowWidth = IsSigned ? Width - 1 : Width;
  اگر (HiExactLog2 >= 0 &&
      static_cast<unsigned>(HiExactLog2) == PositiveOverflowWidth) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    اگر (IntegralHi.isNegative() == IntegralLo.isNegative())
      بازگشت opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    ثابت opStatus LoStatus = IntegralLo.convertToInteger(
        درون_داد, Width, /*IsSigned=*/درست, RM, &LoIsExact);
    اگر (LoStatus == opInvalidOp)
      بازگشت opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    اگر (IsSigned && !IntegralHi.isNegative())
      APInt::tcClearBit(درون_داد.data(), PositiveOverflowWidth);
    *IsExact = RoundStatus == opOK;
    بازگشت RoundStatus;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  ثابت opStatus HiStatus = IntegralHi.convertToInteger(
      درون_داد, Width, IsSigned, rmTowardZero, &HiIsExact);
  اگر (HiStatus == opInvalidOp)
    بازگشت HiStatus;

  // Convert Lo into a temporary integer of the same width.
  APSInt LoResult{Width, /*isUnsigned=*/!IsSigned};
  ثابت opStatus LoStatus =
      IntegralLo.convertToInteger(LoResult, rmTowardZero, &LoIsExact);
  اگر (LoStatus == opInvalidOp)
    بازگشت LoStatus;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tcAdd(درون_داد.data(), LoResult.getRawData(), /*carry=*/0, درون_داد.size());

  *IsExact = RoundStatus == opOK;
  بازگشت RoundStatus;
}

APFloat::opStatus
DoubleAPFloat::convertToInteger(MutableArrayRef<integerPart> درون_داد,
                                unsigned صحیح Width, منطقی IsSigned,
                                roundingMode RM, منطقی *IsExact) ثابت {
  opStatus FS =
      convertToSignExtendedInteger(درون_داد, Width, IsSigned, RM, IsExact);

  اگر (FS == opInvalidOp) {
    ثابت unsigned Dst_Parts_شمار = part_شمار_For_Bits(Width);
    أكد(Dst_Parts_شمار <= درون_داد.size() && "Integer too big");

    unsigned Bits;
    اگر (getCategory() == fcNaN)
      Bits = 0;
    وگرنه اگر (isNegative())
      Bits = IsSigned;
    وگرنه
      Bits = Width - IsSigned;

    tcSetLeastSignificantBits(درون_داد.data(), Dst_Parts_شمار, Bits);
    اگر (isNegative() && IsSigned)
      APInt::tc_Shift_چپ(درون_داد.data(), Dst_Parts_شمار, Width - 1);
  }

  بازگشت FS;
}

APFloat::opStatus DoubleAPFloat::handleOverflow(roundingMode RM) {
  انتخاب (RM) {
  حالت APFloat::rmTowardZero:
    makeLargest(/*Neg=*/isNegative());
    بشکن;
  حالت APFloat::rmTowardNegative:
    اگر (isNegative())
      makeInf(/*Neg=*/درست);
    وگرنه
      makeLargest(/*Neg=*/نادرست);
    بشکن;
  حالت APFloat::rmTowardPositive:
    اگر (isNegative())
      makeLargest(/*Neg=*/درست);
    وگرنه
      makeInf(/*Neg=*/نادرست);
    بشکن;
  حالت APFloat::rmNearestTiesToAway:
  حالت APFloat::rmNearestTiesToEven:
    makeInf(/*Neg=*/isNegative());
    بشکن;
  پیشفرض:
    llvm_unreachable("Invalid rounding mode found");
  }
  opStatus S = opInexact;
  اگر (!getFirst().isFinite())
    S = static_cast<opStatus>(S | opOverflow);
  بازگشت S;
}

APFloat::opStatus DoubleAPFloat::convertFromUnsignedParts(
    ثابت integerPart *Src, unsigned صحیح Src_شمار, roundingMode RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  ثابت unsigned SrcMSB = APInt::tcMSB(Src, Src_شمار);
  اگر (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/نادرست);
    بازگشت opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  ثابت unsigned SrcBitWidth = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/SrcBitWidth, ArrayRef(Src, Src_شمار)},
                /*isUnsigned=*/درست};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{getFirst().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/نادرست, rmNearestTiesToEven);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  اگر (!Hi.isFinite())
    بازگشت handleOverflow(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  منطقی HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/درست};
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
  اگر (Error.isNegative()) {
    اگر (RM == rmNearestTiesToAway) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      ثابت unsigned ErrorActiveBits = Error.getSignificantBits() - 1;
      ثابت unsigned LoPrecision = getSecond().getSemantics().precision;
      اگر (ErrorActiveBits > LoPrecision) {
        ثابت unsigned RoundingBoundary = ErrorActiveBits - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        اگر (Error.شمار_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rmTowardPositive;
      }
    } وگرنه اگر (RM == rmTowardZero) {
      // For rmTowardZero, the final positive result must be truncated (rounded
      // down). When Hi is an overestimate, Error is negative. A standard
      // rmTowardZero conversion of Error would make it *less* negative,
      // effectively rounding the final sum Hi + Lo *up*. To ensure the sum
      // rounds down correctly, we force Lo to round toward -infinity.
      LoRM = rmTowardNegative;
    }
  }

  APFloat Lo{getSecond().getSemantics()};
  opStatus Status = Lo.convertFromAPInt(Error, /*IsSigned=*/درست, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fastTwoSum(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  اگر (!getFirst().isFinite())
    بازگشت handleOverflow(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  اگر (getFirst().isFiniteNonZero() && Floats[0].isLargest()) {
    DoubleAPFloat Largest{*Semantics};
    Largest.makeLargest(/*Neg=*/نادرست);
    اگر (compare(Largest) == APFloat::cmpGreaterThan)
      بازگشت handleOverflow(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  بازگشت Status;
}

APFloat::opStatus DoubleAPFloat::convertFromAPInt(ثابت APInt &درون_داد,
                                                  منطقی IsSigned,
                                                  roundingMode RM) {
  ثابت منطقی Negate_درون_داد = IsSigned && درون_داد.isNegative();
  APInt API = درون_داد;
  اگر (Negate_درون_داد)
    API.negate();

  ثابت APFloat::opStatus Status =
      convertFromUnsignedParts(API.getRawData(), API.get_رقم_واژگان(), RM);
  اگر (Negate_درون_داد)
    changeSign();
  بازگشت Status;
}

unsigned صحیح DoubleAPFloat::convertToHexString(نویسه *DST,
                                               unsigned صحیح HexDigits,
                                               منطقی UpperCase,
                                               roundingMode RM) ثابت {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  بازگشت APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .convertToHexString(DST, HexDigits, UpperCase, RM);
}

منطقی DoubleAPFloat::isDenormal() ثابت {
  بازگشت getCategory() == fcNormal &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

منطقی DoubleAPFloat::isSmallest() ثابت {
  اگر (getCategory() != fcNormal)
    بازگشت نادرست;
  DoubleAPFloat Tmp(*این);
  Tmp.makeSmallest(این->isNegative());
  بازگشت Tmp.compare(*این) == cmpEqual;
}

منطقی DoubleAPFloat::isSmallestNormalized() ثابت {
  اگر (getCategory() != fcNormal)
    بازگشت نادرست;

  DoubleAPFloat Tmp(*این);
  Tmp.makeSmallestNormalized(این->isNegative());
  بازگشت Tmp.compare(*این) == cmpEqual;
}

منطقی DoubleAPFloat::isLargest() ثابت {
  اگر (getCategory() != fcNormal)
    بازگشت نادرست;
  DoubleAPFloat Tmp(*این);
  Tmp.makeLargest(این->isNegative());
  بازگشت Tmp.compare(*این) == cmpEqual;
}

منطقی DoubleAPFloat::isInteger() ثابت {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  بازگشت Floats[0].isInteger() && Floats[1].isInteger();
}

تهی DoubleAPFloat::toString(SmallVectorImpl<نویسه> &Str,
                             unsigned FormatPrecision,
                             unsigned FormatMaxPadding,
                             منطقی TruncateZero) ثابت {
  أكد(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .toString(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

صحیح DoubleAPFloat::getExactLog2Abs() ثابت {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  اگر (getSecond().isNonZero())
    بازگشت INT_MIN;
  بازگشت getFirst().getExactLog2Abs();
}

صحیح ilogb(ثابت DoubleAPFloat &Arg) {
  ثابت APFloat &Hi = Arg.getFirst();
  ثابت APFloat &Lo = Arg.getSecond();
  صحیح IlogbResult = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  اگر (Arg.getCategory() != fcNormal)
    بازگشت IlogbResult;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  اگر (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    بازگشت IlogbResult;
  اگر (Hi.getExactLog2Abs() == INT_MIN)
    بازگشت IlogbResult;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  بازگشت IlogbResult - 1;
}

DoubleAPFloat scalbn(ثابت DoubleAPFloat &Arg, صحیح Exp,
                     APFloat::roundingMode RM) {
  أكد(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");
  بازگشت DoubleAPFloat(APFloatBase::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(ثابت DoubleAPFloat &Arg, صحیح &Exp,
                    APFloat::roundingMode RM) {
  أكد(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  اگر (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.getFirst() = Quiet.getFirst().makeQuiet();
    بازگشت Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  اگر (Exp == APFloat::IEK_Inf)
    بازگشت Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  اگر (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    بازگشت Arg;
  }

  ثابت APFloat &Hi = Arg.getFirst();
  ثابت APFloat &Lo = Arg.getSecond();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  ثابت منطقی SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat Second = Lo;
  اگر (Arg.getCategory() == APFloat::fcNormal && Lo.isFiniteNonZero()) {
    roundingMode LoRoundingMode;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    اگر (RM == rmTowardZero)
      LoRoundingMode = Arg.isNegative() ? rmTowardPositive : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    وگرنه اگر (RM == rmNearestTiesToAway && SignsDisagree && Exp > 0)
      LoRoundingMode = rmNearestTiesToEven;
    وگرنه
      LoRoundingMode = RM;
    Second = scalbn(Lo, -Exp, LoRoundingMode);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    اگر (RM == rmNearestTiesToAway && LoRoundingMode == rmNearestTiesToEven) {
      // Re-scale the result back to check if rounding occurred.
      ثابت APFloat RecomposedLo = scalbn(Second, Exp, rmNearestTiesToEven);
      اگر (RecomposedLo != Lo) {
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
        ثابت منطقی IsMidpoint = abs(RoundingError) == ScaledUlpOfSecond;
        ثابت منطقی RoundedLoAway =
            Second.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        اگر (IsMidpoint && RoundedLoAway)
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
    اگر (Second.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat First = scalbn(Hi, -Exp, RM);
  بازگشت DoubleAPFloat(APFloatBase::PPCDoubleDouble(), std::move(First),
                       std::move(Second));
}

APInt DoubleAPFloat::getNaNPayload() ثابت { بازگشت Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, ثابت fltSemantics &Semantics) {
  اگر (usesLayout<IEEEFloat>(Semantics)) {
    جدید (&IEEE) IEEEFloat(std::move(F));
    بازگشت;
  }
  اگر (usesLayout<DoubleAPFloat>(Semantics)) {
    ثابت fltSemantics& S = F.getSemantics();
    جدید (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloatBase::IEEEdouble()));
    بازگشت;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::opStatus> APFloat::convertFromString(StringRef Str,
                                                       roundingMode RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_code hash_مقدار(ثابت APFloat &Arg) {
  اگر (APFloat::usesLayout<detail::IEEEFloat>(Arg.getSemantics()))
    بازگشت hash_مقدار(Arg.U.IEEE);
  اگر (APFloat::usesLayout<detail::DoubleAPFloat>(Arg.getSemantics()))
    بازگشت hash_مقدار(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(ثابت fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  auto StatusOrErr = convertFromString(S, rmNearestTiesToEven);
  أكد(StatusOrErr && "Invalid floating point representation");
  consumeError(StatusOrErr.takeError());
}

FPClassTest APFloat::classify() ثابت {
  اگر (isZero())
    بازگشت isNegative() ? fcNegZero : fcPosZero;
  اگر (isNormal())
    بازگشت isNegative() ? fcNegNormal : fcPosNormal;
  اگر (isDenormal())
    بازگشت isNegative() ? fcNegSubnormal : fcPosSubnormal;
  اگر (isInfinity())
    بازگشت isNegative() ? fcNegInf : fcPosInf;
  أكد(isNaN() && "Other class of FP constant");
  بازگشت isSignaling() ? fcSNan : fcQNan;
}

منطقی APFloat::getExactInverse(APFloat *Inv) ثابت {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  اگر (!isFiniteNonZero())
    بازگشت نادرست;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  اگر (isDenormal())
    بازگشت نادرست;

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
  ثابت صحیح Exp = getExactLog2Abs();
  اگر (Exp == INT_MIN)
    بازگشت نادرست;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  اگر (Reciprocal.getExactLog2Abs() != -Exp)
    بازگشت نادرست;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  اگر (Reciprocal.isDenormal())
    بازگشت نادرست;

  أكد(Reciprocal.isFiniteNonZero());

  اگر (Inv)
    *Inv = std::move(Reciprocal);

  بازگشت درست;
}

APFloat::opStatus APFloat::convert(ثابت fltSemantics &ToSemantics,
                                   roundingMode RM, منطقی *losesInfo) {
  اگر (&getSemantics() == &ToSemantics) {
    *losesInfo = نادرست;
    بازگشت opOK;
  }
  اگر (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics))
    بازگشت U.IEEE.convert(ToSemantics, RM, losesInfo);
  اگر (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<DoubleAPFloat>(ToSemantics)) {
    أكد(&ToSemantics == &APFloatBase::semPPCDoubleDouble);
    auto Ret =
        U.IEEE.convert(APFloatBase::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *این = APFloat(ToSemantics, U.IEEE.bitcastToAPInt());
    بازگشت Ret;
  }
  اگر (usesLayout<DoubleAPFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics)) {
    auto Ret = getIEEE().convert(ToSemantics, RM, losesInfo);
    *این = APFloat(std::move(getIEEE()), ToSemantics);
    بازگشت Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_مقدار(ثابت fltSemantics &Semantics) {
  بازگشت APFloat(Semantics, APInt::getAllOnes(Semantics.sizeInBits));
}

تهی APFloat::print(raw_ostream &OS) ثابت {
  SmallVector<نویسه, 16> Buffer;
  toString(Buffer);
  OS << Buffer;
}

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD تهی APFloat::dump() ثابت {
  print(dbgs());
  dbgs() << '\n';
}
#endif

تهی APFloat::Profile(Folding_Set_گره_ID &NID) ثابت {
  NID.Add(bitcastToAPInt());
}

APFloat::opStatus APFloat::convertToInteger(APSInt &result,
                                            roundingMode rounding_mode,
                                            منطقی *isExact) ثابت {
  unsigned bitWidth = result.getBitWidth();
  SmallVector<uint64_t, 4> parts(result.get_رقم_واژگان());
  opStatus status = convertToInteger(parts, bitWidth, result.isSigned(),
                                     rounding_mode, isExact);
  // Keeps the original signed-ness.
  result = APInt(bitWidth, parts);
  بازگشت status;
}

دوگانه APFloat::convertToDouble() ثابت {
  اگر (&getSemantics() == &APFloatBase::semIEEEdouble)
    بازگشت getIEEE().convertToDouble();
  أكد(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat Temp = *این;
  منطقی LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  بازگشت Temp.getIEEE().convertToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 APFloat::convertToQuad() ثابت {
  اگر (&getSemantics() == &APFloatBase::semIEEEquad)
    بازگشت getIEEE().convertToQuad();
  أكد(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat Temp = *این;
  منطقی LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEquad, rmNearestTiesToEven, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  بازگشت Temp.getIEEE().convertToQuad();
}
#endif

اعشاری APFloat::convertToFloat() ثابت {
  اگر (&getSemantics() == &APFloatBase::semIEEEsingle)
    بازگشت getIEEE().convertToFloat();
  أكد(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat Temp = *این;
  منطقی LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEsingle, rmNearestTiesToEven, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  بازگشت Temp.getIEEE().convertToFloat();
}

unsigned APFloatBase::getArbitraryFPFormatSizeInBits(StringRef Format) {
  بازگشت StringSwitch<unsigned>(Format)
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

منطقی APFloatBase::isValidArbitraryFPFormat(StringRef Format) {
  بازگشت getArbitraryFPFormatSizeInBits(Format) != 0;
}

ثابت fltSemantics *APFloatBase::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  بازگشت StringSwitch<ثابت fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(nullptr);
}

APFloat::Storage::~Storage() {
  اگر (usesLayout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    بازگشت;
  }
  اگر (usesLayout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    بازگشت;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(ثابت APFloat::Storage &RHS) {
  اگر (usesLayout<IEEEFloat>(*RHS.semantics)) {
    جدید (این) IEEEFloat(RHS.IEEE);
    بازگشت;
  }
  اگر (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    جدید (این) DoubleAPFloat(RHS.Double);
    بازگشت;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  اگر (usesLayout<IEEEFloat>(*RHS.semantics)) {
    جدید (این) IEEEFloat(std::move(RHS.IEEE));
    بازگشت;
  }
  اگر (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    جدید (این) DoubleAPFloat(std::move(RHS.Double));
    بازگشت;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::operator=(ثابت APFloat::Storage &RHS) {
  اگر (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } وگرنه اگر (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } وگرنه اگر (این != &RHS) {
    این->~Storage();
    جدید (این) Storage(RHS);
  }
  بازگشت *این;
}

APFloat::Storage &APFloat::Storage::operator=(APFloat::Storage &&RHS) {
  اگر (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } وگرنه اگر (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } وگرنه اگر (این != &RHS) {
    این->~Storage();
    جدید (این) Storage(std::move(RHS));
  }
  بازگشت *این;
}

فضاینام {

APFloat::opStatus getOpStatusFromLibc(صحیح libc_exceptions) {
  APFloat::opStatus status = APFloat::opOK;
  اگر (libc_exceptions & FE_INVALID)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInvalidOp);
  اگر (libc_exceptions & FE_DIVBYZERO)
    status = static_cast<APFloat::opStatus>(status | APFloat::opDivByZero);
  اگر (libc_exceptions & FE_OVERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opOverflow);
  اگر (libc_exceptions & FE_UNDERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opUnderflow);
  اگر (libc_exceptions & FE_INEXACT)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInexact);
  بازگشت status;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> exp(ثابت APFloat &x, RoundingMode rounding_mode,
                           APFloat::opStatus *status) {

  اگر (rounding_mode == APFloatBase::rmNearestTiesToEven) {
    اگر (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEsingle) {
      اعشاری x_val = x.convertToFloat();
      صحیح exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      اگر (status) {
        *status = getOpStatusFromLibc(exc);
        اگر (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      اعشاری result = LIBC_NAMESPACE::shared::expf(x_val);
      بازگشت APFloat(result);
    }
    اگر (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEdouble) {
      دوگانه x_val = x.convertToDouble();
      صحیح exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      اگر (status) {
        *status = getOpStatusFromLibc(exc);
        اگر (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      دوگانه result = LIBC_NAMESPACE::shared::exp(x_val);
      بازگشت APFloat(result);
    }
  }
  بازگشت std::nullopt;
}

} // namespace llvm

#undef APFLOAT_DISPATCH_ON_SEMANTICS
