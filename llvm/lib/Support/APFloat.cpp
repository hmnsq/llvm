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

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/APSInt.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/FloatingPointMode.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
#include <cstring>
#include <iterator>
#include <limits.h>

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

#include "shared/math.h"
#include "shared/math_check_exceptions.h"

#define APFLOAT_DISPATCH_ON_SEMANTICS(METHOD_CALL)                             \
  do {                                                                         \
    නම් (usesLayout<IEEEFloat>(getSemantics()))                                 \
      ආපසු U.IEEE.METHOD_CALL;                                               \
    නම් (usesLayout<DoubleAPFloat>(getSemantics()))                             \
      ආපසු U.Double.METHOD_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } අතරතුර (false)

using namespace llvm;

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

const llvm::fltSemantics &APFloatBase::EnumToSemantics(Semantics S) {
  switch (S) {
  pak_sa S_IEEEhalf:
    ආපසු IEEEhalf();
  pak_sa S_BFloat:
    ආපසු BFloat();
  pak_sa S_IEEEsingle:
    ආපසු IEEEsingle();
  pak_sa S_IEEEdouble:
    ආපසු IEEEdouble();
  pak_sa S_IEEEquad:
    ආපසු IEEEquad();
  pak_sa S_x87DoubleExtended:
    ආපසු x87DoubleExtended();
  pak_sa S_PPCDoubleDouble:
    ආපසු PPCDoubleDouble();
  pak_sa S_PPCDoubleDoubleLegacy:
    ආපසු PPCDoubleDoubleLegacy();
  pak_sa S_Float8E5M2:
    ආපසු Float8E5M2();
  pak_sa S_Float8E5M2FNUZ:
    ආපසු Float8E5M2FNUZ();
  pak_sa S_Float8E4M3:
    ආපසු Float8E4M3();
  pak_sa S_Float8E4M3FN:
    ආපසු Float8E4M3FN();
  pak_sa S_Float8E4M3FNUZ:
    ආපසු Float8E4M3FNUZ();
  pak_sa S_Float8E4M3B11FNUZ:
    ආපසු Float8E4M3B11FNUZ();
  pak_sa S_Float8E3M4:
    ආපසු Float8E3M4();
  pak_sa S_FloatTF32:
    ආපසු FloatTF32();
  pak_sa S_Float8E8M0FNU:
    ආපසු Float8E8M0FNU();
  pak_sa S_Float8E5M3FNU:
    ආපසු Float8E5M3FNU();
  pak_sa S_Float6E3M2FN:
    ආපසු Float6E3M2FN();
  pak_sa S_Float6E2M3FN:
    ආපසු Float6E2M3FN();
  pak_sa S_Float4E2M1FN:
    ආපසු Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloatBase::Semantics
APFloatBase::SemanticsToEnum(const llvm::fltSemantics &Sem) {
  නම් (&Sem == &llvm::APFloat::IEEEhalf())
    ආපසු S_IEEEhalf;
  නැතහොත් නම් (&Sem == &llvm::APFloat::BFloat())
    ආපසු S_BFloat;
  නැතහොත් නම් (&Sem == &llvm::APFloat::IEEEsingle())
    ආපසු S_IEEEsingle;
  නැතහොත් නම් (&Sem == &llvm::APFloat::IEEEdouble())
    ආපසු S_IEEEdouble;
  නැතහොත් නම් (&Sem == &llvm::APFloat::IEEEquad())
    ආපසු S_IEEEquad;
  නැතහොත් නම් (&Sem == &llvm::APFloat::x87DoubleExtended())
    ආපසු S_x87DoubleExtended;
  නැතහොත් නම් (&Sem == &llvm::APFloat::PPCDoubleDouble())
    ආපසු S_PPCDoubleDouble;
  නැතහොත් නම් (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    ආපසු S_PPCDoubleDoubleLegacy;
  නැතහොත් නම් (&Sem == &llvm::APFloat::Float8E5M2())
    ආපසු S_Float8E5M2;
  නැතහොත් නම් (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    ආපසු S_Float8E5M2FNUZ;
  නැතහොත් නම් (&Sem == &llvm::APFloat::Float8E4M3())
    ආපසු S_Float8E4M3;
  නැතහොත් නම් (&Sem == &llvm::APFloat::Float8E4M3FN())
    ආපසු S_Float8E4M3FN;
  නැතහොත් නම් (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    ආපසු S_Float8E4M3FNUZ;
  නැතහොත් නම් (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    ආපසු S_Float8E4M3B11FNUZ;
  නැතහොත් නම් (&Sem == &llvm::APFloat::Float8E3M4())
    ආපසු S_Float8E3M4;
  නැතහොත් නම් (&Sem == &llvm::APFloat::FloatTF32())
    ආපසු S_FloatTF32;
  නැතහොත් නම් (&Sem == &llvm::APFloat::Float8E8M0FNU())
    ආපසු S_Float8E8M0FNU;
  නැතහොත් නම් (&Sem == &llvm::APFloat::Float8E5M3FNU())
    ආපසු S_Float8E5M3FNU;
  නැතහොත් නම් (&Sem == &llvm::APFloat::Float6E3M2FN())
    ආපසු S_Float6E3M2FN;
  නැතහොත් නම් (&Sem == &llvm::APFloat::Float6E2M3FN())
    ආපසු S_Float6E2M3FN;
  නැතහොත් නම් (&Sem == &llvm::APFloat::Float4E2M1FN())
    ආපසු S_Float4E2M1FN;
  නැතහොත්
    llvm_unreachable("Unknown floating semantics");
}

bool APFloatBase::isRepresentableBy(const fltSemantics &A,
                                    const fltSemantics &B) {
  ආපසු A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

bool APFloatBase::isLosslesslyConvertibleTo(const fltSemantics &From,
                                            const fltSemantics &InitialTo,
                                            bool IgnoreNaNs) {
  නම් (&From == &InitialTo)
    ආපසු true;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  නම් (&From == &semPPCDoubleDouble)
    ආපසු false;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  const fltSemantics &To =
      &InitialTo == &semPPCDoubleDouble ? semIEEEdouble : InitialTo;

  නම් (!isRepresentableBy(From, To))
    ආපසු false;

  නම් ((From.hasZero && !To.hasZero) ||
      (From.hasSignedRepr && !To.hasSignedRepr))
    ආපසු false;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  bool FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  bool ToHasSignedZero = To.hasZero && To.hasSignedRepr &&
                         To.nanEncoding != fltNanEncoding::NegativeZero;
  නම් (FromHasSignedZero && !ToHasSignedZero)
    ආපසු false;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat SmallestFrom = APFloat::getSmallest(From);
  bool LosesInfo = false;
  (void)SmallestFrom.convert(To, APFloat::rmNearestTiesToEven, &LosesInfo);
  නම් (LosesInfo)
    ආපසු false;

  නම් (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    ආපසු true;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  නම් (IgnoreNaNs) {
    නම් (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      ආපසු To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    ආපසු To.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  නම් (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    ආපසු false;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  නම් (To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    ආපසු true;
  ආපසු To.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
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
static constexpr unsigned NumPow5PartsOnStack = 16;

static unsigned පූර්ණ_සංඛ්‍යා powerOfFivePartsForExponent(unsigned පූර්ණ_සංඛ්‍යා power) {
  assert(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  ආපසු 2 + ((power * 815) / (351 * APFloatBase::integerPartWidth));
}

unsigned පූර්ණ_සංඛ්‍යා APFloatBase::semanticsPrecision(const fltSemantics &semantics) {
  ආපසු semantics.precision;
}
APFloatBase::ExponentType
APFloatBase::semanticsMaxExponent(const fltSemantics &semantics) {
  ආපසු semantics.maxExponent;
}
APFloatBase::ExponentType
APFloatBase::semanticsMinExponent(const fltSemantics &semantics) {
  ආපසු semantics.minExponent;
}
unsigned පූර්ණ_සංඛ්‍යා APFloatBase::semanticsSizeInBits(const fltSemantics &semantics) {
  ආපසු semantics.sizeInBits;
}
unsigned පූර්ණ_සංඛ්‍යා APFloatBase::semanticsIntSizeInBits(const fltSemantics &semantics,
                                                 bool isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  unsigned පූර්ණ_සංඛ්‍යා MinBitWidth = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  නම් (isSigned)
    ++MinBitWidth;
  ආපසු MinBitWidth;
}

bool APFloatBase::semanticsHasZero(const fltSemantics &semantics) {
  ආපසු semantics.hasZero;
}

bool APFloatBase::semanticsHasSignedRepr(const fltSemantics &semantics) {
  ආපසු semantics.hasSignedRepr;
}

bool APFloatBase::semanticsHasInf(const fltSemantics &semantics) {
  ආපසු semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

bool APFloatBase::semanticsHasNaN(const fltSemantics &semantics) {
  ආපසු semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

bool APFloatBase::isIEEELikeFP(const fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  ආපසු SemanticsToEnum(semantics) <= S_IEEEquad;
}

bool APFloatBase::hasSignBitInMSB(const fltSemantics &semantics) {
  ආපසු semantics.hasSignBitInMSB;
}

bool APFloatBase::isRepresentableAsNormalIn(const fltSemantics &Src,
                                            const fltSemantics &Dst) {
  // Exponent range must be larger.
  නම් (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    ආපසු false;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  ආපසු Dst.precision >= Src.precision;
}

unsigned APFloatBase::getSizeInBits(const fltSemantics &Sem) {
  ආපසු Sem.sizeInBits;
}

static constexpr APFloatBase::ExponentType
exponentZero(const fltSemantics &semantics) {
  ආපසු semantics.minExponent - 1;
}

static constexpr APFloatBase::ExponentType
exponentInf(const fltSemantics &semantics) {
  ආපසු semantics.maxExponent + 1;
}

static constexpr APFloatBase::ExponentType
exponentNaN(const fltSemantics &semantics) {
  නම් (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    නම් (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      ආපසු exponentZero(semantics);
    නම් (semantics.hasSignedRepr || semantics.precision > 1)
      ආපසු semantics.maxExponent;
  }
  ආපසු semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

static anta_hstha Error createError(const Twine &Err) {
  ආපසු make_error<StringError>(Err, inconvertibleErrorCode());
}

static constexpr anta_hstha unsigned පූර්ණ_සංඛ්‍යා partCountForBits(unsigned පූර්ණ_සංඛ්‍යා bits) {
  ආපසු std::max(1u, (bits + APFloatBase::integerPartWidth - 1) /
                          APFloatBase::integerPartWidth);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
static anta_hstha unsigned පූර්ණ_සංඛ්‍යා
decDigitValue(unsigned පූර්ණ_සංඛ්‍යා c)
{
  ආපසු c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
static Expected<පූර්ණ_සංඛ්‍යා> readExponent(StringRef::iterator begin,
                                  StringRef::iterator end) {
  const unsigned පූර්ණ_සංඛ්‍යා overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  නම් (p == end || ((*p == '-' || *p == '+') && (p + 1) == end))
    ආපසු 0;

  bool isNegative = *p == '-';
  නම් (*p == '-' || *p == '+') {
    p++;
    නම් (p == end)
      ආපසු createError("Exponent has no digits");
  }

  unsigned absExponent = decDigitValue(*p++);
  නම් (absExponent >= 10U)
    ආපසු createError("Invalid character in exponent");

  සඳහා (; p != end; ++p) {
    unsigned value = decDigitValue(*p);
    නම් (value >= 10U)
      ආපසු createError("Invalid character in exponent");

    absExponent = absExponent * 10U + value;
    නම් (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      bha_nga;
    }
  }

  නම් (isNegative)
    ආපසු -(පූර්ණ_සංඛ්‍යා) absExponent;
  නැතහොත්
    ආපසු (පූර්ණ_සංඛ්‍යා) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
static Expected<පූර්ණ_සංඛ්‍යා> totalExponent(StringRef::iterator p,
                                   StringRef::iterator end,
                                   පූර්ණ_සංඛ්‍යා exponentAdjustment) {
  පූර්ණ_සංඛ්‍යා exponent = 0;

  නම් (p == end)
    ආපසු createError("Exponent has no digits");

  bool negative = *p == '-';
  නම් (*p == '-' || *p == '+') {
    p++;
    නම් (p == end)
      ආපසු createError("Exponent has no digits");
  }

  පූර්ණ_සංඛ්‍යා unsignedExponent = 0;
  bool overflow = false;
  සඳහා (; p != end; ++p) {
    unsigned පූර්ණ_සංඛ්‍යා value;

    value = decDigitValue(*p);
    නම් (value >= 10U)
      ආපසු createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + value;
    නම් (unsignedExponent > 32767) {
      overflow = true;
      bha_nga;
    }
  }

  නම් (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    overflow = true;

  නම් (!overflow) {
    exponent = unsignedExponent;
    නම් (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    නම් (exponent > 32767 || exponent < -32768)
      overflow = true;
  }

  නම් (overflow)
    exponent = negative ? -32768: 32767;

  ආපසු exponent;
}

static Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator end,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = end;
  අතරතුර (p != end && *p == '0')
    p++;

  නම් (p != end && *p == '.') {
    *dot = p++;

    නම් (end - begin == 1)
      ආපසු createError("Significand has no digits");

    අතරතුර (p != end && *p == '0')
      p++;
  }

  ආපසු p;
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
sa_nracanaa decimalInfo {
  const var_na *firstSigDigit;
  const var_na *lastSigDigit;
  පූර්ණ_සංඛ්‍යා exponent;
  පූර්ණ_සංඛ්‍යා normalizedExponent;
};

static Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator end, decimalInfo *D) {
  StringRef::iterator dot = end;

  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  නම් (!PtrOrErr)
    ආපසු PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->firstSigDigit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  සඳහා (; p != end; ++p) {
    නම් (*p == '.') {
      නම් (dot != end)
        ආපසු createError("String contains multiple dots");
      dot = p++;
      නම් (p == end)
        bha_nga;
    }
    නම් (decDigitValue(*p) >= 10U)
      bha_nga;
  }

  නම් (p != end) {
    නම් (*p != 'e' && *p != 'E')
      ආපසු createError("Invalid character in significand");
    නම් (p == begin)
      ආපසු createError("Significand has no digits");
    නම් (dot != end && p - begin == 1)
      ආපසු createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    auto ExpOrErr = readExponent(p + 1, end);
    නම් (!ExpOrErr)
      ආපසු ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    නම් (dot == end)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  නම් (p != D->firstSigDigit) {
    /* Drop insignificant trailing zeroes.  */
    නම් (p != begin) {
      do
        do
          p--;
        අතරතුර (p != begin && *p == '0');
      අතරතුර (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->firstSigDigit)
                                      - (dot > D->firstSigDigit && dot < p)));
  }

  D->lastSigDigit = p;
  ආපසු Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
static Expected<lostFraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator end,
                            unsigned පූර්ණ_සංඛ්‍යා digitValue) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  නම් (digitValue > 8)
    ආපසු lfMoreThanHalf;
  නැතහොත් නම් (digitValue < 8 && digitValue > 0)
    ආපසු lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  අතරතුර (p != end && (*p == '0' || *p == '.'))
    p++;

  නම් (p == end)
    ආපසු createError("Invalid trailing hexadecimal fraction!");

  unsigned hexDigit = hexDigitValue(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  නම් (hexDigit == UINT_MAX)
    ආපසු digitValue == 0 ? lfExactlyZero: lfExactlyHalf;
  නැතහොත්
    ආපසු digitValue == 0 ? lfLessThanHalf: lfMoreThanHalf;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
static lostFraction
lostFractionThroughTruncation(const APFloatBase::integerPart *parts,
                              unsigned පූර්ණ_සංඛ්‍යා partCount,
                              unsigned පූර්ණ_සංඛ්‍යා bits)
{
  unsigned lsb = APInt::tcLSB(parts, partCount);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  නම් (bits <= lsb)
    ආපසු lfExactlyZero;
  නම් (bits == lsb + 1)
    ආපසු lfExactlyHalf;
  නම් (bits <= partCount * APFloatBase::integerPartWidth &&
      APInt::tcExtractBit(parts, bits - 1))
    ආපසු lfMoreThanHalf;

  ආපසු lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
static lostFraction
shiftRight(APFloatBase::integerPart *dst, unsigned පූර්ණ_සංඛ්‍යා parts, unsigned පූර්ණ_සංඛ්‍යා bits)
{
  lostFraction lost_fraction = lostFractionThroughTruncation(dst, parts, bits);

  APInt::tcShiftRight(dst, parts, bits);

  ආපසු lost_fraction;
}

/* Combine the effect of two lost fractions.  */
static lostFraction
combineLostFractions(lostFraction moreSignificant,
                     lostFraction lessSignificant)
{
  නම් (lessSignificant != lfExactlyZero) {
    නම් (moreSignificant == lfExactlyZero)
      moreSignificant = lfLessThanHalf;
    නැතහොත් නම් (moreSignificant == lfExactlyHalf)
      moreSignificant = lfMoreThanHalf;
  }

  ආපසු moreSignificant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
static unsigned පූර්ණ_සංඛ්‍යා
HUerrBound(bool inexactMultiply, unsigned පූර්ණ_සංඛ්‍යා HUerr1, unsigned පූර්ණ_සංඛ්‍යා HUerr2)
{
  assert(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  නම් (HUerr1 + HUerr2 == 0)
    ආපසු inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  නැතහොත්
    ආපසු inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
static APFloatBase::integerPart
ulpsFromBoundary(const APFloatBase::integerPart *parts, unsigned පූර්ණ_සංඛ්‍යා bits,
                 bool isNearest) {
  assert(bits != 0);

  bits--;
  unsigned count = bits / APFloatBase::integerPartWidth;
  unsigned partBits = bits % APFloatBase::integerPartWidth + 1;

  APFloatBase::integerPart part =
      parts[count] & (~(APFloatBase::integerPart)0 >>
                      (APFloatBase::integerPartWidth - partBits));

  APFloatBase::integerPart boundary;
  නම් (isNearest)
    boundary = (APFloatBase::integerPart) 1 << (partBits - 1);
  නැතහොත්
    boundary = 0;

  නම් (count == 0) {
    නම් (part - boundary <= boundary - part)
      ආපසු part - boundary;
    නැතහොත්
      ආපසු boundary - part;
  }

  නම් (part == boundary) {
    අතරතුර (--count)
      නම් (parts[count])
        ආපසු ~(APFloatBase::integerPart) 0; /* A lot.  */

    ආපසු parts[0];
  } නැතහොත් නම් (part == boundary - 1) {
    අතරතුර (--count)
      නම් (~parts[count])
        ආපසු ~(APFloatBase::integerPart) 0; /* A lot.  */

    ආපසු -parts[0];
  }

  ආපසු ~(APFloatBase::integerPart) 0; /* A lot.  */
}

static constexpr unsigned PowerOfFivePartCounts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

static constexpr APFloatBase::integerPart PowerOfFiveParts[] = {
#include "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  unsigned Sum = 0;
  සඳහා (unsigned Count : PowerOfFivePartCounts)
    Sum += Count;
  ආපසු Sum;
}() == std::size(PowerOfFiveParts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
static unsigned පූර්ණ_සංඛ්‍යා
powerOf5(APFloatBase::integerPart *dst, unsigned පූර්ණ_සංඛ්‍යා power) {
  static const APFloatBase::integerPart firstEightPowers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  SmallVector<APFloatBase::integerPart, NumPow5PartsOnStack> scratch;
  scratch.resize_for_overwrite(powerOfFivePartsForExponent(power));

  APFloatBase::integerPart *p1, *p2;
  const APFloatBase::integerPart *pow5;

  p1 = dst;
  p2 = scratch.data();

  *p1 = firstEightPowers[power & 7];
  power >>= 3;

  unsigned result = 1;
  pow5 = PowerOfFiveParts;

  සඳහා (unsigned පූර්ණ_සංඛ්‍යා n = 0; power; power >>= 1, n++) {
    unsigned partsCount = PowerOfFivePartCounts[n];

    නම් (power & 1) {
      APFloatBase::integerPart *tmp;

      APInt::tcFullMultiply(p2, p1, pow5, result, partsCount);
      result += partsCount;
      නම් (p2[result - 1] == 0)
        result--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += partsCount;
  }

  නම් (p1 != dst)
    APInt::tcAssign(dst, p1, result);

  ආපසු result;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
static const var_na hexDigitsLower[] = "0123456789abcdef0";
static const var_na hexDigitsUpper[] = "0123456789ABCDEF0";
static const var_na infinityL[] = "infinity";
static const var_na infinityU[] = "INFINITY";
static const var_na NaNL[] = "nan";
static const var_na NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
static unsigned පූර්ණ_සංඛ්‍යා
partAsHex (var_na *dst, APFloatBase::integerPart part, unsigned පූර්ණ_සංඛ්‍යා count,
           const var_na *hexDigitChars)
{
  unsigned පූර්ණ_සංඛ්‍යා result = count;

  assert(count != 0 && count <= APFloatBase::integerPartWidth / 4);

  part >>= (APFloatBase::integerPartWidth - 4 * count);
  අතරතුර (count--) {
    dst[count] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  ආපසු result;
}

/* Write out an unsigned decimal integer.  */
static var_na *writeUnsignedDecimal(var_na *dst, unsigned පූර්ණ_සංඛ්‍යා n) {
  var_na buff[40], *p;

  p = buff;
  do
    *p++ = '0' + n % 10;
  අතරතුර (n /= 10);

  do
    *dst++ = *--p;
  අතරතුර (p != buff);

  ආපසු dst;
}

/* Write out a signed decimal integer.  */
static var_na *writeSignedDecimal(var_na *dst, පූර්ණ_සංඛ්‍යා value) {
  නම් (value < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(unsigned) value);
  } නැතහොත් {
    dst = writeUnsignedDecimal(dst, value);
  }

  ආපසු dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
static APFloat harrisonUlp(const APFloat &X) {
  const fltSemantics &Sem = X.getSemantics();
  switch (X.getCategory()) {
  pak_sa APFloat::fcNaN:
    ආපසු APFloat::getQNaN(Sem);
  pak_sa APFloat::fcInfinity:
    ආපසු APFloat::getInf(Sem);
  pak_sa APFloat::fcZero:
    ආපසු APFloat::getSmallest(Sem);
  pak_sa APFloat::fcNormal:
    bha_nga;
  }
  නම් (X.isDenormal() || X.isSmallestNormalized())
    ආපසු APFloat::getSmallest(Sem);
  පූර්ණ_සංඛ්‍යා Exp = ilogb(X);
  නම් (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  ආපසු scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rmNearestTiesToEven);
}

namespace detail {
/* Constructors.  */
void IEEEFloat::initialize(const fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  unsigned count = partCount();
  නම් (count > 1)
    significand.parts = new integerPart[count];
}

void IEEEFloat::freeSignificand() {
  නම් (needsCleanup())
    delete [] significand.parts;
}

void IEEEFloat::assign(const IEEEFloat &rhs) {
  assert(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  නම් (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

void IEEEFloat::copySignificand(const IEEEFloat &rhs) {
  assert(isFiniteNonZero() || category == fcNaN);
  assert(rhs.partCount() >= partCount());

  APInt::tcAssign(significandParts(), rhs.significandParts(),
                  partCount());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
void IEEEFloat::makeNaN(bool SNaN, bool Negative, const APInt *fill) {
  නම් (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  නම් (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  unsigned numParts = partCount();

  APInt fill_storage;
  නම් (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = false;
    නම් (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = true;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } නැතහොත් {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  නම් (!fill || fill->getNumWords() < numParts)
    APInt::tcSet(significand, 0, numParts);
  නම් (fill) {
    APInt::tcAssign(significand, fill->getRawData(),
                    std::min(fill->getNumWords(), numParts));

    // Zero out the excess bits of the significand.
    unsigned bitsToPreserve = semantics->precision - 1;
    unsigned part = bitsToPreserve / 64;
    bitsToPreserve %= 64;
    significand[part] &= ((1ULL << bitsToPreserve) - 1);
    සඳහා (part++; part != numParts; ++part)
      significand[part] = 0;
  }

  unsigned QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  නම් (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tcClearBit(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    නම් (APInt::tcIsZero(significand, numParts))
      APInt::tcSetBit(significand, QNaNBit - 1);
  } නැතහොත් නම් (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } නැතහොත් {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tcSetBit(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  නම් (semantics == &APFloatBase::semX87DoubleExtended)
    APInt::tcSetBit(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::operator=(const IEEEFloat &rhs) {
  නම් (this != &rhs) {
    නම් (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    assign(rhs);
  }

  ආපසු *this;
}

IEEEFloat &IEEEFloat::operator=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloatBase::semBogus;
  ආපසු *this;
}

bool IEEEFloat::isDenormal() const {
  ආපසු getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tcExtractBit(significandParts(), semantics->precision - 1) ==
          0);
}

bool IEEEFloat::isSmallest() const {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  ආපසු isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

bool IEEEFloat::isSmallestNormalized() const {
  ආපසු getCategory() == fcNormal && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

unsigned පූර්ණ_සංඛ්‍යා IEEEFloat::getNumHighBits() const {
  const unsigned පූර්ණ_සංඛ්‍යා PartCount = partCountForBits(semantics->precision);
  const unsigned පූර්ණ_සංඛ්‍යා Bits = PartCount * integerPartWidth;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  const unsigned පූර්ණ_සංඛ්‍යා NumHighBits = (semantics->precision > 1)
                                       ? (Bits - semantics->precision + 1)
                                       : (Bits - semantics->precision);
  ආපසු NumHighBits;
}

bool IEEEFloat::isSignificandAllOnes() const {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  const integerPart *Parts = significandParts();
  const unsigned PartCount = partCountForBits(semantics->precision);
  සඳහා (unsigned i = 0; i < PartCount - 1; i++)
    නම් (~Parts[i])
      ආපසු false;

  // Set the unused high bits to all ones when we compare.
  const unsigned NumHighBits = getNumHighBits();
  assert(NumHighBits <= integerPartWidth && NumHighBits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  const integerPart HighBitFill =
    ~integerPart(0) << (integerPartWidth - NumHighBits);
  නම් ((semantics->precision <= 1) || (~(Parts[PartCount - 1] | HighBitFill)))
    ආපසු false;

  ආපසු true;
}

bool IEEEFloat::isSignificandAllOnesExceptLSB() const {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  const integerPart *Parts = significandParts();

  නම් (Parts[0] & 1)
    ආපසු false;

  const unsigned PartCount = partCountForBits(semantics->precision);
  සඳහා (unsigned i = 0; i < PartCount - 1; i++) {
    නම් (~Parts[i] & ~unsigned{!i})
      ආපසු false;
  }

  // Set the unused high bits to all ones when we compare.
  const unsigned NumHighBits = getNumHighBits();
  assert(NumHighBits <= integerPartWidth && NumHighBits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  const integerPart HighBitFill = ~integerPart(0)
                                  << (integerPartWidth - NumHighBits);
  නම් (~(Parts[PartCount - 1] | HighBitFill | 0x1))
    ආපසු false;

  ආපසු true;
}

bool IEEEFloat::isSignificandAllZeros() const {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  const integerPart *Parts = significandParts();
  const unsigned PartCount = partCountForBits(semantics->precision);

  සඳහා (unsigned i = 0; i < PartCount - 1; i++)
    නම් (Parts[i])
      ආපසු false;

  // Compute how many bits are used in the final word.
  const unsigned NumHighBits = getNumHighBits();
  assert(NumHighBits < integerPartWidth && "Can not have more high bits to "
         "clear than integerPartWidth");
  const integerPart HighBitMask = ~integerPart(0) >> NumHighBits;

  නම් ((semantics->precision > 1) && (Parts[PartCount - 1] & HighBitMask))
    ආපසු false;

  ආපසු true;
}

bool IEEEFloat::isSignificandAllZerosExceptMSB() const {
  const integerPart *Parts = significandParts();
  const unsigned PartCount = partCountForBits(semantics->precision);

  සඳහා (unsigned i = 0; i < PartCount - 1; i++) {
    නම් (Parts[i])
      ආපසු false;
  }

  const unsigned NumHighBits = getNumHighBits();
  const integerPart MSBMask = integerPart(1)
                              << (integerPartWidth - NumHighBits);
  ආපසු ((semantics->precision <= 1) || (Parts[PartCount - 1] == MSBMask));
}

bool IEEEFloat::isLargest() const {
  bool IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  නම් (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    ආපසු (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } නැතහොත් {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    ආපසු IsMaxExp && isSignificandAllOnes();
  }
}

bool IEEEFloat::isInteger() const {
  // This could be made more efficient; I'm going for obviously correct.
  නම් (!isFinite()) ආපසු false;
  IEEEFloat truncated = *this;
  truncated.roundToIntegral(rmTowardZero);
  ආපසු compare(truncated) == cmpEqual;
}

bool IEEEFloat::bitwiseIsEqual(const IEEEFloat &rhs) const {
  නම් (this == &rhs)
    ආපසු true;
  නම් (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    ආපසු false;
  නම් (category==fcZero || category==fcInfinity)
    ආපසු true;

  නම් (isFiniteNonZero() && exponent != rhs.exponent)
    ආපසු false;

  ආපසු std::equal(significandParts(), significandParts() + partCount(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(const fltSemantics &ourSemantics, integerPart value) {
  initialize(&ourSemantics);
  sign = 0;
  category = fcNormal;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = value;
  normalize(rmNearestTiesToEven, lfExactlyZero);
}

IEEEFloat::IEEEFloat(const fltSemantics &ourSemantics) {
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
IEEEFloat::IEEEFloat(const fltSemantics &ourSemantics, uninitializedTag tag)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(const IEEEFloat &rhs) {
  initialize(rhs.semantics);
  assign(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloatBase::semBogus) {
  *this = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

unsigned පූර්ණ_සංඛ්‍යා IEEEFloat::partCount() const {
  ආපසු partCountForBits(semantics->precision + 1);
}

const APFloat::integerPart *IEEEFloat::significandParts() const {
  ආපසු const_cast<IEEEFloat *>(this)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  නම් (partCount() > 1)
    ආපසු significand.parts;
  නැතහොත්
    ආපසු &significand.part;
}

void IEEEFloat::zeroSignificand() {
  APInt::tcSet(significandParts(), 0, partCount());
}

/* Increment an fcNormal floating point number's significand.  */
void IEEEFloat::incrementSignificand() {
  [[maybe_unused]] integerPart carry =
      APInt::tcIncrement(significandParts(), partCount());

  /* Our callers should never cause us to overflow.  */
  assert(carry == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::addSignificand(const IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  assert(semantics == rhs.semantics);
  assert(exponent == rhs.exponent);

  ආපසු APInt::tcAdd(parts, rhs.significandParts(), 0, partCount());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::subtractSignificand(const IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  assert(semantics == rhs.semantics);
  assert(exponent == rhs.exponent);

  ආපසු APInt::tcSubtract(parts, rhs.significandParts(), borrow,
                           partCount());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
lostFraction IEEEFloat::multiplySignificand(const IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            bool ignoreAddend) {
  integerPart scratch[4];
  bool ignored;

  assert(semantics == rhs.semantics);

  unsigned precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  unsigned newPartsCount = partCountForBits(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *fullSignificand =
      newPartsCount > 4 ? new integerPart[newPartsCount] : scratch;

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

  නම් (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    const fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    unsigned extendedPrecision = 2 * precision + 1;
    නම් (omsb != extendedPrecision - 1) {
      assert(extendedPrecision > omsb);
      APInt::tcShiftLeft(fullSignificand, newPartsCount,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    නම් (newPartsCount == 1)
      significand.part = fullSignificand[0];
    නැතහොත්
      significand.parts = fullSignificand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_unused]] opStatus status = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    assert(status == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    lost_fraction = extendedAddend.shiftSignificandRight(1);
    assert(lost_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    lost_fraction = addOrSubtractSignificand(extendedAddend, false);

    /* Restore our state.  */
    නම් (newPartsCount == 1)
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
  නම් (omsb > precision) {
    unsigned පූර්ණ_සංඛ්‍යා bits, significantParts;
    lostFraction lf;

    bits = omsb - precision;
    significantParts = partCountForBits(omsb);
    lf = shiftRight(fullSignificand, significantParts, bits);
    lost_fraction = combineLostFractions(lf, lost_fraction);
    exponent += bits;
  }

  APInt::tcAssign(lhsSignificand, fullSignificand, partsCount);

  නම් (newPartsCount > 4)
    delete [] fullSignificand;

  ආපසු lost_fraction;
}

lostFraction IEEEFloat::multiplySignificand(const IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  ආපසු multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
lostFraction IEEEFloat::divideSignificand(const IEEEFloat &rhs) {
  integerPart scratch[4];

  assert(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  const integerPart *rhsSignificand = rhs.significandParts();
  unsigned partsCount = partCount();

  integerPart *dividend =
      partsCount > 2 ? new integerPart[partsCount * 2] : scratch;
  integerPart *divisor = dividend + partsCount;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  සඳහා (unsigned i = 0; i < partsCount; i++) {
    dividend[i] = lhsSignificand[i];
    divisor[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  unsigned පූර්ණ_සංඛ්‍යා precision = semantics->precision;

  /* Normalize the divisor.  */
  unsigned bit = precision - APInt::tcMSB(divisor, partsCount) - 1;
  නම් (bit) {
    exponent += bit;
    APInt::tcShiftLeft(divisor, partsCount, bit);
  }

  /* Normalize the dividend.  */
  bit = precision - APInt::tcMSB(dividend, partsCount) - 1;
  නම් (bit) {
    exponent -= bit;
    APInt::tcShiftLeft(dividend, partsCount, bit);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  නම් (APInt::tcCompare(dividend, divisor, partsCount) < 0) {
    exponent--;
    APInt::tcShiftLeft(dividend, partsCount, 1);
    assert(APInt::tcCompare(dividend, divisor, partsCount) >= 0);
  }

  /* Long division.  */
  සඳහා (bit = precision; bit; bit -= 1) {
    නම් (APInt::tcCompare(dividend, divisor, partsCount) >= 0) {
      APInt::tcSubtract(dividend, divisor, 0, partsCount);
      APInt::tcSetBit(lhsSignificand, bit - 1);
    }

    APInt::tcShiftLeft(dividend, partsCount, 1);
  }

  /* Figure out the lost fraction.  */
  පූර්ණ_සංඛ්‍යා cmp = APInt::tcCompare(dividend, divisor, partsCount);

  lostFraction lost_fraction;
  නම් (cmp > 0)
    lost_fraction = lfMoreThanHalf;
  නැතහොත් නම් (cmp == 0)
    lost_fraction = lfExactlyHalf;
  නැතහොත් නම් (APInt::tcIsZero(dividend, partsCount))
    lost_fraction = lfExactlyZero;
  නැතහොත්
    lost_fraction = lfLessThanHalf;

  නම් (partsCount > 2)
    delete [] dividend;

  ආපසු lost_fraction;
}

unsigned පූර්ණ_සංඛ්‍යා IEEEFloat::significandMSB() const {
  ආපසු APInt::tcMSB(significandParts(), partCount());
}

unsigned පූර්ණ_සංඛ්‍යා IEEEFloat::significandLSB() const {
  ආපසු APInt::tcLSB(significandParts(), partCount());
}

/* Note that a zero result is NOT normalized to fcZero.  */
lostFraction IEEEFloat::shiftSignificandRight(unsigned පූර්ණ_සංඛ්‍යා bits) {
  /* Our exponent should not overflow.  */
  assert((ExponentType) (exponent + bits) >= exponent);

  exponent += bits;

  ආපසු shiftRight(significandParts(), partCount(), bits);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
void IEEEFloat::shiftSignificandLeft(unsigned පූර්ණ_සංඛ්‍යා bits) {
  assert(bits < semantics->precision ||
         (semantics->precision == 1 && bits <= 1));

  නම් (bits) {
    unsigned පූර්ණ_සංඛ්‍යා partsCount = partCount();

    APInt::tcShiftLeft(significandParts(), partsCount, bits);
    exponent -= bits;

    assert(!APInt::tcIsZero(significandParts(), partsCount));
  }
}

APFloat::cmpResult IEEEFloat::compareAbsoluteValue(const IEEEFloat &rhs) const {
  assert(semantics == rhs.semantics);
  assert(isFiniteNonZero());
  assert(rhs.isFiniteNonZero());

  පූර්ණ_සංඛ්‍යා compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  නම් (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               partCount());

  නම් (compare > 0)
    ආපසු cmpGreaterThan;
  නැතහොත් නම් (compare < 0)
    ආපසු cmpLessThan;
  නැතහොත්
    ආපසු cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
static void tcSetLeastSignificantBits(APInt::WordType *dst, unsigned parts,
                                      unsigned bits) {
  unsigned i = 0;
  අතරතුර (bits > APInt::APINT_BITS_PER_WORD) {
    dst[i++] = ~(APInt::WordType)0;
    bits -= APInt::APINT_BITS_PER_WORD;
  }

  නම් (bits)
    dst[i++] = ~(APInt::WordType)0 >> (APInt::APINT_BITS_PER_WORD - bits);

  අතරතුර (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::opStatus IEEEFloat::handleOverflow(roundingMode rounding_mode) {
  නම් (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    නම් (rounding_mode == rmNearestTiesToEven ||
        rounding_mode == rmNearestTiesToAway ||
        (rounding_mode == rmTowardPositive && !sign) ||
        (rounding_mode == rmTowardNegative && sign)) {
      නම් (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(false, sign);
      නැතහොත්
        category = fcInfinity;
      ආපසු static_cast<opStatus>(opOverflow | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fcNormal;
  exponent = semantics->maxExponent;
  tcSetLeastSignificantBits(significandParts(), partCount(),
                            semantics->precision);
  නම් (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tcClearBit(significandParts(), 0);

  ආපසු opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
bool IEEEFloat::roundAwayFromZero(roundingMode rounding_mode,
                                  lostFraction lost_fraction,
                                  unsigned පූර්ණ_සංඛ්‍යා bit) const {
  /* NaNs and infinities should not have lost fractions.  */
  assert(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  assert(lost_fraction != lfExactlyZero);

  switch (rounding_mode) {
  pak_sa rmNearestTiesToAway:
    ආපසු lost_fraction == lfExactlyHalf || lost_fraction == lfMoreThanHalf;

  pak_sa rmNearestTiesToEven:
    නම් (lost_fraction == lfMoreThanHalf)
      ආපසු true;

    /* Our zeroes don't have a significand to test.  */
    නම් (lost_fraction == lfExactlyHalf && category != fcZero)
      ආපසු APInt::tcExtractBit(significandParts(), bit);

    ආපසු false;

  pak_sa rmTowardZero:
    ආපසු false;

  pak_sa rmTowardPositive:
    ආපසු !sign;

  pak_sa rmTowardNegative:
    ආපසු sign;

  default:
    bha_nga;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::opStatus IEEEFloat::normalize(roundingMode rounding_mode,
                                       lostFraction lost_fraction) {
  නම් (!isFiniteNonZero())
    ආපසු opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  unsigned omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  නම් (omsb || lost_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    පූර්ණ_සංඛ්‍යා exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    නම් (exponent + exponentChange > semantics->maxExponent)
      ආපසු handleOverflow(rounding_mode);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    නම් (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    නම් (exponentChange < 0) {
      assert(lost_fraction == lfExactlyZero);

      shiftSignificandLeft(-exponentChange);

      ආපසු opOK;
    }

    නම් (exponentChange > 0) {
      lostFraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = shiftSignificandRight(exponentChange);

      lost_fraction = combineLostFractions(lf, lost_fraction);

      /* Keep OMSB up-to-date.  */
      නම් (omsb > (unsigned) exponentChange)
        omsb -= exponentChange;
      නැතහොත්
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  නම් (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    ආපසු handleOverflow(rounding_mode);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  නම් (lost_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    නම් (omsb == 0) {
      category = fcZero;
      නම් (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = false;
      නම් (!semantics->hasZero)
        makeSmallestNormalized(false);
    }

    ආපසු opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  නම් (roundAwayFromZero(rounding_mode, lost_fraction, 0)) {
    නම් (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    නම් (omsb == (unsigned) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      නම් (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        ආපසු handleOverflow(sign ? rmTowardNegative : rmTowardPositive);

      shiftSignificandRight(1);

      ආපසු opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    නම් (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      ආපසු handleOverflow(rounding_mode);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  නම් (omsb == semantics->precision)
    ආපසු opInexact;

  /* We have a non-zero denormal.  */
  assert(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  නම් (omsb == 0) {
    category = fcZero;
    නම් (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = false;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    නම් (!semantics->hasZero)
      makeSmallestNormalized(false);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  ආපසු (opStatus) (opUnderflow | opInexact);
}

APFloat::opStatus IEEEFloat::addOrSubtractSpecials(const IEEEFloat &rhs,
                                                   bool subtract) {
  switch (PackCategoriesIntoKey(category, rhs.category)) {
  default:
    llvm_unreachable(nullptr);

  pak_sa PackCategoriesIntoKey(fcZero, fcNaN):
  pak_sa PackCategoriesIntoKey(fcNormal, fcNaN):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  pak_sa PackCategoriesIntoKey(fcNaN, fcZero):
  pak_sa PackCategoriesIntoKey(fcNaN, fcNormal):
  pak_sa PackCategoriesIntoKey(fcNaN, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcNaN, fcNaN):
    නම් (isSignaling()) {
      makeQuiet();
      ආපසු opInvalidOp;
    }
    ආපසු rhs.isSignaling() ? opInvalidOp : opOK;

  pak_sa PackCategoriesIntoKey(fcNormal, fcZero):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcNormal):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcZero):
    ආපසු opOK;

  pak_sa PackCategoriesIntoKey(fcNormal, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ subtract;
    ආපසු opOK;

  pak_sa PackCategoriesIntoKey(fcZero, fcNormal):
    assign(rhs);
    sign = rhs.sign ^ subtract;
    ආපසු opOK;

  pak_sa PackCategoriesIntoKey(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    ආපසු opOK;

  pak_sa PackCategoriesIntoKey(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    නම් (((sign ^ rhs.sign)!=0) != subtract) {
      makeNaN();
      ආපසු opInvalidOp;
    }

    ආපසු opOK;

  pak_sa PackCategoriesIntoKey(fcNormal, fcNormal):
    ආපසු opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
lostFraction IEEEFloat::addOrSubtractSignificand(const IEEEFloat &rhs,
                                                 bool subtract) {
  [[maybe_unused]] integerPart carry = 0;
  lostFraction lost_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  subtract ^= static_cast<bool>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  පූර්ණ_සංඛ්‍යා bits = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  නම් (subtract) {
    නම් ((bits < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat temp_rhs(rhs);
    bool lost_fraction_is_from_rhs = false;

    නම් (bits == 0)
      lost_fraction = lfExactlyZero;
    නැතහොත් නම් (bits > 0) {
      lost_fraction = temp_rhs.shiftSignificandRight(bits - 1);
      lost_fraction_is_from_rhs = true;
      shiftSignificandLeft(1);
    } නැතහොත් {
      lost_fraction = shiftSignificandRight(-bits - 1);
      temp_rhs.shiftSignificandLeft(1);
    }

    // Should we reverse the subtraction.
    cmpResult cmp_result = compareAbsoluteValue(temp_rhs);
    නම් (cmp_result == cmpLessThan) {
      bool borrow =
          lost_fraction != lfExactlyZero && !lost_fraction_is_from_rhs;
      නම් (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        නම් (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        නැතහොත් නම් (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = temp_rhs.subtractSignificand(*this, borrow);
      copySignificand(temp_rhs);
      sign = !sign;
    } නැතහොත් නම් (cmp_result == cmpGreaterThan) {
      bool borrow = lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs;
      නම් (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        නම් (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        නැතහොත් නම් (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = subtractSignificand(temp_rhs, borrow);
    } නැතහොත් { // cmpEqual
      zeroSignificand();
      නම් (lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    assert(!carry);
  } නැතහොත් {
    නම් (bits > 0) {
      IEEEFloat temp_rhs(rhs);

      lost_fraction = temp_rhs.shiftSignificandRight(bits);
      carry = addSignificand(temp_rhs);
    } නැතහොත් {
      lost_fraction = shiftSignificandRight(-bits);
      carry = addSignificand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    assert(!carry);
  }

  ආපසු lost_fraction;
}

APFloat::opStatus IEEEFloat::multiplySpecials(const IEEEFloat &rhs) {
  switch (PackCategoriesIntoKey(category, rhs.category)) {
  default:
    llvm_unreachable(nullptr);

  pak_sa PackCategoriesIntoKey(fcZero, fcNaN):
  pak_sa PackCategoriesIntoKey(fcNormal, fcNaN):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcNaN):
    assign(rhs);
    sign = false;
    [[fallthrough]];
  pak_sa PackCategoriesIntoKey(fcNaN, fcZero):
  pak_sa PackCategoriesIntoKey(fcNaN, fcNormal):
  pak_sa PackCategoriesIntoKey(fcNaN, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    නම් (isSignaling()) {
      makeQuiet();
      ආපසු opInvalidOp;
    }
    ආපසු rhs.isSignaling() ? opInvalidOp : opOK;

  pak_sa PackCategoriesIntoKey(fcNormal, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcNormal):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcInfinity):
    category = fcInfinity;
    ආපසු opOK;

  pak_sa PackCategoriesIntoKey(fcZero, fcNormal):
  pak_sa PackCategoriesIntoKey(fcNormal, fcZero):
  pak_sa PackCategoriesIntoKey(fcZero, fcZero):
    category = fcZero;
    ආපසු opOK;

  pak_sa PackCategoriesIntoKey(fcZero, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcZero):
    makeNaN();
    ආපසු opInvalidOp;

  pak_sa PackCategoriesIntoKey(fcNormal, fcNormal):
    ආපසු opOK;
  }
}

APFloat::opStatus IEEEFloat::divideSpecials(const IEEEFloat &rhs) {
  switch (PackCategoriesIntoKey(category, rhs.category)) {
  default:
    llvm_unreachable(nullptr);

  pak_sa PackCategoriesIntoKey(fcZero, fcNaN):
  pak_sa PackCategoriesIntoKey(fcNormal, fcNaN):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcNaN):
    assign(rhs);
    sign = false;
    [[fallthrough]];
  pak_sa PackCategoriesIntoKey(fcNaN, fcZero):
  pak_sa PackCategoriesIntoKey(fcNaN, fcNormal):
  pak_sa PackCategoriesIntoKey(fcNaN, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    නම් (isSignaling()) {
      makeQuiet();
      ආපසු opInvalidOp;
    }
    ආපසු rhs.isSignaling() ? opInvalidOp : opOK;

  pak_sa PackCategoriesIntoKey(fcInfinity, fcZero):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcNormal):
  pak_sa PackCategoriesIntoKey(fcZero, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcZero, fcNormal):
    ආපසු opOK;

  pak_sa PackCategoriesIntoKey(fcNormal, fcInfinity):
    category = fcZero;
    ආපසු opOK;

  pak_sa PackCategoriesIntoKey(fcNormal, fcZero):
    නම් (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(false, sign);
    නැතහොත්
      category = fcInfinity;
    ආපසු opDivByZero;

  pak_sa PackCategoriesIntoKey(fcInfinity, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcZero, fcZero):
    makeNaN();
    ආපසු opInvalidOp;

  pak_sa PackCategoriesIntoKey(fcNormal, fcNormal):
    ආපසු opOK;
  }
}

APFloat::opStatus IEEEFloat::modSpecials(const IEEEFloat &rhs) {
  switch (PackCategoriesIntoKey(category, rhs.category)) {
  default:
    llvm_unreachable(nullptr);

  pak_sa PackCategoriesIntoKey(fcZero, fcNaN):
  pak_sa PackCategoriesIntoKey(fcNormal, fcNaN):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  pak_sa PackCategoriesIntoKey(fcNaN, fcZero):
  pak_sa PackCategoriesIntoKey(fcNaN, fcNormal):
  pak_sa PackCategoriesIntoKey(fcNaN, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcNaN, fcNaN):
    නම් (isSignaling()) {
      makeQuiet();
      ආපසු opInvalidOp;
    }
    ආපසු rhs.isSignaling() ? opInvalidOp : opOK;

  pak_sa PackCategoriesIntoKey(fcZero, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcZero, fcNormal):
  pak_sa PackCategoriesIntoKey(fcNormal, fcInfinity):
    ආපසු opOK;

  pak_sa PackCategoriesIntoKey(fcNormal, fcZero):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcZero):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcNormal):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcZero, fcZero):
    makeNaN();
    ආපසු opInvalidOp;

  pak_sa PackCategoriesIntoKey(fcNormal, fcNormal):
    ආපසු opOK;
  }
}

APFloat::opStatus IEEEFloat::remainderSpecials(const IEEEFloat &rhs) {
  switch (PackCategoriesIntoKey(category, rhs.category)) {
  default:
    llvm_unreachable(nullptr);

  pak_sa PackCategoriesIntoKey(fcZero, fcNaN):
  pak_sa PackCategoriesIntoKey(fcNormal, fcNaN):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  pak_sa PackCategoriesIntoKey(fcNaN, fcZero):
  pak_sa PackCategoriesIntoKey(fcNaN, fcNormal):
  pak_sa PackCategoriesIntoKey(fcNaN, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcNaN, fcNaN):
    නම් (isSignaling()) {
      makeQuiet();
      ආපසු opInvalidOp;
    }
    ආපසු rhs.isSignaling() ? opInvalidOp : opOK;

  pak_sa PackCategoriesIntoKey(fcZero, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcZero, fcNormal):
  pak_sa PackCategoriesIntoKey(fcNormal, fcInfinity):
    ආපසු opOK;

  pak_sa PackCategoriesIntoKey(fcNormal, fcZero):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcZero):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcNormal):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcZero, fcZero):
    makeNaN();
    ආපසු opInvalidOp;

  pak_sa PackCategoriesIntoKey(fcNormal, fcNormal):
    ආපසු opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
void IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  නම් (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    ආපසු;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::opStatus IEEEFloat::addOrSubtract(const IEEEFloat &rhs,
                                           roundingMode rounding_mode,
                                           bool subtract) {
  opStatus fs = addOrSubtractSpecials(rhs, subtract);

  /* This return code means it was not a simple case.  */
  නම් (fs == opDivByZero) {
    lostFraction lost_fraction;

    lost_fraction = addOrSubtractSignificand(rhs, subtract);
    fs = normalize(rounding_mode, lost_fraction);

    /* Can only be zero if we lost no fraction.  */
    assert(category != fcZero || lost_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  නම් (category == fcZero) {
    නම් (rhs.category != fcZero || (sign == rhs.sign) == subtract)
      sign = (rounding_mode == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    නම් (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = false;
  }

  ආපසු fs;
}

/* Normalized addition.  */
APFloat::opStatus IEEEFloat::add(const IEEEFloat &rhs,
                                 roundingMode rounding_mode) {
  ආපසු addOrSubtract(rhs, rounding_mode, false);
}

/* Normalized subtraction.  */
APFloat::opStatus IEEEFloat::subtract(const IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  ආපසු addOrSubtract(rhs, rounding_mode, true);
}

/* Normalized multiply.  */
APFloat::opStatus IEEEFloat::multiply(const IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = multiplySpecials(rhs);

  නම් (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = false;
  නම් (isFiniteNonZero()) {
    lostFraction lost_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    නම් (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  ආපසු fs;
}

/* Normalized divide.  */
APFloat::opStatus IEEEFloat::divide(const IEEEFloat &rhs,
                                    roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = divideSpecials(rhs);

  නම් (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = false;
  නම් (isFiniteNonZero()) {
    lostFraction lost_fraction = divideSignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    නම් (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  ආපසු fs;
}

/* Normalized remainder.  */
APFloat::opStatus IEEEFloat::remainder(const IEEEFloat &rhs) {
  unsigned පූර්ණ_සංඛ්‍යා origSign = sign;

  // First handle the special cases.
  opStatus fs = remainderSpecials(rhs);
  නම් (fs != opDivByZero)
    ආපසු fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  නම් (P2.add(rhs, rmNearestTiesToEven) == opOK) {
    fs = mod(P2);
    assert(fs == opOK);
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

  IEEEFloat VEx = *this;
  fs = VEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  assert(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  assert(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.add(VEx, rmNearestTiesToEven);
  assert(fs == opOK);

  නම් (VEx.compare(PEx) == cmpGreaterThan) {
    fs = subtract(P, rmNearestTiesToEven);
    assert(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    assert(fs == opOK);
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    assert(fs == opOK);

    cmpResult result = VEx.compare(PEx);
    නම් (result == cmpGreaterThan || result == cmpEqual) {
      fs = subtract(P, rmNearestTiesToEven);
      assert(fs == opOK);
    }
  }

  නම් (isZero()) {
    sign = origSign;    // IEEE754 requires this
    නම් (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = false;
  } නැතහොත් {
    sign ^= origSign;
  }
  ආපසු fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::opStatus IEEEFloat::mod(const IEEEFloat &rhs) {
  opStatus fs = modSpecials(rhs);
  unsigned පූර්ණ_සංඛ්‍යා origSign = sign;

  අතරතුර (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compareAbsoluteValue(rhs) != cmpLessThan) {
    පූර්ණ_සංඛ්‍යා Exp = ilogb(*this) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rmNearestTiesToEven);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    නම් (V.isNaN() || compareAbsoluteValue(V) == cmpLessThan)
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
    නම් (!semantics->hasZero && this->isSmallest())
      bha_nga;

    assert(fs==opOK);
  }
  නම් (isZero()) {
    sign = origSign; // fmod requires this
    නම් (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = false;
  }
  ආපසු fs;
}

/* Normalized fused-multiply-add.  */
APFloat::opStatus IEEEFloat::fusedMultiplyAdd(const IEEEFloat &multiplicand,
                                              const IEEEFloat &addend,
                                              roundingMode rounding_mode) {
  opStatus fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  නම් (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    lostFraction lost_fraction;

    lost_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_mode, lost_fraction);
    නම් (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    නම් (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_mode == rmTowardNegative);
      නම් (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = false;
    }
  } නැතහොත් {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    නම් (fs == opOK)
      fs = addOrSubtract(addend, rounding_mode, false);
  }

  ආපසු fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::opStatus IEEEFloat::roundToIntegral(roundingMode rounding_mode) {
  නම් (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    ආපසු opOK;

  නම් (isNaN()) {
    නම් (isSignaling()) {
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
      ආපසු opInvalidOp;
    } නැතහොත් {
      // [IEEE Std 754-2008 6.2]:
      // For an operation with quiet NaN inputs, other than maximum and minimum
      // operations, if a floating-point result is to be delivered the result
      // shall be a quiet NaN which should be one of the input NaNs.
      // ...
      // Every general-computational and quiet-computational operation involving
      // one or more input NaNs, none of them signaling, shall signal no
      // exception, except fusedMultiplyAdd might signal the invalid operation
      // exception(see 7.2).
      ආපසු opOK;
    }
  }

  නම් (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    ආපසු opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  නම් (exponent + 1 >= (පූර්ණ_සංඛ්‍යා)APFloat::semanticsPrecision(*semantics))
    ආපසු opOK;

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
  opStatus fs = MagicConstant.convertFromAPInt(IntegerConstant, false,
                                               rmNearestTiesToEven);
  assert(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  bool inputSign = isNegative();

  fs = add(MagicConstant, rounding_mode);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  subtract(MagicConstant, rounding_mode);

  // Restore the input sign.
  නම් (inputSign != isNegative())
    changeSign();

  ආපසු fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmpResult IEEEFloat::compare(const IEEEFloat &rhs) const {
  assert(semantics == rhs.semantics);

  switch (PackCategoriesIntoKey(category, rhs.category)) {
  default:
    llvm_unreachable(nullptr);

  pak_sa PackCategoriesIntoKey(fcNaN, fcZero):
  pak_sa PackCategoriesIntoKey(fcNaN, fcNormal):
  pak_sa PackCategoriesIntoKey(fcNaN, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcNaN, fcNaN):
  pak_sa PackCategoriesIntoKey(fcZero, fcNaN):
  pak_sa PackCategoriesIntoKey(fcNormal, fcNaN):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcNaN):
    ආපසු cmpUnordered;

  pak_sa PackCategoriesIntoKey(fcInfinity, fcNormal):
  pak_sa PackCategoriesIntoKey(fcInfinity, fcZero):
  pak_sa PackCategoriesIntoKey(fcNormal, fcZero):
    නම් (sign)
      ආපසු cmpLessThan;
    නැතහොත්
      ආපසු cmpGreaterThan;

  pak_sa PackCategoriesIntoKey(fcNormal, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcZero, fcInfinity):
  pak_sa PackCategoriesIntoKey(fcZero, fcNormal):
    නම් (rhs.sign)
      ආපසු cmpGreaterThan;
    නැතහොත්
      ආපසු cmpLessThan;

  pak_sa PackCategoriesIntoKey(fcInfinity, fcInfinity):
    නම් (sign == rhs.sign)
      ආපසු cmpEqual;
    නැතහොත් නම් (sign)
      ආපසු cmpLessThan;
    නැතහොත්
      ආපසු cmpGreaterThan;

  pak_sa PackCategoriesIntoKey(fcZero, fcZero):
    ආපසු cmpEqual;

  pak_sa PackCategoriesIntoKey(fcNormal, fcNormal):
    bha_nga;
  }

  cmpResult result;
  /* Two normal numbers.  Do they have the same sign?  */
  නම් (sign != rhs.sign) {
    නම් (sign)
      result = cmpLessThan;
    නැතහොත්
      result = cmpGreaterThan;
  } නැතහොත් {
    /* Compare absolute values; invert result if negative.  */
    result = compareAbsoluteValue(rhs);

    නම් (sign) {
      නම් (result == cmpLessThan)
        result = cmpGreaterThan;
      නැතහොත් නම් (result == cmpGreaterThan)
        result = cmpLessThan;
    }
  }

  ආපසු result;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::opStatus IEEEFloat::convert(const fltSemantics &toSemantics,
                                     roundingMode rounding_mode,
                                     bool *losesInfo) {
  opStatus fs;
  const fltSemantics &fromSemantics = *semantics;
  bool is_signaling = isSignaling();

  lostFraction lostFraction = lfExactlyZero;
  unsigned newPartCount = partCountForBits(toSemantics.precision + 1);
  unsigned oldPartCount = partCount();
  පූර්ණ_සංඛ්‍යා shift = toSemantics.precision - fromSemantics.precision;

  bool X86SpecialNan = false;
  නම් (&fromSemantics == &APFloatBase::semX87DoubleExtended &&
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
  නම් (shift < 0 && isFiniteNonZero()) {
    පූර්ණ_සංඛ්‍යා omsb = significandMSB() + 1;
    පූර්ණ_සංඛ්‍යා exponentChange = omsb - fromSemantics.precision;
    නම් (exponent + exponentChange < toSemantics.minExponent)
      exponentChange = toSemantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, shift);
    නම් (exponentChange < 0) {
      shift -= exponentChange;
      exponent += exponentChange;
    } නැතහොත් නම් (omsb <= -shift) {
      exponentChange = omsb + shift - 1; // leave at least one bit set
      shift -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  නම් (shift < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    lostFraction = shiftRight(significandParts(), oldPartCount, -shift);

  // Fix the storage so it can hold to new value.
  නම් (newPartCount > oldPartCount) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = new integerPart[newPartCount];
    APInt::tcSet(newParts, 0, newPartCount);
    නම් (isFiniteNonZero() || category==fcNaN)
      APInt::tcAssign(newParts, significandParts(), oldPartCount);
    freeSignificand();
    significand.parts = newParts;
  } නැතහොත් නම් (newPartCount == 1 && oldPartCount != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    නම් (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &toSemantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  නම් (shift > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tcShiftLeft(significandParts(), newPartCount, shift);

  නම් (isFiniteNonZero()) {
    fs = normalize(rounding_mode, lostFraction);
    *losesInfo = (fs != opOK);
  } නැතහොත් නම් (category == fcNaN) {
    නම් (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(false, sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } නැතහොත් {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      නම් (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(false, false);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      නම් (!APFloat::hasSignificand(fromSemantics))
        makeNaN(false, sign);

      *losesInfo = lostFraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      නම් (!X86SpecialNan && semantics == &APFloatBase::semX87DoubleExtended)
        APInt::tcSetBit(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      නම් (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } නැතහොත් {
        fs = opOK;
      }
    }
  } නැතහොත් නම් (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(false, sign);
    *losesInfo = true;
    fs = opInexact;
  } නැතහොත් නම් (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = false;
  } නැතහොත් {
    *losesInfo = false;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  නම් ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = true;
    නම් (fs == opOK)
      fs = opInexact;
  }

  නම් (category == fcZero && !semantics->hasZero)
    makeSmallestNormalized(false);
  ආපසු fs;
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
    MutableArrayRef<integerPart> parts, unsigned පූර්ණ_සංඛ්‍යා width, bool isSigned,
    roundingMode rounding_mode, bool *isExact) const {
  *isExact = false;

  /* Handle the three special cases first.  */
  නම් (category == fcInfinity || category == fcNaN)
    ආපසු opInvalidOp;

  unsigned dstPartsCount = partCountForBits(width);
  assert(dstPartsCount <= parts.size() && "Integer too big");

  නම් (category == fcZero) {
    APInt::tcSet(parts.data(), 0, dstPartsCount);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    ආපසු opOK;
  }

  const integerPart *src = significandParts();

  unsigned truncatedBits;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  නම් (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tcSet(parts.data(), 0, dstPartsCount);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncatedBits = semantics->precision -1U - exponent;
  } නැතහොත් {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    unsigned පූර්ණ_සංඛ්‍යා bits = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    නම් (bits > width)
      ආපසු opInvalidOp;

    නම් (bits < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncatedBits = semantics->precision - bits;
      APInt::tcExtract(parts.data(), dstPartsCount, src, bits, truncatedBits);
    } නැතහොත් {
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
  නම් (truncatedBits) {
    lost_fraction = lostFractionThroughTruncation(src, partCount(),
                                                  truncatedBits);
    නම් (lost_fraction != lfExactlyZero &&
        roundAwayFromZero(rounding_mode, lost_fraction, truncatedBits)) {
      නම් (APInt::tcIncrement(parts.data(), dstPartsCount))
        ආපසු opInvalidOp;     /* Overflow.  */
    }
  } නැතහොත් {
    lost_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  unsigned පූර්ණ_සංඛ්‍යා omsb = APInt::tcMSB(parts.data(), dstPartsCount) + 1;

  නම් (sign) {
    නම් (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      නම් (omsb != 0)
        ආපසු opInvalidOp;
    } නැතහොත් {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      නම් (omsb == width &&
          APInt::tcLSB(parts.data(), dstPartsCount) + 1 != omsb)
        ආපසු opInvalidOp;

      /* This case can happen because of rounding.  */
      නම් (omsb > width)
        ආපසු opInvalidOp;
    }

    APInt::tcNegate (parts.data(), dstPartsCount);
  } නැතහොත් {
    නම් (omsb >= width + !isSigned)
      ආපසු opInvalidOp;
  }

  නම් (lost_fraction == lfExactlyZero) {
    *isExact = true;
    ආපසු opOK;
  }
  ආපසු opInexact;
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
                            unsigned පූර්ණ_සංඛ්‍යා width, bool isSigned,
                            roundingMode rounding_mode, bool *isExact) const {
  opStatus fs = convertToSignExtendedInteger(parts, width, isSigned,
                                             rounding_mode, isExact);

  නම් (fs == opInvalidOp) {
    unsigned පූර්ණ_සංඛ්‍යා bits, dstPartsCount;

    dstPartsCount = partCountForBits(width);
    assert(dstPartsCount <= parts.size() && "Integer too big");

    නම් (category == fcNaN)
      bits = 0;
    නැතහොත් නම් (sign)
      bits = isSigned;
    නැතහොත්
      bits = width - isSigned;

    tcSetLeastSignificantBits(parts.data(), dstPartsCount, bits);
    නම් (sign && isSigned)
      APInt::tcShiftLeft(parts.data(), dstPartsCount, width - 1);
  }

  ආපසු fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::opStatus IEEEFloat::convertFromUnsignedParts(
    const integerPart *src, unsigned පූර්ණ_සංඛ්‍යා srcCount, roundingMode rounding_mode) {
  category = fcNormal;
  unsigned omsb = APInt::tcMSB(src, srcCount) + 1;
  integerPart *dst = significandParts();
  unsigned dstCount = partCount();
  unsigned precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  lostFraction lost_fraction;
  නම් (precision <= omsb) {
    exponent = omsb - 1;
    lost_fraction = lostFractionThroughTruncation(src, srcCount,
                                                  omsb - precision);
    APInt::tcExtract(dst, dstCount, src, precision, omsb - precision);
  } නැතහොත් {
    exponent = precision - 1;
    lost_fraction = lfExactlyZero;
    APInt::tcExtract(dst, dstCount, src, omsb, 0);
  }

  ආපසු normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus IEEEFloat::convertFromAPInt(const APInt &Val, bool isSigned,
                                              roundingMode rounding_mode) {
  unsigned පූර්ණ_සංඛ්‍යා partCount = Val.getNumWords();
  APInt api = Val;

  sign = false;
  නම් (isSigned && api.isNegative()) {
    sign = true;
    api = -api;
  }

  ආපසු convertFromUnsignedParts(api.getRawData(), partCount, rounding_mode);
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
  bool computedTrailingFraction = false;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator end = s.end();
  StringRef::iterator dot;
  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  නම් (!PtrOrErr)
    ආපසු PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator firstSignificantDigit = p;

  අතරතුර (p != end) {
    integerPart hex_value;

    නම් (*p == '.') {
      නම් (dot != end)
        ආපසු createError("String contains multiple dots");
      dot = p++;
      continue;
    }

    hex_value = hexDigitValue(*p);
    නම් (hex_value == UINT_MAX)
      bha_nga;

    p++;

    // Store the number while we have space.
    නම් (bitPos) {
      bitPos -= 4;
      hex_value <<= bitPos % integerPartWidth;
      significand[bitPos / integerPartWidth] |= hex_value;
    } නැතහොත් නම් (!computedTrailingFraction) {
      auto FractOrErr = trailingHexadecimalFraction(p, end, hex_value);
      නම් (!FractOrErr)
        ආපසු FractOrErr.takeError();
      lost_fraction = *FractOrErr;
      computedTrailingFraction = true;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  නම් (p == end)
    ආපසු createError("Hex strings require an exponent");
  නම් (*p != 'p' && *p != 'P')
    ආපසු createError("Invalid character in significand");
  නම් (p == begin)
    ආපසු createError("Significand has no digits");
  නම් (dot != end && p - begin == 1)
    ආපසු createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  නම් (p != firstSignificantDigit) {
    පූර්ණ_සංඛ්‍යා expAdjustment;

    /* Implicit hexadecimal point?  */
    නම් (dot == end)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<පූර්ණ_සංඛ්‍යා>(dot - firstSignificantDigit);
    නම් (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= partsCount * integerPartWidth;

    /* Adjust for the given exponent.  */
    auto ExpOrErr = totalExponent(p + 1, end, expAdjustment);
    නම් (!ExpOrErr)
      ආපසු ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  ආපසු normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus
IEEEFloat::roundSignificandWithExponent(const integerPart *decSigParts,
                                        unsigned sigPartCount, පූර්ණ_සංඛ්‍යා exp,
                                        roundingMode rounding_mode) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  unsigned පූර්ණ_සංඛ්‍යා power = exp >= 0 ? exp : -exp;
  unsigned පූර්ණ_සංඛ්‍යා pow5PartCapacity = powerOfFivePartsForExponent(power);
  SmallVector<integerPart, NumPow5PartsOnStack> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5PartCapacity);

  bool isNearest = rounding_mode == rmNearestTiesToEven ||
                   rounding_mode == rmNearestTiesToAway;

  unsigned parts = partCountForBits(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  unsigned pow5PartCount = powerOf5(pow5Parts.data(), power);

  සඳහා (;; parts *= 2) {
    unsigned පූර්ණ_සංඛ්‍යා excessPrecision, truncatedBits;

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
    unsigned පූර්ණ_සංඛ්‍යා powHUerr;

    නම් (exp >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calcLostFraction = decSig.multiplySignificand(pow5);
      powHUerr = powStatus != opOK;
    } නැතහොත් {
      calcLostFraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      නම් (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncatedBits = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (powStatus == opOK && calcLostFraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    assert(APInt::tcExtractBit
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calcLostFraction != lfExactlyZero, sigStatus != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, isNearest);

    /* Are we guaranteed to round correctly if we truncate?  */
    නම් (HUdistance >= HUerr) {
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
      ආපසු static_cast<opStatus>(normalize(rounding_mode, calcLostFraction) |
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
  නම් (Error Err = interpretDecimal(p, str.end(), &D))
    ආපසු std::move(Err);

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
  නම් (D.firstSigDigit == str.end() || decDigitValue(*D.firstSigDigit) >= 10U) {
    category = fcZero;
    fs = opOK;
    නම් (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = false;
    නම් (!semantics->hasZero)
      makeSmallestNormalized(false);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } නැතහොත් නම් (D.normalizedExponent - 1 > INT_MAX / 42039) {
    fs = handleOverflow(rounding_mode);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } නැතහොත් නම් (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (පූර්ණ_සංඛ්‍යා) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fcNormal;
    zeroSignificand();
    fs = normalize(rounding_mode, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } නැතහොත් නම් ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handleOverflow(rounding_mode);
  } නැතහොත් {
    integerPart *decSignificand;
    unsigned පූර්ණ_සංඛ්‍යා partCount;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    partCount = static_cast<unsigned පූර්ණ_සංඛ්‍යා>(D.lastSigDigit - D.firstSigDigit) + 1;
    partCount = partCountForBits(1 + 196 * partCount / 59);
    decSignificand = new integerPart[partCount + 1];
    partCount = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    do {
      integerPart decValue, val, multiplier;

      val = 0;
      multiplier = 1;

      do {
        නම් (*p == '.') {
          p++;
          නම් (p == str.end()) {
            bha_nga;
          }
        }
        decValue = decDigitValue(*p++);
        නම් (decValue >= 10U) {
          delete[] decSignificand;
          ආපසු createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + decValue;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } අතරතුර (p <= D.lastSigDigit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            partCount, partCount + 1, false);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      නම් (decSignificand[partCount])
        partCount++;
    } අතරතුර (p <= D.lastSigDigit);

    category = fcNormal;
    fs = roundSignificandWithExponent(decSignificand, partCount,
                                      D.exponent, rounding_mode);

    delete [] decSignificand;
  }

  ආපසු fs;
}

bool IEEEFloat::convertFromStringSpecials(StringRef str) {
  const size_t MIN_NAME_SIZE = 3;

  නම් (str.size() < MIN_NAME_SIZE)
    ආපසු false;

  නම් (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(false);
    ආපසු true;
  }

  bool IsNegative = str.consume_front("-");
  නම් (IsNegative) {
    නම් (str.size() < MIN_NAME_SIZE)
      ආපසු false;

    නම් (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(true);
      ආපසු true;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  bool IsSignaling = str.consume_front_insensitive("s");
  නම් (IsSignaling) {
    නම් (str.size() < MIN_NAME_SIZE)
      ආපසු false;
  }

  නම් (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    නම් (str.empty()) {
      makeNaN(IsSignaling, IsNegative);
      ආපසු true;
    }

    // Allow the payload to be inside parentheses.
    නම් (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      නම් (str.size() <= 2 || str.back() != ')')
        ආපසු false;

      str = str.slice(1, str.size() - 1);
    }

    // Determine the payload number's radix.
    unsigned Radix = 10;
    නම් (str[0] == '0') {
      නම් (str.size() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } නැතහොත් {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    නම් (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      ආපසු true;
    }
  }

  ආපසු false;
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromString(StringRef str, roundingMode rounding_mode) {
  නම් (str.empty())
    ආපසු createError("Invalid string length");

  // Handle special cases.
  නම් (convertFromStringSpecials(str))
    ආපසු opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  sign = *p == '-' ? 1 : 0;
  නම් (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  නම් (*p == '-' || *p == '+') {
    p++;
    slen--;
    නම් (!slen)
      ආපසු createError("String has no digits");
  }

  නම් (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    නම් (slen == 2)
      ආපසු createError("Invalid string");
    ආපසු convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_mode);
  }

  ආපසු convertFromDecimalString(StringRef(p, slen), rounding_mode);
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
unsigned පූර්ණ_සංඛ්‍යා IEEEFloat::convertToHexString(var_na *dst, unsigned පූර්ණ_සංඛ්‍යා hexDigits,
                                           bool upperCase,
                                           roundingMode rounding_mode) const {
  var_na *p = dst;
  නම් (sign)
    *dst++ = '-';

  switch (category) {
  pak_sa fcInfinity:
    memcpy (dst, upperCase ? infinityU: infinityL, parimaa_na infinityU - 1);
    dst += parimaa_na infinityL - 1;
    bha_nga;

  pak_sa fcNaN:
    memcpy (dst, upperCase ? NaNU: NaNL, parimaa_na NaNU - 1);
    dst += parimaa_na NaNU - 1;
    bha_nga;

  pak_sa fcZero:
    *dst++ = '0';
    *dst++ = upperCase ? 'X': 'x';
    *dst++ = '0';
    නම් (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = upperCase ? 'P': 'p';
    *dst++ = '0';
    bha_nga;

  pak_sa fcNormal:
    dst = convertNormalToHexString (dst, hexDigits, upperCase, rounding_mode);
    bha_nga;
  }

  *dst = 0;

  ආපසු static_cast<unsigned පූර්ණ_සංඛ්‍යා>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
var_na *IEEEFloat::convertNormalToHexString(var_na *dst, unsigned පූර්ණ_සංඛ්‍යා hexDigits,
                                          bool upperCase,
                                          roundingMode rounding_mode) const {
  *dst++ = '0';
  *dst++ = upperCase ? 'X': 'x';

  bool roundUp = false;
  const var_na *hexDigitChars = upperCase ? hexDigitsUpper : hexDigitsLower;

  const integerPart *significand = significandParts();
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
  නම් (hexDigits) {
    නම් (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      unsigned පූර්ණ_සංඛ්‍යා bits;
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
  var_na *p = ++dst;

  unsigned count = (valueBits + integerPartWidth - 1) / integerPartWidth;

  අතරතුර (outputDigits && count) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    නම් (--count == partsCount)
      part = 0;  /* An imaginary higher zero part.  */
    නැතහොත්
      part = significand[count] << shift;

    නම් (count && shift)
      part |= significand[count - 1] >> (integerPartWidth - shift);

    /* Convert as much of "part" to hexdigits as we can.  */
    unsigned පූර්ණ_සංඛ්‍යා curDigits = integerPartWidth / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  නම් (roundUp) {
    var_na *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    do {
      q--;
      *q = hexDigitChars[hexDigitValue (*q) + 1];
    } අතරතුර (*q == '0');
    assert(q >= p);
  } නැතහොත් {
    /* Add trailing zeroes.  */
    memset (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  නම් (dst -1 == p)
    dst--;
  නැතහොත්
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = upperCase ? 'P': 'p';

  ආපසු writeSignedDecimal (dst, exponent);
}

hash_code hash_value(const IEEEFloat &Arg) {
  නම් (!Arg.isFiniteNonZero())
    ආපසු hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  ආපසු hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
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

APInt IEEEFloat::convertF80LongDoubleAPFloatToAPInt() const {
  assert(partCount() == 2);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semX87DoubleExtended>();
}

APInt IEEEFloat::convertPPCDoubleDoubleLegacyAPFloatToAPInt() const {
  assert(semantics ==
         (const llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy);
  assert(partCount()==2);

  uint64_t words[2];
  bool losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloatBase::semIEEEdouble.minExponent;
  IEEEFloat extended(*this);
  [[maybe_unused]] opStatus fs =
      extended.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  assert(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
  assert(fs == opOK || fs == opInexact);
  words[0] = *u.convertDoubleAPFloatToAPInt().getRawData();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  නම් (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
    assert(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.subtract(u, rmNearestTiesToEven);
    fs = v.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
    assert(fs == opOK && !losesInfo);
    words[1] = *v.convertDoubleAPFloatToAPInt().getRawData();
  } නැතහොත් {
    words[1] = 0;
  }

  ආපසු APInt(128, words);
}

template <const fltSemantics &S>
APInt IEEEFloat::convertIEEEFloatToAPInt() const {
  assert(semantics == &S);
  constexpr unsigned පූර්ණ_සංඛ්‍යා trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr පූර්ණ_සංඛ්‍යා integer_bit_part = (S.precision - 1) / integerPartWidth;
  constexpr integerPart integer_bit = integerPart{1}
                                      << ((S.precision - 1) % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned පූර්ණ_සංඛ්‍යා exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr bool is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr පූර්ණ_සංඛ්‍යා bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, partCountForBits(trailing_significand_bits)>
      mysignificand;

  නම් (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
    නම් (myexponent == 1 &&
        !(significandParts()[integer_bit_part] & integer_bit))
      myexponent = 0; // denormal
  } නැතහොත් නම් (category == fcZero) {
    නම් (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } නැතහොත් නම් (category == fcInfinity) {
    නම් (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    නම් constexpr (S.hasExplicitIntegerBit) {
      mysignificand[0] = integerPart{1} << (trailing_significand_bits - 1);
    }
  } නැතහොත් {
    assert(category == fcNaN && "Unknown category!");
    නම් (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.sizeInBits + 63) / 64> words;
  auto words_iter =
      std::copy_n(mysignificand.begin(), mysignificand.size(), words.begin());
  නම් constexpr (!S.hasExplicitIntegerBit) {
    නම් constexpr (significand_mask != 0 || trailing_significand_bits == 0) {
      // Clear the integer bit.
      words[mysignificand.size() - 1] &= significand_mask;
    }
  }
  std::fill(words_iter, words.end(), uint64_t{0});
  constexpr size_t last_word = words.size() - 1;
  නම් constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.sizeInBits - 1) % 64);
    words[last_word] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_mask)
                              << (trailing_significand_bits % 64);
  words[last_word] |= shifted_exponent;
  නම් constexpr (last_word == 0) {
    ආපසු APInt(S.sizeInBits, words[0]);
  }
  ආපසු APInt(S.sizeInBits, words);
}

APInt IEEEFloat::convertQuadrupleAPFloatToAPInt() const {
  assert(partCount() == 2);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semIEEEquad>();
}

APInt IEEEFloat::convertDoubleAPFloatToAPInt() const {
  assert(partCount()==1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semIEEEdouble>();
}

APInt IEEEFloat::convertFloatAPFloatToAPInt() const {
  assert(partCount()==1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semIEEEsingle>();
}

APInt IEEEFloat::convertBFloatAPFloatToAPInt() const {
  assert(partCount() == 1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semBFloat>();
}

APInt IEEEFloat::convertHalfAPFloatToAPInt() const {
  assert(partCount()==1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::APFloatBase::semIEEEhalf>();
}

APInt IEEEFloat::convertFloat8E5M2APFloatToAPInt() const {
  assert(partCount() == 1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2>();
}

APInt IEEEFloat::convertFloat8E5M2FNUZAPFloatToAPInt() const {
  assert(partCount() == 1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3APFloatToAPInt() const {
  assert(partCount() == 1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3>();
}

APInt IEEEFloat::convertFloat8E4M3FNAPFloatToAPInt() const {
  assert(partCount() == 1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FN>();
}

APInt IEEEFloat::convertFloat8E4M3FNUZAPFloatToAPInt() const {
  assert(partCount() == 1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3B11FNUZAPFloatToAPInt() const {
  assert(partCount() == 1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convertFloat8E3M4APFloatToAPInt() const {
  assert(partCount() == 1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semFloat8E3M4>();
}

APInt IEEEFloat::convertFloatTF32APFloatToAPInt() const {
  assert(partCount() == 1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semFloatTF32>();
}

APInt IEEEFloat::convertFloat8E8M0FNUAPFloatToAPInt() const {
  assert(partCount() == 1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convertFloat8E5M3FNUAPFloatToAPInt() const {
  assert(partCount() == 1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convertFloat6E3M2FNAPFloatToAPInt() const {
  assert(partCount() == 1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semFloat6E3M2FN>();
}

APInt IEEEFloat::convertFloat6E2M3FNAPFloatToAPInt() const {
  assert(partCount() == 1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semFloat6E2M3FN>();
}

APInt IEEEFloat::convertFloat4E2M1FNAPFloatToAPInt() const {
  assert(partCount() == 1);
  ආපසු convertIEEEFloatToAPInt<APFloatBase::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcastToAPInt() const {
  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semIEEEhalf)
    ආපසු convertHalfAPFloatToAPInt();

  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semBFloat)
    ආපසු convertBFloatAPFloatToAPInt();

  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semIEEEsingle)
    ආපසු convertFloatAPFloatToAPInt();

  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semIEEEdouble)
    ආපසු convertDoubleAPFloatToAPInt();

  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semIEEEquad)
    ආපසු convertQuadrupleAPFloatToAPInt();

  නම් (semantics ==
      (const llvm::fltSemantics *)&APFloatBase::semX87DoubleExtended)
    ආපසු convertF80LongDoubleAPFloatToAPInt();

  නම් (semantics ==
      (const llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy)
    ආපසු convertPPCDoubleDoubleLegacyAPFloatToAPInt();

  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semFloat8E5M2)
    ආපසු convertFloat8E5M2APFloatToAPInt();

  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semFloat8E5M2FNUZ)
    ආපසු convertFloat8E5M2FNUZAPFloatToAPInt();

  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semFloat8E4M3)
    ආපසු convertFloat8E4M3APFloatToAPInt();

  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FN)
    ආපසු convertFloat8E4M3FNAPFloatToAPInt();

  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FNUZ)
    ආපසු convertFloat8E4M3FNUZAPFloatToAPInt();

  නම් (semantics ==
      (const llvm::fltSemantics *)&APFloatBase::semFloat8E4M3B11FNUZ)
    ආපසු convertFloat8E4M3B11FNUZAPFloatToAPInt();

  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semFloat8E3M4)
    ආපසු convertFloat8E3M4APFloatToAPInt();

  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semFloatTF32)
    ආපසු convertFloatTF32APFloatToAPInt();

  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semFloat8E8M0FNU)
    ආපසු convertFloat8E8M0FNUAPFloatToAPInt();

  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semFloat8E5M3FNU)
    ආපසු convertFloat8E5M3FNUAPFloatToAPInt();

  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semFloat6E3M2FN)
    ආපසු convertFloat6E3M2FNAPFloatToAPInt();

  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semFloat6E2M3FN)
    ආපසු convertFloat6E2M3FNAPFloatToAPInt();

  නම් (semantics == (const llvm::fltSemantics *)&APFloatBase::semFloat4E2M1FN)
    ආපසු convertFloat4E2M1FNAPFloatToAPInt();

  llvm_unreachable("unknown format!");
}

float IEEEFloat::convertToFloat() const {
  assert(semantics == (const llvm::fltSemantics *)&APFloatBase::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcastToAPInt();
  ආපසු api.bitsToFloat();
}

dviguu_na IEEEFloat::convertToDouble() const {
  assert(semantics == (const llvm::fltSemantics *)&APFloatBase::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcastToAPInt();
  ආපසු api.bitsToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 IEEEFloat::convertToQuad() const {
  assert(semantics == (const llvm::fltSemantics *)&APFloatBase::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcastToAPInt();
  ආපසු api.bitsToQuad();
}
#endif

void IEEEFloat::initFromF80LongDoubleAPInt(const APInt &api) {
  ආපසු initFromIEEEAPInt<APFloatBase::semX87DoubleExtended>(api);
}

void IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(const APInt &api) {
  uint64_t i1 = api.getRawData()[0];
  uint64_t i2 = api.getRawData()[1];
  bool losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_unused]] opStatus fs = convert(APFloatBase::semPPCDoubleDoubleLegacy,
                                         rmNearestTiesToEven, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  assert((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  නම් (isFiniteNonZero()) {
    IEEEFloat v(APFloatBase::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    නම් (v.isFinite()) {
      fs = v.convert(APFloatBase::semPPCDoubleDoubleLegacy, rmNearestTiesToEven,
                     &losesInfo);
      assert(fs == opOK && !losesInfo);

      add(v, rmNearestTiesToEven);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
void IEEEFloat::initFromFloat8E8M0FNUAPInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E8M0FNU>(api);
}

void IEEEFloat::initFromFloat8E5M3FNUAPInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M3FNU>(api);
}

template <const fltSemantics &S>
void IEEEFloat::initFromIEEEAPInt(const APInt &api) {
  assert(api.getBitWidth() == S.sizeInBits);

  constexpr unsigned පූර්ණ_සංඛ්‍යා trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr integerPart integer_bit =
      integerPart{1} << (trailing_significand_bits % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned පූර්ණ_සංඛ්‍යා exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr unsigned පූර්ණ_සංඛ්‍යා stored_significand_parts =
      partCountForBits(trailing_significand_bits + 1);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr bool is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr පූර්ණ_සංඛ්‍යා bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr bool has_significand = trailing_significand_bits > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  නම් constexpr (has_significand) {
    std::copy_n(api.getRawData(), mysignificand.size(), mysignificand.begin());
    නම් constexpr (significand_mask != 0 || S.precision >= integerPartWidth) {
      mysignificand[mysignificand.size() - 1] &= significand_mask;
    }
  } නැතහොත් {
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
  assert(partCount() == mysignificand.size());

  sign = S.hasSignedRepr
             ? static_cast<unsigned පූර්ණ_සංඛ්‍යා>(last_word >> ((S.sizeInBits - 1) % 64))
             : 0;

  bool all_zero_significand =
      has_significand && llvm::all_of(mysignificand, equal_to(0));

  bool is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  නම් constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    bool is_inf = false;

    නම් constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;
      const integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_mask_no_int_bit) == 0;
    } නැතහොත් {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    නම් (is_inf) {
      makeInf(sign);
      ආපසු;
    }
  }

  bool is_nan = false;

  නම් constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    නම් constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      const integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;

      නම් (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_mask_no_int_bit) != 0) {
        // regular NaN and pseudoNaN
        is_nan = true;
      } නැතහොත් නම් (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_mask_no_int_bit) == 0) {
        // pseudoinfinity
        is_nan = true;
      } නැතහොත් නම් (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = true;
      }
    } නැතහොත් {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } නැතහොත් නම් constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    bool all_ones_significand =
        std::all_of(mysignificand.begin(), mysignificand.end() - 1,
                    [](integerPart bits) { ආපසු bits == ~integerPart{0}; }) &&
        (!significand_mask ||
         mysignificand[mysignificand.size() - 1] == significand_mask);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } නැතහොත් නම් constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  නම් (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.size(),
                significandParts());
    ආපසු;
  }

  නම් (is_zero) {
    makeZero(sign);
    ආපසු;
  }

  category = fcNormal;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.size(), significandParts());
  නම් (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  නැතහොත් {
    නම් constexpr (!S.hasExplicitIntegerBit) {
      significandParts()[mysignificand.size() - 1] |= integer_bit;
    }
  }
}

void IEEEFloat::initFromQuadrupleAPInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEquad>(api);
}

void IEEEFloat::initFromDoubleAPInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEdouble>(api);
}

void IEEEFloat::initFromFloatAPInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEsingle>(api);
}

void IEEEFloat::initFromBFloatAPInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semBFloat>(api);
}

void IEEEFloat::initFromHalfAPInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEhalf>(api);
}

void IEEEFloat::initFromFloat8E5M2APInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2>(api);
}

void IEEEFloat::initFromFloat8E5M2FNUZAPInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2FNUZ>(api);
}

void IEEEFloat::initFromFloat8E4M3APInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3>(api);
}

void IEEEFloat::initFromFloat8E4M3FNAPInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FN>(api);
}

void IEEEFloat::initFromFloat8E4M3FNUZAPInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FNUZ>(api);
}

void IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3B11FNUZ>(api);
}

void IEEEFloat::initFromFloat8E3M4APInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E3M4>(api);
}

void IEEEFloat::initFromFloatTF32APInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloatTF32>(api);
}

void IEEEFloat::initFromFloat6E3M2FNAPInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E3M2FN>(api);
}

void IEEEFloat::initFromFloat6E2M3FNAPInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E2M3FN>(api);
}

void IEEEFloat::initFromFloat4E2M1FNAPInt(const APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
void IEEEFloat::initFromAPInt(const fltSemantics *Sem, const APInt &api) {
  assert(api.getBitWidth() == Sem->sizeInBits);
  නම් (Sem == &APFloatBase::semIEEEhalf)
    ආපසු initFromHalfAPInt(api);
  නම් (Sem == &APFloatBase::semBFloat)
    ආපසු initFromBFloatAPInt(api);
  නම් (Sem == &APFloatBase::semIEEEsingle)
    ආපසු initFromFloatAPInt(api);
  නම් (Sem == &APFloatBase::semIEEEdouble)
    ආපසු initFromDoubleAPInt(api);
  නම් (Sem == &APFloatBase::semIEEEquad)
    ආපසු initFromQuadrupleAPInt(api);
  නම් (Sem == &APFloatBase::semX87DoubleExtended)
    ආපසු initFromF80LongDoubleAPInt(api);
  නම් (Sem == &APFloatBase::semPPCDoubleDoubleLegacy)
    ආපසු initFromPPCDoubleDoubleLegacyAPInt(api);
  නම් (Sem == &APFloatBase::semFloat8E5M2)
    ආපසු initFromFloat8E5M2APInt(api);
  නම් (Sem == &APFloatBase::semFloat8E5M2FNUZ)
    ආපසු initFromFloat8E5M2FNUZAPInt(api);
  නම් (Sem == &APFloatBase::semFloat8E4M3)
    ආපසු initFromFloat8E4M3APInt(api);
  නම් (Sem == &APFloatBase::semFloat8E4M3FN)
    ආපසු initFromFloat8E4M3FNAPInt(api);
  නම් (Sem == &APFloatBase::semFloat8E4M3FNUZ)
    ආපසු initFromFloat8E4M3FNUZAPInt(api);
  නම් (Sem == &APFloatBase::semFloat8E4M3B11FNUZ)
    ආපසු initFromFloat8E4M3B11FNUZAPInt(api);
  නම් (Sem == &APFloatBase::semFloat8E3M4)
    ආපසු initFromFloat8E3M4APInt(api);
  නම් (Sem == &APFloatBase::semFloatTF32)
    ආපසු initFromFloatTF32APInt(api);
  නම් (Sem == &APFloatBase::semFloat8E8M0FNU)
    ආපසු initFromFloat8E8M0FNUAPInt(api);
  නම් (Sem == &APFloatBase::semFloat8E5M3FNU)
    ආපසු initFromFloat8E5M3FNUAPInt(api);
  නම් (Sem == &APFloatBase::semFloat6E3M2FN)
    ආපසු initFromFloat6E3M2FNAPInt(api);
  නම් (Sem == &APFloatBase::semFloat6E2M3FN)
    ආපසු initFromFloat6E2M3FNAPInt(api);
  නම් (Sem == &APFloatBase::semFloat4E2M1FN)
    ආපසු initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
void IEEEFloat::makeLargest(bool Negative) {
  නම් (Negative && !semantics->hasSignedRepr)
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
  memset(significand, 0xFF, parimaa_na(integerPart)*(PartCount - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  const unsigned NumUnusedHighBits =
    PartCount*integerPartWidth - semantics->precision;
  significand[PartCount - 1] = (NumUnusedHighBits < integerPartWidth)
                                   ? (~integerPart(0) >> NumUnusedHighBits)
                                   : 0;
  නම් (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
void IEEEFloat::makeSmallest(bool Negative) {
  නම් (Negative && !semantics->hasSignedRepr)
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

void IEEEFloat::makeSmallestNormalized(bool Negative) {
  නම් (Negative && !semantics->hasSignedRepr)
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

IEEEFloat::IEEEFloat(const fltSemantics &Sem, const APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(float f) {
  initFromAPInt(&APFloatBase::semIEEEsingle, APInt::floatToBits(f));
}

IEEEFloat::IEEEFloat(dviguu_na d) {
  initFromAPInt(&APFloatBase::semIEEEdouble, APInt::doubleToBits(d));
}

namespace {
  void append(SmallVectorImpl<var_na> &Buffer, StringRef Str) {
    Buffer.append(Str.begin(), Str.end());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  void AdjustToPrecision(APInt &significand,
                         පූර්ණ_සංඛ්‍යා &exp, unsigned FormatPrecision) {
    unsigned bits = significand.getActiveBits();

    // 196/59 is a very slight overestimate of lg_2(10).
    unsigned bitsRequired = (FormatPrecision * 196 + 58) / 59;

    නම් (bits <= bitsRequired) ආපසු;

    unsigned tensRemovable = (bits - bitsRequired) * 59 / 196;
    නම් (!tensRemovable) ආපසු;

    exp += tensRemovable;

    APInt divisor(significand.getBitWidth(), 1);
    APInt powten(significand.getBitWidth(), 10);
    අතරතුර (true) {
      නම් (tensRemovable & 1)
        divisor *= powten;
      tensRemovable >>= 1;
      නම් (!tensRemovable) bha_nga;
      powten *= powten;
    }

    significand = significand.udiv(divisor);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.getActiveBits());
  }


  void AdjustToPrecision(SmallVectorImpl<var_na> &buffer,
                         පූර්ණ_සංඛ්‍යා &exp, unsigned FormatPrecision) {
    unsigned N = buffer.size();
    නම් (N <= FormatPrecision) ආපසු;

    // The most significant figures are the last ones in the buffer.
    unsigned FirstSignificant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    නම් (buffer[FirstSignificant - 1] < '5') {
      අතරතුර (FirstSignificant < N && buffer[FirstSignificant] == '0')
        FirstSignificant++;

      exp += FirstSignificant;
      buffer.erase(&buffer[0], &buffer[FirstSignificant]);
      ආපසු;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    සඳහා (unsigned I = FirstSignificant; I != N; ++I) {
      නම් (buffer[I] == '9') {
        FirstSignificant++;
      } නැතහොත් {
        buffer[I]++;
        bha_nga;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    නම් (FirstSignificant == N) {
      exp += FirstSignificant;
      buffer.clear();
      buffer.push_back('1');
      ආපසු;
    }

    exp += FirstSignificant;
    buffer.erase(&buffer[0], &buffer[FirstSignificant]);
  }

  void toStringImpl(SmallVectorImpl<var_na> &Str, const bool isNeg, පූර්ණ_සංඛ්‍යා exp,
                    APInt significand, unsigned FormatPrecision,
                    unsigned FormatMaxPadding, bool TruncateZero) {
    const පූර්ණ_සංඛ්‍යා semanticsPrecision = significand.getBitWidth();

    නම් (isNeg)
      Str.push_back('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    නම් (!FormatPrecision) {
      // We use enough digits so the number can be round-tripped back to an
      // APFloat. The formula comes from "How to Print Floating-Point Numbers
      // Accurately" by Steele and White.
      // FIXME: Using a formula based purely on the precision is conservative;
      // we can print fewer digits depending on the actual value being printed.

      // FormatPrecision = 2 + floor(significandBits / lg_2(10))
      FormatPrecision = 2 + semanticsPrecision * 59 / 196;
    }

    // Ignore trailing binary zeros.
    පූර්ණ_සංඛ්‍යා trailingZeros = significand.countr_zero();
    exp += trailingZeros;
    significand.lshrInPlace(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    නම් (exp == 0) {
      // Nothing to do.
    } නැතහොත් නම් (exp > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + exp);
      significand <<= exp;
      exp = 0;
    } නැතහොත් { /* exp < 0 */
      පූර්ණ_සංඛ්‍යා texp = -exp;

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
      අතරතුර (true) {
        නම් (texp & 1)
          significand *= five_to_the_i;

        texp >>= 1;
        නම් (!texp)
          bha_nga;
        five_to_the_i *= five_to_the_i;
      }
    }

    AdjustToPrecision(significand, exp, FormatPrecision);

    SmallVector<var_na, 256> buffer;

    // Fill the buffer.
    unsigned precision = significand.getBitWidth();
    නම් (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    bool inTrail = true;
    අතරතුර (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      unsigned d = digit.getZExtValue();

      // Drop trailing zeros.
      නම් (inTrail && !d)
        exp++;
      නැතහොත් {
        buffer.push_back((var_na) ('0' + d));
        inTrail = false;
      }
    }

    assert(!buffer.empty() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    AdjustToPrecision(buffer, exp, FormatPrecision);

    unsigned NDigits = buffer.size();

    // Check whether we should use scientific notation.
    bool FormatScientific;
    නම් (!FormatMaxPadding) {
      FormatScientific = true;
    } නැතහොත් {
      නම් (exp >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((unsigned) exp > FormatMaxPadding ||
                            NDigits + (unsigned) exp > FormatPrecision);
      } නැතහොත් {
        // Power of the most significant digit.
        පූර්ණ_සංඛ්‍යා MSD = exp + (පූර්ණ_සංඛ්‍යා) (NDigits - 1);
        නම් (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = false;
        } නැතහොත් {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((unsigned) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    නම් (FormatScientific) {
      exp += (NDigits - 1);

      Str.push_back(buffer[NDigits-1]);
      Str.push_back('.');
      නම් (NDigits == 1 && TruncateZero)
        Str.push_back('0');
      නැතහොත්
        සඳහා (unsigned I = 1; I != NDigits; ++I)
          Str.push_back(buffer[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      නම් (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.push_back(TruncateZero ? 'E' : 'e');

      Str.push_back(exp >= 0 ? '+' : '-');
      නම් (exp < 0)
        exp = -exp;
      SmallVector<var_na, 6> expbuf;
      do {
        expbuf.push_back((var_na) ('0' + (exp % 10)));
        exp /= 10;
      } අතරතුර (exp);
      // Exponent always at least two digits if we do not truncate zeros.
      නම් (!TruncateZero && expbuf.size() < 2)
        expbuf.push_back('0');
      සඳහා (unsigned I = 0, E = expbuf.size(); I != E; ++I)
        Str.push_back(expbuf[E-1-I]);
      ආපසු;
    }

    // Non-scientific, positive exponents.
    නම් (exp >= 0) {
      සඳහා (unsigned I = 0; I != NDigits; ++I)
        Str.push_back(buffer[NDigits-1-I]);
      සඳහා (unsigned I = 0; I != (unsigned) exp; ++I)
        Str.push_back('0');
      ආපසු;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    පූර්ණ_සංඛ්‍යා NWholeDigits = exp + (පූර්ණ_සංඛ්‍යා) NDigits;

    unsigned I = 0;
    නම් (NWholeDigits > 0) {
      සඳහා (; I != (unsigned) NWholeDigits; ++I)
        Str.push_back(buffer[NDigits-I-1]);
      Str.push_back('.');
    } නැතහොත් {
      unsigned NZeros = 1 + (unsigned) -NWholeDigits;

      Str.push_back('0');
      Str.push_back('.');
      සඳහා (unsigned Z = 1; Z != NZeros; ++Z)
        Str.push_back('0');
    }

    සඳහා (; I != NDigits; ++I)
      Str.push_back(buffer[NDigits-I-1]);

  }
} // namespace

void IEEEFloat::toString(SmallVectorImpl<var_na> &Str, unsigned FormatPrecision,
                         unsigned FormatMaxPadding, bool TruncateZero) const {
  switch (category) {
  pak_sa fcInfinity:
    නම් (isNegative())
      ආපසු append(Str, "-Inf");
    නැතහොත්
      ආපසු append(Str, "+Inf");

  pak_sa fcNaN: ආපසු append(Str, "NaN");

  pak_sa fcZero:
    නම් (isNegative())
      Str.push_back('-');

    නම් (!FormatMaxPadding) {
      නම් (TruncateZero)
        append(Str, "0.0E+0");
      නැතහොත් {
        append(Str, "0.0");
        නම් (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } නැතහොත් {
      Str.push_back('0');
    }
    ආපසු;

  pak_sa fcNormal:
    bha_nga;
  }

  // Decompose the number into an APInt and an exponent.
  පූර්ණ_සංඛ්‍යා exp = exponent - ((පූර්ණ_සංඛ්‍යා) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), partCountForBits(semantics->precision)));

  toStringImpl(Str, isNegative(), exp, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

පූර්ණ_සංඛ්‍යා IEEEFloat::getExactLog2Abs() const {
  නම් (!isFinite() || isZero())
    ආපසු INT_MIN;

  const integerPart *Parts = significandParts();
  const පූර්ණ_සංඛ්‍යා PartCount = partCountForBits(semantics->precision);

  පූර්ණ_සංඛ්‍යා PopCount = 0;
  සඳහා (පූර්ණ_සංඛ්‍යා i = 0; i < PartCount; ++i) {
    PopCount += llvm::popcount(Parts[i]);
    නම් (PopCount > 1)
      ආපසු INT_MIN;
  }

  නම් (exponent != semantics->minExponent)
    ආපසු exponent;

  පූර්ණ_සංඛ්‍යා CountrParts = 0;
  සඳහා (පූර්ණ_සංඛ්‍යා i = 0; i < PartCount;
       ++i, CountrParts += APInt::APINT_BITS_PER_WORD) {
    නම් (Parts[i] != 0) {
      ආපසු exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

bool IEEEFloat::isSignaling() const {
  නම් (!isNaN())
    ආපසු false;
  නම් (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    ආපසු false;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  ආපසු !APInt::tcExtractBit(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::opStatus IEEEFloat::next(bool nextDown) {
  // If we are performing nextDown, swap sign so we have -x.
  නම් (nextDown)
    changeSign();

  // Compute nextUp(x)
  opStatus result = opOK;

  // Handle each float category separately.
  switch (category) {
  pak_sa fcInfinity:
    // nextUp(+inf) = +inf
    නම් (!isNegative())
      bha_nga;
    // nextUp(-inf) = -getLargest()
    makeLargest(true);
    bha_nga;
  pak_sa fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    නම් (isSignaling()) {
      result = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(false, isNegative(), nullptr);
    }
    bha_nga;
  pak_sa fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(false);
    bha_nga;
  pak_sa fcNormal:
    // nextUp(-getSmallest()) = -0
    නම් (isSmallest() && isNegative()) {
      APInt::tcSet(significandParts(), 0, partCount());
      category = fcZero;
      exponent = 0;
      නම් (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = false;
      නම් (!semantics->hasZero)
        makeSmallestNormalized(false);
      bha_nga;
    }

    නම් (isLargest() && !isNegative()) {
      නම් (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        bha_nga;
      } නැතහොත් නම් (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        bha_nga;
      } නැතහොත් {
        // nextUp(getLargest()) == INFINITY
        APInt::tcSet(significandParts(), 0, partCount());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        bha_nga;
      }
    }

    // nextUp(normal) == normal + inc.
    නම් (isNegative()) {
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
      APInt::tcDecrement(Parts, partCount());

      නම් (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tcSetBit(Parts, semantics->precision - 1);
        exponent--;
      }
    } නැතහොත් {
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

      නම් (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tcSet(Parts, 0, partCount());
        APInt::tcSetBit(Parts, semantics->precision - 1);
        assert(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } නැතහොත් {
        incrementSignificand();
      }
    }
    bha_nga;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  නම් (nextDown)
    changeSign();

  ආපසු result;
}

APInt IEEEFloat::getNaNPayload() const {
  assert(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  unsigned Bits = semantics->precision - 1;
  ආපසු APInt(Bits, ArrayRef(significandParts(), partCountForBits(Bits)));
}

APFloatBase::ExponentType IEEEFloat::exponentNaN() const {
  ආපසු ::exponentNaN(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentInf() const {
  ආපසු ::exponentInf(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentZero() const {
  ආපසු ::exponentZero(*semantics);
}

void IEEEFloat::makeInf(bool Negative) {
  නම් (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  නම් (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(false, Negative);
    ආපසු;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tcSet(significandParts(), 0, partCount());
}

void IEEEFloat::makeZero(bool Negative) {
  නම් (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  නම් (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = false;
  }
  exponent = exponentZero();
  APInt::tcSet(significandParts(), 0, partCount());
}

void IEEEFloat::makeQuiet() {
  assert(isNaN());
  නම් (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tcSetBit(significandParts(), semantics->precision - 2);
}

පූර්ණ_සංඛ්‍යා ilogb(const IEEEFloat &Arg) {
  නම් (Arg.isNaN())
    ආපසු APFloat::IEK_NaN;
  නම් (Arg.isZero())
    ආපසු APFloat::IEK_Zero;
  නම් (Arg.isInfinity())
    ආපසු APFloat::IEK_Inf;
  නම් (!Arg.isDenormal())
    ආපසු Arg.exponent;

  IEEEFloat Normalized(Arg);
  පූර්ණ_සංඛ්‍යා SignificandBits = Arg.getSemantics().precision - 1;

  Normalized.exponent += SignificandBits;
  Normalized.normalize(APFloat::rmNearestTiesToEven, lfExactlyZero);
  ආපසු Normalized.exponent - SignificandBits;
}

IEEEFloat scalbn(IEEEFloat X, පූර්ණ_සංඛ්‍යා Exp, roundingMode RoundingMode) {
  auto MaxExp = X.getSemantics().maxExponent;
  auto MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  පූර්ණ_සංඛ්‍යා SignificandBits = X.getSemantics().precision - 1;
  පූර්ණ_සංඛ්‍යා MaxIncrement = MaxExp - (MinExp - SignificandBits) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(RoundingMode, lfExactlyZero);
  නම් (X.isNaN())
    X.makeQuiet();
  ආපසු X;
}

IEEEFloat frexp(const IEEEFloat &Val, පූර්ණ_සංඛ්‍යා &Exp, roundingMode RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  නම් (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    ආපසු Quiet;
  }

  නම් (Exp == APFloat::IEK_Inf)
    ආපසු Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  ආපසු scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(const fltSemantics &S)
    : Semantics(&S),
      Floats(new APFloat[2]{APFloat(APFloatBase::semIEEEdouble),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(const fltSemantics &S, uninitializedTag)
    : Semantics(&S), Floats(new APFloat[2]{
                         APFloat(APFloatBase::semIEEEdouble, uninitialized),
                         APFloat(APFloatBase::semIEEEdouble, uninitialized)}) {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(const fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(new APFloat[2]{APFloat(APFloatBase::semIEEEdouble, I),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(const fltSemantics &S, const APInt &I)
    : Semantics(&S),
      Floats(new APFloat[2]{
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[0])),
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[1]))}) {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(const fltSemantics &S, APFloat &&First,
                             APFloat &&Second)
    : Semantics(&S),
      Floats(new APFloat[2]{std::move(First), std::move(Second)}) {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble);
  assert(&Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  assert(&Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(const DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? new APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : nullptr) {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloatBase::semBogus;
  RHS.Floats = nullptr;
  assert(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::operator=(const DoubleAPFloat &RHS) {
  නම් (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } නැතහොත් නම් (this != &RHS) {
    this->~DoubleAPFloat();
    new (this) DoubleAPFloat(RHS);
  }
  ආපසු *this;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
static std::pair<APFloat, APFloat> fastTwoSum(APFloat X, APFloat Y) {
  නම් (!X.isFinite())
    ආපසු {X, APFloat::getZero(X.getSemantics(), /*Negative=*/false)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  ආපසු {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::opStatus DoubleAPFloat::addImpl(const APFloat &a, const APFloat &aa,
                                         const APFloat &c, const APFloat &cc,
                                         roundingMode RM) {
  පූර්ණ_සංඛ්‍යා Status = opOK;
  APFloat z = a;
  Status |= z.add(c, RM);
  නම් (!z.isFinite()) {
    නම් (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ false);
      ආපසු (opStatus)Status;
    }
    Status = opOK;
    auto AComparedToC = a.compareAbsoluteValue(c);
    z = cc;
    Status |= z.add(aa, RM);
    නම් (AComparedToC == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      Status |= z.add(c, RM);
      Status |= z.add(a, RM);
    } නැතහොත් {
      // z = cc + aa + a + c;
      Status |= z.add(a, RM);
      Status |= z.add(c, RM);
    }
    නම් (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ false);
      ආපසු (opStatus)Status;
    }
    Floats[0] = z;
    APFloat zz = aa;
    Status |= zz.add(cc, RM);
    නම් (AComparedToC == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(c, RM);
      Status |= Floats[1].add(zz, RM);
    } නැතහොත් {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(a, RM);
      Status |= Floats[1].add(zz, RM);
    }
  } නැතහොත් {
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
    නම් (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ false);
      ආපසු opOK;
    }
    Floats[0] = z;
    Status |= Floats[0].add(zz, RM);
    නම් (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ false);
      ආපසු (opStatus)Status;
    }
    Floats[1] = std::move(z);
    Status |= Floats[1].subtract(Floats[0], RM);
    Status |= Floats[1].add(zz, RM);
  }
  ආපසු (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::addWithSpecial(const DoubleAPFloat &LHS,
                                                const DoubleAPFloat &RHS,
                                                DoubleAPFloat &Out,
                                                roundingMode RM) {
  නම් (LHS.getCategory() == fcNaN) {
    Out = LHS;
    ආපසු opOK;
  }
  නම් (RHS.getCategory() == fcNaN) {
    Out = RHS;
    ආපසු opOK;
  }
  නම් (LHS.getCategory() == fcZero) {
    Out = RHS;
    ආපසු opOK;
  }
  නම් (RHS.getCategory() == fcZero) {
    Out = LHS;
    ආපසු opOK;
  }
  නම් (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    Out.makeNaN(false, Out.isNegative(), nullptr);
    ආපසු opInvalidOp;
  }
  නම් (LHS.getCategory() == fcInfinity) {
    Out = LHS;
    ආපසු opOK;
  }
  නම් (RHS.getCategory() == fcInfinity) {
    Out = RHS;
    ආපසු opOK;
  }
  assert(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  assert(&A.getSemantics() == &APFloatBase::semIEEEdouble);
  assert(&AA.getSemantics() == &APFloatBase::semIEEEdouble);
  assert(&C.getSemantics() == &APFloatBase::semIEEEdouble);
  assert(&CC.getSemantics() == &APFloatBase::semIEEEdouble);
  assert(&Out.Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  assert(&Out.Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
  ආපසු Out.addImpl(A, AA, C, CC, RM);
}

APFloat::opStatus DoubleAPFloat::add(const DoubleAPFloat &RHS,
                                     roundingMode RM) {
  ආපසු addWithSpecial(*this, RHS, *this, RM);
}

APFloat::opStatus DoubleAPFloat::subtract(const DoubleAPFloat &RHS,
                                          roundingMode RM) {
  changeSign();
  auto Ret = add(RHS, RM);
  changeSign();
  ආපසු Ret;
}

APFloat::opStatus DoubleAPFloat::multiply(const DoubleAPFloat &RHS,
                                          APFloat::roundingMode RM) {
  const auto &LHS = *this;
  auto &Out = *this;
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
  නම් (LHS.getCategory() == fcNaN) {
    Out = LHS;
    ආපසු opOK;
  }
  නම් (RHS.getCategory() == fcNaN) {
    Out = RHS;
    ආපසු opOK;
  }
  නම් ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    Out.makeNaN(false, false, nullptr);
    ආපසු opOK;
  }
  නම් (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    Out = LHS;
    ආපසු opOK;
  }
  නම් (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    Out = RHS;
    ආපසු opOK;
  }
  assert(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal &&
         "Special cases not handled exhaustively");

  පූර්ණ_සංඛ්‍යා Status = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  Status |= T.multiply(C, RM);
  නම් (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ false);
    ආපසු (opStatus)Status;
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
  නම් (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ false);
  } නැතහොත් {
    // Floats[1] = (t - u) + tau
    Status |= T.subtract(U, RM);
    Status |= T.add(Tau, RM);
    Floats[1] = std::move(T);
  }
  ආපසු (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::divide(const DoubleAPFloat &RHS,
                                        APFloat::roundingMode RM) {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.divide(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()), RM);
  *this = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  ආපසු Ret;
}

APFloat::opStatus DoubleAPFloat::remainder(const DoubleAPFloat &RHS) {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.remainder(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *this = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  ආපසු Ret;
}

APFloat::opStatus DoubleAPFloat::mod(const DoubleAPFloat &RHS) {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.mod(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *this = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  ආපසු Ret;
}

APFloat::opStatus
DoubleAPFloat::fusedMultiplyAdd(const DoubleAPFloat &Multiplicand,
                                const DoubleAPFloat &Addend,
                                APFloat::roundingMode RM) {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.fusedMultiplyAdd(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcastToAPInt()),
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, Addend.bitcastToAPInt()),
      RM);
  *this = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  ආපසු Ret;
}

APFloat::opStatus DoubleAPFloat::roundToIntegral(APFloat::roundingMode RM) {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  const APFloat &Hi = getFirst();
  const APFloat &Lo = getSecond();

  APFloat RoundedHi = Hi;
  const opStatus HiStatus = RoundedHi.roundToIntegral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  නම් (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/false);
    ආපසු HiStatus;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  auto RoundToNearestHelper = [](APFloat ToRound, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    const APFloat RoundingError = Rounded - ToRound;
    නම් (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        abs(RoundingError).isExactlyValue(0.5))
      Rounded.add(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rmNearestTiesToEven);
    ආපසු Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  නම් (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    නම් (RM == rmNearestTiesToAway || RM == rmNearestTiesToEven)
      RoundedHi = RoundToNearestHelper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/false);
    ආපසු HiStatus;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  RoundingMode LoRoundingMode;
  නම් (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    LoRoundingMode = isNegative() ? rmTowardPositive : rmTowardNegative;
  නැතහොත්
    LoRoundingMode = RM;

  APFloat RoundedLo = Lo;
  const opStatus LoStatus = RoundedLo.roundToIntegral(LoRoundingMode);
  නම් (LoRoundingMode == rmNearestTiesToAway)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = RoundToNearestHelper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fastTwoSum(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  ආපසු LoStatus;
}

void DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmpResult
DoubleAPFloat::compareAbsoluteValue(const DoubleAPFloat &RHS) const {
  // Compare absolute values of the high parts.
  const cmpResult HiPartCmp = Floats[0].compareAbsoluteValue(RHS.Floats[0]);
  නම් (HiPartCmp != cmpEqual)
    ආපසු HiPartCmp;

  // Zero, regardless of sign, is equal.
  නම් (Floats[1].isZero() && RHS.Floats[1].isZero())
    ආපසු cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  const bool ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  const bool RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  නම් (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    ආපසු RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  නම් (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    ආපසු ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  නම් (ThisIsSubtractive != RHSIsSubtractive)
    ආපසු ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  const cmpResult LoPartCmp = Floats[1].compareAbsoluteValue(RHS.Floats[1]);

  නම් (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    නම් (LoPartCmp == cmpLessThan)
      ආපසු cmpGreaterThan;
    නම් (LoPartCmp == cmpGreaterThan)
      ආපසු cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  ආපසු LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() const {
  ආපසු Floats[0].getCategory();
}

bool DoubleAPFloat::isNegative() const { ආපසු Floats[0].isNegative(); }

void DoubleAPFloat::makeInf(bool Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ false);
}

void DoubleAPFloat::makeZero(bool Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ false);
}

void DoubleAPFloat::makeLargest(bool Neg) {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  නම් (Neg)
    changeSign();
}

void DoubleAPFloat::makeSmallest(bool Neg) {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].makeSmallest(Neg);
  Floats[1].makeZero(/* Neg = */ false);
}

void DoubleAPFloat::makeSmallestNormalized(bool Neg) {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  නම් (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ false);
}

void DoubleAPFloat::makeNaN(bool SNaN, bool Neg, const APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ false);
}

APFloat::cmpResult DoubleAPFloat::compare(const DoubleAPFloat &RHS) const {
  auto Result = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  නම් (Result == APFloat::cmpEqual)
    ආපසු Floats[1].compare(RHS.Floats[1]);
  ආපසු Result;
}

bool DoubleAPFloat::bitwiseIsEqual(const DoubleAPFloat &RHS) const {
  ආපසු Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_code hash_value(const DoubleAPFloat &Arg) {
  නම් (Arg.Floats)
    ආපසු hash_combine(hash_value(Arg.Floats[0]), hash_value(Arg.Floats[1]));
  ආපසු hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcastToAPInt() const {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t Data[] = {
      Floats[0].bitcastToAPInt().getRawData()[0],
      Floats[1].bitcastToAPInt().getRawData()[0],
  };
  ආපසු APInt(128, Data);
}

Expected<APFloat::opStatus> DoubleAPFloat::convertFromString(StringRef S,
                                                             roundingMode RM) {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy);
  auto Ret = Tmp.convertFromString(S, RM);
  *this = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  ආපසු Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::opStatus DoubleAPFloat::next(bool nextDown) {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  නම් (nextDown) {
    changeSign();
    APFloat::opStatus Result = next(/*nextDown=*/false);
    changeSign();
    ආපසු Result;
  }
  switch (getCategory()) {
  pak_sa fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    නම් (isNegative())
      makeLargest(true);
    ආපසු opOK;

  pak_sa fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    නම් (getFirst().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(false, isNegative(), nullptr);
      ආපසු opInvalidOp;
    }
    ආපසු opOK;

  pak_sa fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(false);
    ආපසු opOK;

  pak_sa fcNormal:
    bha_nga;
  }

  const APFloat &HiOld = getFirst();
  const APFloat &LoOld = getSecond();

  APFloat NextLo = LoOld;
  NextLo.next(/*nextDown=*/false);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  auto InLattice = [](const APFloat &Hi, const APFloat &Lo) {
    ආපසු Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  නම් (InLattice(HiOld, NextLo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(NextLo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat Largest{*Semantics, uninitialized};
    Largest.makeLargest(/*Neg=*/false);
    නම් (compare(Largest) == cmpGreaterThan)
      makeInf(/*Neg=*/false);

    ආපසු opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat NextHi = HiOld;
  NextHi.next(/*nextDown=*/false);

  // nextUp(getLargest()) == INFINITY
  නම් (NextHi.isInfinity()) {
    makeInf(/*Neg=*/false);
    ආපසු opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  නම් (NextHi.isZero()) {
    makeZero(/*Neg=*/true);
    ආපසු opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  NextLo = neg(scalbn(harrisonUlp(NextHi), -1, rmTowardZero));
  නම් (!InLattice(NextHi, NextLo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    NextLo.next(/*nextDown=*/false);

  Floats[0] = std::move(NextHi);
  Floats[1] = std::move(NextLo);

  ආපසු opOK;
}

APFloat::opStatus DoubleAPFloat::convertToSignExtendedInteger(
    MutableArrayRef<integerPart> Input, unsigned පූර්ණ_සංඛ්‍යා Width, bool IsSigned,
    roundingMode RM, bool *IsExact) const {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  නම් (!getFirst().isFiniteNonZero() || getSecond().isZero())
    ආපසු getFirst().convertToInteger(Input, Width, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = false;
  DoubleAPFloat Integral = *this;
  const opStatus RoundStatus = Integral.roundToIntegral(RM);
  නම් (RoundStatus == opInvalidOp)
    ආපසු opInvalidOp;
  const APFloat &IntegralHi = Integral.getFirst();
  const APFloat &IntegralLo = Integral.getSecond();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  bool HiIsExact;
  නම් (IntegralHi.isZero() || IntegralLo.isZero()) {
    const opStatus HiStatus =
        IntegralHi.convertToInteger(Input, Width, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    ආපසු HiStatus == opOK ? opInexact : HiStatus;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  නම් (!IsSigned && IntegralHi.isNegative())
    ආපසු opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  bool LoIsExact;
  const පූර්ණ_සංඛ්‍යා HiExactLog2 = IntegralHi.getExactLog2Abs();
  const unsigned PositiveOverflowWidth = IsSigned ? Width - 1 : Width;
  නම් (HiExactLog2 >= 0 &&
      static_cast<unsigned>(HiExactLog2) == PositiveOverflowWidth) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    නම් (IntegralHi.isNegative() == IntegralLo.isNegative())
      ආපසු opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    const opStatus LoStatus = IntegralLo.convertToInteger(
        Input, Width, /*IsSigned=*/true, RM, &LoIsExact);
    නම් (LoStatus == opInvalidOp)
      ආපසු opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    නම් (IsSigned && !IntegralHi.isNegative())
      APInt::tcClearBit(Input.data(), PositiveOverflowWidth);
    *IsExact = RoundStatus == opOK;
    ආපසු RoundStatus;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  const opStatus HiStatus = IntegralHi.convertToInteger(
      Input, Width, IsSigned, rmTowardZero, &HiIsExact);
  නම් (HiStatus == opInvalidOp)
    ආපසු HiStatus;

  // Convert Lo into a temporary integer of the same width.
  APSInt LoResult{Width, /*isUnsigned=*/!IsSigned};
  const opStatus LoStatus =
      IntegralLo.convertToInteger(LoResult, rmTowardZero, &LoIsExact);
  නම් (LoStatus == opInvalidOp)
    ආපසු LoStatus;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tcAdd(Input.data(), LoResult.getRawData(), /*carry=*/0, Input.size());

  *IsExact = RoundStatus == opOK;
  ආපසු RoundStatus;
}

APFloat::opStatus
DoubleAPFloat::convertToInteger(MutableArrayRef<integerPart> Input,
                                unsigned පූර්ණ_සංඛ්‍යා Width, bool IsSigned,
                                roundingMode RM, bool *IsExact) const {
  opStatus FS =
      convertToSignExtendedInteger(Input, Width, IsSigned, RM, IsExact);

  නම් (FS == opInvalidOp) {
    const unsigned DstPartsCount = partCountForBits(Width);
    assert(DstPartsCount <= Input.size() && "Integer too big");

    unsigned Bits;
    නම් (getCategory() == fcNaN)
      Bits = 0;
    නැතහොත් නම් (isNegative())
      Bits = IsSigned;
    නැතහොත්
      Bits = Width - IsSigned;

    tcSetLeastSignificantBits(Input.data(), DstPartsCount, Bits);
    නම් (isNegative() && IsSigned)
      APInt::tcShiftLeft(Input.data(), DstPartsCount, Width - 1);
  }

  ආපසු FS;
}

APFloat::opStatus DoubleAPFloat::handleOverflow(roundingMode RM) {
  switch (RM) {
  pak_sa APFloat::rmTowardZero:
    makeLargest(/*Neg=*/isNegative());
    bha_nga;
  pak_sa APFloat::rmTowardNegative:
    නම් (isNegative())
      makeInf(/*Neg=*/true);
    නැතහොත්
      makeLargest(/*Neg=*/false);
    bha_nga;
  pak_sa APFloat::rmTowardPositive:
    නම් (isNegative())
      makeLargest(/*Neg=*/true);
    නැතහොත්
      makeInf(/*Neg=*/false);
    bha_nga;
  pak_sa APFloat::rmNearestTiesToAway:
  pak_sa APFloat::rmNearestTiesToEven:
    makeInf(/*Neg=*/isNegative());
    bha_nga;
  default:
    llvm_unreachable("Invalid rounding mode found");
  }
  opStatus S = opInexact;
  නම් (!getFirst().isFinite())
    S = static_cast<opStatus>(S | opOverflow);
  ආපසු S;
}

APFloat::opStatus DoubleAPFloat::convertFromUnsignedParts(
    const integerPart *Src, unsigned පූර්ණ_සංඛ්‍යා SrcCount, roundingMode RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  const unsigned SrcMSB = APInt::tcMSB(Src, SrcCount);
  නම් (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/false);
    ආපසු opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  const unsigned SrcBitWidth = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/SrcBitWidth, ArrayRef(Src, SrcCount)},
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
  නම් (!Hi.isFinite())
    ආපසු handleOverflow(RM);

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
  const APInt Error = SrcInt.zext(HiAsInt.getBitWidth()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  roundingMode LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  නම් (Error.isNegative()) {
    නම් (RM == rmNearestTiesToAway) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      const unsigned ErrorActiveBits = Error.getSignificantBits() - 1;
      const unsigned LoPrecision = getSecond().getSemantics().precision;
      නම් (ErrorActiveBits > LoPrecision) {
        const unsigned RoundingBoundary = ErrorActiveBits - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        නම් (Error.countTrailingZeros() == RoundingBoundary - 1)
          LoRM = rmTowardPositive;
      }
    } නැතහොත් නම් (RM == rmTowardZero) {
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
  නම් (!getFirst().isFinite())
    ආපසු handleOverflow(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  නම් (getFirst().isFiniteNonZero() && Floats[0].isLargest()) {
    DoubleAPFloat Largest{*Semantics};
    Largest.makeLargest(/*Neg=*/false);
    නම් (compare(Largest) == APFloat::cmpGreaterThan)
      ආපසු handleOverflow(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  ආපසු Status;
}

APFloat::opStatus DoubleAPFloat::convertFromAPInt(const APInt &Input,
                                                  bool IsSigned,
                                                  roundingMode RM) {
  const bool NegateInput = IsSigned && Input.isNegative();
  APInt API = Input;
  නම් (NegateInput)
    API.negate();

  const APFloat::opStatus Status =
      convertFromUnsignedParts(API.getRawData(), API.getNumWords(), RM);
  නම් (NegateInput)
    changeSign();
  ආපසු Status;
}

unsigned පූර්ණ_සංඛ්‍යා DoubleAPFloat::convertToHexString(var_na *DST,
                                               unsigned පූර්ණ_සංඛ්‍යා HexDigits,
                                               bool UpperCase,
                                               roundingMode RM) const {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ආපසු APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .convertToHexString(DST, HexDigits, UpperCase, RM);
}

bool DoubleAPFloat::isDenormal() const {
  ආපසු getCategory() == fcNormal &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

bool DoubleAPFloat::isSmallest() const {
  නම් (getCategory() != fcNormal)
    ආපසු false;
  DoubleAPFloat Tmp(*this);
  Tmp.makeSmallest(this->isNegative());
  ආපසු Tmp.compare(*this) == cmpEqual;
}

bool DoubleAPFloat::isSmallestNormalized() const {
  නම් (getCategory() != fcNormal)
    ආපසු false;

  DoubleAPFloat Tmp(*this);
  Tmp.makeSmallestNormalized(this->isNegative());
  ආපසු Tmp.compare(*this) == cmpEqual;
}

bool DoubleAPFloat::isLargest() const {
  නම් (getCategory() != fcNormal)
    ආපසු false;
  DoubleAPFloat Tmp(*this);
  Tmp.makeLargest(this->isNegative());
  ආපසු Tmp.compare(*this) == cmpEqual;
}

bool DoubleAPFloat::isInteger() const {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  ආපසු Floats[0].isInteger() && Floats[1].isInteger();
}

void DoubleAPFloat::toString(SmallVectorImpl<var_na> &Str,
                             unsigned FormatPrecision,
                             unsigned FormatMaxPadding,
                             bool TruncateZero) const {
  assert(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .toString(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

පූර්ණ_සංඛ්‍යා DoubleAPFloat::getExactLog2Abs() const {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  නම් (getSecond().isNonZero())
    ආපසු INT_MIN;
  ආපසු getFirst().getExactLog2Abs();
}

පූර්ණ_සංඛ්‍යා ilogb(const DoubleAPFloat &Arg) {
  const APFloat &Hi = Arg.getFirst();
  const APFloat &Lo = Arg.getSecond();
  පූර්ණ_සංඛ්‍යා IlogbResult = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  නම් (Arg.getCategory() != fcNormal)
    ආපසු IlogbResult;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  නම් (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    ආපසු IlogbResult;
  නම් (Hi.getExactLog2Abs() == INT_MIN)
    ආපසු IlogbResult;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  ආපසු IlogbResult - 1;
}

DoubleAPFloat scalbn(const DoubleAPFloat &Arg, පූර්ණ_සංඛ්‍යා Exp,
                     APFloat::roundingMode RM) {
  assert(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");
  ආපසු DoubleAPFloat(APFloatBase::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(const DoubleAPFloat &Arg, පූර්ණ_සංඛ්‍යා &Exp,
                    APFloat::roundingMode RM) {
  assert(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  නම් (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.getFirst() = Quiet.getFirst().makeQuiet();
    ආපසු Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  නම් (Exp == APFloat::IEK_Inf)
    ආපසු Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  නම් (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    ආපසු Arg;
  }

  const APFloat &Hi = Arg.getFirst();
  const APFloat &Lo = Arg.getSecond();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  const bool SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat Second = Lo;
  නම් (Arg.getCategory() == APFloat::fcNormal && Lo.isFiniteNonZero()) {
    roundingMode LoRoundingMode;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    නම් (RM == rmTowardZero)
      LoRoundingMode = Arg.isNegative() ? rmTowardPositive : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    නැතහොත් නම් (RM == rmNearestTiesToAway && SignsDisagree && Exp > 0)
      LoRoundingMode = rmNearestTiesToEven;
    නැතහොත්
      LoRoundingMode = RM;
    Second = scalbn(Lo, -Exp, LoRoundingMode);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    නම් (RM == rmNearestTiesToAway && LoRoundingMode == rmNearestTiesToEven) {
      // Re-scale the result back to check if rounding occurred.
      const APFloat RecomposedLo = scalbn(Second, Exp, rmNearestTiesToEven);
      නම් (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        const APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        const APFloat UlpOfSecond = harrisonUlp(Second);
        const APFloat ScaledUlpOfSecond =
            scalbn(UlpOfSecond, Exp - 1, rmNearestTiesToEven);
        const bool IsMidpoint = abs(RoundingError) == ScaledUlpOfSecond;
        const bool RoundedLoAway =
            Second.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        නම් (IsMidpoint && RoundedLoAway)
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
    නම් (Second.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat First = scalbn(Hi, -Exp, RM);
  ආපසු DoubleAPFloat(APFloatBase::PPCDoubleDouble(), std::move(First),
                       std::move(Second));
}

APInt DoubleAPFloat::getNaNPayload() const { ආපසු Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, const fltSemantics &Semantics) {
  නම් (usesLayout<IEEEFloat>(Semantics)) {
    new (&IEEE) IEEEFloat(std::move(F));
    ආපසු;
  }
  නම් (usesLayout<DoubleAPFloat>(Semantics)) {
    const fltSemantics& S = F.getSemantics();
    new (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloatBase::IEEEdouble()));
    ආපසු;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::opStatus> APFloat::convertFromString(StringRef Str,
                                                       roundingMode RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_code hash_value(const APFloat &Arg) {
  නම් (APFloat::usesLayout<detail::IEEEFloat>(Arg.getSemantics()))
    ආපසු hash_value(Arg.U.IEEE);
  නම් (APFloat::usesLayout<detail::DoubleAPFloat>(Arg.getSemantics()))
    ආපසු hash_value(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(const fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  auto StatusOrErr = convertFromString(S, rmNearestTiesToEven);
  assert(StatusOrErr && "Invalid floating point representation");
  consumeError(StatusOrErr.takeError());
}

FPClassTest APFloat::classify() const {
  නම් (isZero())
    ආපසු isNegative() ? fcNegZero : fcPosZero;
  නම් (isNormal())
    ආපසු isNegative() ? fcNegNormal : fcPosNormal;
  නම් (isDenormal())
    ආපසු isNegative() ? fcNegSubnormal : fcPosSubnormal;
  නම් (isInfinity())
    ආපසු isNegative() ? fcNegInf : fcPosInf;
  assert(isNaN() && "Other class of FP constant");
  ආපසු isSignaling() ? fcSNan : fcQNan;
}

bool APFloat::getExactInverse(APFloat *Inv) const {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  නම් (!isFiniteNonZero())
    ආපසු false;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  නම් (isDenormal())
    ආපසු false;

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
  const පූර්ණ_සංඛ්‍යා Exp = getExactLog2Abs();
  නම් (Exp == INT_MIN)
    ආපසු false;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  නම් (Reciprocal.getExactLog2Abs() != -Exp)
    ආපසු false;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  නම් (Reciprocal.isDenormal())
    ආපසු false;

  assert(Reciprocal.isFiniteNonZero());

  නම් (Inv)
    *Inv = std::move(Reciprocal);

  ආපසු true;
}

APFloat::opStatus APFloat::convert(const fltSemantics &ToSemantics,
                                   roundingMode RM, bool *losesInfo) {
  නම් (&getSemantics() == &ToSemantics) {
    *losesInfo = false;
    ආපසු opOK;
  }
  නම් (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics))
    ආපසු U.IEEE.convert(ToSemantics, RM, losesInfo);
  නම් (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<DoubleAPFloat>(ToSemantics)) {
    assert(&ToSemantics == &APFloatBase::semPPCDoubleDouble);
    auto Ret =
        U.IEEE.convert(APFloatBase::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *this = APFloat(ToSemantics, U.IEEE.bitcastToAPInt());
    ආපසු Ret;
  }
  නම් (usesLayout<DoubleAPFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics)) {
    auto Ret = getIEEE().convert(ToSemantics, RM, losesInfo);
    *this = APFloat(std::move(getIEEE()), ToSemantics);
    ආපසු Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::getAllOnesValue(const fltSemantics &Semantics) {
  ආපසු APFloat(Semantics, APInt::getAllOnes(Semantics.sizeInBits));
}

void APFloat::print(raw_ostream &OS) const {
  SmallVector<var_na, 16> Buffer;
  toString(Buffer);
  OS << Buffer;
}

#නම් !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void APFloat::dump() const {
  print(dbgs());
  dbgs() << '\n';
}
#endif

void APFloat::Profile(FoldingSetNodeID &NID) const {
  NID.Add(bitcastToAPInt());
}

APFloat::opStatus APFloat::convertToInteger(APSInt &result,
                                            roundingMode rounding_mode,
                                            bool *isExact) const {
  unsigned bitWidth = result.getBitWidth();
  SmallVector<uint64_t, 4> parts(result.getNumWords());
  opStatus status = convertToInteger(parts, bitWidth, result.isSigned(),
                                     rounding_mode, isExact);
  // Keeps the original signed-ness.
  result = APInt(bitWidth, parts);
  ආපසු status;
}

dviguu_na APFloat::convertToDouble() const {
  නම් (&getSemantics() == &APFloatBase::semIEEEdouble)
    ආපසු getIEEE().convertToDouble();
  assert(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat Temp = *this;
  bool LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &LosesInfo);
  assert(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  ආපසු Temp.getIEEE().convertToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 APFloat::convertToQuad() const {
  නම් (&getSemantics() == &APFloatBase::semIEEEquad)
    ආපසු getIEEE().convertToQuad();
  assert(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat Temp = *this;
  bool LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEquad, rmNearestTiesToEven, &LosesInfo);
  assert(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  ආපසු Temp.getIEEE().convertToQuad();
}
#endif

float APFloat::convertToFloat() const {
  නම් (&getSemantics() == &APFloatBase::semIEEEsingle)
    ආපසු getIEEE().convertToFloat();
  assert(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat Temp = *this;
  bool LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEsingle, rmNearestTiesToEven, &LosesInfo);
  assert(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  ආපසු Temp.getIEEE().convertToFloat();
}

unsigned APFloatBase::getArbitraryFPFormatSizeInBits(StringRef Format) {
  ආපසු StringSwitch<unsigned>(Format)
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

bool APFloatBase::isValidArbitraryFPFormat(StringRef Format) {
  ආපසු getArbitraryFPFormatSizeInBits(Format) != 0;
}

const fltSemantics *APFloatBase::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  ආපසු StringSwitch<const fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(nullptr);
}

APFloat::Storage::~Storage() {
  නම් (usesLayout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    ආපසු;
  }
  නම් (usesLayout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    ආපසු;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(const APFloat::Storage &RHS) {
  නම් (usesLayout<IEEEFloat>(*RHS.semantics)) {
    new (this) IEEEFloat(RHS.IEEE);
    ආපසු;
  }
  නම් (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    new (this) DoubleAPFloat(RHS.Double);
    ආපසු;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  නම් (usesLayout<IEEEFloat>(*RHS.semantics)) {
    new (this) IEEEFloat(std::move(RHS.IEEE));
    ආපසු;
  }
  නම් (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    new (this) DoubleAPFloat(std::move(RHS.Double));
    ආපසු;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::operator=(const APFloat::Storage &RHS) {
  නම් (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } නැතහොත් නම් (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } නැතහොත් නම් (this != &RHS) {
    this->~Storage();
    new (this) Storage(RHS);
  }
  ආපසු *this;
}

APFloat::Storage &APFloat::Storage::operator=(APFloat::Storage &&RHS) {
  නම් (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } නැතහොත් නම් (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } නැතහොත් නම් (this != &RHS) {
    this->~Storage();
    new (this) Storage(std::move(RHS));
  }
  ආපසු *this;
}

namespace {

APFloat::opStatus getOpStatusFromLibc(පූර්ණ_සංඛ්‍යා libc_exceptions) {
  APFloat::opStatus status = APFloat::opOK;
  නම් (libc_exceptions & FE_INVALID)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInvalidOp);
  නම් (libc_exceptions & FE_DIVBYZERO)
    status = static_cast<APFloat::opStatus>(status | APFloat::opDivByZero);
  නම් (libc_exceptions & FE_OVERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opOverflow);
  නම් (libc_exceptions & FE_UNDERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opUnderflow);
  නම් (libc_exceptions & FE_INEXACT)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInexact);
  ආපසු status;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> exp(const APFloat &x, RoundingMode rounding_mode,
                           APFloat::opStatus *status) {

  නම් (rounding_mode == APFloatBase::rmNearestTiesToEven) {
    නම් (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEsingle) {
      float x_val = x.convertToFloat();
      පූර්ණ_සංඛ්‍යා exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      නම් (status) {
        *status = getOpStatusFromLibc(exc);
        නම් (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      float result = LIBC_NAMESPACE::shared::expf(x_val);
      ආපසු APFloat(result);
    }
    නම් (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEdouble) {
      dviguu_na x_val = x.convertToDouble();
      පූර්ණ_සංඛ්‍යා exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      නම් (status) {
        *status = getOpStatusFromLibc(exc);
        නම් (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      dviguu_na result = LIBC_NAMESPACE::shared::exp(x_val);
      ආපසු APFloat(result);
    }
  }
  ආපසු std::nullopt;
}

} // namespace llvm

#undef APFLOAT_DISPATCH_ON_SEMANTICS
