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
#تعریف_کرو LIBC_NAMESPACE __llvm_libc_apfloat
#تعریف_کرو LIBC_MATH (LIBC_MATH_NO_ERRNO | LIBC_MATH_NO_EXCEPT)

#ضمن "shared/math.h"
#ضمن "shared/math_check_exceptions.h"

#تعریف_کرو APFLOAT_DISPATCH_ON_SEMANTICS(طریقہ_CALL)                             \
  افعل {                                                                         \
    إذا (ورتدا_Layout<IEEEFloat>(getSemantics()))                                 \
      أرجع U.IEEE.طریقہ_CALL;                                               \
    إذا (ورتدا_Layout<DoubleAPFloat>(getSemantics()))                             \
      أرجع U.Double.طریقہ_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } طالما (خطأ)

ورتوں نطاق llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#تعریف_کرو Pack_Categories_Into_کلید(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloat_ادھار::integer_Part_چوڑائی % 4 == 0, "Part width must be divisible by 4!");

نطاق llvm {

constexpr fltSemantics APFloat_ادھار::semIEEEhalf = {15, -14, 11, 16};
constexpr fltSemantics APFloat_ادھار::semBFloat = {127, -126, 8, 16};
constexpr fltSemantics APFloat_ادھار::semIEEEsingle = {127, -126, 24, 32};
constexpr fltSemantics APFloat_ادھار::semIEEEdouble = {1023, -1022, 53, 64};
constexpr fltSemantics APFloat_ادھار::semIEEEquad = {16383, -16382, 113, 128};
constexpr fltSemantics APFloat_ادھار::semX87DoubleExtended = {
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
constexpr fltSemantics APFloat_ادھار::semPPCDoubleDouble = {-1, 0, 0, 128};
constexpr fltSemantics APFloat_ادھار::semPPCDoubleDoubleLegacy = {
    1023, -1022 + 53, 53 + 53, 128};
constexpr fltSemantics APFloat_ادھار::semFloat8E5M2 = {15, -14, 3, 8};
constexpr fltSemantics APFloat_ادھار::semFloat8E5M2FNUZ = {
    15, -15, 3, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_ادھار::semFloat8E4M3 = {7, -6, 4, 8};
constexpr fltSemantics APFloat_ادھار::semFloat8E4M3FN = {
    8, -6, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::AllOnes};
constexpr fltSemantics APFloat_ادھار::semFloat8E4M3FNUZ = {
    7, -7, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_ادھار::semFloat8E4M3B11FNUZ = {
    4, -10, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_ادھار::semFloat8E3M4 = {3, -2, 5, 8};
constexpr fltSemantics APFloat_ادھار::semFloatTF32 = {127, -126, 11, 19};
constexpr fltSemantics APFloat_ادھار::semFloat8E8M0FNU = {
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

constexpr fltSemantics APFloat_ادھار::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    سچ,
    خطأ,
    خطأ};

constexpr fltSemantics APFloat_ادھار::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_ادھار::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_ادھار::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_ادھار::semBogus = {0, 0, 0, 0};

ثابت llvm::fltSemantics &APFloat_ادھار::Enum_منزل_Semantics(Semantics S) {
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

APFloat_ادھار::Semantics
APFloat_ادھار::Semantics_منزل_Enum(ثابت llvm::fltSemantics &Sem) {
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

منطقي APFloat_ادھار::isRepresentableBy(ثابت fltSemantics &A,
                                    ثابت fltSemantics &B) {
  أرجع A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

منطقي APFloat_ادھار::is_Losslessly_Convertible_منزل(ثابت fltSemantics &From,
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
  APFloat سبھ_توں_چھوٹا_From = APFloat::get_سبھ_توں_چھوٹا(From);
  منطقي LosesInfo = خطأ;
  (فراغ)سبھ_توں_چھوٹا_From.convert(منزل, APFloat::rm_سبھ_توں_نیڑے_Ties_منزل_Even, &LosesInfo);
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
[[maybe_بن_ورتیا]] constexpr بنانشانی maxExponent = 16383;

// Decimal exponents in the f16 and f32 ranges need at most 2 and 6 parts.
// Keep 16 parts (128 bytes) inline: enough for 5^413, but not a large stack
// frame. Longer fractional spellings use heap storage.
ساكن constexpr بنانشانی عدد_Pow5_Parts_On_ڈھیر = 16;

ساكن بنانشانی صحيح power_دا_Five_Parts_For_Exponent(بنانشانی صحيح power) {
  أكد(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  أرجع 2 + ((power * 815) / (351 * APFloat_ادھار::integer_Part_چوڑائی));
}

بنانشانی صحيح APFloat_ادھار::semanticsPrecision(ثابت fltSemantics &semantics) {
  أرجع semantics.precision;
}
APFloat_ادھار::ExponentType
APFloat_ادھار::semanticsMaxExponent(ثابت fltSemantics &semantics) {
  أرجع semantics.maxExponent;
}
APFloat_ادھار::ExponentType
APFloat_ادھار::semanticsMinExponent(ثابت fltSemantics &semantics) {
  أرجع semantics.minExponent;
}
بنانشانی صحيح APFloat_ادھار::semantics_ناپ_وچ_بٹ(ثابت fltSemantics &semantics) {
  أرجع semantics.ناپ_وچ_بٹ;
}
بنانشانی صحيح APFloat_ادھار::semantics_Int_ناپ_وچ_بٹ(ثابت fltSemantics &semantics,
                                                 منطقي isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  بنانشانی صحيح Min_ثنائی_ہندسہ_چوڑائی = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  إذا (isSigned)
    ++Min_ثنائی_ہندسہ_چوڑائی;
  أرجع Min_ثنائی_ہندسہ_چوڑائی;
}

منطقي APFloat_ادھار::semanticsHasZero(ثابت fltSemantics &semantics) {
  أرجع semantics.hasZero;
}

منطقي APFloat_ادھار::semanticsHasSignedRepr(ثابت fltSemantics &semantics) {
  أرجع semantics.hasSignedRepr;
}

منطقي APFloat_ادھار::semanticsHasInf(ثابت fltSemantics &semantics) {
  أرجع semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

منطقي APFloat_ادھار::semanticsHasNaN(ثابت fltSemantics &semantics) {
  أرجع semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

منطقي APFloat_ادھار::isIEEELikeFP(ثابت fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  أرجع Semantics_منزل_Enum(semantics) <= S_IEEEquad;
}

منطقي APFloat_ادھار::has_Sign_ثنائی_ہندسہ_وچ_MSB(ثابت fltSemantics &semantics) {
  أرجع semantics.has_Sign_ثنائی_ہندسہ_وچ_MSB;
}

منطقي APFloat_ادھار::is_Representable_As_عام_وچ(ثابت fltSemantics &Src,
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

بنانشانی APFloat_ادھار::get_ناپ_وچ_بٹ(ثابت fltSemantics &Sem) {
  أرجع Sem.ناپ_وچ_بٹ;
}

ساكن constexpr APFloat_ادھار::ExponentType
exponentZero(ثابت fltSemantics &semantics) {
  أرجع semantics.minExponent - 1;
}

ساكن constexpr APFloat_ادھار::ExponentType
exponentInf(ثابت fltSemantics &semantics) {
  أرجع semantics.maxExponent + 1;
}

ساكن constexpr APFloat_ادھار::ExponentType
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
  أرجع make_error<StringError>(Err, inconvertible_Error_کوڈ());
}

ساكن constexpr مضمن بنانشانی صحيح part_گنتی_For_بٹ(بنانشانی صحيح بٹ) {
  أرجع std::max(1u, (بٹ + APFloat_ادھار::integer_Part_چوڑائی - 1) /
                          APFloat_ادھار::integer_Part_چوڑائی);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
ساكن مضمن بنانشانی صحيح
dec_Digit_قدر(بنانشانی صحيح c)
{
  أرجع c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
ساكن Expected<صحيح> readExponent(StringRef::iterator begin,
                                  StringRef::iterator سرا) {
  ثابت بنانشانی صحيح overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  إذا (p == سرا || ((*p == '-' || *p == '+') && (p + 1) == سرا))
    أرجع 0;

  منطقي isNegative = *p == '-';
  إذا (*p == '-' || *p == '+') {
    p++;
    إذا (p == سرا)
      أرجع createError("Exponent has no digits");
  }

  بنانشانی absExponent = dec_Digit_قدر(*p++);
  إذا (absExponent >= 10U)
    أرجع createError("Invalid character in exponent");

  لكل (; p != سرا; ++p) {
    بنانشانی قدر_2 = dec_Digit_قدر(*p);
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
ساكن Expected<صحيح> کل_Exponent(StringRef::iterator p,
                                   StringRef::iterator سرا,
                                   صحيح exponentAdjustment) {
  صحيح exponent = 0;

  إذا (p == سرا)
    أرجع createError("Exponent has no digits");

  منطقي negative = *p == '-';
  إذا (*p == '-' || *p == '+') {
    p++;
    إذا (p == سرا)
      أرجع createError("Exponent has no digits");
  }

  صحيح unsignedExponent = 0;
  منطقي حد_توں_ودھ_2 = خطأ;
  لكل (; p != سرا; ++p) {
    بنانشانی صحيح قدر_2;

    قدر_2 = dec_Digit_قدر(*p);
    إذا (قدر_2 >= 10U)
      أرجع createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + قدر_2;
    إذا (unsignedExponent > 32767) {
      حد_توں_ودھ_2 = سچ;
      اكسر;
    }
  }

  إذا (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    حد_توں_ودھ_2 = سچ;

  إذا (!حد_توں_ودھ_2) {
    exponent = unsignedExponent;
    إذا (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    إذا (exponent > 32767 || exponent < -32768)
      حد_توں_ودھ_2 = سچ;
  }

  إذا (حد_توں_ودھ_2)
    exponent = negative ? -32768: 32767;

  أرجع exponent;
}

ساكن Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator سرا,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = سرا;
  طالما (p != سرا && *p == '0')
    p++;

  إذا (p != سرا && *p == '.') {
    *dot = p++;

    إذا (سرا - begin == 1)
      أرجع createError("Significand has no digits");

    طالما (p != سرا && *p == '0')
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
  ثابت حرف *پہلا_Sig_Digit;
  ثابت حرف *آخری_Sig_Digit;
  صحيح exponent;
  صحيح normalizedExponent;
};

ساكن Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator سرا, decimalInfo *D) {
  StringRef::iterator dot = سرا;

  خودکار PtrOrErr = skipLeadingZeroesAndAnyDot(begin, سرا, &dot);
  إذا (!PtrOrErr)
    أرجع PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->پہلا_Sig_Digit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  لكل (; p != سرا; ++p) {
    إذا (*p == '.') {
      إذا (dot != سرا)
        أرجع createError("String contains multiple dots");
      dot = p++;
      إذا (p == سرا)
        اكسر;
    }
    إذا (dec_Digit_قدر(*p) >= 10U)
      اكسر;
  }

  إذا (p != سرا) {
    إذا (*p != 'e' && *p != 'E')
      أرجع createError("Invalid character in significand");
    إذا (p == begin)
      أرجع createError("Significand has no digits");
    إذا (dot != سرا && p - begin == 1)
      أرجع createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    خودکار ExpOrErr = readExponent(p + 1, سرا);
    إذا (!ExpOrErr)
      أرجع ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    إذا (dot == سرا)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  إذا (p != D->پہلا_Sig_Digit) {
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
              static_cast<APFloat::ExponentType>((p - D->پہلا_Sig_Digit)
                                      - (dot > D->پہلا_Sig_Digit && dot < p)));
  }

  D->آخری_Sig_Digit = p;
  أرجع Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
ساكن Expected<گواچا_Fraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator سرا,
                            بنانشانی صحيح digit_قدر) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  إذا (digit_قدر > 8)
    أرجع lf_ہور_Than_Half;
  وإلا إذا (digit_قدر < 8 && digit_قدر > 0)
    أرجع lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  طالما (p != سرا && (*p == '0' || *p == '.'))
    p++;

  إذا (p == سرا)
    أرجع createError("Invalid trailing hexadecimal fraction!");

  بنانشانی hexDigit = hex_Digit_قدر(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  إذا (hexDigit == UINT_MAX)
    أرجع digit_قدر == 0 ? lfExactlyZero: lfExactlyHalf;
  وإلا
    أرجع digit_قدر == 0 ? lfLessThanHalf: lf_ہور_Than_Half;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
ساكن گواچا_Fraction
گواچا_Fraction_Through_Truncation(ثابت APFloat_ادھار::integerPart *parts,
                              بنانشانی صحيح part_گنتی,
                              بنانشانی صحيح بٹ)
{
  بنانشانی lsb = APInt::tcLSB(parts, part_گنتی);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  إذا (بٹ <= lsb)
    أرجع lfExactlyZero;
  إذا (بٹ == lsb + 1)
    أرجع lfExactlyHalf;
  إذا (بٹ <= part_گنتی * APFloat_ادھار::integer_Part_چوڑائی &&
      APInt::tc_کڈھو_ثنائی_ہندسہ(parts, بٹ - 1))
    أرجع lf_ہور_Than_Half;

  أرجع lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
ساكن گواچا_Fraction
کھسکاؤ_دایاں(APFloat_ادھار::integerPart *dst, بنانشانی صحيح parts, بنانشانی صحيح بٹ)
{
  گواچا_Fraction گواچا_fraction = گواچا_Fraction_Through_Truncation(dst, parts, بٹ);

  APInt::tc_کھسکاؤ_دایاں(dst, parts, بٹ);

  أرجع گواچا_fraction;
}

/* Combine the effect of two lost fractions.  */
ساكن گواچا_Fraction
combine_گواچا_Fractions(گواچا_Fraction ہور_Significant,
                     گواچا_Fraction lessSignificant)
{
  إذا (lessSignificant != lfExactlyZero) {
    إذا (ہور_Significant == lfExactlyZero)
      ہور_Significant = lfLessThanHalf;
    وإلا إذا (ہور_Significant == lfExactlyHalf)
      ہور_Significant = lf_ہور_Than_Half;
  }

  أرجع ہور_Significant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
ساكن بنانشانی صحيح
HUerrBound(منطقي inexactMultiply, بنانشانی صحيح HUerr1, بنانشانی صحيح HUerr2)
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
ساكن APFloat_ادھار::integerPart
ulpsFromBoundary(ثابت APFloat_ادھار::integerPart *parts, بنانشانی صحيح بٹ,
                 منطقي is_سبھ_توں_نیڑے) {
  أكد(بٹ != 0);

  بٹ--;
  بنانشانی گنتی = بٹ / APFloat_ادھار::integer_Part_چوڑائی;
  بنانشانی part_بٹ = بٹ % APFloat_ادھار::integer_Part_چوڑائی + 1;

  APFloat_ادھار::integerPart part =
      parts[گنتی] & (~(APFloat_ادھار::integerPart)0 >>
                      (APFloat_ادھار::integer_Part_چوڑائی - part_بٹ));

  APFloat_ادھار::integerPart boundary;
  إذا (is_سبھ_توں_نیڑے)
    boundary = (APFloat_ادھار::integerPart) 1 << (part_بٹ - 1);
  وإلا
    boundary = 0;

  إذا (گنتی == 0) {
    إذا (part - boundary <= boundary - part)
      أرجع part - boundary;
    وإلا
      أرجع boundary - part;
  }

  إذا (part == boundary) {
    طالما (--گنتی)
      إذا (parts[گنتی])
        أرجع ~(APFloat_ادھار::integerPart) 0; /* A lot.  */

    أرجع parts[0];
  } وإلا إذا (part == boundary - 1) {
    طالما (--گنتی)
      إذا (~parts[گنتی])
        أرجع ~(APFloat_ادھار::integerPart) 0; /* A lot.  */

    أرجع -parts[0];
  }

  أرجع ~(APFloat_ادھار::integerPart) 0; /* A lot.  */
}

ساكن constexpr بنانشانی Power_دا_Five_Part_Counts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

ساكن constexpr APFloat_ادھار::integerPart Power_دا_Five_Parts[] = {
#ضمن "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  بنانشانی جوڑ = 0;
  لكل (بنانشانی گنتی_2 : Power_دا_Five_Part_Counts)
    جوڑ += گنتی_2;
  أرجع جوڑ;
}() == std::ناپ_2(Power_دا_Five_Parts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
ساكن بنانشانی صحيح
powerOf5(APFloat_ادھار::integerPart *dst, بنانشانی صحيح power) {
  ساكن ثابت APFloat_ادھار::integerPart پہلا_Eight_Powers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  Small_سمتیہ<APFloat_ادھار::integerPart, عدد_Pow5_Parts_On_ڈھیر> scratch;
  scratch.resize_for_overwrite(power_دا_Five_Parts_For_Exponent(power));

  APFloat_ادھار::integerPart *p1, *p2;
  ثابت APFloat_ادھار::integerPart *pow5;

  p1 = dst;
  p2 = scratch.انکڑے();

  *p1 = پہلا_Eight_Powers[power & 7];
  power >>= 3;

  بنانشانی نتیجہ_2 = 1;
  pow5 = Power_دا_Five_Parts;

  لكل (بنانشانی صحيح n = 0; power; power >>= 1, n++) {
    بنانشانی parts_گنتی = Power_دا_Five_Part_Counts[n];

    إذا (power & 1) {
      APFloat_ادھار::integerPart *tmp;

      APInt::tc_بھریا_Multiply(p2, p1, pow5, نتیجہ_2, parts_گنتی);
      نتیجہ_2 += parts_گنتی;
      إذا (p2[نتیجہ_2 - 1] == 0)
        نتیجہ_2--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_گنتی;
  }

  إذا (p1 != dst)
    APInt::tc_سونپو(dst, p1, نتیجہ_2);

  أرجع نتیجہ_2;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
ساكن ثابت حرف hex_Digits_ہیٹھلا[] = "0123456789abcdef0";
ساكن ثابت حرف hex_Digits_اتلا[] = "0123456789ABCDEF0";
ساكن ثابت حرف infinityL[] = "infinity";
ساكن ثابت حرف infinityU[] = "INFINITY";
ساكن ثابت حرف NaNL[] = "nan";
ساكن ثابت حرف NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
ساكن بنانشانی صحيح
partAsHex (حرف *dst, APFloat_ادھار::integerPart part, بنانشانی صحيح گنتی,
           ثابت حرف *hexDigitChars)
{
  بنانشانی صحيح نتیجہ_2 = گنتی;

  أكد(گنتی != 0 && گنتی <= APFloat_ادھار::integer_Part_چوڑائی / 4);

  part >>= (APFloat_ادھار::integer_Part_چوڑائی - 4 * گنتی);
  طالما (گنتی--) {
    dst[گنتی] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  أرجع نتیجہ_2;
}

/* Write out an unsigned decimal integer.  */
ساكن حرف *writeUnsignedDecimal(حرف *dst, بنانشانی صحيح n) {
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
    dst = writeUnsignedDecimal(dst, -(بنانشانی) قدر_2);
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
    أرجع APFloat::get_سبھ_توں_چھوٹا(Sem);
  حالة APFloat::fc_عام:
    اكسر;
  }
  إذا (X.isDenormal() || X.is_سبھ_توں_چھوٹا_Normalized())
    أرجع APFloat::get_سبھ_توں_چھوٹا(Sem);
  صحيح Exp = ilogb(X);
  إذا (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  أرجع scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rm_سبھ_توں_نیڑے_Ties_منزل_Even);
}

نطاق detail {
/* Constructors.  */
فراغ IEEEFloat::initialize(ثابت fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  بنانشانی گنتی = part_گنتی();
  إذا (گنتی > 1)
    significand.parts = جديد integerPart[گنتی];
}

فراغ IEEEFloat::freeSignificand() {
  إذا (needsCleanup())
    مٹاؤ [] significand.parts;
}

فراغ IEEEFloat::سونپو(ثابت IEEEFloat &rhs) {
  أكد(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  إذا (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

فراغ IEEEFloat::copySignificand(ثابت IEEEFloat &rhs) {
  أكد(isFiniteNonZero() || category == fcNaN);
  أكد(rhs.part_گنتی() >= part_گنتی());

  APInt::tc_سونپو(significandParts(), rhs.significandParts(),
                  part_گنتی());
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
  بنانشانی عدد_Parts = part_گنتی();

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
  إذا (!fill || fill->get_عدد_الفاظ() < عدد_Parts)
    APInt::tc_سیٹ(significand, 0, عدد_Parts);
  إذا (fill) {
    APInt::tc_سونپو(significand, fill->get_Raw_انکڑے(),
                    std::min(fill->get_عدد_الفاظ(), عدد_Parts));

    // Zero out the excess bits of the significand.
    بنانشانی بٹ_منزل_Preserve = semantics->precision - 1;
    بنانشانی part = بٹ_منزل_Preserve / 64;
    بٹ_منزل_Preserve %= 64;
    significand[part] &= ((1ULL << بٹ_منزل_Preserve) - 1);
    لكل (part++; part != عدد_Parts; ++part)
      significand[part] = 0;
  }

  بنانشانی QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  إذا (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tc_Clear_ثنائی_ہندسہ(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    إذا (APInt::tcIsZero(significand, عدد_Parts))
      APInt::tc_سیٹ_ثنائی_ہندسہ(significand, QNaNBit - 1);
  } وإلا إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } وإلا {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tc_سیٹ_ثنائی_ہندسہ(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  إذا (semantics == &APFloat_ادھار::semX87DoubleExtended)
    APInt::tc_سیٹ_ثنائی_ہندسہ(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::عامل=(ثابت IEEEFloat &rhs) {
  إذا (هذا != &rhs) {
    إذا (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    سونپو(rhs);
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

  rhs.semantics = &APFloat_ادھار::semBogus;
  أرجع *هذا;
}

منطقي IEEEFloat::isDenormal() ثابت {
  أرجع getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tc_کڈھو_ثنائی_ہندسہ(significandParts(), semantics->precision - 1) ==
          0);
}

منطقي IEEEFloat::is_سبھ_توں_چھوٹا() ثابت {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  أرجع isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

منطقي IEEEFloat::is_سبھ_توں_چھوٹا_Normalized() ثابت {
  أرجع getCategory() == fc_عام && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

بنانشانی صحيح IEEEFloat::get_عدد_بلند_بٹ() ثابت {
  ثابت بنانشانی صحيح Part_گنتی = part_گنتی_For_بٹ(semantics->precision);
  ثابت بنانشانی صحيح بٹ_2 = Part_گنتی * integer_Part_چوڑائی;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  ثابت بنانشانی صحيح عدد_بلند_بٹ = (semantics->precision > 1)
                                       ? (بٹ_2 - semantics->precision + 1)
                                       : (بٹ_2 - semantics->precision);
  أرجع عدد_بلند_بٹ;
}

منطقي IEEEFloat::isSignificandAllOnes() ثابت {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  ثابت integerPart *Parts = significandParts();
  ثابت بنانشانی Part_گنتی = part_گنتی_For_بٹ(semantics->precision);
  لكل (بنانشانی i = 0; i < Part_گنتی - 1; i++)
    إذا (~Parts[i])
      أرجع خطأ;

  // Set the unused high bits to all ones when we compare.
  ثابت بنانشانی عدد_بلند_بٹ = get_عدد_بلند_بٹ();
  أكد(عدد_بلند_بٹ <= integer_Part_چوڑائی && عدد_بلند_بٹ > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ثابت integerPart بلند_ثنائی_ہندسہ_Fill =
    ~integerPart(0) << (integer_Part_چوڑائی - عدد_بلند_بٹ);
  إذا ((semantics->precision <= 1) || (~(Parts[Part_گنتی - 1] | بلند_ثنائی_ہندسہ_Fill)))
    أرجع خطأ;

  أرجع سچ;
}

منطقي IEEEFloat::isSignificandAllOnesExceptLSB() ثابت {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  ثابت integerPart *Parts = significandParts();

  إذا (Parts[0] & 1)
    أرجع خطأ;

  ثابت بنانشانی Part_گنتی = part_گنتی_For_بٹ(semantics->precision);
  لكل (بنانشانی i = 0; i < Part_گنتی - 1; i++) {
    إذا (~Parts[i] & ~بنانشانی{!i})
      أرجع خطأ;
  }

  // Set the unused high bits to all ones when we compare.
  ثابت بنانشانی عدد_بلند_بٹ = get_عدد_بلند_بٹ();
  أكد(عدد_بلند_بٹ <= integer_Part_چوڑائی && عدد_بلند_بٹ > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ثابت integerPart بلند_ثنائی_ہندسہ_Fill = ~integerPart(0)
                                  << (integer_Part_چوڑائی - عدد_بلند_بٹ);
  إذا (~(Parts[Part_گنتی - 1] | بلند_ثنائی_ہندسہ_Fill | 0x1))
    أرجع خطأ;

  أرجع سچ;
}

منطقي IEEEFloat::isSignificandAllZeros() ثابت {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  ثابت integerPart *Parts = significandParts();
  ثابت بنانشانی Part_گنتی = part_گنتی_For_بٹ(semantics->precision);

  لكل (بنانشانی i = 0; i < Part_گنتی - 1; i++)
    إذا (Parts[i])
      أرجع خطأ;

  // Compute how many bits are used in the final word.
  ثابت بنانشانی عدد_بلند_بٹ = get_عدد_بلند_بٹ();
  أكد(عدد_بلند_بٹ < integer_Part_چوڑائی && "Can not have more high bits to "
         "clear than integerPartWidth");
  ثابت integerPart بلند_ثنائی_ہندسہ_نقاب = ~integerPart(0) >> عدد_بلند_بٹ;

  إذا ((semantics->precision > 1) && (Parts[Part_گنتی - 1] & بلند_ثنائی_ہندسہ_نقاب))
    أرجع خطأ;

  أرجع سچ;
}

منطقي IEEEFloat::isSignificandAllZerosExceptMSB() ثابت {
  ثابت integerPart *Parts = significandParts();
  ثابت بنانشانی Part_گنتی = part_گنتی_For_بٹ(semantics->precision);

  لكل (بنانشانی i = 0; i < Part_گنتی - 1; i++) {
    إذا (Parts[i])
      أرجع خطأ;
  }

  ثابت بنانشانی عدد_بلند_بٹ = get_عدد_بلند_بٹ();
  ثابت integerPart MSBMask = integerPart(1)
                              << (integer_Part_چوڑائی - عدد_بلند_بٹ);
  أرجع ((semantics->precision <= 1) || (Parts[Part_گنتی - 1] == MSBMask));
}

منطقي IEEEFloat::is_سبھ_توں_وڈا() ثابت {
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
  truncated.گیڑا_منزل_Integral(rmTowardZero);
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

  أرجع std::equal(significandParts(), significandParts() + part_گنتی(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics, integerPart قدر_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fc_عام;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = قدر_2;
  normalize(rm_سبھ_توں_نیڑے_Ties_منزل_Even, lfExactlyZero);
}

IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(خطأ) : make_سبھ_توں_چھوٹا_Normalized(خطأ);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics, uninitialized_ٹیگ ٹیگ_2)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(ثابت IEEEFloat &rhs) {
  initialize(rhs.semantics);
  سونپو(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloat_ادھار::semBogus) {
  *هذا = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

بنانشانی صحيح IEEEFloat::part_گنتی() ثابت {
  أرجع part_گنتی_For_بٹ(semantics->precision + 1);
}

ثابت APFloat::integerPart *IEEEFloat::significandParts() ثابت {
  أرجع const_cast<IEEEFloat *>(هذا)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  إذا (part_گنتی() > 1)
    أرجع significand.parts;
  وإلا
    أرجع &significand.part;
}

فراغ IEEEFloat::zeroSignificand() {
  APInt::tc_سیٹ(significandParts(), 0, part_گنتی());
}

/* Increment an fcNormal floating point number's significand.  */
فراغ IEEEFloat::incrementSignificand() {
  [[maybe_بن_ورتیا]] integerPart حاصل =
      APInt::tcIncrement(significandParts(), part_گنتی());

  /* Our callers should never cause us to overflow.  */
  أكد(حاصل == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::جوڑو_Significand(ثابت IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  أكد(semantics == rhs.semantics);
  أكد(exponent == rhs.exponent);

  أرجع APInt::tc_جوڑو(parts, rhs.significandParts(), 0, part_گنتی());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::گھٹاؤ_Significand(ثابت IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  أكد(semantics == rhs.semantics);
  أكد(exponent == rhs.exponent);

  أرجع APInt::tc_گھٹاؤ(parts, rhs.significandParts(), borrow,
                           part_گنتی());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
گواچا_Fraction IEEEFloat::multiplySignificand(ثابت IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            منطقي ignoreAddend) {
  integerPart scratch[4];
  منطقي ignored;

  أكد(semantics == rhs.semantics);

  بنانشانی precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  بنانشانی new_Parts_گنتی = part_گنتی_For_بٹ(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *بھریا_Significand =
      new_Parts_گنتی > 4 ? جديد integerPart[new_Parts_گنتی] : scratch;

  integerPart *lhsSignificand = significandParts();
  بنانشانی parts_گنتی = part_گنتی();

  APInt::tc_بھریا_Multiply(بھریا_Significand, lhsSignificand,
                        rhs.significandParts(), parts_گنتی, parts_گنتی);

  گواچا_Fraction گواچا_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  بنانشانی omsb = APInt::tcMSB(بھریا_Significand, new_Parts_گنتی) + 1;
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
    بنانشانی extendedPrecision = 2 * precision + 1;
    إذا (omsb != extendedPrecision - 1) {
      أكد(extendedPrecision > omsb);
      APInt::tc_کھسکاؤ_بایاں(بھریا_Significand, new_Parts_گنتی,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    إذا (new_Parts_گنتی == 1)
      significand.part = بھریا_Significand[0];
    وإلا
      significand.parts = بھریا_Significand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_بن_ورتیا]] op_حالت حالت_2 = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    أكد(حالت_2 == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    گواچا_fraction = extendedAddend.کھسکاؤ_Significand_دایاں(1);
    أكد(گواچا_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    گواچا_fraction = جوڑو_Or_گھٹاؤ_Significand(extendedAddend, خطأ);

    /* Restore our state.  */
    إذا (new_Parts_گنتی == 1)
      بھریا_Significand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(بھریا_Significand, new_Parts_گنتی) + 1;
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
    بنانشانی صحيح بٹ, significantParts;
    گواچا_Fraction lf;

    بٹ = omsb - precision;
    significantParts = part_گنتی_For_بٹ(omsb);
    lf = کھسکاؤ_دایاں(بھریا_Significand, significantParts, بٹ);
    گواچا_fraction = combine_گواچا_Fractions(lf, گواچا_fraction);
    exponent += بٹ;
  }

  APInt::tc_سونپو(lhsSignificand, بھریا_Significand, parts_گنتی);

  إذا (new_Parts_گنتی > 4)
    مٹاؤ [] بھریا_Significand;

  أرجع گواچا_fraction;
}

گواچا_Fraction IEEEFloat::multiplySignificand(ثابت IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  أرجع multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
گواچا_Fraction IEEEFloat::divideSignificand(ثابت IEEEFloat &rhs) {
  integerPart scratch[4];

  أكد(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  ثابت integerPart *rhsSignificand = rhs.significandParts();
  بنانشانی parts_گنتی = part_گنتی();

  integerPart *dividend =
      parts_گنتی > 2 ? جديد integerPart[parts_گنتی * 2] : scratch;
  integerPart *مقسوم_علیہ = dividend + parts_گنتی;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  لكل (بنانشانی i = 0; i < parts_گنتی; i++) {
    dividend[i] = lhsSignificand[i];
    مقسوم_علیہ[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  بنانشانی صحيح precision = semantics->precision;

  /* Normalize the divisor.  */
  بنانشانی ثنائی_ہندسہ = precision - APInt::tcMSB(مقسوم_علیہ, parts_گنتی) - 1;
  إذا (ثنائی_ہندسہ) {
    exponent += ثنائی_ہندسہ;
    APInt::tc_کھسکاؤ_بایاں(مقسوم_علیہ, parts_گنتی, ثنائی_ہندسہ);
  }

  /* Normalize the dividend.  */
  ثنائی_ہندسہ = precision - APInt::tcMSB(dividend, parts_گنتی) - 1;
  إذا (ثنائی_ہندسہ) {
    exponent -= ثنائی_ہندسہ;
    APInt::tc_کھسکاؤ_بایاں(dividend, parts_گنتی, ثنائی_ہندسہ);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  إذا (APInt::tcCompare(dividend, مقسوم_علیہ, parts_گنتی) < 0) {
    exponent--;
    APInt::tc_کھسکاؤ_بایاں(dividend, parts_گنتی, 1);
    أكد(APInt::tcCompare(dividend, مقسوم_علیہ, parts_گنتی) >= 0);
  }

  /* Long division.  */
  لكل (ثنائی_ہندسہ = precision; ثنائی_ہندسہ; ثنائی_ہندسہ -= 1) {
    إذا (APInt::tcCompare(dividend, مقسوم_علیہ, parts_گنتی) >= 0) {
      APInt::tc_گھٹاؤ(dividend, مقسوم_علیہ, 0, parts_گنتی);
      APInt::tc_سیٹ_ثنائی_ہندسہ(lhsSignificand, ثنائی_ہندسہ - 1);
    }

    APInt::tc_کھسکاؤ_بایاں(dividend, parts_گنتی, 1);
  }

  /* Figure out the lost fraction.  */
  صحيح cmp = APInt::tcCompare(dividend, مقسوم_علیہ, parts_گنتی);

  گواچا_Fraction گواچا_fraction;
  إذا (cmp > 0)
    گواچا_fraction = lf_ہور_Than_Half;
  وإلا إذا (cmp == 0)
    گواچا_fraction = lfExactlyHalf;
  وإلا إذا (APInt::tcIsZero(dividend, parts_گنتی))
    گواچا_fraction = lfExactlyZero;
  وإلا
    گواچا_fraction = lfLessThanHalf;

  إذا (parts_گنتی > 2)
    مٹاؤ [] dividend;

  أرجع گواچا_fraction;
}

بنانشانی صحيح IEEEFloat::significandMSB() ثابت {
  أرجع APInt::tcMSB(significandParts(), part_گنتی());
}

بنانشانی صحيح IEEEFloat::significandLSB() ثابت {
  أرجع APInt::tcLSB(significandParts(), part_گنتی());
}

/* Note that a zero result is NOT normalized to fcZero.  */
گواچا_Fraction IEEEFloat::کھسکاؤ_Significand_دایاں(بنانشانی صحيح بٹ) {
  /* Our exponent should not overflow.  */
  أكد((ExponentType) (exponent + بٹ) >= exponent);

  exponent += بٹ;

  أرجع کھسکاؤ_دایاں(significandParts(), part_گنتی(), بٹ);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
فراغ IEEEFloat::کھسکاؤ_Significand_بایاں(بنانشانی صحيح بٹ) {
  أكد(بٹ < semantics->precision ||
         (semantics->precision == 1 && بٹ <= 1));

  إذا (بٹ) {
    بنانشانی صحيح parts_گنتی = part_گنتی();

    APInt::tc_کھسکاؤ_بایاں(significandParts(), parts_گنتی, بٹ);
    exponent -= بٹ;

    أكد(!APInt::tcIsZero(significandParts(), parts_گنتی));
  }
}

APFloat::cmp_نتیجہ IEEEFloat::compare_Absolute_قدر(ثابت IEEEFloat &rhs) ثابت {
  أكد(semantics == rhs.semantics);
  أكد(isFiniteNonZero());
  أكد(rhs.isFiniteNonZero());

  صحيح compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  إذا (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_گنتی());

  إذا (compare > 0)
    أرجع cmpGreaterThan;
  وإلا إذا (compare < 0)
    أرجع cmpLessThan;
  وإلا
    أرجع cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
ساكن فراغ tc_سیٹ_گھٹ_توں_گھٹ_Significant_بٹ(APInt::لفظ_Type *dst, بنانشانی parts,
                                      بنانشانی بٹ) {
  بنانشانی i = 0;
  طالما (بٹ > APInt::APINT_بٹ_فی_لفظ) {
    dst[i++] = ~(APInt::لفظ_Type)0;
    بٹ -= APInt::APINT_بٹ_فی_لفظ;
  }

  إذا (بٹ)
    dst[i++] = ~(APInt::لفظ_Type)0 >> (APInt::APINT_بٹ_فی_لفظ - بٹ);

  طالما (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::op_حالت IEEEFloat::handle_حد_توں_ودھ(rounding_منوال rounding_منوال_2) {
  إذا (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    إذا (rounding_منوال_2 == rm_سبھ_توں_نیڑے_Ties_منزل_Even ||
        rounding_منوال_2 == rm_سبھ_توں_نیڑے_Ties_منزل_Away ||
        (rounding_منوال_2 == rm_Toward_مثبت && !sign) ||
        (rounding_منوال_2 == rmTowardNegative && sign)) {
      إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(خطأ, sign);
      وإلا
        category = fcInfinity;
      أرجع static_cast<op_حالت>(op_حد_توں_ودھ | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fc_عام;
  exponent = semantics->maxExponent;
  tc_سیٹ_گھٹ_توں_گھٹ_Significant_بٹ(significandParts(), part_گنتی(),
                            semantics->precision);
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tc_Clear_ثنائی_ہندسہ(significandParts(), 0);

  أرجع opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
منطقي IEEEFloat::گیڑا_Away_From_Zero(rounding_منوال rounding_منوال_2,
                                  گواچا_Fraction گواچا_fraction,
                                  بنانشانی صحيح ثنائی_ہندسہ) ثابت {
  /* NaNs and infinities should not have lost fractions.  */
  أكد(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  أكد(گواچا_fraction != lfExactlyZero);

  بدل (rounding_منوال_2) {
  حالة rm_سبھ_توں_نیڑے_Ties_منزل_Away:
    أرجع گواچا_fraction == lfExactlyHalf || گواچا_fraction == lf_ہور_Than_Half;

  حالة rm_سبھ_توں_نیڑے_Ties_منزل_Even:
    إذا (گواچا_fraction == lf_ہور_Than_Half)
      أرجع سچ;

    /* Our zeroes don't have a significand to test.  */
    إذا (گواچا_fraction == lfExactlyHalf && category != fcZero)
      أرجع APInt::tc_کڈھو_ثنائی_ہندسہ(significandParts(), ثنائی_ہندسہ);

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

APFloat::op_حالت IEEEFloat::normalize(rounding_منوال rounding_منوال_2,
                                       گواچا_Fraction گواچا_fraction) {
  إذا (!isFiniteNonZero())
    أرجع opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  بنانشانی omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  إذا (omsb || گواچا_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    صحيح exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    إذا (exponent + exponentChange > semantics->maxExponent)
      أرجع handle_حد_توں_ودھ(rounding_منوال_2);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    إذا (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    إذا (exponentChange < 0) {
      أكد(گواچا_fraction == lfExactlyZero);

      کھسکاؤ_Significand_بایاں(-exponentChange);

      أرجع opOK;
    }

    إذا (exponentChange > 0) {
      گواچا_Fraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = کھسکاؤ_Significand_دایاں(exponentChange);

      گواچا_fraction = combine_گواچا_Fractions(lf, گواچا_fraction);

      /* Keep OMSB up-to-date.  */
      إذا (omsb > (بنانشانی) exponentChange)
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
    أرجع handle_حد_توں_ودھ(rounding_منوال_2);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  إذا (گواچا_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    إذا (omsb == 0) {
      category = fcZero;
      إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = خطأ;
      إذا (!semantics->hasZero)
        make_سبھ_توں_چھوٹا_Normalized(خطأ);
    }

    أرجع opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  إذا (گیڑا_Away_From_Zero(rounding_منوال_2, گواچا_fraction, 0)) {
    إذا (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    إذا (omsb == (بنانشانی) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      إذا (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        أرجع handle_حد_توں_ودھ(sign ? rmTowardNegative : rm_Toward_مثبت);

      کھسکاؤ_Significand_دایاں(1);

      أرجع opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      أرجع handle_حد_توں_ودھ(rounding_منوال_2);
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
      make_سبھ_توں_چھوٹا_Normalized(خطأ);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  أرجع (op_حالت) (opUnderflow | opInexact);
}

APFloat::op_حالت IEEEFloat::جوڑو_Or_گھٹاؤ_Specials(ثابت IEEEFloat &rhs,
                                                   منطقي گھٹاؤ) {
  بدل (Pack_Categories_Into_کلید(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(خالی_سوچک);

  حالة Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالة Pack_Categories_Into_کلید(fc_عام, fcNaN):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    سونپو(rhs);
    [[fallthrough]];
  حالة Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالة Pack_Categories_Into_کلید(fcNaN, fc_عام):
  حالة Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcNaN, fcNaN):
    إذا (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_کلید(fc_عام, fcZero):
  حالة Pack_Categories_Into_کلید(fcInfinity, fc_عام):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcZero):
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fc_عام, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ گھٹاؤ;
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fcZero, fc_عام):
    سونپو(rhs);
    sign = rhs.sign ^ گھٹاؤ;
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    إذا (((sign ^ rhs.sign)!=0) != گھٹاؤ) {
      makeNaN();
      أرجع opInvalidOp;
    }

    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fc_عام, fc_عام):
    أرجع opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
گواچا_Fraction IEEEFloat::جوڑو_Or_گھٹاؤ_Significand(ثابت IEEEFloat &rhs,
                                                 منطقي گھٹاؤ) {
  [[maybe_بن_ورتیا]] integerPart حاصل = 0;
  گواچا_Fraction گواچا_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  گھٹاؤ ^= static_cast<منطقي>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  صحيح بٹ = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  إذا (گھٹاؤ) {
    إذا ((بٹ < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat عارضی_rhs(rhs);
    منطقي گواچا_fraction_is_from_rhs = خطأ;

    إذا (بٹ == 0)
      گواچا_fraction = lfExactlyZero;
    وإلا إذا (بٹ > 0) {
      گواچا_fraction = عارضی_rhs.کھسکاؤ_Significand_دایاں(بٹ - 1);
      گواچا_fraction_is_from_rhs = سچ;
      کھسکاؤ_Significand_بایاں(1);
    } وإلا {
      گواچا_fraction = کھسکاؤ_Significand_دایاں(-بٹ - 1);
      عارضی_rhs.کھسکاؤ_Significand_بایاں(1);
    }

    // Should we reverse the subtraction.
    cmp_نتیجہ cmp_نتیجہ_2 = compare_Absolute_قدر(عارضی_rhs);
    إذا (cmp_نتیجہ_2 == cmpLessThan) {
      منطقي borrow =
          گواچا_fraction != lfExactlyZero && !گواچا_fraction_is_from_rhs;
      إذا (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        إذا (گواچا_fraction == lfLessThanHalf)
          گواچا_fraction = lf_ہور_Than_Half;
        وإلا إذا (گواچا_fraction == lf_ہور_Than_Half)
          گواچا_fraction = lfLessThanHalf;
      }
      حاصل = عارضی_rhs.گھٹاؤ_Significand(*هذا, borrow);
      copySignificand(عارضی_rhs);
      sign = !sign;
    } وإلا إذا (cmp_نتیجہ_2 == cmpGreaterThan) {
      منطقي borrow = گواچا_fraction != lfExactlyZero && گواچا_fraction_is_from_rhs;
      إذا (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        إذا (گواچا_fraction == lfLessThanHalf)
          گواچا_fraction = lf_ہور_Than_Half;
        وإلا إذا (گواچا_fraction == lf_ہور_Than_Half)
          گواچا_fraction = lfLessThanHalf;
      }
      حاصل = گھٹاؤ_Significand(عارضی_rhs, borrow);
    } وإلا { // cmpEqual
      zeroSignificand();
      إذا (گواچا_fraction != lfExactlyZero && گواچا_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    أكد(!حاصل);
  } وإلا {
    إذا (بٹ > 0) {
      IEEEFloat عارضی_rhs(rhs);

      گواچا_fraction = عارضی_rhs.کھسکاؤ_Significand_دایاں(بٹ);
      حاصل = جوڑو_Significand(عارضی_rhs);
    } وإلا {
      گواچا_fraction = کھسکاؤ_Significand_دایاں(-بٹ);
      حاصل = جوڑو_Significand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    أكد(!حاصل);
  }

  أرجع گواچا_fraction;
}

APFloat::op_حالت IEEEFloat::multiplySpecials(ثابت IEEEFloat &rhs) {
  بدل (Pack_Categories_Into_کلید(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(خالی_سوچک);

  حالة Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالة Pack_Categories_Into_کلید(fc_عام, fcNaN):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    سونپو(rhs);
    sign = خطأ;
    [[fallthrough]];
  حالة Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالة Pack_Categories_Into_کلید(fcNaN, fc_عام):
  حالة Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    إذا (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_کلید(fc_عام, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcInfinity, fc_عام):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
    category = fcInfinity;
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fcZero, fc_عام):
  حالة Pack_Categories_Into_کلید(fc_عام, fcZero):
  حالة Pack_Categories_Into_کلید(fcZero, fcZero):
    category = fcZero;
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcZero):
    makeNaN();
    أرجع opInvalidOp;

  حالة Pack_Categories_Into_کلید(fc_عام, fc_عام):
    أرجع opOK;
  }
}

APFloat::op_حالت IEEEFloat::divideSpecials(ثابت IEEEFloat &rhs) {
  بدل (Pack_Categories_Into_کلید(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(خالی_سوچک);

  حالة Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالة Pack_Categories_Into_کلید(fc_عام, fcNaN):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    سونپو(rhs);
    sign = خطأ;
    [[fallthrough]];
  حالة Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالة Pack_Categories_Into_کلید(fcNaN, fc_عام):
  حالة Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    إذا (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_کلید(fcInfinity, fcZero):
  حالة Pack_Categories_Into_کلید(fcInfinity, fc_عام):
  حالة Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fc_عام):
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fc_عام, fcInfinity):
    category = fcZero;
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fc_عام, fcZero):
    إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(خطأ, sign);
    وإلا
      category = fcInfinity;
    أرجع opDivByZero;

  حالة Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fcZero):
    makeNaN();
    أرجع opInvalidOp;

  حالة Pack_Categories_Into_کلید(fc_عام, fc_عام):
    أرجع opOK;
  }
}

APFloat::op_حالت IEEEFloat::modSpecials(ثابت IEEEFloat &rhs) {
  بدل (Pack_Categories_Into_کلید(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(خالی_سوچک);

  حالة Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالة Pack_Categories_Into_کلید(fc_عام, fcNaN):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    سونپو(rhs);
    [[fallthrough]];
  حالة Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالة Pack_Categories_Into_کلید(fcNaN, fc_عام):
  حالة Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcNaN, fcNaN):
    إذا (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fc_عام):
  حالة Pack_Categories_Into_کلید(fc_عام, fcInfinity):
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fc_عام, fcZero):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcZero):
  حالة Pack_Categories_Into_کلید(fcInfinity, fc_عام):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fcZero):
    makeNaN();
    أرجع opInvalidOp;

  حالة Pack_Categories_Into_کلید(fc_عام, fc_عام):
    أرجع opOK;
  }
}

APFloat::op_حالت IEEEFloat::remainderSpecials(ثابت IEEEFloat &rhs) {
  بدل (Pack_Categories_Into_کلید(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(خالی_سوچک);

  حالة Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالة Pack_Categories_Into_کلید(fc_عام, fcNaN):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    سونپو(rhs);
    [[fallthrough]];
  حالة Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالة Pack_Categories_Into_کلید(fcNaN, fc_عام):
  حالة Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcNaN, fcNaN):
    إذا (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fc_عام):
  حالة Pack_Categories_Into_کلید(fc_عام, fcInfinity):
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fc_عام, fcZero):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcZero):
  حالة Pack_Categories_Into_کلید(fcInfinity, fc_عام):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fcZero):
    makeNaN();
    أرجع opInvalidOp;

  حالة Pack_Categories_Into_کلید(fc_عام, fc_عام):
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
APFloat::op_حالت IEEEFloat::جوڑو_Or_گھٹاؤ(ثابت IEEEFloat &rhs,
                                           rounding_منوال rounding_منوال_2,
                                           منطقي گھٹاؤ) {
  op_حالت fs = جوڑو_Or_گھٹاؤ_Specials(rhs, گھٹاؤ);

  /* This return code means it was not a simple case.  */
  إذا (fs == opDivByZero) {
    گواچا_Fraction گواچا_fraction;

    گواچا_fraction = جوڑو_Or_گھٹاؤ_Significand(rhs, گھٹاؤ);
    fs = normalize(rounding_منوال_2, گواچا_fraction);

    /* Can only be zero if we lost no fraction.  */
    أكد(category != fcZero || گواچا_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  إذا (category == fcZero) {
    إذا (rhs.category != fcZero || (sign == rhs.sign) == گھٹاؤ)
      sign = (rounding_منوال_2 == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = خطأ;
  }

  أرجع fs;
}

/* Normalized addition.  */
APFloat::op_حالت IEEEFloat::جوڑو(ثابت IEEEFloat &rhs,
                                 rounding_منوال rounding_منوال_2) {
  أرجع جوڑو_Or_گھٹاؤ(rhs, rounding_منوال_2, خطأ);
}

/* Normalized subtraction.  */
APFloat::op_حالت IEEEFloat::گھٹاؤ(ثابت IEEEFloat &rhs,
                                      rounding_منوال rounding_منوال_2) {
  أرجع جوڑو_Or_گھٹاؤ(rhs, rounding_منوال_2, سچ);
}

/* Normalized multiply.  */
APFloat::op_حالت IEEEFloat::multiply(ثابت IEEEFloat &rhs,
                                      rounding_منوال rounding_منوال_2) {
  sign ^= rhs.sign;
  op_حالت fs = multiplySpecials(rhs);

  إذا (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = خطأ;
  إذا (isFiniteNonZero()) {
    گواچا_Fraction گواچا_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_منوال_2, گواچا_fraction);
    إذا (گواچا_fraction != lfExactlyZero)
      fs = (op_حالت) (fs | opInexact);
  }

  أرجع fs;
}

/* Normalized divide.  */
APFloat::op_حالت IEEEFloat::divide(ثابت IEEEFloat &rhs,
                                    rounding_منوال rounding_منوال_2) {
  sign ^= rhs.sign;
  op_حالت fs = divideSpecials(rhs);

  إذا (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = خطأ;
  إذا (isFiniteNonZero()) {
    گواچا_Fraction گواچا_fraction = divideSignificand(rhs);
    fs = normalize(rounding_منوال_2, گواچا_fraction);
    إذا (گواچا_fraction != lfExactlyZero)
      fs = (op_حالت) (fs | opInexact);
  }

  أرجع fs;
}

/* Normalized remainder.  */
APFloat::op_حالت IEEEFloat::remainder(ثابت IEEEFloat &rhs) {
  بنانشانی صحيح origSign = sign;

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
  إذا (P2.جوڑو(rhs, rm_سبھ_توں_نیڑے_Ties_منزل_Even) == opOK) {
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
  fs = VEx.convert(extendedSemantics, rm_سبھ_توں_نیڑے_Ties_منزل_Even, &losesInfo);
  أكد(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rm_سبھ_توں_نیڑے_Ties_منزل_Even, &losesInfo);
  أكد(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.جوڑو(VEx, rm_سبھ_توں_نیڑے_Ties_منزل_Even);
  أكد(fs == opOK);

  إذا (VEx.compare(PEx) == cmpGreaterThan) {
    fs = گھٹاؤ(P, rm_سبھ_توں_نیڑے_Ties_منزل_Even);
    أكد(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.گھٹاؤ(PEx, rm_سبھ_توں_نیڑے_Ties_منزل_Even);
    أكد(fs == opOK);
    fs = VEx.گھٹاؤ(PEx, rm_سبھ_توں_نیڑے_Ties_منزل_Even);
    أكد(fs == opOK);

    cmp_نتیجہ نتیجہ_2 = VEx.compare(PEx);
    إذا (نتیجہ_2 == cmpGreaterThan || نتیجہ_2 == cmpEqual) {
      fs = گھٹاؤ(P, rm_سبھ_توں_نیڑے_Ties_منزل_Even);
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
  بنانشانی صحيح origSign = sign;

  طالما (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_قدر(rhs) != cmpLessThan) {
    صحيح Exp = ilogb(*هذا) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rm_سبھ_توں_نیڑے_Ties_منزل_Even);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    إذا (V.isNaN() || compare_Absolute_قدر(V) == cmpLessThan)
      V = scalbn(rhs, Exp - 1, rm_سبھ_توں_نیڑے_Ties_منزل_Even);
    V.sign = sign;

    fs = گھٹاؤ(V, rm_سبھ_توں_نیڑے_Ties_منزل_Even);

    // When the semantics supports zero, this loop's
    // exit-condition is handled by the 'isFiniteNonZero'
    // category check above. However, when the semantics
    // does not have 'fcZero' and we have reached the
    // minimum possible value, (and any further subtract
    // will underflow to the same value) explicitly
    // provide an exit-path here.
    إذا (!semantics->hasZero && هذا->is_سبھ_توں_چھوٹا())
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
APFloat::op_حالت IEEEFloat::fused_Multiply_جوڑو(ثابت IEEEFloat &multiplicand,
                                              ثابت IEEEFloat &addend,
                                              rounding_منوال rounding_منوال_2) {
  op_حالت fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  إذا (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    گواچا_Fraction گواچا_fraction;

    گواچا_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_منوال_2, گواچا_fraction);
    إذا (گواچا_fraction != lfExactlyZero)
      fs = (op_حالت) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    إذا (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_منوال_2 == rmTowardNegative);
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
      fs = جوڑو_Or_گھٹاؤ(addend, rounding_منوال_2, خطأ);
  }

  أرجع fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::op_حالت IEEEFloat::گیڑا_منزل_Integral(rounding_منوال rounding_منوال_2) {
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
  APInt IntegerConstant(اگلا_Power_Of2(APFloat::semanticsPrecision(*semantics)),
                        1);
  IntegerConstant <<= APFloat::semanticsPrecision(*semantics) - 1;
  IEEEFloat MagicConstant(*semantics);
  op_حالت fs = MagicConstant.convertFromAPInt(IntegerConstant, خطأ,
                                               rm_سبھ_توں_نیڑے_Ties_منزل_Even);
  أكد(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  منطقي داخلہ_Sign = isNegative();

  fs = جوڑو(MagicConstant, rounding_منوال_2);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  گھٹاؤ(MagicConstant, rounding_منوال_2);

  // Restore the input sign.
  إذا (داخلہ_Sign != isNegative())
    changeSign();

  أرجع fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmp_نتیجہ IEEEFloat::compare(ثابت IEEEFloat &rhs) ثابت {
  أكد(semantics == rhs.semantics);

  بدل (Pack_Categories_Into_کلید(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(خالی_سوچک);

  حالة Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالة Pack_Categories_Into_کلید(fcNaN, fc_عام):
  حالة Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcNaN, fcNaN):
  حالة Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالة Pack_Categories_Into_کلید(fc_عام, fcNaN):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    أرجع cmpUnordered;

  حالة Pack_Categories_Into_کلید(fcInfinity, fc_عام):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcZero):
  حالة Pack_Categories_Into_کلید(fc_عام, fcZero):
    إذا (sign)
      أرجع cmpLessThan;
    وإلا
      أرجع cmpGreaterThan;

  حالة Pack_Categories_Into_کلید(fc_عام, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fc_عام):
    إذا (rhs.sign)
      أرجع cmpGreaterThan;
    وإلا
      أرجع cmpLessThan;

  حالة Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
    إذا (sign == rhs.sign)
      أرجع cmpEqual;
    وإلا إذا (sign)
      أرجع cmpLessThan;
    وإلا
      أرجع cmpGreaterThan;

  حالة Pack_Categories_Into_کلید(fcZero, fcZero):
    أرجع cmpEqual;

  حالة Pack_Categories_Into_کلید(fc_عام, fc_عام):
    اكسر;
  }

  cmp_نتیجہ نتیجہ_2;
  /* Two normal numbers.  Do they have the same sign?  */
  إذا (sign != rhs.sign) {
    إذا (sign)
      نتیجہ_2 = cmpLessThan;
    وإلا
      نتیجہ_2 = cmpGreaterThan;
  } وإلا {
    /* Compare absolute values; invert result if negative.  */
    نتیجہ_2 = compare_Absolute_قدر(rhs);

    إذا (sign) {
      إذا (نتیجہ_2 == cmpLessThan)
        نتیجہ_2 = cmpGreaterThan;
      وإلا إذا (نتیجہ_2 == cmpGreaterThan)
        نتیجہ_2 = cmpLessThan;
    }
  }

  أرجع نتیجہ_2;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::op_حالت IEEEFloat::convert(ثابت fltSemantics &منزل_Semantics,
                                     rounding_منوال rounding_منوال_2,
                                     منطقي *losesInfo) {
  op_حالت fs;
  ثابت fltSemantics &fromSemantics = *semantics;
  منطقي is_signaling = isSignaling();

  گواچا_Fraction گواچا_Fraction = lfExactlyZero;
  بنانشانی new_Part_گنتی = part_گنتی_For_بٹ(منزل_Semantics.precision + 1);
  بنانشانی old_Part_گنتی = part_گنتی();
  صحيح کھسکاؤ = منزل_Semantics.precision - fromSemantics.precision;

  منطقي X86SpecialNan = خطأ;
  إذا (&fromSemantics == &APFloat_ادھار::semX87DoubleExtended &&
      &منزل_Semantics != &APFloat_ادھار::semX87DoubleExtended && category == fcNaN &&
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
  إذا (کھسکاؤ < 0 && isFiniteNonZero()) {
    صحيح omsb = significandMSB() + 1;
    صحيح exponentChange = omsb - fromSemantics.precision;
    إذا (exponent + exponentChange < منزل_Semantics.minExponent)
      exponentChange = منزل_Semantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, کھسکاؤ);
    إذا (exponentChange < 0) {
      کھسکاؤ -= exponentChange;
      exponent += exponentChange;
    } وإلا إذا (omsb <= -کھسکاؤ) {
      exponentChange = omsb + کھسکاؤ - 1; // leave at least one bit set
      کھسکاؤ -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  إذا (کھسکاؤ < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    گواچا_Fraction = کھسکاؤ_دایاں(significandParts(), old_Part_گنتی, -کھسکاؤ);

  // Fix the storage so it can hold to new value.
  إذا (new_Part_گنتی > old_Part_گنتی) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = جديد integerPart[new_Part_گنتی];
    APInt::tc_سیٹ(newParts, 0, new_Part_گنتی);
    إذا (isFiniteNonZero() || category==fcNaN)
      APInt::tc_سونپو(newParts, significandParts(), old_Part_گنتی);
    freeSignificand();
    significand.parts = newParts;
  } وإلا إذا (new_Part_گنتی == 1 && old_Part_گنتی != 1) {
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
  إذا (کھسکاؤ > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_کھسکاؤ_بایاں(significandParts(), new_Part_گنتی, کھسکاؤ);

  إذا (isFiniteNonZero()) {
    fs = normalize(rounding_منوال_2, گواچا_Fraction);
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

      *losesInfo = گواچا_Fraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      إذا (!X86SpecialNan && semantics == &APFloat_ادھار::semX87DoubleExtended)
        APInt::tc_سیٹ_ثنائی_ہندسہ(significandParts(), semantics->precision - 1);

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
    make_سبھ_توں_چھوٹا_Normalized(خطأ);
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
    MutableArrayRef<integerPart> parts, بنانشانی صحيح چوڑائی_2, منطقي isSigned,
    rounding_منوال rounding_منوال_2, منطقي *isExact) ثابت {
  *isExact = خطأ;

  /* Handle the three special cases first.  */
  إذا (category == fcInfinity || category == fcNaN)
    أرجع opInvalidOp;

  بنانشانی dst_Parts_گنتی = part_گنتی_For_بٹ(چوڑائی_2);
  أكد(dst_Parts_گنتی <= parts.ناپ_2() && "Integer too big");

  إذا (category == fcZero) {
    APInt::tc_سیٹ(parts.انکڑے(), 0, dst_Parts_گنتی);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    أرجع opOK;
  }

  ثابت integerPart *src = significandParts();

  بنانشانی truncated_بٹ;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  إذا (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tc_سیٹ(parts.انکڑے(), 0, dst_Parts_گنتی);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncated_بٹ = semantics->precision -1U - exponent;
  } وإلا {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    بنانشانی صحيح بٹ = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    إذا (بٹ > چوڑائی_2)
      أرجع opInvalidOp;

    إذا (بٹ < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncated_بٹ = semantics->precision - بٹ;
      APInt::tc_کڈھو(parts.انکڑے(), dst_Parts_گنتی, src, بٹ, truncated_بٹ);
    } وإلا {
      /* We want at least as many bits as are available.  */
      APInt::tc_کڈھو(parts.انکڑے(), dst_Parts_گنتی, src, semantics->precision,
                       0);
      APInt::tc_کھسکاؤ_بایاں(parts.انکڑے(), dst_Parts_گنتی,
                         بٹ - semantics->precision);
      truncated_بٹ = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  گواچا_Fraction گواچا_fraction;
  إذا (truncated_بٹ) {
    گواچا_fraction = گواچا_Fraction_Through_Truncation(src, part_گنتی(),
                                                  truncated_بٹ);
    إذا (گواچا_fraction != lfExactlyZero &&
        گیڑا_Away_From_Zero(rounding_منوال_2, گواچا_fraction, truncated_بٹ)) {
      إذا (APInt::tcIncrement(parts.انکڑے(), dst_Parts_گنتی))
        أرجع opInvalidOp;     /* Overflow.  */
    }
  } وإلا {
    گواچا_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  بنانشانی صحيح omsb = APInt::tcMSB(parts.انکڑے(), dst_Parts_گنتی) + 1;

  إذا (sign) {
    إذا (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      إذا (omsb != 0)
        أرجع opInvalidOp;
    } وإلا {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      إذا (omsb == چوڑائی_2 &&
          APInt::tcLSB(parts.انکڑے(), dst_Parts_گنتی) + 1 != omsb)
        أرجع opInvalidOp;

      /* This case can happen because of rounding.  */
      إذا (omsb > چوڑائی_2)
        أرجع opInvalidOp;
    }

    APInt::tcNegate (parts.انکڑے(), dst_Parts_گنتی);
  } وإلا {
    إذا (omsb >= چوڑائی_2 + !isSigned)
      أرجع opInvalidOp;
  }

  إذا (گواچا_fraction == lfExactlyZero) {
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
                            بنانشانی صحيح چوڑائی_2, منطقي isSigned,
                            rounding_منوال rounding_منوال_2, منطقي *isExact) ثابت {
  op_حالت fs = convert_منزل_Sign_Extended_Integer(parts, چوڑائی_2, isSigned,
                                             rounding_منوال_2, isExact);

  إذا (fs == opInvalidOp) {
    بنانشانی صحيح بٹ, dst_Parts_گنتی;

    dst_Parts_گنتی = part_گنتی_For_بٹ(چوڑائی_2);
    أكد(dst_Parts_گنتی <= parts.ناپ_2() && "Integer too big");

    إذا (category == fcNaN)
      بٹ = 0;
    وإلا إذا (sign)
      بٹ = isSigned;
    وإلا
      بٹ = چوڑائی_2 - isSigned;

    tc_سیٹ_گھٹ_توں_گھٹ_Significant_بٹ(parts.انکڑے(), dst_Parts_گنتی, بٹ);
    إذا (sign && isSigned)
      APInt::tc_کھسکاؤ_بایاں(parts.انکڑے(), dst_Parts_گنتی, چوڑائی_2 - 1);
  }

  أرجع fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::op_حالت IEEEFloat::convertFromUnsignedParts(
    ثابت integerPart *src, بنانشانی صحيح src_گنتی, rounding_منوال rounding_منوال_2) {
  category = fc_عام;
  بنانشانی omsb = APInt::tcMSB(src, src_گنتی) + 1;
  integerPart *dst = significandParts();
  بنانشانی dst_گنتی = part_گنتی();
  بنانشانی precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  گواچا_Fraction گواچا_fraction;
  إذا (precision <= omsb) {
    exponent = omsb - 1;
    گواچا_fraction = گواچا_Fraction_Through_Truncation(src, src_گنتی,
                                                  omsb - precision);
    APInt::tc_کڈھو(dst, dst_گنتی, src, precision, omsb - precision);
  } وإلا {
    exponent = precision - 1;
    گواچا_fraction = lfExactlyZero;
    APInt::tc_کڈھو(dst, dst_گنتی, src, omsb, 0);
  }

  أرجع normalize(rounding_منوال_2, گواچا_fraction);
}

APFloat::op_حالت IEEEFloat::convertFromAPInt(ثابت APInt &Val, منطقي isSigned,
                                              rounding_منوال rounding_منوال_2) {
  بنانشانی صحيح part_گنتی = Val.get_عدد_الفاظ();
  APInt api = Val;

  sign = خطأ;
  إذا (isSigned && api.isNegative()) {
    sign = سچ;
    api = -api;
  }

  أرجع convertFromUnsignedParts(api.get_Raw_انکڑے(), part_گنتی, rounding_منوال_2);
}

Expected<APFloat::op_حالت>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        rounding_منوال rounding_منوال_2) {
  گواچا_Fraction گواچا_fraction = lfExactlyZero;

  category = fc_عام;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  بنانشانی parts_گنتی = part_گنتی();
  بنانشانی ثنائی_ہندسہ_Pos = parts_گنتی * integer_Part_چوڑائی;
  منطقي computedTrailingFraction = خطأ;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator سرا = s.سرا();
  StringRef::iterator dot;
  خودکار PtrOrErr = skipLeadingZeroesAndAnyDot(begin, سرا, &dot);
  إذا (!PtrOrErr)
    أرجع PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator پہلا_Significant_Digit = p;

  طالما (p != سرا) {
    integerPart hex_قدر;

    إذا (*p == '.') {
      إذا (dot != سرا)
        أرجع createError("String contains multiple dots");
      dot = p++;
      استمر;
    }

    hex_قدر = hex_Digit_قدر(*p);
    إذا (hex_قدر == UINT_MAX)
      اكسر;

    p++;

    // Store the number while we have space.
    إذا (ثنائی_ہندسہ_Pos) {
      ثنائی_ہندسہ_Pos -= 4;
      hex_قدر <<= ثنائی_ہندسہ_Pos % integer_Part_چوڑائی;
      significand[ثنائی_ہندسہ_Pos / integer_Part_چوڑائی] |= hex_قدر;
    } وإلا إذا (!computedTrailingFraction) {
      خودکار FractOrErr = trailingHexadecimalFraction(p, سرا, hex_قدر);
      إذا (!FractOrErr)
        أرجع FractOrErr.takeError();
      گواچا_fraction = *FractOrErr;
      computedTrailingFraction = سچ;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  إذا (p == سرا)
    أرجع createError("Hex strings require an exponent");
  إذا (*p != 'p' && *p != 'P')
    أرجع createError("Invalid character in significand");
  إذا (p == begin)
    أرجع createError("Significand has no digits");
  إذا (dot != سرا && p - begin == 1)
    أرجع createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  إذا (p != پہلا_Significant_Digit) {
    صحيح expAdjustment;

    /* Implicit hexadecimal point?  */
    إذا (dot == سرا)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<صحيح>(dot - پہلا_Significant_Digit);
    إذا (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_گنتی * integer_Part_چوڑائی;

    /* Adjust for the given exponent.  */
    خودکار ExpOrErr = کل_Exponent(p + 1, سرا, expAdjustment);
    إذا (!ExpOrErr)
      أرجع ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  أرجع normalize(rounding_منوال_2, گواچا_fraction);
}

APFloat::op_حالت
IEEEFloat::گیڑا_Significand_With_Exponent(ثابت integerPart *decSigParts,
                                        بنانشانی sig_Part_گنتی, صحيح قوت_نما,
                                        rounding_منوال rounding_منوال_2) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  بنانشانی صحيح power = قوت_نما >= 0 ? قوت_نما : -قوت_نما;
  بنانشانی صحيح pow5_Part_سمرتھا = power_دا_Five_Parts_For_Exponent(power);
  Small_سمتیہ<integerPart, عدد_Pow5_Parts_On_ڈھیر> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5_Part_سمرتھا);

  منطقي is_سبھ_توں_نیڑے = rounding_منوال_2 == rm_سبھ_توں_نیڑے_Ties_منزل_Even ||
                   rounding_منوال_2 == rm_سبھ_توں_نیڑے_Ties_منزل_Away;

  بنانشانی parts = part_گنتی_For_بٹ(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  بنانشانی pow5_Part_گنتی = powerOf5(pow5Parts.انکڑے(), power);

  لكل (;; parts *= 2) {
    بنانشانی صحيح excessPrecision, truncated_بٹ;

    calcSemantics.precision = parts * integer_Part_چوڑائی - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncated_بٹ = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    op_حالت sig_حالت = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_گنتی, rm_سبھ_توں_نیڑے_Ties_منزل_Even);
    op_حالت pow_حالت = pow5.convertFromUnsignedParts(
        pow5Parts.انکڑے(), pow5_Part_گنتی, rm_سبھ_توں_نیڑے_Ties_منزل_Even);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += قوت_نما;

    گواچا_Fraction calc_گواچا_Fraction;
    integerPart HUerr, HUdistance;
    بنانشانی صحيح powHUerr;

    إذا (قوت_نما >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calc_گواچا_Fraction = decSig.multiplySignificand(pow5);
      powHUerr = pow_حالت != opOK;
    } وإلا {
      calc_گواچا_Fraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      إذا (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncated_بٹ = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (pow_حالت == opOK && calc_گواچا_Fraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    أكد(APInt::tc_کڈھو_ثنائی_ہندسہ
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calc_گواچا_Fraction != lfExactlyZero, sig_حالت != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, is_سبھ_توں_نیڑے);

    /* Are we guaranteed to round correctly if we truncate?  */
    إذا (HUdistance >= HUerr) {
      APInt::tc_کڈھو(significandParts(), part_گنتی(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calc_گواچا_Fraction = گواچا_Fraction_Through_Truncation(decSig.significandParts(),
                                                       decSig.part_گنتی(),
                                                       truncated_بٹ);
      أرجع static_cast<op_حالت>(normalize(rounding_منوال_2, calc_گواچا_Fraction) |
                                   ((sig_حالت | pow_حالت) & opInexact));
    }
  }
}

Expected<APFloat::op_حالت>
IEEEFloat::convertFromDecimalString(StringRef str, rounding_منوال rounding_منوال_2) {
  decimalInfo D;
  op_حالت fs;

  /* Scan the text.  */
  StringRef::iterator p = str.begin();
  إذا (Error Err = interpretDecimal(p, str.سرا(), &D))
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
  إذا (D.پہلا_Sig_Digit == str.سرا() || dec_Digit_قدر(*D.پہلا_Sig_Digit) >= 10U) {
    category = fcZero;
    fs = opOK;
    إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = خطأ;
    إذا (!semantics->hasZero)
      make_سبھ_توں_چھوٹا_Normalized(خطأ);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } وإلا إذا (D.normalizedExponent - 1 > ودھ_توں_ودھ_عدد / 42039) {
    fs = handle_حد_توں_ودھ(rounding_منوال_2);

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
    fs = normalize(rounding_منوال_2, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } وإلا إذا ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handle_حد_توں_ودھ(rounding_منوال_2);
  } وإلا {
    integerPart *decSignificand;
    بنانشانی صحيح part_گنتی;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_گنتی = static_cast<بنانشانی صحيح>(D.آخری_Sig_Digit - D.پہلا_Sig_Digit) + 1;
    part_گنتی = part_گنتی_For_بٹ(1 + 196 * part_گنتی / 59);
    decSignificand = جديد integerPart[part_گنتی + 1];
    part_گنتی = 0;

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
          إذا (p == str.سرا()) {
            اكسر;
          }
        }
        dec_قدر = dec_Digit_قدر(*p++);
        إذا (dec_قدر >= 10U) {
          مٹاؤ[] decSignificand;
          أرجع createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_قدر;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } طالما (p <= D.آخری_Sig_Digit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_گنتی, part_گنتی + 1, خطأ);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      إذا (decSignificand[part_گنتی])
        part_گنتی++;
    } طالما (p <= D.آخری_Sig_Digit);

    category = fc_عام;
    fs = گیڑا_Significand_With_Exponent(decSignificand, part_گنتی,
                                      D.exponent, rounding_منوال_2);

    مٹاؤ [] decSignificand;
  }

  أرجع fs;
}

منطقي IEEEFloat::convertFromStringSpecials(StringRef str) {
  ثابت size_t MIN_NAME_ناپ = 3;

  إذا (str.ناپ_2() < MIN_NAME_ناپ)
    أرجع خطأ;

  إذا (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(خطأ);
    أرجع سچ;
  }

  منطقي IsNegative = str.consume_front("-");
  إذا (IsNegative) {
    إذا (str.ناپ_2() < MIN_NAME_ناپ)
      أرجع خطأ;

    إذا (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(سچ);
      أرجع سچ;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  منطقي IsSignaling = str.consume_front_insensitive("s");
  إذا (IsSignaling) {
    إذا (str.ناپ_2() < MIN_NAME_ناپ)
      أرجع خطأ;
  }

  إذا (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    إذا (str.خالی_تھاواں()) {
      makeNaN(IsSignaling, IsNegative);
      أرجع سچ;
    }

    // Allow the payload to be inside parentheses.
    إذا (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      إذا (str.ناپ_2() <= 2 || str.پٹھ() != ')')
        أرجع خطأ;

      str = str.ٹکڑا(1, str.ناپ_2() - 1);
    }

    // Determine the payload number's radix.
    بنانشانی Radix = 10;
    إذا (str[0] == '0') {
      إذا (str.ناپ_2() > 1 && tolower(str[1]) == 'x') {
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
IEEEFloat::convertFromString(StringRef str, rounding_منوال rounding_منوال_2) {
  إذا (str.خالی_تھاواں())
    أرجع createError("Invalid string length");

  // Handle special cases.
  إذا (convertFromStringSpecials(str))
    أرجع opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.ناپ_2();
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
                                        rounding_منوال_2);
  }

  أرجع convertFromDecimalString(StringRef(p, slen), rounding_منوال_2);
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
بنانشانی صحيح IEEEFloat::convert_منزل_Hex_String(حرف *dst, بنانشانی صحيح hexDigits,
                                           منطقي اتلا_Case,
                                           rounding_منوال rounding_منوال_2) ثابت {
  حرف *p = dst;
  إذا (sign)
    *dst++ = '-';

  بدل (category) {
  حالة fcInfinity:
    حافظہ_نقل (dst, اتلا_Case ? infinityU: infinityL, حجم infinityU - 1);
    dst += حجم infinityL - 1;
    اكسر;

  حالة fcNaN:
    حافظہ_نقل (dst, اتلا_Case ? NaNU: NaNL, حجم NaNU - 1);
    dst += حجم NaNU - 1;
    اكسر;

  حالة fcZero:
    *dst++ = '0';
    *dst++ = اتلا_Case ? 'X': 'x';
    *dst++ = '0';
    إذا (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = اتلا_Case ? 'P': 'p';
    *dst++ = '0';
    اكسر;

  حالة fc_عام:
    dst = convert_عام_منزل_Hex_String (dst, hexDigits, اتلا_Case, rounding_منوال_2);
    اكسر;
  }

  *dst = 0;

  أرجع static_cast<بنانشانی صحيح>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
حرف *IEEEFloat::convert_عام_منزل_Hex_String(حرف *dst, بنانشانی صحيح hexDigits,
                                          منطقي اتلا_Case,
                                          rounding_منوال rounding_منوال_2) ثابت {
  *dst++ = '0';
  *dst++ = اتلا_Case ? 'X': 'x';

  منطقي گیڑا_اتے = خطأ;
  ثابت حرف *hexDigitChars = اتلا_Case ? hex_Digits_اتلا : hex_Digits_ہیٹھلا;

  ثابت integerPart *significand = significandParts();
  بنانشانی parts_گنتی = part_گنتی();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  بنانشانی قدر_بٹ = semantics->precision + 3;
  بنانشانی کھسکاؤ = integer_Part_چوڑائی - قدر_بٹ % integer_Part_چوڑائی;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  بنانشانی outputDigits = (قدر_بٹ - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  إذا (hexDigits) {
    إذا (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      بنانشانی صحيح بٹ;
      گواچا_Fraction fraction;

      بٹ = قدر_بٹ - hexDigits * 4;
      fraction = گواچا_Fraction_Through_Truncation (significand, parts_گنتی, بٹ);
      گیڑا_اتے = گیڑا_Away_From_Zero(rounding_منوال_2, fraction, بٹ);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  حرف *p = ++dst;

  بنانشانی گنتی = (قدر_بٹ + integer_Part_چوڑائی - 1) / integer_Part_چوڑائی;

  طالما (outputDigits && گنتی) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    إذا (--گنتی == parts_گنتی)
      part = 0;  /* An imaginary higher zero part.  */
    وإلا
      part = significand[گنتی] << کھسکاؤ;

    إذا (گنتی && کھسکاؤ)
      part |= significand[گنتی - 1] >> (integer_Part_چوڑائی - کھسکاؤ);

    /* Convert as much of "part" to hexdigits as we can.  */
    بنانشانی صحيح curDigits = integer_Part_چوڑائی / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  إذا (گیڑا_اتے) {
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
  *dst++ = اتلا_Case ? 'P': 'p';

  أرجع writeSignedDecimal (dst, exponent);
}

hash_کوڈ hash_قدر(ثابت IEEEFloat &Arg) {
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
                        Arg.significandParts() + Arg.part_گنتی()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convert_F80_Long_Double_APFloat_منزل_APInt() ثابت {
  أكد(part_گنتی() == 2);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semX87DoubleExtended>();
}

APInt IEEEFloat::convert_PPCDouble_Double_Legacy_APFloat_منزل_APInt() ثابت {
  أكد(semantics ==
         (ثابت llvm::fltSemantics *)&APFloat_ادھار::semPPCDoubleDoubleLegacy);
  أكد(part_گنتی()==2);

  uint64_t الفاظ[2];
  منطقي losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloat_ادھار::semIEEEdouble.minExponent;
  IEEEFloat extended(*هذا);
  [[maybe_بن_ورتیا]] op_حالت fs =
      extended.convert(extendedSemantics, rm_سبھ_توں_نیڑے_Ties_منزل_Even, &losesInfo);
  أكد(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloat_ادھار::semIEEEdouble, rm_سبھ_توں_نیڑے_Ties_منزل_Even, &losesInfo);
  أكد(fs == opOK || fs == opInexact);
  الفاظ[0] = *u.convert_Double_APFloat_منزل_APInt().get_Raw_انکڑے();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  إذا (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rm_سبھ_توں_نیڑے_Ties_منزل_Even, &losesInfo);
    أكد(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.گھٹاؤ(u, rm_سبھ_توں_نیڑے_Ties_منزل_Even);
    fs = v.convert(APFloat_ادھار::semIEEEdouble, rm_سبھ_توں_نیڑے_Ties_منزل_Even, &losesInfo);
    أكد(fs == opOK && !losesInfo);
    الفاظ[1] = *v.convert_Double_APFloat_منزل_APInt().get_Raw_انکڑے();
  } وإلا {
    الفاظ[1] = 0;
  }

  أرجع APInt(128, الفاظ);
}

سانچا <ثابت fltSemantics &S>
APInt IEEEFloat::convert_IEEEFloat_منزل_APInt() ثابت {
  أكد(semantics == &S);
  constexpr بنانشانی صحيح trailing_significand_بٹ =
      S.precision - 1 + S.has_Explicit_Integer_ثنائی_ہندسہ;
  constexpr صحيح integer_ثنائی_ہندسہ_part = (S.precision - 1) / integer_Part_چوڑائی;
  constexpr integerPart integer_ثنائی_ہندسہ = integerPart{1}
                                      << ((S.precision - 1) % integer_Part_چوڑائی);
  constexpr uint64_t significand_نقاب = integer_ثنائی_ہندسہ - 1;
  constexpr بنانشانی صحيح exponent_بٹ =
      S.ناپ_وچ_بٹ - (S.hasSignedRepr ? 1 : 0) - trailing_significand_بٹ;
  static_assert(exponent_بٹ < 64);
  constexpr uint64_t exponent_نقاب = (uint64_t{1} << exponent_بٹ) - 1;
  constexpr منطقي is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr صحيح bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_گنتی_For_بٹ(trailing_significand_بٹ)>
      mysignificand;

  إذا (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.ناپ_2(),
                mysignificand.begin());
    إذا (myexponent == 1 &&
        !(significandParts()[integer_ثنائی_ہندسہ_part] & integer_ثنائی_ہندسہ))
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
    إذا constexpr (S.has_Explicit_Integer_ثنائی_ہندسہ) {
      mysignificand[0] = integerPart{1} << (trailing_significand_بٹ - 1);
    }
  } وإلا {
    أكد(category == fcNaN && "Unknown category!");
    إذا (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.ناپ_2(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.ناپ_وچ_بٹ + 63) / 64> الفاظ;
  خودکار الفاظ_iter =
      std::copy_n(mysignificand.begin(), mysignificand.ناپ_2(), الفاظ.begin());
  إذا constexpr (!S.has_Explicit_Integer_ثنائی_ہندسہ) {
    إذا constexpr (significand_نقاب != 0 || trailing_significand_بٹ == 0) {
      // Clear the integer bit.
      الفاظ[mysignificand.ناپ_2() - 1] &= significand_نقاب;
    }
  }
  std::fill(الفاظ_iter, الفاظ.سرا(), uint64_t{0});
  constexpr size_t آخری_لفظ = الفاظ.ناپ_2() - 1;
  إذا constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.ناپ_وچ_بٹ - 1) % 64);
    الفاظ[آخری_لفظ] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_نقاب)
                              << (trailing_significand_بٹ % 64);
  الفاظ[آخری_لفظ] |= shifted_exponent;
  إذا constexpr (آخری_لفظ == 0) {
    أرجع APInt(S.ناپ_وچ_بٹ, الفاظ[0]);
  }
  أرجع APInt(S.ناپ_وچ_بٹ, الفاظ);
}

APInt IEEEFloat::convert_Quadruple_APFloat_منزل_APInt() ثابت {
  أكد(part_گنتی() == 2);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semIEEEquad>();
}

APInt IEEEFloat::convert_Double_APFloat_منزل_APInt() ثابت {
  أكد(part_گنتی()==1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semIEEEdouble>();
}

APInt IEEEFloat::convert_Float_APFloat_منزل_APInt() ثابت {
  أكد(part_گنتی()==1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semIEEEsingle>();
}

APInt IEEEFloat::convert_BFloat_APFloat_منزل_APInt() ثابت {
  أكد(part_گنتی() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semBFloat>();
}

APInt IEEEFloat::convert_Half_APFloat_منزل_APInt() ثابت {
  أكد(part_گنتی()==1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::APFloat_ادھار::semIEEEhalf>();
}

APInt IEEEFloat::convert_Float8_E5_M2_APFloat_منزل_APInt() ثابت {
  أكد(part_گنتی() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semFloat8E5M2>();
}

APInt IEEEFloat::convert_Float8_E5_M2_FNUZAPFloat_منزل_APInt() ثابت {
  أكد(part_گنتی() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convert_Float8_E4_M3_APFloat_منزل_APInt() ثابت {
  أكد(part_گنتی() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semFloat8E4M3>();
}

APInt IEEEFloat::convert_Float8_E4_M3_FNAPFloat_منزل_APInt() ثابت {
  أكد(part_گنتی() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semFloat8E4M3FN>();
}

APInt IEEEFloat::convert_Float8_E4_M3_FNUZAPFloat_منزل_APInt() ثابت {
  أكد(part_گنتی() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convert_Float8_E4_M3_B11_FNUZAPFloat_منزل_APInt() ثابت {
  أكد(part_گنتی() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convert_Float8_E3_M4_APFloat_منزل_APInt() ثابت {
  أكد(part_گنتی() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semFloat8E3M4>();
}

APInt IEEEFloat::convert_Float_TF32_APFloat_منزل_APInt() ثابت {
  أكد(part_گنتی() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semFloatTF32>();
}

APInt IEEEFloat::convert_Float8_E8_M0_FNUAPFloat_منزل_APInt() ثابت {
  أكد(part_گنتی() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convert_Float8_E5_M3_FNUAPFloat_منزل_APInt() ثابت {
  أكد(part_گنتی() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convert_Float6_E3_M2_FNAPFloat_منزل_APInt() ثابت {
  أكد(part_گنتی() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semFloat6E3M2FN>();
}

APInt IEEEFloat::convert_Float6_E2_M3_FNAPFloat_منزل_APInt() ثابت {
  أكد(part_گنتی() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semFloat6E2M3FN>();
}

APInt IEEEFloat::convert_Float4_E2_M1_FNAPFloat_منزل_APInt() ثابت {
  أكد(part_گنتی() == 1);
  أرجع convert_IEEEFloat_منزل_APInt<APFloat_ادھار::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcast_منزل_APInt() ثابت {
  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semIEEEhalf)
    أرجع convert_Half_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semBFloat)
    أرجع convert_BFloat_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semIEEEsingle)
    أرجع convert_Float_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semIEEEdouble)
    أرجع convert_Double_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semIEEEquad)
    أرجع convert_Quadruple_APFloat_منزل_APInt();

  إذا (semantics ==
      (ثابت llvm::fltSemantics *)&APFloat_ادھار::semX87DoubleExtended)
    أرجع convert_F80_Long_Double_APFloat_منزل_APInt();

  إذا (semantics ==
      (ثابت llvm::fltSemantics *)&APFloat_ادھار::semPPCDoubleDoubleLegacy)
    أرجع convert_PPCDouble_Double_Legacy_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semFloat8E5M2)
    أرجع convert_Float8_E5_M2_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semFloat8E5M2FNUZ)
    أرجع convert_Float8_E5_M2_FNUZAPFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semFloat8E4M3)
    أرجع convert_Float8_E4_M3_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semFloat8E4M3FN)
    أرجع convert_Float8_E4_M3_FNAPFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semFloat8E4M3FNUZ)
    أرجع convert_Float8_E4_M3_FNUZAPFloat_منزل_APInt();

  إذا (semantics ==
      (ثابت llvm::fltSemantics *)&APFloat_ادھار::semFloat8E4M3B11FNUZ)
    أرجع convert_Float8_E4_M3_B11_FNUZAPFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semFloat8E3M4)
    أرجع convert_Float8_E3_M4_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semFloatTF32)
    أرجع convert_Float_TF32_APFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semFloat8E8M0FNU)
    أرجع convert_Float8_E8_M0_FNUAPFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semFloat8E5M3FNU)
    أرجع convert_Float8_E5_M3_FNUAPFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semFloat6E3M2FN)
    أرجع convert_Float6_E3_M2_FNAPFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semFloat6E2M3FN)
    أرجع convert_Float6_E2_M3_FNAPFloat_منزل_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semFloat4E2M1FN)
    أرجع convert_Float4_E2_M1_FNAPFloat_منزل_APInt();

  llvm_unreachable("unknown format!");
}

عائم IEEEFloat::convert_منزل_Float() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcast_منزل_APInt();
  أرجع api.بٹ_منزل_Float();
}

مزدوج IEEEFloat::convert_منزل_Double() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcast_منزل_APInt();
  أرجع api.بٹ_منزل_Double();
}

#جے_تعریف_شدہ HAS_IEE754_FLOAT128
float128 IEEEFloat::convert_منزل_Quad() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloat_ادھار::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcast_منزل_APInt();
  أرجع api.بٹ_منزل_Quad();
}
#جے_انت

فراغ IEEEFloat::initFromF80LongDoubleAPInt(ثابت APInt &api) {
  أرجع initFromIEEEAPInt<APFloat_ادھار::semX87DoubleExtended>(api);
}

فراغ IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(ثابت APInt &api) {
  uint64_t i1 = api.get_Raw_انکڑے()[0];
  uint64_t i2 = api.get_Raw_انکڑے()[1];
  منطقي losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_بن_ورتیا]] op_حالت fs = convert(APFloat_ادھار::semPPCDoubleDoubleLegacy,
                                         rm_سبھ_توں_نیڑے_Ties_منزل_Even, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  أكد((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  إذا (isFiniteNonZero()) {
    IEEEFloat v(APFloat_ادھار::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    إذا (v.isFinite()) {
      fs = v.convert(APFloat_ادھار::semPPCDoubleDoubleLegacy, rm_سبھ_توں_نیڑے_Ties_منزل_Even,
                     &losesInfo);
      أكد(fs == opOK && !losesInfo);

      جوڑو(v, rm_سبھ_توں_نیڑے_Ties_منزل_Even);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
فراغ IEEEFloat::initFromFloat8E8M0FNUAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semFloat8E8M0FNU>(api);
}

فراغ IEEEFloat::initFromFloat8E5M3FNUAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semFloat8E5M3FNU>(api);
}

سانچا <ثابت fltSemantics &S>
فراغ IEEEFloat::initFromIEEEAPInt(ثابت APInt &api) {
  أكد(api.get_ثنائی_ہندسہ_چوڑائی() == S.ناپ_وچ_بٹ);

  constexpr بنانشانی صحيح trailing_significand_بٹ =
      S.precision - 1 + S.has_Explicit_Integer_ثنائی_ہندسہ;
  constexpr integerPart integer_ثنائی_ہندسہ =
      integerPart{1} << (trailing_significand_بٹ % integer_Part_چوڑائی);
  constexpr uint64_t significand_نقاب = integer_ثنائی_ہندسہ - 1;
  constexpr بنانشانی صحيح exponent_بٹ =
      S.ناپ_وچ_بٹ - (S.hasSignedRepr ? 1 : 0) - trailing_significand_بٹ;
  static_assert(exponent_بٹ < 64);
  constexpr بنانشانی صحيح stored_significand_parts =
      part_گنتی_For_بٹ(trailing_significand_بٹ + 1);
  constexpr uint64_t exponent_نقاب = (uint64_t{1} << exponent_بٹ) - 1;
  constexpr منطقي is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr صحيح bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr منطقي has_significand = trailing_significand_بٹ > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  إذا constexpr (has_significand) {
    std::copy_n(api.get_Raw_انکڑے(), mysignificand.ناپ_2(), mysignificand.begin());
    إذا constexpr (significand_نقاب != 0 || S.precision >= integer_Part_چوڑائی) {
      mysignificand[mysignificand.ناپ_2() - 1] &= significand_نقاب;
    }
  } وإلا {
    std::fill_n(mysignificand.begin(), mysignificand.ناپ_2(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t آخری_لفظ = api.get_Raw_انکڑے()[api.get_عدد_الفاظ() - 1];
  uint64_t myexponent =
      (آخری_لفظ >> (trailing_significand_بٹ % 64)) & exponent_نقاب;

  initialize(&S);
  أكد(part_گنتی() == mysignificand.ناپ_2());

  sign = S.hasSignedRepr
             ? static_cast<بنانشانی صحيح>(آخری_لفظ >> ((S.ناپ_وچ_بٹ - 1) % 64))
             : 0;

  منطقي all_zero_significand =
      has_significand && llvm::all_دا(mysignificand, equal_منزل(0));

  منطقي is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  إذا constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    منطقي is_inf = خطأ;

    إذا constexpr (S.has_Explicit_Integer_ثنائی_ہندسہ) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_نقاب_no_int_ثنائی_ہندسہ =
          (uint64_t{1} << (trailing_significand_بٹ - 1)) - 1;
      ثابت integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_بٹ - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_نقاب_no_int_ثنائی_ہندسہ) == 0;
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
    إذا constexpr (S.has_Explicit_Integer_ثنائی_ہندسہ) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      ثابت integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_بٹ - 1);
      constexpr integerPart significand_نقاب_no_int_ثنائی_ہندسہ =
          (uint64_t{1} << (trailing_significand_بٹ - 1)) - 1;

      إذا (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_نقاب_no_int_ثنائی_ہندسہ) != 0) {
        // regular NaN and pseudoNaN
        is_nan = سچ;
      } وإلا إذا (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_نقاب_no_int_ثنائی_ہندسہ) == 0) {
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
        std::all_دا(mysignificand.begin(), mysignificand.سرا() - 1,
                    [](integerPart بٹ) { أرجع بٹ == ~integerPart{0}; }) &&
        (!significand_نقاب ||
         mysignificand[mysignificand.ناپ_2() - 1] == significand_نقاب);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } وإلا إذا constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  إذا (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.ناپ_2(),
                significandParts());
    أرجع;
  }

  إذا (is_zero) {
    makeZero(sign);
    أرجع;
  }

  category = fc_عام;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.ناپ_2(), significandParts());
  إذا (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  وإلا {
    إذا constexpr (!S.has_Explicit_Integer_ثنائی_ہندسہ) {
      significandParts()[mysignificand.ناپ_2() - 1] |= integer_ثنائی_ہندسہ;
    }
  }
}

فراغ IEEEFloat::initFromQuadrupleAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semIEEEquad>(api);
}

فراغ IEEEFloat::initFromDoubleAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semIEEEdouble>(api);
}

فراغ IEEEFloat::initFromFloatAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semIEEEsingle>(api);
}

فراغ IEEEFloat::initFromBFloatAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semBFloat>(api);
}

فراغ IEEEFloat::initFromHalfAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semIEEEhalf>(api);
}

فراغ IEEEFloat::initFromFloat8E5M2APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semFloat8E5M2>(api);
}

فراغ IEEEFloat::initFromFloat8E5M2FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semFloat8E5M2FNUZ>(api);
}

فراغ IEEEFloat::initFromFloat8E4M3APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semFloat8E4M3>(api);
}

فراغ IEEEFloat::initFromFloat8E4M3FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semFloat8E4M3FN>(api);
}

فراغ IEEEFloat::initFromFloat8E4M3FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semFloat8E4M3FNUZ>(api);
}

فراغ IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semFloat8E4M3B11FNUZ>(api);
}

فراغ IEEEFloat::initFromFloat8E3M4APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semFloat8E3M4>(api);
}

فراغ IEEEFloat::initFromFloatTF32APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semFloatTF32>(api);
}

فراغ IEEEFloat::initFromFloat6E3M2FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semFloat6E3M2FN>(api);
}

فراغ IEEEFloat::initFromFloat6E2M3FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semFloat6E2M3FN>(api);
}

فراغ IEEEFloat::initFromFloat4E2M1FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_ادھار::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
فراغ IEEEFloat::initFromAPInt(ثابت fltSemantics *Sem, ثابت APInt &api) {
  أكد(api.get_ثنائی_ہندسہ_چوڑائی() == Sem->ناپ_وچ_بٹ);
  إذا (Sem == &APFloat_ادھار::semIEEEhalf)
    أرجع initFromHalfAPInt(api);
  إذا (Sem == &APFloat_ادھار::semBFloat)
    أرجع initFromBFloatAPInt(api);
  إذا (Sem == &APFloat_ادھار::semIEEEsingle)
    أرجع initFromFloatAPInt(api);
  إذا (Sem == &APFloat_ادھار::semIEEEdouble)
    أرجع initFromDoubleAPInt(api);
  إذا (Sem == &APFloat_ادھار::semIEEEquad)
    أرجع initFromQuadrupleAPInt(api);
  إذا (Sem == &APFloat_ادھار::semX87DoubleExtended)
    أرجع initFromF80LongDoubleAPInt(api);
  إذا (Sem == &APFloat_ادھار::semPPCDoubleDoubleLegacy)
    أرجع initFromPPCDoubleDoubleLegacyAPInt(api);
  إذا (Sem == &APFloat_ادھار::semFloat8E5M2)
    أرجع initFromFloat8E5M2APInt(api);
  إذا (Sem == &APFloat_ادھار::semFloat8E5M2FNUZ)
    أرجع initFromFloat8E5M2FNUZAPInt(api);
  إذا (Sem == &APFloat_ادھار::semFloat8E4M3)
    أرجع initFromFloat8E4M3APInt(api);
  إذا (Sem == &APFloat_ادھار::semFloat8E4M3FN)
    أرجع initFromFloat8E4M3FNAPInt(api);
  إذا (Sem == &APFloat_ادھار::semFloat8E4M3FNUZ)
    أرجع initFromFloat8E4M3FNUZAPInt(api);
  إذا (Sem == &APFloat_ادھار::semFloat8E4M3B11FNUZ)
    أرجع initFromFloat8E4M3B11FNUZAPInt(api);
  إذا (Sem == &APFloat_ادھار::semFloat8E3M4)
    أرجع initFromFloat8E3M4APInt(api);
  إذا (Sem == &APFloat_ادھار::semFloatTF32)
    أرجع initFromFloatTF32APInt(api);
  إذا (Sem == &APFloat_ادھار::semFloat8E8M0FNU)
    أرجع initFromFloat8E8M0FNUAPInt(api);
  إذا (Sem == &APFloat_ادھار::semFloat8E5M3FNU)
    أرجع initFromFloat8E5M3FNUAPInt(api);
  إذا (Sem == &APFloat_ادھار::semFloat6E3M2FN)
    أرجع initFromFloat6E3M2FNAPInt(api);
  إذا (Sem == &APFloat_ادھار::semFloat6E2M3FN)
    أرجع initFromFloat6E2M3FNAPInt(api);
  إذا (Sem == &APFloat_ادھار::semFloat4E2M1FN)
    أرجع initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
فراغ IEEEFloat::make_سبھ_توں_وڈا(منطقي Negative) {
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
  بنانشانی Part_گنتی = part_گنتی();
  memset(significand, 0xFF, حجم(integerPart)*(Part_گنتی - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  ثابت بنانشانی عدد_بن_ورتیا_بلند_بٹ =
    Part_گنتی*integer_Part_چوڑائی - semantics->precision;
  significand[Part_گنتی - 1] = (عدد_بن_ورتیا_بلند_بٹ < integer_Part_چوڑائی)
                                   ? (~integerPart(0) >> عدد_بن_ورتیا_بلند_بٹ)
                                   : 0;
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
فراغ IEEEFloat::make_سبھ_توں_چھوٹا(منطقي Negative) {
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
  APInt::tc_سیٹ(significandParts(), 1, part_گنتی());
}

فراغ IEEEFloat::make_سبھ_توں_چھوٹا_Normalized(منطقي Negative) {
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
  APInt::tc_سیٹ_ثنائی_ہندسہ(significandParts(), semantics->precision - 1);
}

IEEEFloat::IEEEFloat(ثابت fltSemantics &Sem, ثابت APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(عائم f) {
  initFromAPInt(&APFloat_ادھار::semIEEEsingle, APInt::float_منزل_بٹ(f));
}

IEEEFloat::IEEEFloat(مزدوج d) {
  initFromAPInt(&APFloat_ادھار::semIEEEdouble, APInt::double_منزل_بٹ(d));
}

نطاق {
  فراغ append(Small_سمتیہ_Impl<حرف> &بفر, StringRef Str) {
    بفر.append(Str.begin(), Str.سرا());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  فراغ Adjust_منزل_Precision(APInt &significand,
                         صحيح &قوت_نما, بنانشانی FormatPrecision) {
    بنانشانی بٹ = significand.get_سرگرم_بٹ();

    // 196/59 is a very slight overestimate of lg_2(10).
    بنانشانی بٹ_Required = (FormatPrecision * 196 + 58) / 59;

    إذا (بٹ <= بٹ_Required) أرجع;

    بنانشانی tensRemovable = (بٹ - بٹ_Required) * 59 / 196;
    إذا (!tensRemovable) أرجع;

    قوت_نما += tensRemovable;

    APInt مقسوم_علیہ(significand.get_ثنائی_ہندسہ_چوڑائی(), 1);
    APInt powten(significand.get_ثنائی_ہندسہ_چوڑائی(), 10);
    طالما (سچ) {
      إذا (tensRemovable & 1)
        مقسوم_علیہ *= powten;
      tensRemovable >>= 1;
      إذا (!tensRemovable) اكسر;
      powten *= powten;
    }

    significand = significand.udiv(مقسوم_علیہ);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.get_سرگرم_بٹ());
  }


  فراغ Adjust_منزل_Precision(Small_سمتیہ_Impl<حرف> &بفر_2,
                         صحيح &قوت_نما, بنانشانی FormatPrecision) {
    بنانشانی N = بفر_2.ناپ_2();
    إذا (N <= FormatPrecision) أرجع;

    // The most significant figures are the last ones in the buffer.
    بنانشانی پہلا_Significant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    إذا (بفر_2[پہلا_Significant - 1] < '5') {
      طالما (پہلا_Significant < N && بفر_2[پہلا_Significant] == '0')
        پہلا_Significant++;

      قوت_نما += پہلا_Significant;
      بفر_2.erase(&بفر_2[0], &بفر_2[پہلا_Significant]);
      أرجع;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    لكل (بنانشانی I = پہلا_Significant; I != N; ++I) {
      إذا (بفر_2[I] == '9') {
        پہلا_Significant++;
      } وإلا {
        بفر_2[I]++;
        اكسر;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    إذا (پہلا_Significant == N) {
      قوت_نما += پہلا_Significant;
      بفر_2.clear();
      بفر_2.ڈالو_پٹھ('1');
      أرجع;
    }

    قوت_نما += پہلا_Significant;
    بفر_2.erase(&بفر_2[0], &بفر_2[پہلا_Significant]);
  }

  فراغ منزل_String_Impl(Small_سمتیہ_Impl<حرف> &Str, ثابت منطقي isNeg, صحيح قوت_نما,
                    APInt significand, بنانشانی FormatPrecision,
                    بنانشانی FormatMaxPadding, منطقي TruncateZero) {
    ثابت صحيح semanticsPrecision = significand.get_ثنائی_ہندسہ_چوڑائی();

    إذا (isNeg)
      Str.ڈالو_پٹھ('-');

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
    significand.lshr_وچ_رکھو(trailingZeros);

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

      بنانشانی precision = semanticsPrecision + (137 * texp + 136) / 59;

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

    Small_سمتیہ<حرف, 256> بفر_2;

    // Fill the buffer.
    بنانشانی precision = significand.get_ثنائی_ہندسہ_چوڑائی();
    إذا (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    منطقي وچ_Trail = سچ;
    طالما (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      بنانشانی d = digit.get_ZExt_قدر();

      // Drop trailing zeros.
      إذا (وچ_Trail && !d)
        قوت_نما++;
      وإلا {
        بفر_2.ڈالو_پٹھ((حرف) ('0' + d));
        وچ_Trail = خطأ;
      }
    }

    أكد(!بفر_2.خالی_تھاواں() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    Adjust_منزل_Precision(بفر_2, قوت_نما, FormatPrecision);

    بنانشانی NDigits = بفر_2.ناپ_2();

    // Check whether we should use scientific notation.
    منطقي FormatScientific;
    إذا (!FormatMaxPadding) {
      FormatScientific = سچ;
    } وإلا {
      إذا (قوت_نما >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((بنانشانی) قوت_نما > FormatMaxPadding ||
                            NDigits + (بنانشانی) قوت_نما > FormatPrecision);
      } وإلا {
        // Power of the most significant digit.
        صحيح MSD = قوت_نما + (صحيح) (NDigits - 1);
        إذا (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = خطأ;
        } وإلا {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((بنانشانی) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    إذا (FormatScientific) {
      قوت_نما += (NDigits - 1);

      Str.ڈالو_پٹھ(بفر_2[NDigits-1]);
      Str.ڈالو_پٹھ('.');
      إذا (NDigits == 1 && TruncateZero)
        Str.ڈالو_پٹھ('0');
      وإلا
        لكل (بنانشانی I = 1; I != NDigits; ++I)
          Str.ڈالو_پٹھ(بفر_2[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      إذا (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.ڈالو_پٹھ(TruncateZero ? 'E' : 'e');

      Str.ڈالو_پٹھ(قوت_نما >= 0 ? '+' : '-');
      إذا (قوت_نما < 0)
        قوت_نما = -قوت_نما;
      Small_سمتیہ<حرف, 6> expbuf;
      افعل {
        expbuf.ڈالو_پٹھ((حرف) ('0' + (قوت_نما % 10)));
        قوت_نما /= 10;
      } طالما (قوت_نما);
      // Exponent always at least two digits if we do not truncate zeros.
      إذا (!TruncateZero && expbuf.ناپ_2() < 2)
        expbuf.ڈالو_پٹھ('0');
      لكل (بنانشانی I = 0, E = expbuf.ناپ_2(); I != E; ++I)
        Str.ڈالو_پٹھ(expbuf[E-1-I]);
      أرجع;
    }

    // Non-scientific, positive exponents.
    إذا (قوت_نما >= 0) {
      لكل (بنانشانی I = 0; I != NDigits; ++I)
        Str.ڈالو_پٹھ(بفر_2[NDigits-1-I]);
      لكل (بنانشانی I = 0; I != (بنانشانی) قوت_نما; ++I)
        Str.ڈالو_پٹھ('0');
      أرجع;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    صحيح NWholeDigits = قوت_نما + (صحيح) NDigits;

    بنانشانی I = 0;
    إذا (NWholeDigits > 0) {
      لكل (; I != (بنانشانی) NWholeDigits; ++I)
        Str.ڈالو_پٹھ(بفر_2[NDigits-I-1]);
      Str.ڈالو_پٹھ('.');
    } وإلا {
      بنانشانی NZeros = 1 + (بنانشانی) -NWholeDigits;

      Str.ڈالو_پٹھ('0');
      Str.ڈالو_پٹھ('.');
      لكل (بنانشانی Z = 1; Z != NZeros; ++Z)
        Str.ڈالو_پٹھ('0');
    }

    لكل (; I != NDigits; ++I)
      Str.ڈالو_پٹھ(بفر_2[NDigits-I-1]);

  }
} // namespace

فراغ IEEEFloat::منزل_String(Small_سمتیہ_Impl<حرف> &Str, بنانشانی FormatPrecision,
                         بنانشانی FormatMaxPadding, منطقي TruncateZero) ثابت {
  بدل (category) {
  حالة fcInfinity:
    إذا (isNegative())
      أرجع append(Str, "-Inf");
    وإلا
      أرجع append(Str, "+Inf");

  حالة fcNaN: أرجع append(Str, "NaN");

  حالة fcZero:
    إذا (isNegative())
      Str.ڈالو_پٹھ('-');

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
      Str.ڈالو_پٹھ('0');
    }
    أرجع;

  حالة fc_عام:
    اكسر;
  }

  // Decompose the number into an APInt and an exponent.
  صحيح قوت_نما = exponent - ((صحيح) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_گنتی_For_بٹ(semantics->precision)));

  منزل_String_Impl(Str, isNegative(), قوت_نما, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

صحيح IEEEFloat::getExactLog2Abs() ثابت {
  إذا (!isFinite() || isZero())
    أرجع INT_MIN;

  ثابت integerPart *Parts = significandParts();
  ثابت صحيح Part_گنتی = part_گنتی_For_بٹ(semantics->precision);

  صحيح نکالو_گنتی = 0;
  لكل (صحيح i = 0; i < Part_گنتی; ++i) {
    نکالو_گنتی += llvm::popcount(Parts[i]);
    إذا (نکالو_گنتی > 1)
      أرجع INT_MIN;
  }

  إذا (exponent != semantics->minExponent)
    أرجع exponent;

  صحيح CountrParts = 0;
  لكل (صحيح i = 0; i < Part_گنتی;
       ++i, CountrParts += APInt::APINT_بٹ_فی_لفظ) {
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
  أرجع !APInt::tc_کڈھو_ثنائی_ہندسہ(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::op_حالت IEEEFloat::اگلا(منطقي اگلا_تھلے) {
  // If we are performing nextDown, swap sign so we have -x.
  إذا (اگلا_تھلے)
    changeSign();

  // Compute nextUp(x)
  op_حالت نتیجہ_2 = opOK;

  // Handle each float category separately.
  بدل (category) {
  حالة fcInfinity:
    // nextUp(+inf) = +inf
    إذا (!isNegative())
      اكسر;
    // nextUp(-inf) = -getLargest()
    make_سبھ_توں_وڈا(سچ);
    اكسر;
  حالة fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    إذا (isSignaling()) {
      نتیجہ_2 = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(خطأ, isNegative(), خالی_سوچک);
    }
    اكسر;
  حالة fcZero:
    // nextUp(pm 0) = +getSmallest()
    make_سبھ_توں_چھوٹا(خطأ);
    اكسر;
  حالة fc_عام:
    // nextUp(-getSmallest()) = -0
    إذا (is_سبھ_توں_چھوٹا() && isNegative()) {
      APInt::tc_سیٹ(significandParts(), 0, part_گنتی());
      category = fcZero;
      exponent = 0;
      إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = خطأ;
      إذا (!semantics->hasZero)
        make_سبھ_توں_چھوٹا_Normalized(خطأ);
      اكسر;
    }

    إذا (is_سبھ_توں_وڈا() && !isNegative()) {
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
        APInt::tc_سیٹ(significandParts(), 0, part_گنتی());
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
      APInt::tcDecrement(Parts, part_گنتی());

      إذا (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tc_سیٹ_ثنائی_ہندسہ(Parts, semantics->precision - 1);
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
        APInt::tc_سیٹ(Parts, 0, part_گنتی());
        APInt::tc_سیٹ_ثنائی_ہندسہ(Parts, semantics->precision - 1);
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
  إذا (اگلا_تھلے)
    changeSign();

  أرجع نتیجہ_2;
}

APInt IEEEFloat::getNaNPayload() ثابت {
  أكد(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  بنانشانی بٹ_2 = semantics->precision - 1;
  أرجع APInt(بٹ_2, ArrayRef(significandParts(), part_گنتی_For_بٹ(بٹ_2)));
}

APFloat_ادھار::ExponentType IEEEFloat::exponentNaN() ثابت {
  أرجع ::exponentNaN(*semantics);
}

APFloat_ادھار::ExponentType IEEEFloat::exponentInf() ثابت {
  أرجع ::exponentInf(*semantics);
}

APFloat_ادھار::ExponentType IEEEFloat::exponentZero() ثابت {
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
  APInt::tc_سیٹ(significandParts(), 0, part_گنتی());
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
  APInt::tc_سیٹ(significandParts(), 0, part_گنتی());
}

فراغ IEEEFloat::makeQuiet() {
  أكد(isNaN());
  إذا (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tc_سیٹ_ثنائی_ہندسہ(significandParts(), semantics->precision - 2);
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
  صحيح Significand_بٹ = Arg.getSemantics().precision - 1;

  Normalized.exponent += Significand_بٹ;
  Normalized.normalize(APFloat::rm_سبھ_توں_نیڑے_Ties_منزل_Even, lfExactlyZero);
  أرجع Normalized.exponent - Significand_بٹ;
}

IEEEFloat scalbn(IEEEFloat X, صحيح Exp, rounding_منوال Rounding_منوال) {
  خودکار MaxExp = X.getSemantics().maxExponent;
  خودکار MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  صحيح Significand_بٹ = X.getSemantics().precision - 1;
  صحيح MaxIncrement = MaxExp - (MinExp - Significand_بٹ) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(Rounding_منوال, lfExactlyZero);
  إذا (X.isNaN())
    X.makeQuiet();
  أرجع X;
}

IEEEFloat frexp(ثابت IEEEFloat &Val, صحيح &Exp, rounding_منوال RM) {
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
      Floats(جديد APFloat[2]{APFloat(APFloat_ادھار::semIEEEdouble),
                            APFloat(APFloat_ادھار::semIEEEdouble)}) {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, uninitialized_ٹیگ)
    : Semantics(&S), Floats(جديد APFloat[2]{
                         APFloat(APFloat_ادھار::semIEEEdouble, uninitialized),
                         APFloat(APFloat_ادھار::semIEEEdouble, uninitialized)}) {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(جديد APFloat[2]{APFloat(APFloat_ادھار::semIEEEdouble, I),
                            APFloat(APFloat_ادھار::semIEEEdouble)}) {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, ثابت APInt &I)
    : Semantics(&S),
      Floats(جديد APFloat[2]{
          APFloat(APFloat_ادھار::semIEEEdouble, APInt(64, I.get_Raw_انکڑے()[0])),
          APFloat(APFloat_ادھار::semIEEEdouble, APInt(64, I.get_Raw_انکڑے()[1]))}) {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, APFloat &&پہلا,
                             APFloat &&دوجا)
    : Semantics(&S),
      Floats(جديد APFloat[2]{std::move(پہلا), std::move(دوجا)}) {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble);
  أكد(&Floats[0].getSemantics() == &APFloat_ادھار::semIEEEdouble);
  أكد(&Floats[1].getSemantics() == &APFloat_ادھار::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? جديد APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : خالی_سوچک) {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloat_ادھار::semBogus;
  RHS.Floats = خالی_سوچک;
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble);
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
ساكن std::pair<APFloat, APFloat> fast_Two_جوڑ(APFloat X, APFloat Y) {
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
APFloat::op_حالت DoubleAPFloat::جوڑو_Impl(ثابت APFloat &a, ثابت APFloat &aa,
                                         ثابت APFloat &c, ثابت APFloat &cc,
                                         rounding_منوال RM) {
  صحيح حالت = opOK;
  APFloat z = a;
  حالت |= z.جوڑو(c, RM);
  إذا (!z.isFinite()) {
    إذا (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ خطأ);
      أرجع (op_حالت)حالت;
    }
    حالت = opOK;
    خودکار ACompared_منزل_C = a.compare_Absolute_قدر(c);
    z = cc;
    حالت |= z.جوڑو(aa, RM);
    إذا (ACompared_منزل_C == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      حالت |= z.جوڑو(c, RM);
      حالت |= z.جوڑو(a, RM);
    } وإلا {
      // z = cc + aa + a + c;
      حالت |= z.جوڑو(a, RM);
      حالت |= z.جوڑو(c, RM);
    }
    إذا (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ خطأ);
      أرجع (op_حالت)حالت;
    }
    Floats[0] = z;
    APFloat zz = aa;
    حالت |= zz.جوڑو(cc, RM);
    إذا (ACompared_منزل_C == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      حالت |= Floats[1].گھٹاؤ(z, RM);
      حالت |= Floats[1].جوڑو(c, RM);
      حالت |= Floats[1].جوڑو(zz, RM);
    } وإلا {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      حالت |= Floats[1].گھٹاؤ(z, RM);
      حالت |= Floats[1].جوڑو(a, RM);
      حالت |= Floats[1].جوڑو(zz, RM);
    }
  } وإلا {
    // q = a - z;
    APFloat q = a;
    حالت |= q.گھٹاؤ(z, RM);

    // zz = q + c + (a - (q + z)) + aa + cc;
    // Compute a - (q + z) as -((q + z) - a) to avoid temporary copies.
    خودکار zz = q;
    حالت |= zz.جوڑو(c, RM);
    حالت |= q.جوڑو(z, RM);
    حالت |= q.گھٹاؤ(a, RM);
    q.changeSign();
    حالت |= zz.جوڑو(q, RM);
    حالت |= zz.جوڑو(aa, RM);
    حالت |= zz.جوڑو(cc, RM);
    إذا (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ خطأ);
      أرجع opOK;
    }
    Floats[0] = z;
    حالت |= Floats[0].جوڑو(zz, RM);
    إذا (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ خطأ);
      أرجع (op_حالت)حالت;
    }
    Floats[1] = std::move(z);
    حالت |= Floats[1].گھٹاؤ(Floats[0], RM);
    حالت |= Floats[1].جوڑو(zz, RM);
  }
  أرجع (op_حالت)حالت;
}

APFloat::op_حالت DoubleAPFloat::جوڑو_With_Special(ثابت DoubleAPFloat &LHS,
                                                ثابت DoubleAPFloat &RHS,
                                                DoubleAPFloat &باہرلا,
                                                rounding_منوال RM) {
  إذا (LHS.getCategory() == fcNaN) {
    باہرلا = LHS;
    أرجع opOK;
  }
  إذا (RHS.getCategory() == fcNaN) {
    باہرلا = RHS;
    أرجع opOK;
  }
  إذا (LHS.getCategory() == fcZero) {
    باہرلا = RHS;
    أرجع opOK;
  }
  إذا (RHS.getCategory() == fcZero) {
    باہرلا = LHS;
    أرجع opOK;
  }
  إذا (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    باہرلا.makeNaN(خطأ, باہرلا.isNegative(), خالی_سوچک);
    أرجع opInvalidOp;
  }
  إذا (LHS.getCategory() == fcInfinity) {
    باہرلا = LHS;
    أرجع opOK;
  }
  إذا (RHS.getCategory() == fcInfinity) {
    باہرلا = RHS;
    أرجع opOK;
  }
  أكد(LHS.getCategory() == fc_عام && RHS.getCategory() == fc_عام);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  أكد(&A.getSemantics() == &APFloat_ادھار::semIEEEdouble);
  أكد(&AA.getSemantics() == &APFloat_ادھار::semIEEEdouble);
  أكد(&C.getSemantics() == &APFloat_ادھار::semIEEEdouble);
  أكد(&CC.getSemantics() == &APFloat_ادھار::semIEEEdouble);
  أكد(&باہرلا.Floats[0].getSemantics() == &APFloat_ادھار::semIEEEdouble);
  أكد(&باہرلا.Floats[1].getSemantics() == &APFloat_ادھار::semIEEEdouble);
  أرجع باہرلا.جوڑو_Impl(A, AA, C, CC, RM);
}

APFloat::op_حالت DoubleAPFloat::جوڑو(ثابت DoubleAPFloat &RHS,
                                     rounding_منوال RM) {
  أرجع جوڑو_With_Special(*هذا, RHS, *هذا, RM);
}

APFloat::op_حالت DoubleAPFloat::گھٹاؤ(ثابت DoubleAPFloat &RHS,
                                          rounding_منوال RM) {
  changeSign();
  خودکار Ret = جوڑو(RHS, RM);
  changeSign();
  أرجع Ret;
}

APFloat::op_حالت DoubleAPFloat::multiply(ثابت DoubleAPFloat &RHS,
                                          APFloat::rounding_منوال RM) {
  ثابت خودکار &LHS = *هذا;
  خودکار &باہرلا = *هذا;
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
    باہرلا = LHS;
    أرجع opOK;
  }
  إذا (RHS.getCategory() == fcNaN) {
    باہرلا = RHS;
    أرجع opOK;
  }
  إذا ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    باہرلا.makeNaN(خطأ, خطأ, خالی_سوچک);
    أرجع opOK;
  }
  إذا (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    باہرلا = LHS;
    أرجع opOK;
  }
  إذا (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    باہرلا = RHS;
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
  حالت |= Tau.fused_Multiply_جوڑو(C, T, RM);
  T.changeSign();
  {
    // v = a * d
    APFloat V = A;
    حالت |= V.multiply(D, RM);
    // w = b * c
    APFloat W = B;
    حالت |= W.multiply(C, RM);
    حالت |= V.جوڑو(W, RM);
    // tau += v + w
    حالت |= Tau.جوڑو(V, RM);
  }
  // u = t + tau
  APFloat U = T;
  حالت |= U.جوڑو(Tau, RM);

  Floats[0] = U;
  إذا (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ خطأ);
  } وإلا {
    // Floats[1] = (t - u) + tau
    حالت |= T.گھٹاؤ(U, RM);
    حالت |= T.جوڑو(Tau, RM);
    Floats[1] = std::move(T);
  }
  أرجع (op_حالت)حالت;
}

APFloat::op_حالت DoubleAPFloat::divide(ثابت DoubleAPFloat &RHS,
                                        APFloat::rounding_منوال RM) {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_ادھار::semPPCDoubleDoubleLegacy, bitcast_منزل_APInt());
  خودکار Ret = Tmp.divide(
      APFloat(APFloat_ادھار::semPPCDoubleDoubleLegacy, RHS.bitcast_منزل_APInt()), RM);
  *هذا = DoubleAPFloat(APFloat_ادھار::semPPCDoubleDouble, Tmp.bitcast_منزل_APInt());
  أرجع Ret;
}

APFloat::op_حالت DoubleAPFloat::remainder(ثابت DoubleAPFloat &RHS) {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_ادھار::semPPCDoubleDoubleLegacy, bitcast_منزل_APInt());
  خودکار Ret = Tmp.remainder(
      APFloat(APFloat_ادھار::semPPCDoubleDoubleLegacy, RHS.bitcast_منزل_APInt()));
  *هذا = DoubleAPFloat(APFloat_ادھار::semPPCDoubleDouble, Tmp.bitcast_منزل_APInt());
  أرجع Ret;
}

APFloat::op_حالت DoubleAPFloat::mod(ثابت DoubleAPFloat &RHS) {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_ادھار::semPPCDoubleDoubleLegacy, bitcast_منزل_APInt());
  خودکار Ret = Tmp.mod(
      APFloat(APFloat_ادھار::semPPCDoubleDoubleLegacy, RHS.bitcast_منزل_APInt()));
  *هذا = DoubleAPFloat(APFloat_ادھار::semPPCDoubleDouble, Tmp.bitcast_منزل_APInt());
  أرجع Ret;
}

APFloat::op_حالت
DoubleAPFloat::fused_Multiply_جوڑو(ثابت DoubleAPFloat &Multiplicand,
                                ثابت DoubleAPFloat &Addend,
                                APFloat::rounding_منوال RM) {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_ادھار::semPPCDoubleDoubleLegacy, bitcast_منزل_APInt());
  خودکار Ret = Tmp.fused_Multiply_جوڑو(
      APFloat(APFloat_ادھار::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcast_منزل_APInt()),
      APFloat(APFloat_ادھار::semPPCDoubleDoubleLegacy, Addend.bitcast_منزل_APInt()),
      RM);
  *هذا = DoubleAPFloat(APFloat_ادھار::semPPCDoubleDouble, Tmp.bitcast_منزل_APInt());
  أرجع Ret;
}

APFloat::op_حالت DoubleAPFloat::گیڑا_منزل_Integral(APFloat::rounding_منوال RM) {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ثابت APFloat &Hi = get_پہلا();
  ثابت APFloat &Lo = get_دوجا();

  APFloat RoundedHi = Hi;
  ثابت op_حالت Hi_حالت = RoundedHi.گیڑا_منزل_Integral(RM);

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
  خودکار گیڑا_منزل_سبھ_توں_نیڑے_Helper = [](APFloat منزل_گیڑا, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    ثابت APFloat RoundingError = Rounded - منزل_گیڑا;
    إذا (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        مطلق_قدر(RoundingError).is_Exactly_قدر(0.5))
      Rounded.جوڑو(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rm_سبھ_توں_نیڑے_Ties_منزل_Even);
    أرجع Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  إذا (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    إذا (RM == rm_سبھ_توں_نیڑے_Ties_منزل_Away || RM == rm_سبھ_توں_نیڑے_Ties_منزل_Even)
      RoundedHi = گیڑا_منزل_سبھ_توں_نیڑے_Helper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/خطأ);
    أرجع Hi_حالت;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  Rounding_منوال Lo_Rounding_منوال;
  إذا (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    Lo_Rounding_منوال = isNegative() ? rm_Toward_مثبت : rmTowardNegative;
  وإلا
    Lo_Rounding_منوال = RM;

  APFloat RoundedLo = Lo;
  ثابت op_حالت Lo_حالت = RoundedLo.گیڑا_منزل_Integral(Lo_Rounding_منوال);
  إذا (Lo_Rounding_منوال == rm_سبھ_توں_نیڑے_Ties_منزل_Away)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = گیڑا_منزل_سبھ_توں_نیڑے_Helper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fast_Two_جوڑ(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  أرجع Lo_حالت;
}

فراغ DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmp_نتیجہ
DoubleAPFloat::compare_Absolute_قدر(ثابت DoubleAPFloat &RHS) ثابت {
  // Compare absolute values of the high parts.
  ثابت cmp_نتیجہ HiPartCmp = Floats[0].compare_Absolute_قدر(RHS.Floats[0]);
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
  ثابت cmp_نتیجہ LoPartCmp = Floats[1].compare_Absolute_قدر(RHS.Floats[1]);

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

فراغ DoubleAPFloat::make_سبھ_توں_وڈا(منطقي Neg) {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloat_ادھار::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloat_ادھار::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  إذا (Neg)
    changeSign();
}

فراغ DoubleAPFloat::make_سبھ_توں_چھوٹا(منطقي Neg) {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].make_سبھ_توں_چھوٹا(Neg);
  Floats[1].makeZero(/* Neg = */ خطأ);
}

فراغ DoubleAPFloat::make_سبھ_توں_چھوٹا_Normalized(منطقي Neg) {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloat_ادھار::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  إذا (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ خطأ);
}

فراغ DoubleAPFloat::makeNaN(منطقي SNaN, منطقي Neg, ثابت APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ خطأ);
}

APFloat::cmp_نتیجہ DoubleAPFloat::compare(ثابت DoubleAPFloat &RHS) ثابت {
  خودکار نتیجہ = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  إذا (نتیجہ == APFloat::cmpEqual)
    أرجع Floats[1].compare(RHS.Floats[1]);
  أرجع نتیجہ;
}

منطقي DoubleAPFloat::bitwiseIsEqual(ثابت DoubleAPFloat &RHS) ثابت {
  أرجع Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_کوڈ hash_قدر(ثابت DoubleAPFloat &Arg) {
  إذا (Arg.Floats)
    أرجع hash_combine(hash_قدر(Arg.Floats[0]), hash_قدر(Arg.Floats[1]));
  أرجع hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcast_منزل_APInt() ثابت {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t انکڑے_2[] = {
      Floats[0].bitcast_منزل_APInt().get_Raw_انکڑے()[0],
      Floats[1].bitcast_منزل_APInt().get_Raw_انکڑے()[0],
  };
  أرجع APInt(128, انکڑے_2);
}

Expected<APFloat::op_حالت> DoubleAPFloat::convertFromString(StringRef S,
                                                             rounding_منوال RM) {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_ادھار::semPPCDoubleDoubleLegacy);
  خودکار Ret = Tmp.convertFromString(S, RM);
  *هذا = DoubleAPFloat(APFloat_ادھار::semPPCDoubleDouble, Tmp.bitcast_منزل_APInt());
  أرجع Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::op_حالت DoubleAPFloat::اگلا(منطقي اگلا_تھلے) {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  إذا (اگلا_تھلے) {
    changeSign();
    APFloat::op_حالت نتیجہ = اگلا(/*nextDown=*/خطأ);
    changeSign();
    أرجع نتیجہ;
  }
  بدل (getCategory()) {
  حالة fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    إذا (isNegative())
      make_سبھ_توں_وڈا(سچ);
    أرجع opOK;

  حالة fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    إذا (get_پہلا().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(خطأ, isNegative(), خالی_سوچک);
      أرجع opInvalidOp;
    }
    أرجع opOK;

  حالة fcZero:
    // nextUp(pm 0) = +getSmallest()
    make_سبھ_توں_چھوٹا(خطأ);
    أرجع opOK;

  حالة fc_عام:
    اكسر;
  }

  ثابت APFloat &HiOld = get_پہلا();
  ثابت APFloat &LoOld = get_دوجا();

  APFloat اگلا_Lo = LoOld;
  اگلا_Lo.اگلا(/*nextDown=*/خطأ);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  خودکار وچ_Lattice = [](ثابت APFloat &Hi, ثابت APFloat &Lo) {
    أرجع Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  إذا (وچ_Lattice(HiOld, اگلا_Lo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(اگلا_Lo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat سبھ_توں_وڈا{*Semantics, uninitialized};
    سبھ_توں_وڈا.make_سبھ_توں_وڈا(/*Neg=*/خطأ);
    إذا (compare(سبھ_توں_وڈا) == cmpGreaterThan)
      makeInf(/*Neg=*/خطأ);

    أرجع opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat اگلا_Hi = HiOld;
  اگلا_Hi.اگلا(/*nextDown=*/خطأ);

  // nextUp(getLargest()) == INFINITY
  إذا (اگلا_Hi.isInfinity()) {
    makeInf(/*Neg=*/خطأ);
    أرجع opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  إذا (اگلا_Hi.isZero()) {
    makeZero(/*Neg=*/سچ);
    أرجع opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  اگلا_Lo = neg(scalbn(harrisonUlp(اگلا_Hi), -1, rmTowardZero));
  إذا (!وچ_Lattice(اگلا_Hi, اگلا_Lo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    اگلا_Lo.اگلا(/*nextDown=*/خطأ);

  Floats[0] = std::move(اگلا_Hi);
  Floats[1] = std::move(اگلا_Lo);

  أرجع opOK;
}

APFloat::op_حالت DoubleAPFloat::convert_منزل_Sign_Extended_Integer(
    MutableArrayRef<integerPart> داخلہ, بنانشانی صحيح چوڑائی, منطقي IsSigned,
    rounding_منوال RM, منطقي *IsExact) ثابت {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  إذا (!get_پہلا().isFiniteNonZero() || get_دوجا().isZero())
    أرجع get_پہلا().convert_منزل_Integer(داخلہ, چوڑائی, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = خطأ;
  DoubleAPFloat Integral = *هذا;
  ثابت op_حالت گیڑا_حالت = Integral.گیڑا_منزل_Integral(RM);
  إذا (گیڑا_حالت == opInvalidOp)
    أرجع opInvalidOp;
  ثابت APFloat &IntegralHi = Integral.get_پہلا();
  ثابت APFloat &IntegralLo = Integral.get_دوجا();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  منطقي HiIsExact;
  إذا (IntegralHi.isZero() || IntegralLo.isZero()) {
    ثابت op_حالت Hi_حالت =
        IntegralHi.convert_منزل_Integer(داخلہ, چوڑائی, IsSigned, RM, &HiIsExact);
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
  ثابت بنانشانی مثبت_حد_توں_ودھ_چوڑائی = IsSigned ? چوڑائی - 1 : چوڑائی;
  إذا (HiExactLog2 >= 0 &&
      static_cast<بنانشانی>(HiExactLog2) == مثبت_حد_توں_ودھ_چوڑائی) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    إذا (IntegralHi.isNegative() == IntegralLo.isNegative())
      أرجع opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    ثابت op_حالت Lo_حالت = IntegralLo.convert_منزل_Integer(
        داخلہ, چوڑائی, /*IsSigned=*/سچ, RM, &LoIsExact);
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
      APInt::tc_Clear_ثنائی_ہندسہ(داخلہ.انکڑے(), مثبت_حد_توں_ودھ_چوڑائی);
    *IsExact = گیڑا_حالت == opOK;
    أرجع گیڑا_حالت;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  ثابت op_حالت Hi_حالت = IntegralHi.convert_منزل_Integer(
      داخلہ, چوڑائی, IsSigned, rmTowardZero, &HiIsExact);
  إذا (Hi_حالت == opInvalidOp)
    أرجع Hi_حالت;

  // Convert Lo into a temporary integer of the same width.
  APSInt Lo_نتیجہ{چوڑائی, /*isUnsigned=*/!IsSigned};
  ثابت op_حالت Lo_حالت =
      IntegralLo.convert_منزل_Integer(Lo_نتیجہ, rmTowardZero, &LoIsExact);
  إذا (Lo_حالت == opInvalidOp)
    أرجع Lo_حالت;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tc_جوڑو(داخلہ.انکڑے(), Lo_نتیجہ.get_Raw_انکڑے(), /*carry=*/0, داخلہ.ناپ_2());

  *IsExact = گیڑا_حالت == opOK;
  أرجع گیڑا_حالت;
}

APFloat::op_حالت
DoubleAPFloat::convert_منزل_Integer(MutableArrayRef<integerPart> داخلہ,
                                بنانشانی صحيح چوڑائی, منطقي IsSigned,
                                rounding_منوال RM, منطقي *IsExact) ثابت {
  op_حالت FS =
      convert_منزل_Sign_Extended_Integer(داخلہ, چوڑائی, IsSigned, RM, IsExact);

  إذا (FS == opInvalidOp) {
    ثابت بنانشانی Dst_Parts_گنتی = part_گنتی_For_بٹ(چوڑائی);
    أكد(Dst_Parts_گنتی <= داخلہ.ناپ_2() && "Integer too big");

    بنانشانی بٹ_2;
    إذا (getCategory() == fcNaN)
      بٹ_2 = 0;
    وإلا إذا (isNegative())
      بٹ_2 = IsSigned;
    وإلا
      بٹ_2 = چوڑائی - IsSigned;

    tc_سیٹ_گھٹ_توں_گھٹ_Significant_بٹ(داخلہ.انکڑے(), Dst_Parts_گنتی, بٹ_2);
    إذا (isNegative() && IsSigned)
      APInt::tc_کھسکاؤ_بایاں(داخلہ.انکڑے(), Dst_Parts_گنتی, چوڑائی - 1);
  }

  أرجع FS;
}

APFloat::op_حالت DoubleAPFloat::handle_حد_توں_ودھ(rounding_منوال RM) {
  بدل (RM) {
  حالة APFloat::rmTowardZero:
    make_سبھ_توں_وڈا(/*Neg=*/isNegative());
    اكسر;
  حالة APFloat::rmTowardNegative:
    إذا (isNegative())
      makeInf(/*Neg=*/سچ);
    وإلا
      make_سبھ_توں_وڈا(/*Neg=*/خطأ);
    اكسر;
  حالة APFloat::rm_Toward_مثبت:
    إذا (isNegative())
      make_سبھ_توں_وڈا(/*Neg=*/سچ);
    وإلا
      makeInf(/*Neg=*/خطأ);
    اكسر;
  حالة APFloat::rm_سبھ_توں_نیڑے_Ties_منزل_Away:
  حالة APFloat::rm_سبھ_توں_نیڑے_Ties_منزل_Even:
    makeInf(/*Neg=*/isNegative());
    اكسر;
  افتراضي:
    llvm_unreachable("Invalid rounding mode found");
  }
  op_حالت S = opInexact;
  إذا (!get_پہلا().isFinite())
    S = static_cast<op_حالت>(S | op_حد_توں_ودھ);
  أرجع S;
}

APFloat::op_حالت DoubleAPFloat::convertFromUnsignedParts(
    ثابت integerPart *Src, بنانشانی صحيح Src_گنتی, rounding_منوال RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  ثابت بنانشانی SrcMSB = APInt::tcMSB(Src, Src_گنتی);
  إذا (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/خطأ);
    أرجع opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  ثابت بنانشانی Src_ثنائی_ہندسہ_چوڑائی = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/Src_ثنائی_ہندسہ_چوڑائی, ArrayRef(Src, Src_گنتی)},
                /*isUnsigned=*/سچ};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{get_پہلا().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/خطأ, rm_سبھ_توں_نیڑے_Ties_منزل_Even);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  إذا (!Hi.isFinite())
    أرجع handle_حد_توں_ودھ(RM);

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
  Hi.convert_منزل_Integer(HiAsInt, rm_سبھ_توں_نیڑے_Ties_منزل_Even, &HiAsIntIsExact);
  ثابت APInt Error = SrcInt.zext(HiAsInt.get_ثنائی_ہندسہ_چوڑائی()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  rounding_منوال LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  إذا (Error.isNegative()) {
    إذا (RM == rm_سبھ_توں_نیڑے_Ties_منزل_Away) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      ثابت بنانشانی Error_سرگرم_بٹ = Error.get_Significant_بٹ() - 1;
      ثابت بنانشانی LoPrecision = get_دوجا().getSemantics().precision;
      إذا (Error_سرگرم_بٹ > LoPrecision) {
        ثابت بنانشانی RoundingBoundary = Error_سرگرم_بٹ - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        إذا (Error.گنتی_Trailing_Zeros() == RoundingBoundary - 1)
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

  APFloat Lo{get_دوجا().getSemantics()};
  op_حالت حالت = Lo.convertFromAPInt(Error, /*IsSigned=*/سچ, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fast_Two_جوڑ(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  إذا (!get_پہلا().isFinite())
    أرجع handle_حد_توں_ودھ(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  إذا (get_پہلا().isFiniteNonZero() && Floats[0].is_سبھ_توں_وڈا()) {
    DoubleAPFloat سبھ_توں_وڈا{*Semantics};
    سبھ_توں_وڈا.make_سبھ_توں_وڈا(/*Neg=*/خطأ);
    إذا (compare(سبھ_توں_وڈا) == APFloat::cmpGreaterThan)
      أرجع handle_حد_توں_ودھ(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  أرجع حالت;
}

APFloat::op_حالت DoubleAPFloat::convertFromAPInt(ثابت APInt &داخلہ,
                                                  منطقي IsSigned,
                                                  rounding_منوال RM) {
  ثابت منطقي Negate_داخلہ = IsSigned && داخلہ.isNegative();
  APInt API = داخلہ;
  إذا (Negate_داخلہ)
    API.negate();

  ثابت APFloat::op_حالت حالت =
      convertFromUnsignedParts(API.get_Raw_انکڑے(), API.get_عدد_الفاظ(), RM);
  إذا (Negate_داخلہ)
    changeSign();
  أرجع حالت;
}

بنانشانی صحيح DoubleAPFloat::convert_منزل_Hex_String(حرف *DST,
                                               بنانشانی صحيح HexDigits,
                                               منطقي اتلا_Case_2,
                                               rounding_منوال RM) ثابت {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble &&
         "Unexpected Semantics");
  أرجع APFloat(APFloat_ادھار::semPPCDoubleDoubleLegacy, bitcast_منزل_APInt())
      .convert_منزل_Hex_String(DST, HexDigits, اتلا_Case_2, RM);
}

منطقي DoubleAPFloat::isDenormal() ثابت {
  أرجع getCategory() == fc_عام &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

منطقي DoubleAPFloat::is_سبھ_توں_چھوٹا() ثابت {
  إذا (getCategory() != fc_عام)
    أرجع خطأ;
  DoubleAPFloat Tmp(*هذا);
  Tmp.make_سبھ_توں_چھوٹا(هذا->isNegative());
  أرجع Tmp.compare(*هذا) == cmpEqual;
}

منطقي DoubleAPFloat::is_سبھ_توں_چھوٹا_Normalized() ثابت {
  إذا (getCategory() != fc_عام)
    أرجع خطأ;

  DoubleAPFloat Tmp(*هذا);
  Tmp.make_سبھ_توں_چھوٹا_Normalized(هذا->isNegative());
  أرجع Tmp.compare(*هذا) == cmpEqual;
}

منطقي DoubleAPFloat::is_سبھ_توں_وڈا() ثابت {
  إذا (getCategory() != fc_عام)
    أرجع خطأ;
  DoubleAPFloat Tmp(*هذا);
  Tmp.make_سبھ_توں_وڈا(هذا->isNegative());
  أرجع Tmp.compare(*هذا) == cmpEqual;
}

منطقي DoubleAPFloat::isInteger() ثابت {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble &&
         "Unexpected Semantics");
  أرجع Floats[0].isInteger() && Floats[1].isInteger();
}

فراغ DoubleAPFloat::منزل_String(Small_سمتیہ_Impl<حرف> &Str,
                             بنانشانی FormatPrecision,
                             بنانشانی FormatMaxPadding,
                             منطقي TruncateZero) ثابت {
  أكد(Semantics == &APFloat_ادھار::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloat_ادھار::semPPCDoubleDoubleLegacy, bitcast_منزل_APInt())
      .منزل_String(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

صحيح DoubleAPFloat::getExactLog2Abs() ثابت {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  إذا (get_دوجا().isNonZero())
    أرجع INT_MIN;
  أرجع get_پہلا().getExactLog2Abs();
}

صحيح ilogb(ثابت DoubleAPFloat &Arg) {
  ثابت APFloat &Hi = Arg.get_پہلا();
  ثابت APFloat &Lo = Arg.get_دوجا();
  صحيح Ilogb_نتیجہ = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  إذا (Arg.getCategory() != fc_عام)
    أرجع Ilogb_نتیجہ;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  إذا (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    أرجع Ilogb_نتیجہ;
  إذا (Hi.getExactLog2Abs() == INT_MIN)
    أرجع Ilogb_نتیجہ;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  أرجع Ilogb_نتیجہ - 1;
}

DoubleAPFloat scalbn(ثابت DoubleAPFloat &Arg, صحيح Exp,
                     APFloat::rounding_منوال RM) {
  أكد(Arg.Semantics == &APFloat_ادھار::PPCDoubleDouble() &&
         "Unexpected Semantics");
  أرجع DoubleAPFloat(APFloat_ادھار::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(ثابت DoubleAPFloat &Arg, صحيح &Exp,
                    APFloat::rounding_منوال RM) {
  أكد(Arg.Semantics == &APFloat_ادھار::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  إذا (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.get_پہلا() = Quiet.get_پہلا().makeQuiet();
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

  ثابت APFloat &Hi = Arg.get_پہلا();
  ثابت APFloat &Lo = Arg.get_دوجا();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  ثابت منطقي SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat دوجا = Lo;
  إذا (Arg.getCategory() == APFloat::fc_عام && Lo.isFiniteNonZero()) {
    rounding_منوال Lo_Rounding_منوال;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    إذا (RM == rmTowardZero)
      Lo_Rounding_منوال = Arg.isNegative() ? rm_Toward_مثبت : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    وإلا إذا (RM == rm_سبھ_توں_نیڑے_Ties_منزل_Away && SignsDisagree && Exp > 0)
      Lo_Rounding_منوال = rm_سبھ_توں_نیڑے_Ties_منزل_Even;
    وإلا
      Lo_Rounding_منوال = RM;
    دوجا = scalbn(Lo, -Exp, Lo_Rounding_منوال);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    إذا (RM == rm_سبھ_توں_نیڑے_Ties_منزل_Away && Lo_Rounding_منوال == rm_سبھ_توں_نیڑے_Ties_منزل_Even) {
      // Re-scale the result back to check if rounding occurred.
      ثابت APFloat RecomposedLo = scalbn(دوجا, Exp, rm_سبھ_توں_نیڑے_Ties_منزل_Even);
      إذا (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        ثابت APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        ثابت APFloat Ulp_دا_دوجا = harrisonUlp(دوجا);
        ثابت APFloat Scaled_Ulp_دا_دوجا =
            scalbn(Ulp_دا_دوجا, Exp - 1, rm_سبھ_توں_نیڑے_Ties_منزل_Even);
        ثابت منطقي IsMidpoint = مطلق_قدر(RoundingError) == Scaled_Ulp_دا_دوجا;
        ثابت منطقي RoundedLoAway =
            دوجا.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        إذا (IsMidpoint && RoundedLoAway)
          دوجا.اگلا(/*nextDown=*/!دوجا.isNegative());
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
    إذا (دوجا.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat پہلا = scalbn(Hi, -Exp, RM);
  أرجع DoubleAPFloat(APFloat_ادھار::PPCDoubleDouble(), std::move(پہلا),
                       std::move(دوجا));
}

APInt DoubleAPFloat::getNaNPayload() ثابت { أرجع Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, ثابت fltSemantics &Semantics) {
  إذا (ورتدا_Layout<IEEEFloat>(Semantics)) {
    جديد (&IEEE) IEEEFloat(std::move(F));
    أرجع;
  }
  إذا (ورتدا_Layout<DoubleAPFloat>(Semantics)) {
    ثابت fltSemantics& S = F.getSemantics();
    جديد (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloat_ادھار::IEEEdouble()));
    أرجع;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::op_حالت> APFloat::convertFromString(StringRef Str,
                                                       rounding_منوال RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_کوڈ hash_قدر(ثابت APFloat &Arg) {
  إذا (APFloat::ورتدا_Layout<detail::IEEEFloat>(Arg.getSemantics()))
    أرجع hash_قدر(Arg.U.IEEE);
  إذا (APFloat::ورتدا_Layout<detail::DoubleAPFloat>(Arg.getSemantics()))
    أرجع hash_قدر(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(ثابت fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  خودکار حالت_Or_Err = convertFromString(S, rm_سبھ_توں_نیڑے_Ties_منزل_Even);
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
                                   rounding_منوال RM, منطقي *losesInfo) {
  إذا (&getSemantics() == &منزل_Semantics_2) {
    *losesInfo = خطأ;
    أرجع opOK;
  }
  إذا (ورتدا_Layout<IEEEFloat>(getSemantics()) &&
      ورتدا_Layout<IEEEFloat>(منزل_Semantics_2))
    أرجع U.IEEE.convert(منزل_Semantics_2, RM, losesInfo);
  إذا (ورتدا_Layout<IEEEFloat>(getSemantics()) &&
      ورتدا_Layout<DoubleAPFloat>(منزل_Semantics_2)) {
    أكد(&منزل_Semantics_2 == &APFloat_ادھار::semPPCDoubleDouble);
    خودکار Ret =
        U.IEEE.convert(APFloat_ادھار::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *هذا = APFloat(منزل_Semantics_2, U.IEEE.bitcast_منزل_APInt());
    أرجع Ret;
  }
  إذا (ورتدا_Layout<DoubleAPFloat>(getSemantics()) &&
      ورتدا_Layout<IEEEFloat>(منزل_Semantics_2)) {
    خودکار Ret = getIEEE().convert(منزل_Semantics_2, RM, losesInfo);
    *هذا = APFloat(std::move(getIEEE()), منزل_Semantics_2);
    أرجع Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_قدر(ثابت fltSemantics &Semantics) {
  أرجع APFloat(Semantics, APInt::getAllOnes(Semantics.ناپ_وچ_بٹ));
}

فراغ APFloat::print(raw_ostream &OS) ثابت {
  Small_سمتیہ<حرف, 16> بفر;
  منزل_String(بفر);
  OS << بفر;
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طریقہ فراغ APFloat::dump() ثابت {
  print(dbgs());
  dbgs() << '\n';
}
#جے_انت

فراغ APFloat::Profile(Folding_سیٹ_گرہ_ID &NID) ثابت {
  NID.جوڑو_2(bitcast_منزل_APInt());
}

APFloat::op_حالت APFloat::convert_منزل_Integer(APSInt &نتیجہ_2,
                                            rounding_منوال rounding_منوال_2,
                                            منطقي *isExact) ثابت {
  بنانشانی ثنائی_ہندسہ_چوڑائی = نتیجہ_2.get_ثنائی_ہندسہ_چوڑائی();
  Small_سمتیہ<uint64_t, 4> parts(نتیجہ_2.get_عدد_الفاظ());
  op_حالت حالت_2 = convert_منزل_Integer(parts, ثنائی_ہندسہ_چوڑائی, نتیجہ_2.isSigned(),
                                     rounding_منوال_2, isExact);
  // Keeps the original signed-ness.
  نتیجہ_2 = APInt(ثنائی_ہندسہ_چوڑائی, parts);
  أرجع حالت_2;
}

مزدوج APFloat::convert_منزل_Double() ثابت {
  إذا (&getSemantics() == &APFloat_ادھار::semIEEEdouble)
    أرجع getIEEE().convert_منزل_Double();
  أكد(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat عارضی = *هذا;
  منطقي LosesInfo;
  [[maybe_بن_ورتیا]] op_حالت St =
      عارضی.convert(APFloat_ادھار::semIEEEdouble, rm_سبھ_توں_نیڑے_Ties_منزل_Even, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  أرجع عارضی.getIEEE().convert_منزل_Double();
}

#جے_تعریف_شدہ HAS_IEE754_FLOAT128
float128 APFloat::convert_منزل_Quad() ثابت {
  إذا (&getSemantics() == &APFloat_ادھار::semIEEEquad)
    أرجع getIEEE().convert_منزل_Quad();
  أكد(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat عارضی = *هذا;
  منطقي LosesInfo;
  [[maybe_بن_ورتیا]] op_حالت St =
      عارضی.convert(APFloat_ادھار::semIEEEquad, rm_سبھ_توں_نیڑے_Ties_منزل_Even, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  أرجع عارضی.getIEEE().convert_منزل_Quad();
}
#جے_انت

عائم APFloat::convert_منزل_Float() ثابت {
  إذا (&getSemantics() == &APFloat_ادھار::semIEEEsingle)
    أرجع getIEEE().convert_منزل_Float();
  أكد(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat عارضی = *هذا;
  منطقي LosesInfo;
  [[maybe_بن_ورتیا]] op_حالت St =
      عارضی.convert(APFloat_ادھار::semIEEEsingle, rm_سبھ_توں_نیڑے_Ties_منزل_Even, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  أرجع عارضی.getIEEE().convert_منزل_Float();
}

بنانشانی APFloat_ادھار::get_Arbitrary_FPFormat_ناپ_وچ_بٹ(StringRef Format) {
  أرجع StringSwitch<بنانشانی>(Format)
      .Case("Float8E5M2", get_ناپ_وچ_بٹ(semFloat8E5M2))
      .Case("Float8E5M2FNUZ", get_ناپ_وچ_بٹ(semFloat8E5M2FNUZ))
      .Case("Float8E4M3", get_ناپ_وچ_بٹ(semFloat8E4M3))
      .Case("Float8E4M3FN", get_ناپ_وچ_بٹ(semFloat8E4M3FN))
      .Case("Float8E4M3FNUZ", get_ناپ_وچ_بٹ(semFloat8E4M3FNUZ))
      .Case("Float8E4M3B11FNUZ", get_ناپ_وچ_بٹ(semFloat8E4M3B11FNUZ))
      .Case("Float8E3M4", get_ناپ_وچ_بٹ(semFloat8E3M4))
      .Case("Float8E8M0FNU", get_ناپ_وچ_بٹ(semFloat8E8M0FNU))
      .Case("Float6E3M2FN", get_ناپ_وچ_بٹ(semFloat6E3M2FN))
      .Case("Float6E2M3FN", get_ناپ_وچ_بٹ(semFloat6E2M3FN))
      .Case("Float4E2M1FN", get_ناپ_وچ_بٹ(semFloat4E2M1FN))
      .Case("Float8E5M3FNU", get_ناپ_وچ_بٹ(semFloat8E5M3FNU))
      .Default(0);
}

منطقي APFloat_ادھار::isValidArbitraryFPFormat(StringRef Format) {
  أرجع get_Arbitrary_FPFormat_ناپ_وچ_بٹ(Format) != 0;
}

ثابت fltSemantics *APFloat_ادھار::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  أرجع StringSwitch<ثابت fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(خالی_سوچک);
}

APFloat::Storage::~Storage() {
  إذا (ورتدا_Layout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    أرجع;
  }
  إذا (ورتدا_Layout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    أرجع;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(ثابت APFloat::Storage &RHS) {
  إذا (ورتدا_Layout<IEEEFloat>(*RHS.semantics)) {
    جديد (هذا) IEEEFloat(RHS.IEEE);
    أرجع;
  }
  إذا (ورتدا_Layout<DoubleAPFloat>(*RHS.semantics)) {
    جديد (هذا) DoubleAPFloat(RHS.Double);
    أرجع;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  إذا (ورتدا_Layout<IEEEFloat>(*RHS.semantics)) {
    جديد (هذا) IEEEFloat(std::move(RHS.IEEE));
    أرجع;
  }
  إذا (ورتدا_Layout<DoubleAPFloat>(*RHS.semantics)) {
    جديد (هذا) DoubleAPFloat(std::move(RHS.Double));
    أرجع;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::عامل=(ثابت APFloat::Storage &RHS) {
  إذا (ورتدا_Layout<IEEEFloat>(*semantics) &&
      ورتدا_Layout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } وإلا إذا (ورتدا_Layout<DoubleAPFloat>(*semantics) &&
             ورتدا_Layout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } وإلا إذا (هذا != &RHS) {
    هذا->~Storage();
    جديد (هذا) Storage(RHS);
  }
  أرجع *هذا;
}

APFloat::Storage &APFloat::Storage::عامل=(APFloat::Storage &&RHS) {
  إذا (ورتدا_Layout<IEEEFloat>(*semantics) &&
      ورتدا_Layout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } وإلا إذا (ورتدا_Layout<DoubleAPFloat>(*semantics) &&
             ورتدا_Layout<DoubleAPFloat>(*RHS.semantics)) {
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
  إذا (libc_exceptions & FE_حد_توں_ودھ)
    حالت_2 = static_cast<APFloat::op_حالت>(حالت_2 | APFloat::op_حد_توں_ودھ);
  إذا (libc_exceptions & FE_UNDERFLOW)
    حالت_2 = static_cast<APFloat::op_حالت>(حالت_2 | APFloat::opUnderflow);
  إذا (libc_exceptions & FE_INEXACT)
    حالت_2 = static_cast<APFloat::op_حالت>(حالت_2 | APFloat::opInexact);
  أرجع حالت_2;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> قوت_نما(ثابت APFloat &x, Rounding_منوال rounding_منوال_2,
                           APFloat::op_حالت *حالت_2) {

  إذا (rounding_منوال_2 == APFloat_ادھار::rm_سبھ_توں_نیڑے_Ties_منزل_Even) {
    إذا (APFloat::Semantics_منزل_Enum(x.getSemantics()) ==
        APFloat_ادھار::S_IEEEsingle) {
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
      عائم نتیجہ_2 = LIBC_NAMESPACE::shared::expf(x_val);
      أرجع APFloat(نتیجہ_2);
    }
    إذا (APFloat::Semantics_منزل_Enum(x.getSemantics()) ==
        APFloat_ادھار::S_IEEEdouble) {
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
      مزدوج نتیجہ_2 = LIBC_NAMESPACE::shared::قوت_نما(x_val);
      أرجع APFloat(نتیجہ_2);
    }
  }
  أرجع std::nullopt;
}

} // namespace llvm

#تعریف_رد APFLOAT_DISPATCH_ON_SEMANTICS
