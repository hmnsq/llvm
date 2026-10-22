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
#تعریف_کرو LIBC_NAMESPACE __llvm_libc_apfloat
#تعریف_کرو LIBC_MATH (LIBC_MATH_NO_ERRNO | LIBC_MATH_NO_EXCEPT)

#شامل "shared/math.h"
#شامل "shared/math_check_exceptions.h"

#تعریف_کرو APFLOAT_DISPATCH_ON_SEMANTICS(طریقہ_CALL)                             \
  کرو {                                                                         \
    اگر (استعمال_کرتا_Layout<IEEEFloat>(getSemantics()))                                 \
      واپس U.IEEE.طریقہ_CALL;                                               \
    اگر (استعمال_کرتا_Layout<DoubleAPFloat>(getSemantics()))                             \
      واپس U.Double.طریقہ_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } جب (جھوٹ)

استعمال نامخلا llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#تعریف_کرو Pack_Categories_Into_کلید(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloat_بنیاد::integer_Part_چوڑائی % 4 == 0, "Part width must be divisible by 4!");

نامخلا llvm {

constexpr fltSemantics APFloat_بنیاد::semIEEEhalf = {15, -14, 11, 16};
constexpr fltSemantics APFloat_بنیاد::semBFloat = {127, -126, 8, 16};
constexpr fltSemantics APFloat_بنیاد::semIEEEsingle = {127, -126, 24, 32};
constexpr fltSemantics APFloat_بنیاد::semIEEEdouble = {1023, -1022, 53, 64};
constexpr fltSemantics APFloat_بنیاد::semIEEEquad = {16383, -16382, 113, 128};
constexpr fltSemantics APFloat_بنیاد::semX87DoubleExtended = {
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
constexpr fltSemantics APFloat_بنیاد::semPPCDoubleDouble = {-1, 0, 0, 128};
constexpr fltSemantics APFloat_بنیاد::semPPCDoubleDoubleLegacy = {
    1023, -1022 + 53, 53 + 53, 128};
constexpr fltSemantics APFloat_بنیاد::semFloat8E5M2 = {15, -14, 3, 8};
constexpr fltSemantics APFloat_بنیاد::semFloat8E5M2FNUZ = {
    15, -15, 3, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_بنیاد::semFloat8E4M3 = {7, -6, 4, 8};
constexpr fltSemantics APFloat_بنیاد::semFloat8E4M3FN = {
    8, -6, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::AllOnes};
constexpr fltSemantics APFloat_بنیاد::semFloat8E4M3FNUZ = {
    7, -7, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_بنیاد::semFloat8E4M3B11FNUZ = {
    4, -10, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_بنیاد::semFloat8E3M4 = {3, -2, 5, 8};
constexpr fltSemantics APFloat_بنیاد::semFloatTF32 = {127, -126, 11, 19};
constexpr fltSemantics APFloat_بنیاد::semFloat8E8M0FNU = {
    127,
    -127,
    1,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    جھوٹ,
    جھوٹ,
    جھوٹ,
    جھوٹ};

constexpr fltSemantics APFloat_بنیاد::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    سچ,
    جھوٹ,
    جھوٹ};

constexpr fltSemantics APFloat_بنیاد::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_بنیاد::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_بنیاد::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_بنیاد::semBogus = {0, 0, 0, 0};

ثابت llvm::fltSemantics &APFloat_بنیاد::Enum_مقصد_Semantics(Semantics S) {
  انتخاب (S) {
  حالت S_IEEEhalf:
    واپس IEEEhalf();
  حالت S_BFloat:
    واپس BFloat();
  حالت S_IEEEsingle:
    واپس IEEEsingle();
  حالت S_IEEEdouble:
    واپس IEEEdouble();
  حالت S_IEEEquad:
    واپس IEEEquad();
  حالت S_x87DoubleExtended:
    واپس x87DoubleExtended();
  حالت S_PPCDoubleDouble:
    واپس PPCDoubleDouble();
  حالت S_PPCDoubleDoubleLegacy:
    واپس PPCDoubleDoubleLegacy();
  حالت S_Float8E5M2:
    واپس Float8E5M2();
  حالت S_Float8E5M2FNUZ:
    واپس Float8E5M2FNUZ();
  حالت S_Float8E4M3:
    واپس Float8E4M3();
  حالت S_Float8E4M3FN:
    واپس Float8E4M3FN();
  حالت S_Float8E4M3FNUZ:
    واپس Float8E4M3FNUZ();
  حالت S_Float8E4M3B11FNUZ:
    واپس Float8E4M3B11FNUZ();
  حالت S_Float8E3M4:
    واپس Float8E3M4();
  حالت S_FloatTF32:
    واپس FloatTF32();
  حالت S_Float8E8M0FNU:
    واپس Float8E8M0FNU();
  حالت S_Float8E5M3FNU:
    واپس Float8E5M3FNU();
  حالت S_Float6E3M2FN:
    واپس Float6E3M2FN();
  حالت S_Float6E2M3FN:
    واپس Float6E2M3FN();
  حالت S_Float4E2M1FN:
    واپس Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloat_بنیاد::Semantics
APFloat_بنیاد::Semantics_مقصد_Enum(ثابت llvm::fltSemantics &Sem) {
  اگر (&Sem == &llvm::APFloat::IEEEhalf())
    واپس S_IEEEhalf;
  ورنہ اگر (&Sem == &llvm::APFloat::BFloat())
    واپس S_BFloat;
  ورنہ اگر (&Sem == &llvm::APFloat::IEEEsingle())
    واپس S_IEEEsingle;
  ورنہ اگر (&Sem == &llvm::APFloat::IEEEdouble())
    واپس S_IEEEdouble;
  ورنہ اگر (&Sem == &llvm::APFloat::IEEEquad())
    واپس S_IEEEquad;
  ورنہ اگر (&Sem == &llvm::APFloat::x87DoubleExtended())
    واپس S_x87DoubleExtended;
  ورنہ اگر (&Sem == &llvm::APFloat::PPCDoubleDouble())
    واپس S_PPCDoubleDouble;
  ورنہ اگر (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    واپس S_PPCDoubleDoubleLegacy;
  ورنہ اگر (&Sem == &llvm::APFloat::Float8E5M2())
    واپس S_Float8E5M2;
  ورنہ اگر (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    واپس S_Float8E5M2FNUZ;
  ورنہ اگر (&Sem == &llvm::APFloat::Float8E4M3())
    واپس S_Float8E4M3;
  ورنہ اگر (&Sem == &llvm::APFloat::Float8E4M3FN())
    واپس S_Float8E4M3FN;
  ورنہ اگر (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    واپس S_Float8E4M3FNUZ;
  ورنہ اگر (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    واپس S_Float8E4M3B11FNUZ;
  ورنہ اگر (&Sem == &llvm::APFloat::Float8E3M4())
    واپس S_Float8E3M4;
  ورنہ اگر (&Sem == &llvm::APFloat::FloatTF32())
    واپس S_FloatTF32;
  ورنہ اگر (&Sem == &llvm::APFloat::Float8E8M0FNU())
    واپس S_Float8E8M0FNU;
  ورنہ اگر (&Sem == &llvm::APFloat::Float8E5M3FNU())
    واپس S_Float8E5M3FNU;
  ورنہ اگر (&Sem == &llvm::APFloat::Float6E3M2FN())
    واپس S_Float6E3M2FN;
  ورنہ اگر (&Sem == &llvm::APFloat::Float6E2M3FN())
    واپس S_Float6E2M3FN;
  ورنہ اگر (&Sem == &llvm::APFloat::Float4E2M1FN())
    واپس S_Float4E2M1FN;
  ورنہ
    llvm_unreachable("Unknown floating semantics");
}

منطقی APFloat_بنیاد::isRepresentableBy(ثابت fltSemantics &A,
                                    ثابت fltSemantics &B) {
  واپس A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

منطقی APFloat_بنیاد::is_Losslessly_Convertible_مقصد(ثابت fltSemantics &From,
                                            ثابت fltSemantics &Initial_مقصد,
                                            منطقی IgnoreNaNs) {
  اگر (&From == &Initial_مقصد)
    واپس سچ;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  اگر (&From == &semPPCDoubleDouble)
    واپس جھوٹ;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  ثابت fltSemantics &مقصد =
      &Initial_مقصد == &semPPCDoubleDouble ? semIEEEdouble : Initial_مقصد;

  اگر (!isRepresentableBy(From, مقصد))
    واپس جھوٹ;

  اگر ((From.hasZero && !مقصد.hasZero) ||
      (From.hasSignedRepr && !مقصد.hasSignedRepr))
    واپس جھوٹ;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  منطقی FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  منطقی مقصد_Has_Signed_Zero = مقصد.hasZero && مقصد.hasSignedRepr &&
                         مقصد.nanEncoding != fltNanEncoding::NegativeZero;
  اگر (FromHasSignedZero && !مقصد_Has_Signed_Zero)
    واپس جھوٹ;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat سب_سے_چھوٹا_From = APFloat::get_سب_سے_چھوٹا(From);
  منطقی LosesInfo = جھوٹ;
  (خالی)سب_سے_چھوٹا_From.convert(مقصد, APFloat::rm_قریب_ترین_Ties_مقصد_Even, &LosesInfo);
  اگر (LosesInfo)
    واپس جھوٹ;

  اگر (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    واپس سچ;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  اگر (IgnoreNaNs) {
    اگر (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      واپس مقصد.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    واپس مقصد.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  اگر (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    واپس جھوٹ;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  اگر (مقصد.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    واپس سچ;
  واپس مقصد.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
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
[[maybe_غیر_مستعمل]] constexpr غیرعلامتی maxExponent = 16383;

// Decimal exponents in the f16 and f32 ranges need at most 2 and 6 parts.
// Keep 16 parts (128 bytes) inline: enough for 5^413, but not a large stack
// frame. Longer fractional spellings use heap storage.
ساکن constexpr غیرعلامتی عدد_Pow5_Parts_On_ڈھیر = 16;

ساکن غیرعلامتی صحیح power_کا_Five_Parts_For_Exponent(غیرعلامتی صحیح power) {
  أكد(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  واپس 2 + ((power * 815) / (351 * APFloat_بنیاد::integer_Part_چوڑائی));
}

غیرعلامتی صحیح APFloat_بنیاد::semanticsPrecision(ثابت fltSemantics &semantics) {
  واپس semantics.precision;
}
APFloat_بنیاد::ExponentType
APFloat_بنیاد::semanticsMaxExponent(ثابت fltSemantics &semantics) {
  واپس semantics.maxExponent;
}
APFloat_بنیاد::ExponentType
APFloat_بنیاد::semanticsMinExponent(ثابت fltSemantics &semantics) {
  واپس semantics.minExponent;
}
غیرعلامتی صحیح APFloat_بنیاد::semantics_سائز_اندر_بٹ(ثابت fltSemantics &semantics) {
  واپس semantics.سائز_اندر_بٹ;
}
غیرعلامتی صحیح APFloat_بنیاد::semantics_Int_سائز_اندر_بٹ(ثابت fltSemantics &semantics,
                                                 منطقی isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  غیرعلامتی صحیح Min_ثنائی_ہندسہ_چوڑائی = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  اگر (isSigned)
    ++Min_ثنائی_ہندسہ_چوڑائی;
  واپس Min_ثنائی_ہندسہ_چوڑائی;
}

منطقی APFloat_بنیاد::semanticsHasZero(ثابت fltSemantics &semantics) {
  واپس semantics.hasZero;
}

منطقی APFloat_بنیاد::semanticsHasSignedRepr(ثابت fltSemantics &semantics) {
  واپس semantics.hasSignedRepr;
}

منطقی APFloat_بنیاد::semanticsHasInf(ثابت fltSemantics &semantics) {
  واپس semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

منطقی APFloat_بنیاد::semanticsHasNaN(ثابت fltSemantics &semantics) {
  واپس semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

منطقی APFloat_بنیاد::isIEEELikeFP(ثابت fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  واپس Semantics_مقصد_Enum(semantics) <= S_IEEEquad;
}

منطقی APFloat_بنیاد::has_Sign_ثنائی_ہندسہ_اندر_MSB(ثابت fltSemantics &semantics) {
  واپس semantics.has_Sign_ثنائی_ہندسہ_اندر_MSB;
}

منطقی APFloat_بنیاد::is_Representable_As_معمول_اندر(ثابت fltSemantics &Src,
                                            ثابت fltSemantics &Dst) {
  // Exponent range must be larger.
  اگر (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    واپس جھوٹ;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  واپس Dst.precision >= Src.precision;
}

غیرعلامتی APFloat_بنیاد::get_سائز_اندر_بٹ(ثابت fltSemantics &Sem) {
  واپس Sem.سائز_اندر_بٹ;
}

ساکن constexpr APFloat_بنیاد::ExponentType
exponentZero(ثابت fltSemantics &semantics) {
  واپس semantics.minExponent - 1;
}

ساکن constexpr APFloat_بنیاد::ExponentType
exponentInf(ثابت fltSemantics &semantics) {
  واپس semantics.maxExponent + 1;
}

ساکن constexpr APFloat_بنیاد::ExponentType
exponentNaN(ثابت fltSemantics &semantics) {
  اگر (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    اگر (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      واپس exponentZero(semantics);
    اگر (semantics.hasSignedRepr || semantics.precision > 1)
      واپس semantics.maxExponent;
  }
  واپس semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

ساکن اندرونی Error createError(ثابت Twine &Err) {
  واپس make_error<StringError>(Err, inconvertible_Error_رمز());
}

ساکن constexpr اندرونی غیرعلامتی صحیح part_گنتی_For_بٹ(غیرعلامتی صحیح بٹ) {
  واپس std::max(1u, (بٹ + APFloat_بنیاد::integer_Part_چوڑائی - 1) /
                          APFloat_بنیاد::integer_Part_چوڑائی);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
ساکن اندرونی غیرعلامتی صحیح
dec_Digit_قدر(غیرعلامتی صحیح c)
{
  واپس c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
ساکن Expected<صحیح> readExponent(StringRef::iterator begin,
                                  StringRef::iterator انتہا) {
  ثابت غیرعلامتی صحیح overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  اگر (p == انتہا || ((*p == '-' || *p == '+') && (p + 1) == انتہا))
    واپس 0;

  منطقی isNegative = *p == '-';
  اگر (*p == '-' || *p == '+') {
    p++;
    اگر (p == انتہا)
      واپس createError("Exponent has no digits");
  }

  غیرعلامتی absExponent = dec_Digit_قدر(*p++);
  اگر (absExponent >= 10U)
    واپس createError("Invalid character in exponent");

  برائے (; p != انتہا; ++p) {
    غیرعلامتی قدر_2 = dec_Digit_قدر(*p);
    اگر (قدر_2 >= 10U)
      واپس createError("Invalid character in exponent");

    absExponent = absExponent * 10U + قدر_2;
    اگر (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      توڑو;
    }
  }

  اگر (isNegative)
    واپس -(صحیح) absExponent;
  ورنہ
    واپس (صحیح) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
ساکن Expected<صحیح> کل_Exponent(StringRef::iterator p,
                                   StringRef::iterator انتہا,
                                   صحیح exponentAdjustment) {
  صحیح exponent = 0;

  اگر (p == انتہا)
    واپس createError("Exponent has no digits");

  منطقی negative = *p == '-';
  اگر (*p == '-' || *p == '+') {
    p++;
    اگر (p == انتہا)
      واپس createError("Exponent has no digits");
  }

  صحیح unsignedExponent = 0;
  منطقی حد_سے_تجاوز_2 = جھوٹ;
  برائے (; p != انتہا; ++p) {
    غیرعلامتی صحیح قدر_2;

    قدر_2 = dec_Digit_قدر(*p);
    اگر (قدر_2 >= 10U)
      واپس createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + قدر_2;
    اگر (unsignedExponent > 32767) {
      حد_سے_تجاوز_2 = سچ;
      توڑو;
    }
  }

  اگر (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    حد_سے_تجاوز_2 = سچ;

  اگر (!حد_سے_تجاوز_2) {
    exponent = unsignedExponent;
    اگر (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    اگر (exponent > 32767 || exponent < -32768)
      حد_سے_تجاوز_2 = سچ;
  }

  اگر (حد_سے_تجاوز_2)
    exponent = negative ? -32768: 32767;

  واپس exponent;
}

ساکن Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator انتہا,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = انتہا;
  جب (p != انتہا && *p == '0')
    p++;

  اگر (p != انتہا && *p == '.') {
    *dot = p++;

    اگر (انتہا - begin == 1)
      واپس createError("Significand has no digits");

    جب (p != انتہا && *p == '0')
      p++;
  }

  واپس p;
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
ساخت decimalInfo {
  ثابت حرف *پہلا_Sig_Digit;
  ثابت حرف *آخری_Sig_Digit;
  صحیح exponent;
  صحیح normalizedExponent;
};

ساکن Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator انتہا, decimalInfo *D) {
  StringRef::iterator dot = انتہا;

  خودکار PtrOrErr = skipLeadingZeroesAndAnyDot(begin, انتہا, &dot);
  اگر (!PtrOrErr)
    واپس PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->پہلا_Sig_Digit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  برائے (; p != انتہا; ++p) {
    اگر (*p == '.') {
      اگر (dot != انتہا)
        واپس createError("String contains multiple dots");
      dot = p++;
      اگر (p == انتہا)
        توڑو;
    }
    اگر (dec_Digit_قدر(*p) >= 10U)
      توڑو;
  }

  اگر (p != انتہا) {
    اگر (*p != 'e' && *p != 'E')
      واپس createError("Invalid character in significand");
    اگر (p == begin)
      واپس createError("Significand has no digits");
    اگر (dot != انتہا && p - begin == 1)
      واپس createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    خودکار ExpOrErr = readExponent(p + 1, انتہا);
    اگر (!ExpOrErr)
      واپس ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    اگر (dot == انتہا)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  اگر (p != D->پہلا_Sig_Digit) {
    /* Drop insignificant trailing zeroes.  */
    اگر (p != begin) {
      کرو
        کرو
          p--;
        جب (p != begin && *p == '0');
      جب (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->پہلا_Sig_Digit)
                                      - (dot > D->پہلا_Sig_Digit && dot < p)));
  }

  D->آخری_Sig_Digit = p;
  واپس Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
ساکن Expected<گمشدہ_Fraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator انتہا,
                            غیرعلامتی صحیح digit_قدر) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  اگر (digit_قدر > 8)
    واپس lf_مزید_Than_Half;
  ورنہ اگر (digit_قدر < 8 && digit_قدر > 0)
    واپس lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  جب (p != انتہا && (*p == '0' || *p == '.'))
    p++;

  اگر (p == انتہا)
    واپس createError("Invalid trailing hexadecimal fraction!");

  غیرعلامتی hexDigit = hex_Digit_قدر(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  اگر (hexDigit == UINT_MAX)
    واپس digit_قدر == 0 ? lfExactlyZero: lfExactlyHalf;
  ورنہ
    واپس digit_قدر == 0 ? lfLessThanHalf: lf_مزید_Than_Half;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
ساکن گمشدہ_Fraction
گمشدہ_Fraction_Through_Truncation(ثابت APFloat_بنیاد::integerPart *parts,
                              غیرعلامتی صحیح part_گنتی,
                              غیرعلامتی صحیح بٹ)
{
  غیرعلامتی lsb = APInt::tcLSB(parts, part_گنتی);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  اگر (بٹ <= lsb)
    واپس lfExactlyZero;
  اگر (بٹ == lsb + 1)
    واپس lfExactlyHalf;
  اگر (بٹ <= part_گنتی * APFloat_بنیاد::integer_Part_چوڑائی &&
      APInt::tc_نکالو_ثنائی_ہندسہ(parts, بٹ - 1))
    واپس lf_مزید_Than_Half;

  واپس lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
ساکن گمشدہ_Fraction
سرکاؤ_دایاں(APFloat_بنیاد::integerPart *dst, غیرعلامتی صحیح parts, غیرعلامتی صحیح بٹ)
{
  گمشدہ_Fraction گمشدہ_fraction = گمشدہ_Fraction_Through_Truncation(dst, parts, بٹ);

  APInt::tc_سرکاؤ_دایاں(dst, parts, بٹ);

  واپس گمشدہ_fraction;
}

/* Combine the effect of two lost fractions.  */
ساکن گمشدہ_Fraction
combine_گمشدہ_Fractions(گمشدہ_Fraction مزید_Significant,
                     گمشدہ_Fraction lessSignificant)
{
  اگر (lessSignificant != lfExactlyZero) {
    اگر (مزید_Significant == lfExactlyZero)
      مزید_Significant = lfLessThanHalf;
    ورنہ اگر (مزید_Significant == lfExactlyHalf)
      مزید_Significant = lf_مزید_Than_Half;
  }

  واپس مزید_Significant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
ساکن غیرعلامتی صحیح
HUerrBound(منطقی inexactMultiply, غیرعلامتی صحیح HUerr1, غیرعلامتی صحیح HUerr2)
{
  أكد(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  اگر (HUerr1 + HUerr2 == 0)
    واپس inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  ورنہ
    واپس inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
ساکن APFloat_بنیاد::integerPart
ulpsFromBoundary(ثابت APFloat_بنیاد::integerPart *parts, غیرعلامتی صحیح بٹ,
                 منطقی is_قریب_ترین) {
  أكد(بٹ != 0);

  بٹ--;
  غیرعلامتی گنتی = بٹ / APFloat_بنیاد::integer_Part_چوڑائی;
  غیرعلامتی part_بٹ = بٹ % APFloat_بنیاد::integer_Part_چوڑائی + 1;

  APFloat_بنیاد::integerPart part =
      parts[گنتی] & (~(APFloat_بنیاد::integerPart)0 >>
                      (APFloat_بنیاد::integer_Part_چوڑائی - part_بٹ));

  APFloat_بنیاد::integerPart boundary;
  اگر (is_قریب_ترین)
    boundary = (APFloat_بنیاد::integerPart) 1 << (part_بٹ - 1);
  ورنہ
    boundary = 0;

  اگر (گنتی == 0) {
    اگر (part - boundary <= boundary - part)
      واپس part - boundary;
    ورنہ
      واپس boundary - part;
  }

  اگر (part == boundary) {
    جب (--گنتی)
      اگر (parts[گنتی])
        واپس ~(APFloat_بنیاد::integerPart) 0; /* A lot.  */

    واپس parts[0];
  } ورنہ اگر (part == boundary - 1) {
    جب (--گنتی)
      اگر (~parts[گنتی])
        واپس ~(APFloat_بنیاد::integerPart) 0; /* A lot.  */

    واپس -parts[0];
  }

  واپس ~(APFloat_بنیاد::integerPart) 0; /* A lot.  */
}

ساکن constexpr غیرعلامتی Power_کا_Five_Part_Counts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

ساکن constexpr APFloat_بنیاد::integerPart Power_کا_Five_Parts[] = {
#شامل "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  غیرعلامتی مجموع = 0;
  برائے (غیرعلامتی گنتی_2 : Power_کا_Five_Part_Counts)
    مجموع += گنتی_2;
  واپس مجموع;
}() == std::سائز_2(Power_کا_Five_Parts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
ساکن غیرعلامتی صحیح
powerOf5(APFloat_بنیاد::integerPart *dst, غیرعلامتی صحیح power) {
  ساکن ثابت APFloat_بنیاد::integerPart پہلا_Eight_Powers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  Small_سمتیہ<APFloat_بنیاد::integerPart, عدد_Pow5_Parts_On_ڈھیر> scratch;
  scratch.resize_for_overwrite(power_کا_Five_Parts_For_Exponent(power));

  APFloat_بنیاد::integerPart *p1, *p2;
  ثابت APFloat_بنیاد::integerPart *pow5;

  p1 = dst;
  p2 = scratch.ڈیٹا();

  *p1 = پہلا_Eight_Powers[power & 7];
  power >>= 3;

  غیرعلامتی نتیجہ_2 = 1;
  pow5 = Power_کا_Five_Parts;

  برائے (غیرعلامتی صحیح n = 0; power; power >>= 1, n++) {
    غیرعلامتی parts_گنتی = Power_کا_Five_Part_Counts[n];

    اگر (power & 1) {
      APFloat_بنیاد::integerPart *tmp;

      APInt::tc_بھرا_Multiply(p2, p1, pow5, نتیجہ_2, parts_گنتی);
      نتیجہ_2 += parts_گنتی;
      اگر (p2[نتیجہ_2 - 1] == 0)
        نتیجہ_2--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_گنتی;
  }

  اگر (p1 != dst)
    APInt::tc_تفویض(dst, p1, نتیجہ_2);

  واپس نتیجہ_2;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
ساکن ثابت حرف hex_Digits_زیریں[] = "0123456789abcdef0";
ساکن ثابت حرف hex_Digits_بالائی[] = "0123456789ABCDEF0";
ساکن ثابت حرف infinityL[] = "infinity";
ساکن ثابت حرف infinityU[] = "INFINITY";
ساکن ثابت حرف NaNL[] = "nan";
ساکن ثابت حرف NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
ساکن غیرعلامتی صحیح
partAsHex (حرف *dst, APFloat_بنیاد::integerPart part, غیرعلامتی صحیح گنتی,
           ثابت حرف *hexDigitChars)
{
  غیرعلامتی صحیح نتیجہ_2 = گنتی;

  أكد(گنتی != 0 && گنتی <= APFloat_بنیاد::integer_Part_چوڑائی / 4);

  part >>= (APFloat_بنیاد::integer_Part_چوڑائی - 4 * گنتی);
  جب (گنتی--) {
    dst[گنتی] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  واپس نتیجہ_2;
}

/* Write out an unsigned decimal integer.  */
ساکن حرف *writeUnsignedDecimal(حرف *dst, غیرعلامتی صحیح n) {
  حرف buff[40], *p;

  p = buff;
  کرو
    *p++ = '0' + n % 10;
  جب (n /= 10);

  کرو
    *dst++ = *--p;
  جب (p != buff);

  واپس dst;
}

/* Write out a signed decimal integer.  */
ساکن حرف *writeSignedDecimal(حرف *dst, صحیح قدر_2) {
  اگر (قدر_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(غیرعلامتی) قدر_2);
  } ورنہ {
    dst = writeUnsignedDecimal(dst, قدر_2);
  }

  واپس dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
ساکن APFloat harrisonUlp(ثابت APFloat &X) {
  ثابت fltSemantics &Sem = X.getSemantics();
  انتخاب (X.getCategory()) {
  حالت APFloat::fcNaN:
    واپس APFloat::getQNaN(Sem);
  حالت APFloat::fcInfinity:
    واپس APFloat::getInf(Sem);
  حالت APFloat::fcZero:
    واپس APFloat::get_سب_سے_چھوٹا(Sem);
  حالت APFloat::fc_معمول:
    توڑو;
  }
  اگر (X.isDenormal() || X.is_سب_سے_چھوٹا_Normalized())
    واپس APFloat::get_سب_سے_چھوٹا(Sem);
  صحیح Exp = ilogb(X);
  اگر (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  واپس scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rm_قریب_ترین_Ties_مقصد_Even);
}

نامخلا detail {
/* Constructors.  */
خالی IEEEFloat::initialize(ثابت fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  غیرعلامتی گنتی = part_گنتی();
  اگر (گنتی > 1)
    significand.parts = نیا integerPart[گنتی];
}

خالی IEEEFloat::freeSignificand() {
  اگر (needsCleanup())
    حذف [] significand.parts;
}

خالی IEEEFloat::تفویض(ثابت IEEEFloat &rhs) {
  أكد(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  اگر (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

خالی IEEEFloat::copySignificand(ثابت IEEEFloat &rhs) {
  أكد(isFiniteNonZero() || category == fcNaN);
  أكد(rhs.part_گنتی() >= part_گنتی());

  APInt::tc_تفویض(significandParts(), rhs.significandParts(),
                  part_گنتی());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
خالی IEEEFloat::makeNaN(منطقی SNaN, منطقی Negative, ثابت APInt *fill) {
  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  اگر (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  غیرعلامتی عدد_Parts = part_گنتی();

  APInt fill_storage;
  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = جھوٹ;
    اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = سچ;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } ورنہ {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  اگر (!fill || fill->get_عدد_الفاظ() < عدد_Parts)
    APInt::tc_سیٹ(significand, 0, عدد_Parts);
  اگر (fill) {
    APInt::tc_تفویض(significand, fill->get_Raw_ڈیٹا(),
                    std::min(fill->get_عدد_الفاظ(), عدد_Parts));

    // Zero out the excess bits of the significand.
    غیرعلامتی بٹ_مقصد_Preserve = semantics->precision - 1;
    غیرعلامتی part = بٹ_مقصد_Preserve / 64;
    بٹ_مقصد_Preserve %= 64;
    significand[part] &= ((1ULL << بٹ_مقصد_Preserve) - 1);
    برائے (part++; part != عدد_Parts; ++part)
      significand[part] = 0;
  }

  غیرعلامتی QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  اگر (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tc_Clear_ثنائی_ہندسہ(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    اگر (APInt::tcIsZero(significand, عدد_Parts))
      APInt::tc_سیٹ_ثنائی_ہندسہ(significand, QNaNBit - 1);
  } ورنہ اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } ورنہ {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tc_سیٹ_ثنائی_ہندسہ(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  اگر (semantics == &APFloat_بنیاد::semX87DoubleExtended)
    APInt::tc_سیٹ_ثنائی_ہندسہ(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::عامل=(ثابت IEEEFloat &rhs) {
  اگر (یہ != &rhs) {
    اگر (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    تفویض(rhs);
  }

  واپس *یہ;
}

IEEEFloat &IEEEFloat::عامل=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloat_بنیاد::semBogus;
  واپس *یہ;
}

منطقی IEEEFloat::isDenormal() ثابت {
  واپس getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tc_نکالو_ثنائی_ہندسہ(significandParts(), semantics->precision - 1) ==
          0);
}

منطقی IEEEFloat::is_سب_سے_چھوٹا() ثابت {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  واپس isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

منطقی IEEEFloat::is_سب_سے_چھوٹا_Normalized() ثابت {
  واپس getCategory() == fc_معمول && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

غیرعلامتی صحیح IEEEFloat::get_عدد_بلند_بٹ() ثابت {
  ثابت غیرعلامتی صحیح Part_گنتی = part_گنتی_For_بٹ(semantics->precision);
  ثابت غیرعلامتی صحیح بٹ_2 = Part_گنتی * integer_Part_چوڑائی;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  ثابت غیرعلامتی صحیح عدد_بلند_بٹ = (semantics->precision > 1)
                                       ? (بٹ_2 - semantics->precision + 1)
                                       : (بٹ_2 - semantics->precision);
  واپس عدد_بلند_بٹ;
}

منطقی IEEEFloat::isSignificandAllOnes() ثابت {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  ثابت integerPart *Parts = significandParts();
  ثابت غیرعلامتی Part_گنتی = part_گنتی_For_بٹ(semantics->precision);
  برائے (غیرعلامتی i = 0; i < Part_گنتی - 1; i++)
    اگر (~Parts[i])
      واپس جھوٹ;

  // Set the unused high bits to all ones when we compare.
  ثابت غیرعلامتی عدد_بلند_بٹ = get_عدد_بلند_بٹ();
  أكد(عدد_بلند_بٹ <= integer_Part_چوڑائی && عدد_بلند_بٹ > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ثابت integerPart بلند_ثنائی_ہندسہ_Fill =
    ~integerPart(0) << (integer_Part_چوڑائی - عدد_بلند_بٹ);
  اگر ((semantics->precision <= 1) || (~(Parts[Part_گنتی - 1] | بلند_ثنائی_ہندسہ_Fill)))
    واپس جھوٹ;

  واپس سچ;
}

منطقی IEEEFloat::isSignificandAllOnesExceptLSB() ثابت {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  ثابت integerPart *Parts = significandParts();

  اگر (Parts[0] & 1)
    واپس جھوٹ;

  ثابت غیرعلامتی Part_گنتی = part_گنتی_For_بٹ(semantics->precision);
  برائے (غیرعلامتی i = 0; i < Part_گنتی - 1; i++) {
    اگر (~Parts[i] & ~غیرعلامتی{!i})
      واپس جھوٹ;
  }

  // Set the unused high bits to all ones when we compare.
  ثابت غیرعلامتی عدد_بلند_بٹ = get_عدد_بلند_بٹ();
  أكد(عدد_بلند_بٹ <= integer_Part_چوڑائی && عدد_بلند_بٹ > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ثابت integerPart بلند_ثنائی_ہندسہ_Fill = ~integerPart(0)
                                  << (integer_Part_چوڑائی - عدد_بلند_بٹ);
  اگر (~(Parts[Part_گنتی - 1] | بلند_ثنائی_ہندسہ_Fill | 0x1))
    واپس جھوٹ;

  واپس سچ;
}

منطقی IEEEFloat::isSignificandAllZeros() ثابت {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  ثابت integerPart *Parts = significandParts();
  ثابت غیرعلامتی Part_گنتی = part_گنتی_For_بٹ(semantics->precision);

  برائے (غیرعلامتی i = 0; i < Part_گنتی - 1; i++)
    اگر (Parts[i])
      واپس جھوٹ;

  // Compute how many bits are used in the final word.
  ثابت غیرعلامتی عدد_بلند_بٹ = get_عدد_بلند_بٹ();
  أكد(عدد_بلند_بٹ < integer_Part_چوڑائی && "Can not have more high bits to "
         "clear than integerPartWidth");
  ثابت integerPart بلند_ثنائی_ہندسہ_نقاب = ~integerPart(0) >> عدد_بلند_بٹ;

  اگر ((semantics->precision > 1) && (Parts[Part_گنتی - 1] & بلند_ثنائی_ہندسہ_نقاب))
    واپس جھوٹ;

  واپس سچ;
}

منطقی IEEEFloat::isSignificandAllZerosExceptMSB() ثابت {
  ثابت integerPart *Parts = significandParts();
  ثابت غیرعلامتی Part_گنتی = part_گنتی_For_بٹ(semantics->precision);

  برائے (غیرعلامتی i = 0; i < Part_گنتی - 1; i++) {
    اگر (Parts[i])
      واپس جھوٹ;
  }

  ثابت غیرعلامتی عدد_بلند_بٹ = get_عدد_بلند_بٹ();
  ثابت integerPart MSBMask = integerPart(1)
                              << (integer_Part_چوڑائی - عدد_بلند_بٹ);
  واپس ((semantics->precision <= 1) || (Parts[Part_گنتی - 1] == MSBMask));
}

منطقی IEEEFloat::is_سب_سے_بڑا() ثابت {
  منطقی IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    واپس (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } ورنہ {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    واپس IsMaxExp && isSignificandAllOnes();
  }
}

منطقی IEEEFloat::isInteger() ثابت {
  // This could be made more efficient; I'm going for obviously correct.
  اگر (!isFinite()) واپس جھوٹ;
  IEEEFloat truncated = *یہ;
  truncated.باری_مقصد_Integral(rmTowardZero);
  واپس compare(truncated) == cmpEqual;
}

منطقی IEEEFloat::bitwiseIsEqual(ثابت IEEEFloat &rhs) ثابت {
  اگر (یہ == &rhs)
    واپس سچ;
  اگر (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    واپس جھوٹ;
  اگر (category==fcZero || category==fcInfinity)
    واپس سچ;

  اگر (isFiniteNonZero() && exponent != rhs.exponent)
    واپس جھوٹ;

  واپس std::equal(significandParts(), significandParts() + part_گنتی(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics, integerPart قدر_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fc_معمول;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = قدر_2;
  normalize(rm_قریب_ترین_Ties_مقصد_Even, lfExactlyZero);
}

IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(جھوٹ) : make_سب_سے_چھوٹا_Normalized(جھوٹ);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(ثابت fltSemantics &ourSemantics, uninitialized_لیبل لیبل_2)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(ثابت IEEEFloat &rhs) {
  initialize(rhs.semantics);
  تفویض(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloat_بنیاد::semBogus) {
  *یہ = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

غیرعلامتی صحیح IEEEFloat::part_گنتی() ثابت {
  واپس part_گنتی_For_بٹ(semantics->precision + 1);
}

ثابت APFloat::integerPart *IEEEFloat::significandParts() ثابت {
  واپس const_cast<IEEEFloat *>(یہ)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  اگر (part_گنتی() > 1)
    واپس significand.parts;
  ورنہ
    واپس &significand.part;
}

خالی IEEEFloat::zeroSignificand() {
  APInt::tc_سیٹ(significandParts(), 0, part_گنتی());
}

/* Increment an fcNormal floating point number's significand.  */
خالی IEEEFloat::incrementSignificand() {
  [[maybe_غیر_مستعمل]] integerPart حاصل =
      APInt::tcIncrement(significandParts(), part_گنتی());

  /* Our callers should never cause us to overflow.  */
  أكد(حاصل == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::جوڑو_Significand(ثابت IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  أكد(semantics == rhs.semantics);
  أكد(exponent == rhs.exponent);

  واپس APInt::tc_جوڑو(parts, rhs.significandParts(), 0, part_گنتی());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::تفریق_Significand(ثابت IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  أكد(semantics == rhs.semantics);
  أكد(exponent == rhs.exponent);

  واپس APInt::tc_تفریق(parts, rhs.significandParts(), borrow,
                           part_گنتی());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
گمشدہ_Fraction IEEEFloat::multiplySignificand(ثابت IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            منطقی ignoreAddend) {
  integerPart scratch[4];
  منطقی ignored;

  أكد(semantics == rhs.semantics);

  غیرعلامتی precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  غیرعلامتی new_Parts_گنتی = part_گنتی_For_بٹ(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *بھرا_Significand =
      new_Parts_گنتی > 4 ? نیا integerPart[new_Parts_گنتی] : scratch;

  integerPart *lhsSignificand = significandParts();
  غیرعلامتی parts_گنتی = part_گنتی();

  APInt::tc_بھرا_Multiply(بھرا_Significand, lhsSignificand,
                        rhs.significandParts(), parts_گنتی, parts_گنتی);

  گمشدہ_Fraction گمشدہ_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  غیرعلامتی omsb = APInt::tcMSB(بھرا_Significand, new_Parts_گنتی) + 1;
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
    غیرعلامتی extendedPrecision = 2 * precision + 1;
    اگر (omsb != extendedPrecision - 1) {
      أكد(extendedPrecision > omsb);
      APInt::tc_سرکاؤ_بایاں(بھرا_Significand, new_Parts_گنتی,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    اگر (new_Parts_گنتی == 1)
      significand.part = بھرا_Significand[0];
    ورنہ
      significand.parts = بھرا_Significand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_غیر_مستعمل]] op_حالت حالت_3 = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    أكد(حالت_3 == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    گمشدہ_fraction = extendedAddend.سرکاؤ_Significand_دایاں(1);
    أكد(گمشدہ_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    گمشدہ_fraction = جوڑو_Or_تفریق_Significand(extendedAddend, جھوٹ);

    /* Restore our state.  */
    اگر (new_Parts_گنتی == 1)
      بھرا_Significand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(بھرا_Significand, new_Parts_گنتی) + 1;
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
    غیرعلامتی صحیح بٹ, significantParts;
    گمشدہ_Fraction lf;

    بٹ = omsb - precision;
    significantParts = part_گنتی_For_بٹ(omsb);
    lf = سرکاؤ_دایاں(بھرا_Significand, significantParts, بٹ);
    گمشدہ_fraction = combine_گمشدہ_Fractions(lf, گمشدہ_fraction);
    exponent += بٹ;
  }

  APInt::tc_تفویض(lhsSignificand, بھرا_Significand, parts_گنتی);

  اگر (new_Parts_گنتی > 4)
    حذف [] بھرا_Significand;

  واپس گمشدہ_fraction;
}

گمشدہ_Fraction IEEEFloat::multiplySignificand(ثابت IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  واپس multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
گمشدہ_Fraction IEEEFloat::divideSignificand(ثابت IEEEFloat &rhs) {
  integerPart scratch[4];

  أكد(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  ثابت integerPart *rhsSignificand = rhs.significandParts();
  غیرعلامتی parts_گنتی = part_گنتی();

  integerPart *dividend =
      parts_گنتی > 2 ? نیا integerPart[parts_گنتی * 2] : scratch;
  integerPart *مقسوم_علیہ = dividend + parts_گنتی;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  برائے (غیرعلامتی i = 0; i < parts_گنتی; i++) {
    dividend[i] = lhsSignificand[i];
    مقسوم_علیہ[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  غیرعلامتی صحیح precision = semantics->precision;

  /* Normalize the divisor.  */
  غیرعلامتی ثنائی_ہندسہ = precision - APInt::tcMSB(مقسوم_علیہ, parts_گنتی) - 1;
  اگر (ثنائی_ہندسہ) {
    exponent += ثنائی_ہندسہ;
    APInt::tc_سرکاؤ_بایاں(مقسوم_علیہ, parts_گنتی, ثنائی_ہندسہ);
  }

  /* Normalize the dividend.  */
  ثنائی_ہندسہ = precision - APInt::tcMSB(dividend, parts_گنتی) - 1;
  اگر (ثنائی_ہندسہ) {
    exponent -= ثنائی_ہندسہ;
    APInt::tc_سرکاؤ_بایاں(dividend, parts_گنتی, ثنائی_ہندسہ);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  اگر (APInt::tcCompare(dividend, مقسوم_علیہ, parts_گنتی) < 0) {
    exponent--;
    APInt::tc_سرکاؤ_بایاں(dividend, parts_گنتی, 1);
    أكد(APInt::tcCompare(dividend, مقسوم_علیہ, parts_گنتی) >= 0);
  }

  /* Long division.  */
  برائے (ثنائی_ہندسہ = precision; ثنائی_ہندسہ; ثنائی_ہندسہ -= 1) {
    اگر (APInt::tcCompare(dividend, مقسوم_علیہ, parts_گنتی) >= 0) {
      APInt::tc_تفریق(dividend, مقسوم_علیہ, 0, parts_گنتی);
      APInt::tc_سیٹ_ثنائی_ہندسہ(lhsSignificand, ثنائی_ہندسہ - 1);
    }

    APInt::tc_سرکاؤ_بایاں(dividend, parts_گنتی, 1);
  }

  /* Figure out the lost fraction.  */
  صحیح cmp = APInt::tcCompare(dividend, مقسوم_علیہ, parts_گنتی);

  گمشدہ_Fraction گمشدہ_fraction;
  اگر (cmp > 0)
    گمشدہ_fraction = lf_مزید_Than_Half;
  ورنہ اگر (cmp == 0)
    گمشدہ_fraction = lfExactlyHalf;
  ورنہ اگر (APInt::tcIsZero(dividend, parts_گنتی))
    گمشدہ_fraction = lfExactlyZero;
  ورنہ
    گمشدہ_fraction = lfLessThanHalf;

  اگر (parts_گنتی > 2)
    حذف [] dividend;

  واپس گمشدہ_fraction;
}

غیرعلامتی صحیح IEEEFloat::significandMSB() ثابت {
  واپس APInt::tcMSB(significandParts(), part_گنتی());
}

غیرعلامتی صحیح IEEEFloat::significandLSB() ثابت {
  واپس APInt::tcLSB(significandParts(), part_گنتی());
}

/* Note that a zero result is NOT normalized to fcZero.  */
گمشدہ_Fraction IEEEFloat::سرکاؤ_Significand_دایاں(غیرعلامتی صحیح بٹ) {
  /* Our exponent should not overflow.  */
  أكد((ExponentType) (exponent + بٹ) >= exponent);

  exponent += بٹ;

  واپس سرکاؤ_دایاں(significandParts(), part_گنتی(), بٹ);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
خالی IEEEFloat::سرکاؤ_Significand_بایاں(غیرعلامتی صحیح بٹ) {
  أكد(بٹ < semantics->precision ||
         (semantics->precision == 1 && بٹ <= 1));

  اگر (بٹ) {
    غیرعلامتی صحیح parts_گنتی = part_گنتی();

    APInt::tc_سرکاؤ_بایاں(significandParts(), parts_گنتی, بٹ);
    exponent -= بٹ;

    أكد(!APInt::tcIsZero(significandParts(), parts_گنتی));
  }
}

APFloat::cmp_نتیجہ IEEEFloat::compare_Absolute_قدر(ثابت IEEEFloat &rhs) ثابت {
  أكد(semantics == rhs.semantics);
  أكد(isFiniteNonZero());
  أكد(rhs.isFiniteNonZero());

  صحیح compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  اگر (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_گنتی());

  اگر (compare > 0)
    واپس cmpGreaterThan;
  ورنہ اگر (compare < 0)
    واپس cmpLessThan;
  ورنہ
    واپس cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
ساکن خالی tc_سیٹ_کم_سے_کم_Significant_بٹ(APInt::لفظ_Type *dst, غیرعلامتی parts,
                                      غیرعلامتی بٹ) {
  غیرعلامتی i = 0;
  جب (بٹ > APInt::APINT_بٹ_فی_لفظ) {
    dst[i++] = ~(APInt::لفظ_Type)0;
    بٹ -= APInt::APINT_بٹ_فی_لفظ;
  }

  اگر (بٹ)
    dst[i++] = ~(APInt::لفظ_Type)0 >> (APInt::APINT_بٹ_فی_لفظ - بٹ);

  جب (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::op_حالت IEEEFloat::handle_حد_سے_تجاوز(rounding_منوال rounding_منوال_2) {
  اگر (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    اگر (rounding_منوال_2 == rm_قریب_ترین_Ties_مقصد_Even ||
        rounding_منوال_2 == rm_قریب_ترین_Ties_مقصد_Away ||
        (rounding_منوال_2 == rm_Toward_مثبت && !sign) ||
        (rounding_منوال_2 == rmTowardNegative && sign)) {
      اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(جھوٹ, sign);
      ورنہ
        category = fcInfinity;
      واپس static_cast<op_حالت>(op_حد_سے_تجاوز | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fc_معمول;
  exponent = semantics->maxExponent;
  tc_سیٹ_کم_سے_کم_Significant_بٹ(significandParts(), part_گنتی(),
                            semantics->precision);
  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tc_Clear_ثنائی_ہندسہ(significandParts(), 0);

  واپس opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
منطقی IEEEFloat::باری_Away_From_Zero(rounding_منوال rounding_منوال_2,
                                  گمشدہ_Fraction گمشدہ_fraction,
                                  غیرعلامتی صحیح ثنائی_ہندسہ) ثابت {
  /* NaNs and infinities should not have lost fractions.  */
  أكد(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  أكد(گمشدہ_fraction != lfExactlyZero);

  انتخاب (rounding_منوال_2) {
  حالت rm_قریب_ترین_Ties_مقصد_Away:
    واپس گمشدہ_fraction == lfExactlyHalf || گمشدہ_fraction == lf_مزید_Than_Half;

  حالت rm_قریب_ترین_Ties_مقصد_Even:
    اگر (گمشدہ_fraction == lf_مزید_Than_Half)
      واپس سچ;

    /* Our zeroes don't have a significand to test.  */
    اگر (گمشدہ_fraction == lfExactlyHalf && category != fcZero)
      واپس APInt::tc_نکالو_ثنائی_ہندسہ(significandParts(), ثنائی_ہندسہ);

    واپس جھوٹ;

  حالت rmTowardZero:
    واپس جھوٹ;

  حالت rm_Toward_مثبت:
    واپس !sign;

  حالت rmTowardNegative:
    واپس sign;

  طےشدہ:
    توڑو;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::op_حالت IEEEFloat::normalize(rounding_منوال rounding_منوال_2,
                                       گمشدہ_Fraction گمشدہ_fraction) {
  اگر (!isFiniteNonZero())
    واپس opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  غیرعلامتی omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  اگر (omsb || گمشدہ_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    صحیح exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    اگر (exponent + exponentChange > semantics->maxExponent)
      واپس handle_حد_سے_تجاوز(rounding_منوال_2);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    اگر (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    اگر (exponentChange < 0) {
      أكد(گمشدہ_fraction == lfExactlyZero);

      سرکاؤ_Significand_بایاں(-exponentChange);

      واپس opOK;
    }

    اگر (exponentChange > 0) {
      گمشدہ_Fraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = سرکاؤ_Significand_دایاں(exponentChange);

      گمشدہ_fraction = combine_گمشدہ_Fractions(lf, گمشدہ_fraction);

      /* Keep OMSB up-to-date.  */
      اگر (omsb > (غیرعلامتی) exponentChange)
        omsb -= exponentChange;
      ورنہ
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    واپس handle_حد_سے_تجاوز(rounding_منوال_2);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  اگر (گمشدہ_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    اگر (omsb == 0) {
      category = fcZero;
      اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = جھوٹ;
      اگر (!semantics->hasZero)
        make_سب_سے_چھوٹا_Normalized(جھوٹ);
    }

    واپس opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  اگر (باری_Away_From_Zero(rounding_منوال_2, گمشدہ_fraction, 0)) {
    اگر (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    اگر (omsb == (غیرعلامتی) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      اگر (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        واپس handle_حد_سے_تجاوز(sign ? rmTowardNegative : rm_Toward_مثبت);

      سرکاؤ_Significand_دایاں(1);

      واپس opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      واپس handle_حد_سے_تجاوز(rounding_منوال_2);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  اگر (omsb == semantics->precision)
    واپس opInexact;

  /* We have a non-zero denormal.  */
  أكد(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  اگر (omsb == 0) {
    category = fcZero;
    اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = جھوٹ;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    اگر (!semantics->hasZero)
      make_سب_سے_چھوٹا_Normalized(جھوٹ);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  واپس (op_حالت) (opUnderflow | opInexact);
}

APFloat::op_حالت IEEEFloat::جوڑو_Or_تفریق_Specials(ثابت IEEEFloat &rhs,
                                                   منطقی تفریق) {
  انتخاب (Pack_Categories_Into_کلید(category, rhs.category)) {
  طےشدہ:
    llvm_unreachable(خالی_اشاریہ);

  حالت Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالت Pack_Categories_Into_کلید(fc_معمول, fcNaN):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    تفویض(rhs);
    [[fallthrough]];
  حالت Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالت Pack_Categories_Into_کلید(fcNaN, fc_معمول):
  حالت Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNaN):
    اگر (isSignaling()) {
      makeQuiet();
      واپس opInvalidOp;
    }
    واپس rhs.isSignaling() ? opInvalidOp : opOK;

  حالت Pack_Categories_Into_کلید(fc_معمول, fcZero):
  حالت Pack_Categories_Into_کلید(fcInfinity, fc_معمول):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcZero):
    واپس opOK;

  حالت Pack_Categories_Into_کلید(fc_معمول, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ تفریق;
    واپس opOK;

  حالت Pack_Categories_Into_کلید(fcZero, fc_معمول):
    تفویض(rhs);
    sign = rhs.sign ^ تفریق;
    واپس opOK;

  حالت Pack_Categories_Into_کلید(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    واپس opOK;

  حالت Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    اگر (((sign ^ rhs.sign)!=0) != تفریق) {
      makeNaN();
      واپس opInvalidOp;
    }

    واپس opOK;

  حالت Pack_Categories_Into_کلید(fc_معمول, fc_معمول):
    واپس opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
گمشدہ_Fraction IEEEFloat::جوڑو_Or_تفریق_Significand(ثابت IEEEFloat &rhs,
                                                 منطقی تفریق) {
  [[maybe_غیر_مستعمل]] integerPart حاصل = 0;
  گمشدہ_Fraction گمشدہ_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  تفریق ^= static_cast<منطقی>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  صحیح بٹ = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  اگر (تفریق) {
    اگر ((بٹ < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat عارضی_rhs(rhs);
    منطقی گمشدہ_fraction_is_from_rhs = جھوٹ;

    اگر (بٹ == 0)
      گمشدہ_fraction = lfExactlyZero;
    ورنہ اگر (بٹ > 0) {
      گمشدہ_fraction = عارضی_rhs.سرکاؤ_Significand_دایاں(بٹ - 1);
      گمشدہ_fraction_is_from_rhs = سچ;
      سرکاؤ_Significand_بایاں(1);
    } ورنہ {
      گمشدہ_fraction = سرکاؤ_Significand_دایاں(-بٹ - 1);
      عارضی_rhs.سرکاؤ_Significand_بایاں(1);
    }

    // Should we reverse the subtraction.
    cmp_نتیجہ cmp_نتیجہ_2 = compare_Absolute_قدر(عارضی_rhs);
    اگر (cmp_نتیجہ_2 == cmpLessThan) {
      منطقی borrow =
          گمشدہ_fraction != lfExactlyZero && !گمشدہ_fraction_is_from_rhs;
      اگر (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        اگر (گمشدہ_fraction == lfLessThanHalf)
          گمشدہ_fraction = lf_مزید_Than_Half;
        ورنہ اگر (گمشدہ_fraction == lf_مزید_Than_Half)
          گمشدہ_fraction = lfLessThanHalf;
      }
      حاصل = عارضی_rhs.تفریق_Significand(*یہ, borrow);
      copySignificand(عارضی_rhs);
      sign = !sign;
    } ورنہ اگر (cmp_نتیجہ_2 == cmpGreaterThan) {
      منطقی borrow = گمشدہ_fraction != lfExactlyZero && گمشدہ_fraction_is_from_rhs;
      اگر (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        اگر (گمشدہ_fraction == lfLessThanHalf)
          گمشدہ_fraction = lf_مزید_Than_Half;
        ورنہ اگر (گمشدہ_fraction == lf_مزید_Than_Half)
          گمشدہ_fraction = lfLessThanHalf;
      }
      حاصل = تفریق_Significand(عارضی_rhs, borrow);
    } ورنہ { // cmpEqual
      zeroSignificand();
      اگر (گمشدہ_fraction != lfExactlyZero && گمشدہ_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    أكد(!حاصل);
  } ورنہ {
    اگر (بٹ > 0) {
      IEEEFloat عارضی_rhs(rhs);

      گمشدہ_fraction = عارضی_rhs.سرکاؤ_Significand_دایاں(بٹ);
      حاصل = جوڑو_Significand(عارضی_rhs);
    } ورنہ {
      گمشدہ_fraction = سرکاؤ_Significand_دایاں(-بٹ);
      حاصل = جوڑو_Significand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    أكد(!حاصل);
  }

  واپس گمشدہ_fraction;
}

APFloat::op_حالت IEEEFloat::multiplySpecials(ثابت IEEEFloat &rhs) {
  انتخاب (Pack_Categories_Into_کلید(category, rhs.category)) {
  طےشدہ:
    llvm_unreachable(خالی_اشاریہ);

  حالت Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالت Pack_Categories_Into_کلید(fc_معمول, fcNaN):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    تفویض(rhs);
    sign = جھوٹ;
    [[fallthrough]];
  حالت Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالت Pack_Categories_Into_کلید(fcNaN, fc_معمول):
  حالت Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    اگر (isSignaling()) {
      makeQuiet();
      واپس opInvalidOp;
    }
    واپس rhs.isSignaling() ? opInvalidOp : opOK;

  حالت Pack_Categories_Into_کلید(fc_معمول, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcInfinity, fc_معمول):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
    category = fcInfinity;
    واپس opOK;

  حالت Pack_Categories_Into_کلید(fcZero, fc_معمول):
  حالت Pack_Categories_Into_کلید(fc_معمول, fcZero):
  حالت Pack_Categories_Into_کلید(fcZero, fcZero):
    category = fcZero;
    واپس opOK;

  حالت Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcZero):
    makeNaN();
    واپس opInvalidOp;

  حالت Pack_Categories_Into_کلید(fc_معمول, fc_معمول):
    واپس opOK;
  }
}

APFloat::op_حالت IEEEFloat::divideSpecials(ثابت IEEEFloat &rhs) {
  انتخاب (Pack_Categories_Into_کلید(category, rhs.category)) {
  طےشدہ:
    llvm_unreachable(خالی_اشاریہ);

  حالت Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالت Pack_Categories_Into_کلید(fc_معمول, fcNaN):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    تفویض(rhs);
    sign = جھوٹ;
    [[fallthrough]];
  حالت Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالت Pack_Categories_Into_کلید(fcNaN, fc_معمول):
  حالت Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    اگر (isSignaling()) {
      makeQuiet();
      واپس opInvalidOp;
    }
    واپس rhs.isSignaling() ? opInvalidOp : opOK;

  حالت Pack_Categories_Into_کلید(fcInfinity, fcZero):
  حالت Pack_Categories_Into_کلید(fcInfinity, fc_معمول):
  حالت Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fc_معمول):
    واپس opOK;

  حالت Pack_Categories_Into_کلید(fc_معمول, fcInfinity):
    category = fcZero;
    واپس opOK;

  حالت Pack_Categories_Into_کلید(fc_معمول, fcZero):
    اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(جھوٹ, sign);
    ورنہ
      category = fcInfinity;
    واپس opDivByZero;

  حالت Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fcZero):
    makeNaN();
    واپس opInvalidOp;

  حالت Pack_Categories_Into_کلید(fc_معمول, fc_معمول):
    واپس opOK;
  }
}

APFloat::op_حالت IEEEFloat::modSpecials(ثابت IEEEFloat &rhs) {
  انتخاب (Pack_Categories_Into_کلید(category, rhs.category)) {
  طےشدہ:
    llvm_unreachable(خالی_اشاریہ);

  حالت Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالت Pack_Categories_Into_کلید(fc_معمول, fcNaN):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    تفویض(rhs);
    [[fallthrough]];
  حالت Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالت Pack_Categories_Into_کلید(fcNaN, fc_معمول):
  حالت Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNaN):
    اگر (isSignaling()) {
      makeQuiet();
      واپس opInvalidOp;
    }
    واپس rhs.isSignaling() ? opInvalidOp : opOK;

  حالت Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fc_معمول):
  حالت Pack_Categories_Into_کلید(fc_معمول, fcInfinity):
    واپس opOK;

  حالت Pack_Categories_Into_کلید(fc_معمول, fcZero):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcZero):
  حالت Pack_Categories_Into_کلید(fcInfinity, fc_معمول):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fcZero):
    makeNaN();
    واپس opInvalidOp;

  حالت Pack_Categories_Into_کلید(fc_معمول, fc_معمول):
    واپس opOK;
  }
}

APFloat::op_حالت IEEEFloat::remainderSpecials(ثابت IEEEFloat &rhs) {
  انتخاب (Pack_Categories_Into_کلید(category, rhs.category)) {
  طےشدہ:
    llvm_unreachable(خالی_اشاریہ);

  حالت Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالت Pack_Categories_Into_کلید(fc_معمول, fcNaN):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    تفویض(rhs);
    [[fallthrough]];
  حالت Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالت Pack_Categories_Into_کلید(fcNaN, fc_معمول):
  حالت Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNaN):
    اگر (isSignaling()) {
      makeQuiet();
      واپس opInvalidOp;
    }
    واپس rhs.isSignaling() ? opInvalidOp : opOK;

  حالت Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fc_معمول):
  حالت Pack_Categories_Into_کلید(fc_معمول, fcInfinity):
    واپس opOK;

  حالت Pack_Categories_Into_کلید(fc_معمول, fcZero):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcZero):
  حالت Pack_Categories_Into_کلید(fcInfinity, fc_معمول):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fcZero):
    makeNaN();
    واپس opInvalidOp;

  حالت Pack_Categories_Into_کلید(fc_معمول, fc_معمول):
    واپس opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
خالی IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    واپس;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::op_حالت IEEEFloat::جوڑو_Or_تفریق(ثابت IEEEFloat &rhs,
                                           rounding_منوال rounding_منوال_2,
                                           منطقی تفریق) {
  op_حالت fs = جوڑو_Or_تفریق_Specials(rhs, تفریق);

  /* This return code means it was not a simple case.  */
  اگر (fs == opDivByZero) {
    گمشدہ_Fraction گمشدہ_fraction;

    گمشدہ_fraction = جوڑو_Or_تفریق_Significand(rhs, تفریق);
    fs = normalize(rounding_منوال_2, گمشدہ_fraction);

    /* Can only be zero if we lost no fraction.  */
    أكد(category != fcZero || گمشدہ_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  اگر (category == fcZero) {
    اگر (rhs.category != fcZero || (sign == rhs.sign) == تفریق)
      sign = (rounding_منوال_2 == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = جھوٹ;
  }

  واپس fs;
}

/* Normalized addition.  */
APFloat::op_حالت IEEEFloat::جوڑو(ثابت IEEEFloat &rhs,
                                 rounding_منوال rounding_منوال_2) {
  واپس جوڑو_Or_تفریق(rhs, rounding_منوال_2, جھوٹ);
}

/* Normalized subtraction.  */
APFloat::op_حالت IEEEFloat::تفریق(ثابت IEEEFloat &rhs,
                                      rounding_منوال rounding_منوال_2) {
  واپس جوڑو_Or_تفریق(rhs, rounding_منوال_2, سچ);
}

/* Normalized multiply.  */
APFloat::op_حالت IEEEFloat::multiply(ثابت IEEEFloat &rhs,
                                      rounding_منوال rounding_منوال_2) {
  sign ^= rhs.sign;
  op_حالت fs = multiplySpecials(rhs);

  اگر (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = جھوٹ;
  اگر (isFiniteNonZero()) {
    گمشدہ_Fraction گمشدہ_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_منوال_2, گمشدہ_fraction);
    اگر (گمشدہ_fraction != lfExactlyZero)
      fs = (op_حالت) (fs | opInexact);
  }

  واپس fs;
}

/* Normalized divide.  */
APFloat::op_حالت IEEEFloat::divide(ثابت IEEEFloat &rhs,
                                    rounding_منوال rounding_منوال_2) {
  sign ^= rhs.sign;
  op_حالت fs = divideSpecials(rhs);

  اگر (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = جھوٹ;
  اگر (isFiniteNonZero()) {
    گمشدہ_Fraction گمشدہ_fraction = divideSignificand(rhs);
    fs = normalize(rounding_منوال_2, گمشدہ_fraction);
    اگر (گمشدہ_fraction != lfExactlyZero)
      fs = (op_حالت) (fs | opInexact);
  }

  واپس fs;
}

/* Normalized remainder.  */
APFloat::op_حالت IEEEFloat::remainder(ثابت IEEEFloat &rhs) {
  غیرعلامتی صحیح origSign = sign;

  // First handle the special cases.
  op_حالت fs = remainderSpecials(rhs);
  اگر (fs != opDivByZero)
    واپس fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  اگر (P2.جوڑو(rhs, rm_قریب_ترین_Ties_مقصد_Even) == opOK) {
    fs = mod(P2);
    أكد(fs == opOK);
  }

  // Lets work with absolute numbers.
  IEEEFloat P = rhs;
  P.sign = جھوٹ;
  sign = جھوٹ;

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

  IEEEFloat VEx = *یہ;
  fs = VEx.convert(extendedSemantics, rm_قریب_ترین_Ties_مقصد_Even, &losesInfo);
  أكد(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rm_قریب_ترین_Ties_مقصد_Even, &losesInfo);
  أكد(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.جوڑو(VEx, rm_قریب_ترین_Ties_مقصد_Even);
  أكد(fs == opOK);

  اگر (VEx.compare(PEx) == cmpGreaterThan) {
    fs = تفریق(P, rm_قریب_ترین_Ties_مقصد_Even);
    أكد(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.تفریق(PEx, rm_قریب_ترین_Ties_مقصد_Even);
    أكد(fs == opOK);
    fs = VEx.تفریق(PEx, rm_قریب_ترین_Ties_مقصد_Even);
    أكد(fs == opOK);

    cmp_نتیجہ نتیجہ_2 = VEx.compare(PEx);
    اگر (نتیجہ_2 == cmpGreaterThan || نتیجہ_2 == cmpEqual) {
      fs = تفریق(P, rm_قریب_ترین_Ties_مقصد_Even);
      أكد(fs == opOK);
    }
  }

  اگر (isZero()) {
    sign = origSign;    // IEEE754 requires this
    اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = جھوٹ;
  } ورنہ {
    sign ^= origSign;
  }
  واپس fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::op_حالت IEEEFloat::mod(ثابت IEEEFloat &rhs) {
  op_حالت fs = modSpecials(rhs);
  غیرعلامتی صحیح origSign = sign;

  جب (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_قدر(rhs) != cmpLessThan) {
    صحیح Exp = ilogb(*یہ) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rm_قریب_ترین_Ties_مقصد_Even);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    اگر (V.isNaN() || compare_Absolute_قدر(V) == cmpLessThan)
      V = scalbn(rhs, Exp - 1, rm_قریب_ترین_Ties_مقصد_Even);
    V.sign = sign;

    fs = تفریق(V, rm_قریب_ترین_Ties_مقصد_Even);

    // When the semantics supports zero, this loop's
    // exit-condition is handled by the 'isFiniteNonZero'
    // category check above. However, when the semantics
    // does not have 'fcZero' and we have reached the
    // minimum possible value, (and any further subtract
    // will underflow to the same value) explicitly
    // provide an exit-path here.
    اگر (!semantics->hasZero && یہ->is_سب_سے_چھوٹا())
      توڑو;

    أكد(fs==opOK);
  }
  اگر (isZero()) {
    sign = origSign; // fmod requires this
    اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = جھوٹ;
  }
  واپس fs;
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
  اگر (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    گمشدہ_Fraction گمشدہ_fraction;

    گمشدہ_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_منوال_2, گمشدہ_fraction);
    اگر (گمشدہ_fraction != lfExactlyZero)
      fs = (op_حالت) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    اگر (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_منوال_2 == rmTowardNegative);
      اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = جھوٹ;
    }
  } ورنہ {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    اگر (fs == opOK)
      fs = جوڑو_Or_تفریق(addend, rounding_منوال_2, جھوٹ);
  }

  واپس fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::op_حالت IEEEFloat::باری_مقصد_Integral(rounding_منوال rounding_منوال_2) {
  اگر (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    واپس opOK;

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
      واپس opInvalidOp;
    } ورنہ {
      // [IEEE Std 754-2008 6.2]:
      // For an operation with quiet NaN inputs, other than maximum and minimum
      // operations, if a floating-point result is to be delivered the result
      // shall be a quiet NaN which should be one of the input NaNs.
      // ...
      // Every general-computational and quiet-computational operation involving
      // one or more input NaNs, none of them signaling, shall signal no
      // exception, except fusedMultiplyAdd might signal the invalid operation
      // exception(see 7.2).
      واپس opOK;
    }
  }

  اگر (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    واپس opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  اگر (exponent + 1 >= (صحیح)APFloat::semanticsPrecision(*semantics))
    واپس opOK;

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
  op_حالت fs = MagicConstant.convertFromAPInt(IntegerConstant, جھوٹ,
                                               rm_قریب_ترین_Ties_مقصد_Even);
  أكد(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  منطقی داخلہ_Sign = isNegative();

  fs = جوڑو(MagicConstant, rounding_منوال_2);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  تفریق(MagicConstant, rounding_منوال_2);

  // Restore the input sign.
  اگر (داخلہ_Sign != isNegative())
    changeSign();

  واپس fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmp_نتیجہ IEEEFloat::compare(ثابت IEEEFloat &rhs) ثابت {
  أكد(semantics == rhs.semantics);

  انتخاب (Pack_Categories_Into_کلید(category, rhs.category)) {
  طےشدہ:
    llvm_unreachable(خالی_اشاریہ);

  حالت Pack_Categories_Into_کلید(fcNaN, fcZero):
  حالت Pack_Categories_Into_کلید(fcNaN, fc_معمول):
  حالت Pack_Categories_Into_کلید(fcNaN, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcNaN, fcNaN):
  حالت Pack_Categories_Into_کلید(fcZero, fcNaN):
  حالت Pack_Categories_Into_کلید(fc_معمول, fcNaN):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcNaN):
    واپس cmpUnordered;

  حالت Pack_Categories_Into_کلید(fcInfinity, fc_معمول):
  حالت Pack_Categories_Into_کلید(fcInfinity, fcZero):
  حالت Pack_Categories_Into_کلید(fc_معمول, fcZero):
    اگر (sign)
      واپس cmpLessThan;
    ورنہ
      واپس cmpGreaterThan;

  حالت Pack_Categories_Into_کلید(fc_معمول, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fcInfinity):
  حالت Pack_Categories_Into_کلید(fcZero, fc_معمول):
    اگر (rhs.sign)
      واپس cmpGreaterThan;
    ورنہ
      واپس cmpLessThan;

  حالت Pack_Categories_Into_کلید(fcInfinity, fcInfinity):
    اگر (sign == rhs.sign)
      واپس cmpEqual;
    ورنہ اگر (sign)
      واپس cmpLessThan;
    ورنہ
      واپس cmpGreaterThan;

  حالت Pack_Categories_Into_کلید(fcZero, fcZero):
    واپس cmpEqual;

  حالت Pack_Categories_Into_کلید(fc_معمول, fc_معمول):
    توڑو;
  }

  cmp_نتیجہ نتیجہ_2;
  /* Two normal numbers.  Do they have the same sign?  */
  اگر (sign != rhs.sign) {
    اگر (sign)
      نتیجہ_2 = cmpLessThan;
    ورنہ
      نتیجہ_2 = cmpGreaterThan;
  } ورنہ {
    /* Compare absolute values; invert result if negative.  */
    نتیجہ_2 = compare_Absolute_قدر(rhs);

    اگر (sign) {
      اگر (نتیجہ_2 == cmpLessThan)
        نتیجہ_2 = cmpGreaterThan;
      ورنہ اگر (نتیجہ_2 == cmpGreaterThan)
        نتیجہ_2 = cmpLessThan;
    }
  }

  واپس نتیجہ_2;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::op_حالت IEEEFloat::convert(ثابت fltSemantics &مقصد_Semantics,
                                     rounding_منوال rounding_منوال_2,
                                     منطقی *losesInfo) {
  op_حالت fs;
  ثابت fltSemantics &fromSemantics = *semantics;
  منطقی is_signaling = isSignaling();

  گمشدہ_Fraction گمشدہ_Fraction = lfExactlyZero;
  غیرعلامتی new_Part_گنتی = part_گنتی_For_بٹ(مقصد_Semantics.precision + 1);
  غیرعلامتی old_Part_گنتی = part_گنتی();
  صحیح سرکاؤ = مقصد_Semantics.precision - fromSemantics.precision;

  منطقی X86SpecialNan = جھوٹ;
  اگر (&fromSemantics == &APFloat_بنیاد::semX87DoubleExtended &&
      &مقصد_Semantics != &APFloat_بنیاد::semX87DoubleExtended && category == fcNaN &&
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
  اگر (سرکاؤ < 0 && isFiniteNonZero()) {
    صحیح omsb = significandMSB() + 1;
    صحیح exponentChange = omsb - fromSemantics.precision;
    اگر (exponent + exponentChange < مقصد_Semantics.minExponent)
      exponentChange = مقصد_Semantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, سرکاؤ);
    اگر (exponentChange < 0) {
      سرکاؤ -= exponentChange;
      exponent += exponentChange;
    } ورنہ اگر (omsb <= -سرکاؤ) {
      exponentChange = omsb + سرکاؤ - 1; // leave at least one bit set
      سرکاؤ -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  اگر (سرکاؤ < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    گمشدہ_Fraction = سرکاؤ_دایاں(significandParts(), old_Part_گنتی, -سرکاؤ);

  // Fix the storage so it can hold to new value.
  اگر (new_Part_گنتی > old_Part_گنتی) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = نیا integerPart[new_Part_گنتی];
    APInt::tc_سیٹ(newParts, 0, new_Part_گنتی);
    اگر (isFiniteNonZero() || category==fcNaN)
      APInt::tc_تفویض(newParts, significandParts(), old_Part_گنتی);
    freeSignificand();
    significand.parts = newParts;
  } ورنہ اگر (new_Part_گنتی == 1 && old_Part_گنتی != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    اگر (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &مقصد_Semantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  اگر (سرکاؤ > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_سرکاؤ_بایاں(significandParts(), new_Part_گنتی, سرکاؤ);

  اگر (isFiniteNonZero()) {
    fs = normalize(rounding_منوال_2, گمشدہ_Fraction);
    *losesInfo = (fs != opOK);
  } ورنہ اگر (category == fcNaN) {
    اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(جھوٹ, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } ورنہ {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      اگر (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(جھوٹ, جھوٹ);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      اگر (!APFloat::hasSignificand(fromSemantics))
        makeNaN(جھوٹ, sign);

      *losesInfo = گمشدہ_Fraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      اگر (!X86SpecialNan && semantics == &APFloat_بنیاد::semX87DoubleExtended)
        APInt::tc_سیٹ_ثنائی_ہندسہ(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      اگر (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } ورنہ {
        fs = opOK;
      }
    }
  } ورنہ اگر (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(جھوٹ, sign);
    *losesInfo = سچ;
    fs = opInexact;
  } ورنہ اگر (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = جھوٹ;
  } ورنہ {
    *losesInfo = جھوٹ;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  اگر ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = سچ;
    اگر (fs == opOK)
      fs = opInexact;
  }

  اگر (category == fcZero && !semantics->hasZero)
    make_سب_سے_چھوٹا_Normalized(جھوٹ);
  واپس fs;
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
APFloat::op_حالت IEEEFloat::convert_مقصد_Sign_Extended_Integer(
    MutableArrayRef<integerPart> parts, غیرعلامتی صحیح چوڑائی_2, منطقی isSigned,
    rounding_منوال rounding_منوال_2, منطقی *isExact) ثابت {
  *isExact = جھوٹ;

  /* Handle the three special cases first.  */
  اگر (category == fcInfinity || category == fcNaN)
    واپس opInvalidOp;

  غیرعلامتی dst_Parts_گنتی = part_گنتی_For_بٹ(چوڑائی_2);
  أكد(dst_Parts_گنتی <= parts.سائز_2() && "Integer too big");

  اگر (category == fcZero) {
    APInt::tc_سیٹ(parts.ڈیٹا(), 0, dst_Parts_گنتی);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    واپس opOK;
  }

  ثابت integerPart *src = significandParts();

  غیرعلامتی truncated_بٹ;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  اگر (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tc_سیٹ(parts.ڈیٹا(), 0, dst_Parts_گنتی);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncated_بٹ = semantics->precision -1U - exponent;
  } ورنہ {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    غیرعلامتی صحیح بٹ = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    اگر (بٹ > چوڑائی_2)
      واپس opInvalidOp;

    اگر (بٹ < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncated_بٹ = semantics->precision - بٹ;
      APInt::tc_نکالو(parts.ڈیٹا(), dst_Parts_گنتی, src, بٹ, truncated_بٹ);
    } ورنہ {
      /* We want at least as many bits as are available.  */
      APInt::tc_نکالو(parts.ڈیٹا(), dst_Parts_گنتی, src, semantics->precision,
                       0);
      APInt::tc_سرکاؤ_بایاں(parts.ڈیٹا(), dst_Parts_گنتی,
                         بٹ - semantics->precision);
      truncated_بٹ = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  گمشدہ_Fraction گمشدہ_fraction;
  اگر (truncated_بٹ) {
    گمشدہ_fraction = گمشدہ_Fraction_Through_Truncation(src, part_گنتی(),
                                                  truncated_بٹ);
    اگر (گمشدہ_fraction != lfExactlyZero &&
        باری_Away_From_Zero(rounding_منوال_2, گمشدہ_fraction, truncated_بٹ)) {
      اگر (APInt::tcIncrement(parts.ڈیٹا(), dst_Parts_گنتی))
        واپس opInvalidOp;     /* Overflow.  */
    }
  } ورنہ {
    گمشدہ_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  غیرعلامتی صحیح omsb = APInt::tcMSB(parts.ڈیٹا(), dst_Parts_گنتی) + 1;

  اگر (sign) {
    اگر (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      اگر (omsb != 0)
        واپس opInvalidOp;
    } ورنہ {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      اگر (omsb == چوڑائی_2 &&
          APInt::tcLSB(parts.ڈیٹا(), dst_Parts_گنتی) + 1 != omsb)
        واپس opInvalidOp;

      /* This case can happen because of rounding.  */
      اگر (omsb > چوڑائی_2)
        واپس opInvalidOp;
    }

    APInt::tcNegate (parts.ڈیٹا(), dst_Parts_گنتی);
  } ورنہ {
    اگر (omsb >= چوڑائی_2 + !isSigned)
      واپس opInvalidOp;
  }

  اگر (گمشدہ_fraction == lfExactlyZero) {
    *isExact = سچ;
    واپس opOK;
  }
  واپس opInexact;
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
IEEEFloat::convert_مقصد_Integer(MutableArrayRef<integerPart> parts,
                            غیرعلامتی صحیح چوڑائی_2, منطقی isSigned,
                            rounding_منوال rounding_منوال_2, منطقی *isExact) ثابت {
  op_حالت fs = convert_مقصد_Sign_Extended_Integer(parts, چوڑائی_2, isSigned,
                                             rounding_منوال_2, isExact);

  اگر (fs == opInvalidOp) {
    غیرعلامتی صحیح بٹ, dst_Parts_گنتی;

    dst_Parts_گنتی = part_گنتی_For_بٹ(چوڑائی_2);
    أكد(dst_Parts_گنتی <= parts.سائز_2() && "Integer too big");

    اگر (category == fcNaN)
      بٹ = 0;
    ورنہ اگر (sign)
      بٹ = isSigned;
    ورنہ
      بٹ = چوڑائی_2 - isSigned;

    tc_سیٹ_کم_سے_کم_Significant_بٹ(parts.ڈیٹا(), dst_Parts_گنتی, بٹ);
    اگر (sign && isSigned)
      APInt::tc_سرکاؤ_بایاں(parts.ڈیٹا(), dst_Parts_گنتی, چوڑائی_2 - 1);
  }

  واپس fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::op_حالت IEEEFloat::convertFromUnsignedParts(
    ثابت integerPart *src, غیرعلامتی صحیح src_گنتی, rounding_منوال rounding_منوال_2) {
  category = fc_معمول;
  غیرعلامتی omsb = APInt::tcMSB(src, src_گنتی) + 1;
  integerPart *dst = significandParts();
  غیرعلامتی dst_گنتی = part_گنتی();
  غیرعلامتی precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  گمشدہ_Fraction گمشدہ_fraction;
  اگر (precision <= omsb) {
    exponent = omsb - 1;
    گمشدہ_fraction = گمشدہ_Fraction_Through_Truncation(src, src_گنتی,
                                                  omsb - precision);
    APInt::tc_نکالو(dst, dst_گنتی, src, precision, omsb - precision);
  } ورنہ {
    exponent = precision - 1;
    گمشدہ_fraction = lfExactlyZero;
    APInt::tc_نکالو(dst, dst_گنتی, src, omsb, 0);
  }

  واپس normalize(rounding_منوال_2, گمشدہ_fraction);
}

APFloat::op_حالت IEEEFloat::convertFromAPInt(ثابت APInt &Val, منطقی isSigned,
                                              rounding_منوال rounding_منوال_2) {
  غیرعلامتی صحیح part_گنتی = Val.get_عدد_الفاظ();
  APInt api = Val;

  sign = جھوٹ;
  اگر (isSigned && api.isNegative()) {
    sign = سچ;
    api = -api;
  }

  واپس convertFromUnsignedParts(api.get_Raw_ڈیٹا(), part_گنتی, rounding_منوال_2);
}

Expected<APFloat::op_حالت>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        rounding_منوال rounding_منوال_2) {
  گمشدہ_Fraction گمشدہ_fraction = lfExactlyZero;

  category = fc_معمول;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  غیرعلامتی parts_گنتی = part_گنتی();
  غیرعلامتی ثنائی_ہندسہ_Pos = parts_گنتی * integer_Part_چوڑائی;
  منطقی computedTrailingFraction = جھوٹ;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator انتہا = s.انتہا();
  StringRef::iterator dot;
  خودکار PtrOrErr = skipLeadingZeroesAndAnyDot(begin, انتہا, &dot);
  اگر (!PtrOrErr)
    واپس PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator پہلا_Significant_Digit = p;

  جب (p != انتہا) {
    integerPart hex_قدر;

    اگر (*p == '.') {
      اگر (dot != انتہا)
        واپس createError("String contains multiple dots");
      dot = p++;
      جاری;
    }

    hex_قدر = hex_Digit_قدر(*p);
    اگر (hex_قدر == UINT_MAX)
      توڑو;

    p++;

    // Store the number while we have space.
    اگر (ثنائی_ہندسہ_Pos) {
      ثنائی_ہندسہ_Pos -= 4;
      hex_قدر <<= ثنائی_ہندسہ_Pos % integer_Part_چوڑائی;
      significand[ثنائی_ہندسہ_Pos / integer_Part_چوڑائی] |= hex_قدر;
    } ورنہ اگر (!computedTrailingFraction) {
      خودکار FractOrErr = trailingHexadecimalFraction(p, انتہا, hex_قدر);
      اگر (!FractOrErr)
        واپس FractOrErr.takeError();
      گمشدہ_fraction = *FractOrErr;
      computedTrailingFraction = سچ;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  اگر (p == انتہا)
    واپس createError("Hex strings require an exponent");
  اگر (*p != 'p' && *p != 'P')
    واپس createError("Invalid character in significand");
  اگر (p == begin)
    واپس createError("Significand has no digits");
  اگر (dot != انتہا && p - begin == 1)
    واپس createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  اگر (p != پہلا_Significant_Digit) {
    صحیح expAdjustment;

    /* Implicit hexadecimal point?  */
    اگر (dot == انتہا)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<صحیح>(dot - پہلا_Significant_Digit);
    اگر (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_گنتی * integer_Part_چوڑائی;

    /* Adjust for the given exponent.  */
    خودکار ExpOrErr = کل_Exponent(p + 1, انتہا, expAdjustment);
    اگر (!ExpOrErr)
      واپس ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  واپس normalize(rounding_منوال_2, گمشدہ_fraction);
}

APFloat::op_حالت
IEEEFloat::باری_Significand_With_Exponent(ثابت integerPart *decSigParts,
                                        غیرعلامتی sig_Part_گنتی, صحیح قوت_نما,
                                        rounding_منوال rounding_منوال_2) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  غیرعلامتی صحیح power = قوت_نما >= 0 ? قوت_نما : -قوت_نما;
  غیرعلامتی صحیح pow5_Part_استعداد = power_کا_Five_Parts_For_Exponent(power);
  Small_سمتیہ<integerPart, عدد_Pow5_Parts_On_ڈھیر> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5_Part_استعداد);

  منطقی is_قریب_ترین = rounding_منوال_2 == rm_قریب_ترین_Ties_مقصد_Even ||
                   rounding_منوال_2 == rm_قریب_ترین_Ties_مقصد_Away;

  غیرعلامتی parts = part_گنتی_For_بٹ(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  غیرعلامتی pow5_Part_گنتی = powerOf5(pow5Parts.ڈیٹا(), power);

  برائے (;; parts *= 2) {
    غیرعلامتی صحیح excessPrecision, truncated_بٹ;

    calcSemantics.precision = parts * integer_Part_چوڑائی - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncated_بٹ = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    op_حالت sig_حالت = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_گنتی, rm_قریب_ترین_Ties_مقصد_Even);
    op_حالت pow_حالت = pow5.convertFromUnsignedParts(
        pow5Parts.ڈیٹا(), pow5_Part_گنتی, rm_قریب_ترین_Ties_مقصد_Even);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += قوت_نما;

    گمشدہ_Fraction calc_گمشدہ_Fraction;
    integerPart HUerr, HUdistance;
    غیرعلامتی صحیح powHUerr;

    اگر (قوت_نما >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calc_گمشدہ_Fraction = decSig.multiplySignificand(pow5);
      powHUerr = pow_حالت != opOK;
    } ورنہ {
      calc_گمشدہ_Fraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      اگر (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncated_بٹ = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (pow_حالت == opOK && calc_گمشدہ_Fraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    أكد(APInt::tc_نکالو_ثنائی_ہندسہ
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calc_گمشدہ_Fraction != lfExactlyZero, sig_حالت != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, is_قریب_ترین);

    /* Are we guaranteed to round correctly if we truncate?  */
    اگر (HUdistance >= HUerr) {
      APInt::tc_نکالو(significandParts(), part_گنتی(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calc_گمشدہ_Fraction = گمشدہ_Fraction_Through_Truncation(decSig.significandParts(),
                                                       decSig.part_گنتی(),
                                                       truncated_بٹ);
      واپس static_cast<op_حالت>(normalize(rounding_منوال_2, calc_گمشدہ_Fraction) |
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
  اگر (Error Err = interpretDecimal(p, str.انتہا(), &D))
    واپس std::move(Err);

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
  اگر (D.پہلا_Sig_Digit == str.انتہا() || dec_Digit_قدر(*D.پہلا_Sig_Digit) >= 10U) {
    category = fcZero;
    fs = opOK;
    اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = جھوٹ;
    اگر (!semantics->hasZero)
      make_سب_سے_چھوٹا_Normalized(جھوٹ);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } ورنہ اگر (D.normalizedExponent - 1 > زیادہ_سے_زیادہ_عدد / 42039) {
    fs = handle_حد_سے_تجاوز(rounding_منوال_2);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } ورنہ اگر (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (صحیح) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fc_معمول;
    zeroSignificand();
    fs = normalize(rounding_منوال_2, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } ورنہ اگر ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handle_حد_سے_تجاوز(rounding_منوال_2);
  } ورنہ {
    integerPart *decSignificand;
    غیرعلامتی صحیح part_گنتی;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_گنتی = static_cast<غیرعلامتی صحیح>(D.آخری_Sig_Digit - D.پہلا_Sig_Digit) + 1;
    part_گنتی = part_گنتی_For_بٹ(1 + 196 * part_گنتی / 59);
    decSignificand = نیا integerPart[part_گنتی + 1];
    part_گنتی = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    کرو {
      integerPart dec_قدر, val, multiplier;

      val = 0;
      multiplier = 1;

      کرو {
        اگر (*p == '.') {
          p++;
          اگر (p == str.انتہا()) {
            توڑو;
          }
        }
        dec_قدر = dec_Digit_قدر(*p++);
        اگر (dec_قدر >= 10U) {
          حذف[] decSignificand;
          واپس createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_قدر;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } جب (p <= D.آخری_Sig_Digit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_گنتی, part_گنتی + 1, جھوٹ);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      اگر (decSignificand[part_گنتی])
        part_گنتی++;
    } جب (p <= D.آخری_Sig_Digit);

    category = fc_معمول;
    fs = باری_Significand_With_Exponent(decSignificand, part_گنتی,
                                      D.exponent, rounding_منوال_2);

    حذف [] decSignificand;
  }

  واپس fs;
}

منطقی IEEEFloat::convertFromStringSpecials(StringRef str) {
  ثابت size_t MIN_NAME_سائز = 3;

  اگر (str.سائز_2() < MIN_NAME_سائز)
    واپس جھوٹ;

  اگر (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(جھوٹ);
    واپس سچ;
  }

  منطقی IsNegative = str.consume_front("-");
  اگر (IsNegative) {
    اگر (str.سائز_2() < MIN_NAME_سائز)
      واپس جھوٹ;

    اگر (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(سچ);
      واپس سچ;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  منطقی IsSignaling = str.consume_front_insensitive("s");
  اگر (IsSignaling) {
    اگر (str.سائز_2() < MIN_NAME_سائز)
      واپس جھوٹ;
  }

  اگر (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    اگر (str.خالی_خانے()) {
      makeNaN(IsSignaling, IsNegative);
      واپس سچ;
    }

    // Allow the payload to be inside parentheses.
    اگر (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      اگر (str.سائز_2() <= 2 || str.عقب() != ')')
        واپس جھوٹ;

      str = str.قاش(1, str.سائز_2() - 1);
    }

    // Determine the payload number's radix.
    غیرعلامتی Radix = 10;
    اگر (str[0] == '0') {
      اگر (str.سائز_2() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } ورنہ {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    اگر (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      واپس سچ;
    }
  }

  واپس جھوٹ;
}

Expected<APFloat::op_حالت>
IEEEFloat::convertFromString(StringRef str, rounding_منوال rounding_منوال_2) {
  اگر (str.خالی_خانے())
    واپس createError("Invalid string length");

  // Handle special cases.
  اگر (convertFromStringSpecials(str))
    واپس opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.سائز_2();
  sign = *p == '-' ? 1 : 0;
  اگر (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  اگر (*p == '-' || *p == '+') {
    p++;
    slen--;
    اگر (!slen)
      واپس createError("String has no digits");
  }

  اگر (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    اگر (slen == 2)
      واپس createError("Invalid string");
    واپس convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_منوال_2);
  }

  واپس convertFromDecimalString(StringRef(p, slen), rounding_منوال_2);
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
غیرعلامتی صحیح IEEEFloat::convert_مقصد_Hex_String(حرف *dst, غیرعلامتی صحیح hexDigits,
                                           منطقی بالائی_Case,
                                           rounding_منوال rounding_منوال_2) ثابت {
  حرف *p = dst;
  اگر (sign)
    *dst++ = '-';

  انتخاب (category) {
  حالت fcInfinity:
    حافظہ_نقل (dst, بالائی_Case ? infinityU: infinityL, حجم infinityU - 1);
    dst += حجم infinityL - 1;
    توڑو;

  حالت fcNaN:
    حافظہ_نقل (dst, بالائی_Case ? NaNU: NaNL, حجم NaNU - 1);
    dst += حجم NaNU - 1;
    توڑو;

  حالت fcZero:
    *dst++ = '0';
    *dst++ = بالائی_Case ? 'X': 'x';
    *dst++ = '0';
    اگر (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = بالائی_Case ? 'P': 'p';
    *dst++ = '0';
    توڑو;

  حالت fc_معمول:
    dst = convert_معمول_مقصد_Hex_String (dst, hexDigits, بالائی_Case, rounding_منوال_2);
    توڑو;
  }

  *dst = 0;

  واپس static_cast<غیرعلامتی صحیح>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
حرف *IEEEFloat::convert_معمول_مقصد_Hex_String(حرف *dst, غیرعلامتی صحیح hexDigits,
                                          منطقی بالائی_Case,
                                          rounding_منوال rounding_منوال_2) ثابت {
  *dst++ = '0';
  *dst++ = بالائی_Case ? 'X': 'x';

  منطقی باری_اوپر = جھوٹ;
  ثابت حرف *hexDigitChars = بالائی_Case ? hex_Digits_بالائی : hex_Digits_زیریں;

  ثابت integerPart *significand = significandParts();
  غیرعلامتی parts_گنتی = part_گنتی();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  غیرعلامتی قدر_بٹ = semantics->precision + 3;
  غیرعلامتی سرکاؤ = integer_Part_چوڑائی - قدر_بٹ % integer_Part_چوڑائی;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  غیرعلامتی outputDigits = (قدر_بٹ - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  اگر (hexDigits) {
    اگر (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      غیرعلامتی صحیح بٹ;
      گمشدہ_Fraction fraction;

      بٹ = قدر_بٹ - hexDigits * 4;
      fraction = گمشدہ_Fraction_Through_Truncation (significand, parts_گنتی, بٹ);
      باری_اوپر = باری_Away_From_Zero(rounding_منوال_2, fraction, بٹ);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  حرف *p = ++dst;

  غیرعلامتی گنتی = (قدر_بٹ + integer_Part_چوڑائی - 1) / integer_Part_چوڑائی;

  جب (outputDigits && گنتی) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    اگر (--گنتی == parts_گنتی)
      part = 0;  /* An imaginary higher zero part.  */
    ورنہ
      part = significand[گنتی] << سرکاؤ;

    اگر (گنتی && سرکاؤ)
      part |= significand[گنتی - 1] >> (integer_Part_چوڑائی - سرکاؤ);

    /* Convert as much of "part" to hexdigits as we can.  */
    غیرعلامتی صحیح curDigits = integer_Part_چوڑائی / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  اگر (باری_اوپر) {
    حرف *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    کرو {
      q--;
      *q = hexDigitChars[hex_Digit_قدر (*q) + 1];
    } جب (*q == '0');
    أكد(q >= p);
  } ورنہ {
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
  ورنہ
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = بالائی_Case ? 'P': 'p';

  واپس writeSignedDecimal (dst, exponent);
}

hash_رمز hash_قدر(ثابت IEEEFloat &Arg) {
  اگر (!Arg.isFiniteNonZero())
    واپس hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  واپس hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
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

APInt IEEEFloat::convert_F80_Long_Double_APFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی() == 2);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semX87DoubleExtended>();
}

APInt IEEEFloat::convert_PPCDouble_Double_Legacy_APFloat_مقصد_APInt() ثابت {
  أكد(semantics ==
         (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semPPCDoubleDoubleLegacy);
  أكد(part_گنتی()==2);

  uint64_t الفاظ[2];
  منطقی losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloat_بنیاد::semIEEEdouble.minExponent;
  IEEEFloat extended(*یہ);
  [[maybe_غیر_مستعمل]] op_حالت fs =
      extended.convert(extendedSemantics, rm_قریب_ترین_Ties_مقصد_Even, &losesInfo);
  أكد(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloat_بنیاد::semIEEEdouble, rm_قریب_ترین_Ties_مقصد_Even, &losesInfo);
  أكد(fs == opOK || fs == opInexact);
  الفاظ[0] = *u.convert_Double_APFloat_مقصد_APInt().get_Raw_ڈیٹا();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  اگر (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rm_قریب_ترین_Ties_مقصد_Even, &losesInfo);
    أكد(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.تفریق(u, rm_قریب_ترین_Ties_مقصد_Even);
    fs = v.convert(APFloat_بنیاد::semIEEEdouble, rm_قریب_ترین_Ties_مقصد_Even, &losesInfo);
    أكد(fs == opOK && !losesInfo);
    الفاظ[1] = *v.convert_Double_APFloat_مقصد_APInt().get_Raw_ڈیٹا();
  } ورنہ {
    الفاظ[1] = 0;
  }

  واپس APInt(128, الفاظ);
}

سانچہ <ثابت fltSemantics &S>
APInt IEEEFloat::convert_IEEEFloat_مقصد_APInt() ثابت {
  أكد(semantics == &S);
  constexpr غیرعلامتی صحیح trailing_significand_بٹ =
      S.precision - 1 + S.has_Explicit_Integer_ثنائی_ہندسہ;
  constexpr صحیح integer_ثنائی_ہندسہ_part = (S.precision - 1) / integer_Part_چوڑائی;
  constexpr integerPart integer_ثنائی_ہندسہ = integerPart{1}
                                      << ((S.precision - 1) % integer_Part_چوڑائی);
  constexpr uint64_t significand_نقاب = integer_ثنائی_ہندسہ - 1;
  constexpr غیرعلامتی صحیح exponent_بٹ =
      S.سائز_اندر_بٹ - (S.hasSignedRepr ? 1 : 0) - trailing_significand_بٹ;
  static_assert(exponent_بٹ < 64);
  constexpr uint64_t exponent_نقاب = (uint64_t{1} << exponent_بٹ) - 1;
  constexpr منطقی is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr صحیح bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_گنتی_For_بٹ(trailing_significand_بٹ)>
      mysignificand;

  اگر (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.سائز_2(),
                mysignificand.begin());
    اگر (myexponent == 1 &&
        !(significandParts()[integer_ثنائی_ہندسہ_part] & integer_ثنائی_ہندسہ))
      myexponent = 0; // denormal
  } ورنہ اگر (category == fcZero) {
    اگر (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } ورنہ اگر (category == fcInfinity) {
    اگر (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    اگر constexpr (S.has_Explicit_Integer_ثنائی_ہندسہ) {
      mysignificand[0] = integerPart{1} << (trailing_significand_بٹ - 1);
    }
  } ورنہ {
    أكد(category == fcNaN && "Unknown category!");
    اگر (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.سائز_2(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.سائز_اندر_بٹ + 63) / 64> الفاظ;
  خودکار الفاظ_iter =
      std::copy_n(mysignificand.begin(), mysignificand.سائز_2(), الفاظ.begin());
  اگر constexpr (!S.has_Explicit_Integer_ثنائی_ہندسہ) {
    اگر constexpr (significand_نقاب != 0 || trailing_significand_بٹ == 0) {
      // Clear the integer bit.
      الفاظ[mysignificand.سائز_2() - 1] &= significand_نقاب;
    }
  }
  std::fill(الفاظ_iter, الفاظ.انتہا(), uint64_t{0});
  constexpr size_t آخری_لفظ = الفاظ.سائز_2() - 1;
  اگر constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.سائز_اندر_بٹ - 1) % 64);
    الفاظ[آخری_لفظ] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_نقاب)
                              << (trailing_significand_بٹ % 64);
  الفاظ[آخری_لفظ] |= shifted_exponent;
  اگر constexpr (آخری_لفظ == 0) {
    واپس APInt(S.سائز_اندر_بٹ, الفاظ[0]);
  }
  واپس APInt(S.سائز_اندر_بٹ, الفاظ);
}

APInt IEEEFloat::convert_Quadruple_APFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی() == 2);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semIEEEquad>();
}

APInt IEEEFloat::convert_Double_APFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی()==1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semIEEEdouble>();
}

APInt IEEEFloat::convert_Float_APFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی()==1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semIEEEsingle>();
}

APInt IEEEFloat::convert_BFloat_APFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی() == 1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semBFloat>();
}

APInt IEEEFloat::convert_Half_APFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی()==1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::APFloat_بنیاد::semIEEEhalf>();
}

APInt IEEEFloat::convert_Float8_E5_M2_APFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی() == 1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semFloat8E5M2>();
}

APInt IEEEFloat::convert_Float8_E5_M2_FNUZAPFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی() == 1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convert_Float8_E4_M3_APFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی() == 1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semFloat8E4M3>();
}

APInt IEEEFloat::convert_Float8_E4_M3_FNAPFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی() == 1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semFloat8E4M3FN>();
}

APInt IEEEFloat::convert_Float8_E4_M3_FNUZAPFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی() == 1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convert_Float8_E4_M3_B11_FNUZAPFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی() == 1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convert_Float8_E3_M4_APFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی() == 1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semFloat8E3M4>();
}

APInt IEEEFloat::convert_Float_TF32_APFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی() == 1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semFloatTF32>();
}

APInt IEEEFloat::convert_Float8_E8_M0_FNUAPFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی() == 1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convert_Float8_E5_M3_FNUAPFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی() == 1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convert_Float6_E3_M2_FNAPFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی() == 1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semFloat6E3M2FN>();
}

APInt IEEEFloat::convert_Float6_E2_M3_FNAPFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی() == 1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semFloat6E2M3FN>();
}

APInt IEEEFloat::convert_Float4_E2_M1_FNAPFloat_مقصد_APInt() ثابت {
  أكد(part_گنتی() == 1);
  واپس convert_IEEEFloat_مقصد_APInt<APFloat_بنیاد::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcast_مقصد_APInt() ثابت {
  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semIEEEhalf)
    واپس convert_Half_APFloat_مقصد_APInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semBFloat)
    واپس convert_BFloat_APFloat_مقصد_APInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semIEEEsingle)
    واپس convert_Float_APFloat_مقصد_APInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semIEEEdouble)
    واپس convert_Double_APFloat_مقصد_APInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semIEEEquad)
    واپس convert_Quadruple_APFloat_مقصد_APInt();

  اگر (semantics ==
      (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semX87DoubleExtended)
    واپس convert_F80_Long_Double_APFloat_مقصد_APInt();

  اگر (semantics ==
      (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semPPCDoubleDoubleLegacy)
    واپس convert_PPCDouble_Double_Legacy_APFloat_مقصد_APInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semFloat8E5M2)
    واپس convert_Float8_E5_M2_APFloat_مقصد_APInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semFloat8E5M2FNUZ)
    واپس convert_Float8_E5_M2_FNUZAPFloat_مقصد_APInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semFloat8E4M3)
    واپس convert_Float8_E4_M3_APFloat_مقصد_APInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semFloat8E4M3FN)
    واپس convert_Float8_E4_M3_FNAPFloat_مقصد_APInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semFloat8E4M3FNUZ)
    واپس convert_Float8_E4_M3_FNUZAPFloat_مقصد_APInt();

  اگر (semantics ==
      (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semFloat8E4M3B11FNUZ)
    واپس convert_Float8_E4_M3_B11_FNUZAPFloat_مقصد_APInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semFloat8E3M4)
    واپس convert_Float8_E3_M4_APFloat_مقصد_APInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semFloatTF32)
    واپس convert_Float_TF32_APFloat_مقصد_APInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semFloat8E8M0FNU)
    واپس convert_Float8_E8_M0_FNUAPFloat_مقصد_APInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semFloat8E5M3FNU)
    واپس convert_Float8_E5_M3_FNUAPFloat_مقصد_APInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semFloat6E3M2FN)
    واپس convert_Float6_E3_M2_FNAPFloat_مقصد_APInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semFloat6E2M3FN)
    واپس convert_Float6_E2_M3_FNAPFloat_مقصد_APInt();

  اگر (semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semFloat4E2M1FN)
    واپس convert_Float4_E2_M1_FNAPFloat_مقصد_APInt();

  llvm_unreachable("unknown format!");
}

اعشاری IEEEFloat::convert_مقصد_Float() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcast_مقصد_APInt();
  واپس api.بٹ_مقصد_Float();
}

دوہرا IEEEFloat::convert_مقصد_Double() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcast_مقصد_APInt();
  واپس api.بٹ_مقصد_Double();
}

#اگر_تعریف_شدہ HAS_IEE754_FLOAT128
float128 IEEEFloat::convert_مقصد_Quad() ثابت {
  أكد(semantics == (ثابت llvm::fltSemantics *)&APFloat_بنیاد::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcast_مقصد_APInt();
  واپس api.بٹ_مقصد_Quad();
}
#اگر_ختم

خالی IEEEFloat::initFromF80LongDoubleAPInt(ثابت APInt &api) {
  واپس initFromIEEEAPInt<APFloat_بنیاد::semX87DoubleExtended>(api);
}

خالی IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(ثابت APInt &api) {
  uint64_t i1 = api.get_Raw_ڈیٹا()[0];
  uint64_t i2 = api.get_Raw_ڈیٹا()[1];
  منطقی losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_غیر_مستعمل]] op_حالت fs = convert(APFloat_بنیاد::semPPCDoubleDoubleLegacy,
                                         rm_قریب_ترین_Ties_مقصد_Even, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  أكد((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  اگر (isFiniteNonZero()) {
    IEEEFloat v(APFloat_بنیاد::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    اگر (v.isFinite()) {
      fs = v.convert(APFloat_بنیاد::semPPCDoubleDoubleLegacy, rm_قریب_ترین_Ties_مقصد_Even,
                     &losesInfo);
      أكد(fs == opOK && !losesInfo);

      جوڑو(v, rm_قریب_ترین_Ties_مقصد_Even);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
خالی IEEEFloat::initFromFloat8E8M0FNUAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semFloat8E8M0FNU>(api);
}

خالی IEEEFloat::initFromFloat8E5M3FNUAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semFloat8E5M3FNU>(api);
}

سانچہ <ثابت fltSemantics &S>
خالی IEEEFloat::initFromIEEEAPInt(ثابت APInt &api) {
  أكد(api.get_ثنائی_ہندسہ_چوڑائی() == S.سائز_اندر_بٹ);

  constexpr غیرعلامتی صحیح trailing_significand_بٹ =
      S.precision - 1 + S.has_Explicit_Integer_ثنائی_ہندسہ;
  constexpr integerPart integer_ثنائی_ہندسہ =
      integerPart{1} << (trailing_significand_بٹ % integer_Part_چوڑائی);
  constexpr uint64_t significand_نقاب = integer_ثنائی_ہندسہ - 1;
  constexpr غیرعلامتی صحیح exponent_بٹ =
      S.سائز_اندر_بٹ - (S.hasSignedRepr ? 1 : 0) - trailing_significand_بٹ;
  static_assert(exponent_بٹ < 64);
  constexpr غیرعلامتی صحیح stored_significand_parts =
      part_گنتی_For_بٹ(trailing_significand_بٹ + 1);
  constexpr uint64_t exponent_نقاب = (uint64_t{1} << exponent_بٹ) - 1;
  constexpr منطقی is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr صحیح bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr منطقی has_significand = trailing_significand_بٹ > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  اگر constexpr (has_significand) {
    std::copy_n(api.get_Raw_ڈیٹا(), mysignificand.سائز_2(), mysignificand.begin());
    اگر constexpr (significand_نقاب != 0 || S.precision >= integer_Part_چوڑائی) {
      mysignificand[mysignificand.سائز_2() - 1] &= significand_نقاب;
    }
  } ورنہ {
    std::fill_n(mysignificand.begin(), mysignificand.سائز_2(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t آخری_لفظ = api.get_Raw_ڈیٹا()[api.get_عدد_الفاظ() - 1];
  uint64_t myexponent =
      (آخری_لفظ >> (trailing_significand_بٹ % 64)) & exponent_نقاب;

  initialize(&S);
  أكد(part_گنتی() == mysignificand.سائز_2());

  sign = S.hasSignedRepr
             ? static_cast<غیرعلامتی صحیح>(آخری_لفظ >> ((S.سائز_اندر_بٹ - 1) % 64))
             : 0;

  منطقی all_zero_significand =
      has_significand && llvm::all_کا(mysignificand, equal_مقصد(0));

  منطقی is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  اگر constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    منطقی is_inf = جھوٹ;

    اگر constexpr (S.has_Explicit_Integer_ثنائی_ہندسہ) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_نقاب_no_int_ثنائی_ہندسہ =
          (uint64_t{1} << (trailing_significand_بٹ - 1)) - 1;
      ثابت integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_بٹ - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_نقاب_no_int_ثنائی_ہندسہ) == 0;
    } ورنہ {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    اگر (is_inf) {
      makeInf(sign);
      واپس;
    }
  }

  منطقی is_nan = جھوٹ;

  اگر constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    اگر constexpr (S.has_Explicit_Integer_ثنائی_ہندسہ) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      ثابت integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_بٹ - 1);
      constexpr integerPart significand_نقاب_no_int_ثنائی_ہندسہ =
          (uint64_t{1} << (trailing_significand_بٹ - 1)) - 1;

      اگر (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_نقاب_no_int_ثنائی_ہندسہ) != 0) {
        // regular NaN and pseudoNaN
        is_nan = سچ;
      } ورنہ اگر (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_نقاب_no_int_ثنائی_ہندسہ) == 0) {
        // pseudoinfinity
        is_nan = سچ;
      } ورنہ اگر (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = سچ;
      }
    } ورنہ {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } ورنہ اگر constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    منطقی all_ones_significand =
        std::all_کا(mysignificand.begin(), mysignificand.انتہا() - 1,
                    [](integerPart بٹ) { واپس بٹ == ~integerPart{0}; }) &&
        (!significand_نقاب ||
         mysignificand[mysignificand.سائز_2() - 1] == significand_نقاب);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } ورنہ اگر constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  اگر (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.سائز_2(),
                significandParts());
    واپس;
  }

  اگر (is_zero) {
    makeZero(sign);
    واپس;
  }

  category = fc_معمول;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.سائز_2(), significandParts());
  اگر (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  ورنہ {
    اگر constexpr (!S.has_Explicit_Integer_ثنائی_ہندسہ) {
      significandParts()[mysignificand.سائز_2() - 1] |= integer_ثنائی_ہندسہ;
    }
  }
}

خالی IEEEFloat::initFromQuadrupleAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semIEEEquad>(api);
}

خالی IEEEFloat::initFromDoubleAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semIEEEdouble>(api);
}

خالی IEEEFloat::initFromFloatAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semIEEEsingle>(api);
}

خالی IEEEFloat::initFromBFloatAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semBFloat>(api);
}

خالی IEEEFloat::initFromHalfAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semIEEEhalf>(api);
}

خالی IEEEFloat::initFromFloat8E5M2APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semFloat8E5M2>(api);
}

خالی IEEEFloat::initFromFloat8E5M2FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semFloat8E5M2FNUZ>(api);
}

خالی IEEEFloat::initFromFloat8E4M3APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semFloat8E4M3>(api);
}

خالی IEEEFloat::initFromFloat8E4M3FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semFloat8E4M3FN>(api);
}

خالی IEEEFloat::initFromFloat8E4M3FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semFloat8E4M3FNUZ>(api);
}

خالی IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semFloat8E4M3B11FNUZ>(api);
}

خالی IEEEFloat::initFromFloat8E3M4APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semFloat8E3M4>(api);
}

خالی IEEEFloat::initFromFloatTF32APInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semFloatTF32>(api);
}

خالی IEEEFloat::initFromFloat6E3M2FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semFloat6E3M2FN>(api);
}

خالی IEEEFloat::initFromFloat6E2M3FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semFloat6E2M3FN>(api);
}

خالی IEEEFloat::initFromFloat4E2M1FNAPInt(ثابت APInt &api) {
  initFromIEEEAPInt<APFloat_بنیاد::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
خالی IEEEFloat::initFromAPInt(ثابت fltSemantics *Sem, ثابت APInt &api) {
  أكد(api.get_ثنائی_ہندسہ_چوڑائی() == Sem->سائز_اندر_بٹ);
  اگر (Sem == &APFloat_بنیاد::semIEEEhalf)
    واپس initFromHalfAPInt(api);
  اگر (Sem == &APFloat_بنیاد::semBFloat)
    واپس initFromBFloatAPInt(api);
  اگر (Sem == &APFloat_بنیاد::semIEEEsingle)
    واپس initFromFloatAPInt(api);
  اگر (Sem == &APFloat_بنیاد::semIEEEdouble)
    واپس initFromDoubleAPInt(api);
  اگر (Sem == &APFloat_بنیاد::semIEEEquad)
    واپس initFromQuadrupleAPInt(api);
  اگر (Sem == &APFloat_بنیاد::semX87DoubleExtended)
    واپس initFromF80LongDoubleAPInt(api);
  اگر (Sem == &APFloat_بنیاد::semPPCDoubleDoubleLegacy)
    واپس initFromPPCDoubleDoubleLegacyAPInt(api);
  اگر (Sem == &APFloat_بنیاد::semFloat8E5M2)
    واپس initFromFloat8E5M2APInt(api);
  اگر (Sem == &APFloat_بنیاد::semFloat8E5M2FNUZ)
    واپس initFromFloat8E5M2FNUZAPInt(api);
  اگر (Sem == &APFloat_بنیاد::semFloat8E4M3)
    واپس initFromFloat8E4M3APInt(api);
  اگر (Sem == &APFloat_بنیاد::semFloat8E4M3FN)
    واپس initFromFloat8E4M3FNAPInt(api);
  اگر (Sem == &APFloat_بنیاد::semFloat8E4M3FNUZ)
    واپس initFromFloat8E4M3FNUZAPInt(api);
  اگر (Sem == &APFloat_بنیاد::semFloat8E4M3B11FNUZ)
    واپس initFromFloat8E4M3B11FNUZAPInt(api);
  اگر (Sem == &APFloat_بنیاد::semFloat8E3M4)
    واپس initFromFloat8E3M4APInt(api);
  اگر (Sem == &APFloat_بنیاد::semFloatTF32)
    واپس initFromFloatTF32APInt(api);
  اگر (Sem == &APFloat_بنیاد::semFloat8E8M0FNU)
    واپس initFromFloat8E8M0FNUAPInt(api);
  اگر (Sem == &APFloat_بنیاد::semFloat8E5M3FNU)
    واپس initFromFloat8E5M3FNUAPInt(api);
  اگر (Sem == &APFloat_بنیاد::semFloat6E3M2FN)
    واپس initFromFloat6E3M2FNAPInt(api);
  اگر (Sem == &APFloat_بنیاد::semFloat6E2M3FN)
    واپس initFromFloat6E2M3FNAPInt(api);
  اگر (Sem == &APFloat_بنیاد::semFloat4E2M1FN)
    واپس initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
خالی IEEEFloat::make_سب_سے_بڑا(منطقی Negative) {
  اگر (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 1..10
  //   significand = 1..1
  category = fc_معمول;
  sign = Negative;
  exponent = semantics->maxExponent;

  // Use memset to set all but the highest integerPart to all ones.
  integerPart *significand = significandParts();
  غیرعلامتی Part_گنتی = part_گنتی();
  memset(significand, 0xFF, حجم(integerPart)*(Part_گنتی - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  ثابت غیرعلامتی عدد_غیر_مستعمل_بلند_بٹ =
    Part_گنتی*integer_Part_چوڑائی - semantics->precision;
  significand[Part_گنتی - 1] = (عدد_غیر_مستعمل_بلند_بٹ < integer_Part_چوڑائی)
                                   ? (~integerPart(0) >> عدد_غیر_مستعمل_بلند_بٹ)
                                   : 0;
  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
خالی IEEEFloat::make_سب_سے_چھوٹا(منطقی Negative) {
  اگر (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fc_معمول;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tc_سیٹ(significandParts(), 1, part_گنتی());
}

خالی IEEEFloat::make_سب_سے_چھوٹا_Normalized(منطقی Negative) {
  اگر (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 10..0

  category = fc_معمول;
  zeroSignificand();
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tc_سیٹ_ثنائی_ہندسہ(significandParts(), semantics->precision - 1);
}

IEEEFloat::IEEEFloat(ثابت fltSemantics &Sem, ثابت APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(اعشاری f) {
  initFromAPInt(&APFloat_بنیاد::semIEEEsingle, APInt::float_مقصد_بٹ(f));
}

IEEEFloat::IEEEFloat(دوہرا d) {
  initFromAPInt(&APFloat_بنیاد::semIEEEdouble, APInt::double_مقصد_بٹ(d));
}

نامخلا {
  خالی append(Small_سمتیہ_Impl<حرف> &بفر, StringRef Str) {
    بفر.append(Str.begin(), Str.انتہا());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  خالی Adjust_مقصد_Precision(APInt &significand,
                         صحیح &قوت_نما, غیرعلامتی FormatPrecision) {
    غیرعلامتی بٹ = significand.get_فعال_بٹ();

    // 196/59 is a very slight overestimate of lg_2(10).
    غیرعلامتی بٹ_Required = (FormatPrecision * 196 + 58) / 59;

    اگر (بٹ <= بٹ_Required) واپس;

    غیرعلامتی tensRemovable = (بٹ - بٹ_Required) * 59 / 196;
    اگر (!tensRemovable) واپس;

    قوت_نما += tensRemovable;

    APInt مقسوم_علیہ(significand.get_ثنائی_ہندسہ_چوڑائی(), 1);
    APInt powten(significand.get_ثنائی_ہندسہ_چوڑائی(), 10);
    جب (سچ) {
      اگر (tensRemovable & 1)
        مقسوم_علیہ *= powten;
      tensRemovable >>= 1;
      اگر (!tensRemovable) توڑو;
      powten *= powten;
    }

    significand = significand.udiv(مقسوم_علیہ);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.get_فعال_بٹ());
  }


  خالی Adjust_مقصد_Precision(Small_سمتیہ_Impl<حرف> &بفر_2,
                         صحیح &قوت_نما, غیرعلامتی FormatPrecision) {
    غیرعلامتی N = بفر_2.سائز_2();
    اگر (N <= FormatPrecision) واپس;

    // The most significant figures are the last ones in the buffer.
    غیرعلامتی پہلا_Significant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    اگر (بفر_2[پہلا_Significant - 1] < '5') {
      جب (پہلا_Significant < N && بفر_2[پہلا_Significant] == '0')
        پہلا_Significant++;

      قوت_نما += پہلا_Significant;
      بفر_2.erase(&بفر_2[0], &بفر_2[پہلا_Significant]);
      واپس;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    برائے (غیرعلامتی I = پہلا_Significant; I != N; ++I) {
      اگر (بفر_2[I] == '9') {
        پہلا_Significant++;
      } ورنہ {
        بفر_2[I]++;
        توڑو;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    اگر (پہلا_Significant == N) {
      قوت_نما += پہلا_Significant;
      بفر_2.clear();
      بفر_2.ڈالو_عقب('1');
      واپس;
    }

    قوت_نما += پہلا_Significant;
    بفر_2.erase(&بفر_2[0], &بفر_2[پہلا_Significant]);
  }

  خالی مقصد_String_Impl(Small_سمتیہ_Impl<حرف> &Str, ثابت منطقی isNeg, صحیح قوت_نما,
                    APInt significand, غیرعلامتی FormatPrecision,
                    غیرعلامتی FormatMaxPadding, منطقی TruncateZero) {
    ثابت صحیح semanticsPrecision = significand.get_ثنائی_ہندسہ_چوڑائی();

    اگر (isNeg)
      Str.ڈالو_عقب('-');

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
    قوت_نما += trailingZeros;
    significand.lshr_اندر_رکھو(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    اگر (قوت_نما == 0) {
      // Nothing to do.
    } ورنہ اگر (قوت_نما > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + قوت_نما);
      significand <<= قوت_نما;
      قوت_نما = 0;
    } ورنہ { /* exp < 0 */
      صحیح texp = -قوت_نما;

      // We transform this using the identity:
      //   (N)(2^-e) == (N)(5^e)(10^-e)
      // This means we have to multiply N (the significand) by 5^e.
      // To avoid overflow, we have to operate on numbers large
      // enough to store N * 5^e:
      //   log2(N * 5^e) == log2(N) + e * log2(5)
      //                 <= semantics->precision + e * 137 / 59
      //   (log_2(5) ~ 2.321928 < 2.322034 ~ 137/59)

      غیرعلامتی precision = semanticsPrecision + (137 * texp + 136) / 59;

      // Multiply significand by 5^e.
      //   N * 5^0101 == N * 5^(1*1) * 5^(0*2) * 5^(1*4) * 5^(0*8)
      significand = significand.zext(precision);
      APInt five_مقصد_the_i(precision, 5);
      جب (سچ) {
        اگر (texp & 1)
          significand *= five_مقصد_the_i;

        texp >>= 1;
        اگر (!texp)
          توڑو;
        five_مقصد_the_i *= five_مقصد_the_i;
      }
    }

    Adjust_مقصد_Precision(significand, قوت_نما, FormatPrecision);

    Small_سمتیہ<حرف, 256> بفر_2;

    // Fill the buffer.
    غیرعلامتی precision = significand.get_ثنائی_ہندسہ_چوڑائی();
    اگر (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    منطقی اندر_Trail = سچ;
    جب (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      غیرعلامتی d = digit.get_ZExt_قدر();

      // Drop trailing zeros.
      اگر (اندر_Trail && !d)
        قوت_نما++;
      ورنہ {
        بفر_2.ڈالو_عقب((حرف) ('0' + d));
        اندر_Trail = جھوٹ;
      }
    }

    أكد(!بفر_2.خالی_خانے() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    Adjust_مقصد_Precision(بفر_2, قوت_نما, FormatPrecision);

    غیرعلامتی NDigits = بفر_2.سائز_2();

    // Check whether we should use scientific notation.
    منطقی FormatScientific;
    اگر (!FormatMaxPadding) {
      FormatScientific = سچ;
    } ورنہ {
      اگر (قوت_نما >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((غیرعلامتی) قوت_نما > FormatMaxPadding ||
                            NDigits + (غیرعلامتی) قوت_نما > FormatPrecision);
      } ورنہ {
        // Power of the most significant digit.
        صحیح MSD = قوت_نما + (صحیح) (NDigits - 1);
        اگر (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = جھوٹ;
        } ورنہ {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((غیرعلامتی) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    اگر (FormatScientific) {
      قوت_نما += (NDigits - 1);

      Str.ڈالو_عقب(بفر_2[NDigits-1]);
      Str.ڈالو_عقب('.');
      اگر (NDigits == 1 && TruncateZero)
        Str.ڈالو_عقب('0');
      ورنہ
        برائے (غیرعلامتی I = 1; I != NDigits; ++I)
          Str.ڈالو_عقب(بفر_2[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      اگر (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.ڈالو_عقب(TruncateZero ? 'E' : 'e');

      Str.ڈالو_عقب(قوت_نما >= 0 ? '+' : '-');
      اگر (قوت_نما < 0)
        قوت_نما = -قوت_نما;
      Small_سمتیہ<حرف, 6> expbuf;
      کرو {
        expbuf.ڈالو_عقب((حرف) ('0' + (قوت_نما % 10)));
        قوت_نما /= 10;
      } جب (قوت_نما);
      // Exponent always at least two digits if we do not truncate zeros.
      اگر (!TruncateZero && expbuf.سائز_2() < 2)
        expbuf.ڈالو_عقب('0');
      برائے (غیرعلامتی I = 0, E = expbuf.سائز_2(); I != E; ++I)
        Str.ڈالو_عقب(expbuf[E-1-I]);
      واپس;
    }

    // Non-scientific, positive exponents.
    اگر (قوت_نما >= 0) {
      برائے (غیرعلامتی I = 0; I != NDigits; ++I)
        Str.ڈالو_عقب(بفر_2[NDigits-1-I]);
      برائے (غیرعلامتی I = 0; I != (غیرعلامتی) قوت_نما; ++I)
        Str.ڈالو_عقب('0');
      واپس;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    صحیح NWholeDigits = قوت_نما + (صحیح) NDigits;

    غیرعلامتی I = 0;
    اگر (NWholeDigits > 0) {
      برائے (; I != (غیرعلامتی) NWholeDigits; ++I)
        Str.ڈالو_عقب(بفر_2[NDigits-I-1]);
      Str.ڈالو_عقب('.');
    } ورنہ {
      غیرعلامتی NZeros = 1 + (غیرعلامتی) -NWholeDigits;

      Str.ڈالو_عقب('0');
      Str.ڈالو_عقب('.');
      برائے (غیرعلامتی Z = 1; Z != NZeros; ++Z)
        Str.ڈالو_عقب('0');
    }

    برائے (; I != NDigits; ++I)
      Str.ڈالو_عقب(بفر_2[NDigits-I-1]);

  }
} // namespace

خالی IEEEFloat::مقصد_String(Small_سمتیہ_Impl<حرف> &Str, غیرعلامتی FormatPrecision,
                         غیرعلامتی FormatMaxPadding, منطقی TruncateZero) ثابت {
  انتخاب (category) {
  حالت fcInfinity:
    اگر (isNegative())
      واپس append(Str, "-Inf");
    ورنہ
      واپس append(Str, "+Inf");

  حالت fcNaN: واپس append(Str, "NaN");

  حالت fcZero:
    اگر (isNegative())
      Str.ڈالو_عقب('-');

    اگر (!FormatMaxPadding) {
      اگر (TruncateZero)
        append(Str, "0.0E+0");
      ورنہ {
        append(Str, "0.0");
        اگر (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } ورنہ {
      Str.ڈالو_عقب('0');
    }
    واپس;

  حالت fc_معمول:
    توڑو;
  }

  // Decompose the number into an APInt and an exponent.
  صحیح قوت_نما = exponent - ((صحیح) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_گنتی_For_بٹ(semantics->precision)));

  مقصد_String_Impl(Str, isNegative(), قوت_نما, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

صحیح IEEEFloat::getExactLog2Abs() ثابت {
  اگر (!isFinite() || isZero())
    واپس INT_MIN;

  ثابت integerPart *Parts = significandParts();
  ثابت صحیح Part_گنتی = part_گنتی_For_بٹ(semantics->precision);

  صحیح نکالو_گنتی = 0;
  برائے (صحیح i = 0; i < Part_گنتی; ++i) {
    نکالو_گنتی += llvm::popcount(Parts[i]);
    اگر (نکالو_گنتی > 1)
      واپس INT_MIN;
  }

  اگر (exponent != semantics->minExponent)
    واپس exponent;

  صحیح CountrParts = 0;
  برائے (صحیح i = 0; i < Part_گنتی;
       ++i, CountrParts += APInt::APINT_بٹ_فی_لفظ) {
    اگر (Parts[i] != 0) {
      واپس exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

منطقی IEEEFloat::isSignaling() ثابت {
  اگر (!isNaN())
    واپس جھوٹ;
  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    واپس جھوٹ;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  واپس !APInt::tc_نکالو_ثنائی_ہندسہ(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::op_حالت IEEEFloat::اگلا(منطقی اگلا_نیچے_کو) {
  // If we are performing nextDown, swap sign so we have -x.
  اگر (اگلا_نیچے_کو)
    changeSign();

  // Compute nextUp(x)
  op_حالت نتیجہ_2 = opOK;

  // Handle each float category separately.
  انتخاب (category) {
  حالت fcInfinity:
    // nextUp(+inf) = +inf
    اگر (!isNegative())
      توڑو;
    // nextUp(-inf) = -getLargest()
    make_سب_سے_بڑا(سچ);
    توڑو;
  حالت fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    اگر (isSignaling()) {
      نتیجہ_2 = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(جھوٹ, isNegative(), خالی_اشاریہ);
    }
    توڑو;
  حالت fcZero:
    // nextUp(pm 0) = +getSmallest()
    make_سب_سے_چھوٹا(جھوٹ);
    توڑو;
  حالت fc_معمول:
    // nextUp(-getSmallest()) = -0
    اگر (is_سب_سے_چھوٹا() && isNegative()) {
      APInt::tc_سیٹ(significandParts(), 0, part_گنتی());
      category = fcZero;
      exponent = 0;
      اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = جھوٹ;
      اگر (!semantics->hasZero)
        make_سب_سے_چھوٹا_Normalized(جھوٹ);
      توڑو;
    }

    اگر (is_سب_سے_بڑا() && !isNegative()) {
      اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        توڑو;
      } ورنہ اگر (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        توڑو;
      } ورنہ {
        // nextUp(getLargest()) == INFINITY
        APInt::tc_سیٹ(significandParts(), 0, part_گنتی());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        توڑو;
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
      APInt::tcDecrement(Parts, part_گنتی());

      اگر (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tc_سیٹ_ثنائی_ہندسہ(Parts, semantics->precision - 1);
        exponent--;
      }
    } ورنہ {
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
        APInt::tc_سیٹ(Parts, 0, part_گنتی());
        APInt::tc_سیٹ_ثنائی_ہندسہ(Parts, semantics->precision - 1);
        أكد(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } ورنہ {
        incrementSignificand();
      }
    }
    توڑو;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  اگر (اگلا_نیچے_کو)
    changeSign();

  واپس نتیجہ_2;
}

APInt IEEEFloat::getNaNPayload() ثابت {
  أكد(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  غیرعلامتی بٹ_2 = semantics->precision - 1;
  واپس APInt(بٹ_2, ArrayRef(significandParts(), part_گنتی_For_بٹ(بٹ_2)));
}

APFloat_بنیاد::ExponentType IEEEFloat::exponentNaN() ثابت {
  واپس ::exponentNaN(*semantics);
}

APFloat_بنیاد::ExponentType IEEEFloat::exponentInf() ثابت {
  واپس ::exponentInf(*semantics);
}

APFloat_بنیاد::ExponentType IEEEFloat::exponentZero() ثابت {
  واپس ::exponentZero(*semantics);
}

خالی IEEEFloat::makeInf(منطقی Negative) {
  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  اگر (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(جھوٹ, Negative);
    واپس;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tc_سیٹ(significandParts(), 0, part_گنتی());
}

خالی IEEEFloat::makeZero(منطقی Negative) {
  اگر (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  اگر (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = جھوٹ;
  }
  exponent = exponentZero();
  APInt::tc_سیٹ(significandParts(), 0, part_گنتی());
}

خالی IEEEFloat::makeQuiet() {
  أكد(isNaN());
  اگر (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tc_سیٹ_ثنائی_ہندسہ(significandParts(), semantics->precision - 2);
}

صحیح ilogb(ثابت IEEEFloat &Arg) {
  اگر (Arg.isNaN())
    واپس APFloat::IEK_NaN;
  اگر (Arg.isZero())
    واپس APFloat::IEK_Zero;
  اگر (Arg.isInfinity())
    واپس APFloat::IEK_Inf;
  اگر (!Arg.isDenormal())
    واپس Arg.exponent;

  IEEEFloat Normalized(Arg);
  صحیح Significand_بٹ = Arg.getSemantics().precision - 1;

  Normalized.exponent += Significand_بٹ;
  Normalized.normalize(APFloat::rm_قریب_ترین_Ties_مقصد_Even, lfExactlyZero);
  واپس Normalized.exponent - Significand_بٹ;
}

IEEEFloat scalbn(IEEEFloat X, صحیح Exp, rounding_منوال Rounding_منوال) {
  خودکار MaxExp = X.getSemantics().maxExponent;
  خودکار MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  صحیح Significand_بٹ = X.getSemantics().precision - 1;
  صحیح MaxIncrement = MaxExp - (MinExp - Significand_بٹ) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(Rounding_منوال, lfExactlyZero);
  اگر (X.isNaN())
    X.makeQuiet();
  واپس X;
}

IEEEFloat frexp(ثابت IEEEFloat &Val, صحیح &Exp, rounding_منوال RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  اگر (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    واپس Quiet;
  }

  اگر (Exp == APFloat::IEK_Inf)
    واپس Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  واپس scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S)
    : Semantics(&S),
      Floats(نیا APFloat[2]{APFloat(APFloat_بنیاد::semIEEEdouble),
                            APFloat(APFloat_بنیاد::semIEEEdouble)}) {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, uninitialized_لیبل)
    : Semantics(&S), Floats(نیا APFloat[2]{
                         APFloat(APFloat_بنیاد::semIEEEdouble, uninitialized),
                         APFloat(APFloat_بنیاد::semIEEEdouble, uninitialized)}) {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(نیا APFloat[2]{APFloat(APFloat_بنیاد::semIEEEdouble, I),
                            APFloat(APFloat_بنیاد::semIEEEdouble)}) {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, ثابت APInt &I)
    : Semantics(&S),
      Floats(نیا APFloat[2]{
          APFloat(APFloat_بنیاد::semIEEEdouble, APInt(64, I.get_Raw_ڈیٹا()[0])),
          APFloat(APFloat_بنیاد::semIEEEdouble, APInt(64, I.get_Raw_ڈیٹا()[1]))}) {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت fltSemantics &S, APFloat &&پہلا,
                             APFloat &&دوسرا)
    : Semantics(&S),
      Floats(نیا APFloat[2]{std::move(پہلا), std::move(دوسرا)}) {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble);
  أكد(&Floats[0].getSemantics() == &APFloat_بنیاد::semIEEEdouble);
  أكد(&Floats[1].getSemantics() == &APFloat_بنیاد::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(ثابت DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? نیا APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : خالی_اشاریہ) {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloat_بنیاد::semBogus;
  RHS.Floats = خالی_اشاریہ;
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::عامل=(ثابت DoubleAPFloat &RHS) {
  اگر (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } ورنہ اگر (یہ != &RHS) {
    یہ->~DoubleAPFloat();
    نیا (یہ) DoubleAPFloat(RHS);
  }
  واپس *یہ;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
ساکن std::pair<APFloat, APFloat> fast_Two_مجموع(APFloat X, APFloat Y) {
  اگر (!X.isFinite())
    واپس {X, APFloat::getZero(X.getSemantics(), /*Negative=*/جھوٹ)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  واپس {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::op_حالت DoubleAPFloat::جوڑو_Impl(ثابت APFloat &a, ثابت APFloat &aa,
                                         ثابت APFloat &c, ثابت APFloat &cc,
                                         rounding_منوال RM) {
  صحیح حالت_2 = opOK;
  APFloat z = a;
  حالت_2 |= z.جوڑو(c, RM);
  اگر (!z.isFinite()) {
    اگر (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ جھوٹ);
      واپس (op_حالت)حالت_2;
    }
    حالت_2 = opOK;
    خودکار ACompared_مقصد_C = a.compare_Absolute_قدر(c);
    z = cc;
    حالت_2 |= z.جوڑو(aa, RM);
    اگر (ACompared_مقصد_C == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      حالت_2 |= z.جوڑو(c, RM);
      حالت_2 |= z.جوڑو(a, RM);
    } ورنہ {
      // z = cc + aa + a + c;
      حالت_2 |= z.جوڑو(a, RM);
      حالت_2 |= z.جوڑو(c, RM);
    }
    اگر (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ جھوٹ);
      واپس (op_حالت)حالت_2;
    }
    Floats[0] = z;
    APFloat zz = aa;
    حالت_2 |= zz.جوڑو(cc, RM);
    اگر (ACompared_مقصد_C == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      حالت_2 |= Floats[1].تفریق(z, RM);
      حالت_2 |= Floats[1].جوڑو(c, RM);
      حالت_2 |= Floats[1].جوڑو(zz, RM);
    } ورنہ {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      حالت_2 |= Floats[1].تفریق(z, RM);
      حالت_2 |= Floats[1].جوڑو(a, RM);
      حالت_2 |= Floats[1].جوڑو(zz, RM);
    }
  } ورنہ {
    // q = a - z;
    APFloat q = a;
    حالت_2 |= q.تفریق(z, RM);

    // zz = q + c + (a - (q + z)) + aa + cc;
    // Compute a - (q + z) as -((q + z) - a) to avoid temporary copies.
    خودکار zz = q;
    حالت_2 |= zz.جوڑو(c, RM);
    حالت_2 |= q.جوڑو(z, RM);
    حالت_2 |= q.تفریق(a, RM);
    q.changeSign();
    حالت_2 |= zz.جوڑو(q, RM);
    حالت_2 |= zz.جوڑو(aa, RM);
    حالت_2 |= zz.جوڑو(cc, RM);
    اگر (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ جھوٹ);
      واپس opOK;
    }
    Floats[0] = z;
    حالت_2 |= Floats[0].جوڑو(zz, RM);
    اگر (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ جھوٹ);
      واپس (op_حالت)حالت_2;
    }
    Floats[1] = std::move(z);
    حالت_2 |= Floats[1].تفریق(Floats[0], RM);
    حالت_2 |= Floats[1].جوڑو(zz, RM);
  }
  واپس (op_حالت)حالت_2;
}

APFloat::op_حالت DoubleAPFloat::جوڑو_With_Special(ثابت DoubleAPFloat &LHS,
                                                ثابت DoubleAPFloat &RHS,
                                                DoubleAPFloat &برآمد,
                                                rounding_منوال RM) {
  اگر (LHS.getCategory() == fcNaN) {
    برآمد = LHS;
    واپس opOK;
  }
  اگر (RHS.getCategory() == fcNaN) {
    برآمد = RHS;
    واپس opOK;
  }
  اگر (LHS.getCategory() == fcZero) {
    برآمد = RHS;
    واپس opOK;
  }
  اگر (RHS.getCategory() == fcZero) {
    برآمد = LHS;
    واپس opOK;
  }
  اگر (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    برآمد.makeNaN(جھوٹ, برآمد.isNegative(), خالی_اشاریہ);
    واپس opInvalidOp;
  }
  اگر (LHS.getCategory() == fcInfinity) {
    برآمد = LHS;
    واپس opOK;
  }
  اگر (RHS.getCategory() == fcInfinity) {
    برآمد = RHS;
    واپس opOK;
  }
  أكد(LHS.getCategory() == fc_معمول && RHS.getCategory() == fc_معمول);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  أكد(&A.getSemantics() == &APFloat_بنیاد::semIEEEdouble);
  أكد(&AA.getSemantics() == &APFloat_بنیاد::semIEEEdouble);
  أكد(&C.getSemantics() == &APFloat_بنیاد::semIEEEdouble);
  أكد(&CC.getSemantics() == &APFloat_بنیاد::semIEEEdouble);
  أكد(&برآمد.Floats[0].getSemantics() == &APFloat_بنیاد::semIEEEdouble);
  أكد(&برآمد.Floats[1].getSemantics() == &APFloat_بنیاد::semIEEEdouble);
  واپس برآمد.جوڑو_Impl(A, AA, C, CC, RM);
}

APFloat::op_حالت DoubleAPFloat::جوڑو(ثابت DoubleAPFloat &RHS,
                                     rounding_منوال RM) {
  واپس جوڑو_With_Special(*یہ, RHS, *یہ, RM);
}

APFloat::op_حالت DoubleAPFloat::تفریق(ثابت DoubleAPFloat &RHS,
                                          rounding_منوال RM) {
  changeSign();
  خودکار Ret = جوڑو(RHS, RM);
  changeSign();
  واپس Ret;
}

APFloat::op_حالت DoubleAPFloat::multiply(ثابت DoubleAPFloat &RHS,
                                          APFloat::rounding_منوال RM) {
  ثابت خودکار &LHS = *یہ;
  خودکار &برآمد = *یہ;
  /* Interesting observation: For special categories, finding the lowest
     common ancestor of the following layered graph gives the correct
     return category:

        NaN
       /   \
     Zero  Inf
       \   /
معمول

     e.g. NaN * NaN = NaN
          Zero * Inf = NaN
          Normal * Zero = Zero
          Normal * Inf = Inf
  */
  اگر (LHS.getCategory() == fcNaN) {
    برآمد = LHS;
    واپس opOK;
  }
  اگر (RHS.getCategory() == fcNaN) {
    برآمد = RHS;
    واپس opOK;
  }
  اگر ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    برآمد.makeNaN(جھوٹ, جھوٹ, خالی_اشاریہ);
    واپس opOK;
  }
  اگر (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    برآمد = LHS;
    واپس opOK;
  }
  اگر (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    برآمد = RHS;
    واپس opOK;
  }
  أكد(LHS.getCategory() == fc_معمول && RHS.getCategory() == fc_معمول &&
         "Special cases not handled exhaustively");

  صحیح حالت_2 = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  حالت_2 |= T.multiply(C, RM);
  اگر (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ جھوٹ);
    واپس (op_حالت)حالت_2;
  }

  // tau = fmsub(a, c, t), that is -fmadd(-a, c, t).
  APFloat Tau = A;
  T.changeSign();
  حالت_2 |= Tau.fused_Multiply_جوڑو(C, T, RM);
  T.changeSign();
  {
    // v = a * d
    APFloat V = A;
    حالت_2 |= V.multiply(D, RM);
    // w = b * c
    APFloat W = B;
    حالت_2 |= W.multiply(C, RM);
    حالت_2 |= V.جوڑو(W, RM);
    // tau += v + w
    حالت_2 |= Tau.جوڑو(V, RM);
  }
  // u = t + tau
  APFloat U = T;
  حالت_2 |= U.جوڑو(Tau, RM);

  Floats[0] = U;
  اگر (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ جھوٹ);
  } ورنہ {
    // Floats[1] = (t - u) + tau
    حالت_2 |= T.تفریق(U, RM);
    حالت_2 |= T.جوڑو(Tau, RM);
    Floats[1] = std::move(T);
  }
  واپس (op_حالت)حالت_2;
}

APFloat::op_حالت DoubleAPFloat::divide(ثابت DoubleAPFloat &RHS,
                                        APFloat::rounding_منوال RM) {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_بنیاد::semPPCDoubleDoubleLegacy, bitcast_مقصد_APInt());
  خودکار Ret = Tmp.divide(
      APFloat(APFloat_بنیاد::semPPCDoubleDoubleLegacy, RHS.bitcast_مقصد_APInt()), RM);
  *یہ = DoubleAPFloat(APFloat_بنیاد::semPPCDoubleDouble, Tmp.bitcast_مقصد_APInt());
  واپس Ret;
}

APFloat::op_حالت DoubleAPFloat::remainder(ثابت DoubleAPFloat &RHS) {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_بنیاد::semPPCDoubleDoubleLegacy, bitcast_مقصد_APInt());
  خودکار Ret = Tmp.remainder(
      APFloat(APFloat_بنیاد::semPPCDoubleDoubleLegacy, RHS.bitcast_مقصد_APInt()));
  *یہ = DoubleAPFloat(APFloat_بنیاد::semPPCDoubleDouble, Tmp.bitcast_مقصد_APInt());
  واپس Ret;
}

APFloat::op_حالت DoubleAPFloat::mod(ثابت DoubleAPFloat &RHS) {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_بنیاد::semPPCDoubleDoubleLegacy, bitcast_مقصد_APInt());
  خودکار Ret = Tmp.mod(
      APFloat(APFloat_بنیاد::semPPCDoubleDoubleLegacy, RHS.bitcast_مقصد_APInt()));
  *یہ = DoubleAPFloat(APFloat_بنیاد::semPPCDoubleDouble, Tmp.bitcast_مقصد_APInt());
  واپس Ret;
}

APFloat::op_حالت
DoubleAPFloat::fused_Multiply_جوڑو(ثابت DoubleAPFloat &Multiplicand,
                                ثابت DoubleAPFloat &Addend,
                                APFloat::rounding_منوال RM) {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_بنیاد::semPPCDoubleDoubleLegacy, bitcast_مقصد_APInt());
  خودکار Ret = Tmp.fused_Multiply_جوڑو(
      APFloat(APFloat_بنیاد::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcast_مقصد_APInt()),
      APFloat(APFloat_بنیاد::semPPCDoubleDoubleLegacy, Addend.bitcast_مقصد_APInt()),
      RM);
  *یہ = DoubleAPFloat(APFloat_بنیاد::semPPCDoubleDouble, Tmp.bitcast_مقصد_APInt());
  واپس Ret;
}

APFloat::op_حالت DoubleAPFloat::باری_مقصد_Integral(APFloat::rounding_منوال RM) {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ثابت APFloat &Hi = get_پہلا();
  ثابت APFloat &Lo = get_دوسرا();

  APFloat RoundedHi = Hi;
  ثابت op_حالت Hi_حالت = RoundedHi.باری_مقصد_Integral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  اگر (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/جھوٹ);
    واپس Hi_حالت;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  خودکار باری_مقصد_قریب_ترین_Helper = [](APFloat مقصد_باری, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    ثابت APFloat RoundingError = Rounded - مقصد_باری;
    اگر (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        مطلق_قدر(RoundingError).is_Exactly_قدر(0.5))
      Rounded.جوڑو(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rm_قریب_ترین_Ties_مقصد_Even);
    واپس Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  اگر (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    اگر (RM == rm_قریب_ترین_Ties_مقصد_Away || RM == rm_قریب_ترین_Ties_مقصد_Even)
      RoundedHi = باری_مقصد_قریب_ترین_Helper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/جھوٹ);
    واپس Hi_حالت;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  Rounding_منوال Lo_Rounding_منوال;
  اگر (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    Lo_Rounding_منوال = isNegative() ? rm_Toward_مثبت : rmTowardNegative;
  ورنہ
    Lo_Rounding_منوال = RM;

  APFloat RoundedLo = Lo;
  ثابت op_حالت Lo_حالت = RoundedLo.باری_مقصد_Integral(Lo_Rounding_منوال);
  اگر (Lo_Rounding_منوال == rm_قریب_ترین_Ties_مقصد_Away)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = باری_مقصد_قریب_ترین_Helper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fast_Two_مجموع(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  واپس Lo_حالت;
}

خالی DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmp_نتیجہ
DoubleAPFloat::compare_Absolute_قدر(ثابت DoubleAPFloat &RHS) ثابت {
  // Compare absolute values of the high parts.
  ثابت cmp_نتیجہ HiPartCmp = Floats[0].compare_Absolute_قدر(RHS.Floats[0]);
  اگر (HiPartCmp != cmpEqual)
    واپس HiPartCmp;

  // Zero, regardless of sign, is equal.
  اگر (Floats[1].isZero() && RHS.Floats[1].isZero())
    واپس cmpEqual;

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
    واپس RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  اگر (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    واپس ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  اگر (ThisIsSubtractive != RHSIsSubtractive)
    واپس ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  ثابت cmp_نتیجہ LoPartCmp = Floats[1].compare_Absolute_قدر(RHS.Floats[1]);

  اگر (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    اگر (LoPartCmp == cmpLessThan)
      واپس cmpGreaterThan;
    اگر (LoPartCmp == cmpGreaterThan)
      واپس cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  واپس LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() ثابت {
  واپس Floats[0].getCategory();
}

منطقی DoubleAPFloat::isNegative() ثابت { واپس Floats[0].isNegative(); }

خالی DoubleAPFloat::makeInf(منطقی Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ جھوٹ);
}

خالی DoubleAPFloat::makeZero(منطقی Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ جھوٹ);
}

خالی DoubleAPFloat::make_سب_سے_بڑا(منطقی Neg) {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloat_بنیاد::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloat_بنیاد::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  اگر (Neg)
    changeSign();
}

خالی DoubleAPFloat::make_سب_سے_چھوٹا(منطقی Neg) {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].make_سب_سے_چھوٹا(Neg);
  Floats[1].makeZero(/* Neg = */ جھوٹ);
}

خالی DoubleAPFloat::make_سب_سے_چھوٹا_Normalized(منطقی Neg) {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloat_بنیاد::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  اگر (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ جھوٹ);
}

خالی DoubleAPFloat::makeNaN(منطقی SNaN, منطقی Neg, ثابت APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ جھوٹ);
}

APFloat::cmp_نتیجہ DoubleAPFloat::compare(ثابت DoubleAPFloat &RHS) ثابت {
  خودکار نتیجہ = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  اگر (نتیجہ == APFloat::cmpEqual)
    واپس Floats[1].compare(RHS.Floats[1]);
  واپس نتیجہ;
}

منطقی DoubleAPFloat::bitwiseIsEqual(ثابت DoubleAPFloat &RHS) ثابت {
  واپس Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_رمز hash_قدر(ثابت DoubleAPFloat &Arg) {
  اگر (Arg.Floats)
    واپس hash_combine(hash_قدر(Arg.Floats[0]), hash_قدر(Arg.Floats[1]));
  واپس hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcast_مقصد_APInt() ثابت {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t ڈیٹا_2[] = {
      Floats[0].bitcast_مقصد_APInt().get_Raw_ڈیٹا()[0],
      Floats[1].bitcast_مقصد_APInt().get_Raw_ڈیٹا()[0],
  };
  واپس APInt(128, ڈیٹا_2);
}

Expected<APFloat::op_حالت> DoubleAPFloat::convertFromString(StringRef S,
                                                             rounding_منوال RM) {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_بنیاد::semPPCDoubleDoubleLegacy);
  خودکار Ret = Tmp.convertFromString(S, RM);
  *یہ = DoubleAPFloat(APFloat_بنیاد::semPPCDoubleDouble, Tmp.bitcast_مقصد_APInt());
  واپس Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::op_حالت DoubleAPFloat::اگلا(منطقی اگلا_نیچے_کو) {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  اگر (اگلا_نیچے_کو) {
    changeSign();
    APFloat::op_حالت نتیجہ = اگلا(/*nextDown=*/جھوٹ);
    changeSign();
    واپس نتیجہ;
  }
  انتخاب (getCategory()) {
  حالت fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    اگر (isNegative())
      make_سب_سے_بڑا(سچ);
    واپس opOK;

  حالت fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    اگر (get_پہلا().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(جھوٹ, isNegative(), خالی_اشاریہ);
      واپس opInvalidOp;
    }
    واپس opOK;

  حالت fcZero:
    // nextUp(pm 0) = +getSmallest()
    make_سب_سے_چھوٹا(جھوٹ);
    واپس opOK;

  حالت fc_معمول:
    توڑو;
  }

  ثابت APFloat &HiOld = get_پہلا();
  ثابت APFloat &LoOld = get_دوسرا();

  APFloat اگلا_Lo = LoOld;
  اگلا_Lo.اگلا(/*nextDown=*/جھوٹ);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  خودکار اندر_Lattice = [](ثابت APFloat &Hi, ثابت APFloat &Lo) {
    واپس Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  اگر (اندر_Lattice(HiOld, اگلا_Lo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(اگلا_Lo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat سب_سے_بڑا{*Semantics, uninitialized};
    سب_سے_بڑا.make_سب_سے_بڑا(/*Neg=*/جھوٹ);
    اگر (compare(سب_سے_بڑا) == cmpGreaterThan)
      makeInf(/*Neg=*/جھوٹ);

    واپس opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat اگلا_Hi = HiOld;
  اگلا_Hi.اگلا(/*nextDown=*/جھوٹ);

  // nextUp(getLargest()) == INFINITY
  اگر (اگلا_Hi.isInfinity()) {
    makeInf(/*Neg=*/جھوٹ);
    واپس opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  اگر (اگلا_Hi.isZero()) {
    makeZero(/*Neg=*/سچ);
    واپس opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  اگلا_Lo = neg(scalbn(harrisonUlp(اگلا_Hi), -1, rmTowardZero));
  اگر (!اندر_Lattice(اگلا_Hi, اگلا_Lo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    اگلا_Lo.اگلا(/*nextDown=*/جھوٹ);

  Floats[0] = std::move(اگلا_Hi);
  Floats[1] = std::move(اگلا_Lo);

  واپس opOK;
}

APFloat::op_حالت DoubleAPFloat::convert_مقصد_Sign_Extended_Integer(
    MutableArrayRef<integerPart> داخلہ, غیرعلامتی صحیح چوڑائی, منطقی IsSigned,
    rounding_منوال RM, منطقی *IsExact) ثابت {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  اگر (!get_پہلا().isFiniteNonZero() || get_دوسرا().isZero())
    واپس get_پہلا().convert_مقصد_Integer(داخلہ, چوڑائی, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = جھوٹ;
  DoubleAPFloat Integral = *یہ;
  ثابت op_حالت باری_حالت = Integral.باری_مقصد_Integral(RM);
  اگر (باری_حالت == opInvalidOp)
    واپس opInvalidOp;
  ثابت APFloat &IntegralHi = Integral.get_پہلا();
  ثابت APFloat &IntegralLo = Integral.get_دوسرا();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  منطقی HiIsExact;
  اگر (IntegralHi.isZero() || IntegralLo.isZero()) {
    ثابت op_حالت Hi_حالت =
        IntegralHi.convert_مقصد_Integer(داخلہ, چوڑائی, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    واپس Hi_حالت == opOK ? opInexact : Hi_حالت;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  اگر (!IsSigned && IntegralHi.isNegative())
    واپس opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  منطقی LoIsExact;
  ثابت صحیح HiExactLog2 = IntegralHi.getExactLog2Abs();
  ثابت غیرعلامتی مثبت_حد_سے_تجاوز_چوڑائی = IsSigned ? چوڑائی - 1 : چوڑائی;
  اگر (HiExactLog2 >= 0 &&
      static_cast<غیرعلامتی>(HiExactLog2) == مثبت_حد_سے_تجاوز_چوڑائی) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    اگر (IntegralHi.isNegative() == IntegralLo.isNegative())
      واپس opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    ثابت op_حالت Lo_حالت = IntegralLo.convert_مقصد_Integer(
        داخلہ, چوڑائی, /*IsSigned=*/سچ, RM, &LoIsExact);
    اگر (Lo_حالت == opInvalidOp)
      واپس opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    اگر (IsSigned && !IntegralHi.isNegative())
      APInt::tc_Clear_ثنائی_ہندسہ(داخلہ.ڈیٹا(), مثبت_حد_سے_تجاوز_چوڑائی);
    *IsExact = باری_حالت == opOK;
    واپس باری_حالت;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  ثابت op_حالت Hi_حالت = IntegralHi.convert_مقصد_Integer(
      داخلہ, چوڑائی, IsSigned, rmTowardZero, &HiIsExact);
  اگر (Hi_حالت == opInvalidOp)
    واپس Hi_حالت;

  // Convert Lo into a temporary integer of the same width.
  APSInt Lo_نتیجہ{چوڑائی, /*isUnsigned=*/!IsSigned};
  ثابت op_حالت Lo_حالت =
      IntegralLo.convert_مقصد_Integer(Lo_نتیجہ, rmTowardZero, &LoIsExact);
  اگر (Lo_حالت == opInvalidOp)
    واپس Lo_حالت;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tc_جوڑو(داخلہ.ڈیٹا(), Lo_نتیجہ.get_Raw_ڈیٹا(), /*carry=*/0, داخلہ.سائز_2());

  *IsExact = باری_حالت == opOK;
  واپس باری_حالت;
}

APFloat::op_حالت
DoubleAPFloat::convert_مقصد_Integer(MutableArrayRef<integerPart> داخلہ,
                                غیرعلامتی صحیح چوڑائی, منطقی IsSigned,
                                rounding_منوال RM, منطقی *IsExact) ثابت {
  op_حالت FS =
      convert_مقصد_Sign_Extended_Integer(داخلہ, چوڑائی, IsSigned, RM, IsExact);

  اگر (FS == opInvalidOp) {
    ثابت غیرعلامتی Dst_Parts_گنتی = part_گنتی_For_بٹ(چوڑائی);
    أكد(Dst_Parts_گنتی <= داخلہ.سائز_2() && "Integer too big");

    غیرعلامتی بٹ_2;
    اگر (getCategory() == fcNaN)
      بٹ_2 = 0;
    ورنہ اگر (isNegative())
      بٹ_2 = IsSigned;
    ورنہ
      بٹ_2 = چوڑائی - IsSigned;

    tc_سیٹ_کم_سے_کم_Significant_بٹ(داخلہ.ڈیٹا(), Dst_Parts_گنتی, بٹ_2);
    اگر (isNegative() && IsSigned)
      APInt::tc_سرکاؤ_بایاں(داخلہ.ڈیٹا(), Dst_Parts_گنتی, چوڑائی - 1);
  }

  واپس FS;
}

APFloat::op_حالت DoubleAPFloat::handle_حد_سے_تجاوز(rounding_منوال RM) {
  انتخاب (RM) {
  حالت APFloat::rmTowardZero:
    make_سب_سے_بڑا(/*Neg=*/isNegative());
    توڑو;
  حالت APFloat::rmTowardNegative:
    اگر (isNegative())
      makeInf(/*Neg=*/سچ);
    ورنہ
      make_سب_سے_بڑا(/*Neg=*/جھوٹ);
    توڑو;
  حالت APFloat::rm_Toward_مثبت:
    اگر (isNegative())
      make_سب_سے_بڑا(/*Neg=*/سچ);
    ورنہ
      makeInf(/*Neg=*/جھوٹ);
    توڑو;
  حالت APFloat::rm_قریب_ترین_Ties_مقصد_Away:
  حالت APFloat::rm_قریب_ترین_Ties_مقصد_Even:
    makeInf(/*Neg=*/isNegative());
    توڑو;
  طےشدہ:
    llvm_unreachable("Invalid rounding mode found");
  }
  op_حالت S = opInexact;
  اگر (!get_پہلا().isFinite())
    S = static_cast<op_حالت>(S | op_حد_سے_تجاوز);
  واپس S;
}

APFloat::op_حالت DoubleAPFloat::convertFromUnsignedParts(
    ثابت integerPart *Src, غیرعلامتی صحیح Src_گنتی, rounding_منوال RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  ثابت غیرعلامتی SrcMSB = APInt::tcMSB(Src, Src_گنتی);
  اگر (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/جھوٹ);
    واپس opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  ثابت غیرعلامتی Src_ثنائی_ہندسہ_چوڑائی = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/Src_ثنائی_ہندسہ_چوڑائی, ArrayRef(Src, Src_گنتی)},
                /*isUnsigned=*/سچ};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{get_پہلا().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/جھوٹ, rm_قریب_ترین_Ties_مقصد_Even);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  اگر (!Hi.isFinite())
    واپس handle_حد_سے_تجاوز(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  منطقی HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/سچ};
  Hi.convert_مقصد_Integer(HiAsInt, rm_قریب_ترین_Ties_مقصد_Even, &HiAsIntIsExact);
  ثابت APInt Error = SrcInt.zext(HiAsInt.get_ثنائی_ہندسہ_چوڑائی()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  rounding_منوال LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  اگر (Error.isNegative()) {
    اگر (RM == rm_قریب_ترین_Ties_مقصد_Away) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      ثابت غیرعلامتی Error_فعال_بٹ = Error.get_Significant_بٹ() - 1;
      ثابت غیرعلامتی LoPrecision = get_دوسرا().getSemantics().precision;
      اگر (Error_فعال_بٹ > LoPrecision) {
        ثابت غیرعلامتی RoundingBoundary = Error_فعال_بٹ - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        اگر (Error.گنتی_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rm_Toward_مثبت;
      }
    } ورنہ اگر (RM == rmTowardZero) {
      // For rmTowardZero, the final positive result must be truncated (rounded
      // down). When Hi is an overestimate, Error is negative. A standard
      // rmTowardZero conversion of Error would make it *less* negative,
      // effectively rounding the final sum Hi + Lo *up*. To ensure the sum
      // rounds down correctly, we force Lo to round toward -infinity.
      LoRM = rmTowardNegative;
    }
  }

  APFloat Lo{get_دوسرا().getSemantics()};
  op_حالت حالت_2 = Lo.convertFromAPInt(Error, /*IsSigned=*/سچ, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fast_Two_مجموع(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  اگر (!get_پہلا().isFinite())
    واپس handle_حد_سے_تجاوز(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  اگر (get_پہلا().isFiniteNonZero() && Floats[0].is_سب_سے_بڑا()) {
    DoubleAPFloat سب_سے_بڑا{*Semantics};
    سب_سے_بڑا.make_سب_سے_بڑا(/*Neg=*/جھوٹ);
    اگر (compare(سب_سے_بڑا) == APFloat::cmpGreaterThan)
      واپس handle_حد_سے_تجاوز(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  واپس حالت_2;
}

APFloat::op_حالت DoubleAPFloat::convertFromAPInt(ثابت APInt &داخلہ,
                                                  منطقی IsSigned,
                                                  rounding_منوال RM) {
  ثابت منطقی Negate_داخلہ = IsSigned && داخلہ.isNegative();
  APInt API = داخلہ;
  اگر (Negate_داخلہ)
    API.negate();

  ثابت APFloat::op_حالت حالت_2 =
      convertFromUnsignedParts(API.get_Raw_ڈیٹا(), API.get_عدد_الفاظ(), RM);
  اگر (Negate_داخلہ)
    changeSign();
  واپس حالت_2;
}

غیرعلامتی صحیح DoubleAPFloat::convert_مقصد_Hex_String(حرف *DST,
                                               غیرعلامتی صحیح HexDigits,
                                               منطقی بالائی_Case_2,
                                               rounding_منوال RM) ثابت {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  واپس APFloat(APFloat_بنیاد::semPPCDoubleDoubleLegacy, bitcast_مقصد_APInt())
      .convert_مقصد_Hex_String(DST, HexDigits, بالائی_Case_2, RM);
}

منطقی DoubleAPFloat::isDenormal() ثابت {
  واپس getCategory() == fc_معمول &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

منطقی DoubleAPFloat::is_سب_سے_چھوٹا() ثابت {
  اگر (getCategory() != fc_معمول)
    واپس جھوٹ;
  DoubleAPFloat Tmp(*یہ);
  Tmp.make_سب_سے_چھوٹا(یہ->isNegative());
  واپس Tmp.compare(*یہ) == cmpEqual;
}

منطقی DoubleAPFloat::is_سب_سے_چھوٹا_Normalized() ثابت {
  اگر (getCategory() != fc_معمول)
    واپس جھوٹ;

  DoubleAPFloat Tmp(*یہ);
  Tmp.make_سب_سے_چھوٹا_Normalized(یہ->isNegative());
  واپس Tmp.compare(*یہ) == cmpEqual;
}

منطقی DoubleAPFloat::is_سب_سے_بڑا() ثابت {
  اگر (getCategory() != fc_معمول)
    واپس جھوٹ;
  DoubleAPFloat Tmp(*یہ);
  Tmp.make_سب_سے_بڑا(یہ->isNegative());
  واپس Tmp.compare(*یہ) == cmpEqual;
}

منطقی DoubleAPFloat::isInteger() ثابت {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  واپس Floats[0].isInteger() && Floats[1].isInteger();
}

خالی DoubleAPFloat::مقصد_String(Small_سمتیہ_Impl<حرف> &Str,
                             غیرعلامتی FormatPrecision,
                             غیرعلامتی FormatMaxPadding,
                             منطقی TruncateZero) ثابت {
  أكد(Semantics == &APFloat_بنیاد::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloat_بنیاد::semPPCDoubleDoubleLegacy, bitcast_مقصد_APInt())
      .مقصد_String(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

صحیح DoubleAPFloat::getExactLog2Abs() ثابت {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  اگر (get_دوسرا().isNonZero())
    واپس INT_MIN;
  واپس get_پہلا().getExactLog2Abs();
}

صحیح ilogb(ثابت DoubleAPFloat &Arg) {
  ثابت APFloat &Hi = Arg.get_پہلا();
  ثابت APFloat &Lo = Arg.get_دوسرا();
  صحیح Ilogb_نتیجہ = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  اگر (Arg.getCategory() != fc_معمول)
    واپس Ilogb_نتیجہ;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  اگر (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    واپس Ilogb_نتیجہ;
  اگر (Hi.getExactLog2Abs() == INT_MIN)
    واپس Ilogb_نتیجہ;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  واپس Ilogb_نتیجہ - 1;
}

DoubleAPFloat scalbn(ثابت DoubleAPFloat &Arg, صحیح Exp,
                     APFloat::rounding_منوال RM) {
  أكد(Arg.Semantics == &APFloat_بنیاد::PPCDoubleDouble() &&
         "Unexpected Semantics");
  واپس DoubleAPFloat(APFloat_بنیاد::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(ثابت DoubleAPFloat &Arg, صحیح &Exp,
                    APFloat::rounding_منوال RM) {
  أكد(Arg.Semantics == &APFloat_بنیاد::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  اگر (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.get_پہلا() = Quiet.get_پہلا().makeQuiet();
    واپس Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  اگر (Exp == APFloat::IEK_Inf)
    واپس Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  اگر (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    واپس Arg;
  }

  ثابت APFloat &Hi = Arg.get_پہلا();
  ثابت APFloat &Lo = Arg.get_دوسرا();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  ثابت منطقی SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat دوسرا = Lo;
  اگر (Arg.getCategory() == APFloat::fc_معمول && Lo.isFiniteNonZero()) {
    rounding_منوال Lo_Rounding_منوال;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    اگر (RM == rmTowardZero)
      Lo_Rounding_منوال = Arg.isNegative() ? rm_Toward_مثبت : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    ورنہ اگر (RM == rm_قریب_ترین_Ties_مقصد_Away && SignsDisagree && Exp > 0)
      Lo_Rounding_منوال = rm_قریب_ترین_Ties_مقصد_Even;
    ورنہ
      Lo_Rounding_منوال = RM;
    دوسرا = scalbn(Lo, -Exp, Lo_Rounding_منوال);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    اگر (RM == rm_قریب_ترین_Ties_مقصد_Away && Lo_Rounding_منوال == rm_قریب_ترین_Ties_مقصد_Even) {
      // Re-scale the result back to check if rounding occurred.
      ثابت APFloat RecomposedLo = scalbn(دوسرا, Exp, rm_قریب_ترین_Ties_مقصد_Even);
      اگر (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        ثابت APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        ثابت APFloat Ulp_کا_دوسرا = harrisonUlp(دوسرا);
        ثابت APFloat Scaled_Ulp_کا_دوسرا =
            scalbn(Ulp_کا_دوسرا, Exp - 1, rm_قریب_ترین_Ties_مقصد_Even);
        ثابت منطقی IsMidpoint = مطلق_قدر(RoundingError) == Scaled_Ulp_کا_دوسرا;
        ثابت منطقی RoundedLoAway =
            دوسرا.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        اگر (IsMidpoint && RoundedLoAway)
          دوسرا.اگلا(/*nextDown=*/!دوسرا.isNegative());
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
    اگر (دوسرا.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat پہلا = scalbn(Hi, -Exp, RM);
  واپس DoubleAPFloat(APFloat_بنیاد::PPCDoubleDouble(), std::move(پہلا),
                       std::move(دوسرا));
}

APInt DoubleAPFloat::getNaNPayload() ثابت { واپس Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, ثابت fltSemantics &Semantics) {
  اگر (استعمال_کرتا_Layout<IEEEFloat>(Semantics)) {
    نیا (&IEEE) IEEEFloat(std::move(F));
    واپس;
  }
  اگر (استعمال_کرتا_Layout<DoubleAPFloat>(Semantics)) {
    ثابت fltSemantics& S = F.getSemantics();
    نیا (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloat_بنیاد::IEEEdouble()));
    واپس;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::op_حالت> APFloat::convertFromString(StringRef Str,
                                                       rounding_منوال RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_رمز hash_قدر(ثابت APFloat &Arg) {
  اگر (APFloat::استعمال_کرتا_Layout<detail::IEEEFloat>(Arg.getSemantics()))
    واپس hash_قدر(Arg.U.IEEE);
  اگر (APFloat::استعمال_کرتا_Layout<detail::DoubleAPFloat>(Arg.getSemantics()))
    واپس hash_قدر(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(ثابت fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  خودکار حالت_Or_Err = convertFromString(S, rm_قریب_ترین_Ties_مقصد_Even);
  أكد(حالت_Or_Err && "Invalid floating point representation");
  consumeError(حالت_Or_Err.takeError());
}

FPClassTest APFloat::classify() ثابت {
  اگر (isZero())
    واپس isNegative() ? fcNegZero : fcPosZero;
  اگر (is_معمول())
    واپس isNegative() ? fc_Neg_معمول : fc_Pos_معمول;
  اگر (isDenormal())
    واپس isNegative() ? fcNegSubnormal : fcPosSubnormal;
  اگر (isInfinity())
    واپس isNegative() ? fcNegInf : fcPosInf;
  أكد(isNaN() && "Other class of FP constant");
  واپس isSignaling() ? fcSNan : fcQNan;
}

منطقی APFloat::getExactInverse(APFloat *Inv) ثابت {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  اگر (!isFiniteNonZero())
    واپس جھوٹ;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  اگر (isDenormal())
    واپس جھوٹ;

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
    واپس جھوٹ;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  اگر (Reciprocal.getExactLog2Abs() != -Exp)
    واپس جھوٹ;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  اگر (Reciprocal.isDenormal())
    واپس جھوٹ;

  أكد(Reciprocal.isFiniteNonZero());

  اگر (Inv)
    *Inv = std::move(Reciprocal);

  واپس سچ;
}

APFloat::op_حالت APFloat::convert(ثابت fltSemantics &مقصد_Semantics_2,
                                   rounding_منوال RM, منطقی *losesInfo) {
  اگر (&getSemantics() == &مقصد_Semantics_2) {
    *losesInfo = جھوٹ;
    واپس opOK;
  }
  اگر (استعمال_کرتا_Layout<IEEEFloat>(getSemantics()) &&
      استعمال_کرتا_Layout<IEEEFloat>(مقصد_Semantics_2))
    واپس U.IEEE.convert(مقصد_Semantics_2, RM, losesInfo);
  اگر (استعمال_کرتا_Layout<IEEEFloat>(getSemantics()) &&
      استعمال_کرتا_Layout<DoubleAPFloat>(مقصد_Semantics_2)) {
    أكد(&مقصد_Semantics_2 == &APFloat_بنیاد::semPPCDoubleDouble);
    خودکار Ret =
        U.IEEE.convert(APFloat_بنیاد::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *یہ = APFloat(مقصد_Semantics_2, U.IEEE.bitcast_مقصد_APInt());
    واپس Ret;
  }
  اگر (استعمال_کرتا_Layout<DoubleAPFloat>(getSemantics()) &&
      استعمال_کرتا_Layout<IEEEFloat>(مقصد_Semantics_2)) {
    خودکار Ret = getIEEE().convert(مقصد_Semantics_2, RM, losesInfo);
    *یہ = APFloat(std::move(getIEEE()), مقصد_Semantics_2);
    واپس Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_قدر(ثابت fltSemantics &Semantics) {
  واپس APFloat(Semantics, APInt::getAllOnes(Semantics.سائز_اندر_بٹ));
}

خالی APFloat::print(raw_ostream &OS) ثابت {
  Small_سمتیہ<حرف, 16> بفر;
  مقصد_String(بفر);
  OS << بفر;
}

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طریقہ خالی APFloat::dump() ثابت {
  print(dbgs());
  dbgs() << '\n';
}
#اگر_ختم

خالی APFloat::Profile(Folding_سیٹ_گرہ_ID &NID) ثابت {
  NID.جوڑو_2(bitcast_مقصد_APInt());
}

APFloat::op_حالت APFloat::convert_مقصد_Integer(APSInt &نتیجہ_2,
                                            rounding_منوال rounding_منوال_2,
                                            منطقی *isExact) ثابت {
  غیرعلامتی ثنائی_ہندسہ_چوڑائی = نتیجہ_2.get_ثنائی_ہندسہ_چوڑائی();
  Small_سمتیہ<uint64_t, 4> parts(نتیجہ_2.get_عدد_الفاظ());
  op_حالت حالت_3 = convert_مقصد_Integer(parts, ثنائی_ہندسہ_چوڑائی, نتیجہ_2.isSigned(),
                                     rounding_منوال_2, isExact);
  // Keeps the original signed-ness.
  نتیجہ_2 = APInt(ثنائی_ہندسہ_چوڑائی, parts);
  واپس حالت_3;
}

دوہرا APFloat::convert_مقصد_Double() ثابت {
  اگر (&getSemantics() == &APFloat_بنیاد::semIEEEdouble)
    واپس getIEEE().convert_مقصد_Double();
  أكد(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat عارضی = *یہ;
  منطقی LosesInfo;
  [[maybe_غیر_مستعمل]] op_حالت St =
      عارضی.convert(APFloat_بنیاد::semIEEEdouble, rm_قریب_ترین_Ties_مقصد_Even, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  واپس عارضی.getIEEE().convert_مقصد_Double();
}

#اگر_تعریف_شدہ HAS_IEE754_FLOAT128
float128 APFloat::convert_مقصد_Quad() ثابت {
  اگر (&getSemantics() == &APFloat_بنیاد::semIEEEquad)
    واپس getIEEE().convert_مقصد_Quad();
  أكد(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat عارضی = *یہ;
  منطقی LosesInfo;
  [[maybe_غیر_مستعمل]] op_حالت St =
      عارضی.convert(APFloat_بنیاد::semIEEEquad, rm_قریب_ترین_Ties_مقصد_Even, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  واپس عارضی.getIEEE().convert_مقصد_Quad();
}
#اگر_ختم

اعشاری APFloat::convert_مقصد_Float() ثابت {
  اگر (&getSemantics() == &APFloat_بنیاد::semIEEEsingle)
    واپس getIEEE().convert_مقصد_Float();
  أكد(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat عارضی = *یہ;
  منطقی LosesInfo;
  [[maybe_غیر_مستعمل]] op_حالت St =
      عارضی.convert(APFloat_بنیاد::semIEEEsingle, rm_قریب_ترین_Ties_مقصد_Even, &LosesInfo);
  أكد(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  واپس عارضی.getIEEE().convert_مقصد_Float();
}

غیرعلامتی APFloat_بنیاد::get_Arbitrary_FPFormat_سائز_اندر_بٹ(StringRef Format) {
  واپس StringSwitch<غیرعلامتی>(Format)
      .Case("Float8E5M2", get_سائز_اندر_بٹ(semFloat8E5M2))
      .Case("Float8E5M2FNUZ", get_سائز_اندر_بٹ(semFloat8E5M2FNUZ))
      .Case("Float8E4M3", get_سائز_اندر_بٹ(semFloat8E4M3))
      .Case("Float8E4M3FN", get_سائز_اندر_بٹ(semFloat8E4M3FN))
      .Case("Float8E4M3FNUZ", get_سائز_اندر_بٹ(semFloat8E4M3FNUZ))
      .Case("Float8E4M3B11FNUZ", get_سائز_اندر_بٹ(semFloat8E4M3B11FNUZ))
      .Case("Float8E3M4", get_سائز_اندر_بٹ(semFloat8E3M4))
      .Case("Float8E8M0FNU", get_سائز_اندر_بٹ(semFloat8E8M0FNU))
      .Case("Float6E3M2FN", get_سائز_اندر_بٹ(semFloat6E3M2FN))
      .Case("Float6E2M3FN", get_سائز_اندر_بٹ(semFloat6E2M3FN))
      .Case("Float4E2M1FN", get_سائز_اندر_بٹ(semFloat4E2M1FN))
      .Case("Float8E5M3FNU", get_سائز_اندر_بٹ(semFloat8E5M3FNU))
      .Default(0);
}

منطقی APFloat_بنیاد::isValidArbitraryFPFormat(StringRef Format) {
  واپس get_Arbitrary_FPFormat_سائز_اندر_بٹ(Format) != 0;
}

ثابت fltSemantics *APFloat_بنیاد::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  واپس StringSwitch<ثابت fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(خالی_اشاریہ);
}

APFloat::Storage::~Storage() {
  اگر (استعمال_کرتا_Layout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    واپس;
  }
  اگر (استعمال_کرتا_Layout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    واپس;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(ثابت APFloat::Storage &RHS) {
  اگر (استعمال_کرتا_Layout<IEEEFloat>(*RHS.semantics)) {
    نیا (یہ) IEEEFloat(RHS.IEEE);
    واپس;
  }
  اگر (استعمال_کرتا_Layout<DoubleAPFloat>(*RHS.semantics)) {
    نیا (یہ) DoubleAPFloat(RHS.Double);
    واپس;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  اگر (استعمال_کرتا_Layout<IEEEFloat>(*RHS.semantics)) {
    نیا (یہ) IEEEFloat(std::move(RHS.IEEE));
    واپس;
  }
  اگر (استعمال_کرتا_Layout<DoubleAPFloat>(*RHS.semantics)) {
    نیا (یہ) DoubleAPFloat(std::move(RHS.Double));
    واپس;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::عامل=(ثابت APFloat::Storage &RHS) {
  اگر (استعمال_کرتا_Layout<IEEEFloat>(*semantics) &&
      استعمال_کرتا_Layout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } ورنہ اگر (استعمال_کرتا_Layout<DoubleAPFloat>(*semantics) &&
             استعمال_کرتا_Layout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } ورنہ اگر (یہ != &RHS) {
    یہ->~Storage();
    نیا (یہ) Storage(RHS);
  }
  واپس *یہ;
}

APFloat::Storage &APFloat::Storage::عامل=(APFloat::Storage &&RHS) {
  اگر (استعمال_کرتا_Layout<IEEEFloat>(*semantics) &&
      استعمال_کرتا_Layout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } ورنہ اگر (استعمال_کرتا_Layout<DoubleAPFloat>(*semantics) &&
             استعمال_کرتا_Layout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } ورنہ اگر (یہ != &RHS) {
    یہ->~Storage();
    نیا (یہ) Storage(std::move(RHS));
  }
  واپس *یہ;
}

نامخلا {

APFloat::op_حالت get_Op_حالت_From_Libc(صحیح libc_exceptions) {
  APFloat::op_حالت حالت_3 = APFloat::opOK;
  اگر (libc_exceptions & FE_INVALID)
    حالت_3 = static_cast<APFloat::op_حالت>(حالت_3 | APFloat::opInvalidOp);
  اگر (libc_exceptions & FE_DIVBYZERO)
    حالت_3 = static_cast<APFloat::op_حالت>(حالت_3 | APFloat::opDivByZero);
  اگر (libc_exceptions & FE_حد_سے_تجاوز)
    حالت_3 = static_cast<APFloat::op_حالت>(حالت_3 | APFloat::op_حد_سے_تجاوز);
  اگر (libc_exceptions & FE_UNDERFLOW)
    حالت_3 = static_cast<APFloat::op_حالت>(حالت_3 | APFloat::opUnderflow);
  اگر (libc_exceptions & FE_INEXACT)
    حالت_3 = static_cast<APFloat::op_حالت>(حالت_3 | APFloat::opInexact);
  واپس حالت_3;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> قوت_نما(ثابت APFloat &x, Rounding_منوال rounding_منوال_2,
                           APFloat::op_حالت *حالت_3) {

  اگر (rounding_منوال_2 == APFloat_بنیاد::rm_قریب_ترین_Ties_مقصد_Even) {
    اگر (APFloat::Semantics_مقصد_Enum(x.getSemantics()) ==
        APFloat_بنیاد::S_IEEEsingle) {
      اعشاری x_val = x.convert_مقصد_Float();
      صحیح exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      اگر (حالت_3) {
        *حالت_3 = get_Op_حالت_From_Libc(exc);
        اگر (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *حالت_3 =
              static_cast<APFloat::op_حالت>(*حالت_3 | APFloat::opInvalidOp);
        }
      }
      اعشاری نتیجہ_2 = LIBC_NAMESPACE::shared::expf(x_val);
      واپس APFloat(نتیجہ_2);
    }
    اگر (APFloat::Semantics_مقصد_Enum(x.getSemantics()) ==
        APFloat_بنیاد::S_IEEEdouble) {
      دوہرا x_val = x.convert_مقصد_Double();
      صحیح exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      اگر (حالت_3) {
        *حالت_3 = get_Op_حالت_From_Libc(exc);
        اگر (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *حالت_3 =
              static_cast<APFloat::op_حالت>(*حالت_3 | APFloat::opInvalidOp);
        }
      }
      دوہرا نتیجہ_2 = LIBC_NAMESPACE::shared::قوت_نما(x_val);
      واپس APFloat(نتیجہ_2);
    }
  }
  واپس std::nullopt;
}

} // namespace llvm

#تعریف_منسوخ APFLOAT_DISPATCH_ON_SEMANTICS
