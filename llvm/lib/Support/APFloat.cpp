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

#அந்தர்பாவ "llvm/ADT/APFloat.h"
#அந்தர்பாவ "llvm/ADT/APSInt.h"
#அந்தர்பாவ "llvm/ADT/ArrayRef.h"
#அந்தர்பாவ "llvm/ADT/FloatingPointMode.h"
#அந்தர்பாவ "llvm/ADT/FoldingSet.h"
#அந்தர்பாவ "llvm/ADT/Hashing.h"
#அந்தர்பாவ "llvm/ADT/STLExtras.h"
#அந்தர்பாவ "llvm/ADT/SmallVector.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/ADT/StringSwitch.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/Error.h"
#அந்தர்பாவ "llvm/Support/MathExtras.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <cstring>
#அந்தர்பாவ <iterator>
#அந்தர்பாவ <limits.h>

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

#அந்தர்பாவ "shared/math.h"
#அந்தர்பாவ "shared/math_check_exceptions.h"

#define APFLOAT_DISPATCH_ON_SEMANTICS(METHOD_CALL)                             \
  செய் {                                                                         \
    எனில் (usesLayout<IEEEFloat>(getSemantics()))                                 \
      திரும்பு U.IEEE.METHOD_CALL;                                               \
    எனில் (usesLayout<DoubleAPFloat>(getSemantics()))                             \
      திரும்பு U.Double.METHOD_CALL;                                             \
    llvm_unreachable("Unexpected semantics");                                  \
  } போது (பொய்)

using நாமாகாஶ llvm;

/// A macro used to combine two fcCategory enums into one key which can be used
/// in a switch statement to classify how the interaction of two APFloat's
/// categories affects an operation.
///
/// TODO: If clang source code is ever allowed to use constexpr in its own
/// codebase, change this into a static inline function.
#define Pack_Categories_Into_சாவி(_lhs, _rhs) ((_lhs) * 4 + (_rhs))

/* Assumed in hexadecimal significand parsing, and conversion to
   hexadecimal strings.  */
static_assert(APFloatBase::integerPartWidth % 4 == 0, "Part width must be divisible by 4!");

நாமாகாஶ llvm {

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
    உண்மை,
    உண்மை,
    உண்மை,
    உண்மை,
    உண்மை};
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
    பொய்,
    பொய்,
    பொய்,
    பொய்};

constexpr fltSemantics APFloatBase::semFloat8E5M3FNU = {
    16,
    -14,
    4,
    8,
    fltNonfiniteBehavior::NanOnly,
    fltNanEncoding::AllOnes,
    உண்மை,
    பொய்,
    பொய்};

constexpr fltSemantics APFloatBase::semFloat6E3M2FN = {
    4, -2, 3, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat6E2M3FN = {
    2, 0, 4, 6, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semFloat4E2M1FN = {
    2, 0, 2, 4, fltNonfiniteBehavior::FiniteOnly};
constexpr fltSemantics APFloatBase::semBogus = {0, 0, 0, 0};

மாறிலி llvm::fltSemantics &APFloatBase::EnumToSemantics(Semantics S) {
  தேர்வு (S) {
  நிலை S_IEEEhalf:
    திரும்பு IEEEhalf();
  நிலை S_BFloat:
    திரும்பு BFloat();
  நிலை S_IEEEsingle:
    திரும்பு IEEEsingle();
  நிலை S_IEEEdouble:
    திரும்பு IEEEdouble();
  நிலை S_IEEEquad:
    திரும்பு IEEEquad();
  நிலை S_x87DoubleExtended:
    திரும்பு x87DoubleExtended();
  நிலை S_PPCDoubleDouble:
    திரும்பு PPCDoubleDouble();
  நிலை S_PPCDoubleDoubleLegacy:
    திரும்பு PPCDoubleDoubleLegacy();
  நிலை S_Float8E5M2:
    திரும்பு Float8E5M2();
  நிலை S_Float8E5M2FNUZ:
    திரும்பு Float8E5M2FNUZ();
  நிலை S_Float8E4M3:
    திரும்பு Float8E4M3();
  நிலை S_Float8E4M3FN:
    திரும்பு Float8E4M3FN();
  நிலை S_Float8E4M3FNUZ:
    திரும்பு Float8E4M3FNUZ();
  நிலை S_Float8E4M3B11FNUZ:
    திரும்பு Float8E4M3B11FNUZ();
  நிலை S_Float8E3M4:
    திரும்பு Float8E3M4();
  நிலை S_FloatTF32:
    திரும்பு FloatTF32();
  நிலை S_Float8E8M0FNU:
    திரும்பு Float8E8M0FNU();
  நிலை S_Float8E5M3FNU:
    திரும்பு Float8E5M3FNU();
  நிலை S_Float6E3M2FN:
    திரும்பு Float6E3M2FN();
  நிலை S_Float6E2M3FN:
    திரும்பு Float6E2M3FN();
  நிலை S_Float4E2M1FN:
    திரும்பு Float4E2M1FN();
  }
  llvm_unreachable("Unrecognised floating semantics");
}

APFloatBase::Semantics
APFloatBase::SemanticsToEnum(மாறிலி llvm::fltSemantics &Sem) {
  எனில் (&Sem == &llvm::APFloat::IEEEhalf())
    திரும்பு S_IEEEhalf;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::BFloat())
    திரும்பு S_BFloat;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::IEEEsingle())
    திரும்பு S_IEEEsingle;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::IEEEdouble())
    திரும்பு S_IEEEdouble;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::IEEEquad())
    திரும்பு S_IEEEquad;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::x87DoubleExtended())
    திரும்பு S_x87DoubleExtended;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::PPCDoubleDouble())
    திரும்பு S_PPCDoubleDouble;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::PPCDoubleDoubleLegacy())
    திரும்பு S_PPCDoubleDoubleLegacy;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::Float8E5M2())
    திரும்பு S_Float8E5M2;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::Float8E5M2FNUZ())
    திரும்பு S_Float8E5M2FNUZ;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::Float8E4M3())
    திரும்பு S_Float8E4M3;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::Float8E4M3FN())
    திரும்பு S_Float8E4M3FN;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::Float8E4M3FNUZ())
    திரும்பு S_Float8E4M3FNUZ;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::Float8E4M3B11FNUZ())
    திரும்பு S_Float8E4M3B11FNUZ;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::Float8E3M4())
    திரும்பு S_Float8E3M4;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::FloatTF32())
    திரும்பு S_FloatTF32;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::Float8E8M0FNU())
    திரும்பு S_Float8E8M0FNU;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::Float8E5M3FNU())
    திரும்பு S_Float8E5M3FNU;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::Float6E3M2FN())
    திரும்பு S_Float6E3M2FN;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::Float6E2M3FN())
    திரும்பு S_Float6E2M3FN;
  இல்லையெனில் எனில் (&Sem == &llvm::APFloat::Float4E2M1FN())
    திரும்பு S_Float4E2M1FN;
  இல்லையெனில்
    llvm_unreachable("Unknown floating semantics");
}

தருக்க APFloatBase::isRepresentableBy(மாறிலி fltSemantics &A,
                                    மாறிலி fltSemantics &B) {
  திரும்பு A.maxExponent <= B.maxExponent && A.minExponent >= B.minExponent &&
         A.precision <= B.precision;
}

தருக்க APFloatBase::isLosslesslyConvertibleTo(மாறிலி fltSemantics &From,
                                            மாறிலி fltSemantics &InitialTo,
                                            தருக்க IgnoreNaNs) {
  எனில் (&From == &InitialTo)
    திரும்பு உண்மை;

  // PPC double-double cannot be described by a conventional exponent range
  // and precision. In particular, converting it to another semantics drops
  // its low double, so conservatively reject conversions from it.
  எனில் (&From == &semPPCDoubleDouble)
    திரும்பு பொய்;

  // Converting to PPC double-double is lossless exactly when the value fits
  // in its high double. In other words, the conversion is lossless if the
  // conversion to IEEEdouble is lossless.
  மாறிலி fltSemantics &To =
      &InitialTo == &semPPCDoubleDouble ? semIEEEdouble : InitialTo;

  எனில் (!isRepresentableBy(From, To))
    திரும்பு பொய்;

  எனில் ((From.hasZero && !To.hasZero) ||
      (From.hasSignedRepr && !To.hasSignedRepr))
    திரும்பு பொய்;

  // NegativeZero NaN encoding repurposes the negative-zero bit pattern, so a
  // conversion to such a format cannot preserve a source negative zero.
  தருக்க FromHasSignedZero = From.hasZero && From.hasSignedRepr &&
                           From.nanEncoding != fltNanEncoding::NegativeZero;
  தருக்க ToHasSignedZero = To.hasZero && To.hasSignedRepr &&
                         To.nanEncoding != fltNanEncoding::NegativeZero;
  எனில் (FromHasSignedZero && !ToHasSignedZero)
    திரும்பு பொய்;

  // isRepresentableBy compares normalized exponent ranges. Also ensure that
  // the smallest source value, which may be denormal, is represented exactly
  // by the destination semantics.
  APFloat SmallestFrom = APFloat::getSmallest(From);
  தருக்க LosesInfo = பொய்;
  (வெறுமை)SmallestFrom.convert(To, APFloat::rmNearestTiesToEven, &LosesInfo);
  எனில் (LosesInfo)
    திரும்பு பொய்;

  எனில் (From.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    திரும்பு உண்மை;

  // Even when NaN representations can be ignored, NaNs must remain NaNs and
  // infinities must remain infinities. Otherwise the conversion can change
  // whether an operation with nnan has poison-producing operands.
  எனில் (IgnoreNaNs) {
    எனில் (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
      திரும்பு To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
    திரும்பு To.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
  }

  எனில் (From.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    // Converting an IEEE signaling NaN to another semantics quiets it, so the
    // original value cannot be recovered by converting it back.
    திரும்பு பொய்;
  }

  // NanOnly formats have no signaling NaNs. IEEE semantics can represent
  // their quiet NaNs; conversions between NanOnly formats are conservatively
  // accepted only when they use the same NaN encoding.
  எனில் (To.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754)
    திரும்பு உண்மை;
  திரும்பு To.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
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
நிலையான constexpr unsigned இலக்கம்_Pow5_Parts_On_அடுக்கு = 16;

நிலையான unsigned முழு powerOfFivePartsForExponent(unsigned முழு power) {
  ப்ரதிஜ்ஞா(power <= maxExponent);

  // 815 / 351 is an upper bound on log2(5).  Reserve an additional part for
  // tcFullMultiply, which writes the full product width even when its most
  // significant part is zero.
  திரும்பு 2 + ((power * 815) / (351 * APFloatBase::integerPartWidth));
}

unsigned முழு APFloatBase::semanticsPrecision(மாறிலி fltSemantics &semantics) {
  திரும்பு semantics.precision;
}
APFloatBase::ExponentType
APFloatBase::semanticsMaxExponent(மாறிலி fltSemantics &semantics) {
  திரும்பு semantics.maxExponent;
}
APFloatBase::ExponentType
APFloatBase::semanticsMinExponent(மாறிலி fltSemantics &semantics) {
  திரும்பு semantics.minExponent;
}
unsigned முழு APFloatBase::semanticsSizeInBits(மாறிலி fltSemantics &semantics) {
  திரும்பு semantics.sizeInBits;
}
unsigned முழு APFloatBase::semanticsIntSizeInBits(மாறிலி fltSemantics &semantics,
                                                 தருக்க isSigned) {
  // The max FP value is pow(2, MaxExponent) * (1 + MaxFraction), so we need
  // at least one more bit than the MaxExponent to hold the max FP value.
  unsigned முழு MinBitWidth = semanticsMaxExponent(semantics) + 1;
  // Extra sign bit needed.
  எனில் (isSigned)
    ++MinBitWidth;
  திரும்பு MinBitWidth;
}

தருக்க APFloatBase::semanticsHasZero(மாறிலி fltSemantics &semantics) {
  திரும்பு semantics.hasZero;
}

தருக்க APFloatBase::semanticsHasSignedRepr(மாறிலி fltSemantics &semantics) {
  திரும்பு semantics.hasSignedRepr;
}

தருக்க APFloatBase::semanticsHasInf(மாறிலி fltSemantics &semantics) {
  திரும்பு semantics.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754;
}

தருக்க APFloatBase::semanticsHasNaN(மாறிலி fltSemantics &semantics) {
  திரும்பு semantics.nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly;
}

தருக்க APFloatBase::isIEEELikeFP(மாறிலி fltSemantics &semantics) {
  // Keep in sync with Type::isIEEELikeFPTy
  திரும்பு SemanticsToEnum(semantics) <= S_IEEEquad;
}

தருக்க APFloatBase::hasSignBitInMSB(மாறிலி fltSemantics &semantics) {
  திரும்பு semantics.hasSignBitInMSB;
}

தருக்க APFloatBase::isRepresentableAsNormalIn(மாறிலி fltSemantics &Src,
                                            மாறிலி fltSemantics &Dst) {
  // Exponent range must be larger.
  எனில் (Src.maxExponent >= Dst.maxExponent || Src.minExponent <= Dst.minExponent)
    திரும்பு பொய்;

  // If the mantissa is long enough, the result value could still be denormal
  // with a larger exponent range.
  //
  // FIXME: This condition is probably not accurate but also shouldn't be a
  // practical concern with existing types.
  திரும்பு Dst.precision >= Src.precision;
}

unsigned APFloatBase::getSizeInBits(மாறிலி fltSemantics &Sem) {
  திரும்பு Sem.sizeInBits;
}

நிலையான constexpr APFloatBase::ExponentType
exponentZero(மாறிலி fltSemantics &semantics) {
  திரும்பு semantics.minExponent - 1;
}

நிலையான constexpr APFloatBase::ExponentType
exponentInf(மாறிலி fltSemantics &semantics) {
  திரும்பு semantics.maxExponent + 1;
}

நிலையான constexpr APFloatBase::ExponentType
exponentNaN(மாறிலி fltSemantics &semantics) {
  எனில் (semantics.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    எனில் (semantics.nanEncoding == fltNanEncoding::NegativeZero)
      திரும்பு exponentZero(semantics);
    எனில் (semantics.hasSignedRepr || semantics.precision > 1)
      திரும்பு semantics.maxExponent;
  }
  திரும்பு semantics.maxExponent + 1;
}

/* A bunch of private, handy routines.  */

நிலையான உள்வரி Error createError(மாறிலி Twine &Err) {
  திரும்பு make_error<StringError>(Err, inconvertibleErrorCode());
}

நிலையான constexpr உள்வரி unsigned முழு part_மொத்தம்_For_Bits(unsigned முழு bits) {
  திரும்பு std::max(1u, (bits + APFloatBase::integerPartWidth - 1) /
                          APFloatBase::integerPartWidth);
}

/* Returns 0U-9U.  Return values >= 10U are not digits.  */
நிலையான உள்வரி unsigned முழு
dec_Digit_மதிப்பு(unsigned முழு c)
{
  திரும்பு c - '0';
}

/* Return the value of a decimal exponent of the form
   [+-]ddddddd.

   If the exponent overflows, returns a large exponent with the
   appropriate sign.  */
நிலையான Expected<முழு> readExponent(StringRef::iterator begin,
                                  StringRef::iterator end) {
  மாறிலி unsigned முழு overlargeExponent = 24000;  /* FIXME.  */
  StringRef::iterator p = begin;

  // Treat no exponent as 0 to match binutils
  எனில் (p == end || ((*p == '-' || *p == '+') && (p + 1) == end))
    திரும்பு 0;

  தருக்க isNegative = *p == '-';
  எனில் (*p == '-' || *p == '+') {
    p++;
    எனில் (p == end)
      திரும்பு createError("Exponent has no digits");
  }

  unsigned absExponent = dec_Digit_மதிப்பு(*p++);
  எனில் (absExponent >= 10U)
    திரும்பு createError("Invalid character in exponent");

  க்கு (; p != end; ++p) {
    unsigned மதிப்பு_2 = dec_Digit_மதிப்பு(*p);
    எனில் (மதிப்பு_2 >= 10U)
      திரும்பு createError("Invalid character in exponent");

    absExponent = absExponent * 10U + மதிப்பு_2;
    எனில் (absExponent >= overlargeExponent) {
      absExponent = overlargeExponent;
      நிறுத்து;
    }
  }

  எனில் (isNegative)
    திரும்பு -(முழு) absExponent;
  இல்லையெனில்
    திரும்பு (முழு) absExponent;
}

/* This is ugly and needs cleaning up, but I don't immediately see
   how whilst remaining safe.  */
நிலையான Expected<முழு> totalExponent(StringRef::iterator p,
                                   StringRef::iterator end,
                                   முழு exponentAdjustment) {
  முழு exponent = 0;

  எனில் (p == end)
    திரும்பு createError("Exponent has no digits");

  தருக்க negative = *p == '-';
  எனில் (*p == '-' || *p == '+') {
    p++;
    எனில் (p == end)
      திரும்பு createError("Exponent has no digits");
  }

  முழு unsignedExponent = 0;
  தருக்க overflow = பொய்;
  க்கு (; p != end; ++p) {
    unsigned முழு மதிப்பு_2;

    மதிப்பு_2 = dec_Digit_மதிப்பு(*p);
    எனில் (மதிப்பு_2 >= 10U)
      திரும்பு createError("Invalid character in exponent");

    unsignedExponent = unsignedExponent * 10 + மதிப்பு_2;
    எனில் (unsignedExponent > 32767) {
      overflow = உண்மை;
      நிறுத்து;
    }
  }

  எனில் (exponentAdjustment > 32767 || exponentAdjustment < -32768)
    overflow = உண்மை;

  எனில் (!overflow) {
    exponent = unsignedExponent;
    எனில் (negative)
      exponent = -exponent;
    exponent += exponentAdjustment;
    எனில் (exponent > 32767 || exponent < -32768)
      overflow = உண்மை;
  }

  எனில் (overflow)
    exponent = negative ? -32768: 32767;

  திரும்பு exponent;
}

நிலையான Expected<StringRef::iterator>
skipLeadingZeroesAndAnyDot(StringRef::iterator begin, StringRef::iterator end,
                           StringRef::iterator *dot) {
  StringRef::iterator p = begin;
  *dot = end;
  போது (p != end && *p == '0')
    p++;

  எனில் (p != end && *p == '.') {
    *dot = p++;

    எனில் (end - begin == 1)
      திரும்பு createError("Significand has no digits");

    போது (p != end && *p == '0')
      p++;
  }

  திரும்பு p;
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
கட்டமைப்பு decimalInfo {
  மாறிலி எழுத்து *firstSigDigit;
  மாறிலி எழுத்து *lastSigDigit;
  முழு exponent;
  முழு normalizedExponent;
};

நிலையான Error interpretDecimal(StringRef::iterator begin,
                              StringRef::iterator end, decimalInfo *D) {
  StringRef::iterator dot = end;

  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  எனில் (!PtrOrErr)
    திரும்பு PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;

  D->firstSigDigit = p;
  D->exponent = 0;
  D->normalizedExponent = 0;

  க்கு (; p != end; ++p) {
    எனில் (*p == '.') {
      எனில் (dot != end)
        திரும்பு createError("String contains multiple dots");
      dot = p++;
      எனில் (p == end)
        நிறுத்து;
    }
    எனில் (dec_Digit_மதிப்பு(*p) >= 10U)
      நிறுத்து;
  }

  எனில் (p != end) {
    எனில் (*p != 'e' && *p != 'E')
      திரும்பு createError("Invalid character in significand");
    எனில் (p == begin)
      திரும்பு createError("Significand has no digits");
    எனில் (dot != end && p - begin == 1)
      திரும்பு createError("Significand has no digits");

    /* p points to the first non-digit in the string */
    auto ExpOrErr = readExponent(p + 1, end);
    எனில் (!ExpOrErr)
      திரும்பு ExpOrErr.takeError();
    D->exponent = *ExpOrErr;

    /* Implied decimal point?  */
    எனில் (dot == end)
      dot = p;
  }

  /* If number is all zeroes accept any exponent.  */
  எனில் (p != D->firstSigDigit) {
    /* Drop insignificant trailing zeroes.  */
    எனில் (p != begin) {
      செய்
        செய்
          p--;
        போது (p != begin && *p == '0');
      போது (p != begin && *p == '.');
    }

    /* Adjust the exponents for any decimal point.  */
    D->exponent += static_cast<APFloat::ExponentType>((dot - p) - (dot > p));
    D->normalizedExponent = (D->exponent +
              static_cast<APFloat::ExponentType>((p - D->firstSigDigit)
                                      - (dot > D->firstSigDigit && dot < p)));
  }

  D->lastSigDigit = p;
  திரும்பு Error::success();
}

/* Return the trailing fraction of a hexadecimal number.
   DIGITVALUE is the first hex digit of the fraction, P points to
   the next digit.  */
நிலையான Expected<lostFraction>
trailingHexadecimalFraction(StringRef::iterator p, StringRef::iterator end,
                            unsigned முழு digit_மதிப்பு) {
  /* If the first trailing digit isn't 0 or 8 we can work out the
     fraction immediately.  */
  எனில் (digit_மதிப்பு > 8)
    திரும்பு lfMoreThanHalf;
  இல்லையெனில் எனில் (digit_மதிப்பு < 8 && digit_மதிப்பு > 0)
    திரும்பு lfLessThanHalf;

  // Otherwise we need to find the first non-zero digit.
  போது (p != end && (*p == '0' || *p == '.'))
    p++;

  எனில் (p == end)
    திரும்பு createError("Invalid trailing hexadecimal fraction!");

  unsigned hexDigit = hex_Digit_மதிப்பு(*p);

  /* If we ran off the end it is exactly zero or one-half, otherwise
     a little more.  */
  எனில் (hexDigit == UINT_MAX)
    திரும்பு digit_மதிப்பு == 0 ? lfExactlyZero: lfExactlyHalf;
  இல்லையெனில்
    திரும்பு digit_மதிப்பு == 0 ? lfLessThanHalf: lfMoreThanHalf;
}

/* Return the fraction lost were a bignum truncated losing the least
   significant BITS bits.  */
நிலையான lostFraction
lostFractionThroughTruncation(மாறிலி APFloatBase::integerPart *parts,
                              unsigned முழு part_மொத்தம்,
                              unsigned முழு bits)
{
  unsigned lsb = APInt::tcLSB(parts, part_மொத்தம்);

  /* Note this is guaranteed true if bits == 0, or LSB == UINT_MAX.  */
  எனில் (bits <= lsb)
    திரும்பு lfExactlyZero;
  எனில் (bits == lsb + 1)
    திரும்பு lfExactlyHalf;
  எனில் (bits <= part_மொத்தம் * APFloatBase::integerPartWidth &&
      APInt::tcExtractBit(parts, bits - 1))
    திரும்பு lfMoreThanHalf;

  திரும்பு lfLessThanHalf;
}

/* Shift DST right BITS bits noting lost fraction.  */
நிலையான lostFraction
shift_வலது(APFloatBase::integerPart *dst, unsigned முழு parts, unsigned முழு bits)
{
  lostFraction lost_fraction = lostFractionThroughTruncation(dst, parts, bits);

  APInt::tc_Shift_வலது(dst, parts, bits);

  திரும்பு lost_fraction;
}

/* Combine the effect of two lost fractions.  */
நிலையான lostFraction
combineLostFractions(lostFraction moreSignificant,
                     lostFraction lessSignificant)
{
  எனில் (lessSignificant != lfExactlyZero) {
    எனில் (moreSignificant == lfExactlyZero)
      moreSignificant = lfLessThanHalf;
    இல்லையெனில் எனில் (moreSignificant == lfExactlyHalf)
      moreSignificant = lfMoreThanHalf;
  }

  திரும்பு moreSignificant;
}

/* The error from the true value, in half-ulps, on multiplying two
   floating point numbers, which differ from the value they
   approximate by at most HUE1 and HUE2 half-ulps, is strictly less
   than the returned value.

   See "How to Read Floating Point Numbers Accurately" by William D
   Clinger.  */
நிலையான unsigned முழு
HUerrBound(தருக்க inexactMultiply, unsigned முழு HUerr1, unsigned முழு HUerr2)
{
  ப்ரதிஜ்ஞா(HUerr1 < 2 || HUerr2 < 2 || (HUerr1 + HUerr2 < 8));

  எனில் (HUerr1 + HUerr2 == 0)
    திரும்பு inexactMultiply * 2;  /* <= inexactMultiply half-ulps.  */
  இல்லையெனில்
    திரும்பு inexactMultiply + 2 * (HUerr1 + HUerr2);
}

/* The number of ulps from the boundary (zero, or half if ISNEAREST)
   when the least significant BITS are truncated.  BITS cannot be
   zero.  */
நிலையான APFloatBase::integerPart
ulpsFromBoundary(மாறிலி APFloatBase::integerPart *parts, unsigned முழு bits,
                 தருக்க isNearest) {
  ப்ரதிஜ்ஞா(bits != 0);

  bits--;
  unsigned மொத்தம் = bits / APFloatBase::integerPartWidth;
  unsigned partBits = bits % APFloatBase::integerPartWidth + 1;

  APFloatBase::integerPart part =
      parts[மொத்தம்] & (~(APFloatBase::integerPart)0 >>
                      (APFloatBase::integerPartWidth - partBits));

  APFloatBase::integerPart boundary;
  எனில் (isNearest)
    boundary = (APFloatBase::integerPart) 1 << (partBits - 1);
  இல்லையெனில்
    boundary = 0;

  எனில் (மொத்தம் == 0) {
    எனில் (part - boundary <= boundary - part)
      திரும்பு part - boundary;
    இல்லையெனில்
      திரும்பு boundary - part;
  }

  எனில் (part == boundary) {
    போது (--மொத்தம்)
      எனில் (parts[மொத்தம்])
        திரும்பு ~(APFloatBase::integerPart) 0; /* A lot.  */

    திரும்பு parts[0];
  } இல்லையெனில் எனில் (part == boundary - 1) {
    போது (--மொத்தம்)
      எனில் (~parts[மொத்தம்])
        திரும்பு ~(APFloatBase::integerPart) 0; /* A lot.  */

    திரும்பு -parts[0];
  }

  திரும்பு ~(APFloatBase::integerPart) 0; /* A lot.  */
}

நிலையான constexpr unsigned PowerOfFivePartCounts[] = {1,  1,  2,  3,   5,  10,
                                                     19, 38, 75, 149, 298};

நிலையான constexpr APFloatBase::integerPart PowerOfFiveParts[] = {
#அந்தர்பாவ "APFloatPowerOfFiveTable.inc"
};

static_assert([] {
  unsigned Sum = 0;
  க்கு (unsigned மொத்தம்_2 : PowerOfFivePartCounts)
    Sum += மொத்தம்_2;
  திரும்பு Sum;
}() == std::size(PowerOfFiveParts));

/* Place pow(5, power) in DST, and return the number of parts used.
   DST must be at least one part larger than size of the answer.  */
நிலையான unsigned முழு
powerOf5(APFloatBase::integerPart *dst, unsigned முழு power) {
  நிலையான மாறிலி APFloatBase::integerPart firstEightPowers[] = {
      1, 5, 25, 125, 625, 3125, 15625, 78125};

  SmallVector<APFloatBase::integerPart, இலக்கம்_Pow5_Parts_On_அடுக்கு> scratch;
  scratch.resize_for_overwrite(powerOfFivePartsForExponent(power));

  APFloatBase::integerPart *p1, *p2;
  மாறிலி APFloatBase::integerPart *pow5;

  p1 = dst;
  p2 = scratch.data();

  *p1 = firstEightPowers[power & 7];
  power >>= 3;

  unsigned result = 1;
  pow5 = PowerOfFiveParts;

  க்கு (unsigned முழு n = 0; power; power >>= 1, n++) {
    unsigned parts_மொத்தம் = PowerOfFivePartCounts[n];

    எனில் (power & 1) {
      APFloatBase::integerPart *tmp;

      APInt::tcFullMultiply(p2, p1, pow5, result, parts_மொத்தம்);
      result += parts_மொத்தம்;
      எனில் (p2[result - 1] == 0)
        result--;

      /* Now result is in p1 with partsCount parts and p2 is scratch
         space.  */
      tmp = p1;
      p1 = p2;
      p2 = tmp;
    }

    pow5 += parts_மொத்தம்;
  }

  எனில் (p1 != dst)
    APInt::tcAssign(dst, p1, result);

  திரும்பு result;
}

/* Zero at the end to avoid modular arithmetic when adding one; used
   when rounding up during hexadecimal output.  */
நிலையான மாறிலி எழுத்து hexDigitsLower[] = "0123456789abcdef0";
நிலையான மாறிலி எழுத்து hexDigitsUpper[] = "0123456789ABCDEF0";
நிலையான மாறிலி எழுத்து infinityL[] = "infinity";
நிலையான மாறிலி எழுத்து infinityU[] = "INFINITY";
நிலையான மாறிலி எழுத்து NaNL[] = "nan";
நிலையான மாறிலி எழுத்து NaNU[] = "NAN";

/* Write out an integerPart in hexadecimal, starting with the most
   significant nibble.  Write out exactly COUNT hexdigits, return
   COUNT.  */
நிலையான unsigned முழு
partAsHex (எழுத்து *dst, APFloatBase::integerPart part, unsigned முழு மொத்தம்,
           மாறிலி எழுத்து *hexDigitChars)
{
  unsigned முழு result = மொத்தம்;

  ப்ரதிஜ்ஞா(மொத்தம் != 0 && மொத்தம் <= APFloatBase::integerPartWidth / 4);

  part >>= (APFloatBase::integerPartWidth - 4 * மொத்தம்);
  போது (மொத்தம்--) {
    dst[மொத்தம்] = hexDigitChars[part & 0xf];
    part >>= 4;
  }

  திரும்பு result;
}

/* Write out an unsigned decimal integer.  */
நிலையான எழுத்து *writeUnsignedDecimal(எழுத்து *dst, unsigned முழு n) {
  எழுத்து buff[40], *p;

  p = buff;
  செய்
    *p++ = '0' + n % 10;
  போது (n /= 10);

  செய்
    *dst++ = *--p;
  போது (p != buff);

  திரும்பு dst;
}

/* Write out a signed decimal integer.  */
நிலையான எழுத்து *writeSignedDecimal(எழுத்து *dst, முழு மதிப்பு_2) {
  எனில் (மதிப்பு_2 < 0) {
    *dst++ = '-';
    dst = writeUnsignedDecimal(dst, -(unsigned) மதிப்பு_2);
  } இல்லையெனில் {
    dst = writeUnsignedDecimal(dst, மதிப்பு_2);
  }

  திரும்பு dst;
}

// Compute the ULP of the input using a definition from:
// Jean-Michel Muller. On the definition of ulp(x). [Research Report] RR-5504,
// LIP RR-2005-09, INRIA, LIP. 2005, pp.16. inria-00070503
நிலையான APFloat harrisonUlp(மாறிலி APFloat &X) {
  மாறிலி fltSemantics &Sem = X.getSemantics();
  தேர்வு (X.getCategory()) {
  நிலை APFloat::fcNaN:
    திரும்பு APFloat::getQNaN(Sem);
  நிலை APFloat::fcInfinity:
    திரும்பு APFloat::getInf(Sem);
  நிலை APFloat::fcZero:
    திரும்பு APFloat::getSmallest(Sem);
  நிலை APFloat::fcNormal:
    நிறுத்து;
  }
  எனில் (X.isDenormal() || X.isSmallestNormalized())
    திரும்பு APFloat::getSmallest(Sem);
  முழு Exp = ilogb(X);
  எனில் (X.getExactLog2() != INT_MIN)
    Exp -= 1;
  திரும்பு scalbn(APFloat::getOne(Sem), Exp - (Sem.precision - 1),
                APFloat::rmNearestTiesToEven);
}

நாமாகாஶ detail {
/* Constructors.  */
வெறுமை IEEEFloat::initialize(மாறிலி fltSemantics *ourSemantics) {
  semantics = ourSemantics;
  unsigned மொத்தம் = part_மொத்தம்();
  எனில் (மொத்தம் > 1)
    significand.parts = புதிய integerPart[மொத்தம்];
}

வெறுமை IEEEFloat::freeSignificand() {
  எனில் (needsCleanup())
    delete [] significand.parts;
}

வெறுமை IEEEFloat::assign(மாறிலி IEEEFloat &rhs) {
  ப்ரதிஜ்ஞா(semantics == rhs.semantics);

  sign = rhs.sign;
  category = rhs.category;
  exponent = rhs.exponent;
  எனில் (isFiniteNonZero() || category == fcNaN)
    copySignificand(rhs);
}

வெறுமை IEEEFloat::copySignificand(மாறிலி IEEEFloat &rhs) {
  ப்ரதிஜ்ஞா(isFiniteNonZero() || category == fcNaN);
  ப்ரதிஜ்ஞா(rhs.part_மொத்தம்() >= part_மொத்தம்());

  APInt::tcAssign(significandParts(), rhs.significandParts(),
                  part_மொத்தம்());
}

/* Make this number a NaN, with an arbitrary but deterministic value
   for the significand.  If double or longer, this is a signalling NaN,
   which may not be ideal.  If float, this is QNaN(0).  */
வெறுமை IEEEFloat::makeNaN(தருக்க SNaN, தருக்க Negative, மாறிலி APInt *fill) {
  எனில் (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support NaN");

  எனில் (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  category = fcNaN;
  sign = Negative;
  exponent = exponentNaN();

  integerPart *significand = significandParts();
  unsigned இலக்கம்_Parts = part_மொத்தம்();

  APInt fill_storage;
  எனில் (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // Finite-only types do not distinguish signalling and quiet NaN, so
    // make them all signalling.
    SNaN = பொய்;
    எனில் (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
      sign = உண்மை;
      fill_storage = APInt::getZero(semantics->precision - 1);
    } இல்லையெனில் {
      fill_storage = APInt::getAllOnes(semantics->precision - 1);
    }
    fill = &fill_storage;
  }

  // Set the significand bits to the fill.
  எனில் (!fill || fill->get_இலக்கம்_சொற்கள்() < இலக்கம்_Parts)
    APInt::tcSet(significand, 0, இலக்கம்_Parts);
  எனில் (fill) {
    APInt::tcAssign(significand, fill->getRawData(),
                    std::min(fill->get_இலக்கம்_சொற்கள்(), இலக்கம்_Parts));

    // Zero out the excess bits of the significand.
    unsigned bitsToPreserve = semantics->precision - 1;
    unsigned part = bitsToPreserve / 64;
    bitsToPreserve %= 64;
    significand[part] &= ((1ULL << bitsToPreserve) - 1);
    க்கு (part++; part != இலக்கம்_Parts; ++part)
      significand[part] = 0;
  }

  unsigned QNaNBit =
      (semantics->precision >= 2) ? (semantics->precision - 2) : 0;

  எனில் (SNaN) {
    // We always have to clear the QNaN bit to make it an SNaN.
    APInt::tcClearBit(significand, QNaNBit);

    // If there are no bits set in the payload, we have to set
    // *something* to make it a NaN instead of an infinity;
    // conventionally, this is the next bit down from the QNaN bit.
    எனில் (APInt::tcIsZero(significand, இலக்கம்_Parts))
      APInt::tcSetBit(significand, QNaNBit - 1);
  } இல்லையெனில் எனில் (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // The only NaN is a quiet NaN, and it has no bits sets in the significand.
    // Do nothing.
  } இல்லையெனில் {
    // We always have to set the QNaN bit to make it a QNaN.
    APInt::tcSetBit(significand, QNaNBit);
  }

  // For x87 extended precision, we want to make a NaN, not a
  // pseudo-NaN.  Maybe we should expose the ability to make
  // pseudo-NaNs?
  எனில் (semantics == &APFloatBase::semX87DoubleExtended)
    APInt::tcSetBit(significand, QNaNBit + 1);
}

IEEEFloat &IEEEFloat::operator=(மாறிலி IEEEFloat &rhs) {
  எனில் (இது != &rhs) {
    எனில் (semantics != rhs.semantics) {
      freeSignificand();
      initialize(rhs.semantics);
    }
    assign(rhs);
  }

  திரும்பு *இது;
}

IEEEFloat &IEEEFloat::operator=(IEEEFloat &&rhs) {
  freeSignificand();

  semantics = rhs.semantics;
  significand = rhs.significand;
  exponent = rhs.exponent;
  category = rhs.category;
  sign = rhs.sign;

  rhs.semantics = &APFloatBase::semBogus;
  திரும்பு *இது;
}

தருக்க IEEEFloat::isDenormal() மாறிலி {
  திரும்பு getSemantics().hasDenormals && isFiniteNonZero() &&
         (exponent == semantics->minExponent) &&
         (APInt::tcExtractBit(significandParts(), semantics->precision - 1) ==
          0);
}

தருக்க IEEEFloat::isSmallest() மாறிலி {
  // The smallest number by magnitude in our format will be the smallest
  // denormal, i.e. the floating point number with exponent being minimum
  // exponent and significand bitwise equal to 1 (i.e. with MSB equal to 0).
  திரும்பு isFiniteNonZero() && exponent == semantics->minExponent &&
    significandMSB() == 0;
}

தருக்க IEEEFloat::isSmallestNormalized() மாறிலி {
  திரும்பு getCategory() == fcNormal && exponent == semantics->minExponent &&
         isSignificandAllZerosExceptMSB();
}

unsigned முழு IEEEFloat::get_இலக்கம்_உயர்வு_Bits() மாறிலி {
  மாறிலி unsigned முழு Part_மொத்தம் = part_மொத்தம்_For_Bits(semantics->precision);
  மாறிலி unsigned முழு Bits = Part_மொத்தம் * integerPartWidth;

  // Compute how many bits are used in the final word.
  // When precision is just 1, it represents the 'Pth'
  // Precision bit and not the actual significand bit.
  மாறிலி unsigned முழு இலக்கம்_உயர்வு_Bits = (semantics->precision > 1)
                                       ? (Bits - semantics->precision + 1)
                                       : (Bits - semantics->precision);
  திரும்பு இலக்கம்_உயர்வு_Bits;
}

தருக்க IEEEFloat::isSignificandAllOnes() மாறிலி {
  // Test if the significand excluding the integral bit is all ones. This allows
  // us to test for binade boundaries.
  மாறிலி integerPart *Parts = significandParts();
  மாறிலி unsigned Part_மொத்தம் = part_மொத்தம்_For_Bits(semantics->precision);
  க்கு (unsigned i = 0; i < Part_மொத்தம் - 1; i++)
    எனில் (~Parts[i])
      திரும்பு பொய்;

  // Set the unused high bits to all ones when we compare.
  மாறிலி unsigned இலக்கம்_உயர்வு_Bits = get_இலக்கம்_உயர்வு_Bits();
  ப்ரதிஜ்ஞா(இலக்கம்_உயர்வு_Bits <= integerPartWidth && இலக்கம்_உயர்வு_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  மாறிலி integerPart உயர்வு_Bit_Fill =
    ~integerPart(0) << (integerPartWidth - இலக்கம்_உயர்வு_Bits);
  எனில் ((semantics->precision <= 1) || (~(Parts[Part_மொத்தம் - 1] | உயர்வு_Bit_Fill)))
    திரும்பு பொய்;

  திரும்பு உண்மை;
}

தருக்க IEEEFloat::isSignificandAllOnesExceptLSB() மாறிலி {
  // Test if the significand excluding the integral bit is all ones except for
  // the least significant bit.
  மாறிலி integerPart *Parts = significandParts();

  எனில் (Parts[0] & 1)
    திரும்பு பொய்;

  மாறிலி unsigned Part_மொத்தம் = part_மொத்தம்_For_Bits(semantics->precision);
  க்கு (unsigned i = 0; i < Part_மொத்தம் - 1; i++) {
    எனில் (~Parts[i] & ~unsigned{!i})
      திரும்பு பொய்;
  }

  // Set the unused high bits to all ones when we compare.
  மாறிலி unsigned இலக்கம்_உயர்வு_Bits = get_இலக்கம்_உயர்வு_Bits();
  ப்ரதிஜ்ஞா(இலக்கம்_உயர்வு_Bits <= integerPartWidth && இலக்கம்_உயர்வு_Bits > 0 &&
         "Can not have more high bits to fill than integerPartWidth");
  மாறிலி integerPart உயர்வு_Bit_Fill = ~integerPart(0)
                                  << (integerPartWidth - இலக்கம்_உயர்வு_Bits);
  எனில் (~(Parts[Part_மொத்தம் - 1] | உயர்வு_Bit_Fill | 0x1))
    திரும்பு பொய்;

  திரும்பு உண்மை;
}

தருக்க IEEEFloat::isSignificandAllZeros() மாறிலி {
  // Test if the significand excluding the integral bit is all zeros. This
  // allows us to test for binade boundaries.
  மாறிலி integerPart *Parts = significandParts();
  மாறிலி unsigned Part_மொத்தம் = part_மொத்தம்_For_Bits(semantics->precision);

  க்கு (unsigned i = 0; i < Part_மொத்தம் - 1; i++)
    எனில் (Parts[i])
      திரும்பு பொய்;

  // Compute how many bits are used in the final word.
  மாறிலி unsigned இலக்கம்_உயர்வு_Bits = get_இலக்கம்_உயர்வு_Bits();
  ப்ரதிஜ்ஞா(இலக்கம்_உயர்வு_Bits < integerPartWidth && "Can not have more high bits to "
         "clear than integerPartWidth");
  மாறிலி integerPart உயர்வு_Bit_Mask = ~integerPart(0) >> இலக்கம்_உயர்வு_Bits;

  எனில் ((semantics->precision > 1) && (Parts[Part_மொத்தம் - 1] & உயர்வு_Bit_Mask))
    திரும்பு பொய்;

  திரும்பு உண்மை;
}

தருக்க IEEEFloat::isSignificandAllZerosExceptMSB() மாறிலி {
  மாறிலி integerPart *Parts = significandParts();
  மாறிலி unsigned Part_மொத்தம் = part_மொத்தம்_For_Bits(semantics->precision);

  க்கு (unsigned i = 0; i < Part_மொத்தம் - 1; i++) {
    எனில் (Parts[i])
      திரும்பு பொய்;
  }

  மாறிலி unsigned இலக்கம்_உயர்வு_Bits = get_இலக்கம்_உயர்வு_Bits();
  மாறிலி integerPart MSBMask = integerPart(1)
                              << (integerPartWidth - இலக்கம்_உயர்வு_Bits);
  திரும்பு ((semantics->precision <= 1) || (Parts[Part_மொத்தம் - 1] == MSBMask));
}

தருக்க IEEEFloat::isLargest() மாறிலி {
  தருக்க IsMaxExp = isFiniteNonZero() && exponent == semantics->maxExponent;
  எனில் (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes) {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones except
    // the LSB.
    திரும்பு (IsMaxExp && APFloat::hasSignificand(*semantics))
               ? isSignificandAllOnesExceptLSB()
               : IsMaxExp;
  } இல்லையெனில் {
    // The largest number by magnitude in our format will be the floating point
    // number with maximum exponent and with significand that is all ones.
    திரும்பு IsMaxExp && isSignificandAllOnes();
  }
}

தருக்க IEEEFloat::isInteger() மாறிலி {
  // This could be made more efficient; I'm going for obviously correct.
  எனில் (!isFinite()) திரும்பு பொய்;
  IEEEFloat truncated = *இது;
  truncated.roundToIntegral(rmTowardZero);
  திரும்பு compare(truncated) == cmpEqual;
}

தருக்க IEEEFloat::bitwiseIsEqual(மாறிலி IEEEFloat &rhs) மாறிலி {
  எனில் (இது == &rhs)
    திரும்பு உண்மை;
  எனில் (semantics != rhs.semantics ||
      category != rhs.category ||
      sign != rhs.sign)
    திரும்பு பொய்;
  எனில் (category==fcZero || category==fcInfinity)
    திரும்பு உண்மை;

  எனில் (isFiniteNonZero() && exponent != rhs.exponent)
    திரும்பு பொய்;

  திரும்பு std::equal(significandParts(), significandParts() + part_மொத்தம்(),
                    rhs.significandParts());
}

IEEEFloat::IEEEFloat(மாறிலி fltSemantics &ourSemantics, integerPart மதிப்பு_2) {
  initialize(&ourSemantics);
  sign = 0;
  category = fcNormal;
  zeroSignificand();
  exponent = ourSemantics.precision - 1;
  significandParts()[0] = மதிப்பு_2;
  normalize(rmNearestTiesToEven, lfExactlyZero);
}

IEEEFloat::IEEEFloat(மாறிலி fltSemantics &ourSemantics) {
  initialize(&ourSemantics);
  // The Float8E8MOFNU format does not have a representation
  // for zero. So, use the closest representation instead.
  // Moreover, the all-zero encoding represents a valid
  // normal value (which is the smallestNormalized here).
  // Hence, we call makeSmallestNormalized (where category is
  // 'fcNormal') instead of makeZero (where category is 'fcZero').
  ourSemantics.hasZero ? makeZero(பொய்) : makeSmallestNormalized(பொய்);
}

// Delegate to the previous constructor, because later copy constructor may
// actually inspects category, which can't be garbage.
IEEEFloat::IEEEFloat(மாறிலி fltSemantics &ourSemantics, uninitializedTag tag)
    : IEEEFloat(ourSemantics) {}

IEEEFloat::IEEEFloat(மாறிலி IEEEFloat &rhs) {
  initialize(rhs.semantics);
  assign(rhs);
}

IEEEFloat::IEEEFloat(IEEEFloat &&rhs) : semantics(&APFloatBase::semBogus) {
  *இது = std::move(rhs);
}

IEEEFloat::~IEEEFloat() { freeSignificand(); }

unsigned முழு IEEEFloat::part_மொத்தம்() மாறிலி {
  திரும்பு part_மொத்தம்_For_Bits(semantics->precision + 1);
}

மாறிலி APFloat::integerPart *IEEEFloat::significandParts() மாறிலி {
  திரும்பு const_cast<IEEEFloat *>(இது)->significandParts();
}

APFloat::integerPart *IEEEFloat::significandParts() {
  எனில் (part_மொத்தம்() > 1)
    திரும்பு significand.parts;
  இல்லையெனில்
    திரும்பு &significand.part;
}

வெறுமை IEEEFloat::zeroSignificand() {
  APInt::tcSet(significandParts(), 0, part_மொத்தம்());
}

/* Increment an fcNormal floating point number's significand.  */
வெறுமை IEEEFloat::incrementSignificand() {
  [[maybe_unused]] integerPart carry =
      APInt::tcIncrement(significandParts(), part_மொத்தம்());

  /* Our callers should never cause us to overflow.  */
  ப்ரதிஜ்ஞா(carry == 0);
}

/* Add the significand of the RHS.  Returns the carry flag.  */
APFloat::integerPart IEEEFloat::addSignificand(மாறிலி IEEEFloat &rhs) {
  integerPart *parts = significandParts();

  ப்ரதிஜ்ஞா(semantics == rhs.semantics);
  ப்ரதிஜ்ஞா(exponent == rhs.exponent);

  திரும்பு APInt::tcAdd(parts, rhs.significandParts(), 0, part_மொத்தம்());
}

/* Subtract the significand of the RHS with a borrow flag.  Returns
   the borrow flag.  */
APFloat::integerPart IEEEFloat::subtractSignificand(மாறிலி IEEEFloat &rhs,
                                                    integerPart borrow) {
  integerPart *parts = significandParts();

  ப்ரதிஜ்ஞா(semantics == rhs.semantics);
  ப்ரதிஜ்ஞா(exponent == rhs.exponent);

  திரும்பு APInt::tcSubtract(parts, rhs.significandParts(), borrow,
                           part_மொத்தம்());
}

/* Multiply the significand of the RHS.  If ADDEND is non-NULL, add it
   on to the full-precision result of the multiplication.  Returns the
   lost fraction.  */
lostFraction IEEEFloat::multiplySignificand(மாறிலி IEEEFloat &rhs,
                                            IEEEFloat addend,
                                            தருக்க ignoreAddend) {
  integerPart scratch[4];
  தருக்க ignored;

  ப்ரதிஜ்ஞா(semantics == rhs.semantics);

  unsigned precision = semantics->precision;

  // Allocate space for twice as many bits as the original significand, plus one
  // extra bit for the addition to overflow into.
  unsigned new_Parts_மொத்தம் = part_மொத்தம்_For_Bits(precision * 2 + 1);

  // FIXME: Replace with SmallVector<4>.
  integerPart *fullSignificand =
      new_Parts_மொத்தம் > 4 ? புதிய integerPart[new_Parts_மொத்தம்] : scratch;

  integerPart *lhsSignificand = significandParts();
  unsigned parts_மொத்தம் = part_மொத்தம்();

  APInt::tcFullMultiply(fullSignificand, lhsSignificand,
                        rhs.significandParts(), parts_மொத்தம், parts_மொத்தம்);

  lostFraction lost_fraction = lfExactlyZero;
  // One, not zero, based MSB.
  unsigned omsb = APInt::tcMSB(fullSignificand, new_Parts_மொத்தம்) + 1;
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

  எனில் (!ignoreAddend && addend.isNonZero()) {
    // The intermediate result of the multiplication has "2 * precision"
    // signicant bit; adjust the addend to be consistent with mul result.
    //
    Significand savedSignificand = significand;
    மாறிலி fltSemantics *savedSemantics = semantics;

    // Normalize our MSB to one below the top bit to allow for overflow.
    unsigned extendedPrecision = 2 * precision + 1;
    எனில் (omsb != extendedPrecision - 1) {
      ப்ரதிஜ்ஞா(extendedPrecision > omsb);
      APInt::tc_Shift_இடது(fullSignificand, new_Parts_மொத்தம்,
                         (extendedPrecision - 1) - omsb);
      exponent -= (extendedPrecision - 1) - omsb;
    }

    /* Create new semantics.  */
    fltSemantics extendedSemantics = *semantics;
    extendedSemantics.precision = extendedPrecision;

    எனில் (new_Parts_மொத்தம் == 1)
      significand.part = fullSignificand[0];
    இல்லையெனில்
      significand.parts = fullSignificand;
    semantics = &extendedSemantics;

    // Make a copy so we can convert it to the extended semantics.
    // Note that we cannot convert the addend directly, as the extendedSemantics
    // is a local variable (which we take a reference to).
    IEEEFloat extendedAddend(addend);
    [[maybe_unused]] opStatus status = extendedAddend.convert(
        extendedSemantics, APFloat::rmTowardZero, &ignored);
    ப்ரதிஜ்ஞா(status == APFloat::opOK);

    // Shift the significand of the addend right by one bit. This guarantees
    // that the high bit of the significand is zero (same as fullSignificand),
    // so the addition will overflow (if it does overflow at all) into the top bit.
    lost_fraction = extendedAddend.shift_Significand_வலது(1);
    ப்ரதிஜ்ஞா(lost_fraction == lfExactlyZero &&
           "Lost precision while shifting addend for fused-multiply-add.");

    lost_fraction = addOrSubtractSignificand(extendedAddend, பொய்);

    /* Restore our state.  */
    எனில் (new_Parts_மொத்தம் == 1)
      fullSignificand[0] = significand.part;
    significand = savedSignificand;
    semantics = savedSemantics;

    omsb = APInt::tcMSB(fullSignificand, new_Parts_மொத்தம்) + 1;
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
  எனில் (omsb > precision) {
    unsigned முழு bits, significantParts;
    lostFraction lf;

    bits = omsb - precision;
    significantParts = part_மொத்தம்_For_Bits(omsb);
    lf = shift_வலது(fullSignificand, significantParts, bits);
    lost_fraction = combineLostFractions(lf, lost_fraction);
    exponent += bits;
  }

  APInt::tcAssign(lhsSignificand, fullSignificand, parts_மொத்தம்);

  எனில் (new_Parts_மொத்தம் > 4)
    delete [] fullSignificand;

  திரும்பு lost_fraction;
}

lostFraction IEEEFloat::multiplySignificand(மாறிலி IEEEFloat &rhs) {
  // When the given semantics has zero, the addend here is a zero.
  // i.e . it belongs to the 'fcZero' category.
  // But when the semantics does not support zero, we need to
  // explicitly convey that this addend should be ignored
  // for multiplication.
  திரும்பு multiplySignificand(rhs, IEEEFloat(*semantics), !semantics->hasZero);
}

/* Multiply the significands of LHS and RHS to DST.  */
lostFraction IEEEFloat::divideSignificand(மாறிலி IEEEFloat &rhs) {
  integerPart scratch[4];

  ப்ரதிஜ்ஞா(semantics == rhs.semantics);

  integerPart *lhsSignificand = significandParts();
  மாறிலி integerPart *rhsSignificand = rhs.significandParts();
  unsigned parts_மொத்தம் = part_மொத்தம்();

  integerPart *dividend =
      parts_மொத்தம் > 2 ? புதிய integerPart[parts_மொத்தம் * 2] : scratch;
  integerPart *வகுஎண் = dividend + parts_மொத்தம்;

  /* Copy the dividend and divisor as they will be modified in-place.  */
  க்கு (unsigned i = 0; i < parts_மொத்தம்; i++) {
    dividend[i] = lhsSignificand[i];
    வகுஎண்[i] = rhsSignificand[i];
    lhsSignificand[i] = 0;
  }

  exponent -= rhs.exponent;

  unsigned முழு precision = semantics->precision;

  /* Normalize the divisor.  */
  unsigned bit = precision - APInt::tcMSB(வகுஎண், parts_மொத்தம்) - 1;
  எனில் (bit) {
    exponent += bit;
    APInt::tc_Shift_இடது(வகுஎண், parts_மொத்தம், bit);
  }

  /* Normalize the dividend.  */
  bit = precision - APInt::tcMSB(dividend, parts_மொத்தம்) - 1;
  எனில் (bit) {
    exponent -= bit;
    APInt::tc_Shift_இடது(dividend, parts_மொத்தம், bit);
  }

  /* Ensure the dividend >= divisor initially for the loop below.
     Incidentally, this means that the division loop below is
     guaranteed to set the integer bit to one.  */
  எனில் (APInt::tcCompare(dividend, வகுஎண், parts_மொத்தம்) < 0) {
    exponent--;
    APInt::tc_Shift_இடது(dividend, parts_மொத்தம், 1);
    ப்ரதிஜ்ஞா(APInt::tcCompare(dividend, வகுஎண், parts_மொத்தம்) >= 0);
  }

  /* Long division.  */
  க்கு (bit = precision; bit; bit -= 1) {
    எனில் (APInt::tcCompare(dividend, வகுஎண், parts_மொத்தம்) >= 0) {
      APInt::tcSubtract(dividend, வகுஎண், 0, parts_மொத்தம்);
      APInt::tcSetBit(lhsSignificand, bit - 1);
    }

    APInt::tc_Shift_இடது(dividend, parts_மொத்தம், 1);
  }

  /* Figure out the lost fraction.  */
  முழு cmp = APInt::tcCompare(dividend, வகுஎண், parts_மொத்தம்);

  lostFraction lost_fraction;
  எனில் (cmp > 0)
    lost_fraction = lfMoreThanHalf;
  இல்லையெனில் எனில் (cmp == 0)
    lost_fraction = lfExactlyHalf;
  இல்லையெனில் எனில் (APInt::tcIsZero(dividend, parts_மொத்தம்))
    lost_fraction = lfExactlyZero;
  இல்லையெனில்
    lost_fraction = lfLessThanHalf;

  எனில் (parts_மொத்தம் > 2)
    delete [] dividend;

  திரும்பு lost_fraction;
}

unsigned முழு IEEEFloat::significandMSB() மாறிலி {
  திரும்பு APInt::tcMSB(significandParts(), part_மொத்தம்());
}

unsigned முழு IEEEFloat::significandLSB() மாறிலி {
  திரும்பு APInt::tcLSB(significandParts(), part_மொத்தம்());
}

/* Note that a zero result is NOT normalized to fcZero.  */
lostFraction IEEEFloat::shift_Significand_வலது(unsigned முழு bits) {
  /* Our exponent should not overflow.  */
  ப்ரதிஜ்ஞா((ExponentType) (exponent + bits) >= exponent);

  exponent += bits;

  திரும்பு shift_வலது(significandParts(), part_மொத்தம்(), bits);
}

/* Shift the significand left BITS bits, subtract BITS from its exponent.  */
வெறுமை IEEEFloat::shift_Significand_இடது(unsigned முழு bits) {
  ப்ரதிஜ்ஞா(bits < semantics->precision ||
         (semantics->precision == 1 && bits <= 1));

  எனில் (bits) {
    unsigned முழு parts_மொத்தம் = part_மொத்தம்();

    APInt::tc_Shift_இடது(significandParts(), parts_மொத்தம், bits);
    exponent -= bits;

    ப்ரதிஜ்ஞா(!APInt::tcIsZero(significandParts(), parts_மொத்தம்));
  }
}

APFloat::cmpResult IEEEFloat::compare_Absolute_மதிப்பு(மாறிலி IEEEFloat &rhs) மாறிலி {
  ப்ரதிஜ்ஞா(semantics == rhs.semantics);
  ப்ரதிஜ்ஞா(isFiniteNonZero());
  ப்ரதிஜ்ஞா(rhs.isFiniteNonZero());

  முழு compare = exponent - rhs.exponent;

  /* If exponents are equal, do an unsigned bignum comparison of the
     significands.  */
  எனில் (compare == 0)
    compare = APInt::tcCompare(significandParts(), rhs.significandParts(),
                               part_மொத்தம்());

  எனில் (compare > 0)
    திரும்பு cmpGreaterThan;
  இல்லையெனில் எனில் (compare < 0)
    திரும்பு cmpLessThan;
  இல்லையெனில்
    திரும்பு cmpEqual;
}

/* Set the least significant BITS bits of a bignum, clear the
   rest.  */
நிலையான வெறுமை tcSetLeastSignificantBits(APInt::WordType *dst, unsigned parts,
                                      unsigned bits) {
  unsigned i = 0;
  போது (bits > APInt::APINT_BITS_PER_WORD) {
    dst[i++] = ~(APInt::WordType)0;
    bits -= APInt::APINT_BITS_PER_WORD;
  }

  எனில் (bits)
    dst[i++] = ~(APInt::WordType)0 >> (APInt::APINT_BITS_PER_WORD - bits);

  போது (i < parts)
    dst[i++] = 0;
}

/* Handle overflow.  Sign is preserved.  We either become infinity or
   the largest finite number.  */
APFloat::opStatus IEEEFloat::handleOverflow(roundingMode rounding_mode) {
  எனில் (semantics->nonFiniteBehavior != fltNonfiniteBehavior::FiniteOnly) {
    /* Infinity?  */
    எனில் (rounding_mode == rmNearestTiesToEven ||
        rounding_mode == rmNearestTiesToAway ||
        (rounding_mode == rmTowardPositive && !sign) ||
        (rounding_mode == rmTowardNegative && sign)) {
      எனில் (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
        makeNaN(பொய், sign);
      இல்லையெனில்
        category = fcInfinity;
      திரும்பு static_cast<opStatus>(opOverflow | opInexact);
    }
  }

  /* Otherwise we become the largest finite number.  */
  category = fcNormal;
  exponent = semantics->maxExponent;
  tcSetLeastSignificantBits(significandParts(), part_மொத்தம்(),
                            semantics->precision);
  எனில் (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes)
    APInt::tcClearBit(significandParts(), 0);

  திரும்பு opInexact;
}

/* Returns TRUE if, when truncating the current number, with BIT the
   new LSB, with the given lost fraction and rounding mode, the result
   would need to be rounded away from zero (i.e., by increasing the
   signficand).  This routine must work for fcZero of both signs, and
   fcNormal numbers.  */
தருக்க IEEEFloat::roundAwayFromZero(roundingMode rounding_mode,
                                  lostFraction lost_fraction,
                                  unsigned முழு bit) மாறிலி {
  /* NaNs and infinities should not have lost fractions.  */
  ப்ரதிஜ்ஞா(isFiniteNonZero() || category == fcZero);

  /* Current callers never pass this so we don't handle it.  */
  ப்ரதிஜ்ஞா(lost_fraction != lfExactlyZero);

  தேர்வு (rounding_mode) {
  நிலை rmNearestTiesToAway:
    திரும்பு lost_fraction == lfExactlyHalf || lost_fraction == lfMoreThanHalf;

  நிலை rmNearestTiesToEven:
    எனில் (lost_fraction == lfMoreThanHalf)
      திரும்பு உண்மை;

    /* Our zeroes don't have a significand to test.  */
    எனில் (lost_fraction == lfExactlyHalf && category != fcZero)
      திரும்பு APInt::tcExtractBit(significandParts(), bit);

    திரும்பு பொய்;

  நிலை rmTowardZero:
    திரும்பு பொய்;

  நிலை rmTowardPositive:
    திரும்பு !sign;

  நிலை rmTowardNegative:
    திரும்பு sign;

  இயல்பு:
    நிறுத்து;
  }
  llvm_unreachable("Invalid rounding mode found");
}

APFloat::opStatus IEEEFloat::normalize(roundingMode rounding_mode,
                                       lostFraction lost_fraction) {
  எனில் (!isFiniteNonZero())
    திரும்பு opOK;

  /* Before rounding normalize the exponent of fcNormal numbers.  */
  /* One, not zero, based MSB.  */
  unsigned omsb = significandMSB() + 1;

  // Only skip this `if` if the value is exactly zero.
  எனில் (omsb || lost_fraction != lfExactlyZero) {
    /* OMSB is numbered from 1.  We want to place it in the integer
       bit numbered PRECISION if possible, with a compensating change in
       the exponent.  */
    முழு exponentChange = omsb - semantics->precision;

    /* If the resulting exponent is too high, overflow according to
       the rounding mode.  */
    எனில் (exponent + exponentChange > semantics->maxExponent)
      திரும்பு handleOverflow(rounding_mode);

    /* Subnormal numbers have exponent minExponent, and their MSB
       is forced based on that.  */
    எனில் (exponent + exponentChange < semantics->minExponent)
      exponentChange = semantics->minExponent - exponent;

    /* Shifting left is easy as we don't lose precision.  */
    எனில் (exponentChange < 0) {
      ப்ரதிஜ்ஞா(lost_fraction == lfExactlyZero);

      shift_Significand_இடது(-exponentChange);

      திரும்பு opOK;
    }

    எனில் (exponentChange > 0) {
      lostFraction lf;

      /* Shift right and capture any new lost fraction.  */
      lf = shift_Significand_வலது(exponentChange);

      lost_fraction = combineLostFractions(lf, lost_fraction);

      /* Keep OMSB up-to-date.  */
      எனில் (omsb > (unsigned) exponentChange)
        omsb -= exponentChange;
      இல்லையெனில்
        omsb = 0;
    }
  }

  // The all-ones values is an overflow if NaN is all ones. If NaN is
  // represented by negative zero, then it is a valid finite value.
  எனில் (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      exponent == semantics->maxExponent && isSignificandAllOnes())
    திரும்பு handleOverflow(rounding_mode);

  /* Now round the number according to rounding_mode given the lost
     fraction.  */

  /* As specified in IEEE 754, since we do not trap we do not report
     underflow for exact results.  */
  எனில் (lost_fraction == lfExactlyZero) {
    /* Canonicalize zeroes.  */
    எனில் (omsb == 0) {
      category = fcZero;
      எனில் (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = பொய்;
      எனில் (!semantics->hasZero)
        makeSmallestNormalized(பொய்);
    }

    திரும்பு opOK;
  }

  /* Increment the significand if we're rounding away from zero.  */
  எனில் (roundAwayFromZero(rounding_mode, lost_fraction, 0)) {
    எனில் (omsb == 0)
      exponent = semantics->minExponent;

    incrementSignificand();
    omsb = significandMSB() + 1;

    /* Did the significand increment overflow?  */
    எனில் (omsb == (unsigned) semantics->precision + 1) {
      /* Renormalize by incrementing the exponent and shifting our
         significand right one.  However if we already have the
         maximum exponent we overflow to infinity.  */
      எனில் (exponent == semantics->maxExponent)
        // Invoke overflow handling with a rounding mode that will guarantee
        // that the result gets turned into the correct infinity representation.
        // This is needed instead of just setting the category to infinity to
        // account for 8-bit floating point types that have no inf, only NaN.
        திரும்பு handleOverflow(sign ? rmTowardNegative : rmTowardPositive);

      shift_Significand_வலது(1);

      திரும்பு opInexact;
    }

    // The all-ones values is an overflow if NaN is all ones. If NaN is
    // represented by negative zero, then it is a valid finite value.
    எனில் (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
        semantics->nanEncoding == fltNanEncoding::AllOnes &&
        exponent == semantics->maxExponent && isSignificandAllOnes())
      திரும்பு handleOverflow(rounding_mode);
  }

  /* The normal case - we were and are not denormal, and any
     significand increment above didn't overflow.  */
  எனில் (omsb == semantics->precision)
    திரும்பு opInexact;

  /* We have a non-zero denormal.  */
  ப்ரதிஜ்ஞா(omsb < semantics->precision);

  /* Canonicalize zeroes.  */
  எனில் (omsb == 0) {
    category = fcZero;
    எனில் (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = பொய்;
    // This condition handles the case where the semantics
    // does not have zero but uses the all-zero encoding
    // to represent the smallest normal value.
    எனில் (!semantics->hasZero)
      makeSmallestNormalized(பொய்);
  }

  /* The fcZero case is a denormal that underflowed to zero.  */
  திரும்பு (opStatus) (opUnderflow | opInexact);
}

APFloat::opStatus IEEEFloat::addOrSubtractSpecials(மாறிலி IEEEFloat &rhs,
                                                   தருக்க subtract) {
  தேர்வு (Pack_Categories_Into_சாவி(category, rhs.category)) {
  இயல்பு:
    llvm_unreachable(nullptr);

  நிலை Pack_Categories_Into_சாவி(fcZero, fcNaN):
  நிலை Pack_Categories_Into_சாவி(fcNormal, fcNaN):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcZero):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcNormal):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcNaN):
    எனில் (isSignaling()) {
      makeQuiet();
      திரும்பு opInvalidOp;
    }
    திரும்பு rhs.isSignaling() ? opInvalidOp : opOK;

  நிலை Pack_Categories_Into_சாவி(fcNormal, fcZero):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcNormal):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcZero):
    திரும்பு opOK;

  நிலை Pack_Categories_Into_சாவி(fcNormal, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcZero, fcInfinity):
    category = fcInfinity;
    sign = rhs.sign ^ subtract;
    திரும்பு opOK;

  நிலை Pack_Categories_Into_சாவி(fcZero, fcNormal):
    assign(rhs);
    sign = rhs.sign ^ subtract;
    திரும்பு opOK;

  நிலை Pack_Categories_Into_சாவி(fcZero, fcZero):
    /* Sign depends on rounding mode; handled by caller.  */
    திரும்பு opOK;

  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcInfinity):
    /* Differently signed infinities can only be validly
       subtracted.  */
    எனில் (((sign ^ rhs.sign)!=0) != subtract) {
      makeNaN();
      திரும்பு opInvalidOp;
    }

    திரும்பு opOK;

  நிலை Pack_Categories_Into_சாவி(fcNormal, fcNormal):
    திரும்பு opDivByZero;
  }
}

/* Add or subtract two normal numbers.  */
lostFraction IEEEFloat::addOrSubtractSignificand(மாறிலி IEEEFloat &rhs,
                                                 தருக்க subtract) {
  [[maybe_unused]] integerPart carry = 0;
  lostFraction lost_fraction;

  /* Determine if the operation on the absolute values is effectively
     an addition or subtraction.  */
  subtract ^= static_cast<தருக்க>(sign ^ rhs.sign);

  /* Are we bigger exponent-wise than the RHS?  */
  முழு bits = exponent - rhs.exponent;

  /* Subtraction is more subtle than one might naively expect.  */
  எனில் (subtract) {
    எனில் ((bits < 0) && !semantics->hasSignedRepr)
      llvm_unreachable(
          "This floating point format does not support signed values");

    IEEEFloat temp_rhs(rhs);
    தருக்க lost_fraction_is_from_rhs = பொய்;

    எனில் (bits == 0)
      lost_fraction = lfExactlyZero;
    இல்லையெனில் எனில் (bits > 0) {
      lost_fraction = temp_rhs.shift_Significand_வலது(bits - 1);
      lost_fraction_is_from_rhs = உண்மை;
      shift_Significand_இடது(1);
    } இல்லையெனில் {
      lost_fraction = shift_Significand_வலது(-bits - 1);
      temp_rhs.shift_Significand_இடது(1);
    }

    // Should we reverse the subtraction.
    cmpResult cmp_result = compare_Absolute_மதிப்பு(temp_rhs);
    எனில் (cmp_result == cmpLessThan) {
      தருக்க borrow =
          lost_fraction != lfExactlyZero && !lost_fraction_is_from_rhs;
      எனில் (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        எனில் (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        இல்லையெனில் எனில் (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = temp_rhs.subtractSignificand(*இது, borrow);
      copySignificand(temp_rhs);
      sign = !sign;
    } இல்லையெனில் எனில் (cmp_result == cmpGreaterThan) {
      தருக்க borrow = lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs;
      எனில் (borrow) {
        // The lost fraction is being subtracted, borrow from the significand
        // and invert `lost_fraction`.
        எனில் (lost_fraction == lfLessThanHalf)
          lost_fraction = lfMoreThanHalf;
        இல்லையெனில் எனில் (lost_fraction == lfMoreThanHalf)
          lost_fraction = lfLessThanHalf;
      }
      carry = subtractSignificand(temp_rhs, borrow);
    } இல்லையெனில் { // cmpEqual
      zeroSignificand();
      எனில் (lost_fraction != lfExactlyZero && lost_fraction_is_from_rhs) {
        // rhs is slightly larger due to the lost fraction, flip the sign.
        sign = !sign;
      }
    }

    /* The code above is intended to ensure that no borrow is
       necessary.  */
    ப்ரதிஜ்ஞா(!carry);
  } இல்லையெனில் {
    எனில் (bits > 0) {
      IEEEFloat temp_rhs(rhs);

      lost_fraction = temp_rhs.shift_Significand_வலது(bits);
      carry = addSignificand(temp_rhs);
    } இல்லையெனில் {
      lost_fraction = shift_Significand_வலது(-bits);
      carry = addSignificand(rhs);
    }

    /* We have a guard bit; generating a carry cannot happen.  */
    ப்ரதிஜ்ஞா(!carry);
  }

  திரும்பு lost_fraction;
}

APFloat::opStatus IEEEFloat::multiplySpecials(மாறிலி IEEEFloat &rhs) {
  தேர்வு (Pack_Categories_Into_சாவி(category, rhs.category)) {
  இயல்பு:
    llvm_unreachable(nullptr);

  நிலை Pack_Categories_Into_சாவி(fcZero, fcNaN):
  நிலை Pack_Categories_Into_சாவி(fcNormal, fcNaN):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcNaN):
    assign(rhs);
    sign = பொய்;
    [[fallthrough]];
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcZero):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcNormal):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    எனில் (isSignaling()) {
      makeQuiet();
      திரும்பு opInvalidOp;
    }
    திரும்பு rhs.isSignaling() ? opInvalidOp : opOK;

  நிலை Pack_Categories_Into_சாவி(fcNormal, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcNormal):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcInfinity):
    category = fcInfinity;
    திரும்பு opOK;

  நிலை Pack_Categories_Into_சாவி(fcZero, fcNormal):
  நிலை Pack_Categories_Into_சாவி(fcNormal, fcZero):
  நிலை Pack_Categories_Into_சாவி(fcZero, fcZero):
    category = fcZero;
    திரும்பு opOK;

  நிலை Pack_Categories_Into_சாவி(fcZero, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcZero):
    makeNaN();
    திரும்பு opInvalidOp;

  நிலை Pack_Categories_Into_சாவி(fcNormal, fcNormal):
    திரும்பு opOK;
  }
}

APFloat::opStatus IEEEFloat::divideSpecials(மாறிலி IEEEFloat &rhs) {
  தேர்வு (Pack_Categories_Into_சாவி(category, rhs.category)) {
  இயல்பு:
    llvm_unreachable(nullptr);

  நிலை Pack_Categories_Into_சாவி(fcZero, fcNaN):
  நிலை Pack_Categories_Into_சாவி(fcNormal, fcNaN):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcNaN):
    assign(rhs);
    sign = பொய்;
    [[fallthrough]];
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcZero):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcNormal):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcNaN):
    sign ^= rhs.sign; // restore the original sign
    எனில் (isSignaling()) {
      makeQuiet();
      திரும்பு opInvalidOp;
    }
    திரும்பு rhs.isSignaling() ? opInvalidOp : opOK;

  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcZero):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcNormal):
  நிலை Pack_Categories_Into_சாவி(fcZero, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcZero, fcNormal):
    திரும்பு opOK;

  நிலை Pack_Categories_Into_சாவி(fcNormal, fcInfinity):
    category = fcZero;
    திரும்பு opOK;

  நிலை Pack_Categories_Into_சாவி(fcNormal, fcZero):
    எனில் (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly)
      makeNaN(பொய், sign);
    இல்லையெனில்
      category = fcInfinity;
    திரும்பு opDivByZero;

  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcZero, fcZero):
    makeNaN();
    திரும்பு opInvalidOp;

  நிலை Pack_Categories_Into_சாவி(fcNormal, fcNormal):
    திரும்பு opOK;
  }
}

APFloat::opStatus IEEEFloat::modSpecials(மாறிலி IEEEFloat &rhs) {
  தேர்வு (Pack_Categories_Into_சாவி(category, rhs.category)) {
  இயல்பு:
    llvm_unreachable(nullptr);

  நிலை Pack_Categories_Into_சாவி(fcZero, fcNaN):
  நிலை Pack_Categories_Into_சாவி(fcNormal, fcNaN):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcZero):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcNormal):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcNaN):
    எனில் (isSignaling()) {
      makeQuiet();
      திரும்பு opInvalidOp;
    }
    திரும்பு rhs.isSignaling() ? opInvalidOp : opOK;

  நிலை Pack_Categories_Into_சாவி(fcZero, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcZero, fcNormal):
  நிலை Pack_Categories_Into_சாவி(fcNormal, fcInfinity):
    திரும்பு opOK;

  நிலை Pack_Categories_Into_சாவி(fcNormal, fcZero):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcZero):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcNormal):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcZero, fcZero):
    makeNaN();
    திரும்பு opInvalidOp;

  நிலை Pack_Categories_Into_சாவி(fcNormal, fcNormal):
    திரும்பு opOK;
  }
}

APFloat::opStatus IEEEFloat::remainderSpecials(மாறிலி IEEEFloat &rhs) {
  தேர்வு (Pack_Categories_Into_சாவி(category, rhs.category)) {
  இயல்பு:
    llvm_unreachable(nullptr);

  நிலை Pack_Categories_Into_சாவி(fcZero, fcNaN):
  நிலை Pack_Categories_Into_சாவி(fcNormal, fcNaN):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcNaN):
    assign(rhs);
    [[fallthrough]];
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcZero):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcNormal):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcNaN):
    எனில் (isSignaling()) {
      makeQuiet();
      திரும்பு opInvalidOp;
    }
    திரும்பு rhs.isSignaling() ? opInvalidOp : opOK;

  நிலை Pack_Categories_Into_சாவி(fcZero, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcZero, fcNormal):
  நிலை Pack_Categories_Into_சாவி(fcNormal, fcInfinity):
    திரும்பு opOK;

  நிலை Pack_Categories_Into_சாவி(fcNormal, fcZero):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcZero):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcNormal):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcZero, fcZero):
    makeNaN();
    திரும்பு opInvalidOp;

  நிலை Pack_Categories_Into_சாவி(fcNormal, fcNormal):
    திரும்பு opDivByZero; // fake status, indicating this is not a special case
  }
}

/* Change sign.  */
வெறுமை IEEEFloat::changeSign() {
  // With NaN-as-negative-zero, neither NaN or negative zero can change
  // their signs.
  எனில் (semantics->nanEncoding == fltNanEncoding::NegativeZero &&
      (isZero() || isNaN()))
    திரும்பு;
  /* Look mummy, this one's easy.  */
  sign = !sign;
}

/* Normalized addition or subtraction.  */
APFloat::opStatus IEEEFloat::addOrSubtract(மாறிலி IEEEFloat &rhs,
                                           roundingMode rounding_mode,
                                           தருக்க subtract) {
  opStatus fs = addOrSubtractSpecials(rhs, subtract);

  /* This return code means it was not a simple case.  */
  எனில் (fs == opDivByZero) {
    lostFraction lost_fraction;

    lost_fraction = addOrSubtractSignificand(rhs, subtract);
    fs = normalize(rounding_mode, lost_fraction);

    /* Can only be zero if we lost no fraction.  */
    ப்ரதிஜ்ஞா(category != fcZero || lost_fraction == lfExactlyZero);
  }

  /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
     positive zero unless rounding to minus infinity, except that
     adding two like-signed zeroes gives that zero.  */
  எனில் (category == fcZero) {
    எனில் (rhs.category != fcZero || (sign == rhs.sign) == subtract)
      sign = (rounding_mode == rmTowardNegative);
    // NaN-in-negative-zero means zeros need to be normalized to +0.
    எனில் (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = பொய்;
  }

  திரும்பு fs;
}

/* Normalized addition.  */
APFloat::opStatus IEEEFloat::add(மாறிலி IEEEFloat &rhs,
                                 roundingMode rounding_mode) {
  திரும்பு addOrSubtract(rhs, rounding_mode, பொய்);
}

/* Normalized subtraction.  */
APFloat::opStatus IEEEFloat::subtract(மாறிலி IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  திரும்பு addOrSubtract(rhs, rounding_mode, உண்மை);
}

/* Normalized multiply.  */
APFloat::opStatus IEEEFloat::multiply(மாறிலி IEEEFloat &rhs,
                                      roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = multiplySpecials(rhs);

  எனில் (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = பொய்;
  எனில் (isFiniteNonZero()) {
    lostFraction lost_fraction = multiplySignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    எனில் (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  திரும்பு fs;
}

/* Normalized divide.  */
APFloat::opStatus IEEEFloat::divide(மாறிலி IEEEFloat &rhs,
                                    roundingMode rounding_mode) {
  sign ^= rhs.sign;
  opStatus fs = divideSpecials(rhs);

  எனில் (isZero() && semantics->nanEncoding == fltNanEncoding::NegativeZero)
    sign = பொய்;
  எனில் (isFiniteNonZero()) {
    lostFraction lost_fraction = divideSignificand(rhs);
    fs = normalize(rounding_mode, lost_fraction);
    எனில் (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);
  }

  திரும்பு fs;
}

/* Normalized remainder.  */
APFloat::opStatus IEEEFloat::remainder(மாறிலி IEEEFloat &rhs) {
  unsigned முழு origSign = sign;

  // First handle the special cases.
  opStatus fs = remainderSpecials(rhs);
  எனில் (fs != opDivByZero)
    திரும்பு fs;

  fs = opOK;

  // Make sure the current value is less than twice the denom. If the addition
  // did not succeed (an overflow has happened), which means that the finite
  // value we currently posses must be less than twice the denom (as we are
  // using the same semantics).
  IEEEFloat P2 = rhs;
  எனில் (P2.add(rhs, rmNearestTiesToEven) == opOK) {
    fs = mod(P2);
    ப்ரதிஜ்ஞா(fs == opOK);
  }

  // Lets work with absolute numbers.
  IEEEFloat P = rhs;
  P.sign = பொய்;
  sign = பொய்;

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
  தருக்க losesInfo;
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.maxExponent++;
  extendedSemantics.minExponent--;
  extendedSemantics.precision += 2;

  IEEEFloat VEx = *இது;
  fs = VEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  ப்ரதிஜ்ஞா(fs == opOK && !losesInfo);
  IEEEFloat PEx = P;
  fs = PEx.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  ப்ரதிஜ்ஞா(fs == opOK && !losesInfo);

  // It is simpler to work with 2x instead of 0.5p, and we do not need to lose
  // any fraction.
  fs = VEx.add(VEx, rmNearestTiesToEven);
  ப்ரதிஜ்ஞா(fs == opOK);

  எனில் (VEx.compare(PEx) == cmpGreaterThan) {
    fs = subtract(P, rmNearestTiesToEven);
    ப்ரதிஜ்ஞா(fs == opOK);

    // Make VEx = this.add(this), but because we have different semantics, we do
    // not want to `convert` again, so we just subtract PEx twice (which equals
    // to the desired value).
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    ப்ரதிஜ்ஞா(fs == opOK);
    fs = VEx.subtract(PEx, rmNearestTiesToEven);
    ப்ரதிஜ்ஞா(fs == opOK);

    cmpResult result = VEx.compare(PEx);
    எனில் (result == cmpGreaterThan || result == cmpEqual) {
      fs = subtract(P, rmNearestTiesToEven);
      ப்ரதிஜ்ஞா(fs == opOK);
    }
  }

  எனில் (isZero()) {
    sign = origSign;    // IEEE754 requires this
    எனில் (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      // But some 8-bit floats only have positive 0.
      sign = பொய்;
  } இல்லையெனில் {
    sign ^= origSign;
  }
  திரும்பு fs;
}

/* Normalized llvm frem (C fmod). */
APFloat::opStatus IEEEFloat::mod(மாறிலி IEEEFloat &rhs) {
  opStatus fs = modSpecials(rhs);
  unsigned முழு origSign = sign;

  போது (isFiniteNonZero() && rhs.isFiniteNonZero() &&
         compare_Absolute_மதிப்பு(rhs) != cmpLessThan) {
    முழு Exp = ilogb(*இது) - ilogb(rhs);
    IEEEFloat V = scalbn(rhs, Exp, rmNearestTiesToEven);
    // V can overflow to NaN with fltNonfiniteBehavior::NanOnly, so explicitly
    // check for it.
    எனில் (V.isNaN() || compare_Absolute_மதிப்பு(V) == cmpLessThan)
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
    எனில் (!semantics->hasZero && இது->isSmallest())
      நிறுத்து;

    ப்ரதிஜ்ஞா(fs==opOK);
  }
  எனில் (isZero()) {
    sign = origSign; // fmod requires this
    எனில் (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = பொய்;
  }
  திரும்பு fs;
}

/* Normalized fused-multiply-add.  */
APFloat::opStatus IEEEFloat::fusedMultiplyAdd(மாறிலி IEEEFloat &multiplicand,
                                              மாறிலி IEEEFloat &addend,
                                              roundingMode rounding_mode) {
  opStatus fs;

  /* Post-multiplication sign, before addition.  */
  sign ^= multiplicand.sign;

  /* If and only if all arguments are normal do we need to do an
     extended-precision calculation.  */
  எனில் (isFiniteNonZero() &&
      multiplicand.isFiniteNonZero() &&
      addend.isFinite()) {
    lostFraction lost_fraction;

    lost_fraction = multiplySignificand(multiplicand, addend);
    fs = normalize(rounding_mode, lost_fraction);
    எனில் (lost_fraction != lfExactlyZero)
      fs = (opStatus) (fs | opInexact);

    /* If two numbers add (exactly) to zero, IEEE 754 decrees it is a
       positive zero unless rounding to minus infinity, except that
       adding two like-signed zeroes gives that zero.  */
    எனில் (category == fcZero && !(fs & opUnderflow) && sign != addend.sign) {
      sign = (rounding_mode == rmTowardNegative);
      எனில் (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = பொய்;
    }
  } இல்லையெனில் {
    fs = multiplySpecials(multiplicand);

    /* FS can only be opOK or opInvalidOp.  There is no more work
       to do in the latter case.  The IEEE-754R standard says it is
       implementation-defined in this case whether, if ADDEND is a
       quiet NaN, we raise invalid op; this implementation does so.

       If we need to do the addition we can do so with normal
       precision.  */
    எனில் (fs == opOK)
      fs = addOrSubtract(addend, rounding_mode, பொய்);
  }

  திரும்பு fs;
}

/* Rounding-mode correct round to integral value.  */
APFloat::opStatus IEEEFloat::roundToIntegral(roundingMode rounding_mode) {
  எனில் (isInfinity())
    // [IEEE Std 754-2008 6.1]:
    // The behavior of infinity in floating-point arithmetic is derived from the
    // limiting cases of real arithmetic with operands of arbitrarily
    // large magnitude, when such a limit exists.
    // ...
    // Operations on infinite operands are usually exact and therefore signal no
    // exceptions ...
    திரும்பு opOK;

  எனில் (isNaN()) {
    எனில் (isSignaling()) {
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
      திரும்பு opInvalidOp;
    } இல்லையெனில் {
      // [IEEE Std 754-2008 6.2]:
      // For an operation with quiet NaN inputs, other than maximum and minimum
      // operations, if a floating-point result is to be delivered the result
      // shall be a quiet NaN which should be one of the input NaNs.
      // ...
      // Every general-computational and quiet-computational operation involving
      // one or more input NaNs, none of them signaling, shall signal no
      // exception, except fusedMultiplyAdd might signal the invalid operation
      // exception(see 7.2).
      திரும்பு opOK;
    }
  }

  எனில் (isZero()) {
    // [IEEE Std 754-2008 6.3]:
    // ... the sign of the result of conversions, the quantize operation, the
    // roundToIntegral operations, and the roundToIntegralExact(see 5.3.1) is
    // the sign of the first or only operand.
    திரும்பு opOK;
  }

  // If the exponent is large enough, we know that this value is already
  // integral, and the arithmetic below would potentially cause it to saturate
  // to +/-Inf.  Bail out early instead.
  எனில் (exponent + 1 >= (முழு)APFloat::semanticsPrecision(*semantics))
    திரும்பு opOK;

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
  opStatus fs = MagicConstant.convertFromAPInt(IntegerConstant, பொய்,
                                               rmNearestTiesToEven);
  ப்ரதிஜ்ஞா(fs == opOK);
  MagicConstant.sign = sign;

  // Preserve the input sign so that we can handle the case of zero result
  // correctly.
  தருக்க உள்ளீடு_Sign = isNegative();

  fs = add(MagicConstant, rounding_mode);

  // Current value and 'MagicConstant' are both integers, so the result of the
  // subtraction is always exact according to Sterbenz' lemma.
  subtract(MagicConstant, rounding_mode);

  // Restore the input sign.
  எனில் (உள்ளீடு_Sign != isNegative())
    changeSign();

  திரும்பு fs;
}

/* Comparison requires normalized numbers.  */
APFloat::cmpResult IEEEFloat::compare(மாறிலி IEEEFloat &rhs) மாறிலி {
  ப்ரதிஜ்ஞா(semantics == rhs.semantics);

  தேர்வு (Pack_Categories_Into_சாவி(category, rhs.category)) {
  இயல்பு:
    llvm_unreachable(nullptr);

  நிலை Pack_Categories_Into_சாவி(fcNaN, fcZero):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcNormal):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcNaN, fcNaN):
  நிலை Pack_Categories_Into_சாவி(fcZero, fcNaN):
  நிலை Pack_Categories_Into_சாவி(fcNormal, fcNaN):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcNaN):
    திரும்பு cmpUnordered;

  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcNormal):
  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcZero):
  நிலை Pack_Categories_Into_சாவி(fcNormal, fcZero):
    எனில் (sign)
      திரும்பு cmpLessThan;
    இல்லையெனில்
      திரும்பு cmpGreaterThan;

  நிலை Pack_Categories_Into_சாவி(fcNormal, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcZero, fcInfinity):
  நிலை Pack_Categories_Into_சாவி(fcZero, fcNormal):
    எனில் (rhs.sign)
      திரும்பு cmpGreaterThan;
    இல்லையெனில்
      திரும்பு cmpLessThan;

  நிலை Pack_Categories_Into_சாவி(fcInfinity, fcInfinity):
    எனில் (sign == rhs.sign)
      திரும்பு cmpEqual;
    இல்லையெனில் எனில் (sign)
      திரும்பு cmpLessThan;
    இல்லையெனில்
      திரும்பு cmpGreaterThan;

  நிலை Pack_Categories_Into_சாவி(fcZero, fcZero):
    திரும்பு cmpEqual;

  நிலை Pack_Categories_Into_சாவி(fcNormal, fcNormal):
    நிறுத்து;
  }

  cmpResult result;
  /* Two normal numbers.  Do they have the same sign?  */
  எனில் (sign != rhs.sign) {
    எனில் (sign)
      result = cmpLessThan;
    இல்லையெனில்
      result = cmpGreaterThan;
  } இல்லையெனில் {
    /* Compare absolute values; invert result if negative.  */
    result = compare_Absolute_மதிப்பு(rhs);

    எனில் (sign) {
      எனில் (result == cmpLessThan)
        result = cmpGreaterThan;
      இல்லையெனில் எனில் (result == cmpGreaterThan)
        result = cmpLessThan;
    }
  }

  திரும்பு result;
}

/// IEEEFloat::convert - convert a value of one floating point type to another.
/// The return value corresponds to the IEEE754 exceptions.  *losesInfo
/// records whether the transformation lost information, i.e. whether
/// converting the result back to the original type will produce the
/// original value (this is almost the same as return value==fsOK, but there
/// are edge cases where this is not so).

APFloat::opStatus IEEEFloat::convert(மாறிலி fltSemantics &toSemantics,
                                     roundingMode rounding_mode,
                                     தருக்க *losesInfo) {
  opStatus fs;
  மாறிலி fltSemantics &fromSemantics = *semantics;
  தருக்க is_signaling = isSignaling();

  lostFraction lostFraction = lfExactlyZero;
  unsigned new_Part_மொத்தம் = part_மொத்தம்_For_Bits(toSemantics.precision + 1);
  unsigned old_Part_மொத்தம் = part_மொத்தம்();
  முழு shift = toSemantics.precision - fromSemantics.precision;

  தருக்க X86SpecialNan = பொய்;
  எனில் (&fromSemantics == &APFloatBase::semX87DoubleExtended &&
      &toSemantics != &APFloatBase::semX87DoubleExtended && category == fcNaN &&
      (!(*significandParts() & 0x8000000000000000ULL) ||
       !(*significandParts() & 0x4000000000000000ULL))) {
    // x86 has some unusual NaNs which cannot be represented in any other
    // format; note them here.
    X86SpecialNan = உண்மை;
  }

  // If this is a truncation of a denormal number, and the target semantics
  // has larger exponent range than the source semantics (this can happen
  // when truncating from PowerPC double-double to double format), the
  // right shift could lose result mantissa bits.  Adjust exponent instead
  // of performing excessive shift.
  // Also do a similar trick in case shifting denormal would produce zero
  // significand as this case isn't handled correctly by normalize.
  எனில் (shift < 0 && isFiniteNonZero()) {
    முழு omsb = significandMSB() + 1;
    முழு exponentChange = omsb - fromSemantics.precision;
    எனில் (exponent + exponentChange < toSemantics.minExponent)
      exponentChange = toSemantics.minExponent - exponent;
    exponentChange = std::max(exponentChange, shift);
    எனில் (exponentChange < 0) {
      shift -= exponentChange;
      exponent += exponentChange;
    } இல்லையெனில் எனில் (omsb <= -shift) {
      exponentChange = omsb + shift - 1; // leave at least one bit set
      shift -= exponentChange;
      exponent += exponentChange;
    }
  }

  // If this is a truncation, perform the shift before we narrow the storage.
  எனில் (shift < 0 && (isFiniteNonZero() ||
                    (category == fcNaN && semantics->nonFiniteBehavior !=
                                              fltNonfiniteBehavior::NanOnly)))
    lostFraction = shift_வலது(significandParts(), old_Part_மொத்தம், -shift);

  // Fix the storage so it can hold to new value.
  எனில் (new_Part_மொத்தம் > old_Part_மொத்தம்) {
    // The new type requires more storage; make it available.
    integerPart *newParts;
    newParts = புதிய integerPart[new_Part_மொத்தம்];
    APInt::tcSet(newParts, 0, new_Part_மொத்தம்);
    எனில் (isFiniteNonZero() || category==fcNaN)
      APInt::tcAssign(newParts, significandParts(), old_Part_மொத்தம்);
    freeSignificand();
    significand.parts = newParts;
  } இல்லையெனில் எனில் (new_Part_மொத்தம் == 1 && old_Part_மொத்தம் != 1) {
    // Switch to built-in storage for a single part.
    integerPart newPart = 0;
    எனில் (isFiniteNonZero() || category==fcNaN)
      newPart = significandParts()[0];
    freeSignificand();
    significand.part = newPart;
  }

  // Now that we have the right storage, switch the semantics.
  semantics = &toSemantics;

  // If this is an extension, perform the shift now that the storage is
  // available.
  எனில் (shift > 0 && (isFiniteNonZero() || category==fcNaN))
    APInt::tc_Shift_இடது(significandParts(), new_Part_மொத்தம், shift);

  எனில் (isFiniteNonZero()) {
    fs = normalize(rounding_mode, lostFraction);
    *losesInfo = (fs != opOK);
  } இல்லையெனில் எனில் (category == fcNaN) {
    எனில் (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
      *losesInfo =
          fromSemantics.nonFiniteBehavior != fltNonfiniteBehavior::NanOnly;
      makeNaN(பொய், sign);
      fs = is_signaling ? opInvalidOp : opOK;
    } இல்லையெனில் {
      // If NaN is negative zero, we need to create a new NaN to avoid
      // converting NaN to -Inf.
      எனில் (fromSemantics.nanEncoding == fltNanEncoding::NegativeZero &&
          semantics->nanEncoding != fltNanEncoding::NegativeZero)
        makeNaN(பொய், பொய்);

      // If the source has no significand, there are no payload bits to carry
      // over, and an all-zero significand would encode an Inf. Create a new
      // NaN.
      எனில் (!APFloat::hasSignificand(fromSemantics))
        makeNaN(பொய், sign);

      *losesInfo = lostFraction != lfExactlyZero || X86SpecialNan;

      // For x87 extended precision, we want to make a NaN, not a special NaN
      // if the input wasn't special either.
      எனில் (!X86SpecialNan && semantics == &APFloatBase::semX87DoubleExtended)
        APInt::tcSetBit(significandParts(), semantics->precision - 1);

      // Convert of sNaN creates qNaN and raises an exception (invalid op).
      // This also guarantees that a sNaN does not become Inf on a truncation
      // that loses all payload bits.
      எனில் (is_signaling) {
        makeQuiet();
        fs = opInvalidOp;
      } இல்லையெனில் {
        fs = opOK;
      }
    }
  } இல்லையெனில் எனில் (category == fcInfinity &&
             semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    makeNaN(பொய், sign);
    *losesInfo = உண்மை;
    fs = opInexact;
  } இல்லையெனில் எனில் (category == fcZero &&
             semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Negative zero loses info, but positive zero doesn't.
    *losesInfo =
        fromSemantics.nanEncoding != fltNanEncoding::NegativeZero && sign;
    fs = *losesInfo ? opInexact : opOK;
    // NaN is negative zero means -0 -> +0, which can lose information
    sign = பொய்;
  } இல்லையெனில் {
    *losesInfo = பொய்;
    fs = opOK;
  }

  // The target may have no encoding for a negative value, or none for zero.
  // The paths above only report what rounding lost, so report these here too:
  // a caller that checks losesInfo would otherwise accept a result the target
  // cannot represent, and printing that result asserts.
  எனில் ((sign && !semantics->hasSignedRepr) ||
      (category == fcZero && !semantics->hasZero)) {
    *losesInfo = உண்மை;
    எனில் (fs == opOK)
      fs = opInexact;
  }

  எனில் (category == fcZero && !semantics->hasZero)
    makeSmallestNormalized(பொய்);
  திரும்பு fs;
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
    MutableArrayRef<integerPart> parts, unsigned முழு width, தருக்க isSigned,
    roundingMode rounding_mode, தருக்க *isExact) மாறிலி {
  *isExact = பொய்;

  /* Handle the three special cases first.  */
  எனில் (category == fcInfinity || category == fcNaN)
    திரும்பு opInvalidOp;

  unsigned dst_Parts_மொத்தம் = part_மொத்தம்_For_Bits(width);
  ப்ரதிஜ்ஞா(dst_Parts_மொத்தம் <= parts.size() && "Integer too big");

  எனில் (category == fcZero) {
    APInt::tcSet(parts.data(), 0, dst_Parts_மொத்தம்);
    // Negative zero can't be represented as an int.
    *isExact = !sign;
    திரும்பு opOK;
  }

  மாறிலி integerPart *src = significandParts();

  unsigned truncatedBits;
  /* Step 1: place our absolute value, with any fraction truncated, in
     the destination.  */
  எனில் (exponent < 0) {
    /* Our absolute value is less than one; truncate everything.  */
    APInt::tcSet(parts.data(), 0, dst_Parts_மொத்தம்);
    /* For exponent -1 the integer bit represents .5, look at that.
       For smaller exponents leftmost truncated bit is 0. */
    truncatedBits = semantics->precision -1U - exponent;
  } இல்லையெனில் {
    /* We want the most significant (exponent + 1) bits; the rest are
       truncated.  */
    unsigned முழு bits = exponent + 1U;

    /* Hopelessly large in magnitude?  */
    எனில் (bits > width)
      திரும்பு opInvalidOp;

    எனில் (bits < semantics->precision) {
      /* We truncate (semantics->precision - bits) bits.  */
      truncatedBits = semantics->precision - bits;
      APInt::tcExtract(parts.data(), dst_Parts_மொத்தம், src, bits, truncatedBits);
    } இல்லையெனில் {
      /* We want at least as many bits as are available.  */
      APInt::tcExtract(parts.data(), dst_Parts_மொத்தம், src, semantics->precision,
                       0);
      APInt::tc_Shift_இடது(parts.data(), dst_Parts_மொத்தம்,
                         bits - semantics->precision);
      truncatedBits = 0;
    }
  }

  /* Step 2: work out any lost fraction, and increment the absolute
     value if we would round away from zero.  */
  lostFraction lost_fraction;
  எனில் (truncatedBits) {
    lost_fraction = lostFractionThroughTruncation(src, part_மொத்தம்(),
                                                  truncatedBits);
    எனில் (lost_fraction != lfExactlyZero &&
        roundAwayFromZero(rounding_mode, lost_fraction, truncatedBits)) {
      எனில் (APInt::tcIncrement(parts.data(), dst_Parts_மொத்தம்))
        திரும்பு opInvalidOp;     /* Overflow.  */
    }
  } இல்லையெனில் {
    lost_fraction = lfExactlyZero;
  }

  /* Step 3: check if we fit in the destination.  */
  unsigned முழு omsb = APInt::tcMSB(parts.data(), dst_Parts_மொத்தம்) + 1;

  எனில் (sign) {
    எனில் (!isSigned) {
      /* Negative numbers cannot be represented as unsigned.  */
      எனில் (omsb != 0)
        திரும்பு opInvalidOp;
    } இல்லையெனில் {
      /* It takes omsb bits to represent the unsigned integer value.
         We lose a bit for the sign, but care is needed as the
         maximally negative integer is a special case.  */
      எனில் (omsb == width &&
          APInt::tcLSB(parts.data(), dst_Parts_மொத்தம்) + 1 != omsb)
        திரும்பு opInvalidOp;

      /* This case can happen because of rounding.  */
      எனில் (omsb > width)
        திரும்பு opInvalidOp;
    }

    APInt::tcNegate (parts.data(), dst_Parts_மொத்தம்);
  } இல்லையெனில் {
    எனில் (omsb >= width + !isSigned)
      திரும்பு opInvalidOp;
  }

  எனில் (lost_fraction == lfExactlyZero) {
    *isExact = உண்மை;
    திரும்பு opOK;
  }
  திரும்பு opInexact;
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
                            unsigned முழு width, தருக்க isSigned,
                            roundingMode rounding_mode, தருக்க *isExact) மாறிலி {
  opStatus fs = convertToSignExtendedInteger(parts, width, isSigned,
                                             rounding_mode, isExact);

  எனில் (fs == opInvalidOp) {
    unsigned முழு bits, dst_Parts_மொத்தம்;

    dst_Parts_மொத்தம் = part_மொத்தம்_For_Bits(width);
    ப்ரதிஜ்ஞா(dst_Parts_மொத்தம் <= parts.size() && "Integer too big");

    எனில் (category == fcNaN)
      bits = 0;
    இல்லையெனில் எனில் (sign)
      bits = isSigned;
    இல்லையெனில்
      bits = width - isSigned;

    tcSetLeastSignificantBits(parts.data(), dst_Parts_மொத்தம், bits);
    எனில் (sign && isSigned)
      APInt::tc_Shift_இடது(parts.data(), dst_Parts_மொத்தம், width - 1);
  }

  திரும்பு fs;
}

/* Convert an unsigned integer SRC to a floating point number,
   rounding according to ROUNDING_MODE.  The sign of the floating
   point number is not modified.  */
APFloat::opStatus IEEEFloat::convertFromUnsignedParts(
    மாறிலி integerPart *src, unsigned முழு src_மொத்தம், roundingMode rounding_mode) {
  category = fcNormal;
  unsigned omsb = APInt::tcMSB(src, src_மொத்தம்) + 1;
  integerPart *dst = significandParts();
  unsigned dst_மொத்தம் = part_மொத்தம்();
  unsigned precision = semantics->precision;

  /* We want the most significant PRECISION bits of SRC.  There may not
     be that many; extract what we can.  */
  lostFraction lost_fraction;
  எனில் (precision <= omsb) {
    exponent = omsb - 1;
    lost_fraction = lostFractionThroughTruncation(src, src_மொத்தம்,
                                                  omsb - precision);
    APInt::tcExtract(dst, dst_மொத்தம், src, precision, omsb - precision);
  } இல்லையெனில் {
    exponent = precision - 1;
    lost_fraction = lfExactlyZero;
    APInt::tcExtract(dst, dst_மொத்தம், src, omsb, 0);
  }

  திரும்பு normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus IEEEFloat::convertFromAPInt(மாறிலி APInt &Val, தருக்க isSigned,
                                              roundingMode rounding_mode) {
  unsigned முழு part_மொத்தம் = Val.get_இலக்கம்_சொற்கள்();
  APInt api = Val;

  sign = பொய்;
  எனில் (isSigned && api.isNegative()) {
    sign = உண்மை;
    api = -api;
  }

  திரும்பு convertFromUnsignedParts(api.getRawData(), part_மொத்தம், rounding_mode);
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromHexadecimalString(StringRef s,
                                        roundingMode rounding_mode) {
  lostFraction lost_fraction = lfExactlyZero;

  category = fcNormal;
  zeroSignificand();
  exponent = 0;

  integerPart *significand = significandParts();
  unsigned parts_மொத்தம் = part_மொத்தம்();
  unsigned bitPos = parts_மொத்தம் * integerPartWidth;
  தருக்க computedTrailingFraction = பொய்;

  // Skip leading zeroes and any (hexa)decimal point.
  StringRef::iterator begin = s.begin();
  StringRef::iterator end = s.end();
  StringRef::iterator dot;
  auto PtrOrErr = skipLeadingZeroesAndAnyDot(begin, end, &dot);
  எனில் (!PtrOrErr)
    திரும்பு PtrOrErr.takeError();
  StringRef::iterator p = *PtrOrErr;
  StringRef::iterator firstSignificantDigit = p;

  போது (p != end) {
    integerPart hex_மதிப்பு;

    எனில் (*p == '.') {
      எனில் (dot != end)
        திரும்பு createError("String contains multiple dots");
      dot = p++;
      தொடர்;
    }

    hex_மதிப்பு = hex_Digit_மதிப்பு(*p);
    எனில் (hex_மதிப்பு == UINT_MAX)
      நிறுத்து;

    p++;

    // Store the number while we have space.
    எனில் (bitPos) {
      bitPos -= 4;
      hex_மதிப்பு <<= bitPos % integerPartWidth;
      significand[bitPos / integerPartWidth] |= hex_மதிப்பு;
    } இல்லையெனில் எனில் (!computedTrailingFraction) {
      auto FractOrErr = trailingHexadecimalFraction(p, end, hex_மதிப்பு);
      எனில் (!FractOrErr)
        திரும்பு FractOrErr.takeError();
      lost_fraction = *FractOrErr;
      computedTrailingFraction = உண்மை;
    }
  }

  /* Hex floats require an exponent but not a hexadecimal point.  */
  எனில் (p == end)
    திரும்பு createError("Hex strings require an exponent");
  எனில் (*p != 'p' && *p != 'P')
    திரும்பு createError("Invalid character in significand");
  எனில் (p == begin)
    திரும்பு createError("Significand has no digits");
  எனில் (dot != end && p - begin == 1)
    திரும்பு createError("Significand has no digits");

  /* Ignore the exponent if we are zero.  */
  எனில் (p != firstSignificantDigit) {
    முழு expAdjustment;

    /* Implicit hexadecimal point?  */
    எனில் (dot == end)
      dot = p;

    /* Calculate the exponent adjustment implicit in the number of
       significant digits.  */
    expAdjustment = static_cast<முழு>(dot - firstSignificantDigit);
    எனில் (expAdjustment < 0)
      expAdjustment++;
    expAdjustment = expAdjustment * 4 - 1;

    /* Adjust for writing the significand starting at the most
       significant nibble.  */
    expAdjustment += semantics->precision;
    expAdjustment -= parts_மொத்தம் * integerPartWidth;

    /* Adjust for the given exponent.  */
    auto ExpOrErr = totalExponent(p + 1, end, expAdjustment);
    எனில் (!ExpOrErr)
      திரும்பு ExpOrErr.takeError();
    exponent = *ExpOrErr;
  }

  திரும்பு normalize(rounding_mode, lost_fraction);
}

APFloat::opStatus
IEEEFloat::roundSignificandWithExponent(மாறிலி integerPart *decSigParts,
                                        unsigned sig_Part_மொத்தம், முழு exp,
                                        roundingMode rounding_mode) {
  fltSemantics calcSemantics = { 32767, -32767, 0, 0 };
  unsigned முழு power = exp >= 0 ? exp : -exp;
  unsigned முழு pow5PartCapacity = powerOfFivePartsForExponent(power);
  SmallVector<integerPart, இலக்கம்_Pow5_Parts_On_அடுக்கு> pow5Parts;
  pow5Parts.resize_for_overwrite(pow5PartCapacity);

  தருக்க isNearest = rounding_mode == rmNearestTiesToEven ||
                   rounding_mode == rmNearestTiesToAway;

  unsigned parts = part_மொத்தம்_For_Bits(semantics->precision + 11);

  /* Calculate pow(5, abs(exp)).  */
  unsigned pow5_Part_மொத்தம் = powerOf5(pow5Parts.data(), power);

  க்கு (;; parts *= 2) {
    unsigned முழு excessPrecision, truncatedBits;

    calcSemantics.precision = parts * integerPartWidth - 1;
    excessPrecision = calcSemantics.precision - semantics->precision;
    truncatedBits = excessPrecision;

    IEEEFloat decSig(calcSemantics, uninitialized);
    decSig.makeZero(sign);
    IEEEFloat pow5(calcSemantics);

    opStatus sigStatus = decSig.convertFromUnsignedParts(
        decSigParts, sig_Part_மொத்தம், rmNearestTiesToEven);
    opStatus powStatus = pow5.convertFromUnsignedParts(
        pow5Parts.data(), pow5_Part_மொத்தம், rmNearestTiesToEven);
    /* Add exp, as 10^n = 5^n * 2^n.  */
    decSig.exponent += exp;

    lostFraction calcLostFraction;
    integerPart HUerr, HUdistance;
    unsigned முழு powHUerr;

    எனில் (exp >= 0) {
      /* multiplySignificand leaves the precision-th bit set to 1.  */
      calcLostFraction = decSig.multiplySignificand(pow5);
      powHUerr = powStatus != opOK;
    } இல்லையெனில் {
      calcLostFraction = decSig.divideSignificand(pow5);
      /* Denormal numbers have less precision.  */
      எனில் (decSig.exponent < semantics->minExponent) {
        excessPrecision += (semantics->minExponent - decSig.exponent);
        truncatedBits = excessPrecision;
        excessPrecision = std::min(excessPrecision, calcSemantics.precision);
      }
      /* Extra half-ulp lost in reciprocal of exponent.  */
      powHUerr = (powStatus == opOK && calcLostFraction == lfExactlyZero) ? 0:2;
    }

    /* Both multiplySignificand and divideSignificand return the
       result with the integer bit set.  */
    ப்ரதிஜ்ஞா(APInt::tcExtractBit
           (decSig.significandParts(), calcSemantics.precision - 1) == 1);

    HUerr = HUerrBound(calcLostFraction != lfExactlyZero, sigStatus != opOK,
                       powHUerr);
    HUdistance = 2 * ulpsFromBoundary(decSig.significandParts(),
                                      excessPrecision, isNearest);

    /* Are we guaranteed to round correctly if we truncate?  */
    எனில் (HUdistance >= HUerr) {
      APInt::tcExtract(significandParts(), part_மொத்தம்(), decSig.significandParts(),
                       calcSemantics.precision - excessPrecision,
                       excessPrecision);
      /* Take the exponent of decSig.  If we tcExtract-ed less bits
         above we must adjust our exponent to compensate for the
         implicit right shift.  */
      exponent = (decSig.exponent + semantics->precision
                  - (calcSemantics.precision - excessPrecision));
      calcLostFraction = lostFractionThroughTruncation(decSig.significandParts(),
                                                       decSig.part_மொத்தம்(),
                                                       truncatedBits);
      திரும்பு static_cast<opStatus>(normalize(rounding_mode, calcLostFraction) |
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
  எனில் (Error Err = interpretDecimal(p, str.end(), &D))
    திரும்பு std::move(Err);

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
  எனில் (D.firstSigDigit == str.end() || dec_Digit_மதிப்பு(*D.firstSigDigit) >= 10U) {
    category = fcZero;
    fs = opOK;
    எனில் (semantics->nanEncoding == fltNanEncoding::NegativeZero)
      sign = பொய்;
    எனில் (!semantics->hasZero)
      makeSmallestNormalized(பொய்);

    /* Check whether the normalized exponent is high enough to overflow
       max during the log-rebasing in the max-exponent check below. */
  } இல்லையெனில் எனில் (D.normalizedExponent - 1 > INT_MAX / 42039) {
    fs = handleOverflow(rounding_mode);

  /* If it wasn't, then it also wasn't high enough to overflow max
     during the log-rebasing in the min-exponent check.  Check that it
     won't overflow min in either check, then perform the min-exponent
     check. */
  } இல்லையெனில் எனில் (D.normalizedExponent - 1 < INT_MIN / 42039 ||
             (D.normalizedExponent + 1) * 28738 <=
               8651 * (semantics->minExponent - (முழு) semantics->precision)) {
    /* Underflow to zero and round.  */
    category = fcNormal;
    zeroSignificand();
    fs = normalize(rounding_mode, lfLessThanHalf);

  /* We can finally safely perform the max-exponent check. */
  } இல்லையெனில் எனில் ((D.normalizedExponent - 1) * 42039
             >= 12655 * semantics->maxExponent) {
    /* Overflow and round.  */
    fs = handleOverflow(rounding_mode);
  } இல்லையெனில் {
    integerPart *decSignificand;
    unsigned முழு part_மொத்தம்;

    /* A tight upper bound on number of bits required to hold an
       N-digit decimal integer is N * 196 / 59.  Allocate enough space
       to hold the full significand, and an extra part required by
       tcMultiplyPart.  */
    part_மொத்தம் = static_cast<unsigned முழு>(D.lastSigDigit - D.firstSigDigit) + 1;
    part_மொத்தம் = part_மொத்தம்_For_Bits(1 + 196 * part_மொத்தம் / 59);
    decSignificand = புதிய integerPart[part_மொத்தம் + 1];
    part_மொத்தம் = 0;

    /* Convert to binary efficiently - we do almost all multiplication
       in an integerPart.  When this would overflow do we do a single
       bignum multiplication, and then revert again to multiplication
       in an integerPart.  */
    செய் {
      integerPart dec_மதிப்பு, val, multiplier;

      val = 0;
      multiplier = 1;

      செய் {
        எனில் (*p == '.') {
          p++;
          எனில் (p == str.end()) {
            நிறுத்து;
          }
        }
        dec_மதிப்பு = dec_Digit_மதிப்பு(*p++);
        எனில் (dec_மதிப்பு >= 10U) {
          delete[] decSignificand;
          திரும்பு createError("Invalid character in significand");
        }
        multiplier *= 10;
        val = val * 10 + dec_மதிப்பு;
        /* The maximum number that can be multiplied by ten with any
           digit added without overflowing an integerPart.  */
      } போது (p <= D.lastSigDigit && multiplier <= (~ (integerPart) 0 - 9) / 10);

      /* Multiply out the current part.  */
      APInt::tcMultiplyPart(decSignificand, decSignificand, multiplier, val,
                            part_மொத்தம், part_மொத்தம் + 1, பொய்);

      /* If we used another part (likely but not guaranteed), increase
         the count.  */
      எனில் (decSignificand[part_மொத்தம்])
        part_மொத்தம்++;
    } போது (p <= D.lastSigDigit);

    category = fcNormal;
    fs = roundSignificandWithExponent(decSignificand, part_மொத்தம்,
                                      D.exponent, rounding_mode);

    delete [] decSignificand;
  }

  திரும்பு fs;
}

தருக்க IEEEFloat::convertFromStringSpecials(StringRef str) {
  மாறிலி size_t MIN_NAME_SIZE = 3;

  எனில் (str.size() < MIN_NAME_SIZE)
    திரும்பு பொய்;

  எனில் (str == "inf" || str == "INFINITY" || str == "+Inf" || str == "+inf") {
    makeInf(பொய்);
    திரும்பு உண்மை;
  }

  தருக்க IsNegative = str.consume_front("-");
  எனில் (IsNegative) {
    எனில் (str.size() < MIN_NAME_SIZE)
      திரும்பு பொய்;

    எனில் (str == "inf" || str == "INFINITY" || str == "Inf") {
      makeInf(உண்மை);
      திரும்பு உண்மை;
    }
  }

  // If we have a 's' (or 'S') prefix, then this is a Signaling NaN.
  தருக்க IsSignaling = str.consume_front_insensitive("s");
  எனில் (IsSignaling) {
    எனில் (str.size() < MIN_NAME_SIZE)
      திரும்பு பொய்;
  }

  எனில் (str.consume_front("nan") || str.consume_front("NaN")) {
    // A NaN without payload.
    எனில் (str.empty()) {
      makeNaN(IsSignaling, IsNegative);
      திரும்பு உண்மை;
    }

    // Allow the payload to be inside parentheses.
    எனில் (str.front() == '(') {
      // Parentheses should be balanced (and not empty).
      எனில் (str.size() <= 2 || str.back() != ')')
        திரும்பு பொய்;

      str = str.slice(1, str.size() - 1);
    }

    // Determine the payload number's radix.
    unsigned Radix = 10;
    எனில் (str[0] == '0') {
      எனில் (str.size() > 1 && tolower(str[1]) == 'x') {
        str = str.drop_front(2);
        Radix = 16;
      } இல்லையெனில் {
        Radix = 8;
      }
    }

    // Parse the payload and make the NaN.
    APInt Payload;
    எனில் (!str.getAsInteger(Radix, Payload)) {
      makeNaN(IsSignaling, IsNegative, &Payload);
      திரும்பு உண்மை;
    }
  }

  திரும்பு பொய்;
}

Expected<APFloat::opStatus>
IEEEFloat::convertFromString(StringRef str, roundingMode rounding_mode) {
  எனில் (str.empty())
    திரும்பு createError("Invalid string length");

  // Handle special cases.
  எனில் (convertFromStringSpecials(str))
    திரும்பு opOK;

  /* Handle a leading minus sign.  */
  StringRef::iterator p = str.begin();
  size_t slen = str.size();
  sign = *p == '-' ? 1 : 0;
  எனில் (sign && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");

  எனில் (*p == '-' || *p == '+') {
    p++;
    slen--;
    எனில் (!slen)
      திரும்பு createError("String has no digits");
  }

  எனில் (slen >= 2 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
    எனில் (slen == 2)
      திரும்பு createError("Invalid string");
    திரும்பு convertFromHexadecimalString(StringRef(p + 2, slen - 2),
                                        rounding_mode);
  }

  திரும்பு convertFromDecimalString(StringRef(p, slen), rounding_mode);
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
unsigned முழு IEEEFloat::convertToHexString(எழுத்து *dst, unsigned முழு hexDigits,
                                           தருக்க upperCase,
                                           roundingMode rounding_mode) மாறிலி {
  எழுத்து *p = dst;
  எனில் (sign)
    *dst++ = '-';

  தேர்வு (category) {
  நிலை fcInfinity:
    memcpy (dst, upperCase ? infinityU: infinityL, அளவு infinityU - 1);
    dst += அளவு infinityL - 1;
    நிறுத்து;

  நிலை fcNaN:
    memcpy (dst, upperCase ? NaNU: NaNL, அளவு NaNU - 1);
    dst += அளவு NaNU - 1;
    நிறுத்து;

  நிலை fcZero:
    *dst++ = '0';
    *dst++ = upperCase ? 'X': 'x';
    *dst++ = '0';
    எனில் (hexDigits > 1) {
      *dst++ = '.';
      memset (dst, '0', hexDigits - 1);
      dst += hexDigits - 1;
    }
    *dst++ = upperCase ? 'P': 'p';
    *dst++ = '0';
    நிறுத்து;

  நிலை fcNormal:
    dst = convertNormalToHexString (dst, hexDigits, upperCase, rounding_mode);
    நிறுத்து;
  }

  *dst = 0;

  திரும்பு static_cast<unsigned முழு>(dst - p);
}

/* Does the hard work of outputting the correctly rounded hexadecimal
   form of a normal floating point number with the specified number of
   hexadecimal digits.  If HEXDIGITS is zero the minimum number of
   digits necessary to print the value precisely is output.  */
எழுத்து *IEEEFloat::convertNormalToHexString(எழுத்து *dst, unsigned முழு hexDigits,
                                          தருக்க upperCase,
                                          roundingMode rounding_mode) மாறிலி {
  *dst++ = '0';
  *dst++ = upperCase ? 'X': 'x';

  தருக்க roundUp = பொய்;
  மாறிலி எழுத்து *hexDigitChars = upperCase ? hexDigitsUpper : hexDigitsLower;

  மாறிலி integerPart *significand = significandParts();
  unsigned parts_மொத்தம் = part_மொத்தம்();

  /* +3 because the first digit only uses the single integer bit, so
     we have 3 virtual zero most-significant-bits.  */
  unsigned மதிப்பு_Bits = semantics->precision + 3;
  unsigned shift = integerPartWidth - மதிப்பு_Bits % integerPartWidth;

  /* The natural number of digits required ignoring trailing
     insignificant zeroes.  */
  unsigned outputDigits = (மதிப்பு_Bits - significandLSB() + 3) / 4;

  /* hexDigits of zero means use the required number for the
     precision.  Otherwise, see if we are truncating.  If we are,
     find out if we need to round away from zero.  */
  எனில் (hexDigits) {
    எனில் (hexDigits < outputDigits) {
      /* We are dropping non-zero bits, so need to check how to round.
         "bits" is the number of dropped bits.  */
      unsigned முழு bits;
      lostFraction fraction;

      bits = மதிப்பு_Bits - hexDigits * 4;
      fraction = lostFractionThroughTruncation (significand, parts_மொத்தம், bits);
      roundUp = roundAwayFromZero(rounding_mode, fraction, bits);
    }
    outputDigits = hexDigits;
  }

  /* Write the digits consecutively, and start writing in the location
     of the hexadecimal point.  We move the most significant digit
     left and add the hexadecimal point later.  */
  எழுத்து *p = ++dst;

  unsigned மொத்தம் = (மதிப்பு_Bits + integerPartWidth - 1) / integerPartWidth;

  போது (outputDigits && மொத்தம்) {
    integerPart part;

    /* Put the most significant integerPartWidth bits in "part".  */
    எனில் (--மொத்தம் == parts_மொத்தம்)
      part = 0;  /* An imaginary higher zero part.  */
    இல்லையெனில்
      part = significand[மொத்தம்] << shift;

    எனில் (மொத்தம் && shift)
      part |= significand[மொத்தம் - 1] >> (integerPartWidth - shift);

    /* Convert as much of "part" to hexdigits as we can.  */
    unsigned முழு curDigits = integerPartWidth / 4;

    curDigits = std::min(curDigits, outputDigits);
    dst += partAsHex (dst, part, curDigits, hexDigitChars);
    outputDigits -= curDigits;
  }

  எனில் (roundUp) {
    எழுத்து *q = dst;

    /* Note that hexDigitChars has a trailing '0'.  */
    செய் {
      q--;
      *q = hexDigitChars[hex_Digit_மதிப்பு (*q) + 1];
    } போது (*q == '0');
    ப்ரதிஜ்ஞா(q >= p);
  } இல்லையெனில் {
    /* Add trailing zeroes.  */
    memset (dst, '0', outputDigits);
    dst += outputDigits;
  }

  /* Move the most significant digit to before the point, and if there
     is something after the decimal point add it.  This must come
     after rounding above.  */
  p[-1] = p[0];
  எனில் (dst -1 == p)
    dst--;
  இல்லையெனில்
    p[0] = '.';

  /* Finally output the exponent.  */
  *dst++ = upperCase ? 'P': 'p';

  திரும்பு writeSignedDecimal (dst, exponent);
}

hash_code hash_மதிப்பு(மாறிலி IEEEFloat &Arg) {
  எனில் (!Arg.isFiniteNonZero())
    திரும்பு hash_combine((uint8_t)Arg.category,
                        // NaN has no sign, fix it at zero.
                        Arg.isNaN() ? (uint8_t)0 : (uint8_t)Arg.sign,
                        Arg.semantics->precision);

  // Normal floats need their exponent and significand hashed.
  திரும்பு hash_combine((uint8_t)Arg.category, (uint8_t)Arg.sign,
                      Arg.semantics->precision, Arg.exponent,
                      hash_combine_range(
                        Arg.significandParts(),
                        Arg.significandParts() + Arg.part_மொத்தம்()));
}

// Conversion from APFloat to/from host float/double.  It may eventually be
// possible to eliminate these and have everybody deal with APFloats, but that
// will take a while.  This approach will not easily extend to long double.
// Current implementation requires integerPartWidth==64, which is correct at
// the moment but could be made more general.

// Denormals have exponent minExponent in APFloat, but minExponent-1 in
// the actual IEEE respresentations.  We compensate for that here.

APInt IEEEFloat::convertF80LongDoubleAPFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்() == 2);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semX87DoubleExtended>();
}

APInt IEEEFloat::convertPPCDoubleDoubleLegacyAPFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(semantics ==
         (மாறிலி llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy);
  ப்ரதிஜ்ஞா(part_மொத்தம்()==2);

  uint64_t சொற்கள்[2];
  தருக்க losesInfo;

  // Convert number to double.  To avoid spurious underflows, we re-
  // normalize against the "double" minExponent first, and only *then*
  // truncate the mantissa.  The result of that second conversion
  // may be inexact, but should never underflow.
  // Declare fltSemantics before APFloat that uses it (and
  // saves pointer to it) to ensure correct destruction order.
  fltSemantics extendedSemantics = *semantics;
  extendedSemantics.minExponent = APFloatBase::semIEEEdouble.minExponent;
  IEEEFloat extended(*இது);
  [[maybe_unused]] opStatus fs =
      extended.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
  ப்ரதிஜ்ஞா(fs == opOK && !losesInfo);

  IEEEFloat u(extended);
  fs = u.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
  ப்ரதிஜ்ஞா(fs == opOK || fs == opInexact);
  சொற்கள்[0] = *u.convertDoubleAPFloatToAPInt().getRawData();

  // If conversion was exact or resulted in a special case, we're done;
  // just set the second double to zero.  Otherwise, re-convert back to
  // the extended format and compute the difference.  This now should
  // convert exactly to double.
  எனில் (u.isFiniteNonZero() && losesInfo) {
    fs = u.convert(extendedSemantics, rmNearestTiesToEven, &losesInfo);
    ப்ரதிஜ்ஞா(fs == opOK && !losesInfo);

    IEEEFloat v(extended);
    v.subtract(u, rmNearestTiesToEven);
    fs = v.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &losesInfo);
    ப்ரதிஜ்ஞா(fs == opOK && !losesInfo);
    சொற்கள்[1] = *v.convertDoubleAPFloatToAPInt().getRawData();
  } இல்லையெனில் {
    சொற்கள்[1] = 0;
  }

  திரும்பு APInt(128, சொற்கள்);
}

template <மாறிலி fltSemantics &S>
APInt IEEEFloat::convertIEEEFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(semantics == &S);
  constexpr unsigned முழு trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr முழு integer_bit_part = (S.precision - 1) / integerPartWidth;
  constexpr integerPart integer_bit = integerPart{1}
                                      << ((S.precision - 1) % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned முழு exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr தருக்க is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr முழு bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));

  uint64_t myexponent;
  std::array<integerPart, part_மொத்தம்_For_Bits(trailing_significand_bits)>
      mysignificand;

  எனில் (isFiniteNonZero()) {
    myexponent = exponent + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
    எனில் (myexponent == 1 &&
        !(significandParts()[integer_bit_part] & integer_bit))
      myexponent = 0; // denormal
  } இல்லையெனில் எனில் (category == fcZero) {
    எனில் (!S.hasZero)
      llvm_unreachable("semantics does not support zero!");
    myexponent = ::exponentZero(S) + bias;
    mysignificand.fill(0);
  } இல்லையெனில் எனில் (category == fcInfinity) {
    எனில் (S.nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
        S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support inf!");
    myexponent = ::exponentInf(S) + bias;
    mysignificand.fill(0);
    எனில் constexpr (S.hasExplicitIntegerBit) {
      mysignificand[0] = integerPart{1} << (trailing_significand_bits - 1);
    }
  } இல்லையெனில் {
    ப்ரதிஜ்ஞா(category == fcNaN && "Unknown category!");
    எனில் (S.nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
      llvm_unreachable("semantics don't support NaN!");
    myexponent = ::exponentNaN(S) + bias;
    std::copy_n(significandParts(), mysignificand.size(),
                mysignificand.begin());
  }
  std::array<uint64_t, (S.sizeInBits + 63) / 64> சொற்கள்;
  auto சொற்கள்_iter =
      std::copy_n(mysignificand.begin(), mysignificand.size(), சொற்கள்.begin());
  எனில் constexpr (!S.hasExplicitIntegerBit) {
    எனில் constexpr (significand_mask != 0 || trailing_significand_bits == 0) {
      // Clear the integer bit.
      சொற்கள்[mysignificand.size() - 1] &= significand_mask;
    }
  }
  std::fill(சொற்கள்_iter, சொற்கள்.end(), uint64_t{0});
  constexpr size_t last_word = சொற்கள்.size() - 1;
  எனில் constexpr (S.hasSignedRepr) {
    uint64_t shifted_sign = static_cast<uint64_t>(sign & 1)
                            << ((S.sizeInBits - 1) % 64);
    சொற்கள்[last_word] |= shifted_sign;
  }
  uint64_t shifted_exponent = (myexponent & exponent_mask)
                              << (trailing_significand_bits % 64);
  சொற்கள்[last_word] |= shifted_exponent;
  எனில் constexpr (last_word == 0) {
    திரும்பு APInt(S.sizeInBits, சொற்கள்[0]);
  }
  திரும்பு APInt(S.sizeInBits, சொற்கள்);
}

APInt IEEEFloat::convertQuadrupleAPFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்() == 2);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semIEEEquad>();
}

APInt IEEEFloat::convertDoubleAPFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்()==1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semIEEEdouble>();
}

APInt IEEEFloat::convertFloatAPFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்()==1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semIEEEsingle>();
}

APInt IEEEFloat::convertBFloatAPFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்() == 1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semBFloat>();
}

APInt IEEEFloat::convertHalfAPFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்()==1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::APFloatBase::semIEEEhalf>();
}

APInt IEEEFloat::convertFloat8E5M2APFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்() == 1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2>();
}

APInt IEEEFloat::convertFloat8E5M2FNUZAPFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்() == 1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M2FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3APFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்() == 1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3>();
}

APInt IEEEFloat::convertFloat8E4M3FNAPFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்() == 1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FN>();
}

APInt IEEEFloat::convertFloat8E4M3FNUZAPFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்() == 1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3FNUZ>();
}

APInt IEEEFloat::convertFloat8E4M3B11FNUZAPFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்() == 1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semFloat8E4M3B11FNUZ>();
}

APInt IEEEFloat::convertFloat8E3M4APFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்() == 1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semFloat8E3M4>();
}

APInt IEEEFloat::convertFloatTF32APFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்() == 1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semFloatTF32>();
}

APInt IEEEFloat::convertFloat8E8M0FNUAPFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்() == 1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semFloat8E8M0FNU>();
}

APInt IEEEFloat::convertFloat8E5M3FNUAPFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்() == 1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semFloat8E5M3FNU>();
}

APInt IEEEFloat::convertFloat6E3M2FNAPFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்() == 1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semFloat6E3M2FN>();
}

APInt IEEEFloat::convertFloat6E2M3FNAPFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்() == 1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semFloat6E2M3FN>();
}

APInt IEEEFloat::convertFloat4E2M1FNAPFloatToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(part_மொத்தம்() == 1);
  திரும்பு convertIEEEFloatToAPInt<APFloatBase::semFloat4E2M1FN>();
}

// This function creates an APInt that is just a bit map of the floating
// point constant as it would appear in memory.  It is not a conversion,
// and treating the result as a normal integer is unlikely to be useful.

APInt IEEEFloat::bitcastToAPInt() மாறிலி {
  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semIEEEhalf)
    திரும்பு convertHalfAPFloatToAPInt();

  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semBFloat)
    திரும்பு convertBFloatAPFloatToAPInt();

  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semIEEEsingle)
    திரும்பு convertFloatAPFloatToAPInt();

  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semIEEEdouble)
    திரும்பு convertDoubleAPFloatToAPInt();

  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semIEEEquad)
    திரும்பு convertQuadrupleAPFloatToAPInt();

  எனில் (semantics ==
      (மாறிலி llvm::fltSemantics *)&APFloatBase::semX87DoubleExtended)
    திரும்பு convertF80LongDoubleAPFloatToAPInt();

  எனில் (semantics ==
      (மாறிலி llvm::fltSemantics *)&APFloatBase::semPPCDoubleDoubleLegacy)
    திரும்பு convertPPCDoubleDoubleLegacyAPFloatToAPInt();

  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semFloat8E5M2)
    திரும்பு convertFloat8E5M2APFloatToAPInt();

  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semFloat8E5M2FNUZ)
    திரும்பு convertFloat8E5M2FNUZAPFloatToAPInt();

  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semFloat8E4M3)
    திரும்பு convertFloat8E4M3APFloatToAPInt();

  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FN)
    திரும்பு convertFloat8E4M3FNAPFloatToAPInt();

  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semFloat8E4M3FNUZ)
    திரும்பு convertFloat8E4M3FNUZAPFloatToAPInt();

  எனில் (semantics ==
      (மாறிலி llvm::fltSemantics *)&APFloatBase::semFloat8E4M3B11FNUZ)
    திரும்பு convertFloat8E4M3B11FNUZAPFloatToAPInt();

  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semFloat8E3M4)
    திரும்பு convertFloat8E3M4APFloatToAPInt();

  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semFloatTF32)
    திரும்பு convertFloatTF32APFloatToAPInt();

  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semFloat8E8M0FNU)
    திரும்பு convertFloat8E8M0FNUAPFloatToAPInt();

  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semFloat8E5M3FNU)
    திரும்பு convertFloat8E5M3FNUAPFloatToAPInt();

  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semFloat6E3M2FN)
    திரும்பு convertFloat6E3M2FNAPFloatToAPInt();

  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semFloat6E2M3FN)
    திரும்பு convertFloat6E2M3FNAPFloatToAPInt();

  எனில் (semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semFloat4E2M1FN)
    திரும்பு convertFloat4E2M1FNAPFloatToAPInt();

  llvm_unreachable("unknown format!");
}

தசம IEEEFloat::convertToFloat() மாறிலி {
  ப்ரதிஜ்ஞா(semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semIEEEsingle &&
         "Float semantics are not IEEEsingle");
  APInt api = bitcastToAPInt();
  திரும்பு api.bitsToFloat();
}

இரட்டை IEEEFloat::convertToDouble() மாறிலி {
  ப்ரதிஜ்ஞா(semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semIEEEdouble &&
         "Float semantics are not IEEEdouble");
  APInt api = bitcastToAPInt();
  திரும்பு api.bitsToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 IEEEFloat::convertToQuad() மாறிலி {
  ப்ரதிஜ்ஞா(semantics == (மாறிலி llvm::fltSemantics *)&APFloatBase::semIEEEquad &&
         "Float semantics are not IEEEquads");
  APInt api = bitcastToAPInt();
  திரும்பு api.bitsToQuad();
}
#endif

வெறுமை IEEEFloat::initFromF80LongDoubleAPInt(மாறிலி APInt &api) {
  திரும்பு initFromIEEEAPInt<APFloatBase::semX87DoubleExtended>(api);
}

வெறுமை IEEEFloat::initFromPPCDoubleDoubleLegacyAPInt(மாறிலி APInt &api) {
  uint64_t i1 = api.getRawData()[0];
  uint64_t i2 = api.getRawData()[1];
  தருக்க losesInfo;

  // Get the first double and convert to our format.
  initFromDoubleAPInt(APInt(64, i1));
  [[maybe_unused]] opStatus fs = convert(APFloatBase::semPPCDoubleDoubleLegacy,
                                         rmNearestTiesToEven, &losesInfo);
  // (convert may return opInvalidOp if i1 is an sNaN).
  ப்ரதிஜ்ஞா((fs == opOK || fs == opInvalidOp) && !losesInfo);

  // Unless we have a special case, add in second double.
  எனில் (isFiniteNonZero()) {
    IEEEFloat v(APFloatBase::semIEEEdouble, APInt(64, i2));
    // The first double determines the category, so ignore a non-canonical
    // second double.
    எனில் (v.isFinite()) {
      fs = v.convert(APFloatBase::semPPCDoubleDoubleLegacy, rmNearestTiesToEven,
                     &losesInfo);
      ப்ரதிஜ்ஞா(fs == opOK && !losesInfo);

      add(v, rmNearestTiesToEven);
    }
  }
}

// The E8M0 format has the following characteristics:
// It is an 8-bit unsigned format with only exponents (no actual significand).
// No encodings for {zero, infinities or denorms}.
// NaN is represented by all 1's.
// Bias is 127.
வெறுமை IEEEFloat::initFromFloat8E8M0FNUAPInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E8M0FNU>(api);
}

வெறுமை IEEEFloat::initFromFloat8E5M3FNUAPInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M3FNU>(api);
}

template <மாறிலி fltSemantics &S>
வெறுமை IEEEFloat::initFromIEEEAPInt(மாறிலி APInt &api) {
  ப்ரதிஜ்ஞா(api.getBitWidth() == S.sizeInBits);

  constexpr unsigned முழு trailing_significand_bits =
      S.precision - 1 + S.hasExplicitIntegerBit;
  constexpr integerPart integer_bit =
      integerPart{1} << (trailing_significand_bits % integerPartWidth);
  constexpr uint64_t significand_mask = integer_bit - 1;
  constexpr unsigned முழு exponent_bits =
      S.sizeInBits - (S.hasSignedRepr ? 1 : 0) - trailing_significand_bits;
  static_assert(exponent_bits < 64);
  constexpr unsigned முழு stored_significand_parts =
      part_மொத்தம்_For_Bits(trailing_significand_bits + 1);
  constexpr uint64_t exponent_mask = (uint64_t{1} << exponent_bits) - 1;
  constexpr தருக்க is_zero_exp_reserved = S.hasDenormals || S.hasZero;
  constexpr முழு bias = -(S.minExponent - (is_zero_exp_reserved ? 1 : 0));
  constexpr தருக்க has_significand = trailing_significand_bits > 0;

  // Copy the bits of the significand. We need to clear out the exponent and
  // sign bit in the last word.
  std::array<integerPart, stored_significand_parts> mysignificand;
  எனில் constexpr (has_significand) {
    std::copy_n(api.getRawData(), mysignificand.size(), mysignificand.begin());
    எனில் constexpr (significand_mask != 0 || S.precision >= integerPartWidth) {
      mysignificand[mysignificand.size() - 1] &= significand_mask;
    }
  } இல்லையெனில் {
    std::fill_n(mysignificand.begin(), mysignificand.size(), 0);
    // Always set integer bit to 1 for consistency in APFloat's internal
    // representation.
    mysignificand[0] = 1;
  }

  // We assume the last word holds the sign bit, the exponent, and potentially
  // some of the trailing significand field.
  uint64_t last_word = api.getRawData()[api.get_இலக்கம்_சொற்கள்() - 1];
  uint64_t myexponent =
      (last_word >> (trailing_significand_bits % 64)) & exponent_mask;

  initialize(&S);
  ப்ரதிஜ்ஞா(part_மொத்தம்() == mysignificand.size());

  sign = S.hasSignedRepr
             ? static_cast<unsigned முழு>(last_word >> ((S.sizeInBits - 1) % 64))
             : 0;

  தருக்க all_zero_significand =
      has_significand && llvm::all_of(mysignificand, equal_to(0));

  தருக்க is_zero = myexponent == 0 && all_zero_significand && S.hasZero;

  எனில் constexpr (S.nonFiniteBehavior == fltNonfiniteBehavior::IEEE754) {
    தருக்க is_inf = பொய்;

    எனில் constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;
      மாறிலி integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);

      is_inf = myexponent - bias == ::exponentInf(S) && myintegerbit == 1 &&
               (mysignificand[0] & significand_mask_no_int_bit) == 0;
    } இல்லையெனில் {
      is_inf = myexponent - bias == ::exponentInf(S) && all_zero_significand;
    }

    எனில் (is_inf) {
      makeInf(sign);
      திரும்பு;
    }
  }

  தருக்க is_nan = பொய்;

  எனில் constexpr (S.nanEncoding == fltNanEncoding::IEEE) {
    எனில் constexpr (S.hasExplicitIntegerBit) {
      // This is only used and tested for x87DoubleExtended
      static_assert(S.precision == 64);
      மாறிலி integerPart myintegerbit =
          mysignificand[0] >> (trailing_significand_bits - 1);
      constexpr integerPart significand_mask_no_int_bit =
          (uint64_t{1} << (trailing_significand_bits - 1)) - 1;

      எனில் (myexponent - bias == ::exponentNaN(S) &&
          (mysignificand[0] & significand_mask_no_int_bit) != 0) {
        // regular NaN and pseudoNaN
        is_nan = உண்மை;
      } இல்லையெனில் எனில் (myexponent - bias == ::exponentNaN(S) &&
                 (mysignificand[0] & significand_mask_no_int_bit) == 0) {
        // pseudoinfinity
        is_nan = உண்மை;
      } இல்லையெனில் எனில் (myexponent - bias != ::exponentNaN(S) && myexponent != 0 &&
                 myintegerbit == 0) {
        // unnormal
        is_nan = உண்மை;
      }
    } இல்லையெனில் {
      is_nan = myexponent - bias == ::exponentNaN(S) && !all_zero_significand;
    }
  } இல்லையெனில் எனில் constexpr (S.nanEncoding == fltNanEncoding::AllOnes) {
    தருக்க all_ones_significand =
        std::all_of(mysignificand.begin(), mysignificand.end() - 1,
                    [](integerPart bits) { திரும்பு bits == ~integerPart{0}; }) &&
        (!significand_mask ||
         mysignificand[mysignificand.size() - 1] == significand_mask);
    is_nan = myexponent - bias == ::exponentNaN(S) && all_ones_significand;
  } இல்லையெனில் எனில் constexpr (S.nanEncoding == fltNanEncoding::NegativeZero) {
    is_nan = is_zero && sign;
  }

  எனில் (is_nan) {
    category = fcNaN;
    exponent = ::exponentNaN(S);
    std::copy_n(mysignificand.begin(), mysignificand.size(),
                significandParts());
    திரும்பு;
  }

  எனில் (is_zero) {
    makeZero(sign);
    திரும்பு;
  }

  category = fcNormal;
  exponent = myexponent - bias;
  std::copy_n(mysignificand.begin(), mysignificand.size(), significandParts());
  எனில் (myexponent == 0 && S.hasDenormals) // denormal
    exponent = S.minExponent;
  இல்லையெனில் {
    எனில் constexpr (!S.hasExplicitIntegerBit) {
      significandParts()[mysignificand.size() - 1] |= integer_bit;
    }
  }
}

வெறுமை IEEEFloat::initFromQuadrupleAPInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEquad>(api);
}

வெறுமை IEEEFloat::initFromDoubleAPInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEdouble>(api);
}

வெறுமை IEEEFloat::initFromFloatAPInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEsingle>(api);
}

வெறுமை IEEEFloat::initFromBFloatAPInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semBFloat>(api);
}

வெறுமை IEEEFloat::initFromHalfAPInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semIEEEhalf>(api);
}

வெறுமை IEEEFloat::initFromFloat8E5M2APInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2>(api);
}

வெறுமை IEEEFloat::initFromFloat8E5M2FNUZAPInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E5M2FNUZ>(api);
}

வெறுமை IEEEFloat::initFromFloat8E4M3APInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3>(api);
}

வெறுமை IEEEFloat::initFromFloat8E4M3FNAPInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FN>(api);
}

வெறுமை IEEEFloat::initFromFloat8E4M3FNUZAPInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3FNUZ>(api);
}

வெறுமை IEEEFloat::initFromFloat8E4M3B11FNUZAPInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E4M3B11FNUZ>(api);
}

வெறுமை IEEEFloat::initFromFloat8E3M4APInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat8E3M4>(api);
}

வெறுமை IEEEFloat::initFromFloatTF32APInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloatTF32>(api);
}

வெறுமை IEEEFloat::initFromFloat6E3M2FNAPInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E3M2FN>(api);
}

வெறுமை IEEEFloat::initFromFloat6E2M3FNAPInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat6E2M3FN>(api);
}

வெறுமை IEEEFloat::initFromFloat4E2M1FNAPInt(மாறிலி APInt &api) {
  initFromIEEEAPInt<APFloatBase::semFloat4E2M1FN>(api);
}

/// Treat api as containing the bits of a floating point number.
வெறுமை IEEEFloat::initFromAPInt(மாறிலி fltSemantics *Sem, மாறிலி APInt &api) {
  ப்ரதிஜ்ஞா(api.getBitWidth() == Sem->sizeInBits);
  எனில் (Sem == &APFloatBase::semIEEEhalf)
    திரும்பு initFromHalfAPInt(api);
  எனில் (Sem == &APFloatBase::semBFloat)
    திரும்பு initFromBFloatAPInt(api);
  எனில் (Sem == &APFloatBase::semIEEEsingle)
    திரும்பு initFromFloatAPInt(api);
  எனில் (Sem == &APFloatBase::semIEEEdouble)
    திரும்பு initFromDoubleAPInt(api);
  எனில் (Sem == &APFloatBase::semIEEEquad)
    திரும்பு initFromQuadrupleAPInt(api);
  எனில் (Sem == &APFloatBase::semX87DoubleExtended)
    திரும்பு initFromF80LongDoubleAPInt(api);
  எனில் (Sem == &APFloatBase::semPPCDoubleDoubleLegacy)
    திரும்பு initFromPPCDoubleDoubleLegacyAPInt(api);
  எனில் (Sem == &APFloatBase::semFloat8E5M2)
    திரும்பு initFromFloat8E5M2APInt(api);
  எனில் (Sem == &APFloatBase::semFloat8E5M2FNUZ)
    திரும்பு initFromFloat8E5M2FNUZAPInt(api);
  எனில் (Sem == &APFloatBase::semFloat8E4M3)
    திரும்பு initFromFloat8E4M3APInt(api);
  எனில் (Sem == &APFloatBase::semFloat8E4M3FN)
    திரும்பு initFromFloat8E4M3FNAPInt(api);
  எனில் (Sem == &APFloatBase::semFloat8E4M3FNUZ)
    திரும்பு initFromFloat8E4M3FNUZAPInt(api);
  எனில் (Sem == &APFloatBase::semFloat8E4M3B11FNUZ)
    திரும்பு initFromFloat8E4M3B11FNUZAPInt(api);
  எனில் (Sem == &APFloatBase::semFloat8E3M4)
    திரும்பு initFromFloat8E3M4APInt(api);
  எனில் (Sem == &APFloatBase::semFloatTF32)
    திரும்பு initFromFloatTF32APInt(api);
  எனில் (Sem == &APFloatBase::semFloat8E8M0FNU)
    திரும்பு initFromFloat8E8M0FNUAPInt(api);
  எனில் (Sem == &APFloatBase::semFloat8E5M3FNU)
    திரும்பு initFromFloat8E5M3FNUAPInt(api);
  எனில் (Sem == &APFloatBase::semFloat6E3M2FN)
    திரும்பு initFromFloat6E3M2FNAPInt(api);
  எனில் (Sem == &APFloatBase::semFloat6E2M3FN)
    திரும்பு initFromFloat6E2M3FNAPInt(api);
  எனில் (Sem == &APFloatBase::semFloat4E2M1FN)
    திரும்பு initFromFloat4E2M1FNAPInt(api);

  llvm_unreachable("unsupported semantics");
}

/// Make this number the largest magnitude normal number in the given
/// semantics.
வெறுமை IEEEFloat::makeLargest(தருக்க Negative) {
  எனில் (Negative && !semantics->hasSignedRepr)
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
  unsigned Part_மொத்தம் = part_மொத்தம்();
  memset(significand, 0xFF, அளவு(integerPart)*(Part_மொத்தம் - 1));

  // Set the high integerPart especially setting all unused top bits for
  // internal consistency.
  மாறிலி unsigned இலக்கம்_Unused_உயர்வு_Bits =
    Part_மொத்தம்*integerPartWidth - semantics->precision;
  significand[Part_மொத்தம் - 1] = (இலக்கம்_Unused_உயர்வு_Bits < integerPartWidth)
                                   ? (~integerPart(0) >> இலக்கம்_Unused_உயர்வு_Bits)
                                   : 0;
  எனில் (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly &&
      semantics->nanEncoding == fltNanEncoding::AllOnes &&
      (semantics->precision > 1))
    significand[0] &= ~integerPart(1);
}

/// Make this number the smallest magnitude denormal number in the given
/// semantics.
வெறுமை IEEEFloat::makeSmallest(தருக்க Negative) {
  எனில் (Negative && !semantics->hasSignedRepr)
    llvm_unreachable(
        "This floating point format does not support signed values");
  // We want (in interchange format):
  //   sign = {Negative}
  //   exponent = 0..0
  //   significand = 0..01
  category = fcNormal;
  sign = Negative;
  exponent = semantics->minExponent;
  APInt::tcSet(significandParts(), 1, part_மொத்தம்());
}

வெறுமை IEEEFloat::makeSmallestNormalized(தருக்க Negative) {
  எனில் (Negative && !semantics->hasSignedRepr)
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

IEEEFloat::IEEEFloat(மாறிலி fltSemantics &Sem, மாறிலி APInt &API) {
  initFromAPInt(&Sem, API);
}

IEEEFloat::IEEEFloat(தசம f) {
  initFromAPInt(&APFloatBase::semIEEEsingle, APInt::floatToBits(f));
}

IEEEFloat::IEEEFloat(இரட்டை d) {
  initFromAPInt(&APFloatBase::semIEEEdouble, APInt::doubleToBits(d));
}

நாமாகாஶ {
  வெறுமை append(SmallVectorImpl<எழுத்து> &Buffer, StringRef Str) {
    Buffer.append(Str.begin(), Str.end());
  }

  /// Removes data from the given significand until it is no more
  /// precise than is required for the desired precision.
  வெறுமை AdjustToPrecision(APInt &significand,
                         முழு &exp, unsigned FormatPrecision) {
    unsigned bits = significand.getActiveBits();

    // 196/59 is a very slight overestimate of lg_2(10).
    unsigned bitsRequired = (FormatPrecision * 196 + 58) / 59;

    எனில் (bits <= bitsRequired) திரும்பு;

    unsigned tensRemovable = (bits - bitsRequired) * 59 / 196;
    எனில் (!tensRemovable) திரும்பு;

    exp += tensRemovable;

    APInt வகுஎண்(significand.getBitWidth(), 1);
    APInt powten(significand.getBitWidth(), 10);
    போது (உண்மை) {
      எனில் (tensRemovable & 1)
        வகுஎண் *= powten;
      tensRemovable >>= 1;
      எனில் (!tensRemovable) நிறுத்து;
      powten *= powten;
    }

    significand = significand.udiv(வகுஎண்);

    // Truncate the significand down to its active bit count.
    significand = significand.trunc(significand.getActiveBits());
  }


  வெறுமை AdjustToPrecision(SmallVectorImpl<எழுத்து> &buffer,
                         முழு &exp, unsigned FormatPrecision) {
    unsigned N = buffer.size();
    எனில் (N <= FormatPrecision) திரும்பு;

    // The most significant figures are the last ones in the buffer.
    unsigned FirstSignificant = N - FormatPrecision;

    // Round.
    // FIXME: this probably shouldn't use 'round half up'.

    // Rounding down is just a truncation, except we also want to drop
    // trailing zeros from the new result.
    எனில் (buffer[FirstSignificant - 1] < '5') {
      போது (FirstSignificant < N && buffer[FirstSignificant] == '0')
        FirstSignificant++;

      exp += FirstSignificant;
      buffer.erase(&buffer[0], &buffer[FirstSignificant]);
      திரும்பு;
    }

    // Rounding up requires a decimal add-with-carry.  If we continue
    // the carry, the newly-introduced zeros will just be truncated.
    க்கு (unsigned I = FirstSignificant; I != N; ++I) {
      எனில் (buffer[I] == '9') {
        FirstSignificant++;
      } இல்லையெனில் {
        buffer[I]++;
        நிறுத்து;
      }
    }

    // If we carried through, we have exactly one digit of precision.
    எனில் (FirstSignificant == N) {
      exp += FirstSignificant;
      buffer.clear();
      buffer.தள்ளு_back('1');
      திரும்பு;
    }

    exp += FirstSignificant;
    buffer.erase(&buffer[0], &buffer[FirstSignificant]);
  }

  வெறுமை toStringImpl(SmallVectorImpl<எழுத்து> &Str, மாறிலி தருக்க isNeg, முழு exp,
                    APInt significand, unsigned FormatPrecision,
                    unsigned FormatMaxPadding, தருக்க TruncateZero) {
    மாறிலி முழு semanticsPrecision = significand.getBitWidth();

    எனில் (isNeg)
      Str.தள்ளு_back('-');

    // Set FormatPrecision if zero.  We want to do this before we
    // truncate trailing zeros, as those are part of the precision.
    எனில் (!FormatPrecision) {
      // We use enough digits so the number can be round-tripped back to an
      // APFloat. The formula comes from "How to Print Floating-Point Numbers
      // Accurately" by Steele and White.
      // FIXME: Using a formula based purely on the precision is conservative;
      // we can print fewer digits depending on the actual value being printed.

      // FormatPrecision = 2 + floor(significandBits / lg_2(10))
      FormatPrecision = 2 + semanticsPrecision * 59 / 196;
    }

    // Ignore trailing binary zeros.
    முழு trailingZeros = significand.countr_zero();
    exp += trailingZeros;
    significand.lshrInPlace(trailingZeros);

    // Change the exponent from 2^e to 10^e.
    எனில் (exp == 0) {
      // Nothing to do.
    } இல்லையெனில் எனில் (exp > 0) {
      // Just shift left.
      significand = significand.zext(semanticsPrecision + exp);
      significand <<= exp;
      exp = 0;
    } இல்லையெனில் { /* exp < 0 */
      முழு texp = -exp;

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
      போது (உண்மை) {
        எனில் (texp & 1)
          significand *= five_to_the_i;

        texp >>= 1;
        எனில் (!texp)
          நிறுத்து;
        five_to_the_i *= five_to_the_i;
      }
    }

    AdjustToPrecision(significand, exp, FormatPrecision);

    SmallVector<எழுத்து, 256> buffer;

    // Fill the buffer.
    unsigned precision = significand.getBitWidth();
    எனில் (precision < 4) {
      // We need enough precision to store the value 10.
      precision = 4;
      significand = significand.zext(precision);
    }
    APInt ten(precision, 10);
    APInt digit(precision, 0);

    தருக்க inTrail = உண்மை;
    போது (significand != 0) {
      // digit <- significand % 10
      // significand <- significand / 10
      APInt::udivrem(significand, ten, significand, digit);

      unsigned d = digit.get_ZExt_மதிப்பு();

      // Drop trailing zeros.
      எனில் (inTrail && !d)
        exp++;
      இல்லையெனில் {
        buffer.தள்ளு_back((எழுத்து) ('0' + d));
        inTrail = பொய்;
      }
    }

    ப்ரதிஜ்ஞா(!buffer.empty() && "no characters in buffer!");

    // Drop down to FormatPrecision.
    // TODO: don't do more precise calculations above than are required.
    AdjustToPrecision(buffer, exp, FormatPrecision);

    unsigned NDigits = buffer.size();

    // Check whether we should use scientific notation.
    தருக்க FormatScientific;
    எனில் (!FormatMaxPadding) {
      FormatScientific = உண்மை;
    } இல்லையெனில் {
      எனில் (exp >= 0) {
        // 765e3 --> 765000
        //              ^^^
        // But we shouldn't make the number look more precise than it is.
        FormatScientific = ((unsigned) exp > FormatMaxPadding ||
                            NDigits + (unsigned) exp > FormatPrecision);
      } இல்லையெனில் {
        // Power of the most significant digit.
        முழு MSD = exp + (முழு) (NDigits - 1);
        எனில் (MSD >= 0) {
          // 765e-2 == 7.65
          FormatScientific = பொய்;
        } இல்லையெனில் {
          // 765e-5 == 0.00765
          //           ^ ^^
          FormatScientific = ((unsigned) -MSD) > FormatMaxPadding;
        }
      }
    }

    // Scientific formatting is pretty straightforward.
    எனில் (FormatScientific) {
      exp += (NDigits - 1);

      Str.தள்ளு_back(buffer[NDigits-1]);
      Str.தள்ளு_back('.');
      எனில் (NDigits == 1 && TruncateZero)
        Str.தள்ளு_back('0');
      இல்லையெனில்
        க்கு (unsigned I = 1; I != NDigits; ++I)
          Str.தள்ளு_back(buffer[NDigits-1-I]);
      // Fill with zeros up to FormatPrecision.
      எனில் (!TruncateZero && FormatPrecision > NDigits - 1)
        Str.append(FormatPrecision - NDigits + 1, '0');
      // For !TruncateZero we use lower 'e'.
      Str.தள்ளு_back(TruncateZero ? 'E' : 'e');

      Str.தள்ளு_back(exp >= 0 ? '+' : '-');
      எனில் (exp < 0)
        exp = -exp;
      SmallVector<எழுத்து, 6> expbuf;
      செய் {
        expbuf.தள்ளு_back((எழுத்து) ('0' + (exp % 10)));
        exp /= 10;
      } போது (exp);
      // Exponent always at least two digits if we do not truncate zeros.
      எனில் (!TruncateZero && expbuf.size() < 2)
        expbuf.தள்ளு_back('0');
      க்கு (unsigned I = 0, E = expbuf.size(); I != E; ++I)
        Str.தள்ளு_back(expbuf[E-1-I]);
      திரும்பு;
    }

    // Non-scientific, positive exponents.
    எனில் (exp >= 0) {
      க்கு (unsigned I = 0; I != NDigits; ++I)
        Str.தள்ளு_back(buffer[NDigits-1-I]);
      க்கு (unsigned I = 0; I != (unsigned) exp; ++I)
        Str.தள்ளு_back('0');
      திரும்பு;
    }

    // Non-scientific, negative exponents.

    // The number of digits to the left of the decimal point.
    முழு NWholeDigits = exp + (முழு) NDigits;

    unsigned I = 0;
    எனில் (NWholeDigits > 0) {
      க்கு (; I != (unsigned) NWholeDigits; ++I)
        Str.தள்ளு_back(buffer[NDigits-I-1]);
      Str.தள்ளு_back('.');
    } இல்லையெனில் {
      unsigned NZeros = 1 + (unsigned) -NWholeDigits;

      Str.தள்ளு_back('0');
      Str.தள்ளு_back('.');
      க்கு (unsigned Z = 1; Z != NZeros; ++Z)
        Str.தள்ளு_back('0');
    }

    க்கு (; I != NDigits; ++I)
      Str.தள்ளு_back(buffer[NDigits-I-1]);

  }
} // namespace

வெறுமை IEEEFloat::toString(SmallVectorImpl<எழுத்து> &Str, unsigned FormatPrecision,
                         unsigned FormatMaxPadding, தருக்க TruncateZero) மாறிலி {
  தேர்வு (category) {
  நிலை fcInfinity:
    எனில் (isNegative())
      திரும்பு append(Str, "-Inf");
    இல்லையெனில்
      திரும்பு append(Str, "+Inf");

  நிலை fcNaN: திரும்பு append(Str, "NaN");

  நிலை fcZero:
    எனில் (isNegative())
      Str.தள்ளு_back('-');

    எனில் (!FormatMaxPadding) {
      எனில் (TruncateZero)
        append(Str, "0.0E+0");
      இல்லையெனில் {
        append(Str, "0.0");
        எனில் (FormatPrecision > 1)
          Str.append(FormatPrecision - 1, '0');
        append(Str, "e+00");
      }
    } இல்லையெனில் {
      Str.தள்ளு_back('0');
    }
    திரும்பு;

  நிலை fcNormal:
    நிறுத்து;
  }

  // Decompose the number into an APInt and an exponent.
  முழு exp = exponent - ((முழு) semantics->precision - 1);
  APInt significand(
      semantics->precision,
      ArrayRef(significandParts(), part_மொத்தம்_For_Bits(semantics->precision)));

  toStringImpl(Str, isNegative(), exp, significand, FormatPrecision,
               FormatMaxPadding, TruncateZero);

}

முழு IEEEFloat::getExactLog2Abs() மாறிலி {
  எனில் (!isFinite() || isZero())
    திரும்பு INT_MIN;

  மாறிலி integerPart *Parts = significandParts();
  மாறிலி முழு Part_மொத்தம் = part_மொத்தம்_For_Bits(semantics->precision);

  முழு எடு_மொத்தம் = 0;
  க்கு (முழு i = 0; i < Part_மொத்தம்; ++i) {
    எடு_மொத்தம் += llvm::popcount(Parts[i]);
    எனில் (எடு_மொத்தம் > 1)
      திரும்பு INT_MIN;
  }

  எனில் (exponent != semantics->minExponent)
    திரும்பு exponent;

  முழு CountrParts = 0;
  க்கு (முழு i = 0; i < Part_மொத்தம்;
       ++i, CountrParts += APInt::APINT_BITS_PER_WORD) {
    எனில் (Parts[i] != 0) {
      திரும்பு exponent - semantics->precision + CountrParts +
             llvm::countr_zero(Parts[i]) + 1;
    }
  }

  llvm_unreachable("didn't find the set bit");
}

தருக்க IEEEFloat::isSignaling() மாறிலி {
  எனில் (!isNaN())
    திரும்பு பொய்;
  எனில் (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly ||
      semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    திரும்பு பொய்;

  // IEEE-754R 2008 6.2.1: A signaling NaN bit string should be encoded with the
  // first bit of the trailing significand being 0.
  திரும்பு !APInt::tcExtractBit(significandParts(), semantics->precision - 2);
}

/// IEEE-754R 2008 5.3.1: nextUp/nextDown.
///
/// *NOTE* since nextDown(x) = -nextUp(-x), we only implement nextUp with
/// appropriate sign switching before/after the computation.
APFloat::opStatus IEEEFloat::next(தருக்க nextDown) {
  // If we are performing nextDown, swap sign so we have -x.
  எனில் (nextDown)
    changeSign();

  // Compute nextUp(x)
  opStatus result = opOK;

  // Handle each float category separately.
  தேர்வு (category) {
  நிலை fcInfinity:
    // nextUp(+inf) = +inf
    எனில் (!isNegative())
      நிறுத்து;
    // nextUp(-inf) = -getLargest()
    makeLargest(உண்மை);
    நிறுத்து;
  நிலை fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    எனில் (isSignaling()) {
      result = opInvalidOp;
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(பொய், isNegative(), nullptr);
    }
    நிறுத்து;
  நிலை fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(பொய்);
    நிறுத்து;
  நிலை fcNormal:
    // nextUp(-getSmallest()) = -0
    எனில் (isSmallest() && isNegative()) {
      APInt::tcSet(significandParts(), 0, part_மொத்தம்());
      category = fcZero;
      exponent = 0;
      எனில் (semantics->nanEncoding == fltNanEncoding::NegativeZero)
        sign = பொய்;
      எனில் (!semantics->hasZero)
        makeSmallestNormalized(பொய்);
      நிறுத்து;
    }

    எனில் (isLargest() && !isNegative()) {
      எனில் (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
        // nextUp(getLargest()) == NAN
        makeNaN();
        நிறுத்து;
      } இல்லையெனில் எனில் (semantics->nonFiniteBehavior ==
                 fltNonfiniteBehavior::FiniteOnly) {
        // nextUp(getLargest()) == getLargest()
        நிறுத்து;
      } இல்லையெனில் {
        // nextUp(getLargest()) == INFINITY
        APInt::tcSet(significandParts(), 0, part_மொத்தம்());
        category = fcInfinity;
        exponent = semantics->maxExponent + 1;
        நிறுத்து;
      }
    }

    // nextUp(normal) == normal + inc.
    எனில் (isNegative()) {
      // If we are negative, we need to decrement the significand.

      // We only cross a binade boundary that requires adjusting the exponent
      // if:
      //   1. exponent != semantics->minExponent. This implies we are not in the
      //   smallest binade or are dealing with denormals.
      //   2. Our significand excluding the integral bit is all zeros.
      தருக்க WillCrossBinadeBoundary =
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
      APInt::tcDecrement(Parts, part_மொத்தம்());

      எனில் (WillCrossBinadeBoundary) {
        // Our result is a normal number. Do the following:
        // 1. Set the integral bit to 1.
        // 2. Decrement the exponent.
        APInt::tcSetBit(Parts, semantics->precision - 1);
        exponent--;
      }
    } இல்லையெனில் {
      // If we are positive, we need to increment the significand.

      // We only cross a binade boundary that requires adjusting the exponent if
      // the input is not a denormal and all of said input's significand bits
      // are set. If all of said conditions are true: clear the significand, set
      // the integral bit to 1, and increment the exponent. If we have a
      // denormal always increment since moving denormals and the numbers in the
      // smallest normal binade have the same exponent in our representation.
      // If there are only exponents, any increment always crosses the
      // BinadeBoundary.
      தருக்க WillCrossBinadeBoundary = !APFloat::hasSignificand(*semantics) ||
                                     (!isDenormal() && isSignificandAllOnes());

      எனில் (WillCrossBinadeBoundary) {
        integerPart *Parts = significandParts();
        APInt::tcSet(Parts, 0, part_மொத்தம்());
        APInt::tcSetBit(Parts, semantics->precision - 1);
        ப்ரதிஜ்ஞா(exponent != semantics->maxExponent &&
               "We can not increment an exponent beyond the maxExponent allowed"
               " by the given floating point semantics.");
        exponent++;
      } இல்லையெனில் {
        incrementSignificand();
      }
    }
    நிறுத்து;
  }

  // If we are performing nextDown, swap sign so we have -nextUp(-x)
  எனில் (nextDown)
    changeSign();

  திரும்பு result;
}

APInt IEEEFloat::getNaNPayload() மாறிலி {
  ப்ரதிஜ்ஞா(isNaN() && "Can only be called on NaN values");
  // Number of bits in the payload, excluding the (maybe implied) integer bit.
  unsigned Bits = semantics->precision - 1;
  திரும்பு APInt(Bits, ArrayRef(significandParts(), part_மொத்தம்_For_Bits(Bits)));
}

APFloatBase::ExponentType IEEEFloat::exponentNaN() மாறிலி {
  திரும்பு ::exponentNaN(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentInf() மாறிலி {
  திரும்பு ::exponentInf(*semantics);
}

APFloatBase::ExponentType IEEEFloat::exponentZero() மாறிலி {
  திரும்பு ::exponentZero(*semantics);
}

வெறுமை IEEEFloat::makeInf(தருக்க Negative) {
  எனில் (semantics->nonFiniteBehavior == fltNonfiniteBehavior::FiniteOnly)
    llvm_unreachable("This floating point format does not support Inf");

  எனில் (semantics->nonFiniteBehavior == fltNonfiniteBehavior::NanOnly) {
    // There is no Inf, so make NaN instead.
    makeNaN(பொய், Negative);
    திரும்பு;
  }
  category = fcInfinity;
  sign = Negative;
  exponent = exponentInf();
  APInt::tcSet(significandParts(), 0, part_மொத்தம்());
}

வெறுமை IEEEFloat::makeZero(தருக்க Negative) {
  எனில் (!semantics->hasZero)
    llvm_unreachable("This floating point format does not support Zero");

  category = fcZero;
  sign = Negative;
  எனில் (semantics->nanEncoding == fltNanEncoding::NegativeZero) {
    // Merge negative zero to positive because 0b10000...000 is used for NaN
    sign = பொய்;
  }
  exponent = exponentZero();
  APInt::tcSet(significandParts(), 0, part_மொத்தம்());
}

வெறுமை IEEEFloat::makeQuiet() {
  ப்ரதிஜ்ஞா(isNaN());
  எனில் (semantics->nonFiniteBehavior != fltNonfiniteBehavior::NanOnly)
    APInt::tcSetBit(significandParts(), semantics->precision - 2);
}

முழு ilogb(மாறிலி IEEEFloat &Arg) {
  எனில் (Arg.isNaN())
    திரும்பு APFloat::IEK_NaN;
  எனில் (Arg.isZero())
    திரும்பு APFloat::IEK_Zero;
  எனில் (Arg.isInfinity())
    திரும்பு APFloat::IEK_Inf;
  எனில் (!Arg.isDenormal())
    திரும்பு Arg.exponent;

  IEEEFloat Normalized(Arg);
  முழு SignificandBits = Arg.getSemantics().precision - 1;

  Normalized.exponent += SignificandBits;
  Normalized.normalize(APFloat::rmNearestTiesToEven, lfExactlyZero);
  திரும்பு Normalized.exponent - SignificandBits;
}

IEEEFloat scalbn(IEEEFloat X, முழு Exp, roundingMode RoundingMode) {
  auto MaxExp = X.getSemantics().maxExponent;
  auto MinExp = X.getSemantics().minExponent;

  // If Exp is wildly out-of-scale, simply adding it to X.exponent will
  // overflow; clamp it to a safe range before adding, but ensure that the range
  // is large enough that the clamp does not change the result. The range we
  // need to support is the difference between the largest possible exponent and
  // the normalized exponent of half the smallest denormal.

  முழு SignificandBits = X.getSemantics().precision - 1;
  முழு MaxIncrement = MaxExp - (MinExp - SignificandBits) + 1;

  // Clamp to one past the range ends to let normalize handle overlflow.
  X.exponent += std::clamp(Exp, -MaxIncrement - 1, MaxIncrement);
  X.normalize(RoundingMode, lfExactlyZero);
  எனில் (X.isNaN())
    X.makeQuiet();
  திரும்பு X;
}

IEEEFloat frexp(மாறிலி IEEEFloat &Val, முழு &Exp, roundingMode RM) {
  Exp = ilogb(Val);

  // Quiet signalling nans.
  எனில் (Exp == APFloat::IEK_NaN) {
    IEEEFloat Quiet(Val);
    Quiet.makeQuiet();
    திரும்பு Quiet;
  }

  எனில் (Exp == APFloat::IEK_Inf)
    திரும்பு Val;

  // 1 is added because frexp is defined to return a normalized fraction in
  // +/-[0.5, 1.0), rather than the usual +/-[1.0, 2.0).
  Exp = Exp == APFloat::IEK_Zero ? 0 : Exp + 1;
  திரும்பு scalbn(Val, -Exp, RM);
}

DoubleAPFloat::DoubleAPFloat(மாறிலி fltSemantics &S)
    : Semantics(&S),
      Floats(புதிய APFloat[2]{APFloat(APFloatBase::semIEEEdouble),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(மாறிலி fltSemantics &S, uninitializedTag)
    : Semantics(&S), Floats(புதிய APFloat[2]{
                         APFloat(APFloatBase::semIEEEdouble, uninitialized),
                         APFloat(APFloatBase::semIEEEdouble, uninitialized)}) {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(மாறிலி fltSemantics &S, integerPart I)
    : Semantics(&S),
      Floats(புதிய APFloat[2]{APFloat(APFloatBase::semIEEEdouble, I),
                            APFloat(APFloatBase::semIEEEdouble)}) {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(மாறிலி fltSemantics &S, மாறிலி APInt &I)
    : Semantics(&S),
      Floats(புதிய APFloat[2]{
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[0])),
          APFloat(APFloatBase::semIEEEdouble, APInt(64, I.getRawData()[1]))}) {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(மாறிலி fltSemantics &S, APFloat &&First,
                             APFloat &&Second)
    : Semantics(&S),
      Floats(புதிய APFloat[2]{std::move(First), std::move(Second)}) {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble);
  ப்ரதிஜ்ஞா(&Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  ப்ரதிஜ்ஞா(&Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
}

DoubleAPFloat::DoubleAPFloat(மாறிலி DoubleAPFloat &RHS)
    : Semantics(RHS.Semantics),
      Floats(RHS.Floats ? புதிய APFloat[2]{APFloat(RHS.Floats[0]),
                                         APFloat(RHS.Floats[1])}
                        : nullptr) {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat::DoubleAPFloat(DoubleAPFloat &&RHS)
    : Semantics(RHS.Semantics), Floats(RHS.Floats) {
  RHS.Semantics = &APFloatBase::semBogus;
  RHS.Floats = nullptr;
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble);
}

DoubleAPFloat &DoubleAPFloat::operator=(மாறிலி DoubleAPFloat &RHS) {
  எனில் (Semantics == RHS.Semantics && RHS.Floats) {
    Floats[0] = RHS.Floats[0];
    Floats[1] = RHS.Floats[1];
  } இல்லையெனில் எனில் (இது != &RHS) {
    இது->~DoubleAPFloat();
    புதிய (இது) DoubleAPFloat(RHS);
  }
  திரும்பு *இது;
}

// Returns a result such that:
// 1. abs(Lo) <= ulp(Hi)/2
// 2. Hi == RTNE(Hi + Lo)
// 3. Hi + Lo == X + Y
//
// Requires that log2(X) >= log2(Y).
நிலையான std::pair<APFloat, APFloat> fastTwoSum(APFloat X, APFloat Y) {
  எனில் (!X.isFinite())
    திரும்பு {X, APFloat::getZero(X.getSemantics(), /*Negative=*/பொய்)};
  APFloat Hi = X + Y;
  APFloat Delta = Hi - X;
  APFloat Lo = Y - Delta;
  திரும்பு {Hi, Lo};
}

// Implement addition, subtraction, multiplication and division based on:
// "Software for Doubled-Precision Floating-Point Computations",
// by Seppo Linnainmaa, ACM TOMS vol 7 no 3, September 1981, pages 272-283.
APFloat::opStatus DoubleAPFloat::addImpl(மாறிலி APFloat &a, மாறிலி APFloat &aa,
                                         மாறிலி APFloat &c, மாறிலி APFloat &cc,
                                         roundingMode RM) {
  முழு Status = opOK;
  APFloat z = a;
  Status |= z.add(c, RM);
  எனில் (!z.isFinite()) {
    எனில் (!z.isInfinity()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ பொய்);
      திரும்பு (opStatus)Status;
    }
    Status = opOK;
    auto AComparedToC = a.compare_Absolute_மதிப்பு(c);
    z = cc;
    Status |= z.add(aa, RM);
    எனில் (AComparedToC == APFloat::cmpGreaterThan) {
      // z = cc + aa + c + a;
      Status |= z.add(c, RM);
      Status |= z.add(a, RM);
    } இல்லையெனில் {
      // z = cc + aa + a + c;
      Status |= z.add(a, RM);
      Status |= z.add(c, RM);
    }
    எனில் (!z.isFinite()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ பொய்);
      திரும்பு (opStatus)Status;
    }
    Floats[0] = z;
    APFloat zz = aa;
    Status |= zz.add(cc, RM);
    எனில் (AComparedToC == APFloat::cmpGreaterThan) {
      // Floats[1] = a - z + c + zz;
      Floats[1] = a;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(c, RM);
      Status |= Floats[1].add(zz, RM);
    } இல்லையெனில் {
      // Floats[1] = c - z + a + zz;
      Floats[1] = c;
      Status |= Floats[1].subtract(z, RM);
      Status |= Floats[1].add(a, RM);
      Status |= Floats[1].add(zz, RM);
    }
  } இல்லையெனில் {
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
    எனில் (zz.isZero() && !zz.isNegative()) {
      Floats[0] = std::move(z);
      Floats[1].makeZero(/* Neg = */ பொய்);
      திரும்பு opOK;
    }
    Floats[0] = z;
    Status |= Floats[0].add(zz, RM);
    எனில் (!Floats[0].isFinite()) {
      Floats[1].makeZero(/* Neg = */ பொய்);
      திரும்பு (opStatus)Status;
    }
    Floats[1] = std::move(z);
    Status |= Floats[1].subtract(Floats[0], RM);
    Status |= Floats[1].add(zz, RM);
  }
  திரும்பு (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::addWithSpecial(மாறிலி DoubleAPFloat &LHS,
                                                மாறிலி DoubleAPFloat &RHS,
                                                DoubleAPFloat &Out,
                                                roundingMode RM) {
  எனில் (LHS.getCategory() == fcNaN) {
    Out = LHS;
    திரும்பு opOK;
  }
  எனில் (RHS.getCategory() == fcNaN) {
    Out = RHS;
    திரும்பு opOK;
  }
  எனில் (LHS.getCategory() == fcZero) {
    Out = RHS;
    திரும்பு opOK;
  }
  எனில் (RHS.getCategory() == fcZero) {
    Out = LHS;
    திரும்பு opOK;
  }
  எனில் (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcInfinity &&
      LHS.isNegative() != RHS.isNegative()) {
    Out.makeNaN(பொய், Out.isNegative(), nullptr);
    திரும்பு opInvalidOp;
  }
  எனில் (LHS.getCategory() == fcInfinity) {
    Out = LHS;
    திரும்பு opOK;
  }
  எனில் (RHS.getCategory() == fcInfinity) {
    Out = RHS;
    திரும்பு opOK;
  }
  ப்ரதிஜ்ஞா(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal);

  APFloat A(LHS.Floats[0]), AA(LHS.Floats[1]), C(RHS.Floats[0]),
      CC(RHS.Floats[1]);
  ப்ரதிஜ்ஞா(&A.getSemantics() == &APFloatBase::semIEEEdouble);
  ப்ரதிஜ்ஞா(&AA.getSemantics() == &APFloatBase::semIEEEdouble);
  ப்ரதிஜ்ஞா(&C.getSemantics() == &APFloatBase::semIEEEdouble);
  ப்ரதிஜ்ஞா(&CC.getSemantics() == &APFloatBase::semIEEEdouble);
  ப்ரதிஜ்ஞா(&Out.Floats[0].getSemantics() == &APFloatBase::semIEEEdouble);
  ப்ரதிஜ்ஞா(&Out.Floats[1].getSemantics() == &APFloatBase::semIEEEdouble);
  திரும்பு Out.addImpl(A, AA, C, CC, RM);
}

APFloat::opStatus DoubleAPFloat::add(மாறிலி DoubleAPFloat &RHS,
                                     roundingMode RM) {
  திரும்பு addWithSpecial(*இது, RHS, *இது, RM);
}

APFloat::opStatus DoubleAPFloat::subtract(மாறிலி DoubleAPFloat &RHS,
                                          roundingMode RM) {
  changeSign();
  auto Ret = add(RHS, RM);
  changeSign();
  திரும்பு Ret;
}

APFloat::opStatus DoubleAPFloat::multiply(மாறிலி DoubleAPFloat &RHS,
                                          APFloat::roundingMode RM) {
  மாறிலி auto &LHS = *இது;
  auto &Out = *இது;
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
  எனில் (LHS.getCategory() == fcNaN) {
    Out = LHS;
    திரும்பு opOK;
  }
  எனில் (RHS.getCategory() == fcNaN) {
    Out = RHS;
    திரும்பு opOK;
  }
  எனில் ((LHS.getCategory() == fcZero && RHS.getCategory() == fcInfinity) ||
      (LHS.getCategory() == fcInfinity && RHS.getCategory() == fcZero)) {
    Out.makeNaN(பொய், பொய், nullptr);
    திரும்பு opOK;
  }
  எனில் (LHS.getCategory() == fcZero || LHS.getCategory() == fcInfinity) {
    Out = LHS;
    திரும்பு opOK;
  }
  எனில் (RHS.getCategory() == fcZero || RHS.getCategory() == fcInfinity) {
    Out = RHS;
    திரும்பு opOK;
  }
  ப்ரதிஜ்ஞா(LHS.getCategory() == fcNormal && RHS.getCategory() == fcNormal &&
         "Special cases not handled exhaustively");

  முழு Status = opOK;
  APFloat A = Floats[0], B = Floats[1], C = RHS.Floats[0], D = RHS.Floats[1];
  // t = a * c
  APFloat T = A;
  Status |= T.multiply(C, RM);
  எனில் (!T.isFiniteNonZero()) {
    Floats[0] = std::move(T);
    Floats[1].makeZero(/* Neg = */ பொய்);
    திரும்பு (opStatus)Status;
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
  எனில் (!U.isFinite()) {
    Floats[1].makeZero(/* Neg = */ பொய்);
  } இல்லையெனில் {
    // Floats[1] = (t - u) + tau
    Status |= T.subtract(U, RM);
    Status |= T.add(Tau, RM);
    Floats[1] = std::move(T);
  }
  திரும்பு (opStatus)Status;
}

APFloat::opStatus DoubleAPFloat::divide(மாறிலி DoubleAPFloat &RHS,
                                        APFloat::roundingMode RM) {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.divide(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()), RM);
  *இது = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  திரும்பு Ret;
}

APFloat::opStatus DoubleAPFloat::remainder(மாறிலி DoubleAPFloat &RHS) {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.remainder(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *இது = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  திரும்பு Ret;
}

APFloat::opStatus DoubleAPFloat::mod(மாறிலி DoubleAPFloat &RHS) {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.mod(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, RHS.bitcastToAPInt()));
  *இது = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  திரும்பு Ret;
}

APFloat::opStatus
DoubleAPFloat::fusedMultiplyAdd(மாறிலி DoubleAPFloat &Multiplicand,
                                மாறிலி DoubleAPFloat &Addend,
                                APFloat::roundingMode RM) {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt());
  auto Ret = Tmp.fusedMultiplyAdd(
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy,
              Multiplicand.bitcastToAPInt()),
      APFloat(APFloatBase::semPPCDoubleDoubleLegacy, Addend.bitcastToAPInt()),
      RM);
  *இது = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  திரும்பு Ret;
}

APFloat::opStatus DoubleAPFloat::roundToIntegral(APFloat::roundingMode RM) {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  மாறிலி APFloat &Hi = getFirst();
  மாறிலி APFloat &Lo = getSecond();

  APFloat RoundedHi = Hi;
  மாறிலி opStatus HiStatus = RoundedHi.roundToIntegral(RM);

  // We can reduce the problem to just the high part if the input:
  // 1. Represents a non-finite value.
  // 2. Has a component which is zero.
  எனில் (!Hi.isFiniteNonZero() || Lo.isZero()) {
    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/பொய்);
    திரும்பு HiStatus;
  }

  // Adjust `Rounded` in the direction of `TieBreaker` if `ToRound` was at a
  // halfway point.
  auto RoundToNearestHelper = [](APFloat ToRound, APFloat Rounded,
                                 APFloat TieBreaker) {
    // RoundingError tells us which direction we rounded:
    //   - RoundingError > 0: we rounded up.
    //   - RoundingError < 0: we rounded down.
    // Sterbenz' lemma ensures that RoundingError is exact.
    மாறிலி APFloat RoundingError = Rounded - ToRound;
    எனில் (TieBreaker.isNonZero() &&
        TieBreaker.isNegative() != RoundingError.isNegative() &&
        abs(RoundingError).is_Exactly_மதிப்பு(0.5))
      Rounded.add(
          APFloat::getOne(Rounded.getSemantics(), TieBreaker.isNegative()),
          rmNearestTiesToEven);
    திரும்பு Rounded;
  };

  // Case 1: Hi is not an integer.
  // Special cases are for rounding modes that are sensitive to ties.
  எனில் (RoundedHi != Hi) {
    // We need to consider the case where Hi was between two integers and the
    // rounding mode broke the tie when, in fact, Lo may have had a different
    // sign than Hi.
    எனில் (RM == rmNearestTiesToAway || RM == rmNearestTiesToEven)
      RoundedHi = RoundToNearestHelper(Hi, RoundedHi, Lo);

    Floats[0] = std::move(RoundedHi);
    Floats[1].makeZero(/*Neg=*/பொய்);
    திரும்பு HiStatus;
  }

  // Case 2: Hi is an integer.
  // Special cases are for rounding modes which are rounding towards or away from zero.
  RoundingMode LoRoundingMode;
  எனில் (RM == rmTowardZero)
    // When our input is positive, we want the Lo component rounded toward
    // negative infinity to get the smallest result magnitude. Likewise,
    // negative inputs want the Lo component rounded toward positive infinity.
    LoRoundingMode = isNegative() ? rmTowardPositive : rmTowardNegative;
  இல்லையெனில்
    LoRoundingMode = RM;

  APFloat RoundedLo = Lo;
  மாறிலி opStatus LoStatus = RoundedLo.roundToIntegral(LoRoundingMode);
  எனில் (LoRoundingMode == rmNearestTiesToAway)
    // We need to consider the case where Lo was between two integers and the
    // rounding mode broke the tie when, in fact, Hi may have had a different
    // sign than Lo.
    RoundedLo = RoundToNearestHelper(Lo, RoundedLo, Hi);

  // We must ensure that the final result has no overlap between the two APFloat values.
  std::tie(RoundedHi, RoundedLo) = fastTwoSum(RoundedHi, RoundedLo);

  Floats[0] = std::move(RoundedHi);
  Floats[1] = std::move(RoundedLo);
  திரும்பு LoStatus;
}

வெறுமை DoubleAPFloat::changeSign() {
  Floats[0].changeSign();
  Floats[1].changeSign();
}

APFloat::cmpResult
DoubleAPFloat::compare_Absolute_மதிப்பு(மாறிலி DoubleAPFloat &RHS) மாறிலி {
  // Compare absolute values of the high parts.
  மாறிலி cmpResult HiPartCmp = Floats[0].compare_Absolute_மதிப்பு(RHS.Floats[0]);
  எனில் (HiPartCmp != cmpEqual)
    திரும்பு HiPartCmp;

  // Zero, regardless of sign, is equal.
  எனில் (Floats[1].isZero() && RHS.Floats[1].isZero())
    திரும்பு cmpEqual;

  // At this point, |this->Hi| == |RHS.Hi|.
  // The magnitude is |Hi+Lo| which is Hi+|Lo| if signs of Hi and Lo are the
  // same, and Hi-|Lo| if signs are different.
  மாறிலி தருக்க ThisIsSubtractive =
      Floats[0].isNegative() != Floats[1].isNegative();
  மாறிலி தருக்க RHSIsSubtractive =
      RHS.Floats[0].isNegative() != RHS.Floats[1].isNegative();

  // Case 1: The low part of 'this' is zero.
  எனில் (Floats[1].isZero())
    // We are comparing |Hi| vs. |Hi| ± |RHS.Lo|.
    // If RHS is subtractive, its magnitude is smaller.
    // If RHS is additive, its magnitude is larger.
    திரும்பு RHSIsSubtractive ? cmpGreaterThan : cmpLessThan;

  // Case 2: The low part of 'RHS' is zero (and we know 'this' is not).
  எனில் (RHS.Floats[1].isZero())
    // We are comparing |Hi| ± |This.Lo| vs. |Hi|.
    // If 'this' is subtractive, its magnitude is smaller.
    // If 'this' is additive, its magnitude is larger.
    திரும்பு ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // If their natures differ, the additive one is larger.
  எனில் (ThisIsSubtractive != RHSIsSubtractive)
    திரும்பு ThisIsSubtractive ? cmpLessThan : cmpGreaterThan;

  // Case 3: Both are additive (Hi+|Lo|) or both are subtractive (Hi-|Lo|).
  // The comparison now depends on the magnitude of the low parts.
  மாறிலி cmpResult LoPartCmp = Floats[1].compare_Absolute_மதிப்பு(RHS.Floats[1]);

  எனில் (ThisIsSubtractive) {
    // Both are subtractive (Hi-|Lo|), so the comparison of |Lo| is inverted.
    எனில் (LoPartCmp == cmpLessThan)
      திரும்பு cmpGreaterThan;
    எனில் (LoPartCmp == cmpGreaterThan)
      திரும்பு cmpLessThan;
  }

  // If additive, the comparison of |Lo| is direct.
  // If equal, they are equal.
  திரும்பு LoPartCmp;
}

APFloat::fltCategory DoubleAPFloat::getCategory() மாறிலி {
  திரும்பு Floats[0].getCategory();
}

தருக்க DoubleAPFloat::isNegative() மாறிலி { திரும்பு Floats[0].isNegative(); }

வெறுமை DoubleAPFloat::makeInf(தருக்க Neg) {
  Floats[0].makeInf(Neg);
  Floats[1].makeZero(/* Neg = */ பொய்);
}

வெறுமை DoubleAPFloat::makeZero(தருக்க Neg) {
  Floats[0].makeZero(Neg);
  Floats[1].makeZero(/* Neg = */ பொய்);
}

வெறுமை DoubleAPFloat::makeLargest(தருக்க Neg) {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7fefffffffffffffull));
  Floats[1] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x7c8ffffffffffffeull));
  எனில் (Neg)
    changeSign();
}

வெறுமை DoubleAPFloat::makeSmallest(தருக்க Neg) {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0].makeSmallest(Neg);
  Floats[1].makeZero(/* Neg = */ பொய்);
}

வெறுமை DoubleAPFloat::makeSmallestNormalized(தருக்க Neg) {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  Floats[0] =
      APFloat(APFloatBase::semIEEEdouble, APInt(64, 0x0360000000000000ull));
  எனில் (Neg)
    Floats[0].changeSign();
  Floats[1].makeZero(/* Neg = */ பொய்);
}

வெறுமை DoubleAPFloat::makeNaN(தருக்க SNaN, தருக்க Neg, மாறிலி APInt *fill) {
  Floats[0].makeNaN(SNaN, Neg, fill);
  Floats[1].makeZero(/* Neg = */ பொய்);
}

APFloat::cmpResult DoubleAPFloat::compare(மாறிலி DoubleAPFloat &RHS) மாறிலி {
  auto Result = Floats[0].compare(RHS.Floats[0]);
  // |Float[0]| > |Float[1]|
  எனில் (Result == APFloat::cmpEqual)
    திரும்பு Floats[1].compare(RHS.Floats[1]);
  திரும்பு Result;
}

தருக்க DoubleAPFloat::bitwiseIsEqual(மாறிலி DoubleAPFloat &RHS) மாறிலி {
  திரும்பு Floats[0].bitwiseIsEqual(RHS.Floats[0]) &&
         Floats[1].bitwiseIsEqual(RHS.Floats[1]);
}

hash_code hash_மதிப்பு(மாறிலி DoubleAPFloat &Arg) {
  எனில் (Arg.Floats)
    திரும்பு hash_combine(hash_மதிப்பு(Arg.Floats[0]), hash_மதிப்பு(Arg.Floats[1]));
  திரும்பு hash_combine(Arg.Semantics);
}

APInt DoubleAPFloat::bitcastToAPInt() மாறிலி {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  uint64_t Data[] = {
      Floats[0].bitcastToAPInt().getRawData()[0],
      Floats[1].bitcastToAPInt().getRawData()[0],
  };
  திரும்பு APInt(128, Data);
}

Expected<APFloat::opStatus> DoubleAPFloat::convertFromString(StringRef S,
                                                             roundingMode RM) {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat Tmp(APFloatBase::semPPCDoubleDoubleLegacy);
  auto Ret = Tmp.convertFromString(S, RM);
  *இது = DoubleAPFloat(APFloatBase::semPPCDoubleDouble, Tmp.bitcastToAPInt());
  திரும்பு Ret;
}

// The double-double lattice of values corresponds to numbers which obey:
// - abs(lo) <= 1/2 * ulp(hi)
// - roundTiesToEven(hi + lo) == hi
//
// nextUp must choose the smallest output > input that follows these rules.
// nexDown must choose the largest output < input that follows these rules.
APFloat::opStatus DoubleAPFloat::next(தருக்க nextDown) {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  // nextDown(x) = -nextUp(-x)
  எனில் (nextDown) {
    changeSign();
    APFloat::opStatus Result = next(/*nextDown=*/பொய்);
    changeSign();
    திரும்பு Result;
  }
  தேர்வு (getCategory()) {
  நிலை fcInfinity:
    // nextUp(+inf) = +inf
    // nextUp(-inf) = -getLargest()
    எனில் (isNegative())
      makeLargest(உண்மை);
    திரும்பு opOK;

  நிலை fcNaN:
    // IEEE-754R 2008 6.2 Par 2: nextUp(sNaN) = qNaN. Set Invalid flag.
    // IEEE-754R 2008 6.2: nextUp(qNaN) = qNaN. Must be identity so we do not
    //                     change the payload.
    எனில் (getFirst().isSignaling()) {
      // For consistency, propagate the sign of the sNaN to the qNaN.
      makeNaN(பொய், isNegative(), nullptr);
      திரும்பு opInvalidOp;
    }
    திரும்பு opOK;

  நிலை fcZero:
    // nextUp(pm 0) = +getSmallest()
    makeSmallest(பொய்);
    திரும்பு opOK;

  நிலை fcNormal:
    நிறுத்து;
  }

  மாறிலி APFloat &HiOld = getFirst();
  மாறிலி APFloat &LoOld = getSecond();

  APFloat NextLo = LoOld;
  NextLo.next(/*nextDown=*/பொய்);

  // We want to admit values where:
  // 1. abs(Lo) <= ulp(Hi)/2
  // 2. Hi == RTNE(Hi + lo)
  auto InLattice = [](மாறிலி APFloat &Hi, மாறிலி APFloat &Lo) {
    திரும்பு Hi + Lo == Hi;
  };

  // Check if (HiOld, nextUp(LoOld) is in the lattice.
  எனில் (InLattice(HiOld, NextLo)) {
    // Yes, the result is (HiOld, nextUp(LoOld)).
    Floats[1] = std::move(NextLo);

    // TODO: Because we currently rely on semPPCDoubleDoubleLegacy, our maximum
    // value is defined to have exactly 106 bits of precision. This limitation
    // results in semPPCDoubleDouble being unable to reach its maximum canonical
    // value.
    DoubleAPFloat Largest{*Semantics, uninitialized};
    Largest.makeLargest(/*Neg=*/பொய்);
    எனில் (compare(Largest) == cmpGreaterThan)
      makeInf(/*Neg=*/பொய்);

    திரும்பு opOK;
  }

  // Now we need to handle the cases where (HiOld, nextUp(LoOld)) is not the
  // correct result. We know the new hi component will be nextUp(HiOld) but our
  // lattice rules make it a little ambiguous what the correct NextLo must be.
  APFloat NextHi = HiOld;
  NextHi.next(/*nextDown=*/பொய்);

  // nextUp(getLargest()) == INFINITY
  எனில் (NextHi.isInfinity()) {
    makeInf(/*Neg=*/பொய்);
    திரும்பு opOK;
  }

  // IEEE 754-2019 5.3.1:
  // "If x is the negative number of least magnitude in x's format, nextUp(x) is
  // -0."
  எனில் (NextHi.isZero()) {
    makeZero(/*Neg=*/உண்மை);
    திரும்பு opOK;
  }

  // abs(NextLo) must be <= ulp(NextHi)/2. We want NextLo to be as close to
  // negative infinity as possible.
  NextLo = neg(scalbn(harrisonUlp(NextHi), -1, rmTowardZero));
  எனில் (!InLattice(NextHi, NextLo))
    // RTNE may mean that Lo must be < ulp(NextHi) / 2 so we bump NextLo.
    NextLo.next(/*nextDown=*/பொய்);

  Floats[0] = std::move(NextHi);
  Floats[1] = std::move(NextLo);

  திரும்பு opOK;
}

APFloat::opStatus DoubleAPFloat::convertToSignExtendedInteger(
    MutableArrayRef<integerPart> உள்ளீடு, unsigned முழு Width, தருக்க IsSigned,
    roundingMode RM, தருக்க *IsExact) மாறிலி {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");

  // If Hi is not finite, or Lo is zero, the value is entirely represented
  // by Hi. Delegate to the simpler single-APFloat conversion.
  எனில் (!getFirst().isFiniteNonZero() || getSecond().isZero())
    திரும்பு getFirst().convertToInteger(உள்ளீடு, Width, IsSigned, RM, IsExact);

  // First, round the full double-double value to an integral value. This
  // simplifies the rest of the function, as we no longer need to consider
  // fractional parts.
  *IsExact = பொய்;
  DoubleAPFloat Integral = *இது;
  மாறிலி opStatus RoundStatus = Integral.roundToIntegral(RM);
  எனில் (RoundStatus == opInvalidOp)
    திரும்பு opInvalidOp;
  மாறிலி APFloat &IntegralHi = Integral.getFirst();
  மாறிலி APFloat &IntegralLo = Integral.getSecond();

  // If rounding results in either component being zero, the sum is trivial.
  // Delegate to the simpler single-APFloat conversion.
  தருக்க HiIsExact;
  எனில் (IntegralHi.isZero() || IntegralLo.isZero()) {
    மாறிலி opStatus HiStatus =
        IntegralHi.convertToInteger(உள்ளீடு, Width, IsSigned, RM, &HiIsExact);
    // The conversion from an integer-valued float to an APInt may fail if the
    // result would be out of range.  Regardless, taking this path is only
    // possible if rounding occurred during the initial `roundToIntegral`.
    திரும்பு HiStatus == opOK ? opInexact : HiStatus;
  }

  // A negative number cannot be represented by an unsigned integer.
  // Since a double-double is canonical, if Hi is negative, the sum is negative.
  எனில் (!IsSigned && IntegralHi.isNegative())
    திரும்பு opInvalidOp;

  // Handle the special boundary case where |Hi| is exactly the power of two
  // that marks the edge of the integer's range (e.g., 2^63 for int64_t). In
  // this situation, Hi itself won't fit, but the sum Hi + Lo might.
  // `PositiveOverflowWidth` is the bit number for this boundary (N-1 for
  // signed, N for unsigned).
  தருக்க LoIsExact;
  மாறிலி முழு HiExactLog2 = IntegralHi.getExactLog2Abs();
  மாறிலி unsigned PositiveOverflowWidth = IsSigned ? Width - 1 : Width;
  எனில் (HiExactLog2 >= 0 &&
      static_cast<unsigned>(HiExactLog2) == PositiveOverflowWidth) {
    // If Hi and Lo have the same sign, |Hi + Lo| > |Hi|, so the sum is
    // guaranteed to overflow. E.g., for uint128_t, (2^128, 1) overflows.
    எனில் (IntegralHi.isNegative() == IntegralLo.isNegative())
      திரும்பு opInvalidOp;

    // If the signs differ, the sum will fit. We can compute the result using
    // properties of two's complement arithmetic without a wide intermediate
    // integer. E.g., for uint128_t, (2^128, -1) should be 2^128 - 1.
    மாறிலி opStatus LoStatus = IntegralLo.convertToInteger(
        உள்ளீடு, Width, /*IsSigned=*/உண்மை, RM, &LoIsExact);
    எனில் (LoStatus == opInvalidOp)
      திரும்பு opInvalidOp;

    // Adjust the bit pattern of Lo to account for Hi's value:
    //  - For unsigned (Hi=2^Width): `2^Width + Lo` in `Width`-bit
    //    arithmetic is equivalent to just `Lo`. The conversion of `Lo` above
    //    already produced the correct final bit pattern.
    //  - For signed (Hi=2^(Width-1)): The sum `2^(Width-1) + Lo` (where Lo<0)
    //    can be computed by taking the two's complement pattern for `Lo` and
    //    clearing the sign bit.
    எனில் (IsSigned && !IntegralHi.isNegative())
      APInt::tcClearBit(உள்ளீடு.data(), PositiveOverflowWidth);
    *IsExact = RoundStatus == opOK;
    திரும்பு RoundStatus;
  }

  // Convert Hi into an integer.  This may not fit but that is OK: we know that
  // Hi + Lo would not fit either in this situation.
  மாறிலி opStatus HiStatus = IntegralHi.convertToInteger(
      உள்ளீடு, Width, IsSigned, rmTowardZero, &HiIsExact);
  எனில் (HiStatus == opInvalidOp)
    திரும்பு HiStatus;

  // Convert Lo into a temporary integer of the same width.
  APSInt LoResult{Width, /*isUnsigned=*/!IsSigned};
  மாறிலி opStatus LoStatus =
      IntegralLo.convertToInteger(LoResult, rmTowardZero, &LoIsExact);
  எனில் (LoStatus == opInvalidOp)
    திரும்பு LoStatus;

  // Add Lo to Hi. This addition is guaranteed not to overflow because of the
  // double-double canonicalization rule (`|Lo| <= ulp(Hi)/2`). The only case
  // where the sum could cross the integer type's boundary is when Hi is a
  // power of two, which is handled by the special case block above.
  APInt::tcAdd(உள்ளீடு.data(), LoResult.getRawData(), /*carry=*/0, உள்ளீடு.size());

  *IsExact = RoundStatus == opOK;
  திரும்பு RoundStatus;
}

APFloat::opStatus
DoubleAPFloat::convertToInteger(MutableArrayRef<integerPart> உள்ளீடு,
                                unsigned முழு Width, தருக்க IsSigned,
                                roundingMode RM, தருக்க *IsExact) மாறிலி {
  opStatus FS =
      convertToSignExtendedInteger(உள்ளீடு, Width, IsSigned, RM, IsExact);

  எனில் (FS == opInvalidOp) {
    மாறிலி unsigned Dst_Parts_மொத்தம் = part_மொத்தம்_For_Bits(Width);
    ப்ரதிஜ்ஞா(Dst_Parts_மொத்தம் <= உள்ளீடு.size() && "Integer too big");

    unsigned Bits;
    எனில் (getCategory() == fcNaN)
      Bits = 0;
    இல்லையெனில் எனில் (isNegative())
      Bits = IsSigned;
    இல்லையெனில்
      Bits = Width - IsSigned;

    tcSetLeastSignificantBits(உள்ளீடு.data(), Dst_Parts_மொத்தம், Bits);
    எனில் (isNegative() && IsSigned)
      APInt::tc_Shift_இடது(உள்ளீடு.data(), Dst_Parts_மொத்தம், Width - 1);
  }

  திரும்பு FS;
}

APFloat::opStatus DoubleAPFloat::handleOverflow(roundingMode RM) {
  தேர்வு (RM) {
  நிலை APFloat::rmTowardZero:
    makeLargest(/*Neg=*/isNegative());
    நிறுத்து;
  நிலை APFloat::rmTowardNegative:
    எனில் (isNegative())
      makeInf(/*Neg=*/உண்மை);
    இல்லையெனில்
      makeLargest(/*Neg=*/பொய்);
    நிறுத்து;
  நிலை APFloat::rmTowardPositive:
    எனில் (isNegative())
      makeLargest(/*Neg=*/உண்மை);
    இல்லையெனில்
      makeInf(/*Neg=*/பொய்);
    நிறுத்து;
  நிலை APFloat::rmNearestTiesToAway:
  நிலை APFloat::rmNearestTiesToEven:
    makeInf(/*Neg=*/isNegative());
    நிறுத்து;
  இயல்பு:
    llvm_unreachable("Invalid rounding mode found");
  }
  opStatus S = opInexact;
  எனில் (!getFirst().isFinite())
    S = static_cast<opStatus>(S | opOverflow);
  திரும்பு S;
}

APFloat::opStatus DoubleAPFloat::convertFromUnsignedParts(
    மாறிலி integerPart *Src, unsigned முழு Src_மொத்தம், roundingMode RM) {
  // Find the most significant bit of the source integer. APInt::tcMSB returns
  // UINT_MAX for a zero value.
  மாறிலி unsigned SrcMSB = APInt::tcMSB(Src, Src_மொத்தம்);
  எனில் (SrcMSB == UINT_MAX) {
    // The source integer is 0.
    makeZero(/*Neg=*/பொய்);
    திரும்பு opOK;
  }

  // Create a minimally-sized APInt to represent the source value.
  மாறிலி unsigned SrcBitWidth = SrcMSB + 1;
  APSInt SrcInt{APInt{/*numBits=*/SrcBitWidth, ArrayRef(Src, Src_மொத்தம்)},
                /*isUnsigned=*/உண்மை};

  // Stage 1: Initial Approximation.
  // Convert the source integer SrcInt to the Hi part of the DoubleAPFloat.
  // We use round-to-nearest because it minimizes the initial error, which is
  // crucial for the subsequent steps.
  APFloat Hi{getFirst().getSemantics()};
  Hi.convertFromAPInt(SrcInt, /*IsSigned=*/பொய், rmNearestTiesToEven);

  // If the first approximation already overflows, the number is too large.
  // NOTE: The underlying semantics are *more* conservative when choosing to
  // overflow because their notion of ULP is much larger. As such, it is always
  // safe to overflow at the DoubleAPFloat level if the APFloat overflows.
  எனில் (!Hi.isFinite())
    திரும்பு handleOverflow(RM);

  // Stage 2: Exact Error Calculation.
  // Calculate the exact error of the first approximation: Error = SrcInt - Hi.
  // This is done by converting Hi back to an integer and subtracting it from
  // the original source.
  தருக்க HiAsIntIsExact;
  // Create an integer representation of Hi. Its width is determined by the
  // exponent of Hi, ensuring it's just large enough. This width can exceed
  // SrcBitWidth if the conversion to Hi rounded up to a power of two.
  // accurately when converted back to an integer.
  APSInt HiAsInt{static_cast<uint32_t>(ilogb(Hi) + 1), /*isUnsigned=*/உண்மை};
  Hi.convertToInteger(HiAsInt, rmNearestTiesToEven, &HiAsIntIsExact);
  மாறிலி APInt Error = SrcInt.zext(HiAsInt.getBitWidth()) - HiAsInt;

  // Stage 3: Error Approximation and Rounding.
  // Convert the integer error into the Lo part of the DoubleAPFloat. This step
  // captures the remainder of the original number. The rounding mode for this
  // conversion (LoRM) may need to be adjusted from the user-requested RM to
  // ensure the final sum (Hi + Lo) rounds correctly.
  roundingMode LoRM = RM;
  // Adjustments are only necessary when the initial approximation Hi was an
  // overestimate, making the Error negative.
  எனில் (Error.isNegative()) {
    எனில் (RM == rmNearestTiesToAway) {
      // For rmNearestTiesToAway, a tie should round away from zero. Since
      // SrcInt is positive, this means rounding toward +infinity.
      // A standard conversion of a negative Error would round ties toward
      // -infinity, causing the final sum Hi + Lo to be smaller. To
      // counteract this, we detect the tie case and override the rounding
      // mode for Lo to rmTowardPositive.
      மாறிலி unsigned ErrorActiveBits = Error.getSignificantBits() - 1;
      மாறிலி unsigned LoPrecision = getSecond().getSemantics().precision;
      எனில் (ErrorActiveBits > LoPrecision) {
        மாறிலி unsigned RoundingBoundary = ErrorActiveBits - LoPrecision;
        // A tie occurs when the bits to be truncated are of the form 100...0.
        // This is detected by checking if the number of trailing zeros is
        // exactly one less than the number of bits being truncated.
        எனில் (Error.மொத்தம்_Trailing_Zeros() == RoundingBoundary - 1)
          LoRM = rmTowardPositive;
      }
    } இல்லையெனில் எனில் (RM == rmTowardZero) {
      // For rmTowardZero, the final positive result must be truncated (rounded
      // down). When Hi is an overestimate, Error is negative. A standard
      // rmTowardZero conversion of Error would make it *less* negative,
      // effectively rounding the final sum Hi + Lo *up*. To ensure the sum
      // rounds down correctly, we force Lo to round toward -infinity.
      LoRM = rmTowardNegative;
    }
  }

  APFloat Lo{getSecond().getSemantics()};
  opStatus Status = Lo.convertFromAPInt(Error, /*IsSigned=*/உண்மை, LoRM);

  // Renormalize the pair (Hi, Lo) into a canonical DoubleAPFloat form where the
  // components do not overlap. fastTwoSum performs this operation.
  std::tie(Hi, Lo) = fastTwoSum(Hi, Lo);
  Floats[0] = std::move(Hi);
  Floats[1] = std::move(Lo);

  // A final check for overflow is needed because fastTwoSum can cause a
  // carry-out from Lo that pushes Hi to infinity.
  எனில் (!getFirst().isFinite())
    திரும்பு handleOverflow(RM);

  // The largest DoubleAPFloat must be canonical. Values which are larger are
  // not canonical and are equivalent to overflow.
  எனில் (getFirst().isFiniteNonZero() && Floats[0].isLargest()) {
    DoubleAPFloat Largest{*Semantics};
    Largest.makeLargest(/*Neg=*/பொய்);
    எனில் (compare(Largest) == APFloat::cmpGreaterThan)
      திரும்பு handleOverflow(RM);
  }

  // The final status of the operation is determined by the conversion of the
  // error term. If Lo could represent Error exactly, the entire conversion
  // is exact. Otherwise, it's inexact.
  திரும்பு Status;
}

APFloat::opStatus DoubleAPFloat::convertFromAPInt(மாறிலி APInt &உள்ளீடு,
                                                  தருக்க IsSigned,
                                                  roundingMode RM) {
  மாறிலி தருக்க Negate_உள்ளீடு = IsSigned && உள்ளீடு.isNegative();
  APInt API = உள்ளீடு;
  எனில் (Negate_உள்ளீடு)
    API.negate();

  மாறிலி APFloat::opStatus Status =
      convertFromUnsignedParts(API.getRawData(), API.get_இலக்கம்_சொற்கள்(), RM);
  எனில் (Negate_உள்ளீடு)
    changeSign();
  திரும்பு Status;
}

unsigned முழு DoubleAPFloat::convertToHexString(எழுத்து *DST,
                                               unsigned முழு HexDigits,
                                               தருக்க UpperCase,
                                               roundingMode RM) மாறிலி {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  திரும்பு APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .convertToHexString(DST, HexDigits, UpperCase, RM);
}

தருக்க DoubleAPFloat::isDenormal() மாறிலி {
  திரும்பு getCategory() == fcNormal &&
         (Floats[0].isDenormal() || Floats[1].isDenormal() ||
          // (double)(Hi + Lo) == Hi defines a normal number.
          Floats[0] != Floats[0] + Floats[1]);
}

தருக்க DoubleAPFloat::isSmallest() மாறிலி {
  எனில் (getCategory() != fcNormal)
    திரும்பு பொய்;
  DoubleAPFloat Tmp(*இது);
  Tmp.makeSmallest(இது->isNegative());
  திரும்பு Tmp.compare(*இது) == cmpEqual;
}

தருக்க DoubleAPFloat::isSmallestNormalized() மாறிலி {
  எனில் (getCategory() != fcNormal)
    திரும்பு பொய்;

  DoubleAPFloat Tmp(*இது);
  Tmp.makeSmallestNormalized(இது->isNegative());
  திரும்பு Tmp.compare(*இது) == cmpEqual;
}

தருக்க DoubleAPFloat::isLargest() மாறிலி {
  எனில் (getCategory() != fcNormal)
    திரும்பு பொய்;
  DoubleAPFloat Tmp(*இது);
  Tmp.makeLargest(இது->isNegative());
  திரும்பு Tmp.compare(*இது) == cmpEqual;
}

தருக்க DoubleAPFloat::isInteger() மாறிலி {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  திரும்பு Floats[0].isInteger() && Floats[1].isInteger();
}

வெறுமை DoubleAPFloat::toString(SmallVectorImpl<எழுத்து> &Str,
                             unsigned FormatPrecision,
                             unsigned FormatMaxPadding,
                             தருக்க TruncateZero) மாறிலி {
  ப்ரதிஜ்ஞா(Semantics == &APFloatBase::semPPCDoubleDouble &&
         "Unexpected Semantics");
  APFloat(APFloatBase::semPPCDoubleDoubleLegacy, bitcastToAPInt())
      .toString(Str, FormatPrecision, FormatMaxPadding, TruncateZero);
}

முழு DoubleAPFloat::getExactLog2Abs() மாறிலி {
  // In order for Hi + Lo to be a power of two, the following must be true:
  // 1. Hi must be a power of two.
  // 2. Lo must be zero.
  எனில் (getSecond().isNonZero())
    திரும்பு INT_MIN;
  திரும்பு getFirst().getExactLog2Abs();
}

முழு ilogb(மாறிலி DoubleAPFloat &Arg) {
  மாறிலி APFloat &Hi = Arg.getFirst();
  மாறிலி APFloat &Lo = Arg.getSecond();
  முழு IlogbResult = ilogb(Hi);
  // Zero and non-finite values can delegate to ilogb(Hi).
  எனில் (Arg.getCategory() != fcNormal)
    திரும்பு IlogbResult;
  // If Lo can't change the binade, we can delegate to ilogb(Hi).
  எனில் (Lo.isZero() || Hi.isNegative() == Lo.isNegative())
    திரும்பு IlogbResult;
  எனில் (Hi.getExactLog2Abs() == INT_MIN)
    திரும்பு IlogbResult;
  // Numbers of the form 2^a - 2^b or -2^a + 2^b are almost powers of two but
  // get nudged out of the binade by the low component.
  திரும்பு IlogbResult - 1;
}

DoubleAPFloat scalbn(மாறிலி DoubleAPFloat &Arg, முழு Exp,
                     APFloat::roundingMode RM) {
  ப்ரதிஜ்ஞா(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");
  திரும்பு DoubleAPFloat(APFloatBase::PPCDoubleDouble(),
                       scalbn(Arg.Floats[0], Exp, RM),
                       scalbn(Arg.Floats[1], Exp, RM));
}

DoubleAPFloat frexp(மாறிலி DoubleAPFloat &Arg, முழு &Exp,
                    APFloat::roundingMode RM) {
  ப்ரதிஜ்ஞா(Arg.Semantics == &APFloatBase::PPCDoubleDouble() &&
         "Unexpected Semantics");

  // Get the unbiased exponent e of the number, where |Arg| = m * 2^e for m in
  // [1.0, 2.0).
  Exp = ilogb(Arg);

  // For NaNs, quiet any signaling NaN and return the result, as per standard
  // practice.
  எனில் (Exp == APFloat::IEK_NaN) {
    DoubleAPFloat Quiet{Arg};
    Quiet.getFirst() = Quiet.getFirst().makeQuiet();
    திரும்பு Quiet;
  }

  // For infinity, return it unchanged. The exponent remains IEK_Inf.
  எனில் (Exp == APFloat::IEK_Inf)
    திரும்பு Arg;

  // For zero, the fraction is zero and the standard requires the exponent be 0.
  எனில் (Exp == APFloat::IEK_Zero) {
    Exp = 0;
    திரும்பு Arg;
  }

  மாறிலி APFloat &Hi = Arg.getFirst();
  மாறிலி APFloat &Lo = Arg.getSecond();

  // frexp requires the fraction's absolute value to be in [0.5, 1.0).
  // ilogb provides an exponent for an absolute value in [1.0, 2.0).
  // Increment the exponent to ensure the fraction is in the correct range.
  ++Exp;

  மாறிலி தருக்க SignsDisagree = Hi.isNegative() != Lo.isNegative();
  APFloat Second = Lo;
  எனில் (Arg.getCategory() == APFloat::fcNormal && Lo.isFiniteNonZero()) {
    roundingMode LoRoundingMode;
    // The interpretation of rmTowardZero depends on the sign of the combined
    // Arg rather than the sign of the component.
    எனில் (RM == rmTowardZero)
      LoRoundingMode = Arg.isNegative() ? rmTowardPositive : rmTowardNegative;
    // For rmNearestTiesToAway, we face a similar problem. If signs disagree,
    // Lo is a correction *toward* zero relative to Hi. Rounding Lo
    // "away from zero" based on its own sign would move the value in the
    // wrong direction. As a safe proxy, we use rmNearestTiesToEven, which is
    // direction-agnostic. We only need to bother with this if Lo is scaled
    // down.
    இல்லையெனில் எனில் (RM == rmNearestTiesToAway && SignsDisagree && Exp > 0)
      LoRoundingMode = rmNearestTiesToEven;
    இல்லையெனில்
      LoRoundingMode = RM;
    Second = scalbn(Lo, -Exp, LoRoundingMode);
    // The rmNearestTiesToEven proxy is correct most of the time, but it
    // differs from rmNearestTiesToAway when the scaled value of Lo is an
    // exact midpoint.
    // NOTE: This is morally equivalent to roundTiesTowardZero.
    எனில் (RM == rmNearestTiesToAway && LoRoundingMode == rmNearestTiesToEven) {
      // Re-scale the result back to check if rounding occurred.
      மாறிலி APFloat RecomposedLo = scalbn(Second, Exp, rmNearestTiesToEven);
      எனில் (RecomposedLo != Lo) {
        // RoundingError tells us which direction we rounded:
        //   - RoundingError > 0: we rounded up.
        //   - RoundingError < 0: we down up.
        மாறிலி APFloat RoundingError = RecomposedLo - Lo;
        // Determine if scalbn(Lo, -Exp) landed exactly on a midpoint.
        // We do this by checking if the absolute rounding error is exactly
        // half a ULP of the result.
        மாறிலி APFloat UlpOfSecond = harrisonUlp(Second);
        மாறிலி APFloat ScaledUlpOfSecond =
            scalbn(UlpOfSecond, Exp - 1, rmNearestTiesToEven);
        மாறிலி தருக்க IsMidpoint = abs(RoundingError) == ScaledUlpOfSecond;
        மாறிலி தருக்க RoundedLoAway =
            Second.isNegative() == RoundingError.isNegative();
        // The sign of Hi and Lo disagree and we rounded Lo away: we must
        // decrease the magnitude of Second to increase the magnitude
        // First+Second.
        எனில் (IsMidpoint && RoundedLoAway)
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
    எனில் (Second.isZero() && SignsDisagree && Hi.getExactLog2Abs() != INT_MIN)
      ++Exp;
  }

  APFloat First = scalbn(Hi, -Exp, RM);
  திரும்பு DoubleAPFloat(APFloatBase::PPCDoubleDouble(), std::move(First),
                       std::move(Second));
}

APInt DoubleAPFloat::getNaNPayload() மாறிலி { திரும்பு Floats[0].getNaNPayload(); }
} // namespace detail

APFloat::Storage::Storage(IEEEFloat F, மாறிலி fltSemantics &Semantics) {
  எனில் (usesLayout<IEEEFloat>(Semantics)) {
    புதிய (&IEEE) IEEEFloat(std::move(F));
    திரும்பு;
  }
  எனில் (usesLayout<DoubleAPFloat>(Semantics)) {
    மாறிலி fltSemantics& S = F.getSemantics();
    புதிய (&Double) DoubleAPFloat(Semantics, APFloat(std::move(F), S),
                                APFloat(APFloatBase::IEEEdouble()));
    திரும்பு;
  }
  llvm_unreachable("Unexpected semantics");
}

Expected<APFloat::opStatus> APFloat::convertFromString(StringRef Str,
                                                       roundingMode RM) {
  APFLOAT_DISPATCH_ON_SEMANTICS(convertFromString(Str, RM));
}

hash_code hash_மதிப்பு(மாறிலி APFloat &Arg) {
  எனில் (APFloat::usesLayout<detail::IEEEFloat>(Arg.getSemantics()))
    திரும்பு hash_மதிப்பு(Arg.U.IEEE);
  எனில் (APFloat::usesLayout<detail::DoubleAPFloat>(Arg.getSemantics()))
    திரும்பு hash_மதிப்பு(Arg.U.Double);
  llvm_unreachable("Unexpected semantics");
}

APFloat::APFloat(மாறிலி fltSemantics &Semantics, StringRef S)
    : APFloat(Semantics) {
  auto StatusOrErr = convertFromString(S, rmNearestTiesToEven);
  ப்ரதிஜ்ஞா(StatusOrErr && "Invalid floating point representation");
  consumeError(StatusOrErr.takeError());
}

FPClassTest APFloat::classify() மாறிலி {
  எனில் (isZero())
    திரும்பு isNegative() ? fcNegZero : fcPosZero;
  எனில் (isNormal())
    திரும்பு isNegative() ? fcNegNormal : fcPosNormal;
  எனில் (isDenormal())
    திரும்பு isNegative() ? fcNegSubnormal : fcPosSubnormal;
  எனில் (isInfinity())
    திரும்பு isNegative() ? fcNegInf : fcPosInf;
  ப்ரதிஜ்ஞா(isNaN() && "Other class of FP constant");
  திரும்பு isSignaling() ? fcSNan : fcQNan;
}

தருக்க APFloat::getExactInverse(APFloat *Inv) மாறிலி {
  // Only finite, non-zero numbers can have a useful, representable inverse.
  // This check filters out +/- zero, +/- infinity, and NaN.
  எனில் (!isFiniteNonZero())
    திரும்பு பொய்;

  // Historically, this function rejects subnormal inputs.  One reason why this
  // might be important is that subnormals may behave differently under FTZ/DAZ
  // runtime behavior.
  எனில் (isDenormal())
    திரும்பு பொய்;

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
  மாறிலி முழு Exp = getExactLog2Abs();
  எனில் (Exp == INT_MIN)
    திரும்பு பொய்;

  // The inverse of +/- 2^Exp is +/- 2^(-Exp). We can compute this by
  // scaling 1.0 by the negated exponent.
  APFloat Reciprocal =
      scalbn(APFloat::getOne(getSemantics(), /*Negative=*/isNegative()), -Exp,
             rmTowardZero);

  // scalbn might round if the resulting exponent -Exp is outside the
  // representable range, causing overflow (to infinity) or underflow. We
  // must verify that the result is still the exact power of two we expect.
  எனில் (Reciprocal.getExactLog2Abs() != -Exp)
    திரும்பு பொய்;

  // Avoid multiplication with a subnormal, it is not safe on all platforms and
  // may be slower than a normal division.
  எனில் (Reciprocal.isDenormal())
    திரும்பு பொய்;

  ப்ரதிஜ்ஞா(Reciprocal.isFiniteNonZero());

  எனில் (Inv)
    *Inv = std::move(Reciprocal);

  திரும்பு உண்மை;
}

APFloat::opStatus APFloat::convert(மாறிலி fltSemantics &ToSemantics,
                                   roundingMode RM, தருக்க *losesInfo) {
  எனில் (&getSemantics() == &ToSemantics) {
    *losesInfo = பொய்;
    திரும்பு opOK;
  }
  எனில் (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics))
    திரும்பு U.IEEE.convert(ToSemantics, RM, losesInfo);
  எனில் (usesLayout<IEEEFloat>(getSemantics()) &&
      usesLayout<DoubleAPFloat>(ToSemantics)) {
    ப்ரதிஜ்ஞா(&ToSemantics == &APFloatBase::semPPCDoubleDouble);
    auto Ret =
        U.IEEE.convert(APFloatBase::semPPCDoubleDoubleLegacy, RM, losesInfo);
    *இது = APFloat(ToSemantics, U.IEEE.bitcastToAPInt());
    திரும்பு Ret;
  }
  எனில் (usesLayout<DoubleAPFloat>(getSemantics()) &&
      usesLayout<IEEEFloat>(ToSemantics)) {
    auto Ret = getIEEE().convert(ToSemantics, RM, losesInfo);
    *இது = APFloat(std::move(getIEEE()), ToSemantics);
    திரும்பு Ret;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat APFloat::get_All_Ones_மதிப்பு(மாறிலி fltSemantics &Semantics) {
  திரும்பு APFloat(Semantics, APInt::getAllOnes(Semantics.sizeInBits));
}

வெறுமை APFloat::print(raw_ostream &OS) மாறிலி {
  SmallVector<எழுத்து, 16> Buffer;
  toString(Buffer);
  OS << Buffer;
}

#எனில் !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD வெறுமை APFloat::dump() மாறிலி {
  print(dbgs());
  dbgs() << '\n';
}
#endif

வெறுமை APFloat::Profile(Folding_Set_முனை_ID &NID) மாறிலி {
  NID.Add(bitcastToAPInt());
}

APFloat::opStatus APFloat::convertToInteger(APSInt &result,
                                            roundingMode rounding_mode,
                                            தருக்க *isExact) மாறிலி {
  unsigned bitWidth = result.getBitWidth();
  SmallVector<uint64_t, 4> parts(result.get_இலக்கம்_சொற்கள்());
  opStatus status = convertToInteger(parts, bitWidth, result.isSigned(),
                                     rounding_mode, isExact);
  // Keeps the original signed-ness.
  result = APInt(bitWidth, parts);
  திரும்பு status;
}

இரட்டை APFloat::convertToDouble() மாறிலி {
  எனில் (&getSemantics() == &APFloatBase::semIEEEdouble)
    திரும்பு getIEEE().convertToDouble();
  ப்ரதிஜ்ஞா(isRepresentableBy(getSemantics(), semIEEEdouble) &&
         "Float semantics is not representable by IEEEdouble");
  APFloat Temp = *இது;
  தருக்க LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEdouble, rmNearestTiesToEven, &LosesInfo);
  ப்ரதிஜ்ஞா(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  திரும்பு Temp.getIEEE().convertToDouble();
}

#ifdef HAS_IEE754_FLOAT128
float128 APFloat::convertToQuad() மாறிலி {
  எனில் (&getSemantics() == &APFloatBase::semIEEEquad)
    திரும்பு getIEEE().convertToQuad();
  ப்ரதிஜ்ஞா(isRepresentableBy(getSemantics(), semIEEEquad) &&
         "Float semantics is not representable by IEEEquad");
  APFloat Temp = *இது;
  தருக்க LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEquad, rmNearestTiesToEven, &LosesInfo);
  ப்ரதிஜ்ஞா(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  திரும்பு Temp.getIEEE().convertToQuad();
}
#endif

தசம APFloat::convertToFloat() மாறிலி {
  எனில் (&getSemantics() == &APFloatBase::semIEEEsingle)
    திரும்பு getIEEE().convertToFloat();
  ப்ரதிஜ்ஞா(isRepresentableBy(getSemantics(), semIEEEsingle) &&
         "Float semantics is not representable by IEEEsingle");
  APFloat Temp = *இது;
  தருக்க LosesInfo;
  [[maybe_unused]] opStatus St =
      Temp.convert(APFloatBase::semIEEEsingle, rmNearestTiesToEven, &LosesInfo);
  ப்ரதிஜ்ஞா(!(St & opInexact) && !LosesInfo && "Unexpected imprecision");
  திரும்பு Temp.getIEEE().convertToFloat();
}

unsigned APFloatBase::getArbitraryFPFormatSizeInBits(StringRef Format) {
  திரும்பு StringSwitch<unsigned>(Format)
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

தருக்க APFloatBase::isValidArbitraryFPFormat(StringRef Format) {
  திரும்பு getArbitraryFPFormatSizeInBits(Format) != 0;
}

மாறிலி fltSemantics *APFloatBase::getArbitraryFPSemantics(StringRef Format) {
  // TODO: extend to remaining arbitrary FP types: Float8E4M3, Float8E3M4,
  // Float8E5M2FNUZ, Float8E4M3FNUZ, Float8E4M3B11FNUZ, Float8E8M0FNU.
  திரும்பு StringSwitch<மாறிலி fltSemantics *>(Format)
      .Case("Float8E5M2", &semFloat8E5M2)
      .Case("Float8E4M3FN", &semFloat8E4M3FN)
      .Case("Float8E5M3FNU", &semFloat8E5M3FNU)
      .Case("Float4E2M1FN", &semFloat4E2M1FN)
      .Case("Float6E3M2FN", &semFloat6E3M2FN)
      .Case("Float6E2M3FN", &semFloat6E2M3FN)
      .Default(nullptr);
}

APFloat::Storage::~Storage() {
  எனில் (usesLayout<IEEEFloat>(*semantics)) {
    IEEE.~IEEEFloat();
    திரும்பு;
  }
  எனில் (usesLayout<DoubleAPFloat>(*semantics)) {
    Double.~DoubleAPFloat();
    திரும்பு;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(மாறிலி APFloat::Storage &RHS) {
  எனில் (usesLayout<IEEEFloat>(*RHS.semantics)) {
    புதிய (இது) IEEEFloat(RHS.IEEE);
    திரும்பு;
  }
  எனில் (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    புதிய (இது) DoubleAPFloat(RHS.Double);
    திரும்பு;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage::Storage(APFloat::Storage &&RHS) {
  எனில் (usesLayout<IEEEFloat>(*RHS.semantics)) {
    புதிய (இது) IEEEFloat(std::move(RHS.IEEE));
    திரும்பு;
  }
  எனில் (usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    புதிய (இது) DoubleAPFloat(std::move(RHS.Double));
    திரும்பு;
  }
  llvm_unreachable("Unexpected semantics");
}

APFloat::Storage &APFloat::Storage::operator=(மாறிலி APFloat::Storage &RHS) {
  எனில் (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = RHS.IEEE;
  } இல்லையெனில் எனில் (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = RHS.Double;
  } இல்லையெனில் எனில் (இது != &RHS) {
    இது->~Storage();
    புதிய (இது) Storage(RHS);
  }
  திரும்பு *இது;
}

APFloat::Storage &APFloat::Storage::operator=(APFloat::Storage &&RHS) {
  எனில் (usesLayout<IEEEFloat>(*semantics) &&
      usesLayout<IEEEFloat>(*RHS.semantics)) {
    IEEE = std::move(RHS.IEEE);
  } இல்லையெனில் எனில் (usesLayout<DoubleAPFloat>(*semantics) &&
             usesLayout<DoubleAPFloat>(*RHS.semantics)) {
    Double = std::move(RHS.Double);
  } இல்லையெனில் எனில் (இது != &RHS) {
    இது->~Storage();
    புதிய (இது) Storage(std::move(RHS));
  }
  திரும்பு *இது;
}

நாமாகாஶ {

APFloat::opStatus getOpStatusFromLibc(முழு libc_exceptions) {
  APFloat::opStatus status = APFloat::opOK;
  எனில் (libc_exceptions & FE_INVALID)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInvalidOp);
  எனில் (libc_exceptions & FE_DIVBYZERO)
    status = static_cast<APFloat::opStatus>(status | APFloat::opDivByZero);
  எனில் (libc_exceptions & FE_OVERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opOverflow);
  எனில் (libc_exceptions & FE_UNDERFLOW)
    status = static_cast<APFloat::opStatus>(status | APFloat::opUnderflow);
  எனில் (libc_exceptions & FE_INEXACT)
    status = static_cast<APFloat::opStatus>(status | APFloat::opInexact);
  திரும்பு status;
}

} // namespace

// TODO: Support other rounding modes when LLVM libc math implement static
// roundings.
std::optional<APFloat> exp(மாறிலி APFloat &x, RoundingMode rounding_mode,
                           APFloat::opStatus *status) {

  எனில் (rounding_mode == APFloatBase::rmNearestTiesToEven) {
    எனில் (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEsingle) {
      தசம x_val = x.convertToFloat();
      முழு exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      எனில் (status) {
        *status = getOpStatusFromLibc(exc);
        எனில் (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      தசம result = LIBC_NAMESPACE::shared::expf(x_val);
      திரும்பு APFloat(result);
    }
    எனில் (APFloat::SemanticsToEnum(x.getSemantics()) ==
        APFloatBase::S_IEEEdouble) {
      இரட்டை x_val = x.convertToDouble();
      முழு exc =
          LIBC_NAMESPACE::shared::check::exp_exceptions(x_val, FE_TONEAREST);
      எனில் (status) {
        *status = getOpStatusFromLibc(exc);
        எனில் (x.isSignaling()) {
          // 32-bit x86 will silence sNaN when loading floats, so we explicitly
          // add the INVALID exception here.
          *status =
              static_cast<APFloat::opStatus>(*status | APFloat::opInvalidOp);
        }
      }
      இரட்டை result = LIBC_NAMESPACE::shared::exp(x_val);
      திரும்பு APFloat(result);
    }
  }
  திரும்பு std::nullopt;
}

} // namespace llvm

#undef APFLOAT_DISPATCH_ON_SEMANTICS
