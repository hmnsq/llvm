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
#تعریف_کن LIBC_NAMESPACE __llvm_libc_apfloat
#تعریف_کن LIBC_MATH (LIBC_MATH_NO_ERRNO | LIBC_MATH_NO_EXCEPT)

#ضمن "shared/math.h"
#ضمن "shared/math_check_exceptions.h"

#تعریف_کن APFLOAT_DISPATCH_ON_SEMANTICS(روش_CALL)                             \
  افعل {                                                                         \
    إذا (استفاده_میکند_Layout<IEEEFloat>(getSemantics()))                                 \
      أرجع U.IEEE.روش_CALL;                                               \
    إذا (استفاده_میکند_Layout<DoubleAPFloat>(getSemantics()))                             \
      أرجع U.Double.روش_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } طالما (خطأ)

استفاده نطاق llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#تعریف_کن Pack_Categories_Into_کلید(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloat_پایه::integer_Part_عرض % 4 == 0, "Part width must be divisible by 4!");

نطاق llvm {

constexpr fltSemantics APFloat_پایه::semIEEEhalf = {15, -14, 11, 16};
constexpr fltSemantics APFloat_پایه::semBFloat = {127, -126, 8, 16};
constexpr fltSemantics APFloat_پایه::semIEEEsingle = {127, -126, 24, 32};
constexpr fltSemantics APFloat_پایه::semIEEEdouble = {1023, -1022, 53, 64};
constexpr fltSemantics APFloat_پایه::semIEEEquad = {16383, -16382, 113, 128};
constexpr fltSemantics APFloat_پایه::semX87DoubleExtended = {
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
constexpr fltSemantics APFloat_پایه::semPPCDoubleDouble = {-1, 0, 0, 128};
constexpr fltSemantics APFloat_پایه::semPPCDoubleDoubleLegacy = {
    1023, -1022 + 53, 53 + 53, 128};
constexpr fltSemantics APFloat_پایه::semFloat8E5M2 = {15, -14, 3, 8};
constexpr fltSemantics APFloat_پایه::semFloat8E5M2FNUZ = {
    15, -15, 3, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_پایه::semFloat8E4M3 = {7, -6, 4, 8};
constexpr fltSemantics APFloat_پایه::semFloat8E4M3FN = {
    8, -6, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::AllOnes};
constexpr fltSemantics APFloat_پایه::semFloat8E4M3FNUZ = {
    7, -7, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_پایه::semFloat8E4M3B11FNUZ = {
    4, -10, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_پایه::semFloat8E3M4 = {3, -2, 5, 8};
constexpr fltSemantics APFloat_پایه::semFloatTF32 = {127, -126, 11, 19};
constexpr fltSemantics APFloat_پایه::semFloat8E8M0FNU = {
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

constexpr fltSemantics APFloat_پایه::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    درست,
    خطأ,
    خطأ};

constexpr fltSemantics APFloat_پایه::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_پایه::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_پایه::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_پایه::semBogus = {0, 0, 0, 0};

ثابت llvm::fltSemantics &APFloat_پایه::Enum_مقصد_Semantics(Semantics S) {
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

APFloat_پایه::Semantics
APFloat_پایه::Semantics_مقصد_Enum(ثابت llvm::fltSemantics &Sem) {
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

منطقي APFloat_پایه::isRepresentableBy(ثابت fltSemantics &A,
                                    ثابت fltSemantics &B) {
  أرجع A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

منطقي APFloat_پایه::is_Losslessly_Convertible_مقصد(ثابت fltSemantics &From,
                                            ثابت fltSemantics &Initial_مقصد,
                                            منطقي IgnoreNaNs) {
  إذا (&From == &Initial_مقصد)
    أرجع درست;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  إذا (&From == &semPPCDoubleDouble)
    أرجع خطأ;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  ثابت fltSemantics &مقصد =
      &Initial_مقصد == &semPPCDoubleDouble ? semIEEEdouble : Initial_مقصد;

  إذا (!isRepresentableBy(From, مقصد))
    أرجع خطأ;

  إذا ((From.hasZero && !مقصد.hasZero) ||
      (From.hasSignedRepr && !مقصد.hasSignedRepr))
    أرجع خطأ;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  منطقي FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  منطقي مقصد_Has_Signed_Zero = مقصد.hasZero && مقصد.hasSignedRepr &&
                         مقصد.nanEncoding != fltNanEncoding::NegativeZero;
  إذا (FromHasSignedZero && !مقصد_Has_Signed_Zero)
    أرجع خطأ;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat کوچکترین_From = APFloat::get_کوچکترین(From);
  منطقي LosesInfo = خطأ;
  (فراغ)کوچکترین_From.convert(مقصد, APFloat::rm_نزدیکترین_Ties_مقصد_Even, &LosesInfo);
  إذا (LosesInfo)
    أرجع خطأ;

  إذا (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    أرجع درست;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  إذا (IgnoreNaNs) {
    إذا (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      أرجع مقصد.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    أرجع مقصد.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  إذا (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    أرجع خطأ;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  إذا (مقصد.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    أرجع درست;
  أرجع مقصد.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
         From.nanEncoding == مقصد.nanEncoding;
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
[[maybe_استفادهنشده]] constexpr بیعلامت maxExponent = 16383;

// Decimal exponents in the f16 and f32 ranges need at most 2 and 6 parts.
// Keep 16 parts (128 bytes) inline: enough for 5^413, but not a large stack
// frame. Longer fractional spellings use heap storage.
ساكن constexpr بیعلامت رقم_Pow5_Parts_On_پشته = 16;

ساكن بیعلامت صحيح power_از_Five_Parts_For_Exponent(بیعلامت صحيح power) {
  أكد(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  أرجع 2 + ((power * 815) / (351 * APFloat_پایه::integer_Part_عرض));
}

بیعلامت صحيح APFloat_پایه::semanticsPrecision(ثابت fltSemantics &semantics) {
  أرجع semantics.precision;
}
APFloat_پایه::ExponentType
APFloat_پایه::semanticsMaxExponent(ثابت fltSemantics &semantics) {
  أرجع semantics.maxExponent;
}
APFloat_پایه::ExponentType
APFloat_پایه::semanticsMinExponent(ثابت fltSemantics &semantics) {
  أرجع semantics.minExponent;
}
بیعلامت صحيح APFloat_پایه::semantics_اندازه_درون_بیتها(ثابت fltSemantics &semantics) {
  أرجع semantics.اندازه_درون_بیتها;
}
بیعلامت صحيح APFloat_پایه::semantics_Int_اندازه_درون_بیتها(ثابت fltSemantics &semantics,
                                                 منطقي isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  بیعلامت صحيح Min_بیت_عرض = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  إذا (isSigned)
    ++Min_بیت_عرض;
  أرجع Min_بیت_عرض;
}

منطقي APFloat_پایه::semanticsHasZero(ثابت fltSemantics &semantics) {
  أرجع semantics.hasZero;
}

منطقي APFloat_پایه::semanticsHasSignedRepr(ثابت fltSemantics &semantics) {
  أرجع semantics.hasSignedRepr;
}

منطقي APFloat_پایه::semanticsHasInf(ثابت fltSemantics &semantics) {
  أرجع semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

منطقي APFloat_پایه::semanticsHasNaN(ثابت fltSemantics &semantics) {
  أرجع semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

منطقي APFloat_پایه::isIEEELikeFP(ثابت fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  أرجع Semantics_مقصد_Enum(semantics) <= S_IEEEquad;
}

منطقي APFloat_پایه::has_Sign_بیت_درون_MSB(ثابت fltSemantics &semantics) {
  أرجع semantics.has_Sign_بیت_درون_MSB;
}

منطقي APFloat_پایه::is_Representable_As_نرمال_درون(ثابت fltSemantics &Src,
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

بیعلامت APFloat_پایه::get_اندازه_درون_بیتها(ثابت fltSemantics &Sem) {
  أرجع Sem.اندازه_درون_بیتها;
}

ساكن constexpr APFloat_پایه::ExponentType
exponentZero(ثابت fltSemantics &semantics) {
  أرجع semantics.minExponent - 1;
}

ساكن constexpr APFloat_پایه::ExponentType
exponentInf(ثابت fltSemantics &semantics) {
  أرجع semantics.maxExponent + 1;
}

ساكن constexpr APFloat_پایه::ExponentType
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
  أرجع make_error<StringError>(Err, inconvertible_Error_کد());
}

ساكن constexpr مضمن بیعلامت صحيح part_شمار_For_بیتها(بیعلامت صحيح بیتها) {
  أرجع std::max(1u, (بیتها + APFloat_پایه::integer_Part_عرض - 1) /
                          APFloat_پایه::integer_Part_عرض);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
ساكن مضمن بیعلامت صحيح
dec_Digit_مقدار(بیعلامت صحيح c)
{
  أرجع c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
ساكن Expected<صحيح> readExponent(StringRef::iterator begin,
                                  StringRef::iterator انتها) {
  ثابت بیعلامت صحيح overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  إذا (p == انتها || ((*p == '-' || *p == '+') && (p + 1) == انتها))
    أرجع 0;

  منطقي isNegative = *p == '-';
  إذا (*p == '-' || *p == '+') {
    p++;
    إذا (p == انتها)
      أرجع createError("Exponent has no digits");
  }

  بیعلامت absExponent = dec_Digit_مقدار(*p++);
  إذا (absExponent >= 10U)
    أرجع createError("Invalid character in exponent");

  لكل (; p != انتها; ++p) {
    بیعلامت مقدار_2 = dec_Digit_مقدار(*p);
    إذا (مقدار_2 >= 10U)
      أرجع createError("Invalid character in exponent");

    absExponent = absExponent * 10U + مقدار_2;
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
ساكن Expected<صحيح> مجموع_Exponent(StringRef::iterator p,
                                   StringRef::iterator انتها,
                                   صحيح exponentAdjustment) {
  صحيح exponent = 0;

  إذا (p == انتها)
    أرجع createError("Exponent has no digits");

  منطقي negative = *p == '-';
  إذا (*p == '-' || *p == '+') {
    p++;
    إذا (p == انتها)
      أرجع createError("Exponent has no digits");
  }

  صحيح unsignedExponent = 0;
  منطقي سرریز_2 = خطأ;
  لكل (; p != انتها; ++p) {
    بیعلامت صحيح مقدار_2;

    مقدار_2 = dec_Digit_مقدار(*p);
    إذا (مقدار_2 >= 10U)
      أرجع createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + مقدار_2;
    إذا (unsignedExponent > 32767) {
      سرریز_2 = درست;
      اكسر;
    }
  }

  إذا (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    سرریز_2 = درست;

  إذا (!سرریز_2) {
    exponent = unsignedExponent;
    إذا (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    إذا (exponent > 32767 || exponent < -32768)
      سرریز_2 = درست;
  }

  إذا (سرریز_2)
    exponent = negative ? -32768: 32767;

  أرجع exponent;
}

ساكن Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator انتها,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = انتها;
  طالما (p != انتها && *p == '0')
    p++;

  إذا (p != انتها && *p == '.') {
    *dot = p++;

    إذا (انتها - begin == 1)
      أرجع createError("Significand has no digits");

    طالما (p != انتها && *p == '0')
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
  ثابت حرف *اولی_Sig_Digit;
  ثابت حرف *آخرین_Sig_Digit;
  صحيح exponent;
  صحيح normalizedExponent;
};

ساكن Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator انتها, decimalInfo *D) {
  StringRef::iterator dot = انتها;

  خودکار PtrOrErr = skipLeadingZeroesAndAnyDot(begin, انتها, &dot);
  إذا (!PtrOrErr)
    أرجع PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->اولی_Sig_Digit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  لكل (; p != انتها; ++p) {
    إذا (*p == '.') {
      إذا (dot != انتها)
        أرجع createError("String contains multiple dots");
      dot = p++;
      إذا (p == انتها)
        اكسر;
    }
    إذا (dec_Digit_مقدار(*p) >= 10U)
      اكسر;
  }

  إذا (p != انتها) {
    إذا (*p != 'e' && *p != 'E')
      أرجع createError("Invalid character in significand");
    إذا (p == begin)
      أرجع createError("Significand has no digits");
    إذا (dot != انتها && p - begin == 1)
      أرجع createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    خودکار ExpOrErr = readExponent(p + 1, انتها);
    إذا (!ExpOrErr)
      أرجع ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    إذا (dot == انتها)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  إذا (p != D->اولی_Sig_Digit) {
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
              static_cast<APFloat::ExponentType>((p - D->اولی_Sig_Digit)
                                      - (dot > D->اولی_Sig_Digit && dot < p)));
  }

  D->آخرین_Sig_Digit = p;
  أرجع Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
ساكن Expected<گمشده_Fraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator انتها,
                            بیعلامت صحيح digit_مقدار) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  إذا (digit_مقدار > 8)
    أرجع lf_بیشتر_Than_Half;
  وإلا إذا (digit_مقدار < 8 && digit_مقدار > 0)
    أرجع lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  طالما (p != انتها && (*p == '0' || *p == '.'))
    p++;

  إذا (p == انتها)
    أرجع createError("Invalid trailing hexadecimal fraction!");

  بیعلامت hexDigit = hex_Digit_مقدار(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  إذا (hexDigit == UINT_MAX)
    أرجع digit_مقدار == 0 ? lfExactlyZero: lfExactlyHalf;
  وإلا
    أرجع digit_مقدار == 0 ? lfLessThanHalf: lf_بیشتر_Than_Half;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
ساكن گمشده_Fraction
گمشده_Fraction_Through_Truncation(ثابت APFloat_پایه::integerPart *parts,
                              بیعلامت صحيح part_شمار,
                              بیعلامت صحيح بیتها)
{
  بیعلامت lsb = APInt::tcLSB(parts, part_شمار);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  إذا (بیتها <= lsb)
    أرجع lfExactlyZero;
  إذا (بیتها == lsb + 1)
    أرجع lfExactlyHalf;
  إذا (بیتها <= part_شمار * APFloat_پایه::integer_Part_عرض &&
      APInt::tc_استخراج_بیت(parts, بیتها - 1))
    أرجع lf_بیشتر_Than_Half;

  أرجع lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
ساكن گمشده_Fraction
لغزش_راست(APFloat_پایه::integerPart *dst, بیعلامت صحيح parts, بیعلامت صحيح بیتها)
{
  گمشده_Fraction گمشده_fraction = گمشده_Fraction_Through_Truncation(dst, parts, بیتها);

  APInt::tc_لغزش_راست(dst, parts, بیتها);

  أرجع گمشده_fraction;
}

/* Combine the effect of two lost fractions.  */
ساكن گمشده_Fraction
combine_گمشده_Fractions(گمشده_Fraction بیشتر_Significant,
                     گمشده_Fraction lessSignificant)
{
  إذا (lessSignificant != lfExactlyZero) {
    إذا (بیشتر_Significant == lfExactlyZero)
      بیشتر_Significant = lfLessThanHalf;
    وإلا إذا (بیشتر_Significant == lfExactlyHalf)
      بیشتر_Significant = lf_بیشتر_Than_Half;
  }

  أرجع بیشتر_Significant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
ساكن بیعلامت صحيح
HUerrBound(منطقي inexactMultiply, بیعلامت صحيح HUerr1, بیعلامت صحيح HUerr2)
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
ساكن APFloat_پایه::integerPart
ulpsFromBoundary(ثابت APFloat_پایه::integerPart *parts, بیعلامت صحيح بیتها,
                 منطقي is_نزدیکترین) {
  أكد(بیتها != 0);

  بیتها--;
  بیعلامت شمار = بیتها / APFloat_پایه::integer_Part_عرض;
  بیعلامت part_بیتها = بیتها % APFloat_پایه::integer_Part_عرض + 1;

  APFloat_پایه::integerPart part =
      parts[شمار] & (~(APFloat_پایه::integerPart)0 >>
                      (APFloat_پایه::integer_Part_عرض - part_بیتها));

  APFloat_پایه::integerPart boundary;
  إذا (is_نزدیکترین)
    boundary = (APFloat_پایه::integerPart) 1 << (part_بیتها - 1);
  وإلا
    boundary = 0;

  إذا (شمار == 0) {
    إذا (part - boundary <= boundary - part)
      أرجع part - boundary;
    وإلا
      أرجع boundary - part;
  }

  إذا (part == boundary) {
    طالما (--شمار)
      إذا (parts[شمار])
        أرجع ~(APFloat_پایه::integerPart) 0; /* A lot.  */

    أرجع parts[0];
  } وإلا إذا (part == boundary - 1) {
    طالما (--شمار)
      إذا (~parts[شمار])
        أرجع ~(APFloat_پایه::integerPart) 0; /* A lot.  */

    أرجع -parts[0];
  }

  أرجع ~(APFloat_پایه::integerPart) 0; /* A lot.  */
}

ساكن constexpr بیعلامت Power_از_Five_Part_Counts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

ساكن constexpr APFloat_پایه::integerPart Power_از_Five_Parts[] = {
#ضمن "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  بیعلامت جمع = 0;
  لكل (بیعلامت شمار_2 : Power_از_Five_Part_Counts)
    جمع += شمار_2;
  أرجع جمع;
}() == std::اندازه_2(Power_از_Five_Parts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
ساكن بیعلامت صحيح
powerOf5(APFloat_پایه::integerPart *dst, بیعلامت صحيح power) {
  ساكن ثابت APFloat_پایه::integerPart اولی_Eight_Powers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  Small_وکتور<APFloat_پایه::integerPart, رقم_Pow5_Parts_On_پشته> scratch;
  scratch.resize_for_overwrite(power_از_Five_Parts_For_Exponent(power));

  APFloat_پایه::integerPart *p1, *p2;
  ثابت APFloat_پایه::integerPart *pow5;

  p1 = dst;
  p2 = scratch.داده();

  *p1 = اولی_Eight_Powers[power & 7];
  power >>= 3;

  بیعلامت نتیجه_2 = 1;
  pow5 = Power_از_Five_Parts;

  لكل (بیعلامت صحيح n = 0; power; power >>= 1, n++) {
    بیعلامت parts_شمار = Power_از_Five_Part_Counts[n];

    إذا (power & 1) {
      APFloat_پایه::integerPart *tmp;

      APInt::tc_پر_Multiply(p2, p1, pow5, نتیجه_2, parts_شمار);
      نتیجه_2 += parts_شمار;
      إذا (p2[نتیجه_2 - 1] == 0)
        نتیجه_2--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_شمار;
  }

  إذا (p1 != dst)
    APInt::tc_تخصیص_بده(dst, p1, نتیجه_2);

  أرجع نتیجه_2;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
ساكن ثابت حرف hex_Digits_پایینی[] = "0123456789abcdef0";
ساكن ثابت حرف hex_Digits_فوقانی[] = "0123456789ABCDEF0";
ساكن ثابت حرف infinityL[] = "infinity";
ساكن ثابت حرف infinityU[] = "INFINITY";
ساكن ثابت حرف NaNL[] = "nan";
ساكن ثابت حرف NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
ساكن بیعلامت صحيح
partAsHex (حرف *dst, APFloat_پایه::integerPart part, بیعلامت صحيح شمار,
           ثابت حرف *hexDigitChars)
{
  بیعلامت صحيح نتیجه_2 = شمار;

  أكد(شمار != 0 && شمار <= APFloat_پایه::integer_Part_عرض / 4);

  part >>= (APFloat_پایه::integer_Part_عرض - 4 * شمار);
  طالما (شمار--) {
    dst[شمار] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  أرجع نتیجه_2;
}

/* Write out an unsigned decimal integer.  */
ساكن حرف *writeUnsignedDecimal(حرف *dst, بیعلامت صحيح n) {
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
ساكن حرف *writeSignedDecimal(حرف *dst, صحيح مقدار_2) {
  إذا (مقدار_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(بیعلامت) مقدار_2);
  } وإلا {
    dst = writeUnsignedDecimal(dst, مقدار_2);
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
    أرجع APFloat::get_کوچکترین(Sem);
  حالة APFloat::fc_نرمال:
    اكسر;
  }
  إذا (X.isDenormal() || X.is_کوچکترین_Normalized())
    أرجع APFloat::get_کوچکترین(Sem);
  صحيح Exp = ilogb(X);
  إذا (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  أرجع scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rm_نزدیکترین_Ties_مقصد_Even);
}

نطاق detail {
/* Constructors.  */
فراغ IEEEFloat::initialize(ثابت fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  بیعلامت شمار = part_شمار();
  إذا (شمار > 1)
    significand.parts = جديد integerPart[شمار];
}

فراغ IEEEFloat::freeSignificand() {
  إذا (needsCleanup())
    حذف [] significand.parts;
}

فراغ IEEEFloat::تخصیص_بده(ثابت IEEEFloat &rhs) {
  أكد(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  إذا (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

فراغ IEEEFloat::copySignificand(ثابت IEEEFloat &rhs) {
  أكد(isFiniteNonZero() || category == fcNaN);
  أكد(rhs.part_شمار() >= part_شمار());

  APInt::tc_تخصیص_بده(significandParts(), rhs.significandParts(),
                  part_شمار());
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
  بیعلامت رقم_Parts = part_شمار();

  APInt fill_storage;
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = خطأ;
    إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = درست;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } وإلا {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  إذا (!fill || fill->get_رقم_واژگان() < رقم_Parts)
    APInt::tc_مجموعه(significand, 0, رقم_Parts);
  إذا (fill) {
    APInt::tc_تخصیص_بده(significand, fill->get_Raw_داده(),
                    std::min(fill->get_رقم_واژگان(), رقم_Parts));

    // Zero out the excess bits of the significand.
    بیعلامت بیتها_مقصد_Preserve = semantics->precision - 1;
    بیعلامت part = بیتها_مقصد_Preserve / 64;
    بیتها_مقصد_Preserve %= 64;
    significand[part] &= ((1ULL << بیتها_مقصد_Preserve) - 1);
    لكل (part++; part != رقم_Parts; ++part)
      significand[part] = 0;
  }

  بیعلامت QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  إذا (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tc_Clear_بیت(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    إذا (APInt::tcIsZero(significand, رقم_Parts))
      APInt::tc_مجموعه_بیت(significand, QNaNBit - 1);
  } وإلا إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } وإلا {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tc_مجموعه_بیت(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  إذا (semantics == &APFloat_پایه::semX87DoubleExtended)
    APInt::tc_مجموعه_بیت(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::عملگر=(ثابت IEEEFloat &rhs) {
  إذا (هذا != &rhs) {
    إذا (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    تخصیص_بده(rhs);
  }

  أرجع *هذا;
}

IEEEFloat &IEEEFloat::عملگر=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloat_پایه::semBogus;
  أرجع *هذا;
}

منطقي IEEEFloat::isDenormal() ثابت {
  أرجع getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tc_استخراج_بیت(significandParts(), semantics->precision - 1) ==
          0);
}

منطقي IEEEFloat::is_کوچکترین() ثابت {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  أرجع isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

منطقي IEEEFloat::is_کوچکترین_Normalized() ثابت {
  أرجع getCategory() == fc_نرمال && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

بیعلامت صحيح IEEEFloat::get_رقم_بالا_بیتها() ثابت {
  ثابت بیعلامت صحيح Part_شمار = part_شمار_For_بیتها(semantics->precision);
  ثابت بیعلامت صحيح بیتها_2 = Part_شمار * integer_Part_عرض;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  ثابت بیعلامت صحيح رقم_بالا_بیتها = (semantics->precision > 1)
                                       ? (بیتها_2 - semantics->precision + 1)
                                       : (بیتها_2 - semantics->precision);
  أرجع رقم_بالا_بیتها;
}

منطقي IEEEFloat::isSignificandAllOnes() ثابت {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  ثابت integerPart *Parts = significandParts();
  ثابت بیعلامت Part_شمار = part_شمار_For_بیتها(semantics->precision);
  لكل (بیعلامت i = 0; i < Part_شمار - 1; i++)
    إذا (~Parts[i])
      أرجع خطأ;

  // Set the unused high bits to all ones when we compare.
  ثابت بیعلامت رقم_بالا_بیتها = get_رقم_بالا_بیتها();
  أكد(رقم_بالا_بیتها <= integer_Part_عرض && رقم_بالا_بیتها > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ثابت integerPart بالا_بیت_Fill =
    ~integerPart(0) << (integer_Part_عرض - رقم_بالا_بیتها);
  إذا ((semantics->precision <= 1) || (~(Parts[Part_شمار - 1] | بالا_بیت_Fill)))
    أرجع خطأ;

  أرجع درست;
}

منطقي IEEEFloat::isSignificandAllOnesExceptLSB() ثابت {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  ثابت integerPart *Parts = significandParts();

  إذا (Parts[0] & 1)
    أرجع خطأ;

  ثابت بیعلامت Part_شمار = part_شمار_For_بیتها(semantics->precision);
  لكل (بیعلامت i = 0; i < Part_شمار - 1; i++) {
    إذا (~Parts[i] & ~بیعلامت{!i})
      أرجع خطأ;
  }

  // Set the unused high bits to all ones when we compare.
  ثابت بیعلامت رقم_بالا_بیتها = get_رقم_بالا_بیتها();
  أكد(رقم_بالا_بیتها <= integer_Part_عرض && رقم_بالا_بیتها > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ثابت integerPart بالا_بیت_Fill = ~integerPart(0)
                                  << (integer_Part_عرض - رقم_بالا_بیتها);
  إذا (~(Parts[Part_شمار - 1] | بالا_بیت_Fill | 0x1))
    أرجع خطأ;

  أرجع درست;
}

منطقي IEEEFloat::isSignificandAllZeros() ثابت {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  ثابت integerPart *Parts = significandParts();
  ثابت بیعلامت Part_شمار = part_شمار_For_بیتها(semantics->precision);

  لكل (بیعلامت i = 0; i < Part_شمار - 1; i++)
    إذا (Parts[i])
      أرجع خطأ;

  // Compute how many bits are used in the final word.
  ثابت بیعلامت رقم_بالا_بیتها = get_رقم_بالا_بیتها();
  أكد(رقم_بالا_بیتها < integer_Part_عرض && "Can not have more high bits to "
         "clear than integerPartWidth");
  ثابت integerPart بالا_بیت_ماسک = ~integerPart(0) >> رقم_بالا_بیتها;

  إذا ((semantics->precision > 1) && (Parts[Part_شمار - 1] & بالا_بیت_ماسک))
    أرجع خطأ;

  أرجع درست;
}

منطقي IEEEFloat::isSignificandAllZerosExceptMSB() ثابت {
  ثابت integerPart *Parts = significandParts();
  ثابت بیعلامت Part_شمار = part_شمار_For_بیتها(semantics->precision);

  لكل (بیعلامت i = 0; i < Part_شمار - 1; i++) {
    إذا (Parts[i])
      أرجع خطأ;
  }

  ثابت بیعلامت رقم_بالا_بیتها = get_رقم_بالا_بیتها();
  ثابت integerPart MSBMask = integerPart(1)
                              << (integer_Part_عرض - رقم_بالا_بیتها);
  أرجع ((semantics->precision <= 1) || (Parts[Part_شمار - 1] == MSBMask));
}

منطقي IEEEFloat::is_بزرگترین() ثابت {
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
  truncated.دور_مقصد_Integral(rmTowardZero);
  أرجع compare(truncated) == cmpEqual;
}

منطقي IEEEFloat::bitwiseIsEqual(ثابت IEEEFloat &rhs) ثابت {
  إذا (هذا == &rhs)
    أرجع درست;
  إذا (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    أرجع خطأ;
  إذا (category==fcZero || category==fcInfinity)
    أرجع درست;

  إذا (isFiniteNonZero() && exponent != rhs.exponent)
    أرجع خطأ;

  أرجع std::equal(significandParts(), significandParts() + part_شمار(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics, integerPart مقدار_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fc_نرمال;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = مقدار_2;
  normalize(rm_نزدیکترین_Ties_مقصد_Even, lfExactlyZero);
}

IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(خطأ) : make_کوچکترین_Normalized(خطأ);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics, uninitialized_برچسب برچسب_2)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(ثابت IEEEFloat &rhs) {
  initialize(rhs.semantics);
  تخصیص_بده(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloat_پایه::semBogus) {
  *هذا = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

بیعلامت صحيح IEEEFloat::part_شمار() ثابت {
  أرجع part_شمار_For_بیتها(semantics->precision + 1);
}

ثابت APFloat::integerPart *IEEEFloat::significandParts() ثابت {
  أرجع const_cast<IEEEFloat *>(هذا)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  إذا (part_شمار() > 1)
    أرجع significand.parts;
  وإلا
    أرجع &significand.part;
}

فراغ IEEEFloat::zeroSignificand() {
  APInt::tc_مجموعه(significandParts(), 0, part_شمار());
}

/* Increment an fcNormal floating point number's significand.  */
فراغ IEEEFloat::incrementSignificand() {
  [[maybe_استفادهنشده]] integerPart انتقال =
      APInt::tcIncrement(significandParts(), part_شمار());

  /* Our callers should never cause us to overflow.  */
  أكد(انتقال == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::جمع_کن_Significand(ثابت IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  أكد(semantics == rhs.semantics);
  أكد(exponent == rhs.exponent);

  أرجع APInt::tc_جمع_کن(parts, rhs.significandParts(), 0, part_شمار());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::تفریق_Significand(ثابت IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  أكد(semantics == rhs.semantics);
  أكد(exponent == rhs.exponent);

  أرجع APInt::tc_تفریق(parts, rhs.significandParts(), borrow,
                           part_شمار());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
گمشده_Fraction IEEEFloat::multiplySignificand(ثابت IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            منطقي ignoreAddend) {
  integerPart scratch[4];
  منطقي ignored;

  أكد(semantics == rhs.semantics);

  بیعلامت precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  بیعلامت new_Parts_شمار = part_شمار_For_بیتها(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *پر_Significand =
      new_Parts_شمار > 4 ? جديد integerPart[new_Parts_شمار] : scratch;

  integerPart *lhsSignificand = significandParts();
  بیعلامت parts_شمار = part_شمار();

  APInt::tc_پر_Multiply(پر_Significand, lhsSignificand,
                        rhs.significandParts(), parts_شمار, parts_شمار);

  گمشده_Fraction گمشده_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  بیعلامت omsb = APInt::tcMSB(پر_Significand, new_Parts_شمار) + 1;
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
    بیعلامت extendedPrecision = 2 * precision + 1;
    إذا (omsb != extendedPrecision - 1) {
      أكد(extendedPrecision > omsb);
      APInt::tc_لغزش_چپ(پر_Significand, new_Parts_شمار,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    إذا (new_Parts_شمار == 1)
      significand.part = پر_Significand[0];
    وإلا
      significand.parts = پر_Significand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_استفادهنشده]] op_وضعیت وضعیت_2 = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    أكد(وضعیت_2 == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    گمشده_fraction = extendedAddend.لغزش_Significand_راست(1);
    أكد(گمشده_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    گمشده_fraction = جمع_کن_Or_تفریق_Significand(extendedAddend, خطأ);

    /* Restore our state.  */
    إذا (new_Parts_شمار == 1)
      پر_Significand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(پر_Significand, new_Parts_شمار) + 1;
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
    بیعلامت صحيح بیتها, significantParts;
    گمشده_Fraction lf;

    بیتها = omsb - precision;
    significantParts = part_شمار_For_بیتها(omsb);
    lf = لغزش_راست(پر_Significand, significantParts, بیتها);
    گمشده_fraction = combine_گمشده_Fractions(lf, گمشده_fraction);
    exponent += بیتها;
  }

  APInt::tc_تخصیص_بده(lhsSignificand, پر_Significand, parts_شمار);

  إذا (new_Parts_شمار > 4)
    حذف [] پر_Significand;

  أرجع گمشده_fraction;
}

گمشده_Fraction IEEEFloat::multiplySignificand(ثابت IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  أرجع multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
گمشده_Fraction IEEEFloat::divideSignificand(ثابت IEEEFloat &rhs) {
  integerPart scratch[4];

  أكد(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  ثابت integerPart *rhsSignificand = rhs.significandParts();
  بیعلامت parts_شمار = part_شمار();

  integerPart *dividend =
      parts_شمار > 2 ? جديد integerPart[parts_شمار * 2] : scratch;
  integerPart *مقسوم_علیه = dividend + parts_شمار;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  لكل (بیعلامت i = 0; i < parts_شمار; i++) {
    dividend[i] = lhsSignificand[i];
    مقسوم_علیه[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  بیعلامت صحيح precision = semantics->precision;

  /* Normalize the divisor.  */
  بیعلامت بیت = precision - APInt::tcMSB(مقسوم_علیه, parts_شمار) - 1;
  إذا (بیت) {
    exponent += بیت;
    APInt::tc_لغزش_چپ(مقسوم_علیه, parts_شمار, بیت);
  }

  /* Normalize the dividend.  */
  بیت = precision - APInt::tcMSB(dividend, parts_شمار) - 1;
  إذا (بیت) {
    exponent -= بیت;
    APInt::tc_لغزش_چپ(dividend, parts_شمار, بیت);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  إذا (APInt::tcCompare(dividend, مقسوم_علیه, parts_شمار) < 0) {
    exponent--;
    APInt::tc_لغزش_چپ(dividend, parts_شمار, 1);
    أكد(APInt::tcCompare(dividend, مقسوم_علیه, parts_شمار) >= 0);
  }

  /* Long division.  */
  لكل (بیت = precision; بیت; بیت -= 1) {
    إذا (APInt::tcCompare(dividend, مقسوم_علیه, parts_شمار) >= 0) {
      APInt::tc_تفریق(dividend, مقسوم_علیه, 0, parts_شمار);
      APInt::tc_مجموعه_بیت(lhsSignificand, بیت - 1);
    }

    APInt::tc_لغزش_چپ(dividend, parts_شمار, 1);
  }

  /* Figure out the lost fraction.  */
  صحيح cmp = APInt::tcCompare(dividend, مقسوم_علیه, parts_شمار);

  گمشده_Fraction گمشده_fraction;
  إذا (cmp > 0)
    گمشده_fraction = lf_بیشتر_Than_Half;
  وإلا إذا (cmp == 0)
    گمشده_fraction = lfExactlyHalf;
  وإلا إذا (APInt::tcIsZero(dividend, parts_شمار))
    گمشده_fraction = lfExactlyZero;
  وإلا
    گمشده_fraction = lfLessThanHalf;

  إذا (parts_شمار > 2)
    حذف [] dividend;

  أرجع گمشده_fraction;
}

بیعلامت صحيح IEEEFloat::significandMSB() ثابت {
  أرجع APInt::tcMSB(significandParts(), part_شمار());
}

بیعلامت صحيح IEEEFloat::significandLSB() ثابت {
  أرجع APInt::tcLSB(significandParts(), part_شمار());
}

/* Note that a zero result is NOT normalized to fcZero.  */
گمشده_Fraction IEEEFloat::لغزش_Significand_راست(بیعلامت صحيح بیتها) {
  /* Our exponent should not overflow.  */
  أكد((ExponentType) (exponent + بیتها) >= exponent);

  exponent += بیتها;

  أرجع لغزش_راست(significandParts(), part_شمار(), بیتها);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
فراغ IEEEFloat::لغزش_Significand_چپ(بیعلامت صحيح بیتها) {
  أكد(بیتها < semantics->precision ||
         (semantics->precision == 1 && بیتها <= 1));

  إذا (بیتها) {
    بیعلامت صحيح parts_شمار = part_شمار();

    APInt::tc_لغزش_چپ(significandParts(), parts_شمار, بیتها);
    exponent -= بیتها;

    أكد(!APInt::tcIsZero(significandParts(), parts_شمار));
  }
}

APFloat::cmp_نتیجه IEEEFloat::compare_Absolute_مقدار(ثابت IEEEFloat &rhs) ثابت {
  أكد(semantics == rhs.semantics);
  أكد(isFiniteNonZero());
  أكد(rhs.isFiniteNonZero());

  صحيح compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  إذا (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_شمار());

  إذا (compare > 0)
    أرجع cmpGreaterThan;
  وإلا إذا (compare < 0)
    أرجع cmpLessThan;
  وإلا
    أرجع cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
ساكن فراغ tc_مجموعه_کمترین_Significant_بیتها(APInt::کلمه_Type *dst, بیعلامت parts,
                                      بیعلامت بیتها) {
  بیعلامت i = 0;
  طالما (بیتها > APInt::APINT_بیتها_در_هر_کلمه) {
    dst[i++] = ~(APInt::کلمه_Type)0;
    بیتها -= APInt::APINT_بیتها_در_هر_کلمه;
  }

  إذا (بیتها)
    dst[i++] = ~(APInt::کلمه_Type)0 >> (APInt::APINT_بیتها_در_هر_کلمه - بیتها);

  طالما (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::op_وضعیت IEEEFloat::handle_سرریز(rounding_نما rounding_نما_2) {
  إذا (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    إذا (rounding_نما_2 == rm_نزدیکترین_Ties_مقصد_Even ||
        rounding_نما_2 == rm_نزدیکترین_Ties_مقصد_Away ||
        (rounding_نما_2 == rm_Toward_مثبت && !sign) ||
        (rounding_نما_2 == rmTowardNegative && sign)) {
      إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(خطأ, sign);
      وإلا
        category = fcInfinity;
      أرجع static_cast<op_وضعیت>(op_سرریز | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fc_نرمال;
  exponent = semantics->maxExponent;
  tc_مجموعه_کمترین_Significant_بیتها(significandParts(), part_شمار(),
                            semantics->precision);
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tc_Clear_بیت(significandParts(), 0);

  أرجع opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
منطقي IEEEFloat::دور_Away_From_Zero(rounding_نما rounding_نما_2,
                                  گمشده_Fraction گمشده_fraction,
                                  بیعلامت صحيح بیت) ثابت {
  /* NaNs and infinities should not have lost fractions.  */
  أكد(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  أكد(گمشده_fraction != lfExactlyZero);

  بدل (rounding_نما_2) {
  حالة rm_نزدیکترین_Ties_مقصد_Away:
    أرجع گمشده_fraction == lfExactlyHalf || گمشده_fraction == lf_بیشتر_Than_Half;

  حالة rm_نزدیکترین_Ties_مقصد_Even:
    إذا (گمشده_fraction == lf_بیشتر_Than_Half)
      أرجع درست;

    /* Our zeroes don't have a significand to test.  */
    إذا (گمشده_fraction == lfExactlyHalf && category != fcZero)
      أرجع APInt::tc_استخراج_بیت(significandParts(), بیت);

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

APFloat::op_وضعیت IEEEFloat::normalize(rounding_نما rounding_نما_2,
                                       گمشده_Fraction گمشده_fraction) {
  إذا (!isFiniteNonZero())
    أرجع opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  بیعلامت omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  إذا (omsb || گمشده_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    صحيح exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    إذا (exponent + exponentChange > semantics->maxExponent)
      أرجع handle_سرریز(rounding_نما_2);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    إذا (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    إذا (exponentChange < 0) {
      أكد(گمشده_fraction == lfExactlyZero);

      لغزش_Significand_چپ(-exponentChange);

      أرجع opOK;
    }

    إذا (exponentChange > 0) {
      گمشده_Fraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = لغزش_Significand_راست(exponentChange);

      گمشده_fraction = combine_گمشده_Fractions(lf, گمشده_fraction);

      /* Keep OMSB up-to-date.  */
      إذا (omsb > (بیعلامت) exponentChange)
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
    أرجع handle_سرریز(rounding_نما_2);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  إذا (گمشده_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    إذا (omsb == 0) {
      category = fcZero;
      إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = خطأ;
      إذا (!semantics->hasZero)
        make_کوچکترین_Normalized(خطأ);
    }

    أرجع opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  إذا (دور_Away_From_Zero(rounding_نما_2, گمشده_fraction, 0)) {
    إذا (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    إذا (omsb == (بیعلامت) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      إذا (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        أرجع handle_سرریز(sign ? rmTowardNegative : rm_Toward_مثبت);

      لغزش_Significand_راست(1);

      أرجع opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      أرجع handle_سرریز(rounding_نما_2);
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
      make_کوچکترین_Normalized(خطأ);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  أرجع (op_وضعیت) (opUnderflow | opInexact);
}

APFloat::op_وضعیت IEEEFloat::جمع_کن_Or_تفریق_Specials(ثابت IEEEFloat &rhs,
                                                   منطقي تفریق) {
  بدل (Pack_Categories_Into_کلید(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(اشاره_تهی);

  حالة Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالة Pack_Categories_Into_کلید(fc_نرمال, fcNaN):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    تخصیص_بده(rhs);
    [[fallthrough]];
  حالة Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالة Pack_Categories_Into_کلید(fcNaN, fc_نرمال):
  حالة Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcNaN, fcNaN):
    إذا (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_کلید(fc_نرمال, fcZero):
  حالة Pack_Categories_Into_کلید(fcInfinity, fc_نرمال):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcZero):
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fc_نرمال, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ تفریق;
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fcZero, fc_نرمال):
    تخصیص_بده(rhs);
    sign = rhs.sign ^ تفریق;
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    إذا (((sign ^ rhs.sign)!=0) != تفریق) {
      makeNaN();
      أرجع opInvalidOp;
    }

    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fc_نرمال, fc_نرمال):
    أرجع opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
گمشده_Fraction IEEEFloat::جمع_کن_Or_تفریق_Significand(ثابت IEEEFloat &rhs,
                                                 منطقي تفریق) {
  [[maybe_استفادهنشده]] integerPart انتقال = 0;
  گمشده_Fraction گمشده_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  تفریق ^= static_cast<منطقي>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  صحيح بیتها = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  إذا (تفریق) {
    إذا ((بیتها < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat موقت_rhs(rhs);
    منطقي گمشده_fraction_is_from_rhs = خطأ;

    إذا (بیتها == 0)
      گمشده_fraction = lfExactlyZero;
    وإلا إذا (بیتها > 0) {
      گمشده_fraction = موقت_rhs.لغزش_Significand_راست(بیتها - 1);
      گمشده_fraction_is_from_rhs = درست;
      لغزش_Significand_چپ(1);
    } وإلا {
      گمشده_fraction = لغزش_Significand_راست(-بیتها - 1);
      موقت_rhs.لغزش_Significand_چپ(1);
    }

    // Should we reverse the subtraction.
    cmp_نتیجه cmp_نتیجه_2 = compare_Absolute_مقدار(موقت_rhs);
    إذا (cmp_نتیجه_2 == cmpLessThan) {
      منطقي borrow =
          گمشده_fraction != lfExactlyZero && !گمشده_fraction_is_from_rhs;
      إذا (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        إذا (گمشده_fraction == lfLessThanHalf)
          گمشده_fraction = lf_بیشتر_Than_Half;
        وإلا إذا (گمشده_fraction == lf_بیشتر_Than_Half)
          گمشده_fraction = lfLessThanHalf;
      }
      انتقال = موقت_rhs.تفریق_Significand(*هذا, borrow);
      copySignificand(موقت_rhs);
      sign = !sign;
    } وإلا إذا (cmp_نتیجه_2 == cmpGreaterThan) {
      منطقي borrow = گمشده_fraction != lfExactlyZero && گمشده_fraction_is_from_rhs;
      إذا (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        إذا (گمشده_fraction == lfLessThanHalf)
          گمشده_fraction = lf_بیشتر_Than_Half;
        وإلا إذا (گمشده_fraction == lf_بیشتر_Than_Half)
          گمشده_fraction = lfLessThanHalf;
      }
      انتقال = تفریق_Significand(موقت_rhs, borrow);
    } وإلا { // cmpEqual
      zeroSignificand();
      إذا (گمشده_fraction != lfExactlyZero && گمشده_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    أكد(!انتقال);
  } وإلا {
    إذا (بیتها > 0) {
      IEEEFloat موقت_rhs(rhs);

      گمشده_fraction = موقت_rhs.لغزش_Significand_راست(بیتها);
      انتقال = جمع_کن_Significand(موقت_rhs);
    } وإلا {
      گمشده_fraction = لغزش_Significand_راست(-بیتها);
      انتقال = جمع_کن_Significand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    أكد(!انتقال);
  }

  أرجع گمشده_fraction;
}

APFloat::op_وضعیت IEEEFloat::multiplySpecials(ثابت IEEEFloat &rhs) {
  بدل (Pack_Categories_Into_کلید(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(اشاره_تهی);

  حالة Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالة Pack_Categories_Into_کلید(fc_نرمال, fcNaN):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    تخصیص_بده(rhs);
    sign = خطأ;
    [[fallthrough]];
  حالة Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالة Pack_Categories_Into_کلید(fcNaN, fc_نرمال):
  حالة Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    إذا (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_کلید(fc_نرمال, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcInfinity, fc_نرمال):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
    category = fcInfinity;
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fcZero, fc_نرمال):
  حالة Pack_Categories_Into_کلید(fc_نرمال, fcZero):
  حالة Pack_Categories_Into_کلید(fcZero, fcZero):
    category = fcZero;
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcZero):
    makeNaN();
    أرجع opInvalidOp;

  حالة Pack_Categories_Into_کلید(fc_نرمال, fc_نرمال):
    أرجع opOK;
  }
}

APFloat::op_وضعیت IEEEFloat::divideSpecials(ثابت IEEEFloat &rhs) {
  بدل (Pack_Categories_Into_کلید(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(اشاره_تهی);

  حالة Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالة Pack_Categories_Into_کلید(fc_نرمال, fcNaN):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    تخصیص_بده(rhs);
    sign = خطأ;
    [[fallthrough]];
  حالة Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالة Pack_Categories_Into_کلید(fcNaN, fc_نرمال):
  حالة Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    إذا (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_کلید(fcInfinity, fcZero):
  حالة Pack_Categories_Into_کلید(fcInfinity, fc_نرمال):
  حالة Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fc_نرمال):
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fc_نرمال, fcInfinity):
    category = fcZero;
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fc_نرمال, fcZero):
    إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(خطأ, sign);
    وإلا
      category = fcInfinity;
    أرجع opDivByZero;

  حالة Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fcZero):
    makeNaN();
    أرجع opInvalidOp;

  حالة Pack_Categories_Into_کلید(fc_نرمال, fc_نرمال):
    أرجع opOK;
  }
}

APFloat::op_وضعیت IEEEFloat::modSpecials(ثابت IEEEFloat &rhs) {
  بدل (Pack_Categories_Into_کلید(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(اشاره_تهی);

  حالة Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالة Pack_Categories_Into_کلید(fc_نرمال, fcNaN):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    تخصیص_بده(rhs);
    [[fallthrough]];
  حالة Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالة Pack_Categories_Into_کلید(fcNaN, fc_نرمال):
  حالة Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcNaN, fcNaN):
    إذا (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fc_نرمال):
  حالة Pack_Categories_Into_کلید(fc_نرمال, fcInfinity):
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fc_نرمال, fcZero):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcZero):
  حالة Pack_Categories_Into_کلید(fcInfinity, fc_نرمال):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fcZero):
    makeNaN();
    أرجع opInvalidOp;

  حالة Pack_Categories_Into_کلید(fc_نرمال, fc_نرمال):
    أرجع opOK;
  }
}

APFloat::op_وضعیت IEEEFloat::remainderSpecials(ثابت IEEEFloat &rhs) {
  بدل (Pack_Categories_Into_کلید(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(اشاره_تهی);

  حالة Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالة Pack_Categories_Into_کلید(fc_نرمال, fcNaN):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    تخصیص_بده(rhs);
    [[fallthrough]];
  حالة Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالة Pack_Categories_Into_کلید(fcNaN, fc_نرمال):
  حالة Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcNaN, fcNaN):
    إذا (isSignaling()) {
      makeQuiet();
      أرجع opInvalidOp;
    }
    أرجع rhs.isSignaling() ? opInvalidOp : opOK;

  حالة Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fc_نرمال):
  حالة Pack_Categories_Into_کلید(fc_نرمال, fcInfinity):
    أرجع opOK;

  حالة Pack_Categories_Into_کلید(fc_نرمال, fcZero):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcZero):
  حالة Pack_Categories_Into_کلید(fcInfinity, fc_نرمال):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fcZero):
    makeNaN();
    أرجع opInvalidOp;

  حالة Pack_Categories_Into_کلید(fc_نرمال, fc_نرمال):
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
APFloat::op_وضعیت IEEEFloat::جمع_کن_Or_تفریق(ثابت IEEEFloat &rhs,
                                           rounding_نما rounding_نما_2,
                                           منطقي تفریق) {
  op_وضعیت fs = جمع_کن_Or_تفریق_Specials(rhs, تفریق);

  /* This return code means it was not a simple case.  */
  إذا (fs == opDivByZero) {
    گمشده_Fraction گمشده_fraction;

    گمشده_fraction = جمع_کن_Or_تفریق_Significand(rhs, تفریق);
    fs = normalize(rounding_نما_2, گمشده_fraction);

    /* Can only be zero if we lost no fraction.  */
    أكد(category != fcZero || گمشده_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  إذا (category == fcZero) {
    إذا (rhs.category != fcZero || (sign == rhs.sign) == تفریق)
      sign = (rounding_نما_2 == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = خطأ;
  }

  أرجع fs;
}

/* Normalized addition.  */
APFloat::op_وضعیت IEEEFloat::جمع_کن(ثابت IEEEFloat &rhs,
                                 rounding_نما rounding_نما_2) {
  أرجع جمع_کن_Or_تفریق(rhs, rounding_نما_2, خطأ);
}

/* Normalized subtraction.  */
APFloat::op_وضعیت IEEEFloat::تفریق(ثابت IEEEFloat &rhs,
                                      rounding_نما rounding_نما_2) {
  أرجع جمع_کن_Or_تفریق(rhs, rounding_نما_2, درست);
}

/* Normalized multiply.  */
APFloat::op_وضعیت IEEEFloat::multiply(ثابت IEEEFloat &rhs,
                                      rounding_نما rounding_نما_2) {
  sign ^= rhs.sign;
  op_وضعیت fs = multiplySpecials(rhs);

  إذا (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = خطأ;
  إذا (isFiniteNonZero()) {
    گمشده_Fraction گمشده_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_نما_2, گمشده_fraction);
    إذا (گمشده_fraction != lfExactlyZero)
      fs = (op_وضعیت) (fs | opInexact);
  }

  أرجع fs;
}

/* Normalized divide.  */
APFloat::op_وضعیت IEEEFloat::divide(ثابت IEEEFloat &rhs,
                                    rounding_نما rounding_نما_2) {
  sign ^= rhs.sign;
  op_وضعیت fs = divideSpecials(rhs);

  إذا (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = خطأ;
  إذا (isFiniteNonZero()) {
    گمشده_Fraction گمشده_fraction = divideSignificand(rhs);
    fs = normalize(rounding_نما_2, گمشده_fraction);
    إذا (گمشده_fraction != lfExactlyZero)
      fs = (op_وضعیت) (fs | opInexact);
  }

  أرجع fs;
}

/* Normalized remainder.  */
APFloat::op_وضعیت IEEEFloat::remainder(ثابت IEEEFloat &rhs) {
  بیعلامت صحيح origSign = sign;

  // First handle the special cases.
  op_وضعیت fs = remainderSpecials(rhs);
  إذا (fs != opDivByZero)
    أرجع fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  إذا (P2.جمع_کن(rhs, rm_نزدیکترین_Ties_مقصد_Even) == opOK) {
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
  fs = VEx.convert(extendedSemantics, rm_نزدیکترین_Ties_مقصد_Even, &losesInfo);
  أكد(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rm_نزدیکترین_Ties_مقصد_Even, &losesInfo);
  أكد(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.جمع_کن(VEx, rm_نزدیکترین_Ties_مقصد_Even);
  أكد(fs == opOK);

  إذا (VEx.compare(PEx) == cmpGreaterThan) {
    fs = تفریق(P, rm_نزدیکترین_Ties_مقصد_Even);
    أكد(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.تفریق(PEx, rm_نزدیکترین_Ties_مقصد_Even);
    أكد(fs == opOK);
    fs = VEx.تفریق(PEx, rm_نزدیکترین_Ties_مقصد_Even);
    أكد(fs == opOK);

    cmp_نتیجه نتیجه_2 = VEx.compare(PEx);
    إذا (نتیجه_2 == cmpGreaterThan || نتیجه_2 == cmpEqual) {
      fs = تفریق(P, rm_نزدیکترین_Ties_مقصد_Even);
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
APFloat::op_وضعیت IEEEFloat::mod(ثابت IEEEFloat &rhs) {
  op_وضعیت fs = modSpecials(rhs);
  بیعلامت صحيح origSign = sign;

  طالما (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_مقدار(rhs) != cmpLessThan) {
    صحيح Exp = ilogb(*هذا) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rm_نزدیکترین_Ties_مقصد_Even);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    إذا (V.isNaN() || compare_Absolute_مقدار(V) == cmpLessThan)
      V = scalbn(rhs, Exp - 1, rm_نزدیکترین_Ties_مقصد_Even);
    V.sign = sign;

    fs = تفریق(V, rm_نزدیکترین_Ties_مقصد_Even);

    // When the semantics supports zero, this loop's
    // exit-condition is handled by the 'isFiniteNonZero'
    // category check above. However, when the semantics
    // does not have 'fcZero' and we have reached the
    // minimum possible value, (and any further subtract
    // will underflow to the same value) explicitly
    // provide an exit-path here.
    إذا (!semantics->hasZero && هذا->is_کوچکترین())
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
APFloat::op_وضعیت IEEEFloat::fused_Multiply_جمع_کن(ثابت IEEEFloat &multiplicand,
                                              ثابت IEEEFloat &addend,
                                              rounding_نما rounding_نما_2) {
  op_وضعیت fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  إذا (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    گمشده_Fraction گمشده_fraction;

    گمشده_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_نما_2, گمشده_fraction);
    إذا (گمشده_fraction != lfExactlyZero)
      fs = (op_وضعیت) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    إذا (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_نما_2 == rmTowardNegative);
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
      fs = جمع_کن_Or_تفریق(addend, rounding_نما_2, خطأ);
  }

  أرجع fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::op_وضعیت IEEEFloat::دور_مقصد_Integral(rounding_نما rounding_نما_2) {
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
  APInt IntegerConstant(بعدی_Power_Of2(APFloat::semanticsPrecision(*semantics)),
                        1);
  IntegerConstant <<= APFloat::semanticsPrecision(*semantics) - 1;
  IEEEFloat MagicConstant(*semantics);
  op_وضعیت fs = MagicConstant.convertFromAPInt(IntegerConstant, خطأ,
                                               rm_نزدیکترین_Ties_مقصد_Even);
  أكد(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  منطقي درون_داد_Sign = isNegative();

  fs = جمع_کن(MagicConstant, rounding_نما_2);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  تفریق(MagicConstant, rounding_نما_2);

  // Restore the input sign.
  إذا (درون_داد_Sign != isNegative())
    changeSign();

  أرجع fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmp_نتیجه IEEEFloat::compare(ثابت IEEEFloat &rhs) ثابت {
  أكد(semantics == rhs.semantics);

  بدل (Pack_Categories_Into_کلید(category, rhs.category)) {
  افتراضي:
    llvm_unreachable(اشاره_تهی);

  حالة Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالة Pack_Categories_Into_کلید(fcNaN, fc_نرمال):
  حالة Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcNaN, fcNaN):
  حالة Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالة Pack_Categories_Into_کلید(fc_نرمال, fcNaN):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    أرجع cmpUnordered;

  حالة Pack_Categories_Into_کلید(fcInfinity, fc_نرمال):
  حالة Pack_Categories_Into_کلید(fcInfinity, fcZero):
  حالة Pack_Categories_Into_کلید(fc_نرمال, fcZero):
    إذا (sign)
      أرجع cmpLessThan;
    وإلا
      أرجع cmpGreaterThan;

  حالة Pack_Categories_Into_کلید(fc_نرمال, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالة Pack_Categories_Into_کلید(fcZero, fc_نرمال):
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

  حالة Pack_Categories_Into_کلید(fc_نرمال, fc_نرمال):
    اكسر;
  }

  cmp_نتیجه نتیجه_2;
  /* Two normal numbers.  Do they have the same sign?  */
  إذا (sign != rhs.sign) {
    إذا (sign)
      نتیجه_2 = cmpLessThan;
    وإلا
      نتیجه_2 = cmpGreaterThan;
  } وإلا {
    /* Compare absolute values; invert result if negative.  */
    نتیجه_2 = compare_Absolute_مقدار(rhs);

    إذا (sign) {
      إذا (نتیجه_2 == cmpLessThan)
        نتیجه_2 = cmpGreaterThan;
      وإلا إذا (نتیجه_2 == cmpGreaterThan)
        نتیجه_2 = cmpLessThan;
    }
  }

  أرجع نتیجه_2;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::op_وضعیت IEEEFloat::convert(ثابت fltSemantics &مقصد_Semantics,
                                     rounding_نما rounding_نما_2,
                                     منطقي *losesInfo) {
  op_وضعیت fs;
  ثابت fltSemantics &fromSemantics = *semantics;
  منطقي is_signaling = isSignaling();

  گمشده_Fraction گمشده_Fraction = lfExactlyZero;
  بیعلامت new_Part_شمار = part_شمار_For_بیتها(مقصد_Semantics.precision + 1);
  بیعلامت old_Part_شمار = part_شمار();
  صحيح لغزش = مقصد_Semantics.precision - fromSemantics.precision;

  منطقي X86SpecialNan = خطأ;
  إذا (&fromSemantics == &APFloat_پایه::semX87DoubleExtended &&
      &مقصد_Semantics != &APFloat_پایه::semX87DoubleExtended && category == fcNaN &&
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
  إذا (لغزش < 0 && isFiniteNonZero()) {
    صحيح omsb = significandMSB() + 1;
    صحيح exponentChange = omsb - fromSemantics.precision;
    إذا (exponent + exponentChange < مقصد_Semantics.minExponent)
      exponentChange = مقصد_Semantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, لغزش);
    إذا (exponentChange < 0) {
      لغزش -= exponentChange;
      exponent += exponentChange;
    } وإلا إذا (omsb <= -لغزش) {
      exponentChange = omsb + لغزش - 1; // leave at least one bit set
      لغزش -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  إذا (لغزش < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    گمشده_Fraction = لغزش_راست(significandParts(), old_Part_شمار, -لغزش);

  // Fix the storage so it can hold to new value.
  إذا (new_Part_شمار > old_Part_شمار) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = جديد integerPart[new_Part_شمار];
    APInt::tc_مجموعه(newParts, 0, new_Part_شمار);
    إذا (isFiniteNonZero() || category==fcNaN)
      APInt::tc_تخصیص_بده(newParts, significandParts(), old_Part_شمار);
    freeSignificand();
    significand.parts = newParts;
  } وإلا إذا (new_Part_شمار == 1 && old_Part_شمار != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    إذا (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &مقصد_Semantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  إذا (لغزش > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_لغزش_چپ(significandParts(), new_Part_شمار, لغزش);

  إذا (isFiniteNonZero()) {
    fs = normalize(rounding_نما_2, گمشده_Fraction);
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

      *losesInfo = گمشده_Fraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      إذا (!X86SpecialNan && semantics == &APFloat_پایه::semX87DoubleExtended)
        APInt::tc_مجموعه_بیت(significandParts(), semantics->precision - 1);

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
    *losesInfo = درست;
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
    *losesInfo = درست;
    إذا (fs == opOK)
      fs = opInexact;
  }

  إذا (category == fcZero && !semantics->hasZero)
    make_کوچکترین_Normalized(خطأ);
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
APFloat::op_وضعیت IEEEFloat::convert_مقصد_Sign_Extended_Integer(
    MutableArrayRef<integerPart> parts, بیعلامت صحيح عرض_2, منطقي isSigned,
    rounding_نما rounding_نما_2, منطقي *isExact) ثابت {
  *isExact = خطأ;

  /* Handle the three special cases first.  */
  إذا (category == fcInfinity || category == fcNaN)
    أرجع opInvalidOp;

  بیعلامت dst_Parts_شمار = part_شمار_For_بیتها(عرض_2);
  أكد(dst_Parts_شمار <= parts.اندازه_2() && "Integer too big");

  إذا (category == fcZero) {
    APInt::tc_مجموعه(parts.داده(), 0, dst_Parts_شمار);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    أرجع opOK;
  }

  ثابت integerPart *src = significandParts();

  بیعلامت truncated_بیتها;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  إذا (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tc_مجموعه(parts.داده(), 0, dst_Parts_شمار);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncated_بیتها = semantics->precision -1U - exponent;
  } وإلا {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    بیعلامت صحيح بیتها = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    إذا (بیتها > عرض_2)
      أرجع opInvalidOp;

    إذا (بیتها < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncated_بیتها = semantics->precision - بیتها;
      APInt::tc_استخراج(parts.داده(), dst_Parts_شمار, src, بیتها, truncated_بیتها);
    } وإلا {
      /* We want at least as many bits as are available.  */
      APInt::tc_استخراج(parts.داده(), dst_Parts_شمار, src, semantics->precision,
                       0);
      APInt::tc_لغزش_چپ(parts.داده(), dst_Parts_شمار,
                         بیتها - semantics->precision);
      truncated_بیتها = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  گمشده_Fraction گمشده_fraction;
  إذا (truncated_بیتها) {
    گمشده_fraction = گمشده_Fraction_Through_Truncation(src, part_شمار(),
                                                  truncated_بیتها);
    إذا (گمشده_fraction != lfExactlyZero &&
        دور_Away_From_Zero(rounding_نما_2, گمشده_fraction, truncated_بیتها)) {
      إذا (APInt::tcIncrement(parts.داده(), dst_Parts_شمار))
        أرجع opInvalidOp;     /* Overflow.  */
    }
  } وإلا {
    گمشده_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  بیعلامت صحيح omsb = APInt::tcMSB(parts.داده(), dst_Parts_شمار) + 1;

  إذا (sign) {
    إذا (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      إذا (omsb != 0)
        أرجع opInvalidOp;
    } وإلا {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      إذا (omsb == عرض_2 &&
          APInt::tcLSB(parts.داده(), dst_Parts_شمار) + 1 != omsb)
        أرجع opInvalidOp;

      /* This case can happen because of rounding.  */
      إذا (omsb > عرض_2)
        أرجع opInvalidOp;
    }

    APInt::tcNegate (parts.داده(), dst_Parts_شمار);
  } وإلا {
    إذا (omsb >= عرض_2 + !isSigned)
      أرجع opInvalidOp;
  }

  إذا (گمشده_fraction == lfExactlyZero) {
    *isExact = درست;
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
APFloat::op_وضعیت
IEEEFloat::convert_مقصد_Integer(MutableArrayRef<integerPart> parts,
                            بیعلامت صحيح عرض_2, منطقي isSigned,
                            rounding_نما rounding_نما_2, منطقي *isExact) ثابت {
  op_وضعیت fs = convert_مقصد_Sign_Extended_Integer(parts, عرض_2, isSigned,
                                             rounding_نما_2, isExact);

  إذا (fs == opInvalidOp) {
    بیعلامت صحيح بیتها, dst_Parts_شمار;

    dst_Parts_شمار = part_شمار_For_بیتها(عرض_2);
    أكد(dst_Parts_شمار <= parts.اندازه_2() && "Integer too big");

    إذا (category == fcNaN)
      بیتها = 0;
    وإلا إذا (sign)
      بیتها = isSigned;
    وإلا
      بیتها = عرض_2 - isSigned;

    tc_مجموعه_کمترین_Significant_بیتها(parts.داده(), dst_Parts_شمار, بیتها);
    إذا (sign && isSigned)
      APInt::tc_لغزش_چپ(parts.داده(), dst_Parts_شمار, عرض_2 - 1);
  }

  أرجع fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::op_وضعیت IEEEFloat::convertFromUnsignedParts(
    ثابت integerPart *src, بیعلامت صحيح src_شمار, rounding_نما rounding_نما_2) {
  category = fc_نرمال;
  بیعلامت omsb = APInt::tcMSB(src, src_شمار) + 1;
  integerPart *dst = significandParts();
  بیعلامت dst_شمار = part_شمار();
  بیعلامت precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  گمشده_Fraction گمشده_fraction;
  إذا (precision <= omsb) {
    exponent = omsb - 1;
    گمشده_fraction = گمشده_Fraction_Through_Truncation(src, src_شمار,
                                                  omsb - precision);
    APInt::tc_استخراج(dst, dst_شمار, src, precision, omsb - precision);
  } وإلا {
    exponent = precision - 1;
    گمشده_fraction = lfExactlyZero;
    APInt::tc_استخراج(dst, dst_شمار, src, omsb, 0);
  }

  أرجع normalize(rounding_نما_2, گمشده_fraction);
}

APFloat::op_وضعیت IEEEFloat::convertFromAPInt(ثابت APInt &Val, منطقي isSigned,
                                              rounding_نما rounding_نما_2) {
  بیعلامت صحيح part_شمار = Val.get_رقم_واژگان();
  APInt api = Val;

  sign = خطأ;
  إذا (isSigned && api.isNegative()) {
    sign = درست;
    api = -api;
  }

  أرجع convertFromUnsignedParts(api.get_Raw_داده(), part_شمار, rounding_نما_2);
}

Expected<APFloat::op_وضعیت>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        rounding_نما rounding_نما_2) {
  گمشده_Fraction گمشده_fraction = lfExactlyZero;

  category = fc_نرمال;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  بیعلامت parts_شمار = part_شمار();
  بیعلامت بیت_Pos = parts_شمار * integer_Part_عرض;
  منطقي computedTrailingFraction = خطأ;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator انتها = s.انتها();
  StringRef::iterator dot;
  خودکار PtrOrErr = skipLeadingZeroesAndAnyDot(begin, انتها, &dot);
  إذا (!PtrOrErr)
    أرجع PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator اولی_Significant_Digit = p;

  طالما (p != انتها) {
    integerPart hex_مقدار;

    إذا (*p == '.') {
      إذا (dot != انتها)
        أرجع createError("String contains multiple dots");
      dot = p++;
      استمر;
    }

    hex_مقدار = hex_Digit_مقدار(*p);
    إذا (hex_مقدار == UINT_MAX)
      اكسر;

    p++;

    // Store the number while we have space.
    إذا (بیت_Pos) {
      بیت_Pos -= 4;
      hex_مقدار <<= بیت_Pos % integer_Part_عرض;
      significand[بیت_Pos / integer_Part_عرض] |= hex_مقدار;
    } وإلا إذا (!computedTrailingFraction) {
      خودکار FractOrErr = trailingHexadecimalFraction(p, انتها, hex_مقدار);
      إذا (!FractOrErr)
        أرجع FractOrErr.takeError();
      گمشده_fraction = *FractOrErr;
      computedTrailingFraction = درست;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  إذا (p == انتها)
    أرجع createError("Hex strings require an exponent");
  إذا (*p != 'p' && *p != 'P')
    أرجع createError("Invalid character in significand");
  إذا (p == begin)
    أرجع createError("Significand has no digits");
  إذا (dot != انتها && p - begin == 1)
    أرجع createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  إذا (p != اولی_Significant_Digit) {
    صحيح expAdjustment;

    /* Implicit hexadecimal point?  */
    إذا (dot == انتها)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<صحيح>(dot - اولی_Significant_Digit);
    إذا (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_شمار * integer_Part_عرض;

    /* Adjust for the given exponent.  */
    خودکار ExpOrErr = مجموع_Exponent(p + 1, انتها, expAdjustment);
    إذا (!ExpOrErr)
      أرجع ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  أرجع normalize(rounding_نما_2, گمشده_fraction);
}

APFloat::op_وضعیت
IEEEFloat::دور_Significand_With_Exponent(ثابت integerPart *decSigParts,
                                        بیعلامت sig_Part_شمار, صحيح نمایی_تابع,
                                        rounding_نما rounding_نما_2) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  بیعلامت صحيح power = نمایی_تابع >= 0 ? نمایی_تابع : -نمایی_تابع;
  بیعلامت صحيح pow5_Part_ظرفیت = power_از_Five_Parts_For_Exponent(power);
  Small_وکتور<integerPart, رقم_Pow5_Parts_On_پشته> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5_Part_ظرفیت);

  منطقي is_نزدیکترین = rounding_نما_2 == rm_نزدیکترین_Ties_مقصد_Even ||
                   rounding_نما_2 == rm_نزدیکترین_Ties_مقصد_Away;

  بیعلامت parts = part_شمار_For_بیتها(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  بیعلامت pow5_Part_شمار = powerOf5(pow5Parts.داده(), power);

  لكل (;; parts *= 2) {
    بیعلامت صحيح excessPrecision, truncated_بیتها;

    calcSemantics.precision = parts * integer_Part_عرض - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncated_بیتها = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    op_وضعیت sig_وضعیت = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_شمار, rm_نزدیکترین_Ties_مقصد_Even);
    op_وضعیت pow_وضعیت = pow5.convertFromUnsignedParts(
        pow5Parts.داده(), pow5_Part_شمار, rm_نزدیکترین_Ties_مقصد_Even);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += نمایی_تابع;

    گمشده_Fraction calc_گمشده_Fraction;
    integerPart HUerr, HUdistance;
    بیعلامت صحيح powHUerr;

    إذا (نمایی_تابع >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calc_گمشده_Fraction = decSig.multiplySignificand(pow5);
      powHUerr = pow_وضعیت != opOK;
    } وإلا {
      calc_گمشده_Fraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      إذا (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncated_بیتها = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (pow_وضعیت == opOK && calc_گمشده_Fraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    أكد(APInt::tc_استخراج_بیت
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calc_گمشده_Fraction != lfExactlyZero, sig_وضعیت != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, is_نزدیکترین);

    /* Are we guaranteed to round correctly if we truncate?  */
    إذا (HUdistance >= HUerr) {
      APInt::tc_استخراج(significandParts(), part_شمار(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calc_گمشده_Fraction = گمشده_Fraction_Through_Truncation(decSig.significandParts(),
                                                       decSig.part_شمار(),
                                                       truncated_بیتها);
      أرجع static_cast<op_وضعیت>(normalize(rounding_نما_2, calc_گمشده_Fraction) |
                                   ((sig_وضعیت | pow_وضعیت) & opInexact));
    }
  }
}

Expected<APFloat::op_وضعیت>
IEEEFloat::convertFromDecimalString(StringRef str, rounding_نما rounding_نما_2) {
  decimalInfo D;
  op_وضعیت fs;

  /* Scan the text.  */
  StringRef::iterator p = str.begin();
  إذا (Error Err = interpretDecimal(p, str.انتها(), &D))
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
  إذا (D.اولی_Sig_Digit == str.انتها() || dec_Digit_مقدار(*D.اولی_Sig_Digit) >= 10U) {
    category = fcZero;
    fs = opOK;
    إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = خطأ;
    إذا (!semantics->hasZero)
      make_کوچکترین_Normalized(خطأ);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } وإلا إذا (D.normalizedExponent - 1 > بیشترین_صحیح / 42039) {
    fs = handle_سرریز(rounding_نما_2);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } وإلا إذا (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (صحيح) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fc_نرمال;
    zeroSignificand();
    fs = normalize(rounding_نما_2, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } وإلا إذا ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handle_سرریز(rounding_نما_2);
  } وإلا {
    integerPart *decSignificand;
    بیعلامت صحيح part_شمار;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_شمار = static_cast<بیعلامت صحيح>(D.آخرین_Sig_Digit - D.اولی_Sig_Digit) + 1;
    part_شمار = part_شمار_For_بیتها(1 + 196 * part_شمار / 59);
    decSignificand = جديد integerPart[part_شمار + 1];
    part_شمار = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    افعل {
      integerPart dec_مقدار, val, multiplier;

      val = 0;
      multiplier = 1;

      افعل {
        إذا (*p == '.') {
          p++;
          إذا (p == str.انتها()) {
            اكسر;
          }
        }
        dec_مقدار = dec_Digit_مقدار(*p++);
        إذا (dec_مقدار >= 10U) {
          حذف[] decSignificand;
          أرجع createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_مقدار;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } طالما (p <= D.آخرین_Sig_Digit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_شمار, part_شمار + 1, خطأ);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      إذا (decSignificand[part_شمار])
        part_شمار++;
    } طالما (p <= D.آخرین_Sig_Digit);

    category = fc_نرمال;
    fs = دور_Significand_With_Exponent(decSignificand, part_شمار,
                                      D.exponent, rounding_نما_2);

    حذف [] decSignificand;
  }

  أرجع fs;
}

منطقي IEEEFloat::convertFromStringSpecials(StringRef str) {
  ثابت size_t MIN_NAME_اندازه = 3;

  إذا (str.اندازه_2() < MIN_NAME_اندازه)
    أرجع خطأ;

  إذا (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(خطأ);
    أرجع درست;
  }

  منطقي IsNegative = str.consume_front("-");
  إذا (IsNegative) {
    إذا (str.اندازه_2() < MIN_NAME_اندازه)
      أرجع خطأ;

    إذا (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(درست);
      أرجع درست;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  منطقي IsSignaling = str.consume_front_insensitive("s");
  إذا (IsSignaling) {
    إذا (str.اندازه_2() < MIN_NAME_اندازه)
      أرجع خطأ;
  }

  إذا (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    إذا (str.خالی()) {
      makeNaN(IsSignaling, IsNegative);
      أرجع درست;
    }

    // Allow the payload to be inside parentheses.
    إذا (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      إذا (str.اندازه_2() <= 2 || str.پشت() != ')')
        أرجع خطأ;

      str = str.برش(1, str.اندازه_2() - 1);
    }

    // Determine the payload number's radix.
    بیعلامت Radix = 10;
    إذا (str[0] == '0') {
      إذا (str.اندازه_2() > 1 && tolower(str[1]) == 'x') {
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
      أرجع درست;
    }
  }

  أرجع خطأ;
}

Expected<APFloat::op_وضعیت>
IEEEFloat::convertFromString(StringRef str, rounding_نما rounding_نما_2) {
  إذا (str.خالی())
    أرجع createError("Invalid string length");

  // Handle special cases.
  إذا (convertFromStringSpecials(str))
    أرجع opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.اندازه_2();
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
                                        rounding_نما_2);
  }

  أرجع convertFromDecimalString(StringRef(p, slen), rounding_نما_2);
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
بیعلامت صحيح IEEEFloat::convert_مقصد_Hex_String(حرف *dst, بیعلامت صحيح hexDigits,
                                           منطقي فوقانی_Case,
                                           rounding_نما rounding_نما_2) ثابت {
  حرف *p = dst;
  إذا (sign)
    *dst++ = '-';

  بدل (category) {
  حالة fcInfinity:
    کپی_حافظه (dst, فوقانی_Case ? infinityU: infinityL, حجم infinityU - 1);
    dst += حجم infinityL - 1;
    اكسر;

  حالة fcNaN:
    کپی_حافظه (dst, فوقانی_Case ? NaNU: NaNL, حجم NaNU - 1);
    dst += حجم NaNU - 1;
    اكسر;

  حالة fcZero:
    *dst++ = '0';
    *dst++ = فوقانی_Case ? 'X': 'x';
    *dst++ = '0';
    إذا (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = فوقانی_Case ? 'P': 'p';
    *dst++ = '0';
    اكسر;

  حالة fc_نرمال:
    dst = convert_نرمال_مقصد_Hex_String (dst, hexDigits, فوقانی_Case, rounding_نما_2);
    اكسر;
  }

  *dst = 0;

  أرجع static_cast<بیعلامت صحيح>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
حرف *IEEEFloat::convert_نرمال_مقصد_Hex_String(حرف *dst, بیعلامت صحيح hexDigits,
                                          منطقي فوقانی_Case,
                                          rounding_نما rounding_نما_2) ثابت {
  *dst++ = '0';
  *dst++ = فوقانی_Case ? 'X': 'x';

  منطقي دور_به_بالا = خطأ;
  ثابت حرف *hexDigitChars = فوقانی_Case ? hex_Digits_فوقانی : hex_Digits_پایینی;

  ثابت integerPart *significand = significandParts();
  بیعلامت parts_شمار = part_شمار();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  بیعلامت مقدار_بیتها = semantics->precision + 3;
  بیعلامت لغزش = integer_Part_عرض - مقدار_بیتها % integer_Part_عرض;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  بیعلامت outputDigits = (مقدار_بیتها - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  إذا (hexDigits) {
    إذا (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      بیعلامت صحيح بیتها;
      گمشده_Fraction fraction;

      بیتها = مقدار_بیتها - hexDigits * 4;
      fraction = گمشده_Fraction_Through_Truncation (significand, parts_شمار, بیتها);
      دور_به_بالا = دور_Away_From_Zero(rounding_نما_2, fraction, بیتها);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  حرف *p = ++dst;

  بیعلامت شمار = (مقدار_بیتها + integer_Part_عرض - 1) / integer_Part_عرض;

  طالما (outputDigits && شمار) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    إذا (--شمار == parts_شمار)
      part = 0;  /* An imaginary higher zero part.  */
    وإلا
      part = significand[شمار] << لغزش;

    إذا (شمار && لغزش)
      part |= significand[شمار - 1] >> (integer_Part_عرض - لغزش);

    /* Convert as much of "part" to hexdigits as we can.  */
    بیعلامت صحيح curDigits = integer_Part_عرض / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  إذا (دور_به_بالا) {
    حرف *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    افعل {
      q--;
      *q = hexDigitChars[hex_Digit_مقدار (*q) + 1];
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
  *dst++ = فوقانی_Case ? 'P': 'p';

  أرجع writeSignedDecimal (dst, exponent);
}

hash_کد hash_مقدار(ثابت IEEEFloat &Arg) {
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
                        Arg.significandParts() + Arg.part_شمار()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convert_F80_Long_Double_APFloat_مقصد_APInt() ثابت {
  أكد(part_شمار() == 2);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semX87DoubleExtended>();
}

APInt IEEEFloat::convert_PPCDouble_Double_Legacy_APFloat_مقصد_APInt() ثابت {
  أكد(semantics ==
         (ثابت llvm::fltSemantics *)&APFloat_پایه::semPPCDoubleDoubleLegacy);
  أكد(part_شمار()==2);

  uint64_t واژگان[2];
  منطقي losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloat_پایه::semIEEEdouble.minExponent;
  IEEEFloat extended(*هذا);
  [[maybe_استفادهنشده]] op_وضعیت fs =
      extended.convert(extendedSemantics, rm_نزدیکترین_Ties_مقصد_Even, &losesInfo);
  أكد(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloat_پایه::semIEEEdouble, rm_نزدیکترین_Ties_مقصد_Even, &losesInfo);
  أكد(fs == opOK || fs == opInexact);
  واژگان[0] = *u.convert_Double_APFloat_مقصد_APInt().get_Raw_داده();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  إذا (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rm_نزدیکترین_Ties_مقصد_Even, &losesInfo);
    أكد(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.تفریق(u, rm_نزدیکترین_Ties_مقصد_Even);
    fs = v.convert(APFloat_پایه::semIEEEdouble, rm_نزدیکترین_Ties_مقصد_Even, &losesInfo);
    أكد(fs == opOK && !losesInfo);
    واژگان[1] = *v.convert_Double_APFloat_مقصد_APInt().get_Raw_داده();
  } وإلا {
    واژگان[1] = 0;
  }

  أرجع APInt(128, واژگان);
}

قالب <ثابت fltSemantics &S>
APInt IEEEFloat::convert_IEEEFloat_مقصد_APInt() ثابت {
  أكد(semantics == &S);
  constexpr بیعلامت صحيح trailing_significand_بیتها =
      S.precision - 1 + S.has_Explicit_Integer_بیت;
  constexpr صحيح integer_بیت_part = (S.precision - 1) / integer_Part_عرض;
  constexpr integerPart integer_بیت = integerPart{1}
                                      << ((S.precision - 1) % integer_Part_عرض);
  constexpr uint64_t significand_ماسک = integer_بیت - 1;
  constexpr بیعلامت صحيح exponent_بیتها =
      S.اندازه_درون_بیتها - (S.hasSignedRepr ? 1 : 0) - trailing_significand_بیتها;
  static_assert(exponent_بیتها < 64);
  constexpr uint64_t exponent_ماسک = (uint64_t{1} << exponent_بیتها) - 1;
  constexpr منطقي is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr صحيح bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_شمار_For_بیتها(trailing_significand_بیتها)>
      mysignificand;

  إذا (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.اندازه_2(),
                mysignificand.begin());
    إذا (myexponent == 1 &&
        !(significandParts()[integer_بیت_part] & integer_بیت))
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
    إذا constexpr (S.has_Explicit_Integer_بیت) {
      mysignificand[0] = integerPart{1} << (trailing_significand_بیتها - 1);
    }
  } وإلا {
    أكد(category == fcNaN && "Unknown category!");
    إذا (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.اندازه_2(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.اندازه_درون_بیتها + 63) / 64> واژگان;
  خودکار واژگان_iter =
      std::copy_n(mysignificand.begin(), mysignificand.اندازه_2(), واژگان.begin());
  إذا constexpr (!S.has_Explicit_Integer_بیت) {
    إذا constexpr (significand_ماسک != 0 || trailing_significand_بیتها == 0) {
      // Clear the integer bit.
      واژگان[mysignificand.اندازه_2() - 1] &= significand_ماسک;
    }
  }
  std::fill(واژگان_iter, واژگان.انتها(), uint64_t{0});
  constexpr size_t آخرین_کلمه = واژگان.اندازه_2() - 1;
  إذا constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.اندازه_درون_بیتها - 1) % 64);
    واژگان[آخرین_کلمه] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_ماسک)
                              << (trailing_significand_بیتها % 64);
  واژگان[آخرین_کلمه] |= shifted_exponent;
  إذا constexpr (آخرین_کلمه == 0) {
    أرجع APInt(S.اندازه_درون_بیتها, واژگان[0]);
  }
  أرجع APInt(S.اندازه_درون_بیتها, واژگان);
}

APInt IEEEFloat::convert_Quadruple_APFloat_مقصد_APInt() ثابت {
  أكد(part_شمار() == 2);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semIEEEquad>();
}

APInt IEEEFloat::convert_Double_APFloat_مقصد_APInt() ثابت {
  أكد(part_شمار()==1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semIEEEdouble>();
}

APInt IEEEFloat::convert_Float_APFloat_مقصد_APInt() ثابت {
  أكد(part_شمار()==1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semIEEEsingle>();
}

APInt IEEEFloat::convert_BFloat_APFloat_مقصد_APInt() ثابت {
  أكد(part_شمار() == 1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semBFloat>();
}

APInt IEEEFloat::convert_Half_APFloat_مقصد_APInt() ثابت {
  أكد(part_شمار()==1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::APFloat_پایه::semIEEEhalf>();
}

APInt IEEEFloat::convert_Float8_E5_M2_APFloat_مقصد_APInt() ثابت {
  أكد(part_شمار() == 1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semFloat8E5M2>();
}

APInt IEEEFloat::convert_Float8_E5_M2_FNUZAPFloat_مقصد_APInt() ثابت {
  أكد(part_شمار() == 1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convert_Float8_E4_M3_APFloat_مقصد_APInt() ثابت {
  أكد(part_شمار() == 1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semFloat8E4M3>();
}

APInt IEEEFloat::convert_Float8_E4_M3_FNAPFloat_مقصد_APInt() ثابت {
  أكد(part_شمار() == 1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semFloat8E4M3FN>();
}

APInt IEEEFloat::convert_Float8_E4_M3_FNUZAPFloat_مقصد_APInt() ثابت {
  أكد(part_شمار() == 1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convert_Float8_E4_M3_B11_FNUZAPFloat_مقصد_APInt() ثابت {
  أكد(part_شمار() == 1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convert_Float8_E3_M4_APFloat_مقصد_APInt() ثابت {
  أكد(part_شمار() == 1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semFloat8E3M4>();
}

APInt IEEEFloat::convert_Float_TF32_APFloat_مقصد_APInt() ثابت {
  أكد(part_شمار() == 1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semFloatTF32>();
}

APInt IEEEFloat::convert_Float8_E8_M0_FNUAPFloat_مقصد_APInt() ثابت {
  أكد(part_شمار() == 1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convert_Float8_E5_M3_FNUAPFloat_مقصد_APInt() ثابت {
  أكد(part_شمار() == 1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convert_Float6_E3_M2_FNAPFloat_مقصد_APInt() ثابت {
  أكد(part_شمار() == 1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semFloat6E3M2FN>();
}

APInt IEEEFloat::convert_Float6_E2_M3_FNAPFloat_مقصد_APInt() ثابت {
  أكد(part_شمار() == 1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semFloat6E2M3FN>();
}

APInt IEEEFloat::convert_Float4_E2_M1_FNAPFloat_مقصد_APInt() ثابت {
  أكد(part_شمار() == 1);
  أرجع convert_IEEEFloat_مقصد_APInt<APFloat_پایه::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcast_مقصد_APInt() ثابت {
  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semIEEEhalf)
    أرجع convert_Half_APFloat_مقصد_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semBFloat)
    أرجع convert_BFloat_APFloat_مقصد_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semIEEEsingle)
    أرجع convert_Float_APFloat_مقصد_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semIEEEdouble)
    أرجع convert_Double_APFloat_مقصد_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semIEEEquad)
    أرجع convert_Quadruple_APFloat_مقصد_APInt();

  إذا (semantics ==
      (ثابت llvm::fltSemantics *)&APFloat_پایه::semX87DoubleExtended)
    أرجع convert_F80_Long_Double_APFloat_مقصد_APInt();

  إذا (semantics ==
      (ثابت llvm::fltSemantics *)&APFloat_پایه::semPPCDoubleDoubleLegacy)
    أرجع convert_PPCDouble_Double_Legacy_APFloat_مقصد_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semFloat8E5M2)
    أرجع convert_Float8_E5_M2_APFloat_مقصد_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semFloat8E5M2FNUZ)
    أرجع convert_Float8_E5_M2_FNUZAPFloat_مقصد_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semFloat8E4M3)
    أرجع convert_Float8_E4_M3_APFloat_مقصد_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semFloat8E4M3FN)
    أرجع convert_Float8_E4_M3_FNAPFloat_مقصد_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semFloat8E4M3FNUZ)
    أرجع convert_Float8_E4_M3_FNUZAPFloat_مقصد_APInt();

  إذا (semantics ==
      (ثابت llvm::fltSemantics *)&APFloat_پایه::semFloat8E4M3B11FNUZ)
    أرجع convert_Float8_E4_M3_B11_FNUZAPFloat_مقصد_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semFloat8E3M4)
    أرجع convert_Float8_E3_M4_APFloat_مقصد_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semFloatTF32)
    أرجع convert_Float_TF32_APFloat_مقصد_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semFloat8E8M0FNU)
    أرجع convert_Float8_E8_M0_FNUAPFloat_مقصد_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semFloat8E5M3FNU)
    أرجع convert_Float8_E5_M3_FNUAPFloat_مقصد_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semFloat6E3M2FN)
    أرجع convert_Float6_E3_M2_FNAPFloat_مقصد_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semFloat6E2M3FN)
    أرجع convert_Float6_E2_M3_FNAPFloat_مقصد_APInt();

  إذا (semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semFloat4E2M1FN)
    أرجع convert_Float4_E2_M1_FNAPFloat_مقصد_APInt();

  llvm_unreachable("unknown format!");
}

عائم IEEEFloat::convert_مقصد_Float() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcast_مقصد_APInt();
  أرجع api.بیتها_مقصد_Float();
}

مزدوج IEEEFloat::convert_مقصد_Double() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcast_مقصد_APInt();
  أرجع api.بیتها_مقصد_Double();
}

#اگر_تعریف_شده HAS_IEE754_FLOAT128
float128 IEEEFloat::convert_مقصد_Quad() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloat_پایه::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcast_مقصد_APInt();
  أرجع api.بیتها_مقصد_Quad();
}
#پایان_اگر

فراغ IEEEFloat::initFromF80LongDoubleAPInt(ثابت APInt &api) {
  أرجع initFromIEEEAPInt<APFloat_پایه::semX87DoubleExtended>(api);
}

فراغ IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(ثابت APInt &api) {
  uint64_t i1 = api.get_Raw_داده()[0];
  uint64_t i2 = api.get_Raw_داده()[1];
  منطقي losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_استفادهنشده]] op_وضعیت fs = convert(APFloat_پایه::semPPCDoubleDoubleLegacy,
                                         rm_نزدیکترین_Ties_مقصد_Even, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  أكد((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  إذا (isFiniteNonZero()) {
    IEEEFloat v(APFloat_پایه::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    إذا (v.isFinite()) {
      fs = v.convert(APFloat_پایه::semPPCDoubleDoubleLegacy, rm_نزدیکترین_Ties_مقصد_Even,
                     &losesInfo);
      أكد(fs == opOK && !losesInfo);

      جمع_کن(v, rm_نزدیکترین_Ties_مقصد_Even);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
فراغ IEEEFloat::initFromFloat8E8M0FNUAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semFloat8E8M0FNU>(api);
}

فراغ IEEEFloat::initFromFloat8E5M3FNUAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semFloat8E5M3FNU>(api);
}

قالب <ثابت fltSemantics &S>
فراغ IEEEFloat::initFromIEEEAPInt(ثابت APInt &api) {
  أكد(api.get_بیت_عرض() == S.اندازه_درون_بیتها);

  constexpr بیعلامت صحيح trailing_significand_بیتها =
      S.precision - 1 + S.has_Explicit_Integer_بیت;
  constexpr integerPart integer_بیت =
      integerPart{1} << (trailing_significand_بیتها % integer_Part_عرض);
  constexpr uint64_t significand_ماسک = integer_بیت - 1;
  constexpr بیعلامت صحيح exponent_بیتها =
      S.اندازه_درون_بیتها - (S.hasSignedRepr ? 1 : 0) - trailing_significand_بیتها;
  static_assert(exponent_بیتها < 64);
  constexpr بیعلامت صحيح stored_significand_parts =
      part_شمار_For_بیتها(trailing_significand_بیتها + 1);
  constexpr uint64_t exponent_ماسک = (uint64_t{1} << exponent_بیتها) - 1;
  constexpr منطقي is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr صحيح bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr منطقي has_significand = trailing_significand_بیتها > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  إذا constexpr (has_significand) {
    std::copy_n(api.get_Raw_داده(), mysignificand.اندازه_2(), mysignificand.begin());
    إذا constexpr (significand_ماسک != 0 || S.precision >= integer_Part_عرض) {
      mysignificand[mysignificand.اندازه_2() - 1] &= significand_ماسک;
    }
  } وإلا {
    std::fill_n(mysignificand.begin(), mysignificand.اندازه_2(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t آخرین_کلمه = api.get_Raw_داده()[api.get_رقم_واژگان() - 1];
  uint64_t myexponent =
      (آخرین_کلمه >> (trailing_significand_بیتها % 64)) & exponent_ماسک;

  initialize(&S);
  أكد(part_شمار() == mysignificand.اندازه_2());

  sign = S.hasSignedRepr
             ? static_cast<بیعلامت صحيح>(آخرین_کلمه >> ((S.اندازه_درون_بیتها - 1) % 64))
             : 0;

  منطقي all_zero_significand =
      has_significand && llvm::all_از(mysignificand, equal_مقصد(0));

  منطقي is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  إذا constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    منطقي is_inf = خطأ;

    إذا constexpr (S.has_Explicit_Integer_بیت) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_ماسک_no_int_بیت =
          (uint64_t{1} << (trailing_significand_بیتها - 1)) - 1;
      ثابت integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_بیتها - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_ماسک_no_int_بیت) == 0;
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
    إذا constexpr (S.has_Explicit_Integer_بیت) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      ثابت integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_بیتها - 1);
      constexpr integerPart significand_ماسک_no_int_بیت =
          (uint64_t{1} << (trailing_significand_بیتها - 1)) - 1;

      إذا (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_ماسک_no_int_بیت) != 0) {
        // regular NaN and pseudoNaN
        is_nan = درست;
      } وإلا إذا (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_ماسک_no_int_بیت) == 0) {
        // pseudoinfinity
        is_nan = درست;
      } وإلا إذا (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = درست;
      }
    } وإلا {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } وإلا إذا constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    منطقي all_ones_significand =
        std::all_از(mysignificand.begin(), mysignificand.انتها() - 1,
                    [](integerPart بیتها) { أرجع بیتها == ~integerPart{0}; }) &&
        (!significand_ماسک ||
         mysignificand[mysignificand.اندازه_2() - 1] == significand_ماسک);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } وإلا إذا constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  إذا (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.اندازه_2(),
                significandParts());
    أرجع;
  }

  إذا (is_zero) {
    makeZero(sign);
    أرجع;
  }

  category = fc_نرمال;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.اندازه_2(), significandParts());
  إذا (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  وإلا {
    إذا constexpr (!S.has_Explicit_Integer_بیت) {
      significandParts()[mysignificand.اندازه_2() - 1] |= integer_بیت;
    }
  }
}

فراغ IEEEFloat::initFromQuadrupleAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semIEEEquad>(api);
}

فراغ IEEEFloat::initFromDoubleAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semIEEEdouble>(api);
}

فراغ IEEEFloat::initFromFloatAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semIEEEsingle>(api);
}

فراغ IEEEFloat::initFromBFloatAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semBFloat>(api);
}

فراغ IEEEFloat::initFromHalfAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semIEEEhalf>(api);
}

فراغ IEEEFloat::initFromFloat8E5M2APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semFloat8E5M2>(api);
}

فراغ IEEEFloat::initFromFloat8E5M2FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semFloat8E5M2FNUZ>(api);
}

فراغ IEEEFloat::initFromFloat8E4M3APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semFloat8E4M3>(api);
}

فراغ IEEEFloat::initFromFloat8E4M3FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semFloat8E4M3FN>(api);
}

فراغ IEEEFloat::initFromFloat8E4M3FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semFloat8E4M3FNUZ>(api);
}

فراغ IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semFloat8E4M3B11FNUZ>(api);
}

فراغ IEEEFloat::initFromFloat8E3M4APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semFloat8E3M4>(api);
}

فراغ IEEEFloat::initFromFloatTF32APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semFloatTF32>(api);
}

فراغ IEEEFloat::initFromFloat6E3M2FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semFloat6E3M2FN>(api);
}

فراغ IEEEFloat::initFromFloat6E2M3FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semFloat6E2M3FN>(api);
}

فراغ IEEEFloat::initFromFloat4E2M1FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_پایه::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
فراغ IEEEFloat::initFromAPInt(ثابت fltSemantics *Sem, ثابت APInt &api) {
  أكد(api.get_بیت_عرض() == Sem->اندازه_درون_بیتها);
  إذا (Sem == &APFloat_پایه::semIEEEhalf)
    أرجع initFromHalfAPInt(api);
  إذا (Sem == &APFloat_پایه::semBFloat)
    أرجع initFromBFloatAPInt(api);
  إذا (Sem == &APFloat_پایه::semIEEEsingle)
    أرجع initFromFloatAPInt(api);
  إذا (Sem == &APFloat_پایه::semIEEEdouble)
    أرجع initFromDoubleAPInt(api);
  إذا (Sem == &APFloat_پایه::semIEEEquad)
    أرجع initFromQuadrupleAPInt(api);
  إذا (Sem == &APFloat_پایه::semX87DoubleExtended)
    أرجع initFromF80LongDoubleAPInt(api);
  إذا (Sem == &APFloat_پایه::semPPCDoubleDoubleLegacy)
    أرجع initFromPPCDoubleDoubleLegacyAPInt(api);
  إذا (Sem == &APFloat_پایه::semFloat8E5M2)
    أرجع initFromFloat8E5M2APInt(api);
  إذا (Sem == &APFloat_پایه::semFloat8E5M2FNUZ)
    أرجع initFromFloat8E5M2FNUZAPInt(api);
  إذا (Sem == &APFloat_پایه::semFloat8E4M3)
    أرجع initFromFloat8E4M3APInt(api);
  إذا (Sem == &APFloat_پایه::semFloat8E4M3FN)
    أرجع initFromFloat8E4M3FNAPInt(api);
  إذا (Sem == &APFloat_پایه::semFloat8E4M3FNUZ)
    أرجع initFromFloat8E4M3FNUZAPInt(api);
  إذا (Sem == &APFloat_پایه::semFloat8E4M3B11FNUZ)
    أرجع initFromFloat8E4M3B11FNUZAPInt(api);
  إذا (Sem == &APFloat_پایه::semFloat8E3M4)
    أرجع initFromFloat8E3M4APInt(api);
  إذا (Sem == &APFloat_پایه::semFloatTF32)
    أرجع initFromFloatTF32APInt(api);
  إذا (Sem == &APFloat_پایه::semFloat8E8M0FNU)
    أرجع initFromFloat8E8M0FNUAPInt(api);
  إذا (Sem == &APFloat_پایه::semFloat8E5M3FNU)
    أرجع initFromFloat8E5M3FNUAPInt(api);
  إذا (Sem == &APFloat_پایه::semFloat6E3M2FN)
    أرجع initFromFloat6E3M2FNAPInt(api);
  إذا (Sem == &APFloat_پایه::semFloat6E2M3FN)
    أرجع initFromFloat6E2M3FNAPInt(api);
  إذا (Sem == &APFloat_پایه::semFloat4E2M1FN)
    أرجع initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
فراغ IEEEFloat::make_بزرگترین(منطقي Negative) {
  إذا (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 1..10
  //   significand = 1..1
  category = fc_نرمال;
  sign = Negative;
  exponent = semantics->maxExponent;

  // Use memset to set all but the highest integerPart to all ones.
  integerPart *significand = significandParts();
  بیعلامت Part_شمار = part_شمار();
  memset(significand, 0xFF, حجم(integerPart)*(Part_شمار - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  ثابت بیعلامت رقم_استفادهنشده_بالا_بیتها =
    Part_شمار*integer_Part_عرض - semantics->precision;
  significand[Part_شمار - 1] = (رقم_استفادهنشده_بالا_بیتها < integer_Part_عرض)
                                   ? (~integerPart(0) >> رقم_استفادهنشده_بالا_بیتها)
                                   : 0;
  إذا (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
فراغ IEEEFloat::make_کوچکترین(منطقي Negative) {
  إذا (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fc_نرمال;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tc_مجموعه(significandParts(), 1, part_شمار());
}

فراغ IEEEFloat::make_کوچکترین_Normalized(منطقي Negative) {
  إذا (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 10..0

  category = fc_نرمال;
  zeroSignificand();
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tc_مجموعه_بیت(significandParts(), semantics->precision - 1);
}

IEEEFloat::IEEEFloat(ثابت fltSemantics &Sem, ثابت APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(عائم f) {
  initFromAPInt(&APFloat_پایه::semIEEEsingle, APInt::float_مقصد_بیتها(f));
}

IEEEFloat::IEEEFloat(مزدوج d) {
  initFromAPInt(&APFloat_پایه::semIEEEdouble, APInt::double_مقصد_بیتها(d));
}

نطاق {
  فراغ append(Small_وکتور_Impl<حرف> &میانگیر, StringRef Str) {
    میانگیر.append(Str.begin(), Str.انتها());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  فراغ Adjust_مقصد_Precision(APInt &significand,
                         صحيح &نمایی_تابع, بیعلامت FormatPrecision) {
    بیعلامت بیتها = significand.get_فعال_بیتها();

    // 196/59 is a very slight overestimate of lg_2(10).
    بیعلامت بیتها_Required = (FormatPrecision * 196 + 58) / 59;

    إذا (بیتها <= بیتها_Required) أرجع;

    بیعلامت tensRemovable = (بیتها - بیتها_Required) * 59 / 196;
    إذا (!tensRemovable) أرجع;

    نمایی_تابع += tensRemovable;

    APInt مقسوم_علیه(significand.get_بیت_عرض(), 1);
    APInt powten(significand.get_بیت_عرض(), 10);
    طالما (درست) {
      إذا (tensRemovable & 1)
        مقسوم_علیه *= powten;
      tensRemovable >>= 1;
      إذا (!tensRemovable) اكسر;
      powten *= powten;
    }

    significand = significand.udiv(مقسوم_علیه);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.get_فعال_بیتها());
  }


  فراغ Adjust_مقصد_Precision(Small_وکتور_Impl<حرف> &میانگیر_2,
                         صحيح &نمایی_تابع, بیعلامت FormatPrecision) {
    بیعلامت N = میانگیر_2.اندازه_2();
    إذا (N <= FormatPrecision) أرجع;

    // The most significant figures are the last ones in the buffer.
    بیعلامت اولی_Significant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    إذا (میانگیر_2[اولی_Significant - 1] < '5') {
      طالما (اولی_Significant < N && میانگیر_2[اولی_Significant] == '0')
        اولی_Significant++;

      نمایی_تابع += اولی_Significant;
      میانگیر_2.erase(&میانگیر_2[0], &میانگیر_2[اولی_Significant]);
      أرجع;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    لكل (بیعلامت I = اولی_Significant; I != N; ++I) {
      إذا (میانگیر_2[I] == '9') {
        اولی_Significant++;
      } وإلا {
        میانگیر_2[I]++;
        اكسر;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    إذا (اولی_Significant == N) {
      نمایی_تابع += اولی_Significant;
      میانگیر_2.clear();
      میانگیر_2.افزودن_پشت('1');
      أرجع;
    }

    نمایی_تابع += اولی_Significant;
    میانگیر_2.erase(&میانگیر_2[0], &میانگیر_2[اولی_Significant]);
  }

  فراغ مقصد_String_Impl(Small_وکتور_Impl<حرف> &Str, ثابت منطقي isNeg, صحيح نمایی_تابع,
                    APInt significand, بیعلامت FormatPrecision,
                    بیعلامت FormatMaxPadding, منطقي TruncateZero) {
    ثابت صحيح semanticsPrecision = significand.get_بیت_عرض();

    إذا (isNeg)
      Str.افزودن_پشت('-');

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
    نمایی_تابع += trailingZeros;
    significand.lshr_درون_قراربده(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    إذا (نمایی_تابع == 0) {
      // Nothing to do.
    } وإلا إذا (نمایی_تابع > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + نمایی_تابع);
      significand <<= نمایی_تابع;
      نمایی_تابع = 0;
    } وإلا { /* exp < 0 */
      صحيح texp = -نمایی_تابع;

      // We transform this using the identity:
      //   (N)(2^-e) == (N)(5^e)(10^-e)
      // This means we have to multiply N (the significand) by 5^e.
      // To avoid overflow, we have to operate on numbers large
      // enough to store N * 5^e:
      //   log2(N * 5^e) == log2(N) + e * log2(5)
      //                 <= semantics->precision + e * 137 / 59
      //   (log_2(5) ~ 2.321928 < 2.322034 ~ 137/59)

      بیعلامت precision = semanticsPrecision + (137 * texp + 136) / 59;

      // Multiply significand by 5^e.
      //   N * 5^0101 == N * 5^(1*1) * 5^(0*2) * 5^(1*4) * 5^(0*8)
      significand = significand.zext(precision);
      APInt five_مقصد_the_i(precision, 5);
      طالما (درست) {
        إذا (texp & 1)
          significand *= five_مقصد_the_i;

        texp >>= 1;
        إذا (!texp)
          اكسر;
        five_مقصد_the_i *= five_مقصد_the_i;
      }
    }

    Adjust_مقصد_Precision(significand, نمایی_تابع, FormatPrecision);

    Small_وکتور<حرف, 256> میانگیر_2;

    // Fill the buffer.
    بیعلامت precision = significand.get_بیت_عرض();
    إذا (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    منطقي درون_Trail = درست;
    طالما (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      بیعلامت d = digit.get_ZExt_مقدار();

      // Drop trailing zeros.
      إذا (درون_Trail && !d)
        نمایی_تابع++;
      وإلا {
        میانگیر_2.افزودن_پشت((حرف) ('0' + d));
        درون_Trail = خطأ;
      }
    }

    أكد(!میانگیر_2.خالی() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    Adjust_مقصد_Precision(میانگیر_2, نمایی_تابع, FormatPrecision);

    بیعلامت NDigits = میانگیر_2.اندازه_2();

    // Check whether we should use scientific notation.
    منطقي FormatScientific;
    إذا (!FormatMaxPadding) {
      FormatScientific = درست;
    } وإلا {
      إذا (نمایی_تابع >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((بیعلامت) نمایی_تابع > FormatMaxPadding ||
                            NDigits + (بیعلامت) نمایی_تابع > FormatPrecision);
      } وإلا {
        // Power of the most significant digit.
        صحيح MSD = نمایی_تابع + (صحيح) (NDigits - 1);
        إذا (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = خطأ;
        } وإلا {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((بیعلامت) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    إذا (FormatScientific) {
      نمایی_تابع += (NDigits - 1);

      Str.افزودن_پشت(میانگیر_2[NDigits-1]);
      Str.افزودن_پشت('.');
      إذا (NDigits == 1 && TruncateZero)
        Str.افزودن_پشت('0');
      وإلا
        لكل (بیعلامت I = 1; I != NDigits; ++I)
          Str.افزودن_پشت(میانگیر_2[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      إذا (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.افزودن_پشت(TruncateZero ? 'E' : 'e');

      Str.افزودن_پشت(نمایی_تابع >= 0 ? '+' : '-');
      إذا (نمایی_تابع < 0)
        نمایی_تابع = -نمایی_تابع;
      Small_وکتور<حرف, 6> expbuf;
      افعل {
        expbuf.افزودن_پشت((حرف) ('0' + (نمایی_تابع % 10)));
        نمایی_تابع /= 10;
      } طالما (نمایی_تابع);
      // Exponent always at least two digits if we do not truncate zeros.
      إذا (!TruncateZero && expbuf.اندازه_2() < 2)
        expbuf.افزودن_پشت('0');
      لكل (بیعلامت I = 0, E = expbuf.اندازه_2(); I != E; ++I)
        Str.افزودن_پشت(expbuf[E-1-I]);
      أرجع;
    }

    // Non-scientific, positive exponents.
    إذا (نمایی_تابع >= 0) {
      لكل (بیعلامت I = 0; I != NDigits; ++I)
        Str.افزودن_پشت(میانگیر_2[NDigits-1-I]);
      لكل (بیعلامت I = 0; I != (بیعلامت) نمایی_تابع; ++I)
        Str.افزودن_پشت('0');
      أرجع;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    صحيح NWholeDigits = نمایی_تابع + (صحيح) NDigits;

    بیعلامت I = 0;
    إذا (NWholeDigits > 0) {
      لكل (; I != (بیعلامت) NWholeDigits; ++I)
        Str.افزودن_پشت(میانگیر_2[NDigits-I-1]);
      Str.افزودن_پشت('.');
    } وإلا {
      بیعلامت NZeros = 1 + (بیعلامت) -NWholeDigits;

      Str.افزودن_پشت('0');
      Str.افزودن_پشت('.');
      لكل (بیعلامت Z = 1; Z != NZeros; ++Z)
        Str.افزودن_پشت('0');
    }

    لكل (; I != NDigits; ++I)
      Str.افزودن_پشت(میانگیر_2[NDigits-I-1]);

  }
} // namespace

فراغ IEEEFloat::مقصد_String(Small_وکتور_Impl<حرف> &Str, بیعلامت FormatPrecision,
                         بیعلامت FormatMaxPadding, منطقي TruncateZero) ثابت {
  بدل (category) {
  حالة fcInfinity:
    إذا (isNegative())
      أرجع append(Str, "-Inf");
    وإلا
      أرجع append(Str, "+Inf");

  حالة fcNaN: أرجع append(Str, "NaN");

  حالة fcZero:
    إذا (isNegative())
      Str.افزودن_پشت('-');

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
      Str.افزودن_پشت('0');
    }
    أرجع;

  حالة fc_نرمال:
    اكسر;
  }

  // Decompose the number into an APInt and an exponent.
  صحيح نمایی_تابع = exponent - ((صحيح) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_شمار_For_بیتها(semantics->precision)));

  مقصد_String_Impl(Str, isNegative(), نمایی_تابع, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

صحيح IEEEFloat::getExactLog2Abs() ثابت {
  إذا (!isFinite() || isZero())
    أرجع INT_MIN;

  ثابت integerPart *Parts = significandParts();
  ثابت صحيح Part_شمار = part_شمار_For_بیتها(semantics->precision);

  صحيح برداشت_شمار = 0;
  لكل (صحيح i = 0; i < Part_شمار; ++i) {
    برداشت_شمار += llvm::popcount(Parts[i]);
    إذا (برداشت_شمار > 1)
      أرجع INT_MIN;
  }

  إذا (exponent != semantics->minExponent)
    أرجع exponent;

  صحيح CountrParts = 0;
  لكل (صحيح i = 0; i < Part_شمار;
       ++i, CountrParts += APInt::APINT_بیتها_در_هر_کلمه) {
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
  أرجع !APInt::tc_استخراج_بیت(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::op_وضعیت IEEEFloat::بعدی(منطقي بعدی_پایین) {
  // If we are performing nextDown, swap sign so we have -x.
  إذا (بعدی_پایین)
    changeSign();

  // Compute nextUp(x)
  op_وضعیت نتیجه_2 = opOK;

  // Handle each float category separately.
  بدل (category) {
  حالة fcInfinity:
    // nextUp(+inf) = +inf
    إذا (!isNegative())
      اكسر;
    // nextUp(-inf) = -getLargest()
    make_بزرگترین(درست);
    اكسر;
  حالة fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    إذا (isSignaling()) {
      نتیجه_2 = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(خطأ, isNegative(), اشاره_تهی);
    }
    اكسر;
  حالة fcZero:
    // nextUp(pm 0) = +getSmallest()
    make_کوچکترین(خطأ);
    اكسر;
  حالة fc_نرمال:
    // nextUp(-getSmallest()) = -0
    إذا (is_کوچکترین() && isNegative()) {
      APInt::tc_مجموعه(significandParts(), 0, part_شمار());
      category = fcZero;
      exponent = 0;
      إذا (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = خطأ;
      إذا (!semantics->hasZero)
        make_کوچکترین_Normalized(خطأ);
      اكسر;
    }

    إذا (is_بزرگترین() && !isNegative()) {
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
        APInt::tc_مجموعه(significandParts(), 0, part_شمار());
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
      APInt::tcDecrement(Parts, part_شمار());

      إذا (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tc_مجموعه_بیت(Parts, semantics->precision - 1);
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
        APInt::tc_مجموعه(Parts, 0, part_شمار());
        APInt::tc_مجموعه_بیت(Parts, semantics->precision - 1);
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
  إذا (بعدی_پایین)
    changeSign();

  أرجع نتیجه_2;
}

APInt IEEEFloat::getNaNPayload() ثابت {
  أكد(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  بیعلامت بیتها_2 = semantics->precision - 1;
  أرجع APInt(بیتها_2, ArrayRef(significandParts(), part_شمار_For_بیتها(بیتها_2)));
}

APFloat_پایه::ExponentType IEEEFloat::exponentNaN() ثابت {
  أرجع ::exponentNaN(*semantics);
}

APFloat_پایه::ExponentType IEEEFloat::exponentInf() ثابت {
  أرجع ::exponentInf(*semantics);
}

APFloat_پایه::ExponentType IEEEFloat::exponentZero() ثابت {
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
  APInt::tc_مجموعه(significandParts(), 0, part_شمار());
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
  APInt::tc_مجموعه(significandParts(), 0, part_شمار());
}

فراغ IEEEFloat::makeQuiet() {
  أكد(isNaN());
  إذا (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tc_مجموعه_بیت(significandParts(), semantics->precision - 2);
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
  صحيح Significand_بیتها = Arg.getSemantics().precision - 1;

  Normalized.exponent += Significand_بیتها;
  Normalized.normalize(APFloat::rm_نزدیکترین_Ties_مقصد_Even, lfExactlyZero);
  أرجع Normalized.exponent - Significand_بیتها;
}

IEEEFloat scalbn(IEEEFloat X, صحيح Exp, rounding_نما Rounding_نما) {
  خودکار MaxExp = X.getSemantics().maxExponent;
  خودکار MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  صحيح Significand_بیتها = X.getSemantics().precision - 1;
  صحيح MaxIncrement = MaxExp - (MinExp - Significand_بیتها) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(Rounding_نما, lfExactlyZero);
  إذا (X.isNaN())
    X.makeQuiet();
  أرجع X;
}

IEEEFloat frexp(ثابت IEEEFloat &Val, صحيح &Exp, rounding_نما RM) {
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
      Floats(جديد APFloat[2]{APFloat(APFloat_پایه::semIEEEdouble),
                            APFloat(APFloat_پایه::semIEEEdouble)}) {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, uninitialized_برچسب)
    : Semantics(&S), Floats(جديد APFloat[2]{
                         APFloat(APFloat_پایه::semIEEEdouble, uninitialized),
                         APFloat(APFloat_پایه::semIEEEdouble, uninitialized)}) {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(جديد APFloat[2]{APFloat(APFloat_پایه::semIEEEdouble, I),
                            APFloat(APFloat_پایه::semIEEEdouble)}) {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, ثابت APInt &I)
    : Semantics(&S),
      Floats(جديد APFloat[2]{
          APFloat(APFloat_پایه::semIEEEdouble, APInt(64, I.get_Raw_داده()[0])),
          APFloat(APFloat_پایه::semIEEEdouble, APInt(64, I.get_Raw_داده()[1]))}) {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, APFloat &&اولی,
                             APFloat &&دومی)
    : Semantics(&S),
      Floats(جديد APFloat[2]{std::move(اولی), std::move(دومی)}) {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble);
  أكد(&Floats[0].getSemantics() == &APFloat_پایه::semIEEEdouble);
  أكد(&Floats[1].getSemantics() == &APFloat_پایه::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? جديد APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : اشاره_تهی) {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloat_پایه::semBogus;
  RHS.Floats = اشاره_تهی;
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::عملگر=(ثابت DoubleAPFloat &RHS) {
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
ساكن std::pair<APFloat, APFloat> fast_Two_جمع(APFloat X, APFloat Y) {
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
APFloat::op_وضعیت DoubleAPFloat::جمع_کن_Impl(ثابت APFloat &a, ثابت APFloat &aa,
                                         ثابت APFloat &c, ثابت APFloat &cc,
                                         rounding_نما RM) {
  صحيح وضعیت = opOK;
  APFloat z = a;
  وضعیت |= z.جمع_کن(c, RM);
  إذا (!z.isFinite()) {
    إذا (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ خطأ);
      أرجع (op_وضعیت)وضعیت;
    }
    وضعیت = opOK;
    خودکار ACompared_مقصد_C = a.compare_Absolute_مقدار(c);
    z = cc;
    وضعیت |= z.جمع_کن(aa, RM);
    إذا (ACompared_مقصد_C == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      وضعیت |= z.جمع_کن(c, RM);
      وضعیت |= z.جمع_کن(a, RM);
    } وإلا {
      // z = cc + aa + a + c;
      وضعیت |= z.جمع_کن(a, RM);
      وضعیت |= z.جمع_کن(c, RM);
    }
    إذا (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ خطأ);
      أرجع (op_وضعیت)وضعیت;
    }
    Floats[0] = z;
    APFloat zz = aa;
    وضعیت |= zz.جمع_کن(cc, RM);
    إذا (ACompared_مقصد_C == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      وضعیت |= Floats[1].تفریق(z, RM);
      وضعیت |= Floats[1].جمع_کن(c, RM);
      وضعیت |= Floats[1].جمع_کن(zz, RM);
    } وإلا {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      وضعیت |= Floats[1].تفریق(z, RM);
      وضعیت |= Floats[1].جمع_کن(a, RM);
      وضعیت |= Floats[1].جمع_کن(zz, RM);
    }
  } وإلا {
    // q = a - z;
    APFloat q = a;
    وضعیت |= q.تفریق(z, RM);

    // zz = q + c + (a - (q + z)) + aa + cc;
    // Compute a - (q + z) as -((q + z) - a) to avoid temporary copies.
    خودکار zz = q;
    وضعیت |= zz.جمع_کن(c, RM);
    وضعیت |= q.جمع_کن(z, RM);
    وضعیت |= q.تفریق(a, RM);
    q.changeSign();
    وضعیت |= zz.جمع_کن(q, RM);
    وضعیت |= zz.جمع_کن(aa, RM);
    وضعیت |= zz.جمع_کن(cc, RM);
    إذا (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ خطأ);
      أرجع opOK;
    }
    Floats[0] = z;
    وضعیت |= Floats[0].جمع_کن(zz, RM);
    إذا (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ خطأ);
      أرجع (op_وضعیت)وضعیت;
    }
    Floats[1] = std::move(z);
    وضعیت |= Floats[1].تفریق(Floats[0], RM);
    وضعیت |= Floats[1].جمع_کن(zz, RM);
  }
  أرجع (op_وضعیت)وضعیت;
}

APFloat::op_وضعیت DoubleAPFloat::جمع_کن_With_Special(ثابت DoubleAPFloat &LHS,
                                                ثابت DoubleAPFloat &RHS,
                                                DoubleAPFloat &بیرون,
                                                rounding_نما RM) {
  إذا (LHS.getCategory() == fcNaN) {
    بیرون = LHS;
    أرجع opOK;
  }
  إذا (RHS.getCategory() == fcNaN) {
    بیرون = RHS;
    أرجع opOK;
  }
  إذا (LHS.getCategory() == fcZero) {
    بیرون = RHS;
    أرجع opOK;
  }
  إذا (RHS.getCategory() == fcZero) {
    بیرون = LHS;
    أرجع opOK;
  }
  إذا (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    بیرون.makeNaN(خطأ, بیرون.isNegative(), اشاره_تهی);
    أرجع opInvalidOp;
  }
  إذا (LHS.getCategory() == fcInfinity) {
    بیرون = LHS;
    أرجع opOK;
  }
  إذا (RHS.getCategory() == fcInfinity) {
    بیرون = RHS;
    أرجع opOK;
  }
  أكد(LHS.getCategory() == fc_نرمال && RHS.getCategory() == fc_نرمال);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  أكد(&A.getSemantics() == &APFloat_پایه::semIEEEdouble);
  أكد(&AA.getSemantics() == &APFloat_پایه::semIEEEdouble);
  أكد(&C.getSemantics() == &APFloat_پایه::semIEEEdouble);
  أكد(&CC.getSemantics() == &APFloat_پایه::semIEEEdouble);
  أكد(&بیرون.Floats[0].getSemantics() == &APFloat_پایه::semIEEEdouble);
  أكد(&بیرون.Floats[1].getSemantics() == &APFloat_پایه::semIEEEdouble);
  أرجع بیرون.جمع_کن_Impl(A, AA, C, CC, RM);
}

APFloat::op_وضعیت DoubleAPFloat::جمع_کن(ثابت DoubleAPFloat &RHS,
                                     rounding_نما RM) {
  أرجع جمع_کن_With_Special(*هذا, RHS, *هذا, RM);
}

APFloat::op_وضعیت DoubleAPFloat::تفریق(ثابت DoubleAPFloat &RHS,
                                          rounding_نما RM) {
  changeSign();
  خودکار Ret = جمع_کن(RHS, RM);
  changeSign();
  أرجع Ret;
}

APFloat::op_وضعیت DoubleAPFloat::multiply(ثابت DoubleAPFloat &RHS,
                                          APFloat::rounding_نما RM) {
  ثابت خودکار &LHS = *هذا;
  خودکار &بیرون = *هذا;
  /* Interesting observation: For special categories, finding the lowest
     common ancestor of the following layered graph gives the correct
     return category:

        NaN
       /   \
     Zero  Inf
       \   /
نرمال

     e.g. NaN * NaN = NaN
          Zero * Inf = NaN
          Normal * Zero = Zero
          Normal * Inf = Inf
  */
  إذا (LHS.getCategory() == fcNaN) {
    بیرون = LHS;
    أرجع opOK;
  }
  إذا (RHS.getCategory() == fcNaN) {
    بیرون = RHS;
    أرجع opOK;
  }
  إذا ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    بیرون.makeNaN(خطأ, خطأ, اشاره_تهی);
    أرجع opOK;
  }
  إذا (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    بیرون = LHS;
    أرجع opOK;
  }
  إذا (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    بیرون = RHS;
    أرجع opOK;
  }
  أكد(LHS.getCategory() == fc_نرمال && RHS.getCategory() == fc_نرمال &&
         "Special cases not handled exhaustively");

  صحيح وضعیت = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  وضعیت |= T.multiply(C, RM);
  إذا (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ خطأ);
    أرجع (op_وضعیت)وضعیت;
  }

  // tau = fmsub(a, c, t), that is -fmadd(-a, c, t).
  APFloat Tau = A;
  T.changeSign();
  وضعیت |= Tau.fused_Multiply_جمع_کن(C, T, RM);
  T.changeSign();
  {
    // v = a * d
    APFloat V = A;
    وضعیت |= V.multiply(D, RM);
    // w = b * c
    APFloat W = B;
    وضعیت |= W.multiply(C, RM);
    وضعیت |= V.جمع_کن(W, RM);
    // tau += v + w
    وضعیت |= Tau.جمع_کن(V, RM);
  }
  // u = t + tau
  APFloat U = T;
  وضعیت |= U.جمع_کن(Tau, RM);

  Floats[0] = U;
  إذا (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ خطأ);
  } وإلا {
    // Floats[1] = (t - u) + tau
    وضعیت |= T.تفریق(U, RM);
    وضعیت |= T.جمع_کن(Tau, RM);
    Floats[1] = std::move(T);
  }
  أرجع (op_وضعیت)وضعیت;
}

APFloat::op_وضعیت DoubleAPFloat::divide(ثابت DoubleAPFloat &RHS,
                                        APFloat::rounding_نما RM) {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_پایه::semPPCDoubleDoubleLegacy, bitcast_مقصد_APInt());
  خودکار Ret = Tmp.divide(
      APFloat(APFloat_پایه::semPPCDoubleDoubleLegacy, RHS.bitcast_مقصد_APInt()), RM);
  *هذا = DoubleAPFloat(APFloat_پایه::semPPCDoubleDouble, Tmp.bitcast_مقصد_APInt());
  أرجع Ret;
}

APFloat::op_وضعیت DoubleAPFloat::remainder(ثابت DoubleAPFloat &RHS) {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_پایه::semPPCDoubleDoubleLegacy, bitcast_مقصد_APInt());
  خودکار Ret = Tmp.remainder(
      APFloat(APFloat_پایه::semPPCDoubleDoubleLegacy, RHS.bitcast_مقصد_APInt()));
  *هذا = DoubleAPFloat(APFloat_پایه::semPPCDoubleDouble, Tmp.bitcast_مقصد_APInt());
  أرجع Ret;
}

APFloat::op_وضعیت DoubleAPFloat::mod(ثابت DoubleAPFloat &RHS) {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_پایه::semPPCDoubleDoubleLegacy, bitcast_مقصد_APInt());
  خودکار Ret = Tmp.mod(
      APFloat(APFloat_پایه::semPPCDoubleDoubleLegacy, RHS.bitcast_مقصد_APInt()));
  *هذا = DoubleAPFloat(APFloat_پایه::semPPCDoubleDouble, Tmp.bitcast_مقصد_APInt());
  أرجع Ret;
}

APFloat::op_وضعیت
DoubleAPFloat::fused_Multiply_جمع_کن(ثابت DoubleAPFloat &Multiplicand,
                                ثابت DoubleAPFloat &Addend,
                                APFloat::rounding_نما RM) {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_پایه::semPPCDoubleDoubleLegacy, bitcast_مقصد_APInt());
  خودکار Ret = Tmp.fused_Multiply_جمع_کن(
      APFloat(APFloat_پایه::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcast_مقصد_APInt()),
      APFloat(APFloat_پایه::semPPCDoubleDoubleLegacy, Addend.bitcast_مقصد_APInt()),
      RM);
  *هذا = DoubleAPFloat(APFloat_پایه::semPPCDoubleDouble, Tmp.bitcast_مقصد_APInt());
  أرجع Ret;
}

APFloat::op_وضعیت DoubleAPFloat::دور_مقصد_Integral(APFloat::rounding_نما RM) {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ثابت APFloat &Hi = get_اولی();
  ثابت APFloat &Lo = get_دومی();

  APFloat RoundedHi = Hi;
  ثابت op_وضعیت Hi_وضعیت = RoundedHi.دور_مقصد_Integral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  إذا (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/خطأ);
    أرجع Hi_وضعیت;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  خودکار دور_مقصد_نزدیکترین_Helper = [](APFloat مقصد_دور, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    ثابت APFloat RoundingError = Rounded - مقصد_دور;
    إذا (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        قدر_مطلق(RoundingError).is_Exactly_مقدار(0.5))
      Rounded.جمع_کن(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rm_نزدیکترین_Ties_مقصد_Even);
    أرجع Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  إذا (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    إذا (RM == rm_نزدیکترین_Ties_مقصد_Away || RM == rm_نزدیکترین_Ties_مقصد_Even)
      RoundedHi = دور_مقصد_نزدیکترین_Helper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/خطأ);
    أرجع Hi_وضعیت;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  Rounding_نما Lo_Rounding_نما;
  إذا (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    Lo_Rounding_نما = isNegative() ? rm_Toward_مثبت : rmTowardNegative;
  وإلا
    Lo_Rounding_نما = RM;

  APFloat RoundedLo = Lo;
  ثابت op_وضعیت Lo_وضعیت = RoundedLo.دور_مقصد_Integral(Lo_Rounding_نما);
  إذا (Lo_Rounding_نما == rm_نزدیکترین_Ties_مقصد_Away)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = دور_مقصد_نزدیکترین_Helper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fast_Two_جمع(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  أرجع Lo_وضعیت;
}

فراغ DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmp_نتیجه
DoubleAPFloat::compare_Absolute_مقدار(ثابت DoubleAPFloat &RHS) ثابت {
  // Compare absolute values of the high parts.
  ثابت cmp_نتیجه HiPartCmp = Floats[0].compare_Absolute_مقدار(RHS.Floats[0]);
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
  ثابت cmp_نتیجه LoPartCmp = Floats[1].compare_Absolute_مقدار(RHS.Floats[1]);

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

فراغ DoubleAPFloat::make_بزرگترین(منطقي Neg) {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloat_پایه::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloat_پایه::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  إذا (Neg)
    changeSign();
}

فراغ DoubleAPFloat::make_کوچکترین(منطقي Neg) {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].make_کوچکترین(Neg);
  Floats[1].makeZero(/* Neg = */ خطأ);
}

فراغ DoubleAPFloat::make_کوچکترین_Normalized(منطقي Neg) {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloat_پایه::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  إذا (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ خطأ);
}

فراغ DoubleAPFloat::makeNaN(منطقي SNaN, منطقي Neg, ثابت APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ خطأ);
}

APFloat::cmp_نتیجه DoubleAPFloat::compare(ثابت DoubleAPFloat &RHS) ثابت {
  خودکار نتیجه = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  إذا (نتیجه == APFloat::cmpEqual)
    أرجع Floats[1].compare(RHS.Floats[1]);
  أرجع نتیجه;
}

منطقي DoubleAPFloat::bitwiseIsEqual(ثابت DoubleAPFloat &RHS) ثابت {
  أرجع Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_کد hash_مقدار(ثابت DoubleAPFloat &Arg) {
  إذا (Arg.Floats)
    أرجع hash_combine(hash_مقدار(Arg.Floats[0]), hash_مقدار(Arg.Floats[1]));
  أرجع hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcast_مقصد_APInt() ثابت {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t داده_2[] = {
      Floats[0].bitcast_مقصد_APInt().get_Raw_داده()[0],
      Floats[1].bitcast_مقصد_APInt().get_Raw_داده()[0],
  };
  أرجع APInt(128, داده_2);
}

Expected<APFloat::op_وضعیت> DoubleAPFloat::convertFromString(StringRef S,
                                                             rounding_نما RM) {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_پایه::semPPCDoubleDoubleLegacy);
  خودکار Ret = Tmp.convertFromString(S, RM);
  *هذا = DoubleAPFloat(APFloat_پایه::semPPCDoubleDouble, Tmp.bitcast_مقصد_APInt());
  أرجع Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::op_وضعیت DoubleAPFloat::بعدی(منطقي بعدی_پایین) {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  إذا (بعدی_پایین) {
    changeSign();
    APFloat::op_وضعیت نتیجه = بعدی(/*nextDown=*/خطأ);
    changeSign();
    أرجع نتیجه;
  }
  بدل (getCategory()) {
  حالة fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    إذا (isNegative())
      make_بزرگترین(درست);
    أرجع opOK;

  حالة fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    إذا (get_اولی().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(خطأ, isNegative(), اشاره_تهی);
      أرجع opInvalidOp;
    }
    أرجع opOK;

  حالة fcZero:
    // nextUp(pm 0) = +getSmallest()
    make_کوچکترین(خطأ);
    أرجع opOK;

  حالة fc_نرمال:
    اكسر;
  }

  ثابت APFloat &HiOld = get_اولی();
  ثابت APFloat &LoOld = get_دومی();

  APFloat بعدی_Lo = LoOld;
  بعدی_Lo.بعدی(/*nextDown=*/خطأ);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  خودکار درون_Lattice = [](ثابت APFloat &Hi, ثابت APFloat &Lo) {
    أرجع Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  إذا (درون_Lattice(HiOld, بعدی_Lo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(بعدی_Lo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat بزرگترین{*Semantics, uninitialized};
    بزرگترین.make_بزرگترین(/*Neg=*/خطأ);
    إذا (compare(بزرگترین) == cmpGreaterThan)
      makeInf(/*Neg=*/خطأ);

    أرجع opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat بعدی_Hi = HiOld;
  بعدی_Hi.بعدی(/*nextDown=*/خطأ);

  // nextUp(getLargest()) == INFINITY
  إذا (بعدی_Hi.isInfinity()) {
    makeInf(/*Neg=*/خطأ);
    أرجع opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  إذا (بعدی_Hi.isZero()) {
    makeZero(/*Neg=*/درست);
    أرجع opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  بعدی_Lo = neg(scalbn(harrisonUlp(بعدی_Hi), -1, rmTowardZero));
  إذا (!درون_Lattice(بعدی_Hi, بعدی_Lo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    بعدی_Lo.بعدی(/*nextDown=*/خطأ);

  Floats[0] = std::move(بعدی_Hi);
  Floats[1] = std::move(بعدی_Lo);

  أرجع opOK;
}

APFloat::op_وضعیت DoubleAPFloat::convert_مقصد_Sign_Extended_Integer(
    MutableArrayRef<integerPart> درون_داد, بیعلامت صحيح عرض, منطقي IsSigned,
    rounding_نما RM, منطقي *IsExact) ثابت {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  إذا (!get_اولی().isFiniteNonZero() || get_دومی().isZero())
    أرجع get_اولی().convert_مقصد_Integer(درون_داد, عرض, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = خطأ;
  DoubleAPFloat Integral = *هذا;
  ثابت op_وضعیت دور_وضعیت = Integral.دور_مقصد_Integral(RM);
  إذا (دور_وضعیت == opInvalidOp)
    أرجع opInvalidOp;
  ثابت APFloat &IntegralHi = Integral.get_اولی();
  ثابت APFloat &IntegralLo = Integral.get_دومی();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  منطقي HiIsExact;
  إذا (IntegralHi.isZero() || IntegralLo.isZero()) {
    ثابت op_وضعیت Hi_وضعیت =
        IntegralHi.convert_مقصد_Integer(درون_داد, عرض, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    أرجع Hi_وضعیت == opOK ? opInexact : Hi_وضعیت;
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
  ثابت بیعلامت مثبت_سرریز_عرض = IsSigned ? عرض - 1 : عرض;
  إذا (HiExactLog2 >= 0 &&
      static_cast<بیعلامت>(HiExactLog2) == مثبت_سرریز_عرض) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    إذا (IntegralHi.isNegative() == IntegralLo.isNegative())
      أرجع opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    ثابت op_وضعیت Lo_وضعیت = IntegralLo.convert_مقصد_Integer(
        درون_داد, عرض, /*IsSigned=*/درست, RM, &LoIsExact);
    إذا (Lo_وضعیت == opInvalidOp)
      أرجع opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    إذا (IsSigned && !IntegralHi.isNegative())
      APInt::tc_Clear_بیت(درون_داد.داده(), مثبت_سرریز_عرض);
    *IsExact = دور_وضعیت == opOK;
    أرجع دور_وضعیت;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  ثابت op_وضعیت Hi_وضعیت = IntegralHi.convert_مقصد_Integer(
      درون_داد, عرض, IsSigned, rmTowardZero, &HiIsExact);
  إذا (Hi_وضعیت == opInvalidOp)
    أرجع Hi_وضعیت;

  // Convert Lo into a temporary integer of the same width.
  APSInt Lo_نتیجه{عرض, /*isUnsigned=*/!IsSigned};
  ثابت op_وضعیت Lo_وضعیت =
      IntegralLo.convert_مقصد_Integer(Lo_نتیجه, rmTowardZero, &LoIsExact);
  إذا (Lo_وضعیت == opInvalidOp)
    أرجع Lo_وضعیت;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tc_جمع_کن(درون_داد.داده(), Lo_نتیجه.get_Raw_داده(), /*carry=*/0, درون_داد.اندازه_2());

  *IsExact = دور_وضعیت == opOK;
  أرجع دور_وضعیت;
}

APFloat::op_وضعیت
DoubleAPFloat::convert_مقصد_Integer(MutableArrayRef<integerPart> درون_داد,
                                بیعلامت صحيح عرض, منطقي IsSigned,
                                rounding_نما RM, منطقي *IsExact) ثابت {
  op_وضعیت FS =
      convert_مقصد_Sign_Extended_Integer(درون_داد, عرض, IsSigned, RM, IsExact);

  إذا (FS == opInvalidOp) {
    ثابت بیعلامت Dst_Parts_شمار = part_شمار_For_بیتها(عرض);
    أكد(Dst_Parts_شمار <= درون_داد.اندازه_2() && "Integer too big");

    بیعلامت بیتها_2;
    إذا (getCategory() == fcNaN)
      بیتها_2 = 0;
    وإلا إذا (isNegative())
      بیتها_2 = IsSigned;
    وإلا
      بیتها_2 = عرض - IsSigned;

    tc_مجموعه_کمترین_Significant_بیتها(درون_داد.داده(), Dst_Parts_شمار, بیتها_2);
    إذا (isNegative() && IsSigned)
      APInt::tc_لغزش_چپ(درون_داد.داده(), Dst_Parts_شمار, عرض - 1);
  }

  أرجع FS;
}

APFloat::op_وضعیت DoubleAPFloat::handle_سرریز(rounding_نما RM) {
  بدل (RM) {
  حالة APFloat::rmTowardZero:
    make_بزرگترین(/*Neg=*/isNegative());
    اكسر;
  حالة APFloat::rmTowardNegative:
    إذا (isNegative())
      makeInf(/*Neg=*/درست);
    وإلا
      make_بزرگترین(/*Neg=*/خطأ);
    اكسر;
  حالة APFloat::rm_Toward_مثبت:
    إذا (isNegative())
      make_بزرگترین(/*Neg=*/درست);
    وإلا
      makeInf(/*Neg=*/خطأ);
    اكسر;
  حالة APFloat::rm_نزدیکترین_Ties_مقصد_Away:
  حالة APFloat::rm_نزدیکترین_Ties_مقصد_Even:
    makeInf(/*Neg=*/isNegative());
    اكسر;
  افتراضي:
    llvm_unreachable("Invalid rounding mode found");
  }
  op_وضعیت S = opInexact;
  إذا (!get_اولی().isFinite())
    S = static_cast<op_وضعیت>(S | op_سرریز);
  أرجع S;
}

APFloat::op_وضعیت DoubleAPFloat::convertFromUnsignedParts(
    ثابت integerPart *Src, بیعلامت صحيح Src_شمار, rounding_نما RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  ثابت بیعلامت SrcMSB = APInt::tcMSB(Src, Src_شمار);
  إذا (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/خطأ);
    أرجع opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  ثابت بیعلامت Src_بیت_عرض = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/Src_بیت_عرض, ArrayRef(Src, Src_شمار)},
                /*isUnsigned=*/درست};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{get_اولی().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/خطأ, rm_نزدیکترین_Ties_مقصد_Even);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  إذا (!Hi.isFinite())
    أرجع handle_سرریز(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  منطقي HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/درست};
  Hi.convert_مقصد_Integer(HiAsInt, rm_نزدیکترین_Ties_مقصد_Even, &HiAsIntIsExact);
  ثابت APInt Error = SrcInt.zext(HiAsInt.get_بیت_عرض()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  rounding_نما LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  إذا (Error.isNegative()) {
    إذا (RM == rm_نزدیکترین_Ties_مقصد_Away) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      ثابت بیعلامت Error_فعال_بیتها = Error.get_Significant_بیتها() - 1;
      ثابت بیعلامت LoPrecision = get_دومی().getSemantics().precision;
      إذا (Error_فعال_بیتها > LoPrecision) {
        ثابت بیعلامت RoundingBoundary = Error_فعال_بیتها - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        إذا (Error.شمار_Trailing_Zeros() == RoundingBoundary - 1)
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

  APFloat Lo{get_دومی().getSemantics()};
  op_وضعیت وضعیت = Lo.convertFromAPInt(Error, /*IsSigned=*/درست, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fast_Two_جمع(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  إذا (!get_اولی().isFinite())
    أرجع handle_سرریز(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  إذا (get_اولی().isFiniteNonZero() && Floats[0].is_بزرگترین()) {
    DoubleAPFloat بزرگترین{*Semantics};
    بزرگترین.make_بزرگترین(/*Neg=*/خطأ);
    إذا (compare(بزرگترین) == APFloat::cmpGreaterThan)
      أرجع handle_سرریز(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  أرجع وضعیت;
}

APFloat::op_وضعیت DoubleAPFloat::convertFromAPInt(ثابت APInt &درون_داد,
                                                  منطقي IsSigned,
                                                  rounding_نما RM) {
  ثابت منطقي Negate_درون_داد = IsSigned && درون_داد.isNegative();
  APInt API = درون_داد;
  إذا (Negate_درون_داد)
    API.negate();

  ثابت APFloat::op_وضعیت وضعیت =
      convertFromUnsignedParts(API.get_Raw_داده(), API.get_رقم_واژگان(), RM);
  إذا (Negate_درون_داد)
    changeSign();
  أرجع وضعیت;
}

بیعلامت صحيح DoubleAPFloat::convert_مقصد_Hex_String(حرف *DST,
                                               بیعلامت صحيح HexDigits,
                                               منطقي فوقانی_Case_2,
                                               rounding_نما RM) ثابت {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble &&
         "Unexpected Semantics");
  أرجع APFloat(APFloat_پایه::semPPCDoubleDoubleLegacy, bitcast_مقصد_APInt())
      .convert_مقصد_Hex_String(DST, HexDigits, فوقانی_Case_2, RM);
}

منطقي DoubleAPFloat::isDenormal() ثابت {
  أرجع getCategory() == fc_نرمال &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

منطقي DoubleAPFloat::is_کوچکترین() ثابت {
  إذا (getCategory() != fc_نرمال)
    أرجع خطأ;
  DoubleAPFloat Tmp(*هذا);
  Tmp.make_کوچکترین(هذا->isNegative());
  أرجع Tmp.compare(*هذا) == cmpEqual;
}

منطقي DoubleAPFloat::is_کوچکترین_Normalized() ثابت {
  إذا (getCategory() != fc_نرمال)
    أرجع خطأ;

  DoubleAPFloat Tmp(*هذا);
  Tmp.make_کوچکترین_Normalized(هذا->isNegative());
  أرجع Tmp.compare(*هذا) == cmpEqual;
}

منطقي DoubleAPFloat::is_بزرگترین() ثابت {
  إذا (getCategory() != fc_نرمال)
    أرجع خطأ;
  DoubleAPFloat Tmp(*هذا);
  Tmp.make_بزرگترین(هذا->isNegative());
  أرجع Tmp.compare(*هذا) == cmpEqual;
}

منطقي DoubleAPFloat::isInteger() ثابت {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble &&
         "Unexpected Semantics");
  أرجع Floats[0].isInteger() && Floats[1].isInteger();
}

فراغ DoubleAPFloat::مقصد_String(Small_وکتور_Impl<حرف> &Str,
                             بیعلامت FormatPrecision,
                             بیعلامت FormatMaxPadding,
                             منطقي TruncateZero) ثابت {
  أكد(Semantics == &APFloat_پایه::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloat_پایه::semPPCDoubleDoubleLegacy, bitcast_مقصد_APInt())
      .مقصد_String(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

صحيح DoubleAPFloat::getExactLog2Abs() ثابت {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  إذا (get_دومی().isNonZero())
    أرجع INT_MIN;
  أرجع get_اولی().getExactLog2Abs();
}

صحيح ilogb(ثابت DoubleAPFloat &Arg) {
  ثابت APFloat &Hi = Arg.get_اولی();
  ثابت APFloat &Lo = Arg.get_دومی();
  صحيح Ilogb_نتیجه = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  إذا (Arg.getCategory() != fc_نرمال)
    أرجع Ilogb_نتیجه;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  إذا (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    أرجع Ilogb_نتیجه;
  إذا (Hi.getExactLog2Abs() == INT_MIN)
    أرجع Ilogb_نتیجه;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  أرجع Ilogb_نتیجه - 1;
}

DoubleAPFloat scalbn(ثابت DoubleAPFloat &Arg, صحيح Exp,
                     APFloat::rounding_نما RM) {
  أكد(Arg.Semantics == &APFloat_پایه::PPCDoubleDouble() &&
         "Unexpected Semantics");
  أرجع DoubleAPFloat(APFloat_پایه::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(ثابت DoubleAPFloat &Arg, صحيح &Exp,
                    APFloat::rounding_نما RM) {
  أكد(Arg.Semantics == &APFloat_پایه::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  إذا (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.get_اولی() = Quiet.get_اولی().makeQuiet();
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

  ثابت APFloat &Hi = Arg.get_اولی();
  ثابت APFloat &Lo = Arg.get_دومی();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  ثابت منطقي SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat دومی = Lo;
  إذا (Arg.getCategory() == APFloat::fc_نرمال && Lo.isFiniteNonZero()) {
    rounding_نما Lo_Rounding_نما;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    إذا (RM == rmTowardZero)
      Lo_Rounding_نما = Arg.isNegative() ? rm_Toward_مثبت : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    وإلا إذا (RM == rm_نزدیکترین_Ties_مقصد_Away && SignsDisagree && Exp > 0)
      Lo_Rounding_نما = rm_نزدیکترین_Ties_مقصد_Even;
    وإلا
      Lo_Rounding_نما = RM;
    دومی = scalbn(Lo, -Exp, Lo_Rounding_نما);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    إذا (RM == rm_نزدیکترین_Ties_مقصد_Away && Lo_Rounding_نما == rm_نزدیکترین_Ties_مقصد_Even) {
      // Re-scale the result back to check if rounding occurred.
      ثابت APFloat RecomposedLo = scalbn(دومی, Exp, rm_نزدیکترین_Ties_مقصد_Even);
      إذا (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        ثابت APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        ثابت APFloat Ulp_از_دومی = harrisonUlp(دومی);
        ثابت APFloat Scaled_Ulp_از_دومی =
            scalbn(Ulp_از_دومی, Exp - 1, rm_نزدیکترین_Ties_مقصد_Even);
        ثابت منطقي IsMidpoint = قدر_مطلق(RoundingError) == Scaled_Ulp_از_دومی;
        ثابت منطقي RoundedLoAway =
            دومی.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        إذا (IsMidpoint && RoundedLoAway)
          دومی.بعدی(/*nextDown=*/!دومی.isNegative());
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
    إذا (دومی.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat اولی = scalbn(Hi, -Exp, RM);
  أرجع DoubleAPFloat(APFloat_پایه::PPCDoubleDouble(), std::move(اولی),
                       std::move(دومی));
}

APInt DoubleAPFloat::getNaNPayload() ثابت { أرجع Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, ثابت fltSemantics &Semantics) {
  إذا (استفاده_میکند_Layout<IEEEFloat>(Semantics)) {
    جديد (&IEEE) IEEEFloat(std::move(F));
    أرجع;
  }
  إذا (استفاده_میکند_Layout<DoubleAPFloat>(Semantics)) {
    ثابت fltSemantics& S = F.getSemantics();
    جديد (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloat_پایه::IEEEdouble()));
    أرجع;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::op_وضعیت> APFloat::convertFromString(StringRef Str,
                                                       rounding_نما RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_کد hash_مقدار(ثابت APFloat &Arg) {
  إذا (APFloat::استفاده_میکند_Layout<detail::IEEEFloat>(Arg.getSemantics()))
    أرجع hash_مقدار(Arg.U.IEEE);
  إذا (APFloat::استفاده_میکند_Layout<detail::DoubleAPFloat>(Arg.getSemantics()))
    أرجع hash_مقدار(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(ثابت fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  خودکار وضعیت_Or_Err = convertFromString(S, rm_نزدیکترین_Ties_مقصد_Even);
  أكد(وضعیت_Or_Err && "Invalid floating point representation");
  consumeError(وضعیت_Or_Err.takeError());
}

FPClassTest APFloat::classify() ثابت {
  إذا (isZero())
    أرجع isNegative() ? fcNegZero : fcPosZero;
  إذا (is_نرمال())
    أرجع isNegative() ? fc_Neg_نرمال : fc_Pos_نرمال;
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

  أرجع درست;
}

APFloat::op_وضعیت APFloat::convert(ثابت fltSemantics &مقصد_Semantics_2,
                                   rounding_نما RM, منطقي *losesInfo) {
  إذا (&getSemantics() == &مقصد_Semantics_2) {
    *losesInfo = خطأ;
    أرجع opOK;
  }
  إذا (استفاده_میکند_Layout<IEEEFloat>(getSemantics()) &&
      استفاده_میکند_Layout<IEEEFloat>(مقصد_Semantics_2))
    أرجع U.IEEE.convert(مقصد_Semantics_2, RM, losesInfo);
  إذا (استفاده_میکند_Layout<IEEEFloat>(getSemantics()) &&
      استفاده_میکند_Layout<DoubleAPFloat>(مقصد_Semantics_2)) {
    أكد(&مقصد_Semantics_2 == &APFloat_پایه::semPPCDoubleDouble);
    خودکار Ret =
        U.IEEE.convert(APFloat_پایه::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *هذا = APFloat(مقصد_Semantics_2, U.IEEE.bitcast_مقصد_APInt());
    أرجع Ret;
  }
  إذا (استفاده_میکند_Layout<DoubleAPFloat>(getSemantics()) &&
      استفاده_میکند_Layout<IEEEFloat>(مقصد_Semantics_2)) {
    خودکار Ret = getIEEE().convert(مقصد_Semantics_2, RM, losesInfo);
    *هذا = APFloat(std::move(getIEEE()), مقصد_Semantics_2);
    أرجع Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_مقدار(ثابت fltSemantics &Semantics) {
  أرجع APFloat(Semantics, APInt::getAllOnes(Semantics.اندازه_درون_بیتها));
}

فراغ APFloat::print(raw_ostream &OS) ثابت {
  Small_وکتور<حرف, 16> میانگیر;
  مقصد_String(میانگیر);
  OS << میانگیر;
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_روش فراغ APFloat::dump() ثابت {
  print(dbgs());
  dbgs() << '\n';
}
#پایان_اگر

فراغ APFloat::Profile(Folding_مجموعه_گره_ID &NID) ثابت {
  NID.جمع_کن_2(bitcast_مقصد_APInt());
}

APFloat::op_وضعیت APFloat::convert_مقصد_Integer(APSInt &نتیجه_2,
                                            rounding_نما rounding_نما_2,
                                            منطقي *isExact) ثابت {
  بیعلامت بیت_عرض = نتیجه_2.get_بیت_عرض();
  Small_وکتور<uint64_t, 4> parts(نتیجه_2.get_رقم_واژگان());
  op_وضعیت وضعیت_2 = convert_مقصد_Integer(parts, بیت_عرض, نتیجه_2.isSigned(),
                                     rounding_نما_2, isExact);
  // Keeps the original signed-ness.
  نتیجه_2 = APInt(بیت_عرض, parts);
  أرجع وضعیت_2;
}

مزدوج APFloat::convert_مقصد_Double() ثابت {
  إذا (&getSemantics() == &APFloat_پایه::semIEEEdouble)
    أرجع getIEEE().convert_مقصد_Double();
  أكد(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat موقت = *هذا;
  منطقي LosesInfo;
  [[maybe_استفادهنشده]] op_وضعیت St =
      موقت.convert(APFloat_پایه::semIEEEdouble, rm_نزدیکترین_Ties_مقصد_Even, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  أرجع موقت.getIEEE().convert_مقصد_Double();
}

#اگر_تعریف_شده HAS_IEE754_FLOAT128
float128 APFloat::convert_مقصد_Quad() ثابت {
  إذا (&getSemantics() == &APFloat_پایه::semIEEEquad)
    أرجع getIEEE().convert_مقصد_Quad();
  أكد(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat موقت = *هذا;
  منطقي LosesInfo;
  [[maybe_استفادهنشده]] op_وضعیت St =
      موقت.convert(APFloat_پایه::semIEEEquad, rm_نزدیکترین_Ties_مقصد_Even, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  أرجع موقت.getIEEE().convert_مقصد_Quad();
}
#پایان_اگر

عائم APFloat::convert_مقصد_Float() ثابت {
  إذا (&getSemantics() == &APFloat_پایه::semIEEEsingle)
    أرجع getIEEE().convert_مقصد_Float();
  أكد(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat موقت = *هذا;
  منطقي LosesInfo;
  [[maybe_استفادهنشده]] op_وضعیت St =
      موقت.convert(APFloat_پایه::semIEEEsingle, rm_نزدیکترین_Ties_مقصد_Even, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  أرجع موقت.getIEEE().convert_مقصد_Float();
}

بیعلامت APFloat_پایه::get_Arbitrary_FPFormat_اندازه_درون_بیتها(StringRef Format) {
  أرجع StringSwitch<بیعلامت>(Format)
      .Case("Float8E5M2", get_اندازه_درون_بیتها(semFloat8E5M2))
      .Case("Float8E5M2FNUZ", get_اندازه_درون_بیتها(semFloat8E5M2FNUZ))
      .Case("Float8E4M3", get_اندازه_درون_بیتها(semFloat8E4M3))
      .Case("Float8E4M3FN", get_اندازه_درون_بیتها(semFloat8E4M3FN))
      .Case("Float8E4M3FNUZ", get_اندازه_درون_بیتها(semFloat8E4M3FNUZ))
      .Case("Float8E4M3B11FNUZ", get_اندازه_درون_بیتها(semFloat8E4M3B11FNUZ))
      .Case("Float8E3M4", get_اندازه_درون_بیتها(semFloat8E3M4))
      .Case("Float8E8M0FNU", get_اندازه_درون_بیتها(semFloat8E8M0FNU))
      .Case("Float6E3M2FN", get_اندازه_درون_بیتها(semFloat6E3M2FN))
      .Case("Float6E2M3FN", get_اندازه_درون_بیتها(semFloat6E2M3FN))
      .Case("Float4E2M1FN", get_اندازه_درون_بیتها(semFloat4E2M1FN))
      .Case("Float8E5M3FNU", get_اندازه_درون_بیتها(semFloat8E5M3FNU))
      .Default(0);
}

منطقي APFloat_پایه::isValidArbitraryFPFormat(StringRef Format) {
  أرجع get_Arbitrary_FPFormat_اندازه_درون_بیتها(Format) != 0;
}

ثابت fltSemantics *APFloat_پایه::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  أرجع StringSwitch<ثابت fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(اشاره_تهی);
}

APFloat::Storage::~Storage() {
  إذا (استفاده_میکند_Layout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    أرجع;
  }
  إذا (استفاده_میکند_Layout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    أرجع;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(ثابت APFloat::Storage &RHS) {
  إذا (استفاده_میکند_Layout<IEEEFloat>(*RHS.semantics)) {
    جديد (هذا) IEEEFloat(RHS.IEEE);
    أرجع;
  }
  إذا (استفاده_میکند_Layout<DoubleAPFloat>(*RHS.semantics)) {
    جديد (هذا) DoubleAPFloat(RHS.Double);
    أرجع;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  إذا (استفاده_میکند_Layout<IEEEFloat>(*RHS.semantics)) {
    جديد (هذا) IEEEFloat(std::move(RHS.IEEE));
    أرجع;
  }
  إذا (استفاده_میکند_Layout<DoubleAPFloat>(*RHS.semantics)) {
    جديد (هذا) DoubleAPFloat(std::move(RHS.Double));
    أرجع;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::عملگر=(ثابت APFloat::Storage &RHS) {
  إذا (استفاده_میکند_Layout<IEEEFloat>(*semantics) &&
      استفاده_میکند_Layout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } وإلا إذا (استفاده_میکند_Layout<DoubleAPFloat>(*semantics) &&
             استفاده_میکند_Layout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } وإلا إذا (هذا != &RHS) {
    هذا->~Storage();
    جديد (هذا) Storage(RHS);
  }
  أرجع *هذا;
}

APFloat::Storage &APFloat::Storage::عملگر=(APFloat::Storage &&RHS) {
  إذا (استفاده_میکند_Layout<IEEEFloat>(*semantics) &&
      استفاده_میکند_Layout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } وإلا إذا (استفاده_میکند_Layout<DoubleAPFloat>(*semantics) &&
             استفاده_میکند_Layout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } وإلا إذا (هذا != &RHS) {
    هذا->~Storage();
    جديد (هذا) Storage(std::move(RHS));
  }
  أرجع *هذا;
}

نطاق {

APFloat::op_وضعیت get_Op_وضعیت_From_Libc(صحيح libc_exceptions) {
  APFloat::op_وضعیت وضعیت_2 = APFloat::opOK;
  إذا (libc_exceptions & FE_INVALID)
    وضعیت_2 = static_cast<APFloat::op_وضعیت>(وضعیت_2 | APFloat::opInvalidOp);
  إذا (libc_exceptions & FE_DIVBYZERO)
    وضعیت_2 = static_cast<APFloat::op_وضعیت>(وضعیت_2 | APFloat::opDivByZero);
  إذا (libc_exceptions & FE_سرریز)
    وضعیت_2 = static_cast<APFloat::op_وضعیت>(وضعیت_2 | APFloat::op_سرریز);
  إذا (libc_exceptions & FE_UNDERFLOW)
    وضعیت_2 = static_cast<APFloat::op_وضعیت>(وضعیت_2 | APFloat::opUnderflow);
  إذا (libc_exceptions & FE_INEXACT)
    وضعیت_2 = static_cast<APFloat::op_وضعیت>(وضعیت_2 | APFloat::opInexact);
  أرجع وضعیت_2;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> نمایی_تابع(ثابت APFloat &x, Rounding_نما rounding_نما_2,
                           APFloat::op_وضعیت *وضعیت_2) {

  إذا (rounding_نما_2 == APFloat_پایه::rm_نزدیکترین_Ties_مقصد_Even) {
    إذا (APFloat::Semantics_مقصد_Enum(x.getSemantics()) ==
        APFloat_پایه::S_IEEEsingle) {
      عائم x_val = x.convert_مقصد_Float();
      صحيح exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      إذا (وضعیت_2) {
        *وضعیت_2 = get_Op_وضعیت_From_Libc(exc);
        إذا (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *وضعیت_2 =
              static_cast<APFloat::op_وضعیت>(*وضعیت_2 | APFloat::opInvalidOp);
        }
      }
      عائم نتیجه_2 = LIBC_NAMESPACE::shared::expf(x_val);
      أرجع APFloat(نتیجه_2);
    }
    إذا (APFloat::Semantics_مقصد_Enum(x.getSemantics()) ==
        APFloat_پایه::S_IEEEdouble) {
      مزدوج x_val = x.convert_مقصد_Double();
      صحيح exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      إذا (وضعیت_2) {
        *وضعیت_2 = get_Op_وضعیت_From_Libc(exc);
        إذا (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *وضعیت_2 =
              static_cast<APFloat::op_وضعیت>(*وضعیت_2 | APFloat::opInvalidOp);
        }
      }
      مزدوج نتیجه_2 = LIBC_NAMESPACE::shared::نمایی_تابع(x_val);
      أرجع APFloat(نتیجه_2);
    }
  }
  أرجع std::nullopt;
}

} // namespace llvm

#لغو_تعریف APFLOAT_DISPATCH_ON_SEMANTICS
