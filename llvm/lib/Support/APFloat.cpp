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
#وصف_ڪريو LIBC_NAMESPACE __llvm_libc_apfloat
#وصف_ڪريو LIBC_MATH (LIBC_MATH_NO_ERRNO | LIBC_MATH_NO_EXCEPT)

#ضمن "shared/math.h"
#ضمن "shared/math_check_exceptions.h"

#وصف_ڪريو APFLOAT_DISPATCH_ON_SEMANTICS(طريقو_CALL)                             \
  افعل {                                                                         \
    إذا (استعمال_ڪري_Layout<IEEEFloat>(getSemantics()))                                 \
      أرجع U.IEEE.طريقو_CALL;                                               \
    إذا (استعمال_ڪري_Layout<DoubleAPFloat>(getSemantics()))                             \
      أرجع U.Double.طريقو_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } طالما (خطأ)

استعمال نطاق llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#وصف_ڪريو Pack_Categories_Into_ڪنجي(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloat_بنياد::integer_Part_ويڪر % 4 == 0, "Part width must be divisible by 4!");

نطاق llvm {

constexpr fltSemantics APFloat_بنياد::semIEEEhalf = {15, -14, 11, 16};
constexpr fltSemantics APFloat_بنياد::semBFloat = {127, -126, 8, 16};
constexpr fltSemantics APFloat_بنياد::semIEEEsingle = {127, -126, 24, 32};
constexpr fltSemantics APFloat_بنياد::semIEEEdouble = {1023, -1022, 53, 64};
constexpr fltSemantics APFloat_بنياد::semIEEEquad = {16383, -16382, 113, 128};
constexpr fltSemantics APFloat_بنياد::semX87DoubleExtended = {
    16383,
    -16382,
    64,
    80,
    fltNonfiniteBehavior::IEEE754,
    fltNanEncoding::IEEE,
    سچ,
    سچ,
    سچ,
    سچ,
    سچ};
constexpr fltSemantics APFloat_بنياد::semPPCDoubleDouble = {-1, 0, 0, 128};
constexpr fltSemantics APFloat_بنياد::semPPCDoubleDoubleLegacy = {
    1023, -1022 + 53, 53 + 53, 128};
constexpr fltSemantics APFloat_بنياد::semFloat8E5M2 = {15, -14, 3, 8};
constexpr fltSemantics APFloat_بنياد::semFloat8E5M2FNUZ = {
    15, -15, 3, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_بنياد::semFloat8E4M3 = {7, -6, 4, 8};
constexpr fltSemantics APFloat_بنياد::semFloat8E4M3FN = {
    8, -6, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::AllOnes};
constexpr fltSemantics APFloat_بنياد::semFloat8E4M3FNUZ = {
    7, -7, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_بنياد::semFloat8E4M3B11FNUZ = {
    4, -10, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_بنياد::semFloat8E3M4 = {3, -2, 5, 8};
constexpr fltSemantics APFloat_بنياد::semFloatTF32 = {127, -126, 11, 19};
constexpr fltSemantics APFloat_بنياد::semFloat8E8M0FNU = {
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

constexpr fltSemantics APFloat_بنياد::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    سچ,
    خطأ,
    خطأ};

constexpr fltSemantics APFloat_بنياد::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_بنياد::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_بنياد::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_بنياد::semBogus = {0, 0, 0, 0};

ثابت llvm::fltSemantics &APFloat_بنياد::Enum_منزل_Semantics(Semantics S) {
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

APFloat_بنياد::Semantics
APFloat_بنياد::Semantics_منزل_Enum(ثابت llvm::fltSemantics &Sem) {
  إذا (&Sem == &llvm::APFloat::IEEEhalf())
    أرجع S_IEEEhalf;
  وإلا إذا (&Sem == &llvm::APFloat::BFloat())
    أرجع S_BFloat;
  وإلا إذا (&Sem == &llvm::APFloat::IEEEsingle())
    أرجع S_IEEEsingle;
  وإلا إذا (&Sem == &llvm::APFloat::IEEEdouble())
    أرجع S_IEEEdouble;
  وإلا إذا (&Sem == &llvm::APFloat::IEEEquad())
    أرجع S_IEEEquad;
  وإلا إذا (&Sem == &llvm::APFloat::x87DoubleExtended())
    أرجع S_x87DoubleExtended;
  وإلا إذا (&Sem == &llvm::APFloat::PPCDoubleDouble())
    أرجع S_PPCDoubleDouble;
  وإلا إذا (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    أرجع S_PPCDoubleDoubleLegacy;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E5M2())
    أرجع S_Float8E5M2;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    أرجع S_Float8E5M2FNUZ;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E4M3())
    أرجع S_Float8E4M3;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E4M3FN())
    أرجع S_Float8E4M3FN;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    أرجع S_Float8E4M3FNUZ;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    أرجع S_Float8E4M3B11FNUZ;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E3M4())
    أرجع S_Float8E3M4;
  وإلا إذا (&Sem == &llvm::APFloat::FloatTF32())
    أرجع S_FloatTF32;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E8M0FNU())
    أرجع S_Float8E8M0FNU;
  وإلا إذا (&Sem == &llvm::APFloat::Float8E5M3FNU())
    أرجع S_Float8E5M3FNU;
  وإلا إذا (&Sem == &llvm::APFloat::Float6E3M2FN())
    أرجع S_Float6E3M2FN;
  وإلا إذا (&Sem == &llvm::APFloat::Float6E2M3FN())
    أرجع S_Float6E2M3FN;
  وإلا إذا (&Sem == &llvm::APFloat::Float4E2M1FN())
    أرجع S_Float4E2M1FN;
  وإلا
    llvm_unreachable("Unknown floating semantics");
}

منطقي APFloat_بنياد::isRepresentableBy(ثابت fltSemantics &A,
                                    ثابت fltSemantics &B) {
  أرجع A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

منطقي APFloat_بنياد::is_Losslessly_Convertible_منزل(ثابت fltSemantics &From,
                                            ثابت fltSemantics &Initial_منزل,
                                            منطقي IgnoreNaNs) {
  إذا (&From == &Initial_منزل)
    أرجع سچ;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  إذا (&From == &semPPCDoubleDouble)
    أرجع خطأ;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  ثابت fltSemantics &منزل =
      &Initial_منزل == &semPPCDoubleDouble ? semIEEEdouble : Initial_منزل;

  إذا (!isRepresentableBy(From, منزل))
    أرجع خطأ;

  إذا ((From.hasZero && !منزل.hasZero) ||
      (From.hasSignedRepr && !منزل.hasSignedRepr))
    أرجع خطأ;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  منطقي FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  منطقي منزل_Has_Signed_Zero = منزل.hasZero && منزل.hasSignedRepr &&
                         منزل.nanEncoding != fltNanEncoding::NegativeZero;
  إذا (FromHasSignedZero && !منزل_Has_Signed_Zero)
    أرجع خطأ;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat سڀ_کان_ننڍو_From = APFloat::get_سڀ_کان_ننڍو(From);
  منطقي LosesInfo = خطأ;
  (فراغ)سڀ_کان_ننڍو_From.convert(منزل, APFloat::rm_سڀ_کان_ويجهو_Ties_منزل_Even, &LosesInfo);
  إذا (LosesInfo)
    أرجع خطأ;

  إذا (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    أرجع سچ;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  إذا (IgnoreNaNs) {
    إذا (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      أرجع منزل.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    أرجع منزل.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  إذا (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    أرجع خطأ;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  إذا (منزل.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    أرجع سچ;
  أرجع منزل.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
         From.nanEncoding == منزل.nanEncoding;
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
[[maybe_اڻ_استعمال_ٿيل]] constexpr بنانشان maxExponent = 16383;

// Decimal exponents in the f16 and f32 ranges need at most 2 and 6 parts.
// Keep 16 parts (128 bytes) inline: enough for 5^413, but not a large stack
// frame. Longer fractional spellings use heap storage.
ساكن constexpr بنانشان انگ_Pow5_Parts_On_ڍير = 16;

ساكن بنانشان صحيح power_جو_Five_Parts_For_Exponent(بنانشان صحيح power) {
  أكد(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  أرجع 2 + ((power * 815) / (351 * APFloat_بنياد::integer_Part_ويڪر));
}

بنانشان صحيح APFloat_بنياد::semanticsPrecision(ثابت fltSemantics &semantics) {
  أرجع semantics.precision;
}
APFloat_بنياد::ExponentType
APFloat_بنياد::semanticsMaxExponent(ثابت fltSemantics &semantics) {
  أرجع semantics.maxExponent;
}
APFloat_بنياد::ExponentType
APFloat_بنياد::semanticsMinExponent(ثابت fltSemantics &semantics) {
  أرجع semantics.minExponent;
}
بنانشان صحيح APFloat_بنياد::semantics_سائيز_اندريون_بٽ(ثابت fltSemantics &semantics) {
  أرجع semantics.سائيز_اندريون_بٽ;
}
بنانشان صحيح APFloat_بنياد::semantics_Int_سائيز_اندريون_بٽ(ثابت fltSemantics &semantics,
                                                 منطقي isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  بنانشان صحيح Min_ٻٽو_انگ_ويڪر = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  إذا (isSigned)
    ++Min_ٻٽو_انگ_ويڪر;
  أرجع Min_ٻٽو_انگ_ويڪر;
}

منطقي APFloat_بنياد::semanticsHasZero(ثابت fltSemantics &semantics) {
  أرجع semantics.hasZero;
}

منطقي APFloat_بنياد::semanticsHasSignedRepr(ثابت fltSemantics &semantics) {
  أرجع semantics.hasSignedRepr;
}

منطقي APFloat_بنياد::semanticsHasInf(ثابت fltSemantics &semantics) {
  أرجع semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

منطقي APFloat_بنياد::semanticsHasNaN(ثابت fltSemantics &semantics) {
  أرجع semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

منطقي APFloat_بنياد::isIEEELikeFP(ثابت fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  أرجع Semantics_منزل_Enum(semantics) <= S_IEEEquad;
}

منطقي APFloat_بنياد::has_Sign_ٻٽو_انگ_اندريون_MSB(ثابت fltSemantics &semantics) {
  أرجع semantics.has_Sign_ٻٽو_انگ_اندريون_MSB;
}

منطقي APFloat_بنياد::is_Representable_As_عام_اندريون(ثابت fltSemantics &Src,
                                            ثابت fltSemantics &Dst) {
  // Exponent range must be larger.
  إذا (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    أرجع خطأ;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  أرجع Dst.precision >= Src.precision;
}

بنانشان APFloat_بنياد::get_سائيز_اندريون_بٽ(ثابت fltSemantics &Sem) {
  أرجع Sem.سائيز_اندريون_بٽ;
}

ساكن constexpr APFloat_بنياد::ExponentType
exponentZero(ثابت fltSemantics &semantics) {
  أرجع semantics.minExponent - 1;
}

ساكن constexpr APFloat_بنياد::ExponentType
exponentInf(ثابت fltSemantics &semantics) {
  أرجع semantics.maxExponent + 1;
}

ساكن constexpr APFloat_بنياد::ExponentType
exponentNaN(ثابت fltSemantics &semantics) {
  إذا (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    إذا (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      أرجع exponentZero(semantics);
    إذا (semantics.hasSignedRepr || semantics.precision > 1)
      أرجع semantics.maxExponent;
  }
  أرجع semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

ساكن مضمن Error createError(ثابت Twine &Err) {
  أرجع make_error<StringError>(Err, inconvertible_Error_ڪوڊ());
}

ساكن constexpr مضمن بنانشان صحيح part_ڳڻپ_For_بٽ(بنانشان صحيح بٽ) {
  أرجع std::max(1u, (بٽ + APFloat_بنياد::integer_Part_ويڪر - 1) /
                          APFloat_بنياد::integer_Part_ويڪر);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
ساكن مضمن بنانشان صحيح
dec_Digit_قدر(بنانشان صحيح c)
{
  أرجع c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
ساكن Expected<صحيح> readExponent(StringRef::iterator begin,
                                  StringRef::iterator پڇاڙي) {
  ثابت بنانشان صحيح overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  إذا (p == پڇاڙي || ((*p == '-' || *p == '+') && (p + 1) == پڇاڙي))
    أرجع 0;

  منطقي isNegative = *p == '-';
  إذا (*p == '-' || *p == '+') {
    p++;
    إذا (p == پڇاڙي)
      أرجع createError("Exponent has no digits");
  }

  بنانشان absExponent = dec_Digit_قدر(*p++);
  إذا (absExponent >= 10U)
    أرجع createError("Invalid character in exponent");

  لكل (; p != پڇاڙي; ++p) {
    بنانشان قدر_2 = dec_Digit_قدر(*p);
    إذا (قدر_2 >= 10U)
      أرجع createError("Invalid character in exponent");

    absExponent = absExponent * 10U + قدر_2;
    إذا (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      اكسر;
    }
  }

  إذا (isNegative)
    أرجع -(صحيح) absExponent;
  وإلا
    أرجع (صحيح) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
ساكن Expected<صحيح> ڪل_Exponent(StringRef::iterator p,
                                   StringRef::iterator پڇاڙي,
                                   صحيح exponentAdjustment) {
  صحيح exponent = 0;

  إذا (p == پڇاڙي)
    أرجع createError("Exponent has no digits");

  منطقي negative = *p == '-';
  إذا (*p == '-' || *p == '+') {
    p++;
    إذا (p == پڇاڙي)
      أرجع createError("Exponent has no digits");
  }

  صحيح unsignedExponent = 0;
  منطقي حد_کان_مٿي_2 = خطأ;
  لكل (; p != پڇاڙي; ++p) {
    بنانشان صحيح قدر_2;

    قدر_2 = dec_Digit_قدر(*p);
    إذا (قدر_2 >= 10U)
      أرجع createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + قدر_2;
    إذا (unsignedExponent > 32767) {
      حد_کان_مٿي_2 = سچ;
      اكسر;
    }
  }

  إذا (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    حد_کان_مٿي_2 = سچ;

  إذا (!حد_کان_مٿي_2) {
    exponent = unsignedExponent;
    إذا (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    إذا (exponent > 32767 || exponent < -32768)
      حد_کان_مٿي_2 = سچ;
  }

  إذا (حد_کان_مٿي_2)
    exponent = negative ? -32768: 32767;

  أرجع exponent;
}

ساكن Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator پڇاڙي,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = پڇاڙي;
  طالما (p != پڇاڙي && *p == '0')
    p++;

  إذا (p != پڇاڙي && *p == '.') {
    *dot = p++;

    إذا (پڇاڙي - begin == 1)
      أرجع createError("Significand has no digits");

    طالما (p != پڇاڙي && *p == '0')
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
  ثابت حرف *پهريون_Sig_Digit;
  ثابت حرف *آخري_Sig_Digit;
  صحيح exponent;
  صحيح normalizedExponent;
};

ساكن Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator پڇاڙي, decimalInfo *D) {
  StringRef::iterator dot = پڇاڙي;

  خودڪار PtrOrErr = skipLeadingZeroesAndAnyDot(begin, پڇاڙي, &dot);
  إذا (!PtrOrErr)
    أرجع PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->پهريون_Sig_Digit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  لكل (; p != پڇاڙي; ++p) {
    إذا (*p == '.') {
      إذا (dot != پڇاڙي)
        أرجع createError("String contains multiple dots");
      dot = p++;
      إذا (p == پڇاڙي)
        اكسر;
    }
    إذا (dec_Digit_قدر(*p) >= 10U)
      اكسر;
  }

  إذا (p != پڇاڙي) {
    إذا (*p != 'e' && *p != 'E')
      أرجع createError("Invalid character in significand");
    إذا (p == begin)
      أرجع createError("Significand has no digits");
    إذا (dot != پڇاڙي && p - begin == 1)
      أرجع createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    خودڪار ExpOrErr = readExponent(p + 1, پڇاڙي);
    إذا (!ExpOrErr)
      أرجع ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    إذا (dot == پڇاڙي)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  إذا (p != D->پهريون_Sig_Digit) {
    /* Drop insignificant trailing zeroes.  */
    إذا (p != begin) {
      افعل
        افعل
          p--;
        طالما (p != begin && *p == '0');
      طالما (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->پهريون_Sig_Digit)
                                      - (dot > D->پهريون_Sig_Digit && dot < p)));
  }

  D->آخري_Sig_Digit = p;
  أرجع Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
ساكن Expected<گم_Fraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator پڇاڙي,
                            بنانشان صحيح digit_قدر) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  إذا (digit_قدر > 8)
    أرجع lf_وڌيڪ_Than_Half;
  وإلا إذا (digit_قدر < 8 && digit_قدر > 0)
    أرجع lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  طالما (p != پڇاڙي && (*p == '0' || *p == '.'))
    p++;

  إذا (p == پڇاڙي)
    أرجع createError("Invalid trailing hexadecimal fraction!");

  بنانشان hexDigit = hex_Digit_قدر(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  إذا (hexDigit == UINT_MAX)
    أرجع digit_قدر == 0 ? lfExactlyZero: lfExactlyHalf;
  وإلا
    أرجع digit_قدر == 0 ? lfLessThanHalf: lf_وڌيڪ_Than_Half;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
ساكن گم_Fraction
گم_Fraction_Through_Truncation(ثابت APFloat_بنياد::integerPart *parts,
                              بنانشان صحيح part_ڳڻپ,
                              بنانشان صحيح بٽ)
{
  بنانشان lsb = APInt::tcLSB(parts, part_ڳڻپ);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  إذا (بٽ <= lsb)
    أرجع lfExactlyZero;
  إذا (بٽ == lsb + 1)
    أرجع lfExactlyHalf;
  إذا (بٽ <= part_ڳڻپ * APFloat_بنياد::integer_Part_ويڪر &&
      APInt::tc_ڪڍو_ٻٽو_انگ(parts, بٽ - 1))
    أرجع lf_وڌيڪ_Than_Half;

  أرجع lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
ساكن گم_Fraction
سرڪايو_ساڄو(APFloat_بنياد::integerPart *dst, بنانشان صحيح parts, بنانشان صحيح بٽ)
{
  گم_Fraction گم_fraction = گم_Fraction_Through_Truncation(dst, parts, بٽ);

  APInt::tc_سرڪايو_ساڄو(dst, parts, بٽ);

  أرجع گم_fraction;
}

/* Combine the effect of two lost fractions.  */
ساكن گم_Fraction
combine_گم_Fractions(گم_Fraction وڌيڪ_Significant,
                     گم_Fraction lessSignificant)
{
  إذا (lessSignificant != lfExactlyZero) {
    إذا (وڌيڪ_Significant == lfExactlyZero)
      وڌيڪ_Significant = lfLessThanHalf;
    وإلا إذا (وڌيڪ_Significant == lfExactlyHalf)
      وڌيڪ_Significant = lf_وڌيڪ_Than_Half;
  }

  أرجع وڌيڪ_Significant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
ساكن بنانشان صحيح
HUerrBound(منطقي inexactMultiply, بنانشان صحيح HUerr1, بنانشان صحيح HUerr2)
{
  أكد(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  إذا (HUerr1 + HUerr2 == 0)
    أرجع inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  وإلا
    أرجع inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
ساكن APFloat_بنياد::integerPart
ulpsFromBoundary(ثابت APFloat_بنياد::integerPart *parts, بنانشان صحيح بٽ,
                 منطقي is_سڀ_کان_ويجهو) {
  أكد(بٽ != 0);

  بٽ--;
  بنانشان ڳڻپ = بٽ / APFloat_بنياد::integer_Part_ويڪر;
  بنانشان part_بٽ = بٽ % APFloat_بنياد::integer_Part_ويڪر + 1;

  APFloat_بنياد::integerPart part =
      parts[ڳڻپ] & (~(APFloat_بنياد::integerPart)0 >>
                      (APFloat_بنياد::integer_Part_ويڪر - part_بٽ));

  APFloat_بنياد::integerPart boundary;
  إذا (is_سڀ_کان_ويجهو)
    boundary = (APFloat_بنياد::integerPart) 1 << (part_بٽ - 1);
  وإلا
    boundary = 0;

  إذا (ڳڻپ == 0) {
    إذا (part - boundary <= boundary - part)
      أرجع part - boundary;
    وإلا
      أرجع boundary - part;
  }

  إذا (part == boundary) {
    طالما (--ڳڻپ)
      إذا (parts[ڳڻپ])
        أرجع ~(APFloat_بنياد::integerPart) 0; /* A lot.  */

    أرجع parts[0];
  } وإلا إذا (part == boundary - 1) {
    طالما (--ڳڻپ)
      إذا (~parts[ڳڻپ])
        أرجع ~(APFloat_بنياد::integerPart) 0; /* A lot.  */

    أرجع -parts[0];
  }

  أرجع ~(APFloat_بنياد::integerPart) 0; /* A lot.  */
}

ساكن constexpr بنانشان Power_جو_Five_Part_Counts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

ساكن constexpr APFloat_بنياد::integerPart Power_جو_Five_Parts[] = {
#ضمن "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  بنانشان جوڙ = 0;
  لكل (بنانشان ڳڻپ_2 : Power_جو_Five_Part_Counts)
    جوڙ += ڳڻپ_2;
  أرجع جوڙ;
}() == std::سائيز_2(Power_جو_Five_Parts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
ساكن بنانشان صحيح
powerOf5(APFloat_بنياد::integerPart *dst, بنانشان صحيح power) {
  ساكن ثابت APFloat_بنياد::integerPart پهريون_Eight_Powers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  Small_ويڪٽر<APFloat_بنياد::integerPart, انگ_Pow5_Parts_On_ڍير> scratch;
  scratch.resize_for_overwrite(power_جو_Five_Parts_For_Exponent(power));

  APFloat_بنياد::integerPart *p1, *p2;
  ثابت APFloat_بنياد::integerPart *pow5;

  p1 = dst;
  p2 = scratch.معلومات();

  *p1 = پهريون_Eight_Powers[power & 7];
  power >>= 3;

  بنانشان نتيجو_2 = 1;
  pow5 = Power_جو_Five_Parts;

  لكل (بنانشان صحيح n = 0; power; power >>= 1, n++) {
    بنانشان parts_ڳڻپ = Power_جو_Five_Part_Counts[n];

    إذا (power & 1) {
      APFloat_بنياد::integerPart *tmp;

      APInt::tc_ڀريل_Multiply(p2, p1, pow5, نتيجو_2, parts_ڳڻپ);
      نتيجو_2 += parts_ڳڻپ;
      إذا (p2[نتيجو_2 - 1] == 0)
        نتيجو_2--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_ڳڻپ;
  }

  إذا (p1 != dst)
    APInt::tc_سونپيو(dst, p1, نتيجو_2);

  أرجع نتيجو_2;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
ساكن ثابت حرف hex_Digits_هيٺاهون[] = "0123456789abcdef0";
ساكن ثابت حرف hex_Digits_مٿانهون[] = "0123456789ABCDEF0";
ساكن ثابت حرف infinityL[] = "infinity";
ساكن ثابت حرف infinityU[] = "INFINITY";
ساكن ثابت حرف NaNL[] = "nan";
ساكن ثابت حرف NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
ساكن بنانشان صحيح
partAsHex (حرف *dst, APFloat_بنياد::integerPart part, بنانشان صحيح ڳڻپ,
           ثابت حرف *hexDigitChars)
{
  بنانشان صحيح نتيجو_2 = ڳڻپ;

  أكد(ڳڻپ != 0 && ڳڻپ <= APFloat_بنياد::integer_Part_ويڪر / 4);

  part >>= (APFloat_بنياد::integer_Part_ويڪر - 4 * ڳڻپ);
  طالما (ڳڻپ--) {
    dst[ڳڻپ] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  أرجع نتيجو_2;
}

/* Write out an unsigned decimal integer.  */
ساكن حرف *writeUnsignedDecimal(حرف *dst, بنانشان صحيح n) {
  حرف buff[40], *p;

  p = buff;
  افعل
    *p++ = '0' + n % 10;
  طالما (n /= 10);

  افعل
    *dst++ = *--p;
  طالما (p != buff);

  أرجع dst;
}

/* Write out a signed decimal integer.  */
ساكن حرف *writeSignedDecimal(حرف *dst, صحيح قدر_2) {
  إذا (قدر_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(بنانشان) قدر_2);
  } وإلا {
    dst = writeUnsignedDecimal(dst, قدر_2);
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
    أرجع APFloat::get_سڀ_کان_ننڍو(Sem);
  حالة APFloat::fc_عام:
    اكسر;
  }
  إذا (X.isDenormal() || X.is_سڀ_کان_ننڍو_Normalized())
    أرجع APFloat::get_سڀ_کان_ننڍو(Sem);
  صحيح Exp = ilogb(X);
  إذا (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  أرجع scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rm_سڀ_کان_ويجهو_Ties_منزل_Even);
}

نطاق detail {
/* Constructors.  */
فراغ IEEEFloat::initialize(ثابت fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  بنانشان ڳڻپ = part_ڳڻپ();
  إذا (ڳڻپ > 1)
    significand.parts = جديد integerPart[ڳڻپ];
}

فراغ IEEEFloat::freeSignificand() {
  إذا (needsCleanup())
    ميٽيو [] significand.parts;
}

فراغ IEEEFloat::سونپيو(ثابت IEEEFloat &rhs) {
  أكد(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  إذا (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

فراغ IEEEFloat::copySignificand(ثابت IEEEFloat &rhs) {
  أكد(isFiniteNonZero() || category == fcNaN);
  أكد(rhs.part_ڳڻپ() >= part_ڳڻپ());

  APInt::tc_سونپيو(significandParts(), rhs.significandParts(),
                  part_ڳڻپ());
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
  بنانشان انگ_Parts = part_ڳڻپ();

  APInt fill_storage;
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = خطأ;
    إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = سچ;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } وإلا {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  إذا (!fill || fill->get_انگ_لفظ() < انگ_Parts)
    APInt::tc_سيٽ(significand, 0, انگ_Parts);
  إذا (fill) {
    APInt::tc_سونپيو(significand, fill->get_Raw_معلومات(),
                    std::min(fill->get_انگ_لفظ(), انگ_Parts));

    // Zero out the excess bits of the significand.
    بنانشان بٽ_منزل_Preserve = semantics->precision - 1;
    بنانشان part = بٽ_منزل_Preserve / 64;
    بٽ_منزل_Preserve %= 64;
    significand[part] &= ((1ULL << بٽ_منزل_Preserve) - 1);
    لكل (part++; part != انگ_Parts; ++part)
      significand[part] = 0;
  }

  بنانشان QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  إذا (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tc_Clear_ٻٽو_انگ(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    إذا (APInt::tcIsZero(significand, انگ_Parts))
      APInt::tc_سيٽ_ٻٽو_انگ(significand, QNaNBit - 1);
  } وإلا إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } وإلا {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tc_سيٽ_ٻٽو_انگ(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  إذا (semantics == &APFloat_بنياد::semX87DoubleExtended)
    APInt::tc_سيٽ_ٻٽو_انگ(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::عامل=(ثابت IEEEFloat &rhs) {
  إذا (هذا != &rhs) {
    إذا (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    سونپيو(rhs);
  }

  أرجع *هذا;
}

IEEEFloat &IEEEFloat::عامل=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloat_بنياد::semBogus;
  أرجع *هذا;
}

منطقي IEEEFloat::isDenormal() ثابت {
  أرجع getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tc_ڪڍو_ٻٽو_انگ(significandParts(), semantics->precision - 1) ==
          0);
}

منطقي IEEEFloat::is_سڀ_کان_ننڍو() ثابت {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  أرجع isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

منطقي IEEEFloat::is_سڀ_کان_ننڍو_Normalized() ثابت {
  أرجع getCategory() == fc_عام && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

بنانشان صحيح IEEEFloat::get_انگ_مٿاهون_بٽ() ثابت {
  ثابت بنانشان صحيح Part_ڳڻپ = part_ڳڻپ_For_بٽ(semantics->precision);
  ثابت بنانشان صحيح بٽ_2 = Part_ڳڻپ * integer_Part_ويڪر;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  ثابت بنانشان صحيح انگ_مٿاهون_بٽ = (semantics->precision > 1)
                                       ? (بٽ_2 - semantics->precision + 1)
                                       : (بٽ_2 - semantics->precision);
  أرجع انگ_مٿاهون_بٽ;
}

منطقي IEEEFloat::isSignificandAllOnes() ثابت {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  ثابت integerPart *Parts = significandParts();
  ثابت بنانشان Part_ڳڻپ = part_ڳڻپ_For_بٽ(semantics->precision);
  لكل (بنانشان i = 0; i < Part_ڳڻپ - 1; i++)
    إذا (~Parts[i])
      أرجع خطأ;

  // Set the unused high bits to all ones when we compare.
  ثابت بنانشان انگ_مٿاهون_بٽ = get_انگ_مٿاهون_بٽ();
  أكد(انگ_مٿاهون_بٽ <= integer_Part_ويڪر && انگ_مٿاهون_بٽ > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ثابت integerPart مٿاهون_ٻٽو_انگ_Fill =
    ~integerPart(0) << (integer_Part_ويڪر - انگ_مٿاهون_بٽ);
  إذا ((semantics->precision <= 1) || (~(Parts[Part_ڳڻپ - 1] | مٿاهون_ٻٽو_انگ_Fill)))
    أرجع خطأ;

  أرجع سچ;
}

منطقي IEEEFloat::isSignificandAllOnesExceptLSB() ثابت {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  ثابت integerPart *Parts = significandParts();

  إذا (Parts[0] & 1)
    أرجع خطأ;

  ثابت بنانشان Part_ڳڻپ = part_ڳڻپ_For_بٽ(semantics->precision);
  لكل (بنانشان i = 0; i < Part_ڳڻپ - 1; i++) {
    إذا (~Parts[i] & ~بنانشان{!i})
      أرجع خطأ;
  }

  // Set the unused high bits to all ones when we compare.
  ثابت بنانشان انگ_مٿاهون_بٽ = get_انگ_مٿاهون_بٽ();
  أكد(انگ_مٿاهون_بٽ <= integer_Part_ويڪر && انگ_مٿاهون_بٽ > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ثابت integerPart مٿاهون_ٻٽو_انگ_Fill = ~integerPart(0)
                                  << (integer_Part_ويڪر - انگ_مٿاهون_بٽ);
  إذا (~(Parts[Part_ڳڻپ - 1] | مٿاهون_ٻٽو_انگ_Fill | 0x1))
    أرجع خطأ;

  أرجع سچ;
}

منطقي IEEEFloat::isSignificandAllZeros() ثابت {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  ثابت integerPart *Parts = significandParts();
  ثابت بنانشان Part_ڳڻپ = part_ڳڻپ_For_بٽ(semantics->precision);

  لكل (بنانشان i = 0; i < Part_ڳڻپ - 1; i++)
    إذا (Parts[i])
      أرجع خطأ;

  // Compute how many bits are used in the final word.
  ثابت بنانشان انگ_مٿاهون_بٽ = get_انگ_مٿاهون_بٽ();
  أكد(انگ_مٿاهون_بٽ < integer_Part_ويڪر && "Can not have more high bits to "
         "clear than integerPartWidth");
  ثابت integerPart مٿاهون_ٻٽو_انگ_نقاب = ~integerPart(0) >> انگ_مٿاهون_بٽ;

  إذا ((semantics->precision > 1) && (Parts[Part_ڳڻپ - 1] & مٿاهون_ٻٽو_انگ_نقاب))
    أرجع خطأ;

  أرجع سچ;
}

منطقي IEEEFloat::isSignificandAllZerosExceptMSB() ثابت {
  ثابت integerPart *Parts = significandParts();
  ثابت بنانشان Part_ڳڻپ = part_ڳڻپ_For_بٽ(semantics->precision);

  لكل (بنانشان i = 0; i < Part_ڳڻپ - 1; i++) {
    إذا (Parts[i])
      أرجع خطأ;
  }

  ثابت بنانشان انگ_مٿاهون_بٽ = get_انگ_مٿاهون_بٽ();
  ثابت integerPart MSBMask = integerPart(1)
                              << (integer_Part_ويڪر - انگ_مٿاهون_بٽ);
  أرجع ((semantics->precision <= 1) || (Parts[Part_ڳڻپ - 1] == MSBMask));
}

منطقي IEEEFloat::is_سڀ_کان_وڏو() ثابت {
  منطقي IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
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
  إذا (!isFinite()) أرجع خطأ;
  IEEEFloat truncated = *هذا;
  truncated.ڦيرو_منزل_Integral(rmTowardZero);
  أرجع compare(truncated) == cmpEqual;
}

منطقي IEEEFloat::bitwiseIsEqual(ثابت IEEEFloat &rhs) ثابت {
  إذا (هذا == &rhs)
    أرجع سچ;
  إذا (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    أرجع خطأ;
  إذا (category==fcZero || category==fcInfinity)
    أرجع سچ;

  إذا (isFiniteNonZero() && exponent != rhs.exponent)
    أرجع خطأ;

  أرجع std::equal(significandParts(), significandParts() + part_ڳڻپ(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics, integerPart قدر_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fc_عام;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = قدر_2;
  normalize(rm_سڀ_کان_ويجهو_Ties_منزل_Even, lfExactlyZero);
}

IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(خطأ) : make_سڀ_کان_ننڍو_Normalized(خطأ);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics, uninitialized_ٽيگ ٽيگ_2)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(ثابت IEEEFloat &rhs) {
  initialize(rhs.semantics);
  سونپيو(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloat_بنياد::semBogus) {
  *هذا = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

بنانشان صحيح IEEEFloat::part_ڳڻپ() ثابت {
  أرجع part_ڳڻپ_For_بٽ(semantics->precision + 1);
}

ثابت APFloat::integerPart *IEEEFloat::significandParts() ثابت {
  أرجع const_cast<IEEEFloat *>(هذا)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  إذا (part_ڳڻپ() > 1)
    أرجع significand.parts;
  وإلا
    أرجع &significand.part;
}

فراغ IEEEFloat::zeroSignificand() {
  APInt::tc_سيٽ(significandParts(), 0, part_ڳڻپ());
}

/* Increment an fcNormal floating point number's significand.  */
فراغ IEEEFloat::incrementSignificand() {
  [[maybe_اڻ_استعمال_ٿيل]] integerPart حاصل =
      APInt::tcIncrement(significandParts(), part_ڳڻپ());

  /* Our callers should never cause us to overflow.  */
  أكد(حاصل == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::جوڙيو_Significand(ثابت IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  أكد(semantics == rhs.semantics);
  أكد(exponent == rhs.exponent);

  أرجع APInt::tc_جوڙيو(parts, rhs.significandParts(), 0, part_ڳڻپ());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::ڪٽيو_Significand(ثابت IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  أكد(semantics == rhs.semantics);
  أكد(exponent == rhs.exponent);

  أرجع APInt::tc_ڪٽيو(parts, rhs.significandParts(), borrow,
                           part_ڳڻپ());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
گم_Fraction IEEEFloat::multiplySignificand(ثابت IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            منطقي ignoreAddend) {
  integerPart scratch[4];
  منطقي ignored;

  أكد(semantics == rhs.semantics);

  بنانشان precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  بنانشان new_Parts_ڳڻپ = part_ڳڻپ_For_بٽ(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *ڀريل_Significand =
      new_Parts_ڳڻپ > 4 ? جديد integerPart[new_Parts_ڳڻپ] : scratch;

  integerPart *lhsSignificand = significandParts();
  بنانشان parts_ڳڻپ = part_ڳڻپ();

  APInt::tc_ڀريل_Multiply(ڀريل_Significand, lhsSignificand,
                        rhs.significandParts(), parts_ڳڻپ, parts_ڳڻپ);

  گم_Fraction گم_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  بنانشان omsb = APInt::tcMSB(ڀريل_Significand, new_Parts_ڳڻپ) + 1;
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
    بنانشان extendedPrecision = 2 * precision + 1;
    إذا (omsb != extendedPrecision - 1) {
      أكد(extendedPrecision > omsb);
      APInt::tc_سرڪايو_کاٻو(ڀريل_Significand, new_Parts_ڳڻپ,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    إذا (new_Parts_ڳڻپ == 1)
      significand.part = ڀريل_Significand[0];
    وإلا
      significand.parts = ڀريل_Significand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_اڻ_استعمال_ٿيل]] op_حالت حالت_2 = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    أكد(حالت_2 == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    گم_fraction = extendedAddend.سرڪايو_Significand_ساڄو(1);
    أكد(گم_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    گم_fraction = جوڙيو_Or_ڪٽيو_Significand(extendedAddend, خطأ);

    /* Restore our state.  */
    إذا (new_Parts_ڳڻپ == 1)
      ڀريل_Significand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(ڀريل_Significand, new_Parts_ڳڻپ) + 1;
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
    بنانشان صحيح بٽ, significantParts;
    گم_Fraction lf;

    بٽ = omsb - precision;
    significantParts = part_ڳڻپ_For_بٽ(omsb);
    lf = سرڪايو_ساڄو(ڀريل_Significand, significantParts, بٽ);
    گم_fraction = combine_گم_Fractions(lf, گم_fraction);
    exponent += بٽ;
  }

  APInt::tc_سونپيو(lhsSignificand, ڀريل_Significand, parts_ڳڻپ);

  إذا (new_Parts_ڳڻپ > 4)
    ميٽيو [] ڀريل_Significand;

  أرجع گم_fraction;
}

گم_Fraction IEEEFloat::multiplySignificand(ثابت IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  أرجع multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
گم_Fraction IEEEFloat::divideSignificand(ثابت IEEEFloat &rhs) {
  integerPart scratch[4];

  أكد(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  ثابت integerPart *rhsSignificand = rhs.significandParts();
  بنانشان parts_ڳڻپ = part_ڳڻپ();

  integerPart *dividend =
      parts_ڳڻپ > 2 ? جديد integerPart[parts_ڳڻپ * 2] : scratch;
  integerPart *ورهائيندڙ = dividend + parts_ڳڻپ;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  لكل (بنانشان i = 0; i < parts_ڳڻپ; i++) {
    dividend[i] = lhsSignificand[i];
    ورهائيندڙ[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  بنانشان صحيح precision = semantics->precision;

  /* Normalize the divisor.  */
  بنانشان ٻٽو_انگ = precision - APInt::tcMSB(ورهائيندڙ, parts_ڳڻپ) - 1;
  إذا (ٻٽو_انگ) {
    exponent += ٻٽو_انگ;
    APInt::tc_سرڪايو_کاٻو(ورهائيندڙ, parts_ڳڻپ, ٻٽو_انگ);
  }

  /* Normalize the dividend.  */
  ٻٽو_انگ = precision - APInt::tcMSB(dividend, parts_ڳڻپ) - 1;
  إذا (ٻٽو_انگ) {
    exponent -= ٻٽو_انگ;
    APInt::tc_سرڪايو_کاٻو(dividend, parts_ڳڻپ, ٻٽو_انگ);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  إذا (APInt::tcCompare(dividend, ورهائيندڙ, parts_ڳڻپ) < 0) {
    exponent--;
    APInt::tc_سرڪايو_کاٻو(dividend, parts_ڳڻپ, 1);
    أكد(APInt::tcCompare(dividend, ورهائيندڙ, parts_ڳڻپ) >= 0);
  }

  /* Long division.  */
  لكل (ٻٽو_انگ = precision; ٻٽو_انگ; ٻٽو_انگ -= 1) {
    إذا (APInt::tcCompare(dividend, ورهائيندڙ, parts_ڳڻپ) >= 0) {
      APInt::tc_ڪٽيو(dividend, ورهائيندڙ, 0, parts_ڳڻپ);
      APInt::tc_سيٽ_ٻٽو_انگ(lhsSignificand, ٻٽو_انگ - 1);
    }

    APInt::tc_سرڪايو_کاٻو(dividend, parts_ڳڻپ, 1);
  }

  /* Figure out the lost fraction.  */
  صحيح cmp = APInt::tcCompare(dividend, ورهائيندڙ, parts_ڳڻپ);

  گم_Fraction گم_fraction;
  إذا (cmp > 0)
    گم_fraction = lf_وڌيڪ_Than_Half;
  وإلا إذا (cmp == 0)
    گم_fraction = lfExactlyHalf;
  وإلا إذا (APInt::tcIsZero(dividend, parts_ڳڻپ))
    گم_fraction = lfExactlyZero;
  وإلا
    گم_fraction = lfLessThanHalf;

  إذا (parts_ڳڻپ > 2)
    ميٽيو [] dividend;

  أرجع گم_fraction;
}

بنانشان صحيح IEEEFloat::significandMSB() ثابت {
  أرجع APInt::tcMSB(significandParts(), part_ڳڻپ());
}

بنانشان صحيح IEEEFloat::significandLSB() ثابت {
  أرجع APInt::tcLSB(significandParts(), part_ڳڻپ());
}

/* Note that a zero result is NOT normalized to fcZero.  */
گم_Fraction IEEEFloat::سرڪايو_Significand_ساڄو(بنانشان صحيح بٽ) {
  /* Our exponent should not overflow.  */
  أكد((ExponentType) (exponent + بٽ) >= exponent);

  exponent += بٽ;

  أرجع سرڪايو_ساڄو(significandParts(), part_ڳڻپ(), بٽ);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
فراغ IEEEFloat::سرڪايو_Significand_کاٻو(بنانشان صحيح بٽ) {
  أكد(بٽ < semantics->precision ||
         (semantics->precision == 1 && بٽ <= 1));

  إذا (بٽ) {
    بنانشان صحيح parts_ڳڻپ = part_ڳڻپ();

    APInt::tc_سرڪايو_کاٻو(significandParts(), parts_ڳڻپ, بٽ);
    exponent -= بٽ;

    أكد(!APInt::tcIsZero(significandParts(), parts_ڳڻپ));
  }
}

APFloat::cmp_نتيجو IEEEFloat::compare_Absolute_قدر(ثابت IEEEFloat &rhs) ثابت {
  أكد(semantics == rhs.semantics);
  أكد(isFiniteNonZero());
  أكد(rhs.isFiniteNonZero());

  صحيح compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  إذا (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_ڳڻپ());

  إذا (compare > 0)
    أرجع cmpGreaterThan;
  وإلا إذا (compare < 0)
    أرجع cmpLessThan;
  وإلا
    أرجع cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
ساكن فراغ tc_سيٽ_گهٽ_کان_گهٽ_Significant_بٽ(APInt::لفظ_Type *dst, بنانشان parts,
                                      بنانشان بٽ) {
  بنانشان i = 0;
  طالما (بٽ > APInt::APINT_بٽ_في_لفظ) {
    dst[i++] = ~(APInt::لفظ_Type)0;
    بٽ -= APInt::APINT_بٽ_في_لفظ;
  }

  إذا (بٽ)
    dst[i++] = ~(APInt::لفظ_Type)0 >> (APInt::APINT_بٽ_في_لفظ - بٽ);

  طالما (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::op_حالت IEEEFloat::handle_حد_کان_مٿي(rounding_گهڻائي rounding_گهڻائي_2) {
  إذا (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    إذا (rounding_گهڻائي_2 == rm_سڀ_کان_ويجهو_Ties_منزل_Even ||
        rounding_گهڻائي_2 == rm_سڀ_کان_ويجهو_Ties_منزل_Away ||
        (rounding_گهڻائي_2 == rm_Toward_مثبت && !sign) ||
        (rounding_گهڻائي_2 == rmTowardNegative && sign)) {
      إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(خطأ, sign);
      وإلا
        category = fcInfinity;
      أرجع static_cast<op_حالت>(op_حد_کان_مٿي | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fc_عام;
  exponent = semantics->maxExponent;
  tc_سيٽ_گهٽ_کان_گهٽ_Significant_بٽ(significandParts(), part_ڳڻپ(),
                            semantics->precision);
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tc_Clear_ٻٽو_انگ(significandParts(), 0);

  أرجع opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
منطقي IEEEFloat::ڦيرو_Away_From_Zero(rounding_گهڻائي rounding_گهڻائي_2,
                                  گم_Fraction گم_fraction,
                                  بنانشان صحيح ٻٽو_انگ) ثابت {
  /* NaNs and infinities should not have lost fractions.  */
  أكد(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  أكد(گم_fraction != lfExactlyZero);

  بدل (rounding_گهڻائي_2) {
  حالة rm_سڀ_کان_ويجهو_Ties_منزل_Away:
    أرجع گم_fraction == lfExactlyHalf || گم_fraction == lf_وڌيڪ_Than_Half;

  حالة rm_سڀ_کان_ويجهو_Ties_منزل_Even:
    إذا (گم_fraction == lf_وڌيڪ_Than_Half)
      أرجع سچ;

    /* Our zeroes don't have a significand to test.  */
    إذا (گم_fraction == lfExactlyHalf && category != fcZero)
      أرجع APInt::tc_ڪڍو_ٻٽو_انگ(significandParts(), ٻٽو_انگ);

    أرجع خطأ;

  حالة rmTowardZero:
    أرجع خطأ;

  حالة rm_Toward_مثبت:
    أرجع !sign;

  حالة rmTowardNegative:
    أرجع sign;

  افتراضي:
    اكسر;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::op_حالت IEEEFloat::normalize(rounding_گهڻائي rounding_گهڻائي_2,
                                       گم_Fraction گم_fraction) {
  إذا (!isFiniteNonZero())
    أرجع opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  بنانشان omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  إذا (omsb || گم_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    صحيح exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    إذا (exponent + exponentChange > semantics->maxExponent)
      أرجع handle_حد_کان_مٿي(rounding_گهڻائي_2);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    إذا (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    إذا (exponentChange < 0) {
      أكد(گم_fraction == lfExactlyZero);

      سرڪايو_Significand_کاٻو(-exponentChange);

      أرجع opOK;
    }

    إذا (exponentChange > 0) {
      گم_Fraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = سرڪايو_Significand_ساڄو(exponentChange);

      گم_fraction = combine_گم_Fractions(lf, گم_fraction);

      /* Keep OMSB up-to-date.  */
      إذا (omsb > (بنانشان) exponentChange)
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
    أرجع handle_حد_کان_مٿي(rounding_گهڻائي_2);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  إذا (گم_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    إذا (omsb == 0) {
      category = fcZero;
      إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = خطأ;
      إذا (!semantics->hasZero)
        make_سڀ_کان_ننڍو_Normalized(خطأ);
    }

    أرجع opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  إذا (ڦيرو_Away_From_Zero(rounding_گهڻائي_2, گم_fraction, 0)) {
    إذا (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    إذا (omsb == (بنانشان) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      إذا (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        أرجع handle_حد_کان_مٿي(sign ? rmTowardNegative : rm_Toward_مثبت);

      سرڪايو_Significand_ساڄو(1);

      أرجع opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      أرجع handle_حد_کان_مٿي(rounding_گهڻائي_2);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  إذا (omsb == semantics->precision)
    أرجع opInexact;

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
      make_سڀ_کان_ننڍو_Normalized(خطأ);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  أرجع (op_حالت) (opUnderflow | opInexact);
}

APFloat::op_حالت IEEEFloat::جوڙيو_Or_ڪٽيو_Specials(ثابت IEEEFloat &rhs,
                                                   منطقي ڪٽيو) {
  بدل (Pack_Categories_Into_ڪنجي(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(خالي_اشارو);

  حالة Pack_Categories_Into_ڪنجي(fcZero, fcNaN):
  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcNaN):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcNaN):
    سونپيو(rhs);
    [[fallthrough]];
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcZero):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fc_عام):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcNaN):
    إذا (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcZero):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fc_عام):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcZero):
    أرجع opOK;

  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ ڪٽيو;
    أرجع opOK;

  حالة Pack_Categories_Into_ڪنجي(fcZero, fc_عام):
    سونپيو(rhs);
    sign = rhs.sign ^ ڪٽيو;
    أرجع opOK;

  حالة Pack_Categories_Into_ڪنجي(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    أرجع opOK;

  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    إذا (((sign ^ rhs.sign)!=0) != ڪٽيو) {
      makeNaN();
      أرجع opInvalidOp;
    }

    أرجع opOK;

  حالة Pack_Categories_Into_ڪنجي(fc_عام, fc_عام):
    أرجع opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
گم_Fraction IEEEFloat::جوڙيو_Or_ڪٽيو_Significand(ثابت IEEEFloat &rhs,
                                                 منطقي ڪٽيو) {
  [[maybe_اڻ_استعمال_ٿيل]] integerPart حاصل = 0;
  گم_Fraction گم_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  ڪٽيو ^= static_cast<منطقي>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  صحيح بٽ = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  إذا (ڪٽيو) {
    إذا ((بٽ < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat عارضي_rhs(rhs);
    منطقي گم_fraction_is_from_rhs = خطأ;

    إذا (بٽ == 0)
      گم_fraction = lfExactlyZero;
    وإلا إذا (بٽ > 0) {
      گم_fraction = عارضي_rhs.سرڪايو_Significand_ساڄو(بٽ - 1);
      گم_fraction_is_from_rhs = سچ;
      سرڪايو_Significand_کاٻو(1);
    } وإلا {
      گم_fraction = سرڪايو_Significand_ساڄو(-بٽ - 1);
      عارضي_rhs.سرڪايو_Significand_کاٻو(1);
    }

    // Should we reverse the subtraction.
    cmp_نتيجو cmp_نتيجو_2 = compare_Absolute_قدر(عارضي_rhs);
    إذا (cmp_نتيجو_2 == cmpLessThan) {
      منطقي borrow =
          گم_fraction != lfExactlyZero && !گم_fraction_is_from_rhs;
      إذا (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        إذا (گم_fraction == lfLessThanHalf)
          گم_fraction = lf_وڌيڪ_Than_Half;
        وإلا إذا (گم_fraction == lf_وڌيڪ_Than_Half)
          گم_fraction = lfLessThanHalf;
      }
      حاصل = عارضي_rhs.ڪٽيو_Significand(*هذا, borrow);
      copySignificand(عارضي_rhs);
      sign = !sign;
    } وإلا إذا (cmp_نتيجو_2 == cmpGreaterThan) {
      منطقي borrow = گم_fraction != lfExactlyZero && گم_fraction_is_from_rhs;
      إذا (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        إذا (گم_fraction == lfLessThanHalf)
          گم_fraction = lf_وڌيڪ_Than_Half;
        وإلا إذا (گم_fraction == lf_وڌيڪ_Than_Half)
          گم_fraction = lfLessThanHalf;
      }
      حاصل = ڪٽيو_Significand(عارضي_rhs, borrow);
    } وإلا { // cmpEqual
      zeroSignificand();
      إذا (گم_fraction != lfExactlyZero && گم_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    أكد(!حاصل);
  } وإلا {
    إذا (بٽ > 0) {
      IEEEFloat عارضي_rhs(rhs);

      گم_fraction = عارضي_rhs.سرڪايو_Significand_ساڄو(بٽ);
      حاصل = جوڙيو_Significand(عارضي_rhs);
    } وإلا {
      گم_fraction = سرڪايو_Significand_ساڄو(-بٽ);
      حاصل = جوڙيو_Significand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    أكد(!حاصل);
  }

  أرجع گم_fraction;
}

APFloat::op_حالت IEEEFloat::multiplySpecials(ثابت IEEEFloat &rhs) {
  بدل (Pack_Categories_Into_ڪنجي(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(خالي_اشارو);

  حالة Pack_Categories_Into_ڪنجي(fcZero, fcNaN):
  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcNaN):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcNaN):
    سونپيو(rhs);
    sign = خطأ;
    [[fallthrough]];
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcZero):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fc_عام):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    إذا (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fc_عام):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcInfinity):
    category = fcInfinity;
    أرجع opOK;

  حالة Pack_Categories_Into_ڪنجي(fcZero, fc_عام):
  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcZero):
  حالة Pack_Categories_Into_ڪنجي(fcZero, fcZero):
    category = fcZero;
    أرجع opOK;

  حالة Pack_Categories_Into_ڪنجي(fcZero, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcZero):
    makeNaN();
    أرجع opInvalidOp;

  حالة Pack_Categories_Into_ڪنجي(fc_عام, fc_عام):
    أرجع opOK;
  }
}

APFloat::op_حالت IEEEFloat::divideSpecials(ثابت IEEEFloat &rhs) {
  بدل (Pack_Categories_Into_ڪنجي(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(خالي_اشارو);

  حالة Pack_Categories_Into_ڪنجي(fcZero, fcNaN):
  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcNaN):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcNaN):
    سونپيو(rhs);
    sign = خطأ;
    [[fallthrough]];
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcZero):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fc_عام):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    إذا (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcZero):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fc_عام):
  حالة Pack_Categories_Into_ڪنجي(fcZero, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcZero, fc_عام):
    أرجع opOK;

  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcInfinity):
    category = fcZero;
    أرجع opOK;

  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcZero):
    إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(خطأ, sign);
    وإلا
      category = fcInfinity;
    أرجع opDivByZero;

  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcZero, fcZero):
    makeNaN();
    أرجع opInvalidOp;

  حالة Pack_Categories_Into_ڪنجي(fc_عام, fc_عام):
    أرجع opOK;
  }
}

APFloat::op_حالت IEEEFloat::modSpecials(ثابت IEEEFloat &rhs) {
  بدل (Pack_Categories_Into_ڪنجي(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(خالي_اشارو);

  حالة Pack_Categories_Into_ڪنجي(fcZero, fcNaN):
  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcNaN):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcNaN):
    سونپيو(rhs);
    [[fallthrough]];
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcZero):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fc_عام):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcNaN):
    إذا (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_ڪنجي(fcZero, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcZero, fc_عام):
  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcInfinity):
    أرجع opOK;

  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcZero):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcZero):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fc_عام):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcZero, fcZero):
    makeNaN();
    أرجع opInvalidOp;

  حالة Pack_Categories_Into_ڪنجي(fc_عام, fc_عام):
    أرجع opOK;
  }
}

APFloat::op_حالت IEEEFloat::remainderSpecials(ثابت IEEEFloat &rhs) {
  بدل (Pack_Categories_Into_ڪنجي(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(خالي_اشارو);

  حالة Pack_Categories_Into_ڪنجي(fcZero, fcNaN):
  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcNaN):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcNaN):
    سونپيو(rhs);
    [[fallthrough]];
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcZero):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fc_عام):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcNaN):
    إذا (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_ڪنجي(fcZero, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcZero, fc_عام):
  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcInfinity):
    أرجع opOK;

  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcZero):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcZero):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fc_عام):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcZero, fcZero):
    makeNaN();
    أرجع opInvalidOp;

  حالة Pack_Categories_Into_ڪنجي(fc_عام, fc_عام):
    أرجع opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
فراغ IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    أرجع;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::op_حالت IEEEFloat::جوڙيو_Or_ڪٽيو(ثابت IEEEFloat &rhs,
                                           rounding_گهڻائي rounding_گهڻائي_2,
                                           منطقي ڪٽيو) {
  op_حالت fs = جوڙيو_Or_ڪٽيو_Specials(rhs, ڪٽيو);

  /* This return code means it was not a simple case.  */
  إذا (fs == opDivByZero) {
    گم_Fraction گم_fraction;

    گم_fraction = جوڙيو_Or_ڪٽيو_Significand(rhs, ڪٽيو);
    fs = normalize(rounding_گهڻائي_2, گم_fraction);

    /* Can only be zero if we lost no fraction.  */
    أكد(category != fcZero || گم_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  إذا (category == fcZero) {
    إذا (rhs.category != fcZero || (sign == rhs.sign) == ڪٽيو)
      sign = (rounding_گهڻائي_2 == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = خطأ;
  }

  أرجع fs;
}

/* Normalized addition.  */
APFloat::op_حالت IEEEFloat::جوڙيو(ثابت IEEEFloat &rhs,
                                 rounding_گهڻائي rounding_گهڻائي_2) {
  أرجع جوڙيو_Or_ڪٽيو(rhs, rounding_گهڻائي_2, خطأ);
}

/* Normalized subtraction.  */
APFloat::op_حالت IEEEFloat::ڪٽيو(ثابت IEEEFloat &rhs,
                                      rounding_گهڻائي rounding_گهڻائي_2) {
  أرجع جوڙيو_Or_ڪٽيو(rhs, rounding_گهڻائي_2, سچ);
}

/* Normalized multiply.  */
APFloat::op_حالت IEEEFloat::multiply(ثابت IEEEFloat &rhs,
                                      rounding_گهڻائي rounding_گهڻائي_2) {
  sign ^= rhs.sign;
  op_حالت fs = multiplySpecials(rhs);

  إذا (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = خطأ;
  إذا (isFiniteNonZero()) {
    گم_Fraction گم_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_گهڻائي_2, گم_fraction);
    إذا (گم_fraction != lfExactlyZero)
      fs = (op_حالت) (fs | opInexact);
  }

  أرجع fs;
}

/* Normalized divide.  */
APFloat::op_حالت IEEEFloat::divide(ثابت IEEEFloat &rhs,
                                    rounding_گهڻائي rounding_گهڻائي_2) {
  sign ^= rhs.sign;
  op_حالت fs = divideSpecials(rhs);

  إذا (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = خطأ;
  إذا (isFiniteNonZero()) {
    گم_Fraction گم_fraction = divideSignificand(rhs);
    fs = normalize(rounding_گهڻائي_2, گم_fraction);
    إذا (گم_fraction != lfExactlyZero)
      fs = (op_حالت) (fs | opInexact);
  }

  أرجع fs;
}

/* Normalized remainder.  */
APFloat::op_حالت IEEEFloat::remainder(ثابت IEEEFloat &rhs) {
  بنانشان صحيح origSign = sign;

  // First handle the special cases.
  op_حالت fs = remainderSpecials(rhs);
  إذا (fs != opDivByZero)
    أرجع fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  إذا (P2.جوڙيو(rhs, rm_سڀ_کان_ويجهو_Ties_منزل_Even) == opOK) {
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
  fs = VEx.convert(extendedSemantics, rm_سڀ_کان_ويجهو_Ties_منزل_Even, &losesInfo);
  أكد(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rm_سڀ_کان_ويجهو_Ties_منزل_Even, &losesInfo);
  أكد(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.جوڙيو(VEx, rm_سڀ_کان_ويجهو_Ties_منزل_Even);
  أكد(fs == opOK);

  إذا (VEx.compare(PEx) == cmpGreaterThan) {
    fs = ڪٽيو(P, rm_سڀ_کان_ويجهو_Ties_منزل_Even);
    أكد(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.ڪٽيو(PEx, rm_سڀ_کان_ويجهو_Ties_منزل_Even);
    أكد(fs == opOK);
    fs = VEx.ڪٽيو(PEx, rm_سڀ_کان_ويجهو_Ties_منزل_Even);
    أكد(fs == opOK);

    cmp_نتيجو نتيجو_2 = VEx.compare(PEx);
    إذا (نتيجو_2 == cmpGreaterThan || نتيجو_2 == cmpEqual) {
      fs = ڪٽيو(P, rm_سڀ_کان_ويجهو_Ties_منزل_Even);
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
  أرجع fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::op_حالت IEEEFloat::mod(ثابت IEEEFloat &rhs) {
  op_حالت fs = modSpecials(rhs);
  بنانشان صحيح origSign = sign;

  طالما (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_قدر(rhs) != cmpLessThan) {
    صحيح Exp = ilogb(*هذا) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rm_سڀ_کان_ويجهو_Ties_منزل_Even);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    إذا (V.isNaN() || compare_Absolute_قدر(V) == cmpLessThan)
      V = scalbn(rhs, Exp - 1, rm_سڀ_کان_ويجهو_Ties_منزل_Even);
    V.sign = sign;

    fs = ڪٽيو(V, rm_سڀ_کان_ويجهو_Ties_منزل_Even);

    // When the semantics supports zero, this loop's
    // exit-condition is handled by the 'isFiniteNonZero'
    // category check above. However, when the semantics
    // does not have 'fcZero' and we have reached the
    // minimum possible value, (and any further subtract
    // will underflow to the same value) explicitly
    // provide an exit-path here.
    إذا (!semantics->hasZero && هذا->is_سڀ_کان_ننڍو())
      اكسر;

    أكد(fs==opOK);
  }
  إذا (isZero()) {
    sign = origSign; // fmod requires this
    إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = خطأ;
  }
  أرجع fs;
}

/* Normalized fused-multiply-add.  */
APFloat::op_حالت IEEEFloat::fused_Multiply_جوڙيو(ثابت IEEEFloat &multiplicand,
                                              ثابت IEEEFloat &addend,
                                              rounding_گهڻائي rounding_گهڻائي_2) {
  op_حالت fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  إذا (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    گم_Fraction گم_fraction;

    گم_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_گهڻائي_2, گم_fraction);
    إذا (گم_fraction != lfExactlyZero)
      fs = (op_حالت) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    إذا (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_گهڻائي_2 == rmTowardNegative);
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
      fs = جوڙيو_Or_ڪٽيو(addend, rounding_گهڻائي_2, خطأ);
  }

  أرجع fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::op_حالت IEEEFloat::ڦيرو_منزل_Integral(rounding_گهڻائي rounding_گهڻائي_2) {
  إذا (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    أرجع opOK;

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

  إذا (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    أرجع opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  إذا (exponent + 1 >= (صحيح)APFloat::semanticsPrecision(*semantics))
    أرجع opOK;

  // The algorithm here is quite simple: we add 2^(p-1), where p is the
  // precision of our format, and then subtract it back off again.  The choice
  // of rounding modes for the addition/subtraction determines the rounding mode
  // for our integral rounding as well.
  // NOTE: When the input value is negative, we do subtraction followed by
  // addition instead.
  APInt IntegerConstant(ايندڙ_Power_Of2(APFloat::semanticsPrecision(*semantics)),
                        1);
  IntegerConstant <<= APFloat::semanticsPrecision(*semantics) - 1;
  IEEEFloat MagicConstant(*semantics);
  op_حالت fs = MagicConstant.convertFromAPInt(IntegerConstant, خطأ,
                                               rm_سڀ_کان_ويجهو_Ties_منزل_Even);
  أكد(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  منطقي داخلا_Sign = isNegative();

  fs = جوڙيو(MagicConstant, rounding_گهڻائي_2);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  ڪٽيو(MagicConstant, rounding_گهڻائي_2);

  // Restore the input sign.
  إذا (داخلا_Sign != isNegative())
    changeSign();

  أرجع fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmp_نتيجو IEEEFloat::compare(ثابت IEEEFloat &rhs) ثابت {
  أكد(semantics == rhs.semantics);

  بدل (Pack_Categories_Into_ڪنجي(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(خالي_اشارو);

  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcZero):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fc_عام):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcNaN, fcNaN):
  حالة Pack_Categories_Into_ڪنجي(fcZero, fcNaN):
  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcNaN):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcNaN):
    أرجع cmpUnordered;

  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fc_عام):
  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcZero):
  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcZero):
    إذا (sign)
      أرجع cmpLessThan;
    وإلا
      أرجع cmpGreaterThan;

  حالة Pack_Categories_Into_ڪنجي(fc_عام, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcZero, fcInfinity):
  حالة Pack_Categories_Into_ڪنجي(fcZero, fc_عام):
    إذا (rhs.sign)
      أرجع cmpGreaterThan;
    وإلا
      أرجع cmpLessThan;

  حالة Pack_Categories_Into_ڪنجي(fcInfinity, fcInfinity):
    إذا (sign == rhs.sign)
      أرجع cmpEqual;
    وإلا إذا (sign)
      أرجع cmpLessThan;
    وإلا
      أرجع cmpGreaterThan;

  حالة Pack_Categories_Into_ڪنجي(fcZero, fcZero):
    أرجع cmpEqual;

  حالة Pack_Categories_Into_ڪنجي(fc_عام, fc_عام):
    اكسر;
  }

  cmp_نتيجو نتيجو_2;
  /* Two normal numbers.  Do they have the same sign?  */
  إذا (sign != rhs.sign) {
    إذا (sign)
      نتيجو_2 = cmpLessThan;
    وإلا
      نتيجو_2 = cmpGreaterThan;
  } وإلا {
    /* Compare absolute values; invert result if negative.  */
    نتيجو_2 = compare_Absolute_قدر(rhs);

    إذا (sign) {
      إذا (نتيجو_2 == cmpLessThan)
        نتيجو_2 = cmpGreaterThan;
      وإلا إذا (نتيجو_2 == cmpGreaterThan)
        نتيجو_2 = cmpLessThan;
    }
  }

  أرجع نتيجو_2;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::op_حالت IEEEFloat::convert(ثابت fltSemantics &منزل_Semantics,
                                     rounding_گهڻائي rounding_گهڻائي_2,
                                     منطقي *losesInfo) {
  op_حالت fs;
  ثابت fltSemantics &fromSemantics = *semantics;
  منطقي is_signaling = isSignaling();

  گم_Fraction گم_Fraction = lfExactlyZero;
  بنانشان new_Part_ڳڻپ = part_ڳڻپ_For_بٽ(منزل_Semantics.precision + 1);
  بنانشان old_Part_ڳڻپ = part_ڳڻپ();
  صحيح سرڪايو = منزل_Semantics.precision - fromSemantics.precision;

  منطقي X86SpecialNan = خطأ;
  إذا (&fromSemantics == &APFloat_بنياد::semX87DoubleExtended &&
      &منزل_Semantics != &APFloat_بنياد::semX87DoubleExtended && category == fcNaN &&
      (!(*significandParts() & 0x8000000000000000ULL) ||
       !(*significandParts() & 0x4000000000000000ULL))) {
    // x86 has some unusual NaNs which cannot be represented in any other
    // format; note them here.
    X86SpecialNan = سچ;
  }

  // If this is a truncation of a denormal number, and the target semantics
  // has larger exponent range than the source semantics (this can happen
  // when truncating from PowerPC double-double to double format), the
  // right shift could lose result mantissa bits.  Adjust exponent instead
  // of performing excessive shift.
  // Also do a similar trick in case shifting denormal would produce zero
  // significand as this case isn't handled correctly by normalize.
  إذا (سرڪايو < 0 && isFiniteNonZero()) {
    صحيح omsb = significandMSB() + 1;
    صحيح exponentChange = omsb - fromSemantics.precision;
    إذا (exponent + exponentChange < منزل_Semantics.minExponent)
      exponentChange = منزل_Semantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, سرڪايو);
    إذا (exponentChange < 0) {
      سرڪايو -= exponentChange;
      exponent += exponentChange;
    } وإلا إذا (omsb <= -سرڪايو) {
      exponentChange = omsb + سرڪايو - 1; // leave at least one bit set
      سرڪايو -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  إذا (سرڪايو < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    گم_Fraction = سرڪايو_ساڄو(significandParts(), old_Part_ڳڻپ, -سرڪايو);

  // Fix the storage so it can hold to new value.
  إذا (new_Part_ڳڻپ > old_Part_ڳڻپ) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = جديد integerPart[new_Part_ڳڻپ];
    APInt::tc_سيٽ(newParts, 0, new_Part_ڳڻپ);
    إذا (isFiniteNonZero() || category==fcNaN)
      APInt::tc_سونپيو(newParts, significandParts(), old_Part_ڳڻپ);
    freeSignificand();
    significand.parts = newParts;
  } وإلا إذا (new_Part_ڳڻپ == 1 && old_Part_ڳڻپ != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    إذا (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &منزل_Semantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  إذا (سرڪايو > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_سرڪايو_کاٻو(significandParts(), new_Part_ڳڻپ, سرڪايو);

  إذا (isFiniteNonZero()) {
    fs = normalize(rounding_گهڻائي_2, گم_Fraction);
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

      *losesInfo = گم_Fraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      إذا (!X86SpecialNan && semantics == &APFloat_بنياد::semX87DoubleExtended)
        APInt::tc_سيٽ_ٻٽو_انگ(significandParts(), semantics->precision - 1);

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
    *losesInfo = سچ;
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
    *losesInfo = سچ;
    إذا (fs == opOK)
      fs = opInexact;
  }

  إذا (category == fcZero && !semantics->hasZero)
    make_سڀ_کان_ننڍو_Normalized(خطأ);
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
APFloat::op_حالت IEEEFloat::convert_منزل_Sign_Extended_Integer(
    MutableArrayRef<integerPart> parts, بنانشان صحيح ويڪر_2, منطقي isSigned,
    rounding_گهڻائي rounding_گهڻائي_2, منطقي *isExact) ثابت {
  *isExact = خطأ;

  /* Handle the three special cases first.  */
  إذا (category == fcInfinity || category == fcNaN)
    أرجع opInvalidOp;

  بنانشان dst_Parts_ڳڻپ = part_ڳڻپ_For_بٽ(ويڪر_2);
  أكد(dst_Parts_ڳڻپ <= parts.سائيز_2() && "Integer too big");

  إذا (category == fcZero) {
    APInt::tc_سيٽ(parts.معلومات(), 0, dst_Parts_ڳڻپ);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    أرجع opOK;
  }

  ثابت integerPart *src = significandParts();

  بنانشان truncated_بٽ;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  إذا (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tc_سيٽ(parts.معلومات(), 0, dst_Parts_ڳڻپ);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncated_بٽ = semantics->precision -1U - exponent;
  } وإلا {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    بنانشان صحيح بٽ = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    إذا (بٽ > ويڪر_2)
      أرجع opInvalidOp;

    إذا (بٽ < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncated_بٽ = semantics->precision - بٽ;
      APInt::tc_ڪڍو(parts.معلومات(), dst_Parts_ڳڻپ, src, بٽ, truncated_بٽ);
    } وإلا {
      /* We want at least as many bits as are available.  */
      APInt::tc_ڪڍو(parts.معلومات(), dst_Parts_ڳڻپ, src, semantics->precision,
                       0);
      APInt::tc_سرڪايو_کاٻو(parts.معلومات(), dst_Parts_ڳڻپ,
                         بٽ - semantics->precision);
      truncated_بٽ = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  گم_Fraction گم_fraction;
  إذا (truncated_بٽ) {
    گم_fraction = گم_Fraction_Through_Truncation(src, part_ڳڻپ(),
                                                  truncated_بٽ);
    إذا (گم_fraction != lfExactlyZero &&
        ڦيرو_Away_From_Zero(rounding_گهڻائي_2, گم_fraction, truncated_بٽ)) {
      إذا (APInt::tcIncrement(parts.معلومات(), dst_Parts_ڳڻپ))
        أرجع opInvalidOp;     /* Overflow.  */
    }
  } وإلا {
    گم_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  بنانشان صحيح omsb = APInt::tcMSB(parts.معلومات(), dst_Parts_ڳڻپ) + 1;

  إذا (sign) {
    إذا (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      إذا (omsb != 0)
        أرجع opInvalidOp;
    } وإلا {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      إذا (omsb == ويڪر_2 &&
          APInt::tcLSB(parts.معلومات(), dst_Parts_ڳڻپ) + 1 != omsb)
        أرجع opInvalidOp;

      /* This case can happen because of rounding.  */
      إذا (omsb > ويڪر_2)
        أرجع opInvalidOp;
    }

    APInt::tcNegate (parts.معلومات(), dst_Parts_ڳڻپ);
  } وإلا {
    إذا (omsb >= ويڪر_2 + !isSigned)
      أرجع opInvalidOp;
  }

  إذا (گم_fraction == lfExactlyZero) {
    *isExact = سچ;
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
APFloat::op_حالت
IEEEFloat::convert_منزل_Integer(MutableArrayRef<integerPart> parts,
                            بنانشان صحيح ويڪر_2, منطقي isSigned,
                            rounding_گهڻائي rounding_گهڻائي_2, منطقي *isExact) ثابت {
  op_حالت fs = convert_منزل_Sign_Extended_Integer(parts, ويڪر_2, isSigned,
                                             rounding_گهڻائي_2, isExact);

  إذا (fs == opInvalidOp) {
    بنانشان صحيح بٽ, dst_Parts_ڳڻپ;

    dst_Parts_ڳڻپ = part_ڳڻپ_For_بٽ(ويڪر_2);
    أكد(dst_Parts_ڳڻپ <= parts.سائيز_2() && "Integer too big");

    إذا (category == fcNaN)
      بٽ = 0;
    وإلا إذا (sign)
      بٽ = isSigned;
    وإلا
      بٽ = ويڪر_2 - isSigned;

    tc_سيٽ_گهٽ_کان_گهٽ_Significant_بٽ(parts.معلومات(), dst_Parts_ڳڻپ, بٽ);
    إذا (sign && isSigned)
      APInt::tc_سرڪايو_کاٻو(parts.معلومات(), dst_Parts_ڳڻپ, ويڪر_2 - 1);
  }

  أرجع fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::op_حالت IEEEFloat::convertFromUnsignedParts(
    ثابت integerPart *src, بنانشان صحيح src_ڳڻپ, rounding_گهڻائي rounding_گهڻائي_2) {
  category = fc_عام;
  بنانشان omsb = APInt::tcMSB(src, src_ڳڻپ) + 1;
  integerPart *dst = significandParts();
  بنانشان dst_ڳڻپ = part_ڳڻپ();
  بنانشان precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  گم_Fraction گم_fraction;
  إذا (precision <= omsb) {
    exponent = omsb - 1;
    گم_fraction = گم_Fraction_Through_Truncation(src, src_ڳڻپ,
                                                  omsb - precision);
    APInt::tc_ڪڍو(dst, dst_ڳڻپ, src, precision, omsb - precision);
  } وإلا {
    exponent = precision - 1;
    گم_fraction = lfExactlyZero;
    APInt::tc_ڪڍو(dst, dst_ڳڻپ, src, omsb, 0);
  }

  أرجع normalize(rounding_گهڻائي_2, گم_fraction);
}

APFloat::op_حالت IEEEFloat::convertFromAPInt(ثابت APInt &Val, منطقي isSigned,
                                              rounding_گهڻائي rounding_گهڻائي_2) {
  بنانشان صحيح part_ڳڻپ = Val.get_انگ_لفظ();
  APInt api = Val;

  sign = خطأ;
  إذا (isSigned && api.isNegative()) {
    sign = سچ;
    api = -api;
  }

  أرجع convertFromUnsignedParts(api.get_Raw_معلومات(), part_ڳڻپ, rounding_گهڻائي_2);
}

Expected<APFloat::op_حالت>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        rounding_گهڻائي rounding_گهڻائي_2) {
  گم_Fraction گم_fraction = lfExactlyZero;

  category = fc_عام;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  بنانشان parts_ڳڻپ = part_ڳڻپ();
  بنانشان ٻٽو_انگ_Pos = parts_ڳڻپ * integer_Part_ويڪر;
  منطقي computedTrailingFraction = خطأ;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator پڇاڙي = s.پڇاڙي();
  StringRef::iterator dot;
  خودڪار PtrOrErr = skipLeadingZeroesAndAnyDot(begin, پڇاڙي, &dot);
  إذا (!PtrOrErr)
    أرجع PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator پهريون_Significant_Digit = p;

  طالما (p != پڇاڙي) {
    integerPart hex_قدر;

    إذا (*p == '.') {
      إذا (dot != پڇاڙي)
        أرجع createError("String contains multiple dots");
      dot = p++;
      استمر;
    }

    hex_قدر = hex_Digit_قدر(*p);
    إذا (hex_قدر == UINT_MAX)
      اكسر;

    p++;

    // Store the number while we have space.
    إذا (ٻٽو_انگ_Pos) {
      ٻٽو_انگ_Pos -= 4;
      hex_قدر <<= ٻٽو_انگ_Pos % integer_Part_ويڪر;
      significand[ٻٽو_انگ_Pos / integer_Part_ويڪر] |= hex_قدر;
    } وإلا إذا (!computedTrailingFraction) {
      خودڪار FractOrErr = trailingHexadecimalFraction(p, پڇاڙي, hex_قدر);
      إذا (!FractOrErr)
        أرجع FractOrErr.takeError();
      گم_fraction = *FractOrErr;
      computedTrailingFraction = سچ;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  إذا (p == پڇاڙي)
    أرجع createError("Hex strings require an exponent");
  إذا (*p != 'p' && *p != 'P')
    أرجع createError("Invalid character in significand");
  إذا (p == begin)
    أرجع createError("Significand has no digits");
  إذا (dot != پڇاڙي && p - begin == 1)
    أرجع createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  إذا (p != پهريون_Significant_Digit) {
    صحيح expAdjustment;

    /* Implicit hexadecimal point?  */
    إذا (dot == پڇاڙي)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<صحيح>(dot - پهريون_Significant_Digit);
    إذا (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_ڳڻپ * integer_Part_ويڪر;

    /* Adjust for the given exponent.  */
    خودڪار ExpOrErr = ڪل_Exponent(p + 1, پڇاڙي, expAdjustment);
    إذا (!ExpOrErr)
      أرجع ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  أرجع normalize(rounding_گهڻائي_2, گم_fraction);
}

APFloat::op_حالت
IEEEFloat::ڦيرو_Significand_With_Exponent(ثابت integerPart *decSigParts,
                                        بنانشان sig_Part_ڳڻپ, صحيح قوت_نما,
                                        rounding_گهڻائي rounding_گهڻائي_2) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  بنانشان صحيح power = قوت_نما >= 0 ? قوت_نما : -قوت_نما;
  بنانشان صحيح pow5_Part_صلاحيت = power_جو_Five_Parts_For_Exponent(power);
  Small_ويڪٽر<integerPart, انگ_Pow5_Parts_On_ڍير> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5_Part_صلاحيت);

  منطقي is_سڀ_کان_ويجهو = rounding_گهڻائي_2 == rm_سڀ_کان_ويجهو_Ties_منزل_Even ||
                   rounding_گهڻائي_2 == rm_سڀ_کان_ويجهو_Ties_منزل_Away;

  بنانشان parts = part_ڳڻپ_For_بٽ(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  بنانشان pow5_Part_ڳڻپ = powerOf5(pow5Parts.معلومات(), power);

  لكل (;; parts *= 2) {
    بنانشان صحيح excessPrecision, truncated_بٽ;

    calcSemantics.precision = parts * integer_Part_ويڪر - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncated_بٽ = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    op_حالت sig_حالت = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_ڳڻپ, rm_سڀ_کان_ويجهو_Ties_منزل_Even);
    op_حالت pow_حالت = pow5.convertFromUnsignedParts(
        pow5Parts.معلومات(), pow5_Part_ڳڻپ, rm_سڀ_کان_ويجهو_Ties_منزل_Even);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += قوت_نما;

    گم_Fraction calc_گم_Fraction;
    integerPart HUerr, HUdistance;
    بنانشان صحيح powHUerr;

    إذا (قوت_نما >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calc_گم_Fraction = decSig.multiplySignificand(pow5);
      powHUerr = pow_حالت != opOK;
    } وإلا {
      calc_گم_Fraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      إذا (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncated_بٽ = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (pow_حالت == opOK && calc_گم_Fraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    أكد(APInt::tc_ڪڍو_ٻٽو_انگ
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calc_گم_Fraction != lfExactlyZero, sig_حالت != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, is_سڀ_کان_ويجهو);

    /* Are we guaranteed to round correctly if we truncate?  */
    إذا (HUdistance >= HUerr) {
      APInt::tc_ڪڍو(significandParts(), part_ڳڻپ(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calc_گم_Fraction = گم_Fraction_Through_Truncation(decSig.significandParts(),
                                                       decSig.part_ڳڻپ(),
                                                       truncated_بٽ);
      أرجع static_cast<op_حالت>(normalize(rounding_گهڻائي_2, calc_گم_Fraction) |
                                   ((sig_حالت | pow_حالت) & opInexact));
    }
  }
}

Expected<APFloat::op_حالت>
IEEEFloat::convertFromDecimalString(StringRef str, rounding_گهڻائي rounding_گهڻائي_2) {
  decimalInfo D;
  op_حالت fs;

  /* Scan the text.  */
  StringRef::iterator p = str.begin();
  إذا (Error Err = interpretDecimal(p, str.پڇاڙي(), &D))
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
  إذا (D.پهريون_Sig_Digit == str.پڇاڙي() || dec_Digit_قدر(*D.پهريون_Sig_Digit) >= 10U) {
    category = fcZero;
    fs = opOK;
    إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = خطأ;
    إذا (!semantics->hasZero)
      make_سڀ_کان_ننڍو_Normalized(خطأ);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } وإلا إذا (D.normalizedExponent - 1 > وڌ_کان_وڌ_عدد / 42039) {
    fs = handle_حد_کان_مٿي(rounding_گهڻائي_2);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } وإلا إذا (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (صحيح) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fc_عام;
    zeroSignificand();
    fs = normalize(rounding_گهڻائي_2, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } وإلا إذا ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handle_حد_کان_مٿي(rounding_گهڻائي_2);
  } وإلا {
    integerPart *decSignificand;
    بنانشان صحيح part_ڳڻپ;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_ڳڻپ = static_cast<بنانشان صحيح>(D.آخري_Sig_Digit - D.پهريون_Sig_Digit) + 1;
    part_ڳڻپ = part_ڳڻپ_For_بٽ(1 + 196 * part_ڳڻپ / 59);
    decSignificand = جديد integerPart[part_ڳڻپ + 1];
    part_ڳڻپ = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    افعل {
      integerPart dec_قدر, val, multiplier;

      val = 0;
      multiplier = 1;

      افعل {
        إذا (*p == '.') {
          p++;
          إذا (p == str.پڇاڙي()) {
            اكسر;
          }
        }
        dec_قدر = dec_Digit_قدر(*p++);
        إذا (dec_قدر >= 10U) {
          ميٽيو[] decSignificand;
          أرجع createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_قدر;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } طالما (p <= D.آخري_Sig_Digit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_ڳڻپ, part_ڳڻپ + 1, خطأ);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      إذا (decSignificand[part_ڳڻپ])
        part_ڳڻپ++;
    } طالما (p <= D.آخري_Sig_Digit);

    category = fc_عام;
    fs = ڦيرو_Significand_With_Exponent(decSignificand, part_ڳڻپ,
                                      D.exponent, rounding_گهڻائي_2);

    ميٽيو [] decSignificand;
  }

  أرجع fs;
}

منطقي IEEEFloat::convertFromStringSpecials(StringRef str) {
  ثابت size_t MIN_NAME_سائيز = 3;

  إذا (str.سائيز_2() < MIN_NAME_سائيز)
    أرجع خطأ;

  إذا (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(خطأ);
    أرجع سچ;
  }

  منطقي IsNegative = str.consume_front("-");
  إذا (IsNegative) {
    إذا (str.سائيز_2() < MIN_NAME_سائيز)
      أرجع خطأ;

    إذا (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(سچ);
      أرجع سچ;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  منطقي IsSignaling = str.consume_front_insensitive("s");
  إذا (IsSignaling) {
    إذا (str.سائيز_2() < MIN_NAME_سائيز)
      أرجع خطأ;
  }

  إذا (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    إذا (str.خالي_جايون()) {
      makeNaN(IsSignaling, IsNegative);
      أرجع سچ;
    }

    // Allow the payload to be inside parentheses.
    إذا (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      إذا (str.سائيز_2() <= 2 || str.پٺ() != ')')
        أرجع خطأ;

      str = str.ٽڪرو(1, str.سائيز_2() - 1);
    }

    // Determine the payload number's radix.
    بنانشان Radix = 10;
    إذا (str[0] == '0') {
      إذا (str.سائيز_2() > 1 && tolower(str[1]) == 'x') {
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
      أرجع سچ;
    }
  }

  أرجع خطأ;
}

Expected<APFloat::op_حالت>
IEEEFloat::convertFromString(StringRef str, rounding_گهڻائي rounding_گهڻائي_2) {
  إذا (str.خالي_جايون())
    أرجع createError("Invalid string length");

  // Handle special cases.
  إذا (convertFromStringSpecials(str))
    أرجع opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.سائيز_2();
  sign = *p == '-' ? 1 : 0;
  إذا (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  إذا (*p == '-' || *p == '+') {
    p++;
    slen--;
    إذا (!slen)
      أرجع createError("String has no digits");
  }

  إذا (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    إذا (slen == 2)
      أرجع createError("Invalid string");
    أرجع convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_گهڻائي_2);
  }

  أرجع convertFromDecimalString(StringRef(p, slen), rounding_گهڻائي_2);
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
بنانشان صحيح IEEEFloat::convert_منزل_Hex_String(حرف *dst, بنانشان صحيح hexDigits,
                                           منطقي مٿانهون_Case,
                                           rounding_گهڻائي rounding_گهڻائي_2) ثابت {
  حرف *p = dst;
  إذا (sign)
    *dst++ = '-';

  بدل (category) {
  حالة fcInfinity:
    يادگيري_نقل (dst, مٿانهون_Case ? infinityU: infinityL, حجم infinityU - 1);
    dst += حجم infinityL - 1;
    اكسر;

  حالة fcNaN:
    يادگيري_نقل (dst, مٿانهون_Case ? NaNU: NaNL, حجم NaNU - 1);
    dst += حجم NaNU - 1;
    اكسر;

  حالة fcZero:
    *dst++ = '0';
    *dst++ = مٿانهون_Case ? 'X': 'x';
    *dst++ = '0';
    إذا (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = مٿانهون_Case ? 'P': 'p';
    *dst++ = '0';
    اكسر;

  حالة fc_عام:
    dst = convert_عام_منزل_Hex_String (dst, hexDigits, مٿانهون_Case, rounding_گهڻائي_2);
    اكسر;
  }

  *dst = 0;

  أرجع static_cast<بنانشان صحيح>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
حرف *IEEEFloat::convert_عام_منزل_Hex_String(حرف *dst, بنانشان صحيح hexDigits,
                                          منطقي مٿانهون_Case,
                                          rounding_گهڻائي rounding_گهڻائي_2) ثابت {
  *dst++ = '0';
  *dst++ = مٿانهون_Case ? 'X': 'x';

  منطقي ڦيرو_مٿي = خطأ;
  ثابت حرف *hexDigitChars = مٿانهون_Case ? hex_Digits_مٿانهون : hex_Digits_هيٺاهون;

  ثابت integerPart *significand = significandParts();
  بنانشان parts_ڳڻپ = part_ڳڻپ();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  بنانشان قدر_بٽ = semantics->precision + 3;
  بنانشان سرڪايو = integer_Part_ويڪر - قدر_بٽ % integer_Part_ويڪر;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  بنانشان outputDigits = (قدر_بٽ - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  إذا (hexDigits) {
    إذا (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      بنانشان صحيح بٽ;
      گم_Fraction fraction;

      بٽ = قدر_بٽ - hexDigits * 4;
      fraction = گم_Fraction_Through_Truncation (significand, parts_ڳڻپ, بٽ);
      ڦيرو_مٿي = ڦيرو_Away_From_Zero(rounding_گهڻائي_2, fraction, بٽ);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  حرف *p = ++dst;

  بنانشان ڳڻپ = (قدر_بٽ + integer_Part_ويڪر - 1) / integer_Part_ويڪر;

  طالما (outputDigits && ڳڻپ) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    إذا (--ڳڻپ == parts_ڳڻپ)
      part = 0;  /* An imaginary higher zero part.  */
    وإلا
      part = significand[ڳڻپ] << سرڪايو;

    إذا (ڳڻپ && سرڪايو)
      part |= significand[ڳڻپ - 1] >> (integer_Part_ويڪر - سرڪايو);

    /* Convert as much of "part" to hexdigits as we can.  */
    بنانشان صحيح curDigits = integer_Part_ويڪر / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  إذا (ڦيرو_مٿي) {
    حرف *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    افعل {
      q--;
      *q = hexDigitChars[hex_Digit_قدر (*q) + 1];
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
  إذا (dst -1 == p)
    dst--;
  وإلا
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = مٿانهون_Case ? 'P': 'p';

  أرجع writeSignedDecimal (dst, exponent);
}

hash_ڪوڊ hash_قدر(ثابت IEEEFloat &Arg) {
  إذا (!Arg.isFiniteNonZero())
    أرجع hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  أرجع hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.part_ڳڻپ()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convert_F80_Long_Double_APFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ() == 2);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semX87DoubleExtended>();
}

APInt IEEEFloat::convert_PPCDouble_Double_Legacy_APFloat_منزل_APInt() ثابت {
  أكد(semantics ==
         (ثابت llvm::fltSemantics *)&APFloat_بنياد::semPPCDoubleDoubleLegacy);
  أكد(part_ڳڻپ()==2);

  uint64_t لفظ[2];
  منطقي losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloat_بنياد::semIEEEdouble.minExponent;
  IEEEFloat extended(*هذا);
  [[maybe_اڻ_استعمال_ٿيل]] op_حالت fs =
      extended.convert(extendedSemantics, rm_سڀ_کان_ويجهو_Ties_منزل_Even, &losesInfo);
  أكد(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloat_بنياد::semIEEEdouble, rm_سڀ_کان_ويجهو_Ties_منزل_Even, &losesInfo);
  أكد(fs == opOK || fs == opInexact);
  لفظ[0] = *u.convert_Double_APFloat_منزل_APInt().get_Raw_معلومات();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  إذا (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rm_سڀ_کان_ويجهو_Ties_منزل_Even, &losesInfo);
    أكد(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.ڪٽيو(u, rm_سڀ_کان_ويجهو_Ties_منزل_Even);
    fs = v.convert(APFloat_بنياد::semIEEEdouble, rm_سڀ_کان_ويجهو_Ties_منزل_Even, &losesInfo);
    أكد(fs == opOK && !losesInfo);
    لفظ[1] = *v.convert_Double_APFloat_منزل_APInt().get_Raw_معلومات();
  } وإلا {
    لفظ[1] = 0;
  }

  أرجع APInt(128, لفظ);
}

سانچو <ثابت fltSemantics &S>
APInt IEEEFloat::convert_IEEEFloat_منزل_APInt() ثابت {
  أكد(semantics == &S);
  constexpr بنانشان صحيح trailing_significand_بٽ =
      S.precision - 1 + S.has_Explicit_Integer_ٻٽو_انگ;
  constexpr صحيح integer_ٻٽو_انگ_part = (S.precision - 1) / integer_Part_ويڪر;
  constexpr integerPart integer_ٻٽو_انگ = integerPart{1}
                                      << ((S.precision - 1) % integer_Part_ويڪر);
  constexpr uint64_t significand_نقاب = integer_ٻٽو_انگ - 1;
  constexpr بنانشان صحيح exponent_بٽ =
      S.سائيز_اندريون_بٽ - (S.hasSignedRepr ? 1 : 0) - trailing_significand_بٽ;
  static_assert(exponent_بٽ < 64);
  constexpr uint64_t exponent_نقاب = (uint64_t{1} << exponent_بٽ) - 1;
  constexpr منطقي is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr صحيح bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_ڳڻپ_For_بٽ(trailing_significand_بٽ)>
      mysignificand;

  إذا (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.سائيز_2(),
                mysignificand.begin());
    إذا (myexponent == 1 &&
        !(significandParts()[integer_ٻٽو_انگ_part] & integer_ٻٽو_انگ))
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
    إذا constexpr (S.has_Explicit_Integer_ٻٽو_انگ) {
      mysignificand[0] = integerPart{1} << (trailing_significand_بٽ - 1);
    }
  } وإلا {
    أكد(category == fcNaN && "Unknown category!");
    إذا (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.سائيز_2(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.سائيز_اندريون_بٽ + 63) / 64> لفظ;
  خودڪار لفظ_iter =
      std::copy_n(mysignificand.begin(), mysignificand.سائيز_2(), لفظ.begin());
  إذا constexpr (!S.has_Explicit_Integer_ٻٽو_انگ) {
    إذا constexpr (significand_نقاب != 0 || trailing_significand_بٽ == 0) {
      // Clear the integer bit.
      لفظ[mysignificand.سائيز_2() - 1] &= significand_نقاب;
    }
  }
  std::fill(لفظ_iter, لفظ.پڇاڙي(), uint64_t{0});
  constexpr size_t آخري_لفظ = لفظ.سائيز_2() - 1;
  إذا constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.سائيز_اندريون_بٽ - 1) % 64);
    لفظ[آخري_لفظ] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_نقاب)
                              << (trailing_significand_بٽ % 64);
  لفظ[آخري_لفظ] |= shifted_exponent;
  إذا constexpr (آخري_لفظ == 0) {
    أرجع APInt(S.سائيز_اندريون_بٽ, لفظ[0]);
  }
  أرجع APInt(S.سائيز_اندريون_بٽ, لفظ);
}

APInt IEEEFloat::convert_Quadruple_APFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ() == 2);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semIEEEquad>();
}

APInt IEEEFloat::convert_Double_APFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ()==1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semIEEEdouble>();
}

APInt IEEEFloat::convert_Float_APFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ()==1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semIEEEsingle>();
}

APInt IEEEFloat::convert_BFloat_APFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semBFloat>();
}

APInt IEEEFloat::convert_Half_APFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ()==1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::APFloat_بنياد::semIEEEhalf>();
}

APInt IEEEFloat::convert_Float8_E5_M2_APFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semFloat8E5M2>();
}

APInt IEEEFloat::convert_Float8_E5_M2_FNUZAPFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convert_Float8_E4_M3_APFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semFloat8E4M3>();
}

APInt IEEEFloat::convert_Float8_E4_M3_FNAPFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semFloat8E4M3FN>();
}

APInt IEEEFloat::convert_Float8_E4_M3_FNUZAPFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convert_Float8_E4_M3_B11_FNUZAPFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convert_Float8_E3_M4_APFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semFloat8E3M4>();
}

APInt IEEEFloat::convert_Float_TF32_APFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semFloatTF32>();
}

APInt IEEEFloat::convert_Float8_E8_M0_FNUAPFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convert_Float8_E5_M3_FNUAPFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convert_Float6_E3_M2_FNAPFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semFloat6E3M2FN>();
}

APInt IEEEFloat::convert_Float6_E2_M3_FNAPFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semFloat6E2M3FN>();
}

APInt IEEEFloat::convert_Float4_E2_M1_FNAPFloat_منزل_APInt() ثابت {
  أكد(part_ڳڻپ() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_بنياد::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcast_منزل_APInt() ثابت {
  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semIEEEhalf)
    أرجع convert_Half_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semBFloat)
    أرجع convert_BFloat_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semIEEEsingle)
    أرجع convert_Float_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semIEEEdouble)
    أرجع convert_Double_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semIEEEquad)
    أرجع convert_Quadruple_APFloat_منزل_APInt();

  إذا (semantics ==
      (ثابت llvm::fltSemantics *)&APFloat_بنياد::semX87DoubleExtended)
    أرجع convert_F80_Long_Double_APFloat_منزل_APInt();

  إذا (semantics ==
      (ثابت llvm::fltSemantics *)&APFloat_بنياد::semPPCDoubleDoubleLegacy)
    أرجع convert_PPCDouble_Double_Legacy_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semFloat8E5M2)
    أرجع convert_Float8_E5_M2_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semFloat8E5M2FNUZ)
    أرجع convert_Float8_E5_M2_FNUZAPFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semFloat8E4M3)
    أرجع convert_Float8_E4_M3_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semFloat8E4M3FN)
    أرجع convert_Float8_E4_M3_FNAPFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semFloat8E4M3FNUZ)
    أرجع convert_Float8_E4_M3_FNUZAPFloat_منزل_APInt();

  إذا (semantics ==
      (ثابت llvm::fltSemantics *)&APFloat_بنياد::semFloat8E4M3B11FNUZ)
    أرجع convert_Float8_E4_M3_B11_FNUZAPFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semFloat8E3M4)
    أرجع convert_Float8_E3_M4_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semFloatTF32)
    أرجع convert_Float_TF32_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semFloat8E8M0FNU)
    أرجع convert_Float8_E8_M0_FNUAPFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semFloat8E5M3FNU)
    أرجع convert_Float8_E5_M3_FNUAPFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semFloat6E3M2FN)
    أرجع convert_Float6_E3_M2_FNAPFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semFloat6E2M3FN)
    أرجع convert_Float6_E2_M3_FNAPFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semFloat4E2M1FN)
    أرجع convert_Float4_E2_M1_FNAPFloat_منزل_APInt();

  llvm_unreachable("unknown format!");
}

عائم IEEEFloat::convert_منزل_Float() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcast_منزل_APInt();
  أرجع api.بٽ_منزل_Float();
}

مزدوج IEEEFloat::convert_منزل_Double() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcast_منزل_APInt();
  أرجع api.بٽ_منزل_Double();
}

#جيڪڏهن_وصف_ٿيل HAS_IEE754_FLOAT128
float128 IEEEFloat::convert_منزل_Quad() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloat_بنياد::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcast_منزل_APInt();
  أرجع api.بٽ_منزل_Quad();
}
#جيڪڏهن_پورو

فراغ IEEEFloat::initFromF80LongDoubleAPInt(ثابت APInt &api) {
  أرجع initFromIEEEAPInt<APFloat_بنياد::semX87DoubleExtended>(api);
}

فراغ IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(ثابت APInt &api) {
  uint64_t i1 = api.get_Raw_معلومات()[0];
  uint64_t i2 = api.get_Raw_معلومات()[1];
  منطقي losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_اڻ_استعمال_ٿيل]] op_حالت fs = convert(APFloat_بنياد::semPPCDoubleDoubleLegacy,
                                         rm_سڀ_کان_ويجهو_Ties_منزل_Even, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  أكد((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  إذا (isFiniteNonZero()) {
    IEEEFloat v(APFloat_بنياد::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    إذا (v.isFinite()) {
      fs = v.convert(APFloat_بنياد::semPPCDoubleDoubleLegacy, rm_سڀ_کان_ويجهو_Ties_منزل_Even,
                     &losesInfo);
      أكد(fs == opOK && !losesInfo);

      جوڙيو(v, rm_سڀ_کان_ويجهو_Ties_منزل_Even);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
فراغ IEEEFloat::initFromFloat8E8M0FNUAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semFloat8E8M0FNU>(api);
}

فراغ IEEEFloat::initFromFloat8E5M3FNUAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semFloat8E5M3FNU>(api);
}

سانچو <ثابت fltSemantics &S>
فراغ IEEEFloat::initFromIEEEAPInt(ثابت APInt &api) {
  أكد(api.get_ٻٽو_انگ_ويڪر() == S.سائيز_اندريون_بٽ);

  constexpr بنانشان صحيح trailing_significand_بٽ =
      S.precision - 1 + S.has_Explicit_Integer_ٻٽو_انگ;
  constexpr integerPart integer_ٻٽو_انگ =
      integerPart{1} << (trailing_significand_بٽ % integer_Part_ويڪر);
  constexpr uint64_t significand_نقاب = integer_ٻٽو_انگ - 1;
  constexpr بنانشان صحيح exponent_بٽ =
      S.سائيز_اندريون_بٽ - (S.hasSignedRepr ? 1 : 0) - trailing_significand_بٽ;
  static_assert(exponent_بٽ < 64);
  constexpr بنانشان صحيح stored_significand_parts =
      part_ڳڻپ_For_بٽ(trailing_significand_بٽ + 1);
  constexpr uint64_t exponent_نقاب = (uint64_t{1} << exponent_بٽ) - 1;
  constexpr منطقي is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr صحيح bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr منطقي has_significand = trailing_significand_بٽ > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  إذا constexpr (has_significand) {
    std::copy_n(api.get_Raw_معلومات(), mysignificand.سائيز_2(), mysignificand.begin());
    إذا constexpr (significand_نقاب != 0 || S.precision >= integer_Part_ويڪر) {
      mysignificand[mysignificand.سائيز_2() - 1] &= significand_نقاب;
    }
  } وإلا {
    std::fill_n(mysignificand.begin(), mysignificand.سائيز_2(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t آخري_لفظ = api.get_Raw_معلومات()[api.get_انگ_لفظ() - 1];
  uint64_t myexponent =
      (آخري_لفظ >> (trailing_significand_بٽ % 64)) & exponent_نقاب;

  initialize(&S);
  أكد(part_ڳڻپ() == mysignificand.سائيز_2());

  sign = S.hasSignedRepr
             ? static_cast<بنانشان صحيح>(آخري_لفظ >> ((S.سائيز_اندريون_بٽ - 1) % 64))
             : 0;

  منطقي all_zero_significand =
      has_significand && llvm::all_جو(mysignificand, equal_منزل(0));

  منطقي is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  إذا constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    منطقي is_inf = خطأ;

    إذا constexpr (S.has_Explicit_Integer_ٻٽو_انگ) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_نقاب_no_int_ٻٽو_انگ =
          (uint64_t{1} << (trailing_significand_بٽ - 1)) - 1;
      ثابت integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_بٽ - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_نقاب_no_int_ٻٽو_انگ) == 0;
    } وإلا {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    إذا (is_inf) {
      makeInf(sign);
      أرجع;
    }
  }

  منطقي is_nan = خطأ;

  إذا constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    إذا constexpr (S.has_Explicit_Integer_ٻٽو_انگ) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      ثابت integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_بٽ - 1);
      constexpr integerPart significand_نقاب_no_int_ٻٽو_انگ =
          (uint64_t{1} << (trailing_significand_بٽ - 1)) - 1;

      إذا (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_نقاب_no_int_ٻٽو_انگ) != 0) {
        // regular NaN and pseudoNaN
        is_nan = سچ;
      } وإلا إذا (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_نقاب_no_int_ٻٽو_انگ) == 0) {
        // pseudoinfinity
        is_nan = سچ;
      } وإلا إذا (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = سچ;
      }
    } وإلا {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } وإلا إذا constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    منطقي all_ones_significand =
        std::all_جو(mysignificand.begin(), mysignificand.پڇاڙي() - 1,
                    [](integerPart بٽ) { أرجع بٽ == ~integerPart{0}; }) &&
        (!significand_نقاب ||
         mysignificand[mysignificand.سائيز_2() - 1] == significand_نقاب);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } وإلا إذا constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  إذا (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.سائيز_2(),
                significandParts());
    أرجع;
  }

  إذا (is_zero) {
    makeZero(sign);
    أرجع;
  }

  category = fc_عام;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.سائيز_2(), significandParts());
  إذا (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  وإلا {
    إذا constexpr (!S.has_Explicit_Integer_ٻٽو_انگ) {
      significandParts()[mysignificand.سائيز_2() - 1] |= integer_ٻٽو_انگ;
    }
  }
}

فراغ IEEEFloat::initFromQuadrupleAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semIEEEquad>(api);
}

فراغ IEEEFloat::initFromDoubleAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semIEEEdouble>(api);
}

فراغ IEEEFloat::initFromFloatAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semIEEEsingle>(api);
}

فراغ IEEEFloat::initFromBFloatAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semBFloat>(api);
}

فراغ IEEEFloat::initFromHalfAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semIEEEhalf>(api);
}

فراغ IEEEFloat::initFromFloat8E5M2APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semFloat8E5M2>(api);
}

فراغ IEEEFloat::initFromFloat8E5M2FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semFloat8E5M2FNUZ>(api);
}

فراغ IEEEFloat::initFromFloat8E4M3APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semFloat8E4M3>(api);
}

فراغ IEEEFloat::initFromFloat8E4M3FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semFloat8E4M3FN>(api);
}

فراغ IEEEFloat::initFromFloat8E4M3FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semFloat8E4M3FNUZ>(api);
}

فراغ IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semFloat8E4M3B11FNUZ>(api);
}

فراغ IEEEFloat::initFromFloat8E3M4APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semFloat8E3M4>(api);
}

فراغ IEEEFloat::initFromFloatTF32APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semFloatTF32>(api);
}

فراغ IEEEFloat::initFromFloat6E3M2FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semFloat6E3M2FN>(api);
}

فراغ IEEEFloat::initFromFloat6E2M3FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semFloat6E2M3FN>(api);
}

فراغ IEEEFloat::initFromFloat4E2M1FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنياد::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
فراغ IEEEFloat::initFromAPInt(ثابت fltSemantics *Sem, ثابت APInt &api) {
  أكد(api.get_ٻٽو_انگ_ويڪر() == Sem->سائيز_اندريون_بٽ);
  إذا (Sem == &APFloat_بنياد::semIEEEhalf)
    أرجع initFromHalfAPInt(api);
  إذا (Sem == &APFloat_بنياد::semBFloat)
    أرجع initFromBFloatAPInt(api);
  إذا (Sem == &APFloat_بنياد::semIEEEsingle)
    أرجع initFromFloatAPInt(api);
  إذا (Sem == &APFloat_بنياد::semIEEEdouble)
    أرجع initFromDoubleAPInt(api);
  إذا (Sem == &APFloat_بنياد::semIEEEquad)
    أرجع initFromQuadrupleAPInt(api);
  إذا (Sem == &APFloat_بنياد::semX87DoubleExtended)
    أرجع initFromF80LongDoubleAPInt(api);
  إذا (Sem == &APFloat_بنياد::semPPCDoubleDoubleLegacy)
    أرجع initFromPPCDoubleDoubleLegacyAPInt(api);
  إذا (Sem == &APFloat_بنياد::semFloat8E5M2)
    أرجع initFromFloat8E5M2APInt(api);
  إذا (Sem == &APFloat_بنياد::semFloat8E5M2FNUZ)
    أرجع initFromFloat8E5M2FNUZAPInt(api);
  إذا (Sem == &APFloat_بنياد::semFloat8E4M3)
    أرجع initFromFloat8E4M3APInt(api);
  إذا (Sem == &APFloat_بنياد::semFloat8E4M3FN)
    أرجع initFromFloat8E4M3FNAPInt(api);
  إذا (Sem == &APFloat_بنياد::semFloat8E4M3FNUZ)
    أرجع initFromFloat8E4M3FNUZAPInt(api);
  إذا (Sem == &APFloat_بنياد::semFloat8E4M3B11FNUZ)
    أرجع initFromFloat8E4M3B11FNUZAPInt(api);
  إذا (Sem == &APFloat_بنياد::semFloat8E3M4)
    أرجع initFromFloat8E3M4APInt(api);
  إذا (Sem == &APFloat_بنياد::semFloatTF32)
    أرجع initFromFloatTF32APInt(api);
  إذا (Sem == &APFloat_بنياد::semFloat8E8M0FNU)
    أرجع initFromFloat8E8M0FNUAPInt(api);
  إذا (Sem == &APFloat_بنياد::semFloat8E5M3FNU)
    أرجع initFromFloat8E5M3FNUAPInt(api);
  إذا (Sem == &APFloat_بنياد::semFloat6E3M2FN)
    أرجع initFromFloat6E3M2FNAPInt(api);
  إذا (Sem == &APFloat_بنياد::semFloat6E2M3FN)
    أرجع initFromFloat6E2M3FNAPInt(api);
  إذا (Sem == &APFloat_بنياد::semFloat4E2M1FN)
    أرجع initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
فراغ IEEEFloat::make_سڀ_کان_وڏو(منطقي Negative) {
  إذا (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 1..10
  //   significand = 1..1
  category = fc_عام;
  sign = Negative;
  exponent = semantics->maxExponent;

  // Use memset to set all but the highest integerPart to all ones.
  integerPart *significand = significandParts();
  بنانشان Part_ڳڻپ = part_ڳڻپ();
  memset(significand, 0xFF, حجم(integerPart)*(Part_ڳڻپ - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  ثابت بنانشان انگ_اڻ_استعمال_ٿيل_مٿاهون_بٽ =
    Part_ڳڻپ*integer_Part_ويڪر - semantics->precision;
  significand[Part_ڳڻپ - 1] = (انگ_اڻ_استعمال_ٿيل_مٿاهون_بٽ < integer_Part_ويڪر)
                                   ? (~integerPart(0) >> انگ_اڻ_استعمال_ٿيل_مٿاهون_بٽ)
                                   : 0;
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
فراغ IEEEFloat::make_سڀ_کان_ننڍو(منطقي Negative) {
  إذا (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fc_عام;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tc_سيٽ(significandParts(), 1, part_ڳڻپ());
}

فراغ IEEEFloat::make_سڀ_کان_ننڍو_Normalized(منطقي Negative) {
  إذا (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 10..0

  category = fc_عام;
  zeroSignificand();
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tc_سيٽ_ٻٽو_انگ(significandParts(), semantics->precision - 1);
}

IEEEFloat::IEEEFloat(ثابت fltSemantics &Sem, ثابت APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(عائم f) {
  initFromAPInt(&APFloat_بنياد::semIEEEsingle, APInt::float_منزل_بٽ(f));
}

IEEEFloat::IEEEFloat(مزدوج d) {
  initFromAPInt(&APFloat_بنياد::semIEEEdouble, APInt::double_منزل_بٽ(d));
}

نطاق {
  فراغ append(Small_ويڪٽر_Impl<حرف> &بفر, StringRef Str) {
    بفر.append(Str.begin(), Str.پڇاڙي());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  فراغ Adjust_منزل_Precision(APInt &significand,
                         صحيح &قوت_نما, بنانشان FormatPrecision) {
    بنانشان بٽ = significand.get_سرگرم_بٽ();

    // 196/59 is a very slight overestimate of lg_2(10).
    بنانشان بٽ_Required = (FormatPrecision * 196 + 58) / 59;

    إذا (بٽ <= بٽ_Required) أرجع;

    بنانشان tensRemovable = (بٽ - بٽ_Required) * 59 / 196;
    إذا (!tensRemovable) أرجع;

    قوت_نما += tensRemovable;

    APInt ورهائيندڙ(significand.get_ٻٽو_انگ_ويڪر(), 1);
    APInt powten(significand.get_ٻٽو_انگ_ويڪر(), 10);
    طالما (سچ) {
      إذا (tensRemovable & 1)
        ورهائيندڙ *= powten;
      tensRemovable >>= 1;
      إذا (!tensRemovable) اكسر;
      powten *= powten;
    }

    significand = significand.udiv(ورهائيندڙ);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.get_سرگرم_بٽ());
  }


  فراغ Adjust_منزل_Precision(Small_ويڪٽر_Impl<حرف> &بفر_2,
                         صحيح &قوت_نما, بنانشان FormatPrecision) {
    بنانشان N = بفر_2.سائيز_2();
    إذا (N <= FormatPrecision) أرجع;

    // The most significant figures are the last ones in the buffer.
    بنانشان پهريون_Significant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    إذا (بفر_2[پهريون_Significant - 1] < '5') {
      طالما (پهريون_Significant < N && بفر_2[پهريون_Significant] == '0')
        پهريون_Significant++;

      قوت_نما += پهريون_Significant;
      بفر_2.erase(&بفر_2[0], &بفر_2[پهريون_Significant]);
      أرجع;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    لكل (بنانشان I = پهريون_Significant; I != N; ++I) {
      إذا (بفر_2[I] == '9') {
        پهريون_Significant++;
      } وإلا {
        بفر_2[I]++;
        اكسر;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    إذا (پهريون_Significant == N) {
      قوت_نما += پهريون_Significant;
      بفر_2.clear();
      بفر_2.ڌڪو_پٺ('1');
      أرجع;
    }

    قوت_نما += پهريون_Significant;
    بفر_2.erase(&بفر_2[0], &بفر_2[پهريون_Significant]);
  }

  فراغ منزل_String_Impl(Small_ويڪٽر_Impl<حرف> &Str, ثابت منطقي isNeg, صحيح قوت_نما,
                    APInt significand, بنانشان FormatPrecision,
                    بنانشان FormatMaxPadding, منطقي TruncateZero) {
    ثابت صحيح semanticsPrecision = significand.get_ٻٽو_انگ_ويڪر();

    إذا (isNeg)
      Str.ڌڪو_پٺ('-');

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
    قوت_نما += trailingZeros;
    significand.lshr_اندريون_رکو(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    إذا (قوت_نما == 0) {
      // Nothing to do.
    } وإلا إذا (قوت_نما > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + قوت_نما);
      significand <<= قوت_نما;
      قوت_نما = 0;
    } وإلا { /* exp < 0 */
      صحيح texp = -قوت_نما;

      // We transform this using the identity:
      //   (N)(2^-e) == (N)(5^e)(10^-e)
      // This means we have to multiply N (the significand) by 5^e.
      // To avoid overflow, we have to operate on numbers large
      // enough to store N * 5^e:
      //   log2(N * 5^e) == log2(N) + e * log2(5)
      //                 <= semantics->precision + e * 137 / 59
      //   (log_2(5) ~ 2.321928 < 2.322034 ~ 137/59)

      بنانشان precision = semanticsPrecision + (137 * texp + 136) / 59;

      // Multiply significand by 5^e.
      //   N * 5^0101 == N * 5^(1*1) * 5^(0*2) * 5^(1*4) * 5^(0*8)
      significand = significand.zext(precision);
      APInt five_منزل_the_i(precision, 5);
      طالما (سچ) {
        إذا (texp & 1)
          significand *= five_منزل_the_i;

        texp >>= 1;
        إذا (!texp)
          اكسر;
        five_منزل_the_i *= five_منزل_the_i;
      }
    }

    Adjust_منزل_Precision(significand, قوت_نما, FormatPrecision);

    Small_ويڪٽر<حرف, 256> بفر_2;

    // Fill the buffer.
    بنانشان precision = significand.get_ٻٽو_انگ_ويڪر();
    إذا (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    منطقي اندريون_Trail = سچ;
    طالما (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      بنانشان d = digit.get_ZExt_قدر();

      // Drop trailing zeros.
      إذا (اندريون_Trail && !d)
        قوت_نما++;
      وإلا {
        بفر_2.ڌڪو_پٺ((حرف) ('0' + d));
        اندريون_Trail = خطأ;
      }
    }

    أكد(!بفر_2.خالي_جايون() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    Adjust_منزل_Precision(بفر_2, قوت_نما, FormatPrecision);

    بنانشان NDigits = بفر_2.سائيز_2();

    // Check whether we should use scientific notation.
    منطقي FormatScientific;
    إذا (!FormatMaxPadding) {
      FormatScientific = سچ;
    } وإلا {
      إذا (قوت_نما >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((بنانشان) قوت_نما > FormatMaxPadding ||
                            NDigits + (بنانشان) قوت_نما > FormatPrecision);
      } وإلا {
        // Power of the most significant digit.
        صحيح MSD = قوت_نما + (صحيح) (NDigits - 1);
        إذا (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = خطأ;
        } وإلا {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((بنانشان) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    إذا (FormatScientific) {
      قوت_نما += (NDigits - 1);

      Str.ڌڪو_پٺ(بفر_2[NDigits-1]);
      Str.ڌڪو_پٺ('.');
      إذا (NDigits == 1 && TruncateZero)
        Str.ڌڪو_پٺ('0');
      وإلا
        لكل (بنانشان I = 1; I != NDigits; ++I)
          Str.ڌڪو_پٺ(بفر_2[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      إذا (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.ڌڪو_پٺ(TruncateZero ? 'E' : 'e');

      Str.ڌڪو_پٺ(قوت_نما >= 0 ? '+' : '-');
      إذا (قوت_نما < 0)
        قوت_نما = -قوت_نما;
      Small_ويڪٽر<حرف, 6> expbuf;
      افعل {
        expbuf.ڌڪو_پٺ((حرف) ('0' + (قوت_نما % 10)));
        قوت_نما /= 10;
      } طالما (قوت_نما);
      // Exponent always at least two digits if we do not truncate zeros.
      إذا (!TruncateZero && expbuf.سائيز_2() < 2)
        expbuf.ڌڪو_پٺ('0');
      لكل (بنانشان I = 0, E = expbuf.سائيز_2(); I != E; ++I)
        Str.ڌڪو_پٺ(expbuf[E-1-I]);
      أرجع;
    }

    // Non-scientific, positive exponents.
    إذا (قوت_نما >= 0) {
      لكل (بنانشان I = 0; I != NDigits; ++I)
        Str.ڌڪو_پٺ(بفر_2[NDigits-1-I]);
      لكل (بنانشان I = 0; I != (بنانشان) قوت_نما; ++I)
        Str.ڌڪو_پٺ('0');
      أرجع;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    صحيح NWholeDigits = قوت_نما + (صحيح) NDigits;

    بنانشان I = 0;
    إذا (NWholeDigits > 0) {
      لكل (; I != (بنانشان) NWholeDigits; ++I)
        Str.ڌڪو_پٺ(بفر_2[NDigits-I-1]);
      Str.ڌڪو_پٺ('.');
    } وإلا {
      بنانشان NZeros = 1 + (بنانشان) -NWholeDigits;

      Str.ڌڪو_پٺ('0');
      Str.ڌڪو_پٺ('.');
      لكل (بنانشان Z = 1; Z != NZeros; ++Z)
        Str.ڌڪو_پٺ('0');
    }

    لكل (; I != NDigits; ++I)
      Str.ڌڪو_پٺ(بفر_2[NDigits-I-1]);

  }
} // namespace

فراغ IEEEFloat::منزل_String(Small_ويڪٽر_Impl<حرف> &Str, بنانشان FormatPrecision,
                         بنانشان FormatMaxPadding, منطقي TruncateZero) ثابت {
  بدل (category) {
  حالة fcInfinity:
    إذا (isNegative())
      أرجع append(Str, "-Inf");
    وإلا
      أرجع append(Str, "+Inf");

  حالة fcNaN: أرجع append(Str, "NaN");

  حالة fcZero:
    إذا (isNegative())
      Str.ڌڪو_پٺ('-');

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
      Str.ڌڪو_پٺ('0');
    }
    أرجع;

  حالة fc_عام:
    اكسر;
  }

  // Decompose the number into an APInt and an exponent.
  صحيح قوت_نما = exponent - ((صحيح) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_ڳڻپ_For_بٽ(semantics->precision)));

  منزل_String_Impl(Str, isNegative(), قوت_نما, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

صحيح IEEEFloat::getExactLog2Abs() ثابت {
  إذا (!isFinite() || isZero())
    أرجع INT_MIN;

  ثابت integerPart *Parts = significandParts();
  ثابت صحيح Part_ڳڻپ = part_ڳڻپ_For_بٽ(semantics->precision);

  صحيح ڪڍو_ڳڻپ = 0;
  لكل (صحيح i = 0; i < Part_ڳڻپ; ++i) {
    ڪڍو_ڳڻپ += llvm::popcount(Parts[i]);
    إذا (ڪڍو_ڳڻپ > 1)
      أرجع INT_MIN;
  }

  إذا (exponent != semantics->minExponent)
    أرجع exponent;

  صحيح CountrParts = 0;
  لكل (صحيح i = 0; i < Part_ڳڻپ;
       ++i, CountrParts += APInt::APINT_بٽ_في_لفظ) {
    إذا (Parts[i] != 0) {
      أرجع exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

منطقي IEEEFloat::isSignaling() ثابت {
  إذا (!isNaN())
    أرجع خطأ;
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    أرجع خطأ;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  أرجع !APInt::tc_ڪڍو_ٻٽو_انگ(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::op_حالت IEEEFloat::ايندڙ(منطقي ايندڙ_هيٺ_ڏانهن) {
  // If we are performing nextDown, swap sign so we have -x.
  إذا (ايندڙ_هيٺ_ڏانهن)
    changeSign();

  // Compute nextUp(x)
  op_حالت نتيجو_2 = opOK;

  // Handle each float category separately.
  بدل (category) {
  حالة fcInfinity:
    // nextUp(+inf) = +inf
    إذا (!isNegative())
      اكسر;
    // nextUp(-inf) = -getLargest()
    make_سڀ_کان_وڏو(سچ);
    اكسر;
  حالة fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    إذا (isSignaling()) {
      نتيجو_2 = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(خطأ, isNegative(), خالي_اشارو);
    }
    اكسر;
  حالة fcZero:
    // nextUp(pm 0) = +getSmallest()
    make_سڀ_کان_ننڍو(خطأ);
    اكسر;
  حالة fc_عام:
    // nextUp(-getSmallest()) = -0
    إذا (is_سڀ_کان_ننڍو() && isNegative()) {
      APInt::tc_سيٽ(significandParts(), 0, part_ڳڻپ());
      category = fcZero;
      exponent = 0;
      إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = خطأ;
      إذا (!semantics->hasZero)
        make_سڀ_کان_ننڍو_Normalized(خطأ);
      اكسر;
    }

    إذا (is_سڀ_کان_وڏو() && !isNegative()) {
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
        APInt::tc_سيٽ(significandParts(), 0, part_ڳڻپ());
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
      APInt::tcDecrement(Parts, part_ڳڻپ());

      إذا (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tc_سيٽ_ٻٽو_انگ(Parts, semantics->precision - 1);
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
        APInt::tc_سيٽ(Parts, 0, part_ڳڻپ());
        APInt::tc_سيٽ_ٻٽو_انگ(Parts, semantics->precision - 1);
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
  إذا (ايندڙ_هيٺ_ڏانهن)
    changeSign();

  أرجع نتيجو_2;
}

APInt IEEEFloat::getNaNPayload() ثابت {
  أكد(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  بنانشان بٽ_2 = semantics->precision - 1;
  أرجع APInt(بٽ_2, ArrayRef(significandParts(), part_ڳڻپ_For_بٽ(بٽ_2)));
}

APFloat_بنياد::ExponentType IEEEFloat::exponentNaN() ثابت {
  أرجع ::exponentNaN(*semantics);
}

APFloat_بنياد::ExponentType IEEEFloat::exponentInf() ثابت {
  أرجع ::exponentInf(*semantics);
}

APFloat_بنياد::ExponentType IEEEFloat::exponentZero() ثابت {
  أرجع ::exponentZero(*semantics);
}

فراغ IEEEFloat::makeInf(منطقي Negative) {
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(خطأ, Negative);
    أرجع;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tc_سيٽ(significandParts(), 0, part_ڳڻپ());
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
  APInt::tc_سيٽ(significandParts(), 0, part_ڳڻپ());
}

فراغ IEEEFloat::makeQuiet() {
  أكد(isNaN());
  إذا (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tc_سيٽ_ٻٽو_انگ(significandParts(), semantics->precision - 2);
}

صحيح ilogb(ثابت IEEEFloat &Arg) {
  إذا (Arg.isNaN())
    أرجع APFloat::IEK_NaN;
  إذا (Arg.isZero())
    أرجع APFloat::IEK_Zero;
  إذا (Arg.isInfinity())
    أرجع APFloat::IEK_Inf;
  إذا (!Arg.isDenormal())
    أرجع Arg.exponent;

  IEEEFloat Normalized(Arg);
  صحيح Significand_بٽ = Arg.getSemantics().precision - 1;

  Normalized.exponent += Significand_بٽ;
  Normalized.normalize(APFloat::rm_سڀ_کان_ويجهو_Ties_منزل_Even, lfExactlyZero);
  أرجع Normalized.exponent - Significand_بٽ;
}

IEEEFloat scalbn(IEEEFloat X, صحيح Exp, rounding_گهڻائي Rounding_گهڻائي) {
  خودڪار MaxExp = X.getSemantics().maxExponent;
  خودڪار MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  صحيح Significand_بٽ = X.getSemantics().precision - 1;
  صحيح MaxIncrement = MaxExp - (MinExp - Significand_بٽ) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(Rounding_گهڻائي, lfExactlyZero);
  إذا (X.isNaN())
    X.makeQuiet();
  أرجع X;
}

IEEEFloat frexp(ثابت IEEEFloat &Val, صحيح &Exp, rounding_گهڻائي RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  إذا (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    أرجع Quiet;
  }

  إذا (Exp == APFloat::IEK_Inf)
    أرجع Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  أرجع scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S)
    : Semantics(&S),
      Floats(جديد APFloat[2]{APFloat(APFloat_بنياد::semIEEEdouble),
                            APFloat(APFloat_بنياد::semIEEEdouble)}) {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, uninitialized_ٽيگ)
    : Semantics(&S), Floats(جديد APFloat[2]{
                         APFloat(APFloat_بنياد::semIEEEdouble, uninitialized),
                         APFloat(APFloat_بنياد::semIEEEdouble, uninitialized)}) {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(جديد APFloat[2]{APFloat(APFloat_بنياد::semIEEEdouble, I),
                            APFloat(APFloat_بنياد::semIEEEdouble)}) {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, ثابت APInt &I)
    : Semantics(&S),
      Floats(جديد APFloat[2]{
          APFloat(APFloat_بنياد::semIEEEdouble, APInt(64, I.get_Raw_معلومات()[0])),
          APFloat(APFloat_بنياد::semIEEEdouble, APInt(64, I.get_Raw_معلومات()[1]))}) {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, APFloat &&پهريون,
                             APFloat &&ٻيو)
    : Semantics(&S),
      Floats(جديد APFloat[2]{std::move(پهريون), std::move(ٻيو)}) {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble);
  أكد(&Floats[0].getSemantics() == &APFloat_بنياد::semIEEEdouble);
  أكد(&Floats[1].getSemantics() == &APFloat_بنياد::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? جديد APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : خالي_اشارو) {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloat_بنياد::semBogus;
  RHS.Floats = خالي_اشارو;
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::عامل=(ثابت DoubleAPFloat &RHS) {
  إذا (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } وإلا إذا (هذا != &RHS) {
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
ساكن std::pair<APFloat, APFloat> fast_Two_جوڙ(APFloat X, APFloat Y) {
  إذا (!X.isFinite())
    أرجع {X, APFloat::getZero(X.getSemantics(), /*Negative=*/خطأ)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  أرجع {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::op_حالت DoubleAPFloat::جوڙيو_Impl(ثابت APFloat &a, ثابت APFloat &aa,
                                         ثابت APFloat &c, ثابت APFloat &cc,
                                         rounding_گهڻائي RM) {
  صحيح حالت = opOK;
  APFloat z = a;
  حالت |= z.جوڙيو(c, RM);
  إذا (!z.isFinite()) {
    إذا (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ خطأ);
      أرجع (op_حالت)حالت;
    }
    حالت = opOK;
    خودڪار ACompared_منزل_C = a.compare_Absolute_قدر(c);
    z = cc;
    حالت |= z.جوڙيو(aa, RM);
    إذا (ACompared_منزل_C == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      حالت |= z.جوڙيو(c, RM);
      حالت |= z.جوڙيو(a, RM);
    } وإلا {
      // z = cc + aa + a + c;
      حالت |= z.جوڙيو(a, RM);
      حالت |= z.جوڙيو(c, RM);
    }
    إذا (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ خطأ);
      أرجع (op_حالت)حالت;
    }
    Floats[0] = z;
    APFloat zz = aa;
    حالت |= zz.جوڙيو(cc, RM);
    إذا (ACompared_منزل_C == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      حالت |= Floats[1].ڪٽيو(z, RM);
      حالت |= Floats[1].جوڙيو(c, RM);
      حالت |= Floats[1].جوڙيو(zz, RM);
    } وإلا {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      حالت |= Floats[1].ڪٽيو(z, RM);
      حالت |= Floats[1].جوڙيو(a, RM);
      حالت |= Floats[1].جوڙيو(zz, RM);
    }
  } وإلا {
    // q = a - z;
    APFloat q = a;
    حالت |= q.ڪٽيو(z, RM);

    // zz = q + c + (a - (q + z)) + aa + cc;
    // Compute a - (q + z) as -((q + z) - a) to avoid temporary copies.
    خودڪار zz = q;
    حالت |= zz.جوڙيو(c, RM);
    حالت |= q.جوڙيو(z, RM);
    حالت |= q.ڪٽيو(a, RM);
    q.changeSign();
    حالت |= zz.جوڙيو(q, RM);
    حالت |= zz.جوڙيو(aa, RM);
    حالت |= zz.جوڙيو(cc, RM);
    إذا (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ خطأ);
      أرجع opOK;
    }
    Floats[0] = z;
    حالت |= Floats[0].جوڙيو(zz, RM);
    إذا (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ خطأ);
      أرجع (op_حالت)حالت;
    }
    Floats[1] = std::move(z);
    حالت |= Floats[1].ڪٽيو(Floats[0], RM);
    حالت |= Floats[1].جوڙيو(zz, RM);
  }
  أرجع (op_حالت)حالت;
}

APFloat::op_حالت DoubleAPFloat::جوڙيو_With_Special(ثابت DoubleAPFloat &LHS,
                                                ثابت DoubleAPFloat &RHS,
                                                DoubleAPFloat &ٻاهريون,
                                                rounding_گهڻائي RM) {
  إذا (LHS.getCategory() == fcNaN) {
    ٻاهريون = LHS;
    أرجع opOK;
  }
  إذا (RHS.getCategory() == fcNaN) {
    ٻاهريون = RHS;
    أرجع opOK;
  }
  إذا (LHS.getCategory() == fcZero) {
    ٻاهريون = RHS;
    أرجع opOK;
  }
  إذا (RHS.getCategory() == fcZero) {
    ٻاهريون = LHS;
    أرجع opOK;
  }
  إذا (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    ٻاهريون.makeNaN(خطأ, ٻاهريون.isNegative(), خالي_اشارو);
    أرجع opInvalidOp;
  }
  إذا (LHS.getCategory() == fcInfinity) {
    ٻاهريون = LHS;
    أرجع opOK;
  }
  إذا (RHS.getCategory() == fcInfinity) {
    ٻاهريون = RHS;
    أرجع opOK;
  }
  أكد(LHS.getCategory() == fc_عام && RHS.getCategory() == fc_عام);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  أكد(&A.getSemantics() == &APFloat_بنياد::semIEEEdouble);
  أكد(&AA.getSemantics() == &APFloat_بنياد::semIEEEdouble);
  أكد(&C.getSemantics() == &APFloat_بنياد::semIEEEdouble);
  أكد(&CC.getSemantics() == &APFloat_بنياد::semIEEEdouble);
  أكد(&ٻاهريون.Floats[0].getSemantics() == &APFloat_بنياد::semIEEEdouble);
  أكد(&ٻاهريون.Floats[1].getSemantics() == &APFloat_بنياد::semIEEEdouble);
  أرجع ٻاهريون.جوڙيو_Impl(A, AA, C, CC, RM);
}

APFloat::op_حالت DoubleAPFloat::جوڙيو(ثابت DoubleAPFloat &RHS,
                                     rounding_گهڻائي RM) {
  أرجع جوڙيو_With_Special(*هذا, RHS, *هذا, RM);
}

APFloat::op_حالت DoubleAPFloat::ڪٽيو(ثابت DoubleAPFloat &RHS,
                                          rounding_گهڻائي RM) {
  changeSign();
  خودڪار Ret = جوڙيو(RHS, RM);
  changeSign();
  أرجع Ret;
}

APFloat::op_حالت DoubleAPFloat::multiply(ثابت DoubleAPFloat &RHS,
                                          APFloat::rounding_گهڻائي RM) {
  ثابت خودڪار &LHS = *هذا;
  خودڪار &ٻاهريون = *هذا;
  /* Interesting observation: For special categories, finding the lowest
     common ancestor of the following layered graph gives the correct
     return category:

        NaN
       /   \
     Zero  Inf
       \   /
عام

     e.g. NaN * NaN = NaN
          Zero * Inf = NaN
          Normal * Zero = Zero
          Normal * Inf = Inf
  */
  إذا (LHS.getCategory() == fcNaN) {
    ٻاهريون = LHS;
    أرجع opOK;
  }
  إذا (RHS.getCategory() == fcNaN) {
    ٻاهريون = RHS;
    أرجع opOK;
  }
  إذا ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    ٻاهريون.makeNaN(خطأ, خطأ, خالي_اشارو);
    أرجع opOK;
  }
  إذا (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    ٻاهريون = LHS;
    أرجع opOK;
  }
  إذا (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    ٻاهريون = RHS;
    أرجع opOK;
  }
  أكد(LHS.getCategory() == fc_عام && RHS.getCategory() == fc_عام &&
         "Special cases not handled exhaustively");

  صحيح حالت = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  حالت |= T.multiply(C, RM);
  إذا (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ خطأ);
    أرجع (op_حالت)حالت;
  }

  // tau = fmsub(a, c, t), that is -fmadd(-a, c, t).
  APFloat Tau = A;
  T.changeSign();
  حالت |= Tau.fused_Multiply_جوڙيو(C, T, RM);
  T.changeSign();
  {
    // v = a * d
    APFloat V = A;
    حالت |= V.multiply(D, RM);
    // w = b * c
    APFloat W = B;
    حالت |= W.multiply(C, RM);
    حالت |= V.جوڙيو(W, RM);
    // tau += v + w
    حالت |= Tau.جوڙيو(V, RM);
  }
  // u = t + tau
  APFloat U = T;
  حالت |= U.جوڙيو(Tau, RM);

  Floats[0] = U;
  إذا (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ خطأ);
  } وإلا {
    // Floats[1] = (t - u) + tau
    حالت |= T.ڪٽيو(U, RM);
    حالت |= T.جوڙيو(Tau, RM);
    Floats[1] = std::move(T);
  }
  أرجع (op_حالت)حالت;
}

APFloat::op_حالت DoubleAPFloat::divide(ثابت DoubleAPFloat &RHS,
                                        APFloat::rounding_گهڻائي RM) {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_بنياد::semPPCDoubleDoubleLegacy, bitcast_منزل_APInt());
  خودڪار Ret = Tmp.divide(
      APFloat(APFloat_بنياد::semPPCDoubleDoubleLegacy, RHS.bitcast_منزل_APInt()), RM);
  *هذا = DoubleAPFloat(APFloat_بنياد::semPPCDoubleDouble, Tmp.bitcast_منزل_APInt());
  أرجع Ret;
}

APFloat::op_حالت DoubleAPFloat::remainder(ثابت DoubleAPFloat &RHS) {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_بنياد::semPPCDoubleDoubleLegacy, bitcast_منزل_APInt());
  خودڪار Ret = Tmp.remainder(
      APFloat(APFloat_بنياد::semPPCDoubleDoubleLegacy, RHS.bitcast_منزل_APInt()));
  *هذا = DoubleAPFloat(APFloat_بنياد::semPPCDoubleDouble, Tmp.bitcast_منزل_APInt());
  أرجع Ret;
}

APFloat::op_حالت DoubleAPFloat::mod(ثابت DoubleAPFloat &RHS) {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_بنياد::semPPCDoubleDoubleLegacy, bitcast_منزل_APInt());
  خودڪار Ret = Tmp.mod(
      APFloat(APFloat_بنياد::semPPCDoubleDoubleLegacy, RHS.bitcast_منزل_APInt()));
  *هذا = DoubleAPFloat(APFloat_بنياد::semPPCDoubleDouble, Tmp.bitcast_منزل_APInt());
  أرجع Ret;
}

APFloat::op_حالت
DoubleAPFloat::fused_Multiply_جوڙيو(ثابت DoubleAPFloat &Multiplicand,
                                ثابت DoubleAPFloat &Addend,
                                APFloat::rounding_گهڻائي RM) {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_بنياد::semPPCDoubleDoubleLegacy, bitcast_منزل_APInt());
  خودڪار Ret = Tmp.fused_Multiply_جوڙيو(
      APFloat(APFloat_بنياد::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcast_منزل_APInt()),
      APFloat(APFloat_بنياد::semPPCDoubleDoubleLegacy, Addend.bitcast_منزل_APInt()),
      RM);
  *هذا = DoubleAPFloat(APFloat_بنياد::semPPCDoubleDouble, Tmp.bitcast_منزل_APInt());
  أرجع Ret;
}

APFloat::op_حالت DoubleAPFloat::ڦيرو_منزل_Integral(APFloat::rounding_گهڻائي RM) {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ثابت APFloat &Hi = get_پهريون();
  ثابت APFloat &Lo = get_ٻيو();

  APFloat RoundedHi = Hi;
  ثابت op_حالت Hi_حالت = RoundedHi.ڦيرو_منزل_Integral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  إذا (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/خطأ);
    أرجع Hi_حالت;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  خودڪار ڦيرو_منزل_سڀ_کان_ويجهو_Helper = [](APFloat منزل_ڦيرو, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    ثابت APFloat RoundingError = Rounded - منزل_ڦيرو;
    إذا (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        مطلق_قدر(RoundingError).is_Exactly_قدر(0.5))
      Rounded.جوڙيو(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rm_سڀ_کان_ويجهو_Ties_منزل_Even);
    أرجع Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  إذا (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    إذا (RM == rm_سڀ_کان_ويجهو_Ties_منزل_Away || RM == rm_سڀ_کان_ويجهو_Ties_منزل_Even)
      RoundedHi = ڦيرو_منزل_سڀ_کان_ويجهو_Helper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/خطأ);
    أرجع Hi_حالت;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  Rounding_گهڻائي Lo_Rounding_گهڻائي;
  إذا (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    Lo_Rounding_گهڻائي = isNegative() ? rm_Toward_مثبت : rmTowardNegative;
  وإلا
    Lo_Rounding_گهڻائي = RM;

  APFloat RoundedLo = Lo;
  ثابت op_حالت Lo_حالت = RoundedLo.ڦيرو_منزل_Integral(Lo_Rounding_گهڻائي);
  إذا (Lo_Rounding_گهڻائي == rm_سڀ_کان_ويجهو_Ties_منزل_Away)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = ڦيرو_منزل_سڀ_کان_ويجهو_Helper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fast_Two_جوڙ(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  أرجع Lo_حالت;
}

فراغ DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmp_نتيجو
DoubleAPFloat::compare_Absolute_قدر(ثابت DoubleAPFloat &RHS) ثابت {
  // Compare absolute values of the high parts.
  ثابت cmp_نتيجو HiPartCmp = Floats[0].compare_Absolute_قدر(RHS.Floats[0]);
  إذا (HiPartCmp != cmpEqual)
    أرجع HiPartCmp;

  // Zero, regardless of sign, is equal.
  إذا (Floats[1].isZero() && RHS.Floats[1].isZero())
    أرجع cmpEqual;

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
    أرجع RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  إذا (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    أرجع ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  إذا (ThisIsSubtractive != RHSIsSubtractive)
    أرجع ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  ثابت cmp_نتيجو LoPartCmp = Floats[1].compare_Absolute_قدر(RHS.Floats[1]);

  إذا (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    إذا (LoPartCmp == cmpLessThan)
      أرجع cmpGreaterThan;
    إذا (LoPartCmp == cmpGreaterThan)
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

فراغ DoubleAPFloat::make_سڀ_کان_وڏو(منطقي Neg) {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloat_بنياد::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloat_بنياد::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  إذا (Neg)
    changeSign();
}

فراغ DoubleAPFloat::make_سڀ_کان_ننڍو(منطقي Neg) {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].make_سڀ_کان_ننڍو(Neg);
  Floats[1].makeZero(/* Neg = */ خطأ);
}

فراغ DoubleAPFloat::make_سڀ_کان_ننڍو_Normalized(منطقي Neg) {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloat_بنياد::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  إذا (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ خطأ);
}

فراغ DoubleAPFloat::makeNaN(منطقي SNaN, منطقي Neg, ثابت APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ خطأ);
}

APFloat::cmp_نتيجو DoubleAPFloat::compare(ثابت DoubleAPFloat &RHS) ثابت {
  خودڪار نتيجو = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  إذا (نتيجو == APFloat::cmpEqual)
    أرجع Floats[1].compare(RHS.Floats[1]);
  أرجع نتيجو;
}

منطقي DoubleAPFloat::bitwiseIsEqual(ثابت DoubleAPFloat &RHS) ثابت {
  أرجع Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_ڪوڊ hash_قدر(ثابت DoubleAPFloat &Arg) {
  إذا (Arg.Floats)
    أرجع hash_combine(hash_قدر(Arg.Floats[0]), hash_قدر(Arg.Floats[1]));
  أرجع hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcast_منزل_APInt() ثابت {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t معلومات_2[] = {
      Floats[0].bitcast_منزل_APInt().get_Raw_معلومات()[0],
      Floats[1].bitcast_منزل_APInt().get_Raw_معلومات()[0],
  };
  أرجع APInt(128, معلومات_2);
}

Expected<APFloat::op_حالت> DoubleAPFloat::convertFromString(StringRef S,
                                                             rounding_گهڻائي RM) {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_بنياد::semPPCDoubleDoubleLegacy);
  خودڪار Ret = Tmp.convertFromString(S, RM);
  *هذا = DoubleAPFloat(APFloat_بنياد::semPPCDoubleDouble, Tmp.bitcast_منزل_APInt());
  أرجع Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::op_حالت DoubleAPFloat::ايندڙ(منطقي ايندڙ_هيٺ_ڏانهن) {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  إذا (ايندڙ_هيٺ_ڏانهن) {
    changeSign();
    APFloat::op_حالت نتيجو = ايندڙ(/*nextDown=*/خطأ);
    changeSign();
    أرجع نتيجو;
  }
  بدل (getCategory()) {
  حالة fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    إذا (isNegative())
      make_سڀ_کان_وڏو(سچ);
    أرجع opOK;

  حالة fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    إذا (get_پهريون().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(خطأ, isNegative(), خالي_اشارو);
      أرجع opInvalidOp;
    }
    أرجع opOK;

  حالة fcZero:
    // nextUp(pm 0) = +getSmallest()
    make_سڀ_کان_ننڍو(خطأ);
    أرجع opOK;

  حالة fc_عام:
    اكسر;
  }

  ثابت APFloat &HiOld = get_پهريون();
  ثابت APFloat &LoOld = get_ٻيو();

  APFloat ايندڙ_Lo = LoOld;
  ايندڙ_Lo.ايندڙ(/*nextDown=*/خطأ);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  خودڪار اندريون_Lattice = [](ثابت APFloat &Hi, ثابت APFloat &Lo) {
    أرجع Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  إذا (اندريون_Lattice(HiOld, ايندڙ_Lo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(ايندڙ_Lo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat سڀ_کان_وڏو{*Semantics, uninitialized};
    سڀ_کان_وڏو.make_سڀ_کان_وڏو(/*Neg=*/خطأ);
    إذا (compare(سڀ_کان_وڏو) == cmpGreaterThan)
      makeInf(/*Neg=*/خطأ);

    أرجع opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat ايندڙ_Hi = HiOld;
  ايندڙ_Hi.ايندڙ(/*nextDown=*/خطأ);

  // nextUp(getLargest()) == INFINITY
  إذا (ايندڙ_Hi.isInfinity()) {
    makeInf(/*Neg=*/خطأ);
    أرجع opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  إذا (ايندڙ_Hi.isZero()) {
    makeZero(/*Neg=*/سچ);
    أرجع opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  ايندڙ_Lo = neg(scalbn(harrisonUlp(ايندڙ_Hi), -1, rmTowardZero));
  إذا (!اندريون_Lattice(ايندڙ_Hi, ايندڙ_Lo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    ايندڙ_Lo.ايندڙ(/*nextDown=*/خطأ);

  Floats[0] = std::move(ايندڙ_Hi);
  Floats[1] = std::move(ايندڙ_Lo);

  أرجع opOK;
}

APFloat::op_حالت DoubleAPFloat::convert_منزل_Sign_Extended_Integer(
    MutableArrayRef<integerPart> داخلا, بنانشان صحيح ويڪر, منطقي IsSigned,
    rounding_گهڻائي RM, منطقي *IsExact) ثابت {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  إذا (!get_پهريون().isFiniteNonZero() || get_ٻيو().isZero())
    أرجع get_پهريون().convert_منزل_Integer(داخلا, ويڪر, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = خطأ;
  DoubleAPFloat Integral = *هذا;
  ثابت op_حالت ڦيرو_حالت = Integral.ڦيرو_منزل_Integral(RM);
  إذا (ڦيرو_حالت == opInvalidOp)
    أرجع opInvalidOp;
  ثابت APFloat &IntegralHi = Integral.get_پهريون();
  ثابت APFloat &IntegralLo = Integral.get_ٻيو();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  منطقي HiIsExact;
  إذا (IntegralHi.isZero() || IntegralLo.isZero()) {
    ثابت op_حالت Hi_حالت =
        IntegralHi.convert_منزل_Integer(داخلا, ويڪر, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    أرجع Hi_حالت == opOK ? opInexact : Hi_حالت;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  إذا (!IsSigned && IntegralHi.isNegative())
    أرجع opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  منطقي LoIsExact;
  ثابت صحيح HiExactLog2 = IntegralHi.getExactLog2Abs();
  ثابت بنانشان مثبت_حد_کان_مٿي_ويڪر = IsSigned ? ويڪر - 1 : ويڪر;
  إذا (HiExactLog2 >= 0 &&
      static_cast<بنانشان>(HiExactLog2) == مثبت_حد_کان_مٿي_ويڪر) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    إذا (IntegralHi.isNegative() == IntegralLo.isNegative())
      أرجع opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    ثابت op_حالت Lo_حالت = IntegralLo.convert_منزل_Integer(
        داخلا, ويڪر, /*IsSigned=*/سچ, RM, &LoIsExact);
    إذا (Lo_حالت == opInvalidOp)
      أرجع opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    إذا (IsSigned && !IntegralHi.isNegative())
      APInt::tc_Clear_ٻٽو_انگ(داخلا.معلومات(), مثبت_حد_کان_مٿي_ويڪر);
    *IsExact = ڦيرو_حالت == opOK;
    أرجع ڦيرو_حالت;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  ثابت op_حالت Hi_حالت = IntegralHi.convert_منزل_Integer(
      داخلا, ويڪر, IsSigned, rmTowardZero, &HiIsExact);
  إذا (Hi_حالت == opInvalidOp)
    أرجع Hi_حالت;

  // Convert Lo into a temporary integer of the same width.
  APSInt Lo_نتيجو{ويڪر, /*isUnsigned=*/!IsSigned};
  ثابت op_حالت Lo_حالت =
      IntegralLo.convert_منزل_Integer(Lo_نتيجو, rmTowardZero, &LoIsExact);
  إذا (Lo_حالت == opInvalidOp)
    أرجع Lo_حالت;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tc_جوڙيو(داخلا.معلومات(), Lo_نتيجو.get_Raw_معلومات(), /*carry=*/0, داخلا.سائيز_2());

  *IsExact = ڦيرو_حالت == opOK;
  أرجع ڦيرو_حالت;
}

APFloat::op_حالت
DoubleAPFloat::convert_منزل_Integer(MutableArrayRef<integerPart> داخلا,
                                بنانشان صحيح ويڪر, منطقي IsSigned,
                                rounding_گهڻائي RM, منطقي *IsExact) ثابت {
  op_حالت FS =
      convert_منزل_Sign_Extended_Integer(داخلا, ويڪر, IsSigned, RM, IsExact);

  إذا (FS == opInvalidOp) {
    ثابت بنانشان Dst_Parts_ڳڻپ = part_ڳڻپ_For_بٽ(ويڪر);
    أكد(Dst_Parts_ڳڻپ <= داخلا.سائيز_2() && "Integer too big");

    بنانشان بٽ_2;
    إذا (getCategory() == fcNaN)
      بٽ_2 = 0;
    وإلا إذا (isNegative())
      بٽ_2 = IsSigned;
    وإلا
      بٽ_2 = ويڪر - IsSigned;

    tc_سيٽ_گهٽ_کان_گهٽ_Significant_بٽ(داخلا.معلومات(), Dst_Parts_ڳڻپ, بٽ_2);
    إذا (isNegative() && IsSigned)
      APInt::tc_سرڪايو_کاٻو(داخلا.معلومات(), Dst_Parts_ڳڻپ, ويڪر - 1);
  }

  أرجع FS;
}

APFloat::op_حالت DoubleAPFloat::handle_حد_کان_مٿي(rounding_گهڻائي RM) {
  بدل (RM) {
  حالة APFloat::rmTowardZero:
    make_سڀ_کان_وڏو(/*Neg=*/isNegative());
    اكسر;
  حالة APFloat::rmTowardNegative:
    إذا (isNegative())
      makeInf(/*Neg=*/سچ);
    وإلا
      make_سڀ_کان_وڏو(/*Neg=*/خطأ);
    اكسر;
  حالة APFloat::rm_Toward_مثبت:
    إذا (isNegative())
      make_سڀ_کان_وڏو(/*Neg=*/سچ);
    وإلا
      makeInf(/*Neg=*/خطأ);
    اكسر;
  حالة APFloat::rm_سڀ_کان_ويجهو_Ties_منزل_Away:
  حالة APFloat::rm_سڀ_کان_ويجهو_Ties_منزل_Even:
    makeInf(/*Neg=*/isNegative());
    اكسر;
  افتراضي:
    llvm_unreachable("Invalid rounding mode found");
  }
  op_حالت S = opInexact;
  إذا (!get_پهريون().isFinite())
    S = static_cast<op_حالت>(S | op_حد_کان_مٿي);
  أرجع S;
}

APFloat::op_حالت DoubleAPFloat::convertFromUnsignedParts(
    ثابت integerPart *Src, بنانشان صحيح Src_ڳڻپ, rounding_گهڻائي RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  ثابت بنانشان SrcMSB = APInt::tcMSB(Src, Src_ڳڻپ);
  إذا (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/خطأ);
    أرجع opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  ثابت بنانشان Src_ٻٽو_انگ_ويڪر = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/Src_ٻٽو_انگ_ويڪر, ArrayRef(Src, Src_ڳڻپ)},
                /*isUnsigned=*/سچ};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{get_پهريون().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/خطأ, rm_سڀ_کان_ويجهو_Ties_منزل_Even);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  إذا (!Hi.isFinite())
    أرجع handle_حد_کان_مٿي(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  منطقي HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/سچ};
  Hi.convert_منزل_Integer(HiAsInt, rm_سڀ_کان_ويجهو_Ties_منزل_Even, &HiAsIntIsExact);
  ثابت APInt Error = SrcInt.zext(HiAsInt.get_ٻٽو_انگ_ويڪر()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  rounding_گهڻائي LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  إذا (Error.isNegative()) {
    إذا (RM == rm_سڀ_کان_ويجهو_Ties_منزل_Away) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      ثابت بنانشان Error_سرگرم_بٽ = Error.get_Significant_بٽ() - 1;
      ثابت بنانشان LoPrecision = get_ٻيو().getSemantics().precision;
      إذا (Error_سرگرم_بٽ > LoPrecision) {
        ثابت بنانشان RoundingBoundary = Error_سرگرم_بٽ - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        إذا (Error.ڳڻپ_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rm_Toward_مثبت;
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

  APFloat Lo{get_ٻيو().getSemantics()};
  op_حالت حالت = Lo.convertFromAPInt(Error, /*IsSigned=*/سچ, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fast_Two_جوڙ(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  إذا (!get_پهريون().isFinite())
    أرجع handle_حد_کان_مٿي(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  إذا (get_پهريون().isFiniteNonZero() && Floats[0].is_سڀ_کان_وڏو()) {
    DoubleAPFloat سڀ_کان_وڏو{*Semantics};
    سڀ_کان_وڏو.make_سڀ_کان_وڏو(/*Neg=*/خطأ);
    إذا (compare(سڀ_کان_وڏو) == APFloat::cmpGreaterThan)
      أرجع handle_حد_کان_مٿي(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  أرجع حالت;
}

APFloat::op_حالت DoubleAPFloat::convertFromAPInt(ثابت APInt &داخلا,
                                                  منطقي IsSigned,
                                                  rounding_گهڻائي RM) {
  ثابت منطقي Negate_داخلا = IsSigned && داخلا.isNegative();
  APInt API = داخلا;
  إذا (Negate_داخلا)
    API.negate();

  ثابت APFloat::op_حالت حالت =
      convertFromUnsignedParts(API.get_Raw_معلومات(), API.get_انگ_لفظ(), RM);
  إذا (Negate_داخلا)
    changeSign();
  أرجع حالت;
}

بنانشان صحيح DoubleAPFloat::convert_منزل_Hex_String(حرف *DST,
                                               بنانشان صحيح HexDigits,
                                               منطقي مٿانهون_Case_2,
                                               rounding_گهڻائي RM) ثابت {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  أرجع APFloat(APFloat_بنياد::semPPCDoubleDoubleLegacy, bitcast_منزل_APInt())
      .convert_منزل_Hex_String(DST, HexDigits, مٿانهون_Case_2, RM);
}

منطقي DoubleAPFloat::isDenormal() ثابت {
  أرجع getCategory() == fc_عام &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

منطقي DoubleAPFloat::is_سڀ_کان_ننڍو() ثابت {
  إذا (getCategory() != fc_عام)
    أرجع خطأ;
  DoubleAPFloat Tmp(*هذا);
  Tmp.make_سڀ_کان_ننڍو(هذا->isNegative());
  أرجع Tmp.compare(*هذا) == cmpEqual;
}

منطقي DoubleAPFloat::is_سڀ_کان_ننڍو_Normalized() ثابت {
  إذا (getCategory() != fc_عام)
    أرجع خطأ;

  DoubleAPFloat Tmp(*هذا);
  Tmp.make_سڀ_کان_ننڍو_Normalized(هذا->isNegative());
  أرجع Tmp.compare(*هذا) == cmpEqual;
}

منطقي DoubleAPFloat::is_سڀ_کان_وڏو() ثابت {
  إذا (getCategory() != fc_عام)
    أرجع خطأ;
  DoubleAPFloat Tmp(*هذا);
  Tmp.make_سڀ_کان_وڏو(هذا->isNegative());
  أرجع Tmp.compare(*هذا) == cmpEqual;
}

منطقي DoubleAPFloat::isInteger() ثابت {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  أرجع Floats[0].isInteger() && Floats[1].isInteger();
}

فراغ DoubleAPFloat::منزل_String(Small_ويڪٽر_Impl<حرف> &Str,
                             بنانشان FormatPrecision,
                             بنانشان FormatMaxPadding,
                             منطقي TruncateZero) ثابت {
  أكد(Semantics == &APFloat_بنياد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloat_بنياد::semPPCDoubleDoubleLegacy, bitcast_منزل_APInt())
      .منزل_String(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

صحيح DoubleAPFloat::getExactLog2Abs() ثابت {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  إذا (get_ٻيو().isNonZero())
    أرجع INT_MIN;
  أرجع get_پهريون().getExactLog2Abs();
}

صحيح ilogb(ثابت DoubleAPFloat &Arg) {
  ثابت APFloat &Hi = Arg.get_پهريون();
  ثابت APFloat &Lo = Arg.get_ٻيو();
  صحيح Ilogb_نتيجو = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  إذا (Arg.getCategory() != fc_عام)
    أرجع Ilogb_نتيجو;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  إذا (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    أرجع Ilogb_نتيجو;
  إذا (Hi.getExactLog2Abs() == INT_MIN)
    أرجع Ilogb_نتيجو;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  أرجع Ilogb_نتيجو - 1;
}

DoubleAPFloat scalbn(ثابت DoubleAPFloat &Arg, صحيح Exp,
                     APFloat::rounding_گهڻائي RM) {
  أكد(Arg.Semantics == &APFloat_بنياد::PPCDoubleDouble() &&
         "Unexpected Semantics");
  أرجع DoubleAPFloat(APFloat_بنياد::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(ثابت DoubleAPFloat &Arg, صحيح &Exp,
                    APFloat::rounding_گهڻائي RM) {
  أكد(Arg.Semantics == &APFloat_بنياد::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  إذا (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.get_پهريون() = Quiet.get_پهريون().makeQuiet();
    أرجع Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  إذا (Exp == APFloat::IEK_Inf)
    أرجع Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  إذا (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    أرجع Arg;
  }

  ثابت APFloat &Hi = Arg.get_پهريون();
  ثابت APFloat &Lo = Arg.get_ٻيو();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  ثابت منطقي SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat ٻيو = Lo;
  إذا (Arg.getCategory() == APFloat::fc_عام && Lo.isFiniteNonZero()) {
    rounding_گهڻائي Lo_Rounding_گهڻائي;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    إذا (RM == rmTowardZero)
      Lo_Rounding_گهڻائي = Arg.isNegative() ? rm_Toward_مثبت : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    وإلا إذا (RM == rm_سڀ_کان_ويجهو_Ties_منزل_Away && SignsDisagree && Exp > 0)
      Lo_Rounding_گهڻائي = rm_سڀ_کان_ويجهو_Ties_منزل_Even;
    وإلا
      Lo_Rounding_گهڻائي = RM;
    ٻيو = scalbn(Lo, -Exp, Lo_Rounding_گهڻائي);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    إذا (RM == rm_سڀ_کان_ويجهو_Ties_منزل_Away && Lo_Rounding_گهڻائي == rm_سڀ_کان_ويجهو_Ties_منزل_Even) {
      // Re-scale the result back to check if rounding occurred.
      ثابت APFloat RecomposedLo = scalbn(ٻيو, Exp, rm_سڀ_کان_ويجهو_Ties_منزل_Even);
      إذا (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        ثابت APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        ثابت APFloat Ulp_جو_ٻيو = harrisonUlp(ٻيو);
        ثابت APFloat Scaled_Ulp_جو_ٻيو =
            scalbn(Ulp_جو_ٻيو, Exp - 1, rm_سڀ_کان_ويجهو_Ties_منزل_Even);
        ثابت منطقي IsMidpoint = مطلق_قدر(RoundingError) == Scaled_Ulp_جو_ٻيو;
        ثابت منطقي RoundedLoAway =
            ٻيو.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        إذا (IsMidpoint && RoundedLoAway)
          ٻيو.ايندڙ(/*nextDown=*/!ٻيو.isNegative());
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
    إذا (ٻيو.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat پهريون = scalbn(Hi, -Exp, RM);
  أرجع DoubleAPFloat(APFloat_بنياد::PPCDoubleDouble(), std::move(پهريون),
                       std::move(ٻيو));
}

APInt DoubleAPFloat::getNaNPayload() ثابت { أرجع Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, ثابت fltSemantics &Semantics) {
  إذا (استعمال_ڪري_Layout<IEEEFloat>(Semantics)) {
    جديد (&IEEE) IEEEFloat(std::move(F));
    أرجع;
  }
  إذا (استعمال_ڪري_Layout<DoubleAPFloat>(Semantics)) {
    ثابت fltSemantics& S = F.getSemantics();
    جديد (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloat_بنياد::IEEEdouble()));
    أرجع;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::op_حالت> APFloat::convertFromString(StringRef Str,
                                                       rounding_گهڻائي RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_ڪوڊ hash_قدر(ثابت APFloat &Arg) {
  إذا (APFloat::استعمال_ڪري_Layout<detail::IEEEFloat>(Arg.getSemantics()))
    أرجع hash_قدر(Arg.U.IEEE);
  إذا (APFloat::استعمال_ڪري_Layout<detail::DoubleAPFloat>(Arg.getSemantics()))
    أرجع hash_قدر(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(ثابت fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  خودڪار حالت_Or_Err = convertFromString(S, rm_سڀ_کان_ويجهو_Ties_منزل_Even);
  أكد(حالت_Or_Err && "Invalid floating point representation");
  consumeError(حالت_Or_Err.takeError());
}

FPClassTest APFloat::classify() ثابت {
  إذا (isZero())
    أرجع isNegative() ? fcNegZero : fcPosZero;
  إذا (is_عام())
    أرجع isNegative() ? fc_Neg_عام : fc_Pos_عام;
  إذا (isDenormal())
    أرجع isNegative() ? fcNegSubnormal : fcPosSubnormal;
  إذا (isInfinity())
    أرجع isNegative() ? fcNegInf : fcPosInf;
  أكد(isNaN() && "Other class of FP constant");
  أرجع isSignaling() ? fcSNan : fcQNan;
}

منطقي APFloat::getExactInverse(APFloat *Inv) ثابت {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  إذا (!isFiniteNonZero())
    أرجع خطأ;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  إذا (isDenormal())
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
  إذا (Exp == INT_MIN)
    أرجع خطأ;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  إذا (Reciprocal.getExactLog2Abs() != -Exp)
    أرجع خطأ;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  إذا (Reciprocal.isDenormal())
    أرجع خطأ;

  أكد(Reciprocal.isFiniteNonZero());

  إذا (Inv)
    *Inv = std::move(Reciprocal);

  أرجع سچ;
}

APFloat::op_حالت APFloat::convert(ثابت fltSemantics &منزل_Semantics_2,
                                   rounding_گهڻائي RM, منطقي *losesInfo) {
  إذا (&getSemantics() == &منزل_Semantics_2) {
    *losesInfo = خطأ;
    أرجع opOK;
  }
  إذا (استعمال_ڪري_Layout<IEEEFloat>(getSemantics()) &&
      استعمال_ڪري_Layout<IEEEFloat>(منزل_Semantics_2))
    أرجع U.IEEE.convert(منزل_Semantics_2, RM, losesInfo);
  إذا (استعمال_ڪري_Layout<IEEEFloat>(getSemantics()) &&
      استعمال_ڪري_Layout<DoubleAPFloat>(منزل_Semantics_2)) {
    أكد(&منزل_Semantics_2 == &APFloat_بنياد::semPPCDoubleDouble);
    خودڪار Ret =
        U.IEEE.convert(APFloat_بنياد::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *هذا = APFloat(منزل_Semantics_2, U.IEEE.bitcast_منزل_APInt());
    أرجع Ret;
  }
  إذا (استعمال_ڪري_Layout<DoubleAPFloat>(getSemantics()) &&
      استعمال_ڪري_Layout<IEEEFloat>(منزل_Semantics_2)) {
    خودڪار Ret = getIEEE().convert(منزل_Semantics_2, RM, losesInfo);
    *هذا = APFloat(std::move(getIEEE()), منزل_Semantics_2);
    أرجع Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_قدر(ثابت fltSemantics &Semantics) {
  أرجع APFloat(Semantics, APInt::getAllOnes(Semantics.سائيز_اندريون_بٽ));
}

فراغ APFloat::print(raw_ostream &OS) ثابت {
  Small_ويڪٽر<حرف, 16> بفر;
  منزل_String(بفر);
  OS << بفر;
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طريقو فراغ APFloat::dump() ثابت {
  print(dbgs());
  dbgs() << '\n';
}
#جيڪڏهن_پورو

فراغ APFloat::Profile(Folding_سيٽ_ڳنڍ_ID &NID) ثابت {
  NID.جوڙيو_2(bitcast_منزل_APInt());
}

APFloat::op_حالت APFloat::convert_منزل_Integer(APSInt &نتيجو_2,
                                            rounding_گهڻائي rounding_گهڻائي_2,
                                            منطقي *isExact) ثابت {
  بنانشان ٻٽو_انگ_ويڪر = نتيجو_2.get_ٻٽو_انگ_ويڪر();
  Small_ويڪٽر<uint64_t, 4> parts(نتيجو_2.get_انگ_لفظ());
  op_حالت حالت_2 = convert_منزل_Integer(parts, ٻٽو_انگ_ويڪر, نتيجو_2.isSigned(),
                                     rounding_گهڻائي_2, isExact);
  // Keeps the original signed-ness.
  نتيجو_2 = APInt(ٻٽو_انگ_ويڪر, parts);
  أرجع حالت_2;
}

مزدوج APFloat::convert_منزل_Double() ثابت {
  إذا (&getSemantics() == &APFloat_بنياد::semIEEEdouble)
    أرجع getIEEE().convert_منزل_Double();
  أكد(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat عارضي = *هذا;
  منطقي LosesInfo;
  [[maybe_اڻ_استعمال_ٿيل]] op_حالت St =
      عارضي.convert(APFloat_بنياد::semIEEEdouble, rm_سڀ_کان_ويجهو_Ties_منزل_Even, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  أرجع عارضي.getIEEE().convert_منزل_Double();
}

#جيڪڏهن_وصف_ٿيل HAS_IEE754_FLOAT128
float128 APFloat::convert_منزل_Quad() ثابت {
  إذا (&getSemantics() == &APFloat_بنياد::semIEEEquad)
    أرجع getIEEE().convert_منزل_Quad();
  أكد(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat عارضي = *هذا;
  منطقي LosesInfo;
  [[maybe_اڻ_استعمال_ٿيل]] op_حالت St =
      عارضي.convert(APFloat_بنياد::semIEEEquad, rm_سڀ_کان_ويجهو_Ties_منزل_Even, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  أرجع عارضي.getIEEE().convert_منزل_Quad();
}
#جيڪڏهن_پورو

عائم APFloat::convert_منزل_Float() ثابت {
  إذا (&getSemantics() == &APFloat_بنياد::semIEEEsingle)
    أرجع getIEEE().convert_منزل_Float();
  أكد(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat عارضي = *هذا;
  منطقي LosesInfo;
  [[maybe_اڻ_استعمال_ٿيل]] op_حالت St =
      عارضي.convert(APFloat_بنياد::semIEEEsingle, rm_سڀ_کان_ويجهو_Ties_منزل_Even, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  أرجع عارضي.getIEEE().convert_منزل_Float();
}

بنانشان APFloat_بنياد::get_Arbitrary_FPFormat_سائيز_اندريون_بٽ(StringRef Format) {
  أرجع StringSwitch<بنانشان>(Format)
      .Case("Float8E5M2", get_سائيز_اندريون_بٽ(semFloat8E5M2))
      .Case("Float8E5M2FNUZ", get_سائيز_اندريون_بٽ(semFloat8E5M2FNUZ))
      .Case("Float8E4M3", get_سائيز_اندريون_بٽ(semFloat8E4M3))
      .Case("Float8E4M3FN", get_سائيز_اندريون_بٽ(semFloat8E4M3FN))
      .Case("Float8E4M3FNUZ", get_سائيز_اندريون_بٽ(semFloat8E4M3FNUZ))
      .Case("Float8E4M3B11FNUZ", get_سائيز_اندريون_بٽ(semFloat8E4M3B11FNUZ))
      .Case("Float8E3M4", get_سائيز_اندريون_بٽ(semFloat8E3M4))
      .Case("Float8E8M0FNU", get_سائيز_اندريون_بٽ(semFloat8E8M0FNU))
      .Case("Float6E3M2FN", get_سائيز_اندريون_بٽ(semFloat6E3M2FN))
      .Case("Float6E2M3FN", get_سائيز_اندريون_بٽ(semFloat6E2M3FN))
      .Case("Float4E2M1FN", get_سائيز_اندريون_بٽ(semFloat4E2M1FN))
      .Case("Float8E5M3FNU", get_سائيز_اندريون_بٽ(semFloat8E5M3FNU))
      .Default(0);
}

منطقي APFloat_بنياد::isValidArbitraryFPFormat(StringRef Format) {
  أرجع get_Arbitrary_FPFormat_سائيز_اندريون_بٽ(Format) != 0;
}

ثابت fltSemantics *APFloat_بنياد::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  أرجع StringSwitch<ثابت fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(خالي_اشارو);
}

APFloat::Storage::~Storage() {
  إذا (استعمال_ڪري_Layout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    أرجع;
  }
  إذا (استعمال_ڪري_Layout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    أرجع;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(ثابت APFloat::Storage &RHS) {
  إذا (استعمال_ڪري_Layout<IEEEFloat>(*RHS.semantics)) {
    جديد (هذا) IEEEFloat(RHS.IEEE);
    أرجع;
  }
  إذا (استعمال_ڪري_Layout<DoubleAPFloat>(*RHS.semantics)) {
    جديد (هذا) DoubleAPFloat(RHS.Double);
    أرجع;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  إذا (استعمال_ڪري_Layout<IEEEFloat>(*RHS.semantics)) {
    جديد (هذا) IEEEFloat(std::move(RHS.IEEE));
    أرجع;
  }
  إذا (استعمال_ڪري_Layout<DoubleAPFloat>(*RHS.semantics)) {
    جديد (هذا) DoubleAPFloat(std::move(RHS.Double));
    أرجع;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::عامل=(ثابت APFloat::Storage &RHS) {
  إذا (استعمال_ڪري_Layout<IEEEFloat>(*semantics) &&
      استعمال_ڪري_Layout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } وإلا إذا (استعمال_ڪري_Layout<DoubleAPFloat>(*semantics) &&
             استعمال_ڪري_Layout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } وإلا إذا (هذا != &RHS) {
    هذا->~Storage();
    جديد (هذا) Storage(RHS);
  }
  أرجع *هذا;
}

APFloat::Storage &APFloat::Storage::عامل=(APFloat::Storage &&RHS) {
  إذا (استعمال_ڪري_Layout<IEEEFloat>(*semantics) &&
      استعمال_ڪري_Layout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } وإلا إذا (استعمال_ڪري_Layout<DoubleAPFloat>(*semantics) &&
             استعمال_ڪري_Layout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } وإلا إذا (هذا != &RHS) {
    هذا->~Storage();
    جديد (هذا) Storage(std::move(RHS));
  }
  أرجع *هذا;
}

نطاق {

APFloat::op_حالت get_Op_حالت_From_Libc(صحيح libc_exceptions) {
  APFloat::op_حالت حالت_2 = APFloat::opOK;
  إذا (libc_exceptions & FE_INVALID)
    حالت_2 = static_cast<APFloat::op_حالت>(حالت_2 | APFloat::opInvalidOp);
  إذا (libc_exceptions & FE_DIVBYZERO)
    حالت_2 = static_cast<APFloat::op_حالت>(حالت_2 | APFloat::opDivByZero);
  إذا (libc_exceptions & FE_حد_کان_مٿي)
    حالت_2 = static_cast<APFloat::op_حالت>(حالت_2 | APFloat::op_حد_کان_مٿي);
  إذا (libc_exceptions & FE_UNDERFLOW)
    حالت_2 = static_cast<APFloat::op_حالت>(حالت_2 | APFloat::opUnderflow);
  إذا (libc_exceptions & FE_INEXACT)
    حالت_2 = static_cast<APFloat::op_حالت>(حالت_2 | APFloat::opInexact);
  أرجع حالت_2;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> قوت_نما(ثابت APFloat &x, Rounding_گهڻائي rounding_گهڻائي_2,
                           APFloat::op_حالت *حالت_2) {

  إذا (rounding_گهڻائي_2 == APFloat_بنياد::rm_سڀ_کان_ويجهو_Ties_منزل_Even) {
    إذا (APFloat::Semantics_منزل_Enum(x.getSemantics()) ==
        APFloat_بنياد::S_IEEEsingle) {
      عائم x_val = x.convert_منزل_Float();
      صحيح exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      إذا (حالت_2) {
        *حالت_2 = get_Op_حالت_From_Libc(exc);
        إذا (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *حالت_2 =
              static_cast<APFloat::op_حالت>(*حالت_2 | APFloat::opInvalidOp);
        }
      }
      عائم نتيجو_2 = LIBC_NAMESPACE::shared::expf(x_val);
      أرجع APFloat(نتيجو_2);
    }
    إذا (APFloat::Semantics_منزل_Enum(x.getSemantics()) ==
        APFloat_بنياد::S_IEEEdouble) {
      مزدوج x_val = x.convert_منزل_Double();
      صحيح exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      إذا (حالت_2) {
        *حالت_2 = get_Op_حالت_From_Libc(exc);
        إذا (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *حالت_2 =
              static_cast<APFloat::op_حالت>(*حالت_2 | APFloat::opInvalidOp);
        }
      }
      مزدوج نتيجو_2 = LIBC_NAMESPACE::shared::قوت_نما(x_val);
      أرجع APFloat(نتيجو_2);
    }
  }
  أرجع std::nullopt;
}

} // namespace llvm

#وصف_رد APFLOAT_DISPATCH_ON_SEMANTICS
