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
#परिभाषय LIBC_NAMESPACE __llvm_libc_apfloat
#परिभाषय LIBC_MATH (LIBC_MATH_NO_ERRNO | LIBC_MATH_NO_EXCEPT)

#अन्तर्भाव "shared/math.h"
#अन्तर्भाव "shared/math_check_exceptions.h"

#परिभाषय APFLOAT_DISPATCH_ON_SEMANTICS(विधानम्_CALL)                             \
  कुरु {                                                                         \
    यदि (उपयुङ्क्ते_Layout<IEEEFloat>(getSemantics()))                                 \
      प्रत्यागम U.IEEE.विधानम्_CALL;                                               \
    यदि (उपयुङ्क्ते_Layout<DoubleAPFloat>(getSemantics()))                             \
      प्रत्यागम U.Double.विधानम्_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } यावत् (असत्य)

उपयुज्य नामाकाश llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#परिभाषय Pack_Categories_Into_कुञ्चिका(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloat_आधारः::integer_Part_विस्तारः % 4 == 0, "Part width must be divisible by 4!");

नामाकाश llvm {

constexpr fltSemantics APFloat_आधारः::semIEEEhalf = {15, -14, 11, 16};
constexpr fltSemantics APFloat_आधारः::semBFloat = {127, -126, 8, 16};
constexpr fltSemantics APFloat_आधारः::semIEEEsingle = {127, -126, 24, 32};
constexpr fltSemantics APFloat_आधारः::semIEEEdouble = {1023, -1022, 53, 64};
constexpr fltSemantics APFloat_आधारः::semIEEEquad = {16383, -16382, 113, 128};
constexpr fltSemantics APFloat_आधारः::semX87DoubleExtended = {
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
constexpr fltSemantics APFloat_आधारः::semPPCDoubleDouble = {-1, 0, 0, 128};
constexpr fltSemantics APFloat_आधारः::semPPCDoubleDoubleLegacy = {
    1023, -1022 + 53, 53 + 53, 128};
constexpr fltSemantics APFloat_आधारः::semFloat8E5M2 = {15, -14, 3, 8};
constexpr fltSemantics APFloat_आधारः::semFloat8E5M2FNUZ = {
    15, -15, 3, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_आधारः::semFloat8E4M3 = {7, -6, 4, 8};
constexpr fltSemantics APFloat_आधारः::semFloat8E4M3FN = {
    8, -6, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::AllOnes};
constexpr fltSemantics APFloat_आधारः::semFloat8E4M3FNUZ = {
    7, -7, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_आधारः::semFloat8E4M3B11FNUZ = {
    4, -10, 4, 8, fltNonfiniteBehavior::NanOnly, fltNanEncoding::NegativeZero};
constexpr fltSemantics APFloat_आधारः::semFloat8E3M4 = {3, -2, 5, 8};
constexpr fltSemantics APFloat_आधारः::semFloatTF32 = {127, -126, 11, 19};
constexpr fltSemantics APFloat_आधारः::semFloat8E8M0FNU = {
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

constexpr fltSemantics APFloat_आधारः::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    सत्य,
    असत्य,
    असत्य};

constexpr fltSemantics APFloat_आधारः::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_आधारः::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_आधारः::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloat_आधारः::semBogus = {0, 0, 0, 0};

ध्रुव llvm::fltSemantics &APFloat_आधारः::Enum_गन्तव्यम्_Semantics(Semantics S) {
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

APFloat_आधारः::Semantics
APFloat_आधारः::Semantics_गन्तव्यम्_Enum(ध्रुव llvm::fltSemantics &Sem) {
  यदि (&Sem == &llvm::APFloat::IEEEhalf())
    प्रत्यागम S_IEEEhalf;
  अन्यथा यदि (&Sem == &llvm::APFloat::BFloat())
    प्रत्यागम S_BFloat;
  अन्यथा यदि (&Sem == &llvm::APFloat::IEEEsingle())
    प्रत्यागम S_IEEEsingle;
  अन्यथा यदि (&Sem == &llvm::APFloat::IEEEdouble())
    प्रत्यागम S_IEEEdouble;
  अन्यथा यदि (&Sem == &llvm::APFloat::IEEEquad())
    प्रत्यागम S_IEEEquad;
  अन्यथा यदि (&Sem == &llvm::APFloat::x87DoubleExtended())
    प्रत्यागम S_x87DoubleExtended;
  अन्यथा यदि (&Sem == &llvm::APFloat::PPCDoubleDouble())
    प्रत्यागम S_PPCDoubleDouble;
  अन्यथा यदि (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    प्रत्यागम S_PPCDoubleDoubleLegacy;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E5M2())
    प्रत्यागम S_Float8E5M2;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    प्रत्यागम S_Float8E5M2FNUZ;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E4M3())
    प्रत्यागम S_Float8E4M3;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E4M3FN())
    प्रत्यागम S_Float8E4M3FN;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    प्रत्यागम S_Float8E4M3FNUZ;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    प्रत्यागम S_Float8E4M3B11FNUZ;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E3M4())
    प्रत्यागम S_Float8E3M4;
  अन्यथा यदि (&Sem == &llvm::APFloat::FloatTF32())
    प्रत्यागम S_FloatTF32;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E8M0FNU())
    प्रत्यागम S_Float8E8M0FNU;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E5M3FNU())
    प्रत्यागम S_Float8E5M3FNU;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float6E3M2FN())
    प्रत्यागम S_Float6E3M2FN;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float6E2M3FN())
    प्रत्यागम S_Float6E2M3FN;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float4E2M1FN())
    प्रत्यागम S_Float4E2M1FN;
  अन्यथा
    llvm_unreachable("Unknown floating semantics");
}

तार्किक APFloat_आधारः::isRepresentableBy(ध्रुव fltSemantics &A,
                                    ध्रुव fltSemantics &B) {
  प्रत्यागम A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

तार्किक APFloat_आधारः::is_Losslessly_Convertible_गन्तव्यम्(ध्रुव fltSemantics &From,
                                            ध्रुव fltSemantics &Initial_गन्तव्यम्,
                                            तार्किक IgnoreNaNs) {
  यदि (&From == &Initial_गन्तव्यम्)
    प्रत्यागम सत्य;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  यदि (&From == &semPPCDoubleDouble)
    प्रत्यागम असत्य;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  ध्रुव fltSemantics &गन्तव्यम् =
      &Initial_गन्तव्यम् == &semPPCDoubleDouble ? semIEEEdouble : Initial_गन्तव्यम्;

  यदि (!isRepresentableBy(From, गन्तव्यम्))
    प्रत्यागम असत्य;

  यदि ((From.hasZero && !गन्तव्यम्.hasZero) ||
      (From.hasSignedRepr && !गन्तव्यम्.hasSignedRepr))
    प्रत्यागम असत्य;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  तार्किक FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  तार्किक गन्तव्यम्_Has_Signed_Zero = गन्तव्यम्.hasZero && गन्तव्यम्.hasSignedRepr &&
                         गन्तव्यम्.nanEncoding != fltNanEncoding::NegativeZero;
  यदि (FromHasSignedZero && !गन्तव्यम्_Has_Signed_Zero)
    प्रत्यागम असत्य;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat SmallestFrom = APFloat::getSmallest(From);
  तार्किक LosesInfo = असत्य;
  (शून्य)SmallestFrom.convert(गन्तव्यम्, APFloat::rm_निकटतमः_Ties_गन्तव्यम्_Even, &LosesInfo);
  यदि (LosesInfo)
    प्रत्यागम असत्य;

  यदि (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    प्रत्यागम सत्य;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  यदि (IgnoreNaNs) {
    यदि (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      प्रत्यागम गन्तव्यम्.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    प्रत्यागम गन्तव्यम्.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  यदि (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    प्रत्यागम असत्य;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  यदि (गन्तव्यम्.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    प्रत्यागम सत्य;
  प्रत्यागम गन्तव्यम्.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
         From.nanEncoding == गन्तव्यम्.nanEncoding;
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
[[maybe_अप्रयुक्तम्]] constexpr अचिह्नित maxExponent = 16383;

// Decimal exponents in the f16 and f32 ranges need at most 2 and 6 parts.
// Keep 16 parts (128 bytes) inline: enough for 5^413, but not a large stack
// frame. Longer fractional spellings use heap storage.
स्थिर constexpr अचिह्नित अङ्कः_Pow5_Parts_On_राशिः = 16;

स्थिर अचिह्नित पूर्णाङ्क power_सम्बन्धि_Five_Parts_For_Exponent(अचिह्नित पूर्णाङ्क power) {
  प्रतिज्ञा(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  प्रत्यागम 2 + ((power * 815) / (351 * APFloat_आधारः::integer_Part_विस्तारः));
}

अचिह्नित पूर्णाङ्क APFloat_आधारः::semanticsPrecision(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.precision;
}
APFloat_आधारः::ExponentType
APFloat_आधारः::semanticsMaxExponent(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.maxExponent;
}
APFloat_आधारः::ExponentType
APFloat_आधारः::semanticsMinExponent(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.minExponent;
}
अचिह्नित पूर्णाङ्क APFloat_आधारः::semantics_आकार_अन्तर्_द्वयङ्काः(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.आकार_अन्तर्_द्वयङ्काः;
}
अचिह्नित पूर्णाङ्क APFloat_आधारः::semantics_Int_आकार_अन्तर्_द्वयङ्काः(ध्रुव fltSemantics &semantics,
                                                 तार्किक isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  अचिह्नित पूर्णाङ्क Min_Bit_विस्तारः = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  यदि (isSigned)
    ++Min_Bit_विस्तारः;
  प्रत्यागम Min_Bit_विस्तारः;
}

तार्किक APFloat_आधारः::semanticsHasZero(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.hasZero;
}

तार्किक APFloat_आधारः::semanticsHasSignedRepr(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.hasSignedRepr;
}

तार्किक APFloat_आधारः::semanticsHasInf(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

तार्किक APFloat_आधारः::semanticsHasNaN(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

तार्किक APFloat_आधारः::isIEEELikeFP(ध्रुव fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  प्रत्यागम Semantics_गन्तव्यम्_Enum(semantics) <= S_IEEEquad;
}

तार्किक APFloat_आधारः::has_Sign_Bit_अन्तर्_MSB(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.has_Sign_Bit_अन्तर्_MSB;
}

तार्किक APFloat_आधारः::is_Representable_As_प्रसामान्यम्_अन्तर्(ध्रुव fltSemantics &Src,
                                            ध्रुव fltSemantics &Dst) {
  // Exponent range must be larger.
  यदि (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    प्रत्यागम असत्य;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  प्रत्यागम Dst.precision >= Src.precision;
}

अचिह्नित APFloat_आधारः::get_आकार_अन्तर्_द्वयङ्काः(ध्रुव fltSemantics &Sem) {
  प्रत्यागम Sem.आकार_अन्तर्_द्वयङ्काः;
}

स्थिर constexpr APFloat_आधारः::ExponentType
exponentZero(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.minExponent - 1;
}

स्थिर constexpr APFloat_आधारः::ExponentType
exponentInf(ध्रुव fltSemantics &semantics) {
  प्रत्यागम semantics.maxExponent + 1;
}

स्थिर constexpr APFloat_आधारः::ExponentType
exponentNaN(ध्रुव fltSemantics &semantics) {
  यदि (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    यदि (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      प्रत्यागम exponentZero(semantics);
    यदि (semantics.hasSignedRepr || semantics.precision > 1)
      प्रत्यागम semantics.maxExponent;
  }
  प्रत्यागम semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

स्थिर अन्तःस्थ Error createError(ध्रुव Twine &Err) {
  प्रत्यागम make_error<StringError>(Err, inconvertible_Error_संहिता());
}

स्थिर constexpr अन्तःस्थ अचिह्नित पूर्णाङ्क part_गणनम्_For_द्वयङ्काः(अचिह्नित पूर्णाङ्क द्वयङ्काः) {
  प्रत्यागम std::max(1u, (द्वयङ्काः + APFloat_आधारः::integer_Part_विस्तारः - 1) /
                          APFloat_आधारः::integer_Part_विस्तारः);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
स्थिर अन्तःस्थ अचिह्नित पूर्णाङ्क
dec_Digit_मानम्(अचिह्नित पूर्णाङ्क c)
{
  प्रत्यागम c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
स्थिर Expected<पूर्णाङ्क> readExponent(StringRef::iterator begin,
                                  StringRef::iterator अन्तः) {
  ध्रुव अचिह्नित पूर्णाङ्क overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  यदि (p == अन्तः || ((*p == '-' || *p == '+') && (p + 1) == अन्तः))
    प्रत्यागम 0;

  तार्किक isNegative = *p == '-';
  यदि (*p == '-' || *p == '+') {
    p++;
    यदि (p == अन्तः)
      प्रत्यागम createError("Exponent has no digits");
  }

  अचिह्नित absExponent = dec_Digit_मानम्(*p++);
  यदि (absExponent >= 10U)
    प्रत्यागम createError("Invalid character in exponent");

  प्रति (; p != अन्तः; ++p) {
    अचिह्नित मानम्_2 = dec_Digit_मानम्(*p);
    यदि (मानम्_2 >= 10U)
      प्रत्यागम createError("Invalid character in exponent");

    absExponent = absExponent * 10U + मानम्_2;
    यदि (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      भङ्ग;
    }
  }

  यदि (isNegative)
    प्रत्यागम -(पूर्णाङ्क) absExponent;
  अन्यथा
    प्रत्यागम (पूर्णाङ्क) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
स्थिर Expected<पूर्णाङ्क> योग_Exponent(StringRef::iterator p,
                                   StringRef::iterator अन्तः,
                                   पूर्णाङ्क exponentAdjustment) {
  पूर्णाङ्क exponent = 0;

  यदि (p == अन्तः)
    प्रत्यागम createError("Exponent has no digits");

  तार्किक negative = *p == '-';
  यदि (*p == '-' || *p == '+') {
    p++;
    यदि (p == अन्तः)
      प्रत्यागम createError("Exponent has no digits");
  }

  पूर्णाङ्क unsignedExponent = 0;
  तार्किक अतिप्रवाहः_2 = असत्य;
  प्रति (; p != अन्तः; ++p) {
    अचिह्नित पूर्णाङ्क मानम्_2;

    मानम्_2 = dec_Digit_मानम्(*p);
    यदि (मानम्_2 >= 10U)
      प्रत्यागम createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + मानम्_2;
    यदि (unsignedExponent > 32767) {
      अतिप्रवाहः_2 = सत्य;
      भङ्ग;
    }
  }

  यदि (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    अतिप्रवाहः_2 = सत्य;

  यदि (!अतिप्रवाहः_2) {
    exponent = unsignedExponent;
    यदि (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    यदि (exponent > 32767 || exponent < -32768)
      अतिप्रवाहः_2 = सत्य;
  }

  यदि (अतिप्रवाहः_2)
    exponent = negative ? -32768: 32767;

  प्रत्यागम exponent;
}

स्थिर Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator अन्तः,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = अन्तः;
  यावत् (p != अन्तः && *p == '0')
    p++;

  यदि (p != अन्तः && *p == '.') {
    *dot = p++;

    यदि (अन्तः - begin == 1)
      प्रत्यागम createError("Significand has no digits");

    यावत् (p != अन्तः && *p == '0')
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
  ध्रुव वर्ण *चरमः_Sig_Digit;
  पूर्णाङ्क exponent;
  पूर्णाङ्क normalizedExponent;
};

स्थिर Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator अन्तः, decimalInfo *D) {
  StringRef::iterator dot = अन्तः;

  स्वचालित PtrOrErr = skipLeadingZeroesAndAnyDot(begin, अन्तः, &dot);
  यदि (!PtrOrErr)
    प्रत्यागम PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->firstSigDigit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  प्रति (; p != अन्तः; ++p) {
    यदि (*p == '.') {
      यदि (dot != अन्तः)
        प्रत्यागम createError("String contains multiple dots");
      dot = p++;
      यदि (p == अन्तः)
        भङ्ग;
    }
    यदि (dec_Digit_मानम्(*p) >= 10U)
      भङ्ग;
  }

  यदि (p != अन्तः) {
    यदि (*p != 'e' && *p != 'E')
      प्रत्यागम createError("Invalid character in significand");
    यदि (p == begin)
      प्रत्यागम createError("Significand has no digits");
    यदि (dot != अन्तः && p - begin == 1)
      प्रत्यागम createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    स्वचालित ExpOrErr = readExponent(p + 1, अन्तः);
    यदि (!ExpOrErr)
      प्रत्यागम ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    यदि (dot == अन्तः)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  यदि (p != D->firstSigDigit) {
    /* Drop insignificant trailing zeroes.  */
    यदि (p != begin) {
      कुरु
        कुरु
          p--;
        यावत् (p != begin && *p == '0');
      यावत् (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->firstSigDigit)
                                      - (dot > D->firstSigDigit && dot < p)));
  }

  D->चरमः_Sig_Digit = p;
  प्रत्यागम Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
स्थिर Expected<नष्टम्_Fraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator अन्तः,
                            अचिह्नित पूर्णाङ्क digit_मानम्) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  यदि (digit_मानम् > 8)
    प्रत्यागम lf_अधिकम्_Than_Half;
  अन्यथा यदि (digit_मानम् < 8 && digit_मानम् > 0)
    प्रत्यागम lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  यावत् (p != अन्तः && (*p == '0' || *p == '.'))
    p++;

  यदि (p == अन्तः)
    प्रत्यागम createError("Invalid trailing hexadecimal fraction!");

  अचिह्नित hexDigit = hex_Digit_मानम्(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  यदि (hexDigit == UINT_MAX)
    प्रत्यागम digit_मानम् == 0 ? lfExactlyZero: lfExactlyHalf;
  अन्यथा
    प्रत्यागम digit_मानम् == 0 ? lfLessThanHalf: lf_अधिकम्_Than_Half;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
स्थिर नष्टम्_Fraction
नष्टम्_Fraction_Through_Truncation(ध्रुव APFloat_आधारः::integerPart *parts,
                              अचिह्नित पूर्णाङ्क part_गणनम्,
                              अचिह्नित पूर्णाङ्क द्वयङ्काः)
{
  अचिह्नित lsb = APInt::tcLSB(parts, part_गणनम्);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  यदि (द्वयङ्काः <= lsb)
    प्रत्यागम lfExactlyZero;
  यदि (द्वयङ्काः == lsb + 1)
    प्रत्यागम lfExactlyHalf;
  यदि (द्वयङ्काः <= part_गणनम् * APFloat_आधारः::integer_Part_विस्तारः &&
      APInt::tcExtractBit(parts, द्वयङ्काः - 1))
    प्रत्यागम lf_अधिकम्_Than_Half;

  प्रत्यागम lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
स्थिर नष्टम्_Fraction
सरणम्_दक्षिणः(APFloat_आधारः::integerPart *dst, अचिह्नित पूर्णाङ्क parts, अचिह्नित पूर्णाङ्क द्वयङ्काः)
{
  नष्टम्_Fraction नष्टम्_fraction = नष्टम्_Fraction_Through_Truncation(dst, parts, द्वयङ्काः);

  APInt::tc_सरणम्_दक्षिणः(dst, parts, द्वयङ्काः);

  प्रत्यागम नष्टम्_fraction;
}

/* Combine the effect of two lost fractions.  */
स्थिर नष्टम्_Fraction
combine_नष्टम्_Fractions(नष्टम्_Fraction अधिकम्_Significant,
                     नष्टम्_Fraction lessSignificant)
{
  यदि (lessSignificant != lfExactlyZero) {
    यदि (अधिकम्_Significant == lfExactlyZero)
      अधिकम्_Significant = lfLessThanHalf;
    अन्यथा यदि (अधिकम्_Significant == lfExactlyHalf)
      अधिकम्_Significant = lf_अधिकम्_Than_Half;
  }

  प्रत्यागम अधिकम्_Significant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
स्थिर अचिह्नित पूर्णाङ्क
HUerrBound(तार्किक inexactMultiply, अचिह्नित पूर्णाङ्क HUerr1, अचिह्नित पूर्णाङ्क HUerr2)
{
  प्रतिज्ञा(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  यदि (HUerr1 + HUerr2 == 0)
    प्रत्यागम inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  अन्यथा
    प्रत्यागम inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
स्थिर APFloat_आधारः::integerPart
ulpsFromBoundary(ध्रुव APFloat_आधारः::integerPart *parts, अचिह्नित पूर्णाङ्क द्वयङ्काः,
                 तार्किक is_निकटतमः) {
  प्रतिज्ञा(द्वयङ्काः != 0);

  द्वयङ्काः--;
  अचिह्नित गणनम् = द्वयङ्काः / APFloat_आधारः::integer_Part_विस्तारः;
  अचिह्नित part_द्वयङ्काः = द्वयङ्काः % APFloat_आधारः::integer_Part_विस्तारः + 1;

  APFloat_आधारः::integerPart part =
      parts[गणनम्] & (~(APFloat_आधारः::integerPart)0 >>
                      (APFloat_आधारः::integer_Part_विस्तारः - part_द्वयङ्काः));

  APFloat_आधारः::integerPart boundary;
  यदि (is_निकटतमः)
    boundary = (APFloat_आधारः::integerPart) 1 << (part_द्वयङ्काः - 1);
  अन्यथा
    boundary = 0;

  यदि (गणनम् == 0) {
    यदि (part - boundary <= boundary - part)
      प्रत्यागम part - boundary;
    अन्यथा
      प्रत्यागम boundary - part;
  }

  यदि (part == boundary) {
    यावत् (--गणनम्)
      यदि (parts[गणनम्])
        प्रत्यागम ~(APFloat_आधारः::integerPart) 0; /* A lot.  */

    प्रत्यागम parts[0];
  } अन्यथा यदि (part == boundary - 1) {
    यावत् (--गणनम्)
      यदि (~parts[गणनम्])
        प्रत्यागम ~(APFloat_आधारः::integerPart) 0; /* A lot.  */

    प्रत्यागम -parts[0];
  }

  प्रत्यागम ~(APFloat_आधारः::integerPart) 0; /* A lot.  */
}

स्थिर constexpr अचिह्नित Power_सम्बन्धि_Five_Part_Counts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

स्थिर constexpr APFloat_आधारः::integerPart Power_सम्बन्धि_Five_Parts[] = {
#अन्तर्भाव "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  अचिह्नित योग = 0;
  प्रति (अचिह्नित गणनम्_2 : Power_सम्बन्धि_Five_Part_Counts)
    योग += गणनम्_2;
  प्रत्यागम योग;
}() == std::आकार_2(Power_सम्बन्धि_Five_Parts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
स्थिर अचिह्नित पूर्णाङ्क
powerOf5(APFloat_आधारः::integerPart *dst, अचिह्नित पूर्णाङ्क power) {
  स्थिर ध्रुव APFloat_आधारः::integerPart firstEightPowers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  Small_सदिशम्<APFloat_आधारः::integerPart, अङ्कः_Pow5_Parts_On_राशिः> scratch;
  scratch.resize_for_overwrite(power_सम्बन्धि_Five_Parts_For_Exponent(power));

  APFloat_आधारः::integerPart *p1, *p2;
  ध्रुव APFloat_आधारः::integerPart *pow5;

  p1 = dst;
  p2 = scratch.दत्त();

  *p1 = firstEightPowers[power & 7];
  power >>= 3;

  अचिह्नित परिणाम_2 = 1;
  pow5 = Power_सम्बन्धि_Five_Parts;

  प्रति (अचिह्नित पूर्णाङ्क n = 0; power; power >>= 1, n++) {
    अचिह्नित parts_गणनम् = Power_सम्बन्धि_Five_Part_Counts[n];

    यदि (power & 1) {
      APFloat_आधारः::integerPart *tmp;

      APInt::tc_पूर्णम्_Multiply(p2, p1, pow5, परिणाम_2, parts_गणनम्);
      परिणाम_2 += parts_गणनम्;
      यदि (p2[परिणाम_2 - 1] == 0)
        परिणाम_2--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_गणनम्;
  }

  यदि (p1 != dst)
    APInt::tc_नियोजय(dst, p1, परिणाम_2);

  प्रत्यागम परिणाम_2;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
स्थिर ध्रुव वर्ण hex_Digits_अधस्तनम्[] = "0123456789abcdef0";
स्थिर ध्रुव वर्ण hex_Digits_ऊर्ध्वस्थम्[] = "0123456789ABCDEF0";
स्थिर ध्रुव वर्ण infinityL[] = "infinity";
स्थिर ध्रुव वर्ण infinityU[] = "INFINITY";
स्थिर ध्रुव वर्ण NaNL[] = "nan";
स्थिर ध्रुव वर्ण NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
स्थिर अचिह्नित पूर्णाङ्क
partAsHex (वर्ण *dst, APFloat_आधारः::integerPart part, अचिह्नित पूर्णाङ्क गणनम्,
           ध्रुव वर्ण *hexDigitChars)
{
  अचिह्नित पूर्णाङ्क परिणाम_2 = गणनम्;

  प्रतिज्ञा(गणनम् != 0 && गणनम् <= APFloat_आधारः::integer_Part_विस्तारः / 4);

  part >>= (APFloat_आधारः::integer_Part_विस्तारः - 4 * गणनम्);
  यावत् (गणनम्--) {
    dst[गणनम्] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  प्रत्यागम परिणाम_2;
}

/* Write out an unsigned decimal integer.  */
स्थिर वर्ण *writeUnsignedDecimal(वर्ण *dst, अचिह्नित पूर्णाङ्क n) {
  वर्ण buff[40], *p;

  p = buff;
  कुरु
    *p++ = '0' + n % 10;
  यावत् (n /= 10);

  कुरु
    *dst++ = *--p;
  यावत् (p != buff);

  प्रत्यागम dst;
}

/* Write out a signed decimal integer.  */
स्थिर वर्ण *writeSignedDecimal(वर्ण *dst, पूर्णाङ्क मानम्_2) {
  यदि (मानम्_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(अचिह्नित) मानम्_2);
  } अन्यथा {
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
  पक्ष APFloat::fc_प्रसामान्यम्:
    भङ्ग;
  }
  यदि (X.isDenormal() || X.isSmallestNormalized())
    प्रत्यागम APFloat::getSmallest(Sem);
  पूर्णाङ्क Exp = ilogb(X);
  यदि (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  प्रत्यागम scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rm_निकटतमः_Ties_गन्तव्यम्_Even);
}

नामाकाश detail {
/* Constructors.  */
शून्य IEEEFloat::initialize(ध्रुव fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  अचिह्नित गणनम् = part_गणनम्();
  यदि (गणनम् > 1)
    significand.parts = नूतन integerPart[गणनम्];
}

शून्य IEEEFloat::freeSignificand() {
  यदि (needsCleanup())
    विलोपय [] significand.parts;
}

शून्य IEEEFloat::नियोजय(ध्रुव IEEEFloat &rhs) {
  प्रतिज्ञा(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  यदि (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

शून्य IEEEFloat::copySignificand(ध्रुव IEEEFloat &rhs) {
  प्रतिज्ञा(isFiniteNonZero() || category == fcNaN);
  प्रतिज्ञा(rhs.part_गणनम्() >= part_गणनम्());

  APInt::tc_नियोजय(significandParts(), rhs.significandParts(),
                  part_गणनम्());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
शून्य IEEEFloat::makeNaN(तार्किक SNaN, तार्किक Negative, ध्रुव APInt *fill) {
  यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  यदि (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  अचिह्नित अङ्कः_Parts = part_गणनम्();

  APInt fill_storage;
  यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = असत्य;
    यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = सत्य;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } अन्यथा {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  यदि (!fill || fill->get_अङ्कः_शब्दाः() < अङ्कः_Parts)
    APInt::tc_समुच्चयः(significand, 0, अङ्कः_Parts);
  यदि (fill) {
    APInt::tc_नियोजय(significand, fill->get_Raw_दत्त(),
                    std::min(fill->get_अङ्कः_शब्दाः(), अङ्कः_Parts));

    // Zero out the excess bits of the significand.
    अचिह्नित द्वयङ्काः_गन्तव्यम्_Preserve = semantics->precision - 1;
    अचिह्नित part = द्वयङ्काः_गन्तव्यम्_Preserve / 64;
    द्वयङ्काः_गन्तव्यम्_Preserve %= 64;
    significand[part] &= ((1ULL << द्वयङ्काः_गन्तव्यम्_Preserve) - 1);
    प्रति (part++; part != अङ्कः_Parts; ++part)
      significand[part] = 0;
  }

  अचिह्नित QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  यदि (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tcClearBit(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    यदि (APInt::tcIsZero(significand, अङ्कः_Parts))
      APInt::tc_समुच्चयः_Bit(significand, QNaNBit - 1);
  } अन्यथा यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } अन्यथा {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tc_समुच्चयः_Bit(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  यदि (semantics == &APFloat_आधारः::semX87DoubleExtended)
    APInt::tc_समुच्चयः_Bit(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::संक्रियकः=(ध्रुव IEEEFloat &rhs) {
  यदि (एतत् != &rhs) {
    यदि (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    नियोजय(rhs);
  }

  प्रत्यागम *एतत्;
}

IEEEFloat &IEEEFloat::संक्रियकः=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloat_आधारः::semBogus;
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
  प्रत्यागम getCategory() == fc_प्रसामान्यम् && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

अचिह्नित पूर्णाङ्क IEEEFloat::get_अङ्कः_उच्चः_द्वयङ्काः() ध्रुव {
  ध्रुव अचिह्नित पूर्णाङ्क Part_गणनम् = part_गणनम्_For_द्वयङ्काः(semantics->precision);
  ध्रुव अचिह्नित पूर्णाङ्क द्वयङ्काः_2 = Part_गणनम् * integer_Part_विस्तारः;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  ध्रुव अचिह्नित पूर्णाङ्क अङ्कः_उच्चः_द्वयङ्काः = (semantics->precision > 1)
                                       ? (द्वयङ्काः_2 - semantics->precision + 1)
                                       : (द्वयङ्काः_2 - semantics->precision);
  प्रत्यागम अङ्कः_उच्चः_द्वयङ्काः;
}

तार्किक IEEEFloat::isSignificandAllOnes() ध्रुव {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  ध्रुव integerPart *Parts = significandParts();
  ध्रुव अचिह्नित Part_गणनम् = part_गणनम्_For_द्वयङ्काः(semantics->precision);
  प्रति (अचिह्नित i = 0; i < Part_गणनम् - 1; i++)
    यदि (~Parts[i])
      प्रत्यागम असत्य;

  // Set the unused high bits to all ones when we compare.
  ध्रुव अचिह्नित अङ्कः_उच्चः_द्वयङ्काः = get_अङ्कः_उच्चः_द्वयङ्काः();
  प्रतिज्ञा(अङ्कः_उच्चः_द्वयङ्काः <= integer_Part_विस्तारः && अङ्कः_उच्चः_द्वयङ्काः > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ध्रुव integerPart उच्चः_Bit_Fill =
    ~integerPart(0) << (integer_Part_विस्तारः - अङ्कः_उच्चः_द्वयङ्काः);
  यदि ((semantics->precision <= 1) || (~(Parts[Part_गणनम् - 1] | उच्चः_Bit_Fill)))
    प्रत्यागम असत्य;

  प्रत्यागम सत्य;
}

तार्किक IEEEFloat::isSignificandAllOnesExceptLSB() ध्रुव {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  ध्रुव integerPart *Parts = significandParts();

  यदि (Parts[0] & 1)
    प्रत्यागम असत्य;

  ध्रुव अचिह्नित Part_गणनम् = part_गणनम्_For_द्वयङ्काः(semantics->precision);
  प्रति (अचिह्नित i = 0; i < Part_गणनम् - 1; i++) {
    यदि (~Parts[i] & ~अचिह्नित{!i})
      प्रत्यागम असत्य;
  }

  // Set the unused high bits to all ones when we compare.
  ध्रुव अचिह्नित अङ्कः_उच्चः_द्वयङ्काः = get_अङ्कः_उच्चः_द्वयङ्काः();
  प्रतिज्ञा(अङ्कः_उच्चः_द्वयङ्काः <= integer_Part_विस्तारः && अङ्कः_उच्चः_द्वयङ्काः > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  ध्रुव integerPart उच्चः_Bit_Fill = ~integerPart(0)
                                  << (integer_Part_विस्तारः - अङ्कः_उच्चः_द्वयङ्काः);
  यदि (~(Parts[Part_गणनम् - 1] | उच्चः_Bit_Fill | 0x1))
    प्रत्यागम असत्य;

  प्रत्यागम सत्य;
}

तार्किक IEEEFloat::isSignificandAllZeros() ध्रुव {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  ध्रुव integerPart *Parts = significandParts();
  ध्रुव अचिह्नित Part_गणनम् = part_गणनम्_For_द्वयङ्काः(semantics->precision);

  प्रति (अचिह्नित i = 0; i < Part_गणनम् - 1; i++)
    यदि (Parts[i])
      प्रत्यागम असत्य;

  // Compute how many bits are used in the final word.
  ध्रुव अचिह्नित अङ्कः_उच्चः_द्वयङ्काः = get_अङ्कः_उच्चः_द्वयङ्काः();
  प्रतिज्ञा(अङ्कः_उच्चः_द्वयङ्काः < integer_Part_विस्तारः && "Can not have more high bits to "
         "clear than integerPartWidth");
  ध्रुव integerPart उच्चः_Bit_आवरणम् = ~integerPart(0) >> अङ्कः_उच्चः_द्वयङ्काः;

  यदि ((semantics->precision > 1) && (Parts[Part_गणनम् - 1] & उच्चः_Bit_आवरणम्))
    प्रत्यागम असत्य;

  प्रत्यागम सत्य;
}

तार्किक IEEEFloat::isSignificandAllZerosExceptMSB() ध्रुव {
  ध्रुव integerPart *Parts = significandParts();
  ध्रुव अचिह्नित Part_गणनम् = part_गणनम्_For_द्वयङ्काः(semantics->precision);

  प्रति (अचिह्नित i = 0; i < Part_गणनम् - 1; i++) {
    यदि (Parts[i])
      प्रत्यागम असत्य;
  }

  ध्रुव अचिह्नित अङ्कः_उच्चः_द्वयङ्काः = get_अङ्कः_उच्चः_द्वयङ्काः();
  ध्रुव integerPart MSBMask = integerPart(1)
                              << (integer_Part_विस्तारः - अङ्कः_उच्चः_द्वयङ्काः);
  प्रत्यागम ((semantics->precision <= 1) || (Parts[Part_गणनम् - 1] == MSBMask));
}

तार्किक IEEEFloat::isLargest() ध्रुव {
  तार्किक IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    प्रत्यागम (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } अन्यथा {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    प्रत्यागम IsMaxExp && isSignificandAllOnes();
  }
}

तार्किक IEEEFloat::isInteger() ध्रुव {
  // This could be made more efficient; I'm going for obviously correct.
  यदि (!isFinite()) प्रत्यागम असत्य;
  IEEEFloat truncated = *एतत्;
  truncated.आवर्तनम्_गन्तव्यम्_Integral(rmTowardZero);
  प्रत्यागम compare(truncated) == cmpEqual;
}

तार्किक IEEEFloat::bitwiseIsEqual(ध्रुव IEEEFloat &rhs) ध्रुव {
  यदि (एतत् == &rhs)
    प्रत्यागम सत्य;
  यदि (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    प्रत्यागम असत्य;
  यदि (category==fcZero || category==fcInfinity)
    प्रत्यागम सत्य;

  यदि (isFiniteNonZero() && exponent != rhs.exponent)
    प्रत्यागम असत्य;

  प्रत्यागम std::equal(significandParts(), significandParts() + part_गणनम्(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(ध्रुव fltSemantics &ourSemantics, integerPart मानम्_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fc_प्रसामान्यम्;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = मानम्_2;
  normalize(rm_निकटतमः_Ties_गन्तव्यम्_Even, lfExactlyZero);
}

IEEEFloat::IEEEFloat(ध्रुव fltSemantics &ourSemantics) {
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
IEEEFloat::IEEEFloat(ध्रुव fltSemantics &ourSemantics, uninitialized_चिह्नकम् चिह्नकम्_2)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(ध्रुव IEEEFloat &rhs) {
  initialize(rhs.semantics);
  नियोजय(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloat_आधारः::semBogus) {
  *एतत् = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

अचिह्नित पूर्णाङ्क IEEEFloat::part_गणनम्() ध्रुव {
  प्रत्यागम part_गणनम्_For_द्वयङ्काः(semantics->precision + 1);
}

ध्रुव APFloat::integerPart *IEEEFloat::significandParts() ध्रुव {
  प्रत्यागम const_cast<IEEEFloat *>(एतत्)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  यदि (part_गणनम्() > 1)
    प्रत्यागम significand.parts;
  अन्यथा
    प्रत्यागम &significand.part;
}

शून्य IEEEFloat::zeroSignificand() {
  APInt::tc_समुच्चयः(significandParts(), 0, part_गणनम्());
}

/* Increment an fcNormal floating point number's significand.  */
शून्य IEEEFloat::incrementSignificand() {
  [[maybe_अप्रयुक्तम्]] integerPart carry =
      APInt::tcIncrement(significandParts(), part_गणनम्());

  /* Our callers should never cause us to overflow.  */
  प्रतिज्ञा(carry == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::सङ्कलय_Significand(ध्रुव IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  प्रतिज्ञा(semantics == rhs.semantics);
  प्रतिज्ञा(exponent == rhs.exponent);

  प्रत्यागम APInt::tc_सङ्कलय(parts, rhs.significandParts(), 0, part_गणनम्());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::व्यवकलय_Significand(ध्रुव IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  प्रतिज्ञा(semantics == rhs.semantics);
  प्रतिज्ञा(exponent == rhs.exponent);

  प्रत्यागम APInt::tc_व्यवकलय(parts, rhs.significandParts(), borrow,
                           part_गणनम्());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
नष्टम्_Fraction IEEEFloat::multiplySignificand(ध्रुव IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            तार्किक ignoreAddend) {
  integerPart scratch[4];
  तार्किक ignored;

  प्रतिज्ञा(semantics == rhs.semantics);

  अचिह्नित precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  अचिह्नित new_Parts_गणनम् = part_गणनम्_For_द्वयङ्काः(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *पूर्णम्_Significand =
      new_Parts_गणनम् > 4 ? नूतन integerPart[new_Parts_गणनम्] : scratch;

  integerPart *lhsSignificand = significandParts();
  अचिह्नित parts_गणनम् = part_गणनम्();

  APInt::tc_पूर्णम्_Multiply(पूर्णम्_Significand, lhsSignificand,
                        rhs.significandParts(), parts_गणनम्, parts_गणनम्);

  नष्टम्_Fraction नष्टम्_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  अचिह्नित omsb = APInt::tcMSB(पूर्णम्_Significand, new_Parts_गणनम्) + 1;
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

  यदि (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    ध्रुव fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    अचिह्नित extendedPrecision = 2 * precision + 1;
    यदि (omsb != extendedPrecision - 1) {
      प्रतिज्ञा(extendedPrecision > omsb);
      APInt::tc_सरणम्_वामः(पूर्णम्_Significand, new_Parts_गणनम्,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    यदि (new_Parts_गणनम् == 1)
      significand.part = पूर्णम्_Significand[0];
    अन्यथा
      significand.parts = पूर्णम्_Significand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_अप्रयुक्तम्]] op_स्थितिः स्थितिः_2 = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    प्रतिज्ञा(स्थितिः_2 == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    नष्टम्_fraction = extendedAddend.सरणम्_Significand_दक्षिणः(1);
    प्रतिज्ञा(नष्टम्_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    नष्टम्_fraction = सङ्कलय_Or_व्यवकलय_Significand(extendedAddend, असत्य);

    /* Restore our state.  */
    यदि (new_Parts_गणनम् == 1)
      पूर्णम्_Significand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(पूर्णम्_Significand, new_Parts_गणनम्) + 1;
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
  यदि (omsb > precision) {
    अचिह्नित पूर्णाङ्क द्वयङ्काः, significantParts;
    नष्टम्_Fraction lf;

    द्वयङ्काः = omsb - precision;
    significantParts = part_गणनम्_For_द्वयङ्काः(omsb);
    lf = सरणम्_दक्षिणः(पूर्णम्_Significand, significantParts, द्वयङ्काः);
    नष्टम्_fraction = combine_नष्टम्_Fractions(lf, नष्टम्_fraction);
    exponent += द्वयङ्काः;
  }

  APInt::tc_नियोजय(lhsSignificand, पूर्णम्_Significand, parts_गणनम्);

  यदि (new_Parts_गणनम् > 4)
    विलोपय [] पूर्णम्_Significand;

  प्रत्यागम नष्टम्_fraction;
}

नष्टम्_Fraction IEEEFloat::multiplySignificand(ध्रुव IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  प्रत्यागम multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
नष्टम्_Fraction IEEEFloat::divideSignificand(ध्रुव IEEEFloat &rhs) {
  integerPart scratch[4];

  प्रतिज्ञा(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  ध्रुव integerPart *rhsSignificand = rhs.significandParts();
  अचिह्नित parts_गणनम् = part_गणनम्();

  integerPart *dividend =
      parts_गणनम् > 2 ? नूतन integerPart[parts_गणनम् * 2] : scratch;
  integerPart *भाजकः = dividend + parts_गणनम्;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  प्रति (अचिह्नित i = 0; i < parts_गणनम्; i++) {
    dividend[i] = lhsSignificand[i];
    भाजकः[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  अचिह्नित पूर्णाङ्क precision = semantics->precision;

  /* Normalize the divisor.  */
  अचिह्नित bit = precision - APInt::tcMSB(भाजकः, parts_गणनम्) - 1;
  यदि (bit) {
    exponent += bit;
    APInt::tc_सरणम्_वामः(भाजकः, parts_गणनम्, bit);
  }

  /* Normalize the dividend.  */
  bit = precision - APInt::tcMSB(dividend, parts_गणनम्) - 1;
  यदि (bit) {
    exponent -= bit;
    APInt::tc_सरणम्_वामः(dividend, parts_गणनम्, bit);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  यदि (APInt::tcCompare(dividend, भाजकः, parts_गणनम्) < 0) {
    exponent--;
    APInt::tc_सरणम्_वामः(dividend, parts_गणनम्, 1);
    प्रतिज्ञा(APInt::tcCompare(dividend, भाजकः, parts_गणनम्) >= 0);
  }

  /* Long division.  */
  प्रति (bit = precision; bit; bit -= 1) {
    यदि (APInt::tcCompare(dividend, भाजकः, parts_गणनम्) >= 0) {
      APInt::tc_व्यवकलय(dividend, भाजकः, 0, parts_गणनम्);
      APInt::tc_समुच्चयः_Bit(lhsSignificand, bit - 1);
    }

    APInt::tc_सरणम्_वामः(dividend, parts_गणनम्, 1);
  }

  /* Figure out the lost fraction.  */
  पूर्णाङ्क cmp = APInt::tcCompare(dividend, भाजकः, parts_गणनम्);

  नष्टम्_Fraction नष्टम्_fraction;
  यदि (cmp > 0)
    नष्टम्_fraction = lf_अधिकम्_Than_Half;
  अन्यथा यदि (cmp == 0)
    नष्टम्_fraction = lfExactlyHalf;
  अन्यथा यदि (APInt::tcIsZero(dividend, parts_गणनम्))
    नष्टम्_fraction = lfExactlyZero;
  अन्यथा
    नष्टम्_fraction = lfLessThanHalf;

  यदि (parts_गणनम् > 2)
    विलोपय [] dividend;

  प्रत्यागम नष्टम्_fraction;
}

अचिह्नित पूर्णाङ्क IEEEFloat::significandMSB() ध्रुव {
  प्रत्यागम APInt::tcMSB(significandParts(), part_गणनम्());
}

अचिह्नित पूर्णाङ्क IEEEFloat::significandLSB() ध्रुव {
  प्रत्यागम APInt::tcLSB(significandParts(), part_गणनम्());
}

/* Note that a zero result is NOT normalized to fcZero.  */
नष्टम्_Fraction IEEEFloat::सरणम्_Significand_दक्षिणः(अचिह्नित पूर्णाङ्क द्वयङ्काः) {
  /* Our exponent should not overflow.  */
  प्रतिज्ञा((ExponentType) (exponent + द्वयङ्काः) >= exponent);

  exponent += द्वयङ्काः;

  प्रत्यागम सरणम्_दक्षिणः(significandParts(), part_गणनम्(), द्वयङ्काः);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
शून्य IEEEFloat::सरणम्_Significand_वामः(अचिह्नित पूर्णाङ्क द्वयङ्काः) {
  प्रतिज्ञा(द्वयङ्काः < semantics->precision ||
         (semantics->precision == 1 && द्वयङ्काः <= 1));

  यदि (द्वयङ्काः) {
    अचिह्नित पूर्णाङ्क parts_गणनम् = part_गणनम्();

    APInt::tc_सरणम्_वामः(significandParts(), parts_गणनम्, द्वयङ्काः);
    exponent -= द्वयङ्काः;

    प्रतिज्ञा(!APInt::tcIsZero(significandParts(), parts_गणनम्));
  }
}

APFloat::cmp_परिणाम IEEEFloat::compare_Absolute_मानम्(ध्रुव IEEEFloat &rhs) ध्रुव {
  प्रतिज्ञा(semantics == rhs.semantics);
  प्रतिज्ञा(isFiniteNonZero());
  प्रतिज्ञा(rhs.isFiniteNonZero());

  पूर्णाङ्क compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  यदि (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_गणनम्());

  यदि (compare > 0)
    प्रत्यागम cmpGreaterThan;
  अन्यथा यदि (compare < 0)
    प्रत्यागम cmpLessThan;
  अन्यथा
    प्रत्यागम cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
स्थिर शून्य tc_समुच्चयः_न्यूनतमम्_Significant_द्वयङ्काः(APInt::WordType *dst, अचिह्नित parts,
                                      अचिह्नित द्वयङ्काः) {
  अचिह्नित i = 0;
  यावत् (द्वयङ्काः > APInt::APINT_द्वयङ्काः_प्रत्येकम्_WORD) {
    dst[i++] = ~(APInt::WordType)0;
    द्वयङ्काः -= APInt::APINT_द्वयङ्काः_प्रत्येकम्_WORD;
  }

  यदि (द्वयङ्काः)
    dst[i++] = ~(APInt::WordType)0 >> (APInt::APINT_द्वयङ्काः_प्रत्येकम्_WORD - द्वयङ्काः);

  यावत् (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::op_स्थितिः IEEEFloat::handle_अतिप्रवाहः(rounding_बहुलकम् rounding_बहुलकम्_2) {
  यदि (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    यदि (rounding_बहुलकम्_2 == rm_निकटतमः_Ties_गन्तव्यम्_Even ||
        rounding_बहुलकम्_2 == rm_निकटतमः_Ties_गन्तव्यम्_Away ||
        (rounding_बहुलकम्_2 == rm_Toward_धनात्मकम् && !sign) ||
        (rounding_बहुलकम्_2 == rmTowardNegative && sign)) {
      यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(असत्य, sign);
      अन्यथा
        category = fcInfinity;
      प्रत्यागम static_cast<op_स्थितिः>(op_अतिप्रवाहः | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fc_प्रसामान्यम्;
  exponent = semantics->maxExponent;
  tc_समुच्चयः_न्यूनतमम्_Significant_द्वयङ्काः(significandParts(), part_गणनम्(),
                            semantics->precision);
  यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tcClearBit(significandParts(), 0);

  प्रत्यागम opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
तार्किक IEEEFloat::आवर्तनम्_Away_From_Zero(rounding_बहुलकम् rounding_बहुलकम्_2,
                                  नष्टम्_Fraction नष्टम्_fraction,
                                  अचिह्नित पूर्णाङ्क bit) ध्रुव {
  /* NaNs and infinities should not have lost fractions.  */
  प्रतिज्ञा(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  प्रतिज्ञा(नष्टम्_fraction != lfExactlyZero);

  विकल्प (rounding_बहुलकम्_2) {
  पक्ष rm_निकटतमः_Ties_गन्तव्यम्_Away:
    प्रत्यागम नष्टम्_fraction == lfExactlyHalf || नष्टम्_fraction == lf_अधिकम्_Than_Half;

  पक्ष rm_निकटतमः_Ties_गन्तव्यम्_Even:
    यदि (नष्टम्_fraction == lf_अधिकम्_Than_Half)
      प्रत्यागम सत्य;

    /* Our zeroes don't have a significand to test.  */
    यदि (नष्टम्_fraction == lfExactlyHalf && category != fcZero)
      प्रत्यागम APInt::tcExtractBit(significandParts(), bit);

    प्रत्यागम असत्य;

  पक्ष rmTowardZero:
    प्रत्यागम असत्य;

  पक्ष rm_Toward_धनात्मकम्:
    प्रत्यागम !sign;

  पक्ष rmTowardNegative:
    प्रत्यागम sign;

  स्वभाव:
    भङ्ग;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::op_स्थितिः IEEEFloat::normalize(rounding_बहुलकम् rounding_बहुलकम्_2,
                                       नष्टम्_Fraction नष्टम्_fraction) {
  यदि (!isFiniteNonZero())
    प्रत्यागम opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  अचिह्नित omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  यदि (omsb || नष्टम्_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    पूर्णाङ्क exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    यदि (exponent + exponentChange > semantics->maxExponent)
      प्रत्यागम handle_अतिप्रवाहः(rounding_बहुलकम्_2);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    यदि (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    यदि (exponentChange < 0) {
      प्रतिज्ञा(नष्टम्_fraction == lfExactlyZero);

      सरणम्_Significand_वामः(-exponentChange);

      प्रत्यागम opOK;
    }

    यदि (exponentChange > 0) {
      नष्टम्_Fraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = सरणम्_Significand_दक्षिणः(exponentChange);

      नष्टम्_fraction = combine_नष्टम्_Fractions(lf, नष्टम्_fraction);

      /* Keep OMSB up-to-date.  */
      यदि (omsb > (अचिह्नित) exponentChange)
        omsb -= exponentChange;
      अन्यथा
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    प्रत्यागम handle_अतिप्रवाहः(rounding_बहुलकम्_2);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  यदि (नष्टम्_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    यदि (omsb == 0) {
      category = fcZero;
      यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = असत्य;
      यदि (!semantics->hasZero)
        makeSmallestNormalized(असत्य);
    }

    प्रत्यागम opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  यदि (आवर्तनम्_Away_From_Zero(rounding_बहुलकम्_2, नष्टम्_fraction, 0)) {
    यदि (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    यदि (omsb == (अचिह्नित) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      यदि (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        प्रत्यागम handle_अतिप्रवाहः(sign ? rmTowardNegative : rm_Toward_धनात्मकम्);

      सरणम्_Significand_दक्षिणः(1);

      प्रत्यागम opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      प्रत्यागम handle_अतिप्रवाहः(rounding_बहुलकम्_2);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  यदि (omsb == semantics->precision)
    प्रत्यागम opInexact;

  /* We have a non-zero denormal.  */
  प्रतिज्ञा(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  यदि (omsb == 0) {
    category = fcZero;
    यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = असत्य;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    यदि (!semantics->hasZero)
      makeSmallestNormalized(असत्य);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  प्रत्यागम (op_स्थितिः) (opUnderflow | opInexact);
}

APFloat::op_स्थितिः IEEEFloat::सङ्कलय_Or_व्यवकलय_Specials(ध्रुव IEEEFloat &rhs,
                                                   तार्किक व्यवकलय) {
  विकल्प (Pack_Categories_Into_कुञ्चिका(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(शून्यसूचकः);

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNaN):
    नियोजय(rhs);
    [[fallthrough]];
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fc_प्रसामान्यम्):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNaN):
    यदि (isSignaling()) {
      makeQuiet();
      प्रत्यागम opInvalidOp;
    }
    प्रत्यागम rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fc_प्रसामान्यम्):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ व्यवकलय;
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fc_प्रसामान्यम्):
    नियोजय(rhs);
    sign = rhs.sign ^ व्यवकलय;
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    यदि (((sign ^ rhs.sign)!=0) != व्यवकलय) {
      makeNaN();
      प्रत्यागम opInvalidOp;
    }

    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fc_प्रसामान्यम्):
    प्रत्यागम opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
नष्टम्_Fraction IEEEFloat::सङ्कलय_Or_व्यवकलय_Significand(ध्रुव IEEEFloat &rhs,
                                                 तार्किक व्यवकलय) {
  [[maybe_अप्रयुक्तम्]] integerPart carry = 0;
  नष्टम्_Fraction नष्टम्_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  व्यवकलय ^= static_cast<तार्किक>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  पूर्णाङ्क द्वयङ्काः = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  यदि (व्यवकलय) {
    यदि ((द्वयङ्काः < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat अस्थायी_rhs(rhs);
    तार्किक नष्टम्_fraction_is_from_rhs = असत्य;

    यदि (द्वयङ्काः == 0)
      नष्टम्_fraction = lfExactlyZero;
    अन्यथा यदि (द्वयङ्काः > 0) {
      नष्टम्_fraction = अस्थायी_rhs.सरणम्_Significand_दक्षिणः(द्वयङ्काः - 1);
      नष्टम्_fraction_is_from_rhs = सत्य;
      सरणम्_Significand_वामः(1);
    } अन्यथा {
      नष्टम्_fraction = सरणम्_Significand_दक्षिणः(-द्वयङ्काः - 1);
      अस्थायी_rhs.सरणम्_Significand_वामः(1);
    }

    // Should we reverse the subtraction.
    cmp_परिणाम cmp_परिणाम_2 = compare_Absolute_मानम्(अस्थायी_rhs);
    यदि (cmp_परिणाम_2 == cmpLessThan) {
      तार्किक borrow =
          नष्टम्_fraction != lfExactlyZero && !नष्टम्_fraction_is_from_rhs;
      यदि (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        यदि (नष्टम्_fraction == lfLessThanHalf)
          नष्टम्_fraction = lf_अधिकम्_Than_Half;
        अन्यथा यदि (नष्टम्_fraction == lf_अधिकम्_Than_Half)
          नष्टम्_fraction = lfLessThanHalf;
      }
      carry = अस्थायी_rhs.व्यवकलय_Significand(*एतत्, borrow);
      copySignificand(अस्थायी_rhs);
      sign = !sign;
    } अन्यथा यदि (cmp_परिणाम_2 == cmpGreaterThan) {
      तार्किक borrow = नष्टम्_fraction != lfExactlyZero && नष्टम्_fraction_is_from_rhs;
      यदि (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        यदि (नष्टम्_fraction == lfLessThanHalf)
          नष्टम्_fraction = lf_अधिकम्_Than_Half;
        अन्यथा यदि (नष्टम्_fraction == lf_अधिकम्_Than_Half)
          नष्टम्_fraction = lfLessThanHalf;
      }
      carry = व्यवकलय_Significand(अस्थायी_rhs, borrow);
    } अन्यथा { // cmpEqual
      zeroSignificand();
      यदि (नष्टम्_fraction != lfExactlyZero && नष्टम्_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    प्रतिज्ञा(!carry);
  } अन्यथा {
    यदि (द्वयङ्काः > 0) {
      IEEEFloat अस्थायी_rhs(rhs);

      नष्टम्_fraction = अस्थायी_rhs.सरणम्_Significand_दक्षिणः(द्वयङ्काः);
      carry = सङ्कलय_Significand(अस्थायी_rhs);
    } अन्यथा {
      नष्टम्_fraction = सरणम्_Significand_दक्षिणः(-द्वयङ्काः);
      carry = सङ्कलय_Significand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    प्रतिज्ञा(!carry);
  }

  प्रत्यागम नष्टम्_fraction;
}

APFloat::op_स्थितिः IEEEFloat::multiplySpecials(ध्रुव IEEEFloat &rhs) {
  विकल्प (Pack_Categories_Into_कुञ्चिका(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(शून्यसूचकः);

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNaN):
    नियोजय(rhs);
    sign = असत्य;
    [[fallthrough]];
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fc_प्रसामान्यम्):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    यदि (isSignaling()) {
      makeQuiet();
      प्रत्यागम opInvalidOp;
    }
    प्रत्यागम rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fc_प्रसामान्यम्):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
    category = fcInfinity;
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fc_प्रसामान्यम्):
  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    category = fcZero;
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
    makeNaN();
    प्रत्यागम opInvalidOp;

  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fc_प्रसामान्यम्):
    प्रत्यागम opOK;
  }
}

APFloat::op_स्थितिः IEEEFloat::divideSpecials(ध्रुव IEEEFloat &rhs) {
  विकल्प (Pack_Categories_Into_कुञ्चिका(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(शून्यसूचकः);

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNaN):
    नियोजय(rhs);
    sign = असत्य;
    [[fallthrough]];
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fc_प्रसामान्यम्):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    यदि (isSignaling()) {
      makeQuiet();
      प्रत्यागम opInvalidOp;
    }
    प्रत्यागम rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fc_प्रसामान्यम्):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fc_प्रसामान्यम्):
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcInfinity):
    category = fcZero;
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcZero):
    यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(असत्य, sign);
    अन्यथा
      category = fcInfinity;
    प्रत्यागम opDivByZero;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    makeNaN();
    प्रत्यागम opInvalidOp;

  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fc_प्रसामान्यम्):
    प्रत्यागम opOK;
  }
}

APFloat::op_स्थितिः IEEEFloat::modSpecials(ध्रुव IEEEFloat &rhs) {
  विकल्प (Pack_Categories_Into_कुञ्चिका(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(शून्यसूचकः);

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNaN):
    नियोजय(rhs);
    [[fallthrough]];
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fc_प्रसामान्यम्):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNaN):
    यदि (isSignaling()) {
      makeQuiet();
      प्रत्यागम opInvalidOp;
    }
    प्रत्यागम rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fc_प्रसामान्यम्):
  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcInfinity):
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fc_प्रसामान्यम्):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    makeNaN();
    प्रत्यागम opInvalidOp;

  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fc_प्रसामान्यम्):
    प्रत्यागम opOK;
  }
}

APFloat::op_स्थितिः IEEEFloat::remainderSpecials(ध्रुव IEEEFloat &rhs) {
  विकल्प (Pack_Categories_Into_कुञ्चिका(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(शून्यसूचकः);

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNaN):
    नियोजय(rhs);
    [[fallthrough]];
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fc_प्रसामान्यम्):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNaN):
    यदि (isSignaling()) {
      makeQuiet();
      प्रत्यागम opInvalidOp;
    }
    प्रत्यागम rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fc_प्रसामान्यम्):
  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcInfinity):
    प्रत्यागम opOK;

  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fc_प्रसामान्यम्):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    makeNaN();
    प्रत्यागम opInvalidOp;

  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fc_प्रसामान्यम्):
    प्रत्यागम opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
शून्य IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    प्रत्यागम;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::op_स्थितिः IEEEFloat::सङ्कलय_Or_व्यवकलय(ध्रुव IEEEFloat &rhs,
                                           rounding_बहुलकम् rounding_बहुलकम्_2,
                                           तार्किक व्यवकलय) {
  op_स्थितिः fs = सङ्कलय_Or_व्यवकलय_Specials(rhs, व्यवकलय);

  /* This return code means it was not a simple case.  */
  यदि (fs == opDivByZero) {
    नष्टम्_Fraction नष्टम्_fraction;

    नष्टम्_fraction = सङ्कलय_Or_व्यवकलय_Significand(rhs, व्यवकलय);
    fs = normalize(rounding_बहुलकम्_2, नष्टम्_fraction);

    /* Can only be zero if we lost no fraction.  */
    प्रतिज्ञा(category != fcZero || नष्टम्_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  यदि (category == fcZero) {
    यदि (rhs.category != fcZero || (sign == rhs.sign) == व्यवकलय)
      sign = (rounding_बहुलकम्_2 == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = असत्य;
  }

  प्रत्यागम fs;
}

/* Normalized addition.  */
APFloat::op_स्थितिः IEEEFloat::सङ्कलय(ध्रुव IEEEFloat &rhs,
                                 rounding_बहुलकम् rounding_बहुलकम्_2) {
  प्रत्यागम सङ्कलय_Or_व्यवकलय(rhs, rounding_बहुलकम्_2, असत्य);
}

/* Normalized subtraction.  */
APFloat::op_स्थितिः IEEEFloat::व्यवकलय(ध्रुव IEEEFloat &rhs,
                                      rounding_बहुलकम् rounding_बहुलकम्_2) {
  प्रत्यागम सङ्कलय_Or_व्यवकलय(rhs, rounding_बहुलकम्_2, सत्य);
}

/* Normalized multiply.  */
APFloat::op_स्थितिः IEEEFloat::multiply(ध्रुव IEEEFloat &rhs,
                                      rounding_बहुलकम् rounding_बहुलकम्_2) {
  sign ^= rhs.sign;
  op_स्थितिः fs = multiplySpecials(rhs);

  यदि (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = असत्य;
  यदि (isFiniteNonZero()) {
    नष्टम्_Fraction नष्टम्_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_बहुलकम्_2, नष्टम्_fraction);
    यदि (नष्टम्_fraction != lfExactlyZero)
      fs = (op_स्थितिः) (fs | opInexact);
  }

  प्रत्यागम fs;
}

/* Normalized divide.  */
APFloat::op_स्थितिः IEEEFloat::divide(ध्रुव IEEEFloat &rhs,
                                    rounding_बहुलकम् rounding_बहुलकम्_2) {
  sign ^= rhs.sign;
  op_स्थितिः fs = divideSpecials(rhs);

  यदि (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = असत्य;
  यदि (isFiniteNonZero()) {
    नष्टम्_Fraction नष्टम्_fraction = divideSignificand(rhs);
    fs = normalize(rounding_बहुलकम्_2, नष्टम्_fraction);
    यदि (नष्टम्_fraction != lfExactlyZero)
      fs = (op_स्थितिः) (fs | opInexact);
  }

  प्रत्यागम fs;
}

/* Normalized remainder.  */
APFloat::op_स्थितिः IEEEFloat::remainder(ध्रुव IEEEFloat &rhs) {
  अचिह्नित पूर्णाङ्क origSign = sign;

  // First handle the special cases.
  op_स्थितिः fs = remainderSpecials(rhs);
  यदि (fs != opDivByZero)
    प्रत्यागम fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  यदि (P2.सङ्कलय(rhs, rm_निकटतमः_Ties_गन्तव्यम्_Even) == opOK) {
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

  IEEEFloat VEx = *एतत्;
  fs = VEx.convert(extendedSemantics, rm_निकटतमः_Ties_गन्तव्यम्_Even, &losesInfo);
  प्रतिज्ञा(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rm_निकटतमः_Ties_गन्तव्यम्_Even, &losesInfo);
  प्रतिज्ञा(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.सङ्कलय(VEx, rm_निकटतमः_Ties_गन्तव्यम्_Even);
  प्रतिज्ञा(fs == opOK);

  यदि (VEx.compare(PEx) == cmpGreaterThan) {
    fs = व्यवकलय(P, rm_निकटतमः_Ties_गन्तव्यम्_Even);
    प्रतिज्ञा(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.व्यवकलय(PEx, rm_निकटतमः_Ties_गन्तव्यम्_Even);
    प्रतिज्ञा(fs == opOK);
    fs = VEx.व्यवकलय(PEx, rm_निकटतमः_Ties_गन्तव्यम्_Even);
    प्रतिज्ञा(fs == opOK);

    cmp_परिणाम परिणाम_2 = VEx.compare(PEx);
    यदि (परिणाम_2 == cmpGreaterThan || परिणाम_2 == cmpEqual) {
      fs = व्यवकलय(P, rm_निकटतमः_Ties_गन्तव्यम्_Even);
      प्रतिज्ञा(fs == opOK);
    }
  }

  यदि (isZero()) {
    sign = origSign;    // IEEE754 requires this
    यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = असत्य;
  } अन्यथा {
    sign ^= origSign;
  }
  प्रत्यागम fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::op_स्थितिः IEEEFloat::mod(ध्रुव IEEEFloat &rhs) {
  op_स्थितिः fs = modSpecials(rhs);
  अचिह्नित पूर्णाङ्क origSign = sign;

  यावत् (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_मानम्(rhs) != cmpLessThan) {
    पूर्णाङ्क Exp = ilogb(*एतत्) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rm_निकटतमः_Ties_गन्तव्यम्_Even);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    यदि (V.isNaN() || compare_Absolute_मानम्(V) == cmpLessThan)
      V = scalbn(rhs, Exp - 1, rm_निकटतमः_Ties_गन्तव्यम्_Even);
    V.sign = sign;

    fs = व्यवकलय(V, rm_निकटतमः_Ties_गन्तव्यम्_Even);

    // When the semantics supports zero, this loop's
    // exit-condition is handled by the 'isFiniteNonZero'
    // category check above. However, when the semantics
    // does not have 'fcZero' and we have reached the
    // minimum possible value, (and any further subtract
    // will underflow to the same value) explicitly
    // provide an exit-path here.
    यदि (!semantics->hasZero && एतत्->isSmallest())
      भङ्ग;

    प्रतिज्ञा(fs==opOK);
  }
  यदि (isZero()) {
    sign = origSign; // fmod requires this
    यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = असत्य;
  }
  प्रत्यागम fs;
}

/* Normalized fused-multiply-add.  */
APFloat::op_स्थितिः IEEEFloat::fused_Multiply_सङ्कलय(ध्रुव IEEEFloat &multiplicand,
                                              ध्रुव IEEEFloat &addend,
                                              rounding_बहुलकम् rounding_बहुलकम्_2) {
  op_स्थितिः fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  यदि (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    नष्टम्_Fraction नष्टम्_fraction;

    नष्टम्_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_बहुलकम्_2, नष्टम्_fraction);
    यदि (नष्टम्_fraction != lfExactlyZero)
      fs = (op_स्थितिः) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    यदि (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_बहुलकम्_2 == rmTowardNegative);
      यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero)
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
    यदि (fs == opOK)
      fs = सङ्कलय_Or_व्यवकलय(addend, rounding_बहुलकम्_2, असत्य);
  }

  प्रत्यागम fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::op_स्थितिः IEEEFloat::आवर्तनम्_गन्तव्यम्_Integral(rounding_बहुलकम् rounding_बहुलकम्_2) {
  यदि (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    प्रत्यागम opOK;

  यदि (isNaN()) {
    यदि (isSignaling()) {
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
      प्रत्यागम opOK;
    }
  }

  यदि (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    प्रत्यागम opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  यदि (exponent + 1 >= (पूर्णाङ्क)APFloat::semanticsPrecision(*semantics))
    प्रत्यागम opOK;

  // The algorithm here is quite simple: we add 2^(p-1), where p is the
  // precision of our format, and then subtract it back off again.  The choice
  // of rounding modes for the addition/subtraction determines the rounding mode
  // for our integral rounding as well.
  // NOTE: When the input value is negative, we do subtraction followed by
  // addition instead.
  APInt IntegerConstant(अग्र_Power_Of2(APFloat::semanticsPrecision(*semantics)),
                        1);
  IntegerConstant <<= APFloat::semanticsPrecision(*semantics) - 1;
  IEEEFloat MagicConstant(*semantics);
  op_स्थितिः fs = MagicConstant.convertFromAPInt(IntegerConstant, असत्य,
                                               rm_निकटतमः_Ties_गन्तव्यम्_Even);
  प्रतिज्ञा(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  तार्किक निवेशः_Sign = isNegative();

  fs = सङ्कलय(MagicConstant, rounding_बहुलकम्_2);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  व्यवकलय(MagicConstant, rounding_बहुलकम्_2);

  // Restore the input sign.
  यदि (निवेशः_Sign != isNegative())
    changeSign();

  प्रत्यागम fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmp_परिणाम IEEEFloat::compare(ध्रुव IEEEFloat &rhs) ध्रुव {
  प्रतिज्ञा(semantics == rhs.semantics);

  विकल्प (Pack_Categories_Into_कुञ्चिका(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(शून्यसूचकः);

  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fc_प्रसामान्यम्):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcNaN, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcNaN):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcNaN):
    प्रत्यागम cmpUnordered;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fc_प्रसामान्यम्):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcZero):
  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcZero):
    यदि (sign)
      प्रत्यागम cmpLessThan;
    अन्यथा
      प्रत्यागम cmpGreaterThan;

  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fc_प्रसामान्यम्):
    यदि (rhs.sign)
      प्रत्यागम cmpGreaterThan;
    अन्यथा
      प्रत्यागम cmpLessThan;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcInfinity, fcInfinity):
    यदि (sign == rhs.sign)
      प्रत्यागम cmpEqual;
    अन्यथा यदि (sign)
      प्रत्यागम cmpLessThan;
    अन्यथा
      प्रत्यागम cmpGreaterThan;

  पक्ष Pack_Categories_Into_कुञ्चिका(fcZero, fcZero):
    प्रत्यागम cmpEqual;

  पक्ष Pack_Categories_Into_कुञ्चिका(fc_प्रसामान्यम्, fc_प्रसामान्यम्):
    भङ्ग;
  }

  cmp_परिणाम परिणाम_2;
  /* Two normal numbers.  Do they have the same sign?  */
  यदि (sign != rhs.sign) {
    यदि (sign)
      परिणाम_2 = cmpLessThan;
    अन्यथा
      परिणाम_2 = cmpGreaterThan;
  } अन्यथा {
    /* Compare absolute values; invert result if negative.  */
    परिणाम_2 = compare_Absolute_मानम्(rhs);

    यदि (sign) {
      यदि (परिणाम_2 == cmpLessThan)
        परिणाम_2 = cmpGreaterThan;
      अन्यथा यदि (परिणाम_2 == cmpGreaterThan)
        परिणाम_2 = cmpLessThan;
    }
  }

  प्रत्यागम परिणाम_2;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::op_स्थितिः IEEEFloat::convert(ध्रुव fltSemantics &गन्तव्यम्_Semantics,
                                     rounding_बहुलकम् rounding_बहुलकम्_2,
                                     तार्किक *losesInfo) {
  op_स्थितिः fs;
  ध्रुव fltSemantics &fromSemantics = *semantics;
  तार्किक is_signaling = isSignaling();

  नष्टम्_Fraction नष्टम्_Fraction = lfExactlyZero;
  अचिह्नित new_Part_गणनम् = part_गणनम्_For_द्वयङ्काः(गन्तव्यम्_Semantics.precision + 1);
  अचिह्नित old_Part_गणनम् = part_गणनम्();
  पूर्णाङ्क सरणम् = गन्तव्यम्_Semantics.precision - fromSemantics.precision;

  तार्किक X86SpecialNan = असत्य;
  यदि (&fromSemantics == &APFloat_आधारः::semX87DoubleExtended &&
      &गन्तव्यम्_Semantics != &APFloat_आधारः::semX87DoubleExtended && category == fcNaN &&
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
  यदि (सरणम् < 0 && isFiniteNonZero()) {
    पूर्णाङ्क omsb = significandMSB() + 1;
    पूर्णाङ्क exponentChange = omsb - fromSemantics.precision;
    यदि (exponent + exponentChange < गन्तव्यम्_Semantics.minExponent)
      exponentChange = गन्तव्यम्_Semantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, सरणम्);
    यदि (exponentChange < 0) {
      सरणम् -= exponentChange;
      exponent += exponentChange;
    } अन्यथा यदि (omsb <= -सरणम्) {
      exponentChange = omsb + सरणम् - 1; // leave at least one bit set
      सरणम् -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  यदि (सरणम् < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    नष्टम्_Fraction = सरणम्_दक्षिणः(significandParts(), old_Part_गणनम्, -सरणम्);

  // Fix the storage so it can hold to new value.
  यदि (new_Part_गणनम् > old_Part_गणनम्) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = नूतन integerPart[new_Part_गणनम्];
    APInt::tc_समुच्चयः(newParts, 0, new_Part_गणनम्);
    यदि (isFiniteNonZero() || category==fcNaN)
      APInt::tc_नियोजय(newParts, significandParts(), old_Part_गणनम्);
    freeSignificand();
    significand.parts = newParts;
  } अन्यथा यदि (new_Part_गणनम् == 1 && old_Part_गणनम् != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    यदि (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &गन्तव्यम्_Semantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  यदि (सरणम् > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_सरणम्_वामः(significandParts(), new_Part_गणनम्, सरणम्);

  यदि (isFiniteNonZero()) {
    fs = normalize(rounding_बहुलकम्_2, नष्टम्_Fraction);
    *losesInfo = (fs != opOK);
  } अन्यथा यदि (category == fcNaN) {
    यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(असत्य, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } अन्यथा {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      यदि (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(असत्य, असत्य);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      यदि (!APFloat::hasSignificand(fromSemantics))
        makeNaN(असत्य, sign);

      *losesInfo = नष्टम्_Fraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      यदि (!X86SpecialNan && semantics == &APFloat_आधारः::semX87DoubleExtended)
        APInt::tc_समुच्चयः_Bit(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      यदि (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } अन्यथा {
        fs = opOK;
      }
    }
  } अन्यथा यदि (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(असत्य, sign);
    *losesInfo = सत्य;
    fs = opInexact;
  } अन्यथा यदि (category == fcZero &&
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
  यदि ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = सत्य;
    यदि (fs == opOK)
      fs = opInexact;
  }

  यदि (category == fcZero && !semantics->hasZero)
    makeSmallestNormalized(असत्य);
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
APFloat::op_स्थितिः IEEEFloat::convert_गन्तव्यम्_Sign_Extended_Integer(
    MutableArrayRef<integerPart> parts, अचिह्नित पूर्णाङ्क विस्तारः_2, तार्किक isSigned,
    rounding_बहुलकम् rounding_बहुलकम्_2, तार्किक *isExact) ध्रुव {
  *isExact = असत्य;

  /* Handle the three special cases first.  */
  यदि (category == fcInfinity || category == fcNaN)
    प्रत्यागम opInvalidOp;

  अचिह्नित dst_Parts_गणनम् = part_गणनम्_For_द्वयङ्काः(विस्तारः_2);
  प्रतिज्ञा(dst_Parts_गणनम् <= parts.आकार_2() && "Integer too big");

  यदि (category == fcZero) {
    APInt::tc_समुच्चयः(parts.दत्त(), 0, dst_Parts_गणनम्);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    प्रत्यागम opOK;
  }

  ध्रुव integerPart *src = significandParts();

  अचिह्नित truncated_द्वयङ्काः;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  यदि (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tc_समुच्चयः(parts.दत्त(), 0, dst_Parts_गणनम्);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncated_द्वयङ्काः = semantics->precision -1U - exponent;
  } अन्यथा {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    अचिह्नित पूर्णाङ्क द्वयङ्काः = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    यदि (द्वयङ्काः > विस्तारः_2)
      प्रत्यागम opInvalidOp;

    यदि (द्वयङ्काः < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncated_द्वयङ्काः = semantics->precision - द्वयङ्काः;
      APInt::tcExtract(parts.दत्त(), dst_Parts_गणनम्, src, द्वयङ्काः, truncated_द्वयङ्काः);
    } अन्यथा {
      /* We want at least as many bits as are available.  */
      APInt::tcExtract(parts.दत्त(), dst_Parts_गणनम्, src, semantics->precision,
                       0);
      APInt::tc_सरणम्_वामः(parts.दत्त(), dst_Parts_गणनम्,
                         द्वयङ्काः - semantics->precision);
      truncated_द्वयङ्काः = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  नष्टम्_Fraction नष्टम्_fraction;
  यदि (truncated_द्वयङ्काः) {
    नष्टम्_fraction = नष्टम्_Fraction_Through_Truncation(src, part_गणनम्(),
                                                  truncated_द्वयङ्काः);
    यदि (नष्टम्_fraction != lfExactlyZero &&
        आवर्तनम्_Away_From_Zero(rounding_बहुलकम्_2, नष्टम्_fraction, truncated_द्वयङ्काः)) {
      यदि (APInt::tcIncrement(parts.दत्त(), dst_Parts_गणनम्))
        प्रत्यागम opInvalidOp;     /* Overflow.  */
    }
  } अन्यथा {
    नष्टम्_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  अचिह्नित पूर्णाङ्क omsb = APInt::tcMSB(parts.दत्त(), dst_Parts_गणनम्) + 1;

  यदि (sign) {
    यदि (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      यदि (omsb != 0)
        प्रत्यागम opInvalidOp;
    } अन्यथा {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      यदि (omsb == विस्तारः_2 &&
          APInt::tcLSB(parts.दत्त(), dst_Parts_गणनम्) + 1 != omsb)
        प्रत्यागम opInvalidOp;

      /* This case can happen because of rounding.  */
      यदि (omsb > विस्तारः_2)
        प्रत्यागम opInvalidOp;
    }

    APInt::tcNegate (parts.दत्त(), dst_Parts_गणनम्);
  } अन्यथा {
    यदि (omsb >= विस्तारः_2 + !isSigned)
      प्रत्यागम opInvalidOp;
  }

  यदि (नष्टम्_fraction == lfExactlyZero) {
    *isExact = सत्य;
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
APFloat::op_स्थितिः
IEEEFloat::convert_गन्तव्यम्_Integer(MutableArrayRef<integerPart> parts,
                            अचिह्नित पूर्णाङ्क विस्तारः_2, तार्किक isSigned,
                            rounding_बहुलकम् rounding_बहुलकम्_2, तार्किक *isExact) ध्रुव {
  op_स्थितिः fs = convert_गन्तव्यम्_Sign_Extended_Integer(parts, विस्तारः_2, isSigned,
                                             rounding_बहुलकम्_2, isExact);

  यदि (fs == opInvalidOp) {
    अचिह्नित पूर्णाङ्क द्वयङ्काः, dst_Parts_गणनम्;

    dst_Parts_गणनम् = part_गणनम्_For_द्वयङ्काः(विस्तारः_2);
    प्रतिज्ञा(dst_Parts_गणनम् <= parts.आकार_2() && "Integer too big");

    यदि (category == fcNaN)
      द्वयङ्काः = 0;
    अन्यथा यदि (sign)
      द्वयङ्काः = isSigned;
    अन्यथा
      द्वयङ्काः = विस्तारः_2 - isSigned;

    tc_समुच्चयः_न्यूनतमम्_Significant_द्वयङ्काः(parts.दत्त(), dst_Parts_गणनम्, द्वयङ्काः);
    यदि (sign && isSigned)
      APInt::tc_सरणम्_वामः(parts.दत्त(), dst_Parts_गणनम्, विस्तारः_2 - 1);
  }

  प्रत्यागम fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::op_स्थितिः IEEEFloat::convertFromUnsignedParts(
    ध्रुव integerPart *src, अचिह्नित पूर्णाङ्क src_गणनम्, rounding_बहुलकम् rounding_बहुलकम्_2) {
  category = fc_प्रसामान्यम्;
  अचिह्नित omsb = APInt::tcMSB(src, src_गणनम्) + 1;
  integerPart *dst = significandParts();
  अचिह्नित dst_गणनम् = part_गणनम्();
  अचिह्नित precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  नष्टम्_Fraction नष्टम्_fraction;
  यदि (precision <= omsb) {
    exponent = omsb - 1;
    नष्टम्_fraction = नष्टम्_Fraction_Through_Truncation(src, src_गणनम्,
                                                  omsb - precision);
    APInt::tcExtract(dst, dst_गणनम्, src, precision, omsb - precision);
  } अन्यथा {
    exponent = precision - 1;
    नष्टम्_fraction = lfExactlyZero;
    APInt::tcExtract(dst, dst_गणनम्, src, omsb, 0);
  }

  प्रत्यागम normalize(rounding_बहुलकम्_2, नष्टम्_fraction);
}

APFloat::op_स्थितिः IEEEFloat::convertFromAPInt(ध्रुव APInt &Val, तार्किक isSigned,
                                              rounding_बहुलकम् rounding_बहुलकम्_2) {
  अचिह्नित पूर्णाङ्क part_गणनम् = Val.get_अङ्कः_शब्दाः();
  APInt api = Val;

  sign = असत्य;
  यदि (isSigned && api.isNegative()) {
    sign = सत्य;
    api = -api;
  }

  प्रत्यागम convertFromUnsignedParts(api.get_Raw_दत्त(), part_गणनम्, rounding_बहुलकम्_2);
}

Expected<APFloat::op_स्थितिः>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        rounding_बहुलकम् rounding_बहुलकम्_2) {
  नष्टम्_Fraction नष्टम्_fraction = lfExactlyZero;

  category = fc_प्रसामान्यम्;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  अचिह्नित parts_गणनम् = part_गणनम्();
  अचिह्नित bitPos = parts_गणनम् * integer_Part_विस्तारः;
  तार्किक computedTrailingFraction = असत्य;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator अन्तः = s.अन्तः();
  StringRef::iterator dot;
  स्वचालित PtrOrErr = skipLeadingZeroesAndAnyDot(begin, अन्तः, &dot);
  यदि (!PtrOrErr)
    प्रत्यागम PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator firstSignificantDigit = p;

  यावत् (p != अन्तः) {
    integerPart hex_मानम्;

    यदि (*p == '.') {
      यदि (dot != अन्तः)
        प्रत्यागम createError("String contains multiple dots");
      dot = p++;
      अनुवर्त;
    }

    hex_मानम् = hex_Digit_मानम्(*p);
    यदि (hex_मानम् == UINT_MAX)
      भङ्ग;

    p++;

    // Store the number while we have space.
    यदि (bitPos) {
      bitPos -= 4;
      hex_मानम् <<= bitPos % integer_Part_विस्तारः;
      significand[bitPos / integer_Part_विस्तारः] |= hex_मानम्;
    } अन्यथा यदि (!computedTrailingFraction) {
      स्वचालित FractOrErr = trailingHexadecimalFraction(p, अन्तः, hex_मानम्);
      यदि (!FractOrErr)
        प्रत्यागम FractOrErr.takeError();
      नष्टम्_fraction = *FractOrErr;
      computedTrailingFraction = सत्य;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  यदि (p == अन्तः)
    प्रत्यागम createError("Hex strings require an exponent");
  यदि (*p != 'p' && *p != 'P')
    प्रत्यागम createError("Invalid character in significand");
  यदि (p == begin)
    प्रत्यागम createError("Significand has no digits");
  यदि (dot != अन्तः && p - begin == 1)
    प्रत्यागम createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  यदि (p != firstSignificantDigit) {
    पूर्णाङ्क expAdjustment;

    /* Implicit hexadecimal point?  */
    यदि (dot == अन्तः)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<पूर्णाङ्क>(dot - firstSignificantDigit);
    यदि (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_गणनम् * integer_Part_विस्तारः;

    /* Adjust for the given exponent.  */
    स्वचालित ExpOrErr = योग_Exponent(p + 1, अन्तः, expAdjustment);
    यदि (!ExpOrErr)
      प्रत्यागम ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  प्रत्यागम normalize(rounding_बहुलकम्_2, नष्टम्_fraction);
}

APFloat::op_स्थितिः
IEEEFloat::आवर्तनम्_Significand_With_Exponent(ध्रुव integerPart *decSigParts,
                                        अचिह्नित sig_Part_गणनम्, पूर्णाङ्क घातांकः,
                                        rounding_बहुलकम् rounding_बहुलकम्_2) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  अचिह्नित पूर्णाङ्क power = घातांकः >= 0 ? घातांकः : -घातांकः;
  अचिह्नित पूर्णाङ्क pow5_Part_क्षमता = power_सम्बन्धि_Five_Parts_For_Exponent(power);
  Small_सदिशम्<integerPart, अङ्कः_Pow5_Parts_On_राशिः> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5_Part_क्षमता);

  तार्किक is_निकटतमः = rounding_बहुलकम्_2 == rm_निकटतमः_Ties_गन्तव्यम्_Even ||
                   rounding_बहुलकम्_2 == rm_निकटतमः_Ties_गन्तव्यम्_Away;

  अचिह्नित parts = part_गणनम्_For_द्वयङ्काः(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  अचिह्नित pow5_Part_गणनम् = powerOf5(pow5Parts.दत्त(), power);

  प्रति (;; parts *= 2) {
    अचिह्नित पूर्णाङ्क excessPrecision, truncated_द्वयङ्काः;

    calcSemantics.precision = parts * integer_Part_विस्तारः - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncated_द्वयङ्काः = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    op_स्थितिः sig_स्थितिः = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_गणनम्, rm_निकटतमः_Ties_गन्तव्यम्_Even);
    op_स्थितिः pow_स्थितिः = pow5.convertFromUnsignedParts(
        pow5Parts.दत्त(), pow5_Part_गणनम्, rm_निकटतमः_Ties_गन्तव्यम्_Even);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += घातांकः;

    नष्टम्_Fraction calc_नष्टम्_Fraction;
    integerPart HUerr, HUdistance;
    अचिह्नित पूर्णाङ्क powHUerr;

    यदि (घातांकः >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calc_नष्टम्_Fraction = decSig.multiplySignificand(pow5);
      powHUerr = pow_स्थितिः != opOK;
    } अन्यथा {
      calc_नष्टम्_Fraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      यदि (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncated_द्वयङ्काः = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (pow_स्थितिः == opOK && calc_नष्टम्_Fraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    प्रतिज्ञा(APInt::tcExtractBit
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calc_नष्टम्_Fraction != lfExactlyZero, sig_स्थितिः != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, is_निकटतमः);

    /* Are we guaranteed to round correctly if we truncate?  */
    यदि (HUdistance >= HUerr) {
      APInt::tcExtract(significandParts(), part_गणनम्(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calc_नष्टम्_Fraction = नष्टम्_Fraction_Through_Truncation(decSig.significandParts(),
                                                       decSig.part_गणनम्(),
                                                       truncated_द्वयङ्काः);
      प्रत्यागम static_cast<op_स्थितिः>(normalize(rounding_बहुलकम्_2, calc_नष्टम्_Fraction) |
                                   ((sig_स्थितिः | pow_स्थितिः) & opInexact));
    }
  }
}

Expected<APFloat::op_स्थितिः>
IEEEFloat::convertFromDecimalString(StringRef str, rounding_बहुलकम् rounding_बहुलकम्_2) {
  decimalInfo D;
  op_स्थितिः fs;

  /* Scan the text.  */
  StringRef::iterator p = str.begin();
  यदि (Error Err = interpretDecimal(p, str.अन्तः(), &D))
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
  यदि (D.firstSigDigit == str.अन्तः() || dec_Digit_मानम्(*D.firstSigDigit) >= 10U) {
    category = fcZero;
    fs = opOK;
    यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = असत्य;
    यदि (!semantics->hasZero)
      makeSmallestNormalized(असत्य);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } अन्यथा यदि (D.normalizedExponent - 1 > अधिकतम_पूर्णाङ्कः / 42039) {
    fs = handle_अतिप्रवाहः(rounding_बहुलकम्_2);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } अन्यथा यदि (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (पूर्णाङ्क) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fc_प्रसामान्यम्;
    zeroSignificand();
    fs = normalize(rounding_बहुलकम्_2, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } अन्यथा यदि ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handle_अतिप्रवाहः(rounding_बहुलकम्_2);
  } अन्यथा {
    integerPart *decSignificand;
    अचिह्नित पूर्णाङ्क part_गणनम्;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_गणनम् = static_cast<अचिह्नित पूर्णाङ्क>(D.चरमः_Sig_Digit - D.firstSigDigit) + 1;
    part_गणनम् = part_गणनम्_For_द्वयङ्काः(1 + 196 * part_गणनम् / 59);
    decSignificand = नूतन integerPart[part_गणनम् + 1];
    part_गणनम् = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    कुरु {
      integerPart dec_मानम्, val, multiplier;

      val = 0;
      multiplier = 1;

      कुरु {
        यदि (*p == '.') {
          p++;
          यदि (p == str.अन्तः()) {
            भङ्ग;
          }
        }
        dec_मानम् = dec_Digit_मानम्(*p++);
        यदि (dec_मानम् >= 10U) {
          विलोपय[] decSignificand;
          प्रत्यागम createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_मानम्;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } यावत् (p <= D.चरमः_Sig_Digit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_गणनम्, part_गणनम् + 1, असत्य);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      यदि (decSignificand[part_गणनम्])
        part_गणनम्++;
    } यावत् (p <= D.चरमः_Sig_Digit);

    category = fc_प्रसामान्यम्;
    fs = आवर्तनम्_Significand_With_Exponent(decSignificand, part_गणनम्,
                                      D.exponent, rounding_बहुलकम्_2);

    विलोपय [] decSignificand;
  }

  प्रत्यागम fs;
}

तार्किक IEEEFloat::convertFromStringSpecials(StringRef str) {
  ध्रुव size_t MIN_NAME_आकार = 3;

  यदि (str.आकार_2() < MIN_NAME_आकार)
    प्रत्यागम असत्य;

  यदि (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(असत्य);
    प्रत्यागम सत्य;
  }

  तार्किक IsNegative = str.consume_front("-");
  यदि (IsNegative) {
    यदि (str.आकार_2() < MIN_NAME_आकार)
      प्रत्यागम असत्य;

    यदि (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(सत्य);
      प्रत्यागम सत्य;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  तार्किक IsSignaling = str.consume_front_insensitive("s");
  यदि (IsSignaling) {
    यदि (str.आकार_2() < MIN_NAME_आकार)
      प्रत्यागम असत्य;
  }

  यदि (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    यदि (str.रिक्तानि()) {
      makeNaN(IsSignaling, IsNegative);
      प्रत्यागम सत्य;
    }

    // Allow the payload to be inside parentheses.
    यदि (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      यदि (str.आकार_2() <= 2 || str.पृष्ठभागः() != ')')
        प्रत्यागम असत्य;

      str = str.खण्डिका(1, str.आकार_2() - 1);
    }

    // Determine the payload number's radix.
    अचिह्नित Radix = 10;
    यदि (str[0] == '0') {
      यदि (str.आकार_2() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } अन्यथा {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    यदि (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      प्रत्यागम सत्य;
    }
  }

  प्रत्यागम असत्य;
}

Expected<APFloat::op_स्थितिः>
IEEEFloat::convertFromString(StringRef str, rounding_बहुलकम् rounding_बहुलकम्_2) {
  यदि (str.रिक्तानि())
    प्रत्यागम createError("Invalid string length");

  // Handle special cases.
  यदि (convertFromStringSpecials(str))
    प्रत्यागम opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.आकार_2();
  sign = *p == '-' ? 1 : 0;
  यदि (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  यदि (*p == '-' || *p == '+') {
    p++;
    slen--;
    यदि (!slen)
      प्रत्यागम createError("String has no digits");
  }

  यदि (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    यदि (slen == 2)
      प्रत्यागम createError("Invalid string");
    प्रत्यागम convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_बहुलकम्_2);
  }

  प्रत्यागम convertFromDecimalString(StringRef(p, slen), rounding_बहुलकम्_2);
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
अचिह्नित पूर्णाङ्क IEEEFloat::convert_गन्तव्यम्_Hex_String(वर्ण *dst, अचिह्नित पूर्णाङ्क hexDigits,
                                           तार्किक ऊर्ध्वस्थम्_Case,
                                           rounding_बहुलकम् rounding_बहुलकम्_2) ध्रुव {
  वर्ण *p = dst;
  यदि (sign)
    *dst++ = '-';

  विकल्प (category) {
  पक्ष fcInfinity:
    स्मृतिं_प्रतिलिख (dst, ऊर्ध्वस्थम्_Case ? infinityU: infinityL, परिमाण infinityU - 1);
    dst += परिमाण infinityL - 1;
    भङ्ग;

  पक्ष fcNaN:
    स्मृतिं_प्रतिलिख (dst, ऊर्ध्वस्थम्_Case ? NaNU: NaNL, परिमाण NaNU - 1);
    dst += परिमाण NaNU - 1;
    भङ्ग;

  पक्ष fcZero:
    *dst++ = '0';
    *dst++ = ऊर्ध्वस्थम्_Case ? 'X': 'x';
    *dst++ = '0';
    यदि (hexDigits > 1) {
      *dst++ = '.';
      स्मृतिं_पूरय (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = ऊर्ध्वस्थम्_Case ? 'P': 'p';
    *dst++ = '0';
    भङ्ग;

  पक्ष fc_प्रसामान्यम्:
    dst = convert_प्रसामान्यम्_गन्तव्यम्_Hex_String (dst, hexDigits, ऊर्ध्वस्थम्_Case, rounding_बहुलकम्_2);
    भङ्ग;
  }

  *dst = 0;

  प्रत्यागम static_cast<अचिह्नित पूर्णाङ्क>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
वर्ण *IEEEFloat::convert_प्रसामान्यम्_गन्तव्यम्_Hex_String(वर्ण *dst, अचिह्नित पूर्णाङ्क hexDigits,
                                          तार्किक ऊर्ध्वस्थम्_Case,
                                          rounding_बहुलकम् rounding_बहुलकम्_2) ध्रुव {
  *dst++ = '0';
  *dst++ = ऊर्ध्वस्थम्_Case ? 'X': 'x';

  तार्किक आवर्तनम्_ऊर्ध्वम् = असत्य;
  ध्रुव वर्ण *hexDigitChars = ऊर्ध्वस्थम्_Case ? hex_Digits_ऊर्ध्वस्थम् : hex_Digits_अधस्तनम्;

  ध्रुव integerPart *significand = significandParts();
  अचिह्नित parts_गणनम् = part_गणनम्();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  अचिह्नित मानम्_द्वयङ्काः = semantics->precision + 3;
  अचिह्नित सरणम् = integer_Part_विस्तारः - मानम्_द्वयङ्काः % integer_Part_विस्तारः;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  अचिह्नित outputDigits = (मानम्_द्वयङ्काः - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  यदि (hexDigits) {
    यदि (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      अचिह्नित पूर्णाङ्क द्वयङ्काः;
      नष्टम्_Fraction fraction;

      द्वयङ्काः = मानम्_द्वयङ्काः - hexDigits * 4;
      fraction = नष्टम्_Fraction_Through_Truncation (significand, parts_गणनम्, द्वयङ्काः);
      आवर्तनम्_ऊर्ध्वम् = आवर्तनम्_Away_From_Zero(rounding_बहुलकम्_2, fraction, द्वयङ्काः);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  वर्ण *p = ++dst;

  अचिह्नित गणनम् = (मानम्_द्वयङ्काः + integer_Part_विस्तारः - 1) / integer_Part_विस्तारः;

  यावत् (outputDigits && गणनम्) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    यदि (--गणनम् == parts_गणनम्)
      part = 0;  /* An imaginary higher zero part.  */
    अन्यथा
      part = significand[गणनम्] << सरणम्;

    यदि (गणनम् && सरणम्)
      part |= significand[गणनम् - 1] >> (integer_Part_विस्तारः - सरणम्);

    /* Convert as much of "part" to hexdigits as we can.  */
    अचिह्नित पूर्णाङ्क curDigits = integer_Part_विस्तारः / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  यदि (आवर्तनम्_ऊर्ध्वम्) {
    वर्ण *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    कुरु {
      q--;
      *q = hexDigitChars[hex_Digit_मानम् (*q) + 1];
    } यावत् (*q == '0');
    प्रतिज्ञा(q >= p);
  } अन्यथा {
    /* Add trailing zeroes.  */
    स्मृतिं_पूरय (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  यदि (dst -1 == p)
    dst--;
  अन्यथा
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = ऊर्ध्वस्थम्_Case ? 'P': 'p';

  प्रत्यागम writeSignedDecimal (dst, exponent);
}

hash_संहिता hash_मानम्(ध्रुव IEEEFloat &Arg) {
  यदि (!Arg.isFiniteNonZero())
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

APInt IEEEFloat::convert_F80_Long_Double_APFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 2);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semX87DoubleExtended>();
}

APInt IEEEFloat::convert_PPCDouble_Double_Legacy_APFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(semantics ==
         (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semPPCDoubleDoubleLegacy);
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
  extendedSemantics.minExponent = APFloat_आधारः::semIEEEdouble.minExponent;
  IEEEFloat extended(*एतत्);
  [[maybe_अप्रयुक्तम्]] op_स्थितिः fs =
      extended.convert(extendedSemantics, rm_निकटतमः_Ties_गन्तव्यम्_Even, &losesInfo);
  प्रतिज्ञा(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloat_आधारः::semIEEEdouble, rm_निकटतमः_Ties_गन्तव्यम्_Even, &losesInfo);
  प्रतिज्ञा(fs == opOK || fs == opInexact);
  शब्दाः[0] = *u.convert_Double_APFloat_गन्तव्यम्_APInt().get_Raw_दत्त();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  यदि (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rm_निकटतमः_Ties_गन्तव्यम्_Even, &losesInfo);
    प्रतिज्ञा(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.व्यवकलय(u, rm_निकटतमः_Ties_गन्तव्यम्_Even);
    fs = v.convert(APFloat_आधारः::semIEEEdouble, rm_निकटतमः_Ties_गन्तव्यम्_Even, &losesInfo);
    प्रतिज्ञा(fs == opOK && !losesInfo);
    शब्दाः[1] = *v.convert_Double_APFloat_गन्तव्यम्_APInt().get_Raw_दत्त();
  } अन्यथा {
    शब्दाः[1] = 0;
  }

  प्रत्यागम APInt(128, शब्दाः);
}

प्रतिरूपम् <ध्रुव fltSemantics &S>
APInt IEEEFloat::convert_IEEEFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(semantics == &S);
  constexpr अचिह्नित पूर्णाङ्क trailing_significand_द्वयङ्काः =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr पूर्णाङ्क integer_bit_part = (S.precision - 1) / integer_Part_विस्तारः;
  constexpr integerPart integer_bit = integerPart{1}
                                      << ((S.precision - 1) % integer_Part_विस्तारः);
  constexpr uint64_t significand_आवरणम् = integer_bit - 1;
  constexpr अचिह्नित पूर्णाङ्क exponent_द्वयङ्काः =
      S.आकार_अन्तर्_द्वयङ्काः - (S.hasSignedRepr ? 1 : 0) - trailing_significand_द्वयङ्काः;
  static_assert(exponent_द्वयङ्काः < 64);
  constexpr uint64_t exponent_आवरणम् = (uint64_t{1} << exponent_द्वयङ्काः) - 1;
  constexpr तार्किक is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr पूर्णाङ्क bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_गणनम्_For_द्वयङ्काः(trailing_significand_द्वयङ्काः)>
      mysignificand;

  यदि (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.आकार_2(),
                mysignificand.begin());
    यदि (myexponent == 1 &&
        !(significandParts()[integer_bit_part] & integer_bit))
      myexponent = 0; // denormal
  } अन्यथा यदि (category == fcZero) {
    यदि (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } अन्यथा यदि (category == fcInfinity) {
    यदि (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    यदि constexpr (S.hasExplicitIntegerBit) {
      mysignificand[0] = integerPart{1} << (trailing_significand_द्वयङ्काः - 1);
    }
  } अन्यथा {
    प्रतिज्ञा(category == fcNaN && "Unknown category!");
    यदि (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.आकार_2(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.आकार_अन्तर्_द्वयङ्काः + 63) / 64> शब्दाः;
  स्वचालित शब्दाः_iter =
      std::copy_n(mysignificand.begin(), mysignificand.आकार_2(), शब्दाः.begin());
  यदि constexpr (!S.hasExplicitIntegerBit) {
    यदि constexpr (significand_आवरणम् != 0 || trailing_significand_द्वयङ्काः == 0) {
      // Clear the integer bit.
      शब्दाः[mysignificand.आकार_2() - 1] &= significand_आवरणम्;
    }
  }
  std::fill(शब्दाः_iter, शब्दाः.अन्तः(), uint64_t{0});
  constexpr size_t चरमः_word = शब्दाः.आकार_2() - 1;
  यदि constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.आकार_अन्तर्_द्वयङ्काः - 1) % 64);
    शब्दाः[चरमः_word] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_आवरणम्)
                              << (trailing_significand_द्वयङ्काः % 64);
  शब्दाः[चरमः_word] |= shifted_exponent;
  यदि constexpr (चरमः_word == 0) {
    प्रत्यागम APInt(S.आकार_अन्तर्_द्वयङ्काः, शब्दाः[0]);
  }
  प्रत्यागम APInt(S.आकार_अन्तर्_द्वयङ्काः, शब्दाः);
}

APInt IEEEFloat::convert_Quadruple_APFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 2);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semIEEEquad>();
}

APInt IEEEFloat::convert_Double_APFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्()==1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semIEEEdouble>();
}

APInt IEEEFloat::convert_Float_APFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्()==1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semIEEEsingle>();
}

APInt IEEEFloat::convert_BFloat_APFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semBFloat>();
}

APInt IEEEFloat::convert_Half_APFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्()==1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::APFloat_आधारः::semIEEEhalf>();
}

APInt IEEEFloat::convert_Float8_E5_M2_APFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semFloat8E5M2>();
}

APInt IEEEFloat::convert_Float8_E5_M2_FNUZAPFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convert_Float8_E4_M3_APFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semFloat8E4M3>();
}

APInt IEEEFloat::convert_Float8_E4_M3_FNAPFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semFloat8E4M3FN>();
}

APInt IEEEFloat::convert_Float8_E4_M3_FNUZAPFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convert_Float8_E4_M3_B11_FNUZAPFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convert_Float8_E3_M4_APFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semFloat8E3M4>();
}

APInt IEEEFloat::convert_Float_TF32_APFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semFloatTF32>();
}

APInt IEEEFloat::convert_Float8_E8_M0_FNUAPFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convert_Float8_E5_M3_FNUAPFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convert_Float6_E3_M2_FNAPFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semFloat6E3M2FN>();
}

APInt IEEEFloat::convert_Float6_E2_M3_FNAPFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semFloat6E2M3FN>();
}

APInt IEEEFloat::convert_Float4_E2_M1_FNAPFloat_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(part_गणनम्() == 1);
  प्रत्यागम convert_IEEEFloat_गन्तव्यम्_APInt<APFloat_आधारः::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcast_गन्तव्यम्_APInt() ध्रुव {
  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semIEEEhalf)
    प्रत्यागम convert_Half_APFloat_गन्तव्यम्_APInt();

  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semBFloat)
    प्रत्यागम convert_BFloat_APFloat_गन्तव्यम्_APInt();

  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semIEEEsingle)
    प्रत्यागम convert_Float_APFloat_गन्तव्यम्_APInt();

  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semIEEEdouble)
    प्रत्यागम convert_Double_APFloat_गन्तव्यम्_APInt();

  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semIEEEquad)
    प्रत्यागम convert_Quadruple_APFloat_गन्तव्यम्_APInt();

  यदि (semantics ==
      (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semX87DoubleExtended)
    प्रत्यागम convert_F80_Long_Double_APFloat_गन्तव्यम्_APInt();

  यदि (semantics ==
      (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semPPCDoubleDoubleLegacy)
    प्रत्यागम convert_PPCDouble_Double_Legacy_APFloat_गन्तव्यम्_APInt();

  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semFloat8E5M2)
    प्रत्यागम convert_Float8_E5_M2_APFloat_गन्तव्यम्_APInt();

  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semFloat8E5M2FNUZ)
    प्रत्यागम convert_Float8_E5_M2_FNUZAPFloat_गन्तव्यम्_APInt();

  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semFloat8E4M3)
    प्रत्यागम convert_Float8_E4_M3_APFloat_गन्तव्यम्_APInt();

  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semFloat8E4M3FN)
    प्रत्यागम convert_Float8_E4_M3_FNAPFloat_गन्तव्यम्_APInt();

  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semFloat8E4M3FNUZ)
    प्रत्यागम convert_Float8_E4_M3_FNUZAPFloat_गन्तव्यम्_APInt();

  यदि (semantics ==
      (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semFloat8E4M3B11FNUZ)
    प्रत्यागम convert_Float8_E4_M3_B11_FNUZAPFloat_गन्तव्यम्_APInt();

  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semFloat8E3M4)
    प्रत्यागम convert_Float8_E3_M4_APFloat_गन्तव्यम्_APInt();

  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semFloatTF32)
    प्रत्यागम convert_Float_TF32_APFloat_गन्तव्यम्_APInt();

  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semFloat8E8M0FNU)
    प्रत्यागम convert_Float8_E8_M0_FNUAPFloat_गन्तव्यम्_APInt();

  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semFloat8E5M3FNU)
    प्रत्यागम convert_Float8_E5_M3_FNUAPFloat_गन्तव्यम्_APInt();

  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semFloat6E3M2FN)
    प्रत्यागम convert_Float6_E3_M2_FNAPFloat_गन्तव्यम्_APInt();

  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semFloat6E2M3FN)
    प्रत्यागम convert_Float6_E2_M3_FNAPFloat_गन्तव्यम्_APInt();

  यदि (semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semFloat4E2M1FN)
    प्रत्यागम convert_Float4_E2_M1_FNAPFloat_गन्तव्यम्_APInt();

  llvm_unreachable("unknown format!");
}

भिन्न IEEEFloat::convert_गन्तव्यम्_Float() ध्रुव {
  प्रतिज्ञा(semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcast_गन्तव्यम्_APInt();
  प्रत्यागम api.द्वयङ्काः_गन्तव्यम्_Float();
}

द्विगुण IEEEFloat::convert_गन्तव्यम्_Double() ध्रुव {
  प्रतिज्ञा(semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcast_गन्तव्यम्_APInt();
  प्रत्यागम api.द्वयङ्काः_गन्तव्यम्_Double();
}

#यदि_परिभाषितम् HAS_IEE754_FLOAT128
float128 IEEEFloat::convert_गन्तव्यम्_Quad() ध्रुव {
  प्रतिज्ञा(semantics == (ध्रुव llvm::fltSemantics *)&APFloat_आधारः::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcast_गन्तव्यम्_APInt();
  प्रत्यागम api.द्वयङ्काः_गन्तव्यम्_Quad();
}
#यदि_अन्तः

शून्य IEEEFloat::initFromF80LongDoubleAPInt(ध्रुव APInt &api) {
  प्रत्यागम initFromIEEEAPInt<APFloat_आधारः::semX87DoubleExtended>(api);
}

शून्य IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(ध्रुव APInt &api) {
  uint64_t i1 = api.get_Raw_दत्त()[0];
  uint64_t i2 = api.get_Raw_दत्त()[1];
  तार्किक losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_अप्रयुक्तम्]] op_स्थितिः fs = convert(APFloat_आधारः::semPPCDoubleDoubleLegacy,
                                         rm_निकटतमः_Ties_गन्तव्यम्_Even, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  प्रतिज्ञा((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  यदि (isFiniteNonZero()) {
    IEEEFloat v(APFloat_आधारः::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    यदि (v.isFinite()) {
      fs = v.convert(APFloat_आधारः::semPPCDoubleDoubleLegacy, rm_निकटतमः_Ties_गन्तव्यम्_Even,
                     &losesInfo);
      प्रतिज्ञा(fs == opOK && !losesInfo);

      सङ्कलय(v, rm_निकटतमः_Ties_गन्तव्यम्_Even);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
शून्य IEEEFloat::initFromFloat8E8M0FNUAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semFloat8E8M0FNU>(api);
}

शून्य IEEEFloat::initFromFloat8E5M3FNUAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semFloat8E5M3FNU>(api);
}

प्रतिरूपम् <ध्रुव fltSemantics &S>
शून्य IEEEFloat::initFromIEEEAPInt(ध्रुव APInt &api) {
  प्रतिज्ञा(api.get_Bit_विस्तारः() == S.आकार_अन्तर्_द्वयङ्काः);

  constexpr अचिह्नित पूर्णाङ्क trailing_significand_द्वयङ्काः =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr integerPart integer_bit =
      integerPart{1} << (trailing_significand_द्वयङ्काः % integer_Part_विस्तारः);
  constexpr uint64_t significand_आवरणम् = integer_bit - 1;
  constexpr अचिह्नित पूर्णाङ्क exponent_द्वयङ्काः =
      S.आकार_अन्तर्_द्वयङ्काः - (S.hasSignedRepr ? 1 : 0) - trailing_significand_द्वयङ्काः;
  static_assert(exponent_द्वयङ्काः < 64);
  constexpr अचिह्नित पूर्णाङ्क stored_significand_parts =
      part_गणनम्_For_द्वयङ्काः(trailing_significand_द्वयङ्काः + 1);
  constexpr uint64_t exponent_आवरणम् = (uint64_t{1} << exponent_द्वयङ्काः) - 1;
  constexpr तार्किक is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr पूर्णाङ्क bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr तार्किक has_significand = trailing_significand_द्वयङ्काः > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  यदि constexpr (has_significand) {
    std::copy_n(api.get_Raw_दत्त(), mysignificand.आकार_2(), mysignificand.begin());
    यदि constexpr (significand_आवरणम् != 0 || S.precision >= integer_Part_विस्तारः) {
      mysignificand[mysignificand.आकार_2() - 1] &= significand_आवरणम्;
    }
  } अन्यथा {
    std::fill_n(mysignificand.begin(), mysignificand.आकार_2(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t चरमः_word = api.get_Raw_दत्त()[api.get_अङ्कः_शब्दाः() - 1];
  uint64_t myexponent =
      (चरमः_word >> (trailing_significand_द्वयङ्काः % 64)) & exponent_आवरणम्;

  initialize(&S);
  प्रतिज्ञा(part_गणनम्() == mysignificand.आकार_2());

  sign = S.hasSignedRepr
             ? static_cast<अचिह्नित पूर्णाङ्क>(चरमः_word >> ((S.आकार_अन्तर्_द्वयङ्काः - 1) % 64))
             : 0;

  तार्किक all_zero_significand =
      has_significand && llvm::all_सम्बन्धि(mysignificand, equal_गन्तव्यम्(0));

  तार्किक is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  यदि constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    तार्किक is_inf = असत्य;

    यदि constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_आवरणम्_no_int_bit =
          (uint64_t{1} << (trailing_significand_द्वयङ्काः - 1)) - 1;
      ध्रुव integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_द्वयङ्काः - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_आवरणम्_no_int_bit) == 0;
    } अन्यथा {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    यदि (is_inf) {
      makeInf(sign);
      प्रत्यागम;
    }
  }

  तार्किक is_nan = असत्य;

  यदि constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    यदि constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      ध्रुव integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_द्वयङ्काः - 1);
      constexpr integerPart significand_आवरणम्_no_int_bit =
          (uint64_t{1} << (trailing_significand_द्वयङ्काः - 1)) - 1;

      यदि (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_आवरणम्_no_int_bit) != 0) {
        // regular NaN and pseudoNaN
        is_nan = सत्य;
      } अन्यथा यदि (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_आवरणम्_no_int_bit) == 0) {
        // pseudoinfinity
        is_nan = सत्य;
      } अन्यथा यदि (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = सत्य;
      }
    } अन्यथा {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } अन्यथा यदि constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    तार्किक all_ones_significand =
        std::all_सम्बन्धि(mysignificand.begin(), mysignificand.अन्तः() - 1,
                    [](integerPart द्वयङ्काः) { प्रत्यागम द्वयङ्काः == ~integerPart{0}; }) &&
        (!significand_आवरणम् ||
         mysignificand[mysignificand.आकार_2() - 1] == significand_आवरणम्);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } अन्यथा यदि constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  यदि (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.आकार_2(),
                significandParts());
    प्रत्यागम;
  }

  यदि (is_zero) {
    makeZero(sign);
    प्रत्यागम;
  }

  category = fc_प्रसामान्यम्;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.आकार_2(), significandParts());
  यदि (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  अन्यथा {
    यदि constexpr (!S.hasExplicitIntegerBit) {
      significandParts()[mysignificand.आकार_2() - 1] |= integer_bit;
    }
  }
}

शून्य IEEEFloat::initFromQuadrupleAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semIEEEquad>(api);
}

शून्य IEEEFloat::initFromDoubleAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semIEEEdouble>(api);
}

शून्य IEEEFloat::initFromFloatAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semIEEEsingle>(api);
}

शून्य IEEEFloat::initFromBFloatAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semBFloat>(api);
}

शून्य IEEEFloat::initFromHalfAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semIEEEhalf>(api);
}

शून्य IEEEFloat::initFromFloat8E5M2APInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semFloat8E5M2>(api);
}

शून्य IEEEFloat::initFromFloat8E5M2FNUZAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semFloat8E5M2FNUZ>(api);
}

शून्य IEEEFloat::initFromFloat8E4M3APInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semFloat8E4M3>(api);
}

शून्य IEEEFloat::initFromFloat8E4M3FNAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semFloat8E4M3FN>(api);
}

शून्य IEEEFloat::initFromFloat8E4M3FNUZAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semFloat8E4M3FNUZ>(api);
}

शून्य IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semFloat8E4M3B11FNUZ>(api);
}

शून्य IEEEFloat::initFromFloat8E3M4APInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semFloat8E3M4>(api);
}

शून्य IEEEFloat::initFromFloatTF32APInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semFloatTF32>(api);
}

शून्य IEEEFloat::initFromFloat6E3M2FNAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semFloat6E3M2FN>(api);
}

शून्य IEEEFloat::initFromFloat6E2M3FNAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semFloat6E2M3FN>(api);
}

शून्य IEEEFloat::initFromFloat4E2M1FNAPInt(ध्रुव APInt &api) {
  initFromIEEEAPInt<APFloat_आधारः::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
शून्य IEEEFloat::initFromAPInt(ध्रुव fltSemantics *Sem, ध्रुव APInt &api) {
  प्रतिज्ञा(api.get_Bit_विस्तारः() == Sem->आकार_अन्तर्_द्वयङ्काः);
  यदि (Sem == &APFloat_आधारः::semIEEEhalf)
    प्रत्यागम initFromHalfAPInt(api);
  यदि (Sem == &APFloat_आधारः::semBFloat)
    प्रत्यागम initFromBFloatAPInt(api);
  यदि (Sem == &APFloat_आधारः::semIEEEsingle)
    प्रत्यागम initFromFloatAPInt(api);
  यदि (Sem == &APFloat_आधारः::semIEEEdouble)
    प्रत्यागम initFromDoubleAPInt(api);
  यदि (Sem == &APFloat_आधारः::semIEEEquad)
    प्रत्यागम initFromQuadrupleAPInt(api);
  यदि (Sem == &APFloat_आधारः::semX87DoubleExtended)
    प्रत्यागम initFromF80LongDoubleAPInt(api);
  यदि (Sem == &APFloat_आधारः::semPPCDoubleDoubleLegacy)
    प्रत्यागम initFromPPCDoubleDoubleLegacyAPInt(api);
  यदि (Sem == &APFloat_आधारः::semFloat8E5M2)
    प्रत्यागम initFromFloat8E5M2APInt(api);
  यदि (Sem == &APFloat_आधारः::semFloat8E5M2FNUZ)
    प्रत्यागम initFromFloat8E5M2FNUZAPInt(api);
  यदि (Sem == &APFloat_आधारः::semFloat8E4M3)
    प्रत्यागम initFromFloat8E4M3APInt(api);
  यदि (Sem == &APFloat_आधारः::semFloat8E4M3FN)
    प्रत्यागम initFromFloat8E4M3FNAPInt(api);
  यदि (Sem == &APFloat_आधारः::semFloat8E4M3FNUZ)
    प्रत्यागम initFromFloat8E4M3FNUZAPInt(api);
  यदि (Sem == &APFloat_आधारः::semFloat8E4M3B11FNUZ)
    प्रत्यागम initFromFloat8E4M3B11FNUZAPInt(api);
  यदि (Sem == &APFloat_आधारः::semFloat8E3M4)
    प्रत्यागम initFromFloat8E3M4APInt(api);
  यदि (Sem == &APFloat_आधारः::semFloatTF32)
    प्रत्यागम initFromFloatTF32APInt(api);
  यदि (Sem == &APFloat_आधारः::semFloat8E8M0FNU)
    प्रत्यागम initFromFloat8E8M0FNUAPInt(api);
  यदि (Sem == &APFloat_आधारः::semFloat8E5M3FNU)
    प्रत्यागम initFromFloat8E5M3FNUAPInt(api);
  यदि (Sem == &APFloat_आधारः::semFloat6E3M2FN)
    प्रत्यागम initFromFloat6E3M2FNAPInt(api);
  यदि (Sem == &APFloat_आधारः::semFloat6E2M3FN)
    प्रत्यागम initFromFloat6E2M3FNAPInt(api);
  यदि (Sem == &APFloat_आधारः::semFloat4E2M1FN)
    प्रत्यागम initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
शून्य IEEEFloat::makeLargest(तार्किक Negative) {
  यदि (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 1..10
  //   significand = 1..1
  category = fc_प्रसामान्यम्;
  sign = Negative;
  exponent = semantics->maxExponent;

  // Use memset to set all but the highest integerPart to all ones.
  integerPart *significand = significandParts();
  अचिह्नित Part_गणनम् = part_गणनम्();
  स्मृतिं_पूरय(significand, 0xFF, परिमाण(integerPart)*(Part_गणनम् - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  ध्रुव अचिह्नित अङ्कः_अप्रयुक्तम्_उच्चः_द्वयङ्काः =
    Part_गणनम्*integer_Part_विस्तारः - semantics->precision;
  significand[Part_गणनम् - 1] = (अङ्कः_अप्रयुक्तम्_उच्चः_द्वयङ्काः < integer_Part_विस्तारः)
                                   ? (~integerPart(0) >> अङ्कः_अप्रयुक्तम्_उच्चः_द्वयङ्काः)
                                   : 0;
  यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
शून्य IEEEFloat::makeSmallest(तार्किक Negative) {
  यदि (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fc_प्रसामान्यम्;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tc_समुच्चयः(significandParts(), 1, part_गणनम्());
}

शून्य IEEEFloat::makeSmallestNormalized(तार्किक Negative) {
  यदि (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 10..0

  category = fc_प्रसामान्यम्;
  zeroSignificand();
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tc_समुच्चयः_Bit(significandParts(), semantics->precision - 1);
}

IEEEFloat::IEEEFloat(ध्रुव fltSemantics &Sem, ध्रुव APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(भिन्न f) {
  initFromAPInt(&APFloat_आधारः::semIEEEsingle, APInt::float_गन्तव्यम्_द्वयङ्काः(f));
}

IEEEFloat::IEEEFloat(द्विगुण d) {
  initFromAPInt(&APFloat_आधारः::semIEEEdouble, APInt::double_गन्तव्यम्_द्वयङ्काः(d));
}

नामाकाश {
  शून्य append(Small_सदिशम्_Impl<वर्ण> &बफर, StringRef Str) {
    बफर.append(Str.begin(), Str.अन्तः());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  शून्य Adjust_गन्तव्यम्_Precision(APInt &significand,
                         पूर्णाङ्क &घातांकः, अचिह्नित FormatPrecision) {
    अचिह्नित द्वयङ्काः = significand.get_सक्रिय_द्वयङ्काः();

    // 196/59 is a very slight overestimate of lg_2(10).
    अचिह्नित द्वयङ्काः_Required = (FormatPrecision * 196 + 58) / 59;

    यदि (द्वयङ्काः <= द्वयङ्काः_Required) प्रत्यागम;

    अचिह्नित tensRemovable = (द्वयङ्काः - द्वयङ्काः_Required) * 59 / 196;
    यदि (!tensRemovable) प्रत्यागम;

    घातांकः += tensRemovable;

    APInt भाजकः(significand.get_Bit_विस्तारः(), 1);
    APInt powten(significand.get_Bit_विस्तारः(), 10);
    यावत् (सत्य) {
      यदि (tensRemovable & 1)
        भाजकः *= powten;
      tensRemovable >>= 1;
      यदि (!tensRemovable) भङ्ग;
      powten *= powten;
    }

    significand = significand.udiv(भाजकः);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.get_सक्रिय_द्वयङ्काः());
  }


  शून्य Adjust_गन्तव्यम्_Precision(Small_सदिशम्_Impl<वर्ण> &बफर_2,
                         पूर्णाङ्क &घातांकः, अचिह्नित FormatPrecision) {
    अचिह्नित N = बफर_2.आकार_2();
    यदि (N <= FormatPrecision) प्रत्यागम;

    // The most significant figures are the last ones in the buffer.
    अचिह्नित FirstSignificant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    यदि (बफर_2[FirstSignificant - 1] < '5') {
      यावत् (FirstSignificant < N && बफर_2[FirstSignificant] == '0')
        FirstSignificant++;

      घातांकः += FirstSignificant;
      बफर_2.erase(&बफर_2[0], &बफर_2[FirstSignificant]);
      प्रत्यागम;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    प्रति (अचिह्नित I = FirstSignificant; I != N; ++I) {
      यदि (बफर_2[I] == '9') {
        FirstSignificant++;
      } अन्यथा {
        बफर_2[I]++;
        भङ्ग;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    यदि (FirstSignificant == N) {
      घातांकः += FirstSignificant;
      बफर_2.clear();
      बफर_2.प्रक्षिप_पृष्ठभागः('1');
      प्रत्यागम;
    }

    घातांकः += FirstSignificant;
    बफर_2.erase(&बफर_2[0], &बफर_2[FirstSignificant]);
  }

  शून्य गन्तव्यम्_String_Impl(Small_सदिशम्_Impl<वर्ण> &Str, ध्रुव तार्किक isNeg, पूर्णाङ्क घातांकः,
                    APInt significand, अचिह्नित FormatPrecision,
                    अचिह्नित FormatMaxPadding, तार्किक TruncateZero) {
    ध्रुव पूर्णाङ्क semanticsPrecision = significand.get_Bit_विस्तारः();

    यदि (isNeg)
      Str.प्रक्षिप_पृष्ठभागः('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    यदि (!FormatPrecision) {
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
    घातांकः += trailingZeros;
    significand.lshr_अन्तर्_Place(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    यदि (घातांकः == 0) {
      // Nothing to do.
    } अन्यथा यदि (घातांकः > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + घातांकः);
      significand <<= घातांकः;
      घातांकः = 0;
    } अन्यथा { /* exp < 0 */
      पूर्णाङ्क texp = -घातांकः;

      // We transform this using the identity:
      //   (N)(2^-e) == (N)(5^e)(10^-e)
      // This means we have to multiply N (the significand) by 5^e.
      // To avoid overflow, we have to operate on numbers large
      // enough to store N * 5^e:
      //   log2(N * 5^e) == log2(N) + e * log2(5)
      //                 <= semantics->precision + e * 137 / 59
      //   (log_2(5) ~ 2.321928 < 2.322034 ~ 137/59)

      अचिह्नित precision = semanticsPrecision + (137 * texp + 136) / 59;

      // Multiply significand by 5^e.
      //   N * 5^0101 == N * 5^(1*1) * 5^(0*2) * 5^(1*4) * 5^(0*8)
      significand = significand.zext(precision);
      APInt five_गन्तव्यम्_the_i(precision, 5);
      यावत् (सत्य) {
        यदि (texp & 1)
          significand *= five_गन्तव्यम्_the_i;

        texp >>= 1;
        यदि (!texp)
          भङ्ग;
        five_गन्तव्यम्_the_i *= five_गन्तव्यम्_the_i;
      }
    }

    Adjust_गन्तव्यम्_Precision(significand, घातांकः, FormatPrecision);

    Small_सदिशम्<वर्ण, 256> बफर_2;

    // Fill the buffer.
    अचिह्नित precision = significand.get_Bit_विस्तारः();
    यदि (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    तार्किक अन्तर्_Trail = सत्य;
    यावत् (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      अचिह्नित d = digit.get_ZExt_मानम्();

      // Drop trailing zeros.
      यदि (अन्तर्_Trail && !d)
        घातांकः++;
      अन्यथा {
        बफर_2.प्रक्षिप_पृष्ठभागः((वर्ण) ('0' + d));
        अन्तर्_Trail = असत्य;
      }
    }

    प्रतिज्ञा(!बफर_2.रिक्तानि() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    Adjust_गन्तव्यम्_Precision(बफर_2, घातांकः, FormatPrecision);

    अचिह्नित NDigits = बफर_2.आकार_2();

    // Check whether we should use scientific notation.
    तार्किक FormatScientific;
    यदि (!FormatMaxPadding) {
      FormatScientific = सत्य;
    } अन्यथा {
      यदि (घातांकः >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((अचिह्नित) घातांकः > FormatMaxPadding ||
                            NDigits + (अचिह्नित) घातांकः > FormatPrecision);
      } अन्यथा {
        // Power of the most significant digit.
        पूर्णाङ्क MSD = घातांकः + (पूर्णाङ्क) (NDigits - 1);
        यदि (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = असत्य;
        } अन्यथा {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((अचिह्नित) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    यदि (FormatScientific) {
      घातांकः += (NDigits - 1);

      Str.प्रक्षिप_पृष्ठभागः(बफर_2[NDigits-1]);
      Str.प्रक्षिप_पृष्ठभागः('.');
      यदि (NDigits == 1 && TruncateZero)
        Str.प्रक्षिप_पृष्ठभागः('0');
      अन्यथा
        प्रति (अचिह्नित I = 1; I != NDigits; ++I)
          Str.प्रक्षिप_पृष्ठभागः(बफर_2[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      यदि (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.प्रक्षिप_पृष्ठभागः(TruncateZero ? 'E' : 'e');

      Str.प्रक्षिप_पृष्ठभागः(घातांकः >= 0 ? '+' : '-');
      यदि (घातांकः < 0)
        घातांकः = -घातांकः;
      Small_सदिशम्<वर्ण, 6> expbuf;
      कुरु {
        expbuf.प्रक्षिप_पृष्ठभागः((वर्ण) ('0' + (घातांकः % 10)));
        घातांकः /= 10;
      } यावत् (घातांकः);
      // Exponent always at least two digits if we do not truncate zeros.
      यदि (!TruncateZero && expbuf.आकार_2() < 2)
        expbuf.प्रक्षिप_पृष्ठभागः('0');
      प्रति (अचिह्नित I = 0, E = expbuf.आकार_2(); I != E; ++I)
        Str.प्रक्षिप_पृष्ठभागः(expbuf[E-1-I]);
      प्रत्यागम;
    }

    // Non-scientific, positive exponents.
    यदि (घातांकः >= 0) {
      प्रति (अचिह्नित I = 0; I != NDigits; ++I)
        Str.प्रक्षिप_पृष्ठभागः(बफर_2[NDigits-1-I]);
      प्रति (अचिह्नित I = 0; I != (अचिह्नित) घातांकः; ++I)
        Str.प्रक्षिप_पृष्ठभागः('0');
      प्रत्यागम;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    पूर्णाङ्क NWholeDigits = घातांकः + (पूर्णाङ्क) NDigits;

    अचिह्नित I = 0;
    यदि (NWholeDigits > 0) {
      प्रति (; I != (अचिह्नित) NWholeDigits; ++I)
        Str.प्रक्षिप_पृष्ठभागः(बफर_2[NDigits-I-1]);
      Str.प्रक्षिप_पृष्ठभागः('.');
    } अन्यथा {
      अचिह्नित NZeros = 1 + (अचिह्नित) -NWholeDigits;

      Str.प्रक्षिप_पृष्ठभागः('0');
      Str.प्रक्षिप_पृष्ठभागः('.');
      प्रति (अचिह्नित Z = 1; Z != NZeros; ++Z)
        Str.प्रक्षिप_पृष्ठभागः('0');
    }

    प्रति (; I != NDigits; ++I)
      Str.प्रक्षिप_पृष्ठभागः(बफर_2[NDigits-I-1]);

  }
} // namespace

शून्य IEEEFloat::गन्तव्यम्_String(Small_सदिशम्_Impl<वर्ण> &Str, अचिह्नित FormatPrecision,
                         अचिह्नित FormatMaxPadding, तार्किक TruncateZero) ध्रुव {
  विकल्प (category) {
  पक्ष fcInfinity:
    यदि (isNegative())
      प्रत्यागम append(Str, "-Inf");
    अन्यथा
      प्रत्यागम append(Str, "+Inf");

  पक्ष fcNaN: प्रत्यागम append(Str, "NaN");

  पक्ष fcZero:
    यदि (isNegative())
      Str.प्रक्षिप_पृष्ठभागः('-');

    यदि (!FormatMaxPadding) {
      यदि (TruncateZero)
        append(Str, "0.0E+0");
      अन्यथा {
        append(Str, "0.0");
        यदि (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } अन्यथा {
      Str.प्रक्षिप_पृष्ठभागः('0');
    }
    प्रत्यागम;

  पक्ष fc_प्रसामान्यम्:
    भङ्ग;
  }

  // Decompose the number into an APInt and an exponent.
  पूर्णाङ्क घातांकः = exponent - ((पूर्णाङ्क) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_गणनम्_For_द्वयङ्काः(semantics->precision)));

  गन्तव्यम्_String_Impl(Str, isNegative(), घातांकः, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

पूर्णाङ्क IEEEFloat::getExactLog2Abs() ध्रुव {
  यदि (!isFinite() || isZero())
    प्रत्यागम INT_MIN;

  ध्रुव integerPart *Parts = significandParts();
  ध्रुव पूर्णाङ्क Part_गणनम् = part_गणनम्_For_द्वयङ्काः(semantics->precision);

  पूर्णाङ्क निष्कासय_गणनम् = 0;
  प्रति (पूर्णाङ्क i = 0; i < Part_गणनम्; ++i) {
    निष्कासय_गणनम् += llvm::popcount(Parts[i]);
    यदि (निष्कासय_गणनम् > 1)
      प्रत्यागम INT_MIN;
  }

  यदि (exponent != semantics->minExponent)
    प्रत्यागम exponent;

  पूर्णाङ्क CountrParts = 0;
  प्रति (पूर्णाङ्क i = 0; i < Part_गणनम्;
       ++i, CountrParts += APInt::APINT_द्वयङ्काः_प्रत्येकम्_WORD) {
    यदि (Parts[i] != 0) {
      प्रत्यागम exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

तार्किक IEEEFloat::isSignaling() ध्रुव {
  यदि (!isNaN())
    प्रत्यागम असत्य;
  यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    प्रत्यागम असत्य;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  प्रत्यागम !APInt::tcExtractBit(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::op_स्थितिः IEEEFloat::अग्र(तार्किक अग्र_अधस्तात्) {
  // If we are performing nextDown, swap sign so we have -x.
  यदि (अग्र_अधस्तात्)
    changeSign();

  // Compute nextUp(x)
  op_स्थितिः परिणाम_2 = opOK;

  // Handle each float category separately.
  विकल्प (category) {
  पक्ष fcInfinity:
    // nextUp(+inf) = +inf
    यदि (!isNegative())
      भङ्ग;
    // nextUp(-inf) = -getLargest()
    makeLargest(सत्य);
    भङ्ग;
  पक्ष fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    यदि (isSignaling()) {
      परिणाम_2 = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(असत्य, isNegative(), शून्यसूचकः);
    }
    भङ्ग;
  पक्ष fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(असत्य);
    भङ्ग;
  पक्ष fc_प्रसामान्यम्:
    // nextUp(-getSmallest()) = -0
    यदि (isSmallest() && isNegative()) {
      APInt::tc_समुच्चयः(significandParts(), 0, part_गणनम्());
      category = fcZero;
      exponent = 0;
      यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = असत्य;
      यदि (!semantics->hasZero)
        makeSmallestNormalized(असत्य);
      भङ्ग;
    }

    यदि (isLargest() && !isNegative()) {
      यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        भङ्ग;
      } अन्यथा यदि (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        भङ्ग;
      } अन्यथा {
        // nextUp(getLargest()) == INFINITY
        APInt::tc_समुच्चयः(significandParts(), 0, part_गणनम्());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        भङ्ग;
      }
    }

    // nextUp(normal) == normal + inc.
    यदि (isNegative()) {
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

      यदि (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tc_समुच्चयः_Bit(Parts, semantics->precision - 1);
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

      यदि (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tc_समुच्चयः(Parts, 0, part_गणनम्());
        APInt::tc_समुच्चयः_Bit(Parts, semantics->precision - 1);
        प्रतिज्ञा(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } अन्यथा {
        incrementSignificand();
      }
    }
    भङ्ग;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  यदि (अग्र_अधस्तात्)
    changeSign();

  प्रत्यागम परिणाम_2;
}

APInt IEEEFloat::getNaNPayload() ध्रुव {
  प्रतिज्ञा(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  अचिह्नित द्वयङ्काः_2 = semantics->precision - 1;
  प्रत्यागम APInt(द्वयङ्काः_2, ArrayRef(significandParts(), part_गणनम्_For_द्वयङ्काः(द्वयङ्काः_2)));
}

APFloat_आधारः::ExponentType IEEEFloat::exponentNaN() ध्रुव {
  प्रत्यागम ::exponentNaN(*semantics);
}

APFloat_आधारः::ExponentType IEEEFloat::exponentInf() ध्रुव {
  प्रत्यागम ::exponentInf(*semantics);
}

APFloat_आधारः::ExponentType IEEEFloat::exponentZero() ध्रुव {
  प्रत्यागम ::exponentZero(*semantics);
}

शून्य IEEEFloat::makeInf(तार्किक Negative) {
  यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(असत्य, Negative);
    प्रत्यागम;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tc_समुच्चयः(significandParts(), 0, part_गणनम्());
}

शून्य IEEEFloat::makeZero(तार्किक Negative) {
  यदि (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = असत्य;
  }
  exponent = exponentZero();
  APInt::tc_समुच्चयः(significandParts(), 0, part_गणनम्());
}

शून्य IEEEFloat::makeQuiet() {
  प्रतिज्ञा(isNaN());
  यदि (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tc_समुच्चयः_Bit(significandParts(), semantics->precision - 2);
}

पूर्णाङ्क ilogb(ध्रुव IEEEFloat &Arg) {
  यदि (Arg.isNaN())
    प्रत्यागम APFloat::IEK_NaN;
  यदि (Arg.isZero())
    प्रत्यागम APFloat::IEK_Zero;
  यदि (Arg.isInfinity())
    प्रत्यागम APFloat::IEK_Inf;
  यदि (!Arg.isDenormal())
    प्रत्यागम Arg.exponent;

  IEEEFloat Normalized(Arg);
  पूर्णाङ्क Significand_द्वयङ्काः = Arg.getSemantics().precision - 1;

  Normalized.exponent += Significand_द्वयङ्काः;
  Normalized.normalize(APFloat::rm_निकटतमः_Ties_गन्तव्यम्_Even, lfExactlyZero);
  प्रत्यागम Normalized.exponent - Significand_द्वयङ्काः;
}

IEEEFloat scalbn(IEEEFloat X, पूर्णाङ्क Exp, rounding_बहुलकम् Rounding_बहुलकम्) {
  स्वचालित MaxExp = X.getSemantics().maxExponent;
  स्वचालित MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  पूर्णाङ्क Significand_द्वयङ्काः = X.getSemantics().precision - 1;
  पूर्णाङ्क MaxIncrement = MaxExp - (MinExp - Significand_द्वयङ्काः) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(Rounding_बहुलकम्, lfExactlyZero);
  यदि (X.isNaN())
    X.makeQuiet();
  प्रत्यागम X;
}

IEEEFloat frexp(ध्रुव IEEEFloat &Val, पूर्णाङ्क &Exp, rounding_बहुलकम् RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  यदि (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    प्रत्यागम Quiet;
  }

  यदि (Exp == APFloat::IEK_Inf)
    प्रत्यागम Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  प्रत्यागम scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(ध्रुव fltSemantics &S)
    : Semantics(&S),
      Floats(नूतन APFloat[2]{APFloat(APFloat_आधारः::semIEEEdouble),
                            APFloat(APFloat_आधारः::semIEEEdouble)}) {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ध्रुव fltSemantics &S, uninitialized_चिह्नकम्)
    : Semantics(&S), Floats(नूतन APFloat[2]{
                         APFloat(APFloat_आधारः::semIEEEdouble, uninitialized),
                         APFloat(APFloat_आधारः::semIEEEdouble, uninitialized)}) {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ध्रुव fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(नूतन APFloat[2]{APFloat(APFloat_आधारः::semIEEEdouble, I),
                            APFloat(APFloat_आधारः::semIEEEdouble)}) {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ध्रुव fltSemantics &S, ध्रुव APInt &I)
    : Semantics(&S),
      Floats(नूतन APFloat[2]{
          APFloat(APFloat_आधारः::semIEEEdouble, APInt(64, I.get_Raw_दत्त()[0])),
          APFloat(APFloat_आधारः::semIEEEdouble, APInt(64, I.get_Raw_दत्त()[1]))}) {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(ध्रुव fltSemantics &S, APFloat &&First,
                             APFloat &&Second)
    : Semantics(&S),
      Floats(नूतन APFloat[2]{std::move(First), std::move(Second)}) {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble);
  प्रतिज्ञा(&Floats[0].getSemantics() == &APFloat_आधारः::semIEEEdouble);
  प्रतिज्ञा(&Floats[1].getSemantics() == &APFloat_आधारः::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(ध्रुव DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? नूतन APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : शून्यसूचकः) {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloat_आधारः::semBogus;
  RHS.Floats = शून्यसूचकः;
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::संक्रियकः=(ध्रुव DoubleAPFloat &RHS) {
  यदि (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } अन्यथा यदि (एतत् != &RHS) {
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
स्थिर std::pair<APFloat, APFloat> fast_Two_योग(APFloat X, APFloat Y) {
  यदि (!X.isFinite())
    प्रत्यागम {X, APFloat::getZero(X.getSemantics(), /*Negative=*/असत्य)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  प्रत्यागम {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::op_स्थितिः DoubleAPFloat::सङ्कलय_Impl(ध्रुव APFloat &a, ध्रुव APFloat &aa,
                                         ध्रुव APFloat &c, ध्रुव APFloat &cc,
                                         rounding_बहुलकम् RM) {
  पूर्णाङ्क स्थितिः = opOK;
  APFloat z = a;
  स्थितिः |= z.सङ्कलय(c, RM);
  यदि (!z.isFinite()) {
    यदि (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ असत्य);
      प्रत्यागम (op_स्थितिः)स्थितिः;
    }
    स्थितिः = opOK;
    स्वचालित ACompared_गन्तव्यम्_C = a.compare_Absolute_मानम्(c);
    z = cc;
    स्थितिः |= z.सङ्कलय(aa, RM);
    यदि (ACompared_गन्तव्यम्_C == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      स्थितिः |= z.सङ्कलय(c, RM);
      स्थितिः |= z.सङ्कलय(a, RM);
    } अन्यथा {
      // z = cc + aa + a + c;
      स्थितिः |= z.सङ्कलय(a, RM);
      स्थितिः |= z.सङ्कलय(c, RM);
    }
    यदि (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ असत्य);
      प्रत्यागम (op_स्थितिः)स्थितिः;
    }
    Floats[0] = z;
    APFloat zz = aa;
    स्थितिः |= zz.सङ्कलय(cc, RM);
    यदि (ACompared_गन्तव्यम्_C == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      स्थितिः |= Floats[1].व्यवकलय(z, RM);
      स्थितिः |= Floats[1].सङ्कलय(c, RM);
      स्थितिः |= Floats[1].सङ्कलय(zz, RM);
    } अन्यथा {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      स्थितिः |= Floats[1].व्यवकलय(z, RM);
      स्थितिः |= Floats[1].सङ्कलय(a, RM);
      स्थितिः |= Floats[1].सङ्कलय(zz, RM);
    }
  } अन्यथा {
    // q = a - z;
    APFloat q = a;
    स्थितिः |= q.व्यवकलय(z, RM);

    // zz = q + c + (a - (q + z)) + aa + cc;
    // Compute a - (q + z) as -((q + z) - a) to avoid temporary copies.
    स्वचालित zz = q;
    स्थितिः |= zz.सङ्कलय(c, RM);
    स्थितिः |= q.सङ्कलय(z, RM);
    स्थितिः |= q.व्यवकलय(a, RM);
    q.changeSign();
    स्थितिः |= zz.सङ्कलय(q, RM);
    स्थितिः |= zz.सङ्कलय(aa, RM);
    स्थितिः |= zz.सङ्कलय(cc, RM);
    यदि (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ असत्य);
      प्रत्यागम opOK;
    }
    Floats[0] = z;
    स्थितिः |= Floats[0].सङ्कलय(zz, RM);
    यदि (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ असत्य);
      प्रत्यागम (op_स्थितिः)स्थितिः;
    }
    Floats[1] = std::move(z);
    स्थितिः |= Floats[1].व्यवकलय(Floats[0], RM);
    स्थितिः |= Floats[1].सङ्कलय(zz, RM);
  }
  प्रत्यागम (op_स्थितिः)स्थितिः;
}

APFloat::op_स्थितिः DoubleAPFloat::सङ्कलय_With_Special(ध्रुव DoubleAPFloat &LHS,
                                                ध्रुव DoubleAPFloat &RHS,
                                                DoubleAPFloat &बहिर्गमनम्,
                                                rounding_बहुलकम् RM) {
  यदि (LHS.getCategory() == fcNaN) {
    बहिर्गमनम् = LHS;
    प्रत्यागम opOK;
  }
  यदि (RHS.getCategory() == fcNaN) {
    बहिर्गमनम् = RHS;
    प्रत्यागम opOK;
  }
  यदि (LHS.getCategory() == fcZero) {
    बहिर्गमनम् = RHS;
    प्रत्यागम opOK;
  }
  यदि (RHS.getCategory() == fcZero) {
    बहिर्गमनम् = LHS;
    प्रत्यागम opOK;
  }
  यदि (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    बहिर्गमनम्.makeNaN(असत्य, बहिर्गमनम्.isNegative(), शून्यसूचकः);
    प्रत्यागम opInvalidOp;
  }
  यदि (LHS.getCategory() == fcInfinity) {
    बहिर्गमनम् = LHS;
    प्रत्यागम opOK;
  }
  यदि (RHS.getCategory() == fcInfinity) {
    बहिर्गमनम् = RHS;
    प्रत्यागम opOK;
  }
  प्रतिज्ञा(LHS.getCategory() == fc_प्रसामान्यम् && RHS.getCategory() == fc_प्रसामान्यम्);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  प्रतिज्ञा(&A.getSemantics() == &APFloat_आधारः::semIEEEdouble);
  प्रतिज्ञा(&AA.getSemantics() == &APFloat_आधारः::semIEEEdouble);
  प्रतिज्ञा(&C.getSemantics() == &APFloat_आधारः::semIEEEdouble);
  प्रतिज्ञा(&CC.getSemantics() == &APFloat_आधारः::semIEEEdouble);
  प्रतिज्ञा(&बहिर्गमनम्.Floats[0].getSemantics() == &APFloat_आधारः::semIEEEdouble);
  प्रतिज्ञा(&बहिर्गमनम्.Floats[1].getSemantics() == &APFloat_आधारः::semIEEEdouble);
  प्रत्यागम बहिर्गमनम्.सङ्कलय_Impl(A, AA, C, CC, RM);
}

APFloat::op_स्थितिः DoubleAPFloat::सङ्कलय(ध्रुव DoubleAPFloat &RHS,
                                     rounding_बहुलकम् RM) {
  प्रत्यागम सङ्कलय_With_Special(*एतत्, RHS, *एतत्, RM);
}

APFloat::op_स्थितिः DoubleAPFloat::व्यवकलय(ध्रुव DoubleAPFloat &RHS,
                                          rounding_बहुलकम् RM) {
  changeSign();
  स्वचालित Ret = सङ्कलय(RHS, RM);
  changeSign();
  प्रत्यागम Ret;
}

APFloat::op_स्थितिः DoubleAPFloat::multiply(ध्रुव DoubleAPFloat &RHS,
                                          APFloat::rounding_बहुलकम् RM) {
  ध्रुव स्वचालित &LHS = *एतत्;
  स्वचालित &बहिर्गमनम् = *एतत्;
  /* Interesting observation: For special categories, finding the lowest
     common ancestor of the following layered graph gives the correct
     return category:

        NaN
       /   \
     Zero  Inf
       \   /
प्रसामान्यम्

     e.g. NaN * NaN = NaN
          Zero * Inf = NaN
          Normal * Zero = Zero
          Normal * Inf = Inf
  */
  यदि (LHS.getCategory() == fcNaN) {
    बहिर्गमनम् = LHS;
    प्रत्यागम opOK;
  }
  यदि (RHS.getCategory() == fcNaN) {
    बहिर्गमनम् = RHS;
    प्रत्यागम opOK;
  }
  यदि ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    बहिर्गमनम्.makeNaN(असत्य, असत्य, शून्यसूचकः);
    प्रत्यागम opOK;
  }
  यदि (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    बहिर्गमनम् = LHS;
    प्रत्यागम opOK;
  }
  यदि (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    बहिर्गमनम् = RHS;
    प्रत्यागम opOK;
  }
  प्रतिज्ञा(LHS.getCategory() == fc_प्रसामान्यम् && RHS.getCategory() == fc_प्रसामान्यम् &&
         "Special cases not handled exhaustively");

  पूर्णाङ्क स्थितिः = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  स्थितिः |= T.multiply(C, RM);
  यदि (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ असत्य);
    प्रत्यागम (op_स्थितिः)स्थितिः;
  }

  // tau = fmsub(a, c, t), that is -fmadd(-a, c, t).
  APFloat Tau = A;
  T.changeSign();
  स्थितिः |= Tau.fused_Multiply_सङ्कलय(C, T, RM);
  T.changeSign();
  {
    // v = a * d
    APFloat V = A;
    स्थितिः |= V.multiply(D, RM);
    // w = b * c
    APFloat W = B;
    स्थितिः |= W.multiply(C, RM);
    स्थितिः |= V.सङ्कलय(W, RM);
    // tau += v + w
    स्थितिः |= Tau.सङ्कलय(V, RM);
  }
  // u = t + tau
  APFloat U = T;
  स्थितिः |= U.सङ्कलय(Tau, RM);

  Floats[0] = U;
  यदि (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ असत्य);
  } अन्यथा {
    // Floats[1] = (t - u) + tau
    स्थितिः |= T.व्यवकलय(U, RM);
    स्थितिः |= T.सङ्कलय(Tau, RM);
    Floats[1] = std::move(T);
  }
  प्रत्यागम (op_स्थितिः)स्थितिः;
}

APFloat::op_स्थितिः DoubleAPFloat::divide(ध्रुव DoubleAPFloat &RHS,
                                        APFloat::rounding_बहुलकम् RM) {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_आधारः::semPPCDoubleDoubleLegacy, bitcast_गन्तव्यम्_APInt());
  स्वचालित Ret = Tmp.divide(
      APFloat(APFloat_आधारः::semPPCDoubleDoubleLegacy, RHS.bitcast_गन्तव्यम्_APInt()), RM);
  *एतत् = DoubleAPFloat(APFloat_आधारः::semPPCDoubleDouble, Tmp.bitcast_गन्तव्यम्_APInt());
  प्रत्यागम Ret;
}

APFloat::op_स्थितिः DoubleAPFloat::remainder(ध्रुव DoubleAPFloat &RHS) {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_आधारः::semPPCDoubleDoubleLegacy, bitcast_गन्तव्यम्_APInt());
  स्वचालित Ret = Tmp.remainder(
      APFloat(APFloat_आधारः::semPPCDoubleDoubleLegacy, RHS.bitcast_गन्तव्यम्_APInt()));
  *एतत् = DoubleAPFloat(APFloat_आधारः::semPPCDoubleDouble, Tmp.bitcast_गन्तव्यम्_APInt());
  प्रत्यागम Ret;
}

APFloat::op_स्थितिः DoubleAPFloat::mod(ध्रुव DoubleAPFloat &RHS) {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_आधारः::semPPCDoubleDoubleLegacy, bitcast_गन्तव्यम्_APInt());
  स्वचालित Ret = Tmp.mod(
      APFloat(APFloat_आधारः::semPPCDoubleDoubleLegacy, RHS.bitcast_गन्तव्यम्_APInt()));
  *एतत् = DoubleAPFloat(APFloat_आधारः::semPPCDoubleDouble, Tmp.bitcast_गन्तव्यम्_APInt());
  प्रत्यागम Ret;
}

APFloat::op_स्थितिः
DoubleAPFloat::fused_Multiply_सङ्कलय(ध्रुव DoubleAPFloat &Multiplicand,
                                ध्रुव DoubleAPFloat &Addend,
                                APFloat::rounding_बहुलकम् RM) {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_आधारः::semPPCDoubleDoubleLegacy, bitcast_गन्तव्यम्_APInt());
  स्वचालित Ret = Tmp.fused_Multiply_सङ्कलय(
      APFloat(APFloat_आधारः::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcast_गन्तव्यम्_APInt()),
      APFloat(APFloat_आधारः::semPPCDoubleDoubleLegacy, Addend.bitcast_गन्तव्यम्_APInt()),
      RM);
  *एतत् = DoubleAPFloat(APFloat_आधारः::semPPCDoubleDouble, Tmp.bitcast_गन्तव्यम्_APInt());
  प्रत्यागम Ret;
}

APFloat::op_स्थितिः DoubleAPFloat::आवर्तनम्_गन्तव्यम्_Integral(APFloat::rounding_बहुलकम् RM) {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ध्रुव APFloat &Hi = getFirst();
  ध्रुव APFloat &Lo = getSecond();

  APFloat RoundedHi = Hi;
  ध्रुव op_स्थितिः Hi_स्थितिः = RoundedHi.आवर्तनम्_गन्तव्यम्_Integral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  यदि (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/असत्य);
    प्रत्यागम Hi_स्थितिः;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  स्वचालित आवर्तनम्_गन्तव्यम्_निकटतमः_Helper = [](APFloat गन्तव्यम्_आवर्तनम्, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    ध्रुव APFloat RoundingError = Rounded - गन्तव्यम्_आवर्तनम्;
    यदि (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        निरपेक्षम्(RoundingError).is_Exactly_मानम्(0.5))
      Rounded.सङ्कलय(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rm_निकटतमः_Ties_गन्तव्यम्_Even);
    प्रत्यागम Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  यदि (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    यदि (RM == rm_निकटतमः_Ties_गन्तव्यम्_Away || RM == rm_निकटतमः_Ties_गन्तव्यम्_Even)
      RoundedHi = आवर्तनम्_गन्तव्यम्_निकटतमः_Helper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/असत्य);
    प्रत्यागम Hi_स्थितिः;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  Rounding_बहुलकम् Lo_Rounding_बहुलकम्;
  यदि (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    Lo_Rounding_बहुलकम् = isNegative() ? rm_Toward_धनात्मकम् : rmTowardNegative;
  अन्यथा
    Lo_Rounding_बहुलकम् = RM;

  APFloat RoundedLo = Lo;
  ध्रुव op_स्थितिः Lo_स्थितिः = RoundedLo.आवर्तनम्_गन्तव्यम्_Integral(Lo_Rounding_बहुलकम्);
  यदि (Lo_Rounding_बहुलकम् == rm_निकटतमः_Ties_गन्तव्यम्_Away)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = आवर्तनम्_गन्तव्यम्_निकटतमः_Helper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fast_Two_योग(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  प्रत्यागम Lo_स्थितिः;
}

शून्य DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmp_परिणाम
DoubleAPFloat::compare_Absolute_मानम्(ध्रुव DoubleAPFloat &RHS) ध्रुव {
  // Compare absolute values of the high parts.
  ध्रुव cmp_परिणाम HiPartCmp = Floats[0].compare_Absolute_मानम्(RHS.Floats[0]);
  यदि (HiPartCmp != cmpEqual)
    प्रत्यागम HiPartCmp;

  // Zero, regardless of sign, is equal.
  यदि (Floats[1].isZero() && RHS.Floats[1].isZero())
    प्रत्यागम cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  ध्रुव तार्किक ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  ध्रुव तार्किक RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  यदि (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    प्रत्यागम RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  यदि (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    प्रत्यागम ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  यदि (ThisIsSubtractive != RHSIsSubtractive)
    प्रत्यागम ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  ध्रुव cmp_परिणाम LoPartCmp = Floats[1].compare_Absolute_मानम्(RHS.Floats[1]);

  यदि (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    यदि (LoPartCmp == cmpLessThan)
      प्रत्यागम cmpGreaterThan;
    यदि (LoPartCmp == cmpGreaterThan)
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
  Floats[1].makeZero(/* Neg = */ असत्य);
}

शून्य DoubleAPFloat::makeZero(तार्किक Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ असत्य);
}

शून्य DoubleAPFloat::makeLargest(तार्किक Neg) {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloat_आधारः::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloat_आधारः::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  यदि (Neg)
    changeSign();
}

शून्य DoubleAPFloat::makeSmallest(तार्किक Neg) {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].makeSmallest(Neg);
  Floats[1].makeZero(/* Neg = */ असत्य);
}

शून्य DoubleAPFloat::makeSmallestNormalized(तार्किक Neg) {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloat_आधारः::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  यदि (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ असत्य);
}

शून्य DoubleAPFloat::makeNaN(तार्किक SNaN, तार्किक Neg, ध्रुव APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ असत्य);
}

APFloat::cmp_परिणाम DoubleAPFloat::compare(ध्रुव DoubleAPFloat &RHS) ध्रुव {
  स्वचालित परिणाम = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  यदि (परिणाम == APFloat::cmpEqual)
    प्रत्यागम Floats[1].compare(RHS.Floats[1]);
  प्रत्यागम परिणाम;
}

तार्किक DoubleAPFloat::bitwiseIsEqual(ध्रुव DoubleAPFloat &RHS) ध्रुव {
  प्रत्यागम Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_संहिता hash_मानम्(ध्रुव DoubleAPFloat &Arg) {
  यदि (Arg.Floats)
    प्रत्यागम hash_combine(hash_मानम्(Arg.Floats[0]), hash_मानम्(Arg.Floats[1]));
  प्रत्यागम hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcast_गन्तव्यम्_APInt() ध्रुव {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t दत्त_2[] = {
      Floats[0].bitcast_गन्तव्यम्_APInt().get_Raw_दत्त()[0],
      Floats[1].bitcast_गन्तव्यम्_APInt().get_Raw_दत्त()[0],
  };
  प्रत्यागम APInt(128, दत्त_2);
}

Expected<APFloat::op_स्थितिः> DoubleAPFloat::convertFromString(StringRef S,
                                                             rounding_बहुलकम् RM) {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloat_आधारः::semPPCDoubleDoubleLegacy);
  स्वचालित Ret = Tmp.convertFromString(S, RM);
  *एतत् = DoubleAPFloat(APFloat_आधारः::semPPCDoubleDouble, Tmp.bitcast_गन्तव्यम्_APInt());
  प्रत्यागम Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::op_स्थितिः DoubleAPFloat::अग्र(तार्किक अग्र_अधस्तात्) {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  यदि (अग्र_अधस्तात्) {
    changeSign();
    APFloat::op_स्थितिः परिणाम = अग्र(/*nextDown=*/असत्य);
    changeSign();
    प्रत्यागम परिणाम;
  }
  विकल्प (getCategory()) {
  पक्ष fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    यदि (isNegative())
      makeLargest(सत्य);
    प्रत्यागम opOK;

  पक्ष fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    यदि (getFirst().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(असत्य, isNegative(), शून्यसूचकः);
      प्रत्यागम opInvalidOp;
    }
    प्रत्यागम opOK;

  पक्ष fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(असत्य);
    प्रत्यागम opOK;

  पक्ष fc_प्रसामान्यम्:
    भङ्ग;
  }

  ध्रुव APFloat &HiOld = getFirst();
  ध्रुव APFloat &LoOld = getSecond();

  APFloat अग्र_Lo = LoOld;
  अग्र_Lo.अग्र(/*nextDown=*/असत्य);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  स्वचालित अन्तर्_Lattice = [](ध्रुव APFloat &Hi, ध्रुव APFloat &Lo) {
    प्रत्यागम Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  यदि (अन्तर्_Lattice(HiOld, अग्र_Lo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(अग्र_Lo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat Largest{*Semantics, uninitialized};
    Largest.makeLargest(/*Neg=*/असत्य);
    यदि (compare(Largest) == cmpGreaterThan)
      makeInf(/*Neg=*/असत्य);

    प्रत्यागम opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat अग्र_Hi = HiOld;
  अग्र_Hi.अग्र(/*nextDown=*/असत्य);

  // nextUp(getLargest()) == INFINITY
  यदि (अग्र_Hi.isInfinity()) {
    makeInf(/*Neg=*/असत्य);
    प्रत्यागम opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  यदि (अग्र_Hi.isZero()) {
    makeZero(/*Neg=*/सत्य);
    प्रत्यागम opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  अग्र_Lo = neg(scalbn(harrisonUlp(अग्र_Hi), -1, rmTowardZero));
  यदि (!अन्तर्_Lattice(अग्र_Hi, अग्र_Lo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    अग्र_Lo.अग्र(/*nextDown=*/असत्य);

  Floats[0] = std::move(अग्र_Hi);
  Floats[1] = std::move(अग्र_Lo);

  प्रत्यागम opOK;
}

APFloat::op_स्थितिः DoubleAPFloat::convert_गन्तव्यम्_Sign_Extended_Integer(
    MutableArrayRef<integerPart> निवेशः, अचिह्नित पूर्णाङ्क विस्तारः, तार्किक IsSigned,
    rounding_बहुलकम् RM, तार्किक *IsExact) ध्रुव {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  यदि (!getFirst().isFiniteNonZero() || getSecond().isZero())
    प्रत्यागम getFirst().convert_गन्तव्यम्_Integer(निवेशः, विस्तारः, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = असत्य;
  DoubleAPFloat Integral = *एतत्;
  ध्रुव op_स्थितिः आवर्तनम्_स्थितिः = Integral.आवर्तनम्_गन्तव्यम्_Integral(RM);
  यदि (आवर्तनम्_स्थितिः == opInvalidOp)
    प्रत्यागम opInvalidOp;
  ध्रुव APFloat &IntegralHi = Integral.getFirst();
  ध्रुव APFloat &IntegralLo = Integral.getSecond();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  तार्किक HiIsExact;
  यदि (IntegralHi.isZero() || IntegralLo.isZero()) {
    ध्रुव op_स्थितिः Hi_स्थितिः =
        IntegralHi.convert_गन्तव्यम्_Integer(निवेशः, विस्तारः, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    प्रत्यागम Hi_स्थितिः == opOK ? opInexact : Hi_स्थितिः;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  यदि (!IsSigned && IntegralHi.isNegative())
    प्रत्यागम opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  तार्किक LoIsExact;
  ध्रुव पूर्णाङ्क HiExactLog2 = IntegralHi.getExactLog2Abs();
  ध्रुव अचिह्नित धनात्मकम्_अतिप्रवाहः_विस्तारः = IsSigned ? विस्तारः - 1 : विस्तारः;
  यदि (HiExactLog2 >= 0 &&
      static_cast<अचिह्नित>(HiExactLog2) == धनात्मकम्_अतिप्रवाहः_विस्तारः) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    यदि (IntegralHi.isNegative() == IntegralLo.isNegative())
      प्रत्यागम opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    ध्रुव op_स्थितिः Lo_स्थितिः = IntegralLo.convert_गन्तव्यम्_Integer(
        निवेशः, विस्तारः, /*IsSigned=*/सत्य, RM, &LoIsExact);
    यदि (Lo_स्थितिः == opInvalidOp)
      प्रत्यागम opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    यदि (IsSigned && !IntegralHi.isNegative())
      APInt::tcClearBit(निवेशः.दत्त(), धनात्मकम्_अतिप्रवाहः_विस्तारः);
    *IsExact = आवर्तनम्_स्थितिः == opOK;
    प्रत्यागम आवर्तनम्_स्थितिः;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  ध्रुव op_स्थितिः Hi_स्थितिः = IntegralHi.convert_गन्तव्यम्_Integer(
      निवेशः, विस्तारः, IsSigned, rmTowardZero, &HiIsExact);
  यदि (Hi_स्थितिः == opInvalidOp)
    प्रत्यागम Hi_स्थितिः;

  // Convert Lo into a temporary integer of the same width.
  APSInt Lo_परिणाम{विस्तारः, /*isUnsigned=*/!IsSigned};
  ध्रुव op_स्थितिः Lo_स्थितिः =
      IntegralLo.convert_गन्तव्यम्_Integer(Lo_परिणाम, rmTowardZero, &LoIsExact);
  यदि (Lo_स्थितिः == opInvalidOp)
    प्रत्यागम Lo_स्थितिः;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tc_सङ्कलय(निवेशः.दत्त(), Lo_परिणाम.get_Raw_दत्त(), /*carry=*/0, निवेशः.आकार_2());

  *IsExact = आवर्तनम्_स्थितिः == opOK;
  प्रत्यागम आवर्तनम्_स्थितिः;
}

APFloat::op_स्थितिः
DoubleAPFloat::convert_गन्तव्यम्_Integer(MutableArrayRef<integerPart> निवेशः,
                                अचिह्नित पूर्णाङ्क विस्तारः, तार्किक IsSigned,
                                rounding_बहुलकम् RM, तार्किक *IsExact) ध्रुव {
  op_स्थितिः FS =
      convert_गन्तव्यम्_Sign_Extended_Integer(निवेशः, विस्तारः, IsSigned, RM, IsExact);

  यदि (FS == opInvalidOp) {
    ध्रुव अचिह्नित Dst_Parts_गणनम् = part_गणनम्_For_द्वयङ्काः(विस्तारः);
    प्रतिज्ञा(Dst_Parts_गणनम् <= निवेशः.आकार_2() && "Integer too big");

    अचिह्नित द्वयङ्काः_2;
    यदि (getCategory() == fcNaN)
      द्वयङ्काः_2 = 0;
    अन्यथा यदि (isNegative())
      द्वयङ्काः_2 = IsSigned;
    अन्यथा
      द्वयङ्काः_2 = विस्तारः - IsSigned;

    tc_समुच्चयः_न्यूनतमम्_Significant_द्वयङ्काः(निवेशः.दत्त(), Dst_Parts_गणनम्, द्वयङ्काः_2);
    यदि (isNegative() && IsSigned)
      APInt::tc_सरणम्_वामः(निवेशः.दत्त(), Dst_Parts_गणनम्, विस्तारः - 1);
  }

  प्रत्यागम FS;
}

APFloat::op_स्थितिः DoubleAPFloat::handle_अतिप्रवाहः(rounding_बहुलकम् RM) {
  विकल्प (RM) {
  पक्ष APFloat::rmTowardZero:
    makeLargest(/*Neg=*/isNegative());
    भङ्ग;
  पक्ष APFloat::rmTowardNegative:
    यदि (isNegative())
      makeInf(/*Neg=*/सत्य);
    अन्यथा
      makeLargest(/*Neg=*/असत्य);
    भङ्ग;
  पक्ष APFloat::rm_Toward_धनात्मकम्:
    यदि (isNegative())
      makeLargest(/*Neg=*/सत्य);
    अन्यथा
      makeInf(/*Neg=*/असत्य);
    भङ्ग;
  पक्ष APFloat::rm_निकटतमः_Ties_गन्तव्यम्_Away:
  पक्ष APFloat::rm_निकटतमः_Ties_गन्तव्यम्_Even:
    makeInf(/*Neg=*/isNegative());
    भङ्ग;
  स्वभाव:
    llvm_unreachable("Invalid rounding mode found");
  }
  op_स्थितिः S = opInexact;
  यदि (!getFirst().isFinite())
    S = static_cast<op_स्थितिः>(S | op_अतिप्रवाहः);
  प्रत्यागम S;
}

APFloat::op_स्थितिः DoubleAPFloat::convertFromUnsignedParts(
    ध्रुव integerPart *Src, अचिह्नित पूर्णाङ्क Src_गणनम्, rounding_बहुलकम् RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  ध्रुव अचिह्नित SrcMSB = APInt::tcMSB(Src, Src_गणनम्);
  यदि (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/असत्य);
    प्रत्यागम opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  ध्रुव अचिह्नित Src_Bit_विस्तारः = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/Src_Bit_विस्तारः, ArrayRef(Src, Src_गणनम्)},
                /*isUnsigned=*/सत्य};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{getFirst().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/असत्य, rm_निकटतमः_Ties_गन्तव्यम्_Even);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  यदि (!Hi.isFinite())
    प्रत्यागम handle_अतिप्रवाहः(RM);

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
  Hi.convert_गन्तव्यम्_Integer(HiAsInt, rm_निकटतमः_Ties_गन्तव्यम्_Even, &HiAsIntIsExact);
  ध्रुव APInt Error = SrcInt.zext(HiAsInt.get_Bit_विस्तारः()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  rounding_बहुलकम् LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  यदि (Error.isNegative()) {
    यदि (RM == rm_निकटतमः_Ties_गन्तव्यम्_Away) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      ध्रुव अचिह्नित Error_सक्रिय_द्वयङ्काः = Error.get_Significant_द्वयङ्काः() - 1;
      ध्रुव अचिह्नित LoPrecision = getSecond().getSemantics().precision;
      यदि (Error_सक्रिय_द्वयङ्काः > LoPrecision) {
        ध्रुव अचिह्नित RoundingBoundary = Error_सक्रिय_द्वयङ्काः - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        यदि (Error.गणनम्_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rm_Toward_धनात्मकम्;
      }
    } अन्यथा यदि (RM == rmTowardZero) {
      // For rmTowardZero, the final positive result must be truncated (rounded
      // down). When Hi is an overestimate, Error is negative. A standard
      // rmTowardZero conversion of Error would make it *less* negative,
      // effectively rounding the final sum Hi + Lo *up*. To ensure the sum
      // rounds down correctly, we force Lo to round toward -infinity.
      LoRM = rmTowardNegative;
    }
  }

  APFloat Lo{getSecond().getSemantics()};
  op_स्थितिः स्थितिः = Lo.convertFromAPInt(Error, /*IsSigned=*/सत्य, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fast_Two_योग(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  यदि (!getFirst().isFinite())
    प्रत्यागम handle_अतिप्रवाहः(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  यदि (getFirst().isFiniteNonZero() && Floats[0].isLargest()) {
    DoubleAPFloat Largest{*Semantics};
    Largest.makeLargest(/*Neg=*/असत्य);
    यदि (compare(Largest) == APFloat::cmpGreaterThan)
      प्रत्यागम handle_अतिप्रवाहः(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  प्रत्यागम स्थितिः;
}

APFloat::op_स्थितिः DoubleAPFloat::convertFromAPInt(ध्रुव APInt &निवेशः,
                                                  तार्किक IsSigned,
                                                  rounding_बहुलकम् RM) {
  ध्रुव तार्किक Negate_निवेशः = IsSigned && निवेशः.isNegative();
  APInt API = निवेशः;
  यदि (Negate_निवेशः)
    API.negate();

  ध्रुव APFloat::op_स्थितिः स्थितिः =
      convertFromUnsignedParts(API.get_Raw_दत्त(), API.get_अङ्कः_शब्दाः(), RM);
  यदि (Negate_निवेशः)
    changeSign();
  प्रत्यागम स्थितिः;
}

अचिह्नित पूर्णाङ्क DoubleAPFloat::convert_गन्तव्यम्_Hex_String(वर्ण *DST,
                                               अचिह्नित पूर्णाङ्क HexDigits,
                                               तार्किक ऊर्ध्वस्थम्_Case_2,
                                               rounding_बहुलकम् RM) ध्रुव {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble &&
         "Unexpected Semantics");
  प्रत्यागम APFloat(APFloat_आधारः::semPPCDoubleDoubleLegacy, bitcast_गन्तव्यम्_APInt())
      .convert_गन्तव्यम्_Hex_String(DST, HexDigits, ऊर्ध्वस्थम्_Case_2, RM);
}

तार्किक DoubleAPFloat::isDenormal() ध्रुव {
  प्रत्यागम getCategory() == fc_प्रसामान्यम् &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

तार्किक DoubleAPFloat::isSmallest() ध्रुव {
  यदि (getCategory() != fc_प्रसामान्यम्)
    प्रत्यागम असत्य;
  DoubleAPFloat Tmp(*एतत्);
  Tmp.makeSmallest(एतत्->isNegative());
  प्रत्यागम Tmp.compare(*एतत्) == cmpEqual;
}

तार्किक DoubleAPFloat::isSmallestNormalized() ध्रुव {
  यदि (getCategory() != fc_प्रसामान्यम्)
    प्रत्यागम असत्य;

  DoubleAPFloat Tmp(*एतत्);
  Tmp.makeSmallestNormalized(एतत्->isNegative());
  प्रत्यागम Tmp.compare(*एतत्) == cmpEqual;
}

तार्किक DoubleAPFloat::isLargest() ध्रुव {
  यदि (getCategory() != fc_प्रसामान्यम्)
    प्रत्यागम असत्य;
  DoubleAPFloat Tmp(*एतत्);
  Tmp.makeLargest(एतत्->isNegative());
  प्रत्यागम Tmp.compare(*एतत्) == cmpEqual;
}

तार्किक DoubleAPFloat::isInteger() ध्रुव {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble &&
         "Unexpected Semantics");
  प्रत्यागम Floats[0].isInteger() && Floats[1].isInteger();
}

शून्य DoubleAPFloat::गन्तव्यम्_String(Small_सदिशम्_Impl<वर्ण> &Str,
                             अचिह्नित FormatPrecision,
                             अचिह्नित FormatMaxPadding,
                             तार्किक TruncateZero) ध्रुव {
  प्रतिज्ञा(Semantics == &APFloat_आधारः::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloat_आधारः::semPPCDoubleDoubleLegacy, bitcast_गन्तव्यम्_APInt())
      .गन्तव्यम्_String(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

पूर्णाङ्क DoubleAPFloat::getExactLog2Abs() ध्रुव {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  यदि (getSecond().isNonZero())
    प्रत्यागम INT_MIN;
  प्रत्यागम getFirst().getExactLog2Abs();
}

पूर्णाङ्क ilogb(ध्रुव DoubleAPFloat &Arg) {
  ध्रुव APFloat &Hi = Arg.getFirst();
  ध्रुव APFloat &Lo = Arg.getSecond();
  पूर्णाङ्क Ilogb_परिणाम = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  यदि (Arg.getCategory() != fc_प्रसामान्यम्)
    प्रत्यागम Ilogb_परिणाम;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  यदि (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    प्रत्यागम Ilogb_परिणाम;
  यदि (Hi.getExactLog2Abs() == INT_MIN)
    प्रत्यागम Ilogb_परिणाम;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  प्रत्यागम Ilogb_परिणाम - 1;
}

DoubleAPFloat scalbn(ध्रुव DoubleAPFloat &Arg, पूर्णाङ्क Exp,
                     APFloat::rounding_बहुलकम् RM) {
  प्रतिज्ञा(Arg.Semantics == &APFloat_आधारः::PPCDoubleDouble() &&
         "Unexpected Semantics");
  प्रत्यागम DoubleAPFloat(APFloat_आधारः::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(ध्रुव DoubleAPFloat &Arg, पूर्णाङ्क &Exp,
                    APFloat::rounding_बहुलकम् RM) {
  प्रतिज्ञा(Arg.Semantics == &APFloat_आधारः::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  यदि (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.getFirst() = Quiet.getFirst().makeQuiet();
    प्रत्यागम Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  यदि (Exp == APFloat::IEK_Inf)
    प्रत्यागम Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  यदि (Exp == APFloat::IEK_Zero) {
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
  यदि (Arg.getCategory() == APFloat::fc_प्रसामान्यम् && Lo.isFiniteNonZero()) {
    rounding_बहुलकम् Lo_Rounding_बहुलकम्;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    यदि (RM == rmTowardZero)
      Lo_Rounding_बहुलकम् = Arg.isNegative() ? rm_Toward_धनात्मकम् : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    अन्यथा यदि (RM == rm_निकटतमः_Ties_गन्तव्यम्_Away && SignsDisagree && Exp > 0)
      Lo_Rounding_बहुलकम् = rm_निकटतमः_Ties_गन्तव्यम्_Even;
    अन्यथा
      Lo_Rounding_बहुलकम् = RM;
    Second = scalbn(Lo, -Exp, Lo_Rounding_बहुलकम्);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    यदि (RM == rm_निकटतमः_Ties_गन्तव्यम्_Away && Lo_Rounding_बहुलकम् == rm_निकटतमः_Ties_गन्तव्यम्_Even) {
      // Re-scale the result back to check if rounding occurred.
      ध्रुव APFloat RecomposedLo = scalbn(Second, Exp, rm_निकटतमः_Ties_गन्तव्यम्_Even);
      यदि (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        ध्रुव APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        ध्रुव APFloat Ulp_सम्बन्धि_Second = harrisonUlp(Second);
        ध्रुव APFloat Scaled_Ulp_सम्बन्धि_Second =
            scalbn(Ulp_सम्बन्धि_Second, Exp - 1, rm_निकटतमः_Ties_गन्तव्यम्_Even);
        ध्रुव तार्किक IsMidpoint = निरपेक्षम्(RoundingError) == Scaled_Ulp_सम्बन्धि_Second;
        ध्रुव तार्किक RoundedLoAway =
            Second.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        यदि (IsMidpoint && RoundedLoAway)
          Second.अग्र(/*nextDown=*/!Second.isNegative());
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
    यदि (Second.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat First = scalbn(Hi, -Exp, RM);
  प्रत्यागम DoubleAPFloat(APFloat_आधारः::PPCDoubleDouble(), std::move(First),
                       std::move(Second));
}

APInt DoubleAPFloat::getNaNPayload() ध्रुव { प्रत्यागम Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, ध्रुव fltSemantics &Semantics) {
  यदि (उपयुङ्क्ते_Layout<IEEEFloat>(Semantics)) {
    नूतन (&IEEE) IEEEFloat(std::move(F));
    प्रत्यागम;
  }
  यदि (उपयुङ्क्ते_Layout<DoubleAPFloat>(Semantics)) {
    ध्रुव fltSemantics& S = F.getSemantics();
    नूतन (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloat_आधारः::IEEEdouble()));
    प्रत्यागम;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::op_स्थितिः> APFloat::convertFromString(StringRef Str,
                                                       rounding_बहुलकम् RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_संहिता hash_मानम्(ध्रुव APFloat &Arg) {
  यदि (APFloat::उपयुङ्क्ते_Layout<detail::IEEEFloat>(Arg.getSemantics()))
    प्रत्यागम hash_मानम्(Arg.U.IEEE);
  यदि (APFloat::उपयुङ्क्ते_Layout<detail::DoubleAPFloat>(Arg.getSemantics()))
    प्रत्यागम hash_मानम्(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(ध्रुव fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  स्वचालित स्थितिः_Or_Err = convertFromString(S, rm_निकटतमः_Ties_गन्तव्यम्_Even);
  प्रतिज्ञा(स्थितिः_Or_Err && "Invalid floating point representation");
  consumeError(स्थितिः_Or_Err.takeError());
}

FPClassTest APFloat::classify() ध्रुव {
  यदि (isZero())
    प्रत्यागम isNegative() ? fcNegZero : fcPosZero;
  यदि (is_प्रसामान्यम्())
    प्रत्यागम isNegative() ? fc_Neg_प्रसामान्यम् : fc_Pos_प्रसामान्यम्;
  यदि (isDenormal())
    प्रत्यागम isNegative() ? fcNegSubnormal : fcPosSubnormal;
  यदि (isInfinity())
    प्रत्यागम isNegative() ? fcNegInf : fcPosInf;
  प्रतिज्ञा(isNaN() && "Other class of FP constant");
  प्रत्यागम isSignaling() ? fcSNan : fcQNan;
}

तार्किक APFloat::getExactInverse(APFloat *Inv) ध्रुव {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  यदि (!isFiniteNonZero())
    प्रत्यागम असत्य;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  यदि (isDenormal())
    प्रत्यागम असत्य;

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
  यदि (Exp == INT_MIN)
    प्रत्यागम असत्य;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  यदि (Reciprocal.getExactLog2Abs() != -Exp)
    प्रत्यागम असत्य;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  यदि (Reciprocal.isDenormal())
    प्रत्यागम असत्य;

  प्रतिज्ञा(Reciprocal.isFiniteNonZero());

  यदि (Inv)
    *Inv = std::move(Reciprocal);

  प्रत्यागम सत्य;
}

APFloat::op_स्थितिः APFloat::convert(ध्रुव fltSemantics &गन्तव्यम्_Semantics_2,
                                   rounding_बहुलकम् RM, तार्किक *losesInfo) {
  यदि (&getSemantics() == &गन्तव्यम्_Semantics_2) {
    *losesInfo = असत्य;
    प्रत्यागम opOK;
  }
  यदि (उपयुङ्क्ते_Layout<IEEEFloat>(getSemantics()) &&
      उपयुङ्क्ते_Layout<IEEEFloat>(गन्तव्यम्_Semantics_2))
    प्रत्यागम U.IEEE.convert(गन्तव्यम्_Semantics_2, RM, losesInfo);
  यदि (उपयुङ्क्ते_Layout<IEEEFloat>(getSemantics()) &&
      उपयुङ्क्ते_Layout<DoubleAPFloat>(गन्तव्यम्_Semantics_2)) {
    प्रतिज्ञा(&गन्तव्यम्_Semantics_2 == &APFloat_आधारः::semPPCDoubleDouble);
    स्वचालित Ret =
        U.IEEE.convert(APFloat_आधारः::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *एतत् = APFloat(गन्तव्यम्_Semantics_2, U.IEEE.bitcast_गन्तव्यम्_APInt());
    प्रत्यागम Ret;
  }
  यदि (उपयुङ्क्ते_Layout<DoubleAPFloat>(getSemantics()) &&
      उपयुङ्क्ते_Layout<IEEEFloat>(गन्तव्यम्_Semantics_2)) {
    स्वचालित Ret = getIEEE().convert(गन्तव्यम्_Semantics_2, RM, losesInfo);
    *एतत् = APFloat(std::move(getIEEE()), गन्तव्यम्_Semantics_2);
    प्रत्यागम Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_मानम्(ध्रुव fltSemantics &Semantics) {
  प्रत्यागम APFloat(Semantics, APInt::getAllOnes(Semantics.आकार_अन्तर्_द्वयङ्काः));
}

शून्य APFloat::print(raw_ostream &OS) ध्रुव {
  Small_सदिशम्<वर्ण, 16> बफर;
  गन्तव्यम्_String(बफर);
  OS << बफर;
}

#यदि !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_विधानम् शून्य APFloat::dump() ध्रुव {
  print(dbgs());
  dbgs() << '\n';
}
#यदि_अन्तः

शून्य APFloat::Profile(Folding_समुच्चयः_ग्रन्थिः_ID &NID) ध्रुव {
  NID.सङ्कलय_2(bitcast_गन्तव्यम्_APInt());
}

APFloat::op_स्थितिः APFloat::convert_गन्तव्यम्_Integer(APSInt &परिणाम_2,
                                            rounding_बहुलकम् rounding_बहुलकम्_2,
                                            तार्किक *isExact) ध्रुव {
  अचिह्नित bit_विस्तारः = परिणाम_2.get_Bit_विस्तारः();
  Small_सदिशम्<uint64_t, 4> parts(परिणाम_2.get_अङ्कः_शब्दाः());
  op_स्थितिः स्थितिः_2 = convert_गन्तव्यम्_Integer(parts, bit_विस्तारः, परिणाम_2.isSigned(),
                                     rounding_बहुलकम्_2, isExact);
  // Keeps the original signed-ness.
  परिणाम_2 = APInt(bit_विस्तारः, parts);
  प्रत्यागम स्थितिः_2;
}

द्विगुण APFloat::convert_गन्तव्यम्_Double() ध्रुव {
  यदि (&getSemantics() == &APFloat_आधारः::semIEEEdouble)
    प्रत्यागम getIEEE().convert_गन्तव्यम्_Double();
  प्रतिज्ञा(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat अस्थायी = *एतत्;
  तार्किक LosesInfo;
  [[maybe_अप्रयुक्तम्]] op_स्थितिः St =
      अस्थायी.convert(APFloat_आधारः::semIEEEdouble, rm_निकटतमः_Ties_गन्तव्यम्_Even, &LosesInfo);
  प्रतिज्ञा(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  प्रत्यागम अस्थायी.getIEEE().convert_गन्तव्यम्_Double();
}

#यदि_परिभाषितम् HAS_IEE754_FLOAT128
float128 APFloat::convert_गन्तव्यम्_Quad() ध्रुव {
  यदि (&getSemantics() == &APFloat_आधारः::semIEEEquad)
    प्रत्यागम getIEEE().convert_गन्तव्यम्_Quad();
  प्रतिज्ञा(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat अस्थायी = *एतत्;
  तार्किक LosesInfo;
  [[maybe_अप्रयुक्तम्]] op_स्थितिः St =
      अस्थायी.convert(APFloat_आधारः::semIEEEquad, rm_निकटतमः_Ties_गन्तव्यम्_Even, &LosesInfo);
  प्रतिज्ञा(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  प्रत्यागम अस्थायी.getIEEE().convert_गन्तव्यम्_Quad();
}
#यदि_अन्तः

भिन्न APFloat::convert_गन्तव्यम्_Float() ध्रुव {
  यदि (&getSemantics() == &APFloat_आधारः::semIEEEsingle)
    प्रत्यागम getIEEE().convert_गन्तव्यम्_Float();
  प्रतिज्ञा(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat अस्थायी = *एतत्;
  तार्किक LosesInfo;
  [[maybe_अप्रयुक्तम्]] op_स्थितिः St =
      अस्थायी.convert(APFloat_आधारः::semIEEEsingle, rm_निकटतमः_Ties_गन्तव्यम्_Even, &LosesInfo);
  प्रतिज्ञा(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  प्रत्यागम अस्थायी.getIEEE().convert_गन्तव्यम्_Float();
}

अचिह्नित APFloat_आधारः::get_Arbitrary_FPFormat_आकार_अन्तर्_द्वयङ्काः(StringRef Format) {
  प्रत्यागम StringSwitch<अचिह्नित>(Format)
      .Case("Float8E5M2", get_आकार_अन्तर्_द्वयङ्काः(semFloat8E5M2))
      .Case("Float8E5M2FNUZ", get_आकार_अन्तर्_द्वयङ्काः(semFloat8E5M2FNUZ))
      .Case("Float8E4M3", get_आकार_अन्तर्_द्वयङ्काः(semFloat8E4M3))
      .Case("Float8E4M3FN", get_आकार_अन्तर्_द्वयङ्काः(semFloat8E4M3FN))
      .Case("Float8E4M3FNUZ", get_आकार_अन्तर्_द्वयङ्काः(semFloat8E4M3FNUZ))
      .Case("Float8E4M3B11FNUZ", get_आकार_अन्तर्_द्वयङ्काः(semFloat8E4M3B11FNUZ))
      .Case("Float8E3M4", get_आकार_अन्तर्_द्वयङ्काः(semFloat8E3M4))
      .Case("Float8E8M0FNU", get_आकार_अन्तर्_द्वयङ्काः(semFloat8E8M0FNU))
      .Case("Float6E3M2FN", get_आकार_अन्तर्_द्वयङ्काः(semFloat6E3M2FN))
      .Case("Float6E2M3FN", get_आकार_अन्तर्_द्वयङ्काः(semFloat6E2M3FN))
      .Case("Float4E2M1FN", get_आकार_अन्तर्_द्वयङ्काः(semFloat4E2M1FN))
      .Case("Float8E5M3FNU", get_आकार_अन्तर्_द्वयङ्काः(semFloat8E5M3FNU))
      .Default(0);
}

तार्किक APFloat_आधारः::isValidArbitraryFPFormat(StringRef Format) {
  प्रत्यागम get_Arbitrary_FPFormat_आकार_अन्तर्_द्वयङ्काः(Format) != 0;
}

ध्रुव fltSemantics *APFloat_आधारः::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  प्रत्यागम StringSwitch<ध्रुव fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(शून्यसूचकः);
}

APFloat::Storage::~Storage() {
  यदि (उपयुङ्क्ते_Layout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    प्रत्यागम;
  }
  यदि (उपयुङ्क्ते_Layout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    प्रत्यागम;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(ध्रुव APFloat::Storage &RHS) {
  यदि (उपयुङ्क्ते_Layout<IEEEFloat>(*RHS.semantics)) {
    नूतन (एतत्) IEEEFloat(RHS.IEEE);
    प्रत्यागम;
  }
  यदि (उपयुङ्क्ते_Layout<DoubleAPFloat>(*RHS.semantics)) {
    नूतन (एतत्) DoubleAPFloat(RHS.Double);
    प्रत्यागम;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  यदि (उपयुङ्क्ते_Layout<IEEEFloat>(*RHS.semantics)) {
    नूतन (एतत्) IEEEFloat(std::move(RHS.IEEE));
    प्रत्यागम;
  }
  यदि (उपयुङ्क्ते_Layout<DoubleAPFloat>(*RHS.semantics)) {
    नूतन (एतत्) DoubleAPFloat(std::move(RHS.Double));
    प्रत्यागम;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::संक्रियकः=(ध्रुव APFloat::Storage &RHS) {
  यदि (उपयुङ्क्ते_Layout<IEEEFloat>(*semantics) &&
      उपयुङ्क्ते_Layout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } अन्यथा यदि (उपयुङ्क्ते_Layout<DoubleAPFloat>(*semantics) &&
             उपयुङ्क्ते_Layout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } अन्यथा यदि (एतत् != &RHS) {
    एतत्->~Storage();
    नूतन (एतत्) Storage(RHS);
  }
  प्रत्यागम *एतत्;
}

APFloat::Storage &APFloat::Storage::संक्रियकः=(APFloat::Storage &&RHS) {
  यदि (उपयुङ्क्ते_Layout<IEEEFloat>(*semantics) &&
      उपयुङ्क्ते_Layout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } अन्यथा यदि (उपयुङ्क्ते_Layout<DoubleAPFloat>(*semantics) &&
             उपयुङ्क्ते_Layout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } अन्यथा यदि (एतत् != &RHS) {
    एतत्->~Storage();
    नूतन (एतत्) Storage(std::move(RHS));
  }
  प्रत्यागम *एतत्;
}

नामाकाश {

APFloat::op_स्थितिः get_Op_स्थितिः_From_Libc(पूर्णाङ्क libc_exceptions) {
  APFloat::op_स्थितिः स्थितिः_2 = APFloat::opOK;
  यदि (libc_exceptions & FE_INVALID)
    स्थितिः_2 = static_cast<APFloat::op_स्थितिः>(स्थितिः_2 | APFloat::opInvalidOp);
  यदि (libc_exceptions & FE_DIVBYZERO)
    स्थितिः_2 = static_cast<APFloat::op_स्थितिः>(स्थितिः_2 | APFloat::opDivByZero);
  यदि (libc_exceptions & FE_अतिप्रवाहः)
    स्थितिः_2 = static_cast<APFloat::op_स्थितिः>(स्थितिः_2 | APFloat::op_अतिप्रवाहः);
  यदि (libc_exceptions & FE_UNDERFLOW)
    स्थितिः_2 = static_cast<APFloat::op_स्थितिः>(स्थितिः_2 | APFloat::opUnderflow);
  यदि (libc_exceptions & FE_INEXACT)
    स्थितिः_2 = static_cast<APFloat::op_स्थितिः>(स्थितिः_2 | APFloat::opInexact);
  प्रत्यागम स्थितिः_2;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> घातांकः(ध्रुव APFloat &x, Rounding_बहुलकम् rounding_बहुलकम्_2,
                           APFloat::op_स्थितिः *स्थितिः_2) {

  यदि (rounding_बहुलकम्_2 == APFloat_आधारः::rm_निकटतमः_Ties_गन्तव्यम्_Even) {
    यदि (APFloat::Semantics_गन्तव्यम्_Enum(x.getSemantics()) ==
        APFloat_आधारः::S_IEEEsingle) {
      भिन्न x_val = x.convert_गन्तव्यम्_Float();
      पूर्णाङ्क exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      यदि (स्थितिः_2) {
        *स्थितिः_2 = get_Op_स्थितिः_From_Libc(exc);
        यदि (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *स्थितिः_2 =
              static_cast<APFloat::op_स्थितिः>(*स्थितिः_2 | APFloat::opInvalidOp);
        }
      }
      भिन्न परिणाम_2 = LIBC_NAMESPACE::shared::expf(x_val);
      प्रत्यागम APFloat(परिणाम_2);
    }
    यदि (APFloat::Semantics_गन्तव्यम्_Enum(x.getSemantics()) ==
        APFloat_आधारः::S_IEEEdouble) {
      द्विगुण x_val = x.convert_गन्तव्यम्_Double();
      पूर्णाङ्क exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      यदि (स्थितिः_2) {
        *स्थितिः_2 = get_Op_स्थितिः_From_Libc(exc);
        यदि (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *स्थितिः_2 =
              static_cast<APFloat::op_स्थितिः>(*स्थितिः_2 | APFloat::opInvalidOp);
        }
      }
      द्विगुण परिणाम_2 = LIBC_NAMESPACE::shared::घातांकः(x_val);
      प्रत्यागम APFloat(परिणाम_2);
    }
  }
  प्रत्यागम std::nullopt;
}

} // namespace llvm

#अपरिभाषय APFLOAT_DISPATCH_ON_SEMANTICS
