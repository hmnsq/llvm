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
  गर {                                                                         \
    यदि (usesLayout<IEEEFloat>(getSemantics()))                                 \
      फर्काऊ U.IEEE.METHOD_CALL;                                               \
    यदि (usesLayout<DoubleAPFloat>(getSemantics()))                             \
      फर्काऊ U.Double.METHOD_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } जबसम्म (असत्य)

using नामाकाश llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#define Pack_Categories_Into_साँचो(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

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

अचर llvm::fltSemantics &APFloatBase::EnumToSemantics(Semantics S) {
  विकल्प (S) {
  पक्ष S_IEEEhalf:
    फर्काऊ IEEEhalf();
  पक्ष S_BFloat:
    फर्काऊ BFloat();
  पक्ष S_IEEEsingle:
    फर्काऊ IEEEsingle();
  पक्ष S_IEEEdouble:
    फर्काऊ IEEEdouble();
  पक्ष S_IEEEquad:
    फर्काऊ IEEEquad();
  पक्ष S_x87DoubleExtended:
    फर्काऊ x87DoubleExtended();
  पक्ष S_PPCDoubleDouble:
    फर्काऊ PPCDoubleDouble();
  पक्ष S_PPCDoubleDoubleLegacy:
    फर्काऊ PPCDoubleDoubleLegacy();
  पक्ष S_Float8E5M2:
    फर्काऊ Float8E5M2();
  पक्ष S_Float8E5M2FNUZ:
    फर्काऊ Float8E5M2FNUZ();
  पक्ष S_Float8E4M3:
    फर्काऊ Float8E4M3();
  पक्ष S_Float8E4M3FN:
    फर्काऊ Float8E4M3FN();
  पक्ष S_Float8E4M3FNUZ:
    फर्काऊ Float8E4M3FNUZ();
  पक्ष S_Float8E4M3B11FNUZ:
    फर्काऊ Float8E4M3B11FNUZ();
  पक्ष S_Float8E3M4:
    फर्काऊ Float8E3M4();
  पक्ष S_FloatTF32:
    फर्काऊ FloatTF32();
  पक्ष S_Float8E8M0FNU:
    फर्काऊ Float8E8M0FNU();
  पक्ष S_Float8E5M3FNU:
    फर्काऊ Float8E5M3FNU();
  पक्ष S_Float6E3M2FN:
    फर्काऊ Float6E3M2FN();
  पक्ष S_Float6E2M3FN:
    फर्काऊ Float6E2M3FN();
  पक्ष S_Float4E2M1FN:
    फर्काऊ Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloatBase::Semantics
APFloatBase::SemanticsToEnum(अचर llvm::fltSemantics &Sem) {
  यदि (&Sem == &llvm::APFloat::IEEEhalf())
    फर्काऊ S_IEEEhalf;
  अन्यथा यदि (&Sem == &llvm::APFloat::BFloat())
    फर्काऊ S_BFloat;
  अन्यथा यदि (&Sem == &llvm::APFloat::IEEEsingle())
    फर्काऊ S_IEEEsingle;
  अन्यथा यदि (&Sem == &llvm::APFloat::IEEEdouble())
    फर्काऊ S_IEEEdouble;
  अन्यथा यदि (&Sem == &llvm::APFloat::IEEEquad())
    फर्काऊ S_IEEEquad;
  अन्यथा यदि (&Sem == &llvm::APFloat::x87DoubleExtended())
    फर्काऊ S_x87DoubleExtended;
  अन्यथा यदि (&Sem == &llvm::APFloat::PPCDoubleDouble())
    फर्काऊ S_PPCDoubleDouble;
  अन्यथा यदि (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    फर्काऊ S_PPCDoubleDoubleLegacy;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E5M2())
    फर्काऊ S_Float8E5M2;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    फर्काऊ S_Float8E5M2FNUZ;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E4M3())
    फर्काऊ S_Float8E4M3;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E4M3FN())
    फर्काऊ S_Float8E4M3FN;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    फर्काऊ S_Float8E4M3FNUZ;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    फर्काऊ S_Float8E4M3B11FNUZ;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E3M4())
    फर्काऊ S_Float8E3M4;
  अन्यथा यदि (&Sem == &llvm::APFloat::FloatTF32())
    फर्काऊ S_FloatTF32;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E8M0FNU())
    फर्काऊ S_Float8E8M0FNU;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float8E5M3FNU())
    फर्काऊ S_Float8E5M3FNU;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float6E3M2FN())
    फर्काऊ S_Float6E3M2FN;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float6E2M3FN())
    फर्काऊ S_Float6E2M3FN;
  अन्यथा यदि (&Sem == &llvm::APFloat::Float4E2M1FN())
    फर्काऊ S_Float4E2M1FN;
  अन्यथा
    llvm_unreachable("Unknown floating semantics");
}

तार्किक APFloatBase::isRepresentableBy(अचर fltSemantics &A,
                                    अचर fltSemantics &B) {
  फर्काऊ A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

तार्किक APFloatBase::isLosslesslyConvertibleTo(अचर fltSemantics &From,
                                            अचर fltSemantics &InitialTo,
                                            तार्किक IgnoreNaNs) {
  यदि (&From == &InitialTo)
    फर्काऊ सत्य;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  यदि (&From == &semPPCDoubleDouble)
    फर्काऊ असत्य;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  अचर fltSemantics &To =
      &InitialTo == &semPPCDoubleDouble ? semIEEEdouble : InitialTo;

  यदि (!isRepresentableBy(From, To))
    फर्काऊ असत्य;

  यदि ((From.hasZero && !To.hasZero) ||
      (From.hasSignedRepr && !To.hasSignedRepr))
    फर्काऊ असत्य;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  तार्किक FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  तार्किक ToHasSignedZero = To.hasZero && To.hasSignedRepr &&
                         To.nanEncoding != fltNanEncoding::NegativeZero;
  यदि (FromHasSignedZero && !ToHasSignedZero)
    फर्काऊ असत्य;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat SmallestFrom = APFloat::getSmallest(From);
  तार्किक LosesInfo = असत्य;
  (शून्य)SmallestFrom.convert(To, APFloat::rmNearestTiesToEven, &LosesInfo);
  यदि (LosesInfo)
    फर्काऊ असत्य;

  यदि (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    फर्काऊ सत्य;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  यदि (IgnoreNaNs) {
    यदि (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      फर्काऊ To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    फर्काऊ To.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  यदि (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    फर्काऊ असत्य;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  यदि (To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    फर्काऊ सत्य;
  फर्काऊ To.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
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
स्थिर constexpr अचिन्हित अङ्क_Pow5_Parts_On_थाक = 16;

स्थिर अचिन्हित पूर्णाङ्क powerOfFivePartsForExponent(अचिन्हित पूर्णाङ्क power) {
  प्रतिज्ञा(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  फर्काऊ 2 + ((power * 815) / (351 * APFloatBase::integerPartWidth));
}

अचिन्हित पूर्णाङ्क APFloatBase::semanticsPrecision(अचर fltSemantics &semantics) {
  फर्काऊ semantics.precision;
}
APFloatBase::ExponentType
APFloatBase::semanticsMaxExponent(अचर fltSemantics &semantics) {
  फर्काऊ semantics.maxExponent;
}
APFloatBase::ExponentType
APFloatBase::semanticsMinExponent(अचर fltSemantics &semantics) {
  फर्काऊ semantics.minExponent;
}
अचिन्हित पूर्णाङ्क APFloatBase::semanticsSizeInBits(अचर fltSemantics &semantics) {
  फर्काऊ semantics.sizeInBits;
}
अचिन्हित पूर्णाङ्क APFloatBase::semanticsIntSizeInBits(अचर fltSemantics &semantics,
                                                 तार्किक isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  अचिन्हित पूर्णाङ्क MinBitWidth = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  यदि (isSigned)
    ++MinBitWidth;
  फर्काऊ MinBitWidth;
}

तार्किक APFloatBase::semanticsHasZero(अचर fltSemantics &semantics) {
  फर्काऊ semantics.hasZero;
}

तार्किक APFloatBase::semanticsHasSignedRepr(अचर fltSemantics &semantics) {
  फर्काऊ semantics.hasSignedRepr;
}

तार्किक APFloatBase::semanticsHasInf(अचर fltSemantics &semantics) {
  फर्काऊ semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

तार्किक APFloatBase::semanticsHasNaN(अचर fltSemantics &semantics) {
  फर्काऊ semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

तार्किक APFloatBase::isIEEELikeFP(अचर fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  फर्काऊ SemanticsToEnum(semantics) <= S_IEEEquad;
}

तार्किक APFloatBase::hasSignBitInMSB(अचर fltSemantics &semantics) {
  फर्काऊ semantics.hasSignBitInMSB;
}

तार्किक APFloatBase::isRepresentableAsNormalIn(अचर fltSemantics &Src,
                                            अचर fltSemantics &Dst) {
  // Exponent range must be larger.
  यदि (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    फर्काऊ असत्य;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  फर्काऊ Dst.precision >= Src.precision;
}

अचिन्हित APFloatBase::getSizeInBits(अचर fltSemantics &Sem) {
  फर्काऊ Sem.sizeInBits;
}

स्थिर constexpr APFloatBase::ExponentType
exponentZero(अचर fltSemantics &semantics) {
  फर्काऊ semantics.minExponent - 1;
}

स्थिर constexpr APFloatBase::ExponentType
exponentInf(अचर fltSemantics &semantics) {
  फर्काऊ semantics.maxExponent + 1;
}

स्थिर constexpr APFloatBase::ExponentType
exponentNaN(अचर fltSemantics &semantics) {
  यदि (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    यदि (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      फर्काऊ exponentZero(semantics);
    यदि (semantics.hasSignedRepr || semantics.precision > 1)
      फर्काऊ semantics.maxExponent;
  }
  फर्काऊ semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

स्थिर अन्तःस्थ Error createError(अचर Twine &Err) {
  फर्काऊ make_error<StringError>(Err, inconvertibleErrorCode());
}

स्थिर constexpr अन्तःस्थ अचिन्हित पूर्णाङ्क part_गन्ती_For_Bits(अचिन्हित पूर्णाङ्क bits) {
  फर्काऊ std::max(1u, (bits + APFloatBase::integerPartWidth - 1) /
                          APFloatBase::integerPartWidth);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
स्थिर अन्तःस्थ अचिन्हित पूर्णाङ्क
dec_Digit_मान(अचिन्हित पूर्णाङ्क c)
{
  फर्काऊ c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
स्थिर Expected<पूर्णाङ्क> readExponent(StringRef::iterator begin,
                                  StringRef::iterator end) {
  अचर अचिन्हित पूर्णाङ्क overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  यदि (p == end || ((*p == '-' || *p == '+') && (p + 1) == end))
    फर्काऊ 0;

  तार्किक isNegative = *p == '-';
  यदि (*p == '-' || *p == '+') {
    p++;
    यदि (p == end)
      फर्काऊ createError("Exponent has no digits");
  }

  अचिन्हित absExponent = dec_Digit_मान(*p++);
  यदि (absExponent >= 10U)
    फर्काऊ createError("Invalid character in exponent");

  लागि (; p != end; ++p) {
    अचिन्हित मान_2 = dec_Digit_मान(*p);
    यदि (मान_2 >= 10U)
      फर्काऊ createError("Invalid character in exponent");

    absExponent = absExponent * 10U + मान_2;
    यदि (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      रोक;
    }
  }

  यदि (isNegative)
    फर्काऊ -(पूर्णाङ्क) absExponent;
  अन्यथा
    फर्काऊ (पूर्णाङ्क) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
स्थिर Expected<पूर्णाङ्क> totalExponent(StringRef::iterator p,
                                   StringRef::iterator end,
                                   पूर्णाङ्क exponentAdjustment) {
  पूर्णाङ्क exponent = 0;

  यदि (p == end)
    फर्काऊ createError("Exponent has no digits");

  तार्किक negative = *p == '-';
  यदि (*p == '-' || *p == '+') {
    p++;
    यदि (p == end)
      फर्काऊ createError("Exponent has no digits");
  }

  पूर्णाङ्क unsignedExponent = 0;
  तार्किक overflow = असत्य;
  लागि (; p != end; ++p) {
    अचिन्हित पूर्णाङ्क मान_2;

    मान_2 = dec_Digit_मान(*p);
    यदि (मान_2 >= 10U)
      फर्काऊ createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + मान_2;
    यदि (unsignedExponent > 32767) {
      overflow = सत्य;
      रोक;
    }
  }

  यदि (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    overflow = सत्य;

  यदि (!overflow) {
    exponent = unsignedExponent;
    यदि (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    यदि (exponent > 32767 || exponent < -32768)
      overflow = सत्य;
  }

  यदि (overflow)
    exponent = negative ? -32768: 32767;

  फर्काऊ exponent;
}

स्थिर Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator end,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = end;
  जबसम्म (p != end && *p == '0')
    p++;

  यदि (p != end && *p == '.') {
    *dot = p++;

    यदि (end - begin == 1)
      फर्काऊ createError("Significand has no digits");

    जबसम्म (p != end && *p == '0')
      p++;
  }

  फर्काऊ p;
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
  अचर वर्ण *firstSigDigit;
  अचर वर्ण *lastSigDigit;
  पूर्णाङ्क exponent;
  पूर्णाङ्क normalizedExponent;
};

स्थिर Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator end, decimalInfo *D) {
  StringRef::iterator dot = end;

  स्वतः PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  यदि (!PtrOrErr)
    फर्काऊ PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->firstSigDigit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  लागि (; p != end; ++p) {
    यदि (*p == '.') {
      यदि (dot != end)
        फर्काऊ createError("String contains multiple dots");
      dot = p++;
      यदि (p == end)
        रोक;
    }
    यदि (dec_Digit_मान(*p) >= 10U)
      रोक;
  }

  यदि (p != end) {
    यदि (*p != 'e' && *p != 'E')
      फर्काऊ createError("Invalid character in significand");
    यदि (p == begin)
      फर्काऊ createError("Significand has no digits");
    यदि (dot != end && p - begin == 1)
      फर्काऊ createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    स्वतः ExpOrErr = readExponent(p + 1, end);
    यदि (!ExpOrErr)
      फर्काऊ ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    यदि (dot == end)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  यदि (p != D->firstSigDigit) {
    /* Drop insignificant trailing zeroes.  */
    यदि (p != begin) {
      गर
        गर
          p--;
        जबसम्म (p != begin && *p == '0');
      जबसम्म (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->firstSigDigit)
                                      - (dot > D->firstSigDigit && dot < p)));
  }

  D->lastSigDigit = p;
  फर्काऊ Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
स्थिर Expected<lostFraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator end,
                            अचिन्हित पूर्णाङ्क digit_मान) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  यदि (digit_मान > 8)
    फर्काऊ lfMoreThanHalf;
  अन्यथा यदि (digit_मान < 8 && digit_मान > 0)
    फर्काऊ lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  जबसम्म (p != end && (*p == '0' || *p == '.'))
    p++;

  यदि (p == end)
    फर्काऊ createError("Invalid trailing hexadecimal fraction!");

  अचिन्हित hexDigit = hex_Digit_मान(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  यदि (hexDigit == अच_पूर्णांक_उच्च)
    फर्काऊ digit_मान == 0 ? lfExactlyZero: lfExactlyHalf;
  अन्यथा
    फर्काऊ digit_मान == 0 ? lfLessThanHalf: lfMoreThanHalf;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
स्थिर lostFraction
lostFractionThroughTruncation(अचर APFloatBase::integerPart *parts,
                              अचिन्हित पूर्णाङ्क part_गन्ती,
                              अचिन्हित पूर्णाङ्क bits)
{
  अचिन्हित lsb = APInt::tcLSB(parts, part_गन्ती);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  यदि (bits <= lsb)
    फर्काऊ lfExactlyZero;
  यदि (bits == lsb + 1)
    फर्काऊ lfExactlyHalf;
  यदि (bits <= part_गन्ती * APFloatBase::integerPartWidth &&
      APInt::tcExtractBit(parts, bits - 1))
    फर्काऊ lfMoreThanHalf;

  फर्काऊ lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
स्थिर lostFraction
shift_दायाँ(APFloatBase::integerPart *dst, अचिन्हित पूर्णाङ्क parts, अचिन्हित पूर्णाङ्क bits)
{
  lostFraction lost_fraction = lostFractionThroughTruncation(dst, parts, bits);

  APInt::tc_Shift_दायाँ(dst, parts, bits);

  फर्काऊ lost_fraction;
}

/* Combine the effect of two lost fractions.  */
स्थिर lostFraction
combineLostFractions(lostFraction moreSignificant,
                     lostFraction lessSignificant)
{
  यदि (lessSignificant != lfExactlyZero) {
    यदि (moreSignificant == lfExactlyZero)
      moreSignificant = lfLessThanHalf;
    अन्यथा यदि (moreSignificant == lfExactlyHalf)
      moreSignificant = lfMoreThanHalf;
  }

  फर्काऊ moreSignificant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
स्थिर अचिन्हित पूर्णाङ्क
HUerrBound(तार्किक inexactMultiply, अचिन्हित पूर्णाङ्क HUerr1, अचिन्हित पूर्णाङ्क HUerr2)
{
  प्रतिज्ञा(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  यदि (HUerr1 + HUerr2 == 0)
    फर्काऊ inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  अन्यथा
    फर्काऊ inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
स्थिर APFloatBase::integerPart
ulpsFromBoundary(अचर APFloatBase::integerPart *parts, अचिन्हित पूर्णाङ्क bits,
                 तार्किक isNearest) {
  प्रतिज्ञा(bits != 0);

  bits--;
  अचिन्हित गन्ती = bits / APFloatBase::integerPartWidth;
  अचिन्हित partBits = bits % APFloatBase::integerPartWidth + 1;

  APFloatBase::integerPart part =
      parts[गन्ती] & (~(APFloatBase::integerPart)0 >>
                      (APFloatBase::integerPartWidth - partBits));

  APFloatBase::integerPart boundary;
  यदि (isNearest)
    boundary = (APFloatBase::integerPart) 1 << (partBits - 1);
  अन्यथा
    boundary = 0;

  यदि (गन्ती == 0) {
    यदि (part - boundary <= boundary - part)
      फर्काऊ part - boundary;
    अन्यथा
      फर्काऊ boundary - part;
  }

  यदि (part == boundary) {
    जबसम्म (--गन्ती)
      यदि (parts[गन्ती])
        फर्काऊ ~(APFloatBase::integerPart) 0; /* A lot.  */

    फर्काऊ parts[0];
  } अन्यथा यदि (part == boundary - 1) {
    जबसम्म (--गन्ती)
      यदि (~parts[गन्ती])
        फर्काऊ ~(APFloatBase::integerPart) 0; /* A lot.  */

    फर्काऊ -parts[0];
  }

  फर्काऊ ~(APFloatBase::integerPart) 0; /* A lot.  */
}

स्थिर constexpr अचिन्हित PowerOfFivePartCounts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

स्थिर constexpr APFloatBase::integerPart PowerOfFiveParts[] = {
#अन्तर्भाव "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  अचिन्हित Sum = 0;
  लागि (अचिन्हित गन्ती_2 : PowerOfFivePartCounts)
    Sum += गन्ती_2;
  फर्काऊ Sum;
}() == std::size(PowerOfFiveParts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
स्थिर अचिन्हित पूर्णाङ्क
powerOf5(APFloatBase::integerPart *dst, अचिन्हित पूर्णाङ्क power) {
  स्थिर अचर APFloatBase::integerPart firstEightPowers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  SmallVector<APFloatBase::integerPart, अङ्क_Pow5_Parts_On_थाक> scratch;
  scratch.resize_for_overwrite(powerOfFivePartsForExponent(power));

  APFloatBase::integerPart *p1, *p2;
  अचर APFloatBase::integerPart *pow5;

  p1 = dst;
  p2 = scratch.data();

  *p1 = firstEightPowers[power & 7];
  power >>= 3;

  अचिन्हित result = 1;
  pow5 = PowerOfFiveParts;

  लागि (अचिन्हित पूर्णाङ्क n = 0; power; power >>= 1, n++) {
    अचिन्हित parts_गन्ती = PowerOfFivePartCounts[n];

    यदि (power & 1) {
      APFloatBase::integerPart *tmp;

      APInt::tcFullMultiply(p2, p1, pow5, result, parts_गन्ती);
      result += parts_गन्ती;
      यदि (p2[result - 1] == 0)
        result--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_गन्ती;
  }

  यदि (p1 != dst)
    APInt::tcAssign(dst, p1, result);

  फर्काऊ result;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
स्थिर अचर वर्ण hexDigitsLower[] = "0123456789abcdef0";
स्थिर अचर वर्ण hexDigitsUpper[] = "0123456789ABCDEF0";
स्थिर अचर वर्ण infinityL[] = "infinity";
स्थिर अचर वर्ण infinityU[] = "INFINITY";
स्थिर अचर वर्ण NaNL[] = "nan";
स्थिर अचर वर्ण NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
स्थिर अचिन्हित पूर्णाङ्क
partAsHex (वर्ण *dst, APFloatBase::integerPart part, अचिन्हित पूर्णाङ्क गन्ती,
           अचर वर्ण *hexDigitChars)
{
  अचिन्हित पूर्णाङ्क result = गन्ती;

  प्रतिज्ञा(गन्ती != 0 && गन्ती <= APFloatBase::integerPartWidth / 4);

  part >>= (APFloatBase::integerPartWidth - 4 * गन्ती);
  जबसम्म (गन्ती--) {
    dst[गन्ती] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  फर्काऊ result;
}

/* Write out an unsigned decimal integer.  */
स्थिर वर्ण *writeUnsignedDecimal(वर्ण *dst, अचिन्हित पूर्णाङ्क n) {
  वर्ण buff[40], *p;

  p = buff;
  गर
    *p++ = '0' + n % 10;
  जबसम्म (n /= 10);

  गर
    *dst++ = *--p;
  जबसम्म (p != buff);

  फर्काऊ dst;
}

/* Write out a signed decimal integer.  */
स्थिर वर्ण *writeSignedDecimal(वर्ण *dst, पूर्णाङ्क मान_2) {
  यदि (मान_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(अचिन्हित) मान_2);
  } अन्यथा {
    dst = writeUnsignedDecimal(dst, मान_2);
  }

  फर्काऊ dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
स्थिर APFloat harrisonUlp(अचर APFloat &X) {
  अचर fltSemantics &Sem = X.getSemantics();
  विकल्प (X.getCategory()) {
  पक्ष APFloat::fcNaN:
    फर्काऊ APFloat::getQNaN(Sem);
  पक्ष APFloat::fcInfinity:
    फर्काऊ APFloat::getInf(Sem);
  पक्ष APFloat::fcZero:
    फर्काऊ APFloat::getSmallest(Sem);
  पक्ष APFloat::fcNormal:
    रोक;
  }
  यदि (X.isDenormal() || X.isSmallestNormalized())
    फर्काऊ APFloat::getSmallest(Sem);
  पूर्णाङ्क Exp = ilogb(X);
  यदि (X.getExactLog2() != पूर्णांक_न्यून)
    Exp -= 1;
  फर्काऊ scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rmNearestTiesToEven);
}

नामाकाश detail {
/* Constructors.  */
शून्य IEEEFloat::initialize(अचर fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  अचिन्हित गन्ती = part_गन्ती();
  यदि (गन्ती > 1)
    significand.parts = नूतन integerPart[गन्ती];
}

शून्य IEEEFloat::freeSignificand() {
  यदि (needsCleanup())
    delete [] significand.parts;
}

शून्य IEEEFloat::assign(अचर IEEEFloat &rhs) {
  प्रतिज्ञा(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  यदि (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

शून्य IEEEFloat::copySignificand(अचर IEEEFloat &rhs) {
  प्रतिज्ञा(isFiniteNonZero() || category == fcNaN);
  प्रतिज्ञा(rhs.part_गन्ती() >= part_गन्ती());

  APInt::tcAssign(significandParts(), rhs.significandParts(),
                  part_गन्ती());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
शून्य IEEEFloat::makeNaN(तार्किक SNaN, तार्किक Negative, अचर APInt *fill) {
  यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  यदि (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  अचिन्हित अङ्क_Parts = part_गन्ती();

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
  यदि (!fill || fill->get_अङ्क_शब्दहरू() < अङ्क_Parts)
    APInt::tcSet(significand, 0, अङ्क_Parts);
  यदि (fill) {
    APInt::tcAssign(significand, fill->getRawData(),
                    std::min(fill->get_अङ्क_शब्दहरू(), अङ्क_Parts));

    // Zero out the excess bits of the significand.
    अचिन्हित bitsToPreserve = semantics->precision - 1;
    अचिन्हित part = bitsToPreserve / 64;
    bitsToPreserve %= 64;
    significand[part] &= ((1ULL << bitsToPreserve) - 1);
    लागि (part++; part != अङ्क_Parts; ++part)
      significand[part] = 0;
  }

  अचिन्हित QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  यदि (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tcClearBit(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    यदि (APInt::tcIsZero(significand, अङ्क_Parts))
      APInt::tcSetBit(significand, QNaNBit - 1);
  } अन्यथा यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } अन्यथा {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tcSetBit(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  यदि (semantics == &APFloatBase::semX87DoubleExtended)
    APInt::tcSetBit(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::चालक=(अचर IEEEFloat &rhs) {
  यदि (एतत् != &rhs) {
    यदि (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    assign(rhs);
  }

  फर्काऊ *एतत्;
}

IEEEFloat &IEEEFloat::चालक=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloatBase::semBogus;
  फर्काऊ *एतत्;
}

तार्किक IEEEFloat::isDenormal() अचर {
  फर्काऊ getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tcExtractBit(significandParts(), semantics->precision - 1) ==
          0);
}

तार्किक IEEEFloat::isSmallest() अचर {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  फर्काऊ isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

तार्किक IEEEFloat::isSmallestNormalized() अचर {
  फर्काऊ getCategory() == fcNormal && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

अचिन्हित पूर्णाङ्क IEEEFloat::get_अङ्क_माथिल्लो_Bits() अचर {
  अचर अचिन्हित पूर्णाङ्क Part_गन्ती = part_गन्ती_For_Bits(semantics->precision);
  अचर अचिन्हित पूर्णाङ्क Bits = Part_गन्ती * integerPartWidth;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  अचर अचिन्हित पूर्णाङ्क अङ्क_माथिल्लो_Bits = (semantics->precision > 1)
                                       ? (Bits - semantics->precision + 1)
                                       : (Bits - semantics->precision);
  फर्काऊ अङ्क_माथिल्लो_Bits;
}

तार्किक IEEEFloat::isSignificandAllOnes() अचर {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  अचर integerPart *Parts = significandParts();
  अचर अचिन्हित Part_गन्ती = part_गन्ती_For_Bits(semantics->precision);
  लागि (अचिन्हित i = 0; i < Part_गन्ती - 1; i++)
    यदि (~Parts[i])
      फर्काऊ असत्य;

  // Set the unused high bits to all ones when we compare.
  अचर अचिन्हित अङ्क_माथिल्लो_Bits = get_अङ्क_माथिल्लो_Bits();
  प्रतिज्ञा(अङ्क_माथिल्लो_Bits <= integerPartWidth && अङ्क_माथिल्लो_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  अचर integerPart माथिल्लो_Bit_Fill =
    ~integerPart(0) << (integerPartWidth - अङ्क_माथिल्लो_Bits);
  यदि ((semantics->precision <= 1) || (~(Parts[Part_गन्ती - 1] | माथिल्लो_Bit_Fill)))
    फर्काऊ असत्य;

  फर्काऊ सत्य;
}

तार्किक IEEEFloat::isSignificandAllOnesExceptLSB() अचर {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  अचर integerPart *Parts = significandParts();

  यदि (Parts[0] & 1)
    फर्काऊ असत्य;

  अचर अचिन्हित Part_गन्ती = part_गन्ती_For_Bits(semantics->precision);
  लागि (अचिन्हित i = 0; i < Part_गन्ती - 1; i++) {
    यदि (~Parts[i] & ~अचिन्हित{!i})
      फर्काऊ असत्य;
  }

  // Set the unused high bits to all ones when we compare.
  अचर अचिन्हित अङ्क_माथिल्लो_Bits = get_अङ्क_माथिल्लो_Bits();
  प्रतिज्ञा(अङ्क_माथिल्लो_Bits <= integerPartWidth && अङ्क_माथिल्लो_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  अचर integerPart माथिल्लो_Bit_Fill = ~integerPart(0)
                                  << (integerPartWidth - अङ्क_माथिल्लो_Bits);
  यदि (~(Parts[Part_गन्ती - 1] | माथिल्लो_Bit_Fill | 0x1))
    फर्काऊ असत्य;

  फर्काऊ सत्य;
}

तार्किक IEEEFloat::isSignificandAllZeros() अचर {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  अचर integerPart *Parts = significandParts();
  अचर अचिन्हित Part_गन्ती = part_गन्ती_For_Bits(semantics->precision);

  लागि (अचिन्हित i = 0; i < Part_गन्ती - 1; i++)
    यदि (Parts[i])
      फर्काऊ असत्य;

  // Compute how many bits are used in the final word.
  अचर अचिन्हित अङ्क_माथिल्लो_Bits = get_अङ्क_माथिल्लो_Bits();
  प्रतिज्ञा(अङ्क_माथिल्लो_Bits < integerPartWidth && "Can not have more high bits to "
         "clear than integerPartWidth");
  अचर integerPart माथिल्लो_Bit_Mask = ~integerPart(0) >> अङ्क_माथिल्लो_Bits;

  यदि ((semantics->precision > 1) && (Parts[Part_गन्ती - 1] & माथिल्लो_Bit_Mask))
    फर्काऊ असत्य;

  फर्काऊ सत्य;
}

तार्किक IEEEFloat::isSignificandAllZerosExceptMSB() अचर {
  अचर integerPart *Parts = significandParts();
  अचर अचिन्हित Part_गन्ती = part_गन्ती_For_Bits(semantics->precision);

  लागि (अचिन्हित i = 0; i < Part_गन्ती - 1; i++) {
    यदि (Parts[i])
      फर्काऊ असत्य;
  }

  अचर अचिन्हित अङ्क_माथिल्लो_Bits = get_अङ्क_माथिल्लो_Bits();
  अचर integerPart MSBMask = integerPart(1)
                              << (integerPartWidth - अङ्क_माथिल्लो_Bits);
  फर्काऊ ((semantics->precision <= 1) || (Parts[Part_गन्ती - 1] == MSBMask));
}

तार्किक IEEEFloat::isLargest() अचर {
  तार्किक IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    फर्काऊ (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } अन्यथा {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    फर्काऊ IsMaxExp && isSignificandAllOnes();
  }
}

तार्किक IEEEFloat::isInteger() अचर {
  // This could be made more efficient; I'm going for obviously correct.
  यदि (!isFinite()) फर्काऊ असत्य;
  IEEEFloat truncated = *एतत्;
  truncated.roundToIntegral(rmTowardZero);
  फर्काऊ compare(truncated) == cmpEqual;
}

तार्किक IEEEFloat::bitwiseIsEqual(अचर IEEEFloat &rhs) अचर {
  यदि (एतत् == &rhs)
    फर्काऊ सत्य;
  यदि (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    फर्काऊ असत्य;
  यदि (category==fcZero || category==fcInfinity)
    फर्काऊ सत्य;

  यदि (isFiniteNonZero() && exponent != rhs.exponent)
    फर्काऊ असत्य;

  फर्काऊ std::equal(significandParts(), significandParts() + part_गन्ती(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(अचर fltSemantics &ourSemantics, integerPart मान_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fcNormal;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = मान_2;
  normalize(rmNearestTiesToEven, lfExactlyZero);
}

IEEEFloat::IEEEFloat(अचर fltSemantics &ourSemantics) {
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
IEEEFloat::IEEEFloat(अचर fltSemantics &ourSemantics, uninitializedTag tag)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(अचर IEEEFloat &rhs) {
  initialize(rhs.semantics);
  assign(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloatBase::semBogus) {
  *एतत् = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

अचिन्हित पूर्णाङ्क IEEEFloat::part_गन्ती() अचर {
  फर्काऊ part_गन्ती_For_Bits(semantics->precision + 1);
}

अचर APFloat::integerPart *IEEEFloat::significandParts() अचर {
  फर्काऊ const_cast<IEEEFloat *>(एतत्)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  यदि (part_गन्ती() > 1)
    फर्काऊ significand.parts;
  अन्यथा
    फर्काऊ &significand.part;
}

शून्य IEEEFloat::zeroSignificand() {
  APInt::tcSet(significandParts(), 0, part_गन्ती());
}

/* Increment an fcNormal floating point number's significand.  */
शून्य IEEEFloat::incrementSignificand() {
  [[maybe_unused]] integerPart carry =
      APInt::tcIncrement(significandParts(), part_गन्ती());

  /* Our callers should never cause us to overflow.  */
  प्रतिज्ञा(carry == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::addSignificand(अचर IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  प्रतिज्ञा(semantics == rhs.semantics);
  प्रतिज्ञा(exponent == rhs.exponent);

  फर्काऊ APInt::tcAdd(parts, rhs.significandParts(), 0, part_गन्ती());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::subtractSignificand(अचर IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  प्रतिज्ञा(semantics == rhs.semantics);
  प्रतिज्ञा(exponent == rhs.exponent);

  फर्काऊ APInt::tcSubtract(parts, rhs.significandParts(), borrow,
                           part_गन्ती());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
lostFraction IEEEFloat::multiplySignificand(अचर IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            तार्किक ignoreAddend) {
  integerPart scratch[4];
  तार्किक ignored;

  प्रतिज्ञा(semantics == rhs.semantics);

  अचिन्हित precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  अचिन्हित new_Parts_गन्ती = part_गन्ती_For_Bits(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *fullSignificand =
      new_Parts_गन्ती > 4 ? नूतन integerPart[new_Parts_गन्ती] : scratch;

  integerPart *lhsSignificand = significandParts();
  अचिन्हित parts_गन्ती = part_गन्ती();

  APInt::tcFullMultiply(fullSignificand, lhsSignificand,
                        rhs.significandParts(), parts_गन्ती, parts_गन्ती);

  lostFraction lost_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  अचिन्हित omsb = APInt::tcMSB(fullSignificand, new_Parts_गन्ती) + 1;
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
    अचर fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    अचिन्हित extendedPrecision = 2 * precision + 1;
    यदि (omsb != extendedPrecision - 1) {
      प्रतिज्ञा(extendedPrecision > omsb);
      APInt::tc_Shift_बायाँ(fullSignificand, new_Parts_गन्ती,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    यदि (new_Parts_गन्ती == 1)
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
    lost_fraction = extendedAddend.shift_Significand_दायाँ(1);
    प्रतिज्ञा(lost_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    lost_fraction = addOrSubtractSignificand(extendedAddend, असत्य);

    /* Restore our state.  */
    यदि (new_Parts_गन्ती == 1)
      fullSignificand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(fullSignificand, new_Parts_गन्ती) + 1;
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
    अचिन्हित पूर्णाङ्क bits, significantParts;
    lostFraction lf;

    bits = omsb - precision;
    significantParts = part_गन्ती_For_Bits(omsb);
    lf = shift_दायाँ(fullSignificand, significantParts, bits);
    lost_fraction = combineLostFractions(lf, lost_fraction);
    exponent += bits;
  }

  APInt::tcAssign(lhsSignificand, fullSignificand, parts_गन्ती);

  यदि (new_Parts_गन्ती > 4)
    delete [] fullSignificand;

  फर्काऊ lost_fraction;
}

lostFraction IEEEFloat::multiplySignificand(अचर IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  फर्काऊ multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
lostFraction IEEEFloat::divideSignificand(अचर IEEEFloat &rhs) {
  integerPart scratch[4];

  प्रतिज्ञा(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  अचर integerPart *rhsSignificand = rhs.significandParts();
  अचिन्हित parts_गन्ती = part_गन्ती();

  integerPart *dividend =
      parts_गन्ती > 2 ? नूतन integerPart[parts_गन्ती * 2] : scratch;
  integerPart *भाजक = dividend + parts_गन्ती;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  लागि (अचिन्हित i = 0; i < parts_गन्ती; i++) {
    dividend[i] = lhsSignificand[i];
    भाजक[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  अचिन्हित पूर्णाङ्क precision = semantics->precision;

  /* Normalize the divisor.  */
  अचिन्हित bit = precision - APInt::tcMSB(भाजक, parts_गन्ती) - 1;
  यदि (bit) {
    exponent += bit;
    APInt::tc_Shift_बायाँ(भाजक, parts_गन्ती, bit);
  }

  /* Normalize the dividend.  */
  bit = precision - APInt::tcMSB(dividend, parts_गन्ती) - 1;
  यदि (bit) {
    exponent -= bit;
    APInt::tc_Shift_बायाँ(dividend, parts_गन्ती, bit);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  यदि (APInt::tcCompare(dividend, भाजक, parts_गन्ती) < 0) {
    exponent--;
    APInt::tc_Shift_बायाँ(dividend, parts_गन्ती, 1);
    प्रतिज्ञा(APInt::tcCompare(dividend, भाजक, parts_गन्ती) >= 0);
  }

  /* Long division.  */
  लागि (bit = precision; bit; bit -= 1) {
    यदि (APInt::tcCompare(dividend, भाजक, parts_गन्ती) >= 0) {
      APInt::tcSubtract(dividend, भाजक, 0, parts_गन्ती);
      APInt::tcSetBit(lhsSignificand, bit - 1);
    }

    APInt::tc_Shift_बायाँ(dividend, parts_गन्ती, 1);
  }

  /* Figure out the lost fraction.  */
  पूर्णाङ्क cmp = APInt::tcCompare(dividend, भाजक, parts_गन्ती);

  lostFraction lost_fraction;
  यदि (cmp > 0)
    lost_fraction = lfMoreThanHalf;
  अन्यथा यदि (cmp == 0)
    lost_fraction = lfExactlyHalf;
  अन्यथा यदि (APInt::tcIsZero(dividend, parts_गन्ती))
    lost_fraction = lfExactlyZero;
  अन्यथा
    lost_fraction = lfLessThanHalf;

  यदि (parts_गन्ती > 2)
    delete [] dividend;

  फर्काऊ lost_fraction;
}

अचिन्हित पूर्णाङ्क IEEEFloat::significandMSB() अचर {
  फर्काऊ APInt::tcMSB(significandParts(), part_गन्ती());
}

अचिन्हित पूर्णाङ्क IEEEFloat::significandLSB() अचर {
  फर्काऊ APInt::tcLSB(significandParts(), part_गन्ती());
}

/* Note that a zero result is NOT normalized to fcZero.  */
lostFraction IEEEFloat::shift_Significand_दायाँ(अचिन्हित पूर्णाङ्क bits) {
  /* Our exponent should not overflow.  */
  प्रतिज्ञा((ExponentType) (exponent + bits) >= exponent);

  exponent += bits;

  फर्काऊ shift_दायाँ(significandParts(), part_गन्ती(), bits);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
शून्य IEEEFloat::shift_Significand_बायाँ(अचिन्हित पूर्णाङ्क bits) {
  प्रतिज्ञा(bits < semantics->precision ||
         (semantics->precision == 1 && bits <= 1));

  यदि (bits) {
    अचिन्हित पूर्णाङ्क parts_गन्ती = part_गन्ती();

    APInt::tc_Shift_बायाँ(significandParts(), parts_गन्ती, bits);
    exponent -= bits;

    प्रतिज्ञा(!APInt::tcIsZero(significandParts(), parts_गन्ती));
  }
}

APFloat::cmpResult IEEEFloat::compare_Absolute_मान(अचर IEEEFloat &rhs) अचर {
  प्रतिज्ञा(semantics == rhs.semantics);
  प्रतिज्ञा(isFiniteNonZero());
  प्रतिज्ञा(rhs.isFiniteNonZero());

  पूर्णाङ्क compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  यदि (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_गन्ती());

  यदि (compare > 0)
    फर्काऊ cmpGreaterThan;
  अन्यथा यदि (compare < 0)
    फर्काऊ cmpLessThan;
  अन्यथा
    फर्काऊ cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
स्थिर शून्य tcSetLeastSignificantBits(APInt::WordType *dst, अचिन्हित parts,
                                      अचिन्हित bits) {
  अचिन्हित i = 0;
  जबसम्म (bits > APInt::APINT_BITS_PER_WORD) {
    dst[i++] = ~(APInt::WordType)0;
    bits -= APInt::APINT_BITS_PER_WORD;
  }

  यदि (bits)
    dst[i++] = ~(APInt::WordType)0 >> (APInt::APINT_BITS_PER_WORD - bits);

  जबसम्म (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::opStatus IEEEFloat::handleOverflow(roundingMode rounding_mode) {
  यदि (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    यदि (rounding_mode == rmNearestTiesToEven ||
        rounding_mode == rmNearestTiesToAway ||
        (rounding_mode == rmTowardPositive && !sign) ||
        (rounding_mode == rmTowardNegative && sign)) {
      यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(असत्य, sign);
      अन्यथा
        category = fcInfinity;
      फर्काऊ static_cast<opStatus>(opOverflow | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fcNormal;
  exponent = semantics->maxExponent;
  tcSetLeastSignificantBits(significandParts(), part_गन्ती(),
                            semantics->precision);
  यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tcClearBit(significandParts(), 0);

  फर्काऊ opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
तार्किक IEEEFloat::roundAwayFromZero(roundingMode rounding_mode,
                                  lostFraction lost_fraction,
                                  अचिन्हित पूर्णाङ्क bit) अचर {
  /* NaNs and infinities should not have lost fractions.  */
  प्रतिज्ञा(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  प्रतिज्ञा(lost_fraction != lfExactlyZero);

  विकल्प (rounding_mode) {
  पक्ष rmNearestTiesToAway:
    फर्काऊ lost_fraction == lfExactlyHalf || lost_fraction == lfMoreThanHalf;

  पक्ष rmNearestTiesToEven:
    यदि (lost_fraction == lfMoreThanHalf)
      फर्काऊ सत्य;

    /* Our zeroes don't have a significand to test.  */
    यदि (lost_fraction == lfExactlyHalf && category != fcZero)
      फर्काऊ APInt::tcExtractBit(significandParts(), bit);

    फर्काऊ असत्य;

  पक्ष rmTowardZero:
    फर्काऊ असत्य;

  पक्ष rmTowardPositive:
    फर्काऊ !sign;

  पक्ष rmTowardNegative:
    फर्काऊ sign;

  स्वभाव:
    रोक;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::opStatus IEEEFloat::normalize(roundingMode rounding_mode,
                                       lostFraction lost_fraction) {
  यदि (!isFiniteNonZero())
    फर्काऊ opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  अचिन्हित omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  यदि (omsb || lost_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    पूर्णाङ्क exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    यदि (exponent + exponentChange > semantics->maxExponent)
      फर्काऊ handleOverflow(rounding_mode);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    यदि (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    यदि (exponentChange < 0) {
      प्रतिज्ञा(lost_fraction == lfExactlyZero);

      shift_Significand_बायाँ(-exponentChange);

      फर्काऊ opOK;
    }

    यदि (exponentChange > 0) {
      lostFraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = shift_Significand_दायाँ(exponentChange);

      lost_fraction = combineLostFractions(lf, lost_fraction);

      /* Keep OMSB up-to-date.  */
      यदि (omsb > (अचिन्हित) exponentChange)
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
    फर्काऊ handleOverflow(rounding_mode);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  यदि (lost_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    यदि (omsb == 0) {
      category = fcZero;
      यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = असत्य;
      यदि (!semantics->hasZero)
        makeSmallestNormalized(असत्य);
    }

    फर्काऊ opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  यदि (roundAwayFromZero(rounding_mode, lost_fraction, 0)) {
    यदि (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    यदि (omsb == (अचिन्हित) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      यदि (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        फर्काऊ handleOverflow(sign ? rmTowardNegative : rmTowardPositive);

      shift_Significand_दायाँ(1);

      फर्काऊ opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      फर्काऊ handleOverflow(rounding_mode);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  यदि (omsb == semantics->precision)
    फर्काऊ opInexact;

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
  फर्काऊ (opStatus) (opUnderflow | opInexact);
}

APFloat::opStatus IEEEFloat::addOrSubtractSpecials(अचर IEEEFloat &rhs,
                                                   तार्किक subtract) {
  विकल्प (Pack_Categories_Into_साँचो(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(nullptr);

  पक्ष Pack_Categories_Into_साँचो(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcNaN):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcNormal):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcNaN):
    यदि (isSignaling()) {
      makeQuiet();
      फर्काऊ opInvalidOp;
    }
    फर्काऊ rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcZero):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcZero):
    फर्काऊ opOK;

  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ subtract;
    फर्काऊ opOK;

  पक्ष Pack_Categories_Into_साँचो(fcZero, fcNormal):
    assign(rhs);
    sign = rhs.sign ^ subtract;
    फर्काऊ opOK;

  पक्ष Pack_Categories_Into_साँचो(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    फर्काऊ opOK;

  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    यदि (((sign ^ rhs.sign)!=0) != subtract) {
      makeNaN();
      फर्काऊ opInvalidOp;
    }

    फर्काऊ opOK;

  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcNormal):
    फर्काऊ opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
lostFraction IEEEFloat::addOrSubtractSignificand(अचर IEEEFloat &rhs,
                                                 तार्किक subtract) {
  [[maybe_unused]] integerPart carry = 0;
  lostFraction lost_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  subtract ^= static_cast<तार्किक>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  पूर्णाङ्क bits = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  यदि (subtract) {
    यदि ((bits < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat temp_rhs(rhs);
    तार्किक lost_fraction_is_from_rhs = असत्य;

    यदि (bits == 0)
      lost_fraction = lfExactlyZero;
    अन्यथा यदि (bits > 0) {
      lost_fraction = temp_rhs.shift_Significand_दायाँ(bits - 1);
      lost_fraction_is_from_rhs = सत्य;
      shift_Significand_बायाँ(1);
    } अन्यथा {
      lost_fraction = shift_Significand_दायाँ(-bits - 1);
      temp_rhs.shift_Significand_बायाँ(1);
    }

    // Should we reverse the subtraction.
    cmpResult cmp_result = compare_Absolute_मान(temp_rhs);
    यदि (cmp_result == cmpLessThan) {
      तार्किक borrow =
          lost_fraction != lfExactlyZero && !lost_fraction_is_from_rhs;
      यदि (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        यदि (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        अन्यथा यदि (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = temp_rhs.subtractSignificand(*एतत्, borrow);
      copySignificand(temp_rhs);
      sign = !sign;
    } अन्यथा यदि (cmp_result == cmpGreaterThan) {
      तार्किक borrow = lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs;
      यदि (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        यदि (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        अन्यथा यदि (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = subtractSignificand(temp_rhs, borrow);
    } अन्यथा { // cmpEqual
      zeroSignificand();
      यदि (lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    प्रतिज्ञा(!carry);
  } अन्यथा {
    यदि (bits > 0) {
      IEEEFloat temp_rhs(rhs);

      lost_fraction = temp_rhs.shift_Significand_दायाँ(bits);
      carry = addSignificand(temp_rhs);
    } अन्यथा {
      lost_fraction = shift_Significand_दायाँ(-bits);
      carry = addSignificand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    प्रतिज्ञा(!carry);
  }

  फर्काऊ lost_fraction;
}

APFloat::opStatus IEEEFloat::multiplySpecials(अचर IEEEFloat &rhs) {
  विकल्प (Pack_Categories_Into_साँचो(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(nullptr);

  पक्ष Pack_Categories_Into_साँचो(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcNaN):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcNaN):
    assign(rhs);
    sign = असत्य;
    [[fallthrough]];
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcNormal):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    यदि (isSignaling()) {
      makeQuiet();
      फर्काऊ opInvalidOp;
    }
    फर्काऊ rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcInfinity):
    category = fcInfinity;
    फर्काऊ opOK;

  पक्ष Pack_Categories_Into_साँचो(fcZero, fcNormal):
  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcZero):
  पक्ष Pack_Categories_Into_साँचो(fcZero, fcZero):
    category = fcZero;
    फर्काऊ opOK;

  पक्ष Pack_Categories_Into_साँचो(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcZero):
    makeNaN();
    फर्काऊ opInvalidOp;

  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcNormal):
    फर्काऊ opOK;
  }
}

APFloat::opStatus IEEEFloat::divideSpecials(अचर IEEEFloat &rhs) {
  विकल्प (Pack_Categories_Into_साँचो(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(nullptr);

  पक्ष Pack_Categories_Into_साँचो(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcNaN):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcNaN):
    assign(rhs);
    sign = असत्य;
    [[fallthrough]];
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcNormal):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    यदि (isSignaling()) {
      makeQuiet();
      फर्काऊ opInvalidOp;
    }
    फर्काऊ rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcZero):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_साँचो(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcZero, fcNormal):
    फर्काऊ opOK;

  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcInfinity):
    category = fcZero;
    फर्काऊ opOK;

  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcZero):
    यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(असत्य, sign);
    अन्यथा
      category = fcInfinity;
    फर्काऊ opDivByZero;

  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcZero, fcZero):
    makeNaN();
    फर्काऊ opInvalidOp;

  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcNormal):
    फर्काऊ opOK;
  }
}

APFloat::opStatus IEEEFloat::modSpecials(अचर IEEEFloat &rhs) {
  विकल्प (Pack_Categories_Into_साँचो(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(nullptr);

  पक्ष Pack_Categories_Into_साँचो(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcNaN):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcNormal):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcNaN):
    यदि (isSignaling()) {
      makeQuiet();
      फर्काऊ opInvalidOp;
    }
    फर्काऊ rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_साँचो(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcZero, fcNormal):
  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcInfinity):
    फर्काऊ opOK;

  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcZero):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcZero):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcZero, fcZero):
    makeNaN();
    फर्काऊ opInvalidOp;

  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcNormal):
    फर्काऊ opOK;
  }
}

APFloat::opStatus IEEEFloat::remainderSpecials(अचर IEEEFloat &rhs) {
  विकल्प (Pack_Categories_Into_साँचो(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(nullptr);

  पक्ष Pack_Categories_Into_साँचो(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcNaN):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcNormal):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcNaN):
    यदि (isSignaling()) {
      makeQuiet();
      फर्काऊ opInvalidOp;
    }
    फर्काऊ rhs.isSignaling() ? opInvalidOp : opOK;

  पक्ष Pack_Categories_Into_साँचो(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcZero, fcNormal):
  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcInfinity):
    फर्काऊ opOK;

  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcZero):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcZero):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcZero, fcZero):
    makeNaN();
    फर्काऊ opInvalidOp;

  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcNormal):
    फर्काऊ opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
शून्य IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    फर्काऊ;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::opStatus IEEEFloat::addOrSubtract(अचर IEEEFloat &rhs,
                                           roundingMode rounding_mode,
                                           तार्किक subtract) {
  opStatus fs = addOrSubtractSpecials(rhs, subtract);

  /* This return code means it was not a simple case.  */
  यदि (fs == opDivByZero) {
    lostFraction lost_fraction;

    lost_fraction = addOrSubtractSignificand(rhs, subtract);
    fs = normalize(rounding_mode, lost_fraction);

    /* Can only be zero if we lost no fraction.  */
    प्रतिज्ञा(category != fcZero || lost_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  यदि (category == fcZero) {
    यदि (rhs.category != fcZero || (sign == rhs.sign) == subtract)
      sign = (rounding_mode == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = असत्य;
  }

  फर्काऊ fs;
}

/* Normalized addition.  */
APFloat::opStatus IEEEFloat::add(अचर IEEEFloat &rhs,
                                 roundingMode rounding_mode) {
  फर्काऊ addOrSubtract(rhs, rounding_mode, असत्य);
}

/* Normalized subtraction.  */
APFloat::opStatus IEEEFloat::subtract(अचर IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  फर्काऊ addOrSubtract(rhs, rounding_mode, सत्य);
}

/* Normalized multiply.  */
APFloat::opStatus IEEEFloat::multiply(अचर IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = multiplySpecials(rhs);

  यदि (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = असत्य;
  यदि (isFiniteNonZero()) {
    lostFraction lost_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    यदि (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  फर्काऊ fs;
}

/* Normalized divide.  */
APFloat::opStatus IEEEFloat::divide(अचर IEEEFloat &rhs,
                                    roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = divideSpecials(rhs);

  यदि (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = असत्य;
  यदि (isFiniteNonZero()) {
    lostFraction lost_fraction = divideSignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    यदि (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  फर्काऊ fs;
}

/* Normalized remainder.  */
APFloat::opStatus IEEEFloat::remainder(अचर IEEEFloat &rhs) {
  अचिन्हित पूर्णाङ्क origSign = sign;

  // First handle the special cases.
  opStatus fs = remainderSpecials(rhs);
  यदि (fs != opDivByZero)
    फर्काऊ fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  यदि (P2.add(rhs, rmNearestTiesToEven) == opOK) {
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
  fs = VEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  प्रतिज्ञा(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  प्रतिज्ञा(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.add(VEx, rmNearestTiesToEven);
  प्रतिज्ञा(fs == opOK);

  यदि (VEx.compare(PEx) == cmpGreaterThan) {
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
    यदि (result == cmpGreaterThan || result == cmpEqual) {
      fs = subtract(P, rmNearestTiesToEven);
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
  फर्काऊ fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::opStatus IEEEFloat::mod(अचर IEEEFloat &rhs) {
  opStatus fs = modSpecials(rhs);
  अचिन्हित पूर्णाङ्क origSign = sign;

  जबसम्म (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_मान(rhs) != cmpLessThan) {
    पूर्णाङ्क Exp = ilogb(*एतत्) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rmNearestTiesToEven);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    यदि (V.isNaN() || compare_Absolute_मान(V) == cmpLessThan)
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
    यदि (!semantics->hasZero && एतत्->isSmallest())
      रोक;

    प्रतिज्ञा(fs==opOK);
  }
  यदि (isZero()) {
    sign = origSign; // fmod requires this
    यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = असत्य;
  }
  फर्काऊ fs;
}

/* Normalized fused-multiply-add.  */
APFloat::opStatus IEEEFloat::fusedMultiplyAdd(अचर IEEEFloat &multiplicand,
                                              अचर IEEEFloat &addend,
                                              roundingMode rounding_mode) {
  opStatus fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  यदि (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    lostFraction lost_fraction;

    lost_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_mode, lost_fraction);
    यदि (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    यदि (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_mode == rmTowardNegative);
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
      fs = addOrSubtract(addend, rounding_mode, असत्य);
  }

  फर्काऊ fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::opStatus IEEEFloat::roundToIntegral(roundingMode rounding_mode) {
  यदि (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    फर्काऊ opOK;

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
      फर्काऊ opInvalidOp;
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
      फर्काऊ opOK;
    }
  }

  यदि (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    फर्काऊ opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  यदि (exponent + 1 >= (पूर्णाङ्क)APFloat::semanticsPrecision(*semantics))
    फर्काऊ opOK;

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
  तार्किक आगत_Sign = isNegative();

  fs = add(MagicConstant, rounding_mode);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  subtract(MagicConstant, rounding_mode);

  // Restore the input sign.
  यदि (आगत_Sign != isNegative())
    changeSign();

  फर्काऊ fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmpResult IEEEFloat::compare(अचर IEEEFloat &rhs) अचर {
  प्रतिज्ञा(semantics == rhs.semantics);

  विकल्प (Pack_Categories_Into_साँचो(category, rhs.category)) {
  स्वभाव:
    llvm_unreachable(nullptr);

  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcZero):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcNormal):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcNaN, fcNaN):
  पक्ष Pack_Categories_Into_साँचो(fcZero, fcNaN):
  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcNaN):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcNaN):
    फर्काऊ cmpUnordered;

  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcNormal):
  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcZero):
  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcZero):
    यदि (sign)
      फर्काऊ cmpLessThan;
    अन्यथा
      फर्काऊ cmpGreaterThan;

  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcZero, fcInfinity):
  पक्ष Pack_Categories_Into_साँचो(fcZero, fcNormal):
    यदि (rhs.sign)
      फर्काऊ cmpGreaterThan;
    अन्यथा
      फर्काऊ cmpLessThan;

  पक्ष Pack_Categories_Into_साँचो(fcInfinity, fcInfinity):
    यदि (sign == rhs.sign)
      फर्काऊ cmpEqual;
    अन्यथा यदि (sign)
      फर्काऊ cmpLessThan;
    अन्यथा
      फर्काऊ cmpGreaterThan;

  पक्ष Pack_Categories_Into_साँचो(fcZero, fcZero):
    फर्काऊ cmpEqual;

  पक्ष Pack_Categories_Into_साँचो(fcNormal, fcNormal):
    रोक;
  }

  cmpResult result;
  /* Two normal numbers.  Do they have the same sign?  */
  यदि (sign != rhs.sign) {
    यदि (sign)
      result = cmpLessThan;
    अन्यथा
      result = cmpGreaterThan;
  } अन्यथा {
    /* Compare absolute values; invert result if negative.  */
    result = compare_Absolute_मान(rhs);

    यदि (sign) {
      यदि (result == cmpLessThan)
        result = cmpGreaterThan;
      अन्यथा यदि (result == cmpGreaterThan)
        result = cmpLessThan;
    }
  }

  फर्काऊ result;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::opStatus IEEEFloat::convert(अचर fltSemantics &toSemantics,
                                     roundingMode rounding_mode,
                                     तार्किक *losesInfo) {
  opStatus fs;
  अचर fltSemantics &fromSemantics = *semantics;
  तार्किक is_signaling = isSignaling();

  lostFraction lostFraction = lfExactlyZero;
  अचिन्हित new_Part_गन्ती = part_गन्ती_For_Bits(toSemantics.precision + 1);
  अचिन्हित old_Part_गन्ती = part_गन्ती();
  पूर्णाङ्क shift = toSemantics.precision - fromSemantics.precision;

  तार्किक X86SpecialNan = असत्य;
  यदि (&fromSemantics == &APFloatBase::semX87DoubleExtended &&
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
  यदि (shift < 0 && isFiniteNonZero()) {
    पूर्णाङ्क omsb = significandMSB() + 1;
    पूर्णाङ्क exponentChange = omsb - fromSemantics.precision;
    यदि (exponent + exponentChange < toSemantics.minExponent)
      exponentChange = toSemantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, shift);
    यदि (exponentChange < 0) {
      shift -= exponentChange;
      exponent += exponentChange;
    } अन्यथा यदि (omsb <= -shift) {
      exponentChange = omsb + shift - 1; // leave at least one bit set
      shift -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  यदि (shift < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    lostFraction = shift_दायाँ(significandParts(), old_Part_गन्ती, -shift);

  // Fix the storage so it can hold to new value.
  यदि (new_Part_गन्ती > old_Part_गन्ती) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = नूतन integerPart[new_Part_गन्ती];
    APInt::tcSet(newParts, 0, new_Part_गन्ती);
    यदि (isFiniteNonZero() || category==fcNaN)
      APInt::tcAssign(newParts, significandParts(), old_Part_गन्ती);
    freeSignificand();
    significand.parts = newParts;
  } अन्यथा यदि (new_Part_गन्ती == 1 && old_Part_गन्ती != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    यदि (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &toSemantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  यदि (shift > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_Shift_बायाँ(significandParts(), new_Part_गन्ती, shift);

  यदि (isFiniteNonZero()) {
    fs = normalize(rounding_mode, lostFraction);
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

      *losesInfo = lostFraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      यदि (!X86SpecialNan && semantics == &APFloatBase::semX87DoubleExtended)
        APInt::tcSetBit(significandParts(), semantics->precision - 1);

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
  फर्काऊ fs;
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
    MutableArrayRef<integerPart> parts, अचिन्हित पूर्णाङ्क width, तार्किक isSigned,
    roundingMode rounding_mode, तार्किक *isExact) अचर {
  *isExact = असत्य;

  /* Handle the three special cases first.  */
  यदि (category == fcInfinity || category == fcNaN)
    फर्काऊ opInvalidOp;

  अचिन्हित dst_Parts_गन्ती = part_गन्ती_For_Bits(width);
  प्रतिज्ञा(dst_Parts_गन्ती <= parts.size() && "Integer too big");

  यदि (category == fcZero) {
    APInt::tcSet(parts.data(), 0, dst_Parts_गन्ती);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    फर्काऊ opOK;
  }

  अचर integerPart *src = significandParts();

  अचिन्हित truncatedBits;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  यदि (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tcSet(parts.data(), 0, dst_Parts_गन्ती);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncatedBits = semantics->precision -1U - exponent;
  } अन्यथा {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    अचिन्हित पूर्णाङ्क bits = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    यदि (bits > width)
      फर्काऊ opInvalidOp;

    यदि (bits < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncatedBits = semantics->precision - bits;
      APInt::tcExtract(parts.data(), dst_Parts_गन्ती, src, bits, truncatedBits);
    } अन्यथा {
      /* We want at least as many bits as are available.  */
      APInt::tcExtract(parts.data(), dst_Parts_गन्ती, src, semantics->precision,
                       0);
      APInt::tc_Shift_बायाँ(parts.data(), dst_Parts_गन्ती,
                         bits - semantics->precision);
      truncatedBits = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  lostFraction lost_fraction;
  यदि (truncatedBits) {
    lost_fraction = lostFractionThroughTruncation(src, part_गन्ती(),
                                                  truncatedBits);
    यदि (lost_fraction != lfExactlyZero &&
        roundAwayFromZero(rounding_mode, lost_fraction, truncatedBits)) {
      यदि (APInt::tcIncrement(parts.data(), dst_Parts_गन्ती))
        फर्काऊ opInvalidOp;     /* Overflow.  */
    }
  } अन्यथा {
    lost_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  अचिन्हित पूर्णाङ्क omsb = APInt::tcMSB(parts.data(), dst_Parts_गन्ती) + 1;

  यदि (sign) {
    यदि (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      यदि (omsb != 0)
        फर्काऊ opInvalidOp;
    } अन्यथा {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      यदि (omsb == width &&
          APInt::tcLSB(parts.data(), dst_Parts_गन्ती) + 1 != omsb)
        फर्काऊ opInvalidOp;

      /* This case can happen because of rounding.  */
      यदि (omsb > width)
        फर्काऊ opInvalidOp;
    }

    APInt::tcNegate (parts.data(), dst_Parts_गन्ती);
  } अन्यथा {
    यदि (omsb >= width + !isSigned)
      फर्काऊ opInvalidOp;
  }

  यदि (lost_fraction == lfExactlyZero) {
    *isExact = सत्य;
    फर्काऊ opOK;
  }
  फर्काऊ opInexact;
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
                            अचिन्हित पूर्णाङ्क width, तार्किक isSigned,
                            roundingMode rounding_mode, तार्किक *isExact) अचर {
  opStatus fs = convertToSignExtendedInteger(parts, width, isSigned,
                                             rounding_mode, isExact);

  यदि (fs == opInvalidOp) {
    अचिन्हित पूर्णाङ्क bits, dst_Parts_गन्ती;

    dst_Parts_गन्ती = part_गन्ती_For_Bits(width);
    प्रतिज्ञा(dst_Parts_गन्ती <= parts.size() && "Integer too big");

    यदि (category == fcNaN)
      bits = 0;
    अन्यथा यदि (sign)
      bits = isSigned;
    अन्यथा
      bits = width - isSigned;

    tcSetLeastSignificantBits(parts.data(), dst_Parts_गन्ती, bits);
    यदि (sign && isSigned)
      APInt::tc_Shift_बायाँ(parts.data(), dst_Parts_गन्ती, width - 1);
  }

  फर्काऊ fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::opStatus IEEEFloat::convertFromUnsignedParts(
    अचर integerPart *src, अचिन्हित पूर्णाङ्क src_गन्ती, roundingMode rounding_mode) {
  category = fcNormal;
  अचिन्हित omsb = APInt::tcMSB(src, src_गन्ती) + 1;
  integerPart *dst = significandParts();
  अचिन्हित dst_गन्ती = part_गन्ती();
  अचिन्हित precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  lostFraction lost_fraction;
  यदि (precision <= omsb) {
    exponent = omsb - 1;
    lost_fraction = lostFractionThroughTruncation(src, src_गन्ती,
                                                  omsb - precision);
    APInt::tcExtract(dst, dst_गन्ती, src, precision, omsb - precision);
  } अन्यथा {
    exponent = precision - 1;
    lost_fraction = lfExactlyZero;
    APInt::tcExtract(dst, dst_गन्ती, src, omsb, 0);
  }

  फर्काऊ normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus IEEEFloat::convertFromAPInt(अचर APInt &Val, तार्किक isSigned,
                                              roundingMode rounding_mode) {
  अचिन्हित पूर्णाङ्क part_गन्ती = Val.get_अङ्क_शब्दहरू();
  APInt api = Val;

  sign = असत्य;
  यदि (isSigned && api.isNegative()) {
    sign = सत्य;
    api = -api;
  }

  फर्काऊ convertFromUnsignedParts(api.getRawData(), part_गन्ती, rounding_mode);
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        roundingMode rounding_mode) {
  lostFraction lost_fraction = lfExactlyZero;

  category = fcNormal;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  अचिन्हित parts_गन्ती = part_गन्ती();
  अचिन्हित bitPos = parts_गन्ती * integerPartWidth;
  तार्किक computedTrailingFraction = असत्य;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator end = s.end();
  StringRef::iterator dot;
  स्वतः PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  यदि (!PtrOrErr)
    फर्काऊ PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator firstSignificantDigit = p;

  जबसम्म (p != end) {
    integerPart hex_मान;

    यदि (*p == '.') {
      यदि (dot != end)
        फर्काऊ createError("String contains multiple dots");
      dot = p++;
      जारी;
    }

    hex_मान = hex_Digit_मान(*p);
    यदि (hex_मान == अच_पूर्णांक_उच्च)
      रोक;

    p++;

    // Store the number while we have space.
    यदि (bitPos) {
      bitPos -= 4;
      hex_मान <<= bitPos % integerPartWidth;
      significand[bitPos / integerPartWidth] |= hex_मान;
    } अन्यथा यदि (!computedTrailingFraction) {
      स्वतः FractOrErr = trailingHexadecimalFraction(p, end, hex_मान);
      यदि (!FractOrErr)
        फर्काऊ FractOrErr.takeError();
      lost_fraction = *FractOrErr;
      computedTrailingFraction = सत्य;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  यदि (p == end)
    फर्काऊ createError("Hex strings require an exponent");
  यदि (*p != 'p' && *p != 'P')
    फर्काऊ createError("Invalid character in significand");
  यदि (p == begin)
    फर्काऊ createError("Significand has no digits");
  यदि (dot != end && p - begin == 1)
    फर्काऊ createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  यदि (p != firstSignificantDigit) {
    पूर्णाङ्क expAdjustment;

    /* Implicit hexadecimal point?  */
    यदि (dot == end)
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
    expAdjustment -= parts_गन्ती * integerPartWidth;

    /* Adjust for the given exponent.  */
    स्वतः ExpOrErr = totalExponent(p + 1, end, expAdjustment);
    यदि (!ExpOrErr)
      फर्काऊ ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  फर्काऊ normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus
IEEEFloat::roundSignificandWithExponent(अचर integerPart *decSigParts,
                                        अचिन्हित sig_Part_गन्ती, पूर्णाङ्क exp,
                                        roundingMode rounding_mode) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  अचिन्हित पूर्णाङ्क power = exp >= 0 ? exp : -exp;
  अचिन्हित पूर्णाङ्क pow5PartCapacity = powerOfFivePartsForExponent(power);
  SmallVector<integerPart, अङ्क_Pow5_Parts_On_थाक> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5PartCapacity);

  तार्किक isNearest = rounding_mode == rmNearestTiesToEven ||
                   rounding_mode == rmNearestTiesToAway;

  अचिन्हित parts = part_गन्ती_For_Bits(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  अचिन्हित pow5_Part_गन्ती = powerOf5(pow5Parts.data(), power);

  लागि (;; parts *= 2) {
    अचिन्हित पूर्णाङ्क excessPrecision, truncatedBits;

    calcSemantics.precision = parts * integerPartWidth - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncatedBits = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    opStatus sigStatus = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_गन्ती, rmNearestTiesToEven);
    opStatus powStatus = pow5.convertFromUnsignedParts(
        pow5Parts.data(), pow5_Part_गन्ती, rmNearestTiesToEven);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += exp;

    lostFraction calcLostFraction;
    integerPart HUerr, HUdistance;
    अचिन्हित पूर्णाङ्क powHUerr;

    यदि (exp >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calcLostFraction = decSig.multiplySignificand(pow5);
      powHUerr = powStatus != opOK;
    } अन्यथा {
      calcLostFraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      यदि (decSig.exponent < semantics->minExponent) {
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
    यदि (HUdistance >= HUerr) {
      APInt::tcExtract(significandParts(), part_गन्ती(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calcLostFraction = lostFractionThroughTruncation(decSig.significandParts(),
                                                       decSig.part_गन्ती(),
                                                       truncatedBits);
      फर्काऊ static_cast<opStatus>(normalize(rounding_mode, calcLostFraction) |
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
  यदि (Error Err = interpretDecimal(p, str.end(), &D))
    फर्काऊ std::move(Err);

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
  यदि (D.firstSigDigit == str.end() || dec_Digit_मान(*D.firstSigDigit) >= 10U) {
    category = fcZero;
    fs = opOK;
    यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = असत्य;
    यदि (!semantics->hasZero)
      makeSmallestNormalized(असत्य);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } अन्यथा यदि (D.normalizedExponent - 1 > पूर्णांक_उच्च / 42039) {
    fs = handleOverflow(rounding_mode);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } अन्यथा यदि (D.normalizedExponent - 1 < पूर्णांक_न्यून / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (पूर्णाङ्क) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fcNormal;
    zeroSignificand();
    fs = normalize(rounding_mode, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } अन्यथा यदि ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handleOverflow(rounding_mode);
  } अन्यथा {
    integerPart *decSignificand;
    अचिन्हित पूर्णाङ्क part_गन्ती;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_गन्ती = static_cast<अचिन्हित पूर्णाङ्क>(D.lastSigDigit - D.firstSigDigit) + 1;
    part_गन्ती = part_गन्ती_For_Bits(1 + 196 * part_गन्ती / 59);
    decSignificand = नूतन integerPart[part_गन्ती + 1];
    part_गन्ती = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    गर {
      integerPart dec_मान, val, multiplier;

      val = 0;
      multiplier = 1;

      गर {
        यदि (*p == '.') {
          p++;
          यदि (p == str.end()) {
            रोक;
          }
        }
        dec_मान = dec_Digit_मान(*p++);
        यदि (dec_मान >= 10U) {
          delete[] decSignificand;
          फर्काऊ createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_मान;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } जबसम्म (p <= D.lastSigDigit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_गन्ती, part_गन्ती + 1, असत्य);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      यदि (decSignificand[part_गन्ती])
        part_गन्ती++;
    } जबसम्म (p <= D.lastSigDigit);

    category = fcNormal;
    fs = roundSignificandWithExponent(decSignificand, part_गन्ती,
                                      D.exponent, rounding_mode);

    delete [] decSignificand;
  }

  फर्काऊ fs;
}

तार्किक IEEEFloat::convertFromStringSpecials(StringRef str) {
  अचर माप_प्रकार MIN_NAME_SIZE = 3;

  यदि (str.size() < MIN_NAME_SIZE)
    फर्काऊ असत्य;

  यदि (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(असत्य);
    फर्काऊ सत्य;
  }

  तार्किक IsNegative = str.consume_front("-");
  यदि (IsNegative) {
    यदि (str.size() < MIN_NAME_SIZE)
      फर्काऊ असत्य;

    यदि (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(सत्य);
      फर्काऊ सत्य;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  तार्किक IsSignaling = str.consume_front_insensitive("s");
  यदि (IsSignaling) {
    यदि (str.size() < MIN_NAME_SIZE)
      फर्काऊ असत्य;
  }

  यदि (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    यदि (str.empty()) {
      makeNaN(IsSignaling, IsNegative);
      फर्काऊ सत्य;
    }

    // Allow the payload to be inside parentheses.
    यदि (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      यदि (str.size() <= 2 || str.back() != ')')
        फर्काऊ असत्य;

      str = str.slice(1, str.size() - 1);
    }

    // Determine the payload number's radix.
    अचिन्हित Radix = 10;
    यदि (str[0] == '0') {
      यदि (str.size() > 1 && छोटे(str[1]) == 'x') {
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
      फर्काऊ सत्य;
    }
  }

  फर्काऊ असत्य;
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromString(StringRef str, roundingMode rounding_mode) {
  यदि (str.empty())
    फर्काऊ createError("Invalid string length");

  // Handle special cases.
  यदि (convertFromStringSpecials(str))
    फर्काऊ opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  माप_प्रकार slen = str.size();
  sign = *p == '-' ? 1 : 0;
  यदि (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  यदि (*p == '-' || *p == '+') {
    p++;
    slen--;
    यदि (!slen)
      फर्काऊ createError("String has no digits");
  }

  यदि (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    यदि (slen == 2)
      फर्काऊ createError("Invalid string");
    फर्काऊ convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_mode);
  }

  फर्काऊ convertFromDecimalString(StringRef(p, slen), rounding_mode);
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
अचिन्हित पूर्णाङ्क IEEEFloat::convertToHexString(वर्ण *dst, अचिन्हित पूर्णाङ्क hexDigits,
                                           तार्किक upperCase,
                                           roundingMode rounding_mode) अचर {
  वर्ण *p = dst;
  यदि (sign)
    *dst++ = '-';

  विकल्प (category) {
  पक्ष fcInfinity:
    स_नकल (dst, upperCase ? infinityU: infinityL, परिमाण infinityU - 1);
    dst += परिमाण infinityL - 1;
    रोक;

  पक्ष fcNaN:
    स_नकल (dst, upperCase ? NaNU: NaNL, परिमाण NaNU - 1);
    dst += परिमाण NaNU - 1;
    रोक;

  पक्ष fcZero:
    *dst++ = '0';
    *dst++ = upperCase ? 'X': 'x';
    *dst++ = '0';
    यदि (hexDigits > 1) {
      *dst++ = '.';
      स_रखो (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = upperCase ? 'P': 'p';
    *dst++ = '0';
    रोक;

  पक्ष fcNormal:
    dst = convertNormalToHexString (dst, hexDigits, upperCase, rounding_mode);
    रोक;
  }

  *dst = 0;

  फर्काऊ static_cast<अचिन्हित पूर्णाङ्क>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
वर्ण *IEEEFloat::convertNormalToHexString(वर्ण *dst, अचिन्हित पूर्णाङ्क hexDigits,
                                          तार्किक upperCase,
                                          roundingMode rounding_mode) अचर {
  *dst++ = '0';
  *dst++ = upperCase ? 'X': 'x';

  तार्किक roundUp = असत्य;
  अचर वर्ण *hexDigitChars = upperCase ? hexDigitsUpper : hexDigitsLower;

  अचर integerPart *significand = significandParts();
  अचिन्हित parts_गन्ती = part_गन्ती();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  अचिन्हित मान_Bits = semantics->precision + 3;
  अचिन्हित shift = integerPartWidth - मान_Bits % integerPartWidth;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  अचिन्हित outputDigits = (मान_Bits - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  यदि (hexDigits) {
    यदि (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      अचिन्हित पूर्णाङ्क bits;
      lostFraction fraction;

      bits = मान_Bits - hexDigits * 4;
      fraction = lostFractionThroughTruncation (significand, parts_गन्ती, bits);
      roundUp = roundAwayFromZero(rounding_mode, fraction, bits);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  वर्ण *p = ++dst;

  अचिन्हित गन्ती = (मान_Bits + integerPartWidth - 1) / integerPartWidth;

  जबसम्म (outputDigits && गन्ती) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    यदि (--गन्ती == parts_गन्ती)
      part = 0;  /* An imaginary higher zero part.  */
    अन्यथा
      part = significand[गन्ती] << shift;

    यदि (गन्ती && shift)
      part |= significand[गन्ती - 1] >> (integerPartWidth - shift);

    /* Convert as much of "part" to hexdigits as we can.  */
    अचिन्हित पूर्णाङ्क curDigits = integerPartWidth / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  यदि (roundUp) {
    वर्ण *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    गर {
      q--;
      *q = hexDigitChars[hex_Digit_मान (*q) + 1];
    } जबसम्म (*q == '0');
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
  यदि (dst -1 == p)
    dst--;
  अन्यथा
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = upperCase ? 'P': 'p';

  फर्काऊ writeSignedDecimal (dst, exponent);
}

hash_code hash_मान(अचर IEEEFloat &Arg) {
  यदि (!Arg.isFiniteNonZero())
    फर्काऊ hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  फर्काऊ hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.part_गन्ती()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convertF80LongDoubleAPFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती() == 2);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semX87DoubleExtended>();
}

APInt IEEEFloat::convertPPCDoubleDoubleLegacyAPFloatToAPInt() अचर {
  प्रतिज्ञा(semantics ==
         (अचर llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy);
  प्रतिज्ञा(part_गन्ती()==2);

  uint64_t शब्दहरू[2];
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
  शब्दहरू[0] = *u.convertDoubleAPFloatToAPInt().getRawData();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  यदि (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
    प्रतिज्ञा(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.subtract(u, rmNearestTiesToEven);
    fs = v.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
    प्रतिज्ञा(fs == opOK && !losesInfo);
    शब्दहरू[1] = *v.convertDoubleAPFloatToAPInt().getRawData();
  } अन्यथा {
    शब्दहरू[1] = 0;
  }

  फर्काऊ APInt(128, शब्दहरू);
}

ढाँचा <अचर fltSemantics &S>
APInt IEEEFloat::convertIEEEFloatToAPInt() अचर {
  प्रतिज्ञा(semantics == &S);
  constexpr अचिन्हित पूर्णाङ्क trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr पूर्णाङ्क integer_bit_part = (S.precision - 1) / integerPartWidth;
  constexpr integerPart integer_bit = integerPart{1}
                                      << ((S.precision - 1) % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr अचिन्हित पूर्णाङ्क exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr तार्किक is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr पूर्णाङ्क bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_गन्ती_For_Bits(trailing_significand_bits)>
      mysignificand;

  यदि (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.size(),
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
      mysignificand[0] = integerPart{1} << (trailing_significand_bits - 1);
    }
  } अन्यथा {
    प्रतिज्ञा(category == fcNaN && "Unknown category!");
    यदि (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.sizeInBits + 63) / 64> शब्दहरू;
  स्वतः शब्दहरू_iter =
      std::copy_n(mysignificand.begin(), mysignificand.size(), शब्दहरू.begin());
  यदि constexpr (!S.hasExplicitIntegerBit) {
    यदि constexpr (significand_mask != 0 || trailing_significand_bits == 0) {
      // Clear the integer bit.
      शब्दहरू[mysignificand.size() - 1] &= significand_mask;
    }
  }
  std::fill(शब्दहरू_iter, शब्दहरू.end(), uint64_t{0});
  constexpr माप_प्रकार last_word = शब्दहरू.size() - 1;
  यदि constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.sizeInBits - 1) % 64);
    शब्दहरू[last_word] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_mask)
                              << (trailing_significand_bits % 64);
  शब्दहरू[last_word] |= shifted_exponent;
  यदि constexpr (last_word == 0) {
    फर्काऊ APInt(S.sizeInBits, शब्दहरू[0]);
  }
  फर्काऊ APInt(S.sizeInBits, शब्दहरू);
}

APInt IEEEFloat::convertQuadrupleAPFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती() == 2);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semIEEEquad>();
}

APInt IEEEFloat::convertDoubleAPFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती()==1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semIEEEdouble>();
}

APInt IEEEFloat::convertFloatAPFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती()==1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semIEEEsingle>();
}

APInt IEEEFloat::convertBFloatAPFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती() == 1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semBFloat>();
}

APInt IEEEFloat::convertHalfAPFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती()==1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::APFloatBase::semIEEEhalf>();
}

APInt IEEEFloat::convertFloat8E5M2APFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती() == 1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2>();
}

APInt IEEEFloat::convertFloat8E5M2FNUZAPFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती() == 1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3APFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती() == 1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3>();
}

APInt IEEEFloat::convertFloat8E4M3FNAPFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती() == 1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FN>();
}

APInt IEEEFloat::convertFloat8E4M3FNUZAPFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती() == 1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3B11FNUZAPFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती() == 1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convertFloat8E3M4APFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती() == 1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semFloat8E3M4>();
}

APInt IEEEFloat::convertFloatTF32APFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती() == 1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semFloatTF32>();
}

APInt IEEEFloat::convertFloat8E8M0FNUAPFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती() == 1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convertFloat8E5M3FNUAPFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती() == 1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convertFloat6E3M2FNAPFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती() == 1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semFloat6E3M2FN>();
}

APInt IEEEFloat::convertFloat6E2M3FNAPFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती() == 1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semFloat6E2M3FN>();
}

APInt IEEEFloat::convertFloat4E2M1FNAPFloatToAPInt() अचर {
  प्रतिज्ञा(part_गन्ती() == 1);
  फर्काऊ convertIEEEFloatToAPInt<APFloatBase::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcastToAPInt() अचर {
  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semIEEEhalf)
    फर्काऊ convertHalfAPFloatToAPInt();

  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semBFloat)
    फर्काऊ convertBFloatAPFloatToAPInt();

  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semIEEEsingle)
    फर्काऊ convertFloatAPFloatToAPInt();

  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semIEEEdouble)
    फर्काऊ convertDoubleAPFloatToAPInt();

  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semIEEEquad)
    फर्काऊ convertQuadrupleAPFloatToAPInt();

  यदि (semantics ==
      (अचर llvm::fltSemantics *)&APFloatBase::semX87DoubleExtended)
    फर्काऊ convertF80LongDoubleAPFloatToAPInt();

  यदि (semantics ==
      (अचर llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy)
    फर्काऊ convertPPCDoubleDoubleLegacyAPFloatToAPInt();

  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semFloat8E5M2)
    फर्काऊ convertFloat8E5M2APFloatToAPInt();

  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semFloat8E5M2FNUZ)
    फर्काऊ convertFloat8E5M2FNUZAPFloatToAPInt();

  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semFloat8E4M3)
    फर्काऊ convertFloat8E4M3APFloatToAPInt();

  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FN)
    फर्काऊ convertFloat8E4M3FNAPFloatToAPInt();

  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FNUZ)
    फर्काऊ convertFloat8E4M3FNUZAPFloatToAPInt();

  यदि (semantics ==
      (अचर llvm::fltSemantics *)&APFloatBase::semFloat8E4M3B11FNUZ)
    फर्काऊ convertFloat8E4M3B11FNUZAPFloatToAPInt();

  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semFloat8E3M4)
    फर्काऊ convertFloat8E3M4APFloatToAPInt();

  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semFloatTF32)
    फर्काऊ convertFloatTF32APFloatToAPInt();

  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semFloat8E8M0FNU)
    फर्काऊ convertFloat8E8M0FNUAPFloatToAPInt();

  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semFloat8E5M3FNU)
    फर्काऊ convertFloat8E5M3FNUAPFloatToAPInt();

  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semFloat6E3M2FN)
    फर्काऊ convertFloat6E3M2FNAPFloatToAPInt();

  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semFloat6E2M3FN)
    फर्काऊ convertFloat6E2M3FNAPFloatToAPInt();

  यदि (semantics == (अचर llvm::fltSemantics *)&APFloatBase::semFloat4E2M1FN)
    फर्काऊ convertFloat4E2M1FNAPFloatToAPInt();

  llvm_unreachable("unknown format!");
}

भिन्न IEEEFloat::convertToFloat() अचर {
  प्रतिज्ञा(semantics == (अचर llvm::fltSemantics *)&APFloatBase::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcastToAPInt();
  फर्काऊ api.bitsToFloat();
}

द्विगुण IEEEFloat::convertToDouble() अचर {
  प्रतिज्ञा(semantics == (अचर llvm::fltSemantics *)&APFloatBase::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcastToAPInt();
  फर्काऊ api.bitsToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 IEEEFloat::convertToQuad() अचर {
  प्रतिज्ञा(semantics == (अचर llvm::fltSemantics *)&APFloatBase::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcastToAPInt();
  फर्काऊ api.bitsToQuad();
}
#endif

शून्य IEEEFloat::initFromF80LongDoubleAPInt(अचर APInt &api) {
  फर्काऊ initFromIEEEAPInt<APFloatBase::semX87DoubleExtended>(api);
}

शून्य IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(अचर APInt &api) {
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
  यदि (isFiniteNonZero()) {
    IEEEFloat v(APFloatBase::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    यदि (v.isFinite()) {
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
शून्य IEEEFloat::initFromFloat8E8M0FNUAPInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E8M0FNU>(api);
}

शून्य IEEEFloat::initFromFloat8E5M3FNUAPInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M3FNU>(api);
}

ढाँचा <अचर fltSemantics &S>
शून्य IEEEFloat::initFromIEEEAPInt(अचर APInt &api) {
  प्रतिज्ञा(api.getBitWidth() == S.sizeInBits);

  constexpr अचिन्हित पूर्णाङ्क trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr integerPart integer_bit =
      integerPart{1} << (trailing_significand_bits % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr अचिन्हित पूर्णाङ्क exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr अचिन्हित पूर्णाङ्क stored_significand_parts =
      part_गन्ती_For_Bits(trailing_significand_bits + 1);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr तार्किक is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr पूर्णाङ्क bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr तार्किक has_significand = trailing_significand_bits > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  यदि constexpr (has_significand) {
    std::copy_n(api.getRawData(), mysignificand.size(), mysignificand.begin());
    यदि constexpr (significand_mask != 0 || S.precision >= integerPartWidth) {
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
  uint64_t last_word = api.getRawData()[api.get_अङ्क_शब्दहरू() - 1];
  uint64_t myexponent =
      (last_word >> (trailing_significand_bits % 64)) & exponent_mask;

  initialize(&S);
  प्रतिज्ञा(part_गन्ती() == mysignificand.size());

  sign = S.hasSignedRepr
             ? static_cast<अचिन्हित पूर्णाङ्क>(last_word >> ((S.sizeInBits - 1) % 64))
             : 0;

  तार्किक all_zero_significand =
      has_significand && llvm::all_of(mysignificand, equal_to(0));

  तार्किक is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  यदि constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    तार्किक is_inf = असत्य;

    यदि constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;
      अचर integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_mask_no_int_bit) == 0;
    } अन्यथा {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    यदि (is_inf) {
      makeInf(sign);
      फर्काऊ;
    }
  }

  तार्किक is_nan = असत्य;

  यदि constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    यदि constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      अचर integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;

      यदि (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_mask_no_int_bit) != 0) {
        // regular NaN and pseudoNaN
        is_nan = सत्य;
      } अन्यथा यदि (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_mask_no_int_bit) == 0) {
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
        std::all_of(mysignificand.begin(), mysignificand.end() - 1,
                    [](integerPart bits) { फर्काऊ bits == ~integerPart{0}; }) &&
        (!significand_mask ||
         mysignificand[mysignificand.size() - 1] == significand_mask);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } अन्यथा यदि constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  यदि (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.size(),
                significandParts());
    फर्काऊ;
  }

  यदि (is_zero) {
    makeZero(sign);
    फर्काऊ;
  }

  category = fcNormal;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.size(), significandParts());
  यदि (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  अन्यथा {
    यदि constexpr (!S.hasExplicitIntegerBit) {
      significandParts()[mysignificand.size() - 1] |= integer_bit;
    }
  }
}

शून्य IEEEFloat::initFromQuadrupleAPInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEquad>(api);
}

शून्य IEEEFloat::initFromDoubleAPInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEdouble>(api);
}

शून्य IEEEFloat::initFromFloatAPInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEsingle>(api);
}

शून्य IEEEFloat::initFromBFloatAPInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semBFloat>(api);
}

शून्य IEEEFloat::initFromHalfAPInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEhalf>(api);
}

शून्य IEEEFloat::initFromFloat8E5M2APInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2>(api);
}

शून्य IEEEFloat::initFromFloat8E5M2FNUZAPInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2FNUZ>(api);
}

शून्य IEEEFloat::initFromFloat8E4M3APInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3>(api);
}

शून्य IEEEFloat::initFromFloat8E4M3FNAPInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FN>(api);
}

शून्य IEEEFloat::initFromFloat8E4M3FNUZAPInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FNUZ>(api);
}

शून्य IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3B11FNUZ>(api);
}

शून्य IEEEFloat::initFromFloat8E3M4APInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E3M4>(api);
}

शून्य IEEEFloat::initFromFloatTF32APInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloatTF32>(api);
}

शून्य IEEEFloat::initFromFloat6E3M2FNAPInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E3M2FN>(api);
}

शून्य IEEEFloat::initFromFloat6E2M3FNAPInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E2M3FN>(api);
}

शून्य IEEEFloat::initFromFloat4E2M1FNAPInt(अचर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
शून्य IEEEFloat::initFromAPInt(अचर fltSemantics *Sem, अचर APInt &api) {
  प्रतिज्ञा(api.getBitWidth() == Sem->sizeInBits);
  यदि (Sem == &APFloatBase::semIEEEhalf)
    फर्काऊ initFromHalfAPInt(api);
  यदि (Sem == &APFloatBase::semBFloat)
    फर्काऊ initFromBFloatAPInt(api);
  यदि (Sem == &APFloatBase::semIEEEsingle)
    फर्काऊ initFromFloatAPInt(api);
  यदि (Sem == &APFloatBase::semIEEEdouble)
    फर्काऊ initFromDoubleAPInt(api);
  यदि (Sem == &APFloatBase::semIEEEquad)
    फर्काऊ initFromQuadrupleAPInt(api);
  यदि (Sem == &APFloatBase::semX87DoubleExtended)
    फर्काऊ initFromF80LongDoubleAPInt(api);
  यदि (Sem == &APFloatBase::semPPCDoubleDoubleLegacy)
    फर्काऊ initFromPPCDoubleDoubleLegacyAPInt(api);
  यदि (Sem == &APFloatBase::semFloat8E5M2)
    फर्काऊ initFromFloat8E5M2APInt(api);
  यदि (Sem == &APFloatBase::semFloat8E5M2FNUZ)
    फर्काऊ initFromFloat8E5M2FNUZAPInt(api);
  यदि (Sem == &APFloatBase::semFloat8E4M3)
    फर्काऊ initFromFloat8E4M3APInt(api);
  यदि (Sem == &APFloatBase::semFloat8E4M3FN)
    फर्काऊ initFromFloat8E4M3FNAPInt(api);
  यदि (Sem == &APFloatBase::semFloat8E4M3FNUZ)
    फर्काऊ initFromFloat8E4M3FNUZAPInt(api);
  यदि (Sem == &APFloatBase::semFloat8E4M3B11FNUZ)
    फर्काऊ initFromFloat8E4M3B11FNUZAPInt(api);
  यदि (Sem == &APFloatBase::semFloat8E3M4)
    फर्काऊ initFromFloat8E3M4APInt(api);
  यदि (Sem == &APFloatBase::semFloatTF32)
    फर्काऊ initFromFloatTF32APInt(api);
  यदि (Sem == &APFloatBase::semFloat8E8M0FNU)
    फर्काऊ initFromFloat8E8M0FNUAPInt(api);
  यदि (Sem == &APFloatBase::semFloat8E5M3FNU)
    फर्काऊ initFromFloat8E5M3FNUAPInt(api);
  यदि (Sem == &APFloatBase::semFloat6E3M2FN)
    फर्काऊ initFromFloat6E3M2FNAPInt(api);
  यदि (Sem == &APFloatBase::semFloat6E2M3FN)
    फर्काऊ initFromFloat6E2M3FNAPInt(api);
  यदि (Sem == &APFloatBase::semFloat4E2M1FN)
    फर्काऊ initFromFloat4E2M1FNAPInt(api);

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
  category = fcNormal;
  sign = Negative;
  exponent = semantics->maxExponent;

  // Use memset to set all but the highest integerPart to all ones.
  integerPart *significand = significandParts();
  अचिन्हित Part_गन्ती = part_गन्ती();
  स_रखो(significand, 0xFF, परिमाण(integerPart)*(Part_गन्ती - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  अचर अचिन्हित अङ्क_Unused_माथिल्लो_Bits =
    Part_गन्ती*integerPartWidth - semantics->precision;
  significand[Part_गन्ती - 1] = (अङ्क_Unused_माथिल्लो_Bits < integerPartWidth)
                                   ? (~integerPart(0) >> अङ्क_Unused_माथिल्लो_Bits)
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
  category = fcNormal;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tcSet(significandParts(), 1, part_गन्ती());
}

शून्य IEEEFloat::makeSmallestNormalized(तार्किक Negative) {
  यदि (Negative && !semantics->hasSignedRepr)
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

IEEEFloat::IEEEFloat(अचर fltSemantics &Sem, अचर APInt &API) {
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
                         पूर्णाङ्क &exp, अचिन्हित FormatPrecision) {
    अचिन्हित bits = significand.getActiveBits();

    // 196/59 is a very slight overestimate of lg_2(10).
    अचिन्हित bitsRequired = (FormatPrecision * 196 + 58) / 59;

    यदि (bits <= bitsRequired) फर्काऊ;

    अचिन्हित tensRemovable = (bits - bitsRequired) * 59 / 196;
    यदि (!tensRemovable) फर्काऊ;

    exp += tensRemovable;

    APInt भाजक(significand.getBitWidth(), 1);
    APInt powten(significand.getBitWidth(), 10);
    जबसम्म (सत्य) {
      यदि (tensRemovable & 1)
        भाजक *= powten;
      tensRemovable >>= 1;
      यदि (!tensRemovable) रोक;
      powten *= powten;
    }

    significand = significand.udiv(भाजक);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.getActiveBits());
  }


  शून्य AdjustToPrecision(SmallVectorImpl<वर्ण> &buffer,
                         पूर्णाङ्क &exp, अचिन्हित FormatPrecision) {
    अचिन्हित N = buffer.size();
    यदि (N <= FormatPrecision) फर्काऊ;

    // The most significant figures are the last ones in the buffer.
    अचिन्हित FirstSignificant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    यदि (buffer[FirstSignificant - 1] < '5') {
      जबसम्म (FirstSignificant < N && buffer[FirstSignificant] == '0')
        FirstSignificant++;

      exp += FirstSignificant;
      buffer.erase(&buffer[0], &buffer[FirstSignificant]);
      फर्काऊ;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    लागि (अचिन्हित I = FirstSignificant; I != N; ++I) {
      यदि (buffer[I] == '9') {
        FirstSignificant++;
      } अन्यथा {
        buffer[I]++;
        रोक;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    यदि (FirstSignificant == N) {
      exp += FirstSignificant;
      buffer.clear();
      buffer.थप_back('1');
      फर्काऊ;
    }

    exp += FirstSignificant;
    buffer.erase(&buffer[0], &buffer[FirstSignificant]);
  }

  शून्य toStringImpl(SmallVectorImpl<वर्ण> &Str, अचर तार्किक isNeg, पूर्णाङ्क exp,
                    APInt significand, अचिन्हित FormatPrecision,
                    अचिन्हित FormatMaxPadding, तार्किक TruncateZero) {
    अचर पूर्णाङ्क semanticsPrecision = significand.getBitWidth();

    यदि (isNeg)
      Str.थप_back('-');

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
    exp += trailingZeros;
    significand.lshrInPlace(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    यदि (exp == 0) {
      // Nothing to do.
    } अन्यथा यदि (exp > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + exp);
      significand <<= exp;
      exp = 0;
    } अन्यथा { /* exp < 0 */
      पूर्णाङ्क texp = -exp;

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
      जबसम्म (सत्य) {
        यदि (texp & 1)
          significand *= five_to_the_i;

        texp >>= 1;
        यदि (!texp)
          रोक;
        five_to_the_i *= five_to_the_i;
      }
    }

    AdjustToPrecision(significand, exp, FormatPrecision);

    SmallVector<वर्ण, 256> buffer;

    // Fill the buffer.
    अचिन्हित precision = significand.getBitWidth();
    यदि (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    तार्किक inTrail = सत्य;
    जबसम्म (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      अचिन्हित d = digit.get_ZExt_मान();

      // Drop trailing zeros.
      यदि (inTrail && !d)
        exp++;
      अन्यथा {
        buffer.थप_back((वर्ण) ('0' + d));
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
    यदि (!FormatMaxPadding) {
      FormatScientific = सत्य;
    } अन्यथा {
      यदि (exp >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((अचिन्हित) exp > FormatMaxPadding ||
                            NDigits + (अचिन्हित) exp > FormatPrecision);
      } अन्यथा {
        // Power of the most significant digit.
        पूर्णाङ्क MSD = exp + (पूर्णाङ्क) (NDigits - 1);
        यदि (MSD >= 0) {
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
    यदि (FormatScientific) {
      exp += (NDigits - 1);

      Str.थप_back(buffer[NDigits-1]);
      Str.थप_back('.');
      यदि (NDigits == 1 && TruncateZero)
        Str.थप_back('0');
      अन्यथा
        लागि (अचिन्हित I = 1; I != NDigits; ++I)
          Str.थप_back(buffer[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      यदि (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.थप_back(TruncateZero ? 'E' : 'e');

      Str.थप_back(exp >= 0 ? '+' : '-');
      यदि (exp < 0)
        exp = -exp;
      SmallVector<वर्ण, 6> expbuf;
      गर {
        expbuf.थप_back((वर्ण) ('0' + (exp % 10)));
        exp /= 10;
      } जबसम्म (exp);
      // Exponent always at least two digits if we do not truncate zeros.
      यदि (!TruncateZero && expbuf.size() < 2)
        expbuf.थप_back('0');
      लागि (अचिन्हित I = 0, E = expbuf.size(); I != E; ++I)
        Str.थप_back(expbuf[E-1-I]);
      फर्काऊ;
    }

    // Non-scientific, positive exponents.
    यदि (exp >= 0) {
      लागि (अचिन्हित I = 0; I != NDigits; ++I)
        Str.थप_back(buffer[NDigits-1-I]);
      लागि (अचिन्हित I = 0; I != (अचिन्हित) exp; ++I)
        Str.थप_back('0');
      फर्काऊ;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    पूर्णाङ्क NWholeDigits = exp + (पूर्णाङ्क) NDigits;

    अचिन्हित I = 0;
    यदि (NWholeDigits > 0) {
      लागि (; I != (अचिन्हित) NWholeDigits; ++I)
        Str.थप_back(buffer[NDigits-I-1]);
      Str.थप_back('.');
    } अन्यथा {
      अचिन्हित NZeros = 1 + (अचिन्हित) -NWholeDigits;

      Str.थप_back('0');
      Str.थप_back('.');
      लागि (अचिन्हित Z = 1; Z != NZeros; ++Z)
        Str.थप_back('0');
    }

    लागि (; I != NDigits; ++I)
      Str.थप_back(buffer[NDigits-I-1]);

  }
} // namespace

शून्य IEEEFloat::toString(SmallVectorImpl<वर्ण> &Str, अचिन्हित FormatPrecision,
                         अचिन्हित FormatMaxPadding, तार्किक TruncateZero) अचर {
  विकल्प (category) {
  पक्ष fcInfinity:
    यदि (isNegative())
      फर्काऊ append(Str, "-Inf");
    अन्यथा
      फर्काऊ append(Str, "+Inf");

  पक्ष fcNaN: फर्काऊ append(Str, "NaN");

  पक्ष fcZero:
    यदि (isNegative())
      Str.थप_back('-');

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
      Str.थप_back('0');
    }
    फर्काऊ;

  पक्ष fcNormal:
    रोक;
  }

  // Decompose the number into an APInt and an exponent.
  पूर्णाङ्क exp = exponent - ((पूर्णाङ्क) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_गन्ती_For_Bits(semantics->precision)));

  toStringImpl(Str, isNegative(), exp, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

पूर्णाङ्क IEEEFloat::getExactLog2Abs() अचर {
  यदि (!isFinite() || isZero())
    फर्काऊ पूर्णांक_न्यून;

  अचर integerPart *Parts = significandParts();
  अचर पूर्णाङ्क Part_गन्ती = part_गन्ती_For_Bits(semantics->precision);

  पूर्णाङ्क झिक_गन्ती = 0;
  लागि (पूर्णाङ्क i = 0; i < Part_गन्ती; ++i) {
    झिक_गन्ती += llvm::popcount(Parts[i]);
    यदि (झिक_गन्ती > 1)
      फर्काऊ पूर्णांक_न्यून;
  }

  यदि (exponent != semantics->minExponent)
    फर्काऊ exponent;

  पूर्णाङ्क CountrParts = 0;
  लागि (पूर्णाङ्क i = 0; i < Part_गन्ती;
       ++i, CountrParts += APInt::APINT_BITS_PER_WORD) {
    यदि (Parts[i] != 0) {
      फर्काऊ exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

तार्किक IEEEFloat::isSignaling() अचर {
  यदि (!isNaN())
    फर्काऊ असत्य;
  यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    फर्काऊ असत्य;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  फर्काऊ !APInt::tcExtractBit(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::opStatus IEEEFloat::next(तार्किक nextDown) {
  // If we are performing nextDown, swap sign so we have -x.
  यदि (nextDown)
    changeSign();

  // Compute nextUp(x)
  opStatus result = opOK;

  // Handle each float category separately.
  विकल्प (category) {
  पक्ष fcInfinity:
    // nextUp(+inf) = +inf
    यदि (!isNegative())
      रोक;
    // nextUp(-inf) = -getLargest()
    makeLargest(सत्य);
    रोक;
  पक्ष fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    यदि (isSignaling()) {
      result = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(असत्य, isNegative(), nullptr);
    }
    रोक;
  पक्ष fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(असत्य);
    रोक;
  पक्ष fcNormal:
    // nextUp(-getSmallest()) = -0
    यदि (isSmallest() && isNegative()) {
      APInt::tcSet(significandParts(), 0, part_गन्ती());
      category = fcZero;
      exponent = 0;
      यदि (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = असत्य;
      यदि (!semantics->hasZero)
        makeSmallestNormalized(असत्य);
      रोक;
    }

    यदि (isLargest() && !isNegative()) {
      यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        रोक;
      } अन्यथा यदि (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        रोक;
      } अन्यथा {
        // nextUp(getLargest()) == INFINITY
        APInt::tcSet(significandParts(), 0, part_गन्ती());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        रोक;
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
      APInt::tcDecrement(Parts, part_गन्ती());

      यदि (WillCrossBinadeBoundary) {
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

      यदि (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tcSet(Parts, 0, part_गन्ती());
        APInt::tcSetBit(Parts, semantics->precision - 1);
        प्रतिज्ञा(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } अन्यथा {
        incrementSignificand();
      }
    }
    रोक;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  यदि (nextDown)
    changeSign();

  फर्काऊ result;
}

APInt IEEEFloat::getNaNPayload() अचर {
  प्रतिज्ञा(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  अचिन्हित Bits = semantics->precision - 1;
  फर्काऊ APInt(Bits, ArrayRef(significandParts(), part_गन्ती_For_Bits(Bits)));
}

APFloatBase::ExponentType IEEEFloat::exponentNaN() अचर {
  फर्काऊ ::exponentNaN(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentInf() अचर {
  फर्काऊ ::exponentInf(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentZero() अचर {
  फर्काऊ ::exponentZero(*semantics);
}

शून्य IEEEFloat::makeInf(तार्किक Negative) {
  यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  यदि (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(असत्य, Negative);
    फर्काऊ;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tcSet(significandParts(), 0, part_गन्ती());
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
  APInt::tcSet(significandParts(), 0, part_गन्ती());
}

शून्य IEEEFloat::makeQuiet() {
  प्रतिज्ञा(isNaN());
  यदि (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tcSetBit(significandParts(), semantics->precision - 2);
}

पूर्णाङ्क ilogb(अचर IEEEFloat &Arg) {
  यदि (Arg.isNaN())
    फर्काऊ APFloat::IEK_NaN;
  यदि (Arg.isZero())
    फर्काऊ APFloat::IEK_Zero;
  यदि (Arg.isInfinity())
    फर्काऊ APFloat::IEK_Inf;
  यदि (!Arg.isDenormal())
    फर्काऊ Arg.exponent;

  IEEEFloat Normalized(Arg);
  पूर्णाङ्क SignificandBits = Arg.getSemantics().precision - 1;

  Normalized.exponent += SignificandBits;
  Normalized.normalize(APFloat::rmNearestTiesToEven, lfExactlyZero);
  फर्काऊ Normalized.exponent - SignificandBits;
}

IEEEFloat scalbn(IEEEFloat X, पूर्णाङ्क Exp, roundingMode RoundingMode) {
  स्वतः MaxExp = X.getSemantics().maxExponent;
  स्वतः MinExp = X.getSemantics().minExponent;

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
  यदि (X.isNaN())
    X.makeQuiet();
  फर्काऊ X;
}

IEEEFloat frexp(अचर IEEEFloat &Val, पूर्णाङ्क &Exp, roundingMode RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  यदि (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    फर्काऊ Quiet;
  }

  यदि (Exp == APFloat::IEK_Inf)
    फर्काऊ Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  फर्काऊ scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(अचर fltSemantics &S)
    : Semantics(&S),
      Floats(नूतन APFloat[2]{APFloat(APFloatBase::semIEEEdouble),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(अचर fltSemantics &S, uninitializedTag)
    : Semantics(&S), Floats(नूतन APFloat[2]{
                         APFloat(APFloatBase::semIEEEdouble, uninitialized),
                         APFloat(APFloatBase::semIEEEdouble, uninitialized)}) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(अचर fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(नूतन APFloat[2]{APFloat(APFloatBase::semIEEEdouble, I),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(अचर fltSemantics &S, अचर APInt &I)
    : Semantics(&S),
      Floats(नूतन APFloat[2]{
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[0])),
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[1]))}) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(अचर fltSemantics &S, APFloat &&First,
                             APFloat &&Second)
    : Semantics(&S),
      Floats(नूतन APFloat[2]{std::move(First), std::move(Second)}) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble);
  प्रतिज्ञा(&Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  प्रतिज्ञा(&Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(अचर DoubleAPFloat &RHS)
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

DoubleAPFloat &DoubleAPFloat::चालक=(अचर DoubleAPFloat &RHS) {
  यदि (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } अन्यथा यदि (एतत् != &RHS) {
    एतत्->~DoubleAPFloat();
    नूतन (एतत्) DoubleAPFloat(RHS);
  }
  फर्काऊ *एतत्;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
स्थिर std::pair<APFloat, APFloat> fastTwoSum(APFloat X, APFloat Y) {
  यदि (!X.isFinite())
    फर्काऊ {X, APFloat::getZero(X.getSemantics(), /*Negative=*/असत्य)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  फर्काऊ {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::opStatus DoubleAPFloat::addImpl(अचर APFloat &a, अचर APFloat &aa,
                                         अचर APFloat &c, अचर APFloat &cc,
                                         roundingMode RM) {
  पूर्णाङ्क Status = opOK;
  APFloat z = a;
  Status |= z.add(c, RM);
  यदि (!z.isFinite()) {
    यदि (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ असत्य);
      फर्काऊ (opStatus)Status;
    }
    Status = opOK;
    स्वतः AComparedToC = a.compare_Absolute_मान(c);
    z = cc;
    Status |= z.add(aa, RM);
    यदि (AComparedToC == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      Status |= z.add(c, RM);
      Status |= z.add(a, RM);
    } अन्यथा {
      // z = cc + aa + a + c;
      Status |= z.add(a, RM);
      Status |= z.add(c, RM);
    }
    यदि (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ असत्य);
      फर्काऊ (opStatus)Status;
    }
    Floats[0] = z;
    APFloat zz = aa;
    Status |= zz.add(cc, RM);
    यदि (AComparedToC == APFloat::cmpGreaterThan) {
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
    यदि (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ असत्य);
      फर्काऊ opOK;
    }
    Floats[0] = z;
    Status |= Floats[0].add(zz, RM);
    यदि (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ असत्य);
      फर्काऊ (opStatus)Status;
    }
    Floats[1] = std::move(z);
    Status |= Floats[1].subtract(Floats[0], RM);
    Status |= Floats[1].add(zz, RM);
  }
  फर्काऊ (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::addWithSpecial(अचर DoubleAPFloat &LHS,
                                                अचर DoubleAPFloat &RHS,
                                                DoubleAPFloat &Out,
                                                roundingMode RM) {
  यदि (LHS.getCategory() == fcNaN) {
    Out = LHS;
    फर्काऊ opOK;
  }
  यदि (RHS.getCategory() == fcNaN) {
    Out = RHS;
    फर्काऊ opOK;
  }
  यदि (LHS.getCategory() == fcZero) {
    Out = RHS;
    फर्काऊ opOK;
  }
  यदि (RHS.getCategory() == fcZero) {
    Out = LHS;
    फर्काऊ opOK;
  }
  यदि (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    Out.makeNaN(असत्य, Out.isNegative(), nullptr);
    फर्काऊ opInvalidOp;
  }
  यदि (LHS.getCategory() == fcInfinity) {
    Out = LHS;
    फर्काऊ opOK;
  }
  यदि (RHS.getCategory() == fcInfinity) {
    Out = RHS;
    फर्काऊ opOK;
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
  फर्काऊ Out.addImpl(A, AA, C, CC, RM);
}

APFloat::opStatus DoubleAPFloat::add(अचर DoubleAPFloat &RHS,
                                     roundingMode RM) {
  फर्काऊ addWithSpecial(*एतत्, RHS, *एतत्, RM);
}

APFloat::opStatus DoubleAPFloat::subtract(अचर DoubleAPFloat &RHS,
                                          roundingMode RM) {
  changeSign();
  स्वतः Ret = add(RHS, RM);
  changeSign();
  फर्काऊ Ret;
}

APFloat::opStatus DoubleAPFloat::multiply(अचर DoubleAPFloat &RHS,
                                          APFloat::roundingMode RM) {
  अचर स्वतः &LHS = *एतत्;
  स्वतः &Out = *एतत्;
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
  यदि (LHS.getCategory() == fcNaN) {
    Out = LHS;
    फर्काऊ opOK;
  }
  यदि (RHS.getCategory() == fcNaN) {
    Out = RHS;
    फर्काऊ opOK;
  }
  यदि ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    Out.makeNaN(असत्य, असत्य, nullptr);
    फर्काऊ opOK;
  }
  यदि (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    Out = LHS;
    फर्काऊ opOK;
  }
  यदि (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    Out = RHS;
    फर्काऊ opOK;
  }
  प्रतिज्ञा(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal &&
         "Special cases not handled exhaustively");

  पूर्णाङ्क Status = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  Status |= T.multiply(C, RM);
  यदि (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ असत्य);
    फर्काऊ (opStatus)Status;
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
  यदि (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ असत्य);
  } अन्यथा {
    // Floats[1] = (t - u) + tau
    Status |= T.subtract(U, RM);
    Status |= T.add(Tau, RM);
    Floats[1] = std::move(T);
  }
  फर्काऊ (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::divide(अचर DoubleAPFloat &RHS,
                                        APFloat::roundingMode RM) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  स्वतः Ret = Tmp.divide(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()), RM);
  *एतत् = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  फर्काऊ Ret;
}

APFloat::opStatus DoubleAPFloat::remainder(अचर DoubleAPFloat &RHS) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  स्वतः Ret = Tmp.remainder(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *एतत् = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  फर्काऊ Ret;
}

APFloat::opStatus DoubleAPFloat::mod(अचर DoubleAPFloat &RHS) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  स्वतः Ret = Tmp.mod(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *एतत् = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  फर्काऊ Ret;
}

APFloat::opStatus
DoubleAPFloat::fusedMultiplyAdd(अचर DoubleAPFloat &Multiplicand,
                                अचर DoubleAPFloat &Addend,
                                APFloat::roundingMode RM) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  स्वतः Ret = Tmp.fusedMultiplyAdd(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcastToAPInt()),
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, Addend.bitcastToAPInt()),
      RM);
  *एतत् = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  फर्काऊ Ret;
}

APFloat::opStatus DoubleAPFloat::roundToIntegral(APFloat::roundingMode RM) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  अचर APFloat &Hi = getFirst();
  अचर APFloat &Lo = getSecond();

  APFloat RoundedHi = Hi;
  अचर opStatus HiStatus = RoundedHi.roundToIntegral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  यदि (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/असत्य);
    फर्काऊ HiStatus;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  स्वतः RoundToNearestHelper = [](APFloat ToRound, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    अचर APFloat RoundingError = Rounded - ToRound;
    यदि (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        असल(RoundingError).is_Exactly_मान(0.5))
      Rounded.add(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rmNearestTiesToEven);
    फर्काऊ Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  यदि (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    यदि (RM == rmNearestTiesToAway || RM == rmNearestTiesToEven)
      RoundedHi = RoundToNearestHelper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/असत्य);
    फर्काऊ HiStatus;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  RoundingMode LoRoundingMode;
  यदि (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    LoRoundingMode = isNegative() ? rmTowardPositive : rmTowardNegative;
  अन्यथा
    LoRoundingMode = RM;

  APFloat RoundedLo = Lo;
  अचर opStatus LoStatus = RoundedLo.roundToIntegral(LoRoundingMode);
  यदि (LoRoundingMode == rmNearestTiesToAway)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = RoundToNearestHelper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fastTwoSum(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  फर्काऊ LoStatus;
}

शून्य DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmpResult
DoubleAPFloat::compare_Absolute_मान(अचर DoubleAPFloat &RHS) अचर {
  // Compare absolute values of the high parts.
  अचर cmpResult HiPartCmp = Floats[0].compare_Absolute_मान(RHS.Floats[0]);
  यदि (HiPartCmp != cmpEqual)
    फर्काऊ HiPartCmp;

  // Zero, regardless of sign, is equal.
  यदि (Floats[1].isZero() && RHS.Floats[1].isZero())
    फर्काऊ cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  अचर तार्किक ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  अचर तार्किक RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  यदि (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    फर्काऊ RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  यदि (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    फर्काऊ ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  यदि (ThisIsSubtractive != RHSIsSubtractive)
    फर्काऊ ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  अचर cmpResult LoPartCmp = Floats[1].compare_Absolute_मान(RHS.Floats[1]);

  यदि (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    यदि (LoPartCmp == cmpLessThan)
      फर्काऊ cmpGreaterThan;
    यदि (LoPartCmp == cmpGreaterThan)
      फर्काऊ cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  फर्काऊ LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() अचर {
  फर्काऊ Floats[0].getCategory();
}

तार्किक DoubleAPFloat::isNegative() अचर { फर्काऊ Floats[0].isNegative(); }

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
  यदि (Neg)
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
  यदि (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ असत्य);
}

शून्य DoubleAPFloat::makeNaN(तार्किक SNaN, तार्किक Neg, अचर APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ असत्य);
}

APFloat::cmpResult DoubleAPFloat::compare(अचर DoubleAPFloat &RHS) अचर {
  स्वतः Result = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  यदि (Result == APFloat::cmpEqual)
    फर्काऊ Floats[1].compare(RHS.Floats[1]);
  फर्काऊ Result;
}

तार्किक DoubleAPFloat::bitwiseIsEqual(अचर DoubleAPFloat &RHS) अचर {
  फर्काऊ Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_code hash_मान(अचर DoubleAPFloat &Arg) {
  यदि (Arg.Floats)
    फर्काऊ hash_combine(hash_मान(Arg.Floats[0]), hash_मान(Arg.Floats[1]));
  फर्काऊ hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcastToAPInt() अचर {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t Data[] = {
      Floats[0].bitcastToAPInt().getRawData()[0],
      Floats[1].bitcastToAPInt().getRawData()[0],
  };
  फर्काऊ APInt(128, Data);
}

Expected<APFloat::opStatus> DoubleAPFloat::convertFromString(StringRef S,
                                                             roundingMode RM) {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy);
  स्वतः Ret = Tmp.convertFromString(S, RM);
  *एतत् = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  फर्काऊ Ret;
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
  यदि (nextDown) {
    changeSign();
    APFloat::opStatus Result = next(/*nextDown=*/असत्य);
    changeSign();
    फर्काऊ Result;
  }
  विकल्प (getCategory()) {
  पक्ष fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    यदि (isNegative())
      makeLargest(सत्य);
    फर्काऊ opOK;

  पक्ष fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    यदि (getFirst().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(असत्य, isNegative(), nullptr);
      फर्काऊ opInvalidOp;
    }
    फर्काऊ opOK;

  पक्ष fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(असत्य);
    फर्काऊ opOK;

  पक्ष fcNormal:
    रोक;
  }

  अचर APFloat &HiOld = getFirst();
  अचर APFloat &LoOld = getSecond();

  APFloat NextLo = LoOld;
  NextLo.next(/*nextDown=*/असत्य);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  स्वतः InLattice = [](अचर APFloat &Hi, अचर APFloat &Lo) {
    फर्काऊ Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  यदि (InLattice(HiOld, NextLo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(NextLo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat Largest{*Semantics, uninitialized};
    Largest.makeLargest(/*Neg=*/असत्य);
    यदि (compare(Largest) == cmpGreaterThan)
      makeInf(/*Neg=*/असत्य);

    फर्काऊ opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat NextHi = HiOld;
  NextHi.next(/*nextDown=*/असत्य);

  // nextUp(getLargest()) == INFINITY
  यदि (NextHi.isInfinity()) {
    makeInf(/*Neg=*/असत्य);
    फर्काऊ opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  यदि (NextHi.isZero()) {
    makeZero(/*Neg=*/सत्य);
    फर्काऊ opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  NextLo = neg(scalbn(harrisonUlp(NextHi), -1, rmTowardZero));
  यदि (!InLattice(NextHi, NextLo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    NextLo.next(/*nextDown=*/असत्य);

  Floats[0] = std::move(NextHi);
  Floats[1] = std::move(NextLo);

  फर्काऊ opOK;
}

APFloat::opStatus DoubleAPFloat::convertToSignExtendedInteger(
    MutableArrayRef<integerPart> आगत, अचिन्हित पूर्णाङ्क Width, तार्किक IsSigned,
    roundingMode RM, तार्किक *IsExact) अचर {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  यदि (!getFirst().isFiniteNonZero() || getSecond().isZero())
    फर्काऊ getFirst().convertToInteger(आगत, Width, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = असत्य;
  DoubleAPFloat Integral = *एतत्;
  अचर opStatus RoundStatus = Integral.roundToIntegral(RM);
  यदि (RoundStatus == opInvalidOp)
    फर्काऊ opInvalidOp;
  अचर APFloat &IntegralHi = Integral.getFirst();
  अचर APFloat &IntegralLo = Integral.getSecond();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  तार्किक HiIsExact;
  यदि (IntegralHi.isZero() || IntegralLo.isZero()) {
    अचर opStatus HiStatus =
        IntegralHi.convertToInteger(आगत, Width, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    फर्काऊ HiStatus == opOK ? opInexact : HiStatus;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  यदि (!IsSigned && IntegralHi.isNegative())
    फर्काऊ opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  तार्किक LoIsExact;
  अचर पूर्णाङ्क HiExactLog2 = IntegralHi.getExactLog2Abs();
  अचर अचिन्हित PositiveOverflowWidth = IsSigned ? Width - 1 : Width;
  यदि (HiExactLog2 >= 0 &&
      static_cast<अचिन्हित>(HiExactLog2) == PositiveOverflowWidth) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    यदि (IntegralHi.isNegative() == IntegralLo.isNegative())
      फर्काऊ opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    अचर opStatus LoStatus = IntegralLo.convertToInteger(
        आगत, Width, /*IsSigned=*/सत्य, RM, &LoIsExact);
    यदि (LoStatus == opInvalidOp)
      फर्काऊ opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    यदि (IsSigned && !IntegralHi.isNegative())
      APInt::tcClearBit(आगत.data(), PositiveOverflowWidth);
    *IsExact = RoundStatus == opOK;
    फर्काऊ RoundStatus;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  अचर opStatus HiStatus = IntegralHi.convertToInteger(
      आगत, Width, IsSigned, rmTowardZero, &HiIsExact);
  यदि (HiStatus == opInvalidOp)
    फर्काऊ HiStatus;

  // Convert Lo into a temporary integer of the same width.
  APSInt LoResult{Width, /*isUnsigned=*/!IsSigned};
  अचर opStatus LoStatus =
      IntegralLo.convertToInteger(LoResult, rmTowardZero, &LoIsExact);
  यदि (LoStatus == opInvalidOp)
    फर्काऊ LoStatus;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tcAdd(आगत.data(), LoResult.getRawData(), /*carry=*/0, आगत.size());

  *IsExact = RoundStatus == opOK;
  फर्काऊ RoundStatus;
}

APFloat::opStatus
DoubleAPFloat::convertToInteger(MutableArrayRef<integerPart> आगत,
                                अचिन्हित पूर्णाङ्क Width, तार्किक IsSigned,
                                roundingMode RM, तार्किक *IsExact) अचर {
  opStatus FS =
      convertToSignExtendedInteger(आगत, Width, IsSigned, RM, IsExact);

  यदि (FS == opInvalidOp) {
    अचर अचिन्हित Dst_Parts_गन्ती = part_गन्ती_For_Bits(Width);
    प्रतिज्ञा(Dst_Parts_गन्ती <= आगत.size() && "Integer too big");

    अचिन्हित Bits;
    यदि (getCategory() == fcNaN)
      Bits = 0;
    अन्यथा यदि (isNegative())
      Bits = IsSigned;
    अन्यथा
      Bits = Width - IsSigned;

    tcSetLeastSignificantBits(आगत.data(), Dst_Parts_गन्ती, Bits);
    यदि (isNegative() && IsSigned)
      APInt::tc_Shift_बायाँ(आगत.data(), Dst_Parts_गन्ती, Width - 1);
  }

  फर्काऊ FS;
}

APFloat::opStatus DoubleAPFloat::handleOverflow(roundingMode RM) {
  विकल्प (RM) {
  पक्ष APFloat::rmTowardZero:
    makeLargest(/*Neg=*/isNegative());
    रोक;
  पक्ष APFloat::rmTowardNegative:
    यदि (isNegative())
      makeInf(/*Neg=*/सत्य);
    अन्यथा
      makeLargest(/*Neg=*/असत्य);
    रोक;
  पक्ष APFloat::rmTowardPositive:
    यदि (isNegative())
      makeLargest(/*Neg=*/सत्य);
    अन्यथा
      makeInf(/*Neg=*/असत्य);
    रोक;
  पक्ष APFloat::rmNearestTiesToAway:
  पक्ष APFloat::rmNearestTiesToEven:
    makeInf(/*Neg=*/isNegative());
    रोक;
  स्वभाव:
    llvm_unreachable("Invalid rounding mode found");
  }
  opStatus S = opInexact;
  यदि (!getFirst().isFinite())
    S = static_cast<opStatus>(S | opOverflow);
  फर्काऊ S;
}

APFloat::opStatus DoubleAPFloat::convertFromUnsignedParts(
    अचर integerPart *Src, अचिन्हित पूर्णाङ्क Src_गन्ती, roundingMode RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  अचर अचिन्हित SrcMSB = APInt::tcMSB(Src, Src_गन्ती);
  यदि (SrcMSB == अच_पूर्णांक_उच्च) {
    // The source integer is 0.
    makeZero(/*Neg=*/असत्य);
    फर्काऊ opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  अचर अचिन्हित SrcBitWidth = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/SrcBitWidth, ArrayRef(Src, Src_गन्ती)},
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
  यदि (!Hi.isFinite())
    फर्काऊ handleOverflow(RM);

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
  अचर APInt Error = SrcInt.zext(HiAsInt.getBitWidth()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  roundingMode LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  यदि (Error.isNegative()) {
    यदि (RM == rmNearestTiesToAway) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      अचर अचिन्हित ErrorActiveBits = Error.getSignificantBits() - 1;
      अचर अचिन्हित LoPrecision = getSecond().getSemantics().precision;
      यदि (ErrorActiveBits > LoPrecision) {
        अचर अचिन्हित RoundingBoundary = ErrorActiveBits - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        यदि (Error.गन्ती_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rmTowardPositive;
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
  opStatus Status = Lo.convertFromAPInt(Error, /*IsSigned=*/सत्य, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fastTwoSum(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  यदि (!getFirst().isFinite())
    फर्काऊ handleOverflow(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  यदि (getFirst().isFiniteNonZero() && Floats[0].isLargest()) {
    DoubleAPFloat Largest{*Semantics};
    Largest.makeLargest(/*Neg=*/असत्य);
    यदि (compare(Largest) == APFloat::cmpGreaterThan)
      फर्काऊ handleOverflow(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  फर्काऊ Status;
}

APFloat::opStatus DoubleAPFloat::convertFromAPInt(अचर APInt &आगत,
                                                  तार्किक IsSigned,
                                                  roundingMode RM) {
  अचर तार्किक Negate_आगत = IsSigned && आगत.isNegative();
  APInt API = आगत;
  यदि (Negate_आगत)
    API.negate();

  अचर APFloat::opStatus Status =
      convertFromUnsignedParts(API.getRawData(), API.get_अङ्क_शब्दहरू(), RM);
  यदि (Negate_आगत)
    changeSign();
  फर्काऊ Status;
}

अचिन्हित पूर्णाङ्क DoubleAPFloat::convertToHexString(वर्ण *DST,
                                               अचिन्हित पूर्णाङ्क HexDigits,
                                               तार्किक UpperCase,
                                               roundingMode RM) अचर {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  फर्काऊ APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .convertToHexString(DST, HexDigits, UpperCase, RM);
}

तार्किक DoubleAPFloat::isDenormal() अचर {
  फर्काऊ getCategory() == fcNormal &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

तार्किक DoubleAPFloat::isSmallest() अचर {
  यदि (getCategory() != fcNormal)
    फर्काऊ असत्य;
  DoubleAPFloat Tmp(*एतत्);
  Tmp.makeSmallest(एतत्->isNegative());
  फर्काऊ Tmp.compare(*एतत्) == cmpEqual;
}

तार्किक DoubleAPFloat::isSmallestNormalized() अचर {
  यदि (getCategory() != fcNormal)
    फर्काऊ असत्य;

  DoubleAPFloat Tmp(*एतत्);
  Tmp.makeSmallestNormalized(एतत्->isNegative());
  फर्काऊ Tmp.compare(*एतत्) == cmpEqual;
}

तार्किक DoubleAPFloat::isLargest() अचर {
  यदि (getCategory() != fcNormal)
    फर्काऊ असत्य;
  DoubleAPFloat Tmp(*एतत्);
  Tmp.makeLargest(एतत्->isNegative());
  फर्काऊ Tmp.compare(*एतत्) == cmpEqual;
}

तार्किक DoubleAPFloat::isInteger() अचर {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  फर्काऊ Floats[0].isInteger() && Floats[1].isInteger();
}

शून्य DoubleAPFloat::toString(SmallVectorImpl<वर्ण> &Str,
                             अचिन्हित FormatPrecision,
                             अचिन्हित FormatMaxPadding,
                             तार्किक TruncateZero) अचर {
  प्रतिज्ञा(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .toString(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

पूर्णाङ्क DoubleAPFloat::getExactLog2Abs() अचर {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  यदि (getSecond().isNonZero())
    फर्काऊ पूर्णांक_न्यून;
  फर्काऊ getFirst().getExactLog2Abs();
}

पूर्णाङ्क ilogb(अचर DoubleAPFloat &Arg) {
  अचर APFloat &Hi = Arg.getFirst();
  अचर APFloat &Lo = Arg.getSecond();
  पूर्णाङ्क IlogbResult = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  यदि (Arg.getCategory() != fcNormal)
    फर्काऊ IlogbResult;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  यदि (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    फर्काऊ IlogbResult;
  यदि (Hi.getExactLog2Abs() == पूर्णांक_न्यून)
    फर्काऊ IlogbResult;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  फर्काऊ IlogbResult - 1;
}

DoubleAPFloat scalbn(अचर DoubleAPFloat &Arg, पूर्णाङ्क Exp,
                     APFloat::roundingMode RM) {
  प्रतिज्ञा(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");
  फर्काऊ DoubleAPFloat(APFloatBase::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(अचर DoubleAPFloat &Arg, पूर्णाङ्क &Exp,
                    APFloat::roundingMode RM) {
  प्रतिज्ञा(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  यदि (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.getFirst() = Quiet.getFirst().makeQuiet();
    फर्काऊ Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  यदि (Exp == APFloat::IEK_Inf)
    फर्काऊ Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  यदि (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    फर्काऊ Arg;
  }

  अचर APFloat &Hi = Arg.getFirst();
  अचर APFloat &Lo = Arg.getSecond();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  अचर तार्किक SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat Second = Lo;
  यदि (Arg.getCategory() == APFloat::fcNormal && Lo.isFiniteNonZero()) {
    roundingMode LoRoundingMode;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    यदि (RM == rmTowardZero)
      LoRoundingMode = Arg.isNegative() ? rmTowardPositive : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    अन्यथा यदि (RM == rmNearestTiesToAway && SignsDisagree && Exp > 0)
      LoRoundingMode = rmNearestTiesToEven;
    अन्यथा
      LoRoundingMode = RM;
    Second = scalbn(Lo, -Exp, LoRoundingMode);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    यदि (RM == rmNearestTiesToAway && LoRoundingMode == rmNearestTiesToEven) {
      // Re-scale the result back to check if rounding occurred.
      अचर APFloat RecomposedLo = scalbn(Second, Exp, rmNearestTiesToEven);
      यदि (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        अचर APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        अचर APFloat UlpOfSecond = harrisonUlp(Second);
        अचर APFloat ScaledUlpOfSecond =
            scalbn(UlpOfSecond, Exp - 1, rmNearestTiesToEven);
        अचर तार्किक IsMidpoint = असल(RoundingError) == ScaledUlpOfSecond;
        अचर तार्किक RoundedLoAway =
            Second.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        यदि (IsMidpoint && RoundedLoAway)
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
    यदि (Second.isZero() && SignsDisagree && Hi.getExactLog2Abs() != पूर्णांक_न्यून)
      ++Exp;
  }

  APFloat First = scalbn(Hi, -Exp, RM);
  फर्काऊ DoubleAPFloat(APFloatBase::PPCDoubleDouble(), std::move(First),
                       std::move(Second));
}

APInt DoubleAPFloat::getNaNPayload() अचर { फर्काऊ Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, अचर fltSemantics &Semantics) {
  यदि (usesLayout<IEEEFloat>(Semantics)) {
    नूतन (&IEEE) IEEEFloat(std::move(F));
    फर्काऊ;
  }
  यदि (usesLayout<DoubleAPFloat>(Semantics)) {
    अचर fltSemantics& S = F.getSemantics();
    नूतन (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloatBase::IEEEdouble()));
    फर्काऊ;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::opStatus> APFloat::convertFromString(StringRef Str,
                                                       roundingMode RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_code hash_मान(अचर APFloat &Arg) {
  यदि (APFloat::usesLayout<detail::IEEEFloat>(Arg.getSemantics()))
    फर्काऊ hash_मान(Arg.U.IEEE);
  यदि (APFloat::usesLayout<detail::DoubleAPFloat>(Arg.getSemantics()))
    फर्काऊ hash_मान(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(अचर fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  स्वतः StatusOrErr = convertFromString(S, rmNearestTiesToEven);
  प्रतिज्ञा(StatusOrErr && "Invalid floating point representation");
  consumeError(StatusOrErr.takeError());
}

FPClassTest APFloat::classify() अचर {
  यदि (isZero())
    फर्काऊ isNegative() ? fcNegZero : fcPosZero;
  यदि (isNormal())
    फर्काऊ isNegative() ? fcNegNormal : fcPosNormal;
  यदि (isDenormal())
    फर्काऊ isNegative() ? fcNegSubnormal : fcPosSubnormal;
  यदि (isInfinity())
    फर्काऊ isNegative() ? fcNegInf : fcPosInf;
  प्रतिज्ञा(isNaN() && "Other class of FP constant");
  फर्काऊ isSignaling() ? fcSNan : fcQNan;
}

तार्किक APFloat::getExactInverse(APFloat *Inv) अचर {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  यदि (!isFiniteNonZero())
    फर्काऊ असत्य;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  यदि (isDenormal())
    फर्काऊ असत्य;

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
  अचर पूर्णाङ्क Exp = getExactLog2Abs();
  यदि (Exp == पूर्णांक_न्यून)
    फर्काऊ असत्य;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  यदि (Reciprocal.getExactLog2Abs() != -Exp)
    फर्काऊ असत्य;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  यदि (Reciprocal.isDenormal())
    फर्काऊ असत्य;

  प्रतिज्ञा(Reciprocal.isFiniteNonZero());

  यदि (Inv)
    *Inv = std::move(Reciprocal);

  फर्काऊ सत्य;
}

APFloat::opStatus APFloat::convert(अचर fltSemantics &ToSemantics,
                                   roundingMode RM, तार्किक *losesInfo) {
  यदि (&getSemantics() == &ToSemantics) {
    *losesInfo = असत्य;
    फर्काऊ opOK;
  }
  यदि (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics))
    फर्काऊ U.IEEE.convert(ToSemantics, RM, losesInfo);
  यदि (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<DoubleAPFloat>(ToSemantics)) {
    प्रतिज्ञा(&ToSemantics == &APFloatBase::semPPCDoubleDouble);
    स्वतः Ret =
        U.IEEE.convert(APFloatBase::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *एतत् = APFloat(ToSemantics, U.IEEE.bitcastToAPInt());
    फर्काऊ Ret;
  }
  यदि (usesLayout<DoubleAPFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics)) {
    स्वतः Ret = getIEEE().convert(ToSemantics, RM, losesInfo);
    *एतत् = APFloat(std::move(getIEEE()), ToSemantics);
    फर्काऊ Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_मान(अचर fltSemantics &Semantics) {
  फर्काऊ APFloat(Semantics, APInt::getAllOnes(Semantics.sizeInBits));
}

शून्य APFloat::print(raw_ostream &OS) अचर {
  SmallVector<वर्ण, 16> Buffer;
  toString(Buffer);
  OS << Buffer;
}

#यदि !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य APFloat::dump() अचर {
  print(dbgs());
  dbgs() << '\n';
}
#endif

शून्य APFloat::Profile(Folding_Set_गाँठो_ID &NID) अचर {
  NID.Add(bitcastToAPInt());
}

APFloat::opStatus APFloat::convertToInteger(APSInt &result,
                                            roundingMode rounding_mode,
                                            तार्किक *isExact) अचर {
  अचिन्हित bitWidth = result.getBitWidth();
  SmallVector<uint64_t, 4> parts(result.get_अङ्क_शब्दहरू());
  opStatus status = convertToInteger(parts, bitWidth, result.isSigned(),
                                     rounding_mode, isExact);
  // Keeps the original signed-ness.
  result = APInt(bitWidth, parts);
  फर्काऊ status;
}

द्विगुण APFloat::convertToDouble() अचर {
  यदि (&getSemantics() == &APFloatBase::semIEEEdouble)
    फर्काऊ getIEEE().convertToDouble();
  प्रतिज्ञा(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat Temp = *एतत्;
  तार्किक LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &LosesInfo);
  प्रतिज्ञा(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  फर्काऊ Temp.getIEEE().convertToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 APFloat::convertToQuad() अचर {
  यदि (&getSemantics() == &APFloatBase::semIEEEquad)
    फर्काऊ getIEEE().convertToQuad();
  प्रतिज्ञा(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat Temp = *एतत्;
  तार्किक LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEquad, rmNearestTiesToEven, &LosesInfo);
  प्रतिज्ञा(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  फर्काऊ Temp.getIEEE().convertToQuad();
}
#endif

भिन्न APFloat::convertToFloat() अचर {
  यदि (&getSemantics() == &APFloatBase::semIEEEsingle)
    फर्काऊ getIEEE().convertToFloat();
  प्रतिज्ञा(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat Temp = *एतत्;
  तार्किक LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEsingle, rmNearestTiesToEven, &LosesInfo);
  प्रतिज्ञा(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  फर्काऊ Temp.getIEEE().convertToFloat();
}

अचिन्हित APFloatBase::getArbitraryFPFormatSizeInBits(StringRef Format) {
  फर्काऊ StringSwitch<अचिन्हित>(Format)
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
  फर्काऊ getArbitraryFPFormatSizeInBits(Format) != 0;
}

अचर fltSemantics *APFloatBase::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  फर्काऊ StringSwitch<अचर fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(nullptr);
}

APFloat::Storage::~Storage() {
  यदि (usesLayout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    फर्काऊ;
  }
  यदि (usesLayout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    फर्काऊ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(अचर APFloat::Storage &RHS) {
  यदि (usesLayout<IEEEFloat>(*RHS.semantics)) {
    नूतन (एतत्) IEEEFloat(RHS.IEEE);
    फर्काऊ;
  }
  यदि (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    नूतन (एतत्) DoubleAPFloat(RHS.Double);
    फर्काऊ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  यदि (usesLayout<IEEEFloat>(*RHS.semantics)) {
    नूतन (एतत्) IEEEFloat(std::move(RHS.IEEE));
    फर्काऊ;
  }
  यदि (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    नूतन (एतत्) DoubleAPFloat(std::move(RHS.Double));
    फर्काऊ;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::चालक=(अचर APFloat::Storage &RHS) {
  यदि (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } अन्यथा यदि (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } अन्यथा यदि (एतत् != &RHS) {
    एतत्->~Storage();
    नूतन (एतत्) Storage(RHS);
  }
  फर्काऊ *एतत्;
}

APFloat::Storage &APFloat::Storage::चालक=(APFloat::Storage &&RHS) {
  यदि (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } अन्यथा यदि (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } अन्यथा यदि (एतत् != &RHS) {
    एतत्->~Storage();
    नूतन (एतत्) Storage(std::move(RHS));
  }
  फर्काऊ *एतत्;
}

नामाकाश {

APFloat::opStatus getOpStatusFromLibc(पूर्णाङ्क libc_exceptions) {
  APFloat::opStatus status = APFloat::opOK;
  यदि (libc_exceptions & FE_INVALID)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInvalidOp);
  यदि (libc_exceptions & FE_DIVBYZERO)
    status = static_cast<APFloat::opStatus>(status | APFloat::opDivByZero);
  यदि (libc_exceptions & FE_OVERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opOverflow);
  यदि (libc_exceptions & FE_UNDERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opUnderflow);
  यदि (libc_exceptions & FE_INEXACT)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInexact);
  फर्काऊ status;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> exp(अचर APFloat &x, RoundingMode rounding_mode,
                           APFloat::opStatus *status) {

  यदि (rounding_mode == APFloatBase::rmNearestTiesToEven) {
    यदि (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEsingle) {
      भिन्न x_val = x.convertToFloat();
      पूर्णाङ्क exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      यदि (status) {
        *status = getOpStatusFromLibc(exc);
        यदि (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      भिन्न result = LIBC_NAMESPACE::shared::expf(x_val);
      फर्काऊ APFloat(result);
    }
    यदि (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEdouble) {
      द्विगुण x_val = x.convertToDouble();
      पूर्णाङ्क exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      यदि (status) {
        *status = getOpStatusFromLibc(exc);
        यदि (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      द्विगुण result = LIBC_NAMESPACE::shared::exp(x_val);
      फर्काऊ APFloat(result);
    }
  }
  फर्काऊ std::nullopt;
}

} // namespace llvm

#undef APFLOAT_DISPATCH_ON_SEMANTICS
