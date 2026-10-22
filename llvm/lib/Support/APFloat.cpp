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

#समावेश "llvm/ADT/APFloat.h"
#समावेश "llvm/ADT/APSInt.h"
#समावेश "llvm/ADT/ArrayRef.h"
#समावेश "llvm/ADT/FloatingPointMode.h"
#समावेश "llvm/ADT/FoldingSet.h"
#समावेश "llvm/ADT/Hashing.h"
#समावेश "llvm/ADT/STLExtras.h"
#समावेश "llvm/ADT/SmallVector.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/ADT/StringSwitch.h"
#समावेश "llvm/Config/llvm-config.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/Error.h"
#समावेश "llvm/Support/MathExtras.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <cstring>
#समावेश <iterator>
#समावेश <limits.h>

/// Shared headers from LLVM libc
/// Make sure to add ${LLVM_SOURCE_DIR}/../libc to include directories.
///
/// Notes: So far it looks like APFloat does not check errnos or floating-point
/// exceptions after calling the math functions, so we will configure LLVM libc
/// math functions to skip setting errnos and floating-point exceptions
/// explicitly.  We also put them in a separate namespace so that the symbols
/// do not clash with other libc math builds just in case.
#घोषणा LIBC_NAMESPACE __llvm_libc_apfloat
#घोषणा LIBC_MATH (LIBC_MATH_NO_ERRNO | LIBC_MATH_NO_EXCEPT)

#समावेश "shared/math.h"
#समावेश "shared/math_check_exceptions.h"

#घोषणा APFLOAT_DISPATCH_ON_SEMANTICS(METHOD_CALL)                             \
  करो {                                                                         \
    अगर (usesLayout<IEEEFloat>(getSemantics()))                                 \
      वापस U.IEEE.METHOD_CALL;                                               \
    अगर (usesLayout<DoubleAPFloat>(getSemantics()))                             \
      वापस U.Double.METHOD_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } जबतक (false)

using namespace llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#घोषणा Pack_Categories_Into_कुंजी(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloatBase::integerPartWidth % 4 == 0, "Part width must be divisible by 4!");

namespace llvm {

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
    false,
    false,
    false,
    false};

constexpr fltSemantics APFloatBase::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    true,
    false,
    false};

constexpr fltSemantics APFloatBase::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semBogus = {0, 0, 0, 0};

स्थिर llvm::fltSemantics &APFloatBase::EnumToSemantics(Semantics S) {
  चयन (S) {
  हाल S_IEEEhalf:
    वापस IEEEhalf();
  हाल S_BFloat:
    वापस BFloat();
  हाल S_IEEEsingle:
    वापस IEEEsingle();
  हाल S_IEEEdouble:
    वापस IEEEdouble();
  हाल S_IEEEquad:
    वापस IEEEquad();
  हाल S_x87DoubleExtended:
    वापस x87DoubleExtended();
  हाल S_PPCDoubleDouble:
    वापस PPCDoubleDouble();
  हाल S_PPCDoubleDoubleLegacy:
    वापस PPCDoubleDoubleLegacy();
  हाल S_Float8E5M2:
    वापस Float8E5M2();
  हाल S_Float8E5M2FNUZ:
    वापस Float8E5M2FNUZ();
  हाल S_Float8E4M3:
    वापस Float8E4M3();
  हाल S_Float8E4M3FN:
    वापस Float8E4M3FN();
  हाल S_Float8E4M3FNUZ:
    वापस Float8E4M3FNUZ();
  हाल S_Float8E4M3B11FNUZ:
    वापस Float8E4M3B11FNUZ();
  हाल S_Float8E3M4:
    वापस Float8E3M4();
  हाल S_FloatTF32:
    वापस FloatTF32();
  हाल S_Float8E8M0FNU:
    वापस Float8E8M0FNU();
  हाल S_Float8E5M3FNU:
    वापस Float8E5M3FNU();
  हाल S_Float6E3M2FN:
    वापस Float6E3M2FN();
  हाल S_Float6E2M3FN:
    वापस Float6E2M3FN();
  हाल S_Float4E2M1FN:
    वापस Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloatBase::Semantics
APFloatBase::SemanticsToEnum(स्थिर llvm::fltSemantics &Sem) {
  अगर (&Sem == &llvm::APFloat::IEEEhalf())
    वापस S_IEEEhalf;
  अन्यथा अगर (&Sem == &llvm::APFloat::BFloat())
    वापस S_BFloat;
  अन्यथा अगर (&Sem == &llvm::APFloat::IEEEsingle())
    वापस S_IEEEsingle;
  अन्यथा अगर (&Sem == &llvm::APFloat::IEEEdouble())
    वापस S_IEEEdouble;
  अन्यथा अगर (&Sem == &llvm::APFloat::IEEEquad())
    वापस S_IEEEquad;
  अन्यथा अगर (&Sem == &llvm::APFloat::x87DoubleExtended())
    वापस S_x87DoubleExtended;
  अन्यथा अगर (&Sem == &llvm::APFloat::PPCDoubleDouble())
    वापस S_PPCDoubleDouble;
  अन्यथा अगर (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    वापस S_PPCDoubleDoubleLegacy;
  अन्यथा अगर (&Sem == &llvm::APFloat::Float8E5M2())
    वापस S_Float8E5M2;
  अन्यथा अगर (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    वापस S_Float8E5M2FNUZ;
  अन्यथा अगर (&Sem == &llvm::APFloat::Float8E4M3())
    वापस S_Float8E4M3;
  अन्यथा अगर (&Sem == &llvm::APFloat::Float8E4M3FN())
    वापस S_Float8E4M3FN;
  अन्यथा अगर (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    वापस S_Float8E4M3FNUZ;
  अन्यथा अगर (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    वापस S_Float8E4M3B11FNUZ;
  अन्यथा अगर (&Sem == &llvm::APFloat::Float8E3M4())
    वापस S_Float8E3M4;
  अन्यथा अगर (&Sem == &llvm::APFloat::FloatTF32())
    वापस S_FloatTF32;
  अन्यथा अगर (&Sem == &llvm::APFloat::Float8E8M0FNU())
    वापस S_Float8E8M0FNU;
  अन्यथा अगर (&Sem == &llvm::APFloat::Float8E5M3FNU())
    वापस S_Float8E5M3FNU;
  अन्यथा अगर (&Sem == &llvm::APFloat::Float6E3M2FN())
    वापस S_Float6E3M2FN;
  अन्यथा अगर (&Sem == &llvm::APFloat::Float6E2M3FN())
    वापस S_Float6E2M3FN;
  अन्यथा अगर (&Sem == &llvm::APFloat::Float4E2M1FN())
    वापस S_Float4E2M1FN;
  अन्यथा
    llvm_unreachable("Unknown floating semantics");
}

bool APFloatBase::isRepresentableBy(स्थिर fltSemantics &A,
                                    स्थिर fltSemantics &B) {
  वापस A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

bool APFloatBase::isLosslesslyConvertibleTo(स्थिर fltSemantics &From,
                                            स्थिर fltSemantics &InitialTo,
                                            bool IgnoreNaNs) {
  अगर (&From == &InitialTo)
    वापस true;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  अगर (&From == &semPPCDoubleDouble)
    वापस false;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  स्थिर fltSemantics &To =
      &InitialTo == &semPPCDoubleDouble ? semIEEEdouble : InitialTo;

  अगर (!isRepresentableBy(From, To))
    वापस false;

  अगर ((From.hasZero && !To.hasZero) ||
      (From.hasSignedRepr && !To.hasSignedRepr))
    वापस false;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  bool FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  bool ToHasSignedZero = To.hasZero && To.hasSignedRepr &&
                         To.nanEncoding != fltNanEncoding::NegativeZero;
  अगर (FromHasSignedZero && !ToHasSignedZero)
    वापस false;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat SmallestFrom = APFloat::getSmallest(From);
  bool LosesInfo = false;
  (व्योम)SmallestFrom.convert(To, APFloat::rmNearestTiesToEven, &LosesInfo);
  अगर (LosesInfo)
    वापस false;

  अगर (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    वापस true;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  अगर (IgnoreNaNs) {
    अगर (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      वापस To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    वापस To.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  अगर (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    वापस false;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  अगर (To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    वापस true;
  वापस To.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
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
अटल constexpr अचिन्हित अंक_Pow5_Parts_On_ढेर = 16;

अटल अचिन्हित पूर्णांक powerOfFivePartsForExponent(अचिन्हित पूर्णांक power) {
  निश्चित(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  वापस 2 + ((power * 815) / (351 * APFloatBase::integerPartWidth));
}

अचिन्हित पूर्णांक APFloatBase::semanticsPrecision(स्थिर fltSemantics &semantics) {
  वापस semantics.precision;
}
APFloatBase::ExponentType
APFloatBase::semanticsMaxExponent(स्थिर fltSemantics &semantics) {
  वापस semantics.maxExponent;
}
APFloatBase::ExponentType
APFloatBase::semanticsMinExponent(स्थिर fltSemantics &semantics) {
  वापस semantics.minExponent;
}
अचिन्हित पूर्णांक APFloatBase::semantics_आकार_In_Bits(स्थिर fltSemantics &semantics) {
  वापस semantics.आकार_In_Bits;
}
अचिन्हित पूर्णांक APFloatBase::semantics_Int_आकार_In_Bits(स्थिर fltSemantics &semantics,
                                                 bool isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  अचिन्हित पूर्णांक MinBitWidth = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  अगर (isSigned)
    ++MinBitWidth;
  वापस MinBitWidth;
}

bool APFloatBase::semanticsHasZero(स्थिर fltSemantics &semantics) {
  वापस semantics.hasZero;
}

bool APFloatBase::semanticsHasSignedRepr(स्थिर fltSemantics &semantics) {
  वापस semantics.hasSignedRepr;
}

bool APFloatBase::semanticsHasInf(स्थिर fltSemantics &semantics) {
  वापस semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

bool APFloatBase::semanticsHasNaN(स्थिर fltSemantics &semantics) {
  वापस semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

bool APFloatBase::isIEEELikeFP(स्थिर fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  वापस SemanticsToEnum(semantics) <= S_IEEEquad;
}

bool APFloatBase::hasSignBitInMSB(स्थिर fltSemantics &semantics) {
  वापस semantics.hasSignBitInMSB;
}

bool APFloatBase::isRepresentableAsNormalIn(स्थिर fltSemantics &Src,
                                            स्थिर fltSemantics &Dst) {
  // Exponent range must be larger.
  अगर (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    वापस false;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  वापस Dst.precision >= Src.precision;
}

अचिन्हित APFloatBase::get_आकार_In_Bits(स्थिर fltSemantics &Sem) {
  वापस Sem.आकार_In_Bits;
}

अटल constexpr APFloatBase::ExponentType
exponentZero(स्थिर fltSemantics &semantics) {
  वापस semantics.minExponent - 1;
}

अटल constexpr APFloatBase::ExponentType
exponentInf(स्थिर fltSemantics &semantics) {
  वापस semantics.maxExponent + 1;
}

अटल constexpr APFloatBase::ExponentType
exponentNaN(स्थिर fltSemantics &semantics) {
  अगर (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    अगर (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      वापस exponentZero(semantics);
    अगर (semantics.hasSignedRepr || semantics.precision > 1)
      वापस semantics.maxExponent;
  }
  वापस semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

अटल अंतरभूत Error createError(स्थिर Twine &Err) {
  वापस make_error<StringError>(Err, inconvertibleErrorCode());
}

अटल constexpr अंतरभूत अचिन्हित पूर्णांक part_गिनती_For_Bits(अचिन्हित पूर्णांक bits) {
  वापस std::max(1u, (bits + APFloatBase::integerPartWidth - 1) /
                          APFloatBase::integerPartWidth);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
अटल अंतरभूत अचिन्हित पूर्णांक
dec_Digit_मूल्य(अचिन्हित पूर्णांक c)
{
  वापस c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
अटल Expected<पूर्णांक> readExponent(StringRef::iterator begin,
                                  StringRef::iterator end) {
  स्थिर अचिन्हित पूर्णांक overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  अगर (p == end || ((*p == '-' || *p == '+') && (p + 1) == end))
    वापस 0;

  bool isNegative = *p == '-';
  अगर (*p == '-' || *p == '+') {
    p++;
    अगर (p == end)
      वापस createError("Exponent has no digits");
  }

  अचिन्हित absExponent = dec_Digit_मूल्य(*p++);
  अगर (absExponent >= 10U)
    वापस createError("Invalid character in exponent");

  क्रम (; p != end; ++p) {
    अचिन्हित मूल्य_2 = dec_Digit_मूल्य(*p);
    अगर (मूल्य_2 >= 10U)
      वापस createError("Invalid character in exponent");

    absExponent = absExponent * 10U + मूल्य_2;
    अगर (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      अवरोध;
    }
  }

  अगर (isNegative)
    वापस -(पूर्णांक) absExponent;
  अन्यथा
    वापस (पूर्णांक) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
अटल Expected<पूर्णांक> योग_Exponent(StringRef::iterator p,
                                   StringRef::iterator end,
                                   पूर्णांक exponentAdjustment) {
  पूर्णांक exponent = 0;

  अगर (p == end)
    वापस createError("Exponent has no digits");

  bool negative = *p == '-';
  अगर (*p == '-' || *p == '+') {
    p++;
    अगर (p == end)
      वापस createError("Exponent has no digits");
  }

  पूर्णांक unsignedExponent = 0;
  bool overflow = false;
  क्रम (; p != end; ++p) {
    अचिन्हित पूर्णांक मूल्य_2;

    मूल्य_2 = dec_Digit_मूल्य(*p);
    अगर (मूल्य_2 >= 10U)
      वापस createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + मूल्य_2;
    अगर (unsignedExponent > 32767) {
      overflow = true;
      अवरोध;
    }
  }

  अगर (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    overflow = true;

  अगर (!overflow) {
    exponent = unsignedExponent;
    अगर (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    अगर (exponent > 32767 || exponent < -32768)
      overflow = true;
  }

  अगर (overflow)
    exponent = negative ? -32768: 32767;

  वापस exponent;
}

अटल Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator end,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = end;
  जबतक (p != end && *p == '0')
    p++;

  अगर (p != end && *p == '.') {
    *dot = p++;

    अगर (end - begin == 1)
      वापस createError("Significand has no digits");

    जबतक (p != end && *p == '0')
      p++;
  }

  वापस p;
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
काष्ठा decimalInfo {
  स्थिर अक्षर *firstSigDigit;
  स्थिर अक्षर *lastSigDigit;
  पूर्णांक exponent;
  पूर्णांक normalizedExponent;
};

अटल Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator end, decimalInfo *D) {
  StringRef::iterator dot = end;

  स्वतः PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  अगर (!PtrOrErr)
    वापस PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->firstSigDigit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  क्रम (; p != end; ++p) {
    अगर (*p == '.') {
      अगर (dot != end)
        वापस createError("String contains multiple dots");
      dot = p++;
      अगर (p == end)
        अवरोध;
    }
    अगर (dec_Digit_मूल्य(*p) >= 10U)
      अवरोध;
  }

  अगर (p != end) {
    अगर (*p != 'e' && *p != 'E')
      वापस createError("Invalid character in significand");
    अगर (p == begin)
      वापस createError("Significand has no digits");
    अगर (dot != end && p - begin == 1)
      वापस createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    स्वतः ExpOrErr = readExponent(p + 1, end);
    अगर (!ExpOrErr)
      वापस ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    अगर (dot == end)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  अगर (p != D->firstSigDigit) {
    /* Drop insignificant trailing zeroes.  */
    अगर (p != begin) {
      करो
        करो
          p--;
        जबतक (p != begin && *p == '0');
      जबतक (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->firstSigDigit)
                                      - (dot > D->firstSigDigit && dot < p)));
  }

  D->lastSigDigit = p;
  वापस Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
अटल Expected<lostFraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator end,
                            अचिन्हित पूर्णांक digit_मूल्य) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  अगर (digit_मूल्य > 8)
    वापस lfMoreThanHalf;
  अन्यथा अगर (digit_मूल्य < 8 && digit_मूल्य > 0)
    वापस lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  जबतक (p != end && (*p == '0' || *p == '.'))
    p++;

  अगर (p == end)
    वापस createError("Invalid trailing hexadecimal fraction!");

  अचिन्हित hexDigit = hex_Digit_मूल्य(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  अगर (hexDigit == अच_पूर्णांक_उच्च)
    वापस digit_मूल्य == 0 ? lfExactlyZero: lfExactlyHalf;
  अन्यथा
    वापस digit_मूल्य == 0 ? lfLessThanHalf: lfMoreThanHalf;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
अटल lostFraction
lostFractionThroughTruncation(स्थिर APFloatBase::integerPart *parts,
                              अचिन्हित पूर्णांक part_गिनती,
                              अचिन्हित पूर्णांक bits)
{
  अचिन्हित lsb = APInt::tcLSB(parts, part_गिनती);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  अगर (bits <= lsb)
    वापस lfExactlyZero;
  अगर (bits == lsb + 1)
    वापस lfExactlyHalf;
  अगर (bits <= part_गिनती * APFloatBase::integerPartWidth &&
      APInt::tcExtractBit(parts, bits - 1))
    वापस lfMoreThanHalf;

  वापस lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
अटल lostFraction
shift_दायाँ(APFloatBase::integerPart *dst, अचिन्हित पूर्णांक parts, अचिन्हित पूर्णांक bits)
{
  lostFraction lost_fraction = lostFractionThroughTruncation(dst, parts, bits);

  APInt::tc_Shift_दायाँ(dst, parts, bits);

  वापस lost_fraction;
}

/* Combine the effect of two lost fractions.  */
अटल lostFraction
combineLostFractions(lostFraction moreSignificant,
                     lostFraction lessSignificant)
{
  अगर (lessSignificant != lfExactlyZero) {
    अगर (moreSignificant == lfExactlyZero)
      moreSignificant = lfLessThanHalf;
    अन्यथा अगर (moreSignificant == lfExactlyHalf)
      moreSignificant = lfMoreThanHalf;
  }

  वापस moreSignificant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
अटल अचिन्हित पूर्णांक
HUerrBound(bool inexactMultiply, अचिन्हित पूर्णांक HUerr1, अचिन्हित पूर्णांक HUerr2)
{
  निश्चित(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  अगर (HUerr1 + HUerr2 == 0)
    वापस inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  अन्यथा
    वापस inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
अटल APFloatBase::integerPart
ulpsFromBoundary(स्थिर APFloatBase::integerPart *parts, अचिन्हित पूर्णांक bits,
                 bool isNearest) {
  निश्चित(bits != 0);

  bits--;
  अचिन्हित गिनती = bits / APFloatBase::integerPartWidth;
  अचिन्हित partBits = bits % APFloatBase::integerPartWidth + 1;

  APFloatBase::integerPart part =
      parts[गिनती] & (~(APFloatBase::integerPart)0 >>
                      (APFloatBase::integerPartWidth - partBits));

  APFloatBase::integerPart boundary;
  अगर (isNearest)
    boundary = (APFloatBase::integerPart) 1 << (partBits - 1);
  अन्यथा
    boundary = 0;

  अगर (गिनती == 0) {
    अगर (part - boundary <= boundary - part)
      वापस part - boundary;
    अन्यथा
      वापस boundary - part;
  }

  अगर (part == boundary) {
    जबतक (--गिनती)
      अगर (parts[गिनती])
        वापस ~(APFloatBase::integerPart) 0; /* A lot.  */

    वापस parts[0];
  } अन्यथा अगर (part == boundary - 1) {
    जबतक (--गिनती)
      अगर (~parts[गिनती])
        वापस ~(APFloatBase::integerPart) 0; /* A lot.  */

    वापस -parts[0];
  }

  वापस ~(APFloatBase::integerPart) 0; /* A lot.  */
}

अटल constexpr अचिन्हित PowerOfFivePartCounts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

अटल constexpr APFloatBase::integerPart PowerOfFiveParts[] = {
#समावेश "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  अचिन्हित योग = 0;
  क्रम (अचिन्हित गिनती_2 : PowerOfFivePartCounts)
    योग += गिनती_2;
  वापस योग;
}() == std::आकार_2(PowerOfFiveParts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
अटल अचिन्हित पूर्णांक
powerOf5(APFloatBase::integerPart *dst, अचिन्हित पूर्णांक power) {
  अटल स्थिर APFloatBase::integerPart firstEightPowers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  SmallVector<APFloatBase::integerPart, अंक_Pow5_Parts_On_ढेर> scratch;
  scratch.resize_for_overwrite(powerOfFivePartsForExponent(power));

  APFloatBase::integerPart *p1, *p2;
  स्थिर APFloatBase::integerPart *pow5;

  p1 = dst;
  p2 = scratch.तथ्य();

  *p1 = firstEightPowers[power & 7];
  power >>= 3;

  अचिन्हित परिणाम_2 = 1;
  pow5 = PowerOfFiveParts;

  क्रम (अचिन्हित पूर्णांक n = 0; power; power >>= 1, n++) {
    अचिन्हित parts_गिनती = PowerOfFivePartCounts[n];

    अगर (power & 1) {
      APFloatBase::integerPart *tmp;

      APInt::tcFullMultiply(p2, p1, pow5, परिणाम_2, parts_गिनती);
      परिणाम_2 += parts_गिनती;
      अगर (p2[परिणाम_2 - 1] == 0)
        परिणाम_2--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_गिनती;
  }

  अगर (p1 != dst)
    APInt::tcAssign(dst, p1, परिणाम_2);

  वापस परिणाम_2;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
अटल स्थिर अक्षर hexDigitsLower[] = "0123456789abcdef0";
अटल स्थिर अक्षर hexDigitsUpper[] = "0123456789ABCDEF0";
अटल स्थिर अक्षर infinityL[] = "infinity";
अटल स्थिर अक्षर infinityU[] = "INFINITY";
अटल स्थिर अक्षर NaNL[] = "nan";
अटल स्थिर अक्षर NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
अटल अचिन्हित पूर्णांक
partAsHex (अक्षर *dst, APFloatBase::integerPart part, अचिन्हित पूर्णांक गिनती,
           स्थिर अक्षर *hexDigitChars)
{
  अचिन्हित पूर्णांक परिणाम_2 = गिनती;

  निश्चित(गिनती != 0 && गिनती <= APFloatBase::integerPartWidth / 4);

  part >>= (APFloatBase::integerPartWidth - 4 * गिनती);
  जबतक (गिनती--) {
    dst[गिनती] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  वापस परिणाम_2;
}

/* Write out an unsigned decimal integer.  */
अटल अक्षर *writeUnsignedDecimal(अक्षर *dst, अचिन्हित पूर्णांक n) {
  अक्षर buff[40], *p;

  p = buff;
  करो
    *p++ = '0' + n % 10;
  जबतक (n /= 10);

  करो
    *dst++ = *--p;
  जबतक (p != buff);

  वापस dst;
}

/* Write out a signed decimal integer.  */
अटल अक्षर *writeSignedDecimal(अक्षर *dst, पूर्णांक मूल्य_2) {
  अगर (मूल्य_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(अचिन्हित) मूल्य_2);
  } अन्यथा {
    dst = writeUnsignedDecimal(dst, मूल्य_2);
  }

  वापस dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
अटल APFloat harrisonUlp(स्थिर APFloat &X) {
  स्थिर fltSemantics &Sem = X.getSemantics();
  चयन (X.getCategory()) {
  हाल APFloat::fcNaN:
    वापस APFloat::getQNaN(Sem);
  हाल APFloat::fcInfinity:
    वापस APFloat::getInf(Sem);
  हाल APFloat::fcZero:
    वापस APFloat::getSmallest(Sem);
  हाल APFloat::fcNormal:
    अवरोध;
  }
  अगर (X.isDenormal() || X.isSmallestNormalized())
    वापस APFloat::getSmallest(Sem);
  पूर्णांक Exp = ilogb(X);
  अगर (X.getExactLog2() != पूर्णांक_न्यून)
    Exp -= 1;
  वापस scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rmNearestTiesToEven);
}

namespace detail {
/* Constructors.  */
व्योम IEEEFloat::initialize(स्थिर fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  अचिन्हित गिनती = part_गिनती();
  अगर (गिनती > 1)
    significand.parts = नया integerPart[गिनती];
}

व्योम IEEEFloat::freeSignificand() {
  अगर (needsCleanup())
    मिटाओ [] significand.parts;
}

व्योम IEEEFloat::assign(स्थिर IEEEFloat &rhs) {
  निश्चित(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  अगर (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

व्योम IEEEFloat::copySignificand(स्थिर IEEEFloat &rhs) {
  निश्चित(isFiniteNonZero() || category == fcNaN);
  निश्चित(rhs.part_गिनती() >= part_गिनती());

  APInt::tcAssign(significandParts(), rhs.significandParts(),
                  part_गिनती());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
व्योम IEEEFloat::makeNaN(bool SNaN, bool Negative, स्थिर APInt *fill) {
  अगर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  अगर (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  अचिन्हित अंक_Parts = part_गिनती();

  APInt fill_storage;
  अगर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = false;
    अगर (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = true;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } अन्यथा {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  अगर (!fill || fill->get_अंक_शब्दगण() < अंक_Parts)
    APInt::tcSet(significand, 0, अंक_Parts);
  अगर (fill) {
    APInt::tcAssign(significand, fill->get_Raw_तथ्य(),
                    std::min(fill->get_अंक_शब्दगण(), अंक_Parts));

    // Zero out the excess bits of the significand.
    अचिन्हित bitsToPreserve = semantics->precision - 1;
    अचिन्हित part = bitsToPreserve / 64;
    bitsToPreserve %= 64;
    significand[part] &= ((1ULL << bitsToPreserve) - 1);
    क्रम (part++; part != अंक_Parts; ++part)
      significand[part] = 0;
  }

  अचिन्हित QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  अगर (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tcClearBit(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    अगर (APInt::tcIsZero(significand, अंक_Parts))
      APInt::tcSetBit(significand, QNaNBit - 1);
  } अन्यथा अगर (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } अन्यथा {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tcSetBit(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  अगर (semantics == &APFloatBase::semX87DoubleExtended)
    APInt::tcSetBit(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::चालक=(स्थिर IEEEFloat &rhs) {
  अगर (यह != &rhs) {
    अगर (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    assign(rhs);
  }

  वापस *यह;
}

IEEEFloat &IEEEFloat::चालक=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloatBase::semBogus;
  वापस *यह;
}

bool IEEEFloat::isDenormal() स्थिर {
  वापस getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tcExtractBit(significandParts(), semantics->precision - 1) ==
          0);
}

bool IEEEFloat::isSmallest() स्थिर {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  वापस isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

bool IEEEFloat::isSmallestNormalized() स्थिर {
  वापस getCategory() == fcNormal && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

अचिन्हित पूर्णांक IEEEFloat::get_अंक_ऊपरी_Bits() स्थिर {
  स्थिर अचिन्हित पूर्णांक Part_गिनती = part_गिनती_For_Bits(semantics->precision);
  स्थिर अचिन्हित पूर्णांक Bits = Part_गिनती * integerPartWidth;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  स्थिर अचिन्हित पूर्णांक अंक_ऊपरी_Bits = (semantics->precision > 1)
                                       ? (Bits - semantics->precision + 1)
                                       : (Bits - semantics->precision);
  वापस अंक_ऊपरी_Bits;
}

bool IEEEFloat::isSignificandAllOnes() स्थिर {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  स्थिर integerPart *Parts = significandParts();
  स्थिर अचिन्हित Part_गिनती = part_गिनती_For_Bits(semantics->precision);
  क्रम (अचिन्हित i = 0; i < Part_गिनती - 1; i++)
    अगर (~Parts[i])
      वापस false;

  // Set the unused high bits to all ones when we compare.
  स्थिर अचिन्हित अंक_ऊपरी_Bits = get_अंक_ऊपरी_Bits();
  निश्चित(अंक_ऊपरी_Bits <= integerPartWidth && अंक_ऊपरी_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  स्थिर integerPart ऊपरी_Bit_Fill =
    ~integerPart(0) << (integerPartWidth - अंक_ऊपरी_Bits);
  अगर ((semantics->precision <= 1) || (~(Parts[Part_गिनती - 1] | ऊपरी_Bit_Fill)))
    वापस false;

  वापस true;
}

bool IEEEFloat::isSignificandAllOnesExceptLSB() स्थिर {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  स्थिर integerPart *Parts = significandParts();

  अगर (Parts[0] & 1)
    वापस false;

  स्थिर अचिन्हित Part_गिनती = part_गिनती_For_Bits(semantics->precision);
  क्रम (अचिन्हित i = 0; i < Part_गिनती - 1; i++) {
    अगर (~Parts[i] & ~अचिन्हित{!i})
      वापस false;
  }

  // Set the unused high bits to all ones when we compare.
  स्थिर अचिन्हित अंक_ऊपरी_Bits = get_अंक_ऊपरी_Bits();
  निश्चित(अंक_ऊपरी_Bits <= integerPartWidth && अंक_ऊपरी_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  स्थिर integerPart ऊपरी_Bit_Fill = ~integerPart(0)
                                  << (integerPartWidth - अंक_ऊपरी_Bits);
  अगर (~(Parts[Part_गिनती - 1] | ऊपरी_Bit_Fill | 0x1))
    वापस false;

  वापस true;
}

bool IEEEFloat::isSignificandAllZeros() स्थिर {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  स्थिर integerPart *Parts = significandParts();
  स्थिर अचिन्हित Part_गिनती = part_गिनती_For_Bits(semantics->precision);

  क्रम (अचिन्हित i = 0; i < Part_गिनती - 1; i++)
    अगर (Parts[i])
      वापस false;

  // Compute how many bits are used in the final word.
  स्थिर अचिन्हित अंक_ऊपरी_Bits = get_अंक_ऊपरी_Bits();
  निश्चित(अंक_ऊपरी_Bits < integerPartWidth && "Can not have more high bits to "
         "clear than integerPartWidth");
  स्थिर integerPart ऊपरी_Bit_Mask = ~integerPart(0) >> अंक_ऊपरी_Bits;

  अगर ((semantics->precision > 1) && (Parts[Part_गिनती - 1] & ऊपरी_Bit_Mask))
    वापस false;

  वापस true;
}

bool IEEEFloat::isSignificandAllZerosExceptMSB() स्थिर {
  स्थिर integerPart *Parts = significandParts();
  स्थिर अचिन्हित Part_गिनती = part_गिनती_For_Bits(semantics->precision);

  क्रम (अचिन्हित i = 0; i < Part_गिनती - 1; i++) {
    अगर (Parts[i])
      वापस false;
  }

  स्थिर अचिन्हित अंक_ऊपरी_Bits = get_अंक_ऊपरी_Bits();
  स्थिर integerPart MSBMask = integerPart(1)
                              << (integerPartWidth - अंक_ऊपरी_Bits);
  वापस ((semantics->precision <= 1) || (Parts[Part_गिनती - 1] == MSBMask));
}

bool IEEEFloat::isLargest() स्थिर {
  bool IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  अगर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    वापस (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } अन्यथा {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    वापस IsMaxExp && isSignificandAllOnes();
  }
}

bool IEEEFloat::isInteger() स्थिर {
  // This could be made more efficient; I'm going for obviously correct.
  अगर (!isFinite()) वापस false;
  IEEEFloat truncated = *यह;
  truncated.roundToIntegral(rmTowardZero);
  वापस compare(truncated) == cmpEqual;
}

bool IEEEFloat::bitwiseIsEqual(स्थिर IEEEFloat &rhs) स्थिर {
  अगर (यह == &rhs)
    वापस true;
  अगर (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    वापस false;
  अगर (category==fcZero || category==fcInfinity)
    वापस true;

  अगर (isFiniteNonZero() && exponent != rhs.exponent)
    वापस false;

  वापस std::equal(significandParts(), significandParts() + part_गिनती(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(स्थिर fltSemantics &ourSemantics, integerPart मूल्य_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fcNormal;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = मूल्य_2;
  normalize(rmNearestTiesToEven, lfExactlyZero);
}

IEEEFloat::IEEEFloat(स्थिर fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(false) : makeSmallestNormalized(false);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(स्थिर fltSemantics &ourSemantics, uninitializedTag tag)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(स्थिर IEEEFloat &rhs) {
  initialize(rhs.semantics);
  assign(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloatBase::semBogus) {
  *यह = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

अचिन्हित पूर्णांक IEEEFloat::part_गिनती() स्थिर {
  वापस part_गिनती_For_Bits(semantics->precision + 1);
}

स्थिर APFloat::integerPart *IEEEFloat::significandParts() स्थिर {
  वापस const_cast<IEEEFloat *>(यह)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  अगर (part_गिनती() > 1)
    वापस significand.parts;
  अन्यथा
    वापस &significand.part;
}

व्योम IEEEFloat::zeroSignificand() {
  APInt::tcSet(significandParts(), 0, part_गिनती());
}

/* Increment an fcNormal floating point number's significand.  */
व्योम IEEEFloat::incrementSignificand() {
  [[maybe_unused]] integerPart carry =
      APInt::tcIncrement(significandParts(), part_गिनती());

  /* Our callers should never cause us to overflow.  */
  निश्चित(carry == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::addSignificand(स्थिर IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  निश्चित(semantics == rhs.semantics);
  निश्चित(exponent == rhs.exponent);

  वापस APInt::tcAdd(parts, rhs.significandParts(), 0, part_गिनती());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::subtractSignificand(स्थिर IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  निश्चित(semantics == rhs.semantics);
  निश्चित(exponent == rhs.exponent);

  वापस APInt::tcSubtract(parts, rhs.significandParts(), borrow,
                           part_गिनती());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
lostFraction IEEEFloat::multiplySignificand(स्थिर IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            bool ignoreAddend) {
  integerPart scratch[4];
  bool ignored;

  निश्चित(semantics == rhs.semantics);

  अचिन्हित precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  अचिन्हित new_Parts_गिनती = part_गिनती_For_Bits(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *fullSignificand =
      new_Parts_गिनती > 4 ? नया integerPart[new_Parts_गिनती] : scratch;

  integerPart *lhsSignificand = significandParts();
  अचिन्हित parts_गिनती = part_गिनती();

  APInt::tcFullMultiply(fullSignificand, lhsSignificand,
                        rhs.significandParts(), parts_गिनती, parts_गिनती);

  lostFraction lost_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  अचिन्हित omsb = APInt::tcMSB(fullSignificand, new_Parts_गिनती) + 1;
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

  अगर (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    स्थिर fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    अचिन्हित extendedPrecision = 2 * precision + 1;
    अगर (omsb != extendedPrecision - 1) {
      निश्चित(extendedPrecision > omsb);
      APInt::tc_Shift_बायाँ(fullSignificand, new_Parts_गिनती,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    अगर (new_Parts_गिनती == 1)
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
    निश्चित(status == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    lost_fraction = extendedAddend.shift_Significand_दायाँ(1);
    निश्चित(lost_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    lost_fraction = addOrSubtractSignificand(extendedAddend, false);

    /* Restore our state.  */
    अगर (new_Parts_गिनती == 1)
      fullSignificand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(fullSignificand, new_Parts_गिनती) + 1;
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
  अगर (omsb > precision) {
    अचिन्हित पूर्णांक bits, significantParts;
    lostFraction lf;

    bits = omsb - precision;
    significantParts = part_गिनती_For_Bits(omsb);
    lf = shift_दायाँ(fullSignificand, significantParts, bits);
    lost_fraction = combineLostFractions(lf, lost_fraction);
    exponent += bits;
  }

  APInt::tcAssign(lhsSignificand, fullSignificand, parts_गिनती);

  अगर (new_Parts_गिनती > 4)
    मिटाओ [] fullSignificand;

  वापस lost_fraction;
}

lostFraction IEEEFloat::multiplySignificand(स्थिर IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  वापस multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
lostFraction IEEEFloat::divideSignificand(स्थिर IEEEFloat &rhs) {
  integerPart scratch[4];

  निश्चित(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  स्थिर integerPart *rhsSignificand = rhs.significandParts();
  अचिन्हित parts_गिनती = part_गिनती();

  integerPart *dividend =
      parts_गिनती > 2 ? नया integerPart[parts_गिनती * 2] : scratch;
  integerPart *भाजक = dividend + parts_गिनती;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  क्रम (अचिन्हित i = 0; i < parts_गिनती; i++) {
    dividend[i] = lhsSignificand[i];
    भाजक[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  अचिन्हित पूर्णांक precision = semantics->precision;

  /* Normalize the divisor.  */
  अचिन्हित bit = precision - APInt::tcMSB(भाजक, parts_गिनती) - 1;
  अगर (bit) {
    exponent += bit;
    APInt::tc_Shift_बायाँ(भाजक, parts_गिनती, bit);
  }

  /* Normalize the dividend.  */
  bit = precision - APInt::tcMSB(dividend, parts_गिनती) - 1;
  अगर (bit) {
    exponent -= bit;
    APInt::tc_Shift_बायाँ(dividend, parts_गिनती, bit);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  अगर (APInt::tcCompare(dividend, भाजक, parts_गिनती) < 0) {
    exponent--;
    APInt::tc_Shift_बायाँ(dividend, parts_गिनती, 1);
    निश्चित(APInt::tcCompare(dividend, भाजक, parts_गिनती) >= 0);
  }

  /* Long division.  */
  क्रम (bit = precision; bit; bit -= 1) {
    अगर (APInt::tcCompare(dividend, भाजक, parts_गिनती) >= 0) {
      APInt::tcSubtract(dividend, भाजक, 0, parts_गिनती);
      APInt::tcSetBit(lhsSignificand, bit - 1);
    }

    APInt::tc_Shift_बायाँ(dividend, parts_गिनती, 1);
  }

  /* Figure out the lost fraction.  */
  पूर्णांक cmp = APInt::tcCompare(dividend, भाजक, parts_गिनती);

  lostFraction lost_fraction;
  अगर (cmp > 0)
    lost_fraction = lfMoreThanHalf;
  अन्यथा अगर (cmp == 0)
    lost_fraction = lfExactlyHalf;
  अन्यथा अगर (APInt::tcIsZero(dividend, parts_गिनती))
    lost_fraction = lfExactlyZero;
  अन्यथा
    lost_fraction = lfLessThanHalf;

  अगर (parts_गिनती > 2)
    मिटाओ [] dividend;

  वापस lost_fraction;
}

अचिन्हित पूर्णांक IEEEFloat::significandMSB() स्थिर {
  वापस APInt::tcMSB(significandParts(), part_गिनती());
}

अचिन्हित पूर्णांक IEEEFloat::significandLSB() स्थिर {
  वापस APInt::tcLSB(significandParts(), part_गिनती());
}

/* Note that a zero result is NOT normalized to fcZero.  */
lostFraction IEEEFloat::shift_Significand_दायाँ(अचिन्हित पूर्णांक bits) {
  /* Our exponent should not overflow.  */
  निश्चित((ExponentType) (exponent + bits) >= exponent);

  exponent += bits;

  वापस shift_दायाँ(significandParts(), part_गिनती(), bits);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
व्योम IEEEFloat::shift_Significand_बायाँ(अचिन्हित पूर्णांक bits) {
  निश्चित(bits < semantics->precision ||
         (semantics->precision == 1 && bits <= 1));

  अगर (bits) {
    अचिन्हित पूर्णांक parts_गिनती = part_गिनती();

    APInt::tc_Shift_बायाँ(significandParts(), parts_गिनती, bits);
    exponent -= bits;

    निश्चित(!APInt::tcIsZero(significandParts(), parts_गिनती));
  }
}

APFloat::cmp_परिणाम IEEEFloat::compare_Absolute_मूल्य(स्थिर IEEEFloat &rhs) स्थिर {
  निश्चित(semantics == rhs.semantics);
  निश्चित(isFiniteNonZero());
  निश्चित(rhs.isFiniteNonZero());

  पूर्णांक compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  अगर (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_गिनती());

  अगर (compare > 0)
    वापस cmpGreaterThan;
  अन्यथा अगर (compare < 0)
    वापस cmpLessThan;
  अन्यथा
    वापस cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
अटल व्योम tcSetLeastSignificantBits(APInt::WordType *dst, अचिन्हित parts,
                                      अचिन्हित bits) {
  अचिन्हित i = 0;
  जबतक (bits > APInt::APINT_BITS_PER_WORD) {
    dst[i++] = ~(APInt::WordType)0;
    bits -= APInt::APINT_BITS_PER_WORD;
  }

  अगर (bits)
    dst[i++] = ~(APInt::WordType)0 >> (APInt::APINT_BITS_PER_WORD - bits);

  जबतक (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::opStatus IEEEFloat::handleOverflow(roundingMode rounding_mode) {
  अगर (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    अगर (rounding_mode == rmNearestTiesToEven ||
        rounding_mode == rmNearestTiesToAway ||
        (rounding_mode == rmTowardPositive && !sign) ||
        (rounding_mode == rmTowardNegative && sign)) {
      अगर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(false, sign);
      अन्यथा
        category = fcInfinity;
      वापस static_cast<opStatus>(opOverflow | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fcNormal;
  exponent = semantics->maxExponent;
  tcSetLeastSignificantBits(significandParts(), part_गिनती(),
                            semantics->precision);
  अगर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tcClearBit(significandParts(), 0);

  वापस opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
bool IEEEFloat::roundAwayFromZero(roundingMode rounding_mode,
                                  lostFraction lost_fraction,
                                  अचिन्हित पूर्णांक bit) स्थिर {
  /* NaNs and infinities should not have lost fractions.  */
  निश्चित(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  निश्चित(lost_fraction != lfExactlyZero);

  चयन (rounding_mode) {
  हाल rmNearestTiesToAway:
    वापस lost_fraction == lfExactlyHalf || lost_fraction == lfMoreThanHalf;

  हाल rmNearestTiesToEven:
    अगर (lost_fraction == lfMoreThanHalf)
      वापस true;

    /* Our zeroes don't have a significand to test.  */
    अगर (lost_fraction == lfExactlyHalf && category != fcZero)
      वापस APInt::tcExtractBit(significandParts(), bit);

    वापस false;

  हाल rmTowardZero:
    वापस false;

  हाल rmTowardPositive:
    वापस !sign;

  हाल rmTowardNegative:
    वापस sign;

  शेष:
    अवरोध;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::opStatus IEEEFloat::normalize(roundingMode rounding_mode,
                                       lostFraction lost_fraction) {
  अगर (!isFiniteNonZero())
    वापस opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  अचिन्हित omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  अगर (omsb || lost_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    पूर्णांक exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    अगर (exponent + exponentChange > semantics->maxExponent)
      वापस handleOverflow(rounding_mode);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    अगर (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    अगर (exponentChange < 0) {
      निश्चित(lost_fraction == lfExactlyZero);

      shift_Significand_बायाँ(-exponentChange);

      वापस opOK;
    }

    अगर (exponentChange > 0) {
      lostFraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = shift_Significand_दायाँ(exponentChange);

      lost_fraction = combineLostFractions(lf, lost_fraction);

      /* Keep OMSB up-to-date.  */
      अगर (omsb > (अचिन्हित) exponentChange)
        omsb -= exponentChange;
      अन्यथा
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  अगर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    वापस handleOverflow(rounding_mode);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  अगर (lost_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    अगर (omsb == 0) {
      category = fcZero;
      अगर (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = false;
      अगर (!semantics->hasZero)
        makeSmallestNormalized(false);
    }

    वापस opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  अगर (roundAwayFromZero(rounding_mode, lost_fraction, 0)) {
    अगर (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    अगर (omsb == (अचिन्हित) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      अगर (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        वापस handleOverflow(sign ? rmTowardNegative : rmTowardPositive);

      shift_Significand_दायाँ(1);

      वापस opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    अगर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      वापस handleOverflow(rounding_mode);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  अगर (omsb == semantics->precision)
    वापस opInexact;

  /* We have a non-zero denormal.  */
  निश्चित(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  अगर (omsb == 0) {
    category = fcZero;
    अगर (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = false;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    अगर (!semantics->hasZero)
      makeSmallestNormalized(false);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  वापस (opStatus) (opUnderflow | opInexact);
}

APFloat::opStatus IEEEFloat::addOrSubtractSpecials(स्थिर IEEEFloat &rhs,
                                                   bool subtract) {
  चयन (Pack_Categories_Into_कुंजी(category, rhs.category)) {
  शेष:
    llvm_unreachable(nullptr);

  हाल Pack_Categories_Into_कुंजी(fcZero, fcNaN):
  हाल Pack_Categories_Into_कुंजी(fcNormal, fcNaN):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcZero):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcNormal):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcNaN):
    अगर (isSignaling()) {
      makeQuiet();
      वापस opInvalidOp;
    }
    वापस rhs.isSignaling() ? opInvalidOp : opOK;

  हाल Pack_Categories_Into_कुंजी(fcNormal, fcZero):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcNormal):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcZero):
    वापस opOK;

  हाल Pack_Categories_Into_कुंजी(fcNormal, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ subtract;
    वापस opOK;

  हाल Pack_Categories_Into_कुंजी(fcZero, fcNormal):
    assign(rhs);
    sign = rhs.sign ^ subtract;
    वापस opOK;

  हाल Pack_Categories_Into_कुंजी(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    वापस opOK;

  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    अगर (((sign ^ rhs.sign)!=0) != subtract) {
      makeNaN();
      वापस opInvalidOp;
    }

    वापस opOK;

  हाल Pack_Categories_Into_कुंजी(fcNormal, fcNormal):
    वापस opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
lostFraction IEEEFloat::addOrSubtractSignificand(स्थिर IEEEFloat &rhs,
                                                 bool subtract) {
  [[maybe_unused]] integerPart carry = 0;
  lostFraction lost_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  subtract ^= static_cast<bool>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  पूर्णांक bits = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  अगर (subtract) {
    अगर ((bits < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat अस्थायी_rhs(rhs);
    bool lost_fraction_is_from_rhs = false;

    अगर (bits == 0)
      lost_fraction = lfExactlyZero;
    अन्यथा अगर (bits > 0) {
      lost_fraction = अस्थायी_rhs.shift_Significand_दायाँ(bits - 1);
      lost_fraction_is_from_rhs = true;
      shift_Significand_बायाँ(1);
    } अन्यथा {
      lost_fraction = shift_Significand_दायाँ(-bits - 1);
      अस्थायी_rhs.shift_Significand_बायाँ(1);
    }

    // Should we reverse the subtraction.
    cmp_परिणाम cmp_परिणाम_2 = compare_Absolute_मूल्य(अस्थायी_rhs);
    अगर (cmp_परिणाम_2 == cmpLessThan) {
      bool borrow =
          lost_fraction != lfExactlyZero && !lost_fraction_is_from_rhs;
      अगर (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        अगर (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        अन्यथा अगर (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = अस्थायी_rhs.subtractSignificand(*यह, borrow);
      copySignificand(अस्थायी_rhs);
      sign = !sign;
    } अन्यथा अगर (cmp_परिणाम_2 == cmpGreaterThan) {
      bool borrow = lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs;
      अगर (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        अगर (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        अन्यथा अगर (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = subtractSignificand(अस्थायी_rhs, borrow);
    } अन्यथा { // cmpEqual
      zeroSignificand();
      अगर (lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    निश्चित(!carry);
  } अन्यथा {
    अगर (bits > 0) {
      IEEEFloat अस्थायी_rhs(rhs);

      lost_fraction = अस्थायी_rhs.shift_Significand_दायाँ(bits);
      carry = addSignificand(अस्थायी_rhs);
    } अन्यथा {
      lost_fraction = shift_Significand_दायाँ(-bits);
      carry = addSignificand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    निश्चित(!carry);
  }

  वापस lost_fraction;
}

APFloat::opStatus IEEEFloat::multiplySpecials(स्थिर IEEEFloat &rhs) {
  चयन (Pack_Categories_Into_कुंजी(category, rhs.category)) {
  शेष:
    llvm_unreachable(nullptr);

  हाल Pack_Categories_Into_कुंजी(fcZero, fcNaN):
  हाल Pack_Categories_Into_कुंजी(fcNormal, fcNaN):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcNaN):
    assign(rhs);
    sign = false;
    [[fallthrough]];
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcZero):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcNormal):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    अगर (isSignaling()) {
      makeQuiet();
      वापस opInvalidOp;
    }
    वापस rhs.isSignaling() ? opInvalidOp : opOK;

  हाल Pack_Categories_Into_कुंजी(fcNormal, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcNormal):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcInfinity):
    category = fcInfinity;
    वापस opOK;

  हाल Pack_Categories_Into_कुंजी(fcZero, fcNormal):
  हाल Pack_Categories_Into_कुंजी(fcNormal, fcZero):
  हाल Pack_Categories_Into_कुंजी(fcZero, fcZero):
    category = fcZero;
    वापस opOK;

  हाल Pack_Categories_Into_कुंजी(fcZero, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcZero):
    makeNaN();
    वापस opInvalidOp;

  हाल Pack_Categories_Into_कुंजी(fcNormal, fcNormal):
    वापस opOK;
  }
}

APFloat::opStatus IEEEFloat::divideSpecials(स्थिर IEEEFloat &rhs) {
  चयन (Pack_Categories_Into_कुंजी(category, rhs.category)) {
  शेष:
    llvm_unreachable(nullptr);

  हाल Pack_Categories_Into_कुंजी(fcZero, fcNaN):
  हाल Pack_Categories_Into_कुंजी(fcNormal, fcNaN):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcNaN):
    assign(rhs);
    sign = false;
    [[fallthrough]];
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcZero):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcNormal):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    अगर (isSignaling()) {
      makeQuiet();
      वापस opInvalidOp;
    }
    वापस rhs.isSignaling() ? opInvalidOp : opOK;

  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcZero):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcNormal):
  हाल Pack_Categories_Into_कुंजी(fcZero, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcZero, fcNormal):
    वापस opOK;

  हाल Pack_Categories_Into_कुंजी(fcNormal, fcInfinity):
    category = fcZero;
    वापस opOK;

  हाल Pack_Categories_Into_कुंजी(fcNormal, fcZero):
    अगर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(false, sign);
    अन्यथा
      category = fcInfinity;
    वापस opDivByZero;

  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcZero, fcZero):
    makeNaN();
    वापस opInvalidOp;

  हाल Pack_Categories_Into_कुंजी(fcNormal, fcNormal):
    वापस opOK;
  }
}

APFloat::opStatus IEEEFloat::modSpecials(स्थिर IEEEFloat &rhs) {
  चयन (Pack_Categories_Into_कुंजी(category, rhs.category)) {
  शेष:
    llvm_unreachable(nullptr);

  हाल Pack_Categories_Into_कुंजी(fcZero, fcNaN):
  हाल Pack_Categories_Into_कुंजी(fcNormal, fcNaN):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcZero):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcNormal):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcNaN):
    अगर (isSignaling()) {
      makeQuiet();
      वापस opInvalidOp;
    }
    वापस rhs.isSignaling() ? opInvalidOp : opOK;

  हाल Pack_Categories_Into_कुंजी(fcZero, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcZero, fcNormal):
  हाल Pack_Categories_Into_कुंजी(fcNormal, fcInfinity):
    वापस opOK;

  हाल Pack_Categories_Into_कुंजी(fcNormal, fcZero):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcZero):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcNormal):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcZero, fcZero):
    makeNaN();
    वापस opInvalidOp;

  हाल Pack_Categories_Into_कुंजी(fcNormal, fcNormal):
    वापस opOK;
  }
}

APFloat::opStatus IEEEFloat::remainderSpecials(स्थिर IEEEFloat &rhs) {
  चयन (Pack_Categories_Into_कुंजी(category, rhs.category)) {
  शेष:
    llvm_unreachable(nullptr);

  हाल Pack_Categories_Into_कुंजी(fcZero, fcNaN):
  हाल Pack_Categories_Into_कुंजी(fcNormal, fcNaN):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcZero):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcNormal):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcNaN):
    अगर (isSignaling()) {
      makeQuiet();
      वापस opInvalidOp;
    }
    वापस rhs.isSignaling() ? opInvalidOp : opOK;

  हाल Pack_Categories_Into_कुंजी(fcZero, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcZero, fcNormal):
  हाल Pack_Categories_Into_कुंजी(fcNormal, fcInfinity):
    वापस opOK;

  हाल Pack_Categories_Into_कुंजी(fcNormal, fcZero):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcZero):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcNormal):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcZero, fcZero):
    makeNaN();
    वापस opInvalidOp;

  हाल Pack_Categories_Into_कुंजी(fcNormal, fcNormal):
    वापस opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
व्योम IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  अगर (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    वापस;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::opStatus IEEEFloat::addOrSubtract(स्थिर IEEEFloat &rhs,
                                           roundingMode rounding_mode,
                                           bool subtract) {
  opStatus fs = addOrSubtractSpecials(rhs, subtract);

  /* This return code means it was not a simple case.  */
  अगर (fs == opDivByZero) {
    lostFraction lost_fraction;

    lost_fraction = addOrSubtractSignificand(rhs, subtract);
    fs = normalize(rounding_mode, lost_fraction);

    /* Can only be zero if we lost no fraction.  */
    निश्चित(category != fcZero || lost_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  अगर (category == fcZero) {
    अगर (rhs.category != fcZero || (sign == rhs.sign) == subtract)
      sign = (rounding_mode == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    अगर (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = false;
  }

  वापस fs;
}

/* Normalized addition.  */
APFloat::opStatus IEEEFloat::add(स्थिर IEEEFloat &rhs,
                                 roundingMode rounding_mode) {
  वापस addOrSubtract(rhs, rounding_mode, false);
}

/* Normalized subtraction.  */
APFloat::opStatus IEEEFloat::subtract(स्थिर IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  वापस addOrSubtract(rhs, rounding_mode, true);
}

/* Normalized multiply.  */
APFloat::opStatus IEEEFloat::multiply(स्थिर IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = multiplySpecials(rhs);

  अगर (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = false;
  अगर (isFiniteNonZero()) {
    lostFraction lost_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    अगर (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  वापस fs;
}

/* Normalized divide.  */
APFloat::opStatus IEEEFloat::divide(स्थिर IEEEFloat &rhs,
                                    roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = divideSpecials(rhs);

  अगर (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = false;
  अगर (isFiniteNonZero()) {
    lostFraction lost_fraction = divideSignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    अगर (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  वापस fs;
}

/* Normalized remainder.  */
APFloat::opStatus IEEEFloat::remainder(स्थिर IEEEFloat &rhs) {
  अचिन्हित पूर्णांक origSign = sign;

  // First handle the special cases.
  opStatus fs = remainderSpecials(rhs);
  अगर (fs != opDivByZero)
    वापस fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  अगर (P2.add(rhs, rmNearestTiesToEven) == opOK) {
    fs = mod(P2);
    निश्चित(fs == opOK);
  }

  // Lets work with absolute numbers.
  IEEEFloat P = rhs;
  P.sign = false;
  sign = false;

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
  bool losesInfo;
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.maxExponent++;
  extendedSemantics.minExponent--;
  extendedSemantics.precision += 2;

  IEEEFloat VEx = *यह;
  fs = VEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  निश्चित(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  निश्चित(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.add(VEx, rmNearestTiesToEven);
  निश्चित(fs == opOK);

  अगर (VEx.compare(PEx) == cmpGreaterThan) {
    fs = subtract(P, rmNearestTiesToEven);
    निश्चित(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    निश्चित(fs == opOK);
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    निश्चित(fs == opOK);

    cmp_परिणाम परिणाम_2 = VEx.compare(PEx);
    अगर (परिणाम_2 == cmpGreaterThan || परिणाम_2 == cmpEqual) {
      fs = subtract(P, rmNearestTiesToEven);
      निश्चित(fs == opOK);
    }
  }

  अगर (isZero()) {
    sign = origSign;    // IEEE754 requires this
    अगर (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = false;
  } अन्यथा {
    sign ^= origSign;
  }
  वापस fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::opStatus IEEEFloat::mod(स्थिर IEEEFloat &rhs) {
  opStatus fs = modSpecials(rhs);
  अचिन्हित पूर्णांक origSign = sign;

  जबतक (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_मूल्य(rhs) != cmpLessThan) {
    पूर्णांक Exp = ilogb(*यह) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rmNearestTiesToEven);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    अगर (V.isNaN() || compare_Absolute_मूल्य(V) == cmpLessThan)
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
    अगर (!semantics->hasZero && यह->isSmallest())
      अवरोध;

    निश्चित(fs==opOK);
  }
  अगर (isZero()) {
    sign = origSign; // fmod requires this
    अगर (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = false;
  }
  वापस fs;
}

/* Normalized fused-multiply-add.  */
APFloat::opStatus IEEEFloat::fusedMultiplyAdd(स्थिर IEEEFloat &multiplicand,
                                              स्थिर IEEEFloat &addend,
                                              roundingMode rounding_mode) {
  opStatus fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  अगर (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    lostFraction lost_fraction;

    lost_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_mode, lost_fraction);
    अगर (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    अगर (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_mode == rmTowardNegative);
      अगर (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = false;
    }
  } अन्यथा {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    अगर (fs == opOK)
      fs = addOrSubtract(addend, rounding_mode, false);
  }

  वापस fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::opStatus IEEEFloat::roundToIntegral(roundingMode rounding_mode) {
  अगर (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    वापस opOK;

  अगर (isNaN()) {
    अगर (isSignaling()) {
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
      वापस opInvalidOp;
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
      वापस opOK;
    }
  }

  अगर (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    वापस opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  अगर (exponent + 1 >= (पूर्णांक)APFloat::semanticsPrecision(*semantics))
    वापस opOK;

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
  opStatus fs = MagicConstant.convertFromAPInt(IntegerConstant, false,
                                               rmNearestTiesToEven);
  निश्चित(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  bool निवेश_Sign = isNegative();

  fs = add(MagicConstant, rounding_mode);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  subtract(MagicConstant, rounding_mode);

  // Restore the input sign.
  अगर (निवेश_Sign != isNegative())
    changeSign();

  वापस fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmp_परिणाम IEEEFloat::compare(स्थिर IEEEFloat &rhs) स्थिर {
  निश्चित(semantics == rhs.semantics);

  चयन (Pack_Categories_Into_कुंजी(category, rhs.category)) {
  शेष:
    llvm_unreachable(nullptr);

  हाल Pack_Categories_Into_कुंजी(fcNaN, fcZero):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcNormal):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcNaN, fcNaN):
  हाल Pack_Categories_Into_कुंजी(fcZero, fcNaN):
  हाल Pack_Categories_Into_कुंजी(fcNormal, fcNaN):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcNaN):
    वापस cmpUnordered;

  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcNormal):
  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcZero):
  हाल Pack_Categories_Into_कुंजी(fcNormal, fcZero):
    अगर (sign)
      वापस cmpLessThan;
    अन्यथा
      वापस cmpGreaterThan;

  हाल Pack_Categories_Into_कुंजी(fcNormal, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcZero, fcInfinity):
  हाल Pack_Categories_Into_कुंजी(fcZero, fcNormal):
    अगर (rhs.sign)
      वापस cmpGreaterThan;
    अन्यथा
      वापस cmpLessThan;

  हाल Pack_Categories_Into_कुंजी(fcInfinity, fcInfinity):
    अगर (sign == rhs.sign)
      वापस cmpEqual;
    अन्यथा अगर (sign)
      वापस cmpLessThan;
    अन्यथा
      वापस cmpGreaterThan;

  हाल Pack_Categories_Into_कुंजी(fcZero, fcZero):
    वापस cmpEqual;

  हाल Pack_Categories_Into_कुंजी(fcNormal, fcNormal):
    अवरोध;
  }

  cmp_परिणाम परिणाम_2;
  /* Two normal numbers.  Do they have the same sign?  */
  अगर (sign != rhs.sign) {
    अगर (sign)
      परिणाम_2 = cmpLessThan;
    अन्यथा
      परिणाम_2 = cmpGreaterThan;
  } अन्यथा {
    /* Compare absolute values; invert result if negative.  */
    परिणाम_2 = compare_Absolute_मूल्य(rhs);

    अगर (sign) {
      अगर (परिणाम_2 == cmpLessThan)
        परिणाम_2 = cmpGreaterThan;
      अन्यथा अगर (परिणाम_2 == cmpGreaterThan)
        परिणाम_2 = cmpLessThan;
    }
  }

  वापस परिणाम_2;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::opStatus IEEEFloat::convert(स्थिर fltSemantics &toSemantics,
                                     roundingMode rounding_mode,
                                     bool *losesInfo) {
  opStatus fs;
  स्थिर fltSemantics &fromSemantics = *semantics;
  bool is_signaling = isSignaling();

  lostFraction lostFraction = lfExactlyZero;
  अचिन्हित new_Part_गिनती = part_गिनती_For_Bits(toSemantics.precision + 1);
  अचिन्हित old_Part_गिनती = part_गिनती();
  पूर्णांक shift = toSemantics.precision - fromSemantics.precision;

  bool X86SpecialNan = false;
  अगर (&fromSemantics == &APFloatBase::semX87DoubleExtended &&
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
  अगर (shift < 0 && isFiniteNonZero()) {
    पूर्णांक omsb = significandMSB() + 1;
    पूर्णांक exponentChange = omsb - fromSemantics.precision;
    अगर (exponent + exponentChange < toSemantics.minExponent)
      exponentChange = toSemantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, shift);
    अगर (exponentChange < 0) {
      shift -= exponentChange;
      exponent += exponentChange;
    } अन्यथा अगर (omsb <= -shift) {
      exponentChange = omsb + shift - 1; // leave at least one bit set
      shift -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  अगर (shift < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    lostFraction = shift_दायाँ(significandParts(), old_Part_गिनती, -shift);

  // Fix the storage so it can hold to new value.
  अगर (new_Part_गिनती > old_Part_गिनती) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = नया integerPart[new_Part_गिनती];
    APInt::tcSet(newParts, 0, new_Part_गिनती);
    अगर (isFiniteNonZero() || category==fcNaN)
      APInt::tcAssign(newParts, significandParts(), old_Part_गिनती);
    freeSignificand();
    significand.parts = newParts;
  } अन्यथा अगर (new_Part_गिनती == 1 && old_Part_गिनती != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    अगर (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &toSemantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  अगर (shift > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_Shift_बायाँ(significandParts(), new_Part_गिनती, shift);

  अगर (isFiniteNonZero()) {
    fs = normalize(rounding_mode, lostFraction);
    *losesInfo = (fs != opOK);
  } अन्यथा अगर (category == fcNaN) {
    अगर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(false, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } अन्यथा {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      अगर (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(false, false);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      अगर (!APFloat::hasSignificand(fromSemantics))
        makeNaN(false, sign);

      *losesInfo = lostFraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      अगर (!X86SpecialNan && semantics == &APFloatBase::semX87DoubleExtended)
        APInt::tcSetBit(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      अगर (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } अन्यथा {
        fs = opOK;
      }
    }
  } अन्यथा अगर (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(false, sign);
    *losesInfo = true;
    fs = opInexact;
  } अन्यथा अगर (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = false;
  } अन्यथा {
    *losesInfo = false;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  अगर ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = true;
    अगर (fs == opOK)
      fs = opInexact;
  }

  अगर (category == fcZero && !semantics->hasZero)
    makeSmallestNormalized(false);
  वापस fs;
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
    MutableArrayRef<integerPart> parts, अचिन्हित पूर्णांक width, bool isSigned,
    roundingMode rounding_mode, bool *isExact) स्थिर {
  *isExact = false;

  /* Handle the three special cases first.  */
  अगर (category == fcInfinity || category == fcNaN)
    वापस opInvalidOp;

  अचिन्हित dst_Parts_गिनती = part_गिनती_For_Bits(width);
  निश्चित(dst_Parts_गिनती <= parts.आकार_2() && "Integer too big");

  अगर (category == fcZero) {
    APInt::tcSet(parts.तथ्य(), 0, dst_Parts_गिनती);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    वापस opOK;
  }

  स्थिर integerPart *src = significandParts();

  अचिन्हित truncatedBits;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  अगर (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tcSet(parts.तथ्य(), 0, dst_Parts_गिनती);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncatedBits = semantics->precision -1U - exponent;
  } अन्यथा {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    अचिन्हित पूर्णांक bits = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    अगर (bits > width)
      वापस opInvalidOp;

    अगर (bits < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncatedBits = semantics->precision - bits;
      APInt::tcExtract(parts.तथ्य(), dst_Parts_गिनती, src, bits, truncatedBits);
    } अन्यथा {
      /* We want at least as many bits as are available.  */
      APInt::tcExtract(parts.तथ्य(), dst_Parts_गिनती, src, semantics->precision,
                       0);
      APInt::tc_Shift_बायाँ(parts.तथ्य(), dst_Parts_गिनती,
                         bits - semantics->precision);
      truncatedBits = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  lostFraction lost_fraction;
  अगर (truncatedBits) {
    lost_fraction = lostFractionThroughTruncation(src, part_गिनती(),
                                                  truncatedBits);
    अगर (lost_fraction != lfExactlyZero &&
        roundAwayFromZero(rounding_mode, lost_fraction, truncatedBits)) {
      अगर (APInt::tcIncrement(parts.तथ्य(), dst_Parts_गिनती))
        वापस opInvalidOp;     /* Overflow.  */
    }
  } अन्यथा {
    lost_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  अचिन्हित पूर्णांक omsb = APInt::tcMSB(parts.तथ्य(), dst_Parts_गिनती) + 1;

  अगर (sign) {
    अगर (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      अगर (omsb != 0)
        वापस opInvalidOp;
    } अन्यथा {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      अगर (omsb == width &&
          APInt::tcLSB(parts.तथ्य(), dst_Parts_गिनती) + 1 != omsb)
        वापस opInvalidOp;

      /* This case can happen because of rounding.  */
      अगर (omsb > width)
        वापस opInvalidOp;
    }

    APInt::tcNegate (parts.तथ्य(), dst_Parts_गिनती);
  } अन्यथा {
    अगर (omsb >= width + !isSigned)
      वापस opInvalidOp;
  }

  अगर (lost_fraction == lfExactlyZero) {
    *isExact = true;
    वापस opOK;
  }
  वापस opInexact;
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
                            अचिन्हित पूर्णांक width, bool isSigned,
                            roundingMode rounding_mode, bool *isExact) स्थिर {
  opStatus fs = convertToSignExtendedInteger(parts, width, isSigned,
                                             rounding_mode, isExact);

  अगर (fs == opInvalidOp) {
    अचिन्हित पूर्णांक bits, dst_Parts_गिनती;

    dst_Parts_गिनती = part_गिनती_For_Bits(width);
    निश्चित(dst_Parts_गिनती <= parts.आकार_2() && "Integer too big");

    अगर (category == fcNaN)
      bits = 0;
    अन्यथा अगर (sign)
      bits = isSigned;
    अन्यथा
      bits = width - isSigned;

    tcSetLeastSignificantBits(parts.तथ्य(), dst_Parts_गिनती, bits);
    अगर (sign && isSigned)
      APInt::tc_Shift_बायाँ(parts.तथ्य(), dst_Parts_गिनती, width - 1);
  }

  वापस fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::opStatus IEEEFloat::convertFromUnsignedParts(
    स्थिर integerPart *src, अचिन्हित पूर्णांक src_गिनती, roundingMode rounding_mode) {
  category = fcNormal;
  अचिन्हित omsb = APInt::tcMSB(src, src_गिनती) + 1;
  integerPart *dst = significandParts();
  अचिन्हित dst_गिनती = part_गिनती();
  अचिन्हित precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  lostFraction lost_fraction;
  अगर (precision <= omsb) {
    exponent = omsb - 1;
    lost_fraction = lostFractionThroughTruncation(src, src_गिनती,
                                                  omsb - precision);
    APInt::tcExtract(dst, dst_गिनती, src, precision, omsb - precision);
  } अन्यथा {
    exponent = precision - 1;
    lost_fraction = lfExactlyZero;
    APInt::tcExtract(dst, dst_गिनती, src, omsb, 0);
  }

  वापस normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus IEEEFloat::convertFromAPInt(स्थिर APInt &Val, bool isSigned,
                                              roundingMode rounding_mode) {
  अचिन्हित पूर्णांक part_गिनती = Val.get_अंक_शब्दगण();
  APInt api = Val;

  sign = false;
  अगर (isSigned && api.isNegative()) {
    sign = true;
    api = -api;
  }

  वापस convertFromUnsignedParts(api.get_Raw_तथ्य(), part_गिनती, rounding_mode);
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        roundingMode rounding_mode) {
  lostFraction lost_fraction = lfExactlyZero;

  category = fcNormal;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  अचिन्हित parts_गिनती = part_गिनती();
  अचिन्हित bitPos = parts_गिनती * integerPartWidth;
  bool computedTrailingFraction = false;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator end = s.end();
  StringRef::iterator dot;
  स्वतः PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  अगर (!PtrOrErr)
    वापस PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator firstSignificantDigit = p;

  जबतक (p != end) {
    integerPart hex_मूल्य;

    अगर (*p == '.') {
      अगर (dot != end)
        वापस createError("String contains multiple dots");
      dot = p++;
      जारी;
    }

    hex_मूल्य = hex_Digit_मूल्य(*p);
    अगर (hex_मूल्य == अच_पूर्णांक_उच्च)
      अवरोध;

    p++;

    // Store the number while we have space.
    अगर (bitPos) {
      bitPos -= 4;
      hex_मूल्य <<= bitPos % integerPartWidth;
      significand[bitPos / integerPartWidth] |= hex_मूल्य;
    } अन्यथा अगर (!computedTrailingFraction) {
      स्वतः FractOrErr = trailingHexadecimalFraction(p, end, hex_मूल्य);
      अगर (!FractOrErr)
        वापस FractOrErr.takeError();
      lost_fraction = *FractOrErr;
      computedTrailingFraction = true;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  अगर (p == end)
    वापस createError("Hex strings require an exponent");
  अगर (*p != 'p' && *p != 'P')
    वापस createError("Invalid character in significand");
  अगर (p == begin)
    वापस createError("Significand has no digits");
  अगर (dot != end && p - begin == 1)
    वापस createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  अगर (p != firstSignificantDigit) {
    पूर्णांक expAdjustment;

    /* Implicit hexadecimal point?  */
    अगर (dot == end)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<पूर्णांक>(dot - firstSignificantDigit);
    अगर (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_गिनती * integerPartWidth;

    /* Adjust for the given exponent.  */
    स्वतः ExpOrErr = योग_Exponent(p + 1, end, expAdjustment);
    अगर (!ExpOrErr)
      वापस ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  वापस normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus
IEEEFloat::roundSignificandWithExponent(स्थिर integerPart *decSigParts,
                                        अचिन्हित sig_Part_गिनती, पूर्णांक exp,
                                        roundingMode rounding_mode) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  अचिन्हित पूर्णांक power = exp >= 0 ? exp : -exp;
  अचिन्हित पूर्णांक pow5PartCapacity = powerOfFivePartsForExponent(power);
  SmallVector<integerPart, अंक_Pow5_Parts_On_ढेर> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5PartCapacity);

  bool isNearest = rounding_mode == rmNearestTiesToEven ||
                   rounding_mode == rmNearestTiesToAway;

  अचिन्हित parts = part_गिनती_For_Bits(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  अचिन्हित pow5_Part_गिनती = powerOf5(pow5Parts.तथ्य(), power);

  क्रम (;; parts *= 2) {
    अचिन्हित पूर्णांक excessPrecision, truncatedBits;

    calcSemantics.precision = parts * integerPartWidth - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncatedBits = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    opStatus sigStatus = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_गिनती, rmNearestTiesToEven);
    opStatus powStatus = pow5.convertFromUnsignedParts(
        pow5Parts.तथ्य(), pow5_Part_गिनती, rmNearestTiesToEven);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += exp;

    lostFraction calcLostFraction;
    integerPart HUerr, HUdistance;
    अचिन्हित पूर्णांक powHUerr;

    अगर (exp >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calcLostFraction = decSig.multiplySignificand(pow5);
      powHUerr = powStatus != opOK;
    } अन्यथा {
      calcLostFraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      अगर (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncatedBits = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (powStatus == opOK && calcLostFraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    निश्चित(APInt::tcExtractBit
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calcLostFraction != lfExactlyZero, sigStatus != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, isNearest);

    /* Are we guaranteed to round correctly if we truncate?  */
    अगर (HUdistance >= HUerr) {
      APInt::tcExtract(significandParts(), part_गिनती(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calcLostFraction = lostFractionThroughTruncation(decSig.significandParts(),
                                                       decSig.part_गिनती(),
                                                       truncatedBits);
      वापस static_cast<opStatus>(normalize(rounding_mode, calcLostFraction) |
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
  अगर (Error Err = interpretDecimal(p, str.end(), &D))
    वापस std::move(Err);

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
  अगर (D.firstSigDigit == str.end() || dec_Digit_मूल्य(*D.firstSigDigit) >= 10U) {
    category = fcZero;
    fs = opOK;
    अगर (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = false;
    अगर (!semantics->hasZero)
      makeSmallestNormalized(false);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } अन्यथा अगर (D.normalizedExponent - 1 > पूर्णांक_उच्च / 42039) {
    fs = handleOverflow(rounding_mode);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } अन्यथा अगर (D.normalizedExponent - 1 < पूर्णांक_न्यून / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (पूर्णांक) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fcNormal;
    zeroSignificand();
    fs = normalize(rounding_mode, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } अन्यथा अगर ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handleOverflow(rounding_mode);
  } अन्यथा {
    integerPart *decSignificand;
    अचिन्हित पूर्णांक part_गिनती;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_गिनती = static_cast<अचिन्हित पूर्णांक>(D.lastSigDigit - D.firstSigDigit) + 1;
    part_गिनती = part_गिनती_For_Bits(1 + 196 * part_गिनती / 59);
    decSignificand = नया integerPart[part_गिनती + 1];
    part_गिनती = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    करो {
      integerPart dec_मूल्य, val, multiplier;

      val = 0;
      multiplier = 1;

      करो {
        अगर (*p == '.') {
          p++;
          अगर (p == str.end()) {
            अवरोध;
          }
        }
        dec_मूल्य = dec_Digit_मूल्य(*p++);
        अगर (dec_मूल्य >= 10U) {
          मिटाओ[] decSignificand;
          वापस createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_मूल्य;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } जबतक (p <= D.lastSigDigit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_गिनती, part_गिनती + 1, false);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      अगर (decSignificand[part_गिनती])
        part_गिनती++;
    } जबतक (p <= D.lastSigDigit);

    category = fcNormal;
    fs = roundSignificandWithExponent(decSignificand, part_गिनती,
                                      D.exponent, rounding_mode);

    मिटाओ [] decSignificand;
  }

  वापस fs;
}

bool IEEEFloat::convertFromStringSpecials(StringRef str) {
  स्थिर माप_प्रकार MIN_NAME_आकार = 3;

  अगर (str.आकार_2() < MIN_NAME_आकार)
    वापस false;

  अगर (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(false);
    वापस true;
  }

  bool IsNegative = str.consume_front("-");
  अगर (IsNegative) {
    अगर (str.आकार_2() < MIN_NAME_आकार)
      वापस false;

    अगर (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(true);
      वापस true;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  bool IsSignaling = str.consume_front_insensitive("s");
  अगर (IsSignaling) {
    अगर (str.आकार_2() < MIN_NAME_आकार)
      वापस false;
  }

  अगर (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    अगर (str.empty()) {
      makeNaN(IsSignaling, IsNegative);
      वापस true;
    }

    // Allow the payload to be inside parentheses.
    अगर (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      अगर (str.आकार_2() <= 2 || str.back() != ')')
        वापस false;

      str = str.slice(1, str.आकार_2() - 1);
    }

    // Determine the payload number's radix.
    अचिन्हित Radix = 10;
    अगर (str[0] == '0') {
      अगर (str.आकार_2() > 1 && छोटे(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } अन्यथा {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    अगर (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      वापस true;
    }
  }

  वापस false;
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromString(StringRef str, roundingMode rounding_mode) {
  अगर (str.empty())
    वापस createError("Invalid string length");

  // Handle special cases.
  अगर (convertFromStringSpecials(str))
    वापस opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  माप_प्रकार slen = str.आकार_2();
  sign = *p == '-' ? 1 : 0;
  अगर (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  अगर (*p == '-' || *p == '+') {
    p++;
    slen--;
    अगर (!slen)
      वापस createError("String has no digits");
  }

  अगर (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    अगर (slen == 2)
      वापस createError("Invalid string");
    वापस convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_mode);
  }

  वापस convertFromDecimalString(StringRef(p, slen), rounding_mode);
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
                                           bool upperCase,
                                           roundingMode rounding_mode) स्थिर {
  अक्षर *p = dst;
  अगर (sign)
    *dst++ = '-';

  चयन (category) {
  हाल fcInfinity:
    स_नकल (dst, upperCase ? infinityU: infinityL, माप infinityU - 1);
    dst += माप infinityL - 1;
    अवरोध;

  हाल fcNaN:
    स_नकल (dst, upperCase ? NaNU: NaNL, माप NaNU - 1);
    dst += माप NaNU - 1;
    अवरोध;

  हाल fcZero:
    *dst++ = '0';
    *dst++ = upperCase ? 'X': 'x';
    *dst++ = '0';
    अगर (hexDigits > 1) {
      *dst++ = '.';
      स_रखो (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = upperCase ? 'P': 'p';
    *dst++ = '0';
    अवरोध;

  हाल fcNormal:
    dst = convertNormalToHexString (dst, hexDigits, upperCase, rounding_mode);
    अवरोध;
  }

  *dst = 0;

  वापस static_cast<अचिन्हित पूर्णांक>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
अक्षर *IEEEFloat::convertNormalToHexString(अक्षर *dst, अचिन्हित पूर्णांक hexDigits,
                                          bool upperCase,
                                          roundingMode rounding_mode) स्थिर {
  *dst++ = '0';
  *dst++ = upperCase ? 'X': 'x';

  bool roundUp = false;
  स्थिर अक्षर *hexDigitChars = upperCase ? hexDigitsUpper : hexDigitsLower;

  स्थिर integerPart *significand = significandParts();
  अचिन्हित parts_गिनती = part_गिनती();

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
  अगर (hexDigits) {
    अगर (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      अचिन्हित पूर्णांक bits;
      lostFraction fraction;

      bits = मूल्य_Bits - hexDigits * 4;
      fraction = lostFractionThroughTruncation (significand, parts_गिनती, bits);
      roundUp = roundAwayFromZero(rounding_mode, fraction, bits);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  अक्षर *p = ++dst;

  अचिन्हित गिनती = (मूल्य_Bits + integerPartWidth - 1) / integerPartWidth;

  जबतक (outputDigits && गिनती) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    अगर (--गिनती == parts_गिनती)
      part = 0;  /* An imaginary higher zero part.  */
    अन्यथा
      part = significand[गिनती] << shift;

    अगर (गिनती && shift)
      part |= significand[गिनती - 1] >> (integerPartWidth - shift);

    /* Convert as much of "part" to hexdigits as we can.  */
    अचिन्हित पूर्णांक curDigits = integerPartWidth / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  अगर (roundUp) {
    अक्षर *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    करो {
      q--;
      *q = hexDigitChars[hex_Digit_मूल्य (*q) + 1];
    } जबतक (*q == '0');
    निश्चित(q >= p);
  } अन्यथा {
    /* Add trailing zeroes.  */
    स_रखो (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  अगर (dst -1 == p)
    dst--;
  अन्यथा
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = upperCase ? 'P': 'p';

  वापस writeSignedDecimal (dst, exponent);
}

hash_code hash_मूल्य(स्थिर IEEEFloat &Arg) {
  अगर (!Arg.isFiniteNonZero())
    वापस hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  वापस hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.part_गिनती()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convertF80LongDoubleAPFloatToAPInt() स्थिर {
  निश्चित(part_गिनती() == 2);
  वापस convertIEEEFloatToAPInt<APFloatBase::semX87DoubleExtended>();
}

APInt IEEEFloat::convertPPCDoubleDoubleLegacyAPFloatToAPInt() स्थिर {
  निश्चित(semantics ==
         (स्थिर llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy);
  निश्चित(part_गिनती()==2);

  uint64_t शब्दगण[2];
  bool losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloatBase::semIEEEdouble.minExponent;
  IEEEFloat extended(*यह);
  [[maybe_unused]] opStatus fs =
      extended.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  निश्चित(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
  निश्चित(fs == opOK || fs == opInexact);
  शब्दगण[0] = *u.convertDoubleAPFloatToAPInt().get_Raw_तथ्य();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  अगर (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
    निश्चित(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.subtract(u, rmNearestTiesToEven);
    fs = v.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
    निश्चित(fs == opOK && !losesInfo);
    शब्दगण[1] = *v.convertDoubleAPFloatToAPInt().get_Raw_तथ्य();
  } अन्यथा {
    शब्दगण[1] = 0;
  }

  वापस APInt(128, शब्दगण);
}

ढाँचा <स्थिर fltSemantics &S>
APInt IEEEFloat::convertIEEEFloatToAPInt() स्थिर {
  निश्चित(semantics == &S);
  constexpr अचिन्हित पूर्णांक trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr पूर्णांक integer_bit_part = (S.precision - 1) / integerPartWidth;
  constexpr integerPart integer_bit = integerPart{1}
                                      << ((S.precision - 1) % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr अचिन्हित पूर्णांक exponent_bits =
      S.आकार_In_Bits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr bool is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr पूर्णांक bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_गिनती_For_Bits(trailing_significand_bits)>
      mysignificand;

  अगर (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.आकार_2(),
                mysignificand.begin());
    अगर (myexponent == 1 &&
        !(significandParts()[integer_bit_part] & integer_bit))
      myexponent = 0; // denormal
  } अन्यथा अगर (category == fcZero) {
    अगर (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } अन्यथा अगर (category == fcInfinity) {
    अगर (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    अगर constexpr (S.hasExplicitIntegerBit) {
      mysignificand[0] = integerPart{1} << (trailing_significand_bits - 1);
    }
  } अन्यथा {
    निश्चित(category == fcNaN && "Unknown category!");
    अगर (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.आकार_2(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.आकार_In_Bits + 63) / 64> शब्दगण;
  स्वतः शब्दगण_iter =
      std::copy_n(mysignificand.begin(), mysignificand.आकार_2(), शब्दगण.begin());
  अगर constexpr (!S.hasExplicitIntegerBit) {
    अगर constexpr (significand_mask != 0 || trailing_significand_bits == 0) {
      // Clear the integer bit.
      शब्दगण[mysignificand.आकार_2() - 1] &= significand_mask;
    }
  }
  std::fill(शब्दगण_iter, शब्दगण.end(), uint64_t{0});
  constexpr माप_प्रकार last_word = शब्दगण.आकार_2() - 1;
  अगर constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.आकार_In_Bits - 1) % 64);
    शब्दगण[last_word] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_mask)
                              << (trailing_significand_bits % 64);
  शब्दगण[last_word] |= shifted_exponent;
  अगर constexpr (last_word == 0) {
    वापस APInt(S.आकार_In_Bits, शब्दगण[0]);
  }
  वापस APInt(S.आकार_In_Bits, शब्दगण);
}

APInt IEEEFloat::convertQuadrupleAPFloatToAPInt() स्थिर {
  निश्चित(part_गिनती() == 2);
  वापस convertIEEEFloatToAPInt<APFloatBase::semIEEEquad>();
}

APInt IEEEFloat::convertDoubleAPFloatToAPInt() स्थिर {
  निश्चित(part_गिनती()==1);
  वापस convertIEEEFloatToAPInt<APFloatBase::semIEEEdouble>();
}

APInt IEEEFloat::convertFloatAPFloatToAPInt() स्थिर {
  निश्चित(part_गिनती()==1);
  वापस convertIEEEFloatToAPInt<APFloatBase::semIEEEsingle>();
}

APInt IEEEFloat::convertBFloatAPFloatToAPInt() स्थिर {
  निश्चित(part_गिनती() == 1);
  वापस convertIEEEFloatToAPInt<APFloatBase::semBFloat>();
}

APInt IEEEFloat::convertHalfAPFloatToAPInt() स्थिर {
  निश्चित(part_गिनती()==1);
  वापस convertIEEEFloatToAPInt<APFloatBase::APFloatBase::semIEEEhalf>();
}

APInt IEEEFloat::convertFloat8E5M2APFloatToAPInt() स्थिर {
  निश्चित(part_गिनती() == 1);
  वापस convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2>();
}

APInt IEEEFloat::convertFloat8E5M2FNUZAPFloatToAPInt() स्थिर {
  निश्चित(part_गिनती() == 1);
  वापस convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3APFloatToAPInt() स्थिर {
  निश्चित(part_गिनती() == 1);
  वापस convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3>();
}

APInt IEEEFloat::convertFloat8E4M3FNAPFloatToAPInt() स्थिर {
  निश्चित(part_गिनती() == 1);
  वापस convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FN>();
}

APInt IEEEFloat::convertFloat8E4M3FNUZAPFloatToAPInt() स्थिर {
  निश्चित(part_गिनती() == 1);
  वापस convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3B11FNUZAPFloatToAPInt() स्थिर {
  निश्चित(part_गिनती() == 1);
  वापस convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convertFloat8E3M4APFloatToAPInt() स्थिर {
  निश्चित(part_गिनती() == 1);
  वापस convertIEEEFloatToAPInt<APFloatBase::semFloat8E3M4>();
}

APInt IEEEFloat::convertFloatTF32APFloatToAPInt() स्थिर {
  निश्चित(part_गिनती() == 1);
  वापस convertIEEEFloatToAPInt<APFloatBase::semFloatTF32>();
}

APInt IEEEFloat::convertFloat8E8M0FNUAPFloatToAPInt() स्थिर {
  निश्चित(part_गिनती() == 1);
  वापस convertIEEEFloatToAPInt<APFloatBase::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convertFloat8E5M3FNUAPFloatToAPInt() स्थिर {
  निश्चित(part_गिनती() == 1);
  वापस convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convertFloat6E3M2FNAPFloatToAPInt() स्थिर {
  निश्चित(part_गिनती() == 1);
  वापस convertIEEEFloatToAPInt<APFloatBase::semFloat6E3M2FN>();
}

APInt IEEEFloat::convertFloat6E2M3FNAPFloatToAPInt() स्थिर {
  निश्चित(part_गिनती() == 1);
  वापस convertIEEEFloatToAPInt<APFloatBase::semFloat6E2M3FN>();
}

APInt IEEEFloat::convertFloat4E2M1FNAPFloatToAPInt() स्थिर {
  निश्चित(part_गिनती() == 1);
  वापस convertIEEEFloatToAPInt<APFloatBase::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcastToAPInt() स्थिर {
  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semIEEEhalf)
    वापस convertHalfAPFloatToAPInt();

  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semBFloat)
    वापस convertBFloatAPFloatToAPInt();

  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semIEEEsingle)
    वापस convertFloatAPFloatToAPInt();

  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semIEEEdouble)
    वापस convertDoubleAPFloatToAPInt();

  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semIEEEquad)
    वापस convertQuadrupleAPFloatToAPInt();

  अगर (semantics ==
      (स्थिर llvm::fltSemantics *)&APFloatBase::semX87DoubleExtended)
    वापस convertF80LongDoubleAPFloatToAPInt();

  अगर (semantics ==
      (स्थिर llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy)
    वापस convertPPCDoubleDoubleLegacyAPFloatToAPInt();

  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semFloat8E5M2)
    वापस convertFloat8E5M2APFloatToAPInt();

  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semFloat8E5M2FNUZ)
    वापस convertFloat8E5M2FNUZAPFloatToAPInt();

  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semFloat8E4M3)
    वापस convertFloat8E4M3APFloatToAPInt();

  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FN)
    वापस convertFloat8E4M3FNAPFloatToAPInt();

  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FNUZ)
    वापस convertFloat8E4M3FNUZAPFloatToAPInt();

  अगर (semantics ==
      (स्थिर llvm::fltSemantics *)&APFloatBase::semFloat8E4M3B11FNUZ)
    वापस convertFloat8E4M3B11FNUZAPFloatToAPInt();

  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semFloat8E3M4)
    वापस convertFloat8E3M4APFloatToAPInt();

  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semFloatTF32)
    वापस convertFloatTF32APFloatToAPInt();

  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semFloat8E8M0FNU)
    वापस convertFloat8E8M0FNUAPFloatToAPInt();

  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semFloat8E5M3FNU)
    वापस convertFloat8E5M3FNUAPFloatToAPInt();

  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semFloat6E3M2FN)
    वापस convertFloat6E3M2FNAPFloatToAPInt();

  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semFloat6E2M3FN)
    वापस convertFloat6E2M3FNAPFloatToAPInt();

  अगर (semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semFloat4E2M1FN)
    वापस convertFloat4E2M1FNAPFloatToAPInt();

  llvm_unreachable("unknown format!");
}

भग्न IEEEFloat::convertToFloat() स्थिर {
  निश्चित(semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcastToAPInt();
  वापस api.bitsToFloat();
}

द्विगुन IEEEFloat::convertToDouble() स्थिर {
  निश्चित(semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcastToAPInt();
  वापस api.bitsToDouble();
}

#अगर_घोषित HAS_IEE754_FLOAT128
float128 IEEEFloat::convertToQuad() स्थिर {
  निश्चित(semantics == (स्थिर llvm::fltSemantics *)&APFloatBase::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcastToAPInt();
  वापस api.bitsToQuad();
}
#पूर्ण_अगर

व्योम IEEEFloat::initFromF80LongDoubleAPInt(स्थिर APInt &api) {
  वापस initFromIEEEAPInt<APFloatBase::semX87DoubleExtended>(api);
}

व्योम IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(स्थिर APInt &api) {
  uint64_t i1 = api.get_Raw_तथ्य()[0];
  uint64_t i2 = api.get_Raw_तथ्य()[1];
  bool losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_unused]] opStatus fs = convert(APFloatBase::semPPCDoubleDoubleLegacy,
                                         rmNearestTiesToEven, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  निश्चित((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  अगर (isFiniteNonZero()) {
    IEEEFloat v(APFloatBase::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    अगर (v.isFinite()) {
      fs = v.convert(APFloatBase::semPPCDoubleDoubleLegacy, rmNearestTiesToEven,
                     &losesInfo);
      निश्चित(fs == opOK && !losesInfo);

      add(v, rmNearestTiesToEven);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
व्योम IEEEFloat::initFromFloat8E8M0FNUAPInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E8M0FNU>(api);
}

व्योम IEEEFloat::initFromFloat8E5M3FNUAPInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M3FNU>(api);
}

ढाँचा <स्थिर fltSemantics &S>
व्योम IEEEFloat::initFromIEEEAPInt(स्थिर APInt &api) {
  निश्चित(api.getBitWidth() == S.आकार_In_Bits);

  constexpr अचिन्हित पूर्णांक trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr integerPart integer_bit =
      integerPart{1} << (trailing_significand_bits % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr अचिन्हित पूर्णांक exponent_bits =
      S.आकार_In_Bits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr अचिन्हित पूर्णांक stored_significand_parts =
      part_गिनती_For_Bits(trailing_significand_bits + 1);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr bool is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr पूर्णांक bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr bool has_significand = trailing_significand_bits > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  अगर constexpr (has_significand) {
    std::copy_n(api.get_Raw_तथ्य(), mysignificand.आकार_2(), mysignificand.begin());
    अगर constexpr (significand_mask != 0 || S.precision >= integerPartWidth) {
      mysignificand[mysignificand.आकार_2() - 1] &= significand_mask;
    }
  } अन्यथा {
    std::fill_n(mysignificand.begin(), mysignificand.आकार_2(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t last_word = api.get_Raw_तथ्य()[api.get_अंक_शब्दगण() - 1];
  uint64_t myexponent =
      (last_word >> (trailing_significand_bits % 64)) & exponent_mask;

  initialize(&S);
  निश्चित(part_गिनती() == mysignificand.आकार_2());

  sign = S.hasSignedRepr
             ? static_cast<अचिन्हित पूर्णांक>(last_word >> ((S.आकार_In_Bits - 1) % 64))
             : 0;

  bool all_zero_significand =
      has_significand && llvm::all_of(mysignificand, equal_to(0));

  bool is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  अगर constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    bool is_inf = false;

    अगर constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;
      स्थिर integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_mask_no_int_bit) == 0;
    } अन्यथा {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    अगर (is_inf) {
      makeInf(sign);
      वापस;
    }
  }

  bool is_nan = false;

  अगर constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    अगर constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      स्थिर integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;

      अगर (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_mask_no_int_bit) != 0) {
        // regular NaN and pseudoNaN
        is_nan = true;
      } अन्यथा अगर (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_mask_no_int_bit) == 0) {
        // pseudoinfinity
        is_nan = true;
      } अन्यथा अगर (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = true;
      }
    } अन्यथा {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } अन्यथा अगर constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    bool all_ones_significand =
        std::all_of(mysignificand.begin(), mysignificand.end() - 1,
                    [](integerPart bits) { वापस bits == ~integerPart{0}; }) &&
        (!significand_mask ||
         mysignificand[mysignificand.आकार_2() - 1] == significand_mask);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } अन्यथा अगर constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  अगर (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.आकार_2(),
                significandParts());
    वापस;
  }

  अगर (is_zero) {
    makeZero(sign);
    वापस;
  }

  category = fcNormal;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.आकार_2(), significandParts());
  अगर (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  अन्यथा {
    अगर constexpr (!S.hasExplicitIntegerBit) {
      significandParts()[mysignificand.आकार_2() - 1] |= integer_bit;
    }
  }
}

व्योम IEEEFloat::initFromQuadrupleAPInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEquad>(api);
}

व्योम IEEEFloat::initFromDoubleAPInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEdouble>(api);
}

व्योम IEEEFloat::initFromFloatAPInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEsingle>(api);
}

व्योम IEEEFloat::initFromBFloatAPInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semBFloat>(api);
}

व्योम IEEEFloat::initFromHalfAPInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEhalf>(api);
}

व्योम IEEEFloat::initFromFloat8E5M2APInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2>(api);
}

व्योम IEEEFloat::initFromFloat8E5M2FNUZAPInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2FNUZ>(api);
}

व्योम IEEEFloat::initFromFloat8E4M3APInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3>(api);
}

व्योम IEEEFloat::initFromFloat8E4M3FNAPInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FN>(api);
}

व्योम IEEEFloat::initFromFloat8E4M3FNUZAPInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FNUZ>(api);
}

व्योम IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3B11FNUZ>(api);
}

व्योम IEEEFloat::initFromFloat8E3M4APInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E3M4>(api);
}

व्योम IEEEFloat::initFromFloatTF32APInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloatTF32>(api);
}

व्योम IEEEFloat::initFromFloat6E3M2FNAPInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E3M2FN>(api);
}

व्योम IEEEFloat::initFromFloat6E2M3FNAPInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E2M3FN>(api);
}

व्योम IEEEFloat::initFromFloat4E2M1FNAPInt(स्थिर APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
व्योम IEEEFloat::initFromAPInt(स्थिर fltSemantics *Sem, स्थिर APInt &api) {
  निश्चित(api.getBitWidth() == Sem->आकार_In_Bits);
  अगर (Sem == &APFloatBase::semIEEEhalf)
    वापस initFromHalfAPInt(api);
  अगर (Sem == &APFloatBase::semBFloat)
    वापस initFromBFloatAPInt(api);
  अगर (Sem == &APFloatBase::semIEEEsingle)
    वापस initFromFloatAPInt(api);
  अगर (Sem == &APFloatBase::semIEEEdouble)
    वापस initFromDoubleAPInt(api);
  अगर (Sem == &APFloatBase::semIEEEquad)
    वापस initFromQuadrupleAPInt(api);
  अगर (Sem == &APFloatBase::semX87DoubleExtended)
    वापस initFromF80LongDoubleAPInt(api);
  अगर (Sem == &APFloatBase::semPPCDoubleDoubleLegacy)
    वापस initFromPPCDoubleDoubleLegacyAPInt(api);
  अगर (Sem == &APFloatBase::semFloat8E5M2)
    वापस initFromFloat8E5M2APInt(api);
  अगर (Sem == &APFloatBase::semFloat8E5M2FNUZ)
    वापस initFromFloat8E5M2FNUZAPInt(api);
  अगर (Sem == &APFloatBase::semFloat8E4M3)
    वापस initFromFloat8E4M3APInt(api);
  अगर (Sem == &APFloatBase::semFloat8E4M3FN)
    वापस initFromFloat8E4M3FNAPInt(api);
  अगर (Sem == &APFloatBase::semFloat8E4M3FNUZ)
    वापस initFromFloat8E4M3FNUZAPInt(api);
  अगर (Sem == &APFloatBase::semFloat8E4M3B11FNUZ)
    वापस initFromFloat8E4M3B11FNUZAPInt(api);
  अगर (Sem == &APFloatBase::semFloat8E3M4)
    वापस initFromFloat8E3M4APInt(api);
  अगर (Sem == &APFloatBase::semFloatTF32)
    वापस initFromFloatTF32APInt(api);
  अगर (Sem == &APFloatBase::semFloat8E8M0FNU)
    वापस initFromFloat8E8M0FNUAPInt(api);
  अगर (Sem == &APFloatBase::semFloat8E5M3FNU)
    वापस initFromFloat8E5M3FNUAPInt(api);
  अगर (Sem == &APFloatBase::semFloat6E3M2FN)
    वापस initFromFloat6E3M2FNAPInt(api);
  अगर (Sem == &APFloatBase::semFloat6E2M3FN)
    वापस initFromFloat6E2M3FNAPInt(api);
  अगर (Sem == &APFloatBase::semFloat4E2M1FN)
    वापस initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
व्योम IEEEFloat::makeLargest(bool Negative) {
  अगर (Negative && !semantics->hasSignedRepr)
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
  अचिन्हित Part_गिनती = part_गिनती();
  स_रखो(significand, 0xFF, माप(integerPart)*(Part_गिनती - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  स्थिर अचिन्हित अंक_Unused_ऊपरी_Bits =
    Part_गिनती*integerPartWidth - semantics->precision;
  significand[Part_गिनती - 1] = (अंक_Unused_ऊपरी_Bits < integerPartWidth)
                                   ? (~integerPart(0) >> अंक_Unused_ऊपरी_Bits)
                                   : 0;
  अगर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
व्योम IEEEFloat::makeSmallest(bool Negative) {
  अगर (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fcNormal;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tcSet(significandParts(), 1, part_गिनती());
}

व्योम IEEEFloat::makeSmallestNormalized(bool Negative) {
  अगर (Negative && !semantics->hasSignedRepr)
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

IEEEFloat::IEEEFloat(स्थिर fltSemantics &Sem, स्थिर APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(भग्न f) {
  initFromAPInt(&APFloatBase::semIEEEsingle, APInt::floatToBits(f));
}

IEEEFloat::IEEEFloat(द्विगुन d) {
  initFromAPInt(&APFloatBase::semIEEEdouble, APInt::doubleToBits(d));
}

namespace {
  व्योम append(SmallVectorImpl<अक्षर> &बफर, StringRef Str) {
    बफर.append(Str.begin(), Str.end());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  व्योम AdjustToPrecision(APInt &significand,
                         पूर्णांक &exp, अचिन्हित FormatPrecision) {
    अचिन्हित bits = significand.getActiveBits();

    // 196/59 is a very slight overestimate of lg_2(10).
    अचिन्हित bitsRequired = (FormatPrecision * 196 + 58) / 59;

    अगर (bits <= bitsRequired) वापस;

    अचिन्हित tensRemovable = (bits - bitsRequired) * 59 / 196;
    अगर (!tensRemovable) वापस;

    exp += tensRemovable;

    APInt भाजक(significand.getBitWidth(), 1);
    APInt powten(significand.getBitWidth(), 10);
    जबतक (true) {
      अगर (tensRemovable & 1)
        भाजक *= powten;
      tensRemovable >>= 1;
      अगर (!tensRemovable) अवरोध;
      powten *= powten;
    }

    significand = significand.udiv(भाजक);

    // Truncate the significand down to its active bit count.
    significand = significand.छाँटो(significand.getActiveBits());
  }


  व्योम AdjustToPrecision(SmallVectorImpl<अक्षर> &बफर_2,
                         पूर्णांक &exp, अचिन्हित FormatPrecision) {
    अचिन्हित N = बफर_2.आकार_2();
    अगर (N <= FormatPrecision) वापस;

    // The most significant figures are the last ones in the buffer.
    अचिन्हित FirstSignificant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    अगर (बफर_2[FirstSignificant - 1] < '5') {
      जबतक (FirstSignificant < N && बफर_2[FirstSignificant] == '0')
        FirstSignificant++;

      exp += FirstSignificant;
      बफर_2.erase(&बफर_2[0], &बफर_2[FirstSignificant]);
      वापस;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    क्रम (अचिन्हित I = FirstSignificant; I != N; ++I) {
      अगर (बफर_2[I] == '9') {
        FirstSignificant++;
      } अन्यथा {
        बफर_2[I]++;
        अवरोध;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    अगर (FirstSignificant == N) {
      exp += FirstSignificant;
      बफर_2.सफा();
      बफर_2.धकेलो_back('1');
      वापस;
    }

    exp += FirstSignificant;
    बफर_2.erase(&बफर_2[0], &बफर_2[FirstSignificant]);
  }

  व्योम toStringImpl(SmallVectorImpl<अक्षर> &Str, स्थिर bool isNeg, पूर्णांक exp,
                    APInt significand, अचिन्हित FormatPrecision,
                    अचिन्हित FormatMaxPadding, bool TruncateZero) {
    स्थिर पूर्णांक semanticsPrecision = significand.getBitWidth();

    अगर (isNeg)
      Str.धकेलो_back('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    अगर (!FormatPrecision) {
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
    अगर (exp == 0) {
      // Nothing to do.
    } अन्यथा अगर (exp > 0) {
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
      जबतक (true) {
        अगर (texp & 1)
          significand *= five_to_the_i;

        texp >>= 1;
        अगर (!texp)
          अवरोध;
        five_to_the_i *= five_to_the_i;
      }
    }

    AdjustToPrecision(significand, exp, FormatPrecision);

    SmallVector<अक्षर, 256> बफर_2;

    // Fill the buffer.
    अचिन्हित precision = significand.getBitWidth();
    अगर (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    bool inTrail = true;
    जबतक (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      अचिन्हित d = digit.get_ZExt_मूल्य();

      // Drop trailing zeros.
      अगर (inTrail && !d)
        exp++;
      अन्यथा {
        बफर_2.धकेलो_back((अक्षर) ('0' + d));
        inTrail = false;
      }
    }

    निश्चित(!बफर_2.empty() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    AdjustToPrecision(बफर_2, exp, FormatPrecision);

    अचिन्हित NDigits = बफर_2.आकार_2();

    // Check whether we should use scientific notation.
    bool FormatScientific;
    अगर (!FormatMaxPadding) {
      FormatScientific = true;
    } अन्यथा {
      अगर (exp >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((अचिन्हित) exp > FormatMaxPadding ||
                            NDigits + (अचिन्हित) exp > FormatPrecision);
      } अन्यथा {
        // Power of the most significant digit.
        पूर्णांक MSD = exp + (पूर्णांक) (NDigits - 1);
        अगर (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = false;
        } अन्यथा {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((अचिन्हित) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    अगर (FormatScientific) {
      exp += (NDigits - 1);

      Str.धकेलो_back(बफर_2[NDigits-1]);
      Str.धकेलो_back('.');
      अगर (NDigits == 1 && TruncateZero)
        Str.धकेलो_back('0');
      अन्यथा
        क्रम (अचिन्हित I = 1; I != NDigits; ++I)
          Str.धकेलो_back(बफर_2[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      अगर (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.धकेलो_back(TruncateZero ? 'E' : 'e');

      Str.धकेलो_back(exp >= 0 ? '+' : '-');
      अगर (exp < 0)
        exp = -exp;
      SmallVector<अक्षर, 6> expbuf;
      करो {
        expbuf.धकेलो_back((अक्षर) ('0' + (exp % 10)));
        exp /= 10;
      } जबतक (exp);
      // Exponent always at least two digits if we do not truncate zeros.
      अगर (!TruncateZero && expbuf.आकार_2() < 2)
        expbuf.धकेलो_back('0');
      क्रम (अचिन्हित I = 0, E = expbuf.आकार_2(); I != E; ++I)
        Str.धकेलो_back(expbuf[E-1-I]);
      वापस;
    }

    // Non-scientific, positive exponents.
    अगर (exp >= 0) {
      क्रम (अचिन्हित I = 0; I != NDigits; ++I)
        Str.धकेलो_back(बफर_2[NDigits-1-I]);
      क्रम (अचिन्हित I = 0; I != (अचिन्हित) exp; ++I)
        Str.धकेलो_back('0');
      वापस;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    पूर्णांक NWholeDigits = exp + (पूर्णांक) NDigits;

    अचिन्हित I = 0;
    अगर (NWholeDigits > 0) {
      क्रम (; I != (अचिन्हित) NWholeDigits; ++I)
        Str.धकेलो_back(बफर_2[NDigits-I-1]);
      Str.धकेलो_back('.');
    } अन्यथा {
      अचिन्हित NZeros = 1 + (अचिन्हित) -NWholeDigits;

      Str.धकेलो_back('0');
      Str.धकेलो_back('.');
      क्रम (अचिन्हित Z = 1; Z != NZeros; ++Z)
        Str.धकेलो_back('0');
    }

    क्रम (; I != NDigits; ++I)
      Str.धकेलो_back(बफर_2[NDigits-I-1]);

  }
} // namespace

व्योम IEEEFloat::toString(SmallVectorImpl<अक्षर> &Str, अचिन्हित FormatPrecision,
                         अचिन्हित FormatMaxPadding, bool TruncateZero) स्थिर {
  चयन (category) {
  हाल fcInfinity:
    अगर (isNegative())
      वापस append(Str, "-Inf");
    अन्यथा
      वापस append(Str, "+Inf");

  हाल fcNaN: वापस append(Str, "NaN");

  हाल fcZero:
    अगर (isNegative())
      Str.धकेलो_back('-');

    अगर (!FormatMaxPadding) {
      अगर (TruncateZero)
        append(Str, "0.0E+0");
      अन्यथा {
        append(Str, "0.0");
        अगर (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } अन्यथा {
      Str.धकेलो_back('0');
    }
    वापस;

  हाल fcNormal:
    अवरोध;
  }

  // Decompose the number into an APInt and an exponent.
  पूर्णांक exp = exponent - ((पूर्णांक) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_गिनती_For_Bits(semantics->precision)));

  toStringImpl(Str, isNegative(), exp, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

पूर्णांक IEEEFloat::getExactLog2Abs() स्थिर {
  अगर (!isFinite() || isZero())
    वापस पूर्णांक_न्यून;

  स्थिर integerPart *Parts = significandParts();
  स्थिर पूर्णांक Part_गिनती = part_गिनती_For_Bits(semantics->precision);

  पूर्णांक निकालो_गिनती = 0;
  क्रम (पूर्णांक i = 0; i < Part_गिनती; ++i) {
    निकालो_गिनती += llvm::popcount(Parts[i]);
    अगर (निकालो_गिनती > 1)
      वापस पूर्णांक_न्यून;
  }

  अगर (exponent != semantics->minExponent)
    वापस exponent;

  पूर्णांक CountrParts = 0;
  क्रम (पूर्णांक i = 0; i < Part_गिनती;
       ++i, CountrParts += APInt::APINT_BITS_PER_WORD) {
    अगर (Parts[i] != 0) {
      वापस exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

bool IEEEFloat::isSignaling() स्थिर {
  अगर (!isNaN())
    वापस false;
  अगर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    वापस false;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  वापस !APInt::tcExtractBit(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::opStatus IEEEFloat::अग्र(bool अग्र_Down) {
  // If we are performing nextDown, swap sign so we have -x.
  अगर (अग्र_Down)
    changeSign();

  // Compute nextUp(x)
  opStatus परिणाम_2 = opOK;

  // Handle each float category separately.
  चयन (category) {
  हाल fcInfinity:
    // nextUp(+inf) = +inf
    अगर (!isNegative())
      अवरोध;
    // nextUp(-inf) = -getLargest()
    makeLargest(true);
    अवरोध;
  हाल fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    अगर (isSignaling()) {
      परिणाम_2 = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(false, isNegative(), nullptr);
    }
    अवरोध;
  हाल fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(false);
    अवरोध;
  हाल fcNormal:
    // nextUp(-getSmallest()) = -0
    अगर (isSmallest() && isNegative()) {
      APInt::tcSet(significandParts(), 0, part_गिनती());
      category = fcZero;
      exponent = 0;
      अगर (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = false;
      अगर (!semantics->hasZero)
        makeSmallestNormalized(false);
      अवरोध;
    }

    अगर (isLargest() && !isNegative()) {
      अगर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        अवरोध;
      } अन्यथा अगर (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        अवरोध;
      } अन्यथा {
        // nextUp(getLargest()) == INFINITY
        APInt::tcSet(significandParts(), 0, part_गिनती());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        अवरोध;
      }
    }

    // nextUp(normal) == normal + inc.
    अगर (isNegative()) {
      // If we are negative, we need to decrement the significand.

      // We only cross a binade boundary that requires adjusting the exponent
      // if:
      //   1. exponent != semantics->minExponent. This implies we are not in the
      //   smallest binade or are dealing with denormals.
      //   2. Our significand excluding the integral bit is all zeros.
      bool WillCrossBinadeBoundary =
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
      APInt::tcDecrement(Parts, part_गिनती());

      अगर (WillCrossBinadeBoundary) {
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
      bool WillCrossBinadeBoundary = !APFloat::hasSignificand(*semantics) ||
                                     (!isDenormal() && isSignificandAllOnes());

      अगर (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tcSet(Parts, 0, part_गिनती());
        APInt::tcSetBit(Parts, semantics->precision - 1);
        निश्चित(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } अन्यथा {
        incrementSignificand();
      }
    }
    अवरोध;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  अगर (अग्र_Down)
    changeSign();

  वापस परिणाम_2;
}

APInt IEEEFloat::getNaNPayload() स्थिर {
  निश्चित(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  अचिन्हित Bits = semantics->precision - 1;
  वापस APInt(Bits, ArrayRef(significandParts(), part_गिनती_For_Bits(Bits)));
}

APFloatBase::ExponentType IEEEFloat::exponentNaN() स्थिर {
  वापस ::exponentNaN(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentInf() स्थिर {
  वापस ::exponentInf(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentZero() स्थिर {
  वापस ::exponentZero(*semantics);
}

व्योम IEEEFloat::makeInf(bool Negative) {
  अगर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  अगर (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(false, Negative);
    वापस;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tcSet(significandParts(), 0, part_गिनती());
}

व्योम IEEEFloat::makeZero(bool Negative) {
  अगर (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  अगर (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = false;
  }
  exponent = exponentZero();
  APInt::tcSet(significandParts(), 0, part_गिनती());
}

व्योम IEEEFloat::makeQuiet() {
  निश्चित(isNaN());
  अगर (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tcSetBit(significandParts(), semantics->precision - 2);
}

पूर्णांक ilogb(स्थिर IEEEFloat &Arg) {
  अगर (Arg.isNaN())
    वापस APFloat::IEK_NaN;
  अगर (Arg.isZero())
    वापस APFloat::IEK_Zero;
  अगर (Arg.isInfinity())
    वापस APFloat::IEK_Inf;
  अगर (!Arg.isDenormal())
    वापस Arg.exponent;

  IEEEFloat Normalized(Arg);
  पूर्णांक SignificandBits = Arg.getSemantics().precision - 1;

  Normalized.exponent += SignificandBits;
  Normalized.normalize(APFloat::rmNearestTiesToEven, lfExactlyZero);
  वापस Normalized.exponent - SignificandBits;
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
  अगर (X.isNaN())
    X.makeQuiet();
  वापस X;
}

IEEEFloat frexp(स्थिर IEEEFloat &Val, पूर्णांक &Exp, roundingMode RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  अगर (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    वापस Quiet;
  }

  अगर (Exp == APFloat::IEK_Inf)
    वापस Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  वापस scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(स्थिर fltSemantics &S)
    : Semantics(&S),
      Floats(नया APFloat[2]{APFloat(APFloatBase::semIEEEdouble),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(स्थिर fltSemantics &S, uninitializedTag)
    : Semantics(&S), Floats(नया APFloat[2]{
                         APFloat(APFloatBase::semIEEEdouble, uninitialized),
                         APFloat(APFloatBase::semIEEEdouble, uninitialized)}) {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(स्थिर fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(नया APFloat[2]{APFloat(APFloatBase::semIEEEdouble, I),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(स्थिर fltSemantics &S, स्थिर APInt &I)
    : Semantics(&S),
      Floats(नया APFloat[2]{
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.get_Raw_तथ्य()[0])),
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.get_Raw_तथ्य()[1]))}) {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(स्थिर fltSemantics &S, APFloat &&First,
                             APFloat &&Second)
    : Semantics(&S),
      Floats(नया APFloat[2]{std::move(First), std::move(Second)}) {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble);
  निश्चित(&Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  निश्चित(&Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(स्थिर DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? नया APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : nullptr) {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloatBase::semBogus;
  RHS.Floats = nullptr;
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::चालक=(स्थिर DoubleAPFloat &RHS) {
  अगर (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } अन्यथा अगर (यह != &RHS) {
    यह->~DoubleAPFloat();
    नया (यह) DoubleAPFloat(RHS);
  }
  वापस *यह;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
अटल std::pair<APFloat, APFloat> fast_Two_योग(APFloat X, APFloat Y) {
  अगर (!X.isFinite())
    वापस {X, APFloat::getZero(X.getSemantics(), /*Negative=*/false)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  वापस {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::opStatus DoubleAPFloat::addImpl(स्थिर APFloat &a, स्थिर APFloat &aa,
                                         स्थिर APFloat &c, स्थिर APFloat &cc,
                                         roundingMode RM) {
  पूर्णांक Status = opOK;
  APFloat z = a;
  Status |= z.add(c, RM);
  अगर (!z.isFinite()) {
    अगर (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ false);
      वापस (opStatus)Status;
    }
    Status = opOK;
    स्वतः AComparedToC = a.compare_Absolute_मूल्य(c);
    z = cc;
    Status |= z.add(aa, RM);
    अगर (AComparedToC == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      Status |= z.add(c, RM);
      Status |= z.add(a, RM);
    } अन्यथा {
      // z = cc + aa + a + c;
      Status |= z.add(a, RM);
      Status |= z.add(c, RM);
    }
    अगर (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ false);
      वापस (opStatus)Status;
    }
    Floats[0] = z;
    APFloat zz = aa;
    Status |= zz.add(cc, RM);
    अगर (AComparedToC == APFloat::cmpGreaterThan) {
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
    अगर (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ false);
      वापस opOK;
    }
    Floats[0] = z;
    Status |= Floats[0].add(zz, RM);
    अगर (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ false);
      वापस (opStatus)Status;
    }
    Floats[1] = std::move(z);
    Status |= Floats[1].subtract(Floats[0], RM);
    Status |= Floats[1].add(zz, RM);
  }
  वापस (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::addWithSpecial(स्थिर DoubleAPFloat &LHS,
                                                स्थिर DoubleAPFloat &RHS,
                                                DoubleAPFloat &Out,
                                                roundingMode RM) {
  अगर (LHS.getCategory() == fcNaN) {
    Out = LHS;
    वापस opOK;
  }
  अगर (RHS.getCategory() == fcNaN) {
    Out = RHS;
    वापस opOK;
  }
  अगर (LHS.getCategory() == fcZero) {
    Out = RHS;
    वापस opOK;
  }
  अगर (RHS.getCategory() == fcZero) {
    Out = LHS;
    वापस opOK;
  }
  अगर (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    Out.makeNaN(false, Out.isNegative(), nullptr);
    वापस opInvalidOp;
  }
  अगर (LHS.getCategory() == fcInfinity) {
    Out = LHS;
    वापस opOK;
  }
  अगर (RHS.getCategory() == fcInfinity) {
    Out = RHS;
    वापस opOK;
  }
  निश्चित(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  निश्चित(&A.getSemantics() == &APFloatBase::semIEEEdouble);
  निश्चित(&AA.getSemantics() == &APFloatBase::semIEEEdouble);
  निश्चित(&C.getSemantics() == &APFloatBase::semIEEEdouble);
  निश्चित(&CC.getSemantics() == &APFloatBase::semIEEEdouble);
  निश्चित(&Out.Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  निश्चित(&Out.Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
  वापस Out.addImpl(A, AA, C, CC, RM);
}

APFloat::opStatus DoubleAPFloat::add(स्थिर DoubleAPFloat &RHS,
                                     roundingMode RM) {
  वापस addWithSpecial(*यह, RHS, *यह, RM);
}

APFloat::opStatus DoubleAPFloat::subtract(स्थिर DoubleAPFloat &RHS,
                                          roundingMode RM) {
  changeSign();
  स्वतः Ret = add(RHS, RM);
  changeSign();
  वापस Ret;
}

APFloat::opStatus DoubleAPFloat::multiply(स्थिर DoubleAPFloat &RHS,
                                          APFloat::roundingMode RM) {
  स्थिर स्वतः &LHS = *यह;
  स्वतः &Out = *यह;
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
  अगर (LHS.getCategory() == fcNaN) {
    Out = LHS;
    वापस opOK;
  }
  अगर (RHS.getCategory() == fcNaN) {
    Out = RHS;
    वापस opOK;
  }
  अगर ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    Out.makeNaN(false, false, nullptr);
    वापस opOK;
  }
  अगर (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    Out = LHS;
    वापस opOK;
  }
  अगर (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    Out = RHS;
    वापस opOK;
  }
  निश्चित(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal &&
         "Special cases not handled exhaustively");

  पूर्णांक Status = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  Status |= T.multiply(C, RM);
  अगर (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ false);
    वापस (opStatus)Status;
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
  अगर (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ false);
  } अन्यथा {
    // Floats[1] = (t - u) + tau
    Status |= T.subtract(U, RM);
    Status |= T.add(Tau, RM);
    Floats[1] = std::move(T);
  }
  वापस (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::divide(स्थिर DoubleAPFloat &RHS,
                                        APFloat::roundingMode RM) {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  स्वतः Ret = Tmp.divide(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()), RM);
  *यह = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  वापस Ret;
}

APFloat::opStatus DoubleAPFloat::remainder(स्थिर DoubleAPFloat &RHS) {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  स्वतः Ret = Tmp.remainder(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *यह = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  वापस Ret;
}

APFloat::opStatus DoubleAPFloat::mod(स्थिर DoubleAPFloat &RHS) {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  स्वतः Ret = Tmp.mod(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *यह = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  वापस Ret;
}

APFloat::opStatus
DoubleAPFloat::fusedMultiplyAdd(स्थिर DoubleAPFloat &Multiplicand,
                                स्थिर DoubleAPFloat &Addend,
                                APFloat::roundingMode RM) {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  स्वतः Ret = Tmp.fusedMultiplyAdd(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcastToAPInt()),
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, Addend.bitcastToAPInt()),
      RM);
  *यह = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  वापस Ret;
}

APFloat::opStatus DoubleAPFloat::roundToIntegral(APFloat::roundingMode RM) {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  स्थिर APFloat &Hi = getFirst();
  स्थिर APFloat &Lo = getSecond();

  APFloat RoundedHi = Hi;
  स्थिर opStatus HiStatus = RoundedHi.roundToIntegral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  अगर (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/false);
    वापस HiStatus;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  स्वतः RoundToNearestHelper = [](APFloat ToRound, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    स्थिर APFloat RoundingError = Rounded - ToRound;
    अगर (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        असल(RoundingError).is_Exactly_मूल्य(0.5))
      Rounded.add(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rmNearestTiesToEven);
    वापस Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  अगर (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    अगर (RM == rmNearestTiesToAway || RM == rmNearestTiesToEven)
      RoundedHi = RoundToNearestHelper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/false);
    वापस HiStatus;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  RoundingMode LoRoundingMode;
  अगर (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    LoRoundingMode = isNegative() ? rmTowardPositive : rmTowardNegative;
  अन्यथा
    LoRoundingMode = RM;

  APFloat RoundedLo = Lo;
  स्थिर opStatus LoStatus = RoundedLo.roundToIntegral(LoRoundingMode);
  अगर (LoRoundingMode == rmNearestTiesToAway)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = RoundToNearestHelper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fast_Two_योग(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  वापस LoStatus;
}

व्योम DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmp_परिणाम
DoubleAPFloat::compare_Absolute_मूल्य(स्थिर DoubleAPFloat &RHS) स्थिर {
  // Compare absolute values of the high parts.
  स्थिर cmp_परिणाम HiPartCmp = Floats[0].compare_Absolute_मूल्य(RHS.Floats[0]);
  अगर (HiPartCmp != cmpEqual)
    वापस HiPartCmp;

  // Zero, regardless of sign, is equal.
  अगर (Floats[1].isZero() && RHS.Floats[1].isZero())
    वापस cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  स्थिर bool ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  स्थिर bool RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  अगर (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    वापस RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  अगर (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    वापस ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  अगर (ThisIsSubtractive != RHSIsSubtractive)
    वापस ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  स्थिर cmp_परिणाम LoPartCmp = Floats[1].compare_Absolute_मूल्य(RHS.Floats[1]);

  अगर (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    अगर (LoPartCmp == cmpLessThan)
      वापस cmpGreaterThan;
    अगर (LoPartCmp == cmpGreaterThan)
      वापस cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  वापस LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() स्थिर {
  वापस Floats[0].getCategory();
}

bool DoubleAPFloat::isNegative() स्थिर { वापस Floats[0].isNegative(); }

व्योम DoubleAPFloat::makeInf(bool Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ false);
}

व्योम DoubleAPFloat::makeZero(bool Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ false);
}

व्योम DoubleAPFloat::makeLargest(bool Neg) {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  अगर (Neg)
    changeSign();
}

व्योम DoubleAPFloat::makeSmallest(bool Neg) {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].makeSmallest(Neg);
  Floats[1].makeZero(/* Neg = */ false);
}

व्योम DoubleAPFloat::makeSmallestNormalized(bool Neg) {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  अगर (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ false);
}

व्योम DoubleAPFloat::makeNaN(bool SNaN, bool Neg, स्थिर APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ false);
}

APFloat::cmp_परिणाम DoubleAPFloat::compare(स्थिर DoubleAPFloat &RHS) स्थिर {
  स्वतः परिणाम = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  अगर (परिणाम == APFloat::cmpEqual)
    वापस Floats[1].compare(RHS.Floats[1]);
  वापस परिणाम;
}

bool DoubleAPFloat::bitwiseIsEqual(स्थिर DoubleAPFloat &RHS) स्थिर {
  वापस Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_code hash_मूल्य(स्थिर DoubleAPFloat &Arg) {
  अगर (Arg.Floats)
    वापस hash_combine(hash_मूल्य(Arg.Floats[0]), hash_मूल्य(Arg.Floats[1]));
  वापस hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcastToAPInt() स्थिर {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t तथ्य_2[] = {
      Floats[0].bitcastToAPInt().get_Raw_तथ्य()[0],
      Floats[1].bitcastToAPInt().get_Raw_तथ्य()[0],
  };
  वापस APInt(128, तथ्य_2);
}

Expected<APFloat::opStatus> DoubleAPFloat::convertFromString(StringRef S,
                                                             roundingMode RM) {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy);
  स्वतः Ret = Tmp.convertFromString(S, RM);
  *यह = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  वापस Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::opStatus DoubleAPFloat::अग्र(bool अग्र_Down) {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  अगर (अग्र_Down) {
    changeSign();
    APFloat::opStatus परिणाम = अग्र(/*nextDown=*/false);
    changeSign();
    वापस परिणाम;
  }
  चयन (getCategory()) {
  हाल fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    अगर (isNegative())
      makeLargest(true);
    वापस opOK;

  हाल fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    अगर (getFirst().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(false, isNegative(), nullptr);
      वापस opInvalidOp;
    }
    वापस opOK;

  हाल fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(false);
    वापस opOK;

  हाल fcNormal:
    अवरोध;
  }

  स्थिर APFloat &HiOld = getFirst();
  स्थिर APFloat &LoOld = getSecond();

  APFloat अग्र_Lo = LoOld;
  अग्र_Lo.अग्र(/*nextDown=*/false);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  स्वतः InLattice = [](स्थिर APFloat &Hi, स्थिर APFloat &Lo) {
    वापस Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  अगर (InLattice(HiOld, अग्र_Lo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(अग्र_Lo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat Largest{*Semantics, uninitialized};
    Largest.makeLargest(/*Neg=*/false);
    अगर (compare(Largest) == cmpGreaterThan)
      makeInf(/*Neg=*/false);

    वापस opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat अग्र_Hi = HiOld;
  अग्र_Hi.अग्र(/*nextDown=*/false);

  // nextUp(getLargest()) == INFINITY
  अगर (अग्र_Hi.isInfinity()) {
    makeInf(/*Neg=*/false);
    वापस opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  अगर (अग्र_Hi.isZero()) {
    makeZero(/*Neg=*/true);
    वापस opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  अग्र_Lo = neg(scalbn(harrisonUlp(अग्र_Hi), -1, rmTowardZero));
  अगर (!InLattice(अग्र_Hi, अग्र_Lo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    अग्र_Lo.अग्र(/*nextDown=*/false);

  Floats[0] = std::move(अग्र_Hi);
  Floats[1] = std::move(अग्र_Lo);

  वापस opOK;
}

APFloat::opStatus DoubleAPFloat::convertToSignExtendedInteger(
    MutableArrayRef<integerPart> निवेश, अचिन्हित पूर्णांक Width, bool IsSigned,
    roundingMode RM, bool *IsExact) स्थिर {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  अगर (!getFirst().isFiniteNonZero() || getSecond().isZero())
    वापस getFirst().convertToInteger(निवेश, Width, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = false;
  DoubleAPFloat Integral = *यह;
  स्थिर opStatus RoundStatus = Integral.roundToIntegral(RM);
  अगर (RoundStatus == opInvalidOp)
    वापस opInvalidOp;
  स्थिर APFloat &IntegralHi = Integral.getFirst();
  स्थिर APFloat &IntegralLo = Integral.getSecond();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  bool HiIsExact;
  अगर (IntegralHi.isZero() || IntegralLo.isZero()) {
    स्थिर opStatus HiStatus =
        IntegralHi.convertToInteger(निवेश, Width, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    वापस HiStatus == opOK ? opInexact : HiStatus;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  अगर (!IsSigned && IntegralHi.isNegative())
    वापस opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  bool LoIsExact;
  स्थिर पूर्णांक HiExactLog2 = IntegralHi.getExactLog2Abs();
  स्थिर अचिन्हित PositiveOverflowWidth = IsSigned ? Width - 1 : Width;
  अगर (HiExactLog2 >= 0 &&
      static_cast<अचिन्हित>(HiExactLog2) == PositiveOverflowWidth) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    अगर (IntegralHi.isNegative() == IntegralLo.isNegative())
      वापस opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    स्थिर opStatus LoStatus = IntegralLo.convertToInteger(
        निवेश, Width, /*IsSigned=*/true, RM, &LoIsExact);
    अगर (LoStatus == opInvalidOp)
      वापस opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    अगर (IsSigned && !IntegralHi.isNegative())
      APInt::tcClearBit(निवेश.तथ्य(), PositiveOverflowWidth);
    *IsExact = RoundStatus == opOK;
    वापस RoundStatus;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  स्थिर opStatus HiStatus = IntegralHi.convertToInteger(
      निवेश, Width, IsSigned, rmTowardZero, &HiIsExact);
  अगर (HiStatus == opInvalidOp)
    वापस HiStatus;

  // Convert Lo into a temporary integer of the same width.
  APSInt Lo_परिणाम{Width, /*isUnsigned=*/!IsSigned};
  स्थिर opStatus LoStatus =
      IntegralLo.convertToInteger(Lo_परिणाम, rmTowardZero, &LoIsExact);
  अगर (LoStatus == opInvalidOp)
    वापस LoStatus;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tcAdd(निवेश.तथ्य(), Lo_परिणाम.get_Raw_तथ्य(), /*carry=*/0, निवेश.आकार_2());

  *IsExact = RoundStatus == opOK;
  वापस RoundStatus;
}

APFloat::opStatus
DoubleAPFloat::convertToInteger(MutableArrayRef<integerPart> निवेश,
                                अचिन्हित पूर्णांक Width, bool IsSigned,
                                roundingMode RM, bool *IsExact) स्थिर {
  opStatus FS =
      convertToSignExtendedInteger(निवेश, Width, IsSigned, RM, IsExact);

  अगर (FS == opInvalidOp) {
    स्थिर अचिन्हित Dst_Parts_गिनती = part_गिनती_For_Bits(Width);
    निश्चित(Dst_Parts_गिनती <= निवेश.आकार_2() && "Integer too big");

    अचिन्हित Bits;
    अगर (getCategory() == fcNaN)
      Bits = 0;
    अन्यथा अगर (isNegative())
      Bits = IsSigned;
    अन्यथा
      Bits = Width - IsSigned;

    tcSetLeastSignificantBits(निवेश.तथ्य(), Dst_Parts_गिनती, Bits);
    अगर (isNegative() && IsSigned)
      APInt::tc_Shift_बायाँ(निवेश.तथ्य(), Dst_Parts_गिनती, Width - 1);
  }

  वापस FS;
}

APFloat::opStatus DoubleAPFloat::handleOverflow(roundingMode RM) {
  चयन (RM) {
  हाल APFloat::rmTowardZero:
    makeLargest(/*Neg=*/isNegative());
    अवरोध;
  हाल APFloat::rmTowardNegative:
    अगर (isNegative())
      makeInf(/*Neg=*/true);
    अन्यथा
      makeLargest(/*Neg=*/false);
    अवरोध;
  हाल APFloat::rmTowardPositive:
    अगर (isNegative())
      makeLargest(/*Neg=*/true);
    अन्यथा
      makeInf(/*Neg=*/false);
    अवरोध;
  हाल APFloat::rmNearestTiesToAway:
  हाल APFloat::rmNearestTiesToEven:
    makeInf(/*Neg=*/isNegative());
    अवरोध;
  शेष:
    llvm_unreachable("Invalid rounding mode found");
  }
  opStatus S = opInexact;
  अगर (!getFirst().isFinite())
    S = static_cast<opStatus>(S | opOverflow);
  वापस S;
}

APFloat::opStatus DoubleAPFloat::convertFromUnsignedParts(
    स्थिर integerPart *Src, अचिन्हित पूर्णांक Src_गिनती, roundingMode RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  स्थिर अचिन्हित SrcMSB = APInt::tcMSB(Src, Src_गिनती);
  अगर (SrcMSB == अच_पूर्णांक_उच्च) {
    // The source integer is 0.
    makeZero(/*Neg=*/false);
    वापस opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  स्थिर अचिन्हित SrcBitWidth = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/SrcBitWidth, ArrayRef(Src, Src_गिनती)},
                /*isUnsigned=*/true};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{getFirst().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/false, rmNearestTiesToEven);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  अगर (!Hi.isFinite())
    वापस handleOverflow(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  bool HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/true};
  Hi.convertToInteger(HiAsInt, rmNearestTiesToEven, &HiAsIntIsExact);
  स्थिर APInt Error = SrcInt.zext(HiAsInt.getBitWidth()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  roundingMode LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  अगर (Error.isNegative()) {
    अगर (RM == rmNearestTiesToAway) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      स्थिर अचिन्हित ErrorActiveBits = Error.getSignificantBits() - 1;
      स्थिर अचिन्हित LoPrecision = getSecond().getSemantics().precision;
      अगर (ErrorActiveBits > LoPrecision) {
        स्थिर अचिन्हित RoundingBoundary = ErrorActiveBits - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        अगर (Error.गिनती_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rmTowardPositive;
      }
    } अन्यथा अगर (RM == rmTowardZero) {
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
  std::tie(Hi, Lo) = fast_Two_योग(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  अगर (!getFirst().isFinite())
    वापस handleOverflow(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  अगर (getFirst().isFiniteNonZero() && Floats[0].isLargest()) {
    DoubleAPFloat Largest{*Semantics};
    Largest.makeLargest(/*Neg=*/false);
    अगर (compare(Largest) == APFloat::cmpGreaterThan)
      वापस handleOverflow(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  वापस Status;
}

APFloat::opStatus DoubleAPFloat::convertFromAPInt(स्थिर APInt &निवेश,
                                                  bool IsSigned,
                                                  roundingMode RM) {
  स्थिर bool Negate_निवेश = IsSigned && निवेश.isNegative();
  APInt API = निवेश;
  अगर (Negate_निवेश)
    API.negate();

  स्थिर APFloat::opStatus Status =
      convertFromUnsignedParts(API.get_Raw_तथ्य(), API.get_अंक_शब्दगण(), RM);
  अगर (Negate_निवेश)
    changeSign();
  वापस Status;
}

अचिन्हित पूर्णांक DoubleAPFloat::convertToHexString(अक्षर *DST,
                                               अचिन्हित पूर्णांक HexDigits,
                                               bool UpperCase,
                                               roundingMode RM) स्थिर {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  वापस APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .convertToHexString(DST, HexDigits, UpperCase, RM);
}

bool DoubleAPFloat::isDenormal() स्थिर {
  वापस getCategory() == fcNormal &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

bool DoubleAPFloat::isSmallest() स्थिर {
  अगर (getCategory() != fcNormal)
    वापस false;
  DoubleAPFloat Tmp(*यह);
  Tmp.makeSmallest(यह->isNegative());
  वापस Tmp.compare(*यह) == cmpEqual;
}

bool DoubleAPFloat::isSmallestNormalized() स्थिर {
  अगर (getCategory() != fcNormal)
    वापस false;

  DoubleAPFloat Tmp(*यह);
  Tmp.makeSmallestNormalized(यह->isNegative());
  वापस Tmp.compare(*यह) == cmpEqual;
}

bool DoubleAPFloat::isLargest() स्थिर {
  अगर (getCategory() != fcNormal)
    वापस false;
  DoubleAPFloat Tmp(*यह);
  Tmp.makeLargest(यह->isNegative());
  वापस Tmp.compare(*यह) == cmpEqual;
}

bool DoubleAPFloat::isInteger() स्थिर {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  वापस Floats[0].isInteger() && Floats[1].isInteger();
}

व्योम DoubleAPFloat::toString(SmallVectorImpl<अक्षर> &Str,
                             अचिन्हित FormatPrecision,
                             अचिन्हित FormatMaxPadding,
                             bool TruncateZero) स्थिर {
  निश्चित(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .toString(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

पूर्णांक DoubleAPFloat::getExactLog2Abs() स्थिर {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  अगर (getSecond().isNonZero())
    वापस पूर्णांक_न्यून;
  वापस getFirst().getExactLog2Abs();
}

पूर्णांक ilogb(स्थिर DoubleAPFloat &Arg) {
  स्थिर APFloat &Hi = Arg.getFirst();
  स्थिर APFloat &Lo = Arg.getSecond();
  पूर्णांक Ilogb_परिणाम = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  अगर (Arg.getCategory() != fcNormal)
    वापस Ilogb_परिणाम;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  अगर (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    वापस Ilogb_परिणाम;
  अगर (Hi.getExactLog2Abs() == पूर्णांक_न्यून)
    वापस Ilogb_परिणाम;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  वापस Ilogb_परिणाम - 1;
}

DoubleAPFloat scalbn(स्थिर DoubleAPFloat &Arg, पूर्णांक Exp,
                     APFloat::roundingMode RM) {
  निश्चित(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");
  वापस DoubleAPFloat(APFloatBase::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(स्थिर DoubleAPFloat &Arg, पूर्णांक &Exp,
                    APFloat::roundingMode RM) {
  निश्चित(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  अगर (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.getFirst() = Quiet.getFirst().makeQuiet();
    वापस Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  अगर (Exp == APFloat::IEK_Inf)
    वापस Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  अगर (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    वापस Arg;
  }

  स्थिर APFloat &Hi = Arg.getFirst();
  स्थिर APFloat &Lo = Arg.getSecond();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  स्थिर bool SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat Second = Lo;
  अगर (Arg.getCategory() == APFloat::fcNormal && Lo.isFiniteNonZero()) {
    roundingMode LoRoundingMode;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    अगर (RM == rmTowardZero)
      LoRoundingMode = Arg.isNegative() ? rmTowardPositive : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    अन्यथा अगर (RM == rmNearestTiesToAway && SignsDisagree && Exp > 0)
      LoRoundingMode = rmNearestTiesToEven;
    अन्यथा
      LoRoundingMode = RM;
    Second = scalbn(Lo, -Exp, LoRoundingMode);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    अगर (RM == rmNearestTiesToAway && LoRoundingMode == rmNearestTiesToEven) {
      // Re-scale the result back to check if rounding occurred.
      स्थिर APFloat RecomposedLo = scalbn(Second, Exp, rmNearestTiesToEven);
      अगर (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        स्थिर APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        स्थिर APFloat UlpOfSecond = harrisonUlp(Second);
        स्थिर APFloat ScaledUlpOfSecond =
            scalbn(UlpOfSecond, Exp - 1, rmNearestTiesToEven);
        स्थिर bool IsMidpoint = असल(RoundingError) == ScaledUlpOfSecond;
        स्थिर bool RoundedLoAway =
            Second.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        अगर (IsMidpoint && RoundedLoAway)
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
    अगर (Second.isZero() && SignsDisagree && Hi.getExactLog2Abs() != पूर्णांक_न्यून)
      ++Exp;
  }

  APFloat First = scalbn(Hi, -Exp, RM);
  वापस DoubleAPFloat(APFloatBase::PPCDoubleDouble(), std::move(First),
                       std::move(Second));
}

APInt DoubleAPFloat::getNaNPayload() स्थिर { वापस Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, स्थिर fltSemantics &Semantics) {
  अगर (usesLayout<IEEEFloat>(Semantics)) {
    नया (&IEEE) IEEEFloat(std::move(F));
    वापस;
  }
  अगर (usesLayout<DoubleAPFloat>(Semantics)) {
    स्थिर fltSemantics& S = F.getSemantics();
    नया (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloatBase::IEEEdouble()));
    वापस;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::opStatus> APFloat::convertFromString(StringRef Str,
                                                       roundingMode RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_code hash_मूल्य(स्थिर APFloat &Arg) {
  अगर (APFloat::usesLayout<detail::IEEEFloat>(Arg.getSemantics()))
    वापस hash_मूल्य(Arg.U.IEEE);
  अगर (APFloat::usesLayout<detail::DoubleAPFloat>(Arg.getSemantics()))
    वापस hash_मूल्य(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(स्थिर fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  स्वतः StatusOrErr = convertFromString(S, rmNearestTiesToEven);
  निश्चित(StatusOrErr && "Invalid floating point representation");
  consumeError(StatusOrErr.takeError());
}

FPClassTest APFloat::classify() स्थिर {
  अगर (isZero())
    वापस isNegative() ? fcNegZero : fcPosZero;
  अगर (isNormal())
    वापस isNegative() ? fcNegNormal : fcPosNormal;
  अगर (isDenormal())
    वापस isNegative() ? fcNegSubnormal : fcPosSubnormal;
  अगर (isInfinity())
    वापस isNegative() ? fcNegInf : fcPosInf;
  निश्चित(isNaN() && "Other class of FP constant");
  वापस isSignaling() ? fcSNan : fcQNan;
}

bool APFloat::getExactInverse(APFloat *Inv) स्थिर {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  अगर (!isFiniteNonZero())
    वापस false;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  अगर (isDenormal())
    वापस false;

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
  स्थिर पूर्णांक Exp = getExactLog2Abs();
  अगर (Exp == पूर्णांक_न्यून)
    वापस false;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  अगर (Reciprocal.getExactLog2Abs() != -Exp)
    वापस false;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  अगर (Reciprocal.isDenormal())
    वापस false;

  निश्चित(Reciprocal.isFiniteNonZero());

  अगर (Inv)
    *Inv = std::move(Reciprocal);

  वापस true;
}

APFloat::opStatus APFloat::convert(स्थिर fltSemantics &ToSemantics,
                                   roundingMode RM, bool *losesInfo) {
  अगर (&getSemantics() == &ToSemantics) {
    *losesInfo = false;
    वापस opOK;
  }
  अगर (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics))
    वापस U.IEEE.convert(ToSemantics, RM, losesInfo);
  अगर (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<DoubleAPFloat>(ToSemantics)) {
    निश्चित(&ToSemantics == &APFloatBase::semPPCDoubleDouble);
    स्वतः Ret =
        U.IEEE.convert(APFloatBase::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *यह = APFloat(ToSemantics, U.IEEE.bitcastToAPInt());
    वापस Ret;
  }
  अगर (usesLayout<DoubleAPFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics)) {
    स्वतः Ret = getIEEE().convert(ToSemantics, RM, losesInfo);
    *यह = APFloat(std::move(getIEEE()), ToSemantics);
    वापस Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_मूल्य(स्थिर fltSemantics &Semantics) {
  वापस APFloat(Semantics, APInt::getAllOnes(Semantics.आकार_In_Bits));
}

व्योम APFloat::print(raw_ostream &OS) स्थिर {
  SmallVector<अक्षर, 16> बफर;
  toString(बफर);
  OS << बफर;
}

#अगर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD व्योम APFloat::dump() स्थिर {
  print(dbgs());
  dbgs() << '\n';
}
#पूर्ण_अगर

व्योम APFloat::Profile(Folding_Set_गाँठ_ID &NID) स्थिर {
  NID.Add(bitcastToAPInt());
}

APFloat::opStatus APFloat::convertToInteger(APSInt &परिणाम_2,
                                            roundingMode rounding_mode,
                                            bool *isExact) स्थिर {
  अचिन्हित bitWidth = परिणाम_2.getBitWidth();
  SmallVector<uint64_t, 4> parts(परिणाम_2.get_अंक_शब्दगण());
  opStatus status = convertToInteger(parts, bitWidth, परिणाम_2.isSigned(),
                                     rounding_mode, isExact);
  // Keeps the original signed-ness.
  परिणाम_2 = APInt(bitWidth, parts);
  वापस status;
}

द्विगुन APFloat::convertToDouble() स्थिर {
  अगर (&getSemantics() == &APFloatBase::semIEEEdouble)
    वापस getIEEE().convertToDouble();
  निश्चित(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat अस्थायी = *यह;
  bool LosesInfo;
  [[maybe_unused]] opStatus St =
      अस्थायी.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &LosesInfo);
  निश्चित(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  वापस अस्थायी.getIEEE().convertToDouble();
}

#अगर_घोषित HAS_IEE754_FLOAT128
float128 APFloat::convertToQuad() स्थिर {
  अगर (&getSemantics() == &APFloatBase::semIEEEquad)
    वापस getIEEE().convertToQuad();
  निश्चित(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat अस्थायी = *यह;
  bool LosesInfo;
  [[maybe_unused]] opStatus St =
      अस्थायी.convert(APFloatBase::semIEEEquad, rmNearestTiesToEven, &LosesInfo);
  निश्चित(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  वापस अस्थायी.getIEEE().convertToQuad();
}
#पूर्ण_अगर

भग्न APFloat::convertToFloat() स्थिर {
  अगर (&getSemantics() == &APFloatBase::semIEEEsingle)
    वापस getIEEE().convertToFloat();
  निश्चित(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat अस्थायी = *यह;
  bool LosesInfo;
  [[maybe_unused]] opStatus St =
      अस्थायी.convert(APFloatBase::semIEEEsingle, rmNearestTiesToEven, &LosesInfo);
  निश्चित(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  वापस अस्थायी.getIEEE().convertToFloat();
}

अचिन्हित APFloatBase::get_Arbitrary_FPFormat_आकार_In_Bits(StringRef Format) {
  वापस StringSwitch<अचिन्हित>(Format)
      .Case("Float8E5M2", get_आकार_In_Bits(semFloat8E5M2))
      .Case("Float8E5M2FNUZ", get_आकार_In_Bits(semFloat8E5M2FNUZ))
      .Case("Float8E4M3", get_आकार_In_Bits(semFloat8E4M3))
      .Case("Float8E4M3FN", get_आकार_In_Bits(semFloat8E4M3FN))
      .Case("Float8E4M3FNUZ", get_आकार_In_Bits(semFloat8E4M3FNUZ))
      .Case("Float8E4M3B11FNUZ", get_आकार_In_Bits(semFloat8E4M3B11FNUZ))
      .Case("Float8E3M4", get_आकार_In_Bits(semFloat8E3M4))
      .Case("Float8E8M0FNU", get_आकार_In_Bits(semFloat8E8M0FNU))
      .Case("Float6E3M2FN", get_आकार_In_Bits(semFloat6E3M2FN))
      .Case("Float6E2M3FN", get_आकार_In_Bits(semFloat6E2M3FN))
      .Case("Float4E2M1FN", get_आकार_In_Bits(semFloat4E2M1FN))
      .Case("Float8E5M3FNU", get_आकार_In_Bits(semFloat8E5M3FNU))
      .Default(0);
}

bool APFloatBase::isValidArbitraryFPFormat(StringRef Format) {
  वापस get_Arbitrary_FPFormat_आकार_In_Bits(Format) != 0;
}

स्थिर fltSemantics *APFloatBase::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  वापस StringSwitch<स्थिर fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(nullptr);
}

APFloat::Storage::~Storage() {
  अगर (usesLayout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    वापस;
  }
  अगर (usesLayout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    वापस;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(स्थिर APFloat::Storage &RHS) {
  अगर (usesLayout<IEEEFloat>(*RHS.semantics)) {
    नया (यह) IEEEFloat(RHS.IEEE);
    वापस;
  }
  अगर (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    नया (यह) DoubleAPFloat(RHS.Double);
    वापस;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  अगर (usesLayout<IEEEFloat>(*RHS.semantics)) {
    नया (यह) IEEEFloat(std::move(RHS.IEEE));
    वापस;
  }
  अगर (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    नया (यह) DoubleAPFloat(std::move(RHS.Double));
    वापस;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::चालक=(स्थिर APFloat::Storage &RHS) {
  अगर (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } अन्यथा अगर (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } अन्यथा अगर (यह != &RHS) {
    यह->~Storage();
    नया (यह) Storage(RHS);
  }
  वापस *यह;
}

APFloat::Storage &APFloat::Storage::चालक=(APFloat::Storage &&RHS) {
  अगर (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } अन्यथा अगर (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } अन्यथा अगर (यह != &RHS) {
    यह->~Storage();
    नया (यह) Storage(std::move(RHS));
  }
  वापस *यह;
}

namespace {

APFloat::opStatus getOpStatusFromLibc(पूर्णांक libc_exceptions) {
  APFloat::opStatus status = APFloat::opOK;
  अगर (libc_exceptions & FE_INVALID)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInvalidOp);
  अगर (libc_exceptions & FE_DIVBYZERO)
    status = static_cast<APFloat::opStatus>(status | APFloat::opDivByZero);
  अगर (libc_exceptions & FE_OVERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opOverflow);
  अगर (libc_exceptions & FE_UNDERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opUnderflow);
  अगर (libc_exceptions & FE_INEXACT)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInexact);
  वापस status;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> exp(स्थिर APFloat &x, RoundingMode rounding_mode,
                           APFloat::opStatus *status) {

  अगर (rounding_mode == APFloatBase::rmNearestTiesToEven) {
    अगर (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEsingle) {
      भग्न x_val = x.convertToFloat();
      पूर्णांक exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      अगर (status) {
        *status = getOpStatusFromLibc(exc);
        अगर (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      भग्न परिणाम_2 = LIBC_NAMESPACE::shared::expf(x_val);
      वापस APFloat(परिणाम_2);
    }
    अगर (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEdouble) {
      द्विगुन x_val = x.convertToDouble();
      पूर्णांक exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      अगर (status) {
        *status = getOpStatusFromLibc(exc);
        अगर (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      द्विगुन परिणाम_2 = LIBC_NAMESPACE::shared::exp(x_val);
      वापस APFloat(परिणाम_2);
    }
  }
  वापस std::nullopt;
}

} // namespace llvm

#अघोषित APFLOAT_DISPATCH_ON_SEMANTICS
